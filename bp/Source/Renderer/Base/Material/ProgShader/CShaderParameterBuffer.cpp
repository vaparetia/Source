//------------------------------------------------------------------------------------------
// CShaderParameterBuffer.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CShaderParameterBuffer.h"

//----------------------------------------------------------------------------

#include "Engine/System/CStopWatch.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include "cell/gcm.h"
#include "cell/atomic.h"
#include "Renderer/Base/Backend/PS3/PS3CTexture.h"
#include "Renderer/Renderer_SPU/NRenderQueue.h"
#endif

//------------------------------------------------------------------------------------------

#if (BPE_TARGET == BPE_TARGET_PS3)
   #define BPE_ALIGN_16 __attribute__((aligned(16)))
#else
   #define BPE_ALIGN_16
   #define cellAtomicNop32(x) (*x)
#endif

//------------------------------------------------------------------------------------------

namespace
{
   inline void copy_with_doubles( void *dst, void const *src, int num_doubles )
   {
      double *pDoubleDst = reinterpret_cast<double *>( dst );
      double const *pDoubleSrc = reinterpret_cast<double const *>( src );

      for ( int i = 0; i < num_doubles; ++i )
      {
         pDoubleDst[i] = pDoubleSrc[i];
      }
   }
}

//------------------------------------------------------------------------------------------

namespace NShaderParameterBufferAllocator
{
   // these are used to verify correct usage of the functions
   uint8*         sRegionBeginPtr = NULL;
   bool           sBufferWaitingForFinalize = false;

   // maximum size of a region
   int const      kRegionReserveSize = 64 * 1024;
   // total size of ring buffer
   int const      kParameterRingBufferSize = 512 * 1024;

   BPE_CTASSERT(kRegionReserveSize <= kParameterRingBufferSize);
   
   uint8          sFrameParameterBuffer[kParameterRingBufferSize];
   // this is the write ptr of the ppu side
   uint8*         sCurrentPtr = sFrameParameterBuffer;
   // points to end of parameter buffer
   uint8* const   sEndPtr = sFrameParameterBuffer + kParameterRingBufferSize;
   // this is the consumed ptr from the spu side, everything prior up to this 
   // point has been used by the SPU and is ready for reuse by the PPU
   uint8*         sFreePtr BPE_ALIGN_16 = sFrameParameterBuffer;
};

//----------------------------------------------------------------------------

void CShaderParameterBufferAllocator::BeginRegion()
{
   using namespace NShaderParameterBufferAllocator;

   // If this region would go past the end of the buffer, we wrap around
   if( (sCurrentPtr + kRegionReserveSize) > sEndPtr )
      sCurrentPtr = sFrameParameterBuffer;

   CStopWatch watch;
   bool didStall = false;

   // Stall for enough room to become available
   while(true)
   {
#if BPE_TARGET == BPE_TARGET_PS3
      uint8* pFreePtr = (uint8*)cellAtomicNop32((uint32_t*)&sFreePtr);
#else
      uint8* pFreePtr = (uint8*)cellAtomicNop32(&sFreePtr);
#endif

      int remainingSize = pFreePtr - sCurrentPtr;

      if( remainingSize <= 0 )
         remainingSize += kParameterRingBufferSize;

      // Update peak size metric
      RenderBackend()->Metrics().mParameterBufferRingBufferPeakSize = bpe::max_val(RenderBackend()->Metrics().mParameterBufferRingBufferPeakSize, kParameterRingBufferSize - remainingSize);

      // If we have enough room available we're done here.
      if( kRegionReserveSize < remainingSize )
         break;

#if BPE_TARGET == BPE_TARGET_PS3
      sys_timer_usleep(5);
#endif
      didStall = true;
   }

   if( didStall )
      RenderBackend()->Metrics().mParameterBufferRingBufferStallUS += watch.GetElapsedMicroSeconds();

   sRegionBeginPtr = sCurrentPtr;
}

//----------------------------------------------------------------------------

CShaderParameterBuffer const CShaderParameterBufferAllocator::AllocateParameterBuffer()
{
   using namespace NShaderParameterBufferAllocator;

   BPE_VERIFY(sBufferWaitingForFinalize == false, false, "Must finalize last buffer before allocating a new one");
   BPE_VERIFY(sRegionBeginPtr != NULL, false, "Must start region before allocating parameter buffer");

   sBufferWaitingForFinalize = true;

   // because we use these buffers on the SPU we align them to 16 bytes
   uint8* pNewAddress = (uint8*)(uint32(sCurrentPtr + 15) & (~15));

   return CShaderParameterBuffer(pNewAddress);
}

//----------------------------------------------------------------------------

void CShaderParameterBufferAllocator::FinalizeParameterBuffer(CShaderParameterBuffer const & parameterBuffer)
{
   using namespace NShaderParameterBufferAllocator;

   BPE_VERIFY(sBufferWaitingForFinalize == true, false, "Can't finalize buffer without allocating one first");

   sCurrentPtr = parameterBuffer.GetCurrentParameterPtr();
   sBufferWaitingForFinalize = false;
}

//----------------------------------------------------------------------------

void CShaderParameterBufferAllocator::EndRegion()
{
   using namespace NShaderParameterBufferAllocator;

   BPE_VERIFY(sBufferWaitingForFinalize == false, false, "Must not have buffer waiting for finalize");

   int const usedRegionSize = sCurrentPtr - sRegionBeginPtr;
   BPE_VERIFY(usedRegionSize < kRegionReserveSize, false, "Region reserve size not big enough!");

   sRegionBeginPtr = NULL;

#if BPE_TARGET == BPE_TARGET_PS3
   // Execute cmd on spu to update free ptr, note that commands are processed in order, 
   // so this is guaranteed to occur AFTER the preceding commands which use the buffers.
   NRenderQueue::SRenderCmd cmd;
   cmd.mCmd = NRenderQueue::SRenderCmd::kCmd_FreeParameterBufferRegion;
   cmd.mFreeParameterBufferRegionParams.mFreePtrEA = (uint32)&sFreePtr;
   cmd.mFreeParameterBufferRegionParams.mFreePtrValue = (uint32)sCurrentPtr;

   gpRenderBackend->AddFragmentRenderCmd(&cmd);

#else
   // On WIN32 we don't have asynchronous processing of render queue commands, so we can update the free ptr immediately.
   sFreePtr = sCurrentPtr;
#endif
}

//------------------------------------------------------------------------------------------

CShaderParameterBuffer::CShaderParameterBuffer(uint8* pParameters)
:  mpStartAddress(reinterpret_cast<uint32*>(pParameters))
,  mpParameters(reinterpret_cast<uint32*>(pParameters))
{
}

//----------------------------------------------------------------------------

void CShaderParameterBuffer::AddTexture(uint32 const parameterCRC, CBaseTexture const * pTexture, bool const clampU, bool const clampV, bool const srgb)
{
   // write crc
   *mpParameters++ = parameterCRC;

#if BPE_TARGET == BPE_TARGET_PS3
   *mpParameters++ = sizeof(CellGcmTexture) + 3 * sizeof(uint32);

   // write srgb value
   *mpParameters++ = (uint32)srgb;

   // write clampU value
   *mpParameters++ = (uint32)clampU;

   // write clampV value
   *mpParameters++ = (uint32)clampV;

   // write texture structure
   CTexture const* pTex = (CTexture const*)pTexture;
   memcpy(mpParameters, &pTex->mTexture, sizeof(CellGcmTexture));
   mpParameters += sizeof(CellGcmTexture) >> 2;
#else
   // write size
   *mpParameters++ = 2 * sizeof(uint32);

   // write texture ptr
   *mpParameters++ = (uint32)pTexture;

   // write srgb value
   *mpParameters++ = (uint32)srgb;
#endif
}

//----------------------------------------------------------------------------

void CShaderParameterBuffer::AddParameter(uint32 const parameterCRC, CVector4 const & value)
{
   int const kNumElements = 4;

   // write crc
   *mpParameters++ = parameterCRC;

   // write size
   *mpParameters++ = kNumElements * sizeof(real32);

   // write data
   copy_with_doubles(mpParameters, &value, kNumElements >> 1);

   mpParameters += kNumElements;
}

//----------------------------------------------------------------------------

void CShaderParameterBuffer::AddParameter(uint32 const parameterCRC, CMatrix34 const & value)
{
   int const kNumElements = 12;

   // write crc
   *mpParameters++ = parameterCRC;

   // write size
   *mpParameters++ = kNumElements * sizeof(real32);

   real32* pDest = (real32*)mpParameters;
   real32 const * pData = (real32 const*)&value;

   pDest[0] = pData[0];
   pDest[1] = pData[3];
   pDest[2] = pData[6];
   pDest[3] = pData[9];
   pDest[4] = pData[1];
   pDest[5] = pData[4];
   pDest[6] = pData[7];
   pDest[7] = pData[10];
   pDest[8] = pData[2];
   pDest[9] = pData[5];
   pDest[10] = pData[8];
   pDest[11] = pData[11];

   mpParameters += kNumElements;
}

//----------------------------------------------------------------------------

void CShaderParameterBuffer::AddParameter(uint32 const parameterCRC, CMatrix4 const & value)
{
   int const kNumElements = 16;

   // write crc
   *mpParameters++ = parameterCRC;

   // write size
   *mpParameters++ = kNumElements * sizeof(real32);

   real32* pDest = (real32*)mpParameters;
   real32 const * pData = (real32 const*)&value;

   pDest[0] = pData[0];
   pDest[1] = pData[4];
   pDest[2] = pData[8];
   pDest[3] = pData[12];
   pDest[4] = pData[1];
   pDest[5] = pData[5];
   pDest[6] = pData[9];
   pDest[7] = pData[13];
   pDest[8] = pData[2];
   pDest[9] = pData[6];
   pDest[10] = pData[10];
   pDest[11] = pData[14];
   pDest[12] = pData[3];
   pDest[13] = pData[7];
   pDest[14] = pData[11];
   pDest[15] = pData[15];

   mpParameters += kNumElements;
}

//----------------------------------------------------------------------------

void CShaderParameterBuffer::PutTo(COutputStream & stream) const
{
   // NOTE: this function will not write out the correctly swapped data if a texture was added.
   // But this doesn't make any sense anyway (to write out a texture and then store is to disk!)

   for( uint32* pCurrentPtr = mpStartAddress; pCurrentPtr != mpParameters; ++pCurrentPtr )
   {
      stream.WriteUint32(*pCurrentPtr);
   }
}
