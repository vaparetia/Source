/*
	main.c
	

	2002/06/04 K.Takabe
	$Id: main.c,v 1.1 2002/06/05 05:23:18 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

enum {
	OPT_CVD		= 0x0001,
	OPT_TRICODE	= 0x0002,
	OPT_EVM		= 0x0004,
	OPT_EVM2	= 0x0004,
	OPT_VERUP	= 0x0010,
	OPT_VERUP2	= 0x0020,
	OPT_OPTKMS2	= 0x0040,
};

typedef struct _command_options {
	int			flag ;
	char		*out_path ;
} COMMAND_OPTIONS ;

/* ---------------------------------------------------------------- */
extern int Convert_KmxToKmy( char *input_file, char *output_file );
int ProgMain( int n_files, char **in_files, COMMAND_OPTIONS *option );

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256], *out_path = NULL ;
	int		mode = -1 ;
	COMMAND_OPTIONS	option ;

	memset( &option, 0, sizeof(COMMAND_OPTIONS) );
	option.flag = 0 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'd' ){
				option.out_path = argv[1] ;
				argv++ ;
				argc-- ;
			}
			if ( argv[0][1] == 'k' ){
				option.flag = 1 ;
			}
			if ( argv[0][1] == 'v' ){
				//option.flag |= OPT_VERUP2 ;
			}
			if ( argv[0][1] == 'o' ){
				//option.flag |= OPT_OPTKMS2 ;
			}
			if ( argv[0][1] == 't' ){
				//mode = 0 ;
			}
			if ( argv[0][1] == 'e' ){
				//option.flag |= OPT_EVM ;
				//mode = -1 ;
			}
			if ( argv[0][1] == 'm' ){
				//mode = 1 ;
			}
			if ( argv[0][1] == 'n' ){
				//mode = 2 ;
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
		fprintf( stderr, "Kmx 2 kmy Converter  Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: kms2kmy [options] <inputfile(*.kms)> ... \n" );
		fprintf( stderr, "(Options)  -d <directry/>       ... output directry \n" );
		fprintf( stderr, "           -k                   ... kmy dump mode \n" );
		//fprintf( stderr, "           -u                ... kms to kms2 vertion up \n" );
		//fprintf( stderr, "           -e                ... convert kms to evm file\n" );
		//fprintf( stderr, "           -o                ... optimize strip for kms2 file\n" );
		//fprintf( stderr, "           -m <kms-filename> ... set multi texture 1 kms (need -e option)\n" );
		//fprintf( stderr, "           -n <kms-filename> ... set multi texture 2 kms (need -e option)\n" );
		//fprintf( stderr, "           -v                ... kms2 to kms2a vertion up \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return ProgMain( get_input_files, input_files, &option );
}

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, COMMAND_OPTIONS *option )
{
	int		ret ;
	char	out_filename[ 1024 ];
	for ( ; n_files > 0 ; n_files--, in_files++ ){
		if ( option->flag == 0 ){
			if ( option->out_path == NULL ){
				MakeFileName( out_filename, *in_files, *in_files, ".kmy" );
			} else {
				MakeFileName( out_filename, option->out_path, *in_files, ".kmy" );
			}
			fprintf( stdout, "convert...(%s,%s)\n", *in_files, out_filename );
			ret = Convert_KmxToKmy( *in_files, out_filename );
		} else {
			DumpKmy( *in_files );
		}
	}
	return ( 0 );
}




