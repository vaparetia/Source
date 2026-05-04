//----------------------------------------------------------------------------
// TransfarringMemoryCardFileList.h
//
// A list of files that can manipulate the save game system
//----------------------------------------------------------------------------

#include "TransfarringMemoryCardFileList.h"

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include "MGS_Common.h"

#if defined(BP_VITA)
#include "TransfarringVTA.h"
#endif

#if defined(BP_PS3)
#include "Transfarring_PS3.h"
#include <sys/sys_time.h>
#include <sys/timer.h>
#endif

#include "BP_SaveLoadMGS.h"

extern "C" {
#include "../../source/user/arai/mc/mc.h"
#include "../../source/user/arai/mc/mc_actor_ext.h"
#include "libcdvd.h"
#include "../../source/user/subsistence/ps2/mg_mc_gui.h"
extern int BP_Frames60ToFramesSys( int frames60 );
}

extern int skMaxSaveNum[kST_MaxSaveTypes];

static int fakeIconData = 0;

static void MC_Transfarring_MakeGameFileName( char *name, int num, int crc, int mgs3like, char* linkvarBuf, STransfarringSaveDate saveDate );

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
   return kST_Game;
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
   BPE_ASSERT(saveType == kST_Game, "TransfarringMemoryCardFileList::LoadFileList_SubThread - Bad save type");

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
   
   CTransfarringManager::Instance()->LockSaveLoadMutex();
   MC_FILE mc_file[32];
   sprintf(mc_file[0].name, "%s*", GetGFilename()); // product code
   sprintf(mc_file[1].name, "%s", MC_FILE_SEARCH); // wildcard
   int mc_file_num = skMaxSaveNum[saveType];
   McActorGetFile(0, mc_file, mc_file_num);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int mcActorResult = WaitForMemoryCard_SubThread();
   if( mcActorResult != MC_OK )
   {
      return retval;
   }

   CTransfarringManager::Instance()->EndProgressPhase();

   mc_file_num = McActorGetDirNum();

   for (int i=0; i < mc_file_num; i++)
   {
      TTransfarringSaveFilePtr file( new TransfarringSaveFile() );

      int isValid = CreateSaveFileFromCodedString(mc_file[i].name, file.get(), saveType );

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

      if( !isValid )
      {
         // BP code from mc_gui.c
         // Scan the directory for the save index
         sscanf(&mc_file[i].bp_dirName[14], "%x", &file->mSaveIndex); 

         SetFileCorrupt_Platform( true, saveType, file->mSaveIndex );
      }

      AddSaveFilePtr(file, saveType, file->mSaveIndex);
   }

   SET_TRANSFARRING_ERROR_SUCCESS( retval );

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::SaveFile_SubThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int saveIndex)
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );

   // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
   CTransfarringManager::Instance()->LockTransfarringIDUpdates();

   int crc = 0;
   MC_CalcCRC(&data.mSaveData, SAVE_DATA_SIZE, &crc);

   MC_FILE mc_file[32];

   MC_MakeGameDirName(mc_file[0].name, saveIndex);

   //TODO: global.doyoulikemgs3 Normally stored in GCL
   MC_Transfarring_MakeGameFileName(mc_file[1].name, saveIndex, crc, true, data.mSaveData.gcl_linkvar, data.mExtraData.mSaveDate );

   MC_DumpGame(mc_file[2].name);

   // Add it to the save file list
   TTransfarringSaveFilePtr newFile( new TransfarringSaveFile() );

   // Build the info data from the coded name
   CreateSaveFileFromCodedString(mc_file[1].name, newFile.get(), saveType);

   // Set the save index to the where we're saving it
   newFile->mSaveIndex = saveIndex;

   ConcatenatedSaveIndex concatIndex = GetConcatenatedSaveIndexFromPlatform(saveType, saveIndex);
   TTransfarringSaveFilePtr curFile = GetSaveFilePtr(saveType, concatIndex);
   if (curFile)
   {
      TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);

      // Be sure to preserve the transfarring ID here
      long long tempTID[2];
      long long tempVersion[2];
      long long tempPSNAccount[2];
      memcpy(tempTID, curFile->transfarringID, TransfarringDefines::kHashLengthBytes);
      memcpy(tempVersion, curFile->cloudVersionHash, TransfarringDefines::kHashLengthBytes);
      memcpy(tempPSNAccount, curFile->psnAccountHash, TransfarringDefines::kHashLengthBytes);

      *curFile = *newFile;

      if( tid.m_Type != kTT_Invalid )
      {
         memcpy(curFile->transfarringID, tempTID, TransfarringDefines::kHashLengthBytes);
      }
      else
      {
         memset(curFile->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
      }

      memcpy(curFile->transfarringID, tempTID, TransfarringDefines::kHashLengthBytes);
      memcpy(curFile->cloudVersionHash, tempVersion, TransfarringDefines::kHashLengthBytes);
      memcpy(curFile->psnAccountHash, tempPSNAccount, TransfarringDefines::kHashLengthBytes);
   }
   else
   {
      // Copy TransfarringID into save file
      TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);

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
      AddSaveFilePtr(newFile, saveType, saveIndex);
   }

   MC_EncodeData(&data.mSaveData, SAVE_DATA_SIZE);

   mc_file[1].data = &data.mSaveData;
   mc_file[1].size = sizeof(_GAME_SAVE_DATA);

   MC_MakeGameDirName(mc_file[2].name, saveIndex);
   mc_file[2].data = mc_file[1].name;
   mc_file[2].size = strlen(mc_file[1].name);

   MC_MakeIconName(mc_file[3].name, 0);
   mc_file[3].data = &fakeIconData;//icon_data;
   mc_file[3].size = sizeof(fakeIconData);//icon_size;

   char title[MC_TITLE_BUF_LEN];
   strcpy(title, MC_GAME_TITLE_SJIS_1 MC_GAME_TITLE_SJIS_2);
   sceMcIconSys *iconsys = MC_GetIconSysGame();

   MC_ReplaceNumSJIS(title, saveIndex);
   strcpy((char*)iconsys->TitleName, title);
   MC_CheckTitle((char*)iconsys->TitleName, iconsys->OffsLF);

   // icon.sys
   MC_MakeIconSysName(mc_file[4].name);
   mc_file[4].data = iconsys;
   mc_file[4].size = sizeof(sceMcIconSys);

   strcpy(mc_file[5].name, "");

   // Save BP linkvars
   SBP_LinkVars saved_bp_linkvars;
   SBP_LinkVars saved_sv_bp_linkvars;

   memcpy( &saved_bp_linkvars, &bp_linkvars, sizeof(SBP_LinkVars) );
   memcpy( &saved_sv_bp_linkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars) );

   memcpy(&bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));
   memcpy(&sv_bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));

   CTransfarringManager::Instance()->LockSaveLoadMutex();

   McActorSave(0, mc_file);

   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int mcactorResult = WaitForMemoryCard_SubThread();
   if( mcactorResult != MC_OK )
   {
      CTransfarringManager::Instance()->AllowTransfarringIDUpdates();
      return retval;
   }

   CTransfarringManager::Instance()->EndProgressPhase();

   CTransfarringManager::Instance()->AllowTransfarringIDUpdates();

   // Restore BP linkvars
   memcpy( &bp_linkvars, &saved_bp_linkvars, sizeof(SBP_LinkVars) );
   memcpy( &sv_bp_linkvars, &saved_sv_bp_linkvars, sizeof(SBP_LinkVars) );

   SET_TRANSFARRING_ERROR_SUCCESS( retval );
   return retval;
}

STransfarringError TransfarringMemoryCardFileList::LoadFile_SubThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int loadIndex)
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );

   // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
   CTransfarringManager::Instance()->LockTransfarringIDUpdates();

   MC_FILE mc_file[32];

   // Save BP linkvars
   SBP_LinkVars saved_bp_linkvars;
   SBP_LinkVars saved_sv_bp_linkvars;

   memcpy( &saved_bp_linkvars, &bp_linkvars, sizeof(SBP_LinkVars) );
   memcpy( &saved_sv_bp_linkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars) );

   CTransfarringManager::Instance()->LockSaveLoadMutex();

   MC_MakeGameDirName(mc_file[0].name, loadIndex);
   strcpy(mc_file[1].name, MC_FILE_SEARCH);
   mc_file[1].data = &data.mSaveData;

   McActorLoad(0, mc_file);

   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int mcActorResult = WaitForMemoryCard_SubThread();
   if( mcActorResult != MC_OK )
   {
      CTransfarringManager::Instance()->AllowTransfarringIDUpdates();
      return retval;
   }

   if( MGS_SaveStatus_WrongUser() )
   {
      // Reset BP linkvars and clear wrong user flag
      memset( &sv_bp_linkvars, 0, sizeof(sv_bp_linkvars) );

      MGS_SaveStatus_ClearWrongUser();

      CTransfarringManager::Instance()->AllowTransfarringIDUpdates();

      SET_TRANSFARRING_ERROR( retval, kTE_OwnershipError, 0 );

      return retval;
   }

   CTransfarringManager::Instance()->EndProgressPhase();

   MC_DecodeData(&data.mSaveData, SAVE_DATA_SIZE);

   // Copy BP linkvars that were just loaded
   memcpy(&data.mBPLinkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars));

   // Restore BP linkvars
   memcpy( &bp_linkvars, &saved_bp_linkvars, sizeof(SBP_LinkVars) );
   memcpy( &sv_bp_linkvars, &saved_sv_bp_linkvars, sizeof(SBP_LinkVars) );

   TTransfarringSaveFilePtr curFile = GetSaveFilePtrPlatform( saveType, loadIndex );

   // Update anything that can be changed at game save time
   TransfarringSaveFile tempFile;
   int isValid = CreateSaveFileFromCodedString(mc_file[1].name, &tempFile, saveType );

   curFile->mClearCount = tempFile.mClearCount;
   curFile->mStageIndex = tempFile.mStageIndex;
   curFile->mPlayTime = tempFile.mPlayTime;
   curFile->mSaveDate = tempFile.mSaveDate;

   // Copy date into extra data
   data.mExtraData.mSaveDate = curFile->mSaveDate;

   // Gigantic hack: There's a legacy bug where the current playtime is saved into the info data, 
   // while the last checkpoint's playtime is saved into the save data.
   // So you can hit a check point, wait 20 minutes, save, close the game, load the file, and save again,
   // which results in your play time being 20 minutes less than it was before.
   // This becomes glaringly obvious in the transfarring menu when you have two files right next to each other with different playtimes
   // So we're just going to slam the playtime to be whatever the infodata said it was.
   *((int*)(data.mSaveData.gcl_linkvar + 76)) = (int)curFile->mPlayTime;

   CTransfarringManager::Instance()->AllowTransfarringIDUpdates();

   SET_TRANSFARRING_ERROR_SUCCESS( retval );
   return retval;
}

STransfarringError TransfarringMemoryCardFileList::DeleteFile_SubThread(ESaveType saveType, int deleteIndex)
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );

#if BP_VITA
   CTransfarringManager::Instance()->m_bNeedsRealDelete = true;

   CTransfarringManager::Instance()->LockSaveLoadMutex();
   MC_FILE mc_file[32];
   MC_MakeGameDirName(mc_file[0].name, deleteIndex);
   strcpy(mc_file[1].name, MC_FILE_SEARCH);
   McActorDelete(0, mc_file);
   CTransfarringManager::Instance()->UnlockSaveLoadMutex();

   int mcActorResult = WaitForMemoryCard_SubThread();
   if( mcActorResult != MC_OK )
   {
      return retval;
   }

   CTransfarringManager::Instance()->EndProgressPhase();

   RemoveSaveFile(saveType, deleteIndex);

   CTransfarringManager::Instance()->m_bNeedsRealDelete = false;

   SET_TRANSFARRING_ERROR_SUCCESS( retval );
#else
   BPE_ASSERT(false, "Transfarring file deletion only valid on PS Vita");
#endif

   return retval;
}

int TransfarringMemoryCardFileList::WaitForMemoryCard_SubThread()
{
   while (McActorIsBusy()) 
   {
      int progress = (int)McActorGetProgress();
      CTransfarringManager::Instance()->SetProgress(progress);

#if BP_VITA
      sceKernelDelayThread(5 * 1000);
#elif BP_PS3
      sys_timer_usleep(5 * 1000);
#endif
   }

   return McActorGetResult();
}

void TransfarringMemoryCardFileList::SetMemoryCardActorType(ESaveType saveType)
{
   // Not needed on MGS3
}

STransfarringError TransfarringMemoryCardFileList::LoadAllFileLists_GameThread()
{
   enum
   {
      kTMC_LoadFileListBegin = 0,
      kTMC_LoadFileListEnd = 5
   };

   static int step = kTMC_LoadFileListBegin;
   static MC_FILE mc_file[32];

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

   switch (step)
   {
   case kTMC_LoadFileListBegin:
      sprintf(mc_file[0].name, "%s*", GetGFilename()); // product code
      sprintf(mc_file[1].name, "%s", MC_FILE_SEARCH); // wildcard
      McActorGetFile(0, mc_file, skMaxSaveNum[kST_Game]);
      step = kTMC_LoadFileListEnd;

      break;
   case kTMC_LoadFileListEnd:
      {
         if (!McActorIsBusy())
         {
            CTransfarringManager::Instance()->EndProgressPhase();

            int mcActorResult = McActorGetResult();
            if( mcActorResult != MC_OK )
            {
               SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
               break;
            }

            int mc_file_num = McActorGetDirNum();
            for (int i=0; i < mc_file_num; i++)
            {
               TTransfarringSaveFilePtr file( new TransfarringSaveFile() );

               int isValid = CreateSaveFileFromCodedString(mc_file[i].name, file.get(), kST_Game);

               TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(kST_Game, file->mSaveIndex);
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

               if( !isValid )
               {
                  // BP code from mc_gui.c
                  // Scan the directory for the save index
                  sscanf(&mc_file[i].bp_dirName[14], "%x", &file->mSaveIndex); 

                  SetFileCorrupt_Platform( true, kST_Game, file->mSaveIndex );
               }

               AddSaveFilePtr(file, kST_Game, file->mSaveIndex);
            }

            SET_TRANSFARRING_ERROR_SUCCESS( retval );
         }
      }
      break;
   default:
      BPE_ASSERT(step < kTMC_LoadFileListEnd, "TransfarringMemoryCardFileList::LoadAllFileLists_GameThread step is out of bounds");
      step++;
      break;
   }

   if( retval.mHighLevelError != kTE_Processing )
   {
      step = kTMC_LoadFileListBegin;
   }

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::DeleteFile_GameThread(ESaveType saveType, int deleteIndex)
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

#ifdef BP_VITA
   enum
   {
      kTMC_DeleteStepBegin,
      kTMC_DeleteStepEnd,
   };

   static int step = kTMC_DeleteStepBegin;
   static TRANSFARRING_GAME_DATA sLoadData;
   
   MC_FILE mc_file[32];

   switch (step)
   {
   case kTMC_DeleteStepBegin:
      
      CTransfarringManager::Instance()->m_bNeedsRealDelete = true;

      MC_MakeGameDirName(mc_file[0].name, deleteIndex);
      strcpy(mc_file[1].name, MC_FILE_SEARCH);
      McActorDelete(0, mc_file);

      step = kTMC_DeleteStepEnd;
      break;
   case kTMC_DeleteStepEnd:
      if (!McActorIsBusy())
      {
         int mcActorResult = McActorGetResult();
         if( mcActorResult != MC_OK )
         {
            SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
            break;
         }

         RemoveSaveFile(saveType, deleteIndex);

         SET_TRANSFARRING_ERROR_SUCCESS( retval );
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
   BPE_VERIFY(false, false, "Transfarring file deletion only valid on PS Vita");
#endif

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::DeleteNonGameFile_GameThread(ENonGameSaveType gameType, int deleteIndex)
{
   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

#ifdef BP_VITA
   enum
   {
      kTMC_DeleteStepBegin,
      kTMC_DeleteStepEnd,
   };

   static int step = kTMC_DeleteStepBegin;

   MC_FILE mc_file[32];

   bool retVal = false;
   switch (step)
   {
   case kTMC_DeleteStepBegin:

      CTransfarringManager::Instance()->m_bNeedsRealDelete = true;

      switch( gameType )
      {
      case kNGST_Photo:
         MC_MakePhotoDirName(mc_file[0].name, deleteIndex);
         break;
      case kNGST_MG1:
         MC_MakeMG1DirName(mc_file[0].name, deleteIndex);
         break;
      case kNGST_MG2:
         MC_MakeMG2DirName(mc_file[0].name, deleteIndex);
         break;
      default:
         BPE_ASSERT( false , "DeleteNonGameFile_GameThread was given a bad gameType" );
         break;
      }

      strcpy(mc_file[1].name, MC_FILE_SEARCH);
      McActorDelete(0, mc_file);

      step = kTMC_DeleteStepEnd;
      break;
   case kTMC_DeleteStepEnd:
      if (!McActorIsBusy())
      {
         int mcActorResult = McActorGetResult();
         if( mcActorResult != MC_OK )
         {
            SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
            break;
         }

         SET_TRANSFARRING_ERROR_SUCCESS( retval );
      }
      break;
   default:
      BPE_ASSERT(false, "TransfarringMemoryCardFileList::DeleteFile_GameThread step is out of bounds");
      break;
   };

   if( retval.mHighLevelError != kTE_Processing )
   {
      step = kTMC_DeleteStepBegin;
      CTransfarringManager::Instance()->m_bNeedsRealDelete = false;
   }

#else
   BPE_VERIFY(false, false, "Transfarring file deletion only valid on PS Vita");
#endif

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::LoadFile_GameThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int loadIndex)
{
   enum
   {
      kTMC_LoadStepBegin = 0,
      kTMC_LoadStepLoadGame = 1,
      kTMC_LoadStepEnd = 5
   };

   static int step = kTMC_LoadStepBegin;
   static MC_FILE mc_file[32];

   static SBP_LinkVars s_saved_bp_linkvars;
   static SBP_LinkVars s_saved_sv_bp_linkvars;

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

   switch (step)
   {
   case kTMC_LoadStepBegin:
      // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
      CTransfarringManager::Instance()->LockTransfarringIDUpdates();

      SetMemoryCardActorType(saveType);

      memset(mc_file, 0, sizeof(mc_file));

      MC_MakeGameDirName(mc_file[0].name, loadIndex);
      strcpy(mc_file[1].name, MC_FILE_SEARCH);
      mc_file[1].data = &data.mSaveData;

      memcpy( &s_saved_bp_linkvars, &bp_linkvars, sizeof(SBP_LinkVars) );
      memcpy( &s_saved_sv_bp_linkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars) );

      step++;
      break;
   case kTMC_LoadStepLoadGame:
      McActorLoad(0, mc_file);
      step++;
      break;
   case kTMC_LoadStepEnd:
      if (McActorIsBusy())
      {
         int progress = (int)McActorGetProgress();
         CTransfarringManager::Instance()->SetProgress(progress);
      }
      else
      {
         CTransfarringManager::Instance()->EndProgressPhase();

         int mcActorResult = McActorGetResult();
         if( mcActorResult != MC_OK )
         {
            SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
            break;
         }

         if( MGS_SaveStatus_WrongUser() )
         {
            // Reset BP linkvars and clear wrong user flag
            memset( &sv_bp_linkvars, 0, sizeof(sv_bp_linkvars) );

            MGS_SaveStatus_ClearWrongUser();

            SET_TRANSFARRING_ERROR( retval, kTE_OwnershipError, 0 );
            break;
         }

         MC_DecodeData(&data.mSaveData, SAVE_DATA_SIZE);

         // Copy BP linkvars that were just loaded
         memcpy(&data.mBPLinkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars));

         // Restore BP linkvars
         memcpy( &bp_linkvars, &s_saved_bp_linkvars, sizeof(SBP_LinkVars) );
         memcpy( &sv_bp_linkvars, &s_saved_sv_bp_linkvars, sizeof(SBP_LinkVars) );

         TTransfarringSaveFilePtr curFile = GetSaveFilePtrPlatform( saveType, loadIndex );

         // Update anything that can be changed at game save time
         TransfarringSaveFile tempFile;
         int isValid = CreateSaveFileFromCodedString(mc_file[1].name, &tempFile, saveType);

         if( curFile.get() )
         {
            curFile->mClearCount = tempFile.mClearCount;
            curFile->mStageIndex = tempFile.mStageIndex;
            curFile->mPlayTime = tempFile.mPlayTime;
            curFile->mSaveDate = tempFile.mSaveDate;
         }
         else
         {
            curFile.reset( new TransfarringSaveFile() );
            *curFile = tempFile;
            AddSaveFilePtr( curFile, saveType, loadIndex );
         }

         // Copy date into extra data
         data.mExtraData.mSaveDate = curFile->mSaveDate;

         // Gigantic hack: There's a legacy bug where the current playtime is saved into the info data, 
         // while the last checkpoint's playtime is saved into the save data.
         // So you can hit a check point, wait 20 minutes, save, close the game, load the file, and save again,
         // which results in your play time being 20 minutes less than it was before.
         // This becomes glaringly obvious in the transfarring menu when you have two files right next to each other with different playtimes
         // So we're just going to slam the playtime to be whatever the infodata said it was.
         *((int*)(data.mSaveData.gcl_linkvar + 76)) = (int)curFile->mPlayTime;

         SET_TRANSFARRING_ERROR_SUCCESS( retval );
      }
      break;
   default:
      BPE_ASSERT(step < kTMC_LoadStepEnd, "TransfarringMemoryCardFileList::LoadFile_GameThread step is out of bounds");
      step++;
      break;
   };

   if( retval.mHighLevelError != kTE_Processing )
   {
      step = kTMC_LoadStepBegin;
      CTransfarringManager::Instance()->AllowTransfarringIDUpdates();
   }

   return retval;
}

STransfarringError TransfarringMemoryCardFileList::SaveFile_GameThread(TRANSFARRING_GAME_DATA& data, ESaveType saveType, int saveIndex)
{
   enum
   {
      kTMC_SaveStepBegin = 0,
      kTMC_SaveStepStartSave = 1,
      kTMC_SaveStepEnd = 5
   };

   static int step = kTMC_SaveStepBegin;
   static MC_FILE mc_file[32];
   static int crc = 0;

   ConcatenatedSaveIndex concatIndex = -1;
   TTransfarringSaveFilePtr curFile;
   TTransfarringSaveFilePtr newFile;
   sceMcIconSys *iconsys = NULL;

   static SBP_LinkVars s_saved_bp_linkvars;
   static SBP_LinkVars s_saved_sv_bp_linkvars;

   STransfarringError retval;
   SET_TRANSFARRING_ERROR( retval, kTE_Processing, 0 );

   switch (step)
   {
   case kTMC_SaveStepBegin:
      // Don't update the transfarring IDs when we're loading or saving via the Transfarring manager
      CTransfarringManager::Instance()->LockTransfarringIDUpdates();

      SetMemoryCardActorType(saveType);

      step++;
      break;
   case kTMC_SaveStepStartSave:
      MC_CalcCRC(&data.mSaveData, SAVE_DATA_SIZE, &crc);

      MC_MakeGameDirName(mc_file[0].name, saveIndex);

      //TODO: global.doyoulikemgs3 Normally stored in GCL
      MC_Transfarring_MakeGameFileName(mc_file[1].name, saveIndex, crc, true, data.mSaveData.gcl_linkvar, data.mExtraData.mSaveDate );

      MC_DumpGame(mc_file[2].name);

      // Add it to the save file list
      newFile.reset( new TransfarringSaveFile() );

      // Build the info data from the coded name
      CreateSaveFileFromCodedString(mc_file[1].name, newFile.get(), saveType);

      // Set the save index to the where we're saving it
      newFile->mSaveIndex = saveIndex;

      concatIndex = GetConcatenatedSaveIndexFromPlatform(saveType, saveIndex);
      curFile = GetSaveFilePtr(saveType, concatIndex);
      if (curFile)
      {
         TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);

         // Be sure to preserve the transfarring ID here
         long long tempTID[2];
         long long tempVersion[2];
         long long tempPSNAccount[2];
         memcpy(tempTID, curFile->transfarringID, TransfarringDefines::kHashLengthBytes);
         memcpy(tempVersion, curFile->cloudVersionHash, TransfarringDefines::kHashLengthBytes);
         memcpy(tempPSNAccount, curFile->psnAccountHash, TransfarringDefines::kHashLengthBytes);

         *curFile = *newFile;

         if( tid.m_Type != kTT_Invalid )
         {
            memcpy(curFile->transfarringID, tempTID, TransfarringDefines::kHashLengthBytes);
         }
         else
         {
            memset(curFile->transfarringID, 0, TransfarringDefines::kHashLengthBytes);
         }

         memcpy(curFile->transfarringID, tempTID, TransfarringDefines::kHashLengthBytes);
         memcpy(curFile->cloudVersionHash, tempVersion, TransfarringDefines::kHashLengthBytes);
         memcpy(curFile->psnAccountHash, tempPSNAccount, TransfarringDefines::kHashLengthBytes);
      }
      else
      {
         // Copy TransfarringID into save file
         TransfarringID tid = CTransfarringManager::Instance()->GetTransfarringID(saveType, saveIndex);

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
         AddSaveFilePtr(newFile, saveType, saveIndex);
      }

      MC_EncodeData(&data.mSaveData, SAVE_DATA_SIZE);

      mc_file[1].data = &data.mSaveData;
      mc_file[1].size = sizeof(_GAME_SAVE_DATA);

      MC_MakeGameDirName(mc_file[2].name, saveIndex);
      mc_file[2].data = mc_file[1].name;
      mc_file[2].size = strlen(mc_file[1].name);

      MC_MakeIconName(mc_file[3].name, 0);
      mc_file[3].data = &fakeIconData;//icon_data;
      mc_file[3].size = sizeof(fakeIconData);//icon_size;

      char title[MC_TITLE_BUF_LEN];
      strcpy(title, MC_GAME_TITLE_SJIS_1 MC_GAME_TITLE_SJIS_2);
      iconsys = MC_GetIconSysGame();

      MC_ReplaceNumSJIS(title, saveIndex);
      strcpy((char*)iconsys->TitleName, title);
      MC_CheckTitle((char*)iconsys->TitleName, iconsys->OffsLF);

      // icon.sys
      MC_MakeIconSysName(mc_file[4].name);
      mc_file[4].data = iconsys;
      mc_file[4].size = sizeof(sceMcIconSys);

      strcpy(mc_file[5].name, "");

      // Save BP linkvars
      memcpy( &s_saved_bp_linkvars, &bp_linkvars, sizeof(SBP_LinkVars) );
      memcpy( &s_saved_sv_bp_linkvars, &sv_bp_linkvars, sizeof(SBP_LinkVars) );

      memcpy(&bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));
      memcpy(&sv_bp_linkvars, &data.mBPLinkvars, sizeof(SBP_LinkVars));

      McActorSave(0, mc_file);

      step++;
      break;
   case kTMC_SaveStepEnd:
      if (McActorIsBusy())
      {
         int progress = (int)McActorGetProgress();
         CTransfarringManager::Instance()->SetProgress(progress);
      }
      else
      {
         CTransfarringManager::Instance()->EndProgressPhase();

         int res = McActorGetResult();
         if( res != MC_OK )
         {
            SET_TRANSFARRING_ERROR( retval, kTE_StorageMediaError, 0 );
            break;
         }

         // Restore BP linkvars
         memcpy( &bp_linkvars, &s_saved_bp_linkvars, sizeof(SBP_LinkVars) );
         memcpy( &sv_bp_linkvars, &s_saved_sv_bp_linkvars, sizeof(SBP_LinkVars) );

         SET_TRANSFARRING_ERROR_SUCCESS( retval );
      }
      break;
   default:
      BPE_ASSERT(step < kTMC_SaveStepEnd, "TransfarringMemoryCardFileList::LoadFile_GameThread step is out of bounds");
      step++;
      break;
   };

   if( retval.mHighLevelError != kTE_Processing )
   {
      step = kTMC_SaveStepBegin;
      CTransfarringManager::Instance()->AllowTransfarringIDUpdates();
   }

   return retval;
}

int TransfarringMemoryCardFileList::CreateSaveFileFromCodedString(const char* name, TransfarringSaveFile* file, ESaveType saveType)
{
   MC_LIST_GAME info;
   int retval = MC_DecodeDataName_Game(name, &info);

   if( retval != 0 )
   {
      file->mSaveType = saveType;
      
      file->mSaveIndex = info.num;
      file->mStageIndex = info.area;
      file->mDifficulty = info.level;
      file->mPlayTime = info.mgs3_play_time;
      file->mClearCount = info.count;

      int ymd = info.ymd;
      MC_Clock2YMD(&ymd, &file->mSaveDate.mYear, &file->mSaveDate.mMonth, &file->mSaveDate.mDay);
   }

   return retval;
}

// Internal game-specific functions
static void MC_Transfarring_MakeGameFileName( char *name, int num, int crc, int mgs3like, char* linkvarBuf, STransfarringSaveDate saveDate )
{
   MC_LIST_GAME list = {0};
   ASSERT( name );

   list.num = num;

   // All this stuff is used for the ape minigame and therefore isn't used
   list.ape_mask = 0;//(GM_IV_GetItemNum(IT_MonkeyMask) > 0 ? 1 : 0);
   list.banana_camouf = 0;//(GM_IV_GetItemNum(IT_EqBanana) > 0 ? 1 : 0);
   list.green = 0;//(GM_IV_GetItemNum(IT_EqFpGreen) > 0 ? 1 : 0);
   list.dpm = 0;//(GM_IV_GetItemNum(IT_EqDpm) > 0 ? 1 : 0);

   list.level = *( short * )( linkvarBuf + 6 );//GM_GameLevel;

   //BP note: this actually causes inconsistency in display because it doesn't match what's being saved as the actual play time,
   //in the savedata body, which is the time of the last checkpoint. (legacy issue)
   list.mgs3_play_time = BP_Frames60ToFramesSys( *((int*)(linkvarBuf + 76))); //GM_PlayTime
   list.crc = crc;
   list.area = *((int*)(linkvarBuf + 16));//GM_SaveArea
   short clearCount = *( short * )( linkvarBuf + 0 );
   list.count = ( clearCount > 31 ? 31 : clearCount );
   list.mgs3like = ( mgs3like ? 1 : 0 );
   list.ape = *( char *)( linkvarBuf + 12662 );//SARU_StageOpenFlag

   int ymd;
   MC_YMD2Clock( &ymd, &saveDate.mYear, &saveDate.mMonth, &saveDate.mDay );
   list.ymd = ymd;

   MC_MakeDataName_Game( name, &list, &crc );
}