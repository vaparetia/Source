/*
   main.c : mdl2kms起動モジュール for linux

   by M.Sonoyama 1999.Sep.～ 

   $Id: main.c,v 1.15 1999/12/03 01:44:12 usr02011 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#define STRICT

#include <stdlib.h>
#include <string.h>
#include <SFDLINUX.h>
#include <METALGEAR.h>

#include "MDU_util.h"
#include "MDU_mdl.h"      
#include "MDU_tex.h"      

extern	int	MDU_M2K_N_ObjFlags ;
extern	char	MDU_M2K_ObjNames[][ 256 ] ;
extern	int	MDU_M2K_ObjFlags[] ;

static	void	Usage( void )
{
    printf( "mdl2kms : .mdl -> .kms converter for linux\n" ) ;
    printf( "\t Usage : mdl2kms inputfile(*.mdl) [outputfile(*.kms)]\n" ) ;
    printf( "\t\t\t[-d x_div y_div z_div] [-s scale] [-k] [-x] [-fast] [-b]\n" ) ;
    printf( "\t\t\t[-l save_dir] [-r rpt_file(*.rpt)]\n" ) ;
    printf( "\t\t -d : divide mdl( x_div y_div z_div )\n" ) ;
    printf( "\t\t -s : scaling_value( int )\n" ) ;
    printf( "\t\t -k : skelton model mode\n" ) ;
    printf( "\t\t -x : save .km3\n" ) ;
    printf( "\t\t -f : divide skip\n" ) ;
    printf( "\t\t -l : save directory\n" ) ;
    printf( "\t\t -b : no bitmap\n" ) ;
    printf( "\t\t -r : setting by rpt_ file\n" ) ;
    exit( -1 ) ;
}

int		main( argc, argv )
int		argc ;
char		**argv ;
{
    P3DXYZ    	xyzScale ;
    HP3DMODEL 	hP3DModel ;
    KM3_DEF2	*def2 ;
    KMS_DEF	*sdef ;
    SVECTOR	div ;
    char	in[ 128 ], out[ 256 ], out2[ 256 ], *outp ;
    char	dir[ 256 ], full[ 256 ] ;
    int		skel ;

    if ( argc < 2 ) Usage() ;

    /* 初期設定 */
    xyzScale.x = 10.0F ;
    xyzScale.y = 10.0F ;
    xyzScale.z = 10.0F ;
    skel = 0 ;
    div.vx = div.vy = div.vz = 0 ;
    MDU_M2K_N_ObjFlags = 0 ;

    MDU_ParseOption( argc, argv ) ;
#if 0
    def2 = MDU_LoadKm3( argv[ 1 ] ) ;
/*
    div.vx = 6000 ;
    div.vy = 6000 ;
    div.vz = 6000 ;

    def2 = MDU_DivideKm3( def2, &div ) ;
*/
    sdef = MDU_Km32Kms( def2 ) ;    
    exit( 0 ) ;
#endif
    strcpy( in, argv[ 1 ] ) ;
    if ( argc == 2 || argv[ 2 ][ 0 ] == '-' ) strcpy( out, in ) ;
    else 	     strcpy( out, argv[ 2 ] ) ;
    if ( MDU_GetOption( 'l' ) != NULL ) {
	strcpy( dir, MDU_GetNextValue() ) ;
	strcat( dir, "/" ) ;
    } else {
	strcpy( dir, "./" ) ;
    }
    if ( strcmp( MDU_GetExtension( in ), "mdl" ) ) Usage() ;
    if ( strcmp( MDU_GetExtension( out ), "kms" ) ) {
	outp = MDU_TailName( out ) ;
	strcpy( out, outp ) ;
	MDU_ChangeExtension( out, "kms" ) ;
    }

    /* rptファイルから設定 */
    if ( MDU_GetOption( 'r' ) != NULL ) {
	FILE		*fp ;
	char		*rptfile, *ptr ;
	u_char		buff[ 256 ], elm[ 8 ][ 64 ] ;
	int		n_elms, i ;

	rptfile = MDU_GetNextValue() ;
	fp = fopen( rptfile, "r" ) ;
	if ( fp == NULL ) goto rpt_error_skip ;
	while( !feof( fp ) ) {
	    if ( fgets( buff, 256, fp ) == NULL ) break ;
	    n_elms = 0 ;
	    ptr = buff ;
	    while( 1 ) {
		if ( ptr[ 0 ] == '\n' || ptr[ 0 ] == '\0' ||
		     n_elms == 8 ) break ;
		for ( i = 0; *ptr != ' ' && *ptr != '\t' &&
		             *ptr != '\n'; i ++ ) {
		    elm[ n_elms ][ i ] = *ptr ;
		    ptr ++ ;
		}
		if ( i != 0 ) {
		    elm[ n_elms ][ i ] = '\0' ;
		    printf( "%s ", elm[ n_elms ] ) ;
		    n_elms ++ ;
		} else {
		    ptr ++ ;
		}
	    }
	    printf( "\n" ) ;
	    if ( !strcmp( elm[ 0 ], "Scale" ) ) {
		xyzScale.x = atoi( elm[ 1 ] ) ;
		xyzScale.y = atoi( elm[ 1 ] ) ;
		xyzScale.z = atoi( elm[ 1 ] ) ;
	    } else if ( !strcmp( elm[ 0 ], "DIVIDEX" ) ) {
		div.vx = atoi( elm[ 1 ] ) ;
	    } else if ( !strcmp( elm[ 0 ], "DIVIDEY" ) ) {
		div.vy = atoi( elm[ 1 ] ) ;
	    } else if ( !strcmp( elm[ 0 ], "DIVIDEZ" ) ) {
		div.vz = atoi( elm[ 1 ] ) ;
	    } else if ( !strcmp( elm[ 0 ], "MDLTYPE" ) ) {
		if ( strcmp( elm[ 1 ], "MESH" ) ) skel = 1 ;
	    } else {
		/* モデル情報 */
		strcpy( &MDU_M2K_ObjNames[ MDU_M2K_N_ObjFlags ], elm[ 0 ] ) ;
		MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] = 0 ;
		for ( i = 1; i < n_elms; i ++ ) {
		    if ( !strcmp( elm[ i ], "TRANS" ) ) {
			MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_TRANS ;
		    } else if ( !strcmp( elm[ i ], "NOSHADE" ) ) {
			MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_NOSHADE ;
		    } else if ( strstr( elm[ i ], "OVERLAY0" ) != NULL ) {
			MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_OVERLAY0 ;
		    } else if ( strstr( elm[ i ], "OVERLAY1" ) != NULL ) {
			MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_OVERLAY1 ;
		    } else if ( strstr( elm[ i ], "OVERLAY2" ) != NULL ) {
			MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_OVERLAY2 ;
		    }
		}
		MDU_M2K_N_ObjFlags ++ ;
	    }
	}
	fclose( fp ) ;
    }

    {
	int		i ;

	for ( i = 0; i < MDU_M2K_N_ObjFlags; i ++ ) {
	    printf( "%x ", MDU_M2K_ObjFlags[ i ] ) ;
	}
	printf( "\n" ) ;
    }
rpt_error_skip :
    /* スケール */
    if ( MDU_GetOption( 's' ) != NULL ) {
	xyzScale.x = ( float )MDU_GetNextInt() ;
	xyzScale.y = xyzScale.x ;
	xyzScale.z = xyzScale.x ;
    }
    /* スケルトンモデル？ */
    if ( MDU_GetOption( 'k' ) != NULL ) skel = 1 ;
    /* 分割 */
    if ( MDU_GetOption( 'd' ) != NULL ) {
	div.vx = MDU_GetNextInt() ;
	div.vy = MDU_GetNextInt() ;
	div.vz = MDU_GetNextInt() ;
    }

    if( ( hP3DModel = P3DReadFileModel( in, NULL, 0 ) ) == NULL ) return -1 ;
    printf( "Read P3DModel OK\n" ) ;
    if ( ( def2 = MDU_Mdl2Km3( out, &xyzScale, hP3DModel, skel ) ) == NULL ) {
	P3DDestroyModel( hP3DModel ) ;
	return -1 ;
    }

    /* テクスチャ抽出 */
    if ( MDU_GetOption( 'b' ) == NULL ) {
	MDU_ExtractTextureFromMdl( hP3DModel ) ;
	MDU_SaveExtractTextures( dir ) ;
	P3DDestroyModel( hP3DModel ) ;
    }

    /* スケルトンモデルなら、半透明属性分割 */
    if ( skel ) def2 = MDU_SplitKm3( def2 ) ;

    /* 分割設定してあれば分割 */
    if ( MDU_GetOption( 'f' ) == NULL ) {
	if ( div.vx >= 1000 && div.vy >= 1000 && div.vz >= 1000 ) {
	    def2 = MDU_DivideKm3( def2, &div ) ;
	}
    }

    /* km3セーブ */
    if ( MDU_GetOption( 'x' ) != NULL ) {
	strcpy( out2, out ) ;
	MDU_ChangeExtension( out2, "km3" ) ;
//	MDU_DumpKm3( def2 ) ;
	strcpy( full, dir ) ;
	strcat( full, out2 ) ;
	printf( "save %s\n", full ) ;
	MDU_SaveKm3( full, def2 ) ;
	/* km3セーブの場合ここで終わり */
	exit( 0 ) ;
    }

    sdef = MDU_Km32Kms( def2 ) ;
    strcpy( full, dir ) ;
    strcat( full, out ) ;
    printf( "save %s\n", full ) ;
    MDU_SaveKms( full, sdef ) ;

    return 0 ;
}
