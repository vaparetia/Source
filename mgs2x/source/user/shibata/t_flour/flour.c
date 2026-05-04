//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	flour.c
	メリケン粉

	2000/02/01 H.TANAKA
	2000/03/31 S.Okajima
	$Id: flour.c,v 1.1.1.3 2002/11/19 11:48:50 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"

/* ---------------------------------------------------------------- */
#define  FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define   BODY_FLAG (DG_FLAG_TEXT | DG_FLAG_FINISHCALC | DG_FLAG_SHADE | DG_FLAG_ONEPIECE)

#define  FLOUR_VITALITY   5
#define  MAX_FLOUR_DOWN 8
#define  MAX_FLOUR_FALL 2
#define  MAX_FLOUR_GAS 1
enum {
	ACHUU=0,
	REQ_NO
};


typedef   struct
{
	GV_ACT_EX			actor;
	TARGET			target ;
	POWER_TARGET	power ;

	FVECTOR			pos ;
	FMATRIX			light[2] ;

	SVECTOR			rot ;

	DG_OBJS			*objs ;
	DG_MDL			*mdl ;
	DG_DEF			*def ;
	int				name;
	int				map;
	
	int				mdl_id;
	int				down_count;
	int				fall_count;
	int				gas_count;
	int				id ;
} Work ;

extern void  *NewSpreadFlour(FMATRIX *world ) ;
extern void  *NewFlour_Splay(FMATRIX  *world) ;
extern void  BIG_VectoMat(FMATRIX *world, FVECTOR *force, FVECTOR *hit, int mode) ;
extern void *NewFlour_Fall( int map, int name, FVECTOR *hit, FVECTOR *force );
extern void *NewFlour_Down( int map, int name, FVECTOR *hit, FVECTOR *force );
extern void *NewFlour_Sara( int map, int name, FVECTOR *hit, FVECTOR *force );
extern void *NewFlour_Gas3( FVECTOR *center );

static  void  Flour_TargetCallBack(TARGET *off, TARGET *def, void *ptr)
{
	Work		  *work = (Work *)ptr ;
	FMATRIX	   world ; 
	FVECTOR	   hit ;
	FVECTOR	   t_force ;
	int		   mode ;
	float		 tmp ;

	if(work->objs){
		if(def->damaged & TARGET_POWER){
			if( ( off->class & TARGET_POWER ) && ( off->power != NULL ) ) {
				t_force = def->power->force = off->power->force;
				if( def->weapon_type & (WP_BULLET|WP_M92|WP_BLAST) ){
					def->weapon_type = 0;
//printf("Flour_TargetCallBack\n");

					/* worldを求める */
					/* 小麦粉 拡散 */					
					if( def->weapon_type & WP_BLOW ){
						_sceVu0AddVector( &hit, &def->center, &def->offset );
					}else{
						DG_COPY_VEC( &hit, &def->hit );
					}
					GM_SeSetMode( SD_A_RICKOM01, &hit, GM_SEMODE_BOMB ) ;
					world = DG_UnitMatrix ;
					DG_COPY_VEC( (FVECTOR *)world.m[3], &def->hit );
					NewSpreadFlour(&world) ;

					/* 飛び散りの向きを測定 */
					if(t_force.vy < 0.0F)
					t_force.vy = - t_force.vy ;
					tmp = bp_sqrtf(t_force.vx * t_force.vx + t_force.vz * t_force.vz) ;  //BP_MATH - emulate PS2 sqrtf
					if(t_force.vy < tmp)
					t_force.vy = tmp ;
					mode = 0x0012 ;
					BIG_VectoMat(&world,&t_force,&hit,mode) ;
/*	柴田 付け足し */

					if( work->fall_count++ < MAX_FLOUR_FALL){
						NewFlour_Fall( work->map,work->name, &hit,&t_force );
						NewFlour_Sara( work->map,work->name, &hit,&off->power->force );
					}
					//if( work->gas_count++ < MAX_FLOUR_GAS)
					NewFlour_Gas3(&hit);
						
						//NewFlour_Sara( work->map,work->name, &hit,&off->power->force );


/* ここまで */
					/* 飛び散りの小麦粉 */
					NewFlour_Splay( &world ) ;

					if(!(work->id & 0x80000000)) GCL_ExecProc(work->id, NULL) ;

//					def->power->vital -- ;
					if(def->power->vital <= 0){
						DG_DequeueObjs(work->objs) ;
						DG_FreeObjs(work->objs) ;
						work->objs = NULL ;		
						GM_ClearTargetDamage( def ) ;
						//GM_FreeTarget( def ) ;
					}
				}
			}
		}
	}
	GM_ClearTargetDamage( def ) ;
}

static  void  Act( Work *work)
{
	GV_MSG *msg;
	int mes_num;
	int num;

//	if( !(work->map & GM_CurrentStageMap) ) return ;
	if( work->objs == NULL ) return ;


	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case ACHUU:
		  default:
			{
				extern void *New_ACHUU( FMATRIX *world );
				New_ACHUU( &GM_PlayerBody->objs->objs[12].world );
			}
			break;
		}
		msg--;
	}

	DG_GetLightMatrix((FVECTOR *)work->objs->world.m[3],work->light) ;
}

static  void  Die( Work *work)
{
	if(work->objs != NULL){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs ) ;
	}
	GM_FreeTarget( &work->target ) ;
}

/* オプションを指定する */
static  int  GetOptionValue( Work *work )
{
	int   buf[3] ;
	
	/* ポジション決定 */
	if(GCL_GetOption('p') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		GV_IVtoFV(buf,(float *)&work->pos,3) ;
	}else{
		work->pos.vx = 0.0F ;
		work->pos.vy = 0.0F ;
		work->pos.vz = 0.0F ;
	}

	/* 回転取得 */
	if( GCL_GetOption('r') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		work->rot.vx = buf[0] ;
		work->rot.vy = buf[1] ;
		work->rot.vz = buf[2] ;
	}else{
		work->rot.vx = 0 ;
		work->rot.vy = 0 ;
		work->rot.vz = 0 ;
	}

	/* モデル */
	if( GCL_GetOption('m') != NULL){
		work->mdl_id = GCL_GetNextInt() ;
	}else{
		printf("there is not option m of flour \n") ;
		return -1 ;
	}

	/* ブロックIDを取得 */
	if( (GCL_GetOption('e')) != NULL){
		work->id = GCL_GetNextInt() ;
	}else{
		work->id = 0x80000000 ;
	}

	return 0 ;

}

/* ターゲットを取得 */
static void InitTarget(Work *work, FMATRIX *world)
{
	TARGET	 *t ;
	DG_MDL	 *mdl ;
	FVECTOR	t_size ;
	FVECTOR	pos ;
	DG_DEF	*def = work->def;
	t = &work->target ;
	mdl = work->mdl ;
#if 0
	t_size.vx = (mdl->ux - mdl->lx) * 0.5F ;
	t_size.vy = (mdl->uy - mdl->ly) * 0.5F ;
	t_size.vz = (mdl->uz - mdl->lz) * 0.5F ;
#else
	t_size.vx = (def->ux - def->lx) * 0.5F ;
	t_size.vy = (def->uy - def->ly) * 0.5F ;
	t_size.vz = (def->uz - def->lz) * 0.5F ;
#endif
	pos.vx = 0.0F ;
	pos.vy = t_size.vy ;
	pos.vz = 0.0F ;

	t_size.vx *= 0.9F ;
	t_size.vy *= 0.9F ;
	t_size.vz *= 0.9F ;

	printf("t_size %f:%f:%f:\n",t_size.vx,t_size.vy,t_size.vz);
	
	GM_SetTarget(t,TARGET_DEFENSE | TARGET_ROTATE | TARGET_SEEK, 0, BOTH_SIDE, &t_size, &pos) ;
	GM_SetPowerTarget(t,&work->power,POWER_DECREASE,FLOUR_VITALITY,0,0,&DG_ZeroVector) ;
	GM_SetTargetCallBack(t,Flour_TargetCallBack, work) ;
	GM_PutTarget(t) ;

	GM_MoveTarget2(t,&work->objs->world) ;

//	NewTargetView(t, 255, 255, 0) ; 
}

static  int  GetResources(Work *work)
{
	DG_OBJS	  *objs ;
	DG_DEF	   *def ;
	FMATRIX	  world ;


	work->down_count = 0;
	work->fall_count = 0;
	work->gas_count = 0;

	/* モデルを取得 */
	if( GetOptionValue(work) ) return -1 ;

	/* モデル情報 */
	def = work->def = (DG_DEF *)GV_GetCache(GV_CacheID(work->mdl_id,'k')) ;
	if(def == NULL){
		printf("there is not a cfr_trp.kms\n") ;
		return -1 ;
	}

	/* モデル取得 */
	objs = work->objs = DG_MakeObjs(def,BODY_FLAG, 0);
	if(objs == NULL)
	return -1;
	DG_QueueObjs((DG_OBJS*)work->objs);
	GM_GroupObjs( work->objs, work->map );
	work->mdl = def->models ;

	/* オブジェクトを配置 */
	DG_SetPos2(&work->pos,&work->rot);
	DG_GetPos(&world);
	DG_PutObjs(objs);

	/* ターゲットを設定 */
	InitTarget(work, &world);

	DG_SetLightMatrix( work->objs, work->light );

	return 0 ;
}

/* メリケン粉のエフェクト */
void  *NewFlour( int name, int map )
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if(work != NULL){

		work->name = name ;
		work->map  = map ;

		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work ) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
