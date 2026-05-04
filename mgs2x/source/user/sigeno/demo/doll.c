//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	doll.c
	モーション芝居  *NewDemoDoll
	2000/07/18 K.Sigeno
	$Id: doll.c,v 1.1.1.3 2002/11/19 11:49:12 Yoshizawa1 Exp $
*/

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

//#include "enemy.h"
//#include "enemy.x"

extern int BP_FRAMES_PER_SEC();

#define		MAX_SEQ	(8)
#define		END_SEQ	(255)
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	OBJECT		body ;
	FMATRIX		lights[2] ;
	CONTROL		control;
	FVECTOR		pos;	/*表示座標*/
	int			name ;
	int			seq_num ; /*進行管理*/

	int			status ;	/*シナリオ指定*/
	int			proc_id	;	/*終了時ぷろっく*/

	u_char		list[MAX_SEQ] ; /*芝居台本*/
} Work ;

#if 1
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE|DG_FLAG_IRREACTION)
#else
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#endif


/*シナリオ ステータス*/
#if 0
#define	NO_XZ_MOVE		(0x01) /*モーションの水平移動を無視*/
#define	NO_GRAVITY		(0x02) /*重力接地無し*/
#define	NO_HAZARD		(0x04) /*壁アタリ無し*/
#define	PAUSE_MOT		(0x08) /*静止*/
#endif
#define INTERP   6*(300/BP_FRAMES_PER_SEC())

#if 0
static void MotSleep(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	work->status |= PAUSE_MOT ;

}
static void MotWakeUp(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	work->status &= ~PAUSE_MOT ;
}
static void ChangeMot(Work *work,int mot){
	if( work->body.m_ctrl->mt3_ctrl->motion_num != mot){
		GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,INTERP);
	}
}
#endif
#define STEP_TEST
static void ForceChangeMot(Work *work,int mot){
	GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,INTERP);
}
static void RotCorrect(Work *work){
	int correct;
	if( ( correct = work->body.m_ctrl->rot_correct ) ){
		work->control.rot.vy += correct;
		work->control.rot.vy &=4095;
		work->control.turn.vy = work->control.rot.vy ;
		work->control.rot.vx = work->control.turn.vx 
			= -work->control.turn.vx ;
		work->body.m_ctrl->rot_correct = 0;
	}
}

static void Act(Work *work)
{
	OBJECT *body ;
	CONTROL	*ctrl ;
	body = &work->body ;
	ctrl = &work->control ;

#ifdef STEP_TEST
	ctrl->step.vy = 0 ;
#endif

	GM_ActControl(&work->control) ;
	DG_GetLightMatrix( &work->control.mov, work->lights );
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	GM_ActObject(body) ;
	work->control.height = work->body.m_ctrl->height ;

#ifdef STEP_TEST
	ctrl->step.vy = work->control.mov.vy  ;
#endif


	work->control.mov.vy 
	= work->pos.vy + work->body.m_ctrl->height ; 

#ifdef STEP_TEST
	ctrl->step.vy = work->control.mov.vy - ctrl->step.vy ;
#endif


	if( GM_CheckObject_PlayEnd( body, 0 ) ) {
		{
			/*モーション終了判定*/
//			ChangeMot(work,work->bas_mot) ;
			work->seq_num ++ ;
			if(
			(work->seq_num >= MAX_SEQ)
			||(work->list[work->seq_num] == END_SEQ)
			){
				if(work->proc_id != NULL)
					GM_ExecProc( work->proc_id, NULL );
				GV_DestroyActor(work) ;
			}else {
				ForceChangeMot(work,work->list[work->seq_num]) ;
			}
		}
	}
//	CheckMessage(work);
	RotCorrect(work);
}
static void Die(Work *work)
{
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
	GM_FreeObject(&(work->body));
	GM_FreeControl( &work->control);
}
static int GetResources(Work *work, int name, int where)
{
    int i,buf[3],model= 0,motion ;

	work->name = name ;

	/*ステータス*/
	work->status = GCL_GetOptionValue( 's', 0 ) ;

	GM_InitControl( &work->control, name, where );
	GM_ConfigControlTrapCheck( &work->control ) ;

//	GM_ConfigControlMessageCheck( &work->control ) ;
//	GM_ConfigControlHazard( &work->control, 500, 100, 110 ) ;
//	GM_ConfigControlMapCheck( &work->control ) ;

//	work->control.seg_flag |= HZX_SEG_NO_PLAYER ;
//	work->control.flr_flag |= HZX_FLOOR_NO_PLAYER ;
	
#if 0
	/*CONTROLフラグ*/
	if( work->status & NO_HAZARD) {
		/*壁チェックなし*/
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
	}
	if( work->status & NO_GRAVITY) {
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
	}
#else
	work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
	work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
#endif

	/*シナリオリード*/
	/*方向*/
	work->control.turn.vy = GCL_GetOptionValue( 'd', 0 ) ;
	work->control.rot.vx = 0 ;
	work->control.rot.vy = work->control.turn.vy ;
	work->control.rot.vz = 0 ;

	/* 座標 */
	if ( GCL_GetOption( 'p' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->control.mov ) ;
	}else {
		return 0;
		work->control.mov = DG_ZeroVector;
	}
	work->pos = work->control.mov ;

	/* モデル */
	if ( GCL_GetOption( 'k' ) != NULL ) {
		model = GCL_GetNextInt() ;
	}else {
//		ASSERT(0) ;
	}
	if ( GCL_GetOption( 'c' ) != NULL ) {
		model = GCL_GetNextInt() ;
	}

	/* モーションファイル */
	if ( GCL_GetOption( 'm' ) == NULL ) {
		motion = GV_StrCode("locker_cp") ;
	} else {
		motion = GCL_GetNextInt() ;
	}

	/*シーケンス設定*/
	if ( GCL_GetOption( 'l' ) == NULL ) {
		return 0;
	}else {	
		for(i=0;i<MAX_SEQ;i++){
			if( GCL_NextStr() != NULL ){
				work->list[i] = (u_char) GCL_GetNextInt() ;
			}else {
				work->list[i] = END_SEQ;
			}
		}
	}
	if ( GCL_GetOption( 'e' ) != NULL ) {
		work->proc_id = GCL_GetNextInt() ;
	}
	
	work->seq_num = 0 ;

	GM_InitObject(&(work->body),model,OBJECT_FLAG );
	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;
	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	GM_ConfigObjectMotion( &(work->body), 0,
		motion,MT_FLAG_HUMAN2);
	GM_ConfigObjectAction( &work->body, 0, work->list[0], 0,0xfffff,0);
	GM_ConfigObjectLight(&(work->body),work->lights) ;

	/*初期姿勢にする*/
	GM_ActMotion( &work->body ) ;

	work->control.mov.vy
	= work->pos.vy + work->body.m_ctrl->height ; 
	GM_ConfigControlMapID( &work->control ) ;

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );

#if 0
	if(work->status & PAUSE_MOT ){
		/*一時停止*/
		MotSleep(work) ;
	}
#endif
	return 1;
}

/* 初期化部メイン */
void *NewDemoDoll( name , where )
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
