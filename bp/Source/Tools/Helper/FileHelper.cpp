//----------------------------------------------------------------------------
// FileHelper.cpp
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "FileHelper.h"

//----------------------------------------------------------------------------

#include "Helper.h"
#include "windows.h"

#include "shlwapi.h"

//----------------------------------------------------------------------------

using namespace Helper;
using namespace System;
using namespace System::Collections::Generic;

//----------------------------------------------------------------------------

void FindMatches(std::string const & currentFolder, std::vector<std::string> const & pathMatchSpecs, std::list<std::string> & matches, bool recursive, std::string const & resultCurrentFolder)
{
   std::string const currentPathPlusWildcard = currentFolder + "/*";

   WIN32_FIND_DATAA fileData;
   HANDLE hFind = FindFirstFileA(currentPathPlusWildcard.c_str(), &fileData);
   if( hFind != INVALID_HANDLE_VALUE )
   {
      do 
      {
         if( strcmp(fileData.cFileName, ".") != 0 &&
             strcmp(fileData.cFileName, "..") != 0 )
         {
            std::string fullPath = currentFolder + "/" + fileData.cFileName;
            std::string fullResultPath = resultCurrentFolder + "/" + fileData.cFileName;

            if( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
               if( recursive )
               {
                  FindMatches(fullPath, pathMatchSpecs, matches, recursive, fullResultPath);
               }
            }
            else
            {
               for(int i = 0; i < pathMatchSpecs.size(); ++i )
               {
                  if( PathMatchSpecA(fullPath.c_str(), pathMatchSpecs[i].c_str()) )
                  {
                     matches.push_back(fullResultPath);
                     break;
                  }
               }
            }
         }
      }
      while (FindNextFileA(hFind, &fileData));

      FindClose(hFind);
   }
}

//----------------------------------------------------------------------------

List<String^>^ FileHelper::MatchFiles(String^ rootFolder, List<String^>^ matchStrings, bool recursive, String^ resultRootFolder)
{
   std::string const rootPathNative = StringHelper::ConvertString(rootFolder);
   std::string const resultRootPathNative = StringHelper::ConvertString(resultRootFolder);

   std::vector<std::string> pathMatchSpecs;

   for( int i = 0; i < matchStrings->Count; ++i )
   {
      pathMatchSpecs.push_back(StringHelper::ConvertString(matchStrings[i]));
      pathMatchSpecs.push_back("*/" + StringHelper::ConvertString(matchStrings[i]));
   }

   std::list<std::string> matches;

   FindMatches(rootPathNative, pathMatchSpecs, matches, recursive, resultRootPathNative);

   List<String^>^ result = gcnew List<String^>(matches.size());

   for(std::list<std::string>::const_iterator it = matches.begin(); it != matches.end(); ++it )
   {
      result->Add(gcnew String(it->c_str()));
   }

   return result;
}
