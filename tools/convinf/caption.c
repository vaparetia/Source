/*
	字幕データの解析と出力
	$Id: caption.c,v 1.3 2001/07/02 09:19:50 usr01475 Exp $
*/

#include <stdio.h>
#include "infdata.h"
#include "fontconv.h"

// PACKET_SIZE + font_size が MAX_CAPTION_PACKET_SIZE を越えたら終了
// MAX_CAPTION_DATA_SIZEより大きければエラー終了とする

#define MAX_CAPTION_PACKET_SIZE		2048	// これ以上
#define MAX_CAPTION_DATA_SIZE		(5*1024)	// 必ずこれ以下

#define MAX_ACTION_DATA_SIZE		2048

extern int verbose_mode;
#define PRINTF( fmt, arg... )	if( verbose_mode ) printf( fmt, ## arg )

/* ---------------------------------------------------------------------- */
/*
	アクションデータ作成部
*/

typedef struct {
	int start;
	int name;
	int value;
	int option;
} ACTION;

static char action_buffer[ MAX_ACTION_DATA_SIZE ];
static char *action_buffer_p;

static void init_action_buffer( void )
{
	action_buffer_p = action_buffer;
}

static TALK_ACTION *set_action( TALK_ACTION *ap, int end )
{
	// end までのACTIONを出力する
	if( ap == NULL ) return NULL;

	for( ;; ){
		TALK_ACTION *next;

		next = ap;
		if( next == NULL ){
			return NULL;
		}

		if( next->start < end ){
			ACTION act;
			act.start = next->start;
			act.name = next->name;
			act.value = next->value;
			act.option = 0;
PRINTF( "output ACTION %d %X\n", act.value, act.name );
			memcpy( action_buffer_p, &act, sizeof( act ) );
			action_buffer_p += sizeof( act );
			ap = get_next_action( ap );
		} else {
			break;
		}
	}

	return ap;
}

/* ---------------------------------------------------------------------- */
/*
	字幕データ生成部
*/

/* データのチャンク構造 */

typedef struct {
	int start;
	int end;
	int name;
	int len;
	char caption[ 0 ];	// 4 byte align
} CAPTION;

static char caption_buffer[ MAX_CAPTION_DATA_SIZE ];
static char *caption_buffer_p;

static void init_caption_buffer( void )
{
	caption_buffer_p = caption_buffer;
}

static void set_caption( TALK_DIALOG *dp, int name )
{
	CAPTION *cap;
	int len;

	cap = ( CAPTION * )caption_buffer_p;
	cap->start = dp->start;
	cap->end = dp->end;
	cap->name = name;
	font_conv_buffer( cap->caption, dp->string );
	len = strlen( cap->caption ) + 1;
	len = ( len + 3 ) & ~3;		// 4 byte align
	cap->len = len + sizeof( CAPTION );
PRINTF( "CAPTION NAME = %X\n", name );
	caption_buffer_p = caption_buffer_p + sizeof( CAPTION ) + len;
}

static int get_caption_data_size( void )
{
	int size;
	size = sizeof( CAPTION ) + sizeof( int );	// ヘッダ
	size += caption_buffer_p - caption_buffer;	// 字幕データ本体
	size += action_buffer_p - action_buffer;	// アクションデータ本体
	size += font_get_data_size();		// フォントデータ

	return size;
}

/* ---------------------------------------------------------------------- */
/*
	ストリームデータ出力部
*/

/* ストリーミングデータヘッダ */

typedef struct {
	int type;
	int size;
	int time;
	int option;
} STREAM_TAG;

static void output_caption( FILE *out )
{
	/* データをストリームパケットとして出力 */
	STREAM_TAG tag;
	CAPTION *cap;
	int size, bufsize, actsize;
	int fpos;

	if( caption_buffer_p == caption_buffer ){
		// なにもしない
		return;
	}
PRINTF( "OUTPUT CAPTION\n" );
	cap = ( CAPTION * )caption_buffer;	// 先頭

	size = get_caption_data_size();
	size = ( size + 15 ) & ~15;			// 16 バイト境界にそろえる
PRINTF( "SIZE = %d\n", size );
	tag.type = 0;
	tag.size = size;
	tag.time = cap->start;				// 先頭パケットの時間

	actsize = action_buffer_p - action_buffer;

	if( actsize == 0 ){
		tag.option = 0;
	} else {
		tag.option = actsize;
	}

	if( tag.size >= MAX_CAPTION_DATA_SIZE ){
		printf( "ERROR !!: too large packet %d\n", tag.size );
		exit( 1 );
	}

	bufsize = caption_buffer_p - caption_buffer + actsize;
	
	fwrite( &tag, sizeof( STREAM_TAG ), 1, out );
	fwrite( &bufsize, sizeof( int ), 1, out );
	fwrite( caption_buffer, sizeof( char ), bufsize - actsize, out );
	if( actsize > 0 ){
		fwrite( action_buffer, sizeof( char ), actsize, out );
	}
	font_output_font_data( out );

	// 16 バイトアライメントにそろえる
	fpos = ftell( out );
	if( fpos % 16 > 0 ){
		static char dummy[ 16 ] = { 0 };
		fwrite( dummy, sizeof( char ), 16 - fpos % 16, out );
	}
PRINTF( "END %ld\n", ftell( out ) );
}

void make_caption_data( char *filename )
{
	/* 字幕データを作成する */
	TALK_DIALOG *dp;
	TALK_ACTION *ap;
	FILE *out;

	if( ( out = fopen( filename, "wb" ) ) == NULL ){
		printf( "FILE %s open error\n", filename );
		exit( 1 );
	}

	font_reset_font_table();
	init_caption_buffer();
	init_action_buffer();

	dp = get_dialog_top();
	ap = get_action_top();

	for( ; dp != NULL; dp = get_next_dialog( dp ) ){
		TALK_AREA *area;
		int name;
		area = get_talk_area( dp->start, dp->end );
		if( area == NULL ){
			/* ERROR !! */
			printf( "warning: NO TALK AREA %s\n", dp->string );
			name = 0;
		} else {
			name = area->name;
		}
		set_caption( dp, name );
		ap = set_action( ap, dp->end );
		if( get_caption_data_size() > MAX_CAPTION_PACKET_SIZE ){
			output_caption( out );

			font_reset_font_table();
			init_caption_buffer();
			init_action_buffer();
		}
	}

	while( ( ap = set_action( ap, 0x7FFFFFFF ) ) != NULL ){
		;
	}
	output_caption( out );

	fclose( out );
}
