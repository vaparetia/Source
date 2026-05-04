/*
	main.c
	

	1999/10/14 K.Takabe
	$Id: main.c,v 1.1 2001/06/17 09:33:25 usr02774 Exp $
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct _command_options {
	int			flag ;
} COMMAND_OPTIONS ;
enum {
	OPT_DEBUG = 0x0001,
};

/* ---------------------------------------------------------------- */
int ProgMain( char *output_file, char **in_files, int n_files, COMMAND_OPTIONS *option );
int InitArchiveData( void );
int AddKmsData( char *filename );
int WriteZmsData( char *outfilename );
int DebugZms( char *filename );
void SetDebugMode( int mode );

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[256], *output_file ;
	int		mode = -1 ;
	COMMAND_OPTIONS	option ;

	memset( &option, 0, sizeof(COMMAND_OPTIONS) );
	option.flag = 0 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'd' ){
				option.flag |= OPT_DEBUG ;
			}
		} else {
			switch ( mode ){
			  case -1:
				output_file =  argv[0] ;
				mode = 0 ;
				break ;
			  default:/* 入力ファイル取得 */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files < 1 || get_input_files > 255 ){
		fprintf( stderr, "Kms archiver  Ver. 0.01   By K.Takabe\n" );
		fprintf( stderr, "Usage: kms_achiver [options] <outputfile(.zms)> <inputfile(*.kms)> ... \n" );
		fprintf( stderr, "(Options)  -d                ... debug-mode \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return ProgMain( output_file, input_files, get_input_files, &option );
}

/* ---------------------------------------------------------------- */
int ProgMain( char *output_file, char **in_files, int n_files, COMMAND_OPTIONS *option )
{
	int		ret ;

	if ( option->flag & OPT_DEBUG ) SetDebugMode( 1 );

	InitArchiveData();
	for ( ; n_files > 0 ; n_files--, in_files++ ){
		fprintf( stdout, "convert...(%s)\n", *in_files );
		ret = AddKmsData( *in_files );
		if ( ret != 0 ) return ( ret );
	}
	ret = WriteZmsData( output_file );
	//DebugZms( output_file );
	
	return ( ret );
}




