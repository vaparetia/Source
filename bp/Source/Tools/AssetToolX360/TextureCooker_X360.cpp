//----------------------------------------------------------------------------
// TextureCookerX360.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "stdafx.h"

#include <windows.h>
#include <d3d9.h>
#include <xgraphics.h>

//----------------------------------------------------------------------------
#include "Engine/Streams/CDiskOutputStream.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/Graphics/CColor.h"
#include "Engine/Graphics/CColorf.h"

#include "Tools/AssetToolMPP/TextureCooker.h"

#include "IL/il.h"
#include "IL/ilu.h"
#include "math.h"
#include "nvtt/nvtt.h"
#include "boost/scoped_ptr.hpp"
#include "boost/scoped_array.hpp"

#define X360TOOLSAPI extern "C" __declspec(dllexport)

using namespace TextureCooker;
//----------------------------------------------------------------------------
enum PixelRGBAOrder
{
   kMaxR,
   kMaxG,
   kMaxB,
   kMaxA,

   kMinR,
   kMinG,
   kMinB,
   kMinA,

   kAvgR,
   kAvgG,
   kAvgB,
   kAvgA,

   kPRGBA_Total
};
void CalcMinMaxAverageRGBA(ILuint imageHandle, uint32* aMaxMinAvg)
{
   aMaxMinAvg[kMaxR] = 0;
   aMaxMinAvg[kMaxG] = 0;
   aMaxMinAvg[kMaxB] = 0;
   aMaxMinAvg[kMaxA] = 0;

   aMaxMinAvg[kMinR] = 255;
   aMaxMinAvg[kMinG] = 255;
   aMaxMinAvg[kMinB] = 255;
   aMaxMinAvg[kMinA] = 255;

   aMaxMinAvg[kAvgR] = 0;
   aMaxMinAvg[kAvgG] = 0;
   aMaxMinAvg[kAvgB] = 0;
   aMaxMinAvg[kAvgA] = 0;

   {
      ilBindImage(imageHandle);

      int const width = ilGetInteger(IL_IMAGE_WIDTH);
      int const height = ilGetInteger(IL_IMAGE_HEIGHT);
      int const depth = ilGetInteger( IL_IMAGE_DEPTH );
      int const texelCount = width * height * depth;

      std::auto_ptr<uint32> pData( new uint32 [ width * height ] );             
      ilCopyPixels( 0, 0, 0, width, height, depth, IL_RGBA, IL_UNSIGNED_BYTE, pData.get() );                  

      for( int i = 0; i < texelCount; ++i )
      {
         // IL uses 'reverse' RGBA, i.e. as hex it reads as '0xAABBGGRRR'
         uint32 const rgba = *(pData.get() + i);
         uint32 const r = (rgba >>  0) & 0xFF;
         uint32 const g = (rgba >>  8) & 0xFF;
         uint32 const b = (rgba >> 16) & 0xFF;
         uint32 const a = (rgba >> 24) & 0xFF;

         aMaxMinAvg[kMinR] = min(aMaxMinAvg[kMinR], r);
         aMaxMinAvg[kMinG] = min(aMaxMinAvg[kMinG], g);
         aMaxMinAvg[kMinB] = min(aMaxMinAvg[kMinB], b);
         aMaxMinAvg[kMinA] = min(aMaxMinAvg[kMinA], a);

         aMaxMinAvg[kMaxR] = max(aMaxMinAvg[kMaxR], r);
         aMaxMinAvg[kMaxG] = max(aMaxMinAvg[kMaxG], g);
         aMaxMinAvg[kMaxB] = max(aMaxMinAvg[kMaxB], b);
         aMaxMinAvg[kMaxA] = max(aMaxMinAvg[kMaxA], a);

         aMaxMinAvg[kAvgR] += r;
         aMaxMinAvg[kAvgG] += g;
         aMaxMinAvg[kAvgB] += b;
         aMaxMinAvg[kAvgA] += a;
      }

      aMaxMinAvg[kAvgR] /= texelCount;
      aMaxMinAvg[kAvgG] /= texelCount;
      aMaxMinAvg[kAvgB] /= texelCount;
      aMaxMinAvg[kAvgA] /= texelCount;
   }
}

int const get_num_mip_maps(int w, int h, int d)
{
   int const size = bpe::max_val( w, bpe::max_val( h, d ) );
   return 1 + static_cast<int>( ceilf( logf( (real32) size) / logf(2.0f) ) );
}

int const get_num_mip_maps(CTextureToCook const & tex, int w, int h, int d)
{
   if( tex.mGenerateMipMaps != kGM_No )
   {
      return get_num_mip_maps(w, h, d);
   }
   return 1;
}

struct OutputInfoStructure
{
   std::string outputFileName;
   bool writeOutDebugInformation;
};

//----------------------------------------------------------------------------

class CDDS : public nvtt::OutputHandler
{
public:
   CDDS()
   {
      mStream.SetEndian( COutputStream::kOE_LittleEndian );
   }

   ~CDDS()
   {
   }

   void beginImage(int size, int width, int height, int depth, int face, int miplevel)
   {
      //write size
      mStream.WriteUint32(size);
      mStream.WriteUint32(width);
      mStream.WriteUint32(height);
      mStream.WriteUint32(depth);
   }

   bool writeData(const void * data, int size)
   {
      mStream.Put(data, size);
      return true;
   }

public:
   CGrowableMemoryOutStream mStream;
};

struct DDSOutputInfo
{
   nvtt::TextureType textureType;
   CBaseTexture::EDiscFormat discFormat;
   CBaseTexture::EFormat format;
   int fullWidth;
   int fullHeight;
   int fullDepth;
   int mipLevels;
   D3DFORMAT d3dFormat;

   int PTCQuantizationStep;
};

struct XGInfo
{
   boost::scoped_ptr<IDirect3DBaseTexture9> pBaseTexture;
   boost::scoped_array<unsigned char> pTexBuffer;

   unsigned int sizeofTextureHeader;
   unsigned int baseMapBytes;
   unsigned int mipMapBytes;
   float oneBitThreshold;
};

inline void WriteOutCommonHeaderInfo(CDiskOutputStream & outStream, CTextureToCook const & tex, DDSOutputInfo & ddsOutputInfo, bool hasAlpha)
{
   outStream.WriteUint32('TXTR');
   outStream.WriteUint32(3);//version 
   outStream.WriteUint32(ddsOutputInfo.discFormat);//CBaseTexture::EDiscFormat
   outStream.WriteUint32(ddsOutputInfo.format);//CBaseTexture::EFormat

   outStream.WriteUint16(ddsOutputInfo.fullWidth);//width
   outStream.WriteUint16(ddsOutputInfo.fullHeight);//height
   outStream.WriteUint16(ddsOutputInfo.fullDepth);//depth
   outStream.WriteUint8(ddsOutputInfo.mipLevels);//miplevels
   outStream.WriteBool(hasAlpha);//hasalpha
   outStream.WriteUint32(tex.mMinRGBA);
   outStream.WriteUint32(tex.mMaxRGBA);
   outStream.WriteUint32(tex.mAdditionalFlags);

   outStream.WriteUint32('DATA');
   outStream.WriteUint32(ddsOutputInfo.textureType);//nvtt::TextureType
}

inline bool WriteOutDebugInfo(std::string const & fileName, 
                              int maxR, int minR, int avgR, 
                              int maxG, int minG, int avgG,
                              int maxB, int minB, int avgB,
                              int maxA, int minA, int avgA)
{
   CDiskOutputStream outStream(fileName.c_str());

   char outputString[MAX_PATH];

   sprintf( outputString, "RMax: %d\r\nRMin: %d\r\nRAverage: %d\r\n", maxR, minR, avgR );
   outStream.Put( outputString, strlen(outputString) );

   sprintf( outputString, "GMax: %d\r\nGMin: %d\r\nGAverage: %d\r\n", maxG, minG, avgG );
   outStream.Put( outputString, strlen(outputString) );

   sprintf( outputString, "BMax: %d\r\nBMin: %d\r\nBAverage: %d\r\n", maxB, minB, avgB );
   outStream.Put( outputString, strlen(outputString) );

   sprintf( outputString, "AMax: %d\r\nAMin: %d\r\nAAverage: %d\r\n", maxA, minA, avgA );
   outStream.Put( outputString, strlen(outputString) );
   return true;
}

bool BuildDDS(CTextureToCook const & tex, CDDS * pDDS, DDSOutputInfo & outputInfo)
{
   ilBindImage(tex.GetTopLevelImageHandle());

   int const fullWidth = ilGetInteger( IL_IMAGE_WIDTH );
   int const fullHeight = ilGetInteger( IL_IMAGE_HEIGHT );
   int const fullDepth = ilGetInteger( IL_IMAGE_DEPTH );

   outputInfo.discFormat = tex.mDiscFormat;
   outputInfo.PTCQuantizationStep = 100;
   // Special rules to prevent doing kDiscFormat_PTC
   if( outputInfo.discFormat == CBaseTexture::kDiscFormat_PTC )
   {
      if( fullWidth <= 32 || fullHeight <= 32 )
      {
         outputInfo.discFormat = CBaseTexture::kDiscFormat_Raw;
      }
      if( tex.mTextureFaces.size() != 1 )
      {
         outputInfo.discFormat = CBaseTexture::kDiscFormat_Raw;
      }
      if( fullDepth != 1 )
      {
         outputInfo.discFormat = CBaseTexture::kDiscFormat_Raw;
      }
   }

   // Setup input options
   nvtt::InputOptions inputOptions;
   //inputOptions.setMipmapFilter()

   nvtt::TextureType textureType = nvtt::TextureType_2D;

   if( tex.mTextureFaces.size() == 6 )
   {
      textureType = nvtt::TextureType_Cube;
   }
   else if ( fullDepth > 1 )
   {
      textureType = nvtt::TextureType_3D;
   }

   outputInfo.textureType = textureType;
   outputInfo.fullWidth = fullWidth;
   outputInfo.fullHeight = fullHeight;
   outputInfo.fullDepth = fullDepth;
   TextureCooker::EGenerateMipMaps generateMipMaps = tex.mGenerateMipMaps;
   if( outputInfo.discFormat == CBaseTexture::kDiscFormat_PTC )
   {
      BPE_VERIFY( tex.mTextureFaces.size() == 1, false, "PTC format does not currently support cube map!" );
      BPE_VERIFY( fullDepth == 1, false, "PTC format does not currently support volume textures!" );
      generateMipMaps = TextureCooker::kGM_No;
   }
   outputInfo.mipLevels = get_num_mip_maps(tex, outputInfo.fullWidth, outputInfo.fullHeight, outputInfo.fullDepth);

   inputOptions.setTextureLayout(textureType, fullWidth, fullHeight, fullDepth );

   // get data out of library into raw buffer
   for( int texFaceIndex = 0; texFaceIndex < tex.mTextureFaces.size(); texFaceIndex++ )
   {
      CTextureFace const & texFace = tex.mTextureFaces[texFaceIndex];
      ILuint topLevelMipHandle = texFace.mMipLevels[0].mImageHandle;

      ilBindImage(topLevelMipHandle);

      int const imageWidth = ilGetInteger( IL_IMAGE_WIDTH );
      int const imageHeight = ilGetInteger( IL_IMAGE_HEIGHT );
      int const imageDepth = ilGetInteger( IL_IMAGE_DEPTH );

      BPE_VERIFY( imageWidth == fullWidth && imageHeight == fullHeight && imageDepth == fullDepth, false, "Tex face heights and widths are out of whack" );

      std::auto_ptr<char> pRawData( new char [ imageWidth * imageHeight * imageDepth * sizeof(uint32) ] ); 
      ilCopyPixels( 0, 0, 0, imageWidth, imageHeight, imageDepth, IL_BGRA, IL_UNSIGNED_BYTE, pRawData.get() );

      inputOptions.setMipmapData(pRawData.get(), imageWidth, imageHeight, imageDepth, texFaceIndex);
      if(generateMipMaps != kGM_No)
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
   outputOptions.setOutputHeader(false);

   nvtt::Format nvttFormat;

   bool colorDithering = false;
   bool alphaDithering = false;
   bool binaryAlpha = false;

   outputInfo.format = tex.mFormat;
   switch(outputInfo.format)
   {
   case CBaseTexture::kFormat_A8R8G8B8:
      outputInfo.d3dFormat = D3DFMT_A8R8G8B8;
      break;

   case CBaseTexture::kFormat_DXT1:
      outputInfo.d3dFormat = D3DFMT_DXT1;
      binaryAlpha = true;
      break;

   case CBaseTexture::kFormat_DXT3:
      outputInfo.d3dFormat = D3DFMT_DXT3;
      break;

   case CBaseTexture::kFormat_DXT5:
      outputInfo.d3dFormat = D3DFMT_DXT5;
      break;

   default:
      BPE_VERIFYA(false, "Unsupported texture format");
   }
   nvttFormat = nvtt::Format_RGBA;

   // Setup compression options
   nvtt::CompressionOptions compressionOptions;
   compressionOptions.setQuality(nvtt::Quality_Normal);

   compressionOptions.setFormat(nvttFormat);

   uint32 const bitCount   = 32;
   uint32 const redMask    = 0x00FF0000;
   uint32 const greenMask  = 0x0000FF00;
   uint32 const blueMask   = 0x000000FF;
   uint32 const alphaMask  = 0xFF000000;

   compressionOptions.setPixelFormat(bitCount, redMask, greenMask, blueMask, alphaMask);

   // Do compression
   nvtt::Compressor compressor;
   compressor.enableCudaAcceleration(false);
   compressor.process(inputOptions, compressionOptions, outputOptions);

   return true;
}

bool DDS_ToDiscFormat_Raw(OutputInfoStructure const & outputInfoStructure, CTextureToCook const & tex, CDDS & dds, DDSOutputInfo & ddsOutputInfo, bool hasAlpha)
{
   XGInfo xgInfo;
   xgInfo.oneBitThreshold = 0.5f;
   if( tex.mTextureFaces.size() == 6 )
   {
      IDirect3DCubeTexture9 * pCubeTexture = new D3DCubeTexture;
      xgInfo.pBaseTexture.reset(pCubeTexture);
      xgInfo.sizeofTextureHeader = sizeof(IDirect3DCubeTexture9);
      XGSetCubeTextureHeaderEx(
         ddsOutputInfo.fullWidth,
         ddsOutputInfo.mipLevels,
         0,
         ddsOutputInfo.d3dFormat,
         0,
         0,
         0,
         XGHEADER_CONTIGUOUS_MIP_OFFSET,
         pCubeTexture,
         &xgInfo.baseMapBytes,
         &xgInfo.mipMapBytes
         );
   }
   else if( ddsOutputInfo.fullDepth > 1 )
   {
      IDirect3DVolumeTexture9 * pTexture = new D3DVolumeTexture;
      xgInfo.pBaseTexture.reset(pTexture);
      xgInfo.sizeofTextureHeader = sizeof(IDirect3DVolumeTexture9);
      XGSetVolumeTextureHeaderEx(
         ddsOutputInfo.fullWidth,
         ddsOutputInfo.fullHeight,
         ddsOutputInfo.fullDepth,
         ddsOutputInfo.mipLevels,
         0,
         ddsOutputInfo.d3dFormat,
         0,
         0,
         0,
         XGHEADER_CONTIGUOUS_MIP_OFFSET,
         pTexture,
         &xgInfo.baseMapBytes,
         &xgInfo.mipMapBytes
         );
   }
   else
   {
      IDirect3DTexture9 * pTexture = new D3DTexture;
      xgInfo.pBaseTexture.reset(pTexture);
      xgInfo.sizeofTextureHeader = sizeof(IDirect3DTexture9);
      XGSetTextureHeaderEx(
         ddsOutputInfo.fullWidth,
         ddsOutputInfo.fullHeight,
         ddsOutputInfo.mipLevels,
         0,
         ddsOutputInfo.d3dFormat,
         0,
         0,
         0,
         XGHEADER_CONTIGUOUS_MIP_OFFSET,
         0,
         pTexture,
         &xgInfo.baseMapBytes,
         &xgInfo.mipMapBytes
         );
   }

   //-------------------------------------------------------------------------------------------
   boost::scoped_array<unsigned char> pLinearBuffer;
   // create a buffer in which a D3D texture can be built
   xgInfo.pTexBuffer.reset(new unsigned char[ xgInfo.baseMapBytes + xgInfo.mipMapBytes ]);
   if( !xgInfo.pTexBuffer.get() )
   {
      return false;
   }

   // Get the description of the base texture level.
   XGTEXTURE_DESC baseDesc;
   XGGetTextureDesc(xgInfo.pBaseTexture.get(), 0, &baseDesc);

   unsigned int gpuFormat = XGGetGpuFormat((D3DFORMAT)baseDesc.Format);
   BOOL bTiled = XGIsTiledFormat((D3DFORMAT)baseDesc.Format);
   D3DFORMAT fmtCompress = (D3DFORMAT)(baseDesc.Format & ~D3DFORMAT_TILED_MASK);

   // determine creation flags
   unsigned int createFlags = 0;
   //if( !in.bDither )
   if( true )
   {
      createFlags |= XGCOMPRESS_NO_DITHERING;
   }
   //if( in.bPremultiplyAlpha )
   if( false )
   {
      createFlags |= XGCOMPRESS_PREMULTIPLY;
   }

   if( bTiled )
   {
      int test = sizeof(boost::scoped_array<char>);
      pLinearBuffer.reset(new unsigned char[ xgInfo.baseMapBytes + xgInfo.mipMapBytes ]);
      if( !pLinearBuffer.get() )
      {
         return false;
      }
   }

   unsigned char * pSrcData = (unsigned char *)dds.mStream.Data();
   unsigned int srcLength = dds.mStream.GetWrittenBytes();
   if( ddsOutputInfo.fullDepth > 1 )
   {
      for( unsigned int face = 0; face < tex.mTextureFaces.size(); ++face )
      {
         for( unsigned int level = 0; level < ddsOutputInfo.mipLevels; ++level )
         {
            unsigned int LevelSize = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            unsigned int Width = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            unsigned int Height = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            unsigned int Depth = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            // Perform the endian-swap in-place on the image data...
            XGEndianSwapMemory(
               (void *)pSrcData,
               (void *)pSrcData,
               XGENDIAN_8IN32,
               sizeof(DWORD),
               Width*Height*Depth);

            // get the description of the destination mip level
            XGTEXTURE_DESC dstMIPDesc;
            XGGetTextureDesc(xgInfo.pBaseTexture.get(), level, &dstMIPDesc);

            // get the mip level offset within our buffer
            unsigned int dstMIPOffset = XGGetMipLevelOffset(xgInfo.pBaseTexture.get(), face, level);
            if( (level > 0) && (xgInfo.mipMapBytes > 0) )
            {
               dstMIPOffset += xgInfo.baseMapBytes;
            }

            // if tiled, create a linear compressed version first
            HRESULT hr;
            if( bTiled )
            {
               hr = XGCopyVolume( pLinearBuffer.get(), 
                  dstMIPDesc.RowPitch,
                  dstMIPDesc.SlicePitch,
                  dstMIPDesc.Width,
                  dstMIPDesc.Height,
                  dstMIPDesc.Depth,
                  fmtCompress,
                  NULL,
                  pSrcData,
                  dstMIPDesc.Width * sizeof(DWORD),
                  dstMIPDesc.Width * dstMIPDesc.Height * sizeof(DWORD),
                  D3DFMT_LIN_A8R8G8B8,
                  NULL,
                  createFlags,
                  xgInfo.oneBitThreshold);
               if( hr != S_OK )
               {
                  return false;
               }
               // now tile
               XGTileVolumeTextureLevel( baseDesc.Width,
                  baseDesc.Height,
                  baseDesc.Depth,
                  level,
                  gpuFormat,
                  0,
                  xgInfo.pTexBuffer.get() + dstMIPOffset,
                  NULL,
                  pLinearBuffer.get(),
                  dstMIPDesc.RowPitch,
                  dstMIPDesc.SlicePitch,
                  NULL);
            }
            else
            {
               hr = XGCopyVolume( xgInfo.pTexBuffer.get() + dstMIPOffset, 
                  dstMIPDesc.RowPitch,
                  dstMIPDesc.SlicePitch,
                  dstMIPDesc.Width,
                  dstMIPDesc.Height,
                  dstMIPDesc.Depth,
                  dstMIPDesc.Format,
                  NULL,
                  pSrcData,
                  dstMIPDesc.Width * sizeof(DWORD),
                  dstMIPDesc.Width * dstMIPDesc.Height * sizeof(DWORD),
                  D3DFMT_LIN_A8R8G8B8,
                  NULL,
                  createFlags,
                  xgInfo.oneBitThreshold);
            }

            // advance the pointer to the next mip level
            pSrcData += LevelSize;
         }
      }
   }
   else
   {
      for( unsigned int face = 0; face < tex.mTextureFaces.size(); ++face )
      {
         for( unsigned int level = 0; level < ddsOutputInfo.mipLevels; ++level )
         {
            unsigned int LevelSize = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            unsigned int Width = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            unsigned int Height = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            unsigned int Depth = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
            // Perform the endian-swap in-place on the image data...
            XGEndianSwapMemory(
               (void *)pSrcData,
               (void *)pSrcData,
               XGENDIAN_8IN32,
               sizeof(DWORD),
               Width*Height*Depth);

            // get the description of the destination mip level
            XGTEXTURE_DESC dstMIPDesc;
            XGGetTextureDesc(xgInfo.pBaseTexture.get(), level, &dstMIPDesc);

            // get the mip level offset within our buffer
            unsigned int dstMIPOffset = XGGetMipLevelOffset(xgInfo.pBaseTexture.get(), face, level);
            if( (level > 0) && (xgInfo.mipMapBytes > 0) )
            {
               dstMIPOffset += xgInfo.baseMapBytes;
            }

            // if tiled, create a linear compressed version first
            HRESULT hr;
            if( bTiled )
            {
               hr = XGCopySurface( pLinearBuffer.get(), 
                  dstMIPDesc.RowPitch,
                  dstMIPDesc.Width,
                  dstMIPDesc.Height,
                  fmtCompress,
                  NULL,
                  pSrcData,
                  dstMIPDesc.Width * sizeof(DWORD),
                  D3DFMT_LIN_A8R8G8B8,
                  NULL,
                  createFlags,
                  xgInfo.oneBitThreshold);
               if( hr != S_OK )
               {
                  return false;
               }
               // now tile
               XGTileTextureLevel( baseDesc.Width,
                  baseDesc.Height,
                  level,
                  gpuFormat,
                  0,
                  xgInfo.pTexBuffer.get() + dstMIPOffset,
                  NULL,
                  pLinearBuffer.get(),
                  dstMIPDesc.RowPitch,
                  NULL);
            }
            else
            {
               hr = XGCopySurface( xgInfo.pTexBuffer.get() + dstMIPOffset,
                  dstMIPDesc.RowPitch,
                  dstMIPDesc.Width,
                  dstMIPDesc.Height,
                  dstMIPDesc.Format,
                  NULL,
                  pSrcData,
                  dstMIPDesc.Width * sizeof(DWORD),
                  D3DFMT_LIN_A8R8G8B8,
                  NULL,
                  createFlags,
                  xgInfo.oneBitThreshold);
            }

            // advance the pointer to the next mip level
            pSrcData += LevelSize;
         }
      }
   }

   //-----------------------------------------------------------------------------

   // write file to disk.
   CDiskOutputStream outStream(outputInfoStructure.outputFileName.c_str());
   WriteOutCommonHeaderInfo(outStream, tex, ddsOutputInfo, hasAlpha);
   // format specific data
   outStream.WriteUint32(xgInfo.sizeofTextureHeader);
   // Endian swap the D3D texture header.
   XGEndianSwapMemory( xgInfo.pBaseTexture.get(), xgInfo.pBaseTexture.get(), XGENDIAN_8IN32, sizeof( DWORD ), xgInfo.sizeofTextureHeader / sizeof( DWORD ) );
   outStream.Put(xgInfo.pBaseTexture.get(), xgInfo.sizeofTextureHeader);

   outStream.WriteUint32(xgInfo.baseMapBytes + xgInfo.mipMapBytes);
   outStream.Put(xgInfo.pTexBuffer.get(), xgInfo.baseMapBytes + xgInfo.mipMapBytes);

   if( outputInfoStructure.writeOutDebugInformation )
   {
      uint32 aMinMaxAvg[kPRGBA_Total];
      CalcMinMaxAverageRGBA(tex.GetTopLevelImageHandle(), aMinMaxAvg);

      std::string debugOutputFileName = outputInfoStructure.outputFileName;
      debugOutputFileName += ".txt";
         
      WriteOutDebugInfo(debugOutputFileName, 
         aMinMaxAvg[kMaxR], aMinMaxAvg[kMinR], aMinMaxAvg[kAvgR],
         aMinMaxAvg[kMaxG], aMinMaxAvg[kMinG], aMinMaxAvg[kAvgG],
         aMinMaxAvg[kMaxB], aMinMaxAvg[kMinB], aMinMaxAvg[kAvgB],
         aMinMaxAvg[kMaxA], aMinMaxAvg[kMinA], aMinMaxAvg[kAvgA]);
   }

   return true;
}

bool DDS_ToDiscFormat_PTC(OutputInfoStructure const & outputInfoStructure, CTextureToCook const & tex, CDDS & dds, DDSOutputInfo & ddsOutputInfo, bool hasAlpha)
{
   unsigned char * pSrcData = (unsigned char *)dds.mStream.Data();
   unsigned int srcLength = dds.mStream.GetWrittenBytes();

   unsigned int LevelSize = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
   unsigned int Width = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
   unsigned int Height = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
   unsigned int Depth = *(uint32*)pSrcData; pSrcData += sizeof(uint32);
   BPE_VERIFY( Depth == 1, false, "PTC format does not currently support volume textures!" );
   // Perform the endian-swap in-place on the image data...
   XGEndianSwapMemory(
      (void *)pSrcData,
      (void *)pSrcData,
      XGENDIAN_8IN32,
      sizeof(DWORD),
      Width*Height*Depth);

   void* pCompressedData = NULL;
   UINT compressedSize = 0;
   HRESULT hr = S_OK;
   D3DFORMAT PTCd3dFormat = D3DFMT_LIN_A8R8G8B8;
   if( FAILED( hr = XGPTCCompressSurface(&pCompressedData, &compressedSize, pSrcData, ddsOutputInfo.fullWidth * sizeof(DWORD), ddsOutputInfo.fullWidth, ddsOutputInfo.fullHeight, PTCd3dFormat, NULL, ddsOutputInfo.PTCQuantizationStep) ) )
   {
      BPE_VERIFYA( false, "PTC compression failed!");
      return false;
   }

   //-----------------------------------------------------------------------------

   // write file to disk.
   CDiskOutputStream outStream(outputInfoStructure.outputFileName.c_str());
   WriteOutCommonHeaderInfo(outStream, tex, ddsOutputInfo, hasAlpha);
   // format specific data
   outStream.WriteUint32(compressedSize);
   outStream.Put(pCompressedData, compressedSize);

   if( outputInfoStructure.writeOutDebugInformation )
   {
      // First decompression the PCT
      if( FAILED( hr = XGPTCDecompressSurface( pSrcData, Width*sizeof(DWORD),
         Width, Height, D3DFMT_LIN_A8R8G8B8, NULL,
         pCompressedData, compressedSize ) ) )
      {
         BPE_VERIFYA(false, "PTC decompression failed!")
      }
      // Endian swap back to RGBA8
      XGEndianSwapMemory(
         (void *)pSrcData,
         (void *)pSrcData,
         XGENDIAN_8IN32,
         sizeof(DWORD),
         Width*Height*Depth);
      // Bind to il library
      ILuint pctDecompressedHandle;
      ilGenImages(1, &pctDecompressedHandle);
      ilBindImage(pctDecompressedHandle);
      //ILboolean test = ilLoadL(IL_RAW, pSrcData, Width*Height*Depth*sizeof(DWORD));
      ilTexImage(Width, Height, Depth, 4, IL_RGBA, IL_BYTE, pSrcData);
      iluFlipImage();
      // Save out .tga
      std::string debugOutputTGAFileName = outputInfoStructure.outputFileName;
      debugOutputTGAFileName += ".ptc._tga";
      ilSave( IL_TGA, (ILstring)debugOutputTGAFileName.c_str() );
      
      // Loop over pixels, find max, min, avg
      {
         // Write out debug info file
         {
            uint32 aMinMaxAvg[kPRGBA_Total];
            CalcMinMaxAverageRGBA(pctDecompressedHandle, aMinMaxAvg);

            std::string debugOutputFileName = outputInfoStructure.outputFileName;
            debugOutputFileName += ".ptc.txt";

            WriteOutDebugInfo(debugOutputFileName, 
               aMinMaxAvg[kMaxR], aMinMaxAvg[kMinR], aMinMaxAvg[kAvgR],
               aMinMaxAvg[kMaxG], aMinMaxAvg[kMinG], aMinMaxAvg[kAvgG],
               aMinMaxAvg[kMaxB], aMinMaxAvg[kMinB], aMinMaxAvg[kAvgB],
               aMinMaxAvg[kMaxA], aMinMaxAvg[kMinA], aMinMaxAvg[kAvgA]);
         }
         // Write out original debug info file
         {
            uint32 aMinMaxAvg[kPRGBA_Total];
            CalcMinMaxAverageRGBA(tex.GetTopLevelImageHandle(), aMinMaxAvg);

            std::string debugOutputFileName = outputInfoStructure.outputFileName;
            debugOutputFileName += ".txt";

            WriteOutDebugInfo(debugOutputFileName, 
               aMinMaxAvg[kMaxR], aMinMaxAvg[kMinR], aMinMaxAvg[kAvgR],
               aMinMaxAvg[kMaxG], aMinMaxAvg[kMinG], aMinMaxAvg[kAvgG],
               aMinMaxAvg[kMaxB], aMinMaxAvg[kMinB], aMinMaxAvg[kAvgB],
               aMinMaxAvg[kMaxA], aMinMaxAvg[kMinA], aMinMaxAvg[kAvgA]);
         }
      }
      // delete our new il image handle
      ilDeleteImages(1, &pctDecompressedHandle);
   }
   // pCompressedData must be freed via the function below
   XGPTCFreeMemory( pCompressedData );

   return true;
}

bool DDS_ToDiscFormat(OutputInfoStructure const & outputInfoStructure, CTextureToCook const & tex, CDDS & dds, DDSOutputInfo & ddsOutputInfo, bool hasAlpha)
{
   if( ddsOutputInfo.discFormat == CBaseTexture::kDiscFormat_Raw )
   {
      return DDS_ToDiscFormat_Raw(outputInfoStructure, tex, dds, ddsOutputInfo, hasAlpha);
   }
   else if( ddsOutputInfo.discFormat == CBaseTexture::kDiscFormat_PTC )
   {
      return DDS_ToDiscFormat_PTC(outputInfoStructure, tex, dds, ddsOutputInfo, hasAlpha);
   }
   else
   {
      return false;
   }
}

X360TOOLSAPI bool WriteTextureX360(std::string const & outputPath, CTextureToCook const & tex, bool writeOutDebugInformation)
{
   //
   OutputInfoStructure outputInfoStructure;
   outputInfoStructure.outputFileName = outputPath;
   outputInfoStructure.writeOutDebugInformation = writeOutDebugInformation;

   // Texture is considered to have alpha if some of the pixels are not black or full white.
   // If all pixels are black or all pixels are white, the texture is NOT considered to have alpha.

   bool hasAlpha = false;
   {
      ilBindImage(tex.GetTopLevelImageHandle());      

      ILubyte* alphaBuffer = ilGetAlpha(ilGetInteger(IL_IMAGE_TYPE));

      int const width = ilGetInteger(IL_IMAGE_WIDTH);
      int const height = ilGetInteger(IL_IMAGE_HEIGHT);
      int const depth = ilGetInteger(IL_IMAGE_DEPTH);
      int const texelCount = width * height * depth;

      int whitePixels = 0;

      for( int i = 0; i < (texelCount ); ++i )
      {
         if( alphaBuffer[i] == 255 )
            whitePixels++;
      }

      if( whitePixels >= 0 && whitePixels < (texelCount) )
         hasAlpha = true;
   }

   // build dds file
   DDSOutputInfo ddsOutputInfo;
   CDDS dds;
   if( !BuildDDS(tex, &dds, ddsOutputInfo) )
   {
      return false;
   }
   // convert dds file to disc format
   if( !DDS_ToDiscFormat(outputInfoStructure, tex, dds, ddsOutputInfo, hasAlpha) )
   {
      return false;
   }
   return true;
}

/*
'TXTR' (u32)
VERSION (u32)
EDiscFormat (u32)
EFormat (u32)
WIDTH (u16)
HEIGHT (u16)
DEPTH (u16)
MIPLEVELS (u8)
HASALPHA (u8)
MINRGBA (u32)
MAXRGBA (u32)
ADDITIONALFLAGS (u32) (EAdditionalFlags)
'DATA' (u32)
TEXTURE TYPE (u32)

//Switch on EDiscFormat
//Case kDiscFormat_Raw
SIZE OF TEXTURE HEADER (u32)
TEXTURE HEADER
SIZE OF TEXTURE DATA (u32)
TEXTURE DATA

//Case kDiscFormat_Raw
SIZE OF COMPRESSED PTC (u32)
PTC TEXTURE DATA
*/
// X360TOOLSAPI bool WriteTextureX360( std::string const & outputPath, CTextureToCook const &tex )
// {
//    int x = 34;
//    tex.GetTopLevelImageHandle();
// 
//    return true;
// }