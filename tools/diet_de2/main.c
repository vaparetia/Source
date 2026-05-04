/*
	main.c
	デモファイル圧縮プログラム

	2000/09/20 K.Takabe
	$Id: main.c,v 1.2 2001/08/07 10:52:18 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "gte.h"
#include "quat.h"
#include "fmt_demo.h"

/* ---------------------------------------------------------------- */
int ProgMain( char *out_file, int n_files, char **in_files );
extern int DietDe2( char *out_filename, char *in_filename );

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256], *out_file = NULL ;
	int		mode = -1 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'i' ){
				mode = -1 ;
			}
			if ( argv[0][1] == 'o' ){
				mode = 0 ;
			}
		} else {
			switch ( mode ){
			case 0:
				out_file = argv[0] ;
				break ;
			default:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( out_file == NULL || get_input_files < 1 || get_input_files > 255 ){
		fprintf( stderr, ".de2 file compresser   Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: diet_de2 [options] -o <outfile(*.de2)> -i <inputfile(*.de2)> \n" );
		//fprintf( stderr, "(Options)  -d div_x div_y div_z ... group divide \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return ProgMain( out_file, get_input_files, input_files );
}

/* ---------------------------------------------------------------- */
int ProgMain( char *out_file, int n_files, char **in_files )
{
	int	ret ;
	for ( ; n_files > 0 ; n_files--, in_files++ ){
		fprintf( stdout, "convert...(%s)\n", *in_files );
		ret = DietDe2( out_file, *in_files );
		if ( ret ) return ( ret );
	}
	return ( 0 );
}
