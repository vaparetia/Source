//----------------------------------------------------------------------------
// BP_Transfarring.h
//
// C interface for CTransfarringManager; shared between platforms
// Platform-specific transfarring functions will be in TransfarringVTACGlue.h or Transfarring_PS3.h
//----------------------------------------------------------------------------

#pragma once

#include "TransfarringDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

void Transfarring_InitializeSystems();

void Transfarring_LockSaveLoadMutex();
void Transfarring_UnlockSaveLoadMutex();
int Transfarring_GetTransfarringStatus();
int Transfarring_GetProgress();

int Transfarring_IsTransfarringStatusIdle();
int Transfarring_IsTransfarringStatusError();

// Functions for obtaining information about transfarred files
int Transfarring_IsFileLocked( ESaveType saveType, int saveIndex ); // Returns true if the file has a WiFi ID and the platform is PS3
int Transfarring_IsFileLinked( ESaveType saveType, int saveIndex ); // Returns true if the file has a WiFi ID, regardless of platform
int Transfarring_IsFileCloud( ESaveType saveType, int saveIndex );
int Transfarring_HasOfflineWork( ESaveType saveType, int saveIndex );
int Transfarring_HasCloudConflict( ESaveType saveType, int saveIndex );
int Transfarring_IsLocalFileCorrupt( ESaveType saveType, int saveIndex );
int Transfarring_IsRemoteFileCloud( TransfarringSaveFile* saveFile );
int Transfarring_IsRemoteFileLinked( TransfarringSaveFile* saveFile );

int Transfarring_IsLocalFileCorrupt_Platform( ESaveType saveType, int saveIndex ); // Uses platform index instead of concatenated index
void Transfarring_SetLocalFileCorrupt_Platform( int isCorrupt, ESaveType saveType, int saveIndex ); // Uses platform index instead of concatenated index

void Transfarring_LockTransfarringIDUpdates();
void Transfarring_AllowTransfarringIDUpdates();

void Transfarring_SetOfflineWork( ESaveType saveType, int saveIndex, int isOffline );

STransfarringError Transfarring_GetWifiTransfarringError();
void Transfarring_ClearTransfarringError();

// Returns 0 if file:
// 1) Is a locked WiFi file
// 2) Is a cloud file with a mismatched PSN account
int Transfarring_CanLoadFile(ESaveType saveType, int saveIndex);

// Converts an ASCII file index to an integer (MGS3 uses hex values in the filenames)
int Transfarring_SaveIndexStringToInt(const char* str);

// Checks whether the entire trophy system should be disabled
int Transfarring_CheckTrophyCompatibility( const SBP_LinkVars* bplv, ESaveType saveType, int saveIndex );
int Transfarring_CheckTrophyCompatibilityTID( const SBP_LinkVars* bplv, unsigned char* tid );

// Checks where the trophy valid flag is set
int Transfarring_CheckTrophyValidFlag( const SBP_LinkVars* bplv );

// Writes the originator data if it's not present
void Transfarring_TryWriteOriginatorData( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex );

// Allocate a new TransfarringID (used when saving a new game)
void Transfarring_MakeNewTransfarringID( ESaveType saveType, int saveIndex );

// Clear originator data minus the trophy valid flag (Only use this if you know what you're doing!)
void Transfarring_ClearOriginatorData( SBP_LinkVars* bplv );

void Transfarring_SetLoadedTransfarringID( ESaveType saveType, int saveIndex );
void Transfarring_SetLoadedTransfarringIDByString( unsigned char* tid );
void Transfarring_GetLoadedTransfarringID( unsigned char* tid );

ESaveType Transfarring_KonamiSaveTypeToBPSaveType(int file_kind);

int Transfarring_IsLocalFileListValid();
int Transfarring_GetNumLocalSaveFiles(ESaveType saveType);
TransfarringSaveFile* Transfarring_GetLocalSaveFile(ESaveType saveType, int index);

int Transfarring_IsCloudFileListValid();
int Transfarring_GetNumCloudSaveFiles(ESaveType saveType);
TransfarringSaveFile* Transfarring_GetCloudSaveFile(ESaveType saveType, int index);
int Transfarring_IsCloudFileCorrupt( ESaveType saveType, int index );

// After calling these functions, you must continuously call Transfarring_ProcessWork until it returns a nonzero number
STransfarringError Transfarring_PostWork_FlagCloudFile(ESaveType saveType, int index); // Flag a file as cloud and push it to the server
STransfarringError Transfarring_PostWork_UnflagCloudFile( unsigned char const * const transfarringID ); // Unflag a file and delete it from the server
STransfarringError Transfarring_PostWork_UnlockSaveFile(ESaveType saveType, int saveIndex);

STransfarringError Transfarring_PostWork_CheckSyncNeeded( ESaveType saveType, int treatVersionMismatchesAsConflicts );
STransfarringError Transfarring_PostWork_WillTrophiesBeDisabled( ESaveType saveType, int index );

#if MGS_VERSION == 2
STransfarringError Transfarring_PostWork_SyncCloudFiles(MCMAN_WORK* mcActor, int syncAllTypes, int treatVersionMismatchesAsConflicts);

STransfarringError Transfarring_PostWork_SyncSingleCloudFile(
   MCMAN_WORK* mcActor, 
   ESaveType saveType, 
   int saveIndex,
   ECloudConflictResolution conflictResolution,
   int treatVersionMismatchesAsConflicts);

#endif
#if MGS_VERSION == 3
STransfarringError Transfarring_PostWork_SyncCloudFiles(void* mcActor, int syncAllTypes, int treatVersionMismatchesAsConflicts);

STransfarringError Transfarring_PostWork_SyncSingleCloudFile(
   void* mcActor, 
   ESaveType saveType, 
   int saveIndex,
   ECloudConflictResolution conflictResolution,
   int treatVersionMismatchesAsConflicts);

#endif

// Process cloud work
STransfarringError Transfarring_ProcessWork();

// Trophy-related functions
void Transfarring_SetTrophyInvalidFlag( SBP_LinkVars* bplv );
int Transfarring_SetTrophyInvalidFlagIfApplicable( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex );
int Transfarring_WillTrophiesBecomeInvalid( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex );

// Common network thread functions
STransfarringError Transfarring_PostNetworkThreadWork_LoadFileList();
STransfarringError Transfarring_PostNetworkThreadWork_UnloadFileList();
STransfarringError Transfarring_PostNetworkThreadWork_CleanupSystem();

void Transfarring_MakeAccessoryData( 
   SCloudAccessoryData * const pOut, 
   TRANSFARRING_GAME_DATA const * const pGameData, 
   TransfarringSaveFile* pSaveFile );

#ifdef __cplusplus
};
#endif