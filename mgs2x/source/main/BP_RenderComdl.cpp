//----------------------------------------------------------------------------
// BP_RenderClone.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderComdl.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"

#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_RenderGS.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

#include "BP_RenderShared.h"
#include "BP_RenderBufferTypes.h"

//----------------------------------------------------------------------------

namespace
{
   int gNumRendered;
#if BP_VITA
   int gOptimizeLevel = 2;
#else
   int gOptimizeLevel = 1;
#endif
   int gShowStats = 0;
}

//----------------------------------------------------------------------------

typedef struct _SF3d
{
   float x;
   float y;
   float z;
} SF3d;

typedef struct _SS3d
{
   short x;
   short y;
   short z;
} SS3d;

SF3d SS3d_sub(SS3d *a, SS3d *b)
{
   SF3d r;
   r.x = (float)(a->x - b->x);
   r.y = (float)(a->y - b->y);
   r.z = (float)(a->z - b->z);
   return r;
}

SF3d SF3d_cross(SF3d *a, SF3d *b)
{
   SF3d r;
   r.x = a->y*b->z - a->z*b->y;
   r.y = b->x*a->z - b->z*a->x;
   r.z = a->x*b->y - a->y*b->x;
   return r;
}

float SF3d_dot(SF3d *a, SF3d *b)
{
   return a->x*b->x + a->y*b->y + a->z*b->z;
}

#define DRAWING_KICK 0x8000

//----------------------------------------------------------------------------

int gComdl_Enable = 1;
int g2PassComdl_Enable = 1;

//----------------------------------------------------------------------------

struct CLONE_VERTEX
{
   float vx, vy, vz, vw;
   float u0, v0;
};

//----------------------------------------------------------------------------

namespace ComdlShader
{
   int const kFogOffOn = 2;//Off=0, On=1
   int const kInstanceOffOn = 2;//Off=0, On=1
   CCompiledShader* gpShader[kFogOffOn]
#if BP_VITA
   [kInstanceOffOn][skGS_Vita_NumSpecialBlends][CRenderBackend::kAF_Count]
#endif
   = { 0 };

   enum ERegisters
   {
      kReg_Persp     = 20,
      kReg_Col       = 24,
      kReg_UVParams  = 25,

      kReg_Matrix0   = 26,
   };

   BPE_FORCEINLINE CCompiledShader *Shader( int const fog, int const instance )
   {
      CCompiledShader* pShader = gpShader[ fog ]
#if BP_VITA
      [ instance ][ gGS_Vita_CurrentSpecialBlend ][ BP_GS_GetCurrentVitaAlphaFunc() ]
#endif
      ;

      BPE_ASSERT_NO_MSG(pShader != NULL);
      return pShader;
   }

   int const kMaxInstanceCount = 30;
}

void BP_InitComdlShader()
{
   using namespace ComdlShader;

   CShaderFileId const shaderId("$/EngineSupport/Shaders/Comdl.fx");

   for( int isFogOffOn = 0; isFogOffOn < kFogOffOn; ++isFogOffOn )
   {
#if BP_VITA
      for ( int isInstaceOffOn = 0; isInstaceOffOn < kInstanceOffOn; ++isInstaceOffOn )
      {
         for ( int blendMode = 0; blendMode < skGS_Vita_NumSpecialBlends; ++blendMode )
         {
            for ( int alphaFunc = 0; alphaFunc < CRenderBackend::kAF_Count; alphaFunc++ )
            {
               if (BP_IsValidVitaShaderCombination(blendMode, alphaFunc))
               {
                  CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, 
                     CStringExtras::Stringize_s("FOG=%d;INSTANCE=%d;VITA_SPECIAL_BLEND=%d;VITA_PS2_ATEST_FUNC=%d", 
                     isFogOffOn, isInstaceOffOn, blendMode, alphaFunc ) );

                  BPE_ASSERT_NO_MSG(pShader);
                  gpShader[isFogOffOn][isInstaceOffOn][blendMode][alphaFunc] = pShader;
               }
            }
         }
      }
#else
      CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("FOG=%d", isFogOffOn));
      assert(pShader);

      gpShader[isFogOffOn] = pShader;
#endif
   }
}

//----------------------------------------------------------------------------

void BP_Comdl_FrameBegin()
{
   gNumRendered = 0;
}

void BP_Comdl_FrameEnd()
{
   if( gShowStats )
   {
      BP_DebugText_Print("Obj Count: %d", gNumRendered);
   }
}

//----------------------------------------------------------------------------

void BP_Comdl_InitPacket(char* pData)
{
   if( !gComdl_Enable )
      return;

   using namespace ComdlShader;

   SBP_Comdl_InitPacket* pPacket = (SBP_Comdl_InitPacket*)pData;

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
   gpRenderBackend->SetDepthWriteEnabled(false);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   real32 temp[16];

   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->pers, temp);
   gpRenderBackend->SetVertexRegisters(kReg_Persp, 4, (CVector4 const*)temp);

   extern CVector4 gBP_FogColor;
   gpRenderBackend->SetVertexRegisters(kSReg_FogColor, 1, &gBP_FogColor);
   gpRenderBackend->SetVertexRegisters(kSReg_FogParam, 1, (CVector4 const*)&pPacket->fogParam);
}

void BP_CopyToVertexBuffer(void* pVertexData, SBP_Comdl_Render* pPacket)
{
   CLONE_VERTEX * pVertData =  (CLONE_VERTEX*)pVertexData;
   for (int v = 0; v < pPacket->n_verts; ++v)
   {
      //pos
      {
         short *vert = &pPacket->verts[4*v];

         // the verts are 1.15.0 format.
         // the weight (vert.w) is in 1.3.12 format.
         // the bboxes in the def and obj are just for information
         float const kWeightScale = 1.0f / 4096.0f;
         float x = vert[0];
         float y = vert[1];
         float z = vert[2];
         float w = kWeightScale * vert[3];

         //copy to vbo
         pVertData->vx = x;
         pVertData->vy = y;
         pVertData->vz = z;
         pVertData->vw = w;
      }
      //uvs
      {
         float uv_u = 0.0f;
         float uv_v = 0.0f;
         if( pPacket->uvs )
         {
            // uv are 1.3.12
            short *uvs = &pPacket->uvs[2*v];

            uv_u = uvs[0]/4096.0f;
            uv_v = uvs[1]/4096.0f;
         }

         //copy to vbo
         pVertData->u0 = uv_u;
         pVertData->v0 = uv_v;
         ++pVertData;
      }
   }
}

BPE_FORCEINLINE 
void BP_Matrix44Color_to_VS_Matrix44(float const * input, int32 const * color, float * output)
{
   output[0] = input[0];
   output[1] = input[4];
   output[2] = input[8];
   output[3] = input[12];

   output[4] = input[1];
   output[5] = input[5];
   output[6] = input[9];
   output[7] = input[13];

   output[8] = input[2];
   output[9] = input[6];
   output[10] = input[10];
   output[11] = input[14];

   output[12] = color[0];
   output[13] = color[1];
   output[14] = color[2];
   output[15] = color[3];
}

void BP_Comdl_Render(char* pData)
{
   if( !gComdl_Enable )
      return;

   using namespace ComdlShader;

   SBP_Comdl_Render* pPacket = (SBP_Comdl_Render*)pData;

   // Build vertex buffer
   CDynamicVertexBufferPoolChunk_RT * pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(CLONE_VERTEX) * pPacket->n_verts);
   if( !pVertexBuffer )
      return;
   {
      void* pVertexData = pVertexBuffer->Lock();

      BP_CopyToVertexBuffer(pVertexData, pPacket);

      pVertexBuffer->Unlock();
   }

   // Assume worst case triangle count -> One contiguous tri-strip.
   int const kMaxTriangleCount = pPacket->n_verts - 2;
   
   // We'll collect the actual number of triangles as we walk the strips.
   int actualTriangleCount = 0;

   // Build index buffer
   CDynamicIndexBufferPool_RT::TChunk pIndexBuffer = gpRenderBackend->GetIndexBufferPool_RT()->AllocChunk(kMaxTriangleCount * 3);
   if( CDynamicIndexBufferPool_RT::IsChunkNull( pIndexBuffer ) )
      return;

   int t = 0; // Vertex indices start at 1 for OBJ and 0 for MDL   

   {
      uint16* pIndices = CDynamicIndexBufferPool_RT::LockChunk( pIndexBuffer );

      int i1 = t, i2 = t + 1, i3 = t + 2;
      int clk = 1;
      for (int v = 2; v < pPacket->n_verts; v++)
      {
         short *verts = pPacket->verts;
         short *norm = &pPacket->norms[4*v];
         short w = norm[3];
         if( (w & DRAWING_KICK) == 0 )
         {
            SS3d *v1v = (SS3d *)&verts[4*(i1-t)];
            SS3d *v2v = (SS3d *)&verts[4*(i2-t)];
            SS3d *v3v = (SS3d *)&verts[4*(i3-t)];
            SF3d e1 = SS3d_sub(v2v, v1v);
            SF3d e2 = SS3d_sub(v3v, v1v);
            SF3d nc = SF3d_cross(&e1, &e2);
            SF3d n;
            n.x = norm[0]/4096.0f;
            n.y = norm[1]/4096.0f;
            n.z = norm[2]/4096.0f;

            bool const bTriFlipped = (SF3d_dot(&n, &nc) <= 0.0f);
            if (!bTriFlipped)
            {
               *pIndices++ = i1;
               *pIndices++ = i2;
               *pIndices++ = i3;
            }
            else
            {
               *pIndices++ = i1;
               *pIndices++ = i3;
               *pIndices++ = i2;
            }

            ++actualTriangleCount;
         }
         if (clk)
         {
            i1 = i3;
            clk = 0;
         }
         else
         {
            i2 = i3;
            clk = 1;
         }
         i3++;
      }
      
      CDynamicIndexBufferPool_RT::UnlockChunk( pIndexBuffer );
   }

   CVertexData vertexData;

   size_t const vertexBufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof(CLONE_VERTEX) );
   vertexData.SetAttribute(kVDS_Position, offsetof(CLONE_VERTEX, vx) , kVDT_Float4, vertexBufferIndex);
   vertexData.SetAttribute(kVDS_TexCoord0, offsetof(CLONE_VERTEX, u0), kVDT_Float2, vertexBufferIndex);

   CShaderVertexDataBinding binding;
   binding.Set(kVDU_Position, kVDS_Position);
   binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);

   bool canUseSingleDrawPass = false;
   int minTextureAlpha = 255;
   int maxTextureAlpha = 255;
   // Bind texture
   {
      DG_TEX_BP* pTex = pPacket->tex;

      BP_GS_SetAlpha(pTex->tex_trans.alpha.data);

      CBaseTexture const * pTexture = (CBaseTexture const *)pTex->BP_TextureHandle;
      gpRenderBackend->SetTexture(0, pTexture ? pTexture : &gpRenderBackend->GetWhiteMap());
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
      BP_GS_SetClamp(0, pTex->tex_trans.clamp.data);

      real32 const uScale = 1.0f / pTex->u_scale;
      real32 const vScale = 1.0f / pTex->v_scale;
      CVector4 const uvParams(uScale, vScale, -pTex->u_offset * uScale, -pTex->v_offset * vScale);
      gpRenderBackend->SetVertexRegisters(kReg_UVParams, 1, &uvParams);

      maxTextureAlpha = (pTexture->mMaxRGBA&0x000000FF);
      minTextureAlpha = (pTexture->mMinRGBA&0x000000FF);
   }

   int isFogOffOn;
   if ( pPacket->flag & DG_COMDL_NOFOG )
      isFogOffOn = 0;
   else
      isFogOffOn = 1;

   if( minTextureAlpha == maxTextureAlpha )
   {
      canUseSingleDrawPass = true;
   }
   
   gpRenderBackend->SetIndexData(pIndexBuffer);

   DG_COMDL_POS* pMatrixColor = (DG_COMDL_POS*)pPacket->pos;
#if BP_VITA
   if( gOptimizeLevel >= 2 && canUseSingleDrawPass )
   {
      CDynamicVertexBufferPoolChunk_RT * pInstanceBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(FMATRIX) * pPacket->n_objs);
      if( !pInstanceBuffer )
         return;

      gpRenderBackend->SetIndexSourceOverwrite(1, SCE_GXM_INDEX_SOURCE_INSTANCE_16BIT);

      size_t const instanceBufferIndex = vertexData.AddBuffer( pInstanceBuffer, sizeof(FMATRIX) );
      vertexData.SetAttribute(kVDS_TexCoord1, offsetof(FMATRIX, m[0][0]), kVDT_Float4, instanceBufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord2, offsetof(FMATRIX, m[1][0]), kVDT_Float4, instanceBufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord3, offsetof(FMATRIX, m[2][0]), kVDT_Float4, instanceBufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord4, offsetof(FMATRIX, m[3][0]), kVDT_Float4, instanceBufferIndex);

      binding.Set(kVDU_TexCoord1, kVDS_TexCoord1);
      binding.Set(kVDU_TexCoord2, kVDS_TexCoord2);
      binding.Set(kVDU_TexCoord3, kVDS_TexCoord3);
      binding.Set(kVDU_TexCoord4, kVDS_TexCoord4);

      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();

      real32 vsMatrix[16];
      EAlphaTestPass lastDrawPass;
      // we need to find and set first object's "lastDrawPass"
      int i = 0;
      for( ; i < pPacket->n_objs; ++i )
      {
         if( pMatrixColor[i].color.vw != 0.0f )
         {
            int finalInstanceAlpha = minTextureAlpha*(pMatrixColor[i].color.vw/128.0f)*2.0f;
            lastDrawPass = BP_GS_GetPassFor_SingleAlphaValue(finalInstanceAlpha);
            //BP_GS_SetupAlphaFailPass(lastDrawPass);
            //BP_GS_SetAlphaTestEnabled(false);
            break;
         }
      }
      // Note: we are totally cheating since we know we are on vita here, we don't unlock the vertex buffer
      // properly or worry about it needing to sync up to GPU cache memory like other platforms do, notice calls to SetVertexLocation_Unsafe & SetIndexSourceOverwrite
      FMatrix* pInstanceData = (FMatrix*)pInstanceBuffer->Lock();
      // start drawing stuff
      int instanceCount = 0;
      for( ; i < pPacket->n_objs; ++i )
      {
         if( pMatrixColor[i].color.vw != 0.0f )
         {
            int finalInstanceAlpha = minTextureAlpha*(pMatrixColor[i].color.vw/128.0f)*2.0f;
            EAlphaTestPass newDrawPass = BP_GS_GetPassFor_SingleAlphaValue(finalInstanceAlpha);
            if( lastDrawPass == newDrawPass )
            {
               BP_Matrix44Color_to_VS_Matrix44((real32 const*)&pMatrixColor[i].world, (int32 const*)&pMatrixColor[i].color.vx, (real32*)pInstanceData);
               ++pInstanceData;
               ++instanceCount;
            }
            else
            {
               // if we have a instanceCount we have to draw all them now
               if( instanceCount )
               {
                  BP_GS_SetupAlphaFailPass(lastDrawPass);
                  BP_BeginShader(Shader(isFogOffOn, 1/*instance*/), NULL, 0);
                  gpRenderBackend->RenderPrimitivesInstanced(CMeshChunk::kPrimitive_TriangleList, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
                     0, pPacket->n_verts, 
#endif
                     0, actualTriangleCount * 3 * instanceCount, actualTriangleCount * 3);

                  vertexData.ChangeBufferAddress_Unsafe( instanceBufferIndex, pInstanceData );

                  gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);

                  gNumRendered += instanceCount;
                  instanceCount = 0;
               }

               lastDrawPass = newDrawPass;
               //BP_GS_SetupAlphaFailPass(lastDrawPass);
               BP_Matrix44Color_to_VS_Matrix44((real32 const*)&pMatrixColor[i].world, (int32 const*)&pMatrixColor[i].color.vx, (real32*)pInstanceData);
               ++pInstanceData;
               ++instanceCount;
            }
         }
      }
      if( instanceCount )
      {
         BP_GS_SetupAlphaFailPass(lastDrawPass);
         BP_BeginShader(Shader(isFogOffOn, 1/*instance*/), NULL, 0);
         gpRenderBackend->RenderPrimitivesInstanced(CMeshChunk::kPrimitive_TriangleList, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
            0, pPacket->n_verts, 
#endif
            0, actualTriangleCount * 3 * instanceCount, actualTriangleCount * 3);
         gNumRendered += instanceCount;
         instanceCount = 0;
      }

      gpRenderBackend->SetIndexSourceOverwrite(1, (SceGxmIndexSource)-1);
   }
   else if( gOptimizeLevel >= 2 && g2PassComdl_Enable )
   {
      // Two pass instancing, not exactly same result as VITA but close enough to justify the speed up
      CDynamicVertexBufferPoolChunk_RT * pInstanceBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(FMATRIX) * pPacket->n_objs);
      if( !pInstanceBuffer )
         return;

      gpRenderBackend->SetIndexSourceOverwrite(1, SCE_GXM_INDEX_SOURCE_INSTANCE_16BIT);

      size_t const instanceBufferIndex = vertexData.AddBuffer( pInstanceBuffer, sizeof(FMATRIX) );

      vertexData.SetAttribute(kVDS_TexCoord1, offsetof(FMATRIX, m[0][0]), kVDT_Float4, instanceBufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord2, offsetof(FMATRIX, m[1][0]), kVDT_Float4, instanceBufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord3, offsetof(FMATRIX, m[2][0]), kVDT_Float4, instanceBufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord4, offsetof(FMATRIX, m[3][0]), kVDT_Float4, instanceBufferIndex);

      binding.Set(kVDU_TexCoord1, kVDS_TexCoord1);
      binding.Set(kVDU_TexCoord2, kVDS_TexCoord2);
      binding.Set(kVDU_TexCoord3, kVDS_TexCoord3);
      binding.Set(kVDU_TexCoord4, kVDS_TexCoord4);

      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();

      real32 vsMatrix[16];
      // Note: we are totally cheating since we know we are on vita here, we don't unlock the vertex buffer
      // properly or worry about it needing to sync up to GPU cache memory like other platforms do, notice calls to SetVertexLocation_Unsafe & SetIndexSourceOverwrite
      FMatrix* pInstanceData = (FMatrix*)pInstanceBuffer->Lock();
      // start drawing stuff
      int instanceCount = 0;
      for( int i = 0; i < pPacket->n_objs; ++i )
      {
         if( pMatrixColor[i].color.vw != 0.0f )
         {
            int finalInstanceAlpha = minTextureAlpha*(pMatrixColor[i].color.vw/128.0f)*2.0f;
            EAlphaTestPass newDrawPass = BP_GS_GetPassFor_SingleAlphaValue(finalInstanceAlpha);
            
            BP_Matrix44Color_to_VS_Matrix44((real32 const*)&pMatrixColor[i].world, (int32 const*)&pMatrixColor[i].color.vx, (real32*)pInstanceData);
            ++pInstanceData;
            ++instanceCount;
         }
      }
      if( instanceCount )
      {
         BP_GS_SetupAlphaFailPass(kPass_DepthWrite);
         BP_BeginShader(Shader(isFogOffOn, 1/*instance*/), NULL, 0);
         gpRenderBackend->RenderPrimitivesInstanced(CMeshChunk::kPrimitive_TriangleList, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
            0, pPacket->n_verts, 
#endif
            0, actualTriangleCount * 3 * instanceCount, actualTriangleCount * 3);
         gNumRendered += instanceCount;
         //
         BP_GS_SetupAlphaFailPass(kPass_NoDepthWrite);
         BP_BeginShader(Shader(isFogOffOn, 1/*instance*/), NULL, 0);
         gpRenderBackend->RenderPrimitivesInstanced(CMeshChunk::kPrimitive_TriangleList, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
            0, pPacket->n_verts, 
#endif
            0, actualTriangleCount * 3 * instanceCount, actualTriangleCount * 3);
         gNumRendered += instanceCount;
      }

      gpRenderBackend->SetIndexSourceOverwrite(1, (SceGxmIndexSource)-1);
   }
   else
#endif
   if( gOptimizeLevel >= 1 )
   {
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();

      real32 vsMatrix[16];
      for( int i = 0; i < pPacket->n_objs; ++i )
      {
         if( pMatrixColor[i].color.vw != 0.0f )
         {
            BP_Matrix44Color_to_VS_Matrix44((real32 const*)&pMatrixColor[i].world, (int32 const*)&pMatrixColor[i].color.vx, vsMatrix);
            gpRenderBackend->SetVertexRegisters(kReg_Matrix0, 4, (CVector4 const*)vsMatrix);

            int AlphaFailPassCount_Optimized = gGS_AlphaFailPassCount;
            SBP_TestState* pOptimizeTestState = NULL;
            if( gGS_AlphaFailPassCount > 1 )
            {
               int constantParamAlpha = pMatrixColor[i].color.vw; // (0-128) [0.0-1.0]

               int maxCombineAlpha = (maxTextureAlpha*constantParamAlpha)>>6; // we are only dividing by 64 to simulate *2 alpha in shader code
               int minCombineAlpha = (minTextureAlpha*constantParamAlpha)>>6; // we are only dividing by 64 to simulate *2 alpha in shader code
               AlphaFailPassCount_Optimized = BP_GS_AlphaFailPass_Optimize(minCombineAlpha, maxCombineAlpha, minCombineAlpha, &pOptimizeTestState);
            } 
            for( int alphaTestPass = 0; alphaTestPass < AlphaFailPassCount_Optimized; ++alphaTestPass )
            {
               BP_GS_SetupAlphaFailPass_Optimize(gGS_AlphaFailPassStart + alphaTestPass, pOptimizeTestState);
               BP_BeginShader(Shader(isFogOffOn, 0/*instance*/), NULL, 0);
               gpRenderBackend->RenderPrimitives(CMeshChunk::kPrimitive_TriangleList, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
                  0, pPacket->n_verts, 
#endif
                  0, actualTriangleCount * 3);
               ++gNumRendered;
            }
         }
      }
   }
   else
   {
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();

      real32 vsMatrix[16];
      for( int i = 0; i < pPacket->n_objs; ++i )
      {
         //if( pMatrixColor[i].color.vw )
         {
            BP_Matrix44Color_to_VS_Matrix44((real32 const*)&pMatrixColor[i].world, (int32 const*)&pMatrixColor[i].color.vx, vsMatrix);
            gpRenderBackend->SetVertexRegisters(kReg_Matrix0, 4, (CVector4 const*)vsMatrix);

            for( int alphaTestPass = 0; alphaTestPass < gGS_AlphaFailPassCount; ++alphaTestPass )
            {
               BP_BeginShader(Shader(isFogOffOn, 0/*instance*/), NULL, 0);
               BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart + alphaTestPass);
               gpRenderBackend->RenderPrimitives(CMeshChunk::kPrimitive_TriangleList, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
                  0, pPacket->n_verts, 
#endif
                  0, actualTriangleCount * 3);
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

void BP_Comdl_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   int const renderMenu = BP_DebugMenu_GetMenu("Render");

   int const debugMenu = BP_DebugMenu_AddMenu("Comdl", renderMenu);
   BP_DebugMenu_AddBool(debugMenu, "Enable Render", &gComdl_Enable);

   static const char* skOptimizeCloneRenderMode[] = { "none", "level 1", "level 2" };
   BP_DebugMenu_AddEnum(debugMenu, "Optimization Level", skOptimizeCloneRenderMode, &gOptimizeLevel, 0, 2);

   BP_DebugMenu_AddBool(debugMenu, "Enable 2 Pass Comdl", &g2PassComdl_Enable);

   BP_DebugMenu_AddBool(debugMenu, "Show Stats", &gShowStats);
#endif
}
