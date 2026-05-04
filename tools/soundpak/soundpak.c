/*
   ストリームデータ作成ルーチン
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "soundpak.h"

char *program_name = "soundpak";

void usage( void )
{
	printf( "soundpak [ scrfile ... ] [ -o outputpath ] [ -s soundpath ]\n" );
	printf( "          -v : verbose mode\n" );
	printf( "          -d <dirname> : directory filter\n" );
}

static char output_path[ 128 ] = "";
static char sound_path[ 128 ] = "";
static char dir_filter[ 64 ] = "";

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
				if( argv[ i ][ 2 ] != '\0' ){
					strcpy( output_path, &( argv[ i ][ 2 ] ) );
				} else {
					strcpy( output_path, argv[ i + 1 ] );
					argv[ i ][ 0 ] = '\0';
					i++;
				}
				break;
			  case 's':
				if( argv[ i ][ 2 ] != '\0' ){
					strcpy( sound_path, &( argv[ i ][ 2 ] ) );
				} else {
					strcpy( sound_path, argv[ i + 1 ] );
					argv[ i ][ 0 ] = '\0';
					i++;
				}
				break;
			  case 'v':
				{
					extern long verbose_mode;
					verbose_mode = 1;
				}
				break;
			  case 'd':
				/* ディレクトリフィルタ指定 */
				{
					char filter[ 64 ];
					if( argv[ i ][ 2 ] != '\0' ){
						strcpy( filter, &( argv[ i ][ 2 ] ) );
					} else {
						strcpy( filter, argv[ i + 1 ] );
						argv[ i ][ 0 ] = '\0';
						i++;
					}
					sprintf( dir_filter, "%s/", filter );
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

int main( int argc, char *argv[] )
{
	long i;

	if( ! analize_option( argc, argv ) ){
		usage();
		return 1;
	}

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] != '\0' ){
			make_soundpak( argv[ i ], output_path, sound_path, dir_filter );
		}
	}

	return 0;
}
