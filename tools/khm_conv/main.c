/*
	main.c
	

	2002/10/29 K.Takabe
	$Id: main.c,v 1.1 2002/11/18 07:22:48 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

enum {
	OPT_DUMP	= 0x0001,
	OPT_TRICODE	= 0x0002,
	OPT_CONVERT = 0x8000,
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
extern int Convert_KmsToKms2( char *in_files );
extern int Convert_Kms2ToKms2a( char *input_filename );
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

	option.flag |= OPT_CONVERT ;/* debug */

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'd' ){
				option.flag |= OPT_DUMP ;
			}
			if ( argv[0][1] == 't' ){
				mode = 0 ;
			}
		} else {
			switch ( mode ){
			  case 0:
				option.flag |= OPT_TRICODE ;
				strcpy( option.tri_file, argv[0] );
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
		fprintf( stderr, "Khm converter  Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: khm_couv [options] <inputfile(*.kms)> ... \n" );
		fprintf( stderr, "(Options)  -d                ... khm dump (debug mode) \n" );
		fprintf( stderr, "           -t <tri-filename> ... set tri-filename id \n" );
//		fprintf( stderr, "           -u                ... kms to kms2 vertion up \n" );
//		fprintf( stderr, "           -e                ... convert kms to evm file\n" );
//		fprintf( stderr, "           -o                ... optimize strip for kms2 file\n" );
//		fprintf( stderr, "           -m <kms-filename> ... set multi texture 1 kms (need -e option)\n" );
//		fprintf( stderr, "           -n <kms-filename> ... set multi texture 2 kms (need -e option)\n" );
//		fprintf( stderr, "           -v                ... kms2 to kms2a vertion up \n" );
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
		if ( option->flag & OPT_DUMP ){
			ret = Dump_Khm( *in_files );
		} else {
			ret = Convert_Kms2_Khm( *in_files, "test.khm" );
		}
	}
	return ( 0 );
}




