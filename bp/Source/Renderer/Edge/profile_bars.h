/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0 mlaa_preview_1
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef PROFILE_BARS
#define PROFILE_BARS

#include <sys/sys_time.h>
#include <cell/gcm.h>

class CellSpurs2;
class CellSpursTracePacket;

class ProfileBars
{
public:
   static void						Init(CellSpurs2* spurs1, uint32_t numSpus1, CellSpurs2* spurs2, uint32_t numSpus2, uint32_t maxTracePacketsPerSpu = 4096);
   
   static void                Enable(bool enable) {}

   static void						StartFrame(bool showBars);
   static void						AddPpuMarker(uint32_t color, const char* pName);
   static void						AddRsxMarker(uint32_t color, const char* pName);
   static void						AddRsxMarker(CellGcmContextData* context, uint32_t color, const char* pName);
   static void						Render();
   static void						Shutdown();
   static void						SetTargetFps( float targetFps );

   static int GetNumGPUMarker() 
   { 
      return mNumRsxMarkers;
   }
   static void GetGPUMarkerDisplayData(int iMarker, char const ** ppName, float* pTimeMS, float & previousTime)
   {
      assert( iMarker < GetNumGPUMarker() );

      ProfileBars::RsxMarker const & marker = ProfileBars::mRsxMarkers[iMarker];
      float delta = marker.time - previousTime;
      previousTime = marker.time;

      *ppName = marker.name.c_str();
      *pTimeMS = delta * 1000.0f;
   }
   static float GetGPUTotalTime()
   {
      float res = 0.0f;
      if( GetNumGPUMarker() )
      {
         ProfileBars::RsxMarker const & marker = ProfileBars::mRsxMarkers[ProfileBars::mNumRsxMarkers - 1];
         res = marker.time * 1000.0f;
      }
      return res;
   }

   static int GetNumCPU() 
   { 
      return 1;
   }
   static int GetNumCPUMarker(int iCPU) 
   { 
      assert( iCPU < GetNumCPU() );

      return ProfileBars::mNumPpuMarkers;
   }
   static void GetCPUMarkerDisplayData(int iCPU, int iMarker, char const ** ppName, float* pTimeMS, float & previousTime)
   {
      assert( iMarker < GetNumCPUMarker(iCPU) );

      ProfileBars::PpuMarker const & marker = ProfileBars::mPpuMarkers[iMarker];
      float delta = marker.time - previousTime;
      previousTime = marker.time;

      *ppName = marker.name.c_str();
      float ticksToMS = 1000.0f / sys_time_get_timebase_frequency();
      *pTimeMS = delta*ticksToMS;
   }

public:
   struct Vert
   {
      float		x, y, z;
      uint32_t	color;
   };

   struct JobInfo
   {
      uint64_t	guid;
      uint32_t	color;
      uint32_t	pad;
   };

   struct PpuMarker
   {
      uint32_t	time;
      uint32_t	color;
      std::string name;
   };

   struct RsxMarker
   {
      uint32_t	   color;
      float       time;
      std::string name;
   };

   static void						PrepareProfilerCommandBuffer( CellGcmContextData *ctx, bool showBars );
   static void						PrepareProfilerCommandBufferSPUTrace(  CellGcmContextData *ctx, int const firstSpuId, int const mNumSpus, uint32_t * mLastValidPackedIndex, void* mSpursTraceBuffer, float const incXPerTick, uint32_t const timebase, uint64_t const endFrameTime, Vert*& v, float const x0, float const x1, float & y, float const barHeight );

   static Vert*					AddQuad(Vert* v, float x0, float x1, float y0, float y1, uint32_t color);

   static const uint32_t			kMaxPpuMarkers = 128;
   static const uint32_t			kMaxRsxMarkers = 128;
   static const uint32_t			kCmdbufSize = 0xffff;

   static void*					   mSpursTraceBuffer1;
   static uint32_t					mNumSpus1;
   static uint32_t					mLastValidPackedIndex1[6];

   static void*					   mSpursTraceBuffer2;
   static uint32_t					mNumSpus2;
   static uint32_t					mLastValidPackedIndex2[6];

   static uint32_t					mMaxTracePacketsPerSpu;
   static void*					   mVertexBuffer;
   static void*					   mVertexBufferCur;
   static uint32_t					mVertexBufferSize;
   static CGprogram              mCGVertexProgram;
   static CGprogram              mCGFragmentProgram;
   static void*                  mVertexProgramUCode;
   static void*                  mFragmentProgramUCode;
   static uint32_t               mFragmentProgramOffset;
   static PpuMarker				   mPpuMarkers[kMaxPpuMarkers];
   static RsxMarker				   mRsxMarkers[kMaxRsxMarkers];
   static uint32_t					mNumPpuMarkers;
   static uint32_t					mNumRsxMarkers;
   static uint64_t					mFrameTimeStart;
   static float					   mFpsTarget;
   static uint32_t*				   mCmdAddr;
   static const char*            mRsxMarkerNames[kMaxRsxMarkers];
};

inline ProfileBars::Vert* ProfileBars::AddQuad(Vert* v, float x0, float x1, float y0, float y1, uint32_t color)
{
   assert((uint32_t)v + 0x40 - (uint32_t)mVertexBufferCur <= mVertexBufferSize);

   v[0].x = x0;	v[0].y = y0;	v[0].z = 0;		v[0].color = color;
   v[1].x = x1;	v[1].y = y0;	v[1].z = 0;		v[1].color = color;
   v[2].x = x1;	v[2].y = y1;	v[2].z = 0;		v[2].color = color;
   v[3].x = x0;	v[3].y = y1;	v[3].z = 0;		v[3].color = color;

   return v+4;
}

#endif
