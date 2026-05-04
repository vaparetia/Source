//----------------------------------------------------------------------------
// CFileUtils.cpp
// Bluepoint
// Copyright 2003
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CFileUtils.h"
#include <stdlib.h>

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>
#elif BPE_TARGET == BPE_TARGET_PS3
#include <cell/cell_fs.h>
#elif BPE_TARGET==BPE_TARGET_RVL
#include "Engine/System/RVL/RVLFileIO.h"
#endif

//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

std::string const CFileUtils::UnifyPath( std::string const & inpath )
{
   string result = inpath;
   
   // ensure slashes are all back slashes
   
   string::iterator lastSlash = result.end();

   for( string::iterator it = result.begin(); it != result.end();  )
   {
      if( *it == '/' || *it == '\\' )
      {
         if( lastSlash == (it - 1) )
         {
            it = result.erase( it );
            continue;
         }

         lastSlash = it;
         *it = BPE_PATH_SEPARATOR[0];
      }
      // Make filename lowercase (PS3 filenames are case sensitive!)
      *it = tolower(*it);
      
      ++it;
   }

   return result;
}

//----------------------------------------------------------------------------
   
std::string const CFileUtils::GetFullPathMinusExtension( std::string const & inPath )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char name[_MAX_FNAME];
   char ext[_MAX_EXT];
   
   _splitpath( inPath.c_str(), drive, dir, name, ext );

   return string( drive ) + dir + name;
#else
   int const foundPos = inPath.rfind('.');
   if( foundPos != std::string::npos )
   {
      return inPath.substr(0, foundPos);
   }
   else
   {
      return inPath;
   }
#endif
}

//----------------------------------------------------------------------------

std::string const CFileUtils::GetFileExtension( std::string const & inPath )
{        
   string extension( inPath.substr(inPath.rfind('.') ) );
   return extension;
}

//----------------------------------------------------------------------------

std::string const CFileUtils::GetFileName( std::string const & inPath )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char name[_MAX_FNAME];
   char ext[_MAX_EXT];
   
   _splitpath_s( inPath.c_str(), drive, dir, name, ext );

   return string( name );
#else
   BPE_VERIFYA(false, "Not supported");
   return string("");
#endif
}

//----------------------------------------------------------------------------

std::string const CFileUtils::GetFilePath( std::string const & inPath )
{
   std::string outPath(inPath);
   int const lastSlash = outPath.find_last_of("/\\");
   if ((lastSlash != -1) && (lastSlash < outPath.size()))
   {
      // Keep last slash
      outPath.erase(lastSlash + 1);
   }
   return outPath;
}

//----------------------------------------------------------------------------

std::string const CFileUtils::CombinePaths( std::string const & path1, std::string const & path2)
{
   std::string output = path1;
   if( !output.empty() )
   {
      char lastChar = output[output.size() - 1];
      if( lastChar != '/' && lastChar  != '\\' )
         output += "/";
   }

   output += path2;

   return output;
}

//----------------------------------------------------------------------------
   
bool const CFileUtils::MakePath( std::string const & fullPath )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   if( fullPath.empty() )
      return true;

   string const unifiedPath = UnifyPath( fullPath );
   vector< string > directories;

   CStringExtras::Tokenize( unifiedPath, directories, "/" );

   if( !directories.empty() )
   {
      bool hasDrive = false;
      
      if( directories[0].find( ":" ) != string::npos )
         hasDrive = true;

      int      i = 0;
      string   currentPath;

      if( hasDrive )
      {
         i = 1;
         currentPath = directories[0];
      }

      for( ; i < directories.size(); ++i )
      {
         if( !currentPath.empty() )
         {
            currentPath += BPE_PATH_SEPARATOR;
         }
         currentPath += directories[i];

         DWORD const attr = ::GetFileAttributes( currentPath.c_str() );

         if( attr == INVALID_FILE_ATTRIBUTES )
         {
            if( !::CreateDirectory( currentPath.c_str(), NULL ) )
               return false;
         }
         else if( ( attr & FILE_ATTRIBUTE_DIRECTORY) == 0 )
         {
            // found file
            return false;
         }
      }
      
      return true;
   }
#else
   BPE_VERIFY(false, false, "Not supported.");
#endif

   return false;
}

//----------------------------------------------------------------------------

bool const CFileUtils::DoesExist( std::string const & fileName )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   HANDLE handle = CreateFile( fileName.c_str(), GENERIC_READ, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, NULL );
   if( handle != INVALID_HANDLE_VALUE )
   {
      CloseHandle(handle);
      return true;
   }
#elif BPE_TARGET == BPE_TARGET_RVL
   return RVLFileIO::DoesExist( fileName.c_str() );
#else
   FILE *fp = fopen(fileName.c_str(), "rb");
   bool bExists = (fp != NULL);
   bool const bOpened = (fp != NULL);
   if (fp) fclose(fp);
   if (bOpened)
   {
      return true;
   }
#endif

   return false;
}

//----------------------------------------------------------------------------

int64 CFileUtils::GetFileSize(char const * const pFilename)
{
   int64 fileSize = -1;
#if BPE_TARGET == BPE_TARGET_WIN32

   {
      HANDLE handle = CreateFile( pFilename, GENERIC_READ, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, NULL );
      if( handle == INVALID_HANDLE_VALUE )
      {
         ::CloseHandle(handle);
      }
      else
      {
         fileSize = ::GetFileSize(handle, NULL);
         ::CloseHandle(handle);
      }
   }
   return fileSize;

#elif BPE_TARGET==BPE_TARGET_RVL
   RVLFileIO::TFileInfo info;

   if ( RVLFileIO::OpenForRead( &info, pFilename, NULL ) )
   {
      fileSize = RVLFileIO::GetFileSize( &info );
      RVLFileIO::Close( &info );
   }

   return fileSize;

#elif BPE_TARGET == BPE_TARGET_PS3
   {
      CellFsStat sb;
      int ret = cellFsStat(pFilename, &sb);
      if (ret == CELL_FS_SUCCEEDED)
      {
         fileSize = (int64) sb.st_size;
      }
      else if (ret == CELL_FS_ENOENT)
      {
         // File doesn't exist
      }
      else
      {
         // Internal error
         bpe_debugger_printf("CFileUtils::GetFileSize: %s - PS3 Error: %d\n", pFilename, ret);
      }
   }

   return fileSize;

#else

   // FILE version
   {
//      BPE_CTASSERT(0);  // Not tested
      FILE *fp = fopen(pFilename, "rb");
      if (fp != NULL)
      {
         fseek((FILE*) fp, 0, SEEK_END);
         fileSize = ftell((FILE*) fp);
         fclose(fp);
      }
   }

   return fileSize;

#endif
}

//----------------------------------------------------------------------------

int64 CFileUtils::LoadFileAsString(char const * const pFilename, std::string &outString)
{
   int64 fileSize = -1;
   char * pStringData = NULL;
   outString.clear();

#if BPE_TARGET == BPE_TARGET_WIN32

   {
      HANDLE handle = CreateFile( pFilename, GENERIC_READ, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, NULL );
      if( handle == INVALID_HANDLE_VALUE )
      {
         CloseHandle(handle);
      }
      else
      {
         fileSize = ::GetFileSize(handle, NULL);
         pStringData = (char *) malloc((size_t) (fileSize + 1));
         DWORD numberOfBytesRead = 0;
         ::ReadFile( handle, pStringData, (DWORD) fileSize, &numberOfBytesRead, NULL );
         pStringData[fileSize] = 0;
         ::CloseHandle(handle);
      }
   }

#elif BPE_TARGET==BPE_TARGET_RVL
   RVLFileIO::TFileInfo fileInfo;

   if ( RVLFileIO::OpenForRead( &fileInfo, pFilename, NULL ) )
   {
      size_t readBytes = 0;
      fileSize = RVLFileIO::GetFileSize( &fileInfo );
      pStringData = (char *) malloc((size_t) (fileSize + 1));

      RVLFileIO::ReadSync( &fileInfo, pStringData, fileSize, &readBytes );
      pStringData[ readBytes ] = 0;
      RVLFileIO::Close( &fileInfo );

      fileSize = readBytes;
   }

#elif BPE_TARGET == BPE_TARGET_PS3

   {
      int handle = 0;
      CellFsErrno ret = cellFsOpen(pFilename, CELL_FS_O_RDONLY, &handle, NULL, 0);
      if (ret == CELL_FS_SUCCEEDED)
      {
         CellFsStat sb;
         ret = cellFsFstat(handle, &sb);
         fileSize = (int64) sb.st_size;
         pStringData = (char *) malloc(fileSize + 1);
         uint64 numberOfBytesRead = 0;

         ret = cellFsRead(handle, pStringData, fileSize, &numberOfBytesRead);
         pStringData[fileSize] = 0;
         cellFsClose(handle);

      }
      else if (ret == CELL_FS_ENOENT)
      {
         // File doesn't exist
      }
      else
      {
         // Internal error
         bpe_debugger_printf("CFileUtils::LoadFileAsString: %s - PS3 Error: %d\n", pFilename, ret);
      }
   }

#else

   // FILE version
   {

//      BPE_CTASSERT(0);  // Not tested
      FILE *fp = fopen(pFilename, "rb");
      if (fp != NULL)
      {
         fseek((FILE*) fp, 0, SEEK_END);
         fileSize = ftell((FILE*) fp);
         fseek((FILE*) fp, 0, SEEK_SET);
         pStringData = reinterpret_cast<char *>( malloc(fileSize + 1) );
         int numberOfBytesRead = 0;
         numberOfBytesRead = fread(pStringData, fileSize, 1, fp);
         pStringData[fileSize] = 0;
         fclose(fp);
      }
   }

#endif

   if (pStringData != NULL)
   {
      outString = std::string(pStringData);
      free(pStringData);
   }

   return fileSize;
}

//----------------------------------------------------------------------------

int64 CFileUtils::WriteTextFile(char const * const pFilename, char const * const pText)
{
   int const stringLength = strlen(pText);
   FILE *fp = fopen(pFilename, "wb");
   if (fp != NULL)
   {
      int const numberOfBytesWritten = fwrite(pText, stringLength, 1, fp);
      fclose(fp);
      return numberOfBytesWritten;
   }

   return -1;
}

//----------------------------------------------------------------------------

bool const CFileUtils::CanCreate( std::string const & fileName )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   HANDLE handle = CreateFile( fileName.c_str(), GENERIC_WRITE, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   if( handle != INVALID_HANDLE_VALUE )
   {
      CloseHandle(handle);
      return true;
   }
#elif BPE_TARGET == BPE_TARGET_RVL
   return false;
#else
   FILE *fp = fopen(fileName.c_str(), "wb");
   bool const bOpened = (fp != NULL);
   if (fp) fclose(fp);
   if (bOpened)
   {
      return true;
   }
#endif

   return false;
}

//----------------------------------------------------------------------------

CFileUtils::EType const CFileUtils::GetFileType( std::string const & fileName )
{
#if BPE_TARGET == BPE_TARGET_WIN32
   if( !DoesExist( fileName ) )
   {
      return kType_Invalid;
   }
   
   const uint32 fileAttributes = GetFileAttributes( fileName.c_str() );
   
   if( fileAttributes & FILE_ATTRIBUTE_DIRECTORY )
   {
      return kType_Dir;
   }
   else
   {
      return kType_File;
   }
#else

   BPE_VERIFY(false, false, "Not supported");

#endif
   return kType_Invalid;
}

//----------------------------------------------------------------------------

