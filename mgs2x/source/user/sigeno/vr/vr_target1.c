//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_target1.c
	ＶＲステージ移動的
	chara	ＶＲ移動標的[NewVRTarget1] 
	2002/01/30 K.Sigeno
	$Id: vr_target1.c,v 1.1.1.3 2002/11/19 11:49:58 Yoshizawa1 Exp $
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

//#include "break.h"

#include "gameheader.h"
#include "libutl.h"
#include "korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"

#include "vr.h"
#include "../effect/sig_vanim.h"
#include "trg_cmd.h"
#include "inc_trg.h"

extern int BP_FRAMES_PER_SEC();
extern void VR_CheckHomingStatus(HOMING_TRG *hom_trg ,int *sw );

//#define RECURSIVE

/*ポイントタイムの値がこの時、mesg受信まで待機*/
#define		P_TIME_WAIT	(-3)	
#define		P_ACT_INVISIBLE	(1)	




#define		BASE_SIZE		(50.0f)
#define		GUN_SHIFT		(500.0f)
#define		HEAD_OFFSET		(0.0f)
#define		Y_SHIFT			(0.0f)

/* ワーク */

#define CUBE_MDL_NUM (5)
#define	R_PROC_NUM	(8)
typedef	struct _Work {
	GV_ACT_EX	actor ;
	VR_TRG_Work	trg_w ;
	ROUTENAVI	rnavi;	/*ルート誘導*/
	TRGPOINT	trg;	/*移動目標情報*/
	/*吹っ飛び同士討ち用*/
	TARGET			off_trg;
	POWER_TARGET	off_b_power;
	FVECTOR		fromvec ;
	FVECTOR		tovec ;
	FVECTOR		frombuf,tobuf ;
	FVECTOR		shift ;

	float		speed ; 
	float		circle_speed ; 
	float		radius ;	/*公転半径*/
	float		p_acttime_rate ;

	int			r_proc_id[R_PROC_NUM] ;
	char		p_node[R_PROC_NUM] ;
	short		p_route[R_PROC_NUM] ;

	short		kouten_off;	/*公転オフセット*/
	short		route ; /*ルート番号*/

	short		bound_time ;
	short		bound_cnt ;

//	int			debug_cnt ;
} Work ;
enum {
	VR_TRG01_MSG_ROUTE_CHANGE = 1 ,
	VR_TRG01_MSG_ABS_ROUTE_CHANGE = 2 ,
	VR_TRG01_MSG_ROUTE_NODE_CHANGE = 3 ,
	VR_TRG01_MSG_ABS_ROUTE_NODE_CHANGE = 4 ,
};

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void RouteView(ROUTENAVI *) ;

#endif







static inline void AddVec(FVECTOR *res ,FVECTOR *vec1,FVECTOR *vec2){
	res->vx = vec1->vx + vec2->vx ;
	res->vy = vec1->vy + vec2->vy ;
	res->vz = vec1->vz + vec2->vz ;
}
static void  WaitAddVec(FVECTOR *res ,FVECTOR *vec1,FVECTOR *vec2,float rate1)
{
	float rate2 ;
	rate2 = 1.0f- rate1 ;
	res->vx = vec1->vx*rate1 + vec2->vx*rate2 ;
	res->vy = vec1->vy*rate1 + vec2->vy*rate2 ;
	res->vz = vec1->vz*rate1 + vec2->vz*rate2 ;
	res->vw = 0.0f ;
}
static void SetPointAction( Work *work ){
	FVECTOR from,to,nextto,tmp,fromvec,tovec ;
	float len ;
	ROUTENAVI *rnavi ;
	rnavi = &work->rnavi ;
	rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node];

	if(rnavi->pa_time[(int)rnavi->next_node] != P_TIME_WAIT){
		rnavi->p_acttime = DIRECT_TICK(rnavi->pa_time[(int)rnavi->next_node]) ;
		rnavi->p_acttime = (int)((float)rnavi->p_acttime*work->p_acttime_rate) ;
	}else {
		rnavi->p_acttime = P_TIME_WAIT ;
	}
//	rnavi->p_acttime = 0 ;

	rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node];
	rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node];
	rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node];

	if(rnavi->p_acttime == P_TIME_WAIT){
		work->trg_w.status |= TRG_ST_WAIT ;
	}

	DG_COPY_VEC( &(work->fromvec), &(work->tovec) );

	if(rnavi->next_node == 0 ){
		from = rnavi->nodes[ (short)rnavi->n_nodes - 1 ] ;
	}else {
		from = rnavi->nodes[ (short)rnavi->next_node - 1 ] ;
	}
	to = rnavi->nodes[ (short)rnavi->next_node  ] ;
	if(rnavi->next_node == (rnavi->n_nodes-1) ){
		nextto = rnavi->nodes[ 0 ] ;
	}else {
		nextto = rnavi->nodes[ rnavi->next_node +1 ] ;
	}

	GV_SubVec3F( &to , &from, &fromvec ) ;
	fromvec.vw = 0.0f ;
    _sceVu0Normalize( &fromvec, &fromvec ) ;

	GV_SubVec3F( &nextto , &to, &tovec ) ;
	len = GV_VecLen3F(&tovec) ;
	tovec.vw = 0.0f ;
    _sceVu0Normalize( &tovec, &tovec ) ;


	WaitAddVec(&tmp,&fromvec,&tovec,0.5f) ;
    _sceVu0Normalize( &tmp, &tmp ) ;

    _sceVu0ScaleVector( &tmp, &tmp, len ) ;

	DG_COPY_VEC( &(work->tovec), &(tmp) );

#if 0
	{
	    _sceVu0ScaleVector( &fromvec, &fromvec, 1000.0f ) ;
	    _sceVu0ScaleVector( &tovec, &tovec, 1000.0f ) ;

		DG_COPY_VEC( &(work->tobuf), &(tovec) );
		DG_COPY_VEC( &(work->frombuf), &(fromvec) );
	}
#endif
}

static void SetNextnode( Work *work ){
	ENE_SetNextnode( &work->rnavi ) ;
	work->trg.pos = work->rnavi.nodes[ (short)work->rnavi.next_node ] ;
	work->trg.map = work->rnavi.mapbit[ (short)work->rnavi.next_node ] ;
}

static void VrCheckRouteProc(Work *work){
	int i;
	for(i=0 ;i<R_PROC_NUM ;i++){
		if(work->r_proc_id[i] == 0){
			return ;
		}
		if(work->rnavi.c_route == work->p_route[i] ){
			if(work->rnavi.next_node == work->p_node[i]){
				VR_ExecProcName(work->r_proc_id[i],work->trg_w.name) ;
			}
		}
	}
}
static float DirectTrace3D(Work *work,float speed){

	FVECTOR		shift ,*pos ;
	float		rest;
	TRGPOINT *trgp ;
	CONTROL *ctrl ;
	SVECTOR	svec ;

//float		GV_VecLen3F( vec )
//void		GV_SubVec3F( vec1, vec2, vec3 ) 1-2=3

	trgp = &work->trg ;
	ctrl = &work->trg_w.control ;
	pos = &ctrl->mov;
#if 0
	shift.vx = trgp->pos.vx - pos->vx ;
	shift.vy = trgp->pos.vy - pos->vy ;
	shift.vz = trgp->pos.vz - pos->vz ;
#endif


	GV_SubVec3F( &trgp->pos, pos, &shift ) ;
	rest = GV_VecLen3F(&shift) ; 
//printf("speed [%f] SUB X[%f]Y[%f]Z[%f]\n",speed,shift.vx,shift.vy,shift.vz) ;
	if(rest <= speed ){
		/*到達*/
//printf("speed [%f] SUB X[%f]Y[%f]Z[%f] CNT[%d]\n",speed,shift.vx,shift.vy,shift.vz,work->debug_cnt) ;

		trgp->dir = _FVecDir2( &shift ) ;
//		ctrl->step = shift ;
		ctrl->step = DG_ZeroVector ;
		ctrl->mov = trgp->pos ;
	}else {
//	rot->vy = GV_VecDir2( vec ) ;
//	rot->vx = GV_VecDir2X( vec ) ;
		ctrl->turn.vy = trgp->dir = GV_VecDir2( &shift ) ;
		if((shift.vx == 0.0f)&&(shift.vy == 0.0f)){
			if(shift.vz>0.0f){
				shift.vz = +speed ;
			}else {
				shift.vz = -speed ;
			}
		}else if((shift.vx == 0.0f)&&(shift.vz == 0.0f)){
			if(shift.vy>0.0f){
				shift.vy = +speed ;
			}else {
				shift.vy = -speed ;
			}
		}else if((shift.vy == 0.0f)&&(shift.vz == 0.0f)){
			if(shift.vx>0.0f){
				shift.vx = +speed ;
			}else {
				shift.vx = -speed ;
			}
		}else {
			GV_LenVec3F( &shift , &shift, 0.0F, speed ) ;
		}
#if 0
		if((trgp->dir%1024)==0){
			/*鉛直線のみ整数保証*/
			shift.vx = shift.vy = shift.vz = 0.0f ;
			switch((trgp->dir/1024)%4){
				case 0 :
					shift.vz = +speed ;
					break ;
				case 1 :
					shift.vx = speed ;
					break ;
				case 2 :
					shift.vz = -speed ;
					break ;
				case 3 :
					shift.vx = -speed ;
					break ;
				default :
					ASSERT(0) ;
					break ;
			}
		}else {
			GV_LenVec3F( &shift , &shift, 0.0F, speed ) ;
		}
#endif
#if 1
		ctrl->step = shift ;
//printf("VEC X[%f] Y[%f] Z[%f] dir[%d]\n",ctrl->step.vx,ctrl->step.vy,ctrl->step.vz,trgp->dir);
#else
		svec.vx = shift.vx ;
		svec.vy = shift.vy ;
		svec.vz = shift.vz ;
//printf("VEC X[%d] Y[%d] Z[%d] dir[%d]\n",svec.vx,svec.vy,svec.vz,trgp->dir);
		ctrl->step.vx = svec.vx ;
		ctrl->step.vy = svec.vy ;
		ctrl->step.vz = svec.vz ;
#endif
	}
//	return (float)((int)(speed-rest)) ;
	return (speed-rest) ;

}

static void MoveRoute(Work *work,float speed){
	float rest ;
	ROUTENAVI *rnavi ;
	rnavi = &work->rnavi ;
	/*移動制御と到達チェック*/

	if(GM_VRStatus & GM_VR_IDLE){
		work->trg_w.control.step = DG_ZeroVector ;
		return ;
	}
	if((rest = DirectTrace3D(work,speed)) >= 0.0f ){
		/*ポイント到達*/
		VrCheckRouteProc(work) ;
		if(rnavi->p_acttime == 0 ){
	/*待機時間無しなのですぐに次ポイントへ*/
			SetNextnode( work ) ;
			SetPointAction( work );
#ifdef RECURSIVE
			MoveRoute(work,rest) ;
#else
			DirectTrace3D(work,rest) ;
#endif
		}else {
	/*待機モードへ*/
//GV_BREAK_CLEAR() ;
			work->trg_w.mode = TRG_ACTION_MODE ;
//GV_BREAK_WRITE_CHECK( void *ptr )
//GV_BREAK_VALUE_CHECK( void *ptr, int value )
//			if(GV_PadData[ 0 ].status & PAD_A){
//				GV_BREAK_VALUE_CHECK( &work->trg_w.mode, TRG_MOVE_MODE ) ;
//			}
		}
	}else {
		/*進行方向*/
		work->trg_w.control.turn.vy = work->trg.dir ;
		work->trg_w.control.turn.vx = 0 ;
	}
#if 0
printf("NOW POS X[%f] TRG X[%f] step X[%f] rest [%f]\n",
work->trg_w.control.mov.vx ,
work->trg.pos.vx,
work->trg_w.control.step.vx ,
rest) ;

#endif
}
//#define	DECAY_RATE (0.980f)
#define	DECAY_RATE (0.990f)


extern float BP_AdjustTick3(float);
#define	DECAY_TIME (DIRECT_TICK(60))
#define	PUNCH_FORCE	(BP_AdjustTick3(100.0f))

static void BoundMove(Work *work){
	if(work->trg_w.status & TRG_ST_DESTROY){

		work->trg_w.control.step.vx = 0 ;
		work->trg_w.control.step.vz = 0 ;

		return ;
	}
	if(work->bound_cnt> work->bound_time ){
printf("KATANA TARGET FLY TIME END!!\n");
#if 0
		work->trg_w.mode = work->trg_w.mode_buf;
		work->trg_w.b_trg.class &= ~TARGET_SKIP ;
#else
		work->trg_w.vital = VR_TARGET01_LIFE_MAX ;
#endif
	}else {
		FVECTOR	tmp ;
		work->trg_w.control.step.vx *= DECAY_RATE ;
		work->trg_w.control.step.vz *= DECAY_RATE ;
//		work->trg_w.control.step.vy -= 0.001f ;
		if(work->trg_w.control.n_touches > 0){
printf("KATANA TARGET HIT WALL!!\n");
			work->trg_w.vital = VR_TARGET01_LIFE_MAX ;
			work->trg_w.finish = VR_TrgHitFunc(work->trg_w.str_work,&work->trg_w.control.mov,&work->trg_w.control.mov,
				NULL,VR_TRG_LEVEL1,VR_TRG_TYPE_KATANA) ;
		}
#if 1
		GV_MatToVec( &work->trg_w.body.objs->world, &tmp ) ;
		GM_PutTarget( &work->off_trg ) ;
		GM_MoveTarget( &work->off_trg, &tmp) ;
//		work->trg_w.control.turn.vy += 300 ;
//		work->trg_w.control.turn.vy &=4095 ;
#endif
	}

	if(work->trg_w.vital >= VR_TARGET01_LIFE_MAX){
//カタナ的 吹っ飛び後破壊音 ＯＫ
#if 1
		VR_TRG_Clash_SE(&work->trg_w.control.mov,work->trg_w.type) ;
#else
		GM_SeSetMode(SD_A_V_CLASHA,&work->trg_w.control.mov,GM_SEMODE_BOMB) ;
		if(work->trg_w.type & VR_TARGET_TYPE_NG){
			GM_SeSetMode(SD_A_V_HITNG1,&work->trg_w.control.mov,GM_SEMODE_BOMB) ;
		}else {
			GM_SeSetMode(SD_A_V_HITOK1,&work->trg_w.control.mov,GM_SEMODE_BOMB) ;
		}
#endif
	}
}

static void ActionSeq(Work *work)
{
	if(work->rnavi.p_dir >=0 ) {
		work->trg_w.control.turn.vy = work->rnavi.p_dir ;
		work->trg_w.control.turn.vx = 0 ;
		/*自転的でなければdir_offは無用なので*/
		if(work->trg_w.type & (VR_TARGET_TYPE_DIR|VR_TARGET_TYPE_P_DIR)){
			work->trg_w.dir_off = work->rnavi.p_dir ;
		}
	}
	if(
	(work->rnavi.p_acttime>0)
	&&(work->rnavi.p_acttime != P_TIME_WAIT)
	){
/*待機時間中*/
		if(work->trg_w.test_flag== 0){
//test cut
			work->rnavi.p_acttime--;
		}
	}else if(
	(work->rnavi.p_acttime <= 0)
	&&(work->rnavi.p_acttime != P_TIME_WAIT)
	){
/*待機終了 次のポイントセット*/
//		work->nowpos = work->trg.pos ;
		SetNextnode( work ) ;
		SetPointAction( work );
		work->trg_w.mode = TRG_MOVE_MODE ;
		
#if 1
//再帰処理
		{
			FVECTOR		shift ,*pos ;
			float		rest;
			TRGPOINT *trgp ;
			CONTROL *ctrl ;

			trgp = &work->trg ;
			ctrl = &work->trg_w.control ;
			pos = &ctrl->mov;
			GV_SubVec3F( &trgp->pos, pos, &shift ) ;
			rest = GV_VecLen3F(&shift) ; 
			if(rest<1.0f){
//				到達扱い

#ifdef RECURSIVE
				ActionSeq(work) ;
#else
				work->trg_w.mode = TRG_ACTION_MODE ;
//				SetNextnode( work ) ;
//				SetPointAction( work );
#endif
			}
		}
#endif
	}else if (work->rnavi.p_acttime == P_TIME_WAIT){
	}
}
/********************************/









static void HermiteLerp(Work *work , FVECTOR *res)
{
#if 0
	FVECTOR herpos,line[2] ,line2[2],from, to ;
#else
	FVECTOR from, to ;
#endif
	float  dis, route ;
	ROUTENAVI *rnavi ;
	rnavi = &work->rnavi ;
	if(rnavi->next_node != 0 ){
		from = rnavi->nodes[ (short)rnavi->next_node - 1 ] ;
	}else {
		from = rnavi->nodes[ rnavi->n_nodes - 1 ] ;
	}


	to = rnavi->nodes[ (short)rnavi->next_node ] ;

	dis = GV_VecLen3F2( &from, &to ) ;
	route = GV_VecLen3F2( &from, &work->trg_w.control.mov ) ;
	MT_HermiteLerpVec( res, &from,&to,&(work->fromvec),	&(work->tovec),route/dis );


#if 0
/*debug view*/
	{
		line[0] = from ;
		line[0].vw = 1.0f ;
		line[1].vx = from.vx+work->fromvec.vx ;
		line[1].vy = from.vy+work->fromvec.vy ;
		line[1].vz = from.vz+work->fromvec.vz ;
		line[1].vw = 1.0f ;

		NewLineView( line ,1,255,0,0) ;

		line2[0] = to ;
		line2[0].vw = 1.0f ;
		line2[1].vx = to.vx+work->tovec.vx ;
		line2[1].vy = to.vy+work->tovec.vy ;
		line2[1].vz = to.vz+work->tovec.vz ;
		line2[1].vw = 1.0f ;

		NewLineView( line2 ,1,0,0,255) ;

		rgb.vx = 0 ;
		rgb.vy = 255 ;
		rgb.vz = 255 ;
			
		line[0] = to ;
		line[0].vw = 1.0f ;
		line[1].vx = to.vx+work->tobuf.vx ;
		line[1].vy = to.vy+work->tobuf.vy ;
		line[1].vz = to.vz+work->tobuf.vz ;
		line[1].vw = 1.0f ;

		NewLineView( line ,1,0,255,255) ;

		line[0] = from ;
		line[0].vw = 1.0f ;
		line[1].vx = from.vx+work->frombuf.vx ;
		line[1].vy = from.vy+work->frombuf.vy ;
		line[1].vz = from.vz+work->frombuf.vz ;
		line[1].vw = 1.0f ;
		NewLineView( line ,1,0,255,255) ;
	}
#endif
}





//#include "inc_trg.c"



static	void	VRtrg01CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code ;
    n_msg = work->trg_w.control.n_msg ;
	msg = work->trg_w.control.msg ;

	if(work->trg_w.mode == TRG_BOUND_MODE ){
		return ;
	}

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case VR_TRG01_MSG_ROUTE_CHANGE :

				work->route = msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				ENE_InitRouteNavi( &work->rnavi, work->route, 0 );
				break;
			case VR_TRG01_MSG_ROUTE_NODE_CHANGE :
				work->route = msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				ENE_InitRouteNavi( &work->rnavi, work->route, msg->message[ 2 ] );
				break;
			case VR_TRG01_MSG_ABS_ROUTE_CHANGE :
				work->route = msg->message[ 1 ] ;

				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				ENE_InitRouteNavi( &work->rnavi, work->route, 0 );
				work->trg.pos = work->rnavi.nodes[ (short)work->rnavi.next_node ] ;
				work->trg.map = work->rnavi.mapbit[ (short)work->rnavi.next_node ] ;
				SetPointAction( work );
				work->trg_w.mode = TRG_MOVE_MODE ;
				break;
			case VR_TRG01_MSG_ABS_ROUTE_NODE_CHANGE :
				work->route = msg->message[ 1 ] ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->route += GM_RouteOffset ; 
				}
				ENE_InitRouteNavi( &work->rnavi, work->route, msg->message[ 2 ] );
				work->trg.pos = work->rnavi.nodes[ (short)work->rnavi.next_node ] ;
				work->trg.map = work->rnavi.mapbit[ (short)work->rnavi.next_node ] ;
				SetPointAction( work );
				work->trg_w.mode = TRG_MOVE_MODE ;
				break;

		}
		msg++ ;
	}
}

static void Act(Work *work)
{
	FVECTOR		t_pos ;
	SVECTOR		rgb ;
	int dir ;

	if(!(work->trg_w.type & VR_TARGET_TYPE_WAIT)){
		VR_trglifecheck(&work->trg_w,&work->actor) ;
	}

	if((work->trg_w.type & VR_TARGET_TYPE_NO_GHOST)&&(work->trg_w.status & TRG_ST_DESTROY)){
		return ;
	}
	{
	/*巡回中*/
		if(
		(work->trg_w.type & VR_TARGET_TYPE_WAIT)
		){
		}else {
			if(work->trg_w.type & VR_TARGET_TYPE_MINE){
				if(!(GM_VRStatus & GM_VR_IDLE)&&(work->trg_w.mine_act == NULL)){
					extern void *NewWaterMineLamp( FMATRIX *world );
					GV_SetActorChild( work,work->trg_w.mine_act = 
						NewWaterMineLamp( &work->trg_w.body.objs->world ) );
				}
			}
//printf("MY NAME IS [%d]\n",work->trg_w.name) ;
			switch (work->trg_w.mode){
				case TRG_MOVE_MODE :
//printf("MOVE MODE \n") ;

//rgb.vx = 0 ;rgb.vy = 0 ;rgb.vz = 127 ; PosBox(&work->trg_w.control.mov,1000.0f ,&rgb );
					if((work->trg_w.amb_cnt>0)&&(work->trg_w.type & VR_TARGET_TYPE_KATANA)){
						work->trg_w.control.step = DG_ZeroVector ;
					}else {
						MoveRoute(work,work->speed) ;
					}
					break;
				case TRG_ACTION_MODE :
//	work->debug_cnt = 0;

//printf("ACT MODE \n") ;
//rgb.vx = 127 ;rgb.vy = 0 ;rgb.vz = 0 ; PosBox(&work->trg_w.control.mov,1000.0f ,&rgb );

					work->trg_w.control.step = DG_ZeroVector ;
					ActionSeq(work);
					break;
				case TRG_BOUND_MODE :
					BoundMove(work) ;
					break;
			}
		}
	}
	work->trg_w.control.interp = 8 ;

	if(work->trg_w.type & VR_TARGET_TYPE_WAIT){
		/*待機中はメッセージチェックのみ*/
		work->trg_w.control.n_msg = GV_ReceiveMessage( work->trg_w.name, &work->trg_w.control.msg );
	}else {
		GM_ActControl(&work->trg_w.control) ;
	}
	VR_CheckMessage(&work->trg_w);	/*共通mesg*/
	if(!(work->trg_w.type & VR_TARGET_TYPE_WAIT)){
		VRtrg01CheckMessage( work ) ;	/*専用mesg*/
		VR_CheckWakeEffect(&work->trg_w) ;
		if(work->trg_w.type & VR_TARGET_TYPE_LERP){
			HermiteLerp(work , &t_pos) ;
		}else if(work->trg_w.type & VR_TARGET_TYPE_CIRCLE){
			t_pos = work->trg_w.control.mov ;
			dir = work->trg_w.cnt*work->circle_speed + work->kouten_off ;
			dir &= 4095 ;
			t_pos.vx -= (work->radius)*_RcosF( dir ) ;
			t_pos.vz += (work->radius)*_RsinF( dir ) ;
		}else {
			t_pos = work->trg_w.control.mov ;
		}
/*絶対座標シフト*/
		if(work->trg_w.type &VR_TARGET_TYPE_ABSSHIFT){
			AddVec(&t_pos ,&t_pos,&work->shift ) ;
			VR_movetrg(&work->trg_w,&t_pos) ;
/*相対座標シフト*/
		}else {
			DG_SetPos2( &t_pos, &work->trg_w.control.rot ) ;
			DG_PutVector( &work->shift, &t_pos, 1 ) ;
			VR_movetrg(&work->trg_w,&t_pos) ;
		}
		if(work->trg_w.amb_cnt>0){
			if(!(GM_VRStatus & GM_VR_IDLE)){
				work->trg_w.amb_cnt--;
			}
			if((work->trg_w.amb_cnt&3)>1){
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

		if((!(GM_VRStatus & GM_VR_IDLE))&&(work->trg_w.mode != TRG_BOUND_MODE )){
			work->trg_w.cnt++;
		}
		if(work->trg_w.mode == TRG_BOUND_MODE ){
			work->bound_cnt++ ;
		}

		if(work->trg_w.visi_cnt>0){
			if(!(GM_VRStatus & GM_VR_IDLE)){
				work->trg_w.visi_cnt-- ;
			}
		}

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


static	void	ChildTargCallBack_Mine( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	work = ( Work * )ptr ;

	def->damaged &= ~TARGET_POWER ;
	def->weapon_type &= ~( off->weapon_type ) ;

//	if(( GM_PlayerStatus & PLAYER_INVINCIBLE) &&( GM_PlayerStatus & PLAYER_FORCE) ){
	if(( GM_PlayerStatus & PLAYER_INVINCIBLE)||(def->side != PLAYER_SIDE)){
	}else {
		work->trg_w.vital= 255 ;
	}
}

static void VR_SetTargetMine(Work *work)
{
	int flag,map;
	FVECTOR	b_size = { 400.0f, 400.0f, 400.0f ,1.0f} ;

	work->trg_w.mine_trg = GV_Malloc( sizeof(TARGET) ) ;

	ASSERT(work->trg_w.mine_trg != NULL) ;

	/*ターゲット設定*/
	flag =(TARGET_OFFENSE|TARGET_NO_LOCKON|TARGET_POWER);
	map = work->trg_w.control.map;
	GM_SetTarget( work->trg_w.mine_trg, (flag),map,PLAYER_SIDE, &b_size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( work->trg_w.mine_trg, 0 ) ;
//	GM_SetPowerTarget( work->trg_w.mine_trg,&work->off_b_power,POWER_ONCE,0,0,0,&DG_ZeroVector);
    GM_SetPowerTarget( work->trg_w.mine_trg,&work->off_b_power,POWER_ONCE,0,0,0,&DG_ZeroVector);

	GM_SetTargetCallBack( work->trg_w.mine_trg, ChildTargCallBack_Mine, work ) ;


//	GM_SetTarget(&work->off_trg,(TARGET_POWER|TARGET_OFFENSE),0,ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
//	GM_SetTargetWeaponType( &work->off_trg, WP_KICK|WP_NOPLAYER ) ; 
	GM_SetPowerTarget( &work->off_trg,&work->off_b_power,POWER_CONST,255,0,ENE_PUNCH_DMG,&DG_ZeroVector);


//	NewTargetView( work->trg_w.mine_trg, 0, 0, 255 ) ;
}


static int GetResources(Work *work, int name, int where)
{
//	DG_DEF  *def ;
	int i ,node;
//	SVECTOR	range ;
//	float	length ;
	work->trg_w.name = name ;
	work->trg_w.vital = 0 ;
	work->trg_w.cnt = 0 ;

	if ( GCL_GetOption( 't' ) != NULL ){
		work->trg_w.test_flag = GCL_GetNextInt() ;
	}else {
		work->trg_w.test_flag = 0 ;
	}

	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->trg_w.type = GCL_GetNextInt() ;
	}else {
		work->trg_w.type = 0 ;
	}
	if(work->trg_w.type & VR_TARGET_TYPE_MINE){
		work->trg_w.type |= VR_TARGET_TYPE_BLAST ;
	}

	if(work->trg_w.type & VR_TARGET_TYPE_CIRCLE){
		/*スピードを１フレでの角度に変換*/
//	-kouten	$i:公転速度 $i:初期公転方向 $i:公転半径 \ /*公転移動タイプのみ有効 回転角/秒　360度＝4096 負で逆回転*/
		if ( GCL_GetOption( 'k' ) != NULL ) {
			work->circle_speed = (float) GCL_GetNextInt() ;

         work->circle_speed /= BP_FRAMES_PER_SEC();

			work->kouten_off = 1024+(short)GCL_GetNextInt() ;
			work->radius = (float)GCL_GetNextInt() ;
		}
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

      work->trg_w.jiten /= (float)BP_FRAMES_PER_SEC();

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


/*
MDL_VR_TRG_TRI_01	ノーマル部位
MDL_VR_TRG_CUBE_01	箱
MDL_VR_TRG_OCT_01	8面
MDL_VR_TRG_STR_01	刀
MDL_VR_TRG_TRI_02	ノーマル部位 中心高得点
MDL_VR_TRG_TRI_EXP_01	爆発
MDL_VR_TRG_TRI_TAB_01	NG
*/

	init_vr_trgobj(&work->trg_w,&work->actor) ;

	/*ルート設定*/
	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->route = GCL_GetNextInt() ;
		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			work->route += GM_RouteOffset ; 
		}
		if( GCL_NextStr() != NULL ){
			node = GCL_GetNextInt() ;
		}else {
			node = 0 ;
		}
		ENE_InitRouteNavi( &work->rnavi, work->route, node  );
		SetPointAction( work );
	}else {
		return 0 ;
	}

	if ( GCL_GetOption( 's' ) != NULL ) 
   {
		work->speed = (float) BP_AdjustTick3((float)GCL_GetNextInt());
	}

	/*係数*/
	work->speed *= VR_TRG_SPEED_RATE ;
	/*ルートポイントに配置*/
	{
		FVECTOR	t_pos ;
		work->trg_w.control.mov = work->rnavi.nodes[ node ] ;
		t_pos = work->trg_w.control.mov ;
		if(work->trg_w.type & VR_TARGET_TYPE_CIRCLE){
			int dir ;
			dir = work->trg_w.cnt*work->circle_speed + work->kouten_off ;
			dir &= 4095 ;
			t_pos.vx -= (work->radius)*_RsinF(dir);
			t_pos.vz += (work->radius)*_RcosF(dir);
		}
		work->trg_w.control.rot.vy = work->trg_w.dir_off ;
		GM_ConfigControlMapID( &work->trg_w.control ) ;
		DG_SetPos2( &t_pos, &work->trg_w.control.rot ) ;
		DG_PutObjs( work->trg_w.body.objs );
	}
	work->trg_w.mode = TRG_ACTION_MODE ;
	if(work->trg_w.type & VR_TARGET_TYPE_KATANA){
		FVECTOR	b_size = { 1000.0f, 250.0f, 1000.0f ,1.0f} ;
		VR_SetTargetKatana(&work->trg_w) ;
/*offense */
#if 1
		GM_SetTarget(&work->off_trg,(TARGET_POWER|TARGET_OFFENSE),0,ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( &work->off_trg, WP_KICK|WP_NOPLAYER ) ; 
		GM_SetPowerTarget( &work->off_trg,&work->off_b_power,POWER_CONST,255,0,ENE_PUNCH_DMG,&DG_ZeroVector);
#ifdef DEBUG_MODE
		if(work->trg_w.type & VR_TARGET_DEBUG){
			NewTargetView( &work->off_trg, 0, 0, 255 ) ;
		}
#endif
#endif
/***********/
	}else if(work->trg_w.type & VR_TARGET_TYPE_OCT){
		VR_SetTargetOct(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_KATANA){
		VR_SetTargetKatana(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_CROSS){
		VR_SetTargetCross(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_MINE){
//		VR_SetTargetOct(&work->trg_w) ;
		VR_SetTargetMine(work) ;
#if 0
		{
			extern void *NewWaterMineLamp( FMATRIX *world );
			work->trg_w.mine_act = NULL ;
			GV_SetActorChild( work,work->trg_w.mine_act =  NewWaterMineLamp( &work->trg_w.body.objs->world ) );
		}
#endif
	}else if(work->trg_w.type & VR_TARGET_TYPE_HEX){
		VR_SetTargetHex(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_ONE){
		VR_SetTargetOne(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_ONE_TRI){
		VR_SetTargetOneTri(&work->trg_w) ;
	}else if(work->trg_w.type & VR_TARGET_TYPE_DARK_OCTA){
		VR_SetTargetDark(&work->trg_w) ;
	}else {
		VR_SetTargetTri02(&work->trg_w) ;
	}
	if(!(work->trg_w.type & VR_TARGET_TYPE_NG)){
		VR_AddTarget() ;
	}


	{
		FVECTOR	t_pos ;
		t_pos = work->trg_w.control.mov ;

		if(work->trg_w.type &VR_TARGET_TYPE_ABSSHIFT){
			AddVec(&t_pos ,&t_pos,&work->shift ) ;
			DG_SetPos2(&t_pos,&DG_ZeroSVector) ;
//			DG_GetPos(&work->objs.world) ;
			DG_GetPos( &(work->trg_w.body.objs->world) );
		}else {
			DG_SetPos2( &t_pos, &work->trg_w.control.rot ) ;
			DG_PutVector( &work->shift, &t_pos, 1 ) ;

			DG_SetPos2(&t_pos,&DG_ZeroSVector) ;
//			DG_GetPos(&work->objs.world) ;
			DG_GetPos( &(work->trg_w.body.objs->world) );

		}
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

	if ( GCL_GetOption( 'x' ) != NULL ){
		for(i=0 ;i<R_PROC_NUM ;i++){
			if( GCL_NextStr() != NULL ){
				work->r_proc_id[i] = GCL_GetNextInt();
				work->p_route[i] = GCL_GetNextInt();
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->p_route[i] += GM_RouteOffset ; 
				}
				work->p_node[i] = GCL_GetNextInt();
			}
		}
	}


	if(work->trg_w.type & VR_TARGET_TYPE_WAIT){
		VR_AllTrgSkip(&work->trg_w) ;
		work->trg_w.b_trg.class &= ~TARGET_LOCKON ;
		DG_InvisibleObjs(work->trg_w.body.objs) ;
		if(work->trg_w.hlt_objs != NULL ){
			DG_InvisibleObjs(work->trg_w.hlt_objs) ;
		}
		SET_FLAG( work->trg_w.hom_trg.status, HOMING_SKIP ) ;
	}

	if ( GCL_GetOption( 'q' ) != NULL ){
		work->p_acttime_rate = (float)GCL_GetNextInt()/4096.0f;
	}else {
		work->p_acttime_rate = 1.000f;
	}
	work->trg_w.visi_cnt = 0 ;

	if ( GCL_GetOption( 'F' ) != NULL ){
		work->bound_time = DIRECT_TICK(GCL_GetNextInt()) ;
		work->trg_w.punch_force = (float) GCL_GetNextInt();
	}else {
		work->bound_time = DECAY_TIME ;
		work->trg_w.punch_force = PUNCH_FORCE;
	}
	work->bound_cnt = 0 ;
//	work->debug_cnt = 0;

	return 1;
}
/* 初期化部メイン */
void *NewVRTarget1( name , where )
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
