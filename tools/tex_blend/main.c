/*
	main.c
	テクスチャαブレンド合成プログラム

	2001/03/05 K.Takabe
	$Id: main.c,v 1.2 2001/12/04 10:18:50 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>


/* ---------------------------------------------------------------- */
int ProgMain( char *output_file, char **in_files );
extern int Conv( char *out_filename, char *in_filename1, char *in_filename2 );
extern int Conv2( char *out_filename, char *in_filename1 );

static int conv_mode = 0 ;

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256], *output_file = NULL ;
	int		mode = -1 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'i' ){
				mode = -1 ;
			}
			if ( argv[0][1] == 'o' ){
				mode = 1 ;
			}
			if ( argv[0][1] == 'm' ){
				conv_mode = 1 ;
			}
		} else {
			switch ( mode ){
			case 1:
				output_file = argv[0] ;
				break ;
			default:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files < 1 || get_input_files > 2 ){
		fprintf( stderr, ".tex alpha blend program   Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: tex_blend [options]-o <outfile> -i <col_file(*.bmp)> <alpha_file(*.bmp)> \n" );
		fprintf( stderr, "(Options)  -m ... make mipmap \n" );
		//fprintf( stderr, "(Options)  -d div_x div_y div_z ... group divide \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	if ( get_input_files == 1 ) input_files[1] = NULL ;
	if ( output_file == NULL ) output_file = input_files[0] ;
	return ProgMain( output_file, input_files );
}

/* ---------------------------------------------------------------- */
int ProgMain( char *output_file, char **in_files )
{
	int	ret, n_files ;

	n_files = 2 ;
	printf("output file: %s\n", output_file );
	switch ( conv_mode ){
	case 0:
		ret = Conv( output_file, in_files[0], in_files[1] );
		break ;
	case 1:
		ret = Conv2( output_file, in_files[0] );
		break ;
	}
	if ( ret ) return ( ret );
	return ( 0 );
}
