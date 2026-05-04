#include "StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "profile_bars.h"

bool ProfileBars::mNextFrameEnabled;
bool ProfileBars::mEnabled;

DWORD ProfileBars::mRawFrameCount;
DWORD ProfileBars::mCurrFrame;

LARGE_INTEGER ProfileBars::mCPUFrequency;
LARGE_INTEGER ProfileBars::mGPUFrequency;

int ProfileBars::mQuadCount;
IDirect3DDevice9* ProfileBars::mpD3DDevice;
void* ProfileBars::mVertexBuffer;
IDirect3DVertexDeclaration9* ProfileBars::mpVertexDecl;
IDirect3DVertexShader9* ProfileBars::mpVertexShader;
IDirect3DPixelShader9* ProfileBars::mpPixelShader;
void* ProfileBars::mVertexProgramUCode;
void* ProfileBars::mFragmentProgramUCode;

ProfileBars::Frame ProfileBars::mFrame[kMaxFrameCount];
ProfileBars::Frame* ProfileBars::mpCurrFrame;
ProfileBars::Frame* ProfileBars::mpLastFrame;
ProfileBars::Frame* ProfileBars::mpLastLastFrame;
ProfileBars::Frame* ProfileBars::mpLastLastLastFrame;
int ProfileBars::mCPUMarkersStackii[kMaxCPUProcessor];
int ProfileBars::mCPUMarkersStack[kMaxCPUProcessor][kMaxCPUMarkers];
int ProfileBars::mGPUMarkersStackii;
int ProfileBars::mGPUMarkersStack[kMaxGPUMarkers];  

float ProfileBars::mFpsTarget;

#define SAFE_RELEASE(x) if(x) { (x)->Release(); }
#define USE_SHADER_DWORDS 0

#if USE_SHADER_DWORDS

char vertexProgram[] = {
#include "profile_bars_VShader.h"
};

char fragmentProgram[] = {
#include "profile_bars_PShader.h"
};

#else

char hlslProgram[] = "                                               \
sampler2D TextureSampler : register(s0);                             \
                                                                     \
struct InV																		      \
{																				         \
	float4 Position : POSITION;													\
   float4 Color : COLOR0;                                            \
};																				         \
																				         \
struct OutV																		      \
{																				         \
	float4 Position : POSITION;													\
	float4 Color : COLOR0;        												\
};																				         \
																				         \
struct InP																		      \
{																				         \
	float4 Color : COLOR0;												         \
};																				         \
																				         \
OutV Main_V( InV In )															   \
{																				         \
	OutV Out = (OutV)0;															   \
																				         \
   float4 outPos = In.Position;                                      \
   outPos.y = 1.0f - outPos.y;                                       \
   outPos.xy *= float2(2.0f,2.0f);                                   \
   outPos.xy -= float2(1.0f,1.0f);                                   \
	Out.Position = outPos;													      \
	Out.Color = In.Color;											            \
																				         \
	return Out;																	      \
}																				         \
																				         \
float4 Main_P( InP In ) : COLOR													\
{																				         \
	float4 done = In.Color;                               				\
																				         \
	return done;																      \
}																				         \
";

#endif

void ProfileBars::Init(IDirect3DDevice9* pD3DDevice)
{
   int res = 0;

   mEnabled = false;
   
   mRawFrameCount = 0;
   mCurrFrame = 0;
   mpCurrFrame = &mFrame[3];
   mpLastFrame = &mFrame[2];
   mpLastLastFrame = &mFrame[1];
   mpLastLastLastFrame = &mFrame[0];
   mFpsTarget = 60.0f;

   mQuadCount = 0;
   mpD3DDevice = pD3DDevice;

   //500 MHz
   //From pix timings 32,884 cycles == 66 microsec, TODO: must be a constant somewhere in SDK
   //1,000,000 microseconds
   //gpu cycles are EndValues.RBBM[0].QuadPart
   QueryPerformanceFrequency( &mCPUFrequency );
   mGPUFrequency.QuadPart = (LONGLONG)(1000000.0f/66.0f*32884.0f);

   mpCurrFrame->Clear();
   // create PrimUP buffer
   mVertexBuffer = malloc(kVertexBufferSize);


#if USE_SHADER_DWORDS
   assert(0 ==1); // TODO
#else
   ID3DXBuffer* pShaderBuffer = NULL;
   ID3DXBuffer* pErrorBuffer = NULL;
   {

      if( FAILED( D3DXCompileShader( hlslProgram, sizeof(hlslProgram), NULL, NULL, "Main_V", "vs_3_0", 0, &pShaderBuffer, &pErrorBuffer, NULL ) ) )
      {
         char* errTxt = (char*)pErrorBuffer->GetBufferPointer();
         SAFE_RELEASE( pShaderBuffer );
         SAFE_RELEASE( pErrorBuffer );
         BPE_VERIFYA(false, "X360RenderBackend failed to Init! Failed to compile default system vertex shader Main_V!");
         return;
      }
      DWORD* pShader = (DWORD*)pShaderBuffer->GetBufferPointer();
      DWORD shaderSize = pShaderBuffer->GetBufferSize()/sizeof(DWORD);
      {
         if( FAILED(mpD3DDevice->CreateVertexShader( pShader, &mpVertexShader ) ) )
         {
            SAFE_RELEASE( pShaderBuffer );
            SAFE_RELEASE( pErrorBuffer );
            BPE_VERIFYA(false, "Failed to create default system vertex shader Main_V");
            return;
         }
      }
   }
   {
      ID3DXBuffer* pShaderBuffer = NULL;
      ID3DXBuffer* pErrorBuffer = NULL;
      if( FAILED( D3DXCompileShader( hlslProgram, sizeof(hlslProgram), NULL, NULL, "Main_P", "ps_3_0", 0, &pShaderBuffer, &pErrorBuffer, NULL ) ) )
      {
         SAFE_RELEASE( pShaderBuffer );
         SAFE_RELEASE( pErrorBuffer );
         BPE_VERIFYA(false, "X360RenderBackend failed to Init! Failed to compile default system vertex shader Main_V!");
         return;
      }
      DWORD* pShader = (DWORD*)pShaderBuffer->GetBufferPointer();
      DWORD shaderSize = pShaderBuffer->GetBufferSize()/sizeof(DWORD);
      {
         if( FAILED(mpD3DDevice->CreatePixelShader( pShader, &mpPixelShader ) ) )
         {
            SAFE_RELEASE( pShaderBuffer );
            SAFE_RELEASE( pErrorBuffer );
            BPE_VERIFYA(false, "Failed to create default system pixel shader Main_P");
            return;
         }
      }
   }
   SAFE_RELEASE( pShaderBuffer );
   SAFE_RELEASE( pErrorBuffer );

#define CUSTOM_COLOR MAKED3DDECLTYPE(GPUVERTEXFORMAT_8_8_8_8, GPUENDIAN_8IN32, GPUVERTEXSIGN_UNSIGNED, GPUNUMFORMAT_FRACTION, GPUSWIZZLE_RGBA)
   D3DVERTEXELEMENT9 VertexElements[] =
   {
      { 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
      { 0, 12, CUSTOM_COLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
      D3DDECL_END()
   };
   mpD3DDevice->CreateVertexDeclaration( VertexElements, &mpVertexDecl );
#endif
   for( int iFrame=0;iFrame<kMaxFrameCount;++iFrame )
   {
      Frame& refFrame = mFrame[iFrame];
      for( int iGPU=0;iGPU<kMaxGPUProcessor;++iGPU )
      {
         GPUProcessor& refGPU = refFrame.mGPUProcessor[iGPU];
         mpD3DDevice->CreatePerfCounters( &refGPU.m_pPerfCounterStart, 1 );
         mpD3DDevice->CreatePerfCounters( &refGPU.m_pPerfCounterEnd, 1 );
         
         for( int iGPUMark=0;iGPUMark<kMaxGPUMarkers;++iGPUMark )
         {
            GPUMarker& refGPUMarker = refGPU.mGPUMarker[iGPUMark];
            mpD3DDevice->CreatePerfCounters( &refGPUMarker.m_pPerfCounterStart, 1 );
            mpD3DDevice->CreatePerfCounters( &refGPUMarker.m_pPerfCounterEnd, 1 );
         }
      }
   }
   mpD3DDevice->EnablePerfCounters( TRUE );
   // Enable the performance counters we care about.
   D3DPERFCOUNTER_EVENTS PerfEvents;
   ZeroMemory( &PerfEvents, sizeof( D3DPERFCOUNTER_EVENTS ) );
   // CP clock cycles.
   PerfEvents.CP[0] = GPUPE_CP_COUNT;
   // NRT busy cycles.
   PerfEvents.RBBM[0] = GPUPE_RBBM_NRT_BUSY;
   // Texture and vertex cache reads from system memory.
   PerfEvents.MH[0] = GPUPE_TC0_READ;
   PerfEvents.MH[1] = GPUPE_TC1_READ;
   PerfEvents.MH[2] = GPUPE_VC0_READ_MEMORY;
   // Vertex cache performance.
   PerfEvents.VC[0] = GPUPE_CC_HITS;
   PerfEvents.VC[1] = GPUPE_CC_MISSES;
   // Texture cache performance.
   PerfEvents.TCF[0] = GPUPE_TAG_HITS;
   PerfEvents.TCF[1] = GPUPE_TAG_MISSES;
   // SQ stuff.
   PerfEvents.SQ[0] = GPUPE_SQ_CONSTANTS_SENT_SP_SIMD0;
   PerfEvents.SQ[1] = GPUPE_SQ_CONSTANTS_USED_SIMD0;
   mpD3DDevice->SetPerfCounterEvents( &PerfEvents, 0 );
}

void ProfileBars::Shutdown()
{
   free(mVertexBuffer);
   mVertexBuffer = NULL;
   SAFE_RELEASE( mpVertexDecl );
   SAFE_RELEASE( mpVertexShader );
   SAFE_RELEASE( mpPixelShader );
}

void ProfileBars::StartFrame(bool /*showBars*/)
{
   mEnabled = mNextFrameEnabled;
   if( mEnabled )
   {
      PrepareProfilerCommandBuffer();
      
      // reset for next frame
      ++mRawFrameCount;
      mCurrFrame = mRawFrameCount % kMaxFrameCount;
      mpLastLastLastFrame = mpLastLastFrame;
      mpLastLastFrame = mpLastFrame;
      mpLastFrame = mpCurrFrame;
      mpCurrFrame = &mFrame[mCurrFrame];
      mpCurrFrame->Clear();
      QueryPerformanceCounter( &mpCurrFrame->mFrameTimeStart );

      // Insert dummy marker at beginning of frame to that when the first real marker occurs it has something to pop off.
      for( int i = 0; i < kMaxCPUProcessor; ++i )
         CPUMarkerPushProcessor(0, "", i);
   }
}

void ProfileBars::CPUMarkerPush(unsigned int color, const char* pName)
{
   DWORD pn = GetCurrentProcessorNumber();
   CPUMarkerPushProcessor(color, pName, pn);
}

void ProfileBars::CPUMarkerPushProcessor(unsigned int color, const char* pName, int processor)
{
   if( mEnabled )
   {
      CPUProcessor * __restrict pCPUProcessor = &mpCurrFrame->mCPUProcessor[processor];
      int * __restrict pTotalUsedCount = &pCPUProcessor->mTotalUsedCount;
      if( *pTotalUsedCount < kMaxCPUMarkers-1 )
      {
         CPUMarker * __restrict pCPUMarker = &pCPUProcessor->mCPUMarker[*pTotalUsedCount];

         QueryPerformanceCounter( &pCPUMarker->startTime );
         //       pCPUMarker->color = color;
         //       pCPUMarker->pName = pName;
         //       pCPUMarker->mpStartFrame = mpCurrFrame;
         // Push on stack index for this processor
         int * __restrict pCPUMarkersStackii = &mCPUMarkersStackii[processor];
         mCPUMarkersStack[processor][*pCPUMarkersStackii] = *pTotalUsedCount;
         ++*pCPUMarkersStackii;

         ++*pTotalUsedCount;
      }
   }
}

void ProfileBars::CPUMarkerPop(unsigned int color, const char* pName)
{
   if( mEnabled )
   {
      DWORD pn = GetCurrentProcessorNumber();

      // Pop stack index off for this processor
      int * __restrict pCPUMarkersStackii = &mCPUMarkersStackii[pn];
      if( *pCPUMarkersStackii > 0 )
      {
         --*pCPUMarkersStackii;
         int * __restrict pCPUMarkersStack = &mCPUMarkersStack[pn][*pCPUMarkersStackii];

         CPUProcessor * __restrict pCPUProcessor = &mpCurrFrame->mCPUProcessor[pn];
         CPUMarker * __restrict pCPUMarker = &pCPUProcessor->mCPUMarker[*pCPUMarkersStack];
         QueryPerformanceCounter( &pCPUMarker->endTime );
         pCPUMarker->color = color;
         pCPUMarker->name = pName;
         pCPUMarker->mpStartFrame = mpCurrFrame;
      }
   }
}

void ProfileBars::GPUMarkerPre()
{
   if( mEnabled )
   {
      GPUProcessor * __restrict pGPUProcessor = &mpCurrFrame->mGPUProcessor[0];
      mpD3DDevice->QueryPerfCounters(pGPUProcessor->m_pPerfCounterStart, 0);
      QueryPerformanceCounter( &pGPUProcessor->CPUStartTime );

      // Insert dummy marker at beginning of frame to that when the first real marker occurs it has something to pop off.
      GPUMarkerPush(0, "");
   }
}

void ProfileBars::GPUMarkerPost()
{
   if( mEnabled )
   {
      GPUProcessor * __restrict pGPUProcessor = &mpCurrFrame->mGPUProcessor[0];
      mpD3DDevice->QueryPerfCounters(pGPUProcessor->m_pPerfCounterEnd, 0);
      QueryPerformanceCounter( &pGPUProcessor->CPUEndTime );
   }
}

void ProfileBars::GPUMarkerPush(unsigned int color, const char* pName)
{
   if( mEnabled )
   {
      DWORD pn = GetCurrentProcessorNumber();

      GPUProcessor * __restrict pGPUProcessor = &mpCurrFrame->mGPUProcessor[0];
      int * __restrict pTotalUsedCount = &pGPUProcessor->mTotalUsedCount;
      if( *pTotalUsedCount < kMaxGPUMarkers-1 )
      {
         GPUMarker * __restrict pGPUMarker = &pGPUProcessor->mGPUMarker[*pTotalUsedCount];

         mpD3DDevice->QueryPerfCounters(pGPUMarker->m_pPerfCounterStart, 0);
         //       pGPUMarker->color = color;
         //       pGPUMarker->pName = pName;
         // Push on stack index for this processor
         int * __restrict pGPUMarkersStackii = &mGPUMarkersStackii;
         mGPUMarkersStack[*pGPUMarkersStackii] = *pTotalUsedCount;
         ++*pGPUMarkersStackii;

         ++*pTotalUsedCount;
      }
   }
}

void ProfileBars::GPUMarkerPop(unsigned int color, const char* pName)
{
   if( mEnabled )
   {
      DWORD pn = GetCurrentProcessorNumber();

      // Pop stack index off for this processor
      int * __restrict pGPUMarkersStackii = &mGPUMarkersStackii;
      if( *pGPUMarkersStackii > 0 )
      {
         --*pGPUMarkersStackii;
         int * __restrict pGPUMarkersStack = &mGPUMarkersStack[*pGPUMarkersStackii];

         GPUProcessor * __restrict pGPUProcessor = &mpCurrFrame->mGPUProcessor[0];
         GPUMarker * __restrict pGPUMarker = &pGPUProcessor->mGPUMarker[*pGPUMarkersStack];
         mpD3DDevice->QueryPerfCounters(pGPUMarker->m_pPerfCounterEnd, 0);
         pGPUMarker->color = color;
         pGPUMarker->name = pName;
      }
   }
}

void ProfileBars::SetTargetFps( float targetFps )
{
   mFpsTarget = targetFps;
}

inline float clamp( float a, float min, float max )
{
   a = a > min ? a : min;
   return a < max ? a : max;
}

void ProfileBars::Render()
{
   if( mEnabled )
   {
      PIXBeginNamedEvent(0xFFFFFFFF, "ProfileBars");

      if( mQuadCount )
      {
         // set renderstates
         RenderBackend()->GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
         RenderBackend()->GetCachedDeviceWrapper()->SetRenderState(D3DRS_ALPHATESTENABLE , FALSE);
         RenderBackend()->GetCachedDeviceWrapper()->SetRenderState(D3DRS_HALFPIXELOFFSET, TRUE);
         RenderBackend()->GetCachedDeviceWrapper()->SetVertexShader(mpVertexShader);
         RenderBackend()->GetCachedDeviceWrapper()->SetPixelShader(mpPixelShader);
         RenderBackend()->GetCachedDeviceWrapper()->SetVertexDeclaration(mpVertexDecl);

         RenderBackend()->GetCachedDeviceWrapper()->DrawPrimitiveUP(D3DPT_RECTLIST, mQuadCount, mVertexBuffer, sizeof(Vert) );

         // restore renderstate
         RenderBackend()->GetCachedDeviceWrapper()->SetRenderState(D3DRS_HALFPIXELOFFSET, FALSE);
      }

      PIXEndNamedEvent();
   }
}

static inline bool IsInRange( unsigned int testValue, unsigned int startValue, unsigned int endValue )
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

void ProfileBars::PrepareProfilerCommandBuffer()
{
   mQuadCount = 0;
   //misc GPU update
   //mpD3DDevice->QueryPerfCounters( m_pPerfCounterEnd[ mFrameCount % 3 ], 0 );

   // top left of screen = (0,0)
   // bottom right of screen = (1,1)
   float const x0 = 0.045f;
   float y0 = 0.88f;
   float const barWidth = 1 - 2*x0;
   float const barHeight = 0.00555f;

   float const x1 = x0 + barWidth;
   Vert* v = (Vert*)mVertexBuffer;

   // black background 
   for(unsigned int i=0; i<kMaxCPUProcessor; i++)
   {
      float y = y0 + barHeight*2*i;
      v = AddQuad(v, x0, x1, y, y+barHeight, 0);
   }
   for(unsigned int i=0; i<kMaxGPUProcessor; i++)
   {
      float y = y0 + barHeight*kMaxCPUProcessor*2 + barHeight*2*i;
      v = AddQuad(v, x0, x1, y, y+barHeight, 0);
   }
   // GPU bar
   float y;
   float ticksToSecsGPU = 1.0f / mGPUFrequency.QuadPart;
   float incXPerTickGPU = barWidth * ticksToSecsGPU * mFpsTarget;
   float ticksToSecsCPU = 1.0f / mCPUFrequency.QuadPart;
   float incXPerTickCPU = barWidth * ticksToSecsCPU * mFpsTarget;

   y = y0;
   {
      GPUProcessor & refGPUProcessor = mpLastLastFrame->mGPUProcessor[0];
      D3DPERFCOUNTER_VALUES frameStartValues;
      refGPUProcessor.m_pPerfCounterStart->GetValues( &frameStartValues, 0, NULL );
      D3DPERFCOUNTER_VALUES frameEndValues;
      refGPUProcessor.m_pPerfCounterEnd->GetValues( &frameEndValues, 0, NULL );

//       refGPUProcessor.GPUStartTime.QuadPart = frameStartValues.RBBM[0].QuadPart;
//       refGPUProcessor.GPUEndTime.QuadPart = frameEndValues.RBBM[0].QuadPart;
      refGPUProcessor.GPUStartTime.QuadPart = frameStartValues.CP[0].QuadPart;
      refGPUProcessor.GPUEndTime.QuadPart = frameEndValues.CP[0].QuadPart;

      int & refTotalUsedCount = refGPUProcessor.mTotalUsedCount;
      for(int i=0; i<refTotalUsedCount-1; i++)
      {
         GPUMarker & refGPUMarker = refGPUProcessor.mGPUMarker[i];
         D3DPERFCOUNTER_VALUES startValues;
         refGPUMarker.m_pPerfCounterStart->GetValues( &startValues, 0, NULL );
         D3DPERFCOUNTER_VALUES endValues;
         refGPUMarker.m_pPerfCounterEnd->GetValues( &endValues, 0, NULL );

//          refGPUMarker.startTime.QuadPart = startValues.RBBM[0].QuadPart;
//          refGPUMarker.endTime.QuadPart = endValues.RBBM[0].QuadPart;
//          refGPUMarker.startTime.QuadPart -= refGPUProcessor.GPUStartTime.QuadPart;
//          refGPUMarker.endTime.QuadPart -= refGPUProcessor.GPUStartTime.QuadPart;
         refGPUMarker.startTime.QuadPart = startValues.CP[0].QuadPart;
         refGPUMarker.endTime.QuadPart = endValues.CP[0].QuadPart;
         refGPUMarker.startTime.QuadPart -= refGPUProcessor.GPUStartTime.QuadPart;
         refGPUMarker.endTime.QuadPart -= refGPUProcessor.GPUStartTime.QuadPart;

         unsigned int timeStart = (unsigned int)refGPUMarker.startTime.QuadPart;
         unsigned int timeEnd = (unsigned int)refGPUMarker.endTime.QuadPart;

         unsigned int color = refGPUMarker.color;
         float x1 = x0 + timeStart * incXPerTickGPU;
         float x2 = x0 + timeEnd * incXPerTickGPU;
         v = AddQuad(v, x1, x2, y, y+barHeight, color);
      }
      // just in case a mistake is made reset stack index at this time
      mGPUMarkersStackii = 0;
   }
   // CPU bar
   y = y0 + barHeight*2;
   for(unsigned int iProcessor=0; iProcessor<kMaxCPUProcessor-1; ++iProcessor )
   {
      CPUProcessor & refCPUProcessor = mpLastLastFrame->mCPUProcessor[iProcessor];
      int & refTotalUsedCount = refCPUProcessor.mTotalUsedCount;
      for(int i=0; i<refTotalUsedCount-1; i++)
      {
         CPUMarker & refCPUMarker = refCPUProcessor.mCPUMarker[i];
         refCPUMarker.startTime.QuadPart -= mpLastLastFrame->mFrameTimeStart.QuadPart;
         refCPUMarker.endTime.QuadPart -= mpLastLastFrame->mFrameTimeStart.QuadPart;
         unsigned int timeStart = (unsigned int)refCPUMarker.startTime.QuadPart;
         unsigned int timeEnd = (unsigned int)refCPUMarker.endTime.QuadPart;
         unsigned int color = refCPUMarker.color;
         float x1 = x0 + timeStart * incXPerTickCPU;
         float x2 = x0 + timeEnd * incXPerTickCPU;
         v = AddQuad(v, x1, x2, y, y+barHeight, color);
      }
      y += barHeight*2;
      // just in case a mistake is made reset stack index at this time
      mCPUMarkersStackii[iProcessor] = 0;
   }
   /*
   D3DPERFCOUNTER_VALUES StartValues;
   m_pPerfCounterStart[ ( mFrameCount + 1 ) % 3 ]->GetValues( &StartValues, 0, NULL );
   D3DPERFCOUNTER_VALUES EndValues;
   m_pPerfCounterEnd[ ( mFrameCount + 1 ) % 3 ]->GetValues( &EndValues, 0, NULL );

   // Subtract start values from end values.
   UINT64* pStartValues = ( UINT64* )&StartValues;
   UINT64* pEndValues = ( UINT64* )&EndValues;
   const DWORD dwCount = sizeof( D3DPERFCOUNTER_VALUES ) / sizeof( UINT64 );
   for( DWORD i = 0; i < dwCount; ++i )
   {
      pEndValues[i] -= pStartValues[i];
   }
   //From pix timings 32,884 cycles == 66 microsec, TODO: must be a constant somewhere in SDK
   //1,000,000 microseconds
   //gpu cycles are EndValues.RBBM[0].QuadPart
   */
}
