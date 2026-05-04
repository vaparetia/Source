//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	codectbl.c
		無線テーブル管理ルーチン
	2000/07/17	K.Uehara
	$Id: codectbl.c,v 1.1.1.3 2002/11/19 11:45:01 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"

/*
	無線呼び出しテーブルの管理
	GCLで毎回再設定されるのが前提なので
	常駐部に置く必要はない。
*/

typedef struct {
	int freq;
	int code;
} CODEC_DEF;

#define MAX_FREQS	16		// 周波数の個数の最大

/*
	基本テーブル
*/

static CODEC_DEF normal_table[ MAX_FREQS ];

/*
	トラップにより上書きされるテーブル
*/

static CODEC_DEF override_table[ MAX_FREQS ];

/* ---------------------------------------------------------------------- */
/*
	外部関数
*/

void CODEC_InitTable( void )
{
	int i;
	for( i = 0; i < MAX_FREQS; i++ ){
		normal_table[ i ].freq = 0;
		override_table[ i ].freq = 0;
	}
}

void CODEC_ClearTable( void )
{
	CODEC_InitTable();
}

static CODEC_DEF *search_codec_table( CODEC_DEF *table, int freq )
{
	int i;
	int new;

	new = -1;

	/* freqに設定されているテーブルか,空き領域を返す */
	for( i = 0; i < MAX_FREQS; i++ ){
		if( table[ i ].freq == freq ){
			new = i;
			break;
		} else if( table[ i ].freq == 0 ){
			new = i;
		}
	}
	if( new < 0 ){
		printf( "RADIO TABLE OVER!!\n" );
		return NULL;
	}
	return &table[ new ];
}

static void set_codec_call( CODEC_DEF *table, int freq, int code )
{
	CODEC_DEF *cp;

	if( ( cp = search_codec_table( table, freq ) ) != NULL ){
		if( ( code & 0x80000000 ) == 0 ){
			/* set */
			cp->freq = freq;
			cp->code = code;
		} else {
			/* clear */
			if( ( code & ~0x80000000 ) == cp->code ){
				cp->freq = 0;
				cp->code = 0;
			}
		}
	}
}

void CODEC_SetBaseCall( int freq, int code )
{
	set_codec_call( normal_table, freq, code );
}

void CODEC_SetOverCall( int freq, int code )
{
	set_codec_call( override_table, freq, code );
}

int CODEC_GetCode( int freq )
{
	CODEC_DEF *cp;
	int i;
	static CODEC_DEF *table[ 2 ] = { override_table, normal_table };

	for( i = 0; i < 2; i++ ){
		if( ( cp = search_codec_table( table[ i ], freq ) ) != NULL ){
			if( cp->freq == freq ){
				return cp->code;
			}
		}
	}
	return -1;	
}
