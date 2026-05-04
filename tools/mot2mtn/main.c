/*
	Mot file to Mtn file convert program

	$Id: main.c,v 1.2 2000/03/28 00:26:50 usr02774 Exp $

*/

#include <stdio.h>
#include <stdlib.h>


extern void MakeFileName( char *filepath, char *path, char *name, char *ext );
extern int StringEqual( char *str1, char *str2 );

typedef struct _options{
	int		flag ;
	char	*mdl_file ;
} OPTIONS ;

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, char *out_path, OPTIONS *option );

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[1024], *out_path = NULL, *model_file = NULL ;
	int		mode = -1 ;
	OPTIONS	option = {
		0, NULL
	};

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'd' ){
				mode = 1 ;
			}
			if ( argv[0][1] == 'm' ){
				mode = 2 ;
			}
			if ( argv[0][1] == 'i' ){
				mode = -1 ;
			}
		} else {
			switch ( mode ){
			case 1:
				/* Input -d option param */
				if ( out_path == NULL ) out_path = argv[0] ;
				break ;
			case 2:
				/* Input -m option param */
				if ( model_file == NULL ) model_file = argv[0] ;
				break ;
			default:/* 入力ファイル取得 */
				/* Input -i option param */
				if ( get_input_files < 256 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files < 1 || get_input_files > 255 || model_file == NULL ){
		fprintf( stderr, "Mot to Mtn convert program Ver. 0.01  By K.Takabe\n");
		fprintf( stderr, "Usage: mot2mtn -m <mdl file> -i <in file(*.mot)> [-d <out path>]\n");
		fprintf( stderr, "(Options)  -f   ... face motion convert \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	option.mdl_file = model_file ;
	return  ProgMain( get_input_files, input_files, out_path, &option );
}

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, char *out_path, OPTIONS *option )
{
	char	ext[256], out_file[256] ;

	for ( ; n_files > 0 ; n_files--, in_files++ ){
		if ( option->flag == 0 ){
			fprintf( stdout, "convert...(%s)\n", *in_files );
			MakeFileName( out_file, out_path, *in_files, "mtn" );
			if ( ConvertMotionFile2( out_file, *in_files, option->mdl_file ) == 255 ){
				fprintf( stdout, "ok!\n" );
			}
		}
	}
	return ( 0 );
}
