#define GRAPHICS_UTIL_DEFAULT_DISPLAY_MAX_PENDING_SWAPS		2								
//----------------------------------------------------------------------------
// VTACRenderBackend.cpp
// Bluepoint/Armature
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CIndexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Material/VTA/VTACCompiledShader.h"
#include "Renderer/Base/Backend/VTA/VTACTexture.h"
#include "Renderer/Base/Backend/VTA/NVTAState.h"

#include "Engine/System/VTAThreadPriorities.h"

#include <gxm.h>
#include <display.h>
#include <kernel.h>
#include <common_dialog/common_api.h>

//#undef BPE_CHECK_SCE

#ifndef GOLD_VERSION

#  include <libsysmodule.h>

#  include <razor_capture.h>
#  include <razor_hud.h>
#  include <libperf.h>

// Enable the following define to create a Razor capture file.
#  define ENABLE_RAZOR_CAPTURE
//#  define ENABLE_DETAILED_TIMING
//#  define ENABLE_RAZOR_LIVEMETRICS
//#  define ENABLE_RAZOR_HUD
//#  define VITA_PROFILE_STATE_CHANGES

#endif // defined(GOLD_VERSION)

extern const SceGxmProgram _binary_clear_v_gxp_start;
extern const SceGxmProgram _binary_clear_f_gxp_start;
extern const SceGxmProgram _binary_scissor_f_gxp_start;
extern const SceGxmProgram _binary_mask_f_gxp_start;
extern const SceGxmProgram _binary_maskhalf_f_gxp_start;
extern const SceGxmProgram _binary_upscale_v_gxp_start;
extern const SceGxmProgram _binary_upscale_f_gxp_start;
extern const SceGxmProgram _binary_downclear_v_gxp_start;
extern const SceGxmProgram _binary_downclear_f_gxp_start;

//#define VITA_DEBUG_DEPTH

// Force the game to run no faster than 30fps
#define ARMATURE_FRAME_LOCK_30HZ 1

#ifdef VITA_DEBUG_DEPTH
int gVTADebugDepthDrawNum = -1;
#endif

namespace
{
#if MGS_VERSION == 2
   int const kHostMemorySize = 60 * 1024 * 1024;
#elif MGS_VERSION == 3
   int const kHostMemorySize = 60 * 1024 * 1024;
#endif
   int const kMainMemoryPoolSize = kHostMemorySize; // - kDefaultCommandBufferSize;
   int const kMainMemoryHandleCount = 16384; 

   int sGraphicsPreAllocLPDDRSize = 0;
   int sGraphicsPreAllocCDRAMSize = 0;

#ifdef ENABLE_RAZOR_LIVEMETRICS
   int const kLMNumBuffers = 2;
   int const kLMBufferSize = 256 * 1024;
   static void *sLMBuffers[kLMNumBuffers];
#endif

   size_t const skNoPrimIndexCount = 65535;

   BPE_CTASSERT( ( (~0xFFF) & ( kMainMemoryPoolSize + 0xFFF ) ) == kMainMemoryPoolSize );

   int const kVideoMemoryHandleCount = 8192;

   inline SceGxmIndexFormat index_format_to_gxm( EIndexType it )
   {
      static const SceGxmIndexFormat skIndexFormatTable[] =
      {
         SCE_GXM_INDEX_FORMAT_U32,
         SCE_GXM_INDEX_FORMAT_U16
      };

      BPE_CTASSERT( kIT_Uint16 == 1 );
      BPE_CTASSERT( kIT_Uint32 == 0 );

      return skIndexFormatTable[ it ];
   }

   inline SceGxmPrimitiveType prim_type_to_gxm( CMeshChunk::EPrimitive primitive )
   {
      switch ( primitive )
      {
      case CMeshChunk::kPrimitive_TriangleList:
         return SCE_GXM_PRIMITIVE_TRIANGLES;
      case CMeshChunk::kPrimitive_TriangleStrip:
         return SCE_GXM_PRIMITIVE_TRIANGLE_STRIP;
      case CMeshChunk::kPrimitive_TriangleFan:
         return SCE_GXM_PRIMITIVE_TRIANGLE_FAN;
      case CMeshChunk::kPrimitive_LineList:
         return SCE_GXM_PRIMITIVE_LINES;
      case CMeshChunk::kPrimitive_LineStrip:
         BPE_VERIFYA( false, "Line strips not supported" );
         return SCE_GXM_PRIMITIVE_POINTS;
      case CMeshChunk::kPrimitive_PointList:
         return SCE_GXM_PRIMITIVE_POINTS;
      default:
         BPE_VERIFYA( false, "Unknown prim type" );
         return SCE_GXM_PRIMITIVE_POINTS;
      }
   }

   inline SceGxmPolygonMode primitivetype_to_polygonmode(SceGxmPrimitiveType primType)
   {
      switch ( primType )
      {
      case SCE_GXM_PRIMITIVE_TRIANGLES:
      case SCE_GXM_PRIMITIVE_TRIANGLE_STRIP:
      case SCE_GXM_PRIMITIVE_TRIANGLE_FAN:
         return SCE_GXM_POLYGON_MODE_TRIANGLE_FILL;
      case SCE_GXM_PRIMITIVE_LINES:
         return SCE_GXM_POLYGON_MODE_LINE;
      case SCE_GXM_PRIMITIVE_POINTS:
         return SCE_GXM_POLYGON_MODE_POINT;
      case SCE_GXM_PRIMITIVE_TRIANGLE_EDGES:
         return SCE_GXM_POLYGON_MODE_TRIANGLE_LINE;
      default:
         BPE_ASSERTA( "Unknown polygon prim type" );
         return (SceGxmPolygonMode)-1;
      }
   }

   inline CTexture const *base_to_tex_ptr( CBaseTexture const *ptr ) { return static_cast<CTexture const *>( ptr ); }

}

namespace NVTATextureState
{
   static size_t const skNumTextureUnits = 32;
   SceGxmTexture sTextures[ skNumTextureUnits ];
   const int skDefaultLODBias = 31 - 4; // 31 is no bias, -4 biases it to -0.5 (0.125 each increment)

   struct SOtherTextureState
   {
      SOtherTextureState()
         : mLodBias( skDefaultLODBias )
         , mMinFilter( SCE_GXM_TEXTURE_FILTER_LINEAR )
         , mMagFilter( SCE_GXM_TEXTURE_FILTER_LINEAR )
         , mUAddrMode( SCE_GXM_TEXTURE_ADDR_CLAMP )
         , mVAddrMode( SCE_GXM_TEXTURE_ADDR_CLAMP )
      {
      }

      uint32_t mLodBias;
      SceGxmTextureFilter mMinFilter;
      SceGxmTextureFilter mMagFilter;
      SceGxmTextureAddrMode mUAddrMode;
      SceGxmTextureAddrMode mVAddrMode;
   };

   SOtherTextureState sOtherTextureStates[ skNumTextureUnits ];
   SceGxmTexture *sDirtyTextures[ skNumTextureUnits ];
   size_t sDirtyTextureCount = 0;
   uint32 sDirtyTextureState = 0;

   void SetTextureDirty( uint32 index )
   {
      uint32 bit = ( 1 << index );

      if ( !( sDirtyTextureState & bit ) )
      {
         sDirtyTextureState |= bit;
         sDirtyTextures[ sDirtyTextureCount++ ] = sTextures + index;
      }
   }

   void FlushDirtyTextures()
   {
      SceGxmContext *pContext = gpRenderBackend->Context();

      for ( size_t i = 0; i < sDirtyTextureCount; ++i )
      {
         SceGxmTexture *pTexture = sDirtyTextures[ i ];
         unsigned int const textureIndex = pTexture - sTextures;

         BPE_CHECK_SCE( sceGxmSetFragmentTexture( pContext, textureIndex, pTexture ) );
      }

      sDirtyTextureCount = 0;
      sDirtyTextureState = 0;
   }

   void FlushOtherState( int const texUnit )
   {
      SceGxmTexture *pTexture = sTextures + texUnit;

      SOtherTextureState const &other = sOtherTextureStates[ texUnit ];
      if ( sceGxmTextureGetType( pTexture ) != SCE_GXM_TEXTURE_LINEAR_STRIDED )
      {
         sceGxmTextureSetLodBias( pTexture, other.mLodBias );
         sceGxmTextureSetMinFilter( pTexture, other.mMinFilter );
         sceGxmTextureSetUAddrMode( pTexture, other.mUAddrMode );
         sceGxmTextureSetVAddrMode( pTexture, other.mVAddrMode );
      }

      sceGxmTextureSetMagFilter( pTexture, other.mMagFilter );

      SetTextureDirty( texUnit );
   }

   void SetTextureData( int const texUnit, SceGxmTexture const *texture )
   {
      sTextures[ texUnit ] = *texture;
      FlushOtherState( texUnit );
   }
}

static void *g_alloc(SceKernelMemBlockType type, SceUInt32 size, SceUInt32 alignment, SceUInt32 attribs, SceInt32 *uid)
{
   void	*mem = NULL;
   int		res;

   // CDRAM memblocks must be 256KiB aligned
   if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA) {
      if (alignment > 0x40000)
         return NULL;
      size = (size + 0x3ffffU) & ~0x3ffffU;

      sGraphicsPreAllocCDRAMSize += size;      
   }
   // LPDDR memblocks must be 4KiB aligned
   else {
      if (alignment > 0x1000)
         return NULL;
      size = (size + 0xfffU) & ~0xfffU;

      sGraphicsPreAllocLPDDRSize += size;
   }

   res = sceKernelAllocMemBlock("BPE-Render", type, size, NULL);
   if (res < SCE_OK)
      return NULL;
   *uid = res;

   res = sceKernelGetMemBlockBase(*uid, &mem);
   if (res != SCE_OK)
      return NULL;
   res = sceGxmMapMemory(mem, size, attribs);
   if (res != SCE_OK)
      return NULL;

   return mem;
}

static void g_free(SceUID uid)
{
   void *mem = NULL;
   sceKernelGetMemBlockBase(uid, &mem);
   sceGxmUnmapMemory(mem);
   sceKernelFreeMemBlock(uid);
}

static void *fragment_usse_alloc(uint32_t size, SceUID *uid, uint32_t *usseOffset)
{
   void	*mem = NULL;
   int		res;

   // align to memblock alignment for LPDDR
   size = (size + 0xfffU) & ~0xfffU;

   res = sceKernelAllocMemBlock("BPE", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL);
   if (res < SCE_OK)
      return NULL;
   *uid = res;

   res = sceKernelGetMemBlockBase(*uid, &mem);
   if (res != SCE_OK)
      return NULL;
   res = sceGxmMapFragmentUsseMemory(mem, size, usseOffset);
   if (res != SCE_OK)
      return NULL;

   return mem;
}

static void fragment_usse_free(SceUID uid)
{
   void *mem = NULL;
   sceKernelGetMemBlockBase(uid, &mem);
   sceGxmUnmapFragmentUsseMemory(mem);
   sceKernelFreeMemBlock(uid);
}

static void *combined_usse_alloc(uint32_t size, SceUID *uid, uint32_t *vertexUsseOffset, uint32_t *fragmentUsseOffset)
{
   void	*mem = NULL;
   int		res;

   // align to memblock alignment for LPDDR
   size = (size + 0xfffU) & ~0xfffU;

   res = sceKernelAllocMemBlock("BPE", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL);
   if (res < SCE_OK)
      return NULL;
   *uid = res;

   res = sceKernelGetMemBlockBase(*uid, &mem);
   if (res != SCE_OK)
      return NULL;
   res = sceGxmMapVertexUsseMemory(mem, size, vertexUsseOffset);
   if (res != SCE_OK)
      return NULL;
   res = sceGxmMapFragmentUsseMemory(mem, size, fragmentUsseOffset);
   if (res != SCE_OK)
      return NULL;

   return mem;
}

static void combined_usse_free(SceUID uid)
{
   void *mem = NULL;
   sceKernelGetMemBlockBase(uid, &mem);
   sceGxmUnmapFragmentUsseMemory(mem);
   sceGxmUnmapVertexUsseMemory(mem);
   sceKernelFreeMemBlock(uid);
}

static void *patcher_host_alloc(void *data, uint32_t size)
{
   return malloc(size);
}

static void patcher_host_free(void *data, void *mem)
{
   free(mem);
}

namespace
{
   struct SDisplay
   {
      void *address;
      int buffer_index;
#ifdef ENABLE_DETAILED_TIMING
      CStopWatch timer;
#endif
   };

   void cb_display(const void *data)
   {
      const SDisplay *disp = (const SDisplay *)data;
      SceDisplayFrameBuf framebuf;

#ifdef ENABLE_DETAILED_TIMING
      gpRenderBackend->SetFrameTime(disp->timer.GetElapsedTime() * 1000.0f);
#endif
      memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
      framebuf.size        = sizeof(SceDisplayFrameBuf);
      framebuf.base        = disp->address;
      framebuf.pitch       = CRenderBackend::DISPLAY_STRIDE;
      framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
      framebuf.width       = CRenderBackend::skDisplayWidth;
      framebuf.height      = CRenderBackend::skDisplayHeight;

#if defined(ARMATURE_FRAME_LOCK_30HZ)
      static unsigned short sPriorVblCount = 0;

      // If we're going too fast after the last swap, slow us down some
      // Note that this is done up here and not after the SetFrameBuf, as you can 
      // get a cascading slowdown effect if you want for both vsyncs in this thread.
      // This doesn't cause a wait for 2 frames immediately, simply a a wait for 2 vsyncs to happen 
      // after the last SetFrameBuf call, so if that time's elapsed on the CPU, then this function does nothing.
      if ( gpRenderBackend->GetTargetFPS() == CRenderBackend::kFPS_30 )
      {
         // If our FPS is at 30, then if we wait one frame during this two frame wait, we "can" run at 60.
         
         unsigned short vblCountStart = sceDisplayGetVcount();

         sceDisplayWaitSetFrameBufMulti( 2 );

         unsigned short vblCountEnd = sceDisplayGetVcount();
         unsigned short vblCountStartPlusOneWrapped = vblCountStart + 1; // wrap cause of unsigned shortness

         gpRenderBackend->s_dispDisplayHardwareAt60[ disp->buffer_index ] = ( vblCountStartPlusOneWrapped == vblCountEnd );

         // Even though we don't use the prior here, we set it for the case where we switch to 60
         sPriorVblCount = vblCountEnd;
      }
      else
      {
         // If we're trying to run at 60, simply look to see that we are increasing framerate by 1
         // every frame. If so, we're still running at 60.

         sceDisplayWaitSetFrameBufMulti( 1 );

         unsigned short const newVblCount = sceDisplayGetVcount();
         unsigned short const wantNewVblCountWrapped = sPriorVblCount + 1;

         gpRenderBackend->s_dispDisplayHardwareAt60[ disp->buffer_index ] = ( wantNewVblCountWrapped == newVblCount );

         sPriorVblCount = newVblCount;
      }
#endif

      sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_UPDATETIMING_NEXTVSYNC);
      sceDisplayWaitSetFrameBuf();
   }
}

int CRenderBackend::GetVisibility(int frame, int slot)
{
   int sum = 0;

   frame = (s_dispBack - frame) % DISPLAY_BUFFER_COUNT;

   for (int i = 0; i < SCE_GXM_GPU_CORE_COUNT; ++i)
      sum += s_visibility[frame][slot + i * VISIBILITY_SLOTS];
   return sum;
}

void CRenderBackend::EnableVisibility(int slot)
{
   sceGxmSetFrontVisibilityTestEnable( Context(), SCE_GXM_VISIBILITY_TEST_ENABLED );
   sceGxmSetFrontVisibilityTestIndex( Context(), slot );
   sceGxmSetFrontVisibilityTestOp( Context(), SCE_GXM_VISIBILITY_TEST_OP_INCREMENT );
}

void CRenderBackend::DisableVisibility()
{
   sceGxmSetFrontVisibilityTestEnable( Context(), SCE_GXM_VISIBILITY_TEST_DISABLED );
}

int CRenderBackend::FindRenderTarget(uint32 width, uint32 height, int numScenes, int MSAATrick, int xTiles, int yTiles)
{
   SInternalRenderTarget *p = s_internalRenderTargets;

   int flags = 0;
   bool tiled = false;
   int msaa = MSAATrick;

   if (xTiles && yTiles)
   {
      flags |= SCE_GXM_RENDER_TARGET_MACROTILE_SYNC | (xTiles << SCE_GXM_RENDER_TARGET_MACROTILE_COUNT_X_SHIFT) | (yTiles << SCE_GXM_RENDER_TARGET_MACROTILE_COUNT_Y_SHIFT);
      tiled = true;
   }

   if (msaa)
      flags |= SCE_GXM_RENDER_TARGET_CUSTOM_MULTISAMPLE_LOCATIONS;

   for (int i = 0; i < INTERNAL_RENDERTARGET_COUNT; i++, p++)
   {
      if (p->rt && (p->width == width) && (p->height == height) && (p->tiled == tiled) && (p->flags == flags) && (p->msaa == msaa))
         return i;
      else if (!p->rt)
      {
         if (MakeRenderTarget(p, width, height, numScenes, msaa, xTiles, yTiles))
            return i;
         return -1;
      }
   }
   return -1;
}

bool CRenderBackend::MakeRenderTarget(SInternalRenderTarget *renderTarget, uint32 const width, uint32 const height, int const numScenes,
                                      int const MSAATrick, int const xTiles, int const yTiles)
{
   SceGxmRenderTargetParams	rparam;
   SceUInt32	driverSize;

   if (!numScenes)
      return false;

   memset(&rparam, 0, sizeof(SceGxmRenderTargetParams));

   if (xTiles && yTiles)
   {
      rparam.flags                = SCE_GXM_RENDER_TARGET_MACROTILE_SYNC 
         | (xTiles << SCE_GXM_RENDER_TARGET_MACROTILE_COUNT_X_SHIFT)
         | (yTiles << SCE_GXM_RENDER_TARGET_MACROTILE_COUNT_Y_SHIFT);
      renderTarget->tiled = true;
   }
   else
   {
      rparam.flags                = 0;
      renderTarget->tiled = false;
   }

   rparam.width                = width;
   rparam.height               = height;
   rparam.scenesPerFrame       = numScenes ? numScenes : 1;
   renderTarget->scenes        = numScenes;

   renderTarget->msaa = MSAATrick;
   if (MSAATrick)
   {
      rparam.flags |= SCE_GXM_RENDER_TARGET_CUSTOM_MULTISAMPLE_LOCATIONS;
      rparam.multisampleMode      = SCE_GXM_MULTISAMPLE_4X;
      rparam.multisampleLocations = 0xccc44c44;
   }
   else
   {
      rparam.multisampleMode      = SCE_GXM_MULTISAMPLE_NONE;
      rparam.multisampleLocations = 0;
   }
   rparam.driverMemBlock       = SCE_UID_INVALID_UID;

   renderTarget->flags = rparam.flags;
   
   sceGxmGetRenderTargetMemSize(&rparam, &driverSize);
 
   renderTarget->uid = rparam.driverMemBlock = sceKernelAllocMemBlock(
      "BPE" "_RT",
      SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
      driverSize,
      NULL);
   renderTarget->width = width;
   renderTarget->height = height;
   renderTarget->rt = NULL;
   renderTarget->size = driverSize;
   BPE_CHECK_SCE( sceGxmCreateRenderTarget(&rparam, &renderTarget->rt) );
   return true;
}

CRenderBackend::CRenderBackend(IResourcePool & resourcePool, SRenderInitialization const & initFlags)
: CBaseRenderBackend( resourcePool, initFlags )
, mCameraMatrix( CMatrix4::Identity() )
, mViewMatrix( CMatrix4::Identity() )
, mProjectionTimesViewMatrix( CMatrix4::Identity() )
, mpNullAttribute( NULL )
, mpLastIndexBuffer( NULL )
, mLastIndexBufferCount( 0 )
, mRasterMaskEnabled( false )
, mRasterMaskValid( false )
, mVBLCount( 0 )
, mTargetFPS( kFPS_30 )
, clearVertexProgram( 0 )
, clearFragmentProgram( 0 )
, scissorFragmentProgram( 0 )
, maskFragmentProgram( 0 )
, upscaleVertexProgram( 0 )
, upscaleFragmentProgram( 0 )
, mCurrentSceneDrawNum( 0 )
, mThreadLastCPUTime( 0 )
, mLastDisplayQueueFlipTime( 0 )
{

#if defined(ENABLE_RAZOR_HUD) || defined(ENABLE_RAZOR_LIVEMETRICS)
   sceSysmoduleLoadModule( SCE_SYSMODULE_RAZOR_HUD );
#endif

#ifdef ENABLE_RAZOR_CAPTURE
   // Initialize the Razor capture system.
   // This should be done before the call to sceGxmInitialize().
   BPE_CHECK_SCE( sceRazorCpuPushMarkerWithHud( "Renderer", SCE_RAZOR_COLOR_RED, SCE_RAZOR_MARKER_ENABLE_HUD ) );

   BPE_CHECK_SCE( sceSysmoduleLoadModule( SCE_SYSMODULE_RAZOR_CAPTURE ) );

#endif

#if defined(ENABLE_RAZOR_LIVEMETRICS)
   for (int i = 0; i < kLMNumBuffers; i++)
      sLMBuffers[i] = memalign(8, kLMBufferSize);
#endif

   memset( mpVertexUniformHandles, 0, sizeof( mpVertexUniformHandles ) );

   SceGxmInitializeParams		iparam = { 0 };
   SceGxmContextParams			cparam;
   SceGxmShaderPatcherParams	pparam;

   void		*ringbufVdm;
   void		*ringbufVertex;
   void		*ringbufFragment;
   void		*ringbufFragmentUsse;
   SceUInt32	ringbufFragmentUsseOffset;
   SceUInt32	driverSize;
   void		*patcherBuf;
   SceUInt32	patcherVertexUsseOffset;
   SceUInt32	patcherFragmentUsseOffset;
   void		*patcherCombinedUsse;

   iparam.flags = 0;
   iparam.displayQueueCallback         = cb_display;
   iparam.displayQueueCallbackDataSize = sizeof(SDisplay);
   iparam.displayQueueMaxPendingCount = DISPLAY_PENDING_SWAPS;
   iparam.parameterBufferSize			= SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;

   sGraphicsPreAllocCDRAMSize += iparam.parameterBufferSize;
   BPE_CHECK_SCE( sceGxmInitialize( &iparam ) );

   // create the context
   s_contextHost = malloc(SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE);

   int const skVDMRingBufferSize = SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE * 10;
   int const skVertexRingBufferSize = SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE * 10;
   int const skFragmentRingBufferSize = 2 * SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE;

   ringbufVdm = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
      skVDMRingBufferSize, 4,
      SCE_GXM_MEMORY_ATTRIB_READ, &s_vdmRingBufUid);

   ringbufVertex = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
      skVertexRingBufferSize, 4,
      SCE_GXM_MEMORY_ATTRIB_READ, &s_vertexRingBufUid);

   ringbufFragment = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
      skFragmentRingBufferSize, 4,
      SCE_GXM_MEMORY_ATTRIB_READ, &s_fragmentRingBufUid);

   ringbufFragmentUsse = fragment_usse_alloc(
      SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE,
      &s_fragmentUsseRingBufUid, &ringbufFragmentUsseOffset);

   memset(&cparam, 0, sizeof(SceGxmContextParams));
   cparam.hostMem                       = s_contextHost;
   cparam.hostMemSize                   = SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE;
   cparam.vdmRingBufferMem              = ringbufVdm;
   cparam.vdmRingBufferMemSize          = skVDMRingBufferSize;
   cparam.vertexRingBufferMem           = ringbufVertex;
   cparam.vertexRingBufferMemSize       = skVertexRingBufferSize;
   cparam.fragmentRingBufferMem         = ringbufFragment;
   cparam.fragmentRingBufferMemSize     = skFragmentRingBufferSize;
   cparam.fragmentUsseRingBufferMem     = ringbufFragmentUsse;
   cparam.fragmentUsseRingBufferMemSize = SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE;
   cparam.fragmentUsseRingBufferOffset  = ringbufFragmentUsseOffset;

   s_context = NULL;
   sceGxmCreateContext(&cparam, &s_context);
   NVTAState::Init( s_context );

   // create the render target
   MakeRenderTarget(s_internalRenderTargets, skDisplayWidth, skDisplayHeight, 3, 0, 0, 0);
   s_renderTarget = s_internalRenderTargets[0].rt;
   
   SCreateTextureParams dparam;

#ifdef ENABLE_DETAILED_TIMING
   volatile SceUInt32 *note = sceGxmGetNotificationRegion();
#endif

   // Make sure that we have enough notification objects for live metrics
   BPE_CTASSERT( NVTAState::kNO_DisplayBufferCount == DISPLAY_BUFFER_COUNT );

   dparam.mNumScenes = 3;
   for (int i = 0; i<DISPLAY_BUFFER_COUNT; i++) {
      s_dispBuf[i] = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA,
         DISPLAY_BUFFER_SIZE,
         SCE_GXM_COLOR_SURFACE_ALIGNMENT,
         SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
         &s_dispUid[i]);

      sceGxmColorSurfaceInit(&s_dispSurface[i],
         SCE_GXM_COLOR_FORMAT_A8B8G8R8,
         SCE_GXM_COLOR_SURFACE_LINEAR,
         SCE_GXM_COLOR_SURFACE_SCALE_NONE,
         SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
         skDisplayWidth,
         skDisplayHeight,
         DISPLAY_STRIDE,
         s_dispBuf[i]);

      sceGxmSyncObjectCreate(&s_dispSync[i]);
      dparam.mBaseAddress = (unsigned int)s_dispBuf[i];
      dparam.mStride = DISPLAY_STRIDE * 4;
      s_dispTex[i] = CBaseTexture::Create(skDisplayWidth, skDisplayHeight, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_Video, &dparam);
#ifdef ENABLE_DETAILED_TIMING
      s_dispNote[i].address = &note[i];
      s_dispFrame[i] = 0;
      s_dispElapsed[i] = 0.0f;
#endif
   }

   s_dispFront = 0;
   s_dispBack = 0;
   // create depth buffer
   s_depthBuf = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA,
      DISPLAY_DEPTH_SIZE,
      SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT,
      SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
      &s_depthUid);

   sceGxmDepthStencilSurfaceInit(&s_depthSurface,
      SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24,
      SCE_GXM_DEPTH_STENCIL_SURFACE_LINEAR,
      DISPLAY_ALIGN_WIDTH,
      s_depthBuf,
      NULL);
   dparam.mBaseAddress = (unsigned int)s_depthBuf;
   s_depthTex = CBaseTexture::Create(skDisplayWidth, skDisplayHeight, 1, CBaseTexture::kFormat_D24X8, CBaseTexture::kUsage_DepthBuffer, CBaseTexture::kAA_None, kRM_Video, &dparam);
   sceGxmDepthStencilSurfaceSetForceLoadMode(&s_depthSurface, SCE_GXM_DEPTH_STENCIL_FORCE_LOAD_ENABLED);
   sceGxmDepthStencilSurfaceSetForceStoreMode(&s_depthSurface, SCE_GXM_DEPTH_STENCIL_FORCE_STORE_ENABLED);
   // JM - THESE ARE NOW DIFFERENT BETWEEN MGS2 and 3!!
   //   sceGxmDepthStencilSurfaceSetForceLoadMode(&s_depthSurface, SCE_GXM_DEPTH_STENCIL_FORCE_LOAD_DISABLED);
   //   sceGxmDepthStencilSurfaceSetForceStoreMode(&s_depthSurface, SCE_GXM_DEPTH_STENCIL_FORCE_STORE_DISABLED);

   // setup draw texture
   MakeRenderTarget(&s_internalRenderTargets[1], skDrawWidth, skDrawHeight, 3, 0, 0, 0);
   
   SCreateTextureParams drawParams;
   drawParams.mNumScenes = 3;
   
   s_drawColorBuf = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA,
      skDrawWidth * skDrawHeight * 4,
      SCE_GXM_COLOR_SURFACE_ALIGNMENT,
      SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
      &s_drawColorUid);
   s_drawDepthBuf = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA,
      skDrawWidth * skDrawHeight * 4,
      SCE_GXM_COLOR_SURFACE_ALIGNMENT,
      SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
      &s_drawColorUid);

   drawParams.mBaseAddress = (unsigned int)s_drawColorBuf;
   s_drawColor = CBaseTexture::Create(skDrawWidth, skDrawHeight, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_System, &drawParams);
   drawParams.mBaseAddress = (unsigned int)s_drawDepthBuf;
   //ja -- can't optimize this yet, shadows will trigger a reload, and need a proper depth buffer
   //drawParams.mLoadStore = CBaseTexture::kLS_Store ;
   s_drawDepth = CBaseTexture::Create(skDrawWidth, skDrawHeight, 1, CBaseTexture::kFormat_D24X8, CBaseTexture::kUsage_DepthBuffer, CBaseTexture::kAA_None, kRM_System, &drawParams);

   s_visibilityBuf = (uint32*)g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
                                      VISIBILITY_SLOTS * DISPLAY_BUFFER_COUNT * SCE_GXM_GPU_CORE_COUNT * sizeof(uint32),
                                      SCE_GXM_VISIBILITY_ALIGNMENT,
                                      SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
                                      &s_visibilityUid);
   for (int i = 0; i < DISPLAY_BUFFER_COUNT; ++i)
   {
      s_visibility[i] = s_visibilityBuf + i * SCE_GXM_GPU_CORE_COUNT * VISIBILITY_SLOTS;
   }

   // setup shadow target
   FindRenderTarget(256, 256, 2, 0, 0, 0);
   // create a shader patcher
   patcherBuf = g_alloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE,
      PATCHER_BUFFER_SIZE, 4,
      SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE, &s_patcherBufUid);

   patcherCombinedUsse = combined_usse_alloc(
      PATCHER_COMBINED_USSE_SIZE, &s_patcherCombinedUsseUid,
      &patcherVertexUsseOffset, &patcherFragmentUsseOffset);

   memset(&pparam, 0, sizeof(SceGxmShaderPatcherParams));
   pparam.userData                  = NULL;
   pparam.hostAllocCallback         = &patcher_host_alloc;
   pparam.hostFreeCallback          = &patcher_host_free;
   pparam.bufferAllocCallback       = NULL;
   pparam.bufferFreeCallback        = NULL;
   pparam.bufferMem                 = patcherBuf;
   pparam.bufferMemSize             = PATCHER_BUFFER_SIZE;
   pparam.vertexUsseAllocCallback   = NULL;
   pparam.vertexUsseFreeCallback    = NULL;
   pparam.vertexUsseMem             = patcherCombinedUsse;
   pparam.vertexUsseMemSize         = PATCHER_COMBINED_USSE_SIZE;
   pparam.vertexUsseOffset          = patcherVertexUsseOffset;
   pparam.fragmentUsseAllocCallback = NULL;
   pparam.fragmentUsseFreeCallback  = NULL;
   pparam.fragmentUsseMem           = patcherCombinedUsse;
   pparam.fragmentUsseMemSize       = PATCHER_COMBINED_USSE_SIZE;
   pparam.fragmentUsseOffset        = patcherFragmentUsseOffset;

   NVTAState::s_shaderPatcher = NULL;
   sceGxmShaderPatcherCreate(&pparam, &NVTAState::s_shaderPatcher);


   BPE_CHECK_SCE( mRenderAllocUIDs[ kRM_System ] = sceKernelAllocMemBlock( "graphics main", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, kMainMemoryPoolSize, NULL ) );

   // Try to allocate as much video memory as possible.
   // videoMemoryAllocationSize will change below to be the final size
   // we start, though, by taking 112MiB (memory size) and subtracting what we've already allocated.
   int videoMemoryAllocationSize = ( 112 * 1024 * 1024 - sGraphicsPreAllocCDRAMSize ) & ( ~0x3ffffU );
   {
      SceUID memoryAlloc = -1;

      for ( ;; )
      {
	     // Try to allocate all of videoMemoryAllocationSize
         memoryAlloc = sceKernelAllocMemBlock( "graphics video", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, videoMemoryAllocationSize, NULL );

         if ( memoryAlloc < SCE_OK )
         {
		    // If we failed the allocation, lower by 256kb (page size) and try again
            videoMemoryAllocationSize -= 0x40000U;
         }
         else
         {
            break;
         }
      }

      mRenderAllocUIDs[ kRM_Video ] = memoryAlloc;
   }

   void *pMemBlockBases[ 2 ] = { NULL, NULL };
   BPE_CHECK_SCE( sceKernelGetMemBlockBase( mRenderAllocUIDs[ kRM_System ], pMemBlockBases + kRM_System ) );
   BPE_CHECK_SCE( sceKernelGetMemBlockBase( mRenderAllocUIDs[ kRM_Video ], pMemBlockBases + kRM_Video ) );

   printf( "*************** Graphics Init Info *****************\n" );
   printf( "CDRAM Size: %d\n", sGraphicsPreAllocCDRAMSize );
   printf( "CDRAM Size (Heap): %d\n", videoMemoryAllocationSize );
   printf( "LPDDR Size: %d\n", sGraphicsPreAllocLPDDRSize );
   printf( "LPDDR Size (Heap): %d\n", kMainMemoryPoolSize );
   printf( "****************************************************\n" );

   mpRenderAllocators[ kRM_System ] = new CRenderHWAllocator( pMemBlockBases[ kRM_System ], kMainMemoryPoolSize, kMainMemoryHandleCount, kRM_System );
   mpRenderAllocators[ kRM_Video ] = new CRenderHWAllocator( pMemBlockBases[ kRM_Video ], videoMemoryAllocationSize, kVideoMemoryHandleCount, kRM_Video );

   BPE_CHECK_SCE( sceGxmMapMemory( pMemBlockBases[ kRM_System ], kMainMemoryPoolSize, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE ) );
   BPE_CHECK_SCE( sceGxmMapMemory( pMemBlockBases[ kRM_Video ], videoMemoryAllocationSize, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE ) );

   InitDefaultObjects();

   mpDynamicVertexBufferPool_RT.reset(new CDynamicVertexBufferPool_RT(initFlags.mVertexBufferPoolSize, initFlags.mVertexBufferPoolChunkCount));
   mpDynamicIndexBufferPool_RT.reset(new CDynamicIndexBufferPool_RT(initFlags.mIndexBufferPoolSize, initFlags.mIndexBufferPoolChunkCount));

   mpDynamicVertexBufferPool_UT.reset(new CDynamicVertexBufferPool_UT(initFlags.mVertexBufferPoolSize, initFlags.mVertexBufferPoolChunkCount));
   mpDynamicIndexBufferPool_UT.reset(new CDynamicIndexBufferPool_UT(initFlags.mIndexBufferPoolSize, initFlags.mIndexBufferPoolChunkCount));

//   mUT_PresentationInterval = 1;
//   mRT_PresentationInterval = 1;

   mpNullAttribute = AllocFixed( 128, 128, kRM_System, kRM_Video );
   memset( mpNullAttribute->mpAddress, 0, 128 );
   NVTAState::SetNullAttribute( mpNullAttribute->mpAddress );

   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_clear_v_gxp_start, &clearVertexProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_clear_f_gxp_start, &clearFragmentProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_scissor_f_gxp_start, &scissorFragmentProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_mask_f_gxp_start, &maskFragmentProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_maskhalf_f_gxp_start, &maskHalfFragmentProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_upscale_v_gxp_start, &upscaleVertexProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_upscale_f_gxp_start, &upscaleFragmentProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_downclear_v_gxp_start, &downclearVertexProgramId) );
   BPE_CHECK_SCE( sceGxmShaderPatcherRegisterProgram(NVTAState::s_shaderPatcher, &_binary_downclear_f_gxp_start, &downclearFragmentProgramId) );

   {
      // get attributes by name to create vertex format bindings
      const SceGxmProgram *clearProgram = sceGxmShaderPatcherGetProgramFromId(clearVertexProgramId);
      BPE_ASSERT(clearProgram != NULL, "Bad program" );
      const SceGxmProgramParameter *paramClearPositionAttribute = sceGxmProgramFindParameterByName(clearProgram, "aPosition");
      BPE_ASSERT(paramClearPositionAttribute && (sceGxmProgramParameterGetCategory(paramClearPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE), "Bad category?!" );

      // create clear vertex format
      SceGxmVertexAttribute clearVertexAttributes[1];
      SceGxmVertexStream clearVertexStreams[1];
      clearVertexAttributes[0].streamIndex = 0;
      clearVertexAttributes[0].offset = 0;
      clearVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
      clearVertexAttributes[0].componentCount = 2;
      clearVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramClearPositionAttribute);
      clearVertexStreams[0].stride = sizeof(float) * 2;
      clearVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

      // create sclear programs
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateVertexProgram(
         NVTAState::s_shaderPatcher,
         clearVertexProgramId,
         clearVertexAttributes,
         1,
         clearVertexStreams,
         1,
         &clearVertexProgram) );
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         clearFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(clearVertexProgramId),
         &clearFragmentProgram) );
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         scissorFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(clearVertexProgramId),
         &scissorFragmentProgram) );
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         maskFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(clearVertexProgramId),
         &maskFragmentProgram) );

      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         clearFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(clearVertexProgramId),
         &clearFragmentProgramHalf) );
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         scissorFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(clearVertexProgramId),
         &scissorFragmentProgramHalf) );
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         maskHalfFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(clearVertexProgramId),
         &maskFragmentProgramHalf) );

   }

   {
      // create upscale attributes and programs
      const SceGxmProgram *upscaleProgram = sceGxmShaderPatcherGetProgramFromId(upscaleVertexProgramId);

      BPE_ASSERT(upscaleProgram != NULL, "Bad program" );
      const SceGxmProgramParameter *paramUpscalePositionAttribute = sceGxmProgramFindParameterByName(upscaleProgram, "aPosition");
      BPE_ASSERT(paramUpscalePositionAttribute && (sceGxmProgramParameterGetCategory(paramUpscalePositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE), "Bad category?!" );
      const SceGxmProgramParameter *paramUpscaleUVAttribute = sceGxmProgramFindParameterByName(upscaleProgram, "uv");
      BPE_ASSERT(paramUpscalePositionAttribute && (sceGxmProgramParameterGetCategory(paramUpscalePositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE), "Bad category?!" );

      SceGxmVertexAttribute upscaleVertexAttributes[2];
      SceGxmVertexStream upscaleVertexStreams[1];

      upscaleVertexAttributes[0].streamIndex = 0;
      upscaleVertexAttributes[0].offset = 0;
      upscaleVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
      upscaleVertexAttributes[0].componentCount = 3;
      upscaleVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramUpscalePositionAttribute);
      upscaleVertexAttributes[1].streamIndex = 0;
      upscaleVertexAttributes[1].offset = 3 * sizeof(float);
      upscaleVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
      upscaleVertexAttributes[1].componentCount = 2;
      upscaleVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramUpscaleUVAttribute);
      
      upscaleVertexStreams[0].stride = sizeof(float) * 5;
      upscaleVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;
      
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateVertexProgram(
         NVTAState::s_shaderPatcher,
         upscaleVertexProgramId,
         upscaleVertexAttributes,
         2,
         upscaleVertexStreams,
         1,
         &upscaleVertexProgram) );
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         upscaleFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(upscaleVertexProgramId),
         &upscaleFragmentProgram) );
   }

   {
      // create downscale clear attributes and programs
      const SceGxmProgram *downclearProgram = sceGxmShaderPatcherGetProgramFromId(downclearVertexProgramId);

      BPE_ASSERT(downclearProgram != NULL, "Bad program" );
      const SceGxmProgramParameter *paramDownclearPositionAttribute = sceGxmProgramFindParameterByName(downclearProgram, "pos");
      BPE_ASSERT(paramDownclearPositionAttribute && (sceGxmProgramParameterGetCategory(paramDownclearPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE), "Bad category?!" );
      const SceGxmProgramParameter *paramDownclearUVAttribute = sceGxmProgramFindParameterByName(downclearProgram, "uv");
      BPE_ASSERT(paramDownclearPositionAttribute && (sceGxmProgramParameterGetCategory(paramDownclearPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE), "Bad category?!" );
      const SceGxmProgramParameter *paramDownclearColorAttribute = sceGxmProgramFindParameterByName(downclearProgram, "color");
      BPE_ASSERT(paramDownclearPositionAttribute && (sceGxmProgramParameterGetCategory(paramDownclearPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE), "Bad category?!" );

      SceGxmVertexAttribute downclearVertexAttributes[3];
      SceGxmVertexStream downclearVertexStreams[1];

      downclearVertexAttributes[0].streamIndex = 0;
      downclearVertexAttributes[0].offset = 0;
      downclearVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
      downclearVertexAttributes[0].componentCount = 3;
      downclearVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramDownclearPositionAttribute);
      downclearVertexAttributes[1].streamIndex = 0;
      downclearVertexAttributes[1].offset = 2 * sizeof(float);
      downclearVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
      downclearVertexAttributes[1].componentCount = 2;
      downclearVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramDownclearUVAttribute);
      downclearVertexAttributes[2].streamIndex = 0;
      downclearVertexAttributes[2].offset = 4 * sizeof(float);
      downclearVertexAttributes[2].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N;
      downclearVertexAttributes[2].componentCount = 4;
      downclearVertexAttributes[2].regIndex = sceGxmProgramParameterGetResourceIndex(paramDownclearColorAttribute);
      
      downclearVertexStreams[0].stride = sizeof(float) * 5;
      downclearVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;
      
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateVertexProgram(
         NVTAState::s_shaderPatcher,
         downclearVertexProgramId,
         downclearVertexAttributes,
         3,
         downclearVertexStreams,
         1,
         &downclearVertexProgram) );
      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         downclearFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(downclearVertexProgramId),
         &downclearFragmentProgram) );

      BPE_CHECK_SCE( sceGxmShaderPatcherCreateFragmentProgram(
         NVTAState::s_shaderPatcher,
         downclearFragmentProgramId,
         SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4,
         SCE_GXM_MULTISAMPLE_NONE,
         NULL,
         sceGxmShaderPatcherGetProgramFromId(downclearVertexProgramId),
         &downclearFragmentProgramHalf) );
   }

   NVTAState::FlushInitialStates();

   // The "no primitive" index buffer
   {
      mpNoPrimitiveIndexBuffer.reset( new CStaticIndexBuffer( kIT_Uint16, skNoPrimIndexCount ) );

      uint16 *pPrims = reinterpret_cast<uint16 *>( mpNoPrimitiveIndexBuffer->Lock() );

      for ( size_t i = 0; i < skNoPrimIndexCount; ++i )
      {
         pPrims[i] = static_cast<uint16>( i );
      }
   }

   // The raster mask vertex buffer
   {
      mpRasterMaskVertexBuffer.reset( new CStaticVertexBuffer( 512 * 2 * sizeof(float) ) );

      CVector2 *pVerts = reinterpret_cast<CVector2 *>( mpRasterMaskVertexBuffer->Lock() );

      for ( size_t i = 0; i < 512; i++)
      {
         pVerts[i*2] = CVector2(0.0f, i * 2.0f + 1.0f);
         pVerts[i*2+1] = CVector2(2048.0f, i * 2.0f + 1.0f);
      }
   }

   mVBLCount = sceDisplayGetVcount();
}

CRenderBackend::~CRenderBackend()
{

   FreeImmediate( mpNullAttribute );
   NVTAState::SetNullAttribute( NULL );

   for ( int i = 0; i < BPE_ARRAY_SIZE( mpRenderAllocators ); ++i )
   {
      delete mpRenderAllocators[i];
      sceKernelFreeMemBlock( mRenderAllocUIDs[ i ] );
   }
}

//----------------------------------------------------------------------------

static void OutputProgramParameters( SceGxmProgram const *program )
{
   for ( int paramIndex = 0; paramIndex < sceGxmProgramGetParameterCount( program ); paramIndex++ )
   {
      SceGxmProgramParameter const *param = sceGxmProgramGetParameter( program, paramIndex );

      printf( "Param %d: %s - %d\n",
         paramIndex, sceGxmProgramParameterGetName( param ), sceGxmProgramParameterGetResourceIndex( param ) );
   }
}


void CRenderBackend::FlushDrawState()
{
   NVTAState::FlushStatesAndClear();
   NVTATextureState::FlushDirtyTextures();
}

void CRenderBackend::SetVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData, uint64 const vertexDataHash_1, uint64 const vertexDataHash_2)
{
   NVTAState::SCachedVertexData cvd(vertexDataBinding, vertexData);

   NVTAState::SetLowLevelStreams( cvd.streamsOut, cvd.pStreamTail - cvd.streamsOut );
   NVTAState::SetAttributes( cvd.vtaAttributes.Finalize() );
}

void CRenderBackend::ForceVertexDataRebind()
{
}

void  CRenderBackend::RenderPrimitives(CMeshChunk::EPrimitive type, uint32 const indexBufferOffset, uint32 const indexCount)
{
   int indexBufferOffsetBytes = 0;
   EIndexType indexType = NVTAState::GetLastIndexBufferType();

   if( BPE_BRANCH_HINT_FALSE(indexBufferOffset > 0) )
   {
      indexBufferOffsetBytes = indexBufferOffset * GetSizeForIndexType( indexType );
   }

   WrapGxmDraw( type, index_format_to_gxm( indexType ), mpLastIndexBuffer + indexBufferOffsetBytes, indexCount );
}

void CRenderBackend::RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount)
{
   if ( vertexBufferOffset == 0 && vertexCount < skNoPrimIndexCount )
   {
      SetIndexData_OneToOne( vertexCount );
      RenderPrimitives( type, 0, vertexCount );
   }
   else
   {
      CDynamicIndexBuffer quadIndexBuffer( kRRTU_Render );

      int *indexes = (int *) quadIndexBuffer.Lock( kIT_Uint32, vertexCount );

      for ( int i = 0; i < vertexCount; ++i )
      {
         indexes[i] = i + vertexBufferOffset;
      }

      quadIndexBuffer.Unlock();

      SetIndexData( &quadIndexBuffer );

      RenderPrimitives( type, 0, vertexCount );
   }
}

void CRenderBackend::RenderPrimitivesInstanced(CMeshChunk::EPrimitive type, uint32 const indexBufferOffset, uint32 const indexCount, uint32 const indexWrap)
{
   int indexBufferOffsetBytes = 0;

   if( BPE_BRANCH_HINT_FALSE(indexBufferOffset > 0) )
   {
      indexBufferOffsetBytes = indexBufferOffset * GetSizeForIndexType( NVTAState::GetLastIndexBufferType() );
   }

   WrapGxmDrawInstanced( type, index_format_to_gxm( NVTAState::GetLastIndexBufferType() ), mpLastIndexBuffer + indexBufferOffsetBytes, indexCount, indexWrap );
}

void CRenderBackend::BeginRenderPrimitivesCustom()
{
}

void CRenderBackend::EndRenderPrimitivesCustom()
{
}

void CRenderBackend::RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
                                    CShaderVertexDataBinding binding, 
                                    void const * pData, 
                                    uint32 const numVertices)
{
   BPE_VERIFY( numVertices < skNoPrimIndexCount, false, "Too many prims rendered in RenderPrimitivesUserVertexData" );
      
   CRenderBackend *pThis = static_cast<CRenderBackend *>(this);

   SetIndexData_OneToOne( numVertices );

   NVTAState::CAttributeList attrs;

   int vertexSize = 0;
   if ( binding.HasStream( kVDU_Position ) )
   {
      attrs.Add( kVDU_Position, (SceGxmVertexAttribute) { 0, 0, SCE_GXM_ATTRIBUTE_FORMAT_F32, 3, 0 } );
      vertexSize += sizeof(float)*3;
   }

   if ( binding.HasStream( kVDU_Normal ) )
   {
      attrs.Add( kVDU_Normal, (SceGxmVertexAttribute) { 0, vertexSize, SCE_GXM_ATTRIBUTE_FORMAT_F32, 3, 0 } );
      vertexSize += sizeof(float)*3;
   }

   if ( binding.HasStream( kVDU_Color0 ) )
   {
      attrs.Add( kVDU_Color0, (SceGxmVertexAttribute) { 0, vertexSize, SCE_GXM_ATTRIBUTE_FORMAT_U8, 4, 0 } );
      vertexSize += sizeof(uint8)*4;
   }

   if ( binding.HasStream( kVDU_TexCoord0 ) )
   {
      attrs.Add( kVDU_TexCoord0, (SceGxmVertexAttribute) { 0, vertexSize, SCE_GXM_ATTRIBUTE_FORMAT_F32, 2, 0 } );
      vertexSize += sizeof(float)*2;
   }

   CDynamicVertexBuffer vb( kRRTU_Render );

   size_t totalVertexDataSize = vertexSize * numVertices;
   memcpy( vb.Lock( totalVertexDataSize ), pData, totalVertexDataSize );
   vb.Unlock();

   NVTAState::SetAttributes( attrs.Finalize() );
   NVTAState::SetupOneLowLevelStream( NVTAState::SLowLevelStream( vb.mpMemory_RT->mpAddress, vertexSize ) );
   pThis->WrapGxmDraw( type, index_format_to_gxm( NVTAState::GetLastIndexBufferType() ), mpLastIndexBuffer, mLastIndexBufferCount );
}

void CRenderBackend::DisableScissor()
{
   sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_ALWAYS, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, 0x80, 0x80);
}

void CRenderBackend::EnableScissor()
{
   FlushStencilTest();
}

int gVTADebugRescaleWidth = 720;
int gVTADebugRescaleHeight = 448;

void CRenderBackend::UpscaleDrawBuffer()
{
   // we're switching from the main channel (in MGS2) or some nebulous tbd viewport (in MGS3) to
   // the UI, fill the buffer with an upscale of the lower res rendering results

   sceGxmPushUserMarker( Context(), "Upscale" );

   struct SVertex
   {
      float x, y, z, u, v;
      SVertex(float _x, float _y, float _z, float _u, float _v) : x(_x), y(_y), z(_z), u(_u), v(_v) {}
   };

   uint16 *indexes = GetIndexBufferPool_RT()->AllocChunk( 6 ).Lock();
   SVertex *verts = reinterpret_cast<SVertex *>( GetVertexBufferPool_RT()->AllocChunk( 4 * sizeof( SVertex ) )->Lock() );

   indexes[0] = 0;
   indexes[1] = 1;
   indexes[2] = 2;
   indexes[3] = 2;
   indexes[4] = 1;
   indexes[5] = 3;

   verts[0] = SVertex( -1.0f, 1.0f, 1.0f, 0.0f, 0.0f );
   verts[1] = SVertex(  1.0f, 1.0f, 1.0f, 1.0f, 0.0f );
   verts[2] = SVertex( -1.0f,-1.0f, 1.0f, 0.0f, 1.0f );
   verts[3] = SVertex( 1.0f, -1.0f, 1.0f, 1.0f, 1.0f );

   CTexture *ctex = static_cast<CTexture *>(s_drawColor);

   extern int gVTADebugRescaleWidth;
   extern int gVTADebugRescaleHeight;

   ctex->SetWidth( gVTADebugRescaleWidth );
   ctex->SetHeight( gVTADebugRescaleHeight );
   BPE_CHECK_SCE( sceGxmSetFragmentTexture( Context(), 0, base_to_tex_ptr(s_drawColor)->GetGxmTexture() ) );
   sceGxmSetVertexProgram( Context(), upscaleVertexProgram );
   sceGxmSetFragmentProgram( Context(), upscaleFragmentProgram );

   BPE_CHECK_SCE( sceGxmSetVertexStream( Context(), 0, verts ) );
   BPE_CHECK_SCE( sceGxmDraw( Context(), SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, indexes, 6 ) );

   ++mCurrentSceneDrawNum;

   // Completely flush state after clear
   NVTAState::InvalidateHardwareState();
   sceGxmPopUserMarker( Context() );
   
}

int gVTAEnableScissorOptimization = 0;

void CRenderBackend::FlushStencilTest()
{
   if (mRasterMaskEnabled)
   {
      // Don't toggle scanline every frame, flicker is ugly in the cinematics
      if (0 /*gpRenderBackend->GetFrameCount() & 1*/)
      {
         sceGxmSetFrontStencilRef( Context(), 0xC0 );
      }
      else
      {
         sceGxmSetFrontStencilRef( Context(), 0x80 );
      }

      sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_EQUAL, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, 0xC0, 0x40);
   }
   else
   {
      sceGxmSetFrontStencilRef( Context(), 0x80 );
      sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_EQUAL, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, 0x80, 0x40);
   }
}

void CRenderBackend::SetDepthBias(int slope, int bias)
{
   sceGxmSetFrontDepthBias( Context(), slope, bias );
}

void CRenderBackend::SetRasterMask(bool enable)
{
   if (!mRasterMaskValid)
   {
      ResetRasterMask();
      mRasterMaskValid = true;
   }

   mRasterMaskEnabled = enable;
   FlushStencilTest();
}

void CRenderBackend::ResetRasterMask()
{
   if (mMSAATrickEnabled)
      return;
   
   uint16 *indexes = GetIndexBufferPool_RT()->AllocChunk( 6 ).Lock();
   CVector2 *verts = reinterpret_cast<CVector2 *>( GetVertexBufferPool_RT()->AllocChunk( 4 * sizeof( CVector2 ) )->Lock() );

   sceGxmPushUserMarker( Context(), "RasterMask" );

   indexes[0] = 0;
   indexes[1] = 1;
   indexes[2] = 2;
   indexes[3] = 2;
   indexes[4] = 1;
   indexes[5] = 3;

   verts[0] = CVector2( 0.f, 0.f );
   verts[1] = CVector2( (float)skDisplayWidth, 0.f );
   verts[2] = CVector2( 0.f, (float)skDisplayHeight );
   verts[3] = CVector2( (float)skDisplayWidth, (float)skDisplayHeight );

   sceGxmSetCullMode(Context(), SCE_GXM_CULL_NONE);
   sceGxmSetFrontDepthWriteEnable( Context(), SCE_GXM_DEPTH_WRITE_DISABLED );
   sceGxmSetFrontDepthFunc( Context(), SCE_GXM_DEPTH_FUNC_ALWAYS );
   sceGxmSetFrontPolygonMode( Context(), SCE_GXM_POLYGON_MODE_TRIANGLE_FILL );

   sceGxmSetVertexProgram( Context(), clearVertexProgram );
   sceGxmSetFragmentProgram( Context(), scissorFragmentProgram );

   // disable viewport and send the scissor rect setup in screen coordinates
   sceGxmSetViewportEnable( Context(), SCE_GXM_VIEWPORT_DISABLED );
   sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_ALWAYS, SCE_GXM_STENCIL_OP_ZERO, SCE_GXM_STENCIL_OP_ZERO, SCE_GXM_STENCIL_OP_ZERO, 0x80, 0x40);
   sceGxmSetFrontStencilRef( Context(), 0xFF );
   BPE_CHECK_SCE( sceGxmSetVertexStream( Context(), 0, verts ) );
   BPE_CHECK_SCE( sceGxmDraw( Context(), SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, indexes, 6 ) );

   sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_ALWAYS, SCE_GXM_STENCIL_OP_REPLACE, SCE_GXM_STENCIL_OP_REPLACE, SCE_GXM_STENCIL_OP_REPLACE, 0x80, 0x40);

   indexes = reinterpret_cast<uint16 *>(mpNoPrimitiveIndexBuffer->Lock());
   verts = reinterpret_cast<CVector2 *>(mpRasterMaskVertexBuffer->Lock());
   sceGxmSetFrontPolygonMode( Context(), SCE_GXM_POLYGON_MODE_LINE );
   BPE_CHECK_SCE( sceGxmSetVertexStream( Context(), 0, verts ) );
   BPE_CHECK_SCE( sceGxmDraw( Context(), SCE_GXM_PRIMITIVE_LINES, SCE_GXM_INDEX_FORMAT_U16, indexes, skDisplayHeight ) );

   sceGxmSetViewportEnable( Context(), SCE_GXM_VIEWPORT_ENABLED );
   FlushStencilTest();

   ++mCurrentSceneDrawNum;

   // Completely flush state after clear
   NVTAState::InvalidateHardwareState();
   sceGxmPopUserMarker( Context() );   
}

void CRenderBackend::ResetScissor()
{
   // The SGX hardware doesn't really have a scissor rect, so we fake one with the stencil buffer
   // The PS2 didn't have a stencil test, and reused it's alpha buffer, so nothing will use this normally
   // We clear the stencil to 0, then set it to 1 in the area we want to scissor to, then set the stencil func
   // to reject everything else
   // SetScissor only gets called when the viewport actually changes, this gets called in the clear

   // if we don't have a depth buffer, don't do scissoring
   if (!mCurrentRenderTarget.mpDepthBuffer)
   {
      DisableScissor();
      return;
   }

   if (gVTAEnableScissorOptimization && scissorTrivial)
   {
      DisableScissor();
      return;
   }

   uint16 *indexes = GetIndexBufferPool_RT()->AllocChunk( 12 ).Lock();
   CVector2 *verts = reinterpret_cast<CVector2 *>( GetVertexBufferPool_RT()->AllocChunk( 8 * sizeof( CVector2 ) )->Lock() );

   sceGxmPushUserMarker( Context(), "Scissor" );

   indexes[0] = 0;
   indexes[1] = 1;
   indexes[2] = 2;
   indexes[3] = 2;
   indexes[4] = 1;
   indexes[5] = 3;
   indexes[6] = 4;
   indexes[7] = 5;
   indexes[8] = 6;
   indexes[9] = 6;
   indexes[10] = 5;
   indexes[11] = 7;

   verts[0] = CVector2( 0.f, 0.f );
   verts[1] = CVector2( (float)skDisplayWidth, 0.f );
   verts[2] = CVector2( 0.f, (float)skDisplayHeight );
   verts[3] = CVector2( (float)skDisplayWidth, (float)skDisplayHeight );
   verts[4] = CVector2( scissorLeft, scissorTop ); // UL
   verts[5] = CVector2( scissorRight, scissorTop ); // UR
   verts[6] = CVector2( scissorLeft, scissorBottom ); // LL
   verts[7] = CVector2( scissorRight, scissorBottom ); // LR

   uint32_t clipLeft, clipRight, clipTop, clipBottom;

   clipLeft = scissorLeft & ~(SCE_GXM_TILE_SIZEX - 1);
   clipRight = (scissorRight + SCE_GXM_TILE_SIZEX - 1) & ~(SCE_GXM_TILE_SIZEX - 1);
   clipTop = scissorTop & ~(SCE_GXM_TILE_SIZEY - 1);
   clipBottom = (scissorBottom + SCE_GXM_TILE_SIZEY - 1) & ~(SCE_GXM_TILE_SIZEY - 1);

   sceGxmSetRegionClip( Context(), SCE_GXM_REGION_CLIP_OUTSIDE, clipLeft, clipTop, clipRight-1, clipBottom-1 );

   sceGxmSetCullMode(Context(), SCE_GXM_CULL_NONE);
   sceGxmSetFrontDepthWriteEnable( Context(), SCE_GXM_DEPTH_WRITE_DISABLED );
   sceGxmSetFrontDepthFunc( Context(), SCE_GXM_DEPTH_FUNC_ALWAYS );
   sceGxmSetFrontPolygonMode( Context(), SCE_GXM_POLYGON_MODE_TRIANGLE_FILL );

   sceGxmSetVertexProgram( Context(), clearVertexProgram );
   sceGxmSetFragmentProgram( Context(), scissorFragmentProgram );
   // disable viewport and send the scissor rect setup in screen coordinates
   sceGxmSetViewportEnable( Context(), SCE_GXM_VIEWPORT_DISABLED );
   sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_ALWAYS, SCE_GXM_STENCIL_OP_ZERO, SCE_GXM_STENCIL_OP_ZERO, SCE_GXM_STENCIL_OP_ZERO, 0x80, 0x80);
   sceGxmSetFrontStencilRef( Context(), 0xFF );
   BPE_CHECK_SCE( sceGxmSetVertexStream( Context(), 0, verts ) );
   BPE_CHECK_SCE( sceGxmDraw( Context(), SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, indexes, 6 ) );

   sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_ALWAYS, SCE_GXM_STENCIL_OP_REPLACE, SCE_GXM_STENCIL_OP_REPLACE, SCE_GXM_STENCIL_OP_REPLACE, 0x80, 0x80);

   BPE_CHECK_SCE( sceGxmDraw( Context(), SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, indexes + 6, 6 ) );

   sceGxmSetViewportEnable( Context(), SCE_GXM_VIEWPORT_ENABLED );
   FlushStencilTest();

   ++mCurrentSceneDrawNum;

   // Completely flush state after clear
   NVTAState::InvalidateHardwareState();
   sceGxmPopUserMarker( Context() );
}

void CRenderBackend::SetScissor(int const sX, int const sY, int const sWidth, int const sHeight)
{
   int vx,vy,vw,vh;

   if (mCurrentRenderTarget.mRenderTargetViewPort.mViewportEnabled)
   {
      vx = mCurrentRenderTarget.mRenderTargetViewPort.mViewportX;
      vy = mCurrentRenderTarget.mRenderTargetViewPort.mViewportY;
      vw = mCurrentRenderTarget.mRenderTargetViewPort.mViewportWidth;
      vh = mCurrentRenderTarget.mRenderTargetViewPort.mViewportHeight;
   }
   else
   {
      vx = 0;
      vy = 0;
      vw = mRenderTargetWidth;
      vh = mRenderTargetHeight;
   }

   int sLeft, sRight, sTop, sBottom;
   sLeft = sX;
   sRight = sX + sWidth;
   sTop = sY;
   sBottom = sY + sHeight;

   if (sLeft < vx)
      sLeft = vx;
   if (sRight > vx + vw)
      sRight = vx + vw;
   if (sTop < vy)
      sTop = vy;
   if (sBottom > vy + vh)
      sBottom = vy + vh;

   scissorTrivial = false;
   if (!mCurrentRenderTarget.mRenderTargetViewPort.mViewportEnabled && !sLeft && !sTop && (sRight == vx+vw) && (sBottom == vy+vh))
      scissorTrivial = true;

   scissorLeft = sLeft;
   scissorRight = sRight;
   scissorTop = sTop;
   scissorBottom = sBottom;
}

void CRenderBackend::DownsampleClear(CColor const color, CBaseTexture *depth)
{
   sceGxmPushUserMarker( Context(), "Downsample Clear" );

   struct SVertex
   {
      float x, y, u, v;
      uint32 c;
      SVertex(float _x, float _y, float _u, float _v, uint32 _c) : x(_x), y(_y), u(_u), v(_v), c(_c) {}
   };

   uint16 *indexes = GetIndexBufferPool_RT()->AllocChunk( 3 ).Lock();
   SVertex *verts = reinterpret_cast<SVertex *>( GetVertexBufferPool_RT()->AllocChunk( 3 * sizeof( SVertex ) )->Lock() );

   indexes[0] = 0;
   indexes[1] = 1;
   indexes[2] = 2;

   verts[0] = SVertex( -1.f, 1.f, 0.0f, 0.0f, color.GetARGB() );
   verts[1] = SVertex( 3.f, 1.f, 2.0f, 0.0f, color.GetARGB() );
   verts[2] = SVertex( -1.f, -3.f, 0.0f, 2.0f, color.GetARGB() );
   
   sceGxmSetCullMode(Context(), SCE_GXM_CULL_NONE);
   sceGxmSetFrontDepthWriteEnable( Context(), SCE_GXM_DEPTH_WRITE_ENABLED );
   sceGxmSetFrontDepthFunc( Context(), SCE_GXM_DEPTH_FUNC_ALWAYS );
   sceGxmSetFrontPolygonMode( Context(), SCE_GXM_POLYGON_MODE_TRIANGLE_FILL );

   CTexture *ctex = static_cast<CTexture *>(depth);

   BPE_CHECK_SCE( sceGxmSetFragmentTexture( Context(), 0, ctex->GetGxmTexture() ) );
   sceGxmSetVertexProgram( Context(), downclearVertexProgram );
   if (GetCurrentOutputFormat() == SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4)
      sceGxmSetFragmentProgram( Context(), downclearFragmentProgramHalf );
   else
      sceGxmSetFragmentProgram( Context(), downclearFragmentProgram );

   float *uniform;
   sceGxmReserveVertexDefaultUniformBuffer( Context(), (void**)&uniform );
   uniform[0] = depth->GetWidth();
   uniform[1] = depth->GetHeight();

   BPE_CHECK_SCE( sceGxmSetVertexStream( Context(), 0, verts ) );
   BPE_CHECK_SCE( sceGxmDraw( Context(), SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, indexes, 3 ) );
   ++mCurrentSceneDrawNum;

   // Completely flush state after clear
   NVTAState::InvalidateHardwareState();
   sceGxmPopUserMarker( Context() );
}

void CRenderBackend::Clear(int const clearFlags, CColor const color, real32 const z, int const stencil )
{
   ResetScissor();

   uint16 *indexes = GetIndexBufferPool_RT()->AllocChunk( 3 ).Lock();
   CVector2 *verts = reinterpret_cast<CVector2 *>( GetVertexBufferPool_RT()->AllocChunk( 3 * sizeof( CVector2 ) )->Lock() );

   indexes[0] = 0;
   indexes[1] = 1;
   indexes[2] = 2;

   verts[0] = CVector2( -1.f, -1.f );
   verts[1] = CVector2( 3.f, -1.f );
   verts[2] = CVector2( -1.f, 3.f );

   sceGxmSetCullMode(Context(), SCE_GXM_CULL_NONE);
   if (clearFlags & kFlag_Depth)
      sceGxmSetFrontDepthWriteEnable( Context(), SCE_GXM_DEPTH_WRITE_ENABLED );
   else
      sceGxmSetFrontDepthWriteEnable( Context(), SCE_GXM_DEPTH_WRITE_DISABLED );
   sceGxmSetFrontDepthFunc( Context(), SCE_GXM_DEPTH_FUNC_ALWAYS );
   sceGxmSetFrontPolygonMode( Context(), SCE_GXM_POLYGON_MODE_TRIANGLE_FILL );

   sceGxmSetVertexProgram( Context(), clearVertexProgram );
   if (GetCurrentOutputFormat() == SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4)
      if (clearFlags & kFlag_Color)
         sceGxmSetFragmentProgram( Context(), clearFragmentProgramHalf );
      else
         sceGxmSetFragmentProgram( Context(), scissorFragmentProgramHalf );
   else
      if (clearFlags & kFlag_Color)
         sceGxmSetFragmentProgram( Context(), clearFragmentProgram );
      else
         sceGxmSetFragmentProgram( Context(), scissorFragmentProgram );

   float *clearcolor;
   if (clearFlags & kFlag_Color)
   {
      sceGxmReserveFragmentDefaultUniformBuffer( Context(), (void**)&clearcolor );
      CVector4 const &cv4 = color.GetVector4();
      clearcolor[0] = cv4.mX;
      clearcolor[1] = cv4.mY;
      clearcolor[2] = cv4.mZ;
      clearcolor[3] = cv4.mW;
   }

   BPE_CHECK_SCE( sceGxmSetVertexStream( Context(), 0, verts ) );
   BPE_CHECK_SCE( sceGxmDraw( Context(), SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, indexes, 3 ) );
   ++mCurrentSceneDrawNum;

   // Completely flush state after clear
   NVTAState::InvalidateHardwareState();
}

int gRazorTraceMetrics = 0;
int gCurrentRazorTraceMetrics = 0;
bool gDoRazorTraceReallySoon = false;
int gDoRazorCaptureReallySoon = false;
int gRazorTraceCountdown = 100;
int gDoRazorLiveMetrics = false;
int gReloadHudSettings = false;
int gDumpRazorLiveMetrics = false;
bool gLMActive = false;
int gCurrentLMGroup = -1;
int gLMGroup = 2;
int gLMCurrentBuffer = 0;
int gLMFrame = 0;
int gLMSpan = 4;
int gLMFrameIdx = 1;

#ifdef ENABLE_RAZOR_LIVEMETRICS
SceRazorGpuLiveResultInfo gRazorDrawResults;
#endif

void CRenderBackend::BeginScene()
{
#ifdef ENABLE_RAZOR_CAPTURE

   if ( gDoRazorCaptureReallySoon )
   {
      gDoRazorCaptureReallySoon = false;
      sceRazorCaptureSetTriggerNextFrame( "host0:mgs_razor_profile.sgx" );
   }
#endif
#ifdef ENABLE_RAZOR_HUD
   if (gCurrentRazorTraceMetrics != gRazorTraceMetrics)
   {
      sceRazorGpuTraceSetMetricsGroup(gRazorTraceMetrics);
      gCurrentRazorTraceMetrics = gRazorTraceMetrics;
   }
   if ( gDoRazorTraceReallySoon )
   {
      if (!--gRazorTraceCountdown)
      {
         // Trigger a capture after 100 frames.
         gRazorTraceCountdown = 100;
         gDoRazorTraceReallySoon = false;
         sceRazorGpuTraceSetFilename( "host0:mgs_razor_trace.pmon", 0 );
         sceRazorGpuTraceTrigger();
         gDoRazorCaptureReallySoon = true;
      }
   }
#endif
#ifdef ENABLE_RAZOR_LIVEMETRICS
   // grab them every 4 frames to be sure of getting a complete one
   gLMFrame++;
   if (gLMFrame % gLMSpan == 0)
   {
      if (!gDoRazorLiveMetrics && ~gLMActive && gReloadHudSettings)
      {
         sceRazorHudReloadSettings();
         gReloadHudSettings = 0;
      } 
      // only if enabled
      if (gDoRazorLiveMetrics)
      {
         bool match = gLMGroup == gCurrentLMGroup;

         // active, group match, swap buffers and update visible
         if (gLMActive && match)
         {
            sceRazorGpuLiveSetBuffer(sLMBuffers[gLMCurrentBuffer++], kLMBufferSize, &gRazorDrawResults);
            gLMCurrentBuffer %= kLMNumBuffers;
         }
         // active, group mismatch, stop
         if (gLMActive && !match)
         {
            sceRazorGpuLiveStop();
            gLMActive = false;
         }
         // not active, mismatch, set group
         if (!gLMActive && !match)
         {
            sceRazorGpuLiveSetMetricsGroup(gLMGroup);
            gCurrentLMGroup = gLMGroup;
         }
         // not active, start, set buffer
         if (!gLMActive)
         {
            sceRazorGpuLiveStart();
            sceRazorGpuLiveSetBuffer(sLMBuffers[gLMCurrentBuffer], kLMBufferSize, &gRazorDrawResults);
            gLMActive = true;
         }
      }
      else
      {
         // if active, stop
         if (gLMActive)
         {
            sceRazorGpuLiveStop();
            gLMActive = false;
         }
      }
   }
#endif
   mRenderBusyTimer.Reset();

   InternalBeginScene();

#if !ENABLE_SEPERATE_RENDER_THREAD
   InternalBeginFrameResourceTick();
#endif

//   FrameAllocator::Reset();

   mCurrentRenderTarget.mpColorBuffer[0] = NULL;
   mCurrentRenderTarget.mpDepthBuffer = NULL;
   //BPE_CHECK_SCE( sceGxmBeginScene(s_context, SCE_GXM_SCENE_FRAGMENT_TRANSFER_SYNC, s_renderTarget, NULL, NULL, s_dispSync[s_dispBack], &s_dispSurface[s_dispBack], &s_depthSurface) );
   BPE_CHECK_SCE( sceGxmSetVisibilityBuffer(s_context, s_visibility[s_dispBack], VISIBILITY_SLOTS * sizeof(uint32)) );

   sceGxmSetFrontDepthBias(s_context, 0, 0);

   mCurrentSceneDrawNum = 0;
   s_inScene = false;
   s_fullResCopied = false;
}

void CRenderBackend::BeginFrameResourceTick()
{
   InternalBeginFrameResourceTick();

   mpDynamicVertexBufferPool_RT->FrameReset();
   mpDynamicIndexBufferPool_RT->FrameReset();
   mpDynamicVertexBufferPool_UT->FrameReset();
   mpDynamicIndexBufferPool_UT->FrameReset();
}

void CRenderBackend::EndCapture()
{
   if (s_inScene)
   {
      BPE_CHECK_SCE( sceGxmEndScene(s_context, NULL, NULL) );
   }
   sceGxmFinish( s_context );
   s_inScene = false;
}

void CRenderBackend::EndScene()
{
#ifdef ENABLE_RAZOR_CAPTURE
   BPE_CHECK_SCE( sceRazorCpuPopMarker() );
   sceRazorCpuSync();
   BPE_CHECK_SCE( sceRazorCpuPushMarkerWithHud( "Renderer", SCE_RAZOR_COLOR_RED, SCE_RAZOR_MARKER_ENABLE_HUD ) );
#endif

   memset( s_visibility[s_dispBack], 0, VISIBILITY_SLOTS * SCE_GXM_GPU_CORE_COUNT * sizeof(uint32) );

#ifdef ENABLE_DETAILED_TIMING
   s_dispNote[s_dispBack].value = mVBLCount;

   BPE_CHECK_SCE( sceGxmEndScene(s_context, NULL, &s_dispNote[s_dispBack]) );
   s_dispTimer[s_dispBack].Reset();
#else
   BPE_CHECK_SCE( sceGxmEndScene(s_context, NULL, NULL) );
#endif
   s_inScene = false;

   sceKernelPowerTick( SCE_KERNEL_POWER_TICK_DEFAULT );

   // Must be called after sceGxmEndScene but before the buffers are flipped
//   if (mProcessCommonDialog)
   {
      UpdateCommonDialog();
   }

   // heartbeat function for Performance Analyzer
   BPE_CHECK_SCE( sceGxmPadHeartbeat(&s_dispSurface[s_dispBack], s_dispSync[s_dispBack]) );

   // add entry to display queue
   SDisplay display;
   display.address = s_dispBuf[s_dispBack];
   display.buffer_index = s_dispBack;
#ifdef ENABLE_DETAILED_TIMING
   display.timer.Reset();
   mRenderBusyElapsed += mRenderBusyTimer.GetElapsedTime() * 1000.0f;
   mDisplayWaitTimer.Reset();
#endif


   {
      uint64 preDisplayQueueTime = sceKernelGetProcessTimeWide();
      BPE_CHECK_SCE( sceGxmDisplayQueueAddEntry(s_dispSync[s_dispFront], s_dispSync[s_dispBack], &display) );
      mLastDisplayQueueFlipTime = sceKernelGetProcessTimeWide() - preDisplayQueueTime;
   }


   //mDisplayElapsed += mDisplayWaitTimer.GetElapsedTime() * 1000.0f;

   s_dispFront = s_dispBack;
   s_dispBack = (s_dispBack + 1) % DISPLAY_BUFFER_COUNT;

   for ( int i = 0; i < BPE_ARRAY_SIZE( mpRenderAllocators ); ++i )
   {
      mpRenderAllocators[i]->FlushPendingFree();
   }

   InternalPresent();

   NVTAState::PrintDebugStateCounts();

   UpdateVBLCount();
   mMetrics.mTimeSinceLastFlip = mFrameTimer.GetElapsedTime();
   mFrameTimer.Reset();


//   BPE_CHECK_SCE( sceGxmFinish( Context() ) );
   
#if 0
   BOOST_FOREACH( SceGxmVertexProgram *tempVertexProgram, mTempVertexPrograms )
   {
      sceGxmShaderPatcherReleaseVertexProgram( s_shaderPatcher, tempVertexProgram );
   }

   BOOST_FOREACH( SceGxmFragmentProgram *tempFragmentProgram, mTempFragmentPrograms )
   {
      sceGxmShaderPatcherReleaseFragmentProgram( s_shaderPatcher, tempFragmentProgram );
}
#endif

}

#if !defined(GOLD_VERSION)
extern "C" void BP_DebugText_Print(const char * fmt, ...);
#endif

#ifdef ENABLE_RAZOR_LIVEMETRICS
#define NUMJOBS (4*2*20)  // 4 frames, 2 units (vert/frag), 10 scenes (to cover the current possible sets)
SceRazorGpuLiveEntryJob jobs[NUMJOBS];
int jobsort[NUMJOBS/4];
int jobscn[NUMJOBS/4];
int frames[20][3];
int jtotal[2];

const char *scn_strs[] =
{
   "----",
   "---3",
   "--2-",
   "--23",
   "-1--",
   "-1-3",
   "-12-",
   "-123",
   "0---",
   "0--3",
   "0-2-",
   "0-23",
   "01--",
   "01-3",
   "012-",
   "0123",
};

void ClearJobs()
{
   for (int i = 0; i < NUMJOBS; ++i)
   {
      jobs[i].type = 255;
      jobs[i].endTime = 0;
      jobs[i].frameNumber = 0;
      jobs[i].sceneIndex = 255;
   }
   for (int i = 0; i < NUMJOBS/4; i++)
   {
      jobsort[i] = -1;
   }
   for (int i = 0; i < 20; i++)
   {
      frames[i][0] = 0;
      frames[i][1] = 0;
      frames[i][2] = 0;
   }
   jtotal[0] = 0;
   jtotal[1] = 0;
}

// return true if job A > job B
// for any pass, fragment > vertex thanks to the enum, start time if they're equal
bool CmpJob(int a, int b)
{
   if (jobs[a].type > jobs[b].type)
      return true;
   else if (jobs[a].type == jobs[b].type)
      return jobs[a].startTime > jobs[b].startTime;
   else
      return false;
}

void SumJob(int a, int b)
{
      SceRazorGpuLiveEntryJob *p, *job;
   
      p = &jobs[jobsort[a]];
      job = &jobs[jobsort[b]];

      switch (p->type)
      {
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX1:
         p->jobValues.vertexValues1.usseVertexProcessing += job->jobValues.vertexValues1.usseVertexProcessing;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT1:
         p->jobValues.fragmentValues1.usseFragmentProcessing += job->jobValues.fragmentValues1.usseFragmentProcessing;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX2:
         p->jobValues.vertexValues2.vdmPrimitivesInput += job->jobValues.vertexValues2.vdmPrimitivesInput;
         p->jobValues.vertexValues2.mtePrimitivesOutput += job->jobValues.vertexValues2.mtePrimitivesOutput;
         p->jobValues.vertexValues2.vdmVerticesInput += job->jobValues.vertexValues2.vdmVerticesInput;
         p->jobValues.vertexValues2.mteVerticesOutput += job->jobValues.vertexValues2.mteVerticesOutput;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT2:
         p->jobValues.fragmentValues2.rasterizedPixelsBeforeHsr += job->jobValues.fragmentValues2.rasterizedPixelsBeforeHsr;
         p->jobValues.fragmentValues2.rasterizedOutputPixels += job->jobValues.fragmentValues2.rasterizedOutputPixels;
         p->jobValues.fragmentValues2.rasterizedOutputSamples += job->jobValues.fragmentValues2.rasterizedOutputSamples;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX3:
         p->jobValues.vertexValues3.bifTaMemoryWrite += job->jobValues.vertexValues3.bifTaMemoryWrite;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT3:
         p->jobValues.fragmentValues3.bifIspParameterFetchMemoryRead += job->jobValues.fragmentValues3.bifIspParameterFetchMemoryRead;
         break;
      }
}

void MergeJobs()
{
   // find jobs with overlapping time ranges, when found, or their scene indexes together and perform a union on their times
   int i, j;

   j = 0;
   i = 1;

   jobscn[j] = jobs[jobsort[j]].sceneIndex;
   // i = current mergable job
   // j = job to merge into
   while (i < NUMJOBS/4)
   {
      SceRazorGpuLiveEntryJob *a, *b;

      if (jobsort[i] < 0)
         break;

      a = &jobs[jobsort[j]];
      b = &jobs[jobsort[i]];

      long long s0, e0, s1, e1;
      s0 = a->startTime;
      e0 = a->endTime;
      s1 = b->startTime;
      e1 = b->endTime;

      // jobs are already sorted by start time, so we just have to see if the second job's start time occurs within the current job
#if 0
      if ((a->type == b->type) &&(s1 >= s0) && (s1 <= e0))
      {
         // and if so, merge the scene indexes, and pick the later end time
         if (e0 > e1)
            a->endTime = e0;
         else
            a->endTime = e1;
         jobscn[j] |= 1 << (3-b->sceneIndex);
         SumJob(j, i);
      }
      else
#endif
      {
         // otherwise, copy that job down to the next entry, set it's initial scene mask
         j++;
         jobsort[j] = jobsort[i];
         jobscn[j] = jobs[jobsort[j]].sceneIndex;
      }
      i++;
   }
   while (j < NUMJOBS/4-1)
      jobsort[++j] = -1;
}

void SortJobs()
{
   int frame = 0;
   int numjobs = 0;
   int idx = gLMFrameIdx;

   SceRazorGpuLiveEntryJob *p, *end;
   p = jobs;
   end = jobs + NUMJOBS;
   
   // find the second frame in the capture
   while (p < end)
   {
      if (p->type == 255)
         break;
      // vertex jobs are odd, and should all be in a group
      if (p->type & 1)
      {
         if (p->frameNumber)
         {
            if (frame && frame != p->frameNumber)
            {
               frame = p->frameNumber;
               idx--;
            }
            else
               frame = p->frameNumber;
            if (!idx)
               break;
         }
      }
      p++;
   }
   p = jobs;
   if (!frame)
      return;

   // generate frame counts
   {
      while (p < end)
      {
         int i;
         if (p->type == 255)
            break;
         if (p->type == SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FIRMWARE || !p->frameNumber)
         {
            p++;
            continue;
         }
         for (i = 0; i < 20; i++)
         {
            if (!frames[i][0] || frames[i][0] == p->frameNumber)
               break;
         }
         if (p->type & 1)
            jtotal[0]++;
         else
            jtotal[1]++;
         if (i < 20)
         {
            frames[i][0] = p->frameNumber;
            if (p->type & 1)
               frames[i][1]++;
            else
               frames[i][2]++;
         }
         p++;
      }
   }
   p = jobs;

   // filter down to just vertex and fragment jobs
   while (p < end)
   {
      if (p->type == 255)
         break;
      if (p->frameNumber != frame)
      {
         p++;
         continue;
      }
      if (p->type == SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FIRMWARE)
      {
         p++;
         continue;
      }
      jobsort[numjobs++] = p-jobs;
      ++p;
   }

   // sort jobs based on type and start time, simple bubblesort
   int n = numjobs;
   int last;
   int i;
   int t;
   do
   {
      last = 0;
      for (i = 1; i < n; i++)
      {
         if (CmpJob(jobsort[i-1], jobsort[i]))
         {
            t = jobsort[i];
            jobsort[i] = jobsort[i-1];
            jobsort[i-1] = t;
            last = i;
         }
      }
      n = last;
   } while (n);
}

void DumpJobs(unsigned long long start)
{
   SceRazorGpuLiveEntryJob *p, *end;
   p = jobs;
   end = jobs + NUMJOBS;

   long long dur;
   long long s;
   
   int i;

   BP_DebugText_Print("Total V(%d) F(%d)", jtotal[0], jtotal[1]);
   for (i = 0; i < 20; i++)
   {
      if (!frames[i][0])
         break;
      BP_DebugText_Print("Frm %5d  V(%d) F(%d)", frames[i][0], frames[i][1], frames[i][2]);
   }
   BP_DebugText_Print("T %5s %4s %8s %8s  %-48s", "FRM", "SCN", "Start", "Dura", "Values");
   for (i = 0; i < NUMJOBS/4; i++)
   {
      if (jobsort[i] < 0)
         return;
      p = &jobs[jobsort[i]];
      if (p->type == 255)
         return;
      dur = p->endTime - p->startTime;
      s = (p->startTime > start) ? p->startTime - start : -(start - p->startTime);
      switch (p->type)
      {
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX1:
         BP_DebugText_Print("V %5d %4d %8.3f %8.3f  usse %-43.2f", p->frameNumber, jobscn[i], 
                            s / 1000.f, dur / 1000.f, p->jobValues.vertexValues1.usseVertexProcessing);
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT1:
         BP_DebugText_Print("F %5d %4d %8.3f %8.3f  usse %-43.2f", p->frameNumber, jobscn[i], 
                            s / 1000.f, dur / 1000.f, p->jobValues.fragmentValues1.usseFragmentProcessing);
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX2:
         BP_DebugText_Print("V %5d %4d %8.3f %8.3f  in/out prim (%6d/%6d) vert (%6d/%6d)", p->frameNumber, jobscn[i], 
                            s / 1000.f, dur / 1000.f, 
                            p->jobValues.vertexValues2.vdmPrimitivesInput, p->jobValues.vertexValues2.mtePrimitivesOutput,
                            p->jobValues.vertexValues2.vdmVerticesInput, p->jobValues.vertexValues2.mteVerticesOutput);
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT2:
         BP_DebugText_Print("F %5d %4d %8.3f %8.3f  in/out pix (%9d/%9d)                ", p->frameNumber, jobscn[i], 
                            s / 1000.f, dur / 1000.f, 
                            p->jobValues.fragmentValues2.rasterizedPixelsBeforeHsr, p->jobValues.fragmentValues2.rasterizedOutputPixels);
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX3:
         BP_DebugText_Print("V %5d %4d %8.3f %8.3f  pbuf write %8d                             ", p->frameNumber, jobscn[i], 
                            s / 1000.f, dur / 1000.f, p->jobValues.vertexValues3.bifTaMemoryWrite);
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT3:
         BP_DebugText_Print("F %5d %4d %8.3f %8.3f  pbuf read  %8d                             ", p->frameNumber, jobscn[i],
                            s / 1000.f, dur / 1000.f, p->jobValues.fragmentValues3.bifIspParameterFetchMemoryRead);
         break;
      }
   }
}

void AccumulateJob(SceRazorGpuLiveEntryJob *job)
{
   SceRazorGpuLiveEntryJob *p, *end;

   p = jobs;
   end = jobs + NUMJOBS;
   
   if (!job->frameNumber)
      return;
   while (p < end)
   {
      if (p->type == 255)
         break;
      if (job->type == p->type
          && job->startTime == p->startTime
          && job->frameNumber == p->frameNumber
          && job->sceneIndex == p->sceneIndex)
         break;
      ++p;
   }
   if (p == end)
      return;
   if (p->type == 255)
   {
      p->type = job->type;
      p->startTime = job->startTime;
      p->endTime = job->endTime;
      p->frameNumber = job->frameNumber;
      p->sceneIndex = job->sceneIndex;
      memcpy(p->jobValues.reserved.reserved, job->jobValues.reserved.reserved, sizeof(job->jobValues.reserved.reserved));
   }
   else
   {
      p->endTime = (job->endTime > p->endTime) ? job->endTime : p->endTime;
      switch (p->type)
      {
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX1:
         p->jobValues.vertexValues1.usseVertexProcessing += job->jobValues.vertexValues1.usseVertexProcessing;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT1:
         p->jobValues.fragmentValues1.usseFragmentProcessing += job->jobValues.fragmentValues1.usseFragmentProcessing;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX2:
         p->jobValues.vertexValues2.vdmPrimitivesInput += job->jobValues.vertexValues2.vdmPrimitivesInput;
         p->jobValues.vertexValues2.mtePrimitivesOutput += job->jobValues.vertexValues2.mtePrimitivesOutput;
         p->jobValues.vertexValues2.vdmVerticesInput += job->jobValues.vertexValues2.vdmVerticesInput;
         p->jobValues.vertexValues2.mteVerticesOutput += job->jobValues.vertexValues2.mteVerticesOutput;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT2:
         p->jobValues.fragmentValues2.rasterizedPixelsBeforeHsr += job->jobValues.fragmentValues2.rasterizedPixelsBeforeHsr;
         p->jobValues.fragmentValues2.rasterizedOutputPixels += job->jobValues.fragmentValues2.rasterizedOutputPixels;
         p->jobValues.fragmentValues2.rasterizedOutputSamples += job->jobValues.fragmentValues2.rasterizedOutputSamples;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX3:
         p->jobValues.vertexValues3.bifTaMemoryWrite += job->jobValues.vertexValues3.bifTaMemoryWrite;
         break;
      case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT3:
         p->jobValues.fragmentValues3.bifIspParameterFetchMemoryRead += job->jobValues.fragmentValues3.bifIspParameterFetchMemoryRead;
         break;
      }
   }
}

#endif

void CRenderBackend::PrintTimers()
{
#ifdef ENABLE_DETAILED_TIMING
   BP_DebugText_Print("game %8.4f render %8.4f disp (%8.4f,%8.4f,%8.4f)", mGameElapsed, mRenderBusyElapsed, s_dispElapsed[0], s_dispElapsed[1], s_dispElapsed[2]);
   mGameElapsed = mRenderElapsed = mDisplayElapsed = mRenderBusyElapsed = 0.0f;
#endif
}

void CRenderBackend::PrintLiveMetrics()
{
#ifdef ENABLE_RAZOR_LIVEMETRICS
   SceRazorGpuLiveResultInfo &r = gRazorDrawResults;

   BP_DebugText_Print("%d entries, %d overflow", r.entryCount, r.overflowCount);
   BP_DebugText_Print("%lldus span", r.endTime - r.startTime);

   if (r.resultData)
   {
      unsigned int ptr = (unsigned int)r.resultData;
      unsigned long long start = r.startTime;

      if (gDumpRazorLiveMetrics)
      {
         gDumpRazorLiveMetrics = false;
         FILE *fp = fopen("host0:metrics.txt", "wt");

         for (int i = 0; i < r.entryCount; i++)
         {
            SceRazorGpuLiveEntryHeader const *hdr = (SceRazorGpuLiveEntryHeader const *)ptr;

            if (hdr->entryType == SCE_RAZOR_LIVE_TRACE_METRIC_ENTRY_TYPE_JOB)
            {
               SceRazorGpuLiveEntryJob *p = (SceRazorGpuLiveEntryJob *)ptr;
               fprintf(fp, "JOB   %d   %lld   %d %d %d\n", p->frameNumber, p->startTime, p->sceneIndex, p->type, p->core);
            }
            else if (hdr->entryType == SCE_RAZOR_LIVE_TRACE_METRIC_ENTRY_TYPE_PARAMETER_BUFFER)
            {
               SceRazorGpuLiveEntryParameterBuffer *p = (SceRazorGpuLiveEntryParameterBuffer *)ptr;
               fprintf(fp, "PBUF  %d   %lld   %d %d\n", p->peakUsage, p->peakUsageTime, p->partialRender, p->vertexJobPaused);
            }
            else if (hdr->entryType == SCE_RAZOR_LIVE_TRACE_METRIC_ENTRY_TYPE_FRAME)
            {
               SceRazorGpuLiveEntryFrame *p = (SceRazorGpuLiveEntryFrame *)ptr;
               fprintf(fp, "FRAME %d   %lld   %d\n", p->frameNumber, p->startTime, p->duration);
            }
            ptr += hdr->entrySize;
         }
         fclose(fp);
      }

      ClearJobs();

      for (int i = 0; i < r.entryCount; i++)
      {
         SceRazorGpuLiveEntryHeader const *hdr = (SceRazorGpuLiveEntryHeader const *)ptr;

         if (hdr->entryType == SCE_RAZOR_LIVE_TRACE_METRIC_ENTRY_TYPE_JOB) 
         {
            SceRazorGpuLiveEntryJob *p = (SceRazorGpuLiveEntryJob *)ptr;

            switch (p->type)
            {
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX0:
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX1:
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX2:
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_VERTEX3:
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT0:
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT1:
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT2:
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FRAGMENT3:
               AccumulateJob(p);
               break;
            case SCE_RAZOR_LIVE_TRACE_METRIC_JOB_TYPE_FIRMWARE:
               break;
            }
         }
         else if (hdr->entryType == SCE_RAZOR_LIVE_TRACE_METRIC_ENTRY_TYPE_PARAMETER_BUFFER)
         {
            SceRazorGpuLiveEntryParameterBuffer *p = (SceRazorGpuLiveEntryParameterBuffer *)ptr;
            BP_DebugText_Print("peak %16d  partial %d  paused %d", p->peakUsage, p->partialRender, p->vertexJobPaused);
         }
         else if (hdr->entryType == SCE_RAZOR_LIVE_TRACE_METRIC_ENTRY_TYPE_FRAME)
         {
         }
         ptr += hdr->entrySize;
      }
      SortJobs();
      MergeJobs();
      DumpJobs(start);
   }
#endif
}

void CRenderBackend::ShutDown()
{
#if ENABLE_SEPERATE_RENDER_THREAD
   ShutdownThreadSystem();
#endif
}

extern int gVTADebugUberscale;

void CRenderBackend::BackupScene(CBaseTexture *dst)
{
   if (!s_inScene)
      return;

   BPE_VERIFY(GetCurrentRenderTexture() == GetCurrentDisplayTexture(), false, "Attempting to use BackupScene() when we aren't rendering to the backbuffer");

   BPE_CHECK_SCE( sceGxmEndScene(s_context, NULL, NULL) );

   CTexture *srcTex = reinterpret_cast<CTexture*>(GetCurrentRenderTexture());
   CTexture *dstTex = reinterpret_cast<CTexture*>(dst);

   void *srcAddr;
   int srcPitch;
   void *dstAddr;
   int dstPitch;
   
   srcTex->Lock(&srcAddr, &srcPitch);
   dstTex->Lock(&dstAddr, &dstPitch);

   BPE_CHECK_SCE( sceGxmTransferCopy(
                     srcTex->GetWidth(), srcTex->GetHeight(),
                     0, 0,
                     SCE_GXM_TRANSFER_COLORKEY_NONE,
                     SCE_GXM_TRANSFER_FORMAT_RAW32, SCE_GXM_TRANSFER_LINEAR, srcAddr, 0, 0, srcPitch,
                     SCE_GXM_TRANSFER_FORMAT_RAW32, SCE_GXM_TRANSFER_LINEAR, dstAddr, 0, 0, dstPitch,
                     NULL,
                     SCE_GXM_TRANSFER_FRAGMENT_SYNC,
                     NULL) );
   BPE_CHECK_SCE( sceGxmBeginScene(s_context, SCE_GXM_SCENE_FRAGMENT_TRANSFER_SYNC, s_internalRenderTargets[0].rt, NULL, NULL, s_dispSync[s_dispBack], &s_dispSurface[s_dispBack], &s_depthSurface) );
}

void CRenderBackend::ChangeScene(int newTarget, SRenderTarget const & renderTarget)
{
   SceGxmColorSurface *csurf = NULL;
   SceGxmDepthStencilSurface *dsurf = NULL;
   SceGxmSyncObject *fragSync = NULL;

   if (s_inScene)
   {
      BPE_CHECK_SCE( sceGxmEndScene(s_context, NULL, NULL) );
   }

   if (renderTarget.mpColorBuffer[0])
      csurf = static_cast<CTexture *>(renderTarget.mpColorBuffer[0])->GxmColorSurface();
   if (renderTarget.mpDepthBuffer)
      dsurf = static_cast<CTexture *>(renderTarget.mpDepthBuffer)->GxmDepthStencilSurface();

   // bit of a hack
   if (!newTarget)
   {
      csurf = &s_dispSurface[s_dispBack];
      dsurf = &s_depthSurface;
      fragSync = s_dispSync[s_dispBack];
   }

   int w, h;
   w = h = 0;
   s_currentOutputFormat = SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4;

   if (renderTarget.mpColorBuffer[0])
   {
      if ( renderTarget.mpColorBuffer[0]->GetFormat() == CBaseTexture::kFormat_A16B16G16R16F )
      {
         s_currentOutputFormat = SCE_GXM_OUTPUT_REGISTER_FORMAT_HALF4;
      }

      w = renderTarget.mpColorBuffer[0]->GetWidth();
      h = renderTarget.mpColorBuffer[0]->GetHeight();
   }
   else if (renderTarget.mpDepthBuffer)
   {
      w = renderTarget.mpDepthBuffer->GetWidth();
      h = renderTarget.mpDepthBuffer->GetHeight();
   }

   if (s_internalRenderTargets[newTarget].tiled)
   {
      BPE_CHECK_SCE( sceGxmBeginScene(s_context, SCE_GXM_SCENE_FRAGMENT_TRANSFER_SYNC, s_internalRenderTargets[newTarget].rt, NULL, NULL, fragSync, csurf, dsurf) );
      sceGxmSetFrontDepthBias(s_context, 0, 0);
   }
   else
   {
      SceGxmValidRegion region;
      sceGxmColorSurfaceSetClip(csurf, 0, 0, w-1, h-1);
      region.xMax = w-1;
      region.yMax = h-1;
      BPE_CHECK_SCE( sceGxmBeginScene(s_context, SCE_GXM_SCENE_FRAGMENT_TRANSFER_SYNC, s_internalRenderTargets[newTarget].rt, &region, NULL, fragSync, csurf, dsurf) );
   }

   if (s_internalRenderTargets[newTarget].msaa)
   {
      s_currentMultisampleMode = SCE_GXM_MULTISAMPLE_4X;
      mMSAATrickEnabled = true;
   }
   else
   {
      s_currentMultisampleMode = SCE_GXM_MULTISAMPLE_NONE;
      mMSAATrickEnabled = false;
   }

   // set viewport here so the Z clear is correct
   SetViewport();

   // leave the raster mask untouched over the lowres->MSAA transition
   if (dsurf && !mMSAATrickEnabled)
   {
      // flag the raster mask to be reinitialized
      mRasterMaskValid = false;
   }

   if (!newTarget && !s_fullResCopied)
   {
      // if we aren't in a scene, we clear depth to make up for the unconditional clear removed from frame.c
      // clearing color too causes certain held frames to go black, like the MGS2 load text screen, or the post-codec blip
      // if we are in a scene, and haven't copied up yet, this is switching from the lowres to hires buffers
      // so we should upscale it
      if (!s_inScene)
      {
         Clear(kFlag_Depth, CColor::Black());
      }
      else
      {
         CTexture *ctex = static_cast<CTexture *>(s_drawColor);
         extern int gVTADebugRescaleWidth;
         extern int gVTADebugRescaleHeight;
         ctex->SetWidth( gVTADebugRescaleWidth );
         ctex->SetHeight( gVTADebugRescaleHeight );

         SetCullMode(kCM_None);
         SetDepthCompareEnabled(false);
         SetDepthWriteEnabled(false);
         NVTAState::PolySetMode( SCE_GXM_POLYGON_MODE_TRIANGLE_FILL );
         sceGxmSetFrontStencilFunc( Context(), SCE_GXM_STENCIL_FUNC_ALWAYS, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, SCE_GXM_STENCIL_OP_KEEP, 0xFF, 0xFF);

         if (gVTADebugUberscale && mOverrideUpscale && mpUpscaleFunc)
            mpUpscaleFunc();
         else
            UpscaleDrawBuffer();
         mOverrideUpscale = false;
      }
      s_fullResCopied = true;
   }
      
   s_inScene = true;
}

void CRenderBackend::SetUpscaleOverride( void (*func)() )
{
   mpUpscaleFunc = func;
}

void CRenderBackend::OverrideUpscale()
{
   mOverrideUpscale = true;
}

void CRenderBackend::SetViewport()
{
   int viewportX = 0;
   int viewportY = 0;
   int viewportWidth = mRenderTargetWidth;
   int viewportHeight = mRenderTargetHeight;

   if( mCurrentRenderTarget.mBindBackBuffer != SRenderTarget::kBBM_NoBackBuffer )
   {
      switch( mCurrentRenderTarget.mBindBackBuffer )
      {
      case SRenderTarget::kBBM_BackBufferFill:
         {
            viewportX = 0;
            viewportY = 0;
            viewportWidth = GetBackBufferWidth();
            viewportHeight = GetBackBufferHeight();
         }
         break;

      case SRenderTarget::kBBM_BackBufferCentered:
         {
            int bufferHeight = GetBackBufferHeight();

            if ( GetFrameBufferSize().GetDisplayAspect() == SRenderFrameBufferSize::kDA_Fullscreen )
               bufferHeight *= (3.0f / 4.0f);

            int const diffX = GetBackBufferWidth() - GetMainFrameBufferWidth();
            int const diffY = GetBackBufferHeight() - bufferHeight;
            viewportX = diffX / 2;
            viewportY = diffY / 2;
            viewportWidth = GetMainFrameBufferWidth();
            viewportHeight = bufferHeight;
         }
         break;
      }
   }

   if( mCurrentRenderTarget.mRenderTargetViewPort.mViewportEnabled )
   {
      viewportX = mCurrentRenderTarget.mRenderTargetViewPort.mViewportX;
      viewportY = mCurrentRenderTarget.mRenderTargetViewPort.mViewportY;
      viewportWidth = mCurrentRenderTarget.mRenderTargetViewPort.mViewportWidth;
      viewportHeight = mCurrentRenderTarget.mRenderTargetViewPort.mViewportHeight;
   }

   // This viewport set up matches D3D conventions
   float scale[4];
   scale[0] = viewportWidth * 0.5f;
   scale[1] = -viewportHeight * 0.5f;
   //scale[2] = ( mCurrentRenderTarget.mMaxZ - mCurrentRenderTarget.mMinZ ) * 0.5f;
   // can't rescale the Z yet, the clear in ChangeScene will use the default viewport
   scale[2] = mCurrentRenderTarget.mMaxZ - mCurrentRenderTarget.mMinZ; // the clip space this game uses maps Z from 0 to 1, remap it to mMinZ .. mMaxZ appropriately
   scale[3] = 0.0f;

   float offset[4];
   offset[0] = viewportX + viewportWidth * 0.5f;
   offset[1] = viewportY + viewportHeight * 0.5f;
   //offset[2] = ( mCurrentRenderTarget.mMinZ + mCurrentRenderTarget.mMaxZ ) * 0.5f;
   offset[2] = mCurrentRenderTarget.mMinZ;
   offset[3] = 0.0f;

   // PS2 uses the D3D9 convention of texel centers at 0.5, pixel centers at 0.0, sampling on pixel centers
   // The Vita uses the OpenGL/D3D10+ convetion of texel centers at 0.5, pixel centers at 0.5, sampling on pixel centers
   // Most of the viewports add a half pixel offset to match the PS2's conventions, to get lines and such rendering as close to bug-perfect as possible
   // This complicates the rescaling (and fullscreen work in general) immensely, so mPixelCentersHalf indicates a scene where primitives expect texel and pixel centers
   // to both be at half-integer positions

   // Just to confuse things more, the MSAA tricked halfres particle buffer uses a 0.25 pixel offset, to make the sample centers line up
   if (!mCurrentRenderTarget.mPixelCentersHalf)
   {
      // MSAA offsets by 0.25, so the samples line up appropriately
      if (mMSAATrickEnabled)
      {
         offset[0] += 0.25f;
         offset[1] += 0.25f;
      }
      else
      {
         offset[0] += 0.5f;
         offset[1] += 0.5f;
      }
   }

   sceGxmSetViewport(Context(), offset[0], scale[0], offset[1], scale[1], offset[2], scale[2] );

   // scissor should be done after viewport, as it needs to draw some clear polys
   if( mCurrentRenderTarget.mScissorEnabled )
   {
      SetScissor(mCurrentRenderTarget.mScissorX, mCurrentRenderTarget.mScissorY, mCurrentRenderTarget.mScissorWidth, mCurrentRenderTarget.mScissorHeight);
   }
   else
   {
      // Set scissor test to viewport rectangle
      SetScissor(viewportX, viewportY, viewportWidth, viewportHeight);
   }

   mViewportWidth = viewportWidth;
   mViewportHeight = viewportHeight;

}

void CRenderBackend::SetRenderTarget(SRenderTarget const & renderTarget)
{
   CTexture const *currentTex, *newTex;
   int newIdx, currentIdx;

   if( mCurrentRenderTarget == renderTarget )
      return;

   if (mCurrentRenderTarget.mpColorBuffer[0])
      currentTex = static_cast<CTexture const *>(mCurrentRenderTarget.mpColorBuffer[0]);
   else
      currentTex = static_cast<CTexture const *>(mCurrentRenderTarget.mpDepthBuffer);

   if (renderTarget.mpColorBuffer[0])
      newTex = static_cast<CTexture const *>(renderTarget.mpColorBuffer[0]);
   else
      newTex = static_cast<CTexture const *>(renderTarget.mpDepthBuffer);

   mCurrentRenderTarget = renderTarget;

   newIdx = 0;
   if (newTex)
      newIdx = newTex->GetRenderTargetIndex();
   currentIdx = 0;
   if (currentTex)
      currentIdx = currentTex->GetRenderTargetIndex();

   if (newIdx)
   {
      mRenderTargetWidth = newTex->GetWidth();
      mRenderTargetHeight = newTex->GetHeight();
   }
   else
   {
      mRenderTargetWidth = 960;
      mRenderTargetHeight = 544;
   }

//   mCurrentRenderTargetAAType = CBaseTexture::kAA_None;

   // If we're changing scenes, we want to do it with correct scissor values for the new scene, so that if it tries to clear
   // it does the clear properly
   if (newIdx != currentIdx || !s_inScene)
   {
      ChangeScene(newIdx, renderTarget);
   }
   else
   {
      // ChangeScene sets it's own viewport right after sceGxmBeginScene
      // if we aren't changing scenes, update the viewport and scissor here
      SetViewport();
   }

   ResetScissor();
}

void CRenderBackend::SetIndexData(CIndexBuffer const * indexBuffer)
{
   mpLastIndexBuffer = indexBuffer->mpMemory_RT->mpAddress;
   NVTAState::SetLastIndexBufferType( indexBuffer->mType );
   
   mLastIndexBufferCount = indexBuffer->mCount;
}

void CRenderBackend::SetIndexData_OneToOne( uint16 const count )
{
   CIndexBuffer const * indexBuffer = mpNoPrimitiveIndexBuffer.get();

   mpLastIndexBuffer = indexBuffer->mpMemory_RT->mpAddress;
   NVTAState::SetLastIndexBufferType( indexBuffer->mType );

   mLastIndexBufferCount = count;
}

void CRenderBackend::SetIndexData(CIndexBufferChunk const &chunk)
{
   CIndexBuffer const *pIndexBuffer = chunk.GetIndexBuffer();

   NVTAState::SetLastIndexBufferType( pIndexBuffer->mType );
   mLastIndexBufferCount = chunk.GetCount();
   mpLastIndexBuffer = reinterpret_cast<uint8 const *>( chunk.GetMemory() );
}

// Be very careful to reset this if using
void CRenderBackend::SetIndexSourceOverwrite( uint32 const streamIndex, SceGxmIndexSource indexSource )
{
   NVTAState::SetIndexSourceOverwrite( streamIndex, indexSource );
}

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode) 
{
   if ( !enable )
   {
      NVTAState::BlendDisable();
   }
   else
   {
      SceGxmBlendInfo blend;

      blend.colorSrc = srcBlendMode;
      blend.alphaSrc = srcBlendMode;
      blend.colorDst = dstBlendMode;
      blend.alphaDst = dstBlendMode;

      NVTAState::BlendEnableAndSetSrcDst( blend );
   }
}

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode, uint32 const fixedColor)
{
   // WARNING - Fixed color not supported!
   SetBlendMode( enable, srcBlendMode, dstBlendMode );
}

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode)
{
   if ( !enable )
   {
      NVTAState::BlendDisable();
   }
   else
   {
      SceGxmBlendInfo blend;

      blend.colorSrc = rgbSrcBlendMode;
      blend.alphaSrc = aSrcBlendMode;
      blend.colorDst = rgbDstBlendMode;
      blend.alphaDst = aDstBlendMode;

      NVTAState::BlendEnableAndSetSrcDst( blend );
   }
}

void CRenderBackend::SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode, uint32 const rgbaFixed)
{
   SetBlendMode( enable, rgbSrcBlendMode, rgbDstBlendMode, aSrcBlendMode, aDstBlendMode );
}

void CRenderBackend::SetBlendOp(EBlendOp const blendOp)
{
   SceGxmBlendInfo blend;

   blend.colorFunc = blend.alphaFunc = blendOp;

   NVTAState::BlendSetOp( blend );
}

void CRenderBackend::SetBlendOp(EBlendOp const colorBlendOp, EBlendOp const alphaBlendOp)
{
   SceGxmBlendInfo blend;

   blend.colorFunc = colorBlendOp;
   blend.alphaFunc = alphaBlendOp;

   NVTAState::BlendSetOp( blend );
}

void CRenderBackend::SetColorAlphaWriteEnabled(bool const color, bool const alpha)
{
   uint8_t newMask = 0;
   if ( color ) newMask |= SCE_GXM_COLOR_MASK_R | SCE_GXM_COLOR_MASK_G | SCE_GXM_COLOR_MASK_B;
   if ( alpha ) newMask |= SCE_GXM_COLOR_MASK_A;

   NVTAState::BlendSetMask( newMask );
}

// enable disable stencil testing
void CRenderBackend::SetStencilEnable(bool enable) {}
void CRenderBackend::SetStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass ) {}
void CRenderBackend::SetStencilMask(uint32 mask) {}
void CRenderBackend::SetStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask) {}

void CRenderBackend::SetTexture(int const texUnit, CBaseTexture const * pTexture, bool const allowAniso ) 
{
   pTexture->SetCurrentTextureParams(texUnit);
   pTexture->SetTexture( texUnit );
}

void CRenderBackend::SetTextureAddressMode(int const texUnit, EWrapMode const wrapU, EWrapMode const wrapV) 
{
   SceGxmTexture *pTexture = NVTATextureState::sTextures + texUnit;
   NVTATextureState::SOtherTextureState &other = NVTATextureState::sOtherTextureStates[ texUnit ];

   other.mUAddrMode = SceGxmTextureAddrMode( wrapU );
   other.mVAddrMode = SceGxmTextureAddrMode( wrapV );

   if ( sceGxmTextureGetType( pTexture ) != SCE_GXM_TEXTURE_LINEAR_STRIDED )
   {
      sceGxmTextureSetUAddrMode( pTexture, other.mUAddrMode );
      sceGxmTextureSetVAddrMode( pTexture, other.mVAddrMode );
   }

   NVTATextureState::SetTextureDirty( texUnit );
}

static SceGxmTextureFilter const skTextureFilterModes[] = 
{
   SCE_GXM_TEXTURE_FILTER_POINT,  // kFM_Nearest,
   SCE_GXM_TEXTURE_FILTER_LINEAR, // kFM_Linear,
   SCE_GXM_TEXTURE_FILTER_POINT, // kFM_Nearest_Nearest,
   SCE_GXM_TEXTURE_FILTER_LINEAR,// kFM_Linear_Nearest,
   SCE_GXM_TEXTURE_FILTER_POINT, // kFM_Nearest_Linear,
   SCE_GXM_TEXTURE_FILTER_LINEAR // kFM_Linear_Linear,
};

void CRenderBackend::SetTextureFilter(int const texUnit, EFilterMode minFilter, EFilterMode magFilter) 
{
   SceGxmTexture *pTexture = NVTATextureState::sTextures + texUnit;
   NVTATextureState::SOtherTextureState &other = NVTATextureState::sOtherTextureStates[ texUnit ];

   other.mMinFilter = skTextureFilterModes[ minFilter ];
   other.mMagFilter = skTextureFilterModes[ magFilter ];

   if ( sceGxmTextureGetType( pTexture ) != SCE_GXM_TEXTURE_LINEAR_STRIDED )
   {
      sceGxmTextureSetMinFilter( pTexture, other.mMinFilter );
   }

   sceGxmTextureSetMagFilter( pTexture, other.mMagFilter );
   NVTATextureState::SetTextureDirty( texUnit );
}

void CRenderBackend::SetVertexRegisters(int startRegister, int numVectors, CVector4 const * pVector) 
{
   float32x4_t *pDstReg = (float32x4_t *) NVTAState::mVertexRegisters;
   pDstReg += startRegister;
   
   float32x4_t const *pSrcReg = reinterpret_cast<float32x4_t const *>( pVector );

   for ( int i = 0; i < numVectors; ++i )
   {
      pDstReg[i] = pSrcReg[i];
   }

   NVTAState::AddDirtyState( NVTAState::kDS_VertexRegs );
}

void CRenderBackend::SetFragmentRegisters(int startRegister, int numVectors, CVector4 const * pVector) 
{
   float32x4_t *pDstReg = (float32x4_t *) NVTAState::mFragmentRegisters;
   pDstReg += startRegister;

   float32x4_t const *pSrcReg = reinterpret_cast<float32x4_t const *>( pVector );

   for ( int i = 0; i < numVectors; ++i )
   {
      pDstReg[i] = pSrcReg[i];
   }

   NVTAState::AddDirtyState( NVTAState::kDS_FragmentRegs );
}

void CRenderBackend::RenderQuadVC(real32 minX, real32 const maxX, real32 const minY, real32 const maxY, real32 const z, real32 const minU, real32 const maxU, real32 const minV, real32 const maxV)
{
   CRenderBackend *pThis = static_cast<CRenderBackend *>(this);

   pThis->SetIndexData_OneToOne( 4 );

   struct SVertex
   { 
      real32 x, y, z;
      real32 u, v;
      real32 r, g, b, a;
   };

   SVertex const vertices[] =
   {
      { minX, maxY, z, minU, minV, 1.0f, 1.0f, 1.0f, 1.0f },
      { maxX, maxY, z, maxU, minV, 1.0f, 1.0f, 1.0f, 1.0f },
      { minX, minY, z, minU, maxV, 1.0f, 1.0f, 1.0f, 1.0f },
      { maxX, minY, z, maxU, maxV, 1.0f, 1.0f, 1.0f, 1.0f }
   };

   static NVTAState::SAttributeDesc const skAttributes[] = 
   {
      { kVDU_Position, (SceGxmVertexAttribute) { 0, 0, SCE_GXM_ATTRIBUTE_FORMAT_F32, 3, 0 } },
      { kVDU_TexCoord0, (SceGxmVertexAttribute) { 0, 3 * sizeof(float), SCE_GXM_ATTRIBUTE_FORMAT_F32, 2, 0 } },
      { kVDU_Color0, (SceGxmVertexAttribute) { 0, 5 * sizeof(float), SCE_GXM_ATTRIBUTE_FORMAT_F32, 4, 0 } },
      { kVDU_Invalid }
   };

   CDynamicVertexBuffer vb( kRRTU_Render );

   memcpy( vb.Lock( sizeof( vertices ) ), vertices, sizeof( vertices ) );
   vb.Unlock();
   
   NVTAState::SetAttributes( skAttributes );
   NVTAState::SetupOneLowLevelStream( NVTAState::SLowLevelStream( vb.mpMemory_RT->mpAddress, sizeof( SVertex ) ) );

   pThis->WrapGxmDraw( SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, index_format_to_gxm( NVTAState::GetLastIndexBufferType() ), pThis->mpLastIndexBuffer, pThis->mLastIndexBufferCount );
}

void CBaseRenderBackend::RenderQuad(real32 minX, real32 const maxX, real32 const minY, real32 const maxY, real32 const z, real32 const minU, real32 const maxU, real32 const minV, real32 const maxV)
{
   CRenderBackend *pThis = static_cast<CRenderBackend *>(this);

   pThis->SetIndexData_OneToOne( 4 );

   struct SVertex
   { 
      real32 x, y, z;
      real32 u, v;
   };

   SVertex const vertices[] =
   {
      { minX, maxY, z, minU, minV },
      { maxX, maxY, z, maxU, minV },
      { minX, minY, z, minU, maxV },
      { maxX, minY, z, maxU, maxV }
   };

   static NVTAState::SAttributeDesc const skAttributes[] = 
   {
      { kVDU_Position, (SceGxmVertexAttribute) { 0, 0, SCE_GXM_ATTRIBUTE_FORMAT_F32, 3, 0 } },
      { kVDU_TexCoord0, (SceGxmVertexAttribute) { 0, 3 * sizeof(float), SCE_GXM_ATTRIBUTE_FORMAT_F32, 2, 0 } },
      { kVDU_Invalid }
   };

   CDynamicVertexBuffer vb( kRRTU_Render );

   memcpy( vb.Lock( sizeof( vertices ) ), vertices, sizeof( vertices ) );
   vb.Unlock();
   
   NVTAState::SetAttributes( skAttributes );
   NVTAState::SetupOneLowLevelStream( NVTAState::SLowLevelStream( vb.mpMemory_RT->mpAddress, sizeof( SVertex ) ) );

   pThis->WrapGxmDraw( SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, index_format_to_gxm( NVTAState::GetLastIndexBufferType() ), pThis->mpLastIndexBuffer, pThis->mLastIndexBufferCount );
}

//----------------------------------------------------------------------------

CRenderHWAllocator::SHandle const * CRenderBackend::AllocFixed(int const size, int const alignment, ERenderMemory const memoryType, ERenderMemory const fallbackMemory)
{
   CRenderHWAllocator::SHandle const * handle = NULL;

   handle = mpRenderAllocators[ memoryType ]->Alloc( size, alignment );
   if ( handle == NULL && fallbackMemory != kRM_Invalid )
   {
      handle = mpRenderAllocators[ fallbackMemory ]->Alloc( size, alignment );
   }

   if( handle == NULL )
   {
      bpe_debugger_printf("Failed to allocate %d bytes (alignment: %d) in AllocFixed\n", size, alignment);
   }

   return handle;
}

//----------------------------------------------------------------------------

void CRenderBackend::Free(CRenderHWAllocator::SHandle const * pHandle)
{
   BPE_ASSERT(pHandle != NULL, "Invalid handle passed to free");

   int const framesToKeep = DISPLAY_BUFFER_COUNT;//BPE_ARRAY_SIZE(mDisplayBufferInfo);

   uint32 const allocatorIndex = pHandle->mUserFlags;
   BPE_VERIFY( allocatorIndex < BPE_ARRAY_SIZE( mpRenderAllocators ), false, "Handle user flags had a bad allocator index" );

   mpRenderAllocators[ allocatorIndex ]->PendingFree( pHandle, framesToKeep );
}

//----------------------------------------------------------------------------

void CRenderBackend::FreeImmediate( CRenderHWAllocator::SHandle const *pHandle )
{
   BPE_ASSERT(pHandle != NULL, "Invalid handle passed to free");

   uint32 const allocatorIndex = pHandle->mUserFlags;
   BPE_VERIFY( allocatorIndex < BPE_ARRAY_SIZE( mpRenderAllocators ), false, "Handle user flags had a bad allocator index" );

   mpRenderAllocators[ allocatorIndex ]->ImmediateFree( pHandle );
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetProjectionMatrix(CMatrix4 const &matrix ) 
{ 
   static_cast<CRenderBackend *>(this)->mProjectionMatrix = matrix;
   static_cast<CRenderBackend *>(this)->FlushProjectionTimesViewMatrix();
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetCameraMatrix(CMatrix34 const &matrix)
{
   CRenderBackend *pThis = static_cast<CRenderBackend *>(this);

   pThis->mCameraMatrix = CMatrix4::FromMatrix34(matrix);
   // Might be able to speed this up a bit!
   pThis->mViewMatrix = CMatrix4::Scale(CVector3(1, 1, -1)) * pThis->mCameraMatrix.Inverse();

   pThis->FlushProjectionTimesViewMatrix();
}


//----------------------------------------------------------------------------

void CBaseRenderBackend::SetPerspectiveProjection(CAngle const & fov, 
                                                  real32 const aspect, 
                                                  real32 const minClip, 
                                                  real32 const maxClip )
{
   mFrustumNear = minClip;
   mFrustumFar = maxClip;

   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(
      CMatrix4::Translation(CVector3(mViewportJitterX, mViewportJitterY, 0.0f)) * CMatrix4::Perspective(fov, aspect, minClip, maxClip));
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetOrthographicProjection(real32 const width,
                                                   real32 const height,
                                                   real32 const zNear,
                                                   real32 const zFar)
{
   mFrustumNear = zNear;
   mFrustumFar = zFar;

   real32 const realWidth = (width < 0.0f) ? mRenderTargetWidth : width;
   real32 const realHeight = (height < 0.0f) ? mRenderTargetHeight : height;

   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::Orthographic(realWidth, realHeight, zNear, zFar));

}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetScreenSpaceOrtho()
{
   // 1-1 pixel mapping with 0,0 at screen top left
   real32 const offsetX = -1.0f;
   real32 const offsetY = 1.0f;
   real32 const nearZ = -1.0f;
   real32 const farZ = 1.0f;
   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::OrthographicOffset(real32(GetViewWidth()), 
      real32(-GetViewHeight()),
      offsetX,
      offsetY,
      nearZ, farZ));

   SetCameraMatrix(CMatrix34::Identity());
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetUniformOrtho()
{
   real32 const width = 1.0f;
   real32 const height = -1.0f;
   real32 const offsetX = -1.0f;
   real32 const offsetY = 1.0f;
   real32 const nearZ = -1.0f;
   real32 const farZ = 1.0f;
   static_cast<CRenderBackend *>(this)->SetProjectionMatrix(CMatrix4::OrthographicOffset(width, 
      height,
      offsetX,
      offsetY,
      nearZ, farZ));

   SetCameraMatrix(CMatrix34::Identity());
}

//----------------------------------------------------------------------------

void CRenderBackend::FlushProjectionTimesViewMatrix()
{
   mProjectionTimesViewMatrix = mProjectionMatrix * mViewMatrix;
}

//----------------------------------------------------------------------------

void CGPUProfileMarker::PushMarker(const char* pName)
{
   BPE_CHECK_SCE( sceGxmPushUserMarker( gpRenderBackend->Context(), pName ) );
}

void CGPUProfileMarker::PopMarker()
{
   sceGxmPopUserMarker( gpRenderBackend->Context() );
}

//----------------------------------------------------------------------------

CRenderBackend::EInitializeDisplayResult CBaseRenderBackend::InitializeDisplay(EStereoMode const stereoMode)
{
   return kIDR_Succeded;
}


//----------------------------------------------------------------------------

void CRenderBackend::WrapGxmDraw( CMeshChunk::EPrimitive primitive, SceGxmIndexFormat indexType, const void *indexData, uint32_t indexCount )
{
   SceGxmPrimitiveType primType;

   if ( primitive == CMeshChunk::kPrimitive_LineStrip )
   {
      if ( indexCount == 2 )
      {
         primType = SCE_GXM_PRIMITIVE_LINES;
      }
      else
      {
         // Line strips unsupported!
         return;
      }
   }
   else
   {
      primType = prim_type_to_gxm( primitive );
   }

   if (!indexCount)
      return;

   WrapGxmDraw( primType, indexType, indexData, indexCount );
}

#if BP_ENABLE_TESTTRI
extern "C" const void *BP_Debug_EvalTestTri(int, const void *, uint32_t *);
#endif

namespace NVitaPriorDraw
{
   SceGxmPrimitiveType sPrimType;
   SceGxmIndexFormat sIndexType;
   void const *sIndexData;
   uint32_t sIndexCount;
};

void CRenderBackend::WrapGxmDraw( SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, uint32_t indexCount )
{
   SceGxmPolygonMode polygonMode = primitivetype_to_polygonmode(primType);
   if( polygonMode != (SceGxmPolygonMode)-1 )
   {
      NVTAState::PolySetMode( polygonMode );

#if BP_ENABLE_TESTTRI
      if (primType == SCE_GXM_PRIMITIVE_TRIANGLES)
      {
         indexData = BP_Debug_EvalTestTri(indexType == SCE_GXM_INDEX_FORMAT_U32, indexData, &indexCount);
      }
      
      if (indexData)
      {
         FlushDrawState();

         BPE_CHECK_SCE( sceGxmDraw( Context(), primType, indexType, indexData, indexCount ) );
      }
#else
      FlushDrawState();

      sceGxmDraw( Context(), primType, indexType, indexData, indexCount );
      NVitaPriorDraw::sPrimType = primType;
      NVitaPriorDraw::sIndexType = indexType;
      NVitaPriorDraw::sIndexData = indexData;
      NVitaPriorDraw::sIndexCount = indexCount;
#endif
      ++mCurrentSceneDrawNum;
   }
}

void CRenderBackend::VitaRepeatPriorDraw()
{
   sceGxmDraw( Context(), NVitaPriorDraw::sPrimType, NVitaPriorDraw::sIndexType, NVitaPriorDraw::sIndexData, NVitaPriorDraw::sIndexCount );
}

void CRenderBackend::WrapGxmDrawInstanced( CMeshChunk::EPrimitive primitive, SceGxmIndexFormat indexType, const void *indexData, uint32_t indexCount, uint32_t indexWrap)
{
   SceGxmPrimitiveType primType;

   if ( primitive == CMeshChunk::kPrimitive_LineStrip )
   {
      if ( indexCount == 2 )
      {
         primType = SCE_GXM_PRIMITIVE_LINES;
      }
      else
      {
         // Line strips unsupported!
         return;
      }
   }
   else
   {
      primType = prim_type_to_gxm( primitive );
   }

   WrapGxmDrawInstanced( primType, indexType, indexData, indexCount, indexWrap);
}

void CRenderBackend::WrapGxmDrawInstanced( SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void *indexData, uint32_t indexCount, uint32_t indexWrap)
{
   SceGxmPolygonMode polygonMode = primitivetype_to_polygonmode(primType);
   if( polygonMode != (SceGxmPolygonMode)-1 )
   {
      NVTAState::PolySetMode( polygonMode );

      FlushDrawState();

      BPE_CHECK_SCE( sceGxmDrawInstanced( Context(), primType, indexType, indexData, indexCount, indexWrap ) );
      ++mCurrentSceneDrawNum;
   }
}

namespace
{
   SceUID gEventFlagId = -1;
   SceUID gThreadId = -1;
   SceUID gNoteThreadId = -1;

   CRenderBackend::TFnRenderThread  gRenderThreadFunc = NULL;  
   int                              gRenderThreadParam;
   volatile int                     gRenderThreadAlive = 1;

   enum EEventFlag
   {
      kEF_IsReady = (1 << 0),
      kEF_IsDone  = (1 << 1)
   };
}

#ifdef ENABLE_DETAILED_TIMING
static SceInt32 bp_note_thread(SceSize argSize, void *pArgBlock)
{
   int i;

   while(gRenderThreadAlive)
   {
      for (i = 0; i < CRenderBackend::DISPLAY_BUFFER_COUNT; i++)
      {
         if (*gpRenderBackend->s_dispNote[i].address != gpRenderBackend->s_dispFrame[i])
         {
            gpRenderBackend->s_dispFrame[i] = *gpRenderBackend->s_dispNote[i].address;
            gpRenderBackend->s_dispElapsed[i] = gpRenderBackend->s_dispTimer[i].GetElapsedTime() * 1000.0f;
         }
      }
      sceGxmWaitEvent();
   }
}
#endif

static SceInt32 bp_render_thread(SceSize argSize, void *pArgBlock)
{
   while(gRenderThreadAlive)
   {
      SceUInt32 result;
      SceUInt32 errorValue;
      SceUInt32 timeout_inout = 65 * 1000; // can't be static or const since this is inout
      // pass NULL to never time out - temporary while we only have a single render target
      gpRenderBackend->RenderStartIdle();
      errorValue = sceKernelWaitEventFlag( gEventFlagId, kEF_IsReady, SCE_KERNEL_EVF_WAITMODE_AND | SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL, &result, NULL /* &timeout_inout */ );
      gpRenderBackend->RenderEndIdle();

      CSyncCriticalSectionLocker lock( gpRenderBackend->mCommandBufferCriticalSection );
      
      if ( errorValue == SCE_KERNEL_ERROR_WAIT_TIMEOUT )
      {
         if ( gRenderThreadFunc )
         {
            gRenderThreadFunc(-1);
         }
      }
      else
      {
         BPE_CHECK_SCE( errorValue );

         if ( gRenderThreadFunc )
         {
            uint64 const frameTimeBegin = sceKernelGetProcessTimeWide();

            gRenderThreadFunc(gRenderThreadParam);

            gpRenderBackend->mThreadLastCPUTime = sceKernelGetProcessTimeWide() - frameTimeBegin;
         }
         BPE_CHECK_SCE( sceKernelSetEventFlag( gEventFlagId, kEF_IsDone ) );
      }
   }

   BPE_CHECK_SCE( sceKernelSetEventFlag( gEventFlagId, kEF_IsDone ) );
   return 0;
//   sceKernelExitThread( 0 );
}


//----------------------------------------------------------------------------

void CRenderBackend::StartThreadSystem( TFnRenderThread renderFunction )
{
   gRenderThreadFunc = renderFunction;
   gEventFlagId = BPE_CHECK_SCE( sceKernelCreateEventFlag( "BPE Render Event", 
      SCE_KERNEL_EVF_ATTR_MULTI | SCE_KERNEL_EVF_ATTR_TH_FIFO, kEF_IsDone, NULL ) );

   gThreadId = BPE_CHECK_SCE( sceKernelCreateThread( "BPE Render Thread", bp_render_thread, 
      NVtaThreadPriorities::kPriorityRenderer, 32 * 1024, 0, SCE_KERNEL_CPU_MASK_USER_ALL, NULL ) );

   sceKernelStartThread( gThreadId, 0, NULL );

#ifdef ENABLE_DETAILED_TIMING
   gNoteThreadId = BPE_CHECK_SCE( sceKernelCreateThread( "BPE GPU Tracker Thread", bp_note_thread,
                                                        NVtaThreadPriorities::kPriorityRenderer-1, 4096, 0, SCE_KERNEL_CPU_MASK_USER_ALL, NULL ) );
   sceKernelStartThread( gNoteThreadId, 0, NULL );
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::ShutdownThreadSystem()
{
   gRenderThreadAlive = 0;

   SceInt32 exitCode;
   sceKernelWaitThreadEnd( gRenderThreadAlive, &exitCode, NULL );
}

//----------------------------------------------------------------------------

void CRenderBackend::RunThreadFunction(int nextParam)
{
   WaitThreadFunction();

#if ENABLE_SEPERATE_RENDER_THREAD
   BeginFrameResourceTick();
#endif

   gRenderThreadParam = nextParam;
   
   // We have to flag "not done" now, as the render thread may not wake up fast enough from Ready to clear out done.
   sceKernelClearEventFlag( gEventFlagId, ~( kEF_IsDone ) );
   sceKernelSetEventFlag( gEventFlagId, kEF_IsReady );
}

void CRenderBackend::WaitThreadFunction()
{
   mGameWaitTimer.Reset();
   sceKernelWaitEventFlag( gEventFlagId, kEF_IsDone, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL );
   mGameElapsed += mGameWaitTimer.GetElapsedTime() * 1000.0f;
}

bool CRenderBackend::TryWaitThreadFunction()
{
   SceUInt32 waitTime = 250;

   mGameWaitTimer.Reset();
   SceInt32 ret = sceKernelWaitEventFlag(gEventFlagId, kEF_IsDone, SCE_KERNEL_EVF_WAITMODE_AND, NULL, &waitTime);
   mGameElapsed += mGameWaitTimer.GetElapsedTime() * 1000.0f;
   return (ret == SCE_OK) ? true : false;
}

void CRenderBackend::UpdateCommonDialog()
{
   SceCommonDialogUpdateParam updateParam;

   updateParam.renderTarget.colorFormat = SCE_GXM_COLOR_FORMAT_A8B8G8R8;
   updateParam.renderTarget.surfaceType = SCE_GXM_COLOR_SURFACE_LINEAR;
   updateParam.renderTarget.colorSurfaceData = s_dispBuf[s_dispBack];
   updateParam.renderTarget.depthSurfaceData = NULL;
   updateParam.renderTarget.width = skDisplayWidth;
   updateParam.renderTarget.height = skDisplayHeight;
   updateParam.renderTarget.strideInPixels = DISPLAY_STRIDE;
   memset(updateParam.renderTarget.reserved, 0, sizeof(updateParam.renderTarget.reserved));
   updateParam.displaySyncObject = s_dispSync[s_dispBack];

   sceCommonDialogUpdate(&updateParam);
}

//----------------------------------------------------------------------------

void CRenderBackend::AcquireRenderThreadOwnership()
{
   mCommandBufferCriticalSection.Enter();
}

//----------------------------------------------------------------------------

void CRenderBackend::ReleaseRenderThreadOwnership()
{
   mCommandBufferCriticalSection.Leave();
}

//----------------------------------------------------------------------------

void CRenderBackend::UpdateVBLCount()
{
   uint16 cntVblCountU16 = sceDisplayGetVcount();
   uint16 oldVblCountU16 = mVBLCount & 0xFFFF;

   // Handle 16 -> 32 bit wrap case
   if ( oldVblCountU16 > cntVblCountU16 )
   {
      mVBLCount += 0x10000;
   }

   mVBLCount = ( mVBLCount & 0xFFFF0000 ) | uint32( cntVblCountU16 );
}

//----------------------------------------------------------------------------

void CRenderBackend::SetUnsafeVertexUniformBuffer( int const bufferIndex, void const *ptr )
{
   BPE_ASSERT( ( (unsigned) ptr & 0xF0000000 ) == 0x70000000 || ( (unsigned) ptr & 0xF0000000 ) == 0x80000000, "Bad ptr" );
   BPE_CHECK_SCE( sceGxmSetVertexUniformBuffer( Context(), bufferIndex, ptr ) );
}

//----------------------------------------------------------------------------

void CRenderBackend::SetStaticVertexUniformBuffer( int const bufferIndex, void const *ptr )
{
   BPE_ASSERT( ( (unsigned) ptr & 0xF0000000 ) == 0x70000000 || ( (unsigned) ptr & 0xF0000000 ) == 0x80000000, "Bad ptr" );
   BPE_CHECK_SCE( sceGxmSetVertexUniformBuffer( Context(), bufferIndex, ptr ) );
}

//----------------------------------------------------------------------------

void *CRenderBackend::AllocOneFrameVertexUniformBuffer( int const bufferIndex, size_t const size )
{
   void *pMem = GetVertexBufferPool_RT()->AllocChunk( size )->Lock();

   BPE_CHECK_SCE( sceGxmSetVertexUniformBuffer( Context(), bufferIndex, pMem ) );

   return pMem;
}

//----------------------------------------------------------------------------

void *CRenderBackend::AllocOneFrameFragmentUniformBuffer( int const bufferIndex, size_t const size )
{
   void *pMem = GetVertexBufferPool_RT()->AllocChunk( size )->Lock();

   BPE_CHECK_SCE( sceGxmSetFragmentUniformBuffer( Context(), bufferIndex, pMem ) );

   return pMem;
}

//----------------------------------------------------------------------------

void CRenderBackend::GetMemoryStats(ERenderMemory const memory, SRenderHWAllocatorStats * pStats) const
{
   mpRenderAllocators[ memory ]->GetStats( pStats );
}

//----------------------------------------------------------------------------

bool CRenderBackend::VitaReadGPUCanDo60FPS()
{
   for ( int i = 0; i < DISPLAY_BUFFER_COUNT; ++i )
   {
      if ( s_dispDisplayHardwareAt60[i] == 0 )
      {
         return false;
      }
   }

   return true;
}
