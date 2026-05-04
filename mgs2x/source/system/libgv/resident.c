//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	常駐メモリ領域管理ライブラリ

	2000/01/13 K.Uehara
	$Id: resident.c,v 1.1.1.3 2002/11/19 11:42:45 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include	"libgv.h"
#include	"libgv.cnf"
#include "BP_BaseRenderer.h"

#define CHECK_PAD_1		0x12345679
#define CHECK_PAD_2		0x97654321

/*
	常駐データ
*/

typedef struct {
	int size;
	int id;
	int pad1;
	int pad2;
} RESIDENT_HEADER;

void *GV_ResidentMemoryBottom;
void *GV_UserResidentBottom;
static void _GV_FreeUserResidentData();

void GV_InitResidentMemory( void )
{
	GV_ResidentMemoryBottom = RESIDENT_BOTTOM;
}

void GV_SetSystemResident( void )
{
	GV_UserResidentBottom = GV_ResidentMemoryBottom;
}

void GV_ResetResidentMemory( void )
{
   _GV_FreeUserResidentData(); // BP - New function call

	GV_ResidentMemoryBottom = GV_UserResidentBottom;
}

void *GV_AllocResidentMemory( int size, int id )
{
	RESIDENT_HEADER *hp;

	size = size + sizeof( RESIDENT_HEADER );
	size = NORM16( size );

	GV_ResidentMemoryBottom = ( char * )GV_ResidentMemoryBottom - size;

#ifdef DEBUG_MODE
	if( (char*) GV_ResidentMemoryBottom < (char*) ( RESIDENT_BOTTOM - RESIDENT_SIZE) ){
		printf( "RESIDENT DATA TOO BIG !! limit %X now %X\n"
			   , ( RESIDENT_BOTTOM - RESIDENT_SIZE)
			   , GV_ResidentMemoryBottom );
		HANGUP();
	}
#endif

	hp = GV_ResidentMemoryBottom;
	if( id != 0 ){
		printf( "RESIDENT ID = %X ADR %08X\n", id, hp + 1 );
	}
	hp->size = size;
	hp->id = id;
#ifdef DEBUG_MODE
	hp->pad1 = CHECK_PAD_1;
	hp->pad2 = CHECK_PAD_2;
#endif

	return ( void * )( hp + 1 );
}

void *GV_AllocResidentMemoryAligned( int size, int id, int align )
{
	RESIDENT_HEADER *hp;
	unsigned int addr;
printf( "ALIGN %d SIZE %d\n", align, size );
	addr = ( unsigned int  )GV_ResidentMemoryBottom - size;
	addr = addr - ( addr % align );

	size = ( ( unsigned int  )GV_ResidentMemoryBottom - addr ) +  sizeof( RESIDENT_HEADER );
printf( "TOTAL SIZE = %d\n", size );
	GV_ResidentMemoryBottom = ( char * )GV_ResidentMemoryBottom - size;
printf( "ADD = %X\n", GV_ResidentMemoryBottom );

#ifdef DEBUG_MODE
	if( (char*) GV_ResidentMemoryBottom < (char*) ( RESIDENT_BOTTOM - RESIDENT_SIZE) ){
		printf( "RESIDENT DATA TOO BIG !!\n" );
		HANGUP();
	}
#endif

	hp = GV_ResidentMemoryBottom;
	if( id != 0 ){
		printf( "RESIDENT ID = %X ADR %08X\n", id, hp + 1 );
	}
	hp->size = size;
	hp->id = id;
#ifdef DEBUG_MODE
	hp->pad1 = CHECK_PAD_1;
	hp->pad2 = CHECK_PAD_2;
#endif

	return ( void * )( hp + 1 );
}

void GV_ReinitResidentData( void )
{
	/* 常駐データの再初期化 */
	RESIDENT_HEADER *hp;

	hp = GV_ResidentMemoryBottom;
printf( "RESIDENT TOP = %X\n", hp ) ;

	while( ( void * )hp < ( void* )RESIDENT_BOTTOM ){
		// ID == 0 のものはcacheに登録しない
		if( hp->id != 0 ){

printf( "REINIT %X %X (%c)\n", ( void * )( hp + 1 ), hp->id
		, ( ( hp->id >> 24 ) < MAX_LOADERS ) ? 'a' + ( hp->id >> 24 ) : '?' );

			GV_LoadInit( ( void * )( hp + 1 ), hp->id, GV_INIT_RESIDENT_AGAIN );
		}
#ifdef DEBUG_MODE
		if( hp->pad1 != CHECK_PAD_1 || hp->pad2 != CHECK_PAD_2 ){
			printf( "RESIDENT ERR %X %X %X\n", hp, hp->pad1, hp->pad2 );
			HANGUP();
		}
#endif

      if ( hp->size == 0 )
      {
         printf( "RESIDENT SIZE == 0\n" );
         HANGUP();
      }
		hp = ( RESIDENT_HEADER * )( ( char * )hp + hp->size );
	}
	ASSERT( hp == (RESIDENT_HEADER*) RESIDENT_BOTTOM );
}

int GV_GetResidentDataSize( void *ptr )
{
	RESIDENT_HEADER *head;

	head = ( RESIDENT_HEADER * )ptr - 1;

	return head->size;
}

static void _GV_FreeUserResidentData()
{
   // On MGS3, this function is much more substantial, freeing up a lot of things for slot pages.

   BP_DestroyResources( (unsigned) GV_ResidentMemoryBottom, (unsigned) GV_UserResidentBottom );
   BP_FlushDestroyedResources();
}
