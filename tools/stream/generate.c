/*
   ストリームデータ作成ルーチン
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>

#include "stream.h"
#include "parse.h"

int no_output_mode = 0;
int disc_num_prefix = 0;

typedef unsigned int filepos_t;

/* 入力される各データのチャンク先頭にあるヘッダフォーマット */

typedef struct {
	int reserved;
	int size;
	int start_time;
	int option;
} DATA_HEADER;

/* 出力データの管理用構造体 */

typedef struct _chank_data {
	FILE *infile;
	int type;
	int next_tick;
	int tick_start_offset;
	int size;
	int option;
	int ( *output_func )( struct _chank_data *chank, int tick );
	char filename[ 256 ];
	/* 以下サウンド用 */
	int sound_freq;
	int sound_channel;
	int sound_format;
	/* 以下汎用 */
	DATA_HEADER next;
} CHANK_DATA;

/* 出力データのチャンクヘッダ */

typedef struct {
	int type;
	int size;
	int tick;
	int option;
} STREAM_TAG;

#define MAX_CHANK_TYPES	16

#define ALIGN16( a )	( ( (a)+15 ) & ~15 )

#define GET_TYPE( a )	( (a) & 0xFFFF )
#define TYPE_SOUND	1
#define TICK_PER_SEC	300.0F

/*
   内部変数
*/

static FILE *stream_fp;
static FILE *table_fp;

static int tag_flags;
#define TAGFLAG_SOUND_8BIT	0x40000000
#define TAGFLAG_SOUND_VAG	0x20000000
#define TAGFLAG_SOUND_AC3	0x10000000

static float total_time = 0.0F;

static int type_total[ 16 ];
static int padding_size = 0;

/* ---------------------------------------------------------------------- */
/*
	内部関数
*/

static void *Malloc( int size )
{
	void *ptr;
	if( ( ptr = malloc( size ) ) == NULL ){
		ERROR( "NO MEMORY\n" );
	}
}

static void Free( void *ptr )
{
	free( ptr );
}

static void output_chank( int type, void *buffer, int size, int tick, int option )
{
	STREAM_TAG tag;

	size = ALIGN16( size );
	tag.type = type;
	tag.size = size + sizeof( STREAM_TAG );
	tag.tick = tick;
	tag.option = option;

	type_total[ GET_TYPE( tag.type ) ] += tag.size;

	fwrite( &tag, sizeof( STREAM_TAG ), 1, stream_fp );

	if( size > 0 ){
		fwrite( buffer, sizeof( char ), size, stream_fp );
	}
}

/*
   システムデータ（ストリーム開始データ）出力
*/

static void output_system_chank( int type, int time )
{
	STREAM_TAG tag;

	tag.type = CHANK_TYPE_SYSTEM;
	tag.size = sizeof( STREAM_TAG );
	tag.tick = time;
	tag.option = type;
	fwrite( &tag, sizeof( STREAM_TAG ), 1, stream_fp );
}

/*
   ストリーミングデータ構造
*/

#define MAX_DATA_NUM 16

typedef struct {
	int type;
	int tick_start_offset;
	char filename[ 128 ];
} STR_DATA;

static STR_DATA str_data[ MAX_DATA_NUM ];
static int str_data_p;

static void init_str_data( void )
{
	str_data_p = 0;
}

static void set_str_data( int type, char *filename, int tick_offset )
{
	STR_DATA *p;
	int i, ch;

	/* ファイルの正当性チェック */
	{
		struct stat st;
		if( stat( filename, &st ) < 0 ){
			printf( "Warning: %s not found\n", filename );
			return;
		}
		if( st.st_size <= 0 ){
			printf( "warning: size 0 file %s\n", filename );
			return;
		}
	}

PRINTF( "SET_STR_DATA %X %s\n", type, filename );
	if( str_data_p >= MAX_DATA_NUM ){
		ERROR( "DATA NUM OVER !!\n" );
	}
#if 0
	ch = 0;
	for( i = 0; i < str_data_p; i++ ){
		if( GET_TYPE( str_data[ i ].type ) == type ){
			/* 同じデータタイプがあればchが複数と考える */
			ch++;
		}
	}

	type = type | ( ch << 16 );
#endif

	p = &( str_data[ str_data_p ] );
	p->type = type;
	p->tick_start_offset = tick_offset;

	strcpy( p->filename, filename );

	str_data_p ++;
}

static STR_DATA *get_str_data( int no )
{
	if( no >= str_data_p ) return NULL;
	return str_data + no;
}

static int get_str_data_num( void )
{
	return str_data_p;
}

/* ---------------------------------------------------------------------- */
/*
   サウンドストリーミング
*/

#define SOUND_ONE_READ_SIZE 0x4000
//#define SOUND_PRE_LOAD_SIZE ( SOUND_ONE_READ_SIZE * ( dp->sound_channel ) )

static int sound_duration;
static int sound_file_size;

static int output_sound_chank( CHANK_DATA *dp, int tick )
{
	if( tick == 0 ){
		/* サウンドスタート情報出力 */
		unsigned char *buf;
		int i;
		int wav_len;

		/* サウンドヘッダ出力 */
		buf = Malloc( 2048 );

		if( fread( buf, sizeof( char ), 2048, dp->infile ) < 2048 ){
			ERROR( "サウンドヘッダがロードできません\n" );
		}

		output_chank( dp->type, buf, 16, 0, 0 );

		dp->next_tick = 0;
		dp->size = 0;

		dp->sound_freq = ( ( buf[ 6 ] << 8 ) | buf[ 7 ] );
		dp->sound_channel = buf[ 8 ];
		dp->sound_format = buf[ 10 ];

		wav_len = ( buf[ 0 ] << 24 ) | ( buf[ 1 ] << 16 ) | ( buf[ 2 ] << 8 ) | buf[ 3 ];
		sound_file_size = wav_len;

		Free( buf );

		if( dp->sound_format == 0 ){
			/* VAG */
			tag_flags |= TAGFLAG_SOUND_VAG;
			sound_duration = TICK_PER_SEC * ( wav_len / 16 * 28 )
				/ ( dp->sound_freq * dp->sound_channel );
		} else if( dp->sound_format == 1 ){
			/* 8Bit */
			tag_flags |= TAGFLAG_SOUND_8BIT;
			sound_duration = TICK_PER_SEC * wav_len / ( dp->sound_freq * dp->sound_channel );
		} else if( dp->sound_format == 2 ){
			/* AC3 */
			tag_flags |= TAGFLAG_SOUND_AC3;
			sound_duration = ( TICK_PER_SEC * wav_len * 3.0F
							   / ( dp->sound_freq * 2.0F * 2.0F ) );
		} else if( dp->sound_format == 0x11 ){
			if( dp->sound_channel > 2 ) {
				tag_flags |= TAGFLAG_SOUND_AC3;
			} else {
				tag_flags |= TAGFLAG_SOUND_VAG;
			}
			sound_duration = TICK_PER_SEC * ( wav_len / 0x800 * ( 0x7e0 / 36 * 64 ) )
				/ ( dp->sound_freq * dp->sound_channel );
		} else {
			ERROR( "sound format Error %d\n", dp->sound_format );
		}
PRINTF( "sound start %d freq %d chanl %d form %d\n"
		, 0, dp->sound_freq, dp->sound_channel, dp->sound_format );

/*printf( "total size %d\n", *( int* )buf );*/

		/* 先読み分のデータブロックを出力しておく */

//		for( i = 0; i < dp->sound_channel; i++ ){
		if( dp->sound_channel > 2 ) {
			int size;
			buf = Malloc( SOUND_ONE_READ_SIZE * dp->sound_channel / 2 );
			size = fread( buf, sizeof( char ), SOUND_ONE_READ_SIZE * dp->sound_channel / 2 , dp->infile );
			if( size > 0 ){
				output_chank( dp->type, buf, size, tick, 0 );
			}
			
			Free( buf );
			dp->size += size;
			if( size < SOUND_ONE_READ_SIZE * dp->sound_channel / 2 ){
				return 0;
			}
		} else {
			for( i = 0; i < 2; i++ ){
				int size;
				buf = Malloc( SOUND_ONE_READ_SIZE );
				size = fread( buf, sizeof( char ), SOUND_ONE_READ_SIZE, dp->infile );
				if( size > 0 ){
					output_chank( dp->type, buf, size, tick, 0 );
				}
				
				Free( buf );
				dp->size += size;
				if( size < SOUND_ONE_READ_SIZE ){
					return 0;
				}
			}
		}
	}

	if( tick >= dp->next_tick ){
		/* サウンド本体出力 */
		char *buf;
		int size;
		int readsize;

		if( dp->sound_channel > 2 ) {
			readsize = SOUND_ONE_READ_SIZE * dp->sound_channel / 2;
		} else {
			readsize = SOUND_ONE_READ_SIZE;
		}

		buf = Malloc( readsize );
		size = fread( buf, sizeof( char ), readsize, dp->infile );
		if( size > 0 ){
PRINTF( "SOUND TICK %d size %d\n", dp->next_tick, size );
			output_chank( dp->type, buf, size, tick, 0 );
		}

		Free( buf );
		dp->size += size;
		if( dp->sound_format == 0 ){
			// VAG FORMAT
#if 0
			dp->next_tick = ( int )( ( float )TICK_PER_SEC *
									 ( ( ( ( float )dp->size - SOUND_ONE_READ_SIZE
										   * dp->sound_channel ) * 28 / 16 )
									   / dp->sound_channel ) / ( dp->sound_freq ) );
#else
			int offset;

			if( dp->sound_channel > 1 ){
				offset = SOUND_ONE_READ_SIZE * 2;
			} else {
				offset = SOUND_ONE_READ_SIZE + 0x800 * 2;
			}

			dp->next_tick = ( int )( ( float )TICK_PER_SEC *
									 ( ( ( ( float )dp->size - offset ) * 28 / 16 )
									   / dp->sound_channel ) / ( dp->sound_freq ) );
#endif
		} else if( dp->sound_format == 1 ){
			// 8BIT
			dp->next_tick = ( int )( ( float )TICK_PER_SEC *
									 ( ( ( ( float )dp->size
										   - SOUND_ONE_READ_SIZE * dp->sound_channel ) )
									   / dp->sound_channel ) / ( dp->sound_freq ) );
		} else if( dp->sound_format == 2 ){
			// AC3
			dp->next_tick = ( int )( ( float )TICK_PER_SEC *
									 ( ( ( ( float )dp->size
										   - SOUND_ONE_READ_SIZE ) * 3.0F )
										 / ( dp->sound_freq * 2.0F * 2.0F ) ) );
		} else if( dp->sound_format == 0x11 ){
			int offset;
			if( dp->sound_channel > 2 ) {
				offset = SOUND_ONE_READ_SIZE * dp->sound_channel / 2;
			} else if( dp->sound_channel > 1 ){
				offset = SOUND_ONE_READ_SIZE * 2;
			} else {
				offset = SOUND_ONE_READ_SIZE + 0x800 * 2;
			}
			// X は 36 byte / block, 64 sample / block
			// 0x800 単位に block を詰めて余りは０パディングしている(0x7e0以降)
			// 結局ＰＳ２と一緒（バイト数 * 1.75 = サンプル数）
			dp->next_tick = ( int )( ( float )TICK_PER_SEC *
									 ( ( ( ( float )dp->size - offset ) / 0x800 * ( 0x7e0 / 36 * 64 ) )
									   / dp->sound_channel ) / ( dp->sound_freq ) );
		}
		if( size < readsize ){
			return 0;
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------- */
/*
	汎用ストリーミングルーチン
	( demo, lip, etc .. )
*/

static void output_data_body( int type, void *buffer, int size, int tick, int option )
{
	output_chank( type, buffer, size, tick, option );
}

static int get_next_data_tick( CHANK_DATA *dp )
{
	/* 次のデータのヘッダを読み込んでおく */
	int r;
	if( ( r = fread( &dp->next, sizeof( DATA_HEADER ), 1, dp->infile ) )
		< 1 ){
		/* 読み込みに失敗 */
		/*		WARNING( "%s データヘッダが読み込めません %d\n", dp->filename, r );*/
		return 0;
	}
	dp->next_tick = dp->next.start_time + dp->tick_start_offset;
	dp->size = dp->next.size;
	dp->option = dp->next.option;

	if( feof( dp->infile ) ){
		return 0;
	}
PRINTF( "pos %d next size %d tick %d\n", ftell( dp->infile ), dp->size, dp->next_tick );
	return 1;
}

static int output_data_chank( CHANK_DATA *dp, int tick )
{
	int next;

	next = dp->next_tick;
	if( tick < next ){
		return 1;
	}
	while( tick >= next ){
		char *buf;
		int size, len;
PRINTF( "DATA TICK %d size %d\n", dp->next_tick, dp->size );
		buf = Malloc( dp->size );
		size = dp->size - sizeof( DATA_HEADER );
		if( size < 0 ){
			ERROR( "ファイル'%s'が途中で終わっています\n", dp->filename );
		}

		if( ( len = fread( buf, sizeof( char ), size, dp->infile ) ) < size ){
			ERROR( "ファイル'%s'が途中で終わっています。size %d read %d\n", dp->filename, size, len );
		}

		output_data_body( dp->type, buf, size, dp->next_tick, dp->option );

		Free( buf );

		if( ! get_next_data_tick( dp ) ){
			return 0;
		}
		next = dp->next_tick;
	}
	return 1;
}



/* ---------------------------------------------------------------------- */
/*
	ファイルのオープンクローズ
*/

static void set_data_file( CHANK_DATA *dp, STR_DATA *s )
{
	dp->type = s->type;
	if( ( dp->infile = fopen( s->filename, "rb" ) ) == NULL ){
		ERROR( "File %s is Not Found.\n", s->filename );
	}
	strcpy( dp->filename, s->filename );
	dp->tick_start_offset = s->tick_start_offset;

	if( GET_TYPE( s->type ) == TYPE_SOUND ){
		/* サウンド */
		dp->output_func = output_sound_chank;
	} else {
		/* その他 */
		dp->output_func = output_data_chank;
		/* 先頭のチャンクの情報を取得 */
		if( get_next_data_tick( dp ) == 0 ){
			ERROR( "ファイル %s が不正です\n", dp->filename );
		}
	}
}

static void close_data_file( CHANK_DATA *dp )
{
	fclose( dp->infile );
}

/* ---------------------------------------------------------------------- */
/*
	ストリーミングファイル作成
*/

static void trim_sector_alignment( FILE *fp )
{
	/* sector alignment にあわせる */

	filepos_t pos;
	int size;

	pos = ftell( fp );

	if( pos % SECTOR_SIZE > 0 ){
		int i;
		size = SECTOR_SIZE - ( pos % SECTOR_SIZE );
		padding_size += size;
		for( i = size; i > 0; i-- ){
			fputc( 0, fp );
		}
	}
}

static void make_stream_data( void )
{
	/*
		tick は、1/300sec単位。
	*/
	int i;
	int tick;
	filepos_t pos;
	filepos_t prev, prev_pos;
	int maxband;
	int num;
	int do_flag;
	STR_DATA *str = NULL;

	static CHANK_DATA chank[ MAX_CHANK_TYPES ];

	tick = 0;
	maxband = 0;

	num = get_str_data_num();

	pos = ftell( stream_fp );

	prev = tick;
	prev_pos = pos;

	/* データは１６こまでなので、処理の有無をビットフラグで持つ。*/
	do_flag = 0;
	for( i = 0; i < num; i++ ){
		/* 初期化 */
		int j, outflag;
		str = get_str_data( i );
		set_data_file( &chank[ i ], str );
PRINTF( "CHANK TYPE %X file %s\n", str->type, str->filename );
		outflag = 1;
		for( j = 0; j < i; j++ ){
			if( chank[ j ].type == chank[ i ].type ){
				outflag = 0;
				break;
			}
		}
		if( outflag ){
			output_system_chank( chank[ i ].type, 0 );
		}
		do_flag |= ( 1 << i );
	}

	while( do_flag != 0 ){
		int next_time = tick + TICK_PER_SEC;	// 最低一秒刻み
		for( i = 0; i < num; i++ ){
			if( do_flag & ( 1 << i ) ){
				CHANK_DATA *dp;
				str = get_str_data( i );
				dp = &chank[ i ];
				
				if( !( ( *dp->output_func )( dp, tick ) ) ){
					/* 終了 */
					close_data_file( dp );
					do_flag &= ~( 1 << i );
				} else {
					if( next_time > dp->next_tick ){
						next_time = dp->next_tick;
					}
				}
			}
		}
		if( do_flag != 0 && next_time == 0 ){
			ERROR( "Error !! next = 0 !!\n" );
		}

		tick = next_time;
		if( tick - prev >= TICK_PER_SEC ){
			/* 一秒経過 */
			filepos_t now;
			now = ftell( stream_fp );
			PRINTF( "%dbytes/sec\n", now - prev_pos );
			if( maxband < now - prev_pos ){
				maxband = now - prev_pos;
			}
			prev_pos = now;
			prev = tick;
		}
	}
	output_chank( CHANK_TYPE_END, NULL, 0, tick, 0 );
	trim_sector_alignment( stream_fp );

	/* デバッグ用情報出力 */
	{
		int size, av;
		filepos_t now;
		now = ftell( stream_fp );
		if( now < 0 ){
			fprintf( stderr, "Error:stream file size over 2G bytes!!\n" );
			exit( 1 );
		}
		size = ( now - pos );

		if( tick > 0 ){
			av = ( int )( ( float )size * TICK_PER_SEC / tick / 1024 );
		} else {
			av = 0;
		}
		fprintf( stderr, "TIME:%d(%.2fs) ", tick, ( float )tick / TICK_PER_SEC );
		total_time += ( float )tick / TICK_PER_SEC;
		fprintf( stderr, "SIZE:%d(K) ", size / 1024 );
		fprintf( stderr, "TRANS:av%d:max%d(K/s)\n"
				 , av, ( maxband + 1023 ) / 1024 );
	}
}

/* ---------------------------------------------------------------------- */
/*
   シンボルファイル作成
*/

static void output_symbol( filepos_t pos, char *name )
{
	int tag;

	if( pos % SECTOR_SIZE > 0 ){
		ERROR( "Output size error %d\n", pos % SECTOR_SIZE );
	}
	pos = pos / SECTOR_SIZE;

	tag = pos | tag_flags | ( disc_num_prefix << 24 );

	fprintf( table_fp, "0x%08X %s\n", tag, name );

	if( tag_flags & ( TAGFLAG_SOUND_8BIT | TAGFLAG_SOUND_VAG | TAGFLAG_SOUND_AC3 ) ){
		fprintf( table_fp, "0x%08X %s_len\n", sound_duration, name );
	}
}

/* ---------------------------------------------------------------------- */
/*
	キーワード登録
*/

#define BLOCK_COMMAND	"block"

#define MAX_KEYWORD_STRING_LEN	32

typedef struct _keywords {
	struct _keywords *next;
	int code;
	char keywords[ MAX_KEYWORD_STRING_LEN ];
} KEYWORDS;

static KEYWORDS *keywords = NULL;

static int is_expr_num( char *buf, int *value )
{
	char *p;
	int v;
	int minus_flag;

	v = 0;
	if( buf[ 0 ] == '-' ){
		minus_flag = 1;
		buf ++;
	} else {
		minus_flag = 0;
	}
	if( buf[ 0 ] == '0' ){
		if( ( buf[ 1 ] | 0x20 ) == 'x' ){
			for( p = buf + 2; *p != '\0'; p++ ){
				int c;
				if( *p == '_' ) continue;
				c = *p | 0x20;
				if( c >= '0' && c <= '9' ){
					v = ( v * 16 ) + ( *p - '0' );
				} else if( c >= 'a' && c <= 'f' ){
					v = ( v * 16 ) + ( c - 'a' + 10 );
				} else {
					ERROR( "不正な16進数値です\n" );
				}
			}
			goto END;
		} else if( ( buf[ 1 ] | 0x20 ) == 'b' ){
			for( p = buf + 2; *p != '\0'; p++ ){
				int c;
				c = *p;
				if( c == '_' ) continue;
				if( c == '0' || c == '1' ){
					v = ( v << 1 ) | ( c - '0' );
				} else {
					ERROR( "不正な2進数値です\n" );
				}
			}
			goto END;
		}
	}
	for( p = buf; *p != '\0'; p++ ){
		if( *p >= '0' && *p <= '9' ){
			v = ( v * 10 ) + ( *p - '0' );
		} else {
			return 0;
		}
	}
END:
	if( minus_flag ) v = -v;
	*value = v;
	return 1;
}

static int analyze_define_file( char *deffile )
{
	int max_type = 0;
	keywords = NULL;

	load_file( deffile );

	while( get_line() ){
		WORDBUF buf;

		if( get_word( buf ) == NULL ) continue;
		if( buf[ 0 ] == '#' || buf[ 0 ] == ';'
			|| ( buf[ 0 ] == '/' && buf[ 1 ] == '/' ) ) continue;

		/* キーコードを設定する */
		{
			WORDBUF codebuf;
			int code;
			KEYWORDS *key;

			if( strlen( buf ) >= MAX_KEYWORD_STRING_LEN ){
				ERROR( "keyが長過ぎます。\n" );
			}
			if( get_word( codebuf ) == NULL ){
				ERROR( "コードが設定されていません。\n" );
			}
			if( !is_expr_num( codebuf, &code ) ){
				ERROR( "%s:数字が必要です\n", codebuf );
			}
PRINTF( "KEY %s CODE %x\n", buf, code );
			key = ( KEYWORDS * )Malloc( sizeof( KEYWORDS ) );
			if( code == CHANK_TYPE_FREE
				|| code == CHANK_TYPE_SYSTEM
				|| code == CHANK_TYPE_END
				|| code == CHANK_TYPE_BUFEND ){
				ERROR( "code %X はシステムで予約されています。\n", code );
			}

			strcpy( key->keywords, buf );
			key->code = code;
			key->next = keywords;
			keywords = key;
		}
	}
}

static int search_key( char *key )
{
	KEYWORDS *kp;

	for( kp = keywords; kp != NULL; kp = kp->next ){
		if( strcmp( kp->keywords, key ) == 0 ){
			return kp->code;
		}
	}
	return -1;
}

/* ---------------------------------------------------------------------- */
/*
	スクリプトファイル解析
*/

static char data_name[ 128 ];

static int analyze_scr( char *scrfile )
{
	int scr_num = 0;

	parse_init();
	if( ! load_file( scrfile ) ){
		ERROR( "Can't open %s\n", scrfile );
	}

	while( get_line() ){
		WORDBUF buf;
		int block_flag;
		int stream_flag;
		filepos_t pos;

		if( get_word( buf ) == NULL ) continue;
		if( buf[ 0 ] == '#' || buf[ 0 ] == ';' ) continue;
		if( strncmp( buf, "//", 2 ) == 0 ) continue;

		/* ブロックの設定 */

		stream_flag = 0;
		block_flag = 0;
		tag_flags = 0;

		if( no_output_mode == 0 ){
			pos = ftell( stream_fp );
			PRINTF( "POS = %X\n", pos );
		}

		if( strcmp( buf, BLOCK_COMMAND ) == 0 ){
			get_word( data_name );
			if( no_output_mode == 0 ){
				fprintf( stderr, "STREAM %s: ", data_name );
			}

			init_str_data();

			while( get_line() ){
				int type;
				int tick_offset = 0;
				WORDBUF name;

				if( get_word( buf ) == NULL ) continue;
				if( buf[ 0 ] == '#' || buf[ 0 ] == ';' ) continue;
				if( strcmp( buf, "//" ) == 0 ) continue;

				if( block_flag == 0 ){
					if( strcmp( buf, "{" ) != 0 ){
						ERROR( "ブロックが始まっていません。 %s\n", buf );
					} else {
						block_flag = 1;
						continue;
					}
				}
				if( block_flag == 1 ){
					if( strcmp( buf, "}" ) == 0 ){
						break;
					}
				}
				type = 0;
				if( ( type = search_key( buf ) ) > 0 ){
					if( get_word( name ) == NULL ){
						ERROR( "no stream data file name\n" );
					}
				} else {
					WARNING( "Wrong Key %s\n", buf );
				}

				tick_offset = 0;
				if( get_word( buf ) != NULL ){
					if( ! is_expr_num( buf, &tick_offset ) ){
						ERROR( "need start offset value\n" );
					}
				}
				if( type > 0 ){
					set_str_data( type, name, tick_offset );
					stream_flag = 1;
				}
			}
		} else {
			ERROR( "blockがはじまっていません\n" );
		}

		/* 設定完了 */

		if( stream_flag != 0 ){
			if( no_output_mode == 0 ){
				make_stream_data();
				output_symbol( pos, data_name );
			}
			scr_num ++;
		} else {
			WARNING( "NO STREAM DATA\n" );
		}
	}
	parse_end();
	{
		int i;
		for( i = 0; i < 16; i++ ){
			if( type_total[ i ] > 0 ){
				printf( "TYPE %02X TOTAL %d bytes\n", i, type_total[ i ] );
			}
		}
		printf( "PADDING %d\n", padding_size );
	}

	return scr_num;
}

/* ---------------------------------------------------------------------- */
/*
	外部関数
*/

void make_stream( char *scrfile )
{
	int num;

	total_time = 0.0F;
	num = analyze_scr( scrfile );
	fprintf( stderr, "%d streams created.\n", num );
	fprintf( stderr, "total duration is %.2fs\n", total_time );
}

void open_stream_file( char *filename )
{
	if( no_output_mode ) return;
	if( ( stream_fp = fopen( filename, "wb" ) ) == NULL ){
		printf( "Can't open %s\n", filename );
		exit( 1 );
	}
}

void open_symbol_file( char *filename )
{
	if( no_output_mode ) return;
	if( ( table_fp = fopen( filename, "wt" ) ) == NULL ){
		printf( "Can't open %s\n", filename );
		exit( 1 );
	}
}

void close_stream_file( void )
{
	if( no_output_mode ) return;
	fclose( stream_fp );
}

void close_symbol_file( void )
{
	if( no_output_mode ) return;
	fclose( table_fp );
}

void set_stream_define_file( char *filename )
{
	analyze_define_file( filename );
}
