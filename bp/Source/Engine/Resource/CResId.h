//------------------------------------------------------------------------------------------
// CResId.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

class ENGINE_API CResId : public bpe::istring
{
public:
   explicit CResId( std::string const & other );
   explicit CResId( char const * const pOther );
   explicit CResId( CInputStream & stream );
   ~CResId();

   void     PutTo( COutputStream & stream ) const;
   bool     IsFileResource() const;

   operator std::string const & () const           { return reinterpret_cast<std::string const&>(*this); }
   static CResId const & Null()                    { return skNull; };

private:
   void     UpdatePlatformSpecificPath();

   static CResId const  skNull;
};

//------------------------------------------------------------------------------------------

