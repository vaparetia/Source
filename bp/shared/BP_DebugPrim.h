//----------------------------------------------------------------------------
// BP_DebugPrim.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

void BP_InitDebugShader();
void BP_HandleCommandLineArgs();

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

#define BP_ColorRGBA(_r,_g,_b,_a)  (((_r)<<0) | ((_g)<<8) | ((_b)<<16) | ((_a)<<24))

#if BP_ENABLE_DEBUG_PRIM

void BP_DebugPrim_SetMatrices(FMATRIX const * worldToScreenMatrix, FMATRIX const * worldToViewMatrix);

void BP_DebugPrim_BeginLines(int depthTest);
void BP_DebugPrim_AddVert(FVECTOR const * pos, unsigned int color);

EXTERN_INLINE void BP_DebugPrim_AddVert2Col1(FVECTOR const * pos0, FVECTOR const * pos1, unsigned int color)
{
   BP_DebugPrim_AddVert(pos0, color);
   BP_DebugPrim_AddVert(pos1, color);
}

EXTERN_INLINE void BP_DebugPrim_AddVert2Col2(FVECTOR const * pos0, unsigned int color0, FVECTOR const * pos1, unsigned int color1)
{
   BP_DebugPrim_AddVert(pos0, color0);
   BP_DebugPrim_AddVert(pos1, color1);
}

void BP_DebugPrim_DrawSphere(int depthTest, FVECTOR const * pos, float radius, unsigned int color );

EXTERN_INLINE void BP_DebugPrim_DrawLine(int depthTest, FVECTOR const * pos0, FVECTOR const * pos1, unsigned int color)
{
   BP_DebugPrim_BeginLines(depthTest);
   BP_DebugPrim_AddVert2Col1(pos0, pos1, color);
}

EXTERN_INLINE void BP_DebugPrim_DrawQuad(int depthTest, FVECTOR const * verts, unsigned int color)
{
   BP_DebugPrim_BeginLines(depthTest);
   BP_DebugPrim_AddVert2Col1(&verts[0], &verts[1], color);
   BP_DebugPrim_AddVert2Col1(&verts[1], &verts[2], color);
   BP_DebugPrim_AddVert2Col1(&verts[2], &verts[3], color);
   BP_DebugPrim_AddVert2Col1(&verts[3], &verts[0], color);
}

EXTERN_INLINE void BP_DebugPrim_DrawBox(int depthTest, FVECTOR const * min, FVECTOR const * max, unsigned int color)
{
   FVECTOR vert[8];
   
   vert[0].vx = min->vx ;
   vert[0].vy = max->vy ;
   vert[0].vz = min->vz ;

   vert[1].vx = max->vx ;
   vert[1].vy = max->vy ;
   vert[1].vz = min->vz ;

   vert[2].vx = max->vx ;
   vert[2].vy = max->vy ;
   vert[2].vz = max->vz ;

   vert[3].vx = min->vx ;
   vert[3].vy = max->vy ;
   vert[3].vz = max->vz ;

   vert[4].vx = min->vx ;
   vert[4].vy = min->vy ;
   vert[4].vz = min->vz ;

   vert[5].vx = max->vx ;
   vert[5].vy = min->vy ;
   vert[5].vz = min->vz ;

   vert[6].vx = max->vx ;
   vert[6].vy = min->vy ;
   vert[6].vz = max->vz ;

   vert[7].vx = min->vx ;
   vert[7].vy = min->vy ;
   vert[7].vz = max->vz ;

   BP_DebugPrim_BeginLines(depthTest);
   
   BP_DebugPrim_AddVert2Col1(&vert[0], &vert[1], color);
   BP_DebugPrim_AddVert2Col1(&vert[1], &vert[2], color);
   BP_DebugPrim_AddVert2Col1(&vert[2], &vert[3], color);
   BP_DebugPrim_AddVert2Col1(&vert[3], &vert[0], color);

   BP_DebugPrim_AddVert2Col1(&vert[4], &vert[5], color);
   BP_DebugPrim_AddVert2Col1(&vert[5], &vert[6], color);
   BP_DebugPrim_AddVert2Col1(&vert[6], &vert[7], color);
   BP_DebugPrim_AddVert2Col1(&vert[7], &vert[4], color);

   BP_DebugPrim_AddVert2Col1(&vert[0], &vert[4], color);
   BP_DebugPrim_AddVert2Col1(&vert[1], &vert[5], color);
   BP_DebugPrim_AddVert2Col1(&vert[2], &vert[6], color);
   BP_DebugPrim_AddVert2Col1(&vert[3], &vert[7], color);
}

void BP_DebugPrim_DrawTransformedBox(FMATRIX *world, int depthTest, FVECTOR const * min, FVECTOR const * max, unsigned int color);

void BP_DebugPrim_Render();
void BP_DebugPrim_Clear();

#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------
