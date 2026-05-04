//----------------------------------------------------------------------------
// Win32CTexture.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CTexture.h"

#include "Engine/Mechanics/TComPtr.h"
#include <revolution/gx.h>
#include "..\CRenderHWAllocator.h"

//----------------------------------------------------------------------------

class RENDERER_API CTexture : public CBaseTexture
{
   friend class CBaseTexture;
public:
   virtual                                ~CTexture();

   virtual void SetTexture(int const textureStage) const;
   virtual void Lock(void** pMemory, int * pPitch);
   virtual void Unlock();

protected:
   explicit                               CTexture(EFormat format, EType type, EUsage usage, int width, int height, int depth, CRenderHWAllocator::SHandle const *mpHandle );

   GXTexObj mTexture;
   CRenderHWAllocator::SHandle const *mpHandle;
};

//----------------------------------------------------------------------------


