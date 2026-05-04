//------------------------------------------------------------------------------------------
// BP_SaveLoadX360.cpp
// BP adapted from CP4
//
// Utility functions for saving/loading on XBox360.
//------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "BP_SaveLoad.h"

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CStopWatch.h"

#include "MGS_SysCommon.h"

#ifdef GOLD_VERSION
#define printf(...)
#endif

extern "C"
{
   int gBP_CanSelectNewDevice = 1;
   int gBP_ShowDeviceInvalidUI = 1;
}

extern "C" void ShowDeviceSelector_Sync(unsigned int requiredSizeForNewSave);
extern "C" void ShowGenericOsDialog_Sync(unsigned int dialogType/*EDialogRequestType*/);

//----------------------------------------------------------------------------

static const int skSaveLoadThreadProcessor = 5;

//arbitrary drive label for opened XContent
#define BP_XCONTENT_SAVEDATA_ROOT "save"

//----------------------------------------------------------------------------

#if MGS_VERSION==2
static WCHAR const skSGS_Title_NA[] = L"METAL GEAR SOLID 2";
static WCHAR const skSGS_Title_EU[] = L"METAL GEAR SOLID 2";
static WCHAR const skSGS_Title_J[]  = L"METAL GEAR SOLID 2";
#elif MGS_VERSION==3
static WCHAR const skSGS_Title_NA[] = L"METAL GEAR SOLID 3";
static WCHAR const skSGS_Title_EU[] = L"METAL GEAR SOLID 3";
static WCHAR const skSGS_Title_J[]  = L"METAL GEAR SOLID 3";
#endif

static WCHAR const skSGS_Unknown[] = L"Unknown";

static void _get_save_game_localized_title(WCHAR * const outPath, EInternalSaveData const whichSaveData)
{
   const char * const localized_title_utf8 = save_data_get_save_game_localized_string( kSGS_Title, whichSaveData );
   const char * const localized_subtitle_utf8 = save_data_get_save_game_localized_string( kSGS_Subtitle, whichSaveData );

   char full_title_utf8[XCONTENT_MAX_DISPLAYNAME_LENGTH*2];

   switch( whichSaveData )
   {
   case kISD_Game:
      {
         //Don't add the subtitle "save data" because it's redundant.  All are labeled as Save Data
         //on the second line in the dashboard anyway.
         strcpy( full_title_utf8, localized_title_utf8 );
      }
      break;
   case kISD_MG1:
   case kISD_MG2:
      {
         //Don't concatenate "Metal Gear Solid 3" with "Metal Gear" or "Metal Gear 2 Solid Snake"
         //because that looks stupid and savedata are already divided up by titleid on X360 anyway.
         strcpy( full_title_utf8, localized_subtitle_utf8 );
      }
      break;
   default:
      {
         //Concatenate title for MGS2 or MGS3 with the subtitle for all other save types.
         sprintf( full_title_utf8, "%s %s", localized_title_utf8, localized_subtitle_utf8 );
      }
      break;
   }

   MultiByteToWideChar(CP_UTF8, 0, full_title_utf8, -1, outPath, XCONTENT_MAX_DISPLAYNAME_LENGTH);
}

//----------------------------------------------------------------------------

void save_data_init( const char * const saveDirectoryRoot )
{
   //unused on X360.
}

extern "C" void BP_ShowDeviceInvalid_Internal()
{
   ShowGenericOsDialog_Sync(kDRT_StorageDeviceUnavailable);
}

extern "C" void BP_ShowDeviceInvalid()
{
   if( gBP_ShowDeviceInvalidUI )
   {
      BP_ShowDeviceInvalid_Internal();
      gBP_ShowDeviceInvalidUI = 0;
   }
}

extern "C" int BP_WasDeviceEverValid()
{
   COsContext * pOsContext = OsContext();
   if( pOsContext->mSaveLoadDeviceId != XCONTENTDEVICE_ANY )
   {
      return 1;
   }
   return 0;
}

extern "C" int BP_IsDeviceInvalid()
{
   COsContext * pOsContext = OsContext();
   XDEVICE_DATA deviceData;
   if( pOsContext->mSaveLoadDeviceId == XCONTENTDEVICE_ANY || XContentGetDeviceData(pOsContext->mSaveLoadDeviceId, &deviceData) == ERROR_DEVICE_NOT_CONNECTED )
   {
      BP_ShowDeviceInvalid();
      return 1;
   }
   return 0;
}

extern "C" unsigned int HelperGetTotalSaveSpaceNecessary(CMGS_SaveStatus * pSS)
{
   int directoryCount = 1; 
   unsigned int totalFileSize = pSS->GetMCFilesTotalSizeExcludeSystem();
   return XContentCalculateSize( totalFileSize, directoryCount );
}

//----------------------------------------------------------------------------

static DWORD thr_save_data_get_dir_list( LPVOID lpParameter )
{
   CMGS_SaveStatus * pSS = ( CMGS_SaveStatus * )lpParameter;

   printf( "thr_save_data_get_dir_list: getting %s.\n", pSS->mSaveDirectory );

   //Need enough space to enumerate *all* savedata of all (up to four) types.
   static const int skMaxEnumeratedContentData = 64 + 64 + 64 + 64;
   static XCONTENT_DATA sEnumeratedContentData[skMaxEnumeratedContentData];

   HANDLE hEnum;
   DWORD cbBuffer;

   COsContext * pOsContext = OsContext();

   // Create enumerator for the default device
   int matchCount = 0;
   DWORD dwRet;
   dwRet = XContentCreateEnumerator( pOsContext->mSignedInUser, // Access data associated with this user
      pOsContext->mSaveLoadDeviceId,         // Pass in selected device
      XCONTENTTYPE_SAVEDGAME,
      0,                     // No special flags
      skMaxEnumeratedContentData,
      &cbBuffer,
      &hEnum );
   if( dwRet == ERROR_SUCCESS )
   {
      assert( cbBuffer == sizeof( sEnumeratedContentData ) );

      // Enumerate display names
      int contentDataCount = 0;

      DWORD dwReturnCount = 0;
      dwRet = XEnumerate( hEnum, sEnumeratedContentData, sizeof( sEnumeratedContentData ), &dwReturnCount, NULL );
      if( dwRet == ERROR_SUCCESS || dwRet == ERROR_NO_MORE_FILES )
      {
         printf("XEnumerate Count: %d\n", dwReturnCount);
         contentDataCount = dwReturnCount;
      }
      else
      {
         dwReturnCount = 0;
      }
      CloseHandle( hEnum );

      //Filter and fill in content data here.
      for( int i=0; i < contentDataCount; ++i )
      {
         if( strstr( sEnumeratedContentData[i].szFileName, pSS->mSaveDirectory ) )
         {
            bool isValidFile = true;
#if MGS_VERSION==2
            WIN32_FILE_ATTRIBUTE_DATA fileAttr = {0};
            XCONTENT_DATA contentData = {0};
            strcpy_s( contentData.szFileName, sEnumeratedContentData[i].szFileName );
            contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
            contentData.DeviceID = sEnumeratedContentData[i].DeviceID;

            DWORD dwErr = XContentCreate( pOsContext->mSignedInUser, BP_XCONTENT_SAVEDATA_ROOT, &contentData,
               XCONTENTFLAG_OPENEXISTING, NULL, NULL, NULL );
            if( dwErr == ERROR_SUCCESS )
            {
               const SMCFile * const pFile = pSS->mpMCFiles;
               printf( "thr_save_data_auto_load: loading %s.\n", pFile->mName );

               char fullPath[FILENAME_MAX];
               sprintf( fullPath, "%s:\\%s", BP_XCONTENT_SAVEDATA_ROOT, pFile->mName );

               GetFileAttributesEx(fullPath, GetFileExInfoStandard, &fileAttr );
               XContentClose( BP_XCONTENT_SAVEDATA_ROOT, NULL );
            }
            else if( dwErr == ERROR_FILE_CORRUPT )
            {
               isValidFile = false;
            }

            sceMcTblGetDir * tbl = pSS->mpGetDirTable+matchCount;
            memset( tbl, 0, sizeof(*tbl) );
            strncpy( (char*)tbl->EntryName, sEnumeratedContentData[i].szFileName, sizeof( tbl->EntryName ) - 1 );
            tbl->AttrFile |= 0x0020; //sceMcFileAttrSubdir
            tbl->_Modify = *(time_t*)&fileAttr.ftLastWriteTime;
#elif MGS_VERSION==3
            SceMc2DirParam * tbl = pSS->mpGetDirTable2+matchCount;
            memset( tbl, 0, sizeof(*tbl) );
            strncpy( (char*)tbl->name, sEnumeratedContentData[i].szFileName, sizeof( tbl->name ) - 1 );
            tbl->attribute |= 0x0020; //SCE_MC2_FILE_ATTR_SUBDIR
#endif
            ++matchCount;
            if( matchCount == pSS->mGetDirListMaxEntries )
               break;
         }
      }
   }
#if MGS_VERSION==2
   pSS->mGetDirListExistEntries = matchCount;
#elif MGS_VERSION==3
   *pSS->mpGetDirListExistEntries = matchCount;
#endif

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   ExitThread( 0 );
}

//----------------------------------------------------------------------------
int save_data_space_check( CMGS_SaveStatus * pSS )
{
   //Not required as a preemptive check on X360 because the user can swap out storage devices on the fly.
   return 0;
}

//----------------------------------------------------------------------------

int save_data_get_dir_list( CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;

   HANDLE thread_id = CreateThread( NULL, 0, &thr_save_data_get_dir_list, pSS, CREATE_SUSPENDED, NULL );
   BPE_ASSERT_NO_MSG( thread_id != NULL );
   XSetThreadProcessor( thread_id, skSaveLoadThreadProcessor );
   ResumeThread( thread_id );
   return 0;
}

//----------------------------------------------------------------------------
CStopWatch gSaveDelayTimer;

extern "C" void BP_StartSaveMessageTimer_Internal()
{
   return gSaveDelayTimer.Reset();
}

extern "C" int BP_IsSaveMessageTimerOver_Internal()
{
   return gSaveDelayTimer.GetElapsedMilliseconds() > 3000;
}

//----------------------------------------------------------------------------

static inline void TranslateErrorResult( CMGS_SaveStatus * pSS, DWORD lastError )
{
   if( lastError == ERROR_DISK_FULL )
   {
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoSpace;
   }
   else
   {
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_CELL_Error;
   }
}

bool gIsNewSave = false;
static void save_data_do_auto_save( CMGS_SaveStatus * pSS )
{
   OsContext()->SetUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );

   gSaveDelayTimer.Reset();

   COsContext * pOsContext = OsContext();

   XDEVICE_DATA deviceData;
   if( pOsContext->mSaveLoadDeviceId == XCONTENTDEVICE_ANY || XContentGetDeviceData(pOsContext->mSaveLoadDeviceId, &deviceData) == ERROR_DEVICE_NOT_CONNECTED )
   {
      // guess they don't want to save
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_CELL_Error;
   }
   else
   {
#if MGS_VERSION == 2
      CStopWatch saveDelayTimer;
#endif
      XCONTENT_DATA contentData = {0};
      strcpy_s( contentData.szFileName, pSS->mSaveDirectory );
      BPE_ASSERT_NO_MSG( pSS->mpMCFiles[0].mFileType == kMCFT_SaveGame ); //make sure no ordering error!
      _get_save_game_localized_title( contentData.szDisplayName, pSS->mpMCFiles[0].mInternalFileType );
      contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
      contentData.DeviceID = pOsContext->mSaveLoadDeviceId;

      // Mount the device associated with the display name for writing
      DWORD dwErr = XContentCreate( pOsContext->mSignedInUser, BP_XCONTENT_SAVEDATA_ROOT, &contentData,
         XCONTENTFLAG_OPENALWAYS, NULL, NULL, NULL );
      if( dwErr == ERROR_SUCCESS )
      {
         bool hasHadSaveError = false;

         for( int i=0; i < pSS->GetNumMCFiles(); ++i )
         {
            if( hasHadSaveError )
               continue;
            if( !pSS->ShouldWriteFile(i) )
               continue;
            const SMCFile * const pFile = pSS->mpMCFiles+i;



            printf( __FUNCDNAME__ ": saving %s.\n", pFile->mName );

            switch( pFile->mFileType )
            {
            case kMCFT_SaveGame:
            case kMCFT_Master:
               {
                  char fullPath[FILENAME_MAX];
                  sprintf( fullPath, "%s:\\%s", BP_XCONTENT_SAVEDATA_ROOT, pFile->mName );

                  HANDLE hFile = CreateFile( fullPath, GENERIC_WRITE, 0,
                     NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

                  if( hFile != INVALID_HANDLE_VALUE )
                  {
                     // Write data to the file
                     CHAR szBuffer[] = "Test save game data.\n";
                     DWORD dwWritten;

                     if( WriteFile( hFile, pFile->mData, pFile->mSize, &dwWritten, NULL ) == 0 )
                     {
                        DWORD lastError = GetLastError();
                        printf( __FUNCDNAME__ ": WriteFile failed. Error = %08x\n", lastError );
                        TranslateErrorResult(pSS, lastError);
                        hasHadSaveError = true;
                     }

                     CloseHandle( hFile );
                  }
                  else
                  {
                     DWORD lastError = GetLastError();
                     printf( __FUNCDNAME__ ": CreateFile failed. Error = %08x\n", lastError );
                     TranslateErrorResult(pSS, lastError);
                     hasHadSaveError = true;
                  }
               }
               break;
            case kMCFT_System:
               {
                  //Icon needs to use thumbnail API.
                  dwErr = XContentSetThumbnail( pOsContext->mSignedInUser, &contentData, (const BYTE*)pFile->mData, pFile->mSize, NULL );
                  if( dwErr != ERROR_SUCCESS )
                  {
                     printf( __FUNCDNAME__ ": SetThumbnail. Error = %08x\n", dwErr );
                     TranslateErrorResult(pSS, dwErr);
                     hasHadSaveError = true;
                  }
               }
               break;
            default:
               BP_TODO_BREAK;
            }
         }
         XContentClose( BP_XCONTENT_SAVEDATA_ROOT, NULL );

         if( gIsNewSave && hasHadSaveError )
         {
            XCONTENT_DATA contentData = {0};
            strcpy_s( contentData.szFileName, pSS->mSaveDirectory );
            contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
            contentData.DeviceID = pOsContext->mSaveLoadDeviceId;

            // Mount the device associated with the display name for writing
            DWORD dwErr = XContentDelete( pOsContext->mSignedInUser, &contentData, NULL );
         }
      }
      else
      {
         TranslateErrorResult(pSS, dwErr);
      }
#if MGS_VERSION == 2
      while( saveDelayTimer.GetElapsedMilliseconds() < 1000 ) // pause finishing until save message has been up for at least 1 second
      {
      }
#endif
   }

   OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_SaveData );
}

//----------------------------------------------------------------------------

static DWORD thr_save_data_auto_save( LPVOID lpParameter )
{
   CMGS_SaveStatus * pSS = ( CMGS_SaveStatus * )lpParameter;

   printf( __FUNCDNAME__ ": saving %s.\n", pSS->mSaveDirectory );

   gIsNewSave = 1;
   save_data_do_auto_save( pSS );

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }



   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   ExitThread(0);

}
//----------------------------------------------------------------------------

// CMGS_SaveStatus data must be valid during save/load process.
int save_data_auto_save( CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;

   HANDLE thread_id = CreateThread( NULL, 0, &thr_save_data_auto_save, pSS, CREATE_SUSPENDED, NULL );
   BPE_ASSERT_NO_MSG( thread_id != NULL );
   XSetThreadProcessor( thread_id, skSaveLoadThreadProcessor );
   ResumeThread( thread_id );
   return 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static DWORD thr_save_data_auto_load( LPVOID lpParameter )
{
   DWORD dwFileSize = 0;
   CMGS_SaveStatus * pSS = ( CMGS_SaveStatus * )lpParameter;

   COsContext * pOsContext = OsContext();

   XDEVICE_DATA deviceData;
   if( pOsContext->mSaveLoadDeviceId != XCONTENTDEVICE_ANY && XContentGetDeviceData(pOsContext->mSaveLoadDeviceId, &deviceData) != ERROR_DEVICE_NOT_CONNECTED )
   {
      XCONTENT_DATA contentData = {0};
      strcpy_s( contentData.szFileName, pSS->mSaveDirectory );
      contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
      contentData.DeviceID = pOsContext->mSaveLoadDeviceId;

      // Mount the device associated with the display name for writing
      DWORD dwErr = XContentCreate( pOsContext->mSignedInUser, BP_XCONTENT_SAVEDATA_ROOT, &contentData,
         XCONTENTFLAG_OPENEXISTING, NULL, NULL, NULL );

      if( dwErr == ERROR_SUCCESS )
      {
         for( int i=0; i < pSS->GetNumMCFiles(); ++i )
         {
            const SMCFile * const pFile = pSS->mpMCFiles+i;
            printf( "thr_save_data_auto_load: loading %s.\n", pFile->mName );

            switch( pFile->mFileType )
            {
            case kMCFT_SaveGame:
            case kMCFT_Master:
               {
                  char fullPath[FILENAME_MAX];
                  sprintf( fullPath, "%s:\\%s", BP_XCONTENT_SAVEDATA_ROOT, pFile->mName );

                  HANDLE hFile = CreateFile( fullPath, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

                  if( hFile != INVALID_HANDLE_VALUE )
                  {
                     if( pSS->ShouldReadFile(i) )
                     {
                        // Read data from the file
                        DWORD dwRead = 0;
                        if( ReadFile( hFile, pFile->mData, pFile->mSize, &dwRead, NULL ) == 0 )
                        {
                           printf( "thr_save_data_auto_load: ReadFile failed. Error = %08x\n", GetLastError() );
                           pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
                        }
                        //we still need to know the file modification time
                        ((SMCFile*)pFile)->mGenericResult = dwRead;
                        BY_HANDLE_FILE_INFORMATION fileInformation;
                        if( GetFileInformationByHandle( hFile, &fileInformation ) == 0 )
                        {
                           printf( "thr_save_data_auto_load: ReadFile failed. Error = %08x\n", GetLastError() );
                           ((SMCFile*)pFile)->mGenericResult = 1;
                           pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
                        }
                        ((SMCFile*)pFile)->mFileModifiedTime = *(time_t*)&fileInformation.ftLastWriteTime;
                     }
                     else
                     {
                        //we still need to know how big this file was, and the file modification time
                        if( pFile->mFileType == kMCFT_SaveGame )
                        {
                           BY_HANDLE_FILE_INFORMATION fileInformation;
                           if( GetFileInformationByHandle( hFile, &fileInformation ) == 0 )
                           {
                              CloseHandle( hFile );
                              printf( "thr_save_data_auto_load: ReadFile failed. Error = %08x\n", GetLastError() );
                              ((SMCFile*)pFile)->mGenericResult = 1;
                              pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
                           }
                           else
                           {
                              ((SMCFile*)pFile)->mGenericResult = fileInformation.nFileSizeLow;
                              ((SMCFile*)pFile)->mFileModifiedTime = *(time_t*)&fileInformation.ftLastWriteTime;
                           }
                        }
                     }

                     CloseHandle( hFile );
                  }
                  else
                  {
                     printf( "thr_save_data_auto_load: CreateFile failed. Error = %08x\n", GetLastError() );
                     ((SMCFile*)pFile)->mGenericResult = 1;
                     pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
                  }
               }
               break;
            case kMCFT_System:
               {
                  //TODO: verify that icon matches?
                  continue;
               }
               break;
            default:
               BP_TODO_BREAK;
            }
         }

         XContentClose( BP_XCONTENT_SAVEDATA_ROOT, NULL );
      }
      else if( dwErr == ERROR_PATH_NOT_FOUND )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_NoData;
      }
      else if( dwErr == ERROR_FILE_CORRUPT )
      {
         printf( "thr_save_data_auto_load: Corrupt data. Error = %08x\n", dwErr );
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;

         for( int i=0; i < pSS->GetNumMCFiles(); ++i )
         {
            const SMCFile * const pFile = pSS->mpMCFiles+i;
            ((SMCFile*)pFile)->mGenericResult = 1;
         }      
      }
      else
      {
         printf( "thr_save_data_auto_load: CreateContent failed. Error = %08x\n", dwErr );
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;

         for( int i=0; i < pSS->GetNumMCFiles(); ++i )
         {
            const SMCFile * const pFile = pSS->mpMCFiles+i;
            ((SMCFile*)pFile)->mGenericResult = 1;
         } 
      }
   }
   else
   {
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;

      for( int i=0; i < pSS->GetNumMCFiles(); ++i )
      {
         const SMCFile * const pFile = pSS->mpMCFiles+i;
         ((SMCFile*)pFile)->mGenericResult = 1;
      } 
   }

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   ExitThread(0);
}

//----------------------------------------------------------------------------

int save_data_auto_load( CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;

   HANDLE thread_id = CreateThread( NULL, 0, &thr_save_data_auto_load, pSS, CREATE_SUSPENDED, NULL );
   BPE_ASSERT_NO_MSG( thread_id != NULL );
   XSetThreadProcessor( thread_id, skSaveLoadThreadProcessor );
   ResumeThread( thread_id );
   return 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static void save_data_do_auto_delete( CMGS_SaveStatus * pSS )
{
   COsContext * pOsContext = OsContext();

   XCONTENT_DATA contentData = {0};
   strcpy_s( contentData.szFileName, pSS->mSaveDirectory );
   contentData.dwContentType = XCONTENTTYPE_SAVEDGAME;
   contentData.DeviceID = pOsContext->mSaveLoadDeviceId;

   // Mount the device associated with the display name for writing
   DWORD dwErr = XContentDelete( pOsContext->mSignedInUser, &contentData, NULL );
   if( dwErr == ERROR_SUCCESS )
   {
      printf( __FUNCDNAME__ ": DeleteContent complete.\n" );
   }
   else
   {
      printf( __FUNCDNAME__ ": DeleteContent failed. Error = %08x\n", dwErr );
      //TODO: XBOX360 error codes.
      pSS->mCallbackResult = CMGS_SaveStatus::kCB_CELL_Error;
   }
}

//----------------------------------------------------------------------------

static DWORD thr_save_data_auto_delete( LPVOID lpParameter )
{
   CMGS_SaveStatus * pSS = ( CMGS_SaveStatus * )lpParameter;

   printf( __FUNCDNAME__ ": deleting %s.\n", pSS->mSaveDirectory );

   save_data_do_auto_delete( pSS );

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   ExitThread(0);
}

//----------------------------------------------------------------------------

int save_data_auto_delete( CMGS_SaveStatus * pSS )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;

   HANDLE thread_id = CreateThread( NULL, 0, &thr_save_data_auto_delete, pSS, CREATE_SUSPENDED, NULL );
   BPE_ASSERT_NO_MSG( thread_id != NULL );
   XSetThreadProcessor( thread_id, skSaveLoadThreadProcessor );
   ResumeThread( thread_id );
   return 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

static DWORD thr_save_data_auto_recreate_save( LPVOID lpParameter )
{
   CMGS_SaveStatus * pSS = ( CMGS_SaveStatus * )lpParameter;

   gIsNewSave = 0;
   printf( __FUNCDNAME__ ": saving %s.\n", pSS->mSaveDirectory );
   save_data_do_auto_save( pSS );

   //Last step before calling it done and exiting the thread: call the completion callback, if any.
   if( save_data_async_op_callback callback = (save_data_async_op_callback)pSS->mpAsyncOpCallback )
   {
      callback( pSS );
   }

   pSS->mStatus = CMGS_SaveStatus::kS_Done;

   ExitThread(0);
}

//----------------------------------------------------------------------------

int save_data_auto_recreate_save( CMGS_SaveStatus * pSS, save_data_async_op_callback callback )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mpAsyncOpCallback = (void*)callback;

   pSS->mStatus = CMGS_SaveStatus::kS_Processing;
   pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;

   HANDLE thread_id = CreateThread( NULL, 0, &thr_save_data_auto_recreate_save, pSS, CREATE_SUSPENDED, NULL );
   BPE_ASSERT_NO_MSG( thread_id != NULL );
   XSetThreadProcessor( thread_id, skSaveLoadThreadProcessor );
   ResumeThread( thread_id );
   return 0;
}

//----------------------------------------------------------------------------

extern "C" void ShowDeviceSelector_Sync(unsigned int requiredSizeForNewSave);
extern "C" void ShowGenericOsDialog_Sync(unsigned int dialogType/*EDialogRequestType*/);
int save_data_get_info( CMGS_SaveStatus * pSS )
{
   BPE_ASSERT( pSS->mStatus != CMGS_SaveStatus::kS_Processing, "save or load already in progress" );

   pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;

   COsContext * const pOsContext = OsContext();
   {
      XDEVICE_DATA deviceData;
      if( !gBP_CanSelectNewDevice )
      {
         if( pOsContext->mSaveLoadDeviceId == XCONTENTDEVICE_ANY || XContentGetDeviceData(pOsContext->mSaveLoadDeviceId, &deviceData) == ERROR_DEVICE_NOT_CONNECTED )
         {
            // We can not ask them for a new device, and the device is invalid
            // if the user previously had a save device we have to inform them it is no longer available
            if( pSS->mDeviceId != XCONTENTDEVICE_ANY )
            {
               ShowGenericOsDialog_Sync(kDRT_StorageDeviceUnavailable);
            }
            // let the game think there is no device
            gBP_CanSelectNewDevice = 0;
            pSS->mDeviceId = XCONTENTDEVICE_ANY;
            pSS->mStatus = CMGS_SaveStatus::kS_Done;
         }
         else
         {
            gBP_CanSelectNewDevice = 0;
            pSS->mDeviceId = pOsContext->mSaveLoadDeviceId;
            pSS->mStatus = CMGS_SaveStatus::kS_Done;
         }
      }
      else
      {
         // we can show them a new device selector
         if( pOsContext->mSaveLoadDeviceId == XCONTENTDEVICE_ANY || XContentGetDeviceData(pOsContext->mSaveLoadDeviceId, &deviceData) == ERROR_DEVICE_NOT_CONNECTED )
         {
            if( pOsContext->mSaveLoadDeviceId != XCONTENTDEVICE_ANY )
            {
               ShowGenericOsDialog_Sync(kDRT_StorageDeviceUnavailable);
            }
            //Bring up device selector UI.
            unsigned int requiredSizeForNewSave = HelperGetTotalSaveSpaceNecessary(pSS);
            ShowDeviceSelector_Sync(requiredSizeForNewSave);
         }
         // 
         gBP_CanSelectNewDevice = 0;
         pSS->mDeviceId = pOsContext->mSaveLoadDeviceId;
         pSS->mStatus = CMGS_SaveStatus::kS_Done;
      }
   }
   return 0;
}

//----------------------------------------------------------------------------
