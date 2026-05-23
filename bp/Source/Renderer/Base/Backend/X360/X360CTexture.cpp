//----------------------------------------------------------------------------
// X360CTexture.cpp
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/X360/X360CTexture.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CMemoryInputStream.h"
#include "Renderer/Base/Backend/X360/CRenderBackendPrivate.h"
#include "Renderer/Base/Backend/X360/TDelayComPtr.h"
#include "Engine/System/CTaskQueue.h"

extern CTaskQueue* gpTaskQueue_Render;

//#define NO_DEPTH_BUFFER_CREATE_TEXTURE

//----------------------------------------------------------------------------

int CTexture::mDecompressInProgress = 0;

//----------------------------------------------------------------------------

CTexture::CTexture(EFormat format, EType type, EUsage usage, int width, int height, int depth, int mipCount, EAntiAliasType aaType, unsigned char* dataMem, int textureDataSize, int textureHeaderSize)
: CBaseTexture(format, type, usage)
, mWidth(width)
, mHeight(height)
, mDepth( depth )
, mMipCount( mipCount )
, maaType( aaType )
, mTexture(NULL)
, mpTextureData(dataMem)
, mOrigTextureHeader(NULL)
, mTextureDataSize(textureDataSize)
, mTextureHeaderSize(textureHeaderSize)
{
   ZeroMemory(&mSurfaceParams, sizeof(mSurfaceParams));
   mReleaseFlags = 0;//TODO: clean this up
}

//----------------------------------------------------------------------------

CTexture::~CTexture()
{
   FreeResources();
}

//----------------------------------------------------------------------------

void CTexture::FreeResources()
{
   if( mTexture )
   {
      if( mReleaseFlags == 0 )//managed by us not d3d
      {
         if( mTexture )
         {
            delete [] mTexture;
            mTexture = NULL;
         }
         if( mOrigTextureHeader )
         {
            delete [] mOrigTextureHeader;
            mOrigTextureHeader = NULL;
         }
         if( mpTextureData )
         {
            const DWORD dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES( 0, FALSE, FALSE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_4K, XALLOC_MEMPROTECT_WRITECOMBINE, FALSE, XALLOC_MEMTYPE_PHYSICAL );
            XMemFree( mpTextureData, dwAllocAttributes );
            mpTextureData = NULL;
         }
      }
      else
      {
         AddResourceToBeReleased(mTexture);
         mTexture = NULL;
      }
   }
}

void CTexture::PCTDecompress_Thread(uint32 param0, uint32 param1)
{
   unsigned char* pTempMem = (unsigned char*)param0;
   int compressedSize = param1;
   D3DFORMAT orignalPctFormat = mOrignalPctFormat;
   HRESULT hr;

   UINT const dwWidth = GetWidth();
   UINT const dwHeight = GetHeight();
   int const mipmapLevels = GetMipCount();
   D3DFORMAT const d3dFormat = GetD3DFormat(GetFormat());

   // Create linear texture to decompress initial data into, we will later tile this texture and all it's mips into another texture 
   // using CopyTextureSurface
   IDirect3DTexture9* pTempD3DTexture = new IDirect3DTexture9;
   DWORD textureDataSize = XGSetTextureHeader( dwWidth, dwHeight, 1, 
      D3DUSAGE_CPU_CACHED_MEMORY, 
      orignalPctFormat, 
      0, 
      0,
      XGHEADER_CONTIGUOUS_MIP_OFFSET, 
      0,
      pTempD3DTexture, 
      NULL, 
      NULL );
   const DWORD dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES( 0, FALSE, FALSE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_4K, XALLOC_MEMPROTECT_WRITECOMBINE, FALSE, XALLOC_MEMTYPE_PHYSICAL );
   unsigned char* pTextureData = (unsigned char*)XPhysicalAlloc( textureDataSize, MAXULONG_PTR, 0, PAGE_READWRITE );
   if( pTextureData == NULL )
   {
      BPE_VERIFYA(false, "Memory alloc failed for temp texture PTC decompression!")
   }
   XGOffsetResourceAddress( pTempD3DTexture, pTextureData );

   D3DLOCKED_RECT LockedRect;
   pTempD3DTexture->LockRect( 0, &LockedRect, NULL, 0 );
   if( FAILED( hr = XGPTCDecompressSurface( LockedRect.pBits, LockedRect.Pitch,
      dwWidth, dwHeight, orignalPctFormat, NULL,
      pTempMem, compressedSize ) ) )
   {
      BPE_VERIFYA(false, "PTC decompression failed!")
   }
   pTempD3DTexture->UnlockRect( 0 );
   // tile and create mip maps
   IDirect3DTexture9* pD3DTexture = NULL;
   D3DDeviceUncached()->CreateTexture(dwWidth, dwHeight, mipmapLevels, 0, d3dFormat, D3DPOOL_MANAGED, &pD3DTexture, NULL);
   {
      { 
         IDirect3DSurface9* pSrcD3DSurface;
         pTempD3DTexture->GetSurfaceLevel(0, &pSrcD3DSurface);
         IDirect3DSurface9* pD3DSurface;
         pD3DTexture->GetSurfaceLevel(0, &pD3DSurface);
         D3DXLoadSurfaceFromSurface(pD3DSurface, NULL, NULL, pSrcD3DSurface, NULL, NULL, D3DX_DEFAULT, 0);

         pSrcD3DSurface->Release();
         pD3DSurface->Release();
      }
      DWORD filter = D3DX_FILTER_BOX;
      for( int iSurfaceLevel=1; iSurfaceLevel<mipmapLevels; ++iSurfaceLevel )
      {
         IDirect3DSurface9* pSrcD3DSurface;
         pD3DTexture->GetSurfaceLevel(iSurfaceLevel-1, &pSrcD3DSurface);
         IDirect3DSurface9* pD3DSurface;
         pD3DTexture->GetSurfaceLevel(iSurfaceLevel, &pD3DSurface);
         D3DXLoadSurfaceFromSurface(pD3DSurface, NULL, NULL, pSrcD3DSurface, NULL, NULL, filter, 0);

         pSrcD3DSurface->Release();
         pD3DSurface->Release();
      }
   }
   // clean up temp texture memory and d3d resources
   delete pTempD3DTexture;
   XPhysicalFree( pTextureData );
   delete [] pTempMem;
   //
   mTexture = pD3DTexture;

   --mDecompressInProgress;
}

// TODO: once all textures have been recooked remove support for version 0 textures

// Texture version update history:
// Version 1 - Added minRGBA/maxRGBA
//----------------------------------------------------------------------------

static const int kPTCTask_LowWaterMark = 5;

CBaseTexture* CBaseTexture::Create(void* pMemory, uint32 const size)
{
   CMemoryInputStream stream(pMemory, size, CMemoryInputStream::kOwner_App);

   // read in header and check
   uint32 const header = stream.ReadUint32();
   BPE_VERIFY( header == 'TXTR', false, "invalid header" );

   // read in version and check
   uint32 const version = stream.ReadUint32();
   BPE_VERIFY( version == 0 || version == 1 || version == 2 || version == 3, false, "invalid version" );

   EDiscFormat discFormat = kDiscFormat_Raw;
   if( version >= 2 )
   {
      discFormat = static_cast<EDiscFormat>(stream.ReadUint32());
   }
   EFormat const format = static_cast<EFormat>(stream.ReadUint32());
   // read in dimensions
   int const width = stream.ReadUint16();
   int const height = stream.ReadUint16();
   int const depth = stream.ReadUint16();
   int const mipmapLevels = stream.ReadUint8();
   // read alpha flag
   bool const hasAlpha = stream.ReadBool();
   uint32 minRGBA = 0;
   uint32 maxRGBA = 0xffffffff;
   if( version >= 1 )
   {
      minRGBA = stream.ReadUint32();
      maxRGBA = stream.ReadUint32();
   }
   uint32 additionalFlags = 0;
   if( version >= 3 )
   {
      additionalFlags = stream.ReadUint32();
   }

   uint32 const dataheader = stream.ReadUint32();
   BPE_VERIFY( dataheader == 'DATA', false, "invalid header" );
   uint32 const textureType = stream.ReadUint32();

   CTexture * pTexture = NULL;
   if( discFormat == kDiscFormat_Raw )
   {
      uint32 const textureHeaderSize = stream.ReadUint32();
      IDirect3DBaseTexture9 * pD3DTexture = (IDirect3DBaseTexture9 *)new unsigned char[textureHeaderSize];
      stream.Get( pD3DTexture, textureHeaderSize );

      IDirect3DBaseTexture9 * pOrigD3DTexture = (IDirect3DBaseTexture9 *)new unsigned char[textureHeaderSize];
      memcpy( pOrigD3DTexture, pD3DTexture, textureHeaderSize );

      uint32 const textureDataSize = stream.ReadUint32();
      const DWORD dwAllocAttributes = MAKE_XALLOC_ATTRIBUTES( 0, FALSE, FALSE, FALSE, 0, XALLOC_PHYSICAL_ALIGNMENT_4K, XALLOC_MEMPROTECT_WRITECOMBINE, FALSE, XALLOC_MEMTYPE_PHYSICAL );
      unsigned char* pTextureData = ( BYTE* )XMemAlloc( textureDataSize, dwAllocAttributes );
      stream.Get( pTextureData, textureDataSize );

      // Now fix up the texture header to point to the base and mip addresses inside the texture data.
      // This is the final step - once this is done, the texture is ready to be used!
      XGOffsetBaseTextureAddress( pD3DTexture, pTextureData, pTextureData );

      EType const type = textureType == 1 ? kType_Cube : kType_Normal;
      pTexture = new CTexture(format, type, kUsage_Texture, width, height, depth, mipmapLevels, CBaseTexture::kAA_None, pTextureData, textureDataSize, textureHeaderSize);
      pTexture->mTexture = pD3DTexture;

      pTexture->mOrigTextureHeader = pOrigD3DTexture;
   }
   else
   {
      gpTaskQueue_Render->ProcessTasks(kPTCTask_LowWaterMark);

      HRESULT hr = S_OK;

      // load up PTC
      uint32 const compressedSize = stream.ReadUint32();

      unsigned char* pTempMem = new unsigned char[compressedSize];
      BPE_VERIFY(pTempMem != NULL, false, "Memory alloc failed for pTempMem: PTC decompression!")
      stream.Get( pTempMem, compressedSize );

      UINT dwWidth = 0;
      UINT dwHeight = 0;
      D3DFORMAT orignalPctFormat;
      hr = XGGetPTCImageDesc( pTempMem, compressedSize, &dwWidth, &dwHeight, &orignalPctFormat );
      BPE_VERIFY(SUCCEEDED(hr), false, "Failed to get PTC Image Desc!") ;
      
      // create CTexture class around this data
      EType const type = textureType == 1 ? kType_Cube : kType_Normal;
      pTexture = new CTexture(format, type, kUsage_Texture, width, height, depth, mipmapLevels, CBaseTexture::kAA_None, NULL, 0, 0);
      pTexture->mTexture = NULL; // This gets filled in by PCT_Decompress thread functions in gpTaskQueue_Render //pD3DTexture;
      pTexture->mReleaseFlags = 1;
      pTexture->mOrignalPctFormat = orignalPctFormat;

      ++CTexture::mDecompressInProgress;
      CTaskQueue::TCallback callback(pTexture, &CTexture::PCTDecompress_Thread);
      gpTaskQueue_Render->AddTask( callback, (uint32)pTempMem, (uint32)compressedSize );
   }

   pTexture->mHasAlpha = hasAlpha;
   pTexture->mMinRGBA = minRGBA;
   pTexture->mMaxRGBA = maxRGBA;
   pTexture->mAdditionalFlags = additionalFlags;

   return pTexture;
}

//----------------------------------------------------------------------------

VOID WINAPI NormalizationMapFill( D3DXVECTOR4* pOut, 
                                  D3DXVECTOR3 const * pTexCoord, 
                                  D3DXVECTOR3 const * pTexelSize, 
                                  LPVOID pData )
{
   D3DXVECTOR3 texCoord = *pTexCoord;

   D3DXVec3Normalize( &texCoord, &texCoord );
   texCoord /= 2;
   texCoord += D3DXVECTOR3(.5, .5, .5);

   *pOut = D3DXVECTOR4(texCoord.x, texCoord.y, texCoord.z, 1.f);
}

//----------------------------------------------------------------------------

VOID WINAPI ColorFill( D3DXVECTOR4* pOut, 
                       D3DXVECTOR2 const * pTexCoord, 
                       D3DXVECTOR2 const * pTexelSize, 
                       LPVOID pData )
{
   *pOut = *reinterpret_cast<D3DXVECTOR4*>( pData );
}

//----------------------------------------------------------------------------

VOID WINAPI SpecularPowerFill( D3DXVECTOR4* pOut, 
                               D3DXVECTOR2 const * pTexCoord, 
                               D3DXVECTOR2 const * pTexelSize, 
                               LPVOID pData )
{
   *pOut = (D3DXVECTOR4 const&)CBaseTexture::GetSpecularPowerMapSample(CVector2(pTexCoord->x, pTexCoord->y),
                                                                       CVector2(pTexelSize->x, pTexelSize->y),
                                                                       pData);
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::CreateSolid(CColor const & color)
{
   CTexture* pTexture = new CTexture(CTexture::kFormat_A8R8G8B8, kType_Normal, kUsage_Texture, 4, 4, 1, 1, CBaseTexture::kAA_None, NULL, 0, 0);

   LPDIRECT3DTEXTURE9 pD3DTexture = NULL;
   D3DDeviceUncached()->CreateTexture(pTexture->mWidth, pTexture->mHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pD3DTexture, NULL); 

   D3DXVECTOR4 d3dColor(color.GetR() / 255.0f, color.GetG() / 255.0f, color.GetB() / 255.0f, color.GetA() / 255.0f);
   D3DXFillTexture(pD3DTexture, ColorFill, &d3dColor);

   pTexture->mTexture = pD3DTexture;

   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture* CBaseTexture::Create(EGeneratedTextureType const type)
{
   CTexture* pTexture = NULL;
   
   switch( type )
   {
   case kGTT_White:
   case kGTT_Black:
   case kGTT_FlatNormal:
      {
         pTexture = new CTexture(CTexture::kFormat_A8R8G8B8, kType_Normal, kUsage_Texture, 4, 4, 1, 1, CBaseTexture::kAA_None, NULL, 0, 0);

         D3DXVECTOR4 color;
         switch( type ) 
         {
         case kGTT_White:
            color = D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f );
            break;
         case kGTT_Black:
            color = D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );
            break;
         case kGTT_FlatNormal:
            color = D3DXVECTOR4( 0.5f, 0.5f, 1.0f, 0.0f );
            break;
         }

         LPDIRECT3DTEXTURE9 pD3DTexture = NULL;

         D3DDeviceUncached()->CreateTexture(pTexture->mWidth, pTexture->mHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pD3DTexture, NULL); 

         D3DXFillTexture(pD3DTexture, ColorFill, &color);

         pTexture->mTexture = pD3DTexture;
         pTexture->mReleaseFlags = 1;
      }
      break;
   case kGTT_SpecularPower:
      {
         pTexture = new CTexture(CTexture::kFormat_Invalid, kType_Normal, kUsage_Texture, 512, 128, 1, 1, CBaseTexture::kAA_None, NULL, 0, 0);

         LPDIRECT3DTEXTURE9 pD3DTexture = NULL;

         D3DDeviceUncached()->CreateTexture(pTexture->mWidth, pTexture->mHeight, 1, 0, D3DFMT_L16, D3DPOOL_MANAGED, &pD3DTexture, NULL); 

         D3DXFillTexture(pD3DTexture, SpecularPowerFill, NULL);
         pTexture->mTexture = pD3DTexture;
         pTexture->mReleaseFlags = 1;
      }
      break;
   default:
      break;
   }
   
   return pTexture;
}

//----------------------------------------------------------------------------

unsigned int CTexture::GetEDRAMEnd() const
{
   return mSurfaceParams.Base + mSurfaceSize;
}

//----------------------------------------------------------------------------

unsigned int CTexture::GetEDRAMEndHierarchicalZ() const
{
   return mSurfaceParams.HierarchicalZBase + mHierarchicalZSurfaceSize;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::Create(int const width, 
                                    int const height,
                                    int const levels,
                                    EFormat const format,
                                    EUsage const usage,
                                    EAntiAliasType const aaFlag, 
                                    ERenderMemory const memory,
                                    D3DFORMAT const d3dFormat,
                                    D3DMULTISAMPLE_TYPE multisampleType,
                                    bool fillTexture,
                                    SCreateTextureParams const * pCreateTextureParams)
{
   BPE_VERIFY(pCreateTextureParams != NULL, false, "pCreateTextureParams == NULL");

   int const actualRenderTargetWidth = (pCreateTextureParams->mRenderTargetWidth > 0) ? pCreateTextureParams->mRenderTargetWidth : width;
   int const actualRenderTargetHeight = (pCreateTextureParams->mRenderTargetHeight > 0) ? pCreateTextureParams->mRenderTargetHeight : height;

   CTexture* pTexture = new CTexture(format, kType_Normal, usage, width, height, 1, levels, aaFlag, NULL, 0, 0);

   LPDIRECT3DTEXTURE9 pD3DTexture = NULL;
   LPDIRECT3DSURFACE9 pSurface = NULL;

   switch( usage )
   {
   case kUsage_Texture:
   case kUsage_TextureLinear:
      {
         D3DDeviceUncached()->CreateTexture( width, height, levels, 0, d3dFormat, D3DPOOL_MANAGED, &pD3DTexture, NULL );
      }
      break;

   case kUsage_RenderTarget:
      {
         if( pCreateTextureParams->mCreateRenderTargetTexture )
         {
            D3DDeviceUncached()->CreateTexture( width, height, levels, 0, d3dFormat, D3DPOOL_DEFAULT, &pD3DTexture, NULL );
         }
         pTexture->mSurfaceParams.Base = pCreateTextureParams->mBaseAddress;
         pTexture->mSurfaceSize = XGSurfaceSize(actualRenderTargetWidth, actualRenderTargetHeight, d3dFormat, multisampleType);
         D3DDeviceUncached()->CreateRenderTarget(actualRenderTargetWidth, actualRenderTargetHeight, d3dFormat, multisampleType, 0, FALSE, &pSurface, &pTexture->mSurfaceParams);
         pTexture->mSurface.Reset(pSurface);
      }
      break;

   case kUsage_DepthBuffer:
      {
         pTexture->mSurfaceParams.Base = pCreateTextureParams->mBaseAddress;
         pTexture->mSurfaceParams.HierarchicalZBase = pCreateTextureParams->mHierarchicalZBase;
         pTexture->mSurfaceSize = XGSurfaceSize(actualRenderTargetWidth, actualRenderTargetHeight, d3dFormat, multisampleType);
         pTexture->mHierarchicalZSurfaceSize = XGHierarchicalZSize(actualRenderTargetWidth, actualRenderTargetHeight, multisampleType);
#if BPE_INVERT_ZBUFFER
         pTexture->mSurfaceParams.HiZFunc = D3DHIZFUNC_LESS_EQUAL;
#else
         pTexture->mSurfaceParams.HiZFunc = D3DHIZFUNC_GREATER_EQUAL;
#endif
         D3DDeviceUncached()->CreateDepthStencilSurface( actualRenderTargetWidth, actualRenderTargetHeight, d3dFormat, multisampleType, 0, TRUE, &pSurface, &pTexture->mSurfaceParams );
#ifndef NO_DEPTH_BUFFER_CREATE_TEXTURE
         if( pCreateTextureParams->mCreateDepthTargetTexture )
         {
            D3DDeviceUncached()->CreateTexture( width, height, 1, 0, d3dFormat, D3DPOOL_DEFAULT, &pD3DTexture, NULL);
         }
#endif
         pTexture->mSurface.Reset(pSurface);
      }
      break;
   }

   pTexture->mTexture = pD3DTexture;
   pTexture->mReleaseFlags = 1;
   if( pD3DTexture && fillTexture )
   {
      D3DXVECTOR4 color = D3DXVECTOR4( 0, 0, 0, 0 );
      D3DXFillTexture(pD3DTexture, ColorFill, &color);
   }

   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::Create(int const width, 
                                    int const height,
                                    int const levels,
                                    EFormat const format,
                                    EUsage const usage,
                                    EAntiAliasType const aaFlag, 
                                    ERenderMemory const memory,
                                    SCreateTextureParams const * pCreateTextureParams)
{
   SCreateTextureParams tempTextureParams;
   if( pCreateTextureParams == NULL )
   {
      tempTextureParams.mRenderTargetWidth = width;
      tempTextureParams.mRenderTargetHeight = height;
      pCreateTextureParams = &tempTextureParams;
   }
   D3DFORMAT const d3dFormat = (usage == kUsage_TextureLinear) ? CTexture::GetD3DFormatLinear(format) : CTexture::GetD3DFormat(format);
   D3DMULTISAMPLE_TYPE multisampleType;
   switch( aaFlag )
   {
   case kAA_MSAA2x:
      multisampleType = D3DMULTISAMPLE_2_SAMPLES;
      break;
   case kAA_MSAA4x:
      multisampleType = D3DMULTISAMPLE_4_SAMPLES;
      break;
   default:
      multisampleType = D3DMULTISAMPLE_NONE;
      break;
   }

   return Create(width, height, levels, format, usage, aaFlag, memory, d3dFormat, multisampleType, pCreateTextureParams->mClearMemory, pCreateTextureParams);
}

//----------------------------------------------------------------------------

static D3DFORMAT const gD3DFormatMapping[] =
{
   D3DFMT_A8R8G8B8,
   D3DFMT_X8R8G8B8,

   D3DFMT_A16B16G16R16F,
   D3DFMT_R32F,
   
   D3DFMT_D24X8,     
   
   D3DFMT_DXT1,
   D3DFMT_DXT3,
   D3DFMT_DXT5,

   D3DFMT_A32B32G32R32F,

   D3DFMT_L8,

   D3DFMT_D24FS8,
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gD3DFormatMapping) == CTexture::kFormat_Count);

D3DFORMAT CTexture::GetD3DFormat(EFormat format)
{
   return gD3DFormatMapping[format];
}

D3DFORMAT CTexture::GetD3DFormatLinear(EFormat format)
{
   return (D3DFORMAT)MAKELINFMT(gD3DFormatMapping[format]);
}
//----------------------------------------------------------------------------

void CTexture::SetTexture(int const textureStage) const
{
   SetCurrentTextureParams(textureStage);
   GetD3DDevice()->SetTexture(textureStage, mTexture);
}

//----------------------------------------------------------------------------

void CTexture::Lock(void** pMemory, int * pPitch)
{
   BPE_VERIFY( mType == kType_Normal && mDepth == 1, false, "Cannot lock cubemaps or volume textures" );

   HRESULT res = S_OK;

   IDirect3DTexture9* pTexture = static_cast<IDirect3DTexture9*>( mTexture );

   {
      IDirect3DSurface9* pSurface = NULL;
      res = pTexture->GetSurfaceLevel( 0, &pSurface );
      mLockedSurface.Reset(pSurface);
   }

   D3DLOCKED_RECT rect;
   res = mLockedSurface->LockRect(&rect, NULL, D3DLOCK_NOOVERWRITE);

   *pMemory = rect.pBits;
   *pPitch = rect.Pitch;
}

//----------------------------------------------------------------------------

void CTexture::Unlock()
{
   HRESULT res = S_OK;

   IDirect3DTexture9* pTexture = static_cast<IDirect3DTexture9*>( mTexture );
   
   res = mLockedSurface->UnlockRect();
   mLockedSurface.Reset(NULL);
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetWidth() const
{
   return ((CTexture*)this)->mWidth;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetHeight() const
{
   return ((CTexture*)this)->mHeight;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetDepth() const
{
   return ((CTexture*)this)->mDepth;
}

int CBaseTexture::GetMipCount() const
{
   return ((CTexture*)this)->mMipCount;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasWidth(EAntiAliasType const /*type*/) const
{
   return GetWidth();
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasHeight(EAntiAliasType const /*type*/) const
{
   return GetHeight();
}

//----------------------------------------------------------------------------

CBaseTexture::EAntiAliasType const CBaseTexture::GetAntiAliasType() const
{
   return ((CTexture*)this)->maaType;
}

//----------------------------------------------------------------------------

void CBaseTexture::CloneTexture(CBaseTexture* pSourceBaseTexture)
{
   CTexture* pThis = (CTexture*)this;
   CTexture* pSource = (CTexture*)pSourceBaseTexture;

   BPE_VERIFY( pThis->mLockedSurface.IsNull(), false, "CloneTexture::Surface is already locked" );
   BPE_VERIFY( pSource->mDepth == 1 , false, "CloneTexture::Cannot clone Volume textures");
   BPE_VERIFY( pSource->mType == kType_Normal , false, "CloneTexture::Cannot clone cube textures");

   pThis->FreeResources();

   IDirect3DTexture9* pSrcD3DTexture = (IDirect3DTexture9*)pSource->mTexture;
   D3DSURFACE_DESC srcDesc;
   pSrcD3DTexture->GetLevelDesc(0, &srcDesc);
   D3DFORMAT const d3dFormat = srcDesc.Format;

   IDirect3DTexture9* pD3DTexture = NULL;
   D3DDeviceUncached()->CreateTexture(pSource->mWidth, pSource->mHeight, pSource->mMipCount, 0, d3dFormat, D3DPOOL_MANAGED, &pD3DTexture, NULL);
   {
      DWORD filter = D3DX_FILTER_BOX;
      for( int iSurfaceLevel=0; iSurfaceLevel<pSource->mMipCount; ++iSurfaceLevel )
      {
         IDirect3DSurface9* pSrcD3DSurface;
         pSrcD3DTexture->GetSurfaceLevel(iSurfaceLevel, &pSrcD3DSurface);
         IDirect3DSurface9* pD3DSurface;
         pD3DTexture->GetSurfaceLevel(iSurfaceLevel, &pD3DSurface);
         D3DXLoadSurfaceFromSurface(pD3DSurface, NULL, NULL, pSrcD3DSurface, NULL, NULL, filter, 0);

         pSrcD3DSurface->Release();
         pD3DSurface->Release();
      }
   }

   pThis->mReleaseFlags = 1;
   pThis->mpTextureData = NULL;
   pThis->mTexture = pD3DTexture;

   pThis->mSurfaceSize = pSource->mSurfaceSize;
   pThis->mHierarchicalZSurfaceSize = pSource->mHierarchicalZSurfaceSize;
   pThis->mSurfaceParams = pSource->mSurfaceParams;
   pThis->mSurface = pSource->mSurface;
   pThis->mReleaseFlags = pSource->mReleaseFlags;
   pThis->mLockedSurface = pSource->mLockedSurface;
   pThis->mWidth = pSource->mWidth;
   pThis->mHeight = pSource->mHeight;
   pThis->mDepth = pSource->mDepth;
   pThis->mMipCount = pSource->mMipCount;
   pThis->maaType = pSource->maaType;
   pThis->mTextureDataSize = pSource->mTextureDataSize;
   pThis->mTextureHeaderSize = pSource->mTextureHeaderSize;

   // This copies all parameters defined in CBaseTexture
   CloneBaseTexture(pSource);
}
