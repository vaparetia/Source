//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fshd.c
		ハードディスク読み書き用ファイルシステム

		ファイルは一度に１つしかオープンできない。

	1999/05/27 K.Uehara
	$Id: fshd.c,v 1.1.1.3 2002/11/19 11:42:39 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sifdev.h>

#include "libgv.h"
#include "libfs.h"

#include "cdbios.h"
#include "sd_ee.h"

//#define LOAD_UNIT	(8196*16*16)
#define LOAD_UNIT	(0x7FFFFFFF)

char const * FS_RegionFolders[] =
{
   "us",
   "jp",
   "eu"
};

enum fs_region FS_CurrentRegion = FS_REGION_US;

KP_CTASSERT( sizeof( FS_RegionFolders ) / sizeof( FS_RegionFolders[0] ) == FS_REGION_COUNT );

#if 0 //BP_GCC
static char current_dir[ 64 ] __attribute__((aligned(16)));
#else
static char current_dir[ 64 ];
#endif

typedef struct _HDFILE {
	int fd;
	int size;
	int unit;
	char *buf;
} HDFILE;

static HDFILE finfo;

#ifdef _USB_

#include "usbfs.h"

#if 1
#define sceOpen		usbOpen
#define sceRead		usbRead
#define sceWrite	usbWrite
#define sceClose	usbClose
#define sceLseek	usbLseek
#endif
#endif

int  pcOpen(char *filename, int flag)
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	return sceOpen( filename, flag );
#else
   return 0;
#endif
}

int  pcClose(int fd)
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	return sceClose( fd );
#else
   return 0;
#endif
}

int  pcRead(int fd, void *buf, int nbyte)
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	return sceRead( fd, buf, nbyte );
#else
   return 0;
#endif
}

int  pcWrite(int fd, void *buf, int nbyte)
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	return sceWrite( fd, buf, nbyte );
#else
   return 0;
#endif
}

int  pcLseek(int fd, unsigned int offset, int whence)
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	return sceLseek( fd, offset, whence );
#else
   return 0;
#endif
}

/* ----------------------------------------------------- */

void FS_ChangeDirectory( char *dir )
{
#ifdef _USB_
	sprintf( current_dir, "usb0:stage/%s/", dir );
#else
	sprintf( current_dir, "host0:stage/%s/", dir );
#endif
	SyncDCache( current_dir, current_dir + sizeof( current_dir ) );
	// サウンドにディレクトリを通知
	sd_set_cli( 0x80000000 | ( int )current_dir );
}

int FS_LoadRequest( char *name )
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	char full[ 128 ];
	HDFILE *fn;
	int fd;
	int size;

	fn = &finfo;
	ASSERT( fn->fd == -2 );	// 読み込み中ではないか

	sprintf( full, "%s%s", current_dir, name );
	if( ( fd = sceOpen( full, SCE_RDONLY ) ) < 0 ){
		printf( "FILE %s not found\n", full );
		return 0;
	}
	size = sceLseek( fd, 0, SEEK_END );
	sceLseek( fd, 0, SEEK_SET );

	fn->buf = NULL;
	fn->fd = fd;
printf( "Load : %s %d\n", full, size );
	return size;
#else
   return 0;
#endif
}

void FS_LoadSet( void *buf, int size )
{
	HDFILE *fn;
	fn = &finfo;

	ASSERT( fn->fd >= 0 );
printf( "LoadSet %X size %d\n", buf, size );
	fn->buf = buf;
	fn->unit = LOAD_UNIT;
	fn->size = size;
}

void FS_LoadStop( void )
{
   BP_TODO_BREAK;
#if 0 //BP_TODO

	HDFILE *fn;
	fn = &finfo;

	ASSERT( fn->fd >= 0 );

	sceClose( fn->fd );
	fn->fd = -2;
	fn->buf = NULL;
#endif
}

int FS_LoadSync( void )
{
   BP_TODO_BREAK;
#if 0 //BP_TODO
	HDFILE *fn;
	int size;

	fn = &finfo;

	ASSERT( fn->buf != NULL );

	size = ( fn->size < LOAD_UNIT ) ? fn->size : LOAD_UNIT;
	sceRead( fn->fd, fn->buf, size );
	fn->buf += size;
	if( ( fn->size -= size ) <= 0 ){
		FS_LoadStop();
		return -1;
	}
	return fn->size;
#else
   return 0;
#endif
}

/* ----------------------------------------------------- */

#ifdef _USB_
void FS_USBInit( void )
#else
void FS_HDInit( void )
#endif
{
	HDFILE *fn;

	fn = &finfo;
	fn->fd = -2;
	fn->buf = NULL;
}

/* ---------------------------------------------------------------------- */
/*
	他のファイル読み込み用
*/

#if 0

/* ファイルの実際の名前 */

char *fs_local_filename[ FS_FILE_MAX ] =
{
	[FS_FILE_CODEC] = "host0:./codec.dat",
	[FS_FILE_VOX] = "host0:./vox.dat",
	[FS_FILE_MOVIE] = "host0:./movie.dat",
	[FS_FILE_DEMO] = "host0:./demo.dat",
	[FS_FILE_FACE] = "host0:./face.dat"
};

void FS_LoadFileRequest( int fileno, int offset, int size, void *buffer )
{
	char full[ 64 ];
	char *name;
	int fd;
	HDFILE *fn;

	fn = &finfo;
	ASSERT( fn->fd == -2 );	// 読み込み中ではないか

	name = fs_local_filename[ fileno ];
	ASSERT( name != NULL );
	sprintf( full, "%s", name );

	if( ( fd = pcOpen( full, SCE_RDONLY ) ) < 0 ){
		printf( "%s not found\n", full );
		HANGUP();
	}
	pcLseek( fd, offset * FS_SECTOR_SIZE, SCE_SEEK_SET );

	fn->fd = fd;
	fn->buf = buffer;
	fn->size = size;
	fn->unit = LOAD_UNIT;
}

int  FS_LoadFileSync( void )
{
	return ( FS_LoadSync() > 0 ) ? 1 : 0;
}

#ifdef _USB_
void FS_UsbSetStreamFile( int file_id )
#else
void FS_SetStreamFile( int file_id )
#endif
{
	cdbios_set_hd_file( fs_local_filename[ file_id ] );
}

#else

void FS_LoadFileRequest( int fileno, int offset, int size, void *buffer, int bp_stream_top )
{
   cdbios_read( buffer,
#if 1 //BP_PS2
      FS_GetStreamTop( fileno, offset ),
#else
      FS_GET_FILE_POS( fileno ) + offset,
#endif
      size,
      bp_stream_top
      );
}

int  FS_LoadFileSync( void )
{
	return cdbios_get_status();
}

#endif
