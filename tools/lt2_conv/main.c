/*
	main.c
	旧ライトデータ（.lit）＞新ライトデータ（.lt2）変換プログラム

	1999/09/27 K.Takabe
	$Id: main.c,v 1.2 1999/10/22 05:24:11 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "fmt_lit.h"

typedef struct {
	int		div_x, div_y, div_z ;
} Work ;

/* ---------------------------------------------------------------- */
extern int ProgMain( int n_files, char **in_files, float scale );
extern int ConvertLitFile( char *filename, float scale );

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256] ;
	int		mode = -1, option_flag = 0 ;
	Work	work ;
	float	scale = 1.0f ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 's' ){
				sscanf( argv[1], "%f", &scale );
				argv++ ;
				argc-- ;
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
		fprintf( stderr, ".lit > .lt2 converter Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: lt2_conv [options] <inputfile(*.lit)> ... \n" );
		fprintf( stderr, "(Options)  -s scale ... data scaling \n" );
		//fprintf( stderr, "(Options)  -d div_x div_y div_z ... group divide \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return ProgMain( get_input_files, input_files, scale );
}

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, float scale )
{
	int		ret ;
	for ( ; n_files > 0 ; n_files--, in_files++ ){
		fprintf( stdout, "convert...(%s)\n", *in_files );
		ret = ConvertLitFile( *in_files, scale );
		if ( ret ) return ( ret );
	}
	return ( 0 );
}
