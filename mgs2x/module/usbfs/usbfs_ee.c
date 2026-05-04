/*
	usbfs_ee.c
		USBFS EE用ライブラリ
		IOPのモジュールとの通信を行なう
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <eekernel.h>
#include <sif.h>
#include <sifcmd.h>
#include <sifdev.h>

#include "jsifman.h"
#include "usbfs.h"
#include "usbfs_in.h"

#define DOPEN_SUPPORT	// dopen系関数のサポート

typedef struct {
	int sem_id;
	int res;
} USBFS_WORK;

static USBFS_WORK work __attribute__((aligned(64))) = { 0 };

static void usbfs_callback( SIF_CALLBACK_PARAM *hd, void *data )
{
	USBFS_WORK *w = ( USBFS_WORK * )data;

	w->res = hd->fno;
	iSignalSema( w->sem_id );
}

#define SEND_IOP( _com, _packet ) \
	sif_send_packet( IOP_SIF_USBFS, (_com), &(_packet), sizeof( _packet ) );

static int wait_res( void )
{
	WaitSema( work.sem_id );
	return work.res;
}

int usbInit( char *id )
{
	struct {
		char buf[ 8 ];
	} p;

	if( work.sem_id == 0 ){
		sif_set_callback_func( EE_SIF_USBFS, usbfs_callback, &work );
		while( ! sif_check_setup_module( IOP_SIF_USBFS ) ){
			;
		}

		{
			/* セマフォ作成 */
			struct SemaParam sema;

			sema.initCount = 0;
			sema.maxCount = 1;
			sema.option = 0;
			work.sem_id = CreateSema( &sema );
		}
	}

	strncpy( p.buf, id, 5 );
	p.buf[ 4 ] = '\0';

	SEND_IOP( USBFS_INIT, p );

	return wait_res();
}

int usbOpen( char *filename, int mode )
{
	struct {
		USBFS_PARAM param;
		char buf[ 48 ];
	} p;
	p.param.p1 = strlen( filename ) + 1;
	p.param.p2 = mode;
	strcpy( p.buf, filename );

	SEND_IOP( USBFS_OPEN, p );
	
	return wait_res();
}

int usbClose( int fd )
{
	USBFS_PARAM p;

	p.p1 = fd;

	SEND_IOP( USBFS_CLOSE, p );

	return wait_res();
}

int usbRead( int fd, void *buffer, int size )
{
	USBFS_PARAM p;

	p.p1 = fd;
	p.p2 = ( int )buffer;
	p.p3 = size;

	SyncDCache( buffer, buffer + size );
	InvalidDCache( buffer, buffer + size );

	SEND_IOP( USBFS_READ, p );

	return wait_res();
}

int usbWrite( int fd, void *buffer, int size )
{
	USBFS_PARAM p;

	p.p1 = fd;
	p.p2 = ( int )buffer;
	p.p3 = size;

	SyncDCache( buffer, buffer + size );

	SEND_IOP( USBFS_WRITE, p );

	return wait_res();
}

int usbLseek( int fd, int offset, int where )
{
	USBFS_PARAM p;

	p.p1 = fd;
	p.p2 = offset;
	p.p3 = where;

	SEND_IOP( USBFS_SEEK, p );

	return wait_res();
}

#ifdef DOPEN_SUPPORT

int usbDopen( char *filename )
{
	struct {
		USBFS_PARAM param;
		char buf[ 48 ];
	} p;
	p.param.p1 = strlen( filename ) + 1;
	p.param.p2 = 0;
	strcpy( p.buf, filename );

	SEND_IOP( USBFS_DOPEN, p );
	
	return wait_res();
}

int usbDread( int fd, void *buffer )
{
	USBFS_PARAM p;
	int size;

	p.p1 = fd;
	p.p2 = ( int )buffer;
	size = sizeof( struct sce_dirent );

	SyncDCache( buffer, buffer + size );
	InvalidDCache( buffer, buffer + size );
	
	SEND_IOP( USBFS_DREAD, p );

	return wait_res();
}

int usbDclose( int fd )
{
	USBFS_PARAM p;

	p.p1 = fd;

	SEND_IOP( USBFS_DCLOSE, p );

	return wait_res();
}

int usbGetStat( char *filename, void *buffer )
{
	struct {
		USBFS_PARAM param;
		char buf[ 48 ];
	} p;
	int size = sizeof( struct sce_stat );

	p.param.p1 = strlen( filename ) + 1;
	p.param.p2 = ( int )buffer;
	strcpy( p.buf, filename );

	SyncDCache( buffer, buffer + size );
	InvalidDCache( buffer, buffer + size );
	
	SEND_IOP( USBFS_GETSTAT, p );

	return wait_res();
}

#endif

int usbPuts( char *mes )
{
	USBFS_PARAM p;
	char buf[ 128 ] __attribute__((aligned(16)));
	int len;

	{
		int stat;
		asm volatile( "mfc0 %0,$12" : "=r"(stat): );
		if( !( stat & 0x00010000 ) ){
			return 0;
		}
	}

	len = strlen( mes ) + 1;
	if( len >= 128 ){
		return 0;
	}
	strcpy( buf, mes );

	SyncDCache( buf, buf + len );

	p.p2 = ( int )buf;
	p.p3 = len;
	SEND_IOP( USBFS_PUTS, p );

	return wait_res();
}

int usbPrintf( char *format, ... )
{
	int res;
	va_list args;
	char buf[ 128 ];

	va_start( args, format );

	vsprintf( buf, format, args );
	res = usbPuts( buf );

	va_end( args );
	return res;
}
