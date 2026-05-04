/*
	main.c
	モーションストリーム圧縮プログラム

	2000/09/25 K.Takabe
	$Id: main.c,v 1.4 2001/08/06 11:08:15 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "gte.h"
#include "quat.h"
#include "fmt_demo.h"
#include "motion.h"
#include "main.h"

/* ---------------------------------------------------------------- */
extern int Convert( char *filename, PROGRAM_OPTION *option );

PROGRAM_OPTION	Option ;
/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256] ;
	int		mode = -1 ;
	int		error_flag = 0 ;

	Option .n_datas = 0 ;
	Option.base_tick = 5 ;
	Option.flag = 0 ;
	Option.frame_skip = 2 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'p' ){
				/* PAL mode */
				Option.base_tick = 6 ;
			}
			if ( argv[0][1] == 'e' ){
				Option.flag |= PROGRAM_FLAG_NEWVERTION ;
			}
			if ( argv[0][1] == 'd' ){
				Option.flag |= PROGRAM_FLAG_DUMP ;
			}
			if ( argv[0][1] == 'f' ){
				Option.frame_skip = atoi( argv[1] );
				argv++ ;
			}
			if ( argv[0][1] == 'i' ){
				if ( argc < 3 ){
					error_flag = 1 ;
					break ;
				}
				if ( Option.n_datas >= MAX_DATAS ) continue ;
				Option.input_data[Option.n_datas].strcode = atoi( argv[1] );
				strcpy( Option.input_data[Option.n_datas].filename, argv[2] );
				printf("input data(%d,%s)\n",
					   Option.input_data[Option.n_datas].strcode,
					   Option.input_data[Option.n_datas].filename );
				Option.n_datas++ ;
				argc -= 2 ;
				argv += 2 ;
			}
		} else {
			switch ( mode ){
			default:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				strcpy( Option.out_filename, argv[0] );
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( ( get_input_files < 1 && get_input_files < 1 ) || error_flag ){
		fprintf( stderr, ".mtn to .mst converter   Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: mtn2mst -o <out_file(*.mst)> -i <name_code> <in_file(*.mtn)> [-i ...] \n" );
		fprintf( stderr, "(Options)  -p    ... pal mode out(50frame/sec) \n" );
		fprintf( stderr, "           -e    ... new vertion mst convert \n" );
		fprintf( stderr, "           -f n  ... frame skip (dafault:2) \n" );
		fprintf( stderr, "           -d <in_file(*.mst)>   ... dump mst-file \n" );
		//fprintf( stderr, "(Options)  -d div_x div_y div_z ... group divide \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	if ( Option.flag & PROGRAM_FLAG_DUMP ){
		return DumpMst( input_files[0], &Option );
	}
	if ( Option.flag & PROGRAM_FLAG_NEWVERTION ){
		return Convert2( Option.out_filename, &Option );
	}
	return Convert( Option.out_filename, &Option );
}

/* ---------------------------------------------------------------- */
#if 0
int ProgMain( int n_files, char **in_files )
{
	int	ret ;
	for ( ; n_files > 0 ; n_files--, in_files++ ){
		fprintf( stdout, "convert...(%s)\n", *in_files );
		//ret = DietDe2( "test.de2", *in_files );
		if ( ret ) return ( ret );
	}
	return ( 0 );
}
#endif
