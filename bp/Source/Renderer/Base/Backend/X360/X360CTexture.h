//----------------------------------------------------------------------------
// Win32CTexture.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CTexture.h"

#include "d3d9.h"
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
   virtual unsigned int                   GetEDRAMEnd() const;
   virtual unsigned int                   GetEDRAMEndHierarchicalZ() const;

   static D3DFORMAT                       GetD3DFormat(EFormat format);
   static D3DFORMAT                       GetD3DFormatLinear(EFormat format);

   IDirect3DBaseTexture9 * const          GetTexture() const { return mTexture; }
   IDirect3DBaseTexture9 *                Texture() { return mTexture; }
   
   TComPtr<IDirect3DSurface9> &           Surface() { return mSurface; }

   void                                   FreeResources();

   static int                             GetDecompressInProgressCount() { return mDecompressInProgress; }

protected:
   explicit                               CTexture(EFormat format, EType type, EUsage usage, int width, int height, int depth, int mipCount, EAntiAliasType aaType, unsigned char* dataMem, int textureDataSize, int textureHeaderSize);
   void                                   PCTDecompress_Thread(uint32 param0, uint32 param1);


private:
   //IMPORTANT: Look at CloneTexture when adding/removing/modifying variables
   unsigned int                           mSurfaceSize;
   unsigned int                           mHierarchicalZSurfaceSize;
   D3DSURFACE_PARAMETERS                  mSurfaceParams;
   TComPtr<IDirect3DSurface9>             mSurface;
   IDirect3DBaseTexture9 *                mTexture;
   unsigned char*                         mpTextureData;
   unsigned int                           mReleaseFlags;//TODO: clean this up
   TComPtr<IDirect3DSurface9>             mLockedSurface;
   int                                    mWidth;
   int                                    mHeight;
   int                                    mDepth;
   int                                    mMipCount;
   EAntiAliasType                         maaType;

   //Used when duplicating
   IDirect3DBaseTexture9 *                mOrigTextureHeader;
   int                                    mTextureDataSize;
   int                                    mTextureHeaderSize;
   // PCT Texture decompression support
   D3DFORMAT                              mOrignalPctFormat;
   static int                             mDecompressInProgress;
};

//----------------------------------------------------------------------------


