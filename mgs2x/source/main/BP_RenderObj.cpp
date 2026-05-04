//----------------------------------------------------------------------------
// BP_RenderObj.cpp
//----------------------------------------------------------------------------

#pragma warning( disable : 4244 )

#include "Engine/Stdafx.h"
#include "BP_RenderObj.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Primitive/CMeshBuffers.h"

#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_RenderGS.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

#include "BP_RenderBufferTypes.h"
#include "BP_RenderShared.h"

//----------------------------------------------------------------------------

namespace
{
   int gNumRendered;
   int gNumPacketGroups;
   int gNumPackets;

   int gNumTexDiff;
   int gNumFlagDiff;
}

// Defines from the shader
#define BUFFER_LOCAL_PARAMS 0
#define BUFFER_PRERENDER_PARAMS 1
#define BUFFER_TEXTURE_PARAMS 2
#define BUFFER_ENVMAP_PARAMS 3

int gObj_EnableSingleTex = 1;
int gObj_EnableMultiTex = 1;
int gObj_ForceSingleTex = 0;
int gObj_EnableEnvmap = 1;
int gObj_EnableVertexAnim = 1;
int gObj_DisableSortChainZWrite = 1;
int gObj_OptimizeAlphaTest_EnableFrameToggle = 0;
int gObj_OptimizeAlphaTest = 1;
int gObj_ShowStats = 0;
int gObj_KillFog = 0;
int gObj_IROnly = 0;

int gAS_Obj_Optimize3PassEnv = 1;
int gAS_Obj_Optimize3PassEnv_Pingpong = 0;

//----------------------------------------------------------------------------

namespace ObjCharShader
{
   int const kLightCount = 2;
   int const kFogCount = 2;
   int const kVAnimCount = 2;

   CCompiledShader*  gpShader[kLightCount][kFogCount][kVAnimCount]
#if BP_VITA
   [CRenderBackend::kAF_Count]
#endif
   = { 0 };

   enum ERegisters
   {
      kReg_Persp           = 20,
      kReg_Correction      = 24,
      kReg_TexScaleOffset  = 28,
      kReg_EyeInv          = 29
   };

   BPE_FORCEINLINE CCompiledShader *Shader( int const light, int const fog, int const Anim )
   {
      CCompiledShader* pShader = gpShader[ light ][ fog ][ Anim ]
#if BP_VITA
      [ BP_GS_GetCurrentVitaAlphaFunc() ]
#endif
      ;

      BPE_ASSERT_NO_MSG(pShader != NULL);
      return pShader;
   }
}

//----------------------------------------------------------------------------

namespace ObjShader
{
   int const kTextureCount = 2;
   int const kLightCount = 2;
   int const kFogCount = 2;
   int const kVAnimCount = 2;

   enum EUVType
   {
      kUV_Set0,
      kUV_Set1,
      kUV_Set2,
      kUV_Envmap,
      kUV_Smap,

      kUV_Count
   };

   CCompiledShader*  gpShader[kTextureCount][kLightCount][kFogCount][kVAnimCount][kUV_Count]
#if BP_VITA
   [skGS_Vita_NumSpecialBlends][CRenderBackend::kAF_Count]
#endif
    = { 0 };

   unsigned int gIsSortChain = 0;

   enum ERegisters
   {
      kReg_Persp           = 20,
      kReg_Correction      = 24,
      kReg_TexScaleOffset  = 28,
      kReg_EyeInv          = 29
   };

   BPE_FORCEINLINE CCompiledShader *Shader( int const texture, int const light, int const fog, int const Anim, int const UV )
   {
      CCompiledShader* pShader = gpShader[ texture ][ light ][ fog ][ Anim ][ UV ]
#if BP_VITA
      [ gGS_Vita_CurrentSpecialBlend ][ BP_GS_GetCurrentVitaAlphaFunc() ]
#endif
      ;

      BPE_ASSERT_NO_MSG(pShader != NULL);
      return pShader;
   }

#if BP_VITA
   struct SVitaLocalParams
   {
      SVitaLocalParams();

      float16x4_t mFogColor;
      CVector4 mFogParam;
   };

   KP_CTASSERT( sizeof( SVitaLocalParams ) == 8+16 );

   struct SVitaPreRenderParams
   {
      SVitaPreRenderParams();

      real32 mLightDir[16];
      float16x4x4_t mLightCol;
      real32 mPersp[16];
      real32 mCorrection[16];
      real32 mEyeInv[16];
   };

   KP_CTASSERT( sizeof( SVitaPreRenderParams ) == 8*4 + 16 * 4 * 4 );

   struct SVitaTextureParams
   {
      SVitaTextureParams();

      CVector4 mTexScaleOffset;
   };

   KP_CTASSERT( sizeof( SVitaTextureParams ) == 16 );

   struct SVitaEnvmapParams
   {
      SVitaEnvmapParams();

      float16x4_t mEnvmapParams;
   };

   KP_CTASSERT( sizeof( SVitaEnvmapParams ) == 8 );

   bool sObjNeedsVertexUniformFlush = false;
   void *spLastSetUniformBuffer = NULL;
#endif
}

void BP_InitObjCharShader()
{
   CShaderFileId const shaderId("$/EngineSupport/Shaders/KMSChar.fx");

   for( int light = 0; light < ObjCharShader::kLightCount; ++light )
   {
      for( int fog = 0; fog < ObjCharShader::kFogCount; ++fog )
      {
         for( int vanim = 0; vanim < ObjCharShader::kVAnimCount; ++vanim )
         {
#if BP_VITA
            for ( int alphaFunc = 0; alphaFunc < CRenderBackend::kAF_Count; alphaFunc++ )
            {
               if (BP_IsValidVitaShaderCombination(0, alphaFunc))
               {
                  CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, 
                     CStringExtras::Stringize_s("LIGHT=%d;FOG=%d;VANIM=%d;VITA_PS2_ATEST_FUNC=%d", 
                     light, fog, vanim, alphaFunc ) );

                  BPE_ASSERT_NO_MSG(pShader);
                  ObjCharShader::gpShader[light][fog][vanim][alphaFunc] = pShader;
               }
            }
#else
            CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, 
               CStringExtras::Stringize_s("LIGHT=%d;FOG=%d;VANIM=%d", light, fog, vanim));
            assert(pShader);

            ObjCharShader::gpShader[light][fog][vanim] = pShader;
#endif
         }
      }
   }
}

void BP_InitObjShader()
{
   CShaderFileId const shaderId("$/EngineSupport/Shaders/KMS.fx");

   for( int texture = 0; texture < ObjShader::kTextureCount; ++texture )
   {
      for( int light = 0; light < ObjShader::kLightCount; ++light )
      {
         for( int fog = 0; fog < ObjShader::kFogCount; ++fog )
         {
            for( int vanim = 0; vanim < ObjShader::kVAnimCount; ++vanim )
            {
               for( int uv = 0; uv < ObjShader::kUV_Count; ++uv )
               {
#if BP_VITA
                  for ( int blendMode = 0; blendMode < skGS_Vita_NumSpecialBlends; ++blendMode )
                  {
                     for ( int alphaFunc = 0; alphaFunc < CRenderBackend::kAF_Count; alphaFunc++ )
                     {
                        if (BP_IsValidVitaShaderCombination(blendMode, alphaFunc))
                        {
                           CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, 
                              CStringExtras::Stringize_s("TEXTURE=%d;LIGHT=%d;FOG=%d;VANIM=%d;UV=%d;VITA_SPECIAL_BLEND=%d;VITA_PS2_ATEST_FUNC=%d", 
                              texture, light, fog, vanim, uv, blendMode, alphaFunc ) );

                           BPE_ASSERT_NO_MSG(pShader);
                           ObjShader::gpShader[texture][light][fog][vanim][uv][blendMode][alphaFunc] = pShader;
                        }
                     }
                  }
#else
                  CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d;LIGHT=%d;FOG=%d;VANIM=%d;UV=%d", texture, light, fog, vanim, uv));
                  assert(pShader);

                  ObjShader::gpShader[texture][light][fog][vanim][uv] = pShader;
#endif
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

void BP_Obj_FrameBegin()
{
   gNumRendered = 0;
   gNumPacketGroups = 0;
   gNumPackets = 0;
   gNumTexDiff = 0;
   gNumFlagDiff = 0;
   if( gObj_OptimizeAlphaTest_EnableFrameToggle )
   {
      gObj_OptimizeAlphaTest = !gObj_OptimizeAlphaTest;
   }

   if ( gAS_Obj_Optimize3PassEnv_Pingpong )
   {
      gAS_Obj_Optimize3PassEnv = ( gpRenderBackend->GetFrameCount() % 30 ) >= 15;
   }
}

void BP_Obj_FrameEnd()
{
   if( gObj_ShowStats )
   {
      BP_DebugText_Print("Obj Count: %d", gNumRendered);
      BP_DebugText_Print("Packets: %d per group: %d", gNumPackets, gNumPacketGroups);
      BP_DebugText_Print("Tex Diff: %d Flag Diff: %d", gNumTexDiff, gNumFlagDiff);
   }
}

//----------------------------------------------------------------------------

// temporary memory usage.
#define TEMP_MEM_SIZE         ( 1024 * 1024 )
static unsigned char tempMem[ TEMP_MEM_SIZE ];

typedef struct _PACKEDVERTEX
{
   short vx;
   short vy;
   short vz;
   short vw;
   short nx;
   short ny;
   short nz;
   short pad;
   short u;
   short v;
} PackedVertex;

static int _BP_HasVertexAnim( DG_OBJS* objs )
{
   extern int gBP_DebugVertexAnim_Enabled;

   if( !gBP_DebugVertexAnim_Enabled )
      return 0;

   unsigned int hasVertexAnim = 0;
   unsigned int numModels = objs->n_models;

   for ( unsigned int i = 0; i < numModels; ++i )
   {
      // get the current object, along with it's associated source model.
      DG_OBJ* curObj = objs->objs + i;
      //DG_OBJ* curObj = objs->objs + unit;
      DG_MDL* curMdl = curObj->model;

      if ( curObj->verts != ( SVECTOR* )curMdl->packs->verts )
         hasVertexAnim |= 1;

      if ( curObj->norms != ( SVECTOR* )curMdl->packs->norms )
         hasVertexAnim |= 1;

      for ( unsigned int j = 0; j < 3; ++j )
         if ( curObj->uvs[ j ] != curMdl->packs->uvs[ j ] )
            hasVertexAnim |= 1;
   }

   // return true if the object has vertex animation data.
   return hasVertexAnim;
}

//----------------------------------------------------------------------------

static int _BP_GetPackedVertexCount( DG_OBJS *objs, CMesh const *pMesh )
{
   int dgObjCount = 0;

   for ( unsigned int i = 0; i < objs->n_models; ++i )
   {
      DG_OBJ* obj = objs->objs + i;
      for ( unsigned int j = 0; j < obj->n_packs; ++j )
      {
         // get the current packet.
         DG_OBJ_PACKET* packet = obj->packets + j;

         dgObjCount += packet->n_verts;
      }
   }

   int meshCount = 0;
   if ( pMesh )
   {
      meshCount = pMesh->GetMeshBuffers().GetMaxOriginalVertexIndexPlusOne();
   }

   return bpe::max_val( dgObjCount, meshCount );
}

//----------------------------------------------------------------------------

static void _BP_VAnimBuildInterleavedArray( DG_OBJS* objs, PackedVertex *packedVerts )
{
   // allocate an appropriately sized vertex buffer.
   PackedVertex* destVerts = packedVerts;

   // copy the vertex data to the vertex buffer.
   for ( unsigned int i = 0; i < objs->n_models; ++i )
   {
      // get the current object.
      DG_OBJ* obj = objs->objs + i;
      //DG_OBJ* obj = objs->objs + unit;

      // get the vertex data streams that we care about.
      short* verts = ( short* )obj->verts;
      short* norms = ( short* )obj->norms;
      short* uvs = obj->uvs[ 0 ];

      // iterate over packets and pack the data into our temp buffers.
      for ( unsigned int j = 0; j < obj->n_packs; ++j )
      {
         // get the current packet.
         DG_OBJ_PACKET* packet = obj->packets + j;

         // calculate the UV scale and bias.
         // copy each vertex.
         short* curVert = verts;
         short* curNorm = norms;
         short* curUV = uvs; 
         for ( unsigned int k = 0; k < packet->n_verts; ++k )
         {
            // store the current position.
            destVerts->vx = curVert[ 0 ];
            destVerts->vy = curVert[ 1 ];
            destVerts->vz = curVert[ 2 ];
            destVerts->vw = curVert[ 3 ];

            // store the current normal.
            destVerts->nx = curNorm[ 0 ];
            destVerts->ny = curNorm[ 1 ];
            destVerts->nz = curNorm[ 2 ];

            // store the current texture coordinate.
            destVerts->u = curUV[0];
            destVerts->v = curUV[1];

            // advance to the next source vertex.
            curVert += 4;
            curNorm += 4;
            curUV += 2;

            // advance to the next destination vertex.
            ++destVerts;
         }

         // advance to the next packet.
         verts += packet->verts_offset * 8;
         norms += packet->norms_offset * 8;
         uvs += packet->uvs_offset[ 0 ] * 8;
      }
   }
}

//----------------------------------------------------------------------------

static CDynamicVertexBufferPoolChunk_UT* _BP_VAnimAllocRemappedVertexBuffer( CMesh* targetMesh )
{
   CMeshBuffers& meshBuffers = const_cast< CMeshBuffers& >( targetMesh->GetMeshBuffers() );
   unsigned int vertexCount = meshBuffers.GetVertexCount();
   CDynamicVertexBufferPool_UT* vertexPool = gpRenderBackend->GetVertexBufferPool_UT();
   CDynamicVertexBufferPoolChunk_UT* vertexChunk = vertexPool->AllocChunk( vertexCount * sizeof(PackedVertex) );

   return vertexChunk;
}
//----------------------------------------------------------------------------

static void _BP_VAnimBuildRemappedVertexBuffer( CMesh* targetMesh, PackedVertex* packedVerts, CDynamicVertexBufferPoolChunk_UT *vertexChunk )
{
   // Remap vertex array
   // allocate a dynamic vertex chunk for the given mesh.
   CMeshBuffers& meshBuffers = const_cast< CMeshBuffers& >( targetMesh->GetMeshBuffers() );
   unsigned int vertexCount = meshBuffers.GetVertexCount();

   // remap vertex data into the vertex buffer.
   const uint32* vertexIndices = meshBuffers.GetOriginalVertexIndices();
   PackedVertex* dest = ( PackedVertex* )vertexChunk->Lock();
   for ( unsigned int i = 0; i < vertexCount; ++i )
   {
      // get the original index.
      int const originalIndex = vertexIndices[i];

      // remap vertex
      *dest++ = *(packedVerts + originalIndex);
   }

   // unlock the vertex buffer chunk.
   vertexChunk->Unlock();

   // Print debug info
   {
      extern int gBP_DebugVertexAnim_ShowInfo;
      if( gBP_DebugVertexAnim_ShowInfo )
         BP_DebugText_Print("Vertex Anim Buffer: %d verts", vertexCount);
   }
}

//----------------------------------------------------------------------------

namespace
{
   struct SVAnimUltParam
   {
      SVAnimUltParam( CMesh *pMesh, DG_OBJS *pObjs, CDynamicVertexBufferPoolChunk_UT *pVertexChunk )
         : mpMesh( pMesh ), mpObjs( pObjs ), mpVertexChunk (pVertexChunk )
      {
      }

      CMesh *mpMesh;
      DG_OBJS *mpObjs;
      CDynamicVertexBufferPoolChunk_UT *mpVertexChunk;
   };
}

static void _BP_VAnimBuildRemappedVertexBuffer_Ult( SULTParam const *_param )
{
   SVAnimUltParam const *param = reinterpret_cast<SVAnimUltParam const *>(_param );
   CMesh *pMesh = param->mpMesh;
   DG_OBJS *pObjs = param->mpObjs;
   CDynamicVertexBufferPoolChunk_UT *pVertexChunk = param->mpVertexChunk;

   int const packedVertexCount = _BP_GetPackedVertexCount( pObjs, pMesh );
   PackedVertex *pPackedVerts = new PackedVertex[ packedVertexCount ];

   _BP_VAnimBuildInterleavedArray( pObjs, pPackedVerts );

   _BP_VAnimBuildRemappedVertexBuffer( pMesh, pPackedVerts, pVertexChunk );

   delete [] pPackedVerts;
}

//----------------------------------------------------------------------------

static CDynamicVertexBufferPoolChunk_UT* _BP_GenerateVAnimVertexBuffer( CMesh* targetMesh, DG_OBJS* objs )
{
   // Generate updated original vertex array
   CDynamicVertexBufferPoolChunk_UT* vertexChunk = _BP_VAnimAllocRemappedVertexBuffer( targetMesh );

   if ( vertexChunk )
   {
      SVAnimUltParam param( targetMesh, objs, vertexChunk );

      BP_Render_PostUltWork( 
         _BP_VAnimBuildRemappedVertexBuffer_Ult,
         &param, sizeof( param ) );
   }

   return vertexChunk;
}

//----------------------------------------------------------------------------

void BP_Obj_Render_ComputeAnimatedBuffers(char* pDGObjs_, char* pMesh_)
{
   DG_OBJS* pDGObjs = (DG_OBJS*)pDGObjs_;
   CMesh* pMesh = (CMesh*)pMesh_;
   uint32 const backendFrameCount = gpRenderBackend->GetFrameCount();

   // Only generate or check vertex animation buffer once per frame for dgobjs
   // Because BP_HasVertexAnim below looks through the entire objs list for any given
   // object, which means tons of repeated work.
   if ( pDGObjs->BP_VAnimBufferTimestamp != backendFrameCount )
   {
      pDGObjs->BP_VAnimBufferTimestamp = backendFrameCount;

      int IsVAnim = gObj_EnableVertexAnim && _BP_HasVertexAnim(pDGObjs);

      if( IsVAnim )
      {
         pDGObjs->BP_VAnimBuffer = (unsigned int)_BP_GenerateVAnimVertexBuffer(pMesh, pDGObjs);
      }
      else
      {
         pDGObjs->BP_VAnimBuffer = NULL;
      }
   }
}

//----------------------------------------------------------------------------

namespace NObjLocalParam
{
   int gIsVAnim = 0;
   CDynamicVertexBufferPoolChunk_UT* gpAnimatedVertexBuffer = 0;
   CMesh *pLastMeshRendered = NULL;
   CDynamicVertexBuffer *pPreshadeBuffer = NULL;

   static void FlushObjLocalParamForMesh( CMesh *pMesh, SBP_OBJ_Render* pPacket )
   {
      unsigned int unit = pPacket->unit;
      // If we just worked on this mesh, then punt
      if ( pMesh == pLastMeshRendered )
         return;

      CDynamicVertexBufferPoolChunk_UT* pAnimatedVertexBuffer = (CDynamicVertexBufferPoolChunk_UT*)pPacket->BP_animatedVertexBuffer;
      gIsVAnim = pAnimatedVertexBuffer != NULL;

      bool const hasPreshade = pPreshadeBuffer != 0;

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);
      binding.Set(kVDU_TexCoord2, kVDS_Normal);
      binding.Set(kVDU_TexCoord3, kVDS_TexCoord1);
      binding.Set(kVDU_TexCoord4, kVDS_TexCoord2);

      CMeshBuffers & meshBuffers = const_cast<CMeshBuffers&>(pMesh->GetMeshBuffers());

      // Only override the vertex buffers if we preshade or animate textures
      if ( pAnimatedVertexBuffer || hasPreshade )
      {
         // make a copy of the vertex data object.
         CVertexData vertexData = meshBuffers.VertexData();

         // get the DG_OBJS and determine if it has vertex animations applied to it.
         uint64 hash2 = 0;

         if ( pAnimatedVertexBuffer )
         {
            size_t const vertexBufferIndex = vertexData.AddBuffer( pAnimatedVertexBuffer, sizeof( PackedVertex ) );

            vertexData.SetAttribute(kVDS_Position , offsetof(PackedVertex, vx), kVDT_Short4, vertexBufferIndex);
            vertexData.SetAttribute(kVDS_Normal   , offsetof(PackedVertex, nx), kVDT_Short4, vertexBufferIndex);
            vertexData.SetAttribute(kVDS_TexCoord0, offsetof(PackedVertex, u) , kVDT_Short2, vertexBufferIndex);
            hash2 = ( uint64 )pAnimatedVertexBuffer;
         }

         if ( hasPreshade )
         {
            size_t const vertexBufferIndex = vertexData.AddBuffer( pPreshadeBuffer, sizeof( CVector4 ) );
            vertexData.SetAttribute(kVDS_Color0, 0, kVDT_Float4, vertexBufferIndex);
            hash2 ^= (uint64)pPreshadeBuffer;
         }

         vertexData.ClearCachedVertexDeclaration();
         gpRenderBackend->SetVertexData(binding, vertexData, (uint64)pMesh, hash2);
      }
      else
      {
         meshBuffers.SetVertexData( binding );
      }

      // Set Index Buffer
      gpRenderBackend->SetIndexData(meshBuffers.GetIndexBuffer());

      // Don't do it again
      pLastMeshRendered = pMesh;
   }
}

void BP_Obj_InitPacket(char* pData)
{
   SBP_OBJ_InitPacket* pPacket = (SBP_OBJ_InitPacket*)pData;

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
   gpRenderBackend->SetDepthWriteEnabled(true);
}

void BP_Obj_SortChainInitPacket()
{
   if( gObj_DisableSortChainZWrite )
   {
      gpRenderBackend->SetDepthWriteEnabled(false);

      BP_GS_SetAlphaTestEnabled( false );
//      gpRenderBackend->SetAlphaTestEnable(false);
      ObjShader::gIsSortChain = true;
   }
}

void BP_Obj_SortChainEndPacket()
{
   gpRenderBackend->SetDepthWriteEnabled(true);
   //gpRenderBackend->SetAlphaTestEnable(true); // This gets set by every BP_GS_SetupAlphaFailPass()
   ObjShader::gIsSortChain = false;
}

//----------------------------------------------------------------------------

void BP_Obj_LocalParam(char* pData)
{
   SBP_OBJ_LocalParam* pPacket = (SBP_OBJ_LocalParam*)pData;

   extern CVector4 gBP_FogColor;
#if BP_VITA
   ObjShader::SVitaLocalParams *pParams = gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<ObjShader::SVitaLocalParams>( BUFFER_LOCAL_PARAMS );
   pParams->mFogColor = gBP_FogColor.AsF16();
   pParams->mFogParam.mX = pPacket->fogParam.x;
   pParams->mFogParam.mY = pPacket->fogParam.y;
   pParams->mFogParam.mZ = pPacket->fogParam.z;
   pParams->mFogParam.mW = pPacket->fogParam.w;
#else
   gpRenderBackend->SetVertexRegisters(kSReg_FogColor, 1, &gBP_FogColor);
   gpRenderBackend->SetVertexRegisters(kSReg_FogParam, 1, (CVector4 const*)&pPacket->fogParam);
#endif

   NObjLocalParam::pLastMeshRendered = NULL;
   NObjLocalParam::pPreshadeBuffer = reinterpret_cast<CDynamicVertexBuffer *>( pPacket->preshadeBuffer );
}

//----------------------------------------------------------------------------

class CObjRender
{
public:
   SBP_OBJ_Render* const   mpPacket;
   CMesh* const            mpMesh;
   CMeshChunk const *      pFirstMeshChunk;

   int const         isIR;
   int const         isTexture;
   int const         isFog;
   int const         isLight;
#if BP_VITA
   int const         isMultipartPacket;
#endif

#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
   uint32 minVertex;
   uint32 maxVertex;
#endif
   int minVertAlpha;
   int maxVertAlpha;
   int indexCount;

   BPE_FORCEINLINE CObjRender(SBP_OBJ_Render* pPacket, CMesh* pMesh)
      :  mpPacket(pPacket)
      ,  mpMesh(pMesh)
      ,  isIR(pPacket->isIRMode)
      ,  isTexture(!pPacket->isIRMode)
      ,  isFog((pPacket->obj_flag & DG_FLAG_NOFOG) ? 0 : 1)
      ,  isLight((pPacket->obj_flag & DG_FLAG_PAINT) ? 0 : 1)
#if BP_VITA
      ,  isMultipartPacket( pPacket->BP_precomp && pPacket->BP_precomp->mpNextInterior )
#endif
   {
//         isFog = 0;
   }

   BPE_NOINLINE
   void Setup()
   {
      minVertAlpha = 0x000000FF;
      maxVertAlpha = 0;

#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
      minVertex = gkUint32Max;
      maxVertex = gkUint32Min;
#endif

      indexCount = 0;

      int chunkStart, chunkCount;

      mpMesh->GetChunkRangeForUnit(mpPacket->unit, &chunkStart, &chunkCount);

      pFirstMeshChunk = &mpMesh->GetMeshChunks()[chunkStart + mpPacket->startPacket];

      for( int i = 0; i < mpPacket->packetCount; ++i )
      {
         int const packetIdx = mpPacket->startPacket + i;
         CMeshChunk const * pMeshChunk = &mpMesh->GetMeshChunks()[chunkStart + packetIdx];
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
         minVertex = bpe::min_val(minVertex, pMeshChunk->mVertexBufferOffset);
         maxVertex = bpe::max_val(maxVertex, pMeshChunk->mVertexBufferOffset + pMeshChunk->mVertexCount);
#endif
         int const minAlpha = (pMeshChunk->mVertColorMin & 0x000000FF);
         int const maxAlpha = (pMeshChunk->mVertColorMax & 0x000000FF);
         minVertAlpha = bpe::min_val(minVertAlpha, minAlpha);
         maxVertAlpha = bpe::max_val(maxVertAlpha, maxAlpha);

         indexCount += pMeshChunk->mIndicesCount;
      }

      if( mpPacket->flag & (DG_PACKFLAG_CULLAUTO|DG_PACKFLAG_CULLON) )
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_CCW);
      else 
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

#if BP_VITA
      BPE_ASSERT( !mpPacket->BP_precomp || mpPacket->BP_precomp->mIsStart, "Not start packet?!" );
      if ( isMultipartPacket )
      {
         ObjShader::sObjNeedsVertexUniformFlush = true;
      }
      else if ( /* !isMultipartPacket (is implied) && */ ObjShader::sObjNeedsVertexUniformFlush )
      {
         gpRenderBackend->SetUnsafeVertexUniformBuffer( BUFFER_PRERENDER_PARAMS, ObjShader::spLastSetUniformBuffer );
         ObjShader::sObjNeedsVertexUniformFlush = false;
      }
#endif

   }

   BPE_NOINLINE
   void Teardown()
   {
#if 0
      if ( mpPacket->BP_precomp )
      {
         // We need to flip the bits on the render clocks at the end
         SBP_OBJ_Render *pPacket = mpPacket;

         for ( ; pPacket ; pPacket = pPacket->BP_precomp->mpNextInterior )
         {
            pPacket->BP_precomp->mpUniformBuffers[ gBP_RB_ReadlockedBuffer ] = NULL;
         }

         BPE_ASSERT( mpPacket->BP_precomp->mpUniformBuffers[ gBP_RB_ReadlockedBuffer ] == NULL, "Verify" );
      }
#endif
   }

   inline void SetupTextureParameterUniforms( DG_TEX_BP *pTex )
   {
      if( NObjLocalParam::gIsVAnim )
      {
         real32  uvScale = 1.0f / 4096.0f;

         real32 const uScale = 1.0f / pTex->u_scale;
         real32 const vScale = 1.0f / pTex->v_scale;
         real32 const uOffset = -pTex->u_offset * uScale;
         real32 const vOffset = -pTex->v_offset * vScale;

         CVector4 const uvScaleOffset(uvScale * uScale, uvScale * vScale, uOffset, vOffset);
#if BP_VITA
         gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<ObjShader::SVitaTextureParams>( BUFFER_TEXTURE_PARAMS )
            ->mTexScaleOffset = uvScaleOffset;
#else
         gpRenderBackend->SetVertexRegisters(ObjShader::kReg_TexScaleOffset, 1, &uvScaleOffset);
#endif
      }
   }

   void SetupTextureParameters(DG_TEX_BP* pTex)
   {
      BP_SetTexture(pTex, 0);
      if( !isIR )
         BP_GS_SetAlpha(pTex->tex_trans.alpha.data);
      else
         BP_GS_SetAlpha(0);

      SetupTextureParameterUniforms( pTex );
   }

   void RenderPrims()
   {
#if BP_VITA
      if ( isMultipartPacket )
      {
         SBP_OBJ_Render const *pCurrent = mpPacket;
         int const renderClock = gBP_RB_ReadlockedBuffer;

         // Render the first one conventionally
         for ( ; pCurrent ; pCurrent = pCurrent->BP_precomp->mpNextInterior )
         {
            if ( pCurrent->BP_precomp->mLastFrameTouched[ renderClock ] == DG_Renderframe_FrameCount )
            {
               SBP_OBJ_PreRender_Vita *pPreRender = pCurrent->BP_precomp->mpUniformBuffers[ renderClock ];

               if ( pPreRender )
               {
                  gpRenderBackend->SetUnsafeVertexUniformBuffer( BUFFER_PRERENDER_PARAMS, 
                     pPreRender->mpUniformBufferPtr );

                  gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, 
                     pFirstMeshChunk->mIndexBufferOffset, indexCount);

                  pCurrent = pCurrent->BP_precomp->mpNextInterior;
                  break;
               }
            }
         }

         // Now render the rest as repeats
         for ( ; pCurrent; pCurrent = pCurrent->BP_precomp->mpNextInterior )
         {
            if ( pCurrent->BP_precomp->mLastFrameTouched[ renderClock ] == DG_Renderframe_FrameCount )
            {
               SBP_OBJ_PreRender_Vita *pPreRender = pCurrent->BP_precomp->mpUniformBuffers[ renderClock ];

               if ( pPreRender )
               {
                  gpRenderBackend->SetUnsafeVertexUniformBuffer( BUFFER_PRERENDER_PARAMS, 
                     pPreRender->mpUniformBufferPtr );

                  gpRenderBackend->VitaRepeatPriorDraw();
               }
            }
         }
      }
      else
#endif // BP_VITA
      {
         gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
            minVertex, maxVertex - minVertex, 
#endif
            pFirstMeshChunk->mIndexBufferOffset, indexCount);
      }

   }

   void GetMinMaxCombineAlpha( DG_TEX_BP *pTex, int *pMinCombineAlpha, int *pMaxCombineAlpha )
   {
      int maxTextureAlpha;
      int minTextureAlpha;
      int maxVertexAlpha;
      int minVertexAlpha;

      if ( isTexture && pTex && pTex->BP_TextureHandle )
      {
         CBaseTexture const * pTexture = (CBaseTexture*)pTex->BP_TextureHandle;
         maxTextureAlpha = (pTexture->mMaxRGBA&0x000000FF);
         minTextureAlpha = (pTexture->mMinRGBA&0x000000FF);
      }
      else
      {
         // If we are not texturing, then we are at full alpha,
         // which means 128 all around.
         maxTextureAlpha = 0x80;
         minTextureAlpha = 0x80;
      }

      if( isLight )
      {
         // gMaxConstantParamAlpha && gMinConstantParamAlpha are always 1.0 for KMS.fx
         maxVertexAlpha = 0x80;
         minVertexAlpha = 0x80;
      }
      else
      {
         maxVertexAlpha = (maxVertAlpha);
         minVertexAlpha = (minVertAlpha);
      }

      *pMaxCombineAlpha = (maxTextureAlpha*maxVertexAlpha)>>6; // we are only dividing by 64 to simulate *2 alpha in shader code
      *pMinCombineAlpha = (minTextureAlpha*minVertexAlpha)>>6; // we are only dividing by 64 to simulate *2 alpha in shader code
   }

   void RenderAllPrimitives(DG_TEX_BP* pTex, ObjShader::EUVType uvType)
   {
      int AlphaFailPassCount_Optimized = gGS_AlphaFailPassCount;
      SBP_TestState* pOptimizeTestState = NULL;
      if( gObj_OptimizeAlphaTest && gGS_AlphaFailPassCount > 1 )
      {
         int minCombineAlpha;
         int maxCombineAlpha;
         GetMinMaxCombineAlpha( pTex, &minCombineAlpha, &maxCombineAlpha );
         AlphaFailPassCount_Optimized = BP_GS_AlphaFailPass_Optimize(minCombineAlpha, maxCombineAlpha, minCombineAlpha, &pOptimizeTestState);
      } 

      for( int alphaTestPass = 0; alphaTestPass < AlphaFailPassCount_Optimized; ++alphaTestPass )
      {
         BP_GS_SetupAlphaFailPass_Optimize(gGS_AlphaFailPassStart + alphaTestPass, pOptimizeTestState);
         BP_BeginShader(ObjShader::Shader(isTexture, isLight, gObj_KillFog ? 0 : isFog, NObjLocalParam::gIsVAnim, uvType), NULL, 0);
         RenderPrims();
         ++gNumRendered;
      }
   }

   BPE_NOINLINE
   void RenderSingle()
   {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("KMS Single");
      //BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("KMS Single: %s", mpMesh->mDebugName.c_str()));
#endif
      NObjLocalParam::FlushObjLocalParamForMesh( mpMesh, mpPacket );

      if (gObj_IROnly && !isIR)
         return;
      DG_TEX_BP* pTex = mpPacket->tex[0];
      if( pTex )
      {
         SetupTextureParameters(pTex);
         if( !ObjShader::gIsSortChain )
         {
            RenderAllPrimitives(pTex, ObjShader::kUV_Set0);
         }
         else
         {
            BP_BeginShader(ObjShader::Shader(isTexture, isLight, gObj_KillFog ? 0 : isFog, NObjLocalParam::gIsVAnim, ObjShader::kUV_Set0), NULL, 0);
            RenderPrims();
         }
      }
   }

   void RenderPrims3PassEnvMap()
   {
      int minCombineAlpha;
      int maxCombineAlpha;
      int AlphaFailPassCount_Optimized = gGS_AlphaFailPassCount;
      SBP_TestState* pOptimizeTestState = NULL;

      SetupTextureParameterUniforms( mpPacket->tex[0] );
      GetMinMaxCombineAlpha( mpPacket->tex[0], &minCombineAlpha, &maxCombineAlpha );
      if( gObj_OptimizeAlphaTest && gGS_AlphaFailPassCount > 1 )
      {
         AlphaFailPassCount_Optimized = BP_GS_AlphaFailPass_Optimize(minCombineAlpha, maxCombineAlpha, minCombineAlpha, &pOptimizeTestState);
      } 

      BP_SetTexture( mpPacket->tex[0], 0 );
      BP_SetTexture( mpPacket->tex[1], 1 );
      BP_SetTexture( mpPacket->tex[2], 2 );

      if ( minCombineAlpha >= 0xFF )
      {
         BP_GS_SetAlpha_NoBlend( mpPacket->tex[0]->tex_trans.alpha.data );
      }
      else
      {
         BP_GS_SetAlpha_NormalBlend( mpPacket->tex[0]->tex_trans.alpha.data );
      }

      {
         DG_TEX_BP const *pTex = mpPacket->tex[2];
         CVector4 const envmapParams(pTex->tex_trans.vec1.vx / pTex->u_scale, 
            pTex->tex_trans.vec1.vy / pTex->v_scale, 
            (pTex->tex_trans.vec2.vx - pTex->u_offset) / pTex->u_scale, 
            (pTex->tex_trans.vec2.vy - pTex->v_offset) / pTex->v_scale);

#if BP_VITA
         gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<ObjShader::SVitaEnvmapParams>( BUFFER_ENVMAP_PARAMS )->mEnvmapParams = envmapParams.AsF16();
#else
         gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &envmapParams);
#endif
      }

      for( int alphaTestPass = 0; alphaTestPass < AlphaFailPassCount_Optimized; ++alphaTestPass )
      {
         BP_GS_SetupAlphaFailPass_Optimize(gGS_AlphaFailPassStart + alphaTestPass, pOptimizeTestState);
         BP_BeginShader( ObjCharShader::Shader( isLight, isFog, NObjLocalParam::gIsVAnim ), NULL, 0 );
         RenderPrims();
         ++gNumRendered;
      }
   }

   void RenderMulti()
   {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("KMS Multi");
#endif

      NObjLocalParam::FlushObjLocalParamForMesh( mpMesh, mpPacket );

      int const isTex0 = mpPacket->flag & DG_PACKFLAG_TEX0;
      int const isTex1 = mpPacket->flag & DG_PACKFLAG_TEX1;
      int const isTex2 = mpPacket->flag & DG_PACKFLAG_TEX2;
      int const isUV0 = mpPacket->flag & DG_PACKFLAG_UV0;
      int const isUV1 = mpPacket->flag & DG_PACKFLAG_UV1;
      int const isUV2 = mpPacket->flag & DG_PACKFLAG_UV2;
      int const isEmap = mpPacket->flag & DG_PACKFLAG_EMAP;
      int const isSmap = mpPacket->flag & DG_PACKFLAG_SMAP;

      static int const skSpecialBlendPassCheck = 
         DG_PACKFLAG_TEX0 | DG_PACKFLAG_TEX1 | DG_PACKFLAG_TEX2 | DG_PACKFLAG_UV0 | DG_PACKFLAG_UV1 |                   DG_PACKFLAG_EMAP;
      static int const skSpecialBlendPassMask =  
         DG_PACKFLAG_TEX0 | DG_PACKFLAG_TEX1 | DG_PACKFLAG_TEX2 | DG_PACKFLAG_UV0 | DG_PACKFLAG_UV1 | DG_PACKFLAG_UV2 | DG_PACKFLAG_EMAP | DG_PACKFLAG_SMAP;

      if( 
         ( mpPacket->flag & skSpecialBlendPassMask ) == skSpecialBlendPassCheck && 
         isTexture &&
         ( mpPacket->tex[0]->tex_trans.alpha.data & 0xFF ) == 0x44 && 
         ( mpPacket->tex[1]->tex_trans.alpha.data & 0xFF ) == 0x42 &&
         ( mpPacket->tex[2]->tex_trans.alpha.data & 0xFF ) == 0x58 &&
         gAS_Obj_Optimize3PassEnv)
      {
         // We have our special pass type!
         RenderPrims3PassEnvMap();
         return;
      }

      if (gObj_IROnly && !isIR)
         return;

      // Render first pass
      if( isTex0 )
      {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 0");
#endif
         DG_TEX_BP* pTex = mpPacket->tex[0];
         if( pTex != NULL )
         {
            SetupTextureParameters(pTex);

            RenderAllPrimitives(pTex, ObjShader::kUV_Set0);
            }
         }

      // Render second pass
      if( isTex1 )
      {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 1");
#endif

         DG_TEX_BP* pTex = mpPacket->tex[1];
         if( pTex != NULL )
         {
            SetupTextureParameters(pTex);
            RenderAllPrimitives(pTex, ObjShader::kUV_Set1);
         }
      }

      // Render third pass
      if( isTex2 )
      {
         DG_TEX_BP* pTex = mpPacket->tex[2];
         if( pTex != NULL )
         {
            SetupTextureParameters(pTex);

            if( isUV2 )
            {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
               BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 2");
   #endif
               RenderAllPrimitives(pTex, ObjShader::kUV_Set2);
            }
            else if( isSmap )
            {
               CVector4 const envmapParams(pTex->tex_trans.vec1.vx / pTex->u_scale, pTex->tex_trans.vec1.vy / pTex->v_scale, (pTex->tex_trans.vec2.vx - pTex->u_offset) / pTex->u_scale, (pTex->tex_trans.vec2.vy - pTex->v_offset) / pTex->v_scale);

#if BP_VITA
               gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<ObjShader::SVitaEnvmapParams>( BUFFER_ENVMAP_PARAMS )->mEnvmapParams = envmapParams.AsF16();
#else
               gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &envmapParams);
#endif
               RenderAllPrimitives(pTex, ObjShader::kUV_Smap);
            }
            else if( gObj_EnableEnvmap && isEmap )
            {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
               BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 2 Env");
#endif

               CVector4 const envmapParams(pTex->tex_trans.vec1.vx / pTex->u_scale, 
                  pTex->tex_trans.vec1.vy / pTex->v_scale, 
                  (pTex->tex_trans.vec2.vx - pTex->u_offset) / pTex->u_scale, 
                  (pTex->tex_trans.vec2.vy - pTex->v_offset) / pTex->v_scale);

#if BP_VITA
               gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<ObjShader::SVitaEnvmapParams>( BUFFER_ENVMAP_PARAMS )->mEnvmapParams = envmapParams.AsF16();
#else
               gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &envmapParams);
#endif

               RenderAllPrimitives(pTex, ObjShader::kUV_Envmap);
            }
         }
      }
   }
};

//----------------------------------------------------------------------------

#if BP_VITA

void BP_Obj_PreRender_MakeVitaUniforms( SBP_OBJ_PreRender_Vita *pOut )
{
   DG_OBJ *obj = pOut->mpOrigObj;
   ObjShader::SVitaPreRenderParams *pPreRender = reinterpret_cast<ObjShader::SVitaPreRenderParams *>( gpRenderBackend->GetVertexBufferPool_UT()->AllocChunkUnsafeVita( sizeof( ObjShader::SVitaPreRenderParams ) ) );
   KP_MatrixData const *pMatrixData = reinterpret_cast<KP_MatrixData const *>( obj->matrix_addr );

   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&obj->screen, pPreRender->mPersp );
   BP_Matrix44_to_VS_Matrix43((real32 const*)&obj->inv_mat, pPreRender->mCorrection );
   BP_Matrix44_to_VS_Matrix44((real32 const*)&pMatrixData->local_light, pPreRender->mLightDir);

   real32 temp[16];
   BP_Matrix44_to_VS_Matrix44((real32 const*)&pMatrixData->local_color, temp);
   BP_Convert_Float32x4x4_to_Float16x4x4( &pPreRender->mLightCol, temp );

   pOut->mpUniformBufferPtr = pPreRender;
}

//----------------------------------------------------------------------------

void BP_Obj_PreRenderVita( char *pData )
{
   SBP_OBJ_PreRender_Vita *pPacket = (SBP_OBJ_PreRender_Vita *) pData;

   gpRenderBackend->SetUnsafeVertexUniformBuffer( BUFFER_PRERENDER_PARAMS, pPacket->mpUniformBufferPtr );
   ObjShader::spLastSetUniformBuffer = pPacket->mpUniformBufferPtr;
   ObjShader::sObjNeedsVertexUniformFlush = false;
}

#endif
//----------------------------------------------------------------------------

void BP_Obj_PreRender( char *pData )
{
   SBP_OBJ_PreRender *pPacket = (SBP_OBJ_PreRender *) pData;

#if BP_VITA
   ObjShader::SVitaPreRenderParams *pPreRender = gpRenderBackend->AllocOneFrameVertexUniformBufferTyped<ObjShader::SVitaPreRenderParams>( BUFFER_PRERENDER_PARAMS );

   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, pPreRender->mPersp );
   BP_Matrix44_to_VS_Matrix43((real32 const*)&pPacket->eye_inv, pPreRender->mEyeInv );
   BP_Matrix44_to_VS_Matrix43((real32 const*)&pPacket->correction, pPreRender->mCorrection );

   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightDir, pPreRender->mLightDir);

   real32 temp[16];
   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightCol, temp);
   BP_Convert_Float32x4x4_to_Float16x4x4( &pPreRender->mLightCol, temp );

   ObjShader::spLastSetUniformBuffer = pPreRender;
   ObjShader::sObjNeedsVertexUniformFlush = false;
#else
   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ObjShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_Matrix44_to_VS_Matrix43((real32 const*)&pPacket->eye_inv, temp);
   gpRenderBackend->SetVertexRegisters(ObjShader::kReg_EyeInv, 3, (CVector4 const*)temp);

   BP_Matrix44_to_VS_Matrix43((real32 const*)&pPacket->correction, temp);
   gpRenderBackend->SetVertexRegisters(ObjShader::kReg_Correction, 3, (CVector4 const*)temp);

   //if( pPacket->BP_Flags & kPRF_Light )
   {
      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightDir, temp);
      gpRenderBackend->SetVertexRegisters(kSReg_LightDir, 4, (CVector4 const*)temp);

      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightCol, temp);
      gpRenderBackend->SetVertexRegisters(kSReg_LightCol, 4, (CVector4 const*)temp);
   }
#endif
}

//----------------------------------------------------------------------------

void BP_Obj_Render(char* pData)
{
   if( !gObj_EnableSingleTex )
      return;

   SBP_OBJ_Render* pPacket = (SBP_OBJ_Render*)pData;

   ++gNumPackets;
#if 0
   gNumPacketGroups += pPacket->count;
   gNumTexDiff += pPacket->texDiffCount;
   gNumFlagDiff += pPacket->flagDiffCount;
#endif

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh ) 
   {
      CObjRender render(pPacket, pMesh);
      render.Setup();
      render.RenderSingle();
      render.Teardown();
   }
}

//----------------------------------------------------------------------------

void BP_Obj_RenderMulti(char* pData)
{
   if( !gObj_EnableMultiTex )
      return;

   SBP_OBJ_Render* pPacket = (SBP_OBJ_Render*)pData;

#if BP_VITA
   BPE_ASSERT( pPacket->BP_precomp == NULL, "Precomp passed to render multi!" );
#endif

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh ) 
   {
      CObjRender render(pPacket, pMesh);
      render.Setup();
      if( !gObj_ForceSingleTex )
         render.RenderMulti();
      else
         render.RenderSingle();
      render.Teardown();
   }
}

//----------------------------------------------------------------------------

void BP_AllocPreshadeBuffer(void* pDGObjs)
{
   DG_OBJS* pModel = (DG_OBJS*)pDGObjs;

   if( pModel->n_models > 0 )
   {
      pModel->BP_PreshadeBuffer = (unsigned int)new CDynamicVertexBuffer();
   }
}

//----------------------------------------------------------------------------

void BP_FreePreshadeBuffer(void* pDGObjs)
{
   DG_OBJS* pModel = (DG_OBJS*)pDGObjs;

   CDynamicVertexBuffer* pVertexBuffer = (CDynamicVertexBuffer*)pModel->BP_PreshadeBuffer;
   if( pVertexBuffer )
   {
      gpRenderBackend->DeleteRenderResourceFrameDelayed((IObject*)pVertexBuffer);
   }

   pModel->BP_PreshadeBuffer = 0;
}

//----------------------------------------------------------------------------

void BP_UpdatePreshadeBuffer(void* pDGObjs, u_int const * pMGSVertexColors)
{
   DG_OBJS* pModel = (DG_OBJS*)pDGObjs;

   if ( pModel->n_models == 0 )
   {
      return;
   }

   // Pick the first mesh
   CMesh const *pMesh = (CMesh const *) pModel->objs[0].BP_Model;

   for ( int i = 1; i < pModel->n_models; ++i )
   {
      if ( reinterpret_cast<CMesh const *>( pModel->objs[i].BP_Model ) != pMesh )
      {
         printf( "BP ERROR: Preshade model made up of multiple meshes!\n" );
         BP_RENDER_TODO_BREAK;
         BP_DAK_BREAK;
         BP_MARCO_BREAK;
         return;
      }
   }

   if( pMesh )
   {
      CMeshBuffers const & meshBuffers = pMesh->GetMeshBuffers();

      if ( CDynamicVertexBuffer* pVertexBuffer = (CDynamicVertexBuffer*)pModel->BP_PreshadeBuffer )
      {
         int const vertexCount = meshBuffers.GetVertexCount();
         uint32 const * vertexIndices = meshBuffers.GetOriginalVertexIndices();

         // output color buffer
         CVector4* pPreshadeColor = (CVector4*)pVertexBuffer->Lock(sizeof(CVector4) * vertexCount);

         for( int i = 0; i < vertexCount; ++i )
         {
            int const originalIndex = vertexIndices[i];
            uint32 const mgsColor = pMGSVertexColors[originalIndex];

            CVector4 const color(((mgsColor & 0x000000FF) >>  0) / 128.0f, 
                                 ((mgsColor & 0x0000FF00) >>  8) / 128.0f, 
                                 ((mgsColor & 0x00FF0000) >> 16) / 128.0f, 
                                 ((mgsColor & 0xFF000000) >> 24) / 128.0f);

            pPreshadeColor[i] = color;
         }

         pVertexBuffer->Unlock();
      }
      else
      {
         printf( "BP WARNING: Trying to preshade something with no preshade buffer!\n" );
         BP_RENDER_TODO_BREAK;
      }
   }
}

//----------------------------------------------------------------------------

void BP_Obj_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   int const renderMenu = BP_DebugMenu_GetMenu("Render");

   int const debugMenu = BP_DebugMenu_AddMenu("Obj", renderMenu);
   BP_DebugMenu_AddBool(debugMenu, "Render Single Tex", &gObj_EnableSingleTex);
   BP_DebugMenu_AddBool(debugMenu, "Render Multi Tex", &gObj_EnableMultiTex);
   BP_DebugMenu_AddBool(debugMenu, "Force Single Tex", &gObj_ForceSingleTex);
   BP_DebugMenu_AddBool(debugMenu, "Enable Envmap", &gObj_EnableEnvmap);
   BP_DebugMenu_AddBool(debugMenu, "Enable Vertex Anim", &gObj_EnableVertexAnim);
   BP_DebugMenu_AddBool(debugMenu, "Disable SortChain ZWrite", &gObj_DisableSortChainZWrite);

   BP_DebugMenu_AddBool(debugMenu, "Enable optimize alpha test", &gObj_OptimizeAlphaTest);
   BP_DebugMenu_AddBool(debugMenu, "Enable optimize alpha test every other frame", &gObj_OptimizeAlphaTest_EnableFrameToggle);

   BP_DebugMenu_AddBool(debugMenu, "3 Pass Optimized", &gAS_Obj_Optimize3PassEnv );
   BP_DebugMenu_AddBool(debugMenu, "3 Pass Optimized Ping Pong", &gAS_Obj_Optimize3PassEnv_Pingpong );

   BP_DebugMenu_AddBool(debugMenu, "Show Stats", &gObj_ShowStats);

   BP_DebugMenu_AddBool(debugMenu, "Kill Fog", &gObj_KillFog);
   BP_DebugMenu_AddBool(debugMenu, "IR Only", &gObj_IROnly);
#endif
}

int strendswith( char const *src, char const *sub )
{
   int srclen = strlen( src );
   int sublen = strlen( sub );
   
   if ( srclen >= sublen )
   {
      // Check the end of src if the src is at least as big as sub
      return !strcmp( src + srclen - sublen, sub );
   }
   else
   {
      // src is smaller than sub. Punt.
      return 0;
   }
}

int BP_Obj_CheckInsideAreaWhereWindowRainNoMSAA()
{
   return strendswith( GM_GetArea(), "01b" );
}

int BP_Obj_CheckForceNoMSAAForTRIFile( int tri_file )
{
   switch ( tri_file )
   {
   // AS(JM) - The following are TRI FILE ID's
   // that will cause the entire model to not be in the MSAA
   // scene
   case 0x003c4e81: // w24a.tri - small lines holding up drawbridge
   case 0x003c4b01: // w15a.tri - small lines on sides of building
   case 0x002c0998: // w15_bg.tri - *15a,b,maybe others - crane model - small lines
   case 0x003c5282: // w31b - lots of shadowy overlays MGSTWO-2879
   
   // Uses of w10c_wire00a_alp
   case 0x00573de0:

   // Uses of w10c_line00_alp_add_ovl
   case 0x002a8998:
   case 0x002bad19:
   case 0x003c4ac1:
   case 0x003c4ac2:
   case 0x003c4b02:
   case 0x003c4b41:
   case 0x003c4b61:
   case 0x003c4b81:
   case 0x003c4e82:
   case 0x003c4ea1:
   case 0x003c4ec1:
   case 0x003c4ec2:
   case 0x003c4f01:
   case 0x003c4f02:
   case 0x003c4f04:
   case 0x0051ea37:
   case 0x0089cc57:
   case 0x0089e14b:
   case 0x008a5057:
   case 0x00901b2c:
   case 0x00a05e4c:
   case 0x00dc4f0a:

      return 1;
   case 0x009444cb: 
      // w12a - AS(JM) fences.  Note that I have no idea where this tri number comes from, so filter to *12a
      if ( strendswith( GM_GetArea(), "12a" ) )
      {
         return 1;
      }
      break;

   case 0x0088d077:
   case 0x0088d078:
   case 0x0088d079:
      // w01b(and a01b) - AS(JM) - Filter this for windows that have rain, and are 
      // visible through a locker. MGSTWO-3420
      if ( BP_Obj_CheckInsideAreaWhereWindowRainNoMSAA() )
      {
         return 1;
      }
      break;

   default:
      break;
   }

   return 0;
}
