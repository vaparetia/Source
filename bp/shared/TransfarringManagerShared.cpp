//------------------------------------------------------------------------------------------
// TransfarringManagerShared.cpp
//
// Transfarring manager functions shared between platforms/games
//------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"

#include "MGS_SysCommon.h"

#include <queue>

#include "ITUSInterface.h"
#include "BP_TUS.h"
#include "BP_EndianSupport.h"
#include "BP_TrophySystem.h"
#include "BP_SaveLoad.h"
#include "BP_SaveLoadMGS.h"
#include "BP_MD5.h"
#include "BP_Network.h"

#if defined(BP_VITA)
#include "TransfarringVTA.h"
#include "TransfarringVTACGlue.h"
#elif defined (BP_PS3)
#include "Transfarring_PS3.h"
#include "Transfarring_PS3CGlue.h"

#include <sys/synchronization.h>

// Replacement for CSyncEvent
uint64 const skPS3StartWorkFlag = 1ULL;

#endif

//Hack for MGSTWO-3220.
extern "C" int gSyncSlotsRemaining = 0;

static const unsigned char sPassphraseUnscrambled1[TransfarringDefines::kHashLengthBytes] = 
{ 
   0x34, 
   0x71, 
   0xE0,
   0x04, 
   0x2E, 
   0x87, 
   0x0E, 
   0x5D, 
   0xC0, 
   0x22, 
   0xBD,
   0x64,
   0x41,
   0xE4,
   0x07,
   0x1F,
};

static const unsigned char sPassphraseUnscrambled2[TransfarringDefines::kHashLengthBytes] = 
{ 
   0x63,
   0xD1,
   0xEB,
   0xCD,
   0x58,
   0x43,
   0xAB,
   0xA9,
   0x54,
   0x08,
   0x0C,
   0x6E,
   0x0D,
   0xD2,
   0x83,
   0xEC, 
};

static const unsigned char sPassphraseScrambleOrder[TransfarringDefines::kHashLengthBytes] =
{
   12, 6, 7, 0, 4, 13, 1, 8, 3, 11, 9, 15, 2, 5, 10, 14
};

enum EScrambleByteOperation
{
   kSBO_XOR,
   kSBO_AND,
   kSBO_NOTAND,
   kSBO_OR,
};

static const EScrambleByteOperation sPassphraseScrambleOperations[TransfarringDefines::kHashLengthBytes] =
{
   kSBO_AND,
   kSBO_OR,
   kSBO_NOTAND,
   kSBO_NOTAND,
   kSBO_AND,
   kSBO_NOTAND,
   kSBO_XOR,
   kSBO_NOTAND,
   kSBO_XOR,
   kSBO_XOR,
   kSBO_OR,
   kSBO_XOR,
   kSBO_AND,
   kSBO_OR,
   kSBO_XOR,
   kSBO_OR,
};

void CTransfarringManager::SetTransfarringID(TransfarringID id, ESaveType saveType, int saveIndex)
{
   if (!m_AllowIDUpdates)
   {
      return;
   }

   BPE_ASSERT( saveType < kST_MaxSaveTypes, "CTransfarringManager::SetTransfarringID - Bad save type" );
   if ( saveType >= kST_MaxSaveTypes )
   {
      return;
   }

   TIDMapEntry &idMapEntry = mIDMap[ saveType ];
   idMapEntry[ saveIndex ] = id;

   // Update the local save file list
   m_LocalFileList.UpdateTransfarringID(id, saveType, saveIndex);
}

TransfarringID CTransfarringManager::GetTransfarringID(ESaveType saveType, int saveIndex)
{
   BPE_ASSERT( saveType < kST_MaxSaveTypes, "CTransfarringManager::SetTransfarringID - Bad save type" );
   if ( saveType >= kST_MaxSaveTypes )
   {
      return TransfarringID();
   }

   TIDMapEntry const &idMapEntry = mIDMap[ saveType ];

   std::map<int, TransfarringID>::const_iterator submapIter = idMapEntry.find(saveIndex);
   if (submapIter == idMapEntry.end())
   {
      return TransfarringID();
   }

   return submapIter->second;
}

void CTransfarringManager::GetSaveFileFromUniqueID(const unsigned char* uid, ESaveType& saveType, int& saveIndex)
{
   unsigned char zeroMem[TransfarringDefines::kHashLengthBytes];
   memset( zeroMem, 0, sizeof(zeroMem) );

   if( memcmp( zeroMem, uid, sizeof(zeroMem) ) )
   {
      // Ugly way to look up value by key
      for ( int saveTypeIter = 0; saveTypeIter < kST_MaxSaveTypes; ++saveTypeIter )
      {
         TIDMapEntry const &mapEntry = mIDMap[ saveTypeIter ];

         for ( TIDMapEntry::const_iterator it = mapEntry.begin(); it != mapEntry.end(); ++it )
         {
            if( DoUniqueIDsMatch( uid, reinterpret_cast<const unsigned char*>( it->second.m_UniqueID ) ) )
            {
               saveType = ESaveType( saveTypeIter );
               saveIndex = it->first;
               return;
            }
         }
      }
   }

   // Otherwise we fall through
   saveType = kST_MaxSaveTypes;
   saveIndex = -1;

}

int CTransfarringManager::GetProgress()
{
   if (m_numPhases == 0)
   {
      return 0;
   }

   return m_curProgress / m_numPhases;
}

void CTransfarringManager::StartProgress(int numPhases)
{
   m_numPhases = numPhases;
   m_curProgress = 0;
}

void CTransfarringManager::SetProgress(int progress)
{
   m_curProgress = 100 * m_curPhase + progress;
}

void CTransfarringManager::EndProgressPhase()
{
   ++m_curPhase;
   if (m_numPhases == m_curPhase)
   {
      ClearProgress();
   }
}

void CTransfarringManager::ClearProgress()
{
   m_curPhase = 0;
   m_numPhases = 0;
   m_curProgress = 0;
}

void CTransfarringManager::GetPSNAccountPassphrase( unsigned char* dst )
{
   for( unsigned int i=0; i < sizeof(sPassphraseUnscrambled1); i++ )
   {
      switch( sPassphraseScrambleOperations[i] )
      {
      case kSBO_XOR:
         dst[i] = sPassphraseUnscrambled1[i] ^ sPassphraseUnscrambled2[sPassphraseScrambleOrder[i]];
         break;
      case kSBO_AND:
         dst[i] = sPassphraseUnscrambled1[i] & sPassphraseUnscrambled2[sPassphraseScrambleOrder[i]];
         break;
      case kSBO_NOTAND:
         dst[i] = sPassphraseUnscrambled1[i] & ~sPassphraseUnscrambled2[sPassphraseScrambleOrder[i]];
         break;
      default:
      case kSBO_OR:
         dst[i] = sPassphraseUnscrambled1[i] | sPassphraseUnscrambled2[sPassphraseScrambleOrder[i]];
         break;
      }
   }
}

bool CTransfarringManager::DoUniqueIDsMatch(const unsigned char* a, const unsigned char* b)
{
   return !memcmp( a, b, TransfarringDefines::kHashLengthBytes );
}

TransfarringSaveFile CTransfarringManager::EndianSwapInfoFile(const TransfarringSaveFile& infoFile) const
{
   // Don't swap the file that's passed in, as it will likely be a pointer to an internal structure, we only want to do this when syncing from the server
   TransfarringSaveFile returnFile;
   memcpy(&returnFile, &infoFile, sizeof(returnFile));

   BP_LE_Swap4Bytes_Inp(&returnFile.mSaveIndex);
   BP_LE_Swap4Bytes_Inp(&returnFile.mStageIndex);
   BP_LE_Swap4Bytes_Inp(&returnFile.mDifficulty);
   BP_LE_Swap4Bytes_Inp(&returnFile.mPlayTime);
   BP_LE_Swap4Bytes_Inp(&returnFile.mSaveType);
   BP_LE_Swap4Bytes_Inp(&returnFile.mSaveDate.mYear);
   BP_LE_Swap4Bytes_Inp(&returnFile.mSaveDate.mMonth);
   BP_LE_Swap4Bytes_Inp(&returnFile.mSaveDate.mDay);
   BP_LE_Swap2Bytes_Inp(&returnFile.mClearCount);

   // Don't need to swap the MD5 hash

   return returnFile;
}

void CTransfarringManager::HandleIncomingSaveFile( TRANSFARRING_GAME_DATA* saveFile, bool isWiFi, unsigned char* uniqueID )
{
   // If we received a file with a null PSN hash, then disable trophies.
   if( BP_IsMD5HashZero( saveFile->mBPLinkvars.mOriginalPSNAccount ) )
   {
       saveFile->mBPLinkvars.mTrophyValidFlag = BPLinkvarDefines::kTrophiesInvalid;
   }

   bool const bShouldUnlockTrophies = ShouldAccumulatedTrophiesUnlock( &saveFile->mBPLinkvars, uniqueID );

   if( bShouldUnlockTrophies )
   {
      // Unlock trophies
      BP_TrophySystem_UnlockTrophiesFromBitfield( &saveFile->mBPLinkvars.mTrophyDeltaBitfield );
   }
}

// This function should be called any time a file is saved from an in-game menu.
// It writes originator data, but only if the originator data hasn't already been written.
bool CTransfarringManager::TryWriteOriginatorData( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex, bool updateLoadedTransfarringId )
{
#if BP_PS3
   BPLinkvarDefines::EConsoleTypes const currentSystem = BPLinkvarDefines::kConsolePS3;
#elif BP_VITA
   BPLinkvarDefines::EConsoleTypes const currentSystem = BPLinkvarDefines::kConsoleVita;
#else
   BPLinkvarDefines::EConsoleTypes const currentSystem = BPLinkvarDefines::kConsoleInvalid;
#endif

#if BP_PS3
   bool const legacySave = bplv->mOriginalConsoleType == BPLinkvarDefines::kConsoleLegacyPS3;
#else
   bool const legacySave = false;
#endif

   bool retval = false;

   // Write originator data if empty or legacy
   if ( bplv->mOriginalConsoleType == 0  || legacySave )
   {
      bplv->mOriginalConsoleType = currentSystem;

      unsigned char consolePSID[TransfarringDefines::kHashLengthBytes];
      memset( consolePSID, 0, sizeof( consolePSID ) );
      CTransfarringManager::GetConsoleOpenPSID(consolePSID);
      memcpy( bplv->mOriginalConsolePSID, consolePSID, sizeof( consolePSID ) );

      TransfarringID const tid = GetTransfarringID( saveType, saveIndex );

      unsigned char PSNAccount[TransfarringDefines::kHashLengthBytes];
      memset( PSNAccount, 0, sizeof( PSNAccount ) );

      //If this is the first save and we are not logged into PSN or if this is a legacy save, then write a blank hash.
#if BP_PS3
      if( ( bplv->mVersion != 0 || BP_Network_IsSignedIn() ) && !legacySave )
#else
      if( bplv->mVersion != 0 || BP_Network_Vita_IsPSNSignedIn() )
#endif
      {
         CTransfarringManager::GetPSNAccountHash( true, PSNAccount, (unsigned char*)tid.m_UniqueID );
      }

      memcpy( bplv->mOriginalPSNAccount, PSNAccount, sizeof( PSNAccount ) );

      if(updateLoadedTransfarringId && CTransfarringManager::Instance())
      {
         CTransfarringManager::Instance()->SetCurrentLoadedTransfarringID( (unsigned char*)tid.m_UniqueID );
      }
      retval = true;
   }

   return retval;
}

//Force update the originator data using cached PSN information
void CTransfarringManager::OverwriteOriginatorData( SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   unsigned char consolePSID[TransfarringDefines::kHashLengthBytes];
   memset( consolePSID, 0, sizeof( consolePSID ) );

   unsigned char PSNAccount[TransfarringDefines::kHashLengthBytes];
   memset( PSNAccount, 0, sizeof( PSNAccount ) );

   CTransfarringManager::GetConsoleOpenPSID(consolePSID);

   TransfarringID tid = GetTransfarringID( saveType, saveIndex );
   CTransfarringManager::GetPSNAccountHash( true, PSNAccount, (unsigned char*)tid.m_UniqueID );

#if BP_PS3
   BPLinkvarDefines::EConsoleTypes currentSystem = BPLinkvarDefines::kConsolePS3;
#elif BP_VITA
   BPLinkvarDefines::EConsoleTypes currentSystem = BPLinkvarDefines::kConsoleVita;
#else
   BPLinkvarDefines::EConsoleTypes currentSystem = BPLinkvarDefines::kConsoleInvalid;
#endif

   bplv->mOriginalConsoleType = currentSystem;
   memcpy( bplv->mOriginalConsolePSID, consolePSID, sizeof( consolePSID ) );
   memcpy( bplv->mOriginalPSNAccount, PSNAccount, sizeof( PSNAccount ) );
}

bool CTransfarringManager::CheckTrophyCompatibility( const SBP_LinkVars* bplv, ESaveType saveType, int saveIndex )
{
   TransfarringID tid = GetTransfarringID( saveType, saveIndex );
   return CheckTrophyCompatibility( bplv, (unsigned char*)tid.m_UniqueID );
}

bool CTransfarringManager::CheckTrophyCompatibility( const SBP_LinkVars* bplv, unsigned char* uniqueID )
{
   // Invalidate trophies if the originator data check fails
   BPE_CTASSERT( sizeof(long long) == sizeof(unsigned char) * 8 );
   BPE_CTASSERT( sizeof(bplv->mOriginalConsolePSID) == 16 );
   BPE_CTASSERT( sizeof(bplv->mOriginalPSNAccount) == 16 );

   const int MD5HashBytes = 16;
   unsigned char curConsolePSID[MD5HashBytes];
   unsigned char curPSNAccount[MD5HashBytes];

   CTransfarringManager::GetConsoleOpenPSID(curConsolePSID);
   CTransfarringManager::GetPSNAccountHash( true, curPSNAccount, uniqueID );

   // Trophies are valid if:
   // There is no originator data (meaning that the save file is from vanilla HD)
   bool returnVal = true;
   if( bplv->mOriginalConsoleType != 0 )
   {
      // If the PSN ID is the same between the current environment and the originator data
      if (memcmp(curPSNAccount, bplv->mOriginalPSNAccount, MD5HashBytes))
      {
         // Zeroed hashes will be considered compatible so that we can still unlock trophies after signing up for PSN
         // The trophy invalid flag is always set when transfarring a zeroed PSN hash, so this won't cause ownership issues
         if( !BP_IsMD5HashZero( bplv->mOriginalPSNAccount ) )
         {
            // If all these checks have failed, disable the trophy system
            returnVal = false;
         }
      }
   }

   return returnVal;
}

bool CTransfarringManager::ShouldAccumulatedTrophiesUnlock( const SBP_LinkVars* bplv, unsigned char* uniqueID )
{
#if BP_VITA
   BPLinkvarDefines::EConsoleTypes curPlatform = BPLinkvarDefines::kConsoleVita;
#elif BP_PS3
   BPLinkvarDefines::EConsoleTypes curPlatform = BPLinkvarDefines::kConsolePS3;
#else
   BPLinkvarDefines::EConsoleTypes curPlatform = BPLinkvarDefines::kConsoleInvalid;
#endif
   bool retval = false;

   // When a save file is transfarred in, all of the accumulated trophies will unlock only if all of the following are true:
   // The save file's "Trophy Valid" flag is set
   if( bplv->mTrophyValidFlag == BPLinkvarDefines::kTrophiesValid )
   {
      // The save file contains compatible originator data
      if( CheckTrophyCompatibility( bplv, uniqueID) )
      {
         retval = true;
      }
   }

   return retval;
}

void CTransfarringManager::LockTransfarringIDUpdates()
{
   BPE_ASSERT(m_AllowIDUpdates, "CTransfarringManager::LockTransfarringIDUpdates - Tried to lock ID updates when they were already locked");
   m_AllowIDUpdates = false;
}

void CTransfarringManager::AllowTransfarringIDUpdates()
{
   BPE_ASSERT(!m_AllowIDUpdates, "CTransfarringManager::LockTransfarringIDUpdates - Tried to allow ID updates when they were already allowed");
   m_AllowIDUpdates = true;
}

STransfarringError CTransfarringManager::GetWiFiTransfarringError()
{
   return m_wifiError;
}

bool CTransfarringManager::HasCloudConflict( ESaveType saveType, int saveIndex )
{
   return m_LocalFileList.HasConflict( saveType, saveIndex );
}

bool CTransfarringManager::IsCloudFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex saveIndex ) const
{
   return m_CloudFileList.IsCloudFileCorrupt( saveType, saveIndex );
}

bool CTransfarringManager::IsLocalFileCorrupt( ESaveType saveType, ConcatenatedSaveIndex saveIndex )
{
   return m_LocalFileList.IsFileCorrupt( saveType, saveIndex );
}

void CTransfarringManager::SetLocalFileCorrupt_Platform( bool isCorrupt, ESaveType saveType, int saveIndex )
{
   m_LocalFileList.SetFileCorrupt_Platform( isCorrupt, saveType, saveIndex );
}

bool CTransfarringManager::IsLocalFileCorrupt_Platform( ESaveType saveType, int saveIndex )
{
   return m_LocalFileList.IsFileCorrupt_Platform( saveType, saveIndex );
}

void CTransfarringManager::SetOverrideSaveDate( bool overrideSaveDate, const STransfarringSaveDate* pDate )
{
   mShouldOverrideSaveDate = overrideSaveDate;

   if( mShouldOverrideSaveDate )
   {
      mOverrideSaveDate = *pDate;
   }
}

bool CTransfarringManager::GetOverrideSaveDate( STransfarringSaveDate* pOut )
{
   if( mShouldOverrideSaveDate )
   {
      *pOut = mOverrideSaveDate;
   }

   return mShouldOverrideSaveDate;
}

void CTransfarringManager::ClearTransfarringError()
{
   if (m_bIsWaitingForErrorResolution)
   {
#if defined(BP_VITA)
      m_errorSyncEvent.Set();
#elif defined(BP_PS3)
      sys_event_flag_set( mPS3WifiErrorSyncEvent, skPS3StartWorkFlag );
#endif
   }

   SET_TRANSFARRING_ERROR_INVALID(m_wifiError);

   m_bIsWaitingForErrorResolution = false;
}

void CTransfarringManager::WaitForErrorResolution_NetworkThread()
{
   m_bIsWaitingForErrorResolution = true;

#if defined(BP_VITA)
   EVitaTransfarringStatus lastStatus = m_status;

   m_status = kVTS_Error;

   m_errorSyncEvent.Wait();

   m_status = lastStatus;
#elif defined(BP_PS3)
   EPS3TransfarringStatus lastStatus = m_status;

   m_status = kPTS_Error;

   uint64_t ps3Result = 0;
   sys_event_flag_wait( mPS3WifiErrorSyncEvent, skPS3StartWorkFlag, SYS_EVENT_FLAG_WAIT_AND | SYS_EVENT_FLAG_WAIT_CLEAR, &ps3Result, SYS_NO_TIMEOUT );

   m_status = lastStatus;
#endif
}

void CTransfarringManager::SetMemoryCardActor( MEMORY_CARD_ACTOR* mcActor )
{
    m_LocalFileList.SetMemoryCardActor(mcActor);
}

bool CTransfarringManager::IsLocalFileListValid() const
{
   return mAreFileListsValid;
}

bool CTransfarringManager::IsAccessoryDataValid( SCloudAccessoryData const & accessoryData ) const
{
   bool const bCookieMatch = accessoryData.mCookie == BP_LINKVAR_COOKIE;
   bool const bRegionMatch = accessoryData.mRegion == gpOsContext->mBuildSKU;
   bool const bVersionMatch = accessoryData.mVersion == BP_LINKVAR_VERSION;
   bool const bSaveTypeValid = accessoryData.mSaveFile.mSaveType >= 0 && accessoryData.mSaveFile.mSaveType < kST_MaxSaveTypes;

   // Doesn't check the save data hash; need to pull the file off the server to make that comparison
   return bCookieMatch && bRegionMatch && bVersionMatch && bSaveTypeValid;
}

void CTransfarringManager::MakeAccessoryData( 
   SCloudAccessoryData * const pOut, 
   TRANSFARRING_GAME_DATA const * const pGameData,
   boost::shared_ptr<TransfarringSaveFile> const & pSaveFile) const
{
   pOut->mCookie = BP_LINKVAR_COOKIE;
   pOut->mRegion = gpOsContext->mBuildSKU;
   pOut->mVersion = BP_LINKVAR_VERSION;
   memcpy(&pOut->mSaveFile, pSaveFile.get(), sizeof( TransfarringSaveFile ) );

   // Hash the save data portion
   BP_CreateMD5Hash( pOut->mSaveDataHash, &pGameData->mSaveData, SAVE_DATA_SIZE );
}

void CTransfarringManager::GetCurrentLoadedTransfarringID( unsigned char* tid ) const
{
   memcpy( tid, mLoadedTransfarringID, sizeof(mLoadedTransfarringID) );
}

void CTransfarringManager::SetCurrentLoadedTransfarringID( const unsigned char* tid )
{
   memcpy( mLoadedTransfarringID, tid, sizeof(mLoadedTransfarringID) );
}

SCloudAccessoryData CTransfarringManager::EndianSwapAccessoryData( SCloudAccessoryData const & accessoryData ) const
{
   SCloudAccessoryData swappedData;

   BP_LE_Swap4Bytes( &swappedData.mCookie, &accessoryData.mCookie );
   BP_LE_Swap4Bytes( &swappedData.mRegion, &accessoryData.mRegion );
   BP_LE_Swap4Bytes( &swappedData.mVersion, &accessoryData.mVersion );
   swappedData.mSaveFile = EndianSwapInfoFile( accessoryData.mSaveFile );
   memcpy( swappedData.mSaveDataHash, accessoryData.mSaveDataHash, sizeof( swappedData.mSaveDataHash ) );

   return swappedData;
}

bool CTransfarringManager::CanPostWork()
{
   bool canPostWork = true;
#if defined(BP_VITA)
   if (m_status != kVTS_Idle && m_status != kVTS_AwaitingConnection && m_status != kVTS_Uninitialized)
#else
   if (m_status != kPTS_Idle && m_status != kPTS_AwaitingConnection && m_status != kPTS_Uninitialized)
#endif
   {
      canPostWork = false;
   }

   if (mGameThreadWorkInput_WorkType != kTW_Idle)
   {
      canPostWork = false;
   }

   return canPostWork;
}

STransfarringError CTransfarringManager::PostWork_WillTrophiesBeDisabled(ESaveType saveType, int index)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }
   else
   {
      mGameThreadWorkInput_WorkType = kTW_CheckTrophiesWillBeDisabled;
      mGameThreadWorkInput_SaveType = saveType;
      mGameThreadWorkInput_SaveIndex = index;

      SET_TRANSFARRING_ERROR_SUCCESS( error );
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_FlagCloudFile(ESaveType saveType, int index)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }
   else
   {
      TransfarringID curID = CTransfarringManager::Instance()->GetTransfarringID(saveType, index);
      BPE_ASSERT(curID.m_Type == kTT_Invalid, "Tried to flag a file as a cloud save when it already had a Transfarring ID");

      mGameThreadWorkInput_WorkType = kTW_FlagCloudFile;
      mGameThreadWorkInput_SaveType = saveType;
      mGameThreadWorkInput_SaveIndex = index;

      SET_TRANSFARRING_ERROR_SUCCESS( error );
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_UnflagCloudFile( unsigned char const * const transfarringID )
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }
   else
   {
      mGameThreadWorkInput_WorkType = kTW_UnflagCloudFile;
      memcpy( mGameThreadWorkInput_TransfarringID, transfarringID, sizeof(mGameThreadWorkInput_TransfarringID) );

      SET_TRANSFARRING_ERROR_SUCCESS( error );
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_SyncCloudFiles( MEMORY_CARD_ACTOR* mcActor, bool syncAllTypes, bool treatVersionMismatchesAsConflicts )
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }
   else
   {
      mGameThreadWorkInput_WorkType = kTW_SyncCloudFiles;
      mGameThreadWorkInput_SyncAllTypes = syncAllTypes;
      mGameThreadWorkInput_TreatVersionMismatchesAsConflicts = treatVersionMismatchesAsConflicts;

      // You can use another memory card actor if you're already on a screen that has one made for you, like the save/load GUI
      if (mcActor)
      {
         SetMemoryCardActor(mcActor);
         m_bUsingExternalMemoryCardActor = true;
      }

      SET_TRANSFARRING_ERROR_SUCCESS( error );
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_SyncSingleCloudFile(
   MEMORY_CARD_ACTOR* mcActor, 
   ESaveType saveType, 
   int saveIndex, 
   ECloudConflictResolution conflictResolution,
   bool treatVersionMismatchesAsConflicts)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_SUCCESS(error);

   TransfarringID tid = GetTransfarringID(saveType, saveIndex);

   if (tid.m_Type != kTT_Cloud)
   {
      SET_TRANSFARRING_ERROR(error, kTE_WrongFileType, 0);
   }
   else if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }

   if (error.mHighLevelError == kTE_Success)
   {
      mGameThreadWorkInput_WorkType = kTW_SyncSingleCloudFile;
      mGameThreadWorkInput_SaveType = saveType;
      mGameThreadWorkInput_SaveIndex = saveIndex;
      mGameThreadWorkInput_ConflictResolution = conflictResolution;
      mGameThreadWorkInput_TreatVersionMismatchesAsConflicts = treatVersionMismatchesAsConflicts;

      // You can use another memory card actor if you're already on a screen that has one made for you, like the save/load GUI
      if (mcActor)
      {
         SetMemoryCardActor(mcActor);
         m_bUsingExternalMemoryCardActor = true;
      }
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_UnlockSaveFile(ESaveType saveType, int saveIndex)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_SUCCESS(error);

   TransfarringID tid = GetTransfarringID(saveType, saveIndex);

   if (tid.m_Type != kTT_WiFi)
   {
      SET_TRANSFARRING_ERROR(error, kTE_WrongFileType, 0);
   }
   else if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }

   if (error.mHighLevelError == kTE_Success)
   {
      mGameThreadWorkInput_WorkType = kTW_UnlockSaveFile;
      mGameThreadWorkInput_SaveType = saveType;
      mGameThreadWorkInput_SaveIndex = saveIndex;
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_CheckSyncNeeded( ESaveType saveType, bool treatVersionMismatchesAsConflicts )
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }
   else
   {
      mGameThreadWorkInput_WorkType = kTW_CheckSyncNeeded;
      mGameThreadWorkInput_SaveType = saveType;
      mGameThreadWorkInput_TreatVersionMismatchesAsConflicts = treatVersionMismatchesAsConflicts;

      SET_TRANSFARRING_ERROR_SUCCESS( error );
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_DeleteSaveFile(MEMORY_CARD_ACTOR* mcActor, ESaveType saveType, int saveIndex)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_SUCCESS(error);

   if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }

   if (error.mHighLevelError == kTE_Success)
   {
      mGameThreadWorkInput_WorkType = kTW_DeleteSaveFile;
      mGameThreadWorkInput_SaveType = saveType;
      mGameThreadWorkInput_SaveIndex = saveIndex;

      // You can use another memory card actor if you're already on a screen that has one made for you, like the save/load GUI
      if (mcActor)
      {
         SetMemoryCardActor(mcActor);
         m_bUsingExternalMemoryCardActor = true;
      }
   }

   return error;
}

STransfarringError CTransfarringManager::PostWork_DeleteNonGameFile(MEMORY_CARD_ACTOR* mcActor, ENonGameSaveType saveType, int saveIndex)
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_SUCCESS(error);

   if (!CanPostWork())
   {
      SET_TRANSFARRING_ERROR(error, kTE_WorkInProgress, 0);
   }

   if (error.mHighLevelError == kTE_Success)
   {
      mGameThreadWorkInput_WorkType = kTW_DeletePhotoFile;
      mGameThreadWorkInput_NonGameSaveType = saveType;
      mGameThreadWorkInput_SaveIndex = saveIndex;

      // You can use another memory card actor if you're already on a screen that has one made for you, like the save/load GUI
      if (mcActor)
      {
         SetMemoryCardActor(mcActor);
         m_bUsingExternalMemoryCardActor = true;
      }
   }

   return error;
}

STransfarringError CTransfarringManager::ProcessWork()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR_INVALID(retval);

   switch (mGameThreadWorkInput_WorkType)
   {
   case kTW_Idle:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   case kTW_FlagCloudFile:
      retval = ProcessFlagCloudFile();
      break;
   case kTW_UnflagCloudFile:
      retval = ProcessUnflagCloudFile();
      break;
   case kTW_SyncCloudFiles:
      retval = ProcessSyncCloudFiles();
      break;
   case kTW_CheckSyncNeeded:
      retval = ProcessCheckSyncNeeded();
      break;
   case kTW_DeleteSaveFile:
      retval = ProcessDeleteSaveFile();
      break;
   case kTW_UnlockSaveFile:
      retval = ProcessUnlockSaveFile();
      break;
   case kTW_SyncSingleCloudFile:
      retval = ProcessSyncSingleCloudFile();
      break;
   case kTW_DeletePhotoFile:
      retval = ProcessDeleteNonGameFile();
      break;
   case kTW_CheckTrophiesWillBeDisabled:
      retval = ProcessCheckTrophiesWillBeDisabled();
      break;
   default:
      BPE_ASSERT(false, "TransfarringManager::ProcessWork did not correctly handle mWorkInput_WorkType");
      break;
   }

   if (retval.mHighLevelError != kTE_Processing)
   {
      mGameThreadWorkInput_WorkType = kTW_Idle;
      if (m_bUsingExternalMemoryCardActor)
      {
         SetMemoryCardActor(NULL);
         m_bUsingExternalMemoryCardActor = false;
      }
   }

   return retval;
}

STransfarringError CTransfarringManager::ProcessCheckTrophiesWillBeDisabled()
{
   static TRANSFARRING_GAME_DATA sGameData;

   STransfarringError retval;
   SET_TRANSFARRING_ERROR_INVALID( retval );

   retval = m_LocalFileList.LoadFile_GameThread( sGameData, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex );
   if( retval.mHighLevelError == kTE_Success )
   {
      //If the PSN account hash is 0, then trophies will be disabled upon transfarring.
      bool const willTrophiesBeDisabled = sGameData.mBPLinkvars.mTrophyValidFlag == BPLinkvarDefines::kTrophiesValid
         && BP_IsMD5HashZero( sGameData.mBPLinkvars.mOriginalPSNAccount );
      
      //HACK - Return whether trophies will be disabled in the low level error field.
      SET_TRANSFARRING_ERROR( retval, kTE_Success, willTrophiesBeDisabled);
   }
   return retval;
}

STransfarringError CTransfarringManager::ProcessFlagCloudFile()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   enum
   {
      kCF_StepBegin,
      kCF_StepProcessGetInfo,
      kCF_StepLoad, // Load the game
      kCF_StepFlag, // Set the transfarring ID
      kCF_StepWaitFlag,
      kCF_StepSave, // Re-save the game
      kCF_StepEnd
   };

   class TransfarringCloudFlagWork
   {
   public:
      TransfarringCloudFlagWork()
      {
         mWorkStep = kCF_StepBegin;
         memset(mCloudData, 0, sizeof(mCloudData));
         mCloudSlot = -1;
         mHandle = 0;
         mValidSlots = 0;
      }

      int mWorkStep;
      uint8 mCloudData[kTUSSlotDataSize];
      int mCloudSlot;
      uint64 mHandle;
      uint8 mValidSlots;
      TRANSFARRING_GAME_DATA mCloudGameData;
   };

   static TransfarringCloudFlagWork* sFlagWork = NULL;

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID(tusError);

   // Init work
   if (!sFlagWork)
   {
      sFlagWork = new TransfarringCloudFlagWork();
   }

   switch (sFlagWork->mWorkStep)
   {
   case kCF_StepBegin:
      // Find the first available slot
      sFlagWork->mHandle = BP_TUS_Post_GetSlotInfos();
      sFlagWork->mWorkStep = kCF_StepProcessGetInfo;
      StartProgress( 2 );
      break;
   case kCF_StepProcessGetInfo:
      STUSSlotInfo slotInfo[ 8 ];
      if (BP_TUS_Consume_GetSlotInfos(sFlagWork->mHandle, &tusError, slotInfo, &sFlagWork->mValidSlots))
      {
         if (tusError.mHighLevelError == kTE_Success)
         {
            uint8 mask = 1;
            sFlagWork->mCloudSlot = -1;
            uint8 tempSlots = sFlagWork->mValidSlots;
            for (int i=0; i < 8; i++)
            {
               if ((tempSlots & mask) == 0)
               {
                  sFlagWork->mCloudSlot = i + 1;
                  break;
               }
               tempSlots >>= 1;
            }

            if (sFlagWork->mCloudSlot <= 0 || sFlagWork->mCloudSlot > MAX_NUM_CLOUD_FILES)
            {
               SET_TRANSFARRING_ERROR(retval, kTE_NoCloudSlotsAvailable, 0);
            }
            else
            {
               sFlagWork->mWorkStep = kCF_StepLoad;
            }
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCF_StepLoad:
      tusError = m_LocalFileList.LoadFile_GameThread(sFlagWork->mCloudGameData, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            sFlagWork->mHandle = BP_TUS_Post_IncrementSlotVariable(1, 1ULL);
            sFlagWork->mWorkStep = kCF_StepFlag;
         }
         else
         {
            retval = tusError;
         }
      }
      break;
   case kCF_StepFlag:
      // Create a new cloud transfarring ID
      uint64 oldVariable;
      uint64 newVariable;
      if (BP_TUS_Consume_IncrementSlotVariable(sFlagWork->mHandle, &tusError, &oldVariable, &newVariable))
      {
         if (tusError.mHighLevelError == kTE_Success)
         {
            TransfarringID newID = GetTransfarringID( mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex );
            newID.MakeNewCloud( newVariable );
            SetTransfarringID(newID, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);

            // If there's no originator data, write originator data and disable trophies.
            bool newOriginatorData = TryWriteOriginatorData( 
               &sFlagWork->mCloudGameData.mBPLinkvars, 
               mGameThreadWorkInput_SaveType, 
               mGameThreadWorkInput_SaveIndex,
               false );

            if( newOriginatorData || BP_IsMD5HashZero( sFlagWork->mCloudGameData.mBPLinkvars.mOriginalPSNAccount ) )
            {
               sFlagWork->mCloudGameData.mBPLinkvars.mTrophyValidFlag = BPLinkvarDefines::kTrophiesInvalid;
            }

            // Endian swap the game data and copy it into the cloud data
            EndianSwapGameData( 
               reinterpret_cast<TRANSFARRING_GAME_DATA*>( sFlagWork->mCloudData ), 
               &sFlagWork->mCloudGameData, 
               mGameThreadWorkInput_SaveType );

            TransfarringFileList::TTransfarringSaveFilePtr saveFile = m_LocalFileList.GetSaveFilePtrPlatform(
               mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);

            BPE_ASSERT(saveFile, "TransfarringManager::ProcessFlagCloudFile did not find a save file");

            // Create a new save file for the cloud file list
            TransfarringFileList::TTransfarringSaveFilePtr newFile( new TransfarringSaveFile() );
            *newFile = *saveFile;

            // Add to the cloud file list
            m_CloudFileList.AddCloudFilePtr(newFile, sFlagWork->mCloudSlot);

            // Make new accessory data
            SCloudAccessoryData accessoryData;
            MakeAccessoryData( &accessoryData, reinterpret_cast<TRANSFARRING_GAME_DATA*>( sFlagWork->mCloudData ), newFile );

            STUSSlotInfo newSlotInfo;
            accessoryData = EndianSwapAccessoryData( accessoryData );
            memcpy(newSlotInfo.mAccessoryData, &accessoryData, sizeof( accessoryData ) );

            // Send the cloud data buffer and accessory data to the server
            sFlagWork->mHandle = BP_TUS_Post_SetSlotData(sFlagWork->mCloudSlot, sFlagWork->mCloudData, kTUSSlotDataSize, &newSlotInfo);

            sFlagWork->mWorkStep = kCF_StepWaitFlag;
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCF_StepWaitFlag:
      if (BP_TUS_Consume_SetSlotData(sFlagWork->mHandle, &tusError))
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            sFlagWork->mWorkStep = kCF_StepSave;
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCF_StepSave:
      tusError = m_LocalFileList.SaveFile_GameThread(sFlagWork->mCloudGameData, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            sFlagWork->mWorkStep = kCF_StepEnd;
         }
         else
         {
            retval = tusError;
         }
      }
      break;
   case kCF_StepEnd:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   default:
      BPE_ASSERT(sFlagWork->mWorkStep < kCF_StepEnd, "ProcessWork step is out of bounds");
      sFlagWork->mWorkStep++;
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      delete sFlagWork;
      sFlagWork = NULL;
      ClearProgress();
   }

   return retval;
}

STransfarringError CTransfarringManager::ProcessUnflagCloudFile()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   enum
   {
      kCU_StepBegin,
      kCU_StepProcessGetInfo, // Get the info from the server and find the slot
      kCU_StepDeleteFromServer, // Delete the cloud file from the server
      kCU_DetermineLocalFile,
      kCU_StepDeleteTransfarringID,
      kCU_StepLoad,
      kCU_StepSave, // Save the file with its new invalid transfarring ID
      kCU_StepEnd
   };

   class TransfarringCloudUnflagWork
   {
   public:
      TransfarringCloudUnflagWork()
      {
         mWorkStep = kCU_StepBegin;
         mCloudSlot = -1;
         mHandle = 0;
         mLocalSaveType = kST_MaxSaveTypes;
         mLocalSaveIndex = -1;
      }

      int mWorkStep;
      int mCloudSlot;
      uint64 mHandle;
      TRANSFARRING_GAME_DATA mGameData;
      ESaveType mLocalSaveType;
      int mLocalSaveIndex;
   };

   static TransfarringCloudUnflagWork* sUnflagWork = NULL;

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID(tusError);

   uint8 validSlots = 0;

   // Init work
   if (!sUnflagWork)
   {
      sUnflagWork = new TransfarringCloudUnflagWork();

      // If we're not signed in, just unflag it locally
      if( !BP_Network_IsSignedIn() )
      {
         sUnflagWork->mWorkStep = kCU_DetermineLocalFile;
      }
   }

   switch (sUnflagWork->mWorkStep)
   {
   case kCU_StepBegin:
      sUnflagWork->mHandle = BP_TUS_Post_GetSlotInfos();
      sUnflagWork->mWorkStep = kCU_StepProcessGetInfo;
      StartProgress( 2 );
      break;
   case kCU_StepProcessGetInfo:
      {
      STUSSlotInfo slotInfo[ 8 ];
      memset(slotInfo, 0, sizeof(slotInfo));
      uint8 validSlots = 0;
      if (BP_TUS_Consume_GetSlotInfos(sUnflagWork->mHandle, &tusError, slotInfo, &validSlots))
      {
         if (tusError.mHighLevelError == kTE_Success)
         {
            // Search the cloud file list for the file that has a matching transfarring ID
            sUnflagWork->mCloudSlot = m_CloudFileList.GetCloudSaveIndexFromUniqueID( mGameThreadWorkInput_TransfarringID );

            if( TransfarringCloudFileList::IsCloudSaveIndexValid( sUnflagWork->mCloudSlot ) )
            {
               sUnflagWork->mHandle = BP_TUS_Post_DeleteSlotData(sUnflagWork->mCloudSlot);
               sUnflagWork->mWorkStep = kCU_StepDeleteFromServer;
            }
            else
            {
               sUnflagWork->mWorkStep = kCU_DetermineLocalFile;
            }
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      }
      break;
   case kCU_StepDeleteFromServer:
      if (BP_TUS_Consume_DeleteSlotData(sUnflagWork->mHandle, &tusError))
      {
         if (tusError.mHighLevelError == kTE_Success)
         {
            m_CloudFileList.RemoveCloudFile(sUnflagWork->mCloudSlot);

            sUnflagWork->mWorkStep = kCU_DetermineLocalFile;
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCU_DetermineLocalFile:
      {
      // Search the local TransfarringID list for the file with the matching transfarring ID
      GetSaveFileFromUniqueID( 
         mGameThreadWorkInput_TransfarringID, 
         sUnflagWork->mLocalSaveType, 
         sUnflagWork->mLocalSaveIndex );

      if( sUnflagWork->mLocalSaveType != kST_MaxSaveTypes && sUnflagWork->mLocalSaveIndex > -1 )
      {
         // If the matching file isn't found on the server, just load the file, change its transfarring ID, and delete it
         sUnflagWork->mWorkStep = kCU_StepLoad;
      }
      else
      {
         sUnflagWork->mWorkStep = kCU_StepEnd;
      }

      }
      break;
   case kCU_StepLoad:
      tusError = m_LocalFileList.LoadFile_GameThread(sUnflagWork->mGameData, sUnflagWork->mLocalSaveType, sUnflagWork->mLocalSaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success)
         {
            sUnflagWork->mWorkStep = kCU_StepDeleteTransfarringID;
         }
         else
         {
            retval = tusError;
         }
      }
      break;
   case kCU_StepDeleteTransfarringID:
      {
      TransfarringID newTID = GetTransfarringID( sUnflagWork->mLocalSaveType, sUnflagWork->mLocalSaveIndex );
      
      if( CheckTrophyCompatibility(&sUnflagWork->mGameData.mBPLinkvars, (unsigned char*)newTID.m_UniqueID) )
      {
         //Update originator data if the transfarring id changes and trophies were enabled. MGSTWO-3363, MGSTWO-3178.
         newTID.Invalidate();
         newTID.GenerateUniqueID();

         SetTransfarringID(newTID, sUnflagWork->mLocalSaveType, sUnflagWork->mLocalSaveIndex);

         //Force update of originator data if compatible.
         OverwriteOriginatorData( &sUnflagWork->mGameData.mBPLinkvars, sUnflagWork->mLocalSaveType, sUnflagWork->mLocalSaveIndex );
      }
      else
      {
         newTID.Invalidate();
         newTID.GenerateUniqueID();

         SetTransfarringID(newTID, sUnflagWork->mLocalSaveType, sUnflagWork->mLocalSaveIndex);
      }

      sUnflagWork->mWorkStep = kCU_StepSave;
      }
      break;
   case kCU_StepSave:
      tusError = m_LocalFileList.SaveFile_GameThread(sUnflagWork->mGameData, sUnflagWork->mLocalSaveType, sUnflagWork->mLocalSaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            sUnflagWork->mWorkStep = kCU_StepEnd;
         }
         else
         {
            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }
      }
      break;
   case kCU_StepEnd:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   default:
      BPE_ASSERT(sUnflagWork->mWorkStep < kCU_StepEnd, "ProcessWork step is out of bounds");
      sUnflagWork->mWorkStep++;
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      delete sUnflagWork;
      sUnflagWork = NULL;
      ClearProgress();
   }

   return retval;
}

class TransfarringCloudSaveFile
{
public:
   TransfarringCloudSaveFile( int cloudSlot, TransfarringID tid, ESaveType saveType, int saveIndex, const TransfarringSaveFile* saveFile )
   {
      mCloudSlot = cloudSlot;
      mID = tid;
      mSaveType = saveType;
      mSaveIndex = saveIndex;
      memcpy(&mSaveFile, saveFile, sizeof( TransfarringSaveFile ) );
   }

   int mCloudSlot;
   TransfarringID mID;
   ESaveType mSaveType;
   int mSaveIndex;
   TransfarringSaveFile mSaveFile; // From accessory data
};

STransfarringError CTransfarringManager::ProcessSyncCloudFiles()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   enum
   {
      kCS_StepLoadBegin,
      kCS_StepBegin,
      kCS_StepProcessGetInfo,
      kCS_StepChooseSyncAction,
      kCS_StepSyncFromServer,
      kCS_StepSave,
      kCS_StepSyncToServer,
      kCS_StepLoad,
      kCS_StepReSave, // Re-save the file to save the new TransfarringID
      kCS_StepResolveConflict,
      kCS_StepConsumeIncrementSlotVariable,
      kCS_StepEnd
   };

   class TransfarringCloudSyncWork
   {
   public:
      TransfarringCloudSyncWork(int workStep)
      {
         mWorkStep = workStep;
         mHandle = 0;
         mValidSlots = 0;
         mRanOutOfSpace = false;
      }

      int mWorkStep;
      uint8 mCloudData[kTUSSlotDataSize];
      uint64 mHandle;
      uint8 mValidSlots;
      std::queue<TransfarringCloudSaveFile > mSyncQueue;
      TRANSFARRING_GAME_DATA mGameData;
      bool mRanOutOfSpace;
   };

   static TransfarringCloudSyncWork* syncWork = NULL;

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID(tusError);

   TransfarringCloudSaveFile* cloudSave = NULL;
   if (syncWork && syncWork->mSyncQueue.size())
   {
      cloudSave = &syncWork->mSyncQueue.front();
      gSyncSlotsRemaining = syncWork->mSyncQueue.size();
   }
   else
   {
      gSyncSlotsRemaining = 0;
   }

   if (!syncWork)
   {
      if (m_bUsingExternalMemoryCardActor)
      {
         syncWork = new TransfarringCloudSyncWork(kCS_StepLoadBegin);
      }
      else
      {
         syncWork = new TransfarringCloudSyncWork(kCS_StepBegin);
      }
   }

   // TODO: If you want to disable all syncing, don't even let this flow start
   // Also do this in ProcessSyncSingleCloudFile
   //if( globalValueThatSaysToDisableSyncing )
   //   syncWork->mWorkStep = kCS_StepEnd;
   //   SET_TRANSFARRING_ERROR_SUCCESS( retval.mHighLevelError );

   switch (syncWork->mWorkStep)
   {
   case kCS_StepLoadBegin:
      tusError = m_LocalFileList.LoadAllFileLists_GameThread();
      if( tusError.mHighLevelError != kTE_Processing)
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            syncWork->mWorkStep = kCS_StepBegin;
         }
         else
         {
            retval = tusError;
         }
      }
      break;
   case kCS_StepBegin:
      syncWork->mHandle = BP_TUS_Post_GetSlotInfos();
      syncWork->mWorkStep = kCS_StepProcessGetInfo;
      break;
   case kCS_StepProcessGetInfo:
      STUSSlotInfo slotInfo[ 8 ];
      memset(slotInfo, 0, sizeof(slotInfo));
      if (BP_TUS_Consume_GetSlotInfos(syncWork->mHandle, &tusError, slotInfo, &syncWork->mValidSlots))
      {   
         if (tusError.mHighLevelError == kTE_Success)
         {
            uint8 tempSlots = syncWork->mValidSlots;
            uint8 mask = 1;

            // Find the local save file that this corresponds to
            bpe::reserved_vector<std::vector<TransfarringSaveFile>, kST_MaxSaveTypes> saveTypeToFiles;
            saveTypeToFiles.resize( kST_MaxSaveTypes, std::vector<TransfarringSaveFile>() );

            for (int i=0; i < 8; i++)
            {
               if ((tempSlots & mask) != 0)
               {
                  int cloudSlot = i + 1;

                  SCloudAccessoryData* pAccessoryData = reinterpret_cast<SCloudAccessoryData*>( &slotInfo[i] );
                  *pAccessoryData = EndianSwapAccessoryData( *pAccessoryData );

                  ESaveType saveType;
                  int saveIndex;
                  GetSaveFileFromUniqueID(reinterpret_cast<unsigned char*>(pAccessoryData->mSaveFile.transfarringID), saveType, saveIndex);

                  // If the file is on the cloud but doesn't exist locally, create a new save file
                  if( saveIndex < 0 || saveType == kST_MaxSaveTypes )
                  {
                     // Keep a list of files to add later, so we can tell if we have enough space
                     // Only sync valid files
                     if( IsAccessoryDataValid( *pAccessoryData ) )
                     {
                        TransfarringSaveFile curFile;
                        memcpy(&curFile, &pAccessoryData->mSaveFile, sizeof(TransfarringSaveFile));
                        curFile.mSaveIndex = cloudSlot; // Save index is throwaway data when coming off the cloud, use it to store the cloud slot

                        // Only sync files of requested type
                        if( mGameThreadWorkInput_SyncAllTypes || (ESaveType)curFile.mSaveType == m_LocalFileList.GetMemoryCardActorType() )
                        {
                           saveTypeToFiles[curFile.mSaveType].push_back( curFile );
                        }
                     }
                  }
                  else if( IsAccessoryDataValid( *pAccessoryData ) )
                  {
                     TransfarringID tid = GetTransfarringID(saveType, saveIndex);
                     BPE_ASSERT(tid.m_Type == kTT_Cloud, "CTransfarringManager associated a non-cloud file with a cloud save slot");

                     // Only sync files of requested type
                     if( mGameThreadWorkInput_SyncAllTypes || saveType == m_LocalFileList.GetMemoryCardActorType() )
                     {
                        syncWork->mSyncQueue.push( TransfarringCloudSaveFile( cloudSlot, tid, saveType, saveIndex, &pAccessoryData->mSaveFile ) );
                     }
                  }
               }
               tempSlots >>= 1;
            }

            // Check to see if we have enough space
            bpe::reserved_vector<int, kST_MaxSaveTypes> saveTypeToNumAvailable;
            saveTypeToNumAvailable.resize( kST_MaxSaveTypes, 0 );
            for( int saveType = kST_Game; saveType < kST_MaxSaveTypes; ++saveType )
            {
               int numFiles = saveTypeToFiles[saveType].size();
               saveTypeToNumAvailable[saveType] = m_LocalFileList.GetNumAvailableSaveSlots( (ESaveType)saveType );
               if( numFiles > saveTypeToNumAvailable[saveType] )
               {
                  syncWork->mRanOutOfSpace = true;
               }
            }

            for( int saveType = kST_Game; saveType < kST_MaxSaveTypes; ++saveType )
            {
               TransfarringFileList::TAvailableSaveIterator saveIter = m_LocalFileList.GetSaveSlotIterator( (ESaveType)saveType );

               for (int i=0; i < saveTypeToFiles[saveType].size(); i++)
               {
                  if( i + 1 > saveTypeToNumAvailable[saveType] )
                  {
                     break;
                  }

                  TransfarringFileList::TTransfarringSaveFilePtr curFile( new TransfarringSaveFile );
                  TransfarringSaveFile cloudFile = saveTypeToFiles[saveType][i];
                  *curFile = cloudFile;

                  int cloudSlot = curFile->mSaveIndex; // Stored in the earlier loop

                  ESaveType saveType = (ESaveType)curFile->mSaveType;
                  int saveIndex = m_LocalFileList.FindNextAvailableSaveSlot( saveType, &saveIter );
                  curFile->mSaveIndex = saveIndex;

                  BPE_ASSERT(saveIndex != -1, "TransfarringManager::ProcessSyncCloudFiles - Tried to sync when file list was full");

                  TransfarringID emptyTID;
                  emptyTID.MakeNewInvalid();

                  syncWork->mSyncQueue.push(TransfarringCloudSaveFile(cloudSlot, emptyTID, saveType, saveIndex, &cloudFile));
               }
            }

            // If there's nothing in the sync queue, there's nothing to process
            if (syncWork->mSyncQueue.empty())
            {
               syncWork->mWorkStep = kCS_StepEnd;
            }
            else
            {
               syncWork->mWorkStep = kCS_StepChooseSyncAction;
            }
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
      // The following steps loop until the syncWork->mSyncQueue queue is empty, in which case it goes to kCS_StepEnd
   case kCS_StepChooseSyncAction:
      if (cloudSave)
      {
         if( cloudSave->mID.m_Type == kTT_Invalid )
         {
            // New save
            syncWork->mWorkStep = kCS_StepSyncFromServer;
            syncWork->mHandle = BP_TUS_Post_GetSlotData( cloudSave->mCloudSlot );
         }
         else if (cloudSave->mID.DoVersionIDsMatch((unsigned char*)cloudSave->mSaveFile.cloudVersionHash))
         {
            if (cloudSave->mID.HasOfflineWork())
            {
               // Version match and offline work: Load, then push to server
               syncWork->mWorkStep = kCS_StepLoad;
            }
            else
            {
               // Version match and no offline work: No action
               syncWork->mSyncQueue.pop();
            }
         }
         else
         {
            // Version mismatch and offline work: Resolve conflict
            if ( mGameThreadWorkInput_TreatVersionMismatchesAsConflicts || cloudSave->mID.HasOfflineWork() )
            {
               // Flag as needing conflict resolution
               m_LocalFileList.SetConflict(true, cloudSave->mSaveType, cloudSave->mSaveIndex);
               syncWork->mSyncQueue.pop();
            }
            else
            {
               // Version mismatch and no offline work: Sync from server, then save
               syncWork->mWorkStep = kCS_StepSyncFromServer;
               syncWork->mHandle = BP_TUS_Post_GetSlotData(cloudSave->mCloudSlot);
            }
         }
      }
      else
      {
         syncWork->mWorkStep = kCS_StepEnd;
      }
      break;
   case kCS_StepLoad:
      tusError = m_LocalFileList.LoadFile_GameThread(syncWork->mGameData, cloudSave->mSaveType, cloudSave->mSaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            // Endian swap the game data and copy it into the larger cloud buffer
            EndianSwapGameData( 
               reinterpret_cast<TRANSFARRING_GAME_DATA*>( syncWork->mCloudData ),
               &syncWork->mGameData,
               cloudSave->mSaveType );

            // Increment the variable on the server, then re-save the file
            syncWork->mHandle = BP_TUS_Post_IncrementSlotVariable(1, 1ULL);
            syncWork->mWorkStep = kCS_StepConsumeIncrementSlotVariable;
         }
         else
         {
            retval = tusError;
         }
      }
      break;
   case kCS_StepConsumeIncrementSlotVariable:
      uint64 newVersionValue;
      if (BP_TUS_Consume_IncrementSlotVariable(syncWork->mHandle, &tusError, NULL, &newVersionValue))
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            TransfarringID tid = GetTransfarringID(cloudSave->mSaveType, cloudSave->mSaveIndex);
            BPE_ASSERT(tid.m_Type == kTT_Cloud, "ProcessWork::kCS_StepConsumeIncrementSlotVariable was given a bad transfarring ID");
            uint64 swappedVersionValue = newVersionValue;
            BP_LE_Swap8Bytes_Inp(&swappedVersionValue);
            TransfarringID::CreateHash((unsigned char*)tid.m_VersionID, (unsigned char*)&swappedVersionValue, sizeof(swappedVersionValue));
            tid.m_HasOfflineWork = false; // By pushing this to the server, we no longer have offline data
            SetTransfarringID(tid, cloudSave->mSaveType, cloudSave->mSaveIndex);

            syncWork->mWorkStep = kCS_StepReSave;
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCS_StepReSave:
      tusError = m_LocalFileList.SaveFile_GameThread(syncWork->mGameData, cloudSave->mSaveType, cloudSave->mSaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            // Make new accessory data
            TransfarringFileList::TTransfarringSaveFilePtr curFile = m_LocalFileList.GetSaveFilePtrPlatform(cloudSave->mSaveType, cloudSave->mSaveIndex);
            BPE_ASSERT(curFile, "TransfarringManager::ProcessSyncCloudFiles::kCS_StepReSave was given a bad save file");

            SCloudAccessoryData accessoryData;
            MakeAccessoryData( &accessoryData, reinterpret_cast<TRANSFARRING_GAME_DATA*>( syncWork->mCloudData ), curFile );
            accessoryData = EndianSwapAccessoryData( accessoryData );

            STUSSlotInfo newSlotInfo;
            memset( &newSlotInfo, 0, sizeof( newSlotInfo ) );
            memcpy( newSlotInfo.mAccessoryData, &accessoryData, sizeof( SCloudAccessoryData ) );

            // Update the remote cloud file if we're using that list
            TransfarringFileList::TTransfarringSaveFilePtr cloudFile = m_CloudFileList.GetCloudFilePtr_CloudSlot( cloudSave->mCloudSlot );
            if( cloudFile.get() )
            {
               *cloudFile = *curFile;
               cloudFile->mSaveIndex = cloudSave->mCloudSlot;
            }

            syncWork->mHandle = BP_TUS_Post_SetSlotData( cloudSave->mCloudSlot, syncWork->mCloudData, sizeof(syncWork->mCloudData), &newSlotInfo );

            syncWork->mWorkStep = kCS_StepSyncToServer;
         }
         else
         {
            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }
      }
      break;
   case kCS_StepSyncToServer:
      if (BP_TUS_Consume_SetSlotData(syncWork->mHandle, &tusError))
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            syncWork->mSyncQueue.pop();
            syncWork->mWorkStep = kCS_StepChooseSyncAction; // continue the loop
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCS_StepSyncFromServer:
      STUSSlotInfo newSlotInfo;
      if (BP_TUS_Consume_GetSlotData(syncWork->mHandle, &tusError, syncWork->mCloudData, &newSlotInfo))
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            // Check the hash before endian swapping
            unsigned char cloudDataHash[TransfarringDefines::kHashLengthBytes]; // Save data hash
            TRANSFARRING_GAME_DATA* pCloudGameData = reinterpret_cast<TRANSFARRING_GAME_DATA*>(syncWork->mCloudData);
            BP_CreateMD5Hash( cloudDataHash, &pCloudGameData->mSaveData, SAVE_DATA_SIZE );

            SCloudAccessoryData* pAccessoryData = reinterpret_cast<SCloudAccessoryData*>( newSlotInfo.mAccessoryData );

            if( !memcmp(pAccessoryData->mSaveDataHash, cloudDataHash, sizeof(cloudDataHash) ) )
            {
               memcpy( &syncWork->mGameData, syncWork->mCloudData, sizeof( TRANSFARRING_GAME_DATA ) );
               EndianSwapGameData( &syncWork->mGameData, &syncWork->mGameData, cloudSave->mSaveType );

               HandleIncomingSaveFile( &syncWork->mGameData, false, (unsigned char*)cloudSave->mSaveFile.transfarringID );

               // Before saving, update the Transfarring ID to match the version number
               // If we're syncing to an empty local save slot, set the Transfarring ID now
               if( cloudSave->mID.m_Type != kTT_Cloud )
               {
                  TransfarringID newTID;
                  newTID.m_Type = kTT_Cloud;
                  newTID.m_HasOfflineWork = false;
                  memcpy(newTID.m_UniqueID, cloudSave->mSaveFile.transfarringID, TransfarringDefines::kHashLengthBytes);
                  memcpy(newTID.m_VersionID, cloudSave->mSaveFile.cloudVersionHash, TransfarringDefines::kHashLengthBytes);
                  memcpy(newTID.m_PSNAccountName, cloudSave->mSaveFile.psnAccountHash, TransfarringDefines::kHashLengthBytes);
                  SetTransfarringID(newTID, cloudSave->mSaveType, cloudSave->mSaveIndex );
               }
               else
               {
                  TransfarringID tid = GetTransfarringID( cloudSave->mSaveType, cloudSave->mSaveIndex );
                  memcpy( tid.m_VersionID, cloudSave->mSaveFile.cloudVersionHash, TransfarringDefines::kHashLengthBytes );
                  tid.m_HasOfflineWork = false; // There's no case where you will have offline work after syncing from the server
                  SetTransfarringID( tid, cloudSave->mSaveType, cloudSave->mSaveIndex );
               }

               syncWork->mWorkStep = kCS_StepSave;
            }
            else
            {
               m_CloudFileList.SetCloudFileCorrupt( cloudSave->mCloudSlot, true );

               syncWork->mWorkStep = kCS_StepChooseSyncAction; // continue the loop
            }
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCS_StepSave:
      tusError = m_LocalFileList.SaveFile_GameThread(syncWork->mGameData, cloudSave->mSaveType, cloudSave->mSaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            syncWork->mSyncQueue.pop();
            syncWork->mWorkStep = kCS_StepChooseSyncAction; // continue the loop
         }
         else
         {
            // If we're syncing to an empty save slot, remove the transfarring ID we set earlier, since the save didn't succeed
            if( cloudSave->mID.m_Type != kTT_Cloud )
            {
               SetTransfarringID( cloudSave->mID, cloudSave->mSaveType, cloudSave->mSaveIndex );
            }

            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }
      }
      break;
   case kCS_StepEnd:
      if( syncWork->mRanOutOfSpace )
      {
         SET_TRANSFARRING_ERROR(retval, kTE_NotEnoughLocalSaveSlots, 0);
      }
      else
      {
         SET_TRANSFARRING_ERROR_SUCCESS(retval);
      }
      break;
   default:
      BPE_ASSERT(syncWork->mWorkStep < kCS_StepEnd, "ProcessWork step is out of bounds");
      syncWork->mWorkStep++;
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      delete syncWork;
      syncWork = NULL;
      ClearProgress();
   }

   return retval;
}

namespace TransfarringSyncSingle
{
   enum ESyncSingleSteps
   {
      kCSSF_StepBegin,
      kCSSF_StepProcessGetInfo,
      kCSSF_StepResolveConflict,
      kCSSF_StepSyncFromServer,
      kCSSF_StepSave,
      kCSSF_StepSyncToServer,
      kCSSF_StepLoad,
      kCSSF_StepConsumeIncrementSlotVariable,
      kCSSF_StepReSave, // Re-save the file to save the new TransfarringID
      kCSSF_StepEnd
   };

   class TransfarringSyncSingleWork
   {
   public:

      TransfarringSyncSingleWork( ESyncSingleSteps beginStep )
      : mWorkStep( beginStep )
      , mHandle( 0 )
      , mValidSlots ( 0 )
      , mTID()
      , mCloudSlot( 0 )
      {
         memset( mSlotInfo, 0, sizeof(mSlotInfo) );
         memset( mCloudData, 0, sizeof(kTUSSlotDataSize) );
         memset( &mSaveFile, 0, sizeof(TransfarringSaveFile) );
      }

      ESyncSingleSteps mWorkStep;
      uint64 mHandle;
      STUSSlotInfo mSlotInfo[8];
      uint8 mValidSlots;
      uint8 mCloudData[kTUSSlotDataSize];
      TransfarringID mTID;
      TransfarringSaveFile mSaveFile;
      int mCloudSlot;
   };
}

// This, along with all other process functions, will be refactored to add a context rather than making the function itself keep static data
STransfarringError CTransfarringManager::ProcessSyncSingleCloudFile()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID(tusError);

   // Init work
   static boost::scoped_ptr<TransfarringSyncSingle::TransfarringSyncSingleWork> sSyncSingleWork( NULL );

   if ( !sSyncSingleWork.get() )
   {
      sSyncSingleWork.reset( new TransfarringSyncSingle::TransfarringSyncSingleWork( TransfarringSyncSingle::kCSSF_StepBegin ) );
   }

   switch ( sSyncSingleWork->mWorkStep )
   {
   case TransfarringSyncSingle::kCSSF_StepBegin:
      sSyncSingleWork->mHandle = BP_TUS_Post_GetSlotInfos();
      sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepProcessGetInfo;
      break;
   case TransfarringSyncSingle::kCSSF_StepProcessGetInfo:
      if ( BP_TUS_Consume_GetSlotInfos( sSyncSingleWork->mHandle, &tusError, sSyncSingleWork->mSlotInfo, &sSyncSingleWork->mValidSlots ) )
      {
         if (tusError.mHighLevelError == kTE_Success)
         {
            uint8 tempSlots = sSyncSingleWork->mValidSlots;
            uint8 mask = 1;

            sSyncSingleWork->mTID = GetTransfarringID( mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex );

            BPE_ASSERT( sSyncSingleWork->mTID.m_Type == kTT_Cloud, "SyncSingleCloudFile tried to sync a non-cloud file" );

            sSyncSingleWork->mCloudSlot = 0;

            // Search for the cloud file that this corresponds to
            for( int i=0; i < 8; i++ )
            {
               if( (tempSlots & mask) != 0 )
               {
                  SCloudAccessoryData* accessoryData = reinterpret_cast<SCloudAccessoryData*>( &sSyncSingleWork->mSlotInfo[i] );
                  *accessoryData = EndianSwapAccessoryData( *accessoryData );

                  // Don't sync invalid files
                  if( IsAccessoryDataValid( *accessoryData ) )
                  {
                     if( sSyncSingleWork->mTID.DoUniqueIDsMatch( (const unsigned char*)accessoryData->mSaveFile.transfarringID ) )
                     {
                        sSyncSingleWork->mCloudSlot = i + 1;
                        sSyncSingleWork->mSaveFile = accessoryData->mSaveFile;
                        break;
                     }
                  }
               }
               tempSlots >>= 1;
            }

            if( m_CloudFileList.IsCloudSaveIndexValid( sSyncSingleWork->mCloudSlot ) )
            {
               if( sSyncSingleWork->mTID.DoVersionIDsMatch( (unsigned char*)sSyncSingleWork->mSaveFile.cloudVersionHash ) )
               {
                  if( sSyncSingleWork->mTID.HasOfflineWork() )
                  {
                     // Version match and offline work: Load, then push to server
                     sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepLoad;
                  }
                  else
                  {
                     // Version match and no offline work: No action
                     sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepEnd;
                  }
               }
               else
               {
                  // Version mismatch and offline work: Resolve conflict
                  if( mGameThreadWorkInput_TreatVersionMismatchesAsConflicts || sSyncSingleWork->mTID.HasOfflineWork() )
                  {
                     sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepResolveConflict;
                  }
                  else
                  {
                     // Version mismatch and no offline work: Sync from server, then save
                     sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepSyncFromServer;
                     sSyncSingleWork->mHandle = BP_TUS_Post_GetSlotData( sSyncSingleWork->mCloudSlot );
                  }
               }
            }
            else
            {
               // Data is not present on the cloud; no action
               sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepEnd;
            }
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepResolveConflict:
      if ( mGameThreadWorkInput_ConflictResolution == kCCR_UseCloudSaveFile )
      {
         sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepSyncFromServer;
         sSyncSingleWork->mHandle = BP_TUS_Post_GetSlotData( sSyncSingleWork->mCloudSlot );
      }
      else if( mGameThreadWorkInput_ConflictResolution == kCCR_UseLocalSaveFile )
      {
         sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepLoad;
      }
      else if( mGameThreadWorkInput_ConflictResolution == kCCR_SkipSyncForConflictedFiles )
      {
         sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepEnd;
      }
      else
      {
         BPE_ASSERT( false, "SyncSingleCloudFile had an invalid conflict resolution type" );
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepSyncFromServer:
      STUSSlotInfo newSlotInfo;
      if( BP_TUS_Consume_GetSlotData(sSyncSingleWork->mHandle, &tusError, sSyncSingleWork->mCloudData, &newSlotInfo) )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            // Check the hash before endian swapping
            unsigned char cloudDataHash[TransfarringDefines::kHashLengthBytes]; // Save data hash
            TRANSFARRING_GAME_DATA* pCloudGameData = reinterpret_cast<TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData);
            BP_CreateMD5Hash( cloudDataHash, &pCloudGameData->mSaveData, SAVE_DATA_SIZE );

            SCloudAccessoryData* pAccessoryData = reinterpret_cast<SCloudAccessoryData*>( newSlotInfo.mAccessoryData );

            if( !memcmp(pAccessoryData->mSaveDataHash, cloudDataHash, sizeof(cloudDataHash) ) )
            {
               EndianSwapGameData( 
                  reinterpret_cast<TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData),
                  reinterpret_cast<const TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData),
                  mGameThreadWorkInput_SaveType );

               // Before saving, update the Transfarring ID to match the version number
               memcpy(sSyncSingleWork->mTID.m_VersionID, sSyncSingleWork->mSaveFile.cloudVersionHash, TransfarringDefines::kHashLengthBytes);
               sSyncSingleWork->mTID.m_HasOfflineWork = false; // There's no case where you will have offline work after syncing from the server
               SetTransfarringID(sSyncSingleWork->mTID, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);

               TRANSFARRING_GAME_DATA *const pReceivedData = reinterpret_cast<TRANSFARRING_GAME_DATA*>( sSyncSingleWork->mCloudData );
               // If the received PSN hash is zero, then disable trophies
               if( BP_IsMD5HashZero( pReceivedData->mBPLinkvars.mOriginalPSNAccount ) )
               {
                  pReceivedData->mBPLinkvars.mTrophyValidFlag = BPLinkvarDefines::kTrophiesInvalid;
               }
               
               sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepSave;
            }
            else
            {
               m_CloudFileList.SetCloudFileCorrupt( sSyncSingleWork->mCloudSlot, true );

               sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepEnd; // continue the loop
            }
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepSave:
      tusError = m_LocalFileList.SaveFile_GameThread(
         *(reinterpret_cast<TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData)),
         mGameThreadWorkInput_SaveType,
         mGameThreadWorkInput_SaveIndex );

      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            m_LocalFileList.SetConflict(false, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);

            sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepEnd;
         }
         else
         {
            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepLoad:
      tusError = m_LocalFileList.LoadFile_GameThread(
         *(reinterpret_cast<TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData)),
         mGameThreadWorkInput_SaveType,
         mGameThreadWorkInput_SaveIndex);
     
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            // Endian swap the game data and copy it into the larger cloud buffer
            EndianSwapGameData(
               reinterpret_cast<TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData), 
               reinterpret_cast<const TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData),
               mGameThreadWorkInput_SaveType);

            // Increment the variable on the server, then re-save the file
            sSyncSingleWork->mHandle = BP_TUS_Post_IncrementSlotVariable(1, 1ULL);
            sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepConsumeIncrementSlotVariable;
         }
         else
         {
            retval = tusError;
         }
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepConsumeIncrementSlotVariable:
      uint64 newVersionValue;
      if (BP_TUS_Consume_IncrementSlotVariable(sSyncSingleWork->mHandle, &tusError, NULL, &newVersionValue))
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            uint64 swappedVersionValue = newVersionValue;
            BP_LE_Swap8Bytes_Inp(&swappedVersionValue);
            TransfarringID::CreateHash((unsigned char*)sSyncSingleWork->mTID.m_VersionID, (unsigned char*)&swappedVersionValue, sizeof(swappedVersionValue));
            sSyncSingleWork->mTID.m_HasOfflineWork = false; // By pushing this to the server, we no longer have offline data
            SetTransfarringID(sSyncSingleWork->mTID, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);

            // Pack the transfarring save file into the accessory data
            TransfarringFileList::TTransfarringSaveFilePtr curFile = m_LocalFileList.GetSaveFilePtrPlatform(mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);
            BPE_ASSERT(curFile, "TransfarringManager::ProcessSyncCloudFiles::kCS_StepReSave was given a bad save file");

            TransfarringFileList::TTransfarringSaveFilePtr newFile = TTransfarringSaveFilePtr( new TransfarringSaveFile() );
            *newFile = *curFile;

            m_CloudFileList.AddCloudFilePtr(newFile, sSyncSingleWork->mCloudSlot);

            SCloudAccessoryData accessoryData;
            MakeAccessoryData( &accessoryData, reinterpret_cast<const TRANSFARRING_GAME_DATA*>( sSyncSingleWork->mCloudData ), newFile );
            accessoryData = EndianSwapAccessoryData( accessoryData );
            STUSSlotInfo newSlotInfo;
            memset( &newSlotInfo, 0, sizeof(newSlotInfo) );
            memcpy( newSlotInfo.mAccessoryData, &accessoryData, sizeof(accessoryData) );

            sSyncSingleWork->mHandle = BP_TUS_Post_SetSlotData(
               sSyncSingleWork->mCloudSlot, 
               sSyncSingleWork->mCloudData, 
               sizeof(sSyncSingleWork->mCloudData), 
               &newSlotInfo);

            sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepSyncToServer;
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepReSave:
      tusError = m_LocalFileList.SaveFile_GameThread(
         *(reinterpret_cast<TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData)), 
         mGameThreadWorkInput_SaveType,
         mGameThreadWorkInput_SaveIndex);

      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            m_LocalFileList.SetConflict(false, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);

            sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepEnd;
         }
         else
         {
            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepSyncToServer:
      if (BP_TUS_Consume_SetSlotData(sSyncSingleWork->mHandle, &tusError))
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            // Game data was endian swapped for uploading it to the server; need to swap it back before saving locally
            EndianSwapGameData(
               reinterpret_cast<TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData), 
               reinterpret_cast<const TRANSFARRING_GAME_DATA*>(sSyncSingleWork->mCloudData),
               mGameThreadWorkInput_SaveType);

            sSyncSingleWork->mWorkStep = TransfarringSyncSingle::kCSSF_StepReSave;
         }
         else
         {
            m_CloudFileList.RemoveCloudFile( sSyncSingleWork->mCloudSlot );
            retval = tusError;
            break;
         }
      }
      break;
   case TransfarringSyncSingle::kCSSF_StepEnd:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   default:
      BPE_ASSERT(false, "ProcessSyncSingleCloudFile tried to handle an invalid step" );
      break;
   }

   if (retval.mHighLevelError != kTE_Processing)
   {
      sSyncSingleWork.reset();
      ClearProgress();
   }

   return retval;
}

STransfarringError CTransfarringManager::ProcessGetCloudFileList()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   enum
   {
      kCSF_StepBegin,
      kCSF_StepProcessGetInfo,
      kCSF_StepEnd
   };

   static int sWorkStep = kCSF_StepBegin;
   static uint64 sHandle = 0;
   static STUSSlotInfo* sSlotInfo = NULL;

   uint8 validSlots;
   uint8 tempValidSlots;
   uint8 mask = 1;

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID(tusError);

   switch (sWorkStep)
   {
   case kCSF_StepBegin:
      m_CloudFileList.ClearCloudFiles();
      sHandle = BP_TUS_Post_GetSlotInfos();
      BPE_VERIFY( sSlotInfo == NULL, false, "Slot Info should be NULL here!" );
      sSlotInfo = new STUSSlotInfo[8];
      sWorkStep = kCSF_StepProcessGetInfo;
      break;
   case kCSF_StepProcessGetInfo:
      if (BP_TUS_Consume_GetSlotInfos(sHandle, &tusError, sSlotInfo, &validSlots))
      {
         if (tusError.mHighLevelError == kTE_Success)
         {
            // Fill the list of saves with the accessory data
            tempValidSlots = validSlots;
            for (int i=0; i < 8; i++)
            {
               if (tempValidSlots & mask != 0)
               {
                  // Endian swap the accessory data
                  SCloudAccessoryData accessoryData;
                  memcpy( &accessoryData, sSlotInfo[i].mAccessoryData, sizeof( SCloudAccessoryData ) );
                  accessoryData = EndianSwapAccessoryData( accessoryData );

                  // Copy the fast-access info
                  TransfarringFileList::TTransfarringSaveFilePtr newFile( new TransfarringSaveFile );
                  memcpy( newFile.get(), &accessoryData.mSaveFile, sizeof( TransfarringSaveFile ) );

                  // Check for file corruption (region mismatch, cookie mismatch, etc)
                  if( !IsAccessoryDataValid( accessoryData ) )
                  {
                     m_CloudFileList.SetCloudFileCorrupt( i + 1, true );

                     // Generate a unique transfarring ID; otherwise it's theoretically possible to have collisions from junk data
                     TransfarringID uniqueTID;
                     uniqueTID.GenerateUniqueID();
                     memcpy( newFile->transfarringID, uniqueTID.m_UniqueID, sizeof( newFile->transfarringID ) );
                  }

                  // Add it to the cloud file list
                  m_CloudFileList.AddCloudFilePtr(newFile, i + 1);
               }
               tempValidSlots >>= 1;
            }

            sWorkStep = kCSF_StepEnd;
         }
         else
         {
            retval = tusError;
            break;
         }
      }
      break;
   case kCSF_StepEnd:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   }

   if ( retval.mHighLevelError != kTE_Processing )
   {
      sWorkStep = kCSF_StepBegin;
      delete [] sSlotInfo;
      sSlotInfo = NULL;
      ClearProgress();
   }

   return retval;
}

namespace TransfarringCheckSyncNeeded
{
   enum ECheckSyncNeededSteps
   {
      kCSN_StepBegin,
      kCSN_StepEnd
   };

   class TransfarringCheckSyncNeededWork
   {
   public:

      TransfarringCheckSyncNeededWork()
      {
         mWorkStep = kCSN_StepBegin;
         mHandle = 0;
         memset( mSlotInfo, 0, sizeof(mSlotInfo) );
         mValidSlots = 0;
      }

      ECheckSyncNeededSteps mWorkStep;
      unsigned long long mHandle;
      STUSSlotInfo mSlotInfo[ 8 ];
      uint8 mValidSlots;
   };
}

STransfarringError CTransfarringManager::ProcessCheckSyncNeeded()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID( tusError );

   // Init work
   static boost::scoped_ptr<TransfarringCheckSyncNeeded::TransfarringCheckSyncNeededWork> sCheckSyncNeededWork( NULL );

   if (!sCheckSyncNeededWork.get())
   {
      sCheckSyncNeededWork.reset( new TransfarringCheckSyncNeeded::TransfarringCheckSyncNeededWork() );
   }

   switch (sCheckSyncNeededWork->mWorkStep)
   {
   case TransfarringCheckSyncNeeded::kCSN_StepBegin:
      sCheckSyncNeededWork->mHandle = BP_TUS_Post_GetSlotInfos();
      sCheckSyncNeededWork->mWorkStep = TransfarringCheckSyncNeeded::kCSN_StepEnd;
      break;
   case TransfarringCheckSyncNeeded::kCSN_StepEnd:
      if( BP_TUS_Consume_GetSlotInfos( 
         sCheckSyncNeededWork->mHandle, 
         &tusError, 
         sCheckSyncNeededWork->mSlotInfo, 
         &sCheckSyncNeededWork->mValidSlots ) )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            int needsSync = 0;

            // Look at each slot and compare it against the local transfarring IDs to see if we need to sync files.
            uint8 mask = 1;
            uint8 tempSlots = sCheckSyncNeededWork->mValidSlots;
            for (int i=0; i < 8; i++)
            {
               if ( (tempSlots & mask) != 0 )
               {
                  // Get the accessory data
                  SCloudAccessoryData* pAccessoryData = reinterpret_cast<SCloudAccessoryData*>( &sCheckSyncNeededWork->mSlotInfo[i] );
                  *pAccessoryData = EndianSwapAccessoryData( *pAccessoryData );

                  // Ignore corrupt data or save types that we're not syncing
                  if( !IsAccessoryDataValid( *pAccessoryData ) )
                  {
                     continue;
                  }

                  if( mGameThreadWorkInput_SaveType != pAccessoryData->mSaveFile.mSaveType )
                  {
                     continue;
                  }

                  // Find the local file that corresponds to it
                  ESaveType saveType;
                  int saveIndex;
                  GetSaveFileFromUniqueID(reinterpret_cast<unsigned char*>(pAccessoryData->mSaveFile.transfarringID), saveType, saveIndex);

                  // If we don't have the file, and the cloud file is valid, we need to sync
                  if( saveIndex < 0 || saveType == kST_MaxSaveTypes )
                  {
                     needsSync = 1;
                     break;
                  }
                  else
                  {
                     TransfarringID tid = GetTransfarringID( saveType, saveIndex );

                     if (tid.DoVersionIDsMatch((unsigned char*)pAccessoryData->mSaveFile.cloudVersionHash))
                     {
                        if (tid.HasOfflineWork())
                        {
                           // Version match and offline work: Needs sync
                           needsSync = 1;
                           break;
                        }
                     }
                     else
                     {
                        // Version mismatch and offline work: Set conflict, don't sync
                        if ( mGameThreadWorkInput_TreatVersionMismatchesAsConflicts || tid.HasOfflineWork() )
                        {
                           m_LocalFileList.SetConflict( true, saveType, saveIndex );
                        }
                        else
                        {
                           // Version mismatch and no offline work: Needs sync
                           needsSync = 1;
                           break;
                        }
                     }
                  }
               }
               tempSlots >>= 1;
            }

            SET_TRANSFARRING_ERROR( retval, kTE_Success, needsSync); // KLUDGE: Put the success value in the low level error slot
         }
         else
         {
            retval = tusError;
         }
      }
      break;
   default:
      BPE_ASSERT(false, "ProcessCheckSyncNeeded step is out of bounds");
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      sCheckSyncNeededWork.reset();
   }

   return retval;
}

namespace TransfarringDelete
{
   enum EDeleteSteps
   {
      kCD_StepBegin,
      kCD_StepEnd
   };

   class TransfarringDeleteWork
   {
   public:

      TransfarringDeleteWork()
      {
         mWorkStep = kCD_StepBegin;
      }

      EDeleteSteps mWorkStep;
   };
}

STransfarringError CTransfarringManager::ProcessDeleteSaveFile()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID( tusError );

   // Init work
   static TransfarringDelete::TransfarringDeleteWork* sDeleteWork = NULL;

   if (!sDeleteWork)
   {
      sDeleteWork = new TransfarringDelete::TransfarringDeleteWork();
   }

   switch (sDeleteWork->mWorkStep)
   {
   case TransfarringDelete::kCD_StepBegin:
      tusError = m_LocalFileList.DeleteFile_GameThread(mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            sDeleteWork->mWorkStep = TransfarringDelete::kCD_StepEnd;
         }
         else
         {
            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }
      }
      break;
   case TransfarringDelete::kCD_StepEnd:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   default:
      BPE_ASSERT(false, "ProcessWork step is out of bounds");
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      delete sDeleteWork;
      sDeleteWork = NULL;
      ClearProgress();
   }

   return retval;
}

namespace TransfarringDeletePhoto
{
   enum EDeletePhotoSteps
   {
      kCDP_StepBegin,
      kCDP_StepEnd
   };

   class TransfarringDeletePhotoWork
   {
   public:

      TransfarringDeletePhotoWork()
      {
         mWorkStep = kCDP_StepBegin;
      }

      EDeletePhotoSteps mWorkStep;
   };
}

STransfarringError CTransfarringManager::ProcessDeleteNonGameFile()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   STransfarringError tusError;
   SET_TRANSFARRING_ERROR_INVALID( tusError );

   // Init work
   static TransfarringDeletePhoto::TransfarringDeletePhotoWork* sDeleteWork = NULL;

   if (!sDeleteWork)
   {
      sDeleteWork = new TransfarringDeletePhoto::TransfarringDeletePhotoWork();
   }

   switch( sDeleteWork->mWorkStep )
   {
   case TransfarringDeletePhoto::kCDP_StepBegin:
      tusError = m_LocalFileList.DeleteNonGameFile_GameThread( mGameThreadWorkInput_NonGameSaveType, mGameThreadWorkInput_SaveIndex );
      if( tusError.mHighLevelError != kTE_Processing )
      {
         if( tusError.mHighLevelError == kTE_Success )
         {
            sDeleteWork->mWorkStep = TransfarringDeletePhoto::kCDP_StepEnd;
         }
         else
         {
            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }

      }
      break;
   case TransfarringDeletePhoto::kCDP_StepEnd:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   default:
      BPE_ASSERT(false, "ProcessWork step is out of bounds");
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      delete sDeleteWork;
      sDeleteWork = NULL;
      ClearProgress();
   }

   return retval;
}

namespace TransfarringUnlock
{
   enum EUnlockSteps
   {
      kCU_StepBegin,
      kCU_StepSave,
      kCU_StepEnd
   };

   class TransfarringUnlockWork
   {
   public:

      TransfarringUnlockWork()
      {
         CTransfarringManager::Instance()->StartProgress( 2 );
         mWorkStep = kCU_StepBegin;
      }

      EUnlockSteps mWorkStep;
      TRANSFARRING_GAME_DATA mGameData;
   };
}

STransfarringError CTransfarringManager::ProcessUnlockSaveFile()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR(retval, kTE_Processing, 0);

   static TransfarringUnlock::TransfarringUnlockWork* sUnlockWork = NULL;

   // Init work
   if (!sUnlockWork)
   {
      sUnlockWork = new TransfarringUnlock::TransfarringUnlockWork();
   }

   STransfarringError fileError;
   SET_TRANSFARRING_ERROR_INVALID( fileError );

   switch (sUnlockWork->mWorkStep)
   {
   case TransfarringUnlock::kCU_StepBegin:
      fileError = m_LocalFileList.LoadFile_GameThread(sUnlockWork->mGameData, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);
      if( fileError.mHighLevelError != kTE_Processing )
      {
         if( fileError.mHighLevelError == kTE_Success )
         {
            sUnlockWork->mWorkStep = TransfarringUnlock::kCU_StepSave;

            // Clear transfarring ID
            TransfarringID newTID = GetTransfarringID( mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex );
            newTID.Invalidate();
            SetTransfarringID(newTID, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);
         }
         else
         {
            retval = fileError;
         }
      }
      break;
   case TransfarringUnlock::kCU_StepSave:
      fileError = m_LocalFileList.SaveFile_GameThread(sUnlockWork->mGameData, mGameThreadWorkInput_SaveType, mGameThreadWorkInput_SaveIndex);
      if( fileError.mHighLevelError != kTE_Processing )
      {
         if( fileError.mHighLevelError == kTE_Success )
         {
            sUnlockWork->mWorkStep = TransfarringUnlock::kCU_StepEnd;
         }
         else
         {
            SET_TRANSFARRING_ERROR( retval, kTE_Unknown, 0 );
         }
      }
      break;
   case TransfarringUnlock::kCU_StepEnd:
      SET_TRANSFARRING_ERROR_SUCCESS(retval);
      break;
   default:
      BPE_ASSERT(false, "ProcessWork step is out of bounds");
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      delete sUnlockWork;
      sUnlockWork = NULL;
      ClearProgress();
   }

   return retval;
}
