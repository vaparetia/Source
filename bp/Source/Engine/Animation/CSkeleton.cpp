//----------------------------------------------------------------------------
// CSkeleton.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSkeleton.h"
#include "CAnimationSystem.h"

//----------------------------------------------------------------------------

#include "Engine/Resource/CResourceFactory.h"
#include "edge/anim/edgeanim_structs.h"
#include "../../ExtLibraries/Edge/Branch/target/common/include/edge/anim/edgeanim_common.h"

namespace
{
   struct SSkeletonCustomHeader
   {
      uint32   mVersion;
      uint32   mJointPathNamesOffset;
      uint32   mJointPathNamesCount;
   };
}

//----------------------------------------------------------------------------

CSkeleton::CSkeleton(void const * const pData, int const size)
{
   // Copy in-place data.
   // Later we could perhaps avoid the memory completely.
   mpData = (EdgeAnimSkeleton*)BPE_MALLOC_ALIGNED(128, size);
   memcpy(mpData, pData, size);
   mpPoseBSInv = new CAnimationPoseTransforms(this);
   CAnimationSystem::CalculateInvBindSpaceTransforms(this, mpPoseBSInv);
}

//----------------------------------------------------------------------------

CSkeleton::~CSkeleton()
{
   EdgeAnimSkeleton* pSkeleton = (EdgeAnimSkeleton*)mpData;
   BPE_FREE_ALIGNED(pSkeleton);
   delete mpPoseBSInv;
}

//----------------------------------------------------------------------------

void CSkeleton::FSkeletonFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );

   returnResource.mpResource = new CSkeleton(buildData.mpMemory, buildData.mSize);
}

//----------------------------------------------------------------------------

uint32 const CSkeleton::GetJointCount() const
{
   EdgeAnimSkeleton const * pSkeleton = (EdgeAnimSkeleton const *)mpData;
   return pSkeleton->numJoints;
}

//----------------------------------------------------------------------------

int32 const CSkeleton::GetJointParentIndex(int const jointIndex) const
{
   EdgeAnimSkeleton const * pSkeleton = (EdgeAnimSkeleton const *)mpData;

   int16* parentIndices = EDGE_OFFSET_GET_POINTER(int16, pSkeleton->offsetParentIndicesArray);
   return (int32)parentIndices[jointIndex];
}

//----------------------------------------------------------------------------

int32 const CSkeleton::GetJointIndex(std::string const & jointName) const
{
   return GetJointIndex(jointName.c_str());
}

//----------------------------------------------------------------------------

int32 const CSkeleton::GetJointIndex(const char * const jointName) const
{
   EdgeAnimSkeleton const * pSkeleton = (EdgeAnimSkeleton const *)mpData;

   uint32 const jointNameHash = edgeAnimGenerateNameHash(jointName);

   // locate joint index based on hash for passed in name
   int32 jointIndex = edgeAnimSkeletonGetJointIndexByHash(pSkeleton, jointNameHash);

   // if we didn't find the joint we do a slow search based on the path.
   if( jointIndex == -1 )
   {
      SSkeletonCustomHeader const * pCustomData = EDGE_OFFSET_GET_POINTER(SSkeletonCustomHeader, pSkeleton->offsetCustomData);
      uint32 const * pJointPathOffsets = EDGE_OFFSET_GET_POINTER(uint32, pCustomData->mJointPathNamesOffset);
    
      for( int i = 0; i < pCustomData->mJointPathNamesCount; ++i )
      {
         char const * const pJointPathName = EDGE_OFFSET_GET_POINTER(char, pJointPathOffsets[i]);
         
         uint32 const jointPathHash = edgeAnimGenerateNameHash(pJointPathName);
         
         if( jointNameHash == jointPathHash )
         {
            // find last instance of '|' in path string.
            char const * pJointName = strrchr(pJointPathName, '|');
            
            // We're interested in the actual name which starts after the last '|'
            if( pJointName )
            {
               pJointName += 1;
            }

            bpe_debugger_and_console_printf("Performance Warning: Joint index lookup used slow method, search for %s instead of %s\n", pJointName, jointName);

            jointIndex = i;
            break;
         }
      }

      // If we still weren't able to locate the joint, we print out ALL joints at this point.
      if( jointIndex == -1 )
      {
         bpe_debugger_and_console_printf("ERROR: Joint '%s' not found.\n", jointName);
         bpe_debugger_printf("List of all joints:\n");

         for( int i = 0; i < pCustomData->mJointPathNamesCount; ++i )
         {
            char const * const pJointPathName = EDGE_OFFSET_GET_POINTER(char, pJointPathOffsets[i]);
            char const * pJointName = strrchr(pJointPathName, '|');
            if( pJointName != NULL )
            {
               bpe_debugger_printf("   %s (%s)\n", pJointName + 1, pJointPathName);
            }
         }
      }
   }

   return jointIndex;
}

//----------------------------------------------------------------------------

char const * const CSkeleton::GetJointPath(int32 const jointIndex) const
{
   EdgeAnimSkeleton const * pSkeleton = (EdgeAnimSkeleton const *)mpData;
   if( jointIndex < 0 || jointIndex > pSkeleton->numJoints )
   {
      return NULL;
   }

   SSkeletonCustomHeader const * pCustomData = EDGE_OFFSET_GET_POINTER(SSkeletonCustomHeader, pSkeleton->offsetCustomData);
   uint32 const * pJointPathOffsets = EDGE_OFFSET_GET_POINTER(uint32, pCustomData->mJointPathNamesOffset);
   return EDGE_OFFSET_GET_POINTER(char, pJointPathOffsets[jointIndex]);
}

//----------------------------------------------------------------------------

char const * const CSkeleton::GetJointName(int32 const jointIndex) const
{
   char const * const pJointPath = GetJointPath(jointIndex);
   
   if( pJointPath )
   {
      char const * pJointName = strrchr(pJointPath, '|');
      if( pJointName )
      {
         return pJointName + 1;
      }
   }

   return NULL;
}

