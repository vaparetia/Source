//------------------------------------------------------------------------------------------
// CInputStream.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/Basics/NEndian.h"
#include <string>
#include <vector>

//----------------------------------------------------------------------------

template <class T> inline T const CInputStream::Get(TType<T> const &)
{
   // Default implementation calls stream constructor for object
   return T(*this);
}

//----------------------------------------------------------------------------

template <> inline bool const CInputStream::Get(TType<bool> const &) 
{
   return ReadBool(); 
}

//----------------------------------------------------------------------------

template <> inline int8 const CInputStream::Get(TType<int8> const &) 
{
   return ReadInt8(); 
}

//----------------------------------------------------------------------------

template <> inline int16 const CInputStream::Get(TType<int16> const &) 
{
   return ReadInt16(); 
}

//----------------------------------------------------------------------------

template <> inline int32 const CInputStream::Get(TType<int32> const &) 
{
   return ReadInt32(); 
}

//----------------------------------------------------------------------------

template <> inline int64 const CInputStream::Get(TType<int64> const &) 
{
   return ReadInt64(); 
}

//----------------------------------------------------------------------------

template <> inline uint8 const CInputStream::Get(TType<uint8> const &) 
{
   return ReadUint8(); 
}

//----------------------------------------------------------------------------

template <> inline uint16 const CInputStream::Get(TType<uint16> const &) 
{
   return ReadUint16(); 
}

//----------------------------------------------------------------------------

template <> inline uint32 const CInputStream::Get(TType<uint32> const &) 
{
   return ReadUint32(); 
}

//----------------------------------------------------------------------------

template <> inline uint64 const CInputStream::Get(TType<uint64> const &) 
{
   return ReadUint64(); 
}

//----------------------------------------------------------------------------

template <> inline real32 const CInputStream::Get(TType<real32> const &) 
{
   return ReadReal32(); 
}

//----------------------------------------------------------------------------

template <> inline real64 const CInputStream::Get(TType<real64> const &) 
{
   return ReadReal64(); 
}

//----------------------------------------------------------------------------

inline bool const CInputStream::ReadBool()
{
   return ReadUint8() != 0;
}

//----------------------------------------------------------------------------

inline int8 const CInputStream::ReadInt8()
{
   return static_cast<int8>(ReadUint8());
}

//----------------------------------------------------------------------------

inline int16 const CInputStream::ReadInt16()
{
   return static_cast<int16>(ReadUint16());
}

//----------------------------------------------------------------------------

inline int32 const CInputStream::ReadInt32()
{
   return static_cast<int32>(ReadUint32());
}

//----------------------------------------------------------------------------

inline int64 const CInputStream::ReadInt64()
{
   return static_cast<int64>(ReadUint64());
}

//----------------------------------------------------------------------------

inline uint8 const CInputStream::ReadUint8()
{
   if ( mBufferLength - mBufferOffset )
   {
      ++mReadBytes;
      return mpBuffer[mBufferOffset++];
   }
   else
   {
      uint8 c = 0;
      Get(&c, 1);
      return c;
   }
}

//----------------------------------------------------------------------------

inline uint16 const CInputStream::ReadUint16()
{
   uint16 s;
   Get(&s, sizeof(s));
   NEndian::Swap2Bytes(&s);
   return s;
}

//----------------------------------------------------------------------------

inline uint32 const CInputStream::ReadUint32()
{
   uint32 l;
   Get(&l, sizeof(l));
   NEndian::Swap4Bytes(&l);
   return l;
}

//----------------------------------------------------------------------------

inline uint64 const CInputStream::ReadUint64()
{
   uint64 ll;
   Get(&ll, sizeof(ll));
   NEndian::Swap8Bytes(&ll);
   return ll;
}

//----------------------------------------------------------------------------

inline real32 const CInputStream::ReadReal32()
{
   real32 l;
   Get(&l, sizeof(l));
   NEndian::Swap4Bytes(&l);
   return l;
}

//----------------------------------------------------------------------------

inline real64 const CInputStream::ReadReal64()
{
   real64 ll;
   Get(&ll, sizeof(ll));
   NEndian::Swap8Bytes(&ll);
   return ll;
}

//----------------------------------------------------------------------------

inline std::string const CInputStream::ReadString()
{
   uint32 size = Read7BitEncodedInt();
   std::string outString(size, ' ');
   std::string::iterator iter = outString.begin();
   while (size)
   {
      *iter = ReadUint8();
      ++iter;
      size--;
   }
   return outString;
}

//----------------------------------------------------------------------------

inline bpe::istring const CInputStream::ReadIString()
{
   uint32 size = Read7BitEncodedInt();
   bpe::istring outString(size, ' ');
   bpe::istring::iterator iter = outString.begin();
   while (size)
   {
      *iter = ReadUint8();
      ++iter;
      size--;
   }
   return outString;
}

//------------------------------------------------------------------------------------------
// Reads .NET style size, variable length, base 128, with MSB representing continuation.
// Allows for 1 byte representation for sizes of less than 127 bytes.
// See .NET reflector System.IO.BinaryWriter.Read7BitEncodedInt.

inline uint32 const CInputStream::Read7BitEncodedInt()
{
   uint32 num1 = 0;
   uint32 num2 = 0;
   for (;;)
   {
      BPE_ASSERT(num2 != 0x23, "Format_Bad7BitInt32");   // Not too sure what this is about (max size 2^32 - 1?), but from .NET code so I'll leave it in.
      uint8 num3 = ReadUint8();
      num1 |= ((num3 & 0x7f) << (num2 & 0x1f));
      num2 += 7;
      if ((num3 & 0x80) == 0)
      {
         break;
      }
   }
   return num1;
}

//------------------------------------------------------------------------------------------

