/*
   main.c : mdl2kc2起動モジュール for linux

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/13/99

   $Id: main.c,v 1.22 2002/07/17 14:02:34 usr01363 Exp $

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

#include "Kmx.h"
#include "Evf.h"
#include "block.h"


extern	int	MDU_M2K_N_ObjFlags ;
extern	char	MDU_M2K_ObjNames[][ 256 ] ;
extern	int	MDU_M2K_ObjFlags[] ;


/* オプション等の管理用 */
static P3DXYZ xyzScale;
static SVECTOR div_param;
static int skeleton_flag;
static int fast_flag;
static int multitex_flag;
static int fix_flag;
static int multi_uv_flag;
static int multitex_check_flag;
static int overlap_flag;

static int etc_flag;		// その他フラグ

static char in[ 128 ], out[ 256 ], outcv2[ 256 ], *outp ;
static char dir[ 256 ], full[ 256 ] ;
static int cv2out_flag;
static int min_verts;

static	void	Usage( void )
{
    printf( "mdl2kc : .mdl -> .kms2, .cv2 or .evm converter for linux\n" ) ;
    printf( "\t Usage : mdl2kc inputfile(*.mdl) [outputfile(*.kms)] [outputfile(*.cv2)]\n" ) ;
    printf( "\t\t\t[-d x_div y_div z_div] [-s scale] [-k]\n" ) ;
    printf( "\t\t\t[-l save_dir] [-r rpt_file(*.rpt)] [-f] [-m texture_mode] [-c] [-e] [-u]\n" ) ;
    printf( "\t\t\t[-t vertexes] [-x] [-y]\n" ) ;
    printf( "\t\t -d : divide mdl( x_div y_div z_div )\n" ) ;
    printf( "\t\t -s : scaling_value( int )\n" ) ;
    printf( "\t\t -k : skelton model mode\n" ) ;
    printf( "\t\t -l : save directory\n" ) ;
    printf( "\t\t -r : setting by rpt_ file\n" ) ;
    printf( "\t\t -f : convert faster\n");
    printf( "\t\t -m : Select texture mode ( 0:Auto Select, 1:Single Texture, 2:Multi Texture )\n");
    printf( "\t\t -c : No check a number of textures on a polygon\n" );
    printf( "\t\t -e : convert large model\n");
    printf( "\t\t -u : using multi UV ( usually used only UV0 )\n");
    printf( "\t\t -t : divide model by texture ( vertexes: Minimum number of vertexes to divide )\n");
    printf( "\t\t -x : Output Xbox format data\n");
    printf( "\t\t -y : Output Xbox format data ( kmy, cvy )\n");
    exit( -1 ) ;
}

static void ReadRpt(char *rptfile)
{
    FILE	*fp ;
    char	*ptr ;
    u_char	buff[ 256 ], elm[ 8 ][ 64 ] ;
    int		n_elms, i ;

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
		}
		else if ( !strcmp( elm[ 0 ], "DIVIDEX" ) ) {
			div_param.vx = atoi( elm[ 1 ] ) ;
		}
		else if ( !strcmp( elm[ 0 ], "DIVIDEY" ) ) {
			div_param.vy = atoi( elm[ 1 ] ) ;
		}
		else if ( !strcmp( elm[ 0 ], "DIVIDEZ" ) ) {
			div_param.vz = atoi( elm[ 1 ] ) ;
		}
		else if ( !strcmp( elm[ 0 ], "MDLTYPE" ) ) {
			if ( strcmp( elm[ 1 ], "MESH" ) ) skeleton_flag = 1 ;
		}
		else if ( !strcmp( elm[ 0 ], "VANIM" ) ) {
			/* 隠し仕様  これがONだった場合は、頂点の重複チェックを止める */
			if ( !strcmp( elm[ 1 ], "ON" ) ) overlap_flag = 0 ;
		}
		else if ( !strcmp( elm[ 0 ], "TEXTUREFORMAT" ) ) {
			if ( !strcmp( elm[ 1 ], "SINGLE" ) ) multitex_flag = 0 ;
			if ( !strcmp( elm[ 1 ], "MULTI" ) ) multitex_flag = 2 ;
		}
		else {
			/* モデル情報 */
			strcpy( &MDU_M2K_ObjNames[ MDU_M2K_N_ObjFlags ], elm[ 0 ] ) ;
			MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] = 0 ;

			for ( i = 1; i < n_elms; i ++ ) {
				if ( !strcmp( elm[ i ], "TRANS" ) ) {
					MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_TRANS ;
				}
				else if ( !strcmp( elm[ i ], "NOSHADE" ) ) {
					MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_NOSHADE ;
				}
				else if ( strstr( elm[ i ], "OVERLAY0" ) != NULL ) {
					MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_OVERLAY0 ;
				}
				else if ( strstr( elm[ i ], "OVERLAY1" ) != NULL ) {
					MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_OVERLAY1 ;
				}
				else if ( strstr( elm[ i ], "OVERLAY2" ) != NULL ) {
					MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_OVERLAY2 ;
				}
				else if ( !strcmp( elm[ i ], "BACKCLIP_AUTO" ) ) {
					MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_AUTOSIDE ;
				}
				else if ( !strcmp( elm[ i ], "BACKCLIP" ) ) {
					MDU_M2K_ObjFlags[ MDU_M2K_N_ObjFlags ] |= DG_TYPE_SINGLESIDE ;
				}
			}
			MDU_M2K_N_ObjFlags ++ ;
		}
    }

    fclose( fp ) ;

    {
		int i ;

		for ( i = 0; i < MDU_M2K_N_ObjFlags; i ++ ) {
			printf( "%x ", MDU_M2K_ObjFlags[ i ] ) ;
		}
		printf( "\n" ) ;
    }

rpt_error_skip:
    ;
}

static int MakeKms(HP3DMODEL hP3DModel, char* pszExt )
{
    KMX_DEF	*def ;
    KMS2_DEF	*sdef ;
    CV2_OBJS	*cobjs ;

    printf("---- Start mdl2kms\n");

    if ( strcmp( MDU_GetExtension( out ), pszExt ) ) {
		outp = MDU_TailName( out ) ;
		strcpy( out, outp ) ;
		MDU_ChangeExtension( out, pszExt ) ;
    }

    if ( ( def = MDU_Mdl2Kmx( out, &xyzScale, hP3DModel, skeleton_flag ) ) == NULL ) {
		return -1 ;
    }

    /* スケルトンモデルなら、半透明属性分割 */
    if ( skeleton_flag ) def = MDU_SplitKmx( def ) ;

    if(def==NULL){
		printf("Failed : Split\n");
		return -1;
    }

    /* スケルトンモデルでは無く、分割設定してあれば分割 */
    if(!skeleton_flag){
		if ( div_param.vx >= 1 && div_param.vy >= 1 && div_param.vz >= 1 ) {
			def = MDU_DivideKmx( def, &div_param, min_verts ) ;
		}

#if 0
		else{
			/* 分割されてなければ、おそらくステージデータではない。
			   そのようなモデルでは、tx,ty,tzはセットされるべきでは無いので、
			   強制的に原点にセットする。*/
			def->tx=0.0f;
			def->ty=0.0f;
			def->tz=0.0f;
		}
#endif

    }

    if(def==NULL){
		printf("Failed : Divide\n");
		return -1;
    }

    /* 法線とエンベロープの関係をチェック */
    Kmx_CheckNormal(def);

    if(Kmx_CheckTexture(def,multitex_check_flag)) return -1;

	if( etc_flag & MDU_FLAG_YFORMAT ) {
		if(!MDU_Kmx2Kmsx(def,&sdef,&cobjs,fast_flag,multitex_flag,overlap_flag,
						 etc_flag,multi_uv_flag )){
			Kmx_Free(def);
			printf("Failed to convert Kmx to Kms2.\n");
			return -1;
		}
	}
	else{
		if(!MDU_Kmx2Kms2(def,&sdef,&cobjs,fast_flag,multitex_flag,overlap_flag, etc_flag )){
			Kmx_Free(def);
			printf("Failed to convert Kmx to Kms2.\n");
			return -1;
		}
	}

    strcpy( full, dir ) ;
    strcat( full, out ) ;
    printf( "Save %s  ... ", full ) ;

    {
		int ans;

		if(MDU_SaveKms2(full,sdef,multi_uv_flag)){
			printf("Successed.\n");
			ans=0;
		}
		else{
			printf("Failed.\n");
			ans=-1;
		}
    
		if(cobjs!=NULL){
			if(cv2out_flag){
				strcpy( full, dir ) ;
				strcat( full, outcv2 ) ;
				printf( "save %s  ... ", full ) ;

#if 0
				if(MDU_SaveCv2(full,cobjs)) printf("Successed.\n");
				else printf("Failed.\n");
#else
				MDU_SaveCv2(full,cobjs);
				printf("Successed.\n");
#endif
			}
			Cv2_Free(cobjs);
		}

		Kmx_Free(def);
		Kms2_Free(sdef);

		return ans;
    }
}

static int MakeEvm(HP3DMODEL hP3DModel, char* pszExt )
{
    EVF_DEF	*def ;
    EVM_DEF	*sdef ;

    printf("---- Start mdl2evm\n");

    if ( strcmp( MDU_GetExtension( out ), pszExt ) ) {
		outp = MDU_TailName( out ) ;
		strcpy( out, outp ) ;
		MDU_ChangeExtension( out, pszExt ) ;
    }

    if ( ( def = MDU_Mdl2Evf( out, &xyzScale, hP3DModel ) ) == NULL ) {
		return -1 ;
    }

    if(Evf_CheckTexture(def,multitex_check_flag)) return -1;

    if((sdef=MDU_Evf2Evm(def,fast_flag,fix_flag, etc_flag ))==NULL){
		Evf_Free(def);
		printf("Failed to convert Evf to Evm.\n");
		return -1;
    }

    strcpy( full, dir ) ;
    strcat( full, out ) ;
    printf( "Save %s  ... ", full ) ;

    {
		int ans;

		if(MDU_SaveEvm(full,sdef,multi_uv_flag)){
			printf("Successed.\n");
			ans=0;
		}
		else{
			printf("Failed.\n");
			ans=-1;
		}

		Evf_Free(def);
		Evm_Free(sdef);

		return ans;
    }
}

int		main( argc, argv )
	int		argc ;
char		**argv ;
{
    HP3DMODEL 	hP3DModel ;
    int n_skel,n_mesh;


    if ( argc < 2 ) Usage() ;

    /* 初期設定 */
    xyzScale.x = 10.0F ;
    xyzScale.y = 10.0F ;
    xyzScale.z = 10.0F ;
    div_param.vx = div_param.vy = div_param.vz = 0 ;
    MDU_M2K_N_ObjFlags = 0 ;

    skeleton_flag = 0 ;
    fast_flag=0;
    multitex_flag=0;
    fix_flag=0;
    cv2out_flag=0;
    multi_uv_flag=0;
    multitex_check_flag=1;
    overlap_flag=1;

	etc_flag = 0;

    min_verts=0;

    /* オプションの読み込み */
    MDU_ParseOption( argc, argv ) ;

    strcpy( in, argv[ 1 ] ) ;
    if ( argc == 2 || argv[ 2 ][ 0 ] == '-' ) strcpy( out, in ) ;
    else{
		strcpy( out, argv[ 2 ] ) ;
		if ( argc >= 4 && argv[ 2 ][ 0 ] != '-' && argv[ 3 ][ 0 ] != '-' ){
			strcpy( outcv2, argv[ 3 ] ) ;
			cv2out_flag = 1;
		}
    }

    if ( MDU_GetOption( 'l' ) != NULL ) {
		strcpy( dir, MDU_GetNextValue() ) ;
		strcat( dir, "/" ) ;
    } else {
		strcpy( dir, "./" ) ;
    }

    if ( strcmp( MDU_GetExtension( in ), "mdl" ) ) Usage() ;

    /* rptファイルから設定の取り出し */
    if ( MDU_GetOption( 'r' ) != NULL ) {
		char *rptfile=MDU_GetNextValue();
		ReadRpt(rptfile);
    }

    /* スケール */
    if ( MDU_GetOption( 's' ) != NULL ) {
		xyzScale.x = ( float )MDU_GetNextInt() ;
		xyzScale.y = xyzScale.x ;
		xyzScale.z = xyzScale.x ;
    }
    /* スケルトンモデル？ */
    if ( MDU_GetOption( 'k' ) != NULL ) skeleton_flag = 1 ;
    /* 分割 */
    if ( MDU_GetOption( 'd' ) != NULL ) {
		div_param.vx = MDU_GetNextInt() ;
		div_param.vy = MDU_GetNextInt() ;
		div_param.vz = MDU_GetNextInt() ;
    }

    if ( MDU_GetOption( 'f' ) != NULL ) fast_flag = 1 ;

    if ( MDU_GetOption( 'c' ) != NULL ) multitex_check_flag = 0 ;

    if ( MDU_GetOption( 'e' ) != NULL ) fix_flag = 1 ;

    if ( MDU_GetOption( 'u' ) != NULL ) multi_uv_flag = 1 ;

    /* テクスチャモードの選択 */
    if ( MDU_GetOption( 'm' ) != NULL ){
		multitex_flag = MDU_GetNextInt() ;
		switch(multitex_flag){
		case 1:
			printf("Texture Mode : Selected Single Mode\n");
			break;
		case 2:
			printf("Texture Mode : Selected Multi Mode\n");
			break;
		default:
			printf("Error : Unknown Texture Mode\n");
		case 0:
			printf("Texture Mode : Selected Auto Mode\n");
			break;
		}
    }

    /* Distribute2 を使用; パケットの最大頂点数を取得 */
    if ( MDU_GetOption( 't' ) != NULL ) {
		min_verts = MDU_GetNextInt() ;
    }

	/* Xbox 拡張 */
	if( MDU_GetOption( 'x' ) != NULL ) {
		etc_flag |= MDU_FLAG_NO_LIMIT | MDU_FLAG_INDEX;
	}
	if( MDU_GetOption( 'y' ) != NULL ) {
		etc_flag |= MDU_FLAG_NO_LIMIT | MDU_FLAG_INDEX | MDU_FLAG_YFORMAT;
	}

    if( cv2out_flag ){
		if( etc_flag & MDU_FLAG_YFORMAT ) {
			if ( strcmp( MDU_GetExtension( outcv2 ), "cvy" ) ) {
				MDU_ChangeExtension( outcv2, "cvy" ) ;
			}
		}
		else if( etc_flag & MDU_FLAG_NO_LIMIT ) {
			if ( strcmp( MDU_GetExtension( outcv2 ), "cvx" ) ) {
				MDU_ChangeExtension( outcv2, "cvx" ) ;
			}
		}
		else {
			if ( strcmp( MDU_GetExtension( outcv2 ), "cv2" ) ) {
				MDU_ChangeExtension( outcv2, "cv2" ) ;
			}
		}
    }
    outp = MDU_TailName( out ) ;
    strcpy( out, outp ) ;
    outp = MDU_TailName( outcv2 ) ;
    strcpy( outcv2, outp ) ;

    /* 作業の開始 */
#if 1
    if( ( hP3DModel = P3DReadFileModel( in, NULL, 0 ) ) == NULL ) exit(-1);
#else
    if( ( hP3DModel = P3DReadFileUnpackModel( in, NULL, 0 ) ) == NULL ) exit(-1);
#endif

    printf( "Read P3DModel OK\n" ) ;

    n_skel=0;
    n_mesh=0;
    CountMesh( hP3DModel, NULL, &n_skel, &n_mesh );

    printf("n_skel n_mesh = %d %d\n",n_skel,n_mesh);

    {
		int ans=0;

#if 0
		if(n_skel==n_mesh){
			/* mdl2kc */
			ans=MakeKms(hP3DModel);
		}
		else if(n_mesh==1){
			/* mdl2evm */
			ans=MakeEvm(hP3DModel);
		}
		else{
			/* Error */
			printf("Error : Illegal Model\n");
			ans=-1;
		}
#else
		if(n_mesh==0){
			/* Error */
			printf("Error : Illegal Model\n");
			ans=-1;
		}
		else if(n_skel!=1 && n_mesh==1 && skeleton_flag){
			/* mdl2evm */
			if( etc_flag & MDU_FLAG_NO_LIMIT ) {
				ans=MakeEvm(hP3DModel, "evx" );
			} else {
				ans=MakeEvm(hP3DModel, "evm" );
			}
		}
		else{
			/* mdl2kc */
			if( etc_flag & MDU_FLAG_YFORMAT ) {
				ans=MakeKms(hP3DModel, "kmy" );
			}
			else if( etc_flag & MDU_FLAG_NO_LIMIT ) {
				ans=MakeKms(hP3DModel, "kmx" );
			} else {
				ans=MakeKms(hP3DModel, "kms" );
			}
			
		}
#endif

		// printf("11111\n");

		/* テクスチャ抽出 */
		if(ans==0){
			if((ans=MDU_ExtractTextureFromMdl( hP3DModel ))==0){
				MDU_SaveExtractTextures( dir ) ;
			}
		}

		// printf("22222\n");

		P3DDestroyModel( hP3DModel ) ;

		MDU_MemDump();

		exit(ans);
    }
}
