//------------------------------------------------------------------------------------------
// CLinearCopyStream.h
// Bluepoint
// Copyright 2006
//
// Used for making deep copies of evaluators into a linear buffer.
// This is used for copying PPU data for use on the SPU.
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class ENGINE_API CLinearCopyStream
{
public:

   // Caller owns buffer
   CLinearCopyStream( int maxBufferSize, uint8 * pBuffer, int targetBufferOffset );
   ~CLinearCopyStream();

   // Returns aligned buffer pos before function call
   int Put(void const* pData, int size, int alignment = 4)
   { 
      int const alignedBufferPos = PadAndAlign(alignment);
      memcpy(mpBuffer + alignedBufferPos, pData, size);
      mCurrentBufferPos += size;
      return alignedBufferPos;
   }

   int Put(uint32 data, int alignment = 4)
   { 
      int const alignedBufferPos = PadAndAlign(alignment);
      int const size = sizeof(uint32);
      memcpy(mpBuffer + alignedBufferPos, &data, size);
      mCurrentBufferPos += size;
      return alignedBufferPos;
   }

   int PutPadding(int size)
   {
      while (size)
      {
         *(mpBuffer + mCurrentBufferPos) = 0xCD;   // Mark padded data
         mCurrentBufferPos++;
         size--;
      }
      BPE_ASSERT(mCurrentBufferPos < mMaxBufferSize, "Buffer overflow");
      return mCurrentBufferPos;
   }

   // Adds 'mTargetBufferOffset' to 'data'.
   int PutPointer(uint32 data, int alignment = 4)
   { 
      int const alignedBufferPos = PadAndAlign(alignment);
      int const size = sizeof(uint32);
      uint32 const dataPtrValue = data + mTargetBufferOffset;
      memcpy(mpBuffer + alignedBufferPos, &dataPtrValue, size);
      mCurrentBufferPos += size;
      return alignedBufferPos;
   }

   // Adds 'mTargetBufferOffset' to 'data' and stores at 'bufferPos'
   void SetPointer(uint32 data, int bufferPos)
   {
      BPE_ASSERT(bufferPos < mMaxBufferSize, "Buffer overflow");
      uint32 const dataPtrValue = data + mTargetBufferOffset;
      memcpy(mpBuffer + bufferPos, &dataPtrValue, sizeof(uint32));
   }

   void SetValue(uint32 data, int bufferPos)
   {
      BPE_ASSERT(bufferPos < mMaxBufferSize, "Buffer overflow");
      uint32 const dataPtrValue = data;
      memcpy(mpBuffer + bufferPos, &dataPtrValue, sizeof(uint32));
   }

   int PadAndAlign(int alignment = 4)
   {
      while ((mCurrentBufferPos % alignment) != 0)
      {
         *(mpBuffer + mCurrentBufferPos) = 0xCD;   // Mark padded data
         mCurrentBufferPos++;
      }
      BPE_ASSERT(mCurrentBufferPos < mMaxBufferSize, "Buffer overflow");
      return mCurrentBufferPos;
   }

   template<class X> void PutPODVector(std::vector<X> const &vec, int vectorDeclarationBufferPos, int alignment = 4)
   {
#if BPE_TARGET==BPE_TARGET_RVL
      BPE_VERIFY( false, false, "PutPODVector unsupported on RVL" );
#else

      // We're doing some nasty application specific stuff here.
      // If vector changes, then this code will need to be re-written.
      // Assumes vector layout is like this.
      /*
      Allocator info (8 bytes)   
      pointer _Myfirst;	// pointer to beginning of array
	   pointer _Mylast;	// pointer to current end of sequence
	   pointer _Myend;	// pointer to end of array
      */

      // Vector changes size based on debug modes
      // and platform
#if BPE_TARGET==BPE_TARGET_PS3

#  if _HAS_ITERATOR_DEBUGGING
      static const int kVectorSize = 5;
#  else
      static const int kVectorSize = 4;
#  endif

#elif BPE_TARGET==BPE_TARGET_VITA

#  if _HAS_ITERATOR_DEBUGGING
      static const int kVectorSize = 5;
#  else
      static const int kVectorSize = 4;
#  endif

#elif BPE_TARGET==BPE_TARGET_X360

#  if _HAS_ITERATOR_DEBUGGING
      static const int kVectorSize = 5;
#  else
      static const int kVectorSize = 4;
#  endif

#else

#  if _HAS_ITERATOR_DEBUGGING
      static const int kVectorSize = 5;
#  else
      static const int kVectorSize = 6;
#  endif

#endif

      BPE_CTASSERT(sizeof(std::vector<X>) == (sizeof(uint32) * kVectorSize));

      int vecDataBufferPos = 0;
      if (vec.size())
      {
         vecDataBufferPos = Put(&vec[0], vec.size() * sizeof(X));
         PutPadding((vec.capacity() - vec.size()) * sizeof(X));
      }
      else if (vec.capacity())
      {
         PadAndAlign(alignment);
         vecDataBufferPos = PutPadding(vec.capacity() * sizeof(X));
      }
      else
      {
         // Empty vector, set all to NULL
         SetValue(0, vectorDeclarationBufferPos + (sizeof(uint32) * (kVectorSize - 3)));
         SetValue(0, vectorDeclarationBufferPos + (sizeof(uint32) * (kVectorSize - 2)));
         SetValue(0, vectorDeclarationBufferPos + (sizeof(uint32) * (kVectorSize - 1)));
         return;
      }
      SetPointer(vecDataBufferPos, vectorDeclarationBufferPos + (sizeof(uint32) * (kVectorSize - 3)));
      SetPointer(vecDataBufferPos + (vec.size() * sizeof(X)), vectorDeclarationBufferPos + (sizeof(uint32) * (kVectorSize - 2)));
      SetPointer(vecDataBufferPos + (vec.capacity() * sizeof(X)), vectorDeclarationBufferPos + (sizeof(uint32) * (kVectorSize - 1)));
#endif // TARGET != RVL
   }

   uint32 BufferPosToBufferPointer(int bufferPos)
   {
      uint32 const bufferPtr = bufferPos + mTargetBufferOffset;
      return bufferPtr;
   }

public:
   int      mMaxBufferSize;
   uint8*   mpBuffer;
   int      mTargetBufferOffset;
   int      mCurrentBufferPos;
};

//----------------------------------------------------------------------------
