//----------------------------------------------------------------------------
// TransfarringDefines.h
//
// Structs/defines shared across C/C++
//----------------------------------------------------------------------------

#pragma once

#include "linkvar.h"

#define MAX_NUM_CLOUD_FILES 8

#if MGS_VERSION == 2
#include "kano/mcman/mcman.h"
#endif

// For reasons I can't figure out, including MGS_Common.h here results in some weird errors, so this is copied straight from savedata.h
#if MGS_VERSION == 3
typedef struct {
   char gcl_linkvar[ MAX_LINKVARBUF ];
   char gcl_variable[ 6 * 1024 ];
   int gcl_version;
} _GAME_SAVE_DATA;
#endif

typedef struct STransfarringSaveDate_s
{
   int mYear;
   int mMonth;
   int mDay;
} STransfarringSaveDate;

// Structs
typedef struct
{
   unsigned int mSaveIndex;
   unsigned int mStageIndex;
   unsigned int mDifficulty;
   unsigned int mPlayTime;
   unsigned int mSaveType;

   STransfarringSaveDate mSaveDate;

   long long transfarringID[2];
   long long cloudVersionHash[2];
   long long psnAccountHash[2];

   unsigned short mClearCount;
} TransfarringSaveFile;

typedef struct SCloudAccessoryData_s
{
   unsigned int mCookie; // Cookie
   unsigned int mRegion; // Region (EBuildSKU)
   unsigned int mVersion; // Save data version
   unsigned char mSaveDataHash[16]; // Save data hash
   TransfarringSaveFile mSaveFile; // Fast-access game info
} SCloudAccessoryData;

// Enums
// Transfarrable save types
typedef enum
{
#if MGS_VERSION == 2
   kST_Game,
   kST_VR,
   kST_SnakeTales,
#endif

#if MGS_VERSION == 3
   kST_Game,
#endif

   kST_MaxSaveTypes
} ESaveType;

// Non-transfarrable save types
typedef enum
{
   kNGST_Photo,
#if MGS_VERSION == 3
   kNGST_MG1,
   kNGST_MG2,
#endif
   kNGST_MaxNonGameSaveTypes,
} ENonGameSaveType;

typedef enum
{
   kVTS_Uninitialized,
   kVTS_LoadingFileList,
   kVTS_AwaitingConnection,
   kVTS_ConnectingToPS3,
   kVTS_DisconnectingFromPS3,
   kVTS_Idle,
   kVTS_SendingSaveList,
   kVTS_ReceivingSaveFile,
   kVTS_SendingSaveFile,
   kVTS_UnlockingSaveFile,
   kVTS_DeletingSaveFile,
   kVTS_UnloadFileList,
   kVTS_CleaningUpSystem,
   kVTS_Error,
   kVTS_MaxStatus
} EVitaTransfarringStatus;

typedef enum
{
   kPTS_Uninitialized,
   kPTS_InitializingAccessPoint,
   kPTS_ClosingAccessPoint,
   kPTS_LoadingFileList,
   kPTS_AwaitingConnection,
   kPTS_Idle,
   kPTS_ReceivingSaveList,
   kPTS_SendingSaveFile,
   kPTS_ReceivingSaveFile,
   kPTS_UnlockingSaveFile,
   kPTS_UnloadingFileList,
   kPTS_CleaningUpSystem,
   kPTS_Error,
   kPTS_MaxStatus
} EPS3TransfarringStatus;

// Error codes
typedef enum ETransfarringErrorEnum
{
   kTE_Processing,
   kTE_Success,
   kTE_FailedToSend, // WiFi
   kTE_FailedToReceive, // WiFi
   kTE_NotEnoughRemoteSaveSlots, // WiFi
   kTE_NoCloudSlotsAvailable, // Cloud
   kTE_NotEnoughLocalSaveSlots, // Cloud/WiFi
   kTE_Offline, // Cloud
   kTE_SetSlotData_SizeTooLarge, // Cloud
   kTE_GetSlotData_SlotNotFound, // Cloud
   kTE_SignedInButDisconnected,
   kTE_WrongFileType, // Cloud/WiFi; this error means you tried to do a cloud operation on a WiFi file or vice-versa
   kTE_WorkInProgress, // Cloud/WiFi; this error means you tried to start a new operation while was was already in progress
   kTE_Unknown, // WiFi/Cloud
   kTE_FailedToConnectToPS3, // Vita WiFi only
   kTE_FailedToDisconnectFromPS3, // Vita WiFi only
   kTE_CorruptSaveData,
   kTE_OutOfSpaceLocal,
   kTE_OutOfSpaceRemote,
   kTE_FailedToOpenAccessPoint,
   kTE_WifiVersionMismatch, // Cannot connect to remote system; you are running incompatible software versions
   kTE_StorageMediaError,
   kTE_OwnershipError,
   kTE_MaxNumErrors,
} ETransfarringError;

// Dealing with mismatches in cloud version numbers
typedef enum
{
   kCCR_Invalid,
   kCCR_SkipSyncForConflictedFiles, // Sync the file only if there's no conflicts
   kCCR_UseLocalSaveFile, // Overwrite the version on the cloud with the local version
   kCCR_UseCloudSaveFile, // Overwrite the local version with the cloud version
   kCCR_MaxNumResolutions
} ECloudConflictResolution;

typedef struct
{
   ETransfarringError mHighLevelError; // ETransfarringError
   int mLowLevelError; // Vita/PS3 API errors
} STransfarringError;

#define SET_TRANSFARRING_ERROR(errorStruct, highLevelError, lowLevelError) \
   do { \
   (errorStruct).mHighLevelError = highLevelError; \
   (errorStruct).mLowLevelError = lowLevelError;   \
   } while(0)

#define SET_TRANSFARRING_ERROR_SUCCESS(errorStruct) \
   do { \
   (errorStruct).mHighLevelError = kTE_Success; \
   (errorStruct).mLowLevelError = 0;   \
   } while(0)

#define SET_TRANSFARRING_ERROR_INVALID(errorStruct) \
   do { \
   (errorStruct).mHighLevelError = kTE_MaxNumErrors; \
   (errorStruct).mLowLevelError = 0;   \
   } while(0)

// Typedefs
typedef int ConcatenatedSaveIndex; // Save indices that go from 0 to NumFiles-1. If there are any empty slots, this won't match the actual save slot indices
typedef int CloudSaveIndex; // Save indices that go from 1 to MaxNumCloudFiles

#if MGS_VERSION == 2

typedef struct
{
   MCMAN_GAMEDATA mSaveData;
   SBP_LinkVars mBPLinkvars;

   struct _ExtraData
   {
      unsigned char snake_tales_no;
      unsigned char st_clear_flag;
      STransfarringSaveDate mSaveDate;
   } mExtraData; // Any extra data you might need to append (ie infodata that can't be derived from the save data)
} TRANSFARRING_GAME_DATA;

#define SAVE_DATA_SIZE sizeof(MCMAN_GAMEDATA)

#endif

#if MGS_VERSION == 3

typedef struct
{
   _GAME_SAVE_DATA mSaveData;
   SBP_LinkVars mBPLinkvars;
   struct _ExtraData
   {
      STransfarringSaveDate mSaveDate;
   } mExtraData; // Any extra data you might need to append (ie infodata that can't be derived from the save data)
} TRANSFARRING_GAME_DATA;

#define SAVE_DATA_SIZE sizeof(_GAME_SAVE_DATA)

#endif