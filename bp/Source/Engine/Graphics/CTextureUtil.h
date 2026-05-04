//------------------------------------------------------------------------------------------
// CTextureUtil.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CTextureUtil
{
public:
   enum EAlpha
   {
      kAlpha_One,
      kAlpha_Zero,
      kAlpha_Keep,

      kAlpha_Count,
      
      kAlpha_FirstValid = 0,
      kAlpha_LastValid = kAlpha_Count - 1,
      kAlpha_Invalid = -1
   };

   //static void ConvertTextureData( const CTexture::EFormat srcFormat, const uint8* const pSrcData, const CTexture::EFormat dstFormat, uint8* const pDstData, const int32 numPixels );
   //static inline const uint32 GetPixelByteSize(const CTexture::EFormat format);

private:
   // from ARGB32 to ARGB32
   ENGINE_API static void ConvertARGB32ToARGB32( const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

   // from ARGB32 to ARGB16
   ENGINE_API static void ConvertARGB32ToARGB16( const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );
   // from ARGB16 to ARGB32
   ENGINE_API static void ConvertARGB16ToARGB32( const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

   // from ARGB32 to A1R5G5B5
   ENGINE_API static void ConvertARGB32ToA1R5G5B5(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );
   // from A1R5G5B5 to ARGB32
   ENGINE_API static void ConvertA1R5G5B5ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

   // from ARGB32 to RGB24
   ENGINE_API static void ConvertARGB32ToRGB24(const uint8* pSrcData, uint8* pDstData, const int32 numPixels );
   // from RGB24 to ARGB32
   ENGINE_API static void ConvertRGB24ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

   // from ARGB32 to RGB24
   ENGINE_API static void ConvertARGB32ToR5G6B5(const uint8* pSrcData, uint8* pDstData, const int32 numPixels );
   // from RGB24 to ARGB32
   ENGINE_API static void ConvertR5G6B5ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

   // from ARGB32 to L8
   ENGINE_API static void ConvertARGB32ToL8(const uint8* pSrcData, uint8* pDstData, const int32 numPixels );
   // from L8 to ARGB32
   ENGINE_API static void ConvertL8ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

   // from ARGB32 to A8L8
   ENGINE_API static void ConvertARGB32ToA8L8(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );
   // from A8L8 to ARGB32
   ENGINE_API static void ConvertA8L8ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

   // from ARGB32 to A4L4
   ENGINE_API static void ConvertARGB32ToA4L4(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );
   // from A4L4 to ARGB32
   ENGINE_API static void ConvertA4L4ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode );

};

//------------------------------------------------------------------------------------------

/*
const uint32 CTextureUtil::GetPixelByteSize(const CTexture::EFormat format)
{
   switch( format )
   {
   // 32bit formats
   case CTexture::kFormat_A8R8G8B8:
   case CTexture::kFormat_X8R8G8B8:
      {
         return 4;
      }
      break;
   // 24bit formats
   case CTexture::kFormat_R8G8B8:
      {
         return 3;
      }
      break;

   // 16bit formats
   case CTexture::kFormat_R5G6B5:
   case CTexture::kFormat_X1R5G5B5:
   case CTexture::kFormat_A1R5G5B5:
   case CTexture::kFormat_X4R4G4B4:
   case CTexture::kFormat_A4R4G4B4:
   case CTexture::kFormat_A8L8:
      {
         return 2;
      }
      break;

   // 8bit formats
   case CTexture::kFormat_L8:
   case CTexture::kFormat_A4L4:
      {
         return 1;
      }
      break;

   default:
      BPE_ASSERTA("Unknown texture format");
      return 0;
   }
}
*/
//------------------------------------------------------------------------------------------
