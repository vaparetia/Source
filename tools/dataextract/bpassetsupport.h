#pragma once

#include <string>
#include <vector>

struct SBPAssetLine
{
   std::string mFlatPath;
   std::string mArchivePath;
   std::string mStagePath;

   bool operator < ( const SBPAssetLine & rhs ) const;
};

void SetPlatformSubfolder( const char * subfolder );
const char * GetPlatformSubfolder();
void InsertPlatformSubfolder( char * fullPath );
bool UsingTextureFlatlistRemapping();
void InitTriInfo( const char * const texFlatlistFilename );
void AddTri( const char * const filename );
//std::string GetTextureFlatFilename( const char * const srcFilename );
void GetTriCtxrFilenames( const char * const triFilename, std::vector< SBPAssetLine > & outAssets );
void GetImgCtxrFilenames( const char * const imgFilename, SBPAssetLine & outAsset );
std::string GetCmdlFilename( const char * const mdlFilename );
void GetZmsCmdlFilenames( const char * const zmsFilename, std::vector< SBPAssetLine > & outAssets );

void DumpTriStats( const char * const texDupeStatsFilename );

void InitAssetRemap( const char * const assetRemapFilename );
std::string const GetAssetRemappedName( char const * const srcFilename );
std::string const GetAssetArchiveFilename( char const * const srcFilename, char const * const remapped );
bool UsingAssetRemapping();