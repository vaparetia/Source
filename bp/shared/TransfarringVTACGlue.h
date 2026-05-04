//----------------------------------------------------------------------------
// TransfarringVTACGlue.h
//
// C wrappers for Vita-specific C++ Transfarring functions
//----------------------------------------------------------------------------

#pragma once

#include "BP_Transfarring.h"

#ifdef __cplusplus
extern "C" {
#endif

// Connect to the PS3
STransfarringError Transfarring_PostNetworkThreadWork_ConnectToPS3();

// Disconnect from the PS3
STransfarringError Transfarring_PostNetworkThreadWork_DisconnectFromPS3();

int Transfarring_IsConnectedToPS3();

int Transfarring_NeedsRealDelete();

int Transfarring_GetSystemResumeCount();

#if MGS_VERSION == 2

STransfarringError Transfarring_PostWork_DeleteSaveFile( MCMAN_WORK* mcActor, ESaveType saveType, int saveIndex );
STransfarringError Transfarring_PostWork_DeleteNonGameFile( MCMAN_WORK* mcActor, ENonGameSaveType saveType, int saveIndex );

#endif

#if MGS_VERSION == 3

STransfarringError Transfarring_PostWork_DeleteSaveFile( void* mcActor, ESaveType saveType, int saveIndex );
STransfarringError Transfarring_PostWork_DeleteNonGameFile( void* mcActor, ENonGameSaveType saveType, int saveIndex );

#endif

#ifdef __cplusplus
};
#endif