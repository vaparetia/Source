/*
	main.c
	インデックスマトリクス方式への変換及びスケルトンバウンディング付きフォーマットへの変換
	XBOX専用共有頂点型インデックス形式への変換

	1999/10/14 K.Takabe
	$Id: main.c,v 1.3 2002/06/07 07:00:42 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

enum {
	OPT_EVX		= 0x0001,
};

typedef struct _command_options {
	int			flag ;
	char		tri_file[512] ;
	char		*ovl_tex1 ;
	char		*ovl_tex2 ;
} COMMAND_OPTIONS ;

/* ---------------------------------------------------------------- */
extern int Convert_EvmVersionUp( char *input_filename );
int ProgMain( int n_files, char **in_files, COMMAND_OPTIONS *option );

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256] ;
	int		mode = -1 ;
	COMMAND_OPTIONS	option ;

	memset( &option, 0, sizeof(COMMAND_OPTIONS) );
	option.flag = 0 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'x' ){
				option.flag |= OPT_EVX ;
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
		fprintf( stderr, "Evm converter  Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: evm_conv [options] <inputfile(*.evm)> ... \n" );
		fprintf( stderr, "(Options)  -x                ... convert xbox evx2 \n" );
#if 0
		fprintf( stderr, "           -t <tri-filename> ... set tri-filename id \n" );
		fprintf( stderr, "           -u                ... kms to kms2 vertion up \n" );
		fprintf( stderr, "           -e                ... convert kms to evm file\n" );
		fprintf( stderr, "           -o                ... optimize strip for kms2 file\n" );
		fprintf( stderr, "           -m <kms-filename> ... set multi texture 1 kms (need -e option)\n" );
		fprintf( stderr, "           -n <kms-filename> ... set multi texture 2 kms (need -e option)\n" );
		fprintf( stderr, "           -v                ... kms2 to kms2a vertion up \n" );
#endif
		fprintf( stderr, "\n" );
		return (-1);
	}
	return ProgMain( get_input_files, input_files, &option );
}

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, COMMAND_OPTIONS *option )
{
	int		ret ;
	for ( ; n_files > 0 ; n_files--, in_files++ ){
		fprintf( stdout, "convert...(%s)\n", *in_files );
		if ( option->flag & OPT_EVX ){
			/* XBOX専用変換 */
			printf("xbox evx2 create...\n");
			ret |= Convert_EvmVersionUp2( *in_files );
		} else {
			ret |= Convert_EvmVersionUp( *in_files );
		}
	}
	return ( 0 );
}




