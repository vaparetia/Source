/*
	ストリーミングデータのダンププログラム (チェック用)
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "stream.h"

static int v_flag = 0;

void usage( void )
{
	printf( "dumpstream [option] streamfile [startoffset]\n" );
	printf( "  option -i  : dump input format\n" );
	printf( "         -v  : dump data head (16byte)\n" );
}

typedef struct {
	int type;
	int size;
	int tick;
	int option;
} STREAM_TAG;

static void dump_str( FILE *fp )
{
	STREAM_TAG tag;

	int num = 0;

	for( ;; ){
		int pos;

		pos = ftell( fp );
		if( fread( &tag, sizeof( STREAM_TAG ), 1, fp ) <= 0 ){
			break;
		}
		printf( "%08X : %08X %08X %08d %08X\n"
				, pos, tag.type, tag.size, tag.tick, tag.option );
		if( v_flag != 0 ){
			unsigned char buf[ 16 ];
			if( fread( buf, sizeof( char ), 16, fp ) == 16 ){
				int i;
				printf( "     " );
				for( i = 0; i < 16; i++ ){
					printf( " %02X", buf[ i ] );
				}
				printf( "\n" );
			}
		}
		fseek( fp, pos + tag.size, SEEK_SET );

		if( feof( fp ) ){
			printf( "non terminated data\n ");
			return;
		}

		if( tag.type == CHANK_TYPE_END ){
			/* 終端 */
			pos = ftell( fp );
			/* セクタ単位にそろえる */
			pos = ( pos + SECTOR_SIZE - 1 ) / SECTOR_SIZE * SECTOR_SIZE;
			fseek( fp, pos, SEEK_SET );
			printf( "--- STREAM %d end\n", num );
			num ++;
		}
	}
}

typedef struct {
	int type;
	int size;
	int tick;
	int res;
} INPUT_TAG;

static void dump_input( FILE *fp )
{
	INPUT_TAG tag;

	for( ;; ){
		int pos;

		pos = ftell( fp );
		if( fread( &tag, sizeof( INPUT_TAG ), 1, fp ) <= 0 ){
			break;
		}
		printf( "%08X : %08X %08X %08d %08X\n"
				, pos, tag.type, tag.size, tag.tick, tag.res );
		if( tag.size == 0 ){
			printf( "ZERO PACKET\n" );
			break;
		}
		fseek( fp, pos + tag.size, SEEK_SET );
	}
}

int main( int argc, char *argv[] )
{
	FILE *fp;
	char *filename;
	int which;
	int last;

	if( argc < 2 ){
		usage();
		return 1;
	}
	filename = argv[ 1 ];
	last = 2;
	which = 0;
	if( strcmp( argv[ 1 ], "-i" ) == 0 ){
		if( argc < 3 ){
			usage();
			return 1;
		}
		filename = argv[ 2 ];
		which = 1;
		last = 3;
	}
	if( strcmp( argv[ 1 ], "-v" ) == 0 ){
		if( argc < 3 ){
			usage();
			return 1;
		}
		filename = argv[ 2 ];
		v_flag = 1;
		last = 3;
	}
	if( ( fp = fopen( filename, "rb" ) ) == NULL ){
		printf( "%s is not open\n", filename );
		return 1;
	}
	if( argc > last ){
		int offset;

		offset = strtol( argv[ last ], NULL, 0 );
					   
		printf( "START OFFSET = %X\n", offset );
		fseek( fp, offset * 2048, SEEK_SET );
	}

	if( which == 0 ){
		dump_str( fp );
	} else {
		dump_input( fp );
	}

	fclose( fp );

	return 0;
}
