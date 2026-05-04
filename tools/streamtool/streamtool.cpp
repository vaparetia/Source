//----------------------------------------------------------------------------

#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "streamtool.h"

#if MGS_VERSION==2
#include "CStreamDriver_PCM.h"
#elif MGS_VERSION==3
#include "CStreamDriver_MTA.h"
#include "CStreamDriver_STV.h"
#endif

#include "CStreamDriver_M2V.h"
#include "CStreamDriver_CAP.h"
#include "CStreamDriver_Demo.h"
#include "CStreamDriver_IPU.h"
#include "CStreamDriver_RawExtract.h"

#include "CStreamWriter_CAP.h"
#include "CStreamWriter_RadioCAP.h"
#include "CStreamWriter_Movies.h"
#include "CStreamWriter_XAudio.h"
#include "CStreamWriter_MSF.h"
#include "CStreamWriter_NULL.h"
#include "CStreamWriter_PCM.h"
#include "CStreamWriter_Vita_AT9.h"

#include "CStreamWriter_MergeSOL.h"

#define BP_SPLIT_STREAM_EXTENSION ".sdt"

// uncomment this to enable logging of outgoing packet data.
//#define DUMP_PACKET_LOG

//----------------------------------------------------------------------------

enum EUsageMode
{
   kUsageMode_ExtractSourceData,
   kUsageMode_ExtractStream,
   kUsageMode_DumpInfo,
   kUsageMode_GenerateListFile,
   kUsageMode_RebuildStream,
   kUsageMode_BuildStreamTables,
   kUsageMode_MergeStreamSOL,
   kUsageMode_ForceChangeFrameRate,

   kUsageMode_Invalid = -1
};

enum ERegion
{
   kRegion_US,
   kRegion_JP,
   kRegion_EU
};

enum EInputFileType
{
   kInputFileType_EntireDat,
   kInputFileType_SingleStream,
};

const char * const skPlatformExtension[] =
{
   "_bp/",
   "_bp/_ps3/",
   "_bp/_360/",
   "_bp/_vta/"
};

const char * const skUsRegionFolder = "us/";
const char * const skJpSOLRegionFolder = "jp_sol/";

enum { kUnknownStreamOffset = 0xffffffff };
enum { kStreamPacketAlign = 16 };

struct SCommandLineArgs
{
   char           mMainInputFilename[FILENAME_MAX];
   const char *   mSrcListFileName;
   EUsageMode     mUsageMode;
   const char *   mProcessStreamName;
   int            mProcessStreamIndex;
   ERegion        mRegion;
   EPlatform      mPlatform;
   EInputFileType mInputFileType;
   bool           mbCodecData;
   bool           mbBgmData;
   bool           mbVoxData;
   bool           mbInputFileIsList;
   bool           mbRemapJapaneseCodecNames;
   bool           mbDumpCaptionText;
   int            mDumpLanguage;
   bool           mbExportRadioCaptions;
};

typedef std::pair< unsigned int, std::string > TOffsetNamePair;
typedef std::map< unsigned int, CStreamDriver * >  TStreamDriverMap;
typedef std::vector< CStreamWriter * >  TStreamWriterVector;
typedef std::list< unsigned int > TPacketTypeList;

//----------------------------------------------------------------------------

SCommandLineArgs gCommandLineArgs = { 0 };
std::vector< TOffsetNamePair > gStreamOffsetsNames;
std::vector< TOffsetNamePair > gCodecOffsetsNamesRemapSrc;
std::vector< TOffsetNamePair > gCodecOffsetsNamesRemapDst;
TStreamDriverMap gStreamDriverMap;
TStreamWriterVector gStreamWriterVector;
CStreamWriter_MergeSOL * gpStreamWriter_MergeSOL = NULL;
TPacketTypeList gPacketTypeFilter;
int gInfoModeMask;

char gDatIndexFilename[FILENAME_MAX] = "";
char gOutputFolder[FILENAME_MAX] = "";
char gCurrInputFilename[FILENAME_MAX] = "";
char gCurrStreamName[FILENAME_MAX] = "";
const char * gRegionString = "";

//----------------------------------------------------------------------------

struct SIffChunkHeader
{
   char  mId[4];
   int   mSize;
};

struct SRIffChunk
{
   SIffChunkHeader mHeader;
   char  mWavHeader[4];
};

struct SFmtChunk
{
   SIffChunkHeader   mHeader;
   short             mFormatTag;
   unsigned short    mChannelCount;
   unsigned int      mFrequency;
   unsigned int      mAvgBytesPerSec;
   unsigned short    mBlockAlign;
   unsigned short    mBitsPerSample;
};

void SaveLPCM16WavFile( const char * const filename, const unsigned int channelCount, const unsigned int frequency, const unsigned int dataSize, const unsigned char * const data )
{
   FILE * fp = fopen( filename, "wb" );
   if( !fp )
   {
      printf("Error opening %s for writing!\n", filename );
      throw false;
   }

   printf("Extracting %s.\n", filename );
   const int bytesPerSample = sizeof(short);

   SRIffChunk riffChunk = { { {'R','I','F','F' }, dataSize + 36 }, {'W','A','V','E'} };
   fwrite( &riffChunk, sizeof( riffChunk ), 1, fp );

   SFmtChunk fmtChunk = { { {'f','m','t',' '}, sizeof(SFmtChunk)-sizeof(SIffChunkHeader) }, 1, (unsigned short)channelCount, frequency, frequency * channelCount * bytesPerSample, (unsigned short)(channelCount * bytesPerSample), (unsigned short)(bytesPerSample*8) };
   fwrite( &fmtChunk, sizeof( fmtChunk ), 1, fp );

   SIffChunkHeader dataChunk = { {'d','a','t','a'}, dataSize };
   fwrite( &dataChunk, sizeof( dataChunk ), 1, fp );

   fwrite( data, dataSize, 1, fp );

   fclose( fp );
}

//----------------------------------------------------------------------------

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

bool BP_FileIsReadOnly( const char * const path )
{
   const unsigned int fileAttributes = GetFileAttributesA( path );
   if ( (fileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
   {
      return 1;
   }

   return 0;
}

void BP_UnifyPath( char * path )
{
   int totalLen = 0;
   while( path[totalLen] != NULL )
   {
      if( path[totalLen] == '\\' )
      {
         path[totalLen] = '/';
      }
      ++totalLen;
   }
}

void BP_ReplaceStreamSourceDataRegion( char * outFullPath, const char * const originalInputPath, const char * const newRegion )
{
   char inputPath[1024];
   strcpy(inputPath, originalInputPath);

   const char * const lastSlash = strrchr( inputPath, '/' );
   const char * streamfolder;
   const char * datfolder;
   const char * srcRegion;
   for( streamfolder = lastSlash-1; *streamfolder != '/'; --streamfolder )
   {
      if( streamfolder == inputPath )
      {
         printf("Error: no stream folder!\n" );
         throw false;
      }
   }
   for( datfolder = streamfolder-1; *datfolder != '/'; --datfolder )
   {
      if( datfolder == inputPath )
      {
         printf("Error: no dat folder!\n" );
         throw false;
      }
   }
   for( srcRegion = datfolder-1; (srcRegion != inputPath) && (*srcRegion != '/'); --srcRegion )
   {
   }
   strncpy( outFullPath, inputPath, srcRegion-inputPath );
   strcpy( outFullPath+(srcRegion-inputPath), newRegion );
   strcat( outFullPath, datfolder );
}

const char * BP_GetPlatformSubfolder( const EPlatform platform )
{
   return skPlatformExtension[platform];
}

//----------------------------------------------------------------------------
std::string BP_ConvertManagedString( System::String^ value )
{
   System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(value);

   char const* str = (char const*)(void*) ptr;

   std::string ret( str );

   System::Runtime::InteropServices::Marshal::FreeHGlobal( ptr );

   return ret;
}

static std::string compute_md5_hash( const unsigned char * const buf, const int length )
{
   array<unsigned char>^ stringBytes = gcnew array<unsigned char>( length );
   System::Runtime::InteropServices::Marshal::Copy( (System::IntPtr)(void*)buf, stringBytes, 0, length );

   array<unsigned char>^ md5Hash = (gcnew System::Security::Cryptography::SHA1CryptoServiceProvider())->ComputeHash( stringBytes );
   System::String^ hashString = gcnew System::String("");
   for( int i=0; i < md5Hash->Length; ++i )
   {
      hashString += ( gcnew System::Byte( md5Hash[i] ) )->ToString( "X2" );
   }

   return BP_ConvertManagedString( hashString );
}

static void CreateFullSubdirectories( const char * const fn )
{
   System::String^ filename = gcnew System::String( fn );
   if( !filename->EndsWith( "/" ) )
   {
      filename += "/";
   }
   filename->Replace( '\\', '//' );
   int slashPos = 0;
   while( (slashPos = filename->IndexOf( "/", slashPos )) != -1 )
   {
      System::String^ folder = filename->Substring( 0, slashPos );
      System::IO::Directory::CreateDirectory( folder );
      ++slashPos;
   }
}

//----------------------------------------------------------------------------

static void usage()
{
   printf("streamtool.exe: stream .DAT file manipulator for MGS%d.\n", MGS_VERSION );
   printf("usage: streamtool.exe [-x|i|l|b|s|t|m] [/o<output folder>][/r<us|jp|eu>] [/s<stream name>]\n");
   printf("       [/l<lst filename>] [/i<packet type filter>] [/p[]] [/j] [/d<n>] <dat or sdt filename>\n");
   printf("-i   : dump packets list.\n");
   printf("-x   : extract source data from stream / dat.\n");
   printf("-l   : generate bluepoint list / index file.\n");
   printf("-b   : rebuild <stream name>" BP_SPLIT_STREAM_EXTENSION " file(s), incorporating any replacement audio / movie / etc. data present.\n");
   printf("-s   : extract source stream file(s) from .dat file.\n");
   printf("-t   : build stream table text files for game to use.\n");
   printf("-m   : merge us/jp_sol streams into jp substance streams.\n");
   printf("/s   : limit processing to specified stream.\n");
   printf("/l   : specify source PS2 .lst file for dat file. (required for -l option)\n");
   printf("/o   : specify output folder.\n");
   printf("/r   : specify region of .dat file.\n");
   printf("/p   : specify platform for rebuilt <stream name>" BP_SPLIT_STREAM_EXTENSION " file.\n");
   printf("/j   : force name remapping of codec script names (from Japanese)\n");
   printf("/i   : specify packet type filter for info / extraction (e.g. /i00000004,00010004,00020004,00030004,00040004,00050004\n");
   printf("/d<n>: dump converted caption text of specified language [0...7] into a csv file while rebuilding..\n");

   printf("\nSupported platforms:\n");
   printf("_ps3 : Playstation3\n");
   printf("_360: XBox360\n");
}

//copied from vr.lss
static const char * const skHackVrVoxNames[] =
{
   "vc030201",
   "vc030202",
   "vc030203",
   "vc030211",
   "vc030212",
   "vc030213",
   "vc030214",
   "vc030215",
   "vc030216",
   "vc030217",
   "vc030218",
   "vc030241",
   "vc030242",
   "vc030243",
   "vc030244",
   "vc030231",
   "vc030232",
   "vc030233",
   "vc030234",
   "vc030235",
   "vc030236",
   "vc030237",
   "vc030238",
   "vc030221",
   "vc030222",
   "vc030223",
   "vc030225",
   "vc032011",
   "vc032012",
   "vc032013",
   "vc032014",
   "vc032015",
   "vc032016",
   "vc128001",
   "vc128002",
   "vc128003",
   "vc128004",
   "vc128005",
   "vc128006",
   "vc128007",
   "vc128008",
   "vc128009",
   "vc128010",
   "vc128011",
   "vc128012",
   "vc128013",
   "vc128014",
   "vc128015",
   "vc128016",
   "vc128017",
   "vc031001",
   "vc031003",
   "vc031002",
   "vc032001",
   "vc046000",
   "vc046003",
   "vc046004",
   "vc046005",
   "vc046006",
   "vc046601",
   "vc046062",
   "vc046493",
   "vc046063",
   "vc046072",
   "vc046065",
   "vc046066",
   "vc046067",
   "vc046100",
   "vc046484",
   "vc046008",
   "vc046602",
   "vc04f000",
   "vc04f001",
   "vc04f003",
   "vc04f004",
   "vc04f007",
   "vc04f00a",
   "vc04f00b",
   "vc04f00d",
   "vc04f00f",
   "vc04f010",
   "vc04f011",
   "vc04f013",
   "vc04f014",
   "vc04f017",
   "vc04f01e",
   "vc04f01b",
   "vc04f01d",
//   "vc046000", dupe!
   "vc04f029",
   "vc04f101",
   "vc04f102",
   "vc04f103",
   "vc04f104",
   "vc04f105",
   "vc04e106",
   "vc04f107",
   "vc04f108",
   "vc04f109",
   "vc04f110",
   "vc04f111",
   "vc04f112",
   "vc04f113",
   "vc04f114",
   "vc04f115",
   "vc04f116",
   "vc04f117",
   "vc04f119",
//   "vc04f117", dupe!
//   "vc04f119", dupe!
   "vc04f121",
   "vc04f124",
   "vc04f122",
   "vc04f125",
   "vc04f128",
   "vc04f131",
   "vc04f140",
   "vc04f144",
   "vc04f148",
   "vc04f151",
   "vc04f203",
   "vc04f015",
   "vc04e036",
   "vc04e037",
   "vc04e038",
   "vc04f134",
   "vc04f137"
};

static void do_load_list_file( const char * const filename, std::vector< TOffsetNamePair > & streamOffsetsNames )
{
   FILE * streamfp = fopen( filename, "rb" );
   if( !streamfp )
   {
      printf("Error: could not open list file %s.\n", filename );
      throw false;
   }

   char currLine[512];

   for( int line = 0; fgets( currLine, sizeof(currLine)-1, streamfp ); ++line )
   {
      unsigned int offset;
      char currName[512];
      if( 2 != sscanf( currLine, "%d %s", &offset, currName ) )
      {
         break;
      }

      streamOffsetsNames.push_back( TOffsetNamePair( offset, currName ) );
   }
   fclose( streamfp );
}

static void load_list_file()
{
   gStreamOffsetsNames.reserve( 4096 );

   if( gCommandLineArgs.mUsageMode == kUsageMode_GenerateListFile )
   {
      FILE * streamfp = fopen( gCommandLineArgs.mSrcListFileName, "rb" );
      if( !streamfp )
      {
         printf("Error: could not open list file %s.\n", gCommandLineArgs.mSrcListFileName );
         throw false;
      }

      char currLine[512];

      int accum_offset_hack = 0;

      for( int line = 0; fgets( currLine, sizeof(currLine)-1, streamfp ); ++line )
      {
         unsigned int value;
         char currName[512];
         if( 2 != sscanf( currLine, "0x%x %s", &value, currName ) )
         {
            break;
         }
         if( strstr( currName, "_len" ) )
         {
            //skip over length entries
            continue;
         }

         unsigned int offset;

         if( gCommandLineArgs.mbCodecData )
         {
#if MGS_VERSION==3
            if( line >= 2220 && line < 2267 )
            {
               //cbf_* codec scripts off by 112 bytes
               accum_offset_hack += 112;
            }
            else if( line == 2267 )
            {
               //cbf_14003_**** not included in shipped data due to FIX_MODE (see bottom of fire_support.cdc) (it's a debug function)
               //            printf("Skip %s\n", currName);
               accum_offset_hack += 2304;
               continue;
            }
            else if( line >= 2273 && line < 2320 )
            {
               //cbf_* codec scripts off by 112 bytes
               accum_offset_hack += 112;
            }
            else if( line == 2320 )
            {
               //cbf_14002_**** not included in shipped data due to FIX_MODE (see bottome of return_to_base.cdc) (it's a debug function)
               //            printf("Skip %s\n", currName);
               accum_offset_hack += 2832;
               continue;
            }
#endif
            //codec.lst contains items in a different format
            //first byte is the size (encoded), which we ignore
            offset = ( value & 0x00FFFFFF ) << 4;
            offset -= accum_offset_hack;
         }
         else
         {
            //stream dat file.
            offset = ( value & ~0x10000000 ) * SECTOR_SIZE;
            offset -= accum_offset_hack;
         }

         gStreamOffsetsNames.push_back( TOffsetNamePair( offset, currName ) );

#if MGS_VERSION==2
         if( gCommandLineArgs.mbVoxData && gStreamOffsetsNames.size() == 1503 && gCommandLineArgs.mRegion != kRegion_JP )
         {
            //vox.lst file we've got is missing 124 entries in the middle.
            //They are vr mission voice samples that are duplicated in vox2.dat,
            //listed in vr.lss.
            int voxCount = sizeof(skHackVrVoxNames)/sizeof(*skHackVrVoxNames);
            for( int i=0; i < voxCount; ++i )
            {
               gStreamOffsetsNames.push_back( TOffsetNamePair( (unsigned int)kUnknownStreamOffset, skHackVrVoxNames[i] ) );
            }

            accum_offset_hack += -41818112;
         }
#elif MGS_VERSION==3
         if( gCommandLineArgs.mbBgmData && gStreamOffsetsNames.size() > 0 && gStreamOffsetsNames.back().second == "title_bgm" )
         {
            //Hack to fix inconsistency b/t bgm.lst and BGM.DAT of usa data due to title_bgm changing in size
            accum_offset_hack += 0x11D * SECTOR_SIZE;
         }
#endif
      }
      fclose( streamfp );
   }
   else
   {
      //Assume that the bluepoint-format fully populated and corrected list already exists and load from that instead.
      do_load_list_file( gDatIndexFilename, gStreamOffsetsNames );
   }

   gCommandLineArgs.mProcessStreamIndex = -1;

   if( gCommandLineArgs.mProcessStreamName )
   {
      //Find the stream index this corresponds to.
      for( int i=0; i < (int)gStreamOffsetsNames.size(); ++i )
      {
         if( !strcmp( gStreamOffsetsNames[i].second.c_str(), gCommandLineArgs.mProcessStreamName ) )
         {
            gCommandLineArgs.mProcessStreamIndex = i;
            break;
         }
      }

      if( gCommandLineArgs.mProcessStreamIndex == -1 )
      {
         printf("Error: stream '%s' does not exist in %s.\n", gCommandLineArgs.mProcessStreamName, gCurrInputFilename );
         throw false;
      }
   }
}

static void stream_info( FILE * streamfp )
{
   char fullpath[FILENAME_MAX];
   sprintf( fullpath, "%s/%s/packets%s.txt", gOutputFolder, gCurrStreamName, gPacketTypeFilter.size() ? ".filtered" : "" );

   FILE * packetsfp = fopen( fullpath, "wt" );

   if( !packetsfp )
   {
      printf("Error: could not open %s\n", fullpath );
      throw false;
   }

   STREAM_TAG streamTag;
   std::vector< unsigned char > packetBuf( 16 * 1024, 0 );

   unsigned int at = ftell( streamfp );

   printf("Dumping info for %s.\n", gCurrStreamName );
   do 
   {
      int n = fread( &streamTag, sizeof( streamTag ), 1, streamfp );
      if( n != 1 )
      {
         printf("Error reading %s\n", gCurrInputFilename );
         throw false;
      }

      packetBuf.assign( streamTag._size, 0 );
      memcpy( &packetBuf[0], &streamTag, sizeof( streamTag ) );

      const int bodySize = streamTag._size - sizeof( streamTag );
      if( bodySize )
      {
         fread( &packetBuf[sizeof(streamTag)], sizeof(unsigned char), bodySize, streamfp );
      }

      for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
      {
         CStreamWriter * const pStreamWriter = *it;
         pStreamWriter->WriteOutInfo( streamTag );
      }

      //N.B. only perform hash on body proper, as we already directly print out differences in the header.
      std::string hashString = bodySize ? compute_md5_hash( &packetBuf[sizeof(streamTag)], bodySize ) : std::string("0000000000000000000000000000000000000000");
      if( !gPacketTypeFilter.size() )
      {
         fprintf( packetsfp, "HASH %s TYPE %08x SIZE %6d TIME %6d OPTION %08x\n", hashString.c_str(), streamTag._type, streamTag._size, streamTag._time, streamTag._option );
      }
      else
      {
         if( std::find( gPacketTypeFilter.begin(), gPacketTypeFilter.end(), streamTag._type ) != gPacketTypeFilter.end() )
         {
            //Leave out the time in this report as it will make packets look unneccessarily different.
            fprintf( packetsfp, "HASH %s TYPE %08x SIZE %6d OPTION %08x\n", hashString.c_str(), streamTag._type, streamTag._size, streamTag._option );
         }
      }

      at = ftell( streamfp );
   } while ( streamTag._type != CHUNK_TYPE_END );
   at = ( at + SECTOR_SIZE - 1 ) & (~(SECTOR_SIZE-1) );
   fseek( streamfp, at, SEEK_SET );

   fclose( packetsfp );

   for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
   {
      CStreamWriter * const pStreamWriter = *it;
      pStreamWriter->FinishInfo();
   }
}

static void stream_skip( FILE * streamfp )
{
   STREAM_TAG streamTag;

   unsigned int at = ftell( streamfp );

   do 
   {
      int n = fread( &streamTag, sizeof( streamTag ), 1, streamfp );
      if( n != 1 )
      {
         printf("Error reading %s\n", gCurrInputFilename );
         throw false;
      }

      if( int bodySize = streamTag._size - sizeof( streamTag ) )
      {
         fseek( streamfp, bodySize, SEEK_CUR );
      }

      at = ftell( streamfp );
   } while ( streamTag._type != CHUNK_TYPE_END );
   at = ( at + SECTOR_SIZE - 1 ) & (~(SECTOR_SIZE-1) );
   fseek( streamfp, at, SEEK_SET );
}

static void stream_extract_sourcedata( FILE * streamfp )
{
   STREAM_TAG streamTag;
   std::vector< unsigned char > packetBuf( 16 * 1024, 0 );

   unsigned int at = ftell( streamfp );

   printf("Extracting data from %s.\n", gCurrStreamName );

   do
   {
      int n = fread( &streamTag, sizeof( streamTag ), 1, streamfp );
      if( n != 1 )
      {
         printf("Error reading %s\n", gCurrInputFilename );
         throw false;
      }

      packetBuf.assign( streamTag._size, 0 );
      memcpy( &packetBuf[0], &streamTag, sizeof( streamTag ) );

      if( int bodySize = streamTag._size - sizeof( streamTag ) )
      {
         fread( &packetBuf[sizeof(streamTag)], sizeof(unsigned char), bodySize, streamfp );
      }

      TStreamDriverMap::iterator found = gStreamDriverMap.find( streamTag._type );
      if( found == gStreamDriverMap.end() )
      {
         //No exact match of type+subtype.  See if there's a match of just type (when subtype is masked off).
         found = gStreamDriverMap.find( streamTag._type & 0xffff );
      }

      if( found != gStreamDriverMap.end() )
      {
         CStreamDriver * const pStreamDriver = found->second;
         const void * const pData = &packetBuf[sizeof(streamTag)];
         pStreamDriver->ProcessPacket( streamTag, pData );
      }

      at = ftell( streamfp );
   } while ( streamTag._type != CHUNK_TYPE_END );

   for( TStreamDriverMap::iterator it = gStreamDriverMap.begin(); it != gStreamDriverMap.end(); ++it )
   {
      CStreamDriver * const pStreamDriver = it->second;
      pStreamDriver->EndStream();
   }

   at = ( at + SECTOR_SIZE - 1 ) & (~(SECTOR_SIZE-1) );
   fseek( streamfp, at, SEEK_SET );
}

static void stream_rebuild_stream( FILE * instreamfp, const EPlatform currentPlatform )
{
   //Only actually rebuild the stream if there's at least one writer that has replacement data for it.
   bool bRebuildStream = false;
   const unsigned int buildingPlatformMask = 1 << currentPlatform;
   for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
   {
      CStreamWriter * const pStreamWriter = *it;
      const unsigned int writerPlatformMask = pStreamWriter->GetRebuildPlatformMask();
      if( writerPlatformMask & buildingPlatformMask )
      {
         //This writer type can write data for this platform.
         if( pStreamWriter->ReplacementFileExistsForCurrStream(currentPlatform) )
         {
            //This writer type actually has replacement data for this particular stream.
            bRebuildStream = true;
            
            // Don't break here in order to allow all writers to execute their replacement function. At the very least
            // this provides the writer the ability to display a message to the console..
         }
      }
   }

   FILE * outstreamfp = NULL;
   char outputFileName[FILENAME_MAX];

#ifdef DUMP_PACKET_LOG
   FILE * logfp = NULL;
#endif

   if( bRebuildStream )
   {
      //Ensure that the base _bp rebuild folder exists
      sprintf( outputFileName, "%s/%s", gOutputFolder, skPlatformExtension[kPlatform_Base] );
      _mkdir( outputFileName );
      //Make the folder for the platform, if not any
      sprintf( outputFileName, "%s/%s", gOutputFolder, skPlatformExtension[currentPlatform] );
      _mkdir( outputFileName );
      strcat( outputFileName, gCurrStreamName );
      strcat( outputFileName, BP_SPLIT_STREAM_EXTENSION );

      // Check to see if output filename is readonly
      if (BP_FileExists(outputFileName) && BP_FileIsReadOnly(outputFileName))
      {
         // Readonly file
         printf("Warning: skipping READONLY file: %s\n", outputFileName );
         return;
      }

      outstreamfp = fopen( outputFileName, "w+b" );
      if( !outstreamfp )
      {
         printf("Error: could not open %s for writing.\n", outputFileName );
         throw false;
      }
      printf("Building %s.\n", outputFileName );
      

#ifdef DUMP_PACKET_LOG
      char logNameBuf[ 4096 ];
      if ( ( strlen( outputFileName ) + 4 ) < sizeof( logNameBuf ) )
      {
         strcpy( logNameBuf, outputFileName );
         strcat( logNameBuf, ".log" );
         logfp = fopen( logNameBuf, "wb" );
      }
#endif
   }

   STREAM_TAG streamTag;
   std::vector< unsigned char > packetBuf( 16 * 1024, 0 );

   unsigned int inat = ftell( instreamfp );

   int currTick = 0;

   do
   {
      //Read the next packet.
      int n = fread( &streamTag, sizeof( streamTag ), 1, instreamfp );
      if( n != 1 )
      {
         printf("Error reading %s\n", gCurrInputFilename );
         throw false;
      }

      packetBuf.assign( streamTag._size, 0 );
      memcpy( &packetBuf[0], &streamTag, sizeof( streamTag ) );

      if( int bodySize = streamTag._size - sizeof( streamTag ) )
      {
         fread( &packetBuf[sizeof(streamTag)], sizeof(unsigned char), bodySize, instreamfp );
      }

      if( outstreamfp )
      {
         for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
         {
            CStreamWriter * const pStreamWriter = *it;
            pStreamWriter->ModifyCurrentSourcePacket( streamTag, &packetBuf[0], currentPlatform );
            memcpy( &packetBuf[0], &streamTag, sizeof( streamTag ) );
         }

         //Before the current packet is written, allow all stream writers to insert as much data as they want
         //based on the current stream tick.
         bool moreToWrite;
         do
         {
            //Write up to one packet at a time from each of the potential writing streams.
            unsigned int preAt = ftell( outstreamfp );
            for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
            {
               CStreamWriter * const pStreamWriter = *it;
               pStreamWriter->WriteOnePacket( outstreamfp, currentPlatform, currTick );
            }
            unsigned int postAt = ftell( outstreamfp );
            if ((postAt & (kStreamPacketAlign-1)) != 0)
            {
               printf("Error writing new data. Data is not aligned to 4 bytes.\n");
               throw false;
            }

#ifdef DUMP_PACKET_LOG
            // get the packet information.
            int packetSize = postAt - preAt;
            if ( packetSize > 0 )
            {
               // advance to the start of the packet
               fseek( outstreamfp, preAt, SEEK_SET );

               // read the tag.
               STREAM_TAG tag;
               memset( &tag, 0, sizeof( tag ) );
               fread( &tag, sizeof( STREAM_TAG ), 1, outstreamfp );

               // print the tag to the output log.
               if ( logfp )
               {
                  fprintf( logfp, "Type: %.8X    Size: %.8X    Time:    %.8X    Option:    %.8X\n",
                     tag._type, tag._size, tag._time, tag._option );
               }

               // read the packet header.
               fseek( outstreamfp, postAt, SEEK_SET );
            }
#endif

            moreToWrite = ( postAt != preAt );
         }
         while( moreToWrite );
      }

      if( streamTag._time > currTick )
         currTick = streamTag._time;

      //If there is no stream writer converting or stripping this type of stream,
      //just pass it through to the rebuilt .dat file.
      //Otherwise throw out the packet.
      int discardType = streamTag._type;
      if( discardType == CHUNK_TYPE_SYSTEM )
      {
         //Make sure we throw out the system tag that starts up the original stream driver for this type.
         discardType = streamTag._option;
      }

      if( outstreamfp )
      {
         bool bDiscard = false;
         for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
         {
            CStreamWriter * const pStreamWriter = *it;
            if( pStreamWriter->ReplacesDataForCurrStream(discardType) )
            {
               bDiscard = true;
            }
         }

         if (streamTag._type == CHUNK_TYPE_END)
         {
            for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
            {
               CStreamWriter * const pStreamWriter = *it;
               pStreamWriter->EndStream( outstreamfp, currentPlatform, &streamTag);
            }

            unsigned int postAt = ftell( outstreamfp );
            if ((postAt & (kStreamPacketAlign-1)) != 0)
            {
               printf("Error writing new data. Data is not aligned to 4 bytes.\n");
               throw false;
            }
         }

         if( !bDiscard )
         {
#ifdef DUMP_PACKET_LOG
            // get the stream tag.
            STREAM_TAG* tag = ( STREAM_TAG* )&packetBuf[0];

            // print the tag to the output log.
            if ( logfp )
            {
               fprintf( logfp, "Type: %.8X    Size: %.8X    Time:    %.8X    Option:    %.8X\n",
                  tag->_type, tag->_size, tag->_time, tag->_option );
            }
#endif
            STREAM_TAG* pTag = (STREAM_TAG*)&packetBuf[0];
            if (streamTag._type == CHUNK_TYPE_END)
               fwrite( &streamTag, sizeof(streamTag), 1, outstreamfp );
            else
               fwrite(&packetBuf[0], streamTag._size, 1, outstreamfp);
         }
      }

      inat = ftell( instreamfp );

   } while ( streamTag._type != CHUNK_TYPE_END );

   inat = ( inat + SECTOR_SIZE - 1 ) & (~(SECTOR_SIZE-1) );
   fseek( instreamfp, inat, SEEK_SET );

   if( outstreamfp )
   {
      fclose( outstreamfp );
   }

#ifdef DUMP_PACKET_LOG
   if ( logfp )
      fclose( logfp );
#endif
}

static void stream_merge_stream( FILE * instreamfp )
{
   FILE * outstreamfp = NULL;
   char outputFileName[FILENAME_MAX];

   sprintf( outputFileName, "%s/%s" BP_SPLIT_STREAM_EXTENSION, gOutputFolder, gCurrStreamName );

   outstreamfp = fopen( outputFileName, "wb" );
   if( !outstreamfp )
   {
      printf("Error: could not open %s for writing.\n", outputFileName );
      throw false;
   }
   printf("Merging into %s.\n", outputFileName );

   STREAM_TAG streamTag;
   std::vector< unsigned char > packetBuf( 16 * 1024, 0 );

   unsigned int inat = ftell( instreamfp );

   int currTick = 0;

   bool bMerging = gpStreamWriter_MergeSOL->ReplacementFileExistsForCurrStream(kPlatform_Base);
   do
   {
      //Read the next packet.
      int n = fread( &streamTag, sizeof( streamTag ), 1, instreamfp );
      if( n != 1 )
      {
         printf("Error reading %s\n", gCurrInputFilename );
         throw false;
      }

      packetBuf.assign( streamTag._size, 0 );
      memcpy( &packetBuf[0], &streamTag, sizeof( streamTag ) );

      if( int bodySize = streamTag._size - sizeof( streamTag ) )
      {
         fread( &packetBuf[sizeof(streamTag)], sizeof(unsigned char), bodySize, instreamfp );
      }

      if( bMerging )
      {
         gpStreamWriter_MergeSOL->ModifyCurrentSourcePacket( streamTag, &packetBuf[0], kPlatform_Base );

         //Before the current packet is written, allow SOL merge writer to insert as much data as it wants
         //based on the current stream tick.
         bool moreToWrite;
         do
         {
            //Write up to one packet at a time from each of the potential writing streams.
            unsigned int preAt = ftell( outstreamfp );
            gpStreamWriter_MergeSOL->WriteOnePacket( outstreamfp, kPlatform_Base, currTick );

            unsigned int postAt = ftell( outstreamfp );
            if ((postAt & (kStreamPacketAlign-1)) != 0)
            {
               printf("Error writing new data. Data is not aligned to 4 bytes.\n");
               throw false;
            }

            moreToWrite = ( postAt != preAt );
         }
         while( moreToWrite );
      }

      if( streamTag._time > currTick )
         currTick = streamTag._time;

      //If there is no stream writer converting or stripping this type of stream,
      //just pass it through to the rebuilt .dat file.
      //Otherwise throw out the packet.
      int discardType = streamTag._type;

      bool bDiscard = false;
      if( bMerging && gpStreamWriter_MergeSOL->ReplacesDataForCurrStream(discardType) )
      {
         bDiscard = true;
      }

      if (bMerging && (streamTag._type == CHUNK_TYPE_END))
      {
         //N.B. tick on us and jp_sol end chunk may not match at all; luckily, it seems that the tick
         //on this packet type is ignored and it just flags the end of the stream when it is hit.
         gpStreamWriter_MergeSOL->EndStream( outstreamfp, kPlatform_Base, &streamTag);

         unsigned int postAt = ftell( outstreamfp );
         if ((postAt & (kStreamPacketAlign-1)) != 0)
         {
            printf("Error writing new data. Data is not aligned to 4 bytes.\n");
            throw false;
         }
      }

      if( !bDiscard )
      {
         fwrite( &packetBuf[0], streamTag._size, 1, outstreamfp );
      }

      inat = ftell( instreamfp );

   } while ( streamTag._type != CHUNK_TYPE_END );

   inat = ( inat + SECTOR_SIZE - 1 ) & (~(SECTOR_SIZE-1) );
   fseek( instreamfp, inat, SEEK_SET );

   fclose( outstreamfp );
}

static void stream_extract_stream( FILE * instreamfp )
{
   char fullpath[FILENAME_MAX];
   sprintf( fullpath, "%s/%s" BP_SPLIT_STREAM_EXTENSION, gOutputFolder, gCurrStreamName );

   FILE * outstreamfp = fopen( fullpath, "wb" );

   if( !outstreamfp )
   {
      printf("Error: could not open %s for writing\n", fullpath );
      throw false;
   }

   STREAM_TAG streamTag;
   std::vector< unsigned char > packetBuf( 16 * 1024, 0 );

   unsigned int inat = ftell( instreamfp );

   printf("Extracting stream %s.\n", gCurrStreamName );

   do
   {
      int n = fread( &streamTag, sizeof( streamTag ), 1, instreamfp );
      if( n != 1 )
      {
         printf("Error reading %s\n", gCurrInputFilename );
         throw false;
      }

      packetBuf.assign( streamTag._size, 0 );
      memcpy( &packetBuf[0], &streamTag, sizeof( streamTag ) );

      if( int bodySize = streamTag._size - sizeof( streamTag ) )
      {
         fread( &packetBuf[sizeof(streamTag)], sizeof(unsigned char), bodySize, instreamfp );
      }

      fwrite( &packetBuf[0], streamTag._size, 1, outstreamfp );

      inat = ftell( instreamfp );

   } while ( streamTag._type != CHUNK_TYPE_END );

   inat = ( inat + SECTOR_SIZE - 1 ) & (~(SECTOR_SIZE-1) );
   fseek( instreamfp, inat, SEEK_SET );

   fclose( outstreamfp );
}

static const char * get_stream_source_folder( const char * const streamName, const char * const dstRegionFolder )
{
   if( gCommandLineArgs.mbCodecData )
   {
      //Always take the region-specific stream for codecs.
      return dstRegionFolder;
   }



#if MGS_VERSION==2


#if 1
   if( gCommandLineArgs.mRegion == kRegion_JP )
   {
      //For JP SKU, we've got rebuilt data for all streams now thanks to merge of US/JP_SOL data.
      return dstRegionFolder;
   }
#endif
   //Use US streams for all other in MGS2.
   (void)streamName;
   return skUsRegionFolder;

#elif MGS_VERSION==3

   if( gCommandLineArgs.mRegion == kRegion_JP && !gCommandLineArgs.mbBgmData )
   {
      //All JP streams except for BGM.DAT are different from the other SKU.
      return dstRegionFolder;
   }

   if( !gCommandLineArgs.mbVoxData )
   {
      //For all other streams except for VOX.DAT, use the US data.
      return skUsRegionFolder;
   }

#if 0 //TED - we now have HD movies for these, but only EN and JP.  So we want to use US for all now!
   //Five VOX streams in MGS3 with language-specific IPU streams embedded.
   const char * const skLanguageSpecificStreams[] =
   {
      "v040_010_r010_02",
      "v040_010_r010_07",
      "v050_020_r010_02",
      "v050_020_r010_04",
      "v100_030_r010_03"
   };
   for( int i=0; i < sizeof(skLanguageSpecificStreams)/sizeof(*skLanguageSpecificStreams); ++i )
   {
      if( !strcmp( streamName, skLanguageSpecificStreams[i] ) )
      {
         return dstRegionFolder;
      }
   }
#endif

   //default: use US data.
   return skUsRegionFolder;

#endif
}

static void stream_dat_build_table_file( const EPlatform platform )
{
   char fullTablePath[FILENAME_MAX];
   strcpy( fullTablePath, gOutputFolder );
   char regionFolder[FILENAME_MAX] = { 0 };
   char * const lastSlash = strrchr( fullTablePath, '/' );
   if( !lastSlash )
   {
      printf("Error: .dat file must be in a region folder!\n");
      throw false;
   }

   {
      const char * regionFolderAt = lastSlash;
      while( (regionFolderAt != &fullTablePath[0]) && (*(regionFolderAt-1) != '/') )
      {
         --regionFolderAt;
      }
      strncpy( regionFolder, regionFolderAt, lastSlash-regionFolderAt+1 );
   }

   char * const tableFolderAt = fullTablePath + strlen( fullTablePath );
   strcpy( tableFolderAt, "/" );
   CreateFullSubdirectories( fullTablePath );
   strcpy( tableFolderAt+1, skPlatformExtension[kPlatform_Base] );
   CreateFullSubdirectories( fullTablePath );
   strcpy( tableFolderAt+1, skPlatformExtension[platform] );
   CreateFullSubdirectories( fullTablePath );
   strcat( fullTablePath, "bp_streams.txt" );
   FILE * fp = fopen( fullTablePath, "wt" );
   if( !fp )
   {
      printf("Error: could not open %s for writing.\n", fullTablePath );
      throw false;
   }

   for( std::vector< TOffsetNamePair >::const_iterator it = gStreamOffsetsNames.begin(); it != gStreamOffsetsNames.end(); ++it )
   {
      const TOffsetNamePair & pr = *it;
      const char * const streamName = pr.second.c_str();

      //Write out the offset and full path for the stream we want to include.
      char fullStreamPath[FILENAME_MAX];

      strcpy( fullStreamPath, get_stream_source_folder( streamName, regionFolder ) );

      //Build the rest of the path.
      strcat( fullStreamPath, strrchr( gOutputFolder, '/' ) + 1 );
#if MGS_VERSION==2
      //Hack to redirect all vox2 streams to vox.  Better here than in-game!
      if( char * const vox2At = strstr( fullStreamPath, "/vox2" ) )
      {
         vox2At[4] = 0;
      }
#endif
      strcat( fullStreamPath, "/" );   //subfolder for this dat file
      char * const streamFolder = strrchr( fullStreamPath, '/' ) + 1;

      //Look in rebuilt platform-specific folder first.
      strcpy( streamFolder, skPlatformExtension[platform] );
      strcat( streamFolder, streamName );
      strcat( streamFolder, BP_SPLIT_STREAM_EXTENSION );
      if( !BP_FileExists( fullStreamPath ) )
      {
         //Look in rebuilt non-platform-specific folder.
         strcpy( streamFolder, skPlatformExtension[kPlatform_Base] );
         strcat( streamFolder, streamName );
         strcat( streamFolder, BP_SPLIT_STREAM_EXTENSION );
         if( !BP_FileExists( fullStreamPath ) )
         {
            //Take original stream.
            strcpy( streamFolder, streamName );
            strcat( streamFolder, BP_SPLIT_STREAM_EXTENSION );
         }
      }

      fprintf( fp, "%010d %s\n", pr.first, fullStreamPath );
   }

   fclose( fp );
}

static void stream_process( FILE * streamfp )
{
   char fulldir[FILENAME_MAX];

   sprintf( fulldir, "%s/%s/", gOutputFolder, gCurrStreamName );

   _mkdir( fulldir );

   switch( gCommandLineArgs.mUsageMode )
   {
   case kUsageMode_DumpInfo:
      {
         stream_info( streamfp );
      }
      break;
   case kUsageMode_ExtractSourceData:
      {
         stream_extract_sourcedata( streamfp );
      }
      break;
   case kUsageMode_ExtractStream:
      {
         stream_extract_stream( streamfp );
      }
      break;
   case kUsageMode_GenerateListFile:
      {
         stream_skip( streamfp );
      }
      break;
   case kUsageMode_RebuildStream:
      {
         unsigned int wasat = ftell( streamfp );
         for( int i=kPlatform_FirstValid; i<=kPlatform_LastValid; ++i )
         {
            const EPlatform platform = (EPlatform)i;
            if( ( gCommandLineArgs.mPlatform != kPlatform_Base ) && ( platform != gCommandLineArgs.mPlatform ) )
            {
               //Skip other platforms for this operation.
               continue;
            }

            fseek( streamfp, wasat, SEEK_SET );
            stream_rebuild_stream( streamfp, platform );
         }
      }
      break;
   case kUsageMode_MergeStreamSOL:
      {
         stream_merge_stream( streamfp );
      }
      break;
   default:
      printf("TODO!\n");
      throw false;
   }
}

static void stream_dat_process()
{
   if( !( gCommandLineArgs.mUsageMode == kUsageMode_GenerateListFile || gCommandLineArgs.mUsageMode == kUsageMode_ExtractStream ) )
   {
      printf("Error: usage mode not supported on entire .dat file.  Please extract stream and run on generated " BP_SPLIT_STREAM_EXTENSION " file!\n");
      throw false;
   }

   CreateFullSubdirectories( gOutputFolder );

   FILE * streamfp = fopen( gCurrInputFilename, "rb" );
   if( !streamfp )
   {
      printf("Error: could not open %s\n", gCurrInputFilename );
      throw false;
   }
   fseek( streamfp, 0, SEEK_END );
   unsigned int stream_end = ftell( streamfp );
   fseek( streamfp, 0, SEEK_SET );

   unsigned int inat = 0;

   printf("Processing dat file %s.\n", gCurrInputFilename );

   FILE * indexinfp = NULL;
   if( gCommandLineArgs.mUsageMode == kUsageMode_GenerateListFile )
   {
      indexinfp = fopen( gDatIndexFilename, "wt" );

      if( !indexinfp )
      {
         printf("Error: could not open %s for writing\n", gDatIndexFilename );
         throw false;
      }
   }

   for( unsigned int streamIndex = 0; inat != stream_end; ++streamIndex )
   {
      gCurrStreamName[0] = 0;

      //Check that offset matches that in .lst file if USA data; otherwise assume (hope) that there's a 1:1 ratio of streams in the correct order.
      if( streamIndex < (int)gStreamOffsetsNames.size() )
      {
         const TOffsetNamePair & streamOffsetName = gStreamOffsetsNames[streamIndex];
         if( ( streamOffsetName.first == inat ) || ( streamOffsetName.first == kUnknownStreamOffset ) || ( gCommandLineArgs.mRegion != kRegion_US ) )
         {
            strcpy( gCurrStreamName, gStreamOffsetsNames[streamIndex].second.c_str() );
         }
      }

      if( !gCurrStreamName[0] )
      {
         printf("Error: could not get stream offset name pair for stream at %d!\n", inat );
         throw false;
      }

      if( indexinfp )
      {
         fprintf( indexinfp, "%010d %s\n", inat, gCurrStreamName );
      }

      stream_process( streamfp );

      inat = ftell( streamfp );
   }

   if( indexinfp )
   {
      fclose( indexinfp );
   }

   fclose( streamfp );
}

static void codec_process()
{
   CreateFullSubdirectories( gOutputFolder );

   FILE * streamfp = fopen( gCurrInputFilename, "rb" );
   if( !streamfp )
   {
      printf("Error: could not open %s\n", gCurrInputFilename );
      throw false;
   }
   fseek( streamfp, 0, SEEK_END );
   unsigned int stream_end = ftell( streamfp );
   fseek( streamfp, 0, SEEK_SET );
   unsigned int at = 0;

   char fullpath[FILENAME_MAX];
   sprintf( fullpath, "%s/codecs.txt", gOutputFolder );

   FILE * codecsfp = NULL;

   if( gCommandLineArgs.mUsageMode == kUsageMode_DumpInfo )
   {
      codecsfp = fopen( fullpath, "wt" );

      if( !codecsfp )
      {
         printf("Error: could not open %s\n", fullpath );
         throw false;
      }
   }

   FILE * indexoutfp = NULL;
   if( gCommandLineArgs.mUsageMode == kUsageMode_GenerateListFile )
   {
      indexoutfp = fopen( gDatIndexFilename, "wt" );

      if( !indexoutfp )
      {
         printf("Error: could not open %s for writing\n", gDatIndexFilename );
         throw false;
      }
   }

   std::vector< unsigned char > packetBuf( 16 * 1024, 0 );

   int streamIndex = 0;

   do 
   {
      char codecname[FILENAME_MAX];

      //Failsafe: just name by offset.
      sprintf( codecname, "%08x", at );

      //Check that offset matches that in .lst file if USA data; otherwise assume (hope) that there's a 1:1 ratio of streams in the correct order.
      if( streamIndex < (int)gStreamOffsetsNames.size() )
      {
         const TOffsetNamePair & streamOffsetName = gStreamOffsetsNames[streamIndex];
         if( ( streamOffsetName.first == at ) || ( streamOffsetName.first == kUnknownStreamOffset ) || ( gCommandLineArgs.mRegion != kRegion_US ) )
         {
            sprintf( codecname, "%s", gStreamOffsetsNames[streamIndex].second.c_str() );

            if( gCommandLineArgs.mbRemapJapaneseCodecNames )
            {
               //This is a Japanese .lst.txt file, but we want the translated codec script name.  Look it up in the remapping table.
               for( int i=0; i < (int)gCodecOffsetsNamesRemapSrc.size(); ++i )
               {
                  if( gCodecOffsetsNamesRemapSrc[i].second == codecname )
                  {
                     strcpy( codecname, gCodecOffsetsNamesRemapDst[i].second.c_str() );
                     break;
                  }
               }

            }
         }
      }

      if( indexoutfp )
      {
         fprintf( indexoutfp, "%010d %s\n", at, codecname );
      }

      unsigned int codecHeader;
      int n = fread( &codecHeader, sizeof( codecHeader ), 1, streamfp );
      if( n != 1 )
      {
         printf("Error reading %s\n", gCurrInputFilename );
         throw false;
      }

      //kGCL_VariableVersion is a checksum of the script variables, which is different for
      //every SKU.  It should be consistent against all scripts in a .DAT file.
      static unsigned int kGCL_VariableVersion = 0;
      if( !kGCL_VariableVersion )
      {
         kGCL_VariableVersion = codecHeader;
      }

      if( codecHeader != kGCL_VariableVersion )
      {
         printf("Error: expected codec script to start with %08x\n", kGCL_VariableVersion );
         throw false;
      }

      //Find the beginning of the next script or the end of the file.
      int codecSize = sizeof( codecHeader );
      while( at + codecSize < stream_end )
      {
         unsigned int codecBody;
         n = fread( &codecBody, sizeof( codecBody ), 1, streamfp );
         if( n != 1 )
         {
            printf("Error reading %s\n", gCurrInputFilename );
            throw false;
         }
         if( codecBody == kGCL_VariableVersion )
         {
            //Hit the next codec.
            //Back it up, break, and let the main loop process it.
            fseek( streamfp, -(int)sizeof( codecBody ), SEEK_CUR );
            break;
         }
         codecSize += sizeof( codecBody );
      }

      //Dump info for this codec.
      packetBuf.assign( codecSize, 0 );
      fseek( streamfp, -codecSize, SEEK_CUR );
      fread( &packetBuf[0], codecSize, 1, streamfp );

      if( codecsfp )
      {
         std::string hashString = compute_md5_hash( &packetBuf[0], packetBuf.size() );
         fprintf( codecsfp, "HASH %s SIZE %6d NAME %s\n", hashString.c_str(), codecSize, codecname );
      }

      if( gCommandLineArgs.mUsageMode == kUsageMode_ExtractSourceData || gCommandLineArgs.mUsageMode == kUsageMode_ExtractStream )
      {
         //These were probably .gcx files, but we'll call them .sdt files for consistency with extracted streams proper.
         sprintf( fullpath, "%s/%s" BP_SPLIT_STREAM_EXTENSION, gOutputFolder, codecname );
         FILE * gcxfp = fopen( fullpath, "wb" );
         if( !gcxfp )
         {
            printf("Error: could not open %s\n", fullpath );
            throw false;
         }
         printf("Extracting codec script %s.\n", codecname );
         fwrite( &packetBuf[0], codecSize, 1, gcxfp );
         fclose( gcxfp );
      }

      at += codecSize;
      ++streamIndex;

   } while ( at < stream_end );

   if( codecsfp )
   {
      fclose( codecsfp );
   }

   if( indexoutfp )
   {
      fclose( indexoutfp );
   }

   fclose( streamfp );
}


bool parse_command_line( int argc, char const *argv[] )
{
   if( argc < 3 )
      return false;

   gInfoModeMask = 0;
   gCommandLineArgs.mUsageMode = kUsageMode_Invalid;
   gCommandLineArgs.mRegion = kRegion_US;
   gCommandLineArgs.mPlatform = kPlatform_Base;
   gCommandLineArgs.mInputFileType = kInputFileType_EntireDat;
   gCommandLineArgs.mbInputFileIsList = false;
   gCommandLineArgs.mbRemapJapaneseCodecNames = false;
   gCommandLineArgs.mbExportRadioCaptions = false;
   gCommandLineArgs.mbDumpCaptionText = false;
   gCommandLineArgs.mDumpLanguage = 0;

   for( int i=1; i < argc; ++i )
   {
      const char * arg = argv[i];
      if( !strncmp( arg, "-x", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_ExtractSourceData;
      }
      else if( !strncmp( arg, "-ipuinfo", 8 ) )
      {
         gInfoModeMask |= kInfoMode_IPUMask;
      }
      else if( !strncmp( arg, "-ffps", 5 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_ForceChangeFrameRate;
      }
      else if( !strncmp( arg, "-i", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_DumpInfo;
      }
      else if( !strncmp( arg, "-l", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_GenerateListFile;
      }
      else if( !strncmp( arg, "-b", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_RebuildStream;
      }
      else if( !strncmp( arg, "-s", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_ExtractStream;
      }
      else if( !strncmp( arg, "-t", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_BuildStreamTables;
      }
      else if( !strncmp( arg, "-m", 2 ) )
      {
         gCommandLineArgs.mUsageMode = kUsageMode_MergeStreamSOL;
      }
      else if( !strncmp( arg, "/l", 2 ) )
      {
         gCommandLineArgs.mSrcListFileName = arg+2;
      }
      else if( !strncmp( arg, "/s", 2 ) )
      {
         gCommandLineArgs.mProcessStreamName = arg+2;
      }
      else if( !strncmp( arg, "/o", 2 ) )
      {
         strcpy( gOutputFolder, arg+2 );
         BP_UnifyPath(gOutputFolder);
      }
      else if( !strncmp( arg, "/r", 2 ) )
      {
         gRegionString = arg+2;

         if( !strcmp( arg+2, "jp" ) )
         {
            gCommandLineArgs.mRegion = kRegion_JP;
         }
         else if( !strcmp( arg+2, "eu" ) || !strcmp( arg+2, "fr" ) || !strcmp( arg+2, "gr" ) || !strcmp( arg+2, "it" ) || !strcmp( arg+2, "sp" ) )
         {
            gCommandLineArgs.mRegion = kRegion_EU;
         }
         else if( !strcmp( arg+2, "us" ) )
         {
            gCommandLineArgs.mRegion = kRegion_US;
         }
         else
         {
            printf("Error: unknown region specified: '%s'.\n", arg+2 );
            return false;
         }
      }
      else if( !strncmp( arg, "/p", 2 ) )
      {
         if( !strcmp( arg+2, "_ps3" ) )
         {
            gCommandLineArgs.mPlatform = kPlatform_PS3;
         }
         else if( !strcmp( arg+2, "_360" ) )
         {
            gCommandLineArgs.mPlatform = kPlatform_X360;
         }
         else if (!strcmp(arg + 2, "_vta"))
         {
            gCommandLineArgs.mPlatform = kPlatform_Vita;
         }
         else
         {
            printf("Error: unknown platform specified: '%s'.\n", arg+2 );
            return false;
         }
      }
      else if( !strcmp( arg, "/j" ) )
      {
         gCommandLineArgs.mbRemapJapaneseCodecNames = true;
      }
      else if( !strcmp( arg, "/cc" ) )
      {
         gCommandLineArgs.mbExportRadioCaptions = true;
      }
      else if( !strncmp( arg, "/i", 2 ) )
      {
         char seps[] = ",";
         char * tok = strtok( (char*)arg+2, seps );
         while( tok )
         {
            int packetType;
            if( 1 != sscanf( tok, "%x", &packetType ) )
            {
               printf("Error: bad format packet type: %s\n", tok );
               return false;
            }
            gPacketTypeFilter.push_back( packetType );
            tok = strtok( NULL, seps );
         }
      }
      else if( !strncmp( arg, "/d", 2 ) )
      {
         gCommandLineArgs.mbDumpCaptionText = true;
         if( 1!=sscanf( arg+2,"%d", &gCommandLineArgs.mDumpLanguage ) )
         {
            printf("Error: invalid language for /d option: %s\n", arg+2 );
            throw false;
         }
      }
      else
      {
         strcpy( gCommandLineArgs.mMainInputFilename, arg );
         _strlwr( gCommandLineArgs.mMainInputFilename );
         BP_UnifyPath( gCommandLineArgs.mMainInputFilename );
         if( const char * const at = strchr( gCommandLineArgs.mMainInputFilename, '@' ) )
         {
            if( ( at == gCommandLineArgs.mMainInputFilename ) || ( at == strrchr( gCommandLineArgs.mMainInputFilename, '/' ) ) )
            {
               gCommandLineArgs.mbInputFileIsList = true;
               strcpy( gCommandLineArgs.mMainInputFilename, arg+1 );
               _strlwr( gCommandLineArgs.mMainInputFilename );
            }
         }
      }
   }

   return gCommandLineArgs.mMainInputFilename[0] != NULL && ( gCommandLineArgs.mUsageMode != kUsageMode_Invalid ) && ( gCommandLineArgs.mUsageMode != kUsageMode_GenerateListFile || gCommandLineArgs.mSrcListFileName[0] != NULL );
}


void process_input_file()
{
   if( const char * const sdatExt = strstr( gCurrInputFilename, BP_SPLIT_STREAM_EXTENSION ) )
   {
      gCommandLineArgs.mInputFileType = kInputFileType_SingleStream;
      //strip fullpath and extension to get stream name.
      strcpy( gCurrStreamName, strrchr( gCurrInputFilename,'/')+1 );
      *strstr( gCurrStreamName, BP_SPLIT_STREAM_EXTENSION ) = 0;
   }
   else
   {
      sprintf( gDatIndexFilename, "%s.lst.txt", gCurrInputFilename );
      if( gOutputFolder[0] == 0 )
      {
         //default output folder is dat file name.
         if( const char * ext = strchr(gCurrInputFilename, '.' ) )
         {
            strcpy( gOutputFolder, gCurrInputFilename );
            gOutputFolder[ext-gCurrInputFilename] = 0;
         }
      }
   }

   if( strstr( gCurrInputFilename, "bgm.dat" ) || strstr( gCurrInputFilename, "bgm_2.dat" ) )
   {
      gCommandLineArgs.mbBgmData = true;
   }
   else if( strstr( gCurrInputFilename, "codec" ) )
   {
      gCommandLineArgs.mbCodecData = true;
   }
   else if( strstr( gCurrInputFilename, "vox.dat" ) )
   {
      gCommandLineArgs.mbVoxData = true;
   }

   if( gCommandLineArgs.mInputFileType==kInputFileType_EntireDat )
   {
      load_list_file();
   }

   if( gCommandLineArgs.mUsageMode == kUsageMode_BuildStreamTables )
   {
      for( int i=kPlatform_FirstValid; i<=kPlatform_LastValid; ++i )
      {
         const EPlatform platform = (EPlatform)i;
         if( ( gCommandLineArgs.mPlatform != kPlatform_Base ) && ( platform != gCommandLineArgs.mPlatform ) )
         {
            //Skip other platforms for this operation.
            continue;
         }

         stream_dat_build_table_file( platform );
      }
   }
   else if( gCommandLineArgs.mbCodecData )
   {
      codec_process();
   }
   else
   {
      if( gCommandLineArgs.mInputFileType == kInputFileType_SingleStream )
      {
         //handle extract source data, rebuild stream, or info mode.
         switch( gCommandLineArgs.mUsageMode )
         {
         case kUsageMode_ExtractSourceData:
         case kUsageMode_DumpInfo:
         case kUsageMode_RebuildStream:
         case kUsageMode_MergeStreamSOL:
            {
               FILE * streamfp = fopen( gCurrInputFilename, "rb" );
               if( !streamfp )
               {
                  printf("Error: could not open %s\n", gCurrInputFilename );
                  throw false;
               }
               stream_process( streamfp );
               fclose( streamfp );
            }
            break;
         default:
            {
               printf("Error: usage mode not supported on extracted " BP_SPLIT_STREAM_EXTENSION " file.  Please run on source .dat file!\n");
               throw false;
            }
         }
      }
      else
      {
         //handle build lst.txt file or extract stream mode.
         if( ( gCommandLineArgs.mProcessStreamIndex != -1 ) && (gCommandLineArgs.mUsageMode == kUsageMode_ExtractStream) )
         {
            //single stream processing.
            FILE * streamfp = fopen( gCurrInputFilename, "rb" );
            if( !streamfp )
            {
               printf("Error: could not open %s\n", gCurrInputFilename );
               throw false;
            }

            //Seek to the start of this stream and process just it.
            fseek( streamfp, gStreamOffsetsNames[gCommandLineArgs.mProcessStreamIndex].first, SEEK_SET );
            strcpy( gCurrStreamName, gCommandLineArgs.mProcessStreamName );
            stream_process( streamfp );

            fclose( streamfp );
         }
         else
         {
            //stream dat processing.
            stream_dat_process();
         }
      }
   }
}

static bool is_packet_type_filtered_out( const int packetType )
{
   if( gPacketTypeFilter.empty() )
   {
      //No packet type filter supplied.
      return false;
   }
   if( std::find( gPacketTypeFilter.begin(), gPacketTypeFilter.end(), packetType ) != gPacketTypeFilter.end() )
   {
      //Packet type passes the filter.
      return false;
   }

   //Filtered out.
   return true;
}

void ChangeFPS()
{
   FILE * textfp = fopen( gCommandLineArgs.mMainInputFilename, "rt" );
   if( !textfp )
   {
      printf("Error: could not open %s for reading.\n", gCommandLineArgs.mMainInputFilename );
      throw false;
   }
   char processFilename[FILENAME_MAX];
   while( 1 == fscanf( textfp, "%s", processFilename ) )
   {
      printf("Processing %s...\n", processFilename );
      strcpy( gCurrInputFilename, processFilename );

      ChangeFPS_SingleFile(); // This function is in CStreamWriter_Movies.cpp
   }
   fclose( textfp );
}

int main( int argc, char const *argv[] )
{
   if( !parse_command_line( argc, argv ) )
   {
      usage();
      return -1;
   }

   if( gCommandLineArgs.mUsageMode == kUsageMode_ForceChangeFrameRate )
   {
      // This mode is special, all it does it change fps and exits
      ChangeFPS();
      return 0;
   }
   // MCampbell - Uncomment this to just dump raw data. Useful for seeing original sizes of different types of data
   // in the streams.
   //gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_MTA, new CStreamDriver_RawExtract("mta") ) );
   //gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_STV, new CStreamDriver_RawExtract("stv") ) );

   //Add handlers for the various stream packet types.
#if MGS_VERSION==3
   if( !is_packet_type_filtered_out( CHUNK_TYPE_PCM ) || !is_packet_type_filtered_out( STREAM_TYPE_MTA ) )
   {
      gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_MTA, new CStreamDriver_MTA() ) );
   }
   if( !is_packet_type_filtered_out( CHUNK_TYPE_PCM ) || !is_packet_type_filtered_out( STREAM_TYPE_STV ) )
   {
      gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_STV, new CStreamDriver_STV() ) );
   }

   //Untested in MGS2; format could be same or different.
//   if( !is_packet_type_filtered_out( CHUNK_TYPE_DEMO ) )
//      gStreamDriverMap.insert( TStreamDriverMap::value_type( CHUNK_TYPE_DEMO, new CStreamDriver_Demo() ) );

#elif MGS_VERSION==2
   //N.B. "subtype" which is the top 2 bytes of type can be 2, 1, or 0 for PCM streams in MGS2.
   //1 indicates AC3.  0 or 2 indicates vag or str3.  Seems the only time 2 was used was in streams
   //that contained both AC3 and non-AC3 streams.

   //TODO: need a stream driver that extracts AC3 data so we can check it against the sound dump we got.
   //There are only three (used) AC3 streams in the entire game!
   if( !is_packet_type_filtered_out( CHUNK_TYPE_PCM ) )
   {
      gStreamDriverMap.insert( TStreamDriverMap::value_type( CHUNK_TYPE_PCM, new CStreamDriver_PCM() ) );
      gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_VAG, new CStreamDriver_PCM() ) );
   }

#endif

   //This will replace any PCM audio streams encountered where there is a replacement .pcm file.
   //(Meaning the original MGS2 audio format, only if new platform files are not present)
   gStreamWriterVector.push_back( new CStreamWriter_PCM() );

   if( !is_packet_type_filtered_out( STREAM_TYPE_M2V ) )
      gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_M2V, new CStreamDriver_M2V() ) );

   if( !is_packet_type_filtered_out( STREAM_TYPE_IPU ) )
      gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_IPU, new CStreamDriver_IPU() ) );

   if( !is_packet_type_filtered_out( STREAM_TYPE_CAP ) )
      gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_CAP, new CStreamDriver_CAP(CStreamDriver_CAP::kMode_Caption) ) );

   //TED - adding last minute support to reading / writing codec captions.
   //To minimize risk and croft, don't export this data by default in -x mode and we'll only generate it for ones we need to edit and re-integrate.
   //use /cc option to export this data, and nothing else, in -x mode.
   if( gCommandLineArgs.mbExportRadioCaptions )
   {
      gStreamDriverMap.clear();
      gStreamDriverMap.insert( TStreamDriverMap::value_type( STREAM_TYPE_CODEC_CAP, new CStreamDriver_CAP(CStreamDriver_CAP::kMode_RadioCaption) ) );
   }

   //Insert stream writers.
   gStreamWriterVector.reserve( 256 );

   //This will replace any audio streams encountered where there is a replacement .msf file.
   gStreamWriterVector.push_back( new CStreamWriter_MSF() );

   //This will replace any caption streams encountered
   gStreamWriterVector.push_back( new CStreamWriter_CAP( gCommandLineArgs.mbDumpCaptionText, gCommandLineArgs.mDumpLanguage ) );
   
   //This will replace any radio caption streams encountered
   gStreamWriterVector.push_back( new CStreamWriter_RadioCAP() );

   gStreamWriterVector.push_back(new CStreamWriter_Movies());

   gStreamWriterVector.push_back(new CStreamWriter_XAudio());

   gStreamWriterVector.push_back(new CStreamWriter_Vita_AT9());

   //This will insert 50hz demo packets into any stream that contains (60hz) demo packets.
//   gStreamWriterVector.push_back( new CStreamWriter_Demo() );

   //This writer handles any types we want to just strip out of rebuilt .DAT files.
   gStreamWriterVector.push_back( new CStreamWriter_NULL() );

   //This writer is only used in merge mode.
   gpStreamWriter_MergeSOL = new CStreamWriter_MergeSOL();

   try
   {
      if( gCommandLineArgs.mbRemapJapaneseCodecNames )
      {
         //This is a single-purpose switch to ensure that rebuilt codec scripts get extracted with names
         //that match our automated-translated filenames rather than the ones specified in the source .cdc
         //scripts.  When the .lst.txt file for this (intermediate) .DAT file is parsed, the Japanese
         //names will be converted on the fly to our filenames using this mapping.
         if( const char * const bpe_repository = getenv( "BPE_REPOSITORY" ) )
         {
            char remappingPath[FILENAME_MAX];
            sprintf( remappingPath, "%s/%s/codec.dat.org.lst.txt", bpe_repository, gRegionString );
            do_load_list_file( remappingPath, gCodecOffsetsNamesRemapSrc );
            sprintf( remappingPath, "%s/%s/codec.dat.lst.txt", bpe_repository, gRegionString );
            do_load_list_file( remappingPath, gCodecOffsetsNamesRemapDst );
            if( gCodecOffsetsNamesRemapSrc.size() != gCodecOffsetsNamesRemapDst.size() )
            {
               printf("Error: src and dst remapping table sizes don't match!\n");
               throw false;
            }
         }
         else
         {
            printf("Error: I need BPE_REPOSITORY to load Japanese codec name remapping files!\n");
            throw false;
         }
      }

      if( !gCommandLineArgs.mbInputFileIsList )
      {
         strcpy( gCurrInputFilename, gCommandLineArgs.mMainInputFilename );
         process_input_file();
      }
      else
      {
         FILE * textfp = fopen( gCommandLineArgs.mMainInputFilename, "rt" );
         if( !textfp )
         {
            printf("Error: could not open %s for reading.\n", gCommandLineArgs.mMainInputFilename );
            throw false;
         }
         char processFilename[FILENAME_MAX];
         while( 1 == fscanf( textfp, "%s", processFilename ) )
         {
            printf("Processing %s...\n", processFilename );
            strcpy( gCurrInputFilename, processFilename );
            process_input_file();
         }
         fclose( textfp );
      }
   }
   catch(System::Exception^ e)
   {
      System::Console::WriteLine(e->ToString());
      return -1;
   }
   catch(...)
   {
      printf("streamtool failed.\n");
      return -1;
   }

   for( TStreamWriterVector::iterator it = gStreamWriterVector.begin(); it != gStreamWriterVector.end(); ++it )
   {
      CStreamWriter * const pStreamWriter = *it;
      delete pStreamWriter;
   }
   gStreamWriterVector.clear();

   for( TStreamDriverMap::iterator it = gStreamDriverMap.begin(); it != gStreamDriverMap.end(); ++it )
   {
      CStreamDriver * const pStreamDriver = it->second;
      delete pStreamDriver;
   }
   gStreamDriverMap.clear();

   return 0;
}
