/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ANIM_SPU_H__
#define __EDGE_ANIM_SPU_H__

#include <float.h>
#include "edge/anim/edgeanim_structs_ps3.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

// Typedef definitions / callbacks
typedef void (* EdgeAnimLeafCallback) (EdgeAnimSpuContext* spuContext, const EdgeAnimBlendLeaf* const leaf, const EdgeAnimAnimation* anim, const EdgeAnimSkeleton* const skel, const int pipelineStage, const unsigned int dmaTag, void* userScratchBuffer);
typedef void (* EdgeAnimBranchCallback) (EdgeAnimSpuContext* spuContext, const EdgeAnimBlendBranch* const branch, const EdgeAnimSkeleton* const skel, const int pipelineStage, const unsigned int dmaTag, void* userScratchBuffer);
typedef void (* EdgeAnimUserCallback) (EdgeAnimSpuContext* spuContext, const EdgeAnimCommand* const cmd, const EdgeAnimSkeleton* const skel, const int pipelineStage, const unsigned int dmaTag, void* userScratchBuffer);

typedef enum EdgeAnimRelativeBlendMode
{
	EDGE_ANIM_RELATIVE_BLEND_ADD_DELTA	= 0x000,
	EDGE_ANIM_RELATIVE_COMPOSE_ADD		= 0x010,
	EDGE_ANIM_RELATIVE_COMPOSE_SUB		= 0x011,
	EDGE_ANIM_RELATIVE_NONE				= 0x100
} EdgeAnimRelativeBlendMode;

// Edge animation init
void edgeAnimSpuInitialize(EdgeAnimSpuContext* spuContext, const EdgeAnimPpuContext* ppuContext, uint32_t spuId, void* lsStorage, uint32_t sizeLsStorage, uint32_t numJoints, uint32_t numUserChannels, uint32_t maxSizeEvalBuffer = EDGE_ANIM_EVAL_BUFFER_SIZE, uint32_t maxSizeUserBuffer = 0);
void edgeAnimSpuFinalize(EdgeAnimSpuContext* spuContext);

// Main blend tree processing (ignore, if using edgeAnimProcessCommandList)
void edgeAnimProcessBlendTree(EdgeAnimSpuContext* spuContext, const uint16_t rootIndex, const EdgeAnimBlendBranch* blendBranches, unsigned int numBranches, const EdgeAnimBlendLeaf* blendLeaves, unsigned int numLeaves, const EdgeAnimSkeleton* skeleton, const EdgeAnimMirrorPair* mirrorPairs = 0, uint32_t numMirrorPairs = 0, const EdgeAnimBranchCallback branchCallback = 0, const EdgeAnimLeafCallback leafCallback = 0);

// Command list processing (internal, if using edgeAnimProcessBlendTree)
void edgeAnimProcessCommandList(EdgeAnimSpuContext* spuContext, const EdgeAnimCommand* commandList, const EdgeAnimSkeleton* skeleton, const EdgeAnimMirrorPair* mirrorPairs = 0, uint32_t numMirrorPairs = 0, const EdgeAnimBranchCallback branchCallback = 0, const EdgeAnimLeafCallback leafCallback = 0, const EdgeAnimUserCallback userCallback = 0);

// Synchronisation of DMA transfers originating in command list callbacks
void edgeAnimWaitForCallbackDmas(void);

// Joint processing functions
void edgeAnimLocalJointsToWorldJoints(EdgeAnimJointTransform* outputJoints, const EdgeAnimJointTransform* inputJoints, const EdgeAnimJointTransform* rootJoint, const unsigned short* jointLinkage, unsigned int count);
void edgeAnimLocalJointsToWorldMatrices3x4(void* outputMatrices, const EdgeAnimJointTransform* inputJoints, const EdgeAnimJointTransform* rootJoint, const unsigned short* jointLinkage, unsigned int count);
void edgeAnimLocalJointsToWorldMatrices4x4(void* outputMatrices, const EdgeAnimJointTransform* inputJoints, const EdgeAnimJointTransform* rootJoint, const unsigned short* jointLinkage, unsigned int count);
void edgeAnimWorldJointsToLocalJoints(EdgeAnimJointTransform* outputJoints, const EdgeAnimJointTransform* inputJoints, const EdgeAnimJointTransform* rootJoint, const unsigned short* jointLinkage, unsigned int count);
void edgeAnimJointsToMatrices4x4(void* outputMatrices, const EdgeAnimJointTransform* inputJoints, unsigned int count);
void edgeAnimMatrices4x4ToJoints(EdgeAnimJointTransform* outputJoints, const void* inputMatrices, unsigned int count);
void edgeAnimJointsToMatrices3x4(void* outputMatrices, const EdgeAnimJointTransform* inputJoints, unsigned int count);
void edgeAnimMatrices3x4ToJoints(EdgeAnimJointTransform* outputJoints, const void* inputMatrices, unsigned int count);
void edgeAnimMultiplyMatrices3x4(void* outputMatrices, const void* leftMatrices, const void* rightMatrices, unsigned int count);

// Joint blending functions (semi-internal)
void edgeAnimBlendJointsLinear(EdgeAnimJointTransform* outputJoints, uint8_t* outputWeights, const EdgeAnimJointTransform* leftJoints, const uint8_t* leftWeights, const EdgeAnimJointTransform* rightJoints, const uint8_t* rightWeights, float alpha, unsigned int count);
void edgeAnimBlendJointsRelative(EdgeAnimJointTransform* outputJoints, uint8_t* outputWeights, const EdgeAnimJointTransform* leftJoints, const uint8_t* leftWeights, const EdgeAnimJointTransform* rightJoints, const uint8_t* rightWeights, float alpha, EdgeAnimRelativeBlendMode blendMode, unsigned int count);

// User channel blending functions (semi-internal)
void edgeAnimBlendUserLinear(float* outputChannels, uint8_t* outputWeights, const float* leftChannels, const uint8_t* leftWeights, const float* rightChannels, const uint8_t* rightWeights, const uint8_t* channelFlags, float alpha, unsigned int count);
void edgeAnimBlendUserRelative(float* outputChannels, uint8_t* outputWeights, const float* leftChannels, const uint8_t* leftWeights, const float* rightChannels, const uint8_t* rightWeights, const uint8_t* channelFlags, float alpha, EdgeAnimRelativeBlendMode blendMode, unsigned int count);

// Pose stack
void edgeAnimPoseStackPush(EdgeAnimSpuContext* spuContext);
void edgeAnimPoseStackPop(EdgeAnimSpuContext* spuContext);
unsigned int edgeAnimPoseStackGetPose(EdgeAnimSpuContext* spuContext, EdgeAnimPoseInfo* pose, unsigned int depth);
void edgeAnimBlendPose(EdgeAnimSpuContext* spuContext, unsigned int poseDestDepth, unsigned int poseLeftDepth, unsigned int poseRightDepth, EdgeAnimBlendOp blendOp, float alpha, const EdgeAnimSkeleton* skeleton);

// Internal
void _edgeAnimEvaluate(EdgeAnimJointTransform* outputJoints, float* outputUserChannels, const EdgeAnimAnimation* animHeader, const void* frameSetData, uint32_t intraFrameCount, uint32_t frameInteger, float frameFraction);
void _edgeAnimMirrorJoints(EdgeAnimJointTransform* joints, uint8_t* weights, const EdgeAnimMirrorPair* mirrorSpec, unsigned int count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __EDGE_ANIM_SPU_H__
