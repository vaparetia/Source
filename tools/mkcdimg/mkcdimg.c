/*
	mkcdimg
		CDイメージ作成ツール

	2000/04/19 K.Uehara
	$Id: mkcdimg.c,v 1.3 2001/02/14 01:57:39 usr01475 Exp $

	stage/ にあるディレクトリ中のdata.cnfを元にして、
	すべてのディレクトリとその中のファイルを1ファイルにまとめる。
*/

#include <stdio.h>
#include <string.h>

extern int build_stage_file( char *dest_path, char *src_path, char *(*getnext)( char *buf ), char *tsymbol_file );

#include "parse.h"
#include "print.h"

char *program_name = "mkcdimg";

static char dest_file[ 256 ] = "";
static char src_path[ 256 ] = "";
static char list_file[ 256 ] = "";
static char tsymbol_file[ 256 ] = "";

static void usage( void )
{
	printf( "usage: %s -o <output> -s <srcpath>\n", program_name );
	printf( "option : -l <dirlistfile>\n" );
	printf( "         -t <tsymbolfile>\n" );
	printf( "         -z compression data ( use zlib )\n" );
	printf( "         -v verbose\n" );

	exit( 1 );
}

static int get_arg_filename( char *filename, char **argv )
{
	if( argv[ 0 ][ 2 ] != '\0' ){
		strcpy( filename, &( argv[ 0 ][ 2 ] ) );
		return 0;
	} else {
		strcpy( filename, argv[ 0 + 1 ] );
		argv[ 0 ][ 0 ] = '\0';
		return 1;
	}
}

int analyze_option( int argc, char *argv[] )
{
	int i;
	int flag = 0;

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			/* option */
			switch( argv[ i ][ 1 ] ){
			  case 'h':
				usage();
				break;
			  case 'v':
				verbose_mode = 1;
				break;
			  case 'o':
				i += get_arg_filename( dest_file, argv + i );
				flag |= 1;
				break;
			  case 's':
				i += get_arg_filename( src_path, argv + i );
				flag |= 2;
				break;
			  case 'l':
				i += get_arg_filename( list_file, argv + i );
				flag |= 4;
				break;
			  case 't':
				i += get_arg_filename( tsymbol_file, argv + i );
				flag |= 8;
				break;
			  case 'z':
				{
					extern int compress_mode;
					compress_mode = 1;
				}
				break;
			}
		} else {
			usage();
		}
	}
	return ( ( flag & 3 ) != 0 ) ? flag : 0;
}

static char *get_next( char *word )
{
LOOP:
	if( get_word( word ) == NULL ){
		if( ! get_line() ){
			return NULL;
		}
		goto LOOP;
	}
	return word;
}

int analyze_list_file( char *filename )
{
	parse_init();

	if( !load_file( filename ) ){
		ERROR( "Can't open list file %s.\n", filename );
	}

	return build_stage_file( dest_file, src_path, get_next, tsymbol_file );
}

int main( int argc, char *argv[] )
{
	if( analyze_option( argc, argv ) == 0 ){
		usage();
	}

	if( list_file[ 0 ] == '\0' ){
		// 作成
		build_stage_file( dest_file, src_path, NULL, tsymbol_file );
	} else {
		analyze_list_file( list_file );
	}

	return 0;
}
