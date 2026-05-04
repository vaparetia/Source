//----------------------------------------------------------------------------
// BP_RenderBuffer.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

/*

Two ways to use this:

1) If allocation size for data is known ahead of time.

SomeStructType* pData = BP_RB_Alloc(sizeof(SomeStructType));

...fill in pData...

BP_RB_AddCommand(<id>, pData);

2) If allocation size for data is NOT known ahead of time.

// Get current buffer ptr
void* pData = BP_RB_GetCurrentPtr();

..store however much data is needed in pData, advancing the ptr every time data is stored.

BP_RB_SetCurrentPtr(pData); // Restore current ptr to ptr after all the data we just filled in.

BP_RB_AddCommand(<id>, pData); //IMPORTANT: Must come after BP_RB_SetCurrentPtr

NOTE:
- <id> should be some kind of enum type that indicates what type of data is pointed to by "pData".
- Later on when iterating over the commands to execute them, you use this id in the command to determine which action to take for the data.
*/


#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

enum
{
   /* 0  */ kCmd_Label,

   /* 1  */ kCmd_PushProfileMarker,
   /* 2  */ kCmd_PopProfileMarker,

   /* 3  */ kCmd_PushProfileRegionMarker,
   /* 4  */ kCmd_PopProfileRegionMarker,

   /* 5  */ kCmd_FrameFirstPacket,
   /* 6  */ kCmd_FrameNewViewportInfoPacket,
   /* 7  */ kCmd_FrameInitPacket,
   /* 8  */ kCmd_FrameMSAAEndPacket,

   /* 9  */ kCmd_Prim_StartPass,
   /* 10 */ kCmd_Prim_Render_DEPRECATED,
   /* 11 */ kCmd_Prim_InitPacket,
   /* 12 */kCmd_Prim_LocalParam,
   /* 13 */ kCmd_Prim_RenderPacket,
   /* 14 */ kCmd_Prim_BeginOffscreen,
   /* 15 */ kCmd_Prim_EndOffscreen,

   /* 16 */ kCmd_DmaPack_AutoPacket,
   /* 17 */ kCmd_DmaPack_Direct,

   /* 18 */ kCmd_FrameStore_Begin,
   /* 19 */ kCmd_FrameStore_End,

   /* 20 */ kCmd_CopyRenderTargetToQuarterTemp,

   /* 21 */ kCmd_RestoreFogColor,

   /* 22 */ kCmd_SetViewport,

   /* 23 */ kCmd_SetRasterMask,
   
   /* 24 */ kCmd_UpdateDynamicTexture,

   // BP_RenderFX
   /* 25 */ kCmd_PostFx_Mono,
   /* 26 */ kCmd_PostFx_Blinds,
   // Last
   /* 27 */ kCmd_EndDefaultCommands, 
};

//----------------------------------------------------------------------------

typedef struct _SRenderCommand
{
   unsigned int            mId;
   char*                   mData;
   struct _SRenderCommand* mpNext;
} SRenderCommand;

//----------------------------------------------------------------------------

extern int gBP_RB_ReadlockedBuffer;
extern int gBP_RB_WritableBuffer;

void              BP_RB_Init(int bufferSize);
void              BP_RB_SetCurrentWriterThread();
void              BP_RB_UnsetCurrentWriterThread();
int               BP_RB_DebugGetUsedSize();
int               BP_RB_DebugGetTotalSize();
int               BP_RB_DebugGetPeakSize();
char *            BP_RB_DebugGetPeakArea();

void              BP_RB_LockBufferForRead( int bufferIndex );
void              BP_RB_UnlockBufferForRead( int bufferIndex );

void              BP_RB_SetCurrentBufferIndex(int bufferIndex);

// Clears current render buffer
void              BP_RB_Clear();

// Allocates fixed size packet from render buffer
char*             BP_RB_Alloc(int size);

// Gets current render buffer ptr for unpredictable allocation size
char*             BP_RB_GetCurrentPtr();
// Restores render buffer ptr (can be used after a call to GetCurrentPtr to set the end of the allocated range)
void              BP_RB_SetCurrentPtr(char* ptr);

// Adds command to end of command list
void              BP_RB_AddCommand(unsigned int id, void* data);
// Gets first render command
SRenderCommand*   BP_RB_GetFirstCommand(int const bufferIndex);

int               BP_RB_IsCommandValid( int const bufferIndex, SRenderCommand const *cmd );

void BP_RB_DebugBeginLog( char const *name );
void BP_RB_DebugEndLog();

// helper command to alloc DG_TEX_BP and copy data into allocated memory
void BP_RB_CopyTexture(void** ppDG_TEX_BP_, void* pDG_TEX_);

//----------------------------------------------------------------------------

static unsigned int const kProfileColor_CPU_Game                  = 0xFF0000FF;
static unsigned int const kProfileColor_CPU_VSync                 = 0xFFFF00FF;
static unsigned int const kProfileColor_CPU_FixModel_TmpPreshade  = 0xFFFF57FF;

static unsigned int const kProfileColor_FixModel               = 0x71AF57FF;
static unsigned int const kProfileColor_FixModel_Before        = 0x71AF57FF;
static unsigned int const kProfileColor_FixModel_After         = 0x71AF57FF;
static unsigned int const kProfileColor_Model                  = 0xB37097FF;
static unsigned int const kProfileColor_Model_Before           = 0xB37097FF;
static unsigned int const kProfileColor_Model_After            = 0xB37097FF;
static unsigned int const kProfileColor_Semitrans              = 0xA4FC7EFF;
static unsigned int const kProfileColor_Clone                  = 0x7D8BB5FF;
static unsigned int const kProfileColor_Camouflage             = 0xE3BF49FF;
static unsigned int const kProfileColor_Prim                   = 0x00FF00FF;
static unsigned int const kProfileColor_DmaPack                = 0x808080FF;
static unsigned int const kProfileColor_Patch                  = 0x406080FF;
static unsigned int const kProfileColor_ProjectorShadow        = 0x000000FF;
static unsigned int const kProfileColor_ProjectorSpot          = 0x404040FF;
static unsigned int const kProfileColor_ProjectorTexture       = 0x808080FF;
static unsigned int const kProfileColor_Finalize               = 0xFFFFFFFF;

#if BP_ENABLE_PROFILE_MARKERS
typedef struct _SBP_ProfileRegionMarker
{
   unsigned int   color;
   char           string[0];
} SBP_ProfileRegionMarker;

void              BP_RB_PushMarker(char* pString);
void              BP_RB_PopMarker();
void              BP_RB_PushRegionMarker(unsigned int color, char* pString);
void              BP_RB_PopRegionMarker();
#else
#define           BP_RB_PushMarker(pString)
#define           BP_RB_PopMarker()
#define           BP_RB_PushRegionMarker(color, pString)
#define           BP_RB_PopRegionMarker()
#endif

//----------------------------------------------------------------------------

enum
{
   kRL_DmaPack    =  (1 << 0),
   kRL_PostFx     =  (1 << 1)
};

#if BP_ENABLE_RENDER_LABELS
typedef struct _SBP_RenderLabel
{
   unsigned int   flags;
   char           string[0];
} SBP_RenderLabel;
void              BP_RB_AddLabel(char* pString, unsigned int flags);
#else
#define           BP_RB_AddLabel(pString, flags)
#endif

//----------------------------------------------------------------------------

typedef struct _SBP_FrameInitPacket
{
   u_long64       testValue;
   unsigned int   fogColor;
} SBP_FrameInitPacket;

//----------------------------------------------------------------------------

typedef struct _SBP_RenderDmaPack
{
   void*    dmapack;
   void*    data;
} SBP_RenderDmaPack;

//----------------------------------------------------------------------------

typedef struct _SBP_Prim_StartPass
{
   u_long64 testValue;
} SBP_Prim_StartPass;

//----------------------------------------------------------------------------

typedef struct _SBP_Prim_InitPacket
{
   FMATRIX        pers;
   FVECTOR        fogParam;

} SBP_Prim_InitPacket;

typedef struct _SBP_Prim_LocalParam
{
   FMATRIX        screen;
   int            clone;
#if BP_PRIM_DEBUGGING
   void           *prim;
#endif
} SBP_Prim_LocalParam;

typedef struct _SBP_PrimPacket_Render
{
   unsigned long long tex_trans_alpha_data;
   unsigned long long tex_trans_clamp_data;

   int flag;
   char* fname;
   void* BP_vertexBuffer;
   int BP_indexBuffer[3];
   int BP_vertexCount;
   int BP_startIndex;
   int BP_indexCount;
   int BP_forceDisableAlphaTest;
   int BP_forceZWrite;
   int BP_tex; // DG_TEX_BP * - this DG_TEX_BP is allocated directly into the command buffer
   int BP_instanced;
   int clone;
#if MGS_VERSION == 3
   int BP_fogBuffer;

   int BP_optflag;
   int BP_masktex;
   unsigned long long mask_trans_clamp_data;
   unsigned long long mask_trans_alpha_data;
   int BP_optimizedPrimShapeIndex;
   int BP_indexCountPerInstance;
#endif

   short type;
} SBP_PrimPacket_Render;

//----------------------------------------------------------------------------

typedef struct _SBP_FrameStorePacket
{
   unsigned int   bpTexture;
   int            x0, y0, x1, y1;
   int            clearflags;
} SBP_FrameStorePacket;

//----------------------------------------------------------------------------

typedef struct _SBP_SetViewport
{
   int x, y, width, height;
} SBP_SetViewport;

//----------------------------------------------------------------------------

typedef struct _SBP_UpdateDynamicTexture
{
   void*          vram;
   void*          clut;
   int            bitsPerPixel;
   unsigned int   bp_tex;
} SBP_UpdateDynamicTexture;

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
