/*
	select.c
		ファイルシステムのセレクタ

	1999/05/27 K.Uehara
	$Id: selectx.c,v 1.1.1.3 2002/11/19 11:42:40 Yoshizawa1 Exp $

	2000/04/06 M.Kobayashi
	とりあえず最初のバージョンではHDのみサポート。したがってこのファイルは形骸化。
	Xの場合はメディアによるAPIの差異が小さいのでPS2より統一的に扱えるだろう。
*/

#ifdef KP_XBOX //BP

#include <stdio.h>
#ifndef KP_XBOX
#include <string.h>
#include <windows.h>
#endif

#include "libgv.h"
#include "libfs.h"
#include "cdbios.h"

#include "file.cnf"

/*
	ここで宣言される関数
		// 基本
		void FS_StartDaemon( void );

	void FS_StartDaemon( void );
		各ファイル環境の初期化
		各ストリームをどのメディアから読み込むかの設定
		各ファイル先頭テーブルの初期化
*/

/* ---------------------------------------------------------------------- */
/*
	そのほか
*/

// ダミー関数

void FS_CdStageSet( char *stage )
{
}

void FS_CdStageProgBinFix( void )
{
}

int FS_current_layer = 0;

/* ---------------------------------------------------------------------- */
/*
	StartDaemon
*/

extern void cdbios_init(void);

void FS_StartDaemon( void )
{
	int i;

#ifdef _DEV_
	cdbios_dev_init();

	printf( "DEV init\n" );

	FS_DevInit();

	for( i = 0; i < FS_FILE_MAX; i++ ){
		FS_SET_FILE_MODE( i, FS_FILE_MODE_DEV );
	}
#endif

#ifdef _HDU_
	cdbios_init();
	printf( "HD init\n" );

	// HD環境の初期化
	FS_HDInit();
	for( i = 0; i < FS_FILE_MAX; i++ ){
		FS_SET_FILE_MODE( i, FS_FILE_MODE_HDU );
	}
#endif

#ifdef _XDAT_
	cdbios_xdat_init();
	printf( "HD init\n" );

	// HD環境の初期化
	FS_HDInit();
	for( i = 0; i < FS_FILE_MAX; i++ ){
		FS_SET_FILE_MODE( i, FS_FILE_MODE_HDU );
	}
#endif

//	FS_SET_FILE_MODE( FS_FILE_DEMO, FS_FILE_MODE_HDU );
//	FS_SET_FILE_MODE( FS_FILE_MOVIE, FS_FILE_MODE_HDU );
//	FS_SET_FILE_MODE( FS_FILE_VOX, FS_FILE_MODE_HDU );

	FS_CdFileSetup( fs_file_info );

#if defined( _HDU_ ) || defined( _XDAT_ )
	FS_CdStageFileInit( 0 );
#endif
}

int FS_GetDiscStatus( void )
{
	return cdbios_get_status();
}

#endif //KP_XBOX BP