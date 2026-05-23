//------------------------------------------------------------------------------------------
// CGrowableMemoryOutStream.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/COutputStream.h"

//----------------------------------------------------------------------------

class CGrowableMemoryOutStream : public COutputStream
{
public:
   ENGINE_API explicit CGrowableMemoryOutStream( int const bufferSize = 4096 );
   ENGINE_API virtual ~CGrowableMemoryOutStream();

   ENGINE_API void const * const GetData();
   ENGINE_API void * const Data();
   ENGINE_API int const GetDataSize() const;
   ENGINE_API void Reserve(int const size);
   ENGINE_API void Fill(int const size, uint8 const value);

protected:
   void Write(void const* pBuffer, uint32 const bufferSize);

private:
   std::vector<char> mData;
};

//----------------------------------------------------------------------------

