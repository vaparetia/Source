//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gunspin.c
	chara ガンスピン
	Evmモデルを表示  *NewSigGunSpin
	2001/08/08 K.Sigeno
	$Id: gunspin.c,v 1.1.1.3 2002/11/19 11:49:18 Yoshizawa1 Exp $
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

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	CONTROL		control;
	OBJECT		body ;
	OBJECT		item ; /*持ち物*/
	FMATRIX		lights[2] ;
	FVECTOR		pos;	/*表示座標*/
	int			name ;
	int			bas_mot; /*基本モーション*/
	int			msg_mot; /*一時モーション*/

	short		status ; /*シナリオ指定*/
	short		in_flag ;	/*内部フラグ*/

	int			proc_id	;	/*終了時ぷろっく*/
	int			talk_name ;
} Work ;

#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE (DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)

#define	ITEM_FLAG (DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
//DG_FLAG_IRREACTION 赤外線ゴーグル


/*シナリオ ステータス*/
#define	NO_XZ_MOVE			(0x01) /*モーションの水平移動を無視*/
#define	PAUSE_MOT			(0x08) /*静止*/
#define	INIT_POS			(0x10) /*再生開始時に座標リセット*/
#define	NO_INTERP			(0x20) /*モーション補完無し*/
#define	PRESHADE			(0x40) /*プリシェード使用*/
#define	INVISIBLE_START		(0x80) /*非表示で起動*/
#define	PUT_MOT_THERMAL		(0x0100)	//赤外線ゴーグル反応
#define	PUT_MOT_BOUND		(0x0200)	//デバッグ用表示
#define	MT_ANIM				(0x0400)	//ストリーム

/*内部フラグ*/
#define SEQ_MODE		(0x01) /*シーケンスモード*/
#define ITEM_SET		(0x02) /*アイテム付随*/
#define INIT_POS_ONCE	(0x04) /*終了時に座標初期化一回限り*/

extern int BP_FRAMES_PER_SEC();
#define INTERP   6*(300/BP_FRAMES_PER_SEC())

enum{
	SET_MOTION = 1,
	NEXT_MOTION,
	LAST_MOTION,
	RESET_POS,
	TURN_DIR,
	SEQ_MOTION,
	INVISIBLE,
	VISIBLE,
	CLOSE_DOOR,
	CHANGE_BASE_MOTION
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

static void ChangeMot(Work *work,int mot){
	int interp = INTERP;
	if(work->status & NO_INTERP) interp = 0 ;
	
	if( work->body.m_ctrl->mt3_ctrl->motion_num != mot){
		GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,interp);
	}
	if(work->status & INIT_POS){
		work->control.mov = work->pos ;
		work->control.mov.vy = 
		work->pos.vy + work->body.m_ctrl->height ; 
	}
}
static void ForceChangeMot(Work *work,int mot){
	int interp = INTERP;
	if(work->status & NO_INTERP) interp = 0 ;

	GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,interp);
	if(work->status & INIT_POS){
		work->control.mov = work->pos ;
		work->control.mov.vy = 
		work->pos.vy + work->body.m_ctrl->height ; 
	}
}

static void visiblemdl(work)
Work	*work ;
{
//printf("putmodel visible set!!\n");
	DG_VisibleObjs( work->body.objs ) ;
	if(work->in_flag & ITEM_SET){
		DG_VisibleObjs( work->item.objs ) ;
	}
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
			case SET_MOTION :
				work->msg_mot = msg->message[ 1 ] ;
printf("NOW MOTION %d\n",work->msg_mot);
				work->in_flag &= ~SEQ_MODE ;
				MotWakeUp(work);
				ChangeMot(work,work->msg_mot) ;
				visiblemdl(work);
				break;
			case NEXT_MOTION :
				work->bas_mot++;
				if(work->bas_mot >= n_motion){
					work->bas_mot = 0 ;
				}
printf("NOW MOTION %d\n",work->bas_mot);
				work->in_flag &= ~SEQ_MODE ;
				MotWakeUp(work);
				ChangeMot(work,work->bas_mot) ;
				visiblemdl(work);
				break;
			case LAST_MOTION :
				work->bas_mot--;
				work->in_flag &= ~SEQ_MODE ;
				MotWakeUp(work);

				if(work->bas_mot < 0){
					work->bas_mot = (n_motion-1) ;
				}
printf("NOW MOTION %d\n",work->bas_mot);
				ChangeMot(work,work->bas_mot) ;
				visiblemdl(work);
				break;
			case CHANGE_BASE_MOTION :
				work->bas_mot = msg->message[ 1 ] ;
				break;

			case RESET_POS :
					work->control.mov = work->pos ;
					work->control.mov.vy = 
					work->pos.vy + work->body.m_ctrl->height ; 
				break;
			case TURN_DIR :
					work->control.turn.vy = msg->message[ 1 ] & 4095;
				break;
			case SEQ_MOTION :
				work->msg_mot = msg->message[ 1 ] ;
//				work->bas_mot = msg->message[ 1 ] ;
				work->in_flag |= SEQ_MODE ;
				MotWakeUp(work);
				ForceChangeMot(work,work->msg_mot) ;
				visiblemdl(work);
				break ;
			case INVISIBLE :
				if(work->body.evmobj != NULL){
					work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
				}else {
					DG_InvisibleObjs( work->body.objs ) ;
				}
				if(work->in_flag & ITEM_SET){
					DG_InvisibleObjs( work->item.objs ) ;
				}
				
				break ;
			case VISIBLE :
				if(work->body.evmobj != NULL){
					work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
				}else {
					visiblemdl(work);
				}
				if(work->in_flag & ITEM_SET){
					DG_VisibleObjs( work->item.objs ) ;
				}
				break ;
			case CLOSE_DOOR :
				work->msg_mot = msg->message[ 1 ] ;
				work->in_flag |= SEQ_MODE ;
				work->in_flag |= INIT_POS_ONCE ;
				MotWakeUp(work);
				ChangeMot(work,work->msg_mot) ;
				visiblemdl(work);
				break;
		}
		msg++ ;
	}
}
#endif

static void Act(Work *work)
{
	OBJECT *body ;
	CONTROL	*ctrl ;
	body = &work->body ;
	ctrl = &work->control ;


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

	GM_ActMotion( body ) ;
	work->control.step.vy = work->control.step.vw ; 
	GM_ActControl(&work->control) ;
	DG_GetLightMatrix( &work->control.mov, work->lights );
	GM_ActObject2( body ) ;
	if( work->in_flag & SEQ_MODE ) {
		if(GM_CheckObject_IsEnd( body, 0 ) ) {
			MotSleep(work);
			if(work->in_flag & INIT_POS_ONCE){
				work->in_flag &= ~INIT_POS_ONCE ;
				work->control.mov = work->pos ;
				work->control.mov.vy = 
				work->pos.vy + work->body.m_ctrl->height ; 
			}
			work->in_flag &= ~SEQ_MODE ;
/*2001.01.29*/
			if(work->proc_id != NULL) {
				GM_ExecProc( work->proc_id, NULL );
//				work->proc_id = NULL ;
			}
		}
	}
	if( GM_CheckObject_PlayEnd( body, 0 ) ) {
		{
			/*モーション終了判定*/
			ChangeMot(work,work->bas_mot) ;
		}
	}

	CheckMessage(work);
}
static void Die(Work *work)
{
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
	if(work->status & PRESHADE){
		DG_FreePreshade( work->body.objs);
		if(work->in_flag & ITEM_SET){
			DG_FreePreshade( work->item.objs ) ;
		}
	}
	GM_FreeObject(&(work->body));
	if(work->in_flag & ITEM_SET){
		GM_FreeObject(&(work->item));
	}
	GM_FreeControl( &work->control);
}
static int GetResources(Work *work, int name, int where)
{
    int buf[3],model,motion,evmmodel,evm_sw,item,set_num = -1;
	int body_flag = 0 ;
	FVECTOR		shift ;
	work->name = name ;


	/*ステータス*/
	work->status = GCL_GetOptionValue( 's', 0 ) ;
/*2001.01.26 ステータス一部廃止*/
	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->status |= MT_ANIM ;
		work->talk_name = GCL_GetNextInt() ;
	}

//INVISIBLE_START
	work->in_flag = 0 ;

	GM_InitControl( &work->control, name, where );
	GM_ConfigControlMessageCheck( &work->control ) ;
	GM_ConfigControlTrapCheck( &work->control ) ;

//	GM_ConfigControlHazard( &work->control, 500, 100, 110 ) ;
//	GM_ConfigControlMapCheck( &work->control ) ;


	work->control.seg_flag |= HZX_SEG_NO_PLAYER ;
	work->control.flr_flag |= HZX_FLOOR_NO_PLAYER ;
	
	/*CONTROLフラグ*/
	/*壁チェックなし*/
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
	/*シフト座標*/
	if ( GCL_GetOption( 'a' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &shift ) ;
	}else {
		shift = DG_ZeroVector;
	}

	work->pos = work->control.mov ;
	GM_ConfigControlMapID( &work->control ) ;

	/* モデル */
	if ( GCL_GetOption( 'k' ) == NULL ) {
		model = GV_StrCode("sna_def") ;
	} else {
		model = GCL_GetNextInt() ;
	}

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
		motion = GV_StrCode("locker_cp") ;
	} else {
		motion = GCL_GetNextInt() ;
	}
	/* モーション番号 */
	work->bas_mot = 0 ;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		work->bas_mot = GCL_GetNextInt() ;
    }
	work->msg_mot = 0;

	/* 装備品 */
	item = -1;
	if ( GCL_GetOption( 'i' ) != NULL ) {
		item = GCL_GetNextInt() ;
		set_num = GCL_GetNextInt() ;
    }

	if(work->status & PRESHADE ) body_flag |= OBJECT_FLAG_PRE ;
	else body_flag |= OBJECT_FLAG ;

	/*赤外線*/
	if(work->status & PUT_MOT_THERMAL ) {
		body_flag |= DG_FLAG_IRREACTION ;
	}

	if(evm_sw){
		/*envモデル*/
		GM_InitObject(&(work->body),model,
			(body_flag|DG_FLAG_INVISIBLE));
		GM_ConfigObjectEvm( &(work->body),evmmodel, DG_EVMOBJ_IRREACTION );
	}else {
		GM_InitObject(&(work->body),model,body_flag );
	}
	if(item != -1 ){
		work->in_flag |= ITEM_SET ;
		GM_InitObject(&(work->item),item,ITEM_FLAG);
		GM_ConfigObjectRoot(&work->item,&work->body,set_num );
		if(work->status & PRESHADE){
		    DG_MakePreshade( work->item.objs, GM_GetMap( where )->light ) ;
		}else {
			GM_ConfigObjectLight(&(work->item),work->lights) ;
		}
	}

	if(work->status & INVISIBLE_START) {
		if(work->body.evmobj != NULL){
				work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
		}else {
				DG_InvisibleObjs( work->body.objs ) ;
		}
		if(work->in_flag & ITEM_SET){
			DG_InvisibleObjs( work->item.objs ) ;
		}
	}
	/*モーション終了時プロック*/
	if ( GCL_GetOption( 'C' ) != NULL ) {
		work->proc_id = GCL_GetNextInt() ;
	}


	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;
	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	GM_ConfigObjectMotion( &(work->body), 0,
		motion,MT_FLAG_HUMAN2);

ASSERT(work->body.m_ctrl != NULL) ;


	GM_ConfigObjectAction( &work->body, 0, work->bas_mot, 0,0xfffff,0);

	/*初期姿勢にする*/
	GM_ActMotion( &work->body ) ;

	work->control.mov.vy
	= work->pos.vy + work->body.m_ctrl->height ; 

/*スライドドア 開いた状態からスタート用シフト設定*/
	work->control.mov.vx += shift.vx ;
	work->control.mov.vy += shift.vy ;
	work->control.mov.vz += shift.vz ;

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );
    GM_GroupObjs( work->body.objs, where ) ;

    /* プリシェード */
	if(work->status & PRESHADE){
	    DG_MakePreshade( work->body.objs, GM_GetMap( where )->light ) ;
	}else {
		GM_ConfigObjectLight(&(work->body),work->lights) ;
	}
	if(item != -1 ){
	    GM_GroupObjs( work->item.objs, where ) ;
		if(work->status & PRESHADE){
		    DG_MakePreshade( work->item.objs, GM_GetMap( where )->light ) ;
		}else {
			GM_ConfigObjectLight(&(work->item),work->lights) ;
		}
	}

	if(work->status & PAUSE_MOT ){
		/*一時停止*/
		MotSleep(work) ;
	}
	return 1;
}
/* 初期化部メイン */
void *NewSigGunSpin( name , where )
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

