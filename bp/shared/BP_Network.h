#pragma once

#include "BP_BuildDefines.h"

#if BP_VITA || BP_PS3
#  define BP_USE_SCE_NP 1
#endif

#if BP_VITA
#  include <sdk_version.h>
#  include <np.h>
#endif

#if BP_PS3
#  include <np.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#if BP_PS3
   /* Communication Config */
   typedef struct SceNpCommunicationConfig {
      const SceNpCommunicationId			*commId;
      const SceNpCommunicationPassphrase	*commPassphrase;
      const SceNpCommunicationSignature	*commSignature;
   } SceNpCommunicationConfig;
#endif

// Initialize network.
void BP_Network_Init();

// Sent once per frame. Must be from a consistent thread.
void BP_Network_HeartBeat();

// Returns 0 if the user is signed out or in an indeterminate state
// Returns nonzero if the user is signed in or online
int BP_Network_IsSignedIn();

#if BP_VITA
int BP_Network_Vita_IsPSNSignedIn();
#endif

// Tells the network subsystem that we want PSN
void BP_Network_WantsPSNSignin();

// Returns handle for success.  0 if not, which means busy.
int BP_Network_Dialog_BeginPSN();

// Returns 1 if handle is still processing
// 0 if it's not or if there's an error (handle no longer is valid?)
int BP_Network_Dialog_IsStillProcessing( int handle );

// Returns 1 if handle is done processing properly
// Returns 0 if there's an error or if handle is not done
int BP_Network_Dialog_GetResults( int handle, int *result );

#if BP_USE_SCE_NP

SceNpCommunicationConfig const *BP_Network_GetTUSNPConfig();

// Returns ptr to NP ID. Returns NULL if invalid.
SceNpId const *BP_Network_GetNPID();

// Fills in an NP ID with the one in the cache.  If there is no cached ID, *pIsValid will be 0.
// If there is a cached ID, *pIsValid is 1
void BP_Network_GetNPIDCached( SceNpId *id, int *pIsValid );

// Gets the name of the currently logged in user; not the PSN ID, but the local user
const char* BP_Network_GetCurrentUserName();

#endif

#ifdef __cplusplus
}
#endif // __cplusplus