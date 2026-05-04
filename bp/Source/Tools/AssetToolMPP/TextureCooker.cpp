//----------------------------------------------------------------------------
// TextureCooker.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "AssetToolMPP.h"
//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Math/CVector3.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/CFileUtils.h"

#include "Tools/Helper/Helper.h"

#include "IL/il.h"
#include "IL/ilu.h"

#include "d3dx9tex.h"
#include "TextureCooker.h"

//----------------------------------------------------------------------------

using namespace Tools;
using namespace Tools::Property::Build;
using namespace Tools::Property::Misc;
using namespace Tools::Property::Source;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;
using namespace System::Xml::XPath;

using namespace TextureCooker;

//----------------------------------------------------------------------------

static bool assert_handler( char const * const msg )
{
   throw gcnew System::Exception( gcnew System::String( msg ) );

   return true;
}

//----------------------------------------------------------------------------

void CTextureToCook::CalcMinMaxRGBA()
{
   // Calculate the per channel min/max R, G, B, A values
   // Written out by cooker, useful for optimizing rendering
   // for a particular texture.

   uint32 minR = 255, maxR = 0;
   uint32 minG = 255, maxG = 0;
   uint32 minB = 255, maxB = 0;
   uint32 minA = 255, maxA = 0;
   uint32 minANonZero = 255;
   uint32 texelTotal = 0, texelTransparent = 0, texelTranslucent = 0, texelPunchthrough = 0;
   
   for (int textureFaceIdx = 0; textureFaceIdx < mTextureFaces.size(); textureFaceIdx++)
   {
      for( int mipLevelIdx = 0; mipLevelIdx < mTextureFaces[textureFaceIdx].mMipLevels.size(); ++mipLevelIdx )
      {
         ilBindImage(mTextureFaces[textureFaceIdx].mMipLevels[mipLevelIdx].mImageHandle);

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

            minR = min(minR, r);
            minG = min(minG, g);
            minB = min(minB, b);
            minA = min(minA, a);

            maxR = max(maxR, r);
            maxG = max(maxG, g);
            maxB = max(maxB, b);
            maxA = max(maxA, a);

            if (a >= 64) texelPunchthrough++;
            if (a > 0 && a < 64) texelTranslucent++;
            if (!a) texelTransparent++;
         }
         texelTotal += texelCount;
      }
   }

   // flag it as punchthrough if the nonzero alpha fail texels are less than 6% of the total
   // Uses hex readable RGBA i.e. '0xRRGGBBAA'
   mMinRGBA = minR << 24 | minG << 16 | minB << 8 | minA;
   mMaxRGBA = maxR << 24 | maxG << 16 | maxB << 8 | maxA;
   //printf("%d %d %d (%f %f)\n",
   //       texelTotal, texelPunchthrough, texelTranslucent, (double)texelTranslucent / texelTotal, (double)texelTranslucent / (texelTranslucent + texelPunchthrough));
   if (((double)texelTranslucent / (double)(texelTotal)) < 0.12)
   {
      mMinRGBANonZero = 64;
   }
   else
      mMinRGBANonZero = mMinRGBA;
   mbCalculatedMinMaxRGBA = true;
}

//----------------------------------------------------------------------------

bool CTextureToCook::HasAlpha() const
{
   BPE_ASSERT(mbCalculatedMinMaxRGBA, "Need to call CalcMinMaxRGBA");

   // Texture is considered to have alpha if some of the pixels are not alpha 255 (full white).
   // If all the pixels alpha values are 255, the texture is NOT considered to have alpha.

   uint8 maxA = mMaxRGBA & 0xFF;
   uint8 minA = mMinRGBA & 0xFF;

   if (maxA == minA)
   {
      if (maxA == 255)
      {
         return false;
      }
   }
   return true;
}

//----------------------------------------------------------------------------

namespace
{
//----------------------------------------------------------------------------

LPDIRECT3D9 spDirect3D = NULL;
LPDIRECT3DDEVICE9 spDevice = NULL;

void InitializeDirect3d()
{
   if( spDirect3D == NULL )
   {
      spDirect3D = Direct3DCreate9(D3D_SDK_VERSION);
   }

   if( spDevice == NULL )
   {
      D3DDISPLAYMODE mode;
      spDirect3D->GetAdapterDisplayMode(0, &mode);

      D3DPRESENT_PARAMETERS pp = { 0 };
      pp.BackBufferWidth = 1;
      pp.BackBufferHeight = 1;
      pp.BackBufferFormat = mode.Format;
      pp.BackBufferCount = 1;
      pp.SwapEffect = D3DSWAPEFFECT_COPY;
      pp.Windowed = TRUE;

      spDirect3D->CreateDevice(0, D3DDEVTYPE_NULLREF, GetDesktopWindow(), D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &(spDevice));
      if (!spDevice)
      {
         Console::WriteLine("No D3DDEVTYPE_NULLREF device.\n");
      }
   }
}

//----------------------------------------------------------------------------

void FreeDirect3d()
{
   ULONG rc1 = 0;
   ULONG rc2 = 0;

   if( spDevice != NULL )
   {
      rc1 = spDevice->Release();
      spDevice = NULL;
   }

   if( spDirect3D != NULL )
   {
      rc2 = spDirect3D->Release();
      spDirect3D = NULL; // Set to NULL otherwise D3D won't get reinitialized on the next texture cook.
   }

   if ( rc1 != 0 && rc2 != 0 )
   {
      printf( "WARNING: D3D refcounts not zero! %d %d\n", rc1, rc2 );
   }
}

//----------------------------------------------------------------------------

ILboolean LoadFunction( char* fileName )
{
   // ILboolean ilTexImage(ILuint Width, ILuint Height, ILuint Depth, ILubyte Bpp, ILenum Format, ILvoid *Data);

   /*
   If the image already has the appropriate dimensions but needs a data update, 
   just use ilSetData to let DevIL copy your copy of the image data. 
   Another function you can use to set the image data is ilSetPixels.
   */
   return IL_FALSE;
}

//----------------------------------------------------------------------------

uint32 const nextPOW2(uint32 const val)
{
   uint32 i;
   for (i = 1; i < (1<<30); i <<= 1)
   {
      if (val <= i) 
      {
         return i;
      }
   }
   return i;
}

//----------------------------------------------------------------------------

static inline real32 unsigned_byte_to_signed_real32( int32 num )
{
   return ( real32( num ) * 2.f - 255.f ) / 255.f;
}

//----------------------------------------------------------------------------

static inline int32 encode_normalmap_texel( real32 x_or_y, real32 z )
{
   // This output scale is useful if you want to allow the normal maps
   // to splay out more, sacrificing quality

   real32 const kOutputScale = 1.f;

   // Bring x or y up to the z=1 plane.  Note that divide by 0 is impossible here
   // since 0 is not represented by 0..255 converted to -1..1
   real32 ddz = x_or_y / z;

   // Let's clamp scaled output to 0.255
   real32 scaled_value = ddz / kOutputScale;
   
   // Bring the scaled value from -1..1 to 0 .. 255 or so
   scaled_value = ( ( scaled_value + 1.f ) / 2.f ) * 255.f;
   
   real32 clamped_float = bpe::min_val( 255.f, bpe::max_val( 0.f, scaled_value ) );

   // Now cast to int rounded and clamp
   return bpe::min_val( 255, bpe::max_val( 0, int( clamped_float + 0.5f ) ) );
}

//----------------------------------------------------------------------------

void ProcessImage(TextureCooker::SProcessImageOptions const &processImageOptions)
{
   // Flip image if it's oriented the wrong way around
   if( ilGetInteger(IL_IMAGE_ORIGIN) == IL_ORIGIN_LOWER_LEFT )
   {
      iluFlipImage();
   }

   int imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
   int imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
   int imageDepth = ilGetInteger( IL_IMAGE_DEPTH );

   if( processImageOptions.mColorConversion != kCC_None )
   {
      ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
      uint32* pData = (uint32*)ilGetData();
      int const bytesPerPixel = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
      for ( int z = 0; z < imageDepth; ++z )
      {
         int depthOffset = imageWidth * imageHeight * z;
         for( int y = 0; y < imageHeight; ++y )
         {
            for( int x = 0; x < imageWidth; ++x )
            {
               uint32 & data = pData[depthOffset + y * imageWidth + x];

               int32 const inputR = data & 0x000000ff;
               int32 const inputG = (data & 0x0000ff00) >> 8;
               int32 const inputB = (data & 0x00ff0000) >> 16;
               int32 const inputA = (data & 0xff000000) >> 24;

               int32 r = 0;
               int32 g = 0;
               int32 b = 0;
               int32 a = 0;

               switch(processImageOptions.mColorConversion)
               {
               case kCC_NormalMap_DXT5:
                  {
                     // This is the Insomniac style of normal maps, where the maximum splay is 45 degrees.
                     // First, let's convert the colors to float and normalize them to make them more accurate
                     CVector3 vec( unsigned_byte_to_signed_real32( inputR ), unsigned_byte_to_signed_real32( inputG ), unsigned_byte_to_signed_real32( inputB ) );

                     // Make sure that Z is positive.
                     real32 const kMinimumZ = 1.f / 255.f;
                     if ( vec.GetZ() < kMinimumZ )
                     {
                        vec.SetZ( kMinimumZ );
                     }

                     vec.Normalize();
            
                     // Now encode the y/z and x/z
                     r = 0xff;
                     g = encode_normalmap_texel( vec.GetY(), vec.GetZ() ); // y
                     b = 0;
                     a = encode_normalmap_texel( vec.GetX(), vec.GetZ() ); // x
                  }

                  break;

               case kCC_YCoCg_DXT5:
                  {
                     real32 const rFloat = inputR / 255.0f;
                     real32 const gFloat = inputG / 255.0f;
                     real32 const bFloat = inputB / 255.0f;

                     real32 const y =  0.25f * rFloat + 0.50f * gFloat + 0.25f * bFloat;
                     real32 const u =  0.50f * rFloat + 0.00f * gFloat - 0.50f * bFloat;
                     real32 const v = -0.25f * rFloat + 0.50f * gFloat - 0.25f * bFloat;
                     
                     r = bpe::min_val(255, bpe::max_val(0, (int)((u + 0.5f) * 255.0f)));
                     g = bpe::min_val(255, bpe::max_val(0, (int)((v + 0.5f) * 255.0f)));
                     b = inputA;
                     a = bpe::min_val(255, bpe::max_val(0, (int)(y * 255.0f)));
                  }
                  break;
               }

               uint32 const output = r | (g << 8) | (b << 16) | (a << 24);

               data = output;
            }
         }
      }
   }

   // Textures with mip maps must be power of 2,
   // force this if required      
   if( processImageOptions.mGenerateMipMaps != TextureCooker::kGM_No)
   {
      // TODO: Handle linear (no-gamma) texture and also verify that filtering is gamma correct for different platforms.
      int const powWidth = nextPOW2(imageWidth);
      int const powHeight = nextPOW2(imageHeight);
      int const powDepth = ( imageDepth > 1 ) ? nextPOW2( imageDepth ) : 1;

      if( powWidth != imageWidth || powHeight != imageHeight || powDepth != imageDepth )
      {
         iluImageParameter( ILU_FILTER, ILU_BILINEAR );
         iluScale( powWidth, powHeight, powDepth );
         imageWidth = powWidth;
         imageHeight = powHeight;
         imageDepth = powDepth;
      }
   }

   // Down sample texture if specified, allows us to reduce size of texture without modifing source image.
   if ( processImageOptions.mSkipMipLevels > 0 )
   {
      // TODO: Handle linear (no-gamma) texture and also verify that filtering is gamma correct for different platforms.
      int const skipLevels = processImageOptions.mSkipMipLevels;
      imageWidth >>= skipLevels;
      imageHeight >>= skipLevels;
      if ( imageDepth > 1 )
      {
         imageDepth >>= skipLevels;
      }

      iluImageParameter( ILU_FILTER, ILU_BILINEAR );
      iluScale( imageWidth, imageHeight, imageDepth );
   }
}

//----------------------------------------------------------------------------

bool ConvertColorRampTo3DTexture( std::string const & inputPath, SProcessImageOptions const &processImageOptions, CTextureToCook &tex)
{
   // Load 1D color ramp image
   ILuint handle;
   ilGenImages(1, &handle);

   ilBindImage(handle);

   if( ilLoadImage( const_cast<char*>( inputPath.c_str() ) ) == IL_FALSE )
   {
      System::Console::WriteLine("Error: {0} doesn't exist.", gcnew System::String(inputPath.c_str()));
      return false;
   }

   {
      // Skip levels and make power of 2, but don't color convert
      SProcessImageOptions crPreprocessImageOptions(processImageOptions);
      crPreprocessImageOptions.mColorConversion = kCC_None;
      ProcessImage( crPreprocessImageOptions );
   }

   // Get the first row for the color ramp
   int const width = ilGetInteger( IL_IMAGE_WIDTH );
   bpe::vector_s< uint8 > textureData;
   textureData.resize( 3 * width, 0 );
   ilCopyPixels( 0, 0, 0, width, 1, 1, IL_RGB, IL_UNSIGNED_BYTE, &( textureData[0] ) );

   bpe::vector_s<uint8> outTextureData;
   outTextureData.resize( 3 * width * width * width );

   for ( int z = 0, outIndex = 0; z < width; ++z )
   {
      for ( int y = 0; y < width; ++y )
      {
         for ( int x = 0; x < width; ++x, ++outIndex )
         {
            int realIndex = outIndex * 3;
            int adjustedy = width - 1 - y;

            // This writes the redirected x, y, and z values to the x, y, and z of the 3D texture
            outTextureData[ realIndex+0 ] = textureData[ x * 3 + 0 ];
            outTextureData[ realIndex+1 ] = textureData[ adjustedy * 3 + 1 ];
            outTextureData[ realIndex+2 ] = textureData[ z * 3 + 2 ];
         }
      }
   }

   // Now, let's bind the image to the new texture
   ilBindImage( handle );
   ilTexImage( width, width, width, 3, IL_RGB, IL_UNSIGNED_BYTE, &( outTextureData[0] ) );

   {
      // Now apply any color conversion, but don't skip levels (already done so above)
      SProcessImageOptions crReprocessImageOptions(processImageOptions);
      crReprocessImageOptions.mSkipMipLevels = 0;      
      ProcessImage( crReprocessImageOptions );
   }
   tex.AddImage(handle);
  
   return true;

}

//----------------------------------------------------------------------------

bool BuildCombinedColorWithDistanceFieldAlpha(std::string const & colorPath, 
                                              std::string const & maskPath, 
                                              real32 const userSpread, 
                                              bool const isClamp, 
                                              SProcessImageOptions const &processImageOptions, 
                                              CTextureToCook &tex)
{
   ILuint colorHandle, maskHandle;
   ilGenImages(1, &colorHandle);
   ilGenImages(1, &maskHandle);

   // load color map
   int colorWidth, colorHeight;
   {
      ilBindImage(colorHandle);

      if( ilLoadImage( const_cast<char*>( colorPath.c_str() ) ) == IL_FALSE )
      {
         System::Console::WriteLine("Error: {0} doesn't exist.", gcnew System::String(colorPath.c_str()));
         return false;
      }

      {
         // Skip levels and make power of 2, but don't color convert
         SProcessImageOptions crPreprocessImageOptions(processImageOptions);
         crPreprocessImageOptions.mColorConversion = kCC_None;
         ProcessImage( crPreprocessImageOptions );
      }      

      colorWidth = ilGetInteger(IL_IMAGE_WIDTH);
      colorHeight = ilGetInteger(IL_IMAGE_HEIGHT);
   }

   // load mask
   int maskWidth, maskHeight;
   {
      ilBindImage(maskHandle);

      if( ilLoadImage( const_cast<char*>( maskPath.c_str() ) ) == IL_FALSE )
      {
         System::Console::WriteLine("Error: {0} doesn't exist.", gcnew System::String(maskPath.c_str()));
         return false;
      }

      maskWidth = ilGetInteger(IL_IMAGE_WIDTH);
      maskHeight = ilGetInteger(IL_IMAGE_HEIGHT);
   }

   // Get data for color map
   std::vector<uint32> colorData;
   {
      colorData.resize( colorWidth * colorHeight, 0 );
      ilBindImage(colorHandle);
      ilCopyPixels( 0, 0, 0, colorWidth, colorHeight, 1, IL_RGBA, IL_UNSIGNED_BYTE, &( colorData[0] ) );
   }

   // Get data for mask
   std::vector<uint8> maskData;
   {
      maskData.resize(maskWidth * maskHeight, 0);
      ilBindImage(maskHandle);
      ilCopyPixels( 0, 0, 0, maskWidth, maskHeight, 1, IL_LUMINANCE, IL_UNSIGNED_BYTE, &( maskData[0] ) );
   }

   // Free the images as we no longer need them.
   ilDeleteImages(1, &colorHandle);
   ilDeleteImages(1, &maskHandle);

   // Generate distance field
   {
      real32 const maskWidthRatio = maskWidth / static_cast<real32>(colorWidth);
      real32 const maskHeightRatio = maskHeight / static_cast<real32>(colorHeight);

      real32 const spread = bpe::max_val(userSpread, bpe::max_val(maskWidthRatio, maskHeightRatio));

      int const maxRadius = (int)spread;

      uint32 * pColorData = &colorData.front();
      uint8* pMaskData = &maskData.front();

      for( int y = 0; y < colorHeight; ++y )
      {
         for( int x = 0; x < colorWidth; ++x )
         {
            bool foundPixel = false;
            real32 foundRadius = gkReal32Max;

            int const maskX = static_cast<int>((x + 0.5) * maskWidthRatio - 0.5f);
            int const maskY = static_cast<int>((y + 0.5) * maskHeightRatio - 0.5f);

            bool const refIsInside = pMaskData[maskY * maskWidth + maskX] > 128;

            for( int ry = -maxRadius; ry <= maxRadius; ++ry )
            {
               int curMaskY = maskY + ry;

               if( isClamp && (ry < 0 || ry >= maskHeight) )
               {
                  continue;
               }

               curMaskY %= maskHeight;

               if(curMaskY < 0 )
               {
                  curMaskY += maskHeight;
               }

               for( int rx = -maxRadius; rx <= maxRadius; ++rx )
               {
                  int curMaskX = maskX + rx;

                  if( isClamp && (rx < 0 || rx >= maskWidth) )
                  {
                     continue;
                  }

                  curMaskX %= maskWidth;

                  if(curMaskX < 0 )
                  {
                     curMaskX += maskWidth;
                  }

                  bool const curIsInside = pMaskData[curMaskY * maskWidth + curMaskX] > 128;

                  if( curIsInside != refIsInside )
                  {
                     real32 const pixelDist = sqrtf(static_cast<real32>(rx*rx + ry*ry));
                  
                     if( pixelDist < foundRadius )
                     {
                        foundPixel = true;
                        foundRadius = pixelDist;
                     }
                  }
               }
            }

            real32 normalizedValue = 0.0f;

            if( foundPixel )
            {
               normalizedValue = bpe::max_val(0.0f, bpe::min_val(1.0f, 0.5f - foundRadius / spread / 2));
            }

            if( refIsInside )
            {
               normalizedValue = 1.0f - normalizedValue;
            }

            int const actualY = (colorHeight - y) - 1;
            colorData[actualY * colorWidth + x] &= 0x00FFFFFF;
            colorData[actualY * colorWidth + x] |= uint32(normalizedValue * 255.0f) << 24;
         }
      }
   }

   // Update the color image with our new data
   {
      ILuint outputHandle;
      ilGenImages(1, &outputHandle);
      ilBindImage(outputHandle);
      ilTexImage( colorWidth, colorHeight, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, &(colorData[0]) );

      /*
      ilEnable(IL_FILE_OVERWRITE);
      ilSaveImage("C:\\distanceField.tga");
      */

      tex.AddImage(outputHandle);
   }

   return true;
}

//----------------------------------------------------------------------------

bool ConvertHeightmapToNormalmap( std::string const & inputPath, real32 const bumpHeight, SProcessImageOptions const &processImageOptions, CTextureToCook &tex )
{
   InitializeDirect3d();

   // load height map image
   ILuint handle;
   ilGenImages(1, &handle);

   ilBindImage(handle);

   if( ilLoadImage( const_cast<char*>( inputPath.c_str() ) ) == IL_FALSE )
   {
      System::Console::WriteLine("Error: {0} doesn't exist.", gcnew System::String(inputPath.c_str()));
      return false;
   }

   {
      // Skip levels and make power of 2, but don't color convert
      SProcessImageOptions crPreprocessImageOptions(processImageOptions);
      crPreprocessImageOptions.mColorConversion = kCC_None;
      ProcessImage( crPreprocessImageOptions );
   }

   // get dimensions
   int const width = ilGetInteger( IL_IMAGE_WIDTH );
   int const height = ilGetInteger( IL_IMAGE_HEIGHT );

   // create direct 3d texture for height map
   IDirect3DTexture9* pHeightMap = NULL;
   {
      uint32 const rawSize = width * height * sizeof(uint32);

      // get ARGB data (it's a little turned around in the lib)
      std::auto_ptr<char> pRawData( new char [rawSize] ); 
      ilCopyPixels( 0, 0, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, pRawData.get() );

      D3DXCreateTexture(spDevice,   
         width,     
         height,    
         D3DX_DEFAULT,
         0,          
         D3DFMT_A8R8G8B8,
         D3DPOOL_SCRATCH,
         &pHeightMap);   

      // get toplevel surface
      IDirect3DSurface9* pHeightMapSurface = NULL;
      pHeightMap->GetSurfaceLevel(0, &pHeightMapSurface);

      // load src image data into surface
      RECT rect = { 0, 0, width, height};
      D3DXLoadSurfaceFromMemory(pHeightMapSurface,                
         NULL,                
         NULL,                
         pRawData.get(),             
         D3DFMT_A8R8G8B8,     
         width * sizeof(uint32),
         NULL,                
         &rect,            
         D3DX_FILTER_NONE,    
         0);

      pHeightMapSurface->Release();
   }

   // create matching texture for normal map
   IDirect3DTexture9* pNormalMap = NULL;
   {
      D3DXCreateTexture(spDevice,   
         width,     
         height,    
         D3DX_DEFAULT,
         0,          
         D3DFMT_A8R8G8B8,
         D3DPOOL_SCRATCH,
         &pNormalMap);   
   }

   // compute normal map from height map using specified bump height
   D3DXComputeNormalMap(pNormalMap, pHeightMap, NULL, 0, D3DX_CHANNEL_LUMINANCE, bumpHeight);

   // replace current image with new normal map texture
   {
      ilBindImage(handle);

      IDirect3DSurface9* pNormalSurf = NULL;
      pNormalMap->GetSurfaceLevel(0, &pNormalSurf);

      D3DLOCKED_RECT locked;
      pNormalSurf->LockRect( &locked, NULL, 0 );
      pNormalSurf->UnlockRect();

      ilTexImage(width, height, 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, locked.pBits);
      {
         // Now apply any color conversion, but don't skip levels (already done so above)
         SProcessImageOptions crReprocessImageOptions(processImageOptions);
         crReprocessImageOptions.mSkipMipLevels = 0;      
         ProcessImage( crReprocessImageOptions );
      }

      pNormalSurf->Release();
   }

   ULONG hmRefCount = pHeightMap->Release();
   ULONG nmRefCount = pNormalMap->Release();

   if ( hmRefCount || nmRefCount )
   {
      printf( "WARNING: Heightmap/normalmap ref counts not zero! %d %d\n", hmRefCount, nmRefCount );
   }

   FreeDirect3d();

   tex.AddImage(handle);
   return true;
}

//----------------------------------------------------------------------------

bool BuildCubemap( XmlNode^ textureNode, List<AssetSystem::BuildAsset^ > ^ buildAssets, AssetSystem::Manager^ assetManager, SProcessImageOptions const &processImageOptions, CTextureToCook &tex )
{
   System::Console::WriteLine("Building Cubemap...");

   array<String^>^ imageIds = gcnew array<String^>{ "PX", "NX", "PY", "NY", "PZ", "NZ" };

   for( int i = 0; i < imageIds->Length; ++i )
   {
      XPathNavigator^ propertyNav = Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "Image" + imageIds[i]);
      if( propertyNav != nullptr )
      {
         String^ assetPath = Helpers::GetPropertyValueElement(propertyNav);
         buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset(imageIds[i],  assetPath));

         System::Console::WriteLine("Face {0}: {1}", imageIds[i], assetPath);

         std::string const filePath = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(assetPath));

         ILuint handle;
         ilGenImages(1, &handle);

         ilBindImage(handle);

         if( ilLoadImage( const_cast<char*>( filePath.c_str() ) ) == IL_FALSE )
         {
            System::Console::WriteLine("Error: {0} doesn't exist.", gcnew System::String(filePath.c_str()));
            return false;
         }

         ProcessImage(processImageOptions);

         tex.AddImage(handle);
      }
   }

   return true;
}

//----------------------------------------------------------------------------

static bool LoadImages(AssetSystem::Manager^ assetManager, 
                Tools::Property::Source::IPropertyDataSource^ propertySource, 
                List<AssetSystem::BuildAsset^ > ^buildAssets,
                System::String^ inputPath, 
                CTextureToCook &tex,                   
                SProcessImageOptions const &processImageOptions,
                Tools::AssetSystem::PlatformType::EPlatform const platform)
{
   // TODO: Add explicit mip level support to image types that aren't 2d textures.
   if( String::Compare( System::IO::Path::GetExtension( inputPath ), ".vtex", true ) == 0 )
   {
      // load property data
      XmlDocument^ vtexDoc = gcnew XmlDocument();
      vtexDoc->Load(inputPath);

      // merge property data
      MergePropertiesV2::Merge_Inplace(vtexDoc, propertySource);

      String^ textureType = vtexDoc->SelectSingleNode("//PropertyContainer/ComponentGroup/@activeComponent")->InnerText;
      XmlNode^ textureNode = vtexDoc->SelectSingleNode(String::Format("//PropertyContainer/ComponentGroup/Component[@id='{0}']", textureType));
      if( textureType == "AssembleCubemap" )
      {
         if ( !BuildCubemap(textureNode, buildAssets, assetManager, processImageOptions, tex) )
         {
            return false;
         }

      }
      else if( textureType == "HeightToNormalmap" )
      {
         System::Console::WriteLine("Converting height map to normal map...");

         String^ assetPath = Helpers::GetPropertyValueElement(Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "Image"));
         buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("HeightMap",  assetPath));
         std::string const filePath = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(assetPath));

         real32 const bumpHeight = Single::Parse(Helpers::GetPropertyValueElement(Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "BumpScale")));

         if( !ConvertHeightmapToNormalmap(filePath, bumpHeight, processImageOptions, tex ) )
         {
            return false;
         }
      }
      else if ( textureType == "ColorRampTo3DTexture" )
      {
         System::Console::WriteLine( "Converting color ramp to 3D texture..." );
         String^ assetPath = Helpers::GetPropertyValueElement(Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "Image"));
         buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("ColorRamp",  assetPath));
         std::string const filePath = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(assetPath));

         if( !ConvertColorRampTo3DTexture(filePath, processImageOptions, tex) )
         {
            return false;
         }
      }
      else if ( textureType == "CombineColorWithDistanceFieldAlpha" )
      {
         System::Console::WriteLine( "Building color map with alpha channel generated from highres alpha mask..." );

         String^ colorPathStr = Helpers::GetPropertyValueElement(Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "Color"));
         String^ maskPathStr = Helpers::GetPropertyValueElement(Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "HighresAlphaMask"));
         real32 const userSpread = (real32)System::Double::Parse( Helpers::GetPropertyValueElement(Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "Spread")) );
         bool const isClamp = Tools::Common::Misc::ParseBool( Helpers::GetPropertyValueElement(Helpers::GetPropertyNavForPath(textureNode->CreateNavigator(), "Clamped")) );

         buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("Color", colorPathStr));
         buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("Mask", maskPathStr));

         std::string const colorPath = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(colorPathStr));
         std::string const maskPath = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(maskPathStr));

         if( !BuildCombinedColorWithDistanceFieldAlpha(colorPath, maskPath, userSpread, isClamp, processImageOptions, tex) )
         {
            return false;
         }
      }
      else
      {
         System::Console::WriteLine("Error: Unknown virtual texture type '{0}'", textureType);
      }
   }
   else
   {
      // Regular 2D texture.
      std::string const inputPathStr(Helper::StringHelper::ConvertString(inputPath));

      // First, check on an xml meta
      std::string xmlMetaPath = CFileUtils::GetFullPathMinusExtension( inputPathStr ) + ".xmlmeta";

      if ( CFileUtils::DoesExist( xmlMetaPath ) )
      {
         buildAssets->Add( AssetSystem::BuildAsset::StandardCookingAsset( "XmlMeta", 
            assetManager->GetRepositoryRelativePath( gcnew String( xmlMetaPath.c_str() ) ) ) );
      }
      else
      {
         // would love to force a cook if the xmlmeta exists, but thie version of the BP engine doesn't
         // have that ability.
      }

      // Load main image and also load explicit mip map levels if available (specified by _MM# appended to filename).
      int const texFaceIndex = tex.mTextureFaces.size();
      int const kMaxMips = 12;   // 4k pixels
      for (int mipLevel = 0; mipLevel < 12; mipLevel++)
      {
         ILuint imageHandle;
         ilGenImages(1, &imageHandle);
         ilBindImage(imageHandle);

         if (mipLevel == 0)
         {
            // Create new top level image
            tex.AddImage(imageHandle);

            if( ilLoadImage(const_cast<char*>(inputPathStr.c_str())) == IL_FALSE )
            {
               ilDeleteImages(1, &imageHandle);
               // Must have top level image
               System::Console::WriteLine("Error: {0} doesn't exist.", inputPath);
               return false;
            }

            buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset("Mipmap0", assetManager->GetRepositoryRelativePath( inputPath ) ));

            tex.SetNormalImageInputPath( inputPathStr.c_str(), ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ) );
         }
         else
         {
            // TODO: To determine how to handle skipping levels correctly as currently we will 
            // just downsample all mips. This might be the behavior we want though for explicit 
            // alpha and coloring of mips.

            std::string const mipLevelPath(CStringExtras::Stringize("%s_MM%d%s", 
                  CFileUtils::GetFullPathMinusExtension(inputPathStr).c_str(),
                  mipLevel,
                  CFileUtils::GetFileExtension(inputPathStr).c_str()));

            if( ilLoadImage(const_cast<char*>(mipLevelPath.c_str())) == IL_FALSE )
            {
               ilDeleteImages(1, &imageHandle);
               // No mip level, try next
               continue;
            }

            // Custom mip level, no longer a normal image
            tex.ClearNormalImageInputPath();

            // Add a new mip level
            CTexMipMap mipMap(imageHandle, mipLevel);
            CTextureFace &texFace = tex.mTextureFaces[texFaceIndex];
            texFace.mMipLevels.push_back(mipMap);

            buildAssets->Add(AssetSystem::BuildAsset::StandardCookingAsset(
               String::Format( "Mipmap{0}", mipLevel ), 
               assetManager->GetRepositoryRelativePath( gcnew String( mipLevelPath.c_str() ) ) ) );
         }

         // Safe to process image after creation as images are bound by handle.
         ProcessImage(processImageOptions);
      }
   }

   tex.CalcMinMaxRGBA();

   return true;
}

} // anon namespace

//----------------------------------------------------------------------------

static bool cook_jpeg_txtr(System::String^ inputPath, System::String^ outputPath, AssetSystem::Manager^ assetManager)
{
   if( String::Compare( System::IO::Path::GetExtension( inputPath ), ".jpg", true ) == 0 )
   {
      // Just copy the file to .txtr
      System::IO::File::Copy(assetManager->GetSystemPath(inputPath), assetManager->GetSystemPath(outputPath), true);
      // Remove read only flag
      System::IO::File::SetAttributes(assetManager->GetSystemPath(outputPath), System::IO::FileAttributes::Normal);
      // Touch file so it's newer than original so it doesn't need recooking
      System::IO::File::SetLastWriteTime(assetManager->GetSystemPath(outputPath), System::DateTime::Now);
      return true;
   }

   std::string const inputFilename = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(inputPath));
   std::string const outputFilename = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(outputPath));

   std::string commandLine = CStringExtras::Stringize("im_convert %s  -quality 89 JPEG:%s", inputFilename.c_str(), outputFilename.c_str());
   int result = system(commandLine.c_str());

   return (result == 0);
}

//----------------------------------------------------------------------------

static bool cook_png_txtr(System::String^ inputPath, System::String^ outputPath, AssetSystem::Manager^ assetManager)
{
   if( String::Compare( System::IO::Path::GetExtension( inputPath ), ".png", true ) == 0 )
   {
      // Just copy the file to .txtr
      System::IO::File::Copy(assetManager->GetSystemPath(inputPath), assetManager->GetSystemPath(outputPath), true);
      // Remove read only flag
      System::IO::File::SetAttributes(assetManager->GetSystemPath(outputPath), System::IO::FileAttributes::Normal);
      // Touch file so it's newer than original so it doesn't need recooking
      System::IO::File::SetLastWriteTime(assetManager->GetSystemPath(outputPath), System::DateTime::Now);
      return true;
   }

   std::string const inputFilename = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(inputPath));
   std::string const outputFilename = Helper::StringHelper::ConvertString(assetManager->GetSystemPath(outputPath));

   std::string commandLine = CStringExtras::Stringize("im_convert %s PNG:%s", inputFilename.c_str(), outputFilename.c_str());
   int result = system(commandLine.c_str());

   return (result == 0);
}

//----------------------------------------------------------------------------

static std::vector<char> GenerateRGBThumbnailData( ILuint const handle, int const dstWidth, int const dstHeight )
{
   ILuint tmpImage = 0;
   ilGenImages( 1, &tmpImage );
   ilBindImage( tmpImage );
   ilCopyImage( handle );
   iluImageParameter( ILU_FILTER, ILU_BILINEAR );
   iluScale( dstWidth, dstHeight, 1 );

   std::vector<char> out;
   out.resize( dstWidth * dstHeight * 3, 0 );
   ilCopyPixels( 0, 0, 0, dstWidth, dstHeight, 1, IL_BGR, IL_UNSIGNED_BYTE, &( out[0] ) );

   ilBindImage(handle);
   ilDeleteImages( 1, &tmpImage );

   return out;
}

//----------------------------------------------------------------------------

static String^ GenerateCookedAssetShortDesc( ILuint const handle, SProcessImageOptions const & processImageOptions )
{
   ilBindImage( handle );

   int const fullWidth = ilGetInteger( IL_IMAGE_WIDTH );
   int const fullHeight = ilGetInteger( IL_IMAGE_HEIGHT );

   String^ textureFormat = gcnew String(CBaseTexture::TextureFormatAsString(processImageOptions.mTexFormat));
   String^ generateMips = gcnew String(processImageOptions.mGenerateMipMaps ? "Y" : "N");
   String^ type;
   switch(processImageOptions.mColorConversion )
   {
   case kCC_NormalMap_DXT5:
      type = "Normalmap ";
      break;

   case kCC_YCoCg_DXT5:
      type = "YCoCg ";
      break;
   
   case kCC_None:
      type = "";
      break;
   
   default:
      type = "Unknown";
      break;
   }
   return String::Format( "{0}x{1} {2} {3}Mipmap: {4} ", fullWidth, fullHeight, textureFormat, type, generateMips );
}

//----------------------------------------------------------------------------

static System::Xml::XmlElement ^GenerateThumbnailXMLNode( XmlDocument ^document, ILuint const handle, int const dstWidth, int const dstHeight )
{
   XmlElement ^element = document->CreateElement( "Thumbnail" );
   element->SetAttribute( "width", dstWidth.ToString() );
   element->SetAttribute( "height", dstHeight.ToString() );
   element->SetAttribute( "format", "RGB8" );
   element->SetAttribute( "version", "1" );
   
   std::vector< char > thumbData = GenerateRGBThumbnailData( handle, dstWidth, dstHeight );
   System::IntPtr thumbDataPtr( &( thumbData[0] ) );
   cli::array< unsigned char > ^managedArray = gcnew cli::array< unsigned char >( thumbData.size() );
   System::Runtime::InteropServices::Marshal::Copy( thumbDataPtr, managedArray, 0, thumbData.size() );

   element->InnerText = System::Convert::ToBase64String( managedArray );

   return element;
}

//----------------------------------------------------------------------------

static bool WriteTextureCookedInfo( System::String^ outputPath, SProcessImageOptions const &processImageOptions, CTextureToCook const &tex )
{
   ILuint imageHandle = tex.GetTopLevelImageHandle();

   XmlDocument ^doc = gcnew XmlDocument();
   XmlElement ^root = AssetSystem::CookedAssetInfo::CreateRootElement( doc, 1 );
   //XmlElement ^thumbnail = GenerateThumbnailXMLNode( doc, imageHandle, 32, 32 );
   //root->AppendChild( thumbnail );
   doc->AppendChild( root );

   AssetSystem::CookedAssetInfo::SetShortDescription( root, GenerateCookedAssetShortDesc( imageHandle, processImageOptions ) );

   AssetSystem::CookedAssetInfo::Save( outputPath, doc );

   return true;
}

//----------------------------------------------------------------------------

static int GetAdjustHint(System::String^ value)
{
   if( String::Compare(value, "kCenter", true ) == 0 )
   {
      return CBaseTexture::kAH_Center;
   }
   else if( String::Compare(value, "kLeft", true ) == 0 )
   {
      return CBaseTexture::kAH_Left;
   }
   else if( String::Compare(value, "kRight", true ) == 0 )
   {
      return CBaseTexture::kAH_Right;
   }

   return CBaseTexture::kAH_None;
}

bool AssetToolMPP::Cookers::CookTXTR( System::String^ inputPath, System::String^ outputPath, AssetSystem::Manager^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform, bool debugInfo )
{
   BPE_SET_ASSERT_POPUP_FUNC( assert_handler );

   // Create directory for cooked file.
   {
      String^ directory = System::IO::Path::GetDirectoryName(outputPath);
      if (!System::IO::Directory::Exists(directory))
         System::IO::Directory::CreateDirectory(directory);
   }


   FileBasedPropertyObjectEnumerator ^ propertySource = gcnew FileBasedPropertyObjectEnumerator();
   propertySource->AddFiles(FileBasedPropertyObjectEnumerator::FileSearchParams::DefaultFileSearchParams(assetManager));

   XmlDocument^ metaData = AssetSystem::MetaData::GetMetaData(inputPath, assetManager, propertySource);

   SProcessImageOptions processImageOptions;

   if( metaData != nullptr )
   {
      // if meta data is present normal map compression is based on the format
      processImageOptions.mColorConversion = kCC_None;      

      String^ genMips = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "gen_mips");
      if( genMips != nullptr )
      {
         if( String::Compare(genMips, "kDefault", true ) == 0 )
         {
            // Default is mipmaps on
            processImageOptions.mGenerateMipMaps = TextureCooker::kGM_Yes;
         }
         else if( String::Compare(genMips, "kForceOff", true ) == 0 )
         {
            processImageOptions.mGenerateMipMaps = TextureCooker::kGM_No;
         }
         else if( String::Compare(genMips, "kForceOn", true ) == 0 )
         {
            processImageOptions.mGenerateMipMaps = TextureCooker::kGM_Yes;
         }

         String^ skipLevelsStr = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "skip_levels");
         if( skipLevelsStr != nullptr )
         {
            Int32::TryParse(skipLevelsStr, processImageOptions.mSkipMipLevels);
         }
      }

      String^ mipmapSelectionType = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "mipmap_selection_type");
      if( mipmapSelectionType != nullptr )
      {
         if( String::Compare(mipmapSelectionType, "kPS2", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kAF_UsePS2MipmapSelection;
         }
      }

      String^ textureFilterType = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "texture_filter");
      if( textureFilterType != nullptr )
      {
         if( String::Compare(textureFilterType, "kDefault", true ) == 0 )
         {
            processImageOptions.mFilterHint = CBaseTexture::kFH_Default;
         }
         else if( String::Compare(textureFilterType, "kPoint", true ) == 0 )
         {
            processImageOptions.mFilterHint = CBaseTexture::kFH_Point;
         }
         else if( String::Compare(textureFilterType, "kBilinear", true ) == 0 )
         {
            processImageOptions.mFilterHint = CBaseTexture::kFH_Bilinear;
         }
         else if( String::Compare(textureFilterType, "kTrilinear", true ) == 0 )
         {
            processImageOptions.mFilterHint = CBaseTexture::kFH_Trilinear;
         }
         else if( String::Compare(textureFilterType, "kAniso", true ) == 0 )
         {
            processImageOptions.mFilterHint = CBaseTexture::kFH_Aniso;
         }
      }

      String^ alphaRefStr = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "alpha_ref_override");
      if( alphaRefStr != nullptr )
      {
         Int32::TryParse(alphaRefStr, processImageOptions.mAlphaRefValue);
      }

      String^ alphaFailHintType = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "alpha_fail_hint");
      if( alphaFailHintType != nullptr )
      {
         if( String::Compare(alphaFailHintType, "kDisable", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kAF_DisableAlphaFailHint;
         }
      }

      String^ textureMemory = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "texture_memory");
      if( textureMemory != nullptr )
      {
         if( String::Compare(textureMemory, "kSystem", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kAF_SystemMemory;
         }
      }

      String^ maxLodOffsetStr = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "max_lod_offset");
      if( maxLodOffsetStr != nullptr )
      {
         Int32::TryParse(maxLodOffsetStr, processImageOptions.mMaxLODOffset);
      }

      String^ textureAddressModeU = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "texture_address_mode_u");
      if( textureAddressModeU != nullptr )
      {
         if( String::Compare(textureAddressModeU, "kClamp", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kAF_ForceClampU;
         }
         else if( String::Compare(textureAddressModeU, "kBorder", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kAF_ForceBorderU;
         }
      }

      String^ textureAddressModeV = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "texture_address_mode_v");
      if( textureAddressModeV != nullptr )
      {
         if( String::Compare(textureAddressModeV, "kClamp", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kAF_ForceClampV;
         }
         else if( String::Compare(textureAddressModeV, "kBorder", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kAF_ForceBorderV;
         }
      }

      processImageOptions.mAdditionalFlags |= Tools::Common::Misc::ParseBool(Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "force_disable_clut_lerp"), false) ? CBaseTexture::kAF_ForceDisableClutLerp : 0;

      processImageOptions.mAdditionalFlags |= Tools::Common::Misc::ParseBool(Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "force_disable_depth_write"), false) ? CBaseTexture::kAF_ForceDisableDepthWrite : 0;


      String^ sizeHint = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "size_hint");
      if( sizeHint != nullptr )
      {
         if( String::Compare(sizeHint, "kPixelPerfect", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kSH_PixelPerfect << CBaseTexture::kAF_SizeHintBeginBit;
         }
         else if( String::Compare(sizeHint, "kAspectCorrect", true ) == 0 )
         {
            processImageOptions.mAdditionalFlags |= CBaseTexture::kSH_AspectCorrect << CBaseTexture::kAF_SizeHintBeginBit;
         }
      }

      String^ horizAdjustHint = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "h_adjust_hint");
      if( horizAdjustHint != nullptr )
         processImageOptions.mAdditionalFlags |= GetAdjustHint(horizAdjustHint) << CBaseTexture::kAF_HorizJustificationBeginBit;

      String^ outputType = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "texture_format");

      // On RVL, we can override texture formats
      if ( platform == AssetSystem::PlatformType::EPlatform::kRVL )
      {
         String^ rvlOutputType = Tools::Property::Misc::Helpers::GetValueAtLabel( metaData, "rvl_texture_format" );
         if ( rvlOutputType != "kInherit" && rvlOutputType != "" )
         {
            outputType = rvlOutputType;
         }
      }

      if( outputType != nullptr )
      {
         if( String::Compare(outputType, "kUncompressed", true ) == 0 )
         {
            processImageOptions.mTexFormat = CBaseTexture::kFormat_A8R8G8B8;
         }
         else if( String::Compare(outputType, "kDXT1", true ) == 0 )
         {
            processImageOptions.mTexFormat = CBaseTexture::kFormat_DXT1;
         }
         else if( String::Compare(outputType, "kDXT3", true ) == 0 )
         {
            processImageOptions.mTexFormat = CBaseTexture::kFormat_DXT3;
         }
         else if( String::Compare(outputType, "kDXT5", true ) == 0 )
         {
            processImageOptions.mTexFormat = CBaseTexture::kFormat_DXT5;
         }
         else if( String::Compare(outputType, "kJPEG", true ) == 0 )
         {
            // Special case, cook as a JPEG (convert to jpg, rename to .txtr)
            return cook_jpeg_txtr(inputPath, outputPath, assetManager);
         }
         else if( String::Compare(outputType, "kPNG", true ) == 0 )
         {
            // Special case, cook as a PNG (convert to png, rename to .txtr)
            return cook_png_txtr(inputPath, outputPath, assetManager);
         }
         else if( String::Compare(outputType, "kNormalDXT5", true) == 0 )
         {
            processImageOptions.mColorConversion = kCC_NormalMap_DXT5;
         }
         else if( String::Compare(outputType, "kLightmapDXT5", true) == 0 )
         {
            processImageOptions.mColorConversion = kCC_YCoCg_DXT5;
         }
         else
         {
            System::Console::WriteLine("Error: Unknown texture format '{0}'", outputType);
            return false;
         }
      }

      String^ discFormat = Tools::Property::Misc::Helpers::GetValueAtLabel(metaData, "disc_format");
      if( discFormat != nullptr )
      {
         if( String::Compare(discFormat, "kRaw", true ) == 0 )
         {
            processImageOptions.mDiscFormat = CBaseTexture::kDiscFormat_Raw;
         }
         else if( String::Compare(discFormat, "kPTC", true ) == 0 )
         {
            processImageOptions.mDiscFormat = CBaseTexture::kDiscFormat_PTC;
         }
      }
   }

   // Force texture format based on color conversion
   switch( processImageOptions.mColorConversion )
   {
   case kCC_NormalMap_DXT5:
      processImageOptions.mTexFormat = CBaseTexture::kFormat_DXT5;
      break;
   case kCC_YCoCg_DXT5:
      processImageOptions.mTexFormat = CBaseTexture::kFormat_DXT5;
      break;
   
   default:
      break;
   }

   try
   {

      ilInit();

      CTextureToCook tex;
      tex.mGenerateMipMaps = processImageOptions.mGenerateMipMaps;
      tex.mAdditionalFlags = processImageOptions.mAdditionalFlags;
      tex.mFormat = processImageOptions.mTexFormat;
      tex.mDiscFormat = processImageOptions.mDiscFormat;
      tex.mAlphaRefValue = processImageOptions.mAlphaRefValue;
      tex.mFilterHint = processImageOptions.mFilterHint;

      List<AssetSystem::BuildAsset^ > ^buildAssets = gcnew List<AssetSystem::BuildAsset^ >();

      if( !LoadImages(assetManager, propertySource, buildAssets, inputPath, tex, processImageOptions, platform) )
         return false;
      
      std::string outputPathStr = Helper::StringHelper::ConvertString(outputPath);

      switch(platform)
      {
      case Tools::AssetSystem::PlatformType::EPlatform::kWin32:
         if( !TextureCooker::WriteTextureWin32(outputPathStr, tex) )
            return false;
         break;
      
      case Tools::AssetSystem::PlatformType::EPlatform::kPS3:
         if( !TextureCooker::WriteTexturePS3(outputPathStr, tex) )
            return false;
         break;

      case Tools::AssetSystem::PlatformType::EPlatform::kRVL:
         //if( !TextureCooker::WriteTextureRVL(outputPathStr, tex) )
            return false;
         break;

      case Tools::AssetSystem::PlatformType::EPlatform::kX360:
         if( !TextureCooker::WriteTextureX360(outputPathStr, tex, debugInfo) )
            return false;
         break;

      case Tools::AssetSystem::PlatformType::EPlatform::kVita:
         if( !TextureCooker::WriteTextureVTA(outputPathStr, tex) )
            return false;
         break;

      default:
         Console::WriteLine( "Unknown platform {0}!", platform );
         return false;

      }

      AssetSystem::BuildAssets::WriteAssets( inputPath, buildAssets, assetManager, platform );

      WriteTextureCookedInfo( outputPath, processImageOptions, tex);

      {
         // Delete all image handles
         foreach(TextureCooker::CTextureFace &texFace, tex.mTextureFaces)
         {
            foreach(TextureCooker::CTexMipMap &mipImage, texFace.mMipLevels)
            {
               ILuint imageHandle = mipImage.mImageHandle;
               ilDeleteImages(1, &imageHandle);
            }
         }
      }
   }
   finally
   {
      FreeDirect3d();
      ilShutDown();
   }

   return true;
}

