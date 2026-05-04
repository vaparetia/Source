//----------------------------------------------------------------------------
// BP_RenderRadar.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

enum ERadarCommand
{
   kBP_RadarInit,
   kBP_RadarBeginBackPrimitive,
   kBP_RadarBeginSegments,
   kBP_RadarBeginPrimitives,
   kBP_RadarColor,
   kBP_RadarGeomPos,
   kBP_RadarGeomPosCol,
   kBP_RadarEnd
};

enum ERadarPrim
{
   kBP_RadarPrim_LineList,
   kBP_RadarPrim_LineStrip,
   kBP_RadarPrim_TriStrip,
   kBP_RadarPrim_TriFan
};

enum ERadarShader
{
   kBP_RadarShader1,
   kBP_RadarShader2
};

typedef struct _SBP_RadarVertexPos
{
   float x, y, z, h;
} SBP_RadarVertexPos;

typedef struct _SBP_RadarVertexPosCol
{
   float          x, y, z;
   unsigned int   col;
} SBP_RadarVertexPosCol;

typedef struct _SBP_RadarPacket_Init
{
   int      cmd;

   int      viewX, viewY, viewW, viewH;
   FMATRIX  matrix;

} SBP_RadarPacket_Init;

typedef struct _SBP_RadarPacket_End
{
   int   cmd;
} SBP_RadarPacket_End;

typedef struct _SBP_RadarPacket_BeginBackPrimitive
{
   int   cmd;
} SBP_RadarPacket_BeginBackPrimitive;

typedef struct _SBP_RadarPacket_BeginSegments
{
   int   cmd;
} SBP_RadarPacket_BeginSegments;

typedef struct _SBP_RadarPacket_Color
{
   int      cmd;

   FVECTOR  color1;
   FVECTOR  color2;
} SBP_RadarPacket_Color;

typedef struct _SBP_RadarPacket_GeomPos
{
   int                  cmd;
   int                  shader;
   int                  prim;
   int                  vertexCount;
   SBP_RadarVertexPos   verts[0];
} SBP_RadarPacket_GeomPos;

typedef struct _SBP_RadarPacket_GeomPosCol
{
   int                     cmd;
   int                     shader;
   int                     prim;
   int                     vertexCount;
   SBP_RadarVertexPosCol   verts[0];
} SBP_RadarPacket_GeomPosCol;

typedef struct _SBP_RadarPacket_BeginPrimitive
{
   int      cmd;
} SBP_RadarPacket_BeginPrimitive;

//----------------------------------------------------------------------------

void BP_InitRadarShader();

#if __cplusplus
extern "C"
{
#endif
   void BP_RenderRadar(void* pData);
#if __cplusplus
};
#endif

//----------------------------------------------------------------------------
