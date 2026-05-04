//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	select.c
		ファイルシステムのセレクタ

	1999/05/27 K.Uehara
	$Id: select.c,v 1.1.1.3 2002/11/19 11:42:40 Yoshizawa1 Exp $
*/

#define _HD_

#ifdef PSX2 //BP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <libcdvd.h>
#include <sifrpc.h>
#include <sifdev.h>

#include "mts.h"
#include "libgv.h"
#include "libgv.cnf"
#include "cdbios.h"
#include "libfs.h"

#include "file.cnf"
#include "sonycode.h"

#if defined( _USB_ ) || defined( _HDU_ )
#include "usbfs.h"
#endif

/*
	ここで宣言される関数
		// 基本
		void FS_StartDaemon( void );
		void FS_ResetIOP( void );
		void FS_LoadIopModules( char *modules[] );

		// デバッグ用
		void FS_CdStageSet( char *stage );
		void FS_CdStageProgBinFix( void );

		// ストリーム用（inline化する？不要？）
		int FS_GetStreamTop( int file_id, int offset );

	コンパイル時に指定されるdefineリスト
		// 基本
		_HD_		開発機
		_CD_		実機
		_USB_		USBファイルシステム
		_HDU_		PS2HDUを使った開発環境

		// その他
		_HDDEBUG_	各環境のデバッグ用
		_CDSTREAM_	開発機環境でストリームをCDから
*/

// 各モードに対するコンフィグレーション

#define HDU_DATA_PATH	"pfs0:/mgs2/"
#define CD_DATA_PATH	"cdrom0:\\MGS2\\"
#define HOST_DATA_PATH	"host0:./"
#define USB_DATA_PATH	"usb0:./"

// モジュールの設定

#ifdef DESIGN_PREVIEW
#define HOSTPATH( a )	"host0:../mgs2/cdrom.img/modules/" a
#else
#define HOSTPATH( a )	"host0:./modules/" a
#endif

#define HDUPATH( a )	"pfs0:./modules/" a
#define CDPATH( a )		"cdrom0:\\MODULES\\" a ";1"

// HDUのみ。HDDをマウントするためのirxはCD/DVD/host0から読み込む。
#if defined( _HDU_ ) && defined( _HDDEBUG_ )
static char system_pak[] = HOSTPATH( "syshd.ipk" );
#elif defined( _HDU_ )
static char system_pak[] = CDPATH( "SYSHD.IPK" );
#endif

#if defined( _HD_ )

static char modules_pak[] = HOSTPATH( "modhd.ipk" );

#elif defined( _CD_ ) && defined( _HDDEBUG_ )

static char modules_pak[] = HOSTPATH( "module.ipk" );

#elif defined( _CD_ )

static char modules_pak[] = CDPATH( "MODULE.IPK" );

#elif defined( _USB_ ) && defined( _HDDEBUG_ )

static char modules_pak[] = HOSTPATH( "modusb.ipk" );

#elif defined( _USB_ )

static char modules_pak[] = CDPATH( "MODUSB.IPK" );

#elif defined( _HDU_ ) && defined( _HDDEBUG_ )

static char modules_pak[] = HOSTPATH( "modhdu.ipk" );

#elif defined( _HDU_ )

static char modules_pak[] = HDUPATH( "modhdu.ipk" );

#endif

#if ( !defined( _HD_ ) && !defined( _HDDEBUG_ ) )
#define LOADERMODULE	"cdrom0:\\MODULES\\PLD.IRX;1"
#define IOP_REP_IMG		"cdrom0:\\MODULES\\" IOP_IMAGE_FILE ";1"
#else

#ifdef DESIGN_PREVIEW
#define LOADERMODULE	"host0:../mgs2/module/irx/pld.irx"
#else
#define LOADERMODULE	"host0:../module/irx/pld.irx"
#endif

#define IOP_REP_IMG		"host0:/usr/local/sce/iop/modules/" IOP_IMAGE_file
#endif

#if defined( MEDIA_DVD ) && ( defined( _CD_ ) || defined( _HD_ ) )
// hd, hdc, cd, cddモード時のみ
#define MEDIA_MODE		SCECdDVD
#else
#define MEDIA_MODE		SCECdCD
#endif

int FS_current_layer = 0;

/* ---------------------------------------------------------------------- */
/*
	下請け
*/

// パーティション名を生成してパーティションをmountする。

#if !defined( _HD_ ) && !defined( _CD_ )

static int mount_hdd( char *fs )
{
	char mount_str[ 128 ];
#if defined( _HDU_ )
	extern int main_argc;
	extern char **main_argv;

	if( main_argc <= 1 ){
		// OLD VERSION
		sprintf( mount_str, "hdd0:mgs20920,fpwd" );
	} else {
		strcpy( mount_str, main_argv[ 1 ] );
	}
#else
	// USB, CD
	extern int _compiled_time[];
	sprintf( mount_str, "hdd0:PP.%s.%08X.MGS2HD", SONY_CODE, _compiled_time[ 0 ] );
#endif
	return sceMount( fs, mount_str, SCE_MT_RDONLY, NULL, 0 );
}
#endif

/* ---------------------------------------------------------------------- */
/*
	void FS_ResetIOP( void );
		IOPモジュールのリセット
		メディアタイプのセット
*/

void FS_ResetIOP( void )
{
#if 0//BP_PS2 No IOP modules get loaded for us
	sceCdInit( SCECdINIT );

#if !defined( _HD_ ) && !defined( _HDDEBUG_ )
	// IOPリブート
	sceCdMmode( MEDIA_MODE );

#if !(defined( _USB_ ) && !defined( _HDDEBUG_ ))
	// よくわからないがver2.5.2からUSB環境では必要が無くなった。
	while( !sceSifRebootIop( IOP_REP_IMG ) );
#endif

	while( !sceSifSyncIop() );

	sceSifInitRpc( 0 );
	sceCdInit( SCECdINIT );
	sceFsReset();
#endif

	sceCdMmode( MEDIA_MODE );
#endif
}

/* ---------------------------------------------------------------------- */
/*
	void FS_LoadIopModules( void );
		IOPモジュールの読み込み
*/

// 下請け

static void load_modules( char *fname, void *buffer )
{
   BP_TODO_BREAK;
#if 0//BP_PS2

	void *ptr = buffer;

	int size, rd;
	int fd;
printf( "load module %s\n", fname );
	do {
		fd = sceOpen( fname, SCE_RDONLY );
		ASSERT( fd >= 0 );
	} while( fd < 0 );
	size = sceLseek( fd, 0, SCE_SEEK_END );
	sceLseek( fd, 0, SCE_SEEK_SET );
	rd = sceRead( fd, ptr, size );
	ASSERT( rd == size );
	sceClose( fd );

	ptr = ptr + size - 16;

	FlushCache( 0 );

#endif
}

void FS_LoadIopModules( void *addr )
{
#if 0//BP_PS2 No IOP modules get loaded for us

	char buf[ 16 ];
	int r, res;

	sprintf( buf, "%08x", addr );

#if defined( _HDU_ )
	// HDU時のみHDD用のモジュールはCD/DVDから読み込む
	load_modules( system_pak, addr );
	do {
		r = sceSifLoadStartModule( LOADERMODULE, 9, buf, &res );
		ASSERT( res >= 0 );
	} while( r < 0 );

	// それ以外のモジュールはHDDから読み込むので, Mountしておく
	if( mount_hdd( "pfs0:" ) != 0 ){
		ASSERT( FALSE );
	}
#endif

	load_modules( modules_pak, addr );
	do {
		r = sceSifLoadStartModule( LOADERMODULE, 9, buf, &res );
		ASSERT( res >= 0 );
	} while( r < 0 );

#if defined( _HDU_ )
	// 一度unmountする
	sceUmount( "pfs0:" );
#endif

#endif
}

/* ---------------------------------------------------------------------- */
/*
	void FS_StartDaemon( void );
		各ファイル環境の初期化
		各ストリームをどのメディアから読み込むかの設定
		各ファイル先頭テーブルの初期化
*/

void fs_str_toupper( char *str )
{
	for( ; *str != '\0'; str ++ ){
		*str = toupper( *str );
	}
}

void FS_CdFileSetup( FS_FILE_INFO *finfo )
{
	for( ; finfo->name != NULL; finfo ++ ){
		char buf[ 64 ];
#if defined( _CD_ ) || defined( _HDU_ )
		if( finfo->layer != FS_current_layer ) continue;
#endif
		switch( finfo->mode ){
		  case FS_FILE_MODE_CD:
			sprintf( buf, CD_DATA_PATH"%s;1", finfo->name );
			fs_str_toupper( buf + 7 );		// cdrom0は小文字
#ifdef PSX2
			buf[ 5 ] = '0' + finfo->layer;
#endif
			break;
		  case FS_FILE_MODE_HDU:
			sprintf( buf, HDU_DATA_PATH"%s", finfo->name );
			break;
		  case FS_FILE_MODE_DEV:
#if defined( _USB_ )
			sprintf( buf, USB_DATA_PATH"%s", finfo->name );
#else
			sprintf( buf, HOST_DATA_PATH"%s", finfo->name );
#endif
			break;
		}
#if 0 //BP_PS2
		finfo->pos = cdbios_get_filepos( buf );
#endif
printf( "FILE %12s POS %08X (%s)\n", finfo->name, finfo->pos, buf );
	}
}

void FS_StartDaemon( void )
{
#if defined( _HD_ )
	// 開発環境
	{
		int i;
		int mode;

#if defined( _STREAM_CD_ )
		mode = FS_FILE_MODE_CD;
#elif defined( _STREAM_HDU_ )
		mode = FS_FILE_MODE_HDU;
#else
		mode = FS_FILE_MODE_DEV;
#endif
		for( i = 0; i < FS_FILE_STREAM_TOP; i++ ){
			FS_SET_FILE_MODE( i, FS_FILE_MODE_DEV );
		}
		for( i = FS_FILE_STREAM_TOP; i < FS_FILE_MAX; i++ ){
			FS_SET_FILE_MODE( i, mode );
		}
	}
#else
	// HD以外
#if defined( _USB_ )
	// USBの時はHDがつながってなかったらUSBから読み込む
	{
		int i;

		for( i = 0; i < FS_FILE_MAX; i++ ){
			FS_SET_FILE_MODE( i, FS_FILE_MODE_DEV );
		}
	}
#endif
	// USB以外の時はCDから読み込む
#if !( defined( _CD_ ) )
	// 日本語版もHDD非対応にした。
	if( mount_hdd( "pfs0:" ) == 0 ){
		// パーティションが存在する
		// 各ファイルについて存在確認を行ない, HDUにある場合はそちらから読み込む
		// なければCDから
		int i;
		for( i = 0; i < FS_FILE_MAX; i++ ){
			char file[ 64 ];
			struct sce_stat stat;
			sprintf( file, HDU_DATA_PATH"%s", fs_file_info[ i ].name );
			if( sceGetstat( file, &stat ) == 0 ){
				FS_SET_FILE_MODE( i, FS_FILE_MODE_HDU );
			} else {
#if defined( _USB_ )
				FS_SET_FILE_MODE( i, FS_FILE_MODE_DEV );
#else
				FS_SET_FILE_MODE( i, FS_FILE_MODE_CD );
#endif
			}
		}
	}
#endif	// !_CD_ && ENGLISH
#endif	// !_HD_
	cdbios_init();

	FS_current_layer = 0;
	FS_CdFileSetup( fs_file_info );

#if defined( _HD_ )
	// HD環境の初期化
	printf( "HD init\n" );
	FS_HDInit();
	//BP_PS2 FS_HdStageFileInit();
#elif defined( _USB_ )
	// USB環境の初期化
	printf( "usb init\n" );
	FS_USBInit();
	//BP_PS2 FS_HdStageFileInit();

#elif defined( _CD_ ) || defined( _HDU_ )
	// CD環境の初期化
	printf( "CD init\n" );
	FS_CdInit();
#ifdef PSX2
	// DUAL LAYER
	while( cdbios_get_status() != 0 );

	FS_current_layer = 1;
	FS_CdFileSetup( fs_file_info );
	FS_CdStageFileInit( 0 );

	FS_CdStageFileInit( 1 );
	FS_current_layer = 0;
#else
	FS_CdStageFileInit( 0 );
#endif

#endif

#if defined( _USB_ )
	{
		int i, usb_ok;
		usb_ok = 0;
		for( i = 0; i < 30; i++ ){
			// USBの初期化
			if( usbInit( "" ) == 0 ){
				usb_ok = 1;
				break;
			}
			sceGsSyncV( 0 );
		}
		if( usb_ok ){
			printf( "stdout redirect to usbfs.\n" );
			MTS_SetPrintFunc( ( void * )usbPuts );
		}
	}
#endif

#if defined( _HDU_ ) || defined( _USB_ )
	sceCdStop();
#endif

	FS_SlotSystemInit();
   FS_StreamSystemInit();
}

#if defined( _CD_ ) || defined( _HDU_ )

/* CDファイルシステムのデバッグ時用 */
/* ステージのバイナリファイルをHDから読み込む */

#ifdef _HDDEBUG_
static char stage_name[ 16 ];
#endif

void FS_CdStageSet( char *stage )
{
#ifdef _HDDEBUG_
	strcpy( stage_name, stage );
#endif
}

void FS_CdStageProgBinFix( void )
{
#ifdef _HDDEBUG_
	char path[ 64 ];
	int fd;
	int size;
	extern int _mgs2_keep_end[];

	sprintf( path, "host0:stage/%s/%s.bin", stage_name, stage_name );
	if( ( fd = sceOpen( path, SCE_RDONLY ) ) < 0 ){
		printf( "FIX BIN FAILED\n" );
		return;
	}
	size = sceLseek( fd, 0, SEEK_END );
	sceLseek( fd, 0, SEEK_SET );

	sceRead( fd, _mgs2_keep_end, size );
	sceClose( fd );
#endif
	FlushCache( INVALIDATE_ICACHE );
}

#endif

int FS_GetDiscStatus( void )
{
	return cdbios_get_status();
}

int FS_MediaType( void )
{
#if defined( _CD_ )
	return FS_MEDIA_DISC;
#endif
#if defined( _HD_ )
	return FS_MEDIA_DEV;
#endif
#if defined( _USB_ )
	return FS_MEDIA_USB;
#endif
#if defined( _HDU_ )
	return FS_MEDIA_HDU;
#endif
}

#endif //PSX2 BP
