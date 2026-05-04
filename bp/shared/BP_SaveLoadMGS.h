//----------------------------------------------------------------------------
// BP_SaveLoadMGS.h
// MGS-specific emulation layer between PS2 saveload I/O and CP4 platforms.
//
//----------------------------------------------------------------------------

#pragma once

#include "BP_SaveLoad.h"

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

   //Interfaces to global singleton CMGS_SaveStatus

   void MGS_SaveStatus_Init();

   int MGS_SaveStatus_IsDone();
   int MGS_SaveStatus_CompletedOK();
   int MGS_SaveStatus_HasNoSpace();
   int MGS_SaveStatus_ExtraSpaceReq();    //extra space req. to save current data
   int MGS_SaveStatus_IsCorrupted();
   int MGS_SaveStatus_WrongUser();        //Wrong user of last savedata loaded?

   void MGS_SaveStatus_ClearWrongUser();  //Reset wrong user flag whenever we return to title!

   //These functions return 1 if they started an asynchronous operation.
#if MGS_VERSION==2
   //for sceMc interface (these functions only visible externally in MGS2)
   unsigned int MGS_SaveStatus_Read( void *buff, unsigned int size );
   unsigned int MGS_SaveStatus_Write( const void *buff, unsigned int size );
   unsigned int MGS_SaveStatus_GetDir(const char *name, int maxent, sceMcTblGetDir *table);
   void MGS_SaveStatus_Open( const char * const name, const int mode );
   void MGS_SaveStatus_Close();
   int MGS_SaveStatus_CloseReturnError(); // Returns 0 on success
#elif MGS_VERSION==3
   //for sceMc2 interface
   unsigned int MGS_SaveStatus_GetDir2(const char *name, int maxent, SceMc2DirParam *table, int * const pOutEntCount);
   unsigned int MGS_SaveStatus_Read2( const char * const name, void *buff, unsigned int size );
   unsigned int MGS_SaveStatus_Write2( const char * const name, const void *buff, unsigned int size );
#endif

   int MGS_SaveStatus_GetNumDirEntries();

   //returns number of entries found.
   int MGS_SaveStatus_GetDirResult();

   void MGS_SaveStatus_ChDir( const char * const dirsuffix );

   unsigned int MGS_SaveStatus_Delete( const char * const name );

   //For getting storage device info (only necessary on XBox which has MU's and not necessarily a HDD)
   void MGS_SaveStatus_ResetForNewLoadSaveOperation();
   unsigned int MGS_SaveStatus_GetInfo();
   int MGS_SaveStatus_GetInfoResult();

   unsigned int MGS_SaveStatus_Rename( const char * const org, const char * const dst );
   void MGS_SaveStatus_Seek( const int offset, const int mode );
#ifdef __cplusplus
};
#endif

//------------------------------------------------------------------------------------------
