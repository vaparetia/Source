//----------------------------------------------------------------------------
// Win32CTexture.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/Win32/Win32CTexture.h"

//----------------------------------------------------------------------------

#include "d3dx9.h"

#include "Engine/Streams/CMemoryInputStream.h"
#include "Renderer/Base/Backend/Win32/CRenderBackendPrivate.h"

//#define NO_DEPTH_BUFFER_CREATE_TEXTURE

//----------------------------------------------------------------------------

CTexture::CTexture(EFormat format, EType type, EUsage usage, int width, int height, int depth, int mipCount)
:  CBaseTexture(format, type, usage)
,  mWidth(width)
,  mHeight(height)
,  mDepth( depth )
,  mMipCount( mipCount )
{
}

//----------------------------------------------------------------------------

CTexture::~CTexture()
{
}

//----------------------------------------------------------------------------

CBaseTexture* CBaseTexture::Create(void* pMemory, uint32 const size)
{
   CMemoryInputStream stream(pMemory, size, CMemoryInputStream::kOwner_App);

   // read in header and check
   uint32 const header = stream.ReadUint32();
   BPE_VERIFY( header == 'TXTR', false, "invalid header" );

   // read in version and check
   uint32 const version = stream.ReadUint32();
   BPE_VERIFY( version <= 7, false, "invalid version" );

   // read in dimensions
   int const width = stream.ReadUint16();
   int const height = stream.ReadUint16();
   int const depth = stream.ReadUint16();

   EFormat const format = static_cast<EFormat>(stream.ReadUint32());
   D3DFORMAT const d3dFormat = CTexture::GetD3DFormat(format);

   // read alpha flag
   bool const hasAlpha = stream.ReadBool();
   // Read additional flags
   uint32 const additionalFlags = stream.ReadUint32();
   // Read min/max RGBA
   uint32 minRGBA = stream.ReadUint32();
   uint32 maxRGBA = stream.ReadUint32();

   int8 filterHint = kFH_Default;
   uint8 alphaRefValue = 0;
   
   if( version >= 6 )
   {
      filterHint = stream.ReadInt8();
      alphaRefValue = stream.ReadUint8();
   }

   uint8 maxLODOffset = 0;

   if( version >= 7 )
   {
      maxLODOffset = stream.ReadInt8();
   }

   // read in type
   EType const type = static_cast<EType>(stream.ReadUint32());

   // read in number of levels (mipmaps)
   int const mipmapLevels = stream.ReadUint8();

   CTexture * pTexture = new CTexture(format, type, kUsage_Texture, width, height, depth, mipmapLevels);

   pTexture->mHasAlpha = hasAlpha;
   pTexture->mAdditionalFlags = additionalFlags;
   pTexture->mMinRGBA = minRGBA;
   pTexture->mMaxRGBA = maxRGBA;

   pTexture->mFilterHint = filterHint;
   pTexture->mAlphaRefValue = alphaRefValue;
   pTexture->mMaxLODOffset = maxLODOffset;

   // skip padding of header
   {
      int const kFixedHeaderSize = 128;
      stream.Get( NULL, kFixedHeaderSize - stream.GetReadPosition() );
   }

   int imageCount;

   LPDIRECT3DBASETEXTURE9 pD3DTexture = NULL;

   HRESULT res;
   switch( type )
   {
      case kType_Normal:
         {
            if ( depth > 1 )
            {
               res = D3DDeviceUncached()->CreateVolumeTexture(pTexture->mWidth, 
                  pTexture->mHeight, 
                  pTexture->mDepth,
                  pTexture->mMipCount, 
                  0,   
                  d3dFormat, 
                  D3DPOOL_MANAGED, 
                  (LPDIRECT3DVOLUMETEXTURE9*)&pD3DTexture, 
                  NULL);
            }
            else
            {
               res = D3DDeviceUncached()->CreateTexture(pTexture->mWidth, 
                                                        pTexture->mHeight, 
                                                        pTexture->mMipCount, 
                                                        0,   
                                                        d3dFormat, 
                                                        D3DPOOL_MANAGED, 
                                                        (LPDIRECT3DTEXTURE9*)&pD3DTexture, 
                                                        NULL);
            }
            imageCount = 1;
         }
         break;

      case kType_Cube:
         {
            BPE_VERIFY( pTexture->mWidth == pTexture->mHeight, false, "Invalid cube map dimensions" );
            res = D3DDeviceUncached()->CreateCubeTexture(pTexture->mWidth, 
                                                         pTexture->mMipCount, 
                                                         0,
                                                         d3dFormat,
                                                         D3DPOOL_MANAGED, 
                                                         (LPDIRECT3DCUBETEXTURE9*)&pD3DTexture, 
                                                         NULL);
            imageCount = 6;
         }
         break;
      default:
         BPE_VERIFYA( false, "Invalid texture format" );
         break;
   }

   BPE_VERIFY( pD3DTexture != NULL, false, "Texture creation failed");

   // go over all images (there can be multiple ones for cubemaps)
   for( int imageIdx = 0; imageIdx < imageCount; ++imageIdx )
   {
      // load in texture levels
      for( int levelIdx = 0; levelIdx < pTexture->mMipCount; ++levelIdx )
      {
         uint32 const levelSize = stream.ReadUint32();

         if( levelIdx < pTexture->mMipCount )
         {
            // get surface to load texture into
            LPDIRECT3DSURFACE9 pSurface = NULL;
            LPDIRECT3DVOLUME9 pVolume = NULL;

            switch( type )
            {
            case kType_Normal:
               {
                  if ( depth > 1 )
                  {
                     ((LPDIRECT3DVOLUMETEXTURE9)pD3DTexture)->GetVolumeLevel(levelIdx, &pVolume);
                  }
                  else
                  {
                     ((LPDIRECT3DTEXTURE9)pD3DTexture)->GetSurfaceLevel(levelIdx, &pSurface);
                  }
               }
               break;

            case kType_Cube:
               {
                  ((LPDIRECT3DCUBETEXTURE9)pD3DTexture)->GetCubeMapSurface(static_cast<D3DCUBEMAP_FACES>(imageIdx), levelIdx, &pSurface);
               }
               break;
            }

            if ( pVolume )
            {
               D3DLOCKED_BOX lockedVolume;
               pVolume->LockBox( &lockedVolume, NULL, 0 );
               
               stream.Get( lockedVolume.pBits, levelSize );
               pVolume->UnlockBox();
               pVolume->Release();
            }
            else if ( pSurface )
            {
               D3DLOCKED_RECT rect;
               res = pSurface->LockRect(&rect, NULL, 0);

               stream.Get( rect.pBits, levelSize );

               pSurface->UnlockRect();
               pSurface->Release();
            }
         }
         else
         {
            stream.Get(NULL, levelSize);
         }

         while( stream.GetReadPosition() % 32 )
            stream.ReadInt8();
      }
   }

   pTexture->mTexture = TComPtr<IDirect3DBaseTexture9>(pD3DTexture);

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
   CTexture* pTexture = new CTexture(CTexture::kFormat_A8R8G8B8, kType_Normal, kUsage_Texture, 4, 4, 1, 1);

   LPDIRECT3DTEXTURE9 pD3DTexture = NULL;
   D3DDeviceUncached()->CreateTexture(pTexture->mWidth, pTexture->mHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pD3DTexture, NULL); 

   D3DXVECTOR4 d3dColor(color.GetR() / 255.0f, color.GetG() / 255.0f, color.GetB() / 255.0f, color.GetA() / 255.0f);
   D3DXFillTexture(pD3DTexture, ColorFill, &d3dColor);

   pTexture->mTexture = TComPtr<IDirect3DBaseTexture9>(pD3DTexture);
   
   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture* CBaseTexture::Create(EGeneratedTextureType const type)
{
   CTexture* pTexture = NULL;
   
   switch( type )
   {
   case kGTT_White:
   case kGTT_Grey:
   case kGTT_Black:
   case kGTT_FlatNormal:
      {
         pTexture = new CTexture(CTexture::kFormat_A8R8G8B8, kType_Normal, kUsage_Texture, 4, 4, 1, 1);

         D3DXVECTOR4 color;
         switch( type ) 
         {
         case kGTT_White:
            color = D3DXVECTOR4( 1.0f, 1.0f, 1.0f, 1.0f );
            break;
         case kGTT_Grey:
            color = D3DXVECTOR4( 0.5f, 0.5f, 0.5f, 0.5f );
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

         pTexture->mTexture = TComPtr<IDirect3DBaseTexture9>(pD3DTexture);
      }
      break;
   case kGTT_SpecularPower:
      {
         pTexture = new CTexture(CTexture::kFormat_Invalid, kType_Normal, kUsage_Texture, 512, 128, 1, 1);

         LPDIRECT3DTEXTURE9 pD3DTexture = NULL;

         D3DDeviceUncached()->CreateTexture(pTexture->mWidth, pTexture->mHeight, 1, 0, D3DFMT_L16, D3DPOOL_MANAGED, &pD3DTexture, NULL); 

         D3DXFillTexture(pD3DTexture, SpecularPowerFill, NULL);
         pTexture->mTexture = TComPtr<IDirect3DBaseTexture9>(pD3DTexture);
      }
      break;
   default:
      break;
   }
   
   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::TryCreate(int const width, 
                                       int const height,
                                       int const levels,
                                       EFormat const format,
                                       EUsage const usage,
                                       EAntiAliasType const /*aaFlag*/, 
                                       ERenderMemory const /*memory*/,
                                       SCreateTextureParams const * pCreateTextureParams)
{
   CTexture* pTexture = new CTexture(format, kType_Normal, usage, width, height, 1, levels);
   D3DFORMAT const d3dFormat = CTexture::GetD3DFormat(format);

   LPDIRECT3DTEXTURE9 pD3DTexture = NULL;
   LPDIRECT3DSURFACE9 pSurface = NULL;

   bool isDepthBuffer = false;

   switch( usage )
   {
   case kUsage_Texture:
   case kUsage_TextureLinear:
      {
         DWORD d3dUsage = 0;
         D3DPOOL d3dPool = D3DPOOL_MANAGED;
         
         if( usage == kUsage_TextureLinear )
         {
            d3dUsage = D3DUSAGE_DYNAMIC;
            d3dPool = D3DPOOL_DEFAULT;
         }

         D3DDeviceUncached()->CreateTexture( width, height, levels, d3dUsage, d3dFormat, d3dPool, &pD3DTexture, NULL );
      }
      break;

   case kUsage_RenderTarget:
      {
         D3DDeviceUncached()->CreateTexture( width, height, levels, D3DUSAGE_RENDERTARGET, d3dFormat, D3DPOOL_DEFAULT, &pD3DTexture, NULL );
      }
      break;

   case kUsage_DepthBuffer:
      {
         switch(format)
         {
         case kFormat_A8R8G8B8:
            D3DDeviceUncached()->CreateTexture( width, height, 1, D3DUSAGE_DEPTHSTENCIL, (D3DFORMAT)MAKEFOURCC('I','N','T','Z'), D3DPOOL_DEFAULT, &pD3DTexture, NULL);
            break;

         case kFormat_D24X8:
#ifdef NO_DEPTH_BUFFER_CREATE_TEXTURE
            isDepthBuffer = true;
            D3DDeviceUncached()->CreateDepthStencilSurface( width, height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurface, NULL );
#else
            D3DDeviceUncached()->CreateTexture( width, height, 1, D3DUSAGE_DEPTHSTENCIL, d3dFormat, D3DPOOL_DEFAULT, &pD3DTexture, NULL);
#endif
            break;
         }
      }
      break;
   }

   if( !isDepthBuffer )
   {
      BPE_VERIFY(pD3DTexture != NULL, false, "couldn't create texture");
      pTexture->mTexture = TComPtr<IDirect3DBaseTexture9>(pD3DTexture);
   }

   if( pSurface == NULL )
   {
      pD3DTexture->GetSurfaceLevel(0, &pSurface);
   }

   BPE_VERIFY(pSurface != NULL, false, "couldn't create texture");
   pTexture->mSurface = TComPtr<IDirect3DSurface9>(pSurface);
   
   if( pD3DTexture )
   {
      D3DXVECTOR4 color = D3DXVECTOR4( 0, 0, 0, 0 );
      D3DXFillTexture(pD3DTexture, ColorFill, &color);
   }
   
   return pTexture;
}

CBaseTexture * CBaseTexture::Create(int const width, 
                                    int const height,
                                    int const levels,
                                    EFormat const format,
                                    EUsage const usage,
                                    EAntiAliasType const aaFlag, 
                                    ERenderMemory const memory,
                                    SCreateTextureParams const * pCreateTextureParams)
{
   return TryCreate(width, height, levels, format, usage, aaFlag, memory, pCreateTextureParams);
}

CBaseTexture * CBaseTexture::CreateCheckerboard( int const width, int const height, const CColor & color0, const CColor & color1 )
{
	CTexture * pTexture = new CTexture(CTexture::kFormat_A8R8G8B8, kType_Normal, kUsage_Texture, width, height, 1, 1);
	CVector4 color0v( color0.GetVector4() );
	CVector4 color1v( color1.GetVector4() );
	LPDIRECT3DTEXTURE9 pD3DTexture = NULL;

	D3DDeviceUncached()->CreateTexture(pTexture->mWidth, pTexture->mHeight, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pD3DTexture, NULL); 

	//TODO: actual checkerboard on win32
	D3DXFillTexture(pD3DTexture, ColorFill, (D3DXVECTOR4*)&color0v);

	pTexture->mTexture = TComPtr<IDirect3DBaseTexture9>(pD3DTexture);

	return pTexture;
}

//----------------------------------------------------------------------------

static D3DFORMAT const gD3DFormatMapping[] =
{
   D3DFMT_A8R8G8B8,
   D3DFMT_X8R8G8B8,

   D3DFMT_A16B16G16R16F,
   D3DFMT_R32F,
   
   D3DFMT_D24S8,
   
   D3DFMT_DXT1,
   D3DFMT_DXT3,
   D3DFMT_DXT5,

   D3DFMT_A32B32G32R32F,

   D3DFMT_L8,

   D3DFMT_D24S8,//

   D3DFMT_A8R8G8B8, // Placeholder for SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1,
};

BPE_CTASSERT(BPE_ARRAY_SIZE(gD3DFormatMapping) == CTexture::kFormat_Count);

D3DFORMAT CTexture::GetD3DFormat(EFormat format)
{
   return gD3DFormatMapping[format];
}

//----------------------------------------------------------------------------

void CTexture::SetTexture(int const textureStage) const
{
   SetCurrentTextureParams(textureStage);

   D3DDevice()->SetTexture(textureStage, mTexture.GetPtr());
}

//----------------------------------------------------------------------------

void CTexture::Lock(void** pMemory, int * pPitch)
{
   BPE_VERIFY( mType == kType_Normal && mDepth == 1, false, "Cannot lock cubemaps or volume textures" );

   HRESULT res = S_OK;

   IDirect3DTexture9* pTexture = static_cast<IDirect3DTexture9*>( mTexture.GetPtr() );

   {
      IDirect3DSurface9* pSurface = NULL;
      res = pTexture->GetSurfaceLevel( 0, &pSurface );
      mLockedSurface.Reset(pSurface);
   }

   D3DLOCKED_RECT rect;
   res = mLockedSurface->LockRect(&rect, NULL, 0);

   *pMemory = rect.pBits;
   *pPitch = rect.Pitch;
}

//----------------------------------------------------------------------------

void CTexture::Unlock()
{
   HRESULT res = S_OK;

   IDirect3DTexture9* pTexture = static_cast<IDirect3DTexture9*>( mTexture.GetPtr() );
   
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

int CBaseTexture::GetMipCount() const
{
   return ((CTexture*)this)->mMipCount;
}


//----------------------------------------------------------------------------

int const CBaseTexture::GetDepth() const
{
   return ((CTexture*)this)->mDepth;
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
   return kAA_None;
}

//----------------------------------------------------------------------------

void CBaseTexture::CloneTexture(CBaseTexture* pSourceBaseTexture)
{
   CTexture* pThis = (CTexture*)this;
   CTexture* pSource = (CTexture*)pSourceBaseTexture;

   pThis->mSurface = pSource->mSurface;
   pThis->mTexture = pSource->mTexture;
   
   pThis->mLockedSurface.Reset();

   pThis->mWidth = pSource->mWidth;
   pThis->mHeight = pSource->mHeight;
   pThis->mDepth = pSource->mDepth;
   pThis->mMipCount = pSource->mMipCount;

   // This copies all parameters defined in CBaseTexture
   CloneBaseTexture(pSource);
}