//----------------------------------------------------------------------------
// BP_NetworkStub.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/System/COsContext.h"

#include "BP_Network.h"

void BP_Network_Init()
{
}

void BP_Network_HeartBeat()
{
}

int BP_Network_IsSignedIn()
{
   return 0;
}

int BP_Network_Dialog_BeginPSN()
{
   return 0;
}

int BP_Network_Dialog_IsStillProcessing( int handle )
{
   return 0;
}

int BP_Network_Dialog_GetResults( int handle, int *result )
{
   *result = -1;
   return 0;
}

const char* BP_Network_GetCurrentUserName()
{
   return NULL;
}
