//----------------------------------------------------------------------------
// BP_RenderPatch.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderPatch.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"

#include "BP_Renderer.h"
#include "BP_RenderGS.h"
#include "BP_RendererDebug.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"
#include "bp_matrix.h"
#include "BP_EndianSupport.h"

//----------------------------------------------------------------------------

#include "BP_RenderBufferTypes.h"
#include "BP_RenderShared.h"

// size of each patch vertex.
#define VERTEX_SIZE           ( ( 3 + 3 + 2 ) * sizeof( float ) +    \
                                  1 * sizeof( unsigned int ) )
#define INDEX_SIZE            2

//----------------------------------------------------------------------------

static void patch_assertion_function()
{
   // This function exists just for the ctassert
   SBP_Patch_Render r;

   BPE_CTASSERT( sizeof( r.indexBuffer_UT ) >= sizeof( CDynamicIndexBufferPool_UT::TChunk ) );
}

namespace Patch
{
   // patch shader type.
   const int kBase = 0;    // textured.
   const int kPatchMode = 2;    // textured + environment-mapped.

   // vertex count for render calls.
   unsigned int gVertexCount;

   // shader pointers.
   CCompiledShader*  gpShader[ kPatchMode ];

   enum ERegister
   {
      kReg_Pers      =  16,
      kReg_View      =  20,
      kReg_World     =  24,
      kReg_LightColor=  28,
      kReg_LightVec  =  32,
      kReg_EyeInv    =  36,
   };
}

int gPatch_EnablePatch = 1;

typedef struct _VERTEX
{
   float pos[ 3 ];
   float norm[ 3 ];
   unsigned char bgraColor[ 4 ];
   float uv[ 2 ];
} Vertex;

static CVertexData gPatch_VertexData;

extern "C" 
{
   // AS(JM) Global patch UV scalars.
   // Apparently, V for patches is always 1/2 of what it should be.
   float gAS_Patch_ScaleU = 1.0f;
   float gAS_Patch_ScaleV = 1.0f;
}

#if BP_VITA
static NVTAState::SCachedVertexData *gpPatch_CVD = NULL;
#endif

void AS_Patch_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU

   int const renderMenu = BP_DebugMenu_GetMenu("Render");
   int const debugMenu = BP_DebugMenu_AddMenu("Patch", renderMenu);

   BP_DebugMenu_AddFloat(debugMenu, "Scale U", &gAS_Patch_ScaleU, 0.f, 10.f, 0.1f, 1.0f );
   BP_DebugMenu_AddFloat(debugMenu, "Scale V", &gAS_Patch_ScaleV, 0.f, 10.f, 0.1f, 1.0f );
#endif
}

void BP_InitPatchShader()
{
   // build the file ID.
   CShaderFileId const shaderId("$/EngineSupport/Shaders/Patch.fx");
   for ( unsigned int iPatchMode = 0; iPatchMode < Patch::kPatchMode; ++iPatchMode )
   {
      // compile the current shader and verify that it compiled successfully.
      CCompiledShader* pShader = gpRenderBackend->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("PATCH_MODE=%d", iPatchMode));
      assert(pShader);

      // store the current shader.
      Patch::gpShader[iPatchMode] = pShader;
   }

   // First we set up the patch vertex data structures
   // Note that these will start off with invalid buffer indices

   gPatch_VertexData.SetAttribute( kVDS_Position,   0,    kVDT_Float3,   0 );
   gPatch_VertexData.SetAttribute( kVDS_Normal,     12,   kVDT_Float3,   0 );
   gPatch_VertexData.SetAttribute( kVDS_Color0,     24,   kVDT_UByte4N,  0 );
   gPatch_VertexData.SetAttribute( kVDS_TexCoord0,  28,   kVDT_Float2,   0 );

#if BP_VITA
   // set the vertex binding.
   CShaderVertexDataBinding vertexBinding;
   vertexBinding.Set( kVDU_Position,   kVDS_Position );
   vertexBinding.Set( kVDU_Normal,     kVDS_Normal );
   vertexBinding.Set( kVDU_Color0,     kVDS_Color0 );
   vertexBinding.Set( kVDU_TexCoord0,  kVDS_TexCoord0 );

   gPatch_VertexData.AddBuffer_Unsafe( (void *) 0x80000000, VERTEX_SIZE ); 

   gpPatch_CVD = new NVTAState::SCachedVertexData( vertexBinding, gPatch_VertexData );
#endif
}

void BP_PatchInitFrame(char* pData)
{
   SBP_Patch_InitFrame* patchInfo = ( SBP_Patch_InitFrame* )pData;
   // send matrices to the card...
   real32 temp[16];
   // projection matrix.
   BP_KPPersMatrix_to_VS_Matrix44( ( const real32* )&patchInfo->pers, temp);
   gpRenderBackend->SetVertexRegisters( Patch::kReg_Pers, 4, ( const CVector4* )temp );

   // view matrix.
   BP_Matrix44_to_VS_Matrix44( ( const real32* )&patchInfo->view, temp );
   gpRenderBackend->SetVertexRegisters( Patch::kReg_View, 4, ( const CVector4* )temp );

   // just make it render.
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetDepthCompareEnabled(true);

   BP_GS_SetupAlphaTestSinglePass(false);
}

void BP_PatchInit(char* pData)
{
   // get the patch information.
   SBP_Patch_Init* patchInfo = ( SBP_Patch_Init* )pData;

   // send matrices to the card...
   real32 temp[16];
   // world matrix.
   BP_Matrix44_to_VS_Matrix44( ( const real32* )&patchInfo->world, temp );
   gpRenderBackend->SetVertexRegisters( Patch::kReg_World, 4, ( const CVector4* )temp );

   if( patchInfo->patchMode == PATCH_MODE_REFLECTPLANE )
   {
      BP_Matrix44_to_VS_Matrix44( ( const real32* )&patchInfo->eyeInv, temp );
      gpRenderBackend->SetVertexRegisters( Patch::kReg_EyeInv, 4, ( const CVector4* )temp );
   }
  
   // setup fog.
   if( patchInfo->patchFlags & DG_PATCH_FOGBLACK )
   {
      const CVector4 black( 0, 0, 0, 0 );
      gpRenderBackend->SetVertexRegisters( kSReg_FogColor, 1, ( const CVector4* )&black );
   }
   else
   {
      extern CVector4 gBP_FogColor;
      gpRenderBackend->SetVertexRegisters( kSReg_FogColor, 1, &gBP_FogColor );
   }

   BP_SetRasterMask((patchInfo->patchFlags & DG_PATCH_SEMITRANS) ? 1 : 0);

   gpRenderBackend->SetVertexRegisters(kSReg_FogParam, 1, ( const CVector4* )&patchInfo->fogParam);

   // apply the lighting color.
   gpRenderBackend->SetVertexRegisters( Patch::kReg_LightColor, 4, ( const CVector4* )&patchInfo->lightCol );

   // apply lighting vector.
   gpRenderBackend->SetVertexRegisters( Patch::kReg_LightVec, 4, ( const CVector4* )&patchInfo->lightVec );

   // bind the texture.
   DG_TEX_BP* pTex = patchInfo->tex0;
   if ( pTex != 0 )
   {
      BP_GS_SetAlpha( pTex->tex_trans.alpha.data );

      // NOTE: This is how MGS2 XBOX code worked by default (might be worth investigating globally enabling this behavior if it holds true, though there might be performance reasons not to!)
      BP_GS_SetAlphaTestEnabled( gGS_AlphaBlendEnabled ? true : false );
//      gpRenderBackend->SetAlphaTestEnable(gGS_AlphaBlendEnabled ? true : false);

      BP_SetTexture( pTex, 0 );
   }

   // bind the shader.
   BP_BeginShader( Patch::gpShader[ patchInfo->patchMode ], NULL, 0 );
}

//----------------------------------------------------------------------------

void BP_PatchInitRender(char* pData)
{
   // get the vertex information.
   SBP_Patch_InitRender* initRenderInfo = ( SBP_Patch_InitRender* )pData;
   Patch::gVertexCount = initRenderInfo->n_vertices;

   CDynamicVertexBufferPoolChunk_UT* vertexChunk = reinterpret_cast<CDynamicVertexBufferPoolChunk_UT *>( initRenderInfo->vertexBuffer_UT );

   // initialize a vertex data object.
#if !BP_VITA
   CVertexData vertexData = gPatch_VertexData;

   vertexData.ClearVertexBuffers();
   vertexData.AddBuffer( vertexChunk, VERTEX_SIZE );

   // set the vertex binding.
   CShaderVertexDataBinding vertexBinding;
   vertexBinding.Set( kVDU_Position,   kVDS_Position );
   vertexBinding.Set( kVDU_Normal,     kVDS_Normal );
   vertexBinding.Set( kVDU_Color0,     kVDS_Color0 );
   vertexBinding.Set( kVDU_TexCoord0,  kVDS_TexCoord0 );

   // set the vertex arrays.
   gpRenderBackend->SetVertexData( vertexBinding, vertexData, 0, 0 );
   gpRenderBackend->ForceVertexDataRebind();
#else
   gpPatch_CVD->SetBuffer( 0, vertexChunk );
   gpRenderBackend->SetVertexData(gpPatch_CVD);
#endif
}

void BP_PatchRender(char* pData)
{
   // get the render call information.  Note that there are mutiple render calls for one
   // set of render states.
   SBP_Patch_Render* renderInfo = ( SBP_Patch_Render* )pData;
   
   if (!gPatch_EnablePatch)
      return;

   // allocate space for index data.
   CDynamicIndexBufferPool_UT::TChunk indexChunk = CDynamicIndexBufferPool_UT::ChunkFromInts( renderInfo->indexBuffer_UT );

   // setup the device.
   gpRenderBackend->SetIndexData( indexChunk );

   // render.
   gpRenderBackend->RenderPrimitives( CMeshChunk::kPrimitive_TriangleStrip, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
      0, Patch::gVertexCount, 
#endif
      0, renderInfo->n_indices );
}

void BP_PatchFinalizeFrame(char* pData)
{
   // restore the texture addressing mode.
   // restore the stencil state.
   // clear the stencil buffer.

   BP_SetRasterMask(0);
}
