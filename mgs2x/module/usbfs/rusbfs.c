/*
	rusbfs.c
		usbfs EEからの通信を受け付ける
		モジュール自身がデバッグ用なので、結構いい加減

	2000/05/01 K.Uehara
	$Id: rusbfs.c,v 1.3 2001/02/14 01:20:41 usr01475 Exp $
*/

#include <stdio.h>
#include <kernel.h>
#include <memory.h>
#include <sif.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <string.h>

#include <usb.h>
#include <usbd.h>

#include "usbfs.h"
#include "usbfs_in.h"

#include "jsifman.h"

#define DOPEN_SUPPORT	// dopen系関数のサポート
#ifdef DOPEN_SUPPORT
#include <dirent.h>
#endif

//#define READSIZE	2048
#define READSIZE	4*1024

typedef struct {
	char buffer[ 2 ][ READSIZE ]  __attribute__((aligned(4)));
	int thread_id;
	int command;
	USBFS_PARAM param;
} WORK;

static WORK work  __attribute__((aligned(4)));

static void usbfs_callback( SIF_CALLBACK_PARAM *hd, void *data )
{
	WORK *w = data;

	// リエントラントではない

	w->command = hd->fno;

	switch( hd->fno ){
	  case USBFS_INIT:
		memcpy( w->buffer[ 0 ], hd->data, 4 );
		w->buffer[ 0 ][ 4 ] = 0;
		break;
	  case USBFS_OPEN:
	  case USBFS_DOPEN:
	  case USBFS_GETSTAT:
		memcpy( &( w->param ), hd->data, sizeof( USBFS_PARAM ) );
		strncpy( w->buffer[ 0 ], ( char * )hd->data + sizeof( USBFS_PARAM )
				 , w->param.p1 );
		break;
	  default:
		memcpy( &( w->param ), hd->data, sizeof( USBFS_PARAM ) );
		break;
	}
	iWakeupThread( w->thread_id );
}

static void rusbfs_thread( void )
{
	WORK *w = &work;

	sif_set_callback_func( IOP_SIF_USBFS, usbfs_callback, w );

	for( ;; ){
		int res = 0;

		SleepThread();

		switch( w->command ){
		  case USBFS_INIT:
			res = usbInit( w->buffer[ 0 ] );
			break;
		  case USBFS_OPEN:
			printf( "usbOpen %s %d\n", w->buffer[ 0 ], w->param.p2 );
			res = usbOpen( w->buffer[ 0 ], w->param.p2 );
			break;
		  case USBFS_CLOSE:
			res = usbClose( w->param.p1 );
			break;
		  case USBFS_READ:
			{
				char *buffer;
				unsigned int tr_id;
				int size;
				int fd;
				int last;
				int which;

				fd = w->param.p1;
				buffer = ( void * )w->param.p2;
				size = w->param.p3;

				last = usbReadSet( fd, size );
				res = last;
				if( last < 0 ){
					break;
				}

				which = 0;
				tr_id = 0;
				
				while( last > 0 ){
					char *readbuf;
					int readsize;

					readbuf = w->buffer[ which ];
					readsize = usbReadSync( readbuf, READSIZE, last );
					if( readsize < 0 ){
						res = -1;
						goto END;
					}
					last = last - readsize;
					readsize = ( readsize + 15 ) & ~15;
					if( tr_id > 0 ){
						while( sif_check_status( tr_id ) >= 0 ){
							printf( "delay..\n" );	
							DelayThread( 10000 );
						}
					}
					tr_id = sif_send_mem( buffer, readbuf, readsize );
					buffer += readsize;
					which = 1 - which;
				}
				if( usbReadEnd() < 0 ){
					res = -1;
				}
				while( sif_check_status( tr_id ) >= 0 ){
//					DelayThread( 10000 );
				}
			}
			break;
		  case USBFS_WRITE:
			{
				char *buffer;
				int size;
				int fd;
				int count;
				SIF_RV_QUEUE *rq;

				fd = w->param.p1;
				buffer = ( void * )w->param.p2;
				size = w->param.p3;
				count = 0;
				while( size > 0 ){
					int rs, _rs, cnt;
					rs = ( size > READSIZE ) ? READSIZE : size;
					_rs = ( rs + 15 ) & ~15;

					rq = sif_get_mem( w->buffer[ 0 ], buffer, _rs );
					while( !sif_rv_check_queue( rq ) );
					sif_rv_release_queue( rq );
					cnt = usbWrite( fd, w->buffer[ 0 ], rs );
					count += cnt;

					buffer += _rs;
					size -= rs;
				}
				res = count;
			}
			break;
#ifdef DOPEN_SUPPORT
		  case USBFS_DOPEN:
			printf( "usbDopen %s\n", w->buffer[ 0 ] );
			res = usbDopen( w->buffer[ 0 ] );
			break;
		  case USBFS_DREAD:
			{
				struct sce_dirent dirent;
				unsigned int tr_id;
				res = usbDread( w->param.p1, &dirent );
				tr_id = sif_send_mem( ( void * )w->param.p2, &dirent, sizeof( dirent ) );
				while( sif_check_status( tr_id ) >= 0 ){
//					DelayThread( 10000 );
				}
			}
			break;
		  case USBFS_DCLOSE:
			res = usbDclose( w->param.p1 );
			break;
		  case USBFS_GETSTAT:
			{
				struct sce_stat stat;
				unsigned int tr_id;
				res = usbGetStat( w->buffer[ 0 ], &stat );
				tr_id = sif_send_mem( ( void * )w->param.p2, &stat, sizeof( stat ) );
				while( sif_check_status( tr_id ) >= 0 ){
//					DelayThread( 10000 );
				}
			}
			break;
#endif
		  case USBFS_PUTS:
			{
				char *buffer;
				int size;
				SIF_RV_QUEUE *rq;

				buffer = ( void * )w->param.p2;
				size = w->param.p3;

				size = ( size + 15 ) & ~15;
				rq = sif_get_mem( w->buffer[ 0 ], buffer, size );
				while( !sif_rv_check_queue( rq ) ){
					;
				}
				sif_rv_release_queue( rq );
				usbPuts( w->buffer[ 0 ] );
				res = 0;
			}
			break;
		  case USBFS_SEEK:
			res = usbLseek( w->param.p1, w->param.p2, w->param.p3 );
			break;
		}
END:
		sif_send( EE_SIF_USBFS, res );
	}
}

int init_rusbfs( void )
{
	int th;
	struct ThreadParam param;

	param.attr = TH_C;
	param.entry = rusbfs_thread;
	param.initPriority = 85;
	param.stackSize = 2048;
	param.option = 0;
	th = CreateThread( &param );
	if( th > 0 ){
		printf( "start thread %X\n", th );
		work.thread_id = th;
		StartThread( th, 0 );
	} else {
		printf( "CreateThread Fault %X\n", th );
		return NO_RESIDENT_END;
	}
	return RESIDENT_END;
}
