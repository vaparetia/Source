/*
	cdbios_ee.c
		KCEJ CD/DVD MANAGER for EE
		2000/03/23 K.Uehara
	$Id: cdbios_ee.c,v 1.8 2001/09/19 02:44:46 usr01475 Exp $
*/

#include <stdio.h>
#include <eekernel.h>
#include <sif.h>
#include <sifcmd.h>
#include <libcdvd.h>

#include "jsifman.h"
#include "cdbios_in.h"
#include "cdbios.h"

typedef struct {
	CDBIOS_TASK_STATUS common;	// EE/IOPでの共有アドレス (IOP->EEのみ)
	void (*callback_func)( CDBIOS_READ_STATUS * );
} CDBIOS_WORK;

typedef struct {
	int param1;
	int param2;
	int param3;
	int param4;
} CDBIOS_PARAM;

static CDBIOS_WORK *nocache_work;
static CDBIOS_WORK work __attribute__((aligned(64)));

/* ---------------------------------------------------------------------- */
/*
	コールバックルーチン
*/

static void cdbios_callback( SIF_CALLBACK_PARAM *hd, void *data )
{
	CDBIOS_WORK *w = ( CDBIOS_WORK * )data;

	switch( hd->fno ){
	  case CDBIOS_EE_READ_CALLBACK:
		{
			if( w->callback_func != NULL ){
				( *w->callback_func )( &w->common.read );
			}
			i_sif_send_packet( IOP_SIF_CDBIOS, CDBIOS_COMMAND_READSYNC
							   , ( CDBIOS_TASK_STATUS * ) &w->common
							   , sizeof( CDBIOS_TASK_STATUS ) );
		}
		break;
	}
	ExitHandler();
}

/* ---------------------------------------------------------------------- */
/*
	IOPにパケットを送る
*/

static inline void send_iop( int command, void *data, int data_size )
{
	nocache_work->common.status |= CDBIOS_STATE_COMMAND_SET;
	sif_send_packet( IOP_SIF_CDBIOS, command, data, data_size );
}

#define SEND_IOP( _com, _data ) send_iop( _com, &(_data), sizeof(_data) )
#define SEND_COMMAND( _com )	send_iop( _com, NULL, 0 )

/* ---------------------------------------------------------------------- */
/*
	初期化ルーチン
*/

void cdbios_init( void )
{
	sif_init();
	sif_init_rv_man();

	nocache_work = ( CDBIOS_WORK * )( 0x20000000 | (int) &work );

	sif_set_callback_func( EE_SIF_CDBIOS, cdbios_callback, nocache_work );
	// IOP側の起動待ち
	while( ! sif_check_setup_module( IOP_SIF_CDBIOS ) ){
		;
	}

	// 初期化パケットを送る
	{
		int addr;
		addr = ( int )&( work.common );
		SEND_IOP( CDBIOS_COMMAND_INIT, addr );
		while( *( volatile int * )&nocache_work->common.status & CDBIOS_STATE_COMMAND_SET ){
			;
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	ステータス取得
*/

int cdbios_get_status( void )
{
	return nocache_work->common.status;
}

int cdbios_get_read_size( void )
{
	CDBIOS_READ_STATUS *rs;

	rs = &nocache_work->common.read;
	
	return rs->read_size;
}

/* ---------------------------------------------------------------------- */
/*
	コマンド発行
*/

void cdbios_callback_read( void *buffer, int pos, int size
						   , void (*callback)(CDBIOS_READ_STATUS *), int intr_size )
{
	CDBIOS_TASK_STATUS buf;
	CDBIOS_READ_STATUS *rs;

	rs = ( CDBIOS_READ_STATUS * )&buf.read;
	rs->pos = pos;
	rs->ee_buffer = buffer;
	rs->total_size = size;
	rs->read_size = 0;
	rs->intr_size = intr_size;
	nocache_work->callback_func = callback;

	nocache_work->common.read.read_size = 0;

	SEND_IOP( CDBIOS_COMMAND_READ, buf );
}

void cdbios_read( void *buffer, int pos, int size )
{
	cdbios_callback_read( buffer, pos, size, NULL, -1 );
}

void cdbios_stop( void )
{
	SEND_COMMAND( CDBIOS_COMMAND_STOP );
}

void cdbios_preseek( int pos )
{
	CDBIOS_PARAM p;
	p.param1 = pos;

	SEND_IOP( CDBIOS_COMMAND_SEEK, p );
}

int cdbios_get_time( sceCdCLOCK *rtc )
{
	if( cdbios_get_status() != 0 ){
		rtc->stat = 0xFF;
		return 0;
	}
	SEND_COMMAND( CDBIOS_COMMAND_GETTIME );
	while( cdbios_get_status() != 0 ){
		;
	}
	*rtc = *( sceCdCLOCK * )nocache_work->common.param;
	return 1;
}

int cdbios_get_filepos( char *name )
{
	send_iop( CDBIOS_COMMAND_SRCHFILE, name, ( strlen( name ) + 1 + 15 ) & ~15 );

	while( cdbios_get_status() != 0 ){
		;
	}
	return nocache_work->common.param[ 0 ];
}
