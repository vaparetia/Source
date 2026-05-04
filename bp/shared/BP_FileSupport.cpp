//----------------------------------------------------------------------------
// BP_FileSupport.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/System/COsContext.h"

#include "BP_FileSupport.h"
#include "BP_Memory.h"

#include "libfs.h"

#define BP_SPLIT_STREAM_EXTENSION ".sdt"

// global variable that represents the current disk.
extern "C" unsigned int _BP_curDisk = 0;

#if MGS_VERSION==2
// MGS2 only has one disk
static int const skNumTotalDisks = 1;
#  define BP_FS_FILE_INFO_NAME( f ) (f)->name
#elif MGS_VERSION==3
// MGS2 has two disks
static int const skNumTotalDisks = 2;
#  define BP_FS_FILE_INFO_NAME( f ) (f)->name[_BP_curDisk]
#endif
//----------------------------------------------------------------------------

namespace
{
   unsigned int gCurrentAssetCacheTag = 0xFFFFFFFF;
   char gCurrentFolder[256] = { 0 };

   struct SOffsetNamePair
   {
   public:
      bool operator < ( const SOffsetNamePair & rhs ) const
      {
         return mOffset < rhs.mOffset;
      }

      unsigned int   mOffset;
      const char *   mName;
   };

   typedef std::vector< SOffsetNamePair >          TOffsetNamePairVector;

   std::vector< TOffsetNamePairVector >            sStreamRemappingTables[ skNumTotalDisks ];

   const int SECTOR_SIZE = 2048;
}

//----------------------------------------------------------------------------

extern "C"
{
   char const *gkBP_BluepointAssets_Vita_Archive_Filename_Base = 
#if MGS_VERSION==2
      "mgs2_misc";
#elif MGS_VERSION==3
      "mgs3_misc";
#else
#  error Unknown MGS version
#endif
   char * gBP_X360PlatformRoot = "D:\\";
}

//----------------------------------------------------------------------------

extern "C" void BP_BeginLoadData(const unsigned int assetCacheTag, const char* folderPath)
{
   gCurrentAssetCacheTag = assetCacheTag;
   strcpy(gCurrentFolder, folderPath);
}

//----------------------------------------------------------------------------

extern "C" void BP_EndLoadData()
{
   gCurrentAssetCacheTag = 0xFFFFFFFF;
   gCurrentFolder[0] = 0;
}

//----------------------------------------------------------------------------

extern "C" unsigned int BP_GetCurrentAssetCacheTag()
{
   return gCurrentAssetCacheTag;
}

//----------------------------------------------------------------------------

extern "C" const char * BP_GetCurrentLoadPath()
{
   return gCurrentFolder;
}

//----------------------------------------------------------------------------

extern "C" char *BP_FGetsWinFiles( char *buf, int max_count, FILE *fp )
{
   // Fixes Win32 file streams to remove \r for PS3

   char *ret = fgets( buf, max_count, fp );

#ifdef BP_PS3
   if ( ret )
   {
      int len = strlen( buf );
      if ( len >= 2 )
      {
         if ( buf[len-1] == '\n' && buf[len-2] == '\r' )
         {
            // Ends with CRLF, replace with plain LF
            buf[ len-2 ] = '\n';
            buf[ len-1 ] = 0;
         }
         else if ( buf[ len-1 ] == '\r' )
         {
            // Ends with CR, meaning we ran out of space for LF
            // So just end with NULL
            buf[len-1] = 0;
         }
      }
   }
#endif

   return ret;
}

extern "C" void BP_UnifyPath( char * path )
{
   int totalLen = 0;
   while( path[totalLen] != NULL )
   {
      if( path[totalLen] == '/' )
      {
         path[totalLen] = BP_PATH_SEPARATOR[0];
      }
      ++totalLen;
   }
}

void BP_InsertPlatformSubfolder( char * path )
{
   char justFilename[FILENAME_MAX];
   char * lastSlash = path + strlen(path);
   while( *lastSlash != '/' )
      --lastSlash;
   strcpy( justFilename, lastSlash+1 );
   strcpy( lastSlash+1, BP_PLATFORM_FOLDER "/" );
   strcat( path, justFilename );
}

void BP_Init_FS_Region()
{
   switch( OsContext()->mBuildSKU )
   {
   case COsContext::kBS_Japan:
      FS_CurrentRegion = FS_REGION_JP;
      break;

   // Now we're using EU data for NA, switch USA to use EU Regions.
   case COsContext::kBS_USA:
   case COsContext::kBS_Europe:
#if MGS_VERSION==2
      FS_CurrentRegion = FS_REGION_EU;
#elif MGS_VERSION==3
      switch( OsContext()->mLanguage )
      {
      case COsContext::kL_English:
         FS_CurrentRegion = FS_REGION_FR;
         break;
      case COsContext::kL_French:
         FS_CurrentRegion = FS_REGION_FR;
         break;
      case COsContext::kL_German:
         FS_CurrentRegion = FS_REGION_GR;
         break;
      case COsContext::kL_Italian:
         FS_CurrentRegion = FS_REGION_IT;
         break;
      case COsContext::kL_Spanish:
         FS_CurrentRegion = FS_REGION_SP;
         break;
      }
#endif
      break;
   default:
      BP_TODO_BREAK;
   }
}

void BP_LoadStreamRemappingTables()
{
   for( int i = 0; i < skNumTotalDisks; ++i )
   {
      _BP_curDisk = i;

      for( const FS_FILE_INFO * pFileInfo = fs_file_info; BP_FS_FILE_INFO_NAME(pFileInfo) != 0; ++pFileInfo )
      {
         sStreamRemappingTables[ i ].push_back( TOffsetNamePairVector() );
         TOffsetNamePairVector & table = sStreamRemappingTables[ i ].back();

         char fullPath[FILENAME_MAX];
         sprintf(fullPath, "%s/%s", FS_BP_GetCurrentRegionFolder(), BP_FS_FILE_INFO_NAME(pFileInfo));
         strcpy( strstr( fullPath, ".dat" ), "/" BP_REBUILTDATA_FOLDER "/" );
         char * const baseFolder = fullPath + strlen( fullPath );
         strcpy( baseFolder, BP_PLATFORM_FOLDER "/bp_streams.txt" );
         if( !BP_FileExists( fullPath, NULL ) )
         {
            //Fall back to base stream rebuild folder.  Windows build uses these.
            strcpy( baseFolder, "bp_streams.txt" );
         }

         if( BP_FileExists( fullPath, NULL ) )
         {
            const int lstTxtSize = BP_GetFileSizeAttr( fullPath, NULL );
            //N.B. this memory does not ever get freed; the table points into the stream name
            //part of the text.
            char * lstTxtData = (char *) BP_Memory_Alloc( lstTxtSize + 1, 4, kMT_Permanent, kMC_Stream );
            lstTxtData[lstTxtSize] = 0;
            BP_ReadFileByFilename( fullPath, lstTxtData, 0, lstTxtSize );
            
            //Wipe out all CR & LF.
            for( int i=0; i < lstTxtSize; ++i )
            {
               if( lstTxtData[i] == '\x0a' || lstTxtData[i] == '\x0d' )
               {
                  lstTxtData[i] = 0;
               }
            }

            for( char * currLine = lstTxtData; currLine < lstTxtData+lstTxtSize; )
            {
               //skip over LF
               if( !(*currLine) )
                  ++currLine;

               if( currLine >= lstTxtData+lstTxtSize )
                  break;

               SOffsetNamePair pr = { 0 };
               const char seps[] = " \n";

               char * token = strtok( currLine, seps );
               if( *token == NULL )
               {
                  BP_BREAK;
               }
               if( 1 != sscanf( token, "%d", &pr.mOffset ) )
               {
                  BP_BREAK;
               }

               token = strtok( NULL, seps );
               if( *token == NULL )
               {
                  BP_BREAK;
               }
               pr.mName = token;
               currLine = token + strlen(token) + 1;

               table.push_back( pr );
            }
         }
         //else leave the table empty, it's presumably a stage.dat or slot/face.dat file.
      }

   }
   _BP_curDisk = 0;
}

int BP_GetRemappedStreamNameAndLoc( const int fileIndex, const unsigned int orgLsn, const int orgStreamTop, char * outFullPath )
{
   const int orgLoc = orgLsn * SECTOR_SIZE;
   //First, find the entry for this dat / lsn and convert the lsn.
   const TOffsetNamePairVector & table = sStreamRemappingTables[_BP_curDisk][fileIndex];

   BPE_ASSERT( table.size() != 0, "This is not a stream or codec .dat file!" );
   //Look up stream in our table by exact stream top offset in original PS2 .DAT file.
   const SOffsetNamePair key = { orgStreamTop, NULL };
   TOffsetNamePairVector::const_iterator found = std::lower_bound( table.begin(), table.end(), key );
   BPE_ASSERT( found != table.end() && found->mOffset == orgStreamTop, "stream top not found!" );

   const SOffsetNamePair & offsetName = *found;
   const int remappedLoc = (int)(orgLoc - offsetName.mOffset);

   strcpy(outFullPath, offsetName.mName);
   BP_UnifyPath(outFullPath);

   printf("STREAM NAME: -----------   %s  -------------\n", outFullPath);

   return remappedLoc;
}

const char * BP_FindStreamName( const int pos )
{
   int fileIndex = pos >> 24;

   if( fileIndex == FS_FILE_CODEC )
      BP_BREAK;

   int lsn = pos & 0xffffff;
   int loc = lsn * SECTOR_SIZE;

   const TOffsetNamePairVector & table = sStreamRemappingTables[_BP_curDisk][fileIndex];
   BPE_ASSERT( table.size() != 0, "This is not a stream or codec .dat file!" );

   const SOffsetNamePair key = { loc + 1, NULL };
   TOffsetNamePairVector::const_iterator found = std::lower_bound( table.begin(), table.end(), key );
   --found;

   if( found->mOffset != loc )
   {
      return NULL;
   }

   return found->mName;
}

//----------------------------------------------------------------------------

namespace
{
   const char * const kPlatformFolderWithSlashes = "/" BP_PLATFORM_FOLDER "/";
   int const kPlatformFolderWithSlashesLength = strlen(kPlatformFolderWithSlashes);
   
   const char * kOverrideBuildSKUPaths[] =
   {
      "ovr_us/",
      "ovr_jp/",
      "ovr_eu/"
   };
   
   BPE_CTASSERT(BPE_ARRAY_SIZE(kOverrideBuildSKUPaths) == COsContext::kBS_Count);

#ifdef BP_PS3
   const char * const kOverridePlatformPath = "ovr_ps3/";

#elif defined( BP_360 )
   const char * const kOverridePlatformPath = "ovr_360/";

#elif defined( BP_WIN32 )
   //NOTE: Can change this to any platform you desire (for testing purposes)
   //const char * const kOverridePlatformPath = "ovr_ps3/";
   const char * const kOverridePlatformPath = "ovr_360/";

#elif defined( BP_VITA )
   //NOTE: Can change this to any platform you desire (for testing purposes)
   const char * const kOverridePlatformPath = "ovr_vta/";

#else
# error TODO
#endif

}

static char* BP_GetFilenameIncludingOptionalPlatformSubfolder(char * path)
{
   char* fullPathLastSlash = strrchr(path, '/');
   
   if( fullPathLastSlash != NULL )
   {
      char* pathWithPlatformFolder = fullPathLastSlash + 1 /*after slash*/ - kPlatformFolderWithSlashesLength;
      
      // Ensure folder before filename matches exactly, i.e.: "/_ps3/"
      if( strncmp(pathWithPlatformFolder, kPlatformFolderWithSlashes, kPlatformFolderWithSlashesLength) == 0 )
      {
         // Return filename AFTER the slash (i.e. "_ps3/test.ctxr")
         return pathWithPlatformFolder + 1;
      }

      // No apparent platform folder exists, return just the filename (i.e. "test.ctxr")
      return fullPathLastSlash + 1;
   }
   
   return path;
}

static void BP_GetPathPlatformSKUOverride_Internal(char * const pInputPath, char const * const pPlatformPath, char const * const pLanguagePath, char * pOutputPath)
{
   char* filenameWithPlatformFolder = BP_GetFilenameIncludingOptionalPlatformSubfolder(pInputPath);
   
   int const pathLength = (filenameWithPlatformFolder - pInputPath);

   strncpy(pOutputPath, pInputPath, pathLength);
   
   pOutputPath[pathLength] = 0;

   if( pPlatformPath != NULL )
      strcat(pOutputPath, pPlatformPath);

   if( pLanguagePath != NULL )
      strcat(pOutputPath, pLanguagePath);

   strcat(pOutputPath, filenameWithPlatformFolder);
}

//IMPORTANT: An identical implementation of this functionality exists in Tools/DataTool/Program.cs
void BP_GetPathPlatformSKUOverride(char * loadPath, char * commonLoadPath)
{
   char overrideLoadPath[FILENAME_MAX];

   const char* pSKUPath = kOverrideBuildSKUPaths[OsContext()->mBuildSKU];

   //i.e.: ovr_ps3/ovr_jp/temp.txt
   BP_GetPathPlatformSKUOverride_Internal(loadPath, kOverridePlatformPath, pSKUPath, overrideLoadPath);
   if( BP_FileExists(overrideLoadPath, commonLoadPath) )
   {
      strcpy(loadPath, overrideLoadPath);
      return;
   }

   //i.e.: ovr_ps3/temp.txt
   BP_GetPathPlatformSKUOverride_Internal(loadPath, kOverridePlatformPath, NULL, overrideLoadPath);
   if( BP_FileExists(overrideLoadPath, commonLoadPath) )
   {
      strcpy(loadPath, overrideLoadPath);
      return;
   }
   
   //i.e.: ovr_jp/temp.txt
   BP_GetPathPlatformSKUOverride_Internal(loadPath, NULL, pSKUPath, overrideLoadPath);
   if( BP_FileExists(overrideLoadPath, commonLoadPath) )
   {
      strcpy(loadPath, overrideLoadPath);
      return;
   }
   
   //i.e.: temp.txt
   // We never modified "loadPath", we're done here.

}

// USED FOR TESTING PURPOSES ONLY!
int BP_GetNumFileEntriesFromRemapTable(int const fileIndex)
{
   const TOffsetNamePairVector& table = sStreamRemappingTables[_BP_curDisk][fileIndex];
   return table.size();
}

void BP_GetRemapTableEntries(int const fileIndex, unsigned int* pOffsets, char const** ppStreamNames)
{
   const TOffsetNamePairVector& table = sStreamRemappingTables[_BP_curDisk][fileIndex];
   
   for (TOffsetNamePairVector::const_iterator itor = table.begin(); itor != table.end(); ++itor)
   {
      *pOffsets++ = itor->mOffset;
      *ppStreamNames++ = itor->mName;
   }
}
