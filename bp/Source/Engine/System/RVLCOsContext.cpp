//----------------------------------------------------------------------------
// Win32COsContext.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "COsContext.h"

//----------------------------------------------------------------------------

COsContext::COsContext()
:  CBaseOsContext()
{
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetMachineName()
{
   return "localhost";
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetCurrentUserName()
{
   return "unknown";
}

//----------------------------------------------------------------------------

CBaseOsContext::ELanguage CBaseOsContext::GetLanguage()
{
   mLanguageInitialized = true;
   return mLanguage;
}

//----------------------------------------------------------------------------

void CBaseOsContext::TerminateProcess()
{
   // Force verify, we don't support 'forced' terminate process
   BPE_VERIFYA(false, "CBaseOsContext::TerminateProcess().");
}

//----------------------------------------------------------------------------


