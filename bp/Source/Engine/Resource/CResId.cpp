//----------------------------------------------------------------------------
// CResId.h
// Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CResId.h"
//----------------------------------------------------------------------------

using namespace std;
using namespace bpe;

//----------------------------------------------------------------------------

CResId const CResId::skNull("");

//------------------------------------------------------------------------------------------

CResId::CResId( std::string const & other )
:  bpe::istring( other.c_str() )
{
   UpdatePlatformSpecificPath();
}

//------------------------------------------------------------------------------------------

CResId::CResId( char const * const pOther )
:  bpe::istring( pOther )
{
   UpdatePlatformSpecificPath();
}

//------------------------------------------------------------------------------------------

CResId::CResId( CInputStream & stream )
: bpe::istring( stream.ReadString().c_str() )
{
   UpdatePlatformSpecificPath();
}

//------------------------------------------------------------------------------------------

CResId::~CResId()
{   
}

//------------------------------------------------------------------------------------------

void CResId::PutTo( COutputStream & stream ) const
{
   stream.WriteString(*this);
}

//------------------------------------------------------------------------------------------

void CResId::UpdatePlatformSpecificPath()
{
   // Replace '****' with platform specific directory
   int const psDirIndex = this->find("****");

#if BPE_TARGET == BPE_TARGET_WIN32
   char const * const pPlatformDir = "_win";
#elif BPE_TARGET == BPE_TARGET_PS3
   char const * const pPlatformDir = "_ps3";
#elif BPE_TARGET == BPE_TARGET_RVL
   char const * const pPlatformDir = "_rvl";
#elif BPE_TARGET == BPE_TARGET_X360
   char const * const pPlatformDir = "_360";
#elif BPE_TARGET == BPE_TARGET_VITA
   char const * const pPlatformDir = "_vta";
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
   char const * const pPlatformDir = "_dc";
#else
#error Please add a new platform
#endif

   if (psDirIndex != -1)
   {  
      this->replace(psDirIndex, 4, pPlatformDir);
   }
}

//------------------------------------------------------------------------------------------

bool CResId::IsFileResource() const
{
   if (this->at(0) == '$')
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

