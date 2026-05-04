/*
	@file
	gclconv メインルーチン
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "mymalloc.h"
#include "fontconv.h"
#include "gclconv.h"

#include "version.h"

extern int tsymbol_dump_flag;
extern int no_warn_flag;

char *program_name = "gclconv";
int gco_version = GCO_FORMAT_VERSION;

int output_stage_c = 0;
int block_output_mode = 0;
int make_depend_mode = 0;
int tsymbol_no_check_flag = 0;
extern int tsymbol_no_error_flag;

static char output_stage_path[ 256 ] = "";
static char font_path[ 256 ] = "";
static char dump_function[ 64 ] = "";

static char command_def_file[ 256 ] = "command.def";
static char zen_table_name[ 256 ] = "zen_table";
static char all_font_name[ 256 ] = "fontall.dat";
static int font_option_set = 0;

static void usage( void )
{
	printf( "%s <filename>\n", program_name );
	printf( "option -o <output file>\n" );
	printf( "       -v                : debug mode\n" );
#if 0
	printf( "       -c                : dump chara name\n" );
	printf( "       -e                : eulang flag\n" );
#endif
	printf( "       -k                : Don't convert EUC->SJIS (KOREA)\n" );
	printf( "       -w                : Inhibit warning\n" );
	printf( "       -wt               : Inhibit tsymbol error\n" );
	printf( "       -ct               : dump tsymbol refered\n" );
	printf( "       -b                : block output mode\n" );
	printf( "       -d <filename>     : change command.def\n" );
	printf( "       -f <path>         : font table and data path.\n" );
	printf( "       -l <path>         : external charactor list path\n" );
	printf( "       -s <id>           : dump strid\n" );
	printf( "       -m <path>         : make stage chara .c\n" );
	printf( "       -A <path>         : make stage all chara .c\n" );
	printf( "       -p                : print chara list\n" );
	printf( "       -D <TAG>[=<VALUE>]: define\n" );
	printf( "       -I <inc path>     : include path ( once only )\n" );
	printf( "       -t <filename>     : tsymbol read ( repeat ok )\n" );
	printf( "       -M <filename>     : make depend file\n" );
	printf( "       -cd               : define check all word\n" );
	printf( "       -F <FuncName>     : dump function use line\n" );
	printf( "       -n <01:02:1A..>   : check proc number(hex:max0x20)\n" );
	printf( "       -fl               : limited font\n" );
	printf( "       -fc <keepfont> <w> <h> <cskip> <lskip> : kinsoku\n" );
	printf( "       -fe <keepfont> <w> <h> <cskip> <lskip> : no kinsoku\n" );
	printf( "       -ft <zen_table_name>\n" );
	printf( "       -fa <font_all_name>\n" );
	printf( "                         : font draw check\n" );
	printf( "ver.%s %s %s compiled\n", GCLCONV_VERSION, __DATE__, __TIME__ );
	exit( 1 );
}

static char *make_outfile_name( char *infile, char *ext )
{
	char *p;
	static char outfile_name[ 128 ];

	strcpy( outfile_name, infile );

	if( ( p = strrchr( outfile_name, '.' ) ) != NULL ){
		*p = '\0';
	} else {
		p = outfile_name + strlen( outfile_name );
	}
	strcat( p, ext );
	return outfile_name;
}

static int my_atoi( int *value, char *str )
{
	int v = 0;
	if( str[ 0 ] == '0' && ( str[ 1 ] | 0x20 ) == 'x' ){
		/* 16進 */
		char *p = str + 2;

		while( *p != '\0' ){
			int c = *( p ++ ) | 0x20;
			if( c >= '0' && c <= '9' ){
				c = c - '0';
			} else if( c >= 'a' && c <= 'f' ){
				c = 10 + c - 'a';
			} else {
				return 0;	/* 16進以外 */
			}
			v = v * 16 + c;
		}
	} else {
		/* 10進 */
		char *p = str;

		while( *p != '\0' ){
			int c = *( p ++ );
			if( c >= '0' && c <= '9' ){
				c = c - '0';
			} else {
				return 0;	/* 10進以外 */
			}
			v = v * 10 + c;
		}
	}
	*value = v;
	return 1;
}

static char *get_arg_filename( char **argv, int *offset, int argc )
{
	if( argv[ *offset ][ 2 ] != '\0' ){
		return &( argv[ *offset ][ 2 ] );
	} else if( *offset + 1 < argc ){
		char *res;
		res = argv[ *offset + 1 ];
		argv[ *offset ][ 0 ] = '\0';
		*offset = *offset + 1;
		return res;
	}
	printf( "ERROR: no arg\n" );
	exit( 1 );
	return NULL;
}

static int analyze_option( int argc, char *argv[], char **infile, char **outfile )
{
	int i;

	*infile = NULL;
	*outfile = NULL;

	for( i = 1; i < argc; i++ ){
		if( argv[ i ][ 0 ] == '-' ){
			switch( argv[ i ][ 1 ] ){
			  case 'h':
				usage();
				break;
			  case 'c':
				if( argv[ i ][ 2 ] == 't' ){
					tsymbol_dump_flag = 1;
				} else if( argv[ i ][ 2 ] == 'd' ){
					parse_check_define_mode( 1 );
				}
				break;
			  case 'o':
				if( *outfile != NULL ){
					ERROR( "出力ファイルが複数指定されています\n" );
				}
				*outfile = get_arg_filename( argv, &i, argc );
				break;
			  case 'v':
				{
					extern int verbose_mode;
					verbose_mode = 1;
				}
				break;
			  case 'b':
				block_output_mode = 1;
				break;
			  case 't':
				{
					char *file;

					file = get_arg_filename( argv, &i, argc );
					load_symbol_file( file );
				}
				break;
			  case 's':
				{
					extern int dump_strid_flag;
					extern int dump_strid_code;

					char *str;
					str = get_arg_filename( argv, &i, argc );

					if( my_atoi( &dump_strid_code, str ) ){
						/* 選択表示モード */
						dump_strid_flag = 2;
						printf( "dump strid = %d\n", dump_strid_code );
					} else {
						/* 全部表示モード */
						dump_strid_flag = 1;
					}
				}
				break;
#if 0
			  case 'e':
				{
					extern int eu_lang_flag;
					eu_lang_flag = 1;
				}
				break;
#endif
			  case 'd':
				{
					char *str;

					str = get_arg_filename( argv, &i, argc );
					strcpy( command_def_file, str );
				}
				break;
			  case 'f':
				{
					font_option_set = 1;
					if( argv[ i ][ 2 ] == 'c' || argv[ i ][ 2 ] == 'e' ){
						// font check
						if( i + 5 >= argc ){
							FATAL( "no arg\n" );
						}
						font_area_check_flag = 1;
						strcpy( font_area_check.fontfile, argv[ i + 1 ] );
						font_area_check.width = atoi( argv[ i + 2 ] );
						font_area_check.height = atoi( argv[ i + 3 ] );
						font_area_check.c_skip = atoi( argv[ i + 4 ] );
						font_area_check.l_skip = atoi( argv[ i + 5 ] );
						font_area_check.kinsoku = ( argv[ i ][ 2 ] == 'c' ) ? 1 : 0;
						i += 5;
					} else if( argv[ i ][ 2 ] == 't' ){
						strcpy( zen_table_name, argv[ i + 1 ] );
						i++;
					} else if( argv[ i ][ 2 ] == 'a' ){
						strcpy( all_font_name, argv[ i + 1 ] );
						i++;
					} else if( argv[ i ][ 2 ] == 'l' ){
						extern int font_no_expand_flag;
						font_no_expand_flag = 1;
					} else {
						char *str;

						str = get_arg_filename( argv, &i, argc );
						strcpy( font_path, str );
					}
				}
				break;
			  case 'l':
				{
					char *str;

					str = get_arg_filename( argv, &i, argc );
					set_external_chara_list_path( str );
				}
				break;
			  case 'm':
			  case 'A':
				{
					char *str;

					/* make .c file flag on */
					if( argv[ i ][ 1 ] == 'm' ){
						output_stage_c = 1;
					} else {
						output_stage_c = 2;
					}

					str = get_arg_filename( argv, &i, argc );
					strcpy( output_stage_path, str );
				}
				break;
			  case 'p':
				output_stage_c = 3;
				break;
			  case 'D':
				{
					char *p;
					char buf[ 128 ];

					strcpy( buf, get_arg_filename( argv, &i, argc ) );

					if( ( p = strchr( buf, '=' ) ) != NULL ){
						*p = '\0';
						set_define( buf, p + 1 );
					} else {
						set_define( buf, "1" );
					}
				}
				break;
			  case 'I':
				{
					char *str;

					str = get_arg_filename( argv, &i, argc );
					set_include_path( str );
				}
				break;
			  case 'n':
				{
					char *str;
					int mask;
					extern int proc_prefix_check_mask;

					str = get_arg_filename( argv, &i, argc );
					mask = 0;
					for( ;; ){
						int value;
						value = strtol( str, &str, 16 );
						if( value < 1 || value > 32 ){
							FATAL( "-n: チェックできるのは1から32までです。\n" );
						}
						mask |= ( 1 << ( value - 1 ) );
						if( *str == '\0' ){
							break;
						}
						if( *str == ':' ){
							str++;
						} else {
							FATAL( "-n: [%s]フォーマットが不正です。\n", str );
						}
					}
					proc_prefix_check_mask = mask;
				}
				break;
			  case 'M':
				{
					char *str;
					str = get_arg_filename( argv, &i, argc );

					parse_set_depend_log_file( str );
					make_depend_mode = 1;
				}
				break;
			  case 'F':
				{
					char *str;
					str = get_arg_filename( argv, &i, argc );

					strcpy( dump_function, str );
				}
				break;
			  case 'w':
				if( argv[ i ][ 2 ] == 't' ){
					tsymbol_no_error_flag = 1;
				} else {
					no_warn_flag = 1;
				}
				break;
			  case 'k':
				{
					extern int font_no_euc_conv;
					font_no_euc_conv = 1;
				}
				break;
			}
		} else {
			if( *infile == NULL ){
				*infile = argv[ i ];
			} else {
				WARNING( "複数のファイルが指定されています %s:無視\n", argv[ i ] ); 
			}
		}
	}

	if( *infile == NULL ){
		usage();
	}

	if( *outfile == NULL ){
		*outfile = make_outfile_name( *infile, ".gco" );
	}
	return 1;
}

static void output_depend( char *infile, char *outfile )
{
	parse_init();

	if( ! load_file( infile ) ){
		FATAL( "Error:ファイル %s がオープンできません\n", infile );
	}

	parse_set_depend_outfile( outfile );
	do {
		char buf[ LINE_BUFFER_SIZE ];
		while( get_word_pp( buf ) != NULL );
	} while( get_line() != 0 );
	parse_close_depend_log_file();
	parse_end();
}

static void output_stage_chara_list( char *infile )
{
	/* ステージキャラ定義ファイルの出力 */
	if( output_stage_c != 3 ){
		char *out;
		char stage_out[ 128 ];
		char stage_in[ 128 ];
		
		FILE *fp;

		out = make_outfile_name( infile, ".c" );
		{
			char *p;
			if( ( p = strrchr( out, '/' ) ) != NULL ){
				out = p + 1;
			}
		}
		sprintf( stage_out, "%s/%s", output_stage_path, out );

		if( ( fp = fopen( stage_out, "rb" ) ) == NULL ){
			/* stagefileがない */
			sprintf( stage_in, "_base.c" );
		} else {
			fclose( fp );
			strcpy( stage_in, stage_out );
		}

		if( output_stage_c == 2 ){
			set_all_chara_output();
		}

		printf( "output %s\n", stage_out );
		output_stage_file( stage_in, stage_out );
	} else {
		print_chara_list();
	}
}

extern int bp_argc;
extern char * bp_argv[];
extern void bp_expand_args( int argc, char *argv[] );

/*!
	main
*/
int main( int argc, char *argv[] )
{
	char *infile, *outfile;

	init_Malloc();

	parse_init_define();
	init_sym_table();

   bp_expand_args( argc, argv );

	if( ! analyze_option( bp_argc, bp_argv, &infile, &outfile ) ){
		return 1;
	}

	if( make_depend_mode != 0 ){
		/* depend 出力のみ */
		output_depend( infile, outfile );

		return 0;
	}

	init_group();

	check_duplicate_symbol();

	setup_table( command_def_file );

	if( font_option_set ){
		font_set_load_path( font_path );
		font_init_font_file( GCL_FONT_CODE, all_font_name );
		font_load_table( zen_table_name );
	}

	if( ! block_output_mode ){
		/* 通常GCL出力 */
		if( output_stage_c ){
			tsymbol_no_check_flag = 1;
			convert_script( infile, NULL );
			output_stage_chara_list( infile );
		} else {
			convert_script( infile, outfile );
		}
		if( dump_function[ 0 ] != '\0' ){
			dump_chara_function( dump_function );
		}
	} else {
		/* BLOCK MODE出力 */
		convert_script_block_mode( infile, outfile );
	}
	parse_free_define();
	free_Malloc();

	return 0;
}
