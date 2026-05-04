//----------------------------------------------------------------------------
// BP_RenderDmaPack.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderDmaPack.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "BP_Renderer.h"
#include "BP_RenderGS.h"
#include "BP_BaseRenderer.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderFX.h"
#include "BP_RendererDebug.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

#include "BP_RenderShared.h"

//----------------------------------------------------------------------------

#if MARCO
   #define BP_RENDER_DMA_BREAK BP_BREAK;
#else
   #define BP_RENDER_DMA_BREAK
#endif

#if MGS_VERSION == 2
   #define DG_DMAPACK_CMD_TEXTURE DG_DMAPACK_CMD_TEX 
   #define DG_DMAPACK_TEXTURE DG_DMAPACK_TEX
#endif

#if MGS_VERSION == 2
#  define DMAPACK_SUPPORTS_FLOAT_PACKETS 1
#endif

//----------------------------------------------------------------------------


#if 0
/* FV_I2FSCALE */
SET_VECTOR( &packet->vu1_param[0], 1.0f, 1.0f, 1.0f/16.0f, 1.0f/16.0f );
/* FV_MAT0~3 */
SET_VECTOR( &packet->vu1_param[1], 1.0f, 0.0f, 0.0f, 0.0f );
SET_VECTOR( &packet->vu1_param[2], 0.0f, 1.0f, 0.0f, 0.0f );
SET_VECTOR( &packet->vu1_param[3], 0.0f, 0.0f, 0.0f, 0.0f );
SET_VECTOR( &packet->vu1_param[4], 0.0f, 0.0f, 1.0f, 1.0f );
/* FV_HSCALE, FV_HOFFSET */
SET_VECTOR( &packet->vu1_param[5], (2.0/DRAW_WIDTH), (2.0/DRAW_HEIGHT), 1.0, 0.0 );
SET_VECTOR( &packet->vu1_param[6], -1.0, -1.0, 0.0, 0.0 );
/* FV_PSCALE, FV_POFFSET */
SET_VECTOR( &packet->vu1_param[7], DRAW_WIDTH/2, DRAW_HEIGHT/2, (DRAW_Z_MAX-DRAW_Z_MIN), 0.0 );
SET_VECTOR( &packet->vu1_param[8], 2048.0, 2048.0, DRAW_Z_MIN, 0.0 );
/* FV_TEXSCALE, FV_TEXOFFSET */
SET_VECTOR( &packet->vu1_param[9], 1.0, 1.0, 0.0, 0.0 );
SET_VECTOR( &packet->vu1_param[10], 0.0, 0.0, 0.0, 0.0 );
/* FV_BASECOL */
SET_VECTOR( &packet->vu1_param[11], 1.0f, 1.0f, 1.0f, 1.0f );
#endif

namespace DMAPackShader
{
#if BP_VITA
   int const kTextureCount = 3;
#else
   int const kTextureCount = 2;
#endif
   enum ETexureAlpha
   {
      kTA_NoAlpha,
      kTA_Raw,
      kTA_RenderTarget,
      kTA_Count
   };
   int const kForceRGBWhiteCount = 2;

   CCompiledShader*  gpShader
      [kTextureCount]
      [kTA_Count]
      [kForceRGBWhiteCount]
#if BP_VITA
      [skGS_Vita_NumSpecialBlends][CRenderBackend::kAF_Count]
#endif
      ;

   BPE_FORCEINLINE CCompiledShader *Shader( int const texture, int const textureAlpha, int const forceRgbWhite )
   {
      return gpShader[ texture ][ textureAlpha ][ forceRgbWhite ]
#if BP_VITA
      [gGS_Vita_CurrentSpecialBlend][ BP_GS_GetCurrentVitaAlphaFunc() ]
#endif
      ;
   }

   struct SVertexRegisterLayout
   {
      SVertexRegisterLayout()
#if MGS_VERSION == 2
      :  mI2FScale(1.0f, 1.0f, 1.0f / 16384.0f, 1.0f / 16384.0f )
#elif MGS_VERSION == 3
         :  mI2FScale(1.0f, 1.0f, 1.0f / 16.0f, 1.0f / 16.0f )
#endif
      ,  mMat(CMatrix4::Identity())
      ,  mHScale(2.0f / DRAW_WIDTH, -2.0f / DRAW_HEIGHT, 0.0f, 1.0f)
      ,  mHOffset(-1.0f, 1.0f, 0.0f, 0.0f)
      ,  mPScale(DRAW_WIDTH/2.0f, DRAW_HEIGHT/2.0f, (DRAW_Z_MAX-DRAW_Z_MIN), 0.0f)
      ,  mPOffset(2048.0f, 2048.0f, DRAW_Z_MIN, 0.0f)
      ,  mBaseCol(1.0f, 1.0f, 1.0f, 1.0f)
      ,  mUVMulAdd(1.0f, 1.0f, 0.0f, 0.0f)
      {
      }

      CVector4    mI2FScale;
      CMatrix4    mMat;
      CVector4    mHScale;    // homogeneous scale
      CVector4    mHOffset;   // homogeneous offset
      CVector4    mPScale;    // projection scale
      CVector4    mPOffset;   // projection offset
      CVector4    mBaseCol;
      CVector4    mUVMulAdd;
   };

   CBaseTexture*           gDynamicPlaceholderTexture = NULL;

   SVertexRegisterLayout   gVertexRegisters;
   int                     gHasTexture = 0;
   int                     gUseTextureAlpha = kTA_NoAlpha;
   int                     gUseTextureAlphaOverride = 1; // If both gUseTextureAlphaOverride use gUseTextureAlpha otherwise use kTA_NoAlpha 
   
   void FlushVertexRegisters()
   {
      gpRenderBackend->SetVertexRegisters(0, sizeof(SVertexRegisterLayout) / sizeof(CVector4), (CVector4*)&gVertexRegisters);
   }
}

void BP_InitDmaPackShader()
{
   using namespace DMAPackShader;

   CShaderFileId const shaderId("$/EngineSupport/Shaders/DmaPack.fx");

#if BP_VITA
   CCompiledShaderCache::PushInhibitShaderWarning();
#endif

   for( int texture = 0; texture < kTextureCount; ++texture )
   {
      for( int alpha = 0; alpha < kTA_Count; ++alpha )
      {
         for( int forceRGBWhite = 0; forceRGBWhite < kForceRGBWhiteCount; ++forceRGBWhite )
         {
#if BP_VITA
            for ( int blendMode = 0; blendMode < skGS_Vita_NumSpecialBlends; ++blendMode )
            {
               for ( int alphaFunc = 0; alphaFunc < CRenderBackend::kAF_Count; ++alphaFunc )
               {
                  CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d;TEXTUREALPHA=%d;FORCE_RGB_WHITE=%d;VITA_SPECIAL_BLEND=%d;VITA_PS2_ATEST_FUNC=%d", texture, alpha, forceRGBWhite,
                     blendMode,
                     alphaFunc ) );
                  // it's ok for a shader to be NULL, according to Jack
                  // assert(pShader);

                  gpShader[texture][alpha][forceRGBWhite][blendMode][alphaFunc] = pShader;
               }
            }
#else

            CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d;TEXTUREALPHA=%d;FORCE_RGB_WHITE=%d", texture, alpha, forceRGBWhite));
            assert(pShader);

            gpShader[texture][alpha][forceRGBWhite] = pShader;
#endif
         }
      }
   }

#if BP_VITA
   CCompiledShaderCache::PopInhibitShaderWarning();
#endif

   gDynamicPlaceholderTexture = CBaseTexture::CreateSolid(CColor(128, 0, 128, 16));
}

//----------------------------------------------------------------------------

struct SVertexPosCol
{
   // Make sure _POD changes if this changes!
   CVector4 pos;
   CVector4 col;
};

struct SVertexPosCol_POD
{
   // Make sure that this matches the non _POD version above
   float pos[4];
   float col[4];
};

struct SVertexPosUvCol
{
   // Make sure _POD changes if this changes!
   CVector4 pos;
   CVector2 uv;
   CVector4 col;
   
};

struct SVertexPosUvCol_POD
{
   // Make sure that this matches the non _POD version above
   float pos[4];
   float uv[2];
   float col[4];
};

static void *ExecPoint( void *buffer )
{
   DG_DMAPACK_POINT* packet = (DG_DMAPACK_POINT*)buffer;
   return ( &packet[1] );
}

static void *ExecLine( void *packet_addr )
{
   DG_DMAPACK_LINE* packet = (DG_DMAPACK_LINE*)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 2);
   if( pVertexBuffer )
   {
      {
         SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();

         pVertex->pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba0); 
         pVertex++;
         pVertex->pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba1); 
         pVertex++;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader( 0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineList, 0, 2);
   }

   return ( &packet[1] );
}

#if MGS_VERSION == 2
static void *ExecLine_F( void *packet_addr )
{
   DG_DMAPACK_LINE_F* packet = (DG_DMAPACK_LINE_F*)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 2);
   if( pVertexBuffer )
   {
      {
         SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();

         pVertex->pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba0); 
         pVertex++;
         pVertex->pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba1); 
         pVertex++;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader(0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineList, 0, 2);
   }

   return ( &packet[1] );
}
#endif

static void *ExecTriangle( void *packet_addr )
{
   DG_DMAPACK_TRIANGLE* packet = (DG_DMAPACK_TRIANGLE*)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 3);
   if( pVertexBuffer )
   {
      SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();

      pVertex->pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f);
      pVertex->col = BP_DecodeColorF(packet->rgba0); 
      pVertex++;
      pVertex->pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
      pVertex->col = BP_DecodeColorF(packet->rgba1); 
      pVertex++;
      pVertex->pos = CVector4(packet->x2, packet->y2, 0.0f, 1.0f); 
      pVertex->col = BP_DecodeColorF(packet->rgba2); 
      pVertex++;

      pVertexBuffer->Unlock();

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader(0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleList, 0, 3);
   }

   return ( &packet[1] );
}

static void *ExecQuad( void *packet_addr )
{
   DG_DMAPACK_QUAD* packet = (DG_DMAPACK_QUAD*)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();

         pVertex->pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f);
         pVertex->col = BP_DecodeColorF(packet->rgba0); 
         pVertex++;
         pVertex->pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba1); 
         pVertex++;
         pVertex->pos = CVector4(packet->x2, packet->y2, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba2); 
         pVertex++;
         pVertex->pos = CVector4(packet->x3, packet->y3, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba3); 
         pVertex++;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader(0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
   }

   return ( &packet[1] );
}

#if DMAPACK_SUPPORTS_FLOAT_PACKETS

static void *ExecQuad_F( void *packet_addr )
{
   DG_DMAPACK_QUAD_F *packet = (DG_DMAPACK_QUAD_F *)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();

         pVertex->pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f);
         pVertex->col = BP_DecodeColorF(packet->rgba0); 
         pVertex++;
         pVertex->pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba1); 
         pVertex++;
         pVertex->pos = CVector4(packet->x2, packet->y2, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba2); 
         pVertex++;
         pVertex->pos = CVector4(packet->x3, packet->y3, 0.0f, 1.0f); 
         pVertex->col = BP_DecodeColorF(packet->rgba3); 
         pVertex++;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader(0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
   }

   return &packet[1];
}

#endif // DMAPACK_SUPPORTS_FLOAT_PACKETS

static void *ExecBox( void *packet_addr )
{
   DG_DMAPACK_BOX* packet = (DG_DMAPACK_BOX*)packet_addr;

   //    SCRPAD_WORK			*scrpad = DG_SCRPAD_ADDR ;
//    SPRT_PACKET		*buffer ;
// 
//    buffer = (void*)scrpad->dma_buffer ;
//    buffer->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(*buffer)-1 );
//    buffer->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
//    buffer->dmatag.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x00, 2*2, VIF_VERT_PACK, 0 );
//    buffer->verts[0].x = packet->x ;
//    buffer->verts[0].y = packet->y ;
//    buffer->verts[0].rgba = COLCONV_32_64( packet->rgba );
//    buffer->verts[1].x = packet->w ;
//    buffer->verts[1].y = packet->h ;
//    buffer->verts[1].rgba = COLCONV_32_64( packet->rgba );
//    buffer->vifcode2[0] = SCE_VIF1_SET_MSCAL( Vu1Draw2DPackets_Func[ CMD_SprtNoTex ], 0 );
//    buffer->vifcode2[1] = SCE_VIF1_SET_NOP( 0 );
//    buffer->vifcode2[2] = SCE_VIF1_SET_NOP( 0 );
//    buffer->vifcode2[3] = SCE_VIF1_SET_NOP( 0 );
//    DG_SendCacheFIFO( scrpad->local_work, buffer, SIZEOF_QWORD(*buffer) );

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();

#if MGS_VERSION == 2
         pVertex[0].pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x1, packet->y0, 0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x0, packet->y1, 0.0f, 1.0f); 
#elif MGS_VERSION == 3
         pVertex[0].pos = CVector4(packet->x,              packet->y,              0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x + packet->w,  packet->y,              0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x + packet->w,  packet->y + packet->h,  0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x,              packet->y + packet->h,  0.0f, 1.0f); 
#endif
         pVertex[0].col = pVertex[1].col = pVertex[2].col = pVertex[3].col = BP_DecodeColorF(packet->rgba); 

         pVertex += 4;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader(0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
   }
   return ( &packet[1] );
}

#if DMAPACK_SUPPORTS_FLOAT_PACKETS

static void *ExecBox_F( void *packet_addr )
{
   DG_DMAPACK_BOX_F *packet = (DG_DMAPACK_BOX_F *)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();

#if MGS_VERSION == 2
         pVertex[0].pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x1, packet->y0, 0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x0, packet->y1, 0.0f, 1.0f); 
#elif MGS_VERSION == 3
         pVertex[0].pos = CVector4(packet->x,              packet->y,              0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x + packet->w,  packet->y,              0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x + packet->w,  packet->y + packet->h,  0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x,              packet->y + packet->h,  0.0f, 1.0f); 
#endif
         pVertex[0].col = pVertex[1].col = pVertex[2].col = pVertex[3].col = BP_DecodeColorF(packet->rgba); 

         pVertex += 4;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader(0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
   }
   return &packet[1];
}

#endif // DMAPACK_SUPPORTS_FLOAT_PACKETS

static void *ExecSprt( void *packet_addr )
{
   DG_DMAPACK_SPRT* packet = (DG_DMAPACK_SPRT*)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosUvCol* pVertex = (SVertexPosUvCol*)pVertexBuffer->Lock();

#if MGS_VERSION == 2
         pVertex[0].pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x1, packet->y0, 0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x0, packet->y1, 0.0f, 1.0f); 
#elif MGS_VERSION == 3
         pVertex[0].pos = CVector4(packet->x,              packet->y,              0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x + packet->w,  packet->y,              0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x + packet->w,  packet->y + packet->h,  0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x,              packet->y + packet->h,  0.0f, 1.0f); 
#endif
         pVertex[0].uv = CVector2(packet->u0, packet->v0);
         pVertex[1].uv = CVector2(packet->u1, packet->v0);
         pVertex[2].uv = CVector2(packet->u1, packet->v1);
         pVertex[3].uv = CVector2(packet->u0, packet->v1);

         pVertex[0].col = pVertex[1].col = pVertex[2].col = pVertex[3].col = BP_DecodeColorF(packet->rgba); 

         pVertex += 4;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      int const textureAlphaMode = DMAPackShader::gUseTextureAlphaOverride ? DMAPackShader::gUseTextureAlpha : DMAPackShader::kTA_NoAlpha; 
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();

      // Kyle_op in MGS3 perforated screen secret overlay uses special blendmodes
      if( gGS_BlendModeDrawCount == 1 )
      {
         BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, textureAlphaMode, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
      }
      else
      {
         // Pass 0 call uses previously set or restored blend mode
         BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, textureAlphaMode, gGS_ForceRGBWhite ), NULL, 0);
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
         // Pass 1 - gGS_BlendModeDrawCount set alpha mode based on pass manually and reset to pass 0 mode when done
         for( int iBlendPass=1;iBlendPass<gGS_BlendModeDrawCount;++iBlendPass )
         {
            BP_GS_SetAlpha_UseLastBlendMode(iBlendPass);
            BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, textureAlphaMode, gGS_ForceRGBWhite ), NULL, 0);

            gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
         }
         // restore blend mode for future DmaPack draw calls
         BP_GS_SetAlpha_UseLastBlendMode(0);
      }
   }
   return ( &packet[1] );
}

static void *ExecRSprt( void *packet_addr )
{
   DG_DMAPACK_RSPRT* packet = (DG_DMAPACK_RSPRT*)packet_addr;
 
   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosUvCol* pVertex = (SVertexPosUvCol*)pVertexBuffer->Lock();

         pVertex[0].pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x3, packet->y3, 0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x2, packet->y2, 0.0f, 1.0f); 

         pVertex[0].uv = CVector2(packet->u0, packet->v0);
         pVertex[1].uv = CVector2(packet->u1, packet->v0);
         pVertex[2].uv = CVector2(packet->u1, packet->v1);
         pVertex[3].uv = CVector2(packet->u0, packet->v1);

         pVertex[0].col = pVertex[1].col = pVertex[2].col = pVertex[3].col = BP_DecodeColorF(packet->rgba); 

         pVertex += 4;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      if( DMAPackShader::gUseTextureAlphaOverride )
      {
         BP_BeginShader(DMAPackShader::Shader(DMAPackShader::gHasTexture, DMAPackShader::gUseTextureAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      }
      else
      {
         BP_BeginShader(DMAPackShader::Shader(DMAPackShader::gHasTexture, DMAPackShader::kTA_NoAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      }
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
   }

   return ( &packet[1] );
}

#if DMAPACK_SUPPORTS_FLOAT_PACKETS

static void *ExecSprt_F( void *packet_addr )
{
   DG_DMAPACK_SPRT_F *packet = (DG_DMAPACK_SPRT_F *)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosUvCol *pVertex = (SVertexPosUvCol *)pVertexBuffer->Lock();

#if MGS_VERSION == 2
         pVertex[0].pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x1, packet->y0, 0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x0, packet->y1, 0.0f, 1.0f); 
#elif MGS_VERSION == 3
         pVertex[0].pos = CVector4(packet->x,              packet->y,              0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x + packet->w,  packet->y,              0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x + packet->w,  packet->y + packet->h,  0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x,              packet->y + packet->h,  0.0f, 1.0f); 
#endif
         real32 uoff = 0.0f, voff = 0.0f;
         if (DMAPackShader::gHasTexture)
         {
            // invert shader uv scaling (see DMAPack.fx)
            real32 const oodu = DMAPackShader::gVertexRegisters.mI2FScale.mZ*DMAPackShader::gVertexRegisters.mUVMulAdd.mX;
            real32 const oodv = DMAPackShader::gVertexRegisters.mI2FScale.mW*DMAPackShader::gVertexRegisters.mUVMulAdd.mY;
            // offset half a texel to correct bilinear filtering
            uoff = 1.0f/(2.0f*oodu*CBaseTexture::GetCurrentTextureWidth(0));
            voff = 1.0f/(2.0f*oodv*CBaseTexture::GetCurrentTextureHeight(0));
         }
         pVertex[0].uv = CVector2(packet->u0 + uoff, packet->v0 + voff);
         pVertex[1].uv = CVector2(packet->u1 + uoff, packet->v0 + voff);
         pVertex[2].uv = CVector2(packet->u1 + uoff, packet->v1 + voff);
         pVertex[3].uv = CVector2(packet->u0 + uoff, packet->v1 + voff);

         pVertex[0].col = pVertex[1].col = pVertex[2].col = pVertex[3].col = BP_DecodeColorF(packet->rgba); 

         pVertex += 4;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      int const textureAlphaMode = DMAPackShader::gUseTextureAlphaOverride ? DMAPackShader::gUseTextureAlpha : DMAPackShader::kTA_NoAlpha; 
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();

      // Kyle_op in MGS3 perforated screen secret overlay uses special blendmodes
      if( gGS_BlendModeDrawCount == 1 )
      {
         BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, textureAlphaMode, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
      }
      else
      {
         // Pass 0 call uses previously set or restored blend mode
         BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, textureAlphaMode, gGS_ForceRGBWhite ), NULL, 0);
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
         // Pass 1 - gGS_BlendModeDrawCount set alpha mode based on pass manually and reset to pass 0 mode when done
         for( int iBlendPass=1;iBlendPass<gGS_BlendModeDrawCount;++iBlendPass )
         {
            BP_GS_SetAlpha_UseLastBlendMode(iBlendPass);
            BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, textureAlphaMode, gGS_ForceRGBWhite ), NULL, 0);

            gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
         }
         // restore blend mode for future DmaPack draw calls
         BP_GS_SetAlpha_UseLastBlendMode(0);
      }
   }
   return ( &packet[1] );
}

static void *ExecRSprt_F( void *packet_addr )
{
   DG_DMAPACK_RSPRT_F *packet = (DG_DMAPACK_RSPRT_F *)packet_addr;

   DMAPackShader::FlushVertexRegisters();

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * 4);
   if( pVertexBuffer )
   {
      {
         SVertexPosUvCol* pVertex = (SVertexPosUvCol*)pVertexBuffer->Lock();

         pVertex[0].pos = CVector4(packet->x0, packet->y0, 0.0f, 1.0f); 
         pVertex[1].pos = CVector4(packet->x1, packet->y1, 0.0f, 1.0f); 
         pVertex[2].pos = CVector4(packet->x3, packet->y3, 0.0f, 1.0f); 
         pVertex[3].pos = CVector4(packet->x2, packet->y2, 0.0f, 1.0f); 

         real32 uoff = 0.0f, voff = 0.0f;
         if (DMAPackShader::gHasTexture)
         {
            // invert shader uv scaling (see DMAPack.fx)
            real32 const oodu = DMAPackShader::gVertexRegisters.mI2FScale.mZ*DMAPackShader::gVertexRegisters.mUVMulAdd.mX;
            real32 const oodv = DMAPackShader::gVertexRegisters.mI2FScale.mW*DMAPackShader::gVertexRegisters.mUVMulAdd.mY;
            // offset half a texel to correct bilinear filtering
            uoff = 1.0f/(2.0f*oodu*CBaseTexture::GetCurrentTextureWidth(0));
            voff = 1.0f/(2.0f*oodv*CBaseTexture::GetCurrentTextureHeight(0));
         }
         pVertex[0].uv = CVector2(packet->u0 + uoff, packet->v0 + voff);
         pVertex[1].uv = CVector2(packet->u1 + uoff, packet->v0 + voff);
         pVertex[2].uv = CVector2(packet->u1 + uoff, packet->v1 + voff);
         pVertex[3].uv = CVector2(packet->u0 + uoff, packet->v1 + voff);

         pVertex[0].col = pVertex[1].col = pVertex[2].col = pVertex[3].col = BP_DecodeColorF(packet->rgba); 

         pVertex += 4;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      if( DMAPackShader::gUseTextureAlphaOverride )
      {
         BP_BeginShader(DMAPackShader::Shader(DMAPackShader::gHasTexture, DMAPackShader::gUseTextureAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      }
      else
      {
         BP_BeginShader(DMAPackShader::Shader(DMAPackShader::gHasTexture, DMAPackShader::kTA_NoAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      }
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, 4);
   }

   return ( &packet[1] );
}

#endif // DMAPACK_SUPPORTS_FLOAT_PACKETS

static void *ExecLineStrip( void *packet_addr )
{
   DG_DMAPACK_LINESTRIP* packet = (DG_DMAPACK_LINESTRIP*)packet_addr;

   DG_DMAPACK_VERTEX* vertex = (DG_DMAPACK_VERTEX*)&packet[1];

   if( packet->v_count > 1 )
   {
      DMAPackShader::FlushVertexRegisters();

      int const lineCount = packet->v_count - 1;
      int const finalVertexCount = 2 * lineCount;
      CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * finalVertexCount);
      if( pVertexBuffer )
      {
         {
            SVertexPosUvCol* pVertex = (SVertexPosUvCol*)pVertexBuffer->Lock();

            for ( int i = 0; i < lineCount; ++i )
            {
               pVertex->pos = CVector4(vertex->x, vertex->y, 0.0f, 1.0f);
               pVertex->uv = CVector2(vertex->u, vertex->v);
               pVertex->col = BP_DecodeColorF(vertex->rgba); 
               pVertex++;
               vertex++;
               pVertex->pos = CVector4(vertex->x, vertex->y, 0.0f, 1.0f);
               pVertex->uv = CVector2(vertex->u, vertex->v);
               pVertex->col = BP_DecodeColorF(vertex->rgba); 
               pVertex++;
            }
            vertex++;

            pVertexBuffer->Unlock();
         }

         CVertexData vertexData;
         size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
         vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
         vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
         vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

         CShaderVertexDataBinding binding;
         binding.Set(kVDU_Position, kVDS_Position);
         binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
         binding.Set(kVDU_TexCoord1, kVDS_Color0);

         BP_BeginShader(DMAPackShader::Shader( 0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
         gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
         gpRenderBackend->ForceVertexDataRebind();
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineList, 0, finalVertexCount);
      }
      else
      {
         // Skip over all the verts in case this allocation fails!
         vertex += packet->v_count;
      }
   }
   
   return vertex;
}

#if MGS_VERSION == 2
static void *ExecLineStrip_F( void *packet_addr )
{
   DG_DMAPACK_LINESTRIP_F* packet = (DG_DMAPACK_LINESTRIP_F*)packet_addr;

   DG_DMAPACK_VERTEX_F* vertex = (DG_DMAPACK_VERTEX_F*)&packet[1];

   DMAPackShader::FlushVertexRegisters();

   int const lineCount = packet->v_count - 1;
   int const finalVertexCount = 2 * lineCount;
   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * finalVertexCount);

   if( pVertexBuffer )
   {
      {
         SVertexPosUvCol* pVertex = (SVertexPosUvCol*)pVertexBuffer->Lock();

         for ( int i = 0; i < lineCount; ++i )
         {
            pVertex->pos = CVector4(vertex->x, vertex->y, 0.0f, 1.0f);
            pVertex->uv = CVector2(vertex->u, vertex->v);
            pVertex->col = BP_DecodeColorF(vertex->rgba); 
            pVertex++;
            vertex++;
            pVertex->pos = CVector4(vertex->x, vertex->y, 0.0f, 1.0f);
            pVertex->uv = CVector2(vertex->u, vertex->v);
            pVertex->col = BP_DecodeColorF(vertex->rgba); 
            pVertex++;
         }
         vertex++;

         pVertexBuffer->Unlock();
      }

      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
      vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      BP_BeginShader(DMAPackShader::Shader( 0/*isTexture*/, 0/*isTextureAlpha*/, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineList, 0, finalVertexCount);
   }
   else
   {
      // Skip over all the verts in case this allocation fails!
      vertex += packet->v_count;
   }

   return vertex;
}
#endif

static void *ExecTriangleStrip( void *packet_addr )
{
   DG_DMAPACK_TRIANGLESTRIP* packet = (DG_DMAPACK_TRIANGLESTRIP*)packet_addr;
   DG_DMAPACK_VERTEX* vertex = (DG_DMAPACK_VERTEX*)&packet[1];

//    for ( i = packet->v_count ; i > 2 ; i--, vertex++ ){
//       buffer = (void*)scrpad->dma_buffer ;
//       buffer->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(*buffer)-1 );
//       buffer->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
//       buffer->dmatag.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x00, 2*3, VIF_VERT_PACK, 0 );
//       buffer->verts[0].x = vertex[0].x ;
//       buffer->verts[0].y = vertex[0].y ;
//       buffer->verts[0].u = vertex[0].u ;
//       buffer->verts[0].v = vertex[0].v ;
//       buffer->verts[0].rgba = COLCONV_32_64( vertex[0].rgba );
//       buffer->verts[1].x = vertex[1].x ;
//       buffer->verts[1].y = vertex[1].y ;
//       buffer->verts[1].u = vertex[1].u ;
//       buffer->verts[1].v = vertex[1].v ;
//       buffer->verts[1].rgba = COLCONV_32_64( vertex[1].rgba );
//       buffer->verts[2].x = vertex[2].x ;
//       buffer->verts[2].y = vertex[2].y ;
//       buffer->verts[2].u = vertex[2].u ;
//       buffer->verts[2].v = vertex[2].v ;
//       buffer->verts[2].rgba = COLCONV_32_64( vertex[2].rgba );
//       buffer->vifcode2[0] = SCE_VIF1_SET_MSCAL( scrpad->poly_prog_entry, 0 );
//       buffer->vifcode2[1] = SCE_VIF1_SET_NOP( 0 );
//       buffer->vifcode2[2] = SCE_VIF1_SET_NOP( 0 );
//       buffer->vifcode2[3] = SCE_VIF1_SET_NOP( 0 );
//       DG_SendCacheFIFO( scrpad->local_work, buffer, SIZEOF_QWORD(*buffer) );
//    }

   if( packet->v_count > 0 )
   {
      DMAPackShader::FlushVertexRegisters();

      CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * packet->v_count);
      if( pVertexBuffer )
      {
         {
            SVertexPosUvCol* pVertex = (SVertexPosUvCol*)pVertexBuffer->Lock();

            for ( int i = 0; i < packet->v_count; ++i )
            {
               pVertex->pos = CVector4(vertex->x, vertex->y, 0.0f, 1.0f);
               pVertex->uv = CVector2(vertex->u, vertex->v);
               pVertex->col = BP_DecodeColorF(vertex->rgba); 
               pVertex++;
               vertex++;
            }

            pVertexBuffer->Unlock();
         }

         CVertexData vertexData;
         size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
         vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
         vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
         vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

         CShaderVertexDataBinding binding;
         binding.Set(kVDU_Position, kVDS_Position);
         binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
         binding.Set(kVDU_TexCoord1, kVDS_Color0);

         if( DMAPackShader::gUseTextureAlphaOverride )
         {
            BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, DMAPackShader::gUseTextureAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
         }
         else
         {
            BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, DMAPackShader::kTA_NoAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
         }
         gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
         gpRenderBackend->ForceVertexDataRebind();
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleStrip, 0, packet->v_count);
      }
      else
      {
         // Skip over all the verts in case this allocation fails!
         vertex += packet->v_count;
      }
   }

   return vertex;
}

static void *ExecAlpha( void *packet_addr )
{
   DG_DMAPACK_ALPHA* packet = (DG_DMAPACK_ALPHA*)packet_addr;

   //BP_GS_SetAlpha(SCE_GS_SET_ALPHA(0, 2, 0, 1, 0));
   BP_GS_SetAlpha((uint64)(packet->alpha0) | ((uint64)(packet->alpha1) << 32));
   return ( &packet[1] );
}

static void *ExecTexture( void *packet_addr )
{
   DG_DMAPACK_TEXTURE_BP* packet = (DG_DMAPACK_TEXTURE_BP*)packet_addr;

   if( packet->hasvalidTex )
   {
      DG_TEX* tex = &packet->tex;
      BP_GS_SetAlpha(tex->tex_trans.alpha.data);

      if( tex->BP_TextureHandle )
      {
         BP_DG_SetTexture(tex, 0);
      }
      else if( tex->tri_id != 0 )
      {
         // If we have a valid tri_id but no texture we display a white texture to indicate this.
         gpRenderBackend->SetTexture(0, &gpRenderBackend->GetWhiteMap());
      }
      else
      {
         // If no tri_id and tex_id is specific this means this is just a proxy texture to set up some manually loaded into vram memory and we probably need to fix this!
         gpRenderBackend->SetTexture(0, DMAPackShader::gDynamicPlaceholderTexture);
      }

      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
      BP_GS_SetClamp(0, tex->tex_trans.clamp.data);

#if MGS_VERSION == 2
      if (tex->BP_flag & DG_TEXFLAG_UV_CLAMP)
      {
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      }
#endif

      DMAPackShader::gHasTexture = 1;
      if( tex->BP_flag & DG_TEXFLAG_RENDERTARGET )
      {
         DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
      }
      else
      {
         DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_Raw;
      }

#if MGS_VERSION == 2
      real32 const oneOverUScale = 1.0f / tex->u_scale;
      real32 const oneOverVScale = 1.0f / tex->v_scale;

      DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(oneOverUScale, oneOverVScale, -tex->u_offset * oneOverUScale, -tex->v_offset * oneOverVScale);

#elif MGS_VERSION == 3
      int width, height, offsetX, offsetY;
      DG_GetTexelInfo(&width, &height, &offsetX, &offsetY, tex);

      real32 const oneOverWidth = 1.0f / width;
      real32 const oneOverHeight = 1.0f / height;
      DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(oneOverWidth, oneOverHeight, -offsetX * oneOverWidth, -offsetY * oneOverHeight);
#endif
   }
   else
   {
      DMAPackShader::gHasTexture = 0;
      DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
   }

   return ( &packet[1] );
}


static void *ExecMode( void *packet_addr )
{
   DG_DMAPACK_PARAM* packet = (DG_DMAPACK_PARAM*)packet_addr;

   if ( packet->param & DG_DMAPACK_MODE_NO_BILINEAR )
   {
      BP_TRIVIAL_BREAK;
      //       DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
      //       DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
   }
   else 
   {
      BP_TRIVIAL_BREAK;
      //       DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
      //       DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
   }

   if ( packet->param & DG_DMAPACK_MODE_NO_ALPHA )
   {
      BP_GS_SetAlpha(0);
   }

   if ( packet->param & DG_DMAPACK_MODE_NO_ALPHATEST )
   {
      BP_GS_SetAlphaTestEnabled( false );
//      gpRenderBackend->SetAlphaTestEnable(false);
   }
   else
   {
      BP_GS_SetAlphaTestEnabled( true );
//      gpRenderBackend->SetAlphaTestEnable(true);
   }

   return ( &packet[1] );
}

#if MGS_VERSION == 2
static void * ExecTextureDyn01MGS2( void *packet_addr )
{
   DG_DMAPACK_TEXTURE_DYNAMIC01_BP* packet = (DG_DMAPACK_TEXTURE_DYNAMIC01_BP*)packet_addr;
   
   CBaseTexture* pTexture = (CBaseTexture*)packet->bp_tex;
   if( pTexture )
   {
      gpRenderBackend->SetTexture(0, pTexture);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);

      DMAPackShader::gHasTexture = 1;
      DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_Raw;
      DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
   }
   else
   {
      DMAPackShader::gHasTexture = 0;
      DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
   }
   return &packet[1];
}

static void *ExecTextureLiner( void *packet_addr )
{
   DG_DMAPACK_TEXTURE_LINEAR_BP* packet = (DG_DMAPACK_TEXTURE_LINEAR_BP*)packet_addr;

   if( packet->hasvalidTex )
   {
      DG_TEX_LIN* tex = &packet->tex;
      BP_SetLinerTexture(tex, 0);
      DMAPackShader::gHasTexture = 1;
      DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_Raw;
      DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
   }
   else
   {
      DMAPackShader::gHasTexture = 0;
      DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
   }

   return ( &packet[1] );
}

static void *ExecWindow( void *packet_addr )
{
   DG_DMAPACK_WINDOW* packet = (DG_DMAPACK_WINDOW*)packet_addr;
   BP_TRIVIAL_BREAK;
   return ( &packet[1] );
}

static void *ExecSetZ( void *packet_addr )
{
   DG_DMAPACK_PARAM* packet = (DG_DMAPACK_PARAM*)packet_addr;
   
   real32 const z = *((real32*)&packet->param); 

   DMAPackShader::gVertexRegisters.mHScale[2] = 0.0f;
   DMAPackShader::gVertexRegisters.mHOffset[2] = z;

   return ( &packet[1] );
}

static void *ExecEnable( void *packet_addr )
{
   DG_DMAPACK_PARAM* packet = (DG_DMAPACK_PARAM*)packet_addr;

   if ( packet->param & DG_DMAPACK_MODE_NO_BILINEAR )
   {
      BP_MARCO_BREAK;
//       DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
//       DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
   }

   if ( packet->param & DG_DMAPACK_MODE_NO_ALPHA )
   {
      BP_MARCO_BREAK;
//       DG_SetAlphaMode( 0 );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_NO_ALPHATEST )
   {
//      gpRenderBackend->SetAlphaTestEnable(false);
      BP_GS_SetAlphaTestEnabled( false );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_MODULATE1X )
   {
      BP_MARCO_BREAK;
//       DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
//       DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_ZTEST_REV )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_NO_ZOVERWRITE )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_NO_ZTEST )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_COLORMASK )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState( D3DRS_COLORWRITEENABLE , 0 );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_NO_TEXALPHA )
   {
      DMAPackShader::gUseTextureAlphaOverride = 0;
   }

   return ( &packet[1] );
}

static void *ExecDisable( void *packet_addr )
{
   DG_DMAPACK_PARAM* packet = (DG_DMAPACK_PARAM*)packet_addr;

   if ( packet->param & DG_DMAPACK_MODE_NO_BILINEAR )
   {
      BP_MARCO_BREAK;
//       DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
//       DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_NO_ALPHA )
   {
   }

   if ( packet->param & DG_DMAPACK_MODE_NO_ALPHATEST )
   {
      BP_GS_SetAlphaTestEnabled( true );
//      gpRenderBackend->SetAlphaTestEnable(true);
   }
   
   if ( packet->param & DG_DMAPACK_MODE_MODULATE1X )
   {
      BP_MARCO_BREAK;
//       DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
//       DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_ZTEST_REV )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
   }
   if ( packet->param & DG_DMAPACK_MODE_NO_ZOVERWRITE )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_NO_ZTEST )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
   }
   
   if ( packet->param & DG_DMAPACK_MODE_COLORMASK )
   {
      BP_MARCO_BREAK;
//       DG_SetRenderState( D3DRS_COLORWRITEENABLE , D3DCOLORWRITEENABLE_ALL );
   }
   
   if ( packet->param & DG_DMAPACK_MODE_NO_TEXALPHA )
   {
      DMAPackShader::gUseTextureAlphaOverride = 1;
   }

   return ( &packet[1] );
}

#if BP_VITA
namespace FX
{
   extern CBaseTexture *pHalfresTexture;
};

// hack for now, just point at the lowres one we copied, praying this is only used for menu shrinks
static void *ExecUseFrameTex( void *packet_addr )
{
   DG_DMAPACK_PARAM* packet = (DG_DMAPACK_PARAM*)packet_addr;

   CBaseTexture* pTexture = NULL;

   if ( packet->param != -1 )
   {
      switch ( packet->param )
      {
      case 0:
         //24bit
         pTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
         DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
         DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
         break;

     case 1:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     default:
     case 2:
        //24bit
        pTexture = gpRenderBackend->GetLowresDrawTexture(); 
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
      }

      gpRenderBackend->SetTexture(0, pTexture);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      DMAPackShader::gHasTexture = 1;

      if (packet->param == 2)
      {
         gpRenderBackend->SetTexture(1, FX::pHalfresTexture);
         gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         DMAPackShader::gHasTexture = 2;
      }
   } 
   else 
   {
      DMAPackShader::gHasTexture = 0;
   }

   return ( &packet[1] );
}
#else
static void *ExecUseFrameTex( void *packet_addr )
{
   DG_DMAPACK_PARAM* packet = (DG_DMAPACK_PARAM*)packet_addr;

#ifndef BP_RENDER_SINGLE_RENDER_TARGET
   CBaseTexture* pTexture = NULL;

   if ( packet->param != -1 )
   {
      switch ( packet->param )
      {
      case 0:
         //24bit
         pTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
         DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
         DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
         break;

     case 1:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     default:
     case 2:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f, 1.0f, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
      }

      gpRenderBackend->SetTexture(0, pTexture);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      DMAPackShader::gHasTexture = 1;

   } 
   else 
#endif
   {
      DMAPackShader::gHasTexture = 0;
   }

   return ( &packet[1] );
}
#endif

static void *ExecBackupFrame( void *packet_addr )
{
   DG_DMAPACK_PARAM* packet = (DG_DMAPACK_PARAM*)packet_addr;

#if BP_VITA
   extern void BP_PostFx_Offscreen(void *pData);
   // Force a scene flush if we're in the lowres or halfres scene
   BP_PostFx_Offscreen(NULL);
#endif

#ifndef BP_RENDER_SINGLE_RENDER_TARGET

#  if BPE_TARGET == BPE_TARGET_X360
   // Platform specific version so we can take advantage of the direct resolve current frame buffer into final target texture feature.
   CBaseTexture* pDestTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);

   if(packet->param & 1)
   {
      CBaseTexture* pSourceTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);

      // Store original render target
      SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

      BP_CopyTexture(pSourceTexture, pDestTexture);
      gpRenderBackend->ResolveRenderTarget(pDestTexture);

      // Restore original render target
      // NOTE: TempBuffer does not overlap with the main buffer, so we don't have to restore what this copy operation just trashed.
      gpRenderBackend->SetRenderTarget(originalRenderTarget);

   }
   else
   {
      // Directly resolve current buffer into temp buffer.
      gpRenderBackend->ResolveRenderTarget(pDestTexture);
   }
#  else

   CBaseTexture* pSourceTexture = NULL;

   if(packet->param & 1)
      pSourceTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
   else
      pSourceTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);

   // Store original render target
   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   CBaseTexture* pDestTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
   BP_CopyTexture(pSourceTexture, pDestTexture);

   // Restore original render target
   gpRenderBackend->SetRenderTarget(originalRenderTarget);
#  endif

#endif

   return ( &packet[1] );
}

#endif // MGS_VERSION==2

static void *ExecTextureDynamic( void *packet_addr )
{
   DG_DMAPACK_TEXTURE_DYNAMIC_BP* packet = (DG_DMAPACK_TEXTURE_DYNAMIC_BP*)packet_addr;
   DG_TEX* tex = &packet->tex;

   BP_GS_SetAlpha(tex->tex_trans.alpha.data);

   CBaseTexture const * pTexture = (CBaseTexture const *)packet->bp_tex;

   gpRenderBackend->SetTexture(0, pTexture);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
   BP_GS_SetClamp(0, tex->tex_trans.clamp.data);

   DMAPackShader::gHasTexture = 1;
   if( tex->BP_flag & DG_TEXFLAG_RENDERTARGET )
   {
      DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
   }
   else
   {
      DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_Raw;
   }


   int width, height, offsetX, offsetY;

   width = pTexture->GetWidth();
   height = pTexture->GetHeight();
   offsetX = 0;
   offsetY = 0;

   real32 const oneOverWidth = 1.0f / width;
   real32 const oneOverHeight = 1.0f / height;
   DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(oneOverWidth, oneOverHeight, -offsetX * oneOverWidth, -offsetY * oneOverHeight);

   return ( &packet[1] );
}

static void *ExecViewMapping( void *packet_addr )
{
   DG_DMAPACK_VIEWMAPPING* packet = (DG_DMAPACK_VIEWMAPPING*)packet_addr;

   //BP_DebugText_Print("DmaPack ViewMapping: x0:%f y0:%f x1:%f y1:%f", packet->x0, packet->y0, packet->x1, packet->y1);

   DMAPackShader::gVertexRegisters.mHScale[0] = 2.0f / (packet->x1 - packet->x0);
   DMAPackShader::gVertexRegisters.mHScale[1] = -2.0f / (packet->y1 - packet->y0);
   DMAPackShader::gVertexRegisters.mHOffset[0] = ( packet->x1 + packet->x0 ) * -0.5f * DMAPackShader::gVertexRegisters.mHScale[0];
   DMAPackShader::gVertexRegisters.mHOffset[1] = ( packet->y1 + packet->y0 ) * -0.5f * DMAPackShader::gVertexRegisters.mHScale[1];

   return ( (char *) packet_addr + sizeof( DG_DMAPACK_VIEWMAPPING ) );
}

#if MGS_VERSION == 3

static void *ExecTriangleFan( void *packet_addr )
{
   DG_DMAPACK_TRIANGLEFAN* packet = (DG_DMAPACK_TRIANGLEFAN*)packet_addr;
   DG_DMAPACK_VERTEX* vertex = (DG_DMAPACK_VERTEX*)&packet[1];

   if( packet->v_count > 0 )
   {
      DMAPackShader::FlushVertexRegisters();

      CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosUvCol) * packet->v_count);
      if( pVertexBuffer )
      {
         {
            SVertexPosUvCol* pVertex = (SVertexPosUvCol*)pVertexBuffer->Lock();

            for ( int i = 0; i < packet->v_count; ++i )
            {
               pVertex->pos = CVector4(vertex->x, vertex->y, 0.0f, 1.0f);
               pVertex->uv = CVector2(vertex->u, vertex->v);
               pVertex->col = BP_DecodeColorF(vertex->rgba); 
               pVertex++;
               vertex++;
            }

            pVertexBuffer->Unlock();
         }

         CVertexData vertexData;
         size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosUvCol ) );
         vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosUvCol_POD, pos), kVDT_Float3, bufferIndex);
         vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexPosUvCol_POD, uv), kVDT_Float2, bufferIndex);
         vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosUvCol_POD, col), kVDT_Float4, bufferIndex);

         CShaderVertexDataBinding binding;
         binding.Set(kVDU_Position, kVDS_Position);
         binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
         binding.Set(kVDU_TexCoord1, kVDS_Color0);

         if( DMAPackShader::gUseTextureAlphaOverride )
         {
            BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, DMAPackShader::gUseTextureAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
         }
         else
         {
            BP_BeginShader(DMAPackShader::Shader( DMAPackShader::gHasTexture, DMAPackShader::kTA_NoAlpha, 0/*gGS_ForceRGBWhite*/ ), NULL, 0);
         }
         gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
         gpRenderBackend->ForceVertexDataRebind();
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, packet->v_count);
      }
      else
      {
         // Skip over all the verts in case this allocation fails!
         vertex += packet->v_count;
      }
   }

   return vertex;

}
static void *ExecViewport( void *packet_addr )
{
   DG_DMAPACK_VIEWPORT* packet = (DG_DMAPACK_VIEWPORT*)packet_addr;

//    buffer->scale.vx = packet->w / 2.0f ;
//    buffer->scale.vy = packet->h / 2.0f ;
//    buffer->scale.vz = DRAW_Z_MAX - DRAW_Z_MIN ;
//    buffer->offset.vx = 2048 + ( packet->x + buffer->scale.vx - scrpad->current_viewport_w / 2 );
//    buffer->offset.vy = 2048 + ( packet->y + buffer->scale.vy - scrpad->current_viewport_h / 2 );
//    buffer->offset.vz = DRAW_Z_MIN ;
//   buffer->vifcode2[3] = SCE_VIF1_SET_MSCAL( Vu1Draw2DPackets_Func[ CMD_SetProjectionParam ], 0 );

   if( packet->x != 0 || packet->y != 0 || packet->w != 512 || packet->h != 448 )
   {
      SRenderTarget target = gpRenderBackend->GetCurrentRenderTarget();

      //BP_DebugText_Print("DmaPack Viewport: x:%d y:%d w:%d h:%d", packet->x, packet->y, packet->w, packet->h);

      int const backWidth = target.mpColorBuffer[0] ? target.mpColorBuffer[0]->GetWidth() : gpRenderBackend->GetBackBufferWidth();
      int const backHeight = target.mpColorBuffer[0] ? target.mpColorBuffer[0]->GetHeight() : gpRenderBackend->GetBackBufferHeight();

      target.mRenderTargetViewPort.mViewportEnabled = 1;
      target.mRenderTargetViewPort.mViewportX = packet->x * backWidth / DRAW_WIDTH;
      target.mRenderTargetViewPort.mViewportY = packet->y * backHeight / DRAW_HEIGHT;
      target.mRenderTargetViewPort.mViewportWidth = bpe::max_val(1, packet->w * backWidth / DRAW_WIDTH);
      target.mRenderTargetViewPort.mViewportHeight = bpe::max_val(1, packet->h * backHeight / DRAW_HEIGHT);

      gpRenderBackend->SetRenderTarget(target);
   }

   DMAPackShader::gVertexRegisters.mPScale = CVector4(1.0f, 1.0f, 1.0f, 1.0f);
   DMAPackShader::gVertexRegisters.mPOffset = CVector4(0.0f, 0.0f, 0.0f, 0.0f);

   return ( &packet[1] );
}

static void *ExecCoordinateScale( void *packet_addr )
{
   DG_DMAPACK_COORDINATESCALE* packet = (DG_DMAPACK_COORDINATESCALE*)packet_addr;

//    buffer->param[0] = packet->vertex_scale ;
//    buffer->param[1] = packet->tex_scale ;
//    buffer->vifcode2[0] = SCE_VIF1_SET_NOP( 0 );
//    buffer->vifcode2[1] = SCE_VIF1_SET_MSCAL( Vu1Draw2DPackets_Func[ CMD_SetCoordinateScaleParam ], 0 );

   DMAPackShader::gVertexRegisters.mI2FScale = CVector4(packet->vertex_scale_x, packet->vertex_scale_y, packet->tex_scale, packet->tex_scale);

   return ( &packet[1] );
}

static void *ExecLoadTexture( void *packet_addr )
{
   DG_DMAPACK_LOADTEXTURE* packet = (DG_DMAPACK_LOADTEXTURE*)packet_addr;

   return ( &packet[1] );
}

static void *ExecLoadImage( void *packet_addr )
{
   DG_DMAPACK_LOADIMAGE* packet = (DG_DMAPACK_LOADIMAGE*)packet_addr;

#if MARCO
   BP_DebugText_Print("DmaPack: LoadImage executed.");
#endif

   return ( &packet[1] );
}

static void * ExecRenderBuffer( void *packet_addr )
{
   DG_DMAPACK_RENDERBUFFER* packet = (DG_DMAPACK_RENDERBUFFER*)packet_addr;

#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   switch( packet->page )
   {
   default:
   case 0:
      {
         BP_SetMainRenderTarget(BP_GetCurrentRenderBuffer());
      }
      break;

   case 1:
      {
         BP_SetMainRenderTarget(1 - BP_GetCurrentRenderBuffer());
      }
      break;

   case 2: //fmt = FRAME_BUFFER_COLOR_MODE
   case 3: //fmt = Z_BUFFER_COLOR_MODE
      {
         gpRenderBackend->ResolveRenderTargetPredicated(gpRenderBackend->GetCurrentRenderTexture());
         CBaseTexture* pRenderTargetColorTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
         SRenderTarget renderTarget(pRenderTargetColorTexture, NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA));
         gpRenderBackend->SetRenderTarget(renderTarget);

#if BPE_TARGET == BPE_TARGET_X360
         SRectf SrcDestRect;
         SrcDestRect.x1 = SrcDestRect.y1 = 0.0f;
         SrcDestRect.x2 = SrcDestRect.y2 = 1.0f;
         gpRenderBackend->RestoreRenderTargetColor_Internal(pRenderTargetColorTexture, pRenderTargetColorTexture, SrcDestRect, SrcDestRect);
#endif
      }
      break;

   case 4:
      {
         gpRenderBackend->ResolveRenderTargetPredicated(gpRenderBackend->GetCurrentRenderTexture());
         //gpRenderBackend->ResolveDepthTargetPredicated(gpRenderBackend->GetCurrentRenderTarget().mpDepthBuffer);//TODO:
         CBaseTexture* pRenderTargetColorTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
         CBaseTexture* pRenderTargetDepthTexture = BP_GetRenderTarget(kRT_TempDepthBuffer_NoMSAA);
         SRenderTarget renderTarget(pRenderTargetColorTexture, NULL, NULL, NULL, pRenderTargetDepthTexture);
         gpRenderBackend->SetRenderTarget(renderTarget);

#if BPE_TARGET == BPE_TARGET_X360
         //TODO: this has to restore depth buffer too, need new shader to do this properly
         SRectf SrcDestRect;
         SrcDestRect.x1 = SrcDestRect.y1 = 0.0f;
         SrcDestRect.x2 = SrcDestRect.y2 = 1.0f;
         gpRenderBackend->RestoreRenderTargetColor_Internal(pRenderTargetColorTexture, pRenderTargetColorTexture, SrcDestRect, SrcDestRect);
#endif
      }
      break;
   }

#endif

   return ( &packet[1] );
}

static void * ExecRenderBuffer_BP( void *packet_addr )
{
   DG_DMAPACK_RENDERBUFFER* packet = (DG_DMAPACK_RENDERBUFFER*)packet_addr;

#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   ERenderTarget const renderTarget = (ERenderTarget)packet->page;
   
   gpRenderBackend->ResolveRenderTargetPredicated(gpRenderBackend->GetCurrentRenderTexture());

   CBaseTexture* pTexture = BP_GetRenderTarget(renderTarget);
   gpRenderBackend->SetRenderTarget(SRenderTarget(pTexture, NULL, NULL, NULL, BP_GetRenderTarget(kRT_DefaultDepthBuffer)));

#  if BPE_TARGET == BPE_TARGET_X360
   BP_DrawFullscreenTexture(pTexture, 0, 0);//X360 needs to restore EDRAM to this texture
#  endif
#endif

   return ( &packet[1] );
}


static void *ExecDrawZ( void *packet_addr )
{
   DG_DMAPACK_DRAWZ* packet = (DG_DMAPACK_DRAWZ*)packet_addr;

   DMAPackShader::gVertexRegisters.mHScale[2] = 0.0f;
   DMAPackShader::gVertexRegisters.mHOffset[2] = packet->z;

   return ( &packet[1] );
}

//    buffer->param.vx = packet->x ;
//    buffer->param.vy = packet->y ;
//    buffer->param.vz = s ;
//    buffer->param.vw = c ;
//    buffer->scale = scale ;
//    buffer->vifcode2[0] = SCE_VIF1_SET_MSCAL( Vu1Draw2DPackets_Func[ CMD_SetMatrixParam ], 0 );

// VU:

//    addx.z		vf20,vf00,vf00		lq.xyzw		vf16,0x00(vi13)					; x y offset
//    addx.z		vf21,vf00,vf00		lq.xyzw		vf17,0x01(vi13)					; sin cos
//    nop								   lq.xyzw		vf18,0x02(vi13)					; scale
//    nop								   lq.xyzw		vf01,FV_I2FSCALE(vi00)			; 
//    addy.x		vf20,vf00,vf17		nop											; 
//    addx.y		vf20,vf00,vf17		nop											; 
//    subx.x		vf21,vf00,vf17		nop											; 
//    addy.y		vf21,vf00,vf17		nop											; 

// vf20.x = cos
// vf20.y = sin
// vf20.z = 0
// vf21.x = -sin
// vf21.y = cos
// vf21.z = 0

//    mulx.w		vf23,vf00,vf18		nop											; FV_MAT3.w|...scale...?
//    mul.xy		vf23,vf16,vf01		nop											; 
//    nop								   sq.xyz		vf20,FV_MAT0(vi00)				; 
//    nop								   sq.xyz		vf21,FV_MAT1(vi00)				; 
//    nop								   sq.xyw		vf23,FV_MAT3(vi00)				; 
//    nop								   bal			vi15,setup						; 

// setup:
//    nop								lq.xyzw		vf08,FV_HSCALE(vi00)			; 
//    nop								lq.xyzw		vf09,FV_HOFFSET(vi00)			; 
//    nop								lq.xyzw		vf04,FV_MAT0(vi00)				; 
//    nop								lq.xyzw		vf05,FV_MAT1(vi00)				; 
//    nop								lq.xyzw		vf07,FV_MAT3(vi00)				; 
//    mul.xyz		vf04,vf04,vf08		lq.xyzw		vf01,FV_I2FSCALE(vi00)			; 
//    mul.xyz		vf05,vf05,vf08		lq.xyzw		vf02,FV_PSCALE(vi00)			; 
//    mula.xyz	ACC ,vf07,vf08		lq.xyzw		vf03,FV_POFFSET(vi00)			; 
//    maddw.xyz	vf07,vf09,vf00		jr			vi15							; 
//    mulw.xy		vf06,vf08,vf07		lq.xyzw		vf10,FV_BASECOL(vi00)			; 

static void *ExecOriginMatrix( void *packet_addr )
{
   DG_DMAPACK_ORIGINMATRIX* packet = (DG_DMAPACK_ORIGINMATRIX*)packet_addr;

   real32 const angle = packet->rot / 2048.0f * M_PI ;

   real32 const sin = sinf(angle);
   real32 const cos = cosf(angle);

   real32 const offsetX = packet->x;
   real32 const offsetY = packet->y;

   real32 const scale = packet->scale / 256.0f ;

   DMAPackShader::gVertexRegisters.mMat.Set00(cos * scale);
   DMAPackShader::gVertexRegisters.mMat.Set01(sin * scale);
   DMAPackShader::gVertexRegisters.mMat.Set10(-sin * scale);
   DMAPackShader::gVertexRegisters.mMat.Set11(cos * scale);

   DMAPackShader::gVertexRegisters.mMat.Set03(offsetX);
   DMAPackShader::gVertexRegisters.mMat.Set13(offsetY);

   return ( &packet[1] );
}

static void *ExecBaseColor( void *packet_addr )
{
   DG_DMAPACK_BASECOLOR* packet = (DG_DMAPACK_BASECOLOR*)packet_addr;
   DMAPackShader::gVertexRegisters.mBaseCol = BP_DecodeColorF(packet->rgba);

   return ( &packet[1] );
}

#if BP_VITA
static void *ExecFrameTexture( void *packet_addr )
{
   DG_DMAPACK_FRAMETEXTURE* packet = (DG_DMAPACK_FRAMETEXTURE*)packet_addr;

   CBaseTexture const * pTexture = NULL;

   switch ( packet->page )
   {
     case 0:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     case 1:
     default:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     case 2:
        //24bit
        pTexture = gpRenderBackend->GetLowresDrawTexture();
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;
#if 0
     case 3:
        //16bit
        pTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     case 4:
        pTexture = BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        break;

     case 5:
        pTexture = BP_GetRenderTarget(kRT_TempDepthBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        break;

     case 0x100:
        //32bit
        pTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
        break;

     case 0x101:
        //32bit
        pTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
        break;

     case 0x102:
        //32bit
        pTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
        break;
#endif
   }

   if ( pTexture )
   {
      gpRenderBackend->SetTexture(0, pTexture);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }

   DMAPackShader::gHasTexture = 1;

   return ( &packet[1] );
}
#else
static void *ExecFrameTexture( void *packet_addr )
{
   DG_DMAPACK_FRAMETEXTURE* packet = (DG_DMAPACK_FRAMETEXTURE*)packet_addr;

   CBaseTexture const * pTexture = NULL;

#ifdef BP_RENDER_SINGLE_RENDER_TARGET
   pTexture = &gpRenderBackend->GetBlackMap();
#else

   switch ( packet->page )
   {
     case 0:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     case 1:
     default:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     case 2:
        //24bit
        pTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     case 3:
        //16bit
        pTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;
        break;

     case 4:
        pTexture = BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        break;

     case 5:
        pTexture = BP_GetRenderTarget(kRT_TempDepthBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        break;

     case 0x100:
        //32bit
        pTexture = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
        break;

     case 0x101:
        //32bit
        pTexture = BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
        break;

     case 0x102:
        //32bit
        pTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
        DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
        DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_RenderTarget;
        break;
   }

#endif

   if ( pTexture )
   {
      gpRenderBackend->SetTexture(0, pTexture);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }

   DMAPackShader::gHasTexture = 1;

   return ( &packet[1] );
}
#endif
static void *ExecFrameTexture_BP( void *packet_addr )
{
   DG_DMAPACK_FRAMETEXTURE* packet = (DG_DMAPACK_FRAMETEXTURE*)packet_addr;

   ERenderTarget const renderTarget = (ERenderTarget)packet->page;

   CBaseTexture* pTexture = BP_GetRenderTarget(renderTarget);

   gpRenderBackend->SetTexture(0, pTexture);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   DMAPackShader::gHasTexture = 1;

   DMAPackShader::gVertexRegisters.mUVMulAdd = CVector4(1.0f / DRAW_WIDTH, 1.0f / DRAW_HEIGHT, 0.0f, 0.0f);
   DMAPackShader::gUseTextureAlpha = DMAPackShader::kTA_NoAlpha;

   return ( &packet[1] );
}

static void *ExecTest( void *packet_addr )
{
   DG_DMAPACK_TEST* packet = (DG_DMAPACK_TEST*)packet_addr;

   BP_GS_SetTest(packet->test);
   BP_GS_SetupAlphaTestSinglePass(false);

   return ( &packet[1] );
}

static void *ExecOffset( void *packet_addr )
{
   DG_DMAPACK_OFFSET* packet = (DG_DMAPACK_OFFSET*)packet_addr;

   if( packet->offset_x != 0 || packet->offset_y != 0 )
      BP_BREAK;

   return ( &packet[1] );
}

static void *ExecFilter( void *packet_addr )
{
   DG_DMAPACK_FILTER* packet = (DG_DMAPACK_FILTER*)packet_addr;

   switch ( packet->filter )
   {
     case 0:
        // Mip: None, Min: Nearest, Mag: Nearest
        gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
        break;
     
     default:
        // Mip: None, Min: Linear, Mag: Linear
        gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear_Linear, CRenderBackend::kFM_Linear);
        break;
   }

   return ( &packet[1] );
}

static void *ExecAntiAlias( void *packet_addr )
{
   DG_DMAPACK_ANTIALIAS* packet = (DG_DMAPACK_ANTIALIAS*)packet_addr;

   return ( &packet[1] );
}

#endif

static void* ExecCallback(void* packet_addr)
{
   DG_DMAPACK_CALLBACK* packet = (DG_DMAPACK_CALLBACK*)packet_addr;
   void* data = (char*)packet + sizeof(DG_DMAPACK_CALLBACK);

   packet->callback(data);

   return (char*)data + packet->dataSize;
}
//----------------------------------------------------------------------------

extern int gFXShowPasses;
#if JADEBUG
extern "C" char *BP_SplatBasename(char *buf, char *fn);
#endif

void BP_RenderDmaPack_AutoPacket(char* pPacketData)
{
   SBP_RenderDmaPack* pPacket = (SBP_RenderDmaPack*)pPacketData;

   DG_DMAPACK *dmapack = (DG_DMAPACK*)pPacket->dmapack;

   {
      static char base[1024];
      char *buf = base;
      const char *fn;
      int len;
#if MGS_VERSION == 2
      if (dmapack->phase > 0x10)
#else
      if (dmapack->phase > DG_DMAPACK_PHASE_MAX)
#endif
      {
         BPE_GPU_PUSH_PROFILE_MARKER("Bogus Dmapack");
      }
      else
      {
         buf += sprintf(buf, "DmaPack phase %d prio %d file ", dmapack->phase, dmapack->priority);
#if JADEBUG   
         if (dmapack->fname)
            buf = BP_SplatBasename(buf, dmapack->fname);
#endif
         *buf++ = 0;
         BPE_GPU_PUSH_PROFILE_MARKER(base);
         if (gFXShowPasses)
            BP_DebugText_Puts(base);
      }
   }

   SRenderTarget originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   gpRenderBackend->SetDepthCompareEnabled(true);
   BP_GS_SetupAlphaTestSinglePass(false);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

#if BPE_TARGET == BPE_TARGET_X360
   gpRenderBackend->GetDirect3DDevice()->SetRenderState(D3DRS_LASTPIXEL, FALSE);
#endif

   DMAPackShader::gHasTexture = 0;

   // Restore "Z" offset back to default at beginning of auto packet.
   DMAPackShader::gVertexRegisters.mHScale[2] = 0.0f;
   DMAPackShader::gVertexRegisters.mHOffset[2] = 1.0f;
   
#if MGS_VERSION == 2
   // Set HScale based on current viewport
   if(originalRenderTarget.mRenderTargetViewPort.mViewportEnabled)
   {
      DMAPackShader::gVertexRegisters.mHScale[0] = 2.0f / (originalRenderTarget.mRenderTargetViewPort.mViewportWidth*512.0f/float(gpRenderBackend->GetBackBufferWidth() ));
      DMAPackShader::gVertexRegisters.mHScale[1] = -2.0f / (originalRenderTarget.mRenderTargetViewPort.mViewportHeight*448.0f/float(gpRenderBackend->GetBackBufferHeight() ));
   }
   else
   {
      int backBufferWidth = gpRenderBackend->GetBackBufferWidth();
      int backBufferHeight = gpRenderBackend->GetBackBufferHeight();

#ifdef BP_VITA
      // BP_VITA_TODO
      int colorBufferWidth = backBufferWidth;
      int colorBufferHeight = backBufferHeight;
#else
      int colorBufferWidth = originalRenderTarget.mpColorBuffer[0]->GetWidth();
      int colorBufferHeight = originalRenderTarget.mpColorBuffer[0]->GetHeight();
#endif

      DMAPackShader::gVertexRegisters.mHScale[0] = 2.0f / (colorBufferWidth*512.0f/float( backBufferWidth ) );
      DMAPackShader::gVertexRegisters.mHScale[1] = -2.0f / (colorBufferHeight*448.0f/float( backBufferHeight ) );
   }
#endif

   for(DG_DMAPACK_TAG* header = (DG_DMAPACK_TAG*)pPacket->data; header->cmd != DG_DMAPACK_CMD_END; )
   {
      switch(header->cmd)
      {
      case DG_DMAPACK_CMD_POINT:
         header = (DG_DMAPACK_TAG*)ExecPoint(header);
         break;
      
      case DG_DMAPACK_CMD_LINE:
         header = (DG_DMAPACK_TAG*)ExecLine(header);
         break;

      case DG_DMAPACK_CMD_TRIANGLE:
         header = (DG_DMAPACK_TAG*)ExecTriangle(header);
         break;

      case DG_DMAPACK_CMD_QUAD:
         header = (DG_DMAPACK_TAG*)ExecQuad(header);
         break;

      case DG_DMAPACK_CMD_BOX:
         header = (DG_DMAPACK_TAG*)ExecBox(header);
         break;

      case DG_DMAPACK_CMD_SPRT:
         header = (DG_DMAPACK_TAG*)ExecSprt(header);
         break;

      case DG_DMAPACK_CMD_RSPRT:
         header = (DG_DMAPACK_TAG*)ExecRSprt(header);
         break;

      case DG_DMAPACK_CMD_LINESTRIP:
         header = (DG_DMAPACK_TAG*)ExecLineStrip(header);
         break;

      case DG_DMAPACK_CMD_TRIANGLESTRIP:
         header = (DG_DMAPACK_TAG*)ExecTriangleStrip(header);
         break;

      case DG_DMAPACK_CMD_ALPHA:
         header = (DG_DMAPACK_TAG*)ExecAlpha(header);
         break;

      case DG_DMAPACK_CMD_TEXTURE:
         header = (DG_DMAPACK_TAG*)ExecTexture(header);
         break;

      case DG_DMAPACK_CMD_CALLBACK:
         header = (DG_DMAPACK_TAG*)ExecCallback(header);
         break;

      case DG_DMAPACK_CMD_VIEWMAPPING:
         header = (DG_DMAPACK_TAG*)ExecViewMapping(header);
         break;

      case DG_DMAPACK_CMD_MODE:
         header = (DG_DMAPACK_TAG*)ExecMode(header);
         break;

#if DMAPACK_SUPPORTS_FLOAT_PACKETS
      case DG_DMAPACK_CMD_QUAD_F:
         header = (DG_DMAPACK_TAG*)ExecQuad_F(header);
         break;

      case DG_DMAPACK_CMD_LINE_F:
         header = (DG_DMAPACK_TAG*)ExecLine_F(header);
         break;

      case DG_DMAPACK_CMD_LINESTRIP_F:
         header = (DG_DMAPACK_TAG*)ExecLineStrip_F(header);
         break;
      case DG_DMAPACK_CMD_SPRT_F:
         header = (DG_DMAPACK_TAG *)ExecSprt_F(header);
         break;
      case DG_DMAPACK_CMD_RSPRT_F:
         header = (DG_DMAPACK_TAG *)ExecRSprt_F(header);
         break;
      case DG_DMAPACK_CMD_BOX_F:
         header = (DG_DMAPACK_TAG*)ExecBox_F(header);
         break;
#endif // DMAPACK_SUPPORTS_FLOAT_PACKETS

#if MGS_VERSION == 2

      case DG_DMAPACK_CMD_TEXDYN:
         header = (DG_DMAPACK_TAG*)ExecTextureDynamic(header);
         break;

      case DG_DMAPACK_CMD_TEXDYN01:
         header = (DG_DMAPACK_TAG*)ExecTextureDyn01MGS2(header);
         break;

      case DG_DMAPACK_CMD_TEXLIN:
         header = (DG_DMAPACK_TAG*)ExecTextureLiner(header);
         break;

      case DG_DMAPACK_CMD_WINDOW:
         header = (DG_DMAPACK_TAG*)ExecWindow(header);
         break;

      case DG_DMAPACK_CMD_SETZ:
         header = (DG_DMAPACK_TAG*)ExecSetZ(header);
         break;

      case DG_DMAPACK_CMD_ENABLE:
         header = (DG_DMAPACK_TAG*)ExecEnable(header);
         break;

      case DG_DMAPACK_CMD_DISABLE:
         header = (DG_DMAPACK_TAG*)ExecDisable(header);
         break;

      case DG_DMAPACK_CMD_USEFRAMETEX:
         header = (DG_DMAPACK_TAG*)ExecUseFrameTex(header);
         break;

      case DG_DMAPACK_CMD_BACKUPFRAME:
         header = (DG_DMAPACK_TAG*)ExecBackupFrame(header);
#if BP_VITA
         // change render and depth targets of originalRenderTarget in case this triggered a scene change
         {
            SRenderTarget cRT = gpRenderBackend->GetCurrentRenderTarget();
            originalRenderTarget.mpColorBuffer[0] = cRT.mpColorBuffer[0];
            originalRenderTarget.mpDepthBuffer = cRT.mpDepthBuffer;
         }
#endif
         break;

#elif MGS_VERSION == 3
      case DG_DMAPACK_CMD_TEXTUREDYNAMIC:
         header = (DG_DMAPACK_TAG*)ExecTextureDynamic(header);
         break;

      case DG_DMAPACK_CMD_TRIANGLEFAN:
         header = (DG_DMAPACK_TAG*)ExecTriangleFan(header);
         break;

      case DG_DMAPACK_CMD_VIEWPORT:
         header = (DG_DMAPACK_TAG*)ExecViewport(header);
         break;

      case DG_DMAPACK_CMD_COORDINATESCALE:
         header = (DG_DMAPACK_TAG*)ExecCoordinateScale(header);
         break;

      case DG_DMAPACK_CMD_LOADTEXTURE:
         header = (DG_DMAPACK_TAG*)ExecLoadTexture(header);
         break;

      case DG_DMAPACK_CMD_LOADIMAGE:
         header = (DG_DMAPACK_TAG*)ExecLoadImage(header);
         break;

      case DG_DMAPACK_CMD_RENDERBUFFER:
         header = (DG_DMAPACK_TAG*)ExecRenderBuffer(header);
         break;

      case DG_DMAPACK_CMD_RENDERBUFFER_BP:
         header = (DG_DMAPACK_TAG*)ExecRenderBuffer_BP(header);
         break;

      case DG_DMAPACK_CMD_DRAWZ:
         header = (DG_DMAPACK_TAG*)ExecDrawZ(header);
         break;

      case DG_DMAPACK_CMD_ORIGINMATRIX:
         header = (DG_DMAPACK_TAG*)ExecOriginMatrix(header);
         break;

      case DG_DMAPACK_CMD_BASECOLOR:
         header = (DG_DMAPACK_TAG*)ExecBaseColor(header);
         break;

      case DG_DMAPACK_CMD_FRAMETEXTURE:
         header = (DG_DMAPACK_TAG*)ExecFrameTexture(header);
         break;

      case DG_DMAPACK_CMD_FRAMETEXTURE_BP:
         header = (DG_DMAPACK_TAG*)ExecFrameTexture_BP(header);
         break;

      case DG_DMAPACK_CMD_TEST:
         header = (DG_DMAPACK_TAG*)ExecTest(header);
         break;

      case DG_DMAPACK_CMD_OFFSET:
         header = (DG_DMAPACK_TAG*)ExecOffset(header);
         break;

      case DG_DMAPACK_CMD_FILTER:
         header = (DG_DMAPACK_TAG*)ExecFilter(header);
         break;

      case DG_DMAPACK_CMD_ANTIALIAS:
         header = (DG_DMAPACK_TAG*)ExecAntiAlias(header);
         break;
#endif

      default:
         BP_RENDER_TODO_BREAK;
         return;
      }
   }

#if BPE_TARGET == BPE_TARGET_X360
   gpRenderBackend->GetDirect3DDevice()->SetRenderState(D3DRS_LASTPIXEL, TRUE);
#endif

   gpRenderBackend->SetRenderTarget(originalRenderTarget);
   BPE_GPU_POP_PROFILE_MARKER();
}

//----------------------------------------------------------------------------
// Simple DMA/Vif/Gif packet emulation
//----------------------------------------------------------------------------

union BP_DmaTag
{
   uint32      data;
   
   struct
   {
#if BPE_IS_ENDIAN_BIG()
      uint32   irq:1;
      uint32   id:3;
      uint32   pce:2;
      uint32   _pad:10;
      uint32   qwc:16;
#else
      uint32   qwc:16;
      uint32   _pad:10;
      uint32   pce:2;
      uint32   id:3;
      uint32   irq:1;
#endif
   };
};

union BP_VIFCode
{
   uint32      data;

   struct 
   {
#if BPE_IS_ENDIAN_BIG()
      uint32   cmd : 8;
      uint32   num : 8;
      uint32   immediate : 16;
#else
      uint32   immediate : 16;
      uint32   num : 8;
      uint32   cmd : 8;
#endif
   };
};

struct BP_GifTag
{
   union
   {
      uint64      tag;

      struct  
      {
#if BPE_IS_ENDIAN_BIG()
         uint64   nreg : 4;
         uint64   flg : 2;
         uint64   prim : 11;
         uint64   pre : 1;
         uint64   _pad2 : 14;

         uint64   _pad : 16;
         uint64   eop : 1;
         uint64   nloop : 15;
#else
         uint64   nloop : 15;
         uint64   eop : 1;
         uint64   _pad : 16;
         
         uint64   _pad2 : 14;
         uint64   pre : 1;
         uint64   prim : 11;
         uint64   flg : 2;
         uint64   nreg : 4;
#endif
      };
   };

   uint64      regs;
};

union BP_PrimData   
{
   uint64   data;

   struct 
   {
#if BPE_IS_ENDIAN_BIG()
      uint64   _pad : 53;
      uint64   FIX : 1;
      uint64   CTXT : 1;
      uint64   FST : 1;
      uint64   AA1 : 1;
      uint64   ABE : 1;
      uint64   FGE : 1;
      uint64   TME : 1;
      uint64   IIP : 1;
      uint64   PRIM : 3;
#else
      uint64   PRIM : 3;
      uint64   IIP : 1;
      uint64   TME : 1;
      uint64   FGE : 1;
      uint64   ABE : 1;
      uint64   AA1 : 1;
      uint64   FST : 1;
      uint64   CTXT : 1;
      uint64   FIX : 1;
      uint64   _pad : 53;
#endif
   };
};

struct BP_PrimVertex
{
   // Match with _POD version below
   CVector4 col;
   CVector2 uv;
   CVector4 pos;
};

struct BP_PrimVertex_POD
{
   // Match with non _POD above
   float col[4];
   float uv[2];
   float pos[4];
};

namespace
{
   BP_PrimData gPRIM;
   uint64 gRGBAQ = 0x80808080;
};

namespace GSShader
{
   int const kTextureCount = 2;

   CCompiledShader*  gpShader[kTextureCount]
#if BP_VITA
   [skGS_Vita_NumSpecialBlends][CRenderBackend::kAF_Count]
#endif
   ;

   BPE_FORCEINLINE CCompiledShader *Shader( int const texture )
   {
      return gpShader[ texture ]
#if BP_VITA
      [gGS_Vita_CurrentSpecialBlend][ BP_GS_GetCurrentVitaAlphaFunc() ]
#endif
      ;
   }

   CVector4 gTextureParams;
   CVector4 gViewportParams;

   enum
   {
      kReg_UVParams     = 0,
   };
}

void BP_InitGSShader()
{
   using namespace GSShader;

   CShaderFileId const shaderId("$/EngineSupport/Shaders/GS.fx");

#if BP_VITA
   CCompiledShaderCache::PushInhibitShaderWarning();
#endif

   for( int texture = 0; texture < kTextureCount; ++texture )
   {
#if BP_VITA
      for ( int blendMode = 0; blendMode < skGS_Vita_NumSpecialBlends; ++blendMode )
      {
         for ( int alphaFunc = 0; alphaFunc < CRenderBackend::kAF_Count; ++alphaFunc )
         {
            CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d;VITA_SPECIAL_BLEND=%d;VITA_PS2_ATEST_FUNC=%d", texture, blendMode, alphaFunc ) );
            //assert(pShader);

            gpShader[texture][blendMode][alphaFunc] = pShader;
         }
      }
#else
      CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d", texture));
      assert(pShader);

      gpShader[texture] = pShader;
#endif
   }

#if BP_VITA
   CCompiledShaderCache::PopInhibitShaderWarning();
#endif
}


BP_PrimVertex const BP_Prim_GetSpriteVertex(int x, int y, BP_PrimVertex const & ul, BP_PrimVertex const & br)
{
   BP_PrimVertex result = ul;
   
   result.pos[0] = x ? br.pos[0] : ul.pos[0];
   result.pos[1] = y ? br.pos[1] : ul.pos[1];

   result.uv[0] = x ? br.uv[0] : ul.uv[0];
   result.uv[1] = y ? br.uv[1] : ul.uv[1];

   return result;
}

int BP_Prim_GetOutputVertexCount()
{
   switch(gPRIM.PRIM)
   {
   case SCE_GS_PRIM_SPRITE:
      return 6;
      
   default:
      BP_RENDER_DMA_BREAK;
      return 0;
   }
}

CVector2 const BP_Prim_DecodeUV(uint64 uv)
{
   real32 const u = (uv & 0x3fff) / 16.0f;
   real32 const v = ((uv >> 16) & 0x3fff) / 16.0f;
   
   return CVector2(u, v);
}

CVector4 const BP_Prim_DecodeXYZ(uint64 pos)
{
   real32 const x = (uint16)(pos >> 0) / 16.0f;
   real32 const y = (uint16)(pos >> 16) / 16.0f;
   real32 const z = (uint16)(pos >> 32) / (real32)0xFFFFFF;

   return CVector4(x, y, z, 1);
}

void BP_Prim_DecodeVertex(uint64* pData, BP_PrimVertex* pVertex, uint64 regs, int const nreg)
{
   for( int i = 0; i < nreg; ++i )
   {
      switch(regs & 0xf)
      {
      case SCE_GS_RGBAQ:
         gRGBAQ = *pData;
         break;
      
      case SCE_GS_UV:
         pVertex->uv = BP_Prim_DecodeUV(*pData);
         break;

      case SCE_GS_XYZF2:
         pVertex->col = BP_DecodeColorF(gRGBAQ);
         pVertex->pos = BP_Prim_DecodeXYZ(*pData);
         break;

      default:
         BP_RENDER_DMA_BREAK;
         break;
      }

      ++pData;
      regs >>= 4;
   }
}

BP_PrimVertex* BP_Prim_DecodeData(uint64* pInputData, BP_PrimVertex* pOutVertexPtr, uint64 regs, int const nreg)
{
   switch(gPRIM.PRIM)
   {
   case SCE_GS_PRIM_SPRITE:
      {
         BP_PrimVertex vert0, vert1;
         int const vertexRegCount = nreg / 2;
         BP_Prim_DecodeVertex(pInputData, &vert0, regs, vertexRegCount);
         BP_Prim_DecodeVertex(pInputData + vertexRegCount, &vert1, regs, vertexRegCount);

         *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(0, 0, vert0, vert1);
         *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(1, 0, vert0, vert1);
         *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(1, 1, vert0, vert1);

         *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(0, 0, vert0, vert1);
         *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(1, 1, vert0, vert1);
         *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(0, 1, vert0, vert1);
      }
      break;

   default:
      BP_RENDER_DMA_BREAK;
      break;
   }

   return pOutVertexPtr;
}

static BP_PrimVertex gPackedPrimVertexStorage[1024];

DG_GSREG* BP_RenderDmaPack_ProcessPackedPrim(DG_GSREG* pReg, DG_GSREG* pLastReg)
{
   gPRIM.data = pReg->data;
   pReg++;

   int numVerts = 0;
   int foundNonRenderableReg = 0;

   while( (pReg < pLastReg) && !foundNonRenderableReg )
   {
      switch(pReg->reg)
      {
      case SCE_GS_XYZ2:
      case SCE_GS_XYZ3:
      case SCE_GS_XYZF2:
      case SCE_GS_XYZF3:
         gPackedPrimVertexStorage[numVerts].col = BP_DecodeColorF(gRGBAQ);
         gPackedPrimVertexStorage[numVerts].pos = BP_Prim_DecodeXYZ(pReg->data);
         ++numVerts;
         ++pReg;
         break;

      case SCE_GS_ST:
         ++pReg;
         break;

      case SCE_GS_UV:
         gPackedPrimVertexStorage[numVerts].uv = BP_Prim_DecodeUV(pReg->data);
         ++pReg;
         break;

      case SCE_GS_RGBAQ:
         gRGBAQ = pReg->data;
         ++pReg;
         break;

      default:
         foundNonRenderableReg = 1;
         break;
      }
   }

   if( numVerts == 0 )
      return pReg;

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = NULL;
   int outputVertexCount = 0;
   CMeshChunk::EPrimitive primitiveType = CMeshChunk::kPrimitive_Invalid;

   switch(gPRIM.PRIM)
   {
   case SCE_GS_PRIM_SPRITE:
      {
         int const numPrims = numVerts / 2;
         int const vertsPerPrim = 6;

         outputVertexCount = numPrims * vertsPerPrim;
         primitiveType = CMeshChunk::kPrimitive_TriangleList;

         pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(BP_PrimVertex) * outputVertexCount);
         if( pVertexBuffer )
         {
            BP_PrimVertex* pOutVertexPtr = (BP_PrimVertex*)pVertexBuffer->Lock();

            for( int i = 0; i < numPrims; ++i )
            {
               BP_PrimVertex const & vert0 = gPackedPrimVertexStorage[i*2 + 0];
               BP_PrimVertex const & vert1 = gPackedPrimVertexStorage[i*2 + 1];

               *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(0, 0, vert0, vert1);
               *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(1, 0, vert0, vert1);
               *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(1, 1, vert0, vert1);

               *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(0, 0, vert0, vert1);
               *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(1, 1, vert0, vert1);
               *pOutVertexPtr++ = BP_Prim_GetSpriteVertex(0, 1, vert0, vert1);
            }

            pVertexBuffer->Unlock();
         }
      }
      break;

   case SCE_GS_PRIM_TRISTRIP:
      {
         outputVertexCount = numVerts;
         primitiveType = CMeshChunk::kPrimitive_TriangleStrip;

         pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(BP_PrimVertex) * outputVertexCount);
         if( pVertexBuffer )
         {
            BP_PrimVertex* pOutVertexPtr = (BP_PrimVertex*)pVertexBuffer->Lock();
            memcpy(pOutVertexPtr, gPackedPrimVertexStorage, sizeof(BP_PrimVertex) * outputVertexCount);
            pVertexBuffer->Unlock();
         }
      }
      break;

   default:
      BP_RENDER_DMA_BREAK;
      break;
   }

   if( outputVertexCount && pVertexBuffer != NULL )
   {
      CVertexData vertexData;
      size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( BP_PrimVertex ) );
      vertexData.SetAttribute(kVDS_Position, offsetof(BP_PrimVertex_POD, pos), kVDT_Float4, bufferIndex );
      vertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_PrimVertex_POD, uv), kVDT_Float2, bufferIndex);
      vertexData.SetAttribute(kVDS_Color0, offsetof(BP_PrimVertex_POD, col), kVDT_Float4, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);

      int const isTexture = gPRIM.TME ? 1 : 0;
      BP_BeginShader(GSShader::Shader(isTexture), NULL, 0);

      gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();
      gpRenderBackend->RenderPrimitivesNoIndices(primitiveType, 0, outputVertexCount);
   }

   return pReg;
}

char* BP_RenderDmaPack_ProcessGifPacket(char* addr)
{
   BP_GifTag* pGifTag = (BP_GifTag*)addr;

   switch(pGifTag->flg)
   {
   case SCE_GIF_PACKED:
      {
         int const count = pGifTag->nreg * pGifTag->nloop;

         if( pGifTag->nreg == 1)
         {
            if( pGifTag->regs == SCE_GIF_PACKED_AD )
            {
               DG_GSREG* pReg = (DG_GSREG*)pGifTag + 1;
               DG_GSREG* pLastReg = pReg + count;

               while( pReg < pLastReg )
               {
                  switch(pReg->reg)
                  {
                  case SCE_GS_ALPHA_1:
                     BP_GS_SetAlpha(pReg->data);
                     ++pReg;
                     break;
                  
                  case SCE_GS_TEST_1:
                     BP_GS_SetTest(pReg->data);
                     BP_GS_SetupAlphaTestSinglePass(false);
                     ++pReg;
                     break;
                  
                  case SCE_GS_CLAMP_1:
                     BP_GS_SetClamp(0, pReg->data);
                     ++pReg;
                     break;

                  case SCE_GS_TEXA:
                     ++pReg;
                     break;

                  case SCE_GS_TEXFLUSH:
                     ++pReg;
                     break;

                  case SCE_GS_TEX0_1:
                  case SCE_GS_TEX1_1:
                  case SCE_GS_TEX2_1:
                     ++pReg;
                     break;

                  case SCE_GS_COLCLAMP:
                     ++pReg;
                     break;

                  case BP_GS_TEX:
                     {
#if 1 //BP_TODO
                        DG_TEX* pTex = (DG_TEX*)pReg->data;
                        int width, height, x, y;
                        DG_GetTexelInfo(&width, &height, &x, &y, pTex);
                        
                        gpRenderBackend->SetVertexRegisters(GSShader::kReg_UVParams, 1, &CVector4(1.0f / width, 1.0f / height, (real32)-x / width, (real32)-y / height));

                        BP_DG_SetTexture(pTex, 0);
#endif
                        ++pReg;
                     }
                     break;

                  case BP_GS_DYNTEX:
                     {
                        CBaseTexture const * pTexture = (CBaseTexture const *)pReg->data;

                        int width, height, x, y;
                        width = pTexture->GetWidth();
                        height = pTexture->GetHeight();
                        x = 0;
                        y = 0;
                        gpRenderBackend->SetVertexRegisters(GSShader::kReg_UVParams, 1, &CVector4(1.0f / width, 1.0f / height, (real32)-x / width, (real32)-y / height));
                        gpRenderBackend->SetTexture(0, pTexture);

                        ++pReg;
                     }
                     break;

                  case SCE_GS_RGBAQ:
                     gRGBAQ = pReg->data;
                     ++pReg;
                     break;

                  case SCE_GS_PRIM:
                     pReg = BP_RenderDmaPack_ProcessPackedPrim(pReg, pLastReg);
                     break;

                  case SCE_GS_NOP:
                     ++pReg;
                     break;

                     //TED - frame 11306 of intro cinema
                  case SCE_GS_XYZ2:
                  case SCE_GS_XYZ3:
                  case SCE_GS_XYZF2:
                  case SCE_GS_XYZF3:
                  case SCE_GS_UV:
                     BP_RENDER_DMA_BREAK;
                     ++pReg;
                     break;

                  default:
                     BP_RENDER_DMA_BREAK;
                     ++pReg;
                     break;
                  }
               }

               return (char*)pReg;
            }
         }
      }
      break;
   
   case SCE_GIF_REGLIST:
      {
         uint64* pData = (uint64*)(pGifTag + 1);

         int const primVertexCount = BP_Prim_GetOutputVertexCount();
         int const totalVertexCount = primVertexCount * pGifTag->nloop;

         CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(BP_PrimVertex) * totalVertexCount);
         if( pVertexBuffer )
         {
            BP_PrimVertex* pOutVertexPtr = (BP_PrimVertex*)pVertexBuffer->Lock();

            for( int l = 0; l < pGifTag->nloop; ++l )
            {
               pOutVertexPtr = BP_Prim_DecodeData(pData, pOutVertexPtr, pGifTag->regs, pGifTag->nreg);
               pData += pGifTag->nreg;
            }
            
            // Deal properly with odd number of items
            if( (pGifTag->nloop * pGifTag->nreg) & 1)
            {
               BP_RENDER_DMA_BREAK;
            }

            pVertexBuffer->Unlock();

            CVertexData vertexData;
            size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( BP_PrimVertex ) );
            vertexData.SetAttribute(kVDS_Position, offsetof(BP_PrimVertex_POD, pos), kVDT_Float4, bufferIndex );
            vertexData.SetAttribute(kVDS_TexCoord0, offsetof(BP_PrimVertex_POD, uv), kVDT_Float2, bufferIndex);
            vertexData.SetAttribute(kVDS_Color0, offsetof(BP_PrimVertex_POD, col), kVDT_Float4, bufferIndex);

            CShaderVertexDataBinding binding;
            binding.Set(kVDU_Position, kVDS_Position);
            binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
            binding.Set(kVDU_TexCoord1, kVDS_Color0);

            int const isTexture = gPRIM.TME ? 1 : 0;
            BP_BeginShader(GSShader::Shader(isTexture), NULL, 0);

            gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
            gpRenderBackend->ForceVertexDataRebind();
            gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleList, 0, totalVertexCount);
         }
         else
         {
            for( int l = 0; l < pGifTag->nloop; ++l )
            {
               pData += pGifTag->nreg;
            }
         }

         return (char*)pData;
      }
      break;

   case SCE_GIF_IMAGE:
      {
         BP_RENDER_DMA_BREAK;
      }
      break;

   default:
      BP_RENDER_DMA_BREAK;
      break;
   }

   BP_RENDER_DMA_BREAK;
   return NULL;
}

void BP_RenderDmaPack_ProcessVifPacket(uint32 vif, char* addr)
{
   BP_VIFCode code;
   code.data = vif;

   switch(code.cmd)
   {
   case 0: //SCE_VIF1_SET_NOP
      break;

   case 0x50: //SCE_VIF1_SET_DIRECT
      {
         char* endAddr = addr + 16 * code.immediate;

         while( addr < endAddr )
         {
            addr = BP_RenderDmaPack_ProcessGifPacket(addr);
         }
      }
      break;

   default:
      BP_RENDER_DMA_BREAK;
      break;

   }
}

void BP_RenderDmaPack_Direct(char* pPacketData)
{
   SBP_RenderDmaPack* pPacket = (SBP_RenderDmaPack*)pPacketData;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("DmaPack: Direct");

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);
   gpRenderBackend->SetDepthWriteEnabled(false);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   BP_GS_SetupAlphaTestSinglePass(false);

   // ja - this is a bad idea, it points into the scene buffer memory, which doesn't look to stay around long enough
   //DG_DMAPACK* dmaPack = ( DG_DMAPACK* )pPacket->dmapack;

   DG_DMATAG* pTag = (DG_DMATAG*)pPacket->data;

   while( pTag != NULL )
   {
      BP_DmaTag data;
      data.data = pTag->qwc;

      // Mask off IRQ bit
      switch( pTag->vifcode[0] & 0x7fffffff )
      {
      case SCE_VIF1_SET_FLUSHA(0):
         break;

      case SCE_VIF1_SET_NOP(0):
         break;
      
      default:
         BP_BREAK;
         break;
      }

      switch( data.id )
      {
      case 0: //refe
         BP_RENDER_DMA_BREAK;
         break;

      case 1: //cnt
         {
            DG_DMATAG* pData = pTag + 1;
            BP_RenderDmaPack_ProcessVifPacket(pTag->vifcode[1], (char*)pData);
            pTag = pData + data.qwc;
         }
         break;

      case 2: //next
         BP_RENDER_DMA_BREAK;
         break;

      case 3: //ref
         BP_RenderDmaPack_ProcessVifPacket(pTag->vifcode[1], (char*)pTag->addr);
         pTag = pTag + 1;
         break;

      case 4: //refs
         BP_RENDER_DMA_BREAK;
         break;

      case 5: //call
         BP_RENDER_DMA_BREAK;
         break;

      case 6: //ret
         BP_RenderDmaPack_ProcessVifPacket(pTag->vifcode[1], (char*)(pTag + 1));
         return;

      case 7: //end
         return;
      }
   }
}
