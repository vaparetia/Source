#ifndef __KP_H__
#define __KP_H__

#ifndef MGS_VERSION
#  error MGS_VERSION not defined!
#endif

//----------------------------------------------------------------------------
// The items in this file come from other KP (Konami Productions) code
//----------------------------------------------------------------------------

#define ID_SECTION_END	0x7F000000
#define ID_NOCACHE		0x7F000001
#define ID_CACHE		0x7F000002
#define ID_RESIDENT		0x7F000003
#define ID_SOUND		0x7F000010
#define ID_BINARY		0x7F010000
#define ID_BLOCK( no )	( 0x7F000100 | (no) )

#define ID_COMPRESS		0x7E000000
#define ID_NOP			0x70000000

#define IS_SECTION_TAG( a )	( ( (a)&0xFF000000 ) == 0x7F000000 )
#define IS_SECTION_END( a )	( (a) == ID_SECTION_END )
#define IS_PRELOAD_TAG( a )	( ( (a) & ~0xFF ) != ID_BLOCK(0) )
#define IS_END_TAG( a )		( (a) == 0 )

#define IS_COMPRESSION_TAG( a )		( ( (a)&0xFF000000 ) == 0x7E000000 )
#define COMPRESSION_SIZE( a )		( (a)&0x00FFFFFF )

#define IS_COMMAND_TAG( a )	( ( (a)&0xF0000000 ) == 0x70000000 )

#if MGS_VERSION==3
#define ID_SLOT			0x7D000000
#define IS_SLOT_TAG( a )	( ( (a)&0xFF000000 ) == ID_SLOT )
#define SLOT_NAME( a )		( (a)&0x00FFFFFF )
#else
#define IS_SLOT_TAG(a) ( 0 )
#define SLOT_NAME(a) ( 0 )
#endif

typedef struct {
   unsigned int seed;
   short version;
   short sector;
   short stagenum;
   short padding;
   int install_size;
} STAGE_TABLE_HEADER;

#if MGS_VERSION == 2
static const int STAGE_NAME_LEN = 8;
#elif MGS_VERSION == 3
static const int STAGE_NAME_LEN = 16;
#endif

typedef struct {
   char name[ STAGE_NAME_LEN ];
   int offset;
} STAGE_TABLE;

typedef struct {
   STAGE_TABLE_HEADER table_header;
   STAGE_TABLE *stage_table;
} STAGE_FILE;

typedef struct {
   int id;
   int offset;
} DATACNF_TAG;

typedef struct {
   int tagnum;
#pragma warning( push )
#pragma warning( disable: 4200 )
   DATACNF_TAG tags[0];
#pragma warning( pop )
} DATACNF;

namespace KPCrypt
{
   extern unsigned int seed;
   extern unsigned int _seed;

   extern void setup_decrypt( void *buffer );
   extern void decrypt_buffer( void *buffer, int size );
   extern void set_encode( void );
   extern void encode_buffer( void *buffer, int size );
}

extern int GV_StrCode( char const *string );

unsigned const kSectorSize = 2048;

inline size_t const round_up_sector( size_t const i )
{
   return ( i + (kSectorSize-1) ) & ( ~(kSectorSize-1) );
}

inline size_t const amount_until_next_sector( size_t const size )
{
   return round_up_sector( size ) - size;
}


#endif