/*
   Mdl2Kmx.c

   mdl -> .kmx 変換モジュール for linux

   by M.Sonoyama 1999.Sep.～ 
   Remade by K.Kano , 2/25/2000

   $Id: Mdl2Kmx.c,v 1.28 2002/12/26 07:08:11 usr01363 Exp $

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
#include "MDU_mdl.h"
#include "MDU_tex.h"


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

/* テクスチャ名格納用 */
extern 	char	Textures[ MAX_TEXTURES ][ MAX_NAME_LEN ] ;
extern  float	TexAlphaAverage[ MAX_TEXTURES ];
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

static	void	AddTexture( name, alpha )
char		*name ;
float		alpha ;
{
    strcpy( Textures[ N_Textures ], name ) ;
    TexAlphaAverage[ N_Textures ] = alpha;
    N_Textures ++ ;
}

/*----------------------------------------------------------------------*/


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


/*----------------------------------------------------------------------*/


/* 頂点の付加情報を取得 */
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

/* 法線の付加情報を取得 */
static void GetNormalUserdata( lpP3DNormal, norm_usrdata )
LPP3DNORMAL lpP3DNormal;
NORMAL_USERDATA *norm_usrdata;
{
}

/* プリミティブの付加情報を取得 */
static void GetPrimitiveUserdata( lpP3DPolygon, prim_usrdata )
LPP3DPOLYGON lpP3DPolygon;
PRIMITIVE_USERDATA *prim_usrdata;
{
}

/* -------------------------------------------------------------------------- */

/* ポリゴンデータ変換 */
static	int	ConvertPolygon( mdl, hP3DModel, hP3DObject, lpP3DPolygon )
KMX_MDL		*mdl ;
HP3DMODEL	hP3DModel ;
HP3DOBJECT	hP3DObject ;
LPP3DPOLYGON	lpP3DPolygon ;
{
    int              	i, j, id ;
    char             	szFName[ _MAX_FNAME ] ;
    double           	dTxtU, dTxtV ;
    HP3DPICTURE      	hP3DPicture ;
    P3DPOLYGON       	p3dpolygon ;
    P3DUV            	p3duv ;
    P3DPOLYGONNODE   	p3dpolygonnode[ MAX_PIXEL ] ;
    LPP3DPOLYGONNODE 	lpP3DPolygonNode ;
    LPP3DTEXTURE     	lpP3DTexture ;
    KMX_PRIMS		*new_prims;


    //printf("Check 3.1\n");

    p3dpolygon = *lpP3DPolygon ;
#if 1
    if( p3dpolygon.nPolygonNodeCount > MAX_PIXEL ) {	/* ポリゴンを分割 */
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
#else
    if( p3dpolygon.nPolygonNodeCount > 3 ) {	/* ポリゴンを分割 */
	p3dpolygon.nPolygonNodeCount -- ;
	p3dpolygon.lpP3DPolygonNode  ++ ;
	if( ConvertPolygon( mdl, hP3DModel, hP3DObject, &p3dpolygon ) == 0 )
	    return 0;
	p3dpolygon = *lpP3DPolygon ;
	p3dpolygon.nPolygonNodeCount = 3 ;
	p3dpolygon.lpP3DPolygonNode  = p3dpolygonnode ;
	p3dpolygonnode[ 0 ] = *( lpP3DPolygon->lpP3DPolygonNode + 0 ) ;
	p3dpolygonnode[ 1 ] = *( lpP3DPolygon->lpP3DPolygonNode + 1 ) ;
	p3dpolygonnode[ 2 ] = *( lpP3DPolygon->lpP3DPolygonNode
				 + ( lpP3DPolygon->nPolygonNodeCount - 1 ) ) ;
    }
#endif

    //printf("Check 3.2\n");

    /* ポリゴン領域確保 */
    if((new_prims=Kmx_Prims_Alloc(mdl->n_prims+1))==NULL) return 0;
    if(mdl->n_prims>0){
	Kmx_Prims_Move(new_prims,mdl->prims,mdl->n_prims);
	Kmx_Prims_Free(mdl->prims,mdl->n_prims);
    }

    mdl->prims=new_prims;

    //printf("Check 3.3\n");

    /* 新規確保した位置 */
    new_prims+=mdl->n_prims;
    if(!Kmx_PrimData_Alloc(new_prims,p3dpolygon.nPolygonNodeCount,
			   lpP3DPolygon->nTextureCount)){

	return 0;
    }
    mdl->n_prims ++ ;

    //printf("Check 3.4\n");

    /* 頂点、法線の設定 */
    for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
	 i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {

	/* PlayStation は右回りで座標を構築する */
	*(new_prims->vid + p3dpolygon.nPolygonNodeCount - i - 1 ) = lpP3DPolygonNode->nVertex ;
	*(new_prims->nid + p3dpolygon.nPolygonNodeCount - i - 1 ) = lpP3DPolygonNode->nNormal ;

#if 0
	printf("Vid Nid = %d %d\n",
	       lpP3DPolygonNode->nVertex,lpP3DPolygonNode->nNormal);
#endif
    }



#if 0
    // CheckNormal
    {
	FVECTOR *v0,*v1,*v2,vv0,vv1,vvn;
	FVECTOR *n0,*n1,*n2;

	v0=&((mdl->verts+*(new_prims->vid+0))->v);
	v1=&((mdl->verts+*(new_prims->vid+1))->v);
	v2=&((mdl->verts+*(new_prims->vid+2))->v);

	n0=&((mdl->norms+*(new_prims->nid+0))->n);
	n1=&((mdl->norms+*(new_prims->nid+1))->n);
	n2=&((mdl->norms+*(new_prims->nid+2))->n);

	vv0.vx=v1->vx-v0->vx;
	vv0.vy=v1->vy-v0->vy;
	vv0.vz=v1->vz-v0->vz;

	vv1.vx=v2->vx-v0->vx;
	vv1.vy=v2->vy-v0->vy;
	vv1.vz=v2->vz-v0->vz;

	vvn.vx=vv0.vy*vv1.vz-vv0.vz*vv1.vy;
	vvn.vy=vv0.vz*vv1.vx-vv0.vx*vv1.vz;
	vvn.vz=vv0.vx*vv1.vy-vv0.vy*vv1.vx;

	if(vvn.vx*n0->vx+vvn.vy*n0->vy+vvn.vz*n0->vz<0.0f){
	    printf("<-> 1\n");
	}
	if(vvn.vx*n1->vx+vvn.vy*n1->vy+vvn.vz*n1->vz<0.0f){
	    printf("<-> 2\n");
	}
	if(vvn.vx*n2->vx+vvn.vy*n2->vy+vvn.vz*n2->vz<0.0f){
	    printf("<-> 3\n");
	}
    }
#endif



    //printf("Check 3.5\n");

    if(lpP3DPolygon->nTextureCount==0){
	printf("Warning : Primitive has no Textures.\n");
    }

    //printf("Check 3.6\n");

#if 0 /* N Textures Check */
    printf("Textures = %d\n",lpP3DPolygon->nTextureCount);
#endif

    for(j=0;j<lpP3DPolygon->nTextureCount;j++){

	/* テクスチャの存在のチェック */
	if( ( lpP3DTexture = 
	      P3DFindTextureID( hP3DObject, *(lpP3DPolygon->lpnTextureID + j ) ) ) == NULL ){

	    goto no_texid;
	}
	if( ( hP3DPicture = 
	      P3DFindPictureID( hP3DModel, lpP3DTexture->nPictureID ) ) == NULL ){

no_texid:
	    printf("Warning : Texture has no ID.\n");

	    for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
		 i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {

		( new_prims->uvs
		  + p3dpolygon.nPolygonNodeCount * j
		  + p3dpolygon.nPolygonNodeCount - i - 1 )->u = 0.0f;
		( new_prims->uvs
		  + p3dpolygon.nPolygonNodeCount * j
		  + p3dpolygon.nPolygonNodeCount - i - 1 )->v = 0.0f;
	    }
	    *( new_prims->tid + j ) = 0 ;

	    continue;
	}


	/* UV の中心 */
	dTxtU = 0.0 ;
	dTxtV = 0.0 ;
	for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
	     i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {

#if 0
	    dTxtU += ( lpP3DPolygonNode->lpP3DUVCalc + j )->u ;
	    dTxtV += ( lpP3DPolygonNode->lpP3DUVCalc + j )->v ;
#elif 1
	    dTxtU += ( lpP3DPolygonNode->lpP3DUV + j )->u ;
	    dTxtV += ( lpP3DPolygonNode->lpP3DUV + j )->v ;
#else
	    double u,v;

	    u=*((float *)( lpP3DPolygonNode->lpP3DUVCalc + j )+0);
	    v=*((float *)( lpP3DPolygonNode->lpP3DUVCalc + j )+1);

	    dTxtU += u;
	    dTxtV += v;
#endif

	}
	dTxtU /= ( double )p3dpolygon.nPolygonNodeCount ;
	dTxtV /= ( double )p3dpolygon.nPolygonNodeCount ;

	/* 整数部を算出 */
	dTxtU = floor( dTxtU ) ;
	dTxtV = floor( dTxtV ) ;

	/* ＵＶの設定 */
	for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
	     i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {

	    /* テクスチュアの UV を設定(先頭のテクスチュアのみを変換) */
	    /* 同軸の座標が -1.0～1.0 を超えているばあい整数単位にひく */
#if 0
	    p3duv.u = ( lpP3DPolygonNode->lpP3DUVCalc + j )->u - dTxtU ;
	    p3duv.v = ( lpP3DPolygonNode->lpP3DUVCalc + j )->v - dTxtV ;
#elif 1
	    p3duv.u = ( lpP3DPolygonNode->lpP3DUV + j )->u - dTxtU ;
	    p3duv.v = ( lpP3DPolygonNode->lpP3DUV + j )->v - dTxtV ;
#else
	    double u,v;

	    u=*((float *)( lpP3DPolygonNode->lpP3DUVCalc + j )+0);
	    v=*((float *)( lpP3DPolygonNode->lpP3DUVCalc + j )+1);

	    p3duv.u = u - dTxtU ;
	    p3duv.v = v - dTxtV ;
#endif

	    if( p3duv.u < 0.0 ) p3duv.u = 0.0 ;
	    if( p3duv.u > 1.0 ) p3duv.u = 1.0 ;
	    if( p3duv.v < 0.0 ) p3duv.v = 0.0 ;
	    if( p3duv.v > 1.0 ) p3duv.v = 1.0 ;
	
	    ( new_prims->uvs
	      + p3dpolygon.nPolygonNodeCount * j
	      + p3dpolygon.nPolygonNodeCount - i - 1 )->u = ( float )p3duv.u ;
	    ( new_prims->uvs
	      + p3dpolygon.nPolygonNodeCount * j
	      + p3dpolygon.nPolygonNodeCount - i - 1 )->v = ( float )p3duv.v ;

#if 0
	    printf("( U V ) = ( %5.3f %5.3f ) - ( %5.3f %5.3f ) = ( %5.3f %5.3f )\n",
		   ( lpP3DPolygonNode->lpP3DUVCalc + j )->u,( lpP3DPolygonNode->lpP3DUVCalc + j )->v,
		   dTxtU,dTxtV,
		   ( float )p3duv.u,( float )p3duv.v);
#elif 0
	    printf("( U V ) = ( %5.3f %5.3f ) - ( %5.3f %5.3f ) = ( %5.3f %5.3f )\n",
		   ( lpP3DPolygonNode->lpP3DUV + j )->u,( lpP3DPolygonNode->lpP3DUV + j )->v,
		   dTxtU,dTxtV,
		   ( float )p3duv.u,( float )p3duv.v);
#elif 0
	    printf("( U V ) = ( %5.3f %5.3f ) - ( %5.3f %5.3f ) = ( %5.3f %5.3f )\n",
		   u,v,dTxtU,dTxtV,( float )p3duv.u,( float )p3duv.v);
#endif

	}

	/* テクスチャIDの設定 */
	_splitpath( P3DPicture( hP3DPicture )->szName, NULL, NULL, szFName, NULL ) ;
	id = FindTexture( szFName ) ;
	if ( id < 0 ) {
	    *( new_prims->tid + j ) = N_Textures ;
	    AddTexture( szFName, GetAlphaAverageFromPicture(hP3DPicture) ) ;
	} else {
	    *( new_prims->tid + j ) = id ;
	}
    }

    /* プリミティブの追加情報を取り出す */
    GetPrimitiveUserdata( &p3dpolygon, &(new_prims->prim_usrdata) ) ;

    return 1 ;
}


/* オブジェクト変換 */
static	int	ConvertObject( def, hP3DModel, hP3DObject, lpp3dXYZScale, lpnNo, path_num )
KMX_DEF		*def ;
HP3DMODEL	hP3DModel ;
HP3DOBJECT	hP3DObject ;
LPP3DXYZ	lpp3dXYZScale ;
int		*lpnNo ;
int		path_num;
{
    int          	i ;
    double       	dCalc ;
    P3DXYZ       	p3dXYZ ;
    HP3DOBJECT   	hP3DObjectL ;
    LPP3DVERTEX  	lpP3DVertex ;
    LPP3DNORMAL  	lpP3DNormal ;
    LPP3DPOLYGON 	lpP3DPolygon ;
    KMX_MDL		*mdl ;
    KMX_VERTS		*verts;
    KMX_NORMS		*norms;
    char		*objname, localname[ 256 ] ;
    MT_VEC	v1 ;
    MT_MAT	m1 ;

    extern int SelectPath(char *name);


    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
	return 1 ;
    
    do {
	if(SelectPath(P3DObject(hP3DObjectL)->szName)!=path_num) goto next;

	/* モデル変換 */
	mdl = def->models + *lpnNo ;
	
	P3DObject(hP3DObjectL)->wParam = *lpnNo;	/* 連番 */
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
	if((verts=mdl->verts=Kmx_Verts_Alloc(mdl->n_verts))==NULL) return -1;

#if 0
	printf("%d : %p\n",mdl->n_verts,mdl->verts);
#endif

	if((norms=mdl->norms=Kmx_Norms_Alloc(mdl->n_norms))==NULL) return -1;

	//printf("Check 1\n");

	v1.v[ 0 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.x ;
	v1.v[ 1 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.y ;
	v1.v[ 2 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.z ;
	v1.v[ 0 ] = v1.v[ 0 ] * M_PI / 180.0 ;
	v1.v[ 1 ] = v1.v[ 1 ] * M_PI / 180.0 ;
	v1.v[ 2 ] = v1.v[ 2 ] * M_PI / 180.0 ;
	RotMatZYX( &v1, &m1 ) ;

	/* 頂点データ変換 */
	for( i = 0, lpP3DVertex = P3DObject( hP3DObjectL )->lpP3DVertex;
	     i < mdl->n_verts; i ++, lpP3DVertex++, verts ++ ) {

	    v1.v[ 0 ] = lpP3DVertex->p3dXYZ.x ;
	    v1.v[ 1 ] = lpP3DVertex->p3dXYZ.y ;
	    v1.v[ 2 ] = lpP3DVertex->p3dXYZ.z ;
	    MulMatVec( &m1, &v1, &v1 ) ;
	    verts->v.vx = ( float )( v1.v[ 0 ] * lpp3dXYZScale->x ) ;
	    verts->v.vy = ( float )( v1.v[ 1 ] * lpp3dXYZScale->y ) ;
	    verts->v.vz = ( float )( v1.v[ 2 ] * lpp3dXYZScale->z ) ;

	    /* 頂点の追加情報を取ってくる */
	    GetVertexUserdata( lpP3DVertex, &(verts->vert_usrdata) );

	    if( lpP3DVertex->nWeightCount <= 0 ) {
		// verts->vw = -1 ;
		verts->env.objid=255;
		verts->env.parentid=255;
		verts->env.id=0;
		verts->env.val[0]=1.0f;
		verts->env.val[1]=0.0f;
	    }
	    else {
		//fprintf( stderr, "Weight exist\n" ) ;

		verts->env.objid=*lpnNo;
		verts->env.parentid=( lpP3DVertex->lpP3DWeight + 0 )->nObjectID - 1;
		verts->env.id=0;
		verts->env.val[1]=( float )( lpP3DVertex->lpP3DWeight + 0 )->dWeight ;
		verts->env.val[0]=1.0f-verts->env.val[1];

		/* ウェイト値が1.0の場合は、エンベロープであることをはずす */
		if(verts->env.val[1]==0.0){
		    verts->env.objid=255;
		    verts->env.parentid=255;
		}
	    }
	}

	//printf("Check 2\n");

	/* 法線データをコピー */
	for( i = 0, lpP3DNormal = P3DObject(hP3DObjectL)->lpP3DNormal;
	     i < mdl->n_norms; i ++, lpP3DNormal ++, norms ++) {

	    /* 法線合計値を4096にして内側を向かせる */

	    p3dXYZ = lpP3DNormal->p3dXYZ ;

	    v1.v[ 0 ] = p3dXYZ.x ;
	    v1.v[ 1 ] = p3dXYZ.y ;
	    v1.v[ 2 ] = p3dXYZ.z ;
	    MulMatVec( &m1, &v1, &v1 ) ;

	    // dCalc = sqrt( pow( p3dXYZ.x, 2 ) + pow( p3dXYZ.y, 2 ) + pow( p3dXYZ.z, 2 ) ) ;
	    dCalc=sqrt(v1.v[0]*v1.v[0]+v1.v[1]*v1.v[1]+v1.v[2]*v1.v[2]);
	    if ( dCalc == 0.0 ) dCalc = 1.0 ;

	    dCalc = -( 1.0 / dCalc ) ;
	    v1.v[ 0 ] *= dCalc ;
	    v1.v[ 1 ] *= dCalc ;
	    v1.v[ 2 ] *= dCalc ;
	    norms->n.vx = ( float )v1.v[ 0 ] ;
	    norms->n.vy = ( float )v1.v[ 1 ] ;
	    norms->n.vz = ( float )v1.v[ 2 ] ;
	    norms->n.vw = -1 ;

	    /* 法線の追加情報を取ってくる */
	    GetNormalUserdata( lpP3DNormal,&(norms->norm_usrdata) );
	}	

	//printf("Check 3\n");

	/* ポリゴンデータ変換 */
	for( i = 0, lpP3DPolygon = P3DObject(hP3DObjectL)->lpP3DPolygon;
	     i < P3DObject(hP3DObjectL)->nPolygonCount; i ++, lpP3DPolygon ++ ) {

	    if( ConvertPolygon( mdl, hP3DModel, hP3DObjectL, lpP3DPolygon ) == 0 ) return 0;
	}
	( *lpnNo ) ++ ;

	//printf("Check 4\n");

    next:
	if ( ConvertObject( def, hP3DModel, hP3DObjectL, lpp3dXYZScale,
			   lpnNo, path_num ) == 0 ) return 0 ;

    } while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;

    return 1 ;
}

/* オブジェクト変換２ */
/* 親子番号設定 */
static void ConvertObject2( hP3DModel, hP3DObject )
HP3DMODEL  hP3DModel ;	/* モデルハンドル */
HP3DOBJECT hP3DObject ;	/* 親になるオブジェクトハンドル(NULL でトップレベル) */
{
    HP3DOBJECT 	hP3DObjectL ;
    KMX_MDL	*mdl ;

    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL ) return;
    do {
	mdl = ( KMX_MDL * )P3DObject( hP3DObjectL )->lParam ;
	if( hP3DObject != NULL ) mdl->parent = P3DObject( hP3DObject )->wParam ;
	ConvertObject2( hP3DModel, hP3DObjectL ) ;
    } while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
}


/* オブジェクトの原点が、オブジェクトの中心に
   なるようにモデルを作り直す */
static	int	CenteringModel( mdl )
KMX_MDL	*mdl ;
{
    FVECTOR center;
    KMX_VERTS *verts;
    int		i ;

    printf( "centering model\n" ) ;
    center.vx = ( float )floor( ( double )( ( mdl->lx + mdl->ux ) / 2.0F ) ) ;
    center.vy = ( float )floor( ( double )( ( mdl->ly + mdl->uy ) / 2.0F ) ) ;
    center.vz = ( float )floor( ( double )( ( mdl->lz + mdl->uz ) / 2.0F ) ) ;
    mdl->tx += center.vx ;
    mdl->ty += center.vy ;
    mdl->tz += center.vz ;
    if ( mdl->n_prims > 0 ) {
	verts = mdl->verts ;
	mdl->lx = verts->v.vx - center.vx ;
	mdl->ly = verts->v.vy - center.vy ;
	mdl->lz = verts->v.vz - center.vz ;
	mdl->ux = verts->v.vx - center.vx ;
	mdl->uy = verts->v.vy - center.vy ;
	mdl->uz = verts->v.vz - center.vz ;
	for ( i = 0; i < mdl->n_verts; i ++, verts ++ ) {
	    verts->v.vx -= center.vx ;
	    verts->v.vy -= center.vy ;
	    verts->v.vz -= center.vz ;
	    mdl->lx = __min( mdl->lx, verts->v.vx ) ;
	    mdl->ly = __min( mdl->ly, verts->v.vy ) ;
	    mdl->lz = __min( mdl->lz, verts->v.vz ) ;
	    mdl->ux = __max( mdl->ux, verts->v.vx ) ;
	    mdl->uy = __max( mdl->uy, verts->v.vy ) ;
	    mdl->uz = __max( mdl->uz, verts->v.vz ) ;
	}
    }    
    if ( mdl->lx < -32768.0F || mdl->ly < -32768.0F || mdl->lz < -32768.0F ||
	 mdl->ux > 32767.0F || mdl->uy > 32767.0F || mdl->uz > 32767.0F ) return -1 ;
    return 0 ;
}

/* バウンティングボックスの設定 */
static int SetBoundingBox(KMX_DEF *def)
{
    int i;
    int ans=1;
    KMX_MDL *mdl;
    KMX_VERTS *verts;

    /* センターリングの影響で、tx,ty,tzがずれる可能性があるので、
       あらかじめ絶対座標系にする。*/
    mdl=def->models;
    for(i=0;i<def->n_x_models;i++){
	int parent=(mdl+i)->parent;
	if(parent!=-1){
	    if((mdl+i)->type & DG_TYPE_EXTEND){
		(mdl+i)->tx=(mdl+parent)->tx;
		(mdl+i)->ty=(mdl+parent)->ty;
		(mdl+i)->tz=(mdl+parent)->tz;
	    }
	    else{
		(mdl+i)->tx+=(mdl+parent)->tx;
		(mdl+i)->ty+=(mdl+parent)->ty;
		(mdl+i)->tz+=(mdl+parent)->tz;
	    }
	}
    }

    mdl=def->models;
    for(i=0;i<def->n_x_models;i++,mdl++){
	int j;

#if 1
	mdl->ux=0.0f;
	mdl->uy=0.0f;
	mdl->uz=0.0f;
	mdl->lx=0.0f;
	mdl->ly=0.0f;
	mdl->lz=0.0f;
#else
	mdl->ux=mdl->uy=mdl->uz=-3.40282347e+38f;
	mdl->lx=mdl->ly=mdl->lz=3.40282347e+38f;
#endif

	if(mdl->n_prims>0) {
	    verts = mdl->verts ;
	    mdl->lx = mdl->ux = verts->v.vx ;
	    mdl->ly = mdl->uy = verts->v.vy ;
	    mdl->lz = mdl->uz = verts->v.vz ;
	    verts ++ ;
	    for ( j = 1; j < mdl->n_verts; j ++, verts ++ ) {
		mdl->lx = __min( mdl->lx, verts->v.vx ) ;
		mdl->ly = __min( mdl->ly, verts->v.vy ) ;
		mdl->lz = __min( mdl->lz, verts->v.vz ) ;
		mdl->ux = __max( mdl->ux, verts->v.vx ) ;
		mdl->uy = __max( mdl->uy, verts->v.vy ) ;
		mdl->uz = __max( mdl->uz, verts->v.vz ) ;
	    }
	}
	if ( mdl->lx < -32768.0F || mdl->ly < -32768.0F || mdl->lz < -32768.0F ||
	     mdl->ux > 32767.0F || mdl->uy > 32767.0F || mdl->uz > 32767.0F ) {
	    if ( SkeltonModel == 0 ) {
		if ( CenteringModel( mdl ) < 0 ) {
		    printf( "Error : Obj %d : object too large!!!\n",i ) ;
		    ans=0;
		    // exit( 255 ) ;
		}
	    } else {
		printf( "Error : Obj %d : object too large!!!\n",i ) ;
		ans=0;
		//exit( 255 ) ;
	    }
	}
    }

    /* tx,ty,tzを相対に戻す */
    mdl=def->models;
    for(i=def->n_x_models-1;i>=0;i--){
	int parent=(mdl+i)->parent;
	if(parent!=-1){
	    (mdl+i)->tx-=(mdl+parent)->tx;
	    (mdl+i)->ty-=(mdl+parent)->ty;
	    (mdl+i)->tz-=(mdl+parent)->tz;
	}
    }

    return ans;
}

/* メッシュモデルのバウンディングボックスの計算 */
static void SetModelBoundingBox( def )
KMX_DEF *def ;
{
    int		n_models, i ;
    KMX_MDL	*mdl, *zero, *parent ;
    int flag=0;


    n_models = def->n_x_models ;
    mdl = zero = def->models ;

    def->lx=0.0f;
    def->ly=0.0f;
    def->lz=0.0f;
    def->ux=0.0f;
    def->uy=0.0f;
    def->uz=0.0f;

    for ( i = 0; i < n_models; i ++, mdl ++ ) {
	float lx,ly,lz,ux,uy,uz;

	if(mdl->n_prims<=0) continue;

	lx=mdl->tx+mdl->lx;
	ly=mdl->ty+mdl->ly;
	lz=mdl->tz+mdl->lz;
	ux=mdl->tx+mdl->ux;
	uy=mdl->ty+mdl->uy;
	uz=mdl->tz+mdl->uz;

#if 1
	/* 階層を考慮に入れる場合 */
	if ( mdl->parent != -1 ) {
	    parent = zero + mdl->parent ;

	    while( 1 ) {
		lx+=parent->tx;
		ly+=parent->ty;
		lz+=parent->tz;
		ux+=parent->tx;
		uy+=parent->ty;
		uz+=parent->tz;

		if ( parent->parent == -1 ) break ;
		parent = zero + parent->parent ;
	    }
	}
#endif

	if(flag){
	    def->lx=__min(def->lx,lx);
	    def->ly=__min(def->ly,ly);
	    def->lz=__min(def->lz,lz);
	    def->ux=__max(def->ux,ux);
	    def->uy=__max(def->uy,uy);
	    def->uz=__max(def->uz,uz);
	}
	else{
	    def->lx=lx;
	    def->ly=ly;
	    def->lz=lz;
	    def->ux=ux;
	    def->uy=uy;
	    def->uz=uz;
	    flag=1;
	}
    }
}


/* スケルトンモデルのバウンディングボックスの計算 */
static void SetModelBoundingBox2( def )
KMX_DEF *def ;
{
    int		n_models, i ;
    KMX_MDL	*mdl, *zero, *parent ;
    float	maxlen, len, umax, tmax ;

    n_models = def->n_x_models ;
    mdl = zero = def->models ;
    maxlen = 0.0F ;

    for ( i = 0; i < n_models; i ++, mdl ++ ) {

	if(mdl->n_prims<=0) continue;

	tmax = __max( fabs( mdl->tx ), fabs( mdl->ty ) ) ;
	tmax = __max( tmax, fabs( mdl->tz ) ) ;
	umax = __max( fabs( mdl->ux ), fabs( mdl->uy ) ) ;
	umax = __max( umax, fabs( mdl->uz ) ) ;
	umax = __max( umax, fabs( mdl->lx ) ) ;
	umax = __max( umax, fabs( mdl->ly ) ) ;
	umax = __max( umax, fabs( mdl->lz ) ) ;
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


static KMX_DEF *CreateExtendModel( def, index )
KMX_DEF *def;
int index;
{
    KMX_DEF *new_def;
    KMX_MDL *mdl;
    KMX_MDL *new_mdl1,*new_mdl2;
    KMX_MDL div1,div2;
    KMX_PRIMS *prim,*ext_prim;
    u_int *check_vid,*check_nid;
    int n_check_vid,n_check_nid;
    int i;
    int old_n_prims;
    int new_index;

    mdl=def->models+index;

    if(mdl->n_verts<=MAX_MODELPIXEL &&
       mdl->n_norms<=MAX_MODELPIXEL) return def;

    mdl=def->models+index;

    if((check_vid=(u_int *)MDU_Alloc(sizeof(u_int)*(mdl->n_verts+mdl->n_norms)))==NULL){
	Kmx_Free(def);
	return NULL;
    }
    check_nid=check_vid+mdl->n_verts;
    memset(check_vid,0xff,sizeof(u_int)*(mdl->n_verts+mdl->n_norms));

    n_check_vid=0;
    n_check_nid=0;


    div1.n_verts=0;
    div1.verts=NULL;
    div1.n_norms=0;
    div1.norms=NULL;
    div1.n_prims=0;
    div1.prims=NULL;

    old_n_prims=mdl->n_prims;

    if(!Kmx_MdlData_Alloc(&div1,mdl->n_verts,mdl->n_norms,mdl->n_prims)){
	Kmx_Free(def);
	MDU_Free(check_vid);
	return NULL;
    }


    /* モデル内のプリミティブデータを一つ一つ移動し、頂点データもしくは法線データが
       限度値を越えるまで移動させる */
    prim=mdl->prims;
    ext_prim=div1.prims;
    div1.n_prims=0;
    for(i=0;i<mdl->n_prims;i++,prim++,ext_prim++){
	int j;

	if(!Kmx_PrimData_Alloc(ext_prim,prim->n_id,prim->n_tid)){
	    Kmx_Free(def);
	    MDU_Free(check_vid);
	    Kmx_MdlData_Free(&div1);
	    return NULL;
	}

	/* 頂点データのコピー */
	for(j=0;j<prim->n_id;j++){
	    u_int vid=*(prim->vid+j);
	    KMX_VERTS *vert=mdl->verts+vid;

	    if(*(check_vid+vid)==0xffffffff){
		memcpy(div1.verts+n_check_vid,vert,sizeof(KMX_VERTS));
		*(check_vid+vid)=n_check_vid;
		n_check_vid++;
	    }
	    *(ext_prim->vid+j)=*(check_vid+vid);

	    if(n_check_vid>=MAX_MODELPIXEL) goto next;
	}
	div1.n_verts=n_check_vid;

	/* 法線データのコピー */
	for(j=0;j<prim->n_id;j++){
	    u_int nid=*(prim->nid+j);
	    KMX_NORMS *norm=mdl->norms+nid;

	    if(*(check_nid+nid)==0xffffffff){
		memcpy(div1.norms+n_check_nid,norm,sizeof(KMX_NORMS));
		*(check_nid+nid)=n_check_nid;
		n_check_nid++;
	    }
	    *(ext_prim->nid+j)=*(check_nid+nid);

	    if(n_check_nid>=MAX_MODELPIXEL) goto next;
	}
	div1.n_norms=n_check_nid;

	/* プリミティブデータのコピー */
	memcpy(ext_prim->tid,prim->tid,sizeof(u_int)*prim->n_tid);
	memcpy(ext_prim->uvs,prim->uvs,sizeof(TVECTOR)*prim->n_id*prim->n_tid);
	memcpy(&(ext_prim->prim_usrdata),&(prim->prim_usrdata),sizeof(PRIMITIVE_USERDATA));

	div1.n_prims++;
    }

    /* 特に分割の必要が無い場合(頂点情報や法線情報に余分なデータが含まれている場合)、
       そのまま抜ける。*/
    MDU_Free(check_vid);
    Kmx_MdlData_Free(&div1);

    return def;

next:
    /* 後半部分の作成 */
    div2.n_verts=0;
    div2.verts=NULL;
    div2.n_norms=0;
    div2.norms=NULL;
    div2.n_prims=0;
    div2.prims=NULL;

    if(!Kmx_MdlData_Alloc(&div2,mdl->n_verts,mdl->n_norms,mdl->n_prims)){
	Kmx_Free(def);
	MDU_Free(check_vid);

	div1.n_prims=old_n_prims;
	Kmx_MdlData_Free(&div1);

	return NULL;
    }

    memset(check_vid,0xff,sizeof(u_int)*mdl->n_verts);
    memset(check_nid,0xff,sizeof(u_int)*mdl->n_norms);

    n_check_vid=0;
    n_check_nid=0;

    /* 残りの部分を、div2へコピーする。*/
    ext_prim=div2.prims;
    div2.n_prims=0;
    for( ;i<mdl->n_prims;i++,prim++,ext_prim++){
	int j;

	if(!Kmx_PrimData_Alloc(ext_prim,prim->n_id,prim->n_tid)){
	    Kmx_Free(def);
	    MDU_Free(check_vid);

	    div1.n_prims=old_n_prims;
	    Kmx_MdlData_Free(&div1);

	    div2.n_prims=old_n_prims;
	    Kmx_MdlData_Free(&div2);

	    return NULL;
	}

	/* 頂点データのコピー */
	for(j=0;j<prim->n_id;j++){
	    u_int vid=*(prim->vid+j);
	    KMX_VERTS *vert=mdl->verts+vid;

	    if(*(check_vid+vid)==0xffffffff){
		memcpy(div2.verts+n_check_vid,vert,sizeof(KMX_VERTS));
		*(check_vid+vid)=n_check_vid;
		n_check_vid++;
	    }
	    *(ext_prim->vid+j)=*(check_vid+vid);
	}
	div2.n_verts=n_check_vid;

	/* 法線データのコピー */
	for(j=0;j<prim->n_id;j++){
	    u_int nid=*(prim->nid+j);
	    KMX_NORMS *norm=mdl->norms+nid;

	    if(*(check_nid+nid)==0xffffffff){
		memcpy(div2.norms+n_check_nid,norm,sizeof(KMX_NORMS));
		*(check_nid+nid)=n_check_nid;
		n_check_nid++;
	    }
	    *(ext_prim->nid+j)=*(check_nid+nid);
	}
	div2.n_norms=n_check_nid;

	/* プリミティブデータのコピー */
	memcpy(ext_prim->tid,prim->tid,sizeof(u_int)*prim->n_tid);
	memcpy(ext_prim->uvs,prim->uvs,sizeof(TVECTOR)*prim->n_id*prim->n_tid);
	memcpy(&(ext_prim->prim_usrdata),&(prim->prim_usrdata),sizeof(PRIMITIVE_USERDATA));

	div2.n_prims++;
    }

    /* プリミティブを、前半後半に分けることが出来たので、
       拡張モデル部に後半部を移動する */

    new_index=def->n_x_models;

    if((new_def=Kmx_Alloc(def->n_models,def->n_x_models+1))==NULL){
	Kmx_Free(def);
	MDU_Free(check_vid);

	div1.n_prims=old_n_prims;
	Kmx_MdlData_Free(&div1);

	div2.n_prims=old_n_prims;
	Kmx_MdlData_Free(&div2);

	return NULL;
    }

    new_mdl1=new_def->models+index;
    memcpy(new_mdl1,mdl,sizeof(KMX_MDL));

    new_mdl1->verts=NULL;
    new_mdl1->norms=NULL;
    new_mdl1->prims=NULL;

    new_mdl2=new_def->models+new_index;
    memcpy(new_mdl2,mdl,sizeof(KMX_MDL));

    if(index<def->n_models) new_mdl2->parent=index;
    new_mdl2->extend=def->n_x_models;

    new_mdl2->type|=DG_TYPE_EXTEND;

    new_mdl2->verts=NULL;
    new_mdl2->norms=NULL;
    new_mdl2->prims=NULL;

    new_mdl1->n_verts=div1.n_verts;
    new_mdl1->n_norms=div1.n_norms;
    new_mdl1->n_prims=div1.n_prims;
    new_mdl2->n_verts=div2.n_verts;
    new_mdl2->n_norms=div2.n_norms;
    new_mdl2->n_prims=div2.n_prims;

    if(!Kmx_MdlData_Alloc(new_mdl1,new_mdl1->n_verts,new_mdl1->n_norms,new_mdl1->n_prims)){
	Kmx_Free(def);
	Kmx_Free(new_def);
	MDU_Free(check_vid);

	div1.n_prims=old_n_prims;
	Kmx_MdlData_Free(&div1);

	div2.n_prims=old_n_prims;
	Kmx_MdlData_Free(&div2);

	return NULL;
    }
    if(!Kmx_MdlData_Alloc(new_mdl2,new_mdl2->n_verts,new_mdl2->n_norms,new_mdl2->n_prims)){
	Kmx_Free(def);
	Kmx_Free(new_def);
	MDU_Free(check_vid);

	div1.n_prims=old_n_prims;
	Kmx_MdlData_Free(&div1);

	div2.n_prims=old_n_prims;
	Kmx_MdlData_Free(&div2);

	return NULL;
    }

    /* データをコピー */
    new_def->lx=def->lx;
    new_def->ly=def->ly;
    new_def->lz=def->lz;
    new_def->ux=def->ux;
    new_def->uy=def->uy;
    new_def->uz=def->uz;
    new_def->tx=def->tx;
    new_def->ty=def->ty;
    new_def->tz=def->tz;
    new_def->n_texs=def->n_texs;
    new_def->texs=def->texs;

    def->texs=NULL;

    memcpy(new_mdl1->verts,div1.verts,sizeof(KMX_VERTS)*div1.n_verts);
    memcpy(new_mdl1->norms,div1.norms,sizeof(KMX_NORMS)*div1.n_norms);
    Kmx_Prims_Move(new_mdl1->prims,div1.prims,div1.n_prims);

    memcpy(new_mdl2->verts,div2.verts,sizeof(KMX_VERTS)*div2.n_verts);
    memcpy(new_mdl2->norms,div2.norms,sizeof(KMX_NORMS)*div2.n_norms);
    Kmx_Prims_Move(new_mdl2->prims,div2.prims,div2.n_prims);

    for(i=0;i<def->n_x_models;i++){
	if(index==i) continue;
	memcpy(new_def->models+i,def->models+i,sizeof(KMX_MDL));
	def->models[i].verts=NULL;
	def->models[i].norms=NULL;
	def->models[i].prims=NULL;
    }

    Kmx_Free(def);
    MDU_Free(check_vid);

    div1.n_prims=old_n_prims;
    Kmx_MdlData_Free(&div1);

    div2.n_prims=old_n_prims;
    Kmx_MdlData_Free(&div2);

    new_def=CreateExtendModel(new_def,new_index);

    return new_def;
}

static KMX_DEF *SplitModel( def )
KMX_DEF *def;
{
    int i;
    int n_models;

    n_models=def->n_x_models;
    for(i=0;i<n_models;i++){
	if((def=CreateExtendModel(def,i))==NULL) return NULL;
    }
    return def;
}

/* 変換 */
static	KMX_DEF	*Convert( hP3DModel, lpp3dXYZScale )
HP3DMODEL	hP3DModel ;
LPP3DXYZ	lpp3dXYZScale ;
{
    KMX_DEF	*def ;
    int		nNo ;
    int i;

    /* ここに拡張モデルのポインタが格納される */    
    P3DModel( hP3DModel )->lParam = 0 ;
    nNo = 0 ; N_Textures = 0 ;
    CountModels( hP3DModel, NULL, &nNo ) ;

    D( printf( "%d Models\n", nNo ) ) ;

    def = Kmx_Alloc(nNo,nNo);
    if ( def == NULL ) return NULL ;

    /* オブジェクト変換 */
    nNo = 0 ;

    /* PATH 1 (体の構造部取得) */
    if ( ConvertObject( def, hP3DModel, NULL, lpp3dXYZScale, &nNo, 0 ) < 0 ){
	Kmx_Free(def);
	return NULL ;
    }

    /* PATH 2 (顔の構造部取得) */
    if ( ConvertObject( def, hP3DModel, NULL, lpp3dXYZScale, &nNo, 1 ) < 0 ){
	Kmx_Free(def);
	return NULL ;
    }

    /* PATH 3 (その他の構造部取得) */
    for(i=2;i<8;i++){
	if ( ConvertObject( def, hP3DModel, NULL, lpp3dXYZScale, &nNo, i ) < 0 ){
	    Kmx_Free(def);
	    return NULL ;
	}
    }

    ConvertObject2( hP3DModel, NULL ) ;

    D( printf( "CreateObject OK\n" ) ; ) ;
    
    /* バウンディング設定 */
    if(!SetBoundingBox(def)){
	Kmx_Free(def);
	return NULL ;
    }

#if 0
    if ( SkeltonModel == 0 ) {
	int i;

	/* ０番モデルの位置をモデル全体の原点とする。*/
	def->tx=def->models[ 0 ].tx;
	def->ty=def->models[ 0 ].ty;
	def->tz=def->models[ 0 ].tz;

	for(i=0;i<def->n_x_models;i++){
	    if(def->models[i].parent==-1){
		def->models[i].tx-=def->tx;
		def->models[i].ty-=def->ty;
		def->models[i].tz-=def->tz;
	    }
	}

	SetModelBoundingBox( def ) ;	/* メッシュモデル */
    }
    else {
	def->tx=0.0f;
	def->ty=0.0f;
	def->tz=0.0f;

	/* ０番モデルの位置を原点にする */
	def->models[ 0 ].tx = 0.0F ;
	def->models[ 0 ].ty = 0.0F ;
	def->models[ 0 ].tz = 0.0F ;

	SetModelBoundingBox2( def ) ;	/* スケルトンモデル */
    }
#else
    {
	int i;

	/* ０番モデルの位置をモデル全体の原点とする。*/
	def->tx=def->models[ 0 ].tx;
	def->ty=def->models[ 0 ].ty;
	def->tz=def->models[ 0 ].tz;

	for(i=0;i<def->n_x_models;i++){
	    if(def->models[i].parent==-1){
		def->models[i].tx-=def->tx;
		def->models[i].ty-=def->ty;
		def->models[i].tz-=def->tz;
	    }
	}
    }
    if( SkeltonModel == 0 ) SetModelBoundingBox( def ) ;	/* メッシュモデル */
    else{

#if 0
	def->tx=0.0f;
	def->ty=0.0f;
	def->tz=0.0f;
#endif

	SetModelBoundingBox2( def ) ;				/* スケルトンモデル */
    }
#endif

    D( printf( "SetBounding OK\n" ) ; ) ;

    /* 頂点数、法線数をチェックし、もし限界を越えているようであれば、
       拡張モデルを作成する */
    if ( ( def = SplitModel( def, hP3DModel, NULL ) ) == NULL ) return NULL ;

    D( printf( "SplitMode OK\n" ) ; ) ;
    
    return def ;
}

/*----------------------------------------------------------------------*/

/* 誤差をなくすために、頂点座標を整数にしてしまう */

#define	ADJUST	(0.5F) 

static void ModelVertsInt( def )
KMX_DEF	*def ;
{
    int		i, j, n_models ;
    KMX_VERTS	*verts ;
    FVECTOR	*trans ;
    IVECTOR	*itrans ;
    KMX_MDL	*mdl ;
    float	tx, ty, tz ;
    int		itx, ity, itz, vx, vy, vz ;

    mdl = def->models ;
    n_models = def->n_x_models ;
    tx = ty = tz = 0.0F ;
    itx = ity = itz = 0 ;
    trans = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * n_models ) ;
    itrans = ( IVECTOR * )MDU_Alloc( sizeof( IVECTOR ) * n_models ) ;

    /* 一度全部グローバル座標にする */
    for ( i = 0; i < n_models; i ++ ) {
	trans[ i ].vx = mdl->tx ; 
	trans[ i ].vy = mdl->ty ; 
	trans[ i ].vz = mdl->tz ; 
	if ( mdl->parent != -1 ) {
	    trans[ i ].vx += trans[ mdl->parent ].vx ;
	    trans[ i ].vy += trans[ mdl->parent ].vy ;
	    trans[ i ].vz += trans[ mdl->parent ].vz ;
	}
	tx = trans[ i ].vx ;
	ty = trans[ i ].vy ;
	tz = trans[ i ].vz ;

	verts = mdl->verts ;
	for ( j = 0; j < mdl->n_verts; j ++ ) {
	    verts->v.vx += tx ;
	    verts->v.vy += ty ;
	    verts->v.vz += tz ;
	    verts ++ ;
	}
	mdl ++ ;
    }

    /* ローカル座標に戻す時に整数化 */
    mdl = def->models ;  
    for ( i = 0; i < n_models; i ++ ) {
	itrans[ i ].vx = ( int )floor( (double)(mdl->tx+ADJUST) ) ;
	itrans[ i ].vy = ( int )floor( (double)(mdl->ty+ADJUST) ) ;
	itrans[ i ].vz = ( int )floor( (double)(mdl->tz+ADJUST) ) ;
	mdl->tx = ( float )itrans[ i ].vx ;
	mdl->ty = ( float )itrans[ i ].vy ;
	mdl->tz = ( float )itrans[ i ].vz ;
	if ( mdl->parent != -1 ) {
	    itrans[ i ].vx += itrans[ mdl->parent ].vx ;
	    itrans[ i ].vy += itrans[ mdl->parent ].vy ;
	    itrans[ i ].vz += itrans[ mdl->parent ].vz ;
	}
	itx = itrans[ i ].vx ;
	ity = itrans[ i ].vy ;
	itz = itrans[ i ].vz ;

	verts = mdl->verts ;
	for ( j = 0; j < mdl->n_verts; j ++ ) {
	    vx = ( int )floor( (double)(verts->v.vx + ADJUST) ) ;
	    vy = ( int )floor( (double)(verts->v.vy + ADJUST) ) ;
	    vz = ( int )floor( (double)(verts->v.vz + ADJUST) ) ;
	    verts->v.vx = ( float )( vx - itx ) ;
	    verts->v.vy = ( float )( vy - ity ) ;
	    verts->v.vz = ( float )( vz - itz ) ;
	    verts ++ ;
	}
	mdl ++ ;
    }    
    MDU_Free( trans ) ;
    MDU_Free( itrans ) ;
}

/*----------------------------------------------------------------------*/

KMX_DEF		*MDU_Mdl2Kmx( name, scale, hP3DModel, skel )
LPCSTR		name ;
LPP3DXYZ	scale ;
HP3DMODEL	hP3DModel ;
int		skel ;
{
    KMX_DEF	*def ;
    int i;

    SkeltonModel = skel ;

    D( printf( ".mdl -> .kmx conversion start\n" ) ; ) ;
    def = NULL;

    def = Convert( hP3DModel, scale ) ;
    if ( def == NULL ) return NULL ;

    /* テクスチャ名を移す */
    if((def->texs=Kmx_Texs_Alloc(N_Textures))==NULL){
	Kmx_Free(def);
	return NULL;
    }
    for(i=0;i<N_Textures;i++){
	if(((def->texs+i)->name=Kmx_Tex_Set(Textures[i]))==NULL){
	    Kmx_Free(def);
	    return NULL;
	}
	(def->texs+i)->alpha=TexAlphaAverage[i];

	// printf("Tex = %s %f\n",(def->texs+i)->name,(def->texs+i)->alpha);
    }
    def->n_texs=N_Textures;

#if 0
    if ( SkeltonModel ) ModelVertsInt( def ) ;
#else
    ModelVertsInt( def ) ;
#endif

    return def ;
}
