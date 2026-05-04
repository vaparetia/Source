//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	memcard_access.c
		メモリーカード制御サブルーチン
	
	2001/01/12 K.Kano
	$Id: memcard_access.c,v 1.1.1.3 2002/11/19 11:43:18 Yoshizawa1 Exp $
*/

#include "memcard_access.h"
#if 0 //BP_PS2
#include <libver.h>
#endif

#ifdef BP_WIN32
//TEMP: support for BP emulation versions of sce MC API
#include <Windows.h>
#include <direct.h>
#include <errno.h>
#include <io.h>
#elif defined( BP_PS3 )
#include <cell/cell_fs.h>
#include <sys/paths.h>
#elif defined( BP_360 )
#include <xtl.h>
#include <xbdm.h>
extern void BP_UnifyPathX360( char * path );

#endif

#include "BP_FileSupport.h"
#include "BP_EndianSupport.h"
#include "BP_Misc.h"
#include "BP_SaveLoadMGS.h"

extern void ShowGenericOsDialog_Sync(unsigned int dialogType/*EDialogRequestType*/);
extern void ShowLowDiskSpaceWarning(int *pResult, int kbNeeded);

#define DEBUG_MC_ACCESS 0

#if DEBUG_MC_ACCESS==1
#  define DMC_PRINTF printf
#else
#  define DMC_PRINTF(x,...) (0)
#endif

//----------------------------------------------------------------------------

#define BP_MC_ASYNC_FLAG 0x80000000

char bp_basefilename[0x30];

#if !defined( BP_PS3 ) && !defined(BP_360) && !defined(BP_VITA)
//Use normal file system to emulate PS2 saves on non-PS3 platforms for now.
#define BP_PS2_SAVE_EMULATION
#endif

//----------------------------------------------------------------------------

enum {
	MCAccess_WAIT=0,

	MCAccess_PRECHECK,
	MCAccess_CHECKSTART,
	MCAccess_CHECK,

	MCAccess_OPEN,
	MCAccess_CLOSE,
	MCAccess_SEEK,
	MCAccess_READ,
	MCAccess_WRITE,

	MCAccess_MKDIR,
	MCAccess_CHDIR,
	MCAccess_GETDIR,
	MCAccess_GETDIR_NEXT,

	MCAccess_RENAME,
	MCAccess_DELETE,
	MCAccess_CHMOD,

	MCAccess_FORMAT,
	MCAccess_UNFORMAT,
};


/* ------------------------------------------------------------------------ */

static int sBP_MCResult = 0;
static int sBP_MCCmd = 0;
static char sBP_MCPath[FILENAME_MAX] = "/";

static int * spBP_MCType = NULL;
static int * spBP_MCFree = NULL;
static int * spBP_MCFormat = NULL;

#ifdef BP_WIN32
static const char * skBP_MCRootPath = "mgs2_savedata_win";
#elif defined(BP_PS3)
static const char * skBP_MCRootPath = SYS_APP_HOME "/mgs2_savedata_ps3";
int sBP_MCDirFd = 0;
#elif defined(BP_360)
#define BP_X360_SAVE_HOME "E:\\MGS2"
static const char * skBP_MCRootPath = BP_X360_SAVE_HOME "\\mgs2_savedata_360";
static HRESULT sBP_MountResult = 0;
#endif

//pattern matching code cribbed from http://www.codeproject.com/KB/string/patmatch.aspx
int pattern_match(const char *str, const char *pattern)
{
   enum State {
      Exact,      	// exact match
      Any,        	// ?
      AnyRepeat    	// *
   };

   const char *s = str;
   const char *p = pattern;
   const char *q = 0;
   int state = 0;

   int match = 1;
   while (match && *p) {
      if (*p == '*') {
         state = AnyRepeat;
         q = p+1;
      } else if (*p == '?') state = Any;
      else state = Exact;

      if (*s == 0) break;

      switch (state) {
            case Exact:
               match = *s == *p;
               s++;
               p++;
               break;

            case Any:
               match = 1;
               s++;
               p++;
               break;

            case AnyRepeat:
               match = 1;
               s++;

               if (*s == *q){
                  // make a recursive call so we don't match on just a single character
                  if (pattern_match(s,q)) {
                     p++;
                  }
               }
               break;       
      }
   }

   if (state == AnyRepeat) return (*s == *q);
   else if (state == Any) return (*s == *p);
   else return match && (*s == *p);
}

#ifdef BP_WIN32

static void bp_temp_mc_chdir_push()
{
   //Temporarily change to the current memory card directory.
   int ret;
   char fullPath[FILENAME_MAX];
   strcpy( fullPath, skBP_MCRootPath );
   strcat( fullPath, sBP_MCPath );
   ret = chdir( fullPath );
   if( ret == -1 )
      BP_BREAK;
}

static void bp_temp_mc_chdir_pop()
{
   //Return to normal file serving folder.
   int ret;
   int subdirCount = 0;
   const char * p;
   //For each subfolder (forward slash) in the current mc path, go up one.
   for( p = sBP_MCPath; *p != '\0'; ++p )
   {
      if( *p == '/' )
      {
         ret = chdir( ".." );
         if( ret == -1 )
            BP_BREAK;
      }
   }
}

#endif

int BP_sceMcInit(void)
{
#ifndef BP_PS2_SAVE_EMULATION
   //BP TED - this is now called by the trophy system in its startup thread.
   //Necessary for PS3 to behave properly wrt free space checking and trophy installation.
   //MGS_SaveStatus_Init();
#else

#ifdef BP_WIN32
   mkdir( skBP_MCRootPath );
#elif defined( BP_PS3 )

   int ret = cellFsOpendir( SYS_APP_HOME, &sBP_MCDirFd );
   if( ret == CELL_FS_SUCCEEDED )
   {
      //Create the savegame folder.
      ret = cellFsMkdir( skBP_MCRootPath, CELL_FS_DEFAULT_CREATE_MODE_1 );
   }
   else
   {
      printf("BP_sceMcInit: app_home not found, memory card not supported.\n");
   }

#elif defined( BP_360 )
   sBP_MountResult = DmMapDevkitDrive();
   if( sBP_MountResult == S_OK )
   {
      BOOL ret;
      ret = CreateDirectory( BP_X360_SAVE_HOME, NULL );
      ret = CreateDirectory( skBP_MCRootPath, NULL );
   }
   if( sBP_MountResult != S_OK )
   {
      printf("BP_sceMcInit: could not create directory %s, memory card not supported.\n", skBP_MCRootPath);
   }
#endif

#endif

   return 0;
}

int BP_sceMcGetInfo( int slot, int port, int *type, int *free, int *format )
{
   unsigned int bp_async_flag = 0;

   int outType;
   int outFree;
   int outFormat;
   if(slot == 0 && port == 0
#ifdef BP_PS2_SAVE_EMULATION

#ifdef BP_PS3
      && sBP_MCDirFd != 0
#elif defined( BP_360 )
      && sBP_MountResult == S_OK
#endif

#endif
      )
   {
      //Default empty 8MB card in slot 0 port 0
      outType = 2;      //sceMcTypePS2
      outFree = 8000;   //free cluster count (1 cluster == 1024 bytes)
      outFormat = 1;    //formatted
#ifndef BP_PS2_SAVE_EMULATION
      bp_async_flag = MGS_SaveStatus_GetInfo();
      bp_async_flag *= BP_MC_ASYNC_FLAG;
#endif
   }
   else
   {
      //No card
      outType = 0;   //sceMcTypeNoCard
      outFree = 0;
      outFormat = 0;
   }

   spBP_MCType = type;
   spBP_MCFree = free;
   spBP_MCFormat = format;

   if( type )
      *type = outType;
   if( free )
      *free = outFree;
   if( format )
      *format = outFormat;

   sBP_MCResult = 0;
   sBP_MCCmd = bp_async_flag | sceMcFuncNoCardInfo;
   return 0;
}

static void fix_mc_name_filter( char * filter )
{
   //They seem to have only one use of this function in MGS2,
   //which we can work around easier than implementing.
   //Just strip out the single character '?' wildcards.
   char * annoyingWildcard;
   if( annoyingWildcard = strstr( filter, "????*" ) )
   {
      strcpy( annoyingWildcard, annoyingWildcard + 4 );
   }
   if( strchr( filter, '?' ) )
   {
      //Another case we haven't seen
      BP_TODO_BREAK;
   }
}

#ifdef BP_WIN32

int bp_get_files_in_current_folder( const char *filter, int maxEnt, sceMcTblGetDir *table )
{
   int entCount = 0;
   struct _finddata_t c_file;
   intptr_t hFile;
   char fixupFilter[FILENAME_MAX];
   strcpy( fixupFilter, filter );
   fix_mc_name_filter( fixupFilter );
   if( (hFile = _findfirst( fixupFilter, &c_file ) ) == -1L )
   {
      //no files.
   }
   else
   {
      do
      {
         sceMcTblGetDir * nextFile = table + entCount;
         if( !strcmp(c_file.name, ".") || !strcmp(c_file.name,"..") )
         {
            continue;
         }
         if( entCount == maxEnt )
         {
            break;
         }
         //record the next file.
         memset( nextFile, 0, sizeof( *nextFile ) );
         strncpy( nextFile->EntryName, c_file.name, sizeof( nextFile->EntryName ) );
         nextFile->AttrFile = 0;
         if( c_file.attrib & _A_SUBDIR )
         {
            nextFile->AttrFile |= 0x0020; //sceMcFileAttrSubdir
         }
         else
         {
            nextFile->FileSizeByte = BP_GetFileSizeAttr( c_file.name, NULL );
         }
         //TODO: any other needed attributes.

         ++entCount;
      }
      while( _findnext( hFile, &c_file ) == 0 );
      _findclose( hFile );
   }

   return entCount;
}

int bp_do_get_dir_win( const char *name, int maxent, sceMcTblGetDir *table)
{
   int entcount = 0;
   int i;
   int ret;
   const char * slash = strchr( name, '/' );
   if( slash )
   {
      //name contains a subfolder.
      //First we need to get a list of folders that match the folder part of the name.
      char folder[FILENAME_MAX] = { 0 };
      strncpy( folder, name, slash-name );

      ret = chdir( folder );
      if( ret == -1 )
         BP_BREAK;

      entcount = bp_get_files_in_current_folder( slash+1, maxent, table );

      //TODO: if we were to support recursive directory searching, we'd check acquired files here
      //and iterate through any subfolders.

      ret = chdir( ".." );
      if( ret == -1 )
         BP_BREAK;
   }
   else
   {
      //no folder as part of the name.
      entcount = bp_get_files_in_current_folder( name, maxent, table );
   }

   return entcount;
}

#elif defined( BP_PS3 )

int match_mc_name_filter( const char * const name, const char * const filter )
{
   const char * src = filter;
   const char * dst = name;

   //Taking the easy way out for this temporary functionality.
   //Only support a wildcard at the end of the string.
   const char * const lastAsterisk = strrchr( filter, '*' );
   if( lastAsterisk != NULL && *(lastAsterisk+1) != '\0' )
      BP_TODO_BREAK;

   while( *dst != '\0' )
   {
      const char s = *src;
      const char d = *dst;
      if( s == d || s == '?' )
      {
         //single character match.
         ++src;
         ++dst;
      }
      else
      {
         return 0;
      }
   }

   if( *src != '\0' && *src != '*' )
   {
      //checked entire name, but filter is longer.
      //only match if src is at the magical last asterisk.
      return 0;
   }

   return 1;
}

int bp_do_get_dir_ps3( const char *name, int maxent, sceMcTblGetDir *table)
{
   int entcount = 0;
   int ret;
   char fullPath[FILENAME_MAX] = { 0 };
   char folderOnly[FILENAME_MAX] = { 0 };
   const char * lastSlash;
   const char * filenameOnly;

   strcpy( fullPath, skBP_MCRootPath );
   if( *name != '/' )
   {
      //Need to concatenate current MC path
      strcat( fullPath, sBP_MCPath );
      strcat( fullPath, "/" );
   }

   strcat( fullPath, name );
   
   lastSlash = strrchr( fullPath, '/' );
   filenameOnly = lastSlash+1;
   strncpy( folderOnly, fullPath, filenameOnly - fullPath );

   //Perform the search.
   {
      int fd;
      CellFsDirent entry;
      u_long64 data_count;

      ret = cellFsOpendir( folderOnly, &fd );
      if( ret == CELL_FS_SUCCEEDED )
      {
         for( ret = cellFsReaddir( fd, &entry, &data_count ); ; ret = cellFsReaddir( fd, &entry, &data_count ) )
         {
            sceMcTblGetDir * nextFile = table + entcount;
            if( ret != CELL_FS_SUCCEEDED )
               BP_BREAK;
            if( data_count == 0 )
            {
               //no more entries
               break;
            }
            if( entcount == maxent )
            {
               //no more space for entries
               break;
            }
            if( !strcmp(entry.d_name, ".") || !strcmp(entry.d_name,"..") )
            {
               //skip these
               continue;
            }

            if( pattern_match( entry.d_name, filenameOnly ) )
            {
               //record the next file.
               memset( nextFile, 0, sizeof( *nextFile ) );
               strncpy( (char*)nextFile->EntryName, entry.d_name, sizeof( nextFile->EntryName ) );
               if( entry.d_type == CELL_FS_TYPE_DIRECTORY )
               {
                  nextFile->AttrFile |= 0x0020; //sceMcFileAttrSubdir
               }
               else
               {
                  char fullFilename[FILENAME_MAX];
                  CellFsStat stat;
                  strcpy( fullFilename, folderOnly );
                  strcat( fullFilename, entry.d_name );
                  ret = cellFsStat( fullFilename, &stat );
                  if( ret != CELL_FS_SUCCEEDED )
                     BP_BREAK;
                  nextFile->FileSizeByte = (unsigned)stat.st_size;
               }
               ++entcount;
            }
         }
      }
      cellFsClosedir( fd );
   }
   return entcount;
}

#elif defined( BP_360 )

int bp_do_get_dir_360( const char *name, int maxent, sceMcTblGetDir *table)
{
   int entcount = 0;
   int ret;
   char fullPath[FILENAME_MAX] = { 0 };
   char folderOnly[FILENAME_MAX] = { 0 };
   const char * lastSlash;
   const char * filenameOnly;

   strcpy( fullPath, skBP_MCRootPath );
   if( *name != '/' )
   {
      //Need to concatenate current MC path
      strcat( fullPath, sBP_MCPath );
      strcat( fullPath, "/" );
   }

   strcat( fullPath, name );

   lastSlash = strrchr( fullPath, '/' );
   filenameOnly = lastSlash+1;
   strncpy( folderOnly, fullPath, filenameOnly - fullPath );

   //Perform the search.
   {
      WIN32_FIND_DATA wfd;
      HANDLE hFind;
      char fullPathFilter[FILENAME_MAX];
      strcpy( fullPathFilter, fullPath );
      fix_mc_name_filter( fullPathFilter );
      BP_UnifyPathX360( fullPathFilter );

      // Start the find and check for failure.
      hFind = FindFirstFile( fullPathFilter, &wfd );

      if( INVALID_HANDLE_VALUE == hFind )
      {
      }
      else
      {
         // Display each file and ask for the next.
         do
         {
            sceMcTblGetDir * nextFile = table + entcount;

            if( entcount == maxent )
            {
               //no more space for entries
               break;
            }
            if( !strcmp(wfd.cFileName, ".") || !strcmp(wfd.cFileName,"..") )
            {
               //skip these
               continue;
            }

            if( pattern_match( wfd.cFileName, filenameOnly ) )
            {
               //record the next file.
               memset( nextFile, 0, sizeof( *nextFile ) );
               strncpy( (char*)nextFile->EntryName, wfd.cFileName, sizeof( nextFile->EntryName ) );
               if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
               {
                  nextFile->AttrFile |= 0x0020; //sceMcFileAttrSubdir
               }
               else
               {
                  nextFile->FileSizeByte = (unsigned)wfd.nFileSizeLow;
               }
               ++entcount;
            }

         } while( FindNextFile( hFind, &wfd ) );

         // Close the find handle.
         FindClose( hFind );
      }
   }
   return entcount;
}

#endif

int BP_sceMcGetDir(int port, int slot, const char *name, unsigned int mode, int maxent, sceMcTblGetDir *table)
{
   int entcount = 0;
   int ret;
   int bp_async_flag = 0;

   //Shouldn't get here except with the first memory card port / slot.
   if( port != 0 )
      BP_BREAK;
   if( slot != 0 )
      BP_BREAK;
   if( mode != 0 )
      BP_BREAK;

   if( maxent <= 0 )
      BP_TODO_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   bp_async_flag = MGS_SaveStatus_GetDir( name, maxent, table );
   bp_async_flag *= BP_MC_ASYNC_FLAG;

#else

#ifdef BP_WIN32

   {
      const char * usename = name;
      int rootDir = ( name[0] == '/' );

      if( !rootDir )
      {
         //Subfolder; temporarily chdir to the current folder.
         bp_temp_mc_chdir_push();
      }
      else
      {
         //Root folder; strip the leading '/'
         int ret = chdir( skBP_MCRootPath );
         if( ret == -1 )
            BP_BREAK;
         ++usename;
      }

      entcount = bp_do_get_dir_win( usename, maxent, table);

      if( !rootDir )
      {
         bp_temp_mc_chdir_pop();
      }
      else
      {
         //Get back out of root folder
         ret = chdir( ".." );
         if( ret == -1 )
            BP_BREAK;
      }
   }
#elif defined(BP_PS3)
   {
      entcount = bp_do_get_dir_ps3( name, maxent, table );
   }
#elif defined(BP_360)
   {
      entcount = bp_do_get_dir_360( name, maxent, table );
   }
#elif defined(BP_VITA)
   // VITA TODO MEMCARD
   entcount = 0;
#else
#  error Unknown platform!
#endif

#endif

   sBP_MCResult = entcount;
   sBP_MCCmd = bp_async_flag | sceMcFuncNoGetDir;

   return 0;
}

int BP_sceMcMkdir(int port, int slot, const char *name)
{
   if( port != 0 )
      BP_BREAK;
   if( slot != 0 )
      BP_BREAK;
   if( *name != '/' )
      BP_TODO_BREAK; //it's a subfolder...

#ifndef BP_PS2_SAVE_EMULATION
   //Don't need to do anything at this step.
   //The directory will automatically be created when the
   //save data is created.
   sBP_MCResult = 0;
#else

#ifdef BP_WIN32
   {
      int ret;
      const char * useName = name;
      int rootDir = ( name[0] == '/' );

      if( !rootDir )
      {
         //Change to current folder first.
         bp_temp_mc_chdir_push();
      }
      else
      {
         //Root folder; strip the leading '/'
         int ret = chdir( skBP_MCRootPath );
         if( ret == -1 )
            BP_BREAK;
         ++useName;
      }

      ret = mkdir( useName );

      if( ret == -1 )
      {
         if( errno == EEXIST )
         {
#if 1
            sBP_MCResult = 0;    //MGS doesn't like it if we return an error in this case
#else
            sBP_MCResult = -4;  //directory already exists
#endif
         }
         else
         {
            //Unexpected error!
            BP_BREAK;
         }
      }
      else
      {
         //success
         sBP_MCResult = 0;
      }

      if( !rootDir )
      {
         bp_temp_mc_chdir_pop();
      }
      else
      {
         //Get back out of root folder
         ret = chdir( ".." );
         if( ret == -1 )
            BP_BREAK;
      }
   }
#elif defined(BP_PS3)
   {
      int ret;
      char fullPath[FILENAME_MAX];
      sprintf( fullPath, "%s%s", skBP_MCRootPath, name );
      ret = cellFsMkdir( fullPath, CELL_FS_DEFAULT_CREATE_MODE_1 );
      if( ret != CELL_FS_SUCCEEDED && ret != CELL_FS_EEXIST )
         BP_BREAK;
      sBP_MCResult = 0;
   }
#elif defined(BP_360)
   {
      int ret;
      char fullPath[FILENAME_MAX];
      sprintf( fullPath, "%s%s", skBP_MCRootPath, name );
      BP_UnifyPathX360( fullPath );
      ret = CreateDirectory( fullPath, NULL );
      sBP_MCResult = 0;
   }
#else
   BP_TODO_BREAK;
#endif

#endif

   sBP_MCCmd = sceMcFuncNoMkdir;

   return 0;
}

int BP_sceMcChdir(int port, int slot, const char *path, char *pwd)
{
   if( port != 0 )
      BP_BREAK;
   if( slot != 0 )
      BP_BREAK;

   if( !strcmp( path, ".." ) || !strcmp( path, "." ) )
   {
      BP_TODO_BREAK;
   }
   if( *path != '/' )
      BP_TODO_BREAK; //it's a subfolder...

#ifndef BP_PS2_SAVE_EMULATION

   MGS_SaveStatus_ChDir( path );
   sBP_MCResult = 0;

#else

#ifdef BP_WIN32

   {
      char fullPath[FILENAME_MAX];
      char newMCPath[FILENAME_MAX] = "";
      int ret;
      WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
      BOOL b;
      strcpy( fullPath, skBP_MCRootPath );
      if( *path != '/' )
      {
         //subfolder of current path.
         strcat( fullPath, sBP_MCPath );
         strcat( newMCPath, "/" );
         strcat( newMCPath, path );
      }
      else
      {
         //starting with a forward slash; replace entire BP MC path
         strcat( newMCPath, path );
      }
      //Force to end with a forward slash.
      if( *(newMCPath + strlen(newMCPath)-1) != '/' )
      {
         strcat( newMCPath, "/" );
      }
      strcat( fullPath, newMCPath );
      b = GetFileAttributesExA( fullPath, GetFileExInfoStandard, &fileAttr );
      if( b!=0 && ( ( fileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) )
      {
         //OK.
         //Store folder and assume we can chdir to it.
         strcpy( sBP_MCPath, newMCPath );

         sBP_MCResult = 0;
      }
      else
      {
         //Not found, not a directory, or other error.
         sBP_MCResult = -4;
      }
   }
#elif defined(BP_PS3)
   {
      strcpy( sBP_MCPath, path );
      sBP_MCResult = 0;
   }
#elif defined(BP_360)
   {
      strcpy( sBP_MCPath, path );
      sBP_MCResult = 0;
   }
#else
   BP_TODO_BREAK;
#endif

#endif

   sBP_MCCmd = sceMcFuncNoChDir;

   return 0;
}

int BP_sceMcOpen(int port, int slot, const char *name, int mode)
{
   if( port != 0 )
      BP_BREAK;
   if( slot != 0 )
      BP_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   MGS_SaveStatus_Open( name, mode );

#else

#ifdef BP_WIN32
   {
      FILE * fd;
      const char * modestr = NULL;

      bp_temp_mc_chdir_push();

      switch( mode )
      {
      case SCE_RDONLY:
         modestr = "rb";
         break;
      case SCE_WRONLY:
      case SCE_CREAT:
      case ( SCE_WRONLY | SCE_CREAT ):
         modestr = "wb";
         break;
      case (SCE_RDWR | SCE_CREAT):
         modestr = "w+b";
         break;
      default:
         BP_TODO_BREAK;
      }
      fd = fopen( name, modestr );

      bp_temp_mc_chdir_pop();

      if( fd != NULL )
      {
         sBP_MCResult = (int)fd;
      }
      else
      {
         sBP_MCResult = -1;
      }
   }
#elif defined(BP_PS3)
   {
      int ret;
      int flags;
      char fullPath[FILENAME_MAX];
      int fd;
      sprintf( fullPath, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, name );

      switch( mode )
      {
      case SCE_RDONLY:
         flags = CELL_FS_O_RDONLY;
         break;
      case SCE_WRONLY:
         flags = CELL_FS_O_WRONLY;
         break;
      case ( SCE_WRONLY | SCE_CREAT ):
         flags = CELL_FS_O_WRONLY | CELL_FS_O_CREAT;
         break;
      case (SCE_RDWR | SCE_CREAT):
         flags = CELL_FS_O_RDWR | CELL_FS_O_CREAT;
         break;
      default:
         BP_TODO_BREAK;
      }
      ret = cellFsOpen( fullPath, flags, &fd, NULL, 0 );
      if( ret == CELL_FS_SUCCEEDED )
      {
         sBP_MCResult = fd;
      }
      else
      {
         sBP_MCResult = -1;
      }
   }
#elif defined(BP_360)
   {
      HANDLE fd;
      char fullPath[FILENAME_MAX];
      DWORD desiredAccess;
      DWORD shareMode;
      DWORD creationDisposition;
      DWORD flagsAnsAttributes;
      sprintf( fullPath, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, name );

      switch( mode )
      {
      case SCE_RDONLY:
         desiredAccess = GENERIC_READ;
         shareMode = FILE_SHARE_READ;
         creationDisposition = OPEN_EXISTING;
         break;
      case SCE_WRONLY:
      case ( SCE_WRONLY | SCE_CREAT ):
         desiredAccess = GENERIC_WRITE;
         shareMode = FILE_SHARE_WRITE;
         creationDisposition = CREATE_ALWAYS;
         break;
      default:
         BP_TODO_BREAK;
      }
      BP_UnifyPathX360( fullPath );
      fd = CreateFile(fullPath, desiredAccess, shareMode, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
      if( fd == INVALID_HANDLE_VALUE )
      {
         sBP_MCResult = -1;
      }
      else
      {
         sBP_MCResult = (int)fd;
      }
   }
#else
   BP_TODO_BREAK;
#endif

#endif

   sBP_MCCmd = sceMcFuncNoOpen;

   return 0;
}

int BP_sceMcDelete( int slot, int port, const char* name )
{
   unsigned int bp_async_flag = 0;

   if( port != 0 )
      BP_BREAK;
   if( slot != 0 )
      BP_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   bp_async_flag = MGS_SaveStatus_Delete( name );
   bp_async_flag *= BP_MC_ASYNC_FLAG;

#else

#ifdef BP_WIN32
   {
      int ret;
      const char * useName = name;
      int rootDir = ( name[0] == '/' );

      if( !rootDir )
      {
         //Change to current folder first.
         bp_temp_mc_chdir_push();
      }
      else
      {
         //Root folder; strip the leading '/'
         int ret = chdir( skBP_MCRootPath );
         if( ret == -1 )
            BP_BREAK;
         ++useName;
      }

      ret = _unlink( useName );
      if( ret )
         BP_BREAK;

      sBP_MCResult = 0;

      if( !rootDir )
      {
         bp_temp_mc_chdir_pop();
      }
      else
      {
         //Get back out of root folder
         ret = chdir( ".." );
         if( ret == -1 )
            BP_BREAK;
      }
   }
#elif defined(BP_PS3)
   {
      char fullPath[FILENAME_MAX];
      sprintf( fullPath, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, name );
      cellFsUnlink( fullPath );
   }
#elif defined(BP_360)
   {
      char fullPath[FILENAME_MAX];
      sprintf( fullPath, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, name );
      BP_UnifyPathX360( fullPath );
      DeleteFile( fullPath );
   }
#else
   BP_TODO_BREAK;
#endif

#endif

   sBP_MCCmd = bp_async_flag | sceMcFuncNoDelete;

   return 0;
}

int BP_sceMcRead(int fd, void *buff, int size)
{
   unsigned int bp_async_flag = 0;

   if( fd == -1 )
      BP_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   bp_async_flag = MGS_SaveStatus_Read( buff, size );
   bp_async_flag *= BP_MC_ASYNC_FLAG;
   sBP_MCResult = size;

#else

#ifdef BP_WIN32
   {
      FILE * fp = (FILE*)fd;
      int n = fread( buff, 1, size, fp );
      if( n != size )
         BP_BREAK;

      sBP_MCResult = n;
   }
#elif defined(BP_PS3)
   {
      int ret = cellFsRead( fd, buff, size, NULL );
      if( ret != CELL_FS_SUCCEEDED )
         BP_BREAK;
      sBP_MCResult = size;
   }
#elif defined(BP_360)
   {
      int readCount;
      int readResult = ReadFile((HANDLE)fd, buff, size, (LPDWORD)&readCount, NULL);
      sBP_MCResult = readCount;
   }
#else
   BP_TODO_BREAK;
#endif

#endif

   sBP_MCCmd = bp_async_flag | sceMcFuncNoRead;

   return 0;
}

int BP_sceMcWrite(int fd, const void *buff, int size)
{
   unsigned int bp_async_flag = 0;

   if( fd == -1 )
      BP_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   bp_async_flag = MGS_SaveStatus_Write( buff, size );
   bp_async_flag *= BP_MC_ASYNC_FLAG;
   sBP_MCResult = size;

#else

#ifdef BP_WIN32
   {
      FILE * fp = (FILE*)fd;
      int n = fwrite( buff, 1, size, fp );
      if( n != size )
         BP_BREAK;

      sBP_MCResult = n;
   }
#elif defined(BP_PS3)
   {
      int ret = cellFsWrite( fd, buff, size, NULL );
      if( ret != CELL_FS_SUCCEEDED )
         BP_BREAK;
      sBP_MCResult = size;
   }
#elif defined(BP_360)
   {
      int writeCount;
      int writeResult = WriteFile((HANDLE)fd, buff, size, (LPDWORD)&writeCount, NULL);
      sBP_MCResult = writeCount;
   }
#else
   BP_TODO_BREAK;
#endif

#endif

   sBP_MCCmd = bp_async_flag | sceMcFuncNoWrite;

   return 0;
}

int BP_sceMcClose(int fd)
{
   DMC_PRINTF( "BP_sceMcClose: %d\n", fd );
   
   if( fd == -1 )
      BP_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   if ( MGS_SaveStatus_CloseReturnError() != 0 )
   {
      // Close error because the file is already closed
      return -1;
   }

#else

#ifdef BP_WIN32
   {
      FILE * fp = (FILE*)fd;
      int ret = fclose( fp );
      if( ret != 0 )
         BP_BREAK;
   }
#elif defined(BP_PS3)
   {
      int ret = cellFsClose( fd );
      if( ret != CELL_FS_SUCCEEDED )
         BP_BREAK;
   }
#elif defined(BP_360)
   {
      int ret = CloseHandle((HANDLE)fd);
      if( !ret )
         BP_BREAK;
   }
#else
   BP_TODO_BREAK;
#endif

#endif

   sBP_MCResult = 0;
   sBP_MCCmd = sceMcFuncNoClose;

   return 0;
}

int BP_sceMcRename( int slot, int port, const char* org, const char* dst )
{
   unsigned int bp_async_flag = 0;
   int ret;

   if( port != 0 )
      BP_BREAK;
   if( slot != 0 )
      BP_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   bp_async_flag = MGS_SaveStatus_Rename( org, dst );
   bp_async_flag *= BP_MC_ASYNC_FLAG;

#else

#ifdef BP_WIN32
   {
      bp_temp_mc_chdir_push();
      ret = rename( org, dst );
      if( ret )
         BP_BREAK;
      bp_temp_mc_chdir_pop();
   }
#elif defined(BP_PS3)
   {
      char fullPathOrg[FILENAME_MAX];
      char fullPathDst[FILENAME_MAX];
      sprintf( fullPathOrg, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, org );
      sprintf( fullPathDst, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, dst );
      ret = cellFsRename( fullPathOrg, fullPathDst );
      if( ret != CELL_FS_SUCCEEDED )
         BP_BREAK;
   }
#elif defined(BP_360)
   {
      char fullPathOrg[FILENAME_MAX];
      char fullPathDst[FILENAME_MAX];
      sprintf( fullPathOrg, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, org );
      sprintf( fullPathDst, "%s%s/%s", skBP_MCRootPath, sBP_MCPath, dst );
      BP_UnifyPathX360( fullPathOrg );
      BP_UnifyPathX360( fullPathDst );
      ret = MoveFile(fullPathOrg,fullPathDst);
      if( !ret )
         BP_BREAK;
   }
#else
   BP_TODO_BREAK;
#endif

#endif //BP_PS2_SAVE_EMULATION

   sBP_MCResult = 0;
   sBP_MCCmd = bp_async_flag | sceMcFuncNoRename;

   return 0;
}

int BP_sceMcSeek( int fd, int offset, int mode )
{
   int ret;

   if( fd == -1 )
      BP_BREAK;

#ifndef BP_PS2_SAVE_EMULATION

   MGS_SaveStatus_Seek( offset, mode );

#else

#ifdef BP_WIN32
   {
      FILE * fp = (FILE*)fd;
      ret = fseek( fp, offset, mode );
      if( ret )
         BP_BREAK;
   }
#elif defined(BP_PS3)
   {
      uint64_t pos = 0;
      int ret = cellFsLseek( fd, offset, mode, &pos );
      if( ret != CELL_FS_SUCCEEDED )
         BP_BREAK;
   }
#elif defined(BP_360)
   {
      int ret = SetFilePointer((HANDLE)fd, offset, NULL, mode);
      if( ret == INVALID_SET_FILE_POINTER )
         BP_BREAK;
   }
#else
   BP_TODO_BREAK;
#endif

#endif //BP_PS2_SAVE_EMULATION

   sBP_MCResult = 0;
   sBP_MCCmd = sceMcFuncNoSeek;
   
   return 0;
}

int BP_sceMcSync( int mode, int *pcmd, int *presult )
{
   //mode == 1: async completion check
   //mode == 0: sync wait
#ifndef BP_PS2_SAVE_EMULATION

   int cmd = sBP_MCCmd & (~BP_MC_ASYNC_FLAG);
   unsigned int bp_async_flag = sBP_MCCmd & BP_MC_ASYNC_FLAG;
   if( pcmd )
      *pcmd = cmd;

   if( bp_async_flag )
   {
      //Commands that are performed asynchronously for us
      //through the BP_SaveLoad interface.
      for(;;)
      {
         if( MGS_SaveStatus_IsDone() )
         {
            //Clear loading flag.
            BP_SetSpinnerLoadFlag(kLoadFlag_SaveLoad, 0);

            if( MGS_SaveStatus_CompletedOK() )
            {
               switch( cmd )
               {
               case sceMcFuncNoGetDir:
                  {
                     //Result is the number of entries.
                     sBP_MCResult = MGS_SaveStatus_GetDirResult();
                  }
                  break;
               case sceMcFuncNoCardInfo:
                  {
                     sBP_MCResult = MGS_SaveStatus_GetInfoResult();
                     if( sBP_MCResult == 19 )
                     {
                        //No device inserted.
                        //Convert from SCE_ENODEV to an sceMc* code that MGS2 will understand.
                        sBP_MCResult = -11;
                        //Must also clear out fetched info values since game checks them anyway
                        //and gets stuck in a retry loop if they are set to valid card values.
                        if( spBP_MCType )
                           *spBP_MCType = 0;
                        if( spBP_MCFree )
                           *spBP_MCFree = 0;
                        if( spBP_MCFormat )
                           *spBP_MCFormat = 0;
                     }
                  }
               case sceMcFuncNoRead:
               case sceMcFuncNoWrite:
                  {
                     //Result already stored as number of bytes.
                  }
                  break;
               default:
                  {
                     //success.
                     sBP_MCResult = 0;
                  }
                  break;
               }
            }
            else
            {
               switch(cmd)
               {
               case sceMcFuncNoWrite:
                  //Corrupt, no space, or other error.
                  //This generic error code will result in the MGS save screen putting up an error message at the
                  //bottom of the screen.
                  //The one specific case we need to handle beyond that is for "no space".
#ifdef BP_PS3
                  if( MGS_SaveStatus_HasNoSpace() )
                  {
                     //BP - Put up a system UI to let them know why the save failed.
                     int result = 0;
                     ShowLowDiskSpaceWarning(&result, MGS_SaveStatus_ExtraSpaceReq());  // non-NULL result pointer means we want control back
                  }
#elif defined(BP_360)
                  if( MGS_SaveStatus_HasNoSpace() )
                  {
                     //BP - Put up a system UI to let them know why the save failed.
                     ShowGenericOsDialog_Sync(2/*kDRT_NotEnoughSpace*/);
                  }
#endif

                  //"Data could not be written due to insufficient free space"
                  //This will result in a generic error in MCAccessWrite()
                  sBP_MCResult = -3; //sceMcResFullDevice
                  break;

               case sceMcFuncNoRead:
                  {
                     if( MGS_SaveStatus_IsCorrupted() )
                     {
                        //Don't wait for it to get to the point of checking the crc against the data,
                        //we know this load failed.
                        sBP_MCResult = -3; //sceMcResFullDevice
                     }
                     else
                     {
                        //"File is damaged and could not be read"
                        //This will result in a generic error in MCAccessRead()
                        sBP_MCResult = -3; //sceMcResFullDevice
                     }
                  }
                  break;

               default:
                  //No other cases *should* throw an error through our saveload interface, but just to be sure...
                  sBP_MCResult = -3; //sceMcResFullDevice
                  break;
               }
            }

            if( presult )
               *presult = sBP_MCResult;
            return 1;
         }
         else
         {
            //Still loading.
            BP_SetSpinnerLoadFlag(kLoadFlag_SaveLoad, 1);
         }

         if( mode == 1 )
         {
            //Do not stall for completion.
            break;
         }
         //TODO: sleep thread
      }

      //async operation not complete.
      return 0;
   }
   else
   {
      //Commands that complete immediately for us.
      //They have already set the proper result.
      if( presult )
         *presult = sBP_MCResult;
      return 1;
   }

#else

   if( pcmd )
      *pcmd = sBP_MCCmd;
   if( presult )
      *presult = sBP_MCResult;

   return 1; //finished
#endif
}

/* ------------------------------------------------------------------------ */


#define MCAccessStep(_work,_level)	((_work)->step[(_level)])

#define MCAccessStepClear(_work,_level) \
do{ \
	int _i; \
	for(_i=(_level);_i<STEP_LEVEL_MAX;_i++) (_work)->step[_i]=0; \
}while(0)


#ifdef DEBUG
// #define STEP_CHECK
#endif


#ifdef STEP_CHECK

#define MCAccessStepUp(_work,_level) \
do{ \
	printf("MCAccessStep Up   ( %d ) : %d -> %d\n", \
		   (_level),(_work)->step[(_level)],(_work)->step[(_level)]+1); \
	(_work)->step[(_level)]++; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepDown(_work,_level) \
do{ \
	printf("MCAccessStep Down ( %d ) : %d -> %d\n", \
		   (_level),(_work)->step[(_level)],(_work)->step[(_level)]-1); \
	(_work)->step[(_level)]--; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepSet(_work,_level,_val) \
do{ \
	printf("MCAccessStep Set  ( %d ) : %d -> %d\n", \
		   (_level),(_work)->step[(_level)],(_val)); \
	(_work)->step[(_level)]=(_val); \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#else

#define MCAccessStepUp(_work,_level) \
do{ \
	(_work)->step[(_level)]++; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepDown(_work,_level) \
do{ \
	(_work)->step[(_level)]--; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepSet(_work,_level,_val) \
do{ \
	(_work)->step[(_level)]=(_val); \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#endif


#define MAIN_STEP			MCAccessStep(work,0)
#define MAIN_STEP_UP()		MCAccessStepUp(work,0)
#define MAIN_STEP_DOWN()	MCAccessStepDown(work,0)
#define MAIN_STEP_SET(val)	MCAccessStepSet(work,0,val)

#define SUB_STEP			MCAccessStep(work,1)
#define SUB_STEP_UP()		MCAccessStepUp(work,1)
#define SUB_STEP_DOWN()		MCAccessStepDown(work,1)
#define SUB_STEP_SET(val)	MCAccessStepSet(work,1,val)


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

/* 直前にロードしたゲームデータのID番号を記録する
   本来ならmcman/の方に書かれるべきであるが、常にメモリにあり続けるので、
   メモリの逼迫をさけるために低レベル関数側に書いた。*/
unsigned short mc_loadport;
unsigned short mc_loaddata_id;

/* ロードしたデータの日付 */
unsigned int mc_loaddata_time;

/* 秒単位の日付の計算用 */
const unsigned char conv_month_day[]={
	0,31,28,31,30,31,30,31,31,30,31,30,31,
};


#define DIFFERENT_CARD_0		(-0x200)
#define DIFFERENT_CARD_1		(-0x100)


/* ------------------------------------------------------------------------ */


unsigned int ComGetLoaddataTime(void)
{
#ifdef DEBUG_MODE
	printf("Get Load Data Date = %d\n",mc_loaddata_time);
#endif

	return mc_loaddata_time;
}

unsigned int ComGetNowTime(void)
{
	unsigned int dt=0;
	int i;
	unsigned int days;
	extern const unsigned char conv_month_day[];
	int year,month,day,hour,min,sec;

	/* これはBCD */
	year=(G_rtc.year>>4)*10+(G_rtc.year & 0x0f);
	month=(G_rtc.month>>4)*10+(G_rtc.month & 0x0f);
	day=(G_rtc.day>>4)*10+(G_rtc.day & 0x0f);
	hour=(G_rtc.hour>>4)*10+(G_rtc.hour & 0x0f);
	min=(G_rtc.minute>>4)*10+(G_rtc.minute & 0x0f);
	sec=(G_rtc.second>>4)*10+(G_rtc.second & 0x0f);


	/* year */
	dt=year;
	dt*=365;

	/* month,day */
	days=0;
	for(i=1;i<month;i++){
		days+=conv_month_day[i];
	}
	days+=day;

	/* 閏年 */
	days+=(year+3)/4;
	if((year % 4)==0){
		if(month>2) days++;
	}
	dt+=days;
	dt*=24;

	/* hour */
	dt+=hour;
	dt*=60;

	/* min */
	dt+=min;
	dt*=60;

	/* sec */
	dt+=sec;

#ifdef DEBUG_MODE
	printf("Get Now Date = %d\n",dt);
#endif

	return dt;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

#if defined( PAL ) || defined( SCENARIO_SET_SONYCODE )

/* シナリオ側でプロダクトコードを設定できるような仕掛け */
static char pfilename[0x30];
static char gfilename[0x30];
static char vfilename[0x30];
static char sfilename[0x30];
static char afilename[0x30];

int ComSetProductCode(void)
{
	char *pcode=NULL;
	pcode=GCL_GetNextString();
	if(pcode==NULL){
		pfilename[0]='\0';
		gfilename[0]='\0';
		vfilename[0]='\0';
		sfilename[0]='\0';
		afilename[0]='\0';
		return 0;
	}
	sprintf(pfilename,_MC_PFILE_NAME,pcode);
	sprintf(gfilename,_MC_GFILE_NAME,pcode);
	sprintf(vfilename,_MC_VFILE_NAME,pcode);
	sprintf(sfilename,_MC_SFILE_NAME,pcode);
	sprintf(afilename,_MC_AFILE_NAME,pcode);

   strcpy( bp_basefilename, pcode );

#ifdef DEBUG_MODE
	printf("pfilename = %s\n",pfilename);
	printf("gfilename = %s\n",gfilename);
	printf("vfilename = %s\n",vfilename);
	printf("sfilename = %s\n",sfilename);
	printf("afilename = %s\n",afilename);
#endif

	return 0;
}

char *GetPFilename(void)
{
	return pfilename;
}

char *GetGFilename(void)
{
	return gfilename;
}

char *GetVFilename(void)
{
	return vfilename;
}

char *GetSFilename(void)
{
	return sfilename;
}

char *GetAFilename(void)
{
	return afilename;
}

#else

int ComSetProductCode(void)
{
	return 0;
}

char *GetPFilename(void)
{
	return MC_PFILE_NAME;
}

char *GetGFilename(void)
{
	return MC_GFILE_NAME;
}

char *GetVFilename(void)
{
	return MC_VFILE_NAME;
}

char *GetSFilename(void)
{
	return MC_SFILE_NAME;
}

#endif

// Document のデータがあるかないかを調べるためのしくみ
char ofilename[0x30];
void SetOtherFilename( char* str )
{
	strncpy( ofilename, str, sizeof(ofilename) );
}
char *GetOtherFilename(void)
{
	return ofilename;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* ヘッダ */
typedef struct {
	int version_le;
} PS2ICON_FILEHEADER;

/* モデル */
typedef struct {
	int nbsp_le;
	int attrib_le;
	float bface_le;
	int nbvtx_le;
} PS2ICON_MDLHEADER;

typedef struct {
	short vtx_le[4];
} PS2ICON_MDLVERTEX;

typedef struct {
	short normal_le[4];
	short st_le[2];
	unsigned char color_le[4];
} PS2ICON_MDLVERTEXINFO;

/* アニメーション */
typedef struct {
	int nbseq_le;
} PS2ICON_ANIMSEQHEADER;

typedef struct {
	int nbframe_le;
	float speed_le;
	int offset_le;
	int nbksp_le;
} PS2ICON_ANIMSEQ;

typedef struct {
	int kspid_le;
	int nbkf_le;
} PS2ICON_ANIMSHAPE;

typedef struct {
	float frame_le;
	float weight_le;
} PS2ICON_ANIMFRAME;


/* PS2のアイコンファイルのサイズを計算して返す */
int MCAccessCalcIconSize(void *icon)
{
	int size=0;
	int i,j;

	PS2ICON_FILEHEADER *fileheader;
	PS2ICON_MDLHEADER *mdlheader;
	PS2ICON_MDLVERTEX *mdlvertex;
	PS2ICON_ANIMSEQHEADER *animseqheader;
	PS2ICON_ANIMSEQ *animseq;
	PS2ICON_ANIMSHAPE *animshape;
	PS2ICON_ANIMFRAME *animframe;

	fileheader=(PS2ICON_FILEHEADER *)icon;
	size+=sizeof(PS2ICON_FILEHEADER);

	mdlheader=(PS2ICON_MDLHEADER *)((int)icon+size);
	size+=sizeof(PS2ICON_MDLHEADER);

	mdlvertex=(PS2ICON_MDLVERTEX *)((int)icon+size);
	size+=(sizeof(PS2ICON_MDLVERTEX)*BP_LE_SwapSInt(mdlheader->nbsp_le)+
		   sizeof(PS2ICON_MDLVERTEXINFO))*BP_LE_SwapSInt(mdlheader->nbvtx_le);

	animseqheader=(PS2ICON_ANIMSEQHEADER *)((int)icon+size);
	size+=sizeof(PS2ICON_ANIMSEQHEADER);

	for(i=0;i<BP_LE_SwapSInt(animseqheader->nbseq_le);i++){
		animseq=(PS2ICON_ANIMSEQ *)((int)icon+size);
		size+=sizeof(PS2ICON_ANIMSEQ);

		for(j=0;j<BP_LE_SwapSInt(animseq->nbksp_le);j++){
			animshape=(PS2ICON_ANIMSHAPE *)((int)icon+size);
			size+=sizeof(PS2ICON_ANIMSHAPE);

			animframe=(PS2ICON_ANIMFRAME *)((int)icon+size);
			size+=sizeof(PS2ICON_ANIMFRAME)*BP_LE_SwapSInt(animshape->nbkf_le);
		}
	}

	if(BP_LE_SwapSInt(mdlheader->attrib_le) & 0x08){
		int *texsize=(int *)((int)icon+size);
		size+=BP_LE_SwapSInt(*texsize)+sizeof(int);
	}
	else{
		size+=0x8000;
	}

#ifdef DEBUG
	printf("Icon size = %d\n",size);
#endif

	return size;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void PortSlotNumUpdate(MCAccessWork *work)
{
	work->SelSlot++;
	if(work->SelSlot>=work->SlotMax[work->SelPort]){
		work->SelSlot=0;

		work->SelPort++;
		if(work->SelPort>=PORT_MAX) work->SelPort=0;
	}
}

static int RetryCountup(MCAccessWork *work)
{
	work->RetryCount++;
	if(work->RetryCount>=RETRY_COUNT_MAX) return 1;
	return 0;
}

int MCAccessPrecheckStart(MCAccessWork *work)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	work->AutoPortCheck_Counter=PORTCHECK_COUNT_MAX;
	if(BP_sceMcGetInfo(work->SelPort,work->SelSlot,&(work->TmpType),NULL,NULL)!=0) return 0;

	MAIN_STEP_SET(MCAccess_PRECHECK);
	return 1;
}

static int MCAccessPrecheck(MCAccessWork *work)
{
	int cmd,result;

	switch(BP_sceMcSync(1,&cmd,&result)){
	case 0:
		break;
	case 1:
		if(result==0){
			return 1;
		}
		else if(( result<-10 
#if (SCE_LIBRARY_VERSION >= 0x2400)
				  // ADD M.Kobayashi 2002/06/19
				  // 2.4.x からPS１メモリーカードはこれを返しくさる
				  || result == sceMcResDeniedPermit
#endif
				  )
				&& work->Type[work->SelPort][work->SelSlot]==work->TmpType){
			return 1;
		}
		else{
			work->Type[work->SelPort][work->SelSlot]=DIFFERENT_CARD_0;
			return -1;
		}
		break;
	case -1:
		work->Type[work->SelPort][work->SelSlot]=DIFFERENT_CARD_0;
		return -1;
	}
	return 0;
}

static int MCAccessPrecheck2Check(MCAccessWork *work)
{
	int tmp_max;
	int i,j;

#if 1 //BP_PS2
   work->SlotMax[0] = 1;
   work->SlotMax[1] = 1;
#else
	tmp_max=sceMcGetSlotMax(0);
	if(tmp_max>0) work->SlotMax[0]=tmp_max;
	else return 0;

	tmp_max=sceMcGetSlotMax(1);
	if(tmp_max>0) work->SlotMax[1]=tmp_max;
	else return 0;
#endif

	MAIN_STEP_SET(MCAccess_CHECK);
	work->RetryCount=0;
	work->Result=0;

	for(i=0;i<PORT_MAX;i++){
		for(j=0;j<SLOT_MAX;j++){
			work->OldType[i][j]=work->Type[i][j];
		}
	}

#ifdef DEBUG
	printf("Portcheck Start\n");
#endif
	return 1;
}

int MCAccessCheckStart(MCAccessWork *work)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;
	return MCAccessPrecheck2Check(work);
}

static int MCAccessCheck(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0x00: /* port=0 , slot=0 */
	case 0x10: /* port=0 , slot=1 */
	case 0x20: /* port=0 , slot=2 */
	case 0x30: /* port=0 , slot=3 */

	case 0x40: /* port=1 , slot=0 */
	case 0x50: /* port=1 , slot=1 */
	case 0x60: /* port=1 , slot=2 */
	case 0x70: /* port=1 , slot=3 */
#ifdef KP_XBOX
	case 0x80:	/* port=2 , slot=0 終了条件時に通る 2002/03/20 ADD M.Kobayashi */
#endif		
		{
			int port,slot;
			int slot_max;

			port=(SUB_STEP>>4)>>2;
			slot=(SUB_STEP>>4) & 3;
			slot_max=work->SlotMax[port];

			if(port>=PORT_MAX){
				return 1;
			}
			if(slot>=slot_max){
				if(port){
					return 1;
				}
				else{
					SUB_STEP_SET(0x40);
					break;
				}
			}

#ifdef DEBUG_MODE
			// printf("Port Slot = %d %d\n",port,slot);
#endif

			if(BP_sceMcGetInfo(port,slot,&(work->Type[port][slot]),
							&(work->FreeSize[port][slot]),&(work->Formatted[port][slot]))==0){

				/* Success */
				SUB_STEP_UP();
			}
			else{
				/* Failed */
				if(RetryCountup(work)){
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->Type[port][slot]=-1;
					if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
					printf("Portcheck Failed1 %d %d\n",port,slot);
#endif

				}
			}
		}
		break;

	case 0x01: /* port=0 , slot=0 */
	case 0x11: /* port=0 , slot=1 */
	case 0x21: /* port=0 , slot=2 */
	case 0x31: /* port=0 , slot=3 */

	case 0x41: /* port=1 , slot=0 */
	case 0x51: /* port=1 , slot=1 */
	case 0x61: /* port=1 , slot=2 */
	case 0x71: /* port=1 , slot=3 */
		{
			int cmd,result;
			int port,slot;

			port=(SUB_STEP>>4)>>2;
			slot=(SUB_STEP>>4) & 3;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case 0:
					/* Success  case 1*/
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->RetryCount=0;
					if(work->OldType[port][slot]!=DIFFERENT_CARD_0){
						work->OldType[port][slot]=work->Type[port][slot];
					}
					break;
				case -1:
				case -2:
					/* Success  case 2*/
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->RetryCount=0;
					work->OldType[port][slot]=DIFFERENT_CARD_1;
					if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
					printf("Portcheck Failed2 %d %d\n",port,slot);
#endif
					break;
				default:
					if(result<-10
#if (SCE_LIBRARY_VERSION >= 0x2400)
					   // ADD M.Kobayashi 2002/06/19
					   // 2.4.x からPS１メモリーカードはこれを返しくさる
					   || result == sceMcResDeniedPermit
#endif
					   ){
						switch(work->Type[port][slot]){
						case 2:
							/* Retry */
							if(RetryCountup(work)){
								SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
								work->Type[port][slot]=-1;
								work->RetryCount=0;
								if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
								printf("Portcheck Failed3 %d %d\n",port,slot);
#endif
							}
							else SUB_STEP_DOWN();
							break;
						case 0:
							/* Retry */
							if(RetryCountup(work)){
								SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
								work->RetryCount=0;
								if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
								printf("Portcheck Failed4 %d %d\n",port,slot);
#endif
							}
							else SUB_STEP_DOWN();
							break;
						default:
							SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
							work->RetryCount=0;
							break;
						}
					}
					else{
						/* Retry */
						if(RetryCountup(work)){
							SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
							work->Type[port][slot]=-1;
							work->RetryCount=0;
							if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
							printf("Portcheck Failed5 %d %d\n",port,slot);
#endif
						}
						else SUB_STEP_DOWN();
					}
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)){
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->Type[port][slot]=-1;
					work->RetryCount=0;
					if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
					printf("Portcheck Failed6 %d %d\n",port,slot);
#endif
				}
				else SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessOpenStart(MCAccessWork *work,int port,int slot,char *name,int mode,int *fd)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_OPEN);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.open.name=name;
	work->arg.open.mode=mode;
	work->arg.open.fd=fd;

	return 1;
}

static int MCAccessOpen(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
      DMC_PRINTF( "MCAccessOpen: BP_sceMcOpen %d %d %s %d\n", work->SelPort,work->SelSlot,work->arg.open.name,work->arg.open.mode );

      if(BP_sceMcOpen(work->SelPort,work->SelSlot,work->arg.open.name,work->arg.open.mode)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2: //sceMcResNoFormat
					/* Unformat */
					return -2;
				case -3: //sceMcResFullDevice
				case -4: //sceMcResNoEntry
				case -5: //sceMcResDeniedPermit
				case -7:
					return -1;
               DMC_PRINTF( "MCAccessOpen: Got FD result (error) %d\n", result );
				default:
#if 1 //BP
               //On XBOX360, top bit of valid file handles is 1 so they look negative when cast to int.
               //All of our emulated sceMcOpen() functions set result -1 when an open fails.
               if(result != -1)
#else
					if(result>=0)
#endif
               {
                  DMC_PRINTF( "MCAccessOpen: Got FD %d\n", result );
						work->fd=*(work->arg.open.fd)=result;
						return 1;
					}
					else{
                  DMC_PRINTF( "MCAccessOpen: Got FD error %d\n", result );
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessCloseStart(MCAccessWork *work,int fd)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

   DMC_PRINTF( "MCAccessCloseStart: %d\n", fd );

   MAIN_STEP_SET(MCAccess_CLOSE);
	work->RetryCount=0;
	work->Result=0;

	work->arg.close.fd=fd;

	return 1;
}

static int MCAccessClose(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcClose(work->arg.close.fd)==0){
         DMC_PRINTF( "MCAccessClose: Trying %d\n", work->arg.close.fd );
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
            DMC_PRINTF( "MCAccessClose: retry %d\n", result );
				break;
			case 1:
            DMC_PRINTF( "MCAccessClose: result %d\n", result );
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
					return -1;
				default:
					if(result>=0){
						work->fd=-1;
						return 1;
					}
					else{
						work->fd=-1;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
            DMC_PRINTF( "MCAccessClose: retry %d\n", result );
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessSeekStart(MCAccessWork *work,int fd,int offset,int mode,int *fp)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_SEEK);
	work->RetryCount=0;
	work->Result=0;

	work->arg.seek.fd=fd;
	work->arg.seek.offset=offset;
	work->arg.seek.mode=mode;
	work->arg.seek.fp=fp;

	return 1;
}

static int MCAccessSeek(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcSeek(work->arg.seek.fd,work->arg.seek.offset,work->arg.seek.mode)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
					return -1;
				default:
					if(result>=0){
						if(work->arg.seek.fp!=NULL){
							*(work->arg.seek.fp)=result;
						}
						return 1;
					}
					else{
						work->fd=-1;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessReadStart(MCAccessWork *work,int fd,void *buf,int size,int *retsize)
{
   DMC_PRINTF( "MCReadStart: %d\n", fd );
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_READ);
	work->RetryCount=0;
	work->Result=0;

	work->arg.rw.fd=fd;
	work->arg.rw.buf=buf;
	work->arg.rw.size=size;
	work->arg.rw.retsize=retsize;

	return 1;
}

static int MCAccessRead(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
      DMC_PRINTF( "MCAccessRead start: %d\n", work->arg.rw.fd );
        if(BP_sceMcRead(work->arg.rw.fd,work->arg.rw.buf,work->arg.rw.size)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
            DMC_PRINTF( "MCAccessRead wait: %d\n", result );
				break;
			case 1:
            DMC_PRINTF( "MCAccessRead result: %d\n", result );
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
				case -3:
				case -5:
					return -1;
				default:
					if(result>=0){
						if(work->arg.rw.retsize) *(work->arg.rw.retsize)=result;
						return 1;
					}
					else{
						work->fd=0;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
            DMC_PRINTF( "MCAccessRead retry: %d\n", result );
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessWriteStart(MCAccessWork *work,int fd,void *buf,int size,int *retsize)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_WRITE);
	work->RetryCount=0;
	work->Result=0;

	work->arg.rw.fd=fd;
	work->arg.rw.buf=buf;
	work->arg.rw.size=size;
	work->arg.rw.retsize=retsize;

	return 1;
}

static int MCAccessWrite(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcWrite(work->arg.rw.fd,work->arg.rw.buf,work->arg.rw.size)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
				case -3:
				case -5:
				case -8:
					return -1;
				default:
					if(result>=0){
						if(work->arg.rw.retsize) *(work->arg.rw.retsize)=result;
						return 1;
					}
					else{
						work->fd=-1;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessMkdirStart(MCAccessWork *work,int port,int slot,char *name)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_MKDIR);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.mkdir.name=name;

	return 1;
}

static int MCAccessMkdir(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcMkdir(work->SelPort,work->SelSlot,work->arg.mkdir.name)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -3:
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessChdirStart(MCAccessWork *work,int port,int slot,char *name,char *pwd)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_CHDIR);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.chdir.name=name;
	work->arg.chdir.pwd=pwd;

	return 1;
}

static int MCAccessChdir(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcChdir(work->SelPort,work->SelSlot,
					  work->arg.chdir.name,work->arg.chdir.pwd)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -3:
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

static int MCAccessGetdir0Start(MCAccessWork *work,int port,int slot,
								char *name,sceMcTblGetDir *table,int table_size,int *ret_size)
{
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.getdir.name=name;
	work->arg.getdir.table=table;
	work->arg.getdir.table_size=table_size;
	work->arg.getdir.ret_table_size=ret_size;

	return 1;
}

int MCAccessGetdirStart(MCAccessWork *work,int port,int slot,
						char *name,sceMcTblGetDir *table,int table_size,int *ret_size)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_GETDIR);

	return MCAccessGetdir0Start(work,port,slot,name,table,table_size,ret_size);
}

int MCAccessGetdirNextStart(MCAccessWork *work,int port,int slot,
							char *name,sceMcTblGetDir *table,int table_size,int *ret_size)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_GETDIR_NEXT);

	return MCAccessGetdir0Start(work,port,slot,name,table,table_size,ret_size);
}

static int MCAccessGetdir0(MCAccessWork *work,int flag)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcGetDir(work->SelPort,work->SelSlot,work->arg.getdir.name,flag,
					   work->arg.getdir.table_size,work->arg.getdir.table)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
					return -1;
				default:
					if(result>=0){
						*(work->arg.getdir.ret_table_size)=result;
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessDeleteStart(MCAccessWork *work,int port,int slot,char *name)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_DELETE);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.delete.name=name;

	return 1;
}

static int MCAccessDelete(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcDelete(work->SelPort,work->SelSlot,work->arg.delete.name)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
				case -5:
				case -6:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessRenameStart(MCAccessWork *work,int port,int slot,char *org,char *next)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_RENAME);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.rename.org=org;
	work->arg.rename.next=next;

	return 1;
}

static int MCAccessRename(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(BP_sceMcRename(work->SelPort,work->SelSlot,
					   work->arg.rename.org,work->arg.rename.next)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(BP_sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessChmodStart(MCAccessWork *work,int port,int slot,
					   char *name,sceMcTblGetDir *table,unsigned int valid)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_CHMOD);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.chmod.name=name;
	work->arg.chmod.table=table;
	work->arg.chmod.valid=valid;

	return 1;
}

static int MCAccessChmod(MCAccessWork *work)
{
   BP_TED_BREAK;
#if 0 //BP_PS2
	switch(SUB_STEP){
	case 0:
        if(sceMcSetFileInfo(work->SelPort,work->SelSlot,
							work->arg.chmod.name,
							(unsigned char *)(work->arg.chmod.table),
							work->arg.chmod.valid)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
#endif
	return 0;
}

int MCAccessFormatStart(MCAccessWork *work,int port,int slot)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_FORMAT);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	return 1;
}

static int MCAccessFormat(MCAccessWork *work)
{
   BP_TED_BREAK;
#if 0 //BP_PS2
	switch(SUB_STEP){
	case 0:
		if(sceMcFormat(work->SelPort,work->SelSlot)==0){
			/* Success */
			SUB_STEP_UP();
		}
		else{
			/* Failed */
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				if(result==0){
					/* Success */
					return 1;
				}
				else{
					/* Retry */
					if(RetryCountup(work)) return -1;
					SUB_STEP_DOWN();
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
#endif
	return 0;
}

int MCAccessUnformatStart(MCAccessWork *work,int port,int slot)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_UNFORMAT);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	return 1;
}

static int MCAccessUnformat(MCAccessWork *work)
{
   BP_TED_BREAK;
#if 0 //BP_PS2
	switch(SUB_STEP){
	case 0:
		if(sceMcUnformat(work->SelPort,work->SelSlot)==0){
			/* Success */
			SUB_STEP_UP();
		}
		else{
			/* Failed */
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				if(result==0){
					/* Success */
					return 1;
				}
				else{
					/* Retry */
					if(RetryCountup(work)) return -1;
					SUB_STEP_DOWN();
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
#endif
	return 0;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


int MCAccessing(MCAccessWork *work)
{
	return MAIN_STEP!=MCAccess_WAIT && MAIN_STEP!=MCAccess_PRECHECK;
}

int MCAccessGetResult(MCAccessWork *work)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;
	return work->Result;
}

int MCAccessGetType(MCAccessWork *work,int port,int slot)
{
	return work->Type[port][slot];
}

int MCAccessIsMCChanged(MCAccessWork *work,int port,int slot)
{
	return work->Type[port][slot]!=work->OldType[port][slot];
}

int MCAccessGetFreeSize(MCAccessWork *work,int port,int slot)
{
	return work->FreeSize[port][slot]*1024;
}

int MCAccessIsFormatted(MCAccessWork *work,int port,int slot)
{
	return work->Formatted[port][slot];
}

int MCAccessGetSlotMax(MCAccessWork *work,int port)
{
	return work->SlotMax[port];
}

void MCAccessAutoCheck(MCAccessWork *work,int flag)
{
	if(flag){
		if(work->AutoPortCheck_Counter==0){
			work->AutoPortCheck_Counter=PORTCHECK_COUNT_MAX;
		}
	}
	else{
		work->AutoPortCheck_Counter=0;
	}
}

int MCAccessCheckSeqFlag(MCAccessWork *work)
{
	if(work->CheckSeqFlag==2){
		work->CheckSeqFlag=0;
		return 1;
	}
	return 0;
}

int MCAccessAutoChecking(MCAccessWork *work)
{
	if(work->CheckSeqFlag==1) return 1;
	return 0;
}

#if 0

static int Dummy(void)
{
	return 1;
}

#endif

void MCAccessInit(MCAccessWork *work,int flag)
{
	int i;

   DMC_PRINTF( "MCAccessInit\n");

	if(flag){
		while(BP_sceMcInit()!=0);
		// GV_SetLoader('i',Dummy);
		ResetLoaddataID();
	}

	if(work!=NULL){
		for(i=0;i<STEP_LEVEL_MAX;i++) work->step[i]=0;
		work->SelPort=work->SelSlot=0;
		work->AutoPortCheck_Counter=0;
		work->CheckSeqFlag=0;
		work->fd=-1;

		for(i=0;i<PORT_MAX;i++){
			int j;
			// for(j=0;j<SLOT_MAX;j++) work->Type[i][j]=work->OldType[i][j]=DIFFERENT_CARD_1;
			for(j=0;j<SLOT_MAX;j++) work->Type[i][j]=work->OldType[i][j]=0;
		}

		// MAIN_STEP_SET(MCAccess_CHECKSTART);
	}
}

void MCAccessEnd(MCAccessWork *work)
{
   DMC_PRINTF( "MCAccessEnd: fd: %d\n", work->fd );

   BP_sceMcSync(0,NULL,NULL);
	if(work->fd>=0){
      // BP - JM - The following is confusing - it is just trying to close and then detecting 
      // success when the close doesn't return an error.  I believe that this was because it was possible
      // for MCAccessEnd to happen after the close completed, but possibly before work->fd was cleared out
      
      while(BP_sceMcClose(work->fd) == 0);
		BP_sceMcSync(0,NULL,NULL);
	}
}

void MCAccessAct(MCAccessWork *work)
{
	// printf("%d\n",MAIN_STEP);
   DMC_PRINTF( "MCAccessAct: step: %d\n", MAIN_STEP );

	switch(MAIN_STEP){
	case MCAccess_WAIT:
		/* No Operation */
		break;

	case MCAccess_PRECHECK:
		switch(MCAccessPrecheck(work)){
		case 1:
			MAIN_STEP_SET(MCAccess_WAIT);
			PortSlotNumUpdate(work);
			break;
		case -1:
			MAIN_STEP_SET(MCAccess_CHECKSTART);
			break;
		}
		break;
	case MCAccess_CHECKSTART:
		MCAccessPrecheck2Check(work);
		work->CheckSeqFlag=1;
		break;
	case MCAccess_CHECK:
		if(MCAccessCheck(work)!=0){
			MAIN_STEP_SET(MCAccess_WAIT);
			work->SelPort=work->SelSlot=0;
			if(work->AutoPortCheck_Counter){
				work->AutoPortCheck_Counter=PORTCHECK_COUNT_MAX;
			}
			work->Result=1;
			if(work->CheckSeqFlag) work->CheckSeqFlag=2;
		}
		break;

	case MCAccess_OPEN:
		{
			int ans=MCAccessOpen(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_CLOSE:
		{
			int ans=MCAccessClose(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_SEEK:
		{
			int ans=MCAccessSeek(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_READ:
		{
			int ans=MCAccessRead(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_WRITE:
		{
			int ans=MCAccessWrite(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_MKDIR:
		{
			int ans=MCAccessMkdir(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_CHDIR:
		{
			int ans=MCAccessChdir(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_GETDIR:
		{
			int ans=MCAccessGetdir0(work,0);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_GETDIR_NEXT:
		{
			int ans=MCAccessGetdir0(work,1);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_RENAME:
		{
			int ans=MCAccessRename(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_DELETE:
		{
			int ans=MCAccessDelete(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_CHMOD:
		{
			int ans=MCAccessChmod(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_FORMAT:
		{
			int ans=MCAccessFormat(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_UNFORMAT:
		{
			int ans=MCAccessUnformat(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	}
}

void MCAccessPostAct(MCAccessWork *work)
{
	if(MAIN_STEP==MCAccess_WAIT){
		if(work->AutoPortCheck_Counter){
			work->AutoPortCheck_Counter--;
			if(work->AutoPortCheck_Counter==0) MCAccessPrecheckStart(work);
		}
	}
}
