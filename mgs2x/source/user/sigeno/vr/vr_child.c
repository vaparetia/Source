//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_child.c
	ＶＲステージ子的
	chara	ＶＲ移動標的[NewVRChild] 
	2002/02/20 K.Sigeno
	$Id: vr_child.c,v 1.1.1.3 2002/11/19 11:49:57 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"

#include "vr.h"
#include "trg_cmd.h"
#include "inc_trg.h"

extern int BP_FRAMES_PER_SEC();
extern void VR_CheckHomingStatus(HOMING_TRG *hom_trg ,int *sw );

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_IRREACTION|DG_FLAG_ONEPIECE)


/*ポイントタイムの値がこの時、mesg受信まで待機*/
#define		P_TIME_WAIT	(-3)	
#define		P_ACT_INVISIBLE	(1)	

//#define		BODY_SIZE_X		(250.0f)
//#define		BODY_SIZE_Y		(250.0f)
//#define		BODY_SIZE_Z		(250.0f)

#define		BODY_SIZE_X		(500.0f)
#define		BODY_SIZE_Y		(500.0f)
#define		BODY_SIZE_Z		(500.0f)
#define		BODY_SIZE		(500.0f)

#define		CORE1_SIZE		(150.0f)
#define		CORE2_SIZE		(250.0f)
#define		CORE3_SIZE		(400.0f)


#define		BASE_SIZE		(50.0f)
#define		GUN_SHIFT		(500.0f)
#define		HEAD_OFFSET		(0.0f)
#if 0
#define		Y_SHIFT			(1000.0f)
#else
#define		Y_SHIFT			(0.0f)
#endif

#define LV1_NUM (1)
#define LV2_NUM (2)
#define LV3_NUM (10)

#define OCT_LV1_NUM	(2)
#define OCT_LV2_NUM (4)
#define OCT_LV3_NUM (4)

#define STR_LV1_NUM (1)
#define STR_LV2_NUM (2)
#define STR_LV3_NUM (2)

#define		CUBE_03_LV1_NUM	(1)
#define		CUBE_03_LV2_NUM	(8)
#define		CUBE_03_LV3_NUM	(1)

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	VR_TRG_Work	trg_w ;
	FMATRIX		*parent ;
	float		speed ; 

	FVECTOR		shift ;

} Work ;
enum {
	MSG_ROUTE_CHANGE = 1
};

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void RouteView(ROUTENAVI *) ;

#endif



//#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
//#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

static inline void AddVec(FVECTOR *res ,FVECTOR *vec1,FVECTOR *vec2){
	res->vx = vec1->vx + vec2->vx ;
	res->vy = vec1->vy + vec2->vy ;
	res->vz = vec1->vz + vec2->vz ;
}

#define	PUNCH_FORCE	(50.0f)
static void Act(Work *work)
{
	FVECTOR		t_pos ;


	if(!(work->trg_w.type & VR_TARGET_TYPE_WAIT)){
		VR_trglifecheck(&work->trg_w,&work->actor) ;
	}
	if((work->trg_w.type & VR_TARGET_TYPE_NO_GHOST)&&(work->trg_w.status & TRG_ST_DESTROY)){
		return ;
	}
	if(!(work->trg_w.type & VR_TARGET_TYPE_WAIT)){
		/*公転*/
		SVECTOR	rot ;
		rot.vx = 0 ;
		rot.vy = ((int)((float)work->trg_w.cnt*work->speed))&4095 ;
		rot.vz = 0 ;
		GV_MatToVec( work->parent, &t_pos ) ;
		DG_SetPos2(&t_pos,&rot);
		DG_PutVector( &work->shift, &t_pos, 1 ) ;
//		DG_COPY_VEC( &(work->trg_w.control.mov), &(t_pos) );
	}else {
		GV_MatToVec( work->parent, &t_pos ) ;
	}
	DG_COPY_VEC( &(work->trg_w.control.mov), &(t_pos) );


	work->trg_w.control.interp = 8 ;
	GM_ActControl(&work->trg_w.control) ;
	VR_CheckMessage(&work->trg_w);
	if(!(work->trg_w.type & VR_TARGET_TYPE_WAIT)){
		VR_CheckWakeEffect(&work->trg_w) ;
	}
	if(!(work->trg_w.type & VR_TARGET_TYPE_WAIT)){
		VR_movetrg(&work->trg_w,&t_pos) ;

		if(work->trg_w.amb_cnt>0){
			work->trg_w.amb_cnt--;
			if((work->trg_w.amb_cnt&3)<2){
				work->trg_w.lights[1].m[ 3 ][ 0 ] = (float) 255 ;
				work->trg_w.lights[1].m[ 3 ][ 1 ] = (float) 255 ;
				work->trg_w.lights[1].m[ 3 ][ 2 ] = (float) 255 ;
			}else {
				work->trg_w.lights[1].m[ 3 ][ 0 ] = (float) 16 ;
				work->trg_w.lights[1].m[ 3 ][ 1 ] = (float) 16 ;
				work->trg_w.lights[1].m[ 3 ][ 2 ] = (float) 16 ;
			}
		}else {
			DG_GetLightMatrix( &t_pos, work->trg_w.lights );
		}
		VR_CheckHomingStatus(&work->trg_w.hom_trg ,&work->trg_w.sight ) ;
	}
	if(work->trg_w.type & VR_TARGET_TYPE_WAIT){
	}else {
		if(!(GM_VRStatus & GM_VR_IDLE)){
			work->trg_w.cnt++;
		}
	}
	if(work->trg_w.visi_cnt>0){
		work->trg_w.visi_cnt--;
	}
}
static void Die(Work *work)
{
	GM_FreeTarget( &work->trg_w.b_trg ) ;
	HZX_FlashTrap( work->trg_w.control.hzx_id, &work->trg_w.control.evt ) ;
	free_vr_trgobj(&work->trg_w) ;
	GM_FreeControl( &work->trg_w.control);

	VR_FreeMemory(&work->trg_w) ;

}

/****************************************************/

/****************************************************/



static int GetResources(Work *work, int name, int where)
{
//	DG_DEF  *def ;
	CONTROL	*pctrl = NULL;
//	int model,flag,tmp;
//	SVECTOR	range ;
//	float	length ;
	work->trg_w.name = name ;
	work->trg_w.vital = 0 ;
	work->trg_w.cnt = 0 ;
	work->trg_w.test_flag = 0 ;

	if ( GCL_GetOption( 'p' ) != NULL ) {
		pctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
	}
	ASSERT(pctrl != NULL) ;
	work->parent = &pctrl->object->objs->world ;
//	VR_SetChildDmg(work) ;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->trg_w.type = GCL_GetNextInt() ;
	}else {
		work->trg_w.type = 0 ;
	}
	if ( GCL_GetOption( 'y' ) != NULL ) {
		work->shift.vx = (float)GCL_GetNextInt() ;
		work->shift.vy = (float)GCL_GetNextInt() ;
		work->shift.vz = (float)GCL_GetNextInt() ;
	}else {
		work->shift = DG_ZeroVector ;
	}

	if ( GCL_GetOption( 'd' ) != NULL ) {
		work->trg_w.dir_off = GCL_GetNextInt() ;
		work->trg_w.jiten = (float)GCL_GetNextInt() ;

      work->trg_w.jiten /= BP_FRAMES_PER_SEC();

   }else {
		work->trg_w.dir_off = 0 ;
		work->trg_w.jiten = 0.0f ;
	}

	work->trg_w.jiten *= VR_TRG_SPEED_RATE ;

	GM_InitControl( &work->trg_w.control, name, where );
	GM_ConfigControlMessageCheck( &work->trg_w.control ) ;
//	work->trg_w.control.interp = 16 ;
	/*CONTROLフラグ*/
#if 1
	work->trg_w.control.skip_flag = 
		(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
		|CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
		|CTRL_SKIP_TRAP	); 
#else
	work->trg_w.control.skip_flag = (CTRL_SKIP_FLR_CHECK|CTRL_SKIP_TRAP	); 
#endif
	GM_ConfigControlHazard( &work->trg_w.control , 250, 250, 240 ) ;


	if ( GCL_GetOption( 's' ) != NULL ) {
		work->speed = (float) GCL_GetNextInt() ;
			/*スピードを１フレでの角度に変換*/

      work->speed /= BP_FRAMES_PER_SEC();
	}
	/*係数*/
	work->speed *= VR_TRG_SPEED_RATE ;

	{
		/*公転*/
		SVECTOR	rot ;
		FVECTOR	t_pos ;
		rot.vx = 0 ;
		rot.vy = ((int)((float)work->trg_w.cnt*work->speed))&4095 ;
		rot.vz = 0 ;
		GV_MatToVec( work->parent, &t_pos ) ;
		DG_SetPos2(&t_pos,&rot);
		DG_PutVector( &work->shift, &t_pos, 1 ) ;
		DG_COPY_VEC( &(work->trg_w.control.mov), &(t_pos) );
	}

	/*基本部位的*/
	init_vr_trgobj(&work->trg_w,&work->actor) ;


	work->trg_w.mode = TRG_ACTION_MODE ;

//	if(work->trg_w.type & VR_TARGET_TYPE_PUNCH){
	if(0){
//		VR_SetTargetPunch(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_OCT){
		VR_SetTargetOct(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_KATANA){
		VR_SetTargetKatana(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_ONE){
		VR_SetTargetOne(&work->trg_w) ;
	}else {
		VR_SetTargetTri02(&work->trg_w) ;
	}

	if(!(work->trg_w.type & VR_TARGET_TYPE_NG)){
		VR_AddTarget() ;
	}



	if ( GCL_GetOption( 'z' ) != NULL ) {
		work->trg_w.hom_shift.vx = (float)GCL_GetNextInt() ;
		work->trg_w.hom_shift.vy = (float)GCL_GetNextInt() ;
		work->trg_w.hom_shift.vz = (float)GCL_GetNextInt() ;
	}else {
		work->trg_w.hom_shift = DG_ZeroVector ;
	}
	if ( GCL_GetOption( 'c' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->trg_w.proc_id  = GCL_GetNextInt();
		}else {
			work->trg_w.proc_id  = 0;
		}
	}


	work->trg_w.visi_cnt = 0;

	if(work->trg_w.type & VR_TARGET_TYPE_WAIT){
		VR_AllTrgSkip(&work->trg_w) ;
		work->trg_w.b_trg.class &= ~TARGET_LOCKON ;
		DG_InvisibleObjs(work->trg_w.body.objs) ;
		if(work->trg_w.hlt_objs != NULL ){
			DG_InvisibleObjs(work->trg_w.hlt_objs) ;
		}
		SET_FLAG( work->trg_w.hom_trg.status, HOMING_SKIP ) ;
	}

	return 1;
}

/* 初期化部メイン */
void *NewVRChild( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
