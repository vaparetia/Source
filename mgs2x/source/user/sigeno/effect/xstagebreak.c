//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	stagebreak.c
	ステージモデルを分解飛散
	2002/04/03 K.Sigeno
	$Id: xstagebreak.c,v 1.9 2002/12/03 08:18:44 takaki Exp $
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

extern DG_OBJS *GM_GetMapObjs(int) ;

//#define	MAX_DIF		(5000)
//#define	MAX_DIF		(-10000)
//#define	MAX_DIF		(10000)

#define MAP_NUM		(0) /*変形させるマップ番号*/
enum	{
	MODE_BUILD = 0,
	MODE_ERASE,
};	
#define ROT_Y (0)
#define ROT_NORM (1)
#define ROT_FREE (2)
#define ROT_MODE ROT_Y
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	FVECTOR		pos;
	FVECTOR		bound_min;
	FVECTOR		bound_max;
	FVECTOR		*objs_bound_min;
	FVECTOR		*objs_bound_max;
	SVECTOR		rot ;
	SVECTOR		dir ;
	int			time_a ;
	int			time_b ;
	float		z_min ;
	float		z_max ;
	float		rate ;
	float		len ;
	int			cnt ;
	int			mode ;
	int			map_num ;
	int			status ;
	int			name ;

	u_short		*index ;
	u_int		*rgbs ;
	u_int		*org_rgbs ;
	int			buf_info ;
	int			vanim_info ;
	int			se_cnt ;
	int			se_cyc ;
	int			se_sw ;
	int			flag_buf ;
} Work ;

enum {
	STAGE_WAIT = (0x0001),
};

#define NORM_LEN (4096.0f)
static inline void SetMoveVec(SVECTOR *norms,FVECTOR *buf,float len){
//		GV_LenVec3( norms, &buf, 0.0f, (int)speed ) ;
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


static void SetBoundScale(Work *work ,float size ) {
	int j;
	DG_OBJ	*obj;

	work->bound_min = work->objs->bound_min;
	work->bound_max = work->objs->bound_max;

	/*バウンディングボックスも拡大*/
	work->objs->bound_min.vx -= size ;
	work->objs->bound_min.vy -= size ;
	work->objs->bound_min.vz -= size ;
	work->objs->bound_max.vx += size ;
	work->objs->bound_max.vy += size ;
	work->objs->bound_max.vz += size ;

	for(j= 0 ;j<work->objs->n_models;j++){
		obj = &work->objs->objs[j] ;
		work->objs_bound_min[j] = obj->bound_min ;
		work->objs_bound_max[j] = obj->bound_max ;

		obj->bound_min.vx -= size ;
		obj->bound_min.vy -= size ;
		obj->bound_min.vz -= size ;
		obj->bound_max.vx += size ;
		obj->bound_max.vy += size ;
		obj->bound_max.vz += size ;
	}
}
static void RestoreBound(Work *work) {
	int j;
	DG_OBJ	*obj;

	work->objs->bound_min = work->bound_min ;
	work->objs->bound_max = work->bound_max ;

	for(j= 0 ;j<work->objs->n_models;j++){
		obj = &work->objs->objs[j] ;
		obj->bound_min = work->objs_bound_min[j] ;
		obj->bound_max = work->objs_bound_max[j] ;
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


static inline int SetDelayCnt( Work *work, int cnt, FVECTOR *org_vec, float z_min )
{
	int tmp_cnt ;
	tmp_cnt = cnt ;
	tmp_cnt -= (org_vec->vz - z_min)/work->rate  ;
	if ( tmp_cnt < 0 ) tmp_cnt = 0 ;
	if ( tmp_cnt>work->time_a ) tmp_cnt = work->time_a ;
	return tmp_cnt ;
}

static void SIG_ActVanimStageBuild( Work *work, DG_OBJS *objs, int cnt, float z_min, FVECTOR *pos )
{
	DG_VERTS_ANIME	*vanim ;
	DG_VERTEX_KMSM	*d_verts, *org_verts, *s_verts ;
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	DG_OBJ		*obj;
	float		rate,rate_dif ;
	float		weight,weight_dif;
	int			n_verts, i, j, k, l, kick, tmp_cnt, rank ;
	int			speed ;
	SVECTOR		buf ;
	FVECTOR		tmp;
	FMATRIX		world ;
	u_short			*index, top_index ;
	FVECTOR			*verts, *top_verts, norm, move_vec ;
	float		inv_time_a ;
//DWORD	d_verts_cntr ;
	//printf("cnt %d\n",cnt);

	inv_time_a = 1.0f / work->time_a ;

	for(j= 0 ;j<objs->n_models;j++){
		vanim = &work->vanim[ j ];
		DG_SwitchVAnimeBuffer( vanim );
		DG_RestartAnimVerts( vanim );

		/* 移動量及び変形パラメータを決めておく */
		//tmp_cnt = SetDelayCnt( work , cnt, org_sv, z_min );
		//speed = work->len * (float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;
		//rate_dif = (float)(work->time_a-tmp_cnt)/(float)work->time_a ;
		//rate_dif *= rate_dif ;
		//rate = 1.0f - rate_dif ;

		d_verts = vanim->verts_top[ vanim->buffer_clock ];
//d_verts_cntr = 0 ;
		mdl = vanim->obj->model ;
		mdl_pack = mdl->packs ;
		org_verts = mdl->vbuff ;
		for ( k = 0 ; k < mdl->n_packs ; k++, mdl_pack++ ){

			index = mdl_pack->index ;

			/* 展開頂点データ生成ワーク初期化 */
			verts = (FVECTOR*)SCRPAD_ADDR ;
			top_index = index[ 0 ] ;
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

					/* 移動量ベクトルの計算 */
					norm.vx = org_verts[ top_index ].nx ;
					norm.vy = org_verts[ top_index ].ny ;
					norm.vz = org_verts[ top_index ].nz ;
#if (ROT_MODE==ROT_NORM)
/*法線*/
					ScaleNormalize( &move_vec, &norm, speed );
#elif (ROT_MODE==ROT_Y)
/*Y方向*/
//					move_vec.vx = move_vec.vz = 0.0f ;
//					move_vec.vy = -speed ;
#elif (ROT_MODE==ROT_FREE)
/*任意方向*/
					ScaleNormalize( &move_vec, &work->dir, speed );
#endif

					/* 切り出した１ストリップ分の頂点に対して移動計算 */
					verts = top_verts = (FVECTOR*)SCRPAD_ADDR ;
					tmp_cnt = SetDelayCnt( work , cnt, verts, z_min );
					rate_dif = (float)(work->time_a-tmp_cnt)*inv_time_a ;
					rate_dif *= rate_dif ;
					rate = 1.0f - rate_dif ;
					speed = work->len * rate_dif ;
					for ( l = 0 ; l < n_verts ; l++, verts++ ){
						if ( tmp_cnt == work->time_a ){
							/* アニメーションしない場合 */
							d_verts->vx = verts->vx;
							d_verts->vy = verts->vy;
							d_verts->vz = verts->vz;
						} else {
							if ( l == 0 ){
								/* 先頭の頂点 */
#if (ROT_MODE==ROT_Y)
/*Y方向のみ*/
//								verts->vx = verts->vx ;
//								verts->vy = verts->vy - move_vec.vy ;
//								verts->vz = verts->vz ;

								verts->vy = verts->vy + speed ;
#else 
/*任意方向*/
								verts->vx = verts->vx - move_vec.vx ;
								verts->vy = verts->vy - move_vec.vy ;
								verts->vz = verts->vz - move_vec.vz ;
#endif

							}else {
								/* 先頭の頂点以外は先頭頂点への縮退処理を伴う */
#if (ROT_MODE==ROT_Y)
/*Y方向のみ*/
//								verts->vx = verts->vx ;
//								verts->vy = verts->vy - move_vec.vy ;
//								verts->vz = verts->vz ;

								verts->vy = verts->vy + speed ;
#else 
/*任意方向*/
								verts->vx = verts->vx - move_vec.vx ;
								verts->vy = verts->vy - move_vec.vy ;
								verts->vz = verts->vz - move_vec.vz ;
#endif
#if (ROT_MODE==ROT_Y)
								verts->vx = verts->vx * rate + top_verts->vx * rate_dif ;
								verts->vy = verts->vy * rate + top_verts->vy * rate_dif ;
								verts->vz = verts->vz * rate + top_verts->vz * rate_dif ;
#else
								verts->vx = verts->vx * rate + top_verts->vx * rate_dif ;
								verts->vy = verts->vy * rate + top_verts->vy * rate_dif ;
								verts->vz = verts->vz * rate + top_verts->vz * rate_dif ;
#endif
							}
							d_verts->vx = verts->vx ;
							d_verts->vy = verts->vy ;
							d_verts->vz = verts->vz ;
						}
						d_verts = (void*)( (char*)d_verts + mdl->stride ) ;
//d_verts_cntr++ ;
//printf("d_verts_cntr(%d/%d)\n", d_verts_cntr, vanim->obj->n_verts) ;

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

enum {
	MSG_START = 1,
	MSG_ERASE,
} ;
static void CheckMesg(Work *work){
	GV_MSG *msg;
	int n;
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for( ; n > 0; n-- ){
			switch( msg->message[ 0 ] ) {
				case MSG_START :
printf("MSG_START\n ");
					work->status &= (~STAGE_WAIT) ;
					work->mode = MODE_BUILD ;
					SetVbuf(work) ;
//					work->cnt = 0 ;
					if(work->cnt > (work->time_a+work->time_b+1)) {
						work->cnt = work->time_a+work->time_b+1 ;
					}
					break ;
				case MSG_ERASE :
printf("MSG_ERASE\n ");
					work->status &= (~STAGE_WAIT) ;
					work->mode = MODE_ERASE ;
					SetVbuf(work) ;
//					work->cnt = (work->time_a+work->time_b) ;
					if(work->cnt < 0) {
						work->cnt = 0 ;
					}
					break ;
			}
			msg++;
		}
	}
}
/*バッファの開放と復元*/
static void RestoreVbuf(Work *work){
	int		i, j ;
	DG_OBJ	*obj ;
	DG_MDL	*mdl ;
	DG_MDLPACK	*pack ;
	u_int		*rgbs ;
#if 1
	work->objs->flag = work->flag_buf;
#endif
	if(work->buf_info == 0){
		return ;
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
	/* インデックスの開放 */
	if ( work->index != NULL ) {
		SIG_ResetExpansionIndex( work->objs );
		GV_Free( work->index );
		work->index = NULL ;
	}
	if(work->vanim_info == 1){
		SIG_FreeVanim(work->vanim,work->objs->n_models) ;
		work->vanim_info =  0 ;
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
	work->buf_info = 0 ;
}
static int SetVbuf(Work *work){
	int i;

	work->objs->flag &= ~DG_FLAG_AUTOREPAINT;
	for(i=0;i<work->objs->n_models;i++){
//		work->objs->objs[i].flag &= ~DG_FLAG_AUTOREPAINT;
	}
	if(work->buf_info == 1){
		/*確保済みなので正常終了*/
		return 1 ;
	}

	if(work->vanim_info == 0){
		SIG_InitVanim(work->vanim,work->objs,work->objs->n_models) ;
		work->vanim_info =  1 ;
	}
	if(work->index == NULL) {
		work->index = SIG_SetExpansionIndex( work->objs );
	}
	if(work->index == NULL) {
		return 0 ;
	}
	{/* プリシェードも展開したものを用意する */
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
			/* Color Vertex Buffer適用 */
			if( obj->rgbs || obj->norms ){ DG_AssignDGObjCVertexBuffer(obj) ; }
#endif
		}
		
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
	work->buf_info = 1 ;
	return 1 ;
}
static void Act(Work *work)
{
	int cnt ,se_tick;
	FVECTOR pos ,sepos;
	int i;

//	ASSERT(!(work->objs->flag & DG_FLAG_AUTOREPAINT));
	for(i=0;i<work->objs->n_models;i++){
//		ASSERT(!(work->objs->objs[i].flag & DG_FLAG_AUTOREPAINT));
	}

	CheckMesg(work) ;
	if(work->status & STAGE_WAIT){
		return ;
	}
//printf("STAGE BREAK CNT [%d]\n",work->cnt);
	cnt = work->cnt ;
	pos.vx = pos.vy = pos.vz = 0;
	if(work->mode == MODE_BUILD){
		if(work->cnt > (work->time_a+work->time_b+1)) {
			work->status |= STAGE_WAIT ;
			RestoreVbuf(work) ;
			return ;
		}
	}else {
		if(work->cnt < 0) {
			work->status |= STAGE_WAIT ;
			return ;
		}
	}
	if(work->buf_info == 1){
		SIG_ActVanimStageBuild(work,work->objs ,cnt,work->z_min,&GM_PlayerPosition) ;
	}

#if 0
	if(work->cnt==0){
		GM_SeSetMode(SD_A_V_START1,&work->pos,GM_SEMODE_BOMB) ;
	}else if((work->cnt>=36)&&(work->cnt<=work->time_a+work->time_b-36)&&(work->cnt%13 ==0)){
		sepos = work->pos ;
		work->pos.vz = work->z_min+(work->z_max-work->z_min) * work->cnt/(work->time_a+work->time_b) ;
		GM_SeSetMode(SD_A_V_START2,&work->pos,GM_SEMODE_BOMB) ;
	}
#else
	se_tick = DG_TickCount - work->se_cnt ;
	if(work->map_num == 0){
		if(work->mode == MODE_BUILD){
			if(work->cnt==0){
				GM_SdSet(SD_A_V_START1);
				work->se_cnt = DG_TickCount ;
				work->se_cyc = DG_TickCount;
//			}else if((DIRECT_TICK(36) <=se_tick)&&(work->cnt<(work->time_a+work->time_b - DIRECT_TICK(12)))){
			}else if((DIRECT_TICK(36) <=se_tick)&&(work->cnt<(work->time_a+work->time_b - DIRECT_TICK(18)))){
				if( (DG_TickCount - work->se_cyc) >=DIRECT_TICK(12) ){
//				if( (DG_TickCount - work->se_cyc) >=DIRECT_TICK(16) ){
//printf("SE CYCLE [%d]\n",(DG_TickCount - work->se_cyc));
					sepos = work->pos ;
					if(work->se_sw == 0 ){
						GM_SdSet(SD_A_V_START2);
					}else {
						GM_SdSet(SD_A_V_START4);
//						GM_SdSet(SD_A_V_START2);
					}
					work->se_sw = !(work->se_sw) ; 
					work->se_cyc = DG_TickCount;
				}else {
				}
			}else if(work->cnt == (work->time_a+work->time_b)) {
					GM_SdSet(SD_A_V_START3 );
			}
		}else {
			if(work->cnt == (work->time_a+work->time_b-1)){
				GM_SdSet(SD_A_V_END001);
			}
		}
	}

#endif
//SD_A_V_START1,	//開始時地形創造１（36fps再生した後）//v_start1 1464
//SD_A_V_START2,	//開始時地形創造２（これを12fpsで連）//v_start2 1465

#if 0
	if(GV_PadData[ 0 ].status & PAD_A){
		work->cnt++; 
	}
	if(GV_PadData[ 0 ].status & PAD_B){
		work->cnt--; 
	}
	if(GV_PadData[ 0 ].status & PAD_U){
		work->rot.vx -= 128;
	}else if(GV_PadData[ 0 ].status & PAD_D){
		work->rot.vx += 128;
	}
	if(GV_PadData[ 0 ].status & PAD_R){
		work->rot.vy += 128;
	}else if(GV_PadData[ 0 ].status & PAD_L){
		work->rot.vy -= 128;
	}
	DG_SetPos2( &DG_ZeroVector, &work->rot ) ;
	tmp.vx = 0;
	tmp.vy = 4096.0f;
	tmp.vz = 0;
	DG_PutVector( &tmp, &tmp, 1 ) ;
	work->dir.vx = tmp.vx ;
	work->dir.vy = tmp.vy ;
	work->dir.vz = tmp.vz ;

	work->cnt %= (work->time_a+work->time_b+1) ;
#else

	if(work->mode == MODE_BUILD){
		work->cnt++ ;
	}else {
		work->cnt-- ;
	}
#endif

#if 0
	if(work->cnt>240){
		if(work->cnt&1){
//			DG_VisibleObjs(work->objs) ;
		}else {
//			DG_InvisibleObjs(work->objs) ;
		}
	}
#endif
}
static void Die(Work *work)
{
#ifdef DEBUG_MODE
	printf("STAGEBREAK END\n");
#endif
	RestoreVbuf(work) ;
	if(work->vanim_info == 1){
		SIG_FreeVanim(work->vanim,work->objs->n_models) ;
		work->vanim_info =  0 ;
	}
	if ( work->rgbs != NULL ) {
		GV_Free( work->rgbs );
		work->rgbs = NULL ;
	}
	/* インデックスの開放 */
	if ( work->index != NULL ) {
		SIG_ResetExpansionIndex( work->objs );
		GV_Free( work->index );
		work->index = NULL ;
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
	RestoreBound(work) ;
	GV_Free(work->objs_bound_min);
	GV_Free(work->objs_bound_max);
	GV_Free(work->vanim) ;
}

#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT)
//#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

static int GetResources(Work *work,int name , int where )
{
	work->index = NULL ;
	work->rgbs = NULL ;
	work->vanim_info = 0;
	work->buf_info = 0 ;
	work->name = name ;

	work->rot = DG_ZeroSVector ;
	work->dir.vx = 0;
	work->dir.vy = 4096;
	work->dir.vz = 0;

	if ( GCL_GetOption( 'n' ) != NULL ){
		work->map_num = GCL_GetNextInt() ;
printf("MAPNAME [%d]\n",work->map_num);
		work->map_num = GM_GetMapID( work->map_num ) ;
printf("MAPID [%x]\n",work->map_num);
		work->map_num = GM_GetID (work->map_num) ;
printf("GetID [%d]\n",work->map_num);
	}else {
		if ( GCL_GetOption( 'd' ) != NULL ){
			work->map_num = GCL_GetNextInt() ;
		}else {
			work->map_num = 0 ;
		}
	}

	work->mode = MODE_BUILD;
	if ( GCL_GetOption( 'p' ) != NULL ){
		work->pos.vx = GCL_GetNextInt() ;
		work->pos.vy = GCL_GetNextInt() ;
		work->pos.vz = GCL_GetNextInt() ;
	}

	work->objs = GM_GetMapObjs(work->map_num) ;
	work->flag_buf = work->objs->flag;


	if ( GCL_GetOption( 't' ) != NULL ){
		work->time_a = DIRECT_TICK(GCL_GetNextInt()) ;
		work->time_b = DIRECT_TICK(GCL_GetNextInt()) ;
	}
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->len = (float) GCL_GetNextInt() ;
	}
	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->objs->n_models );
	if(work->vanim == NULL ) return 0 ;
	/* ＸＢＯＸ版では展開された状態での頂点アニメワークを作成 */
	if(!SetVbuf(work)) return 0 ;
	if(work->buf_info == 0 ) return 0 ;
	work->objs_bound_min = GV_Malloc( sizeof( FVECTOR ) * work->objs->n_models );
	work->objs_bound_max = GV_Malloc( sizeof( FVECTOR ) * work->objs->n_models );
	if(work->objs_bound_min == NULL ) return 0 ;
	if(work->objs_bound_max == NULL ) return 0 ;

	work->z_min = work->objs->bound_min.vz ;
	work->z_max = work->objs->bound_max.vz ;
	work->rate = (work->z_max - work->z_min) / (float)work->time_b ;
	SetBoundScale(work,fabs(work->len)) ;

	if(work->mode == MODE_BUILD ){
		work->cnt = 0 ;
	}else {
		work->cnt = (work->time_a+work->time_b) ;
	}
	work->status = STAGE_WAIT ;
	work->se_cyc = 255 ;
	work->se_cnt = 0 ;
	work->se_sw = 0 ;
	SIG_ActVanimStageBuild(work,work->objs ,work->cnt,work->z_min,&GM_PlayerPosition) ;
	return 1;
}

void *NewSigBreakStage( int name , int where )
{
	Work *work ;
//	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,name,where)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

#endif
