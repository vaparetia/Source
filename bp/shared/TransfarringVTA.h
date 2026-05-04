//----------------------------------------------------------------------------
// TransfarringVTA.h
//
// Vita-specific Transfarring system
//----------------------------------------------------------------------------

#pragma once
#include <kernel/common.h>
#include <kernel/cpu.h>
#include <kernel/dipsw.h>
#include <kernel/sysmem.h>
#include <kernel/threadmgr.h>
#include <kernel/iofilemgr.h>
#include <kernel/modulemgr.h>
#include <kernel/processmgr.h>
#include <kernel/process_param.h>
#include <kernel/libkernel.h>

#include <kernel/debug.h>
#include <kernel/backtrace.h>

#include <net.h>
#include <ult.h>
#include <sdk_version.h>

#include <string>
#include <map>
#include <vector>

#include "TransfarringID.h"
#include "TransfarringDefines.h"
#include "TransfarringMemoryCardFileList.h"
#include "TransfarringCloudFileList.h"

#include "Engine/System/CSyncEvent.h"

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"

class CTransfarringManager
{
public:
   typedef boost::shared_ptr<TransfarringSaveFile> TTransfarringSaveFilePtr;
   
   ~CTransfarringManager();

   static CTransfarringManager* Instance();
   static void InitSingleton();

   // Connections
   void InitNetwork();
   bool IsConnectedToPS3();

   STransfarringError PostNetworkThreadWork_LoadFileList();
   STransfarringError PostNetworkThreadWork_ConnectToPS3();
   STransfarringError PostNetworkThreadWork_DisconnectFromPS3();
   STransfarringError PostNetworkThreadWork_UnloadFileList();
   STransfarringError PostNetworkThreadWork_CleanupSystem();

   void LockSaveLoadMutex();
   void UnlockSaveLoadMutex();

   EVitaTransfarringStatus GetTransfarringStatus();
   void SetTransfarringStatus(EVitaTransfarringStatus status);

   int GetNumLocalSaveFiles(ESaveType saveType);
   TTransfarringSaveFilePtr const GetLocalSaveFile(ESaveType saveType, ConcatenatedSaveIndex saveIndex);
   int GetNumCloudSaveFiles(ESaveType saveType);
   TTransfarringSaveFilePtr const GetCloudSaveFile(ESaveType saveType, ConcatenatedSaveIndex saveIndex);
   bool HasCloudConflict( ESaveType saveType, int saveIndex );
   bool IsCloudFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex saveIndex ) const;
   bool IsLocalFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex saveIndex );

   void SetLocalFileCorrupt_Platform( bool isCorrupt, ESaveType saveType, int saveIndex );
   bool IsLocalFileCorrupt_Platform( ESaveType saveType, int saveIndex );

   // Transfarring ID management
   void SetTransfarringID(TransfarringID id, ESaveType saveType, int saveIndex);
   TransfarringID GetTransfarringID(ESaveType saveType, int saveIndex);
   void GetSaveFileFromUniqueID(const unsigned char* uid, ESaveType& saveType, int& saveIndex);

   int GetProgress();
   void StartProgress(int numPhases);
   void SetProgress(int progress);
   void EndProgressPhase();
   void ClearProgress();

   bool DoUniqueIDsMatch(const unsigned char* a, const unsigned char* b);

   bool NeedsRealDelete();

   static void GetConsoleOpenPSID( unsigned char* dst );
   static void GetPSNAccountHash( bool useCached, unsigned char* dst, unsigned char* transfarringID );

   void LockTransfarringIDUpdates();
   void AllowTransfarringIDUpdates();

   // Work functions
   bool CanPostWork();
   STransfarringError PostWork_FlagCloudFile(ESaveType saveType, int index);
   STransfarringError PostWork_UnflagCloudFile( unsigned char const * const transfarringID );
   STransfarringError PostWork_SyncCloudFiles(MEMORY_CARD_ACTOR* mcActor, bool syncAllTypes, bool treatVersionMismatchesAsConflicts);
   STransfarringError PostWork_UnlockSaveFile(ESaveType saveType, int saveIndex);
   STransfarringError PostWork_CheckSyncNeeded( ESaveType saveType, bool treatVersionMismatchesAsConflicts );
   STransfarringError PostWork_DeleteSaveFile(MEMORY_CARD_ACTOR* mcActor, ESaveType saveType, int saveIndex);
   STransfarringError PostWork_DeleteNonGameFile(MEMORY_CARD_ACTOR* mcActor, ENonGameSaveType saveType, int saveIndex);
   STransfarringError PostWork_SyncSingleCloudFile(
      MEMORY_CARD_ACTOR* mcActor, 
      ESaveType saveType, 
      int saveIndex, 
      ECloudConflictResolution conflictResolution, 
      bool treatVersionMismatchesAsConflicts
      );
   STransfarringError PostWork_WillTrophiesBeDisabled( ESaveType saveType, int index );

   // When you post work to the transfarring manager, you must consume it by calling ProcessWork
   STransfarringError ProcessWork();

   int32_t NetworkThread();
   void NetworkCallback(int eventType, void *arg);

   // If the transfarring status is kVTS_Error, call this function to get the error and handle it appropriately
   STransfarringError GetWiFiTransfarringError();
   // If a transfarring error occurs during WiFi transfarring, you must clear the error before continuing
   void ClearTransfarringError();

   // Stubs for PS3 endian swapping functions (all endian swapping actually happens on PS3)
   TransfarringSaveFile EndianSwapInfoFile(const TransfarringSaveFile& infoFile) const;
   void EndianSwapGameData(TRANSFARRING_GAME_DATA* dst, const TRANSFARRING_GAME_DATA* src, ESaveType saveType) const;
   SCloudAccessoryData EndianSwapAccessoryData( SCloudAccessoryData const & accessoryData ) const;

   bool CheckTrophyCompatibility( const SBP_LinkVars* bplv, unsigned char* uniqueID );
   bool CheckTrophyCompatibility( const SBP_LinkVars* bplv, ESaveType saveType, int saveIndex );
   bool CanTrophiesBeUnlockedLocally( const SBP_LinkVars* bplv );
   bool ShouldAccumulatedTrophiesUnlock( const SBP_LinkVars* bplv, unsigned char* uniqueID );

   // Processing an incoming save file to do stuff like unlocking trophies
   void HandleIncomingSaveFile( TRANSFARRING_GAME_DATA* saveFile, bool isWiFi, unsigned char* uniqueID );

   // Try to write/clear originator data, may fail
   bool TryWriteOriginatorData( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex, bool updateLoadedTransfarringId );
   void OverwriteOriginatorData( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex );

   // Force clear originator data
   void ClearOriginatorData( SBP_LinkVars* bplv );

   bool IsLocalFileListValid() const;

   bool IsAccessoryDataValid( SCloudAccessoryData const & accessoryData ) const;
   void MakeAccessoryData( 
      SCloudAccessoryData * const pOut, 
      TRANSFARRING_GAME_DATA const * const pGameData, 
      boost::shared_ptr<TransfarringSaveFile> const & pSaveFile 
      ) const;

   void SetOverrideSaveDate( bool overrideSaveDate, const STransfarringSaveDate* pDate );
   bool GetOverrideSaveDate( STransfarringSaveDate* pOut );

   void GetCurrentLoadedTransfarringID( unsigned char* tid ) const;
   void SetCurrentLoadedTransfarringID( const unsigned char* tid );

   bool m_bNeedsRealDelete;

private:
   // Cloud processing work
   enum ETransfarringWork
   {
      kTW_Idle,
      kTW_FlagCloudFile,
      kTW_UnflagCloudFile,
      kTW_SyncCloudFiles,
      kTW_SyncSingleCloudFile,
      kTW_CheckSyncNeeded,
      kTW_UnlockSaveFile,
      kTW_DeleteSaveFile,
      kTW_DeletePhotoFile,
      kTW_CheckTrophiesWillBeDisabled,
      kTW_MaxNumCloudWork
   };

   // Network thread processing work
   enum EVitaNetworkThreadWork
   {
      kNW_Idle,
      kNW_LoadFileList,
      kNW_ConnectToPS3,
      kNW_DisconnectFromPS3,
      kNW_UnloadFileList,
      kNW_CleanupSystem,
      kNW_MaxNumNetworkThreadWork
   };

   // Results of a network operation
   enum ENetworkOperationResult
   {
      kNOR_Success,
      kNOR_Disconnected,
      kNOR_VersionMismatch,
      kNOR_NoDataReceived,
      kNOR_MaxNumOperations,
   };
   
   typedef std::map<int, TransfarringID> TIDMapEntry;

   // Private constructor
   CTransfarringManager();

   // Network functions
   void CreateNetworkThread();
   void CloseNetworkThread();
   void WaitForErrorResolution_NetworkThread();

   bool CanPostNetworkThreadWork() const;
   void ProcessNetworkThreadWork_NetworkThread();
   void ProcessNetworkMessage_NetworkThread( const char msg );

   void FinalizePostedWork_NetworkThread();

   void LoadFileLists_NetworkThread();
   void UnloadFileLists_NetworkThread();
   void ConnectToPS3_NetworkThread();
   void DisconnectFromPS3_NetworkThread();
   void CleanupSystem_NetworkThread();

   void SendPacket_NetworkThread( void const * const pData, unsigned int dataSize ) const;
   ENetworkOperationResult ReceiveAndVerifyPacket_NetworkThread( void * const pData, unsigned int dataSize, bool shouldWait = true ) const;

   void CreateSaltedHash( 
      void * const pHash, 
      void const * const pData, 
      unsigned int dataSize, 
      void const * const pSalt, 
      unsigned int saltSize ) const;

   // Cloud functions
   STransfarringError ProcessFlagCloudFile();
   STransfarringError ProcessUnflagCloudFile();
   STransfarringError ProcessSyncCloudFiles();
   STransfarringError ProcessSyncSingleCloudFile();
   STransfarringError ProcessCheckSyncNeeded();
   STransfarringError ProcessGetCloudFileList();
   STransfarringError ProcessUnlockSaveFile();
   STransfarringError ProcessDeleteSaveFile();
   STransfarringError ProcessDeleteNonGameFile();
   STransfarringError ProcessCheckTrophiesWillBeDisabled();

   // Memory card actors
   void CreateMemoryCardActor_NetworkThread();
   void DestroyMemoryCardActor_NetworkThread();
   void SetMemoryCardActor(MEMORY_CARD_ACTOR* mcActor);

   void UnlockSaveFileInternal(ESaveType saveType, int saveIndex);
   void DeleteSaveFileInternal(ESaveType saveType, int saveIndex);

   static void GetPSNAccountPassphrase( unsigned char* dst ); // 16 bytes

   // Singleton
   static CTransfarringManager* m_instance;

   // Thread variables
   SceUltUlthreadRuntime m_NetworkULTRuntime;
   SceUltUlthread m_NetworkThread;
   void *m_pRuntimeWorkArea;
   void *m_pThreadContext;
   bool mIsNetworkInitialized;
   volatile bool mIsNetworkThreadRunning;

   volatile bool m_isConnectedToPS3;
   volatile bool m_hasDisconnectedFromPS3;
   
   EVitaTransfarringStatus m_status;
   TransfarringMemoryCardFileList m_LocalFileList;
   TransfarringCloudFileList m_CloudFileList;

   // Transfarring IDs

   bpe::reserved_vector< TIDMapEntry, kST_MaxSaveTypes > mIDMap;

   bool m_AllowIDUpdates;

   int m_numPhases;
   int m_curPhase;
   int m_curProgress;

   // Error handling
   STransfarringError m_wifiError;
   CSyncEvent m_errorSyncEvent;
   bool m_bIsWaitingForErrorResolution;

   bool m_bUsingExternalMemoryCardActor;

   // Game thread work input
   ETransfarringWork mGameThreadWorkInput_WorkType;
   ESaveType mGameThreadWorkInput_SaveType;
   ENonGameSaveType mGameThreadWorkInput_NonGameSaveType;
   int mGameThreadWorkInput_SaveIndex;
   ECloudConflictResolution mGameThreadWorkInput_ConflictResolution;
   bool mGameThreadWorkInput_SyncAllTypes;
   bool mGameThreadWorkInput_TreatVersionMismatchesAsConflicts;
   unsigned char mGameThreadWorkInput_TransfarringID[TransfarringDefines::kHashLengthBytes]; // Used to look up cloud file by ID

   // Network thread work input
   volatile EVitaNetworkThreadWork mNetworkThreadWorkInput_WorkType;
   volatile bool mNetworkThreadWorkPosted;

   // Network thread variables
   CSyncEvent mNetworkThreadSyncEvent;
   bool mNeedsMemoryCardCleanup;
   bool mAreFileListsValid;
   char mSaltString[255];

   // PS3 SKUs
   char const* mPS3SKU;

   // Overriding save date
   bool mShouldOverrideSaveDate;
   STransfarringSaveDate mOverrideSaveDate;

   // Keeping track of the TransfarringID of the currently loaded game
   unsigned char mLoadedTransfarringID[TransfarringDefines::kHashLengthBytes];
};
