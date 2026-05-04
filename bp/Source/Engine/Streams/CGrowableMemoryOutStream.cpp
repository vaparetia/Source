//------------------------------------------------------------------------------------------
// CGrowableMemoryOutStream.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

//----------------------------------------------------------------------------

CGrowableMemoryOutStream::CGrowableMemoryOutStream(int const bufferSize)
: COutputStream(bufferSize)
{
}

//----------------------------------------------------------------------------

CGrowableMemoryOutStream::~CGrowableMemoryOutStream()
{
   Flush();
}

//----------------------------------------------------------------------------

void const * const CGrowableMemoryOutStream::GetData()
{
   Flush();
   return reinterpret_cast<void const * const>(&mData[0]);
}

//----------------------------------------------------------------------------

void * const CGrowableMemoryOutStream::Data()
{
   Flush();
   return reinterpret_cast<void * const>(&mData[0]);
}

//------------------------------------------------------------------------------------------

int const CGrowableMemoryOutStream::GetDataSize() const
{
   return GetWrittenBytes();
}

//------------------------------------------------------------------------------------------

void CGrowableMemoryOutStream::Reserve(int const size)
{
   mData.reserve(size);
}

//------------------------------------------------------------------------------------------

void CGrowableMemoryOutStream::Fill(int const size, uint8 const value)
{
   // Do a higher performance version of this later
   for (int loop = 0; loop < size; loop++)
   {
      WriteUint8(value);
   }
}

//------------------------------------------------------------------------------------------

void CGrowableMemoryOutStream::Write(void const * pBuffer, uint32 const bufferSize)
{
   int currentSize = mData.size();
   mData.resize(mData.size() + bufferSize);
   memcpy(&mData[currentSize], pBuffer, bufferSize);
}

//------------------------------------------------------------------------------------------

