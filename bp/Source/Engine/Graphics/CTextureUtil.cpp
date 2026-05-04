//------------------------------------------------------------------------------------------
// CTextureUtil.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CTextureUtil.h"

#if 0
//------------------------------------------------------------------------------------------

const real32 gkRedToGrey = 0.299f;
const real32 gkGreenToGrey = 0.587f;
const real32 gkBlueToGrey = 0.114f;

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertTextureData( const CTexture::EFormat srcFormat, const uint8* const pSrcData, const CTexture::EFormat dstFormat, uint8* const pDstData, const int32 numPixels )
{
   if( srcFormat == CTexture::kFormat_A8R8G8B8 )
   {
      switch(dstFormat)
      {
      case CTexture::kFormat_X8R8G8B8:
         ConvertARGB32ToARGB32(pSrcData, pDstData, numPixels, kAlpha_Zero);
         break;
      case CTexture::kFormat_A8R8G8B8:
         ConvertARGB32ToARGB32(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_X4R4G4B4:
         ConvertARGB32ToARGB16(pSrcData, pDstData, numPixels, kAlpha_Zero);
         break;
      case CTexture::kFormat_A4R4G4B4:
         ConvertARGB32ToARGB16(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_X1R5G5B5:
         ConvertARGB32ToA1R5G5B5(pSrcData, pDstData, numPixels, kAlpha_Zero);
         break;
      case CTexture::kFormat_A1R5G5B5:
         ConvertARGB32ToA1R5G5B5(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_R8G8B8:
         ConvertARGB32ToRGB24(pSrcData, pDstData, numPixels);
         break;
      case CTexture::kFormat_R5G6B5:
         ConvertARGB32ToR5G6B5(pSrcData, pDstData, numPixels);
         break;
      case CTexture::kFormat_L8:
         ConvertARGB32ToL8(pSrcData, pDstData, numPixels);
         break;
      case CTexture::kFormat_A8L8:
         ConvertARGB32ToA8L8(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_A4L4:
         ConvertARGB32ToA4L4(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      default:
         BPE_ASSERTA("unknown dest texture type");
         break;
      }
   }
   else if( dstFormat == CTexture::kFormat_A8R8G8B8 )
   {
      switch(srcFormat)
      {
      case CTexture::kFormat_X8R8G8B8:
         ConvertARGB32ToARGB32(pSrcData, pDstData, numPixels, kAlpha_One);
         break;
      case CTexture::kFormat_A8R8G8B8:
         ConvertARGB32ToARGB32(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_X4R4G4B4:
         ConvertARGB16ToARGB32(pSrcData, pDstData, numPixels, kAlpha_One);
         break;
      case CTexture::kFormat_A4R4G4B4:
         ConvertARGB16ToARGB32(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_X1R5G5B5:
         ConvertA1R5G5B5ToARGB32(pSrcData, pDstData, numPixels, kAlpha_One);
         break;
      case CTexture::kFormat_A1R5G5B5:
         ConvertA1R5G5B5ToARGB32(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_R5G6B5:
         ConvertR5G6B5ToARGB32(pSrcData, pDstData, numPixels, kAlpha_One);
         break;
      case CTexture::kFormat_R8G8B8:
         ConvertRGB24ToARGB32(pSrcData, pDstData, numPixels, kAlpha_One);
         break;
      case CTexture::kFormat_L8:
         ConvertL8ToARGB32(pSrcData, pDstData, numPixels, kAlpha_One);
         break;
      case CTexture::kFormat_A8L8:
         ConvertA8L8ToARGB32(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      case CTexture::kFormat_A4L4:
         ConvertA4L4ToARGB32(pSrcData, pDstData, numPixels, kAlpha_Keep);
         break;
      default:
         BPE_ASSERTA("unknown dest texture type");
         break;
      }
   }
   else
   {
      BPE_ASSERTA( "source OR dest MUST be A8R8G8B8" );
   }
}

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertARGB32ToARGB32( const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);
   const uint32 mask = (alphaMode == kAlpha_Keep) ? 0xffffffff : 0x00ffffff;
   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;

   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      *pDest++ = (*pSource++ & mask) | or;
   }
}

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertARGB32ToARGB16( const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint16* pDest = reinterpret_cast<uint16*>(pDstData);
   const uint16 mask = (alphaMode == kAlpha_Keep) ? 0xffff : 0x0fff;
   const uint16 or = (alphaMode == kAlpha_One) ? 0xf000 : 0x0000;
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = ( static_cast<const int16>( ( ( sourceVal & 0xf0000000 ) >> 16) |
                                             ( ( sourceVal & 0x00f00000 ) >> 12) |
                                             ( ( sourceVal & 0x0000f000 ) >> 8) |
                                             ( ( sourceVal & 0x000000f0 ) >> 4) ) 
                                             & mask) | or;
   }
}

void CTextureUtil::ConvertARGB16ToARGB32( const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint16* pSource = reinterpret_cast<const uint16*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);
   const uint32 mask = (alphaMode == kAlpha_Keep) ? 0xffffffff : 0x00ffffff;
   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;

   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = ( static_cast<const int32>( ( ( sourceVal & 0xf000 ) << 16) |
                                             ( ( sourceVal & 0x0f00 ) << 12) |
                                             ( ( sourceVal & 0x00f0 ) << 8 ) |
                                             ( ( sourceVal & 0x000f ) << 4 ) ) 
                                             & mask) | or;
   }
}

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertARGB32ToA1R5G5B5(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint16* pDest = reinterpret_cast<uint16*>(pDstData);
   const uint16 mask = (alphaMode == kAlpha_Keep) ? 0xffff : 0x7fff;
   const uint16 or = (alphaMode == kAlpha_One) ? 0x8000 : 0x0000;
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = ( static_cast<const int16>( ( ( sourceVal & 0xff000000 ) ? 0x8000 : 0x0000) |
                                             ( ( sourceVal & ( 31 << 19 ) ) >> 9 ) |
                                             ( ( sourceVal & ( 31 << 11 ) ) >> 6 ) |
                                             ( ( sourceVal & ( 31 << 3  ) ) >> 3 ) ) 
                                             & mask) | or;
   }
}

void CTextureUtil::ConvertA1R5G5B5ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint16* pSource = reinterpret_cast<const uint16*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);
   const uint32 mask = (alphaMode == kAlpha_Keep) ? 0xffffffff : 0x00ffffff;
   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = ( static_cast<const int32>( ( ( sourceVal & 0x8000 ) ? 0xff000000 : 0x00000000) |
                                             ( ( sourceVal & ( 31 << 10 ) ) << 9 ) |
                                             ( ( sourceVal & ( 31 << 5  ) ) << 6 ) |
                                             ( ( sourceVal & ( 31       ) ) << 3 ) ) 
                                             & mask) | or;
   }
}

//------------------------------------------------------------------------------------------

#pragma BPE_TODOMSG("byte order?")

void CTextureUtil::ConvertARGB32ToRGB24(const uint8* pSrcData, uint8* pDstData, const int32 numPixels )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint8* pDest = reinterpret_cast<uint8*>(pDstData);
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const uint8>( sourceVal & 0x000000ff );
      *pDest++ = static_cast<const uint8>( (sourceVal & 0x0000ff00) >> 8 );
      *pDest++ = static_cast<const uint8>( (sourceVal & 0x00ff0000) >> 16 );
   }
}

void CTextureUtil::ConvertRGB24ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   BPE_ASSERT( alphaMode != kAlpha_Keep, "no alpha information in source" );

   const uint8* pSource = reinterpret_cast<const uint8*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);
   
   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;

   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint8 val1 = *pSource++;
      const uint8 val2 = *pSource++;
      const uint8 val3 = *pSource++;

      *pDest++ = static_cast<const uint32>(val1) |
                 (static_cast<const uint32>(val2) << 8) |
                 (static_cast<const uint32>(val3) << 16) |
                 or;
   }

}

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertARGB32ToL8(const uint8* pSrcData, uint8* pDstData, const int32 numPixels )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint8* pDest = reinterpret_cast<uint8*>(pDstData);
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const uint8>(
         ( ( (sourceVal & 0x00ff0000) >> 16) * gkRedToGrey 
         + ( (sourceVal & 0x0000ff00) >> 8) * gkGreenToGrey 
         + ( (sourceVal & 0x000000ff) ) * gkBlueToGrey ) 
         );
   }
}

void CTextureUtil::ConvertL8ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode  )
{
   BPE_ASSERT( alphaMode != kAlpha_Keep, "no alpha information in source" );

   const uint8* pSource = reinterpret_cast<const uint8*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);
   
   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;

   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint8 sourceVal = *pSource++;

      *pDest++ = (static_cast<const uint32>(sourceVal) << 16 ) |
                 (static_cast<const uint32>(sourceVal) << 8 ) |
                 (static_cast<const uint32>(sourceVal)) |
                 or;
   }
}

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertARGB32ToR5G6B5(const uint8* pSrcData, uint8* pDstData, const int32 numPixels )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint16* pDest = reinterpret_cast<uint16*>(pDstData);
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const int16>( ( ( sourceVal & ( 31 << 19 ) ) >> 8 ) |
                                           ( ( sourceVal & ( 63 << 10 ) ) >> 5 ) |
                                           ( ( sourceVal & ( 31 << 3  ) ) >> 3 ) 
                                         );
   }
}

void CTextureUtil::ConvertR5G6B5ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   BPE_ASSERT( alphaMode != kAlpha_Keep, "no alpha information in source" );

   const uint16* pSource = reinterpret_cast<const uint16*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);

   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;
  
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const int32>( ( ( sourceVal & ( 31 << 11 ) ) << 8 ) |
                                           ( ( sourceVal & ( 63 << 5  ) ) << 5 ) |
                                           ( ( sourceVal & ( 31       ) ) << 3 ) | or
                                         );

   }
}

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertARGB32ToA8L8(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint16* pDest = reinterpret_cast<uint16*>(pDstData);
   const uint16 mask = (alphaMode == kAlpha_Keep) ? 0xffff : 0x00ff;
   const uint16 or = (alphaMode == kAlpha_One) ? 0xff00 : 0x0000;
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const int16>( ( ( ( sourceVal & 0xff000000 ) >> 16 )
                                           + ( (sourceVal & 0x00ff0000) >> 16) * gkRedToGrey 
                                           + ( (sourceVal & 0x0000ff00) >> 8) * gkGreenToGrey 
                                           + ( (sourceVal & 0x000000ff) ) * gkBlueToGrey )
                                           ) & mask | or;

   }
}

void CTextureUtil::ConvertA8L8ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint16* pSource = reinterpret_cast<const uint16*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);
   const uint32 mask = (alphaMode == kAlpha_Keep) ? 0xffffffff : 0x00ffffff;
   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const int32>( ( sourceVal & 0xff00 ) << 16 |
                                           ( sourceVal & 0x00ff ) << 16 |
                                           ( sourceVal & 0x00ff ) << 8  |
                                           ( sourceVal & 0x00ff ) ) & mask | or;
   }
}

//------------------------------------------------------------------------------------------

void CTextureUtil::ConvertARGB32ToA4L4(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint32* pSource = reinterpret_cast<const uint32*>(pSrcData);
   uint8* pDest = reinterpret_cast<uint8*>(pDstData);
   const uint8 mask = (alphaMode == kAlpha_Keep) ? 0xff : 0x0f;
   const uint8 or = (alphaMode == kAlpha_One) ? 0xf0 : 0x00;
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const uint8>( ( ( sourceVal & 0xf0000000 ) >> 24 )
                                        + ( ( sourceVal & 0x00f00000 ) >> 20 ) * gkRedToGrey 
                                        + ( ( sourceVal & 0x0000f000 ) >> 12 ) * gkGreenToGrey 
                                        + ( ( sourceVal & 0x000000f0 ) >> 4 ) * gkBlueToGrey )
                                           & mask | or;
   }
}

void CTextureUtil::ConvertA4L4ToARGB32(const uint8* pSrcData, uint8* pDstData, const int32 numPixels, const EAlpha alphaMode )
{
   const uint8* pSource = reinterpret_cast<const uint8*>(pSrcData);
   uint32* pDest = reinterpret_cast<uint32*>(pDstData);
   const uint32 mask = (alphaMode == kAlpha_Keep) ? 0xffffffff : 0x00ffffff;
   const uint32 or = (alphaMode == kAlpha_One) ? 0xff000000 : 0x00000000;
   
   for( int32 loop = 0; loop < numPixels; ++loop )
   {
      const uint32 sourceVal = *pSource++;
      *pDest++ = static_cast<const int32>( ( sourceVal & 0xf0 ) << 24 |
                                           ( sourceVal & 0x0f ) << 20 |
                                           ( sourceVal & 0x0f ) << 12 |
                                           ( sourceVal & 0x0f ) << 4 ) & mask | or;
   }
}

#endif

