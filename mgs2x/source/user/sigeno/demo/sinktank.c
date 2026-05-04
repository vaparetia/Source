//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sinktank.c
	chara 沈没タンカー
	拡大表示＆モーション付け＆プリシェード
	2001/01/30 K.Sigeno
	$Id: sinktank.c,v 1.1.1.3 2002/11/19 11:49:28 Yoshizawa1 Exp $
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


/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	CONTROL		control;
	OBJECT		body ;
	FVECTOR		pos;	/*表示座標*/
	int			name ;
	int			bas_mot; /*基本モーション*/

	short		status ; /*シナリオ指定*/
	short		in_flag ;	/*内部フラグ*/

	int			proc_id	;	/*終了時ぷろっく*/
	int			lit ;		/*光源指定*/
	FVECTOR		scale ;
} Work ;

#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE (DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)

#define	ITEM_FLAG (DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
/*シナリオ ステータス*/
#define	NO_XZ_MOVE		(0x01) /*モーションの水平移動を無視*/
#define	NO_GRAVITY		(0x02) /*重力接地無し*/
#define	NO_HAZARD		(0x04) /*壁アタリ無し*/
#define	PAUSE_MOT		(0x08) /*静止*/
#define	INIT_POS		(0x10) /*再生開始時に座標リセット*/
#define	NO_INTERP		(0x20) /*モーション補完無し*/
#define	PRESHADE		(0x40) /*プリシェード使用*/
/*内部フラグ*/
#define SEQ_MODE		(0x01) /*シーケンスモード*/
#define ITEM_SET		(0x02) /*アイテム付随*/

#define INTERP   6*(300/BP_FRAMES_PER_SEC())


enum{
	MSG_PLAY_MOTION = 1,
	MSG_PLAY_MOTION_NUM,
	MSG_VISIBLE,
	MSG_INVISIBLE,
	MSG_MOT_SPEED
};

extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;

static void MotSleep(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	work->status |= PAUSE_MOT ;

}
static void MotWakeUp(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	work->status &= ~PAUSE_MOT ;
}

#if 0
static void ChangeMot(Work *work,int mot){
	int interp = INTERP;
	if(work->status & NO_INTERP) interp = 0 ;
	
	if( work->body.m_ctrl->mt3_ctrl->motion_num != mot){
		GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,interp);
	}
/*init_pos*/
	work->control.mov = work->pos ;
}
#endif
static void ForceChangeMot(Work *work,int mot){
	GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,0);
/*init_pos*/
//	work->control.mov = work->pos ;
}

static	void	CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	float	mot_speed ;
	int n_msg, code ,n_motion ;
    n_msg = work->control.n_msg ;
	msg = work->control.msg ;
	
	n_motion = work->body.m_ctrl->motion_arc->n_motion ;
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case MSG_PLAY_MOTION :
				MotWakeUp(work);
				break;
			case MSG_PLAY_MOTION_NUM :
				MotWakeUp(work);
				ForceChangeMot(work,msg->message[ 1 ]);
				break;
			case MSG_VISIBLE :
				DG_VisibleObjs( work->body.objs ) ;
				break;
			case MSG_INVISIBLE :
				DG_InvisibleObjs( work->body.objs ) ;
				break;
			case MSG_MOT_SPEED :
				mot_speed = (float) msg->message[ 1 ] ;
				mot_speed /= 1000.0f ;
				mot_speed *= (float) TIME_BASE ;
				MT_SetMotionSpeed( work->body.m_ctrl, mot_speed ) ;
				break;
		}
		msg++ ;
	}
}

static void Act(Work *work)
{
	OBJECT *body ;

	body = &work->body ;

#if 0
	if(GV_PadData[ 0 ].status & PAD_U){
		work->scale.vx += 0.05F;
		work->scale.vy += 0.05F;
		work->scale.vz += 0.05F;
	}
	if(GV_PadData[ 0 ].status & PAD_D){
		work->scale.vx -= 0.05F;
		work->scale.vy -= 0.05F;
		work->scale.vz -= 0.05F;
	}
#endif

	GM_ActControl(&work->control) ;
	CheckMessage(work);

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	GM_ActMotion(body) ;

	work->control.mov.vy 
		= work->pos.vy + body->m_ctrl->height * work->scale.vy ; 
	work->control.mov.vx += work->control.step.vx ; 
	work->control.mov.vz += work->control.step.vz ; 

	DG_ScalePos(&work->scale);
	GM_ActObject2(body);

	work->control.mov.vy 
		= work->pos.vy + body->m_ctrl->height * work->scale.vy ; 
	
	if( work->in_flag & SEQ_MODE ) {
//		if(GM_CheckObject_IsEnd( body, 0 ) ) {
		if( GM_CheckObject_PlayEnd( body, 0 ) ) {
			MotSleep(work);
			work->in_flag &= ~SEQ_MODE ;
		}
	}
#if 0
	if( GM_CheckObject_PlayEnd( body, 0 ) ) {
		{
			/*モーション終了判定*/
			ChangeMot(work,work->bas_mot) ;
/*2001.01.29*/
			if(work->proc_id != NULL) GM_ExecProc( work->proc_id, NULL );
		}
	}
#endif
//	CheckMessage(work);
}
static void Die(Work *work)
{
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
	DG_FreePreshade( work->body.objs);
	GM_FreeObject(&(work->body));
	GM_FreeControl( &work->control);
}
static int GetResources(Work *work, int name, int where)
{
    int buf[3],model = 0,motion = 0 ,scale;
	LIT_DEF	*def ;

	work->name = name ;

	/*ステータス*/
//	work->status = GCL_GetOptionValue( 's', 0 ) ;
	work->status = PAUSE_MOT ;
//	work->in_flag = 0 ;
	work->in_flag = SEQ_MODE ;

	GM_InitControl( &work->control, name, where );
	GM_ConfigControlMessageCheck( &work->control ) ;
	work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
	work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;


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
		work->control.mov = DG_ZeroVector;
	}
	work->pos = work->control.mov ;
//	GM_ConfigControlMapID( &work->control ) ;

	/* モデル */
	if ( GCL_GetOption( 'k' ) != NULL ) {
		model = GCL_GetNextInt() ;
	}
	/* モーションファイル */
	if ( GCL_GetOption( 'm' ) != NULL ) {
		motion = GCL_GetNextInt() ;
	}
	/* 倍率 */
	if ( GCL_GetOption( 's' ) != NULL ) {
		scale = GCL_GetNextInt() ;
		work->scale.vx = (float) scale ;
		work->scale.vy = (float) scale ;
		work->scale.vz = (float) scale ;
		work->scale.vw = (float) 1.0F ;
	}

	/* モーション番号 */
	work->bas_mot = 0 ;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		work->bas_mot = GCL_GetNextInt() ;
	}

	GM_InitObject(&(work->body),model,OBJECT_FLAG_PRE);
#if 0
	/*モーション終了時プロック*/
	if ( GCL_GetOption( 'c' ) != NULL ) {
		work->proc_id = GCL_GetNextInt() ;
	}
#endif

	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;

	DG_SetPos2( &work->pos, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );
    GM_GroupObjs( work->body.objs, where ) ;
    /* プリシェード */
	work->lit = GCL_GetOptionValue( 'l', 0 ) ;
	def = (LIT_DEF*)GV_GetCache( GV_CacheID( work->lit, 'l' ) );
	DG_MakePreshade( work->body.objs, def);

	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	GM_ConfigObjectMotion( &(work->body), 0,
		motion,MT_FLAG_HUMAN2);
	GM_ConfigObjectAction( &work->body, 0, work->bas_mot, 0,0xfffff,0);

	/*初期姿勢にする*/
	GM_ActMotion( &work->body ) ;

	/*モーションのtrans座標に配置*/
	work->control.mov.vy
	= work->pos.vy + work->body.m_ctrl->height * work->scale.vy ; 

	work->control.mov.vx += work->control.step.vx ;
	work->control.mov.vz += work->control.step.vz ;

	work->control.step.vx = 0.0f ;
	work->control.step.vz = 0.0f ;

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );
    GM_GroupObjs( work->body.objs, where ) ;

	/*一時停止状態で待機*/
	MotSleep(work) ;

	return 1;
}

/* 初期化部メイン */
void *NewSigSinkTanker( name , where )
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

