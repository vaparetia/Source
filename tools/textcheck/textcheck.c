/*
	textcheck.c
		テキストファイルの漢字コードと改行コードを簡単にチェックする
	1999/11/24 K.Uehara
	$Id: textcheck.c,v 1.8 2000/12/11 11:00:25 usr01475 Exp $
*/

#include <stdio.h>
#include <stdarg.h>

static int verbose_mode = 0;
static int quiet_mode = 0;

static char *prog_name = "textcheck";

/* ------------------------------------------------------- */

void PRINTF( char *format, ... )
{
	va_list ap;
	va_start( ap, format );
	if( verbose_mode ){
		vprintf( format, ap );
	}
	va_end( ap );
}

void ERROR( char *format, ... )
{
	va_list ap;

	va_start( ap, format );
	if( ! quiet_mode ){
		printf( "%s: Error: ", prog_name );
		vprintf( format, ap );
	}
	va_end( ap );
}

void WARNING( char *format, ... )
{
	va_list ap;

	va_start( ap, format );
	if( ! quiet_mode ){
		printf( "%s: Warning: ", prog_name );
		vprintf( format, ap );
	}
	va_end( ap );
}

void FATAL( char *format, ... )
{
	va_list ap;

	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );

	exit( 1 );
}

/* ------------------------------------------------------- */

static void usage( void )
{
	printf( "Usage: textcheck [options] filename ...\n" );
	printf( " text CR-LF, SJIS check\n" );
	printf( "   -v        verbose mode\n" );
	printf( "   -q        quiet mode\n" );
	exit( -1 );
}

static void option_check( int argc, char *argv[] )
{
	int i;

	if( argc < 2 ){
		usage();
	}

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			switch( argv[ i ][ 1 ] ){
			  case 'v':
				verbose_mode = 1;
				break;
			  case 'q':
				quiet_mode = 1;
				break;
			  default:
				usage();
			}
		}
	}
}

/* ------------------------------------------------------- */

static int analyze_file( char *filename )
{
	FILE *fp;

	/* EUC/UNIX以外の場合はエラーとする */

	if( ( fp = fopen( filename, "rb" ) ) == NULL ){
		PRINTF( "%s: no such file / removed.\n", filename );
		// ファイルがない場合は removeされたとみなす。
		return 0;
	}
	PRINTF( "CHECK %s\n", filename );
	while( !feof( fp ) && !ferror( fp ) ){
		int c;
		c = fgetc( fp );
		if( c == 0x0d ){
			int c2;
			c2 = fgetc( fp );
			if( c2 == 0x0a ){
				ERROR( "%s: This file is DOS CR-LF file.\n", filename );
				return 1;
			} else {
				ERROR( "%s: This file is MAC CR file.\n", filename );
				return 1;
			}
		}

		if( ( c >= 0x81 && c <= 0x9f ) || ( c >= 0xe0 && c <= 0xfc ) ){
			/* SJIS 漢字コードの1文字目 */
			int c2;
			c2 = fgetc( fp );
			if( c2 >= 0x40 && c2 <= 0x7e ){
				/* SJIS 確定 */
				ERROR( "%s: This file is Shift JIS file.\n", filename );
				return 1;
			}
		} else if( c >= 0x80 ){
			/* EUC 漢字コードの1文字目 */
			int c2;
			c2 = fgetc( fp );
			if( c2 < 0x80 ){
				ERROR( "%s: This file is unknown kanji format file.\n", filename );
				return 1;
			}
		} else if( c == 0x1b ){
			ERROR( "%s: This file is JIS format file.\n", filename );
			return 1;
		}
	}

	fclose( fp );

	return 0;
}


int main( int argc, char *argv[] )
{
	int i;
	int num = 0;
	option_check( argc, argv );

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ) continue;

		num += analyze_file( argv[ i ] );
	}

	return num;
}
