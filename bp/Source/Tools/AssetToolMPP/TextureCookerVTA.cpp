//----------------------------------------------------------------------------
// TextureCookerVTA.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "stdafx.h"

using namespace System::Runtime::InteropServices;

//----------------------------------------------------------------------------

#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

#include "IL/il.h"
//#include "math.h"
#include "nvtt/nvtt.h"

#include "gxt\gxt_conversion.h"

#include <time.h>
#include "png.h"
#include "TextureCooker.h"

#include <intrin.h>
#include <boost/scoped_ptr.hpp>

//----------------------------------------------------------------------------

using namespace TextureCooker;

//----------------------------------------------------------------------------

namespace
{
   class CDDS : public nvtt::OutputHandler
   {
   public:
      CDDS()
      {
      }

      virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel)
      {
      }

      virtual bool writeData(const void * data, int size)
      {
         mMemory.Put(data, size);
         return true;
      }

   public:
      CGrowableMemoryOutStream   mMemory;
   };

   //----------------------------------------------------------------------------

   void BuildDDS( CTextureToCook const &tex, bool const hasAlpha, CDDS * pDDS )
   {
      ilBindImage(tex.GetTopLevelImageHandle());

      int const fullWidth = ilGetInteger( IL_IMAGE_WIDTH );
      int const fullHeight = ilGetInteger( IL_IMAGE_HEIGHT );
      int const fullDepth = ilGetInteger( IL_IMAGE_DEPTH );

      // Setup input options
      nvtt::InputOptions inputOptions;

      nvtt::TextureType textureType = nvtt::TextureType_2D;

      if( tex.mTextureFaces.size() == 6 )
      {
         textureType = nvtt::TextureType_Cube;
      }
      else if ( fullDepth > 1 )
      {
         textureType = nvtt::TextureType_3D;
      }

      inputOptions.setTextureLayout(textureType, fullWidth, fullHeight, fullDepth );

      // get data out of library into raw buffer
      for( int texFaceIndex = 0; texFaceIndex < tex.mTextureFaces.size(); texFaceIndex++ )
      {
         CTextureFace const &texFace = tex.mTextureFaces[texFaceIndex];
         ILuint topLevelMipHandle = texFace.mMipLevels[0].mImageHandle;

         int const imageWidth = ilGetInteger( IL_IMAGE_WIDTH );
         int const imageHeight = ilGetInteger( IL_IMAGE_HEIGHT );
         int const imageDepth = ilGetInteger( IL_IMAGE_DEPTH );

         std::auto_ptr<char> pRawData( new char [ imageWidth * imageHeight * imageDepth * sizeof(uint32) ] ); 
         ilCopyPixels( 0, 0, 0, imageWidth, imageHeight, imageDepth, IL_BGRA, IL_UNSIGNED_BYTE, pRawData.get() );

         inputOptions.setMipmapData(pRawData.get(), imageWidth, imageHeight, imageDepth, texFaceIndex);
         if (tex.mGenerateMipMaps != kGM_No)
         {         
            inputOptions.setMipmapGeneration(true);   

            // Add additional explicit mip level images if available (ones not specified will be auto generated)
            for (int additionalMipMaps = 1; additionalMipMaps < texFace.mMipLevels.size(); additionalMipMaps++)
            {
               CTexMipMap const &mipMap = texFace.mMipLevels[additionalMipMaps];
               ilBindImage(mipMap.mImageHandle);

               int const mipWidth = ilGetInteger( IL_IMAGE_WIDTH );
               int const mipHeight = ilGetInteger( IL_IMAGE_HEIGHT );
               int const mipDepth = ilGetInteger( IL_IMAGE_DEPTH );
               int const mipLevel = mipMap.mMipMapLevel;

               std::auto_ptr<char> pRawData( new char [ mipWidth * mipHeight * mipDepth * sizeof(uint32) ] ); 
               ilCopyPixels( 0, 0, 0, mipWidth, mipHeight, mipDepth, IL_BGRA, IL_UNSIGNED_BYTE, pRawData.get() );

               inputOptions.setMipmapData(pRawData.get(), mipWidth, mipHeight, mipDepth, texFaceIndex, mipLevel);
            }
         }
         else
         {
            inputOptions.setMipmapGeneration(false);   
         }
      }

      // Setup output options
      nvtt::OutputOptions outputOptions;
      outputOptions.setOutputHandler(pDDS);
      outputOptions.setOutputHeader(true);

      nvtt::Format nvttFormat;

      bool colorDithering = false;
      bool alphaDithering = false;
      bool binaryAlpha = false;

      switch(tex.mFormat)
      {
      case CBaseTexture::kFormat_A8R8G8B8:
         nvttFormat = nvtt::Format_RGBA;
         break;

      case CBaseTexture::kFormat_DXT1:
         nvttFormat = nvtt::Format_DXT1a;
         binaryAlpha = true;
         break;

      case CBaseTexture::kFormat_DXT3:
         nvttFormat = nvtt::Format_DXT3;
         break;

      case CBaseTexture::kFormat_DXT5:
         nvttFormat = nvtt::Format_DXT5;
         break;

      default:
         BPE_VERIFYA(false, "Unsupported texture format");
      }

      // Setup compression options
      nvtt::CompressionOptions compressionOptions;
      compressionOptions.setQuality(nvtt::Quality_Normal);

      compressionOptions.setFormat(nvttFormat);

      uint32 const bitCount   = 32;
      uint32 const redMask    = 0x00FF0000;
      uint32 const greenMask  = 0x0000FF00;
      uint32 const blueMask   = 0x000000FF;
      uint32 const alphaMask  = hasAlpha ? 0xFF000000 : 0;

      compressionOptions.setPixelFormat(bitCount, redMask, greenMask, blueMask, alphaMask);

      // Do compression
      nvtt::Compressor compressor;
      compressor.enableCudaAcceleration(false);
      compressor.process(inputOptions, compressionOptions, outputOptions);
   }

   //----------------------------------------------------------------------------
   enum VtaImageFormatType
   {
      VTA_FORMAT_JPG,
      VTA_FORMAT_PNG,
      VTA_FORMAT_GXT,
      VTA_FORMAT_PAL,

      VTA_FORMAT_COUNT,
      VTA_FORMAT_INVALID = -1,
   };

   static char const *skFormatTypeToString[] = 
   {
      "jpg",
      "png",
      "gxt",
      "pal"
   };

   BPE_CTASSERT( BPE_ARRAY_SIZE( skFormatTypeToString ) == VTA_FORMAT_COUNT );

   //----------------------------------------------------------------------------
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

   //----------------------------------------------------------------------------

   static inline bool is_power_of_two( int n )
   {
      // http://www.sjbaker.org/wiki/index.php?title=Cool_Code_list#Test_to_see_if_a_number_is_an_exact_power_of_two
      return ((n&(n-1))==0) ;
   }

   static void throw_system( char const *cmd )
   {
      int ret = system( cmd );

      if ( ret != 0 )
      {
         printf( "Error %d: cmd failed: %s\n", ret, cmd );
         throw gcnew System::Exception( "Error spawning something" );
      }
   }

   long long get_file_size( char const *fname )
   {
      FILE *f = fopen( fname, "rb" );
      fseek( f, 0, SEEK_END );
      long long size = ftell( f );
      fclose( f );

      return size;
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

   struct SMGSTextureMetadata
   {
      SMGSTextureMetadata()
         : mMGSVersion( 0 )
         , mDisableGxt( false )
         , mFoundXmlMeta( false )
         , mDisableResize( false )
         , mDisableMips( false )
         , mUsedPerfectResize( false )
         , mOriginalWidth( -1 )
         , mOriginalHeight( -1 )
         , mCurrentWidth( -1 )
         , mCurrentHeight( -1 )
      {
      }

      // This member is always valid
      int mMGSVersion;
      bool mDisableGxt;
      bool mFoundXmlMeta;
      bool mDisableResize;
      bool mDisableMips;
      bool mUsedPerfectResize;
      int mOriginalWidth;
      int mOriginalHeight;

      int mCurrentWidth;
      int mCurrentHeight;
   };

//#define DEBUG_METADATA System::Console::WriteLine
#define DEBUG_METADATA __noop
   SMGSTextureMetadata const get_mgs_texture_metadata( CTextureToCook const &tex )
   {
      SMGSTextureMetadata metadata;

      metadata.mMGSVersion = atoi( getenv( "MGS_VERSION" ) );


      if ( metadata.mMGSVersion < 2 || metadata.mMGSVersion > 3 )
      {
         throw gcnew System::Exception( "Invalid MGS_VERSION" );
      }

      // MGS2+3 - disable GXT generation totally
      if ( metadata.mMGSVersion == 2 || metadata.mMGSVersion == 3)
      {
         metadata.mDisableGxt = true;
      }

      if ( tex.mGenerateMipMaps == kGM_No )
      {
         metadata.mDisableMips = true;
      }

      ilBindImage( tex.GetTopLevelImageHandle() );
      metadata.mCurrentWidth = ilGetInteger( IL_IMAGE_WIDTH );
      metadata.mCurrentHeight = ilGetInteger( IL_IMAGE_HEIGHT );

      DEBUG_METADATA( "Normal input path: {0}", gcnew System::String( tex.mNormalImageInputPath.c_str() ) );
      if ( tex.mNormalImageInputPath.c_str() )
      {
         System::String ^xmlFilename = gcnew System::String( tex.mNormalImageInputPath.c_str() );
         xmlFilename = System::IO::Path::ChangeExtension( xmlFilename, "xmlmeta" );

         DEBUG_METADATA( "Trying metadata: {0}", xmlFilename );
         if ( System::IO::File::Exists( xmlFilename ) )
         {
            System::Xml::XmlDocument ^doc = gcnew System::Xml::XmlDocument();
            doc->Load( xmlFilename );

            DEBUG_METADATA( "Found metadata: {0}", xmlFilename );

            metadata.mFoundXmlMeta = true;

            for each ( System::Xml::XmlElement ^node in doc->DocumentElement->SelectNodes( "TagList/Tag" ) )
            {
               System::String ^nodeName = node->InnerText;

               if ( nodeName == "ui" || nodeName == "font" || nodeName == "has_text" )
               {
                  metadata.mDisableResize = true;
               }

               if ( nodeName == "ui" || nodeName == "font" )
               {
                  DEBUG_METADATA( "Disable mips" );

                  metadata.mDisableMips = true;
               }

               if ( nodeName == "no_gxt" ) 
               {
                  metadata.mDisableGxt = true;
               }

               if ( nodeName == "pr_resize" )
               {
                  metadata.mUsedPerfectResize = true;
               }
            }

            System::Xml::XmlElement ^originalWidthEl = (System::Xml::XmlElement ^) doc->DocumentElement->SelectSingleNode( "OriginalWidth" );
            System::Xml::XmlElement ^originalHeightEl = (System::Xml::XmlElement ^) doc->DocumentElement->SelectSingleNode( "OriginalHeight" );

            if ( originalWidthEl != nullptr )
            {
               metadata.mOriginalWidth = System::Int32::Parse( originalWidthEl->InnerText );
            }

            if ( originalHeightEl != nullptr )
            {
               metadata.mOriginalHeight = System::Int32::Parse( originalHeightEl->InnerText );
            }
         }
      }

      return metadata;
   }

   ILuint alloc_temp_smaller_image_if_needed( CTextureToCook const &tex, SMGSTextureMetadata const &metadata )
   {
      // Will either return the top level image handle, or a new handle generated from it

      if ( 
         metadata.mCurrentWidth == -1 || 
         metadata.mCurrentHeight == -1 ||
         metadata.mOriginalWidth == -1 || 
         metadata.mOriginalHeight == -1 ||
         metadata.mDisableResize ||
         !metadata.mFoundXmlMeta )
      {
         return tex.GetTopLevelImageHandle();
      }

      // If we doubled this with perfect
      if ( 
         metadata.mCurrentWidth >= metadata.mOriginalWidth * 2 &&
         metadata.mCurrentHeight >= metadata.mOriginalHeight * 2)
      {
         ILuint newImage = 0;
         ilGenImages( 1, &newImage );
         
         int imageWidth = metadata.mCurrentWidth;
         int imageHeight = metadata.mCurrentHeight;

         if ( metadata.mMGSVersion == 2 || metadata.mMGSVersion == 3 )
         {
            // For MGS2&3, we scale down in both dimensions
            imageWidth >>= 1;
            imageHeight >>= 1;
         }
         else
         {
            // old path for MGS3 to cut down one resolution
            if ( metadata.mCurrentWidth > metadata.mCurrentHeight )
            {
               imageWidth >>= 1;
            }
            else
            {
               imageHeight >>= 1;
            }
         }

         ilBindImage( newImage );
         ilCopyImage( tex.GetTopLevelImageHandle() );
         iluImageParameter( ILU_FILTER, ILU_BILINEAR );
         iluScale( imageWidth, imageHeight, 1 );

         return newImage;
      }
      else
      {
         return tex.GetTopLevelImageHandle();
      }
   }

   void free_temp_smaller_image_if_needed( CTextureToCook const &tex, ILuint generatedHandle )
   {
      if ( generatedHandle != tex.GetTopLevelImageHandle() )
      {
         ilBindImage( tex.GetTopLevelImageHandle() );
         ilDeleteImages( 1, &generatedHandle );
      }
   }

   inline long long get_gzip_size( char const *filename )
   {
      std::string gzcmd = "gzip --keep \"" + std::string( filename ) + "\"";
      long long compressedSize = 0;
      throw_system( gzcmd.c_str() );

      std::string gzFilename = std::string( filename ) + ".gz";

      // The "compressed" size is the one we run through gz
      compressedSize = get_file_size( gzFilename.c_str() );

      _unlink( gzFilename.c_str() );

      return compressedSize;
   }

   class CPngReadHelper
   {
   public:
      explicit CPngReadHelper( char const *filename ) 
         : mFilePointer( fopen( filename, "rb" ) )
      { 
         BPE_VERIFY( mFilePointer != NULL, false, "Couldn't open file in CPngReadHelper" );
      }

      ~CPngReadHelper()
      {
         Dispose();
      }

      void Dispose()
      {
         if ( mFilePointer )
         {
            fclose( mFilePointer );
            mFilePointer = NULL;
         }
      }

      static void Read(png_structp pPng, png_bytep pDst, png_size_t numBytes)
      {
         CPngReadHelper* readHelper = (CPngReadHelper*)pPng->io_ptr;

         fread( pDst, 1, numBytes, readHelper->mFilePointer );
      }
   private:

      FILE *mFilePointer;

      BPE_DISABLE_COPY_AND_ASSIGNMENT( CPngReadHelper );
   };


   static inline uint32_t bsr(uint32_t x)
   {
      unsigned long res;
      _BitScanReverse(&res, x);
      return res;
   }

   static uint32_t getMortonNumber(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
   {
      uint32_t const logW = bsr(width);
      uint32_t const logH = bsr(height);
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

   static void swizzleLevel(uint8_t *tgt, const uint8_t *src, uint32_t width, uint32_t height, uint32_t bpp)
   {
#if 0
      if(bpp == 4)
         return swizzleLevel4bpp(tgt, src, width, height);
#endif
      assert(bpp && (bpp % 8 == 0));

      uint32_t mx = getMortonNumber(width-1, 0, width, height);
      uint32_t my = getMortonNumber(0, height-1, width, height);

      uint32_t pixelSize = bpp / 8;

      uint32_t oy = 0;
      for(uint32_t y=0; y < height; ++y) {
         uint32_t ox = 0;
         for(uint32_t x=0; x < width; ++x) {
            size_t const tgtOfs = (ox + oy) * pixelSize;
            memcpy(tgt + tgtOfs, src, pixelSize);
            src += pixelSize;

            ox = (ox - mx) & mx;
         }

         oy = (oy - my) & my;
      }
   }

   void GeneratePaletteFile( ILuint imageHandle, CTextureToCook const &tex, SMGSTextureMetadata const &metadata, char const *paletteFilename, System::String ^paletteFilenameManaged, long long *pCompressedSize )
   {
      *pCompressedSize = -1;

      // In general, this function will do the following:
      // 1) Write out the image (and mips) to a PNG 
      // 2) Call pngquant to palettize it
      // 3) Read it back in, swizzle it, arrange the mips
      //    and write a file

      ilEnable( IL_FILE_OVERWRITE );
      ilBindImage( imageHandle );

      int const imageHandleWidth = ilGetInteger( IL_IMAGE_WIDTH );
      int const imageHandleHeight = ilGetInteger( IL_IMAGE_HEIGHT );

      // We do some extension changing and such, so let's just be sure that this 
      // one's what we want (and not png or anything) since the filename part is
      // unique, the extension doesn't need to be.
      if ( !paletteFilenameManaged->EndsWith( ".tmp" ) )
      {
         throw gcnew System::Exception( "Palette filename has bad extension" );
      }

      // Figure out how we're making this image
      CVTAPalettizedTextureOptions const texSettings( ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ), !metadata.mDisableMips );

//      printf( "Palette options: mip:%d swiz:%d dismip:%d\n", texSettings.GetMipCount(), texSettings.GetSwizzled(), metadata.mDisableMips );

      char palettePngName[ 255 ];
      char palettePngQuantName[ 255 ];

      // make tmpfile.png
      // we will write the current image to this
      strcpy( palettePngName, paletteFilename );
      strcpy( palettePngName + strlen( palettePngName ) - 4, ".png" );

      // This is the filename that pngquant will output to.
      // tmpfile-fs8.png
      strcpy( palettePngQuantName, paletteFilename );
      strcpy( palettePngQuantName + strlen( palettePngQuantName ) - 4, "-fs8.png" );

      // This file should not be here, but better to be safe than sorry
      _unlink( palettePngName );

      if ( texSettings.GetMipCount() == 1 )
      {
         // If it's just the top mip level, then we save the current image as a PNG 
         // and we're done.
         ILboolean res = ilSave(IL_PNG, const_cast<char *>( palettePngName ));
         BPE_VERIFY(res == IL_TRUE, false, "Unable to write image.");
      }
      else
      {
         // Otherwise we are going to encode the mipmap levels into the PNG image

         // First, we'll generate a "finalImage" and "workImage"
         // finalImage is the image that will be written to the PNG
         ILuint finalImage, workImage;
         ilGenImages( 1, &finalImage );
         ilGenImages( 1, &workImage );

         // Create temporary mipmap chain storage
         // The largest mipmap level is 0, so let's make the storage that big
         boost::scoped_ptr<uint8> pTextureData( new uint8[ imageHandleWidth * imageHandleHeight * 4 ] );

         // Copy the top mip level to both workImage and to our temp texture data
         ilBindImage( workImage );
         ilCopyImage( imageHandle );
         {
            ILinfo imageInfo;
            iluGetImageInfo(&imageInfo);
            if (imageInfo.Origin == IL_ORIGIN_LOWER_LEFT)
            {
               iluFlipImage();
            }
         }

         ilCopyPixels( 0, 0, 0, imageHandleWidth, imageHandleHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, pTextureData.get() );

         // Make the "final" image be height * 2 and have two copies of the top mipmap level
         // The reason that there are two copies, rather than black, is so that the quant algorithm sees
         // as much of the top mipmap's colors as possible when weighting which colors should be in
         // the color palette
         ilBindImage( finalImage );
         ilTexImage( imageHandleWidth, imageHandleHeight * 2, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, NULL );

         ilSetPixels( 0, 0, 0, imageHandleWidth, imageHandleHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, pTextureData.get() );
         ilSetPixels( 0, imageHandleHeight, 0, imageHandleWidth, imageHandleHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, pTextureData.get() );

         // Now that we have the top mip level, let's go through the rest of the mipmap levels
         // and stack them onto the bottom of the topmost mip level
         {
            int accum_y = imageHandleHeight;

            for ( int mip = 1; mip < texSettings.GetMipCount(); ++mip )
            {
               // Resample the image to the mip and pack it on the bottom of the image
               // Note that we're not starting from the prior mipmap, we're resampling from 
               // the top every time. This reduces error accumulation.

               ilBindImage( workImage );
               ilCopyImage( imageHandle );
               {
                  ILinfo imageInfo;
                  iluGetImageInfo(&imageInfo);
                  if (imageInfo.Origin == IL_ORIGIN_LOWER_LEFT)
                  {
                     iluFlipImage();
                  }
               }

               int const thisMipWidth = imageHandleWidth >> mip;
               int const thisMipHeight = imageHandleHeight >> mip;
               iluScale( thisMipWidth, thisMipHeight, 1 );

               ilCopyPixels( 0, 0, 0, thisMipWidth, thisMipHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, pTextureData.get() );
               
               ilBindImage( finalImage );
               ilSetPixels( 0, accum_y, 0, thisMipWidth, thisMipHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, pTextureData.get() );

               accum_y += thisMipHeight;
            }
         }

         // finalImage contains all mipmap levels, stacked
         ilBindImage( finalImage );

         // It contains the levels upside down, so flip it.
         iluFlipImage();

         // Save png file
         ILboolean res = ilSave(IL_PNG, const_cast<char *>( palettePngName ));
         BPE_VERIFY(res == IL_TRUE, false, "Unable to write image.");

         ilBindImage( imageHandle );
         ilDeleteImages( 1, &finalImage );
         ilDeleteImages( 1, &workImage );
      }

      // This file should not be on disk yet, so we're just deleting it to be safe
      _unlink( palettePngQuantName );

      // Quantize miplevels to a palettized file
      throw_system( ( std::string( "pngquant.exe -force 256 \"" ) + palettePngName + "\"" ).c_str() );

      // We are done with the temporary png file
      _unlink( palettePngName );

      // Now we're going to read in the palettized mipmap chain
      // We're using libpng because everything else is horrible.
      png_structp pPngStruct;
      png_infop pPngInfo;
      png_uint_32 pngWidth = 0;
      png_uint_32 pngHeight = 0;
      int colorType = 0;
      int bpp = 0;

      // Time to pay taxes - standard libpng init...
      pPngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      BPE_ASSERT(pPngStruct, "No png struct" );

      pPngInfo = png_create_info_struct(pPngStruct);
      BPE_ASSERT(pPngInfo, "No png struct" );

      // Handy png file reader
      CPngReadHelper readHelper( palettePngQuantName );
      png_set_read_fn(pPngStruct, &readHelper, &CPngReadHelper::Read);

      png_read_info(pPngStruct, pPngInfo);
      uint32 pngRes = png_get_IHDR(pPngStruct, pPngInfo, &pngWidth, &pngHeight, &bpp, &colorType, NULL, NULL, NULL);
      BPE_VERIFY(pngRes, false, "Error getting png file info.");
      BPE_VERIFY(colorType == PNG_COLOR_TYPE_PALETTE,false,"Needs pallette" );

      // If we have a 16-color or less file, tell libpng to make it 8 bit for us.
      if ( bpp < 8 )
      {
         png_set_packing( pPngStruct );
      }

      // PNG has separate color and alpha palettes.  Let's get at the color first.
      png_colorp pColorPalette = NULL;
      uint8 alphaPalette[ 256 ];
      int num_palette = 0;
      png_get_PLTE( pPngStruct, pPngInfo, &pColorPalette, &num_palette );
      BPE_VERIFY( num_palette <= 256, false, "Invalid palette size" );

      // Pre initialize the palette as fully opaque, as the tRNS chunk
      // keeps the smallest amount of alphas possible
      memset( alphaPalette, 0xFF, sizeof( alphaPalette ) );

      // whatever is in the alpha palette
      {
         png_bytep pAlphaPalette = NULL;
         int num_trans = 0;

         uint32 transResult = png_get_tRNS( pPngStruct, pPngInfo, &pAlphaPalette, &num_trans, NULL );
         if ( transResult == PNG_INFO_tRNS )
         {
            BPE_VERIFY( num_trans <= 256, false, "Invalid num_trans value!" );

            memcpy( alphaPalette, pAlphaPalette, num_trans );
         }
      }

      // Interleave the color and alpha palettes into our 256-color palette
      std::vector< uint8 > palette;
      palette.resize( 256 * 4, 0 );
      for ( int paletteEntry = 0; paletteEntry < num_palette; ++paletteEntry )
      {
         uint8 *pWritePalette = &( palette[ paletteEntry * 4 ] );

         // RGBA
         pWritePalette[0] = pColorPalette[paletteEntry].red;
         pWritePalette[1] = pColorPalette[paletteEntry].green;
         pWritePalette[2] = pColorPalette[paletteEntry].blue;
         pWritePalette[3] = alphaPalette[paletteEntry];
      }

      //
      // Pull in all the mipmap levels, swizzle them, and put them
      // back to back into "imageBytes"
      //

      std::vector< uint8 > imageBytes;
      std::vector< uint8 > rowBytes;

      // Note that we're using the "adjusted" width here.  This will allow us to 
      // get a row, then bad it out later through rowbytes.
      rowBytes.resize( texSettings.GetAdjustedWidth(), 0 );
      imageBytes.reserve( texSettings.BuildByteCountForTextureData() /* way bigger than we need, we only need 1.333x, but who cares? */ ); 

      for ( int miplevel = 0; miplevel < texSettings.GetMipCount(); ++miplevel )
      {
         // Note that we're using the "adjusted" width here.  This will allow us to 
         // get a row, then bad it out later through rowbytes.
         int const miplevelWidth = texSettings.GetAdjustedWidth() >> miplevel;
         int const mipLevelHeight = imageHandleHeight >> miplevel;
         int const mipLevelStartBytes = imageBytes.size();

         // Read each row, then only pull mipWidth bytes from the beginning. The rest of the row is 
         // from the original duplicated top-level mip.
         for ( int y = 0; y < mipLevelHeight; ++y )
         {
            png_read_row( pPngStruct, &( rowBytes[0] ), NULL );

            imageBytes.insert( imageBytes.end(), rowBytes.begin(), rowBytes.begin() + miplevelWidth );
         }

         // Swizzle if needed
         if ( texSettings.GetSwizzled() )
         {
            // Copy this miplevel into a tmp structure, then swizzle from there back into the image
            // bytes.
            std::vector< uint8 > tmp( imageBytes.begin() + mipLevelStartBytes, imageBytes.end() );

            swizzleLevel( &( imageBytes[ mipLevelStartBytes ] ), &( tmp[0] ), miplevelWidth, mipLevelHeight, 8 /* bits per pixel */ );
         }
      }

      // bye bye libpng
      png_destroy_read_struct( &pPngStruct, &pPngInfo, NULL );
      readHelper.Dispose(); // Close the file

      // bye bye pngquant output
      _unlink( palettePngQuantName );

      // Now we just write the image and palette bytes.
      // The general image header is enough info for the game to know how much data to take
      // from this.
      {
         CDiskOutputStream paletteStream( paletteFilename );

         paletteStream.Put( &( imageBytes[0] ), imageBytes.size() );
         paletteStream.AddAlignmentPadding( 64 );
         paletteStream.Put( &palette[0], palette.size() );
      }

      // The "compressed" size of this asset is simply a gzip of it
      *pCompressedSize = get_gzip_size( paletteFilename );
   }

   void GeneratePNGFile( ILuint imageHandle, char const *pngFilename, System::String ^pngFilenameManaged, long long *pCompressedSize )
   {
      ilEnable(IL_FILE_OVERWRITE);
      ilBindImage( imageHandle );

      ILboolean res = ilSave(IL_PNG, const_cast<char *>( pngFilename ));
      BPE_VERIFY(res == IL_TRUE, false, "Unable to write image.");

      *pCompressedSize = get_file_size( pngFilename );
   }

   bool TryGenerateGXTFile( CTextureToCook const &tex, SMGSTextureMetadata const &metadata, char const *gxtFilename, long long *pCompressedSize )
   {
      // Early out if we're a weird image type or if we're not originally power of two and not mipmapping
      // Note that the imageInfo contains stuff that was already sized up pow 2, and this process will size it up as well

      *pCompressedSize = -1;

      // If we weren't working from a plain TGA file, then punt
      if ( tex.mNormalImageInputPath.empty() )
      {
         return false;
      }

      if ( metadata.mDisableGxt )
      {
         return false;
      }

      // Do not allow for anything marked as "UI"
      if ( metadata.mDisableResize )
      {
         return false;
      }

      // Note that we're converting the upscaled TGA file.
      // This is kind of a big deal.

      bool const bIsPowerOfTwo = is_power_of_two( tex.mNormalImageWidth ) && is_power_of_two( tex.mNormalImageHeight );

      if ( metadata.mDisableMips && !bIsPowerOfTwo )
      {
         // If we're not generating mipmaps, then the original image needs to be
         // power or two for a GXT file

         return false;
      }

      std::string inputTgaFileName;
      
      if ( bIsPowerOfTwo )
      {
         inputTgaFileName = tex.mNormalImageInputPath;
      }
      else
      {
         // If we're not power of two, then we need to save our upsized TGA file out
         // for the tool to read it.

         inputTgaFileName = gxtFilename + std::string( ".tga" );

         printf( "Saving upsized texture TGA to %s\n", inputTgaFileName.c_str() );

         ilEnable(IL_FILE_OVERWRITE);
         ilBindImage( tex.GetTopLevelImageHandle() );

         ILboolean res = ilSave(IL_TGA, const_cast<char *>( inputTgaFileName.c_str() ) );
         BPE_VERIFY(res == IL_TRUE, false, "Unable to write image.");
      }



      // If we're here, we can generate the pvr and gxt file reasonably easily

      std::string pvrtcmd = 
         "psp2pvrt -i \"" + inputTgaFileName + "\" -o \"" + gxtFilename + ".pvr\" -wa -th 1";

      if ( !metadata.mDisableMips )
      {
         pvrtcmd += " -m -1";
      }

      throw_system( pvrtcmd.c_str() );
      std::string gxtcmd = "psp2gxt -i \"" + std::string( gxtFilename ) + ".pvr\" -o \"" + gxtFilename + "\"";
      throw_system( gxtcmd.c_str() );
      
      _unlink( ( std::string( gxtFilename ) + ".pvr" ).c_str() );

      *pCompressedSize = get_gzip_size( gxtFilename );

      if ( !bIsPowerOfTwo )
      {
         // If we're not power of two, then delete the TGA we generated
         _unlink( inputTgaFileName.c_str() );
      }


      return true;
   }

   bool TryGenerateJPEGFile( CTextureToCook const &tex, ILuint imageHandle, char const *jpegFilename, long long *pCompressedSize ) 
   {
      // JPEG IS DISABLED FOR NOW
	  *pCompressedSize = -1;
      return false;

      ILboolean res;

      ilEnable(IL_FILE_OVERWRITE);
      ilBindImage( imageHandle );

      ILinfo imageInfo;
      iluGetImageInfo(&imageInfo);


      // Use DevIL to down-sample the image and convert it to a memory stream containing either a jpeg image 
      bool hasAlpha = tex.HasAlpha();
      bool hasConstantAlpha = ((tex.mMinRGBA & 0xff) == (tex.mMaxRGBA & 0xff));

      if ( imageInfo.Width > 2032 || imageInfo.Height > 1088 )
      {
         // Too large for Vita JPEG converter

         return false;
      }

      if ( hasAlpha )
      {
         if ( !hasConstantAlpha )
         {
            return false;
         }
      }

      ilSetInteger(IL_JPG_QUALITY, 90);

      // If the format is jpeg and the width/height do not match the requirements for hardware decoding on the Vita
      // adjust the texture size and blit the source texture into the newly created texture.
      uint32 adjustedWidth = __max((imageInfo.Width + 15) & ~15, 64);
      uint32 adjustedHeight = __max((imageInfo.Height + 15) & ~15, 64);
      ILuint alpha1Image = 0;
      ILuint expandedImage = 0;
      if ((adjustedWidth != imageInfo.Width || adjustedHeight != imageInfo.Height) )
      {
         // DevIL blends images with alpha channels when blitting so just set the alpha values to 1 in the source image.
         unsigned char* pPixelData = new unsigned char[imageInfo.Width * imageInfo.Height * sizeof(uint32)];

         ilCopyPixels(0, 0, 0, imageInfo.Width, imageInfo.Height, 1, IL_BGRA, IL_UNSIGNED_BYTE, pPixelData);
         unsigned char* pCurrPixel = pPixelData + 3;
         for (uint32 y = 0; y < imageInfo.Height; y++)
         {
            for (uint32 x = 0; x < imageInfo.Width; x++)
            {
               *pCurrPixel = 255;
               pCurrPixel += 4;
            }
         }

         ilGenImages(1, &alpha1Image);
         ilBindImage(alpha1Image);

         res = ilTexImage(imageInfo.Width, imageInfo.Height, 1, imageInfo.Bpp, imageInfo.Format, imageInfo.Type, NULL);
         BPE_VERIFY(res == IL_TRUE, false, "Error creating alpha1 texture.");
         ilSetPixels(0, 0, 0, imageInfo.Width, imageInfo.Height, 1, imageInfo.Format, imageInfo.Type, pPixelData);
         delete [] pPixelData;

         ilGenImages(1, &expandedImage);
         ilBindImage(expandedImage);
         res = ilTexImage(adjustedWidth, adjustedHeight, 1, imageInfo.Bpp, imageInfo.Format, imageInfo.Type, NULL);
         BPE_VERIFY(res == IL_TRUE, false, "Error creating expanded texture.");

         ilClearColor(0.0f, 0.0f, 0.0f, 1.0f);
         ilClearImage();

         res = ilBlit(alpha1Image, 0, 0, 0, 0, 0, 0, imageInfo.Width, imageInfo.Height, 1);
         BPE_VERIFY(res == IL_TRUE, false, "Error blitting original image.");
      }

      res = ilSave(IL_JPG, const_cast<char *>( jpegFilename ) );
      BPE_VERIFY(res == IL_TRUE, false, "Unable to write image.");

      // Rebind our original image
      ilBindImage( imageHandle );

      // Delete our temporary images
      ilDeleteImages(1, &alpha1Image);
      ilDeleteImages(1, &expandedImage);

      *pCompressedSize = get_file_size( jpegFilename );

      return true;
   }

   static System::String ^get_decent_random_name( System::String ^ext )
   {
      // GetRandomFileName returns a name with random filename and extension
      // Some tools (like gzip!) care about the extension, so this makes
      // the name filename_ext.tmp instead

      System::String ^systemRandom = System::IO::Path::GetRandomFileName();

      // Remove the dot and add a .tmp extension
      return System::String::Format( "{0}.{1}", systemRandom->Replace( '.', '_' ), ext );
   }

   unsigned char* ConvertImage(CTextureToCook const& tex, SVtaTextureHeader& imgHeader, uint32& numBytes )
   {
      BPE_VERIFY(tex.mbCalculatedMinMaxRGBA, false, "Min/Max RGBA values not calculated.");

      ilBindImage(tex.GetTopLevelImageHandle());
      
      {
         ILinfo imageInfo;
         iluGetImageInfo(&imageInfo);
         if (imageInfo.Origin == IL_ORIGIN_LOWER_LEFT)
            iluFlipImage();

         // There shouldn't be any cube maps or volume textures so make sure...
         BPE_VERIFY(imageInfo.Depth == 1, false, "Volume texture...");
         BPE_VERIFY(tex.mTextureFaces.size() == 1, false, "Cube map...");
      }

      uint32 ilFormat = IL_TYPE_UNKNOWN;

      ILboolean res = IL_FALSE;
      ilSetInteger(IL_JPG_QUALITY, 90);
      ilEnable(IL_FILE_OVERWRITE);

      // Use devIL to save the image to a temporary file and then read it back in to store into a memory buffer. 
      // Unfortunately, devIL does not have the ability to save a jpg or png to a memory stream. 

      // Create a random file name since there could be multiple processes running this code.
      System::String^ tempPath = System::IO::Path::GetTempPath();
      System::String^ fileNameJpeg = System::IO::Path::Combine( tempPath, get_decent_random_name( "jpg" ) );
      System::String^ fileNamePng = System::IO::Path::Combine( tempPath, get_decent_random_name( "png" ) );
      System::String^ fileNameGxt = System::IO::Path::Combine( tempPath, get_decent_random_name( "tmp" ) );
      System::String^ fileNamePal = System::IO::Path::Combine( tempPath, get_decent_random_name( "tmp" ) );

      char* pFileNameJpeg = (char*)Marshal::StringToHGlobalAnsi(fileNameJpeg).ToPointer();
      char* pFileNamePng = (char*)Marshal::StringToHGlobalAnsi(fileNamePng).ToPointer();
      char* pFileNameGxt = (char*)Marshal::StringToHGlobalAnsi(fileNameGxt).ToPointer();
      char* pFileNamePal = (char*)Marshal::StringToHGlobalAnsi(fileNamePal).ToPointer();

      long long pngCompressedSize = -1, jpegCompressedSize = -1, gxtCompressedSize = -1, palCompressedSize;

      SMGSTextureMetadata metadata = get_mgs_texture_metadata( tex );

      ILuint generatedImage = alloc_temp_smaller_image_if_needed( tex, metadata );

      bool hasJpeg;
      bool hasGxt;

      GeneratePaletteFile( generatedImage, tex, metadata, pFileNamePal, fileNamePal, &palCompressedSize );
      GeneratePNGFile( generatedImage, pFileNamePng, fileNamePng, &pngCompressedSize );
      hasJpeg = TryGenerateJPEGFile( tex, generatedImage, pFileNameJpeg, &jpegCompressedSize );
      hasGxt = TryGenerateGXTFile( tex, metadata, pFileNameGxt, &gxtCompressedSize );

#if ALLOW_PNG_FILES
      long long const pngOrPalCompressedSize = bpe::min_val( pngCompressedSize, palCompressedSize );
#else
      long long const pngOrPalCompressedSize = palCompressedSize;
#endif

      BPE_VERIFY( pngOrPalCompressedSize != -1, false, "PNG or PAL should have valid size" );

      printf( "%s: png:%d jpg:%d gxt:%d pal:%d\n", 
         tex.mNormalImageInputPath.c_str(),
         int( pngCompressedSize ),
         int( jpegCompressedSize ),
         int( gxtCompressedSize ),
         int( palCompressedSize ) );

      ilBindImage( generatedImage );

      uint32 fileId = 'ARM2';
      imgHeader.mFileId = fileId;
      imgHeader.mMinRGBA = tex.mMinRGBA;
      imgHeader.mMaxRGBA = tex.mMaxRGBA;
      imgHeader.mGenMips = !metadata.mDisableMips;
      imgHeader.mActualWidth = ilGetInteger( IL_IMAGE_WIDTH );
      imgHeader.mActualHeight = ilGetInteger( IL_IMAGE_HEIGHT );
      imgHeader.mAdditionalFlags = tex.mAdditionalFlags;
      imgHeader.mMinRGBANonZero = tex.mMinRGBANonZero;

      free_temp_smaller_image_if_needed( tex, generatedImage );

      char const *pFileName;

      if ( hasGxt && 
           ( !hasJpeg || gxtCompressedSize <= jpegCompressedSize ) &&
           ( gxtCompressedSize <= pngOrPalCompressedSize ) )
      {
         imgHeader.mFormatType = VTA_FORMAT_GXT;
         pFileName = pFileNameGxt;
      }
      else if ( hasJpeg &&
                ( jpegCompressedSize <= pngOrPalCompressedSize ) )
      {
         imgHeader.mFormatType = VTA_FORMAT_JPG;
         pFileName = pFileNameJpeg;
      }
      else
      {
#if ALLOW_PNG_FILES
         imgHeader.mFormatType = VTA_FORMAT_PNG;
         pFileName = pFileNamePng;
#else
         imgHeader.mFormatType = VTA_FORMAT_PAL;
         pFileName = pFileNamePal;
#endif
      }

      // Read in the file to the file buffer
      FILE *fp = fopen( pFileName, "rb" );
      fseek( fp, 0, SEEK_END );
      long long fileSize = ftell( fp );
      fseek( fp, 0, SEEK_SET );
      unsigned char *pFileBuf = new unsigned char[ fileSize ];
      fread( pFileBuf, 1, fileSize, fp );
      fclose( fp );

      _unlink( pFileNameJpeg );
      _unlink( pFileNamePng );
      _unlink( pFileNameGxt );
      _unlink( pFileNamePal );

      Marshal::FreeHGlobal(System::IntPtr(pFileNameJpeg));
      Marshal::FreeHGlobal(System::IntPtr(pFileNamePng));
      Marshal::FreeHGlobal(System::IntPtr(pFileNameGxt));
      Marshal::FreeHGlobal(System::IntPtr(pFileNamePal));

      numBytes = fileSize;
      return pFileBuf;
   }

} // namespace


namespace TextureCooker
{

   bool WriteTextureVTA( std::string const & outputPath, CTextureToCook const &tex )
   {
      uint32 numPixelDataBytes = 0;
      SVtaTextureHeader header;
      unsigned char* pPixelData = ConvertImage(tex, header, numPixelDataBytes);

      // Create/delete info files
      // Makes sure that there is for is_jpg, is_png, or is_gxt and
      // no others exist for the writable file
      for ( int i = 0; i < VTA_FORMAT_COUNT; ++i )
      {
         std::string const infoFilename = outputPath + ".is_" + skFormatTypeToString[ i ];

         if ( i == header.mFormatType )
         {
            FILE *f = fopen( infoFilename.c_str(), "wb" );
            if ( f )
            {
               fclose( f );
            }
         }
         else
         { 
            _unlink( infoFilename.c_str() );
         }
      }


      CDiskOutputStream outStream(outputPath);

      outStream.Put(&header, sizeof(SVtaTextureHeader));
      outStream.Put(pPixelData, numPixelDataBytes);
      outStream.Flush();
      
      /*
      uint32 pos = outputPath.find_last_of('.');
      std::string newPath = outputPath.substr(0, pos);

      if (header.mFormatType == VTA_FORMAT_JPG)
          newPath += ".jpg";
      else
         newPath += ".png";

      CDiskOutputStream imageStream(newPath);
      imageStream.Put(pPixelData, numPixelDataBytes);
      imageStream.Flush();
      */

      delete [] pPixelData;
      return true;
   }

} // namespace TextureCooker
