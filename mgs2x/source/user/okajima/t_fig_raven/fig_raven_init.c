//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
	fig_raven_init.c
	管理部分
	2000/02/17  H.TANAKA
	2000/10/18 S.Okajima
	$Id: fig_raven_init.c,v 1.1.1.3 2002/11/19 11:47:44 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
#include	"fig_raven.h"
#include	"../etc/ok_util.h"


/* 壊れる */
static  void  FigRaven_Break(Work *work, TARGET *t)
{
	if( work->hit_count==2 ){
		work->rot_move.vx = work->rot.vx -  500 ;
		work->rot_move.vy = work->rot.vy -  340 ;
		work->rot_move.vz = work->rot.vz + 1100 ;
		work->move.vy = work->pos.vy - 20.0F ;
	}else if( work->hit_count%4==0 ){
		work->rot_move.vx = work->rot.vx - 1200 ;
		work->rot_move.vy = work->rot.vy + 2048 ;
		work->rot_move.vz = work->rot.vz ;
//		work->move.vy = work->pos.vy - 50.0F ;
	}else if( work->hit_count%4==2 ){
		work->rot_move.vx = work->rot.vx + 1200 ;
		work->rot_move.vy = work->rot.vy - 2048 ;
		work->rot_move.vz = work->rot.vz ;
//		work->move.vy = work->pos.vy + 50.0F ;
	}else if( work->hit_count > 3 ){
		work->rot_move.vx = work->rot.vx -	0 ;
		work->rot_move.vy = work->rot.vy - irnd()%2048 + 1024 ;
		work->rot_move.vz = work->rot.vz +	0 ;
	}


	work->move.vx = work->pos.vx + rnd()*500.0f - 250.0f ;
	work->move.vz = work->pos.vz + rnd()*500.0f - 250.0f ;
	if( work->move.vx < work->boundary_min.vx ){
		work->move.vx = work->boundary_min.vx;
	}else if( work->move.vx > work->boundary_max.vx ){
		work->move.vx = work->boundary_max.vx;
	}
	if( work->move.vz < work->boundary_min.vz ){
		work->move.vz = work->boundary_min.vz;
	}else if( work->move.vz > work->boundary_max.vz ){
		work->move.vz = work->boundary_max.vz;
	}

}

/* あたりチェック */
void FigRaven_TargetCallBack(TARGET *off, TARGET *def, void *ptr)
{
	Work *work = (Work *)ptr ;

	if(def->damaged & TARGET_POWER){
		if(def->weapon_type & (WP_BULLET|WP_M92|WP_BLAST|WP_WPNONE)){
			if( work->next_available_count <= 0 ){
				work->next_available_count = STABLE_COUNT;
				work->hit_count++;
				if( work->hit_count>1 ){
					FigRaven_Break(work,def) ;
				}
			}
		}
	}
	def->weapon_type = 0 ;
}

/* ターゲット初期化 */
int  FigRaven_InitTarget(Work *work, FMATRIX *world)
{
	TARGET	  *t ;
	DG_MDL	  *mdl ;
	FVECTOR	 t_size ;
	FVECTOR	 pos ;

	t = &work->target ;
	mdl = work->mdl ;

	t_size.vx = (mdl->ux - mdl->lx) * 0.5F ;
	t_size.vy = (mdl->uy - mdl->ly) * 0.5F ;
	t_size.vz = (mdl->uz - mdl->lz) * 0.5F ;

	pos.vx = 0.0F ;
	pos.vy = 0.0F ;
	pos.vz = 0.0F ;
   pos.vw = 1.0F ; // BP_MATH - fix unitized value

//printf("__________________________________:%x:%x:%x\n",work->map,GM_CurrentStageMap,GM_CurrentMap);

	GM_SetTarget(t,TARGET_DEFENSE | TARGET_ROTATE, work->map, BOTH_SIDE, &t_size, &pos) ;
	GM_SetPowerTarget(t,&work->power,POWER_DECREASE,FIGRAVEN_VITALITY,0,0,&DG_ZeroVector) ;
	GM_SetTargetCallBack(t,FigRaven_TargetCallBack, work) ;
	GM_PutTarget(t) ;
	GM_CurrentMap = work->map;	//act control で やるものの代用
	GM_MoveTarget2(t,&work->objs->world) ;

//	NewTargetView(t, 255, 255, 0) ;

	return 0 ;
}

/* -------------------------------------------------- */
int  FigRaven_InitRaven(Work *work)
{
	DG_DEF   *def ;
	DG_OBJS  *objs ;
	int	  model_id ;

	if( GCL_GetOption( 'm' ) != NULL )
	model_id = GCL_GetNextInt() ;
	else
	{
	printf("there is not option m of flashlight \n") ;
	return 1 ;
	}

	/* ライトのモデル初期化部分 */   
	def = (DG_DEF *)GV_GetCache(GV_CacheID(model_id,'k')) ;
	if(def == NULL)
	{
	printf("not flashlight.kms\n") ;
	return 1 ;
	}
	/* モデルを取得 */
	objs = work->objs = DG_MakeObjs(def,BODY_FLAG, 0) ;
	if(objs == NULL)
	return 1 ;
	DG_QueueObjs((DG_OBJS*)work->objs) ;   /* モデル登録 */
	work->mdl = def->models ;
	/* 位置、回転、光源初期化 */
	DG_SetPos2(&work->pos,&work->rot) ;
	DG_PutObjs(objs) ;
	work->light[0] = work->light[1] = DG_UnitMatrix ;
	DG_SetLightMatrix(objs,work->light) ;

	work->rot_move.vx = work->rot.vx ;
	work->rot_move.vy = work->rot.vy ;
	work->rot_move.vz = work->rot.vz ;
	work->move.vx = work->pos.vx ;
	work->move.vy = work->pos.vy ;
	work->move.vz = work->pos.vz ;

	work->world = objs->world ;
		/* ターゲット作成 */
	if(FigRaven_InitTarget(work,&objs->world)) return 1 ;

	work->hit_count=0;
	work->next_available_count=0;


	/* 超暫定 */
	work->boundary_min.vx = work->pos.vx - 1500.0f;
	work->boundary_min.vy = work->pos.vy;
	work->boundary_min.vz = work->pos.vz - 1500.0f;

	work->boundary_max.vx = work->pos.vx + 500.0f;
	work->boundary_max.vy = work->pos.vy;
	work->boundary_max.vz = work->pos.vz;







	/* 写真撮影時にコントロールをチェックする為だけに必要 */
	GM_InitObject( &work->object, model_id, BODY_FLAG ) ;
	if ( GM_InitControl( (CONTROL *)&work->control, work->name, work->map ) < 0 ) return -1 ;
	/*CONTROLフラグ*/
	work->control.skip_flag = (
		CTRL_SKIP_FLR_CHECK|
		CTRL_SKIP_SEG_CHECK|
		CTRL_SKIP_GET_ADDRESS|
		CTRL_SKIP_NEAR_CHECK|
		CTRL_SKIP_ONLINE_CHECK|
		CTRL_SKIP_TRAP
	);
	DG_COPY_VEC( &work->control.mov, &work->pos );
	GM_ConfigControlObject( (CONTROL *)&work->control, &work->object ) ;
	DG_InvisibleObjs( work->object.objs );

	return 0 ;
}

int  FigRaven_GetOptionValue(Work *work)
{
	int   buf[3] ;
	
	/* ポジションを決定 */
	if(GCL_GetOption('p') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		GV_IVtoFV(buf,(float *)(&work->pos),3) ;
	}else{
		printf("flashlight.c : not position\n") ;
		return -1 ;
	} 

	/* 回転を決定 */
	if( GCL_GetOption('r') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		work->rot.vx = buf[0] ;
		work->rot.vy = buf[1] ;
		work->rot.vz = buf[2] ;
	}else{
		printf("flashlight.c : not rotation\n") ;
		return -1 ;
	}

	return 0;
}
