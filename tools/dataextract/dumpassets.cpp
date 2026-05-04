#include <stdio.h>
#include <algorithm>

#include "dumpassets.h"
#include "dataextract.h"
#include "kp.h"
#include <direct.h>
#include "bpassetsupport.h"

//----------------------------------------------------------------------------

bool SBPAssetPair::operator < ( const SBPAssetPair & rhs ) const
{
   if( mFilePath == rhs.mFilePath )
   {
      if( mArchivePath == rhs.mArchivePath )
      {
         printf("Error: duplicate asset pair in a stage or slot!\n");
//         throw false;
      }

      return mArchivePath < rhs.mArchivePath;
   }
   const std::string lhsExt = mFilePath.substr( mFilePath.find_last_of( '.' ) );
   const std::string rhsExt = rhs.mFilePath.substr( rhs.mFilePath.find_last_of( '.' ) );
   if( lhsExt == rhsExt )
   {
      //same asset type.
      return mFilePath < rhs.mFilePath;
   }
   else if( lhsExt == ".ctxr" )
   {
      //always sort ctxr's at the top.
      return true;
   }
   else if( rhsExt == ".ctxr" )
   {
      //always sort ctxr's at the top.
      return false;
   }

   //sort by filename.
   return mFilePath < rhs.mFilePath;
}

//----------------------------------------------------------------------------

CStageDecoder::CStageDecoder()
: mInputStream( NULL )
, mFirstRead( true )
, mIsCompressed( true )
{
   memset( &mZipStream, 0, sizeof( mZipStream ) );
}

//----------------------------------------------------------------------------

CStageDecoder::~CStageDecoder()
{
   if ( mInputStream && mIsCompressed )
   {
      inflateEnd( &mZipStream );
   }
}

//----------------------------------------------------------------------------

void CStageDecoder::SetInputStreamCompressedAndEncoded( FILE *inputStream, size_t inputStreamSize )
{
   // If CStageDecoder is given a new input stream, end the old one
   if ( mInputStream && mIsCompressed )
   {
      inflateEnd( &mZipStream );
   }

   // Initialize this new stream

   mInputStream = inputStream;
   mFirstRead = true;
   mIsCompressed = true;
   memset( &mZipStream, 0, sizeof( mZipStream ) );

   if ( Z_OK != inflateInit( &mZipStream ) )
   {
      printf( "InflateInit failed!\n" );

      throw kAbortException;
   }

   mpInputBufferHead = mpInputBufferTail = NULL;
   mRemainingInput = inputStreamSize;

   BufferAndDecodeMoreInput();
}

//----------------------------------------------------------------------------

void CStageDecoder::SetInputStreamDumb( FILE *inputStream, size_t inputStreamSize )
{
   // If CStageDecoder is given a new input stream, end the old one
   if ( mInputStream && mIsCompressed )
   {
      inflateEnd( &mZipStream );
   }

   mInputStream = inputStream;
   mFirstRead = true;
   mIsCompressed = false;

   mpInputBufferHead = mpInputBufferTail = NULL;
   mRemainingInput = inputStreamSize;
}

//----------------------------------------------------------------------------

void *CStageDecoder::DecodeToBuffer( size_t const outputSize )
{
   unsigned char *pWriting = (unsigned char *) malloc( outputSize );
   memset( pWriting, 0, outputSize );

   if ( !mIsCompressed )
   {
      if ( mRemainingInput < outputSize )
      {
         printf( "Trying to read %d when there is %d remaining\n", outputSize, mRemainingInput );
         throw kAbortException;

      }
      fread( pWriting, 1, outputSize, mInputStream );
    
      mRemainingInput -= outputSize;
   }
   else
   {   
      mZipStream.avail_out = outputSize;
      mZipStream.next_out = pWriting;

      while ( mZipStream.avail_out )
      {
         if ( mZipStream.avail_in == 0 )
         {
            BufferAndDecodeMoreInput();
         }

         int ret = inflate( &mZipStream, Z_SYNC_FLUSH );
         if ( ret < 0 )
         {
            printf( "Inflate returned error: %d %s, ai:%d, ao:%d\n", ret, mZipStream.msg, mZipStream.avail_in, mZipStream.avail_out );
            free( pWriting );
            throw kAbortException;
         }
      }
   }

   return pWriting;
}

//----------------------------------------------------------------------------

void CStageDecoder::SkipOver( size_t const size )
{
   fseek( mInputStream, size, SEEK_CUR );
}

//----------------------------------------------------------------------------

void CStageDecoder::DecodeAndWrite( FILE *outputStream, size_t const outputSize )
{
   // This function decodes outputSize bytes and writes them to outputStream...  It
   // just allocates the right amount of data to output into one buffer and goes to town

   void *pWriting = DecodeToBuffer( outputSize );

   fwrite( pWriting, 1, outputSize, outputStream );
   free( pWriting );
}

//----------------------------------------------------------------------------

void CStageDecoder::BufferAndDecodeMoreInput()
{
   if ( !mIsCompressed )
   {
      printf( "Trying to Decode a non decodable stream?!\n" );

      throw kAbortException;
   }

   // This buffers more input and decodes it, but does NOT decompress it

   size_t amtToRead = sizeof( mInputBuffer );

   if ( amtToRead > mRemainingInput )
   {
      amtToRead = mRemainingInput;
   }

   fread( mInputBuffer, 1, amtToRead, mInputStream );
   Decode( mInputBuffer, amtToRead );

   mpInputBufferHead = mInputBuffer;
   mpInputBufferTail = mInputBuffer + amtToRead;
   mRemainingInput -= amtToRead;

   mZipStream.avail_in = mpInputBufferTail - mpInputBufferHead;
   mZipStream.next_in = mpInputBufferHead;
}

//----------------------------------------------------------------------------

void CStageDecoder::Decode( unsigned char *inputBuffer, size_t const amtToRead )
{
   // This decode (or decrypt) step is pretty much copied from KP's tools

   if ( mIsCompressed )
   {

      if ( mFirstRead )
      {
         mFirstRead = false;
         KPCrypt::setup_decrypt( inputBuffer );
         //         set_encode();
         KPCrypt::decrypt_buffer( inputBuffer, amtToRead );
         *( short * )inputBuffer += ( 0x9C78 - 0x9385 );
      }
      else
      {
         KPCrypt::decrypt_buffer( inputBuffer, amtToRead );
      }
   }
   else
   {
      mFirstRead = false;
   }
}

//----------------------------------------------------------------------------

static void write_slot_page_information( char const *outputFolder, char const *stageName, unsigned const slot_info_tag, SDumpAssetsState *pState )
{
   unsigned size = ( slot_info_tag & 0x00FFFFFF ) * kSectorSize;
   unsigned num = ( slot_info_tag >> 24 ) & 0xFF;

   char outputPath[ _MAX_PATH ];

   sprintf( outputPath, "%s/%s/%s/%8.8x.slot", outputFolder, stageName, pState->mpLastSectionName, pState->mPriorSectionTag & 0x00FFFFFF );

   printf( "SLOT INFO to: %s - num:%d size:%d\n", outputPath, num, size );
   FILE *fp = fopen( outputPath, "r+" );

   fseek( fp, 0, SEEK_SET );

   fwrite( &num, sizeof( num ), 1, fp );
   fwrite( &size, sizeof( size ), 1, fp );

   fclose( fp );
}

//----------------------------------------------------------------------------

static void    decompress_prior_section( char const *outputFolder, char const *stageName, unsigned const next_tag_id, unsigned const next_tag_offset, SDumpAssetsState *pState, FILE * fpArchiveXml )
{
   // Since each section doesn't have a "size," we have to look at the next section's offset to 
   // see how to extract the prior section.  So this doesn't extract the section we're passing in,
   // it extracts the section before it (unless, obviously, it's the first section)

   if ( pState->mPriorSectionTag != 0 )
   {
      size_t fileSize = next_tag_offset - pState->mPriorSectionOffset;

      // Outputs to outputFolder/stageName/section_type
      char stageFolder[ _MAX_PATH ];
      char stageSectionFolder[ _MAX_PATH ];
      char pathname[ _MAX_PATH ];
      char manifestPathName[ _MAX_PATH ];
      char assetFilename[ _MAX_PATH ];

      sprintf( assetFilename, "%8.8x.%s", pState->mPriorSectionTag & 0x00FFFFFF, GetExtensionForTag( pState->mPriorSectionTag ).c_str() );

      sprintf( stageFolder, "%s/%s", outputFolder, stageName );
      sprintf( manifestPathName, "%s/manifest.txt", stageFolder );

      sprintf( stageSectionFolder, "%s/%s", stageFolder, pState->mpLastSectionName );
      sprintf( pathname, "%s/%s", stageSectionFolder, assetFilename );

      // It won't create outputFolder, but it'll create the stageName and section_type folders
      _mkdir( stageFolder );
      _mkdir( stageSectionFolder );

      if( fpArchiveXml )
      {
         std::string assetPath = UsingAssetRemapping() ? GetAssetRemappedName( pathname ) : std::string( pathname );

         //TEMP HACK!  To get rebuilt versions of gcx into the archive.  Need to move this redirection into the manifest.txt step,
         //that way we can get of the runtime fileexists check as well.
         int gcxExtAt = assetPath.find( ".gcx" );
         if( gcxExtAt != std::string::npos )
         {
            int lastSlashAt = assetPath.find_last_of( "/" );
            assetPath.insert( lastSlashAt, "/_bp" );
            extern bool BP_FileExists( const char * const path );
            if( !BP_FileExists( assetPath.c_str() ) )
            {
               //Allow to use original w/o new font system if the rebuilt files are not there.
               assetPath = UsingAssetRemapping() ? GetAssetRemappedName( pathname ) : std::string( pathname );
            }
         }
         add_file_to_archive_xml( fpArchiveXml, assetPath.c_str(), pathname );
         //Just so that the file position still matches up at the end.
         pState->mDecodeStream.SkipOver( fileSize );
      }
      else
      {
         printf( "Writing to %s, size %d, tag 0x%8.8x\n",
            pathname,
            fileSize,
            pState->mPriorSectionTag);

         {
            FILE *fp = fopen( pathname, "wb" );

            if ( fp == NULL )
            {
               printf( "... couldn't open for write.  Aborting.\n" );
               throw kAbortException;
            }

            pState->mDecodeStream.DecodeAndWrite( fp, fileSize );
            fclose( fp );
         }

         if ( IS_SLOT_TAG( pState->mPriorSectionTag ) )
         {
            write_slot_page_information( outputFolder, stageName, pState->mSlotInfoTag, pState );
         }

         FILE *manifestFile = fopen( manifestPathName, "at" );
         if ( manifestFile == NULL )
         {
            printf( "... couldn't open manifest '%s' for write.  Aborting.\n", manifestPathName );
            throw kAbortException;
         }

         if ( UsingAssetRemapping() )
         {
            char qualifiedFname[ _MAX_PATH ];

            sprintf( qualifiedFname, "%s/%s/%s", stageFolder, pState->mpLastSectionName, assetFilename );

            std::string remapped = GetAssetRemappedName( qualifiedFname );

            std::string archiveFilename = GetAssetArchiveFilename( qualifiedFname, remapped.c_str() );

            fprintf( manifestFile, "%s,%s,%s/%s\n", remapped.c_str(), archiveFilename.c_str(), pState->mpLastSectionName, assetFilename );
         }
         else
         {
            fprintf( manifestFile, "%s/%s\n", pState->mpLastSectionName, assetFilename );
         }

         fclose( manifestFile );
      }

      //Add BP assets as necessary to bp_assets.txt
      //Not writing these out unless we've got a texture flatlist remapping.
      if( UsingTextureFlatlistRemapping() )
      {
         const char * const fileExt = pathname + strlen( pathname ) - 4;

         std::vector< SBPAssetLine > bpAssetPaths;
         if( !strcmp( ".tri", fileExt ) )
         {
            AddTri( pathname );

            //add all ctxr files to the archive
            GetTriCtxrFilenames( pathname, bpAssetPaths );
         }
         if( !strcmp( ".img", fileExt ) )
         {
            //one ctxr file for each of these.
            SBPAssetLine bpAsset;

            GetImgCtxrFilenames( pathname, bpAsset );
            bpAssetPaths.push_back( bpAsset );
         }
#if MGS_VERSION==2
         else if( !strcmp( ".kms", fileExt ) || !strcmp( ".evm", fileExt ) )
#else
         else if( !strcmp( ".mdl", fileExt ) )
#endif
         {
            //add the cmdl to the archive (no remapping, so archive name is the same as file path)
            SBPAssetLine assetLine;

            if ( UsingAssetRemapping() )
            {
               assetLine.mArchivePath = GetCmdlFilename( GetAssetArchiveFilename( pathname, GetAssetRemappedName( pathname ).c_str() ).c_str() );
               assetLine.mFlatPath = GetCmdlFilename( GetAssetRemappedName( pathname ).c_str() );
            }
            else
            {
               assetLine.mArchivePath = GetCmdlFilename( pathname );
               assetLine.mFlatPath = GetCmdlFilename( pathname );
            }

            assetLine.mStagePath = GetCmdlFilename( pathname );

            bpAssetPaths.push_back( assetLine );
         }
#if MGS_VERSION == 2
         else if( !strcmp( ".zms", fileExt ) )
         {
            GetZmsCmdlFilenames(pathname, bpAssetPaths);
         }
#endif

         // Add our asset paths to the state
         pState->mBPAssets.insert( pState->mBPAssets.end(), bpAssetPaths.begin(), bpAssetPaths.end() );
      }
   }

   pState->mPriorSectionTag = next_tag_id;
   pState->mPriorSectionOffset = next_tag_offset;
}

//----------------------------------------------------------------------------

static void WriteAssetsForStage_Internal( 
   FILE *fpIn, 
   FILE *fpArchiveXml,
   char const *outputFolder, 
   char const *stageName, 
   char const *sdxFolder, 
   size_t const stageByteOffset, 
   size_t const soundSectionOffset, 
   unsigned const tag_id, 
   unsigned const tag_offset, 
   bool const isCompressed,
   SDumpAssetsState *pState )
{
   // This function is called once for every section inside a stage and is a dumb switch-based state machine
   // It scans for a section to start, and if it's a section with data (cache, nocache, or resident) then it 
   // will start scanning that section for assets.
   // There are also some special cases for slots and compressions, those will be talked about below
   
   if ( !pState->mDeletedManifestFileYet )
   {
      char manifestPath[ _MAX_PATH ];
      char sdxManifestPath[ _MAX_PATH ];
      const char * const afterRegionAt = strrchr( outputFolder, '/' );

      sprintf( manifestPath, "%s/%s/%s", outputFolder, stageName, "manifest.txt" );
      //HACK!  All sdx files are the same among all three PS2 SKU.
      //Force all to use the us copy since that's the only one we'll have in source control.
      //These will likely go through an intra-SKU unification process fairly soon anyway.
      // HACK HACK HACK - Look for this comment above elsewhere in the file
      sprintf( sdxManifestPath, "%s%s/%s/%s", sdxFolder, afterRegionAt, stageName, "sdxmanifest.txt" );

      if( fpArchiveXml )
      {
         //archive xml generation mode.  Include manifest file, bp assets file, and bp asset files.
         char bpAssetsManifestPath[ _MAX_PATH ];

         sprintf( bpAssetsManifestPath, "%s/%s/%s", outputFolder, stageName, "bp_assets.txt" );
         //include manifest.txt for this stage.
         add_file_to_archive_xml( fpArchiveXml, manifestPath, manifestPath );
         //include bp_assets.txt for this stage.
         add_file_to_archive_xml( fpArchiveXml, bpAssetsManifestPath, bpAssetsManifestPath );
         //include bp assets for this stage.
         FILE *bpManifestFile = fopen( bpAssetsManifestPath, "rt" );
         if ( bpManifestFile == NULL )
         {
            printf( "... couldn't open %s for read.  Aborting.\n", bpAssetsManifestPath );
            throw kAbortException;
         }

         //N.B. only include each duplicated asset once here.  The game will only attempt to load the first copy.
         char manifestLine[FILENAME_MAX];
         std::string prevFilePath;
         while ( fgets( manifestLine, sizeof(manifestLine)-1, bpManifestFile ) )
         {
            char filePath[FILENAME_MAX];
            char archivePath[FILENAME_MAX];
            const char seps[]   = " ,\t\n";

            char * token = strtok( manifestLine, seps );
            if( *token == NULL )
            {
               printf("Error: empty column bp assets file list!\n" );
               throw false;
            }
            _strlwr( token );
            strcpy( filePath, token );
            token = strtok( NULL, seps );
            if( *token == NULL )
            {
               printf("Error: empty column bp assets file list!\n" );
               throw false;
            }
            _strlwr( token );
            strcpy( archivePath, token );

            //Add platform subfolder only at the point of adding the reference in the xml file.
            //needs to remain without inside the manifest file.
            InsertPlatformSubfolder( filePath );
            InsertPlatformSubfolder( archivePath );

            std::string currFilePath = filePath;
            if( currFilePath != prevFilePath )
            {
               //not a dupe, include it.
               prevFilePath = currFilePath;
               add_file_to_archive_xml( fpArchiveXml, filePath, archivePath );
            }
            //else skip it, this asset is a dupe.
         }

         fclose( bpManifestFile );
      }
      else
      {
         //extraction mode.  Clear out manifest file.
         _unlink( manifestPath );
         _unlink( sdxManifestPath );
      }

      pState->mDeletedManifestFileYet = true;
   }

   SDumpAssetsState::EDumpAssetsStage const currentStage = pState->mDumpAssetsStage;
   SDumpAssetsState::EDumpAssetsStage nextStage = currentStage;

   switch ( currentStage )
   {
   case SDumpAssetsState::kDAS_ScanningForSectionStart:
      // This is the default state - we're looking for a section to begin.  From this, we'll either be
      // going to a known asset section or another section.  If it's an asset section, the next state 
      // will be to search for the compression tag.

      if ( IS_SECTION_TAG( tag_id ) )
      {
         if ( IS_SECTION_END( tag_id ) )
         {
            printf( "Got end section while scanning for section start\n" );

            throw kAbortException;
         }

         switch ( tag_id )
         {
         case ID_CACHE:
            pState->mpLastSectionName = "cache";
            nextStage = SDumpAssetsState::kDAS_BeganDataSectionBeforeCompression;
            break;
         case ID_NOCACHE:
            pState->mpLastSectionName = "nocache";
            nextStage = SDumpAssetsState::kDAS_BeganDataSectionBeforeCompression;
            break;
         case ID_RESIDENT:
            pState->mpLastSectionName = "resident";
            nextStage = SDumpAssetsState::kDAS_BeganDataSectionBeforeCompression;
            break;
         case ID_SOUND:
            {
               pState->mpLastSectionName = "sound";
               nextStage = SDumpAssetsState::kDAS_InsideSoundSection;

               pState->mPriorSectionOffset = 0;
               //N.B. 0 is a valid tag_id within sound sections.
               pState->mPriorSectionTag = 0xffffffff;

               //Skip to the beginning of the sound section in the file.
               fseek( fpIn, soundSectionOffset + stageByteOffset, SEEK_SET );
            }
            break;
         default:
            nextStage = SDumpAssetsState::kDAS_InsideOtherSection;
            break;
         }

         if ( nextStage == SDumpAssetsState::kDAS_BeganDataSectionBeforeCompression && !isCompressed )
         {
            // If we're not compressed, then do the uncompressed part here

            nextStage = SDumpAssetsState::kDAS_InsideCompressedData;

            pState->mPriorSectionOffset = 0;
            pState->mPriorSectionTag = 0;

            pState->mDecodeStream.SetInputStreamDumb( fpIn, 0x7FFFFFFF );
         }
      }
      break;
   case SDumpAssetsState::kDAS_InsideSoundSection:
      {
         //These are uncompressed .sdx files.
         if( pState->mPriorSectionTag != 0xffffffff )
         {
            //Extract another .sdx file.
            int sdxSize = tag_offset - pState->mPriorSectionOffset;
            char sdxName[80];
            sprintf( sdxName, "pk%06x.sdx", pState->mPriorSectionTag );

            char dirname[ 1024 ];
            char pathname[ 1024 ];
            //"us/stage"

            //HACK!  All sdx files are the same among all three PS2 SKU.
            //Force all to use the us copy since that's the only one we'll have in source control.
            //These will likely go through an intra-SKU unification process fairly soon anyway.
            // HACK HACK HACK - Look for this comment above elsewhere in the file
            const char * const afterRegionAt = strrchr( outputFolder, '/' );
            sprintf( dirname, "%s%s/%s", sdxFolder, afterRegionAt, stageName );
            sprintf( pathname, "%s/%s", dirname, sdxName );

            if( fpArchiveXml )
            {
               add_file_to_archive_xml( fpArchiveXml, pathname, pathname );
               fseek( fpIn, sdxSize, SEEK_CUR );
            }
            else
            {
               printf( "Writing to %s, size %d.\n",
                  pathname,
                  sdxSize );

               _mkdir( dirname );

               char sdxManifest[ 1024 ];
               sprintf( sdxManifest, "%s/sdxmanifest.txt", dirname, sdxName );
               FILE *sdxManifestFile= fopen( sdxManifest, "at" );
               if ( sdxManifestFile == NULL )
               {
                  printf( "... couldn't open sound manifest '%s' for write.  Aborting.\n", sdxManifest );
                  throw kAbortException;
               }

               fprintf( sdxManifestFile, "%s\n", sdxName );
               fclose( sdxManifestFile );

               FILE *sdxfp = fopen( pathname, "wb" );

               if ( sdxfp == NULL )
               {
                  printf( "... couldn't open for write.  Aborting.\n" );
                  throw kAbortException;
               }

               {
                  char * sdxBuf = new char[sdxSize];
                  fread( sdxBuf, 1, sdxSize, fpIn );
                  fwrite( sdxBuf, 1, sdxSize, sdxfp );
                  delete sdxBuf;
               }

               fclose(sdxfp);
            }
         }

         pState->mPriorSectionTag = tag_id;
         pState->mPriorSectionOffset = tag_offset;

         if ( IS_SECTION_END( tag_id ) )
         {
            nextStage = SDumpAssetsState::kDAS_ScanningForSectionStart;
         }
      }
      break;

   case SDumpAssetsState::kDAS_BeganDataSectionBeforeCompression:
      // Here we should definitely have a compression tag, as we're in an asset section.
      // If we don't have a compression tag, we bail, otherwise we set up the decoding stream
      // and move on to being inside compressed data

      if ( IS_COMPRESSION_TAG( tag_id ) )
      {
         pState->mFilePosOfCompressedStream = tag_offset;
         pState->mFileSizeOfCompressedStream = COMPRESSION_SIZE( tag_id );

         pState->mPriorSectionOffset = 0;
         pState->mPriorSectionTag = 0;

         fseek( fpIn, tag_offset + stageByteOffset, SEEK_SET );
         pState->mDecodeStream.SetInputStreamCompressedAndEncoded( fpIn, pState->mFileSizeOfCompressedStream );

         nextStage = SDumpAssetsState::kDAS_InsideCompressedData;
      }
      else
      {
         printf( "Expected compression tag - got none!\n" );
         throw kAbortException;
      }
      break;

   case SDumpAssetsState::kDAS_InsideOtherSection:
      // We don't know anything about this section, let's just wait for it to be over.
      if ( IS_SECTION_END( tag_id ) )
      {
         nextStage = SDumpAssetsState::kDAS_ScanningForSectionStart;
      }
      break;

   case SDumpAssetsState::kDAS_InsideCompressedData:
      // We're in compressed data, chug through...

      decompress_prior_section( outputFolder, stageName, tag_id, tag_offset, pState, fpArchiveXml );

      // If we're a "slot" we have to handle it differently.  This is a "command" tag, not an
      // asset tag.  It's also treated quite differently than most tags, as there are 3 of these
      // in a row.  The first tag is similar to an asset tag, with a normal offset value.  The other
      // two have info we don't know/care about, so we enter a state to ignore them.

      if ( IS_SLOT_TAG( tag_id ) )
      {
         nextStage = SDumpAssetsState::kDAS_SlotInfoStage;
      }
      else
      {
         if ( IS_SECTION_END( tag_id ) )
         {
            nextStage = SDumpAssetsState::kDAS_ScanningForSectionStart;
         }
      }
      break;
   case SDumpAssetsState::kDAS_SlotInfoStage:
      // Extra slot tag #1/2 - Always has a SLOT_NAME of 0 and the offset
      // encodes the number of slot pages and size

      if ( !IS_SLOT_TAG( tag_id ) || SLOT_NAME( tag_id ) != 0 )
      {
         printf( "Error: Expected empty slot tag for stage 2, got %8.8x\n", tag_id );
         throw kAbortException;
      }
   
      pState->mSlotInfoTag = tag_offset;

      nextStage = SDumpAssetsState::kDAS_SlotStage3;
      break;
   case SDumpAssetsState::kDAS_SlotStage3:
      // Garbage slot tag - Always has a SLOT_NAME of 0 and a bogus offset
      // Go back to being inside compressed data

      if ( !IS_SLOT_TAG( tag_id ) || SLOT_NAME( tag_id ) != 0 )
      {
         printf( "Error: Expected empty slot tag for stage 3, got %8.8x\n", tag_id );
         throw kAbortException;
      }

      nextStage = SDumpAssetsState::kDAS_InsideCompressedData;
      break;
   default:
      {
         printf("Internal error: unhandled state %d.\n", currentStage );
         throw kAbortException;
      }
   }

   pState->mDumpAssetsStage = nextStage;
}

void WriteAssetsForStage( 
                         FILE *fpIn,
                         FILE *fpArchiveXml,
                         char const *outputFolder, 
                         char const *stageName, 
                         char const *sdxFolder, 
                         size_t const stageByteOffset, 
                         size_t const soundSectionOffset, 
                         unsigned const tag_id, 
                         unsigned const tag_offset, 
                         SDumpAssetsState *pState )
{
   WriteAssetsForStage_Internal( fpIn, fpArchiveXml, outputFolder, stageName, sdxFolder, stageByteOffset, soundSectionOffset, tag_id, tag_offset, true, pState );
}

void WriteAssetsForSlot(
   FILE *fp, 
   FILE *fpArchiveXml,
   char const *outputFolder, 
   char const *stageName, 
   char const *sdxFolder, 
   unsigned const tag_id, 
   unsigned const tag_offset, 
   SDumpAssetsState *pState )
{
   WriteAssetsForStage_Internal( fp, fpArchiveXml, outputFolder, stageName, sdxFolder, 0, 0, tag_id, tag_offset, false, pState );
}

void WriteBPAssetsManifest( char const *outputFolder, char const *stageName, SDumpAssetsState *pState )
{
   if( !UsingTextureFlatlistRemapping() )
   {
      //Not writing these out unless we've got a texture flatlist remapping.
      return;
   }

   //Textures need to be at the top; duplicated assets need to be grouped.
   std::sort( pState->mBPAssets.begin(), pState->mBPAssets.end() );

   char bpAssetsManifestPath[ _MAX_PATH ];
   sprintf( bpAssetsManifestPath, "%s/%s/%s", outputFolder, stageName, "bp_assets.txt" );
   FILE *manifestFile = fopen( bpAssetsManifestPath, "wt" );
   if ( manifestFile == NULL )
   {
      printf( "... couldn't open bp_assets.txt for write.  Aborting.\n" );
      throw kAbortException;
   }

   for( int i=0; i < (int)pState->mBPAssets.size(); ++i )
   {
      fprintf( manifestFile, "%s,%s,%s\n", 
         pState->mBPAssets[i].mFlatPath.c_str(),
         pState->mBPAssets[i].mArchivePath.c_str(),
         pState->mBPAssets[i].mStagePath.c_str() );
   }
   fclose( manifestFile );
}
