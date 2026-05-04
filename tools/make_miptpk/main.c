/*
	main.c
	Xbox Texture-Image File create program

	2002/04/10 K.Takabe
	$Id: main.c,v 1.2 2002/06/19 08:40:01 usr02774 Exp $

	指定されたミップマップ用のtexpackファイル(*.tpk)から
	ミップマップ用xtiを生成する
	ここで指定するtexpackは非ミップマップのもので大きさが
	ミップマップ用にすべて揃っている必要がある
	（256x256,128x128,64x64,32x32）
	通常のミップマップとの違いは各レベル毎に依存性のない
	画像を適用することができることである
 */
/*
*/
#include <stdio.h>
#include <stdlib.h>

#include "loadinc.h"

int ProgMain( char *out_name, int n_files, char **in_files, int options );

int
main( int argc, char **argv )
{
	int		get_output_file = 0, get_input_files = 0 ;
	char	*output_file, *input_files[256] ;
	int		mode = -1, option_flag = 0 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'o' ){
				mode = 0 ;
			} else if ( argv[0][1] == 'i' ){
				mode = 1 ;
			} else if ( argv[0][1] == 't' ){
				option_flag = TRI_FLAG_TRANS ;
			} else if ( argv[0][1] == 's' ){
				option_flag = TRI_FLAG_STATIC ;
			} else if ( argv[0][1] == 'l' ){
				option_flag = TRI_FLAG_LATTERDRAW ;
			} else if ( argv[0][1] == 'c' ){
				option_flag = TRI_FLAG_COMPRESS ;
			}
		} else {
			switch ( mode ){
			  case 0:/* 出力ファイル取得 */
				output_file = argv[0] ;
				get_output_file += 1 ;
				break ;
			  case 1:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			  default:
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_output_file != 1 || get_input_files < 1 || get_input_files > 255 ){
		fprintf( stderr, "Make mipmap xti program Ver.0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: make_miptpk [options] -o <outputfile(*.tpk)> -i <inputfile(*.tpk)> ... \n" );
		//fprintf( stderr, "(Options)  -t ... transparent texture mode \n" );
		//fprintf( stderr, "           -s ... static palette texture mode \n" );
		//fprintf( stderr, "           -l ... latter draw flag on \n" );
		/*fprintf( stderr, "           -c ... data compression \n" );*/
		fprintf( stderr, "\n" );
		return (-1);
	}
	return ProgMain( output_file, get_input_files, input_files, option_flag );
}

int ProgMain( char *out_name, int n_files, char **in_files, int options )
{
	int	ret = 0 ;

	for ( ; n_files > 0 ; n_files--, in_files++ ){
		fprintf( stdout, "now loading...(%s)\n", *in_files );
		if ( LoadTpkFile( *in_files ) == -1 ){
			ret = -1 ;
		}
	}
	fprintf( stdout, "write file to <%s> \n", out_name );
	WriteXboxTextureImageFile( out_name, options );
	return ret ;
}


