//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	putmodel.c
	chara プットモーションモデル
	Evmモデルを表示  *NewSigPutModelEvm
	2000/04/07 K.Sigeno
	$Id: putmodel.c,v 1.1.1.3 2002/11/19 11:49:25 Yoshizawa1 Exp $
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

//#include "enemy.h"
//#include "enemy.x"
/* ワーク */
typedef	struct _Work_putmodel { //BP_GENERAL - added type so we can debug on PC/X360
	GV_ACT_EX	actor ;
	CONTROL		control;
	OBJECT		body ;
	OBJECT		item ; /*持ち物*/
	FMATRIX		lights[2] ;
	FVECTOR		pos;	/*表示座標*/
	VAR_CONTROL	vctrl ;	/*振動制御*/
	int			name ;
	int			bas_mot; /*基本モーション*/
	int			msg_mot; /*一時モーション*/

	int			status ; /*シナリオ指定*/
	short		in_flag ;	/*内部フラグ*/

	int			proc_id	;	/*終了時ぷろっく*/
	int			talk_name ;
	LIT_DEF		*lit_def ;
    RADAR_CTRL	rctrl ;
	/*******************/
	int cnt ;
	int	model;
	int test_flag ;
	/*キャラ付随処理*/
	FMATRIX	*p_world ;
	FVECTOR shift ;
	SVECTOR rot ;
} Work ;

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)

#define	ITEM_FLAG (DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
//DG_FLAG_IRREACTION 赤外線ゴーグル


/*シナリオ ステータス*/
#define	NO_XZ_MOVE			(0x00000001) /*モーションの水平移動を無視*/
#define	NO_GRAVITY			(0x00000002) /*重力接地無し*/
#define	NO_HAZARD			(0x00000004) /*壁アタリ無し*/
#define	PAUSE_MOT			(0x00000008) /*静止*/
#define	INIT_POS			(0x00000010) /*再生開始時に座標リセット*/
#define	NO_INTERP			(0x00000020) /*モーション補完無し*/
#define	PRESHADE			(0x00000040) /*プリシェード使用*/
#define	INVISIBLE_START		(0x00000080) /*非表示で起動*/
#define	PUT_MOT_THERMAL		(0x00000100)	//赤外線ゴーグル反応
#define	PUT_MOT_BOUND		(0x00000200)	//デバッグ用表示
#define	MT_ANIM				(0x00000400)	//ストリーム口パク
#define	RAD_POINT			(0x00000800)	//ストリーム口パク
#define	SEMI_TRANST			(0x00001000)	//半透明
#define	SCALE_10			(0x00002000)	//ゴルルゴン
#define	SHADOW				(0x00004000)	//投影影あり
#define	ITEM_ONLY_MIRROR	(0x00008000)	//装備品を鏡面にしか表示しない
#define	PAD_VIBRATION		(0x00010000)	//パッド振動有効

#define	ALWAYS_VISIBLE    (0x00020000)   //BP_CAMERA - force object to always be visible


/*内部フラグ*/
#define SEQ_MODE		(0x01) /*シーケンスモード*/
#define ITEM_SET		(0x02) /*アイテム付随*/
#define INIT_POS_ONCE	(0x04) /*終了時に座標初期化一回限り*/
#define PROC_CHECK		(0x08) /*proc判定あり*/
#define PARENT_WORLD	(0x10) /*親オブジェのワールドに付随*/

extern int BP_FRAMES_PER_SEC();
#define INTERP   6*(300/BP_FRAMES_PER_SEC())



//#define MSG_KILL (12)
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
	CHANGE_BASE_MOTION,
	PROC_CALL_PLAY,
	MSG_KILL,
	CHANGE_POS,
	STEP_INVALID,
	STEP_VALID,
};

extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
extern void GM_MouthAnimation( int , DG_EVMOBJ * ) ;
extern int MatToYRot( FMATRIX * ) ;
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}

	return ( NULL );
}


static void MotSleep(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	work->status |= PAUSE_MOT ;
	if(work->status & PAD_VIBRATION){
		GM_VctrlStopVibration( &work->vctrl ) ;
	}
}
static void MotWakeUp(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	work->status &= ~PAUSE_MOT ;
	if(work->status & PAD_VIBRATION){
		GM_VctrlSetVibration( &work->vctrl, work->body.m_ctrl->mt3_ctrl->motion_num, 0 ) ;
	}
}

static void SetWaistTurn(Work *work){
	if (work->body.m_ctrl->mt3_ctrl[ 0 ].file_header->flag & MT3_FLAG_TURN_FLAG){
		work->control.turn.vy = work->control.rot.vy = MatToYRot( &BODYWORLD( &work->body, HUMAN21_KOSHI) ) ;
	}
}
static void ChangeMot(Work *work,int mot){
	int interp = INTERP;
	if(work->status & NO_INTERP) interp = 0 ;

	if( work->body.m_ctrl->mt3_ctrl->motion_num != mot){
		if(GM_GameStatus & STATE_VR_ONLY){
			/*念のためVRステージのみにしておく*/
			SetWaistTurn(work);
		}

		GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,interp);
		if(work->status & PAD_VIBRATION){
			if(work->status & PAUSE_MOT){
				GM_VctrlStopVibration( &work->vctrl ) ;
			}else {
				GM_VctrlSetVibration( &work->vctrl, work->body.m_ctrl->mt3_ctrl->motion_num, 0 ) ;
			}
		}

	}
	if(work->status & INIT_POS){
		work->control.mov = work->pos ;
		if(work->status & SCALE_10 ) {
			work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height * 10.0f ;
		}else {
			work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height ;
		}
	}
}
static void ForceChangeMot(Work *work,int mot){
	int interp = INTERP;
	if(work->status & NO_INTERP) interp = 0 ;

	if(GM_GameStatus & STATE_VR_ONLY){
		SetWaistTurn(work);
	}



	GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,interp);
	if(work->status & PAD_VIBRATION){
		if(work->status & PAUSE_MOT){
			GM_VctrlStopVibration( &work->vctrl ) ;
		}else {
			GM_VctrlSetVibration( &work->vctrl, work->body.m_ctrl->mt3_ctrl->motion_num, 0 ) ;
		}
	}

	if(work->status & INIT_POS){
		work->control.mov = work->pos ;
		if(work->status & SCALE_10 ) {
			work->control.mov.vy =	work->pos.vy + work->body.m_ctrl->height * 10.0f;
		}else {
			work->control.mov.vy =	work->pos.vy + work->body.m_ctrl->height ;
		}
	}
}

static void visiblemdl(work)
Work	*work ;
{
//printf("putmodel visible set!!\n");

	if(work->body.evmobj != NULL){
		work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	}else {
		DG_VisibleObjs( work->body.objs ) ;
	}
	if(work->in_flag & ITEM_SET){
		DG_VisibleObjs( work->item.objs ) ;
	}
	if(work->status & RAD_POINT){
		work->rctrl.flag |= RADAR_VISIBLE ;
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
					if(work->status & SCALE_10 ) {
						work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height * 10.0f ;
					}else {
						work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height ;
					}
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
			case PROC_CALL_PLAY :
				work->msg_mot = msg->message[ 1 ] ;
				work->in_flag &= ~SEQ_MODE ;
				work->in_flag |= PROC_CHECK ;
				MotWakeUp(work);
				ForceChangeMot(work,work->msg_mot) ;
				visiblemdl(work);
				break ;
			case INVISIBLE :

            //BP_CAMERA - force object to always be visible?
            if( work->status & ALWAYS_VISIBLE )
            {
               break;
            }
            //BP_CAMERA - force object to always be visible?

				if(work->body.evmobj != NULL){
					work->body.evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
				}else {
					DG_InvisibleObjs( work->body.objs ) ;
				}
				if(work->in_flag & ITEM_SET){
					DG_InvisibleObjs( work->item.objs ) ;
				}
				if(work->status & RAD_POINT){
					work->rctrl.flag &= ~RADAR_VISIBLE ;
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
				if(work->status & RAD_POINT){
					work->rctrl.flag |= RADAR_VISIBLE ;
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
			case MSG_KILL :
printf("MSG_KILL RECIEVED!!!!!!\n");
				GV_DestroyActor(work) ;
				break;
			case CHANGE_POS :
printf("MSG_CHANGE_POS RECIEVED!!!!!!\n");
				work->pos.vx = (float) msg->message[ 1 ];
				work->pos.vy = (float) msg->message[ 2 ];
				work->pos.vz = (float) msg->message[ 3 ];
printf("CHANGE_POS X [%d]Y[%d] Z[%d] RECIEVED!!!!!!\n",work->pos.vx,work->pos.vy,work->pos.vz);
				work->control.mov = work->pos ;
				break;
			case STEP_INVALID :
				work->status |= NO_XZ_MOVE ;
				break;
			case STEP_VALID :
				work->status &= ~NO_XZ_MOVE ;
				break;
		}
		msg++ ;
	}
}
#endif
#define TIME_A	(30)
#define TIME_B	(90)
#define TIME_C	(60)

#define NORMAL_MODE			(0x00000000)
#define AMB_MODE			(0x00000001)
#define BUILD_Y				(0x00000002)
#define BUILD_UP			(0x00000004)	/*足から変形開始*/
#define NO_CHECK_SW			(0x00000008)
#define DISAPPEAR_MODE		(0x00000010) /*実体から消えへ デフォルトは無から実体へ*/
#define BODY_MODE			(0x00000020)
#define BUILD_X				(0x00000040)
#define BUILD_ROLL			(0x00000080)
#define BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
#define MOTION_TR			(0x00000200) /*モーション追随*/
#define BUILD_RND			(0x00000400)	/*ノイジー*/

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

	if(
	(work->status & NO_XZ_MOVE)
	||(work->status & PAUSE_MOT )
	){
		work->control.step.vx = 0.0F;
		work->control.step.vz = 0.0F;
	}
	if(work->status & SCALE_10){
		work->control.step.vx *= 10.0F;
		work->control.step.vz *= 10.0F;
	}

	GM_ActControl(&work->control) ;

	if(!(work->status & PRESHADE)){
		DG_GetLightMatrix( &work->control.mov, work->lights );
	}
#if 0
	else {
	    DG_MakePreshade( work->body.objs, work->lit_def ) ;
	}
#endif


	if(work->in_flag & PARENT_WORLD){
		DG_SetPos( work->p_world ) ;
		DG_MovePos( &work->shift ) ;
		DG_RotatePosZYX( &work->rot ) ;
//		DG_RotatePos( &work->rot ) ;
	}else {
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	}

	/*振動制御*/
	if(work->status & PAD_VIBRATION){
		GM_ActVctrl( &work->vctrl ) ;	/* ActMotionの上 */
	}
	GM_ActObject(body) ;
    MT_SetMotionSeTable( work->body.m_ctrl, work->control.map, 1,  (0xffff) ,(0xffff) ) ;
//printf("GM_CurrentMap [%x]\n",GM_CurrentMap) ;
//printf("ctrl Map [%x]\n",work->control.map) ;


	if(work->status & MT_ANIM ){
		GM_MouthAnimation( work->talk_name, work->body.evmobj  ) ;
	}
	if(work->status & NO_GRAVITY) {
		if(work->status & SCALE_10 ) {
			work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height *10.0f ;
		}else {
			work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height ;
		}


	}else {
		if(ctrl->level_found){
			ctrl->step.vy = 0;
			if(work->status & SCALE_10 ) {
				ctrl->mov.vy = ctrl->levels[0]  + work->body.m_ctrl->height *10.0f ;
			}else {
				ctrl->mov.vy = ctrl->levels[0]  + work->body.m_ctrl->height ;
			}

			GM_ConfigControlHzxHeight( ctrl,500.0F, ctrl->levels[0] ) ;
		}else {
			ctrl->step.vy -= 16.0F;
		}
	}

	if(work->status & SCALE_10 ) {
		work->control.height = work->body.m_ctrl->height *10.0f ;
	}else {
		work->control.height = work->body.m_ctrl->height ;
	}
	if( work->in_flag & (SEQ_MODE|PROC_CHECK) ) {
		if(GM_CheckObject_IsEnd( body, 0 ) ) {
			if(work->in_flag & SEQ_MODE){
				MotSleep(work);
			}
			if(work->in_flag & INIT_POS_ONCE){
				work->in_flag &= ~INIT_POS_ONCE ;
				work->control.mov = work->pos ;
				if(work->status & SCALE_10 ) {
					work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height * 10.0f;
				}else {
					work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height ;
				}
			}
/*2001.01.29*/
#ifdef PSX2
			if(work->proc_id != NULL) {
#else
			if(work->proc_id != 0) {
#endif

				GM_ExecProc( work->proc_id, NULL );
//				work->proc_id = NULL ;
			}
			work->in_flag &= ~SEQ_MODE ;
			work->in_flag &= ~PROC_CHECK ;
		}
	}
	if( GM_CheckObject_PlayEnd( body, 0 ) ) {
		{
			/*モーション終了判定*/
			ChangeMot(work,work->bas_mot) ;
		}
	}
#if 0

//組み立て版
//test_flag
//1 落下消え
//2 上昇消え
//3 螺旋落下消え
//4 螺旋上昇消え

//NORMAL_MODE			(0x00000000)
//AMB_MODE			(0x00000001)
//BUILD_Y				(0x00000002)
//BUILD_UP			(0x00000004)	/*足から変形開始*/
//NO_CHECK_SW			(0x00000008)
//DISAPPEAR_MODE		(0x00000010) /*実体から消えへ デフォルトは無から実体へ*/
//BODY_MODE			(0x00000020)
//BUILD_X				(0x00000040)
//BUILD_ROLL			(0x00000080)
//BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
//#define BUILD_RND			(0x00000400)	/*ノイジー*/

	if(work->test_flag){
		int mode=0,cnt=0 ,delay;
		if(work->cnt% (TIME_A+TIME_B+TIME_C+1) == 0){
//15363829 shutu
//15363829
//MDL_BLADE
extern void *NewSigBreakBody(DG_OBJS *,float ,CVECTOR *,int,int,int,int,void *) ;

extern void *NewSigBreakObj3(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,
		int delay,int *sw,FVECTOR *shift,SVECTOR *rot) ;

//		work->test_flag = 2 ;


//		NewVR_Fall_Line( 0,&work->control.mov,1000,1000.0f,180) ;

//			CVECTOR col ; col.r = 127 ;col.g = 127; col.b = 127 ;
			switch (work->test_flag){
//下降
				case 1 :
					mode = BUILD_UP|BUILD_Y|BUILD_REV|DISAPPEAR_MODE ;
					break;
				case 2 :
//上昇下降
					mode = BUILD_Y|DISAPPEAR_MODE;
					break;
				case 3 :
//下降分散
					mode = BUILD_UP|BUILD_REV|DISAPPEAR_MODE;
					break;
				case 4 :
//上昇螺旋
					mode = DISAPPEAR_MODE|BUILD_ROLL ;
					break;
//以下出現
				case 5 :
//下降
					mode = BUILD_UP|BUILD_Y;
					break;
				case 6 :
//上昇
					mode = BUILD_Y|BUILD_REV;
					break;
				case 7 :
					mode = BUILD_UP ;
					break;
				case 8 :
//上昇螺旋
					mode = BUILD_ROLL|BUILD_REV ;
					break;
				case 9 :
					mode = BUILD_RND|MOTION_TR ;
					break;
				case 10 :
					mode = BUILD_UP|BUILD_RND|DISAPPEAR_MODE|MOTION_TR ;
					break;

			}
			mode |= MOTION_TR ;
			NewSigBreakBody(work->body.objs,200.0f, NULL,TIME_A,TIME_B,0,mode,NULL) ;
			if( mode & BUILD_UP){
				delay = TIME_B ;
			}else {
				delay = 0 ;
			}
			NewSigBreakObj3(GV_StrCode("rai_hair_shadow_mt"),&(body->objs->objs[HUMAN21_ATAMA].world),
			mode|NO_CHECK_SW ,200.0f,TIME_A ,delay,NULL,&DG_ZeroVector, &DG_ZeroSVector) ;

			DG_InvisibleObjs( work->body.objs ) ;
			if(work->in_flag & ITEM_SET){
				DG_InvisibleObjs( work->item.objs ) ;
			}
		}
		if(work->test_flag <= 4) cnt = TIME_C/2 ;
		if(work->cnt%(TIME_A+TIME_B+TIME_C+1) == (TIME_A+TIME_B+cnt)){
			DG_VisibleObjs( work->body.objs ) ;
			if(work->in_flag & ITEM_SET){
				DG_VisibleObjs( work->item.objs ) ;
			}
		}
	}
#endif
	work->cnt++;

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
	if(work->in_flag & ITEM_SET){
		extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;
		DG_DisconnectObjs( work->body.objs, work->item.objs ) ;
		GM_FreeObject(&(work->item));
	}
	GM_FreeObject(&(work->body));
	if(work->status & RAD_POINT){
		GM_FreeRadarControl(&work->rctrl);
	}
	GM_FreeControl( &work->control);
}
static int GetResources(Work *work, int name, int where)
{
    int buf[3],model,motion,evmmodel,evm_sw,item,set_num = -1,i;
	int body_flag = 0 ,evm_flag = 0;
	FVECTOR		shift ;
	work->name = name ;

	work->lit_def = GM_GetMap( where )->light ;
	/*ステータス*/
	work->status = GCL_GetOptionValue( 's', 0 ) ;
/*2001.01.26 ステータス一部廃止*/
	work->status |= (NO_HAZARD|NO_GRAVITY) ;

	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->status |= MT_ANIM ;
		work->talk_name = GCL_GetNextInt() ;
	}
	if ( GCL_GetOption( 't' ) != NULL ) {
		work->test_flag = GCL_GetNextInt() ;
	}else {
		work->test_flag = 0 ;
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
	if( work->status & NO_HAZARD) {
		/*壁チェックなし*/
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
	}
	if( work->status & NO_GRAVITY) {
		work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
	}
	/*シナリオリード*/
	/*方向*/
	work->control.turn.vy = GCL_GetOptionValue( 'd', 0 ) ;
	work->control.turn.vx = GCL_GetOptionValue( 'x', 0 ) ;
	work->control.rot.vx = work->control.turn.vx ;
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

	if(work->status & PRESHADE ) body_flag = OBJECT_FLAG_PRE ;
	else body_flag = OBJECT_FLAG ;

	/*投影*/
	if(work->status & SHADOW ) {
		body_flag |= DG_FLAG_SHADOWMAKE ;
	}

	/*赤外線*/
	if(work->status & PUT_MOT_THERMAL ) {
		body_flag |= DG_FLAG_IRREACTION ;
		evm_flag |= DG_EVMOBJ_IRREACTION ;
	}
	/*強制半透明*/
	if(work->status & SEMI_TRANST ) {
		body_flag |= DG_FLAG_SEMITRANS ;
		evm_flag |= DG_EVMOBJ_SEMITRANS ;
	}
	if(evm_sw){
		/*envモデル*/
	/*投影影実験*/
		GM_InitObject(&(work->body),model,(body_flag));
		work->body.objs->flag |= DG_FLAG_INVISIBLE ;
		GM_ConfigObjectEvm( &(work->body),evmmodel, evm_flag );
	}else {
		GM_InitObject(&(work->body),model,body_flag );
	}
	if(item != -1 ){
		extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;

		work->in_flag |= ITEM_SET ;
		GM_InitObject(&(work->item),item,ITEM_FLAG);
		GM_ConfigObjectRoot(&work->item,&work->body,set_num );
		if(work->status & PRESHADE){
		    DG_MakePreshade( work->item.objs, work->lit_def ) ;
		}else {
			GM_ConfigObjectLight(&(work->item),work->lights) ;
		}
		DG_ConnectObjs( work->body.objs, work->item.objs ) ;

		if(work->status & ITEM_ONLY_MIRROR){
			/*アイテムを鏡にだけ映す*/
			/*写りこみのためobjのフラグだけ立てる*/
			for(i=0;i<work->item.objs->n_models;i++){
				work->item.objs->objs[i].flag &= ~DG_FLAG_INVISIBLE ;
			}
			work->item.objs->flag |= DG_FLAG_INVISIBLE ;
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


	if(work->status & SCALE_10 ) {
		work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height *10.0f ;
	}else {
		work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height ;
	}
/*スライドドア 開いた状態からスタート用シフト設定*/
	work->control.mov.vx += shift.vx ;
	work->control.mov.vy += shift.vy ;
	work->control.mov.vz += shift.vz ;

	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	//DG_PutObjs( work->body.objs );
    //GM_GroupObjs( work->body.objs, where ) ;
	GM_ActObject2( &work->body ) ;

    /* プリシェード */
	if(work->status & PRESHADE){
		FMATRIX		matTmp ;
		/* PRESHADEは根元の関節しかみないので、
		   関節０のマトリクスを一時的にコピーして使う */
		DG_COPY_MAT( &matTmp, &work->body.objs->world ) ;
		DG_COPY_MAT( &work->body.objs->world, &work->body.objs->objs[ 0 ].world ) ;
	    DG_MakePreshade( work->body.objs, work->lit_def ) ;
		DG_COPY_MAT( &work->body.objs->world, &matTmp ) ;
	}else {
		GM_ConfigObjectLight(&(work->body),work->lights) ;
	}
	if(item != -1 ){
	    GM_GroupObjs( work->item.objs, where ) ;
		if(work->status & PRESHADE){
		    DG_MakePreshade( work->item.objs, work->lit_def ) ;
		}else {
			GM_ConfigObjectLight(&(work->item),work->lights) ;
		}
	}

	if(work->status & PAUSE_MOT ){
		/*一時停止*/
		MotSleep(work) ;
	}

	if(work->status & RAD_POINT){
		GM_InitRadarControl(&work->rctrl,&work->control.mov,
//			RADAR_VISIBLE|RADAR_SIGHT, work->control.map ) ;
			RADAR_VISIBLE, work->control.map ) ;
//		work->rctrl.angle = DEF_EYE_RANGE*2 ;/* 視野 */
//		work->rctrl.col = RADAR_COLOR_BLUE ;	/* 視野描画色 */
//		work->rctrl.range = 3000 ;		/* 視力 */
		work->rctrl.col = 0 ;	/* 視野描画色 */
	    GM_RadarSetVRange( &work->rctrl, 3000 , -3000 );
	}


	if ( GCL_GetOption( 'w' ) != NULL ) {
		CONTROL *p_ctrl ;
		int part_num ;
//printf("GET SEARCH NAME \n");
		p_ctrl = SearchControl(GCL_GetNextInt());
//printf("GET part_num \n");
		part_num = GCL_GetNextInt();
		if(p_ctrl != NULL){
			work->p_world = &(p_ctrl->object->objs->objs[part_num].world) ;
			GCL_GetIV( GCL_NextStr(), buf ) ;
			vu0_IV0toFV( (IVECTOR *)buf, &work->shift ) ;
			work->rot.vx = GCL_GetNextInt() ;
			work->rot.vy = GCL_GetNextInt() ;
			work->rot.vz = GCL_GetNextInt() ;
			work->in_flag |= PARENT_WORLD ;
		}
	}

	/*振動制御*/
	if(work->status & PAD_VIBRATION){
		GM_InitVarControl( &work->actor, &work->vctrl, motion ) ;
		GM_ConfigVctrlMctrl( &work->vctrl, work->body.m_ctrl ) ;

		if(work->status & PAUSE_MOT){
			GM_VctrlStopVibration( &work->vctrl ) ;
		}else {
			GM_VctrlSetVibration( &work->vctrl, work->body.m_ctrl->mt3_ctrl->motion_num, 0 ) ;
		}
	}


//	NewLensFlr_Game( &work->control.mov, 4000.0f, 127 ) ;
	
   //BP_CAMERA - force snake and stillman to always visible in c4 demo?
   if( (name == 0x001aef2a) || (name == 0x0061ad7e) )
   {
      const char* areaName = GM_GetArea();
      if( strcmp( areaName, "w16a" ) == 0 )
      {
         work->status |= ALWAYS_VISIBLE;
      }
   }
   //BP_CAMERA - force snake and stillman to always visible in c4 demo?


	return 1;
}


static int GetResources2(Work *work, int name, int where ,int dir,FVECTOR *pos ,int model,int motion)
{
    int evmmodel,evm_sw,item ;
	int body_flag ;
	work->name = name ;
	work->lit_def = GM_GetMap( where )->light ;
	work->test_flag = 0 ;

	work->status |= (NO_GRAVITY|NO_HAZARD);
	work->in_flag = 0 ;

	GM_InitControl( &work->control, name, where );
	GM_ConfigControlMessageCheck( &work->control ) ;
//	GM_ConfigControlHazard( &work->control, 500, 100, 110 ) ;
//	GM_ConfigControlMapCheck( &work->control ) ;
//	GM_ConfigControlTrapCheck( &work->control ) ;

	work->control.seg_flag |= HZX_SEG_NO_PLAYER ;
	work->control.flr_flag |= HZX_FLOOR_NO_PLAYER ;

	/*壁チェックなし*/
	work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
	work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;

	/*シナリオリード*/
	/*方向*/
	work->control.turn.vy = dir ;
	work->control.rot.vx = 0 ;
	work->control.rot.vy = work->control.turn.vy ;
	work->control.rot.vz = 0 ;

	/* 座標 */
	work->control.mov = *pos ;
	work->pos = work->control.mov ;
	GM_ConfigControlMapID( &work->control ) ;

	/* モデル */
	/*EVMモデル*/
	evmmodel = -1 ;
	evm_sw = 0;

	/* モーション番号 */
	work->msg_mot = 0;

	/* 装備品 */
	item = -1;
	if(work->status & PRESHADE ) body_flag = OBJECT_FLAG_PRE ;
	else body_flag = OBJECT_FLAG ;
	GM_InitObject(&(work->body),model,body_flag );
	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;
	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	GM_ConfigObjectMotion( &(work->body), 0,
		motion,MT_FLAG_HUMAN2);
ASSERT(work->body.m_ctrl != NULL) ;
	GM_ConfigObjectAction( &work->body, 0, work->bas_mot, 0,0xfffff,0);

	/*初期姿勢にする*/
	GM_ActMotion( &work->body ) ;

	if(work->status & SCALE_10 ) {
		work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height * 10.0f ;
	}else {
		work->control.mov.vy = work->pos.vy + work->body.m_ctrl->height ;
	}
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	DG_PutObjs( work->body.objs );
    GM_GroupObjs( work->body.objs, where ) ;

    /* プリシェード */
	if(work->status & PRESHADE){
	    DG_MakePreshade( work->body.objs, work->lit_def ) ;
	}else {
		GM_ConfigObjectLight(&(work->body),work->lights) ;
	}
	if(item != -1 ){
	}

	if(work->status & PAUSE_MOT ){
		/*一時停止*/
		MotSleep(work) ;
	}
	return 1;
}

/* 初期化部メイン */
void *NewSigPutModelEvm( name , where )
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

void *NewSigPutModel2(int name,int where ,int dir,FVECTOR *pos ,int model,int motion,int mot_num,int status)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->status = status ;
		work->bas_mot = mot_num ;
//		if(!GetResources2( work,name,where )){
		if(!GetResources2(work,name,where ,dir,pos,model,motion)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

