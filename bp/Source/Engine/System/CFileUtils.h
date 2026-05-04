//------------------------------------------------------------------------------------------
// CFileUtils.h
// Bluepoint
// Copyright 2003
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class ENGINE_API CFileUtils
{
public:
   enum EType
   {
      kType_Dir,
      kType_File,
      
      kType_Count,
      
      kType_Invalid = -1
   };

public:
   static std::string const UnifyPath( std::string const & inpath );
   
   static std::string const GetFullPathMinusExtension( std::string const & inPath );
   static std::string const GetFileExtension( std::string const & inPath );
   static std::string const GetFileName( std::string const & inPath );
   static std::string const GetFilePath( std::string const & inPath );
   static std::string const CombinePaths( std::string const & path1, std::string const & path2);
   
   static bool const    MakePath( std::string const & fullPath );

   static const bool    DoesExist( std::string const & fileName );
   static int64         GetFileSize(char const * const pFilename);   // returns -1 if file doesn't exist
   static const bool    CanCreate( std::string const & fileName );
   static const EType   GetFileType( std::string const & fileName );

   static int64         LoadFileAsString(char const * const pFilename, std::string &outString);    // returns -1 and empty string if file doesn't exist
   static int64         WriteTextFile(char const * const pFilename, char const * const pText);     // returns -1 if couldn't write file, otherwise returns bytes written
};

