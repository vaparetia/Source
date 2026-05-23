//------------------------------------------------------------------------------------------
// COutputStream.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include "Engine/Basics/NEndian.h"
#include <string>

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32
#define _BPE_OUTPUTSTREAM_ALLOW_ENDIAN_SWITCH
#endif

//----------------------------------------------------------------------------

template <class T> inline void COutputStream::Put(T const & value)
{
   value.PutTo(*this);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(bool const & value)
{
   WriteBool(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(int8 const & value)
{
   WriteInt8(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(int16 const & value)
{
   WriteInt16(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(int32 const & value)
{
   WriteInt32(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(int64 const & value)
{
   WriteInt64(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(uint8 const & value)
{
   WriteUint8(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(uint16 const & value)
{
   WriteUint16(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(uint32 const & value)
{
   WriteUint32(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(uint64 const & value)
{
   WriteUint64(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(real32 const & value)
{
   WriteReal32(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(real64 const & value)
{
   WriteReal64(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(std::string const & value)
{
   WriteString(value);
}

//----------------------------------------------------------------------------

template <> inline void COutputStream::Put(bpe::istring const & value)
{
   WriteString(value);
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteBool(bool const val)
{
   WriteUint8(static_cast<uint8>(val ? 1 : 0) );
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteInt8(int8 const val)
{
   WriteUint8(static_cast<uint8>(val));
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteInt16(int16 const val)
{
   WriteUint16(static_cast<uint16>(val));
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteInt32(int32 const val)
{
   WriteUint32(static_cast<uint32>(val));
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteInt64(int64 const val)
{
   WriteUint64(static_cast<uint64>(val));
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteUint8(uint8 const val)
{
   if( mBufferLength >= mBufferSize )
      Flush();

   ++mWrittenBytes;

   mpBuffer[mBufferLength++] = val;   
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteUint16(uint16 const val)
{
#if defined(_BPE_OUTPUTSTREAM_ALLOW_ENDIAN_SWITCH)
   uint16 newS = (mEndian == kOE_BigEndian) ? NEndian::GetSwapped(val) : val;
   Put(&newS,  sizeof(uint16));
#else
   Put(&val,  sizeof(uint16));
#endif
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteUint32(uint32 const val)
{
#if defined(_BPE_OUTPUTSTREAM_ALLOW_ENDIAN_SWITCH)
   uint32 newL = (mEndian == kOE_BigEndian) ? NEndian::GetSwapped(val) : val;
   Put(&newL,  sizeof(uint32));
#else
   Put(&val,  sizeof(uint32));
#endif
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteUint64(uint64 const val)
{
#if defined(_BPE_OUTPUTSTREAM_ALLOW_ENDIAN_SWITCH)
   uint64 newLL = (mEndian == kOE_BigEndian) ? NEndian::GetSwapped(val) : val;
   Put( &newLL,  sizeof( uint64 ) );
#else
   Put( &val,  sizeof( uint64 ) );
#endif
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteReal32(real32 const val)
{
#if defined(_BPE_OUTPUTSTREAM_ALLOW_ENDIAN_SWITCH)
   real32 newL = (mEndian == kOE_BigEndian) ? NEndian::GetSwapped(val) : val;
   Put(&newL,  sizeof(real32));
#else
   Put(&val,  sizeof(real32));
#endif
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteReal64(real64 const val)
{
#if defined(_BPE_OUTPUTSTREAM_ALLOW_ENDIAN_SWITCH)
   real64 newLL = (mEndian == kOE_BigEndian) ? NEndian::GetSwapped(val) : val;
   Put( &newLL,  sizeof( real64 ) );
#else
   Put( &val,  sizeof( real64 ) );
#endif
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteString(std::string const & val)
{
   Write7BitEncodedInt(val.size()); 
   Put(val.c_str(), static_cast<uint32>(val.size()));
}

//----------------------------------------------------------------------------

inline void COutputStream::WriteString(bpe::istring const & val)
{
   Write7BitEncodedInt(val.size()); 
   Put(val.c_str(), static_cast<uint32>(val.size()));
}

//----------------------------------------------------------------------------
