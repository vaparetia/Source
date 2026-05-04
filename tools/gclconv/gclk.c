/*
   gcl 用リンクプログラム
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "mymalloc.h"
#include "gclk.h"

#include "version.h"

char *program_name = "gclk";
char *varsym_version = GCLK_VERSION;
int gco_version = GCO_FORMAT_VERSION;

static char stage_path_buf[ 128 ] = "";

static char *output_stage_path = NULL;
int block_output_mode = 0;

/*static long read_table_flag = FALSE;*/
static char output_name[ 128 ] = "scenerio.gcx";
static char linkvar_name[ 128 ] = "linkvar.def";
static char table_file_name[128] = "";

static int output_name_flag = 0;
static int var_warn_flag = 1;
static int extend_filename_flag = 0;

#define VARIABLE_TABLE_FILE		"variable.sym"
char variable_table_name[ 128 ] = VARIABLE_TABLE_FILE;
char bp_output_variable_table_name[] = "_bp_variable.sym";
static void usage( void )
{
	printf( "%s <gco files> [options]\n", program_name );
	printf( "\t -fv <variable table file>\n" );
	printf( "\t -fl <link variable file>\n" );
	printf( "\t -p <stage path>\n" );
	printf( "\t -i  : link var header output\n" );
	printf( "\t -v  : debug mode\n" );
	printf( "\t -b  : block output mode\n" );
	printf( "\t -bl : block output mode[large]\n" );
	printf( "\t -w  : inhibit warning\n" );
	printf( "\t -e  : extend filename mode\n" );
	printf( "\t -l <gsize> [<lsize>]  : set variable size limit\n" );
	printf( "\t -o <output name>      : output name ( default scenerio.gcx )\n" );
	printf( "\t -t <output symbol>    : output symbol name ( block mode )\n" );
	printf( "ver.%s %s %s compiled\n", GCLK_VERSION, __DATE__, __TIME__ );

	exit( 1 );
}

#define MAX_FILE_NUM	512
#define MAX_FILE_NAME_BUF	16*1024

static char *file_name_ptr[ MAX_FILE_NUM ];
static char file_name_buf[ MAX_FILE_NAME_BUF ];

static int file_name_num = 0;
static char *file_name_buf_p = file_name_buf;

#ifdef _DOS_
#include <io.h>
#endif

static char *new_file( char *name )
{
	strcpy( file_name_buf_p, name );

	file_name_ptr[ file_name_num ++ ] = file_name_buf_p;
	if( file_name_num >= MAX_FILE_NUM ){
		FATAL( "ファイル数が多すぎます\n" );
	}
	file_name_buf_p += strlen( name ) + 1;
	if( file_name_buf_p - file_name_buf >= MAX_FILE_NAME_BUF ){
		FATAL( "ファイル名バッファがあふれました\n" );
	}
	return 0;
}

static void set_filename( char *name )
{
#ifndef _DOS_
	/* UNIX:ワイルドカードの展開の必要ない */
	new_file( name );
#else
	/* DOS:ワイルドカード展開 */

	if( strchr( name, '*' ) == NULL ){
		new_file( name );
	} else {
		long wild_file = 0;
		struct _finddata_t fd;
		long hf;
		char path[ 128 ], fullname[ 256 ], *p;

		strcpy( path, name );
		if( ( p = strrchr( path, '/' ) ) != NULL
		   || ( p = strrchr( path, '\\' ) ) != NULL ){
			*( p + 1 ) = '\0';
		} else {
			path[ 0 ] = '\0';
		}

		if( ( hf = _findfirst( name, &fd ) ) != -1 ){
			sprintf( fullname, "%s%s", path, fd.name );
			new_file( fullname );
		}
		while( _findnext( hf, &fd ) == 0 ){
			sprintf( fullname, "%s%s", path, fd.name );
			new_file( fullname );
		}
		_findclose( hf );
	}
#endif
}

static int get_file_num( void )
{
	return file_name_num;
}

static char *get_file( long no )
{
	return file_name_ptr[ no ];
}

long analyze_option( int argc, char *argv[] )
{
	long i;

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			switch( argv[ i ][ 1 ] ){
			  default:
			  case 'h':
				usage();
				break;
			  case 'i':
				header_output_flag = TRUE;
				printf( "linkvar header output.\n" );
				break;
			  case 'p':
				if( argv[ i ][ 2 ] != '\0' ){
					strcpy( stage_path_buf, &( argv[ i ][ 2 ] ) );
				} else if( i + 1 < argc ){
					strcpy( stage_path_buf, argv[ i + 1 ] );
					argv[ i ][ 0 ] = '\0';
					i++;
				} else {
					FATAL( "-p:パスを指定して下さい\n" );
				}
				output_stage_path = stage_path_buf;
				break;
			  case 'b':
				{
					if( argv[ i ][ 2 ] == 'l' ){
						// large mode
						block_output_mode = 2;
					} else {
						block_output_mode = 1;
					}
				}
				break;
			  case 'v':
				{
					verbose_mode = 1;
				}
				break;
			  case 'o':
				if( argv[ i ][ 2 ] != '\0' ){
					strcpy( output_name, &( argv[ i ][ 2 ] ) );
				} else if( i + 1 < argc ){
					strcpy( output_name, argv[ i + 1 ] );
					argv[ i ][ 0 ] = '\0';
					i++;
				} else {
					FATAL( "-o:出力名を指定して下さい\n" );
				}
				output_name_flag = 1;
				break;
			  case 'f':
				if( argv[ i ][ 2 ] == 'v' ){
					strcpy( variable_table_name, argv[ i + 1 ] );
				} else if( argv[ i ][ 2 ] == 'l' ){
					strcpy( linkvar_name, argv[ i + 1 ] );
				} else {
					FATAL( "-f[vl]:どちらかを指定してください\n" );
				}
				argv[ i ][ 0 ] = '\0';
				i++;
				break;
			  case 't':
				if( argv[ i ][ 2 ] != '\0' ){
					strcpy( table_file_name, &( argv[ i ][ 2 ] ) );
				} else if( i + 1 < argc ){
					strcpy( table_file_name, argv[ i + 1 ] );
					argv[ i ][ 0 ] = '\0';
					i++;
				} else {
					FATAL( "-t:テーブルファイルを指定して下さい\n" );
				}
				break;
			  case 'w':
				var_warn_flag = 0;
				break;
			  case 'e':
				extend_filename_flag = 1;
				break;
			  case 'l':
				{
					char buf[ 128 ];
					int limit;
					strcpy( buf, "" );
					if( argv[ i ][ 2 ] != '\0' ){
						strcpy( buf, &( argv[ i ][ 2 ] ) );
					} else if( i + 1 < argc ){
						strcpy( buf, argv[ i + 1 ] );
						argv[ i ][ 0 ] = '\0';
						i++;
					} else {
						FATAL( "数値を指定して下さい\n" );
					}

					limit = strtol( buf, NULL, 0 );
					if( limit <= 0 ){
						FATAL( "-l:limitの数値が正しくありません\n" );
					}
					set_variable_buffer_size( limit );

					if( i + 1 < argc && argv[ i + 1 ][ 0 ] != '-' ){
						strcpy( buf, argv[ i + 1 ] );
						argv[ i ][ 0 ] = '\0';
						i++;

						limit = strtol( buf, NULL, 0 );
						if( limit <= 0 ){
							FATAL( "-l:limitの数値が正しくありません\n" );
						}
						set_local_variable_buffer_size( limit );
					}
				}
				break;
			}
			argv[ i ][ 0 ] = '\0';
		} else {
			set_filename( argv[ i ] );
		}
	}

	return 1;
}

static void check_ext( char *buffer )
{
	char *p;
	if( ( p = strrchr( buffer, '.' ) ) != NULL ){
		*p = '\0';
		return;
	}
	ERROR( "入力ファイルの拡張子がありません\n" );
}

static void check_ext_extend( char *input, char *outname, char *outdir )
{
	char buffer[ 256 ];
	char *p;

	strcpy( buffer, input );

	if( ( p = strrchr( buffer, '.' ) ) != NULL ){
		char *pp;

		*p = '\0';

		if( ( pp = strrchr( buffer, '.' ) ) != NULL ){
			*pp = '\0';
			strcpy( outdir, buffer );
			strcpy( outname, pp + 1 );
		} else {
			strcpy( outdir, buffer );
		}
		return;
	}
	ERROR( "入力ファイルの拡張子がありません\n" );
}

static char *binary_file_name( char *name )
{
	char *p;
	static char outfile_name[ 128 ];

	if( output_stage_path == NULL ){
		/* P オプションがついていない */

		if( ! output_name_flag ){
			strcpy( outfile_name, name );

			check_ext( outfile_name );
			strcat( outfile_name, ".gcx" );
		} else {
			strcpy( outfile_name, output_name );
		}
	} else {
		char buf[ 128 ];
		if( ( p = strrchr( name, '/' ) ) != NULL
			 || ( p = strrchr( name, '\\' ) ) != NULL ){
			strcpy( buf, p + 1 );
		} else {
			strcpy( buf, name );
		}
		p = output_stage_path + strlen( output_stage_path ) - 1;
		if( *p == '/' || *p == '\\' ){
			*p = '\0';
		}

		if( extend_filename_flag == 0 ){
			check_ext( buf );
			sprintf( outfile_name, "%s/%s/%s", output_stage_path, buf, output_name );
		} else {
			// 拡張ファイルネーム解析
			char outname[ 64 ];
			char outdir[ 64 ];
			strcpy( outname, "scenerio" );
			check_ext_extend( buf, outname, outdir );
			
			sprintf( outfile_name, "%s/%s/%s.gcx", output_stage_path, outdir, outname );
		}
	}
	return outfile_name;
}

extern int bp_argc;
extern char * bp_argv[];
extern void bp_expand_args( int argc, char *argv[] );

int main( int argc, char *argv[] )
{
	long i;

	init_Malloc();

	parse_init_define();

   bp_expand_args( argc, argv );

	if( ! analyze_option( bp_argc, bp_argv ) ){
		return 1;
	}
	init_variable_table();
	load_linkvar_table( linkvar_name );

	if( header_output_flag ){
		/* linkvar.h を出力するだけ */
		return 0;
	}
	if( get_file_num() == 0 ){
		usage();
	}

	if( load_variable_table( variable_table_name ) == FALSE ){
		PRINTF( "Variable Table Refreshed\n" );
	}

	if( ! block_output_mode ){
		/* 通常モード */
		int num;

		num = get_file_num();

		for( i = 0; i < num; i++ ){
			read_variable_from_file( get_file( i ) );
		}

		output_variable_table( bp_output_variable_table_name, var_warn_flag );

DUMP( "Writing %d gcx files...\n", num );
		for( i = 0; i < num; i++ ){
			write_binarys( get_file( i ), binary_file_name( get_file( i ) ) );
		}
		dump_var_report();
	} else {
		/* ブロックモード */
		FILE *out, *table;

		if( ! output_name_flag ){
			strcpy( output_name, "block.dat" );
		}
		if( table_file_name[ 0 ] == '\0' ){
			char *p;
			strcpy( table_file_name, output_name );

			if( ( p = strrchr( table_file_name, '.' ) ) == NULL ){
				p = table_file_name + strlen( table_file_name );
			}
			strcpy( p, ".lst" );
		}

		for( i = 0; i < get_file_num(); i++ ){
			read_variable_block_mode( get_file( i ) );
		}

		output_variable_table( bp_output_variable_table_name, 0 );

		if( ( out = fopen( output_name, "wb" ) ) == NULL ){
			FATAL( "%sがオープンできません。\n", output_name );
		}
		if( ( table = fopen( table_file_name, "wt" ) ) == NULL ){
			FATAL( "%sがオープンできません。\n", table_file_name );
		}

		printf( "Output %s...\n", output_name );
		{
			int num = 0;
			for( i = 0; i < get_file_num(); i++ ){
				num += write_binarys_block_mode( get_file( i ), out, table );
			}
			printf( "%d Blocks\n", num );
		}
	}

	parse_free_define();
	free_Malloc();

	return 0;
}
