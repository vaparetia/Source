/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ANIM_STRUCTS_H__
#define __EDGE_ANIM_STRUCTS_H__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression constant
#endif

#include "edge/edge_stdint.h"
#include "edge/edge_stdbool.h"
#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_macros.h"

// Definitions

#define EDGE_ANIM_MAX_SPUS                      (6)
#define EDGE_ANIM_POSE_FLAG_JOINT_WEIGHTS_VALID  	(0x01U)	/* indicates that the pose has a valid set of joint weights */
#define EDGE_ANIM_POSE_FLAG_USER_CHAN_WEIGHTS_VALID	(0x02U)	/* indicates that the pose has a valid set of user channel weights */
#define EDGE_ANIM_POSE_FLAG_WEIGHT_ARRAY_VALID		(0x03U)	/* indicates that the pose has valid joint & user channel weights */
#define EDGE_ANIM_POSE_FLAG_USER_STATE				(0x04U)	/* user pose state that gets propagated (OR) through blends */

#define EDGE_ANIM_BLEND_TREE_INDEX_LEAF         (0x8000U)   /* blend tree node is a leaf */
#define EDGE_ANIM_BLEND_TREE_INDEX_BRANCH       (0x4000U)   /* blend tree node is a branch */

#define EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01     (0x01U)     /* clamp user data to [0..1] range after (additive/subtractive) blends */
#define EDGE_ANIM_USER_CHANNEL_FLAG_MINMAX      (0x02U)     /* fuzzy mode : additive/subtractive blends a respectively replaced by max and min */
#define EDGE_ANIM_USER_CHANNEL_FLAG_DEFAULT0    (0x04U)     /* channel value defaults to 0 instead of undefined */
#define EDGE_ANIM_USER_CHANNEL_FLAG_CPT_BIT1	(0x08U)     /* component index bit 1 */
#define EDGE_ANIM_USER_CHANNEL_FLAG_CPT_BIT0	(0x10U)     /* component index bit 0 */

#define EDGE_ANIM_FLAG_BIT_PACKED_R				(0x01U)     /* rotations are bit packed */
#define EDGE_ANIM_FLAG_BIT_PACKED_T				(0x02U)     /* translations are bit packed */
#define EDGE_ANIM_FLAG_BIT_PACKED_S				(0x04U)     /* scales are bit packed */
#define EDGE_ANIM_FLAG_BIT_PACKED_U				(0x08U)     /* user channels are bit packed */

#define EDGE_ANIM_MIRROR_SPEC_NON_PAIRED		(0x881122b3U)	/* mirroring operation for non-paired joints */
#define EDGE_ANIM_MIRROR_SPEC_LINK				(0xb8219203U)	/* mirroring operation for paired joints that have non-paired parents */
#define EDGE_ANIM_MIRROR_SPEC_PAIRED			(0x08192233U)	/* mirroring operation for paired joints that have paired parents */

#define EDGE_ANIM_FLAG_MIRROR					(0x01U)		/* leaf/branch flag to enable mirroring */
#define EDGE_ANIM_FLAG_POSE_FROM_MAIN			(0x02U)		/* leaf flag to specify a pre-computed pose in main memory */
#define EDGE_ANIM_FLAG_POSE_FROM_LOCAL			(0x04U)		/* leaf flag to specify a pre-computed pose in local memory */

// Warnings

#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4200) // warning about zero-sized array
#endif

// PPU (global) context

typedef struct EDGE_ALIGNED(16) EdgeAnimPpuContext
{
    struct ExternalPoseCache
    {
        uint32_t    sizePoseCache;
        uintptr_t   eaPoseCache;
    } poseCacheArray[EDGE_ANIM_MAX_SPUS];
} EdgeAnimPpuContext;

// SPU (local) context

typedef struct EDGE_ALIGNED_PS3(16) EdgeAnimSpuContext
{
    uint32_t    spuId;                          /* current spu index (0..n) - to avoid a spurs-specific inside libEdgeAnim  */

    // scratch evaluation memory setup
    uint8_t*    evalScratch[3];                 /* evaluation scratch buffers */
    uint32_t    sizeEvalScratch;                /* size of one eval scratch buffer - to do validation */
    uint32_t    sizeUserScratch;                /* size of one user scratch buffer */

    // pose cache setup
    uint8_t*    lsPosesBase;                    /* base address of LS pose stack */
    uint32_t    numLsPoses;                     /* number of slots in the LS pose stack */
    uintptr_t   externalPosesBase;              /* base address of main memory pose stack extension */
    uint32_t    numExternalPoses;               /* number of slots in the external main memory pose stack extension */
    uint32_t    sizePose;                       /* size of one slot in the pose stack */
    uint32_t    sizePoseJoints;                 /* size of joint data in a given slot */
    uint32_t    sizePoseUserChannels;           /* size of user channel data in a given slot */
    uint32_t    sizePoseJointWeights;           /* size of joint weights in a given slot */
    uint32_t    sizePoseUserChannelWeights;     /* size of user channel weights in a given slot */
    uint32_t    sizePoseFlags;                  /* size of flags in a given slot */

    // current pose cache state
    uint32_t    depth;                          /* current depth in the pose stack */
    uint32_t    indexMruInLs;                   /* index of the MRU slot - that actually (depth - 1) % numSlots, but we keep it to avoid an integer division */  

	// user data
	uint32_t	userData;						/* accessible from callbacks */

    // padding
    uint8_t     pad[4];                         /* padding */
} EdgeAnimSpuContext;

// Default evaluation buffer size
// TODO: determine what this value should be
#define EDGE_ANIM_EVAL_BUFFER_SIZE              (8*1024)

// Skeleton header - followed by skeleton data

typedef struct EDGE_ALIGNED_PS3(16) EdgeAnimSkeleton    // Must be aligned to 16 bytes
{
	uint32_t    tag;                                /* 0x00: version tag - see edgeAnimGetSkelTag() */
	uint32_t    sizeTotal;                          /* 0x04: total size - in bytes, aligned 16 */
	uint32_t    sizeCustomData;                     /* 0x08: size of custom data, including table - in bytes, aligned 16 */
	uint32_t    sizeNameHashes;                     /* 0x0C: size of joint and user channel name hashes - in bytes, aligned 16 */
	uint16_t    numJoints;                          /* 0x10: number of joints */
	uint16_t    numUserChannels;                    /* 0x12: number of user channels */
	uint16_t    numSimdHierarchyQuads;              /* 0x14: number of "quads" in simd hierarchy - TODO: RENAME */
	uint16_t    locomotionJointIndex;               /* 0x16: index of the joint used for locomotion deltas */
	uint32_t    offsetBasePose;                     /* 0x18: offset of the base pose (EdgeAnimJointTransform*) - numJoints values */
	uint32_t    offsetParentIndicesArray;           /* 0x1C: offset to parent indices array (int16_t*) relative to this address - numJoints values */
	uint32_t    offsetJointNameHashArray;           /* 0x20: offset to joint name hash array (unsigned int*) relative to this address - numJoints values */ 
	uint32_t    offsetUserChannelNameHashArray;     /* 0x24: offset to user channel name hash array (unsigned int*) relative to this address - numUserChannels values */ 
	uint32_t    offsetUserChannelNodeNameHashArray; /* 0x28: offset to user channel node name hash array (unsigned int*) relative to this address - numUserChannels values */ 
	uint32_t    offsetUserChannelFlagsArray;        /* 0x2C: offset to user channel flag array (unsigned char*) relative to this address - numUserChannels values */ 
	uint32_t    offsetCustomData;                   /* 0x30: offset to EdgeAnimCustomDataTable (or raw data from Edge tool) */
	uint32_t    pad0[3];                            /* 0x34: pad */
	uint16_t    simdHierarchy[0];                   /* 0x40: simd hierarchy - TODO: RENAME*/

    // more data after this point..
} EdgeAnimSkeleton;

// Animation header - followed by animation data 
 
typedef struct EDGE_ALIGNED_PS3(16) EdgeAnimAnimation   // Must be aligned to 16 bytes
{
	uint32_t	tag;								/* 0x00: version tag - see edgeAnimGetAnimTag() */
	float		duration;							/* 0x04: duration of animation */
	float		sampleFrequency;					/* 0x08: sampling frequency */
	uint16_t	sizeHeader;							/* 0x0C: size (in bytes, aligned to 16) of the header - TODO: explain structure generated by tools */
	uint16_t	numJoints;							/* 0x0E: number of joints (redundant) */
	uint16_t	numFrames;							/* 0x10: total number of frames */
	uint16_t	numFrameSets;						/* 0x12: number of frame sets */
	uint16_t	evalBufferSizeRequired;				/* 0x14: size of evaluation buffer required for this anim */
	uint16_t	numConstRChannels;					/* 0x16: number of constant rotation channels */
	uint16_t	numConstTChannels;					/* 0x18: number of constant translation channels */
	uint16_t	numConstSChannels;					/* 0x1A: number of constant scale channels */	
	uint16_t	numConstUserChannels;				/* 0x1C: number of constant user channels */
	uint16_t	numAnimRChannels;					/* 0x1E: number of animated rotation channels */
	uint16_t	numAnimTChannels;					/* 0x20: number of animated translation channels */
	uint16_t	numAnimSChannels;					/* 0x22: number of animated scale channels */
	uint16_t	numAnimUserChannels;				/* 0x24: number of animated user channels */
	uint16_t	flags;								/* 0x26: flags */
	uint32_t	sizeJointsWeightArray;				/* 0x28: if size set to 0, it means no array */
#if defined(WIN64) || defined(_M_X64)
	uint64_t	eaUserJointWeightArray;				/* 0x2C: user override for joint weights array - set to NULL by tools*/ 
#else
	uint32_t	eaUserJointWeightArray;				/* 0x2C: user override for joint weights array - set to NULL by tools*/ 
	uint32_t	pad1;								/* 0x30: pad */
#endif
	uint32_t	offsetJointsWeightArray;			/* 0x34: offset of joint weights array - relative to start of header - only if userJointWeightArray is NULL */
	uint32_t	offsetFrameSetDmaArray;				/* 0x38: offset of the frameset dma array (non relocated EdgeDmaListElement*) relative to this address */
	uint32_t	offsetFrameSetInfoArray;			/* 0x3C: offset of the frameset info array (EdgeAnimFrameSetInfo*) relative to this address */
	uint32_t	offsetConstRData;					/* 0x40: offset to constant rotation data relative to this address */
	uint32_t	offsetConstTData;					/* 0x44: offset to constant translation data relative to this address */
	uint32_t	offsetConstSData;					/* 0x48: offset to constant scale data relative to this address */
	uint32_t	offsetConstUserData;				/* 0x4C: offset to constant user channel data relative to this address */
	uint32_t	offsetPackingSpecs;					/* 0x50: offset to packing specs relative to this address (NULL if no bitpacking) */
	uint32_t	offsetCustomData;					/* 0x54: offset to EdgeAnimCustomDataTable (or raw data from Edge tool) */
	uint32_t	sizeCustomData;						/* 0x58: size of custom data, including table - in bytes, aligned 16 */
	uint32_t	offsetLocomotionDelta;				/* 0x5C: offset to locomotion delta (Quat4-Trans4) - NULL if no delta is included */
	uint16_t	channelTables[0];					/* 0x60: channel tables */

	// more data after this point..
} EdgeAnimAnimation;

// Custom Data Hash Table

typedef struct EDGE_ALIGNED(16) EdgeAnimCustomDataTable	// Must be aligned to 16 bytes
{
	uint32_t	numEntries;							/* 0x00: Number of entries in table */
	uint32_t	offsetHashArray;					/* 0x04: Offset to hash names of custom data (uint32_t*) */
	uint32_t	offsetEntrySizes;					/* 0x08: Offset to sizes of custom data entries (uint32_t*) */
	uint32_t	offsetEntries;						/* 0x0C: Offset to custom data entry offset (uint32_t*) */
} EdgeAnimCustomDataTable;

// Headers

typedef struct EDGE_ALIGNED(4) EdgeAnimFrameSetInfo
{
	uint16_t	baseFrame;
	uint16_t	numIntraFrames;
} EdgeAnimFrameSetInfo;  

// Mirroring

struct EdgeAnimMirrorPair
{
	inline EdgeAnimMirrorPair() {}
	inline EdgeAnimMirrorPair(uint16_t _idx0, uint16_t _idx1, uint32_t _spec) : idx0(_idx0), idx1(_idx1), spec(_spec) {}

	uint16_t idx0;			/* index of first joint */
	uint16_t idx1;			/* index of second joint */
	uint32_t spec;			/* mirroring spec */
};

enum EdgeAnimMirrorSpecComponent
{
	EDGE_ANIM_MIRROR_SPEC_SOURCE_X = 0,
	EDGE_ANIM_MIRROR_SPEC_SOURCE_Y = 1,
	EDGE_ANIM_MIRROR_SPEC_SOURCE_Z = 2,
	EDGE_ANIM_MIRROR_SPEC_SOURCE_W = 3,
	EDGE_ANIM_MIRROR_SPEC_SOURCE_NEG_X = 8 | 0,
	EDGE_ANIM_MIRROR_SPEC_SOURCE_NEG_Y = 8 | 1,
	EDGE_ANIM_MIRROR_SPEC_SOURCE_NEG_Z = 8 | 2,
	EDGE_ANIM_MIRROR_SPEC_SOURCE_NEG_W = 8 | 3,
};

// Creates a mirror spec describing the source component for each of the target components
inline uint32_t edgeAnimCreateMirrorPairSpec( EdgeAnimMirrorSpecComponent targetRotX, EdgeAnimMirrorSpecComponent targetRotY,
											  EdgeAnimMirrorSpecComponent targetRotZ, EdgeAnimMirrorSpecComponent targetRotW,
											  EdgeAnimMirrorSpecComponent targetTransX, EdgeAnimMirrorSpecComponent targetTransY,
											  EdgeAnimMirrorSpecComponent targetTransZ )
{
	uint32_t spec = 0;
	spec  = ( targetRotX << 28 ) | ( targetTransX << 24 );
	spec |= ( targetRotY << 20 ) | ( targetTransY << 16 );
	spec |= ( targetRotZ << 12 ) | ( targetTransZ <<  8 );
	spec |= ( targetRotW <<  4 ) | ( 3 );
	return spec;
}

// Blend tree

typedef enum EdgeAnimBlendOp
{
	EDGE_ANIM_BLENDOP_BLEND_LINEAR,
	EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_RIGHT,
	EDGE_ANIM_BLENDOP_BLEND_ADD_DELTA_LEFT,
	EDGE_ANIM_BLENDOP_COMPOSE_ADD_RIGHT,
	EDGE_ANIM_BLENDOP_COMPOSE_ADD_LEFT,
	EDGE_ANIM_BLENDOP_COMPOSE_SUB_RIGHT_FROM_LEFT,
	EDGE_ANIM_BLENDOP_COMPOSE_SUB_LEFT_FROM_RIGHT
} EdgeAnimBlendOp;

typedef struct EDGE_ALIGNED(4) EdgeAnimBlendBranch
{
	inline  EdgeAnimBlendBranch() {}
	inline  EdgeAnimBlendBranch(EdgeAnimBlendOp blendOp, int16_t leftIndex, int16_t rightIndex, float blendFactor, uint16_t _flags = 0, uint32_t user = 0)
	: operation(static_cast<uint16_t>(blendOp)),
	  left(leftIndex),
	  right(rightIndex),
	  flags(_flags),
	  alpha(blendFactor),
	  userVal(user)
	{
		// validate flags
		EDGE_ASSERT((_flags & (~EDGE_ANIM_FLAG_MIRROR)) == 0);
	}

	uint16_t	operation;							/* blending operation see EdgeAnimBlendOp*/
	uint16_t	left;								/* either EDGE_ANIM_BLEND_TREE_INDEX_LEAF (leaf index) or EDGE_ANIM_BLEND_TREE_INDEX_BRANCH (branch index) set */
	uint16_t	right;								/* either EDGE_ANIM_BLEND_TREE_INDEX_LEAF (leaf index) or EDGE_ANIM_BLEND_TREE_INDEX_BRANCH (branch index) set */
	uint16_t	flags;								/* branch flags */
	float		alpha;								/* blend factor */
	uint32_t	userVal;							/* user specified value */
} EdgeAnimBlendBranch;

typedef struct EDGE_ALIGNED(4) EdgeAnimBlendLeaf
{
	inline EdgeAnimBlendLeaf() {}
	inline EdgeAnimBlendLeaf(EdgeAnimAnimation* animation, uint16_t animationSize, float time, uint16_t _flags = 0, uint32_t user = 0)
	: animationHeaderEa(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(animation))),
	  animationHeaderSize(animationSize),
	  flags(_flags),
	  evalTime(time),
	  userVal(user)
	{
		// validate flags
		EDGE_ASSERT((_flags & (~EDGE_ANIM_FLAG_MIRROR)) == 0);
	}
	inline EdgeAnimBlendLeaf(void* pose, uint16_t _flags, uint32_t user = 0)
	: poseAddr(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pose))),
	  animationHeaderSize(0),
	  flags(_flags),
	  evalTime(0),
	  userVal(user)
	{
		// validate flags
		#ifndef NDEBUG
		// one and only one of the pose flags (main or local) must be set
		uint32_t poseFlags = _flags & (EDGE_ANIM_FLAG_POSE_FROM_MAIN | EDGE_ANIM_FLAG_POSE_FROM_LOCAL);
		EDGE_ASSERT((poseFlags == EDGE_ANIM_FLAG_POSE_FROM_MAIN) || (poseFlags == EDGE_ANIM_FLAG_POSE_FROM_LOCAL));
		#endif
	}

	union
	{
		uintptr_t		animationHeaderEa;					/* effective address of animation Header */
		uintptr_t		poseAddr;							/* address of precomputed pose (in main or local mem, depending on flags) */
	};
	uint16_t			animationHeaderSize;				/* size animation header */
	uint16_t			flags;								/* leaf flags */
	float				evalTime;							/* evaluation time */
	uint32_t			userVal;							/* user specified value */
} EdgeAnimBlendLeaf;

// Pose stack / cache

typedef struct EDGE_ALIGNED_PS3(16) EdgeAnimPoseInfo
{
	struct EdgeAnimJointTransform*	jointArray;				/* pointer to the joint array */
	uint8_t*						weightArray;			/* pointer to the joint weights array */
	float*							userChannelArray;		/* pointer to the user channel array */
	uint8_t*						userChannelWeightArray;	/* pointer to the user channel weights array */
	uint32_t*						flags;					/* pointer to flags */
	uint8_t							pad[12];				/* padding */
} EdgeAnimPoseInfo;

// Command list (internal)

enum EdgeAnimCommandOp
{
	EDGE_ANIM_CMD_END_LIST = 0,						/* end of command list marker */
	EDGE_ANIM_CMD_EVAL,								/* evaluate anim to top of pose stack */
	EDGE_ANIM_CMD_PUSH_AND_EVAL,					/* allocate a temporary pose and evaluate */
	EDGE_ANIM_CMD_BLEND_AND_POP,					/* blend and pop */
	EDGE_ANIM_CMD_MIRROR,							/* mirror pose at top of pose stack */
	EDGE_ANIM_CMD_PUSH_POSE,						/* push precomputed pose onto stack */
	EDGE_ANIM_CMD_USER_COMMAND_START				/* user command op-codes should begin at this value */
};

struct EdgeAnimCommand
{
	uint16_t						command;		/* see EdgeAnimCommandOp */
	uint16_t						arg0;			/* helper argument used by some commands */
	union
	{
		const EdgeAnimBlendLeaf*	leaf;			/* if it's an evaluation command (EDGE_ANIM_CMD_PUSH_AND_EVAL) */
		const EdgeAnimBlendBranch*	branch;			/* if it's a blend command (EDGE_ANIM_CMD_BLEND_AND_POP) */
		uint32_t					user;			/* if it's a user command */
	};
};

// Warnings

#ifdef  _MSC_VER
#pragma warning(pop)
#endif

// PA bookmarks

enum
{
	EDGE_BOOKMARK_ANIM_BASE = 0xEDA0,
	EDGE_BOOKMARK_ANIM_INITIALIZE,
	EDGE_BOOKMARK_ANIM_PROCESS_BLEND_TREE,
	EDGE_BOOKMARK_ANIM_PROCESS_COMMAND_LIST,
	EDGE_BOOKMARK_ANIM_COPY_BASE_POSE,
	EDGE_BOOKMARK_ANIM_EVALUATE,
	EDGE_BOOKMARK_ANIM_BLEND_JOINTS_SLERP,
	EDGE_BOOKMARK_ANIM_BLEND_JOINTS_ADDITIVE,
	EDGE_BOOKMARK_ANIM_LOCAL_JOINTS_TO_WORLD_JOINTS,
	EDGE_BOOKMARK_ANIM_WORLD_JOINTS_TO_LOCAL_JOINTS,
	EDGE_BOOKMARK_ANIM_JOINTS_TO_MATRICES_3X4,
	EDGE_BOOKMARK_ANIM_MATRICES_3X4_TO_JOINTS,
	EDGE_BOOKMARK_ANIM_JOINTS_TO_MATRICES_4X4,
	EDGE_BOOKMARK_ANIM_MATRICES_4X4_TO_JOINTS,
	EDGE_BOOKMARK_ANIM_EVAL_DECOMPRESS_TABLES,
	EDGE_BOOKMARK_ANIM_EVAL_CONSTANT_R,
	EDGE_BOOKMARK_ANIM_EVAL_CONSTANT_T,
	EDGE_BOOKMARK_ANIM_EVAL_CONSTANT_S,
	EDGE_BOOKMARK_ANIM_EVAL_CONSTANT_USER,
	EDGE_BOOKMARK_ANIM_EVAL_CONSTANT_R_BITPACKED,
	EDGE_BOOKMARK_ANIM_EVAL_CONSTANT_T_BITPACKED,
	EDGE_BOOKMARK_ANIM_EVAL_CONSTANT_U_BITPACKED,
	EDGE_BOOKMARK_ANIM_EVAL_ANIMATED_R,
	EDGE_BOOKMARK_ANIM_EVAL_ANIMATED_T,
	EDGE_BOOKMARK_ANIM_EVAL_ANIMATED_S,
	EDGE_BOOKMARK_ANIM_EVAL_ANIMATED_USER,
	EDGE_BOOKMARK_ANIM_EVAL_ANIMATED_R_BITPACKED,
	EDGE_BOOKMARK_ANIM_EVAL_ANIMATED_T_BITPACKED,
	EDGE_BOOKMARK_ANIM_EVAL_ANIMATED_U_BITPACKED,
	EDGE_BOOKMARK_ANIM_BLEND_USER_LINEAR,
	EDGE_BOOKMARK_ANIM_BLEND_USER_ADDITIVE,
	EDGE_BOOKMARK_ANIM_LOCAL_JOINTS_TO_WORLD_MATRICES_3X4,
	EDGE_BOOKMARK_ANIM_LOCAL_JOINTS_TO_WORLD_MATRICES_4X4,
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __EDGE_ANIM_STRUCTS_H__

