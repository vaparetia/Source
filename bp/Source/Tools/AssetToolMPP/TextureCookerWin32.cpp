//----------------------------------------------------------------------------
// TextureCookerWin32.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "stdafx.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/Graphics/CColor.h"
#include "Engine/Graphics/CColorf.h"
#include "TextureCooker.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "math.h"
#include "nvtt/nvtt.h"
#include "boost/scoped_ptr.hpp"

//----------------------------------------------------------------------------

using namespace TextureCooker;

namespace
{

//----------------------------------------------------------------------------

   int const get_num_mip_maps(int w, int h, int d)
   {
      int const size = bpe::max_val( w, bpe::max_val( h, d ) );
      return 1 + static_cast<int>( ceilf( logf( (real32) size) / logf(2.0f) ) );
   }

//----------------------------------------------------------------------------

   class CDXTDebugOutput
   {
   public:
      CDXTDebugOutput(std::string const &outputPath)
         : mOutputPath(outputPath)
      {
         mpColorsRaw.reset(new CDiskOutputStream(mOutputPath + ".dxt_colors"));
         mpLookupRaw.reset(new CDiskOutputStream(mOutputPath + ".dxt_lookup"));
         mpDecompRGBA.reset(new CGrowableMemoryOutStream);
         mpColors01RGBA.reset(new CGrowableMemoryOutStream);
         mpColors0_RGBA.reset(new CGrowableMemoryOutStream);
         mpColors_1RGBA.reset(new CGrowableMemoryOutStream);
         mpLookupRGBA.reset(new CGrowableMemoryOutStream);
      }

      void WriteDataCallback(const void * data, int size)
      {
         // Assumes DXT1 64 bit blocks
         for (int loop = 0; loop < size; loop += 8)
         {
            uint8 const *pBlock = ((uint8 const *) data) + loop;

            // Write raw block colors
            mpColorsRaw->Put(pBlock, 4);

            // Write raw lookup
            mpLookupRaw->Put(pBlock + 4, 4);

            // Decompress DXT1
            {
               uint16 const _c0 = *((uint16 *) pBlock);
               uint16 const _c1 = *((uint16 *) (pBlock + 2));
               CColor const c0(CColor::FromRGB565(_c0));
               CColor const c1(CColor::FromRGB565(_c1));
               uint32 lookup = *((uint32 *) (pBlock + 4));
               for (int i = 0; i < 16; i++)
               {
                  uint32 interp = lookup & 3;
                  lookup >>= 2;

                  CColor cOut(c0);
                  if (_c0 > _c1)
                  {
                     // Color encoding
                     switch (interp)
                     {
                     case 1:
                        cOut = c1;
                        break;
                     case 2:
                        cOut = CColor::Lerp(c0, c1, 0.333f);
                        break;
                     case 3:
                        cOut = CColor::Lerp(c0, c1, 0.666f);
                        break;
                     }
                  }
                  else
                  {
                     // Alpha encoding
                     switch (interp)
                     {
                     case 1:
                        cOut = c1;
                        break;
                     case 2:
                        cOut = CColor::Lerp(c0, c1, 0.5f);
                        break;
                     case 3:
                        cOut = CColor::FromARGB(0x00000000);   // Transparent black
                        break;
                     }
                  }
                  mpDecompRGBA->Put(cOut.GetRGBA());

                  // Lookup table
                  {
                     CColor c0(CColor::Black());   // Intentional override
                     CColor c1(CColor::White());   // Intentional override

                     CColor cOut(c0);
                     switch (interp)
                     {
                     case 3:
                        cOut = c1;
                        break;
                     case 1:
                        cOut = CColor::Lerp(c0, c1, 0.45f); // Modified for visibility
                        break;
                     case 2:
                        cOut = CColor::Lerp(c0, c1, 0.666f);
                        break;
                     }
                     mpLookupRGBA->Put(cOut.GetRGBA());
                  }

                  // Human readable block colors
                  {
                     CColor cOut((i < 8) ? c0 : c1);
                     mpColors01RGBA->Put(cOut.GetRGBA());

                     mpColors0_RGBA->Put(c0.GetRGBA());
                     mpColors_1RGBA->Put(c1.GetRGBA());
                  }
               }
            }
         }
      }

      // Decodes RGBA image that has been laid out in 16x1 decompressed DXT format to 4x4 human viewable format
      static void _DXT1BlockToImage(uint32 * pRGBA_In_Out, int const width, int const height)
      {
         uint32 *pNewImage = (uint32 *) malloc(sizeof(uint32) * width * height);
         for (int y = 0; y < height; y++)
         {
            for (int x = 0; x < width; x++)
            {
               int const blockOffset = ((y / 4) * 4 * width) + (x / 4) * 16;
               int const subBlockOffset = (y % 4) * 4 + (x % 4);
               uint32 const * pSrc = pRGBA_In_Out + blockOffset + subBlockOffset;
               int const destY = height - y - 1;   // Flip Y
               uint32 * pDest = pNewImage + (destY * width) + x;
               *pDest = *pSrc;
            }
         }
         memcpy(pRGBA_In_Out, pNewImage, sizeof(uint32) * width * height);
         free(pNewImage);
      }
      void WriteImageDebugInfo(ILuint srcImageHandle)
      {
         ilBindImage(srcImageHandle);

         int const width = ilGetInteger( IL_IMAGE_WIDTH );
         int const height = ilGetInteger( IL_IMAGE_HEIGHT );
            // save decompressed images
            {
               ILuint imageHandle;
               ilGenImages(1, &imageHandle);

               ilBindImage(imageHandle);

               ilEnable(IL_FILE_OVERWRITE);

               // Write decompressed DXT1 image for visual reference
               {
                  CGrowableMemoryOutStream * pUnswizzled = static_cast<CGrowableMemoryOutStream *>(mpDecompRGBA.get());
                  CDXTDebugOutput::_DXT1BlockToImage((uint32*) pUnswizzled->Data(), width, height);
                  ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pUnswizzled->Data());
                  ilSaveImage((char *) (mOutputPath + ".tga").c_str());
               }

               // Write debug images of DXT1 components (color, lookup etc)
               {
                  CGrowableMemoryOutStream * pUnswizzled = static_cast<CGrowableMemoryOutStream *>(mpColors01RGBA.get());
                  CDXTDebugOutput::_DXT1BlockToImage((uint32*) pUnswizzled->Data(), width, height);
                  ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pUnswizzled->Data());
                  ilSaveImage((char *) (mOutputPath + ".dxt_colors01.tga").c_str());
               }
               {
                  CGrowableMemoryOutStream * pUnswizzled = static_cast<CGrowableMemoryOutStream *>(mpColors0_RGBA.get());
                  CDXTDebugOutput::_DXT1BlockToImage((uint32*) pUnswizzled->Data(), width, height);
                  ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pUnswizzled->Data());
                  ilSaveImage((char *) (mOutputPath + ".dxt_colors0_.tga").c_str());
               }
               {
                  CGrowableMemoryOutStream * pUnswizzled = static_cast<CGrowableMemoryOutStream *>(mpColors_1RGBA.get());
                  CDXTDebugOutput::_DXT1BlockToImage((uint32*) pUnswizzled->Data(), width, height);
                  ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pUnswizzled->Data());
                  ilSaveImage((char *) (mOutputPath + ".dxt_colors_1.tga").c_str());
               }
               {
                  CGrowableMemoryOutStream * pUnswizzled = static_cast<CGrowableMemoryOutStream *>(mpLookupRGBA.get());
                  CDXTDebugOutput::_DXT1BlockToImage((uint32*) pUnswizzled->Data(), width, height);
                  ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pUnswizzled->Data());
                  ilSaveImage((char *) (mOutputPath + ".dxt_lookup.tga").c_str());
               }

               ilDeleteImages(1, &imageHandle);
            }

            // Write DXT1 error 
            {
               std::auto_ptr<uint32> pData( new uint32 [ width * height ] );             
               ilBindImage(srcImageHandle);            

               // Input is upside down
               iluFlipImage();            
               ilCopyPixels( 0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, pData.get() );            
               iluFlipImage();

               CGrowableMemoryOutStream * pDecomp = static_cast<CGrowableMemoryOutStream *>(mpDecompRGBA.get());

               bool hasAlpha = false;
               real32 meanSquaredErrorRGB = 0.0f;
               for (int p = 0; p < width * height; p++)
               {
                  uint32 *pSrc = pData.get() + p;
                  uint32 *pDXT1_RGBA = ((uint32*) pDecomp->Data()) + p;

                  CColorf const src(CColor::FromRGBA(*pSrc));
                  if (src.GetA() != 1.0f)
                  {
                     hasAlpha = true;
                  }

                  CColorf const dxt(CColor::FromRGBA(*pDXT1_RGBA));
                  CVector4 const delta((src - dxt).AsVector4());
                  meanSquaredErrorRGB += delta.GetLengthSquared();

                  {
                     // Replace copy of original image with error
                     real32 const scaleDiff = 64.0f;  // Arbitrary scale factor so we can see error easily in image
                     //CVector4 const output(((delta * scaleDiff) + CVector4(1, 1, 1, 1)) * 0.5f);               // signed
                     CVector4 const output(CVector4(fabsf(delta.mX), fabsf(delta.mY), fabsf(delta.mZ), fabsf(delta.mW)) * scaleDiff);  // unsigned

                     *pSrc = CColor::FromVector4(output).GetRGBA();
                  }
               }

               // Write text file with error info
               {
                  int const numChannels = hasAlpha ? 4 : 3;
                  real32 const peakSignalToNoiseRatio = 20.0f * log10(1.0f / sqrt(meanSquaredErrorRGB / (width * height * numChannels)));
                  CFileUtils::WriteTextFile((mOutputPath + ".info.txt").c_str(),
                     CStringExtras::Stringize("PSNR: %0.1f\n", peakSignalToNoiseRatio).c_str());
               }

               // Write error image
               ILuint imageHandle;
               ilGenImages(1, &imageHandle);

               ilBindImage(imageHandle);

               ilTexImage(width,
                          height,
                          1,
                          4,
                          IL_RGBA,
                          IL_UNSIGNED_BYTE,
                          pData.get());

               ilEnable(IL_FILE_OVERWRITE);
               ilSaveImage((char *) (mOutputPath + ".dxt_error.tga").c_str());

               ilDeleteImages(1, &imageHandle);

            }
         }


      std::string                         mOutputPath;
      boost::scoped_ptr<COutputStream>    mpColorsRaw;   // 565 color values, 2x16bit per 4x4
      boost::scoped_ptr<COutputStream>    mpLookupRaw;   // Lookup values, 16x2bit per 4x4
      boost::scoped_ptr<COutputStream>    mpColors01RGBA;
      boost::scoped_ptr<COutputStream>    mpColors0_RGBA;
      boost::scoped_ptr<COutputStream>    mpColors_1RGBA;
      boost::scoped_ptr<COutputStream>    mpLookupRGBA;   // Lookup values, 16x2bit per 4x4
      boost::scoped_ptr<COutputStream>    mpDecompRGBA;   // Decompressed RGBA 16x32bit per 4x4
   };

//----------------------------------------------------------------------------

   class CNVTTOutputHandler : public nvtt::OutputHandler
   {
   public:
      CNVTTOutputHandler(COutputStream & stream, CDXTDebugOutput * pDebugOutput)
         :  mStream(stream)
         ,  mpDebugOutput(pDebugOutput)
         , mWroteSinceBegin( 0 )
      {
      }

      ~CNVTTOutputHandler()
      {
         AlignTo32();
      }

      void beginImage(int size, int width, int height, int depth, int face, int miplevel)
      {
//         handy for nvda output debugging
//         printf( "Got beginImage: %d\n", size );
         mWroteSinceBegin = 0;
         AlignTo32();

         // write size
         mStream.WriteUint32(size);
      }

      bool writeData(const void * data, int size)
      {
//         handy for nvda output debugging
//         printf( "Got writeData: %d %d\n", mWroteSinceBegin, size );
         mWroteSinceBegin += size;
         mStream.Put(data, size);
         if (mpDebugOutput)
         {
            mpDebugOutput->WriteDataCallback(data, size);
         }
         return true;
      }

   private:

      void AlignTo32()
      {
         while( mStream.GetWrittenBytes() % 32 )
         {
            mStream.WriteInt8(0);
         }
      }

   private:
      COutputStream &   mStream;
      CDXTDebugOutput * mpDebugOutput;
      int mWroteSinceBegin;
   };

   void WriteImageData( COutputStream & stream, bool const includeDepth, CTextureToCook const &tex, int const texFaceIndex, CDXTDebugOutput * pDebugOutput )
   {
      CTextureFace const &texFace = tex.mTextureFaces[texFaceIndex];

      // Setup input options
      nvtt::InputOptions inputOptions;
      ILuint topLevelMipHandle = texFace.mMipLevels[0].mImageHandle;

      ilBindImage(topLevelMipHandle);
      int const fullWidth = ilGetInteger( IL_IMAGE_WIDTH );
      int const fullHeight = ilGetInteger( IL_IMAGE_HEIGHT );
      int const fullDepth = includeDepth ? ilGetInteger( IL_IMAGE_DEPTH ) : 1;

      inputOptions.setTextureLayout( ( fullDepth > 1 ) ? nvtt::TextureType_3D : nvtt::TextureType_2D, fullWidth, fullHeight, fullDepth);   

      // Set top level mip image data
      {
         std::auto_ptr<char> pRawData( new char [ fullWidth * fullHeight * fullDepth * sizeof(uint32) ] ); 
         ilCopyPixels( 0, 0, 0, fullWidth, fullHeight, fullDepth, IL_BGRA, IL_UNSIGNED_BYTE, pRawData.get() );

         inputOptions.setMipmapData(pRawData.get(), fullWidth, fullHeight, fullDepth);
      }

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

            int const kFace = 0;
            inputOptions.setMipmapData(pRawData.get(), mipWidth, mipHeight, mipDepth, kFace, mipLevel);
         }
      }
      else
      {
         inputOptions.setMipmapGeneration(false);   
      }

      // Setup output options
      nvtt::OutputOptions outputOptions;
      CNVTTOutputHandler outputHandler(stream, pDebugOutput);
      outputOptions.setOutputHandler(&outputHandler);
      outputOptions.setOutputHeader(false);

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

      // Do compression
      nvtt::Compressor compressor;
      // cuda acceleration currently disabled due to stability issues.
      compressor.enableCudaAcceleration(false);

      compressor.process(inputOptions, compressionOptions, outputOptions);
   }

//----------------------------------------------------------------------------

} // namespace

namespace TextureCooker
{

bool WriteTextureWin32( std::string const & outputPath, CTextureToCook const &tex )
{
   CDiskOutputStream outStream(outputPath);

   // write header
   outStream.WriteUint32('TXTR');
   // write version
   // Version 4 adds depth
   // Version 5 adds min/max RGBA
   // Version 6 adds texture filter hint, alpha fail hint
   // Version 7 adds max lod offset, force clamp u/v
   outStream.WriteUint32(7);

   // write dimensions   
   ilBindImage(tex.GetTopLevelImageHandle());
   ILint width = ilGetInteger(IL_IMAGE_WIDTH);
   outStream.WriteUint16(width);
   ILint height = ilGetInteger(IL_IMAGE_HEIGHT);
   outStream.WriteUint16(height);
   ILint depth = ilGetInteger( IL_IMAGE_DEPTH );
   outStream.WriteUint16( depth );

   int const totalTexelCount = width * height * depth;

   // write texture format
   {
      outStream.WriteUint32(tex.mFormat);
   }

   // write has alpha flag
   bool hasAlpha = tex.HasAlpha();
   outStream.WriteBool(hasAlpha);

   // Write 32 bits of flags
   outStream.WriteUint32(tex.mAdditionalFlags);

   // Write min/max RGBA
   outStream.WriteUint32(tex.mMinRGBA);
   outStream.WriteUint32(tex.mMaxRGBA);

   outStream.WriteInt8(tex.mFilterHint);
   outStream.WriteUint8(tex.mAlphaRefValue);
   outStream.WriteInt8(tex.mMaxLODOffset);

   // write texture type
   {
      CBaseTexture::EType type = CBaseTexture::kType_Normal;
      if ( tex.mTextureFaces.size() == 6 )
      {
         type = CBaseTexture::kType_Cube;
      }

      outStream.WriteUint32(type);
   }

   // write number of levels
   int const numLevels = (tex.mGenerateMipMaps != kGM_No) ? get_num_mip_maps(width, height, depth) : 1;
   outStream.WriteUint8(numLevels);


   // pad header until we reached the fixed size we desire
   {
      int const kFixedHeaderSize = 128;

      BPE_VERIFY( outStream.GetWrittenBytes() <= kFixedHeaderSize, false, "header too big, increase fixed size" );

      while( outStream.GetWrittenBytes() < kFixedHeaderSize )
         outStream.WriteInt8(0);
   }

   for( int texFaceIndex = 0; texFaceIndex < tex.mTextureFaces.size(); texFaceIndex++ )
   {
      // TODO: Rework debug output (see below)
      // write out image data for each texture face
      bool const bWriteDepth = true;
      WriteImageData(outStream, bWriteDepth, tex, texFaceIndex, NULL);
   }

   return true;
}


#if 0
// Example code for debug, needs rework
static void _cook_debug(COutputStream &stream, CTextureToCook const &tex)
{
   for( int faceIndex = 0; faceIndex < tex.mTextureFaces.size(); faceIndex++ )
   {

      boost::scoped_ptr<CDXTDebugOutput> pDebugOutput;
      {
         bool bWriteDebugInfo = (i == 0);
         bWriteDebugInfo = false;
         if (bWriteDebugInfo)
         {
            generateMipMaps = false;   // Disable mipmaps for compression testing
            format = CBaseTexture::kFormat_DXT1;     // Force DXT1
            pDebugOutput.reset(new CDXTDebugOutput(outputPath));
         }
      }

      // write out image data
      WriteImageData(outStream, tex, faceIndex, pDebugOutput.get());

      if (pDebugOutput)
      {
         pDebugOutput->WriteImageDebugInfo(srcImageHandle);
      }
   }
}
#endif

//----------------------------------------------------------------------------

} // namespace TextureCooker
