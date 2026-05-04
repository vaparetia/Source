//------------------------------------------------------------------------------------------
// CGuid.h
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

class CGuid
{
   friend class cguid_hash_compare;
public:
   enum EException
   {
      kException_InvalidString
   };

public:
   ENGINE_API explicit CGuid( std::string const & textGuid );
   ENGINE_API explicit CGuid( BPE_GUID  const &guid );
   ENGINE_API explicit CGuid( uint32 const l,
                              uint16 const w1, 
                              uint16 const w2, 
                              uint16 const w3,
                              uint8 const b1,
                              uint8 const b2,
                              uint8 const b3,
                              uint8 const b4,
                              uint8 const b5,
                              uint8 const b6);
                              
   ENGINE_API explicit CGuid( CInputStream & stream );

   ENGINE_API std::string const AsString() const;

   bool const operator < ( CGuid const &rhs ) const
   {
      if (mUnionUint64_1 < rhs.mUnionUint64_1)
      {
         return true;
      }
      if ((mUnionUint64_1 == rhs.mUnionUint64_1) && (mUnionUint64_2 < rhs.mUnionUint64_2))
      {
         return true;
      }
      return false;
   }

   bool const operator == ( CGuid const &rhs ) const
   {
      return ((mUnionUint64_1 == rhs.mUnionUint64_1) && (mUnionUint64_2 == rhs.mUnionUint64_2));
   }

   bool const operator != ( CGuid const &rhs ) const
   {
      return !(*this == rhs);
   }

#if BPE_TARGET == BPE_TARGET_WIN32   
   ENGINE_API int32 const Hash() const;
   ENGINE_API static CGuid const Generate();
#endif

   ENGINE_API static CGuid const Null();

   ENGINE_API void PutTo( COutputStream & stream ) const;

public:
   union
   {
      BPE_GUID  mGuid;
      struct
      {
         uint32   mUnionUint32_1;
         uint32   mUnionUint32_2;
         uint32   mUnionUint32_3;
         uint32   mUnionUint32_4;
      };
      struct
      {
         uint64   mUnionUint64_1;
         uint64   mUnionUint64_2;
      };
   };
};

//------------------------------------------------------------------------------------------


