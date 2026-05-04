//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breakpart.c
	多間接モデルの1部のみ変形
	2002/06/28 K.Sigeno
	$Id: breakpart.c,v 1.2 2002/11/25 14:15:53 takaki Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "../effect/sig_vanim.h"


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

extern float _RsinF( int ) ;

#define NORMAL_MODE			(0x00000000)
#define BUILD_Y				(0x00000002)
#define BUILD_UP			(0x00000004)	/*足から変形開始*/
#define NO_CHECK_SW			(0x00000008)
#define DISAPPEAR_MODE		(0x00000010) /*実体から消えへ デフォルトは無から実体へ*/
#define BODY_MODE			(0x00000020)
#define BUILD_X				(0x00000040)
#define BUILD_ROLL			(0x00000080)
#define BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
#define MOTION_TR			(0x00000200) /*モーション追随*/
#define BUILD_RND			(0x00000400)	/*ノイジー*/
#define NO_MOVE_VEC			(0x00000800)	/*スケール加工のみ*/
#define NO_DELAY			(0x00001000)	/*全頂点同時*/
#define NO_MAKE_OBJ			(0x00002000)	/*既存OBJを加工*/
#define NO_WEIGHT			(0x00004000)	/*既存OBJを加工*/
#define ABS_WORLD			(0x00008000)	/*移動方向が絶対座標系*/


/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	vanim;
	DG_OBJ		*obj ;
	SVECTOR		rot ;

#ifdef KP_WINDOWS
	float	vanim_pre_rate ;	// 高速化の為
#endif

	float		len ;
	float		len1 ;
	float		len2 ;
	float		len_base ;

	int			tr_cnt ;
	int			time_a ;
	int			time_a1 ;
	int			time_a2 ;

	float		min ;
	float		max ;
	float		rate ;
	float		scale ;

	float		scale1 ;
	float		scale2 ;
	float		scale_dif ;
	u_int			mode ;

	int			*sw ;
	int			cnt ;
	int			re_time ;
	int			re_cnt ;

	int			ver ;	
	int			status ;
	u_short		*index ;			/* 差し替え用インデックス */

} Work ;

#define STATUS_NORMAL	(00)
#define STATUS_DMG		(01)
#define STATUS_DBROKEN	(02)

#define VER_01 (0x01)
#define VER_02 (0x02)

#define VER_01_MASK			(0x0000ffff)
#define VER_02_REC_MASK		(0x0000ffff)
#define VER_02_SEND_NORMAL	(0x00000000)
#define VER_02_SEND_DAMAGE	(0x00010000)
#define VER_02_SEND_BROKEN	(0x00020000)


#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)
#if 0
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif

//#define		ROT_VY	(800)
#define		ROT_VY	(200)

#define		ROT_VX	(0)
#define		WAVE_RATE	(0.5f)
/* ---------------------------------------------------------------- */
/* 計算系ユーティリティ */
/* ベクトルの指定長へのスケール */
static void ScaleNormalize( FVECTOR *res, FVECTOR *vec, float len )
{
	float	lenlen, scale ;
	lenlen = vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz ;
	scale = len / DG_SQRT( lenlen );
	res->vx = vec->vx * scale ;
	res->vy = vec->vy * scale ;
	res->vz = vec->vz * scale ;
}
/* ---------------------------------------------------------------- */



static void SetBoundScale(Work *work,DG_OBJ *obj ,float size) {

	work->min = obj->bound_min.vy ;
	work->max = obj->bound_max.vy ;

	/*バウンディングボックスも拡大*/
	obj->bound_min.vx -= size ;
	obj->bound_min.vy -= size ;
	obj->bound_min.vz -= size ;
	obj->bound_max.vx += size ;
	obj->bound_max.vy += size ;
	obj->bound_max.vz += size ;

}
static void ResetBoundScale(Work *work,DG_OBJ *obj ,float size) {

	obj->bound_min.vx += size ;
	obj->bound_min.vy += size ;
	obj->bound_min.vz += size ;
	obj->bound_max.vx -= size ;
	obj->bound_max.vy -= size ;
	obj->bound_max.vz -= size ;

}
#ifdef PSX2
static inline int SetDelayCnt(int cnt ,SVECTOR *org_sv,Work *work){
#else
static inline int SetDelayCnt(int cnt ,FVECTOR *org_sv,Work *work){
#endif
	int tmp_cnt ;
	if(cnt < 0){
		if(work->mode &DISAPPEAR_MODE){
			tmp_cnt = work->time_a ;
		}else {
			tmp_cnt = 0 ;
		}
	}else {
		tmp_cnt = cnt ;
#if 1
		tmp_cnt -= (org_sv->vy - work->min)/work->rate  ;
#else
		tmp_cnt -= (work->max - org_sv->vy )/work->rate  ;
#endif
	}
	if(tmp_cnt<0) tmp_cnt = 0 ;
	if(tmp_cnt>work->time_a) tmp_cnt = work->time_a ;
	return tmp_cnt ;
}


//#define WEIGHT_BORDER (0.80f) //これ以下はゼロにする
#define WEIGHT_BORDER (0.85f) //これ以下はゼロにする

static inline float SetWeightRate(float rate){
	if(rate < WEIGHT_BORDER) return 0.0f ;
	return (rate-WEIGHT_BORDER)/(1.0f-WEIGHT_BORDER) ;
}
#define BUILD_MODE (BUILD_NORM)
#define ROLL_X_RATE (8)

/* 移動方向ベクトルの生成 */
#ifdef PSX2
#define BUILD_MODE (BUILD_NORM)
#define ROLL_X_RATE (8)

static inline void SetMoveVec(Work *work,SVECTOR *buf,int speed ,SVECTOR *norms){
	FVECTOR tmp ;

	if(work->mode &NO_MOVE_VEC){
		buf->vx = 0 ;
		buf->vy = 0 ;
		buf->vz = 0 ;
		return ;
	}

	if((work->mode & BUILD_Y)||(work->mode & BUILD_X)){
		if(work->mode & BUILD_X){
			if(work->mode & BUILD_ROLL){
				tmp.vx=(-speed/ROLL_X_RATE) ;
			}else {
				tmp.vx= -speed ;
			}
		}else {
			tmp.vx = 0.0f ;
		}
		if(work->mode & BUILD_Y){
			if(work->mode & BUILD_REV){
				tmp.vy = speed ;
			}else {
				tmp.vy =-speed ;
			}
		}else {
			tmp.vy =0.0f ;
		}
		tmp.vz=0.0f;
		DG_RotVector(&tmp,&tmp,1) ;
		buf->vx = tmp.vx ;
		buf->vy = tmp.vy ;
		buf->vz = tmp.vz ;
	}else if(work->mode & BUILD_RND){
		SVECTOR  test ;
		test.vx = ((irnd()>>8)& 255) - 127 ;
		test.vy = ((irnd()>>8)& 255) - 127 ;
		test.vz = ((irnd()>>8)& 255) - 127 ;

		GV_LenVec3( &test, buf, 1.0f, speed ) ;
	}else {
		GV_LenVec3( norms, buf, 1.0f, speed ) ;
		tmp.vx = buf->vx ;
		if(work->mode & BUILD_REV){
			tmp.vy = buf->vy + speed ;
		}else {
			tmp.vy = buf->vy - speed ;
		}
		tmp.vz = buf->vz ;
		DG_RotVector(&tmp,&tmp,1) ;
		buf->vx = tmp.vx ;
		buf->vy = tmp.vy ;
		buf->vz = tmp.vz ;
	}
}



#else
static inline void SetMoveVec( Work *work, FVECTOR *buf, int speed, FVECTOR *norm )
{
	FVECTOR tmp ;

	if(work->mode &NO_MOVE_VEC){
		buf->vx = 0 ;
		buf->vy = 0 ;
		buf->vz = 0 ;
		return ;
	}

	if((work->mode & BUILD_Y)||(work->mode & BUILD_X)){
		if(work->mode & BUILD_X){
			if(work->mode & BUILD_ROLL){
				tmp.vx=(-speed/ROLL_X_RATE) ;
			}else {
				tmp.vx= -speed ;
			}
		}else {
			tmp.vx = 0.0f ;
		}
		if(work->mode & BUILD_Y){
			if(work->mode & BUILD_REV){
				tmp.vy = speed ;
			}else {
				tmp.vy =-speed ;
			}
		}else {
			tmp.vy =0.0f ;
		}
		tmp.vz=0.0f;
		DG_RotVector(&tmp,buf,1) ;
	}else if(work->mode & BUILD_RND){
		FVECTOR  test ;
		test.vx = ((irnd()>>8)& 255) - 127 ;
		test.vy = ((irnd()>>8)& 255) - 127 ;
		test.vz = ((irnd()>>8)& 255) - 127 ;

		ScaleNormalize( buf, &test, speed );
	}else {
		ScaleNormalize( buf, norm, speed );
#if 0
		if(work->mode & BUILD_REV){
			buf->vy += speed ;
		}else {
			buf->vy -= speed ;
		}
#endif
		DG_RotVector( buf, buf, 1 ) ;
	}
}
#endif


/*分解時の離れ距離を決定*/
static int SetMoveLen(Work *work,int tmp_cnt){
#if 0
	/*２次曲線*/
	return (work->len*(float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a)) ;
#else
//printf("len[%f] time_a[%d] tmp_cnt[%d] \n",work->len,work->time_a,tmp_cnt);
	return (work->len*(float)(work->time_a-tmp_cnt)/(float)(work->time_a)) ;
#endif
}

#ifdef PSX2
#define Q_WORD (2)

static void SIG_ActVanimObjBuild(Work *work,int cnt,int clock) {
//xbox
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	FVECTOR ef_pos ;
	float rate,rate_dif ;
	float	tmp_speed,weight,weight_dif;
	int n_verts,i,j,k,l,kick,tmp_cnt,rank;
	int	verts_off,norm_off, speed ;
	DG_OBJ	*obj;
	FVECTOR	tmp;
	FMATRIX	world ;
#ifdef PSX2
	SVECTOR *sv,*tmp_sv,*org_sv,*trg_sv,buf,rot;
#else
//xbox
	DG_VERTEX_KMSM *sv ;
	SVECTOR *tmp_sv,*org_sv,*trg_sv,buf,rot;
#endif
	SVECTOR *norms ;	/* 法線配列バッファ */
	short	n_packs ;
//	DG_OBJ_PACKET	*packets ;
	obj = work->obj ;


//return ;

	if(work->mode & BUILD_ROLL){
//printf("work->rot + 10\n");
		work->rot.vy += ROT_VY  ;
		work->rot.vy &= 4095 ;
		work->rot.vx += ROT_VX  ;
		work->rot.vx &= 4095 ;
//printf("work->rot\n");
	}



	{
		mdl = work->vanim.obj->model;
		mdl_pack = mdl->packs;
#if 1
		if(cnt < 0){
			if(work->mode &DISAPPEAR_MODE){
				tmp_cnt = work->time_a ;
			}else {
				tmp_cnt = 0 ;
			}
		}else {
			tmp_cnt = cnt  ;
		}
#endif
#if 0
		if((tmp_cnt>(work->time_a+1))||(tmp_cnt<0)){
			return ;
		}
#else 
		if(tmp_cnt>work->time_a){
			tmp_cnt = work->time_a ;
		}else if(tmp_cnt<0){
			tmp_cnt = 0 ;
		}
#endif
		speed = SetMoveLen(work,tmp_cnt);
//printf("VERTS LEN [%d]\n",speed);
		n_packs = obj->n_packs ;
		verts_off = 0 ;
		norm_off = 0 ;

		if(work->mode & ABS_WORLD){
			_sceVu0InversMatrix(&world ,&obj->world) ;
			DG_SetPos(&world) ;
		}else {
			DG_SetPos(&obj->world) ;
		}
		if(work->mode & BUILD_ROLL){
			DG_RotatePos(&work->rot) ;
		}
		for(k=0;k<mdl->n_packs;k++){
			if( 1){
				sv = &obj->verts[verts_off*Q_WORD] ;
//				sv = work->vanim.verts_top[clock];

				org_sv = (SVECTOR*)obj->model->packs[k].verts ;
				norms = (SVECTOR*)obj->model->packs[k].norms ;

#if 0
				tmp_cnt = SetDelayCnt(cnt ,sv,work) ;
				speed = SetMoveLen(work,tmp_cnt);
#endif
				n_verts = obj->packets[k].n_verts ;
				SetMoveVec(work,&buf,speed,norms) ;
				rate = (float)tmp_cnt/(float)work->time_a ;
				rate *= work->scale_dif ; rate += work->scale ;
				rate_dif = 1.0f - rate ;
				weight = SetWeightRate(rate) ;
				weight_dif = 1.0f - weight ;
				kick = 1 ;
				trg_sv = sv ;
				for (i=0;i<mdl_pack->n_verts;i++){
					/*頂点キックのみが2回続くならポリゴン切断と判断*/
					if((norms->pad&0xffff)== 0x8fff){
						if(i<(n_verts-1)){
							norms++ ;
							if((norms->pad&0xffff)== 0x8fff){
								kick++ ;
								trg_sv = sv ;

#if 0
								tmp_cnt = SetDelayCnt(cnt ,trg_sv,work) ;
								speed = SetMoveLen(work,tmp_cnt);
#endif
								SetMoveVec(work,&buf,speed,norms) ;
								rate = (float)tmp_cnt/(float)work->time_a ;
								rate *= work->scale_dif ; rate += work->scale ;
								rate_dif = 1.0f - rate ;
								weight = SetWeightRate(rate) ;
								weight_dif = 1.0f - weight ;
							}
							norms-- ;
						}
					}
					{
						if(tmp_cnt == work->time_a){
							sv->vx = org_sv->vx;
							sv->vy = org_sv->vy;
							sv->vz = org_sv->vz;
						}else {
							if(kick){
								sv->vx = org_sv->vx - buf.vx ;
								sv->vy = org_sv->vy - buf.vy ;
								sv->vz = org_sv->vz - buf.vz ;
							}else {
								sv->vx = org_sv->vx - buf.vx ;
								sv->vy = org_sv->vy - buf.vy ;
								sv->vz = org_sv->vz - buf.vz ;
								sv->vx = (float) sv->vx*rate + (float)trg_sv->vx * rate_dif ;
								sv->vy = (float) sv->vy*rate + (float)trg_sv->vy * rate_dif ;
								sv->vz = (float) sv->vz*rate + (float)trg_sv->vz * rate_dif ;
							}
						}
					}
					sv->pad = (((float)org_sv->pad/4096.0f) * weight )*4096.0f + (4096.0f*weight_dif);
					org_sv++ ;
					sv++ ;
					norms++ ;
					kick = 0 ;
				}
			}
			verts_off += obj->packets[k].verts_offset ;
			norm_off += obj->packets[k].norms_offset ;
			mdl_pack++ ;
		}
	}
}

#else
static void XSIG_ActVanimObjBuild(Work *work,int cnt,int clock) {
	DG_VERTS_ANIME	*vanim ;
	DG_VERTEX_KMSM	*d_verts, *org_verts, *s_verts ;
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	DG_OBJ		*obj;
	float		rate,rate_dif ;
	float		weight,weight_dif;
	int			n_verts, i,  k, l, tmp_cnt ;
	int			speed ;
	FMATRIX		world ;
	SVECTOR		rot ;
	u_short			*index, top_index ;
	FVECTOR			*verts, *top_verts, norm, move_vec ;

	obj = work->obj ;


	if(work->mode & BUILD_ROLL){
//printf("work->rot + 10\n");
		work->rot.vy += ROT_VY  ;
		work->rot.vy &= 4095 ;
		work->rot.vx += ROT_VX  ;
		work->rot.vx &= 4095 ;
//printf("work->rot\n");
	}


	{
		vanim = &work->vanim;
		if(cnt < 0){
			if(work->mode &DISAPPEAR_MODE){
				tmp_cnt = work->time_a ;
			}else {
				tmp_cnt = 0 ;
			}
		}else {
			tmp_cnt = cnt ;
		}
#if 0
		if((tmp_cnt>(work->time_a+1))||(tmp_cnt<0)){
			return ;
		}
#else 
		if(tmp_cnt>work->time_a){
			tmp_cnt = work->time_a ;
		}else if(tmp_cnt<0){
			tmp_cnt = 0 ;
		}
#endif


		/* 移動量及び変形パラメータを決めておく */
		speed = SetMoveLen(work,tmp_cnt);

#if 0
		rate = (float)tmp_cnt/(float)work->time_a ;
		rate_dif = 1.0f - rate ;
#else
		rate = (float)tmp_cnt/(float)work->time_a ;
		rate *= work->scale_dif ; rate += work->scale ;
		rate_dif = 1.0f - rate ;
#endif

		weight = SetWeightRate(rate) ;
		weight_dif = 1.0f - weight ;




		if(work->mode & ABS_WORLD){
			_sceVu0InversMatrix(&world ,&obj->world) ;
			DG_SetPos(&world) ;
		}else {
			DG_SetPos(&obj->world) ;
		}


		if(work->mode & BUILD_ROLL){
			DG_RotatePos(&work->rot) ;
		}
		d_verts = vanim->verts_top[ clock ];

#ifndef KP_WINDOWS
		if( work->vanim_pre_rate == rate ){ return ; }	// 高速化...
		work->vanim_pre_rate = rate ;
#endif

		mdl = vanim->obj->model ;
		mdl_pack = mdl->packs ;
		org_verts = mdl->vbuff ;
		for ( k = 0 ; k < mdl->n_packs ; k++, mdl_pack++ ){

			index = mdl_pack->index ;

			/* 展開頂点データ生成ワーク初期化 */
			verts = (FVECTOR*)SCRPAD_ADDR ;
			top_index = index[0] ;
			n_verts = 0 ;

			for ( i = 0 ; i < mdl_pack->n_indices ; i++, index++ ){
				/* インデックスをチェックしながら展開された頂点を作成していく */
				s_verts = (void*)( (char*)org_verts + mdl->stride * (*index) );
				verts->vx = s_verts->vx ;
				verts->vy = s_verts->vy ;
				verts->vz = s_verts->vz ;
#if 1
				verts->vw = (float)s_verts->wt ;
#endif
				verts++ ;
				n_verts++ ;
				if ( i < 2 ) continue ;/* 最初の２点は無視する */
				/* ストリップの切れ目だった場合に移動処理を行なう */
				if ( ( i == ( mdl_pack->n_indices - 1 ) ) || 
					( index[ -1 ] == index[ 0 ] && index[ 1 ] == index[ 2 ] ) ){

					/* 移動量ベクトルの計算 */
					norm.vx = org_verts[ top_index ].nx ;
					norm.vy = org_verts[ top_index ].ny ;
					norm.vz = org_verts[ top_index ].nz ;
					//	short		wt ;			/* ウェイトデータ */
					//speed = work->len *
					//  (float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;
					SetMoveVec( work, &move_vec, speed, &norm ) ;
					//rate = (float)tmp_cnt/(float)work->time_a ;
					//rate_dif = 1.0f - rate ;
					//weight = SetWeightRate(rate) ;
					//weight_dif = 1.0f - weight ;

					/* 切り出した１ストリップ分の頂点に対して移動計算 */
					verts = top_verts = (FVECTOR*)SCRPAD_ADDR ;
					for ( l = 0 ; l < n_verts ; l++, verts++ ){
//						if ( tmp_cnt == work->time_a ){
						if ( tmp_cnt >= work->time_a ){
							/* アニメーションしない場合 */
							d_verts->vx = verts->vx;
							d_verts->vy = verts->vy;
							d_verts->vz = verts->vz;
							d_verts->wt = (short)(verts->vw) ;
						} else {
							if ( l == 0 ){
								/* 先頭の頂点 */
								verts->vx = verts->vx - move_vec.vx ;
								verts->vy = verts->vy - move_vec.vy ;
								verts->vz = verts->vz - move_vec.vz ;
							}else {
								/* 先頭の頂点以外は先頭頂点への縮退処理を伴う */
								verts->vx = verts->vx - move_vec.vx ;
								verts->vy = verts->vy - move_vec.vy ;
								verts->vz = verts->vz - move_vec.vz ;
								verts->vx = verts->vx * rate + top_verts->vx * rate_dif ;
								verts->vy = verts->vy * rate + top_verts->vy * rate_dif ;
								verts->vz = verts->vz * rate + top_verts->vz * rate_dif ;
							}
							d_verts->vx = verts->vx ;
							d_verts->vy = verts->vy ;
							d_verts->vz = verts->vz ;
#if 0
							d_verts->wt = 32767 ;
#else
//							d_verts->wt = (short)(32767.0f * weight_dif) + (short) (verts->vw * weight) ;
//	sv->pad = (((float)org_sv->pad/4096.0f) * weight )*4096.0f + (4096.0f*weight_dif);
#endif
						}
						d_verts = (void*)( (char*)d_verts + mdl->stride ) ;

					}

					/* 展開頂点データ生成ワーク初期化 *//* 次の処理のための初期化 */
					verts = (FVECTOR*)SCRPAD_ADDR ;
					top_index = index[1] ;
					n_verts = 0 ;
				}
			}
		}
#ifdef KP_WINDOWS
		DG_AssignDGObjVertexBuffer(obj) ;
#endif
	}
}
#endif

static void StartDmgMode(Work *work){
	work->re_cnt = work->re_time ;
#if 0
	work->time_a = work->time_a1 ;
//	if(work->status != STATUS_DMG){
	if(work->cnt > work->time_a){
		work->cnt = work->time_a ;
	}
#else
	work->cnt = work->cnt * work->time_a1 / work->time_a ;
	work->time_a = work->time_a1 ;
#endif
//	work->scale = work->scale1 ;
//	work->len = work->len1 ;
	work->status = STATUS_DMG ;
}
static void StartBrkMode(Work *work){
#if 0
	work->time_a = work->time_a1 ;
//	if(work->status != STATUS_DBROKEN){
	if(work->cnt > work->time_a){
		work->cnt = work->time_a ;
	}
#else
	work->cnt = work->cnt * work->time_a1 / work->time_a ;
	work->time_a = work->time_a1 ;
#endif


	work->status = STATUS_DBROKEN ;
}
static void StartNomMode(Work *work){

#if 0
	work->time_a = work->time_a2 ;
#else
	work->cnt = work->cnt * work->time_a2 / work->time_a ;
	work->time_a = work->time_a2 ;
#endif
	work->status = STATUS_NORMAL ;

}

static void SetScaleInterp(float *final ,float *source , float interp){
	float dif ;
	dif = *final - *source ;
//	printf("source [%f] final [%f] ")
	*source += dif /interp ;
}
#define INTERP	(8.0f)
static void Act(Work *work)
{

	float width ,dif;
	int i,n_body;

//	GV_DestroyActor(work) ;
//printf("work->cnt[%d]\n",work->cnt);

	DG_SwitchVAnimeBuffer( &work->vanim );
	DG_RestartAnimVerts( &work->vanim );

#ifdef PSX2
	SIG_ActVanimObjBuild(work,work->cnt,work->vanim.buffer_clock) ;
#else
	XSIG_ActVanimObjBuild(work,work->cnt,work->vanim.buffer_clock) ;
#endif

	if(work->ver == VER_01){
		if((*work->sw & VER_01_MASK)== 1 ) {
			if(work->cnt > 0){
				work->len = work->len_base ;
				work->cnt-- ;
			}else {
				work->len = work->len_base + (work->len_base*WAVE_RATE)* _RsinF( work->tr_cnt ) ;
			}
		}else if((*work->sw & VER_01_MASK)== 0 ) {
			work->len = work->len_base ;
			if(work->cnt < (work->time_a)){
				work->cnt++ ;
			}
		}
	}else if(work->ver == VER_02){


#if 0
	if(GV_PadData[ 0 ].press & PAD_Y){
		*work->sw = 0 ;
	}
	if(GV_PadData[ 0 ].press & PAD_X){
		*work->sw = 1 ;
	}
	if(GV_PadData[ 0 ].press & PAD_B){
		*work->sw = 2 ;
	}
#endif

//mask
		if(((*work->sw)&VER_02_REC_MASK) == STATUS_NORMAL ) {
			/*NOP*/
		}else if(((*work->sw)&VER_02_REC_MASK) == STATUS_DMG ) {
			/*ダメージリクエスト*/
			/*ノーマルモードから壊れモードへ*/
			if(work->status != STATUS_DBROKEN){
//			if(1){
				StartDmgMode(work) ;
			}
		}else if(((*work->sw)&VER_02_REC_MASK) == STATUS_DBROKEN ) {
			if(work->status != STATUS_DBROKEN){
//			if(1){
				StartBrkMode(work) ;
			}
		}
		*work->sw = 0 ;

		switch(work->status){
			case STATUS_NORMAL :
				/*通常へ復帰*/
				work->len = work->len_base ;
				if(work->cnt < (work->time_a)){
					work->cnt++ ;
				}
				SetScaleInterp(&work->scale1,&work->scale,INTERP) ;
				SetScaleInterp(&work->len1,&work->len_base,INTERP) ;
				*work->sw = VER_02_SEND_NORMAL ;
				break ;
			case STATUS_DMG :
				/*ダメージ*/
				if(work->cnt > 0){
					work->len = work->len_base ;
					work->cnt-- ;
				}else {
					work->len = work->len_base + (work->len_base*WAVE_RATE)* _RsinF( work->tr_cnt ) ;
				}
				if(work->re_cnt == 0){
					StartNomMode(work) ;
				}else if(work->re_cnt > 0) {
					work->re_cnt-- ;
				}
				SetScaleInterp(&work->scale1,&work->scale,INTERP) ;
				SetScaleInterp(&work->len1,&work->len_base,INTERP) ;
				*work->sw = VER_02_SEND_DAMAGE ;
				break ;
			case STATUS_DBROKEN :
				/*壊れ*/
				if(work->cnt > 0){
					work->len = work->len_base ;
					work->cnt-- ;
				}else {
					work->len = work->len_base + (work->len_base*WAVE_RATE)* _RsinF( work->tr_cnt ) ;
				}
				SetScaleInterp(&work->scale2,&work->scale,INTERP) ;
				SetScaleInterp(&work->len2,&work->len_base,INTERP) ;
				*work->sw = VER_02_SEND_BROKEN ;
//printf("work->len_base[%f]\n",work->len_base);
				break ;
		}
	}
	work->scale_dif = 1.0f - work->scale ;

	work->tr_cnt+=200 ;
	work->tr_cnt &= 4095 ;
}
static void Die(Work *work)
{
	if(work->index != NULL ) {
		ResetBoundScale(work,work->obj ,fabs(work->len)) ;
#ifndef PSX2
		SIG_ResetExpansionIndexSingle( work->obj );
		GV_Free( work->index );
#endif
	}
	SIG_FreeVanim(&work->vanim,1) ;
}

//static int GetResources(Work *work, DG_OBJ *obj,float len, int mode ,int time_a,int *sw,float scale)
static int GetResources(Work *work, DG_OBJ *obj,float len1,float len2, int mode ,
	int time_a1,int time_a2,int *sw,float scale1,float scale2,int re_time)
{
	int i;
	float max_len ;
	void *v_addr1,*v_addr2 ;
	work->len = work->len1 =work->len_base =  len1 ;
	work->len2 =len2 ;
	work->time_a = work->time_a1 = DIRECT_TICK(time_a1) ;
	work->time_a2 = DIRECT_TICK(time_a2) ;
	work->rot.vx = work->rot.vy = work->rot.vz = 0 ;
	work->obj = obj ;
	work->sw = sw ;
	work->re_cnt = 0 ;
	work->scale = work->scale1 = scale1 ;
	work->scale2 = scale2 ;
	work->scale_dif = 1.0f - scale1 ;

	work->re_time = re_time ;
#if 0
	v_addr1 = &work->obj->verts[0] ;
printf("ORG ADDR [%x]\n",v_addr1) ;
#endif
	work->mode = mode;

	/* ＸＢＯＸ版では展開された状態での頂点アニメワークを作成 */
	if(SIG_InitVanimPart(&work->vanim,work->obj)==0){
		return 0 ;
	}

#ifdef KP_WINDOWS
	work->vanim_pre_rate = -1.0f ;

	/* Windows版はデータ展開用専用VertexBufferを作成する必要あり */
	{
		DG_OBJ	*obj ;

		obj = work->obj ;
		if( obj->n_indices )
		{
			DG_CreateDGObjPrivateVertexBuffer(obj, obj->n_indices, obj->n_indices) ;
		}
	}
#endif

#ifndef PSX2 
	work->index = SIG_SetExpansionIndexSingle( work->obj );
	if(work->index == NULL ) return 0 ;
#endif
	
	max_len = fabs(work->len1) ; 
	if(max_len < fabs(work->len2)) max_len = fabs(work->len2) ;
	SetBoundScale(work,work->obj ,max_len) ;

	work->rate = (work->max - work->min) / 30.0f ;

	if(work->mode &DISAPPEAR_MODE){
		work->cnt = (work->time_a) ;
	}else {
		work->cnt = 0 ;
	}


	work->tr_cnt = work->cnt ;

#ifdef PSX2 
	SIG_ActVanimObjBuild(work,-1,0) ;
	SIG_ActVanimObjBuild(work,-1,1) ;
#endif
	work->status = STATUS_NORMAL ;
	return 1;
}

void *NewSigBreakPart(DG_OBJ *obj,float len1 ,float len2 ,int mode,int *sw,int time_a1,int time_a2,
	float scale1,float scale2,int re_time,int ver )
{
	Work *work ;
	int def_mode = (DISAPPEAR_MODE|BUILD_ROLL) ;
//	int def_mode = (DISAPPEAR_MODE|BUILD_RND) ;

	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
//		if(!GetResources(work,obj,len,def_mode , time_a ,sw,scale)){
		if(!GetResources(work,obj,len1,len2,def_mode , time_a1 ,time_a2 ,sw,scale1,scale2,re_time)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		work->ver = ver ;
	}
	return (void *)work ;
}
void *NewGolBreakPart(DG_OBJ *obj,float len ,int *sw,int time_a,float scale){
	int mode = (DISAPPEAR_MODE|BUILD_ROLL) ;
	return NewSigBreakPart(obj,len ,len,mode,sw,time_a,time_a,scale,0.0f,-1,VER_01) ;
}
void *NewGolBreakPart2(DG_OBJ *obj,int *sw,float len1 ,float len2 ,int time_a1,int time_a2,
	float scale1,float scale2,int re_time){
	int mode = (DISAPPEAR_MODE|BUILD_ROLL) ;
	return NewSigBreakPart(obj,len1 ,len2,mode,sw,time_a1,time_a2,scale1,scale2,re_time,VER_02) ;
}
