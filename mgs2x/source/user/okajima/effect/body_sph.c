//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	body_sph.c
	体からの水飛沫

	1999/10/23 S.Okajima
	$Id: body_sph.c,v 1.4 2002/11/23 12:16:42 Yoshizawa1 Exp $
*/
#endif

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#include "bp_matrix.h"

#define	JOINT_NUM		(22)

#define	POS_VS_UVS	(1 + 2)

#define	DIV_VERTS		(1)
#define	DIV_NORMS		(1)
#define	SCR_DIVISION	( DIV_VERTS + DIV_NORMS )

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
//#define	N_VERTS2		(32)
#define	N_VERTS2		(16)
//関節毎にスクラッチパット使用
#define	N_PRIMS2		( JOINT_NUM )


#define	PARAM1		( (MAX_FVECTOR - N_VERTS2*POS_VS_UVS) / SCR_DIVISION )
#define	PARAM2		(PARAM1 * 16)

#if 0 //BP
//#ifdef PSX2
#define	SCR_POS		((void *)SCRPAD_ADDR)
#else
#define	SCR_POS		((char *)SCRPAD_ADDR)
#endif

#define	SCR_UVS		(SCR_POS   + N_VERTS2 * 16)
#define	SCR_VERTS	(SCR_UVS   + N_VERTS2 * 16 * 2)
#define	SCR_NORMS	(SCR_VERTS + PARAM2 * DIV_VERTS)


#if 1
#define	TEX_NAME		(7733153 /*"splash06_alp"*/)
#define	RAND_WIDTH		(80.0f)
//#define	SCALE			(0.95f)
#define	SCALE			(1.1f)
#define	P_ALPHA_MAX		(24)
#define	P_RGB_MAX		(128)
#define	SIZE			(50)
#else
#define	TEX_NAME		(6715088 /*"rcm_l_msk"*/)
#define	RAND_WIDTH		(0.0f)
#define	SCALE			(1.00f)
#define	P_ALPHA_MAX		(255)
#define	P_RGB_MAX		(255)
#define	SIZE			(20)
#endif



#define	SHIFT_ROT		(256.0f)

#define	NORMS_LIMIT		(0.7f)
//#define	NORMS_LIMIT		(0.5f)
#define	LENGTH_LIMIT	(8000.0f)
#define	LENGTH_MIN		(5000.0f)

extern int ok_flush_status;
extern SVECTOR G_wind_rot;	/* 風向 */

extern int	OK_BodySplashFlag;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;
	int			name;
	int			map;

	CONTROL		*control;
	DG_OBJS		*objs;
	SVECTOR		before_rot;
	FVECTOR		drop_step;

	int			chara_name;
	int			n_models;

	DG_PRIM2	*prim ;
	FMATRIX		*world[JOINT_NUM];
	FVECTOR		*verts[JOINT_NUM];
	FVECTOR		*norms[JOINT_NUM];
	int			n_verts[JOINT_NUM];

	int			*flag;
	int			dummy;
	int			before_status;

	float		length_max;
	float		length_min;

	FVECTOR		scale;

} Work ;

#if 0 //BP_UNUSED
static inline void OK_RotVec( FVECTOR *res, FVECTOR *v )
{
//..使用していない
#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2			vf8, 0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );

#endif
}
#endif

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int flag;
	int i, n;
	int	max_num;
	int	clock;
	FVECTOR	*buff;
	FVECTOR *from;
	FVECTOR	*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		*n_verts;
	FVECTOR	**verts;
	FVECTOR	**norms;
	FMATRIX	**world;
	FVECTOR	direction;
	SVECTOR	rot;
	GV_MSG *msg ;
	CONTROL		*ctrl;
#ifdef KP_WINDOWS
	unsigned int	rseed ;
#endif

	if( work->objs==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}

	//外部条件
	for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ ){
		switch( msg->message[0] ){
		  case 0:
			DG_InvisiblePrim2( work->prim ) ;
			work->before_status = 0;
			work->dummy = 0;
			return;
			break ;
		  case 1:
			DG_VisiblePrim2( work->prim );
			work->before_status = 1;
			work->dummy = 1;
			break ;
		  case 2:
			DG_InvisiblePrim2( work->prim ) ;
			GV_DestroyActor( work ) ;
			return;
			break ;
		}
	}

	switch( *work->flag ){
	  case -1:
		DG_InvisiblePrim2( work->prim ) ;
		GV_DestroyActor( work ) ;
		return;
	  case 0:
		DG_InvisiblePrim2( work->prim ) ;
		work->before_status = 0;
		return;
	  case 1:
	  default:
		DG_VisiblePrim2( work->prim );
		work->before_status = 1;
		break;
	}



	//内部条件
	work->control = GM_SearchWhere(work->chara_name);
	if( work->control==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}


	ctrl = work->control;
	flag=0;
	if( (ctrl!=NULL)  &&  !GM_CheckGameStatus( STATE_DEMO ) ){
		if( ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ){ //自前で天井を見る
			flag = HZX_LevelHazardCheck( ctrl->hzx_id, &( ctrl->mov ), ctrl->hzx_check_type, ctrl->flr_flag ) ;
		}else{
			flag = work->control->level_found ;
		}
		if( (flag & 2)
		 || ( (work->flag==&work->dummy) && (work->objs->flag & DG_FLAG_INVISIBLE0) )   ){
			DG_InvisiblePrim2( work->prim );
			return;
		}
	}


	if( GM_CheckPlayerStatus(PLAYER_CB_BOX) ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}

	if( work->before_status ){
		DG_VisiblePrim2( work->prim ) ;
	}else{
		DG_InvisiblePrim2( work->prim ) ;
	}

#if 0
	_sceVu0SubVector( &fvtemp, &work->control->mov, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	ratio = GV_VecLen3F( &fvtemp );
	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		if ( ratio > LENGTH_LIMIT || ok_flush_status!=0 ){
			DG_InvisiblePrim2( work->prim ) ;
			work->before_status = 0;
			return;
		}else if( ratio < LENGTH_MIN ){
			alpha = P_ALPHA_MAX;
		}else{
			ratio = (LENGTH_LIMIT - ratio) / (LENGTH_LIMIT - LENGTH_MIN);
			alpha = (int)((float)P_ALPHA_MAX * ratio);
		}
	}else{
		ratio = 0.8f ;
		alpha = (int)((float)P_ALPHA_MAX * ratio);
		if ( work->objs->flag & DG_FLAG_INVISIBLE0 && 0 ){
			DG_InvisiblePrim2( work->prim );
			work->before_status = 0;
			return;
		}
	}
//if(GV_Time%60==0) printf("%d\n",alpha);

//	alpha = (int)((float)alpha * OK_slow_param);
#endif

#ifdef KP_WINDOWS	// Skip
	if( DG_CurrentFrameDrawSkip() ){ return ; }
#endif

   if (DG_Arm_SkipThisFrame())
   {
      return;
   }

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;


	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	direction.vx = 0.0f;
	direction.vy = 0.0f;
	direction.vz = -1.0f;
	direction.vw = 1.0f;

	rot.vx = (G_wind_rot.vx + 512-256-128) & 4095;
	rot.vy = (G_wind_rot.vy + 2048 + (short)(rnd()*SHIFT_ROT - SHIFT_ROT*0.5f)) & 4095;
	rot.vz = 0;
	OK_DirectionSmoother( &work->before_rot, &rot, 0.95f );

	DG_SetPos2( &DG_ZeroVector, &work->before_rot );
	DG_RotVector( &direction, &direction, 1 );

#ifdef KP_WINDOWS
	rseed = m_get_rndseed() ;	// rnd()高速化
#endif
	n_verts  = work->n_verts;
	pos      = work->prim->pos[ clock ];
	uvrgbwh  = work->prim->uvrgb[ clock ];
	world    = work->world;
	verts    = work->verts;
	norms    = work->norms;
	i = work->n_models ; 
	while( --i>=0 ){
#ifndef KP_WINDOWS
//printf("a:%d::%x  %d\n",i,*verts,*n_verts);
		OK_Mem_Scr( SCR_VERTS, *verts,  sizeof(FVECTOR), *n_verts) ;
//printf("b:%d::%x  %d\n",i,*norms,*n_verts);
		OK_Mem_Scr( SCR_NORMS, *norms,  sizeof(FVECTOR), *n_verts) ;
//printf("c\n");
#endif

		DG_SetPos( *world );
		DG_RotVector( SCR_NORMS, SCR_NORMS, *n_verts );

#ifndef KP_WINDOWS
		from = SCR_VERTS;
		buff = SCR_NORMS;
#else
		from = *verts ;
		buff = *norms ;
#endif
		n    = *n_verts;

		max_num = N_VERTS2;

		while ( -- n >= 0 ) {
			buff->vw = _sceVu0InnerProduct( buff, &direction );
			if( buff->vw > NORMS_LIMIT ){
				DG_PutVector( from, pos, 1 );
//				_sceVu0ScaleVector( buff, buff, SCALE );
//				_sceVu0AddVector( pos, pos, buff );
#ifndef KP_WINDOWS
				pos->vx += frnd()*RAND_WIDTH;
				pos->vy += frnd()*RAND_WIDTH;
				pos->vz += frnd()*RAND_WIDTH;
#else
				pos->vx += m_frnd(rseed)*RAND_WIDTH;
				pos->vy += m_frnd(rseed)*RAND_WIDTH;
				pos->vz += m_frnd(rseed)*RAND_WIDTH;
#endif
				uvrgbwh->a = P_ALPHA_MAX;
				uvrgbwh++;
				pos++;
				if( --max_num <= 0 ) break;
			}
			buff ++ ;
			from ++ ;
		}

//printf("%d\n",max_num);

		while ( --max_num >= 0 ) {
			uvrgbwh->a = 0;
			uvrgbwh++;
			pos++;		/* skip させる */
		}

		n_verts++;
		world++;
		verts++;
		norms++;
	}
#ifdef KP_WINDOWS
	m_set_rndseed(rseed) ;	// rnd()高速化
#endif

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	pos  = (FVECTOR *)SCRPAD_ADDR ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos->vx = 0.0f * (float)k;
			pos->vy = 0.0f * (float)j;
			pos->vz = 0.0f ;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
		}
	}
	OK_Scr_Mem( prim->pos[ 0 ], SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS2 * work->n_models ) ;
	OK_Scr_Mem( prim->pos[ 1 ], SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS2 * work->n_models ) ;


	//-------------------------------
	uvrgbwh = prim->uvrgb[ 0 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = SIZE ;
			uvrgbwh->h = SIZE ;

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			uvrgbwh ++ ;
		}
	}
	uvrgbwh = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = SIZE ;
			uvrgbwh->h = SIZE ;

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			uvrgbwh ++ ;
		}
	}
}


static int GetResources( Work *work, DG_OBJS *org_objs, CONTROL *control, int model_id )
{
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int		i ;

	work->chara_name = control->name;
	OK_BodySplashFlag = 1;

	work->objs=org_objs;
	work->control = control;

	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 


	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_models, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( TEX_NAME );

	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( work->prim );

	/* モデルの共有頂点データ取得 */
	cvd_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );
//	cvd_def = GV_GetCache( GV_CacheID( 3075579 /*"sna_skl3"*/, 'c' ) );
//	cvd_def = GV_GetCache( GV_CacheID( 13488099 /*"crg_raindammy"*/, 'c' ) );

//printf("org_objs->n_models:%d\n",org_objs->n_models);

	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > PARAM1 ){
//			printf("%d::%d\n",work->n_verts[i],PARAM1 );
			work->n_verts[i] = PARAM1;
		}
		work->verts[i]   = cvd_def->models[i].verts;
		work->norms[i]   = cvd_def->models[i].norms;
		work->world[i]   = &org_objs->objs[i].world;
	}

#if 0
{
	int	j;
	i = work->n_models ; 
	while( --i>=0 ){
//printf("%d:::work->n_verts:%d:\n",i,work->n_verts[i]);
		for( j=0; j<work->n_verts[i]; j++ ){
//printf("%x\n",work->verts[i]);
//printf(":%d:verts:%f:%f:%f\n",j,work->verts[i][j].vx,work->verts[i][j].vy,work->verts[i][j].vz);
//printf("%x\n",work->norms[i]);
//printf(":%d:verts:%f:%f:%f\n",j,work->norms[i][j].vx,work->norms[i][j].vy,work->norms[i][j].vz);
		}
	}
}
#endif

	return (0);
}
static int GetResourcesScn( Work *work, int name, int where )
{
    OBJECT		*object;
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int		i,j ;
	FVECTOR		*fvtemp0;
	FVECTOR		*fvtemp1;

	work->name = name;
	work->map = where;

	if ( GCL_GetOption( 'n' ) != NULL ){
		work->chara_name = GCL_GetNextInt() ;
		work->control = GM_SearchWhere(work->chara_name);
		if( work->control!=NULL ){
			object = (OBJECT *)(work->control + 1);
			if( object==NULL ){
				return -1;
			}
			work->objs = object->objs;
		}else{
			return -1;
		}
	}else{
		return -1;
	}

	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 



	if ( GCL_GetOption( 'c' ) != NULL ){
		cvd_def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'c' ) );
//		cvd_def = GV_GetCache( GV_CacheID( 3075579 /*"sna_skl3"*/, 'c' ) );
	}else{
		return -1;
	}

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_models, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( TEX_NAME );

	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( work->prim );


//printf("work->objs->n_models:%d\n",work->objs->n_models);

#if 0
	i = work->n_models; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
//printf("work->n_verts:%d\n",work->n_verts[i]);
		if( work->n_verts[i] > PARAM1 ){
//			printf("%d::%d\n",work->n_verts[i],PARAM1 );
			work->n_verts[i] = PARAM1;
		}
		work->verts[i]   = cvd_def->models[i].verts;
		work->norms[i]   = cvd_def->models[i].norms;
		work->world[i]   = &work->objs->objs[i].world;
	}
#else
	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > PARAM1 ){
			work->n_verts[i] = PARAM1;
		}
		work->world[i]   = &work->objs->objs[i].world;
		fvtemp0 = work->verts[i]   = cvd_def->models[i].verts;
		fvtemp1 = work->norms[i]   = cvd_def->models[i].norms;
		for( j=0; j<work->n_verts[i]; j++ ){
			if( _sceVu0InnerProduct( fvtemp0, fvtemp1 ) < 0.0f ){
				_sceVu0ScaleVector( fvtemp1, fvtemp1, -1.0f );
			}
			fvtemp0++;
			fvtemp1++;
		}
	}
#endif







	return (0);
}


/* ---------------------------------------------------------------- */
void *NewBodySplash( DG_OBJS *objs, CONTROL *control, int model_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = &work->dummy;
		work->dummy = 1;
		work->before_status = 1;
		work->chara_name = -1;

		if ( GetResources( work, objs, control, model_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
/* ---------------------------------------------------------------- */
void *NewBodySplash3( DG_OBJS *objs, CONTROL *control, int model_id, int name )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = &work->dummy;
		work->dummy = 1;
		work->before_status = 1;
		work->name = name ;
		work->chara_name = -1;

		if ( GetResources( work, objs, control, model_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */
void *NewBodySplash2( DG_OBJS *objs, CONTROL *control, int model_id, int *flag )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = flag;
		work->before_status = 1;

		work->length_max = LENGTH_LIMIT;
		work->length_min = LENGTH_MIN;
		work->chara_name = -1;

		if ( GetResources( work, objs, control, model_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


/* ---------------------------------------------------------------- */
void *NewBodySplashScn( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = &work->dummy;
		work->dummy = 1;
		work->before_status = 1;
		work->chara_name = -1;

		if ( GetResourcesScn( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
