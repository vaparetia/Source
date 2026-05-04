/*
	fshd.c
		ハードディスク読み書き用ファイルシステム

		ファイルは一度に１つしかオープンできない。

	1999/05/27 K.Uehara
	$Id: fshdx.c,v 1.6 2002/08/16 04:29:26 usr01475 Exp $

	2001/04/06 M.Kobayashi
	Xに移植。
	ここのAPIはそろえる。
	
*/

#ifdef KP_XBOX //BP

#include <stdio.h>

#ifdef KP_XBOX
#include <xtl.h>
#else
#define STRICT
#include <windows.h>
#endif

#include "libgv.h"
#include "libfs.h"
#include "cdbios.h"

#define FS_ROOT_DIR		"D:"

//#define LOAD_UNIT	(8196*16*16)
#define LOAD_UNIT	(0x7FFFFFFF)

static char current_dir[ 64 ];

typedef struct _HDFILE {
	HANDLE	hFile;
	DWORD size;
	int unit;
	void *buf;
} HDFILE;

static HDFILE finfo;

/* ----------------------------------------------------- */

void FS_ChangeDirectory( char *dir )
{
	extern void sd_change_directory( const char* path ) ;

	sprintf( current_dir, FS_ROOT_DIR "\\stage\\%s\\", dir );
	// サウンドにディレクトリを通知
	sd_change_directory( current_dir );
}

int FS_LoadRequest( char *name )
{
	char full[ 128 ];
	HDFILE *fn;
	HANDLE hFile;
	DWORD size;

	fn = &finfo;
	ASSERT( fn->hFile == INVALID_HANDLE_VALUE );	// 読み込み中ではないか

	sprintf( full, "%s%s", current_dir, name );
	if( ( hFile = CreateFile( full, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL ) ) == INVALID_HANDLE_VALUE ){
		printf( "FILE %s not found\n", full );
		return 0;
	}
	size = GetFileSize( hFile, NULL );	// 4Gより大きいサイズだと失敗する。まずないだろう。
	ASSERT(size != -1);

	fn->buf = NULL;
	fn->hFile = hFile;
printf( "Load : %s %d\n", full, size );
	return size;
}

void FS_LoadSet( void *buf, int size )
{
	HDFILE *fn;
	fn = &finfo;

	ASSERT( fn->hFile != INVALID_HANDLE_VALUE );
printf( "LoadSet %X size %d\n", buf, size );
	fn->buf = buf;
	fn->unit = LOAD_UNIT;
	fn->size = size;
}

void FS_LoadStop( void )
{
	HDFILE *fn;
	fn = &finfo;

	ASSERT( fn->hFile != INVALID_HANDLE_VALUE );

	CloseHandle( fn->hFile );
	fn->hFile = INVALID_HANDLE_VALUE;
	fn->buf = NULL;
}

int FS_LoadSync( void )
{
	HDFILE *fn;
	DWORD size;
	DWORD dwBytesRead;

	fn = &finfo;

	ASSERT( fn->buf != NULL );

	size = ( fn->size < LOAD_UNIT ) ? fn->size : LOAD_UNIT;
	/* Windows のファイル非同期処理は設計がへたくそなので使いたくない。
	   とりあえず同期で書いておいて別スレッドにすることで非同期にする
	   
	 */
	ReadFile( fn->hFile, fn->buf, size, &dwBytesRead, NULL );
	fn->buf = (void*)((u_char*)fn->buf + dwBytesRead);
	if( ( fn->size -= dwBytesRead ) <= 0 ){
		FS_LoadStop();
		return -1;
	}
	return fn->size;
}

int pcOpen( char *filename, int flag )
{
	return 0;
}

int pcClose( int fd )
{
	return 0;
}

int pcRead( int fd, void *buf, int nbyte )
{
	return 0;
}

int pcWrite( int fd, void *buf, int nbyte )
{
	return 0;
}

int pcLseek( int fd, unsigned int offset, int whence )
{
	return 0;
}

void FS_LoadFileRequest( int fileno, int offset, int size, void *buffer )
{
	while( cdbios_get_status() != 0 ){
		Sleep( 1 );
	}

	cdbios_read( buffer, FS_GET_FILE_POS( fileno ) + offset, size );
}

int  FS_LoadFileSync( void )
{
	return cdbios_get_status();
}

/* ----------------------------------------------------- */

void FS_HDInit( void )
{
	HDFILE *fn;

	fn = &finfo;
	fn->hFile = INVALID_HANDLE_VALUE;
	fn->buf = NULL;
}

#endif //KP_XBOX BP