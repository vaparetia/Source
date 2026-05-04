//----------------------------------------------------------------------------
// TransfarringVTACGlue.cpp
//
// Redirect C functions back into C++
//----------------------------------------------------------------------------

#include <math.h>
#include <string>

#include "MGS_Common.h"
#include "TransfarringVTACGlue.h"

#ifdef BP_VITA

#include "Engine/Stdafx.h"
#include "Engine/System/VTACOsContext.h"

#include "TransfarringVTA.h"

// C wrappers

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_ConnectToPS3()
{
   return CTransfarringManager::Instance()->PostNetworkThreadWork_ConnectToPS3();
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_DisconnectFromPS3()
{
   return CTransfarringManager::Instance()->PostNetworkThreadWork_DisconnectFromPS3();
}

extern "C" int Transfarring_IsConnectedToPS3()
{
   return (int)CTransfarringManager::Instance()->IsConnectedToPS3();
}

extern "C" int Transfarring_NeedsRealDelete()
{
   return (int)CTransfarringManager::Instance()->NeedsRealDelete();
}

extern "C" int Transfarring_GetSystemResumeCount()
{
   return OsContext()->GetSystemResumeCount();
}

extern "C" STransfarringError Transfarring_PostWork_DeleteSaveFile(MEMORY_CARD_ACTOR* mcActor, ESaveType saveType, int saveIndex)
{
   return CTransfarringManager::Instance()->PostWork_DeleteSaveFile(mcActor, saveType, saveIndex);
}

STransfarringError Transfarring_PostWork_DeleteNonGameFile( MEMORY_CARD_ACTOR* mcActor, ENonGameSaveType saveType, int saveIndex )
{
   return CTransfarringManager::Instance()->PostWork_DeleteNonGameFile(mcActor, saveType, saveIndex);
}

#else // Win32 stubs

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_ConnectToPS3()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_DisconnectFromPS3()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" int Transfarring_IsConnectedToPS3()
{
   return 0;
}

extern "C" int Transfarring_NeedsRealDelete()
{
   return 0;
}

#if MGS_VERSION == 2
extern "C" STransfarringError Transfarring_PostWork_DeleteSaveFile(MCMAN_WORK* mcActor, ESaveType saveType, int saveIndex)
#endif
#if MGS_VERSION == 3
extern "C" STransfarringError Transfarring_PostWork_DeleteSaveFile(void* mcActor, ESaveType saveType, int saveIndex)
#endif
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

#if MGS_VERSION == 2
extern "C" STransfarringError Transfarring_PostWork_DeleteNonGameFile(MCMAN_WORK* mcActor, ENonGameSaveType saveType, int saveIndex)
#endif
#if MGS_VERSION == 3
extern "C" STransfarringError Transfarring_PostWork_DeleteNonGameFile(void* mcActor, ENonGameSaveType saveType, int saveIndex)
#endif
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

#endif