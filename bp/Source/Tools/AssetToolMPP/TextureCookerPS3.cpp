//----------------------------------------------------------------------------
// TextureCookerPS3.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "stdafx.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

#include "IL/il.h"
//#include "math.h"
#include "nvtt/nvtt.h"

#include "../../ExtLibraries/libgtfconv/ddsinfo.h"
#include "../../ExtLibraries/libgtfconv/gtfconv.h"
#include "../../ExtLibraries/libgtfconv/utils.h"

#include "TextureCooker.h"

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

} // namespace

namespace TextureCooker
{

bool WriteTexturePS3( std::string const & outputPath, CTextureToCook const &tex )
{
   bool hasAlpha = tex.HasAlpha();

   // build dds file
   CDDS dds;
   BuildDDS(tex, hasAlpha, &dds);

   // get input buffer for dds file
   GTF_BUFFER_T input = { (uint8_t*)dds.mMemory.Data(), dds.mMemory.GetWrittenBytes() };

   uint32 flags = CELL_GTFCONV_FLAG_SWIZZLIZE;
   
   if( hasAlpha )
      flags |= CELL_GTFCONV_FLAG_HAS_VARYING_ALPHA;

   //flags |= CELL_GTFCONV_FLAG_VERBOSE;

   // Overload flags to store our additional flags
   {
   }


   // determine size needed for output
   int const outputSize = gtfconv_dds2gtf_get_size(&input, 1, flags);

   // allocate memory for output
   void* pOutputMemory = malloc(outputSize);

   // convert the texture
   {
      GTF_BUFFER_T output = { (uint8_t*)pOutputMemory, outputSize };
      if( gtfconv_dds2gtf(&output, &input, 1, flags) != CELL_GTFCONV_OK )
         return false;
   }
   // Update BP additional members
   {
      CellGtfTextureAttribute* pAttribute = (CellGtfTextureAttribute*)( (uint8_t*)pOutputMemory + sizeof(CellGtfFileHeader) );

      pAttribute->mAdditionalFlags = tex.mAdditionalFlags;
      NEndian::Swap4Bytes(&pAttribute->mAdditionalFlags);
      pAttribute->mMinRGBA = tex.mMinRGBA;
      NEndian::Swap4Bytes(&pAttribute->mMinRGBA);
      pAttribute->mMaxRGBA = tex.mMaxRGBA;
      NEndian::Swap4Bytes(&pAttribute->mMaxRGBA);

      pAttribute->mFilterHint = tex.mFilterHint;
      pAttribute->mAlphaRefValue = tex.mAlphaRefValue;
      pAttribute->mMaxLODOffset = tex.mMaxLODOffset;
   }

   // write gtf file to disk.
   CDiskOutputStream outStream(outputPath);
   outStream.Put(pOutputMemory, outputSize);

   free(pOutputMemory);
   return true;
}

} // namespace TextureCooker
