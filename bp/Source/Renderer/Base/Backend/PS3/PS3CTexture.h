//----------------------------------------------------------------------------
// PS3CTexture.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"

//----------------------------------------------------------------------------

class RENDERER_API CTexture : public CBaseTexture
{
   friend class CBaseTexture;
   friend class CCompiledShader;
public:
   virtual ~CTexture();

   virtual void SetTexture(int const textureStage) const;

   virtual void Lock(void** pMemory, int * pPitch);
   virtual void Unlock();

   void *       GcmGetAddress() const { return mMemory->mpAddress; }
   uint32 const GcmGetTextureFormat() const { mTexture.format; }
   uint32 const GcmGetSurfaceFormat() const;
   uint32 const GcmGetDepthSurfaceFormat() const;
   uint32 const GcmGetLocation() const { return mTexture.location; }
   uint32 const GcmGetOffset() const { return mTexture.offset; }
   uint32 const GcmGetPitch() const { return mTexture.pitch; }

private:
   CTexture(EFormat format, EType type, EUsage usage);
   
   void FreeMemory();

public:
   //IMPORTANT: Look at CloneTexture when adding/removing/modifying variables
   CRenderHWAllocator::SHandle const * mMemory;
   uint32                              mMemoryAllocationSize;
   CellGcmTexture                      mTexture;
   uint32                              mIsTiled : 1;
   uint32                              mAAType : bpe::bits_to_store< kAA_LastValid >::value;
   uint32                              mMaxAniso : 3;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CTexture);
};

//----------------------------------------------------------------------------



