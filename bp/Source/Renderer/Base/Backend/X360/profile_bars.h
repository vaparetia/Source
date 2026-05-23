#ifndef PROFILE_BARS_X360
#define PROFILE_BARS_X360

#include "Engine/StlExtras/BPEStlExtras.h"

class ProfileBars
{
public:
   static void Init(IDirect3DDevice9* pD3DDevice);

   static void Enable(bool enable)
   { 
      mNextFrameEnabled = enable;
   }

   static void StartFrame(bool showBars);

   static void CPUMarkerPushProcessor(unsigned int color, const char* pName, int processor);
   static void CPUMarkerPush(unsigned int color, const char* pName);
   static void CPUMarkerPop(unsigned int color, const char* pName);

   static void GPUMarkerPre();
   static void GPUMarkerPost();
   static void GPUMarkerPush(unsigned int color, const char* pName);
   static void GPUMarkerPop(unsigned int color, const char* pName);

   static void Render();
   static void Shutdown();
   static void SetTargetFps(float targetFps);

   static LARGE_INTEGER GetCPUFrequency() { return mCPUFrequency; }
   static LARGE_INTEGER GetGPUFrequency() { return mGPUFrequency; }

   static int GetNumGPUMarker() 
   { 
      return bpe::max_val(mpLastLastFrame->mGPUProcessor[0].mTotalUsedCount-1,0);
   }
   static void GetGPUMarkerDisplayData(int iMarker, char const ** ppName, float* pTimeMS, float & previousTime)
   {
      assert( iMarker < GetNumGPUMarker() );

      GPUMarker & refGPUMarker = mpLastLastFrame->mGPUProcessor[0].mGPUMarker[iMarker];

      *ppName = refGPUMarker.name.c_str();
      float ticksToMS = 1000.0f / ProfileBars::GetGPUFrequency().QuadPart;
      *pTimeMS = (refGPUMarker.endTime.QuadPart - refGPUMarker.startTime.QuadPart)*ticksToMS;
   }
   static float GetGPUTotalTime()
   {
      float res = 0.0f;
      if( GetNumGPUMarker() )
      {
         GPUMarker & refGPUMarkerEnd = mpLastLastFrame->mGPUProcessor[0].mGPUMarker[GetNumGPUMarker()-1];
         GPUMarker & refGPUMarkerStart = mpLastLastFrame->mGPUProcessor[0].mGPUMarker[0];

         float ticksToMS = 1000.0f / ProfileBars::GetGPUFrequency().QuadPart;
         res = (refGPUMarkerEnd.endTime.QuadPart - refGPUMarkerStart.startTime.QuadPart)*ticksToMS;
      }
      return res;
   }

   static int GetNumCPU() 
   { 
      return kMaxCPUProcessor;
   }
   static int GetNumCPUMarker(int iCPU) 
   { 
      assert( iCPU < GetNumCPU() );

      return bpe::max_val(mpLastLastFrame->mCPUProcessor[iCPU].mTotalUsedCount-1, 0);
   }
   static void GetCPUMarkerDisplayData(int iCPU, int iMarker, char const ** ppName, float* pTimeMS, float & previousTime)
   {
      assert( iMarker < GetNumCPUMarker(iCPU) );

      CPUMarker & refGPUMarker = mpLastLastFrame->mCPUProcessor[iCPU].mCPUMarker[iMarker];

      *ppName = refGPUMarker.name.c_str();
      float ticksToMS = 1000.0f / ProfileBars::GetCPUFrequency().QuadPart;
      *pTimeMS = (refGPUMarker.endTime.QuadPart - refGPUMarker.startTime.QuadPart)*ticksToMS;
   }

public:

   static const unsigned int kMaxFrameCount = 5;
   // TODO: Experiment with keeping around more frames and graph views
   // might want to optimize format to reduce memory footprint, perhaps some kind of preregister Marker command
   // and then will only have to keep processor number, start/end times around
   static const unsigned int kMaxCPUProcessor = 6;
   static const unsigned int kMaxGPUProcessor = 1;
   static const unsigned int kMaxCPUMarkers = 128;
   static const unsigned int kMaxGPUMarkers = 128;

   struct Vert
   {
      float x, y, z;
      unsigned int color;
   };

   struct Frame;
   struct CPUMarker
   {
      unsigned int color;
      LARGE_INTEGER startTime;
      LARGE_INTEGER endTime;
      std::string name;
      Frame* mpStartFrame;
   };

   struct CPUProcessor
   {
      CPUMarker mCPUMarker[kMaxCPUMarkers];
      int mTotalUsedCount;

      void Clear()
      {
         mTotalUsedCount = 0;
      }
   };

   struct GPUMarker
   {
      unsigned int color;
      LARGE_INTEGER startTime;
      LARGE_INTEGER endTime;
      std::string name;
      D3DPerfCounters* m_pPerfCounterStart;
      D3DPerfCounters* m_pPerfCounterEnd;
      Frame* mpStartFrame;
   };

   struct GPUProcessor
   {
      GPUMarker mGPUMarker[kMaxGPUMarkers];
      int mTotalUsedCount;

      D3DPerfCounters* m_pPerfCounterStart;
      D3DPerfCounters* m_pPerfCounterEnd;
      LARGE_INTEGER GPUStartTime;
      LARGE_INTEGER GPUEndTime;
      LARGE_INTEGER CPUStartTime;
      LARGE_INTEGER CPUEndTime;

      void Clear()
      {
         mTotalUsedCount = 0;
      }
   };

   struct Frame
   {
      LARGE_INTEGER mFrameTimeStart;

      CPUProcessor mCPUProcessor[kMaxCPUProcessor];
      GPUProcessor mGPUProcessor[kMaxGPUProcessor];

      void Clear()
      {
         for( int ii=0;ii<kMaxCPUProcessor;++ii )
         {
            mCPUProcessor[ii].Clear();
         }
         for( int ii=0;ii<kMaxGPUProcessor;++ii )
         {
            mGPUProcessor[ii].Clear();
         }
      }
   };

   static void PrepareProfilerCommandBuffer();

   static Vert* AddQuad(Vert* v, float x0, float x1, float y0, float y1, unsigned int color);

   //Drawn as D3DPT_RECTLIST (v3 is implicitly defined)
   static const unsigned int kVertexBufferClearSize = (kMaxGPUProcessor + kMaxCPUProcessor)*sizeof(Vert)*3;
   static const unsigned int kVertexBufferDrawSize = (kMaxGPUProcessor*kMaxGPUMarkers + kMaxCPUProcessor*kMaxCPUMarkers)*sizeof(Vert)*3;
   static const unsigned int kVertexBufferSize = kVertexBufferClearSize + kVertexBufferDrawSize;

   //TODO: combine into flags?
   static bool mNextFrameEnabled;
   static bool mEnabled;
   static bool mFirstStartOfMain;

   static DWORD mRawFrameCount;
   static DWORD mCurrFrame;

   static LARGE_INTEGER mCPUFrequency;
   static LARGE_INTEGER mGPUFrequency;

   static int mQuadCount;
   static IDirect3DDevice9* mpD3DDevice;
   static void* mVertexBuffer;
   static IDirect3DVertexDeclaration9* mpVertexDecl;
   static IDirect3DVertexShader9* mpVertexShader;
   static IDirect3DPixelShader9* mpPixelShader;
   static void* mVertexProgramUCode;
   static void* mFragmentProgramUCode;

   static Frame mFrame[kMaxFrameCount];
   static Frame* mpCurrFrame;
   static Frame* mpLastFrame;
   static Frame* mpLastLastFrame;
   static Frame* mpLastLastLastFrame;
   static int mCPUMarkersStackii[kMaxCPUProcessor];
   static int mCPUMarkersStack[kMaxCPUProcessor][kMaxCPUMarkers];
   static int mGPUMarkersStackii;
   static int mGPUMarkersStack[kMaxGPUMarkers];  
   
   static float mFpsTarget;
};

inline ProfileBars::Vert* ProfileBars::AddQuad(Vert* v, float x0, float x1, float y0, float y1, unsigned int color)
{
   assert( (unsigned int)v + 3*sizeof(Vert) <= (unsigned int)mVertexBuffer + kVertexBufferSize );

   v[0].x = x0;	
   v[0].y = y0;	
   v[0].z = 0;		
   v[0].color = color;

   v[1].x = x1;	
   v[1].y = y0;	
   v[1].z = 0;		
   v[1].color = color;

   v[2].x = x1;	
   v[2].y = y1;	
   v[2].z = 0;		
   v[2].color = color;

   ++mQuadCount;
   return v+3;
}

#endif
