//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xkatana_ap.c
	モデルを分解飛散
	2002/04/05 K.Sigeno
	$Id: xkatana_ap.c,v 1.6 2003/01/04 12:02:07 takaki Exp $
*/

#ifdef KP_XBOX //BP

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

#define NORMAL_MODE		(0x00000000)
#define BUILD_Y			(0x00000002)
#define BUILD_UP		(0x00000004)
#define NO_CHECK_SW		(0x00000008)
#define DISAPPEAR_MODE		(0x00000010)
#define BODY_MODE			(0x00000020)
#define BUILD_UP_Z			(0x00000040)

#define RAND_VERTS			(0x00000080)



#define ABS_DIE	(01)

//#define	MAX_DIF		(5000)
#define	MAX_DIF		(-10000)
#define CYCLE		(30)
#define RATE_VY		(250.0f)
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	FMATRIX		*world ;

	FVECTOR		pos;
	float		min ;
	float		max ;
	float		len ;
	float		rate ;
	int			cnt ;
	int			time_a ;
	int			time_b ;
	int			mode ;
	int			*sw ;
	FVECTOR		bound_min ;
	FVECTOR		bound_max ;
	FVECTOR		*objs_b_min ;
	FVECTOR		*objs_b_max ;

	u_short		*index ;
} Work ;


static void SvectoFvel(SVECTOR *sv , FVECTOR *res,float speed){
	FVECTOR vel ;
	vel.vx = (float) sv->vx ;
	vel.vy = (float) sv->vy ;
	vel.vz = (float) sv->vz ;
	GV_LenVec3F( &vel, res, 0.0f, speed ) ;
}
#define NORM_LEN (4096.0f)
static inline void SetMoveVec(SVECTOR *norms,FVECTOR *buf,float len){
	float rate ;
	rate = len/NORM_LEN ;
#if 1
	buf->vx = (float)norms->vx*rate ;
	buf->vy = (float)norms->vy*rate ;
	buf->vz = (float)norms->vz*rate ;
#else
	buf->vx = norms->vx*len/NORM_LEN ;
	buf->vy = norms->vy*len/NORM_LEN ;
	buf->vz = norms->vz*len/NORM_LEN ;
#endif
}
#define Q_WORD (2)


static void SetBoundScale(Work *work,DG_OBJS *objs ,float size) {
	int j;
	DG_OBJ	*obj;

//	work->objs_b_min = NULL ;
//	work->objs_b_max = NULL ;

	work->objs_b_min = (FVECTOR *)GV_Malloc( sizeof( FVECTOR ) * work->objs->n_models );
	work->objs_b_max = (FVECTOR *)GV_Malloc( sizeof( FVECTOR ) * work->objs->n_models );


	work->bound_min = objs->bound_min ;
	work->bound_max = objs->bound_max ;

	if(work->mode & BUILD_UP_Z){
		work->min = objs->bound_min.vz ;
		work->max = objs->bound_max.vz ;
	}else {
		work->min = objs->bound_min.vy ;
		work->max = objs->bound_max.vy ;
	}
	/*バウンディングボックスも拡大*/
	objs->bound_min.vx -= size ;
	objs->bound_min.vy -= size ;
	objs->bound_min.vz -= size ;
	objs->bound_max.vx += size ;
	objs->bound_max.vy += size ;
	objs->bound_max.vz += size ;

	for(j= 0 ;j<objs->n_models;j++){
		obj = &objs->objs[j] ;

		work->objs_b_min[j] = obj->bound_min;
		work->objs_b_max[j] = obj->bound_max;

		obj->bound_min.vx -= size ;
		obj->bound_min.vy -= size ;
		obj->bound_min.vz -= size ;
		obj->bound_max.vx += size ;
		obj->bound_max.vy += size ;
		obj->bound_max.vz += size ;
	}
}
static void ReSetBoundScale(Work *work,DG_OBJS *objs ) {
	int j;
	DG_OBJ	*obj;

	objs->bound_min = work->bound_min ;
	objs->bound_max = work->bound_max ;

	for(j= 0 ;j<objs->n_models;j++){
		obj = &objs->objs[j] ;
		obj->bound_min = work->objs_b_min[j] ;
		obj->bound_max = work->objs_b_max[j] ;
	}
}

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

static inline int SetDelayCnt(int cnt ,FVECTOR *org_sv,Work *work){
	int tmp_cnt ;
	tmp_cnt = cnt ;
//BUILD_Y			(0x00000002)
//BUILD_UP		(0x00000004)

	if(work->mode & BUILD_UP_Z){
		if(work->mode & BUILD_UP){
			tmp_cnt -= (org_sv->vz - work->min)/work->rate  ;
		}else {
			tmp_cnt -= (work->max - org_sv->vz )/work->rate  ;
		}
	}else {
		if(work->mode & BUILD_UP){
			tmp_cnt -= (org_sv->vy - work->min)/work->rate  ;
		}else {
			tmp_cnt -= (work->max - org_sv->vy )/work->rate  ;
		}
	}
	if(tmp_cnt<0) tmp_cnt = 0 ;
	if(tmp_cnt>work->time_a) tmp_cnt = work->time_a ;
	return tmp_cnt ;
}


static inline void MakeVerts(Work *work , FVECTOR *buf,FVECTOR *norms,int speed ){
	if(work->mode & BUILD_Y){
		buf->vx=buf->vz = 0; buf->vy = -speed ;
	}else if(work->mode & RAND_VERTS){
		FVECTOR  test ;
		test.vx = ((irnd()>>8)& 1023) - 512 ;
		test.vy = ((irnd()>>8)& 1023) - 512;
		test.vz = ((irnd()>>8)& 1023) - 512;
		ScaleNormalize( buf, &test, speed );
	}else {
		ScaleNormalize( buf, norms, speed );
	}
}

static inline void MakePolyScale(Work *work ,float *rate ,float *rate_dif,int tmp_cnt,int obj_index){
	*rate = (float)tmp_cnt/(float)work->time_a ;

	if((*rate > 0.7f)&&(obj_index!=0)) *rate = 1.0f ;
	*rate_dif = 1.0f - *rate ;
}

#define BUILD_MODE (BUILD_NORM)
static void SIG_ActVanimKatana(Work *work,int cnt)
{
	DG_VERTS_ANIME	*vanim ;
	DG_VERTEX_KMSM	*d_verts, *org_verts, *s_verts ;
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	DG_OBJS		*objs ;
	DG_OBJ		*obj;
	float		rate,rate_dif ;
	float		weight,weight_dif;
	int			n_verts, i, j, k, l, kick, tmp_cnt, rank ;
	int			speed ;
	FVECTOR		tmp;
	FMATRIX		world ;
	u_short			*index, top_index ;
	FVECTOR			*verts, *top_verts, norm, move_vec ;

	objs = work->objs ;
	//printf("cnt %d\n",cnt);

	for(j= 0 ;j<objs->n_models;j++){
		vanim = &work->vanim[ j ];
		DG_SwitchVAnimeBuffer( vanim );
		DG_RestartAnimVerts( vanim );

		d_verts = vanim->verts_top[ vanim->buffer_clock ];
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
				verts++ ;
				n_verts++ ;
				if ( i < 2 ) continue ;/* 最初の２点は無視する */
				/* ストリップの切れ目だった場合に移動処理を行なう */
				if ( ( i == ( mdl_pack->n_indices - 1 ) ) || 
					( index[ -1 ] == index[ 0 ] && index[ 1 ] == index[ 2 ] ) ){

					verts = top_verts = (FVECTOR*)SCRPAD_ADDR ;

					/* 移動量及び変形パラメータを決めておく */
					norm.vx = org_verts[ top_index ].nx ;
					norm.vy = org_verts[ top_index ].ny ;
					norm.vz = org_verts[ top_index ].nz ;
					tmp_cnt = SetDelayCnt( cnt, top_verts, work );
					speed = work->len*(float)((work->time_a-tmp_cnt)*
											  (work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;
					MakeVerts( work, &move_vec, &norm, speed );
					MakePolyScale( work, &rate, &rate_dif, tmp_cnt, j );

					/* 切り出した１ストリップ分の頂点に対して移動計算 */
					for ( l = 0 ; l < n_verts ; l++, verts++ ){
						if ( ( tmp_cnt == work->time_a ) || ( j == 0 ) ){
							/* アニメーションしない場合 */
							d_verts->vx = verts->vx;
							d_verts->vy = verts->vy;
							d_verts->vz = verts->vz;
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
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
							d_verts->wt = 32767 ;
#else
							d_verts->wt = 0 ;
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
		DG_AssignDGObjVertexBuffer(&objs->objs[j]);
#endif

	}
}
static void Act(Work *work)
{
	int cnt ;
	cnt = work->cnt ;

	if(GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE){
		DG_InvisibleObjs(work->objs) ;
	}else {
		DG_VisibleObjs(work->objs) ;
	}


	DG_SetPos( work->world ) ;
	DG_PutObjs( work->objs );

	SIG_ActVanimKatana(work,cnt) ;

	DG_GetLightMatrix( (FVECTOR *)&work->world->m[3], work->lights );



#if 0
	if(GV_PadData[ 0 ].status & PAD_U){
		work->cnt++; 
	}
	if(GV_PadData[ 0 ].status & PAD_D){
		work->cnt--; 
	}
	if(work->cnt<0) work->cnt=0 ;
#else

	if(work->mode &DISAPPEAR_MODE){
		work->cnt-- ;
	}else {
		work->cnt++ ;
	}
#endif

#if 1
//NO_CHECK_SW
	if(
	((!(work->mode & DISAPPEAR_MODE))&&(work->cnt >= (work->time_a+work->time_b)))
	||(((work->mode & DISAPPEAR_MODE))&&(work->cnt <= 0))
	||((*work->sw & ABS_DIE))
	){
//		DG_InvisibleObjs(work->objs) ;
		*work->sw |= ABS_DIE ;
		GV_DestroyActor(work) ;
	}
#endif
}
static void Die(Work *work)
{
	SIG_ResetExpansionIndex( work->objs );
	if ( work->index != NULL ) GV_Free( work->index );

	ReSetBoundScale(work,work->objs ) ;
	SIG_FreeVanim(work->vanim,work->objs->n_models) ;
#ifdef KP_WINDOWS
	/* Windows版はデータ展開用専用VertexBufferを解放する必要あり */
	{
		int		i ;
		DG_OBJ	*obj ;

		obj = work->objs->objs ;
		for(i=work->objs->n_models; i>0; i--, obj++)
		{
			DG_ReleaseDGObjPrivateVertexBuffer(obj) ;
		}
	}
#endif
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
	GV_Free(work->vanim) ;
	GV_Free(work->objs_b_min) ;
	GV_Free(work->objs_b_max) ;
}

//#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT)
#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

static int GetResources(Work *work,int model,FMATRIX *world,int mode ,float len,int time_a,int time_b,int *sw)
{
	DG_DEF	*def ;
	int i ;
	FVECTOR		pos;

	work->world = world ;
	work->mode = mode ;

	work->objs_b_min = NULL ;
	work->objs_b_max = NULL ;

	def = (DG_DEF*) GV_GetCache( GV_CacheID( model, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, MDL_FLAG, DG_CHANL_MAIN ) ;

	work->objs->root = work->world;

	work->sw = sw ;
	DG_QueueObjs( work->objs ) ;
	DG_SetPos( world ) ;

	DG_PutObjs( work->objs );
	DG_SetLightMatrix( work->objs, work->lights );
	work->len = len ;
	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->objs->n_models );
	ASSERT(work->vanim != NULL ) ;
	/* ＸＢＯＸ版では展開された状態での頂点アニメワークを作成 */
	SIG_InitVanim(work->vanim,work->objs,work->objs->n_models) ;
	work->index = SIG_SetExpansionIndex( work->objs );
#ifdef KP_WINDOWS
	/* Windows版はデータ展開用専用VertexBufferを作成する必要あり */
	{
		int		i ;
		DG_OBJ	*obj ;

		obj = work->objs->objs ;
		for(i=work->objs->n_models; i>0; i--, obj++)
		{
			if( !obj->n_indices ){ continue ; }
			DG_CreateDGObjPrivateVertexBuffer(obj, obj->n_indices, obj->n_indices) ;
		}
	}
#endif

    /* プリシェード */
	DG_GetLightMatrix( (FVECTOR *)&work->world->m[3], work->lights );
	SetBoundScale(work,work->objs ,fabs(len)) ;
	work->time_a = DIRECT_TICK(time_a) ;
	work->time_b = DIRECT_TICK(time_b) ;

	if(work->mode &DISAPPEAR_MODE){
		work->cnt = (work->time_a+work->time_b) ;
	}else {
		work->cnt = 0 ;
	}
	work->rate = (work->max - work->min) / (float)work->time_b ;

	SIG_ActVanimKatana(work,work->cnt) ;

	return 1;
}

void *NewSigKatanaAppear(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,int time_b,int *sw ){
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,mdlcode,world,mode ,len,time_a,time_b,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void *NewKatanaAppear(FMATRIX *world,int mode ,int *sw ){
	int flag = 0x80;
	if(mode ==0){
		/*消え*/
		flag = 0x90 ;
	}
	return NewSigKatanaAppear(15363829,world,flag,250.0f,60,0 ,sw) ;
}

#endif
