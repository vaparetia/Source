//----------------------------------------------------------------------------
// VTACTexture.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "VTACTexture.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Memory/VTAPhysContMemory.h"

#include <gxm.h>
#include <gxm/transfer.h>
#include <gxt.h>
#include <scejpeg.h>
#include <scepng.h>
#include <kernel/sysmem.h>
#include <ExtLibraries/libPNG/png.h>

//#define GATHER_TIMING_STATS 1
//#define GATHER_OVERALL_TIMING_STATS 1

//----------------------------------------------------------------------------

namespace
{

   static SceGxmColorFormat const gColorFormatMapping[] =
   {
      SCE_GXM_COLOR_FORMAT_A8R8G8B8,
      SCE_GXM_COLOR_FORMAT_A8R8G8B8, //X8R8G8B8
      
      SCE_GXM_COLOR_FORMAT_F16F16F16F16_ABGR,
      SCE_GXM_COLOR_FORMAT_F32_R,

      SCE_GXM_COLOR_FORMAT_A8R8G8B8,

      SCE_GXM_COLOR_FORMAT_A8R8G8B8, // compressed ones won't work right for this
      SCE_GXM_COLOR_FORMAT_A8R8G8B8,
      SCE_GXM_COLOR_FORMAT_A8R8G8B8,
      
      SCE_GXM_COLOR_FORMAT_F32F32_GR, // float4 isn't supported

      SCE_GXM_COLOR_FORMAT_U8_R,
      SCE_GXM_COLOR_FORMAT_A8R8G8B8, // D24FS8

      SCE_GXM_COLOR_FORMAT_A8R8G8B8, // YVU420

      SCE_GXM_COLOR_FORMAT_A8B8G8R8,
   };

   static SceGxmTextureFormat const gTextureFormatMapping[] =
   {
      SCE_GXM_TEXTURE_FORMAT_A8R8G8B8,
      SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1RGB, //X8R8G8B8

      SCE_GXM_TEXTURE_FORMAT_F16F16F16F16_ABGR, // CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT,
      SCE_GXM_TEXTURE_FORMAT_F32_RRRR, // CELL_GCM_TEXTURE_X32_FLOAT,

      // reinterpret depth texture as ARGB8 so we can extract floating point depth in the shader
      SCE_GXM_TEXTURE_FORMAT_X8U24_SD, //CELL_GCM_TEXTURE_A8R8G8B8, //CELL_GCM_TEXTURE_DEPTH24_D8,     

      SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP_1BGR, //CELL_GCM_TEXTURE_COMPRESSED_DXT1,
      SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP_ABGR, //CELL_GCM_TEXTURE_COMPRESSED_DXT23,
      SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP_ABGR, //CELL_GCM_TEXTURE_COMPRESSED_DXT45,

      SCE_GXM_TEXTURE_FORMAT_F32F32_GR, // CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT, <-- INVALID

      SCE_GXM_TEXTURE_FORMAT_U8_RRRR, // CELL_GCM_TEXTURE_B8,

      SCE_GXM_TEXTURE_FORMAT_A8R8G8B8, // CELL_GCM_TEXTURE_A8R8G8B8, // kFormat_D24FS8 not officially supported on platform

      SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1,

      SCE_GXM_TEXTURE_FORMAT_A8B8G8R8,
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

      4 * 8,    // kFormat_D24FS8 not officially supported on platform

      24, // YVU420P2 24-bit format

      4 * 8,   // 4x byte
   };
   BPE_CTASSERT(BPE_ARRAY_SIZE(gBitsPerPixel) == CTexture::kFormat_Count);

   static inline bool is_power_of_two( int n )
   {
      // http://www.sjbaker.org/wiki/index.php?title=Cool_Code_list#Test_to_see_if_a_number_is_an_exact_power_of_two
      return ((n&(n-1))==0) ;
   }


   // NOTE - This class is in TextureCookerVTA and in
   // VTACTexture.cpp
   class CVTAPalettizedTextureOptions
   {
   public:
      CVTAPalettizedTextureOptions( int const width, int const height, bool wantsMips )
         : mWidth( width )
         , mHeight( height )
         , mAdjustedWidth( width )
         , mSwizzled( false )
         , mMipCount( 1 )
      {
         static int const skMinimumSwizzleWidth = 8;
         static int const skMinimumSwizzleHeight = 2;

         if ( is_power_of_two( width ) && is_power_of_two( height ) && width >= skMinimumSwizzleWidth && height >= skMinimumSwizzleHeight )
         {
            // If we're big enough and power of two, then we're swizzled
            mSwizzled = true;

            if ( wantsMips )
            {
               int mipWidth = width >> 1;
               int mipHeight = height >> 1;

               while ( mipWidth >= skMinimumSwizzleWidth && mipHeight >= skMinimumSwizzleHeight )
               {
                  ++mMipCount;

                  mipWidth >>= 1;
                  mipHeight >>= 1;
               }
            }
         }

         if ( !mSwizzled && ( mWidth & 0x7 ) )
         {
            // If we're linear strided, let's just undo that
            mAdjustedWidth = ( mWidth + 0x7 ) & ~(0x7);
         }
      }

      int GetMipCount() const { return mMipCount; }
      bool GetSwizzled() const { return mSwizzled; }
      int GetWidth() const { return mWidth; }
      int GetHeight() const { return mHeight; }
      int GetAdjustedWidth() const { return mAdjustedWidth; }
      int BuildByteCountForTextureData() const
      {
         int byteCount = mAdjustedWidth * mHeight;

         for ( int mip = 1; mip < mMipCount; ++mip )
         {
            byteCount += ( mAdjustedWidth >> mip ) * ( mHeight >> mip );
         }

         return byteCount;
      }

   private:
      int mWidth;
      int mHeight;
      int mAdjustedWidth;
      bool mSwizzled;
      int mMipCount;
   };

#ifdef GATHER_OVERALL_TIMING_STATS
   int gNumTexturesLoaded = 0;
   float gTotalTextureLoadTime = 0.0f;
#endif
}

struct SVtaTextureHeader
{
   // This structure is in two files and needs to be identical in both -
   // bp\Source\Tools\AssetToolMPP\TextureCookerVTA.cpp
   // bp\Source\Renderer\Base\Backend\VTA\VTACTexture.cpp

   uint32 mFileId;
   uint32 mFormatType;
   uint32 mMinRGBA;
   uint32 mMaxRGBA;
   uint32 mMinRGBANonZero;
   uint32 mGenMips;

   // For jpegs they must be at least 64x64 and must be a multiple of the MCU size which is 16x16 on Vita in order 
   // for the hardware decoder to support it. If images are marked as jpeg and do not fit this requirement, save out
   // a larger jpeg and store the actual width/height in the header so it can be clipped after the texture is loaded.
   uint16 mActualWidth;
   uint16 mActualHeight;

   // AS(JM) - Store additional flags from the texture cooker
   uint32 mAdditionalFlags;

};
BPE_CTASSERT( sizeof( SVtaTextureHeader ) == 8 * sizeof(int) );


// Defines the different types of texture formats supported. This enumeration must match the enumeration defined in 
// TextureCookderVTA.cpp
enum EVtaTextureFormatType
{
   kVTFT_JPG,
   kVTFT_PNG,
   kVTFT_GXT,
   kVTFT_PAL,

   kVTFT_Invalid = -1
};

namespace
{
   enum ESwizzleState
   {
      kSS_None,
      kSS_SwizzledAlready,
      kSS_SwizzleOnCPU
   };

   void* spDecodeBuffer = NULL;
   uint32 sDecodeBufferPos = 0;
   bool sbInitializedJpeg = false;
}

struct SVtaMipmapGenerationInfo
{
   SVtaMipmapGenerationInfo() 
      : mNumBytes( 0 )
      , mNumMips( 0 )
      , mSwizzleState( kSS_None )
      , mPaletteOffset( 0 )
   {
   }

   uint32 mNumBytes;
   uint32 mNumMips;
   ESwizzleState mSwizzleState;
   uint32 mPaletteOffset;
};


//----------------------------------------------------------------------------

static inline uint32 align_value(uint32 align, uint32 inVal)
{
   return (inVal + (align - 1)) & ~(align - 1);
}

//----------------------------------------------------------------------------
static inline bool is_power_of_two_and_nonzero(uint32 val)
{
   return ((val & (val - 1)) == 0) && (val != 0);
}

//----------------------------------------------------------------------------
static inline uint8* alloc_phys_buffer(uint32 numBytes)
{
   // This is primarily just a helper function to verify we have enough memory. Memory is "allocated" by just handing 
   // out chunks of memory during each decoding cycle. gs_DecBufPos is reset to zero when a decoding phase has been 
   // completed.
   BPE_VERIFY((sDecodeBufferPos + numBytes) < NPhysContMem::kPhysContMemAllocSize, false, "Ran out of memory.");
   
   uint8* pMem = (uint8*)spDecodeBuffer + sDecodeBufferPos;
   sDecodeBufferPos += align_value(256, numBytes); // Memory from this buffer must be 256 byte aligned for the jpeg decoder.
   
   return pMem;
}

//----------------------------------------------------------------------------
// Allocates a buffer from the physical buffer to fit the entire texture or 
// the minimum size, whichever is larger
//----------------------------------------------------------------------------
static uint8 * alloc_phys_tex_buffer_with_min_size( SVtaMipmapGenerationInfo const &info, uint32 const minSize )
{
   return alloc_phys_buffer( std::max( info.mNumBytes, minSize ) );
}

//----------------------------------------------------------------------------
// code taken from the gxt conversion texture tools source code.
static uint32_t get_morton_number(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
   uint32_t const logW = 31 - __builtin_clz(width);
   uint32_t const logH = 31 - __builtin_clz(height);
   uint32_t const d    = std::min(logW, logH);
   uint32_t m = 0;
   for(uint32_t i = 0; i < d; ++i) {
      m |= ((x & (1 << i)) << (i + 1)) | ((y & (1 << i)) << i);
   }
   // Append any extra bits
   if(width < height) {
      m |= ((y & ~(width  - 1)) << d);
   } else {
      m |= ((x & ~(height - 1)) << d);
   }

   return m;
}

//----------------------------------------------------------------------------
// code taken from the gxt conversion texture tools source code.
static void swizzle_pixels(uint8_t *tgt, const uint8_t *src, uint32_t width, uint32_t height)
{
   uint32_t mx = get_morton_number(width-1, 0, width, height);
   uint32_t my = get_morton_number(0, height-1, width, height);

   uint32_t oy = 0;
   for(uint32_t y=0; y < height; ++y) {
      uint32_t ox = 0;
      for(uint32_t x=0; x < width; ++x) {
         size_t const tgtOfs = (ox + oy) * 4;
         memcpy(tgt + tgtOfs, src, 4);
         src += 4;
         ox = (ox - mx) & mx;
      }

      oy = (oy - my) & my;
   }
}

//----------------------------------------------------------------------------

static void copy_pixels_async( uint8_t *dst, uint8_t const *src, uint32_t const width, uint32_t const height )
{
   SceGxmTransferFormat const fmt = SCE_GXM_TRANSFER_FORMAT_U8U8U8U8_ABGR;

   // Copies must only be in 1k x 1k chunks for the transfer api
   // So let's chunk it up

   for ( int srcX = 0; srcX < width; srcX += 1024 )
   {
      for ( int srcY = 0; srcY < height; srcY += 1024 )
      {
         int const blockWidth = bpe::min_val( (uint32_t) 1024, width );
         int const blockHeight = bpe::min_val( (uint32_t) 1024, height );

         BPE_CHECK_SCE( sceGxmTransferCopy( blockWidth, blockHeight, 0, 0, SCE_GXM_TRANSFER_COLORKEY_NONE, 
            fmt, SCE_GXM_TRANSFER_LINEAR, src, srcX, srcY, width * 4, 
            fmt, SCE_GXM_TRANSFER_LINEAR, dst, srcX, srcY, width * 4, 
            NULL, 0, NULL ) );
      }
   }

}

//----------------------------------------------------------------------------

static void swizzle_pixels_async( uint8_t *dst, uint8_t const *src, uint32_t const width, uint32_t const height )
{
   SceGxmTransferFormat const fmt = SCE_GXM_TRANSFER_FORMAT_U8U8U8U8_ABGR;

   BPE_CHECK_SCE( sceGxmTransferCopy( width, height, 0, 0, SCE_GXM_TRANSFER_COLORKEY_NONE, fmt, SCE_GXM_TRANSFER_LINEAR,
      src, 0, 0, width * 4, fmt, SCE_GXM_TRANSFER_SWIZZLED, dst, 0, 0, width * 4, NULL, 0, NULL ) );
}

//----------------------------------------------------------------------------

static void downscale_pixels_async( uint8_t *dst, uint8_t const *src, uint32_t const width, uint32_t const height )
{
   SceGxmTransferFormat const fmt = SCE_GXM_TRANSFER_FORMAT_U8U8U8U8_ABGR;

   BPE_CHECK_SCE( sceGxmTransferDownscale( 
      fmt, src, 0, 0, width, height, width * 4, 
      fmt, dst, 0, 0, ( width >> 1 ) * 4 /* dst stride is half src stride */, 
      NULL, 0, NULL ) );
}

//----------------------------------------------------------------------------
static SVtaMipmapGenerationInfo const internal_get_mip_chain_info_for_palettized( SVtaTextureHeader const &header )
{
   CVTAPalettizedTextureOptions palOptions( header.mActualWidth, header.mActualHeight, header.mGenMips );

   SVtaMipmapGenerationInfo outInfo;

   outInfo.mNumMips = palOptions.GetMipCount();
   outInfo.mSwizzleState = palOptions.GetSwizzled() ? kSS_SwizzledAlready : kSS_None;

#if 0
   if ( !IsPowerOf2( header.mActualWidth) || !IsPowerOf2( header.mActualHeight ) )
   {
      outInfo.mIsSwizzled = false;
   }
   else
   {
      outInfo.mIsSwizzled = true;
   }
#endif

   int const totalBitmapDataSize = palOptions.BuildByteCountForTextureData();
   int const alignedBitmapDataSize = align_value( SCE_GXM_PALETTE_ALIGNMENT, totalBitmapDataSize );

   outInfo.mPaletteOffset = alignedBitmapDataSize;

   outInfo.mNumBytes = 
      alignedBitmapDataSize +
      256 * 4 /* palette size */;

   return outInfo;
}

//----------------------------------------------------------------------------
static SVtaMipmapGenerationInfo const internal_get_mip_chain_info( SVtaTextureHeader const &header )
{
   // Textures are always rgba8888
   SVtaMipmapGenerationInfo outInfo;

   if ( !is_power_of_two_and_nonzero( header.mActualWidth) || !is_power_of_two_and_nonzero( header.mActualHeight ) )
   {
      outInfo.mNumMips = 1;
      outInfo.mSwizzleState = kSS_None;
   }
   else 
   {
      outInfo.mSwizzleState = kSS_SwizzleOnCPU;
      
      if ( header.mGenMips )
      {
         uint32 const logW = 31 - __builtin_clz(header.mActualWidth);
         uint32 const logH = 31 - __builtin_clz(header.mActualHeight);

         outInfo.mNumMips = std::min(logW, logH) + 1;
      }
      else
      {
         outInfo.mNumMips = 1;
      }
   }

   // Width and height are modified in the loop below
   uint32 width = header.mActualWidth;
   uint32 height = header.mActualHeight;

   for (uint32 i = 0; i < outInfo.mNumMips; i++)
   {
      outInfo.mNumBytes += (width * height * 4);

      width = (width > 1) ? ( width >> 1 ) : 1;
      height = (height > 1) ? ( height >> 1 ) : 1;
   }

   return outInfo;
}

//----------------------------------------------------------------------------
void CTexture::InternalCreateGxmTexture(CTexture* pTex, SVtaTextureHeader const &vtaHeader, SVtaMipmapGenerationInfo const &mipmapInfo, uint8 * pPixelData, uint8 const *pPaletteData )
{
#ifdef GATHER_TIMING_STATS
   CTimeBlock tb( "Create GXM Texture" );
#endif
   static const int kBytesPerPixel = 4;

   // Allocate the resultant bytes in the textures
   pTex->mMemoryAllocationSize = mipmapInfo.mNumBytes;
   pTex->mMemory = RenderBackend()->AllocFixed(
      pTex->mMemoryAllocationSize, 
      pPaletteData ? SCE_GXM_PALETTE_ALIGNMENT : SCE_GXM_TEXTURE_ALIGNMENT, 
      kRM_Video, kRM_System);

   uint8 *pPaletteInVideoMemory;
   uint8 *pTextureInVideoMemory;

   pTextureInVideoMemory = pTex->mMemory->mpAddress;

   if ( pPaletteData )
   {
      // Palette lives at the end of memory
      pPaletteInVideoMemory = pTex->mMemory->mpAddress + pTex->mMemoryAllocationSize - 256 * 4;
   }
   else
   {
      pPaletteInVideoMemory = NULL;
   }

   if ( pPaletteInVideoMemory )
   {
      // Note that swizzling should already be handled here, so all we need
	   // to do is memcpy stuff.
	  
      memcpy( pTextureInVideoMemory, pPixelData, mipmapInfo.mNumBytes - 256 * 4 );
      memcpy( pPaletteInVideoMemory, pPaletteData, 256 * 4 );
   }
   else if ( mipmapInfo.mSwizzleState == kSS_None )
   {
      // Not swizzled, do a standard copy
      copy_pixels_async( pTextureInVideoMemory, pPixelData, vtaHeader.mActualWidth, vtaHeader.mActualHeight );
   }
   else
   {
      // Swizzle the top-most miplevel over
      swizzle_pixels_async( pTextureInVideoMemory, pPixelData, vtaHeader.mActualWidth, vtaHeader.mActualHeight );

      // Now generate every miplevel and swizzle them over
      if ( mipmapInfo.mNumMips > 1 )
      {
         // We mip within the "current linear mip" space, and write out to the next swizzled one

         uint8 *pCurrentLinearMip = pPixelData;
         uint8 *pCurrentSwizzledMip = pTextureInVideoMemory;
         int currentWidth = vtaHeader.mActualWidth;
         int currentHeight = vtaHeader.mActualHeight;

         for ( int nextMipLevel = 1; nextMipLevel < mipmapInfo.mNumMips; ++nextMipLevel )
         {
            int const currentMipLevelSize = currentWidth * currentHeight * kBytesPerPixel;
            uint8 *pNextLinearMip = pCurrentLinearMip + currentMipLevelSize;
            uint8 *pNextSwizzledMip = pCurrentSwizzledMip + currentMipLevelSize;
            int const nextMipWidth = currentWidth >> 1;
            int const nextMipHeight = currentHeight >> 1;

            // First downsample to the next mip level
            downscale_pixels_async( pNextLinearMip, pCurrentLinearMip, currentWidth, currentHeight );

            // Then swizzle downsampled result
            swizzle_pixels_async( pNextSwizzledMip, pNextLinearMip, nextMipWidth, nextMipHeight );

            pCurrentLinearMip = pNextLinearMip;
            pCurrentSwizzledMip = pNextSwizzledMip;
            currentWidth = nextMipWidth;
            currentHeight = nextMipHeight;
         }
      }
   }

   // Only two texture formats, palette (P8 ABGR) or not (ABGR U8U8U8U8)
   SceGxmTextureFormat const texFormat = pPaletteData ? SCE_GXM_TEXTURE_FORMAT_P8_ABGR : SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR;

   if (mipmapInfo.mSwizzleState != kSS_None )
   {
      BPE_CHECK_SCE( sceGxmTextureInitSwizzled(&pTex->mTexture, pTextureInVideoMemory, texFormat,
         vtaHeader.mActualWidth, vtaHeader.mActualHeight, mipmapInfo.mNumMips) );
      
      sceGxmTextureSetMinFilter(&pTex->mTexture, SCE_GXM_TEXTURE_FILTER_LINEAR);
      if (mipmapInfo.mNumMips > 1 )
      {
         sceGxmTextureSetMipFilter(&pTex->mTexture, SCE_GXM_TEXTURE_MIP_FILTER_ENABLED);
      }
   }
   else
   {
      if ( (vtaHeader.mActualWidth & 7) && ( !pPaletteData ) )
      {
         BPE_CHECK_SCE( sceGxmTextureInitLinearStrided(&pTex->mTexture, pTextureInVideoMemory, texFormat,
            vtaHeader.mActualWidth, vtaHeader.mActualHeight, vtaHeader.mActualWidth * 4) );
      }
      else
      {
         // Linear textures have an 8 texel implicit stride when stride is not specified.
         BPE_CHECK_SCE( sceGxmTextureInitLinear(&pTex->mTexture, pTextureInVideoMemory, texFormat, 
            vtaHeader.mActualWidth, vtaHeader.mActualHeight, 1) );
         sceGxmTextureSetMinFilter(&pTex->mTexture, SCE_GXM_TEXTURE_FILTER_LINEAR);
      }
   }

   if ( pPaletteData )
   {
      BPE_CHECK_SCE( sceGxmTextureSetPalette( &pTex->mTexture, pPaletteInVideoMemory ) );
   }

   sceGxmTextureSetMagFilter(&pTex->mTexture, SCE_GXM_TEXTURE_FILTER_LINEAR);
}

//----------------------------------------------------------------------------

bool CTexture::InternalLoadFromPngLibPng(CTexture* pTex,  SVtaTextureHeader const &vtaHeader, void const * pMemory, uint32 const numBytes)
{
   struct PngReadHelper
   {
      unsigned char const * m_pBytes;
      unsigned int m_readPos;

      PngReadHelper(void const* pBytes) 
         : m_pBytes(reinterpret_cast<unsigned char const *>( pBytes ) )
         , m_readPos(0) 
      { 
      }

      inline bool verify() 
      { 
         return !png_sig_cmp((png_bytep)m_pBytes, 0, 8);
      }

      static void read(png_structp pPng, png_bytep pDst, png_size_t numBytes)
      {
         assert(pPng->io_ptr);
         PngReadHelper* readHelper = (PngReadHelper*)pPng->io_ptr;
         memcpy(pDst, &readHelper->m_pBytes[readHelper->m_readPos], numBytes);
         readHelper->m_readPos += numBytes;
      }
   };

   png_structp pPngStruct;
   png_infop pPngInfo;
   png_uint_32 pngWidth = 0;
   png_uint_32 pngHeight = 0;
   uint8* pDecodedPixels = NULL;
   SVtaMipmapGenerationInfo const mipmapInfo = internal_get_mip_chain_info( vtaHeader );

   {
#ifdef GATHER_TIMING_STATS
      CTimeBlock timer("PNG DECODING");
#endif

      int colorType;
      int bpp = 0;

      pPngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      assert(pPngStruct);

      pPngInfo = png_create_info_struct(pPngStruct);
      assert(pPngInfo);

      PngReadHelper readHelper(pMemory);
      BPE_VERIFY(readHelper.verify(), false, "Not a png file.");

      png_set_read_fn(pPngStruct, &readHelper, &PngReadHelper::read);

      png_read_info(pPngStruct, pPngInfo);
      uint32 res = png_get_IHDR(pPngStruct, pPngInfo, &pngWidth, &pngHeight, &bpp, &colorType, NULL, NULL, NULL);
      BPE_VERIFY(res, false, "Error getting png file info.");
      BPE_VERIFY(colorType == PNG_COLOR_TYPE_RGB_ALPHA || colorType == PNG_COLOR_TYPE_RGB, false, "Unexpected png image type.");
      BPE_VERIFY(pngWidth == vtaHeader.mActualWidth, false, "Width of image does not match expected value.");
      BPE_VERIFY(pngHeight == vtaHeader.mActualHeight, false, "Height of image does not match expected value.")

      // Allocate an rgba texture. If the png contains rgb data, copy that from the row and add 0xff as the alpha.
      uint32 const pixelSize = (colorType == PNG_COLOR_TYPE_RGB) ? 3 : 4;
      pDecodedPixels = alloc_phys_tex_buffer_with_min_size( mipmapInfo, pngWidth * pngHeight * 4);

      if (pixelSize == 4)
      {
            png_set_tRNS_to_alpha(pPngStruct);
         }

         uint8* pRowDst = pDecodedPixels;
         uint32 const dstRowStride = pngWidth * 4;

         if ( pixelSize == 3 )
         {
            // If this is an RGB texture, translate to RGBA (with 128 as A)

            uint8* pRowSrc = alloc_phys_buffer(pngWidth * pixelSize);

            for (uint32 y = 0; y < pngHeight; y++)
            {
               png_read_row(pPngStruct, pRowSrc, NULL);

               uint8* pSrcPixel = pRowSrc;
               for (uint32 x = 0; x < pngWidth; x++)
               {
                  *pRowDst++ = *pSrcPixel++;
                  *pRowDst++ = *pSrcPixel++;
                  *pRowDst++ = *pSrcPixel++;
                  *pRowDst++ = 128;
               }
            }
         }
         else
         {
            for (uint32 y = 0; y < pngHeight; y++)
         {
            png_read_row(pPngStruct, pRowDst, NULL);

            pRowDst += dstRowStride;
         }
      }

      png_destroy_read_struct(&pPngStruct, &pPngInfo, NULL);
   }

   BPE_VERIFY( pngWidth == vtaHeader.mActualWidth, false, "Width and PNG Width don't match" );
   BPE_VERIFY( pngHeight == vtaHeader.mActualHeight, false, "Height and PNG Height don't match" );

   InternalCreateGxmTexture(pTex, vtaHeader, mipmapInfo, pDecodedPixels, NULL);

   return true;
}

//----------------------------------------------------------------------------

bool CTexture::InternalLoadFromJpeg(CTexture* pTex, SVtaTextureHeader const &vtaHeader, const void* pMemory, uint32 const numBytes)
{
   // These constants are taken from the Vita documentation and are due to limitations of the hardware jpeg decoder.
   const uint16 kMaxJpegWidth = 2032;
   const uint16 kMinJpegWidth = 64;
   const uint16 kMaxJpegHeight = 1088;
   const uint16 kMinJpegHeight = 64;

   uint8* pTexBuf = NULL;

   SVtaMipmapGenerationInfo const mipmapInfo = internal_get_mip_chain_info( vtaHeader );

   {
#ifdef GATHER_TIMING_STATS
      CTimeBlock timer("JPEG DECODING");
#endif

      // Need to copy the incoming memory into our decoder buffer first since the decoder buffer is physically continuous.
      uint32 jpegDataSize = numBytes;
      uint8* pJpegDataStart = alloc_phys_buffer(jpegDataSize);
      memcpy(pJpegDataStart, pMemory, numBytes);

      SceJpegOutputInfo jpegInfo;
      SceInt32 res = sceJpegGetOutputInfo(pJpegDataStart, jpegDataSize, SCE_JPEG_NO_CSC_OUTPUT, SCE_JPEG_MJPEG_WITH_DHT, &jpegInfo);
      BPE_VERIFY(res == SCE_OK, false, "Error getting jpeg information.");

      // Make sure this is just a basic jpeg and that it fits the size requirements. Note, the size requirements should have 
      // been checked and fixed when cooking out the jpeg.
      BPE_VERIFY(jpegInfo.imageWidth >= kMinJpegWidth && jpegInfo.imageWidth <= kMaxJpegWidth, false, "Jpeg dimensions are not supported.");
      BPE_VERIFY(jpegInfo.imageHeight >= kMinJpegHeight && jpegInfo.imageHeight <= kMaxJpegHeight, false, "Jpeg dimensions are not supported.");
      BPE_VERIFY(jpegInfo.coefBufferSize == 0, false, "Unsupported jpeg format.");

      // Partition a section of our decode buffer for the decoded ycbcr data.
      uint8* pYCbCrStart = alloc_phys_buffer(jpegInfo.outputBufferSize);
      pTexBuf = alloc_phys_tex_buffer_with_min_size(mipmapInfo, jpegInfo.imageWidth * jpegInfo.imageHeight * 4);

      SceInt32 bytesDecoded = sceJpegDecodeMJpegYCbCr(pJpegDataStart, jpegDataSize, pYCbCrStart, jpegInfo.outputBufferSize, SCE_JPEG_MJPEG_WITH_DHT, NULL, 0);
      if (bytesDecoded < 0)
      {
         // TODO: Some very rare case causes the decoded to fail with an internal decoder error. I'm not sure if this is a bug in the jpeg
         // libs or not as the jpeg looks fine and it is an indeterministic crash.. For now, fill with bright purple so it stands out but
         // don't crash the game..
         printf("JPEG DECODE ERROR: Error Code: %x\n", bytesDecoded);
         uint32* pPixelStart = (uint32*)pTexBuf;
         for (uint32 y = 0; y < jpegInfo.imageHeight; y++)
         {
            for (uint32 x = 0; x < jpegInfo.imageWidth; x++)
            {
               *pPixelStart++ = 0xff00ffff;
            }
         }

         //BPE_VERIFY(bytesDecoded > 0, false, "Error decoding jpeg data.");
      }
      else
      {
         // Once decoding and color space conversion are done we need to clip the results to the actual size of the texture.
         // Partition a section of our decode buffer for the color converted rgba data.      
         res = sceJpegMJpegCsc(pTexBuf, pYCbCrStart, bytesDecoded, jpegInfo.imageWidth, SCE_JPEG_PIXEL_RGBA8888, jpegInfo.colorSpace & 0xffff);
         BPE_VERIFY(res == SCE_OK, false, "Error converting jpeg data to rgba color space.");
      }

      // Now we can allocate memory for the texture and copy it over taking into account the possibility of extra data due 
      // to MCU alignment.
      // Copy over in-place
      if (jpegInfo.imageWidth != vtaHeader.mActualWidth || jpegInfo.imageHeight != vtaHeader.mActualHeight)
      {
         uint32 srcStride = jpegInfo.imageWidth * 4;
         uint32 dstStride = vtaHeader.mActualWidth * 4;
         uint8* pSrcPixels = pTexBuf + srcStride;
         uint8* pDstPixels = pTexBuf + dstStride;
         for (uint32 y = 1; y < vtaHeader.mActualHeight; y++)
         {
            // assuming memcpy can deal with overlapping src and dst
            memcpy(pDstPixels, pSrcPixels, dstStride);
            pSrcPixels += srcStride;
            pDstPixels += dstStride;
         }
      }
      
      // When we are loading jpeg images, the original images may have had an alpha channel with a constant alpha value.
      // Copy that constant alpha value into the alpha channel. For images without alpha min/max rgba should have an alpha
      // value of 255.
      BPE_VERIFY((vtaHeader.mMinRGBA & 0xff) == (vtaHeader.mMaxRGBA & 0xff), false, "Should have the same alpha value.");
      uint8 alphaValue = vtaHeader.mMinRGBA & 0xff;
      //printf("ALPHAVALUE: %d\n", alphaValue);
      uint8* pPixels = pTexBuf + 3;
      for (uint32 y = 0; y < vtaHeader.mActualHeight; y++)
      {
         for (uint32 x = 0; x < vtaHeader.mActualWidth; x++)
         {
            *pPixels = alphaValue;
            pPixels += 4;
         }
      }
   }

   InternalCreateGxmTexture(pTex, vtaHeader, mipmapInfo, pTexBuf, NULL );
   
   return true;
}

//----------------------------------------------------------------------------

bool CTexture::InternalLoadFromPng(CTexture* pTex,  SVtaTextureHeader const &vtaHeader, const unsigned char* pMemory, uint32 const numBytes)
{
   int pngWidth;
   int pngHeight;
   int pngFormat;
   int pngStrFormat;
   int decodeSize;
   uint8* pDecodedPixels = NULL;
   SVtaMipmapGenerationInfo const mipmapInfo = internal_get_mip_chain_info( vtaHeader );
   {
#ifdef GATHER_TIMING_STATS
      CTimeBlock timer("PNG DECODING");
#endif

      decodeSize = scePngGetOutputInfo(pMemory, numBytes, &pngWidth, &pngHeight, &pngFormat, &pngStrFormat);
      BPE_VERIFY(decodeSize > 0, false, "Error determining png format information.");
      BPE_VERIFY(pngFormat == SCE_PNG_FORMAT_RGBA8888, false, "Image is not in rgba format.");
      BPE_VERIFY(pngWidth == vtaHeader.mActualWidth, false, "Image width does not match source");
      BPE_VERIFY(pngHeight == vtaHeader.mActualHeight, false, "Image height does not match source");

      pDecodedPixels = alloc_phys_buffer(decodeSize);
      uint32 res = scePngDec(pDecodedPixels, decodeSize, pMemory, numBytes, &pngWidth, &pngHeight, &pngFormat);
      BPE_VERIFY(res > 0, false, "Unable to get decode png source.");
   }
   
   InternalCreateGxmTexture(pTex, vtaHeader, mipmapInfo, pDecodedPixels, NULL);

   return true;
}

//----------------------------------------------------------------------------

static void phycont_decode_begin()
{

   // VERY IMPORTANT to set this to zero before we start a decode cycle else we may run out of memory due to what
   // amounts to a memory leak. We use chunks of the buffer during a single decoding phase instead of worrying about 
   // allocating and freeing blocks in the middle of a decoding session.
   sDecodeBufferPos = 0;
   spDecodeBuffer = NPhysContMem::LockPhysContMem(NPhysContMem::OWNER_TYPE_TEXTURE_DECOMPRESSION, NULL, NULL);

   sceGxmMapMemory( spDecodeBuffer, NPhysContMem::kPhysContMemAllocSize, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE ); 
}

//----------------------------------------------------------------------------

static void phycont_decode_end()
{
#ifdef GATHER_TIMING_STATS
   CTimeBlock tuberculosis( "GXM TRANSFER FINISH" );
#endif
   BPE_CHECK_SCE( sceGxmTransferFinish() );

   sceGxmUnmapMemory( spDecodeBuffer );

   NPhysContMem::UnlockPhysContMem();
   spDecodeBuffer = NULL;
}

//----------------------------------------------------------------------------

void CTexture::CacheWidthAndHeightFromGxm()
{
   mWidth = sceGxmTextureGetWidth( &mTexture );
   mHeight = sceGxmTextureGetHeight( &mTexture );
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseTexture::Create(void *pAssetMemory, uint32 const size)
{
#ifdef GATHER_OVERALL_TIMING_STATS
   CStopWatch sw;
#endif
   unsigned int const kFileID = 'ARM2';
   CTexture* pTexture = new CTexture(CBaseTexture::kFormat_Invalid, CBaseTexture::kType_Normal, CBaseTexture::kUsage_Texture);

   // Allocate our decoder memory if it hasn't been allocated already.
   if (!sbInitializedJpeg)
   {
      sbInitializedJpeg = true;

      SceJpegMJpegInitParam initParam;
      initParam.size = sizeof(SceJpegMJpegInitParam);
      initParam.maxSplitDecoder = 0;
      initParam.option = SCE_JPEG_MJPEG_INIT_OPTION_LPDDR2_MEMORY;

      SceInt32 res = sceJpegInitMJpegWithParam(&initParam);
      BPE_VERIFY(res == SCE_OK, false, "Error initializing jpeg decoder.");
   }

   // Check for the custom Armature texture format which is an embedded jpeg or png file. If present load it if not, 
   // assume it is a pre-cooked Vita specific (gxt) texture.
   SVtaTextureHeader const * pTexHeader = (SVtaTextureHeader const *)pAssetMemory;

   pTexture->mMinRGBA = pTexHeader->mMinRGBA;
   pTexture->mMaxRGBA = pTexHeader->mMaxRGBA;
   pTexture->mMinRGBANonZero = pTexHeader->mMinRGBANonZero;
   pTexture->mAdditionalFlags = pTexHeader->mAdditionalFlags;

   BPE_VERIFY( pTexHeader->mFileId == kFileID, false, "Invalid texture!" );

   void const *pTextureData = pTexHeader + 1;

   if (pTexHeader->mFormatType == kVTFT_JPG)
   {
#ifdef GATHER_TIMING_STATS
      CTimeBlock timer("JPEG TEXTURE LOAD");
#endif
      phycont_decode_begin();

      CTexture::InternalLoadFromJpeg(pTexture, *pTexHeader, pTextureData, size - 32);

      phycont_decode_end();
   }
   else if (pTexHeader->mFormatType == kVTFT_PNG)
   {
#ifdef GATHER_TIMING_STATS
      CTimeBlock timer("PNG TEXTURE LOAD");
#endif
      phycont_decode_begin();

      CTexture::InternalLoadFromPngLibPng(pTexture, *pTexHeader, pTextureData, size - 32);

      phycont_decode_end();
   }
   else if ( pTexHeader->mFormatType == kVTFT_PAL )
   {
      SVtaMipmapGenerationInfo mipInfo = internal_get_mip_chain_info_for_palettized( *pTexHeader );

      phycont_decode_begin();

      CTexture::InternalCreateGxmTexture( 
         pTexture, 
         *pTexHeader, 
         mipInfo, 
         reinterpret_cast<uint8 *>( const_cast<void*>(pTextureData) ), 
         reinterpret_cast<uint8 const *>( pTextureData ) + mipInfo.mPaletteOffset );

      phycont_decode_end();
   }
   else if ( pTexHeader->mFormatType == kVTFT_GXT )
   {
      BPE_ASSERT(sceGxtCheckData(pTextureData) == SCE_OK, "Incorrect GXT Texture Data");

      const uint32_t gxtDataSize = sceGxtGetDataSize(pTextureData);
      const uint32_t gxtHeaderSize = sceGxtGetHeaderSize(pTextureData);
      const void *gxtDataSrc = sceGxtGetDataAddress(pTextureData);

      pTexture->mMemoryAllocationSize = gxtDataSize;
      pTexture->mMemory = RenderBackend()->AllocFixed( pTexture->mMemoryAllocationSize, SCE_GXM_TEXTURE_ALIGNMENT, kRM_Video, kRM_System );
      memcpy( pTexture->mMemory->mpAddress, gxtDataSrc, gxtDataSize );

      int sceCode = sceGxtInitTexture(&pTexture->mTexture, pTextureData, pTexture->mMemory->mpAddress, 0);
      BPE_ASSERT(sceCode == SCE_OK, "Gxt Texture not initialized \n");

      sceGxmTextureSetMagFilter( &pTexture->mTexture, SCE_GXM_TEXTURE_FILTER_LINEAR);
      sceGxmTextureSetMinFilter( &pTexture->mTexture, SCE_GXM_TEXTURE_FILTER_LINEAR);
      if(sceGxmTextureGetMipmapCount(&pTexture->mTexture) > 1)
      {
         sceGxmTextureSetMipFilter(&pTexture->mTexture, SCE_GXM_TEXTURE_MIP_FILTER_ENABLED);
      }
   }
   else
   {
      BPE_VERIFY(false, false, "Unsupported texture format.");
   }

   pTexture->mAnisoEnabled = true;

#ifdef GATHER_OVERALL_TIMING_STATS
   float thisTextureTime = sw.GetElapsedTime();
   gTotalTextureLoadTime += thisTextureTime;
   printf("[%d] Total texture load time = %f (%f)\n", gNumTexturesLoaded++, gTotalTextureLoadTime, thisTextureTime);
#endif

   static_cast<CTexture *>( pTexture )->CacheWidthAndHeightFromGxm();

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

   return pTexture;
}


//----------------------------------------------------------------------------

static void get_msaa_multipliers( CBaseTexture::EAntiAliasType usesAA, int *pWidthMult, int *pHeightMult )
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
   get_msaa_multipliers( aaFlags, &widthMult, &heightMult );

   int const finalWidth = _inputWidth * widthMult;
   int const finalHeight = _inputHeight * heightMult;
   int pitch = 0;
   int size = 0;

   switch( usage )
   {
   case kUsage_RenderTarget:
      {
         pitch = finalWidth;

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

         if (pCreateTextureParams->mStride)
            pitch = pCreateTextureParams->mStride;

         int height = finalHeight;

         pTexture->mMemoryAllocationSize = pitch * height;

         pTexture->mAAType = uint32( aaFlags );
         pTexture->mRTIndex = RenderBackend()->FindRenderTarget(finalWidth, finalHeight, pCreateTextureParams->mNumScenes, pCreateTextureParams->mMSAATrick, pCreateTextureParams->mXTiles, pCreateTextureParams->mYTiles);
      }
      break;
   case kUsage_DepthBuffer:
      // Only create a 24-8 depth surface
      {
         int height = (finalHeight + SCE_GXM_TILE_SIZEY - 1) & ~(SCE_GXM_TILE_SIZEY - 1);
         
         pitch = 4 * ((finalWidth + SCE_GXM_TILE_SIZEX - 1) & ~(SCE_GXM_TILE_SIZEX - 1));
         if (pCreateTextureParams->mStride)
            pitch = pCreateTextureParams->mStride;
         
         pTexture->mMemoryAllocationSize = pitch * height;

         pTexture->mAAType = uint32( aaFlags );
         pTexture->mRTIndex = RenderBackend()->FindRenderTarget(finalWidth, finalHeight, pCreateTextureParams->mNumScenes, pCreateTextureParams->mMSAATrick, pCreateTextureParams->mXTiles, pCreateTextureParams->mYTiles);
      }
      break;
   case kUsage_Texture:
   case kUsage_TextureLinear:
      {
         if (format == kFormat_YVU420P2_CSC1)
         {
            // Y (luminance) is stored at 8 bits per pixel at the resolution of the texture. VU (chromaticity) 
            // is 16-bit and downsampled vertically and horizontally with each chromaticity value shared by a 2x2 
            // block of luminance values.
            pTexture->mMemoryAllocationSize = finalWidth * finalHeight; // Y
            pTexture->mMemoryAllocationSize += ( (finalWidth >> 1) * (finalHeight >> 1) << 1 ); // VU 
         }
         else
         {
            pitch = finalWidth * 4;
            if (pCreateTextureParams->mStride)
               pitch = pCreateTextureParams->mStride;
            pTexture->mMemoryAllocationSize = pitch * finalHeight;
         }
      }

      break;
   default:
      BPE_VERIFYA( false, "Unknown usage" );
      break;
   }

   void *baseAddress = NULL;

   if ( pCreateTextureParams->mBaseAddress )
   {
      pTexture->mAdditionalFlags |= kAF_RTAlias;
      baseAddress = (void*)pCreateTextureParams->mBaseAddress;
   }
   else
   {
      // Pitch is zero when YUV textures are used.
      if (format == kFormat_YVU420P2_CSC1)
      {
         pTexture->mMemory = RenderBackend()->AllocFixed(pTexture->mMemoryAllocationSize, 128, memory);
      }
      else
      {
         pTexture->mMemory = RenderBackend()->AllocFixed(pitch * finalHeight, 128, memory);
      }

      if( pTexture->mMemory == NULL )
      {
         delete pTexture;
         return NULL;
      }
      baseAddress = pTexture->mMemory->mpAddress;
   }

#if 0
   if ( Levels > 1 )
   {
      sceGxmTextureInitLinear( &pTexture->mTexture,
         baseAddress,
         gTextureFormatMapping[ format ],
         finalWidth,
         finalHeight,
         Levels );
   }
   else
#endif
   {
      if (format == kFormat_YVU420P2_CSC1)
      {
         BPE_CHECK_SCE( sceGxmTextureInitLinear( &pTexture->mTexture,
            baseAddress, 
            gTextureFormatMapping[ format ],
            finalWidth, 
            finalHeight,
            0 ) );
      }
      else
      {
         BPE_CHECK_SCE( sceGxmTextureInitLinearStrided( &pTexture->mTexture,
            baseAddress, 
            gTextureFormatMapping[ format ],
            finalWidth, 
            finalHeight,
            pitch ) );
         BPE_CHECK_SCE( sceGxmTextureSetMagFilter( &pTexture->mTexture, SCE_GXM_TEXTURE_FILTER_LINEAR ) );
      }
   }

   if (usage == kUsage_RenderTarget)
   {
      BPE_CHECK_SCE( sceGxmColorSurfaceInit( &pTexture->mColorSurf,
            gColorFormatMapping[ format ],
            SCE_GXM_COLOR_SURFACE_LINEAR,
            pCreateTextureParams->mMSAATrick ? SCE_GXM_COLOR_SURFACE_SCALE_MSAA_DOWNSCALE : SCE_GXM_COLOR_SURFACE_SCALE_NONE,
            (format == kFormat_A16B16G16R16F) ? SCE_GXM_OUTPUT_REGISTER_SIZE_64BIT : SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
            finalWidth,
            finalHeight,
            pitch / ((format == kFormat_A16B16G16R16F) ? 8 : 4),
            baseAddress ) );            
   }
   else if (usage == kUsage_DepthBuffer)
   {
      BPE_CHECK_SCE( sceGxmDepthStencilSurfaceInit( &pTexture->mDepthSurf,
            SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24,
            SCE_GXM_DEPTH_STENCIL_SURFACE_LINEAR,
            pitch / 4,
            baseAddress,
            NULL ) );
      sceGxmDepthStencilSurfaceSetForceLoadMode(&pTexture->mDepthSurf, (pCreateTextureParams->mLoadStore & kLS_Load) ? SCE_GXM_DEPTH_STENCIL_FORCE_LOAD_ENABLED : SCE_GXM_DEPTH_STENCIL_FORCE_LOAD_DISABLED);
      sceGxmDepthStencilSurfaceSetForceStoreMode(&pTexture->mDepthSurf, (pCreateTextureParams->mLoadStore & kLS_Store) ? SCE_GXM_DEPTH_STENCIL_FORCE_STORE_ENABLED : SCE_GXM_DEPTH_STENCIL_FORCE_STORE_DISABLED);
   }

   static_cast<CTexture *>( pTexture )->CacheWidthAndHeightFromGxm();

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
   //Fall-back
   if(!pTexture)
   {
      pTexture = TryCreate(_inputWidth, _inputHeight, Levels, format, usage, aaFlags, memory == kRM_System ? kRM_Video : kRM_System, pCreateTextureParams);
   }
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
,  mAnisoEnabled( false )
{
   memset(&mTexture, 0, sizeof(SceGxmTexture));

#if 0

   if ( format != kFormat_Invalid )
   {
      sceGxmTextureSetFormat( &mTexture, gTextureFormatMapping[ format ] );
   }

   sceGxmTextureSetMipmapCount( &mTexture, 1 );
#endif
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
//      BPE_ASSERTA( "VITA TODO" );
//      if( mIsTiled )
//         RenderBackend()->FreeTiledRenderTargetImmediate(mMemory);
//      else
         RenderBackend()->FreeImmediate(mMemory);

      mMemory = NULL;
   }
}

//----------------------------------------------------------------------------

void CTexture::SetTexture(int const textureStage) const
{
   if( mMemory || (mAdditionalFlags & CTexture::kAF_RTAlias) )
   {
      BPE_ASSERT( NULL != sceGxmTextureGetData( &mTexture ), "Couldn't get texture data" );

      NVTATextureState::SetTextureData( textureStage, &mTexture );
   }
   else
   {
      BPE_ASSERTA( "Texture data is bad" );
   }
}

//----------------------------------------------------------------------------

void CTexture::Lock(void** pMemory, int * pPitch)
{
   if (mAdditionalFlags & CTexture::kAF_RTAlias)
      *pMemory = sceGxmTextureGetData( &mTexture );
   else
      *pMemory = mMemory->mpAddress;
   *pPitch = sceGxmTextureGetStride( &mTexture );
}

//----------------------------------------------------------------------------

void CTexture::Unlock()
{
}

void CTexture::SetWidth(int width)
{
   sceGxmTextureSetWidth( &static_cast<CTexture *>(this)->mTexture, width );
   mWidth = width;
}

//----------------------------------------------------------------------------

void CTexture::SetHeight(int height)
{
   sceGxmTextureSetHeight( &static_cast<CTexture *>(this)->mTexture, height );
   mHeight = height;
}


//----------------------------------------------------------------------------

int const CBaseTexture::GetDepth() const
{
   return 1; // VITA doesn't have 3D textures?
}

//----------------------------------------------------------------------------

int CBaseTexture::GetMipCount() const
{
   return sceGxmTextureGetMipmapCount( &static_cast<CTexture const *>(this)->mTexture );
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasWidth(EAntiAliasType const type) const
{
   int const width = GetWidth();

   switch ( type )
   {
   case kAA_MSAA2x:
   case kAA_MSAA4x:
      return width >> 1;

   default:
      return width;
   }
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetAntiAliasHeight(EAntiAliasType const type) const
{
   int const height = GetHeight();

   switch ( type )
   {
   case kAA_MSAA4x:
      return height >> 1;

   default:
      return height;
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
      uint8 const *pOldVideoAlloc = pSource->mMemory->mpAddress;
      uint8 const *pOldTextureMemory = reinterpret_cast<uint8 const *>( sceGxmTextureGetData( &pSource->mTexture ) );
      uint8 const *pOldPaletteMemory = reinterpret_cast<uint8 const *>( sceGxmTextureGetPalette( &pSource->mTexture ) );

      // Create new matching allocation
      
      // AS(JM) - Removing "allocateFromSystemMemory" code in same changelist that mAdditionalFlags
      // was being used. That's because before, that would always be false, and it's too close to the 
      // end of MGS to test to see if it works. Plus, this is the only place the flag is checked, so
      // "Clone" wouldn't be doing a proper clone anyway.

//      int const allocateFromSystemMemory = (pSource->mAdditionalFlags & CTexture::kAF_SystemMemory);
      pThis->mMemory = RenderBackend()->AllocFixed(
         pThis->mMemoryAllocationSize, 
         pOldPaletteMemory ? SCE_GXM_PALETTE_ALIGNMENT : SCE_GXM_TEXTURE_ALIGNMENT, 
         kRM_Video, kRM_System );
//         allocateFromSystemMemory ? kRM_System : kRM_Video);
      BPE_VERIFY(pThis->mMemory, false, "Couldn't allocate memory!");

      sceGxmTextureSetData( &pThis->mTexture, pThis->mMemory->mpAddress + ( pOldTextureMemory - pOldVideoAlloc ) );
      if ( pOldPaletteMemory )
      {
         sceGxmTextureSetPalette( &pThis->mTexture, pThis->mMemory->mpAddress + ( pOldPaletteMemory - pOldVideoAlloc ) );
      }

      void *pSourceData;
      int pitch;
      pSource->Lock(&pSourceData, &pitch);
      memcpy(pThis->mMemory->mpAddress, pSourceData, pThis->mMemoryAllocationSize);
   }

   // Copy remaining parameters across (IMPORTANT: mIsTiled is used during call to FreeMemory above!)
   pThis->mIsTiled = pSource->mIsTiled;
   pThis->mAAType = pSource->mAAType;
   pThis->mAnisoEnabled = pSource->mAnisoEnabled;
   pThis->mRTIndex = pSource->mRTIndex;
   pThis->mWidth = pSource->mWidth;
   pThis->mHeight = pSource->mHeight;

   // This copies all parameters defined in CBaseTexture
   CloneBaseTexture(pSource);
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetWidth() const
{
   return static_cast<CTexture const *>(this)->mWidth;
}

//----------------------------------------------------------------------------

int const CBaseTexture::GetHeight() const
{
   return static_cast<CTexture const *>(this)->mHeight;
}

