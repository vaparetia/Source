//------------------------------------------------------------------------------------------
// BP_TrophySystemWin32.cpp
//
// Win32 platform specific trophy functions.
//------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BP_TrophySystem.h"

//----------------------------------------------------------------------------

static bool sDebugTrophySet[kTRP_Count];

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Init()
{
	memset(sDebugTrophySet, 0, sizeof(sDebugTrophySet));
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Shutdown()               {};
extern "C" int BP_TrophySystem_UpdateInProgress()  { return 0; };
extern "C" void BP_TrophySystem_Disable()                {};
extern "C" void BP_TrophySystem_Enable()                {};
extern "C" int BP_TrophySystem_IsDisabled() { return 0; }
extern "C" void BP_TrophySystem_SaveTrophyToBitfield( ETrophies trophyID, STrophyDeltaBitfield* pBitfield ) { }

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_UnlockTrophy(ETrophies trophy)
{
	if (!sDebugTrophySet[trophy])
	{
		sDebugTrophySet[trophy] = true;
		char const * const pTrophyDebugString = BP_TrophySystem_GetDebugString(trophy);
		printf("TROPHY: Unlocked %s\n", pTrophyDebugString);
	}
}

//----------------------------------------------------------------------------
