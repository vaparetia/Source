/*
   MDU_mdlKm32Kms.c

   Ｋｍ３ → Ｋｍｓコンバート関数

   by M.Sonoyama 1999.Sep.～ 
   $Id: MDU_mdlKm32Kms.c,v 1.8 2000/02/15 07:26:40 usr02011 Exp $
   
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
#include "fmt_kms.h"

#include "MDU_util.h"
#include "MDU_mdl.h"

#include "MDU_mdlKm32Kms.h"

static	WORK	work ;

#include "MDU_mdlKm32KmsUtil.c"

#define N_MAX_POINTS 64

/*------------------------------------------------------------------*/

/* Ｋｍ３ → Ｋｍｓ変換 */

/* KM3_DEF2 の開放 */
void		MDU_mdlFreeKm3Def2( def2 )
KM3_DEF2	*def2 ;
{
    int		i ;
    KM3_MDL	*mdl ;

    mdl = def2->def->models ;
    for ( i = 0; i < def2->def->n_x_models; i ++, mdl ++ ) {
	if ( mdl->verts != NULL ) MDU_Free( mdl->verts ) ;
	if ( mdl->norms != NULL ) MDU_Free( mdl->norms ) ;
	if ( mdl->envs != NULL ) MDU_Free( mdl->envs ) ;
	if ( mdl->vid != NULL ) MDU_Free( mdl->vid ) ;
	if ( mdl->nid != NULL ) MDU_Free( mdl->nid ) ;
	if ( mdl->tid != NULL ) MDU_Free( mdl->tid ) ;
	if ( mdl->uvs != NULL ) MDU_Free( mdl->uvs ) ;
    }
    MDU_Free( def2->def ) ;
    MDU_Free( def2 ) ;
}

/* 三角形分割 */
/* ついでに、テクスチャＩＤをＳＴＲＣＯＤＥ変換 */
static	KM3_DEF2	*Km3DividePrimTriangle( olddef2 )
KM3_DEF2		*olddef2 ;
{
    KM3_DEF2	*def2 ;
    KM3_DEF	*olddef, *def ;
    KM3_MDL	*oldmdl, *mdl ;
    int		size, n_x_models, i, j ;
    long      	n_tris;           /*三角形の数*/
    u_short   	*oldvid, *vid ;    
    u_short   	*oldnid, *nid ;
    TVECTOR   	*olduvs, *uvs ;
    u_int   	*oldtid, *tid ;
    long      	oldnum, num ;      /*頂点番号、法線番号、座標ポインタの位置*/
    long      	tri_d ;            /*座標テクスチャポインタの位置*/

    def2 = ( KM3_DEF2 * )MDU_Alloc( sizeof( KM3_DEF2 ) ) ;
    if ( def2 == NULL ) return NULL ;
    olddef = olddef2->def ;
    n_x_models = olddef->n_x_models ;
    size = sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * n_x_models ;
    def = ( KM3_DEF * )MDU_Alloc( size ) ;
    if ( def == NULL ) return NULL ;
    memcpy( def2, olddef2, sizeof( KM3_DEF2 ) ) ;
    memcpy( def, olddef, size ) ;

    def2->def = def ;

    oldmdl = olddef->models ;
    mdl = def->models ;
    for ( i = 0; i < n_x_models; i ++ ) {
	oldvid = oldmdl->vid ;
	oldnid = oldmdl->nid ;
	olduvs = oldmdl->uvs ;
	oldtid = oldmdl->tid ;
	n_tris = 0 ;
	oldnum = 0 ;
	/*   分割後のn_tris の値を求める */
	for( j = 0; j < oldmdl->n_prims ; j ++, oldnum += 4 ) {
	    if( oldvid[ oldnum + 2 ] == oldvid[ oldnum + 3 ] ) n_tris ++ ;
	    else n_tris += 2 ;
	}	
	/*  n_tris分の領域を確保*/
	vid = ( u_short * )MDU_Alloc( sizeof( u_short ) * n_tris * 4 ) ;
	if ( vid == NULL ) return NULL ;
	nid = ( u_short * )MDU_Alloc( sizeof( u_short ) * n_tris * 4 ) ;
	if ( nid == NULL ) return NULL ;
	uvs = ( TVECTOR * )MDU_Alloc( sizeof( TVECTOR ) * n_tris * 4 ) ;
	if ( uvs == NULL ) return NULL ;
	tid = ( u_int * )MDU_Alloc( sizeof( u_int ) * n_tris ) ;
	if ( tid == NULL ) return NULL ;
	mdl->vid = vid ;
	mdl->nid = nid ;
	mdl->uvs = uvs ;
	mdl->tid = tid ;
	mdl->verts = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	if ( mdl->verts == NULL ) return NULL ;
	memcpy( mdl->verts, oldmdl->verts, sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	mdl->envs = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	if ( mdl->envs == NULL ) return NULL ;
	memcpy( mdl->envs, oldmdl->envs, sizeof( FVECTOR ) * oldmdl->n_verts ) ;
	mdl->norms = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * oldmdl->n_norms ) ;
	if ( mdl->norms == NULL ) return NULL ;
	memcpy( mdl->norms, oldmdl->norms, sizeof( FVECTOR ) * oldmdl->n_norms ) ;
	tri_d = 0 ;
	num = 0 ;
	oldnum = 0 ;
	for( j = 0; j < oldmdl->n_prims ; j ++, oldnum += 4 ) {
	/* 頂点番号列の３番目と４番目を比較し、三角形かどうかを判断する */
	    if( oldvid[ oldnum + 2 ] == oldvid[ oldnum + 3 ] ) {
		/*三角形の処理 */
		/* ポリゴンデータを移す */
		tid[ tri_d ] = oldtid[ j ] ;
		vid[ num ] = oldvid[ oldnum ] ; 
		nid[ num ] = oldnid[ oldnum ] ;
		uvs[ num ] = olduvs[ oldnum ] ;
		vid[ num + 1 ] = oldvid[ oldnum + 1 ] ; 
		nid[ num + 1 ] = oldnid[ oldnum + 1 ] ;
		uvs[ num + 1 ] = olduvs[ oldnum + 1 ] ;
		vid[ num + 2 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 2 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 2 ] = olduvs[ oldnum + 2 ] ;
		vid[ num + 3 ] = oldvid[ oldnum + 3 ] ; 
		nid[ num + 3 ] = oldnid[ oldnum + 3 ] ;
		uvs[ num + 3 ] = olduvs[ oldnum + 3 ] ;
		tri_d ++ ;
		num += 4 ;
	    } else {
		tid[ tri_d ] = oldtid[ j ] ;
		vid[ num ] = oldvid[ oldnum ] ; 
		nid[ num ] = oldnid[ oldnum ] ;
		uvs[ num ] = olduvs[ oldnum ] ;
		vid[ num + 1 ] = oldvid[ oldnum + 1 ] ; 
		nid[ num + 1 ] = oldnid[ oldnum + 1 ] ;
		uvs[ num + 1 ] = olduvs[ oldnum + 1 ] ;
		vid[ num + 2 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 2 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 2 ] = olduvs[ oldnum + 2 ] ;
		vid[ num + 3 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 3 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 3 ] = olduvs[ oldnum + 2 ] ;

		tri_d ++ ;
		num += 4 ;
		tid[ tri_d ] = oldtid[ j ] ;
		vid[ num ] = oldvid[ oldnum ] ; 
		nid[ num ] = oldnid[ oldnum ] ;
		uvs[ num ] = olduvs[ oldnum ] ;
		vid[ num + 1 ] = oldvid[ oldnum + 2 ] ; 
		nid[ num + 1 ] = oldnid[ oldnum + 2 ] ;
		uvs[ num + 1 ] = olduvs[ oldnum + 2 ] ;
		vid[ num + 2 ] = oldvid[ oldnum + 3 ] ; 
		nid[ num + 2 ] = oldnid[ oldnum + 3 ] ;
		uvs[ num + 2 ] = olduvs[ oldnum + 3 ] ;
		vid[ num + 3 ] = oldvid[ oldnum + 3 ] ; 
		nid[ num + 3 ] = oldnid[ oldnum + 3 ] ;
		uvs[ num + 3 ] = olduvs[ oldnum + 3 ] ;

		tri_d ++ ;
		num += 4 ;		
	    }
	}
	mdl->n_prims = n_tris ;
#if 1
	for ( j = 0; j < n_tris; j ++ ) {
//	    printf( "%s : ", olddef2->texs[ mdl->tid[ j ] ] ) ;
	    mdl->tid[ j ] = MDU_GetStrCode( olddef2->texs[ mdl->tid[ j ] ] ) ;
//	    printf( "%d\n", mdl->tid[ j ] ) ;
	}
#endif
	mdl ++ ; oldmdl ++ ;
    }
    return def2 ;
}

static	void Strip(void)
{
    long        i;
    DG_MDL      *mdl;
    NEW_OBJECT  *obj;
    KMS_DEF	*sdef ;
    long     n_x_models;
  
    n_x_models = work.dg_def.n_x_models;
    mdl = work.dg_mdl;

    sdef = work.sdef = ( KMS_DEF * )MDU_Alloc( sizeof( KMS_DEF ) +
					       sizeof( KMS_OBJ ) * n_x_models ) ;
    if ( sdef == NULL ) return ;

    work.new_obj = sdef->objs ;

    obj  = work.new_obj ;
    fprintf(stderr,"%4ld /",n_x_models);
    for(i = 0; i < n_x_models; i++,mdl++,obj++)
    {
	Make_Strip(mdl,obj);
	fprintf(stderr,"%4ld,",i);
    }
    fprintf(stderr,"\n");
	
    sdef->n_models = work.dg_def.n_models;
    sdef->n_x_models = work.dg_def.n_x_models;
    sdef->lx       = (float) work.dg_def.lx;
    sdef->ly       = (float) work.dg_def.ly;
    sdef->lz       = (float) work.dg_def.lz;
    sdef->ux       = (float) work.dg_def.ux;
    sdef->uy       = (float) work.dg_def.uy;
    sdef->uz       = (float) work.dg_def.uz;
    
//    memcpy( &( work.new_def ), sdef, sizeof( KMS_DEF ) ) ;

#if 0
    /* 領域開放 */
    for(i = 0; i < n_x_models; i++)
    {
	MDU_Free(work.dg_mdl[i].verts);
	MDU_Free(work.dg_mdl[i].norms);
    }
    MDU_Free(work.dg_mdl);
#endif
    
}

static	void Make_Strip(mdl,obj)
DG_MDL     *mdl;
NEW_OBJECT *obj;
{
    DIV_PRIM   *div_prim;
//    NEW_PACKET *pack;
    STRIP      *strip;
    NEW_TVECTOR *new_uvs;
    POINT2       *point;
    long      n_div_prims;
    long      i,j;
    long     n_pre_strips;

    /* テクスチャを条件によって分割する*/
    Divide_PRIM(mdl);
    div_prim = work.div_prim;
    n_div_prims = work.n_div_prims;
    work.n_strips = 0;
    work.strip = Calloc_STRIP(mdl->n_prims);

    n_pre_strips = 0;
    for(i = 0; i < n_div_prims; i++)
    {	
	n_pre_strips = work.n_strips;
	Strip_Algo_1(&div_prim[i],mdl);
#if 1
	/*頂点数が６４以下かどうかをチェックする*/
	Check_STRIP_Len(n_pre_strips);
#endif
#if 1	
	/* シングルサイド指定の時は以下の処理を
	   行わない */
	if ( !( mdl->type & DG_TYPE_SINGLESIDE ) ) {
	    /*できるだけ頂点をまとめる*/
	    Union_STRIP(n_pre_strips);
	}
#endif
    }
    /*オブジェクトのデータを格納する */
    obj->type    = mdl->type ;
    obj->n_packs = work.n_strips;
    obj->lx      = (float) mdl->lx;
    obj->ly      = (float) mdl->ly;
    obj->lz      = (float) mdl->lz;
    obj->ux      = (float) mdl->ux;
    obj->uy      = (float) mdl->uy;
    obj->uz      = (float) mdl->uz;
    obj->tx      = (float) mdl->tx;
    obj->ty      = (float) mdl->ty;
    obj->tz      = (float) mdl->tz;
    obj->parent  = mdl->parent;
//    obj->extend  = mdl->extend ;

    /*パケットのデータを格納する*/
    obj->pack    = Calloc_NEW_PACKET(work.n_strips);
    strip = work.strip;
    for(i = 0; i < work.n_strips; i ++)
    {
	if ( strip[i].have_env ) obj->pack[ i ].flag = DG_PACKET_ENVELOPE ;
//	obj->pack[i].flag    = strip[i].have_env;
	obj->pack[i].n_verts = strip[i].n_points;
	obj->pack[i].tid     = (u_int)strip[i].tid;
	obj->pack[i].verts   = strip[i].verts;
	obj->pack[i].norms   = strip[i].norms;
	obj->pack[i].uvs     = Calloc_NEW_TVECTOR(strip[i].n_points);
	new_uvs = obj->pack[i].uvs;
	point   = strip[i].point;
	for(j = 0; j < strip[i].n_points; j ++)
	{
//	    new_uvs[j].u = (short) point[j].uvs.u * 4096 / 255;
//	    new_uvs[j].v = (short) point[j].uvs.v * 4096 / 255;
	    new_uvs[j].u = (short)( point[j].uvs.u * 4096 ) ;
	    new_uvs[j].v = (short)( point[j].uvs.v * 4096 ) ;
	}
	
	MDU_Free(strip[i].point);
    }
    /* 領域開放 */
    MDU_Free(strip);
}


/* ストリップをできるだけまとめる */
static	void Union_STRIP(start_num)
long start_num;
{
    long    end_num;
    STRIP   *strip;
    u_short *flag;
    long    max;
    long    n_tmp_verts;
    long    n_rest_verts;
    long    strip_num;
    long    n_strips;
    long    i,j;
    POINT2   *point;
    SVECTOR *verts;
    SVECTOR *norms;

    /*初期化*/
    flag = Calloc_USHORT(work.n_strips);
    n_strips = start_num;
    end_num = work.n_strips;
    strip = work.strip;
    for(i = start_num; i < work.n_strips; i++)
    {
	if(flag[i]) continue;

	/* 初期化 */
	n_tmp_verts = 0;
	n_rest_verts = N_MAX_POINTS;
	point = Calloc_POINT(N_MAX_POINTS);
	verts = Calloc_SVECTOR(N_MAX_POINTS);
	norms = Calloc_SVECTOR(N_MAX_POINTS);
	
	/* 最初のデータを移す */

	/* test */
	strip[i].norms[0].vw = 0x8fff;
	strip[i].norms[1].vw = 0x8fff;
	for(j = 0; j < strip[i].n_points; j++)
	{
	    point[n_tmp_verts] = strip[i].point[j];
	    verts[n_tmp_verts] = strip[i].verts[j];
	    norms[n_tmp_verts] = strip[i].norms[j];
	    n_tmp_verts ++;
	}
	flag[i] = 1;

	MDU_Free(strip[i].point);
	MDU_Free(strip[i].verts);
	MDU_Free(strip[i].norms);

	while(n_tmp_verts < (N_MAX_POINTS - 2))
	{
	    /* 初期化 */
	    n_rest_verts = N_MAX_POINTS - n_tmp_verts;
	    max = 0;
	    strip_num   = -1;
	    /* つなげられるストリップの中で最大のものを選ぶ */
	    for(j = i + 1; j < work.n_strips; j++)
	    {
		if(flag[j]) continue;
		if((strip[j].n_points > max) && (strip[j].n_points <= n_rest_verts))
		{
		    max = strip[j].n_points;
		    strip_num = j;
		}
	    }
	    
	    if(strip_num < 0) break;
	    /* 上で選んだストリップをくっつける */
	    /* くっつける前に非描画領域をセットする */
	    strip[strip_num].norms[0].vw = 0x8fff;
	    strip[strip_num].norms[1].vw = 0x8fff;
	    for(j = 0; j < max; j++)
	    {
		point[n_tmp_verts] = strip[strip_num].point[j];
		verts[n_tmp_verts] = strip[strip_num].verts[j];
		norms[n_tmp_verts] = strip[strip_num].norms[j];
		n_tmp_verts ++;
	    }
	    flag[strip_num] = 1;
	    MDU_Free(strip[strip_num].point);
	    MDU_Free(strip[strip_num].verts);
	    MDU_Free(strip[strip_num].norms);
		
	}
	/* 結果代入*/
	strip[n_strips].point = point;
	strip[n_strips].verts = verts;
	strip[n_strips].norms = norms;
	strip[n_strips].n_points = n_tmp_verts;
	n_strips ++;
    }

    MDU_Free(flag);
    work.n_strips = n_strips;
	    
}
	
/* ストリップの長さをチェックする */
static	void Check_STRIP_Len(start_num)
long start_num;
{
    long     end_num;
    long     n_rest_points;
    POINT2    *point;
    SVECTOR  *verts;
    SVECTOR  *norms;
    long     p_num;
    long     i,j;
    STRIP     *strip;
    char     check;
  
    end_num = work.n_strips;
    strip = work.strip;
    for(i = start_num; i < work.n_strips; i++)
    {
	if(strip[i].n_points <= N_MAX_POINTS) 
	    continue;
	/*初期化*/
	n_rest_points = strip[i].n_points;
	point = strip[i].point;
	verts = strip[i].verts;
	norms = strip[i].norms;
	p_num = 0;

	/*ストリップiの値を新たに作る*/
	strip[i].point = Calloc_POINT(N_MAX_POINTS);
	strip[i].verts = Calloc_SVECTOR(N_MAX_POINTS);
	strip[i].norms = Calloc_SVECTOR(N_MAX_POINTS);
	strip[i].n_points = N_MAX_POINTS;
	for(j = 0; j < N_MAX_POINTS; j++,p_num++)
	{
	    strip[i].point[j] = point[p_num];
	    strip[i].verts[j] = verts[p_num];
	    strip[i].norms[j] = norms[p_num];
	}
	strip[i].n_points = j;
	/* 62,63は次の0,1に対応するので２で調整する */
	n_rest_points -= N_MAX_POINTS;
	if(n_rest_points == 0)
	    continue;
      
	p_num -= 2;
	n_rest_points += 2;
	

	/* 残りのデータを移していく*/
	check = 0;
	while(n_rest_points > N_MAX_POINTS)
	{
	    /*初期化*/
	    strip[end_num].tid = strip[i].tid;
	    strip[end_num].have_env = strip[i].have_env;
	    strip[end_num].point = Calloc_POINT(N_MAX_POINTS);
	    strip[end_num].verts = Calloc_SVECTOR(N_MAX_POINTS);
	    strip[end_num].norms = Calloc_SVECTOR(N_MAX_POINTS);
	    strip[end_num].n_points = N_MAX_POINTS;
	    for(j = 0; j < N_MAX_POINTS; j++,p_num++)
	    {
		strip[end_num].point[j] = point[p_num];
		strip[end_num].verts[j] = verts[p_num];
		strip[end_num].norms[j] = norms[p_num];
	    }
	    /* 62,63は次の0,1に対応するので２で調整する */
	    n_rest_points -= N_MAX_POINTS;
	    strip[end_num].n_points = j;
	    end_num++;
	    if(n_rest_points == 0)
	    {
		check = 1;
		break;
	    }

	    p_num -= 2;
	    n_rest_points += 2;
	    
	}

	/*初期化*/
	if(check == 0)
	{
	    strip[end_num].tid = strip[i].tid;
	    strip[end_num].have_env = strip[i].have_env;
	    strip[end_num].point = Calloc_POINT(n_rest_points);
	    strip[end_num].verts = Calloc_SVECTOR(n_rest_points);
	    strip[end_num].norms = Calloc_SVECTOR(n_rest_points);
#if 0
	strip[end_num].point = Calloc_POINT(N_MAX_POINTS);
	strip[end_num].verts = Calloc_SVECTOR(N_MAX_POINTS);
	strip[end_num].norms = Calloc_SVECTOR(N_MAX_POINTS);
#endif
	    strip[end_num].n_points = n_rest_points;
	    for(j = 0; j < n_rest_points; j++,p_num++)
	    {
		strip[end_num].point[j] = point[p_num];
		strip[end_num].verts[j] = verts[p_num];
		strip[end_num].norms[j] = norms[p_num];
	    }
	    strip[end_num].n_points = j;
	    end_num++;
	}
	MDU_Free(point);
	MDU_Free(verts);
	MDU_Free(norms);
	
    }

    work.n_strips = end_num;
}

/* アルゴリズム１*/
static	void Strip_Algo_1(div_prim,mdl)
DIV_PRIM *div_prim;
DG_MDL   *mdl;
{
    long    n_prims;    /* div_prims内のprimの数 */
    long    n_strips;   /* stripの数 */
    u_short *dummy_flag;
    long    i,j,k;
    STRIP   tmp_strip;
    PRIM    *tmp_prim,*p_tmp_prim;
    long    num;
    u_short vid,nid;
 
    n_prims = div_prim->n_prims;
    n_strips = work.n_strips;
    
    num = n_prims;
    while(num > 0)
    {

        n_prims = num;
	/* フラッグを用意*/
	div_prim->flag = Calloc_USHORT(n_prims);
	/* ダミーのフラッグを用意 */
	dummy_flag = Calloc_USHORT(n_prims);

	work.strip[n_strips].n_points = 0;
	for(i = 0; i < n_prims; i++)
	{
	    for(j = 0; j < 3; j++)
	    {
		tmp_strip = Set_STRIP_1(i,j,div_prim,dummy_flag,RIGHT);
		if(tmp_strip.n_points > work.strip[n_strips].n_points)
		{
		    work.strip[n_strips] = tmp_strip;
		    for(k = 0; k < n_prims; k++)
			div_prim->flag[k] = dummy_flag[k];
		}
		else
		{
		    MDU_Free(tmp_strip.point);
		}
	    }
	    for(j = 0; j < 3; j++)
	    {
		tmp_strip = Set_STRIP_1(i,j,div_prim,dummy_flag,LEFT);
		if(tmp_strip.n_points > work.strip[n_strips].n_points)
		{
		    work.strip[n_strips] = tmp_strip;
		    for(k = 0; k < n_prims; k++)
			div_prim->flag[k] = dummy_flag[k];
		}
		else
		{
		    MDU_Free(tmp_strip.point);
		}
	    }
	}
	/* 分割プリミティブから ストリップへの値の受け渡し */
	work.strip[n_strips].tid = div_prim->tid;
	work.strip[n_strips].have_env = div_prim->have_env;
	work.strip[n_strips].verts = Calloc_SVECTOR(work.strip[n_strips].n_points);
	work.strip[n_strips].norms = Calloc_SVECTOR(work.strip[n_strips].n_points);
	for(i = 0; i < work.strip[n_strips].n_points; i++)
	{
	    vid = work.strip[n_strips].point[i].vid;
	    nid = work.strip[n_strips].point[i].nid;
//	    work.strip[n_strips].verts[i] = mdl->verts[vid];
	    work.strip[n_strips].verts[i].vx = mdl->verts[vid].vx;
	    work.strip[n_strips].verts[i].vy = mdl->verts[vid].vy;
	    work.strip[n_strips].verts[i].vz = mdl->verts[vid].vz;
	    {
		ENVDATA	*ed ;

		ed = ( ENVDATA * )( &( mdl->verts[vid].vw ) ) ;
		if ( ed->parent != 255 ) {
#if 1
		    work.strip[n_strips].verts[i].vw 
			= ( u_short )( mdl->envs[vid].vx * 4096 ) ;
#else
		    work.strip[n_strips].verts[i].vw = 0 ;
#endif
		} else {
		    work.strip[n_strips].verts[i].vw = 4096 ;
		}
	    }
	    
#if 0 
	    if(work.strip[n_strips].verts[i].vw != -1)
	    {
/********* エンベローブの一時しのぎ ***********************/
		parent_vid = work.strip[n_strips].verts[i].vw;
		work.strip[n_strips].verts[i].vx = work.dg_mdl[mdl->parent].verts[parent_vid].vx - mdl->tx;
		work.strip[n_strips].verts[i].vy = work.dg_mdl[mdl->parent].verts[parent_vid].vy - mdl->ty;
		work.strip[n_strips].verts[i].vz = work.dg_mdl[mdl->parent].verts[parent_vid].vz - mdl->tz;
/**********************************************************/
		work.strip[n_strips].verts[i].vw = 0;
	    }
	    else
		work.strip[n_strips].verts[i].vw = 4096;
#endif
//	    work.strip[n_strips].norms[i] = mdl->norms[nid];
	    work.strip[n_strips].norms[i].vx = mdl->norms[nid].vx * 4096.0F ;
	    work.strip[n_strips].norms[i].vy = mdl->norms[nid].vy * 4096.0F ;
	    work.strip[n_strips].norms[i].vz = mdl->norms[nid].vz * 4096.0F ;
	    work.strip[n_strips].norms[i].vw = 0x0fff;  /* 描画領域に初期化 */
	}
	/* 値を更新 */	
	num -= (work.strip[n_strips].n_points - 2); 
	p_tmp_prim = Calloc_PRIM(num);
        tmp_prim = p_tmp_prim;
	for(i = 0; i < n_prims; i++)
	{
	    if(div_prim->flag[i] == 0)
	    {
		*tmp_prim = div_prim->prims[i];
		tmp_prim ++;
	    }
	}
	n_strips ++;
      	MDU_Free(div_prim->flag);
	MDU_Free(div_prim->prims);
	MDU_Free(dummy_flag);

	div_prim->prims = p_tmp_prim;
	div_prim->n_prims = num;
    }
    work.n_strips = n_strips;

    MDU_Free(div_prim->prims);
}

/* ストリップを作る */

static	STRIP Set_STRIP_1(prim_num,verts_num,div_prim,dummy_flag,dir)
long      prim_num;
long      verts_num;
DIV_PRIM  *div_prim;
u_short   *dummy_flag;
int       dir;
{
    long    i,j,d;
    STRIP   tmp_strip;
    u_short *tmp_flag;
    long    n_prims;
    long    n_points;
    POINT2   *strip_point;
    POINT2   *prim_point;
    PRIM    *prim;
    POINT2   line[2];
    int     check;
    int     tmp_num;

    n_prims = div_prim->n_prims;

    tmp_flag = dummy_flag;
    /* 初期化 */
    for(i = 0; i < n_prims; i++,tmp_flag++) *tmp_flag = 0;
    dummy_flag[prim_num] = 1;
    tmp_strip.point = Calloc_POINT(n_prims + 2);
    strip_point = tmp_strip.point;
    prim  = div_prim->prims;

    /* 第１頂点を格納 */
    prim_point = prim[prim_num].point;
    *strip_point = prim_point[verts_num];
    strip_point ++;

    /* 第２頂点を格納 */
    /* strip_point[1] */
    tmp_num = (verts_num + dir + 3) % 3;
    *strip_point = prim_point[tmp_num];
    line[0]  = *strip_point;
    strip_point ++;   
               
    /* 第３頂点を格納 */
    /* strip_point[2] */
    tmp_num = (tmp_num + dir + 3) % 3;
    *strip_point = prim_point[tmp_num];
    line[1] = *strip_point;
    strip_point ++;

    n_points = 3;
   
    while(1)
    { 
	check = 0;
	d = n_points % 2;
	tmp_flag = dummy_flag;
	prim = div_prim->prims;
	for(i = 0; i < n_prims; i++,tmp_flag++,prim++)
	{
	    if(*tmp_flag) continue;
	    prim_point = prim->point;
	    for(j = 0; j < 3; j++)
	    {
		/* 条件を満たせば、くっつける */
		tmp_num = (j + dir + 3) % 3;
		if((P_Equal(&line[d],&prim_point[j]) )
		   && (P_Equal(&line[(d+1) % 2],&prim_point[tmp_num])))
		{
		    *tmp_flag = 1;
		    line[0] = line[1];
		    tmp_num = (tmp_num + dir + 3) % 3;
		    /*strip_point[n_points]*/
		    *strip_point = prim_point[tmp_num];
		    line[1] = *strip_point;
		    strip_point ++;
		
		    n_points ++;
		    check = 1;
		    goto label1;
		}
	    }
	}
	
    label1:
	if(check == 0) break;
    }

    tmp_strip.n_points = n_points;
    strip_point = tmp_strip.point;
    tmp_strip.point = (POINT2 *)MDU_Realloc(strip_point,n_points * sizeof(POINT2));
    if(tmp_strip.point == NULL)
	ERR_PRINT(CALLOC);

    return tmp_strip;
}    

/* テクスチャを条件によって分割する*/
static	void Divide_PRIM(mdl)
DG_MDL    *mdl;
{
    PRIM      *prim;
  
    /* プリミティブを作成する */
    prim = Make_PRIM(mdl);
    /* テクスチャの種類によって分割 */
    Divide_By_TEX(prim,mdl);
    /* 領域開放(プリミティブ) */
    MDU_Free(prim);
    /* エンベロープによる分割 */
    Divide_By_ENV(mdl);
    
}

/* エンベロープの有無による分割 */
static	void Divide_By_ENV(mdl)
DG_MDL *mdl;
{
    /* 新しいプリミティブ */
    DIV_PRIM *new_div_prim;
    long     n_new_div_prims;
    long     n_new_prims;
   
    FVECTOR   *verts;
    PRIM      *prim;
    u_short   *flag;
    DIV_PRIM  *div_prim;
    u_short     vid;
    long      n_div_prims;
    long      n_prims;
    long      i,j,k,d;
    u_short      check;
    u_short      check_env;
    ENVDATA	*ed ;

    /* 初期化 */
    verts = mdl->verts;	
    div_prim = work.div_prim;
    n_div_prims     = work.n_div_prims;
    n_new_div_prims = work.n_div_prims;

    /* 頂点数を調べる */
    for(i = 0; i < n_div_prims; i ++)
    {
	/*設定*/
	prim    = div_prim[i].prims;
	n_prims = div_prim[i].n_prims;
	div_prim[i].flag = Calloc_USHORT(n_prims);
	flag    = div_prim[i].flag;
	check_env = 0;
	for(j = 0; j < n_prims; j++)
	{
	    for(k = 0; k < 3; k++)
	    {
		vid = prim[j].point[k].vid;
		/* エンベロープかどうかを調べる */
		ed = ( ENVDATA * )( &( verts[ vid ].vw ) ) ;
		if ( ed->parent != 255 ) {
		    flag[ j ] = 1 ;
		    check_env = 1 ;
		    goto label_env ;
		}	
#if 0
		if(verts[vid].vw >= 0)
		{  
		    flag[j] = 1;
		    check_env = 1;
		    goto label_env;
		}
#endif
		/* エンベローブがなければ */
		flag[j] = 0;
	    }
	label_env: 
	}
	if(check_env)
	    n_new_div_prims ++;
    }

    /* 分割プリミティブ数が増えていなければそのまま */
    if(n_new_div_prims == n_div_prims)
	return;

    /* 分割する */
    /* 初期化 */
    new_div_prim = Calloc_DIV_PRIM(n_new_div_prims);
    d = 0;

    for(i = 0; i < n_div_prims; i++)
    {
	/* 初期化 */
	prim = div_prim[i].prims;
	flag = div_prim[i].flag;
	n_prims = div_prim[i].n_prims;

	/* 一回目 */
	new_div_prim[d].prims = Calloc_PRIM(n_prims);
	check = 0;
	n_new_prims = 0;
	for(j = 0; j < n_prims; j++)
	{
	    if(flag[j] == 0)
	    {
		new_div_prim[d].prims[n_new_prims] = prim[j];
		n_new_prims ++;
	    }
	    else
		check = 1;
	}
	
	/* 値を代入 */
	new_div_prim[d].n_prims = n_new_prims;
	new_div_prim[d].tid     = div_prim[i].tid;
	new_div_prim[d].have_env = 0;
	d++;

	/* ２回目 */
	if(check)
	{
	    new_div_prim[d].prims = Calloc_PRIM(n_prims);
	    n_new_prims = 0;
	    for(j = 0; j < n_prims; j++)
	    {
		if(flag[j])
		{
		    new_div_prim[d].prims[n_new_prims] = prim[j];
		    n_new_prims ++;
		}
	    }
	    /* 値を代入 */
	    new_div_prim[d].n_prims = n_new_prims;
	    new_div_prim[d].tid     = div_prim[i].tid;
	    new_div_prim[d].have_env = 1;
	    d++;
	}

    }	

    /* 余分な領域を開放 */

    for(i = 0; i < work.n_div_prims; i++)
    {
	MDU_Free(div_prim[i].prims);
	MDU_Free(div_prim[i].flag);
    }
    MDU_Free(div_prim);
    /* workの情報を更新 */
    work.n_div_prims = n_new_div_prims;
    work.div_prim    = new_div_prim;
}
    
/* テクスチャの種類によって分割 */
static	void Divide_By_TEX(prim,mdl)
PRIM    *prim;
DG_MDL  *mdl;
{
    u_short   *flag;
    long      n_div_prims;
    long      i,j;
    long      n;                 /* div_primの数 */
    u_int	   *tid;
    DIV_PRIM  *div_prim;
    
    /* テクスチャ数を調べる */
    flag = Calloc_USHORT(mdl->n_prims);
    tid  = mdl->tid;
    n_div_prims = mdl->n_prims;
    for(i = 0; i < mdl->n_prims; i++)
    {
	if(flag[i]) continue;
	for(j = i + 1; j < mdl->n_prims; j++)
	{
	    if(flag[j]) continue;
	    if(tid[i] == tid[j])
	    {
		flag[j] = 1;
		n_div_prims --;
	    }
	}
    }

    work.n_div_prims = n_div_prims;
    work.div_prim  = Calloc_DIV_PRIM(n_div_prims);
    div_prim = work.div_prim;
    /* フラッグ初期化 */
    for(i = 0; i < mdl->n_prims; i++)
	flag[i] = 0;
    for(i = 0; i < n_div_prims; i++,div_prim++)
    {
	div_prim->prims = Calloc_PRIM(mdl->n_prims);
	n = 0;
	for(j = 0; j < mdl->n_prims; j++)
	{
	    if(flag[j]) continue;
	    if(n == 0)
	    {
		flag[j] = 1;
		div_prim->tid = tid[j];
		div_prim->prims[n] = prim[j];
		n++;
		continue;
	    }
	    if(div_prim->tid == tid[j])
	    {
		flag[j] = 1;
		div_prim->prims[n] = prim[j];
		n++;
		continue;
	    }
	}
	div_prim->n_prims = n;
    } 
    /* 領域開放 */
    MDU_Free(flag);
#if 0
    MDU_Free(mdl->tid);
#endif
}

/* プリミティブを作成する */
static	PRIM *Make_PRIM(mdl)
DG_MDL  *mdl;
{
    PRIM     *prim,*p_prim;
    long     i,j;
    u_short  *vid;
    u_short  *nid;
    TVECTOR  *uvs;

    p_prim = Calloc_PRIM(mdl->n_prims);
    prim   = p_prim;
    vid    = mdl->vid;
    nid    = mdl->nid;
    uvs    = mdl->uvs;

    for(i = 0; i < mdl->n_prims; i++,prim++)
    {
	for(j = 0; j < 4; j++)
	{
	    prim->point[j].vid = *vid;
	    prim->point[j].nid = *nid;
	    prim->point[j].uvs = *uvs;
	    vid ++; nid++; uvs++;
	}
    }
/*  余分な領域の開放(mdl->vid,nid,uvs) */
#if 0
    MDU_Free(mdl->vid);
    MDU_Free(mdl->nid);
    MDU_Free(mdl->uvs);
#endif
    return p_prim;
}
  
KMS_DEF		*MDU_Km32Kms( km3def2 )
KM3_DEF2	*km3def2 ;
{
    KM3_DEF2	*def2 ;
    KMS_DEF	*sdef ;

    printf( ".km3 -> .kms conversion start\n" ) ;
    /* 三角形分割 */
    def2 = Km3DividePrimTriangle( km3def2 ) ;
    work.dg_def = *( def2->def ) ;
    work.dg_mdl = def2->def->models ;
    Strip() ;
    MDU_mdlFreeKm3Def2( def2 ) ;
    sdef = work.sdef ;
    return sdef ;
}
