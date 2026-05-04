//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breakobj2.c
	モデルを分解飛散
	2002/04/05 K.Sigeno
	$Id: xbreakobj2.c,v 1.7 2003/01/04 12:02:07 takaki Exp $
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

#define NORMAL_MODE			(0x00000000)
#define AMB_MODE			(0x00000001)
#define BUILD_Y				(0x00000002)
#define BUILD_UP			(0x00000004)
#define NO_CHECK_SW			(0x00000008)
#define DISAPPEAR_MODE		(0x00000010)
#define BODY_MODE			(0x00000020)
#define BUILD_X				(0x00000040)
#define BUILD_ROLL			(0x00000080)
#define BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
#define MOT_TR				(0x00000200)
#define RAND_VERTS			(0x00000400)
#define BUILD_UP_Z			(0x00000800)
#define PURE_NORM			(0x00001000)
#define NO_MAKE_OBJ			(0x00002000)
#define STOP_END			(0x00004000)
#define PRESHADE_MODE		(0x00008000)



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

	FVECTOR 	shift ;
	SVECTOR 	rot ;

	FVECTOR		pos;
	float		min ;
	float		max ;
	float		len ;
	float		rate ;
	int			cnt ;
	int			time_a ;
	int			time_b ;
	int			delay ;
	int			mode ;
	int			*sw ;
	int			dummy_sw ;

	u_short		*index ;			/* 差し替え用インデックス */
	u_int		*rgbs ;
	u_int		*org_rgbs ;

} Work ;

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)
#if 0
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif

static void SetAmb(Work *work,u_char r,u_char g,u_char b){
	work->lights[1].m[ 3 ][ 0 ] = (float) r ;
	work->lights[1].m[ 3 ][ 1 ] = (float) g ;
	work->lights[1].m[ 3 ][ 2 ] = (float) b ;
}
#define NORM_LEN (4096.0f)
#define Q_WORD (2)


static void SetBoundScale(Work *work,DG_OBJS *objs ,float size) {
	int j;
	DG_OBJ	*obj;

	if(work->mode & BUILD_UP_Z){
		work->min = objs->bound_min.vz ;
		work->max = objs->bound_max.vz ;
	}else {
		work->min = objs->bound_min.vy ;
		work->max = objs->bound_max.vy ;
	}
	if(work->mode &	NO_MAKE_OBJ){
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

static inline void MakeVerts( Work *work, FVECTOR *buf, FVECTOR *norms ,int speed ){
	FVECTOR tmp ;
	if(work->mode & BUILD_Y){
		tmp.vx=tmp.vz = 0; 
		if(work->mode & BUILD_REV){
			tmp.vy = speed ;
		}else {
			tmp.vy = -speed ;
		}
		tmp.vw = 1.0f;
		DG_RotVector(&tmp,buf,1) ;

	}else if(work->mode & RAND_VERTS){
		FVECTOR  test ;
		test.vx = ((irnd()>>8)& 1023) - 512 ;
		test.vy = ((irnd()>>8)& 1023) - 512;
		test.vz = ((irnd()>>8)& 1023) - 512;
		ScaleNormalize( buf, &test, speed );
	}else {
		//法線
		ScaleNormalize( buf, norms, speed );
		if( !( work->mode & PURE_NORM ) ){
			if(work->mode & BUILD_REV){
				buf->vy += speed ;
			}else {
				buf->vy -= speed ;
			}
		}

	}
}

static inline void MakePolyScale(Work *work ,float *rate ,float *rate_dif,int tmp_cnt){
	*rate = (float)tmp_cnt/(float)work->time_a ;
	*rate_dif = 1.0f - *rate ;
}

#define BUILD_MODE (BUILD_NORM)
static void SIG_ActVanimObjBuild(Work *work,int cnt) {
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

	if(work->mode & BUILD_Y){
		_sceVu0InversMatrix(&world ,&objs->objs[0].world) ;
		DG_SetPos(&world) ;
	}
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
					MakePolyScale( work, &rate, &rate_dif, tmp_cnt );

					/* 切り出した１ストリップ分の頂点に対して移動計算 */
					for ( l = 0 ; l < n_verts ; l++, verts++ ){
						if ( tmp_cnt == work->time_a ){
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

/*バッファの開放と復元*/
static void RestoreVbuf(Work *work){
	int		i, j ;
	DG_OBJ	*obj ;
	DG_MDL	*mdl ;
	DG_MDLPACK	*pack ;
	u_int		*rgbs ;

	if ( work->rgbs == NULL ) {
		return 0 ;
	}

	rgbs = work->org_rgbs ;
	/* 展開した頂点カラーを元に戻す */
	obj = work->objs->objs ;
	for ( i = 0 ; i < work->objs->n_models ; i++, obj++ ){
		mdl = obj->model ;
		pack = mdl->packs ;
		obj->rgbs = rgbs ;		/* 新しいバッファの設定 */
		obj->norms = rgbs ;		/* 新しいバッファの設定 */
		rgbs += mdl->n_verts ;
#ifdef KP_WINDOWS
		/* Color Vertex Buffer適用 */
		if( obj->rgbs || obj->norms ){ DG_AssignDGObjCVertexBuffer(obj) ; }
#endif
	}
	if ( work->rgbs != NULL ) {
		GV_Free( work->rgbs );
		work->rgbs = NULL ;
	}
}

static void Act(Work *work)
{
	int cnt ;
	FVECTOR scale ;
	cnt = work->cnt ;
	scale.vx = scale.vy = scale.vz = 0.90f;
	SIG_ActVanimObjBuild(work,cnt) ;

	if(!(work->mode &NO_MAKE_OBJ)){
		if(work->mode & AMB_MODE){
			u_char col;
			col = 255*((work->time_a+work->time_b)-work->cnt)/(work->time_a+work->time_b) ;
			SetAmb(work,col,col,col) ;
		}else {
			DG_GetLightMatrix( (FVECTOR *)&work->world->m[3], work->lights );
		}
	}
	if(!(work->mode &NO_MAKE_OBJ)){
		if((work->mode & MOT_TR)){
			DG_SetPos( work->world ) ;
			DG_MovePos(&work->shift) ;
			DG_RotatePos(&work->rot) ;
			if(work->mode & AMB_MODE){
				DG_ScalePos(&scale) ;
			}
			DG_PutObjs( work->objs );
		}
	}

#if 0
	if(GV_PadData[ 0 ].status & PAD_U){
		work->cnt++; 
	}
	if(GV_PadData[ 0 ].status & PAD_D){
		work->cnt--; 
	}
	if(work->cnt<0) work->cnt=0 ;
#else

	if(work->delay >0){
		work->delay--;
	}else {
		if(work->mode &DISAPPEAR_MODE){
			if(work->cnt >=0){
				work->cnt-- ;
			}
		}else {
			if(work->cnt <= (work->time_a+work->time_b)){
				work->cnt++ ;
			}
		}
	}

#endif


#if 1
//NO_CHECK_SW

	if(
	((!(work->mode & DISAPPEAR_MODE))&&(work->cnt > (work->time_a+work->time_b)))
//	||(((work->mode & DISAPPEAR_MODE))&&(work->cnt <= 0))
	||(((work->mode & DISAPPEAR_MODE))&&(work->cnt < 0))
	||((*work->sw <0)&&(!(work->mode & NO_CHECK_SW)))
	){
//		DG_InvisibleObjs(work->objs) ;
		if(!(work->mode & STOP_END)){
			GV_DestroyActor(work) ;
		}
	}
#endif
}
static void Die(Work *work)
{
	if(work->mode & PRESHADE_MODE){
		RestoreVbuf(work) ;
	}
	if ( work->index != NULL ) {
		SIG_ResetExpansionIndex( work->objs );
		GV_Free( work->index );
	}

	if(work->vanim != NULL ) {
		SIG_FreeVanim(work->vanim,work->objs->n_models) ;
	}
	if(!(work->mode &	NO_MAKE_OBJ)){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
	}
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
	GV_Free(work->vanim) ;
}

//#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT)
#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

static int GetResources(Work *work,DG_DEF *model,FMATRIX *world,int mode ,float len,int time_a,int time_b,int *sw)
{
	DG_DEF	*def ;
	int i ;
	FVECTOR		pos;

	work->world = world ;
	work->mode = mode ;
	work->dummy_sw = 0 ;

	def = model ;
	if(!(work->mode &	NO_MAKE_OBJ)){
		work->objs = DG_MakeObjs( def, MDL_FLAG, DG_CHANL_MAIN ) ;
		DG_QueueObjs( work->objs ) ;
		DG_SetPos( world ) ;
		DG_MovePos(&work->shift) ;
		DG_RotatePos(&work->rot) ;
		DG_PutObjs( work->objs );
		DG_SetLightMatrix( work->objs, work->lights );
	    /* プリシェード */
		if(work->mode & AMB_MODE){
			SetAmb(work,255,255,255) ;
		}else {
			DG_GetLightMatrix( (FVECTOR *)&work->world->m[3], work->lights );
		}
	}

	if((work->mode & NO_CHECK_SW)||(sw==NULL)){
		work->sw = &work->dummy_sw ;
	}else {
		work->sw = sw ;
	}
	work->len = len ;
	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->objs->n_models );
	if(work->vanim == NULL ) {
		return 0 ;
	}
	/* ＸＢＯＸ版では展開された状態での頂点アニメワークを作成 */
	if(SIG_InitVanim(work->vanim,work->objs,work->objs->n_models) == 0){
		return 0 ;
	}
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
	work->index = SIG_SetExpansionIndex( work->objs );
	if(work->index == NULL ) {
		return 0;
	}
	if(work->mode & PRESHADE_MODE){
		/* プリシェードも展開したものを用意する */
		int		i, j, k, size ;
		DG_OBJS	*objs ;
		DG_OBJ	*obj ;
		DG_MDL	*mdl ;
		DG_MDLPACK	*pack ;
		u_int		*org_rgbs, *rgbs, *org_rgbs_mem ;
		u_short		*index ;

		/* 必要な展開後頂点数を求め、メモリを確保する */
		objs = work->objs ;
		obj = objs->objs ;
		size = 0 ;
		for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
			mdl = obj->model ;
			pack = mdl->packs ;
			for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
				size += pack->n_indices ;
			}
		}

		/* メモリの確保 */
		work->org_rgbs = objs->rgb_buff ;
		if(work->rgbs == NULL){
			work->rgbs = rgbs = GV_Malloc( sizeof(u_int) * size );
		}
		if ( rgbs == NULL ) {
			return  0 ;
		}
		//objs->rgb_buff = work->rgbs ;

		/* 展開した頂点カラーを設定する */
		obj = objs->objs ;
		for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
			mdl = obj->model ;
			pack = mdl->packs ;
			org_rgbs = obj->rgbs ;
			obj->rgbs = rgbs ;		/* 新しいバッファの設定 */
			obj->norms = rgbs ;		/* 新しいバッファの設定 */
			for ( j = 0 ; j < mdl->n_packs ; j++, pack++ ){
				index = pack->index ;
				for ( k = 0 ; k < pack->n_indices ; k++ ){
					*rgbs = org_rgbs[ *index ];
					rgbs++ ;
					index++ ;
				}
			}
#ifdef KP_WINDOWS
			DG_AssignDGObjCVertexBuffer(obj);
#endif
		}
	}

	SetBoundScale(work,work->objs ,fabs(len)) ;
	work->time_a = DIRECT_TICK(time_a) ;
	work->time_b = DIRECT_TICK(time_b) ;

	if(work->mode &DISAPPEAR_MODE){
		work->cnt = (work->time_a+work->time_b) ;
	}else {
		work->cnt = 0 ;
	}
	work->rate = (work->max - work->min) / (float)work->time_b ;

//	void *NewObjectOutline(
//	int     cache,		/* キャッシュＩＤ */
//	DG_OBJS *dg_objs)	/* ＤＧオブジェクト */
	SIG_ActVanimObjBuild(work,work->cnt) ;

	return 1;
}

void *NewSigBreakObj2(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,int time_b,int *sw ){
	Work *work ;
	DG_DEF *def ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = 0;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		def = (DG_DEF*) GV_GetCache( GV_CacheID( mdlcode, 'k' ) ) ;
		if(!GetResources(work,def,world,mode ,len,time_a,time_b,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewSigBreakObj3(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,
	int delay,int *sw,FVECTOR *shift,SVECTOR *rot
){
	Work *work ;
	DG_DEF *def ;

	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = *shift ;
		work->rot = *rot ;
		work->delay = DIRECT_TICK(delay);
		def = (DG_DEF*) GV_GetCache( GV_CacheID( mdlcode, 'k' ) ) ;

		if(!GetResources(work,def,world,mode ,len,time_a,0,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewSigBreakObj4(DG_OBJS *objs,FMATRIX *world,int mode ,float len,int time_a ,
	int delay,int *sw,FVECTOR *shift,SVECTOR *rot)
{
	Work *work ;
	DG_DEF *def ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = *shift ;
		work->rot = *rot ;
		work->delay = DIRECT_TICK(delay);
		def = objs->def ;
		if(!GetResources(work,def,world,mode ,len,time_a,0,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*NO_MAKE_OBJ*/
void *NewSigBreakObj5(DG_OBJS *objs,int mode ,float len,int time_a ,int delay,int *sw)
{
	Work *work ;
	DG_DEF *def ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = DIRECT_TICK(delay);
		work->objs = objs;
		def = objs->def ;
		if(!GetResources(work,def,NULL,mode|NO_MAKE_OBJ|NO_CHECK_SW ,len,time_a,0,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewSigBreakObj6(DG_OBJS *objs,float len,int time_a ,int time_b )
{
	Work *work ;
	DG_DEF *def ;
	int mode = (BUILD_UP|BUILD_Y|DISAPPEAR_MODE|STOP_END);
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = 0;
		work->objs = objs;
		def = objs->def ;
		if(!GetResources(work,def,NULL,mode|NO_MAKE_OBJ|NO_CHECK_SW ,len,time_a,time_b,NULL)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewSigBreakObj7(DG_OBJS *objs,float len,int time_a ,int time_b)
{
	Work *work ;
	DG_DEF *def ;
	int mode = (BUILD_UP_Z|DISAPPEAR_MODE|STOP_END|BUILD_REV);
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = 0;
		work->objs = objs;
		def = objs->def ;
		if(!GetResources(work,def,NULL,mode|NO_MAKE_OBJ|NO_CHECK_SW|PRESHADE_MODE ,len,time_a,time_b,NULL)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}



#endif
