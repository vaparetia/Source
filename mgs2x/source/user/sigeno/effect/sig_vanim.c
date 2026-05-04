//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sig_vanim.c
	頂点アニメ
	2002/02/28 K.Sigeno
	$Id: sig_vanim.c,v 1.7 2003/01/09 13:14:20 takaki Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"



#ifdef PSX2
/*
	ＰＳ２版ルーチン
*/

int SIG_InitVanim(DG_VERTS_ANIME *wv_anim,DG_OBJS *wobjs,int num)
{
	DG_OBJS		*objs ;
	DG_TEX			*tex ;
	int			i ;

	if(wobjs->n_models != num){
		printf("WARNING !! OBJ MODELS [%d]\n",wobjs->n_models) ;
		ASSERT(0) ;
	}

	/* モデルの共有頂点データ取得 */
	{
		DG_OBJ			*obj ;
		DG_VERTS_ANIME	*v_anime ;
		obj = wobjs->objs ;
		v_anime = wv_anim ;
		for ( i = wobjs->n_models ; i > 0 ; obj++, v_anime++,i-- ){
			v_anime->mem[0] = NULL ;
		}
		obj = wobjs->objs ;
		v_anime = wv_anim ;

		for ( i = wobjs->n_models ; i > 0 ; obj++, v_anime++,i-- ){
			/* 頂点アニメワークの取得 */
//			DG_MakeAnimVertsBuffer( v_anime, obj, DG_VANIME_VERTS|DG_VANIME_SINGLE );
			DG_MakeAnimVertsBuffer( v_anime, obj, DG_VANIME_VERTS );
			if(v_anime->mem[0] == NULL ) return 0 ;
		}
	}
	return 1 ;
}
int SIG_InitVanimPart(DG_VERTS_ANIME *v_anim,DG_OBJ *obj)
{
	/* モデルの共有頂点データ取得 */
	/* 頂点アニメワークの取得 */
//	DG_MakeAnimVertsBuffer( v_anim, obj, DG_VANIME_VERTS|DG_VANIME_SINGLE );
	DG_MakeAnimVertsBuffer( v_anim, obj, DG_VANIME_VERTS );
	if(v_anim->mem[0] == NULL ) return 0 ;
	return 1 ;
}

void SIG_FreeVanim(DG_VERTS_ANIME *v_anime,int num){
	int i;
	DG_VERTS_ANIME *v ;
	v = v_anime ;
	for(i=0;i<num;i++){
		if(v->mem[0] != NULL ) {
			DG_FreeAnimVertsBuffer( v );
		}
		v++ ;
	}
}

#else

/*
	ＸＢＯＸ版ルーチン
*/

/* system/libdg/xvanime.cのDG_MakeAnimVertsBuffer()関数のカスタマイズ版 */
/*
	ＸＢＯＸで採用した共有頂点型フォーマットではストリップ化された
	ポリゴン毎に切り離すことが不可能なので各頂点を独立化した状態で
	データを持つように修正を加えたもの
*/
static void MakeAnimVertsBuffer( DG_VERTS_ANIME *v_anime, DG_OBJ *obj, int flag )
{
	int v_size, u_size, i, j, size ;
	DG_MDL	*mdl ;
	DG_MDLPACK	*mdl_pack ;
	u_long128	*verts0, *verts1 ;

	GV_ZeroMemory( v_anime, sizeof(DG_VERTS_ANIME) );
	v_anime->flag = flag ;
	v_anime->obj = obj ;
	v_anime->buffer_clock = 0 ;

	/* 頂点数計算 */
	mdl = obj->model ;
	mdl_pack = mdl->packs ;
	size = v_size = u_size = 0 ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		size += mdl_pack->n_indices ;
		mdl_pack++ ;
	}

	/* XBOXでは頂点バッファとして確保するため頂点、法線、ＵＶ全てをセットして確保する */
	v_anime->mem[0] = verts0 = DG_AllocLocalVideoMemory( mdl->stride * size );
	if ( flag & DG_VANIME_SINGLE ){
		v_anime->mem[1] = verts1 = verts0 ;
	} else {
		v_anime->mem[1] = verts1 = DG_AllocLocalVideoMemory( mdl->stride * size );
	}
	if ( verts0 == NULL || verts1 == NULL ){
	    if ( verts0 ){
			DG_FreeLocalVideoMemory( verts0 ) ;
			v_anime->mem[0] = NULL ;
	    }
	    return ;
	}

	/* 頂点メモリの割り当て */
	v_anime->verts_top[0] = verts0 ;
	v_anime->verts_top[1] = verts1 ;
	obj->vbuff = v_anime->verts_top[v_anime->buffer_clock] ;

	/* メモリ内容の初期化 */
	if ( obj->stride == sizeof(DG_VERTEX_KMSS) ){
		DG_VERTEX_KMSS	*src, *dst0, *dst1 ;
		u_short			*index ;

		mdl_pack = mdl->packs ;
		src = mdl->vbuff ;
		dst0 = (void*)verts0 ;
		dst1 = (void*)verts1 ;
		index = mdl->packs->index ;
		for ( i = mdl->n_packs ; i > 0 ; i--, mdl_pack++ ){
			for ( j = 0 ; j < mdl_pack->n_indices ; j++ ){
				*dst0 = src[ *index ] ;
				*dst1 = src[ *index ] ;
				dst0++ ;
				dst1++ ;
				index++ ;
			}
		}
	} else {
		DG_VERTEX_KMSM	*src, *dst0, *dst1 ;
		u_short			*index ;

		mdl_pack = mdl->packs ;
		src = mdl->vbuff ;
		dst0 = (void*)verts0 ;
		dst1 = (void*)verts1 ;
		index = mdl->packs->index ;
		for ( i = mdl->n_packs ; i > 0 ; i--, mdl_pack++ ){
			for ( j = 0 ; j < mdl_pack->n_indices ; j++ ){
				*dst0 = src[ *index ] ;
				*dst1 = src[ *index ] ;
				dst0++ ;
				dst1++ ;
				index++ ;
			}
		}
	}

#ifdef KP_WINDOWS
	/* パケット情報の変更 */
	{
		DG_OBJ_PACKET	*pack ;
		WORD			idx ;
		WORD			num ;

		idx      = 0 ;
		pack     = obj->packets ;
		mdl_pack = mdl->packs ;
		for(i=(int)mdl->n_packs; i>0 ; i--, pack++, mdl_pack++)
		{
			num = mdl_pack->n_indices ;

			pack->idx_vtx_min = idx ;	// 最大/最小参照頂点番号が変化してしまう
			pack->idx_vtx_num = num ;

			idx += num ;
		}
	}

	obj->n_verts = size ;
	obj->wflag |= DG_OBJ_WFLAG_ORDERED_INDEX ;	// 0,1,2,...にINDEXが整列しているフラグ
#endif
}
/* ---------------------------------------------------------------- */

/* そのオブジェクトで使用されている展開後の頂点数（＝インデックス数を計算） */
static int CalcVertexNum( DG_OBJ *obj )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*pack ;
	int			i, n_verts ;

	mdl = obj->model ;
	pack = mdl->packs ;
	n_verts = 0 ;
	for ( i = 0 ; i < mdl->n_packs ; i++, pack++ ){
		n_verts += pack->n_indices ;
	}
	return ( n_verts );
}
/* モデル全体での展開後の頂点数を求める */
static int TotalCalcVertexNum( DG_OBJS *objs )
{
	DG_OBJ		*obj ;
	int			i, n_verts ;

	obj = objs->objs ;
	n_verts = 0 ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		n_verts += CalcVertexNum( obj );
	}
	return ( n_verts );
}
/* オブジェクトのインデックス配列を差し替える */
static u_short *SetObjIndex( DG_OBJ *obj, u_short *index )
{
	DG_MDL			*mdl ;
	DG_MDLPACK		*pack ;
	int		i, j, k, count ;

	/* 実際の描画の際に参照されるインデックスの先頭アドレスを差し替える */
	obj->index = index ;
	/* インデックスの初期化 */
	count = 0 ;
	mdl = obj->model ;
	pack = mdl->packs ;
	for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
		for ( k = 0 ; k < pack->n_indices ; k++ ){
			*index++ = count++ ;
		}
	}
#ifdef KP_WINDOWS
	DG_AssignDGObjIndexBuffer( obj ) ;	// Index変更の適用
#endif
	return ( index );
}
/* 差し替えたインデックスを復元する */
static void ResetObjIndex( DG_OBJ *obj )
{
	DG_MDL			*mdl ;

	mdl = obj->model ;
	obj->index = mdl->packs->index ;
#ifdef KP_WINDOWS
	//DG_AssignDGObjIndexBuffer( obj ) ; // Index変更の適用(多分必要無いのでコメントアウト)
#endif
}

/* ---------------------------------------------------------------- */
/* ＸＢＯＸ版では展開された頂点アニメを作成するように変更 */
int SIG_InitVanim(DG_VERTS_ANIME *wv_anim,DG_OBJS *wobjs,int num)
{
	DG_OBJS		*objs ;
	DG_TEX			*tex ;
	int			i ;

	if(wobjs->n_models != num){
		printf("WARNING !! OBJ MODELS [%d]\n",wobjs->n_models) ;
		ASSERT(0) ;
	}

	/* モデルの共有頂点データ取得 */
	{
		DG_OBJ			*obj ;
		DG_VERTS_ANIME	*v_anime ;
		obj = wobjs->objs ;
		v_anime = wv_anim ;

		for ( i = wobjs->n_models ; i > 0 ; obj++, v_anime++,i-- ){
			v_anime->mem[0] = NULL ;
		}
		obj = wobjs->objs ;
		v_anime = wv_anim ;

		for ( i = wobjs->n_models ; i > 0 ; obj++, v_anime++,i-- ){
			/* 頂点アニメワークの取得 */
			//MakeAnimVertsBuffer( v_anime, obj, DG_VANIME_VERTS|DG_VANIME_SINGLE );/* 本当は良くない */
			MakeAnimVertsBuffer( v_anime, obj, DG_VANIME_VERTS );
			if(v_anime->mem[0] == NULL ) return 0 ;
		}
	}
	return 1 ;
}
/*OBJS ではなくOBJ単位でのアニメワーク生成*/
int SIG_InitVanimPart(DG_VERTS_ANIME *v_anim,DG_OBJ *obj)
{
	/* 頂点アニメワークの取得 */
	MakeAnimVertsBuffer( v_anim, obj, DG_VANIME_VERTS );
	if(v_anim->mem[0] == NULL ) return 0 ;
	return 1 ;
}

void SIG_FreeVanim(DG_VERTS_ANIME *v_anime,int num){
	int i;
	DG_VERTS_ANIME *v ;
	v = v_anime ;
	for(i=0;i<num;i++){
		if(v->mem[0] != NULL ) {
			DG_FreeAnimVertsBuffer( v );
		}
		v++ ;
	}
}

/* 展開されたインデックスを割り当てメモリアドレスを返す */
void *SIG_SetExpansionIndex( DG_OBJS *objs )
{
	u_short		*index, *mem ;
	int			i ;

	mem = GV_Malloc( TotalCalcVertexNum( objs ) * sizeof(short) );
	index = mem ;
	for ( i = 0 ; i < objs->n_models ; i++ ){
		index = SetObjIndex( &objs->objs[ i ], index );
	}
	return ( mem );
}
/* 展開インデックスを元に戻す */
void SIG_ResetExpansionIndex( DG_OBJS *objs )
{
	int			i ;
	for ( i = 0 ; i < objs->n_models ; i++ ){
		ResetObjIndex( &objs->objs[ i ] );
	}
}
/* 単関節での展開されたインデックスを割り当てメモリアドレスを返す */
void *SIG_SetExpansionIndexSingle( DG_OBJ *obj )
{
	u_short		*index, *mem ;
	int			i ;

	mem = GV_Malloc( CalcVertexNum( obj ) * sizeof(short) );
	index = mem ;
	index = SetObjIndex( obj, index );
	return ( mem );
}
/* 単関節での展開インデックスを元に戻す */
void SIG_ResetExpansionIndexSingle( DG_OBJ *obj )
{
	ResetObjIndex( obj );
}

#endif
