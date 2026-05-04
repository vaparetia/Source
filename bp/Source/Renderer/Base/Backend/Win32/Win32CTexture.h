//----------------------------------------------------------------------------
// Win32CTexture.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CTexture.h"

#include "d3d9.h"
#include	"D3dx9tex.h"
#include "Engine/Mechanics/TComPtr.h"

//----------------------------------------------------------------------------

class RENDERER_API CTexture : public CBaseTexture
{
   friend class CBaseTexture;
public:
   virtual                                ~CTexture();

   virtual void                           SetTexture(int const textureStage) const;
   virtual void                           Lock(void** pMemory, int * pPitch);
   virtual void                           Unlock();

   static D3DFORMAT                       GetD3DFormat(EFormat format);

   TComPtr<IDirect3DBaseTexture9> const & GetTexture() const { return mTexture; }
   TComPtr<IDirect3DBaseTexture9> &       Texture() { return mTexture; }
   
   TComPtr<IDirect3DSurface9> &           Surface() { return mSurface; }

protected:
   explicit                               CTexture(EFormat format, EType type, EUsage usage, int width, int height, int depth, int mipCount);

private:
   //IMPORTANT: Look at CloneTexture when adding/removing/modifying variables
   TComPtr<IDirect3DSurface9>             mSurface;
   TComPtr<IDirect3DBaseTexture9>         mTexture;
   TComPtr<IDirect3DSurface9>             mLockedSurface;
   int                                    mWidth;
   int                                    mHeight;
   int                                    mDepth;
   int                                    mMipCount;
};

//----------------------------------------------------------------------------


