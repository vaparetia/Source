//----------------------------------------------------------------------------
// CBonesFile.h
// Copyright 2004
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

typedef uint32 TBoneId;

//----------------------------------------------------------------------------

class CBonesFile
{
public:
   explicit CBonesFile( std::string const & path );
   explicit CBonesFile();

   TBoneId const GetBoneId( std::string const & boneName ) const;

   std::vector< std::string > const & GetStrings() const { return mStrings; }

   TBoneId const AddBone( std::string const & name );

   std::string const GetPathForBoneId( TBoneId const & bone ) const;
   
private:
   static void LoadStrings( std::string const & path, 
                            std::vector<std::string> & strings );

private:
   std::vector<std::string> mStrings;
};

//----------------------------------------------------------------------------
