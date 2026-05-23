//----------------------------------------------------------------------------
// PS3CTexture.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "PS3CTexture.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

using namespace cell::Gcm;

//----------------------------------------------------------------------------

namespace
{
   static uint32 const gTextureFormatMapping[] =
   {
      CELL_GCM_TEXTURE_A8R8G8B8,
      CELL_GCM_TEXTURE_A8R8G8B8, //X8R8G8B8

      CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT,
      CELL_GCM_TEXTURE_X32_FLOAT,

      // reinterpret depth texture as ARGB8 so we can extract floating point depth in the shader
      CELL_GCM_TEXTURE_A8R8G8B8, //CELL_GCM_TEXTURE_DEPTH24_D8,     

      CELL_GCM_TEXTURE_COMPRESSED_DXT1,
      CELL_GCM_TEXTURE_COMPRESSED_DXT23,
      CELL_GCM_TEXTURE_COMPRESSED_DXT45,

      CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT,

      CELL_GCM_TEXTURE_B8,

      CELL_GCM_TEXTURE_A8R8G8B8, // kFormat_D24FS8 not officially supported on platform
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(gTextureFormatMapping) == CTexture::kFormat_Count);

   static uint32 const gBitsPerPixel[] =
   {
      4 * 8,   // 4x byte
      4 * 8,   // 4x byte

      4 * 16,  // 4x half
      1 * 32,  // 1x float

      4 * 8,   // 3x byte (depth) + 1x byte (stencil)

      4,       // 4bits per pixel, DXT1
      8,       // 8bits per pixel, DXT3
      8,       // 8bits per pixel, DXT5

      4 * 32,  // 4x float

      1 * 8,   // 1x byte

      4 * 8    // kFormat_D24FS8 not officially supported on platform
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(gBitsPerPixel) == CTexture::kFormat_Count);

   static uint32 const gSurfaceFormatMapping[] =
   {
      CELL_GCM_SURFACE_A8R8G8B8,
      CELL_GCM_SURFACE_X8R8G8B8_O8R8G8B8,

      CELL_GCM_SURFACE_F_W16Z16Y16X16,
      CELL_GCM_SURFACE_F_X32,

      CELL_GCM_SURFACE_Z24S8,     

      0,
      0,
      0,

      CELL_GCM_SURFACE_F_W32Z32Y32X32,

      CELL_GCM_SURFACE_B8,

      CELL_GCM_SURFACE_Z24S8 // kFormat_D24FS8 not officially supported on platform
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(gSurfaceFormatMapping) == CTexture::kFormat_Count);

   static uint32 const gDepthSurfaceFormatMapping[] =
   {
      CELL_GCM_SURFACE_Z24S8,
      0,

      0,
      0,

      CELL_GCM_SURFACE_Z24S8,     

      0,
      0,
      0,

      0,

      0,

      CELL_GCM_SURFACE_Z24S8 // kFormat_D24FS8 not officially supported on platform
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(gDepthSurfaceFormatMapping) == CTexture::kFormat_Count);

   struct CellGtfFileHeader
   {
      uint32_t  version;      /* Version (Correspond to dds2gtf converter version) */
      uint32_t  size;         /* Total size of Texture (Excluding size of header & attribute) */
      uint32_t  numTexture;   /* Number of textures in this file */
   };

   struct CellGtfTextureAttribute
   {         
      uint32_t id;            /* Texture ID */
      uint32_t offsetToTex;   /* Offset to texture from beginning of file */
      uint32_t textureSize;	/* Size of texture */
      uint32_t mMinRGBA;      // BP - min RGBA values present in texture
      uint32_t mMaxRGBA;      // BP - min color value present in texture
      uint32_t mAdditionalFlags; // BP - misc BP flags
      CellGcmTexture tex;		/* Texture structure defined in GCM library */
      int8_t   mFilterHint;      // BP
      uint8_t  mAlphaRefValue;   // BP
      int8_t   mMaxLODOffset;    // BP
   };

   uint32 const kGtfVersion = 0x02000101;   /* v200.01.00 - BP minor */
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::Create(void *pMemory, uint32 const size)
{
   CTexture* pTexture = NULL;
   
   CellGtfFileHeader* pHeader = (CellGtfFileHeader*)pMemory;
   BPE_VERIFY(pHeader->version <= kGtfVersion, false, "Invalid GTF Version");

   if( pHeader->numTexture == 1 )
   {
      pTexture = new CTexture(CBaseTexture::kFormat_Invalid, CBaseTexture::kType_Normal, CBaseTexture::kUsage_Texture);
      
      pTexture->mMaxAniso = CELL_GCM_TEXTURE_MAX_ANISO_4;

      CellGtfTextureAttribute* pAttribute = (CellGtfTextureAttribute*)( (uint8_t*)pMemory + sizeof(CellGtfFileHeader) );
      
      // Read additional flags and min/max RGBA from GTF
      pTexture->mAdditionalFlags = pAttribute->mAdditionalFlags;
      pTexture->mMinRGBA = pAttribute->mMinRGBA;
      pTexture->mMaxRGBA = pAttribute->mMaxRGBA;

      if( pHeader->version >= 0x02000100 )
      {
         pTexture->mFilterHint = pAttribute->mFilterHint;
         pTexture->mAlphaRefValue = pAttribute->mAlphaRefValue;
      }
      else
      {
         pTexture->mFilterHint = kFH_Default;
         pTexture->mAlphaRefValue = 0;
      }

      if( pHeader->version >= 0x02000101 )
         pTexture->mMaxLODOffset = pAttribute->mMaxLODOffset;
      else
         pTexture->mMaxLODOffset = 0;

      pAttribute->tex._padding = 0; // reset for safety

      pTexture->mTexture = pAttribute->tex;

      // Check alpha value of remap to see if it comes from the texture of it it's constant 0 or 1.
      // See documentation for cellGcmSetTextureRemap for details.
      uint32 const opp = (pAttribute->tex.remap >> 8) & 0x3;
      
      switch(opp)
      {
      case CELL_GCM_TEXTURE_REMAP_ZERO:
      case CELL_GCM_TEXTURE_REMAP_ONE:
         pTexture->mHasAlpha = false;
         break;

      case CELL_GCM_TEXTURE_REMAP_REMAP:
         pTexture->mHasAlpha = true;
         break;
      }

      pTexture->mMemoryAllocationSize = (pAttribute->textureSize + 15) & ~15;
      int const allocateFromSystemMemory = (pTexture->mAdditionalFlags & CTexture::kAF_SystemMemory);

      pTexture->mMemory = RenderBackend()->AllocFixed(pTexture->mMemoryAllocationSize, 128, allocateFromSystemMemory ? kRM_System : kRM_Video);
      BPE_VERIFY(pTexture->mMemory, false, "Couldn't allocate memory!");

      cellGcmAddressToOffset(pTexture->mMemory->mpAddress, &pTexture->mTexture.offset);
      pTexture->mTexture.location = allocateFromSystemMemory ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;

      uint32 const pDest = (uint32)pTexture->mMemory->mpAddress;
      uint32 const pSource = (uint32)pMemory + pAttribute->offsetToTex;
      
      // PPU vs. SPU copy of data
      // Currently still using PPU version because otherwise we're forcing a sync on the fragment patch SPU thread
      //CStopWatch watch;
      gpRenderBackend->SPU_SyncDMA(pDest, pSource, pTexture->mMemoryAllocationSize);
      //bpe_debugger_printf("Time to copy texture: %.3f ms (src: 0x%8.8x dst: 0x%8.8x size: 0x%8.8x)\n", watch.GetElapsedTime() * 1000.0f, pSource, pDest, sizeAligned);
   }

   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::CreateSolid(CColor const & color)
{
   CTexture* pTexture = (CTexture*)CTexture::CreateTexture(4, 4, 1, CTexture::kFormat_A8R8G8B8);
   uint32 fillColor = color.GetARGB();

   uint32* pPtr = NULL;
   int pitch = 0;
   pTexture->Lock((void**)&pPtr, &pitch);

   if( pPtr != NULL )
   {
      int numPixels = pTexture->GetWidth() * pTexture->GetHeight();

      for( int y = 0; y < pTexture->GetHeight(); ++y)
         for( int x = 0; x < pTexture->GetWidth(); ++x )
            pPtr[(y * pitch / 4)  + x] = fillColor;
   }

   pTexture->Unlock();

   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::Create(EGeneratedTextureType const type)
{
   CTexture* pTexture = NULL;

   uint32 fillColor = 0;

   switch( type )
   {
   case kGTT_White:
      {
         pTexture = (CTexture*)CTexture::CreateTexture(4, 4, 1, CTexture::kFormat_A8R8G8B8);
         fillColor = CColor::White().GetARGB();
      }
      break;

   case kGTT_Grey:
      {
         pTexture = (CTexture*)CTexture::CreateTexture(4, 4, 1, CTexture::kFormat_A8R8G8B8);
         fillColor = CColor(128, 128, 128, 128).GetARGB();
      }
      break;
   
   case kGTT_Black:
      {
         pTexture = (CTexture*)CTexture::CreateTexture(4, 4, 1, CTexture::kFormat_A8R8G8B8);
         fillColor = CColor::Black().GetARGB();
      }
      break;

   case kGTT_FlatNormal:
      {
         pTexture = (CTexture*)CTexture::CreateTexture(4, 4, 1, CTexture::kFormat_A8R8G8B8);
         fillColor = CColor(128, 128, 255).GetARGB();
      }
      break;

   case kGTT_SpecularPower:
      {
         pTexture = NULL;
      }
      break;
   }

   if( pTexture != NULL )
   {
      uint32* pPtr = NULL;
      int pitch = 0;
      pTexture->Lock((void**)&pPtr, &pitch);

      if( pPtr != NULL )
      {
         int numPixels = pTexture->GetWidth() * pTexture->GetHeight();

         for( int y = 0; y < pTexture->GetHeight(); ++y)
            for( int x = 0; x < pTexture->GetWidth(); ++x )
               pPtr[(y * pitch / 4)  + x] = fillColor;
      }

      pTexture->Unlock();
   }

   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::CreateCheckerboard( int const width, int const height, const CColor & color0, const CColor & color1 )
{
   CTexture * pTexture = (CTexture*)CTexture::CreateTexture(width, height, 1, CTexture::kFormat_A8R8G8B8);

   uint32* pPtr = NULL;
   int pitch = 0;
   pTexture->Lock((void**)&pPtr, &pitch);

   if( pPtr != NULL )
   {
      int numPixels = pTexture->GetWidth() * pTexture->GetHeight();

      for( int y = 0; y < pTexture->GetHeight(); ++y)
         for( int x = 0; x < pTexture->GetWidth(); x+=2 )
         {
            const uint32 fgColor = ( (y&1) ? color0 : color1 ).GetARGB();
            const uint32 bgColor = ( (y&1) ? color1 : color0 ).GetARGB();

            pPtr[(y * pitch / 4)  + x+0 ] = fgColor;
            pPtr[(y * pitch / 4)  + x+1 ] = bgColor;
         }
   }

   pTexture->Unlock();
}


//----------------------------------------------------------------------------

static void get_flags_from_usage( CBaseTexture::EAntiAliasType usesAA, int *pWidthMult, int *pHeightMult )
{
   switch ( usesAA )
   {
   case CBaseTexture::kAA_MSAA4x:
      *pWidthMult = 2;
      *pHeightMult = 2;
      break;
   case CBaseTexture::kAA_MSAA2x:
      *pWidthMult = 2;
      *pHeightMult = 1;
      break;
   default:
      *pWidthMult = 1;
      *pHeightMult = 1;
      break;
   }
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::TryCreate(int const _inputWidth, 
                                       int const _inputHeight,
                                       int const Levels,
                                       EFormat const format,
                                       EUsage const usage,
                                       EAntiAliasType const aaFlags,
                                       ERenderMemory const memory,
                                       SCreateTextureParams const * pCreateTextureParams)
{
   SCreateTextureParams defaultCreateTextureParams;

   if( pCreateTextureParams == NULL )
      pCreateTextureParams = &defaultCreateTextureParams;

   BPE_VERIFY( usage != kUsage_Texture || aaFlags == kAA_None, false, "Cannot create a non-rendertarget with AA" );

   CTexture * pTexture = new CTexture(format, kType_Normal, usage);
   
   int widthMult = 1;
   int heightMult = 1;
   get_flags_from_usage( aaFlags, &widthMult, &heightMult );

   int const finalWidth = _inputWidth * widthMult;
   int const finalHeight = _inputHeight * heightMult;

   switch( usage )
   {
   case kUsage_RenderTarget:
      {
         bool const allocateTiled = pCreateTextureParams->mIsTiled && ((finalWidth >= 320) || ( aaFlags != kAA_None ));

         int pitch = finalWidth;
         switch(format)
         {
         case kFormat_A16B16G16R16F:
            pitch *= 8;
            break;
         
         case kFormat_A32B32G32R32F:
            pitch *= 16;
            break;
      
         default:
            pitch *= 4;
            break;
         }

         int height = finalHeight;

         CRenderHWAllocator::SHandle const * pTiledMemory = NULL;

         if( allocateTiled )
         {
            pTiledMemory = RenderBackend()->AllocTiledRenderTarget(false, aaFlags, memory, finalWidth, &height, &pitch, false, 0);
         }

         if( pTiledMemory != NULL )
         {
            pTexture->mMemory = pTiledMemory;
            pTexture->mIsTiled = true;
         }
         else
         {
            // pitch must be multiple of 64
            pitch = (pitch + 63) & ~63;
            pTexture->mMemory = RenderBackend()->AllocFixed(pitch * height, 128, memory);
         }

         if( pTexture->mMemory == NULL )
         {
            delete pTexture;
            return NULL;
         }

         pTexture->mAAType = uint32( aaFlags );

         pTexture->mTexture.format = gTextureFormatMapping[format] | CELL_GCM_TEXTURE_LN;
         pTexture->mTexture.mipmap = 1;
         pTexture->mTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;

         pTexture->mTexture.remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 8  |
            CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
            CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
            CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
            CELL_GCM_TEXTURE_REMAP_FROM_A;

         if( format == kFormat_X8R8G8B8 )
         {
            pTexture->mTexture.remap &= ~((CELL_GCM_TEXTURE_REMAP_REMAP << 8) | CELL_GCM_TEXTURE_REMAP_FROM_A);
            pTexture->mTexture.remap |= (CELL_GCM_TEXTURE_REMAP_ONE << 8);
         }

         pTexture->mTexture.width = finalWidth;
         pTexture->mTexture.height = finalHeight;
         pTexture->mTexture.depth = 1;

         pTexture->mTexture.pitch = pitch;

         // Clear memory
         if( pCreateTextureParams->mClearMemory )
         {
            memset(pTexture->mMemory->mpAddress, 0, pTexture->mTexture.pitch*pTexture->mTexture.height);
         }

         cellGcmAddressToOffset(pTexture->mMemory->mpAddress, &pTexture->mTexture.offset);
         pTexture->mTexture.location = memory;
      }
      break;

   case kUsage_DepthBuffer:
      {
         ERenderMemory const depthBufferMemory = kRM_Video;

         bool const allocateTiled = pCreateTextureParams->mIsTiled && ((finalWidth >= 320) || ( aaFlags != kAA_None ));

         int height = finalHeight;
         int pitch = 4 * finalWidth;
         
         CRenderHWAllocator::SHandle const * pTiledMemory = NULL;
         
         if( allocateTiled )
         {
            pTiledMemory = RenderBackend()->AllocTiledRenderTarget(true, aaFlags, depthBufferMemory, finalWidth, &height, &pitch, pCreateTextureParams->mCreateZCull, pCreateTextureParams->mZCullOffset);
         }

         if( pTiledMemory != NULL )
         {
            pTexture->mMemory = pTiledMemory;
            pTexture->mIsTiled = true;
         }
         else
         {
            // pitch must be multiple of 64
            pitch = (pitch + 63) & ~63;
            pTexture->mMemory = RenderBackend()->AllocFixed(pitch * height, 128, depthBufferMemory);
         }

         if( pTexture->mMemory == NULL )
         {
            delete pTexture;
            return NULL;
         }

         pTexture->mAAType = uint32( aaFlags );

         switch(format)
         {
         case kFormat_A8R8G8B8:
            pTexture->mTexture.format = CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN;
            break;

         case kFormat_D24X8:
            pTexture->mTexture.format = CELL_GCM_TEXTURE_DEPTH24_D8 | CELL_GCM_TEXTURE_LN;
            break;
         }

         pTexture->mTexture.mipmap = 1;
         pTexture->mTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;

         pTexture->mTexture.remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 8  |
            CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
            CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
            CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
            CELL_GCM_TEXTURE_REMAP_FROM_A;

         pTexture->mTexture.width = finalWidth;
         pTexture->mTexture.height = finalHeight;
         pTexture->mTexture.depth = 1;

         pTexture->mTexture.pitch = pitch;

         cellGcmAddressToOffset(pTexture->mMemory->mpAddress, &pTexture->mTexture.offset);
         pTexture->mTexture.location = depthBufferMemory;
      }
      break;

   case kUsage_Texture:
   case kUsage_TextureLinear:
      {
         pTexture->mTexture.format = gTextureFormatMapping[format];
         
         if( usage == kUsage_TextureLinear )
            pTexture->mTexture.format |= CELL_GCM_TEXTURE_LN;

         pTexture->mTexture.mipmap = 1;
         pTexture->mTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;

         pTexture->mTexture.remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
            CELL_GCM_TEXTURE_REMAP_REMAP << 8  |
            CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
            CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
            CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
            CELL_GCM_TEXTURE_REMAP_FROM_A;

         if( format == kFormat_X8R8G8B8 )
         {
            pTexture->mTexture.remap &= ~((CELL_GCM_TEXTURE_REMAP_REMAP << 8) | CELL_GCM_TEXTURE_REMAP_FROM_A);
            pTexture->mTexture.remap |= (CELL_GCM_TEXTURE_REMAP_ONE << 8);
         }

         pTexture->mTexture.width = finalWidth;
         pTexture->mTexture.height = finalHeight;
         pTexture->mTexture.depth = 1;

         uint32 const pitch = finalWidth * 4;

         pTexture->mTexture.pitch = pitch;

         pTexture->mMemoryAllocationSize = pitch * finalHeight;
         pTexture->mMemory = RenderBackend()->AllocFixed(pTexture->mMemoryAllocationSize, 128, memory);
         
         if( pTexture->mMemory == NULL )
         {
            delete pTexture;
            return NULL;
         }

         cellGcmAddressToOffset(pTexture->mMemory->mpAddress, &pTexture->mTexture.offset);
         pTexture->mTexture.location = memory;
      }
      break;

   default:
      BPE_VERIFY(false, false, "unsupported usage");
      break;
   }

   return pTexture;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::Create(int const _inputWidth, 
                                    int const _inputHeight,
                                    int const Levels,
                                    EFormat const format,
                                    EUsage const usage,
                                    EAntiAliasType const aaFlags,
                                    ERenderMemory const memory,
                                    SCreateTextureParams const * pCreateTextureParams)
{
   CBaseTexture* pTexture = TryCreate(_inputWidth, _inputHeight, Levels, format, usage, aaFlags, memory, pCreateTextureParams);
   
   BPE_VERIFY(pTexture, false, "Couldn't allocate memory for texture!");

   return pTexture;
}

//----------------------------------------------------------------------------

CTexture::CTexture(EFormat format, EType type, EUsage usage)
:  CBaseTexture(format, type, usage)
,  mMemory(NULL)
,  mMemoryAllocationSize(0)
,  mIsTiled(false)
,  mAAType( kAA_None )
,  mMaxAniso(CELL_GCM_TEXTURE_MAX_ANISO_1)
{
   memset(&mTexture, 0, sizeof(CellGcmTexture));
}

//----------------------------------------------------------------------------

CTexture::~CTexture()
{
   FreeMemory();
}

//----------------------------------------------------------------------------

void CTexture::FreeMemory()
{
   if( mMemory )
   {
      if( mIsTiled )
         RenderBackend()->FreeTiledRenderTargetImmediate(mMemory);
      else
         RenderBackend()->Free(mMemory, 0);

      mMemory = NULL;
   }
}

//----------------------------------------------------------------------------

void CTexture::SetTexture(int const textureStage) const
{
   SetCurrentTextureParams(textureStage);

   if( mMemory )
      cellGcmSetTexture(textureStage, &mTexture);
}

//----------------------------------------------------------------------------

void CTexture::Lock(void** pMemory, int * pPitch)
{
   *pMemory = mMemory->mpAddress;
   *pPitch = mTexture.pitch;
}

//----------------------------------------------------------------------------

void CTexture::Unlock()
{
}

//----------------------------------------------------------------------------

uint32 const CTexture::GcmGetSurfaceFormat() const
{
   return gSurfaceFormatMapping[mFormat];
}

//----------------------------------------------------------------------------

uint32 const CTexture::GcmGetDepthSurfaceFormat() const
{
   return gDepthSurfaceFormatMapping[mFormat];
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetWidth() const
{
   return ((CTexture*)this)->mTexture.width;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetHeight() const
{
   return ((CTexture*)this)->mTexture.height;
}


//----------------------------------------------------------------------------

int const CBaseTexture::GetDepth() const
{
   return ((CTexture*)this)->mTexture.depth;
}

//----------------------------------------------------------------------------

int CBaseTexture::GetMipCount() const
{
   return ((CTexture*)this)->mTexture.mipmap;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasWidth(EAntiAliasType const type) const
{
   switch ( type )
   {
   case kAA_MSAA2x:
   case kAA_MSAA4x:
      return ((CTexture*)this)->mTexture.width >> 1;

   default:
      return ((CTexture*)this)->mTexture.width;
   }
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasHeight(EAntiAliasType const type) const
{
   switch ( type )
   {
   case kAA_MSAA4x:
      return ((CTexture*)this)->mTexture.height >> 1;

   default:
      return ((CTexture*)this)->mTexture.height;
   }
}

//----------------------------------------------------------------------------

CBaseTexture::EAntiAliasType const CBaseTexture::GetAntiAliasType() const
{
   return EAntiAliasType( ((CTexture*)this)->mAAType );
}

//----------------------------------------------------------------------------

void CBaseTexture::CloneTexture(CBaseTexture* pSourceBaseTexture)
{
   CTexture* pThis = (CTexture*)this;
   CTexture* pSource = (CTexture*)pSourceBaseTexture;

   // Free memory for current texture
   pThis->FreeMemory();

   // Copy GcmTexture structure over (IMPORTANT: This needs fixing up!)
   pThis->mTexture = pSource->mTexture;

   // Copy allocation size
   pThis->mMemoryAllocationSize = pSource->mMemoryAllocationSize;

   // Copy memory for texture
   {
      // Create new matching allocation
      int const allocateFromSystemMemory = (pSource->mAdditionalFlags & CTexture::kAF_SystemMemory);
      pThis->mMemory = RenderBackend()->AllocFixed(pThis->mMemoryAllocationSize, 128, allocateFromSystemMemory ? kRM_System : kRM_Video);
      BPE_VERIFY(pThis->mMemory, false, "Couldn't allocate memory!");

      // Patch up offset in GcmTexture
      cellGcmAddressToOffset(pThis->mMemory->mpAddress, &pThis->mTexture.offset);
      
      // Patch up location in GcmTexture
      pThis->mTexture.location = allocateFromSystemMemory ? CELL_GCM_LOCATION_MAIN : CELL_GCM_LOCATION_LOCAL;

      // Copy data accross from source texture.
      uint32 const destPtr = (uint32)pThis->mMemory->mpAddress;
      uint32 const sourcePtr = (uint32)pSource->mMemory->mpAddress;
      gpRenderBackend->SPU_SyncDMA(destPtr, sourcePtr, pThis->mMemoryAllocationSize);
   }

   // Copy remaining parameters accross (IMPORTANT: mIsTiled is used during call to FreeMemory above!)
   pThis->mIsTiled = pSource->mIsTiled;
   pThis->mAAType = pSource->mAAType;
   pThis->mMaxAniso = pSource->mMaxAniso;

   // This copies all parameters defined in CBaseTexture
   CloneBaseTexture(pSource);
}