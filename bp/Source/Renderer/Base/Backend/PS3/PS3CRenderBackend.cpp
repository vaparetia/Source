//----------------------------------------------------------------------------
// PS3CRenderBackend.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

#include "Engine/Input/CInputGenerator.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CSPURSManager.h"

#include "Renderer/Base/Backend/PS3/PS3CTexture.h"
#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Material/PS3/PS3CCompiledShaderCache.h"
#include "Renderer/Base/Material/Shaders/CDefaultShader.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"

#include "Renderer/Renderer_SPU/NRenderQueue.h"
#include "../shared/BP_BaseRenderer.h"

#include "sysutil/sysutil_sysparam.h"
#include "cell/spurs.h"
#include "sysutil/sysutil_screenshot.h"

#define RENDERBACKEND_SUPPORTS_ALPHA_TEST() 1

//----------------------------------------------------------------------------

//#define DEBUG_FIFO_HANGS

//#define FLUSH_AFTER_RENDERCALLS

#define BP_RENDER_GEOMETRY    1

//#define BP_ALLOW_FAKE_3D_RESOLUTION    1

//#define BP_ENABLE_FRAME_ALLOCATOR      1

//----------------------------------------------------------------------------

using namespace cell::Gcm;

//----------------------------------------------------------------------------

namespace
{
   int const kDefaultCommandBufferSize = 2 * 1024 * 1024;
   
   // Main (PPU) memory available for static and dynamic RSX allocations.
   int const kHostMemorySize = 54 * 1024 * 1024;

   // GCM steals the first "kDefaultCommandBufferSize" bytes from this buffer for it's command buffer.
   int const kMainMemoryPoolSize = kHostMemorySize - kDefaultCommandBufferSize;
   int const kMainMemoryHandleCount = 16384; // Increased handle count to allow for very large amount of dynamic vertex buffer allocations (per character) for SotC side-by-side credits scroller.

   int const kLocalMemoryHandleCount = 8192;

   // Bytes per pixel for color back buffer
   int const kColorBufferBPP = 4;

   int const kFragmentRenderQueueDepth = 2048;
   int const kFragmentRenderQueueEntrySize = sizeof(NRenderQueue::SRenderCmd);
   int const kFragmentRenderQueueSize = kFragmentRenderQueueDepth * kFragmentRenderQueueEntrySize;

   int const kFragmentProgramOverfetchAmount = 400;
   int const kFragmentProgramOverfetchAmount128 = (400 + 127) & ~127;
   int const kFragmentProgramRingBufferSize = 512 * 1024;
};

//----------------------------------------------------------------------------

namespace
{
   const uint32 gPrimitiveMapping[] =
   {
      CELL_GCM_PRIMITIVE_TRIANGLES,
      CELL_GCM_PRIMITIVE_TRIANGLE_STRIP,
      CELL_GCM_PRIMITIVE_TRIANGLE_FAN,
      CELL_GCM_PRIMITIVE_LINES,
      CELL_GCM_PRIMITIVE_LINE_STRIP,
      CELL_GCM_PRIMITIVE_POINTS
   };

   BPE_CTASSERT(BPE_ARRAY_SIZE(gPrimitiveMapping) == CMeshChunk::kPrimitive_Count);

   BPE_CTASSERT((int)kIT_Uint32 == (int)CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32);
   BPE_CTASSERT((int)kIT_Uint16 == (int)CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16);
}

//----------------------------------------------------------------------------

extern "C" void BP_AddDebugString(const char * pString, ...);

#if BP_ENABLE_FRAME_ALLOCATOR

namespace FrameAllocator
{
   uint32   mBufferStart = 0;
   uint32   mBufferEnd = 0;
   uint32   mBufferCurrent = 0;
   uint32   mFrameAllocatorUsage = 0;

   void Init(int const bufferSize = 1536 * 1024)
   {
      mBufferStart = (uint32)memalign(16, bufferSize);
      mBufferEnd = mBufferStart + bufferSize;

      mBufferCurrent = mBufferStart;
   }

   void* Alloc(int const size)
   {
      int const alignedSize = (size + 15) & ~15;

      uint32 const current = mBufferCurrent;

      mBufferCurrent += alignedSize;

      if( mBufferCurrent <= mBufferEnd )
      {
         return (void*)current;
      }

      BPE_VERIFYA(false, "Frame allocator out of memory");
   }

   void Reset()
   {
      mFrameAllocatorUsage = mBufferCurrent - mBufferStart;

      mBufferCurrent = mBufferStart;
   }
}

uint32 BP_GetFrameAllocatorUsage()
{
   return FrameAllocator::mFrameAllocatorUsage;
}

#else

uint32 BP_GetFrameAllocatorUsage()
{
   return 0;
}

#endif

//----------------------------------------------------------------------------

extern char _binary_task_Renderer_SPU_elf_start[];

//----------------------------------------------------------------------------

void CGPUProfileMarker::PushMarker(const char * pName)
{
#ifdef USE_GPU_PROFILE_MARKERS
   cellGcmSetPerfMonPushMarker(pName);
#endif
}

//----------------------------------------------------------------------------

void CGPUProfileMarker::PopMarker()
{
#ifdef USE_GPU_PROFILE_MARKERS
   cellGcmSetPerfMonPopMarker();
#endif
}

//----------------------------------------------------------------------------

void CGPUProfileMarker::Event(const char * pName)
{
#ifdef USE_GPU_PROFILE_MARKERS
   cellGcmSetPerfMonMarker(pName);
#endif
}

//----------------------------------------------------------------------------

CFragmentProgramAllocator::CFragmentProgramAllocator()
:  mpMemory(RenderBackend()->AllocFixed(kFragmentProgramRingBufferSize, 128, kRM_Video))
,  mCurrentPtr((uint32)mpMemory->mpAddress + kFragmentProgramRingBufferSize - kFragmentProgramOverfetchAmount128)
,  mRsxProcessedPtrLabel(RenderBackend()->AllocateRSXLabel())
{
   mpRsxProcessedPtrLabelAddress = cellGcmGetLabelAddress(mRsxProcessedPtrLabel);
   mCachedRsxProcessLabelValue = *mpRsxProcessedPtrLabelAddress = mCurrentPtr;

   cellGcmAddressToOffset(mpMemory->mpAddress, (uint32_t*)&mMemoryBaseOffset);
}

//----------------------------------------------------------------------------

CFragmentProgramAllocator::~CFragmentProgramAllocator()
{
   RenderBackend()->Free(mpMemory);
}

//----------------------------------------------------------------------------

//#define FRAGMENT_PROGRAM_ALLOCATE_MEASURE

void CFragmentProgramAllocator::Allocate(int const size, uint8** pAllocationEA, uint32* pAllocationOffset)
{
   int const alignedSize = (size + 127) & ~127;
   BPE_ASSERT(alignedSize*2 < kFragmentProgramRingBufferSize, "Ring buffer not big enough, must at least by twice as big as biggest fragment program, otherwise deadlock can occur");

   uint32 const start = uint32(mpMemory->mpAddress);
   uint32 const end = start + kFragmentProgramRingBufferSize;
   uint32 current = mCurrentPtr;

   int const currentMinusSize = current - alignedSize;
   int const endMinusSize = end - (alignedSize + kFragmentProgramOverfetchAmount128);

#ifdef FRAGMENT_PROGRAM_ALLOCATE_MEASURE
   CStopWatch watch;
#endif

   bool didStall = false;
   bool keepTryingToAllocate = false;

   int stage = 0;

   do
   {
      uint32 rsxProcessedLabelValue = 0;
      
      switch(stage)
      {
      case 0:
         rsxProcessedLabelValue = mCachedRsxProcessLabelValue;
         stage = 1;
         break;
      
      case 1:
         keepTryingToAllocate = false;
         rsxProcessedLabelValue = *mpRsxProcessedPtrLabelAddress;
         mCachedRsxProcessLabelValue = rsxProcessedLabelValue;
         break;
      }

      // Check if fragment program fits into current free buffer.
      if( BPE_BRANCH_HINT_TRUE( ( (rsxProcessedLabelValue >= current) || (currentMinusSize > rsxProcessedLabelValue) ) && ( (currentMinusSize >= start) || (endMinusSize > rsxProcessedLabelValue) ) ) )
      {
         // update current ptr
         if( BPE_BRANCH_HINT_TRUE(currentMinusSize >= start) )
         {
            int const inUseSize = rsxProcessedLabelValue - currentMinusSize;
#ifdef FRAGMENT_PROGRAM_ALLOCATE_MEASURE
            RenderBackend()->Metrics().mFragmentProgramRingBufferPeakSize = bpe::max_val(RenderBackend()->Metrics().mFragmentProgramRingBufferPeakSize, inUseSize);
#endif

            current = currentMinusSize;
         }
         else
         {
            int const inUseSize = (end - endMinusSize) + (rsxProcessedLabelValue - start);
#ifdef FRAGMENT_PROGRAM_ALLOCATE_MEASURE
            RenderBackend()->Metrics().mFragmentProgramRingBufferPeakSize = bpe::max_val(RenderBackend()->Metrics().mFragmentProgramRingBufferPeakSize, inUseSize);
#endif

            current = endMinusSize;
         }

         *pAllocationEA = (uint8*)current;
         *pAllocationOffset = mMemoryBaseOffset + (current - uint32(mpMemory->mpAddress));
      }
      else
      {
         keepTryingToAllocate = true;
         
         // Only do flush and sleep if we have checked the video memory address.
         if( stage == 1 )
         {
            // Flush FIFO so any proceeding "free" write texture label commands will be processed.
            cellGcmFlush();

            sys_timer_usleep(5);

            didStall = true;
         }
      }
   } 
   while( BPE_BRANCH_HINT_FALSE(keepTryingToAllocate) );

#ifdef FRAGMENT_PROGRAM_ALLOCATE_MEASURE
   if( didStall )
      RenderBackend()->Metrics().mFragmentProgramRingBufferStallUS += watch.GetElapsedMicroSeconds();
#endif

   mCurrentPtr = current;
}

bool FindFrameBufferSize(int const width, int const height, CRenderBackend::SRenderFrameBufferSize::EDisplayAspect const aspect, CRenderBackend::SRenderInitialization const & initFlags, CRenderBackend::SRenderFrameBufferSize* pFoundResolution)
{

   for( int i = 0; i < initFlags.mFrameBufferSizeCount; ++i )
   {
      CRenderBackend::SRenderFrameBufferSize const & frameBufferSize = initFlags.mpFrameBufferSize[i];

      if( frameBufferSize.GetDisplayAspect() == aspect &&
         frameBufferSize.GetWidth(CRenderBackend::SRenderFrameBufferSize::kST_Display) == width &&
         frameBufferSize.GetHeight(CRenderBackend::SRenderFrameBufferSize::kST_Display) == height )
      {
         memcpy(pFoundResolution, &frameBufferSize, sizeof(CRenderBackend::SRenderFrameBufferSize));
         return true;
      }
   }

   return false;
}

//----------------------------------------------------------------------------

#if BPE_USE_EDGE_GEOM
#include "edge/geom/edgegeom_structs.h"
#include "Job_EdgeGeom_SPU/job_geom_interface.h"

namespace Edge
{
   int const kEdgeSPUCount          = 3;
   int const kEdgeSharedBufferSize  = 1 * 1024 * 1024;
   int const kEdgeRingBufferSize    = 512 * 1024;

   CRenderHWAllocator::SHandle const * sharedBufferMemory;
   CRenderHWAllocator::SHandle const * ringBufferMemory;

   EdgeGeomOutputBufferInfo outputBuffer;

   int                           gEdgeGeomSubmittedCount = 0;
   EdgeGeomViewportInfo*         pCurrentViewport = NULL;
   EdgeGeomLocalToWorldMatrix*   pCurrentLocalToWorld = NULL;

   int const kQueueSize = 1024;

   typedef cell::Spurs::JobQueue::JobQueue<kQueueSize> TJobQueue;

   int                           kGeomJobTag = 0;

   TJobQueue*                    pJobQueue = NULL;
   cell::Spurs::JobQueue::Port2* pPort = NULL;

   EdgeGeomViewportInfo*         GetWriteableViewport()
   {
      // Force getting new copies every time this is requested!
      gEdgeGeomSubmittedCount = 1;

      if( pCurrentViewport )
      {
         if( gEdgeGeomSubmittedCount )
         {
            EdgeGeomViewportInfo* pOldViewport = pCurrentViewport;
            EdgeGeomViewportInfo* pNewViewport = (EdgeGeomViewportInfo*)FrameAllocator::Alloc(sizeof(EdgeGeomViewportInfo));

            memcpy(pNewViewport, pOldViewport, sizeof(EdgeGeomViewportInfo));

            pCurrentViewport = pNewViewport;
            gEdgeGeomSubmittedCount = 0;
         }
         
         return pCurrentViewport;
      }
      else
      {
         pCurrentViewport = (EdgeGeomViewportInfo*)FrameAllocator::Alloc(sizeof(EdgeGeomViewportInfo));
         memset(pCurrentViewport, 0, sizeof(EdgeGeomViewportInfo));
    
         return pCurrentViewport;
      }
   }
}

uint32 BP_GetEdgeSharedBufferFailedAllocSize()
{
   uint32 const failedSize = Edge::outputBuffer.sharedInfo.failedAllocSize;
   Edge::outputBuffer.sharedInfo.failedAllocSize = 0;

   return failedSize;
}

extern const CellSpursJobHeader _binary_job_Job_EdgeGeom_SPU_jobbin2_jobheader;

void CreateGeomJob(JobGeom256* job, CellGcmContextData *ctx, EdgeGeomPpuConfigInfo const *info, void *skinningMatrices)
{
   // Steam descriptor
   job->inputDmaList[kJobGeomDmaOutputStreamDesc].eal  = (uintptr_t)info->spuOutputStreamDesc;
   job->inputDmaList[kJobGeomDmaOutputStreamDesc].size = info->spuOutputStreamDescSize;

   // Indexes
   job->inputDmaList[kJobGeomDmaIndexes_0].eal  = (uintptr_t)info->indexes;
   job->inputDmaList[kJobGeomDmaIndexes_0].size = info->indexesSizes[0];
   job->inputDmaList[kJobGeomDmaIndexes_1].eal  = job->inputDmaList[kJobGeomDmaIndexes_0].eal + job->inputDmaList[kJobGeomDmaIndexes_0].size;
   job->inputDmaList[kJobGeomDmaIndexes_1].size = info->indexesSizes[1];

   // Skinning Matrices
   job->inputDmaList[kJobGeomDmaSkinMatrices_0].eal  = info->skinMatricesByteOffsets[0] + (uintptr_t)skinningMatrices;
   job->inputDmaList[kJobGeomDmaSkinMatrices_0].size = info->skinMatricesSizes[0];
   job->inputDmaList[kJobGeomDmaSkinMatrices_1].eal  = info->skinMatricesByteOffsets[1] + (uintptr_t)skinningMatrices;
   job->inputDmaList[kJobGeomDmaSkinMatrices_1].size = info->skinMatricesSizes[1];

   // Skinning Indexes & Weights
   job->inputDmaList[kJobGeomDmaSkinIndexesAndWeights_0].eal  = (uintptr_t)info->skinIndexesAndWeights;
   job->inputDmaList[kJobGeomDmaSkinIndexesAndWeights_0].size = info->skinIndexesAndWeightsSizes[0]; 
   job->inputDmaList[kJobGeomDmaSkinIndexesAndWeights_1].eal  = job->inputDmaList[kJobGeomDmaSkinIndexesAndWeights_0].eal + job->inputDmaList[kJobGeomDmaSkinIndexesAndWeights_0].size;
   job->inputDmaList[kJobGeomDmaSkinIndexesAndWeights_1].size = info->skinIndexesAndWeightsSizes[1];

   // Vertexes (stream 1)
   job->inputDmaList[kJobGeomDmaVertexes1_0].eal  = (uintptr_t)info->spuVertexes[0];
   job->inputDmaList[kJobGeomDmaVertexes1_0].size = info->spuVertexesSizes[0];
   job->inputDmaList[kJobGeomDmaVertexes1_1].eal  = job->inputDmaList[kJobGeomDmaVertexes1_0].eal + job->inputDmaList[kJobGeomDmaVertexes1_0].size;
   job->inputDmaList[kJobGeomDmaVertexes1_1].size = info->spuVertexesSizes[1];
   job->inputDmaList[kJobGeomDmaVertexes1_2].eal  = job->inputDmaList[kJobGeomDmaVertexes1_1].eal + job->inputDmaList[kJobGeomDmaVertexes1_1].size;
   job->inputDmaList[kJobGeomDmaVertexes1_2].size = info->spuVertexesSizes[2];

   // Vertexes (stream 2)
   job->inputDmaList[kJobGeomDmaVertexes2_0].eal  = (uintptr_t)info->spuVertexes[1];
   job->inputDmaList[kJobGeomDmaVertexes2_0].size = info->spuVertexesSizes[3];
   job->inputDmaList[kJobGeomDmaVertexes2_1].eal  = job->inputDmaList[kJobGeomDmaVertexes2_0].eal + job->inputDmaList[kJobGeomDmaVertexes2_0].size;
   job->inputDmaList[kJobGeomDmaVertexes2_1].size = info->spuVertexesSizes[4];
   job->inputDmaList[kJobGeomDmaVertexes2_2].eal  = job->inputDmaList[kJobGeomDmaVertexes2_1].eal + job->inputDmaList[kJobGeomDmaVertexes2_1].size;
   job->inputDmaList[kJobGeomDmaVertexes2_2].size = info->spuVertexesSizes[5];

   // Triangle Culling Data
   job->inputDmaList[kJobGeomDmaViewportInfo].eal  = (uintptr_t)Edge::pCurrentViewport;
   job->inputDmaList[kJobGeomDmaViewportInfo].size = sizeof(EdgeGeomViewportInfo);
   job->inputDmaList[kJobGeomDmaLocalToWorldMatrix].eal  = (uintptr_t)Edge::pCurrentLocalToWorld;
   job->inputDmaList[kJobGeomDmaLocalToWorldMatrix].size = sizeof(EdgeGeomLocalToWorldMatrix);

   // SpuConfigInfo
   job->inputDmaList[kJobGeomDmaSpuConfigInfo].eal	 = (uintptr_t)info;
   job->inputDmaList[kJobGeomDmaSpuConfigInfo].size = sizeof(EdgeGeomSpuConfigInfo);

   // Fixed Point Attribute Offsets
   job->inputDmaList[kJobGeomDmaFixedPointOffsets1].eal  = (uintptr_t)info->fixedOffsets[0];	 
   job->inputDmaList[kJobGeomDmaFixedPointOffsets1].size = info->fixedOffsetsSize[0];
   job->inputDmaList[kJobGeomDmaFixedPointOffsets2].eal  =	(uintptr_t)info->fixedOffsets[1];
   job->inputDmaList[kJobGeomDmaFixedPointOffsets2].size = info->fixedOffsetsSize[1];

   // Input Streams Descriptors
   job->inputDmaList[kJobGeomDmaInputStreamDesc1].eal  = (uintptr_t)info->spuInputStreamDescs[0];
   job->inputDmaList[kJobGeomDmaInputStreamDesc1].size = info->spuInputStreamDescSizes[0];
   job->inputDmaList[kJobGeomDmaInputStreamDesc2].eal  = (uintptr_t)info->spuInputStreamDescs[1];
   job->inputDmaList[kJobGeomDmaInputStreamDesc2].size = info->spuInputStreamDescSizes[1];

   // --- User Data ---

   job->userData.eaOutputBufferInfo = (uintptr_t)&Edge::outputBuffer;

   // Command Buffer Hole
   {
      uint32_t holeSize = info->spuConfigInfo.commandBufferHoleSize << 4;

      // Reserve command buffer space
      if(ctx->current + holeSize/4 + 3 > ctx->end) 
      {
         if((*ctx->callback)(ctx, holeSize/4 + 3) != CELL_OK) 
            return;
      }

      // Pad with nops until we reach a 16 byte aligned address
      while(((uint32_t)ctx->current & 0xF) != 0) 
      {
         *ctx->current++ = 0;
      }

      // Insert JTS at beginning of hole and at every 128 byte boundary
      {
         uint32_t holeEa = (uint32_t)ctx->current;
         uint32_t holeEnd = holeEa + holeSize;
         uint32_t jumpOffset;
         cellGcmAddressToOffset(ctx->current, &jumpOffset);
         cellGcmSetJumpCommandUnsafeInline(ctx, jumpOffset);
         uint32_t nextJ2S = ((uint32_t)ctx->current + 0x80) & ~0x7F;

         while(nextJ2S < holeEnd) 
         {
            __dcbz( (void*)nextJ2S ); // prefetching here improves this loop's performance significantly
            ctx->current = (uint32_t*)nextJ2S;
            cellGcmAddressToOffset(ctx->current, &jumpOffset);
            cellGcmSetJumpCommandUnsafeInline(ctx, jumpOffset);
            nextJ2S = ((uint32_t)ctx->current + 0x80) & ~0x7F;
         }

         ctx->current = (uint32_t*)holeEnd;

         job->userData.eaCommandBufferHole = (uintptr_t)holeEa;
      }
   }

   // Blend shapes / shape count
   if(info->blendShapes) 
   {
      // This is where you would fill in actual data.
      job->userData.eaBlendShapeInfo = NULL;
      job->userData.blendShapeInfoCount = 0;
   }
   else
   {
      job->userData.eaBlendShapeInfo = NULL;
      job->userData.blendShapeInfoCount = 0;
   }

   memcpy(&job->header, &_binary_job_Job_EdgeGeom_SPU_jobbin2_jobheader, sizeof(CellSpursJobHeader));

   job->header.sizeDmaList = kJobGeomInputDmaEnd*8;
   job->header.eaHighInput = 0;
   job->header.useInOutBuffer = 1;
   job->header.sizeInOrInOut = info->ioBufferSize;
   //job->header.sizeOut = 0;
   job->header.sizeStack = 0;
   job->header.sizeScratch = info->scratchSizeInQwords;
   //job->header.eaHighCache = 0;
   job->header.sizeCacheDmaList = 0;
}

void CRenderBackend::InitializeEdge()
{
   using namespace Edge;

   sharedBufferMemory = AllocFixed(kEdgeSharedBufferSize, 128, kRM_System);
   ringBufferMemory = AllocFixed(kEdgeSPUCount * kEdgeRingBufferSize, 128, kRM_System);

   memset(&outputBuffer, 0, sizeof(outputBuffer));

   // Initialize shared buffer
   outputBuffer.sharedInfo.startEa = (uint32_t)sharedBufferMemory->mpAddress;
   outputBuffer.sharedInfo.endEa = outputBuffer.sharedInfo.startEa + kEdgeSharedBufferSize;
   outputBuffer.sharedInfo.currentEa = outputBuffer.sharedInfo.startEa;
   outputBuffer.sharedInfo.locationId = CELL_GCM_LOCATION_MAIN;
   cellGcmAddressToOffset((void*)outputBuffer.sharedInfo.startEa, &outputBuffer.sharedInfo.startOffset);

   // Initialize ring buffers
   for(int i = 0; i < kEdgeSPUCount; ++i )
   {
      outputBuffer.ringInfo[i].startEa = (uint32_t)ringBufferMemory->mpAddress + i*kEdgeRingBufferSize;
      outputBuffer.ringInfo[i].endEa = outputBuffer.ringInfo[i].startEa + kEdgeRingBufferSize;
      outputBuffer.ringInfo[i].currentEa = outputBuffer.ringInfo[i].startEa;
      outputBuffer.ringInfo[i].locationId = CELL_GCM_LOCATION_MAIN;

      volatile uint32_t* pRsxLabel = cellGcmGetLabelAddress(AllocateRSXLabel());
      *pRsxLabel = outputBuffer.ringInfo[i].endEa;

      outputBuffer.ringInfo[i].rsxLabelEa = (uint32_t)pRsxLabel;

      cellGcmAddressToOffset( (void*)outputBuffer.ringInfo[i].startEa, &outputBuffer.ringInfo[i].startOffset);
   }

   // Initialize job queue
   pJobQueue = (TJobQueue*)memalign(TJobQueue::kAlign, TJobQueue::kSize);

   uint8_t priorityTable[CELL_SPURS_MAX_SPU];
   
   for( int i = 0; i < CELL_SPURS_MAX_SPU; ++i )
      priorityTable[i] = (i < kEdgeSPUCount) ? 8 : 0;

   TJobQueue::create(pJobQueue, gpSpursManager->mpSPURS_Main.get(), "EDGE GEOM", kEdgeSPUCount /*numSpus*/, priorityTable);

   pPort = (cell::Spurs::JobQueue::Port2*)memalign(CELL_SPURS_JOBQUEUE_PORT2_ALIGN, CELL_SPURS_JOBQUEUE_PORT2_SIZE);
   cell::Spurs::JobQueue::Port2::create(pPort, pJobQueue);
}

void CRenderBackend::ShutdownEdge()
{
   using namespace Edge;

   pPort->destroy();

   free(pPort); 
   pPort = NULL;

   pJobQueue->shutdown();

   int exitCode;
   pJobQueue->join(&exitCode);

   free(pJobQueue);
   pJobQueue = NULL;

   Free(sharedBufferMemory);
   Free(ringBufferMemory);
}

CellSpursJobList* pJobList = NULL;
int jobListJobsRemaining = 0;

int gEdgeJobListSize = 1;

void CRenderBackend::BeginEdgeJobList(int segmentCountHint)
{
   using namespace Edge;

   uint32 const memory = (uint32)FrameAllocator::Alloc(sizeof(CellSpursJobList) + segmentCountHint * sizeof(JobGeom256));

   jobListJobsRemaining = segmentCountHint;

   pJobList = (CellSpursJobList*)memory;
   pJobList->numJobs = 0;
   pJobList->sizeOfJob = sizeof(JobGeom256);
   pJobList->eaJobList = (uint64_t)(memory + sizeof(CellSpursJobList));
}

void CRenderBackend::AddEdgeSegment(void* skinningMatrices, EdgeGeomPpuConfigInfo const * pSegmentList, int const segmentIdx)
{
   using namespace Edge;

   EdgeGeomPpuConfigInfo const * pSegment = pSegmentList + segmentIdx;

   // Bind the RSX-only vertex attributes for this job before it's created
   if (pSegment->rsxOnlyVertexesSize > 0)
   {
      EdgeGeomVertexStreamDescription *rsxOnlyStreamDesc = (EdgeGeomVertexStreamDescription*)(pSegment->rsxOnlyStreamDesc);
      
      uint32_t rsxVertexesOffset = 0;
      cellGcmAddressToOffset(pSegment->rsxOnlyVertexes, &rsxVertexesOffset);

      // Disable ALL attributes (Do not need to disable ATTR0 because that is position which is ALWAYS enabled)
      cellGcmReserveMethodSize(2*15);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(1, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(2, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(3, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(4, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(5, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(6, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(7, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(8, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(9, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(10, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(11, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(12, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(13, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(14, 0, 0, 0, CELL_GCM_VERTEX_F);
      cell::Gcm::UnsafeInline::cellGcmSetVertexDataArrayFormat(15, 0, 0, 0, CELL_GCM_VERTEX_F);

      // Only enable attributes that are actually used.
      for(uint32_t iRsxAttr=0; iRsxAttr < rsxOnlyStreamDesc->numAttributes; ++iRsxAttr)
      {
         EdgeGeomAttributeBlock &attr = rsxOnlyStreamDesc->blocks[iRsxAttr].attributeBlock;
         cellGcmSetVertexDataArray(attr.vertexProgramSlotIndex, 0, rsxOnlyStreamDesc->stride, attr.componentCount, attr.format, CELL_GCM_LOCATION_LOCAL, rsxVertexesOffset + attr.offset);
      }
   }

   // if we don't have a job list, create new one.
   if( !pJobList )
      BeginEdgeJobList(gEdgeJobListSize);

   JobGeom256* pJob = (JobGeom256*)pJobList->eaJobList;
   pJob += pJobList->numJobs;

   ++pJobList->numJobs;
   --jobListJobsRemaining;

   memset(pJob, 0, sizeof(JobGeom256));
   CreateGeomJob(pJob, gCellGcmCurrentContext, pSegment, skinningMatrices);

   // If current job list is full, send it off.
   if( !jobListJobsRemaining )
      SubmitCurrentEdgeJobList();

   gEdgeGeomSubmittedCount++;
}

void CRenderBackend::SubmitCurrentEdgeJobList()
{
   using namespace Edge;

   if( pJobList && pJobList->numJobs )
   {
      for(;;)
      {
         int result = pPort->pushJobList(pJobList, kGeomJobTag, cell::Spurs::JobQueue::Port2::kFlagNonBlocking | cell::Spurs::JobQueue::Port2::kFlagSyncJob);
         
         if( result == CELL_OK )
            break;

         if( result == CELL_SPURS_JOB_ERROR_AGAIN )
         {
            // At this point our edge job queue is full
            // One possible scenario for this is that edge jobs are not completing because they are waiting for ring buffer space 
            // which isn't being freed due to render commands which haven't been allowed to execute yet because the RSX put ptr hasn't been updated yet.

            // Flush FIFO so any preceeding render commands may execute and free edge ring buffer space.
            cellGcmFlush();

            // Wait a little bit to retry.
            sys_timer_usleep(5);
         }
      }
   }
   
   pJobList = NULL;
   jobListJobsRemaining = 0;
}

void CRenderBackend::WaitForEdgeCompletion()
{
   cellGcmFlush();

   // wait for geom jobs to finish
   Edge::pPort->sync(Edge::kGeomJobTag);
}

void CRenderBackend::SetEdgeProjectionMatrix(float* mtx)
{
   EdgeGeomViewportInfo* pViewport = Edge::GetWriteableViewport();
   memcpy(pViewport->viewProjectionMatrix, mtx, sizeof(real32) * 16);
}

void CRenderBackend::SetEdgeLocalToWorld(float* mtx)
{
   Edge::pCurrentLocalToWorld = (EdgeGeomLocalToWorldMatrix*)FrameAllocator::Alloc(sizeof(EdgeGeomLocalToWorldMatrix));
   memcpy(Edge::pCurrentLocalToWorld, mtx, sizeof(real32) * 12);
}

#endif

//----------------------------------------------------------------------------

CRenderBackend::CRenderBackend(IResourcePool & resourcePool,
                               SRenderInitialization const & initFlags)

:  CBaseRenderBackend(resourcePool, initFlags)
,  mCurrentRenderTarget()
,  mCurrentRenderTargetAAType(CBaseTexture::kAA_None)
,  mRenderTargetsNeedsRebind(true)
,  mBoundIndexBuffer(0)
,  mBoundIndexBufferLocation(0)
,  mBoundIndexBufferType(0)
,  mBoundVertexDataHash_1(0)
,  mBoundVertexDataHash_2(0)
,  mDisabledVertexAttributeMask(0)
,  mpHostMemory(NULL)
,  mCurrentDisplayBuffer(0)
,  mRsxLabel(64)
,  mFogEnabled( false )
,  mFogColor( CColor::Black() )
,  mCameraMatrix(CMatrix4::Identity())
,  mViewMatrix(CMatrix4::Identity())
,  mProjectionTimesViewMatrix( CMatrix4::Identity() )
,  mBootupResolutionId(-1)
,  mDisplayIsInitialized(false)
,  mVBLCount(0)
{
   mWaitLabel = AllocateRSXLabel();
   mWaitLabelValue = 1;

#if BP_ENABLE_FRAME_ALLOCATOR
   FrameAllocator::Init();
#endif

   InitializeSPU();

   // Allocator 1mb page aligned buffer for host memory
   mpHostMemory = memalign(1024*1024, kHostMemorySize);
   BPE_VERIFY(mpHostMemory != NULL, false, "Couldn't allocate memory for renderer");

   // Initialize GCM, this will automatically map the buffer that we pass in for the RSX.
   if( cellGcmInit(kDefaultCommandBufferSize, kHostMemorySize, mpHostMemory) != CELL_OK )
      BPE_VERIFYA(false, "cellGcmInit failed!");

   // New flipping code
   {
      mPresentationInterval = 1;
      mAllowImmediate = false;

      mLastFlipId = 0xf;

      mVSyncCountUntilNextFlip = 0;
      mFlipsQueued = 0;
      mFlipsExecuted = 0;

      // Create mutex to control access to flips
      sys_lwmutex_attribute_t attr;
      sys_lwmutex_attribute_initialize(attr);
      attr.attr_recursive = SYS_SYNC_RECURSIVE;
      int result = sys_lwmutex_create(&mFlipMutex, &attr);
      BPE_VERIFY(result == CELL_OK, false, "Invalid result from mutex creation");

      // Allocate RSX label for RSX/VSync syncronization
      mFlipLabel = AllocateRSXLabel();

      // Clear the label
      cellGcmSetWriteBackEndLabel(mFlipLabel, 0xFFFFFFFF);

      // Make sure the clearing of the label is processed by the RSX
      GcmFinish();

      cellGcmSetFlipHandler(&CRenderBackend::GcmFlipHandler);
      cellGcmSetVBlankHandler(&CRenderBackend::GcmVBlankHandler);
      cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);
   }


#ifdef USE_GPU_PROFILE_MARKERS
   int const reportSize = 16 * 1024*1024;
   void* reportEA = memalign(1024*1024, reportSize);
   cellGcmMapEaIoAddress(reportEA, 0x0e000000, reportSize);
#endif

   // Initialize main memory allocator
   // NOTE that GCM puts the default command buffer at the beginning of the buffer we passed into the init function, so we lose this for our main memory pool.
   // IMPORTANT: We also take away 1024 bytes from the end of the pool, this is so when we allocate a really tiny command buffer to be called into we can guarantee that it doesn't fetch into unmapped memory (command buffer dma's can fetch up to 1024 bytes).
   void* pPoolBegin = static_cast<char*>(mpHostMemory) + kDefaultCommandBufferSize;
   mpMainAllocator = new CRenderHWAllocator(pPoolBegin, kMainMemoryPoolSize - 1024, kMainMemoryHandleCount, kRM_System);
    
   // get config
   CellGcmConfig config;
   cellGcmGetConfiguration(&config);

   mpLocalAllocator = new CRenderHWAllocator(config.localAddress, config.localSize, kLocalMemoryHandleCount, kRM_Video);
   bpe_debugger_printf("RSX: (CellGcmControl*)0x%8.8x\n", cellGcmGetControlRegister());
   bpe_debugger_printf("RSX: Local Begin: 0x%8.8x End: 0x%8.8x\n", config.localAddress, (uint32)config.localAddress + config.localSize);
   bpe_debugger_printf("RSX: System Begin: 0x%8.8x End: 0x%8.8x\n", mpHostMemory, (uint32)mpHostMemory + kHostMemorySize);

   mpFragmentProgramAllocator.reset(new CFragmentProgramAllocator);

   bpe_debugger_printf("gfxInitGraphics::GCM Initialized, Host Pool Size: %d Local Pool Size: %d\n", mpMainAllocator->mPoolSize, mpLocalAllocator->mPoolSize );

   // Get current video output state
   SRenderFrameBufferSize::EDisplayAspect currentDisplayAspect;
   {
      CellVideoOutState videoState;
      if( cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState) != CELL_OK )
      {
         BPE_VERIFYA(false, "cellVideoOutGetState failed!");
      }

      mBootupResolutionId = videoState.displayMode.resolutionId;

      // Determine aspect ratio for current display mode
      switch(videoState.displayMode.aspect)
      {
      case CELL_VIDEO_OUT_ASPECT_4_3:
         currentDisplayAspect = SRenderFrameBufferSize::kDA_Fullscreen;
         break;

      case CELL_VIDEO_OUT_ASPECT_16_9:
      default:
         currentDisplayAspect = SRenderFrameBufferSize::kDA_Widescreen;
         break;
      }
   }

   // Determine display aspect for current video resolution
   CellVideoOutResolution resolution;
   cellVideoOutGetResolution(mBootupResolutionId, &resolution);

   // Find frame buffer size for monoscopic setup
   if( !FindFrameBufferSize(resolution.width, resolution.height, currentDisplayAspect, initFlags, &mFrameBufferSizeMono) )
   {
      BPE_VERIFYA(false, "Unable to find monoscopic frame buffer setup");
   }
   
   // Find frame buffer size for stereoscopic setup
   if( !FindFrameBufferSize(1280, 720, SRenderFrameBufferSize::kDA_Widescreen, initFlags, &mFrameBufferSizeStereo) )
   {
      BPE_VERIFYA(false, "Unable to find stereoscopic frame buffer setup");
   }

   InitDefaultObjects();

#if BPE_USE_EDGE_GEOM
   InitializeEdge();
#endif

   mpDynamicVertexBufferPool_RT.reset(new CDynamicVertexBufferPool_RT(initFlags.mVertexBufferPoolSize_RT, initFlags.mVertexBufferPoolChunkCount_RT));
   mpDynamicIndexBufferPool_RT.reset(new CDynamicIndexBufferPool_RT(initFlags.mIndexBufferPoolSize_RT, initFlags.mIndexBufferPoolChunkCount_RT));
   mpDynamicVertexBufferPool_UT.reset(new CDynamicVertexBufferPool_UT(initFlags.mVertexBufferPoolSize_UT, initFlags.mVertexBufferPoolChunkCount_UT));
   mpDynamicIndexBufferPool_UT.reset(new CDynamicIndexBufferPool_UT(initFlags.mIndexBufferPoolSize_UT, initFlags.mIndexBufferPoolChunkCount_UT));
   
#ifndef GOLD_VERSION
   //extra screen shot utility for
   CellScreenShotSetParam screenshot_param = {0, 0, 0, 0};

   screenshot_param.photo_title = "Confidential screenshot"; // GM_GetArea
   screenshot_param.game_title = "Confidential";
   screenshot_param.game_comment = "Confidential";

   int ret;
   ret = cellScreenShotEnable();
#endif
}

//----------------------------------------------------------------------------

CRenderBackend::EInitializeDisplayResult CBaseRenderBackend::InitializeDisplay(EStereoMode const stereoMode)
{
   CRenderBackend* pThis = (CRenderBackend*)this;

   CellVideoOutConfiguration videoConfig = { 0 };

   switch( stereoMode )
   {
   case kSM_Monoscopic:
      {
         if( pThis->mDisplayIsInitialized && !mIsStereo3D )
            return kIDR_AlreadyInSameMode;

         videoConfig.resolutionId = pThis->mBootupResolutionId;
         videoConfig.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
         videoConfig.pitch = cellGcmGetTiledPitchSize(mFrameBufferSizeMono.GetWidth(SRenderFrameBufferSize::kST_Display) * kColorBufferBPP);
       
         mIsStereo3D = false;
      }
      break;

   case kSM_Stereoscopic:
      {
         if( !SupportsStereo3D() )
            return kIDR_Failed;

         if( pThis->mDisplayIsInitialized && mIsStereo3D )
            return kIDR_AlreadyInSameMode;

#if BP_ALLOW_FAKE_3D_RESOLUTION
         videoConfig.resolutionId = CELL_VIDEO_OUT_RESOLUTION_720;
#else
         videoConfig.resolutionId = CELL_VIDEO_OUT_RESOLUTION_720_3D_FRAME_PACKING;
#endif
         videoConfig.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
         videoConfig.pitch = cellGcmGetTiledPitchSize(mFrameBufferSizeStereo.GetWidth(SRenderFrameBufferSize::kST_Display) * kColorBufferBPP);

         mIsStereo3D = true;
      }
      break;
   }

   // If display is currently initialized we need to uninitialize it first
   if( pThis->mDisplayIsInitialized )
      UninitializeDisplay();

   // set video out configuration with waitForEvent set to 0 (4th parameter)
   {
      int ret;
      bpe_debugger_printf("videoConfig.resolutionId: %d\n", videoConfig.resolutionId);
      bpe_debugger_printf("videoConfig.format:       %d\n", videoConfig.format);
      bpe_debugger_printf("videoConfig.pitch:        %d\n", videoConfig.pitch);
      bpe_debugger_printf("videoConfig.aspect:       %d\n", videoConfig.aspect);
      while ((ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videoConfig, NULL, 0)) != CELL_OK)
      {
         bpe_debugger_printf("cellVideoOutConfigure returned 0x%08x\n", ret);
         if (ret == CELL_VIDEO_OUT_ERROR_CONDITION_BUSY)
         {
            // Must be waiting for a video resolution change to finish.
            sys_timer_usleep(10000);
         }
         else
         {
            return kIDR_Failed;
         }
      }
   }

   // Update current refresh rate settings
   {
      CellVideoOutState videoState;

      int ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
      if (ret != CELL_OK )
      {
         bpe_debugger_printf("cellVideoOutGetState returned 0x%08x\n", ret);
         BPE_VERIFYA(false, "cellVideoOutGetState failed!");
      }

      // Refresh rate override for PAL
      bpe_debugger_printf("CellVideoOutState: refresh rate flags 0x%x\n", videoState.displayMode.refreshRates);
      if (videoState.displayMode.refreshRates == CELL_VIDEO_OUT_REFRESH_RATE_50HZ)
         mRefreshRate = 50.0f;
      else
         mRefreshRate = 60.0f;
   }

   // Print info about current video mode
   bpe_debugger_printf("Screen Width: %d Screen Height: %d Framebuffer Width: %d Framebuffer Height: %d Refresh rate: %f Stereo3D: %d\n", 
      GetFrameBufferSize().GetWidth(SRenderFrameBufferSize::kST_Display), GetFrameBufferSize().GetHeight(SRenderFrameBufferSize::kST_Display),
      GetFrameBufferSize().GetWidth(SRenderFrameBufferSize::kST_FrameBuffer), GetFrameBufferSize().GetHeight(SRenderFrameBufferSize::kST_FrameBuffer),
      mRefreshRate, mIsStereo3D);

   for( int i = 0; i < BPE_ARRAY_SIZE(pThis->mDisplayBufferInfo); ++i)
   {
      int const width = GetBackBufferWidth();
      int height = -1;

      if( mIsStereo3D )
         height = GetBackBufferHeight() * 2 + 30;
      else
         height = GetBackBufferHeight();

      CTexture* pDisplayBuffer = static_cast<CTexture *>( CTexture::Create(width, height, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_RenderTarget, CTexture::kAA_None ) );
      pThis->mDisplayBufferInfo[i].mpTexture.reset( pDisplayBuffer );

      cellGcmSetDisplayBuffer(
         i, 
         pThis->mDisplayBufferInfo[i].mpTexture->GcmGetOffset(),
         pThis->mDisplayBufferInfo[i].mpTexture->GcmGetPitch(),
         pThis->mDisplayBufferInfo[i].mpTexture->GetWidth(),
         pThis->mDisplayBufferInfo[i].mpTexture->GetHeight() );
   }

   pThis->mDisplayIsInitialized = true;

   return kIDR_Succeded;
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::UninitializeDisplay()
{
   CRenderBackend* pThis = (CRenderBackend*)this;

   if( !pThis->mDisplayIsInitialized )
      return;

   bpe_debugger_printf("Begin destroy device...\n");

   // Clear all back buffers to black so we don't see corruption during the resolution switch.
   for( int i = 0; i < BPE_ARRAY_SIZE(pThis->mDisplayBufferInfo); ++i )
   {
      pThis->ClearAndSwapForOSD();
   }

   // Wait for RSX to finish doing EVERYTHING!
   pThis->GcmWaitForIdle();

   // Free all back buffers
   for( int i = 0; i < BPE_ARRAY_SIZE(pThis->mDisplayBufferInfo); ++i )
   {
      pThis->mDisplayBufferInfo[i].mpTexture.reset();
   }

   // Make sure all the memory is fully free before we allocate new back buffers.
   pThis->WaitForPendingAllocationsToClear();

   pThis->mDisplayIsInitialized = false;
}

//----------------------------------------------------------------------------

bool CBaseRenderBackend::SupportsStereo3D() const
{
#if BP_ALLOW_FAKE_3D_RESOLUTION
   return true;
#else
   return cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, CELL_VIDEO_OUT_RESOLUTION_720_3D_FRAME_PACKING, CELL_VIDEO_OUT_ASPECT_AUTO, NULL);
#endif
}

//----------------------------------------------------------------------------

real32 CBaseRenderBackend::GetDisplaySize() const
{
    float screenSize = 0.0f;
 
    if( cellVideoOutGetScreenSize(CELL_VIDEO_OUT_PRIMARY, &screenSize) == 0 )
       return screenSize;
    else
      return 40.0f;  // 40 inches by default, but this really only happens if the TV does not support 3D.
}

//----------------------------------------------------------------------------

CRenderBackend::~CRenderBackend()
{
#if BPE_USE_EDGE_GEOM
   ShutdownEdge();
#endif
   ShutdownSPU();

   // wait to make sure we're all done!
   GcmWaitForIdle();

   cellGcmSetFlipHandler(NULL);
   cellGcmSetVBlankHandler(NULL);

   sys_lwmutex_destroy(&mFlipMutex);

   for( int i = 0; i < BPE_ARRAY_SIZE(mDisplayBufferInfo); ++i )
   {
      mDisplayBufferInfo[i].mpTexture.reset();
   }

   mShaderCache.release();
}

//----------------------------------------------------------------------------

void CRenderBackend::BeginScene()
{
   InternalBeginScene();

#if !ENABLE_SEPERATE_RENDER_THREAD
   BeginFrameResourceTick();
#endif

#if BP_ENABLE_FRAME_ALLOCATOR
   FrameAllocator::Reset();
#endif

#if BPE_USE_EDGE_GEOM
   Edge::gEdgeGeomSubmittedCount = 0;
   Edge::pCurrentViewport = NULL;
   Edge::pCurrentLocalToWorld = NULL;
#endif

   // disable dither
   cellGcmSetDitherEnable(CELL_GCM_FALSE);

   // enable culling, CCW triangles are culled by default!
   cellGcmSetFrontFace(CELL_GCM_CW);
   cellGcmSetCullFace(CELL_GCM_BACK);
   cellGcmSetCullFaceEnable(CELL_GCM_TRUE);

   cellGcmSetDepthTestEnable(CELL_GCM_TRUE);
   cellGcmSetDepthFunc(CELL_GCM_LEQUAL);

   cellGcmSetAlphaTestEnable(CELL_GCM_FALSE);
   cellGcmSetAlphaFunc(CELL_GCM_GREATER, 0);
}

//----------------------------------------------------------------------------

void CRenderBackend::EndScene()
{
   Present();
}

//----------------------------------------------------------------------------

void CRenderBackend::Present()
{
   // Wait for last flip to finish.
   {
      CStopWatch timer;

      int const maxQueuedFlips = BPE_ARRAY_SIZE(mDisplayBufferInfo) - 1;

      while( (mFlipsQueued - mFlipsExecuted) > maxQueuedFlips )
      {
         sys_timer_usleep(10);

         if( mAllowImmediate )
            TryFlipImmediate();
      }

      // measure time amount of time we waited
      real32 const vblankStallTime = timer.GetElapsedTime();
      mMetrics.mVBlankWaitTime = vblankStallTime;
   }

   // One frame is now done rendering by the time it gets here.
   mpLocalAllocator->FlushPendingFree();
   mpMainAllocator->FlushPendingFree();

   //printf("Present, Flips remaining: %d\n", mFlipsQueued - mFlipsExecuted);

   // Insert a prepare flip (get as much done as possible before vsync)
   int32_t flipId = cellGcmSetPrepareFlip(mCurrentDisplayBuffer);
   BPE_VERIFY(flipId != CELL_GCM_ERROR_FAILURE, false, "Invalid prepare flip result");

   // Increment number of flips queued
   mFlipsQueued++;

   // Make it so we can wait on the GPU to finish up to here
   cellGcmSetWriteBackEndLabel(mFlipLabel, (mCurrentDisplayBuffer << 8) | flipId);

   // Make sure RSX will process the commands queued up to this point (the flip).
   cellGcmFlush();

   // Advance to the next buffer
   mCurrentDisplayBuffer = (mCurrentDisplayBuffer + 1) % BPE_ARRAY_SIZE(mDisplayBufferInfo);

   // Invalidate bound render target, that way it will rebind it on the next "SetRenderTarget"
   mRenderTargetsNeedsRebind = true;

   mBoundIndexBuffer = 0;
   mBoundIndexBufferLocation = 0;
   mBoundIndexBufferType = 0;

   ForceVertexDataRebind();

   InternalPresent();
}

//----------------------------------------------------------------------------

void CRenderBackend::ShutDown()
{
#if ENABLE_SEPERATE_RENDER_THREAD
   ShutdownThreadSystem();
#endif

   for( int i = 0; i < BPE_ARRAY_SIZE(mDisplayBufferInfo); ++i )
   {
      ClearAndSwapForOSD();
   }

   GcmFinish();
}

//----------------------------------------------------------------------------

void CRenderBackend::WaitForPendingAllocationsToClear()
{
   bool keepTrying = true;

   while( keepTrying )
   {
      keepTrying = false;

      mpLocalAllocator->FlushPendingFree();
      mpMainAllocator->FlushPendingFree();

      SRenderHWAllocatorStats stats;
      RenderBackend()->GetMemoryStats(kRM_System, &stats);

      if( stats.mPendingFreeBlocks > 0 )
      {
         keepTrying = true;
      }

      RenderBackend()->GetMemoryStats(kRM_Video, &stats);
      if( stats.mPendingFreeBlocks > 0 )
      {
         keepTrying = true;
      }
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::Clear(int const clearFlags, CColor const color, real32 const z, int const stencil)
{
   uint32 mask = 0;

   if( clearFlags & kFlag_Color )
   {
      mask |= CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A;
      cellGcmSetClearColor(color.GetARGB());
   }

   if( mCurrentRenderTarget.mpDepthBuffer )
   {
      uint32 depthStencilClearValue = 0;

      if( clearFlags & kFlag_Depth )
      {
         mask |= CELL_GCM_CLEAR_Z;
         depthStencilClearValue |= uint32(0xFFFFFF * MathUtils::ClampMinMax(z, 0.0f, 1.0f)) << 8;
      }

      if( clearFlags & kFlag_Stencil )
      {
         mask |= CELL_GCM_CLEAR_S;
         depthStencilClearValue |= stencil & 0xFF;
      }

      cellGcmSetClearDepthStencil(depthStencilClearValue);
   }

   if( mask != 0 )
      cellGcmSetClearSurface(mask);

#ifdef DEBUG_FIFO_HANGS
   GcmFinish();
#endif

#ifdef FLUSH_AFTER_RENDERCALLS
   cellGcmFlush();
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::SetRenderTarget(SRenderTarget const & renderTarget)
{
   // early out
   if( !mRenderTargetsNeedsRebind && mCurrentRenderTarget == renderTarget )
      return;

#if BPE_USE_EDGE_GEOM
   EdgeGeomViewportInfo* pEdgeViewport = Edge::GetWriteableViewport();
#endif

   mCurrentRenderTarget = renderTarget;

   CellGcmSurface surface;

   surface.type		= CELL_GCM_SURFACE_PITCH;

   // default format
   surface.colorFormat = CELL_GCM_SURFACE_A8R8G8B8;

   // bind color buffers
   uint8 validColorBuffersBitMask = 0;

   // Find first valid render target to be used for unbound render targets, this is done to avoid Setup replay mode (see section 7.4.2 of RSX User Guide)
   // ALL render targets (whether used or not) must be in a tiled region
   CTexture* pFirstValidRenderTarget = NULL;
   CBaseTexture::EAntiAliasType aaType = CBaseTexture::kAA_Invalid;

   // Allow override
   if( renderTarget.mForceAAMode != CBaseTexture::kAA_Invalid )
      aaType = renderTarget.mForceAAMode;

   // Find aa type based on current render target
   for( int i = 0; i < 4; ++i )
   {
      CTexture* pRenderTarget = (CTexture*)mCurrentRenderTarget.mpColorBuffer[i];
      if( pRenderTarget != NULL )
      {
         pFirstValidRenderTarget = pRenderTarget;

         if( aaType == CBaseTexture::kAA_Invalid )
            aaType = pFirstValidRenderTarget->GetAntiAliasType();

         break;
      }
   }

   for( int i = 0; i < 4; ++i )
   {
      CTexture* pRenderTarget = (CTexture*)mCurrentRenderTarget.mpColorBuffer[i];
      
      if( pRenderTarget != NULL )
      {
         validColorBuffersBitMask |= (1 << i);

         mRenderTargetWidth = pRenderTarget->GetAntiAliasWidth(aaType);
         mRenderTargetHeight = pRenderTarget->GetAntiAliasHeight(aaType);

         surface.colorFormat = pRenderTarget->GcmGetSurfaceFormat();
         surface.colorLocation[i] = pRenderTarget->GcmGetLocation();
         surface.colorOffset[i] = pRenderTarget->GcmGetOffset();
         surface.colorPitch[i] = pRenderTarget->GcmGetPitch();
      }
      else
      {
         // "disable" color target
         if( pFirstValidRenderTarget )
         {
            surface.colorFormat = pFirstValidRenderTarget->GcmGetSurfaceFormat();
            surface.colorLocation[i] = pFirstValidRenderTarget->GcmGetLocation();
            surface.colorOffset[i] = pFirstValidRenderTarget->GcmGetOffset();
            surface.colorPitch[i] = pFirstValidRenderTarget->GcmGetPitch();
         }
         else
         {
            surface.colorLocation[i] = CELL_GCM_LOCATION_LOCAL;
            surface.colorOffset[i] = 0;
            surface.colorPitch[i] = 64;
         }
      }
   }

   // bind depth buffer if it is provided
   if( mCurrentRenderTarget.mpDepthBuffer )
   {
      CTexture* pDepthBuffer = (CTexture*)mCurrentRenderTarget.mpDepthBuffer;

      if ( pFirstValidRenderTarget == NULL )
      {
         if( aaType == CBaseTexture::kAA_Invalid )
            aaType = pDepthBuffer->GetAntiAliasType();
      }

      mRenderTargetWidth = pDepthBuffer->GetAntiAliasWidth(aaType);
      mRenderTargetHeight = pDepthBuffer->GetAntiAliasHeight(aaType);

      surface.depthFormat = pDepthBuffer->GcmGetDepthSurfaceFormat();
      surface.depthLocation = pDepthBuffer->GcmGetLocation();
      surface.depthOffset = pDepthBuffer->GcmGetOffset();
      surface.depthPitch = pDepthBuffer->GcmGetPitch();
   }
   else
   {
      surface.depthFormat = CELL_GCM_SURFACE_Z24S8;
      surface.depthLocation = 0;
      surface.depthOffset = 0;
      surface.depthPitch = 64;
   }

   int const backBufferWidth = mDisplayBufferInfo[mCurrentDisplayBuffer].mpTexture->GetWidth();
   int const backBufferHeight = mDisplayBufferInfo[mCurrentDisplayBuffer].mpTexture->GetHeight();

   // determine if we need to bind the back buffer.
   bool bindBackBuffer = false;
   if( renderTarget.mBindBackBuffer != SRenderTarget::kBBM_NoBackBuffer )
   {
      // no color buffers bound yet?
      if( validColorBuffersBitMask == 0 )
      {
         // if we have a depth buffer, only bind it if the dimensions match!
         if( mCurrentRenderTarget.mpDepthBuffer )
         {
            if( backBufferWidth == mCurrentRenderTarget.mpDepthBuffer->GetWidth() &&
                backBufferHeight == mCurrentRenderTarget.mpDepthBuffer->GetHeight() )
            {
               bindBackBuffer = true;
            }
         }
         else
         {
            // if we don't have a depth buffer always bind the back buffer, otherwise we wouldn't be binding ANYTHING.
            bindBackBuffer = true;
         }
      }
   }   

   // if no valid buffers provided we bind the back buffer
   if( bindBackBuffer )
   {
      if( aaType == CBaseTexture::kAA_Invalid )
         aaType = CBaseTexture::kAA_None;

      mRenderTargetWidth = backBufferWidth;
      mRenderTargetHeight = backBufferHeight;

      surface.colorLocation[0] = CELL_GCM_LOCATION_LOCAL;
      surface.colorOffset[0] = mDisplayBufferInfo[mCurrentDisplayBuffer].mpTexture->GcmGetOffset();
      surface.colorPitch[0] = mDisplayBufferInfo[mCurrentDisplayBuffer].mpTexture->GcmGetPitch();

      validColorBuffersBitMask = (1 << 0);

      mCurrentRenderTarget.mpColorBuffer[0] = mDisplayBufferInfo[mCurrentDisplayBuffer].mpTexture.get();
   }

   cellGcmSetAntiAliasingControl(aaType != CBaseTexture::kAA_None ? CELL_GCM_TRUE : CELL_GCM_FALSE, CELL_GCM_FALSE, CELL_GCM_FALSE, 0xFFFF);

   switch(validColorBuffersBitMask)
   {
   case 0:
      surface.colorTarget = CELL_GCM_SURFACE_TARGET_NONE;
      break;

   case (1 << 0):
      surface.colorTarget = CELL_GCM_SURFACE_TARGET_0;
      break;

   case (1 << 1):
      surface.colorTarget = CELL_GCM_SURFACE_TARGET_1;
      break;

   case (1 << 0 | 1 << 1):
      surface.colorTarget = CELL_GCM_SURFACE_TARGET_MRT1;
      break;

   case (1 << 0 | 1 << 1 | 1 << 2):
      surface.colorTarget = CELL_GCM_SURFACE_TARGET_MRT2;
      break;

   case (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3):
      surface.colorTarget = CELL_GCM_SURFACE_TARGET_MRT3;
      break;

   default:
      BPE_VERIFYA(false, "Invalid render target combination");
      break;
   }

   mCurrentRenderTargetAAType = aaType;

   switch ( aaType )
   {
   case CBaseTexture::kAA_None:
      surface.antialias	= CELL_GCM_SURFACE_CENTER_1;
      break;
   case CBaseTexture::kAA_MSAA2x:
      surface.antialias = CELL_GCM_SURFACE_DIAGONAL_CENTERED_2;
      break;
   case CBaseTexture::kAA_MSAA4x:
      surface.antialias = CELL_GCM_SURFACE_SQUARE_ROTATED_4;
      break;
   default:
      BPE_VERIFYA( false, "Invalid antialias mode" );
      break;
   }

   int surfaceOffsetY = 0;

   // Do S3D specific render target setup
   if( bindBackBuffer )
   {
      switch( renderTarget.mStereoViewport )
      {
      case SRenderTarget::kSVS_None:
         break;

      case SRenderTarget::kSVS_Left:
         mRenderTargetHeight = (mRenderTargetHeight - 30) / 2;
         break;

      case SRenderTarget::kSVS_Right:
         mRenderTargetHeight = (mRenderTargetHeight - 30) / 2;
         surfaceOffsetY = 30 + mRenderTargetHeight;
         break;
      
      case SRenderTarget::kSVS_GapArea:
         mRenderTargetHeight = 30;
         surfaceOffsetY = mRenderTargetHeight;
         break;
      }
   }

   surface.x = 0;
   surface.y = surfaceOffsetY;
   surface.width = mRenderTargetWidth;
   surface.height = mRenderTargetHeight;

   //NOTE: This pixel center half is for the WPOS register provided in the shader.
   cellGcmSetSurfaceWindow(&surface, CELL_GCM_WINDOW_ORIGIN_TOP, CELL_GCM_WINDOW_PIXEL_CENTER_HALF);

   // NOTE:
   // - Anti aliasing control is setup inside ValidateState function
   // - Color mask is setup inside ValidateState function.

   int viewportX = 0;
   int viewportY = 0;
   int viewportWidth = mRenderTargetWidth;
   int viewportHeight = mRenderTargetHeight;

   if( renderTarget.mRenderTargetViewPort.mViewportEnabled )
   {
      viewportX = renderTarget.mRenderTargetViewPort.mViewportX;
      viewportY = renderTarget.mRenderTargetViewPort.mViewportY;
      viewportWidth = renderTarget.mRenderTargetViewPort.mViewportWidth;
      viewportHeight = renderTarget.mRenderTargetViewPort.mViewportHeight;
   }

   // if we have a border size, set up a scissor rectangle
   if( renderTarget.mScissorEnabled )
   {
      cellGcmSetScissor(renderTarget.mScissorX, renderTarget.mScissorY, renderTarget.mScissorWidth, renderTarget.mScissorHeight);

#if BPE_USE_EDGE_GEOM
      pEdgeViewport->scissorArea[0] = renderTarget.mScissorX;
      pEdgeViewport->scissorArea[1] = renderTarget.mScissorY;
      pEdgeViewport->scissorArea[2] = renderTarget.mScissorWidth;
      pEdgeViewport->scissorArea[3] = renderTarget.mScissorHeight;
#endif
   }
   else
   {
      // Set scissor test to viewport rectangle
      cellGcmSetScissor(viewportX, viewportY, viewportWidth, viewportHeight);

#if BPE_USE_EDGE_GEOM
      pEdgeViewport->scissorArea[0] = viewportX;
      pEdgeViewport->scissorArea[1] = viewportY;
      pEdgeViewport->scissorArea[2] = viewportWidth;
      pEdgeViewport->scissorArea[3] = viewportHeight;
#endif
   }


   // This viewport set up matches D3D conventions
   float scale[4];
   scale[0] = viewportWidth * 0.5f;
   scale[1] = -viewportHeight * 0.5f;
   scale[2] = ( renderTarget.mMaxZ - renderTarget.mMinZ );
   scale[3] = 0.0f;
   
   float offset[4];
   offset[0] = viewportX + viewportWidth * 0.5f;
   offset[1] = viewportY + viewportHeight * 0.5f;
   offset[2] = renderTarget.mMinZ;
   offset[3] = 0.0f;

   mViewportWidth = viewportWidth;
   mViewportHeight = viewportHeight;

   if( bindBackBuffer )
   {
      offset[0] = GetBackBufferWidth() * 0.5f;
      offset[1] = GetBackBufferHeight() * 0.5f;

      switch( renderTarget.mBindBackBuffer )
      {
      case SRenderTarget::kBBM_BackBufferFill:
         {
            scale[0] = GetBackBufferWidth() * 0.5f;
            scale[1] = -GetBackBufferHeight() * 0.5f;
         }
         break;

      case SRenderTarget::kBBM_BackBufferCentered:
         {
            mViewportWidth = GetBackBufferWidth();
            mViewportHeight = ( GetFrameBufferSize().GetDisplayAspect() == SRenderFrameBufferSize::kDA_Fullscreen ) ? (GetBackBufferHeight() * 3 / 4) : GetBackBufferHeight();

            scale[0] = mViewportWidth * 0.5f;
            scale[1] = mViewportHeight * -0.5f;
         }
         break;      
      }
   }

   // offset by 0.5 pixels to get it to match D3D spec
   offset[0] += 0.5f;
   offset[1] += 0.5f;

   cellGcmSetViewport(viewportX, viewportY, viewportWidth, viewportHeight, renderTarget.mMinZ, renderTarget.mMaxZ, scale, offset);

#if BPE_USE_EDGE_GEOM
   pEdgeViewport->sampleFlavor = CELL_GCM_SURFACE_CENTER_1;

   pEdgeViewport->depthRange[0] = renderTarget.mMinZ;
   pEdgeViewport->depthRange[1] = renderTarget.mMaxZ;

   pEdgeViewport->viewportOffsets[0] = offset[0];
   pEdgeViewport->viewportOffsets[1] = offset[1];
   pEdgeViewport->viewportOffsets[2] = offset[2];
   pEdgeViewport->viewportOffsets[3] = offset[3];

   pEdgeViewport->viewportScales[0] = scale[0];
   pEdgeViewport->viewportScales[1] = scale[1];
   pEdgeViewport->viewportScales[2] = scale[2];
   pEdgeViewport->viewportScales[3] = scale[3];
#endif
}

//----------------------------------------------------------------------------

CBaseTexture* CRenderBackend::GetCurrentBackbufferTexture() const
{
   return mDisplayBufferInfo[mCurrentDisplayBuffer].mpTexture.get();
}

//----------------------------------------------------------------------------

CBaseTexture* CRenderBackend::GetLastBackbufferTexture() const
{
   int const lastBackBufferIndex = mCurrentDisplayBuffer + BPE_ARRAY_SIZE(mDisplayBufferInfo) - 1;

   return mDisplayBufferInfo[ lastBackBufferIndex % BPE_ARRAY_SIZE(mDisplayBufferInfo) ].mpTexture.get();
}

//----------------------------------------------------------------------------

// Node this function is hideously expensive!
void CRenderBackend::RenderQuad2d(CVector2 const & start,
                                  CVector2 const & end,
                                  CColor const & color)
{
#if BP_RENDER_GEOMETRY
   // setup projection
   SetScreenSpaceOrtho();

   // setup default shader with constant color and bind it
   CDefaultShader shader;
   shader.mConstColor = CColorf(color);
   shader.mZEnable = false;
   shader.mBlendMode = CDefaultShader::kBM_Alpha;
   shader.Bind(NULL);
   shader.BeginSinglePassRender();

   // enable position vertex arrays
   BPE_CTASSERT(kVDU_Position == 0);
   cellGcmSetVertexDataArray(kVDU_Position, 0, 12, 2, CELL_GCM_VERTEX_F32, 0, NULL);

   // disable all others vertex arrays
   for( int i = 1; i < kVDU_Count; ++i )
      cellGcmSetVertexDataArray(i, 0, 0, 0, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, 0);

   real32 const positions[] = 
   {
      start.GetX(),  start.GetY(),
      end.GetX(),    start.GetY(),
      end.GetX(),    end.GetY(),
      start.GetX(),  end.GetY()
   };

   ShaderCache()->ValidateState();

   // draw inline, 
   cellGcmSetDrawInlineArray(CELL_GCM_PRIMITIVE_QUADS, sizeof(positions) / sizeof(real32), positions);

#ifdef DEBUG_FIFO_HANGS
   GcmFinish();
#endif

   mMetrics.mNumRenderPrimitiveCalls++;

   // unbind shader
   shader.EndSinglePassRender();

   ForceVertexDataRebind();
#endif
}

//----------------------------------------------------------------------------

// Note this function is hideously expensive!
void CRenderBackend::RenderLine2d(CVector2 const & start,
                                  CVector2 const & end,
                                  CColor const & color )
{
#if BP_RENDER_GEOMETRY
   // setup projection
   SetScreenSpaceOrtho();

   // setup default shader with constant color and bind it
   CDefaultShader shader;
   shader.mConstColor = CColorf(color);
   shader.mZEnable = false;
   shader.mBlendMode = CDefaultShader::kBM_Alpha;
   shader.Bind(NULL);
   shader.BeginSinglePassRender();

   // enable position vertex arrays
   BPE_CTASSERT(kVDU_Position == 0);
   cellGcmSetVertexDataArray(kVDU_Position, 0, 12, 2, CELL_GCM_VERTEX_F32, 0, NULL);

   // disable all others vertex arrays
   for( int i = 1; i < kVDU_Count; ++i )
      cellGcmSetVertexDataArray(i, 0, 0, 0, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, 0);

   real32 const positions[] = 
   {
      start.GetX(),  start.GetY(),
      end.GetX(),    end.GetY()
   };

   ShaderCache()->ValidateState();

   // draw inline, 
   cellGcmSetDrawInlineArray(CELL_GCM_PRIMITIVE_LINES, sizeof(positions) / sizeof(real32), positions);

#ifdef DEBUG_FIFO_HANGS
   GcmFinish();
#endif

   mMetrics.mNumRenderPrimitiveCalls++;

   // unbind shader
   shader.EndSinglePassRender();

   ForceVertexDataRebind();
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
                                                    CShaderVertexDataBinding binding, 
                                                    void const * pData, 
                                                    uint32 const numVertices)
{
#if BP_RENDER_GEOMETRY
   ShaderCache()->ValidateState();

   int const vertexSize = (binding.HasStream(kVDU_Position) ? sizeof(CVector3) : 0) + 
                          (binding.HasStream(kVDU_Normal) ? sizeof(CVector3) : 0) + 
                          (binding.HasStream(kVDU_Color0) ? sizeof(uint32) : 0) + 
                          (binding.HasStream(kVDU_TexCoord0) ? sizeof(CVector2) : 0);

   
   CDynamicVertexBufferPoolChunk_RT* vertexBuffer = GetVertexBufferPool_RT()->AllocChunk(vertexSize * numVertices);
   if( vertexBuffer )
   {
      void* res = vertexBuffer->Lock();

      memcpy(res, pData, vertexSize*numVertices);

      CVertexData vertexData;

      uint32 dataOffset = 0;

      if( binding.HasStream(kVDU_Position) )
      {
         vertexData.SetAttribute(kVDS_Position, dataOffset, vertexSize, kVDT_Float3, vertexBuffer);
         dataOffset += sizeof(CVector3);
      }

      if( binding.HasStream(kVDU_Normal) )
      {
         vertexData.SetAttribute(kVDS_Normal, dataOffset, vertexSize, kVDT_Float3, vertexBuffer);
         dataOffset += sizeof(CVector3);
      }

      if( binding.HasStream(kVDU_Color0) )
      {
         vertexData.SetAttribute(kVDS_Color0, dataOffset, vertexSize, kVDT_UByte4N, vertexBuffer);
         dataOffset += sizeof(uint32);
      }

      if( binding.HasStream(kVDU_TexCoord0) )
      {
         vertexData.SetAttribute(kVDS_TexCoord0, dataOffset, vertexSize, kVDT_Float2, vertexBuffer);
         dataOffset += sizeof(CVector2);
      }

      uint64 const vertexDataHash_1 = binding.GetHash();
      uint64 const vertexDataHash_2 = uint64(res);
      SetVertexData(binding, vertexData, vertexDataHash_1, vertexDataHash_2);

      cellGcmSetDrawArrays(gPrimitiveMapping[type], 0, numVertices);

#ifdef DEBUG_FIFO_HANGS
      GcmFinish();
#endif

      mMetrics.mNumRenderPrimitiveCalls++;
#endif
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTexture(int const texUnit, CBaseTexture const * pTexture, bool const allowAniso)
{
   pTexture->SetCurrentTextureParams(texUnit);

   CTexture const * pPS3Texture = (CTexture const *)pTexture;
   if( pPS3Texture->mMemory )
   {
      CellGcmTexture const * gcmTexture = &pPS3Texture->mTexture;

      cellGcmSetTexture(texUnit, gcmTexture);

      int maxAniso = CELL_GCM_TEXTURE_MAX_ANISO_1;

      bool const hasMips = (gcmTexture->mipmap > 1);
      
      if(hasMips && allowAniso )
         maxAniso = CELL_GCM_TEXTURE_MAX_ANISO_4;

      cellGcmSetTextureControl(texUnit, CELL_GCM_TRUE, 0 << 8, bpe::min_val(12, pTexture->GetMipCount() + pTexture->mMaxLODOffset) << 8, maxAniso);
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTextureAddressMode(int const texUnit, EWrapMode const wrapU, EWrapMode const wrapV)
{
   cellGcmSetTextureAddress(texUnit, wrapU, wrapV, CELL_GCM_TEXTURE_WRAP, CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_LESS, 0);
}

//----------------------------------------------------------------------------

real32 gTextureLODBias = 0.0f;

void CRenderBackend::SetTextureFilter(int const texUnit, EFilterMode minFilter, EFilterMode magFilter)
{
   cellGcmSetTextureFilter(texUnit, (int)(gTextureLODBias * 256.0f) & 0x1fff, minFilter, magFilter, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetTextureFilter(int const texUnit, real32 const lodBias, EFilterMode minFilter, EFilterMode magFilter)
{
   cellGcmSetTextureFilter(texUnit, (int)(lodBias * 256.0f) & 0x1fff, minFilter, magFilter, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetVertexRegisters(int startRegister, int numVectors, CVector4 const * pVector)
{
   cellGcmSetVertexProgramConstants(startRegister, numVectors << 2, (real32*)pVector);
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::RenderQuad(real32 minX, real32 const maxX, real32 const minY, real32 const maxY, real32 const z, real32 const minU, real32 const maxU, real32 const minV, real32 const maxV)
{
#if BP_RENDER_GEOMETRY
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

   for( int i = 0; i < kVDU_Count; ++i )
      cellGcmSetVertexDataArray(i, 0, 0, 0, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, 0);

   cellGcmSetVertexDataArray(kVDU_Position, 0, 20, 3, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, NULL);
   cellGcmSetVertexDataArray(kVDU_TexCoord0, 12, 20, 2, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, NULL);

   ShaderCache()->ValidateState();

   // draw inline, 
   cellGcmSetDrawInlineArray(CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, sizeof(vertices) / sizeof(real32), vertices);

#ifdef DEBUG_FIFO_HANGS
   ((CRenderBackend*)this)->GcmFinish();
#endif

   mMetrics.mNumRenderPrimitiveCalls++;

   ((CRenderBackend*)this)->ForceVertexDataRebind();
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::SetIndexData(CIndexBuffer const * indexBuffer)
{
   CRenderBackend::GcmGetOffsetAndLocation(indexBuffer->mpMemory_RT, &mBoundIndexBuffer, &mBoundIndexBufferLocation);
   mBoundIndexBufferType = indexBuffer->mType;
}

//----------------------------------------------------------------------------

template< class Tx, class Ty > inline void SetIndexDataTemplate(CRenderBackend* pRenderBackend, Tx const * pChunk)
{
   Ty const * pPool = pChunk->GetPool();
   CIndexBuffer const * pIndexBuffer = pPool->GetIndexBuffer();

   // Grab type
   pRenderBackend->mBoundIndexBufferType = pIndexBuffer->mType;

   // Grab offset/location id
   pRenderBackend->GcmGetOffsetAndLocation(pIndexBuffer->mpMemory_RT, &pRenderBackend->mBoundIndexBuffer, &pRenderBackend->mBoundIndexBufferLocation);

   // adjust offset based on chunk start index.
   switch(pIndexBuffer->mType)
   {
   case kIT_Uint16:
      pRenderBackend->mBoundIndexBuffer += pChunk->GetStartIndex() * sizeof(uint16);
      break;

   case kIT_Uint32:
      pRenderBackend->mBoundIndexBuffer += pChunk->GetStartIndex() * sizeof(uint32);
      break;
   }
}

void CRenderBackend::SetIndexData(CDynamicIndexBufferPoolChunk_RT const * pChunk)
{
   SetIndexDataTemplate<CDynamicIndexBufferPoolChunk_RT,CDynamicIndexBufferPool_RT>(this, pChunk);
}

void CRenderBackend::SetIndexData(CDynamicIndexBufferPoolChunk_UT const * pChunk)
{
   SetIndexDataTemplate<CDynamicIndexBufferPoolChunk_UT,CDynamicIndexBufferPool_UT>(this, pChunk);
}

//----------------------------------------------------------------------------

void CRenderBackend::SetIndexData(uint32 const indexBuffer, uint32 const indexBufferLocation, uint32 const indexBufferType)
{
   mBoundIndexBuffer = indexBuffer;
   mBoundIndexBufferLocation = indexBufferLocation;
   mBoundIndexBufferType = indexBufferType;
}

//----------------------------------------------------------------------------

struct SAttrFlags
{
   uint32   mType;
   uint32   mSize;
};

static const SAttrFlags kAttrFlags[] =
{
   // kVDT_Float2
   {  CELL_GCM_VERTEX_F32, 2 },

   // kVDT_Float3
   {  CELL_GCM_VERTEX_F32, 3 },

   // kVDT_Float4
   {  CELL_GCM_VERTEX_F32, 4 },

   // kVDT_UByte4N
   {  CELL_GCM_VERTEX_U8_NR, 4 },

   // kVDT_UByte4
   {  CELL_GCM_VERTEX_U8_UN, 4 },

   // kVDT_Half2
   {  CELL_GCM_VERTEX_F16, 2 },

   // kVDT_Half4
   {  CELL_GCM_VERTEX_F16, 4 },

   // kVDT_Short2N
   {  CELL_GCM_VERTEX_S16_NR, 2 },

   // kVDT_Short4N
   {  CELL_GCM_VERTEX_S16_NR, 4 },

   // kVDT_Packed3N
   {  CELL_GCM_VERTEX_S11_11_10_NR, 1 },

   // kVDT_Int32
   {  CELL_GCM_VERTEX_F32, 1 }, // This is a BS value and should never get hit

   // kVDT_Short2
   {  CELL_GCM_VERTEX_S16_UN, 2 },

   // kVDT_Short4
   {  CELL_GCM_VERTEX_S16_UN, 4 },
};

BPE_CTASSERT(BPE_ARRAY_SIZE(kAttrFlags) == kVDT_Count);

//----------------------------------------------------------------------------

#if 1
void CRenderBackend::SetVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData, uint64 const vertexDataHash_1, uint64 const vertexDataHash_2)
{
   // We don't have a lot of the same vertex data being set back to back.
   if( BPE_BRANCH_HINT_FALSE( (vertexDataHash_1 == mBoundVertexDataHash_1) && (vertexDataHash_2 == mBoundVertexDataHash_2) ) )
      return;

   mBoundVertexDataHash_1 = vertexDataHash_1;
   mBoundVertexDataHash_2 = vertexDataHash_2;

   uint32 const disabledAttributes = ~vertexDataBinding.InternalGetAttributeMask();

   // Bind streams that should be bound according to the vertex data binding.
   // This will also disable any streams that we ought to have according to the vertex data binding but weren't provided as part of the vertex data.
   {
      uint64 streamIndices = vertexDataBinding.InternalGetStreamIndices();

      uint32 currentDisable = disabledAttributes;
      for( int i = 0; i < kVDU_Count; ++i, currentDisable >>= 1, streamIndices >>= 4 )
      {
         int const isDisabled = currentDisable & 1;

         // It's likely to be disabled (There are 16 attributes total and we usually only have about 3-4 set)
         if( BPE_BRANCH_HINT_FALSE(!isDisabled) )
         {
            // Get data for binding that should be bound to the attribute.
            int const streamIndex = streamIndices & 0xF;

            CVertexData::SVertexAttribute const & attribute = vertexData.mAttributes[streamIndex];

            // If the attribute is NOT disabled then it's unlikely for data not to exist!
            if( BPE_BRANCH_HINT_TRUE(attribute.mStride > 0) )
            {
               SAttrFlags const & flags = kAttrFlags[attribute.mType];
               cellGcmSetVertexDataArray(i, 0, attribute.mStride, flags.mSize, flags.mType, attribute.mVertexBufferLocation, attribute.mVertexBuffer + attribute.mOffset);
            }
            else
            {
               // If attribute is enabled and no data in the CVertexData then disable it here
               // NOTE: This is not a very common case.
               cellGcmSetVertexDataArray(i, 0, 0, 0, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, 0);
            }
         }
      }
   }

   // Disable streams that need are not desired by the vertex data binding.
   {
      // Don't disable attributes that are already disabled.
      // Mask out any attributes that are already disabled.
      uint32 const attributesToDisable = disabledAttributes;
      uint32 currentDisable = attributesToDisable & ~mDisabledVertexAttributeMask;
   
      // Anything that needs disabling at all?
      if( currentDisable != 0 ) 
      {
         for( int i = 0; i < kVDU_Count; ++i, currentDisable >>= 1 )
         {
            if( currentDisable & 1 )
            {
               cellGcmSetVertexDataArray(i, 0, 0, 0, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, 0);
            }
         }
      }
   }

   mDisabledVertexAttributeMask = disabledAttributes;
   
   cellGcmSetInvalidateVertexCache();
}
#else
void CRenderBackend::SetVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData, uint64 const vertexDataHash_1, uint64 const vertexDataHash_2)
{
   if ((vertexDataHash_1 == mBoundVertexDataHash_1) &&
      (vertexDataHash_2 == mBoundVertexDataHash_2))
   {
      return;
   }

   mBoundVertexDataHash_1 = vertexDataHash_1;
   mBoundVertexDataHash_2 = vertexDataHash_2;

   // Go over all usages
   for( int i = 0; i < kVDU_Count; ++i )
   {
      // see if we have a binding
      if( vertexDataBinding.HasStream((EVertexDataUsage)i) )
      {

         // get attribute data for binding
         EVertexDataStream const vertexStream = vertexDataBinding.GetStream((EVertexDataUsage)i);
         CVertexData::SVertexAttribute const & attribute = vertexData.mAttributes[vertexStream];

         // if it's enabled, bind it
         if( attribute.mStride )
         {
            SAttrFlags const & flags = kAttrFlags[attribute.mType];

            int const bufferStartOffset = vertexData.mBaseVertexIndex * attribute.mStride;
            cellGcmSetVertexDataArray(i, 0, attribute.mStride, flags.mSize, flags.mType, attribute.mVertexBufferLocation, attribute.mVertexBuffer + bufferStartOffset + attribute.mOffset);

            // move on to next buffer
            continue;
         }
      }

      // If nothing to bind on this attribute, disable fetch of vertex attribute.
      cellGcmSetVertexDataArray(i, 0, 0, 0, CELL_GCM_VERTEX_F32, CELL_GCM_LOCATION_LOCAL, 0);
   }

   cellGcmSetInvalidateVertexCache();
}
#endif

//----------------------------------------------------------------------------
// Forces the cached vertex declaration to get reset

void CRenderBackend::ForceVertexDataRebind()
{
   mBoundVertexDataHash_1 = 0xDEADBEEF;
   mBoundVertexDataHash_2 = 0xFEE7C0DE;
   
   // This will cause ANY unused attributes to be disabled on the next call to SetVertexData
   mDisabledVertexAttributeMask = 0;
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderPrimitives(CMeshChunk::EPrimitive type, uint32 const /*vertexBufferOffset*/, uint32 const /*vertexCount*/, uint32 const indexBufferOffset, uint32 const indexCount)
{
#if BP_RENDER_GEOMETRY
   ShaderCache()->ValidateState();

   int indexBufferOffsetBytes = 0;
   
   if( BPE_BRANCH_HINT_FALSE(indexBufferOffset > 0) )
   {
      switch(mBoundIndexBufferType)
      {
      case CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16:
         indexBufferOffsetBytes = indexBufferOffset * 2;
         break;

      case CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32:
         indexBufferOffsetBytes = indexBufferOffset * 4;
         break;

      default:
         BPE_ASSERT(false, "invalid index type");
      }
   }

   cellGcmSetDrawIndexArray(gPrimitiveMapping[type], 
                            indexCount, 
                            mBoundIndexBufferType, 
                            mBoundIndexBufferLocation, 
                            mBoundIndexBuffer + indexBufferOffsetBytes);

#ifdef DEBUG_FIFO_HANGS
   GcmFinish();
#endif

   //mMetrics.mNumRenderPrimitiveCalls++;

#ifdef FLUSH_AFTER_RENDERCALLS
   cellGcmFlush();
#endif
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount)
{
#if BP_RENDER_GEOMETRY
   ShaderCache()->ValidateState();

   cellGcmSetDrawArrays(gPrimitiveMapping[type], vertexBufferOffset, vertexCount);

#ifdef DEBUG_FIFO_HANGS
   GcmFinish();
#endif

   mMetrics.mNumRenderPrimitiveCalls++;

#ifdef FLUSH_AFTER_RENDERCALLS
   cellGcmFlush();
#endif
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::BeginRenderPrimitivesCustom()
{
   ShaderCache()->ValidateState();
}

//----------------------------------------------------------------------------

void CRenderBackend::EndRenderPrimitivesCustom()
{
#ifdef FLUSH_AFTER_RENDERCALLS
   cellGcmFlush();
#endif

#ifdef DEBUG_FIFO_HANGS
   GcmFinish();
#endif

   mMetrics.mNumRenderPrimitiveCalls++;
}

//----------------------------------------------------------------------------

#pragma pack(push,1)
struct STGAHeader
{
   uint8    mIdentificationFieldSize;
   uint8    mColourMapType;
   uint8    mImageTypeCode;
   uint16   mColorMapOrigin;
   uint16   mColorMapLength;
   uint8    mColourMapEntrySize;
   uint16   mXOrigin;
   uint16   mYOrigin;
   uint16   mWidth;
   uint16   mHeight;
   uint8    mBPP;
   uint8    mImageDescriptorByte;
};
#pragma pack(pop)

#include "Engine/Streams/CDiskOutputStream.h"
#include "cell/rtc.h"

uint16 BigEndian_to_LittleEndian(uint16 val)
{
   return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}

void CRenderBackend::TakeScreenshot(int const screenShotJitterCount)
{
#if 0
   bool const bFirst = (screenShotJitterCount == 0);
   int const kNumPasses = 5;
   bool const bLast = (screenShotJitterCount == ((4 * kNumPasses) - 1));

   int pFullImageOffset = ((screenShotJitterCount & 1)) * 4 + ( (1 - ((screenShotJitterCount & 2) / 2)) * mFrameBufferSize.mDisplayWidth * 8);

   static uint16* spFullData = NULL;
   if (bFirst)
   {
      uint32 bufferSize = mFrameBufferSize.mDisplayWidth * mFrameBufferSize.mDisplayHeight * 4 * 4 * sizeof(uint16);
      spFullData = new uint16 [bufferSize];
      memset(spFullData, 0, bufferSize);
   }

   GcmWaitForIdle();

   int const bufferCount = BPE_ARRAY_SIZE(mDisplayBufferInfo);

   int bufferToUse = (mCurrentDisplayBuffer - 1);

   while(bufferToUse < 0 )
      bufferToUse += bufferCount;

   uint32* pSourceData = (uint32*)mDisplayBufferInfo[bufferToUse].mpTexture->GcmGetAddress();
   int const sourceDataPitch = mDisplayBufferInfo[bufferToUse].mpTexture->GcmGetPitch();

   // Copy into buffer
   for( int y = 0; y < mFrameBufferSize.mDisplayHeight; y++ )
   {
      uint32* pSrc = pSourceData + y * sourceDataPitch / 4;
      // Dest buffer is 4x larger, and is offset
      uint16 * pDest = spFullData + (y * 2 * mFrameBufferSize.mDisplayWidth * 8) + pFullImageOffset;
      for( int x = 0; x < mFrameBufferSize.mDisplayWidth; x++ )
      {
         uint32 const sourceValue = pSrc[0];
         
         // Accumulate
         pDest[0] += (sourceValue & 0x00FF0000) >> 16;
         pDest[1] += (sourceValue & 0x0000FF00) >> 8;
         pDest[2] += (sourceValue & 0x000000FF);
         pDest[3] += (sourceValue & 0xFF000000) >> 24;
         
         pSrc++;
         pDest += 8;         
      }
   }

   if (bLast)
   {
      // write data to disk
      CellRtcDateTime time;

      cellRtcGetCurrentClockLocalTime(&time);

      std::string const filename = CStringExtras::Stringize("$/%4.4d-%2.2d-%2.2d_%2.2d-%2.2d-%2.2d_%d.tga", 
         time.year, time.month, time.day, time.hour, time.minute, time.second, GetFrameCount());

      bpe_debugger_printf("Taking screenshot (%s)...\n", filename.c_str());

      CDiskOutputStream stream(CResourceManager::GetLocalPath(filename));

      STGAHeader header;
      memset(&header, 0, sizeof(STGAHeader));
      header.mImageTypeCode = 2;
      // http://www.gamers.org/dEngine/quake3/TGA.txt
      // bit 5 set means image origin is in upper left hand corner.
      header.mImageDescriptorByte = 1 << 5;

      // TGA file format is little endian!
      header.mWidth = BigEndian_to_LittleEndian(mFrameBufferSize.mDisplayWidth * 2);
      header.mHeight = BigEndian_to_LittleEndian(mFrameBufferSize.mDisplayHeight * 2);
      header.mBPP = 32;

      stream.Put(&header, sizeof(STGAHeader));

      uint32 totalSize = mFrameBufferSize.mDisplayWidth * mFrameBufferSize.mDisplayHeight * 4 * 4;

      for( int i = 0; i < totalSize; i += 4 )
      {
         uint16* pixel = spFullData + i;

         stream.WriteUint8((uint8) (pixel[2] / kNumPasses));
         stream.WriteUint8((uint8) (pixel[1] / kNumPasses));
         stream.WriteUint8((uint8) (pixel[0] / kNumPasses));
         stream.WriteUint8((uint8) (pixel[3] / kNumPasses));
      }

      delete [] spFullData;
   }
#endif
}

//----------------------------------------------------------------------------

void CRenderBackend::GetMemoryStats(ERenderMemory const memory, SRenderHWAllocatorStats * pStats) const
{
   switch(memory)
   {
   case kRM_Video:
      mpLocalAllocator->GetStats(pStats);
      break;
   
   case kRM_System:
      mpMainAllocator->GetStats(pStats);
      break;
   }
}

//----------------------------------------------------------------------------

CRenderHWAllocator::SHandle const * CRenderBackend::AllocFixed(int const size, int const alignment, ERenderMemory const memoryType, ERenderMemory const fallbackMemory)
{
   CRenderHWAllocator::SHandle const * handle = NULL;

   switch(memoryType)
   {
   case kRM_Video:
      handle = mpLocalAllocator->Alloc(size, alignment);
      break;
   case kRM_System:
      handle = mpMainAllocator->Alloc(size, alignment);
      break;
   }

   if( handle == NULL )
   {
      switch(fallbackMemory)
      {
      case kRM_Video:
         handle = mpLocalAllocator->Alloc(size, alignment);
         break;
      case kRM_System:
         handle = mpMainAllocator->Alloc(size, alignment);
         break;
      }
   }

   if( handle == NULL )
   {
      bpe_debugger_printf("Failed to allocate %d bytes (alignment: %d) in AllocFixed\n", size, alignment);

      float const kOneMeg = 1024 * 1024;

      SRenderHWAllocatorStats stats;

      RenderBackend()->GetMemoryStats(kRM_System, &stats);
      bpe_debugger_printf("RSX Main: Total %0.2f MB (%d) / Allocated %0.2f MB (%d) / Pending Free %0.2f MB (%d) / Free %0.2f MB (%d)\n", stats.mTotalSize / kOneMeg, stats.mTotalHandles, stats.mAllocatedSize / kOneMeg, stats.mAllocatedHandles, stats.mPendingFreeSize / kOneMeg, stats.mPendingFreeBlocks, (stats.mTotalSize - stats.mAllocatedSize) / kOneMeg, (stats.mTotalHandles - stats.mAllocatedHandles));

      RenderBackend()->GetMemoryStats(kRM_Video, &stats);
      bpe_debugger_printf("RSX Local: Total %0.2f MB (%d) / Allocated %0.2f MB (%d) / Pending Free %0.2f MB (%d) / Free %0.2f MB (%d)\n", stats.mTotalSize / kOneMeg, stats.mTotalHandles, stats.mAllocatedSize / kOneMeg, stats.mAllocatedHandles, stats.mPendingFreeSize / kOneMeg, stats.mPendingFreeBlocks, (stats.mTotalSize - stats.mAllocatedSize) / kOneMeg, (stats.mTotalHandles - stats.mAllocatedHandles));
   }

   return handle;
}

//----------------------------------------------------------------------------

void CRenderBackend::Free(CRenderHWAllocator::SHandle const * pHandle, int const delayedFree)
{
   BPE_ASSERT(pHandle != NULL, "Invalid handle passed to free");

   uint32 offset, location;
   GcmGetOffsetAndLocation(pHandle, &offset, &location);
   
   if( delayedFree )
   {
      int const framesToKeep = 3;//BPE_ARRAY_SIZE(mDisplayBufferInfo);

      switch(location)
      {
      case CELL_GCM_LOCATION_LOCAL:
         mpLocalAllocator->PendingFree(pHandle, framesToKeep);
         break;

      case CELL_GCM_LOCATION_MAIN:
         mpMainAllocator->PendingFree(pHandle, framesToKeep);
         break;
      }
   }
   else
   {
      switch(location)
      {
      case CELL_GCM_LOCATION_LOCAL:
         mpLocalAllocator->ImmediateFree(pHandle);
         break;

      case CELL_GCM_LOCATION_MAIN:
         mpMainAllocator->ImmediateFree(pHandle);
         break;
      }
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::AdjustSizeForZCull(int* pWidth, int *pHeight)
{
   *pWidth = (*pWidth + 63) & (~63);
   *pHeight = (*pHeight + 63) & (~63);
}

//----------------------------------------------------------------------------

uint32 sCompressionTagOffset = 0;

CRenderHWAllocator::SHandle const * CRenderBackend::AllocTiledRenderTarget(bool const isDepth, CBaseTexture::EAntiAliasType const aa, ERenderMemory const memoryType, int const width, int * pHeight, int * pPitch, bool const createZCull, int const zCullOffset)
{
   // search for a free tile
   int tile = 0;
   for(; tile < BPE_ARRAY_SIZE(mTiles); ++tile )
   {
      if( mTiles[tile].mMemory == NULL )
         break;
   }

   if( tile == BPE_ARRAY_SIZE(mTiles))
      return NULL;

   int const tiledPitch = cellGcmGetTiledPitchSize(*pPitch);
   int const tiledHeight = ((*pHeight) + 63) & ~63;

   int const size = tiledPitch * tiledHeight;
   int const tiledSize = (size + 0xFFFF) & ~0xFFFF;

   CRenderHWAllocator::SHandle const * pMemory = AllocFixed(tiledSize, 64 * 1024, memoryType);
   if( pMemory != NULL )
   {
      *pPitch = tiledPitch;
      *pHeight = tiledHeight;

      mTiles[tile].mMemory = pMemory;

      uint32 pOffset = 0;
      cellGcmAddressToOffset(pMemory->mpAddress, (uint32_t*)&pOffset);

      if( isDepth ) 
      {
         if( createZCull )
         {
            // Try to find free ZCull tile
            int zTile = 0;
            for( ; zTile < BPE_ARRAY_SIZE(mZCullTiles); ++zTile )
            {
               if( mZCullTiles[zTile].mMemory == NULL )
                  break;
            }

            // If we found a free ZCull tile, lets set it up.
            if( zTile != BPE_ARRAY_SIZE(mZCullTiles) )
            {
               uint32 aaMode;
               int zcullWidth = width;
               int zcullHeight = *pHeight;

               // This gets the AA mode and modifies the width and height for zcull
               // zcull acts on the viewport width and height, not the number of samples.  However,
               // the width and height coming in here are samples.
               switch ( aa )
               {
               case CBaseTexture::kAA_MSAA4x:
                  aaMode = CELL_GCM_SURFACE_SQUARE_ROTATED_4;
                  zcullWidth >>= 1;
                  zcullHeight >>= 1;
                  break;
               case CBaseTexture::kAA_MSAA2x:
                  aaMode = CELL_GCM_SURFACE_DIAGONAL_CENTERED_2;
                  zcullWidth >>= 1;
                  break;
               default:
                  aaMode = CELL_GCM_SURFACE_CENTER_1;
                  break;
               }

               // Round the widths and heights up to the next 64 byte boundary
               AdjustSizeForZCull(&zcullWidth, &zcullHeight);
               mZCullTiles[zTile].mMemory = pMemory;

               int32 result = cellGcmBindZcull(zTile, 
                                pOffset, 
                                zcullWidth, 
                                zcullHeight, 
                                zCullOffset,
                                CELL_GCM_ZCULL_Z24S8, 
                                aaMode, 
                                CELL_GCM_ZCULL_GREATER,
                                CELL_GCM_ZCULL_LONES, 
                                CELL_GCM_SCULL_SFUNC_GREATER, 
                                0x80, 
                                0xFF);

               if ( result != CELL_OK )
               {
                  bpe_debugger_printf( "WARNING: zCull failed, result code %d\n", result );
               }

            }
         }

         // Set up regular tile info with depth compression.
         uint32 compressionMode = CELL_GCM_COMPMODE_DISABLED;
         uint16 base = 0;

         switch ( aa )
         {
         case CBaseTexture::kAA_MSAA2x:
            compressionMode = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL;
            base = tiledSize / 0x10000; // HACK <- Need to manage these base addresss for compressed tiles!
            break;
         case CBaseTexture::kAA_MSAA4x:
            compressionMode = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED;
            base = tiledSize / 0x10000; // HACK <- Need to manage these base addresss for compressed tiles!
            break;
         }

         // Main memory buffers do not support compression.
         if( memoryType == kRM_System )
         {
            compressionMode = CELL_GCM_COMPMODE_DISABLED;
         }

         cellGcmSetTileInfo(tile, memoryType, pOffset, tiledSize, tiledPitch, compressionMode, base, 3);
      }
      else
      {
         // Set up regular tile info with no compression (compression on color targets is only for anti aliased targets)
         uint8_t compressionMode;
         switch ( aa )
         {
         case CBaseTexture::kAA_MSAA4x:
            compressionMode = CELL_GCM_COMPMODE_C32_2X2;
            break;
         case CBaseTexture::kAA_MSAA2x:
            compressionMode = CELL_GCM_COMPMODE_C32_2X1;
            break;
         default:
            compressionMode = CELL_GCM_COMPMODE_DISABLED;
            break;
         }

         // Main memory buffers do not support compression.
         if( memoryType == kRM_System )
         {
            compressionMode = CELL_GCM_COMPMODE_DISABLED;
         }

         cellGcmSetTileInfo(tile, memoryType, pOffset, tiledSize, tiledPitch, compressionMode, 0, 0);
      }

      cellGcmBindTile(tile);
   }

   return pMemory;
}

//----------------------------------------------------------------------------

void CRenderBackend::FreeTiledRenderTargetImmediate(CRenderHWAllocator::SHandle const * pMemory)
{
   GcmWaitForIdle();

   // unbind any zcull tiles
   for( int i = 0; i < BPE_ARRAY_SIZE(mZCullTiles); ++i )
   {
      if( mZCullTiles[i].mMemory == pMemory )
      {
         cellGcmUnbindZcull(i);
         mZCullTiles[i].mMemory = NULL;
      }
   }

   // unbind tiles
   for( int i = 0; i < BPE_ARRAY_SIZE(mTiles); ++i )
   {
      if( mTiles[i].mMemory == pMemory )
      {
         cellGcmUnbindTile(i);
         Free(pMemory, 0);
         mTiles[i].mMemory = NULL;
         break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CRenderBackend::GcmGetPrimitiveType(CMeshChunk::EPrimitive const type)
{
   return gPrimitiveMapping[type];
}

//----------------------------------------------------------------------------

void CRenderBackend::ClearAndSwapForOSD()
{
   CSyncCriticalSectionLocker lock(mCommandBufferCriticalSection);

   BeginScene();
   
   SetRenderTarget(SRenderTarget());
   Clear(kFlag_Color, CColor::Black(), 1.0f, 0);
   
   EndScene();
}

//----------------------------------------------------------------------------

class CDisableDebugSyncOff
{
#ifdef CELL_GCM_DEBUG
public:
   CDisableDebugSyncOff()
   {
      mOldDebugCallback = gCellGcmDebugCallback;
      gCellGcmDebugCallback = NULL;
   }

   ~CDisableDebugSyncOff()
   {
      gCellGcmDebugCallback = mOldDebugCallback;
   }

private:
   void (*mOldDebugCallback)(struct CellGcmContextData*);    // The old debug callback.
#endif
};

void* CRenderBackend::AddJTSBlocker()
{
   CDisableDebugSyncOff disable;

   // Make sure there is enough space for the JTS
   cell::Gcm::cellGcmReserveMethodSize(1);
   
   // Get address of JTS
   // We know the jump to self command is going to be at this location because we made sure it fit with the reserve above.
   void* pJTSAddress = gCellGcmCurrentContext->current;

   // Insert jump to self command
   uint32_t offset;
   cellGcmAddressToOffset(gCellGcmCurrentContext->current, &offset);

   cell::Gcm::Unsafe::cellGcmSetJumpCommand(offset);

   return pJTSAddress;
}

//----------------------------------------------------------------------------

CRenderHWAllocator::SHandle const* CRenderBackend::AllocateCommandBuffer(uint32 size)
{
   int commandBufferSize = size;

   // Add space for JTS that will be turned into JTN
   commandBufferSize += 4;

   // Add space for RETURN at very end
   commandBufferSize += 4;

   // Add space for cellGcmSetupContextData.
   // cellGcmSetupContextData will actually eat one command at the very end of the command buffer.
   // My only guess is that the function assumes there needs to be enough space at the end to insert a jump to the "next" command buffer in a normal command buffer setup?
   commandBufferSize += 4;

   // Align size for buffer to be 128 byte aligned, this will make SPU DMA easier.
   commandBufferSize = (commandBufferSize + 127) & (~127);

   // The alignment is a bit wasteful, but this will makes overwriting it from the SPU easier!
   CRenderHWAllocator::SHandle const * pMemory = AllocFixed(commandBufferSize, 128, kRM_System, kRM_System);
   BPE_VERIFY(pMemory != NULL, false, "Couldn't allocate callable command buffer.");

   // Initialize command buffer with some default data, so if the RSX consumes it before the caller (or SPU) has a chance to fill it, the RSX won't hang.
   cell::Gcm::UnsafeInline::CellGcmContext context;
   cellGcmSetupContextData(&context, (uint32_t*)pMemory->mpAddress, commandBufferSize, NULL);

   // Fill command buffer with 0's (NOPS)
   memset(pMemory->mpAddress, 0, commandBufferSize);

   // Insert jump to self at every 128 aligned block (NOTE: the allocation is 128 byte aligned making this a bit more straight forward)
   while( context.current < context.end )
   {
      // get offset to jump.
      uint32_t jumpOffset;
      cellGcmAddressToOffset(context.current, &jumpOffset);

      // write jump command to buffer (this will increment current by 1)
      cellGcmSetJumpCommandUnsafeInline(&context, jumpOffset);

      // advance 127 more words.
      context.current += (128 / 4) - 1;
   }

   // Insert return command at the very end.
   {
      context.current = context.end - 1;
      cellGcmSetReturnCommandUnsafeInline(&context);
   }

   return pMemory;
}

//----------------------------------------------------------------------------

void CRenderBackend::GcmIssueWaitForBackEndFinish()
{
   // set write label command in push buffer, and wait
   // NOTE: this is for RSX to wait
   cellGcmSetWriteBackEndLabel(mWaitLabel, mWaitLabelValue);
   cellGcmSetWaitLabel(mWaitLabel, mWaitLabelValue);

   // increment label value for next use
   ++mWaitLabelValue;
}

//----------------------------------------------------------------------------

void CRenderBackend::GcmWaitForIdle()
{
   // set write label command in push buffer, and wait
   // NOTE: this is for RSX to wait
   cellGcmSetWriteBackEndLabel(mWaitLabel, mWaitLabelValue);
   cellGcmSetWaitLabel(mWaitLabel, mWaitLabelValue);

   // increment label value for next use
   ++mWaitLabelValue;

   GcmFinish();
}

void CRenderBackend::GcmFinish()
{
#if BPE_USE_EDGE_GEOM
   // Submit any current edge job lists, otherwise we might end up in a deadlock between the RSX/SPU's
   SubmitCurrentEdgeJobList();
#endif

   // set write label command in push buffer
   cellGcmSetWriteBackEndLabel(mWaitLabel, mWaitLabelValue );
   // make sure the fifo is flushed
   cellGcmFlush();

   // wait on the ppu side for the label value to change
   while( *(cellGcmGetLabelAddress(mWaitLabel)) != mWaitLabelValue)
   {
      sys_timer_usleep(30);
   }

   // increment label value for next use
   ++mWaitLabelValue;
}

//----------------------------------------------------------------------------

void CRenderBackend::TryFlipImmediate()
{
   int result = sys_lwmutex_trylock(&mFlipMutex);

   if( result == CELL_OK ) 
   {
      // Unpack the RSX label
      unsigned int data = *(volatile unsigned int*)cellGcmGetLabelAddress(mFlipLabel);
      unsigned int bufferId = data >> 8;
      unsigned int flipId = data & 0xf;

      // Handle first frame and re-entry before RSX updates the label
      if( (flipId < 8) && (flipId != mLastFlipId) )
      {
         // check our presentation interval
         if( mVSyncCountUntilNextFlip <= 0 ) 
         {
            mMetrics.mTimeSinceLastFlip = (mPresentationInterval - mVSyncCountUntilNextFlip) / 60.0f;

            // save the current flip so we don't try again
            mLastFlipId = flipId;

            // flip it right now (this does not touch the command buffer)
            int result = cellGcmSetFlipImmediate(flipId);
            BPE_VERIFY(result != CELL_GCM_ERROR_FAILURE, false, "Invalid result from cellGcmSetFlipImmediate");

            //printf("VSync remaining count at flip: %d\n", mVSyncCountUntilNextFlip);

            // we can flip again in X vsyncs
            mVSyncCountUntilNextFlip = mPresentationInterval;
         }
      }

      // done with the mutex
      sys_lwmutex_unlock(&mFlipMutex);
   }
   else 
   {
      BPE_VERIFY(result == EBUSY, false, "Invalid state in TryFlipImmedate");
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::GcmVBlankHandler(uint32_t const /*head*/)
{
   CRenderBackend* pThis = (CRenderBackend*)gpRenderBackend;
   pThis->mVSyncCountUntilNextFlip--;

   pThis->mVBLCount++;

   pThis->TryFlipImmediate();
}

//----------------------------------------------------------------------------

void CRenderBackend::GcmFlipHandler(uint32_t const /*head*/)
{
   CRenderBackend* pThis = (CRenderBackend*)gpRenderBackend;
   pThis->mFlipsExecuted++;
}

//----------------------------------------------------------------------------

void CRenderBackend::InitializeSPU()
{
   InitializeSPUMemcpy();

   // Fragment program patching
   // Create Task set
   {
      uint8_t const kPriority[8] = { 0, 0, 0, 0, 8, 0, 0, 0 };

      CellSpursTasksetAttribute2 attributeTaskset;
      cellSpursTasksetAttribute2Initialize(&attributeTaskset);
      attributeTaskset.argTaskset = 0;
      memcpy(attributeTaskset.priority, kPriority, sizeof(kPriority));
      attributeTaskset.maxContention = 1;
      attributeTaskset.name = "Renderer";

      mpFragmentTaskSet.reset( (CellSpursTaskset2*)memalign(CELL_SPURS_TASKSET2_ALIGN, CELL_SPURS_TASKSET2_SIZE) );

      int ret = cellSpursCreateTaskset2(gpSpursManager->mpSPURS_Main.get(), mpFragmentTaskSet.get(), &attributeTaskset);
      if( ret != CELL_OK )
         printf("cellSpursCreateTasksetWithAttribute error: %d\n", ret);

   }

   // Create queue
   {
      mpFragmentRenderQueueBuffer.reset( (uint8*)memalign(16, kFragmentRenderQueueSize) );

      mpFragmentRenderQueue.reset( (CellSpursLFQueue*)memalign(CELL_SPURS_LFQUEUE_ALIGN, CELL_SPURS_LFQUEUE_SIZE) );
      memset(mpFragmentRenderQueue.get(), 0, CELL_SPURS_LFQUEUE_SIZE);

      cellSpursLFQueueInitialize(mpFragmentTaskSet.get(), mpFragmentRenderQueue.get(), mpFragmentRenderQueueBuffer.get(), kFragmentRenderQueueEntrySize, kFragmentRenderQueueDepth, CELL_SPURS_LFQUEUE_PPU2SPU);

      cellSpursLFQueueAttachLv2EventQueue(mpFragmentRenderQueue.get());
   }

   // Create task
   {
      mpFragmentTaskContextMemory.reset((uint8*)memalign(CELL_SPURS_TASK_CONTEXT_ALIGN, CELL_SPURS_TASK_CONTEXT_SIZE_ALL));

      CellSpursTaskArgument args;
      args.u64[0] = (uint64_t)mpFragmentRenderQueue.get();

      CellSpursTaskAttribute2 attr;
      cellSpursTaskAttribute2Initialize(&attr);
      attr.sizeContext = CELL_SPURS_TASK_CONTEXT_SIZE_ALL;
      attr.eaContext = (uint64_t)mpFragmentTaskContextMemory.get();
      attr.lsPattern = gCellSpursTaskLsAll;

      CellSpursTaskId       tid;

      int ret = cellSpursCreateTask2(mpFragmentTaskSet.get(), &tid, _binary_task_Renderer_SPU_elf_start, &args, &attr);

      if( ret != CELL_OK )
         printf("cellSpursCreateTask error: %d\n", ret);
   }
}

//----------------------------------------------------------------------------

void CRenderBackend::ShutdownSPU()
{
   // Fragment patching
   {
      // Send shutdown notification to task
      NRenderQueue::SRenderCmd cmd;
      cmd.mCmd = NRenderQueue::SRenderCmd::kCmd_Terminate;
      AddFragmentRenderCmd(&cmd);

      // Initiate shutdown of task set, this will wait for all tasks to complete.
      cellSpursShutdownTaskset(mpFragmentTaskSet.get());

      // Wait for shutdown to complete
      cellSpursJoinTaskset(mpFragmentTaskSet.get());

      cellSpursLFQueueDetachLv2EventQueue(mpFragmentRenderQueue.get());
   }

   ShutdownSPUMemcpy();
}

//----------------------------------------------------------------------------

void CRenderBackend::AddFragmentRenderCmd(NRenderQueue::SRenderCmd* pCommand)
{
   //BPE_VERIFY((int(pCommand) & 15) == 0, false, "Command is not 16 byte aligned");

   int ret = cellSpursLFQueuePush(mpFragmentRenderQueue.get(), pCommand);

   BPE_ASSERT(ret == CELL_OK, "cellSpursLFQueuePush failed for AddFragmentRenderCmd");
}

//----------------------------------------------------------------------------

void CRenderBackend::WaitForFragmentRenderQueueIdle()
{
   for( ;; )
   {
      uint32 size = 0;
      cellSpursLFQueueSize(mpFragmentRenderQueue.get(), (unsigned int*)&size);
      
      if( size == 0 )
         break;

      sys_timer_usleep(10);
   }
}

//----------------------------------------------------------------------------

int const CRenderBackend::AllocateRSXLabel()
{
   int const label = mRsxLabel;
   ++mRsxLabel;
   
   return label;
}

//----------------------------------------------------------------------------

int bpe_renderer_ready_to_flip()
{
   return gpRenderBackend->IsReadyToFlip() ? 1 : 0;
}

//----------------------------------------------------------------------------

bool CRenderBackend::IsReadyToFlip() const
{
   int const maxQueuedFlips = BPE_ARRAY_SIZE(mDisplayBufferInfo) - 1;

   if( (mFlipsQueued - mFlipsExecuted) > maxQueuedFlips )
   {
      return false;
   }
   else
   {
      return true;
   }
}

//----------------------------------------------------------------------------

bool CRenderBackend::HasMoreFrameBuffersToFlip() const
{
   int const maxQueuedFlips = BPE_ARRAY_SIZE(mDisplayBufferInfo) - 2;

   if( (mFlipsQueued - mFlipsExecuted) > maxQueuedFlips)
   {
      return false;
   }
   else
   {
      return true;
   }
}

//----------------------------------------------------------------------------

uint32 CRenderBackend::GetFrameBuffersToFlip() const
{
   return mFlipsQueued - mFlipsExecuted;
}

//----------------------------------------------------------------------------

namespace
{
   static uint32 skFogModeMap[] = 
   {
      CELL_GCM_FOG_MODE_LINEAR,
      CELL_GCM_FOG_MODE_EXP,
      CELL_GCM_FOG_MODE_EXP2
   };
}

//----------------------------------------------------------------------------

void CRenderBackend::SetFog( EFogMode const fogMode, real32 const nearZ, real32 const farZ, CColorf const &color )
{
   mFogColor = color;

   float fogScale = 1.0f/(farZ - nearZ);
   float fogParam0 = farZ * fogScale + 1.0f;
   float fogParam1 = -fogScale;

   cellGcmSetFogMode( skFogModeMap[ fogMode ] );
   cellGcmSetFogParams( fogParam0, fogParam1 );
}

//----------------------------------------------------------------------------

void CRenderBackend::SetProjectionMatrix(CMatrix4 const &matrix ) 
{ 
   mProjectionMatrix = matrix;
   FlushProjectionTimesViewMatrix();
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

namespace SPU_Memcpy
{
   typedef cell::Spurs::JobQueue::JobQueue<16> TMemcpyJobQueue;

   TMemcpyJobQueue*              pJobQueue = NULL;
   cell::Spurs::JobQueue::Port2* pPort = NULL;

   CellSpursJob128               memcpyJob;
}

extern const CellSpursJobHeader _binary_job_Job_Memcpy_SPU_jobbin2_jobheader;

void CRenderBackend::InitializeSPUMemcpy()
{
   using namespace SPU_Memcpy;

   pJobQueue = (TMemcpyJobQueue*)memalign(TMemcpyJobQueue::kAlign, TMemcpyJobQueue::kSize);

   uint8 const kPriority[] = { 8, 8, 8, 8, 8, 8, 0, 0};

   TMemcpyJobQueue::create(pJobQueue, gpSpursManager->mpSPURS_System.get(), "SPU memcpy", 1 /*numSpus*/, kPriority /*priority*/);

   pPort = (cell::Spurs::JobQueue::Port2*)memalign(CELL_SPURS_JOBQUEUE_PORT2_ALIGN, CELL_SPURS_JOBQUEUE_PORT2_SIZE);
   cell::Spurs::JobQueue::Port2::create(pPort, pJobQueue);

   memset(&memcpyJob, 0, sizeof(CellSpursJob128));
   memcpy(&memcpyJob.header, &_binary_job_Job_Memcpy_SPU_jobbin2_jobheader, sizeof(CellSpursJobHeader));;

   int const scratchSize = 64 * 1024;
   memcpyJob.header.sizeScratch = scratchSize >> 4; // size in number of quadwords
}

void CRenderBackend::ShutdownSPUMemcpy()
{
   using namespace SPU_Memcpy;
   
   pPort->destroy();
   
   free(pPort); 
   pPort = NULL;

   pJobQueue->shutdown();
   
   int exitCode;
   pJobQueue->join(&exitCode);
   
   free(pJobQueue);
   pJobQueue = NULL;
}

void CRenderBackend::SPU_SyncDMA(uint32 const destEA, uint32 const sourceEA, uint32 const size)
{
   using namespace SPU_Memcpy;

   memcpyJob.workArea.userData[0] = destEA;
   memcpyJob.workArea.userData[1] = sourceEA;
   memcpyJob.workArea.userData[2] = size;

   int const kTagId = 0;

   pPort->pushJob((CellSpursJobHeader*)&memcpyJob, sizeof(CellSpursJob128), kTagId, cell::Spurs::JobQueue::Port2::kFlagSyncJob);
   pPort->sync(kTagId);
}

//----------------------------------------------------------------------------

#include "cell/atomic.h"

namespace
{
   sys_ppu_thread_t                 gRenderThread;
   CRenderBackend::TFnRenderThread  gRenderThreadFunc = NULL;  
   int                              gRenderThreadParam;
   volatile int                     gRenderThreadAlive = 1;

   enum EEventFlag
   {
      kEF_IsReady = (1 << 0),
      kEF_IsDone  = (1 << 1)
   };

   sys_event_flag_t                 gRenderThreadEventFlag;
   int                              gIsSuspended = 0;
}

void CRenderBackend::RenderThread(uint64_t arg)
{
   CRenderBackend* pThis = (CRenderBackend*)arg;
   
   while(gRenderThreadAlive)
   {
      int result = ETIMEDOUT;
      
      if( !gIsSuspended )
      {
         uint64_t wakeupEventFlags;
         result = sys_event_flag_wait(gRenderThreadEventFlag, kEF_IsReady, SYS_EVENT_FLAG_WAIT_AND|SYS_EVENT_FLAG_WAIT_CLEAR_ALL, &wakeupEventFlags, 65 * 1000);
      }

      // Call render function
      {
         CSyncCriticalSectionLocker lock(pThis->mCommandBufferCriticalSection);

         if( result != ETIMEDOUT )
         {
            if( gRenderThreadFunc )
               gRenderThreadFunc(gRenderThreadParam);

            sys_event_flag_set(gRenderThreadEventFlag, kEF_IsDone);
         }
         else
         {
            if( gRenderThreadFunc )
               gRenderThreadFunc(-1);
         }
      }
   }

   sys_event_flag_set(gRenderThreadEventFlag, kEF_IsDone);

   sys_ppu_thread_exit(0);
}

//----------------------------------------------------------------------------

void CRenderBackend::StartThreadSystem(TFnRenderThread renderFunction)
{
   gRenderThreadFunc = renderFunction;

   // initialize event flag in "is done" status.
   sys_event_flag_attribute_t attr;
   sys_event_flag_attribute_initialize(attr);
   sys_event_flag_create(&gRenderThreadEventFlag, &attr, kEF_IsDone);

   int const kThreadPriority = 1001;
   int const kStackSize = 32 * 1024;

   sys_ppu_thread_create(&gRenderThread, RenderThread, (uint64_t)this, kThreadPriority, kStackSize, SYS_PPU_THREAD_CREATE_JOINABLE, "BP Render Thread");
}

//----------------------------------------------------------------------------

void CRenderBackend::ShutdownThreadSystem()
{
   gRenderThreadAlive = 0;

   uint64_t exitCode;
   sys_ppu_thread_join(gRenderThread, &exitCode);
}

//----------------------------------------------------------------------------

void CRenderBackend::RunThreadFunction(int nextParam)
{
   WaitThreadFunction();

#if ENABLE_SEPERATE_RENDER_THREAD
   BeginFrameResourceTick();
#endif

   gRenderThreadParam = nextParam;
   
   sys_event_flag_set(gRenderThreadEventFlag, kEF_IsReady);
}

//----------------------------------------------------------------------------

void CRenderBackend::WaitThreadFunction()
{
   uint64_t wakeupEventFlags;
   int result = sys_event_flag_wait(gRenderThreadEventFlag, kEF_IsDone, SYS_EVENT_FLAG_WAIT_AND, &wakeupEventFlags, SYS_NO_TIMEOUT);
}

//----------------------------------------------------------------------------

bool CRenderBackend::TryWaitThreadFunction()
{
   uint64_t wakeupEventFlags;
   sys_event_flag_trywait(gRenderThreadEventFlag, kEF_IsDone, SYS_EVENT_FLAG_WAIT_AND, &wakeupEventFlags);

   return wakeupEventFlags ? true : false;
}

//----------------------------------------------------------------------------

void CRenderBackend::SuspendGameRenderThread()
{
   ++gIsSuspended;
}

//----------------------------------------------------------------------------

void CRenderBackend::ResumeGameRenderThread()
{
   --gIsSuspended;
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

void CRenderBackend::BeginFrameResourceTick()
{
   InternalBeginFrameResourceTick();

   mpDynamicVertexBufferPool_RT->FrameReset();
   mpDynamicIndexBufferPool_RT->FrameReset();
   mpDynamicVertexBufferPool_UT->FrameReset();
   mpDynamicIndexBufferPool_UT->FrameReset();
}
