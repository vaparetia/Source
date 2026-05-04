//----------------------------------------------------------------------------
// BP_RenderBufferTypes.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "BP_RenderBuffer.h"

//----------------------------------------------------------------------------

enum ERenderCommandType
{
   // To make sure this enum continues where default commands end.
   /* 28 */ kCmd_Dummy = kCmd_EndDefaultCommands + 1,

   /* 29 */ kCmd_ClearViewport,

   /* 30 */ kCmd_Obj_InitPacket,
   /* 31 */ kCmd_Obj_SortChainInitPacket,
   /* 32 */ kCmd_Obj_SortChainEndPacket,
   /* 33 */ kCmd_Obj_LocalParam,
   /* 34 */ kCmd_Obj_PreRender,
            kCmd_Obj_PreRenderVita,
   /* 35 */ kCmd_Obj_Render,
   /* 36 */ kCmd_Obj_RenderMulti,

   /* 37 */ kCmd_Obj_OptCmf_InitPacket,
   /* 38 */ kCmd_Obj_OptCmf_PassParam,
   /* 39 */ kCmd_Obj_OptCmf_LocalParam,
   /* 40 */ kCmd_Obj_OptCmf_Render,

   /* 41 */ kCmd_Obj_Projector_LocalParam,
   /* 42 */ kCmd_Obj_Projector_Render,

   /* 43 */ kCmd_Projector_ShadowStage_StartPacket,
   /* 44 */ kCmd_Projector_ShadowStage_InitPacket,
   /* 45 */ kCmd_Projector_ShadowStage_PostZScreenPacket,
   /* 46 */ kCmd_Projector_ShadowStage_ShadowProjStart,
   /* 47 */ kCmd_Projector_ShadowStage_EndPacket,

   /* 48 */ kCmd_Projector_SpotStage_StartPacket,
   /* 49 */ kCmd_Projector_SpotStage_InitPacket,
   /* 50 */ kCmd_Projector_SpotStage_PostZScreenPacket,
   /* 51 */ kCmd_Projector_SpotStage_ShadowProjStart,
   /* 52 */ kCmd_Projector_SpotStage_EndPacket,

   /* 53 */ kCmd_Evm_InitPacket,
   /* 54 */ kCmd_Evm_LocalParam,
   /* 55 */ kCmd_Evm_Render,
   /* 56 */ kCmd_Evm_RenderMulti,

   /* 57 */ kCmd_RenderRadar,

   /* 58 */ kCmd_Patch_InitFrame,
   /* 59 */ kCmd_Patch_Init,
   /* 60 */ kCmd_Patch_InitRender,
   /* 61 */ kCmd_Patch_Render,
   /* 62 */ kCmd_Patch_FinalizeFrame,

   /* 63 */ kCmd_Comdl_InitPacket,
   /* 64 */ kCmd_Comdl_Render,

   /* 65 */ kCmd_PostFx_FarFocus,
   /* 66 */ kCmd_PostFX_VRClear,
   /* 67 */ kCmd_PostFX_CodexInOut,

   /* 68 */ kCmd_FrameBuffer,
   /* 69 */ kCmd_MSAA_Particles,
   
   /* 70 */ kCmd_PostFx_Offscreen,

   /* 71 */ kCmd_ProjectorNew_StartPacket,
   /* 72 */ kCmd_ProjectorNew_InitPacket,
   /* 73 */ kCmd_ProjectorNew_ShadowProjStart,
   /* 74 */ kCmd_ProjectorNew_SpotProjStart,
   /* 75 */ kCmd_ProjectorNew_EndPacket,
   /* 76 */ kCmd_ProjectorNew_Dummy,

   kCmd_PostFx_ReduceScreen
};

typedef struct _SBP_ClearViewport
{
   unsigned int clearColor;
   int          clearFlags;

} SBP_ClearViewport;

//----------------------------------------------------------------------------
// KMS (DG_OBJ) model rendering
typedef struct _SBP_OBJ_InitPacket
{
   FVECTOR		clamp_min;
   FVECTOR		clamp_max;
   float		   fog_param1, fog_param2, fog_param3, fog_param4;
   float	      color_clip, poly_alpha, specular_mul, specular_clip;
   FVECTOR		scale;
   FVECTOR		offset;
   float	      param1, param2, param3, param4 ;
   FVECTOR		aspect;
   FMATRIX		pers;
} SBP_OBJ_InitPacket;

typedef struct _SBP_OBJ_LocalParam
{
   //unsigned int   model;
   unsigned int   preshadeBuffer;
   unsigned int   dgObjs;
   FVECTOR        fogParam;
} SBP_OBJ_LocalParam;

enum OBJ_PreRenderFlags
{
   kPRF_MultiPass    = (1<<0),
   kPRF_Light        = (1<<1),
};

typedef struct _SBP_OBJ_PreRender
{
   FMATRIX        eye_pers;
   FMATRIX        eye_inv;
   FMATRIX        correction;
   FMATRIX        lightDir;
   FMATRIX        lightCol;

//   unsigned char BP_Flags;
} SBP_OBJ_PreRender;

#if BP_VITA
struct _SBP_OBJ_PreRender_Vita;

typedef struct _SBP_OBJ_PreRender_Vita
{
   void *         mpUniformBufferPtr;
   DG_OBJ *       mpOrigObj;
   struct _SBP_OBJ_PreRender_Vita *mpUltChain;
} SBP_OBJ_PreRender_Vita;

struct _SBP_OBJ_PreComputedInfo;
struct _SBP_OBJ_Render;

typedef SBP_OBJ_PreRender_Vita *TBP_OBJ_PreRender_Vita_Ptr;

typedef struct _SBP_OBJ_PreComputedInfo
{
   struct _SBP_OBJ_Render *mpPrevStart;
   struct _SBP_OBJ_Render *mpNextStart;
   struct _SBP_OBJ_Render *mpNextInterior;
   TBP_OBJ_PreRender_Vita_Ptr mpUniformBuffers[2];
   unsigned mLastFrameTouched[2];
   unsigned char mIsStart;
   unsigned char pad[3];
} SBP_OBJ_PreComputedInfo;

KP_CTASSERT( sizeof( SBP_OBJ_PreComputedInfo ) == 4 * 3 + 4 * 2 + 4*2 + 4 );

#endif

typedef struct _SBP_OBJ_Render
{
   unsigned int   allocSize;
   unsigned int   obj_flag;
   unsigned int   flag;
   unsigned int   model;
   unsigned int   unit;

   DG_TEX_BP*     tex[3];

   unsigned int   startPacket;
   unsigned int   packetCount;
   
   unsigned char  builtForIR;
   unsigned char  isIRMode;

   void*          BP_animatedVertexBuffer;
#if BP_VITA
   SBP_OBJ_PreComputedInfo*
                  BP_precomp;
#endif

#if 0
   unsigned char count;
   unsigned char texDiffCount;
   unsigned char flagDiffCount;
#endif

} SBP_OBJ_Render;

//----------------------------------------------------------------------------
// KMS (DG_OBJ) model rendering with optical camouflage (OptCmf)
typedef struct _SBP_OBJ_OptCmf_InitPacket
{
   FMATRIX eye_pers;
   FMATRIX eyeInv;
} SBP_OBJ_OptCmf_InitPacket;

typedef struct _SBP_OBJ_OptCmf_PassParam
{
   u_long64 alphaData;
} SBP_OBJ_OptCmf_PassParam;

typedef struct _SBP_OBJ_OptCmf_LocalParam
{
   //unsigned int model;
   unsigned int preshadeBuffer;
   FVECTOR fogParam;
} SBP_OBJ_OptCmf_LocalParam;

typedef struct _SBP_OBJ_OptCmf_Render
{
   //FMATRIX eye_pers;
   FMATRIX world;
   FMATRIX correction;
   FMATRIX lightDir;
   FMATRIX lightCol;

   unsigned int obj_flag;
   unsigned int flag;
   unsigned int passFlag;
   unsigned int model;
   unsigned int unit;

   unsigned int startPacket;
   unsigned int packetCount;
} SBP_OBJ_OptCmf_Render;

//----------------------------------------------------------------------------
// KMS (DG_OBJ) shadow projector rendering (shdwchin.c)
//Shadow Stage
typedef struct _SBP_Projector_ShadowStage_InitPacket
{
   FMATRIX eye_pers;
} SBP_Projector_ShadowStage_InitPacket; // kCmd_Projector_ShadowStage_InitPacket

typedef struct _SBP_Projector_ShadowStage_ShadowProjStart
{
   FMATRIX eye_pers;
} SBP_Projector_ShadowStage_ShadowProjStart; // kCmd_Projector_ShadowStage_ShadowProjStart

//Spot stage
typedef struct _SBP_Projector_SpotStage_InitPacket
{
   FMATRIX eye_pers;
} SBP_Projector_SpotStage_InitPacket; // kCmd_Projector_SpotStage_InitPacket

typedef struct _SBP_Projector_SpotStage_PostZScreenPacket
{
   DG_TEX_BP* tex;
   unsigned int mgsColor;
} SBP_Projector_SpotStage_PostZScreenPacket; // kCmd_Projector_SpotStage_PostZScreenPacket

typedef struct _SBP_Projector_SpotStage_ShadowProjStart
{
   FMATRIX eye_pers;
} SBP_Projector_SpotStage_ShadowProjStart; // kCmd_Projector_SpotStage_ShadowProjStart

typedef struct _SBP_Projector_InitPacket
{
   FMATRIX eye_pers;
} SBP_Projector_InitPacket; // kCmd_Obj_Projector_InitPacket

typedef struct _SBP_ProjectorNew_StartPacket
{
   SRenderCommand *me;
   SRenderCommand *head;
   SRenderCommand *tail;
   int nspots;
} SBP_ProjectorNew_StartPacket;

typedef struct _SBP_ProjectorNew_InitPacket
{
   FMATRIX eye_pers;
   int shadow;
} SBP_ProjectorNew_InitPacket; // kCmd_ProjectorNew_InitPacket

typedef struct _SBP_ProjectorNew_ShadowProjStart
{
   FMATRIX eye_pers;
   int shadow;
} SBP_ProjectorNew_ShadowProjStart; // kCmd_ProjectorNew_ShadowProjStart

typedef struct _SBP_OBJ_Projector_LocalParam
{
   //unsigned int model;
   unsigned int preshadeBuffer;
   FVECTOR fogParam;
   unsigned int shaderType;
} SBP_OBJ_Projector_LocalParam; // kCmd_Obj_Projector_LocalParam

typedef struct _SBP_OBJ_Projector_Render
{
   //FMATRIX eye_pers;
   FMATRIX world;
   FMATRIX correction;
   FMATRIX lightDir;
   FMATRIX lightCol;

   unsigned int obj_flag;
   unsigned int flag;
   unsigned int passFlag;
   unsigned int model;
   unsigned int unit;

   DG_TEX_BP* tex;

   unsigned int startPacket;
   unsigned int packetCount;
} SBP_OBJ_Projector_Render; // kCmd_Obj_Projector_Render

//----------------------------------------------------------------------------
// EVM (DG_EVMOBJ) rendering
typedef struct _SBP_EVM_InitPacket
{
   FMATRIX      eye_pers;
   FMATRIX      eye_inv;
   FVECTOR      fogParam;
} SBP_EVM_InitPacket;

typedef struct _SBP_EVM_LocalParam
{
   unsigned int model;
   FMATRIX*     matrix;
   FMATRIX      lightDir;
   FMATRIX      lightCol;
   unsigned int isInIRMode;
   unsigned int specialModel;
   float        specialModelValue;
} SBP_EVM_LocalParam;

typedef struct _SBP_EVM_Render
{
   unsigned int flag;
   unsigned int model;

   DG_TEX_BP* tex[3];

   unsigned int startPacket;
   unsigned int packetCount;
} SBP_EVM_Render;

//----------------------------------------------------------------------------
// Patch rendering
#define PATCH_MODE_NORMAL 0
#define PATCH_MODE_REFLECTPLANE 1
typedef struct _SBP_Patch_InitFrame
{
   FMATRIX        pers;
   FMATRIX        view;
} SBP_Patch_InitFrame;

typedef struct _SBP_Patch_Init
{
   // all related rendering states.
   FMATRIX        world;
   FMATRIX        lightVec;
   FMATRIX        lightCol;
   FMATRIX        eyeInv;

   FVECTOR        fogParam;
   FVECTOR        tex0Scale;
   FVECTOR        tex0Offset;
   
   u_long64       alphaMode;

   DG_TEX_BP*     tex0;
   DG_TEX_BP*     tex1;
   
   unsigned int   patchFlags;
   unsigned int   patchMode;
} SBP_Patch_Init;

typedef struct _SBP_Patch_InitRender
{
   void * vertexBuffer_UT;
   unsigned int n_vertices;
} SBP_Patch_InitRender;

typedef struct _SBP_Patch_Render
{
   int indexBuffer_UT[3];
   unsigned int n_indices;
} SBP_Patch_Render;

//----------------------------------------------------------------------------
// Comdl rendering (Clone rendering in MGS3)
typedef struct _SBP_Comdl_InitPacket
{
   FMATRIX pers;
   FVector fogParam;
} SBP_Comdl_InitPacket;

typedef struct _SBP_Comdl_EndPacket
{
   int dummy;
} SBP_Comdl_EndPacket;

typedef struct _SBP_Comdl_Render
{
   unsigned int   flag;

   DG_TEX_BP*     tex;

   unsigned int   n_objs;
   void*          pos;//DG_COMDL_POS

   unsigned int   n_verts;
   short*         verts;
   short*         norms;
   short*         uvs;
} SBP_Comdl_Render;

//----------------------------------------------------------------------------

typedef struct _SBP_PFX_FarFocus
{
   unsigned int      max_plane;
   float             focus_near;
   float             focus_far;
} SBP_PFX_FarFocus;

//----------------------------------------------------------------------------

typedef struct _SBP_PFX_VRClear
{
   float percentage;
} SBP_PFX_VRClear;

//----------------------------------------------------------------------------

typedef struct _SBP_PFX_CodexInOut
{
   FVector colorWeight;
   FVector uMinMax_vMinMax;
   int needPreviousFrameCopy;
   int needFaceBoxes;
   float faceBoxes[2][4];
} SBP_PFX_CodexInOut;

//----------------------------------------------------------------------------
// Misc helper struct

// This struct matches the data passed in DG_OBJ->matrix_addr.
typedef struct _KP_MatrixData 
{
//   FMATRIX		screen; /* is obj->screen */
   FMATRIX		local_light;
   FMATRIX		local_color;
//   FMATRIX		connection; /* is obj->inv_mat */
//   FMATRIX		option; /* is cp->eye_inv * obj->world */
//   int			n_verts;
//   int			flag;
//   int			fog;
//   int			bound;
} KP_MatrixData;

// Magic FrameBuffer command
// Picks between the fullres 960x544 render and the 736x416 render
typedef struct _SBP_FrameBuffer
{
   int channel;
} SBP_FrameBuffer;

typedef struct _SBP_ReduceScreen
{
   float x1, y1, x2, y2;
   unsigned int color;
} SBP_ReduceScreen;
