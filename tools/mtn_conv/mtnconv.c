/*
	mtnconv.c
	無圧縮モーション（.mt1 or .mtn）＞圧縮モーション（.mt3）変換プログラム

	1999/10/14 K.Takabe
	$Id: mtnconv.c,v 1.7 2001/05/30 11:34:05 usr02774 Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gte.h"
#include "quat.h"
#include "motion.h"

#if 1
#define DIR_SEPARATE '/'
/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, char *out_path, int flag );

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
	char	*input_files[2048], *out_path = NULL ;
	int		mode = -1, option_flag = 0 ;

	/* オプションの解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			if ( argv[0][1] == 'd' ){
				mode = 1 ;
			}
			if ( argv[0][1] == 'i' ){
				mode = -1 ;
			}
			if ( argv[0][1] == 'n' ){
				option_flag = 1 ;
			}
		} else {
			switch ( mode ){
			case 1:
				out_path = argv[0] ;
				break ;
			default:/* 入力ファイル取得 */
				if ( get_input_files < 2048 ) input_files[get_input_files] = argv[0] ;
				get_input_files += 1 ;
				break ;
			}
		}
	}
	/* エラーチェック */
	if ( get_input_files < 1 || get_input_files >= 2048 ){
		fprintf( stderr, "Motion Converter Ver. 0.03  By K.Takabe\n");
		fprintf( stderr, "Usage: mtn_conv -i <in file(*.mt1,*.mtn)> [-d <out path>]\n");
		fprintf( stderr, "(Options)  -n   ... convert mt1 > mtn \n" );
		fprintf( stderr, "\n" );
		return (-1);
	}
	return  ProgMain( get_input_files, input_files, out_path, option_flag );
}

/* ---------------------------------------------------------------- */
int ProgMain( int n_files, char **in_files, char *out_path, int flag )
{
	char	ext[256], out_file[256] ;
	MTN_MOTION	*Motion, *WorkMotion, *EditMotion ;
	int			i, length ;

	for ( ; n_files > 0 ; n_files--, in_files++ ){
		if ( flag == 0 ){
			fprintf( stdout, "convert...(%s)\n", *in_files );

			/* ファイル読み込み処理 */
			MakeFileName( ext, NULL, NULL, *in_files );
			if ( StringEqual( ".mt1", ext ) ){
				fprintf( stdout, "mt1 convert...\n");
				Motion = MT_LoadMt1( *in_files );
			} else if ( StringEqual( ".mtn", ext ) || StringEqual( ".rmt", ext ) ){
				Motion = MTN_OpenMTNFile( *in_files );
				fprintf( stdout, "mtn convert...(l:%d a:%d)\n",
						 Motion->header.error_length, Motion->header.error_angle );
			} else {
				fprintf( stdout, "file error !! (%s)\n", *in_files );
				return ( -1 );
			}
			if ( Motion == NULL ){
				fprintf( stdout, "data load error!!(%s)\n", *in_files );
				return (-1);
			}
			WorkMotion = MTN_DuplicateMotion( Motion );
			EditMotion = MTN_DuplicateMotion( Motion );

			/* モーション変換処理 */
			MTN_CopyMotion( WorkMotion, Motion );
			length = Motion->header.motion_length ;
			if ( WorkMotion->header.flags & MTN_FLAG_TYPE_01 ){
				/* 上半身・下半身分離処理 */
				FVECTOR		*p_rots, *c_rots, *rots ;
				p_rots = &Motion->rots[ ( length + 1 ) * 0 ] ;
				c_rots = &Motion->rots[ ( length + 1 ) * 1 ] ;
				rots = &WorkMotion->rots[ ( length + 1 ) * 1 ] ;
				for ( i = 0 ; i <= length ; i++ ){
					MT_QuatMul( &rots[i], &p_rots[i], &c_rots[i] );
				}
			}
			MTN_Mt3Encode( EditMotion, WorkMotion, 0, 3.14159265f*Motion->header.error_angle/2048.0f );

			if ( out_path == NULL ){
				MakeFileName( out_file, *in_files, *in_files, "mt3" );
			} else {
				MakeFileName( out_file, out_path, *in_files, "mt3" );
			}
			fprintf( stdout, "out mt3 file (%s)...\n", out_file );
			MT3_Mt3Save( EditMotion, out_file, 0 );

			if ( Motion != NULL ) MTN_FreeMotion( Motion );
			if ( WorkMotion != NULL ) MTN_FreeMotion( WorkMotion );
			if ( EditMotion != NULL ) MTN_FreeMotion( EditMotion );
		} else if ( flag == 1 ){
			fprintf( stdout, "convert mt1 > mtn ...(%s)\n", *in_files );

			/* ファイル読み込み処理 */
			MakeFileName( ext, NULL, NULL, *in_files );
			if ( StringEqual( ".mt1", ext ) ){
				fprintf( stdout, "mt1 convert...\n");
				Motion = MT_LoadMt1( *in_files );
			} else {
				fprintf( stdout, "file error !! (%s)\n", *in_files );
				return ( -1 );
			}
			if ( Motion == NULL ){
				fprintf( stdout, "data load error!!(%s)\n", *in_files );
				return (-1);
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
#else
int main(int argc, char* argv[])
{
	if ( argc < 2 ){
		printf("Motion Converter Ver. 0.01  By K.Takabe\n");
		printf("Usage: mtnconv.exe <in file(*.mt1,*.mtn)> <out path>\n");
		return (0);
	}

	AnsiString	InFile, OutFile, Path, Ext ;

	while ( argv[1][0] == '-' ){
		argv++ ;
		argc-- ;
		if ( argc < 2 ) return (-1);
	}
	InFile = AnsiString( argv[1] ) ;

	if ( argc < 3 ){
		Path = AnsiString( argv[1] );
		Path.Delete( Path.LastDelimiter( "\\:" ), Path.Length() );
	} else {
		while ( argv[2][0] == '-' ){
			argv++ ;
			argc-- ;
			if ( argc < 2 ) return (-1);
		}
		Path = AnsiString( argv[2] );
		if ( Path.Length() == Path.LastDelimiter( "\\" ) ){
			Path.Delete( Path.Length(), 1 );
		}
	}
	Ext = InFile ;
	Ext.Delete( 1, Ext.LastDelimiter( "." ) );
	Ext = Ext.UpperCase();
	OutFile = InFile ;
	OutFile.Delete( OutFile.LastDelimiter( "." ), OutFile.Length() );
	OutFile = OutFile + AnsiString(".mt3");
	//printf("in file:%s\n", InFile.c_str());
	//printf("out path:%s\n", Path.c_str());
	//printf("out file:%s\n", OutFile.c_str());
	//printf("Ext:%s\n", Ext.c_str());
	//return 0;

	{// モーション変換
		MTN_MOTION	*Motion, *WorkMotion, *EditMotion ;
		int			i, length ;

		if ( Ext == AnsiString("MT1") ){
			Motion = MT_LoadMt1( InFile.c_str() );
		} else {
			Motion = MTN_OpenMTNFile( InFile.c_str() );
		}
		WorkMotion = MTN_DuplicateMotion( Motion );
		EditMotion = MTN_DuplicateMotion( Motion );


		MTN_CopyMotion( WorkMotion, Motion );
		length = Motion->header.motion_length ;
		if ( WorkMotion->header.flags & MTN_FLAG_TYPE_01 ){
			/* 上半身・下半身分離処理 */
			FVECTOR		*p_rots, *c_rots, *rots ;
			p_rots = &Motion->rots[ ( length + 1 ) * 0 ] ;
			c_rots = &Motion->rots[ ( length + 1 ) * 1 ] ;
			rots = &WorkMotion->rots[ ( length + 1 ) * 1 ] ;
			for ( i = 0 ; i <= length ; i++ ){
				MT_QuatMul( &rots[i], &p_rots[i], &c_rots[i] );
			}
		}
		MTN_Mt3Encode( EditMotion, WorkMotion, 0, 3.14159265f*Motion->header.error_angle/2048.0f );

		MT3_Mt3Save( EditMotion, OutFile.c_str(), 0 );

		if ( Motion != NULL ) MTN_FreeMotion( Motion );
		if ( WorkMotion != NULL ) MTN_FreeMotion( WorkMotion );
		if ( EditMotion != NULL ) MTN_FreeMotion( EditMotion );

	}

	return 0;
}
#endif
