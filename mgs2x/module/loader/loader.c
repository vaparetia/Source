/*
	usbkbd.c
		USB キーボードデータ取得ドライバ
		2000/04/05 K.Uehara
	$Id: loader.c,v 1.6 2001/09/13 03:58:48 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <sif.h>
#include <sifcmd.h>
#include <sifrpc.h>

typedef struct header {
	int total_size;
	int arg_offset;
	int arg_size;
	int resv;
} HEADER;

#define MASK	0x7E8A6B4C
#define MASK_D	0xa84e6b2e

static int mask = MASK;

#ifdef MASTER
#define printf( fmt, args... )
#define Kprintf( fmt, args... )
#endif

static void get_data( void *buffer, void *addr, int size )
{
	sceSifReceiveData rd;
	unsigned int *p;
	int i;

	sceSifGetOtherData( &rd, addr, buffer, size, 0 );

	p = buffer;
	for( i = size; i > 0; i -= sizeof( int ) ){
		*p = *p ^ mask;
		mask = mask + MASK_D;
		p++;
	}
}

static int loader( void *addr )
{
	HEADER header;

	for( ;; ){
		void *buf;
		int intr;

		mask = MASK;

		// HEADERのロード
		get_data( &header, addr, sizeof( header ) );
		addr += sizeof( header );

		if( header.total_size == 0 ){
			// 終り
			break;
		}

		// 本体をメモリの下位にロードする
		CpuSuspendIntr( &intr );
		buf = AllocSysMemory( 1, header.total_size, NULL );
		CpuResumeIntr( intr );

		if( buf == NULL ){
			printf( "NO MEMORY !!\n" );
			return -1;
		}

		get_data( buf, addr, header.total_size );

		addr += header.total_size;
		{
			char *fname, *argp;
			int args;
			int res;
			int module_id;

			fname = buf + header.arg_offset;
			argp = fname + strlen( fname ) + 1;
			args = header.arg_size - ( strlen( fname ) + 1 );
printf( ":%s\n", fname );
			// モジュールをセットアップ
			module_id = LoadModuleBuffer( buf );
			FreeSysMemory( buf );
			if( module_id < 0 ){
				printf( "module error %d\n", module_id );
				continue;
			}
			// バッファを削除

			{
				ModuleStatus ms;
				ReferModuleStatus( module_id, &ms );

				printf( "ID = %d\n", ms.id );
				printf( "name = %s\n", ms.name );
				printf( "entry = %X\n", ms.entry_addr );
				printf( "text_addr = %X\n", ms.text_addr );
			}

			res = StartModule( module_id, fname, args, argp, &res );
			if( res < 0 ){
				printf( "start error %d\n", res );
			}
		}
	}
	return 0;
}

int loader_start( int argc, char *argv[] )
{
	int addr;

	if( argc < 2 ){
		printf( "NO ARG\n" );
	}
	addr = strtol( argv[ 1 ], NULL, 16 );

	printf( ":%X\n", addr );

	sceSifInitRpc( 0 );

	if( loader( ( void * )addr ) < 0 ){
printf( "Error\n" );
		return -1;
	}

	// 読み込んで初期化し終ったら終了

	return NO_RESIDENT_END;
}
