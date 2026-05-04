//----------------------------------------------------------------------------
// BP_RenderObj.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#define TRACK_RENDEROBJ_STATS 0

//----------------------------------------------------------------------------

void BP_InitObjShader();
void BP_InitObjCharShader();
void BP_Obj_FrameBegin();
void BP_Obj_FrameEnd();
void BP_Obj_InitPacket(char* pData);
void BP_Obj_SortChainInitPacket();
void BP_Obj_SortChainEndPacket();
void BP_Obj_LocalParam(char* pData);
void BP_Obj_PreRender( char *pData );
void BP_Obj_Render(char* pData);
void BP_Obj_RenderMulti(char* pData);
void BP_Obj_InitDebugMenu();

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif
   void BP_AllocPreshadeBuffer(void* pDGObjs);
   void BP_FreePreshadeBuffer(void* pDGObjs);
   void BP_UpdatePreshadeBuffer(void* pDGObjs, u_int const * pMGSVertexColors);

   void BP_Obj_Render_ComputeAnimatedBuffers(char* pDGObjs, char* pMesh);

#if BP_VITA
   struct _SBP_OBJ_PreRender_Vita;
   void BP_Obj_PreRenderVita( char *pData );
   void BP_Obj_PreRender_MakeVitaUniforms( struct _SBP_OBJ_PreRender_Vita *pOut );
#endif

   int BP_Obj_CheckForceNoMSAAForTRIFile( int tri_file );
   int BP_Obj_CheckInsideAreaWhereWindowRainNoMSAA();

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
