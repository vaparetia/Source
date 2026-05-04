//------------------------------------------------------------------------------------------
// BP_SaveLoadVTA.cpp
//
// Utility functions for saving/loading on Vita.
//------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <queue>

#include "BP_SaveLoad.h"

#if MGS_VERSION == 2
#include "mgs_type.h"
#elif MGS_VERSION == 3
#include "sys_common.h"
#endif

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/VTAThreadPriorities.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/libkernel.h>
#include <kernel/iofilemgr.h>
#include <sdk_version.h>
#include <apputil.h>
#include <savedata_dialog.h>

#include "Engine/Stdafx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "TransfarringVTA.h"
#include "TransfarringVTACGlue.h"

// We only need to use one save slot for saving data. The save slots are used by the save dialog which 
// we do not rely on for saving games.
static SceAppUtilSaveDataSlotParam gs_SaveGameSlotParam;
const int kSaveGameSlotId = 0;

#if MGS_VERSION==2
static char const *skSGS_Title_Download = "METAL GEAR SOLID 2 HD EDITION";
#elif MGS_VERSION==3
static char const *skSGS_Title_Download = "METAL GEAR SOLID 3 HD EDITION";
#endif

static char const *skSGS_Title_Bundle_NoJP = "METAL GEAR SOLID HD COLLECTION";
static char const *skSGS_Title_Bundle_JP = "METAL GEAR SOLID HD EDITION";

static char const skSGS_Details_E[] = "Save data";
static char const skSGS_Details_F[] = "Sauvegarde";
static char const skSGS_Details_I[] = "Dati salvati";
static char const skSGS_Details_G[] = "Speicherdaten";
static char const skSGS_Details_S[] = "Datos guardados";
static char const skSGS_Details_J[] = "セーブデータ";

static char const skSGS_Unknown[] = "Unknown";

#define CUSTOM_ERROR_FILE_NOT_FOUND 10
#define CUSTOM_ERROR_SIZE_MISMATCH 11

//----------------------------------------------------------------------------
static char const * const internal_get_save_game_localized_string(ESaveGameString const string)
{
   switch (string)
   {
   case kSGS_Title:
      if ( gpOsContext->IsDownloadableVersion() )
      {
         return skSGS_Title_Download;
      }
      else
      {
         if ( gpOsContext->mBuildSKU == COsContext::kBS_Japan )
         {
            return skSGS_Title_Bundle_JP;
         }
         else
         {
            return skSGS_Title_Bundle_NoJP;
         }
      }
      break;

   case kSGS_Subtitle:
      switch (gpOsContext->mLanguage)
      {
      case CBaseOsContext::kL_Japanese:
         return skSGS_Details_J;

      case CBaseOsContext::kL_French:
         return skSGS_Details_F;

      case CBaseOsContext::kL_Italian:
         return skSGS_Details_I;

      case CBaseOsContext::kL_German:
         return skSGS_Details_G;

      case CBaseOsContext::kL_Spanish:
         return skSGS_Details_S;

      default: // Default to English
         return skSGS_Details_E;
      }
   }
   return skSGS_Unknown;
}

//----------------------------------------------------------------------------
static void internal_CMGS_SaveStatus_setResult(CMGS_SaveStatus *pSS, int saveRet)
{
   pSS->mCELL_Result = saveRet;
   if (pSS->mCELL_Result == 0)
   {
      // All good
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;
      //**** Hack to test corruption detection
      //      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
   }
   else 
   {
      // Problem!
      if (pSS->mCELL_Result == SCE_APPUTIL_ERROR_SAVEDATA_NO_SPACE_FS || pSS->mCELL_Result == SCE_APPUTIL_ERROR_SAVEDATA_NO_SPACE_QUOTA )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoSpace;
      }
      else if( pSS->mCELL_Result == CUSTOM_ERROR_FILE_NOT_FOUND )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
      }
      else if( pSS->mCELL_Result == CUSTOM_ERROR_SIZE_MISMATCH )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;;
      }
      else if( pSS->mCELL_Result == -1 )
      {
         // No data; how the old system notified that this is a new save
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoData;
      }
      else
      {
         // Error that's not triggered by us, reset our result to indicate this.
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_CELL_Error;
      }
   }
}

//----------------------------------------------------------------------------
static SceInt32 internal_do_delete(CMGS_SaveStatus* pSS)
{
   SceInt32 res = SCE_OK;

   // In order to delete save data we must go through the apputil library.
   SceAppUtilSaveDataDataSlot saveSlot;
   memset(&saveSlot, 0, sizeof(SceAppUtilSaveDataDataSlot));
   saveSlot.id = kSaveGameSlotId;

   char filePath[SCE_APPUTIL_SAVEDATA_DATA_PATH_MAXSIZE];
   snprintf(filePath, SCE_APPUTIL_SAVEDATA_DATA_PATH_MAXSIZE, "%s/%s", pSS->mSaveDirectoryRoot, pSS->mSaveDirectory);

   SceAppUtilSaveDataDataRemoveItem toRemove;
   memset(&toRemove, 0, sizeof(SceAppUtilSaveDataDataRemoveItem));

   toRemove.dataPath = (const SceChar8*)&filePath[0];
   toRemove.mode = SCE_APPUTIL_SAVEDATA_DATA_REMOVE_MODE_DEFAULT;

   res = sceAppUtilSaveDataDataRemove(&saveSlot, &toRemove, 1, NULL);
   BPE_VERIFY(res == SCE_OK, false, "Error removing existing save data.");

   return res;
}

//----------------------------------------------------------------------------
static SceInt32 internal_thread_delete_save(SceSize argByteSize, void *args)
{
   SceInt32 res = SCE_OK;
   CMGS_SaveStatus* pSS = *(CMGS_SaveStatus**)args;

   internal_do_delete(pSS);

   pSS->mStatus = CMGS_SaveStatus::kS_Done;
   OsContext()->ClearUnsafeToShutDownFlag(CBaseOsContext::kUSDF_SaveData);

   res = sceKernelExitDeleteThread(0);
   return res;
}

//----------------------------------------------------------------------------
static SceInt32 internal_do_save(CMGS_SaveStatus* pSS)
{
   SceInt32 returnResult = SCE_OK;

   for (int i = 0; i < pSS->GetNumMCFiles(); i++)
   {
      SceInt32 res = SCE_OK;

      if (!pSS->ShouldWriteFile(i))
      {
         continue;
      }
      else
      {
         const SMCFile* const pFile = pSS->mpMCFiles + i;
         char filePath[SCE_APPUTIL_SAVEDATA_DATA_PATH_MAXSIZE];

         snprintf(filePath, SCE_APPUTIL_SAVEDATA_DATA_PATH_MAXSIZE, "%s/%s/%s", pSS->mSaveDirectoryRoot,
            pSS->mSaveDirectory, pFile->mName);

         SceAppUtilSaveDataDataSlot	saveSlot;
         memset(&saveSlot, 0, sizeof(SceAppUtilSaveDataDataSlot));
         saveSlot.id = kSaveGameSlotId;
         saveSlot.slotParam = &gs_SaveGameSlotParam;

         SceAppUtilSaveDataDataSaveItem saveData;
         memset(&saveData, 0, sizeof(SceAppUtilSaveDataDataSaveItem));
         saveData.dataPath = (const SceChar8*)&filePath[0];
         saveData.buf = pFile->mData;
         saveData.bufSize = pFile->mSize;

         SceSize reqSize;
         res = sceAppUtilSaveDataDataSave(&saveSlot, &saveData, 1, NULL, &reqSize);

         // TRC compliance: If we lack file space, throw up the error dialog, pause the game, and halt until the user creates space
         if (res != SCE_OK)
         {
            if (res == SCE_APPUTIL_ERROR_SAVEDATA_NO_SPACE_FS)
            {
               while (res != SCE_OK)
               {
                  SceSaveDataDialogParam sddParam;
                  SceSaveDataDialogSystemMessageParam sddSystemMessage;

                  sceSaveDataDialogParamInit( &sddParam );
                  sddParam.mode				= SCE_SAVEDATA_DIALOG_MODE_SYSTEM_MSG;
                  sddParam.dispType			= SCE_SAVEDATA_DIALOG_TYPE_SAVE;
                  sddParam.sysMsgParam		= &sddSystemMessage;

                  memset( &sddSystemMessage, 0, sizeof( SceSaveDataDialogSystemMessageParam ) );
                  sddSystemMessage.sysMsgType	= SCE_SAVEDATA_DIALOG_SYSMSG_TYPE_NOSPACE;
                  sddSystemMessage.value		= reqSize; // Shortfall of space, in KB

                  int dialogError;
                  dialogError = sceSaveDataDialogInit( &sddParam );

                  // Make sure the dialog doesn't collide with any error messages
                  if( dialogError != SCE_OK )
                  {
                     continue;
                  }

                  // Spin the thread until you can save successfully
                  SceCommonDialogStatus cdStatus;
                  SceSaveDataDialogResult dialogResult;
                  SceCommonDialogStatus cdSubStatus;
                  int saveResult = res;
                  do
                  {
                     cdStatus = sceSaveDataDialogGetStatus();

                     if (cdStatus == SCE_COMMON_DIALOG_STATUS_RUNNING)
                     {
                        cdSubStatus		= sceSaveDataDialogGetSubStatus();
                        if( cdSubStatus != SCE_COMMON_DIALOG_STATUS_FINISHED ) {
                           continue;
                        }

                        // get savedata dialog result
                        memset( &dialogResult,     0, sizeof( SceSaveDataDialogResult ) );

                        dialogError = sceSaveDataDialogGetResult(&dialogResult);

                        SceSaveDataDialogFinishParam sddFinish;

                        if( cdStatus == SCE_COMMON_DIALOG_STATUS_RUNNING ) {
                           memset( &sddFinish, 0, sizeof( SceSaveDataDialogFinishParam ) );
                           dialogError = sceSaveDataDialogFinish( &sddFinish );
                        }
                     }
                     else if (cdStatus == SCE_COMMON_DIALOG_STATUS_FINISHED)
                     {
                        dialogError = sceSaveDataDialogTerm();

                        res = sceAppUtilSaveDataDataSave(&saveSlot, &saveData, 1, NULL, &reqSize);
                     }
                  } while (cdStatus == SCE_COMMON_DIALOG_STATUS_RUNNING);
               }
            }
            else
            {
               // AS(JM) - Result is not ok. So set it here and break out so we return.

               returnResult = res;
               break;
            }
         }
      }
   }

   return returnResult;
}

//----------------------------------------------------------------------------
static SceInt32 internal_thread_save(SceSize argByteSize, void *args)
{
   CMGS_SaveStatus* pSS = *(CMGS_SaveStatus**)args;
   SceInt32 res = internal_do_save(pSS);

   internal_CMGS_SaveStatus_setResult(pSS, res);

   if(save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback)
      callback(pSS);

   pSS->mStatus = CMGS_SaveStatus::kS_Done;
   OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );

   res = sceKernelExitDeleteThread(0);
   return res;
}

//----------------------------------------------------------------------------
static SceInt32 internal_do_load(CMGS_SaveStatus* pSS)
{
   SceInt32 res = SCE_OK;

   // Check if the directory exists first
   char filePath[SCE_APPUTIL_SAVEDATA_DATA_PATH_MAXSIZE];

   snprintf(filePath, SCE_APPUTIL_SAVEDATA_DATA_PATH_MAXSIZE, "savedata0:%s/%s", pSS->mSaveDirectoryRoot, pSS->mSaveDirectory);

   SceFiosStat stat;
   SceInt32 statRes = SCE_OK;
   statRes = sceFiosStatSync(NULL, filePath, &stat);
   if( statRes != SCE_OK )
   {
      return -1;
   }

   for (int i = 0; i < pSS->GetNumMCFiles(); i++)
   {
      const SMCFile* const pFile = pSS->mpMCFiles + i;

      // Note, it is safe to use the fios api here since we are only reading the data. Only when writing data do we 
      // need to use the apputil api.
      if (pFile->mFileType == kMCFT_SaveGame || pFile->mFileType == kMCFT_Master)
      {
         snprintf(filePath, SCE_APPUTIL_SAVEDATA_DATA_PATH_MAXSIZE, "savedata0:%s/%s/%s", pSS->mSaveDirectoryRoot,
            pSS->mSaveDirectory, pFile->mName);

         SceFiosStat stat;
         SceInt32 statRes = SCE_OK;
         statRes = sceFiosStatSync(NULL, filePath, &stat);
         if (statRes != SCE_OK)
         {
            return CUSTOM_ERROR_FILE_NOT_FOUND;
         }

         if (pSS->ShouldReadFile(i))
         {
            SceUID fd = -1;

            fd = sceIoOpen(filePath, SCE_O_RDONLY, 0);
            if( fd < 0 )
            {
               // Corrupt save data
               return CUSTOM_ERROR_FILE_NOT_FOUND;
            }

            res = sceIoRead(fd, pFile->mData, pFile->mSize);

            // OK for photos to have a variable size
            if( pFile->mInternalFileType != kISD_Photo )
            {
               //BPE_VERIFY(res == pFile->mSize, false, "Error reading save file.");
               if( res != pFile->mSize )
               {
                  // Corrupt save data
                  return CUSTOM_ERROR_SIZE_MISMATCH;
               }
            }

            ((SMCFile*)pFile)->mGenericResult = res;
            sceIoClose(fd);

            // If this is the master file, read beyond the memory card filename and get the transfarring ID
            if (pFile->mFileType == kMCFT_Master)
            {
               int directoryLen = strlen(pSS->mSaveDirectory);
#if MGS_VERSION == 2
               const char* substring = pSS->mSaveDirectory + directoryLen - 4;
#endif
#if MGS_VERSION == 3
               const char* substring = pSS->mSaveDirectory + directoryLen - 7;
#endif

               char gameType = substring[0];
               substring += 1;

               ESaveType saveTypeIndex = kST_MaxSaveTypes;
               switch(gameType)
               {
               case 'G':
                  saveTypeIndex = kST_Game;
                  break;
#if MGS_VERSION == 2
               case 'V':
                  saveTypeIndex = kST_VR;
                  break;
               case 'S':
                  saveTypeIndex = kST_SnakeTales;
                  break;
#endif
#if MGS_VERSION == 3
#endif
               };

               char saveIndex[32];
               strcpy(saveIndex, substring);

               int saveIndexNum = Transfarring_SaveIndexStringToInt( saveIndex );

               // Only load Transfarring IDs for eligible types
               if( saveTypeIndex != kST_MaxSaveTypes )
               {
                  TransfarringID tid;

                  const char* transfarringID = (const char*)pFile->mData + 32;

                  if( strlen(transfarringID) > 0 )
                  {
                     tid.MakeFromString(transfarringID);
                  }
                  else
                  {
                     tid.MakeNewInvalid();
                  }

                  CTransfarringManager::Instance()->SetTransfarringID(tid, saveTypeIndex, saveIndexNum);
               }
            }
         }
         else
         {
            // Need to get the file size even though we are not actually loading the file.
            if (pFile->mFileType == kMCFT_SaveGame)
            {  
               BPE_VERIFY(stat.fileSize > 0, false, "Unable to determine size of save file.");
               ((SMCFile*)pFile)->mGenericResult = (int)stat.fileSize;
            }
         }
         ((SMCFile*)pFile)->mFileModifiedTime = stat.modificationDate / 1000000000ULL;
      }
   }

   return SCE_OK;
}

//----------------------------------------------------------------------------
static SceInt32 internal_thread_load(SceSize argByteSize, void *args)
{
   CMGS_SaveStatus* pSS = *(CMGS_SaveStatus**)args;
   SceInt32 res = internal_do_load(pSS);

   internal_CMGS_SaveStatus_setResult(pSS, res);
   
   if(save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback)
      callback(pSS);

   pSS->mStatus = CMGS_SaveStatus::kS_Done;
   OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );

   res = sceKernelExitDeleteThread(0);
   return res;
}

//----------------------------------------------------------------------------
static SceInt32 internal_thread_recreate_save(SceSize argByteSize, void *args)
{
   CMGS_SaveStatus* pSS = *(CMGS_SaveStatus**)args;

   internal_do_delete(pSS);
   internal_do_save(pSS);

   internal_CMGS_SaveStatus_setResult(pSS, 0);

   if(save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback)
      callback(pSS);

   pSS->mStatus = CMGS_SaveStatus::kS_Done;
   OsContext()->ClearUnsafeToShutDownFlag(CBaseOsContext::kUSDF_SaveData);

   SceInt32 res = sceKernelExitDeleteThread(0);
   return res;
}

//----------------------------------------------------------------------------
static SceInt32 internal_thread_get_save_list(SceSize argByteSize, void *args)
{
   SceInt32 res = SCE_OK;
   CMGS_SaveStatus* pSS = *(CMGS_SaveStatus**)args;

#if MGS_VERSION==2
   memset(pSS->mpGetDirTable, 0, pSS->mGetDirListMaxEntries * sizeof(*pSS->mpGetDirTable));
#elif MGS_VERSION==3
   memset(pSS->mpGetDirTable2, 0, pSS->mGetDirListMaxEntries * sizeof(*pSS->mpGetDirTable2));
#endif

   int currSaveEntry = 0;
   SceFiosDH saveDirHandle;
   SceFiosBuffer buf;

   void* bufMem = malloc(64 * 1024); // Documentation recommends 64K for this buffer.
   buf.set(bufMem, 64 * 1024);

   char acFilePath[SCE_APPUTIL_MOUNTPOINT_DATA_MAXSIZE + 256] = { 0 };
   strncpy(acFilePath, "savedata0:", sizeof(acFilePath) - 1);
   strncat(acFilePath, pSS->mSaveDirectoryRoot, sizeof(acFilePath) - 1);

   // Find all subdirectories within the save data directory for the current game in order to determine how many save
   // entries exist. Each save entry is within its own subdirectory.
   res = sceFiosDHOpenSync(NULL, &saveDirHandle, acFilePath, buf);
   if (res == SCE_OK) // If this function fails then there wasn't any save data available.
   {
      SceFiosDirEntry saveDirContents;
      while (sceFiosDHReadSync(NULL, saveDirHandle, &saveDirContents) == SCE_FIOS_OK)
      {
         // Remove the save directory root before reporting the list.
         std::string pathStr = saveDirContents.fullPath;
         std::string::size_type npos = pathStr.find_last_of('/');
         pathStr = pathStr.substr(npos + 1, pathStr.length() - npos - 1);         
         
         // Skip the folders for save data we're not looking for
         if (std::strstr(pathStr.c_str(), pSS->mSaveDirectory) == 0)
         {
            continue;
         }
#if MGS_VERSION == 2
         sceMcTblGetDir* pTable = pSS->mpGetDirTable + currSaveEntry;

         memset(pTable, 0, sizeof(*pTable));
         strncpy((char*)&pTable->EntryName[0], pathStr.c_str(), sizeof(pTable->EntryName) - 1);
         pTable->AttrFile |= 0x0020;

         // Get the directory status and set the modification times
         SceFiosStat directoryStatus;
         memset(&directoryStatus, 0, sizeof(directoryStatus));
         int statResult = sceFiosStatSync(NULL, saveDirContents.fullPath, &directoryStatus);

         pTable->_Modify = directoryStatus.modificationDate / 1000000000ULL;

#elif MGS_VERSION == 3
         SceMc2DirParam* pTable = pSS->mpGetDirTable2 + currSaveEntry;
         
         memset(pTable, 0, sizeof(*pTable));
         strncpy((char*)&pTable->name, pathStr.c_str(), sizeof(pTable->name) - 1);
         pTable->attribute |= 0x0020;

         // Get the directory status and set the modification times
         SceFiosStat directoryStatus;
         memset(&directoryStatus, 0, sizeof(directoryStatus));
         int statResult = sceFiosStatSync(NULL, saveDirContents.fullPath, &directoryStatus);
         SceDateTime dateTime;
         tm t;
         sceFiosDateToComponents(directoryStatus.modificationDate, &t);
         pTable->modification.year = t.tm_year;
         pTable->modification.day = t.tm_mday;
         pTable->modification.month = t.tm_mon;
         pTable->modification.hour = t.tm_hour;
         pTable->modification.min = t.tm_min;
         pTable->modification.sec = t.tm_sec;

#endif
         
         currSaveEntry++;
         if (currSaveEntry == pSS->mGetDirListMaxEntries)
            break;
      }

      res = sceFiosDHCloseSync(NULL, saveDirHandle);
      BPE_VERIFY(res == SCE_FIOS_OK, false, "Error closing save directory.");
   }

   free(bufMem);

#if MGS_VERSION==2
   pSS->mGetDirListExistEntries = currSaveEntry;
#elif MGS_VERSION==3
   *pSS->mpGetDirListExistEntries = currSaveEntry;
#endif

   internal_CMGS_SaveStatus_setResult(pSS, 0);

   // Invoke the completion callback if valid.
   if (save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback)
      callback( pSS );

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   res = sceKernelExitDeleteThread(0);
   return res;
}

//----------------------------------------------------------------------------
void save_data_init(const char * const saveDirectoryRoot)
{
   int res;
   
   // Create the save slot if it does not already exist.
   memset(&gs_SaveGameSlotParam, 0, sizeof(SceAppUtilSaveDataSlotParam));

   res = sceAppUtilSaveDataSlotGetParam(kSaveGameSlotId, &gs_SaveGameSlotParam, NULL);
   if (res != SCE_OK)
   {
      // If we were unable to get parameter information for the save slot, it must not exist so create it.
      BPE_ASSERT_NO_MSG(res == SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND);

      strncpy((char*)&gs_SaveGameSlotParam.title, internal_get_save_game_localized_string(kSGS_Title), 
         SCE_APPUTIL_SAVEDATA_SLOT_TITLE_MAXSIZE - 1);
      strncpy((char*)&gs_SaveGameSlotParam.subTitle, internal_get_save_game_localized_string(kSGS_Subtitle), 
         SCE_APPUTIL_SAVEDATA_SLOT_TITLE_MAXSIZE - 1);

      res = sceAppUtilSaveDataSlotCreate(kSaveGameSlotId, &gs_SaveGameSlotParam, NULL);
      BPE_VERIFY(res == SCE_OK, false, "Unable to create save slot.");
   }
}

//----------------------------------------------------------------------------
int save_data_auto_load(CMGS_SaveStatus * pSS, save_data_async_op_callback callback)
{
   BPE_ASSERT(pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress");

   pSS->mpAsyncOpCallback = (void*)callback;

   // To prevent interrupting the processing of the main thread while the save thread is writing out the data.
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   OsContext()->SetUnsafeToShutDownFlag(CBaseOsContext::kUSDF_SaveData);  // don't allow the user to interrupt this

   SceInt32 res = sceKernelCreateThread("SaveData_Thread", internal_thread_load, NVtaThreadPriorities::kPrioritySaveLoad,
      32768, 0, SCE_KERNEL_CPU_MASK_USER_ALL, SCE_NULL);
   BPE_VERIFY(res >= 0, false, "Error spawning save thread.");

   res = sceKernelStartThread(res, sizeof(CMGS_SaveStatus*), &pSS);
   BPE_VERIFY(res == SCE_OK, false, "Error starting load thread.");

   return 0;
}

//----------------------------------------------------------------------------
int save_data_auto_save(CMGS_SaveStatus* pSS, save_data_async_op_callback callback)
{
   BPE_ASSERT(pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress");

   pSS->mpAsyncOpCallback = (void*)callback;

   // To prevent interrupting the processing of the main thread while the save thread is writing out the data.
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   OsContext()->SetUnsafeToShutDownFlag(CBaseOsContext::kUSDF_SaveData);  // don't allow the user to interrupt this

   SceInt32 res = sceKernelCreateThread("SaveData_Thread", internal_thread_save, NVtaThreadPriorities::kPrioritySaveLoad,
      4096, 0, SCE_KERNEL_CPU_MASK_USER_ALL, SCE_NULL);
   BPE_VERIFY(res >= 0, false, "Error spawning save thread.");

   res = sceKernelStartThread(res, sizeof(CMGS_SaveStatus*), &pSS);
   BPE_VERIFY(res == SCE_OK, false, "Error starting save thread.");

   return 0;
}

//----------------------------------------------------------------------------
int save_data_space_check( CMGS_SaveStatus * pSS )
{
   // TODO MCampbell: Do we need to check on Vita?
   return 0;
}

//----------------------------------------------------------------------------
int save_data_auto_delete(CMGS_SaveStatus* pSS)
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   // To prevent interrupting the processing of the main thread while the save thread is writing out the data.
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   OsContext()->SetUnsafeToShutDownFlag(CBaseOsContext::kUSDF_SaveData);  // don't allow the user to interrupt this

   SceInt32 res = sceKernelCreateThread("DeleteSave_Thread", internal_thread_delete_save, 
      NVtaThreadPriorities::kPrioritySaveLoad, 32768, 0, SCE_KERNEL_CPU_MASK_USER_ALL, SCE_NULL);
   BPE_VERIFY(res >= 0, false, "Error spawning delete_save thread.");

   res = sceKernelStartThread(res, sizeof(CMGS_SaveStatus*), &pSS);
   BPE_VERIFY(res == SCE_OK, false, "Error starting delete_save thread.");

   return 0;
}

//----------------------------------------------------------------------------
int save_data_get_dir_list(CMGS_SaveStatus * pSS, save_data_async_op_callback callback)
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );
   
   pSS->mpAsyncOpCallback = (void*)callback;

   // To prevent interrupting the processing of the main thread while the save thread is writing out the data.
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   
   SceInt32 res = sceKernelCreateThread("GetDirList_Thread", internal_thread_get_save_list, 
      NVtaThreadPriorities::kPrioritySaveLoad, 32768, 0, SCE_KERNEL_CPU_MASK_USER_ALL, SCE_NULL);
   BPE_VERIFY(res >= 0, false, "Error spawning get dir list thread.");

   res = sceKernelStartThread(res, sizeof(CMGS_SaveStatus*), &pSS);
   BPE_VERIFY(res == SCE_OK, false, "Error starting get_dir_list thread.");

   return 0;
}

//----------------------------------------------------------------------------
int save_data_get_info(CMGS_SaveStatus * pSS)
{
   BPE_ASSERT(pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress");

   // Not an operation on Vita. Just set the device id to an arbitrary positive integer.
   pSS->mDeviceId = 1;
   internal_CMGS_SaveStatus_setResult(pSS, 0);
   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   return 0;
}

//----------------------------------------------------------------------------
int save_data_auto_recreate_save(CMGS_SaveStatus * pSS, save_data_async_op_callback callback)
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   // To prevent interrupting the processing of the main thread while the save thread is writing out the data.
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   OsContext()->SetUnsafeToShutDownFlag(CBaseOsContext::kUSDF_SaveData);  // don't allow the user to interrupt this

   SceInt32 res = sceKernelCreateThread("RecreateSave_Thread", internal_thread_recreate_save, 
      NVtaThreadPriorities::kPrioritySaveLoad, 32768, 0, SCE_KERNEL_CPU_MASK_USER_ALL, SCE_NULL);
   BPE_VERIFY(res >= 0, false, "Error spawning recreate_save thread.");

   res = sceKernelStartThread(res, sizeof(CMGS_SaveStatus*), &pSS);
   BPE_VERIFY(res == SCE_OK, false, "Error starting recreate_save thread.");

   return 0;
}
