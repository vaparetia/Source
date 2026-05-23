//----------------------------------------------------------------------------
// COutputStream.h
// Bluepoint
// Copyright 2001
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Streams/COutputStream.h"
#include <string>

//----------------------------------------------------------------------------

COutputStream::COutputStream(int const bufferSize)
: mBufferLength( 0 )
, mBufferSize( uint32( bufferSize ) )
, mpBuffer( new uint8[ bufferSize ] )
, mWrittenBytes( 0 )
, mEndian(kOE_BigEndian)
{
   BPE_ASSERT(bufferSize > 0, "COutputStream buffer size must be greater than zero.");
}

//----------------------------------------------------------------------------

COutputStream::~COutputStream()
{
   BPE_ASSERT(!mBufferLength, "COutputStream derived class destructor didn't call Flush.");

   delete mpBuffer;
}

//----------------------------------------------------------------------------

void COutputStream::Put(void const *pBuffer, uint32 const bufferSize)
{
   BPE_ASSERT(pBuffer != NULL, "COutputStream can't write NULL buffer.");

   mWrittenBytes += bufferSize;

   if ((bufferSize + mBufferLength) <= mBufferSize)
   {
      memcpy(mpBuffer + mBufferLength, pBuffer, bufferSize);
      mBufferLength += bufferSize;
   }
   else
   {
      uint32 remainingBytesCount = bufferSize;
      
      while (remainingBytesCount)
      {
         uint32 const writeBufferRemaining = mBufferSize - mBufferLength;
         uint32 const writeBytesCount = (remainingBytesCount < writeBufferRemaining) ? remainingBytesCount : writeBufferRemaining;

         if (writeBytesCount)
         {
            memcpy(mpBuffer + mBufferLength, reinterpret_cast<const char *>(pBuffer) + bufferSize - remainingBytesCount, writeBytesCount);
            remainingBytesCount -= writeBytesCount;
            mBufferLength += writeBytesCount;
         }
         else
         {
            Flush();
         }
      }
   }
}

//----------------------------------------------------------------------------

void COutputStream::WriteCStyleString(std::string const &string)
{
   Put(string.c_str(), static_cast<uint32>(string.size() + 1));
}
   
//----------------------------------------------------------------------------

void COutputStream::Flush()
{
   if (mBufferLength)
   {
      Write(mpBuffer, mBufferLength);
      mBufferLength = 0;
   }
}

//----------------------------------------------------------------------------
// Writes .NET style size, variable length, base 128, with MSB representing continuation.
// Allows for 1 byte representation for sizes of less than 127 bytes.
// See .NET reflector System.IO.BinaryWriter.Write7BitEncodedInt.

void COutputStream::Write7BitEncodedInt(uint32 const value)
{
   uint32 num = value;
   while (num >= 0x80)
   {
      WriteUint8((uint8) (num | 0x80));
      num = num >> 7;
   }
   WriteUint8((uint8) num);
}

//----------------------------------------------------------------------------

void COutputStream::SetEndian(EOutputEndian const endian)
{
#if BPE_TARGET == BPE_TARGET_WIN32
   mEndian = endian;
#else
   BPE_VERIFY(endian == kOE_BigEndian, false, "COutputStream can only switch endian for Win32");
#endif
}

//----------------------------------------------------------------------------

static int GetAlignmentPosition(uint32 const pos, uint32 const alignment)
{
   int const remainder = (pos + (alignment - 1)) & (~(alignment - 1));
   return remainder;
}

//----------------------------------------------------------------------------

void COutputStream::AddAlignmentPadding(uint32 const alignment)
{
   int const aligmentPosition = GetAlignmentPosition(GetWrittenBytes(), alignment);
   while (GetWrittenBytes() < aligmentPosition)
   {
      WriteUint8(0xFF);
   }
}
