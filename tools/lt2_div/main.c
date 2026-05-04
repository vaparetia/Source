/*
	main.c
	新ライトデータ（.lt2）グループ分割プログラム

	1999/10/01 K.Takabe
	$Id: main.c,v 1.3 1999/10/22 05:23:52 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "fmt_lit.h"

typedef struct {
	int		option ;
	int		div_x, div_y, div_z ;
} Work ;

/* ---------------------------------------------------------------- */
extern int ProgMain( int n_files, char **in_files, Work *work );
extern int DivideLt2File( char *filename, char *out_filename, int flag, int x, int y, int z );

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256] ;
	int		mode = -1, option_flag = 0 ;
	Work	work = { 0, 5000, 5000, 5000 } ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'd' ){
				if ( argc > 3 ){
					work.div_x = atoi( argv[1] );
					work.div_y = atoi( argv[2] );
					work.div_z = atoi( argv[3] );
					argv += 3 ;
					argc -= 3 ;
				} else {
					get_input_files = -1 ;
					break ;
				}
			}
		} else {
			switch ( mode ){
			  default:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files < 1 || get_input_files > 255 ){
		fprintf( stderr, ".lt2 divide Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: lt2_div [options] <inputfile(*.lt2)> \n" );
		fprintf( stderr, "(Options)  -d div_x div_y div_z ... group divide \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return ProgMain( get_input_files, input_files, &work );
}

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, Work *work )
{
	int	ret ;
	for ( ; n_files > 0 ; n_files--, in_files++ ){
		fprintf( stdout, "convert...(%s)\n", *in_files );
		ret = DivideLt2File( *in_files, *in_files, work->option, work->div_x, work->div_y, work->div_z );
		if ( ret ) return ( ret );
	}
	return ( 0 );
}
