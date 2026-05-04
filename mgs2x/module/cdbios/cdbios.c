/*
	cdbios.c
		KCEJ CD/DVD MANAGER for IOP module
		2000/03/21 K.Uehara
	$Id: cdbios.c,v 1.27 2002/10/08 01:06:46 usr01475 Exp $
*/

#include <stdio.h>
#include <sys/file.h>
#include <kernel.h>
#include <sif.h>
#include <sifcmd.h>
#include <libcdvd.h>
#include <string.h>

#include "jsifman.h"
#include "cdbios_in.h"
#include "cdbios.h"
#include "usbfs.h"

static char name[] = "KCEJ_DISC_BIOS";

ModuleInfo Module = { name , 0x0101 };

//#define TIMECHECK 1		// デバッグ時

#define EVENT_COMMAND	0x00000001
#define EVENT_READEND	0x00000040
#define EVENT_TIMEOUT	0x00000080
#define EVENT_STOP		0x00000020

#define CDBIOS_COMMAND_READ		0x0001

#define FALSE	0
#define TRUE	1

/* HDの設定 */

#define HD_MAX_FILENUM		32		// 1からはじまるので、最大数はこの数-1
#define HD_MAX_FILENAME		1024

/* モジュール内部ワーク */

#define CDBIOS_PRIO			82
#define CDBIOS_STACK_SIZE	2048

#define CDBIOS_ONE_BUFFER_SIZE	(32*1024)
#define SECTOR_SIZE				(2048)
#define SECTOR( _size )		( (_size+SECTOR_SIZE-1)/SECTOR_SIZE )

typedef struct {
	CDBIOS_TASK_STATUS common;	// EE/IOPでの共有アドレス (IOP->EEのみ)
	int force_stop_flag;
	int ee_common_address;		// commonのアドレス
	int task_id;
	int read_eflag_id;
	int com_eflag_id;
	int sem_id;
	int command;
	int which;
	unsigned int dual_top_pos;			// layer1 top pos
	sceCdRMode cdread_mode;
	void *buffer[ 2 ];
	char *hd_filename[ HD_MAX_FILENUM ];
		// hd_filename[ 0 ]はhd_filename_bufの空き領域を指す。
	char hd_filename_buf[ HD_MAX_FILENAME ];
} CDBIOS_WORK;

#define HD_FILENAME_OFS( _work )	( (( int *)( _work )->hd_filename)[ 0 ] )
#define HD_FILE_ID( _id )			( (_id) << 24 )

static CDBIOS_WORK work;

/* ---------------------------------------------------------------------- */
/*
	コールバック
*/

static void cdbios_callback( SIF_CALLBACK_PARAM *hd, void *data )
{
	CDBIOS_WORK *w = data;

	switch( hd->fno ){
	  case CDBIOS_COMMAND_INIT:
		w->ee_common_address = hd->data[ 0 ];
		Kprintf( "EE %X\n", w->ee_common_address );
		break;
	  case CDBIOS_COMMAND_READ:
	  case CDBIOS_COMMAND_READSYNC:
		{
			CDBIOS_TASK_STATUS *ts;
			ts = ( CDBIOS_TASK_STATUS * )hd->data;

			w->common.read = ts->read;
			if( hd->fno == CDBIOS_COMMAND_READ
				&& w->command == CDBIOS_COMMAND_SEEK ){
				/* 現在シーク中の場合は,次のリードの設定をして終了する */
				return;
			}
		}
		break;
	  case CDBIOS_COMMAND_SEEK:
		{
			w->common.seek_pos = ( ( int * )hd->data )[ 0 ];
			w->common.read.pos = 0;
		}
		break;
	  case CDBIOS_COMMAND_GETTIME:
		break;
	  case CDBIOS_COMMAND_STOP:
		w->force_stop_flag = TRUE;	// 停止フラグを立てる
		iSetEventFlag( w->read_eflag_id, EVENT_STOP );
		break;
#ifdef HD_EMU
	  case CDBIOS_HD_COMMAND_SET_FILE:
#endif // HD_EMU
	  case CDBIOS_COMMAND_SRCHFILE:
		sprintf( w->buffer[ 0 ], "%s", ( char * )hd->data );
		break;
	  default:
		return;
	}
	w->command = hd->fno;
	iSetEventFlag( w->com_eflag_id, EVENT_COMMAND );
}

/* ---------------------------------------------------------------------- */
/*
	メインルーチン
*/

/*
	ステータスのアップデート
*/

static void send_status( CDBIOS_WORK *w )
{
	sif_send_mem( ( void * )w->ee_common_address
				  , &( w->common ), sizeof( CDBIOS_TASK_STATUS ) );
}

static void set_status( CDBIOS_WORK *w, int status )
{
	w->common.status = status;
}

/*
	初期化
*/

static int setup_memory( CDBIOS_WORK *w )
{
	void *ptr;
	int intr;

	CpuSuspendIntr( &intr );
	ptr = AllocSysMemory( 0, CDBIOS_ONE_BUFFER_SIZE * 2, NULL );
	CpuResumeIntr( intr );

	if( ptr == NULL ){
		printf( "cdbios: NO MEMORY!!\n" );
		return FALSE;
	}
	w->buffer[ 0 ] = ptr;
	w->buffer[ 1 ] = ptr + CDBIOS_ONE_BUFFER_SIZE;

	return TRUE;
}

static int setup_eventflag( CDBIOS_WORK *w )
{
	int id;
	struct EventFlagParam param, param1;

	param.attr = EA_SINGLE;
	param.initPattern = 0;
	param.option = 0;

	id = CreateEventFlag( &param );
	if( id < 0 ){
		printf( "cdbios:error ev %d\n", id );
		return FALSE;
	}
	w->read_eflag_id = id;

	param1.attr = EA_SINGLE;
	param1.initPattern = 0;
	param1.option = 0;

	id = CreateEventFlag( &param1 );
	if( id < 0 ){
		printf( "cdbios:error ev %d\n", id );
		return FALSE;
	}
	w->com_eflag_id = id;

	
	return TRUE;
}

static int setup_sema( CDBIOS_WORK *w )
{
	struct SemaParam sem;

	sem.attr = SA_THPRI;
	sem.initCount = 1;
	sem.maxCount = 1;
	sem.option = 0;
	if( ( w->sem_id = CreateSema( &sem ) ) <= 0 ){
		printf( "semaphore failed\n" );
		return FALSE;
	}
	return TRUE;
}

/*
	各状況のステータスマシン
*/

static int wait_command( CDBIOS_WORK *w )
{
	u_long result;
	int res;

	res = WaitEventFlag( w->com_eflag_id, EVENT_COMMAND, EW_OR | EW_CLEAR, &result );

	if( res != KE_OK ){
		printf( "cdbios:WaitErr%d\n", res );
		return CDBIOS_COMMAND_WAIT;
	}
//printf( "COMMAND %d\n", w->command );

	return w->command;
}

static void cdread_callback( int code )
{
	if( code == SCECdFuncRead ){
		// read end
		iSetEventFlag( work.read_eflag_id, EVENT_READEND );
	} else {
		Kprintf( "CB:%d\n", code );
	}
}

static u_int timeout_callback( void *data )
{
	CDBIOS_WORK *w = data;

	iSetEventFlag( w->read_eflag_id, EVENT_TIMEOUT );

	return 0;
}

#define READ_TIMEOUT	0x100
#define READ_STOPCMD	0x101

static int wait_read_end( CDBIOS_WORK *w )
{
	// 読み終わるまで待つ
	int res;
	u_long result;
	struct SysClock clock;

	USec2SysClock( 10 * 1000000, &clock );	// 10 sec.

	// タイムアウト用のコールバックを設定
	res = SetAlarm( &clock, timeout_callback, w );
	if( res != KE_OK ){
		printf( "SetAlarm: %d\n", res );
	}
	// タイムアウトか、READENDが来たら終了

	res = WaitEventFlag( w->read_eflag_id, EVENT_READEND | EVENT_TIMEOUT | EVENT_STOP
						 , EW_OR | EW_CLEAR, &result );
	sceCdCallback( NULL );
	CancelAlarm( timeout_callback, w );

	if( result & EVENT_TIMEOUT ){
		printf( "timeout\n" );
		sceCdBreak();
		return READ_TIMEOUT;
	} else if( result & EVENT_STOP ){
		printf( "force stop\n" );
		sceCdBreak();
		return READ_STOPCMD;
	} else {
		return sceCdGetError();
	}
}

static int readkick( CDBIOS_WORK *w, unsigned int pos, int sector, void *buffer )
{
	w->cdread_mode.spindlctrl = ( pos < w->dual_top_pos ) ? SCECdSpinDvdDL0 : SCECdSpinNom;
	return sceCdRead( pos, sector, buffer, &w->cdread_mode );
}

static void read_start( CDBIOS_WORK *w, int pos, void *buffer, int size )
{
	int res;

	ClearEventFlag( w->read_eflag_id, 0 );
	sceCdCallback( cdread_callback );

	for( ;; ){
		if( w->force_stop_flag ){
			return;
		}
		res = readkick( w, pos, SECTOR( size ), buffer );
		if( res == 0 ){
			res = sceCdStatus();
			printf( "Read Start Err %x\n", res );
			set_status( w, CDBIOS_STATE_COMMAND_ERROR | ( res << 8 ) );
			send_status( w );
			DelayThread( 1000000 );	/* 1 sec wait */
		} else {
			break;
		}
	}
}

static int read_sync_ee( CDBIOS_WORK *w )
{
	int com;
	// パラメータは割り込みルーチンで変化する。
	// 割り込みがかかるまで待ち

	sif_send( EE_SIF_CDBIOS, CDBIOS_EE_READ_CALLBACK );

	com = wait_command( w );
	if( com != CDBIOS_COMMAND_READSYNC ){
		printf( "READSYNC ERR %d !!\n", com );
		return FALSE;
	}

	return TRUE;
}

static void force_stop( void )
{
	/* 中断する */
	sceCdBreak();
	sceCdCallback( NULL );
	printf( "break!\n" );
}

static int read_command( CDBIOS_WORK *w )
{
	volatile CDBIOS_READ_STATUS *rs;
	int which;
	void *tr_buf;
	int tr_size;
	int intr_mode;
	int read_size;
	int dma_id = 0;

#ifdef TIMECHECK
	struct SysClock tm1, tm2;
	int sec1, usec1;
	int sec2, usec2;
	int sec, usec;
	int allsize = 0;
#endif
	printf( "READ START %x\n", w->common.read.pos );

#ifdef TIMECHECK
	GetSystemTime( &tm1 );
#endif

	rs = &w->common.read;

	which = 0;
	tr_buf = NULL;
	tr_size = 0;
	intr_mode = 0;

	set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );

	read_size = rs->read_size;

	while( read_size < rs->total_size ){
		int res;
		int size;

		// ここでは、読み込みは終了
		if( rs->intr_size > 0 && rs->intr_size <= CDBIOS_ONE_BUFFER_SIZE ){
			// 割り込み待ちモード
			size = rs->intr_size;
			intr_mode = 1;
		} else {
			// 通常の読み込みモード
			size = rs->total_size - read_size;
			if( size > CDBIOS_ONE_BUFFER_SIZE ){
				size = CDBIOS_ONE_BUFFER_SIZE;
			}
		}
RETRY:
//printf( "read %d %d %X\n", rs->pos, size, w->buffer[ which ] );
		read_start( w, rs->pos, w->buffer[ which ], size );

#ifdef TIMECHECK
		allsize += size;
#endif

		if( w->force_stop_flag ){
			/* 中断する */
			force_stop();
			break;
		}

		if( tr_buf != NULL ){
			// 一つ前のバッファをDMA転送開始
			dma_id = sif_send_mem( rs->ee_buffer, tr_buf, tr_size );
			rs->ee_buffer += tr_size;
			rs->read_size = read_size;
			tr_buf = NULL;
			send_status( w );
		}

		if( ( res = wait_read_end( w ) ) != SCECdErNO ){
			printf( "Read Err %X\n", res );
			if( res != READ_STOPCMD ){
				set_status( w, CDBIOS_STATE_COMMAND_ERROR | res );
				send_status( w );
				DelayThread( 100000 );	/* 0.1 sec wait */
				goto RETRY;
			} else {
				break;
			}
		}

		// 読み込み終了/ステータス転送
		// エラー状態を解除
		set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );

		// 前回の転送が終っていなければ、そのまま待つ
		while( sif_check_status( dma_id ) >= 0 ){
			;
		}
		dma_id = 0;

		tr_buf = w->buffer[ which ];
		tr_size = size;
		which = 1 - which;

		read_size += size;
		if( rs->intr_size > 0 ){
			rs->intr_size -= size;
		}
		rs->pos += SECTOR( size );

		send_status( w );

		if( intr_mode || read_size == rs->total_size ){
			// 割り込みモード、または読み込み終了の時は、EEと通信して
			// パラメータを変化させる。
			if( tr_buf != NULL ){
				// この場合は向うにコマンドを送る前にデータを送信している
				dma_id = sif_send_mem( rs->ee_buffer, tr_buf, tr_size );
				rs->ee_buffer += tr_size;
				rs->read_size = read_size;
				tr_buf = NULL;
				send_status( w );
			}
			if( !read_sync_ee( w ) ){
				/* 強制終了 */
				force_stop();
				break;
			}
			read_size = rs->read_size;
//printf( "readsize %d total %d\n", rs->read_size, rs->total_size );
			intr_mode = 0;
		}
	}

	// 前回の転送が終っていなければ、そのまま待つ
	if( dma_id != 0 ){
		while( sif_check_status( dma_id ) >= 0 ){
			;
		}
	}

#ifdef TIMECHECK
	GetSystemTime( &tm2 );

	SysClock2USec( &tm1, &sec1, &usec1 );
	SysClock2USec( &tm2, &sec2, &usec2 );

	sec = sec2 - sec1;
	usec = usec2 - usec1;
	if( usec < 0 ){
		sec --;
		usec += 1000000;
	}
	printf( "SEC %d USEC %06d\n", sec, usec );
	if( sec > 0 ){
		printf( "all %d %dbytes/sec\n", allsize
				, (allsize*100)/(sec*100+(usec/10000)));
	}
#endif

	printf( "READ END\n" );
	return CDBIOS_COMMAND_WAIT;
}


// ストリーミング用HDエミュレーションを行なう
static int hd_read_command( CDBIOS_WORK *w )
{
	volatile CDBIOS_READ_STATUS *rs;
	int which;
	int intr_mode;
	int read_size;
	int fd;

	rs = &w->common.read;

printf( "HD %X %d\n", rs->pos, rs->total_size );

	which = 0;
	intr_mode = 0;

	fd = open( w->hd_filename[ rs->pos >> 24 ], O_RDONLY );
	if( fd < 0 ){
		printf( "HD FILE OPEN ERROR!!\n" );
		return CDBIOS_COMMAND_WAIT;
	}

	read_size = rs->read_size;

	set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );
	send_status( w );
	while( read_size < rs->total_size ){
		int size;
		int dma_id;
		int err;

		if( rs->intr_size > 0 && rs->intr_size <= CDBIOS_ONE_BUFFER_SIZE ){
			// 割り込み待ちモード
			size = rs->intr_size;
			intr_mode = 1;
		} else {
			// 通常の割り込みモード
			size = rs->total_size - read_size;
			if( size > CDBIOS_ONE_BUFFER_SIZE ){
				size = CDBIOS_ONE_BUFFER_SIZE;
			}
		}

		if( w->force_stop_flag ){
			// 強制
			break;
		}

		err = lseek( fd, ( rs->pos & 0x00FFFFFF ) * 2048, SEEK_SET );
		if( err < 0 || ( err = read( fd, w->buffer[ 0 ], size ) ) < 0 ){
printf( "err = %d %d %d\n", ( rs->pos & 0x00FFFFFF ), size, err );
			set_status( w, CDBIOS_STATE_COMMAND_ERROR | -err );
			send_status( w );
			DelayThread( 100000 );	/* 0.1 sec wait */
			continue;
		}
		dma_id = sif_send_mem( rs->ee_buffer, w->buffer[ 0 ], size );
		rs->ee_buffer += size;
		read_size += size;
		rs->read_size = read_size;
		if( rs->intr_size > 0 ){
			rs->intr_size -= size;
		}
		rs->pos += SECTOR( size );

		while( sif_check_status( dma_id ) >= 0 ){
			;
		}
		set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );
		send_status( w );

		if( err < size ){
			// 終端
			read_size = rs->total_size;
		}

		if( intr_mode || read_size == rs->total_size ){
			// 割り込みモードまたは読み込み終了時
			if( !read_sync_ee( w ) ){
				/* 強制終了 */
				break;
			}
		}
		intr_mode = 0;
	}
	close( fd );
printf( "HD read end\n ");
	return CDBIOS_COMMAND_WAIT;
}

static int seek_command( CDBIOS_WORK *w )
{
	// 先読みバッファを使いたいのでreadで１セクタのみ読み込む。
	// エラーが出ても気にしない

	int pos;
	int res;

	pos = w->common.seek_pos;
printf( "seek %d\n", pos );

	set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );
	send_status( w );

	ClearEventFlag( w->read_eflag_id, 0 );
	sceCdCallback( cdread_callback );
	res = readkick( w, pos - 1, 1, w->buffer[ 0 ] );
	if( res != 0 ){
		if( ( res = wait_read_end( w ) ) != SCECdErNO ){
			printf( "seek:Read Err %d\n", res );
		}
	} else {
		printf( "seek:Read StartError\n" );
	}
	sceCdCallback( NULL );

printf( "seek end\n" );
	w->command = CDBIOS_COMMAND_WAIT;
	return CDBIOS_COMMAND_WAIT;
}

static void cdbios_thread( int param )
{
	CDBIOS_WORK *w;
	int status;

	w = ( CDBIOS_WORK * )param;

	sceCdInit( SCECdINIT );
	sceCdMmode( w->command );

	if( w->dual_top_pos != 0 ){
		// 2層DVD対応
		int ondual;
		unsigned int layer1_start;

		while( sceCdReadDvdDualInfo( &ondual, &layer1_start ) == 0 ){
			printf( "DualInfo Error %X\n", sceCdGetError() );
		}
		if( ondual ){
			w->dual_top_pos = layer1_start;
		}
	}

	WaitSema( w->sem_id );

	w->cdread_mode.trycount = 0;
	w->cdread_mode.spindlctrl = SCECdSpinNom;
	w->cdread_mode.datapattern = SCECdSecS2048;

	sif_set_callback_func( IOP_SIF_CDBIOS, cdbios_callback, w );

	while( w->ee_common_address == ( int )NULL ){
		wait_command( w );
	}

	status = CDBIOS_COMMAND_WAIT;
	for( ;; ){
		switch( status ){
		  case CDBIOS_COMMAND_WAIT:
			/* wait 状態の時のみセマフォを解放する */
			SignalSema( w->sem_id );

			set_status( w, CDBIOS_STATE_IDLE );
			send_status( w );
			w->force_stop_flag = FALSE;
			status = wait_command( w );

			WaitSema( w->sem_id );
			set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );
			send_status( w );
			break;
		  case CDBIOS_COMMAND_READ:
			if( ( w->common.read.pos & 0xFF000000 ) != 0 ){
				// HD MODE
				status = hd_read_command( w );
			} else {
				// CD MODE
				status = read_command( w );
			}
			break;
		  case CDBIOS_COMMAND_SEEK:
			if( ( w->common.read.pos & 0xFF000000 ) != 0 ){
				// HD MODE
				status = CDBIOS_COMMAND_WAIT;
				break;
			}
			status = seek_command( w );
			if( w->common.read.pos != 0 ){
				// seek中にread要求がきた
				status = CDBIOS_COMMAND_READ;
			}
			break;
		  case CDBIOS_COMMAND_GETTIME:
			while( sceCdReadClock( ( sceCdCLOCK * )w->common.param ) == 0 ){
				DelayThread( 500000 );	/* 500 msec wait */
			}
			status = CDBIOS_COMMAND_WAIT;
			break;
		  case CDBIOS_COMMAND_SRCHFILE:
			{
				char *input = w->buffer[ 0 ];
				
				if( input[ 0 ] == '\\' || strncmp( input, "cdrom", 5 ) == 0 ){
					// CD FILE
					sceCdlFILE fd;
					char *filename;
					int layer;
					layer = input[ 5 ] - '0';
					if( input[ 0 ] == '\\' ){
						filename = input;
					} else {
						filename = input + 7;
					}
					printf( "SRCHFILE '%s'\n", filename );
					if( sceCdLayerSearchFile( &fd, filename, layer ) == 0 ){
						fd.lsn = -1;
					}
					w->common.param[ 0 ] = fd.lsn;
					printf( "RETURN %d\n", fd.lsn );
				} else {
					// HD FILE
					int i;
					w->common.param[ 0 ] = -1;
					for( i = 1; i < HD_MAX_FILENUM; i++ ){
						if( w->hd_filename[ i ] != NULL ){
							if( strcmp( w->hd_filename[ i ], input ) == 0 ){
								// すでに設定済み
								w->common.param[ 0 ] = HD_FILE_ID( i );
								break;
							}
						} else {
							int len;
							len = strlen( input ) + 1;
							if( HD_FILENAME_OFS( w ) + len >= HD_MAX_FILENAME ){
								printf( "FILENAME BUFFER OVER !!\n" );
							} else {
								int ofs;
								ofs = HD_FILENAME_OFS( w );
								w->hd_filename[ i ] = &( w->hd_filename_buf[ ofs ] );
								strcpy( w->hd_filename_buf + ofs, input );
								HD_FILENAME_OFS( w ) = ofs + len;
								w->common.param[ 0 ] = HD_FILE_ID( i );
							}
							break;
						}
					}
				}
			}
			status = CDBIOS_COMMAND_WAIT;
		    break;
		  default:
			status = CDBIOS_COMMAND_WAIT;
			break;
		}
		w->common.exec_com = status;
		send_status( w );
	}
}

/* ---------------------------------------------------------------------- */
/*
	モジュール初期化
*/

int cdbios_start( int argc, char *argv[] )
{
    extern libhead cdbios_entry; /* ライブラリ名_entry を参照 */

    if( RegisterLibraryEntries(&cdbios_entry) != 0 ) {
		/* 既に同名の常駐ライブラリがいるので登録に失敗 */
		return NO_RESIDENT_END; /* 終了してメモリから退去 */
    }

	memset( &work, 0, sizeof( work ) );

	work.command = SCECdCD;
	if( argc > 1 ){
		if( strcmp( argv[ 1 ], "DVD" ) == 0 ){
			work.command = SCECdDVD;
		}
		if( strcmp( argv[ 1 ], "DVD2" ) == 0 ){
			work.command = SCECdDVD;
			work.dual_top_pos = 1;		// !0
		}
		printf( "KCEJ %s MANAGER ver0.01\n", argv[ 1 ] );
	}

	if( !setup_memory( &work ) ){
		printf( "cdbios: no memory\n" );
		return NO_RESIDENT_END;
	}
	{
		struct ThreadParam param;
		int id;

		param.attr = TH_C;
		param.entry = cdbios_thread;
		param.initPriority = CDBIOS_PRIO;
		param.stackSize = CDBIOS_STACK_SIZE;

		id = CreateThread( &param );
		if( id > 0 ){
			work.task_id = id;

			if( !setup_eventflag( &work ) ){
				return NO_RESIDENT_END;
			}
			if( !setup_sema( &work ) ){
				return NO_RESIDENT_END;
			}

			StartThread( id, ( unsigned int )&work );
			return RESIDENT_END;
		} else {
			return NO_RESIDENT_END;
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	IOP側のモジュールから呼び出される関数
*/

void cdRead( int pos, void *buffer, int size )
{
	CDBIOS_WORK *w = &work;
	int res;

	if( size == 0 ) return;

	set_status( w, CDBIOS_STATE_COMMAND_IOP );

	if( ( pos & 0xFF000000 ) != 0 ){
		// HDD MODE
		int fd;
		fd = open( w->hd_filename[ pos >> 24 ], O_RDONLY );
		if( fd < 0 ){
			printf( "HD FILE OPEN ERROR!!\n" );
		}
		lseek( fd, ( pos & 0x00FFFFFF ) * 2048, SEEK_SET );
		read( fd, buffer, size );

		close( fd );
	} else {
		// CD MODE
RETRY:
		read_start( w, pos, buffer, size );
		if( ( res = wait_read_end( w ) ) != SCECdErNO ){
			printf( "ReadErr %x %x %d\n", res, pos, size );
			if( res != READ_STOPCMD ){
				set_status( w, CDBIOS_STATE_COMMAND_ERROR | res );
				send_status( w );
				goto RETRY;
			}
		}
	}
	set_status( w, CDBIOS_STATE_IDLE );
	send_status( w );
}

void cdOpen( void )
{
	WaitSema( work.sem_id );
}

void cdClose( void )
{
	SignalSema( work.sem_id );
}

void *cdGetBuffer( void )
{
	return work.buffer[ 0 ];
}
