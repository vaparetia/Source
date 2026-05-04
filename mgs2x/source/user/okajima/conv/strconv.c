//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   ファイル中に GV_StrCode( "文字列" )があったら、それを数字に変換する
   プリプロセッサ
*/

#include <stdio.h>
#include <stdlib.h>

/* --------------------------- 文字コード変換部 ---------------------------- */

#define STRCODE		"GV_StrCode"		/* 変換用ヒット文字列 */

/* strcode 変換ルーチン。エンディアンには影響されない */
/* かける文字列は必ずEUCであること */

#define BIT_LEN         24

static long get_strcode( char *string )
{
        unsigned char c;
        unsigned char *p;
        unsigned int id, mask;

        p = ( unsigned char * )string;
        id = 0;
        mask = ( 1 << BIT_LEN ) - 1;

        while( ( c = *( p++ ) ) != '\0' ){
                id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
                id += c;
                id &= mask;
        }
        if( id == 0 ) id = 1;

        return id;
}

static long change_strcode( char *top, FILE *out )
{
	char org[ 256 ];
	unsigned char *p, *d;
	long inflag;
	long	count;

	p = ( unsigned char * )top;
	d = ( unsigned char * )org;
	inflag = 0;

	count=0;
	while( *p != ')' ){
		if( *p < 0x20 ){
/*			fprintf( stderr, "Wrong data\n" );*/
			fputc( *top, out );
			return 1;
		} else if( *p == '\"' ){
			if( inflag ){
				*d = '\0';
				inflag = 0;
			} else {
				inflag = 1;
			}
		} else {
			if( inflag ){
				*( d ++ ) = *p;
			}
		}
		count++;
		p++;
	}
//	fprintf( out, "%d /* GV_StrCode( \"%s\" ) */", get_strcode( org ), org );
	fprintf( out, "%d /*\"%s\"*/", get_strcode( org ), org );

	count++;
	p++;

	return count;
}

static long check_strcode( char *p )
{
	char *s;

	s = STRCODE;
	for( ; *s != '\0'; s++, p++ ){
		if( *s != *p ){
			return 0;
		}
	}
	return 1;
}


static char *_NextStr;

static int StrToInt( char *str )
{
	char c;
	long value;

	value = 0;
	c = *str;

	if( ( c >= '0' && c <= '9' ) || c == '-' ){
		int minus, digit;

		/* atoi */
		minus = 0;
		if( c == '-' ){
			minus = 1; str++;
		}
		for( ;; ){
			digit = *( str ++ ) - '0';
			if( digit < 0 || digit > 9 ) break;
			value = value * 10 + digit;
		}
		if( minus ){
			value = - value;
		}
		_NextStr = str - 1;
	}
	return value;
}

static int StrToSV( char *str, short *sv )
{
	int dim;

	dim = 0;
	do {
		*sv ++ = StrToInt( str );
		str = _NextStr + 1;
		dim++;
	} while( str[ -1 ] == ',' );
	return dim;
}

static char *SkipSpace( char *str )
{
	char c;

	do {
		if( ( c = *( str ++ ) ) == '\0' ) break;
	} while( c == ' ' || c == '\t' || c == '\n' || c == '\r' );
	return str - 1;
}

/*
	空白文字列チェック
*/
static	int	LsSpace( char c )
{
	return ( c == ' ' || c == '\t' || c == '\n' || c == '\r' );
}

/*
	次の空白後の文字までスキップする
*/
static char *NextSpNextStr( scr )
char *scr ;
{
	char	c ;

	do {
		c = *(scr++) ;
	} while ( !LsSpace( c ) ) ;
	do {
		c = *(scr++) ;
	} while ( LsSpace( c ) ) ;
	
	return scr - 1 ;
}



/* ------------------ アニメーションフォーマット変換部終わり --------------- */

static void strconv( char *buf, long size, FILE *out )
{
	char *p;

	for( p = buf; size > 0; size --, p++ ){
		if( check_strcode( p ) ){
			/* 変換の必要あり */
			long skip;
			skip = change_strcode( p, out ) - 1;
			size -= skip;
			p += skip;
		} else {
			/* 変換の必要なし */
			fputc( *p, out );
		}
	}
}

/* ファイルをメモリに丸ごと読み込む */

static char *load_file( char *filename, long *filesize )
{
	FILE *fp;
	char *buf;
	long size;

	if( ( fp = fopen( filename, "rb" ) ) == NULL ){
		fprintf( stderr, "FILE %s not Found\n", filename );
		return NULL;
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	if( ( buf = malloc( size ) ) == NULL ){
		fprintf( stderr, "malloc failed !!\n" );
		return NULL;
	}

	fread( buf, sizeof( char ), size, fp );
	fclose( fp );

	*filesize = size;

	return buf;
}

int main( int argc, char *argv[] )
{
	FILE *out;
	char *buf;
	long size;

	if( argc < 2 ){
		fprintf( stderr, "Usage strcode <infile> [outfile]\n" );
		return 1;
	}

	if( ( buf = load_file( argv[ 1 ], &size ) ) == NULL ){
		return 1;
	}

	if( argc < 3 ){
		out = stdout;
	} else {
		if( ( out = fopen( argv[ 2 ], "wb" ) ) == NULL ){
			fprintf( stderr, "Can't open %s\n", argv[ 2 ] );
			return 1;
		}
	}
	strconv( buf, size, out );

	fclose( out );

	free( buf );

	return 0;
}
