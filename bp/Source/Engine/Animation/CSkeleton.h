//----------------------------------------------------------------------------
// CSkeleton.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "CAnimationPoseTransforms.h"

//----------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

class CInputStream;
class CAnimationPoseTransforms;

//----------------------------------------------------------------------------

class ENGINE_API CSkeleton
{
public:
   CSkeleton(void const * const pData, int const size);
   virtual ~CSkeleton();

   static void FSkeletonFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

   uint32 const GetJointCount() const;
   int32 const GetJointParentIndex(int const jointIndex) const;
   int32 const GetJointIndex(std::string const & jointName) const;
   int32 const GetJointIndex(const char * const jointName) const;
   char const * const GetJointPath(int32 const jointIndex) const;
   char const * const GetJointName(int32 const jointIndex) const;

public:
   void *                     mpData;        // EdgeAnimSkeleton
   CAnimationPoseTransforms * mpPoseBSInv;   // Inverse bind pose transforms

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CSkeleton);
};
