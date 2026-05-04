/*
	main.c
	無圧縮モーション（.mt1 or .mtn）＞圧縮モーション（.mt3）変換プログラム

	1999/11/16 K.Takabe
	$Id: main.c,v 1.1 1999/11/16 08:30:31 usr02774 Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gte.h"
#include "quat.h"
#include "motion.h"

#define DIR_SEPARATE '/'
/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, char *out_path, int flag, float angle );

static void MakeFileName( char *filepath, char *path, char *name, char *ext )
{
	char	*tmp_ptr, *ptr ;

	if ( path != NULL ){
		tmp_ptr = path ;
		ptr = path ;
		while ( *ptr != '\0' ){
			if ( *ptr == DIR_SEPARATE ){
				tmp_ptr = ptr + 1 ;
			}
			ptr++ ;
		}
		for ( ptr = path ; ptr != tmp_ptr ; ) *filepath++ = *ptr++ ;
	}

	if ( name != NULL ){
		tmp_ptr = name ;
		ptr = name ;
		while ( *ptr != '\0' ){
			if ( *ptr == DIR_SEPARATE ){
				tmp_ptr = ptr + 1 ;
			}
			ptr++ ;
		}
		for ( ptr = tmp_ptr ; ( *ptr != '\0' ) && ( *ptr != '.') ; ) *filepath++ = *ptr++ ;
	}

	if ( ext != NULL ){
		*filepath++ = '.' ;
		tmp_ptr = ext ;
		ptr = ext ;
		while ( *ptr != '\0' ){
			if ( *ptr == '.' ){
				tmp_ptr = ptr + 1 ;
			}
			ptr++ ;
		}
		for ( ptr = tmp_ptr ; *ptr != '\0' ; ) *filepath++ = *ptr++ ;
	}

	*filepath = '\0' ;
}

int StringEqual( char *str1, char *str2 )
{
	char	c1, c2 ;
	while ( *str1 != '\0' && *str2 != '\0' ){
		c1 = *str1++ ;
		c2 = *str2++ ;
		if ( c1 > 'a' && c1 < 'z' ) c1 = c1 - ( 'a' - 'A' );
		if ( c2 > 'a' && c2 < 'z' ) c2 = c2 - ( 'a' - 'A' );
		if ( c1 != c2 ) return (0);
	}
	return (1);
}

/* ---------------------------------------------------------------- */
int
main( int argc, char **argv )
{
	int		get_input_files = 0 ;
	char	*input_files[1024], *out_path = NULL ;
	int		mode = -1, option_flag = 0 ;
	float	angle = 0.0f ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'd' ){
				mode = 1 ;
			}
			if ( argv[0][1] == 'i' ){
				mode = -1 ;
			}
			if ( argv[0][1] == 'r' ){
				angle = atof( &argv[0][2] );
				if ( angle == 0.0f ) mode = 2 ;
				else {
					angle = 3.14159265 * angle / 180.0f ;
					printf("correct angle : %f\n", angle );
				}
			}
		} else {
			switch ( mode ){
			case 1:
				/* Input -d option param */
				out_path = argv[0] ;
				break ;
			case 2:
				/* Input -r option param */
				//angle = atof( argv[0] );
				sscanf( argv[0], " %f", &angle );
				angle = 3.14159265 * angle / 180.0f ;
				printf("correct angle : %f\n", angle );
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
	if ( get_input_files < 1 || get_input_files > 255 ){
		fprintf( stderr, "MTN file data correct program Ver. 0.01  By K.Takabe\n");
		fprintf( stderr, "Usage: mtn_correct -i <in file(*.mt1,*.mtn)> [-d <out path>]\n");
		fprintf( stderr, "(Options)  -r   ... X rot correct angle ( joint of hands ) \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return  ProgMain( get_input_files, input_files, out_path, option_flag, angle );
}

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, char *out_path, int flag, float angle )
{
	char	ext[256], out_file[256] ;
	MTN_MOTION	*Motion ;
	int			i, length ;
	FVECTOR		correct_quat ;
	correct_quat.vx = 1.0 ;
	correct_quat.vy = 0.0 ;
	correct_quat.vz = 0.0 ;
	correct_quat.vw = angle ;
	MT_QuatSetValue( &correct_quat, &correct_quat );

	for ( ; n_files > 0 ; n_files--, in_files++ ){
		if ( flag == 0 ){
			fprintf( stdout, "convert...(%s)\n", *in_files );

			/* ファイル読み込み処理 */
			MakeFileName( ext, NULL, NULL, *in_files );
			if ( StringEqual( ".mt1", ext ) ){
				fprintf( stdout, "mt1 convert...\n");
				Motion = MT_LoadMt1( *in_files );
			} else if ( StringEqual( ".mtn", ext ) ){
				fprintf( stdout, "mtn convert...\n");
				Motion = MTN_OpenMTNFile( *in_files );
			} else {
				fprintf( stdout, "file error !! (%s)\n", *in_files );
				return ( -1 );
			}
			if ( Motion == NULL ){
				fprintf( stdout, "data load error!!(%s)\n", *in_files );
				return (-1);
			}
			/* モーション変換処理 */
			length = Motion->header.motion_length ;
			{
				FVECTOR		*rots ;
				rots = &Motion->rots[ ( length + 1 ) * 6 ] ;
				for ( i = 0 ; i <= length ; i++ ){
					MT_QuatMul( &rots[i], &rots[i], &correct_quat );
				}
				rots = &Motion->rots[ ( length + 1 ) * 10 ] ;
				for ( i = 0 ; i <= length ; i++ ){
					MT_QuatMul( &rots[i], &rots[i], &correct_quat );
				}
			}

			if ( out_path == NULL ){
				MakeFileName( out_file, *in_files, *in_files, "mtn" );
			} else {
				MakeFileName( out_file, out_path, *in_files, "mtn" );
			}
			fprintf( stdout, "out mtn file (%s)...\n", out_file );
			MTN_SaveMTNFile( out_file, Motion );

			if ( Motion != NULL ) MTN_FreeMotion( Motion );
		}
	}
	return ( 0 );
}
