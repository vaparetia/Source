/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef  _MSC_VER
#include <stdbool.h>
#include <alloca.h>
#include <spu_intrinsics.h>
#include "edge/edge_dma.h"
#include "edge/anim/edgeanim_spu.h"
#else
#include <memory.h>
#include <malloc.h>
#include "edge/anim/edgeanim_windows.h"
#endif

#include "edge/edge_assert.h"
#include "edge/edge_printf.h"
#include "edge/anim/edgeanim_common.h"

enum 
{
	EDGE_ANIM_DMA_TAG_SLOT_START = 2,
	EDGE_ANIM_DMA_TAG_SLOT_END = EDGE_ANIM_DMA_TAG_SLOT_START + 15,
};

// Macros used for PC implementation.
#ifdef _MSC_VER
#define spu_ienable()
#define spu_idisable()
#define spu_readch(c)   1
#define alloca _alloca

#define EDGE_DMA_PUTB(ls, ea, size, tag, tid, rid)	\
	do{												\
		(void)(tag);								\
		(void)(tid);								\
		(void)(rid);								\
		memcpy((void*)(ea), (void*)(ls), size);		\
	__pragma(warning(push))							\
	__pragma(warning(disable:4127))					\
	} while(0)										\
	__pragma(warning(push))
	
#define EDGE_DMA_GETB(ls, ea, size, tag, tid, rid)	\
	do{												\
		(void)(tag);								\
		(void)(tid);								\
		(void)(rid);								\
		memcpy((void*)(ls), (void*)(ea), size);		\
	__pragma(warning(push))							\
	__pragma(warning(disable:4127))					\
	} while(0)										\
	__pragma(warning(push))

#define EDGE_DMA_WAIT_TAG_STATUS_ALL(c)				\
	__pragma(warning(push))							\
	__pragma(warning(disable:4127))					\
	do{ (void)(c); } while(0)						\
	__pragma(warning(pop))

typedef	struct EdgeDmaListElement {
	uint16_t notify   :  1;
	uint16_t reserved : 15;
	uint16_t size;
	uint32_t eal;
} EdgeDmaListElement;
#endif

// TODO: fix maxSizeEvalBuffer - expose to user in a decent way 
void edgeAnimSpuInitialize(EdgeAnimSpuContext* spuContext, 
						   const EdgeAnimPpuContext* ppuContext, uint32_t spuId, 
						   void* lsStorage, uint32_t sizeLsStorage, 
						   uint32_t numJoints, uint32_t numUserChannels, 
						   uint32_t maxSizeEvalBuffer, uint32_t maxSizeUserBuffer)
{   
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_INITIALIZE);
	
	// check parameters 
	EDGE_ASSERT(spuContext);
	EDGE_ASSERT(ppuContext);
	EDGE_ASSERT(EDGE_IS_ALIGNED((uintptr_t) lsStorage, 16U));

	uint8_t* curLsStorage = (uint8_t*) lsStorage;
	size_t remainingSizeLsStorage = sizeLsStorage;

	// scratch space for animation headers / frame data set
	// - since they're used for DMAs, always round to the upper 16 bytes
	// - add 128 to realign eval scratch DMAs (ring bus / split cache lines) x 2	 
	spuContext->sizeEvalScratch = (uint32_t) EDGE_ALIGN(maxSizeEvalBuffer, 16U) + 256;   
	spuContext->sizeUserScratch = (uint32_t) EDGE_ALIGN(maxSizeUserBuffer, 16U);   
	unsigned int sizeScratch = spuContext->sizeEvalScratch + spuContext->sizeUserScratch;
	for(unsigned int scratchLoop = 0; 
		scratchLoop < sizeof(spuContext->evalScratch) / sizeof(spuContext->evalScratch[0]);
		scratchLoop++) {
		spuContext->evalScratch[scratchLoop] = curLsStorage;
		// update temporary ls storage status / make sure there's still enough memory 
		curLsStorage += sizeScratch;
		EDGE_ASSERT(remainingSizeLsStorage >= sizeScratch);
		remainingSizeLsStorage -= sizeScratch;
	}

	// get sizes of pose data
	uint32_t sizePose, sizePoseFlags, sizePoseJoints, sizePoseUserChannels, sizePoseJointWeights, sizePoseUserChannelWeights;
	_edgeAnimGetPoseDataSizes(numJoints, numUserChannels, sizePose, sizePoseFlags, sizePoseJoints, sizePoseUserChannels, sizePoseJointWeights, sizePoseUserChannelWeights);

	spuContext->sizePose = sizePose;
	spuContext->sizePoseFlags = sizePoseFlags;
	spuContext->sizePoseJoints = sizePoseJoints;
	spuContext->sizePoseUserChannels = sizePoseUserChannels;
	spuContext->sizePoseJointWeights = sizePoseJointWeights;
	spuContext->sizePoseUserChannelWeights = sizePoseUserChannelWeights;

	// allocate all remaining local storage space for the pose stack 
	spuContext->lsPosesBase = (uint8_t*) curLsStorage;
	spuContext->numLsPoses = (uint32_t) remainingSizeLsStorage / sizePose; 
	curLsStorage += spuContext->numLsPoses * sizePose;
	remainingSizeLsStorage -= spuContext->numLsPoses * sizePose;

	// we need *at least* 3 pose slots in LS 
	EDGE_ASSERT(spuContext->numLsPoses >= 3);

	// external storage space for the pose stack
	// note that if spuId value is not properly set, multiple 
	// instances of libedge running on multiple spus will conflict..	 
	spuContext->spuId = spuId;
	if (spuId < (sizeof(ppuContext->poseCacheArray) / sizeof(ppuContext->poseCacheArray[0]))) {
		spuContext->externalPosesBase = ppuContext->poseCacheArray[spuId].eaPoseCache;
		spuContext->numExternalPoses = ppuContext->poseCacheArray[spuId].sizePoseCache / sizePose;
	}
	else {
		spuContext->externalPosesBase = 0;
		spuContext->numExternalPoses = 0;   
	}

	// pose stack default state 
	spuContext->indexMruInLs = spuContext->numLsPoses - 1;
	spuContext->depth = 0;
}

void edgeAnimSpuFinalize(EdgeAnimSpuContext* )
{
	// nothing to do at the moment.. for future expansion 
}

static inline 
unsigned int _edgeAnimPoseStackGetDmaTag(unsigned indexSlot)
{
	// we  don't want an integer division here.. number of tags should be a power of two 
	return EDGE_ANIM_DMA_TAG_SLOT_START + indexSlot % 
		( EDGE_ANIM_DMA_TAG_SLOT_END - EDGE_ANIM_DMA_TAG_SLOT_START + 1U );
}

static inline 
unsigned int _edgeAnimPoseStackGetPoseInfo(EdgeAnimSpuContext* spuContext, 
										   EdgeAnimPoseInfo* pose, 
										   unsigned int depth)
{
	// we cannot reference stack entries that have spilled out to main store
	EDGE_ASSERT(depth < spuContext->numLsPoses);

	// index of the pose slot in the array 
	unsigned int indexPoseSlot = (spuContext->indexMruInLs >= depth)
		? spuContext->indexMruInLs - depth
		: spuContext->numLsPoses + spuContext->indexMruInLs - depth;	

	// fill pose info 
	EdgeAnimJointTransform* jointArray = reinterpret_cast<EdgeAnimJointTransform*>(spuContext->lsPosesBase 
		+ (spuContext->sizePose * indexPoseSlot));
	float* userChannelArray = reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(jointArray) 
		+ spuContext->sizePoseJoints);
	uint32_t* flags = reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(userChannelArray) 
		+ spuContext->sizePoseUserChannels);
	uint8_t* weightArray = reinterpret_cast<uint8_t*>(flags) + spuContext->sizePoseFlags; 
	uint8_t* userChannelWeightArray = reinterpret_cast<uint8_t*>(weightArray) + spuContext->sizePoseJointWeights; 
	pose->jointArray = jointArray;
	pose->weightArray = weightArray;
	pose->userChannelArray = userChannelArray;
	pose->userChannelWeightArray = userChannelWeightArray;
	pose->flags = flags;

	// return DMA tag 
	unsigned int dmaTag = _edgeAnimPoseStackGetDmaTag(indexPoseSlot);
	return dmaTag;
}

void edgeAnimPoseStackPush(EdgeAnimSpuContext* spuContext)
{   
	// sanity check : pose stack overflow 
	EDGE_ASSERT(spuContext->depth < (spuContext->numLsPoses + spuContext->numExternalPoses));

	// next slot (wrap) 
	unsigned int prevIndexMruInLs = spuContext->indexMruInLs;
	if (prevIndexMruInLs == (spuContext->numLsPoses - 1)) {
		spuContext->indexMruInLs = 0;
	}
	else {
		spuContext->indexMruInLs = prevIndexMruInLs + 1;
	}

	// update stack depth 
	spuContext->depth++;	

	// external storage management 
	if (__builtin_expect((spuContext->depth >= spuContext->numLsPoses), false)) {
		// predective barriered put of the next pose slot we may need on the next push
		// this may be unnecessary.. TODO: blocking but with less potential bandwidth overhead?		 
		unsigned int indexLocalPoseSlotToPut = spuContext->indexMruInLs + 1;
		if (indexLocalPoseSlotToPut == spuContext->numLsPoses) {
			indexLocalPoseSlotToPut = 0;
		}
		unsigned int indexExternalPoseSlotToPut = spuContext->depth - spuContext->numLsPoses;
		if ( (indexExternalPoseSlotToPut < spuContext->numExternalPoses) 
			&& spuContext->externalPosesBase) {
			unsigned int dmaTag = _edgeAnimPoseStackGetDmaTag(indexLocalPoseSlotToPut);	 
			if (spuContext->externalPosesBase) {

				// interrupt-friendly setup of PUTB DMA
				bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_PUTB(spuContext->lsPosesBase + indexLocalPoseSlotToPut * spuContext->sizePose,
					spuContext->externalPosesBase + indexExternalPoseSlotToPut * spuContext->sizePose,
					spuContext->sizePose, dmaTag, 0, 0);
				if(isEnabled) {
					spu_ienable();
				}
			}
		}
	} 
}

void edgeAnimPoseStackPop(EdgeAnimSpuContext* spuContext)
{   
	// sanity check : pose stack underflow
	EDGE_ASSERT(spuContext->depth);

	// prev slot (wrap)
	unsigned int prevIndexMruInLs = spuContext->indexMruInLs;
	if (!prevIndexMruInLs) {
		spuContext->indexMruInLs = spuContext->numLsPoses - 1;
	}
	else {
		spuContext->indexMruInLs = prevIndexMruInLs - 1;
	}

	// external storage management
	if ( spuContext->depth > spuContext->numLsPoses ) {
		unsigned int indexLocalPoseSlotToGet = prevIndexMruInLs;
		unsigned int indexExternalPoseSlotToGet = spuContext->depth - spuContext->numLsPoses  - 1;
		if ( (indexExternalPoseSlotToGet < spuContext->numExternalPoses) && spuContext->externalPosesBase) {
			unsigned int dmaTag = _edgeAnimPoseStackGetDmaTag(indexLocalPoseSlotToGet);	 
			if (spuContext->externalPosesBase) {

				// interrupt-friendly setup of GETB DMA
				bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_GETB(spuContext->lsPosesBase + indexLocalPoseSlotToGet * spuContext->sizePose,
					spuContext->externalPosesBase + indexExternalPoseSlotToGet * spuContext->sizePose,
					spuContext->sizePose, dmaTag, 0, 0);
				if(isEnabled) {
					spu_ienable();
				}
			}
		}		   
	}

	// update stack depth
	spuContext->depth--;
}

unsigned int edgeAnimPoseStackGetPose(EdgeAnimSpuContext* spuContext, 
									  EdgeAnimPoseInfo* pose, 
									  unsigned int depth)
{
	// fill user poseInfo
	unsigned int dmaTag = _edgeAnimPoseStackGetPoseInfo(spuContext, pose, depth);
	
	// stall for slot contents (interrupt-friendly setup of channel op)
	bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();
	EDGE_DMA_WAIT_TAG_STATUS_ALL(1U << dmaTag );
	if(isEnabled) {
		spu_ienable();
	}

	// return DMA tag
	return dmaTag;
}

#ifndef _MSC_VER
void _edgeAnimCopyQuadwords(void* dst, const void* src, unsigned int qwc)
{
	unsigned int qwc8 = qwc >> 3;

	const vector float* __restrict__ src0 = (vector float*)src;
	vector float* __restrict__ dst0 = (vector float*)dst;

	for (; qwc8; qwc8--)
	{
		const vector float a = src0[0];
		const vector float b = src0[1];
		const vector float c = src0[2];
		const vector float d = src0[3];
		const vector float e = src0[4];
		const vector float f = src0[5];
		const vector float g = src0[6];
		const vector float h = src0[7];

		dst0[0] = a;
		dst0[1] = b;
		dst0[2] = c;
		dst0[3] = d;
		dst0[4] = e;
		dst0[5] = f;
		dst0[6] = g;
		dst0[7] = h;

		src0 += 8;
		dst0 += 8;
	}

	unsigned int qwc0 = qwc & 7;
	for (; qwc0; qwc0--) {
		const vector float a = src0[0];

		dst0[0] = a;

		src0 += 1;
		dst0 += 1;
	}
}
#else
void _edgeAnimCopyQuadwords(void* dst, const void* src, unsigned int qwc)
{
	memcpy(dst, src, qwc*16);
}
#endif

static inline EdgeAnimRelativeBlendMode 
_edgeAnimGetRelativeBlendMode(EdgeAnimBlendOp blendOp)
{
	EdgeAnimRelativeBlendMode blendMode;
	switch(blendOp) {
		case EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_RIGHT:   
		case EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_LEFT:	
			blendMode = EDGE_ANIM_RELATIVE_BLEND_ADD_DELTA;
			break;
		case EDGE_ANIM_BLENDOP_COMPOSE_SUB_RIGHT_FROM_LEFT:	   
		case EDGE_ANIM_BLENDOP_COMPOSE_SUB_LEFT_FROM_RIGHT:		
			blendMode = EDGE_ANIM_RELATIVE_COMPOSE_SUB;
			break;
		case EDGE_ANIM_BLENDOP_COMPOSE_ADD_RIGHT:
		case EDGE_ANIM_BLENDOP_COMPOSE_ADD_LEFT:
			blendMode = EDGE_ANIM_RELATIVE_COMPOSE_ADD;
			break;
		default:
			blendMode = EDGE_ANIM_RELATIVE_NONE;
			break;
	}
	return blendMode;
}

void edgeAnimBlendPose(EdgeAnimSpuContext* spuContext, 
					   unsigned int poseDestDepth, 
					   unsigned int poseLeftDepth, 
					   unsigned int poseRightDepth, 
					   EdgeAnimBlendOp blendOp, 
					   float alpha,
					   const EdgeAnimSkeleton* skeleton)
{
	// get pose slots
	EdgeAnimPoseInfo EDGE_ALIGNED(16) poseDest;
	EdgeAnimPoseInfo EDGE_ALIGNED(16) poseLeft;
	EdgeAnimPoseInfo EDGE_ALIGNED(16) poseRight;

	edgeAnimPoseStackGetPose(spuContext, &poseDest, poseDestDepth);
	edgeAnimPoseStackGetPose(spuContext, &poseLeft, poseLeftDepth);
	edgeAnimPoseStackGetPose(spuContext, &poseRight, poseRightDepth);

	const uint8_t* userChannelFlagsArray = EDGE_OFFSET_GET_POINTER(const uint8_t, skeleton->offsetUserChannelFlagsArray);

	bool jointWeightRightDefined =  !!( (*(poseRight.flags)) & EDGE_ANIM_POSE_FLAG_JOINT_WEIGHTS_VALID);
	bool jointWeightLeftDefined = !!( (*(poseLeft.flags)) & EDGE_ANIM_POSE_FLAG_JOINT_WEIGHTS_VALID);

	bool userChanWeightRightDefined =  !!( (*(poseRight.flags)) & EDGE_ANIM_POSE_FLAG_USER_CHAN_WEIGHTS_VALID);
	bool userChanWeightLeftDefined = !!( (*(poseLeft.flags)) & EDGE_ANIM_POSE_FLAG_USER_CHAN_WEIGHTS_VALID);

	EdgeAnimRelativeBlendMode blendMode = _edgeAnimGetRelativeBlendMode(blendOp);
	
	// linear blend
	if (__builtin_expect(blendMode == EDGE_ANIM_RELATIVE_NONE, true)) {
		edgeAnimBlendJointsLinear(poseDest.jointArray, 
			poseDest.weightArray, 
			poseLeft.jointArray, 
			jointWeightLeftDefined? poseLeft.weightArray: 0,
			poseRight.jointArray, 
			jointWeightRightDefined? poseRight.weightArray: 0,
			alpha, skeleton->numJoints);					

		if(skeleton->numUserChannels) {
			edgeAnimBlendUserLinear(poseDest.userChannelArray, 
				poseDest.userChannelWeightArray, 
				poseLeft.userChannelArray, 
				userChanWeightLeftDefined? poseLeft.userChannelWeightArray: 0,
				poseRight.userChannelArray, 
				userChanWeightRightDefined? poseRight.userChannelWeightArray: 0,
				userChannelFlagsArray,
				alpha, skeleton->numUserChannels);					
		}
	}
	// relative (additive/subtractive) blend
	else {
		edgeAnimBlendJointsRelative(poseDest.jointArray, 
			poseDest.weightArray, 
			poseLeft.jointArray, 
			jointWeightLeftDefined? poseLeft.weightArray: 0,
			poseRight.jointArray, 
			jointWeightRightDefined? poseRight.weightArray: 0,
			alpha, blendMode, skeleton->numJoints);					

		if(skeleton->numUserChannels) {
			edgeAnimBlendUserRelative(poseDest.userChannelArray, 
				poseDest.userChannelWeightArray, 
				poseLeft.userChannelArray, 
				userChanWeightLeftDefined? poseLeft.userChannelWeightArray: 0,
				poseRight.userChannelArray, 
				userChanWeightRightDefined? poseRight.userChannelWeightArray: 0,
				userChannelFlagsArray,
				alpha, blendMode, skeleton->numUserChannels);					
		}
	}

	// mark destination weights as valid
	*(poseDest.flags) |= EDGE_ANIM_POSE_FLAG_WEIGHT_ARRAY_VALID;

    // set destination user flag if either left or right user flag is set
    *poseDest.flags |= (*poseLeft.flags & EDGE_ANIM_POSE_FLAG_USER_STATE) | (*poseRight.flags & EDGE_ANIM_POSE_FLAG_USER_STATE);
}

void edgeAnimWaitForCallbackDmas(void)
{
    register unsigned int kTagScratchBufferBase = 31 - 3;
    register unsigned int kTagUserBufferBase = kTagScratchBufferBase - 3;
    register unsigned int idxCmd0 = 0;
    register unsigned int idxCmd1 = 1;
    register unsigned int idxCmd2 = 2;
	register unsigned int tagMask = ((1U << (kTagUserBufferBase+idxCmd0))	|
									 (1U << (kTagUserBufferBase+idxCmd1))	|
									 (1U << (kTagUserBufferBase+idxCmd2)));
    bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
    spu_idisable();
	EDGE_DMA_WAIT_TAG_STATUS_ALL(tagMask);
    if(isEnabled) {
        spu_ienable();
    }
}

void edgeAnimProcessCommandList(EdgeAnimSpuContext* spuContext, 
								const EdgeAnimCommand* commandList, 
								const EdgeAnimSkeleton* skeleton,
								const EdgeAnimMirrorPair* mirrorPairs,
								uint32_t numMirrorPairs,
								const EdgeAnimBranchCallback branchCallback,
								const EdgeAnimLeafCallback leafCallback,
								const EdgeAnimUserCallback userCallback)
{   
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_PROCESS_COMMAND_LIST);

	// process command list (3 stages pipeline)
	EdgeAnimCommand EDGE_ALIGNED(16) dummyCmp;
	dummyCmp.command = EDGE_ANIM_CMD_END_LIST;
	
	// commands for each pipeline stage
	register const EdgeAnimCommand* cmd0 = &dummyCmp;
	register const EdgeAnimCommand* cmd1 = &dummyCmp;
	register const EdgeAnimCommand* cmd2 = commandList;

	// buffer indices / tag indices for each pipeline stage
	register unsigned int idxCmd0 = 0;
	register unsigned int idxCmd1 = 1;
	register unsigned int idxCmd2 = 2;

	// we need to keep track of the weights, for each stage, because they're used at the 
	// last stage when the animation header is alrady discarded
	EdgeAnimFrameSetInfo EDGE_ALIGNED(16) frameSetInfo[3];

	// realigned scratch addresses
	uint8_t* evalScratch[4] EDGE_ALIGNED_PS3(16);  
	uint8_t* frameSetLs[4] EDGE_ALIGNED_PS3(16);

	// HACK: will be changed when proper pose cache support is implemented
	register unsigned int kTagScratchBufferBase = 31 - 3;
	register unsigned int kTagUserBufferBase = kTagScratchBufferBase - 3;

	while(__builtin_expect(((cmd0->command != EDGE_ANIM_CMD_END_LIST) 
						  ||(cmd1->command != EDGE_ANIM_CMD_END_LIST) 
						  ||(cmd2->command != EDGE_ANIM_CMD_END_LIST)), true)) {

		// stage -2 : next next command
		switch (cmd2->command) {
		case EDGE_ANIM_CMD_EVAL:
		case EDGE_ANIM_CMD_PUSH_AND_EVAL:   {
				// start prefetching animation header (TODO: max size of header)
				EDGE_ASSERT(cmd2->leaf->animationHeaderSize <= spuContext->sizeEvalScratch);

				// interrupt-friendly setup of GETB DMA 
				bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				evalScratch[idxCmd2] = spuContext->evalScratch[idxCmd2] + ( cmd2->leaf->animationHeaderEa & 0x7FU );
				EDGE_DMA_GETB(evalScratch[idxCmd2], cmd2->leaf->animationHeaderEa, 
					cmd2->leaf->animationHeaderSize, kTagScratchBufferBase + idxCmd2, 0, 0);
				if(isEnabled) {
					spu_ienable();
				}

				// user leaf callback for stage -2
				if (__builtin_expect((uintptr_t)leafCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd2] + spuContext->sizeEvalScratch : 0;
					leafCallback(spuContext, cmd2->leaf, 0, skeleton, -2, kTagUserBufferBase + idxCmd2, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_PUSH_POSE:   {
				// user leaf callback for stage -2
				if (__builtin_expect((uintptr_t)leafCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd2] + spuContext->sizeEvalScratch : 0;
					leafCallback(spuContext, cmd2->leaf, 0, skeleton, -2, kTagUserBufferBase + idxCmd2, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_BLEND_AND_POP:   {			 
				// user branch callback for stage -2
				if (__builtin_expect((uintptr_t)branchCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd2] + spuContext->sizeEvalScratch : 0;
					branchCallback(spuContext, cmd2->branch, skeleton, -2, kTagUserBufferBase + idxCmd2, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_MIRROR:
		case EDGE_ANIM_CMD_END_LIST:
			break;
		default:	{
				// user callback for stage -2
				if (__builtin_expect((uintptr_t)userCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd2] + spuContext->sizeEvalScratch : 0;
					userCallback(spuContext, cmd2, skeleton, -2, kTagUserBufferBase + idxCmd2, userScratch);
				}
			}
			break;
		}


		// stage -1 :  next command : fetch additional bits referenced in the animation header (basePose etc)
		switch (cmd1->command)
		{
		case EDGE_ANIM_CMD_EVAL:
		case EDGE_ANIM_CMD_PUSH_AND_EVAL:   {			   
				// stall for previous DMA (interrupt-friendly setup)
				bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << (kTagScratchBufferBase + idxCmd1));
				if(isEnabled) {
					spu_ienable();
				}

				const EdgeAnimAnimation* animHeader = (const EdgeAnimAnimation*) evalScratch[idxCmd1];

				// validate anim header
				if (__builtin_expect(animHeader->tag != edgeAnimGetAnimTag(), false)) {
					EDGE_PRINTF("edgeAnim: unexpected animation tag\n");
					EDGE_ASSERT(false);
				}
			
				// search current frameset (binary search)
				const EdgeAnimFrameSetInfo* infoArray = 
					EDGE_OFFSET_GET_POINTER(EdgeAnimFrameSetInfo, animHeader->offsetFrameSetInfoArray);
				unsigned int leftIndex = 0;
				unsigned int rightIndex = animHeader->numFrameSets - 1;			 
				float frame = cmd1->leaf->evalTime * animHeader->sampleFrequency;
				if(__builtin_expect(frame < 0, false))
					frame = 0;
				unsigned short frameInteger = (unsigned short)frame;

				while (__builtin_expect((leftIndex + 1 < rightIndex), true)) {
					int midIndex = (leftIndex + rightIndex) >> 1;
					bool isSmaller = frameInteger < infoArray[midIndex].baseFrame;
					rightIndex = isSmaller? midIndex: rightIndex;
					leftIndex = isSmaller? leftIndex: midIndex;						
				}
				unsigned int frameSetIndex = leftIndex;

				// copy weights & frameset index (see above)
				frameSetInfo[idxCmd1] = infoArray[frameSetIndex];

				// initiate the frameset DMA for next stage
				const EdgeDmaListElement* frameSetDmaArray = 
					EDGE_OFFSET_GET_POINTER(EdgeDmaListElement, animHeader->offsetFrameSetDmaArray);			   
				const EdgeDmaListElement& frameSetDma = frameSetDmaArray[frameSetIndex];

				// EA of the frameset (offset relative to the animation header - EdgeDmaListElement is not relocated)
				uintptr_t frameSetEa = frameSetDma.eal + cmd1->leaf->animationHeaderEa;

				// frameset will overwrite the frameset info array and frameset dma array in scratch space
				// (realign to the same 128 bytes modulo to get better EIB performance)
				frameSetLs[idxCmd1] = (uint8_t*)frameSetDmaArray;
				frameSetLs[idxCmd1] += ( frameSetEa - (uintptr_t)frameSetLs[idxCmd1] ) & 0x7FU;
				EDGE_ASSERT((uintptr_t)(frameSetLs[idxCmd1] - spuContext->evalScratch[idxCmd1] + frameSetDma.size ) 
					<= spuContext->sizeEvalScratch);
		
				// interrupt-friendly setup of GETB DMA
				spu_idisable();
				EDGE_DMA_GETB(frameSetLs[idxCmd1], (frameSetDma.eal + cmd1->leaf->animationHeaderEa), 
					frameSetDma.size, (kTagScratchBufferBase + idxCmd1), 0, 0);
				if(isEnabled) {
					spu_ienable();
				}

				// user leaf callback for stage -1
				if (__builtin_expect((uintptr_t)leafCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd1] + spuContext->sizeEvalScratch : 0;
					leafCallback(spuContext, cmd1->leaf, 0, skeleton, -1, kTagUserBufferBase + idxCmd1, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_PUSH_POSE:   {
				// user leaf callback for stage -1
				if (__builtin_expect((uintptr_t)leafCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd1] + spuContext->sizeEvalScratch : 0;
					leafCallback(spuContext, cmd1->leaf, 0, skeleton, -1, kTagUserBufferBase + idxCmd1, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_BLEND_AND_POP:   {			 
				// user branch callback for stage -1
				if (__builtin_expect((uintptr_t)branchCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd1] + spuContext->sizeEvalScratch : 0;
					branchCallback(spuContext, cmd1->branch, skeleton, -1, kTagUserBufferBase + idxCmd1, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_MIRROR:
		case EDGE_ANIM_CMD_END_LIST:
			break;
		default:	{
				// user callback for stage -1
				if (__builtin_expect((uintptr_t)userCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd1] + spuContext->sizeEvalScratch : 0;
					userCallback(spuContext, cmd1, skeleton, -1, kTagUserBufferBase + idxCmd1, userScratch);
				}
			}
			break;
		}

		// Stage 0 : current command : execute
		switch (cmd0->command)
		{
		case EDGE_ANIM_CMD_PUSH_AND_EVAL:
			edgeAnimPoseStackPush(spuContext);
			// intentional fall through here...
		case EDGE_ANIM_CMD_EVAL:   {

				const EdgeAnimAnimation* animHeader = (const EdgeAnimAnimation*) evalScratch[idxCmd0];

				// get pointers from pose stack
				EdgeAnimPoseInfo poseMru;
				unsigned int dmaTagPose = edgeAnimPoseStackGetPose(spuContext, &poseMru, 0);
				
				// DMA weights if needed
				if (__builtin_expect(animHeader->sizeJointsWeightArray, 0U)) {
					// mark weights as valid and DMA them
					*poseMru.flags |= EDGE_ANIM_POSE_FLAG_WEIGHT_ARRAY_VALID;
					uintptr_t eaWeights = animHeader->eaUserJointWeightArray
						? (animHeader->eaUserJointWeightArray)
						: (animHeader->offsetJointsWeightArray + cmd0->leaf->animationHeaderEa);

					bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
					spu_idisable();
					EDGE_DMA_GETB(poseMru.weightArray, eaWeights, 
						animHeader->sizeJointsWeightArray, dmaTagPose, 0, 0);
					if(isEnabled) {
						spu_ienable();
					}
				}
				else {
					// mark weights as invalid / all ones (saves DMA / space)
					*poseMru.flags &= ~EDGE_ANIM_POSE_FLAG_WEIGHT_ARRAY_VALID;
				}
		  
				// stall for frameset DMAs
				bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << (kTagScratchBufferBase + idxCmd0));
				if(isEnabled) {
					spu_ienable();
				}

				// retrieve base pose (it is in the skeleton in a cached read-only buffer at the moment,
				// to avoid wasting precious main memory bandwidth)
				const EdgeAnimJointTransform* basePoseInLs = EDGE_OFFSET_GET_POINTER(EdgeAnimJointTransform, skeleton->offsetBasePose);

				// copy base pose into evaluation slot
				EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_COPY_BASE_POSE);
				unsigned int basePoseSize = sizeof(EdgeAnimJointTransform) * skeleton->numJoints;
				_edgeAnimCopyQuadwords(poseMru.jointArray, basePoseInLs, basePoseSize >> 4);

				// set user channels to zero by default
				// TODO: review this
				// TODO: optimise
				for(uint32_t i=0; i < skeleton->numUserChannels; i++)
					poseMru.userChannelArray[i] = 0.0f;

				unsigned int baseFrame = frameSetInfo[idxCmd0].baseFrame;
				unsigned int numIntraFrames = frameSetInfo[idxCmd0].numIntraFrames;

				float frame = (cmd0->leaf->evalTime * animHeader->sampleFrequency);

				// clamp to first frame
				if(__builtin_expect(frame < 0, false))
					frame = 0;

				// get integer & fractional parts of frame (relative to frameset)
				float frameSetFrame = frame - baseFrame;
				uint32_t frameInteger = (uint32_t)frameSetFrame;
				float frameFraction = frameSetFrame - (float)frameInteger;

				// clamp to last frame
				// (required for last frameset if we have gone beyond the end of the animation)
				if(__builtin_expect(frameInteger > numIntraFrames, false))
				{
					frameInteger = numIntraFrames;
					frameFraction = 1;
				}

				_edgeAnimEvaluate(
					poseMru.jointArray, 
					poseMru.userChannelArray, 
					animHeader,
					frameSetLs[idxCmd0],
					numIntraFrames, 
					frameInteger,
					frameFraction);

				// stall for weights DMA if any
				spu_idisable();
				EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << dmaTagPose);			 
				if(isEnabled) {
					spu_ienable();
				}

				// user leaf callback for stage 0
				if (__builtin_expect((uintptr_t)leafCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd0] + spuContext->sizeEvalScratch : 0;
					leafCallback(spuContext, cmd0->leaf, animHeader, skeleton, 0, kTagUserBufferBase + idxCmd0, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_PUSH_POSE: {
				edgeAnimPoseStackPush(spuContext);

				// get pointers from pose stack
				EdgeAnimPoseInfo poseMru;
				unsigned int dmaTagPose = edgeAnimPoseStackGetPose(spuContext, &poseMru, 0);

				if(cmd0->leaf->flags & EDGE_ANIM_FLAG_POSE_FROM_MAIN)
				{
					// fetch pose from main memory
					bool isEnabled = spu_readch(SPU_RdMachStat) & 1;
					spu_idisable();
					EDGE_DMA_GETB(spuContext->lsPosesBase + spuContext->indexMruInLs * spuContext->sizePose,
						cmd0->leaf->poseAddr, spuContext->sizePose, dmaTagPose, 0, 0);
					EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << dmaTagPose);
					if(isEnabled) {
						spu_ienable();
					}
				}
				else
				{
					// pose is in local store, so just copy it onto the pose stack
					_edgeAnimCopyQuadwords(spuContext->lsPosesBase + spuContext->indexMruInLs * spuContext->sizePose, 
										   (void*)cmd0->leaf->poseAddr, spuContext->sizePose >> 4);
				}

				// user leaf callback for stage 0
				if (__builtin_expect((uintptr_t)leafCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd0] + spuContext->sizeEvalScratch : 0;
					leafCallback(spuContext, cmd0->leaf, 0, skeleton, 0, kTagUserBufferBase + idxCmd0, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_BLEND_AND_POP: {						
				// blend operations
				EdgeAnimBlendOp blendOp = (EdgeAnimBlendOp) cmd0->branch->operation;
				switch (__builtin_expect(blendOp, EDGE_ANIM_BLENDOP_BLEND_LINEAR)) {

				// in place to left (depth=1) - base = left, delta = right
				case EDGE_ANIM_BLENDOP_BLEND_LINEAR: 
				case EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_RIGHT: 
				case EDGE_ANIM_BLENDOP_COMPOSE_ADD_RIGHT: 
				case EDGE_ANIM_BLENDOP_COMPOSE_SUB_RIGHT_FROM_LEFT: {
						edgeAnimBlendPose(spuContext, 1, 1, 0, blendOp, 
							cmd0->branch->alpha, skeleton);   
					}
					break;

				// in place to left (depth=1) - base = right, delta = left
				case EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_LEFT:
				case EDGE_ANIM_BLENDOP_COMPOSE_ADD_LEFT:
				case EDGE_ANIM_BLENDOP_COMPOSE_SUB_LEFT_FROM_RIGHT: {  
						edgeAnimBlendPose(spuContext, 1, 0, 1, blendOp, 
							cmd0->branch->alpha, skeleton);					   
					}
					break;
		   
				default:
					// unknown blend operation
					EDGE_ASSERT(false);
					break;
				}

				// pop				
				edgeAnimPoseStackPop(spuContext);

				// user branch callback for stage 0
				if (__builtin_expect((uintptr_t)branchCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd0] + spuContext->sizeEvalScratch : 0;
					branchCallback(spuContext, cmd0->branch, skeleton, 0, kTagUserBufferBase + idxCmd0, userScratch);
				}
			}
			break;
		case EDGE_ANIM_CMD_MIRROR: {						
				// mirror the pose at the top of the stack
				EdgeAnimPoseInfo poseMru;
				edgeAnimPoseStackGetPose(spuContext, &poseMru, 0);
				_edgeAnimMirrorJoints(poseMru.jointArray, poseMru.weightArray, mirrorPairs, numMirrorPairs);
			}
			break;
		case EDGE_ANIM_CMD_END_LIST:
			break;
		default:	{
				// user callback for stage 0
				if (__builtin_expect((uintptr_t)userCallback, 0U)) {
					void* userScratch = spuContext->sizeUserScratch ? spuContext->evalScratch[idxCmd0] + spuContext->sizeEvalScratch : 0;
					userCallback(spuContext, cmd0, skeleton, 0, kTagUserBufferBase + idxCmd0, userScratch);
				}
			}
			break;
		}	   
			 
		// cycle indices
		unsigned int prevIdxCmd0 = idxCmd0;
		idxCmd0 = idxCmd1;
		idxCmd1 = idxCmd2;
		idxCmd2 = prevIdxCmd0;

		// next command?
		cmd0 = cmd1;
		cmd1 = cmd2;
		cmd2 = (cmd1->command == EDGE_ANIM_CMD_END_LIST)? cmd1: ++commandList;  
	}
}

EdgeAnimCommand* _edgeAnimRecursiveProcessBlendTree(EdgeAnimCommand* commandList,
													EdgeAnimCommand* const commandListEnd,
													const unsigned int branch,
													const EdgeAnimBlendBranch* blendBranches, 
													const EdgeAnimBlendLeaf* blendLeaves)
{
	// current branch
	const EdgeAnimBlendBranch* currentBranch = blendBranches + 
		(branch & ~EDGE_ANIM_BLEND_TREE_INDEX_BRANCH);

	// check command list overflow
	if (__builtin_expect(commandList >= commandListEnd, false)) {
		return commandList;
	}

	// recurse left branch...
	if (currentBranch->left & EDGE_ANIM_BLEND_TREE_INDEX_BRANCH) {		
		commandList = _edgeAnimRecursiveProcessBlendTree(commandList, 
			commandListEnd,
			currentBranch->left & ~EDGE_ANIM_BLEND_TREE_INDEX_BRANCH, 
			blendBranches, blendLeaves);
	}
	// ...or eval left leaf
	else if (currentBranch->left & EDGE_ANIM_BLEND_TREE_INDEX_LEAF) {
		const EdgeAnimBlendLeaf* leaf = blendLeaves + (currentBranch->left & ~EDGE_ANIM_BLEND_TREE_INDEX_LEAF);
		if(__builtin_expect(leaf->flags & (EDGE_ANIM_FLAG_POSE_FROM_MAIN | EDGE_ANIM_FLAG_POSE_FROM_LOCAL), false)) {
			commandList->command = EDGE_ANIM_CMD_PUSH_POSE;
		}
		else {
			commandList->command = EDGE_ANIM_CMD_PUSH_AND_EVAL;
		}
		commandList->leaf = leaf;
		commandList++;
		// if leaf is mirrored, add a mirror command
		if(leaf->flags & EDGE_ANIM_FLAG_MIRROR)
		{
			commandList->command = EDGE_ANIM_CMD_MIRROR;
			commandList++;	  
		}
	 }
	// .. or invalid left index
	else {
		EDGE_ASSERT(false);
	}
	
	// check command list overflow
	if (__builtin_expect(commandList >= commandListEnd, false)) {
		return commandList;
	}

	// recurse right branch...
	if (currentBranch->right & EDGE_ANIM_BLEND_TREE_INDEX_BRANCH) {
		commandList = _edgeAnimRecursiveProcessBlendTree(commandList, 
			commandListEnd,
			currentBranch->right & ~EDGE_ANIM_BLEND_TREE_INDEX_BRANCH, 
			blendBranches, blendLeaves);
	}   
	// ...or eval right leaf
	else if (currentBranch->right & EDGE_ANIM_BLEND_TREE_INDEX_LEAF) {
		const EdgeAnimBlendLeaf* leaf = blendLeaves + (currentBranch->right & ~EDGE_ANIM_BLEND_TREE_INDEX_LEAF);
		if(__builtin_expect(leaf->flags & (EDGE_ANIM_FLAG_POSE_FROM_MAIN | EDGE_ANIM_FLAG_POSE_FROM_LOCAL), false)) {
			commandList->command = EDGE_ANIM_CMD_PUSH_POSE;
		}
		else {
			commandList->command = EDGE_ANIM_CMD_PUSH_AND_EVAL;
		}
		commandList->leaf = leaf;
		commandList++;
		// if leaf is mirrored, add a mirror command
		if(leaf->flags & EDGE_ANIM_FLAG_MIRROR)
		{
			commandList->command = EDGE_ANIM_CMD_MIRROR;
			commandList++;	  
		}
	}
	// ...or invalid right index
	else {
		EDGE_ASSERT(false);
	}

	// check command list overflow
	if (__builtin_expect(commandList >= commandListEnd, false)) {
		return commandList;
	}
	
	// add BlendAndPop
	commandList->command = EDGE_ANIM_CMD_BLEND_AND_POP;
	commandList->branch = currentBranch;
	commandList++;  
	// if branch is mirrored, add a mirror command
	if(currentBranch->flags & EDGE_ANIM_FLAG_MIRROR)
	{
		commandList->command = EDGE_ANIM_CMD_MIRROR;
		commandList++;	  
	}

	// return next command to write
	return commandList;
}

void edgeAnimProcessBlendTree(EdgeAnimSpuContext* spuContext,
							  const uint16_t rootIndex,
							  const EdgeAnimBlendBranch* blendBranches, 
							  unsigned int numBranches,
							  const EdgeAnimBlendLeaf* blendLeaves, 
							  unsigned int numLeaves,							 
							  const EdgeAnimSkeleton* skeleton,
							  const EdgeAnimMirrorPair* mirrorPairs,
							  uint32_t numMirrorPairs,
							  const EdgeAnimBranchCallback branchCallback,
							  const EdgeAnimLeafCallback leafCallback)
{
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_PROCESS_BLEND_TREE);

	// rootIndex must be either a leaf or a branch index
	EDGE_ASSERT(((rootIndex & EDGE_ANIM_BLEND_TREE_INDEX_LEAF) != 0) 
			  ^ ((rootIndex & EDGE_ANIM_BLEND_TREE_INDEX_BRANCH) != 0) );

	// allocate stack space for commands (pessimistic worst case)
	// note: there can potentially be two commands per leaf/branch (due to mirroring)
	unsigned int maxCommands = numLeaves * 2 + numBranches * 2 + 2;
	EdgeAnimCommand* commandListStart = (EdgeAnimCommand*) 
		alloca(sizeof(EdgeAnimCommand) * maxCommands);
	EdgeAnimCommand* commandListEnd = commandListStart + maxCommands;
	EdgeAnimCommand* commandList = commandListStart;

	// process the tree if root is a branch
	uint16_t maskedRootIndex = rootIndex & ~(EDGE_ANIM_BLEND_TREE_INDEX_BRANCH 
										   | EDGE_ANIM_BLEND_TREE_INDEX_LEAF);
	if (__builtin_expect((rootIndex & EDGE_ANIM_BLEND_TREE_INDEX_BRANCH), true)) {	  
		EDGE_ASSERT(maskedRootIndex < numBranches);
		commandList = _edgeAnimRecursiveProcessBlendTree(commandList, 
			commandListEnd, maskedRootIndex, blendBranches, blendLeaves);
	}
	// otherwise, add a single leaf to the command list
	else if (rootIndex & EDGE_ANIM_BLEND_TREE_INDEX_LEAF) {
		EDGE_ASSERT(maskedRootIndex < numLeaves);
		const EdgeAnimBlendLeaf* leaf = blendLeaves + maskedRootIndex;;
		if(__builtin_expect(leaf->flags & (EDGE_ANIM_FLAG_POSE_FROM_MAIN | EDGE_ANIM_FLAG_POSE_FROM_LOCAL), false)) {
			commandList->command = EDGE_ANIM_CMD_PUSH_POSE;
		}
		else {
			commandList->command = EDGE_ANIM_CMD_PUSH_AND_EVAL;
		}
		commandList->leaf = leaf;
		commandList++;	  
		// if leaf is mirrored, add a mirror command
		if(leaf->flags & EDGE_ANIM_FLAG_MIRROR)
		{
			commandList->command = EDGE_ANIM_CMD_MIRROR;
			commandList++;	  
		}
	} 
	// neither a branch nor a leaf?
	else {
		// this is just wrong - rootIndex must be either a tree or a leaf
		EDGE_ASSERT(false);
	}

	// check that there is no overflow (=recursion in the blend tree)
	EDGE_ASSERT(commandList < commandListEnd);

	// add end marker
	if (__builtin_expect(commandList < commandListEnd, true)) {
		commandList->command = EDGE_ANIM_CMD_END_LIST;
		commandList++;
	}

	// make sure we haven't overwritten something in the stack
	EDGE_ASSERT(commandList <= commandListEnd);

	// process command list
	edgeAnimProcessCommandList(spuContext, commandListStart, skeleton, mirrorPairs, numMirrorPairs, branchCallback, leafCallback);
}

