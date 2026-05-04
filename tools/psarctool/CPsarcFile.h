//----------------------------------------------------------------------------
// CPsarcFile.h
// encapsulates operations on a psarc file.
//----------------------------------------------------------------------------

#pragma once

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"

//#include <windows.h>

#include <xmmintrin.h>

//----------------------------------------------------------------------------

struct SPsarcHeader
{
   unsigned long  mMagicNumber;
   unsigned long  mVersionNumber;
   unsigned long  mCompressionMethod;
   unsigned long  mTOCSize;
   unsigned long  mTOCEntrySize;
   unsigned long  mNumFiles;
   unsigned long  mBlockSize;
   unsigned long  mArchiveFlags;
};

struct STOCEntry
{
   unsigned char     mNameDigest[16];
   unsigned __int32  mBlockListStart;
   unsigned __int64  mOriginalSize;
   unsigned __int64  mStartOffset;
};

class CPsarcFile
{
public:
   typedef std::vector<STOCEntry>         TTOC;
   typedef std::vector< unsigned int >    TBlockSizeVector;
   typedef std::vector< unsigned char >   TDataBuffer;

public:
   int               ReadPsarcFile( const char * filename );

private:
   int              LoadEntireFile( const unsigned char * nameManifest, TDataBuffer & outDataBuffer ) const;

private:
   std::string       mFilename;
   SPsarcHeader      mHeader;
   TTOC              mTOC;
   TBlockSizeVector  mBlockSizes;
   TDataBuffer       mManifestData;
};

//----------------------------------------------------------------------------
