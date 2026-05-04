//----------------------------------------------------------------------------
// BP_RenderPrim.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderPrim.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "BP_Renderer.h"
#include "BP_RenderGS.h"
#include "BP_RenderFX.h"
#include "BP_RendererDebug.h"
#include "BP_Debug.h"
#include "BP_Simd.h"
//----------------------------------------------------------------------------

#include "MGS_Common.h"
#include "bp_matrix.h"
#include "BP_EndianSupport.h"

//----------------------------------------------------------------------------

#include "BP_RenderBufferTypes.h"
#include "BP_RenderShared.h"

#define BP_PRIM_OFFSCREEN_ENABLE 0

//----------------------------------------------------------------------------

// Since we are stuffing a prim render states into ULT params,
// we need to make sure it's big enough
KP_CTASSERT( sizeof( SULTParam ) >= sizeof( struct SPrimRenderState ) );
KP_CTASSERT( sizeof( SPrimRenderState ) == C_PRIMRENDERSTATE_SIZE );
//----------------------------------------------------------------------------

#if MGS_VERSION == 2

#define DG_PRIM               DG_PRIM2
#define DG_PRIM_PACKET        DG_PRIM2_PACKET
#define DG_PRIM_UVRGB         DG_PRIM2_UVRGB
#define DG_PRIM_UVRGBWH       DG_PRIM2_UVRGBWH

#define DG_PRIM_SHADE         DG_PRIM2_SHADE
#define DG_PRIM_TEX           DG_PRIM2_TEX
#define DG_PRIM_FOG           DG_PRIM2_FOG
#define DG_PRIM_ALPHA         DG_PRIM2_ALPHA
#define DG_PRIM_FBTEX         DG_PRIM2_FRAMETEX
#define DG_PRIM_ANTIALIASING  DG_PRIM2_ANTIALIASING

#define DG_PRIM_LINE          DG_PRIM2_LINE
#define DG_PRIM_POLY          DG_PRIM2_POLY
#define DG_PRIM_CULLPOLY      DG_PRIM2_CULLPOLY
#define DG_PRIM_SPRT          DG_PRIM2_SPRT
#define DG_PRIM_RSPRT         DG_PRIM2_RSPRT

enum 
{
   DG_VERTFLAG_KICK     = 0x0000,
   DG_VERTFLAG_NOKICK   = 0x8000,
   DG_VERTFLAG_2SIDE    = 0x0fff,
   DG_VERTFLAG_CW       = 0x0020,
   DG_VERTFLAG_CCW      = 0x0000
};

#endif

//----------------------------------------------------------------------------

namespace
{
   int gIsFirstPrimForViewport;
   int gNumPrims;
   int gNumPrimPackets;
   int gNumPrimInputVerts;
   int gNumPrimOutputVerts;
   int gParticleParameterBufferSize = 0;

   int gPrimClone = 0;

#if BP_VITA
   float* gpBlackUniformBuffer = NULL;

   CBaseTexture const * gpLastTex;
   unsigned long long gLastTransClamp;
   CBaseTexture const * gpLastMaskTex;
   unsigned long long gLastMaskTransClamp;
#else
   uint8* gpBlackParameterBuffer = NULL;
   uint8* gpFogColorParameterBuffer = NULL;
#endif

   // vita leaves this NULL, so we don't duplicate all of the setup code
   uint8* gpCurrentFogParameterBuffer = NULL;
}

int gPrim_EnableLine = 1;
int gPrim_EnablePoly = 1;
int gPrim_EnableCullPoly = 1;
int gPrim_EnableSprt= 1;
int gPrim_EnableRSprt = 1;
int gPrim_ShowStats = 0;
int gPrim_ShowVertexCountIssues = 0;

int gPrim_PS2ColorClamp = 1;
int gPrim_OptimizeParticleShape = 1;
int gPrim_CullAlpha = 1;
int gPrim_EnableInstancing = 1;
extern "C" int gPrim_EnableMerging = 1;

struct BP_Prim_Vertex
{
   float x, y, z;
   int16 u, v, q, pad0;
   uint32 rgba;
};

struct BP_Prim_Vertex_Mask
{
   float x, y, z;
   int16 u, v, q, pad0;
   uint32 rgba;
   int16 u1, v1, q1, pad1;
};

struct BP_Prim_SpriteVertex
{
   float x, y, z;
   int16 u, v, dx, dy;
   uint32 rgba;
};

struct BP_Prim_SpriteVertexInstanced
{
   float x, y, z;
   int16 u0, v0, u1, v1, dx, dy;
   uint32 rgba;
};

struct BP_Prim_SpriteVertex_Mask
{
   float x, y, z;
   int16 u, v, dx, dy;
   uint32 rgba;
   int16 mu, mv;
};

struct BP_Prim_SpriteVertexInstanced_Mask
{
   float x, y, z;
   int16 u0, v0, u1, v1, dx, dy;
   uint32 rgba;
   int16 mu0, mv0, mu1, mv1;
};

//----------------------------------------------------------------------------

struct SParticleVert
{
   SParticleVert(real32 const inputU, real32 inputV)
   {
      // Flip V coordinate upside down.
      inputV = -inputV;

      originalU = inputU;
      originalV = inputV;

      u = (0.5f + 0.5f * inputU) * 0x10000;
      v = (0.5f + 0.5f * inputV) * 0x10000;

      // To compensate for fact that width (cos)/height (sin) are actual diagonals of the particle quad.
      real32 const scale = 0.70710678118654752440084436210485f;

      sprtX = inputU * 0x10000;
      sprtY = inputV * 0x10000;

      // Rotate by -45 degrees
      real32 const rotSin = -0.70710678118654752440084436210485f;
      real32 const rotCos = 0.70710678118654752440084436210485f;

      rsprtX = ((inputU * rotCos - inputV * rotSin) * scale) * 0x10000;
      rsprtY = ((inputU * rotSin + inputV * rotCos) * scale) * 0x10000;
   }

   float originalU, originalV;
   int   u, v;
   int   sprtX, sprtY;
   int   rsprtX, rsprtY;
};

struct SPrimShape
{
   SPrimShape(int shapeIndex, unsigned int texId, int const numVerts, SParticleVert const * pVerts)
   {
      mShapeIndex = shapeIndex;
      mTexId = texId;
      mNumVerts = numVerts;
      mpVerts = pVerts;
   }

   int                     mShapeIndex;
   unsigned int            mTexId;
   int                     mNumVerts;
   SParticleVert const *   mpVerts;
};

// bombgas1_alp.tga
SParticleVert const gBombGas1_Alp_Verts[] =
{
   // Area reduced to 71.42% (optimal convex area is 70.40%)
   SParticleVert(  0.863641f,  0.576380f ),
   SParticleVert(  0.982591f, -0.387109f ),
   SParticleVert(  0.548983f, -1.008883f ),
   SParticleVert( -0.808572f, -0.700509f ),
   SParticleVert( -0.917431f, -0.008932f ),
   SParticleVert( -0.752024f,  0.671177f ),
   SParticleVert(  0.211245f,  0.962870f ),
};

// bombgas3_alp.tga
SParticleVert const gBombGas3_Alp_Verts[] =
{
#if 0
   // Area reduced to 58.59% (optimal convex area is 54.65%)
   SParticleVert(  0.582411f,  0.755100f ),
   SParticleVert(  0.893663f,  0.132595f ),
   SParticleVert(  0.775566f, -0.694084f ),
   SParticleVert( -0.390388f, -0.873461f ),
   SParticleVert( -0.853618f, -0.062808f ),
   SParticleVert( -0.541545f,  0.873411f ),
#else
   // Area reduced to 52.35% (optimal convex area is 50.92%)
   SParticleVert(  0.557581f,  0.733796f ),
   SParticleVert(  0.913542f,  0.021875f ),
   SParticleVert(  0.678350f, -0.683699f ),
   SParticleVert( -0.098080f, -0.803150f ),
   SParticleVert( -0.530160f, -0.563106f ),
   SParticleVert( -0.820450f, -0.055099f ),
   SParticleVert( -0.618056f,  0.552083f ),
   SParticleVert( -0.184935f,  0.811956f ),
#endif
};

// bombgas4_alp.tga
SParticleVert const gBombGas4_Alp_Verts[] =
{
  // Area reduced to 74.25% (optimal convex area is 70.80%)
  SParticleVert(  1.092562f,  0.484480f ),
  SParticleVert(  0.760877f, -0.641304f ),
  SParticleVert(  0.233717f, -0.913916f ),
  SParticleVert( -0.979139f, -0.644393f ),
  SParticleVert( -0.773707f,  0.896935f ),
  SParticleVert(  0.602279f,  0.886742f ),
};

// bombgas6_alp.tga
SParticleVert const gBombGas6_Alp_Verts[] =
{
  // Area reduced to 69.54% (optimal convex area is 66.76%)
  SParticleVert(  0.848146f,  0.515042f ),
  SParticleVert(  1.028987f, -0.796050f ),
  SParticleVert( -0.889513f, -0.792911f ),
  SParticleVert( -0.887750f,  0.414089f ),
  SParticleVert( -0.405793f,  0.792770f ),
  SParticleVert(  0.495325f,  0.867863f ),
};

// bombgas8_alp.tga
SParticleVert const gBombGas8_Alp_Verts[] =
{
  // Area reduced to 79.31% (optimal convex area is 77.47%)
  SParticleVert(  0.981792f,  0.368031f ),
  SParticleVert(  0.884016f, -0.614099f ),
  SParticleVert(  0.139696f, -1.024758f ),
  SParticleVert( -0.912117f, -0.616647f ),
  SParticleVert( -1.022418f,  0.172010f ),
  SParticleVert( -0.569940f,  1.015253f ),
  SParticleVert(  0.543108f,  0.964660f ),
};

// smoke03_alp.tga
SParticleVert const gSmoke03_Alp_Verts[] =
{
  // Area reduced to 63.53% (optimal convex area is 61.62%)
  SParticleVert(  0.846875f,  0.607813f ),
  SParticleVert(  0.846875f, -0.676094f ),
  SParticleVert(  0.556886f, -0.966083f ),
  SParticleVert( -0.457627f, -0.689397f ),
  SParticleVert( -1.036471f, -0.199606f ),
  SParticleVert( -0.438845f,  0.929243f ),
};

// fire3_alp.tga
SParticleVert const gFire3_Alp_Verts[] =
{
  // Area reduced to 76.55% (optimal convex area is 74.35%)
  SParticleVert(  1.041690f,  0.766356f ),
  SParticleVert(  0.799095f, -0.871161f ),
  SParticleVert( -0.423430f, -0.920062f ),
  SParticleVert( -0.949446f, -0.469191f ),
  SParticleVert( -0.987098f,  0.622729f ),
  SParticleVert( -0.201404f,  0.858437f ),
};

// splash03_alp.tga
SParticleVert const gSplash03_Alp_Verts[] =
{
  // Area reduced to 68.14% (optimal convex area is 63.31%)
  SParticleVert(  0.990612f,  0.159401f ),
  SParticleVert(  0.875727f, -0.587351f ),
  SParticleVert(  0.081312f, -0.940424f ),
  SParticleVert( -0.675039f, -0.637884f ),
  SParticleVert( -0.881319f,  0.668554f ),
  SParticleVert(  0.541953f,  1.056719f ),
};

#define BP_OPTIMIZED_PRIM_SHAPE_MAX_VERTEX_COUNT 8

#define BP_OPTIMIZED_PRIM_SHAPE_COUNT 8
static SPrimShape const gPrimShapes[BP_OPTIMIZED_PRIM_SHAPE_COUNT] =
{
   SPrimShape(0, GV_StrCode("bombgas1_alp"), BPE_ARRAY_SIZE(gBombGas1_Alp_Verts),   gBombGas1_Alp_Verts),
   SPrimShape(1, GV_StrCode("bombgas3_alp"), BPE_ARRAY_SIZE(gBombGas3_Alp_Verts),   gBombGas3_Alp_Verts),
   SPrimShape(2, GV_StrCode("bombgas4_alp"), BPE_ARRAY_SIZE(gBombGas4_Alp_Verts),   gBombGas4_Alp_Verts),
   SPrimShape(3, GV_StrCode("bombgas6_alp"), BPE_ARRAY_SIZE(gBombGas6_Alp_Verts),   gBombGas6_Alp_Verts),
   SPrimShape(4, GV_StrCode("bombgas8_alp"), BPE_ARRAY_SIZE(gBombGas8_Alp_Verts),   gBombGas8_Alp_Verts),
   SPrimShape(5, GV_StrCode("smoke03_alp"),  BPE_ARRAY_SIZE(gSmoke03_Alp_Verts),    gSmoke03_Alp_Verts),
   SPrimShape(6, GV_StrCode("fire3_alp"),    BPE_ARRAY_SIZE(gFire3_Alp_Verts),      gFire3_Alp_Verts),
   SPrimShape(7, GV_StrCode("splash03_alp"), BPE_ARRAY_SIZE(gSplash03_Alp_Verts),   gSplash03_Alp_Verts),
};

//----------------------------------------------------------------------------

void* BP_Prim_LookupPrimShape(DG_PRIM *pPrim)
{
#if MGS_VERSION == 3
   if (!(pPrim->flag & DG_PRIM_AS_NO_OPT_SHAPE))
   {
      DG_TEX* pTex = (DG_TEX*) pPrim->BP_tex;

      unsigned texId = pTex->tex_id;

      if (texId)
      {
         for( int i = 0; i < BPE_ARRAY_SIZE(gPrimShapes); ++i )
         {
            SPrimShape const & shape = gPrimShapes[i];

            if( shape.mTexId == texId )
               return (void*)&shape;
         }
      }
   }
#endif
   return NULL;
}

//----------------------------------------------------------------------------

int const BP_Prim_GetSpriteVertexCount(DG_PRIM* pPrim)
{
#if MGS_VERSION == 3
   SPrimShape const * pPrimShape = (SPrimShape const *)pPrim->BP_primShape;

   if( gPrim_OptimizeParticleShape && pPrimShape)
      return pPrimShape->mNumVerts;
   else
#endif
      return 4;
}

//----------------------------------------------------------------------------

namespace PrimShader
{
   int const kTextureCount = 4;
   int const kSpriteCount = 2;
   int const kFogCount = 2;
   int const kForceRGBWhiteCount = 2;
   int const kPS2ColorClampCount = 2;
   CCompiledShader*  gpShader[kTextureCount][kSpriteCount][kFogCount][kForceRGBWhiteCount][kPS2ColorClampCount];
#if BP_VITA
   CCompiledShader*  gpSpriteShader[kTextureCount][kFogCount][kForceRGBWhiteCount][kPS2ColorClampCount];
#endif

   uint32 gFogColorParameter;

   enum ERegisters
   {
      kReg_Screen          = 16,
      kReg_Pers            = 20,
      kReg_UvScaleOffset   = 24,
      kReg_UvScaleOffset1  = 25
   };
}

static CVector4 const gBP_BlackFogColor(0, 0, 0, 0);
static CVertexData gSpritePrim_VertexData;
static CVertexData gDefaultPrim_VertexData;
static CVertexData gMaskPrim_VertexData;
#if BP_VITA
static CVertexData gSpriteMaskPrim_VertexData;
static CVertexData gSpriteMaskPrimInstanced_VertexData;
static CVertexData gSpritePrimInstanced_VertexData;
static CStaticVertexBuffer* gSpriteInstanceVerticesSprt;
static CStaticVertexBuffer* gSpriteInstanceVerticesRSprt;
static CStaticIndexBuffer* gSpriteInstanceIndices;

#  if MGS_VERSION == 3
static CStaticVertexBuffer *gOptimizedSpriteInstanceVerticesSprt[BP_OPTIMIZED_PRIM_SHAPE_COUNT];
static CStaticVertexBuffer *gOptimizedSpriteInstanceVerticesRSprt[BP_OPTIMIZED_PRIM_SHAPE_COUNT];
static CStaticIndexBuffer  *gOptimizedSpriteInstanceIndices[BP_OPTIMIZED_PRIM_SHAPE_MAX_VERTEX_COUNT + 1];
#  endif

static NVTAState::SCachedVertexData *gpSpritePrim_CVD;
static NVTAState::SCachedVertexData *gpSpritePrimInstanced_CVD;
static NVTAState::SCachedVertexData *gpDefaultPrim_CVD;
static NVTAState::SCachedVertexData *gpMaskPrim_CVD;
static NVTAState::SCachedVertexData *gpSpriteMaskPrim_CVD;
static NVTAState::SCachedVertexData *gpSpriteMaskPrimInstanced_CVD;
#endif

//----------------------------------------------------------------------------

void BP_InitPrimShader()
{
   // First we set up the sprite prim and default prim vertex data structures
   // Note that these will start off with invalid buffer indices
   gSpritePrim_VertexData.SetAttribute(kVDS_Position, offsetof(BP_Prim_SpriteVertex, x) , kVDT_Float3, 0 );
   gSpritePrim_VertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_Prim_SpriteVertex, u), kVDT_Short4, 0 );
   gSpritePrim_VertexData.SetAttribute(kVDS_Color0, offsetof(BP_Prim_SpriteVertex, rgba), kVDT_UByte4N, 0 );
#if BP_VITA
   gSpriteMaskPrim_VertexData.SetAttribute(kVDS_Position, offsetof(BP_Prim_SpriteVertex_Mask, x) , kVDT_Float3, 0 );
   gSpriteMaskPrim_VertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_Prim_SpriteVertex_Mask, u), kVDT_Short4, 0 );
   gSpriteMaskPrim_VertexData.SetAttribute(kVDS_Color0, offsetof(BP_Prim_SpriteVertex_Mask, rgba), kVDT_UByte4N, 0 );
   gSpriteMaskPrim_VertexData.SetAttribute(kVDS_TexCoord1, offsetof(BP_Prim_SpriteVertex_Mask, mu), kVDT_Short4, 0 );

   gSpritePrimInstanced_VertexData.SetAttribute(kVDS_Position, offsetof(BP_Prim_SpriteVertexInstanced, x), kVDT_Float3, 0);
   gSpritePrimInstanced_VertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_Prim_SpriteVertexInstanced, u0), kVDT_Short4, 0);
   gSpritePrimInstanced_VertexData.SetAttribute(kVDS_TexCoord1, offsetof(BP_Prim_SpriteVertexInstanced, dx), kVDT_Short2, 0);
   gSpritePrimInstanced_VertexData.SetAttribute(kVDS_Color0, offsetof(BP_Prim_SpriteVertexInstanced, rgba), kVDT_UByte4N, 0);
   gSpritePrimInstanced_VertexData.SetAttribute(kVDS_TexCoord2, 0, kVDT_Float4, 1);
   gSpritePrimInstanced_VertexData.SetAttribute(kVDS_TexCoord3, 16, kVDT_Float4, 1);

   gSpriteMaskPrimInstanced_VertexData.SetAttribute(kVDS_Position, offsetof(BP_Prim_SpriteVertexInstanced_Mask, x), kVDT_Float3, 0);
   gSpriteMaskPrimInstanced_VertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_Prim_SpriteVertexInstanced_Mask, u0), kVDT_Short4, 0);
   gSpriteMaskPrimInstanced_VertexData.SetAttribute(kVDS_TexCoord1, offsetof(BP_Prim_SpriteVertexInstanced_Mask, dx), kVDT_Short2, 0);
   gSpriteMaskPrimInstanced_VertexData.SetAttribute(kVDS_Color0, offsetof(BP_Prim_SpriteVertexInstanced_Mask, rgba), kVDT_UByte4N, 0);
   gSpriteMaskPrimInstanced_VertexData.SetAttribute(kVDS_TexCoord4, offsetof(BP_Prim_SpriteVertexInstanced_Mask, mu0), kVDT_Short4, 0);
   gSpriteMaskPrimInstanced_VertexData.SetAttribute(kVDS_TexCoord2, 0, kVDT_Float4, 1);
   gSpriteMaskPrimInstanced_VertexData.SetAttribute(kVDS_TexCoord3, 16, kVDT_Float4, 1);

   gSpriteInstanceVerticesSprt = new CStaticVertexBuffer(sizeof(FVECTOR) * 8);
   gSpriteInstanceVerticesRSprt = new CStaticVertexBuffer(sizeof(FVECTOR) * 8);
   gSpriteInstanceIndices = new CStaticIndexBuffer(kIT_Uint16, 6);

   {
      FVECTOR *v = reinterpret_cast<FVECTOR *>( gSpriteInstanceVerticesSprt->Lock() );
      // xofs, yofs, u lerp, v lerp
      v[0].vx = -1.0f; v[0].vy = -1.0f; v[0].vz = 0.0f; v[0].vw = 0.0f;
      v[1].vx =  0.0f; v[1].vy =  0.0f; v[1].vz = 0.0f; v[1].vw = 0.0f;
      v[2].vx =  1.0f; v[2].vy = -1.0f; v[2].vz = 0.0f; v[2].vw = 0.0f;
      v[3].vx =  1.0f; v[3].vy =  0.0f; v[3].vz = 0.0f; v[3].vw = 0.0f;
      v[4].vx =  1.0f; v[4].vy =  1.0f; v[4].vz = 0.0f; v[4].vw = 0.0f;
      v[5].vx =  1.0f; v[5].vy =  1.0f; v[5].vz = 0.0f; v[5].vw = 0.0f;
      v[6].vx = -1.0f; v[6].vy =  1.0f; v[6].vz = 0.0f; v[6].vw = 0.0f;
      v[7].vx =  0.0f; v[7].vy =  1.0f; v[7].vz = 0.0f; v[7].vw = 0.0f;

      v = reinterpret_cast<FVECTOR *>( gSpriteInstanceVerticesRSprt->Lock() );
      // xofs, yofs, u lerp, v lerp
      v[0].vx = -1.0f; v[0].vy = -1.0f; v[0].vz =  0.0f; v[0].vw =  0.0f;
      v[1].vx =  0.0f; v[1].vy =  0.0f; v[1].vz =  0.0f; v[1].vw =  0.0f;
      
      v[2].vx =  0.0f; v[2].vy =  0.0f; v[2].vz =  1.0f; v[2].vw = -1.0f;
      v[3].vx =  1.0f; v[3].vy =  0.0f; v[3].vz =  0.0f; v[3].vw =  0.0f;
      
      v[4].vx =  1.0f; v[4].vy =  1.0f; v[4].vz =  0.0f; v[4].vw =  0.0f;
      v[5].vx =  1.0f; v[5].vy =  1.0f; v[5].vz =  0.0f; v[5].vw =  0.0f;
      
      v[6].vx =  0.0f; v[6].vy =  0.0f; v[6].vz = -1.0f; v[6].vw =  1.0f;
      v[7].vx =  0.0f; v[7].vy =  1.0f; v[7].vz =  0.0f; v[7].vw =  0.0f;
      
      uint16 *idx = reinterpret_cast<uint16 *>( gSpriteInstanceIndices->Lock() );
      idx[0] = 0;
      idx[1] = 1;
      idx[2] = 2;
      idx[3] = 0;
      idx[4] = 2;
      idx[5] = 3;
   }

#  if MGS_VERSION == 3
   for (int i = 0; i < BPE_ARRAY_SIZE(gPrimShapes); ++i)
   {
      SPrimShape const *primShape = &gPrimShapes[i];
      gOptimizedSpriteInstanceVerticesSprt[i] = new CStaticVertexBuffer(2*primShape->mNumVerts*sizeof(FVECTOR));
      gOptimizedSpriteInstanceVerticesRSprt[i] = new CStaticVertexBuffer(2*primShape->mNumVerts*sizeof(FVECTOR));

      FVECTOR *vs = reinterpret_cast<FVECTOR *>(gOptimizedSpriteInstanceVerticesSprt[i]->Lock());
      FVECTOR *vr = reinterpret_cast<FVECTOR *>(gOptimizedSpriteInstanceVerticesRSprt[i]->Lock());
      SParticleVert const *pv = primShape->mpVerts;
      for (int j = 0; j < primShape->mNumVerts; ++j)
      {
         vs->vx = pv->originalU;
         vs->vy = pv->originalV;
         vs->vz = 0.0f;
         vs->vw = 0.0f;
         ++vs;
         vs->vx = 0.5f + 0.5f*pv->originalU;
         vs->vy = 0.5f + 0.5f*pv->originalV;
         vs->vz = 0.0f;
         vs->vw = 0.0f;
         ++vs;

         {
            // for rotated (45 degrees only!) sprites
            // see the SParticleVert constructor
            float rX = 0.5f*(pv->originalU + pv->originalV);
            float rY = 0.5f*(pv->originalV - pv->originalU);
            vr->vx = rX;
            vr->vy = rX;
            vr->vz = -rY;
            vr->vw = rY;
         }
         ++vr;

         vr->vx = 0.5f + 0.5f*pv->originalU;
         vr->vy = 0.5f + 0.5f*pv->originalV;
         vr->vz = 0.0f;
         vr->vw = 0.0f;
         ++vr;

         ++pv;
      }
   }
   for (int i = 5; i <= BP_OPTIMIZED_PRIM_SHAPE_MAX_VERTEX_COUNT; ++i)
   {
      gOptimizedSpriteInstanceIndices[i] = new CStaticIndexBuffer(kIT_Uint16, 3*(i - 2));
      uint16 *idx = reinterpret_cast<uint16 *>(gOptimizedSpriteInstanceIndices[i]->Lock());
      for (int j = 0; j < i - 2; ++j)
      {
         idx[3*j    ] = 0;
         idx[3*j + 1] = j + 1;
         idx[3*j + 2] = j + 2;
      }
   }
#  endif

#endif

   gDefaultPrim_VertexData.SetAttribute(kVDS_Position, offsetof(BP_Prim_Vertex, x),  kVDT_Float3, 0);
   gDefaultPrim_VertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_Prim_Vertex, u), kVDT_Short4, 0);
   gDefaultPrim_VertexData.SetAttribute(kVDS_Color0, offsetof(BP_Prim_Vertex, rgba), kVDT_UByte4N, 0);

   gMaskPrim_VertexData.SetAttribute(kVDS_Position, offsetof(BP_Prim_Vertex_Mask, x), kVDT_Float3, 0);
   gMaskPrim_VertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_Prim_Vertex_Mask, u), kVDT_Short4, 0);
   gMaskPrim_VertexData.SetAttribute(kVDS_Color0, offsetof(BP_Prim_Vertex_Mask, rgba), kVDT_UByte4N, 0);
   gMaskPrim_VertexData.SetAttribute(kVDS_TexCoord1, offsetof(BP_Prim_Vertex_Mask, u1), kVDT_Short4, 0);

   CShaderFileId const shaderId("$/EngineSupport/Shaders/Prim.fx");

   for( int isTexture = 0; isTexture < PrimShader::kTextureCount; ++isTexture )
   {
      for( int isSprite = 0; isSprite < PrimShader::kSpriteCount; ++isSprite )
      {
         for( int isFog = 0; isFog < PrimShader::kFogCount; ++isFog )
         {
            for( int forceRGBWhite = 0; forceRGBWhite < PrimShader::kForceRGBWhiteCount; ++forceRGBWhite )
            {
               for( int colorClamp = 0; colorClamp < PrimShader::kPS2ColorClampCount; ++colorClamp )
               {
                  CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d;SPRITE=%d;FOG=%d;FORCE_RGB_WHITE=%d;PS2_COLOR_CLAMP=%d", isTexture, isSprite, isFog, forceRGBWhite, colorClamp));
                  assert(pShader);

                  PrimShader::gpShader[isTexture][isSprite][isFog][forceRGBWhite][colorClamp] = pShader;
               }
            }
         }
      }
   }

#if BP_VITA
   CShaderFileId const spriteShaderId("$/EngineSupport/Shaders/Sprite.fx");

   for( int isTexture = 0; isTexture < PrimShader::kTextureCount; ++isTexture )
   {
      for( int isFog = 0; isFog < PrimShader::kFogCount; ++isFog )
      {
         for( int forceRGBWhite = 0; forceRGBWhite < PrimShader::kForceRGBWhiteCount; ++forceRGBWhite )
         {
            for( int colorClamp = 0; colorClamp < PrimShader::kPS2ColorClampCount; ++colorClamp )
            {
               CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(spriteShaderId, CStringExtras::Stringize_s("TEXTURE=%d;FOG=%d;FORCE_RGB_WHITE=%d;PS2_COLOR_CLAMP=%d", isTexture, isFog, forceRGBWhite, colorClamp));
               assert(pShader);
               
               PrimShader::gpSpriteShader[isTexture][isFog][forceRGBWhite][colorClamp] = pShader;
            }
         }
      }
   }

   {
      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_TexCoord1);
      binding.Set(kVDU_TexCoord2, kVDS_Color0);
      binding.Set(kVDU_TexCoord3, kVDS_TexCoord2);
      binding.Set(kVDU_TexCoord4, kVDS_TexCoord3);

      int bidx = gSpritePrimInstanced_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( BP_Prim_SpriteVertexInstanced ));
      gSpritePrimInstanced_VertexData.SetInstanced(true, bidx);
      gSpritePrimInstanced_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( FVECTOR ) * 2);
      gpSpritePrimInstanced_CVD = new NVTAState::SCachedVertexData(binding, gSpritePrimInstanced_VertexData);

      binding.Set(kVDU_TexCoord5, kVDS_TexCoord4);

      bidx = gSpriteMaskPrimInstanced_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( BP_Prim_SpriteVertexInstanced_Mask ));
      gSpriteMaskPrimInstanced_VertexData.SetInstanced(true, bidx);
      gSpriteMaskPrimInstanced_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( FVECTOR ) * 2);
      gpSpriteMaskPrimInstanced_CVD = new NVTAState::SCachedVertexData(binding, gSpriteMaskPrimInstanced_VertexData);
   }

   {
      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      gSpritePrim_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( BP_Prim_SpriteVertex ));
      gDefaultPrim_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( BP_Prim_Vertex ));
      gMaskPrim_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( BP_Prim_Vertex_Mask ));
      gSpriteMaskPrim_VertexData.AddBuffer_Unsafe((void*)0x80000000, sizeof( BP_Prim_SpriteVertex_Mask ));

      gpSpritePrim_CVD = new NVTAState::SCachedVertexData(binding, gSpritePrim_VertexData);
      gpDefaultPrim_CVD = new NVTAState::SCachedVertexData(binding, gDefaultPrim_VertexData);
      binding.Set(kVDU_TexCoord2, kVDS_TexCoord1);
      gpMaskPrim_CVD = new NVTAState::SCachedVertexData(binding, gMaskPrim_VertexData);
      gpSpriteMaskPrim_CVD = new NVTAState::SCachedVertexData(binding, gSpriteMaskPrim_VertexData);
   }

   // Create default parameter buffers.
   {
      if( gpBlackUniformBuffer == NULL )
      {
         CRenderHWAllocator::SHandle const *h = RenderBackend()->AllocFixed(sizeof(CVector4), 4, kRM_System); 
         gpBlackUniformBuffer = (float*)h->mpAddress;
         memset(gpBlackUniformBuffer, 0, sizeof(CVector4));
      }
   }
#else
   // Create default parameter buffers.
   {
      if( gpBlackParameterBuffer == NULL && gpFogColorParameterBuffer == NULL )
      {
         gpBlackParameterBuffer = (uint8*)BPE_MALLOC_ALIGNED(128, 64);
         gpFogColorParameterBuffer = (uint8*)BPE_MALLOC_ALIGNED(128, 64);
      }

      PrimShader::gFogColorParameter = CShaderCRCs::GetParameterCRC("gPS_FogColor");

      CShaderParameterBuffer buffer(gpBlackParameterBuffer);
      buffer.AddParameter(PrimShader::gFogColorParameter, gBP_BlackFogColor);
      gParticleParameterBufferSize = buffer.GetSize();
   }
#endif
}

//----------------------------------------------------------------------------

void BP_Prim_FrameBegin()
{
   gIsFirstPrimForViewport = 1;

   gNumPrims = 0;
   gNumPrimPackets = 0;

   gNumPrimInputVerts = 0;
   gNumPrimOutputVerts = 0;

}

void BP_Prim_FrameEnd()
{
   if( gPrim_ShowStats )
   {
      BP_DebugText_Print("Prim Count: %d Prim Packets: %d Input Vertices : %d Output Vertices: %d", gNumPrims, gNumPrimPackets, gNumPrimInputVerts, gNumPrimOutputVerts);
   }
}

//----------------------------------------------------------------------------

SRenderTarget gPrimOffscreen_RenderTargetBackup;
int gPrimOffscren_Enable = 1;
int gPrimOffscreen_Active = 0;
int gPrimOffscreen_PrimCount = 0;

void BP_Prim_BeginOffscreen()
{
#if BP_PRIM_OFFSCREEN_ENABLE
   if( gPrimOffscren_Enable )
   {
      BPE_GPU_PUSH_PROFILE_MARKER("Offscreen Prim Render");

      gPrimOffscreen_RenderTargetBackup = gpRenderBackend->GetCurrentRenderTarget();
      gPrimOffscreen_Active = 1;
      gPrimOffscreen_PrimCount = 0;

      // Setup offscreen prim alpha function
      BP_GS_SetAlphaFunc(BP_GS_SetAlpha_OffscreenPrim);

      SRenderTarget const primTarget(BP_GetRenderTarget(kRT_TempBufferQuarter1), NULL, NULL, NULL, NULL);
      gpRenderBackend->SetRenderTarget(primTarget);
   }
#endif
}

//----------------------------------------------------------------------------

void BP_Prim_EndOffscreen()
{
#if BP_PRIM_OFFSCREEN_ENABLE
   if( gPrimOffscren_Enable )
   {
      gPrimOffscreen_Active = 0;

      // Setup default alpha function
      BP_GS_SetAlphaFunc(BP_GS_SetAlpha_Default);

      CBaseTexture* pOffscreenTexture = BP_GetRenderTarget(kRT_TempBufferQuarter1);

      // Resolve render target if anything got rendered to it.
      if( gPrimOffscreen_PrimCount > 0 )
         gpRenderBackend->ResolveRenderTarget(pOffscreenTexture);

      // Restore original render target
      gpRenderBackend->SetRenderTarget(gPrimOffscreen_RenderTargetBackup);

      // Apply offscreen buffer
      if( gPrimOffscreen_PrimCount > 0 )
      {
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Apply offscreen");

         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add, CRenderBackend::kBO_Add);
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);

         gpRenderBackend->SetAlphaTestEnable(false);
         gpRenderBackend->SetDepthCompareEnabled(false);
         gpRenderBackend->SetStencilEnable(false);

         BP_DrawFullscreenTexture(pOffscreenTexture, 1 /*isFilter*/, 1 /*isRGBA*/);

         gpRenderBackend->SetDepthCompareEnabled(true);
      }

      BPE_GPU_POP_PROFILE_MARKER();
   }
#endif
}

//----------------------------------------------------------------------------

void BP_Prim_RenderOffscreenBegin()
{
   if( gPrimOffscren_Enable )
   {
      if( gPrimOffscreen_Active )
      {
         if( gPrimOffscreen_PrimCount == 0 )
         {
            gpRenderBackend->Clear(CRenderBackend::kFlag_Color, CColor(0, 0, 0, 255));
         }

         ++gPrimOffscreen_PrimCount;
      }
   }
}

//----------------------------------------------------------------------------

#if BPE_IS_ENDIAN_BIG()
   #define BP_PACK_COLOR(R, G, B, A) ( R << 24 | G << 16 | B << 8 | A )
#else
   #define BP_PACK_COLOR(R, G, B, A) ( A << 24 | B << 16 | G << 8 | R )
#endif

//----------------------------------------------------------------------------

#if MGS_VERSION == 3
int BP_Prim_RenderPoly_Mask( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet)
{
   int vertexCount = 0;

   BP_Prim_Vertex_Mask * __restrict pVertex = (BP_Prim_Vertex_Mask*)pVertexBuffer->Lock();
   uint16 * pFirstIndex = (uint16*)CDynamicIndexBufferPool_UT::LockChunk( indexBuffer );
   uint16 * pIndex = pFirstIndex;

   for( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR* __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGB* __restrict uvrgb0 = (DG_PRIM_UVRGB*)packet->uvrgb_addr;
      DG_PRIM_UVRGB* __restrict uvrgb1 = (DG_PRIM_UVRGB*)packet->child_packet->uvrgb_addr;

      uint16* pStartIndex = pIndex;

      packet->BP_startIndex = pIndex - pFirstIndex;

      for( int i = 0 ; i < packet->n_verts; i++, pos++, uvrgb0++, uvrgb1++  )
      {
         BPE_DCACHE_PREFETCH(pos + 1);
         pVertex->x = pos->vx;
         pVertex->y = pos->vy;
         pVertex->z = pos->vz;

         pVertex->rgba = BP_PACK_COLOR( uvrgb1->r, uvrgb1->g, uvrgb1->b, uvrgb0->a );

         pVertex->u = uvrgb0->u;
         pVertex->v = uvrgb0->v;
         pVertex->q = uvrgb0->q;

         pVertex->u1 = uvrgb1->u;
         pVertex->v1 = uvrgb1->v;
         pVertex->q1 = uvrgb1->q;

         ++pVertex;

         BPE_DCACHE_PREFETCH(uvrgb0 + 1);

         if( (vertexCount >= 2) && (uvrgb0->f & 0x8000) )
         {
            *pIndex++ = vertexCount - 1;
         }

         *pIndex++ = vertexCount++;
      }

      packet->BP_indexCount = pIndex - pStartIndex;

      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   CDynamicIndexBufferPool_UT::UnlockChunk( indexBuffer );
   pVertexBuffer->Unlock();

   return vertexCount;
}
#endif

//----------------------------------------------------------------------------

int BP_Prim_RenderPoly( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet)
{
#if MGS_VERSION == 3
   if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
      return BP_Prim_RenderPoly_Mask( pVertexBuffer, indexBuffer, pPrim, packet);
#endif
   int vertexCount = 0;

   BP_Prim_Vertex * __restrict pVertex = (BP_Prim_Vertex*)pVertexBuffer->Lock();
   uint16 * pFirstIndex = (uint16*)CDynamicIndexBufferPool_UT::LockChunk( indexBuffer );
   uint16 * pIndex = pFirstIndex;

   for( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR* __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGB* __restrict uvrgb = (DG_PRIM_UVRGB*)packet->uvrgb_addr;

      uint16* pStartIndex = pIndex;

      packet->BP_startIndex = pIndex - pFirstIndex;

      for( int i = 0 ; i < packet->n_verts; i++, pos++, uvrgb++  )
      {
         BPE_DCACHE_PREFETCH(pos + 1);
         pVertex->x = pos->vx;
         pVertex->y = pos->vy;
         pVertex->z = pos->vz;

         pVertex->rgba = BP_PACK_COLOR( uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a );

         pVertex->u = uvrgb->u;
         pVertex->v = uvrgb->v;
         pVertex->q = uvrgb->q;

         ++pVertex;

         BPE_DCACHE_PREFETCH(uvrgb + 1);

         if( (vertexCount >= 2) && (uvrgb->f & 0x8000) )
         {
            *pIndex++ = vertexCount - 1;
         }

         *pIndex++ = vertexCount++;
      }

      packet->BP_indexCount = pIndex - pStartIndex;

      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   CDynamicIndexBufferPool_UT::UnlockChunk( indexBuffer );
   pVertexBuffer->Unlock();

   return vertexCount;
}

//----------------------------------------------------------------------------

int BP_Prim_RenderLine( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
   int vertexCount = 0;

   int indexCount = 0;

   BP_Prim_Vertex* __restrict pVertex = (BP_Prim_Vertex*)pVertexBuffer->Lock();
   uint16 * __restrict pIndex = (uint16*)CDynamicIndexBufferPool_UT::LockChunk( indexBuffer );

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR* __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGB* __restrict uvrgb = (DG_PRIM_UVRGB*)packet->uvrgb_addr;

      packet->BP_startIndex = indexCount;

      for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgb++ )
      {
         BPE_DCACHE_PREFETCH( pos );
         BPE_DCACHE_PREFETCH( uvrgb );
         bp_neon_copy_vec3( &pVertex->x, &pos->vx );

         pVertex->rgba = BP_PACK_COLOR( uvrgb->r, uvrgb->g, uvrgb->b, uvrgb->a );

         bp_neon_copy_s16_3( &pVertex->u, (const short*)&uvrgb->u );

         pVertex++ ;
         if ( ( i != 0 ) && ( !( uvrgb->f & 0x8000 ) ) )
         {
            pIndex[ 0 ] = vertexCount - 1 ;
            pIndex[ 1 ] = vertexCount ;
            pIndex += 2 ;
            indexCount += 2 ;
         }

         vertexCount++ ;
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   CDynamicIndexBufferPool_UT::UnlockChunk( indexBuffer );
   pVertexBuffer->Unlock();

   return vertexCount;
}

#if MGS_VERSION == 3
//----------------------------------------------------------------------------
int BP_Prim_RenderSprt_Mask( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   int vertexCount = 0;
   int indexCount = 0;

   BPE_ASSERT(!pPrim->BP_instanced, "Somehow got instanced and noninstanced dispatch wrong");

   BP_Prim_SpriteVertex_Mask* __restrict pVertex = (BP_Prim_SpriteVertex_Mask*)pVertexBuffer->Lock();
   uint16 * __restrict pIndex = (uint16*)CDynamicIndexBufferPool_UT::LockChunk( indexBuffer );

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR const * __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh2 = (DG_PRIM_UVRGBWH*)packet->child_packet->uvrgb_addr;
      packet->BP_startIndex = indexCount;

#if MGS_VERSION == 3
      SPrimShape const * pPrimShape = (SPrimShape const *)pPrim->BP_primShape;

      if( gPrim_OptimizeParticleShape && pPrimShape )
      {
         SParticleVert const * pVerts = pPrimShape->mpVerts;

         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );
            uint32 const rgba = BP_PACK_COLOR( uvrgbwh2->r, uvrgbwh2->g, uvrgbwh2->b, uvrgbwh->a );

            for( int k = 0; k < spriteVertexCount; ++k )
            {
               pVertex->x = pos->vx;
               pVertex->y = pos->vy;
               pVertex->z = pos->vz;

               pVertex->dx = (pVerts[k].sprtX * uvrgbwh->w) >> 16;
               pVertex->dy = (pVerts[k].sprtY * uvrgbwh->h) >> 16;

               pVertex->rgba = rgba;

               pVertex->u = uvrgbwh->u0 + (((uvrgbwh->u1 - uvrgbwh->u0) * pVerts[k].u) >> 16);
               pVertex->v = uvrgbwh->v0 + (((uvrgbwh->v1 - uvrgbwh->v0) * pVerts[k].v) >> 16);

               pVertex->mu = uvrgbwh2->u0 + (((uvrgbwh2->u1 - uvrgbwh2->u0) * pVerts[k].u) >> 16);
               pVertex->mv = uvrgbwh2->v0 + (((uvrgbwh2->v1 - uvrgbwh2->v0) * pVerts[k].v) >> 16);

               pVertex++;
            }

            for( int k = 0; k < (spriteVertexCount - 2); ++k )
            {
               *pIndex++ = vertexCount;
               *pIndex++ = vertexCount + k + 1;
               *pIndex++ = vertexCount + k + 2;
            }

            indexCount += spriteIndexCount;
            vertexCount += spriteVertexCount;
         }
      }
      else
#endif
      {
         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (gPrim_CullAlpha && gGS_SrcAlphaBlend && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex[0].x, currentPos );
               bp_neon_store_vec3( &pVertex[1].x, currentPos );
               bp_neon_store_vec3( &pVertex[2].x, currentPos );
               bp_neon_store_vec3( &pVertex[3].x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh2->r, uvrgbwh2->g, uvrgbwh2->b, uvrgbwh->a );
               pVertex[0].rgba = pVertex[1].rgba = pVertex[2].rgba = pVertex[3].rgba = rgba;
            }

            {
               int16 w = uvrgbwh->w;
               int16 h = uvrgbwh->h;

               pVertex[0].dx = -w;
               pVertex[1].dx =  w;
               pVertex[2].dx =  w;
               pVertex[3].dx = -w;

               pVertex[0].dy = -h;
               pVertex[1].dy = -h;
               pVertex[2].dy =  h;
               pVertex[3].dy =  h;
            }

            {
               int16 u0 = uvrgbwh->u0;
               int16 u1 = uvrgbwh->u1;
               int16 v0 = uvrgbwh->v0;
               int16 v1 = uvrgbwh->v1;

               pVertex[0].u = u0;
               pVertex[1].u = u1;
               pVertex[2].u = u1;
               pVertex[3].u = u0;

               pVertex[0].v = v0;
               pVertex[1].v = v0;
               pVertex[2].v = v1;
               pVertex[3].v = v1;
            }

            {
               int16 u0 = uvrgbwh2->u0;
               int16 u1 = uvrgbwh2->u1;
               int16 v0 = uvrgbwh2->v0;
               int16 v1 = uvrgbwh2->v1;

               pVertex[0].mu = u0;
               pVertex[1].mu = u1;
               pVertex[2].mu = u1;
               pVertex[3].mu = u0;

               pVertex[0].mv = v0;
               pVertex[1].mv = v0;
               pVertex[2].mv = v1;
               pVertex[3].mv = v1;
            }

            pIndex[0] = vertexCount;
            pIndex[1] = vertexCount + 1;
            pIndex[2] = vertexCount + 2;
            pIndex[3] = vertexCount;
            pIndex[4] = vertexCount + 2;
            pIndex[5] = vertexCount + 3;

            pIndex += 6;
            pVertex += 4;

            indexCount += 6;
            vertexCount += 4;
         }
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   CDynamicIndexBufferPool_UT::UnlockChunk( indexBuffer );
   pVertexBuffer->Unlock();
   
   return vertexCount;
}
#endif

//----------------------------------------------------------------------------

int BP_Prim_RenderSprt( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
#if MGS_VERSION == 3
   if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
      return BP_Prim_RenderSprt_Mask( pVertexBuffer, indexBuffer, pPrim, packet);
#endif

   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   int vertexCount = 0;
   int indexCount = 0;

   BPE_ASSERT(!pPrim->BP_instanced, "Somehow got instanced and noninstanced dispatch wrong");

   BP_Prim_SpriteVertex* __restrict pVertex = (BP_Prim_SpriteVertex*)pVertexBuffer->Lock();
   uint16 * __restrict pIndex = (uint16*)CDynamicIndexBufferPool_UT::LockChunk( indexBuffer );

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR const * __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;

      packet->BP_startIndex = indexCount;

#if MGS_VERSION == 3
      SPrimShape const * pPrimShape = (SPrimShape const *)pPrim->BP_primShape;

      if( gPrim_OptimizeParticleShape && pPrimShape )
      {
         SParticleVert const * pVerts = pPrimShape->mpVerts;

         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );
            uint32 const rgba = BP_PACK_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );

            for( int k = 0; k < spriteVertexCount; ++k )
            {
               pVertex->x = pos->vx;
               pVertex->y = pos->vy;
               pVertex->z = pos->vz;

               pVertex->dx = (pVerts[k].sprtX * uvrgbwh->w) >> 16;
               pVertex->dy = (pVerts[k].sprtY * uvrgbwh->h) >> 16;

               pVertex->rgba = rgba;

               pVertex->u = uvrgbwh->u0 + (((uvrgbwh->u1 - uvrgbwh->u0) * pVerts[k].u) >> 16);
               pVertex->v = uvrgbwh->v0 + (((uvrgbwh->v1 - uvrgbwh->v0) * pVerts[k].v) >> 16);

               pVertex++;
            }

            for( int k = 0; k < (spriteVertexCount - 2); ++k )
            {
               *pIndex++ = vertexCount;
               *pIndex++ = vertexCount + k + 1;
               *pIndex++ = vertexCount + k + 2;
            }

            indexCount += spriteIndexCount;
            vertexCount += spriteVertexCount;
         }
      }
      else
#endif
      {
         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (gPrim_CullAlpha && gGS_SrcAlphaBlend && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex[0].x, currentPos );
               bp_neon_store_vec3( &pVertex[1].x, currentPos );
               bp_neon_store_vec3( &pVertex[2].x, currentPos );
               bp_neon_store_vec3( &pVertex[3].x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
               pVertex[0].rgba = pVertex[1].rgba = pVertex[2].rgba = pVertex[3].rgba = rgba;
            }

            {
               int16 w = uvrgbwh->w;
               int16 h = uvrgbwh->h;

               pVertex[0].dx = -w;
               pVertex[1].dx =  w;
               pVertex[2].dx =  w;
               pVertex[3].dx = -w;

               pVertex[0].dy = -h;
               pVertex[1].dy = -h;
               pVertex[2].dy =  h;
               pVertex[3].dy =  h;
            }

            {
               int16 u0 = uvrgbwh->u0;
               int16 u1 = uvrgbwh->u1;
               int16 v0 = uvrgbwh->v0;
               int16 v1 = uvrgbwh->v1;

               pVertex[0].u = u0;
               pVertex[1].u = u1;
               pVertex[2].u = u1;
               pVertex[3].u = u0;

               pVertex[0].v = v0;
               pVertex[1].v = v0;
               pVertex[2].v = v1;
               pVertex[3].v = v1;
            }

            pIndex[0] = vertexCount;
            pIndex[1] = vertexCount + 1;
            pIndex[2] = vertexCount + 2;
            pIndex[3] = vertexCount;
            pIndex[4] = vertexCount + 2;
            pIndex[5] = vertexCount + 3;

            pIndex += 6;
            pVertex += 4;

            indexCount += 6;
            vertexCount += 4;
         }
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   CDynamicIndexBufferPool_UT::UnlockChunk( indexBuffer );
   pVertexBuffer->Unlock();
   
   return vertexCount;
}

//----------------------------------------------------------------------------

#if BP_VITA
#if MGS_VERSION == 3
int BP_Prim_RenderSprtInstanced_Mask( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
   int vertexCount = 0;
   int indexCount = 0;

   BP_Prim_SpriteVertexInstanced_Mask* __restrict pVertex = (BP_Prim_SpriteVertexInstanced_Mask*)pVertexBuffer->Lock();

   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR const * __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh2 = (DG_PRIM_UVRGBWH*)packet->child_packet->uvrgb_addr;

      packet->BP_startIndex = indexCount;

      {
         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (gPrim_CullAlpha && gGS_SrcAlphaBlend && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex->x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh2->r, uvrgbwh2->g, uvrgbwh2->b, uvrgbwh2->a );
               pVertex->rgba = rgba;
            }

            {
               int16 w = uvrgbwh->w;
               int16 h = uvrgbwh->h;

               pVertex->dx = w;
               pVertex->dy = h;
            }

            {
               int16 u0 = uvrgbwh->u0;
               int16 u1 = uvrgbwh->u1;
               int16 v0 = uvrgbwh->v0;
               int16 v1 = uvrgbwh->v1;

               pVertex->u0 = u0;
               pVertex->u1 = u1;
               pVertex->v0 = v0;
               pVertex->v1 = v1;
            }

            {
               int16 u0 = uvrgbwh2->u0;
               int16 u1 = uvrgbwh2->u1;
               int16 v0 = uvrgbwh2->v0;
               int16 v1 = uvrgbwh2->v1;

               pVertex->mu0 = u0;
               pVertex->mu1 = u1;
               pVertex->mv0 = v0;
               pVertex->mv1 = v1;
            }

            pVertex ++;

            indexCount += spriteIndexCount;
            vertexCount ++;
         }
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   pVertexBuffer->Unlock();
   
   return vertexCount;
}
#endif
//----------------------------------------------------------------------------

int BP_Prim_RenderSprtInstanced( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
#if MGS_VERSION == 3
   if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
      return BP_Prim_RenderSprtInstanced_Mask( pVertexBuffer, indexBuffer, pPrim, packet);
#endif

   int vertexCount = 0;
   int indexCount = 0;

   BP_Prim_SpriteVertexInstanced* __restrict pVertex = (BP_Prim_SpriteVertexInstanced*)pVertexBuffer->Lock();

   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR const * __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;

      packet->BP_startIndex = indexCount;

      {
         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (gPrim_CullAlpha && gGS_SrcAlphaBlend && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex->x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
               pVertex->rgba = rgba;
            }

            {
               int16 w = uvrgbwh->w;
               int16 h = uvrgbwh->h;

               pVertex->dx = w;
               pVertex->dy = h;
            }

            {
               int16 u0 = uvrgbwh->u0;
               int16 u1 = uvrgbwh->u1;
               int16 v0 = uvrgbwh->v0;
               int16 v1 = uvrgbwh->v1;

               pVertex->u0 = u0;
               pVertex->u1 = u1;
               pVertex->v0 = v0;
               pVertex->v1 = v1;
            }

            pVertex ++;

            indexCount += spriteIndexCount;
            vertexCount ++;
         }
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   pVertexBuffer->Unlock();
   
   return vertexCount;
}
#endif

//----------------------------------------------------------------------------
#if MGS_VERSION == 3
int BP_Prim_RenderRSprt_Mask( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   int vertexCount = 0;
   int indexCount = 0;

   BPE_ASSERT(!pPrim->BP_instanced, "Somehow got instanced and noninstanced dispatch wrong");

   BP_Prim_SpriteVertex_Mask* __restrict pVertex = (BP_Prim_SpriteVertex_Mask*)pVertexBuffer->Lock();
   uint16 * __restrict pIndex = (uint16*)CDynamicIndexBufferPool_UT::LockChunk( indexBuffer );

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR const* __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh2 = (DG_PRIM_UVRGBWH*)packet->child_packet->uvrgb_addr;

      packet->BP_startIndex = indexCount;

#if MGS_VERSION == 3
      SPrimShape const * pPrimShape = (SPrimShape const *)pPrim->BP_primShape;

      if( gPrim_OptimizeParticleShape && pPrimShape )
      {
         SParticleVert const * pVerts = pPrimShape->mpVerts;

         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );
            uint32 const rgba = BP_PACK_COLOR( uvrgbwh2->r, uvrgbwh2->g, uvrgbwh2->b, uvrgbwh->a );

            for( int k = 0; k < spriteVertexCount; ++k )
            {
               pVertex->x = pos->vx;
               pVertex->y = pos->vy;
               pVertex->z = pos->vz;

               pVertex->dx = (pVerts[k].rsprtX * uvrgbwh->w - pVerts[k].rsprtY * uvrgbwh->h) >> 16;
               pVertex->dy = (pVerts[k].rsprtX * uvrgbwh->h + pVerts[k].rsprtY * uvrgbwh->w) >> 16;

               pVertex->rgba = rgba;

               pVertex->u = uvrgbwh->u0 + (((uvrgbwh->u1 - uvrgbwh->u0) * pVerts[k].u) >> 16);
               pVertex->v = uvrgbwh->v0 + (((uvrgbwh->v1 - uvrgbwh->v0) * pVerts[k].v) >> 16);

               pVertex->mu = uvrgbwh2->u0 + (((uvrgbwh2->u1 - uvrgbwh2->u0) * pVerts[k].u) >> 16);
               pVertex->mv = uvrgbwh2->v0 + (((uvrgbwh2->v1 - uvrgbwh2->v0) * pVerts[k].v) >> 16);

               pVertex++;
            }

            for( int k = 0; k < (spriteVertexCount - 2); ++k )
            {
               *pIndex++ = vertexCount;
               *pIndex++ = vertexCount + k + 1;
               *pIndex++ = vertexCount + k + 2;
            }

            indexCount += spriteIndexCount;
            vertexCount += spriteVertexCount;
         }
      }
      else
#endif
      {
         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (gPrim_CullAlpha && gGS_SrcAlphaBlend && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex[0].x, currentPos );
               bp_neon_store_vec3( &pVertex[1].x, currentPos );
               bp_neon_store_vec3( &pVertex[2].x, currentPos );
               bp_neon_store_vec3( &pVertex[3].x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh2->r, uvrgbwh2->g, uvrgbwh2->b, uvrgbwh->a );
               pVertex[0].rgba = pVertex[1].rgba = pVertex[2].rgba = pVertex[3].rgba = rgba;
            }

            {
               int16 w = uvrgbwh->w;
               int16 h = uvrgbwh->h;

               pVertex[0].dx = -w;
               pVertex[1].dx =  h;
               pVertex[2].dx =  w;
               pVertex[3].dx = -h;

               pVertex[0].dy = -h;
               pVertex[1].dy = -w;
               pVertex[2].dy =  h;
               pVertex[3].dy =  w;
            }

            {
               int16 u0 = uvrgbwh->u0;
               int16 u1 = uvrgbwh->u1;
               int16 v0 = uvrgbwh->v0;
               int16 v1 = uvrgbwh->v1;

               pVertex[0].u = u0;
               pVertex[1].u = u1;
               pVertex[2].u = u1;
               pVertex[3].u = u0;

               pVertex[0].v = v0;
               pVertex[1].v = v0;
               pVertex[2].v = v1;
               pVertex[3].v = v1;
            }

            {
               int16 u0 = uvrgbwh2->u0;
               int16 u1 = uvrgbwh2->u1;
               int16 v0 = uvrgbwh2->v0;
               int16 v1 = uvrgbwh2->v1;

               pVertex[0].mu = u0;
               pVertex[1].mu = u1;
               pVertex[2].mu = u1;
               pVertex[3].mu = u0;

               pVertex[0].mv = v0;
               pVertex[1].mv = v0;
               pVertex[2].mv = v1;
               pVertex[3].mv = v1;
            }

            pIndex[0] = vertexCount;
            pIndex[1] = vertexCount + 1;
            pIndex[2] = vertexCount + 2;
            pIndex[3] = vertexCount;
            pIndex[4] = vertexCount + 2;
            pIndex[5] = vertexCount + 3;

            pIndex += 6;
            pVertex += 4;

            indexCount += 6;
            vertexCount += 4;
         }
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   CDynamicIndexBufferPool_UT::UnlockChunk( indexBuffer );
   pVertexBuffer->Unlock();
   return vertexCount;
}
#endif
//----------------------------------------------------------------------------

int BP_Prim_RenderRSprt( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
#if MGS_VERSION == 3
   if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
      return BP_Prim_RenderRSprt_Mask( pVertexBuffer, indexBuffer, pPrim, packet);
#endif

   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   int vertexCount = 0;
   int indexCount = 0;

   BPE_ASSERT(!pPrim->BP_instanced, "Somehow got instanced and noninstanced dispatch wrong");

   BP_Prim_SpriteVertex* __restrict pVertex = (BP_Prim_SpriteVertex*)pVertexBuffer->Lock();
   uint16 * __restrict pIndex = (uint16*)CDynamicIndexBufferPool_UT::LockChunk( indexBuffer );

#if MGS_VERSION == 3
   SPrimShape const * pPrimShape = (SPrimShape const *)pPrim->BP_primShape;

   if( gPrim_OptimizeParticleShape && pPrimShape )
   {
      for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
      {
         FVECTOR const* __restrict pos = packet->pos_addr;
         DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;

         packet->BP_startIndex = indexCount;

         SParticleVert const * pVerts = pPrimShape->mpVerts;

         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );
            uint32 const rgba = BP_PACK_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );

            for( int k = 0; k < spriteVertexCount; ++k )
            {
               pVertex->x = pos->vx;
               pVertex->y = pos->vy;
               pVertex->z = pos->vz;

               pVertex->dx = (pVerts[k].rsprtX * uvrgbwh->w - pVerts[k].rsprtY * uvrgbwh->h) >> 16;
               pVertex->dy = (pVerts[k].rsprtX * uvrgbwh->h + pVerts[k].rsprtY * uvrgbwh->w) >> 16;

               pVertex->rgba = rgba;

               pVertex->u = uvrgbwh->u0 + (((uvrgbwh->u1 - uvrgbwh->u0) * pVerts[k].u) >> 16);
               pVertex->v = uvrgbwh->v0 + (((uvrgbwh->v1 - uvrgbwh->v0) * pVerts[k].v) >> 16);

               pVertex++;
            }

            for( int k = 0; k < (spriteVertexCount - 2); ++k )
            {
               *pIndex++ = vertexCount;
               *pIndex++ = vertexCount + k + 1;
               *pIndex++ = vertexCount + k + 2;
            }

            indexCount += spriteIndexCount;
            vertexCount += spriteVertexCount;
         }

         packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
      }
   }
   else
#endif
   {
      bool const cullNoAlpha = gPrim_CullAlpha && gGS_SrcAlphaBlend;

      for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
      {
         FVECTOR const* __restrict pos = packet->pos_addr;
         DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;

         packet->BP_startIndex = indexCount;

         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (cullNoAlpha && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 const currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex[0].x, currentPos );
               bp_neon_store_vec3( &pVertex[1].x, currentPos );
               bp_neon_store_vec3( &pVertex[2].x, currentPos );
               bp_neon_store_vec3( &pVertex[3].x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
               pVertex[0].rgba = pVertex[1].rgba = pVertex[2].rgba = pVertex[3].rgba = rgba;
            }

            {
               int16 const w = uvrgbwh->w;
               int16 const h = uvrgbwh->h;

               pVertex[0].dx = -w;
               pVertex[1].dx =  h;
               pVertex[2].dx =  w;
               pVertex[3].dx = -h;

               pVertex[0].dy = -h;
               pVertex[1].dy = -w;
               pVertex[2].dy =  h;
               pVertex[3].dy =  w;
            }

            {
               int16 const u0 = uvrgbwh->u0;
               int16 const u1 = uvrgbwh->u1;
               int16 const v0 = uvrgbwh->v0;
               int16 const v1 = uvrgbwh->v1;

               pVertex[0].u = u0;
               pVertex[1].u = u1;
               pVertex[2].u = u1;
               pVertex[3].u = u0;

               pVertex[0].v = v0;
               pVertex[1].v = v0;
               pVertex[2].v = v1;
               pVertex[3].v = v1;
            }

            pIndex[0] = vertexCount;
            pIndex[1] = vertexCount + 1;
            pIndex[2] = vertexCount + 2;
            pIndex[3] = vertexCount;
            pIndex[4] = vertexCount + 2;
            pIndex[5] = vertexCount + 3;

            pIndex += 6;
            pVertex += 4;

            indexCount += 6;
            vertexCount += 4;
         }

         packet->BP_indexCount = indexCount - packet->BP_startIndex;
         BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
      }
   }

   CDynamicIndexBufferPool_UT::UnlockChunk( indexBuffer );
   pVertexBuffer->Unlock();
   return vertexCount;
}

//----------------------------------------------------------------------------

#if BP_VITA
#if MGS_VERSION == 3
int BP_Prim_RenderRSprtInstanced_Mask( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
   int vertexCount = 0;
   int indexCount = 0;

   BP_Prim_SpriteVertexInstanced_Mask* __restrict pVertex = (BP_Prim_SpriteVertexInstanced_Mask*)pVertexBuffer->Lock();

   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR const* __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh2 = (DG_PRIM_UVRGBWH*)packet->child_packet->uvrgb_addr;

      packet->BP_startIndex = indexCount;

      {
         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (gPrim_CullAlpha && gGS_SrcAlphaBlend && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex->x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh2->r, uvrgbwh2->g, uvrgbwh2->b, uvrgbwh->a );
               pVertex->rgba = rgba;
            }

            {
               int16 w = uvrgbwh->w;
               int16 h = uvrgbwh->h;

               pVertex->dx = w;
               pVertex->dy = h;
            }

            {
               int16 u0 = uvrgbwh->u0;
               int16 u1 = uvrgbwh->u1;
               int16 v0 = uvrgbwh->v0;
               int16 v1 = uvrgbwh->v1;

               pVertex->u0 = u0;
               pVertex->u1 = u1;
               pVertex->v0 = v0;
               pVertex->v1 = v1;
            }

            {
               int16 u0 = uvrgbwh2->u0;
               int16 u1 = uvrgbwh2->u1;
               int16 v0 = uvrgbwh2->v0;
               int16 v1 = uvrgbwh2->v1;

               pVertex->mu0 = u0;
               pVertex->mu1 = u1;
               pVertex->mv0 = v0;
               pVertex->mv1 = v1;
            }

            pVertex ++;

            indexCount += spriteIndexCount;
            vertexCount ++;
         }
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   pVertexBuffer->Unlock();
   return vertexCount;
}
#endif

//----------------------------------------------------------------------------

int BP_Prim_RenderRSprtInstanced( CDynamicVertexBufferPoolChunk_UT* __restrict pVertexBuffer, CDynamicIndexBufferPool_UT::TChunkArg indexBuffer, DG_PRIM* __restrict pPrim, DG_PRIM_PACKET* __restrict packet )
{
#if MGS_VERSION == 3
   if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
      return BP_Prim_RenderRSprtInstanced_Mask( pVertexBuffer, indexBuffer, pPrim, packet);
#endif

   int vertexCount = 0;
   int indexCount = 0;

   int const spriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
   int const spriteIndexCount = (spriteVertexCount - 2) * 3;

   BP_Prim_SpriteVertexInstanced* __restrict pVertex = (BP_Prim_SpriteVertexInstanced*)pVertexBuffer->Lock();

   for ( int j = 0 ; j < pPrim->n_prims; j++, packet++ )
   {
      FVECTOR const* __restrict pos = packet->pos_addr;
      DG_PRIM_UVRGBWH const * __restrict uvrgbwh = (DG_PRIM_UVRGBWH*)packet->uvrgb_addr;

      packet->BP_startIndex = indexCount;

      {
         for( int i = 0 ; i < packet->n_verts ; i++, pos++, uvrgbwh++ )
         {
            BPE_DCACHE_PREFETCH( pos );
            BPE_DCACHE_PREFETCH( uvrgbwh );

            if (gPrim_CullAlpha && gGS_SrcAlphaBlend && !uvrgbwh->a)
               continue;

            {
               TBP_Neon_Vec3 currentPos = bp_neon_load_vec3( &pos->vx );
               bp_neon_store_vec3( &pVertex->x, currentPos );
            }

            {
               uint32 const rgba = BP_PACK_COLOR( uvrgbwh->r, uvrgbwh->g, uvrgbwh->b, uvrgbwh->a );
               pVertex->rgba = rgba;
            }

            {
               int16 w = uvrgbwh->w;
               int16 h = uvrgbwh->h;

               pVertex->dx = w;
               pVertex->dy = h;
            }

            {
               int16 u0 = uvrgbwh->u0;
               int16 u1 = uvrgbwh->u1;
               int16 v0 = uvrgbwh->v0;
               int16 v1 = uvrgbwh->v1;

               pVertex->u0 = u0;
               pVertex->u1 = u1;
               pVertex->v0 = v0;
               pVertex->v1 = v1;
            }

            pVertex ++;

            indexCount += spriteIndexCount;
            vertexCount ++;
         }
      }

      packet->BP_indexCount = indexCount - packet->BP_startIndex;
      BPE_ASSERT(packet->BP_indexCount >= 0 && packet->BP_indexCount < 65536, "Something is wrong");
   }

   pVertexBuffer->Unlock();
   return vertexCount;
}
#endif

//----------------------------------------------------------------------------

#if BP_PRIM_DEBUGGING
extern "C" int gDumpPrims = 0;
extern "C" int gDumpingPrims = 0;
extern "C" int gShowPrims = 0;
extern "C" int gClearPrims = 0;
#if BP_VITA
#include <string.h>
int BP_Prim_ProfCtrl(int d);
extern int gDoRazorCaptureReallySoon;
#endif
#endif

void BP_Prim_StartPass(char* pData)
{
   SBP_Prim_StartPass* pPacket = (SBP_Prim_StartPass*)pData;

   BP_GS_SetTest(pPacket->testValue);
}

//----------------------------------------------------------------------------

#if MGS_VERSION == 3
#define ABCD(a, b, c, d) (a | b << 2 | c << 4 | d << 6)

void DG_AS_AnalyzePrim(DG_PRIM *pPrim)
{
   // peephole optimizer to catch the case of alpha-only write followed by dstalpha additive or blend
   union
   {
      unsigned char data;
      struct {
         unsigned int a : 2;
         unsigned int b : 2;
         unsigned int c : 2;
         unsigned int d : 2;
      };
   } p, c;

   // check to see if we've already looked at this prim
   if (!pPrim->BP_optflag)
   {
      if (pPrim->child && ((pPrim->type == DG_PRIM_POLY) || (pPrim->type == DG_PRIM_CULLPOLY)
#if BP_VITA            
                           || (pPrim->type == DG_PRIM_SPRT) || (pPrim->type == DG_PRIM_RSPRT)
#endif
             ))
      {
         p.data = pPrim->tex_trans.alpha.data & 0xFF;
         c.data = pPrim->child->tex_trans.alpha.data & 0xFF;

         unsigned char p_abcd = pPrim->tex_trans.alpha.data & 0xFF;
         unsigned char c_abcd = pPrim->child->tex_trans.alpha.data & 0xFF;
         unsigned char mask0 = ABCD(3, 3, 0, 3);
         unsigned char mask1 = ABCD(3, 0, 3, 3);
         unsigned char add = ABCD(0, 2, 1, 1);
         unsigned char blend = ABCD(0, 1, 1, 1);

         if ((c.a == 0) && (c.c == 1) && (c.d == 1))
         {
            // child is a dst alpha add or blend, flag as alpha masked, and nop the child out
            pPrim->BP_optflag |= kPOpt_Mask;
            pPrim->child->BP_optflag |= kPOpt_Nop;

            if ((p.a != p.b) && (p.d == 1))
            {
               // parent is not an alpha-only pass, so tell it to write color as well
               pPrim->BP_optflag |= kPOpt_Dup;
            }
         }
      }
   }
   // analyze the entire chain
   if (pPrim->child)
      DG_AS_AnalyzePrim(pPrim->child);
}
#endif

//----------------------------------------------------------------------------

int BP_Prim_Render_MakeState( struct SPrimRenderState *pState, void *pData )
{
   // This function allocates the vertex and index buffers necessary to draw the prim.
   // This will go into the "state" variable that will be passed into the functions that 
   // use the vertex and index buffers
   DG_PRIM* pPrim = (DG_PRIM*)pData;

#if MGS_VERSION == 3
   if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Nop))
      return 0;
#endif

   gNumPrims++;

   CDynamicIndexBufferPool_UT::TChunk pIndexBuffer = CDynamicIndexBufferPool_UT::NullChunk();
   CDynamicVertexBufferPoolChunk_UT* pVertexBuffer = NULL;

   // Determine how many verts we need to reserve by iterating over the packets and adding the per packet counts together
   int maxPrimVertexCount = 0;
   {
      DG_PRIM_PACKET* pPrimPacket = pPrim->packet[ pPrim->buffer_clock ];
      for ( int j = 0 ; j < pPrim->n_prims; j++, pPrimPacket++ )
         maxPrimVertexCount += pPrimPacket->n_verts;

      if( gPrim_ShowVertexCountIssues )
      {
         if( maxPrimVertexCount != (pPrim->n_prims * pPrim->packet_verts))
            BP_DebugText_Print("Prim vertex counts disagree: counted: %d estimate: %d\n", maxPrimVertexCount, (pPrim->n_prims * pPrim->packet_verts));
      }
   }

   gNumPrimInputVerts += maxPrimVertexCount;
   pState->mInstanced = 0;

   // Set up index/vertex buffer
   {
      switch(pPrim->type)
      {
      case DG_PRIM_POLY:
         {
            if( gPrim_EnablePoly )
            {
#if MGS_VERSION == 3
               if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
                  pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_Vertex_Mask) * maxPrimVertexCount);
               else
#endif
                  pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_Vertex) * maxPrimVertexCount);
               pIndexBuffer = gpRenderBackend->GetIndexBufferPool_UT()->AllocChunk(maxPrimVertexCount * 2);
            }
         }
         break;

      case DG_PRIM_CULLPOLY:
         {
            if( gPrim_EnableCullPoly )
            {
#if MGS_VERSION == 3
               if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
                  pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_Vertex_Mask) * maxPrimVertexCount);
               else
#endif
                  pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_Vertex) * maxPrimVertexCount);
               pIndexBuffer = gpRenderBackend->GetIndexBufferPool_UT()->AllocChunk(maxPrimVertexCount * 2);
            }
         }
         break;

      case DG_PRIM_LINE:
         {
            if( gPrim_EnableLine )
            {
               pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_Vertex) * maxPrimVertexCount);
               pIndexBuffer = gpRenderBackend->GetIndexBufferPool_UT()->AllocChunk(maxPrimVertexCount * 2);
            }
         }
         break;

      case DG_PRIM_SPRT:
         {
            if( gPrim_EnableSprt )
            {
#if BP_VITA
               if (gPrim_EnableInstancing)
               {
                  pState->mInstanced = 1;
#  if MGS_VERSION == 3
                  if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
                  {
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertexInstanced_Mask) * maxPrimVertexCount);
                  }
                  else
#  endif
                  {
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertexInstanced) * maxPrimVertexCount);
                  }

                  CStaticIndexBuffer *indexBuff;
#  if MGS_VERSION == 3
                  if (gPrim_OptimizeParticleShape && pPrim->BP_primShape)
                  {
                     SPrimShape *primShape = (SPrimShape *)pPrim->BP_primShape;
                     indexBuff = gOptimizedSpriteInstanceIndices[primShape->mNumVerts];
                  }
                  else
#endif
                  {
                     indexBuff = gSpriteInstanceIndices;
                  }
                  pIndexBuffer = CIndexBufferChunk( indexBuff, (uint16 *) indexBuff->Lock(), indexBuff->mCount );
               }
               else
#endif
               {
                  int const kSpriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
                  int const kSpriteTriangleCount = (kSpriteVertexCount - 2) * 3;
                  
#if MGS_VERSION == 3
                  if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
                  {
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertex_Mask) * maxPrimVertexCount * kSpriteVertexCount);
                  }
                  else
#endif
                  {
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertex) * maxPrimVertexCount * kSpriteVertexCount);
                  }
                  pIndexBuffer = gpRenderBackend->GetIndexBufferPool_UT()->AllocChunk(maxPrimVertexCount * kSpriteTriangleCount);
               }
            }
         }
         break;

      case DG_PRIM_RSPRT:
         {
            if( gPrim_EnableRSprt )
            {
#if BP_VITA
               if (gPrim_EnableInstancing)
               {
                  pState->mInstanced = 1;
#if MGS_VERSION == 3
                  if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
                  {
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertexInstanced_Mask) * maxPrimVertexCount);
                  }
                  else
#endif
                  {
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertexInstanced) * maxPrimVertexCount);
                  }
                  CStaticIndexBuffer *indexBuff;
#if MGS_VERSION == 3
                  if (gPrim_OptimizeParticleShape && pPrim->BP_primShape)
                  {
                     SPrimShape *primShape = (SPrimShape *)pPrim->BP_primShape;
                     indexBuff = gOptimizedSpriteInstanceIndices[primShape->mNumVerts];
                  }
                  else
#endif
                  {
                     indexBuff = gSpriteInstanceIndices;
                  }
                  pIndexBuffer = CIndexBufferChunk( indexBuff, (uint16 *) indexBuff->Lock(), indexBuff->mCount );
               }
               else
#endif
               {
                  int const kSpriteVertexCount = BP_Prim_GetSpriteVertexCount(pPrim);
                  int const kSpriteTriangleCount = (kSpriteVertexCount - 2) * 3;
#if MGS_VERSION == 3
                  if (gPrim_EnableMerging && (pPrim->BP_optflag & kPOpt_Mask))
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertex_Mask) * maxPrimVertexCount * kSpriteVertexCount);
                  else
#endif
                     pVertexBuffer = gpRenderBackend->GetVertexBufferPool_UT()->AllocChunk(sizeof(BP_Prim_SpriteVertex) * maxPrimVertexCount * kSpriteVertexCount);
                  pIndexBuffer = gpRenderBackend->GetIndexBufferPool_UT()->AllocChunk(maxPrimVertexCount * kSpriteTriangleCount);
               }
            }
         }
         break;
      }
   }

   pState->mpData = pData;
   pState->mpVertexBuffer = pVertexBuffer;
   pState->mpIndexBuffer = pIndexBuffer;
   return 1;
}

//----------------------------------------------------------------------------

#if BP_PRIM_DEBUGGING
extern "C" char *BP_SplatBasename(char *buf, char *fn);
#endif

void BP_Prim_Render_Execute( struct SPrimRenderState const *pState )
{
   DG_PRIM* pPrim = (DG_PRIM*)pState->mpData;

#if BP_PRIM_DEBUGGING
   {
      char meh[1024];
      char *base = meh;
      char *buf = base;
      buf += sprintf(buf, "%d p %d v ", pPrim->n_prims, pPrim->packet_verts);
      buf = BP_SplatBasename(buf, pPrim->fname);
      *buf++ = '\0';
      BP_Debug_PushCPUMarker(base);
   }
#endif

   CDynamicIndexBufferPool_UT::TChunk pIndexBuffer = pState->mpIndexBuffer;
   CDynamicVertexBufferPoolChunk_UT* pVertexBuffer = reinterpret_cast<CDynamicVertexBufferPoolChunk_UT*>( pState->mpVertexBuffer );

   int vertexCount = 0;

   if( !CDynamicIndexBufferPool_UT::IsChunkNull( pIndexBuffer ) && pVertexBuffer )
   {
      DG_PRIM_PACKET* pPrimPacket = pPrim->packet[ pPrim->buffer_clock ];

      switch(pPrim->type)
      {
      case DG_PRIM_POLY:
      case DG_PRIM_CULLPOLY:
         vertexCount = BP_Prim_RenderPoly(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
         break;
      
      case DG_PRIM_LINE:
         vertexCount = BP_Prim_RenderLine(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
         break;

      case DG_PRIM_SPRT:
#if BP_VITA
         if (pState->mInstanced)
         {
            vertexCount = BP_Prim_RenderSprtInstanced(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
         }
         else
         {
            vertexCount = BP_Prim_RenderSprt(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
         }
#else
            vertexCount = BP_Prim_RenderSprt(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
#endif
         break;

      case DG_PRIM_RSPRT:
#if BP_VITA
         if (pState->mInstanced)
         {
            vertexCount = BP_Prim_RenderRSprtInstanced(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
         }
         else
         {
            vertexCount = BP_Prim_RenderRSprt(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
         }
#else
            vertexCount = BP_Prim_RenderRSprt(pVertexBuffer, pIndexBuffer, pPrim, pPrimPacket);
#endif
         break;

      }
   }

   BPE_CTASSERT( sizeof( pPrim->BP_indexBuffer ) >= sizeof( pIndexBuffer ) );
   CDynamicIndexBufferPool_UT::CopyToInts( pPrim->BP_indexBuffer, pIndexBuffer );
   pPrim->BP_vertexBuffer = pVertexBuffer;
   pPrim->BP_vertexCount = vertexCount;
   pPrim->BP_instanced = pState->mInstanced;
   pPrim->BP_localUniformBuffer = NULL;

   gNumPrimOutputVerts += vertexCount;

#if BP_PRIM_DEBUGGING
   BP_Debug_PopCPUMarker();
#endif
}

//----------------------------------------------------------------------------

namespace
{
   struct SPrimUniforms
   {
      SPrimUniforms();

      CVector4 mFogParam;
      real32 mPersMatrix[ 16 ];
   };

   KP_CTASSERT( sizeof( SPrimUniforms ) == 16 + 16 * 4 );

   struct SPrimLocalUniforms
   {
      SPrimLocalUniforms();

      real32 mScreenMatrix[ 16 ];
   };

   KP_CTASSERT( sizeof( SPrimLocalUniforms ) == 16 * 4 );

   SPrimUniforms *gpCurrentPrimFrameUniforms;

   float *gpCurrentPrimFrameFogColor;
}

//----------------------------------------------------------------------------

void BP_Prim_InitPacket(char* pData)
{
   SBP_Prim_InitPacket* pPacket = (SBP_Prim_InitPacket*)pData;

#if BP_VITA
   {
      SPrimUniforms *pPrimUniforms = gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<SPrimUniforms>( 0 );

      BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->pers, pPrimUniforms->mPersMatrix );
      pPrimUniforms->mFogParam.mX = pPacket->fogParam.vx;
      pPrimUniforms->mFogParam.mY = pPacket->fogParam.vy;
      pPrimUniforms->mFogParam.mZ = pPacket->fogParam.vz;
      pPrimUniforms->mFogParam.mW = pPacket->fogParam.vw;

      gpCurrentPrimFrameUniforms = pPrimUniforms;

      extern CVector4 gBP_FogColor;
      CVector4 *pFogColor = gpRenderBackend->AllocOneFrameFragmentUniformBufferTyped<CVector4>( 4 );
      *pFogColor = gBP_FogColor;
      gpCurrentPrimFrameFogColor = (float*)pFogColor;
   }
#else
   real32 vsMatrix[16];

   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->pers, vsMatrix);
   gpRenderBackend->SetVertexRegisters(PrimShader::kReg_Pers, 4, (CVector4 const*)vsMatrix);

   gpRenderBackend->SetVertexRegisters(kSReg_FogParam, 1, (CVector4 const*)&pPacket->fogParam);

   if( gIsFirstPrimForViewport )
   {
      gIsFirstPrimForViewport = 0;

      extern CVector4 gBP_FogColor;

      CShaderParameterBuffer buffer(gpFogColorParameterBuffer);
      buffer.AddParameter(PrimShader::gFogColorParameter, gBP_FogColor);
   }
#endif
}

//----------------------------------------------------------------------------

void BP_Prim_LocalParam(char* pData)
{
   SBP_Prim_LocalParam* pPacket = (SBP_Prim_LocalParam*)pData;

#if BP_PRIM_DEBUGGING
   if (gDumpingPrims)
   {
      DG_PRIM *prim = (DG_PRIM*)pPacket->prim;
      printf("%s\n", prim->fname);
   }
#endif

#if BP_VITA
   {
      SPrimLocalUniforms *pPrimUniforms = gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<SPrimLocalUniforms>( 1 );

      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->screen, pPrimUniforms->mScreenMatrix );
   }
   gpRenderBackend->SetStaticVertexUniformBuffer(0, gpCurrentPrimFrameUniforms);

   gPrimClone = pPacket->clone;
   gpLastTex = NULL;
   gpLastMaskTex = NULL;
   gLastTransClamp = 0;
   gLastMaskTransClamp = 0;

#else
   real32 vsMatrix[16];
   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->screen, vsMatrix);
   gpRenderBackend->SetVertexRegisters(PrimShader::kReg_Screen, 4, (CVector4 const*)vsMatrix);
#endif
}

//----------------------------------------------------------------------------

#if BP_PRIM_DEBUGGING
struct SPrimProfile
{
   const char *fname;
   int npack;
   int nindex;
   int enable;
};

SPrimProfile prim_prof_list[2048];
SPrimProfile *prim_prof_end = prim_prof_list;
SPrimProfile *prim_prof_ptr = prim_prof_list;

int BP_Prim_ProfCtrl(int d)
{
   if (!d)
   {
      prim_prof_ptr->enable = !prim_prof_ptr->enable;
   }
   else if (d < 0)
   {
      if (prim_prof_ptr > prim_prof_list)
         prim_prof_ptr--;
   }
   else if (d > 0)
   {
      if (prim_prof_ptr < prim_prof_end-1)
         prim_prof_ptr++;
   }
   return 0;
}

void BP_Prim_ResetProfile()
{
   prim_prof_end = prim_prof_list;
   prim_prof_ptr = prim_prof_list;
}

void BP_Prim_ClearProfile()
{
   SPrimProfile *p = prim_prof_list;

   while (p < prim_prof_end)
   {
      p->npack = 0;
      p->nindex = 0;
      p++;
   }
}

SPrimProfile *BP_Prim_AddProfile(SBP_PrimPacket_Render *pack)
{
   SPrimProfile *p = prim_prof_list;

   while (p < prim_prof_end)
   {
      if (p->fname == pack->fname)
      {
         p->npack++;
         p->nindex += pack->BP_indexCount;
         return p;
      }
      p++;
   }
   if (p == prim_prof_end)
   {
      p->fname = pack->fname;
      p->npack = 1;
      p->nindex = pack->BP_indexCount;
      p->enable = 1;
      prim_prof_end++;
   }
   if (prim_prof_end > &prim_prof_list[2047])
      prim_prof_end = &prim_prof_list[2047];
   return p;
}

void BP_Prim_DumpProfile()
{
   int n = 0;
   SPrimProfile *p = prim_prof_list;
   char *str;
   while (p < prim_prof_end && n++ < 32)
   {
      str = strrchr((char*)p->fname, '/');
      if (!str)
         str = strrchr((char*)p->fname, '\\');
      if (!str)
         str = "_UNKNOWN";
      str++;
      BP_DebugText_Print("%c%c%5d %6d  %20s", (p == prim_prof_ptr) ? '*' : ' ', p->enable ? ' ' : 'X', p->npack, p->nindex, str);
      p++;
   }
}
#endif

//----------------------------------------------------------------------------

#if MGS_VERSION == 3
#define INDEXCOUNT pPacket->BP_indexCountPerInstance
#elif MGS_VERSION == 2
#define INDEXCOUNT 6
#else
#error Unknown MGS version
#endif

void BP_Prim_RenderPacket(char* pData)
{
   SBP_PrimPacket_Render* pPacket = (SBP_PrimPacket_Render*)pData;
   if( pPacket->BP_indexCount <= 0 )
   {
      return;
   }

   CDynamicVertexBufferPoolChunk_UT* pVertexBuffer = (CDynamicVertexBufferPoolChunk_UT*)pPacket->BP_vertexBuffer;
   CDynamicIndexBufferPool_UT::TChunk const pIndexBuffer = CDynamicIndexBufferPool_UT::ChunkFromInts( pPacket->BP_indexBuffer );

   if( !pVertexBuffer || CDynamicIndexBufferPool_UT::IsChunkNull( pIndexBuffer ) )
      return;

#if BP_VITA
#if MGS_VERSION == 3
   if( (pPacket->BP_fogBuffer & DG_PRIM_BLACKFOG) )
   {
      gpRenderBackend->SetStaticFragmentUniformBuffer(4, gpBlackUniformBuffer);
   }
   else
#endif
   {
      gpRenderBackend->SetStaticFragmentUniformBuffer(4, gpCurrentPrimFrameFogColor);
   }
#else
#if MGS_VERSION == 3
   if( (pPacket->BP_fogBuffer & DG_PRIM_BLACKFOG) )
   {
      gpCurrentFogParameterBuffer = gpBlackParameterBuffer;
   }
   else
#endif
   {
      gpCurrentFogParameterBuffer = gpFogColorParameterBuffer;
   }
#endif

   gNumPrimPackets++;

#if BP_PRIM_DEBUGGING
   SPrimProfile *prof = BP_Prim_AddProfile(pPacket);

   if (!prof->enable)
      return;
#endif

#if BP_ENABLE_DEBUG_PRIM_GPU_MARKER
   BPE_ADD_SCOPED_GPU_PROFILE_MARKER(pPacket->fname ? pPacket->fname : "(NULL)");
#endif

   //DG_PRIM_SHADE
   //DG_PRIM_ANTIALIASING

   int const isFog = (pPacket->flag & DG_PRIM_FOG) ? 1 : 0;

   int isSprite = 0;

#if MGS_VERSION == 3
   int optFlag = pPacket->BP_optflag;
#else
   int optFlag = 0;
#endif

#if BP_PRIM_DEBUGGING
   if (gDumpingPrims)
   {
      printf("%d  v(%5d)  i(%5d) %.8x  %s\n",
             pPacket->type, pPacket->BP_vertexCount, pPacket->BP_indexCount, pPacket->BP_tex, pPacket->fname);
   }
#endif

#if !BP_VITA
   CVertexData* pVertexData;
   switch(pPacket->type)
   {
   case DG_PRIM_SPRT:
   case DG_PRIM_RSPRT:
      {
#if BP_VITA
         if (pPacket->BP_instanced)
         {
            if (optFlag & kPOpt_Mask)
            {
               pVertexData = &gSpriteMaskPrimInstanced_VertexData;
               pVertexData->ClearVertexBuffers();
               void *buf = pVertexBuffer->GetRenderMemory();
               buf = (void*)((unsigned char*)buf + sizeof( BP_Prim_SpriteVertexInstanced_Mask ) * (pPacket->BP_startIndex/6));
               int bidx = pVertexData->AddBuffer_Unsafe(buf, sizeof( BP_Prim_SpriteVertexInstanced_Mask ) );
               pVertexData->SetInstanced(true, bidx);
               pVertexData->AddBuffer( pPacket->type == DG_PRIM_RSPRT ? gSpriteInstanceVerticesRSprt : gSpriteInstanceVerticesSprt, sizeof( FVECTOR ) * 2 );
            }
            else
            {
               pVertexData = &gSpritePrimInstanced_VertexData;
               pVertexData->ClearVertexBuffers();
               //pVertexData->AddBuffer( pVertexBuffer, sizeof( BP_Prim_SpriteVertexInstanced ) );
               void *buf = pVertexBuffer->GetRenderMemory();
               buf = (void*)((unsigned char*)buf + sizeof( BP_Prim_SpriteVertexInstanced ) * (pPacket->BP_startIndex/6));
               int bidx = pVertexData->AddBuffer_Unsafe(buf, sizeof( BP_Prim_SpriteVertexInstanced ) );
               pVertexData->SetInstanced(true, bidx);
               pVertexData->AddBuffer( pPacket->type == DG_PRIM_RSPRT ? gSpriteInstanceVerticesRSprt : gSpriteInstanceVerticesSprt, sizeof( FVECTOR ) * 2 );
            }
         }
         else
#endif
         {
#if BP_VITA
            if (optFlag & kPOpt_Mask)
            {
               pVertexData = &gSpriteMaskPrim_VertexData;
               pVertexData->ClearVertexBuffers();
               pVertexData->AddBuffer( pVertexBuffer, sizeof( BP_Prim_SpriteVertex_Mask ) );
            }
            else
#endif
            {
               pVertexData = &gSpritePrim_VertexData;
               pVertexData->ClearVertexBuffers();
               pVertexData->AddBuffer( pVertexBuffer, sizeof( BP_Prim_SpriteVertex ) );
            }
         }
         isSprite = 1;
      }
      break;

   default:
      if (optFlag & kPOpt_Mask)
      {
         pVertexData = &gMaskPrim_VertexData;
         pVertexData->ClearVertexBuffers();
         pVertexData->AddBuffer( pVertexBuffer, sizeof( BP_Prim_Vertex_Mask ) );
      }
      else
      {
         pVertexData = &gDefaultPrim_VertexData;
         pVertexData->ClearVertexBuffers();
         pVertexData->AddBuffer( pVertexBuffer, sizeof( BP_Prim_Vertex ) );
      }
      break;
   }

   CShaderVertexDataBinding binding;
#if BP_VITA
   if (pPacket->BP_instanced)
   {
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_TexCoord1);
      binding.Set(kVDU_TexCoord2, kVDS_Color0);
      binding.Set(kVDU_TexCoord3, kVDS_TexCoord2);
      binding.Set(kVDU_TexCoord4, kVDS_TexCoord3);
      if (optFlag & kPOpt_Mask)
         binding.Set(kVDU_TexCoord5, kVDS_TexCoord4);
   }
   else
#endif
   {
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);
      if (optFlag & kPOpt_Mask)
         binding.Set(kVDU_TexCoord2, kVDS_TexCoord1);
   }
#else
   NVTAState::SCachedVertexData* pVertexData;
   switch(pPacket->type)
   {
   case DG_PRIM_SPRT:
   case DG_PRIM_RSPRT:
      {
         if (optFlag & kPOpt_Mask)
         {
            if (pPacket->BP_instanced)
            {
               pVertexData = gpSpriteMaskPrimInstanced_CVD;
               void *buf = pVertexBuffer->GetRenderMemory();
               buf = (void*)((unsigned char*)buf + sizeof( BP_Prim_SpriteVertexInstanced_Mask ) * (pPacket->BP_startIndex/INDEXCOUNT));
               pVertexData->SetBuffer_Unsafe(0, buf);
               CStaticVertexBuffer *vertexBuff;
               if (pPacket->type == DG_PRIM_SPRT)
               {
#if MGS_VERSION == 3
                  if (pPacket->BP_optimizedPrimShapeIndex >= 0)
                  {
                     vertexBuff = gOptimizedSpriteInstanceVerticesSprt[pPacket->BP_optimizedPrimShapeIndex];
                  }
                  else
#endif
                  {
                     vertexBuff = gSpriteInstanceVerticesSprt;
                  }
               }
               else
               {
#if MGS_VERSION == 3
                  if (pPacket->BP_optimizedPrimShapeIndex >= 0)
                  {
                     vertexBuff = gOptimizedSpriteInstanceVerticesRSprt[pPacket->BP_optimizedPrimShapeIndex];
                  }
                  else
#endif
                  {
                     vertexBuff =  gSpriteInstanceVerticesRSprt;
                  }
               }
               pVertexData->SetBuffer(1, vertexBuff);
            }
            else
            {
               pVertexData = gpSpriteMaskPrim_CVD;
               pVertexData->SetBuffer( 0, pVertexBuffer );
            }
         }
         else
         {
            if (pPacket->BP_instanced)
            {
               pVertexData = gpSpritePrimInstanced_CVD;
               void *buf = pVertexBuffer->GetRenderMemory();
               buf = (void*)((unsigned char*)buf + sizeof( BP_Prim_SpriteVertexInstanced ) * (pPacket->BP_startIndex/INDEXCOUNT));
               pVertexData->SetBuffer_Unsafe(0, buf);
               CStaticVertexBuffer *vertexBuff;
               if (pPacket->type == DG_PRIM_SPRT)
               {
#if MGS_VERSION == 3
                  if (pPacket->BP_optimizedPrimShapeIndex >= 0)
                  {
                     vertexBuff = gOptimizedSpriteInstanceVerticesSprt[pPacket->BP_optimizedPrimShapeIndex];
                  }
                  else
#endif
                  {
                     vertexBuff = gSpriteInstanceVerticesSprt;
                  }
               }
               else
               {
#if MGS_VERSION == 3
                  if (pPacket->BP_optimizedPrimShapeIndex >= 0)
                  {
                     vertexBuff = gOptimizedSpriteInstanceVerticesRSprt[pPacket->BP_optimizedPrimShapeIndex];
                  }
                  else
#endif
                  {
                     vertexBuff =  gSpriteInstanceVerticesRSprt;
                  }
               }
               pVertexData->SetBuffer(1, vertexBuff);
            }
            else
            {
               pVertexData = gpSpritePrim_CVD;
               pVertexData->SetBuffer( 0, pVertexBuffer );
            }
         }
         isSprite = 1;
      }
      break;

   default:
      if (optFlag & kPOpt_Mask)
      {
         pVertexData = gpMaskPrim_CVD;
         pVertexData->SetBuffer( 0, pVertexBuffer );
      }
      else
      {
         pVertexData = gpDefaultPrim_CVD;
         pVertexData->SetBuffer( 0, pVertexBuffer );
      }
      break;
   }
#endif

   //NOTE: Antialiasing  implies alpha blending.
   if( pPacket->flag & (DG_PRIM_ALPHA|DG_PRIM_ANTIALIASING) )
   {
      BP_GS_SetAlpha(pPacket->tex_trans_alpha_data);
   }
   else
   {
      BP_GS_SetAlpha(0);
   }

   int textureType = 0;
   
   if( pPacket->flag & DG_PRIM_TEX ) 
   {
      textureType = (pPacket->flag & DG_PRIM_FBTEX) ? 2 : 1;

      DG_TEX_BP* pTex = (DG_TEX_BP*)pPacket->BP_tex;

      if( pTex )
      {
         // texture side render target mode override! (alpha should be halved!)
         if( pTex->BP_flag & DG_TEXFLAG_RENDERTARGET )
            textureType = 2;

         real32 const uScale = 1.0f / pTex->u_scale;
         real32 const vScale = 1.0f / pTex->v_scale;
         real32 const uOffset = uScale * -pTex->u_offset;
         real32 const vOffset = vScale * -pTex->v_offset;

         CVector4 const uvScaleOffset(uScale, vScale, uOffset, vOffset);
#if BP_VITA
         *gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<CVector4>( 2 ) = uvScaleOffset;
#else
         gpRenderBackend->SetVertexRegisters(PrimShader::kReg_UvScaleOffset, 1, &uvScaleOffset);
#endif

#if BP_VITA
         if ((BP_DG_GetTexture(pTex) != gpLastTex) || (gLastTransClamp != pPacket->tex_trans_clamp_data))
         {
            BP_SetTextureNoClamp(pTex, 0);
            BP_GS_SetClamp(0, pPacket->tex_trans_clamp_data);
            
            gpLastTex = BP_DG_GetTexture(pTex);
            gLastTransClamp = pPacket->tex_trans_clamp_data;
         }
#else
         BP_SetTextureNoClamp(pTex, 0);
         BP_GS_SetClamp(0, pPacket->tex_trans_clamp_data);
#endif
      }
#if MGS_VERSION == 3
      if (optFlag & kPOpt_Mask)
      {
         DG_TEX_BP* pMaskTex = (DG_TEX_BP*)pPacket->BP_masktex;

         real32 const uScale = 1.0f / pMaskTex->u_scale;
         real32 const vScale = 1.0f / pMaskTex->v_scale;
         real32 const uOffset = uScale * -pMaskTex->u_offset;
         real32 const vOffset = vScale * -pMaskTex->v_offset;

         CVector4 const uvScaleOffset(uScale, vScale, uOffset, vOffset);
#if BP_VITA
         *gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<CVector4>( 3 ) = uvScaleOffset;
#else
         gpRenderBackend->SetVertexRegisters(PrimShader::kReg_UvScaleOffset1, 1, &uvScaleOffset);
#endif

#if BP_VITA
         if ((BP_DG_GetTexture(pMaskTex) != gpLastMaskTex) || (pPacket->mask_trans_clamp_data != gLastMaskTransClamp))
         {
            BP_SetTextureNoClamp(pMaskTex, 1);
            BP_GS_SetClamp(1, pPacket->mask_trans_clamp_data);
            
            gpLastMaskTex = BP_DG_GetTexture(pMaskTex);
            gLastMaskTransClamp = pPacket->mask_trans_clamp_data;
         }
#else
         BP_SetTextureNoClamp(pMaskTex, 1);
         BP_GS_SetClamp(1, pPacket->mask_trans_clamp_data);
#endif
      }
#endif
   }

#if MGS_VERSION == 2
   if(pPacket->flag & (DG_PRIM2_CW|DG_PRIM2_CCW) )
   {
      if ( pPacket->flag & DG_PRIM2_CW )
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_CCW);
      else 
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_CW);
   }
   else
#endif
   {
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
   }

   gpRenderBackend->SetDepthWriteEnabled(false);

#if BP_VITA
   gpRenderBackend->SetVertexData(pVertexData);
#else
   gpRenderBackend->SetVertexData(binding, *pVertexData, 0, 0);
#endif
   gpRenderBackend->ForceVertexDataRebind();
   gpRenderBackend->SetIndexData(pIndexBuffer);

   if( !pPacket->BP_forceDisableAlphaTest )
   {
      BP_GS_SetupAlphaTestSinglePass(pPacket->BP_forceZWrite != 0);
   }
   else
   {
      BP_GS_SetupAlphaTestSinglePass_ForceDisableAlphaTest(pPacket->BP_forceZWrite != 0);
   }

   CMeshChunk::EPrimitive primType; 
   switch(pPacket->type)
   {
   case DG_PRIM_POLY:
   case DG_PRIM_CULLPOLY: // fallthru
      primType = CMeshChunk::kPrimitive_TriangleStrip;
      break;

   case DG_PRIM_LINE:
      primType = CMeshChunk::kPrimitive_LineList;
      break;

   case DG_PRIM_SPRT: // fallthru
   case DG_PRIM_RSPRT:
      primType = CMeshChunk::kPrimitive_TriangleList;
      break;
   default:
      BPE_ASSERTA( "Bad prim type" );
      return;
   }

#if MGS_VERSION==3
   if( BPE_BRANCH_HINT_FALSE(pPacket->flag & DG_PRIM_NO_DEPTH_TEST) )
      gpRenderBackend->SetDepthCompareEnabled(false);
#endif

   BP_Prim_RenderOffscreenBegin();

#if MGS_VERSION == 3
   if (optFlag & kPOpt_Dup)
   {
      // render with textureType and the base alpha once, then reset to texturetype 3 and set the mask alpha
#if BP_VITA
      if (pPacket->BP_instanced)
      {
         BP_BeginShader(PrimShader::gpSpriteShader[textureType][isFog][gGS_ForceRGBWhite][gPrim_PS2ColorClamp], gpCurrentFogParameterBuffer, gParticleParameterBufferSize);
         gpRenderBackend->RenderPrimitivesInstanced(primType, 0, pPacket->BP_indexCount, INDEXCOUNT);
      }
      else
#endif
      {
         BP_BeginShader(PrimShader::gpShader[textureType][isSprite][isFog][gGS_ForceRGBWhite][gPrim_PS2ColorClamp], gpCurrentFogParameterBuffer, gParticleParameterBufferSize);
         gpRenderBackend->RenderPrimitives(primType, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
                                           0, pPacket->BP_vertexCount, 
#endif
                                           pPacket->BP_startIndex, pPacket->BP_indexCount);
      }
   }
#endif

#if MGS_VERSION == 3
   if (optFlag & kPOpt_Mask)
   {
      // otherwise just switch the alpha and texturetype
      textureType = 3;
      BP_GS_SetAlpha(pPacket->mask_trans_alpha_data);
   }
#endif

#if BP_VITA
   if (pPacket->BP_instanced)
      BP_BeginShader(PrimShader::gpSpriteShader[textureType][isFog][gGS_ForceRGBWhite][gPrim_PS2ColorClamp], gpCurrentFogParameterBuffer, gParticleParameterBufferSize);
   else
#endif
      BP_BeginShader(PrimShader::gpShader[textureType][isSprite][isFog][gGS_ForceRGBWhite][gPrim_PS2ColorClamp], gpCurrentFogParameterBuffer, gParticleParameterBufferSize);

#if BP_VITA
   if (pPacket->BP_instanced)
   {
      gpRenderBackend->RenderPrimitivesInstanced(primType, 0, pPacket->BP_indexCount, INDEXCOUNT);
   }
   else
#endif
   {
      gpRenderBackend->RenderPrimitives(primType, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
                                        0, pPacket->BP_vertexCount, 
#endif
                                        pPacket->BP_startIndex, pPacket->BP_indexCount);

      for( int iBlendPass = 1; iBlendPass < gGS_BlendModeDrawCount; ++iBlendPass )
      {
         BP_GS_SetAlpha(pPacket->tex_trans_alpha_data, iBlendPass);
         BP_BeginShader(PrimShader::gpShader[textureType][isSprite][isFog][gGS_ForceRGBWhite][gPrim_PS2ColorClamp], NULL, 0);
         gpRenderBackend->RenderPrimitives(primType, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
                                           0, pPacket->BP_vertexCount, 
#endif
                                           pPacket->BP_startIndex, pPacket->BP_indexCount);
      }
   }

   if( BPE_BRANCH_HINT_FALSE(pPacket->flag & DG_PRIM_NO_DEPTH_TEST) )
      gpRenderBackend->SetDepthCompareEnabled(true);
}

//----------------------------------------------------------------------------

void BP_Prim_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   int const renderMenu = BP_DebugMenu_GetMenu("Render");

   int const primMenu = BP_DebugMenu_AddMenu("Prim", renderMenu);
   BP_DebugMenu_AddBool(primMenu, "Enable Line", &gPrim_EnableLine);
   BP_DebugMenu_AddBool(primMenu, "Enable Poly", &gPrim_EnablePoly);
   BP_DebugMenu_AddBool(primMenu, "Enable CullPoly", &gPrim_EnableCullPoly);
   BP_DebugMenu_AddBool(primMenu, "Enable Sprt", &gPrim_EnableSprt);
   BP_DebugMenu_AddBool(primMenu, "Enable RSprt", &gPrim_EnableRSprt);
   BP_DebugMenu_AddSeparator(primMenu);
   BP_DebugMenu_AddBool(primMenu, "Show Stats", &gPrim_ShowStats);
   BP_DebugMenu_AddBool(primMenu, "Show Vertex Count Issues", &gPrim_ShowVertexCountIssues);
   BP_DebugMenu_AddBool(primMenu, "PS2 Color Clamp", &gPrim_PS2ColorClamp);
   BP_DebugMenu_AddBool(primMenu, "Offscreen Render", &gPrimOffscren_Enable);
   BP_DebugMenu_AddBool(primMenu, "Optimize Particle Shape", &gPrim_OptimizeParticleShape);
   BP_DebugMenu_AddBool(primMenu, "Alpha Cull", &gPrim_CullAlpha);
#if BP_PRIM_DEBUGGING
#if BP_VITA
   BP_DebugMenu_AddBool(primMenu, "Dump Prims", &gDumpPrims);
   BP_DebugMenu_AddBool(primMenu, "Show Prims", &gShowPrims);
   BP_DebugMenu_AddBool(primMenu, "Clear Prims", &gClearPrims);
   BP_DebugMenu_AddAction(primMenu, "Prof Up", BP_Prim_ProfCtrl, -1);
   BP_DebugMenu_AddAction(primMenu, "Prof Down", BP_Prim_ProfCtrl, 1);
   BP_DebugMenu_AddAction(primMenu, "Prof Toggle", BP_Prim_ProfCtrl, 0);
   BP_DebugMenu_AddBool(primMenu, "Capture", &gDoRazorCaptureReallySoon);
#endif
#endif
   BP_DebugMenu_AddBool(primMenu, "Enable Instancing", &gPrim_EnableInstancing);
   BP_DebugMenu_AddBool(primMenu, "Enable Merging", &gPrim_EnableMerging);
#if MGS_VERSION == 2
   extern int gPatch_EnablePatch;
   BP_DebugMenu_AddBool(primMenu, "Enable Patch", &gPatch_EnablePatch);
#endif
#endif
}

//----------------------------------------------------------------------------

void BP_Prim_BeginViewport()
{
   gIsFirstPrimForViewport = 1;
}

namespace
{
   struct SBP_PrimPacket_RenderWrap;

   struct SBP_PrimPacket_RenderWrap 
   {
      struct SBP_PrimPacket_RenderWrap *mpNext;
      DG_PRIM_PACKET *mpPacket;
      int mClone;
   };

   BPE_CTASSERT( sizeof( SBP_PrimPacket_RenderWrap ) <= sizeof( SBP_PrimPacket_Render ) );

   SBP_PrimPacket_RenderWrap *sRenderWrapHead = NULL;
}

//----------------------------------------------------------------------------

void BP_Prim_Update_AddRenderWrapCommand( DG_PRIM_PACKET *packet, int clone )
{
   SBP_PrimPacket_Render* pPrimPacket_Render = reinterpret_cast<SBP_PrimPacket_Render* >( BP_RB_Alloc(sizeof(SBP_PrimPacket_Render)) );
   SBP_PrimPacket_RenderWrap *pPrimPacket_RenderWrap = (SBP_PrimPacket_RenderWrap *) pPrimPacket_Render;

   pPrimPacket_RenderWrap->mpNext = sRenderWrapHead;
   sRenderWrapHead = pPrimPacket_RenderWrap;
   pPrimPacket_RenderWrap->mpPacket = packet;
   pPrimPacket_RenderWrap->mClone = clone;
   BP_RB_AddCommand(kCmd_Prim_RenderPacket, (char*)pPrimPacket_Render);
}

//----------------------------------------------------------------------------

void BP_Prim_Update_FixupPrimRenderPackets()
{
   int clone;
   while ( sRenderWrapHead )
   {
      SBP_PrimPacket_Render *pPrimPacket_Render = reinterpret_cast<SBP_PrimPacket_Render *>( sRenderWrapHead );
      DG_PRIM_PACKET *packet = sRenderWrapHead->mpPacket;
      clone = sRenderWrapHead->mClone;

      sRenderWrapHead = sRenderWrapHead->mpNext;

      // Ok, sRenderWrapHead now points to the next item, pPrimPacket_Render is our current writable packet
      // and packet is the packet we care about
      DG_PRIM* prim = reinterpret_cast<DG_PRIM *>( packet->prim );

      pPrimPacket_Render->flag = prim->flag;
      pPrimPacket_Render->fname = prim->fname;
      pPrimPacket_Render->BP_vertexBuffer = prim->BP_vertexBuffer;

      BPE_CTASSERT( sizeof( pPrimPacket_Render->BP_indexBuffer ) == sizeof( prim->BP_indexBuffer ) );
      memcpy( pPrimPacket_Render->BP_indexBuffer, prim->BP_indexBuffer, sizeof( prim->BP_indexBuffer ) );
      pPrimPacket_Render->BP_vertexCount = prim->BP_vertexCount;
      pPrimPacket_Render->BP_startIndex = packet->BP_startIndex;
      pPrimPacket_Render->BP_indexCount = packet->BP_indexCount;
      pPrimPacket_Render->BP_instanced = prim->BP_instanced;

      pPrimPacket_Render->BP_forceDisableAlphaTest = prim->BP_forceDisableAlphaTest;
      pPrimPacket_Render->BP_forceZWrite = prim->BP_forceZWrite;

#if MGS_VERSION == 3
      if (prim->flag & DG_PRIM_CLONED && clone >= 0)
      {
         pPrimPacket_Render->tex_trans_alpha_data = prim->clones[clone].tex_trans.alpha.data;
         pPrimPacket_Render->tex_trans_clamp_data = prim->clones[clone].tex_trans.clamp.data;
      }
      else
#endif
      {
         pPrimPacket_Render->tex_trans_alpha_data = prim->tex_trans.alpha.data;
         pPrimPacket_Render->tex_trans_clamp_data = prim->tex_trans.clamp.data;
      }

      BP_RB_CopyTexture(reinterpret_cast<void **>( &pPrimPacket_Render->BP_tex ), prim->BP_tex);
      pPrimPacket_Render->type = prim->type;
     
#if MGS_VERSION == 3
      pPrimPacket_Render->BP_fogBuffer = packet->flag;

      if (gPrim_EnableMerging && (prim->BP_optflag & kPOpt_Mask))
      {
         // change lerp value from DstA to SrcA
         pPrimPacket_Render->mask_trans_alpha_data = prim->child->tex_trans.alpha.data & ~ABCD(0, 0, 3, 0);
         pPrimPacket_Render->mask_trans_clamp_data = prim->child->tex_trans.clamp.data;
         pPrimPacket_Render->BP_optflag = prim->BP_optflag;
         BP_RB_CopyTexture(reinterpret_cast<void **>( &pPrimPacket_Render->BP_masktex ), prim->child->BP_tex);
      }
      else
      {
         pPrimPacket_Render->BP_optflag = 0;
      }

      SPrimShape *primShape = (SPrimShape *)prim->BP_primShape;
      if (gPrim_OptimizeParticleShape && primShape)
      {
         pPrimPacket_Render->BP_optimizedPrimShapeIndex = primShape->mShapeIndex;
         pPrimPacket_Render->BP_indexCountPerInstance = 3*(primShape->mNumVerts - 2);
      }
      else
      {
         pPrimPacket_Render->BP_optimizedPrimShapeIndex = -1;
         pPrimPacket_Render->BP_indexCountPerInstance = 6;
      }
#endif
   }
}
