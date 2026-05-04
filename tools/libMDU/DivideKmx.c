/*
   DivideKmx.c

   モデルユーティリティ for MGS2 on linux

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 3/1/2000

   $Id: DivideKmx.c,v 1.18 2002/08/26 11:22:09 usr01363 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

/*----------------------------------------------------------

   モデル分割関数 
   KMX_DEF		*MDU_DivideKmx( orgdef2, div, vert )
   KMX_DEF		*orgdef2 ;
   SVECTOR		*div ; 		 分割数 
   int          vert ; 		 パケットの最大頂点数

   属性による分割 
   KMX_DEF		*MDU_SplitKmx( orgdef2 )
   KMX_DEF		*orgdef2 ;
   
------------------------------------------------------------*/

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

#include "MDU_util.h"
#include "MDU_mdl.h"
#include "MDU_tex.h"

#include "ToStrip.h"


/* モデル分割（関節モデルは不可） */
/* 元の頂点情報を壊します */

#define TYPES_MAX	256

#define ___MAX(x, y) (((x) > (y)) ? (x) : (y))
#define ___MIN(x, y) (((x) < (y)) ? (x) : (y))

static	int	N_Types;
static	int	Type[ TYPES_MAX ] ;
static	SVECTOR	*Div ;
static	FVECTOR	BoundMin, BoundMax, BlockSize ;
static	int min_verts;


/*--------------------------------------------------------------*/


/* 全体バウンディング、分割オブジェ中心、分割オブジェ数
   などを取得 */
static int GetDivideParam( def ) 
KMX_DEF *def;
{
    KMX_MDL	*mdl ;
    int		n_x_models ;
    int		i, j, type ; 

    N_Types = 0 ;

    BoundMin.vx = def->lx;
    BoundMin.vy = def->ly;
    BoundMin.vz = def->lz;
    BoundMax.vx = def->ux;
    BoundMax.vy = def->uy;
    BoundMax.vz = def->uz;

    n_x_models = def->n_x_models ;
    mdl = def->models ;

    /* 親子関係のチェック */
    for(i=0;i<n_x_models;i++){
		if((mdl+i)->parent!=-1){
			int parent=(mdl+i)->parent;

			if((mdl+i)->type & DG_TYPE_EXTEND){
				(mdl+i)->tx=(mdl+parent)->tx;
				(mdl+i)->ty=(mdl+parent)->ty;
				(mdl+i)->tz=(mdl+parent)->tz;

				(mdl+i)->type &= ~DG_TYPE_EXTEND;
				(mdl+i)->parent=-1;
			}
			else{
				printf("Warning : Model has Parent ID. ( P: %d -> C: %d )\n",parent,i);

#if 1
				(mdl+i)->tx+=(mdl+parent)->tx;
				(mdl+i)->ty+=(mdl+parent)->ty;
				(mdl+i)->tz+=(mdl+parent)->tz;
				(mdl+i)->parent=-1;
#else
				return 0;
#endif
			}
		}
    }

    for ( i = 0; i < n_x_models; i ++ ) {
		/* タイプチェック */
		type = mdl->type ;
		for ( j = 0; j < N_Types; j ++ ) {
			if ( type == Type[ j ] ) break ;
		}
		if ( j == N_Types ) {
			Type[ j ] = type ;
			N_Types ++ ;
		}
		mdl ++ ;
    }

    /* タイプ：オーバーレイを後ろに持ってくる */
#if 1
    {	
		int	type[ TYPES_MAX ], n ;
	
		n = 0 ;

#if 1
		for ( i = 0; i < N_Types; i ++ ) {
			if ( !( Type[ i ] & ( DG_TYPE_OVERLAY0 | DG_TYPE_OVERLAY1 | 
								  DG_TYPE_OVERLAY2 ) ) ) {
				type[ n ] = Type[ i ] ; n ++ ;
			}
		}
#else
		for ( i = 0; i < N_Types; i ++ ) {
			if ( !( Type[ i ] & ( DG_TYPE_TRANS | DG_TYPE_OVERLAY0 | DG_TYPE_OVERLAY1 | 
								  DG_TYPE_OVERLAY2 ) ) ) {
				type[ n ] = Type[ i ] ; n ++ ;
			}
		}
#endif
		for ( i = 0; i < N_Types; i ++ ) {
			if ( Type[ i ] & DG_TYPE_OVERLAY0 ) {
				type[ n ] = Type[ i ] ; n ++ ;
			}
		}
		for ( i = 0; i < N_Types; i ++ ) {
			if ( Type[ i ] & DG_TYPE_OVERLAY1 ) {
				type[ n ] = Type[ i ] ; n ++ ;
			}
		}
		for ( i = 0; i < N_Types; i ++ ) {
			if ( Type[ i ] & DG_TYPE_OVERLAY2 ) {
				type[ n ] = Type[ i ] ; n ++ ;
			}
		}
#if 0
		for ( i = 0; i < N_Types; i ++ ) {
			if ( Type[ i ] & DG_TYPE_TRANS ) {
				type[ n ] = Type[ i ] ; n ++ ;
			}
		}
#endif
		for ( i = 0; i < N_Types; i ++ ){
			Type[ i ] = type[ i ] ;
		}
    }
#endif

    printf( "Bound %f %f %f - ", BoundMin.vx, BoundMin.vy, BoundMin.vz ) ;
    printf( "%f %f %f\n", BoundMax.vx, BoundMax.vy, BoundMax.vz ) ;

#if 1
    BlockSize.vx = ( float )Div->vx ;
    BlockSize.vy = ( float )Div->vy ;
    BlockSize.vz = ( float )Div->vz ;
    Div->vx = ( int )( ( BoundMax.vx - BoundMin.vx ) / ( float )Div->vx ) + 1 ;
    Div->vy = ( int )( ( BoundMax.vy - BoundMin.vy ) / ( float )Div->vy ) + 1 ;
    Div->vz = ( int )( ( BoundMax.vz - BoundMin.vz ) / ( float )Div->vz ) + 1 ;
    printf( "div %d %d %d\n", Div->vx, Div->vy, Div->vz ) ;
#else
    BlockSize.vx = ( BoundMax.vx - BoundMin.vx ) / ( float )Div->vx ;
    BlockSize.vy = ( BoundMax.vy - BoundMin.vy ) / ( float )Div->vy ;
    BlockSize.vz = ( BoundMax.vz - BoundMin.vz ) / ( float )Div->vz ;
#endif

    return 1;
}

/* ポリゴンがどのブロックに入るか調べる */
/* 各頂点の平均値を用いる */
static int CheckInsideBlock( mdl, n )
KMX_MDL *mdl ;
int n ;
{
    FVECTOR	v[ 4 ], c ;
    int		x, y, z ;
    int		res, n_models ;

    if((mdl->prims+n)->n_id==4){
		v[ 0 ] = ( mdl->verts + *((mdl->prims+n)->vid+0) )->v ;
		v[ 1 ] = ( mdl->verts + *((mdl->prims+n)->vid+1) )->v ; 
		v[ 2 ] = ( mdl->verts + *((mdl->prims+n)->vid+2) )->v ;
		v[ 3 ] = ( mdl->verts + *((mdl->prims+n)->vid+3) )->v ;

		c.vx = ( v[ 0 ].vx + v[ 1 ].vx + v[ 2 ].vx + v[ 3 ].vx ) / 4.0F ;
		c.vy = ( v[ 0 ].vy + v[ 1 ].vy + v[ 2 ].vy + v[ 3 ].vy ) / 4.0F ;
		c.vz = ( v[ 0 ].vz + v[ 1 ].vz + v[ 2 ].vz + v[ 3 ].vz ) / 4.0F ;
    }
    else{
		v[ 0 ] = ( mdl->verts + *((mdl->prims+n)->vid+0) )->v ;
		v[ 1 ] = ( mdl->verts + *((mdl->prims+n)->vid+1) )->v ; 
		v[ 2 ] = ( mdl->verts + *((mdl->prims+n)->vid+2) )->v ;

		c.vx = ( v[ 0 ].vx + v[ 1 ].vx + v[ 2 ].vx ) / 3.0F ;
		c.vy = ( v[ 0 ].vy + v[ 1 ].vy + v[ 2 ].vy ) / 3.0F ;
		c.vz = ( v[ 0 ].vz + v[ 1 ].vz + v[ 2 ].vz ) / 3.0F ;
    }

    c.vx+=mdl->tx;
    c.vy+=mdl->ty;
    c.vz+=mdl->tz;

    x = ( int )( ( c.vx - BoundMin.vx ) / BlockSize.vx ) ;
    y = ( int )( ( c.vy - BoundMin.vy ) / BlockSize.vy ) ;
    z = ( int )( ( c.vz - BoundMin.vz ) / BlockSize.vz ) ;

    n_models = Div->vx * Div->vy * Div->vz ;
    res = x + y * Div->vx + z * ( Div->vx * Div->vy ) ;

    if ( res < 0 ) res = 0 ;
    if ( res >= n_models ) res = n_models - 1 ;

    return res ;
}

/* ポリゴン振り分け */
static KMX_DEF *Distribute( def )
KMX_DEF *def;
{
    KMX_DEF *newdef;
    KMX_MDL *src,*dst;
    int n_x_models;
    int i;


    /* 振り分ける領域の確保 */
    n_x_models=N_Types*Div->vx*Div->vy*Div->vz;

#if 0
	printf("N_Types , n models = %d , %d\n",N_Types,n_x_models);
#endif

    if((newdef=Kmx_Alloc(n_x_models,n_x_models))==NULL){
		Kmx_Free(def);
		printf("Error : The memory for dividing model is too large.\n");
		return NULL;
    }

#if 0
    newdef->tx=def->tx;
    newdef->ty=def->ty;
    newdef->tz=def->tz;
#else
	{
		float cx,cy,cz;

		cx=(def->lx+def->ux)/2.0f;
		cy=(def->ly+def->uy)/2.0f;
		cz=(def->lz+def->uz)/2.0f;
		cx=floor(cx);
		cy=floor(cy);
		cz=floor(cz);

		newdef->tx=def->tx+cx;
		newdef->ty=def->ty+cy;
		newdef->tz=def->tz+cz;
	}
#endif
    newdef->lx=def->lx+def->tx-newdef->tx;
    newdef->ly=def->ly+def->ty-newdef->ty;
    newdef->lz=def->lz+def->tz-newdef->tz;
    newdef->ux=def->ux+def->tx-newdef->tx;
    newdef->uy=def->uy+def->ty-newdef->ty;
    newdef->uz=def->uz+def->tz-newdef->tz;

    newdef->n_texs=def->n_texs;
    newdef->texs=def->texs;

    def->n_texs=0;
    def->texs=NULL;

    /* モデルの基本設定 */
    {
		int j,k;
		int n,t;

		n=0;
		for ( t = 0; t < N_Types; t ++ ) {
			for ( i = 0; i < Div->vz; i ++ ) {
				for ( j = 0; j < Div->vy; j ++ ) {
					for ( k = 0; k < Div->vx; k ++ ) {
						newdef->models[n].type=Type[t];

#if 0
						newdef->models[n].tx
							=BoundMin.vx+BlockSize.vx*k+BlockSize.vx/2.0F;
						newdef->models[n].ty
							=BoundMin.vy+BlockSize.vy*j+BlockSize.vy/2.0F;
						newdef->models[n].tz
							=BoundMin.vz+BlockSize.vz*i+BlockSize.vz/2.0F;
#elif 0
						newdef->models[n].tx=def->models[0].tx;
						newdef->models[n].ty=def->models[0].ty;
						newdef->models[n].tz=def->models[0].tz;
#else
						newdef->models[n].tx=0.0f;
						newdef->models[n].ty=0.0f;
						newdef->models[n].tz=0.0f;
#endif

						n++;
					}
				}
			}
		}
    }

    /* プリミティブ領域が、それぞれ、どのくらい必要かをチェック */
    i=def->n_x_models;
    src=def->models;
    dst=newdef->models;
    while(i>0){
		int type_no;
		int index;
		int n_prims;

		for(type_no=0;type_no<N_Types;type_no++){
			if(src->type==Type[type_no]) break;
		}
		if(type_no==N_Types){
			printf( "?????? : 0x%08x\n",src->type );

			Kmx_Free(def);
			Kmx_Free(newdef);
			return NULL;
		}

		n_prims=src->n_prims;
		for(index=0;index<n_prims;index++){
			int n=CheckInsideBlock(src,index);
			n+=type_no*Div->vx*Div->vy*Div->vz;
			(dst+n)->n_prims++;
			(dst+n)->n_verts+=(src->prims+index)->n_id;
		}
		src++;
		i--;
    }

    /* 頂点、法線、プリミティブ領域の確保 */
    for(i=0;i<n_x_models;i++,dst++){
		int n_prims=dst->n_prims;
		int n_verts=dst->n_verts;

		if(n_verts!=0){
			if((dst->verts=Kmx_Verts_Alloc(n_verts))==NULL){
				Kmx_Free(def);
				Kmx_Free(newdef);
				return NULL;
			}
			dst->n_verts=0;

			if((dst->norms=Kmx_Norms_Alloc(n_verts))==NULL){
				Kmx_Free(def);
				Kmx_Free(newdef);
				return NULL;
			}
			dst->n_norms=0;
		}

		if(n_prims!=0){
			if((dst->prims=Kmx_Prims_Alloc(n_prims))==NULL){
				Kmx_Free(def);
				Kmx_Free(newdef);
				return NULL;
			}
			dst->n_prims=0;
		}
    }

    /* 実際に、頂点、法線、プリミティブを割り振る */
    i=def->n_x_models;
    src=def->models;
    dst=newdef->models;
    while(i>0){
		int type_no;
		int index;
		int n_prims;

		/* 進行チェック用 */
		printf(".");
		fflush(stdout);

		for(type_no=0;type_no<N_Types;type_no++){
			if(src->type==Type[type_no]) break;
		}
		if(type_no==N_Types){
			printf( "?????? : 0x%08x\n",src->type );

			Kmx_Free(def);
			Kmx_Free(newdef);
			return NULL;
		}

		n_prims=src->n_prims;
		for(index=0;index<n_prims;index++){
			KMX_PRIMS *src_prim,*dst_prim;
			KMX_VERTS *dst_vert;
			KMX_NORMS *dst_norm;
			int j;

			int n=CheckInsideBlock(src,index);

			n+=type_no*Div->vx*Div->vy*Div->vz;

			src_prim=src->prims+index;
			dst_prim=(dst+n)->prims+(dst+n)->n_prims;
			dst_vert=(dst+n)->verts+(dst+n)->n_verts;
			dst_norm=(dst+n)->norms+(dst+n)->n_norms;

			for(j=0;j<src_prim->n_id;j++){
				int vid=*(src_prim->vid+j);
				int nid=*(src_prim->nid+j);

				memcpy(dst_vert+j,src->verts+vid,sizeof(KMX_VERTS));
				memcpy(dst_norm+j,src->norms+nid,sizeof(KMX_NORMS));

				/* 座標系を変更 */
				(dst_vert+j)->v.vx+=(src->tx+def->tx);
				(dst_vert+j)->v.vy+=(src->ty+def->ty);
				(dst_vert+j)->v.vz+=(src->tz+def->tz);
				(dst_vert+j)->v.vx-=((dst+n)->tx+newdef->tx);
				(dst_vert+j)->v.vy-=((dst+n)->ty+newdef->ty);
				(dst_vert+j)->v.vz-=((dst+n)->tz+newdef->tz);

				*(src_prim->vid+j)=(dst+n)->n_verts+j;
				*(src_prim->nid+j)=(dst+n)->n_norms+j;
			}

			Kmx_Prim_Move(dst_prim,src_prim);

			(dst+n)->n_prims++;
			(dst+n)->n_verts+=dst_prim->n_id;
			(dst+n)->n_norms+=dst_prim->n_id;
		}
		src++;
		i--;
    }

    /* 進行チェック用 */
    printf("\n");

    Kmx_Free(def);

    /* 各モデルのバウンディングボックスの設定 */
    for(i=0;i<n_x_models;i++,dst++){
		KMX_VERTS *vert=dst->verts;
		int j;

		dst->lx=0.0f;
		dst->ly=0.0f;
		dst->lz=0.0f;
		dst->ux=0.0f;
		dst->uy=0.0f;
		dst->uz=0.0f;

		if(dst->n_verts>0){
			dst->lx=vert->v.vx;
			dst->ly=vert->v.vy;
			dst->lz=vert->v.vz;
			dst->ux=vert->v.vx;
			dst->uy=vert->v.vy;
			dst->uz=vert->v.vz;
		}

		vert++;
		for(j=1;j<dst->n_verts;j++,vert++){
			if(dst->lx>vert->v.vx) dst->lx=vert->v.vx;
			if(dst->ly>vert->v.vy) dst->ly=vert->v.vy;
			if(dst->lz>vert->v.vz) dst->lz=vert->v.vz;
			if(dst->ux<vert->v.vx) dst->ux=vert->v.vx;
			if(dst->uy<vert->v.vy) dst->uy=vert->v.vy;
			if(dst->uz<vert->v.vz) dst->uz=vert->v.vz;
		}
    }

    return newdef;
}

/* 各モデルの頂点、法線、プリミティブ領域の確保 */
static int Alloc_Verts_Norms_Prims( def )
KMX_DEF *def;
{
	int i, j;

	for(i=0;i<def->n_x_models;i++)
	{
		KMX_MDL *mdl = &def->models[i];

		j = mdl->n_verts;
		if(j!=0)
		{
			if(    ((mdl->verts = Kmx_Verts_Alloc(j))==NULL)
				|| ((mdl->norms = Kmx_Norms_Alloc(j))==NULL))
				return 0;
			mdl->n_verts = 0;
			mdl->n_norms = 0;
		}

		j = mdl->n_prims;
		if(j!=0)
		{
			if((mdl->prims = Kmx_Prims_Alloc(j))==NULL)
				return 0;
			mdl->n_prims = 0;
		}
	}
	return 1;
}

/* プリミティブが存在しないモデルを削除する */
static KMX_DEF *Remove_Null_Models( def )
KMX_DEF *def;
{
	int     i, j;
	int     n_x_models;
	KMX_DEF *newdef;

	n_x_models = 0;
	for(i=0;i<def->n_x_models;i++)
	{
		if(def->models[i].n_prims>0)
			n_x_models++;
	}

	if(n_x_models!=def->n_x_models)
	{
		if((newdef = Kmx_Alloc(n_x_models, n_x_models))==NULL)
		{
			Kmx_Free(def);
			return NULL;
		}

		newdef->tx = def->tx;
		newdef->ty = def->ty;
		newdef->tz = def->tz;
		newdef->lx = def->lx;
		newdef->ly = def->ly;
		newdef->lz = def->lz;
		newdef->ux = def->ux;
		newdef->uy = def->uy;
		newdef->uz = def->uz;

		newdef->n_texs = def->n_texs;
		newdef->texs   = def->texs;

		def->n_texs = 0;
		def->texs   = NULL;

		i = 0;
		for(j=0;j<def->n_x_models;j++)
		{
			if(def->models[j].n_prims>0)
			{
				Kmx_MdlData1_Move(&newdef->models[i], &def->models[j]);
				i++;
			}
		}
		Kmx_Free(def);
		def = newdef;
    }

	return def;
}

/* 各モデルのバウンディングボックスの設定 */
static void Set_Bounding_Box( def )
KMX_DEF *def;
{
	int i, j;

    for(i=0;i<def->n_x_models;i++)
	{
		KMX_MDL   *mdl  = &def->models[i];
		KMX_VERTS *vert = mdl->verts;

		mdl->lx=0.0f;
		mdl->ly=0.0f;
		mdl->lz=0.0f;
		mdl->ux=0.0f;
		mdl->uy=0.0f;
		mdl->uz=0.0f;

		if(mdl->n_verts>0)
		{
			mdl->lx=vert->v.vx;
			mdl->ly=vert->v.vy;
			mdl->lz=vert->v.vz;
			mdl->ux=vert->v.vx;
			mdl->uy=vert->v.vy;
			mdl->uz=vert->v.vz;
		}

		vert++;
		for(j=1;j<mdl->n_verts;j++,vert++)
		{
			if(mdl->lx>vert->v.vx) mdl->lx=vert->v.vx;
			if(mdl->ly>vert->v.vy) mdl->ly=vert->v.vy;
			if(mdl->lz>vert->v.vz) mdl->lz=vert->v.vz;
			if(mdl->ux<vert->v.vx) mdl->ux=vert->v.vx;
			if(mdl->uy<vert->v.vy) mdl->uy=vert->v.vy;
			if(mdl->uz<vert->v.vz) mdl->uz=vert->v.vz;
		}
    }
}

/* プリミティブをタイプ別にテクスチャ別に振り分ける */
static KMX_DEF *Distribute2( def )
KMX_DEF *def;
{
	int       i, j, k, x;
	int       n_new_x_models;
	float     fx, fy, fz;
	KMX_DEF   *newdef;
	KMX_MDL   *src;
	KMX_MDL   *dst;
	KMX_PRIMS *src_prim;
	KMX_PRIMS *dst_prim;
	KMX_VERTS *dst_vert;
	KMX_NORMS *dst_norm;

	printf("\n***** Distribute2 - Start\n");

	/* 新しいモデル数の決定 */
	n_new_x_models = (def->n_texs + 1) * N_Types;
//	printf("\tNumber of temp models: %d\n", n_new_x_models);

	/* 新 KMX_DEF モデルの生成 */
	if((newdef = Kmx_Alloc(n_new_x_models, n_new_x_models)) == NULL)
	{
		Kmx_Free(def);
		printf("Error : Cannot allocate KMX memory.\n");
		return NULL;
    }

	/* 新 KMX_DEF モデルの移動値の設定 */
	fx = (def->lx + def->ux) / 2.0f;
	fy = (def->ly + def->uy) / 2.0f;
	fz = (def->lz + def->uz) / 2.0f;
	fx = floor(fx);
	fy = floor(fy);
	fz = floor(fz);

	newdef->tx = def->tx + fx;
	newdef->ty = def->ty + fy;
	newdef->tz = def->tz + fz;

	/* 新 KMX_DEF モデルのバウンディングボックス */
	newdef->lx = def->lx + def->tx - newdef->tx;
	newdef->ly = def->ly + def->ty - newdef->ty;
	newdef->lz = def->lz + def->tz - newdef->tz;
	newdef->ux = def->ux + def->tx - newdef->tx;
	newdef->uy = def->uy + def->ty - newdef->ty;
	newdef->uz = def->uz + def->tz - newdef->tz;

	/* 新 KMX_DEF モデルへテクスチャの移動 */
	newdef->n_texs = def->n_texs;
	newdef->texs   = def->texs;

	def->n_texs = 0;
	def->texs   = NULL;

	/* 画面出力 */
//	printf("\tOld model Trans  : (%10.1f, %10.1f, %10.1f)\n", def->tx   , def->ty   , def->tz   );
//	printf("\tOld model B-box L: (%10.1f, %10.1f, %10.1f)\n", def->lx   , def->ly   , def->lz   );
//	printf("\tOld model B-box U: (%10.1f, %10.1f, %10.1f)\n", def->ux   , def->uy   , def->uz   );
//	printf("\tNew model Trans  : (%10.1f, %10.1f, %10.1f)\n", newdef->tx, newdef->ty, newdef->tz);
//	printf("\tNew model B-box L: (%10.1f, %10.1f, %10.1f)\n", newdef->lx, newdef->ly, newdef->lz);
//	printf("\tNew model B-box U: (%10.1f, %10.1f, %10.1f)\n", newdef->ux, newdef->uy, newdef->uz);

	/* 各モデルの移動値の設定 */
	i = 0;
	for(j = 0; j < N_Types; j++)
	{
		for(k = 0; k < newdef->n_texs + 1; k++)
		{
			newdef->models[i].type = Type[j];
			newdef->models[i].tx   = 0.0f;
			newdef->models[i].ty   = 0.0f;
			newdef->models[i].tz   = 0.0f;
			i++;
		}
	}

	/* 各モデルのプリミティブ数の取得 */
	for(i = 0; i < def->n_x_models; i++)
	{
		src = &def->models[i];

		/* 元モデルのタイプを調べる */
		for(j = 0; j < N_Types; j++)
		{
			if(src->type == Type[j])
				break;
		}
		if(j == N_Types)
		{
			Kmx_Free(newdef);
			Kmx_Free(def);
			printf("Error : Cannot match model type.\n");
			return NULL;
		}
		j *= (newdef->n_texs + 1);

		for(k = 0; k < src->n_prims; k++)
		{
			src_prim = src->prims + k;

			/* プリミティブの振り分け先モデルの決定 */
			if(src_prim->n_tid == 0)
			{
				/* テクスチャなし */
				newdef->models[j + newdef->n_texs].n_prims++;
				newdef->models[j + newdef->n_texs].n_verts += src_prim->n_id;
			}
			else
			{
				/* テクスチャ別 */
				if(*src_prim->tid >= (unsigned int)newdef->n_texs)
				{
					Kmx_Free(newdef);
					Kmx_Free(def);
					printf("Error : Invalid texture ID.\n");
					return NULL;
				}
				newdef->models[j + *src_prim->tid].n_prims++;
				newdef->models[j + *src_prim->tid].n_verts += src_prim->n_id;
			}
		}
	}

	/* 各モデルの頂点、法線、プリミティブ領域の確保 */
	if(Alloc_Verts_Norms_Prims(newdef) == 0)
	{
		Kmx_Free(newdef);
		Kmx_Free(def);
		printf("Error : Cannot allocate KMX Verts, Norms, Prims memory.\n");
		return NULL;
	}

	/* 各プリミティブの振り分け */
	for(i = 0; i < def->n_x_models; i++)
	{
		src = &def->models[i];

		/* 進行チェック用 */
//		printf("%d", i);

		/* 元モデルのタイプを調べる */
		for(j = 0; j < N_Types; j++)
		{
			if(src->type == Type[j])
			break;
		}
		j *= (newdef->n_texs + 1);

		for(k = 0; k < src->n_prims; k++)
		{
			src_prim = src->prims + k;

			/* 進行チェック用 */
//			printf(".");
//			fflush(stdout);

			/* 振り分け先の設定 */
			if(src_prim->n_tid == 0) dst = &newdef->models[j + newdef->n_texs];
			else                     dst = &newdef->models[j + *src_prim->tid];
			dst_prim = dst->prims + dst->n_prims;
			dst_vert = dst->verts + dst->n_verts;
			dst_norm = dst->norms + dst->n_norms;

			/* ポリゴンの移動 */
			for(x = 0; x < src_prim->n_id; x++)
			{
				/* 頂点、法線のコピー */
				memcpy(dst_vert + x, src->verts + *(src_prim->vid + x), sizeof(KMX_VERTS));
				memcpy(dst_norm + x, src->norms + *(src_prim->nid + x), sizeof(KMX_NORMS));

				/* 座標系を変更 */
				(dst_vert + x)->v.vx += (src->tx + def->tx);
				(dst_vert + x)->v.vy += (src->ty + def->ty);
				(dst_vert + x)->v.vz += (src->tz + def->tz);
				(dst_vert + x)->v.vx -= (dst->tx + newdef->tx);
				(dst_vert + x)->v.vy -= (dst->ty + newdef->ty);
				(dst_vert + x)->v.vz -= (dst->tz + newdef->tz);

				*(src_prim->vid + x) = dst->n_verts + x;
				*(src_prim->nid + x) = dst->n_norms + x;
			}
			Kmx_Prim_Move(dst_prim, src_prim);

			dst->n_prims++;
			dst->n_verts += dst_prim->n_id;
			dst->n_norms += dst_prim->n_id;
		}

//		printf(" ");
	}

	/* 旧 KMX_DEF モデルの破棄 */
	Kmx_Free(def);

	/* プリミティブが存在しないモデルを削除する */
	if((newdef = Remove_Null_Models(newdef)) == NULL)
	{
		printf("Error : Remove_Null_Models failed.\n");
		return NULL;
	}

	/* 各モデルのバウンディングボックスの設定 */
	Set_Bounding_Box(newdef);

	printf("\n***** Distribute2 - End\n");

    return newdef;
}

/* 二つのプリミティブが繋がっているか判定 */
static int Is_Same_Strip(mdl, prim_no1, prim_no2)
KMX_MDL *mdl;
int     prim_no1;
int     prim_no2;
{
	int       i, j;
	int       count;
	KMX_PRIMS *prim1;
	KMX_PRIMS *prim2;

	prim1 = mdl->prims + prim_no1;
	prim2 = mdl->prims + prim_no2;

	count = 0;
	for(i=0;i<prim1->n_id;i++)
	{
		for(j=0;j<prim2->n_id;j++)
		{
			if(    ((mdl->verts + *(prim1->vid + i))->v.vx==(mdl->verts + *(prim2->vid + j))->v.vx)
				&& ((mdl->verts + *(prim1->vid + i))->v.vy==(mdl->verts + *(prim2->vid + j))->v.vy)
				&& ((mdl->verts + *(prim1->vid + i))->v.vz==(mdl->verts + *(prim2->vid + j))->v.vz))
				count++;
		}
	}

	/* 同じ座標の頂点を 2 つ以上持てば繋がっている */
	if(count > 1) return 1;
	else          return 0;
}

/* モデルをストリップ単位に分ける */
static KMX_DEF *Strip( def )
KMX_DEF *def;
{
	int       i, j, k, x;
	int       flag;
	int       *strip_count;
	int       **strip_nos;
	int       *strip_no;

	int       n_new_x_models;
	KMX_DEF   *newdef;
	KMX_MDL   *src;
	KMX_MDL   *dst;
	KMX_MDL   *dst_tmp;
	KMX_PRIMS *src_prim;
	KMX_PRIMS *dst_prim;
	KMX_VERTS *dst_vert;
	KMX_NORMS *dst_norm;

	printf("\n***** Strip - Start\n");

	/* ワークメモリの確保 */
	if(    ((strip_count = (int* )calloc(def->n_x_models, sizeof(int ))) == NULL)
		|| ((strip_nos   = (int**)calloc(def->n_x_models, sizeof(int*))) == NULL))
	{
		free(strip_count);
		Kmx_Free(def);
		printf("Error : Cannot allocate work memory.\n");
		return NULL;
	}
	for(i = 0; i < def->n_x_models; i++)
	{
		src = &def->models[i];

		if((*(strip_nos + i) = (int*)calloc(src->n_prims, sizeof(int))) == NULL)
		{
			free(strip_count);
			for(j = 0; j < i; j++)
				free(*(strip_nos + j));
			free(strip_nos);
			Kmx_Free(def);
			printf("Error : Cannot allocate work memory.\n");
			return NULL;
		}
	}

	/* 各プリミティブを振り分け先ストリップを決める */
	for(i = 0; i < def->n_x_models; i++)
	{
		src      = &def->models[i];
		strip_no = *(strip_nos + i);

		*(strip_count + i) = 1;

		for(j = 1; j < src->n_prims; j++)
		{
			flag = 0;

			/* 前のプリミティブと繋がっているかを調べる */
			for(k = 0; k < j; k++)
			{
				if(Is_Same_Strip(src, j, k) == 1)
				{
					/* 繋がっているプリミティブを同じストリップに設定 */
					if(flag == 0)
					{
						*(strip_no + j) = *(strip_no + k);
						flag++;
					}
					else
					{
						for(x = 0; x < j; x++)
						{
							if(    (x != k)
								&& (*(strip_no + x) == *(strip_no + k)))
								*(strip_no + x) = *(strip_no + j);
						}
						*(strip_no + k) = *(strip_no + j);
					}
				}
			}

			/* 繋がっているプリミティブがないので新しいストリップ */
			if(flag == 0)
			{
				*(strip_no + j) = *(strip_count + i);
				(*(strip_count + i))++;
			}
		}
	}
	/* ストリップ数の修正 */
	for(i = 0; i < def->n_x_models; i++)
	{
		src      = &def->models[i];
		strip_no = *(strip_nos + i);

		/* 未使用の番号があれば詰める */
		for(j = 0; j < *(strip_count + i); j++)
		{
			for(k = 0; k < src->n_prims; k++)
			{
				if(*(strip_no + k) == j)
					break;
			}
			if(k == src->n_prims)
			{
				for(k = 0; k < src->n_prims; k++)
				{
					if(*(strip_no + k) > j)
						(*(strip_no + k))--;
				}
				(*(strip_count + i))--;
				j--;
			}
		}
	}

	/* 新しいモデル数の決定 */
	n_new_x_models = 0;
	for(i = 0; i < def->n_x_models; i++)
		n_new_x_models += *(strip_count + i);
//	printf("\tNumber of temp models: %d\n", n_new_x_models);

	/* 新 KMX_DEF モデルの生成 */
	if((newdef = Kmx_Alloc(n_new_x_models, n_new_x_models)) == NULL)
	{
		free(strip_count);
		for(i = 0; i < def->n_x_models; i++)
			free(*(strip_nos + i));
		free(strip_nos);
		Kmx_Free(def);
		printf("Error : Cannot allocate KMX memory.\n");
		return NULL;
    }

	/* 新 KMX_DEF モデルの設定 */
	newdef->tx = def->tx;
	newdef->ty = def->ty;
	newdef->tz = def->tz;
	newdef->lx = def->lx;
	newdef->ly = def->ly;
	newdef->lz = def->lz;
	newdef->ux = def->ux;
	newdef->uy = def->uy;
	newdef->uz = def->uz;

	newdef->n_texs = def->n_texs;
	newdef->texs   = def->texs;

	def->n_texs = 0;
	def->texs   = NULL;

	/* 各モデルの移動値の設定 */
	i = 0;
	for(j = 0; j < def->n_x_models; j++)
	{
		src = &def->models[j];

		for(k = 0; k < *(strip_count + j); k++)
		{
			newdef->models[i].type = src->type;
			newdef->models[i].tx   = 0.0f;
			newdef->models[i].ty   = 0.0f;
			newdef->models[i].tz   = 0.0f;
			i++;
		}
	}

	/* 各モデルのプリミティブ数の取得 */
	dst = newdef->models;
	for(i = 0; i < def->n_x_models; i++)
	{
		src      = &def->models[i];
		strip_no = *(strip_nos + i);

		for(j = 0; j < src->n_prims; j++)
		{
			src_prim = src->prims + j;

			/* プリミティブの振り分け先モデルの決定 */
			(dst + *(strip_no + j))->n_prims++;
			(dst + *(strip_no + j))->n_verts += src_prim->n_id;
		}

		dst += *(strip_count + i);
	}

	/* 各モデルの頂点、法線、プリミティブ領域の確保 */
	if(Alloc_Verts_Norms_Prims(newdef) == 0)
	{
		Kmx_Free(newdef);
		Kmx_Free(def);
		printf("Error : Cannot allocate KMX Verts, Norms, Prims memory.\n");
		return NULL;
	}

	/* 各プリミティブの振り分け */
	dst = newdef->models;
	for(i = 0; i < def->n_x_models; i++)
	{
		src      = &def->models[i];
		strip_no = *(strip_nos + i);

		/* 進行チェック用 */
//		printf("%d", i);

		for(j = 0; j < src->n_prims; j++)
		{
			src_prim = src->prims + j;

			/* 進行チェック用 */
//			printf(".");
//			fflush(stdout);

			/* 振り分け先の設定 */
			dst_tmp  = dst + *(strip_no + j);
			dst_prim = dst_tmp->prims + dst_tmp->n_prims;
			dst_vert = dst_tmp->verts + dst_tmp->n_verts;
			dst_norm = dst_tmp->norms + dst_tmp->n_norms;

			/* ポリゴンの移動 */
			for(k = 0; k < src_prim->n_id; k++)
			{
				/* 頂点、法線のコピー */
				memcpy(dst_vert + k, src->verts + *(src_prim->vid + k), sizeof(KMX_VERTS));
				memcpy(dst_norm + k, src->norms + *(src_prim->nid + k), sizeof(KMX_NORMS));

				*(src_prim->vid + k) = dst_tmp->n_verts + k;
				*(src_prim->nid + k) = dst_tmp->n_norms + k;
			}
			Kmx_Prim_Move(dst_prim, src_prim);

			dst_tmp->n_prims++;
			dst_tmp->n_verts += dst_prim->n_id;
			dst_tmp->n_norms += dst_prim->n_id;
		}

		dst += *(strip_count + i);
//		printf(" ");
	}

	/* ワークメモリの解放 */
	free(strip_count);
	for(i = 0; i < def->n_x_models; i++)
		free(*(strip_nos + i));
	free(strip_nos);

	/* 旧 KMX_DEF モデルの破棄 */
	Kmx_Free(def);

	/* 各モデルのバウンディングボックスの設定 */
	Set_Bounding_Box(newdef);

	printf("\n***** Strip - End\n");

	return newdef;
}

/* 範囲が広くて頂点数が多いモデルを分割 */
static KMX_DEF *Divide( def )
KMX_DEF *def;
{
	int       i, j, k, x;
	int       nx, ny, nz;
	float     fx, fy, fz;
	int       *pnx, *pny, *pnz, *pntotal;

	int       n_new_x_models;
	KMX_DEF   *newdef;
	KMX_MDL   *src;
	KMX_MDL   *dst;
	KMX_PRIMS *src_prim;
	KMX_PRIMS *dst_prim;
	KMX_VERTS *dst_vert;
	KMX_NORMS *dst_norm;

	printf("\n***** Divide - Start\n");

	/* ワークメモリの確保 */
	pnx = pny = pnz = pntotal = NULL;
	if(    ((pnx     = (int*)calloc(def->n_x_models, sizeof(int))) == NULL)
		|| ((pny     = (int*)calloc(def->n_x_models, sizeof(int))) == NULL)
		|| ((pnz     = (int*)calloc(def->n_x_models, sizeof(int))) == NULL)
		|| ((pntotal = (int*)calloc(def->n_x_models, sizeof(int))) == NULL))
	{
		free(pnz);
		free(pny);
		free(pnx);
		Kmx_Free(def);
		printf("Error : Cannot allocate work memory.\n");
		return NULL;
	}

	/* 新しいモデル数の決定 */
	n_new_x_models = 0;
	for(i = 0; i < def->n_x_models; i++)
	{
		src = &def->models[i];

		if(    (src->ux - src->lx < BlockSize.vx)
			&& (src->uy - src->ly < BlockSize.vy)
			&& (src->uz - src->lz < BlockSize.vz))
		{
			*(pnx + i) = 1;
			*(pny + i) = 1;
			*(pnz + i) = 1;
		}
		else
		{
			/* モデルの大体の頂点数の取得 */
			x = 0;
			for(j = 0; j < src->n_prims; j++)
			{
				src_prim = src->prims + j;

				x += src_prim->n_id;
			}
			x -= (src->n_prims - 1) * 2;

			if(x <= min_verts)
			{
				/* 頂点数が少ないときは分割しない */
				*(pnx + i) = 1;
				*(pny + i) = 1;
				*(pnz + i) = 1;
			}
			else
			{
				/* ブロックサイズで分割 */
				*(pnx + i) = (int)((src->ux - src->lx) / BlockSize.vx) + 1;
				*(pny + i) = (int)((src->uy - src->ly) / BlockSize.vy) + 1;
				*(pnz + i) = (int)((src->uz - src->lz) / BlockSize.vz) + 1;
			}
		}
		*(pntotal + i)  = *(pnx + i) * *(pny + i) * *(pnz + i);
		n_new_x_models += *(pntotal + i);
	}
//	printf("\tNumber of temp models: %d\n", n_new_x_models);

	/* 新 KMX_DEF モデルの生成 */
	if((newdef = Kmx_Alloc(n_new_x_models, n_new_x_models)) == NULL)
	{
		free(pntotal);
		free(pnz);
		free(pny);
		free(pnx);
		Kmx_Free(def);
		printf("Error : Cannot allocate KMX memory.\n");
		return NULL;
    }

	/* 新 KMX_DEF モデルの設定 */
	newdef->tx = def->tx;
	newdef->ty = def->ty;
	newdef->tz = def->tz;
	newdef->lx = def->lx;
	newdef->ly = def->ly;
	newdef->lz = def->lz;
	newdef->ux = def->ux;
	newdef->uy = def->uy;
	newdef->uz = def->uz;

	newdef->n_texs = def->n_texs;
	newdef->texs   = def->texs;

	def->n_texs = 0;
	def->texs   = NULL;

	/* 各モデルの移動値の設定 */
	i = 0;
	for(j = 0; j < def->n_x_models; j++)
	{
		src = &def->models[j];

		for(k = 0; k < *(pntotal + j); k++)
		{
			newdef->models[i].type = src->type;
			newdef->models[i].tx   = 0.0f;
			newdef->models[i].ty   = 0.0f;
			newdef->models[i].tz   = 0.0f;
			i++;
		}
	}

	/* 各モデルのプリミティブ数の取得 */
	dst = newdef->models;
	for(i = 0; i < def->n_x_models; i++)
	{
		src = &def->models[i];

		if(    (*(pnx + i) == 1)
			&& (*(pny + i) == 1)
			&& (*(pnz + i) == 1))
		{
			/* そのままコピー */
			for(j = 0; j < src->n_prims; j++)
			{
				src_prim = src->prims + j;

				dst->n_prims++;
				dst->n_verts += src_prim->n_id;
			}
		}
		else
		{
			/* ブロック別に振り分ける */
			for(j = 0; j < src->n_prims; j++)
			{
				src_prim = src->prims + j;

				/* ポリゴンの中心座標を求める */
				fx = fy = fz = 0.0;
				for(k = 0; k < src_prim->n_id; k++)
				{
					fx += (src->verts + *(src_prim->vid + k))->v.vx;
					fy += (src->verts + *(src_prim->vid + k))->v.vy;
					fz += (src->verts + *(src_prim->vid + k))->v.vz;
				}
				fx = fx / src_prim->n_id + src->tx;
				fy = fy / src_prim->n_id + src->ty;
				fz = fz / src_prim->n_id + src->tz;

				/* ポリゴンの振り分け先モデルの決定 */
				nx = (int)((fx - src->lx) / BlockSize.vx);
				ny = (int)((fy - src->ly) / BlockSize.vy);
				nz = (int)((fz - src->lz) / BlockSize.vz);

				x = nx + (*(pnx + i) * ny)
					+ ((*(pnx + i) * *(pny + i)) * nz);

				(dst + x)->n_prims++;
				(dst + x)->n_verts += src_prim->n_id;
			}
		}

		dst += *(pntotal + i);
	}

	/* 各モデルの頂点、法線、プリミティブ領域の確保 */
	if(Alloc_Verts_Norms_Prims(newdef) == 0)
	{
		Kmx_Free(newdef);
		Kmx_Free(def);
		printf("Error : Cannot allocate KMX Verts, Norms, Prims memory.\n");
		return NULL;
	}

	/* 各プリミティブの振り分け */
	dst = newdef->models;
	for(i = 0; i < def->n_x_models; i++)
	{
		src = &def->models[i];

		/* 進行チェック用 */
//		printf("%d", i);

		if(    (*(pnx + i) == 1)
			&& (*(pny + i) == 1)
			&& (*(pnz + i) == 1))
		{
			/* そのままコピー */
			for(j = 0; j < src->n_prims; j++)
			{
				src_prim = src->prims + j;

				/* 進行チェック用 */
//				printf(".");
//				fflush(stdout);

				/* 振り分け先の設定 */
				dst_prim = dst->prims + dst->n_prims;
				dst_vert = dst->verts + dst->n_verts;
				dst_norm = dst->norms + dst->n_norms;

				/* ポリゴンの移動 */
				for(k = 0; k < src_prim->n_id; k++)
				{
					/* 頂点、法線のコピー */
					memcpy(dst_vert + k, src->verts + *(src_prim->vid + k), sizeof(KMX_VERTS));
					memcpy(dst_norm + k, src->norms + *(src_prim->nid + k), sizeof(KMX_NORMS));

					*(src_prim->vid + k) = dst->n_verts + k;
					*(src_prim->nid + k) = dst->n_norms + k;
				}
				Kmx_Prim_Move(dst_prim, src_prim);

				dst->n_prims++;
				dst->n_verts += dst_prim->n_id;
				dst->n_norms += dst_prim->n_id;
			}
		}
		else
		{
			/* ブロック別に振り分ける */
			for(j = 0; j < src->n_prims; j++)
			{
				src_prim = src->prims + j;

				/* 進行チェック用 */
//				printf(".");
//				fflush(stdout);

				/* ポリゴンの中心座標を求める */
				fx = fy = fz = 0.0;
				for(k = 0; k < src_prim->n_id; k++)
				{
					fx += (src->verts + *(src_prim->vid + k))->v.vx;
					fy += (src->verts + *(src_prim->vid + k))->v.vy;
					fz += (src->verts + *(src_prim->vid + k))->v.vz;
				}
				fx = fx / src_prim->n_id + src->tx;
				fy = fy / src_prim->n_id + src->ty;
				fz = fz / src_prim->n_id + src->tz;

				/* ポリゴンの振り分け先モデルの決定 */
				nx = (int)((fx - src->lx) / BlockSize.vx);
				ny = (int)((fy - src->ly) / BlockSize.vy);
				nz = (int)((fz - src->lz) / BlockSize.vz);

				x = nx + (*(pnx + i) * ny)
					+ ((*(pnx + i) * *(pny + i)) * nz);

				/* 振り分け先の設定 */
				dst_prim = (dst + x)->prims + (dst + x)->n_prims;
				dst_vert = (dst + x)->verts + (dst + x)->n_verts;
				dst_norm = (dst + x)->norms + (dst + x)->n_norms;

				/* ポリゴンの移動 */
				for(k = 0; k < src_prim->n_id; k++)
				{
					/* 頂点、法線のコピー */
					memcpy(dst_vert + k, src->verts + *(src_prim->vid + k), sizeof(KMX_VERTS));
					memcpy(dst_norm + k, src->norms + *(src_prim->nid + k), sizeof(KMX_NORMS));

					*(src_prim->vid + k) = (dst + x)->n_verts + k;
					*(src_prim->nid + k) = (dst + x)->n_norms + k;
				}
				Kmx_Prim_Move(dst_prim, src_prim);

				(dst + x)->n_prims++;
				(dst + x)->n_verts += dst_prim->n_id;
				(dst + x)->n_norms += dst_prim->n_id;
			}
		}

		dst += *(pntotal + i);
//		printf(" ");
	}

	/* ワークメモリの解放 */
	free(pntotal);
	free(pnz);
	free(pny);
	free(pnx);

	/* 旧 KMX_DEF モデルの破棄 */
	Kmx_Free(def);

	/* プリミティブが存在しないモデルを削除する */
	if((newdef = Remove_Null_Models(newdef)) == NULL)
	{
		printf("Error : Remove_Null_Models failed.\n");
		return NULL;
	}

	/* 各モデルのバウンディングボックスの設定 */
	Set_Bounding_Box(newdef);

	printf("\n***** Divide - End\n");

    return newdef;
}

/* ２つのモデルを合体 */
static int Combine_Model( to, from )
KMX_MDL *to;
KMX_MDL *from;
{
	int       i, j;
	int       *verts_table;
	int       *norms_table;
	int       n_verts;
	int       n_norms;
	int       n_prims;
	KMX_VERTS *verts = NULL;
	KMX_NORMS *norms = NULL;
	KMX_PRIMS *prims = NULL;

	if(    (to->type!=from->type)
		|| (to->tx!=from->tx)
		|| (to->ty!=from->ty)
		|| (to->tz!=from->tz)
		|| (to->parent!=from->parent)
		|| (to->extend!=from->extend))
		return 0;

	/* ワークメモリの確保 */
	if(    ((verts_table = (int*)calloc(from->n_verts, sizeof(int)))==NULL)
		|| ((norms_table = (int*)calloc(from->n_norms, sizeof(int)))==NULL))
	{
		free(verts_table);
		return 0;
	}

	/* バウンディングボックスの設定 */
	if(to->lx>from->lx) to->lx = from->lx;
	if(to->ly>from->ly) to->ly = from->ly;
	if(to->lz>from->lz) to->lz = from->lz;
	if(to->ux<from->ux) to->ux = from->ux;
	if(to->uy<from->uy) to->uy = from->uy;
	if(to->uz<from->uz) to->uz = from->uz;
	from->lx = 0;
	from->ly = 0;
	from->lz = 0;
	from->ux = 0;
	from->uy = 0;
	from->uz = 0;

	/* 頂点数の取得 */
	n_verts = to->n_verts;
	for(i=0;i<from->n_verts;i++)
	{
		for(j=0;j<to->n_verts;j++)
		{
			/* 同じ座標の頂点があるか判定 */
			if(    ((to->verts + j)->v.vx==(from->verts + i)->v.vx)
				&& ((to->verts + j)->v.vy==(from->verts + i)->v.vy)
				&& ((to->verts + j)->v.vz==(from->verts + i)->v.vz))
				break;
		}
		/* from から to への変換テーブルの設定 */
		if(j!=to->n_verts)
		{
			*(verts_table + i) = j;
		}
		else
		{
			*(verts_table + i) = n_verts;
			n_verts++;
		}
	}

	/* 法線数の取得 */
	n_norms = to->n_norms;
	for(i=0;i<from->n_norms;i++)
	{
		for(j=0;j<to->n_norms;j++)
		{
			/* 同じ値の法線があるか判定 */
			if(    ((to->norms + j)->n.vx==(from->norms + i)->n.vx)
				&& ((to->norms + j)->n.vy==(from->norms + i)->n.vy)
				&& ((to->norms + j)->n.vz==(from->norms + i)->n.vz))
				break;
		}
		/* from から to への変換テーブルの設定 */
		if(j!=to->n_norms)
		{
			*(norms_table + i) = j;
		}
		else
		{
			*(norms_table + i) = n_norms;
			n_norms++;
		}
	}

	/* プリミティブ数の取得 */
	n_prims = to->n_prims + from->n_prims;

	/* 頂点、法線、プリミティブ領域の確保 */
	if(    ((verts = Kmx_Verts_Alloc(n_verts))==NULL)
		|| ((norms = Kmx_Norms_Alloc(n_norms))==NULL)
		|| ((prims = Kmx_Prims_Alloc(n_prims))==NULL))
	{
		Kmx_Norms_Free(norms);
		Kmx_Verts_Free(verts);
		free(norms_table);
		free(verts_table);
		return 0;
	}

	/* 頂点の移動 */
	memcpy(verts, to->verts, sizeof(KMX_VERTS) * to->n_verts);
	i = to->n_verts;
	for(j=0;j<from->n_verts;j++)
	{
		if(*(verts_table + j)>=i)
		{
			memcpy(verts + i, from->verts + j, sizeof(KMX_VERTS));
			i++;
		}
	}
	Kmx_Verts_Free(to->verts);
	to->verts   = verts;
	to->n_verts = n_verts;

	Kmx_Verts_Free(from->verts);
	from->verts   = NULL;
	from->n_verts = 0;

	/* 法線の移動 */
	memcpy(norms, to->norms, sizeof(KMX_NORMS) * to->n_norms);
	i = to->n_norms;
	for(j=0;j<from->n_norms;j++)
	{
		if(*(norms_table + j) >= i)
		{
			memcpy(norms + i, from->norms + j, sizeof(KMX_NORMS));
			i++;
		}
	}
	Kmx_Norms_Free(to->norms);
	to->norms   = norms;
	to->n_norms = n_norms;

	Kmx_Norms_Free(from->norms);
	from->norms   = NULL;
	from->n_norms = 0;

	/* プリミティブの移動 */
	for(i=0;i<from->n_prims;i++)
	{
		for(j=0;j<(from->prims + i)->n_id;j++)
		{
			*((from->prims + i)->vid + j) = *(verts_table + *((from->prims + i)->vid + j));
			*((from->prims + i)->nid + j) = *(norms_table + *((from->prims + i)->nid + j));
		}
	}

	for(i=0;i<to->n_prims;i++)
	{
		Kmx_Prim_Move(prims + i, to->prims + i);
	}
	for(j=0;j<from->n_prims;j++)
	{
		Kmx_Prim_Move(prims + i, from->prims + j);
		i++;
	}
	Kmx_Prims_Free(to->prims, to->n_prims);
	to->prims   = prims;
	to->n_prims = n_prims;

	Kmx_Prims_Free(from->prims, from->n_prims);
	from->prims   = NULL;
	from->n_prims = 0;

	/* ワークメモリの解放 */
	free(norms_table);
	free(verts_table);

	return 1;
}

/* 範囲が重なっていて頂点数が少ないモデルを合体 */
static KMX_DEF *Combine( def )
KMX_DEF *def;
{
	int     i, j;
	float   mx1, my1, mz1;
	float   mx2, my2, mz2;
	float   distance;
	int     closest;
	KMX_MDL *src;
	KMX_MDL *dst;

	printf("\n***** Combine - Start\n");

	for(i = 0; i < def->n_x_models; i++)
	{
		dst = &def->models[i];

		/* 進行チェック用 */
//		printf("%d", i);

		if(    (dst->n_prims != 0)
			&& (   (dst->ux - dst->lx < BlockSize.vx)
				&& (dst->uy - dst->ly < BlockSize.vy)
				&& (dst->uz - dst->lz < BlockSize.vz)))
		{
			/* オブジェクトの中心 */
			mx1 = (dst->ux + dst->lx) / 2.0f;
			my1 = (dst->uy + dst->ly) / 2.0f;
			mz1 = (dst->uz + dst->lz) / 2.0f;

			closest = -1;

			/* 最も近いオブジェクトを検索 */
			for(j = i + 1; j < def->n_x_models; j++)
			{
				src = &def->models[j];

				if(dst->type != src->type)
					break;

				if(    (src->n_prims != 0)
					&& (   (src->ux - src->lx < BlockSize.vx)
						&& (src->uy - src->ly < BlockSize.vy)
						&& (src->uz - src->lz < BlockSize.vz)))
				{
					/* ブロックサイズに納まるか */
					if(    (___MAX(dst->ux, src->ux) - ___MIN(dst->lx, src->lx) < BlockSize.vx)
						&& (___MAX(dst->uy, src->uy) - ___MIN(dst->ly, src->ly) < BlockSize.vy)
						&& (___MAX(dst->uz, src->uz) - ___MIN(dst->lz, src->lz) < BlockSize.vz))
					{
						if(closest == -1)
						{
							mx2 = mx1 - (src->ux + src->lx) / 2.0f;
							my2 = my1 - (src->uy + src->ly) / 2.0f;
							mz2 = mz1 - (src->uz + src->lz) / 2.0f;
							distance = mx2 * mx2 + my2 * my2 + mz2 * mz2;
							closest  = j;
						}
						else
						{
							mx2 = mx1 - (src->ux + src->lx) / 2.0f;
							my2 = my1 - (src->uy + src->ly) / 2.0f;
							mz2 = mz1 - (src->uz + src->lz) / 2.0f;
							if(distance > mx2 * mx2 + my2 * my2 + mz2 * mz2)
							{
								distance = mx2 * mx2 + my2 * my2 + mz2 * mz2;
								closest  = j;
							}
						}
					}
				}
			}

			/* 結合する */
			if(closest != -1)
			{
				src = &def->models[closest];

				/* 合体する */
				if(Combine_Model(dst, src) == 0)
				{
					Kmx_Free(def);
					printf("Error : Combine_Model failed.\n");
					return NULL;
				}
				i--;
			}
		}

//		printf("\n");
	}

	/* プリミティブが存在しないモデルを削除する */
	if((def = Remove_Null_Models(def)) == NULL)
	{
		printf("Error : Remove_Null_Models failed.\n");
		return NULL;
	}

	printf("\n***** Combine - End\n");

    return def;
}

/*****************************************************************************/

/* 頂点、法線の重複をチェックする */
static KMX_DEF *KmxJointModel(KMX_DEF *def)
{
    KMX_MDL *mdl;
    int i,n_models;

    /* プリミティブが存在しないところを詰める */
    mdl=def->models;
    n_models=0;
    for(i=0;i<def->n_x_models;i++,mdl++){
		if(mdl->n_prims>0) n_models++;
    }

    if(n_models!=def->n_x_models){
		KMX_DEF *newdef;
		KMX_MDL *newmdl;

		if((newdef=Kmx_Alloc(n_models,n_models))==NULL){
			Kmx_Free(def);
			return NULL;
		}

		newdef->lx=def->lx;
		newdef->ly=def->ly;
		newdef->lz=def->lz;
		newdef->ux=def->ux;
		newdef->uy=def->uy;
		newdef->uz=def->uz;
		newdef->tx=def->tx;
		newdef->ty=def->ty;
		newdef->tz=def->tz;

		newdef->n_texs=def->n_texs;
		newdef->texs=def->texs;

		def->n_texs=0;
		def->texs=NULL;

		mdl=def->models;
		newmdl=newdef->models;
		for(i=0;i<def->n_x_models;i++,mdl++){

			/* 進行チェック用 */
			printf(".");
			fflush(stdout);

			if(mdl->n_prims>0){
				Kmx_MdlData1_Move(newmdl,mdl);
				newmdl++;
			}
		}

		Kmx_Free(def);
		def=newdef;
    }


    /* 重複チェック */
    mdl=def->models;
    for(i=0;i<def->n_x_models;i++,mdl++){
		KMX_VERTS *verts=mdl->verts;
		KMX_NORMS *norms=mdl->norms;
		KMX_PRIMS *prims;
		int j,k;
		int *check;
		int n;

		if(mdl->n_verts!=0){
			/* 進行チェック用 */
			printf(".");
			fflush(stdout);

			/* 頂点の重複チェック */
			if((check=(int *)MDU_Alloc(sizeof(int)*mdl->n_verts))==NULL){
				Kmx_Free(def);
				return NULL;
			}

			for(j=0;j<mdl->n_verts;j++) *(check+j)=-1;

			n=0;
			for(j=0;j<mdl->n_verts;j++){
				for(k=0;k<j;k++){
					if((verts+j)->v.vx==(verts+k)->v.vx &&
					   (verts+j)->v.vy==(verts+k)->v.vy &&
					   (verts+j)->v.vz==(verts+k)->v.vz){

						break;
					}
				}
				if(k==j){
					*(check+j)=n;
					n++;
				}
				else{
					*(check+j)=*(check+k);
				}
			}

			if((verts=Kmx_Verts_Alloc(n))==NULL){
				MDU_Free(check);
				Kmx_Free(def);
				return NULL;
			}
			for(j=0,k=0;j<mdl->n_verts;j++){
				if(*(check+j)==k){
					memcpy(verts+k,mdl->verts+j,sizeof(KMX_VERTS));
					k++;
				}
			}

			/* プリミティブの頂点IDのつけ直し */
			prims=mdl->prims;
			for(j=0;j<mdl->n_prims;j++,prims++){
				for(k=0;k<prims->n_id;k++){
					*(prims->vid+k)=*(check+*(prims->vid+k));
				}
			}

			Kmx_Verts_Free(mdl->verts);
			mdl->verts=verts;
			mdl->n_verts=n;

			MDU_Free(check);
		}

		if(mdl->n_norms!=0){
			/* 進行チェック用 */
			printf("#");
			fflush(stdout);

			/* 法線の重複チェック */
			if((check=(int *)MDU_Alloc(sizeof(int)*mdl->n_norms))==NULL){
				Kmx_Free(def);
				return NULL;
			}

			for(j=0;j<mdl->n_norms;j++) *(check+j)=-1;

			n=0;
			for(j=0;j<mdl->n_norms;j++){
				for(k=0;k<j;k++){
					if((norms+j)->n.vx==(norms+k)->n.vx &&
					   (norms+j)->n.vy==(norms+k)->n.vy &&
					   (norms+j)->n.vz==(norms+k)->n.vz){

						break;
					}
				}
				if(k==j){
					*(check+j)=n;
					n++;
				}
				else{
					*(check+j)=*(check+k);
				}
			}

			if((norms=Kmx_Norms_Alloc(n))==NULL){
				MDU_Free(check);
				Kmx_Free(def);
				return NULL;
			}
			for(j=0,k=0;j<mdl->n_norms;j++){
				if(*(check+j)==k){
					memcpy(norms+k,mdl->norms+j,sizeof(KMX_NORMS));
					k++;
				}
			}

			/* プリミティブの法線IDのつけ直し */
			prims=mdl->prims;
			for(j=0;j<mdl->n_prims;j++,prims++){
				for(k=0;k<prims->n_id;k++){
					*(prims->nid+k)=*(check+*(prims->nid+k));
				}
			}

			Kmx_Norms_Free(mdl->norms);
			mdl->norms=norms;
			mdl->n_norms=n;

			MDU_Free(check);
		}
    }

    /* 進行チェック用 */
    printf("\n");

    return def;
}

/* モデル分割関数 */
KMX_DEF *MDU_DivideKmx( def, div, vert )
KMX_DEF *def ;
SVECTOR	*div ; /* 分割数 */
int     vert ; /* パケットの最大頂点数 */
{
	/* 値の修正 */
	if(div->vx < 6000) div->vx = 6000;
	if(div->vy < 6000) div->vy = 6000;
	if(div->vz < 6000) div->vz = 6000;

    Div = div;
    min_verts = vert;

    /* 進行チェック用 */
    printf("Model divide start\n");

    /* 全体バウンディング、分割オブジェ中心、分割オブジェ数などを取得 */
    if(!GetDivideParam(def)) return def;

	if(min_verts < 1)
	{
		/* ポリゴン振り分け */
		if((def=Distribute(def))==NULL) return NULL;

		/* 進行チェック用 */
		printf("Joint Model start\n");

		/* 頂点＆法線共有化 */
		if((def=KmxJointModel(def))==NULL){
			Kmx_Free(def);
			return NULL;
		}
	}
	else
	{
		/* プリミティブをタイプ別にテクスチャ別に振り分ける */
		if((def=Distribute2(def))==NULL) return NULL;

		/* モデルをストリップ単位に分ける */
		if((def=Strip(def))==NULL) return NULL;

		/* 範囲が広くて頂点数が多いモデルを分割 */
		if((def=Divide(def))==NULL) return NULL;

		/* 進行チェック用 */
		printf("Joint Model start\n");

		/* 頂点＆法線共有化 */
		if((def=KmxJointModel(def))==NULL){
			Kmx_Free(def);
			return NULL;
		}

		/* 範囲が重なっていて頂点数が少ないモデルを合体 */
		if((def=Combine(def))==NULL) return NULL;
	}

    return def;
}

/*--------------------------------------------------------------------------*/

/* 属性によるモデル分割 */
/* １：半透明オブジェに不透明テクスチャ → 新不透明オブジェ作成 
   ２：不透明オブジェに半透明テクスチャ → 新半透明オブジェ作成 
   ３：キャラモデルなら、半透明テクスチャ部分は拡張モデル化 */


/* ３番用分割処理 */
static KMX_DEF *MakeSplitModel(KMX_DEF *def)
{
    KMX_DEF *newdef;
    int	n_extends, n_models;
    int	n_prims,i,j,k,m ;
    KMX_MDL *src,*dst,*ext;
    KMX_PRIMS *prim;
    int *n_checkprims;

    src=def->models;
    n_models=def->n_x_models;
    n_extends=0 ;

    if((n_checkprims=MDU_Alloc(sizeof(int)*n_models))==NULL){
		Kmx_Free(def);
		return NULL;
    }

    /* 拡張モデルに格納するプリミティブをカウント */
    for(i=0;i<n_models;i++,src++){
		*(n_checkprims+i)=0;

		n_prims=src->n_prims;
		prim=src->prims;

		for( ;n_prims>0;prim++,n_prims--){
			int tid;

			if(prim->n_tid==0 || prim->tid==NULL) continue;

#if 0
			for(j=0;j<prim->n_tid;j++){
				tid=*(prim->tid+j);
				if(IsOverlayTexture(def->texs[tid].name)){
					(*(n_checkprims+i))++;
					break;
				}
			}
#else
			tid=*(prim->tid+0);
			if(IsOverlayTexture(def->texs[tid].name)){
				(*(n_checkprims+i))++;
			}
#endif
		}
		if(*(n_checkprims+i)) n_extends++;
    }

    if(n_extends==0){
		MDU_Free(n_checkprims);
		return def;
    }

    /* 進行チェック用 */
    printf("n_extends %d\n",n_extends);

    /* 領域確保 */
    if((newdef=Kmx_Alloc(def->n_models,n_models+n_extends))==NULL){
		MDU_Free(n_checkprims);
		Kmx_Free(def);
		return NULL;
    }

    newdef->lx=def->lx;
    newdef->ly=def->ly;
    newdef->lz=def->lz;
    newdef->ux=def->ux;
    newdef->uy=def->uy;
    newdef->uz=def->uz;
    newdef->tx=def->tx;
    newdef->ty=def->ty;
    newdef->tz=def->tz;

    newdef->n_texs=def->n_texs;
    newdef->texs=def->texs;

    def->n_texs=0;
    def->texs=NULL;

    src=def->models;
    dst=newdef->models;
    ext=newdef->models+n_models;
    m=n_models;

    for(i=0;i<n_models;i++,src++){
		int *check_v,*check_n;
		int *check_v2,*check_n2;

		if(*(n_checkprims+i)==0){
			/* 拡張モデルを作る必要がない場合 */

			/* 進行チェック用 */
			printf("#");
			fflush(stdout);

			memcpy(dst,src,sizeof(KMX_MDL));

			src->n_verts=0;
			src->verts=NULL;
			src->n_norms=0;
			src->norms=NULL;
			src->n_prims=0;
			src->prims=NULL;

			dst++;
			continue;
		}

		if(*(n_checkprims+i)==src->n_prims){
			/* 全てのプリミティブが拡張モデルに作られた場合 */

			/* 進行チェック用 */
			printf(">");
			fflush(stdout);

			memcpy(dst,src,sizeof(KMX_MDL));
			memcpy(ext,src,sizeof(KMX_MDL));

			dst->extend=m;
			dst->n_verts=0;
			dst->verts=NULL;
			dst->n_norms=0;
			dst->norms=NULL;
			dst->n_prims=0;
			dst->prims=NULL;

			ext->type|=DG_TYPE_EXTEND | DG_TYPE_OVERLAY0;
			if(i<def->n_models) ext->parent=i;
			ext->extend=-1;

			src->n_verts=0;
			src->verts=NULL;
			src->n_norms=0;
			src->norms=NULL;
			src->n_prims=0;
			src->prims=NULL;

			dst++;
			ext++;
			m++;
			continue;
		}


		/* 拡張モデルに、プリミティブをいくつか移動する場合 */

		/* 進行チェック用 */
		printf(".");
		fflush(stdout);

		memcpy(dst,src,sizeof(KMX_MDL));
		memcpy(ext,src,sizeof(KMX_MDL));

		dst->extend=m;
		dst->n_verts=0;
		dst->verts=NULL;
		dst->n_norms=0;
		dst->norms=NULL;
		dst->n_prims=0;
		dst->prims=NULL;

		ext->type|=DG_TYPE_EXTEND | DG_TYPE_OVERLAY0;
		if(i<def->n_models) ext->parent=i;
		ext->extend=-1;

		ext->n_verts=0;
		ext->verts=NULL;
		ext->n_norms=0;
		ext->norms=NULL;
		ext->n_prims=0;
		ext->prims=NULL;

		if((check_v=(int *)MDU_Alloc(sizeof(int)*(src->n_verts+src->n_norms)*2))==NULL){
			MDU_Free(n_checkprims);
			Kmx_Free(def);
			Kmx_Free(newdef);
			return NULL;
		}
		check_n=check_v+src->n_verts;
		check_v2=check_n+src->n_norms;
		check_n2=check_v2+src->n_verts;

		if((dst->prims=Kmx_Prims_Alloc(src->n_prims-*(n_checkprims+i)))==NULL){
			MDU_Free(n_checkprims);
			MDU_Free(check_v);
			Kmx_Free(def);
			Kmx_Free(newdef);
			return NULL;
		}
		dst->n_prims=0;

		if((ext->prims=Kmx_Prims_Alloc(*(n_checkprims+i)))==NULL){
			MDU_Free(n_checkprims);
			MDU_Free(check_v);
			Kmx_Free(def);
			Kmx_Free(newdef);
			return NULL;
		}
		ext->n_prims=0;

		for(j=0;j<src->n_verts;j++) *(check_v+j)=*(check_v2+j)=0;
		for(j=0;j<src->n_norms;j++) *(check_n+j)=*(check_n2+j)=0;

		prim=src->prims;
		n_prims=src->n_prims;
		while(n_prims>0){
			int flag=0;
			int tid;

#if 0
			for(k=0;k<prim->n_tid;k++){
				tid=*(prim->tid+k);
				if(IsOverlayTexture(newdef->texs[tid].name)){
					flag=1;
					break;
				}
			}
#else
			if(prim->n_tid>0){
				tid=*(prim->tid+0);
				if(IsOverlayTexture(newdef->texs[tid].name)){
					flag=1;
				}
			}
#endif
			if(flag){
				/* 拡張モデルにプリミティブを移す */

				for(j=0;j<prim->n_id;j++){
					*(check_v2+*(prim->vid+j))=1;
					*(check_n2+*(prim->nid+j))=1;
				}
				Kmx_Prim_Move(ext->prims+ext->n_prims,prim);
				ext->n_prims++;
			}
			else{
				/* 拡張モデルに移さない */

				for(j=0;j<prim->n_id;j++){
					*(check_v+*(prim->vid+j))=1;
					*(check_n+*(prim->nid+j))=1;
				}
				Kmx_Prim_Move(dst->prims+dst->n_prims,prim);
				dst->n_prims++;
			}

			prim++;
			n_prims--;
		}

		{
			int v_cnt,v_cnt2;
			int n_cnt,n_cnt2;

			v_cnt=0;
			v_cnt2=0;
			n_cnt=0;
			n_cnt2=0;

			for(j=0;j<src->n_verts;j++){
				if(*(check_v+j)){
					*(check_v+j)=v_cnt;
					v_cnt++;
				}
				else *(check_v+j)=-1;

				if(*(check_v2+j)){
					*(check_v2+j)=v_cnt2;
					v_cnt2++;
				}
				else *(check_v2+j)=-1;
			}
			for(j=0;j<src->n_norms;j++){
				if(*(check_n+j)){
					*(check_n+j)=n_cnt;
					n_cnt++;
				}
				else *(check_n+j)=-1;

				if(*(check_n2+j)){
					*(check_n2+j)=n_cnt2;
					n_cnt2++;
				}
				else *(check_n2+j)=-1;
			}

			/* 頂点、法線格納場所の確保 */
			if((dst->verts=Kmx_Verts_Alloc(v_cnt))==NULL){
				MDU_Free(n_checkprims);
				MDU_Free(check_v);
				Kmx_Free(def);
				Kmx_Free(newdef);
				return NULL;
			}
			dst->n_verts=v_cnt;

			if((ext->verts=Kmx_Verts_Alloc(v_cnt2))==NULL){
				MDU_Free(n_checkprims);
				MDU_Free(check_v);
				Kmx_Free(def);
				Kmx_Free(newdef);
				return NULL;
			}
			ext->n_verts=v_cnt2;

			if((dst->norms=Kmx_Norms_Alloc(n_cnt))==NULL){
				MDU_Free(n_checkprims);
				MDU_Free(check_v);
				Kmx_Free(def);
				Kmx_Free(newdef);
				return NULL;
			}
			dst->n_norms=n_cnt;

			if((ext->norms=Kmx_Norms_Alloc(n_cnt2))==NULL){
				MDU_Free(n_checkprims);
				MDU_Free(check_v);
				Kmx_Free(def);
				Kmx_Free(newdef);
				return NULL;
			}
			ext->n_norms=n_cnt2;
		}

		/* 頂点の移動 */
		for(j=0;j<src->n_verts;j++){
			int index;

			index=*(check_v+j);
			if(index!=-1){
				memcpy(dst->verts+index,src->verts+j,sizeof(KMX_VERTS));
			}

			index=*(check_v2+j);
			if(index!=-1){
				memcpy(ext->verts+index,src->verts+j,sizeof(KMX_VERTS));
			}
		}

		/* 法線の移動 */
		for(j=0;j<src->n_norms;j++){
			int index;

			index=*(check_n+j);
			if(index!=-1){
				memcpy(dst->norms+index,src->norms+j,sizeof(KMX_NORMS));
			}

			index=*(check_n2+j);
			if(index!=-1){
				memcpy(ext->norms+index,src->norms+j,sizeof(KMX_NORMS));
			}
		}

		/* プリミティブに含まれるID番号の変更 */
		prim=dst->prims;
		n_prims=dst->n_prims;
		while(n_prims>0){
			for(j=0;j<prim->n_id;j++){
				*(prim->vid+j)=*(check_v+*(prim->vid+j));
				*(prim->nid+j)=*(check_n+*(prim->nid+j));
			}
			prim++;
			n_prims--;
		}

		prim=ext->prims;
		n_prims=ext->n_prims;
		while(n_prims>0){
			for(j=0;j<prim->n_id;j++){
				*(prim->vid+j)=*(check_v2+*(prim->vid+j));
				*(prim->nid+j)=*(check_n2+*(prim->nid+j));
			}
			prim++;
			n_prims--;
		}

		dst++;
		ext++;
		m++;

		MDU_Free(check_v);
    }

    MDU_Free(n_checkprims);
    Kmx_Free(def);

    /* 進行チェック用 */
    printf("\n");

    return newdef;
}

KMX_DEF *MDU_SplitKmx(def)
KMX_DEF *def;
{
    int n_models;

    n_models=def->n_x_models;

    /* 進行チェック用 */
    printf("Split Model start\n");

    if((def=MakeSplitModel(def))!=NULL){
		if (n_models==def->n_x_models ) {
			/* 分割の必要なし */
			printf( "--- split by transparent not needed\n" ) ;
		}
    }
    return def;
}

/* -------------------------------------------------------------------------- */
