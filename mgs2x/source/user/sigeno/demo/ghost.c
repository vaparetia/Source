//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ghost.c
	chara 怨霊
	2002/05/28 K.Sigeno
	$Id: ghost.c,v 1.2 2002/12/26 11:04:52 Yoshizawa1 Exp $
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

#include "../effect/sig_vanim.h"

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	CONTROL		control;
	OBJECT		body ;
	FMATRIX		lights[2] ;
	FVECTOR		pos;	/*表示座標*/
	FVECTOR		vel;
	FVECTOR		start_pos ;
	int			name ;
	int			bas_mot; /*基本モーション*/
	int			msg_mot; /*一時モーション*/

	short		status ; /*シナリオ指定*/
	short		in_flag ;	/*内部フラグ*/

	int			proc_id	;	/*終了時ぷろっく*/
	int			talk_name ;
	LIT_DEF		*lit_def ;
    RADAR_CTRL	rctrl ;
	/*******************/
	int cnt ;
	int	model;
	int	esc_cnt ;
	int	bgm_track ;
	CONTROL *t_ctrl ;

} Work ;

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)

#define	ITEM_FLAG (DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
//DG_FLAG_IRREACTION 赤外線ゴーグル


/*シナリオ ステータス*/
#define	NO_XZ_MOVE			(0x0001) /*モーションの水平移動を無視*/
#define	NO_GRAVITY			(0x0002) /*重力接地無し*/
#define	NO_HAZARD			(0x0004) /*壁アタリ無し*/
#define	PAUSE_MOT			(0x0008) /*静止*/
#define	INIT_POS			(0x0010) /*再生開始時に座標リセット*/
#define	NO_INTERP			(0x0020) /*モーション補完無し*/
#define	PRESHADE			(0x0040) /*プリシェード使用*/
#define	INVISIBLE_START		(0x0080) /*非表示で起動*/
#define	PUT_MOT_THERMAL		(0x0100)	//赤外線ゴーグル反応
#define	PUT_MOT_BOUND		(0x0200)	//デバッグ用表示
#define	MT_ANIM				(0x0400)	//ストリーム口パク
#define	RAD_POINT			(0x0800)	//ストリーム口パク
#define	SEMI_TRANST			(0x1000)	//半透明
#define	NO_MOTION			(0x2000)	//モーション無し
#define	LATTERDRAW			(0x4000)	//後回し描画

/*内部フラグ*/
#define SEQ_MODE		(0x01) /*シーケンスモード*/
#define ITEM_SET		(0x02) /*アイテム付随*/
#define INIT_POS_ONCE	(0x04) /*終了時に座標初期化一回限り*/
#define PROC_CHECK		(0x08) /*proc判定あり*/

#define INTERP   (6*(300/BP_FRAMES_PER_SEC()))

//#define MSG_KILL (12)
enum{
	MOVE_START = 1,
	MSG_GO_HOME,
};
#define STOP		(0x0001)
#define GO_HOME		(0x0002)
//#define ESCAPE_TIME	DIRECT_TICK(180)

#define ESCAPE_TIME	DIRECT_TICK(90)

extern void	PosBox(FVECTOR * ,float , SVECTOR * );
extern float _RsinF( int ) ;


static void MotSleep(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	work->status |= PAUSE_MOT ;

}
static void MotWakeUp(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	work->status &= ~PAUSE_MOT ;
}


#if 1
static	void	CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code ,n_motion ;
    n_msg = work->control.n_msg ;
	msg = work->control.msg ;

	n_motion = work->body.m_ctrl->motion_arc->n_motion ;
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case MOVE_START :
				work->in_flag &= ~STOP ;
				work->in_flag &= ~GO_HOME ;
				break;
			case MSG_GO_HOME :
				work->in_flag |= GO_HOME ;
				break;
		}
		msg++ ;
	}
}
#endif

#define CHASE_LIMIT	(1500.0f)
#define SPEED_MAX	(30.0f)
#define WAVE_H		(125.0f)
#define CNT_SPEED	(10)
#define	LIGHT_MIN	(8000.0f)
static void Act(Work *work)
{
	OBJECT *body ;
	CONTROL	*ctrl ;
	FVECTOR	sub ,vel;
	float len = CHASE_LIMIT;
	int dir_sub ;
	short	pl2g_dir ;

	body = &work->body ;
	ctrl = &work->control ;

//	work->start_pos = work->control.mov ;

	if(work->esc_cnt > 0){
		/*プレイヤから逃げる*/

		GV_SubVec3F( &work->control.mov, &GM_PlayerPosition, &sub ) ;
		pl2g_dir = GV_VecDir2FromTo( &GM_PlayerPosition, &work->control.mov ) ;

		dir_sub = GV_DiffDirS( work->control.turn.vy, pl2g_dir ) ;
//		work->control.turn.vy += dir_sub/32 ;
		work->control.turn.vy += dir_sub/8 ;

		GV_LenVec3F( &sub ,&vel ,0.0f ,0.5f) ;
		work->vel.vx += vel.vx ;
		work->vel.vz += vel.vz;

		work->esc_cnt-- ;
	}else if(work->in_flag & GO_HOME ) {
		/*隠れ家へ帰る*/
		dir_sub = GV_DiffDirS( work->control.turn.vy, 
			GV_VecDir2FromTo( &work->control.mov , &work->start_pos  ) ) ;
		work->control.turn.vy += dir_sub/22 ;
		GV_SubVec3F( &work->start_pos, &work->control.mov, &sub ) ;
		sub.vy = 0.0f ;
		len = GV_VecLen3F( &sub ) ;

		if((CHASE_LIMIT<len)){
			GV_LenVec3F( &sub ,&vel ,0.0f ,0.5f) ;
			work->vel.vx += vel.vx ;
			work->vel.vz += vel.vz;
		}
	}else {
		/*プレイヤライトチェック*/
		if((!(work->in_flag & STOP))&&(PL_GetPlayerWeapon()!= 0)&&(GM_PlayerStatus & PLAYER_HOLD)){
			GV_SubVec3F( &GM_PlayerPosition, &work->control.mov, &sub ) ;
			if(GV_VecLen3F(&sub)< LIGHT_MIN){
//short			GV_VecDir2FromTo( FVECTOR *from, FVECTOR *to )
//int			GV_DiffDirAbs( from, to )
				pl2g_dir = GV_VecDir2FromTo( &GM_PlayerPosition, &work->control.mov ) ;
				if(GV_DiffDirAbs( GM_PlayerControl->turn.vy , pl2g_dir ) < 256){
					if(!(HZX_OnlineHazardCheck(GM_PlayerControl->hzx_id,&work->control.mov,
						&GM_PlayerPosition,HZX_CHK_ALL,0,0 ) )){
							/*escape*/
						//printf("ESCAPE MODE!!\n") ;
						work->esc_cnt = ESCAPE_TIME ;
					}
				}
			}
		} 
		if(work->esc_cnt == 0) {
			/*遺体追跡*/
			dir_sub = GV_DiffDirS( work->control.turn.vy, 
				GV_VecDir2FromTo( &work->control.mov , &GM_PlayerPosition  ) ) ;
			work->control.turn.vy += dir_sub/22 ;
			GV_SubVec3F( &work->t_ctrl->mov, &work->control.mov, &sub ) ;
			sub.vy = 0.0f ;
			len = GV_VecLen3F( &sub ) ;

			if((CHASE_LIMIT<len)&&(!(work->in_flag & STOP))){
				GV_LenVec3F( &sub ,&vel ,0.0f ,0.5f) ;
				work->vel.vx += vel.vx ;
				work->vel.vz += vel.vz;
			}
		}
	}

	/*減速*/
	work->vel.vx *= 0.998 ;
	work->vel.vz *= 0.998 ;

	if(GV_VecLen3F(&work->vel)> SPEED_MAX){
		/*ブレーキ*/
		work->vel.vx *= 0.95 ;
		work->vel.vz *= 0.95 ;
	};

	work->control.step.vx = work->vel.vx;
	work->control.step.vz = work->vel.vz;


#ifdef DEBUG_MODE
	if(work->status & PUT_MOT_BOUND)
	{
extern void PosBox(FVECTOR * ,float ,SVECTOR * );

		FVECTOR pos0,pos1 ;
		DG_SetPos2( &work->control.mov,&DG_ZeroSVector ) ;
		DG_MovePos( &work->body.objs->trans ) ;

		DG_PutVector( &work->body.objs->bound_min, &pos0, 1 ) ;
		DG_PutVector( &work->body.objs->bound_max, &pos1, 1 ) ;
		GM_CurrentMap = GM_CurrentStageMap ;
		NewBoundingBoxView(&pos0,&pos1,127,0,0);
	}
#endif

	//ゾーンの無いところにいるので自分マップチェック不可
	//表示常にONでよい
	work->control.map = GM_PlayerMap ;
//	work->control.map = GM_CurrentStageMap ;

	GM_ActControl(&work->control) ;
	{
		work->control.mov.vy
			= work->pos.vy + work->body.m_ctrl->height + (WAVE_H)*_RsinF( work->cnt ) ;
		work->control.height = work->body.m_ctrl->height ;
	}

	DG_GetLightMatrix( &work->control.mov, work->lights );
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	GM_ActObject(body) ;

#if 0
	if(!(work->status & NO_MOTION )){
		work->control.mov.vy
			= work->pos.vy + work->body.m_ctrl->height ;
		work->control.height = work->body.m_ctrl->height ;
	}
#endif

	if ((work->in_flag & STOP)||
	((work->in_flag & GO_HOME )&&(CHASE_LIMIT >len))){
		/*消す*/
		GM_MixConvFader( work->bgm_track, GM_PAN_CENTER, 0 ) ;
	} else {
		/*鳴らす*/
		GM_BgmFaderMode( work->bgm_track, &(work->control.mov), GM_SEMODE_NORMAL ) ;
	}


	work->cnt += CNT_SPEED;
	work->cnt &= 4095 ;

	CheckMessage(work);
}
static void Die(Work *work)
{
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
	GM_FreeObject(&(work->body));
	GM_FreeControl( &work->control);
}
static int GetResources(Work *work, int name, int where)
{
    int buf[3],model,motion,evmmodel,evm_sw,item,set_num = -1;
	int body_flag = 0 ,evm_flag = 0;
	work->name = name ;

	work->vel = DG_ZeroVector ;
	work->lit_def = GM_GetMap( where )->light ;
	/*ステータス*/
	work->status = GCL_GetOptionValue( 's', 0 ) ;
/*2001.01.26 ステータス一部廃止*/
	work->status |= (NO_HAZARD|NO_GRAVITY) ;

//INVISIBLE_START
	work->in_flag = STOP ;

	work->t_ctrl = NULL ;
	if ( GCL_GetOption( 't' ) != NULL ) {
		work->t_ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
	}
	ASSERT(work->t_ctrl != NULL) ;

	GM_InitControl( &work->control, name, where );
	work->control.interp = 16 ;
	GM_ConfigControlMessageCheck( &work->control ) ;
	GM_ConfigControlTrapCheck( &work->control ) ;

//	work->control.seg_flag |= HZX_SEG_NO_PLAYER ;
//	work->control.flr_flag |= HZX_FLOOR_NO_PLAYER ;

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
	work->start_pos = work->control.mov ;

	work->pos = work->control.mov ;
	GM_ConfigControlMapID( &work->control ) ;

	/* モデル */
	if ( GCL_GetOption( 'k' ) == NULL ) {
		model = GV_StrCode("sna_def") ;
	} else {
		model = GCL_GetNextInt() ;
	}
	work->model = model ;
	/*EVMモデル*/
	if ( GCL_GetOption( 'e' ) == NULL ) {
		evmmodel = -1 ;
		evm_sw = 0;
	} else {
		evmmodel = GCL_GetNextInt() ;
		evm_sw = 1;
	}

	/* モーションファイル */
	if ( GCL_GetOption( 'm' ) == NULL ) {
		motion = 0 ;
		work->status |= NO_MOTION ;
	} else {
		motion = GCL_GetNextInt() ;
	}
	/* モーション番号 */
	work->bas_mot = 0 ;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		work->bas_mot = GCL_GetNextInt() ;
    }
	work->msg_mot = 0;


	body_flag = OBJECT_FLAG ;

	/*強制半透明*/
	if(work->status & SEMI_TRANST ) {
		body_flag |= DG_FLAG_SEMITRANS ;
		evm_flag |= DG_EVMOBJ_SEMITRANS ;
	}

	body_flag |= DG_FLAG_LATTERDRAW ;
	evm_flag |= DG_EVMOBJ_LATTERDRAW ;

	{
		/*envモデル*/
		GM_InitObject(&(work->body),model,(body_flag));

		work->body.objs->flag |= DG_FLAG_INVISIBLE ;

		GM_ConfigObjectEvm( &(work->body),evmmodel, evm_flag );
	}
	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;
//	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	{
		GM_ConfigObjectMotion( &(work->body), 0,
			motion,MT_FLAG_HUMAN2);
		GM_ConfigObjectAction( &work->body, 0, work->bas_mot, 0,0xfffff,0);
		/*初期姿勢にする*/
		GM_ActMotion( &work->body ) ;
	}

	if(work->status & NO_MOTION ){
		work->control.mov.vy
		= work->pos.vy ;
	}else {
		work->control.mov.vy
		= work->pos.vy + work->body.m_ctrl->height ;
	}

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	GM_ActObject2( &work->body ) ;
	GM_ConfigObjectLight(&(work->body),work->lights) ;
	/*一時停止*/
	MotSleep(work) ;
	work->esc_cnt = 0 ;

	/* 幽霊音 */
	if ( (work->bgm_track = GCL_GetOptionValue( 'b', -1 )) < 0  ) {
		work->bgm_track = 0 ;
	} else {
#ifdef _WINDOWS		
		//-------------------------------------------------
		//	ＢＧＭ＿ＳＥの再生トラックを0x10以降に全て面した為
		
		if(work->bgm_track < 0x10){
			work->bgm_track += 0x10;
		}
#endif
		/* 個別ＢＧＭトラックを初期化する */
		GM_MixConvFader( work->bgm_track, 0x20, 0 ) ;
	}


	return 1;
}
void *NewGhost( name , where )
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
