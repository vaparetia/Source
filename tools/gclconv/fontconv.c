/*
   フォント関連ルーチン

   zen_table に定義された文字はコード変換。

   定義されていない文字は、登場順にナンバリングしたコードに変換した後、
   その文字データ本体を出力
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mymalloc.h"
#include "gclconv.h"
#include "fontconv.h"

#include "font.h"

#define CONV_EUC_CODE 1

#define MAX_FONT_NUM	2048

#define GCL_FONT_BASE_END		0x9000
#define GCL_FONT_EXTEND_TOP		0x9400

static int noresident_code;

#define NORESIDENT_CODE		noresident_code

#define KEEP	1
#define REGIST	2

int font_area_check_flag = 0;
FONT_AREA_CHECK font_area_check;

long font_conv_flag = 0;
long font_changed_flag = 0;

static long now_max_code;

typedef struct {
	unsigned short code;
	unsigned short flag;
} FONTINFO;

static FONTINFO *conv_table;
static unsigned short *regist_table;
long regist_table_p;

int font_no_euc_conv = 0;
int font_no_expand_flag = 0;

static char load_path[ 256 ];

/* ヨーロッパフォント変換テーブル */

typedef struct {
	char *key;
	char code;
} CODE_TABLE;

static CODE_TABLE codetable[] = {
	{ "`A",0xcb },
	{ "'A",0xe7 },
	{ "^A",0xe5 },
	{ "~A",0x80 },
	{ "~Q",0x82 },
	{ "`E",0xe9 },
	{ "'E",0x83 },
	{ "^E",0xe6 },
	{ "~E",0xe8 },
	{ "`I",0xed },
	{ "'I",0xea },
	{ "^I",0xeb },
	{ "~I",0xec },
	{ "`O",0xf1 },
	{ "'O",0xee },
	{ "^O",0xef },
	{ "~O",0x85 },
	{ "`U",0xf4 },
	{ "'U",0xf2 },
	{ "^U",0xf3 },
	{ "~U",0x86 },
	{ "~s",0xa7 },
	{ "`a",0x88 },
	{ "'a",0x87 },
	{ "^a",0x89 },
	{ "~a",0x8a },
	{ "~q",0x8d },
	{ "`e",0x8f },
	{ "'e",0x8e },
	{ "^e",0x90 },
	{ "~e",0x91 },
	{ "`i",0x93 },
	{ "'i",0x92 },
	{ "^i",0x94 },
	{ "~i",0x95 },
	{ "`o",0x98 },
	{ "'o",0x97 },
	{ "^o",0x99 },
	{ "~o",0x9a },
	{ "`u",0x9d },
	{ "'u",0x9c },
	{ "^u",0x9e },
	{ "~u",0x9f },
	{ "~y",0xd8 },
	{ "@E",0xce },
	{ "@e",0xcf },
	{ "@R",0xa8 },
	{ "~<",0xc7 },
	{ "~>",0xc8 },
	{ "~.",0xc9 },
	{ "~0",0xa1 },
	{ "~!",0xc1 },
	{ "~?",0xc0 },
	{ "~n",0x96 },
	{ "~N",0x84 },
	{ "&&",'&' },
	{ "&'",'\'' },
	{ NULL, 0 }
};

static int check_code( char *p )
{
	CODE_TABLE *tp;

	for( tp = codetable; tp->key != NULL; tp++ ){
		if( strncmp( tp->key, p, 2 ) == 0 ){
			return tp->code;
		}
	}
	return 0;
}

/* ---------------------------------------------------------------------- */
/*
	コード変換メイン
*/

void font_reset_font_table( void )
{
	long i;

	regist_table_p = 0;
	now_max_code = NORESIDENT_CODE;

	for( i = 0; i < 0x8000; i++ ){
		if( conv_table[ i ].flag == REGIST ){
			conv_table[ i ].code = 0;
			conv_table[ i ].flag = 0;
		}
	}
}

int font_change_code( long code )
{
	if( ! font_conv_flag ) return code;
	
	if( conv_table[ code - 0x8000 ].code > 0 ){
		return conv_table[ code - 0x8000 ].code;
	} else {
		if( font_no_expand_flag ){
			int c1, c2;
			c1 = code >> 8;
			c2 = code & 0xFF;
			ERROR( "%c%c: %02X%02X(EUC)はzen_tableに登録されていません\n"
				  , c1, c2, c1, c2 );
		}
		now_max_code = now_max_code + 1;
		if( now_max_code == GCL_FONT_BASE_END ){
			now_max_code = GCL_FONT_EXTEND_TOP;
		}
		if( now_max_code % 256 == 0 ){
			now_max_code ++;
		}

		conv_table[ code - 0x8000 ].code = now_max_code;
		conv_table[ code - 0x8000 ].flag = REGIST;
		font_changed_flag = 1;
		regist_table[ regist_table_p ] = code;
		regist_table_p ++;
#if 0
		if( now_max_code >= NORESIDENT_CODE + MAX_FONT_NUM ){
#if 0
			int i;
			FILE *lfp;

			lfp = fopen( "_log_.txt", "wt" );
			
			for( i = 0; i < 0x7FFF; i++ ){
				if( conv_table[ i ].code > 0 ){
					int c;
					c = i + 0x8000;
					fprintf( lfp, "%X : %c%c\n", c, ( c >> 8 ), ( c & 0xFF ) );
				}
			}

			fclose( lfp );
#endif
			ERROR( "フォント数が%dを越えました。%X\n", MAX_FONT_NUM, code );
		}
#endif
		return now_max_code;
	}
}

static long is_kanji( long c )
{
	if( font_no_euc_conv == 0 ){
		// SJISの1バイト目かどうかを返す。
		if( ( c >= 0x81 && c <= 0x9f ) | ( c >= 0xe0 && c <= 0xfc ) ){
			return 1;
		} else {
			return 0;
		}
	} else {
		// EUCの1バイト目かどうかを返す
		if( c >= 0x80 ){
			return 1;
		} else {
			return 0;
		}
	}
}

#ifdef CONV_EUC_CODE

static void euc_to_sjis( char *buf )
{
	unsigned char *p;
	p = ( unsigned char * )buf;
	while( *p != '\0' ){
		if( *p & 0x80 ){
			int high, low;

			if( *p == 0x8E ){
				// 半角カタカナ
				char errbuf[ 128 ];
				errbuf[ 127 ] = '\0';
				strncpy( errbuf, p, 127 );
				WARNING( "半角カタカナは使用できません %s\n", errbuf );
			}
			high = p[ 0 ] - 0x80;
			low = p[ 1 ] - 0x80;
			if( high & 1 ){
				low += 0x1f;
				if( low >= 0x7f ) low++;
			} else {
				low += 0x7e;
			}
			high = ( high - ' ' - 1 ) / 2 + 0x81;
			if( high >= 0xa0 ) high += '@';
			p[ 0 ] = high;
			p[ 1 ] = low;
			p ++;
		}
		p++;
	}
}

#endif

int font_conv_buffer( char *dest, char *src )
{
	unsigned char *p;
	unsigned char *d;
	char *_src;
	int result = 0;
	int zen_count = 0;
	int total = 0;

	if( ! font_conv_flag ) return 0;

	{
		int size;
		size = strlen( src );
//		_src = malloc( size + 1 );
		_src = (char*)_alloca( size + 1 );
		strcpy( _src, src );
	}
		
#ifdef CONV_EUC_CODE
	if( font_no_euc_conv == 0 ){
		euc_to_sjis( _src );
	}
#endif

	d = dest;
	for( p = ( unsigned char * )_src; *p != '\0'; ){
		if( is_kanji( *p ) ){
			long code;
			code = ( *p << 8 ) | *( p + 1 );
			code = font_change_code( code );
			*d = code >> 8;
			*( d + 1 ) = code & 0xff;
			d += 2;
			p += 2;
			zen_count ++;
		} else if( *p == '&' ){
			int code;
			code = check_code( p + 1 );
			if( code == 0 ){
				*( d ++ ) = *( p ++ );
			} else {
				*( d ++ ) = 0x1f;
				*( d ++ ) = code - 0x80 + 1;
				p += 3;
			}
		} else {
			*( d ++ ) = *( p ++ );
		}
		total ++;
	}
	*d = *p;
//	free( _src );
#if 0
	if( font_area_check_flag ){
		int res;
		if( ( res = font_draw_area_check( dest
								  , font_area_check.width
								  , font_area_check.height
								  , font_area_check.c_skip
								  , font_area_check.l_skip
								  , font_area_check.kinsoku ? 0 : FONT_NO_KINSOKU ) )
			& ( FONT_RESULT_OVER | FONT_RESULT_ORIKAESHI ) ){

			if( res & FONT_RESULT_OVER ){
				result |= FONTCONV_RESULT_OVER;
			}
			if( res & FONT_RESULT_ORIKAESHI ){
				result |= FONTCONV_RESULT_ORIKAESHI;
			}
		}
	}
#endif
	if( total / 4 >= zen_count ){
		result |= FONTCONV_NO_ZENKAKU;
	}
	return result;
}

int fontconv_area_check( char *string, FONT_AREA_CHECK *area )
{
	int result = 0;
	if( font_area_check_flag ){
		int res;
		FONT_AREA_CHECK *fap;
		if( area == NULL ){
			fap = &font_area_check;
		} else {
			fap = area;
		}
		if( ( res = font_draw_area_check( string
								  , fap->width
								  , fap->height
								  , fap->c_skip
								  , fap->l_skip
								  , fap->kinsoku ? 0 : FONT_NO_KINSOKU ) )
			& ( FONT_RESULT_OVER | FONT_RESULT_ORIKAESHI ) ){

			if( res & FONT_RESULT_OVER ){
				result |= FONTCONV_RESULT_OVER;
			}
			if( res & FONT_RESULT_ORIKAESHI ){
				result |= FONTCONV_RESULT_ORIKAESHI;
			}
		}
	}
	return result;
}

/* ---------------------------------------------------------------------- */
/*
	出力系
*/

void font_set_load_path( char *path )
{
	strcpy( load_path, path );
}

static FILE *font_file_open( char *filename, char *flag )
{
	FILE *fp;
	char fname[ 256 ];

	sprintf( fname, "%s/%s", load_path, filename );
	if( ( fp = fopen( fname, flag ) ) != NULL ){
		return fp;
	}

	if( ( fp = fopen( filename, flag ) ) != NULL ){
		return fp;
	}
	return NULL;
}

void font_load_table( char *filename )
{
	FILE *fp;
	long max = 0;

	conv_table = malloc( sizeof( FONTINFO ) * 0x8000 );
	memset( conv_table, 0, sizeof( FONTINFO ) * 0x8000 );

	regist_table = malloc( sizeof( short ) * MAX_FONT_NUM );
	memset( regist_table, 0, sizeof( short ) * MAX_FONT_NUM );

	if( ( fp = font_file_open( filename, "rt" ) ) == NULL ){
		printf( "load_table : %s open Error\n", filename );
		exit( 1 );
	}
	while( ! feof( fp ) ){
		char buffer[ 256 ];
		int org, new;

		fgets( buffer, 256, fp );
		if( sscanf( buffer, "%X %X", &org, &new ) == 2 ){
			conv_table[ org - 0x8000 ].code = ( unsigned short )new;
			conv_table[ org - 0x8000 ].flag = KEEP;
			new = new & ( ~0x6000 );
			if( new > max ) max = new;
		}
	}
	fclose( fp );

/*	printf( "load_table : max %X\n", max );*/

	now_max_code = NORESIDENT_CODE;
	regist_table_p = 0;

	font_changed_flag = 0;
	font_conv_flag = 1;
}

//#define ALLFONT_FILE_NAME "fontall.dat"

static char all_font_file_name[ 128 ];
static int font_one_font_size = 0;

void font_init_font_file( int font_code_top, char *all_font_file )
{
	FILE *in;
	unsigned char buf[ 4 ];

	if( font_no_expand_flag ){
		return;
	}

	noresident_code = font_code_top;

	if( font_area_check_flag ){
		// area check用に常駐フォントファイルを読み込む。
		// zen_tableと対になっている必要あり。
		FILE *fp;
		int size;
		void *buffer;
		if( ( fp = fopen( font_area_check.fontfile, "rb" ) ) == NULL ){
			ERROR( "フォントファイル %sがありません\n", font_area_check.fontfile );
		}
		fseek( fp, 0, SEEK_END );
		size = ftell( fp );
		fseek( fp, 0, SEEK_SET );

		buffer = malloc( size );
		fread( buffer, sizeof( char ), size, fp );

		fclose( fp );
		font_resident_load_set( buffer );
	}

	if( ( in = font_file_open( all_font_file, "rb" ) ) == NULL ){
		printf( "Can't open FONT FILE %s\n", all_font_file );
		return;
	}

	fread( buf, sizeof( int ), 1, in );
	font_one_font_size = ( buf[ 0 ] << 24 ) | ( buf[ 1 ] << 16 )
				| ( buf[ 2 ] << 8 ) | buf[ 3 ];

	fclose( in );

	strcpy( all_font_file_name, all_font_file );
}

int font_get_data_size( void )
{
	/* 登録されたフォントデータの大きさを返す */
	return font_one_font_size * regist_table_p + sizeof( int );
}

void font_set_font_data( void *buffer )
{
	// バッファ出力バージョン
	FILE *in;
	int one_font_size;
	int i;
	char *buf;

	if( regist_table_p == 0 ){
      *( int * )buffer = 0;   //BP - fill in otherwise garbage data for font data size with 0 if there are none.
		return;
	}
#if 0
	if( now_max_code >= NORESIDENT_CODE + MAX_FONT_NUM ){
#if 0
		int i;
		FILE *lfp;

		lfp = fopen( "_log_.txt", "wt" );
			
		for( i = 0; i < 0x7FFF; i++ ){
			if( conv_table[ i ].code > 0 ){
				int c;
				c = i + 0x8000;
				fprintf( lfp, "%X : %c%c\n", c, ( c >> 8 ), ( c & 0xFF ) );
			}
		}

		fclose( lfp );
#endif
		ERROR( "フォント数が%dを越えて%d個あります。\n", MAX_FONT_NUM, now_max_code - NORESIDENT_CODE );
	}
#endif
	// フォントのサイズはファイルの先頭に記載

	if( ( in = font_file_open( all_font_file_name, "rb" ) ) == NULL ){
		printf( "Can't open FONT FILE %s\n", all_font_file_name );
		return;
	}

	{
		unsigned char buf[ 4 ];
		fread( buf, sizeof( int ), 1, in );
		one_font_size = ( buf[ 0 ] << 24 ) | ( buf[ 1 ] << 16 )
			| ( buf[ 2 ] << 8 ) | buf[ 3 ];
	}

	{
		int font_size;
		font_size = regist_table_p * one_font_size;

		*( int * )buffer = font_size;
		buffer = (char*)buffer + sizeof( int );
	}

	buf = buffer;

	for( i = 0; i < regist_table_p; i++ ){
		long code;

		code = regist_table[ i ];
		fseek( in, ( code - 0x8000 ) * one_font_size + sizeof( int )
			   , SEEK_SET );
		fread( buf, sizeof( char ), one_font_size, in );
		buf += one_font_size * sizeof( char );
	}
	fclose( in );
}

void font_output_font_data( FILE *fp )
{
	// ファイル出力バージョン

	int size;
	void *buffer;

	if( font_conv_flag == 0 ){
		return;
	}

	size = font_get_data_size();
	if( size == 0 ){
		return;
	}

	buffer = malloc( size );
	font_set_font_data( buffer );

	if( fwrite( buffer, sizeof( char ), size, fp ) < size ){
		printf( "fatal: write error\n" );
		exit( 1 );
	}

	free( buffer );
}

/* ---------------------------------------------------------------------- */
/*
	for test
*/

#ifdef TEST

static	char buffer[ 4096 ];

int main( int argc, char *argv[] )
{
	FILE *fp, *ofp;
	long i;

	load_table( "zen_table" );

	for( i = 1; i < argc; i++ ){
		if( ( fp = fopen( argv[ i ], "rt" ) ) == NULL ){
			printf( "Can't open %s as input\n", argv[ 1 ] );
			return;
		}
		while( ! feof( fp ) ){
			unsigned char c1, c2;
			c1 = fgetc( fp );
			if( is_kanji( c1 ) ){
				c2 = fgetc( fp );
				change_code( ( c1 << 8 ) | c2 );
			}
		}
		fclose( fp );
	}

	output_table( "zen_table" );
}

#endif
