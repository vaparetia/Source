//------------------------------------------------------------------------------------------
// COutputStream.h
// Bluepoint
// Copyright 2001
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class COutputStream
{
public:
   enum EOutputEndian
   {
      kOE_LittleEndian, // x86
      kOE_BigEndian,    // PPC
      kOE_Count
   };

public:

   ENGINE_API explicit COutputStream(int const bufferSize);
   ENGINE_API virtual ~COutputStream();
   
   // raw data write
   ENGINE_API void Put(void const *pBuffer, uint32 const bufferSize);

   // Template function will automatically call the right function based on the type passed in
   template <class T> void Put(T const &);

   // Zero terminated string
   ENGINE_API void WriteCStyleString(std::string const &string);

   ENGINE_API void WriteBool(bool const val);
   ENGINE_API void WriteInt8(int8 const val);
   ENGINE_API void WriteInt16(int16 const val);
   ENGINE_API void WriteInt32(int32 const val);
   ENGINE_API void WriteInt64(int64 const val);

   ENGINE_API void WriteUint8(uint8 const val);
   ENGINE_API void WriteUint16(uint16 const val);
   ENGINE_API void WriteUint32(uint32 const val);
   ENGINE_API void WriteUint64(uint64 const val);

   ENGINE_API void WriteReal32(real32 const val);
   ENGINE_API void WriteReal64(real64 const val);
  
   ENGINE_API void WriteString(std::string const & val);
   ENGINE_API void WriteString(bpe::istring const & val);

   ENGINE_API void Write7BitEncodedInt(uint32 const value);   // Writes .NET style size

   inline uint32 GetWrittenBytes() const { return mWrittenBytes; }

   ENGINE_API void AddAlignmentPadding(uint32 const alignment);

   ENGINE_API void Flush();
   ENGINE_API void SetEndian(EOutputEndian const endian);   // Only supported under Win32
   ENGINE_API EOutputEndian GetEndian() const { return mEndian; }

protected:
   ENGINE_API virtual void Write(void const *buffer, uint32 const bufferSize) = 0;

private:
   uint32         mBufferLength;
   uint32         mBufferSize;
   uint8 *        mpBuffer;
   uint32         mWrittenBytes;
   EOutputEndian  mEndian;
};

//----------------------------------------------------------------------------

#include "Engine/Streams/COutputStream.inl"

//----------------------------------------------------------------------------
