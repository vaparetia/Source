//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breakobj.c
	モデル頂点を法線方向に飛散
	2002/03/12 K.Sigeno
	$Id: xbreakobj.c,v 1.6 2003/01/04 12:02:07 takaki Exp $
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

#define	BREAK_VER (3)
//#include "enemy.h"
//#include "enemy.x"
#define MDL_NUM (22)
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	CVECTOR		col ;
	SVECTOR		rot ;
	float		speed ;
	float		decay ;
	int			cnt ;
	int			n_models ;
	int			flag ;
	int			decay_time ;

	u_short		*index ;			/* 差し替え用インデックス */
} Work ;

#define GET_LIGHT	(0x01)
#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)
#if 0
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif


static void SetAmb(Work *work,u_char r,u_char g,u_char b){
	work->lights[1].m[ 3 ][ 0 ] = (float) r ;
	work->lights[1].m[ 3 ][ 1 ] = (float) g ;
	work->lights[1].m[ 3 ][ 2 ] = (float) b ;
}
#define Q_WORD (2)
static void SvectoFvel( FVECTOR *sv , FVECTOR *res, float speed )
{
	FVECTOR vel ;
	vel.vx = (float) sv->vx ;
	vel.vy = (float) sv->vy ;
	vel.vz = (float) sv->vz ;
	GV_LenVec3F( &vel, res, 0.0f, speed ) ;
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
	res->vy = vec->vx * scale ;
	res->vz = vec->vx * scale ;
}
/* ---------------------------------------------------------------- */
/*
	obj = v_anime->obj ;
	mdl = obj->model ;
	obj->verts = (SV1ECTOR*)mdl->packs->verts ;
	obj->norms = (SVECTOR*)mdl->packs->norms ;
*/
static void SIG_ActVRTRGBreak(Work *work,DG_OBJS *objs ,float speed,int cnt) {
	int				n_verts, i, j, k, l;
	DG_OBJ			*obj;
	DG_VERTS_ANIME	*vanim ;
	DG_VERTEX_KMSM	*d_verts, *org_verts, *s_verts ;
	FVECTOR			norm, *verts, *top_verts, move_vec ;
	DG_MDLPACK		*mdl_pack;
	DG_MDL			*mdl;
	u_short			*index, top_index ;

	/*バウンディングボックスも拡大*/
	objs->bound_min.vx -= speed ;
	objs->bound_min.vy -= speed ;
	objs->bound_min.vz -= speed ;
	objs->bound_max.vx += speed ;
	objs->bound_max.vy += speed ;
	objs->bound_max.vz += speed ;


	for(j= 0 ;j<objs->n_models;j++){
		vanim = &work->vanim[ j ];
		DG_SwitchVAnimeBuffer( vanim );
		DG_RestartAnimVerts( vanim );

		obj = vanim->obj ;
		mdl = obj->model ;
		/* バウンディングボックスサイズの補正 */
		obj->bound_min.vx -= speed ;
		obj->bound_min.vy -= speed ;
		obj->bound_min.vz -= speed ;
		obj->bound_max.vx += speed ;
		obj->bound_max.vy += speed ;
		obj->bound_max.vz += speed ;

		d_verts = vanim->verts_top[ vanim->buffer_clock ];
		s_verts = vanim->verts_top[ 1 - vanim->buffer_clock ];
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
				//s_verts = (void*)( (char*)org_verts + mdl->stride * (*index) );
				verts->vx = s_verts->vx ;
				verts->vy = s_verts->vy ;
				verts->vz = s_verts->vz ;
				s_verts = (void*)( (char*)s_verts + mdl->stride );
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
					ScaleNormalize( &move_vec, &norm, speed );

					/* 切り出した１ストリップ分の頂点に対して移動計算 */
					if ( i <= ( ( cnt + 1 ) * 16 ) ){
						/* 座標を広げる */
						for ( l = 0 ; l < n_verts ; l++, verts++ ){
							verts->vx = verts->vx - move_vec.vx ;
							verts->vy = verts->vy - move_vec.vy ;
							verts->vz = verts->vz - move_vec.vz ;
							d_verts->vx = verts->vx ;
							d_verts->vy = verts->vy ;
							d_verts->vz = verts->vz ;
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
							d_verts->wt = 32767 ;
#else
							d_verts->wt = 0 ;
#endif
							d_verts = (void*)( (char*)d_verts + mdl->stride ) ;
						}
					} else {
						/* そのままを維持 */
						for ( l = 0 ; l < n_verts ; l++, verts++ ){
							d_verts->vx = verts->vx ;
							d_verts->vy = verts->vy ;
							d_verts->vz = verts->vz ;
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
							d_verts->wt = 32767 ;
#else
							d_verts->wt = 0 ;
#endif
							d_verts = (void*)( (char*)d_verts + mdl->stride ) ;
						}
					}

					/* 展開頂点データ生成ワーク初期化 *//* 次の処理のための初期化 */
					verts = (FVECTOR*)SCRPAD_ADDR ;
					top_index = index[1] ;
					n_verts = 0 ;
				}
			}
		}

#ifdef KP_WINDOWS
		DG_AssignDGObjVertexBuffer(obj);
#endif
	}
}
//#define DECAY_RATE (0.99f)
//#define START_VEL	(45.0f)
//#define LIFE_TIME	(120)
//#define LIFE_TIME	(60)
#define LIFE_TIME	(60)

//#define FADE_TIME	(60)
//#define FADE_TIME	(30)
#define FADE_TIME	(30)

#define	BASE_ROT	(8)
#define VEL_MIN		(5.0f)
#define	F_ROT_RATE	(0.10f)
static void Act(Work *work)
{
	float rate ,speed;
	SVECTOR rot;
	CVECTOR col ;
	speed = work->speed ;

#if (BREAK_VER == 3)
	if(work->cnt > work->decay_time) {
		speed *= work->decay ;
	}
#endif

	SIG_ActVRTRGBreak(work,work->objs ,speed ,work->cnt) ;
#if 0
printf("speed = %6.2f : speed = %6.2f, decay = %6.2f, decay_time = %d, cnt = %d\n",
	   speed, work->speed, work->decay, work->decay_time, work->cnt );
#endif
	if(work->cnt > LIFE_TIME) {
		GV_DestroyActor(work) ;
		return ;
	}



#if 0
/*点滅*/
	if(work->flag & GET_LIGHT ){
		DG_GetLightMatrix( (FVECTOR *)&work->objs->world.m[3], work->lights );
	}
	if(work->cnt>FADE_TIME){
		if(work->cnt&1){
			DG_VisibleObjs(work->objs) ;
		}else {
			DG_InvisibleObjs(work->objs) ;
		}
	}
#else
/*フェード*/
	if(work->cnt<(LIFE_TIME-FADE_TIME)){
		rate = 1.0f ;
	}else {
		rate = (float)  (LIFE_TIME-(work->cnt))  /(float)FADE_TIME ;
	}
//	rate *= 0.5f;
//	rate *= 0.0f;

//	rate *= 0.75f;
	rate *= 0.50f;

	col.r = rate * (float)work->col.r ;
	col.g = rate * (float)work->col.g ;
	col.b = rate * (float)work->col.b ;

	SetAmb(work,(u_char)col.r ,(u_char)col.g,(u_char)col.b) ;
#endif

#if (BREAK_VER == 1)
//Ver 1.0
	if(work->speed > VEL_MIN){
		work->speed *= work->decay ;
		work->rot.vx = (float)work->rot.vx * work->decay ;
		work->rot.vy = (float)work->rot.vy * work->decay ;
		work->rot.vz = (float)work->rot.vz * work->decay ;
	}
	DG_SetPos( &work->objs->world ) ;
	DG_RotatePos(&work->rot) ;
#elif (BREAK_VER == 2)
//Ver 1.1 F.Curve
	if(work->speed > VEL_MIN){
		work->speed *= work->decay ;
		rot.vx = work->rot.vx ;
		rot.vy = work->rot.vy ;
		rot.vz = work->rot.vz ;
	}else {
		rot.vx = work->rot.vx*F_ROT_RATE ;
		rot.vy = work->rot.vy*F_ROT_RATE ;
		rot.vz = work->rot.vz*F_ROT_RATE ;
	}
	DG_SetPos( &work->objs->world ) ;
	DG_RotatePos(&rot) ;
#elif (BREAK_VER == 3)
	if(work->cnt > work->decay_time) {
		rot.vx = work->rot.vx*F_ROT_RATE ;
		rot.vy = work->rot.vy*F_ROT_RATE ;
		rot.vz = work->rot.vz*F_ROT_RATE ;
	}else {
		rot.vx = work->rot.vx ;
		rot.vy = work->rot.vy ;
		rot.vz = work->rot.vz ;
	}
	DG_SetPos( &work->objs->world ) ;
	DG_RotatePos(&rot) ;
#endif

	DG_PutObjs( work->objs );
	work->cnt++ ;
}
static void Die(Work *work)
{
	SIG_ResetExpansionIndex( work->objs );
	if ( work->index != NULL ) GV_Free( work->index );

	SIG_FreeVanim(work->vanim,work->n_models) ;
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
}

static int GetResources(Work *work, int mdl,FMATRIX *world ,float speed,float decay,
CVECTOR *col,SVECTOR *rot,int decay_time)
{
	FVECTOR tmp ;
	DG_DEF	*def ;
	int i;

	work->flag = 0 ;
	def = (DG_DEF*) GV_GetCache( GV_CacheID( mdl, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_NOFOG|DG_FLAG_ONEPIECE, DG_CHANL_MAIN ) ;
	ASSERT(work->objs != NULL ) ;

//	for(i= 0 ;i<work->objs->n_models;i++){
//		if(i!=HUMAN21_ATAMA ){
//			work->objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
//		}
//	}

	DG_QueueObjs( work->objs ) ;
	DG_SetPos( world ) ;
	DG_PutVector(&DG_ZeroVector,&tmp,1);
	DG_PutObjs( work->objs );

//	PosBox(&tmp,250.0f , NULL );
//printf("F OBJ X[%f] Y[%f] Z[%f] \n",tmp.vx,	tmp.vy,	tmp.vz	);


	if(col != NULL){
		work->col = *col ;
		SetAmb(work,(u_char)col->r ,(u_char)col->g,(u_char)col->b) ;
	}else {
		work->flag = GET_LIGHT ;
	}
//	SetAmb(work,(u_char)127 ,(u_char)127,(u_char)127) ;
	DG_SetLightMatrix( work->objs, work->lights );

#ifdef START_VEL
	work->speed = START_VEL ;
#else
	work->speed = speed ;
#endif
#ifdef DECAY_RATE
	work->decay = DECAY_RATE;
#else
	work->decay = decay;
#endif

	work->decay_time = DIRECT_TICK(decay_time);


	work->n_models = work->objs->n_models ;
//	DG_VERTS_ANIME	*vanim;

	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->n_models );
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

//	work->lit_def = GM_GetMap( where )->light ;

//	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
//	DG_PutObjs( work->body.objs );
//  GM_GroupObjs( work->body.objs, where ) ;

    /* プリシェード */
//	    DG_MakePreshade( work->body.objs, work->lit_def ) ;
//		GM_ConfigObjectLight(&(work->body),work->lights) ;
//	DG_InvisibleObjs(work->objs) ;
	work->cnt = 0 ;
	work->rot = *rot ;

	return 1;
}

void *NewSigBreakObj(int mdl,FMATRIX *world,float speed ,float decay,CVECTOR *col,SVECTOR	*rot,int decay_time)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,mdl,world,speed,decay,col,rot,decay_time)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

#endif
