//----------------------------------------------------------------------------
// CInputStream.cpp
// Bluepoint
// Copyright 2001
//----------------------------------------------------------------------------

#include "Stdafx.h"
#include "Engine/Streams/CInputStream.h"

//----------------------------------------------------------------------------

CInputStream::CInputStream(uint32 const bufferSize)
:  mBufferOffset(0)
,  mBufferLength(0)
,  mBufferSize(bufferSize)
,  mpBuffer(new uint8[bufferSize])
,  mOwnsBuffer(true)
,  mReadBytes(0)
{
   BPE_ASSERT(bufferSize > 0, "CInputStream buffer size must be greater than zero.");      
}

//----------------------------------------------------------------------------

CInputStream::CInputStream(void * pBuffer, uint32 const bufferSize, bool const bTakesOwnership)
:  mBufferOffset(0)
,  mBufferLength(bufferSize)
,  mBufferSize(bufferSize)
,  mpBuffer((uint8*)pBuffer)
,  mOwnsBuffer(bTakesOwnership)
,  mReadBytes(0)
{
   BPE_ASSERT(bufferSize > 0, "CInputStream buffer size must be greater than zero.");      
}

//----------------------------------------------------------------------------

CInputStream::~CInputStream()
{
   if (mOwnsBuffer)
   {
      delete mpBuffer;
   }
}

//----------------------------------------------------------------------------

void CInputStream::Get(void * pDest, uint32 const numBytes)
{
   // TODO: Add explicit Skip function.
   if (pDest)
   {
      uint32 remainingBytesToRead = numBytes;
      uint32 destOffset = 0;

      for (;;)
      {
         // How much do we have left in our stream buffer?
         uint32 const numBufferBytes = mBufferLength - mBufferOffset;

         // Read as many bytes as possible from buffer
         if (remainingBytesToRead <= numBufferBytes)
         {
            // We have enough bytes in stream buffer for this read.
            // Copy from internal buffer to dest.
            memcpy( reinterpret_cast<uint8 *>(pDest) + destOffset, mpBuffer + mBufferOffset, remainingBytesToRead );
            mBufferOffset += remainingBytesToRead;
            break;
         }
         else
         {
            // Copy from internal buffer to dest
            memcpy( reinterpret_cast<uint8 *>(pDest) + destOffset, mpBuffer + mBufferOffset, numBufferBytes );
            remainingBytesToRead -= numBufferBytes;
            destOffset += numBufferBytes;
            // No bytes left in buffer, read in next block from stream to refill buffer.
            GetNextBlock();
         }
      }
   }
   else
   {
      // No dest pointer, skip over data
      uint32 remainingBytesToRead = numBytes;
      for (;;)
      {
         // How much do we have left in our stream buffer?
         uint32 const numBufferBytes = mBufferLength - mBufferOffset;

         // Read as many bytes as possible from buffer
         if (remainingBytesToRead <= numBufferBytes)
         {
            // We have enough bytes in stream buffer for this read.
            // Copy from internal buffer to dest.
            mBufferOffset += remainingBytesToRead;
            break;
         }
         else
         {
            // Copy from internal buffer to dest
            remainingBytesToRead -= numBufferBytes;
            // No bytes left in buffer, read in next block from stream to refill buffer.
            GetNextBlock();
         }
      }
   }
   mReadBytes += numBytes;
}

//----------------------------------------------------------------------------

void CInputStream::GetNextBlock()
{
   mBufferLength = Read(mpBuffer, mBufferSize);

   if (!mBufferLength)
   {
      BPE_VERIFYA(false, "CInputStream reached end of stream!" );
   }

   mBufferOffset = 0;
}

//----------------------------------------------------------------------------
// This function is useful for reducing data duplication for memory streams

uint8 * CInputStream::ReadAsMemoryStream(uint32 const size)
{
   BPE_ASSERT(size <= GetUnreadBufferLength(), "CInputStream not enough bytes in buffer!");
   uint8 * pData = mpBuffer + mBufferOffset;
   mReadBytes += size;
   mBufferOffset += size;
   return pData;
}

//----------------------------------------------------------------------------

std::string const CInputStream::ReadCStyleString()
{
   std::vector<char> mStringBuffer;

   char readChar = 0;

   do
   {
      readChar = ReadUint8();
      mStringBuffer.push_back(readChar);
   }
   while (readChar);

   std::string string( static_cast<const char*>( &( *mStringBuffer.begin() ) ) );
   return string;
}

//----------------------------------------------------------------------------


