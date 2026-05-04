//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breabody.c
	多間接モデル頂点を飛散
	2002/03/14 K.Sigeno
	$Id: xbreakbody.c,v 1.11 2003/01/05 04:46:47 takaki Exp $
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

//#include "../../yamashita/outline/outline.h"

//#define VR_EF 0

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

#define BREAK_PARTS_MAX 24
#define NORAMLPARTS_MAX 21

static int BODY_PARTS[BREAK_PARTS_MAX] ={

HUMAN21_MIGI_TSUMASAKI  ,
HUMAN21_HIDARI_TSUMASAKI,
HUMAN21_MIGI_KAKATO     ,
HUMAN21_HIDARI_KAKATO   ,

HUMAN21_MIGI_ASHI2      ,
HUMAN21_HIDARI_ASHI2    ,

HUMAN21_HIDARI_ASHI1    ,
HUMAN21_MIGI_ASHI1      ,

HUMAN21_KOSHI           ,

HUMAN21_ONAKA           ,

HUMAN21_HIDARI_TE       ,
HUMAN21_MIGI_TE         ,

HUMAN21_MUNE            ,


HUMAN21_HIDARI_UDE2     ,
HUMAN21_MIGI_UDE2       ,


HUMAN21_MIGI_UDE1       ,
HUMAN21_HIDARI_UDE1     ,

HUMAN21_MIGI_KATA       ,
HUMAN21_HIDARI_KATA     ,


HUMAN21_KUBI            ,

HUMAN21_ATAMA           ,

-1,-1,-1
};


#define NORMAL_MODE			(0x00000000)
//#define AMB_MODE			(0x00000001)
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


/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	DG_OBJS		*p_objs ;
	FMATRIX		lights[2] ;
//	float		speed ;
	int			cnt ;
	int			n_models ;

	float		len ;
	int			time_a ;
	int			time_b ;
	int			delay ;
	int			mode ;

	float		min ;
	float		max ;
	float		rate ;

	FVECTOR color;
	void		*oozeblood ;		/* にじみ血用 */

	u_short		*index ;			/* 差し替え用インデックス */

#ifdef KP_WINDOWS
	void		*org_rgbs ;			/* にじみ血値保存 */
#endif
} Work ;

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)
#if 0
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif

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

static inline int PartsRank(int j,Work *work){
	int l,check ;

#if 0
	if(work->mode &NO_DELAY){
		return 0 ;
	}
#endif

	if(j >= NORAMLPARTS_MAX){
		check = work->objs->objs[j].parent ;
	}else {
		check = j ;
	}

//DISAPPEAR_MODE

	for(l=0;l<BREAK_PARTS_MAX;l++){
		if(check==BODY_PARTS[l]){
			if(work->mode & DISAPPEAR_MODE){
				if(work->mode & BUILD_UP){
//					return ((work->objs->n_models-1) - l ) ;
					return ((NORAMLPARTS_MAX-1) - l ) ;
				}else {
					return l ;
				}
			}else {
				if(work->mode & BUILD_UP){
					return l ;
				}else {
//					return ((work->objs->n_models-1) - l ) ;
					return ((NORAMLPARTS_MAX-1) - l ) ;
				}
			}
		}
	}
	if(check==BODY_PARTS[l]){
		return BREAK_PARTS_MAX-1 ;
	}else {
		return 0 ;
	}
}

#define Q_WORD (2)

static void SetBoundScale(Work *work,DG_OBJS *objs ,float size) {
	int j;
	DG_OBJ	*obj;

	work->min = objs->bound_min.vy ;
	work->max = objs->bound_max.vy ;

	if(work->mode &NO_MOVE_VEC){
		return ;
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
		obj->bound_min.vx -= size ;
		obj->bound_min.vy -= size ;
		obj->bound_min.vz -= size ;
		obj->bound_max.vx += size ;
		obj->bound_max.vy += size ;
		obj->bound_max.vz += size ;
	}
}

static inline int SetDelayCnt2(int cnt ,Work *work,int parts){
	int tmp_cnt ;
	tmp_cnt = cnt ;

	if(work->mode &NO_DELAY){
		return tmp_cnt ;
	}

//	tmp_cnt -= work->time_b*PartsRank(parts,work)/work->objs->n_models ;
	tmp_cnt -= work->time_b*PartsRank(parts,work)/(NORAMLPARTS_MAX-1) ;

//	if(tmp_cnt<0) tmp_cnt = 0 ;
//	if(tmp_cnt>work->time_a) tmp_cnt = work->time_a ;

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
		if(work->mode & BUILD_REV){
			buf->vy += speed ;
		}else {
			buf->vy -= speed ;
		}
		DG_RotVector( buf, buf, 1 ) ;
	}
}

static void SIG_ActVanimObjBuild(Work *work,int cnt,int clock) {
	DG_VERTS_ANIME	*vanim ;
	DG_VERTEX_KMSM	*d_verts, *org_verts, *s_verts ;
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	FVECTOR		ef_pos ;
	DG_OBJS		*objs ;
	DG_OBJ		*obj;
	float		rate,rate_dif ;
	float		weight,weight_dif;
	int			n_verts, i, j, k, l, tmp_cnt ;
	int			speed ;
	FVECTOR		tmp;
	FMATRIX		world ;
	SVECTOR		rot ;
	u_short			*index, top_index ;
	FVECTOR			*verts, *top_verts, norm, move_vec ;

	objs = work->objs ;
	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 0;

	ef_pos.vx = objs->world.m[3][0] ;
	ef_pos.vy = objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1] ;
	ef_pos.vz = objs->world.m[3][2] ;

	for(j= 0 ;j<objs->n_models;j++){
		vanim = &work->vanim[ j ];
//		DG_SwitchVAnimeBuffer( vanim );
//		DG_RestartAnimVerts( vanim );

		if(cnt < 0){
			if(work->mode &DISAPPEAR_MODE){
				tmp_cnt = work->time_a ;
			}else {
				tmp_cnt = 0 ;
			}
		}else {
			tmp_cnt = SetDelayCnt2(cnt ,work,j) ;
		}
		if((tmp_cnt>(work->time_a+1))||(tmp_cnt<0)){
			continue ;
		}
		/* 移動量及び変形パラメータを決めておく */
		speed = work->len * (float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;
		rate = (float)tmp_cnt/(float)work->time_a ;
		rate_dif = 1.0f - rate ;
		weight = SetWeightRate(rate) ;
		weight_dif = 1.0f - weight ;

		/* fall_line */
		//ef_pos.vy = objs->objs[j].world.m[3][1] ;

		_sceVu0InversMatrix(&world ,&objs->objs[j].world) ;
		DG_SetPos(&world) ;

		if(work->mode & BUILD_ROLL){
			rot.vy = (tmp_cnt*256)&4095;
			DG_RotatePos(&rot) ;
		}
#if 0
		d_verts = vanim->verts_top[ vanim->buffer_clock ];
#else
		d_verts = vanim->verts_top[ clock ];
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
							if(!(work->mode & NO_WEIGHT)){
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
								d_verts->wt = (short)(32767.0f * rate_dif) + (short) (verts->vw * rate) ;
#else
								d_verts->wt = (short) ((float)verts->vw * rate) ;
#endif
//	sv->pad = (((float)org_sv->pad/4096.0f) * weight )*4096.0f + (4096.0f*weight_dif);

							}
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

	float width ;
//	FVECTOR color;
	int i,j,n_body;
	n_body = work->p_objs->n_models ;


#if 1
	if(work->mode & MOTION_TR){
		for(i= 0 ;i<n_body;i++){
			work->objs->objs[i].world = work->p_objs->objs[i].world ;
		}
	}
#endif

	if(!(work->mode & NO_MAKE_OBJ)){
		DG_GetLightMatrix( (FVECTOR *) &work->objs->world.m[3] , work->lights );
	}

	for(j= 0 ;j<work->n_models;j++){
		DG_VERTS_ANIME	*vanim ;
		vanim = &work->vanim[ j ];
		DG_SwitchVAnimeBuffer( vanim );
		DG_RestartAnimVerts( vanim );
	}

	SIG_ActVanimObjBuild(work,work->cnt,work->vanim[ 0 ].buffer_clock) ;

	if(work->mode &DISAPPEAR_MODE){
		if(work->delay>0){
			work->delay-- ;
		}else {
			work->cnt-- ;
		}
		if(work->cnt < 0) {
			GV_DestroyActor(work) ;
		}
	}else {
		if(work->delay>0){
			work->delay-- ;
		}else {
			work->cnt++ ;
		}
		if(work->cnt > work->time_a+work->time_b) {
			GV_DestroyActor(work) ;
		}
	}

}
static void Die(Work *work)
{
	if ( work->index != NULL ) {
		SIG_ResetExpansionIndex( work->objs );
		GV_Free( work->index );
	}
	if(work->vanim != NULL) {
		SIG_FreeVanim(work->vanim,work->n_models) ;
		GV_Free(work->vanim) ;
	}
#ifdef KP_WINDOWS
	/* Windows版はデータ展開用専用VertexBufferを解放する必要あり */
	{
		int		i ;
		DG_OBJ	*obj ;
		void	*rgbs ;

		rgbs = work->org_rgbs ;
		work->objs->rgb_buff = rgbs ;			/* にじみ血値復帰 */

		obj = work->objs->objs ;
		for(i=work->objs->n_models; i>0; i--, obj++)
		{
			ASSERT( !obj->rgbs ) ;
			obj->rgbs = NULL ;	// にじみ血再設定

			DG_ReleaseDGObjPrivateVertexBuffer(obj) ;

#if __DG_OBJ_VERTEX_WEIGHT_REV__
			obj->mdl_type |= obj->model->type & (DG_TYPE_REVWT_WT0 | DG_TYPE_REVWT_WT1) ;
								// フラグ復帰
#endif
		}
	}
#endif
	if(!(work->mode & NO_MAKE_OBJ)){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
	}
}

static int GetResources(Work *work, DG_OBJS *p_objs,float len,CVECTOR *col,int time_a,int time_b)
{
	DG_DEF	*def ;
	int i,j;

	work->len = len ;

	work->time_a = DIRECT_TICK(time_a) ;
	work->time_b = DIRECT_TICK(time_b) ;

	def = (DG_DEF*) p_objs->def ;
	if(work->mode & NO_MAKE_OBJ){
#ifdef DEBUG_MODE
printf("work->mode & NO_MAKE_OBJ \n");
#endif
		work->objs = p_objs ;
		work->p_objs = p_objs ;
	}else {
#ifdef DEBUG_MODE
printf("DG_MakeObjs \n");
#endif
		work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC, DG_CHANL_MAIN ) ;
//		ASSERT(work->objs != NULL ) ;
		if(work->objs == NULL) return 0 ;
		DG_QueueObjs( work->objs ) ;
		work->p_objs = p_objs ;
		work->objs->group_id = work->p_objs->group_id ;
		DG_SetPos( &p_objs->world ) ;
		DG_PutObjs( work->objs );
		for(i= 0 ;i<p_objs->n_models;i++){
			work->objs->objs[i].world = p_objs->objs[i].world ;
		}
		DG_SetLightMatrix( work->objs, work->lights );
		DG_GetLightMatrix( (FVECTOR *) &work->objs->world.m[3] , work->lights );

	}

	work->n_models = work->objs->n_models ;

	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->n_models );
//	ASSERT(work->vanim != NULL ) ;
	if(work->vanim == NULL) return 0 ;
	/* ＸＢＯＸ版では展開された状態での頂点アニメワークを作成 */
	if(SIG_InitVanim(work->vanim,work->objs,work->n_models) == 0 ){
		return 0 ;
	}
#ifdef KP_WINDOWS
	/* Windows版はデータ展開用専用VertexBufferを作成する必要あり */
	{
		int		i ;
		DG_OBJ	*obj ;

		work->org_rgbs = work->objs->rgb_buff ;			/* にじみ血値保存 */

		obj = work->objs->objs ;
		for(i=work->objs->n_models; i>0; i--, obj++)
		{
			obj->rgbs = NULL ;	// にじみ血解除

			if( !obj->n_indices ){ continue ; }
			DG_CreateDGObjPrivateVertexBuffer(obj, obj->n_indices, obj->n_indices) ;

#if __DG_OBJ_VERTEX_WEIGHT_REV__
			obj->mdl_type &= ~(DG_TYPE_REVWT_WT0 | DG_TYPE_REVWT_WT1) ;
									// このフラグは信用できなくなるのでＯＦＦ
#endif
		}
	}
#endif
	work->index = SIG_SetExpansionIndex( work->objs );
	if ( work->index == NULL ) {
		return 0 ;
	}
	SetBoundScale(work,work->objs ,fabs(work->len)) ;

	if(work->mode &DISAPPEAR_MODE){
		work->cnt = (work->time_a+work->time_b) ;
	}else {
		work->cnt = 0 ;
	}

	work->rate = (work->max - work->min) / (float)work->time_b ;

	for(j= 0 ;j<work->n_models;j++){
		DG_VERTS_ANIME	*vanim ;
		vanim = &work->vanim[ j ];
		DG_SwitchVAnimeBuffer( vanim );
		DG_RestartAnimVerts( vanim );
	}
	SIG_ActVanimObjBuild(work,-1,0) ;
	for(j= 0 ;j<work->n_models;j++){
		DG_VERTS_ANIME	*vanim ;
		vanim = &work->vanim[ j ];
		DG_SwitchVAnimeBuffer( vanim );
		DG_RestartAnimVerts( vanim );
	}
	SIG_ActVanimObjBuild(work,-1,1) ;

printf("XBREAK BODY RES SUCCESS\n");
	return 1;
}

//void *NewSigBreakBody(OBJECT *body,float len ,CVECTOR *col,int time_a,int time_b,int mode,void *ooze)
void *NewSigBreakBody(DG_OBJS *p_objs,float len ,CVECTOR *col,int time_a,int time_b,int delay,int mode,void *ooze)
{
	extern void TAKABE_OozeBloodChangeObjs( void *, DG_OBJS * ) ;
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->mode = mode;
		work->delay = delay ;
		if(!GetResources(work,p_objs,len,col,time_a,time_b)){
			printf("NewSigBreakBody init failed !!!\n");
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
#if 0
	if(ooze != NULL){
		work->oozeblood = ooze ;
		TAKABE_OozeBloodChangeObjs( ooze, work->objs ) ;
	}else{
		work->oozeblood = NULL ;
	}

	return (void *)work ;
#else
	return NULL ;
#endif
}


#define TIME_A	(30)
#define TIME_B	(90)
#define ENE_HOLD_LEN	(200.0f)
#define ENE_ELIM_LEN	(600.0f)
#define ENE_HOLD_LEN	(200.0f)


void *NewENE_BreakBody(OBJECT *body,int type,void *ooze){

	int mode=0 ;
	float len = ENE_HOLD_LEN ;
	int time_a,time_b ;

#ifdef DEBUG_MODE
printf("NewENE_BreakBody(%d)\n", type);
#endif

	time_a = TIME_A ;
	time_b = TIME_B ;

	switch (type){
//消え
		case 1 :
//下降
			mode = BUILD_UP|BUILD_Y|BUILD_REV|DISAPPEAR_MODE ;
			break;
		case 2 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//上昇下降 	死体消え
			mode = BUILD_UP|BUILD_Y|DISAPPEAR_MODE;
			len = ENE_ELIM_LEN ;
			GM_SeSetMode( SD_A_V_SHITAI, (FVECTOR *)& body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
			break;
		case 3 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ホールドアップ消え
			GM_SeSetMode( SD_A_V_HOLDUP, (FVECTOR *)& body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
			mode = BUILD_UP|BUILD_REV|DISAPPEAR_MODE;
			len = ENE_HOLD_LEN ;
			break;
		case 4 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//上昇下降 	クリア時用 高速死体消え 
			mode = BUILD_UP|BUILD_Y|DISAPPEAR_MODE;
			len = ENE_ELIM_LEN ;
//			GM_SeSetMode( SD_A_V_SHITAI, (FVECTOR *)& body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
			time_a = 15 ;
			time_b = 15 ;
			break;
//以下出現
		case 5 :
//下降
			mode = BUILD_UP|BUILD_Y;
			break;
		case 6 :
//上昇
			mode = BUILD_Y|BUILD_REV;
		break;
		case 7 :
//下降螺旋
			mode = BUILD_UP ;
			break;
		case 8 :
//上昇螺旋
			mode = BUILD_ROLL|BUILD_ROLL|BUILD_REV ;
			break;
		case 9 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ＶＲ敵兵出現
			mode = NO_MOVE_VEC|NO_MAKE_OBJ|NO_WEIGHT;
			len = 0 ;
			time_a = 8 ;
			time_b = 7 ;
			break;
		case 10 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ＶＲ敵兵消失
			mode = NO_MOVE_VEC|NO_MAKE_OBJ|NO_WEIGHT|DISAPPEAR_MODE ;
			len = 0 ;
			time_a = 8 ;
			time_b = 7 ;
			break;
		case 11 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ＶＲ敵兵消失モデル生成型
			mode = NO_MOVE_VEC|NO_WEIGHT|DISAPPEAR_MODE ;
			len = 0 ;
			time_a = 8 ;
			time_b = 7 ;
			break;
	}
	return NewSigBreakBody(body->objs,len, NULL,time_a,time_b,0,mode,ooze) ;
}


#endif
