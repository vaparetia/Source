//----------------------------------------------------------------------------
// BP_DebugPrim.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

//----------------------------------------------------------------------------

#include "BP_DebugPrim.h"
#include "BP_Renderer.h"
#include "BP_RenderShared.h"

#include "bp_matrix.h"

#include "Renderer/Base/Primitive/CVertexData.h"

//----------------------------------------------------------------------------

namespace DebugShader
{
   CCompiledShader*  gpShader[1];

   enum
   {
      kReg_Projection     = 0,
   };
}

void BP_InitDebugShader()
{
   using namespace DebugShader;

   CShaderFileId const shaderId("$/EngineSupport/Shaders/Debug.fx");

   {
      CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, "");
      assert(pShader);

      gpShader[0] = pShader;
   }
}

//----------------------------------------------------------------------------

#if BP_ENABLE_DEBUG_PRIM

struct SDebugVert
{
   CVector3 pos;
   uint32   col;
};

struct SDebugVert_POD
{
   float pos[3];
   uint32 col;
};

struct SDebugPrim
{
   CMeshChunk::EPrimitive  type;
   int                     depthTest;
   int                     vertexCount;
};

namespace DebugPrim
{
   FMATRIX worldToScreen;
   FMATRIX worldToView;
   FMATRIX viewToWorld;
   FVECTOR bias;

   int const kMaxDebugVerts = 64 * 1024;
   int const kMaxDebugPrims = 16 * 1024;

   int debugVertCount = 0;
   SDebugVert debugVerts[kMaxDebugVerts];
   
   int debugPrimCount = 0;
   SDebugPrim debugPrims[kMaxDebugPrims];

   SDebugPrim* currentDebugPrim = NULL;
};

void BP_DebugPrim_SetMatrices(FMATRIX const * worldToScreenMatrix, FMATRIX const * worldToViewMatrix)
{
   using namespace DebugPrim;
   
   // Save matrices
   worldToScreen = *worldToScreenMatrix;
   worldToView = *worldToViewMatrix;
   BP_Mat_FastInverse( &viewToWorld, &worldToView );

   // Compute z bias in world space so we can add it directly to vertices
   BP_Vec4_Set( &bias, 0.0f, 0.0f, -20.0f, 1.0f );
   BP_Mat_RotateVec3( &bias, &viewToWorld, &bias );
}

void BP_DebugPrim_BeginLines(int depthTest)
{
   using namespace DebugPrim;

   if( !currentDebugPrim || currentDebugPrim->type != CMeshChunk::kPrimitive_LineList || currentDebugPrim->depthTest != depthTest)
   {
      if( debugPrimCount < kMaxDebugPrims )
      {
         currentDebugPrim = debugPrims + debugPrimCount;
         ++debugPrimCount;
         currentDebugPrim->type = CMeshChunk::kPrimitive_LineList;
         currentDebugPrim->depthTest = depthTest;
         currentDebugPrim->vertexCount = 0;
      }
      else
      {
         currentDebugPrim = NULL;
      }
   }
}

void BP_DebugPrim_AddVert(FVECTOR const * pos, unsigned int color)
{
   using namespace DebugPrim;

   if( currentDebugPrim && debugVertCount < kMaxDebugVerts )
   {
      debugVerts[debugVertCount].pos = *(CVector3 const*)pos;
      if( currentDebugPrim->depthTest )
      {
         debugVerts[debugVertCount].pos.mX += bias.vx;
         debugVerts[debugVertCount].pos.mY += bias.vy;
         debugVerts[debugVertCount].pos.mZ += bias.vz;
      }
      debugVerts[debugVertCount].col = color;
      debugVertCount++;
      currentDebugPrim->vertexCount++;
   }
}

void BP_DebugPrim_DrawSphere(int depthTest, FVECTOR const * pos, float radius, unsigned int color )
{
   int32 const numSubsLat = 10;
   int32 const numSubsLong = 10;

   real32 const latInterval = (gkPi32 * 2.0f) / numSubsLat;
   real32 const longInterval = (gkPi32 * 2.0f) / numSubsLong;

   CVector3 c( pos->vx, pos->vy, pos->vz );

   BP_DebugPrim_BeginLines(depthTest);

   for (int latLoop = 0; latLoop < numSubsLat; latLoop++)
   {
      real32 const lat = latLoop * latInterval;
      real32 const nextLat = lat + latInterval;

      real32 const cosLat = radius * cosf( lat );
      real32 const sinLat = radius * sinf( lat );

      real32 const nextCosLat = radius * cosf( nextLat );
      real32 const nextSinLat = radius * sinf( nextLat );

      real32 lon = -gkPiOverTwo32;

      for (int longLoop = 0; longLoop < numSubsLong; longLoop++)
      {
         real32 const nextLon = lon + longInterval;

         real32 const cosLon = cosf( lon );
         real32 const sinLon = sinf( lon );

         real32 const nextCosLon = cosf( nextLon );
         real32 const nextSinLon = sinf( nextLon );
         lon = nextLon;

         CVector3 v1( cosLat * cosLon, cosLat * sinLon, sinLat );
         CVector3 v2( cosLat * nextCosLon, cosLat * nextSinLon, sinLat );
         CVector3 v3( nextCosLat * nextCosLon, nextCosLat * nextSinLon, nextSinLat );

         v1 += c;
         v2 += c;
         v3 += c;

         BP_DebugPrim_AddVert2Col1((FVECTOR const *)&v1, (FVECTOR const *)&v2, color);
         BP_DebugPrim_AddVert2Col1((FVECTOR const *)&v2, (FVECTOR const *)&v3, color);
      }
   }
}

void BP_DebugPrim_Render()
{
   using namespace DebugPrim;

   if( !debugVertCount )
      return;

   // Set Projection
   {
      float temp[16];
      BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&worldToScreen, temp);
      gpRenderBackend->SetVertexRegisters(DebugShader::kReg_Projection, 4, (CVector4 const *)temp);
   }

   // Create vertex buffer
   CDynamicVertexBufferPoolChunk_RT * pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SDebugVert) * debugVertCount);
   if( !pVertexBuffer )
      return;
   {
      void * pData = pVertexBuffer->Lock();
      memcpy(pData, debugVerts, sizeof(SDebugVert) * debugVertCount);
      pVertexBuffer->Unlock();
   }

   CVertexData vertexData;
   size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SDebugVert ) );
   vertexData.SetAttribute(kVDS_Position, offsetof(SDebugVert_POD, pos), kVDT_Float3, bufferIndex);
   vertexData.SetAttribute(kVDS_Color0, offsetof(SDebugVert_POD, col), kVDT_UByte4N, bufferIndex);

   CShaderVertexDataBinding binding;
   binding.Set(kVDU_Position, kVDS_Position);
   binding.Set(kVDU_TexCoord0, kVDS_Color0);

   gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
   gpRenderBackend->ForceVertexDataRebind();

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha);

   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);

   BP_BeginShader(DebugShader::gpShader[0], NULL, 0);

   int currentVertexOffset = 0;

   for( int i = 0; i < debugPrimCount; ++i )
   {
      SDebugPrim const * pPrim = debugPrims + i;
      gpRenderBackend->SetDepthCompareEnabled(pPrim->depthTest ? true : false);

      gpRenderBackend->RenderPrimitivesNoIndices(pPrim->type, currentVertexOffset, pPrim->vertexCount);
      currentVertexOffset += pPrim->vertexCount;
   }

   gpRenderBackend->SetDepthCompareEnabled(true);
}

void BP_DebugPrim_Clear()
{
   using namespace DebugPrim;

   debugVertCount = 0;
   debugPrimCount = 0;
   currentDebugPrim = NULL;
}

void BP_DebugPrim_DrawTransformedBox(FMATRIX *world, int depthTest, FVECTOR const * min, FVECTOR const * max, unsigned int color)
{
   FVECTOR vert[8];
   int i;

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

   for (i = 0; i < 8; ++i)
   {
      BP_Mat_TransformVec3(&vert[i], world, &vert[i]);
   }

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


#endif
