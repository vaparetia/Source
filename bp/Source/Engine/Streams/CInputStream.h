//----------------------------------------------------------------------------
// CInputStream.h
// Bluepoint
// Copyright 2001
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "TType.h"
#include <string>
#include "Engine/StlExtras/BPEStlExtras.h"

//----------------------------------------------------------------------------

const uint32 gkDefaultInputStreamBufferSize = 4096;

//----------------------------------------------------------------------------

class CInputStream
{
public:
   ENGINE_API explicit CInputStream(uint32 const bufferSize = gkDefaultInputStreamBufferSize );
   ENGINE_API explicit CInputStream(void * pBuffer, uint32 const bufferSize, bool const bTakesOwnership = false );
   ENGINE_API virtual ~CInputStream();
      
   // Reads 'numBytes' from stream into 'pDest'
   ENGINE_API void Get(void* pDest, uint32 const numBytes);

   // Template function will automatically call the right get based on the type passed in
   template <class T> T const Get(TType<T> const &);
   
   ENGINE_API bool  const ReadBool();
   ENGINE_API int8  const ReadInt8();
   ENGINE_API int16 const ReadInt16();
   ENGINE_API int32 const ReadInt32();
   ENGINE_API int64 const ReadInt64();

   ENGINE_API uint8  const ReadUint8();
   ENGINE_API uint16 const ReadUint16();
   ENGINE_API uint32 const ReadUint32();
   ENGINE_API uint64 const ReadUint64();

   ENGINE_API real32 const ReadReal32(); 
   ENGINE_API real64 const ReadReal64();
   
   ENGINE_API std::string const  ReadString();
   ENGINE_API bpe::istring const ReadIString();
   ENGINE_API std::string const  ReadCStyleString();

   ENGINE_API uint32 const Read7BitEncodedInt();   // Reads .NET style size

   uint32 const GetReadPosition() const { return mReadBytes; }

   ENGINE_API uint8 * ReadAsMemoryStream(uint32 const size);   // Returns current pointer and offsets buffer position. Dangerous as assumes 'size' is available in buffer.
   uint32 const   GetUnreadBufferLength() const { return mBufferLength - mBufferOffset; }

private:
   void  GetNextBlock();

protected:
   virtual uint32 const Read(void * pBuffer, uint32 const bufferSize) = 0;

private:

   uint32   mBufferOffset;
   uint32   mBufferLength;
   uint32   mBufferSize;

   uint8*   mpBuffer;
   bool     mOwnsBuffer;

   uint32   mReadBytes;
};

//----------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.inl"

