//----------------------------------------------------------------------------
// BP_RenderPrim.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"
#include <libdg.h>

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#ifdef __cplusplus

#include "Renderer\Base\Primitive\CIndexBuffer.h"

struct SPrimRenderState
{
   void *mpData;
   void *mpVertexBuffer;
   CDynamicIndexBufferPool_UT::TChunk mpIndexBuffer;
   int mInstanced;
};

#endif

#ifdef __cplusplus
extern "C"
{
#endif

#if MGS_VERSION==2
#  define DG_PRIM_PACKET_GAME DG_PRIM2_PACKET
#elif MGS_VERSION==3
#  define DG_PRIM_PACKET_GAME DG_PRIM_PACKET
#else
#  error Unknown game version?!
#endif
//----------------------------------------------------------------------------

struct SPrimRenderState;
// The mpIndexBuffer member of SPrimRenderState is larger on Vita
#if BP_VITA
#  define C_PRIMRENDERSTATE_SIZE ( 3 * 4 + 3 * 4 )
#else
#  define C_PRIMRENDERSTATE_SIZE ( 4 * 4 )
#endif

//----------------------------------------------------------------------------

void BP_InitPrimShader();
void BP_Prim_FrameBegin();
void BP_Prim_FrameEnd();
void BP_Prim_StartPass(char* pData);
int  BP_Prim_Render_MakeState( struct SPrimRenderState *pState, void *pData );
void BP_Prim_Render_Execute( struct SPrimRenderState const *pState );
void BP_Prim_InitPacket(char* pData);
void BP_Prim_LocalParam(char* pData);
void BP_Prim_RenderPacket(char* pData);
void BP_Prim_BeginOffscreen();
void BP_Prim_EndOffscreen();
void BP_Prim_BeginViewport();

//void BP_Prim_Update_DrawViewportsBegin();
void BP_Prim_Update_FixupPrimRenderPackets();
void BP_Prim_Update_AddRenderWrapCommand( DG_PRIM_PACKET_GAME *packet, int clone );

void BP_Prim_InitDebugMenu();

void* BP_Prim_LookupPrimShape(DG_PRIM *pPrim);

void BP_Prim_Analyze( DG_PRIM *prim );

enum
{
   kPOpt_Nop        = (1 << 0),
   kPOpt_Mask       = (1 << 1),
   kPOpt_Dup        = (1 << 2)
};

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
