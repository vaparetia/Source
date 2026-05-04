//------------------------------------------------------------------------------------------
// CColor.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CColor.h"

//------------------------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

CColor::CColor(CInputStream& inputStream)
:  mARGB(inputStream.ReadUint32())
{
}

//------------------------------------------------------------------------------------------

void CColor::PutTo(COutputStream& outputStream) const
{
   outputStream.WriteUint32(mARGB);
}

//------------------------------------------------------------------------------------------

CColor const CColor::FromRGB565( uint16 const rgb565 )
{
   uint8 r = (uint8) ((rgb565 & (0x1F << 11)) >> 8);
   r |= (r >> 5) & 7;
   uint8 g = (uint8) ((rgb565 & (0x3F << 5)) >> 3);
   g |= (g >> 6) & 3;
   uint8 b = (uint8) ((rgb565 & (0x1F << 0)) << 3);
   b |= (b >> 5) & 7;

   return CColor( r, g, b, 255);
}

//------------------------------------------------------------------------------------------

uint32 const CColor::GetUint32_PlatformSpecific() const
{
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   return mARGB;
#elif BPE_TARGET == BPE_TARGET_VITA
   // ABGR
   return ((mARGB & 0xff000000)) | ((mARGB & 0x00ff0000) >> 16) | ((mARGB & 0x0000ff00)) | ((mARGB & 0x000000ff) << 16);
#else
   return GetRGBA();
#endif
}

//------------------------------------------------------------------------------------------

uint32 CColor::GetRGBA() const
{
   uint32 const rgba = ((mARGB & 0xFF000000) >> 24) | ((mARGB & 0x00FFFFFF) << 8);
   return rgba;
}

