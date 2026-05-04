/*
   ストリームデータ作成ルーチン
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "stream.h"

char *program_name = "stream";

void usage( void )
{
	printf( "stream [ scrfile ... ][ -d define file ][ -o outputfile ][ -t tablefile ]\n" );
	printf( "          -c[no] : disc no\n" );
	printf( "          -v : verbose mode\n" );
}

static char output_filename[ 256 ] = "stream.dat";
static char table_filename[ 256 ] = "stream.lst";
static char define_filename[ 256 ] = "streamid.def";

static int get_arg_filename( char *filename, char **argv, int lim )
{
	if( argv[ 0 ][ 2 ] != '\0' ){
		strcpy( filename, &( argv[ 0 ][ 2 ] ) );
		return 0;
	} else if( lim > 1 ){
		strcpy( filename, argv[ 0 + 1 ] );
		argv[ 0 ][ 0 ] = '\0';
		return 1;
	}
	printf( "ERROR: no arg\n" );
	exit( 1 );
}

int analize_option( int argc, char *argv[] )
{
	int i;
	int file_flag;

	file_flag = FALSE;

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			switch( argv[ i ][ 1 ] ){
			  default:
			  case 'h':
				usage();
				break;
			  case 'o':
				i += get_arg_filename( output_filename, argv + i, argc - i );
				break;
			  case 't':
				i += get_arg_filename( table_filename, argv + i, argc - i );
				break;
			  case 'd':
				i += get_arg_filename( define_filename, argv + i, argc - i );
				break;
			  case 'v':
				{
					extern int verbose_mode;
					verbose_mode = 1;
				}
				break;
			  case 'n':
				{
					extern int no_output_mode;
					no_output_mode = 1;
				}
				break;
			  case 'c':
				{
					extern int disc_num_prefix;
					char buf[ 256 ];
					i += get_arg_filename( buf, argv + i, argc - i );
					disc_num_prefix = atoi( buf );
				}
				break;
			}
			argv[ i ][ 0 ] = '\0';
		} else {
			file_flag = TRUE;
		}
	}
	return file_flag;
}

extern void make_stream( char *scrfile );

int main( int argc, char *argv[] )
{
	int i;

	if( ! analize_option( argc, argv ) ){
		usage();
		return 1;
	}

	parse_init();

	set_stream_define_file( define_filename );

	open_stream_file( output_filename );
	open_symbol_file( table_filename );

	/* コマンドライン解析が先 */

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] != '\0' ){
			make_stream( argv[ i ] );
		}
	}

	close_stream_file();
	close_symbol_file();

	return 0;
}
