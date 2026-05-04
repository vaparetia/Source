/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0 mlaa_preview_1
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <sys/sys_time.h>
#include <sys/time_util.h>
#include <cell/gcm.h>
#include <cell/spurs.h>
//#include "cellutil.h"
//#include "gcmutil.h"
#include "profile_bars.h"

#include "trace.h"

using namespace cell::Gcm;

void*					ProfileBars::mSpursTraceBuffer1;
uint32_t				ProfileBars::mLastValidPackedIndex1[];
uint32_t				ProfileBars::mNumSpus1;

void*					ProfileBars::mSpursTraceBuffer2;
uint32_t				ProfileBars::mLastValidPackedIndex2[];
uint32_t				ProfileBars::mNumSpus2;

uint32_t				ProfileBars::mMaxTracePacketsPerSpu;

void*					ProfileBars::mVertexBuffer;
void*					ProfileBars::mVertexBufferCur;
uint32_t				ProfileBars::mVertexBufferSize;
CGprogram               ProfileBars::mCGVertexProgram;
CGprogram               ProfileBars::mCGFragmentProgram;
void*                   ProfileBars::mVertexProgramUCode;
void*                   ProfileBars::mFragmentProgramUCode;
uint32_t                ProfileBars::mFragmentProgramOffset;
ProfileBars::PpuMarker	ProfileBars::mPpuMarkers[];
ProfileBars::RsxMarker	ProfileBars::mRsxMarkers[];
uint32_t				ProfileBars::mNumPpuMarkers;
uint32_t				ProfileBars::mNumRsxMarkers;
uint64_t				ProfileBars::mFrameTimeStart;
float					ProfileBars::mFpsTarget = 60.f;
uint32_t*				ProfileBars::mCmdAddr = NULL;


uint8_t vertexProgram[] = {
#include "vpprofileshader.h"
};

uint8_t fragmentProgram[] = {
#include "fpprofileshader.h"
};

uint64_t gSPUColors[] =
{
   0, 0x0000FFFF,
   0, 0x008000FF,
   0, 0x008080FF,
   0, 0x00FF00FF,
   0, 0x71AF57FF,
   0, 0x7D8BB5FF,
   0, 0x808000FF,
   0, 0x8855B6FF,
   0, 0x8FC1FFFF,
   0, 0xA4FC7EFF,
   0, 0xB37097FF,
   0, 0xB5B5B5FF,
   0, 0xE355B6FF,
   0, 0xE37349FF,
   0, 0xE3BF49FF,
   0, 0xFFA0D7FF,
   0, 0xFFFB8FFF,
   0, 0xFFFF00FF,
   0, 0xFFFFFFFF
};

int const kSPUColorCount = sizeof(gSPUColors) / (2 * sizeof(uint64_t));

uint32_t CONVERT_GUID_TO_COLOR(uint64_t guid)
{
   for( int i = 0; i < kSPUColorCount; ++i )
   {
      uint64_t const colorGuid = gSPUColors[i*2 + 0];

      if( !colorGuid || colorGuid == guid )
      {
         gSPUColors[i*2 + 0] = guid;
         return (uint32_t)gSPUColors[i*2 + 1];
      }
   }

   return 0xFF0000FF;
}

void ProfileBars::Init(CellSpurs2* spurs1, uint32_t numSpus1, CellSpurs2* spurs2, uint32_t numSpus2, uint32_t maxTracePacketsPerSpu)
{
   int res = 0;

   mMaxTracePacketsPerSpu = maxTracePacketsPerSpu;
   
   mNumSpus1 = numSpus1;
   mNumSpus2 = numSpus2;

   // init SPURS trace 1
   {
      uint32_t spursTraceBufferSize = sizeof(CellSpursTraceInfo) + maxTracePacketsPerSpu * mNumSpus1 * sizeof(CellSpursTracePacket);
      mSpursTraceBuffer1 = memalign(128, spursTraceBufferSize);
      memset(mSpursTraceBuffer1, 0, spursTraceBufferSize);
      cellSpursTraceInitialize(spurs1, mSpursTraceBuffer1, spursTraceBufferSize, CELL_SPURS_TRACE_MODE_FLAG_WRAP_BUFFER);
      cellSpursTraceStart(spurs1);
   }

   // init SPURS trace 2
   {
      uint32_t spursTraceBufferSize = sizeof(CellSpursTraceInfo) + maxTracePacketsPerSpu * mNumSpus2 * sizeof(CellSpursTracePacket);
      mSpursTraceBuffer2 = memalign(128, spursTraceBufferSize);
      memset(mSpursTraceBuffer2, 0, spursTraceBufferSize);
      res = cellSpursTraceInitialize(spurs2, mSpursTraceBuffer2, spursTraceBufferSize, CELL_SPURS_TRACE_MODE_FLAG_WRAP_BUFFER);
      res = cellSpursTraceStart(spurs2);
   }

   mNumPpuMarkers = 0;
   mNumRsxMarkers = 0;

   for ( unsigned int i = 0; i < mNumSpus1; ++i )
   {
      mLastValidPackedIndex1[i] = mMaxTracePacketsPerSpu - 1;
   }

   for ( unsigned int i = 0; i < mNumSpus2; ++i )
   {
      mLastValidPackedIndex2[i] = mMaxTracePacketsPerSpu - 1;
   }

   // allocate vertex buffers
   // max quads = 8 background quads + max ppu markers + max rsx markers + max spu markers
   unsigned maxQuads = 8 + kMaxPpuMarkers + kMaxPpuMarkers + 6/*numSpus*/ * mMaxTracePacketsPerSpu;
   mVertexBufferSize = maxQuads * 16 * 4;

   uint32_t meg = 1024*1024;
   uint32_t allocSize = ( ( kCmdbufSize + 2*mVertexBufferSize) + meg - 1) & ~(meg - 1);
   mCmdAddr = (uint32_t*)memalign(meg, allocSize);
   mVertexBuffer = mCmdAddr + ( kCmdbufSize / 4);
   uint32_t dummy;
   res = cellGcmMapMainMemory(mCmdAddr, allocSize, &dummy);
   mVertexBufferCur = mVertexBuffer;

   // init shaders
   mCGVertexProgram   = (CGprogram)(void*)vertexProgram;
   mCGFragmentProgram = (CGprogram)(void*)fragmentProgram;

   cellGcmCgInitProgram(mCGVertexProgram);
   cellGcmCgInitProgram(mCGFragmentProgram);

   // allocate video memory for fragment program
   unsigned int ucodeSize;
   void *ucode;
   cellGcmCgGetUCode(mCGFragmentProgram, &ucode, &ucodeSize);

#if 0 //BP
   mFragmentProgramUCode = (void*)cellGcmUtilAllocateLocalMemory(ucodeSize, 1024);
#else
   mFragmentProgramUCode = (void*)gpRenderBackend->AllocFixed(ucodeSize, 1024, kRM_Video)->mpAddress;
#endif

   cellGcmAddressToOffset(mFragmentProgramUCode, &mFragmentProgramOffset);

   memcpy(mFragmentProgramUCode, ucode, ucodeSize); 

   // get and copy 
   cellGcmCgGetUCode(mCGVertexProgram, &ucode, &ucodeSize);
   mVertexProgramUCode = ucode;
}

void ProfileBars::Shutdown()
{
   free(mSpursTraceBuffer1);
   free(mSpursTraceBuffer2);
   free(mCmdAddr);
}

// Note: RSX timestamp 0 must be written at the time of this call (e.g. at the start of the frame)
//       It is the responsibility of the caller to do this
void ProfileBars::StartFrame(bool showBars)
{
   // prepare command context to render previous frame profiler samples
   CellGcmContextData ctx;
   cellGcmSetupContextData( &ctx, mCmdAddr, kCmdbufSize, 0 );
   PrepareProfilerCommandBuffer( &ctx, showBars );
   cellGcmSetReturnCommand( &ctx );

   // reset for next frame
   mNumRsxMarkers = 0;

   cellGcmSetTimeStamp(0);
   cellGcmFlush();

   mNumPpuMarkers = 0;
   SYS_TIMEBASE_GET(mFrameTimeStart);
}

void ProfileBars::AddPpuMarker(uint32_t color, const char* pName)
{
   assert(mNumPpuMarkers < kMaxPpuMarkers);

   uint64_t time;
   SYS_TIMEBASE_GET(time);

   time -= mFrameTimeStart;

   mPpuMarkers[mNumPpuMarkers].time = (uint32_t)time;
   mPpuMarkers[mNumPpuMarkers].color = color;
   mPpuMarkers[mNumPpuMarkers].name = pName;

   mNumPpuMarkers++;
}

void ProfileBars::AddRsxMarker(uint32_t color, const char* pName)
{
   assert(mNumRsxMarkers < kMaxRsxMarkers);

   cellGcmSetTimeStamp(mNumRsxMarkers+1);

   mRsxMarkers[mNumRsxMarkers].color = color;
   mRsxMarkers[mNumRsxMarkers].name = pName;

   mNumRsxMarkers++;
}

void ProfileBars::AddRsxMarker(CellGcmContextData* context, uint32_t color, const char* pName)
{
   assert(mNumRsxMarkers < kMaxRsxMarkers);

   cellGcmSetTimeStamp(context, mNumRsxMarkers+1);

   mRsxMarkers[mNumRsxMarkers].color = color;
   mRsxMarkers[mNumRsxMarkers].name = pName;

   mNumRsxMarkers++;
}

void ProfileBars::SetTargetFps( float targetFps )
{
   mFpsTarget = targetFps;
}

static inline CellSpursTracePacket AtomicRead( const CellSpursTracePacket* pPacket )
{
   //In case an SPU is updating the SPURS Trace buffer at the same time as someone is
   //reading it, we need to make sure that the read of the CellSpursTracePacket is atomic.
   //A load with VMX is a 16-byte atomic read, so does exactly what we need.

   union
   {
      CellSpursTracePacket	packet;
      vector unsigned char	packetVec;
   } myUnion;

   myUnion.packetVec = vec_lvlx( 0, (const unsigned char*)pPacket );
   return myUnion.packet;
}

inline float clamp( float a, float min, float max )
{
   a = a > min ? a : min;
   return a < max ? a : max;
}

void ProfileBars::Render()
{
   // call profiler command buffer
   uint32_t cmdoffset;
   cellGcmAddressToOffset( mCmdAddr, &cmdoffset );
   cellGcmSetCallCommand( cmdoffset );
}

static inline bool IsInRange( uint32_t testValue, uint32_t startValue, uint32_t endValue )
{
   if ( ( testValue - startValue ) >= 0x80000000 )
   {
      return false;
   }

   if ( ( endValue - testValue ) >= 0x80000000 )
   {
      return false;
   }

   return true;
}

void ProfileBars::PrepareProfilerCommandBuffer( CellGcmContextData *ctx, bool showBars )
{
   // top left of screen = (0,0)
   // bottom right of screen = (1,1)
   float const x0 = 0.045f;
   float y0 = 0.88f;
   float const barWidth = 1 - 2*x0;
   float const barHeight = 0.00555f;

   float const x1 = x0 + barWidth;
   float ticksToSecs, incXPerTick;

   // swap vertex buffer
   mVertexBufferCur = mVertexBufferCur == mVertexBuffer ? (void*)((uint32_t)mVertexBuffer + mVertexBufferSize) : mVertexBuffer;

   Vert* v = (Vert*)mVertexBufferCur;

   // black background 
   for(uint32_t i=0; i<8; i++)
   {
      float y = y0 + barHeight*2*i;
      v = AddQuad(v, x0, x1, y, y+barHeight, 0x00000000);
   }

   // rsx bar
   float y = y0;
   ticksToSecs = 1.0f / 1000000000.0f;
   incXPerTick = barWidth * ticksToSecs * mFpsTarget;
   float lastX = x0;
   uint64_t t0 = cellGcmGetTimeStamp(0);
   for(uint32_t i=0; i<mNumRsxMarkers; i++)
   {
      uint32_t color = mRsxMarkers[i].color;
      uint32_t time = (uint32_t)(cellGcmGetTimeStamp(i+1) - t0);
      mRsxMarkers[i].time = time * ticksToSecs;
      float x = x0 + time * incXPerTick;
      v = AddQuad(v, lastX, x, y, y+barHeight, color);
      lastX = x;
   }
   y += barHeight*2;

   // ppu bar
   ticksToSecs = 1.0f / sys_time_get_timebase_frequency();
   incXPerTick = barWidth * ticksToSecs * mFpsTarget;
   lastX = x0;
   for(uint32_t i=0; i<mNumPpuMarkers; i++)
   {
      uint32_t time = mPpuMarkers[i].time;
      uint32_t color = mPpuMarkers[i].color;
      float x = x0 + time * incXPerTick;
      v = AddQuad(v, lastX, x, y, y+barHeight, color);
      lastX = x;
   }
   y += barHeight*2;

   y = y0 + barHeight*4;

   // spu bars
   ticksToSecs = 1.0f / sys_time_get_timebase_frequency();
   incXPerTick = barWidth * ticksToSecs * mFpsTarget;
   uint32_t timebase = (uint32_t)mFrameTimeStart;

   if( showBars )
   {
      // get end frame time
      uint64_t endFrameTime;
      SYS_TIMEBASE_GET(endFrameTime);

      PrepareProfilerCommandBufferSPUTrace(ctx, 0, mNumSpus1, mLastValidPackedIndex1, mSpursTraceBuffer1, incXPerTick, timebase, endFrameTime, v, x0, x1, y, barHeight);
      PrepareProfilerCommandBufferSPUTrace(ctx, mNumSpus1, mNumSpus2, mLastValidPackedIndex2, mSpursTraceBuffer2, incXPerTick, timebase, endFrameTime, v, x0, x1, y, barHeight);

      // draw the whole lot
      int numVerts = v - (Vert*)mVertexBufferCur;
      uint32_t vertsOfs;
      cellGcmSetBlendEnable( ctx, CELL_GCM_FALSE);
      cellGcmSetDepthTestEnable( ctx, CELL_GCM_FALSE);
      cellGcmSetVertexProgram( ctx, mCGVertexProgram, mVertexProgramUCode);
      cellGcmSetFragmentProgram( ctx, mCGFragmentProgram, mFragmentProgramOffset);
      cellGcmAddressToOffset( mVertexBufferCur, &vertsOfs);
      cellGcmSetVertexDataArray( ctx, 0, 0, 16, 3, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_MAIN, vertsOfs);
      cellGcmSetVertexDataArray( ctx, 1, 0, 16, 4, CELL_GCM_VERTEX_UB, CELL_GCM_LOCATION_MAIN, vertsOfs + 12);
      cellGcmSetDrawArrays( ctx, CELL_GCM_PRIMITIVE_QUADS, 0, numVerts);
   }
}

uint32_t gLastGuidColor[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

void	ProfileBars::PrepareProfilerCommandBufferSPUTrace(  CellGcmContextData *ctx, int const firstSpuId, int const mNumSpus, uint32_t * mLastValidPackedIndex, void* mSpursTraceBuffer, float const incXPerTick, uint32_t const timebase, uint64_t const endFrameTime, Vert*& v, float const x0, float const x1, float & y, float const barHeight )
{
   // parse SPURS trace
   for(unsigned int i=0; i<mNumSpus; i++)
   {
      CellSpursTracePacket lastJobPacket	= (CellSpursTracePacket){{0,0,0,0,0},{{0,0}}};;
      bool lastJobPacketIsValid			   = false;
      
      CellSpursTracePacket lastTaskPacket	= (CellSpursTracePacket){{0,0,0,0,0},{{0,0}}};;
      bool lastTaskPacketIsValid			   = false;

      CellSpursTracePacket lastJobQueuePacket	= (CellSpursTracePacket){{0,0,0,0,0},{{0,0}}};;
      bool lastJobQueuePacketIsValid			   = false;

      uint64_t lastGuid					      = 0LL;
      uint32_t & lastGuidColor            = gLastGuidColor[firstSpuId];


      unsigned int startMarker = ( mLastValidPackedIndex[i] + 1) % mMaxTracePacketsPerSpu;
      unsigned int endMarker = mLastValidPackedIndex[i];

      int insideJobQueue = 0;

      for( unsigned int j = startMarker; j != endMarker; j = ( j + 1) % mMaxTracePacketsPerSpu )
      {
         const CellSpursTracePacket* tracePacketBasePointer = (const CellSpursTracePacket*)((uint32_t)mSpursTraceBuffer + sizeof(CellSpursTraceInfo));
         CellSpursTracePacket thisPacket = AtomicRead( &tracePacketBasePointer[ i * mMaxTracePacketsPerSpu + j ] );

         if ( IsInRange( thisPacket.header.time, timebase, (uint32_t)endFrameTime))
         {
            switch(thisPacket.header.tag)
            {
            case CELL_TRACE_TAG_DISPATCH:
            case CELL_TRACE_TAG_RESUME:
               {
                  if( thisPacket.data.start.module[0] == 'J' &&
                      thisPacket.data.start.module[1] == 'O' &&
                      thisPacket.data.start.module[2] == 'B' &&
                      (thisPacket.data.start.module[3] == '0' || thisPacket.data.start.module[3] == '1') )
                  {
                     lastJobQueuePacket = thisPacket;
                     lastJobQueuePacketIsValid = true;
                  }
               }
               break;

            case CELL_TRACE_TAG_EXIT:
            case CELL_TRACE_TAG_YIELD:
            case CELL_TRACE_TAG_SLEEP:
               {
                  if(lastJobQueuePacketIsValid)
                  {
                     lastJobQueuePacketIsValid = false;

                     if(thisPacket.data.guid != lastGuid)
                     {
                        lastGuid = thisPacket.data.guid;
                        lastGuidColor = CONVERT_GUID_TO_COLOR(lastGuid);
                     }

                     if ( lastJobQueuePacket.header.time < thisPacket.header.time )
                     {
                        // job start/end
                        float m0 = (lastJobQueuePacket.header.time - timebase) * incXPerTick;
                        float m1 = (thisPacket.header.time - timebase) * incXPerTick;

                        v = AddQuad(v, x0+m0, x0+m1, y, y+barHeight, lastGuidColor);
                     }

                  }
               }
               break;

            case CELL_SPURS_TRACE_TAG_JOB:
               if ( thisPacket.data.job.jobDescriptor )
               {
                  lastJobPacket			= thisPacket;
                  lastJobPacketIsValid	= true;
               }
               else
               {
                  //pipeline stall
                  lastJobPacketIsValid	= false;
               }
               break;

            case CELL_SPURS_TRACE_TAG_GUID:

               // select a new color based on GUID
               if(thisPacket.data.rawData != lastGuid)
               {
                  lastGuid = thisPacket.data.rawData;
                  lastGuidColor = CONVERT_GUID_TO_COLOR(lastGuid);
               }

               if(lastJobPacketIsValid)
               {
                  if ( lastJobPacket.header.time < thisPacket.header.time )
                  {
                     // job start/end
                     float m0 = (lastJobPacket.header.time - timebase) * incXPerTick;
                     float m1 = (thisPacket.header.time - timebase) * incXPerTick;

                     v = AddQuad(v, x0+m0, x0+m1, y, y+barHeight, lastGuidColor);
                  }

                  lastJobPacketIsValid = false;

                  // mark as processed
                  mLastValidPackedIndex[i] = j;

               } 
               else if( lastTaskPacketIsValid)
               {

                  // Handle mapping/unmapping of new code inside an SPU task
                  if ( lastTaskPacket.header.time < thisPacket.header.time )
                  {
                     float m0 = incXPerTick * clamp( lastTaskPacket.header.time - timebase, .0f, endFrameTime - timebase);
                     float m1 = incXPerTick * clamp( thisPacket.header.time - timebase, .0f, endFrameTime - timebase);

                     v = AddQuad(v, x0+m0, x0+m1, y, y+barHeight, lastGuidColor);
                     lastTaskPacket = thisPacket;
                  }
               } 
               else 
               {

                  // a rough packet from last frame ?
                  float m1 = incXPerTick * clamp( thisPacket.header.time - timebase, .0f, endFrameTime - timebase);

                  v = AddQuad(v, x0, x0+m1, y, y+barHeight, lastGuidColor);
               }

               break;

            case CELL_SPURS_TRACE_TAG_TASK:
               switch(thisPacket.data.task.incident)
               {
               case CELL_SPURS_TRACE_TASK_DISPATCH:
                  lastTaskPacket			= thisPacket;
                  lastTaskPacketIsValid	= true;
                  break;

               case CELL_SPURS_TRACE_TASK_WAIT:
               case CELL_SPURS_TRACE_TASK_YIELD:
               case CELL_SPURS_TRACE_TASK_EXIT:
                  if(lastTaskPacketIsValid)
                  {
                     float m0 = incXPerTick * clamp( lastTaskPacket.header.time - timebase, .0f, endFrameTime - timebase);
                     float m1 = incXPerTick * clamp( thisPacket.header.time - timebase, .0f, endFrameTime - timebase);

                     v = AddQuad(v, x0+m0, x0+m1, y, y+barHeight, lastGuidColor);

                     lastTaskPacketIsValid = false;

                     // mark as processed
                     mLastValidPackedIndex[i] = j;
                  } 
                  else 
                  {

                     // a rough packet from last frame ?
                     float m1 = incXPerTick * clamp( thisPacket.header.time - timebase, .0f, endFrameTime - timebase);

                     v = AddQuad(v, x0, x0+m1, y, y+barHeight, lastGuidColor);
                  }
                  break;
               }
               break;
            }
         }
      }

      // close the last sample
      if ( lastTaskPacketIsValid )
      {
         float m0 = incXPerTick * clamp( lastTaskPacket.header.time - timebase, .0f, endFrameTime - timebase);

         v = AddQuad(v, x0+m0, x1, y, y+barHeight, lastGuidColor);

         lastTaskPacketIsValid = false;
      }

      y += barHeight*2;
   }
}
