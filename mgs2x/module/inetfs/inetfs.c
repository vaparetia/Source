/*
	inetfs.c
		inetライブラリを用いてfsを構築
	2001/02/05	K.Uehara
	$Id: inetfs.c,v 1.8 2002/09/10 02:05:38 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <kernel.h>
#include <sif.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <inet/inet.h>
#include <inet/inetctl.h>

#include <libcdvd.h>

#include "../usbfs/fs.h"

#ifndef sceInetCC_SetAddress  // ver2.2.2以降

#define sceInetCC_SetAddress		0x00000109
#define sceInetCC_SetNetmask		0x0000010a
#define sceInetCC_SetBroadcast		0x0000010b
#define sceInetCC_SetMTU		0x0000010c
#define sceInetCC_SetDHCPHostName	0x0000010e
#define sceInetCC_FlagSetUp		0x00010000	/* Up=1 */
#define sceInetCC_FlagClearUp		0x00010001	/* Up=0 (Down) */
#define sceInetCC_FlagSetBroadcast	0x00010002	/* Broadcast=0 */
#define sceInetCC_FlagClearBroadcast	0x00010003	/* Broadcast=1 */
#define sceInetCC_FlagSetDHCP		0x00010004	/* DHCP=1 */
#define sceInetCC_FlagClearDHCP		0x00010005	/* DHCP=0 */
#define sceInetCC_DHCPSetRelOnStop	0x00011000	/* ReleaseOnStop=1 */
#define sceInetCC_DHCPClearRelOnStop	0x00011001	/* ReleaseOnStop=0 */

#endif

/*
	USB-Etherドライバ an986.irx, HDD-Ether smap.irx に対応。
*/

#define DEFAULT_SERVER_NAME	"taira.konami"
#define TIMEOUT_TIME	20000

static int lock_sema;
static sceInetParam_t inetparam;
static int dev_id = 0;

#define PROT_IN()		WaitSema( lock_sema )
#define PROT_OUT()		SignalSema( lock_sema )

static char server_name[ 32 ] = DEFAULT_SERVER_NAME;
static int portnum = 0xC000 + 123;

typedef struct {
	int uid;
	int gid;
	char path[ 128 ];
} HOSTPARAM;

static HOSTPARAM hostparam = {
	99,
	99,
	"."
};

/* ---------------------------------------------------------------------- */
/*
	ファイルシステム
		Openでソケットを作成/コネクション作成/コネクションを開く
		Closeでコネクションを閉じ,ソケットを廃棄
*/

typedef struct {
	char command;
	char param1;
	char param2;
	char param3;
	int	 lparam;
} COM_HEADER;

enum {
	COM_SETPARAM = 0,
	COM_OPEN = 1,
	COM_CLOSE,
	COM_READ,
	COM_WRITE,
	COM_SEEK,
	COM_GETSTAT,
};

static int send( int cid, void *buf, int len )
{
	int r;
	int flags = 0;

	r = sceInetSend( cid, buf, len, &flags, TIMEOUT_TIME );
	if( r < 0 ){
		printf( "send err %d\n", r );
	}
	return r;
}

static int recv( int cid, void *buf, int len )
{
	int flags;
	int r = 0;
	int l = len;

	while( l > 0 ){
		flags = 0;
		r = sceInetRecv( cid, buf, l, &flags, TIMEOUT_TIME );
		if( flags & sceINETF_TIM ){
			// TIMEOUT
			return -1;
		}
		if( r <= 0 ){
			return -r;
		}
		l -= r;
		buf += r;
	}
	return len - r;
}

static int open_connection( void )
{
	int r, cid;

//	printf( "open connection\n" );
	r = 0;
	for( ;; ){
		if( ( cid = sceInetCreate( &inetparam ) ) < 0 ){
			printf( "create err %d\n", cid );
			r = -1;
		}
		if( ( r = sceInetOpen( cid, TIMEOUT_TIME ) ) < 0 ){
			printf( "inetopen %d\n", r );
			if( r == sceINETE_CONNECTION_RESET ){
				DelayThread( 1000000 );
				continue;
			}
			sceInetClose( cid, -1 );
			if( r != sceINETE_TIMEOUT ){
				cid = -1;
				r = -1;
				break;
			}
		}
		break;
	}
	{
		COM_HEADER com;
		com.command = COM_SETPARAM;
		com.lparam = sizeof( HOSTPARAM );

		send( cid, &com, sizeof( com ) );
		send( cid, &hostparam, com.lparam );
	}
printf( "open_connection %d\n", cid );
	return cid;
}

static int close_connection( int cid )
{
	return sceInetClose( cid, -1 );
}

static int set_server( void )
{
	sceInetParam_t *ip = &inetparam;
	char buf[ 16 ];
	int r;

	memset( ip, 0, sizeof( sceInetParam_t ) );

	ip->type = sceINETT_CONNECT;
	ip->local_port = sceINETP_AUTO;

	r = sceInetName2Address( 0, &ip->remote_addr, server_name, TIMEOUT_TIME, 16 );
	if( r != sceINETE_OK ){
		printf( "name2address %d\n", r );
		return -1;
	}

	sceInetAddress2String( buf, 16, &ip->remote_addr );
	printf( "%s %s : %d\n", server_name, buf, portnum );
	ip->remote_port = portnum;

	return 0;
}

/* ---------------------------------------------------------------------- */
/*
	device function
*/

static int dev_init( struct device_table *drv )
{
	printf( "net_init\n" );

	return 0;
}

static int dev_exit( struct device_table *drv )
{
//	printf( "net_exit\n" );
	return 0;
}

static int dev_open( struct iob *io, char *name, int flg )
{
	int cid, r;

	PROT_IN();

	r = cid = open_connection();

	if( r < 0 ){
		printf( "fail connection\n" );
	} else {
		printf( "open file %s %d\n", name, flg );
	}

	io->i_private = ( void * )cid;

	if( cid > 0 ){
		COM_HEADER com;

		com.command = COM_OPEN;
		com.param1 = strlen( name ) + 1;
		com.param2 = flg;
		com.param3 = 0;
		com.lparam = 0;

		send( cid, &com, sizeof( com ) );
		send( cid, name, com.param1 );

		recv( cid, &com, sizeof( com ) );
		if( com.command != 0 || com.lparam < 0 ){
			// ファイルのオープンに失敗した
			printf( "file open err %d\n", com.lparam );
			sceInetClose( cid, -1 );
		}
		r = com.lparam;
	}
	PROT_OUT();

	return r;
}

static int dev_close( struct iob *io )
{
	int cid = ( int )io->i_private;
	int r;

	PROT_IN();

	{
		COM_HEADER com;
		com.command = COM_CLOSE;
		com.param1 = 0;
		com.param2 = 0;
		com.param3 = 0;
		com.lparam = 0;
		send( cid, &com, sizeof( com ) );
	}

	r = close_connection( cid );

	PROT_OUT();

	// 終了処理
	return r;
}

static int dev_read( struct iob *io, void *buf, int cnt )
{
	COM_HEADER com;
	int cid = ( int )io->i_private;

	PROT_IN();

	memset( &com, 0, sizeof( com ) );
	com.command = COM_READ;
	com.lparam = cnt;

	send( cid, &com, sizeof( com ) );
	recv( cid, &com, sizeof( com ) );
	if( com.lparam > 0 ){
		recv( cid, buf, com.lparam );
	}

	PROT_OUT();
	return com.lparam;
}

static int dev_write( struct iob *io, void *buf, int cnt )
{
	COM_HEADER com;
	int cid = ( int )io->i_private;

	PROT_IN();

	memset( &com, 0, sizeof( com ) );
	com.command = COM_WRITE;
	com.lparam = cnt;

	send( cid, &com, sizeof( com ) );
	send( cid, buf, cnt );

	recv( cid, &com, sizeof( com ) );

	PROT_OUT();
	return com.lparam;
}

static int dev_lseek( struct iob *io, unsigned int offset, int how )
{
	COM_HEADER com;
	int cid = ( int )io->i_private;

	PROT_IN();

	memset( &com, 0, sizeof( com ) );
	com.command = COM_SEEK;
	com.param1 = how;
	com.lparam = offset;

	send( cid, &com, sizeof( com ) );
	recv( cid, &com, sizeof( com ) );
	
	PROT_OUT();

	return com.lparam;
}

static int dev_ioctl( struct iob *io, int cmd, void *arg )
{
	return 0;
}

static char *get_tok( char *dest, char *src )
{
	// ':' か '\0' までをdestにコピー
	while( *src != ':' && *src != '\0' && *src != '\n' ){
		*( dest ++ ) = *( src ++ );
	}
	*dest = '\0';
	return ( *src != '\0' ) ? src + 1 : src;
}

static void set_host_param( HOSTPARAM *host, char *string )
{
	// $host:HOST:PORT:UID:GID:root_path
	// ex. $host:taira.konami:49275:11475:10000:/usr/local/develop/preview/usr01475
	char buf[ 64 ];
	char *s = string;
	int value;

	s = get_tok( server_name, s );
	s = get_tok( buf, s );
	if( ( value = strtol( buf, NULL, 0 ) ) > 0 ){
		portnum = value;
	}
	s = get_tok( buf, s );
	if( ( value = strtol( buf, NULL, 0 ) ) > 0 ){
		host->uid = value;
	}
	s = get_tok( buf, s );
	if( ( value = strtol( buf, NULL, 0 ) ) > 0 ){
		host->gid = value;
	}
	s = get_tok( host->path, s );

printf( "HOSTparam %s:%d:%d:%d:%s\n", server_name, portnum
		, host->uid, host->gid, host->path );

	set_server();
}

static int dev_getstat( struct iob *io, char *name, void *buf )
{
	COM_HEADER com;
	int cid;
	int res;
	// ファイル名を渡せるのでコマンド設定用として使う

	if( name[ 0 ] == '$' ){
		if( strcmp( name, "$check" ) == 0 ){
			// 常駐チェック
			int r;

			r = PollSema( lock_sema );
			if( r == KE_OK ){
				SignalSema( lock_sema );
				return -1;
			}
			return -2;
		} else if( strcmp( name, "$down" ) == 0 ){
			// 終了処理
			if( dev_id >= 0 ){
				printf( "Down:\n" );
				sceInetInterfaceControl( dev_id, sceInetCC_FlagClearUp, NULL, 0 );
				sceInetCtlDownInterface( 0 );
			}
			return -2;
		} else if( strncmp( name, "$host:", 6 ) == 0 ){
			// $host:HOST:PORT:UID:GID:root_path
			// ex. $host:taira.konami:49275:11475:10000:/usr/local/develop/preview/usr01475
			int cid;
			set_host_param( &hostparam, name + 6 );

			PROT_IN();
			cid = open_connection();
			if( cid >= 0 ){
				close_connection( cid );
			}
			PROT_OUT();

			return cid;
#if 1
		} else if( strncmp( name, "$test:", 6 ) == 0 ){
			/* 転送速度をはかってみる */

#define TEST_BUFFER_SIZE	(64 * 1024)
			static char test_buffer[ TEST_BUFFER_SIZE ];
			struct SysClock tm1, tm2;

			struct iob io;
			int size, _size;
			int fd;
			fd = dev_open( &io, name + 6, O_RDONLY );
			if( fd < 0 ){
				printf( "open error\n" );
				return fd;
			}
			_size = size = dev_lseek( &io, 0, SEEK_END );
			printf( "file %s size %d\n", name + 6, size );
			dev_lseek( &io, 0, SEEK_SET );

			GetSystemTime( &tm1 );
			while( size > 0 ){
				int s;
				s = ( size > TEST_BUFFER_SIZE ) ? TEST_BUFFER_SIZE : size;
				dev_read( &io, test_buffer, s );
				size -= s;
				printf( "%d            \r", size );
			}
			GetSystemTime( &tm2 );
			{
				int sec1, usec1;
				int sec2, usec2;
				int sec, usec;

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
					printf( "all %d %dKbytes/sec\n", _size
							, (_size/1024 *100 )/(sec*100+(usec/10000)));
				}
			}

			dev_close( &io );
#endif
		}
	}

	PROT_IN();

	cid = open_connection();
	if( cid < 0 ){
		PROT_OUT();
		return -1;
	}
	res = -1;

	memset( &com, 0, sizeof( com ) );
	com.command = COM_GETSTAT;
	com.lparam = strlen( name ) + 1;

	send( cid, &com, sizeof( com ) );
	send( cid, name, strlen( name ) + 1 );

	recv( cid, &com, sizeof( com ) );
	res = com.lparam;
	if( res > 0 ){
		recv( cid, buf, com.lparam );
	}

	close_connection( cid );

	PROT_OUT();
	
	return res;
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
	df_getstat: dev_getstat,
};

static struct device_table dev_tbl = {
	"netfs",
	DTTYPE_CHAR | DTTYPE_FS,
	5,
	"netfs",
	&dev_func
};

extern int AddDrv( struct device_table *dev_table );

static void DriverInit( void )
{
	int res;

	res = AddDrv( &dev_tbl );
	printf( "ADDDRV %d\n", res );
}

/* ---------------------------------------------------------------------- */
/*
	inet ライブラリの初期化
*/

static void setup_inet( int id )
{
	char buf[ 64 ];

	/* inet libの設定 */
	printf( "setup_inet\n" );
	if( sceInetInterfaceControl( id, sceInetCC_GetInterfaceName, buf, sizeof( buf ) ) == 0 ){
		printf( "interface %s\n", buf );
	}
	if( sceInetInterfaceControl( id, sceInetCC_GetVendorName, buf, sizeof( buf ) ) == 0 ){
		printf( "vendar %s\n", buf );
	}
	if( sceInetInterfaceControl( id, sceInetCC_GetDeviceName, buf, sizeof( buf ) ) == 0 ){
		printf( "device %s\n", buf );
	}
	printf( "set DHCP\n" );

	sceInetInterfaceControl( id, sceInetCC_FlagSetDHCP, NULL, 0 );
	sceInetInterfaceControl( id, sceInetCC_DHCPSetRelOnStop, NULL, 0 );

	sceInetInterfaceControl( id, sceInetCC_FlagSetUp, NULL, 0 );
}

static void interface_change_callback( int id, int type )
{
	printf( "id = %x type = %x\n", id, type );
	switch( type ){
	  case sceINETCTL_IEV_Attach:
		dev_id = id;
		setup_inet( id );
		break;
	  case sceINETCTL_IEV_Start:
		printf( "inet start\n" );
		// サーバーのアドレスを確定
//		set_server();
		// セマフォを確定
		SignalSema( lock_sema );
		break;
	}
}

/* ---------------------------------------------------------------------- */
/*
	RPC サーバ関数
*/

#if 0

#define LOAD_BUFFER_SIZE	1024*1024

static char loadbuffer[ LOAD_BUFFER_SIZE ] __attribute__((aligned(64)));

/* 進行状態表示 */

#define SREG_NO	11

#define EVENT_READEND	0x00000001

static void send_rest( int rest )
{
	static sceSifCmdSRData d;
	d.rno = SREG_NO;
	d.value = rest;
	sceSifSendCmd( SIF_CMDC_SET_SREG, &d, sizeof( d ), 0, 0, 0 );
}

static void copy_from_disc( char *hdfile, char *fname )
{
	unsigned int pos;
	int out;
	void *buf[ 2 ];
	int which, bufsize;
	sceCdRMode mode;
	int size;
	int l, wl, rsize;

	{
		char discfile[ 32 ];
		sceCdlFILE fp;

		char *p;
		if( fname[ 0 ] == '.' ){
			fname ++;
		}
		for( p = fname; *p != '\0'; p++ ){
			*p = toupper( *p );
			if( *p == '/' ) *p = '\\';
		}
		sprintf( discfile, "%s%s;1", "", fname );

		if( sceCdSearchFile( &fp, discfile ) == 0 ){
			printf( "%s NOT FOUND\n", discfile );
			return;
		}	
		pos = fp.lsn;
		size = fp.size;
	}

	out = open( hdfile, O_WRONLY|O_CREAT|O_TRUNC, 0777 );
	if( out < 0 ){
		printf( "%s open error\n", hdfile );
		for( ;; ) ;
	}

//	ioctl2( out, PIOCALLOC, &size, sizeof( int ), NULL, 0 );

	which = 0;
	bufsize = LOAD_BUFFER_SIZE / 2;
	buf[ 0 ] = loadbuffer;
	buf[ 1 ] = loadbuffer + bufsize;

	mode.trycount = 255;
	mode.spindlctrl = SCECdSpinNom;
	mode.datapattern = SCECdSecS2048;

	l = ( size > bufsize ) ? bufsize : size;
	wl = l;
	rsize = ( l + 2047 ) / 2048;
	while( sceCdRead( pos, rsize, buf[ which ], &mode ) == 0 );

	while( size > 0 ){
		int err;

		sceCdSync( 0 );
		if( ( err = sceCdGetError() ) != SCECdErNO ){
			// retry
			printf( "\nRETRY %X\n", err );
			while( sceCdRead( pos, rsize, buf[ which ], &mode ) == 0 );
			continue;
		}
		pos = pos + rsize;
		wl = l;
		if( size - l > 0 ){
			int s = size - wl;
			l = ( s > bufsize ) ? bufsize : s;
			rsize = ( l + 2047 ) / 2048;
			while( sceCdRead( pos, rsize, buf[ 1 - which ], &mode ) == 0 );
		}
		printf( "%d        \r", size );
		send_rest( size );

		if( write( out, buf[ which ], wl ) < 0 ){
			printf( "w Error: PLEASE FORMAT\n" );
			for( ;; );
		}
		size -= wl;
		which = 1 - which;
	}
	printf( "        \r" );
	send_rest( 0 );

//	sceIoctl2( out, PIOCFREE, NULL, 0, NULL, 0 );
	close( out );
}

static void copy_from_net( char *hdfile, char *fname )
{
}

static void *server_func( unsigned int fno, void *data, int size )
{
	char dest[ 128 ];
	char src[ 128 ];

	{
		char *p = data;
		char *d;

		for( d = dest; *p != '?'; p++ ){
			*( d++ ) = *p;
		}
		*d = '\0'; p++;
		for( d = src; *p != '\0'; p++ ){
			*( d++ ) = *p;
		}
		*d = '\0';
	}
	switch( fno ){
	  case 1:
		printf( "data : %s <- %s\n", dest, src );
		copy_from_disc( dest, src );
		break;
	}

	return NULL;
}

static unsigned int buffer[ 256 ] __attribute__((aligned(64)));

static void server_main( void )
{
	sceSifQueueData qd;
	sceSifServeData sd;

printf( "server boot\n" );
	sceSifSetRpcQueue( &qd, GetThreadId() );
	sceSifRegisterRpc( &sd, 0x00000010, server_func, buffer, NULL, NULL, &qd );

	sceSifRpcLoop( &qd );
}

static void set_rpc( void )
{
	printf( "set RPC\n" );
	sceSifInitRpc( 0 );
	{
		struct ThreadParam param;
		int id;

		param.attr = TH_C;
		param.entry = server_main;
		param.initPriority = 80;
		param.stackSize = 2048;

		id = CreateThread( &param );
		if( id > 0 ){
			// success
			StartThread( id, 0 );
		} else {
			// fail
		}
	}
}

#endif

/* ---------------------------------------------------------------------- */
/*
	メイン関数
*/

static sceInetCtlEventHandlers_t ev = {
	func : interface_change_callback,
};

int start( int argc, char *argv[] )
{
	{
		struct SemaParam param;
		int sid;

		param.attr = SA_THPRI;
		param.initCount = 0;
		param.maxCount = 1;
		param.option = 0;
		if( ( sid = CreateSema( &param ) ) <= 0 ){
			printf( "create sema %d\n", sid );
			return NO_RESIDENT_END;
		}
		lock_sema = sid;
	}

	dev_id = -1;

	sceInetCtlRegisterEventHandler( &ev );

	DriverInit();
#if 0
	set_rpc();
#endif

	return RESIDENT_END;
}
