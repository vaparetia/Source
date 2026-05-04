//------------------------------------------------------------------------------------------
// CGuid.cpp
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CGuid.h"
#include "Engine/Basics/CStringExtras.h"

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------
#if BPE_TARGET == BPE_TARGET_WIN32

#pragma BPE_AUTOLINKSYSLIB(rpcrt4)
#include <wtypes.h>
BPE_CTASSERT(sizeof(GUID) == sizeof(BPE_GUID));

#endif

//------------------------------------------------------------------------------------------

CGuid::CGuid(const std::string& textGuid)
{
#if BPE_TARGET == BPE_TARGET_WIN32

   if( UuidFromString( reinterpret_cast<unsigned char*>( const_cast<char*>( textGuid.c_str() ) ), (GUID*) &mGuid ) != RPC_S_OK )
   {
      BPE_VERIFYA(false, "Invalid string.");
   }

#else

   BPE_VERIFYA(false, "Not supported");

#endif
}

//------------------------------------------------------------------------------------------

CGuid::CGuid( BPE_GUID const &guid )
:  mGuid( guid )
{
}

//------------------------------------------------------------------------------------------

std::string const CGuid::AsString() const
{
#if BPE_TARGET == BPE_TARGET_WIN32

   unsigned char* pGuidString = NULL;
   UuidToString( (GUID*)&mGuid, &pGuidString );
   
   std::string const textGuid( reinterpret_cast<char const * const>( pGuidString ) );
   
   RpcStringFree( &pGuidString );
   return textGuid;

#else

   //              <Value>1a02b3f5-d7c1-433f-8613-55bab59aa215</Value>
   uint8 const *pChar = (uint8*) &mGuid;
   std::string textGuid(CStringExtras::Stringize("%x%x%x%x-%x%x-%x%x-%x%x-%x%x%x%x%x%x", 
                           pChar[0],
                           pChar[1],
                           pChar[2],
                           pChar[3],
                           pChar[4],
                           pChar[5],
                           pChar[6],
                           pChar[7],
                           pChar[8],
                           pChar[9],
                           pChar[10],
                           pChar[11],
                           pChar[12],
                           pChar[13],
                           pChar[14],
                           pChar[15]));
   return textGuid;

#endif
}

//------------------------------------------------------------------------------------------

CGuid::CGuid( CInputStream & stream )
{
   stream.Get( &mGuid, sizeof(BPE_GUID) );
}

//------------------------------------------------------------------------------------------

void CGuid::PutTo( COutputStream & stream ) const
{
   stream.Put( &mGuid, sizeof(BPE_GUID) );
}

//------------------------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32

int32 const CGuid::Hash() const
{
   RPC_STATUS temp;
   return UuidHash( (GUID*)&mGuid, &temp );
}

#endif

//------------------------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32

CGuid const CGuid::Generate()
{
   BPE_GUID guid;


   RPC_STATUS const status = UuidCreate((GUID*)&guid);
   BPE_ASSERT( status == RPC_S_OK, "generating guid failed!" );

   return CGuid(guid);
}

#endif
//------------------------------------------------------------------------------------------

CGuid const CGuid::Null()
{
   BPE_GUID nullGuid;
   memset( &nullGuid, 0, sizeof(BPE_GUID) );
   return CGuid( nullGuid );
}

//------------------------------------------------------------------------------------------
