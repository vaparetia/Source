/*
   Mdl2Evf.c

   mdl -> evf 変換モジュール for linux

   by M.Sonoyama 1999.Sep.～ 
   Remade by K.Kano , 3/1/2000

   $Id: Mdl2Evf.c,v 1.17 2002/08/26 11:22:09 usr01363 Exp $

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
#include "MDU_tex.h"

#include "block.h"
#include "ToStrip.h"


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

/* スケルトン、メッシュの数を数える
   kms,evmの自動版別用 */
void CountMesh( HP3DMODEL  hP3DModel, HP3DOBJECT hP3DObject, int *skel, int *mesh )
{
    HP3DOBJECT 	hP3DObjectL ;

    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL ) return;

    do {
	(*skel)++;
	if(P3DObject(hP3DObjectL)->lpP3DPolygon!=NULL) (*mesh)++;
	CountMesh( hP3DModel, hP3DObjectL, skel, mesh ) ;
    } while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
}

/*----------------------------------------------------------------------*/

/* テクスチャ名格納用 */
extern	char	Textures[ MAX_TEXTURES ][ MAX_NAME_LEN ] ;
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


static FVECTOR mesh_xyz;
static int mesh_flag=0;
static int *conv_objid;
static int n_conv_objid;

/* IDが、必ず１から順番についているとは限らないので、このような関数を作った。
   IDに関しての説明が特に無かった。*/
static int SearchConvObjId(int id)
{
    int i;

    for(i=0;i<n_conv_objid;i++){
	if(*(conv_objid+i)==id) return i;
    }
    printf("Wrong Object ID : %d\n",id);
    return -1;
}

static int PtnMatch(char *a,char *b)
{
    while(*b!='\0'){
	if(*a!=*b) return 0;
	a++; b++;
    }
    return 1;
}

int SelectPath(char *name)
{
    char localname[256];

    GetLocalObjName( localname, name ) ;

#if 1
    if(PtnMatch(localname,"SKL_F_")) return 1;
    else if(PtnMatch(localname,"SKL_E_")) return 2;
    else if(PtnMatch(localname,"SKL_E0_")) return 3;
    else if(PtnMatch(localname,"SKL_E1_")) return 4;
    else if(PtnMatch(localname,"SKL_E2_")) return 5;
    else if(PtnMatch(localname,"SKL_E3_")) return 6;
    else if(PtnMatch(localname,"SKL_")) return 0;
    else return 7;
#else
    if(PtnMatch(localname,"SKL_")) return 0;
    else return 1;
#endif
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
	if(P3DObject(hP3DObjectL)->lpP3DPolygon==NULL){
	    ( *lpnNo ) ++ ;
	}
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
/* -------------------------------------------------------------------------- */


#define TRIANGLE	3


/* ポリゴンデータ変換 */
static	int	ConvertPolygon( mdl, hP3DModel, hP3DObject, lpP3DPolygon )
EVF_MESH	*mdl ;
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
    P3DPOLYGONNODE   	p3dpolygonnode[ TRIANGLE ] ;
    LPP3DPOLYGONNODE 	lpP3DPolygonNode ;
    LPP3DTEXTURE     	lpP3DTexture ;
    EVF_PRIMS		*new_prims;


    p3dpolygon = *lpP3DPolygon ;
    if( p3dpolygon.nPolygonNodeCount > TRIANGLE ) {	/* ポリゴンを分割 */
	p3dpolygon.nPolygonNodeCount -- ;
	p3dpolygon.lpP3DPolygonNode  ++ ;
	if( ConvertPolygon( mdl, hP3DModel, hP3DObject, &p3dpolygon ) == 0 )
	    return 0;
	p3dpolygon = *lpP3DPolygon ;
	p3dpolygon.nPolygonNodeCount = TRIANGLE ;
	p3dpolygon.lpP3DPolygonNode  = p3dpolygonnode ;
	p3dpolygonnode[ 0 ] = *( lpP3DPolygon->lpP3DPolygonNode + 0 ) ;
	p3dpolygonnode[ 1 ] = *( lpP3DPolygon->lpP3DPolygonNode + 1 ) ;
	p3dpolygonnode[ 2 ] = *( lpP3DPolygon->lpP3DPolygonNode
				 + ( lpP3DPolygon->nPolygonNodeCount - 1 ) ) ;
    }

    /* ポリゴン領域確保 */
    if((new_prims=Evf_Prims_Alloc(mdl->n_prims+1))==NULL) return 0;
    if(mdl->prims!=NULL){
	Evf_Prims_Move(new_prims,mdl->prims,mdl->n_prims);
	Evf_Prims_Free(mdl->prims,mdl->n_prims);
    }
    mdl->prims=new_prims;

    /* 新規確保した位置 */
    new_prims+=mdl->n_prims;
    if(!Evf_PrimData_Alloc(new_prims,p3dpolygon.nPolygonNodeCount,
			   lpP3DPolygon->nTextureCount)){

	return 0;
    }
    mdl->n_prims ++ ;

    /* 頂点、法線の設定 */
    for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
	 i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {

	/* PlayStation は右回りで座標を構築する */
	*(new_prims->vid + p3dpolygon.nPolygonNodeCount - i - 1 ) = lpP3DPolygonNode->nVertex ;
	*(new_prims->nid + p3dpolygon.nPolygonNodeCount - i - 1 ) = lpP3DPolygonNode->nNormal ;
    }

    for(j=0;j<lpP3DPolygon->nTextureCount;j++){

	/* テクスチャの存在のチェック */
	if( ( ( lpP3DTexture = 
		P3DFindTextureID( hP3DObject, *(lpP3DPolygon->lpnTextureID + j ) ) ) == NULL )
	    || ( ( hP3DPicture = 
		   P3DFindPictureID( hP3DModel, lpP3DTexture->nPictureID ) ) == NULL ) )
	    return 0 ;

	/* UV の中心 */
	dTxtU = 0.0 ;
	dTxtV = 0.0 ;
	for( i = 0, lpP3DPolygonNode = p3dpolygon.lpP3DPolygonNode ;
	     i < p3dpolygon.nPolygonNodeCount ; i ++, lpP3DPolygonNode ++ ) {

#if 0
	    dTxtU += ( lpP3DPolygonNode->lpP3DUVCalc + j )->u ;
	    dTxtV += ( lpP3DPolygonNode->lpP3DUVCalc + j )->v ;
#else
	    dTxtU += ( lpP3DPolygonNode->lpP3DUV + j )->u ;
	    dTxtV += ( lpP3DPolygonNode->lpP3DUV + j )->v ;
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
#else
	    p3duv.u = ( lpP3DPolygonNode->lpP3DUV + j )->u - dTxtU ;
	    p3duv.v = ( lpP3DPolygonNode->lpP3DUV + j )->v - dTxtV ;
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

	    /* 仮 */
	    ( new_prims->uvs
	      + p3dpolygon.nPolygonNodeCount * j
	      + p3dpolygon.nPolygonNodeCount - i - 1 )->w = 1.0f;
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


/* メッシュ情報の変換 */
static int ConvertMeshSub(EVF_DEF *def,HP3DMODEL hP3DModel,HP3DOBJECT hP3DObjectL,
			  LPP3DXYZ lpp3dXYZScale)
{
    int          	i,j ;
    double       	dCalc ;
    P3DXYZ       	p3dXYZ ;
    LPP3DVERTEX  	lpP3DVertex ;
    LPP3DNORMAL  	lpP3DNormal ;
    LPP3DPOLYGON 	lpP3DPolygon ;
    EVF_MESH		*mesh;
    EVF_VERTS		*verts;
    EVF_NORMS		*norms;
    MT_VEC	v1 ;
    MT_MAT	m1 ;


    if(mesh_flag==1){
	printf("Error : Model has 2 meshes.\n");
	return 0;
    }
    mesh_flag=1;


    mesh=&(def->mesh);

    printf( "mov %f %f %f\n", P3DObject( hP3DObjectL )->p3dXYZMove.x,
	    P3DObject( hP3DObjectL )->p3dXYZMove.y,
	    P3DObject( hP3DObjectL )->p3dXYZMove.z ) ;

    printf( "rot %f %f %f\n", P3DObject( hP3DObjectL )->p3dXYZRotate.x,
	    P3DObject( hP3DObjectL )->p3dXYZRotate.y,
	    P3DObject( hP3DObjectL )->p3dXYZRotate.z ) ;

    mesh_xyz.vx=P3DObject( hP3DObjectL )->p3dXYZMove.x * lpp3dXYZScale->x;
    mesh_xyz.vy=P3DObject( hP3DObjectL )->p3dXYZMove.y * lpp3dXYZScale->x;
    mesh_xyz.vz=P3DObject( hP3DObjectL )->p3dXYZMove.z * lpp3dXYZScale->x;


    mesh->n_verts = P3DObject( hP3DObjectL )->nVertexCount ;
    mesh->n_norms = P3DObject( hP3DObjectL )->nNormalCount ;

    /* 頂点・法線データ領域確保 */
    if((verts=mesh->verts=Evf_Verts_Alloc(mesh->n_verts))==NULL) return 0;
    if((norms=mesh->norms=Evf_Norms_Alloc(mesh->n_norms))==NULL) return 0;

    v1.v[ 0 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.x ;
    v1.v[ 1 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.y ;
    v1.v[ 2 ] = P3DObject( hP3DObjectL )->p3dXYZRotate.z ;
    v1.v[ 0 ] = v1.v[ 0 ] * 3.141592653589793 / 180.0 ;
    v1.v[ 1 ] = v1.v[ 1 ] * 3.141592653589793 / 180.0 ;
    v1.v[ 2 ] = v1.v[ 2 ] * 3.141592653589793 / 180.0 ;
    RotMatZYX( &v1, &m1 ) ;

    /* 頂点データ変換 */
    for( i = 0, lpP3DVertex = P3DObject( hP3DObjectL )->lpP3DVertex;
	 i < mesh->n_verts; i ++, lpP3DVertex++, verts ++ ) {

	v1.v[ 0 ] = lpP3DVertex->p3dXYZ.x ;
	v1.v[ 1 ] = lpP3DVertex->p3dXYZ.y ;
	v1.v[ 2 ] = lpP3DVertex->p3dXYZ.z ;
	MulMatVec( &m1, &v1, &v1 ) ;
	verts->v.vx = ( float )( v1.v[ 0 ] * lpp3dXYZScale->x ) ;
	verts->v.vy = ( float )( v1.v[ 1 ] * lpp3dXYZScale->y ) ;
	verts->v.vz = ( float )( v1.v[ 2 ] * lpp3dXYZScale->z ) ;

	/* 頂点の追加情報を取ってくる */
	GetVertexUserdata( lpP3DVertex, &(verts->vert_usrdata) );

	verts->envs=NULL;
	verts->n_envs=lpP3DVertex->nWeightCount;

	if(lpP3DVertex->nWeightCount>0){
	    if((verts->envs=Evf_Envs_Alloc(lpP3DVertex->nWeightCount))==NULL) return 0;

	    for(j=0;j<lpP3DVertex->nWeightCount;j++){
		(verts->envs+j)->objid=SearchConvObjId((lpP3DVertex->lpP3DWeight+j)->nObjectID);
		(verts->envs+j)->env=(float)(lpP3DVertex->lpP3DWeight+j)->dWeight;
	    }
	}
    }

    /* 法線データをコピー */
    for( i = 0, lpP3DNormal = P3DObject(hP3DObjectL)->lpP3DNormal;
	 i < mesh->n_norms; i ++, lpP3DNormal ++, norms ++) {

	/* 法線合計値を1.0にして内側を向かせる */

	p3dXYZ = lpP3DNormal->p3dXYZ ;

	v1.v[ 0 ] = p3dXYZ.x ;
	v1.v[ 1 ] = p3dXYZ.y ;
	v1.v[ 2 ] = p3dXYZ.z ;
	MulMatVec( &m1, &v1, &v1 ) ;

	// dCalc = sqrt( pow( p3dXYZ.x, 2 ) + pow( p3dXYZ.y, 2 ) + pow( p3dXYZ.z, 2 ) ) ;
	dCalc = sqrt(v1.v[0]*v1.v[0]+v1.v[1]*v1.v[1]+v1.v[2]*v1.v[2]) ;
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

    /* ポリゴンデータ変換 */
    for( i = 0, lpP3DPolygon = P3DObject(hP3DObjectL)->lpP3DPolygon;
	 i < P3DObject(hP3DObjectL)->nPolygonCount; i ++, lpP3DPolygon ++ ) {

	if( ConvertPolygon( mesh, hP3DModel, hP3DObjectL, lpP3DPolygon ) == 0 ) return 0;
    }

    return 1 ;
}

static int ConvertMesh(EVF_DEF *def,HP3DMODEL hP3DModel,HP3DOBJECT hP3DObject,
		       LPP3DXYZ lpp3dXYZScale)
{
    HP3DOBJECT   	hP3DObjectL ;

    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL ) return 1 ;
    
    do {
	if(P3DObject(hP3DObjectL)->lpP3DPolygon!=NULL){
	    if(ConvertMeshSub(def,hP3DModel,hP3DObjectL,lpp3dXYZScale)==0) return 0;
	}
	if ( ConvertMesh( def, hP3DModel, hP3DObjectL, lpp3dXYZScale ) == 0 ) return 0 ;

    } while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;

    return 1 ;
}


/* オブジェクト変換 */
static	int	ConvertObject( def, hP3DModel, hP3DObject, lpp3dXYZScale, lpnNo, path_num )
EVF_DEF		*def ;
HP3DMODEL	hP3DModel ;
HP3DOBJECT	hP3DObject ;
LPP3DXYZ	lpp3dXYZScale ;
int		*lpnNo ;
int		path_num;
{
    HP3DOBJECT   	hP3DObjectL ;
    EVF_SKEL		*mdl ;

    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL )
	return 1 ;
    
    do {
	if(P3DObject(hP3DObjectL)->lpP3DPolygon!=NULL) goto next;

	if(SelectPath(P3DObject(hP3DObjectL)->szName)!=path_num) goto next;

	/* モデル変換 */
	if(*lpnNo>=n_conv_objid){
	    printf("Error\n");
	}

	mdl = def->skeletons + *lpnNo ;

	conv_objid[*lpnNo]=P3DObject(hP3DObjectL)->nID;

	P3DObject(hP3DObjectL)->wParam = *lpnNo;	/* 連番 */
	P3DObject(hP3DObjectL)->lParam = (LPARAM)mdl ;

	mdl->parent = -1 ;

	printf( "mov %f %f %f\n", P3DObject( hP3DObjectL )->p3dXYZMove.x,
		P3DObject( hP3DObjectL )->p3dXYZMove.y,
		P3DObject( hP3DObjectL )->p3dXYZMove.z ) ;

	printf( "rot %f %f %f\n", P3DObject( hP3DObjectL )->p3dXYZRotate.x,
		P3DObject( hP3DObjectL )->p3dXYZRotate.y,
		P3DObject( hP3DObjectL )->p3dXYZRotate.z ) ;
        
	mdl->tx = ( float )( P3DObject( hP3DObjectL )->p3dXYZMove.x * lpp3dXYZScale->x ) ;
	mdl->ty = ( float )( P3DObject( hP3DObjectL )->p3dXYZMove.y * lpp3dXYZScale->y ) ;
	mdl->tz = ( float )( P3DObject( hP3DObjectL )->p3dXYZMove.z * lpp3dXYZScale->z ) ;

	( *lpnNo ) ++ ;

    next:
	if ( ConvertObject( def, hP3DModel, hP3DObjectL, lpp3dXYZScale,
			    lpnNo, path_num ) == 0 ){

	    return 0 ;
	}

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
    EVF_SKEL	*mdl ;

    if( ( hP3DObjectL = P3DGetFirstObject( hP3DModel, hP3DObject ) ) == NULL ) return;
    do {
	if(P3DObject(hP3DObjectL)->lpP3DPolygon==NULL){
	    mdl = ( EVF_SKEL * )P3DObject( hP3DObjectL )->lParam ;
	    if( hP3DObject != NULL ) mdl->parent = P3DObject( hP3DObject )->wParam ;
	}
	ConvertObject2( hP3DModel, hP3DObjectL ) ;
    } while( ( hP3DObjectL = P3DGetNextObject( hP3DObjectL ) ) != NULL ) ;
}


/* rt_tx,ty,tzメンバーの設定 */
void SetRootTxyz(EVF_DEF *def)
{
    EVF_SKEL *mdl;
    int i;

    mdl=def->skeletons;
    for(i=0;i<def->n_x_models;i++){
	int parent=(mdl+i)->parent;

	(mdl+i)->rt_tx=(mdl+i)->tx;
	(mdl+i)->rt_ty=(mdl+i)->ty;
	(mdl+i)->rt_tz=(mdl+i)->tz;

	if(parent!=-1){
	    (mdl+i)->rt_tx+=(mdl+parent)->rt_tx;
	    (mdl+i)->rt_ty+=(mdl+parent)->rt_ty;
	    (mdl+i)->rt_tz+=(mdl+parent)->rt_tz;
	}
    }
}

/* 仮でバウンディングボックスを設定 */
void SetBoundingBox(EVF_DEF *def)
{
    EVF_SKEL *mdl;
    int i,j;
    float max=0.0f;

    mdl=def->skeletons;
    for(i=0;i<def->mesh.n_verts;i++){
	EVF_VERTS *v=def->mesh.verts+i;

	for(j=0;j<v->n_envs;j++){
	    float lx,ly,lz;
	    int objid=(v->envs+j)->objid;
	    float tmax=0,lmax=0;

	    lx=v->v.vx-(mdl+objid)->rt_tx;
	    ly=v->v.vy-(mdl+objid)->rt_ty;
	    lz=v->v.vz-(mdl+objid)->rt_tz;

	    lmax=__max(lmax,fabs(lx));
	    lmax=__max(lmax,fabs(ly));
	    lmax=__max(lmax,fabs(lz));

	    tmax=__max(tmax,fabs((mdl+objid)->tx));
	    tmax=__max(tmax,fabs((mdl+objid)->ty));
	    tmax=__max(tmax,fabs((mdl+objid)->tz));

	    tmax+=lmax;

	    while((mdl+objid)->parent>=0){
		lmax=0;
		objid=(mdl+objid)->parent;

		lmax=__max(lmax,fabs((mdl+objid)->tx));
		lmax=__max(lmax,fabs((mdl+objid)->ty));
		lmax=__max(lmax,fabs((mdl+objid)->tz));

		tmax+=lmax;
	    }

	    max=__max(max,tmax);
	}
    }

    def->lx=def->ly=def->lz=-max;
    def->ux=def->uy=def->uz=max;
}

void MeshCoordinateConvert(EVF_DEF *def)
{
    /* スケルトンの０番の座標系を、強制的に原点にし、それに合わせて、
       メッシュの座標系もずらす。*/
    FVECTOR t;
    int i;

    t.vx=mesh_xyz.vx-def->skeletons[0].tx;
    t.vy=mesh_xyz.vy-def->skeletons[0].ty;
    t.vz=mesh_xyz.vz-def->skeletons[0].tz;

    for(i=0;i<def->mesh.n_verts;i++){
	(def->mesh.verts+i)->v.vx+=t.vx;
	(def->mesh.verts+i)->v.vy+=t.vy;
	(def->mesh.verts+i)->v.vz+=t.vz;
    }

    def->skeletons[0].tx=0;
    def->skeletons[0].ty=0;
    def->skeletons[0].tz=0;
}

/*----------------------------------------------------------------------*/

/* 誤差が微妙に存在するようなので、移動値と頂点座標を1/16単位に直す */

#define	ADJUST	(0.5F) 

static void ModelVertsInt(EVF_DEF *def)
{
    IVECTOR t;
    int i;

    t.vx=(long)floor((double)(mesh_xyz.vx*16.0f+ADJUST));
    t.vy=(long)floor((double)(mesh_xyz.vy*16.0f+ADJUST));
    t.vz=(long)floor((double)(mesh_xyz.vz*16.0f+ADJUST));
    mesh_xyz.vx=(float)(t.vx)/16.0f;
    mesh_xyz.vy=(float)(t.vy)/16.0f;
    mesh_xyz.vz=(float)(t.vz)/16.0f;
    for(i=0;i<def->n_x_models;i++){
	t.vx=(long)floor((double)(def->skeletons[i].tx*16.0f+ADJUST));
	t.vy=(long)floor((double)(def->skeletons[i].ty*16.0f+ADJUST));
	t.vz=(long)floor((double)(def->skeletons[i].tz*16.0f+ADJUST));
	def->skeletons[i].tx=(float)(t.vx)/16.0f;
	def->skeletons[i].ty=(float)(t.vy)/16.0f;
	def->skeletons[i].tz=(float)(t.vz)/16.0f;
    }
    for(i=0;i<def->mesh.n_verts;i++){
	t.vx=(long)floor((double)((def->mesh.verts+i)->v.vx*16.0f+ADJUST));
	t.vy=(long)floor((double)((def->mesh.verts+i)->v.vy*16.0f+ADJUST));
	t.vz=(long)floor((double)((def->mesh.verts+i)->v.vz*16.0f+ADJUST));
	(def->mesh.verts+i)->v.vx=(float)(t.vx)/16.0f;
	(def->mesh.verts+i)->v.vy=(float)(t.vy)/16.0f;
	(def->mesh.verts+i)->v.vz=(float)(t.vz)/16.0f;
    }
}

/*----------------------------------------------------------------------*/

static int CheckMeshWeight(EVF_DEF *def)
{
    EVF_VERTS *verts=def->mesh.verts;
    EVF_PRIMS *prims=def->mesh.prims;
    int i,j,k,l;
    int ans=1;

    for(i=0;i<def->mesh.n_verts;i++,verts++){
	/* ウェイト値が付いていない頂点をチェックし表示する。*/
	if(verts->n_envs==0){
	    printf("Error : No Weights !! : ( %5.4f %5.4f %5.4f )\n",
		   verts->v.vx,verts->v.vy,verts->v.vz);
	    ans=0;
	}

#if 1  /* TYPE_VER2 */
	/* ウェイト値が多過ぎる頂点をチェックし表示する */
	if(verts->n_envs>MAX_VERTEX_WEIGHTS){
	    printf("Error : Over %d Weights !! : ( %5.4f %5.4f %5.4f )\n",
		   MAX_VERTEX_WEIGHTS,verts->v.vx,verts->v.vy,verts->v.vz);
	    ans=0;
	}
#endif

    }

    verts=def->mesh.verts;

    /* 各ポリゴンのウェイト情報をチェックし、制限を越えるものを表示する */
    for(i=0;i<def->mesh.n_prims;i++,prims++){
	int objids[MAX_OBJIDS];
	int n_objids=0;

	for(j=0;j<prims->n_id;j++){
	    int vid=*(prims->vid+j);
	    EVF_VERTS *tvert=verts+vid;

	    for(k=0;k<tvert->n_envs;k++){
		int flag=0;

		for(l=0;l<n_objids;l++){
		    if((tvert->envs+k)->objid==objids[l]){
			flag=1;
			break;
		    }
		    if(!flag){
			if(n_objids>=MAX_OBJIDS){
			    n_objids++;
			    goto forbreak;
			}
			else{
			    objids[n_objids]=(tvert->envs+k)->objid;
			    n_objids++;
			}
		    }
		}
	    }
	}

forbreak:
	if(n_objids>MAX_OBJIDS){
	    /* ウェイト値が多く付き過ぎている場合 */
	    printf("Error : Over Weights !! : ");

	    for(j=0;j<prims->n_id;j++){
		int vid=*(prims->vid+j);
		EVF_VERTS *tvert=verts+vid;

		if(j==0){
		    printf("( %5.4f %5.4f %5.4f )",tvert->v.vx,tvert->v.vy,tvert->v.vz);
		}
		else{
		    printf(" - ( %5.4f %5.4f %5.4f )",tvert->v.vx,tvert->v.vy,tvert->v.vz);
		}
	    }

	    printf("\n");

	    ans=0;
	}
    }

    return ans;
}

/*----------------------------------------------------------------------*/

/* 変換 */
static EVF_DEF *Convert( hP3DModel, lpp3dXYZScale )
HP3DMODEL hP3DModel ;
LPP3DXYZ lpp3dXYZScale ;
{
    EVF_DEF	*def ;
    int		nNo ;
    int i;

    /* ここに拡張モデルのポインタが格納される */    
    P3DModel( hP3DModel )->lParam = 0 ;
    nNo = 0 ; N_Textures = 0 ;
    CountModels( hP3DModel, NULL, &nNo ) ;

    D( printf( "%d Models\n", nNo ) ) ;

    def = Evf_Alloc(nNo,nNo);
    if ( def == NULL ) return NULL ;

    if((conv_objid=(int *)MDU_Alloc(sizeof(int)*nNo))==NULL){
	Evf_Free(def);
	return NULL;
    }
    n_conv_objid=nNo;


    /* オブジェクト変換 */
    nNo = 0 ;

    /* PATH 1 (体の構造部取得) */
    if ( !ConvertObject( def, hP3DModel, NULL, lpp3dXYZScale, &nNo, 0 ) ){
	Evf_Free(def);
	return NULL ;
    }

    /* PATH 2 (顔の構造部取得) */
    if ( !ConvertObject( def, hP3DModel, NULL, lpp3dXYZScale, &nNo, 1 ) ){
	Evf_Free(def);
	return NULL ;
    }
    def->n_models=nNo;

    /* PATH 3 (その他の構造部取得) */
    for(i=2;i<8;i++){
	if ( !ConvertObject( def, hP3DModel, NULL, lpp3dXYZScale, &nNo, i ) ){
	    Evf_Free(def);
	    return NULL ;
	}
    }

    ConvertObject2( hP3DModel, NULL ) ;

    D( printf( "CreateObject OK\n" ) ; ) ;

    mesh_flag=0;

    if(!ConvertMesh(def,hP3DModel,NULL,lpp3dXYZScale)){
	Evf_Free(def);
	return NULL ;
    }

    D( printf( "CreateMesh OK\n" ) ; ) ;

#if 1
    if(!CheckMeshWeight(def)){
	Evf_Free(def);
	return NULL ;
    }

    D( printf( "CheckMeshWeight Finished\n" ) ; ) ;
#endif

    /* メッシュやシフト値を、1/16単位にする */
    ModelVertsInt(def);

    /* メッシュの座標系をスケルトンに合わせる。*/
    MeshCoordinateConvert(def);

    /* バウンディング設定 */
    SetRootTxyz(def);
    SetBoundingBox(def);

    D( printf( "SetBounding OK\n" ) ; ) ;

    MDU_Free(conv_objid);

    return def ;
}

/*----------------------------------------------------------------------*/

EVF_DEF		*MDU_Mdl2Evf( name, scale, hP3DModel )
LPCSTR		name ;
LPP3DXYZ	scale ;
HP3DMODEL	hP3DModel ;
{
    EVF_DEF *def ;
    int i;

    D( printf( ".mdl -> .evf conversion start\n" ) ; ) ;
    def = NULL;

    def = Convert( hP3DModel, scale ) ;
    if ( def == NULL ) return NULL ;

    /* テクスチャ名を移す */
    if((def->texs=Evf_Texs_Alloc(N_Textures))==NULL){
	Evf_Free(def);
	return NULL;
    }
    for(i=0;i<N_Textures;i++){
	if(((def->texs+i)->name=Evf_Tex_Set(Textures[i]))==NULL){
	    Evf_Free(def);
	    return NULL;
	}
	(def->texs+i)->alpha=TexAlphaAverage[i];
    }

    return def ;
}
