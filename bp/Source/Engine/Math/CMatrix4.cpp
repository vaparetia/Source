//----------------------------------------------------------------------------
// CMatrix4.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CMatrix4.h"

//----------------------------------------------------------------------------

CMatrix4 const CMatrix4::Perspective(CAngle const & fov, real32 const aspect, real32 const minClip, real32 const maxClip)
{
   real32 yScale = 1.0f / tanf(0.5f * fov.AsRadians());
   real32 xScale = yScale / aspect;

   real32 zRange = maxClip - minClip;

   return CMatrix4(xScale, 0,       0,                         0,
                   0,      yScale,  0,                         0,
                   0,      0,       maxClip / zRange,          1,
                   0,      0,       -minClip*maxClip / zRange, 0);
}

//----------------------------------------------------------------------------

CMatrix4 const CMatrix4::PerspectiveNegZ(CAngle const & fov, real32 const aspect, real32 const minClip, real32 const maxClip)
{
   real32 yScale = 1.0f / tanf(0.5f * fov.AsRadians());
   real32 xScale = yScale / aspect;

   real32 zRange = maxClip - minClip;

   return CMatrix4(
      xScale, 0,       0,                         0,
      0,      yScale,  0,                         0,
      0,      0,       -minClip / zRange,         -1,
      0,      0,       -minClip*maxClip / zRange, 0);
}

//----------------------------------------------------------------------------

CMatrix4 const CMatrix4::Orthographic(real32 const width, real32 const height, real32 const minClip, real32 const maxClip)
{
   return CMatrix4(2.0f / width, 0,             0,                               0,
                   0,            2.0f / height, 0,                               0,
                   0,            0,             1.0f / (maxClip - minClip),      0,
                   0,            0,             minClip / (minClip - maxClip),   1);
}

//----------------------------------------------------------------------------

CMatrix4 const CMatrix4::OrthographicNegZ(real32 const width, real32 const height, real32 const minClip, real32 const maxClip)
{
   return CMatrix4(2.0f / width, 0,             0,                               0,
      0,            2.0f / height, 0,                               0,
      0,            0,             -1.0f / (maxClip - minClip),      0,
      0,            0,             -maxClip / (maxClip - minClip),   1);
}

//----------------------------------------------------------------------------

CMatrix4 const CMatrix4::OrthographicOffset(real32 const width, real32 const height, real32 const offsetX, real32 const offsetY, real32 const minClip, real32 const maxClip)
{
   return CMatrix4(2.0f / width,             0,                         0,                               0,
                   0,                        2.0f / height,             0,                               0,
                   0,                        0,                         1.0f / (maxClip - minClip),      0,
                   offsetX,                  offsetY,                   minClip / (minClip - maxClip),   1);
}

//----------------------------------------------------------------------------

CMatrix4 const CMatrix4::OrthographicOffsetNegZ(real32 const width, real32 const height, real32 const offsetX, real32 const offsetY, real32 const minClip, real32 const maxClip)
{
   return CMatrix4(2.0f / width,             0,                         0,                               0,
      0,                        2.0f / height,             0,                               0,
      0,                        0,                         -1.0f / (maxClip - minClip),      0,
      offsetX,                  offsetY,                   -maxClip / (maxClip - minClip),   1);
}

//------------------------------------------------------------------------------------------

bool const CMatrix4::operator == ( CMatrix4 const & rhs ) const
{
   return (memcmp( this, &rhs, sizeof( CMatrix4 ) ) == 0);
}

//----------------------------------------------------------------------------
