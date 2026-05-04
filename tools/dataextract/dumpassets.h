#ifndef __DUMPASSETS_H__
#define __DUMPASSETS_H__

#include "zlib.h"
#include <memory.h>
#include <string>
#include <vector>
#include "bpassetsupport.h"

class CStageDecoder
{
public:
   CStageDecoder();
   ~CStageDecoder();

   void SetInputStreamCompressedAndEncoded( FILE *inputStream, size_t inputStreamSize );
   void SetInputStreamDumb( FILE *inputStream, size_t inputStreamSize );
   void DecodeAndWrite( FILE *outputStream, size_t const outputSize );
   void *DecodeToBuffer( size_t const outputSize );
   void SkipOver( size_t const size );
private:
   void BufferAndDecodeMoreInput();
   void Decode( unsigned char *inputBuffer, size_t const amtToRead );

   FILE *mInputStream;
   bool mFirstRead;
   bool mIsCompressed;
   z_stream mZipStream;

   unsigned char mInputBuffer[ 16384 ];

   unsigned char *mpInputBufferHead;
   unsigned char *mpInputBufferTail;

   size_t mRemainingInput;
};

struct SBPAssetPair
{
   SBPAssetPair( const std::string & filePath, const std::string & archivePath )
      : mFilePath( filePath )
      , mArchivePath( archivePath )
      {
      }

   std::string    mFilePath;     //unified / flat filename in file system
   std::string    mArchivePath;  //duplicated filename in archive

   bool operator < ( const SBPAssetPair & rhs ) const;
};

struct SDumpAssetsState
{
   enum EDumpAssetsStage
   {
      kDAS_ScanningForSectionStart,
      kDAS_InsideOtherSection,
      kDAS_BeganDataSectionBeforeCompression,
      kDAS_InsideCompressedData,
      kDAS_InsideSoundSection,
      kDAS_SlotInfoStage,
      kDAS_SlotStage3
   };

   SDumpAssetsState()
      : mDumpAssetsStage( kDAS_ScanningForSectionStart )
      , mFilePosOfCompressedStream( 0 )
      , mFileSizeOfCompressedStream( 0 )
      , mPriorSectionTag( 0 )
      , mPriorSectionOffset( 0 )
      , mSlotInfoTag( 0 )
      , mpLastSectionName( NULL )
      , mDeletedManifestFileYet( false )
      , mIsCompressed( true )
      , mBPAssets()
   {
      memset( &mCompressedStream, 0, sizeof( mCompressedStream ) );
   }

   EDumpAssetsStage mDumpAssetsStage;
   CStageDecoder mDecodeStream;

   unsigned mFilePosOfCompressedStream;
   unsigned mFileSizeOfCompressedStream;
   z_stream mCompressedStream;

   unsigned mPriorSectionTag;
   unsigned mPriorSectionOffset;

   unsigned mSlotInfoTag;

   char const *mpLastSectionName;

   bool mDeletedManifestFileYet;

   bool mIsCompressed;

   std::vector <SBPAssetLine> mBPAssets;
};

extern void WriteAssetsForStage( 
   FILE *fpIn, 
   FILE *fpArchiveXml,
   char const *outputFolder, 
   char const *stageName, 
   char const *sdxFolder, 
   size_t const stageByteOffset, 
   size_t const soundSectionOffset, 
   unsigned const tag_id, 
   unsigned const tag_offset, 
   SDumpAssetsState *pState );

extern void WriteAssetsForSlot(
   FILE *fp, 
   FILE *fpArchiveXml,
   char const *outputFolder, 
   char const *stageName, 
   char const *sdxFolder, 
   unsigned const tag_id, 
   unsigned const tag_offset, 
   SDumpAssetsState *pState );

extern void WriteBPAssetsManifest( char const *outputFolder, char const *stageName, SDumpAssetsState *pState );

#endif