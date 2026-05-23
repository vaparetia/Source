//----------------------------------------------------------------------------
// DCCOsContext.cpp
// Dreamcast OS context — KallistiOS
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include BPE_PLATFORM_SPECIFIC( COsContext.h )

#include <arch/arch.h>
#include <kos/thread.h>

//----------------------------------------------------------------------------

static char const * sRootFolder = "/cd";

//----------------------------------------------------------------------------

COsContext::COsContext()
:  CBaseOsContext()
{
   mBuildSKU         = kBS_USA;
   mSystemRegion     = kR_USA;
   mControllerRegion = kCR_USA;
   mLanguage         = kL_English;
   mBootType         = kBT_Disc;

   strcpy(mTitleId,          skTitleId_NA);
   strcpy(mTitleId_SaveData, skTitleId_NA_SaveData);

   strcpy(mBuildVersionString, "DC");
}

//----------------------------------------------------------------------------

COsContext::~COsContext()
{
}

//----------------------------------------------------------------------------

char const * COsContext::GetRootFolder()
{
   return sRootFolder;
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetMachineName()
{
   return "dreamcast";
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetCurrentUserName()
{
   return "dc_user";
}

//----------------------------------------------------------------------------

void CBaseOsContext::TerminateProcess()
{
   arch_exit();
}

//----------------------------------------------------------------------------

CBaseOsContext::EBootType CBaseOsContext::GetBootType() const
{
   return mBootType;
}

//----------------------------------------------------------------------------

CBaseOsContext::EProfileStatus CBaseOsContext::GetUserProfile()
{
   return kPS_Valid;
}

//----------------------------------------------------------------------------

void CBaseOsContext::SetUnsafeToShutDownFlag(uint32 const flag)
{
   // Single-threaded initial port; plain bitwise op is sufficient.
   mUnsafeToShutDownFlags |= flag;
}

//----------------------------------------------------------------------------

void CBaseOsContext::ClearUnsafeToShutDownFlag(uint32 const flag)
{
   mUnsafeToShutDownFlags &= ~flag;
}

//----------------------------------------------------------------------------

bool CBaseOsContext::IsSafeToShutDown() const
{
   return mUnsafeToShutDownFlags == 0;
}

//----------------------------------------------------------------------------
