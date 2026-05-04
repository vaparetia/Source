//----------------------------------------------------------------------------
// Win32COsContext.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "COsContext.h"

//----------------------------------------------------------------------------

class ENGINE_API COsContext : public CBaseOsContext
{
public:
   COsContext();

   // Win32 specific helper functions
   bool  HaveFocus() const;         // Do we have focus? Deals with running in Maya window.
   bool  HostedInWindow() const;

public:
   BPE_HANDLE  mInstanceHandle;
   BPE_HANDLE  mMainWindow;
   BPE_HANDLE  mHostedWindow;    // NULL when not running in a Maya window.
};

//----------------------------------------------------------------------------

__forceinline COsContext * OsContext()
{
   return static_cast<COsContext*>(gpOsContext);
}


