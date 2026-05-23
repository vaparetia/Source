//----------------------------------------------------------------------------
// Win32CTexture.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/RVL/RVLCTexture.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine\Streams\CMemoryInputStream.h"

//----------------------------------------------------------------------------

namespace
{
   GXTexFmt txtr_format_to_gx_format( CBaseTexture::EFormat const format )
   {
      switch ( format )
      {
      case CBaseTexture::kFormat_A8R8G8B8:
         return GX_TF_RGBA8;

      case CBaseTexture::kFormat_DXT1:
         return GX_TF_CMPR;

      case CBaseTexture::kFormat_Luminance8:
         return GX_TF_I8;

      default:
         BPE_VERIFY( false, false, "Unsupported texture format" );
         return GX_TF_I4;
      }
   }
}

//----------------------------------------------------------------------------

CTexture::CTexture(EFormat format, EType type, EUsage usage, int width, int height, int depth, CRenderHWAllocator::SHandle const *pHandle )
:  CBaseTexture(format, type, usage)
,  mpHandle( pHandle )
{
   GXTexWrapMode const wrap_s = MathUtils::IsPowerOfTwo( width ) ? GX_REPEAT : GX_CLAMP;
   GXTexWrapMode const wrap_t = MathUtils::IsPowerOfTwo( height ) ? GX_REPEAT : GX_CLAMP;

   GXInitTexObj( &mTexture, mpHandle->mpAddress, width, height, txtr_format_to_gx_format( format ), 
      wrap_s, wrap_t, GX_FALSE );
   GXInitTexObjFilter( &mTexture, GX_LINEAR, GX_LINEAR );
}

//----------------------------------------------------------------------------

CTexture::~CTexture()
{
   if ( mpHandle )
   {
      gpRenderBackend->Free( mpHandle );
   }
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
   BPE_VERIFY( version == 1, false, "invalid version" );

   int const width = stream.ReadUint16();
   int const height = stream.ReadUint16();

   EFormat const format = static_cast<EFormat>(stream.ReadUint32());

   // read alpha flag
   bool const hasAlpha = stream.ReadBool();

   // read in type
   EType const type = static_cast<EType>(stream.ReadUint32());

   // read in number of levels (mipmaps)
   int const mipmapLevels = stream.ReadUint8();

   unsigned const textureDataSize = stream.ReadUint32();
   while ( stream.GetReadPosition() & 31 )
   {
      stream.ReadUint8();
   }

   boost::scoped_ptr<uint8> pData;

   CRenderHWAllocator::SHandle const *pHandle = gpRenderBackend->AllocFixed( textureDataSize, kRM_Video );
   stream.Get( pHandle->mpAddress, textureDataSize );
   DCFlushRange( pHandle->mpAddress, textureDataSize );

   return new CTexture( format, type, kUsage_Texture, width, height, 1, pHandle );
}

CBaseTexture* CBaseTexture::Create(EGeneratedTextureType const type)
{
   return NULL;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::Create(int const width, 
                                    int const height,
                                    int const levels,
                                    EFormat const format,
                                    EUsage const usage,
                                    EAntiAliasType const /*aaFlag*/, 
                                    ERenderMemory const /*memory*/ )
{
   return NULL;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetWidth() const
{
   return GXGetTexObjWidth( &( (CTexture *) this )->mTexture );
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetHeight() const
{
   return GXGetTexObjHeight( &( (CTexture *) this )->mTexture );
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetDepth() const
{
   return 1;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasWidth() const
{
   return GetWidth();
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasHeight() const
{
   return GetHeight();
}

//----------------------------------------------------------------------------

CBaseTexture::EAntiAliasType const CBaseTexture::GetAntiAliasType() const
{
   return kAA_None;
}

//----------------------------------------------------------------------------

void CTexture::Lock(void** pMemory, int * pPitch)
{
}

//----------------------------------------------------------------------------

void CTexture::Unlock()
{
}

//----------------------------------------------------------------------------

void CTexture::SetTexture(int const textureStage) const
{
   SetCurrentTextureParams(textureStage);

   CTexture const *pThis = reinterpret_cast<CTexture const *>(this);
   
   GXLoadTexObj( &pThis->mTexture, GXTexMapID( textureStage ) );
}
