//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	brooklyn.c
	ブルックリンオブジェクト：フォグ無効
	1999/10/14 H.Tanaka
	2000/10/18 S.Okajima
	$Id: brooklyn.c,v 1.1.1.3 2002/11/19 11:47:46 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
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
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include		"camera.h"
#include	"../etc/ok_util.h"

extern int ok_flush_status;

#define	 N_UNITS	4
#define	 N_OBJS	 6
#define	 BASE_SCALE  15.0F
#define	 WIDE	   10000.0F
//#define	 BODY_FLAG  (DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_NOFOG|DG_FLAG_ONEPIECE)
#define	 BODY_FLAG0  (DG_FLAG_ONEPIECE)
#define	 BODY_FLAG1  (DG_FLAG_NOFOG|DG_FLAG_ONEPIECE)
#define	 FIRST_POS_X   130000.0F 
#define	 FIRST_POS_Y  0.0F

extern void Big_PutManHatLight_R2(FMATRIX *world, FVECTOR *cam) ;
extern void *NewFog_Wall(FVECTOR *model_center,SVECTOR *model_rot, FVECTOR *model_scale);

typedef  struct
{
	DG_OBJS  *objs[N_OBJS]  ;
	FVECTOR  pos[N_OBJS] ;
	FVECTOR  speed ;
	float	local_scale ;
	float	center ;
} Unit ;

typedef  struct
{
	GV_ACT_EX	actor ;
	int	   name   ;
	int	   map   ;

	void	*fog_wall_work0;
	void	*fog_wall_work1;

	int		wait_sync;

	FMATRIX   lights[2] ;
	Unit	  unit[N_UNITS] ;
} Work ;

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

static	void  Act(Work *work)
{
	GM_CameraSet	 *cam ;
	Unit			 *unit ;
	FVECTOR		  *pos ;
	FMATRIX		  world ;
	float			leng ;
	float			wide ;
	int			  i,j ;

	if( work->wait_sync-- <= 0 ) CheckMesgParam( work );

	if( PL_CurrentItem() == IT_Thermal ){
		work->lights[1].m[3][0] = 28.0F;
		work->lights[1].m[3][1] = 28.0F;
		work->lights[1].m[3][2] = 28.0F;
	}else{
		work->lights[1].m[3][0] = 128.0F;
		work->lights[1].m[3][1] = 128.0F;
		work->lights[1].m[3][2] = 128.0F;
	}

/*
	if( ok_flush_status!=0 ){
		unit = work->unit ;
		for(i = 0; i < N_UNITS; i++){
			for(j = 0; j < N_OBJS; j++){
				DG_InvisibleObjs( unit->objs[j] ) ;
			}
			unit ++ ;
		}
		return;
	}
*/

	/* キャラがマップにないときは 何も処理をしない */
	if(!(work->map & GM_CurrentStageMap)) return ;

	cam = GM_GetCurrentCameraSet(0) ;
	/* カメラの位置により大きさを変更 */
	unit = work->unit ;

	for(i = 0; i < N_UNITS; i++,unit++){
		pos = unit->pos ;
		world = DG_UnitMatrix ;
		/* カメラの位置により大きさを変更する */
		leng = pos->vx - cam->position.vx ;
		if(leng < pos->vx){
#if 0
			world.m[3][0] = pos->vx + cam->position.vx ;
#else
			world.m[3][0] = pos->vx ;
#endif
			unit->local_scale   = BASE_SCALE ;
		}else{
			world.m[3][0] = pos->vx ;
			unit->local_scale = leng / pos->vx * BASE_SCALE;
		}

#if 1	
		unit->center += unit->speed.vz * unit->local_scale ;
		wide = WIDE * unit->local_scale ;
		unit->center = (float)( (int)unit->center % (int)wide) ;
//		world.m[3][1] = pos->vy + cam->position.vy * 0.8F;
		world.m[3][1] = pos->vy ;
#else
		unit->center = 0.0F ;
		world.m[3][1] = 0.0F ;
#endif
		/* カメラの位置により高さを調節する */

		world.m[3][2] = unit->center ;
		world.m[0][0] = unit->local_scale ;
		world.m[1][1] = unit->local_scale ;
		world.m[2][2] = unit->local_scale ;

		if(i == 1) Big_PutManHatLight_R2(&world, &(cam->position));

		for(j = 0; j < N_OBJS; j++,pos++){	
			world.m[3][2] = unit->center + unit->local_scale * WIDE * (j - 3);
			DG_SetPos(&world) ;
			DG_PutObjs(unit->objs[j]);
			DG_VisibleObjs(unit->objs[j]) ;
		}
	}

}

static	void  Die(Work *work)
{
	int   i,j ;
	Unit  *unit ;

	unit = work->unit ;
	for(i = 0; i < N_UNITS; i++){
		for(j = 0; j < N_OBJS; j++){
			if(unit->objs[j] != NULL){
				DG_DequeueObjs( unit->objs[j] ) ;
				DG_FreeObjs( unit->objs[j] ) ;
			}
		}
		unit ++ ;
	}


	if( work->fog_wall_work0 != NULL ){
		GV_DestroyOtherActor( work->fog_wall_work0 );
		work->fog_wall_work0 = NULL;
	}
	if( work->fog_wall_work1 != NULL ){
		GV_DestroyOtherActor( work->fog_wall_work1 );
		work->fog_wall_work1 = NULL;
	}
}

static	int   GetResources(Work *work, int fog_flag)
{
	DG_DEF   *def[N_UNITS] ;

	FVECTOR  *pos ;
	Unit	 *unit ;
	int	  i,j ;
	
	FVECTOR   fog_pos ;
	SVECTOR   fog_rot ;
	FVECTOR   fog_scale ;
	int	   model_id ;
	int		height_flag;
	int		height;

	height_flag = 0;
	height = 0;
	if ( GCL_GetOption( 'h' ) != NULL ){
		height_flag = 1;
		height = GCL_GetNextInt() ;
	}

	unit = work->unit ;
	/* マンハッタン */
	def[0] = (DG_DEF*)GV_GetCache(GV_CacheID(1674082 /*"w00_r1"*/,'k')) ;
	def[1] = (DG_DEF*)GV_GetCache(GV_CacheID(1674083 /*"w00_r2"*/,'k')) ;
	def[2] = (DG_DEF*)GV_GetCache(GV_CacheID(1674084 /*"w00_r3"*/,'k')) ;
	def[3] = (DG_DEF*)GV_GetCache(GV_CacheID(1674085 /*"w00_r4"*/,'k')) ;

	for(i = 0; i < N_UNITS ; i++,unit ++){
		unit->center = 0.0F;
		unit->speed  = DG_ZeroVector ;
		if(i == 0){
			unit->speed.vz = 2.0F ;
		}else{
			unit->speed.vz = 2.0F - 0.3F * (i - 1) ;
		}
		unit->local_scale = BASE_SCALE ;
		pos = unit->pos ;
		for(j = 0; j < N_OBJS; j++,pos++){ 
			if(i == 0){
				pos->vx = FIRST_POS_X;
			}else{
//				pos->vx = FIRST_POS_X + 2000.0F + 1000.0F * (i - 1) ;
				pos->vx = FIRST_POS_X + 20000.0F + 10000.0F * (i - 1) ;
			}
			if( height_flag ){
				pos->vy = (float)height ;
			}else{
				pos->vy = FIRST_POS_Y ;
			}
			pos->vz = 0.0F ;

			unit->objs[j] = DG_MakeObjs(def[i],(fog_flag)?BODY_FLAG0:BODY_FLAG1,0) ;

			DG_SetLightMatrix((DG_OBJS *)unit->objs[j], work->lights) ;
			DG_InvisibleObjs(unit->objs[j]) ;
		}
	}
	DG_COPY_MAT( &work->lights[0], &DG_UnitMatrix ) ;
	DG_COPY_MAT( &work->lights[1], &DG_UnitMatrix ) ;
	work->lights[1].m[3][0] = 128.0F;
	work->lights[1].m[3][1] = 128.0F;
	work->lights[1].m[3][2] = 128.0F;


	/* 奥から登録していく */
	unit = work->unit ; 
	for(i = (N_UNITS - 1); i >= 0 ; i--){
		for(j = 0; j < N_OBJS; j++){
			DG_QueueObjs(unit[i].objs[j]) ;
		}
	}
	
	fog_pos.vx = 95000.0F ;
	fog_pos.vy = -30000.0F ;
	fog_pos.vz = -300000.0F ;
	fog_rot.vx = 0 ;
	fog_rot.vy = 0;
	fog_rot.vz = 0 ;
	fog_scale.vx = 10.0F ;
	fog_scale.vy = 15.0F ;
	fog_scale.vz = 35.0F ;
	model_id = 3228778 /*"w00_fog"*/ ;

	work->fog_wall_work0 = NewFog_Wall( &fog_pos,&fog_rot,&fog_scale);
	if( work->fog_wall_work0 == NULL ) return -1 ;

	fog_pos.vz = 250000.0F ;
	fog_rot.vy = 2048 ;

	work->fog_wall_work1 = NewFog_Wall( &fog_pos,&fog_rot,&fog_scale);
	if( work->fog_wall_work1 == NULL ) return -1 ;

	work->wait_sync = 2;

	return 0 ;
}

void *NewBrooklyn3D(int name, int map)
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name ;
		work->map  = map ;
		if ( GetResources( work, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewBrooklyn3D_Fog(int name, int map)
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name ;
		work->map  = map ;
		if ( GetResources( work, 1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
