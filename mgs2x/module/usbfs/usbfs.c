/*
	usb.c
		usbfs クライアント
		PCをサーバーにして,USB経由でファイルを読み込む。
	2000/05/01	K.Uehara
	$Id: usbfs.c,v 1.11 2002/09/10 02:05:56 usr01475 Exp $
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

#define STAT_CHECK	// 受信時にステータスチェックを行なう
#define PUTS_BUFFER	// PUTSをバッファリングする

#define DOPEN_SUPPORT	// dopen系関数のサポート
#define IO_DRV			// SCEのデバイスドライバとして登録する

#include <dirent.h>	// HDD

ModuleInfo Module = { "KCEJ_USB_FILESYSTEM", 0x0101 };

enum {
	CABLE_NO_CONNECT = 0,
	CABLE_TYPE_ELECOM,
	CABLE_TYPE_NEC,
};

typedef struct {
	int pipe;		// OUT pipe id
	int maxsize;
	void *nowp;		// 現在のバッファポインタ
	int last;		// 転送する残りの量
	int thread_id;	// 呼び出したスレッドのID
	int status;		// 終了原因
} UNIT;

typedef struct _trans_info {
	int cable_type;
	int sem_id;
	int func_sem;
	int evf_id;
	int c_pipe;
	int ifnum;
	int as;
	UNIT out;
	UNIT in;
#ifdef STAT_CHECK
	UNIT stat;
#endif
} TRANSINFO;

static TRANSINFO usbfs_work;

#define PROTOCOL_IN() 	WaitSema( usbfs_work.func_sem )
#define PROTOCOL_OUT()	SignalSema( usbfs_work.func_sem )

#define EVF_TRANS_END	0x0001

/* ---------------------------------------------------------------------- */
/*
	転送と転送コールバック
*/

static void data_transfer_done( UNIT *unit, int errorcode )
{
	unit->status = errorcode;	

	SetEventFlag( usbfs_work.evf_id, EVF_TRANS_END );
}

static void data_transfer( UNIT *unit );

static void data_transfer_callback( int result, int count, void *arg )
{
	UNIT *unit = arg;

	if( result != sceUsbd_NOERR ){
		printf( "data_transfer_done : %X\n", result );
		data_transfer_done( unit, result );

		return;
	}
	unit->nowp += count;
	unit->last -= count;
	if( unit->last <= 0 ){
		data_transfer_done( unit, result );
	} else {
		data_transfer( unit );
	}
}

static void data_transfer( UNIT *unit )
{
	int r;
	int size;

	if( unit->last > unit->maxsize ){
		size = unit->maxsize;
	} else {
		size = unit->last;
	}
	if( ( r = sceUsbdBulkTransfer( unit->pipe, unit->nowp, size
								   , data_transfer_callback
								   , unit ) ) != sceUsbd_NOERR ){
		printf( "DATA TRANSFER ERROR %X\n", r );
		data_transfer_done( unit, r );
	}
}

static void set_data_transfer( void )
{
	ClearEventFlag( usbfs_work.evf_id, ~EVF_TRANS_END );
}

#if 1
static void done_callback(  int result, int count, void *arg )
{
	SetEventFlag( usbfs_work.evf_id, EVF_TRANS_END );
}
#endif

static void wait_event_flag( void )
{
	/* イベントフラグ待ち */
	u_long evf;
	int stat;

	stat = WaitEventFlag( usbfs_work.evf_id, EVF_TRANS_END, EW_AND, &evf );
	if( stat != KE_OK ){
		printf( "evf err %d %lX\n", stat, evf );
	}
}

/* ---------------------------------------------------------------------- */
/*
	出力関数
*/

static int usb_io( UNIT *unit, char *buffer, int size )
{
	int stat;

	WaitSema( usbfs_work.sem_id );

	unit->nowp = buffer;
	unit->last = size;

	unit->thread_id = GetThreadId();

	set_data_transfer();
	data_transfer( unit );
	wait_event_flag();

	if( ( stat = unit->status ) != sceUsbd_NOERR ){
		printf( "usb_io error %d\n", unit->status );
	}
	SignalSema( usbfs_work.sem_id );

	return stat;
}

#ifdef STAT_CHECK

static int usb_stat( void )
{
	unsigned char buf[ 8 ];
	TRANSINFO *work = &usbfs_work;
	switch( work->cable_type ){
	  case CABLE_TYPE_ELECOM:
		usb_io( &( work->stat ), buf, 1 );
		return ( ( buf[ 0 ] & 0xF0 ) == 0xE0 );
//		return ( buf[ 0 ] & 1 );
	  case CABLE_TYPE_NEC:
		usb_io( &( usbfs_work.stat ), buf, 8 );
#if 0
		printf( "%02X %02X %02X %02X %02X %02X %02X %02X\n",
				buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7] );
#endif
		return ( ( buf[ 2 ] >> 4 ) > 0 ) ? 1 : 0 ;
	}
	return 1;
}

#endif

static int usb_send( void *buffer, int size )
{
	return usb_io( &( usbfs_work.out ), buffer, size );
}

static int usb_receive( void *buffer, int size )
{
	int res;
#if 0
#ifdef STAT_CHECK
	{
		int timeout = 0;

		while( ! usb_stat() ){
			DelayThread( 10000 );	// 0.01sec
			if( ++timeout > 1000 ){
				printf( "TIMEOUT\n" );
				return 0x1000;
			}
		}
	}
#endif
#endif
	res = usb_io( &( usbfs_work.in ), buffer, size );

	return res;
}

static int usb_receive_timeout( void *buffer, int size, int timeout_sec )
{
	int res;
#ifdef STAT_CHECK
	{
		int timeout = 0;

		while( ! usb_stat() ){
			DelayThread( 10000 );	// 0.01sec
			if( ++timeout > timeout_sec * 100 ){
				printf( "nTIMEOUT\n" );
				return 0x1000;
			}
		}
	}
#endif
	res = usb_io( &( usbfs_work.in ), buffer, size );

	return res;
}

/* ---------------------------------------------------------------------- */
/*
	USB セットアップ用関数
*/

static void setup_end( int result, int count, void *arg )
{
	TRANSINFO *info = arg;
/* セットアップ完了 */

	printf( "usbfs setup end res %d count %d\n", result, count );

	SignalSema( info->sem_id );
}

static int setup_unit_pipe( UNIT *unit, int dev_id, UsbEndpointDescriptor *edesc )
{
	if( ( unit->pipe = sceUsbdOpenPipeAligned( dev_id, edesc ) ) < 0 ){
		printf( "open pipe error\n" );
		return -1;
	}

	unit->maxsize = edesc->wMaxPacketSize0 + edesc->wMaxPacketSize1 * 256;
	return 0;
}

static void set_interface_done(int result, int count, void *arg)
{
	TRANSINFO *info;
	info = arg;

	if( result != sceUsbd_NOERR ){
		printf( "sceUsbdSetInterface %d", result);
	}
	setup_end( 0, 0, info );
}

static void set_config_done(int result, int count, void *arg)
{
	TRANSINFO *unit = arg;
	int r;

	if( result != sceUsbd_NOERR ){
		printf( "sceUsbdSetConfiguration %d\n", result );
	}

	if( ( r = sceUsbdSetInterface(unit->c_pipe, unit->ifnum, unit->as
								  , set_interface_done, unit) ) ){
		printf( "sceUsbdSetInterface %d\n", r );
	}
}

static int usb_attach( int dev_id )
{
	UsbConfigurationDescriptor *cdesc;
	UsbInterfaceDescriptor *idesc;
	UsbEndpointDescriptor *edesc;
	TRANSINFO *tinfo;
	int r;
	int i, as;

	tinfo = &usbfs_work;

	if( tinfo->cable_type == CABLE_TYPE_NEC ){
		as = 2;
	} else {
		as = 0;
	}

	cdesc = sceUsbdScanStaticDescriptor( dev_id, NULL, USB_DESCRIPTOR_TYPE_CONFIGURATION );
	if( cdesc == NULL ){
		return -1;
	}

	idesc = ( void * )cdesc;
	for( i = 0; i <= as; i++ ){
		idesc = sceUsbdScanStaticDescriptor( dev_id, idesc, USB_DESCRIPTOR_TYPE_INTERFACE );
		if( idesc == NULL ){
			return -1;
		}
	}

	edesc = sceUsbdScanStaticDescriptor( dev_id, idesc, USB_DESCRIPTOR_TYPE_ENDPOINT );
	if( edesc == NULL ){
		return -1;
	}

	tinfo->ifnum = idesc->bInterfaceNumber;
	tinfo->as = idesc->bAlternateSetting;

	if( ( tinfo->c_pipe = sceUsbdOpenPipe( dev_id, NULL ) ) < 0 ){
		return -1;
	}
	switch( tinfo->cable_type ){
	  case CABLE_TYPE_ELECOM:
#ifdef STAT_CHECK
		setup_unit_pipe( &tinfo->stat, dev_id, edesc + 0 );
#endif
		setup_unit_pipe( &tinfo->out, dev_id, edesc + 1 );
		setup_unit_pipe( &tinfo->in, dev_id, edesc + 2 );
		break;
	  case CABLE_TYPE_NEC:
#ifdef STAT_CHECK
		setup_unit_pipe( &tinfo->stat, dev_id, edesc + 0 );
#endif
		setup_unit_pipe( &tinfo->in, dev_id, edesc + 1 );
		setup_unit_pipe( &tinfo->out, dev_id, edesc + 2 );
		break;
	}
	tinfo->in.maxsize = 4 * 1024;	// 入力の転送サイズを大きめに設定
									// これがライブラリ(HARD?)のMAX 

	sceUsbdSetPrivateData( dev_id, tinfo );

	if( tinfo->cable_type == CABLE_TYPE_NEC ){
		if( ( r = sceUsbdSetConfiguration( tinfo->c_pipe, cdesc->bConfigurationValue
										   , set_config_done, tinfo ) ) != sceUsbd_NOERR ){
			printf( "sceUsbdSetConfiguration:%d\n", r );
			return -1;
		}
	} else if( tinfo->cable_type == CABLE_TYPE_ELECOM ){
		/* セットアップ完了 */
		sceUsbdControlTransfer( tinfo->c_pipe, 0x41, 3
								, 0x80, 0, 0, NULL, setup_end, tinfo );
	}

	printf("usbfs: attached\n" );

	return(0);
}

typedef struct {
	u_short vendor;
	u_short product;
	int type;
	char *name;
} CABLE_USB_INFO;

static CABLE_USB_INFO cables[] = {
	{ 0x067B, 0x0001, CABLE_TYPE_ELECOM, "ELECOM" },
	{ 0x067B, 0x0000, CABLE_TYPE_ELECOM, "ACTIS" },
	{ 0x0547, 0x2720, CABLE_TYPE_NEC, "NEC" },
	{ 0, 0, 0, NULL }
};

static int usb_probe(int dev_id)
{
	UsbDeviceDescriptor *ddesc;

//	dump_usb_desc( dev_id );

	if(NULL == (ddesc = sceUsbdScanStaticDescriptor(dev_id, NULL,
													USB_DESCRIPTOR_TYPE_DEVICE))){
		return 0;
	}

	if( usbfs_work.cable_type != CABLE_NO_CONNECT ){
		return 0;
	}

	/* リンクケーブルのチェック */
	{
		CABLE_USB_INFO *cp;
		for( cp = cables; cp->name != NULL; cp ++ ){
			if( ddesc->idVendor == cp->vendor && ddesc->idProduct == cp->product ){
				usbfs_work.cable_type = cp->type;
				printf( "%s LINK CABLE\n", cp->name );
				return 1;
			}
		}
	}

	return 0;
}

static int usb_detach( int dev_id )
{
	TRANSINFO *info;

	if(NULL == (info = sceUsbdGetPrivateData(dev_id))){
		return(-1);
	}

	WaitSema( info->sem_id );
	usbfs_work.cable_type = CABLE_NO_CONNECT;

	printf("usbfs: detached\n" );

	return(0);
}

static sceUsbdLddOps usb_ops = {
	NULL, NULL,
	"usbfs",
	usb_probe,
	usb_attach,
	usb_detach,
};

#define BASE_priority  32

int usbfs_init( void )
{
	int r;

	/* セマフォ確保 */
	{
		struct SemaParam sem;

		sem.attr = SA_THPRI;
		sem.initCount = 0;
		sem.maxCount = 1;
		sem.option = 0;
		if( ( usbfs_work.sem_id = CreateSema( &sem ) ) <= 0 ){
			printf( "semaphore failed\n" );
			return NO_RESIDENT_END;
		}
	}
	{
		struct SemaParam sem2;
		sem2.attr = SA_THPRI;
		sem2.initCount = 1;
		sem2.maxCount = 1;
		sem2.option = 0;
		if( ( usbfs_work.func_sem = CreateSema( &sem2 ) ) <= 0 ){
			printf( "semaphore2 failed\n" );
			return NO_RESIDENT_END;
		}
	}
	/* イベントフラグ確保 */
	{
		struct EventFlagParam evf;
		int id;

		evf.attr = EA_SINGLE;
		evf.initPattern = 0;
		evf.option = 0;
		id = CreateEventFlag( &evf );
		if( id < 0 ){
			printf( "usbfs: evf failed\n" );
			return NO_RESIDENT_END;
		}
		usbfs_work.evf_id = id;
	}
	
	usbfs_work.cable_type = CABLE_NO_CONNECT;

	if( ( r = sceUsbdRegisterLdd( &usb_ops ) ) != 0 ){
		printf("usbmouse: sceUsbdRegisterLdd -> 0x%x\n", r);
		return NO_RESIDENT_END;
	}

	return RESIDENT_END;
}

/* ---------------------------------------------------------------------- */
/*
	別のスレッドから呼び出されるサブルーチン
*/

#define USB_COMMAND_TAG	0x01

typedef struct {
	char buf[ 8 ];
} COMBUF __attribute__((aligned(4)));

static int get_ok( COMBUF *com )
{
	if( usb_receive( com, sizeof( COMBUF ) ) != sceUsbd_NOERR ){
		printf( "receive error\n" );
		return 0;
	}
	if( com->buf[ 0 ] != USB_COMMAND_TAG || com->buf[ 1 ] != 0x00 ){
		int i;
		printf( "command error\n" );
		for( i = 0; i < 8; i ++ ){
			printf( "%02X ", com->buf[ i ] );
		}
		printf( "\n" );
		return 0;
	}
	return 1;
}

static void send_command( COMBUF *com, int code, int param1, int param2, int param3 )
{
	char *buf = com->buf;

	buf[ 0 ] = USB_COMMAND_TAG;
	buf[ 1 ] = code;
	buf[ 2 ] = param1;
	buf[ 3 ] = param2;
	*( int * )&buf[ 4 ] = param3;

	usb_send( com, sizeof( COMBUF ) );
}

#if 1
// PL2302用関数

static void send_control( int reqtype, int req, int value )
{
	set_data_transfer();
	sceUsbdControlTransfer( usbfs_work.c_pipe, reqtype, req
						, value, 0, 0, NULL, done_callback, NULL );
	wait_event_flag();
}

static int get_status( void )
{
	u_char buf[ 4 ];
	usb_io( &usbfs_work.stat, buf, 1 );
	return buf[ 0 ];
}

#endif

static int pid = 1;

int usbInit( char *id )
{
	COMBUF com;
	char *buf = com.buf;
	int res;
#if 1
	if( usbfs_work.cable_type == CABLE_NO_CONNECT ){
		return -2;
	}
#endif

	printf( "usbinit\n" );

	PROTOCOL_IN();

	if( usbfs_work.cable_type == CABLE_TYPE_ELECOM ){
		// PL2302の場合、初期化時にリセットが必要
		send_control( 0x41, 1, 0x18 );
		send_control( 0x41, 3, 0x08 );

		while( ( get_status() & 0x10 ) != 0x10 ){
			;
		}
		send_control( 0x41, 1, 0x18 );

		for( ;; ){
			int stat;
			stat = get_status();
			if( ( ( stat & 0x18 ) == 0 ) && ( stat & 0x20 ) ){
				break;
			}
		}
	}
	buf[ 0 ] = USB_COMMAND_TAG;
	buf[ 1 ] = USBFS_INIT;

	strncpy( buf + 2, id, 5 );
	buf[ 2 ] = pid;

	res = 0;
	{
		int i;
		// コマンドパケットの前にSYNCパケットを8個入れる
		buf[ 2 ] = 0;
		for( i = 0; i < 8; i++ ){
			usb_send( buf, sizeof( COMBUF ) );
		}
		buf[ 2 ] = pid;
		usb_send( buf, sizeof( COMBUF ) );
	}
	pid ++;
	for( ;; ){
		if( usb_receive_timeout( buf, sizeof( COMBUF ), 240 ) != sceUsbd_NOERR || buf[ 1 ] ){
			printf( "usbInit:ERROR %d\n", buf[ 1 ] );
			res = -1;
			break;
		}
		// ID=0のパケットがダミーとしてくる。
		if( buf[ 2 ] == 0 ){
			continue;
		}
		if( buf[ 2 ] == pid - 1 ){
			break;
		}
		printf( "Wrong ID %d %d\n", buf[2], pid - 1 );
	}
printf( "INIT END %d\n", res );
	PROTOCOL_OUT();
	return res;
}

int usbOpen( char *filename, int mode )
{
	COMBUF com;
	char filebuf[ 128 ] __attribute__((aligned(4)));
	int len;
	char *p;
	int res;

	if( ( p = strchr( filename, ':' ) ) != NULL ){
		filename = p + 1;
	}

	len = strlen( filename ) + 1;

	PROTOCOL_IN();
	send_command( &com, USBFS_OPEN, mode, 0, len );

	if( !get_ok( &com ) ){
		return -1;
	}

	strcpy( filebuf, filename );
	usb_send( filebuf, len );
	res = get_ok( &com );
	PROTOCOL_OUT();
	if( !res ){
		return -1;
	}
	return com.buf[ 2 ];
}

int usbClose( int fd )
{
	COMBUF com;
	int res;

	PROTOCOL_IN();
	send_command( &com, USBFS_CLOSE, fd, 0, 0 );
	res = get_ok( &com );
	PROTOCOL_OUT();
	if( ! res ){
		return -1;
	}
	return 0;
}

int usbRead( int fd, void *buffer, int size )
{
	COMBUF com;
	int res;

printf( "read size = %d\n", size );
	PROTOCOL_IN();
	send_command( &com, USBFS_READ, fd, 0, size );

	res = get_ok( &com );
	if( ! res ){
		PROTOCOL_OUT();
		return -1;
	}
	size = *( int * )&com.buf[ 4 ];
	send_command( &com, USBFS_OK, 0, 0, 0 );

	if( usb_receive( buffer, size ) != sceUsbd_NOERR ){
		PROTOCOL_OUT();
		return -1;
	}
	send_command( &com, USBFS_OK, 0, 0, 0 );

	res = get_ok( &com );
	PROTOCOL_OUT();
	if( ! res ){
		return -1;
	}
	return size;
}

int usbReadSet( int fd, int size )
{
	COMBUF com;

	PROTOCOL_IN();
	send_command( &com, USBFS_READ, fd, 0, size );
	if( ! get_ok( &com ) ){
		PROTOCOL_OUT();
		return -1;
	}
	size = *( int * )&com.buf[ 4 ];
	send_command( &com, USBFS_OK, 0, 0, 0 );
	return size;
}

int usbReadSync( char *buffer, int size, int last )
{
	if( last < size ){
		size = last;
	}
	if( usb_receive( buffer, size ) != sceUsbd_NOERR ){
		PROTOCOL_OUT();
		return -1;
	}
	return size;
}

int usbReadEnd( void )
{
	COMBUF com;
	send_command( &com, USBFS_OK, 0, 0, 0 );
	if( ! get_ok( &com ) ){
		PROTOCOL_OUT();
		return -1;
	}
	PROTOCOL_OUT();
	return 0;
}

int usbWrite( int fd, void *buffer, int size )
{
	COMBUF com;

	PROTOCOL_IN();
	send_command( &com, USBFS_WRITE, fd, 0, size );
	if( ! get_ok( &com ) ){
		PROTOCOL_OUT();
		return -1;
	}
	usb_send( buffer, size );
	if( ! get_ok( &com ) ){
		PROTOCOL_OUT();
		return -1;
	}
	PROTOCOL_OUT();
	return size;
}

int usbLseek( int fd, int offset, int where )
{
	COMBUF com;

	PROTOCOL_IN();
	send_command( &com, USBFS_SEEK, fd, where, offset );
	if( ! get_ok( &com ) ){
		PROTOCOL_OUT();
		return -1;
	}
	PROTOCOL_OUT();
	offset = *( int * )&com.buf[ 4 ];
	return offset;
}

#ifdef DOPEN_SUPPORT
/*
	dopen系
*/

int usbDopen( char *filename )
{
	COMBUF com;
	char filebuf[ 64 ] __attribute__((aligned(4)));
	int len;
	char *p;
	int res;

	if( ( p = strchr( filename, ':' ) ) != NULL ){
		filename = p + 1;
	}

	len = strlen( filename ) + 1;

	PROTOCOL_IN();
	send_command( &com, USBFS_DOPEN, 0, 0, len );

	if( !get_ok( &com ) ){
		return -1;
	}

	strcpy( filebuf, filename );
	usb_send( filebuf, len );
	res = get_ok( &com );
	PROTOCOL_OUT();
	if( !res ){
		return -1;
	}
	return com.buf[ 2 ];
}

int usbDread( int fd, void *buf )
{
	COMBUF com;
	int size;
	int err = 0;
	
	size = sizeof( struct sce_dirent );

	PROTOCOL_IN();
	send_command( &com, USBFS_DREAD, 0, 0, size);
	if( !get_ok( &com ) ){
		err = -1;	// err
		goto END;
	}
	if( com.buf[ 2 ] == 0 ){
		err = 0;
		goto END;
	}
	if( usb_receive( buf, size ) != sceUsbd_NOERR ){
		err = -1;
		goto END;
	}
	err = 1;
END:
	PROTOCOL_OUT();

	return err;
}

int usbDclose( int fd )
{
	COMBUF com;
	int res;

	PROTOCOL_IN();
	send_command( &com, USBFS_DCLOSE, fd, 0, 0 );
	res = get_ok( &com );
	PROTOCOL_OUT();
	if( ! res ){
		return -1;
	}
	return 0;
}

int usbGetStat( char *filename, void *buf )
{
	COMBUF com;
	char filebuf[ 64 ] __attribute__((aligned(4)));
	int len;
	char *p;
	int res;
	int err = 0;

	if( ( p = strchr( filename, ':' ) ) != NULL ){
		filename = p + 1;
	}

	len = strlen( filename ) + 1;

	PROTOCOL_IN();
	send_command( &com, USBFS_GETSTAT, 0, 0, len );

	if( !get_ok( &com ) ){
		err = -1;
		goto END;
	}

	strcpy( filebuf, filename );
	usb_send( filebuf, len );
	res = get_ok( &com );
	if( !res ){
		err = -1;
		goto END;
	}

	if( usb_receive( buf, sizeof( struct sce_stat ) ) != sceUsbd_NOERR ){
		err = -1;
		goto END;
	}
	err = 0;

END:
	PROTOCOL_OUT();

	return err;
}

#endif

#ifdef PUTS_BUFFER		// PUTSにバッファ使用

#define PUTS_BUF_SIZE	2048
#define PUTS_TASK_PRIO	112
#define PUTS_STACK_SIZE	1024

static char puts_buf[ PUTS_BUF_SIZE ];
static int puts_p = 0;

static void usb_puts_task( void )
{
	for( ;; ){
		if( puts_p > 0 ){
			int len;
			COMBUF com;

			PROTOCOL_IN();
			len = strlen( puts_buf ) + 1;
			send_command( &com, USBFS_PUTS, 0, 0, len );
			usb_send( puts_buf, len );
			puts_p = 0;
			PROTOCOL_OUT();
		}
		DelayThread( 300000 );	// 0.3sec
	}
}

int usbPuts( char *mes )
{
	int len;

	len = strlen( mes ) + 1;
	if( puts_p + len >= PUTS_BUF_SIZE ){
		return 1;
	}
	PROTOCOL_IN();
	strcpy( &( puts_buf[ puts_p ] ), mes );
	puts_p += len - 1;
	PROTOCOL_OUT();

	return 0;
}

#else

int usbPuts( char *mes )
{
	int len;
	COMBUF com;

	len = strlen( mes ) + 1;
	PROTOCOL_IN();
	send_command( &com, USBFS_PUTS, 0, 0, len );
	usb_send( mes, len );
	PROTOCOL_OUT();

	return 0;
}

#endif

#ifdef IO_DRV

// SCEのAddDrvを使って実装してみる
#include "fs.h"

#define SET_FD( _io, _fd )	((_io)->i_private = ( void * )(_fd))
#define GET_FD( _io )	(( ( int )(_io)->i_private ) & 0xFF)
#define BURST_READ_FLAG		0x08000000
#define SET_BURST_READ( _io )	(( int )(_io)->i_private |= BURST_READ_FLAG)
#define RESET_BURST_READ( _io )	(( int )(_io)->i_private &= ~BURST_READ_FLAG)
#define IS_BURST_READ( _io )	(( int )(_io)->i_private & BURST_READ_FLAG)

static int dev_init( struct device_table *drv )
{
	printf( "dev_init\n" );

	return 0;
}

static int dev_exit( struct device_table *drv )
{
//	printf( "dev_exit\n" );
	return 0;
}

static int dev_open( struct iob *io, char *name, int flg )
{
	int fd;

	fd = usbOpen( name, flg );
	SET_FD( io, fd );

	return fd;
}

static int dev_close( struct iob *io )
{
	if( IS_BURST_READ( io ) ){
		RESET_BURST_READ( io );
		usbReadEnd();
	}
	return usbClose( GET_FD( io ) );
}

static int dev_read( struct iob *io, void *buf, int cnt )
{
	if( IS_BURST_READ( io ) ){
		return usbReadSync( buf, cnt, cnt );
	}
	return usbRead( GET_FD( io ), buf, cnt );
}

static int dev_write( struct iob *io, void *buf, int cnt )
{
	return usbWrite( GET_FD( io ), buf, cnt );
}

static int dev_lseek( struct iob *io, unsigned int offset, int how )
{
	if( IS_BURST_READ( io ) ){
		RESET_BURST_READ( io );
		usbReadEnd();
	}
	return usbLseek( GET_FD( io ), offset, how );
}

static int dev_ioctl( struct iob *io, int cmd, void *arg )
{
//printf( "IOCTL %X %d\n", cmd, *( int* )arg );
	if( cmd == ( ( 'u' << 8 ) | 1 ) ){
		SET_BURST_READ( io );
		return usbReadSet( GET_FD( io ), *( int* )arg );
	}
	if( cmd == ( ( 'u' << 8 ) | 2 ) ){
		RESET_BURST_READ( io );
		return usbReadEnd();
	}
	return 0;
}

static struct device_function dev_func = {
	df_init:  dev_init,
	df_exit:  dev_exit,
	df_open:  dev_open,
	df_close: dev_close,
	df_read:  dev_read,
	df_write: dev_write,
	df_lseek: dev_lseek,
	df_ioctl: dev_ioctl,
};

static struct device_table dev_tbl = {
	"usb",
	DTTYPE_CHAR | DTTYPE_FS,
	3,
	"usb",
	&dev_func
};

extern int AddDrv( struct device_table *dev_table );

static void DriverInit( void )
{
	int res;

	res = AddDrv( &dev_tbl );
	printf( "ADDDRV %d\n", res );
}

#endif


/* ---------------------------------------------------------------------- */
/*
	カーネルから呼び出される登録ルーチン
*/

int usbfs_start( void )
{
	int r;

    extern libhead usbfs_entry; /* ライブラリ名_entry を参照 */

    if( RegisterLibraryEntries(&usbfs_entry) != 0 ) {
		/* 既に同名の常駐ライブラリがいるので登録に失敗 */
		return NO_RESIDENT_END; /* 終了してメモリから退去 */
    }

	r = usbfs_init();
	if( r != RESIDENT_END ){
		return r;
	}

	if( init_rusbfs() == NO_RESIDENT_END ){
		return NO_RESIDENT_END;
	}
#ifdef PUTS_BUFFER
	{
		struct ThreadParam param;
		int id;

		param.attr = TH_C;
		param.entry = usb_puts_task;
		param.initPriority = PUTS_TASK_PRIO;
		param.stackSize = PUTS_STACK_SIZE;

		id = CreateThread( &param );

		if( id > 0 ){
			StartThread( id, 0 );
		}
	}
#endif
#ifdef IO_DRV
	DriverInit();
#endif
	printf( "KCEJ usb FILE SYSTEM DRIVER ver.0.01\n" );

	return RESIDENT_END;
}
