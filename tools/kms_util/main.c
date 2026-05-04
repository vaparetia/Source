/*
	main.c
	

	1999/10/14 K.Takabe
	$Id: main.c,v 1.8 2000/08/01 05:39:17 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

enum {
	OPT_CVD		= 0x0001,
	OPT_TRICODE	= 0x0002,
	OPT_EVM		= 0x0004,
	OPT_EVM2	= 0x0004,
};

typedef struct _command_options {
	int			flag ;
	char		tri_file[512] ;
	char		*ovl_tex1 ;
	char		*ovl_tex2 ;
} COMMAND_OPTIONS ;

/* ---------------------------------------------------------------- */
extern int Convert_Kms_To_CommonVertexTabelFile( char *input_filename );
extern int Convert_Kms_To_MultiWeightEnvelopeModel( char *input_filename, char *input_filename2, char *input_filename3 );
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
			if ( argv[0][1] == 'c' ){
				option.flag |= OPT_CVD ;
			}
			if ( argv[0][1] == 't' ){
				mode = 0 ;
			}
			if ( argv[0][1] == 'e' ){
				option.flag |= OPT_EVM ;
				mode = -1 ;
			}
			if ( argv[0][1] == 'm' ){
				mode = 1 ;
			}
			if ( argv[0][1] == 'n' ){
				mode = 2 ;
			}
		} else {
			switch ( mode ){
			  case 0:
				option.flag |= OPT_TRICODE ;
				strcpy( option.tri_file, argv[0] );
				mode = -1 ;
				break ;
			  case 1:
				option.ovl_tex1 =  argv[0] ;
				mode = -1 ;
				break ;
			  case 2:
				option.ovl_tex2 =  argv[0] ;
				mode = -1 ;
				break ;
			  default:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files < 1 || get_input_files > 255 || option.flag == 0 ){
		fprintf( stderr, "Kms Util  Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: kms_util [options] <inputfile(*.kms)> ... \n" );
		fprintf( stderr, "(Options)  -c                ... create cvd file \n" );
		fprintf( stderr, "           -t <tri-filename> ... set tri-filename id \n" );
		fprintf( stderr, "           -e	               ... convert evm file\n" );
		fprintf( stderr, "           -m <kms-filename> ... set multi texture 1 \n" );
		fprintf( stderr, "           -n <kms-filename> ... set multi texture 2 \n" );
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
		if ( option->flag & OPT_CVD ){
			fprintf( stdout, "create cvd file...\n", *in_files );
			ret = Convert_Kms_To_CommonVertexTabelFile( *in_files );
		} else if ( option->flag & OPT_TRICODE ){
			ret = Convert_SetTRICODE( *in_files, option->tri_file );
		} else if ( option->flag & OPT_EVM ){
			ret = Convert_Kms_To_MultiWeightEnvelopeModel( *in_files, option->ovl_tex1, option->ovl_tex2 );
		} else {
			return ( -1 );
		}
	}
	return ( 0 );
}




