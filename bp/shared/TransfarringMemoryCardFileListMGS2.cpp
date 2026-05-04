//----------------------------------------------------------------------------
// TransfarringMemoryCardFileList.h
//
// A list of files that can manipulate the save game system
//----------------------------------------------------------------------------

#include "TransfarringMemoryCardFileList.h"

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#if defined(BP_VITA)
#include "TransfarringVTA.h"
#include "TransfarringVTACGlue.h"
#endif

#if defined(BP_PS3)
#include "Transfarring_PS3.h"
#include "Transfarring_PS3CGlue.h"
#include <sys/sys_time.h>
#include <sys/timer.h>
#endif

#include "BP_SaveLoadMGS.h"

extern "C" {

#include "mode/codec/cdc_load.h"
#include "libfs.h"

int BP_FRAMES_PER_SEC(void);
float Transfarring_CalcVRAchievementRatio( const char* vrScoreDataBuffer );
void EncodeVRInfo( MCMAN_INFODATA* pDst, const MCMAN_VR_INFODATA* pSrc );
void DecodeVRInfo( MCMAN_VR_INFODATA* pDst, const MCMAN_INFODATA* pSrc );
GCL_VAR_REF	TTL_ReferenceVariable[ 6 ];
void CodeData(void *data,int size,unsigned int *seed);
void DecodeData(void *data,int size,unsigned int *seed);
unsigned int calc_crc_code( void *data, int len );
int DecodeDataName_Kind(MCMAN_WORK *work,const char *name,void *info, int file_kind);

}

// Forward declarations of internal game-specific functions that aren't part of the interface
static void Transfarring_SaveGameInfo( MCMAN_INFODATA *info, const char* linkvar, STransfarringSaveDate & const saveDate );
static void Transfarring_SaveVRInfo( MCMAN_VR_INFODATA* pInfo, const TRANSFARRING_GAME_DATA& data);
static TransfarringFileList::TTransfarringSaveFilePtr Transfarring_InfodataToSaveFile( MCMAN_INFODATA* info, ESaveType saveType, int saveIndex );
static STransfarringSaveDate MakeSaveDateFromInfo( MCMAN_INFODATA* info, ESaveType saveType, int saveIndex );

//HACK to deal with MGSTWO-3242
#if defined(BP_VITA) || defined(BP_PS3)
extern "C" int gGameClearChangeDetected = 0;
#endif

namespace
{
   class PushSaveDateOverrideHelper
   {
   public:
      PushSaveDateOverrideHelper( STransfarringSaveDate date )
      {
         mWasOverride = CTransfarringManager::Instance()->GetOverrideSaveDate( &mLastSaveDate );
         CTransfarringManager::Instance()->SetOverrideSaveDate( true, &date );
      }

      ~PushSaveDateOverrideHelper()
      {
         CTransfarringManager::Instance()->SetOverrideSaveDate( mWasOverride, &mLastSaveDate );
      }

      bool mWasOverride;
      STransfarringSaveDate mLastSaveDate;
   };
}

TransfarringMemoryCardFileList::TransfarringMemoryCardFileList(MEMORY_CARD_ACTOR* memcardActor)
: m_memcardActor(memcardActor)
{
}

void TransfarringMemoryCardFileList::SetMemoryCardActor( MEMORY_CARD_ACTOR* memcardActor )
{
   BPE_ASSERT( memcardActor == NULL || m_memcardActor == NULL, "TransfarringMemoryCardFileList tried to overwrite its memory card actor" );

   m_memcardActor = memcardActor;
}

ESaveType TransfarringMemoryCardFileList::GetMemoryCardActorType() const
{
   return Transfarring_KonamiSaveTypeToBPSaveType(m_memcardActor->file_kind);
}

STransfarringError TransfarringMemoryCardFileList::LoadAllFileLists_SubThread()
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR_SUCCESS( retval );

   for (int curSaveType = kST_Game; curSaveType < kST_MaxSaveTypes; curSaveType++)
   {
      LoadFileList_SubThread((ESaveType)curSaveType);
   }

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::LoadFileList_SubThread(ESaveType saveType)
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call LoadFileList_SubThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );

   SetMemoryCardActorType(saveType);

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   GameEasySearchStart(0);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int mcmanResult = WaitForMemoryCard_SubThread();
   if( mcmanResult < 0 )
   {
      return retval;
   }

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   GetGameInfoOnlyBGStart(0);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   mcmanResult = WaitForMemoryCard_SubThread();
   if( mcmanResult < 0 )
   {
      return retval;
   }

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   GetGameInfoOnlyRBGStart(0);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   mcmanResult = WaitForMemoryCard_SubThread();
   if( mcmanResult < 0 )
   {
      return retval;
   }

   for (int i=0; i < MCMAN_GetDataFileMax(m_memcardActor->file_kind); i++)
   {
      if (MCManCheckExactFileFlag(i))
      {
         TTransfarringSaveFilePtr file = Transfarring_InfodataToSaveFile( (MCMAN_INFODATA*)MCManGetFileInfo(i), saveType, i );

         AddSaveFilePtr(file, saveType, file->mSaveIndex);
      }
   }

   SET_TRANSFARRING_ERROR_SUCCESS( retval );

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::SaveFile_SubThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int saveIndex)
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call SaveFile_SubThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );

   // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
   CTransfarringManager::Instance()->LockTransfarringIDUpdates();

   SetMemoryCardActorType(saveType);

   PushSaveDateOverrideHelper( data.mExtraData.mSaveDate );

   // Calculate CRC
   unsigned int crc = calc_crc_code( data.mSaveData.linkvar, MAX_LINKVARBUF );
   crc ^= calc_crc_code( data.mSaveData.varbuf, MAX_VAR_BUF );
   memcpy(data.mSaveData.crc, &crc, sizeof(unsigned int));

   // Start the search
   CTransfarringManager::Instance()->LockSaveLoadMutex();
   GameEasySearchStart(0);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int mcmanResult = WaitForMemoryCard_SubThread();
   if( mcmanResult < 0 )
   {
      return retval;
   }

   // Get the game info
   CTransfarringManager::Instance()->LockSaveLoadMutex();
   GetGameInfoOnlyRBGStart(0);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   mcmanResult = WaitForMemoryCard_SubThread();
   if( mcmanResult < 0 )
   {
      return retval;
   }

   // Create the game info from the linkvar data while it's uncoded
   MCMAN_INFODATA info;
   MCMAN_VR_INFODATA vrInfo;
   if (saveType == kST_VR)
   {
      Transfarring_SaveVRInfo(&vrInfo, data);
      EncodeVRInfo(&info, &vrInfo);
   }
   else
   {
      Transfarring_SaveGameInfo( &info, data.mSaveData.linkvar, data.mExtraData.mSaveDate );
   }

   if (saveType == kST_SnakeTales)
   {
      info.mgs2_playtime = 0; // Snake tales does not keep track of play time
      info.snake_tales_no = data.mExtraData.snake_tales_no;
      info.st_clear_flag = data.mExtraData.st_clear_flag;
   }

   // Seed and code the data
   CTransfarringManager::Instance()->LockSaveLoadMutex();

   unsigned int* seedPtr = (unsigned int*)data.mSaveData.seed;
   CodeData(data.mSaveData.linkvar, MAX_LINKVARBUF, seedPtr);
   CodeData(data.mSaveData.varbuf, MAX_VAR_BUF, seedPtr);
   if( m_memcardActor->file_kind == MCMAN_FILE_KIND_VR ) {
      CodeData(data.mSaveData.vr_score_data, MSN_SAVE_DATA_SIZE, seedPtr );
   } else {
      CodeData(data.mSaveData.tanker_photo_data, GM_TANKER_PICTURE_SIZE, seedPtr);
   }

   // Save BP linkvars
   SBP_LinkVars saved_bp_linkvars;
   SBP_LinkVars saved_sv_bp_linkvars;
   memcpy( &saved_bp_linkvars, &bp_linkvars, sizeof(SBP_LinkVars) );
   memcpy( &saved_sv_bp_linkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars) );

   // Copy data BP linkvars into the structures that actually get saved
   memcpy(&bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));
   memcpy(&sv_bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));

   // Save the game
   SaveGameStart(0, saveIndex, &data.mSaveData, SAVE_DATA_SIZE, NULL, 0, &info);

   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   mcmanResult = WaitForMemoryCard_SubThread();
   if( mcmanResult < 0 )
   {
      return retval;
   }

   // Add it to the save file list
   TTransfarringSaveFilePtr newFile( new TransfarringSaveFile() );

   newFile->mSaveIndex = saveIndex;
   newFile->mSaveType = saveType;
   newFile->mStageIndex = info.stage_num;
   newFile->mDifficulty = info.difficulty;
   newFile->mClearCount = info.clear_count;

   if( saveType == kST_VR )
   {
      newFile->mPlayTime = vrInfo.vr_playtime;
   }
   else
   {
      newFile->mPlayTime = info.mgs2_playtime;
   }

   newFile->mSaveDate = data.mExtraData.mSaveDate;

   ConcatenatedSaveIndex concatIndex = GetConcatenatedSaveIndexFromPlatform(saveType, saveIndex);
   TTransfarringSaveFilePtr curFile = GetSaveFilePtr(saveType, concatIndex);
   if (curFile)
   {
      // Be sure to preserve the transfarring ID here
      long long tempTID[2];
      long long tempVersion[2];
      long long tempPSNAccount[2];
      memcpy(tempTID, curFile->transfarringID, TransfarringDefines::kHashLengthBytes);
      memcpy(tempVersion, curFile->cloudVersionHash, TransfarringDefines::kHashLengthBytes);
      memcpy(tempPSNAccount, curFile->psnAccountHash, TransfarringDefines::kHashLengthBytes);

      *curFile = *newFile;

      TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, newFile->mSaveIndex);
      if( tid.m_Type != kTT_Invalid )
      {
         memcpy(curFile->transfarringID, tempTID, TransfarringDefines::kHashLengthBytes);
      }
      else
      {
         memset(curFile->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
      }
      
      memcpy(curFile->cloudVersionHash, tempVersion, TransfarringDefines::kHashLengthBytes);
      memcpy(curFile->psnAccountHash, tempPSNAccount, TransfarringDefines::kHashLengthBytes);
   }
   else
   {
      // Fill in the transfarring ID
      TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, newFile->mSaveIndex);
      if( tid.m_Type != kTT_Invalid )
      {
         memcpy(newFile->transfarringID, tid.m_UniqueID, TransfarringDefines::kHashLengthBytes);
      }
      else
      {
         memset(newFile->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
      }

      memcpy(newFile->cloudVersionHash, tid.m_VersionID, TransfarringDefines::kHashLengthBytes);
      memcpy(newFile->psnAccountHash, tid.m_PSNAccountName, TransfarringDefines::kHashLengthBytes);

      // Add it to the list of saves so we can detect empty slots
      AddSaveFilePtr(newFile, saveType, newFile->mSaveIndex);
   }

   CTransfarringManager::Instance()->AllowTransfarringIDUpdates();

   // Restore BP linkvars
   memcpy(&bp_linkvars, &saved_bp_linkvars, sizeof(SBP_LinkVars));
   memcpy(&sv_bp_linkvars, &saved_sv_bp_linkvars, sizeof(SBP_LinkVars));

   SET_TRANSFARRING_ERROR_SUCCESS( retval );

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::LoadFile_SubThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int loadIndex)
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call LoadFile_SubThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );

   // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
   CTransfarringManager::Instance()->LockTransfarringIDUpdates();

   SetMemoryCardActorType(saveType);

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   GameEasySearchStart(0);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int memcardResult = WaitForMemoryCard_SubThread();
   if( memcardResult < 0 )
   {
      return retval;
   }

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   GetGameInfoOnlyRBGStart(0);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   memcardResult = WaitForMemoryCard_SubThread();
   if( memcardResult < 0 )
   {
      return retval;
   }

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   LoadGameStart(0, loadIndex, &data.mSaveData);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   memcardResult = WaitForMemoryCard_SubThread();
   if( memcardResult < 0 )
   {
      return retval;
   }

   if( MGS_SaveStatus_WrongUser() )
   {
      // Reset BP linkvars and clear wrong user flag
      memset( &sv_bp_linkvars, 0, sizeof(sv_bp_linkvars) );

      MGS_SaveStatus_ClearWrongUser();

      SET_TRANSFARRING_ERROR( retval, kTE_OwnershipError, 0 );

      return retval;
   }

   unsigned int* _seed = (unsigned int*)&data.mSaveData.seed;

   DecodeData(data.mSaveData.linkvar,MAX_LINKVARBUF,_seed);
   DecodeData(data.mSaveData.varbuf,MAX_VAR_BUF,_seed);
   if( m_memcardActor->file_kind == MCMAN_FILE_KIND_VR ) {
      DecodeData(data.mSaveData.vr_score_data, MSN_SAVE_DATA_SIZE, _seed );
   } else {
      DecodeData(data.mSaveData.tanker_photo_data, GM_TANKER_PICTURE_SIZE, _seed );
   }

   // Copy BP linkvars that were just loaded
   memcpy(&data.mBPLinkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars));

   // Fill in extra data from Snake Tales saves
   if (saveType == kST_SnakeTales)
   {
      MCMAN_INFODATA* info = (MCMAN_INFODATA*)MCManGetFileInfo(loadIndex);

      data.mExtraData.snake_tales_no = info->snake_tales_no;
      data.mExtraData.st_clear_flag = info->st_clear_flag;
   }

   // Update anything that can be changed at game save time
   MCMAN_INFODATA* info = (MCMAN_INFODATA*)MCManGetFileInfo( loadIndex );

   // Set transfarring date
   data.mExtraData.mSaveDate = MakeSaveDateFromInfo( info, saveType, loadIndex );

   TTransfarringSaveFilePtr filePtr = GetSaveFilePtrPlatform( saveType, loadIndex );

   filePtr->mClearCount = info->clear_count;
   filePtr->mPlayTime = info->mgs2_playtime;
   filePtr->mSaveDate = data.mExtraData.mSaveDate;
   filePtr->mStageIndex = info->stage_num;

   if( saveType == kST_VR )
   {
      MCMAN_VR_INFODATA vrinfo;
      DecodeVRInfo( &vrinfo, info );
      filePtr->mPlayTime = vrinfo.vr_playtime;
   }

   CTransfarringManager::Instance()->AllowTransfarringIDUpdates();

   SET_TRANSFARRING_ERROR_SUCCESS( retval );

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::DeleteFile_SubThread(ESaveType saveType, int deleteIndex)
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call DeleteFile_SubThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );

#if BP_VITA
   SetMemoryCardActorType(saveType);

   CTransfarringManager::Instance()->m_bNeedsRealDelete = true;

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   DeleteGameStart(deleteIndex);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int mcmanResult = WaitForMemoryCard_SubThread();
   if( mcmanResult < 0 )
   {
      return retval;
   }

   RemoveSaveFile(saveType, deleteIndex);

   CTransfarringManager::Instance()->m_bNeedsRealDelete = false;
#else
   BPE_ASSERT(false, "Transfarring file deletion only valid on PS Vita");
#endif

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::LoadAllFileLists_GameThread()
{
   enum
   {
      kTMC_LoadFileListInit,
      kTMC_LoadFileListBegin,
      kTMC_LoadFileLists,
      kTMC_LoadFileLists2,
      kTMC_LoadInfoList,
      kTMC_LoadFileListEnd,
   };

   static int step = kTMC_LoadFileListBegin;

   BPE_ASSERT( m_memcardActor != NULL, "Tried to call LoadAllFileLists_GameThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

   int mcmanResult = 0;

   switch (step)
   {
   case kTMC_LoadFileListInit:
      step = kTMC_LoadFileListBegin;
      break;
   case kTMC_LoadFileListBegin:
      if( MCManChecked() || !MCManCheckingOrChecked() )
      {
         GameEasySearchStart(0);
         step = kTMC_LoadFileLists;
      }
      break;
   case kTMC_LoadFileLists:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         GetGameInfoOnlyBGStart(0);
         step = kTMC_LoadFileLists2;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_LoadFileLists2:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         GetGameInfoOnlyRBGStart(0);
         step = kTMC_LoadInfoList;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_LoadInfoList:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         ESaveType saveType = Transfarring_KonamiSaveTypeToBPSaveType(m_memcardActor->file_kind);
         for (int i=0; i < MCMAN_GetDataFileMax(m_memcardActor->file_kind); i++)
         {
            if (MCManCheckExactFileFlag(i))
            {
               MCMAN_INFODATA* info = (MCMAN_INFODATA*)MCManGetFileInfo(i);

               TTransfarringSaveFilePtr file = Transfarring_InfodataToSaveFile( info, saveType, i );

               AddSaveFilePtr(file, saveType, file->mSaveIndex);
            }
         };

         step = kTMC_LoadFileListEnd;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_LoadFileListEnd:
      {
         SET_TRANSFARRING_ERROR_SUCCESS( retval );
      }
      break;
   default:
      BPE_ASSERT(step < kTMC_LoadFileListEnd, "TransfarringMemoryCardFileList::LoadFile_GameThread step is out of bounds");
      step++;
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      step = kTMC_LoadFileListBegin;
   }

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::LoadFile_GameThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int loadIndex)
{
   enum
   {
      kTMC_LoadStepBegin = 0,
      kTMC_LoadStepSearchStart = 1,
      kTMC_LoadStepInfoSearchStart = 2,
      kTMC_LoadStepLoadGame = 3,
      kTMC_LoadStepEnd = 10
   };

   static int step = kTMC_LoadStepBegin;
   static SBP_LinkVars s_saved_bp_linkvars;
   static SBP_LinkVars s_saved_sv_bp_linkvars;

   BPE_ASSERT( m_memcardActor != NULL, "Tried to call LoadFile_GameThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

   int mcmanResult = 0;

   switch (step)
   {
   case kTMC_LoadStepBegin:
      // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
      CTransfarringManager::Instance()->LockTransfarringIDUpdates();

      SetMemoryCardActorType(saveType);

      // Save BP linkvars that get slammed during loading
      memcpy( &s_saved_bp_linkvars, &bp_linkvars, sizeof(SBP_LinkVars) );
      memcpy( &s_saved_sv_bp_linkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars) );

      step++;
      break;
   case kTMC_LoadStepSearchStart:
      GameEasySearchStart(0);
      step++;
      break;
   case kTMC_LoadStepInfoSearchStart:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1)
      {
         GetGameInfoOnlyRBGStart(0);
         step++;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_LoadStepLoadGame:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1)
      {
         LoadGameStart(0, loadIndex, &data.mSaveData);
         step++;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_LoadStepEnd:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1)
      {
         if( MGS_SaveStatus_WrongUser() )
         {
            // Reset BP linkvars and clear wrong user flag
            memset( &sv_bp_linkvars, 0, sizeof(sv_bp_linkvars) );

            MGS_SaveStatus_ClearWrongUser();

            SET_TRANSFARRING_ERROR( retval, kTE_OwnershipError, 0 );
            break;
         }

         unsigned int* _seed = (unsigned int*)&data.mSaveData.seed;

         DecodeData(data.mSaveData.linkvar,MAX_LINKVARBUF,_seed);
         DecodeData(data.mSaveData.varbuf,MAX_VAR_BUF,_seed);
         if( m_memcardActor->file_kind == MCMAN_FILE_KIND_VR ) {
            DecodeData(data.mSaveData.vr_score_data, MSN_SAVE_DATA_SIZE, _seed );
         } else {
            DecodeData(data.mSaveData.tanker_photo_data, GM_TANKER_PICTURE_SIZE, _seed );
         }

         // Copy BP linkvars that were just loaded
         memcpy(&data.mBPLinkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars));

         // Reset BP linkvars so they don't dangle
         memcpy( &bp_linkvars, &s_saved_bp_linkvars, sizeof(SBP_LinkVars) );
         memcpy( &sv_bp_linkvars, &s_saved_sv_bp_linkvars, sizeof(SBP_LinkVars) );

         // Update anything that can be changed at game save time
         MCMAN_INFODATA* info = (MCMAN_INFODATA*)MCManGetFileInfo( loadIndex );

         // Fill in extra data from Snake Tales saves
         if (saveType == kST_SnakeTales)
         {
            data.mExtraData.snake_tales_no = info->snake_tales_no;
            data.mExtraData.st_clear_flag = info->st_clear_flag;
         }

         TTransfarringSaveFilePtr filePtr = GetSaveFilePtrPlatform( saveType, loadIndex );

         // If the file list wasn't loaded beforehand, add the file here
         if( !filePtr.get() )
         {
            filePtr = Transfarring_InfodataToSaveFile( info, saveType, loadIndex );
            AddSaveFilePtr( filePtr, saveType, loadIndex );
         }

         data.mExtraData.mSaveDate = MakeSaveDateFromInfo( info, saveType, loadIndex );

         filePtr->mClearCount = info->clear_count;
         filePtr->mPlayTime = info->mgs2_playtime;
         filePtr->mSaveDate = data.mExtraData.mSaveDate;
         filePtr->mStageIndex = info->stage_num;

         if( saveType == kST_VR )
         {
            MCMAN_VR_INFODATA vrinfo;
            DecodeVRInfo( &vrinfo, info );
            filePtr->mPlayTime = vrinfo.vr_playtime;
         }

         SET_TRANSFARRING_ERROR_SUCCESS( retval );

      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_CorruptSaveData, 0 );
      }
      break;
   default:
      BPE_ASSERT(step < kTMC_LoadStepEnd, "TransfarringMemoryCardFileList::LoadFile_GameThread step is out of bounds");
      step++;
      break;
   };

   if( retval.mHighLevelError != kTE_Processing )
   {
      CTransfarringManager::Instance()->AllowTransfarringIDUpdates();

      step = kTMC_LoadStepBegin;
   }

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::SaveFile_GameThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int saveIndex)
{
   enum
   {
      kTMC_SaveStepBegin = 0,
      kTMC_SaveStepSearchStart = 1,
      kTMC_SaveStepInfoSearchStart = 2,
      kTMC_SaveStepStartSave = 3,
      kTMC_SaveStepEnd = 10
   };

   static int step = kTMC_SaveStepBegin;
   static unsigned int crc = 0;
   static MCMAN_INFODATA info;
   static unsigned int playtime = 0;
   static SBP_LinkVars s_saved_bp_linkvars;
   static SBP_LinkVars s_saved_sv_bp_linkvars;

   BPE_ASSERT( m_memcardActor != NULL, "Tried to call SaveFile_GameThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

   int mcmanResult = 0;

   switch (step)
   {
   case kTMC_SaveStepBegin:
      // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
      CTransfarringManager::Instance()->LockTransfarringIDUpdates();

      SetMemoryCardActorType(saveType);

      // Calculate CRC
      crc = calc_crc_code( data.mSaveData.linkvar, MAX_LINKVARBUF );
      crc ^= calc_crc_code( data.mSaveData.varbuf, MAX_VAR_BUF );
      memcpy(data.mSaveData.crc, &crc, sizeof(unsigned int));

      // Save BP linkvars that get set during saving
      memcpy( &s_saved_bp_linkvars, &bp_linkvars, sizeof(SBP_LinkVars) );
      memcpy( &s_saved_sv_bp_linkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars) );

      CTransfarringManager::Instance()->SetOverrideSaveDate( true, &data.mExtraData.mSaveDate );

      step++;
      break;
   case kTMC_SaveStepSearchStart:
      GameEasySearchStart(0);
      step++;
      break;
   case kTMC_SaveStepInfoSearchStart:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         GetGameInfoOnlyRBGStart(0);
         step++;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_SaveStepStartSave:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         // Create the game info from the linkvar data while it's uncoded
         if (saveType == kST_VR)
         {
            MCMAN_VR_INFODATA vrInfo;
            Transfarring_SaveVRInfo(&vrInfo, data);
            EncodeVRInfo(&info, &vrInfo);
            playtime = vrInfo.vr_playtime;
         }
         else
         {
            Transfarring_SaveGameInfo( &info, data.mSaveData.linkvar, data.mExtraData.mSaveDate );
            playtime = info.mgs2_playtime;
         }

         if (saveType == kST_SnakeTales)
         {
            playtime = info.mgs2_playtime = 0; // Snake Tales doesn't keep track of play time
            info.snake_tales_no = data.mExtraData.snake_tales_no;
            info.st_clear_flag = data.mExtraData.st_clear_flag;
         }

         // Seed and code the data
         CTransfarringManager::Instance()->LockSaveLoadMutex();

         unsigned int* seedPtr = (unsigned int*)data.mSaveData.seed;
         CodeData(data.mSaveData.linkvar, MAX_LINKVARBUF, seedPtr);
         CodeData(data.mSaveData.varbuf, MAX_VAR_BUF, seedPtr);
         if( m_memcardActor->file_kind == MCMAN_FILE_KIND_VR ) {
            CodeData(data.mSaveData.vr_score_data, MSN_SAVE_DATA_SIZE, seedPtr );
         } else {
            CodeData(data.mSaveData.tanker_photo_data, GM_TANKER_PICTURE_SIZE, seedPtr);
         }

         // Save BP linkvars
         memcpy(&bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));
         memcpy(&sv_bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));

         // Save the game
         SaveGameStart(0, saveIndex, &data.mSaveData, SAVE_DATA_SIZE, NULL, 0, &info);

         CTransfarringManager::Instance()->UnlockSaveLoadMutex();

         step++;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_SaveStepEnd:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         // Add it to the save file list
         TTransfarringSaveFilePtr newFile( new TransfarringSaveFile() );

         newFile->mSaveIndex = saveIndex;
         newFile->mSaveType = saveType;
         newFile->mStageIndex = info.stage_num;
         newFile->mDifficulty = info.difficulty;
         newFile->mClearCount = info.clear_count;
         newFile->mPlayTime = playtime;
         newFile->mSaveDate = data.mExtraData.mSaveDate;

         ConcatenatedSaveIndex concatIndex = GetConcatenatedSaveIndexFromPlatform(saveType, saveIndex);
         TTransfarringSaveFilePtr curFile = GetSaveFilePtr(saveType, concatIndex);
         if (curFile)
         {
            // Be sure to preserve the transfarring ID here
            long long tempTID[2];
            long long tempVersion[2];
            long long tempPSNAccount[2];
            memcpy(tempTID, curFile->transfarringID, TransfarringDefines::kHashLengthBytes);
            memcpy(tempVersion, curFile->cloudVersionHash, TransfarringDefines::kHashLengthBytes);
            memcpy(tempPSNAccount, curFile->psnAccountHash, TransfarringDefines::kHashLengthBytes);

            *curFile = *newFile;

            TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, newFile->mSaveIndex);
            if( tid.m_Type != kTT_Invalid )
            {
               memcpy(curFile->transfarringID, tempTID, TransfarringDefines::kHashLengthBytes);
            }
            else
            {
               memset(curFile->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
            }

            memcpy(curFile->cloudVersionHash, tempVersion, TransfarringDefines::kHashLengthBytes);
            memcpy(curFile->psnAccountHash, tempPSNAccount, TransfarringDefines::kHashLengthBytes);
         }
         else
         {
            // Fill in the transfarring ID
            TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, newFile->mSaveIndex);
            if( tid.m_Type != kTT_Invalid )
            {
               memcpy(newFile->transfarringID, tid.m_UniqueID, TransfarringDefines::kHashLengthBytes);
            }
            else
            {
               memset(newFile->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
            }

            memcpy(newFile->cloudVersionHash, tid.m_VersionID, TransfarringDefines::kHashLengthBytes);
            memcpy(newFile->psnAccountHash, tid.m_PSNAccountName, TransfarringDefines::kHashLengthBytes);
            
            // Add it to the list of saves so we can detect empty slots
            AddSaveFilePtr(newFile, saveType, newFile->mSaveIndex);
         }

         // Reset BP linkvars so they don't dangle
         memcpy( &bp_linkvars, &s_saved_bp_linkvars, sizeof(SBP_LinkVars) );
         memcpy( &sv_bp_linkvars, &s_saved_sv_bp_linkvars, sizeof(SBP_LinkVars) );

         SET_TRANSFARRING_ERROR_SUCCESS( retval );
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   default:
      BPE_ASSERT(step < kTMC_SaveStepEnd, "TransfarringMemoryCardFileList::LoadFile_GameThread step is out of bounds");
      step++;
      break;
   };

   if( retval.mHighLevelError != kTE_Processing )
   {
      CTransfarringManager::Instance()->AllowTransfarringIDUpdates();

      CTransfarringManager::Instance()->SetOverrideSaveDate( false, NULL );

      step = kTMC_SaveStepBegin;
   }

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::DeleteFile_GameThread(ESaveType saveType, int deleteIndex)
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call DeleteFile_GameThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

#ifdef BP_VITA
   enum
   {
      kTMC_DeleteStepBegin,
      kTMC_DeleteStepInfoSearchStart,
      kTMC_DeleteStepDelete,
      kTMC_DeleteStepEnd,
   };

   static int step = kTMC_DeleteStepBegin;

   int mcmanResult = 0;
   
   switch( step )
   {
   case kTMC_DeleteStepBegin:
      SetMemoryCardActorType(saveType);

      CTransfarringManager::Instance()->m_bNeedsRealDelete = true;

      GameEasySearchStart(0);

      step = kTMC_DeleteStepInfoSearchStart;
      break;
   case kTMC_DeleteStepInfoSearchStart:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         GetGameInfoOnlyRBGStart(0);
         step = kTMC_DeleteStepDelete;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_DeleteStepDelete:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         int res = DeleteGameStart(deleteIndex);
         if( res )
         {
            step = kTMC_DeleteStepEnd;
         }
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_DeleteStepEnd:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         RemoveSaveFile(saveType, deleteIndex);

         SET_TRANSFARRING_ERROR_SUCCESS( retval );
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   default:
      BPE_ASSERT(false, "TransfarringMemoryCardFileList::DeleteFile_GameThread step is out of bounds");
      break;
   };

   if( retval.mHighLevelError != kTE_Processing )
   {
      CTransfarringManager::Instance()->m_bNeedsRealDelete = false;

      step = kTMC_DeleteStepBegin;
   }

#else
   BPE_ASSERT(false, "Transfarring file deletion only valid on PS Vita");
#endif

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::DeleteNonGameFile_GameThread( ENonGameSaveType gameType, int deleteIndex )
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call DeleteNonGameFile_GameThread without a memory card actor" );

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

#ifdef BP_VITA
   enum
   {
      kTMC_DeleteStepBegin,
      kTMC_DeleteStepDelete,
      kTMC_DeleteStepEnd,
   };

   static int step = kTMC_DeleteStepBegin;

   int mcmanResult = 0;
   
   switch (step)
   {
   case kTMC_DeleteStepBegin:
      m_memcardActor->file_kind = MCMAN_FILE_KIND_PHOTO;

      CTransfarringManager::Instance()->m_bNeedsRealDelete = true;

      step = kTMC_DeleteStepDelete;
      break;
   case kTMC_DeleteStepDelete:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         DeletePhotoStart(deleteIndex);
         step = kTMC_DeleteStepEnd;
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   case kTMC_DeleteStepEnd:
      mcmanResult = MCManGetResult();
      if( mcmanResult == 1 )
      {
         SET_TRANSFARRING_ERROR_SUCCESS( retval );
      }
      else if( mcmanResult < 0 )
      {
         SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
      }
      break;
   default:
      BPE_ASSERT(false, "TransfarringMemoryCardFileList::DeleteFile_GameThread step is out of bounds");
      break;
   };

   if( retval.mHighLevelError != kTE_Processing )
   {
      CTransfarringManager::Instance()->m_bNeedsRealDelete = false;
      step = kTMC_DeleteStepBegin;
   }

#else
   BPE_ASSERT(false, "Photo deletion only valid on PS Vita");
#endif

   return retval;
}

int TransfarringMemoryCardFileList::WaitForMemoryCard_SubThread()
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call WaitForMemoryCard_SubThread without a memory card actor" );

   int result = 0;

   while (!result)
   {
      CTransfarringManager::Instance()->LockSaveLoadMutex();
      result = MCManGetResult();
      CTransfarringManager::Instance()->UnlockSaveLoadMutex();

#if BP_VITA
      sceKernelDelayThread(5 * 1000);
#elif BP_PS3
      sys_timer_usleep(5 * 1000);
#endif

   }
   
   return result;
}

void TransfarringMemoryCardFileList::SetMemoryCardActorType(ESaveType saveType)
{
   BPE_ASSERT( m_memcardActor != NULL, "Tried to call SetMemoryCardActorType without a memory card actor" );

   switch(saveType)
   {
   case kST_Game:
      m_memcardActor->file_kind = MCMAN_FILE_KIND_GAME;
      break;
   case kST_VR:
      m_memcardActor->file_kind = MCMAN_FILE_KIND_VR;
      break;
   case kST_SnakeTales:
      m_memcardActor->file_kind = MCMAN_FILE_KIND_SNAKE_TALES;
      break;
   default:
      BPE_ASSERT(0, "TransfarringVTA.cpp - was given a bad save file type");
      break;
   }
}

int TransfarringMemoryCardFileList::CreateSaveFileFromCodedString(const char* name, TransfarringSaveFile* file, ESaveType saveType)
{
   MCMAN_INFODATA info;
   DecodeDataName_Kind(NULL, name, &info, m_memcardActor->file_kind);

   // Can't set mSaveIndex or mSaveDate here, do it elsewhere where you have access to the data
   file->mSaveType = saveType;
   
   file->mStageIndex = info.stage_num;
   file->mDifficulty = info.difficulty;
   file->mPlayTime = info.mgs2_playtime;
   file->mClearCount = info.clear_count;

   return 1;
}

// Internal game-specific functions that aren't part of the interface
static void Transfarring_SaveGameInfo( MCMAN_INFODATA *info, const char* linkvar, STransfarringSaveDate & const saveDate )
{
   memset(info,0x00,sizeof(MCMAN_INFODATA));

   info->mgs2_playtime = (*( int *)( linkvar + 312 ));// GM_PlayTime
   info->stage_num = (*( short * )( linkvar + 5512 )); // GM_StageNum
   info->difficulty = (*( short * )( linkvar + 16 )); // GM_GameLevel
   info->clear_count = (*( short * )( linkvar + 0 )); // GM_GameClearCount
   info->clear_flag = (*( short * )( linkvar + 5514 )) & ~TITLE_MENU_OPTION_CLEAR_MASK; //GM_TitleMenuStatus

   if((*( short * )( linkvar + 2 )) == 0) // GM_TankerClearCount
   {	
      info->clear_flag &= ~TITLE_MENU_TANKER_CLEARD;
   }
   if( (*( short * )( linkvar + 4 )) == 0) // GM_PlantClearCount
   {
      info->clear_flag &=~ TITLE_MENU_PLANT_CLEARD;
   }

#if defined(BP_VITA) || defined(BP_PS3)
   //MGSTWO-3242 - Updated clear status after saving a transfarred file.
   if((!(GM_TitleMenuStatus & TITLE_MENU_TANKER_CLEARD) && (info->clear_flag & TITLE_MENU_TANKER_CLEARD))
      || (!(GM_TitleMenuStatus & TITLE_MENU_PLANT_CLEARD) && (info->clear_flag & TITLE_MENU_PLANT_CLEARD)))
   {
      gGameClearChangeDetected = 1;
   }
   GM_TitleMenuStatus |= info->clear_flag;
#endif

   info->save_date = (saveDate.mYear - 2000) << 9;
   info->save_date |= saveDate.mMonth << 5;
   info->save_date |= saveDate.mDay & 0x1F;
}

static void Transfarring_SaveVRInfo( MCMAN_VR_INFODATA* pInfo, const TRANSFARRING_GAME_DATA& data)
{
   float fAchieve;

   memcpy( pInfo->name, (( int *)( data.mSaveData.linkvar + 5460 )), MCMAN_VR_NAME_MAX ); // GM_MyName
   pInfo->vr_playtime = *( int *)( data.mSaveData.linkvar + 312 ); //GM_PlayTime

   fAchieve = Transfarring_CalcVRAchievementRatio((const char*)data.mSaveData.vr_score_data);
   pInfo->achieve_high = (int)fAchieve;
   pInfo->achieve_low = (int)(( fAchieve - (float)((int)fAchieve))  * 10.f );
}

static TransfarringFileList::TTransfarringSaveFilePtr Transfarring_InfodataToSaveFile( MCMAN_INFODATA* info, ESaveType saveType, int saveIndex )
{
   TransfarringFileList::TTransfarringSaveFilePtr file( new TransfarringSaveFile() );

   file->mSaveIndex = saveIndex;
   file->mSaveType = saveType;
   file->mStageIndex = info->stage_num;
   file->mDifficulty = info->difficulty;
   file->mPlayTime = info->mgs2_playtime;
   file->mClearCount = info->clear_count;

   file->mSaveDate = MakeSaveDateFromInfo( info, saveType, saveIndex );

   TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, file->mSaveIndex);
   if( tid.m_Type != kTT_Invalid )
   {
      memcpy(file->transfarringID, tid.m_UniqueID, TransfarringDefines::kHashLengthBytes);
   }
   else
   {
      memset(file->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
   }
   
   memcpy(file->cloudVersionHash, tid.m_VersionID, TransfarringDefines::kHashLengthBytes);
   memcpy(file->psnAccountHash, tid.m_PSNAccountName, TransfarringDefines::kHashLengthBytes);

   if( saveType == kST_VR )
   {
      MCMAN_VR_INFODATA vrinfo;
      DecodeVRInfo( &vrinfo, info );
      file->mPlayTime = vrinfo.vr_playtime;
   }

   return file;
}

STransfarringSaveDate MakeSaveDateFromInfo( MCMAN_INFODATA* info, ESaveType saveType, int saveIndex )
{
   STransfarringSaveDate saveDate;
   
   if( saveType != kST_VR && info->save_date != 0 )
   {
      saveDate.mYear = (info->save_date >> 9) + 2000;
      saveDate.mMonth = (info->save_date >> 5) & 0x0F;
      saveDate.mDay = info->save_date & 0x1F;
   }
   else
   {
      // Old file; use the OS modification date
      char* dateTime = (char*)MCManGetDateTime(saveIndex);
      saveDate.mYear = (int)(2000 + dateTime[0]);
      saveDate.mMonth = (int)dateTime[1];
      saveDate.mDay = (int)dateTime[2];
   }

   return saveDate;
}