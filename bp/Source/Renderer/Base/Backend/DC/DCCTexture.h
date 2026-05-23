//----------------------------------------------------------------------------
// DCCTexture.h
// Dreamcast render backend — texture stub
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"

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

   void* GetPVRHandle() const { return mpPVRData; }

private:
   explicit CTexture(EFormat format, EType type, EUsage usage);

   void FreeMemory();

   //IMPORTANT: Look at CloneTexture when adding/removing/modifying variables
   void*    mpPVRData;          // PVRTC texture data in VRAM
   uint32   mMemorySize;
   uint16   mWidth;
   uint16   mHeight;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CTexture);
};

//----------------------------------------------------------------------------
