//----------------------------------------------------------------------------
// BP_SaveLoad.h
// BP adapted from CP3
//
// Utility functions for saving/loading game data.
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#if MGS_VERSION==2

#define BP_LINKVAR_COOKIE 'MGS2'
#define BP_LINKVAR_VERSION 6
#define BP_FIRST_LINKVAR_VERSION_WITH_TROPHY_VALID_FLAG 2
#define BP_FIRST_LINKVAR_VERSION_WITH_TROPHY_DELTA 3
#define BP_FIRST_LINKVAR_VERSION_WITH_ENCRYPTED_PSN_ID 6

#elif MGS_VERSION==3

#define BP_LINKVAR_COOKIE 'MGS3'
#define BP_LINKVAR_VERSION 6
#define BP_FIRST_LINKVAR_VERSION_WITH_TROPHY_VALID_FLAG 2
#define BP_FIRST_LINKVAR_VERSION_WITH_TROPHY_DELTA 3
#define BP_FIRST_LINKVAR_VERSION_WITH_ENCRYPTED_PSN_ID 6

#endif

#ifndef _PS3

typedef enum {									
	CELL_SAVEDATA_FILETYPE_SECUREFILE = 0,
	CELL_SAVEDATA_FILETYPE_NORMALFILE,			
	CELL_SAVEDATA_FILETYPE_CONTENT_ICON0,		
	CELL_SAVEDATA_FILETYPE_CONTENT_ICON1,		
	CELL_SAVEDATA_FILETYPE_CONTENT_PIC1,		
	CELL_SAVEDATA_FILETYPE_CONTENT_SND0			
} CellSaveDataFileType;

#else

#include <sysutil\sysutil_savedata.h>

#endif

#ifdef SN_TARGET_PSP2
#  include <time.h>
#endif

#if MGS_VERSION==2

typedef	struct sceMcTblGetDir {
   //   FILETIME _Create;
   time_t _Modify;
   unsigned FileSizeByte;
   unsigned short AttrFile;
   unsigned short Reserve1;
   unsigned Reserve2;
   unsigned PdaAplNo;
   unsigned char EntryName[32];
} sceMcTblGetDir;

#elif MGS_VERSION==3

#include "libmc2.h"

#endif

//----------------------------------------------------------------------------

//The types of save data in MGS.
typedef enum
{
   kISD_Game,        //main game savedata
   kISD_VR,          //vr missions
   kISD_SnakeTales,  //snake tales
   kISD_Photo,       //photo from digital camera
   kISD_MG1,         //MG1 savedata
   kISD_MG2,         //MG2 savedata

   kISD_Count
} EInternalSaveData;

enum ESaveGameString
{
   kSGS_Title,
   kSGS_Subtitle,

   kSGS_Count
};

#if MGS_VERSION==3
typedef enum
{
   kSGT_MGS3,
   kSGT_MG1,
   kSGT_MG2,
   
   kSGT_Count
} ESaveGameTitle;
#endif

enum
{
   kInvalidAccessTime = -1
};

typedef enum
{
   kMCFT_SaveGame,
   kMCFT_Master,
   kMCFT_System,

   kMCFT_Count
} EMCFileType;

typedef struct _SMCFile
{
   const char *      mName;
   void *            mData;
   int               mSize;
   EMCFileType       mFileType;
   time_t            mFileModifiedTime;

   EInternalSaveData mInternalFileType; // EInternalSaveData

   //fake file I/O interface
   unsigned int      mOpenFileLoc;
   volatile int      mGenericResult;
} SMCFile;

//Support for checking the PS3 HDD for presence of multiple types of savedata on startup
//to ensure that the user can proceed by either having space or having a savedata already
//that can be overwritten.
typedef struct _SMCStartupCheckDir
{
   const char *      mDirNamePrefix;
   int               mContentsSizeKB;   //expected size of files inside, excluding sys data.
} SMCStartupCheckDir;

//----------------------------------------------------------------------------

#ifdef __cplusplus

namespace BPLinkvarDefines
{
   enum EConsoleTypes
   {
      kConsoleInvalid = 0,
      kConsolePS3 = 1,
      kConsoleVita = 2,
      kConsoleLegacyPS3 = 3,
   };

   const unsigned char kTrophiesValid = 0;
   const unsigned char kTrophiesInvalid = 0xFF; // Anything besides 0 means invalid, but this is what we'll copy into the linkvars
};

class CMGS_SaveStatus
{
public:
   enum EStatus
   {
      kS_Init,
      kS_Processing,
      kS_Done,

      kS_Count
   };

   enum ECallbackResult
   {
      kCB_OK,
      kCB_NoData,
      kCB_NoSpace,
      kCB_Corrupted,
      kCB_CELL_Error, // Check 'mCELL_result'

      kCB_Count
   };

public:

   CMGS_SaveStatus()
   {
      mpMCFiles = NULL;
      mNumMCFiles = 0;
      mpMCStartupCheckDirs = NULL;
      mNumMCStartupCheckDirs = 0;

      memset( mSaveDirectoryRoot, 0, sizeof( mSaveDirectoryRoot ) );
      memset( mSaveDirectory, 0, sizeof( mSaveDirectory ) );
      memset( mGetSubdirExactFilename, 0, sizeof(mGetSubdirExactFilename) );

      mGetDirListMaxEntries = 0;
#if MGS_VERSION==2
      mGetDirListExistEntries = 0;
      mpGetDirTable = NULL;
#elif MGS_VERSION==3
      mpGetDirTable2 = NULL;
      mpGetDirListExistEntries = NULL;
#endif

      mRecreatingDeletedSaveData = 0;

      mWrongUser = false;  // this can only be set once, and once the user is wrong, he's always wrong.
      mbDeviceSelected = false;  //this will be set to true once.

      mDeviceId = 0;       //XCONTENTDEVICE_ANY
      ResetStatus();
   }

   void ResetAll(SMCFile const *pMCFiles, int const numMCFiles, SMCStartupCheckDir const * pMCStartupCheckDirs, int const numMCStartupCheckDirs )
   {
      mpMCFiles = pMCFiles;
      mNumMCFiles = numMCFiles;

      mpMCStartupCheckDirs = pMCStartupCheckDirs;
      mNumMCStartupCheckDirs = numMCStartupCheckDirs;

      // Check that first file is a save game (assumes contiguous game order)
      assert(mpMCFiles[0].mFileType == kMCFT_SaveGame);
      ResetStatus();
   }

   void ResetStatus()
   {
      mMatchingFilesFound = 0;
      mUnmatchedFilesFound = 0;
      mCurrentMCFileIndex = 0;
      mChosenGameSave = -1;
      mStatus = kS_Init;
      mCallbackResult = kCB_NoData;
      mCELL_Result = 0;
      mHDSizeReqKB = 0;
      mSysSizePerDirReqKB = 0;
      mHDFreeSizeKB = 0;

      mWrongUserDetectDisabled = 0;  //intended to be only temporarily disabled.

      mpReadWriteBuf = NULL;
      mReadWriteSize = 0;

      mpAsyncOpCallback = NULL;
   }

   // Returns 'true' while save/load thread is working.
   bool IsDone() const
   {
      bool const bFinished = (mStatus == kS_Done);
      return bFinished;
   }

   // Call after 'IsDone'
   bool CompletedOK() const
   {      
      bool const bOK = (mCallbackResult == kCB_OK);
      return bOK;
   }

   bool IsFileIterationValid() const
   {
      if (mUnmatchedFilesFound == 0)
      {
         if (mMatchingFilesFound == GetNumMCFiles())
         {
            // File counts, names and sizes match
            return true;
         }
      }
      // Mismatch on file counts
      return false;
   }

   // Only valid in File save callbacks.
   // Let user handle file index counter.
   bool ShouldWriteFile(int currentMCFileIndex) const
   {
      if (currentMCFileIndex >= GetNumMCFiles())
      {
         // Out of range
         return false;
      }

      // Check to see if we need to write all files.
      if (IsFileIterationValid())
      {
         SMCFile const *pFileInfo = &mpMCFiles[currentMCFileIndex];
         // A valid save already exists.
         // Only need to write the master and one game save
         if (pFileInfo->mFileType == kMCFT_Master)
         {
            return true;
         }
         int const activeGameSlotIndex = mChosenGameSave;
         if (currentMCFileIndex == activeGameSlotIndex)
         {
            return true;
         }
         // Don't need to write this file as it's not changed.
         return false;
      }

      // It's a new save, create all files.

#ifdef SN_TARGET_PSP2
      // Don't write icons on Vita, they're not needed and take up a ton of space
      if( currentMCFileIndex == kMCFT_System ) return false;
#endif

      return true;
   }

   // Only valid in File read callbacks.
   // Let user handle file index counter.
   bool ShouldReadFile(int currentMCFileIndex) const
   {
      if (currentMCFileIndex >= GetNumMCFiles())
      {
         // Out of range
         return false;
      }

      // Read master
      SMCFile const *pFileInfo = &mpMCFiles[currentMCFileIndex];
      if (pFileInfo->mFileType == kMCFT_Master)      
      {
         return true;
      }
      // Read indicated game
      int const activeGameSlotIndex = mChosenGameSave;
      if (currentMCFileIndex == activeGameSlotIndex)
      {
         return true;
      }
      // Don't need to read this file
      return false;
  }


   int Get_CELL_SAVEDATA_FILETYPE(int currentMCFileIndex) const
   {
      SMCFile const *pFileInfo = &mpMCFiles[currentMCFileIndex];
      if (pFileInfo->mFileType == kMCFT_System)
      {
         // Must be the icon file
         assert(strcmp(pFileInfo->mName, "ICON0.PNG") == 0);
         return CELL_SAVEDATA_FILETYPE_CONTENT_ICON0;
      }

      // Otherwise write as a protected file
      return CELL_SAVEDATA_FILETYPE_SECUREFILE;
   }

   SMCFile const * GetMCFiles() const
   {
      return mpMCFiles;
   }

   int GetNumMCFiles() const
   {
      return mNumMCFiles;
   }

   unsigned int GetMCFilesTotalSize() const
   {
      unsigned int sumBytes = 0;
      for (int  i = 0; i < GetNumMCFiles(); i++)
      {
         SMCFile const *pFileInfo = &mpMCFiles[i];
         sumBytes += pFileInfo->mSize;
      }

      return sumBytes;
   }

   unsigned int GetMCFilesTotalSizeInKB() const
   {
      unsigned int sumBytes = 0;
      for (int  i = 0; i < GetNumMCFiles(); i++)
      {
         SMCFile const *pFileInfo = &mpMCFiles[i];
         sumBytes += (pFileInfo->mSize + 1023) / 1024;
      }

      return sumBytes;
   }

   unsigned int GetMCFilesTotalSizeExcludeSystem() const
   {
      unsigned int sumBytes = 0;
      for (int  i = 0; i < GetNumMCFiles(); i++)
      {
         SMCFile const *pFileInfo = &mpMCFiles[i];
         if( pFileInfo->mFileType != kMCFT_System )
         {
            sumBytes += pFileInfo->mSize;
         }
      }

      return sumBytes;
   }

   SMCFile const * FindMCFileInfoFromName(char const * const pFilename) const
   {
      for (int  i = 0; i < GetNumMCFiles(); i++)
      {
         SMCFile const *pFileInfo = &mpMCFiles[i];
         if (strcmp(pFilename, pFileInfo->mName) == 0)
         {
            return pFileInfo;
         }
      }
      return NULL;
   }

   void SetChosenGameSaveDirSuffix( const char * const dir )
   {
      char saveDirectory[FILENAME_MAX];
      strcpy( saveDirectory, mSaveDirectoryRoot );
      strcat( saveDirectory, dir );
      assert( strlen( saveDirectory ) < sizeof( mSaveDirectory ) );
      strncpy( mSaveDirectory, saveDirectory, sizeof( mSaveDirectory ) - 1 );
   }

   SMCFile const *            mpMCFiles;
   int                        mNumMCFiles;

   SMCStartupCheckDir const * mpMCStartupCheckDirs;
   int                        mNumMCStartupCheckDirs;

   volatile EStatus           mStatus;
   volatile ECallbackResult   mCallbackResult;
   volatile int               mCELL_Result;
   char                       mGetSubdirExactFilename[32];

   int                        mChosenGameSave;
   int                        mCurrentMCFileIndex;
   char                       mSaveDirectoryRoot[32];    //shared among all savegames for this game
   char                       mSaveDirectory[32];        //full path, including different suffix for each savegame

   int                        mMatchingFilesFound;
   int                        mUnmatchedFilesFound;
   int                        mHDSizeReqKB;
   int                        mSysSizePerDirReqKB;      //system size, per save record, required.
   int                        mHDFreeSizeKB;            //cached from last callback.
   int                        mWrongUser;
   int                        mWrongUserDetectDisabled;   //support for disabling wrong user detection on auto_load.
   int                        mRecreatingDeletedSaveData;//flag whether we received and ignored a delete and are about to recreate
   unsigned int               mDeviceId;
   //TEMP: allow device selection on XBOX360 only once until we have switching support.
   //We'll need some sort of explicit reset for this whenever the player re-enters a load / save screen.
   bool                       mbDeviceSelected;

   //Storage for getting directory list
   int                        mGetDirListMaxEntries;
#if MGS_VERSION==2
   int                        mGetDirListExistEntries;
   sceMcTblGetDir *           mpGetDirTable;
#else
   SceMc2DirParam *           mpGetDirTable2;
   int *                      mpGetDirListExistEntries;
#endif

   void *                     mpReadWriteBuf;
   int                        mReadWriteSize;

   void *                     mpAsyncOpCallback;
};

void save_data_init( const char * const saveDirectoryRoot );

typedef void (*save_data_async_op_callback)( CMGS_SaveStatus * pSS );

//Sees if there's space for the specified savedata (if no savedata of this type already on HDD)
//+ trophies (if trophies not already installed).
int save_data_space_check( CMGS_SaveStatus * pSS );

int save_data_get_dir_list( CMGS_SaveStatus * pSS, save_data_async_op_callback callback );

// CMGS_SaveStatus data must be valid during save/load process.
int save_data_auto_save( CMGS_SaveStatus * pSS, save_data_async_op_callback callback );
int save_data_auto_load( CMGS_SaveStatus * pSS, save_data_async_op_callback callback );
int save_data_auto_delete( CMGS_SaveStatus * pSS );

// WARNING: Deletes and recreates existing save data, to be used with corrupted save data.
int save_data_auto_recreate_save( CMGS_SaveStatus * pSS, save_data_async_op_callback callback );

int save_data_get_info( CMGS_SaveStatus * pSS );

//shared among platforms.
char const * save_data_get_save_game_localized_string(ESaveGameString const string, EInternalSaveData const whichSaveData);

#if MGS_VERSION==3
ESaveGameTitle get_save_game_title();
#endif

#endif//#ifdef __cplusplus

//------------------------------------------------------------------------------------------
