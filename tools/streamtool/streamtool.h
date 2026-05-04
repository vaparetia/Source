

#pragma once

#pragma warning( disable:4996 )

#include <string>

#define SECTOR_SIZE 0x800
#define STREAM_TICKS_PER_SECOND 300


enum stream_chunk_type {
   CHUNK_TYPE_FREE = 0,

   CHUNK_TYPE_PCM = 1,
   CHUNK_TYPE_DEMO = 2,
   CHUNK_TYPE_LIP	= 3,
   CHUNK_TYPE_CAPTION = 4,
   CHUNK_TYPE_MOTION = 5,
   CHUNK_TYPE_CODEC_CAP = 6,

   CHUNK_TYPE_CRI_MPEG	= 0x0C,
   CHUNK_TYPE_CRI_IPIC	= 0x0D,

   CHUNK_TYPE_MPEG2_VIDEO = 0x0E,
   CHUNK_TYPE_IPU = 0x0F,

   CHUNK_TYPE_SYSTEM = 0x10,

   // BP custom chunks
   CHUNK_TYPE_MOVIE = 0x20,

   CHUNK_TYPE_USE = 0x80,
   CHUNK_TYPE_END = 0xf0,
   CHUNK_TYPE_BUFEND = 0xff
};

#define STREAM_TYPE_STV          0x00100001
#define STREAM_TYPE_MTA	         0x00110001
#define STREAM_TYPE_M2V          CHUNK_TYPE_MPEG2_VIDEO
#define STREAM_TYPE_AC3          0x00010001
#define STREAM_TYPE_VAG          0x00020001
#define STREAM_TYPE_CAP          CHUNK_TYPE_CAPTION
#define STREAM_TYPE_CODEC_CAP    CHUNK_TYPE_CODEC_CAP
#define STREAM_TYPE_IPU          CHUNK_TYPE_IPU

//BP added types
#define STREAM_TYPE_MSF 0x00030001
#define STREAM_TYPE_XAUDIO 0x00040001
#define STREAM_TYPE_VITA_AT9 0x00050001

#define BP_STREAM_SUBTYPE_DEMO_60      0x01000000
#define BP_STREAM_SUBTYPE_DEMO_50      0x02000000

enum {
   GM_LANG_DEFAULT         = 0,
   GM_LANG_ENGLISH         = 1,
   GM_LANG_ENGLISH_USA     = 1,
   GM_LANG_ENGLISH_ENGLAND = 1,
   GM_LANG_FRENCH          = 2,
   GM_LANG_GERMANY         = 3,
   GM_LANG_ITALY           = 4,
   GM_LANG_SPANISH         = 5,
   GM_LANG_KOREAN          = 6,
   GM_LANG_JAPANESE        = 7
};

//----------------------------------------------------------------------------

typedef struct _STREAM_TAG {
   int _type;
   int _size;
   int _time;
   int _option;
} STREAM_TAG;

//Platform for rebuild
enum EPlatform
{
   kPlatform_Base, //not platform specific
   kPlatform_PS3,
   kPlatform_X360,
   kPlatform_Vita,

   kPlatform_Count,
   kPlatform_FirstValid = 0,
   kPlatform_LastValid = kPlatform_Count-1
};

enum EPlatformMask
{
   kPlatformMask_Base = 1 << kPlatform_Base,
   kPlatformMask_PS3 = 1 << kPlatform_PS3,
   kPlatformMask_X360 = 1 << kPlatform_X360,
   kPlatformMask_Vita = 1 << kPlatform_Vita,
};

enum EInfoMode
{
   kInfoMode_IPU,
};

enum EInfoModeMask
{
   kInfoMode_IPUMask = 1 << kInfoMode_IPU,
};

//----------------------------------------------------------------------------

extern void SaveLPCM16WavFile( const char * const filename, const unsigned int channelCount, const unsigned int frequency, const unsigned int dataSize, const unsigned char * const data );

extern char gOutputFolder[];
extern char gCurrInputFilename[];
extern char gCurrStreamName[];
extern const char * gRegionString;
extern int gInfoModeMask; // EInfoModeMask

bool BP_FileExists( const char * const path );
void BP_UnifyPath( char * path );
void BP_ReplaceStreamSourceDataRegion( char * outFullPath, const char * const inputPath, const char * const newRegion );
const char * BP_GetPlatformSubfolder( const EPlatform platform );

std::string BP_ConvertManagedString( System::String^ value );

//----------------------------------------------------------------------------
