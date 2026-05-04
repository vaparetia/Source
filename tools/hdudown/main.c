/*
	main.c
		hdudown メインルーチン
	2000/01/22	K.Uehara
	$Id: main.c,v 1.8 2001/09/19 05:16:32 usr01475 Exp $
*/

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>
#include <libdma.h>
#include <libgraph.h>
#include <sifdev.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <libcdvd.h>
#include <string.h>
#include <libcdvd.h>
#include <libpad.h>
#include <ctype.h>
#include <libmc.h>

#include "cons.h"

#define PS2_EE
#include "zlib.h"

#define cprintf( a, b... )	cons_printf( a, ##b )
#define ERROR()				for(;;);

//#define CDBOOT	1		// make cdの時に外部で定義される。
//#define USBCHECK	1

//#define DISC_ONLY	1		// DISCからのダウンロードONLY(RMCなど用)
//#define BOOT_ONLY	1		// BOOT ONLY
//#define DL_CENTER	1		// ダウンロードセンター専用
//#define PAELLA	1		// paellaからのダウンロード
#define MANILA	1		// manilaからのダウンロード

#define MAX_SELECT 16

#define ZLIB_WORK_SIZE	(45656)

// default 設定
#if 0
static char hostparam[ 256 ] =	"netfs0:$host:paella.konami:49275:11475:10000:" \
								"/usr/local/develop/worm/mgs2/cdrom.img/";
#else
#if defined( PAELLA )
static char hostparam[ 256 ] =	"netfs0:$host:paella.konami:49275:11475:10000:" \
								"/usr/local/develop/worm2/mgs2/cdrom.img/:";
#elif defined( MANILA )
static char hostparam[ 256 ] =	"netfs0:$host:manila.konami:49275:11475:10000:" \
								"/usr/local/develop/image/:";
#else
static char hostparam[ 256 ] =	"netfs0:$host:meji.konami:49275:11475:10000:" \
								"/usr/local/develop/worm/mgs2/cdrom.img/:";
#endif
//static char hostparam[ 256 ] =	"netfs0:$host:taira.konami:49275:11475:10000:" \
//								"/usr/local/home/kaz/work/mgs2/cdrom.img/:";
#endif

void dump( char *top, int len )
{
	int i;
	unsigned char *p;

	p = ( unsigned char * )top;

	printf( "%08X: ", p );

	for( i = 0; i < len; i++ ){
		printf( "%02X ", *p );
		p++;
		if( i % 16 == 15 ){
			printf( "\n%08X: ", p );
		}
	}
	printf( "\n" );
}

#ifndef CDBOOT

#define HD_MODULE		"host0:./image/modules/syshd.ipk"
#define USB_MODULE		"host0:./image/modules/usb.ipk"
#define NET_MODULE		"host0:./image/modules/net.ipk"

#define LOADER_IRX		"host0:./image/modules/pld.irx"

#else

#define IOP_REBOOT		"cdrom0:\\MODULES\\" IOP_IMAGE_FILE ";1"

#define HD_MODULE		"cdrom0:\\MODULES\\SYSHD.IPK;1"
#define USB_MODULE		"cdrom0:\\MODULES\\USB.IPK;1"
#define NET_MODULE		"cdrom0:\\MODULES\\NET.IPK;1"

#define LOADER_IRX		"cdrom0:\\MODULES\\PLD.IRX;1"

#endif

static char *modules[] = {
	HD_MODULE,
	USB_MODULE,
	NET_MODULE,
	NULL
};

enum {
	NO_DOWNLOAD,	// NETcheck->BOOT
	DOWNLOAD_USB,
	DOWNLOAD_NET,
	DOWNLOAD_CD,
};
static char *desc_string[] = { "", "usb0:", "netfs0:", "cdrom0:" };

#ifdef DISC_ONLY
#define LOAD_BUFFER_SIZE 1024 * 1024 * 4
#else
#define LOAD_BUFFER_SIZE 1024 * 1024
#endif

static char loadbuffer[ LOAD_BUFFER_SIZE ] __attribute__((aligned(64)));
static u_long128 pad_dma_buf[ scePadDmaBufferMax ] __attribute__((aligned(64)));

static int download_flag;
static int format_flag = 0;
static int part_remove_flag = 0;

static char mount_string[ 256 ];
static int pfs_zone_size;

#define HDD_EXTEND_STEP	  "256M"
#define DEFAULT_ZONE_SIZE ( 16 * 1024 )

/* ---------------------------------------------------------------------- */
/*
	RPC系処理
	*/
#if 0

#define SREG_NO		11

static sceSifClientData rpc_cd;

static void init_rpc( void )
{
	sceSifBindRpc( &rpc_cd, 0x00000010, 0 );
}

static void call_rpc_copy_from_disc( char *dst, char *src, int size )
{
	static char string[ 256 ];

	sprintf( string, "%s?%s", dst, src );

	sceSifSetSreg( SREG_NO, size );
	sceSifCallRpc( &rpc_cd, 1, SIF_RPCM_NOWAIT, string
				   , strlen( string ) + 1
				   , NULL, 0, NULL, NULL );
	
}

static int get_rpc_rest( void )
{
	return sceSifGetSreg( SREG_NO );
}

#endif

/* ---------------------------------------------------------------------- */
/*
ユーティリティ
*/

static char namebuf[ 128 ];

static int hostOpen( char *fname, int mode )
{
	if( download_flag != DOWNLOAD_CD ){
		sprintf( namebuf, "%s%s", desc_string[ download_flag ], fname );
	} else {
		char *p;
		if( fname[ 0 ] == '.' ){
			fname ++;
		}
		for( p = fname; *p != '\0'; p++ ){
			*p = toupper( *p );
			if( *p == '/' ) *p = '\\';
		}
		sprintf( namebuf, "%s%s;1", desc_string[ download_flag ], fname );
	}
	printf( "sceOpen %s %x\n", namebuf, mode );
	return sceOpen( namebuf, mode );
}

static int hostGetstat( char *fname, struct sce_stat *stat )
{
	if( download_flag == DOWNLOAD_USB ){
		return usbGetStat( fname, stat );
	} else if( download_flag == DOWNLOAD_NET ){

		sprintf( namebuf, "%s%s", desc_string[ download_flag ], fname );
	} else if( download_flag == DOWNLOAD_CD ){
		char *p;
		if( fname[ 0 ] == '.' ){
			fname ++;
		}
		for( p = fname; *p != '\0'; p++ ){
			*p = toupper( *p );
			if( *p == '/' ) *p = '\\';
		}
		sprintf( namebuf, "%s%s;1", desc_string[ download_flag ], fname );
		
	}
	return sceGetstat( namebuf, stat );
}

static int load_file( char *fname, void *buffer )
{
	int fd;
	int size;

	fd = hostOpen( fname, SCE_RDONLY );
	if( fd < 0 ){
		cprintf( "FILE %s not found..\n", fname );
		return 0;
	}
	size = sceLseek( fd, 0, SCE_SEEK_END );
	sceLseek( fd, 0, SCE_SEEK_SET );
	if( sceRead( fd, buffer, size ) < size ){
		printf( "read err\n" );
	}
	sceClose( fd );

	return size;
}

#if 0
static int load_usb_file( char *fname, void *buffer )
{
	int fd;
	int size;

	fd = usbOpen( fname, SCE_RDONLY );
	if( fd < 0 ){
		cprintf( "FILE %s not found..\n" );
		ERROR();
	}
	size = usbLseek( fd, 0, SCE_SEEK_END );
	usbLseek( fd, 0, SCE_SEEK_SET );
	if( usbRead( fd, buffer, size ) < size ){
		printf( "read err\n" );
	}
	usbClose( fd );

	return size;
}
#endif

static void free_area_check( int size )
{
	int freesize;

	for( ;; ){
		int fd, r;

		freesize = sceDevctl( "pfs0:", PDIOC_ZONEFREE, NULL, 0, NULL, 0 ) * pfs_zone_size;
		if( freesize >= size ){
			break;
		}
		sceUmount( "pfs0:" );

		// 拡張する
		fd = sceOpen( mount_string, SCE_RDWR, NULL, 0 );
		if( fd < 0 ){
			cprintf( "open err %d\n", fd );
			ERROR();
		}
		r = sceIoctl2( fd, HIOCADDSUB, HDD_EXTEND_STEP
					   , strlen( HDD_EXTEND_STEP ) + 1, NULL, 0 );
		if( r < 0 ){
			cprintf( "DISC FULL: REBOOT L1+R1 : %d\n", r );
			ERROR();
		}
		sceClose( fd );

		// MOUNT しなおす
		r = sceMount( "pfs0:", mount_string, SCE_MT_RDWR, NULL, 0 );
		if( r < 0 ){
			cprintf( "mount err %d\n", r );
			ERROR();
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
圧縮展開関連
*/

#define DECODE_BUFFER_SIZE	(1024*1024)

static char zlib_buf[ ZLIB_WORK_SIZE ];
static char zlib_decode_buf[ DECODE_BUFFER_SIZE ];
static int zlib_use_flag = 0;
static int zlib_first_flag = 0;
static z_stream z;

static void zlib_decode_init( void )
{
	zlib_use_flag = 1;
	zlib_first_flag = 1;

	z.buffer = zlib_buf;
	if( inflateInit( &z ) != Z_OK ){
		cprintf( "inflate Err\n" );
		ERROR();
	}

	z.next_out = zlib_decode_buf;
	z.avail_out = DECODE_BUFFER_SIZE;
}

static int file_write( int fd, char *buffer, int size )
{
	if( zlib_use_flag == 0 ){
		// 無圧縮データ
		return sceWrite( fd, buffer, size );
	} else {
		// 圧縮データ
		int status;

		if( zlib_first_flag ){
			z.next_in = buffer + 4;
			z.avail_in = size - 4;
			zlib_first_flag = 0;
		} else {
			z.next_in = buffer;
			z.avail_in = size;
		}

		for( ;; ){
			status = inflate( &z, Z_NO_FLUSH );
			if( status == Z_STREAM_END || z.avail_in == 0 ){
				// おわり
				break;
			}
			if( status != Z_OK ){
				printf( "DECODE ERR\n" );
				ERROR();
			}
			if( z.avail_out == 0 ){
				if( sceWrite( fd, zlib_decode_buf, DECODE_BUFFER_SIZE ) < DECODE_BUFFER_SIZE ){
					cprintf( "write err\n" );
					ERROR();
				}
				z.next_out = zlib_decode_buf;
				z.avail_out = DECODE_BUFFER_SIZE;
			}
		}
	}
	return size;
}

static int file_flush( int fd )
{
	if( zlib_use_flag ){
		// 処理が必要なのは圧縮データの時だけ
		int count;
		count = DECODE_BUFFER_SIZE - z.avail_out;
		if( count > 0 ){
			if( sceWrite( fd, zlib_decode_buf, count ) < count ){
				cprintf( "write err\n" );
				ERROR();
			}
		}
		if( inflateEnd( &z ) != Z_OK ){
			cprintf( "inflate end err\n" );
			ERROR();
		}
		zlib_use_flag = 0;
	}
	return 0;
}

static void file_check( char *hostfile )
{
	int size;
	int fd;

	if( zlib_use_flag ){
		fd = hostOpen( hostfile, SCE_RDONLY );
		if( fd < 0 ){
			cprintf( "%s open error\n", hostfile );
			ERROR();
		}
		sceRead( fd, &size, sizeof( int ) );
		sceClose( fd );

		printf( "check size = %d\n", size );
		free_area_check( size );
	}
}

static void file_check_disc( int pos )
{
	if( zlib_use_flag ){
		sceCdRMode mode;
		int size;

		mode.trycount = 0;
		mode.spindlctrl = SCECdSpinNom;
		mode.datapattern = SCECdSecS2048;

		sceCdRead( pos, 1, loadbuffer, &mode );
		sceCdSync( 0 );

		size = *( int * )loadbuffer;

		printf( "check size = %d\n", size );
		free_area_check( size );
	}
}

/* ---------------------------------------------------------------------- */
/*
	ダウンロード処理本体
*/

#define MAX_SCRIPT		( 16 * 1024 )
static char script_buffer[ MAX_SCRIPT ];
static char *current_p;

static int load_scr_file( char *file )
{
	int size;
	size = load_file( file, script_buffer );
	script_buffer[ size ] = '\0';
	current_p = script_buffer;

	return size;
}

static void *get_word( char *buf )
{
	char *p = current_p;
	char *d = buf;

	while( *p != '\n' && isspace( *p ) ){
		p++;
	}
	if( *p == '#' ){
		/* 行末までスキップ */
		while( *p != '\0' && *p != '\n' ){
			p++;
		}
	}
	if( *p == '\0' || *p == '\n' ){
		current_p = p;
		return NULL;
	}

	while( ispunct( *p ) || isalnum( *p ) ){
		*( d ++ ) = *( p ++ );
	}
	*d = '\0';

	current_p = p;

	return buf;
}

static int get_line( void )
{
	if( *current_p == '\0' ){
		return 0;
	}
	if( *current_p == '\n' ){
		current_p ++;
		return 1;
	}
	for( ;; ){
		if( *current_p == '\0' ){
			return 0;
		}
		if( *current_p == '\n' ){
			current_p++;
			return 1;
		}
		current_p ++;
	}
	return 2;
}


/* ---------------------------------------------------------------------- */
/*
	PAD READ
*/

typedef struct {
	unsigned short status;
	unsigned short press;
} PAD;

static void ClearPad( PAD *pad )
{
	pad->status = 0;
	pad->press = 0;
}

static int ReadPad( PAD *pad )
{
	int ps;
	unsigned short button;

	ps = scePadGetState( 0, 0 );
	button = 0;
	if( ps == scePadStateStable || ps == scePadStateFindCTP1 ){
		unsigned char buf[ 32 ];
		if( scePadRead( 0, 0, buf ) != 0 && buf[ 0 ] == 0x00 ){
			button = ~( ( buf[ 2 ] << 8 ) | buf[ 3 ] );
		}
	} else {
		return -1;
	}
	pad->press = ( ~pad->status & button );
	pad->status = button;

	return button;
}


/* ---------------------------------------------------------------------- */
/*
	CDチェック処理
*/

static void disc_change( void )
{
	// CD
	int t;
	int end_flag;
	PAD pad;

	end_flag = 0;
	while( end_flag == 0 ){
		int req;
		sceCdStop();
		sceCdSync( 0 );

		sceCdTrayReq( SCECdTrayOpen, &t );
		for( ;; ){
			// OPEN CHECK
			if( ( sceCdStatus() & SCECdStatShellOpen ) ){
				break;
			}
			sceGsSyncV( 0 );
		}

		ClearPad( &pad );
		req = 0;

		for( ;; ){
			// CLOSE CHECK
			if( !( sceCdStatus() & SCECdStatShellOpen ) ){
				break;
			}
			ReadPad( &pad );
			if( req == 0 && ( pad.press & SCE_PADRright ) ){
				req = 1;
				sceCdTrayReq( SCECdTrayClose, &t );
			}
			sceGsSyncV( 0 );
		}
		cprintf( "DISC CHECK..." );
		sceCdDiskReady( 0 );

		switch( t = sceCdGetDiskType() ){
		  case SCECdPS2DVD:
			sceCdMmode( SCECdDVD );
			cprintf( "PS2 DVD\n" );
			end_flag = 1;
			break;
		  case SCECdPS2CD:
			sceCdMmode( SCECdCD );
			cprintf( "PS2 CD\n" );
			end_flag = 1;
			break;
		  default:
			cprintf( "Disk type Error %02X\n", t );
			break;
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
	メモリカードにかかれているホスト情報のセレクタ
		-----
		: demo game last
		demo:netfs0:$host:hunter.konami:49275:11445.......
		game:netfs0:$host:sinonome.konami:49275:11445.......
		last:netfs0:$host:sinonome.konami:49275:11445.......
		-----
		みたいなスクリプトを入力
		セレクトできるのは最大MAX_SELECT(16)まで
*/

// : item1 item2 item3 ...
// 形式のスクリプトを入力。選択された文字列を返す

static char *select_item( char *selected, int *itemnump, char *script, int timeout )
{
	char *items[ MAX_SELECT ];
	int item_num = 0;
	int i;
	int n;
	char *p;
	char *hosts_top;
	PAD pad;

	/* セレクトする情報の作成 */
	p = script;
	if( *p != ':' ){
		cprintf( "SCRIPT Err\n" );
		return -1;
	}
	p++;
	for( i = 0; i < MAX_SELECT; i++ ){
		while( *p != '\n' && isspace( *p ) ){
			p++;
		}
		items[ i ] = p;
		while( ispunct( *p ) || isalnum( *p ) ){
			p++;
		}
		if( *p == '\n' ){
			if( items[ i ] == p ){
				i--;
			}
			item_num = i + 1;
			i = MAX_SELECT;		// LOOP END
		}
		*( p ++ ) = '\0';
	}
	hosts_top = p;
	n = 0;

	ClearPad( &pad );
	ReadPad( &pad );
	ReadPad( &pad );
	for( i = timeout * 60; i > 0; i-- ){
		cprintf( "%2d: %s        \r", i / 60 + 1, items[ n ] );
		ReadPad( &pad );
		if( pad.press & ( SCE_PADLup | SCE_PADLleft ) ){
			n = ( n + item_num - 1 ) % item_num;
			i = timeout * 60;
		} else if( pad.press & ( SCE_PADLdown | SCE_PADLright ) ){
			n = ( n + 1 ) % item_num;
			i = timeout * 60;
		} else if( pad.press & SCE_PADRright ){
			i = 0;
		}
		sceGsSyncV( 0 );
	}
	cprintf( "SELECT:%s       \n", items[ n ] );

	strcpy( selected, items[ n ] );
	*itemnump = item_num;

	return hosts_top;
}

// 選択されたラベルに対応したネット設定文字列を取得

static int net_select( char *target, char *script, int timeout )
{
	char selected[ 64 ];
	char *hosts_top;
	int item_num;

	hosts_top = select_item( selected, &item_num, script, timeout );

	{
		char *item = selected;
		char *p;
		int i, len;

		len = strlen( item );
		p = hosts_top;
		for( i = 0; i < item_num; i++ ){
			if( strncmp( item, p, len ) == 0 ){
				// HIT
				char *d;
				p = p + len + 1;
				d = target;
				while( ispunct( *p ) || isalnum( *p ) ){
					*( d++ ) = *( p++ );
				}
				*d = '\0';
				return 0;
			}
			while( *p != '\n' && *p != '\r' && *p != '\0' ){
				p++;
			}
			if( *p == '\0' ){
				break;
			}
			while( *p == '\n' || *p == '\r' ){
				p++;
			}
		}
	}
	cprintf( "Err:NO LABEL\n" );
	return -1;
}

/* ---------------------------------------------------------------------- */
/*
	スクリプトで起動するコマンド
*/

static struct sce_stat stat ;

static void mountCmd( void )
{
	int r;
	int exist;
	char buf[ 128 ];

	if( get_word( buf ) == NULL ){
		cprintf( "mount : no param\n" );
		ERROR();
	}

	if( format_flag == 1 ){
#if 0
		if( ( r = sceOpen( buf, SCE_RDONLY ) ) >= 0 ){
			// ある。
			sceClose( r );
			cprintf( "%s is exit .. remove\n", buf );
			if( ( r = sceRemove( buf ) ) != 0 ){
				cprintf( "Err %d\n", r );
			}
		}
#else
		cprintf( "hdd format\n" );
		sceFormat( "hdd0:", NULL, NULL, 0 );
#endif
	}
	exist = 0;

	if( ( r = sceOpen( buf, SCE_RDONLY ) ) >= 0 ){
		/* すでにあるので mount する */
		sceClose( r );
		if( part_remove_flag ){
			cprintf( "REMOVE %s\n", buf );
			r = sceRemove( buf );
			if( r < 0 ){
				cprintf( "err %d\n", r );
				ERROR();
			}
		} else {
			exist = 1;
		}
	}
	if( exist == 0 ){
		int fd;
		int zonesize = DEFAULT_ZONE_SIZE;

		if( format_flag == 2 ){
			cprintf( "hdd format\n" );
			sceFormat( "hdd0:", NULL, NULL, 0 );
		}
		cprintf( "create %s\n", buf );
		fd = sceOpen( buf, SCE_CREAT | SCE_RDWR );
		if( fd < 0 ){
			cprintf( "err %d\n", r );
			ERROR();
		}
		sceClose( fd );
		cprintf( "part format\n" );
		r = sceFormat( "pfs:", buf, &zonesize, sizeof( zonesize ) );
		if( r < 0 ){
			cprintf( "err %d\n", r );
			ERROR();
		}
	}
	cprintf( "mount pfs0: %s\n", buf );
	strcpy( mount_string, buf );

//	r = sceMount( "pfs0:", buf, SCE_MT_RDWR | SCE_MT_ROBUST, NULL, 0 );
	r = sceMount( "pfs0:", buf, SCE_MT_RDWR, NULL, 0 );
	if( r != 0 ){
		cprintf( "err %d\n", r );
		ERROR();
	}
	pfs_zone_size = sceDevctl( "pfs0:", PDIOC_ZONESZ, NULL, 0, NULL, 0 );
}

static void dirCmd( void )
{
	/* directory なかったら作成する */
	int r;
	char dir[ 64 ];
	if( get_word( dir ) == NULL ){
		cprintf( "no dir name\n" );
		ERROR();
	}
	r = sceGetstat( dir, &stat );
	if( r == -2 ){
cprintf( "mkdir %s\n", dir );
		r = sceMkdir( dir, 0777 );
		if( r < 0 ){
			cprintf( "err %d\n", r );
			ERROR();
		}
	}
}

static void file_copy_usb( char *hdfile, char *usbfile, int size )
{
	int in, out;
	void *buf[ 2 ];
	int which, bufsize;

	// usbOpen系の関数の方が高速

	in = usbOpen( usbfile, SCE_RDONLY );
	if( in < 0 ){
		cprintf( "%s open error\n", usbfile );
		ERROR();
	}

	out = sceOpen( hdfile, SCE_WRONLY|SCE_CREAT|SCE_TRUNC, 0777 );
	if( out < 0 ){
		cprintf( "%s open error\n", hdfile );
		ERROR();
	}

//	sceIoctl2( out, PIOCALLOC, &size, sizeof( int ), NULL, 0 );

	which = 0;
	bufsize = LOAD_BUFFER_SIZE / 2;
	buf[ 0 ] = loadbuffer;
	buf[ 1 ] = loadbuffer + bufsize;

	while( size > 0 ){
		int l, r;
		cprintf( "%d        \r", size );

		l = ( size > bufsize ) ? bufsize : size;
		r = usbRead( in, buf[ which ], l );
		if( r < l ){
			cprintf( "\nRead Err %d\n", r );
			ERROR();
		}
		file_write( out, buf[ which ], l );
		size -= l;
		which = 1 - which;
	}
	cprintf( "        \r" );
	file_flush( out );

//	sceIoctl2( out, PIOCFREE, NULL, 0, NULL, 0 );

	usbClose( in );
	sceClose( out );
}

static void file_copy( char *hdfile, char *hostfile, int size )
{
	int in, out;
	void *buf[ 2 ];
	int which, bufsize;
	int l;

	file_check( hostfile );

	in = hostOpen( hostfile, SCE_RDONLY | SCE_NOWAIT );
	if( in < 0 ){
		cprintf( "%s open error\n", hostfile );
		ERROR();
	}

	out = sceOpen( hdfile, SCE_WRONLY|SCE_CREAT|SCE_TRUNC, 0777 );
	if( out < 0 ){
		cprintf( "%s open error\n", hdfile );
		ERROR();
	}

//	sceIoctl2( out, PIOCALLOC, &size, sizeof( int ), NULL, 0 );
	// これをいれると８こ目のWriteOpenでENOMEMになる。

	which = 0;
	bufsize = LOAD_BUFFER_SIZE / 2;
	buf[ 0 ] = loadbuffer;
	buf[ 1 ] = loadbuffer + bufsize;

	l = ( size > bufsize ) ? bufsize : size;
	sceRead( in, buf[ which ], l );

	while( size > 0 ){
		int r, wl;

		wl = l;
		{
			int exec;
			do {
				sceIoctl( in, SCE_FS_EXECUTING, &exec );
			} while( exec );
		}

		if( size - l > 0 ){
			int s = size - wl;
			l = ( s > bufsize ) ? bufsize : s;
			sceRead( in, buf[ 1 - which ], l );
		}
		cprintf( "%d        \r", size );

		file_write( out, buf[ which ], wl );
		size -= wl;
		which = 1 - which;
	}
	cprintf( "        \r" );

//	sceIoctl2( out, PIOCFREE, NULL, 0, NULL, 0 );
	// これをいれると８こ目のWriteOpenでENOMEMになる。

	file_flush( out );

	sceClose( out );
	sceClose( in );
}

static void file_copy_disc( char *hdfile, char *fname, int size )
{
#if 1
	unsigned int pos;
	int out;
	void *buf[ 2 ];
	int which, bufsize;
	sceCdRMode mode;
	int l, rsize;

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

		file_check_disc( pos );
	}

	out = sceOpen( hdfile, SCE_WRONLY|SCE_CREAT|SCE_TRUNC, 0777 );
	if( out < 0 ){
		cprintf( "%s open error\n", hdfile );
		ERROR();
	}

//	sceIoctl2( out, PIOCALLOC, &size, sizeof( int ), NULL, 0 );

	which = 0;
	bufsize = LOAD_BUFFER_SIZE / 2;
	buf[ 0 ] = loadbuffer;
	buf[ 1 ] = loadbuffer + bufsize;

	mode.trycount = 0;
	mode.spindlctrl = SCECdSpinNom;
	mode.datapattern = SCECdSecS2048;

	l = ( size > bufsize ) ? bufsize : size;
	rsize = ( l + 2047 ) / 2048;
	sceCdRead( pos, rsize, buf[ which ], &mode );
	pos += rsize;

	while( size > 0 ){
		int r, wl, err;

		wl = l;
		sceCdSync( 0 );
		if( ( err = sceCdGetError() ) != SCECdErNO ){
			cprintf( "\nr Error %x: PLEASE RESTART\n", err );
			for( ;; );
		}
		if( size - l > 0 ){
			int s = size - wl;
			l = ( s > bufsize ) ? bufsize : s;
			rsize = ( l + 2047 ) / 2048;
			sceCdRead( pos, rsize, buf[ 1 - which ], &mode );
			pos += rsize;
		}
		cprintf( "%d        \r", size );

		if( file_write( out, buf[ which ], wl ) < 0 ){
			cprintf( "w Error: PLEASE FORMAT\n" );
			for( ;; );
		}
		size -= wl;
		which = 1 - which;
	}
	cprintf( "        \r" );
	file_flush( out );

//	sceIoctl2( out, PIOCFREE, NULL, 0, NULL, 0 );
	sceClose( out );
#else
	int rest;

	call_rpc_copy_from_disc( hdfile, fname, size );
	rest = 0;

	for( ;; ){
		int r;
		r = get_rpc_rest();
		if( rest != r ){
			rest = r;
			cprintf( "%d        \r", rest );
		}
		if( rest <= 0 ){
			break;
		}
	}
	cprintf( "        \r" );
#endif
}

#if 0
static char *get_time_str( char *timedata )
{
	static unsigned char timestr[ 32 ];
	unsigned char *t;

	t = ( unsigned char * )timedata;

	sprintf( timestr, "%04d/%02d/%02d %02d:%02d:%02d"
			 , t[ 7 ] * 256 + t[ 6 ], t[ 5 ], t[ 4 ], t[ 3 ], t[ 2 ], t[ 1 ] );
	return timestr;
}
#endif

static void fileCmd( void )
{
	char file1[ 64 ];
	char file2[ 64 ];
	char tmstr[ 64 ];
	int r, wflag;
	static struct sce_stat srcstat __attribute__((aligned(64)));

	wflag = 0;

	if( get_word( file1 ) == NULL ){
		cprintf( "no src file\n" );
		ERROR();
	}
	if( get_word( file2 ) == NULL ){
		cprintf( "no dest file\n" );
		ERROR();
	}

	r = hostGetstat( file1, &srcstat );

	if( r < 0 ){
		printf( "stat %s err %d\n", file1, r );
	}
	r = sceGetstat( file2, &stat );
	if( r < 0 ){
		// ファイルが無い
		wflag = 2;
	} else {
		// ファイルがあったので日付チェック
		int i;

		wflag = 0;
#if 0
		// statの方が新しかったらコピーしない
		for( i = 7; i > 0; i-- ){
			if( stat.st_mtime[ i ] > srcstat.st_mtime[ i ]  ){
				wflag = 0;
				break;
			} else if( stat.st_mtime[ i ] < srcstat.st_mtime[ i ]  ){
				wflag = 1;
				break;
			}
		}
#else
		// statの日付と合致しなかったらコピー
		for( i = 7; i > 0; i-- ){
			if( srcstat.st_mtime[ i ] != stat.st_mtime[ i ] ){
				wflag = 1;
				break;
			}
		}
#endif
	}
	{
#if defined( DISC_ONLY )
		strcpy( tmstr, "" );
#else
		unsigned char *t;
		t = srcstat.st_mtime;
		sprintf( tmstr, "[%02d/%02d %02d:%02d]"
				 , t[ 5 ], t[ 4 ], t[ 3 ], t[ 2 ] );
#endif
	}

	if( wflag == 0 ){
		cprintf( "skip %s %s\n", file1, tmstr );
	} else {
		sceRemove( file2 );

		free_area_check( srcstat.st_size );

		cprintf( "copy %s %s -> %s\n", file1, tmstr, file2 );
		if( download_flag == DOWNLOAD_USB ){
			file_copy_usb( file2, file1, srcstat.st_size );
		} else if( download_flag == DOWNLOAD_CD ){
			file_copy_disc( file2, file1, srcstat.st_size );
		} else {
			file_copy( file2, file1, srcstat.st_size );
		}
		sceChstat( file2, &srcstat, SCE_CST_MT );
	}
}

static void zfileCmd( void )
{
	zlib_decode_init();

	fileCmd();

	zlib_use_flag = 0;
}

#define EI_NIDENT (16)

typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Word;
typedef unsigned int Elf32_Addr;
typedef unsigned int Elf32_Off;

typedef struct
{
	unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
	Elf32_Half	e_type;			/* Object file type */
	Elf32_Half	e_machine;		/* Architecture */
	Elf32_Word	e_version;		/* Object file version */
	Elf32_Addr	e_entry;		/* Entry point virtual address */
	Elf32_Off	e_phoff;		/* Program header table file offset */
	Elf32_Off	e_shoff;		/* Section header table file offset */
	Elf32_Word	e_flags;		/* Processor-specific flags */
	Elf32_Half	e_ehsize;		/* ELF header size in bytes */
	Elf32_Half	e_phentsize;		/* Program header table entry size */
	Elf32_Half	e_phnum;		/* Program header table entry count */
	Elf32_Half	e_shentsize;		/* Section header table entry size */
	Elf32_Half	e_shnum;		/* Section header table entry count */
	Elf32_Half	e_shstrndx;		/* Section header string table index */
} Elf32_Ehdr;

typedef struct
{
	Elf32_Word	p_type;			/* Segment type */
	Elf32_Off	p_offset;		/* Segment file offset */
	Elf32_Addr	p_vaddr;		/* Segment virtual address */
	Elf32_Addr	p_paddr;		/* Segment physical address */
	Elf32_Word	p_filesz;		/* Segment size in file */
	Elf32_Word	p_memsz;		/* Segment size in memory */
	Elf32_Word	p_flags;		/* Segment flags */
	Elf32_Word	p_align;		/* Segment alignment */
} Elf32_Phdr;

static void __loadexec( char *name )
{
	// HEADERを読み込む
	void *exec_address = NULL;
	int offset;
	void *load_address;
	int load_size;
	int fd;
	int n_ph;

	fd = sceOpen( name, 1 );
	if( fd < 0 ){
		cprintf( "load file ERR %s\n", name );
		return;
	}
	{
		// ELF ファイルヘッダ読み込み
		Elf32_Ehdr header;

		if( sceRead( fd, &header, sizeof( Elf32_Ehdr ) ) < sizeof( Elf32_Ehdr ) ){
			cprintf( "load file broken1 %s\n", name );
			return;
		}

		exec_address = ( void * )header.e_entry;
		offset = header.e_phoff;
		n_ph = header.e_phnum;
	}

	if( n_ph > 0 ){
		// ELF プログラムヘッダ読み込み
		int i;
		Elf32_Phdr header[ n_ph ];

		sceLseek( fd, offset, 0 );
		if( sceRead( fd, &header, sizeof( Elf32_Phdr ) * n_ph ) < sizeof( Elf32_Phdr ) * n_ph ){
			cprintf( "load file broken2 %s\n", name );
			return;
		}

		// プログラムバイナリ本体の読み込み
		for( i = 0; i < n_ph; i++ ){
			int lsize;

			offset = header[ i ].p_offset;
			load_address = ( void * )header[ i ].p_paddr;
			load_size = header[ i ].p_filesz;

			sceLseek( fd, offset, 0 );

			if( ( lsize = sceRead( fd, load_address, load_size ) ) < load_size ){
				cprintf( "load file broken3 %s %d %d\n", name, load_size, lsize );
				return;
			}
			
			cprintf( "addr %X size %d ofs %X\n", load_address, load_size, offset );
		}
	}
	sceClose( fd );


	cprintf( "exec %s %X\n", name, exec_address );

	// システムの終了
	{
		struct sce_stat stat;
		sceGetstat( "netfs0:$down", &stat );
	}
	sceUmount( "pfs0:" );

#ifdef DL_CENTER
	if( 1 )
#else
	if( download_flag == DOWNLOAD_CD )
#endif
	{
		cprintf( "download done\n" );
		cprintf( "SET BOOT DISC\n" );
		for( ;; ){
			int fd;
			disc_change();
			fd = sceOpen( "cdrom0:\\KCEJINST.ID;1", SCE_RDONLY );
			if( fd >= 0 ){
				sceClose( fd );
				break;
			}
		}
	}

	scePadEnd();
	sceSifExitCmd();

	FlushCache( WRITEBACK_DCACHE );
	FlushCache( INVALIDATE_ICACHE );

	{
		char *args[] = { name, mount_string };
		ExecPS2( exec_address, 0, 2, args );
	}
}

static void loadexec( char *name )
{
#if 0
	// HEADERを読み込む
	void *exec_address;
	static sceExecData ed;
	int r;

	r = sceSifLoadElf( name, &ed );
	memset( &ed, 0, sizeof( sceExecData ) );
	exec_address = ( void * )ed.epc;
	cprintf( "EXEC ELF %s %X %X %X %d\n", name, ed.epc, ed.gp, ed.sp, r );
	// 読み込んだプログラムの実行

	// システムの終了
	{
		struct sce_stat stat;
		sceGetstat( "netfs0:$down", &stat );
	}
	scePadEnd();

	sceUmount( "pfs0:" );

	sceSifExitCmd();

	ExecPS2( exec_address, ( void * )ed.sp, 0, NULL );
#else
	__loadexec( name );
#endif
}

static void execCmd( void )
{
	char file[ 64 ];

	if( get_word( file ) == NULL ){
		cprintf( "no exec file\n" );
		ERROR();
	}

	loadexec( file );
}

static void formatCmd( void )
{
	int err;
	cprintf( "remove HDD partition.\n" );
#if 0
	if( ( err = sceFormat( "hdd0:", NULL, NULL, 0 ) ) < 0 ){
		cprintf( "err : %d\n", err );
	}
	format_flag = 0;
#else
	part_remove_flag = 1;
#endif
}

/* ---------------------------------------------------------------------- */
/*
	スクリプト解釈メイン
*/

typedef struct {
	char *keys;
	void (*func)( void );
} COMTABLE;

static void download( char *cnffile, int flag )
{
	static COMTABLE table[] = {
		{ "mount", mountCmd },
		{ "dir", dirCmd },
		{ "file", fileCmd },
		{ "zfile", zfileCmd },
		{ "exec", execCmd },
		{ "format", formatCmd },
		{ NULL, NULL },
	};

	// SCRIPT FILE LOAD

	load_scr_file( cnffile );

	do {
		char key[ 64 ];

		if( get_word( key ) != NULL ){
			COMTABLE *tp;
			for( tp = table; tp->keys != NULL; tp++ ){
				if( strcmp( key, tp->keys ) == 0 ){
					( *tp->func )();
					break;
				}
			}
		}
	} while( get_line() );
}

/* ---------------------------------------------------------------------- */
/*
	USBがささっていない時の処理
*/

typedef struct {
	unsigned char name[ 64 ];
	unsigned char time[ 8 ];
} DIRS;

static int dir_compare( const void *p1, const void *p2 )
{
	const DIRS *d1 = p1;
	const DIRS *d2 = p2;

	int i;

	for( i = 7; i > 0; i-- ){
		if( d1->time[ i ] != d2->time[ i ] ){
			return d2->time[ i ] - d1->time[ i ];
		}
	}
	return 0;
}

static void mount_and_exec( int remove_flag )
{
	struct sce_dirent dirbuf;
	int fd;
	int n;
	char selected[ 128 ];
	char buf[ 128 ];

	strcpy( loadbuffer, ": " );

	if( ( fd = sceDopen( "hdd0:" ) ) < 0 ){
		cprintf( "hdd0 open error %d\n", fd );
		return;
	}

	n = 0;
#if 0
	while( sceDread( fd, &dirbuf ) > 0 ){
		if( dirbuf.d_stat.st_mode != 0x0100 ){
			// FS NOT PFS
			continue;
		}
		if( dirbuf.d_stat.st_attr & 0x01 ){
			// SUB PARTITION
			continue;
		}

		/* mount して、hdboot.cnfを検索 */
		if( dirbuf.d_name[ 0 ] == '_' ){
			continue;
		}

		strcat( loadbuffer, dirbuf.d_name );
		strcat( loadbuffer, " " );
		n++;
	}
#else
	{
		static DIRS dirs[ 16 ];

		while( sceDread( fd, &dirbuf ) > 0 ){
			if( dirbuf.d_stat.st_mode != 0x0100 ){
				// FS NOT PFS
				continue;
			}
			if( dirbuf.d_stat.st_attr & 0x01 ){
				// SUB PARTITION
				continue;
			}

			/* mount して、hdboot.cnfを検索 */
			if( dirbuf.d_name[ 0 ] == '_' ){
				continue;
			}

			strcpy( dirs[ n ].name, dirbuf.d_name );
			memcpy( dirs[ n ].time, dirbuf.d_stat.st_mtime, 8 );

			n++;

			if( n > 15 ) break;
		}
		if( n > 0 ){
			int i;
			qsort( dirs, n, sizeof( DIRS ), dir_compare );
			for( i = 0; i < n; i++ ){
				strcat( loadbuffer, dirs[ i ].name );
				strcat( loadbuffer, " " );
			}
		}
	}
#endif
	if( n == 0 ){
		cprintf( "NO PARTITION\n" );
		return;
	}
	strcat( loadbuffer, "\n\n" );

	cprintf( "SELECT PARTITION...\n" );
	select_item( selected, &n, loadbuffer, 15 );

	sprintf( buf, "hdd0:%s,fpwd", selected );

	if( remove_flag == 0 ){
		// EXECUTE
		/* passwdがfpwdなもののみ */
		if( sceMount( "pfs0:", buf, SCE_MT_RDONLY, NULL, 0 ) < 0 ){
			cprintf( "part %s : passwd error\n", dirbuf.d_name );
			return;
		}

		strcpy( mount_string, buf );

		if( load_scr_file( "pfs0:/hdboot.cnf" ) <= 0 ){
			cprintf( "no hdboot.cnf\n" );
			goto NEXT;
		}

		if( get_word( buf ) == NULL || strcmp( buf, "BOOT2" ) != 0 ){
			cprintf( "keyword error %s\n", buf );
			goto NEXT;
		}
		if( get_word( buf ) == NULL || strcmp( buf, "=" ) != 0 ){
			cprintf( "keyword error %s\n", buf );
			goto NEXT;
		}
		if( get_word( buf ) == NULL ){
			cprintf( "keyword error\n" );
			goto NEXT;
		}

		loadexec( buf );
	} else {
		int r;
		r = sceRemove( buf );
		if( r == 0 ){
			cprintf( "REMOVED PARTITION.\n" );
		} else {
			cprintf( "remove Error %d\n", r );
		}
		cprintf( "PLEASE RESET\n" );
		for( ;; );
	}
	return;
NEXT:
	cprintf( "%s:Error Occured\n", selected );
	sceUmount( "pfs0:" );
	return;
}

/* ---------------------------------------------------------------------- */
/*
	メモリーカードチェック
*/

static int memcard_check( void *buffer, char *filename )
{
	// 各ポートを検査し設定ファイルがないかどうかを調べる
	int i;

	sceMcInit();

	for( i = 0; i < 2; i++ ){
		int type, free;
		int res, fd, size;
		sceMcGetInfo( i, 0, &type, &free, NULL );
		sceMcSync( 0, 0, &res );
		if( res < -1 ){
			// error
			continue;
		}
		if( type != 2 ){
			continue;
		}
		sceMcOpen( i, 0, filename, SCE_RDONLY );
		sceMcSync( 0, 0, &res );
		if( res < 0 ){
			continue;
		}
		cprintf( "load port %d %s\n", i, filename );
		fd = res;
		sceMcSeek( fd, 0, SEEK_END );
		sceMcSync( 0, 0, &res );
		size = res;
		if( size < 0 ){
			cprintf( "size err\n" );
			continue;
		}
		sceMcSeek( fd, 0, SEEK_SET );
		sceMcSync( 0, 0, &res );

		memset( buffer, 0, size + 1 );

		sceMcRead( fd, buffer, size );
		sceMcSync( 0, 0, &res );
		if( res < size ){
			cprintf( "read err %d\n", res );
			continue;
		}
		sceMcClose( fd );
		sceMcSync( 0, 0, &res );
		return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------- */
/*
	ベイ型HDD対応電源OFFルーチン
*/

#define STACK_SIZE ( 16 * 1024 )

static u_long128 stack[ STACK_SIZE / sizeof( u_long128 ) ];

static void power_off_thread( void *param )
{
	SleepThread();
	for( ;; ){
		int stat;
		sceDevctl( "pfs:", PDIOC_CLOSEALL, NULL, 0, NULL, 0 );
		sceDevctl( "hdd:", HDIOC_DEV9OFF, NULL, 0, NULL, 0 );
		sceCdPowerOff( &stat );
	}
}

static void power_off_callback( void *param )
{
	iWakeupThread( ( int )param );
}

static void power_off_init( void )
{
	struct ThreadParam tp;
	int tid;

	ChangeThreadPriority( GetThreadId(), 2 );
	tp.stackSize = STACK_SIZE;
	tp.gpReg = &_gp;
	tp.entry = power_off_thread;
	tp.stack = stack;
	tp.initPriority = 1;
	tid = CreateThread( &tp );
	StartThread( tid, NULL );

	sceCdPOffCallback( power_off_callback, ( void * )tid );
}

/* ---------------------------------------------------------------------- */
/*
	ブート処理
*/

static void *load_module( char **fname, void *ptr )
{
	for( ; *fname != NULL; fname ++ ){
		int size;

		size = load_file( *fname, ptr );
		ptr += size - 16;	// 終端タグを上書きして連結する
	}
	ptr += 16;

	FlushCache( 0 );

	return ptr;
}

int main( void )
{
	int i;
	int boot;
	PAD pad;
	int remove_flag;

	sceGsResetPath();
	sceDmaReset( 1 );
	sceSifInitRpc( 0 );

	cons_init();

	cprintf( "HDUDOWN FOR PS2+HDD (%s %s)\n", __DATE__, __TIME__ );

#if defined( DISC_ONLY )
	cprintf( "-- X:BOOT L1+R1:FORMAT R1:REMOVE --\n" );
#elif defined( BOOT_ONLY )
	cprintf( "-- BOOT ---\n" );
#elif defined( DL_CENTER )
	cprintf( "-- NO PAD: FORMAT --\n" );
#else
	cprintf( "-- O:NET TRI:CD/DVD X:BOOT L1+R1:FORMAT R1:REMOVE --\n" );
#endif

	// REBOOT
#ifdef CDBOOT
	cprintf( "REBOOTING %s..\n", IOP_IMAGE_FILE );
	sceCdInit( SCECdINIT );
	sceCdMmode( SCECdCD );

	while( sceSifRebootIop( IOP_REBOOT ) == 0 );
	while( sceSifSyncIop() == 0 );
#endif
	sceSifInitRpc( 0 );

	sceCdInit( SCECdINIT );
	sceCdMmode( SCECdCD );
	sceFsReset();

	cprintf( "load irx.." );

	// IRXのロード
	{
		char buf[ 16 ];
		sprintf( buf, "%08x", ( int )loadbuffer );

		load_module( modules, loadbuffer );
		sceSifLoadModule( LOADER_IRX, 9, buf );
	}

	cprintf( "done\n" );

	power_off_init();

	// JSIFMAN INIT
	sif_init();

//	init_rpc();

	// パッド関係の初期化
	scePadInit( 0 );
	scePadPortOpen( 0, 0, pad_dma_buf );

	download_flag = NO_DOWNLOAD;
#ifdef DISC_ONLY
	download_flag = DOWNLOAD_CD;
#endif

#if 0 /////////////// TEST

	{
		int t, req;

		sceCdStop();
		sceCdSync( 0 );

		sceCdTrayReq( SCECdTrayOpen, &t );
		for( ;; ){
			// OPEN CHECK
			if( ( sceCdStatus() & SCECdStatShellOpen ) ){
				break;
			}
			sceGsSyncV( 0 );
		}

		ClearPad( &pad );
		req = 0;

		for( ;; ){
			// CLOSE CHECK
			if( !( sceCdStatus() & SCECdStatShellOpen ) ){
				break;
			}
			ReadPad( &pad );
			if( req == 0 && ( pad.press & SCE_PADRright ) ){
				req = 1;
				sceCdTrayReq( SCECdTrayClose, &t );
			}
			sceGsSyncV( 0 );
		}
		cprintf( "DISC CHECK..." );
		sceCdDiskReady( 0 );

		switch( t = sceCdGetDiskType() ){
		  case SCECdPS2DVD:
			sceCdMmode( SCECdDVD );
			cprintf( "PS2 DVD\n" );
			break;
		  case SCECdPS2CD:
			sceCdMmode( SCECdCD );
			cprintf( "PS2 CD\n" );
			break;
		  default:
			{
				static unsigned char toc_buf[ 1024 ];
				unsigned char *toc;
				int i;
				sceCdlLOCCD pos;
				int lpos;
				sceCdRMode mode;
#if 0
				sceCdGetToc( toc_buf );
				sceCdSync( 0 );

				toc = toc_buf;

				for( i = 0; i < 12; i++ ){
					printf( "%02X %02X %02X %02X %02X %02X %02X %02X %02X\n"
							, toc[1], toc[2], toc[3], toc[4], toc[5]
							, toc[6], toc[7], toc[8], toc[9] );
					if( toc[ 2 ] == 0x02 ){
						pos.minute = toc[ 7 ];
						pos.second = toc[ 8 ];
						pos.sector = toc[ 9 ];
					}
					toc += 10;
				}
				lpos = sceCdPosToInt( &pos );
#endif
				lpos = 212087;

				printf( "pos = %d\n", lpos );
				mode.trycount = 16;
				mode.spindlctrl = SCECdSpinNom;
				mode.datapattern = SCECdSecS2048;

				if( sceCdRead( lpos, 64, loadbuffer, &mode ) == 0 ){
					printf( "cd read error\n" );
				}
				sceCdSync( 0 );
				printf( "Error = %d\n", sceCdGetError() );
			}
			break;
		}
	}

	for( ;; );


#endif

	// パッド読み取り
	// L1と○が押されていればフォーマットする
	format_flag = 0;
	boot = 0;
	remove_flag = 0;

	ClearPad( &pad );

	for( i = 60; i > 0; i-- ){
		if( ReadPad( &pad ) != -1 ){
			break;
		}
		sceGsSyncV( 0 );
	}
	if( i == 0 ){
		// TIMEOUT
#ifdef DL_CENTER
		download_flag = DOWNLOAD_NET;
		format_flag = 2;
#endif
		goto START;
	}

	if( format_flag ){
		cprintf( "COMMAND: HDD FORMAT REQUEST\n" );
	}
#ifdef BOOT_ONLY
	boot = 1;
	cprintf( "BOOT ONLY.\n" );
#else
	for( i = 0; i < 4; i++ ){
		ReadPad( &pad );
		if( ( pad.status & ( SCE_PADR1 | SCE_PADL1 ) ) == ( SCE_PADR1 | SCE_PADL1 ) ){
			format_flag = 1;
		}
		if( pad.status & SCE_PADRup ){
			download_flag = DOWNLOAD_CD;
			break;
		}
#if !defined( DISC_ONLY )
		if( pad.status & SCE_PADRright ){
			download_flag = DOWNLOAD_NET;
			break;
		}
#endif
		if( pad.status & SCE_PADRdown ){
			boot = 1;
			format_flag = 0;
			download_flag = NO_DOWNLOAD;
			cprintf( "BOOT ONLY.\n" );
			break;
		}
		if( ( pad.status & ( SCE_PADR1 | SCE_PADL1 ) ) == SCE_PADR1 ){
			boot = 1;
			remove_flag = 1;
			cprintf( "PART REMOVE\n" );
			break;
		}
		sceGsSyncV( 0 );
	}
	if( format_flag ){
		cprintf( "DRIVE FORMAT REQUEST\n" );
	}
#endif
	
	// ドライブのステータスを取得
START:
	{
		int stat;
		stat = sceDevctl( "hdd0:", HDIOC_STATUS, NULL, 0, NULL, 0 );
		cprintf( "HDD DRIVE:" );
		switch( stat ){
		  case 0:
			cprintf( "NORMAL\n" );
			break;
		  case 1:
			cprintf( "UNFORMAT\n" );
			cprintf( "format HDD\n" );
			sceFormat( "hdd0:", NULL, NULL, 0 );
			format_flag = 0;
			break;
		  case 2:
			cprintf( "LOCKED\n" );
			for( ;; );
			break;
		  case 3:
			cprintf( "NOT FOUND\n" );
			for( ;; );
			break;
		  default:
			cprintf( "STATE %d\n", stat );
			for( ;; );
			break;
		}
	}

	// USB初期化
#ifdef USBCHECK
	cprintf( "USB CHECK..\n" );
	if( boot == 0 ){
		for( i = 0; i < 32; i++ ){
			if( usbInit( "" ) == 0 ){
				download_flag = DOWNLOAD_USB;
				break;
			}
			sceGsSyncV( 0 );
		}
	}
#endif
	// NET初期化
	if( ( boot == 0 && download_flag == NO_DOWNLOAD ) || download_flag == DOWNLOAD_NET ){
		int res;
		res = memcard_check( loadbuffer, "KCEJ-NETCONF" );
		if( res != 0 ){
			if( strncmp( loadbuffer, "netfs0:", 6 ) == 0 ){
				// 従来通り
				strcpy( hostparam, loadbuffer );
			} else {
				cprintf( "SELECT HOST ...\n" );
				net_select( hostparam, loadbuffer, 5 );
			}
		}

		cprintf( "NET CHECK " );

		for( i = 0; i < 60 * 30; i++ ){
			struct sce_stat stat;
			int r;
			r = sceGetstat( "netfs0:$check", &stat );
			ReadPad( &pad );
			if( pad.status & SCE_PADRdown ){
				boot = 1;
				format_flag = 0;
				cprintf( "CANCEL\n" );
				goto NEXT;
			}
			if( i % 60 == 0 ) cprintf( "." );
			if( r == -1 ){
				cprintf( "FOUND\n" );

				cprintf( "%s\n", hostparam );

				if( sceGetstat( hostparam, &stat ) >= 0 ){
					download_flag = DOWNLOAD_NET;
					goto NEXT;
				}
				cprintf( "CAN'T CONNECT\n" );
				break;
			}
			sceGsSyncV( 0 );
		}
		cprintf( "NOT FOUND\n" );
		for( i = 0; i < 60 * 3; i++ ){
			sceGsSyncV( 0 );
		}
	}
NEXT:
	if( download_flag != NO_DOWNLOAD && boot == 0 ){
		// USB/NET からのdownload 開始
		if( download_flag != DOWNLOAD_CD ){
			// LOCK FILE CHECK
			int first = 0;

			for( ;; ){
				int i;
				int fd;

				fd = hostOpen( "_LOCK", SCE_RDONLY );
				if( fd < 0 ){
					break;
				}
				// LOCKファイルがあった
				if( first == 0 ){
					cprintf( "LOCK FILE WAIT...\n" );
					first = 1;
				}
				sceClose( fd );
				for( i = 0; i < 60 * 2; i++ ){
					sceGsSyncV( 0 );
				}
			}
		} else {
			cprintf( "SET INSTALL CD/DVD\n" );

			// INSTALL DISCのチェック
			for( ;; ){
				int fd;
				disc_change();
				fd = hostOpen( "./hdinst.cnf", SCE_RDONLY );
				if( fd >= 0 ){
					sceClose( fd );
					break;
				}
			}
		}
		{
			static char *desc[] = {
				"USB", "NET", "DISC",
			};
			cprintf( "%s download start\n", desc[ download_flag - 1 ] );
		}
		download( "./hdinst.cnf", download_flag );
		// 実行に移行した場合は戻ってこない

		cprintf( "end\n" );

		sceUmount( "pfs0:" );
	} else {
		/* 可能な限りmountして実行 */
		mount_and_exec( remove_flag );
	}

	return 0;
}
