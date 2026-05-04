/*
	cdbios.h
		KCEJ CD/DVD MANAGER external common header
		2000/03/22 K.Uehara
	$Id: cdbios.h,v 1.6 2001/08/13 11:02:18 usr01475 Exp $
*/

#ifndef __CDBIOS_H__
#define __CDBIOS_H__

// 外部へ公開する関数と構造体

// status

enum {
	CDBIOS_STATE_IDLE				= 0,
	CDBIOS_STATE_COMMAND_SET		= 0x00080000,	// EEでコマンドを送った
	CDBIOS_STATE_COMMAND_EXECUTING	= 0x00040000,	// コマンド実行中
	CDBIOS_STATE_COMMAND_IOP		= 0x00020000,	// IOPでコマンド実行中
	CDBIOS_STATE_COMMAND_ERROR		= 0x00010000,
};

// read 情報

typedef struct {
	int pos;			// 現在の読み取り位置
	void *ee_buffer;	// EE側の読み込みバッファ
	int read_size;		// 全体の読み取りサイズ
	int	intr_size;		// 現在からの割り込みサイズ(-1:割り込みなし)
	int total_size;		// 終了サイズ
} CDBIOS_READ_STATUS;

#ifdef __cplusplus
extern "C" {
#endif

void cdbios_init( void );			// 初期化

int cdbios_get_status( void );		// ステータス取得
int cdbios_get_read_size( void );	// read の進行取得
#define cdbios_get_read_remain() cdbios_get_read_size() // 互換

void cdbios_read( void *buffer, int pos, int size );
void cdbios_callback_read( void *buffer, int pos, int size
						   , void (*callback)(CDBIOS_READ_STATUS *), int intr_size );

void cdbios_stop( void );
void cdbios_preseek( int pos );
int cdbios_get_filepos( char *name );
void cdbios_set_hd_file( char *name );

#include <libcdvd.h>

int cdbios_get_time( sceCdCLOCK *rtc );

#ifdef __cplusplus
}
#endif

#endif
