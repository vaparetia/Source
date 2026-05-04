//----------------------------------------------------------------------------
// BP_Transfarring.cpp
//
// Redirect C functions back into C++
//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/CStopWatch.h"

#include "MGS_Common.h"

#include "TransfarringVTACGlue.h"
#include "BP_SaveLoad.h"
#include "BP_SaveLoadMGS.h"

#include <sstream>
#include <cstdlib>

#if defined( BP_VITA )
#include "TransfarringVTA.h"
#elif defined( BP_PS3 )
#include "Transfarring_PS3.h"
#endif

#if defined( BP_VITA ) || defined( BP_PS3 )

extern "C" void Transfarring_InitializeSystems()
{
   CTransfarringManager::InitSingleton();

   // Launch the network thread
   CTransfarringManager::Instance()->InitNetwork();
}

extern "C" void Transfarring_LockSaveLoadMutex()
{
   CTransfarringManager::Instance()->LockSaveLoadMutex();
}

extern "C" void Transfarring_UnlockSaveLoadMutex()
{
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();
}

extern "C" int Transfarring_GetTransfarringStatus()
{
   return (int)CTransfarringManager::Instance()->GetTransfarringStatus();
}

extern "C" int Transfarring_GetProgress()
{
   return CTransfarringManager::Instance()->GetProgress();
}

extern "C" int Transfarring_IsTransfarringStatusIdle()
{
#if BP_VITA
   return CTransfarringManager::Instance()->GetTransfarringStatus() == kVTS_Idle;
#else
   return CTransfarringManager::Instance()->GetTransfarringStatus() == kPTS_Idle;
#endif
}

extern "C" int Transfarring_IsTransfarringStatusError()
{
#if BP_VITA
   return CTransfarringManager::Instance()->GetTransfarringStatus() == kVTS_Error;
#else
   return CTransfarringManager::Instance()->GetTransfarringStatus() == kPTS_Error;
#endif
}

extern "C" int Transfarring_IsFileLocked(ESaveType saveType, int saveIndex)
{
   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);
   return (int)tid.IsWifiLocked();
}

extern "C" int Transfarring_IsFileLinked(ESaveType saveType, int saveIndex)
{
   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);
   return (int)tid.IsWifiLinked();
}

extern "C" int Transfarring_IsFileCloud(ESaveType saveType, int saveIndex)
{
   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);
   bool ret = tid.m_Type == kTT_Cloud;
   return (int)ret;
}

extern "C" int Transfarring_HasOfflineWork(ESaveType saveType, int saveIndex)
{
   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);
   return (int)tid.HasOfflineWork();
}

extern "C" int Transfarring_HasCloudConflict( ESaveType saveType, int saveIndex )
{
   return (int)CTransfarringManager::Instance()->HasCloudConflict( saveType, saveIndex );
}

extern "C" int Transfarring_IsLocalFileCorrupt( ESaveType saveType, int saveIndex )
{
   return (int)CTransfarringManager::Instance()->IsLocalFileCorrupt( saveType, saveIndex );
}

extern "C" int Transfarring_IsRemoteFileCloud(TransfarringSaveFile* saveFile)
{
   unsigned char zeroMem[TransfarringDefines::kHashLengthBytes];
   memset(zeroMem, 0, sizeof( zeroMem ) );

   int retval = 0;

   if( memcmp( zeroMem, saveFile->cloudVersionHash, sizeof( zeroMem ) ) )
   {
      retval = 1;
   }

   return retval;
}

extern "C" int Transfarring_IsRemoteFileLinked(TransfarringSaveFile* saveFile)
{
   unsigned char zeroMem[TransfarringDefines::kHashLengthBytes];
   memset(zeroMem, 0, sizeof( zeroMem ) );

   int retval = 0;

   if( !memcmp( zeroMem, saveFile->cloudVersionHash, sizeof( zeroMem ) ) 
      && memcmp( zeroMem, saveFile->transfarringID, sizeof( zeroMem ) )
      )
   {
      retval = 1;
   }

   return retval;
}

extern "C" void Transfarring_LockTransfarringIDUpdates()
{
   CTransfarringManager::Instance()->LockTransfarringIDUpdates();
}

extern "C" void Transfarring_AllowTransfarringIDUpdates()
{
   CTransfarringManager::Instance()->AllowTransfarringIDUpdates();
}

extern "C" int Transfarring_CanLoadFile(ESaveType saveType, int saveIndex)
{
   // Returns 0 if file:
   // 1) Is a locked WiFi file
   // 2) Is a cloud file with a mismatched PSN account

   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);

   if (tid.IsWifiLocked())
   {
      return 0;
   }
   else if (tid.m_Type == kTT_Cloud)
   {
      // Compare the current PSN account hash to the account the file was saved with
      unsigned char PSNHash[TransfarringDefines::kHashLengthBytes];
      CTransfarringManager::GetPSNAccountHash( true, PSNHash, (unsigned char*)tid.m_UniqueID );

      if( !tid.DoPSNAccountsMatch( PSNHash ) )
      {
         return 0;
      }
   }

   return 1;
}

extern "C" int Transfarring_CheckTrophyValidFlag( const SBP_LinkVars* bplv )
{
   return (int)(bplv->mTrophyValidFlag == BPLinkvarDefines::kTrophiesValid);
}

extern "C" void Transfarring_TryWriteOriginatorData( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   CTransfarringManager::Instance()->TryWriteOriginatorData( bplv, saveType, saveIndex, true );
}

extern "C" int Transfarring_CheckTrophyCompatibility( const SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   return (int)CTransfarringManager::Instance()->CheckTrophyCompatibility( bplv, saveType, saveIndex );
};

extern "C" int Transfarring_CheckTrophyCompatibilityTID( const SBP_LinkVars* bplv, unsigned char* tid )
{
   return (int)CTransfarringManager::Instance()->CheckTrophyCompatibility( bplv, tid );
}

extern "C" void Transfarring_MakeNewTransfarringID( ESaveType saveType, int saveIndex )
{
   TransfarringID tid;
   tid.MakeNewInvalid();
   CTransfarringManager::Instance()->SetTransfarringID( tid, saveType, saveIndex );
}

extern "C" void Transfarring_ClearOriginatorData( SBP_LinkVars* bplv )
{
   // Don't clear the trophy valid flag
   bplv->mOriginalConsoleType = BPLinkvarDefines::kConsoleInvalid;
   memset( bplv->mOriginalConsolePSID, 0, sizeof(bplv->mOriginalConsolePSID) );
   memset( bplv->mOriginalPSNAccount, 0, sizeof(bplv->mOriginalPSNAccount) );
}

extern "C" void Transfarring_SetLoadedTransfarringID( ESaveType saveType, int saveIndex )
{
   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID( saveType, saveIndex );
   CTransfarringManager::Instance()->SetCurrentLoadedTransfarringID( (unsigned char*)tid.m_UniqueID );
}

extern "C" void Transfarring_SetLoadedTransfarringIDByString( unsigned char* tid )
{
   CTransfarringManager::Instance()->SetCurrentLoadedTransfarringID( (unsigned char*)tid );
}

extern "C" void Transfarring_GetLoadedTransfarringID( unsigned char* tid )
{
   CTransfarringManager::Instance()->GetCurrentLoadedTransfarringID( tid );
}

extern "C" int Transfarring_IsLocalFileListValid()
{
   return (int)CTransfarringManager::Instance()->IsLocalFileListValid();
}

extern "C" int Transfarring_GetNumLocalSaveFiles(ESaveType saveType)
{
   return CTransfarringManager::Instance()->GetNumLocalSaveFiles(saveType);
}

extern "C" TransfarringSaveFile* Transfarring_GetLocalSaveFile(ESaveType saveType, int index)
{
   return CTransfarringManager::Instance()->GetLocalSaveFile(saveType, index).get();
}

extern "C" int Transfarring_IsCloudFileListValid()
{
   return CTransfarringManager::Instance()->IsLocalFileListValid();
}

extern "C" int Transfarring_GetNumCloudSaveFiles(ESaveType saveType)
{
   return CTransfarringManager::Instance()->GetNumCloudSaveFiles(saveType);
}

extern "C" TransfarringSaveFile* Transfarring_GetCloudSaveFile(ESaveType saveType, int index)
{
   return CTransfarringManager::Instance()->GetCloudSaveFile(saveType, index).get();
}

extern "C" int Transfarring_IsCloudFileCorrupt( ESaveType saveType, int index )
{
   return (int)CTransfarringManager::Instance()->IsCloudFileCorrupt( saveType, index );
}

extern "C" int Transfarring_IsLocalFileCorrupt_Platform( ESaveType saveType, int saveIndex )
{
   return (int)CTransfarringManager::Instance()->IsLocalFileCorrupt_Platform( saveType, saveIndex );
}

extern "C" void Transfarring_SetLocalFileCorrupt_Platform( int isCorrupt, ESaveType saveType, int saveIndex )
{
   CTransfarringManager::Instance()->SetLocalFileCorrupt_Platform( (bool)isCorrupt, saveType, saveIndex );
}

extern "C" STransfarringError Transfarring_PostWork_FlagCloudFile(ESaveType saveType, int index)
{
   return CTransfarringManager::Instance()->PostWork_FlagCloudFile(saveType, index);
}

extern "C" STransfarringError Transfarring_PostWork_UnflagCloudFile( unsigned char const * const transfarringID )
{
   return CTransfarringManager::Instance()->PostWork_UnflagCloudFile( transfarringID );
}

extern "C" STransfarringError Transfarring_PostWork_SyncCloudFiles(MEMORY_CARD_ACTOR* mcActor, int syncAllTypes, int treatVersionMismatchesAsConflicts)
{
   return CTransfarringManager::Instance()->PostWork_SyncCloudFiles( mcActor, (bool)syncAllTypes, (bool)treatVersionMismatchesAsConflicts );
}

extern "C" STransfarringError Transfarring_PostWork_SyncSingleCloudFile(
   MEMORY_CARD_ACTOR* mcActor, 
   ESaveType saveType, 
   int saveIndex,
   ECloudConflictResolution conflictResolution,
   int treatVersionMismatchesAsConflicts)
{
   return CTransfarringManager::Instance()->PostWork_SyncSingleCloudFile(
      mcActor, 
      saveType, 
      saveIndex, 
      conflictResolution, 
      treatVersionMismatchesAsConflicts );
}

extern "C" STransfarringError Transfarring_PostWork_WillTrophiesBeDisabled(ESaveType saveType, int index)
{
   return CTransfarringManager::Instance()->PostWork_WillTrophiesBeDisabled(saveType, index);
}

extern "C" STransfarringError Transfarring_ProcessWork()
{
   return CTransfarringManager::Instance()->ProcessWork();
}

extern "C" void Transfarring_SetOfflineWork(ESaveType saveType, int saveIndex, int isOffline)
{
   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);

   BPE_ASSERT(tid.m_Type == kTT_Cloud, "Tried to set offline work on a non-cloud Transfarring ID");

   tid.m_HasOfflineWork = (bool)isOffline;

   CTransfarringManager::Instance()->SetTransfarringID(tid, saveType, saveIndex);
}

extern "C" STransfarringError Transfarring_PostWork_UnlockSaveFile(ESaveType saveType, int saveIndex)
{
   return CTransfarringManager::Instance()->PostWork_UnlockSaveFile(saveType, saveIndex);
}

extern "C" STransfarringError Transfarring_PostWork_CheckSyncNeeded( ESaveType saveType, int treatVersionMismatchesAsConflicts )
{
   return CTransfarringManager::Instance()->PostWork_CheckSyncNeeded( saveType, treatVersionMismatchesAsConflicts );
}

extern "C" STransfarringError Transfarring_GetWifiTransfarringError()
{
   return CTransfarringManager::Instance()->GetWiFiTransfarringError();
}

extern "C" void Transfarring_ClearTransfarringError()
{
   CTransfarringManager::Instance()->ClearTransfarringError();
}

extern "C" void Transfarring_SetTrophyInvalidFlag( SBP_LinkVars* bplv )
{
   bplv->mTrophyValidFlag = BPLinkvarDefines::kTrophiesInvalid;
}

extern "C" int Transfarring_SetTrophyInvalidFlagIfApplicable( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   int retval = 0;

   if( !Transfarring_CheckTrophyCompatibility( bplv, saveType, saveIndex ) )
   {
      bplv->mTrophyValidFlag = BPLinkvarDefines::kTrophiesInvalid;
      retval = 1;
   }

   return retval;
}

extern "C" int Transfarring_WillTrophiesBecomeInvalid( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   return bplv->mTrophyValidFlag == 
      BPLinkvarDefines::kTrophiesValid 
      && !Transfarring_CheckTrophyCompatibility( bplv, saveType, saveIndex );
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_LoadFileList()
{
   return CTransfarringManager::Instance()->PostNetworkThreadWork_LoadFileList();
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_UnloadFileList()
{
   return CTransfarringManager::Instance()->PostNetworkThreadWork_UnloadFileList();
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_CleanupSystem()
{
   return CTransfarringManager::Instance()->PostNetworkThreadWork_CleanupSystem();
}

extern "C" void Transfarring_MakeAccessoryData( 
   SCloudAccessoryData * const pOut, 
   TRANSFARRING_GAME_DATA const * const pGameData, 
   TransfarringSaveFile* pSaveFile )
{
   CTransfarringManager::Instance()->MakeAccessoryData( pOut, pGameData, boost::shared_ptr<TransfarringSaveFile>( pSaveFile ) );
}

#else // Win32 stubs

extern "C" void Transfarring_InitializeSystems()
{
}

extern "C" void Transfarring_LockSaveLoadMutex()
{
}

extern "C" void Transfarring_UnlockSaveLoadMutex()
{
}

extern "C" int Transfarring_GetTransfarringStatus()
{
   return 0;
}

extern "C" int Transfarring_GetProgress()
{
   return 0;
}

extern "C" int Transfarring_IsTransfarringStatusIdle()
{
   return 1;
}

extern "C" int Transfarring_IsTransfarringStatusError()
{
   return 0;
}

extern "C" int Transfarring_IsFileLocked(ESaveType saveType, int saveIndex)
{
   return 0;
}

extern "C" int Transfarring_IsFileLinked(ESaveType saveType, int saveIndex)
{
   return 0;
}

extern "C" int Transfarring_IsFileCloud(ESaveType saveType, int saveIndex)
{
   return 0;
}

extern "C" int Transfarring_HasOfflineWork(ESaveType saveType, int saveIndex)
{
   return 0;
}

extern "C" int Transfarring_HasCloudConflict( ESaveType saveType, int saveIndex )
{
   return 0;
}

extern "C" int Transfarring_IsLocalFileCorrupt( ESaveType saveType, int saveIndex )
{
   return 0;
}

extern "C" int Transfarring_IsLocalFileCorrupt_Platform( ESaveType saveType, int saveIndex )
{
   return 0;
}

extern "C" void Transfarring_SetLocalFileCorrupt_Platform( int isCorrupt, ESaveType saveType, int saveIndex )
{

}

extern "C" int Transfarring_IsRemoteFileCloud(TransfarringSaveFile* saveFile)
{
   return 0;
}

extern "C" int Transfarring_IsRemoteFileLinked(TransfarringSaveFile* saveFile)
{
   return 0;
}

extern "C" int Transfarring_CanLoadFile(ESaveType saveType, int saveIndex)
{
   return 1;
}

extern "C" int Transfarring_CheckTrophyValidFlag( const SBP_LinkVars* bplv )
{
   return 1;
}

extern "C" int Transfarring_CheckTrophyCompatibility( const SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   return 1;
}

extern "C" int Transfarring_CheckTrophyCompatibilityTID( const SBP_LinkVars* bplv, unsigned char* tid )
{
   return 1;
}

extern "C" void Transfarring_TryWriteOriginatorData( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
}

extern "C" int Transfarring_IsLocalFileListValid()
{
   return 0;
}

extern "C" int Transfarring_GetNumLocalSaveFiles(ESaveType saveType)
{
   return 0;
}

extern "C" TransfarringSaveFile* Transfarring_GetLocalSaveFile(ESaveType saveType, int index)
{
   return NULL;
}

extern "C" int Transfarring_IsCloudFileListValid()
{
   return 0;
}

extern "C" int Transfarring_GetNumCloudSaveFiles(ESaveType saveType)
{
   return 0;
}

extern "C" TransfarringSaveFile* Transfarring_GetCloudSaveFile(ESaveType saveType, int index)
{
   return NULL;
}

extern "C" int Transfarring_IsCloudFileCorrupt( ESaveType saveType, int index )
{
   return 0;
}

extern "C" STransfarringError Transfarring_PostWork_FlagCloudFile(ESaveType saveType, int index)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostWork_WillTrophiesBeDisabled(ESaveType saveType, int index)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostWork_UnflagCloudFile( unsigned char const * const transfarringID )
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

#if MGS_VERSION == 2
extern "C" STransfarringError Transfarring_PostWork_SyncCloudFiles(MCMAN_WORK* mcActor, int syncAllTypes, int treatVersionMismatchesAsConflicts)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostWork_SyncSingleCloudFile(
   MCMAN_WORK* mcActor, 
   ESaveType saveType, 
   int saveIndex,
   ECloudConflictResolution conflictResolution,
   int treatVersionMismatchesAsConflicts )
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}
#endif

#if MGS_VERSION == 3
extern "C" STransfarringError Transfarring_PostWork_SyncCloudFiles(void* mcActor, int syncAllTypes, int treatVersionMismatchesAsConflicts)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostWork_SyncSingleCloudFile(
   void* mcActor, 
   ESaveType saveType, 
   int saveIndex,
   ECloudConflictResolution conflictResolution,
   int treatVersionMismatchesAsConflicts )
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}
#endif

extern "C" STransfarringError Transfarring_ProcessWork()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" void Transfarring_SetOfflineWork(ESaveType saveType, int saveIndex, int isOffline)
{
}

extern "C" STransfarringError Transfarring_PostWork_UnlockSaveFile(ESaveType saveType, int saveIndex)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostWork_CheckSyncNeeded( ESaveType saveType, int treatVersionMismatchesAsConflicts )
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_GetWifiTransfarringError()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" void Transfarring_ClearTransfarringError()
{
}

extern "C" void Transfarring_LockTransfarringIDUpdates()
{
}

extern "C" void Transfarring_AllowTransfarringIDUpdates()
{
}

extern "C" void Transfarring_SetTrophyInvalidFlag( SBP_LinkVars* bplv )
{
}

extern "C" int Transfarring_SetTrophyInvalidFlagIfApplicable( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   return 0;
}

extern "C" int Transfarring_WillTrophiesBecomeInvalid( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   return 0;
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_LoadFileList()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_UnloadFileList()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" STransfarringError Transfarring_PostNetworkThreadWork_CleanupSystem()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID(error);
   return error;
}

extern "C" void Transfarring_MakeAccessoryData( 
   SCloudAccessoryData * const pOut, 
   TRANSFARRING_GAME_DATA const * const pGameData, 
   TransfarringSaveFile* pSaveFile )
{
}

extern "C" void Transfarring_MakeNewTransfarringID( ESaveType saveType, int saveIndex )
{
}

extern "C" void Transfarring_ClearOriginatorData( SBP_LinkVars* bplv )
{
}

extern "C" void Transfarring_SetLoadedTransfarringID( ESaveType saveType, int saveIndex )
{
}

extern "C" void Transfarring_SetLoadedTransfarringIDByString( unsigned char* tid )
{
}

extern "C" void Transfarring_GetLoadedTransfarringID( unsigned char* tid )
{
}

#endif

// Functions shared between Win32 and consoles
extern "C" int Transfarring_SaveIndexStringToInt(const char* str)
{
#if MGS_VERSION == 2
   return atoi(str);
#endif

#if MGS_VERSION == 3
   // Convert from hex string to number
   int retval = -1;
   std::stringstream ss;
   ss << std::hex << str;
   ss >> retval;

   return retval;
#endif
}

extern "C" ESaveType Transfarring_KonamiSaveTypeToBPSaveType(int file_kind)
{
   ESaveType saveType;

#if MGS_VERSION == 2
   switch(file_kind)
   {
   case MCMAN_FILE_KIND_VR:
      {
         saveType = kST_VR;
         break;
      }
   case MCMAN_FILE_KIND_SNAKE_TALES:
      {
         saveType = kST_SnakeTales;
         break;
      }
   default:
      saveType = kST_Game;
      break;
   }
#elif MGS_VERSION == 3
   saveType = kST_Game;
#endif

   return saveType;
}