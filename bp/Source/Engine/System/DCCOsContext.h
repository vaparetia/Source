//----------------------------------------------------------------------------
// DCCOsContext.h
// Dreamcast OS context — KallistiOS
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "COsContext.h"

//----------------------------------------------------------------------------

class ENGINE_API COsContext : public CBaseOsContext
{
public:
   COsContext();
   ~COsContext();

   static char const * GetRootFolder();
};

//----------------------------------------------------------------------------

inline COsContext * OsContext()
{
   return static_cast<COsContext*>(gpOsContext);
}

//----------------------------------------------------------------------------
