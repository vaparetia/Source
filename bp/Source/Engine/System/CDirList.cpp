//------------------------------------------------------------------------------------------
// CDirList.cpp
// Bluepoint
// Copyright 2003
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CDirList.h"

#include "Engine/System/CFileUtils.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------
#if BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>
#endif

//------------------------------------------------------------------------------------------

void CDirList::GetFiles( std::string const & path,
                         std::string const & extension,
                         TRecurseDelegate const &recurseDirectory,
                         std::vector< std::string > & output)

{
#if BPE_TARGET == BPE_TARGET_WIN32
   string const realExtension = extension.size() ? extension.c_str() : "*";

   // find all files matching the extension
   {
      WIN32_FIND_DATA wfd;
      HANDLE hFind = FindFirstFile( CFileUtils::UnifyPath( path + "\\" + realExtension ).c_str(), &wfd );

      if( hFind != INVALID_HANDLE_VALUE )
      {
         do
         {
            if( strcmp( wfd.cFileName, "." ) != 0 &&
                strcmp( wfd.cFileName, ".." ) != 0 )
               output.push_back( CFileUtils::UnifyPath( path + "\\" + wfd.cFileName ) );
         } 
         while( FindNextFile( hFind, &wfd ) );

         // Close the find handle.
         FindClose( hFind );
      }
   }

   // find all directories and recursive if asked to do so
   {
      WIN32_FIND_DATA wfd;
      HANDLE hFind = FindFirstFile( CFileUtils::UnifyPath( path + "\\*" ).c_str(), &wfd );

      if( hFind != INVALID_HANDLE_VALUE )
      {
         do
         {
            if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
               string const subFilename( wfd.cFileName );
               if( subFilename != "." &&
                   subFilename != ".." )
               {
                  std::string const directory(path + "\\" + wfd.cFileName);
                  if (recurseDirectory(directory))
                  {
                     GetFiles( path + "\\" + wfd.cFileName,
                              realExtension,
                              recurseDirectory,
                              output );
                  }
               }
            }
         } 
         while( FindNextFile( hFind, &wfd ) );

         // Close the find handle.
         FindClose( hFind );
      }
   }
#endif
}

//------------------------------------------------------------------------------------------

bool dirlist_recurse(std::string const &path)
{
   (path);
   return true;
}

//------------------------------------------------------------------------------------------

bool dirlist_no_recurse(std::string const &path)
{
   (path);
   return false;
}

//------------------------------------------------------------------------------------------

void CDirList::GetFiles( std::string const & path,
                         std::string const & extension,
                         bool const recursive,
                         std::vector< std::string > & output )
{

   TRecurseDelegate recurseDirectory;
   if (recursive)
   {
      recurseDirectory.bind(dirlist_recurse);
   }
   else
   {
      recurseDirectory.bind(dirlist_no_recurse);
   }

   GetFiles(path, extension, recurseDirectory, output);
}

//------------------------------------------------------------------------------------------
