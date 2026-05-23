/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ANIM_COMMON_H__
#define __EDGE_ANIM_COMMON_H__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression constant
#endif

inline unsigned int edgeAnimGetAnimTag()
{
	return(('E' << 24) | ('A' << 16) | ('0' << 8) | ('5' << 0) );
}

inline unsigned int edgeAnimGetSkelTag()
{
	return(('E' << 24) | ('S' << 16) | ('0' << 8) | ('3' << 0) );
}

inline unsigned int edgeAnimGenerateNameHash(const char* name)
{
	unsigned int crcValue = ~0UL;

	if (name) {
		while(*name) {
			unsigned int tableTemp = (unsigned int)((crcValue & 0xff) ^ (unsigned char)*name);
	
			for (int bitLoop = 0; bitLoop < 8; bitLoop++) {
				if (tableTemp & 0x01)
					tableTemp = (tableTemp >> 1) ^ 0xedb88320;
				else
					tableTemp = (tableTemp >> 1);
			}

			crcValue = (crcValue >> 8) ^ tableTemp;
			name++;
		}
	}

	return crcValue;
}

inline void _edgeAnimGetPoseDataSizes(uint32_t numJoints, uint32_t numUserChannels, uint32_t& sizePose, uint32_t& sizePoseFlags, uint32_t& sizePoseJoints, 
							   uint32_t& sizePoseUserChannels, uint32_t& sizePoseJointWeights, uint32_t& sizePoseUserChannelWeights)
{
	// all our loops process 4 joints at a time; we must have extra space 
	// allocated in our poses...
	// for each "pose slot" we have :
	// - joints (numJoints+1 aligned to 4 * sizeof(EdgeAnimJoints) bytes)
	// - user channels (numUserChannels+1 aligned to 4 * sizeof(float) bytes)
	// - flags, used to know whether weights are valid or not (16 bytes) 
	// - weights (numJoints aligned to 16 bytes)
	// NOTE: additional entry in joint/user channel buffers is used by evaluator for dummy indexed stores	 
	uint32_t numJointsPlusOneAligned = (uint32_t) EDGE_ALIGN(numJoints+1, 4U);
	uint32_t numUserChannelsPlusOneAligned = (uint32_t) EDGE_ALIGN(numUserChannels+1, 4U);
	sizePoseJoints = (uint32_t) EDGE_ALIGN(numJointsPlusOneAligned * 0x30, 16U);					// 0x30 = sizeof(EdgeAnimJointTransform). This file gets included in tools.
	sizePoseUserChannels = (uint32_t) EDGE_ALIGN(numUserChannelsPlusOneAligned * sizeof(float), 16U);
	sizePoseJointWeights = (uint32_t) EDGE_ALIGN(numJoints * sizeof(uint8_t), 4U);
	sizePoseUserChannelWeights = (uint32_t) EDGE_ALIGN(numUserChannels * sizeof(uint8_t), 4U);
	uint32_t sizePoseAllWeights = (uint32_t) EDGE_ALIGN(sizePoseJointWeights + sizePoseUserChannelWeights, 16U);
	sizePoseFlags = 16U;
	sizePose = sizePoseJoints + sizePoseUserChannels + sizePoseAllWeights + sizePoseFlags;
}

inline uint32_t edgeAnimGetPoseSize(uint32_t numJoints, uint32_t numUserChannels)
{
	uint32_t sizePose, sizePoseFlags, sizePoseJoints, sizePoseUserChannels, sizePoseJointWeights, sizePoseUserChannelWeights;
	_edgeAnimGetPoseDataSizes(numJoints, numUserChannels, sizePose, sizePoseFlags, sizePoseJoints, sizePoseUserChannels, sizePoseJointWeights, sizePoseUserChannelWeights);

	return sizePose;
}

inline int edgeAnimSkeletonGetJointIndexByHash(const EdgeAnimSkeleton* skeleton, unsigned int jointHash)
{
    /* Validation */
    EDGE_ASSERT(skeleton);
    EDGE_ASSERT(EDGE_IS_ALIGNED(skeleton, 16U));
    EDGE_ASSERT(skeleton->tag == edgeAnimGetSkelTag());

    const unsigned int* pJointHashArray = EDGE_OFFSET_GET_POINTER(const unsigned int, skeleton->offsetJointNameHashArray);
    for(unsigned int jointIndex = 0; jointIndex < skeleton->numJoints; jointIndex++) {
        if (jointHash == pJointHashArray[ jointIndex ]) {
            return jointIndex;
        }
    }
    
    return -1;
}

inline int edgeAnimSkeletonGetJointIndexByName(const EdgeAnimSkeleton* skeleton, const char* jointName)
{
    /* Validation */
    EDGE_ASSERT(skeleton);
    EDGE_ASSERT(EDGE_IS_ALIGNED(skeleton, 16U));
    EDGE_ASSERT(skeleton->tag == edgeAnimGetSkelTag());
    EDGE_ASSERT(jointName);

    /* Call the primary lookup function to get joint index using a hash */
    return edgeAnimSkeletonGetJointIndexByHash(skeleton, edgeAnimGenerateNameHash(jointName));
}

inline int edgeAnimSkeletonGetUserChannelIndexByHash(const EdgeAnimSkeleton* skeleton, unsigned int userChannelNodeNameHash, unsigned int userChannelNameHash)
{
    /* Validation */
    EDGE_ASSERT(skeleton);
    EDGE_ASSERT(EDGE_IS_ALIGNED(skeleton, 16U));
    EDGE_ASSERT(skeleton->tag == edgeAnimGetSkelTag());

    const unsigned int* pUserChannelNodeNameHashArray = EDGE_OFFSET_GET_POINTER(const unsigned int, skeleton->offsetUserChannelNodeNameHashArray);
    const unsigned int* pUserChannelNameHashArray = EDGE_OFFSET_GET_POINTER(const unsigned int, skeleton->offsetUserChannelNameHashArray);
    for(unsigned int userChannelIndex = 0; userChannelIndex < skeleton->numUserChannels; userChannelIndex++) {
        if (userChannelNodeNameHash == pUserChannelNodeNameHashArray[ userChannelIndex ] &&
			userChannelNameHash == pUserChannelNameHashArray[ userChannelIndex ]) {
            return userChannelIndex;
        }
    }
    
    return -1;
}

inline int edgeAnimSkeletonGetUserChannelIndexByName(const EdgeAnimSkeleton* skeleton, const char* userChannelNodeName, const char* userChannelName)
{
    /* Validation */
    EDGE_ASSERT(skeleton);
    EDGE_ASSERT(EDGE_IS_ALIGNED(skeleton, 16U));
    EDGE_ASSERT(skeleton->tag == edgeAnimGetSkelTag());
    EDGE_ASSERT(userChannelName);

    /* Call the primary lookup function to get joint index using a hash */
    return edgeAnimSkeletonGetUserChannelIndexByHash(skeleton, edgeAnimGenerateNameHash(userChannelNodeName), edgeAnimGenerateNameHash(userChannelName));
}

inline void* edgeAnimCustomDataChunk( const EdgeAnimCustomDataTable *pCustomDataTable, uint32_t chunkHash )
{
	/* Validation */
	EDGE_ASSERT( pCustomDataTable );

	const uint32_t* pCustomDataHash = EDGE_OFFSET_GET_POINTER( const uint32_t, pCustomDataTable->offsetHashArray );
	const uint32_t* pCustomDataEntry = EDGE_OFFSET_GET_POINTER( const uint32_t, pCustomDataTable->offsetEntries );

	for( uint32_t tableIndex = 0; tableIndex < pCustomDataTable->numEntries; tableIndex++, pCustomDataHash++, pCustomDataEntry++ )
	{
		if( *pCustomDataHash == chunkHash )
		{
			return EDGE_OFFSET_GET_POINTER( void, *pCustomDataEntry );
		}
	}
	return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __EDGE_ANIM_COMMON_H__

