#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <vector>
#include <map>

#include "zlib.h"
#include "kp.h"
#include "dataextract.h"
#include "dumpassets.h"

#include "bpassetsupport.h"

#define BP_SPLIT_STREAM_EXTENSION ".sdt"

typedef unsigned uint32;

extern "C"
{
   void * zcalloc(void *opaque, unsigned items, unsigned size)
   {
      return malloc( items * size );
   }
   void zcfree(void * opaque, voidpf ptr)
   {
      free( ptr );
   }

   int z_verbose = 0;
   void z_error( char *m )
   {
      printf( "%s\n", m );
      abort();
   }
}

static void _str_tolower(char * pStr)
{
   for( char * pc = pStr; *pc != NULL; ++pc )
   {
      *pc = tolower( *pc );
   }
}

bool BP_FileExists( const char * const path )
{
   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesExA( path, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      return 0;
   }

   return 1;
}

void add_file_to_archive_xml( FILE * fp, const char * const filePath, const char * const archivePath )
{
   char filePathLower[FILENAME_MAX];
   char archivePathLower[FILENAME_MAX];
   strcpy( filePathLower, filePath );
   strcpy( archivePathLower, archivePath );
   _str_tolower( filePathLower );
   _str_tolower( archivePathLower );
   fprintf( fp, "\t\t<file path=\"%s\" archivepath=\"/%s\"/>\n", filePathLower, archivePathLower );
}

void add_dat_file_to_archive_xml( FILE * archivefp, const char * const datFolder )
{
   //Add individual stream files that have been broken out.
   char fullPath[FILENAME_MAX];
   sprintf( fullPath, "%s/_bp/%s/bp_streams.txt", datFolder, GetPlatformSubfolder() );
   FILE * fp = fopen( fullPath, "rt" );
   if( !fp )
   {
      printf("Error: could not open %s for reading.\n", fullPath );
      throw false;
   }

   char currLine[FILENAME_MAX];
   for( int line = 0; fgets( currLine, 1023, fp ); ++line )
   {
      const char seps[] = " \n";

      char * token = strtok( currLine, seps );
      if( *token == NULL )
      {
         printf("Error: empty column in lst txt file line %d\n", line+1 );
         throw false;
      }
      //ignore dat file offset.
      token = strtok( NULL, seps );
      if( *token == NULL )
      {
         printf("Error: empty column in lst txt file line %d\n", line+1 );
         throw false;
      }

      add_file_to_archive_xml( archivefp, token, token );
   }

   fclose(fp);
}

enum EDataType
{
   kDT_Stage,
   kDT_Slot,
   kDT_Face
};

enum EOperationMode
{
   kOM_Extract,
   kOM_BuildArchiveXML
};

// This structure represents the command line options
struct SCommandLine
{
   SCommandLine()
      : mInputName( NULL )
      , mOutputFolder( NULL )
      , mStageFilter( NULL )
      , mArchiveFilename( NULL )
      , mTexDupeStatsFilename( NULL )
      , mTexFlatListFilename( NULL )
      , mAssetListFilename( NULL )
      , mArchivePlatform( NULL )
      , mSdxFolder( NULL )
      , mVerbose( false )
      , mWriteSlotPageFile( false )
      , mAppendStageList( false )
      , mDataType( kDT_Stage )
      , mOperationMode( kOM_Extract )
   {
   }

   char const *mInputName;
   char const *mOutputFolder;
   char const *mStageFilter;
   char const *mArchiveFilename;
   char const *mArchiveInputFilename;
   char const *mTexDupeStatsFilename;
   char const *mTexFlatListFilename;
   char const *mAssetListFilename;
   char const *mArchivePlatform;
   char const *mSdxFolder;
   bool mVerbose;
   bool mWriteSlotPageFile;
   bool mAppendStageList;
   EDataType mDataType;
   EOperationMode mOperationMode;
};

// As we're iterating through a stage, this is the state that is kept
// when printing verbose info about the stage
struct SPrintInfoState
{
   SPrintInfoState()
      : mInsideAsset( false )
      , mInsideSound( false )
      , mSlotStep( 0 )
   {
   }

   bool mInsideAsset;
   bool mInsideSound;
   int mSlotStep;
};


enum EDatFileArchiveType
{
   kDF_Dat,    //archive all streams / codecs from dat file
   kDF_DatLst, //archive just the lst file for this dat file
   kDF_Stage,  //parse as stage dat file and archive individual files
   kDF_Slot,   //parse as slot dat file and archive individual files
   kDF_Face,   //parse as face dat file and archive individual files
};

struct SDatFileInfo
{
   char                 mFilename[FILENAME_MAX];
   char                 mOutputFolder[FILENAME_MAX];
   EDatFileArchiveType  mFileType;
};

#if 0
static const SDatFileInfo skDatFileInfo[] = 
{
#if MGS_VERSION==2
   { "STAGE.DAT", "stage", kDF_Stage },
   { "CODEC.DAT", NULL, kDF_Dat },
   { "FACE.DAT", "face", kDF_Face },
   { "MOVIE.DAT", NULL, kDF_Dat },
   { "VOX.DAT", NULL, kDF_Dat },
   { "DEMO.DAT", NULL, kDF_Dat },

   { "MOVIEVR.DAT", NULL, kDF_Dat },
   { "STAGE2.DAT", "stage", kDF_Stage },
   { "VOX2.DAT", NULL, kDF_Dat },
   { "DEMO2.DAT", NULL, kDF_Dat },
#else
   { "BGM.DAT", NULL, kDF_Dat },
   { "CODEC.DAT", NULL, kDF_Dat },
   { "DEMO.DAT", NULL, kDF_Dat },
   { "MOVIE.DAT", NULL, kDF_Dat },
   { "SLOT.DAT", "slot", kDF_Slot },
   { "STAGE.DAT", "stage", kDF_Stage },
   { "VOX.DAT", NULL, kDF_Dat },
#endif
};
#else
std::vector< SDatFileInfo >   sDatFileInfo;
#endif

struct SExtensionMap
{
#if MGS_VERSION==2
   // In MGS2, a file type is the first letter of the file extension
   char letter;
#elif MGS_VERSION==3
   // In MGS3, a file type can be 0..255 (well, except type & 7 can't be 7)...
   unsigned char code;
#endif
   char const *ext;
};

#if MGS_VERSION==2
// This is an extension list.  While it's called "unsorted" and appears to be 
// sorted, it doesn't have to be.  
SExtensionMap const skUnsortedExtensions[] = 
{
   { 'a', "anm" },
   { 'b', NULL },
   { 'c', "cv2" }, // not in idl
   { 'd', NULL },
   { 'e', "evm" },
   { 'f', "far" },
   { 'g', "gcx" },
   { 'h', "hzx" },
   { 'i', NULL },
   { 'j', NULL },
   { 'k', "kms" },
   { 'l', "lt2" },
   { 'm', "mar" },
   { 'n', NULL },
   { 'o', "o2d" },
   { 'p', NULL },
   { 'q', NULL },
   { 'r', "row" }, // not in idl
   { 's', "sar" },
   { 't', "tri" },
   { 'u', NULL },
   { 'v', "var" },
   { 'w', NULL },
   { 'x', NULL },
   { 'y', NULL },
   { 'z', "zms" },
   { 0, NULL }
};

// These will be the sorted extensions
char const *sExtensions[ 26 ] = { NULL };

#elif MGS_VERSION==3
// This is an extension list of MGS3 file types to file extensions  

SExtensionMap const skUnsortedExtensions[] = 
{
   {  0xFF,	   "psq" },
   {  0x5e,	   "ico" },
   {  0x5f,	   "la2" },
   {  0x7D,	   "slot" },
   {  0x69,	   "img" },
   {  0x6A,	   "vib" },
   {  0x6B,	   "rat" },
   {  0x6C,	   "rcm" },
   {  0x6D,	   "ola" },
   {  0x6E,    "row" },
   {  0x6E,    "raw" },
   {  0x6F,	   "mtra" },
   {  0x68,	   "var" },
   {  1   ,    "bin" },
   {  2   ,    "gcx" },
   {  3   ,    "tri" },
   {  4   ,    "mdh" },
   {  5   ,    "mds" },
   {  6   ,    "lt2" },
   {  7   ,    "cv2" },
   {  8   ,    "mtar" },
   {  9   ,    "mtsq" },
//   {  10  , 	"mtfa" },
   {  10  , 	"far" },
   {  11  , 	"mtcm" },
   {  12  ,    "geom" },
//   {  13  , 	"mdc" },
//   {  13  , 	"mdb" },
   {  13  , 	"mdl" },
   {  15  ,    "nav" },
   {  16  , 	"cvd" },
   {  17  ,    "eft" },
   {  18  ,    "zon" },
   {  19  ,    "rpd" },
   { 0, NULL }
};

// These will be the sorted extensions
char const *sExtensions[ 0x100 ] = { NULL };
#endif


static void init_extensions()
{  
   // This copies the extries from the extension map into sExtensions for easier lookup

   static bool sInitted = false;
   if ( !sInitted )
   {
      sInitted = true;

#if MGS_VERSION == 2
      for ( SExtensionMap const *pEntry = skUnsortedExtensions; pEntry->letter != 0; ++pEntry )
      {
         if ( pEntry->letter >= 'a' && pEntry->letter <= 'z' )
         {
            sExtensions[ pEntry->letter - 'a' ] = pEntry->ext;
         }
         else
         {
            printf( "Extension map is bad.\n" );
            throw kAbortException;
         }
      }
#elif MGS_VERSION == 3
      for ( SExtensionMap const *pEntry = skUnsortedExtensions; pEntry->code != 0; ++pEntry )
      {
         sExtensions[ pEntry->code ] = pEntry->ext;
      }
#endif
   }
}

std::string const GetExtensionForTag( unsigned const tag_id )
{
   // Returns the file extension for a given tag, doing some validation
   // if necessary

   init_extensions();

#if MGS_VERSION==2
   char c = char( 'a' + ( tag_id >> 24 ) );

   if ( c < 'a' || c > 'z' )
   {
      printf( "Invalid tag_id %8.8x found!\n", tag_id );
      throw kAbortException;
   }

   char const *ext = sExtensions[ c - 'a' ];
   if ( !ext )
   {
      printf( "No extension for letter %c found with tag_id %8.8x\n", c, tag_id );
      throw kAbortException;
   }

   return std::string( ext );
#elif MGS_VERSION==3
   if ( IS_SLOT_TAG( tag_id ) )
   {
      // Slots are "special," since a slot tag is technically a command tag
      // TODO: More work in pre-decoding slot data

      return std::string( "slot" );
   }
   else
   {
      unsigned char code = (unsigned char) ( tag_id >> 24 );

      char const *ext = sExtensions[ code ];
      if ( !ext )
      {
         printf( "No extension for code %2.2x(%d) found with tag_id %8.8x\n", code, code, tag_id );
         throw kAbortException;
      }

      return std::string( ext );
   }
#endif
}

static std::string const MakeFourCC( unsigned fourcc )
{
   char out[5] = 
   {
      fourcc & 0xFF,
      ( fourcc >> 8 ) & 0xFF,
      ( fourcc >> 16 ) & 0xFF,
      ( fourcc >> 24 ) & 0xFF,
      0
   };

   for ( int i = 0; i < 4; ++i )
   {
      if ( out[i] <= 26 )
      {
         out[i] = '?';
      }
   }

   return std::string( out );
}

static void PrintUsage()
{
   printf( 
      "Data Extractor for MGS%d .DAT files.\n"
      "\n"
      "Usage:\n"
      "dataextract [-v] [-o outfolder] [-slot] [-s stage] [-sdx folder] <filename>\n" 
      "\n"
      " -v            - Verbose\n"
      " -slot         - Extracting a slot\n"
      " -writePages   - Writes out a slot pages file and creates the folders\n"
      " -appendStages - Opens stages.txt for append instead of write\n"
      " -face         - Extracting a face\n"
      " -o outfolder  -  Extract stage/slot data to outfolder\n"
      " -s stage      - Only process stage\n"
      " -sdx folder   - specify region folder for sdx output (default us, override is for jp_sol)\n"
      " -x filename   - Build archive layout xml file\n"
      " -a filename   - Specify archive input xml file\n"
      " -t filename   - Specify texture duplication stats output xml file\n"
      " -l filename   - Specify flatlist texture remapping file (won't build bp_assets.txt without)\n"
      " -la filename  - Specify asset list remapping file\n"
      " /p[]          - Specify platform for generated archive xml file\n"
      "\n", MGS_VERSION );
   
}

static void print_info_for_tag( unsigned const tag_id, unsigned const tag_offset, SPrintInfoState *pState )
{
   // This function is called simply to print the verbose information for a given
   // stage tag.
   // It will also update the state variable to say if we're inside or outside a section that contains assets

   std::string tagAttr;

   if ( tag_id == ID_BINARY )
   {
      tagAttr = "SECTION binary";
   }
   else if ( tag_id == ID_SECTION_END )
   {
      tagAttr = "SECTION end";
      pState->mInsideAsset = false;
      pState->mInsideSound = false;
   }
   else if ( tag_id == ID_CACHE )
   {
      tagAttr = "SECTION cache";
      pState->mInsideAsset = true;
   }
   else if ( tag_id == ID_NOCACHE )
   {
      tagAttr = "SECTION nocache";
      pState->mInsideAsset = true;
   }
   else if ( tag_id == ID_RESIDENT )
   {
      tagAttr = "SECTION resident";
      pState->mInsideAsset = true;
   }
   else if ( tag_id == ID_SOUND )
   {
      tagAttr = "SECTION sound";
      pState->mInsideSound = true;
   }
   else if ( tag_id == ID_BINARY )
   {
      tagAttr = "binary";
   }
   else if( pState->mInsideSound )
   {
      //tag_id is the index of the .sdx file
      //N.B. check this case before IS_END_TAG because the first .sdx file has the same tag as end
      char str[80];
      sprintf( str, "pk%06x.sdx", tag_id );

      tagAttr = str;
   }
   else if ( IS_END_TAG( tag_id ) )
   {
      tagAttr = "end";
   }
   else if ( IS_COMPRESSION_TAG( tag_id ) )
   {
      char zipSize[80];

      sprintf( zipSize, "zip %d (%8.8x)", COMPRESSION_SIZE(tag_id), COMPRESSION_SIZE(tag_id) );

      tagAttr = zipSize;
   }
   else if ( IS_SLOT_TAG( tag_id ) )
   {
      char slotSize[80];

      if ( pState->mSlotStep == 1 )
      {
         unsigned size = ( tag_offset & 0x00FFFFFF ) * kSectorSize;
         unsigned num = ( tag_offset >> 24 ) & 0xFF;

         sprintf( slotSize, "slot pages:%d ppage:%d total:%d", num, size, num * size );
      }
      else
      {
         sprintf( slotSize, "slot %8.8x", SLOT_NAME( tag_id ) );
      }

      tagAttr = slotSize;

      pState->mSlotStep++;
      if ( pState->mSlotStep > 2 )
      {
         pState->mSlotStep = 0;
      }
   }
   else if ( pState->mInsideAsset )
   {
      char str[80];
      sprintf( str, "ext: %s", GetExtensionForTag( tag_id ).c_str() );

      tagAttr = str;
   }

   printf( "  Section: %8.8x Off: %8d %s\n", 
      tag_id,
      tag_offset,
      tagAttr.c_str() );
}

static size_t get_sound_section_top( std::vector< DATACNF_TAG > const & tags )
{
   //Modified version from MGS3.  Iterate through tags up to ID_SOUND
   //to calculate the offset of the sound section.

   DATACNF_TAG const *p;
   size_t pos;

   p = &tags[0];
   pos = 0;
   for( ; !IS_END_TAG( p->id ); p++ ){
      if( p->id == ID_SOUND )
      {
         return ( pos );
      }
      else if( IS_SECTION_TAG( p->id ) && !IS_SECTION_END( p->id ) ){
         if( IS_COMPRESSION_TAG( ( p + 1 )->id ) ){
            p = p + 1;
            pos = p->offset + round_up_sector( COMPRESSION_SIZE( p->id ) );
         } else {
            pos += round_up_sector( p->offset );
         }
      }
   }
   return 0;
}

static void process_data_file_stage( char const *outputFolder, char const *stageName, char const * sdxFolder, bool const verbose, unsigned const byteOffset, unsigned const tableSeed, FILE *fpIn, FILE * fpArchiveXml )
{
   // This function takes an already opened file and processes one stage's data within that file.
   
   fseek( fpIn, byteOffset, SEEK_SET );
   int stage_id = GV_StrCode( stageName );

   // Set up our seed for decrypting the stage header
   KPCrypt::seed = 0xA78925D9 + ( stage_id << 7 )
      + stage_id + tableSeed;
   KPCrypt::_seed = 0x7A88FB59 + ( stage_id << 7 ) + stage_id;

   // Read in and decrypt the header information.  This consists of the 
   // number of tags and the tags themselves

   int numTags = 0;
   fread( &numTags, sizeof( int ), 1, fpIn );
   KPCrypt::decrypt_buffer( &numTags, sizeof( int ) );

   std::vector< DATACNF_TAG > tags;
   tags.resize( numTags );
   fread( &( tags[0] ), sizeof( DATACNF_TAG ), numTags, fpIn );
   KPCrypt::decrypt_buffer( &( tags[0] ), numTags * sizeof( DATACNF_TAG ) );

   printf( "Stage id: %8.8x, Tags: %d, Start: %d\n", stage_id, numTags, byteOffset );

   // The header is stored in sector-sized chunks, so the byte offset to the data needs to be rounded up
   size_t const byteOffsetToData = byteOffset + round_up_sector( sizeof( DATACNF_TAG ) + numTags * sizeof( DATACNF_TAG ) );

   size_t const soundSectionOffset = get_sound_section_top( tags );

   // Per-stage state structures for the operations we can do on a stage
   SPrintInfoState printInfoState;
   SDumpAssetsState dumpAssetsState;

   for ( size_t i = 0; i < tags.size(); ++i )
   {
      if ( i == 0 && tags[i].id == ID_SECTION_END )
      {
         // mkcdimg bug according to Japanese
         continue;
      }

      unsigned tag_id = tags[i].id;

      if ( verbose )
      {
         print_info_for_tag( tag_id, tags[i].offset, &printInfoState );
      }

      if ( outputFolder )
      {
         WriteAssetsForStage( fpIn, fpArchiveXml, outputFolder, stageName, sdxFolder, byteOffsetToData, soundSectionOffset, tag_id, tags[i].offset, &dumpAssetsState );
      }
   }
   
   if( !fpArchiveXml )
   {
      WriteBPAssetsManifest( outputFolder, stageName, &dumpAssetsState );
   }
}

static void fread_assert( void *buffer, size_t elementSize, size_t elementCount, FILE *fp, char const *what )
{
   size_t realElementCount = fread( buffer, elementSize, elementCount, fp );

   if ( realElementCount != elementCount )
   {
      printf( "ERROR: Expected to read %d %s - only got %d\n", elementCount, what, realElementCount );
      throw kAbortException;
   }
}

static std::string const read_next_line( FILE *fp )
{
   char line[ 255 ];
   line[0] = 0;
   fgets( line, sizeof( line ) - 1, fp );

   int len = strlen( line );

   if ( len > 0 && line[ len - 1 ] == '\n')
   {
      line[ len - 1 ] = 0;
   }

   return std::string( line );
}

static unsigned decode_hex( char const *str )
{
   unsigned id = 0;
   for ( char const *pc = str; *pc; ++pc )
   {
      char c = tolower( *pc );

      if ( c >= '0' && c <= '9' )
      {
         id = unsigned(c-'0') | ( id << 4 );
      }
      else if ( c >= 'a' && c <= 'f' )
      {
         id = unsigned( 10 + c - 'a' ) | ( id << 4 );
      }
   }

   return id;
}

static std::map< uint32, std::string > get_slot_pages_mapping_from_file( char const *pagesFile )
{
   FILE *f = fopen( pagesFile, "rt" );
   std::map<uint32, std::string> mapping;

   if ( f == NULL )
   {
      printf( "Fatal error: Could not open %s for read\n", pagesFile );
      throw kUsageException;
   }

   while ( !feof( f ) )
   {
      std::string line = read_next_line( f );

      if ( !line.empty() )
      {
         char const *space = strchr( line.c_str(), ' ' );
         char const *num_begin = NULL;
         char const *name_end = NULL;
         if ( space )
         {
            num_begin = space + 1;
            name_end = space;
         }
         else
         {
            num_begin = line.c_str();
            name_end = line.c_str() + line.size();
         }

         if ( strlen( num_begin ) != 8 ) 
         {
            printf( "pages.txt error: line '%s' malformed\n", line.c_str() );
            throw kUsageException;
         }

         uint32 id = decode_hex( num_begin );

         // Remove the top 3 nibbles of id for the mapping to look up by offset only (not size)
         id &= 0x000FFFFF;

         mapping.insert( std::pair<uint32,std::string>( id, std::string( line.c_str(), name_end ) ) );
      }
   }

   fclose( f );

   return mapping;
}

static void process_slot_file( FILE *fpIn, FILE * fpArchiveXml, SCommandLine const &commandLine, STAGE_TABLE *pFaceStageTable )
{
   int iIdx = 0;

   char pagesPath[ _MAX_PATH ];
   sprintf( pagesPath, "%s/pages.txt", commandLine.mOutputFolder );
   std::map<uint32, std::string> slotPagesMapping;

   if ( pFaceStageTable == NULL && !commandLine.mWriteSlotPageFile )
   {
      slotPagesMapping = get_slot_pages_mapping_from_file( pagesPath );
   }

   fseek( fpIn, 0, SEEK_END );
   long fileSize = ftell( fpIn );
   fseek( fpIn, kSectorSize, SEEK_SET );

   std::map< unsigned, unsigned > slotPages;

   while ( ftell( fpIn ) != fileSize )
   {
      long tagBegin = ftell( fpIn );

      unsigned numTags = 0;
      fread_assert( &numTags, sizeof( numTags ), 1, fpIn, "slot num tags" );
      
      DATACNF_TAG *pTags = new DATACNF_TAG[ numTags ];

      fread_assert( pTags, sizeof( *pTags ), numTags, fpIn, "slot tags" );

      long tagEnd = ftell( fpIn );

      fseek( fpIn, amount_until_next_sector( tagEnd- tagBegin ), SEEK_CUR );

      if ( commandLine.mVerbose )
         printf( "Tag page %8.8x - %d tags\n", tagBegin / kSectorSize, numTags );


      size_t totalSize = 0;
      for ( size_t tagIndex = 0; tagIndex < numTags; ++tagIndex )
      {
         if ( pTags[ tagIndex ].id == ID_SECTION_END )
         {
            totalSize += pTags[ tagIndex ].offset;
         }
//         printf( " %8.8x - %d\n", pTags[tagIndex].id, pTags[tagIndex].offset );
      }

      size_t beforeData = ftell( fpIn );

      char slotName[ 80 ];
      if( pFaceStageTable == NULL )
      {
         if ( commandLine.mWriteSlotPageFile )
         {
            sprintf( slotName, "%8.8x", tagBegin / kSectorSize );
         }
         else
         {
            std::map<uint32,std::string>::const_iterator found = slotPagesMapping.find( tagBegin / kSectorSize );

            if ( found == slotPagesMapping.end() )
            {
               printf( "Couldn't find mapping entry for slot %8.8x\n", tagBegin / kSectorSize );
               throw kAbortException;
            }

            strcpy( slotName, found->second.c_str() );
         }
      }
      else
      {
         sprintf( slotName, "%s", pFaceStageTable[iIdx].name );
      }


      SDumpAssetsState dumpState;
      SPrintInfoState printState;


      for ( size_t tagIndex = 0; tagIndex < numTags; ++tagIndex )
      {
         if ( commandLine.mVerbose )
         {
            print_info_for_tag( pTags[ tagIndex ].id, pTags[ tagIndex ].offset, &printState );
//            printf("Ftell: %d \n", ftell(fpIn) );
         }

         if ( commandLine.mOutputFolder )
         {
            WriteAssetsForSlot( fpIn, fpArchiveXml, commandLine.mOutputFolder, slotName, commandLine.mSdxFolder, pTags[tagIndex].id, pTags[tagIndex].offset, &dumpState );
         }

      }

      if( !fpArchiveXml )
      {
         WriteBPAssetsManifest( commandLine.mOutputFolder, slotName, &dumpState );
      }

      size_t const totalSizeRounded = round_up_sector( totalSize );

      if ( !commandLine.mOutputFolder )
      {
         fseek( fpIn, totalSize, SEEK_CUR );
      }

      if ( totalSizeRounded != totalSize )
      {
         fseek( fpIn, totalSizeRounded - totalSize, SEEK_CUR );
      }

      size_t afterData = ftell( fpIn );

      if ( beforeData + totalSizeRounded != afterData )
      {
         printf( "Data read mismatch...  bd:%d ts:%d bd+ts:%d, ad:%d\n",
            beforeData, totalSizeRounded, beforeData + totalSizeRounded, afterData );

         throw kAbortException;
      }

      unsigned fullPageId = ( ( ( afterData - tagBegin ) / kSectorSize ) << 20 ) | tagBegin / kSectorSize;

      slotPages.insert( std::pair<unsigned,unsigned>( tagBegin / kSectorSize, fullPageId ) );

      printf( "%8.8x - Offset %d, Size: %d\n",
         fullPageId,
         tagBegin, 
         afterData - tagBegin );

      delete [] pTags;

      iIdx++;

//      break;
   }

   // Flush the slot pages (sorted)
   if ( commandLine.mOutputFolder )
   {
      iIdx = 0;

      if ( commandLine.mWriteSlotPageFile || commandLine.mDataType == kDT_Face )
      {
         FILE *pagesFile = fopen( pagesPath, "wt" );

         if ( pagesFile == NULL ) 
         {
            printf( "Couldn't open %s for write!\n", pagesPath );
            throw kAbortException;
         }

         for ( std::map<unsigned,unsigned>::const_iterator it = slotPages.begin(); it != slotPages.end(); ++it )
         {
            if( pFaceStageTable != NULL )
            {
               fprintf( pagesFile, "%s ", pFaceStageTable[iIdx].name );
               iIdx++;
            }
            else
            {
               fprintf( pagesFile, "%8.8x ", it->first );
            }

            fprintf( pagesFile, "%8.8x\n", it->second );
         }
      }
   }
}

static void process_face_file( FILE *fp, FILE * fpArchiveXml, SCommandLine const &commandLine )
{
   //A Face data file has a table of contents in sector 0 like a stage file, however it is
   //uncompressed and unencrypted like a slot file. So this is an amalgam of the two methods.

   // Each DAT file has a header
   // This header's first member is the seed, the read of the header needs to be decrypted
   STAGE_TABLE_HEADER header = { 0 };

   fread( &header, sizeof( header ), 1, fp );

   printf( "Header:\n Version: %d\n Sector: %d\n StageNum: %d\n Padding: %d\n Size: %u\n",
      header.version, header.sector, header.stagenum, header.padding, header.install_size * kSectorSize );

   std::vector< STAGE_TABLE > stages;

   //we have to write out these slot addresses to a text file just like we do for a slot file. 
   std::map< unsigned, unsigned > slotPages;

   if ( header.version == 1 )
   {
      // Header's version seems to be the same for MGS2 and 3

      STAGE_TABLE *pStages = (STAGE_TABLE *) malloc( sizeof( STAGE_TABLE ) * header.stagenum );

      // Read in the stages
      fread( pStages, sizeof( STAGE_TABLE ), header.stagenum, fp );

      process_slot_file( fp, fpArchiveXml, commandLine, pStages );

      free( pStages );
      printf( "File location: %d\n", ftell( fp ) );

   }
   else
   {
      printf( "Unknown header!\n" );
   }

}

static void process_stage_file( FILE *fpIn, FILE * fpArchiveXml, SCommandLine const &commandLine )
{
   // Each DAT file has a header
   // This header's first member is the seed, the read of the header needs to be decrypted
   STAGE_TABLE_HEADER header = { 0 };

   fread( &header, sizeof( header ), 1, fpIn );

   KPCrypt::seed = header.seed;
   KPCrypt::_seed = KPCrypt::seed ^ 0xF0F0;

   KPCrypt::decrypt_buffer( (char *) (&header) + 4, sizeof( header ) - 4 );

   printf( "Header:\n Version: %d\n Sector: %d\n StageNum: %d\n Padding: %d\n Size: %u\n",
      header.version, header.sector, header.stagenum, header.padding, header.install_size * kSectorSize );

   std::vector< STAGE_TABLE > stages;

   if ( header.version == 1 )
   {
      // Header's version seems to be the same for MGS2 and 3

      STAGE_TABLE *pStages = (STAGE_TABLE *) malloc( sizeof( STAGE_TABLE ) * header.stagenum );

      // Read and decrypt the stages
      fread( pStages, sizeof( STAGE_TABLE ), header.stagenum, fpIn );
      KPCrypt::decrypt_buffer( pStages, sizeof( STAGE_TABLE ) * header.stagenum );


      FILE *stageOutputFile = NULL;
      {
         char stageOutputFilename[ FILENAME_MAX ];
         sprintf( stageOutputFilename, "%s/stages.txt", commandLine.mOutputFolder );
         stageOutputFile = fopen( stageOutputFilename, commandLine.mAppendStageList ? "at" : "wt" );
         if ( !stageOutputFile )
         {
            printf( "Couldn't open '%s' for write\n", stageOutputFilename );
            throw kAbortException;
         }
      }

      // Act on each stage
      for ( int stageNum = 0; stageNum < header.stagenum; ++stageNum )
      {
         // I believe that all stage names are null terminated in the files, but it can't hurt to 
         // make sure
         char stage_name[STAGE_NAME_LEN+2] = { 0 };
         memcpy( stage_name, pStages[ stageNum ].name, sizeof( pStages[ stageNum ].name ) );
      
         fprintf( stageOutputFile, "%s\n", stage_name );

         if ( !commandLine.mStageFilter || _stricmp( commandLine.mStageFilter, stage_name ) == 0 )
         {
            // TED - there are two identical copies of n_title in the US version of STAGE2.DAT of MGS2
            // This code skips over the second copy and will let us know if another such case occurs
            // in any of the other dat files.
            bool bSkip = false;
            for( int i=0; i < (int)stages.size(); ++i )
            {
               if( !strcmp( stages[i].name, stage_name ) )
               {
                  printf( "Warning: skipping additional instance of Stage: %d Name: %s, ofs: %u\n", stageNum, stage_name, pStages[stageNum].offset * kSectorSize );
                  bSkip = true;
               }
            }

            if( bSkip )
            {
               continue;
            }

            // There's no great reason to push the stages into a vector then refer to them there
            // I'm just doing it this way in case we want to decouple the stage list from the rest of the process
            printf( "Stage: %d Name: %s, ofs: %u\n", stageNum, stage_name, pStages[stageNum].offset * kSectorSize );
            stages.push_back( pStages[ stageNum ] );

            process_data_file_stage( commandLine.mOutputFolder, stages.back().name, commandLine.mSdxFolder, commandLine.mVerbose, stages.back().offset * kSectorSize, header.seed, fpIn, fpArchiveXml );
         }
      }

      if ( stageOutputFile )
      {
         fclose( stageOutputFile );
      }

      free( pStages );
      printf( "File location: %d\n", ftell( fpIn ) );
   }
   else
   {
      printf( "Unknown header!\n" );
   }
}

static void read_archive_input_file( SCommandLine * pCommandLine )
{
   if( !pCommandLine->mArchiveInputFilename )
   {
      printf("Error: Must specify archive input file with -a!\n" );
      throw false;
   }

   sDatFileInfo.reserve( 16 );

   FILE * fp = fopen( pCommandLine->mArchiveInputFilename, "rt" );
   if( fp )
   {
      char currLine[1024];
      for( int line = 0; fgets( currLine, 1023, fp ); ++line )
      {
         SDatFileInfo datFileInfo = { { 0 } };

         const char seps[]   = " ,\t\n";

         char * token = strtok( currLine, seps );
         if( *token == NULL )
         {
            printf("Error: empty column in layout input file line %d\n", line+1 );
            throw false;
         }
         strcpy( datFileInfo.mFilename, token );
         token = strtok( NULL, seps );
         if( *token == NULL )
         {
            printf("Error: empty column in layout input file line %d\n", line+1 );
            throw false;
         }
         strcpy( datFileInfo.mOutputFolder, token );

         token = strtok( NULL, seps );
         if( *token == NULL )
         {
            printf("Error: empty column in layout input file line %d\n", line+1 );
            throw false;
         }

         if( !strcmp( token, "stage" ) )
         {
            datFileInfo.mFileType = kDF_Stage;
         }
         else if( !strcmp( token, "face" ) )
         {
            datFileInfo.mFileType = kDF_Face;
         }
         else if( !strcmp( token, "slot" ) )
         {
            datFileInfo.mFileType = kDF_Slot;
         }
         else if( !strcmp( token, "dat" ) )
         {
            datFileInfo.mFileType = kDF_Dat;
         }
         else if( !strcmp( token, "dat_lst" ) )
         {
            datFileInfo.mFileType = kDF_DatLst;
         }
         else
         {
            printf("Unrecognized dat file type '%s', line %d\n", token, line+1 );
            throw false;
         }

         sDatFileInfo.push_back( datFileInfo );
      }
      fclose( fp );
   }
}

void parse_command_line( int argc, char const *argv[], SCommandLine *pCommandLine )
{
   char const *outputFolder = NULL;
   char const *inputName = NULL;
   char const *stageFilter = NULL; 
   char const *archiveFilename = NULL;
   char const *archiveInputFilename = NULL;
   char const *texDupeStatsFilename = NULL;
   char const *texFlatListFilename = NULL;
   char const *assetListFilename = NULL;
   char const *archivePlatform = NULL;
   char const *sdxFolder = NULL;

   for ( int arg = 1; arg < argc; /* inc in loop */ )
   {
      if ( !_stricmp( argv[arg], "-o" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-o must be followed by a path\n" );

            throw kUsageException;
         }
         else
         {
            outputFolder = argv[arg];
            ++arg;
         }
      }
      else if ( !_stricmp( argv[arg], "-s" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-s must be followed by a stage name\n" );

            throw kUsageException;
         }
         else
         {
            stageFilter = argv[arg];
            ++arg;
         }
      }
      else if ( !_stricmp( argv[arg], "-v" ) )
      {
         ++arg;
         pCommandLine->mVerbose = true;
         z_verbose = 1;
      }
      else if ( !_stricmp( argv[arg], "-slot" ) )
      {
         ++arg;
         pCommandLine->mDataType = kDT_Slot;
      }
      else if ( !_stricmp( argv[arg], "-writePages" ) )
      {
         ++arg;
         pCommandLine->mWriteSlotPageFile = true;
      }
      else if ( !_stricmp( argv[arg], "-appendStages" ) )
      {
         ++arg;
         pCommandLine->mAppendStageList = true;
      }
      else if( !_stricmp( argv[arg], "-face" ) )
      {
         ++arg;
         pCommandLine->mDataType = kDT_Face;
      }
      else if( !_stricmp( argv[arg], "-sdx" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-sdx must be followed by a folder name\n" );

            throw kUsageException;
         }
         else
         {
            sdxFolder = argv[arg];
            ++arg;
         }
      }
      else if ( !_stricmp( argv[arg], "-x" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-x must be followed by an archive filename\n" );

            throw kUsageException;
         }
         else
         {
            pCommandLine->mOperationMode = kOM_BuildArchiveXML;
            archiveFilename = argv[arg];
            ++arg;
         }
      }
      else if ( !_stricmp( argv[arg], "-a" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-a must be followed by an archive input filename\n" );

            throw kUsageException;
         }
         else
         {
            archiveInputFilename = argv[arg];
            ++arg;
         }
      }
      else if ( !_stricmp( argv[arg], "-t" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-t must be followed by an xml filename\n" );

            throw kUsageException;
         }
         else
         {
            texDupeStatsFilename = argv[arg];
            ++arg;
         }
      }
      else if ( !_stricmp( argv[arg], "-l" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-l must be followed by a texture flatlist filename\n" );

            throw kUsageException;
         }
         else
         {
            texFlatListFilename = argv[arg];
            ++arg;
         }
      }
      else if ( !_stricmp( argv[arg], "-la" ) )
      {
         ++arg;

         if ( arg == argc )
         {
            printf( "-la must be followed by a asset list filename\n" );

            throw kUsageException;
         }
         else
         {
            assetListFilename = argv[arg];
            ++arg;
         }
      }
      else if( !strncmp( argv[arg], "/p", 2 ) )
      {
         archivePlatform = argv[arg]+2;
         ++arg;
      }
      else
      {
         if ( inputName )
         {
            printf( "Input name was already %s - trying to change to %s\n", inputName, argv[ arg ] );
            throw kUsageException;
         }
         else
         {
            inputName = argv[ arg ];
            ++arg;
         }
      }
   }

   pCommandLine->mOutputFolder = outputFolder;
   pCommandLine->mInputName = inputName;
   pCommandLine->mStageFilter = stageFilter;
   pCommandLine->mArchiveFilename = archiveFilename;
   pCommandLine->mArchiveInputFilename = archiveInputFilename;
   pCommandLine->mTexDupeStatsFilename = texDupeStatsFilename;
   pCommandLine->mTexFlatListFilename = texFlatListFilename;
   pCommandLine->mAssetListFilename = assetListFilename;
   pCommandLine->mArchivePlatform = archivePlatform;
   pCommandLine->mSdxFolder = sdxFolder ? sdxFolder : "us";
}

void do_build_archive_xml( SCommandLine & commandLine )
{
   SetPlatformSubfolder( commandLine.mArchivePlatform );
   char archiveXmlFilename[FILENAME_MAX];
   sprintf( archiveXmlFilename, "%s.xml", commandLine.mArchiveFilename );
   FILE * fpArchiveXml = fopen( archiveXmlFilename, "wt" );
   if( !fpArchiveXml )
   {
      printf( "Archive xml file %s could not be opened. (read-only?)\n", archiveXmlFilename );
      throw kUsageException;
   }

   fprintf( fpArchiveXml, "<psarc>\n" );
   fprintf( fpArchiveXml, "\t<create archive=\"%s\" absolute=\"true\" overwrite=\"true\">\n", commandLine.mArchiveFilename );

   //First in archive: any index text files since they are loaded once at startup.
   for( int i=0; i < (int)sDatFileInfo.size(); ++i )
   {
      const SDatFileInfo & datFile = sDatFileInfo[i];
      char fullPath[FILENAME_MAX];

      switch( datFile.mFileType )
      {
      case kDF_Dat:
      case kDF_DatLst:
         {
            sprintf( fullPath, "%s/_bp/%s/bp_streams.txt", datFile.mOutputFolder, GetPlatformSubfolder() );
            add_file_to_archive_xml( fpArchiveXml, fullPath, fullPath );
         }
         break;
      case kDF_Slot:
         {
            //Explicitly add pages.txt
            sprintf( fullPath, "%s/%s", datFile.mOutputFolder, "pages.txt" );
            add_file_to_archive_xml( fpArchiveXml, fullPath, fullPath );
         }
         break;
      case kDF_Face:
         {
            //Explicitly add pages.txt
            sprintf( fullPath, "%s/%s", datFile.mOutputFolder, "pages.txt" );
            add_file_to_archive_xml( fpArchiveXml, fullPath, fullPath );
         }
         break;
      }
   }

   //TODO: any explicitly packed files (asset precache, etc.) go here.

   //Iterate over list of DAT files, including them in the archive in the appropriate way.
   for( int i=0; i < (int)sDatFileInfo.size(); ++i )
   {
      const SDatFileInfo & datFile = sDatFileInfo[i];

      switch( datFile.mFileType )
      {
      case kDF_Dat:
         {
            add_dat_file_to_archive_xml( fpArchiveXml, datFile.mOutputFolder );
         }
         break;
      case kDF_DatLst:
         //don't archive contents of this dat file.
         break;
      case kDF_Slot:
         {
            commandLine.mInputName = datFile.mFilename;
            FILE *fpIn = fopen( commandLine.mInputName, "rb" );

            if ( !fpIn )
            {
               printf( "Can't open %s\n", commandLine.mInputName );
               throw kUsageException;
            }

            commandLine.mOutputFolder = datFile.mOutputFolder;
            process_slot_file( fpIn, fpArchiveXml, commandLine, NULL );

            fclose( fpIn );
         }
         break;
      case kDF_Stage:
         {
            commandLine.mInputName = datFile.mFilename;
            FILE *fpIn = fopen( commandLine.mInputName, "rb" );

            if ( !fpIn )
            {
               printf( "Can't open %s\n", commandLine.mInputName );
               throw kUsageException;
            }

            commandLine.mOutputFolder = datFile.mOutputFolder;
            process_stage_file( fpIn, fpArchiveXml, commandLine );

            fclose( fpIn );
         }
         break;
      case kDF_Face:
         {
            commandLine.mInputName = datFile.mFilename;
            FILE *fpIn = fopen( commandLine.mInputName, "rb" );

            if ( !fpIn )
            {
               printf( "Can't open %s\n", commandLine.mInputName );
               throw kUsageException;
            }

            commandLine.mOutputFolder = datFile.mOutputFolder;
            process_face_file( fpIn, fpArchiveXml, commandLine );

            fclose( fpIn );
         }
         break;
      default:
         {
            printf("Unhandled dat type in archive xml creation!\n");
            throw false;
         }
      }
   }
   fprintf( fpArchiveXml, "\t</create>\n" );
   fprintf( fpArchiveXml, "</psarc>\n" );
   fclose( fpArchiveXml );
}

int main( int argc, char const *argv[] )
{
   try
   {
      SCommandLine commandLine;
      parse_command_line( argc, argv, &commandLine );
      InitTriInfo( commandLine.mTexFlatListFilename );
      InitAssetRemap( commandLine.mAssetListFilename );

      if ( commandLine.mOperationMode == kOM_Extract && commandLine.mInputName == NULL )
      {
         throw kUsageException;
      }

      if( commandLine.mOperationMode == kOM_Extract )
      {
         FILE *fp = fopen( commandLine.mInputName, "rb" );

         if ( !fp )
         {
            printf( "Can't open %s\n", commandLine.mInputName );
            throw kUsageException;
         }

         if ( commandLine.mOutputFolder )
         {
            // Make sure that the output folder both exists and is a directory
            DWORD attr = GetFileAttributesA( commandLine.mOutputFolder );

            if ( attr == INVALID_FILE_ATTRIBUTES || !( attr & FILE_ATTRIBUTE_DIRECTORY ) )
            {
               printf( "Folder %s doesn't exist or isn't a folder\n", commandLine.mOutputFolder );
               throw kUsageException;
            }
         }

         if ( commandLine.mStageFilter )
         {
            printf( "Only working with stage: %s\n", commandLine.mStageFilter );
         }

         switch ( commandLine.mDataType )
         {
         case kDT_Stage:
            process_stage_file( fp, NULL, commandLine );
            break;
         case kDT_Slot:
            process_slot_file( fp, NULL, commandLine, NULL );
            break;
         case kDT_Face:
            process_face_file( fp, NULL, commandLine );
         }

         fclose( fp );
      }
      else if( commandLine.mOperationMode == kOM_BuildArchiveXML )
      {
         read_archive_input_file( &commandLine );
         if( !commandLine.mArchivePlatform )
         {
            printf("Error: must specify platform subfolder if building an archive xml.\n");
            throw kUsageException;
         }
         do_build_archive_xml( commandLine );
      }

      if( commandLine.mTexDupeStatsFilename )
      {
         DumpTriStats( commandLine.mTexDupeStatsFilename );
      }

      return 0;
   }
#ifdef NDEBUG
   catch ( EAbortException )
   {
      fflush( stderr );
      fflush( stdout );

      return 1;
   }
#endif
   catch (EUsageException)
   {
      PrintUsage();

      return 1;
   }
}
