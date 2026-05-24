//----------------------------------------------------------------------------
// DCCCompiledShader.h
// Dreamcast stub
//----------------------------------------------------------------------------

#pragma once

#include "Engine/Mechanics/IObject.h"

struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

struct SShaderPassData
{
   enum EFlags
   {
      kColorMaskR                   =  (1 << 0),
      kColorMaskG                   =  (1 << 1),
      kColorMaskB                   =  (1 << 2),
      kColorMaskA                   =  (1 << 3),
      kDepthTest                    =  (1 << 4),
      kAlphaToCoverage              =  (1 << 5),
      kWaitForRenderTargetResult    =  (1 << 6),
      kForceDisableAA               =  (1 << 7)
   };

   SShaderPassData()
      :  mData(0)
   {
      mData |= kColorMaskR|kColorMaskG|kColorMaskB|kColorMaskA;
      mData |= kDepthTest;
   }

   int GetFlag(EFlags const flag) const  { return mData & flag; }
   void SetFlag(EFlags const flag, bool const value)
   {
      if (value) mData |=  flag;
      else       mData &= ~flag;
   }

   int mData;
};

class CCompiledShader
{
public:
   int  const Begin()                                    const { return 1; }
   void       End()                                      const {}
   int  const BeginPass(int const /*currentPass*/)       const { return 0; }
   void       EndPass()                                  const {}
   void       BeginShaderBatch(uint8* /*pMatConst*/,     int const /*matConstSize*/,
                                uint8* /*pSharedRT*/,    int const /*sharedRTSize*/,
                                uint8* /*pUnsharedRT*/,  int const /*unsharedRTSize*/) const {}
   void       EndShaderBatch()                           const {}
};

class CCompiledShaderPackage {};
