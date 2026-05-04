/*
	main.c
	SEV to SAR(sequencer achives file) コンパータ

	1999/12/03 K.Takabe
	$Id: main.c,v 1.3 2000/06/29 01:49:02 usr02774 Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt_sar.h"

extern void MakeFileName( char *filepath, char *path, char *name, char *ext );
extern int StringEqual( char *str1, char *str2 );

extern int ProgMain( char *out_archive, int n_files, char **in_files, char *list_file, char *list_file2, int flag );

#define MAX_LISTS	(1024)
#define MAX_DATASIZE	(1024*1024)

int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[1024], *out_archive = NULL, *list_file = NULL, *list_file2 = NULL ;
	int		mode = -1, option_flag = 0 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'n' ){
				option_flag = 0 ;
			}
			if ( argv[0][1] == 'a' ){
				option_flag = 1 ;
			}
			if ( argv[0][1] == 'l' ){
				mode = 2 ;
			}
			if ( argv[0][1] == 'i' ){
				mode = 3 ;
			}
			if ( argv[0][1] == 'e' ){
				mode = 4 ;
			}
		} else {
			switch ( mode ){
			case 2:
				list_file = argv[0] ;
				mode = -1 ;
				break ;
			case 3:/* 入力ファイル取得 */
				if ( get_input_files < 1024 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			case 4:
				list_file2 = argv[0] ;
				mode = -1 ;
				break ;
			default:
				if ( out_archive == NULL ){
					out_archive = argv[0] ;
				}
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files > 1023 || out_archive == NULL || list_file == NULL ){
		fprintf( stderr, "SEV to SAR Ver. 0.01  By K.Takabe\n");
		fprintf( stderr, "Usage: sev_archiver [options] <arcives file> -l <se code file> [-i <in file(*.sev)>] \n");
		fprintf( stderr, "(Options)  -n  ... create new archives \n" );
		fprintf( stderr, "           -a  ... apend to archives \n" );
		fprintf( stderr, "           -e <se_define.h file>  ... extend se code file \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return  ProgMain( out_archive, get_input_files, input_files, list_file, list_file2, option_flag );
}

/* ---------------------------------------------------------------- */
int ProgMain( char *out_archive, int n_files, char **in_files, char *list_file, char *list_file2, int flag )
{
	char	ext[256], name[256] ;
	SAR_HEADER	sar_header ;
	SAR_LIST	*list ;
	char			*datas ;
	int				size ;

	memset( &sar_header, 0, sizeof(SAR_HEADER) );
	list = malloc( sizeof(SAR_LIST) * MAX_LISTS );
	memset( list, 0, sizeof(SAR_LIST) * MAX_LISTS );
	datas = malloc( sizeof(char) * MAX_DATASIZE );
	memset( datas, 0, sizeof(char) * MAX_DATASIZE );

	if ( list_file != NULL ) SAR_LoadSeCodeList( list_file );
	if ( list_file2 != NULL ) SAR_LoadSeCodeList2( list_file2 );

	MakeFileName( name, NULL, out_archive, NULL );
	/* open sar file */
	if ( flag == 0 ){
		/* new */
		sar_header.format_id = SAR_FORMATID_SEV ;
	} else {
		/* append */
		SAR_LoadSAR( out_archive, &sar_header, list, datas );
	}

	for ( ; n_files > 0 ; n_files--, in_files++ ){
		int		s ;
		printf("file:%s\n", *in_files );
		size = SAR_LoadSEV( *in_files, &datas[ sar_header.data_size ], &s );
		if ( size != 0 ){
			MakeFileName( name, NULL, *in_files, NULL );
			list[ sar_header.n_datas ].id = MGS_GetStrCode2( name );
			list[ sar_header.n_datas ].offset = sar_header.data_size ;
			list[ sar_header.n_datas ].size = size ;
			list[ sar_header.n_datas ].local_header_size = s ;
		} else {
			list[ sar_header.n_datas ].id = 0 ;
			list[ sar_header.n_datas ].offset = sar_header.data_size ;
			list[ sar_header.n_datas ].size = size ;
			list[ sar_header.n_datas ].local_header_size = 0 ;
		}
		sar_header.n_datas += 1 ;
		sar_header.data_size += size ;
	}

	SAR_SaveSAR( out_archive, &sar_header, list, datas );

	free( list );
	free( datas );

	return ( 0 );
}

