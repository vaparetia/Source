/*
   MDU_mdlMdl2Km4.c
   
   mdl -> .km4 変換モジュール for linux
   
   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/11/1999
   
   $Id: MDU_mdlMdl2Km4.c,v 1.15 2000/08/30 02:17:35 usr02011 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <SFDLINUX.h>

#include "METALGEAR.h"
#include "fmt_km4.h"
#include "MDU_util.h"

#ifdef MAX_MODELPIXEL
#undef MAX_MODELPIXEL
#define MAX_MODELPIXEL (32768)
#elif
#define MAX_MODELPIXEL (32768)
#endif

#define MAX_MODELPIXEL (32768)

#define	DEBUG
#ifdef DEBUG
#define	D( script ) 	script
#else
#define	D( script )
#endif

extern int	SkeltonModel ;

extern int	MDU_M2K_N_ObjFlags ;
extern char	MDU_M2K_ObjNames[ 256 ][ 256 ] ;
extern int	MDU_M2K_ObjFlags[ 256 ] ;

/*----------------------------------------------------------------------*/

typedef	struct	{
    double	v[ 3 ] ;
} MT_VEC ;

typedef	struct	{
    MT_VEC	m[ 3 ] ;
    MT_VEC	t ;
} MT_MAT ;

/*	回転マトリクスを計算する	*/
static	void	RotMatZYX( v1, m1 )
MT_VEC		*v1 ;
MT_MAT		*m1 ;
{
    double		sx, sy, sz ;
    double		cx, cy, cz ;
    double		th ;

    th = v1->v[ 0 ] ;	sx = sin( th ) ;	cx = cos( th ) ;
    th = v1->v[ 1 ] ;	sy = sin( th ) ;	cy = cos( th ) ;
    th = v1->v[ 2 ] ;	sz = sin( th ) ;	cz = cos( th ) ;
    m1->m[ 0 ].v[ 0 ] = cy * cz ;
    m1->m[ 1 ].v[ 0 ] = cy * sz ;
    m1->m[ 2 ].v[ 0 ] = - sy ;
    m1->m[ 0 ].v[ 1 ] = sx * sy * cz - cx * sz ;
    m1->m[ 1 ].v[ 1 ] = sx * sy * sz + cx * cz ;
    m1->m[ 2 ].v[ 1 ] = sx * cy ;
    m1->m[ 0 ].v[ 2 ] = cx * sy * cz + sx * sz ;
    m1->m[ 1 ].v[ 2 ] = cx * sy * sz - sx * cz ;
    m1->m[ 2 ].v[ 2 ] = cx * cy ;
    m1->t.v[ 0 ] = m1->t.v[ 1 ] = m1->t.v[ 2 ] = 0.0 ;
}

/*	ベクトルとマトリクスを乗算	*/
static	void	MulMatVec( m1, sv1, v1 )
MT_MAT		*m1 ;
MT_VEC		*sv1 ;
MT_VEC		*v1 ;
{
    double		sx, sy, sz ;
    
    {
		double		vx ;
	
		vx = sv1->v[ 0 ] ;
		sx  = m1->m[ 0 ].v[ 0 ] * vx ;
		sy  = m1->m[ 1 ].v[ 0 ] * vx ;
		sz  = m1->m[ 2 ].v[ 0 ] * vx ;
    }
    {
		double		vy ;
	
		vy = sv1->v[ 1 ] ;
		sx += m1->m[ 0 ].v[ 1 ] * vy ;
		sy += m1->m[ 1 ].v[ 1 ] * vy ;
		sz += m1->m[ 2 ].v[ 1 ] * vy ;
    }
    {
		double		vz ;
	
		vz = sv1->v[ 2 ] ;
		sx += m1->m[ 0 ].v[ 2 ] * vz ;
		sy += m1->m[ 1 ].v[ 2 ] * vz ;
		sz += m1->m[ 2 ].v[ 2 ] * vz ;
    }
    v1->v[ 0 ] = sx ;
    v1->v[ 1 ] = sy ;
    v1->v[ 2 ] = sz ;
}

/* オブジェの名前を得る */
static	void 	GetLocalObjName( res, org )
char		*res, *org ;
{
    int		n, open ;
    char	*p, *r, c ;

    n = 0 ; open = 0 ;
    p = org ; r = res ;
    while( ( c = *( p ++ ) ) != '\0' ) {
		if ( open == 1 ) {
			if ( c == '-' ) break ;
			*( r ++ ) = c ;
		}
		if ( c == '-' ) {
			if ( ++ n == 2 ) open = 1 ;
		}
    }
    *r = '\0' ;
}

/*----------------------------------------------------------------------*/

/*	 割り込み処理
	 fnInterrupt 関数を第1引数に処理率(%)、第2引数に lParam 引数をそのまま渡して
	 呼び出します。
	 */
static BOOL ProgressInterrupt
(								/* fnInterrupt 関数の戻り値(fnInterrupt が NULL のときは0以外) */
 long*  lplBuffer,				/* この関数の作業用領域(初回は0にすること) */
 long   lOffset,				/* 現在の位置 */
 long   lSize,					/* 最終位置 */
 LPARAM (*fnInterrupt)(WPARAM, LPARAM),	/* 割り込み関数のポインタ(NULL で無視) */
 LPARAM lParam)					/* fnInterrupt への引数 */
{
    if((fnInterrupt == NULL)
       || ((lOffset != 0) && (lOffset != lSize)
		   && (((double)lOffset / (double)lSize * 100.0) < (*lplBuffer + 5))))
		return 1;
    
    *lplBuffer = (int)((double)lOffset / (double)lSize * 100.0);
    return (BOOL)fnInterrupt((WPARAM)*lplBuffer, lParam);
}

/*----------------------------------------------------------------------*/

/* テクスチャ名格納用 */
extern	char	Textures[ MAX_TEXTURES ][ MAX_NAME_LEN ] ;
extern	int	N_Textures ;

static	int	FindTexture( name )
char		*name ;
{
    int		i ;

    for ( i = 0; i < N_Textures; i ++ ) {
		if ( !strcmp( Textures[ i ], name ) ) return i ;
    }
    return -1 ;
}

static	void	AddTexture( name )
char		*name ;
{
    strcpy( Textures[ N_Textures ], name ) ;
    N_Textures ++ ;
}

/*----------------------------------------------------------------------*/

#if 0

/* スケルトンオブジェクトのポリゴンを破棄 */
static void DestroySkeletonPolygon
(
 HP3DMODEL  hP3DModel,			/* モデルハンドル */
 HP3DOBJECT hP3DObject)			/* オブジェクトハンドル(NULL のときはトップレベルから) */
{
    HP3DOBJECT hP3DObjectL;
    
    if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
    {
		do
			DestroySkeletonPolygon(hP3DModel, hP3DObjectL);
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
    }
#if 0    
    if((hP3DObject != NULL)
       && (P3DObject(hP3DObject)->nType == P3D_OBJECTSKELETON))
		P3DDestroyObjectInfo(hP3DObject);
#endif
}

#endif


/* オブジェクトハンドルを検索する */
static HP3DOBJECT FindObjectName
(								/* オブジェクトハンドル */
 /* NULL オブジェクトが存在しない */
 HP3DMODEL  hP3DModel,			/* モデルハンドル */
 HP3DOBJECT hP3DObject,			/* 親になるオブジェクトハンドル(NULL でトップレベル) */
 LPCSTR     lpcszName)			/* 検索するオブジェクト名称 */
{
    HP3DOBJECT hP3DObjectL;
    HP3DOBJECT hP3DObjectHit;
    
    if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
    {
		do
		{
			if(strncmp(P3DObject(hP3DObjectL)->szName,
					   lpcszName, strlen(lpcszName)) == 0)
				return hP3DObjectL;
	    
			if((hP3DObjectHit = FindObjectName(
											   hP3DModel, hP3DObjectL, lpcszName)) != NULL)
				return hP3DObjectHit;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
    }
    
    return NULL;
}

/* オブジェクト名を変更する */
static void ChangeObjectName
(
 HP3DMODEL  hP3DModel,			/* モデルハンドル */
 HP3DOBJECT hP3DObject)			/* 親になるオブジェクトハンドル(NULL でトップレベル) */
{
    HP3DOBJECT hP3DObjectL;
    
    if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
    {
		do
		{
			strReplace(P3DObject(hP3DObjectL)->szName, "CONVMESH", "");
			ChangeObjectName(hP3DModel, hP3DObjectL);
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
    }
}

/* 親以外を見ているウェイトを削除する */
static void DestroyNoParentWeight
(
 HP3DMODEL  hP3DModel,			/* モデルハンドル */
 HP3DOBJECT hP3DObject)			/* 親になるオブジェクトハンドル(NULL でトップレベル) */
{
    int         i, j;
    HP3DOBJECT  hP3DObjectL;
    LPP3DWEIGHT lpP3DWeight;
    LPP3DVERTEX lpP3DVertex;
    int		parent ;
    double	not_parent_weight ;

    if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
    {
		do {	
			DestroyNoParentWeight(hP3DModel, hP3DObjectL);
		} while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
    }
    
    if(hP3DObject == NULL)
		return;

#if 0
    printf( "Object No %s\n", P3DObject(hP3DObject)->szName ) ;
    for(i = 0, lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex;
		i < P3DObject(hP3DObject)->nVertexCount; i++, lpP3DVertex++)
    {
		j = 0;
		lpP3DWeight = lpP3DVertex->lpP3DWeight;
		while(j < lpP3DVertex->nWeightCount)
		{
			if ( P3DGetParentObject( hP3DObject ) != NULL &&
				( P3DGetParentObject( hP3DObject ) ==
				 P3DFindObjectID( hP3DModel, lpP3DWeight->nObjectID ) ) ) {
				printf( "[%d] %d : %f\n", j, lpP3DWeight->nObjectID, 
					   ( float )lpP3DWeight->dWeight ) ;
			} else {
				printf( "[%d] not parent weight : %d : %f\n", 
					   j, lpP3DWeight->nObjectID, 
					   ( float )lpP3DWeight->dWeight ) ;
			}
			j ++ ;
			lpP3DWeight ++ ;
		}	
    }
#endif
    not_parent_weight = 0.0 ;
    parent = -1 ;
    for(i = 0, lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex;
		i < P3DObject(hP3DObject)->nVertexCount; i++, lpP3DVertex++)
    {
		j = 0;
		lpP3DWeight = lpP3DVertex->lpP3DWeight;
	
		while(j < lpP3DVertex->nWeightCount)
		{
			if((P3DGetParentObject(hP3DObject) == NULL)
			   || (P3DGetParentObject(hP3DObject)
				   != P3DFindObjectID(hP3DModel, lpP3DWeight->nObjectID)))
			{
				not_parent_weight += lpP3DWeight->dWeight ;
				P3DFreeVertexWeight(lpP3DVertex, j);
				j = 0;
				lpP3DWeight = lpP3DVertex->lpP3DWeight;
			}
			else
			{
				parent = lpP3DWeight->nObjectID ;
				j++;
				lpP3DWeight++;
			}
		}
    }
}

/* モデル数を数えておく */
static	int	CountModels( hP3DModel, hP3DObject, lpnNo )
HP3DMODEL	hP3DModel ;
HP3DOBJECT	hP3DObject ;
int		*lpnNo ;
{
    HP3DOBJECT  hP3DObjectL ;

    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
		return 1 ;
    do {
		( *lpnNo ) ++ ;
		if ( CountModels( hP3DModel, hP3DObjectL, lpnNo ) == 0 ) return 0 ;
    } while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
    return 1 ;
}


/* -------------------------------------------------------------------------- */
/* 追加部                                                                     */
/* -------------------------------------------------------------------------- */

static void GetVertexUserdata( lpP3DVertex, vert_usrdata )
LPP3DVERTEX lpP3DVertex;
VERTEX_USERDATA *vert_usrdata;
{
    LPMGS2MODELVERTEX lpMGS2ModelVertex;
    int size;

    lpMGS2ModelVertex
		= (LPMGS2MODELVERTEX)P3DUserDataVertex(lpP3DVertex,
											   MGS2_MODELVERTEXSTATUSKEYNAME, &size);
    if(lpMGS2ModelVertex!=NULL){
		vert_usrdata->nVertexSwing=lpMGS2ModelVertex->nVertexSwing;
		if(size==sizeof(MGS2MODELVERTEX)){
			vert_usrdata->dWeight[0]=(short)(lpMGS2ModelVertex->dWeight[0]*4096.0);
			vert_usrdata->dWeight[1]=(short)(lpMGS2ModelVertex->dWeight[1]*4096.0);
			vert_usrdata->dWeight[2]=(short)(lpMGS2ModelVertex->dWeight[2]*4096.0);
			vert_usrdata->dWeight[3]=(short)(lpMGS2ModelVertex->dWeight[3]*4096.0);

#if 0
			if(lpMGS2ModelVertex->dWeight[0]<0.0 ||
			   lpMGS2ModelVertex->dWeight[0]>1.0){

				printf("Strange0 F = %f\n",lpMGS2ModelVertex->dWeight[0]);
			}
			if(lpMGS2ModelVertex->dWeight[1]<0.0 ||
			   lpMGS2ModelVertex->dWeight[1]>1.0){

				printf("Strange1 F = %f\n",lpMGS2ModelVertex->dWeight[1]);
			}
			if(lpMGS2ModelVertex->dWeight[2]<0.0 ||
			   lpMGS2ModelVertex->dWeight[2]>1.0){

				printf("Strange2 F = %f\n",lpMGS2ModelVertex->dWeight[2]);
			}
			if(lpMGS2ModelVertex->dWeight[3]<0.0 ||
			   lpMGS2ModelVertex->dWeight[3]>1.0){

				printf("Strange3 F = %f\n",lpMGS2ModelVertex->dWeight[3]);
			}
#endif

		}
		else{
			vert_usrdata->dWeight[0]=0;
			vert_usrdata->dWeight[1]=0;
			vert_usrdata->dWeight[2]=0;
			vert_usrdata->dWeight[3]=0;
		}
    }
    else{
		memset(vert_usrdata,0x00,sizeof(VERTEX_USERDATA));
    }
}

static void GetNormalUserdata( lpP3DNormal, norm_usrdata )
LPP3DNORMAL lpP3DNormal;
NORMAL_USERDATA *norm_usrdata;
{
}

static void GetPrimitiveUserdata( lpP3DPolygon, prim_usrdata )
LPP3DPOLYGON lpP3DPolygon;
PRIMITIVE_USERDATA *prim_usrdata;
{
}

static void CheckNormal( def )
KM4_DEF		*def ;
{
    KM4_MDL *mdl;
    int i,j,k;

    struct _check_normal {
		int flag;
		FVECTOR env;
    } *check_normal;
    int n_check_normal;


    mdl=def->models;
    for(i=0;i<def->n_models;i++,mdl++){
		check_normal=(struct _check_normal *)MDU_Alloc(sizeof(struct _check_normal)*mdl->n_norms);
		if(check_normal==NULL) continue;

		memset(check_normal,0x00,sizeof(struct _check_normal)*mdl->n_norms);
		n_check_normal=mdl->n_norms;

		for(j=0;j<mdl->n_prims;j++){

			for(k=0;k<4;k++){
				int vid,nid;

				vid=mdl->vid[j*4+k];
				nid=mdl->nid[j*4+k];

				if(check_normal[nid].flag==0){
					check_normal[nid].env=mdl->envs[vid];
					check_normal[nid].flag=1;
				}
				else if(check_normal[nid].env.vx!=mdl->envs[vid].vx ||
						check_normal[nid].env.vy!=mdl->envs[vid].vy ||
						check_normal[nid].env.vz!=mdl->envs[vid].vz ||
						check_normal[nid].env.vw!=mdl->envs[vid].vw){

					struct _check_normal *check_normal2;
					FVECTOR *norms2;
					NORMAL_USERDATA *norm_usrdata2;

					check_normal2
						=(struct _check_normal *)MDU_Realloc(check_normal,
															 sizeof(struct _check_normal)
															 *(n_check_normal+1));
					if(check_normal2==NULL) continue;
					check_normal=check_normal2;

					norms2=(FVECTOR *)MDU_Realloc(mdl->norms,sizeof(FVECTOR)*(n_check_normal+1));
					if(norms2==NULL) continue;
					mdl->norms=norms2;

					norm_usrdata2
						=(NORMAL_USERDATA *)MDU_Realloc(mdl->norm_usrdata,
														sizeof(NORMAL_USERDATA)*(n_check_normal+1));
					if(norm_usrdata2==NULL && sizeof(NORMAL_USERDATA)!=0) continue;
					mdl->norm_usrdata=norm_usrdata2;

					mdl->norms[n_check_normal]=mdl->norms[nid];
					mdl->norm_usrdata[n_check_normal]=mdl->norm_usrdata[nid];

					nid=mdl->nid[j*4+k]=n_check_normal;

					check_normal[nid].env=mdl->envs[vid];
					check_normal[nid].flag=1;

					n_check_normal++;
				}
			}
		}
		mdl->n_norms=n_check_normal;
		MDU_Free(check_normal);
    }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


/* ポリゴンデータ変換 */
static	int	ConvertPolygon( mdl, hP3DModel, hP3DObject, lpP3DPolygon )
KM4_MDL		*mdl ;
HP3DMODEL	hP3DModel ;
HP3DOBJECT	hP3DObject ;
LPP3DPOLYGON	lpP3DPolygon ;
{
    int              	i, id ;
    char             	szFName[ _MAX_FNAME ] ;
    double           	dTxtU, dTxtV ;
    HP3DPICTURE      	hP3DPicture ;
    P3DPOLYGON       	p3dpolygon ;
    P3DUV            	p3duv ;
    P3DPOLYGONNODE   	p3dpolygonnode[ MAX_PIXEL ] ;
    LPP3DPOLYGONNODE 	lpP3DPolygonNode ;
    LPP3DTEXTURE     	lpP3DTexture ;
    u_short	      	*vid, *nid ;
    TVECTOR		*uvs ;
    u_int		*tid ;
    PRIMITIVE_USERDATA	*prim_usrdata;


    p3dpolygon = *lpP3DPolygon ;
    if( p3dpolygon.nPolygonNodeCount > MAX_PIXEL ) { /* ポリゴンを分割 */
		p3dpolygon.nPolygonNodeCount -= 2;
		p3dpolygon.lpP3DPolygonNode  += 2;
		if( ConvertPolygon( mdl, hP3DModel, hP3DObject, &p3dpolygon ) == 0 )
			return 0;
		p3dpolygon = *lpP3DPolygon ;
		p3dpolygon.nPolygonNodeCount = MAX_PIXEL ;
		p3dpolygon.lpP3DPolygonNode  = p3dpolygonnode ;
		p3dpolygonnode[ 0 ] = *( lpP3DPolygon->lpP3DPolygonNode + 0 ) ;
		p3dpolygonnode[ 1 ] = *( lpP3DPolygon->lpP3DPolygonNode + 1 ) ;
		p3dpolygonnode[ 2 ] = *( lpP3DPolygon->lpP3DPolygonNode + 2 ) ;
		p3dpolygonnode[ 3 ] = *( lpP3DPolygon->lpP3DPolygonNode
								+ ( lpP3DPolygon->nPolygonNodeCount - 1 ) ) ;
    }
    /* ポリゴン領域確保 */
    vid = ( u_short * )reallocEx( ( void * )mdl->vid, 
								 ( sizeof( u_short ) * 4 ) * ( mdl->n_prims + 1 ) ) ;
    nid = ( u_short * )reallocEx( ( void * )mdl->nid, 
								 ( sizeof( u_short ) * 4 ) * ( mdl->n_prims + 1 ) ) ;
    uvs = ( TVECTOR * )reallocEx( ( void * )mdl->uvs, 
								 ( sizeof( TVECTOR ) * 4 ) * ( mdl->n_prims + 1 ) ) ;
    tid = ( u_int * )reallocEx( ( void * )mdl->tid, 
							   sizeof( u_int ) * ( mdl->n_prims + 1 ) ) ;
    prim_usrdata
		= (PRIMITIVE_USERDATA *)reallocEx( ( void * )mdl->prim_usrdata,
										  sizeof( PRIMITIVE_USERDATA ) * ( mdl->n_prims + 1 ) ) ;

    if ( vid == NULL || nid == NULL || uvs == NULL || tid == NULL ) {
		if ( vid != NULL ) MDU_Free( vid ) ;
		if ( nid != NULL ) MDU_Free( nid ) ;
		if ( uvs != NULL ) MDU_Free( uvs ) ;
		if ( tid != NULL ) MDU_Free( tid ) ;
		if ( prim_usrdata != NULL ) MDU_Free( prim_usrdata ) ;
		return 0 ;
    }
    mdl->vid = vid ;
    mdl->nid = nid ;
    mdl->uvs = uvs ;
    mdl->tid = tid ;
    mdl->prim_usrdata = prim_usrdata ;

    /* 新規確保した位置 */
    vid = mdl->vid + mdl->n_prims * 4 ;
    nid = mdl->nid + mdl->n_prims * 4 ;
    uvs = mdl->uvs + mdl->n_prims * 4 ;
    tid = mdl->tid + mdl->n_prims ;
    prim_usrdata = mdl->prim_usrdata + mdl->n_prims ;

    memset( vid, 0x00, sizeof( u_short ) * 4 ) ;
    memset( nid, 0x00, sizeof( u_short ) * 4 ) ;
    memset( uvs, 0x00, sizeof( TVECTOR ) * 4 ) ;
    memset( tid, 0x00, sizeof( u_int ) ) ;
    memset( prim_usrdata, 0x00, sizeof( PRIMITIVE_USERDATA ) ) ;

    /* UV の中心 */
    dTxtU = 0.0 ;
    dTxtV = 0.0 ;
    for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
		i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {
		if( lpP3DPolygon->nTextureCount > 0 ) {
			/* テクスチュアの UV を設定(先頭のテクスチュアのみを変換) */
			//	    dTxtU += ( lpP3DPolygonNode->lpP3DUVCalc + 0 )->u ;
			//	    dTxtV += ( lpP3DPolygonNode->lpP3DUVCalc + 0 )->v ;
			dTxtU += ( lpP3DPolygonNode->lpP3DUV + 0 )->u ;
			dTxtV += ( lpP3DPolygonNode->lpP3DUV + 0 )->v ;
		}
    }

    dTxtU /= ( double )p3dpolygon.nPolygonNodeCount ;
    dTxtV /= ( double )p3dpolygon.nPolygonNodeCount ;

    dTxtU = floor( dTxtU ) ;
    dTxtV = floor( dTxtV ) ;

    /* 頂点・法線・UV を設定 */
	//printf( "[%d]", p3dpolygon.nPolygonNodeCount ) ;
    for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
		i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {
		/* PlayStation は右回りで座標を構築する */
		vid[ p3dpolygon.nPolygonNodeCount - i - 1 ] = lpP3DPolygonNode->nVertex ;
		nid[ p3dpolygon.nPolygonNodeCount - i - 1 ] = lpP3DPolygonNode->nNormal ;

		if ( lpP3DPolygon->nTextureCount <= 0 ) continue ;
		/* テクスチュアの UV を設定(先頭のテクスチュアのみを変換) */
		/* 同軸の座標が -1.0～1.0 を超えているばあい整数単位にひく */
		//	p3duv.u = ( lpP3DPolygonNode->lpP3DUVCalc + 0 )->u - dTxtU ;
		//	p3duv.v = ( lpP3DPolygonNode->lpP3DUVCalc + 0 )->v - dTxtV ;
		p3duv.u = ( lpP3DPolygonNode->lpP3DUV + 0 )->u - dTxtU ;
		p3duv.v = ( lpP3DPolygonNode->lpP3DUV + 0 )->v - dTxtV ;
		if( p3duv.u < 0.0 ) p3duv.u = 0.0 ;
		if( p3duv.u > 1.0 ) p3duv.u = 1.0 ;
		if( p3duv.v < 0.0 ) p3duv.v = 0.0 ;
		if( p3duv.v > 1.0 ) p3duv.v = 1.0 ;
	
		uvs[ p3dpolygon.nPolygonNodeCount - i - 1 ].u = ( float )p3duv.u ;
		uvs[ p3dpolygon.nPolygonNodeCount - i - 1 ].v = ( float )p3duv.v ;
		//printf( "[%d %d %f %f]", vid[ p3dpolygon.nPolygonNodeCount - i - 1 ],
				 //         nid[ p3dpolygon.nPolygonNodeCount - i - 1 ],
				 //         uvs[ p3dpolygon.nPolygonNodeCount - i - 1 ].u,
				 //         uvs[ p3dpolygon.nPolygonNodeCount - i - 1 ].v ) ;
		//printf( "[%f %f]", uvs[ p3dpolygon.nPolygonNodeCount - i - 1 ].u,
				 //         uvs[ p3dpolygon.nPolygonNodeCount - i - 1 ].v ) ;


#if 0
		printf("( U V ) = ( %5.3f %5.3f ) - ( %5.3f %5.3f ) = ( %5.3f %5.3f )\n",
			   ( lpP3DPolygonNode->lpP3DUVCalc + 0 )->u,( lpP3DPolygonNode->lpP3DUVCalc + 0 )->v,
			   dTxtU,dTxtV,
			   ( float )p3duv.u,( float )p3duv.v);
#elif 1
		printf("( U V ) = ( %5.3f %5.3f ) - ( %5.3f %5.3f ) = ( %5.3f %5.3f )\n",
			   ( lpP3DPolygonNode->lpP3DUV + 0 )->u,( lpP3DPolygonNode->lpP3DUV + 0 )->v,
			   dTxtU,dTxtV,
			   ( float )p3duv.u,( float )p3duv.v);
#else
		printf("( U V ) = ( %5.3f %5.3f ) - ( %5.3f %5.3f ) = ( %5.3f %5.3f )\n",
			   u,v,dTxtU,dTxtV,( float )p3duv.u,( float )p3duv.v);
#endif

    }
#if 0
    if ( vid[ 0 ] == vid[ 1 ] ||
		vid[ 0 ] == vid[ 2 ] ||
		vid[ 0 ] == vid[ 3 ] ||
		vid[ 1 ] == vid[ 2 ] ||
		vid[ 1 ] == vid[ 3 ] ) {
		printf( "\n??[%d %d %d %d]??\n", vid[ 0 ], vid[ 1 ], vid[ 2 ], vid[ 3 ] ) ;
    } else {
		printf( "\n" ) ;
    }
#endif

    /* テクスチュアの ID を設定 */
    if( lpP3DPolygon->nTextureCount > 0 ) {
		if( ( ( lpP3DTexture = 
			   P3DFindTextureID( hP3DObject, *(lpP3DPolygon->lpnTextureID + 0 ) ) ) == NULL )
		   || ( ( hP3DPicture = 
				 P3DFindPictureID( hP3DModel, lpP3DTexture->nPictureID ) ) == NULL ) )
			return 0 ;
		_splitpath( P3DPicture( hP3DPicture )->szName, NULL, NULL, szFName, NULL ) ;
		id = FindTexture( szFName ) ;
		if ( id < 0 ) {
			*tid = N_Textures ;
			AddTexture( szFName ) ;
		} else {
			*tid = id ;
		}
    }
    
    if( p3dpolygon.nPolygonNodeCount == 3 ) {
		vid[ 3 ] = vid[ 2 ] ;
		nid[ 3 ] = nid[ 2 ] ;
		uvs[ 3 ].u = uvs[ 2 ].u ;
		uvs[ 3 ].v = uvs[ 2 ].v ;
    }

    /* プリミティブの追加情報を取り出す */
    GetPrimitiveUserdata( &p3dpolygon, prim_usrdata ) ;

    mdl->n_prims ++ ;
    return 1 ;
}

/* オブジェクト変換 */
static	int	ConvertObject( def, hP3DModel, hP3DObject, lpp3dXYZScale, lpnNo )
KM4_DEF		*def ;
HP3DMODEL	hP3DModel ;
HP3DOBJECT	hP3DObject ;
LPP3DXYZ	lpp3dXYZScale ;
int		*lpnNo ;
{
    int          	i ;
    double       	dCalc ;
    P3DXYZ       	p3dXYZ ;
    HP3DOBJECT   	hP3DObjectL ;
    LPP3DVERTEX  	lpP3DVertex ;
    LPP3DNORMAL  	lpP3DNormal ;
    LPP3DPOLYGON 	lpP3DPolygon ;
    KM4_MDL		*mdl ;
    FVECTOR		*verts, *norms, *envs ;
    ENVDATA		*ed ;
    char		*objname, localname[ 256 ] ;
    VERTEX_USERDATA	*vert_usrdata;
    NORMAL_USERDATA	*norm_usrdata;
	
	
    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
		return 1 ;
    
    do {
		/* モデル変換 */
		mdl = def->models + *lpnNo ;
	
		P3DObject(hP3DObjectL)->wParam = *lpnNo; /* 連番 */
		P3DObject(hP3DObjectL)->lParam = (LPARAM)mdl ;

		/* 名前によって属性付け */
		objname = P3DObject( hP3DObjectL )->szName ;
		//printf( "objname %s\n", objname ) ;
#if 0
		if ( MDU_FindString( objname, "TRANS" ) >= 0 ) mdl->type |= DG_TYPE_TRANS ;
		if ( MDU_FindString( objname, "NO_SHADE" ) >= 0 ) mdl->type |= DG_TYPE_NOSHADE ;
		if ( MDU_FindString( objname, "_tr" ) >= 0 ) mdl->type |= DG_TYPE_TRANS ;
		if ( MDU_FindString( objname, "_ns" ) >= 0 ) mdl->type |= DG_TYPE_NOSHADE ;
#endif
		GetLocalObjName( localname, objname ) ;
		for ( i = 0; i < MDU_M2K_N_ObjFlags; i ++ ) {
			if ( !strcmp( localname, MDU_M2K_ObjNames[ i ] ) ) {	
				mdl->type = MDU_M2K_ObjFlags[ i ] ;
				printf( "set flag %x: %s\n", mdl->type, localname ) ;
				break ;
			}
		}

		mdl->type |= DG_TYPE_GT4 ;

		mdl->parent = -1 ;
		mdl->extend = -1 ; 
		printf( "mov %f %f %f\n", P3DObject( hP3DObjectL )->p3dXYZMove.x,
			   P3DObject( hP3DObjectL )->p3dXYZMove.y,
			   P3DObject( hP3DObjectL )->p3dXYZMove.z ) ;
		printf( "rot %f %f %f\n", P3DObject( hP3DObjectL )->p3dXYZRotate.x,
			   P3DObject( hP3DObjectL )->p3dXYZRotate.y,
			   P3DObject( hP3DObjectL )->p3dXYZRotate.z ) ;
        
		mdl->tx = ( float )( P3DObject( hP3DObjectL )->p3dXYZMove.x * lpp3dXYZScale->x ) ;
		mdl->ty = ( float )( P3DObject( hP3DObjectL )->p3dXYZMove.y * lpp3dXYZScale->y ) ;
		mdl->tz = ( float )( P3DObject( hP3DObjectL )->p3dXYZMove.z * lpp3dXYZScale->z ) ;
		mdl->n_verts = P3DObject( hP3DObjectL )->nVertexCount ;
		mdl->n_norms = P3DObject( hP3DObjectL )->nNormalCount ;

		/* 頂点・法線データ領域確保 */
		verts = mdl->verts = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
		norms = mdl->norms = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_norms ) ;
		if ( verts == NULL || norms == NULL ) return 0 ;
		envs = mdl->envs = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
		if ( envs == NULL ) return 0 ;
		vert_usrdata = mdl->vert_usrdata
			= ( VERTEX_USERDATA * )MDU_Alloc( sizeof( VERTEX_USERDATA ) * mdl->n_verts ) ;
		norm_usrdata = mdl->norm_usrdata
			= ( NORMAL_USERDATA * )MDU_Alloc( sizeof( NORMAL_USERDATA ) * mdl->n_norms ) ;
		if ( vert_usrdata == NULL || norm_usrdata == NULL ) return 0 ;

		/* 頂点データ変換 */
		for( i = 0, lpP3DVertex = P3DObject( hP3DObjectL )->lpP3DVertex;
			i < mdl->n_verts; i ++, lpP3DVertex++, verts ++, envs ++, vert_usrdata ++ ) {
#if 0
			verts->vx = ( float )( lpP3DVertex->p3dXYZ.x * lpp3dXYZScale->x ) ;
			verts->vy = ( float )( lpP3DVertex->p3dXYZ.y * lpp3dXYZScale->y ) ;
			verts->vz = ( float )( lpP3DVertex->p3dXYZ.z * lpp3dXYZScale->z ) ;
#else
			{
				MT_VEC	v1, v2 ;
				MT_MAT	m1 ;

				v1.v[ 0 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.x ;
				v1.v[ 1 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.y ;
				v1.v[ 2 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.z ;
				v1.v[ 0 ] = v1.v[ 0 ] * 3.141592653589793 / 180.0 ;
				v1.v[ 1 ] = v1.v[ 1 ] * 3.141592653589793 / 180.0 ;
				v1.v[ 2 ] = v1.v[ 2 ] * 3.141592653589793 / 180.0 ;
				RotMatZYX( &v1, &m1 ) ;
				v2.v[ 0 ] = lpP3DVertex->p3dXYZ.x ;
				v2.v[ 1 ] = lpP3DVertex->p3dXYZ.y ;
				v2.v[ 2 ] = lpP3DVertex->p3dXYZ.z ;
				MulMatVec( &m1, &v2, &v2 ) ;
				verts->vx = ( float )( v2.v[ 0 ] * lpp3dXYZScale->x ) ;
				verts->vy = ( float )( v2.v[ 1 ] * lpp3dXYZScale->y ) ;
				verts->vz = ( float )( v2.v[ 2 ] * lpp3dXYZScale->z ) ;
			}
#endif

			/* 頂点の追加情報を取ってくる */
			GetVertexUserdata( lpP3DVertex, vert_usrdata );


			if( lpP3DVertex->nWeightCount <= 0 ) {
				//		verts->vw = -1 ;
				ed = ( ENVDATA * )( &( verts->vw ) ) ;
				ed->this = ed->parent = 255 ;
				ed->id = ed->pad = 0 ;
			} else {
#if 0
				{
					int		a ;
					for ( a = 0; a < lpP3DVertex->nWeightCount; a ++ ) {
						printf( "%d : %d : %f\n", *lpnNo,
							   ( lpP3DVertex->lpP3DWeight + a )->nObjectID - 1,
							   ( float )( lpP3DVertex->lpP3DWeight + a )->dWeight ) ;
					}
					printf( "\n" ) ;
				}
#endif
				//                fprintf( stderr, "Weight exist\n" ) ;
				verts->vw = 0 ; 
				ed = ( ENVDATA * )( &( verts->vw ) ) ;
				ed->this = *lpnNo ;
				ed->parent = ( lpP3DVertex->lpP3DWeight + 0 )->nObjectID - 1,
				ed->id = ed->pad = 0 ;
				envs->vy = ( float )( lpP3DVertex->lpP3DWeight + 0 )->dWeight ; 
				envs->vx = 1.0F - envs->vy ;
				envs->vz = envs->vw = 0.0F ;
			}
		}	
		/* 法線データをコピー */
		for( i = 0, lpP3DNormal = P3DObject(hP3DObjectL)->lpP3DNormal;
			i < mdl->n_norms; i ++, lpP3DNormal ++, norms ++, norm_usrdata ++ ) {
			/* 法線合計値を4096にして内側を向かせる */
			p3dXYZ = lpP3DNormal->p3dXYZ ;
#if 1
			{
				MT_VEC	v1, v2 ;
				MT_MAT	m1 ;

				v1.v[ 0 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.x ;
				v1.v[ 1 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.y ;
				v1.v[ 2 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.z ;
				v1.v[ 0 ] = v1.v[ 0 ] * 3.141592653589793 / 180.0 ;
				v1.v[ 1 ] = v1.v[ 1 ] * 3.141592653589793 / 180.0 ;
				v1.v[ 2 ] = v1.v[ 2 ] * 3.141592653589793 / 180.0 ;
				RotMatZYX( &v1, &m1 ) ;
				v2.v[ 0 ] = p3dXYZ.x ;
				v2.v[ 1 ] = p3dXYZ.y ;
				v2.v[ 2 ] = p3dXYZ.z ;
				MulMatVec( &m1, &v2, &v2 ) ;
				p3dXYZ.x = v2.v[ 0 ] ;
				p3dXYZ.y = v2.v[ 1 ] ;
				p3dXYZ.z = v2.v[ 2 ] ;
			}	    
#endif
			if ( ( dCalc = sqrt( pow( fabs( p3dXYZ.x ), 2 )
								+ pow( fabs( p3dXYZ.y ), 2 )
								+ pow( fabs( p3dXYZ.z ), 2 ) ) ) == 0.0 )
				dCalc = 1.0 ;
			dCalc = -( 1.0 / dCalc ) ;
			p3dXYZ.x *= dCalc ;
			p3dXYZ.y *= dCalc ;
			p3dXYZ.z *= dCalc ;
			norms->vx = ( float )p3dXYZ.x ;
			norms->vy = ( float )p3dXYZ.y ;
			norms->vz = ( float )p3dXYZ.z ;
			norms->vw = -1 ;

			/* 法線の追加情報を取ってくる */
			GetNormalUserdata( lpP3DNormal, norm_usrdata );
		}	
		/* ポリゴンデータ変換 */
		for( i = 0, lpP3DPolygon = P3DObject(hP3DObjectL)->lpP3DPolygon;
			i < P3DObject(hP3DObjectL)->nPolygonCount; i ++, lpP3DPolygon ++ ) {
			if( ConvertPolygon( mdl, hP3DModel, hP3DObjectL, lpP3DPolygon ) == 0 )
				return 0;
		}	
		( *lpnNo ) ++ ;
		if ( ConvertObject( def, hP3DModel, hP3DObjectL, lpp3dXYZScale, lpnNo ) == 0 )
			return 0 ;
    } while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
#if 0
    {
		int		i, j ;
		u_short		*vid ;

		mdl = def->models ;
		//	for ( i = 0; i < def->n_x_models; i ++, mdl ++ ) {
			for ( i = 0; i < 1; i ++, mdl ++ ) {
				printf( "mdl[%d][%d][%d]\n", i, mdl->n_verts, mdl->n_prims ) ;
				vid = mdl->vid ;
				for ( j = 0; i < mdl->n_prims; j ++ ) {
					if ( vid[ 0 ] == vid[ 1 ] ||
						vid[ 0 ] == vid[ 2 ] ||
						vid[ 0 ] == vid[ 3 ] ||
						vid[ 1 ] == vid[ 2 ] ||
						vid[ 1 ] == vid[ 3 ] ) {
						printf( "[%d %d %d %d]", vid[ 0 ],
							   vid[ 1 ], vid[ 2 ], vid[ 3 ] ) ;
					}
					vid += 4 ;
				}
				printf( "\n" ) ;
			}
		}
	}
#endif
		return 1 ;
}

/* オブジェクト変換２ */
/* 親子番号設定 */
static void ConvertObject2( hP3DModel, hP3DObject )
HP3DMODEL  hP3DModel ;	/* モデルハンドル */
HP3DOBJECT hP3DObject ;	/* 親になるオブジェクトハンドル(NULL でトップレベル) */
{
	HP3DOBJECT 	hP3DObjectL ;
	KM4_MDL	*mdl ;

	if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
		return;
	do {
		mdl = ( KM4_MDL * )P3DObject( hP3DObjectL )->lParam ;
		if( hP3DObject != NULL )
			mdl->parent = P3DObject( hP3DObject )->wParam ;
		ConvertObject2( hP3DModel, hP3DObjectL ) ;
	} while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
}

/* オブジェクトの原点が、オブジェクトの中心に
   なるようにモデルを作り直す */
static	int	CenteringModel( mdl )
KM4_MDL		*mdl ;
{
	FVECTOR	center, *verts ;
	int		i ;

	printf( "centering model\n" ) ;
	center.vx = ( float )floor( ( double )( ( mdl->lx + mdl->ux ) / 2.0F ) + ( double )mdl->tx ) ;
	center.vy = ( float )floor( ( double )( ( mdl->ly + mdl->uy ) / 2.0F ) + ( double )mdl->ty ) ;
	center.vz = ( float )floor( ( double )( ( mdl->lz + mdl->uz ) / 2.0F ) + ( double )mdl->tz ) ;
	if ( mdl->n_prims > 0 ) {
		verts = mdl->verts ;
		verts->vx += mdl->tx ;
		verts->vy += mdl->ty ;
		verts->vz += mdl->tz ;
		mdl->lx = verts->vx - center.vx ;
		mdl->ly = verts->vy - center.vy ;
		mdl->lz = verts->vz - center.vz ;
		mdl->ux = verts->vx - center.vx ;
		mdl->uy = verts->vy - center.vy ;
		mdl->uz = verts->vz - center.vz ;
		verts ++ ;
		for ( i = 1; i < mdl->n_verts; i ++, verts ++ ) {
			verts->vx += mdl->tx ;
			verts->vy += mdl->ty ;
			verts->vz += mdl->tz ;
			verts->vx -= center.vx ;
			verts->vy -= center.vy ;
			verts->vz -= center.vz ;
			mdl->lx = __min( mdl->lx, verts->vx ) ;
			mdl->ly = __min( mdl->ly, verts->vy ) ;
			mdl->lz = __min( mdl->lz, verts->vz ) ;
			mdl->ux = __max( mdl->ux, verts->vx ) ;
			mdl->uy = __max( mdl->uy, verts->vy ) ;
			mdl->uz = __max( mdl->uz, verts->vz ) ;
		}
	}    
	if ( mdl->lx < -32768.0F || mdl->ly < -32768.0F || mdl->lz < -32768.0F ||
		mdl->ux > 32767.0F || mdl->uy > 32767.0F || mdl->uz > 32767.0F ) return -1 ;
	mdl->tx = center.vx ;
	mdl->ty = center.vy ;
	mdl->tz = center.vz ;
	return 0 ;
}

/* バウンティングボックスの設定 */
static void SetBoundingBox( hP3DModel, hP3DObject )
HP3DMODEL  hP3DModel ;	/* モデルハンドル */
HP3DOBJECT hP3DObject ;	/* 親になるオブジェクトハンドル(NULL でトップレベル) */
{
	int          	i ;
	HP3DOBJECT   	hP3DObjectL ;
	KM4_MDL		*mdl ;
	FVECTOR		*verts ;

	if ( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
		return ;
	do {
		mdl = ( KM4_MDL * )P3DObject( hP3DObjectL )->lParam ;
		if ( mdl->n_prims > 0 ) {
			verts = mdl->verts ;
			mdl->lx = mdl->ux = verts->vx ;
			mdl->ly = mdl->uy = verts->vy ;
			mdl->lz = mdl->uz = verts->vz ;
			verts ++ ;
			for ( i = 1; i < mdl->n_verts; i ++, verts ++ ) {
				mdl->lx = __min( mdl->lx, verts->vx ) ;
				mdl->ly = __min( mdl->ly, verts->vy ) ;
				mdl->lz = __min( mdl->lz, verts->vz ) ;
				mdl->ux = __max( mdl->ux, verts->vx ) ;
				mdl->uy = __max( mdl->uy, verts->vy ) ;
				mdl->uz = __max( mdl->uz, verts->vz ) ;
			}
		}
		if ( mdl->lx < -32768.0F || mdl->ly < -32768.0F || mdl->lz < -32768.0F ||
			mdl->ux > 32767.0F || mdl->uy > 32767.0F || mdl->uz > 32767.0F ) {
			if ( SkeltonModel == 0 ) {
				if ( CenteringModel( mdl ) < 0 ) {
					printf( "object too large!!!\n" ) ;
					exit( 255 ) ;
				}
			} else {
				printf( "object too large!!!\n" ) ;
				exit( 255 ) ;
			}
		}
		SetBoundingBox( hP3DModel, hP3DObjectL ) ;
	} while ( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
}

/* モデル全体のバウンティングボックスの設定 */
static void SetModelBoundingBox( def, hP3DModel, hP3DObject, bRotateMax )
KM4_DEF		*def ;
HP3DMODEL       hP3DModel ;	/* モデルハンドル */
HP3DOBJECT      hP3DObject ;	/* 親になるオブジェクトハンドル(NULL でトップレベル) */
BOOL            bRotateMax ;	/* オブジェクトを回転させ、一番大きなバウンティングボックスを取得する */
{
	P3DMATRIX  	p3dMatrix ;
	P3DXYZ     	p3dXYZRotate ;
	P3DXYZ     	p3dXYZMin, p3dXYZMax ;
	HP3DOBJECT 	hP3DObjectL ;
	KM4_MDL	*mdl ;

	if ( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
		return ;
	do {
		mdl = ( KM4_MDL * )P3DObject( hP3DObjectL )->lParam ;
		if( bRotateMax == FALSE ) {
			P3DInitMatrix( p3dMatrix ) ;
			P3DMoveMatrix( p3dMatrix, ( double )mdl->tx, 
						  ( double )mdl->ty, ( double )mdl->tz ) ;
			p3dXYZMin.x = ( double )mdl->lx ;
			p3dXYZMin.y = ( double )mdl->ly ;
			p3dXYZMin.z = ( double )mdl->lz ;
			p3dXYZMax.x = ( double )mdl->ux ;
			p3dXYZMax.y = ( double )mdl->uy ;
			p3dXYZMax.z = ( double )mdl->uz ;
	    
			P3DXYZMatrix( &p3dXYZMin, p3dMatrix ) ;
			P3DXYZMatrix( &p3dXYZMax, p3dMatrix ) ;

			def->lx = __min( def->lx, ( long )p3dXYZMin.x ) ;
			def->ly = __min( def->ly, ( long )p3dXYZMin.y ) ;
			def->lz = __min( def->lz, ( long )p3dXYZMin.z ) ;
			def->ux = __max( def->ux, ( long )p3dXYZMax.x ) ;
			def->uy = __max( def->uy, ( long )p3dXYZMax.y ) ;
			def->uz = __max( def->uz, ( long )p3dXYZMax.z ) ;
		} else {
			for( p3dXYZRotate.x = 0.0 ;
				p3dXYZRotate.x <= 270.0 ; p3dXYZRotate.x += 90.0 ) {
				for( p3dXYZRotate.y = 0.0 ;
					p3dXYZRotate.y <= 270.0 ; p3dXYZRotate.y += 90.0 ) {
					for( p3dXYZRotate.z = 0.0 ;
						p3dXYZRotate.z <= 270.0 ; p3dXYZRotate.z += 90.0 ) {
						P3DInitMatrix( p3dMatrix ) ;
						P3DRotateXYZMatrix( p3dMatrix,
										   p3dXYZRotate.x, p3dXYZRotate.y, p3dXYZRotate.z ) ;
						P3DMoveMatrix( p3dMatrix, ( double )mdl->tx, 
									  ( double )mdl->ty, ( double )mdl->tz ) ;
						p3dXYZMin.x = ( double )mdl->lx ;
						p3dXYZMin.y = ( double )mdl->ly ;
						p3dXYZMin.z = ( double )mdl->lz ;
						p3dXYZMax.x = ( double )mdl->ux ;
						p3dXYZMax.y = ( double )mdl->uy ;
						p3dXYZMax.z = ( double )mdl->uz ;
						P3DXYZMatrix( &p3dXYZMin, p3dMatrix ) ;
						P3DXYZMatrix( &p3dXYZMax, p3dMatrix ) ;
						def->lx = __min( def->lx, ( long )p3dXYZMin.x ) ;
						def->lx = __min( def->lx, ( long )p3dXYZMax.x ) ;
						def->ly = __min( def->ly, ( long )p3dXYZMin.y ) ;
						def->ly = __min( def->ly, ( long )p3dXYZMax.y ) ;
						def->lz = __min( def->lz, ( long )p3dXYZMin.z ) ;
						def->lz = __min( def->lz, ( long )p3dXYZMax.z ) ;

						def->ux = __max( def->ux, ( long )p3dXYZMin.x ) ;
						def->ux = __max( def->ux, ( long )p3dXYZMax.x ) ;
						def->uy = __max( def->uy, ( long )p3dXYZMin.y ) ;
						def->uy = __max( def->uy, ( long )p3dXYZMax.y ) ;
						def->uz = __max( def->uz, ( long )p3dXYZMin.z ) ;
						def->uz = __max( def->uz, ( long )p3dXYZMax.z ) ;
					}
				}
			}
		}
		SetModelBoundingBox( def, hP3DModel, hP3DObjectL, bRotateMax ) ;
	} while ( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
}

/* 拡張モデルの生成終了 ( CreateExtendModel 関数補佐) */
static BOOL CreateExtendUnitExit( bExit, model )
BOOL       	bExit ;		/* 戻り値 */
KM4_MDL	   	*model ;	/* 対象モデル */
{
	freeEx( ( void * )model->verts ) ;
	freeEx( ( void * )model->vid ) ;
	freeEx( ( void * )model->norms ) ;
	freeEx( ( void * )model->nid ) ;
	freeEx( ( void * )model->uvs ) ;
	freeEx( ( void * )model->tid ) ;
	freeEx( ( void * )model->envs ) ;
	freeEx( ( void * )model->vert_usrdata ) ;
	freeEx( ( void * )model->norm_usrdata ) ;
	freeEx( ( void * )model->prim_usrdata ) ;
	return bExit;
}

/* ポリゴンをモデルに設定 ( CreateExtendModel 関数補佐) */
static BOOL SetSplitModelPolygon( mdl, lpKMDVertexIDSrc, lpKMDNormalIDSrc,
								 model, nPolygon )
KM4_MDL		*mdl ;			/* 対象モデル */
LPKMDVECTIDL 	lpKMDVertexIDSrc ;	/* 頂点配列 */
LPKMDVECTIDL 	lpKMDNormalIDSrc ;	/* 法線配列 */
KM4_MDL		*model ;		/* 元のモデル */
int             nPolygon ;		/* 元のモデルのポリゴン位置 */
{
	u_short	*vid, *nid ;
	u_int	*tid ;
	TVECTOR	*uvs ;
	PRIMITIVE_USERDATA	*prim_usrdata;

	vid = nid = NULL ;
	tid = NULL ; uvs = NULL ;
	prim_usrdata = NULL ;

	vid = ( u_short * )reallocEx( ( void * )mdl->vid, 
								 sizeof( u_short ) * 4 * ( mdl->n_prims + 1 ) ) ;
	if ( vid == NULL ) return 0 ;
	nid = ( u_short * )reallocEx( ( void * )mdl->nid, 
								 sizeof( u_short ) * 4 * ( mdl->n_prims + 1 ) ) ;
	if ( nid == NULL ) return 0 ;
	uvs = ( TVECTOR * )reallocEx( ( void * )mdl->uvs, 
								 sizeof( TVECTOR ) * 4 * ( mdl->n_prims + 1 ) ) ;
	if ( uvs == NULL ) return 0 ;
	tid = ( u_int * )reallocEx( ( void * )mdl->tid, 
							   sizeof( u_int ) * ( mdl->n_prims + 1 ) ) ;
	if ( tid == NULL ) return 0 ;

	prim_usrdata
		= (PRIMITIVE_USERDATA *)reallocEx( ( void * )mdl->prim_usrdata,
										  sizeof( PRIMITIVE_USERDATA ) * ( mdl->n_prims + 1 ) ) ;
	if ( prim_usrdata == NULL ) return 0 ;

	mdl->vid = vid ;
	mdl->nid = nid ;
	mdl->uvs = uvs ;
	mdl->tid = tid ;
	mdl->prim_usrdata = prim_usrdata ;

	/* 新規確保した位置 */
	vid = vid + mdl->n_prims * 4 ;
	nid = nid + mdl->n_prims * 4 ;
	uvs = uvs + mdl->n_prims * 4 ;
	tid = tid + mdl->n_prims ;
	prim_usrdata = prim_usrdata + mdl->n_prims ;
    
	/* 頂点番号列を設定 */
	vid[ 0 ] = lpKMDVertexIDSrc->ID[ 0 ] ;
	vid[ 1 ] = lpKMDVertexIDSrc->ID[ 1 ] ;
	vid[ 2 ] = lpKMDVertexIDSrc->ID[ 2 ] ;
	vid[ 3 ] = lpKMDVertexIDSrc->ID[ 3 ] ;
    
	/* 法線番号列を設定 */
	nid[ 0 ] = lpKMDNormalIDSrc->ID[ 0 ] ;
	nid[ 1 ] = lpKMDNormalIDSrc->ID[ 1 ] ;
	nid[ 2 ] = lpKMDNormalIDSrc->ID[ 2 ] ;
	nid[ 3 ] = lpKMDNormalIDSrc->ID[ 3 ] ;
    
	/* テクスチュアの UV を設定 */
	*( uvs + 0 ) = *( model->uvs + nPolygon * 4 + 0 ) ;
	*( uvs + 1 ) = *( model->uvs + nPolygon * 4 + 1 ) ;
	*( uvs + 2 ) = *( model->uvs + nPolygon * 4 + 2 ) ;
	*( uvs + 3 ) = *( model->uvs + nPolygon * 4 + 3 ) ;
	*tid = *( model->tid + nPolygon ) ;

	/* 追加 */
	*prim_usrdata = *( model->prim_usrdata + nPolygon ) ;

	mdl->n_prims ++ ;
	return 1 ;
}

/* 拡張モデルの生成 */
static KM4_DEF	*CreateExtendModel( def, mdl, hP3DModel )
KM4_DEF		*def ;
KM4_MDL		*mdl ;
HP3DMODEL       hP3DModel ;
{
	int          i, j, k;
	LPKMDVECTIDL lpKMDVertexID;
	LPKMDVECTIDL lpKMDNormalID;
	KMDVECTIDL   vertID;
	KMDVECTIDL   normalID;
	KM4_MDL	model, *ext ;
	int		n_verts2, n_norms2 ;
	u_short	*vid, *nid ;
	FVECTOR	*verts, *norms, *verts2, *norms2 ;
	FVECTOR	*envs, *envs2 ;
	VERTEX_USERDATA	*vert_usrdata, *vert_usrdata2 ;
	NORMAL_USERDATA	*norm_usrdata, *norm_usrdata2 ;

	if ( mdl->n_verts <= MAX_MODELPIXEL &&
		mdl->n_norms <= MAX_MODELPIXEL ) return def ;
	model = *mdl ;
	/* 頂点・法線が MAX_MODELPIXEL 以下のモデルを生成する */
	verts2 = ( FVECTOR * )callocEx( MAX_MODELPIXEL, sizeof( FVECTOR ) ) ;
	if ( verts2 == NULL ) return NULL ;
	norms2 = ( FVECTOR * )callocEx( MAX_MODELPIXEL, sizeof( FVECTOR ) ) ;
	if ( norms2 == NULL ) return NULL ;
	envs2 = ( FVECTOR * )callocEx( MAX_MODELPIXEL, sizeof( FVECTOR ) ) ;
	if ( envs2 == NULL ) return NULL ;

	vert_usrdata2 = ( VERTEX_USERDATA * )callocEx( MAX_MODELPIXEL, sizeof( VERTEX_USERDATA ) ) ;
	if ( vert_usrdata2 == NULL ) return NULL;
	norm_usrdata2 = ( NORMAL_USERDATA * )callocEx( MAX_MODELPIXEL, sizeof( NORMAL_USERDATA ) ) ;
	if ( norm_usrdata2 == NULL ) return NULL ;

	mdl->n_prims = 0 ;
	mdl->n_verts = 0 ;
	mdl->verts = verts2 ;
	mdl->vid = NULL ;
	mdl->n_norms = 0 ;
	mdl->norms = norms2 ;
	mdl->nid = NULL ;
	mdl->uvs = NULL ;
	mdl->tid = NULL ;
	mdl->envs = envs2 ;

	mdl->vert_usrdata = NULL ;
	mdl->norm_usrdata = NULL ;
	mdl->prim_usrdata = NULL ;

	n_verts2 = n_norms2 = 0 ;

	for ( i = 0, vid = model.vid, nid = model.nid;
		 i < model.n_prims; i ++, vid += MAX_PIXEL, nid += MAX_PIXEL ) {
		for( j = 0; j < MAX_PIXEL; j ++ ) {
			/* 座標の設定 */
			verts = model.verts + vid[ j ] ;
			envs = model.envs + vid[ j ] ;
			vert_usrdata = model.vert_usrdata + vid[ j ] ;
			for( k = 0, verts2 = mdl->verts, envs2 = mdl->envs, vert_usrdata2 = mdl->vert_usrdata ;
				( k < n_verts2 ) &&
				( verts->vx != verts2->vx ||
				 verts->vy != verts2->vy ||
				 verts->vy != verts2->vy ) ;
				k ++, verts2 ++, envs2 ++, vert_usrdata2 ++ ) ;
			if ( k >= n_verts2 ) { /* 新しい頂点 */
				/* これ以上登録できない */
				if ( n_verts2 >= MAX_MODELPIXEL ) goto gotoEND ;
				*verts2 = *verts ;
				*envs2 = *envs ;
				*vert_usrdata2 = *vert_usrdata ;
				n_verts2 ++ ;
			}
			vertID.ID[ j ] = k ;
	    
			/* 法線の設定 */
			norms = model.norms + ( nid[ j ] & 0x7fff ) ;
			norm_usrdata = model.norm_usrdata + ( nid[ j ] & 0x7fff ) ;
			for( k = 0, norms2 = mdl->norms, norm_usrdata2 = mdl->norm_usrdata ;
				( k < n_norms2 ) && 
				( norms->vx != norms2->vx ||
				 norms->vy != norms2->vy ||
				 norms->vy != norms2->vy ) ;
				k ++, norms2 ++, norm_usrdata2 ++ ) ;
			if ( k >= n_norms2 ) { /* 新しい頂点 */
				/* これ以上登録できない */
				if ( n_norms2 >= MAX_MODELPIXEL ) goto gotoEND ;
				*norms2 = *norms ;
				*norm_usrdata2 = *norm_usrdata ;
				n_norms2 ++ ;
			}
			normalID.ID[ j ] = k ;
			if ( ( nid[ j ] & 0x8000 ) != 0 ) normalID.ID[ j ] |= 0x8000 ;
		}
		mdl->n_verts = n_verts2 ;
		mdl->n_norms = n_norms2 ;
	
		if ( SetSplitModelPolygon( mdl, &vertID, &normalID, &model, i ) == 0 ) {
			CreateExtendUnitExit( 0, &model ) ;
			return NULL ;
		}
	}
    
gotoEND :
    
	/* ポリゴンがすべてなくなったら終了、そのばあいは拡張モデルはなし */
	if ( i >= model.n_prims ) {
		CreateExtendUnitExit( 2, &model ) ;
		return def ;
	}
	/* メモリがリアロケートされる前にする */
	mdl->extend = def->n_x_models ;
    
	/* 拡張モデルの生成(モデルハンドル lParam に生成する) */
	def = ( KM4_DEF * )reallocEx( ( void * )def, sizeof( KM4_DEF ) + 
								 sizeof( KM4_MDL ) * ( def->n_x_models + 1 ) ) ;
	if ( def == NULL ) {
		CreateExtendUnitExit( 0, &model ) ;
		return NULL ;
	}
	/* ここ移行、引数 lpKMDModel は無効 */
	ext = def->models + def->n_x_models ;
	P3DModel( hP3DModel )->lParam = ( LPARAM )ext ;
	def->n_x_models ++ ;
    
	/* 古いモデルから新しいモデルを取り除いたモデルを生成する */
	memcpy( ext, &model, sizeof( KM4_MDL ) ) ;
	ext->n_prims = 0 ;
	ext->n_verts = 0 ;
	ext->verts = NULL ;
	ext->vid = NULL ;
	ext->n_norms = 0 ;
	ext->norms = NULL ;
	ext->nid = NULL ;
	ext->tid = NULL ;
	ext->uvs = NULL ;
	ext->envs = NULL ;
	ext->rgbs = NULL ;

	ext->vert_usrdata = NULL ;
	ext->norm_usrdata = NULL ;
	ext->prim_usrdata = NULL ;

	for ( ; i < model.n_prims; i ++, vid += MAX_PIXEL, nid += MAX_PIXEL ) {
		for( j = 0; j < MAX_PIXEL; j ++ ) {
			/* 座標の設定 */
			verts = model.verts + vid[ j ] ;
			envs = model.envs + vid[ j ] ;
			for ( k = 0, verts2 = ext->verts, envs2 = ext->envs, vert_usrdata2 = ext->vert_usrdata ;
				 ( k < ext->n_verts ) &&
				 ( ( verts->vx != verts2->vx ) ||
				  ( verts->vy != verts2->vy ) ||
				  ( verts->vz != verts2->vz ) ) ;
				 k ++, verts2 ++, envs2 ++, vert_usrdata2 ++ ) ;
			if ( k >= ext->n_verts ) { /* 新しい頂点 */
				verts2 = reallocEx( ( void * )ext->verts, 
								   sizeof( FVECTOR ) * ( ext->n_verts + 1 ) ) ;
				if ( verts2 == NULL ) {
					CreateExtendUnitExit( 0, &model ) ;
					return NULL ;
				}
				ext->verts = verts2 ;
				envs2 = reallocEx( ( void * )ext->envs, 
								  sizeof( FVECTOR ) * ( ext->n_verts + 1 ) ) ;
				if ( envs2 == NULL ) {
					CreateExtendUnitExit( 0, &model ) ;
					return NULL ;
				}
				ext->envs = envs2 ;

				/* 追加 */
				vert_usrdata2 = reallocEx( ( void * )ext->vert_usrdata,
										  sizeof( VERTEX_USERDATA ) * ( ext->n_verts + 1 ) ) ;
				if ( vert_usrdata2 == NULL ) {
					CreateExtendUnitExit( 0, &model ) ;
					return NULL ;
				}
				ext->vert_usrdata = vert_usrdata2 ;

				verts2 = verts2 + ext->n_verts ;
				envs2 = envs2 + ext->n_verts ;

				/* 追加 */
				vert_usrdata = vert_usrdata2 + ext->n_verts ;

				*verts2 = *verts ;
				*envs2 = *envs ;

				/* 追加 */
				*vert_usrdata2 = *vert_usrdata ;

				ext->n_verts ++ ;
			}
			vertID.ID[ j ] = k ;

			/* 法線の設定 */
			norms = model.norms + ( nid[ j ] & 0x7fff ) ;
			for ( k = 0, norms2 = ext->norms, norm_usrdata2 = ext->norm_usrdata ;
				 ( k < ext->n_norms ) &&
				 ( ( norms->vx != norms2->vx ) ||
				  ( norms->vy != norms2->vy ) ||
				  ( norms->vz != norms2->vz ) ) ;
				 k ++, norms2 ++, norm_usrdata2 ++ ) ;
			if ( k >= ext->n_norms ) { /* 新しい頂点 */
				norms2 = reallocEx( ( void * )ext->norms, 
								   sizeof( FVECTOR ) * ( ext->n_norms + 1 ) ) ;
				if ( norms2 == NULL ) {
					CreateExtendUnitExit( 0, &model ) ;
					return NULL ;
				}
				ext->norms = norms2 ;

				/* 追加 */
				norm_usrdata2 = reallocEx( ( void * )ext->norm_usrdata, 
										  sizeof( NORMAL_USERDATA ) * ( ext->n_norms + 1 ) ) ;
				if ( norm_usrdata2 == NULL ) {
					CreateExtendUnitExit( 0, &model ) ;
					return NULL ;
				}
				ext->norm_usrdata = norm_usrdata2 ;

				norms2 = norms2 + ext->n_norms ;

				/* 追加 */
				norm_usrdata2 = norm_usrdata2 + ext->n_norms ;

				*norms2 = *norms ;

				/* 追加 */
				*norm_usrdata2 = *norm_usrdata ;

				ext->n_norms ++ ;
			}
			normalID.ID[ j ] = k ;
			if ( ( nid[ j ] & 0x8000 ) != 0 ) {
				normalID.ID[ j ] |= 0x8000 ;
			}
		}

		if ( SetSplitModelPolygon( ext, &vertID, &normalID, &model, i ) == 0 ) {
			CreateExtendUnitExit( 0, &model ) ;
			return NULL ;
		}
	}
	CreateExtendUnitExit( 0, &model ) ;
	return CreateExtendModel( def, ext, hP3DModel ) ;
}

/* ユニットの分割 */
static KM4_DEF	*SplitModel( def, hP3DModel, hP3DObject )
KM4_DEF		*def ;
HP3DMODEL       hP3DModel ;	/* モデルハンドル */
HP3DOBJECT      hP3DObject ;	/* 親になるオブジェクトハンドル(NULL でトップレベル) */
{
	HP3DOBJECT 	hP3DObjectL ;
	KM4_MDL	*mdl ;
    
	if ( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
		return def ;
	do {
		mdl = ( KM4_MDL * )P3DObject(hP3DObjectL)->lParam ;
		if ( ( def = CreateExtendModel( def, mdl, hP3DModel ) ) == NULL )
			return NULL ;
		if ( ( def = SplitModel( def, hP3DModel, hP3DObjectL ) ) == NULL )
			return NULL ;
	} while ( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
	return def ;
}

/* キャラバウンド計算 */
static	void	SetModelBoundingBox2( def )
KM4_DEF		*def ;
{
	int		n_models, i ;
	KM4_MDL	*mdl, *zero, *parent ;
	float	maxlen, len, umax, tmax ;

	n_models = def->n_models ;
	mdl = zero = def->models ;
	maxlen = 0.0F ;
	for ( i = 0; i < n_models; i ++, mdl ++ ) {
		tmax = __max( fabs( mdl->tx ), fabs( mdl->ty ) ) ;
		tmax = __max( tmax, fabs( mdl->tz ) ) ;
		umax = __max( mdl->ux, mdl->uy ) ;
		umax = __max( umax, mdl->uz ) ;	
		len = tmax + umax ;
		if ( mdl->parent != -1 ) {
			parent = zero + mdl->parent ;
			while( 1 ) {
				tmax = __max( fabs( parent->tx ), fabs( parent->ty ) ) ;
				tmax = __max( tmax, fabs( parent->tz ) ) ;
				len += tmax ;
				if ( parent->parent == -1 ) break ;
				parent = zero + parent->parent ;
			}
		}
		if ( maxlen < len ) maxlen = len ;
	}
	def->lx = def->ly = def->lz = -( int )maxlen ;
	def->ux = def->uy = def->uz = ( int )maxlen ;
}

/* 変換 */
static	KM4_DEF	*Convert( hP3DModel, lpp3dXYZScale )
HP3DMODEL	hP3DModel ;
LPP3DXYZ	lpp3dXYZScale ;
{
	KM4_DEF	*def ;
	int		nNo ;
	HP3DOBJECT	hP3DObject ;
    
	/* ここに拡張モデルのポインタが格納される */    
	P3DModel( hP3DModel )->lParam = 0 ;
	nNo = 0 ; N_Textures = 0 ;
	CountModels( hP3DModel, NULL, &nNo ) ;

	D( printf( "%d Models\n", nNo ) ) ;

	def = ( KM4_DEF * )MDU_Alloc( sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * nNo ) ;
	if ( def == NULL ) return NULL ;

	/* ０クリア */
	memset( def, 0x00, sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * nNo ) ;

	/* オブジェクト変換 */
	def->n_models = nNo ;
	def->n_x_models = nNo ;
	nNo = 0 ;
	if ( ConvertObject( def, hP3DModel, NULL, lpp3dXYZScale, &nNo ) < 0 ) return NULL ;
	ConvertObject2( hP3DModel, NULL ) ;

	/* 追加、法線とエンベロープの関係をチェック */
	CheckNormal(def);

	D( printf( "CreateObject OK\n" ) ; ) ;
    
	/* バウンディング設定 */
	SetBoundingBox( hP3DModel, NULL ) ;
	if ( ( hP3DObject = P3DGetFirstObject( hP3DModel, NULL ) ) != NULL ) {
		def->lx = ( ( KM4_MDL * )( P3DObject( hP3DObject )->lParam ) )->lx ;
		def->ly = ( ( KM4_MDL * )( P3DObject( hP3DObject )->lParam ) )->ly ;
		def->lz = ( ( KM4_MDL * )( P3DObject( hP3DObject )->lParam ) )->lz ;
		def->ux = ( ( KM4_MDL * )( P3DObject( hP3DObject )->lParam ) )->ux ;
		def->uy = ( ( KM4_MDL * )( P3DObject( hP3DObject )->lParam ) )->uy ;
		def->uz = ( ( KM4_MDL * )( P3DObject( hP3DObject )->lParam ) )->uz ;
	}
	if ( SkeltonModel == 0 ) {
		SetModelBoundingBox( def, hP3DModel, NULL, FALSE ) ; /* ステージ */
#if 0
		/* 今だけ */
		def->models[ 0 ].tx = 0.0F ;
		def->models[ 0 ].ty = 0.0F ;
		def->models[ 0 ].tz = 0.0F ;
#endif
	} else {
		SetModelBoundingBox( def, hP3DModel, NULL, TRUE ) ;	/* 人物 */
		/* ０番モデルの位置を原点にする */
		def->models[ 0 ].tx = 0.0F ;
		def->models[ 0 ].ty = 0.0F ;
		def->models[ 0 ].tz = 0.0F ;
		/* バウンディング計算やり直し */
		SetModelBoundingBox2( def, lpp3dXYZScale ) ;
	}

	D( printf( "SetBounding OK\n" ) ; ) ;
	if ( ( def = SplitModel( def, hP3DModel, NULL ) ) == NULL ) return NULL ;
	D( printf( "SplitMode OK\n" ) ; ) ;
    
	return def ;
}

/*----------------------------------------------------------------------*/

KM4_DEF2	*MDU_Mdl2Km4( name, scale, hP3DModel, skel )
LPCSTR		name ;
LPP3DXYZ	scale ;
HP3DMODEL	hP3DModel ;
int		skel ;
{
	KM4_DEF2	*def2 ;
	KM4_DEF	*def ;
	int          i;
	int          nNo;
	int          fFile   = -1;
	long         lBuffer = 0;
	char*        lpszArgs[4];
	char         szText[P3D_MAXOBJECTNAME];
	char         szPath[_MAX_PATH];
	char         szDrive[_MAX_DRIVE];
	char         szDir[_MAX_DIR];
	char         szFName[_MAX_FNAME];
	HP3DMODEL    hP3DModelNew;
	HP3DMODEL    hP3DModelNew2;
	HP3DOBJECT   hP3DObject;
	HP3DPICTURE  hP3DPicture;

	SkeltonModel = skel ;

	D( printf( ".mdl -> .km4 conversion start\n" ) ; ) ;
	def = NULL ; def2 = NULL ;

	ProgressInterrupt( &lBuffer, 0, 100, NULL, 0 ) ;

#if 0
	if((hP3DModelNew = 
		P3DCreateConvertModel(hP3DModel, P3D_CCMMESH, 
							  P3D_CCMSKELETON, P3D_CCMNOCONVERT, NULL, 0)) == NULL) {
		return NULL ;
	}
	D( printf( "CreateConvertModel OK\n" ) ; ) ;
#endif

#if 0
	DestroySkeletonPolygon(hP3DModelNew, NULL) ;
	if((hP3DModelNew2 = P3DCreateWeightModel( hP3DModelNew, 
											 P3D_CWMMAXCHILD)) == NULL) {
		P3DDestroyModel(hP3DModelNew);
		return NULL ;
	}

	D( printf( "CreateWeightModel OK\n" ) ; ) ;
	P3DDestroyModel(hP3DModelNew);
	hP3DModelNew = hP3DModelNew2;

	if((hP3DObject = FindObjectName(hP3DModelNew, NULL, "CONVSKEL")) == NULL) {
		printf( "no skelton model\n" ) ;
		goto no_skelton_model ;
		//	P3DDestroyModel(hP3DModelNew);
		//	return NULL ;
	}
	if( ProgressInterrupt( &lBuffer, 20, 100, NULL, 0) == 0 ) return NULL ;

	ChangeObjectName(hP3DModelNew, NULL);
    
	strcpy(szText, P3DObject(hP3DObject)->szName);
	memset(lpszArgs, 0x00, sizeof(lpszArgs));
	lpszArgs[0] = strtok(szText, "-");
	for(i = 1; (lpszArgs[i - 1] != NULL)
		&& (i < (sizeof(lpszArgs) / sizeof(char*))); i++)
		lpszArgs[i] = strtok(NULL, "-");
    
	if(lpszArgs[2] == NULL) {
		P3DDestroyModel(hP3DModelNew);
		return NULL ;
	}
	sprintf(P3DObject(hP3DObject)->szName, "CONVSKEL-1-%s", lpszArgs[2]);
    
	if((hP3DModelNew2 = 
		P3DCreateConvertModel(hP3DModelNew, P3D_CCMMESH, 
							  P3D_CCMSKELETON, P3D_CCMNOCONVERT)) == NULL) {
		P3DDestroyModel(hP3DModelNew);
		return NULL ;
	}
	D( printf( "CreateConvertModel2 OK\n" ) ; ) ;

	P3DDestroyModel(hP3DModelNew);
	hP3DModelNew = hP3DModelNew2;
	DestroyNoParentWeight(hP3DModelNew, NULL);
#endif

no_skelton_model :
	if( ProgressInterrupt( &lBuffer, 40, 100, NULL, 0) == 0 ) return NULL ;

	def = Convert( hP3DModel, scale ) ;
	if ( def == NULL ) return NULL ;

	if( ProgressInterrupt( &lBuffer, 60, 100, NULL, 0 ) == 0 ) return NULL ;

	def2 = ( KM4_DEF2 * )MDU_Alloc( sizeof( KM4_DEF2 ) ) ;
	if ( def2 == NULL ) return NULL ;
	def2->def = def ;
	def2->n_texs = N_Textures ;
	for ( i = 0;i < N_Textures; i ++ ) {
		strcpy( def2->texs[ i ], Textures[ i ] ) ;
	}

	if( ProgressInterrupt( &lBuffer, 100, 100, NULL, 0 ) == 0 ) return NULL ;
    
	if ( SkeltonModel ) MDU_Km4ModelVertsInt( def ) ;

	return def2 ;
}
