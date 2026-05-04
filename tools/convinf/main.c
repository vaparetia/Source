/*
	convinfメインルーチン
	$Id: main.c,v 1.3 2002/04/05 07:29:44 usr01475 Exp $
*/

#include <stdio.h>
#include <string.h>

#include "infdata.h"
#include "fontconv.h"
#include "mymalloc.h"

extern void make_caption_data( char *filename );		// in caption.c

int verbose_mode = 0;

static char *program_name = "convinf";

static char font_pathname[ 256 ] = ".";
static char zentable_filename[ 256 ] = "zen_table";
static char fontall_filename[ 256 ] = "fontall.dat";
static char input_filename[ 256 ] = "";
static char output_filename[ 256 ] = "";

static void usage( void )
{
	printf( "%s [options] <inffile>\n", program_name );
	printf( " options : \n" );
	printf( "   -o <output filename>\n" );
	printf( "   -f <font filename>\n" );
	printf( "   -t <zen_table filename>\n" );
	printf( "   -a <fontall filename>\n" );
	printf( "   -k       : korea EUC support\n" );
	printf( "   -v       : verbose mode\n" );

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
	int file_set_flag = 0;

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			/* option */
			switch( argv[ i ][ 1 ] ){
			  case 'o':
				i += get_arg_filename( output_filename, argv + i );
				break;
			  case 'f':
				i += get_arg_filename( font_pathname, argv + i );
				break;
			  case 't':
				i += get_arg_filename( zentable_filename, argv + i );
				break;
			  case 'a':
				i += get_arg_filename( fontall_filename, argv + i );
				break;
			  case 'k':
				{
					extern int font_no_euc_conv;
					extern int inf_conv_euc_flag;
					font_no_euc_conv = 1;
					inf_conv_euc_flag = 1;
				}
				break;
			  case 'v':
				{
					extern int verbose_mode;
					verbose_mode = 1;
				}
				break;
			  default:
				usage();
				break;
			}
		} else {
			if( file_set_flag != 0 ){
				printf( "ファイルは1つだけ指定して下さい\n" );
				exit( 1 );
			}
			strcpy( input_filename, argv[ i ] );
			file_set_flag = 1;
		}
	}
	if( output_filename[ 0 ] == '\0' ){
		/* ファイル名作成 */
		strcpy( output_filename, input_filename );
		strcat( output_filename, ".cap" );
	}

	return file_set_flag;
}

int main( int argc, char *argv[] )
{
	FILE *fp;

	if( ! analyze_option( argc, argv ) ){
		/* error */
		usage();
		return 1;
	}

	init_Malloc();

	// inf fileのロード
	if( ( fp = fopen( input_filename, "rb" ) ) == NULL ){
		printf( "file %s not found\n", input_filename );
		exit( 1 );
	}
	load_inf_file( fp );
	fclose( fp );

	// font fileのロード
	font_set_load_path( font_pathname );
	font_load_table( zentable_filename );
	font_init_font_file( fontall_filename );

	// 解析とoutput
	make_caption_data( output_filename );

	return 0;
}
