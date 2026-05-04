//----------------------------------------------------------------------------
// NRenderQueue.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once
#ifdef SN_TARGET_PS3
#define PARTICLE_SPU
#define ALIGN(n)	__attribute__((aligned(n)))
#else
#define ALIGN(n)
#endif


//----------------------------------------------------------------------------

namespace NRenderQueue
{
   struct SRenderCmd
   {
      enum ECmd
      {
         kCmd_SetShader,
         kCmd_FreeParameterBufferRegion,
         kCmd_Memcpy,
         kCmd_Terminate
      };
	  
      uint32   mCmd;
      
      union
      {
         struct
         {
            uint32   mShaderFragmentDataEA;
            uint32   mShaderVertexDataEA;
            uint32   mMaterialConstantParametersEA;
            uint32   mRuntimeSharedParametersEA;
            uint32   mRuntimeUnsharedParametersEA;
            
            uint32   mOutFragmentProgramEA;
            uint32   mOutFragmentProgramOffset;
            uint32   mOutCommandBufferHoleEA;
            uint32   mJumpToNextOffset;

            uint16   mShaderFragmentDataSize;
            uint16   mShaderVertexDataSize;
            uint16   mCommandHoleSize;

            uint16   mMaterialConstantParametersSize;
            uint16   mRuntimeSharedParametersSize;
            uint16   mRuntimeUnsharedParametersSize;
         } 
         mSetShaderParams;

         struct 
         {
            uint32   mFreePtrEA;
            uint32   mFreePtrValue;
         } mFreeParameterBufferRegionParams;
      };
   } ALIGN(16);

   enum EProcessResult
   {
      kPR_Ok,
      kPR_Terminate
   };

   EProcessResult ProcessCommand(SRenderCmd const * pCommand, uint8* pScratchMemory);
}
