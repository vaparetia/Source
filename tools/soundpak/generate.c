/*
   ストリームデータ作成ルーチン
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "soundpak.h"
#include "parse.h"

/* ---------------------------------------------------------------------- */
/*
*/

static void put_int( FILE *fp, int value )
{
	int i;
#if 0
	unsigned char *p = ( char * )&value;

	for( i = 0; i < sizeof( int ); i++ ){
		fputc( p[ 3 - i ], fp );
	}
#else
	fwrite( &value, sizeof( int ), 1, fp );
#endif
}

static void trim_sector_alignment( FILE *fp )
{
	/* sector alignment にあわせる */

	int pos;

	pos = ftell( fp );

	if( pos % SECTOR_SIZE > 0 ){
		int i;
		for( i = SECTOR_SIZE - ( pos % SECTOR_SIZE ); i > 0; i-- ){
			fputc( 0, fp );
		}
	}
}

/* ---------------------------------------------------------------------- */
/*
*/

static char *keywords[] = {
	"pak",
	"wvx",
	"efx",
	"mdx",
	NULL
};

enum {
	KEY_PAK,
	KEY_WVX,
	KEY_EFX,
	KEY_MDX,
};

static int search_key( char *key )
{
	char **p;
	int n;

	for( n = 0, p = keywords; *p != NULL; p++, n++ ){
		if( strcmp( *p, key ) == 0 ){
			return n;
		}
	}
	return -1;
}

/* ---------------------------------------------------------------------- */
/*
*/

static int file_copy( FILE *out, char *infile )
{
	FILE *in;
	int size, res;
	void *buffer;

	if( ( in = fopen( infile, "rb" ) ) == NULL ){
		ERROR( "%s:見つかりません\n", infile );
	}
	fseek( in, 0, SEEK_END );
	size = ftell( in );
	fseek( in, 0, SEEK_SET );
PRINTF( "OPEN %s size %d\n", infile, size );

	buffer = malloc( size );
	if( buffer == NULL ){
		ERROR( "メモリがありません\n" );
	}
	if( ( res = fread( buffer, sizeof( char ), size, in ) ) < size ){
		ERROR( "読み込み時にエラーが発生しました %d\n", res );
	}
	if( fwrite( buffer, sizeof( char ), size, out ) < size ){
		ERROR( "書き込み時にエラーが発生しました\n" );
	}

	fclose( in );
	free( buffer );

	return ( size + SECTOR_SIZE - 1 ) / SECTOR_SIZE;
}

/* ---------------------------------------------------------------------- */
/*
*/

static char out_file[ 256 ];
static char wvx1_file[ 256 ];
static char wvx2_file[ 256 ];
static char efx_file[ 256 ];
static char mdx_file[ 256 ];

static int get_file_no( char *file, char *format )
{
	char *p;
	int no;
	if( ( p = strrchr( file, '/' ) ) != NULL ){
		p++;
	} else {
		p = file;
	}
	sscanf( p, format, &no );
PRINTF( format, no );
PRINTF( "\n" );
	return no;
}

static int analyze_scr( char *scrfile, char *inpath, char *outpath, char *filter )
{
	int scr_num = 0;

	parse_init();

	if( ! load_file( scrfile ) ){
		ERROR( "Can't open %s\n", scrfile );
	}

	while( get_line() ){
		WORDBUF buf;
		int block_flag;
		int wvx1_flag, wvx2_flag, efx_flag, mdx_flag;

		if( get_word( buf ) == NULL ) continue;
		if( buf[ 0 ] == '#' || buf[ 0 ] == ';' ) continue;
		if( strcmp( buf, "//" ) == 0 ) continue;

		/* ブロックの設定 */

		block_flag = 0;

		if( search_key( buf ) == KEY_PAK ){
			wvx1_flag = 0;
			wvx2_flag = 0;
			efx_flag = 0;
			mdx_flag = 0;

			get_word( buf );

			if( filter[ 0 ] != '\0' ){
				/* フィルター処理 */
				int len;
				len = strlen( filter );
				if( strncmp( filter, buf, len ) != 0 ){
					/* 出力ファイル名が違う */
					while( get_line() ){
						get_word( buf );
						if( buf[ 0 ] == '}' ){
							break;
						}
					}
					continue;
				}
			}

			sprintf( out_file, "%s/%s", outpath, buf );

fprintf( stderr, "PAK %s\n", out_file );

			while( get_line() ){
				int type;
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
				switch( search_key( buf ) ){
				  case KEY_WVX:
					if( get_word( name ) == NULL ){
						ERROR( "filenameが必要です\n" );
					}
					if( wvx1_flag == 0 ){
						wvx1_flag = 1;
						sprintf( wvx1_file, "%s/%s", inpath, name );
					} else if( wvx2_flag == 0 ){
						wvx2_flag = 1;
						sprintf( wvx2_file, "%s/%s", inpath, name );
					} else {
						ERROR( "wvxが3個以上あります\n" );
					}
					break;
				  case KEY_EFX:
					if( get_word( name ) == NULL ){
						ERROR( "filenameが必要です\n" );
					}
					if( efx_flag == 0 ){
						efx_flag = 1;
						sprintf( efx_file, "%s/%s", inpath, name );
					} else {
						ERROR( "efxが2個以上あります\n" );
					}
					break;
				  case KEY_MDX:
					if( get_word( name ) == NULL ){
						ERROR( "no vox file name\n" );
					}
					if( mdx_flag == 0 ){
						mdx_flag = 1;
						sprintf( mdx_file, "%s/%s", inpath, name );
					} else {
						ERROR( "mdxが2個以上あります\n" );
					}
					break;
				  default:
					WARNING( "Wrong Key %s\n", buf );
				}
			}
			/* 設定完了 */
			{
				FILE *out;
				int wvx1_no = 0;
				int wvx2_no = 0;
				int efx_no = 0;
				int mdx_no = 0;
				int pos;

				if( ( out = fopen( out_file, "wb" ) ) == NULL ){
					WARNING( "%s: openできませんでした\n", out_file );
					continue;
				}
				/* ダミー */
				put_int( out, wvx1_flag );
				put_int( out, wvx1_no );
				put_int( out, wvx2_flag );
				put_int( out, wvx2_no );
				put_int( out, efx_flag );
				put_int( out, efx_no );
				put_int( out, mdx_flag );
				put_int( out, efx_no );
				trim_sector_alignment( out );

				pos = 1;
				if( wvx1_flag ){
					wvx1_no = get_file_no( wvx1_file, "wv%06x.wvx" );
					wvx1_flag = pos;
					pos += file_copy( out, wvx1_file );
					trim_sector_alignment( out );
				}
				if( wvx2_flag ){
					wvx2_no = get_file_no( wvx2_file, "wv%06x.wvx" );
					wvx2_flag = pos;
					pos += file_copy( out, wvx2_file );
					trim_sector_alignment( out );
				}
				if( efx_flag ){
					efx_no = get_file_no( efx_file, "se%06x.efx" );
					efx_flag = pos;
					pos += file_copy( out, efx_file );
					trim_sector_alignment( out );
				}
				if( mdx_flag ){
					mdx_no = get_file_no( mdx_file, "sg%06x.mdx" );
					mdx_flag = pos;
					pos += file_copy( out, mdx_file );
					trim_sector_alignment( out );
				}
				fseek( out, 0, SEEK_SET );
				put_int( out, wvx1_flag );
				put_int( out, wvx1_no );
				put_int( out, wvx2_flag );
				put_int( out, wvx2_no );
				put_int( out, efx_flag );
				put_int( out, efx_no );
				put_int( out, mdx_flag );
				put_int( out, mdx_no );
				fseek( out, 0, SEEK_END );

				fclose( out );
			}
		}
	}
	parse_end();

	return 0;
}

/*
   外部関数
*/

void make_soundpak( char *scrfile, char *outpath, char *inpath, char *filter )
{
	int num;

	analyze_scr( scrfile, inpath, outpath, filter );
}
