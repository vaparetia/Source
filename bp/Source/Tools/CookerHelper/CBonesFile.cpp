//----------------------------------------------------------------------------
// CBonesFile.cpp
// Copyright 2004
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CBonesFile.h"

//----------------------------------------------------------------------------

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/TTokenSet.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

CBonesFile::CBonesFile( std::string const & path )
{
   LoadStrings( path, mStrings );
}

//----------------------------------------------------------------------------

CBonesFile::CBonesFile()
{  
}

//----------------------------------------------------------------------------

TBoneId const CBonesFile::GetBoneId( std::string const & boneName ) const
{
   bool const boneNameIsPath = !boneName.empty() && boneName[0] == '|';

   uint32 index = 0;
   
   for( ; index < mStrings.size(); ++index )
   {
      string const currentBonePath = boneNameIsPath ? mStrings[index]
                                                    : CStringExtras::SplitRightLastOf( mStrings[index], '|' );

      // did we find it ?
      if( currentBonePath == boneName )
         return TBoneId(index);
   }
   return (uint32)-1;
}

//----------------------------------------------------------------------------

void CBonesFile::LoadStrings( std::string const & path, std::vector<std::string> & strings )
{
   FILE * handle = NULL;
   fopen_s( &handle, path.c_str(), "rt" );
   if( handle )
   {
      static char buffer[1024];
      while( fgets( buffer, 1024, handle ) )
      {
         int const length = strlen( buffer ) - 1;
         if( length >= 0 && buffer[ length ] == '\n' )
            buffer[ length ] = 0;

         strings.push_back( CStringExtras::TrimWhitespace( string( buffer ) ) );
      }

      fclose( handle );
   }
}

//----------------------------------------------------------------------------

TBoneId const CBonesFile::AddBone( std::string const & name )
{ 
   mStrings.push_back(name); 
   return TBoneId(mStrings.size() - 1); 
}

//----------------------------------------------------------------------------

std::string const CBonesFile::GetPathForBoneId( TBoneId const & bone ) const
{
   for( int i = 0; i < GetStrings().size(); ++i )
   {
      std::string const & name = GetStrings()[i];
      
      if( GetBoneId(name) == bone )
         return name;
   }

   return "";
}
