//------------------------------------------------------------------------------------------
// BP_RichPresenceSystem.cpp
//
//------------------------------------------------------------------------------------------

#include "Engine/Stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BP_RichPresenceSystem.h"

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Graphics/CColor.h"

#if defined(BP_360)
#include <xtl.h>
#include "Engine/System/X360COsContext.h"

#if MGS_VERSION==2
#include "..\..\..\Builds\DiskBuilds\X360_XLAST_files\XLAST_files\mgshd.spa.h"
#elif MGS_VERSION==3
#include "..\..\..\Builds\DiskBuilds\X360_XLAST_files\XLAST_files\mgshd.spa.h"
#endif

int gBP_X360ShowRichPresence = 1;

static DWORD sActualPresence[4];
static DWORD sActivePresence;
static CStopWatch sRPStopWatch;

ERichPresence BP_GetActualRichPresence()
{
   COsContext* pOsContext = (COsContext*)gpOsContext;
   return (ERichPresence)sActualPresence[pOsContext->mSignedInUser];
}

const char* BP_GetActualRichPresenceStringAndColor(CColor& outRPColor)
{
   int actualPresence = BP_GetActualRichPresence();
   if( actualPresence < kRP_InTitleSelect || actualPresence > kRP_MGS3_Idle )
   {
      outRPColor = CColor::Red();
      return "Unknown Rich Presence!";
   }

   struct StringAndRPErrorCode
   {
      const char* string;
      CColor color;
   };
   static StringAndRPErrorCode enumRichPresenceNameList[] = 
   { 
      { "kRP_InTitleSelect", CColor::White() },
      { "kRP_MGS2_Idle", CColor::Yellow() },
      { "kRP_MGS2_MAINMENU", CColor::Yellow() },
      { "kRP_MGS2_SNAKETALES", CColor::White() },
      { "kRP_MGS2_VRMISSION", CColor::White() },
      { "kRP_MGS2_ALTMISSION", CColor::White() },
      { "kRP_MGS2_TANKER", CColor::White() },
      { "kRP_MGS2_PLANT", CColor::White() },
      { "kRP_MGS3_MAINMENU", CColor::Yellow() },
      { "kRP_MGS3_VIRTUOUS", CColor::White() },
      { "kRP_MGS3_SNAKEEATER", CColor::White() },
      { "kRP_MGS3_DEMOTHEATER", CColor::White() },
      { "kRP_MGS3_MG1", CColor::White() },
      { "kRP_MGS3_MG2", CColor::White() },
      { "kRP_MGS3_Idle", CColor::Yellow() },
   };

   outRPColor = enumRichPresenceNameList[actualPresence].color;
   return enumRichPresenceNameList[actualPresence].string;
}

void BP_RichPresenceSystem_SetActiveContext(ERichPresence activePresence)
{
   sActivePresence = activePresence;
}

inline void SetUserRichPresence_Internal( int iUser, int activePresence )
{
   sActualPresence[iUser] = activePresence;
   XUserSetContext( iUser, X_CONTEXT_PRESENCE, activePresence );
}

inline void SetUserToIdle( int iUser )
{
#if MGS_VERSION == 2
   SetUserRichPresence_Internal( iUser, kRP_MGS2_Idle );
#elif MGS_VERSION == 3
   SetUserRichPresence_Internal( iUser, kRP_MGS3_Idle );
#endif
}

extern volatile int gBP_ScreenSaverSuspended;
void BP_RichPresenceSystem_HeartBeat()
{
   if( sRPStopWatch.GetElapsedMilliseconds() > 1000 )
   {
      COsContext* pOsContext = (COsContext*)gpOsContext;

      for( int iUser=0;iUser<4;++iUser )
      {
         if( XUserGetSigninState(iUser) != eXUserSigninState_NotSignedIn )
         {
            if( iUser == pOsContext->mSignedInUser )
            {
               if( pOsContext->IsPlayerActive() || gBP_ScreenSaverSuspended )
               {
                  SetUserRichPresence_Internal( iUser, sActivePresence );
               }
               else
               {
                  SetUserToIdle( iUser );
               }
            }
            else
            {
               SetUserToIdle( iUser );
            }
         }
      }
      sRPStopWatch.Reset();
   }
}
#else

// Stubbed out on other platforms
ERichPresence BP_GetActualRichPresence()
{
  return kRP_InTitleSelect;
}
const char* BP_GetActualRichPresenceStringAndColor(CColor& outRPColor)
{
   outRPColor = CColor::Red();
   return "No Rich Presence on this platform";
}
void BP_RichPresenceSystem_SetActiveContext(ERichPresence activePresence)
{
}
void BP_RichPresenceSystem_HeartBeat()
{
}
#endif