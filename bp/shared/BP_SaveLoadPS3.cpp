//------------------------------------------------------------------------------------------
// BP_SaveLoadPS3.cpp
// BP adapted from CP3
//
// Utility functions for saving/loading on PS3.
//------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <sys/paths.h>

#include "assert.h"
#include "BP_SaveLoad.h"

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"

#ifdef GOLD_VERSION
   #define printf(...)
#endif

//NOTE: We still save the savedata info in a file (MASTER.BIN), it is just not used for listing savedatas anymore.
//Slight performance gain in loading/saving we could achieve from removing it is not worth the risk of breaking stuff at this point.
#define BP_PS3_STORE_MGS_SAVEDATA_INFO_IN_PARAM_SFO

extern "C" void BP_Encode_PS3_FileParam( const char * const saveDirectory, const char * const infoBase64, char * const detail, char * const listParam );
extern "C" void BP_Decode_PS3_FileParam( const char * const saveDirectory, char * const infoBase64, const char * const detail, const char * const listParam, const int bind );

//----------------------------------------------------------------------------

static void thr_auto_save(uint64_t arg);
static void thr_auto_load(uint64_t arg);
static void thr_auto_delete(uint64_t arg);
static void thr_auto_recreate_save(uint64_t arg);
static void thr_get_list(uint64_t arg);

static void thr_fake_auto_load_mgs_master(uint64_t arg);

static void callback_data_status_save( CellSaveDataCBResult *result, CellSaveDataStatGet *get, CellSaveDataStatSet *set );
static void callback_file_operation_save( CellSaveDataCBResult *result, CellSaveDataFileGet *get, CellSaveDataFileSet *set );

static void callback_data_status_load( CellSaveDataCBResult *result, CellSaveDataStatGet *get, CellSaveDataStatSet *set );
static void callback_file_operation_load( CellSaveDataCBResult *result, CellSaveDataFileGet *get, CellSaveDataFileSet *set );

static void callback_data_status_recreate_save( CellSaveDataCBResult *result, CellSaveDataStatGet *get, CellSaveDataStatSet *set );
static void callback_file_operation_recreate_save( CellSaveDataCBResult *result, CellSaveDataFileGet *get, CellSaveDataFileSet *set );

static void callback_data_fixed_delete( CellSaveDataCBResult *cbResult, CellSaveDataListGet *get, CellSaveDataFixedSet *set );
static void callback_data_done_delete( CellSaveDataCBResult *result, CellSaveDataDoneGet *get );

static void dumpCellSaveDataStatGet( CellSaveDataStatGet *get );

extern "C" int BP_TrophySystem_HDDNeededKB();

//----------------------------------------------------------------------------

static int const kSAVEDATA_THREAD_PRIORITY	= 1001;
static int const kSAVEDATA_THREAD_STACKSIZE	= 16 * 1024;

// Encrypted ID for protected mData file
// From selection of numbers from online random number generator
static char const sSecureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE] =
{
   0x26, 0x58, 0xd0, 0x26, 0xf5, 0x0b, 0x45, 0x03, 0xb1, 0x16, 0x4c, 0xf6, 0x11, 0xf9, 0xa7, 0x51
};

std::time_t gFileModTime;

//----------------------------------------------------------------------------

// Dump CellSaveDataListGet structure received in the fixed mData callback and mData list callback
static void dumpCellSaveDataListGet( CellSaveDataListGet *get )
{
	printf("Dump CellSaveDataListGet in the CellSaveDataListCallback or CellSaveDataListCallback--------------------\n" );
	printf("\tget->dirNum : %d\n", get->dirNum );
	printf("\tget->dirListNum: %d\n", get->dirListNum );
	/* Dump every directory parameter of received directory list */
	for( unsigned int i = 0; i < get->dirListNum; i++ ) {
		printf("\t%4d  DIRNAME: %s\t\tPARAM: %s\n", i, get->dirList[i].dirName, get->dirList[i].listParam );
	}
}

//----------------------------------------------------------------------------
// Dump CellSaveDataStatGet structure received in the mData status callback
static void dumpCellSaveDataStatGet( CellSaveDataStatGet *get )
{
	printf("Dump CellSaveDataStatGet in CellSaveDataStatCallback--------------------\n" );
	printf("\tget->dir.dirName : %s\n", get->dir.dirName );
	printf("\tget->isNewData: %d\n", get->isNewData );
	printf("\tget->hddFreeSizeKB 0x%x\n", get->hddFreeSizeKB);
	printf("\tget->sizeKB : 0x%x\n", get->sizeKB);
	printf("\tget->sysSizeKB : 0x%x\n", get->sysSizeKB);
	printf("\tget->bind : %d\n", get->bind);
	printf("\tget->dir : dirName : %s  atime : %lld mtime : %lld ctime : %lld\n", get->dir.dirName, get->dir.st_atime, get->dir.st_mtime, get->dir.st_ctime );
	printf("\tget->fileListNum: %d\n", get->fileListNum );
	/* Dump every file parameter of received file list */
	for( unsigned int i = 0; i < get->fileListNum; i++ ) {
		printf("\t%3d  FILENAME: %s   type : %d  size : %lld  atime : %lld mtime : %lld ctime : %lld\n", i,
			get->fileList[i].fileName,
			get->fileList[i].fileType,
			get->fileList[i].st_size,
			get->fileList[i].st_atime,
			get->fileList[i].st_mtime,
			get->fileList[i].st_ctime );
	}
	printf("\tget->fileNum: %d\n", get->fileNum );
	printf("\n" );
	printf("\tPARAM.SFO:TITLE: %s\n", get->getParam.title );
	printf("\tPARAM.SFO:SUB_TITLE: %s\n", get->getParam.subTitle );
	printf("\tPARAM.SFO:DETAIL: %s\n", get->getParam.detail );
	printf("\tPARAM.SFO:ATTRIBUTE: %d\n", get->getParam.attribute );
	printf("\tPARAM.SFO:LIST_PARAM: %s\n", get->getParam.listParam );
	printf("\n" );
}

//----------------------------------------------------------------------------
// Helper function, this is probably going to change a bit.
static void _CMGS_SaveStatus_setResult(CMGS_SaveStatus *pSS, int saveRet)
{
	pSS->mCELL_Result = saveRet;

   if (pSS->mCELL_Result == CELL_OK)
   {
      // All good
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;
//**** Hack to test corruption detection
//      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
   }
   else 
   {
      // Problem!
      if (pSS->mCELL_Result != CELL_SAVEDATA_ERROR_CBRESULT)
      {
         // Error that's not triggered by us, reset our result to indicate this.
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_CELL_Error;
      }
   }
}

//----------------------------------------------------------------------------
//Shared b/t save and recreate-save
static void _CMGS_SaveStatus_setParamSFO( CMGS_SaveStatus *pSS, CellSaveDataStatSet *set )
{
   // Set parameters for savedata PARAM.SFO
   //TODO: Setup age restrictions (do we need to do this anymore?)
   BPE_ASSERT_NO_MSG( pSS->mpMCFiles[0].mFileType == kMCFT_SaveGame ); //make sure no ordering error!
   const EInternalSaveData internalFileType = pSS->mpMCFiles[0].mInternalFileType;
   strncpy( set->setParam->title, save_data_get_save_game_localized_string(kSGS_Title,internalFileType), CELL_SAVEDATA_SYSP_TITLE_SIZE );
   strncpy( set->setParam->subTitle, save_data_get_save_game_localized_string(kSGS_Subtitle,internalFileType), CELL_SAVEDATA_SYSP_SUBTITLE_SIZE );
   if( internalFileType == kISD_MG1 || internalFileType == kISD_MG2 )
   {
      //Metal Gear 1 saves can be difficult to distinguish from one another in the XMB because the playtime recorded in the listing
      //is only updated at checkpoints.  Same problem in MG2.
      //Adding a slot index string that matches the one in the in-game file listing helps avoid a TRC issue.
      const char * const inIndexStr = pSS->mSaveDirectory+strlen(pSS->mSaveDirectory)-3;
      char outIndexStr[16] = { 0 };
      int index = 0;

      if( 1 != sscanf( inIndexStr, "%x", &index ) )
      {
         BPE_VERIFY( false, false, "unexpected save index string" );
      }

      sprintf( outIndexStr, " [%03d]", index );
      strcat( set->setParam->subTitle, outIndexStr );
   }

   memset( set->setParam->detail, 0, CELL_SAVEDATA_SYSP_DETAIL_SIZE );

   memset( set->setParam->listParam, 0, CELL_SAVEDATA_SYSP_LPARAM_SIZE );

   memset( set->setParam->reserved, 0x0, sizeof(set->setParam->reserved) );	// The reserved member must be zero-filled
   memset( set->setParam->reserved2, 0x0, sizeof(set->setParam->reserved2) );	// The reserved member must be zero-filled
   set->setParam->attribute = CELL_SAVEDATA_ATTR_NORMAL;

   BP_Encode_PS3_FileParam( pSS->mSaveDirectory+strlen(pSS->mSaveDirectoryRoot), (const char*)pSS->mpMCFiles[kMCFT_Master].mData, set->setParam->detail, set->setParam->listParam );
}

//----------------------------------------------------------------------------

void save_data_init( const char * const saveDirectoryRoot )
{
   //unused on PS3.
}

//----------------------------------------------------------------------------

int save_data_auto_save( CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   BPE_ASSERT_NO_MSG( pSS->mpMCFiles[0].mFileType == kMCFT_SaveGame ); //make sure no ordering error!
   if (pSS->mWrongUser && ( pSS->mpMCFiles[0].mInternalFileType != kISD_Photo ) )
   {
      // handle the wrong user scenario here, so that we immediately fail if this save game mData cannot be saved.
      // Photos are the exception-- do not prevent the user from overwriting wrong user photos in album view
      // or taking pictures in-game from another user's save.
      pSS->mStatus = CMGS_SaveStatus::kS_Done;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;  // indicate no real failure here.

      //This callback is normally called by the thread that we're not starting.
      if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
      {
         callback( pSS );
      }
   }
   else
   {
      // To prevent interrupting the processing of the main thread, save mData utility function must be called using a sub thread
      pSS->mStatus = CMGS_SaveStatus::kS_Processing;
      OsContext()->SetUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );  // don't allow the user to interrupt this

      sys_ppu_thread_t tid;
      int ret = sys_ppu_thread_create(&tid,
         thr_auto_save, 
         (uint64_t) pSS,
         kSAVEDATA_THREAD_PRIORITY, kSAVEDATA_THREAD_STACKSIZE,
         0, "AUTO_SAVE");


      if (ret != 0) {
         printf("auto save thread create failed %d\n", ret);
         return -1;
      }
   }

   return 0;
}

//----------------------------------------------------------------------------

void thr_auto_save(uint64_t arg)
{
	printf( "thr_auto_save() start\n");

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) arg;

	// Settings for general buffer
	CellSaveDataSetBuf _cell_saveDataSetBuf;
	_cell_saveDataSetBuf.dirListMax  = 0;										      // This member is not used (specify zero)
	_cell_saveDataSetBuf.fileListMax = pSS->GetNumMCFiles();						         // Maximum number of files listed up in the mData status callback
	memset( _cell_saveDataSetBuf.reserved, 0x0, sizeof(_cell_saveDataSetBuf.reserved) );	// The reserved member must be zero-filled

	// Size of buffer must be such that it can store mData of the size specified by the above maximums
   int const _saveDataFileDataBufferSize = pSS->GetNumMCFiles() * sizeof(CellSaveDataFileStat);
   void *_saveDataFileDataBuffer = alloca(_saveDataFileDataBufferSize);
   memset(_saveDataFileDataBuffer, 0, _saveDataFileDataBufferSize);

	_cell_saveDataSetBuf.bufSize = _saveDataFileDataBufferSize;
	_cell_saveDataSetBuf.buf = _saveDataFileDataBuffer;

	// Execute auto save
	int ret = cellSaveDataAutoSave2(
					CELL_SAVEDATA_VERSION_CURRENT,	   // Version of save mData format
               pSS->mSaveDirectory,	               // Full directory mName for save mData
					CELL_SAVEDATA_ERRDIALOG_NONE,     // Settings for outputting error dialog
					&_cell_saveDataSetBuf,				   // CellSaveDataSetBuf
					callback_data_status_save,			         // Data status callback function
					callback_file_operation_save,			      // File operation callback function
					SYS_MEMORY_CONTAINER_ID_INVALID,    // Memory container ID
					(void*)pSS                     // Application-defined mData
				);


	printf("cellSaveDataAutoSave2() : 0x%x\n", ret);

   _CMGS_SaveStatus_setResult(pSS, ret);

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

	printf( "thr_auto_save() end\n");

   if( (CELL_SAVEDATA_RET_OK == ret) && (pSS->mChosenGameSave >= 0) )
   {  // if the save worked, update the file modification time.
      SMCFile const *pMCFile = &pSS->GetMCFiles()[pSS->mChosenGameSave];
		((SMCFile *)pMCFile)->mFileModifiedTime = gFileModTime;
   }

   pSS->mStatus = CMGS_SaveStatus::kS_Done;
   OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );   // the critical process is complete.. you may shut down now

	sys_ppu_thread_exit(0);
}

static bool gOverwriteFile = false;
//----------------------------------------------------------------------------
// Data status callback for cellSaveDataXXXXSave()
void callback_data_status_save( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataStatGet *get, CellSaveDataStatSet *set )
{
   //printf( "->Save: DATA callback\n");

	// Dump obtained save mData status
	dumpCellSaveDataStatGet(get);

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO;	   // Whether or not to first delete save mData
   
   set->setParam = &get->getParam; 							// You can specify get->getParam to set->setParam directly

	uint32_t hdSpaceReq = BP_TrophySystem_HDDNeededKB();
	if (get->isNewData)
	{
		hdSpaceReq += pSS->GetMCFilesTotalSizeInKB();
		hdSpaceReq += get->sysSizeKB;
	}
	if (hdSpaceReq > get->hddFreeSizeKB)
	{
      pSS->mHDSizeReqKB = hdSpaceReq - get->hddFreeSizeKB;
      _cellSaveDataResult->errNeedSizeKB = -pSS->mHDSizeReqKB;   //the docs say to set the value as the negative
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoSpace;
      printf("Save needs %d KB HDD space more. HDD KB free: %d.\n", -_cellSaveDataResult->errNeedSizeKB, get->hddFreeSizeKB);
	}
	else
	if (!get->isNewData)
   {
		printf("Save data %s already exists.\n", get->dir.dirName );

      gOverwriteFile = true; // first delete the save file and then re-create it

      // Validate files
      for (size_t i = 0; i < get->fileListNum; i++)
      {
         CellSaveDataFileStat const *pExistingFile = &get->fileList[i];
         // Check for matching hardcoded filename for our save
         SMCFile const *pMatchingSaveFile = pSS->FindMCFileInfoFromName(pExistingFile->fileName);
#if 1
         //During development, allow icon to change without flagging save file as invalid.
         const bool bPassBecauseItsAnIconFile = pMatchingSaveFile != NULL && pMatchingSaveFile->mFileType == kMCFT_System;
#else
         bool bPassBecauseItsAnIconFile = false;
#endif
         if (pMatchingSaveFile && (pExistingFile->st_size == (uint64_t)pMatchingSaveFile->mSize || bPassBecauseItsAnIconFile))
         {
            // Filename and size both match, we're good
            pSS->mMatchingFilesFound++;
         }
         else
         {
            // Name or size mismatch
            pSS->mUnmatchedFilesFound++;
         }
      }

	   // Check to see if we've got all of our files.
      if (pSS->IsFileIterationValid())
      {
         // We're good, start loading
         _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
      }
      else
      {
         // Not so good, save filenames or sizes are incorrect.
         // We can either fix the offending file, or zap the entire save.
         //TODO: Fix the offending file!
         _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
      }
	}
	else
   {
		// Save data does not exist
		printf("Save data %s DOES NOT exist.\n", get->dir.dirName );

      gOverwriteFile = false;

      _CMGS_SaveStatus_setParamSFO( pSS, set );

      // We've got enough free space on the HDD
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
	}

#if 0 //Test corrupt-on-save case
   _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
   pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
#endif
   //printf( "<-Save: DATA callback\n");
   return;
}

//----------------------------------------------------------------------------
// File operation callback for cellSaveDataXXXXSave()
void callback_file_operation_save( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataFileGet *get, CellSaveDataFileSet *set )
{
	(void)get;

   //printf( "->Save: FILE callback\n");

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

   // Skip over files that don't need creating
   // Check if we've processed all the save files

   // Assume we're on last file, will get overridden below if files remaining.
   _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;

   {
      bool bBreak = false;
      while (!bBreak && (pSS->mCurrentMCFileIndex < pSS->GetNumMCFiles()))
      {      
         // Do we need to create a new save file?
         if (pSS->ShouldWriteFile(pSS->mCurrentMCFileIndex))
         {
            // Yep, need to write this file index
            SMCFile const *pMCFile = &pSS->GetMCFiles()[pSS->mCurrentMCFileIndex];
			// Also update the 'modified' time
			// When the file is read in, it will use the file's timestamp, but we won't be reading the file back in
			// until the user reboots.  Fill this in with the current system time which should be very close to the same time.
            gFileModTime = time(NULL);

		      set->fileType = pSS->Get_CELL_SAVEDATA_FILETYPE(pSS->mCurrentMCFileIndex);

            // Set appropriate structures based on file type.
            if (set->fileType == CELL_SAVEDATA_FILETYPE_SECUREFILE)
            {
               // Set secure file encryption key
               memcpy(set->secureFileId, sSecureFileId, sizeof(sSecureFileId));
               set->fileName = (char *) pMCFile->mName;
            }
            else if (set->fileType == CELL_SAVEDATA_FILETYPE_NORMALFILE)
            {
               set->fileName = (char *) pMCFile->mName;
            }

            if (pMCFile->mFileType == kMCFT_SaveGame && gOverwriteFile)
            {  // this file must first be deleted so that we can then re-write it
               // this is in case the old file has gotten corrupted
		         set->fileOperation = CELL_SAVEDATA_FILEOP_DELETE;
               pSS->mCurrentMCFileIndex--;  // we're deleting this file.. gonna need to come back
               gOverwriteFile = false;
            }
            else
            {
		         set->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
            }
		      set->fileBuf = pMCFile->mData;
		      set->fileBufSize = pMCFile->mSize;
		      set->fileSize = pMCFile->mSize;
		      set->fileOffset = 0;
		      set->reserved = NULL;

            printf( "SAVING: %s\n", pMCFile->mName);
            // Still got some files to process, keep going
            _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
            bBreak = true;
         }

         // We don't need to save this file, move to the next one.
         pSS->mCurrentMCFileIndex++;
      }

   }   

   //printf(   "<-Save: FILE callback\n");
	return;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int save_data_auto_delete( CMGS_SaveStatus * pSS )
{
   BPE_VERIFY( false, false, "" );   //this function is unusable for shipping...
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   if (pSS->mWrongUser)  // handle the wrong user scenario here, so that we immediately fail if this save game mData cannot be saved.
   {
      pSS->mStatus = CMGS_SaveStatus::kS_Done;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;  // indicate no real failure here.
   }
   else
   {
      // To prevent interrupting the processing of the main thread, save mData utility function must be called using a sub thread
      pSS->mStatus = CMGS_SaveStatus::kS_Processing;
      OsContext()->SetUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );  // don't allow the user to interrupt this

      sys_ppu_thread_t tid;
      int ret = sys_ppu_thread_create(&tid,
         thr_auto_delete, 
         (uint64_t) pSS,
         kSAVEDATA_THREAD_PRIORITY, kSAVEDATA_THREAD_STACKSIZE,
         0, "AUTO_SAVE");


      if (ret != 0) {
         printf("auto delete thread create failed %d\n", ret);
         return -1;
      }
   }

   return 0;
}

//----------------------------------------------------------------------------

void thr_auto_delete(uint64_t arg)
{
   printf( "thr_auto_delete() start\n");

   CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) arg;

   CellSaveDataSetList setList = { 0 };
   setList.sortType = CELL_SAVEDATA_SORTTYPE_SUBTITLE;
   setList.sortOrder = CELL_SAVEDATA_SORTORDER_ASCENT;
   setList.dirNamePrefix = pSS->mSaveDirectory;

   CellSaveDataSetBuf setBuf = { 0 };
   setBuf.dirListMax = 1;
   setBuf.fileListMax = 0;
   setBuf.bufSize = setBuf.dirListMax * sizeof(CellSaveDataDirList);
   setBuf.buf = alloca(setBuf.bufSize);

   // Execute auto save
   int ret = cellSaveDataFixedDelete(
      &setList,	               // Full directory mName for save mData
      &setBuf,				   // CellSaveDataSetBuf
      callback_data_fixed_delete,
      callback_data_done_delete,			      // File operation callback function
      SYS_MEMORY_CONTAINER_ID_INVALID,    // Memory container ID
      (void*)pSS                     // Application-defined mData
      );


   printf("cellSaveDataListDelete() : 0x%x\n", ret);

   _CMGS_SaveStatus_setResult(pSS, ret);

   printf( "thr_auto_delete() end\n");

   pSS->mStatus = CMGS_SaveStatus::kS_Done;
   OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );   // the critical process is complete.. you may shut down now

   sys_ppu_thread_exit(0);
}

static void callback_data_fixed_delete( CellSaveDataCBResult *cbResult, CellSaveDataListGet *get, CellSaveDataFixedSet *set )
{
   printf( "callback_data_fixed_delete() start\n");

   dumpCellSaveDataListGet(get);

   if( get->dirListNum == 0 ) {
      cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
      return;
   }

   //Delete the one file we got.
   set->dirName = get->dirList[0].dirName;
   set->newIcon = NULL;
   set->option = CELL_SAVEDATA_OPTION_NOCONFIRM;

   cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

   printf( "callback_data_fixed_delete() end\n");
}

void callback_data_done_delete( CellSaveDataCBResult *cbResult, CellSaveDataDoneGet *get )
{
   switch(get->excResult) {
   case CELL_SAVEDATA_RET_OK:
      printf("DATA \"%s\" (size : %u KB) was deleted. HddFreeSpace : %u KB.\n", get->dirName, get->sizeKB, get->hddFreeSizeKB);
      cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST_NOCONFIRM;
      break;
   case CELL_SAVEDATA_ERROR_FAILURE:
      printf("Failed to delete DATA \"%s\" (size : %u KB). HddFreeSpace : %u KB.\n", get->dirName, get->sizeKB, get->hddFreeSizeKB);
      cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;
      break;
   default:
      printf("callback_data_done_delete: UNKNOWN ERROR %d.\n", get->excResult);
      cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;
      break;
   }
}

//----------------------------------------------------------------------------

void cb_space_check_load( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataStatGet *get, CellSaveDataStatSet *set )
{
   //This function's only purpose is to retrieve the space necessary to save a savedata record of this type
   //and the current free amount of HDD space.  Grab these and return.

   printf( "cb_space_check_load() start\n");

   CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;
   //Record required sys space per savedata record
   pSS->mSysSizePerDirReqKB = get->sysSizeKB;
   //Need this amount of system space for each dir in which we are checking for savedata
   pSS->mHDSizeReqKB += get->sysSizeKB * pSS->mNumMCStartupCheckDirs;

   pSS->mHDFreeSizeKB = get->hddFreeSizeKB;

   _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;

}

void cb_space_check_list( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataListGet *get, CellSaveDataFixedSet *set )
{
   unsigned int i;

   printf( "cb_space_check_list() start\n");

   /* Dump obtained list */
   dumpCellSaveDataListGet(get);

   CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

   if( get->dirNum > get->dirListNum ) {
      /* This case can be handled in any way that is appropriate for the application's specifications */
      printf( "found too many datas. expected[%d] < found[%d]\n", get->dirListNum, get->dirNum );
   }

   CellSaveDataDirStat dirStat;
   CellSaveDataSystemFileParam sysFileParam;
   unsigned int bind;
   int sizeKB;

   //Flag when we encounter valid savedata of each type.
   bool * const bValidSaveDataEncountered = (bool*)alloca(pSS->mNumMCStartupCheckDirs * sizeof(bool));
   memset( bValidSaveDataEncountered, 0, pSS->mNumMCStartupCheckDirs * sizeof(bool) );

   int remainingToCheck = pSS->mNumMCStartupCheckDirs;

   int rootLen = strlen( pSS->mSaveDirectoryRoot );
   for( int i=0; i < get->dirListNum; ++i )
   {
      CellSaveDataDirList & dirList = get->dirList[i];
      //Check which, if any, checked savedata type this entry matches.
      for( int j=0; j<pSS->mNumMCStartupCheckDirs; ++j )
      {
         if( bValidSaveDataEncountered[j] )
         {
            //don't need to look for any more of this savedata type.
            continue;
         }
         const SMCStartupCheckDir & startupCheckDir = pSS->mpMCStartupCheckDirs[j];
         //Check for a match between this entry's dirname and the dirname prefix set in the startup check entry.
         if( !strncmp( dirList.dirName+rootLen, startupCheckDir.mDirNamePrefix, strlen(startupCheckDir.mDirNamePrefix) ) )
         {
            //Check whether this is a valid entry we can count against the startup space required.
            int ret = cellSaveDataGetListItem( dirList.dirName, &dirStat, &sysFileParam, &bind, &sizeKB );
            if( ret == CELL_OK )
            {
               if( sizeKB >= startupCheckDir.mContentsSizeKB + pSS->mSysSizePerDirReqKB )
               {
                  //Assume that this file is valid and we can overwrite it if desired once the game starts.
                  //Subtract this space from "necessary for game startup" number.
                  int newReqKB = bpe::max_val( 0, pSS->mHDSizeReqKB - sizeKB );
                  printf("cb_space_check_list(): found valid %s (%d): HD size req kb %d -> %d\n", dirList.dirName, sizeKB, pSS->mHDSizeReqKB , newReqKB );
                  pSS->mHDSizeReqKB = newReqKB;
                  //Don't look for any more of this type.
                  bValidSaveDataEncountered[j] = true;
                  --remainingToCheck;
                  //Done looking for matches in the startupCheckDirs array after we've found this match.
                  break;
               }
               else
               {
                  //We have to presume something's wrong with this savedata record if it's too small.
                  //Corrupt or missing a file; either way it won't help us to be able to overwrite this file.
               }
            }
            else
            {
               //ignore this entry with apparently something wrong with it.  The MGS UI doesn't let you overwrite damaged files so
               //we can't count this toward the "existing savedata, could overwrite" footprint.
            }

         }//if( dirname prefix name match)

      }//for (startup check dirs)

      if( !remainingToCheck )
      {
         //No need to continue checking once we've found one valid case of each file.
         break;
      }
   }

   _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

void thr_space_check(uint64_t arg)
{
   int ret = 0;
   CellSaveDataSetList setList;
   CellSaveDataSetBuf setBuf;
   CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) arg;

   printf( "thr_space_check() start\n");

   //Total size required to get into the game:
   //Trophies
   //Savedata content size for all startup save types
   //Savedata system size for all startup save types
   pSS->mHDSizeReqKB = BP_TrophySystem_HDDNeededKB();
   for( int i=0; i < pSS->mNumMCStartupCheckDirs; ++i )
   {
      pSS->mHDSizeReqKB += pSS->mpMCStartupCheckDirs[i].mContentsSizeKB;
   }

   //At this point, we don't know the system size required or the free HDD space.
   //Run a fake autoload and retrieve the information in the CellSaveDataStatGet inside the callback to get this.
   {
      // Settings for general buffer
      setBuf.dirListMax  = 0;                                  // This member is not used (specify zero)
      setBuf.fileListMax = pSS->GetNumMCFiles();               // Maximum number of files listed up in the mData status callback
      memset( setBuf.reserved, 0x0, sizeof(setBuf.reserved) ); // The reserved member must be zero-filled

      // Size of buffer must be such that it can store mData of the size specified by the above maximums
      int const _saveDataFileDataBufferSize = pSS->GetNumMCFiles() * sizeof(CellSaveDataFileStat);
      void *_saveDataFileDataBuffer = alloca(_saveDataFileDataBufferSize);
      memset(_saveDataFileDataBuffer, 0, _saveDataFileDataBufferSize);

      setBuf.bufSize = _saveDataFileDataBufferSize;
      setBuf.buf = _saveDataFileDataBuffer;

      /* Execute auto load */
      ret = cellSaveDataAutoLoad2(
         CELL_SAVEDATA_VERSION_CURRENT,	   // Version of save mData format
         pSS->mSaveDirectoryRoot,            // Full directory mName for save mData
         CELL_SAVEDATA_ERRDIALOG_NONE,       // Settings for outputting error dialog
         &setBuf,				                  // CellSaveDataSetBuf
         cb_space_check_load,    			   // Data status callback function
         NULL,			                        // Not performing a file operation.
         SYS_MEMORY_CONTAINER_ID_INVALID,    // Memory container ID
         (void*)pSS             // Application-defined mData
         );

      printf("cellSaveDataAutoLoad2() : 0x%x\n", ret);
      _CMGS_SaveStatus_setResult(pSS, ret);
   }

   if( ret == CELL_OK )
   {
      //Start the directory search.
      setList.sortType      = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
      setList.sortOrder     = CELL_SAVEDATA_SORTORDER_DESCENT;
      setList.dirNamePrefix = pSS->mSaveDirectoryRoot;
      setList.reserved = NULL;

      setBuf.dirListMax  = 64 * 4;  //MGS2: 64 * 4; MGS3: 64+20+20+99 for max count of all savedata types
      setBuf.fileListMax = 0;
      memset( setBuf.reserved, 0x0, sizeof(setBuf.reserved) );
      setBuf.bufSize = setBuf.dirListMax * sizeof(CellSaveDataDirList);
      setBuf.buf = alloca(setBuf.bufSize);

      /* Execute auto save from a list */
      ret = cellSaveDataListAutoLoad(
         CELL_SAVEDATA_VERSION_CURRENT,	/* Version of save data format */
         CELL_SAVEDATA_ERRDIALOG_NONE, /* Settings for outputting error dialog */
         &setList,						/* Settings for obtaining save data list */
         &setBuf,						/* Settings of the general buffer */
         cb_space_check_list,				/* Fixed data callback function */
         NULL,			/* Data status callback function */
         NULL,			/* File operation callback function */
         SYS_MEMORY_CONTAINER_ID_INVALID,/* Memory container ID */
         (void*)pSS
         );

      printf("cellSaveDataListAutoLoad() : 0x%x\n", ret);

      _CMGS_SaveStatus_setResult(pSS, ret);

      if( ret == CELL_OK )
      {
         //Check and record whether we had enough space or not
         if( pSS->mHDSizeReqKB > pSS->mHDFreeSizeKB )
         {
            //Convert from "total size needed" to "additional amount needed".
            pSS->mHDSizeReqKB = pSS->mHDSizeReqKB - pSS->mHDFreeSizeKB;
            //The game will check MGS_SaveStatus_HasNoSpace() after this operation completes.
            pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoSpace;      
         }
         else
         {
            //No extra space needed.
            pSS->mHDSizeReqKB = 0;
         }
      }
   }
   else
   {
      //Error code returned from cellSaveDataAutoLoad2() step
   }

   printf( "thr_space_check() end\n");

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   sys_ppu_thread_exit(0);
}

int save_data_space_check( CMGS_SaveStatus * pSS )
{
   //Check for existence of game progress savedata.
   //Used on app startup to see if we're allowed to proceed into the game.

   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   // To prevent interrupting the processing of the main thread, save mData utility function must be called using a sub thread
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;

   sys_ppu_thread_t tid;
   int ret = sys_ppu_thread_create(&tid,
      thr_space_check, 
      (uint64_t) pSS,
      kSAVEDATA_THREAD_PRIORITY,
      kSAVEDATA_THREAD_STACKSIZE * 4,  //N.B. does a listing of *all* potential savedata on the stack, so increase stack size.
      0, "SPACE_CHECK");

   if (ret != 0) {
      printf("get list thread create failed %d\n", ret);
      return -1;
   }

   return 0;
}

//----------------------------------------------------------------------------

void cb_get_list( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataListGet *get, CellSaveDataFixedSet *set )
{
   unsigned int i;

   printf( "cb_get_list() start\n");

   /* Dump obtained list */
   dumpCellSaveDataListGet(get);

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;
   if( get->dirNum > get->dirListNum ) {
      /* This case can be handled in any way that is appropriate for the application's specifications */
      printf( "found too many datas. expected[%d] < found[%d]\n", get->dirListNum, get->dirNum );
   }

   //copy all of the found files into the result.
   BPE_ASSERT( get->dirListNum <= pSS->mGetDirListMaxEntries, "dir list max not set properly" );

   CellSaveDataDirStat dirStat;
   CellSaveDataSystemFileParam sysFileParam;
   unsigned int bind;
   int sizeKB;

   int getDirListExistEntries = 0;
   int dirListNum = bpe::min_val( (unsigned int)pSS->mGetDirListMaxEntries, get->dirListNum );
   for( int i=0; i < dirListNum; ++i )
   {
      int ret = cellSaveDataGetListItem( get->dirList[i].dirName, &dirStat, &sysFileParam, &bind, &sizeKB );
#if 0 //Simulate corrupt file when listing
      ret = CELL_SAVEDATA_ERROR_ACCESS_ERROR;
#endif
      if( ret == CELL_OK )
      {
         printf("cb_get_list(): OK [%d]: %s\n", getDirListExistEntries, get->dirList[i].dirName );
#if MGS_VERSION==2
         strncpy( (char*)pSS->mpGetDirTable[getDirListExistEntries].EntryName, get->dirList[i].dirName, sizeof( pSS->mpGetDirTable[getDirListExistEntries].EntryName ) - 1 );
         pSS->mpGetDirTable[getDirListExistEntries].AttrFile |= 0x0020; //sceMcFileAttrSubdir
         pSS->mpGetDirTable[getDirListExistEntries]._Modify = dirStat.st_mtime;
         pSS->mGetDirListExistEntries = ++getDirListExistEntries;
#else
         strncpy( (char*)pSS->mpGetDirTable2[i].name, get->dirList[i].dirName, sizeof( pSS->mpGetDirTable2[i].name ) - 1 );
         pSS->mpGetDirTable2[i].attribute |= 0x0020; //SCE_MC2_FILE_ATTR_SUBDIR
         //TODO: store date/time in correct format
         //pSS->mpGetDirTable2[i].modification = dirStat.st_mtime;
         (*pSS->mpGetDirListExistEntries) = ++getDirListExistEntries;
#endif
      }
      else
      {
         //ignore this entry.
         printf("cb_get_list(): ERROR 0x%08x: %s\n", ret,  get->dirList[i].dirName );
      }
   }

   //We're done with this sequence.
   _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

int save_data_get_dir_list(  CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   // To prevent interrupting the processing of the main thread, save mData utility function must be called using a sub thread
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;

   sys_ppu_thread_t tid;
   int ret = sys_ppu_thread_create(&tid,
      thr_get_list, 
      (uint64_t) pSS,
      kSAVEDATA_THREAD_PRIORITY, kSAVEDATA_THREAD_STACKSIZE,
      0, "GET_LIST");


   if (ret != 0) {
      printf("get list thread create failed %d\n", ret);
      return -1;
   }

   return 0;
}

void thr_get_list(uint64_t arg)
{
   int ret = 0;
   char dirNamePrefix[CELL_SAVEDATA_PREFIX_SIZE];
   CellSaveDataSetList setList;
   CellSaveDataSetBuf setBuf;
   CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) arg;

   printf( "thr_get_list() start\n");

#if MGS_VERSION==2
   memset( pSS->mpGetDirTable, 0, pSS->mGetDirListMaxEntries * sizeof( *pSS->mpGetDirTable ) );
#elif MGS_VERSION==3
   memset( pSS->mpGetDirTable2, 0, pSS->mGetDirListMaxEntries * sizeof( *pSS->mpGetDirTable2 ) );
#endif

   /* The save datas with the prefix specified here, */
   /* will be listed up in the fixed data callback.   */

   /* Settings for obtaining save data list */
   setList.sortType      = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
   setList.sortOrder     = CELL_SAVEDATA_SORTORDER_DESCENT;
   setList.dirNamePrefix = pSS->mSaveDirectory;
   setList.reserved = NULL;	/* The reserved member must be zero */

   /* Settings for general buffer */
   setBuf.dirListMax  = pSS->mGetDirListMaxEntries;				/* Maximum number of directories listed up in the fixed data callback */
   setBuf.fileListMax = 0;                            //we are not using the 'data status' step in this sequence.
   memset( setBuf.reserved, 0x0, sizeof(setBuf.reserved) );	/* The reserved member must be zero-filled */
   /* Size of buffer must be such that it can store data of the size specified by the above maximums */
   setBuf.bufSize = setBuf.dirListMax * sizeof(CellSaveDataDirList);
   setBuf.buf = alloca(setBuf.bufSize);

   /* Execute auto save from a list */
   ret = cellSaveDataListAutoLoad(
      CELL_SAVEDATA_VERSION_CURRENT,	/* Version of save data format */
      CELL_SAVEDATA_ERRDIALOG_NONE, /* Settings for outputting error dialog */
      &setList,						/* Settings for obtaining save data list */
      &setBuf,						/* Settings of the general buffer */
      cb_get_list,				/* Fixed data callback function */
      NULL,			/* Data status callback function */
      NULL,			/* File operation callback function */
      SYS_MEMORY_CONTAINER_ID_INVALID,/* Memory container ID */
      (void*)pSS
      );

   printf("cellSaveDataListAutoLoad() : 0x%x\n", ret);

   _CMGS_SaveStatus_setResult(pSS, ret);

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

   printf( "thr_get_list() end\n");

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   sys_ppu_thread_exit(0);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int save_data_auto_load( CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   //MGS specific issue: this "auto_load" operation in this case is supposed to load up just the "master file" which contains
   //a string used for in-game file listing.  Because the PS3 saveload API is atrociously slow, we can't actually do that and
   //get reasonable performance when there are multiple save records on the HDD.  Instead, there is an alternate path that
   //decodes the necessary information from the PARAM.SFO's detail and listParam strings which are relatively quick to retrieve.
#ifdef BP_PS3_STORE_MGS_SAVEDATA_INFO_IN_PARAM_SFO
   //N.B. the "mGetSubdirExactFilename" case is a hack added for the one case where they do a direct listing in MGS3
   //for the padding file named after the PS2 product code, after you save a photo in album view.  We don't want to do this
   //weird PARAM.SFO trick for this request.
   bool bLoadMasterFileFromParamSFO = ( pSS->mChosenGameSave==-1 ) && ( pSS->mGetSubdirExactFilename[0] == 0 );
#else
   bool bLoadMasterFileFromParamSFO = false;
#endif

   // To prevent interrupting the processing of the main thread, save mData utility function must be called using a sub thread
   pSS->mStatus = CMGS_SaveStatus::kS_Processing;

   sys_ppu_thread_t tid;
   int ret = sys_ppu_thread_create(&tid,
      bLoadMasterFileFromParamSFO ? thr_fake_auto_load_mgs_master : thr_auto_load, 
      (uint64_t) pSS,
   	kSAVEDATA_THREAD_PRIORITY, kSAVEDATA_THREAD_STACKSIZE,
   	0, "AUTO_LOAD");


   if (ret != 0) {
   	printf("auto load thread create failed %d\n", ret);
   	return -1;
   }

   return 0;
}

//----------------------------------------------------------------------------

void thr_auto_load(uint64_t arg)
{
	printf( "thr_auto_load() start\n");

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) arg;

	CellSaveDataSetBuf _cell_saveDataSetBuf;
	
	// Settings for general buffer
	_cell_saveDataSetBuf.dirListMax  = 0;										      // This member is not used (specify zero)
	_cell_saveDataSetBuf.fileListMax = pSS->GetNumMCFiles();						         // Maximum number of files listed up in the mData status callback
	memset( _cell_saveDataSetBuf.reserved, 0x0, sizeof(_cell_saveDataSetBuf.reserved) );	// The reserved member must be zero-filled

	// Size of buffer must be such that it can store mData of the size specified by the above maximums
   int const _saveDataFileDataBufferSize = pSS->GetNumMCFiles() * sizeof(CellSaveDataFileStat);
   void *_saveDataFileDataBuffer = alloca(_saveDataFileDataBufferSize);
   memset(_saveDataFileDataBuffer, 0, _saveDataFileDataBufferSize);

	_cell_saveDataSetBuf.bufSize = _saveDataFileDataBufferSize;
	_cell_saveDataSetBuf.buf = _saveDataFileDataBuffer;

	/* Execute auto load */
	int ret = cellSaveDataAutoLoad2(
					CELL_SAVEDATA_VERSION_CURRENT,	   // Version of save mData format
               pSS->mSaveDirectory,	               // Full directory mName for save mData
					CELL_SAVEDATA_ERRDIALOG_NONE,     // Settings for outputting error dialog
					&_cell_saveDataSetBuf,				   // CellSaveDataSetBuf
					callback_data_status_load,			         // Data status callback function
					callback_file_operation_load,			      // File operation callback function
					SYS_MEMORY_CONTAINER_ID_INVALID,    // Memory container ID
					(void*)pSS             // Application-defined mData
				);

	printf("cellSaveDataAutoLoad2() : 0x%x\n", ret);

   _CMGS_SaveStatus_setResult(pSS, ret);

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

	printf( "thr_auto_load() end\n");
	
   pSS->mStatus = CMGS_SaveStatus::kS_Done;

	sys_ppu_thread_exit(0);
}


//----------------------------------------------------------------------------
// Data status callback for cellSaveDataXXXXLoad()
void callback_data_status_load( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataStatGet *get, CellSaveDataStatSet *set )
{
   //printf( "->Load: DATA\n");
	
	// Dump obtained save mData status
	dumpCellSaveDataStatGet(get);

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

   if( !pSS->mWrongUserDetectDisabled )
   {
      if ( get->bind & (CELL_SAVEDATA_BINDSTAT_ERR_NOOWNER | CELL_SAVEDATA_BINDSTAT_ERR_OWNER) )
      {	// this save file is from the wrong user
         pSS->mWrongUser = true;
      }
      else
      {	// this save file is from the current user
	      pSS->mWrongUser = false;
      }
   }

	uint32_t hdSpaceReq = BP_TrophySystem_HDDNeededKB();
	if (get->isNewData)
	{
		hdSpaceReq += pSS->GetMCFilesTotalSizeInKB();
		hdSpaceReq += get->sysSizeKB;
	}
	if (hdSpaceReq > get->hddFreeSizeKB)
	{
     	// Not enough space for save
      pSS->mHDSizeReqKB = hdSpaceReq - get->hddFreeSizeKB;
     	_cellSaveDataResult->errNeedSizeKB = -pSS->mHDSizeReqKB;   //the docs say to set the value as the negative
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoSpace;      
   	printf("Save needs %d KB HDD space more. HDD KB free: %d.\n", -_cellSaveDataResult->errNeedSizeKB, get->hddFreeSizeKB);
		return;
	}


   if (get->isNewData)
   {
      // we have enough space to create a file, but there isn't one yet
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoData;
      return;
   }
   
	// We have save mData
   // Check to see if we've got the correct number of save files
#if 1 //change to #if 0 to quickly simulate "wrong number of files" case
	if ( get->fileListNum < get->fileNum )
#endif
   {
		// If number of file overflow, the save mData should be judged as broken mData
		// Because you may not find file needed in obtained list
		_cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
		return;
	}

   if (get->fileNum != pSS->GetNumMCFiles())
   {
      // Not enough files!
		_cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
		return;
   }

	// Validate file list
	for (unsigned int i = 0; i < get->fileListNum; i++ )
   {
      CellSaveDataFileStat const *pExistingFile = &get->fileList[i];

      // Check for matching hardcoded filename for our save
      SMCFile const *pMatchingSaveFile = pSS->FindMCFileInfoFromName(pExistingFile->fileName);
#if 1
      // During development, we don't want to call savegames corrupt
      // just because we mess with the icon file and it does not match what's packed into the executable.
      const bool bPassBecauseItsAnIconFile = pMatchingSaveFile != NULL && pMatchingSaveFile->mFileType == kMCFT_System;
#else
      // Ship mode - icon mismatch means corrupt!
      bool bPassBecauseItsAnIconFile = false;
#endif
#if MGS_VERSION==3
      const bool bPassBecauseItsAVariableSizePhoto = pMatchingSaveFile != NULL && pMatchingSaveFile->mInternalFileType == kISD_Photo;
#else
      const bool bPassBecauseItsAVariableSizePhoto = false;
#endif
      if (pMatchingSaveFile && (pExistingFile->st_size == (uint64_t)pMatchingSaveFile->mSize || bPassBecauseItsAnIconFile || bPassBecauseItsAVariableSizePhoto))
      {
         // Filename and size both match, we're good
         pSS->mMatchingFilesFound++;
         // Also update the 'modified' time
         ((SMCFile *)pMatchingSaveFile)->mFileModifiedTime = pExistingFile->st_mtime;

         ((SMCFile *)pMatchingSaveFile)->mGenericResult = pExistingFile->st_size;
      }
      else
      {
         // Name or size mismatch
         printf( "Corrupt: %s [%d]\n", pExistingFile->fileName, (int)pExistingFile->st_size);
         pSS->mUnmatchedFilesFound++;
      }
   }

	// Check to see if we've got all the correct files
   if (!pSS->IsFileIterationValid())
   {
		// Missing or mismatching files!
      printf( "Invalid Save: [%d / %d]\n", pSS->mMatchingFilesFound, pSS->mUnmatchedFilesFound);
		_cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
		return;
	}	

	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO_NOBROKEN;		// Do not handle save mData as corrupt mData.
   // Do not let save system prevent trophy unlocks under the hood.
   // We handle this ourselves, and also want to prevent trophy disabling when loading other users' photos for example.
   // Also, without this flag, the first wrong user load applies a *permanent* lockout to the trophy system (no way to
   // turn it back off) which conflicts with our clearing of the "wrong user" flag when the user returns to the main menu.
   set->reCreateMode |= CELL_SAVEDATA_DISABLE_TROPHY_OWNERSHIP_CHECK;
	set->setParam = NULL;		 								// PARAM.SFO won't be updated

	// Specify OK_NEXT to continue
	_cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

   //printf( "<-Load: DATA\n");
	return;
}

//----------------------------------------------------------------------------
// File operation callback for cellSaveDataXXXXLoad()
void callback_file_operation_load( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataFileGet *get, CellSaveDataFileSet *set )
{
   //printf( "->Load: FILE\n");

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

   // We want to load our master file and the indicated game index and
   // skip over files that don't need loading

   // Assume we're on last file, will get overridden below if files remaining.
   _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;

   // Check if we've processed all the files
   {
      bool bBreak = false;
      while (!bBreak && (pSS->mCurrentMCFileIndex < pSS->GetNumMCFiles()))
      {      
         // Do we need to load this file?
         // TODO: Might need to 'load' files so the save sysutil can check if they've been modified
         if (pSS->ShouldReadFile(pSS->mCurrentMCFileIndex))
         {
            // Yep, need to read this file index
            SMCFile const *pMCFile = &pSS->GetMCFiles()[pSS->mCurrentMCFileIndex];

		      set->fileType = pSS->Get_CELL_SAVEDATA_FILETYPE(pSS->mCurrentMCFileIndex);

            // Set appropriate structures based on file type.
            if (set->fileType == CELL_SAVEDATA_FILETYPE_SECUREFILE)
            {
               // Set secure file encryption key
               memcpy(set->secureFileId, sSecureFileId, sizeof(sSecureFileId));
               set->fileName = (char *) pMCFile->mName;
            }
            else if (set->fileType == CELL_SAVEDATA_FILETYPE_NORMALFILE)
            {
               set->fileName = (char *) pMCFile->mName;
            }

		      set->fileOperation = CELL_SAVEDATA_FILEOP_READ;
		      set->fileBuf = pMCFile->mData;
		      set->fileBufSize = pMCFile->mSize;
		      set->fileSize = pMCFile->mSize;
		      set->fileOffset = 0;
		      set->reserved = NULL;

            printf( "LOADING: %s\n", pMCFile->mName);
            _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
            bBreak = true;
         }

         // We don't need to load this file, move to the next one.
         pSS->mCurrentMCFileIndex++;
      }
   }
	
   //printf( "<-Load: FILE\n");
	return;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int save_data_auto_recreate_save( CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   BPE_ASSERT_NO_MSG( pSS->mpMCFiles[0].mFileType == kMCFT_SaveGame ); //make sure no ordering error!
   if (pSS->mWrongUser && ( pSS->mpMCFiles[0].mInternalFileType != kISD_Photo ) )
   {
      // handle the wrong user scenario here, so that we immediately fail if this save game mData cannot be saved.
      // Photos are the exception-- do not prevent the user from overwriting wrong user photos in album view
      // or taking pictures in-game from another user's save.

      pSS->mStatus = CMGS_SaveStatus::kS_Done;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;  // indicate no real failure here.

      //This callback is normally called by the thread that we're not starting.
      if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
      {
         callback( pSS );
      }
   }
   else
   {
      // To prevent interrupting the processing of the main thread, save mData utility function must be called using a sub thread
      pSS->mStatus = CMGS_SaveStatus::kS_Processing;
      OsContext()->SetUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );  // don't allow the user to interrupt this

	   sys_ppu_thread_t tid;
	   int ret = sys_ppu_thread_create(&tid,
		   thr_auto_recreate_save, 
         (uint64_t) pSS,
		   kSAVEDATA_THREAD_PRIORITY, kSAVEDATA_THREAD_STACKSIZE,
		   0, "AUTO_RC_SAVE");


	   if (ret != 0) {
		   printf("auto recreate save thread create failed %d\n", ret);
		   return -1;
	   }
   }

	return 0;
}

//----------------------------------------------------------------------------

void thr_auto_recreate_save(uint64_t arg)
{
	printf( "thr_auto_recreate_save() start\n");

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) arg;

	// Settings for general buffer
	CellSaveDataSetBuf _cell_saveDataSetBuf;
	_cell_saveDataSetBuf.dirListMax  = 0;										      // This member is not used (specify zero)
	_cell_saveDataSetBuf.fileListMax = pSS->GetNumMCFiles();						         // Maximum number of files listed up in the mData status callback
	memset( _cell_saveDataSetBuf.reserved, 0x0, sizeof(_cell_saveDataSetBuf.reserved) );	// The reserved member must be zero-filled

	// Size of buffer must be such that it can store mData of the size specified by the above maximums
   int const _saveDataFileDataBufferSize = pSS->GetNumMCFiles() * sizeof(CellSaveDataFileStat);
   void *_saveDataFileDataBuffer = alloca(_saveDataFileDataBufferSize);
   memset(_saveDataFileDataBuffer, 0, _saveDataFileDataBufferSize);

	_cell_saveDataSetBuf.bufSize = _saveDataFileDataBufferSize;
	_cell_saveDataSetBuf.buf = _saveDataFileDataBuffer;

	// Execute auto save
	int ret = cellSaveDataAutoSave2(
					CELL_SAVEDATA_VERSION_CURRENT,	   // Version of save mData format
               pSS->mSaveDirectory,	               // Full directory mName for save mData
					CELL_SAVEDATA_ERRDIALOG_NONE,     // Settings for outputting error dialog
					&_cell_saveDataSetBuf,				   // CellSaveDataSetBuf
					callback_data_status_recreate_save,			         // Data status callback function
					callback_file_operation_recreate_save,			      // File operation callback function
					SYS_MEMORY_CONTAINER_ID_INVALID,    // Memory container ID
					(void*)pSS                     // Application-defined mData
				);


	printf("cellSaveDataAutoSave2() : 0x%x\n", ret);

   _CMGS_SaveStatus_setResult(pSS, ret);

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

   printf( "thr_auto_recreate_save() end\n");

   if( (CELL_SAVEDATA_RET_OK == ret) && (pSS->mChosenGameSave >= 0) )
   {  // if the save worked, update the file modification time.
      SMCFile const *pMCFile = &pSS->GetMCFiles()[pSS->mChosenGameSave];
		((SMCFile *)pMCFile)->mFileModifiedTime = gFileModTime;
   }

   pSS->mStatus = CMGS_SaveStatus::kS_Done;
   OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );   // the critical process is complete.. you may shut down now

	sys_ppu_thread_exit(0);
}

//----------------------------------------------------------------------------

void cb_fake_auto_load_mgs_master( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataListGet *get, CellSaveDataFixedSet *set )
{
   printf( "cb_fake_auto_load_mgs_master() start\n");

   /* Dump obtained list */
   dumpCellSaveDataListGet(get);

   //This process is run on a specific directory whose exact name was retrieved from the cellSaveDataAPI
   //so there should be exactly one valid result.
   if( get->dirListNum == 1 )
   {
      CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

      CellSaveDataDirStat dirStat;
      CellSaveDataSystemFileParam sysFileParam;
      unsigned int bind;
      int sizeKB;

      BPE_ASSERT_NO_MSG( pSS->mpMCFiles[1].mFileType == kMCFT_Master ); //make sure no ordering error!
      BPE_ASSERT_NO_MSG( pSS->mpMCFiles[0].mFileType == kMCFT_SaveGame ); //make sure no ordering error!

      int ret = cellSaveDataGetListItem( get->dirList[0].dirName, &dirStat, &sysFileParam, &bind, &sizeKB );
      printf("cellSaveDataGetListItem() : ret 0x%x, bind 0x%08x\n", ret, bind);
#if 0 //Simulate corrupt file when fake loading master
      if( strstr( get->dirList[0].dirName, "G000002" ) )
      {
         ret = CELL_SAVEDATA_ERROR_ACCESS_ERROR;
      }
#endif
      if( ret == CELL_OK )
      {
         //Get the base-64 string from the directory information.  It goes in the contents of the master file.
         BP_Decode_PS3_FileParam( pSS->mSaveDirectory+strlen(pSS->mSaveDirectoryRoot), (char *)pSS->mpMCFiles[1].mData, sysFileParam.detail, sysFileParam.listParam, bind );
         //We're done with this sequence.
         _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
         SMCFile* pSaveGame = (SMCFile*)&pSS->mpMCFiles[0];
         pSaveGame->mFileModifiedTime = dirStat.st_mtime;
         printf("OK: %s\n", pSS->mSaveDirectory );
      }
      else
      {
         //Cause a decode error in MC_DecodeDataName() by zeroing out the "filename".  It won't even get to the
         //crc check that is ignored on PS3 because of the invalid character.
         //(makes this item show up grey and unselectable for load or overwrite)
         memset( pSS->mpMCFiles[1].mData, 0, pSS->mpMCFiles[1].mSize );
         _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
         printf("ERROR: %s\n", pSS->mSaveDirectory );
      }
   }
   else
   {
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_INVALID;
   }

}

#if MGS_VERSION==3

//The sole purpose of this callback is to get the variable size of this file since we can't just set it to what we expect
//like we do for other files when we do a fake master file load.
void cb_data_status_load_mgs3_photo( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataStatGet *get, CellSaveDataStatSet *set )
{
   //printf( "->Load: DATA\n");

   // Dump obtained save mData status
//   dumpCellSaveDataStatGet(get);

   CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

   if (get->isNewData)
   {
      // we have enough space to create a file, but there isn't one yet
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoData;
      return;
   }

   // We have save mData
   // Check to see if we've got the correct number of save files
   if ( get->fileListNum < get->fileNum )
   {
      // If number of file overflow, the save mData should be judged as broken mData
      // Because you may not find file needed in obtained list
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
      return;
   }

   if (get->fileNum != pSS->GetNumMCFiles())
   {
      // Not enough files!
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
      return;
   }

   // Validate file list
   for (unsigned int i = 0; i < get->fileListNum; i++ )
   {
      CellSaveDataFileStat const *pExistingFile = &get->fileList[i];

      // Check for matching hardcoded filename for our save
      SMCFile const *pMatchingSaveFile = pSS->FindMCFileInfoFromName(pExistingFile->fileName);

      if( pMatchingSaveFile != NULL && pMatchingSaveFile->mInternalFileType == kISD_Photo )
      {
         // Filename and size both match, we're good
         ((SMCFile *)pMatchingSaveFile)->mGenericResult = pExistingFile->st_size;
         // Done!
         _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
         return;
      }
   }

   // Not much we can do if we get here!  File is apparently missing.
   printf( "Invalid photo data: %s\n", pSS->mSaveDirectory );
   _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
   pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
}

#endif

//Special function to pretend that we're loading the MASTER.BIN of a MGS savedata record, but instead get the directory
//info and build the contents of the intended file (a base-64 string) from the detail and listParam structures with the
//help of an MGS2/MGS3-specific helper function.
void thr_fake_auto_load_mgs_master(uint64_t arg)
{
   int ret = 0;
   char dirNamePrefix[CELL_SAVEDATA_PREFIX_SIZE];
   CellSaveDataSetList setList;
   CellSaveDataSetBuf setBuf;
   CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) arg;

   printf( "thr_fake_auto_load_mgs_master() start\n");

   //Start a cellSaveDataListAutoLoad process because the cell API is only lets you call
   //cellSaveDataGetListItem from inside a callback from that function.

   /* Settings for obtaining save data list */
   setList.sortType      = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
   setList.sortOrder     = CELL_SAVEDATA_SORTORDER_DESCENT;
   setList.dirNamePrefix = pSS->mSaveDirectory;
   setList.reserved = NULL;	/* The reserved member must be zero */

   setBuf.dirListMax  = 1;
   setBuf.fileListMax = 0;
   memset( setBuf.reserved, 0x0, sizeof(setBuf.reserved) );	/* The reserved member must be zero-filled */
   /* Size of buffer must be such that it can store data of the size specified by the above maximums */
   setBuf.bufSize = setBuf.dirListMax * sizeof(CellSaveDataDirList);
   setBuf.buf = alloca(setBuf.bufSize);

   /* Execute auto save from a list */
   ret = cellSaveDataListAutoLoad(
      CELL_SAVEDATA_VERSION_CURRENT,	/* Version of save data format */
      CELL_SAVEDATA_ERRDIALOG_NONE, /* Settings for outputting error dialog */
      &setList,						/* Settings for obtaining save data list */
      &setBuf,						/* Settings of the general buffer */
      cb_fake_auto_load_mgs_master,				/* Fixed data callback function */
      NULL,			/* Data status callback function */
      NULL,			/* File operation callback function */
      SYS_MEMORY_CONTAINER_ID_INVALID,/* Memory container ID */
      (void*)pSS
      );

   printf("cellSaveDataListAutoLoad() : 0x%x\n", ret);

   _CMGS_SaveStatus_setResult(pSS, ret);

#if MGS_VERSION==3
   if( ret == 0 )
   {
      //Additional step needed to support variable-sized MGS3 photos.
      //The only way to find out the file size of the DATA.BIN file is to perform an autoload on it.
      BPE_ASSERT_NO_MSG( pSS->mpMCFiles[0].mFileType == kMCFT_SaveGame ); //make sure no ordering error!
      if( pSS->mpMCFiles[0].mInternalFileType == kISD_Photo )
      {
         CellSaveDataSetBuf _cell_saveDataSetBuf;

         // Settings for general buffer
         _cell_saveDataSetBuf.dirListMax  = 0;										      // This member is not used (specify zero)
         _cell_saveDataSetBuf.fileListMax = pSS->GetNumMCFiles();						         // Maximum number of files listed up in the mData status callback
         memset( _cell_saveDataSetBuf.reserved, 0x0, sizeof(_cell_saveDataSetBuf.reserved) );	// The reserved member must be zero-filled

         // Size of buffer must be such that it can store mData of the size specified by the above maximums
         int const _saveDataFileDataBufferSize = pSS->GetNumMCFiles() * sizeof(CellSaveDataFileStat);
         void *_saveDataFileDataBuffer = alloca(_saveDataFileDataBufferSize);
         memset(_saveDataFileDataBuffer, 0, _saveDataFileDataBufferSize);

         _cell_saveDataSetBuf.bufSize = _saveDataFileDataBufferSize;
         _cell_saveDataSetBuf.buf = _saveDataFileDataBuffer;

         /* Execute auto load */
         int ret = cellSaveDataAutoLoad2(
            CELL_SAVEDATA_VERSION_CURRENT,	   // Version of save mData format
            pSS->mSaveDirectory,	               // Full directory mName for save mData
            CELL_SAVEDATA_ERRDIALOG_NONE,     // Settings for outputting error dialog
            &_cell_saveDataSetBuf,				   // CellSaveDataSetBuf
            cb_data_status_load_mgs3_photo,			         // Data status callback function
            NULL,			      // File operation callback function
            SYS_MEMORY_CONTAINER_ID_INVALID,    // Memory container ID
            (void*)pSS             // Application-defined mData
            );

         _CMGS_SaveStatus_setResult(pSS, ret);
      }
   }
#endif

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

   printf( "thr_fake_auto_load_mgs_master() end\n");

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   sys_ppu_thread_exit(0);
}

//----------------------------------------------------------------------------
// Data status callback for cellSaveDataXXXXSave()
void callback_data_status_recreate_save( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataStatGet *get, CellSaveDataStatSet *set )
{
   //printf( "->RecreateSave: DATA callback\n");

	// Dump obtained save mData status
	dumpCellSaveDataStatGet(get);

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

   // Delete existing savedata *and* reset owner.  Otherwise, in a multiple savedata system, you can overwrite another user's data
   // and the previous owner information persists.
   set->reCreateMode = CELL_SAVEDATA_RECREATE_YES_RESET_OWNER;
	set->setParam = &get->getParam; 							// You can specify get->getParam to set->setParam directly

	uint32_t hdSpaceReq = BP_TrophySystem_HDDNeededKB();
	if (get->isNewData)
	{
		hdSpaceReq += pSS->GetMCFilesTotalSizeInKB();
		hdSpaceReq += get->sysSizeKB;
	}
	if (hdSpaceReq > get->hddFreeSizeKB)
	{
      pSS->mHDSizeReqKB = hdSpaceReq - get->hddFreeSizeKB;
      _cellSaveDataResult->errNeedSizeKB = -pSS->mHDSizeReqKB;   //the docs say to set the value as the negative
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoSpace;
      printf("Save needs %d KB HDD space more. HDD KB free: %d.\n", -_cellSaveDataResult->errNeedSizeKB, get->hddFreeSizeKB);
	}
	else
	{
      if (!get->isNewData)
      {
         // Save data exists
         printf("Recreating existing save mData %s.\n", get->dir.dirName );
      }
      else
      {
         printf("Save data %s DOES NOT exist - Recreating\n", get->dir.dirName );
      }

      _CMGS_SaveStatus_setParamSFO( pSS, set );

      // We've got enough free space on the HDD
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
   }

   //printf( "<-RecreateSave: DATA callback\n");
   return;
}

//----------------------------------------------------------------------------
// File operation callback for cellSaveDataXXXXSave()
void callback_file_operation_recreate_save( CellSaveDataCBResult *_cellSaveDataResult, CellSaveDataFileGet *get, CellSaveDataFileSet *set )
{
	(void)get;

   //printf( "->RecreateSave: FILE callback\n");

	CMGS_SaveStatus * pSS = (CMGS_SaveStatus*) _cellSaveDataResult->userdata;

   // Check if we've processed all the save files (can't write last file with OK_LAST)
   if (pSS->mCurrentMCFileIndex >= pSS->GetNumMCFiles())
   {
      // Done, don't show a confirmation dialog
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
   }
   else
   {
      // Still got some files to process, keep going
      _cellSaveDataResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

      // Write all files
      SMCFile const *pMCFile = &pSS->GetMCFiles()[pSS->mCurrentMCFileIndex];

	   set->fileType = pSS->Get_CELL_SAVEDATA_FILETYPE(pSS->mCurrentMCFileIndex);

      // Set appropriate structures based on file type.
      if (set->fileType == CELL_SAVEDATA_FILETYPE_SECUREFILE)
      {
         // Set secure file encryption key
         memcpy(set->secureFileId, sSecureFileId, sizeof(sSecureFileId));
         set->fileName = (char *) pMCFile->mName;
      }
      else if (set->fileType == CELL_SAVEDATA_FILETYPE_NORMALFILE)
      {
         set->fileName = (char *) pMCFile->mName;
      }

	   set->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
	   set->fileBuf = pMCFile->mData;
	   set->fileBufSize = pMCFile->mSize;
	   set->fileSize = pMCFile->mSize;
	   set->fileOffset = 0;
	   set->reserved = NULL;

      printf( "SAVING: %s\n", pMCFile->mName);
   }

   pSS->mCurrentMCFileIndex++;

   //printf(   "<-RecreateSave: FILE callback\n");
	return;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int save_data_get_info( CMGS_SaveStatus * pSS )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   //No operation on PS3.
   pSS->mDeviceId = 1;  //arbitrary nonzero device id to prevent "no device" result
   _CMGS_SaveStatus_setResult(pSS, 0);
   pSS->mStatus = CMGS_SaveStatus::kS_Done;
}

//----------------------------------------------------------------------------

