//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	corptes.c
	死体

	1999/07/28 Y.Korekado
	$Id: corp.c,v 1.2 2002/12/12 02:25:01 takaki Exp $
	
*/
#endif

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
#include <string.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"define.h"
#include	"corps.h"
#include	"../thing/item.h"
#include	"../../takabe/other/puppetik.h"
#include	"../../kira/face/face.h"

#define	IK_ON	/* IK 始動 */
//#define	DYNAMIC_ON

#define PUT_ITEM (1)
/*-----		-----*/
/* 今の所オルガしかださないので決め打ち */
#define	E_DOGTAG_NAME (9674876)		/* (GV_StrCode( "dogtag_org" )) */

//#define	DEF_ROTTEN_TIME		COUNT_VMODE(60*60*3) 	/* 腐る時間:３分 */
#define	DEF_ROTTEN_TIME		COUNT_VMODE(300)

#define	SHORT_ROTTEN_TIME	COUNT_VMODE(300)
#define DYNAMIC_HEIGHT	(125)
#define DYNAMIC_WIDTH	(300)
#define DYNAMIC_WIDTH2	(200)
#define FLOOR_N			(5)

//#define CORP_MOT_CHECK	(1)	//引き摺りモーションがモデルに合うかチェックする

#define	CORP_MOTION			GV_StrCode("corps")
#define CORP_TRAP_SHADOW	(4088226) 	/*GV_StrCode("corpshdw")*/

#define	MOTION_MAX			10
#define	CHILD_TARGET_NUM	10

static	int	bodynum[] = {
	HUMAN21_ATAMA,			// Lv1
	HUMAN21_MIGI_UDE1,		// Lv2
	HUMAN21_MIGI_UDE2,
	HUMAN21_HIDARI_UDE1,
	HUMAN21_HIDARI_UDE2,
	HUMAN21_MIGI_ASHI1,
	HUMAN21_MIGI_ASHI2,
	HUMAN21_HIDARI_ASHI1,
	HUMAN21_HIDARI_ASHI2,
	HUMAN21_MUNE,			// Lv3
} ;

/* ダウン方法 action/action.h と同じ */
enum {
	DownBack,	/* うつぶせ */
	DownFront,	/* 仰向け */
	DownWall,	/* 壁もたれ */
	DownSpecial	/* 特殊 */
} ;


#define	CORP_STATUS_MALTI	0x00000001	/* マルチテクスチャ */
#define	CORP_STATUS_SLEEP	0x00000002	/* Zzz */
#define	CORP_STATUS_FAINT	0x00000004	/* piyopiyo */
#define	CORP_STATUS_SPLASH	0x00000008	/* 水飛沫 */
#define	CORP_STATUS_OOZE	0x00000010	/* にじみ血有り */
#define	CORP_STATUS_LIVE	0x00000020	/* 生きている */
#define	CORP_STATUS_SHADOW	0x00000040	/* かげ */
#define	CORP_STATUS_NOBLOOD	0x00000080	/* 出血無し */
#define	CORP_STATUS_NOHIT	0x00000100	/* あたり無し */

/*----- ワーク定義 -----*/

typedef	struct	{
	short			level ;		/* ターゲットレベル */
	short			parts_num ;	/* 体の部位番号 */
    TARGET			child_trg ;
    POWER_TARGET	child_pow ;	/* 防御属性 */
} CHILD_TARGETS ;

typedef	struct	_Action{
	CONTROL		*ctrl ;
	OBJECT		*body ;

    TARGET			deftrg ;	/* 防御ターゲット */
    POWER_TARGET	power ;		/* 防御属性 */
	CHILD_TARGETS 	child[ CHILD_TARGET_NUM ] ;
	CAPTURE_TARGET	capture ;	/* 捕まり情報 */
	TARGET			*damtrg ;	/* ダメージを受けたターゲット */
	int				n_damobj ;	/* ダメージを受けたオブジェクト番号 */
	FVECTOR			off_center ;	/* ダメージの発生源 */
	void			*new_ik ;		/* ＩＫ用 */
	int				ik_time ;	/* ＩＫタイマー */
	void			*oozeblood ;
	void			*w ;

	void			( *action )( struct _Action *, int );
	u_int			time ;
	u_int			tmp_time ;
	int				( *CheckDamage)( struct _Action * ) ;	/* ダメージチェック関数 */

	int				status ;
	int				*ene_status ;

	short			down_s ;	/* 倒れた向き（仰向け、うつ伏せ） */
	short			c_motion_num ;	/* 現在のモーション番号 */
	short			act_end ;
	short			padddd ;

	float			old_body_height ;

	ITEM_PROC		item ;
	ITEM_PROC		dogtag_item ;
	int				dogtag_id ;
	int				sw_dogtag ;

	FVECTOR		wakeup_shift[4] ;
	FVECTOR		take_shift[4] ;
} ACTION ;

typedef	void	( *ACTIONMODE )( ACTION *, int ) ;

#define	MAX_PROC	4
enum {
	CORP_PROC_DAMAGE,
} ;

typedef	struct	{
	GV_ACT_EX		actor ;
	CONTROL		control ;
	OBJECT		body ;
	OBJECT		weapon ;
	FMATRIX		lights[2] ;
	RADAR_CTRL	rctrl ;

	int			rotten_count ;
	int			status ;
	int			corp_status ;
	int			new_ik_flag ;

	/* 暫定モーションデータ */
	int			motion_data[ MOTION_MAX ];

	/* アクション */
	ACTION		action ;

	/* 思考 */
	int		think1 ;
	int		think2 ;
	int		think3 ;
	int		count3 ;
	u_int	tmp_time ;
	int		splash ;

	int			oldcorps_name ;
	Z_INTRPT	z_intrpt ;

	int			proc[ MAX_PROC ] ;

	void			*ptr ;
	CORPS_CALLBACK	set ;	/* 初期化関数 */
	CORPS_CALLBACK	die ;	/* ダイ関数 */

	FC_EVM_FACE	*fcanim ;
	void			*d_name ;
	void			*headmark ;
} Work ;

/*-----	----*/
#define	ACT_STATUS_DEATH		0x00000001
#define	ACT_STATUS_IK_DOWN		0x00000002
#define	ACT_STATUS_DAMAGE		0x00000004
#define	ACT_STATUS_IK_PIKU		0x00000008
#define	ACT_STATUS_CAPTURE		0x00000010
#define	ACT_STATUS_WAKEUP		0x00000020
#define	ACT_STATUS_DOWN			0x00000040
#define	ACT_STATUS_TARGET_SKIP	0x00000080
#define	ACT_STATUS_IK_FOOT		0x00000100
#define	ACT_STATUS_IK_HAND		0x00000200
#define	ACT_STATUS_INVISIBLE	0x00000400
#define	ACT_STATUS_CORRECT		0x00000800

#define	ACT_INTERP_DEF		8*5	/* 1/300単位 */
#define	ACT_INTERP_SLOWLY	60*5 /* 1/300単位 */
#define	MOTION_MASK_FULL	0xffffff
#define	FLAG_CLEAR 		0

#define CHILD_TRG_FLAG	(TARGET_ROTATE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER)
#define	ENEMY_PRIO		0x30
/*-----	----*/
enum {
	/* 死体モーション */
	EM_dam_wall_down_p,
	EM_dam_out_pause,
	EM_dead_carry_body_end,
	EM_dead_carry_body_idle,
	EM_dead_carry_body_start_b,
	EM_dead_carry_body_start_f,
	EM_dead_carry_body_walk,
	EM_dead_idle_f,
	EM_dead_carry_leg_end_b,
	EM_dead_carry_leg_end_f,
	EM_dead_carry_leg_idle_b,
	EM_dead_carry_leg_idle_f,
	EM_dead_carry_leg_start_b,
	EM_dead_carry_leg_start_f,
	EM_dead_carry_leg_walk_b,
	EM_dead_carry_leg_walk_f,

	EM_dead_carry_body_put_locker, 
	EM_dead_carry_locker_idle,
	EM_dead_carry_locker_surprise,
	EM_dead_carry_leg_put_locker_f,
	EM_dead_carry_leg_put_locker_b,
} ;
/*-----	----*/
static	void ActCorp( ACTION *, int ) ;
static	void ActCorpWakeup( ACTION *, int ) ;
static	void ActCorpTake( ACTION *, int ) ;
static	void ActCorpMove( ACTION *, int ) ;
static	void ActCorpDown( ACTION *, int ) ;

static	void ActCorpIntoLocker( ACTION *, int ) ;
void ActCorpStandLocker( ACTION *, int ) ;
static	void ActCorpDownLocker( ACTION *, int ) ;

/*-----	----*/
static FVECTOR 	Def_shift[] = {
	/* モーションが始まって１６フレーム目のシフト座標 */
	{ 0.0F, -421.119F, 1192.692F },	/* 仰向け足 */
	{ 26.483F, -393.755F, 940.002F },	/* 仰向け頭 */
	{ 0.0F, -346.867F, 1192.629F },	/* うつぶせ足 */
	{ 45.761F, -317.266F, 946.003F },	/* うつ伏せ頭 */
	/* 持ち上げられてからのシフト座標 */
	{ 0.0F, -382.115F, 422.438F },	/* 仰向け足 */
	{ 0.0F, -113.487F, 291.057F },	/* 仰向け頭 */
	{ 0.0F, -294.030F, 463.501F },	/* うつぶせ足 */
	{ 0.0F, 0.0F, 0.0F },			/* ダミー */
} ;



/*-----	----*/
extern  void *New_Zzz(FMATRIX *world) ;
extern  void *NewPiyori(FMATRIX *world, int *map ) ;
extern void *NewBodySplash2( DG_OBJS *objs, CONTROL *control, int model_id, int *flag ) ;
extern	int		ENE_GCL_GetFVs( FVECTOR * ) ;
extern int SIG_CheckDogTagFlag(int num) ;
extern void *NewCreateDogtag (OBJECT* obj, int model_name, FVECTOR* shift, int* flag);

/*-----	----*/
#if 0 /* XBOXのため */
static inline void SetAction( act, n_layer, data, m_time, mask, interp )
ACTION	*act ;
int		n_layer ;
int		data ;
int		m_time ;
u_long64	mask ;
int		interp ;
#else
static inline void SetAction( ACTION *act, int n_layer, int data, int m_time, u_long64 mask, int interp )
#endif
{
	if ( act->c_motion_num == data ) return ;
	act->c_motion_num = data ;
	GM_ConfigObjectAction( act->body, n_layer, act->c_motion_num, m_time, mask, interp ) ;
}
static	inline	void	SetMode( act, action )
ACTION			*act ;
ACTIONMODE		action ;
{
	act->action = action ;
	act->time = 0 ;
}

/*-----	----*/

static void SetSlopeRotX( ctrl )
CONTROL	*ctrl ;
{
	ctrl->turn.vx = KR_GetGRot( ctrl, 1000.0f ) ;
}
static void ReSetSlopeRotX( ctrl )
CONTROL	*ctrl ;
{
	ctrl->turn.vx = 0 ;
}

static void CorpsMoveTarget( act )
ACTION *act ;
{
	OBJECT		*body ;
	CHILD_TARGETS	*child ;
	int i ;

	body = act->body ;

    GM_MoveTarget( &act->deftrg, &act->ctrl->mov ) ;

	child = &act->child[ 0 ] ;
	for ( i=0; i<CHILD_TARGET_NUM; i++ ) {
	    GM_MoveTarget2( &child->child_trg, &(BODYWORLD( body, child->parts_num )) ) ;
	    child ++ ;
	}
}

static void FloorThrough( ACTION *act )
{
	act->ctrl->step = DG_ZeroVector ;
	if ( act->capture.capture != NULL ) {
		act->capture.capture->flag |= CAPTURE_FREE ;
		act->capture.capture = NULL ;	
		act->capture.flag &= ~CAPTURE_FREE ;
	}
	SetMode( act, ActCorpDown ) ;
}

static	void CorpSetPosition( act, mov )
ACTION	*act ;
FVECTOR	*mov ;
{
    CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov2, *sft ;
    float	     	len ;

	cap = &(act->capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;
	/* 当たりチェック高さはプレイヤーと同じにする */
//	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;
	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	if ( act->capture.flag & CAPTURE_HEAD ) {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				sft = &act->take_shift[1] ;
		    } else {
				sft = &act->take_shift[1] ;
		    }
	} else {
	    if ( act->down_s == DownFront || act->down_s == DownWall ) {
			sft = &act->take_shift[0] ;
	    } else {
			sft = &act->take_shift[2] ;
	    }
	}
	DG_PutVector( sft, mov, 1 ) ;
	if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &mov2, mov,
				     HZX_CHK_ALL, HZX_TYPE_PLAYER, HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( mov, &mov2, &vec ) ;
	} 
}

static	void CorpSetPositionNear( act, mov )
ACTION	*act ;
FVECTOR	*mov ;
{
    CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov2, *sft ;
    float	     	len ;

	cap = &(act->capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;
	/* 当たりチェック高さはプレイヤーと同じにする */
//	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;

	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	if ( act->capture.flag & CAPTURE_HEAD ) {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				sft = &act->wakeup_shift[1] ;
		    } else {
				sft = &act->wakeup_shift[3] ;
		    }
	} else {
	    if ( act->down_s == DownFront || act->down_s == DownWall ) {
			sft = &act->wakeup_shift[0] ;
	    } else {
			sft = &act->wakeup_shift[2] ;
	    }
	}
	DG_PutVector( sft, mov, 1 ) ;
	if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &mov2, mov,
				     HZX_CHK_ALL, HZX_TYPE_PLAYER, HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( mov, &mov2, &vec ) ;
	} 
}

static	int CorpSetStepNear( act, time )
ACTION	*act ;
int time;
{
	FVECTOR	mov, to ;
	CONTROL	*ctrl ;
	
	ctrl = act->ctrl ;
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	CorpSetPositionNear( act, &mov ) ;

	to.vx = GV_NearTimeF( act->ctrl->mov.vx, mov.vx, time ) ;
	to.vz = GV_NearTimeF( act->ctrl->mov.vz, mov.vz, time ) ;

	act->ctrl->step.vx += to.vx - act->ctrl->mov.vx ;
	act->ctrl->step.vz += to.vz - act->ctrl->mov.vz ;

	{
		ctrl->turn.vx = ENE_GetGRotFromPos( &ctrl->mov, &ctrl->rot, 1000.0f, 
					ctrl->hzx_id, HZX_CHK_ALL, ctrl->flr_flag ) ;
	}

	/* 高さ */
	if ( act->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		to.vy = GV_NearTimeF( act->ctrl->mov.vy, mov.vy, time ) ;
		if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &to, &act->ctrl->mov,
					     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
			printf("!!!!!!!!!!!!!!!!!WARNING flor through !!!!!\n" ) ;
			FloorThrough( act ) ;
			return 1 ;
		}
		act->ctrl->mov.vy = to.vy ;
	}

	CorpsMoveTarget( act ) ;
	return 0 ;
}

static	int CorpSetStep( act, time )
ACTION	*act ;
int time;
{
	FVECTOR	mov, to ;
	CONTROL	*ctrl ;
	int		pose ;
	
	ctrl = act->ctrl ;
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	CorpSetPosition( act, &mov ) ;
//printf("snake [%f][%f] corps mov[%f][%f]\n",GM_PlayerPosition.vx, GM_PlayerPosition.vz, mov.vx,mov.vz ) ;
	
	/* 持ち上げた後 */

	pose = 0 ;
	if ( (act->c_motion_num == EM_dead_carry_leg_idle_f) ||
		 (act->c_motion_num == EM_dead_carry_leg_start_f) ||
		 (act->c_motion_num == EM_dead_carry_leg_walk_f) ) {
		pose = 1 ;	/* 階段補正反対 */
	} else if ( (act->c_motion_num == EM_dead_carry_leg_idle_b) ||
		 (act->c_motion_num == EM_dead_carry_leg_walk_b) ) {
		pose = 2 ;	/* 階段補正反対 */
	}

	if ( time >= 0 ) {
		if ( act->c_motion_num == EM_dead_carry_leg_idle_f || 
			act->c_motion_num == EM_dead_carry_leg_walk_f ) {
		    act->ctrl->turn.vy = act->ctrl->rot.vy = 
			act->capture.capture->ctrl->rot.vy + 2048 ;
		} else if (
			act->c_motion_num == EM_dead_carry_body_idle ||
			act->c_motion_num == EM_dead_carry_leg_idle_b ||
			act->c_motion_num == EM_dead_carry_body_walk ||
			act->c_motion_num == EM_dead_carry_leg_walk_b ){
		    act->ctrl->turn.vy = act->ctrl->rot.vy = 
			act->capture.capture->ctrl->rot.vy ;	    
		}
	}

	if ( time < 0 ) time = 0 ;
	to.vx = GV_NearTimeF( act->ctrl->mov.vx, mov.vx, time ) ;
	to.vz = GV_NearTimeF( act->ctrl->mov.vz, mov.vz, time ) ;

	act->ctrl->step.vx += to.vx - act->ctrl->mov.vx ;
	act->ctrl->step.vz += to.vz - act->ctrl->mov.vz ;
	if ( act->ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
#if 1
		float	f, sub ;
		CONTROL	*pl ;
		
		pl = act->capture.capture->ctrl ;
		f = KR_GetGRotFromPos( &ctrl->mov, ctrl->hzx_id, ctrl->hzx_check_type, ctrl->flr_flag ) ;
		if ( (sub = f - pl->levels[0]) > 0.0f ) {
			if ( sub > 450.0f ) sub = 450.0f ;
			switch ( pose ) {
				case 0 :
					mov.vy += (sub / 2) ;
					ctrl->turn.vx = -384 * (sub/450) ;
				break ;
				case 1 :
					mov.vy += (sub / 2) ;
					ctrl->turn.vx = 384 * (sub/450) ;
				break ;
				default :
					mov.vy += (sub ) ;
					ctrl->turn.vx = -384 * (sub/450) * 2 ;
				break ;
			}
//printf("faint move sub+[%f]  vx[%d] pose[%d]\n",sub/2, ctrl->turn.vx, pose );
		} else {
			ctrl->rot.vx = GV_NearTimeF( ctrl->rot.vx, 0, time ) ;
		}
#endif		
		to.vy = GV_NearTimeF( act->ctrl->mov.vy, mov.vy, time ) ;
		if ( HZX_OnlineHazardCheck( act->ctrl->hzx_id, &to, &act->ctrl->mov,
					     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
			printf("!!!!!!!!!!!!!!!!!WARNING flor through !!!!!\n" ) ;
			FloorThrough( act ) ;
			return 1 ;
		}
		act->ctrl->mov.vy = to.vy ;
	}

	CorpsMoveTarget( act ) ;
	return 0 ;
}



static int CorpCaptureCheck( act )
ACTION	*act ;
{
    CAPTURE_TARGET	*cap ;

	cap = &(act->capture) ;
	if ( cap->flag & CAPTURE_FREE ) { /* ひきずられ終わり */
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
		SetMode( act, ActCorpDown ) ;
	    return 1 ;
	}

	if ( cap->flag & CAPTURE_LOCKER ) { /* ロッカー入れられる */
		CorpSetStep( act, 0 ) ;
		cap->capture = NULL ;
		cap->flag &= ~CAPTURE_FREE ;
		act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
		SetMode( act, ActCorpIntoLocker ) ;
	    return 1 ;
	}


	return 0 ;
}

/*-------- ------------------*/
static	void ActCorp( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_DEATH | ACT_STATUS_IK_DOWN ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
printf("corps:down_s = %d \n",act->down_s ) ;
		if ( act->down_s == DownFront ) {
			SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, 0 ) ;
		} else if ( act->down_s == DownWall ) {
			SetAction( act, 0, EM_dam_wall_down_p, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, 0 ) ;
		}
printf("ActCorp:rot_correct = %d \n",act->body->m_ctrl->rot_correct ) ;
		SET_FLAG( act->status, ACT_STATUS_CORRECT ) ;
	    act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
	}
	if ( time == 1 ) {
		CorpsMoveTarget( act ) ;
	}

	if ( act->CheckDamage( act ) ) {
	    return ;
	}
}

static	void ActCorpDamage( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_DEATH|ACT_STATUS_IK_DOWN|ACT_STATUS_DAMAGE ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
	    SET_FLAG( act->status, ACT_STATUS_IK_PIKU ) ;
		act->ctrl->skip_flag &= ~CTRL_SKIP_HZX ;	/* あたりチェック開始 */
	}

	CorpsMoveTarget( act ) ;

	if ( act->CheckDamage( act ) ) {
	    return ;
	}

	if ( time > COUNT_VMODE(12) ) {
	    SetMode( act, ActCorp ) ;
	}
}

static	void ActCorpWakeup( act, time )
ACTION	*act ;
int		time ;
{
	int	left ;

	SET_FLAG( act->status, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_WAKEUP ) ;

    if ( act->capture.flag & CAPTURE_HEAD ) {
		SET_FLAG( act->status, ACT_STATUS_IK_FOOT ) ;
	} else {
		SET_FLAG( act->status, ACT_STATUS_IK_HAND ) ;
	}


	if ( time == 0 ) {
		FVECTOR	vec ;
		int		dir ;

		_sceVu0SubVector(  &vec, &act->capture.capture->ctrl->mov, &act->ctrl->mov ) ;
		dir = _FVecDir2( &vec ) ;

printf("ActCorpWakeup: act->capture.flag [%x] \n",act->capture.flag);

		if ( act->down_s == DownFront ) {
		    if ( act->capture.flag & CAPTURE_HEAD ) {
				SetAction( act, 0, EM_dead_carry_body_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    } else {
				SetAction( act, 0, EM_dead_carry_leg_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		} else if ( act->down_s == DownWall ) {
		    if ( act->capture.flag & CAPTURE_HEAD ) {
				SetAction( act, 0, EM_dead_carry_body_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    } else {
				SetAction( act, 0, EM_dead_carry_leg_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		} else {
		    if ( act->capture.flag & CAPTURE_HEAD ) {
				SetAction( act, 0, EM_dead_carry_body_start_b, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(90) ;
		    } else {
				SetAction( act, 0, EM_dead_carry_leg_start_b, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		}
		act->ctrl->skip_flag &= ~CTRL_SKIP_HZX ;	/* あたりチェック開始 */

		ReSetSlopeRotX( act->ctrl ) ;
	}

	if ( CorpCaptureCheck( act ) ) {
	    return ;
	}

	if ( time > act->tmp_time ) act->ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;

	if ( time < 16 ) {
		if ( CorpSetStepNear( act, 16-time ) ) return ;
	} else {
		left = KR_LeftMotion( &act->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
		if ( CorpSetStep( act, left ) ) return ;
	}


	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SetMode( act, ActCorpTake ) ;
		return ;
	}
}

static	void ActCorpTake( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP ) ;
    if ( act->capture.flag & CAPTURE_HEAD ) {
		SET_FLAG( act->status, ACT_STATUS_IK_FOOT ) ;
	} else {
		SET_FLAG( act->status, ACT_STATUS_IK_HAND ) ;
	}


	if ( time == 0 ) {
		if ( act->capture.flag & CAPTURE_HEAD ) {
		    SetAction( act, 0, EM_dead_carry_body_idle, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				SetAction( act, 0, EM_dead_carry_leg_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				SetAction( act, 0, EM_dead_carry_leg_idle_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
		act->ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;	/* 床の当たりチェックしない */
	}
	if ( CorpCaptureCheck( act ) ) {
	    return ;
	}

	if ( time == 0 ) {
		if ( CorpSetStep( act, -1 ) ) return ;
	} else {
		if ( CorpSetStep( act, 0 ) ) return ;
	}

	if ( act->capture.flag & CAPTURE_MOVE ) {
		SetMode( act, ActCorpMove ) ;
	}
}

static	void ActCorpMove( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP ) ;
    if ( act->capture.flag & CAPTURE_HEAD ) {
		SET_FLAG( act->status, ACT_STATUS_IK_FOOT ) ;
	} else {
		SET_FLAG( act->status, ACT_STATUS_IK_HAND ) ;
	}


	if ( time == 0 ) {
//		SetAction( act, 0, EM_dead_carry_body_walk, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		if ( act->capture.flag & CAPTURE_HEAD ) {
			SetAction( act, 0, EM_dead_carry_body_walk, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				SetAction( act, 0, EM_dead_carry_leg_walk_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				SetAction( act, 0, EM_dead_carry_leg_walk_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
	}

	if ( CorpCaptureCheck( act ) ) {
	    return ;
	}

	if ( CorpSetStep( act, 0 ) ) return ;

	if ( !( act->capture.flag & CAPTURE_MOVE ) ) {
	    SetMode( act, ActCorpTake ) ;
	}
}

static	int FrontCheck( act )
ACTION	*act ;
{
	FMATRIX	mat, *w ;
	static FVECTOR Front = { 0.0, 0.0, 100.0 } ;

	w = &act->body->objs->objs[0].world ;
	DG_SetPos( w ) ;
	DG_MovePos( &Front ) ;
	DG_GetPos( &mat ) ;

	if ( mat.m[3][1] > w->m[3][1] ) {
		return 1 ;
	}

	return 0 ;
}


static	void ActCorpDown( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_DOWN|ACT_STATUS_IK_DOWN|ACT_STATUS_TARGET_SKIP ) ;

	if ( time == 0 ) {
		if ( act->capture.flag & CAPTURE_HEAD ) {
			SetAction( act, 0, EM_dead_carry_body_end, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				SetAction( act, 0, EM_dead_carry_leg_end_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				SetAction( act, 0, EM_dead_carry_leg_end_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
		act->ctrl->skip_flag &= ~CTRL_SKIP_HZX ;	/* あたりチェック開始 */
	}
	CorpsMoveTarget( act ) ;

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act->ctrl ) ;
	}

#ifdef PUT_ITEM
	if ( time == COUNT_VMODE(30) ) {
		if ( act->dogtag_id >= 0 && act->dogtag_item.c_proc == 0 ) {
			KRTH_PutDogTagItemProc( &act->dogtag_item, &act->ctrl->mov, 
				*(act->ene_status), 1.0f, act->dogtag_id ) ;
			act->sw_dogtag = -1 ;
		} else {
			float n ;
			
			n = ( act->capture.flag & CAPTURE_HEAD )? 1.0f : 2.0f ;
			KRTH_PutItemProc( &act->item, &act->ctrl->mov, *(act->ene_status), n ) ;
		}
	}
#endif

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;

	    if ( FrontCheck( act ) ) {
			act->capture.flag = CAPTURE_FRONT ; /* 一度つかまれると仰向け */
			act->down_s = DownFront ;
printf(" DownFront !!\n");
		} else {
			act->capture.flag = CAPTURE_BACK ;
			act->down_s = DownBack ;
printf(" DownBack !!\n");
		}

		SetMode( act, ActCorp ) ;
	    return ;
	}
}

static	void ActCorpIntoLocker( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP ) ;
	act->ctrl->skip_flag |= CTRL_SKIP_HZX ;

	if ( time == 0 ) {
		if ( act->capture.flag & CAPTURE_HEAD ) {
			SetAction( act, 0, EM_dead_carry_body_put_locker,0,MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				SetAction( act, 0, EM_dead_carry_leg_put_locker_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				SetAction( act, 0, EM_dead_carry_leg_put_locker_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
		ReSetSlopeRotX( act->ctrl ) ;
	}

	if ( !(act->capture.flag & CAPTURE_LOCKER) ) { /* ロッカー入れられる */
	    SetMode( act, ActCorpDownLocker ) ;
	    return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	    SetMode( act, ActCorpStandLocker ) ;
	    return ;
	}
}

void ActCorpStandLocker( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_TARGET_SKIP|ACT_STATUS_INVISIBLE ) ;
	act->ctrl->step = DG_ZeroVector ;

	act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
	if ( time == 0 ) {
		if ( act->c_motion_num == EM_dead_carry_leg_put_locker_f ) {
			SetAction( act, 0, EM_dead_carry_locker_idle, 0, MOTION_MASK_FULL, 0 ) ;
		} else {
			SetAction( act, 0, EM_dead_carry_locker_idle, 0, MOTION_MASK_FULL, 0 ) ;
		}
		SET_FLAG( act->status, ACT_STATUS_CORRECT ) ;
	}

	if ( !(act->capture.flag & CAPTURE_LOCKER) ) { /* ロッカー入れられる */
	    SetMode( act, ActCorpDownLocker ) ;
	    return ;
	}
}

static	void ActCorpDownLocker( act, time )
ACTION	*act ;
int		time ;
{
	SET_FLAG( act->status, ACT_STATUS_DOWN|ACT_STATUS_TARGET_SKIP ) ;

	if ( time > COUNT_VMODE(140) ) {
		SET_FLAG( act->status, ACT_STATUS_IK_DOWN ) ;
	} else {
		act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
	}

	if ( time == 0 ) {
		SetAction( act, 0, EM_dead_carry_locker_surprise, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}
	CorpsMoveTarget( act ) ;

	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act->ctrl ) ;
	}

#ifdef PUT_ITEM
	if ( time == COUNT_VMODE(130) ) {
		if ( act->dogtag_id >= 0 && act->dogtag_item.c_proc == 0 ) {
			KRTH_PutDogTagItemProc( &act->dogtag_item, &act->ctrl->mov, 
				*(act->ene_status), 1.0f, act->dogtag_id ) ;
			act->sw_dogtag = -1 ;
		} else {
			float n ;

			n = ( act->capture.flag & CAPTURE_HEAD )? 1.0f : 2.0f ;
			KRTH_PutItemProc( &act->item, &act->ctrl->mov, *(act->ene_status), n ) ;
		}
	}
#endif

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	    if ( FrontCheck( act ) ) {
			act->capture.flag = CAPTURE_FRONT ; /* 一度つかまれると仰向け */
			act->down_s = DownFront ;
		} else {
			act->capture.flag = CAPTURE_BACK ;
			act->down_s = DownBack ;
		}
	    SetMode( act, ActCorp ) ;
	    return ;
	}
}

static int ChildTargetCheck( ACTION *act )
{
	int i ;

	for( i=0; i<CHILD_TARGET_NUM; i++ ) {
		if ( TARGET_POWER & act->child[ i ].child_trg.damaged ) {
printf("hit child num [%d] \n",i ) ;
			return i ;
		}
	}
	return -1 ;
}

/* 全てのダメージフラグをクリア */
static void DamageFlagClear( ACTION *act )
{
	CHILD_TARGETS *ch ;
	TARGET	*def ;
	int i ;
	
	ch = &act->child[ 0 ] ;
	for( i=0; i<CHILD_TARGET_NUM; i++ ) {
		ch->child_trg.damaged = FLAG_CLEAR ;
		ch->child_trg.weapon_type = 0 ;
		ch++ ;
	}
	def = &(act->deftrg) ;
	def->damaged = FLAG_CLEAR ;
	def->weapon_type = 0 ;
}

#if 0//no use
static void CaptureFlagClear( ACTION *act )
{
	if ( act->capture.capture != NULL ) {
		act->capture.capture->flag |= CAPTURE_FREE ;
		act->capture.capture = NULL ;	
	}
	act->capture.flag = 0 ;
}
#endif

static void DamageCaptureFlagClear( ACTION *act )
{
	DamageFlagClear( act ) ;
	if ( act->capture.capture != NULL ) {
		act->capture.capture->flag |= CAPTURE_FREE ;
		act->capture.capture = NULL ;	
		act->capture.flag = 0 ;
	}
}

static void SetBlood( ACTION *act )
{
	extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
	extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;
	Work 	*work ;
	TARGET	*dam ;
	FMATRIX	*world ;

	work = (Work *)act->w ;
	if ( work->corp_status & CORP_STATUS_NOBLOOD ) return ;

	dam = act->damtrg ;
	world = &(BODYWORLD( act->body, act->n_damobj )) ;
	GV_SetActorChild( act->w, NewBlood( world, &dam->hit, &dam->power->force, 0, 0 ) ) ;
	if ( act->oozeblood != NULL ) {
		TAKABE_OozeBloodAdd( act->oozeblood, act->n_damobj, &dam->hit, 50.0F ) ;
	}
}

static int PowerHitCheck( act )
ACTION	*act ;
{
	extern void ENE_SetNeedl( OBJECT *, int, FVECTOR * ) ;
	TARGET	*def ;
	int	child_num ;
	long64	weapon ;
	Work 	*work ;

	work = (Work *)act->w ;

	if ( (child_num = ChildTargetCheck( act )) >= 0 ) {
		def = &(act->child[child_num].child_trg) ;
		weapon = def->weapon_type ;
		act->n_damobj = bodynum[child_num] ;
		def->weapon_type = 0 ;
		act->damtrg = def ;
		DamageCaptureFlagClear( act ) ;

		if ( work->corp_status & CORP_STATUS_NOHIT ) return 0 ;

		if ( weapon & (WP_THROWG|WP_WALLCRASH) ) {
			return 0 ;
		} else if ( weapon & (WP_M92) ) {
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			ENE_SetNeedl( act->body, act->n_damobj, &act->damtrg->hit ) ;
		} else if ( weapon & (WP_BULLET) ) {
			if ( act->n_damobj == HUMAN21_ATAMA ) {
printf("corp hit atama!!\n");
				if ( work->corp_status & CORP_STATUS_LIVE ) {
printf("corp live > die !!\n");
					UNSET_FLAG( work->corp_status, CORP_STATUS_LIVE ) ;
					UNSET_FLAG( work->corp_status, CORP_STATUS_SLEEP ) ;
					UNSET_FLAG( work->corp_status, CORP_STATUS_FAINT ) ;
					if ( work->headmark != NULL ) {
						GV_DestroyOtherActor( work->headmark ) ;
						work->headmark = NULL ;
					}
					UNSET_FLAG( work->status, CORPS_ST_LIVE ) ;
					KR_KillCount() ;
				}
			}
			SetBlood( act ) ;
		}

		return 1 ;
	}

	def = &(act->deftrg) ;
	if ( TARGET_POWER & def->damaged ) {
		weapon = def->weapon_type ;
		act->n_damobj = HUMAN21_MUNE ;
		def->weapon_type = 0 ;
		act->damtrg = def ;
		DamageCaptureFlagClear( act ) ;

		if ( work->corp_status & CORP_STATUS_NOHIT ) return 0 ;

		return 0 ;
	}

	return 0 ;
}

static int	DamagePad( act )
ACTION	*act ;
{
    CAPTURE_TARGET	*cap ;
	TARGET	*def ;

	cap = &(act->capture) ;
	def = &(act->deftrg) ;
    if ( cap->capture != NULL ) { /* 捕まった */
		def->damaged = FLAG_CLEAR ;
//printf("korecorps [%d]\n",def->damaged ) ;
printf("DamagePad: act->capture.flag [%x] \n",act->capture.flag);
		DamageFlagClear( act ) ; /* 同時ダメージは捕まり優先なのでダメージクリア */
		SetMode( act, ActCorpWakeup ) ;
	    SET_FLAG( act->status, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP) ;
		return 1 ;
	}

	if ( PowerHitCheck( act ) ){
printf("power hit [%d]\n",def->damaged ) ;
		SetMode( act, ActCorpDamage ) ;
		SET_FLAG( act->status, ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP ) ;
		return 1 ;
	}

	return 0 ;
}

static	void	Action ( work )
Work	*work ;
{
	int	time ;
	CONTROL		*ctrl ;
	OBJECT		*body ;
	ACTION		*act ;

	ctrl = &work->control ;
	body = &work->body ;
	act = &work->action ;

	/* 行動処理 */
	body->flag = 0 ;				/* オブジェクトフラグ初期化 */
	act->act_end = 0 ;
	act->status = 0 ;

	time = act->time ++ ;
	if ( act->action == NULL ) return ;
	( *act->action )( act, time ) ;

	if ( !(ctrl->skip_flag & CTRL_SKIP_FLR_CHECK) ) {
		if ( ctrl->grounded & 1 ) ctrl->step.vy = 0.0F ;
		ctrl->step.vy -= 64.0F ;
	} else {
		/* Y移動量はモーションに任せる */
		ctrl->step.vy = act->body->height - act->old_body_height ;
	}

	if ( act->status & (ACT_STATUS_TARGET_SKIP) ) {
		SET_FLAG( act->deftrg.class, TARGET_SKIP )  ;
	} else {
		UNSET_FLAG( act->deftrg.class, TARGET_SKIP )  ;
	}

	if ( act->status & ACT_STATUS_INVISIBLE ) {
		if ( act->body->evmobj == NULL ) {
			SET_FLAG( act->body->objs->flag, DG_FLAG_INVISIBLE ) ;
		} else {
			SET_FLAG( act->body->evmobj->flag, DG_EVMOBJ_INVISIBLE ) ;
		}
	} else {
		if ( act->body->evmobj == NULL ) {
			UNSET_FLAG( act->body->objs->flag, DG_FLAG_INVISIBLE ) ;
		} else {
			UNSET_FLAG( act->body->evmobj->flag, DG_EVMOBJ_INVISIBLE ) ;
		}
	}

	if ( act->body->m_ctrl->rot_correct != 0 ) {
//printf("Action:rot_correct = %d \n",act->body->m_ctrl->rot_correct ) ;
		SET_FLAG( act->status, ACT_STATUS_CORRECT ) ;
	    act->ctrl->turn.vy += act->body->m_ctrl->rot_correct ;
		act->ctrl->turn.vy &=4095;
	    act->ctrl->rot.vy = act->ctrl->turn.vy ;
	    act->body->m_ctrl->rot_correct = 0 ;
	}
}

/*-----	----*/

enum {
	TH1_WAIT, TH1_DAMAGE, TH1_TAKE, TH1_MOVE, TH1_ROTTEN, TH1_ROTTEN_WAIT
} ;

static void Think1_Wait( Work *work )
{
	ACTION		*act ;
	
	act = &work->action ;

	if ( work->rotten_count <= 0 ) {
		SET_FLAG( act->deftrg.class, TARGET_SKIP ) ;
		work->status |= CORPS_ST_ROTTEN_START ;
		work->think1 = TH1_ROTTEN ;
		work->count3 = 0 ;
		return ;
	}

	if ( act->status & ACT_STATUS_DAMAGE ) {
		work->status |= CORPS_ST_ACT_CTRL ;
		work->think1 = TH1_DAMAGE ;
		work->count3 = 0 ;
		return ;
	} else if ( act->status & ACT_STATUS_CAPTURE ) {
		work->status |= CORPS_ST_ACT_CTRL ;
		work->think1 = TH1_MOVE ;
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static void Think1_Damage( Work *work )
{
	ACTION		*act ;
	
	act = &work->action ;

	if ( !(act->status & ACT_STATUS_DAMAGE) ) {
		work->status &= ~CORPS_ST_ACT_CTRL ;
		work->think1 = TH1_WAIT ;
		work->count3 = 0 ;
		return ;
	}
	if ( act->status & ACT_STATUS_CAPTURE ) {
		work->think1 = TH1_MOVE ;
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static void Think1_Move( Work *work )
{
	ACTION		*act ;

	act = &work->action ;

	if ( act->status & ACT_STATUS_DOWN && act->act_end == 1 ) {
		work->status &= ~CORPS_ST_ACT_CTRL ;
		work->think1 = TH1_WAIT ;
		work->count3 = 0 ;

		return ;
	}
	work->count3 ++ ;
}

static void Think1_Rotten( Work *work )
{
	ACTION		*act ;

	act = &work->action ;
	if ( work->count3 <COUNT_VMODE(60) ) {
		work->lights[1].m[3][0] -= 1.0F ;
		if ( work->lights[1].m[3][0] < 0 ) work->lights[1].m[3][0] = 0.0F ;
		work->lights[1].m[3][1] -= 1.0F ;
		if ( work->lights[1].m[3][1] < 0 ) work->lights[1].m[3][1] = 0.0F ;
		work->lights[1].m[3][2] -= 2.0F ;
		if ( work->lights[1].m[3][2] < 0 ) work->lights[1].m[3][2] = 0.0F ;
		DG_SetLightMatrix( work->body.objs, work->lights );
	}

	if ( work->count3 > COUNT_VMODE(60) ) {
		extern void *NewOldCorp( int, CONTROL *, OBJECT *, FMATRIX *, int ) ;

		work->status |= CORPS_ST_ROTTEN ;
		work->think1 = TH1_ROTTEN_WAIT ;
		work->count3 = 0 ;
//		NewOldCorp( work->oldcorps_name, &work->control, &work->body, work->lights, 0 ) ;
		GV_DestroyActor( work ) ;

		return ;
	}

	work->count3 ++ ;
}

static void StateManager( Work *work )
{
	switch( work->think1 ) {
		case TH1_WAIT:		/* 死にたて */
			Think1_Wait( work ) ;
		break ;
		case TH1_DAMAGE:		/* ダメージ */
			Think1_Damage( work ) ;
		break ;
		case TH1_MOVE:		/* 運ばれてる */
			Think1_Move( work ) ;
		break ;
		case TH1_ROTTEN:	/* 腐る */
			Think1_Rotten( work ) ;
		break ;
		case TH1_ROTTEN_WAIT:	/* 停止 */
		break ;
	}
}

static void NewSetIk( Work	*work )
{
	ACTION	*act ;

	act = &work->action ;

	if ( (work->status & (CORPS_ST_ACT_CTRL|CORPS_ST_LIVE)) || 
			(act->status & ACT_STATUS_CORRECT) || !(work->new_ik_flag) ) {
		if ( act->status & ACT_STATUS_IK_DOWN ) {
			TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, 0 );
		} else if ( act->status & ACT_STATUS_IK_FOOT ) {
			TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, 1 );
		} else if ( act->status & ACT_STATUS_IK_HAND ) {
			TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, 2 );
		} else {
			TAKABE_UtilPuppetIK_AutoConfigOfEnemey( act->new_ik, -1 );
		}

		if ( act->status & ACT_STATUS_IK_PIKU ) {
			if ( act->n_damobj == HUMAN21_MUNE || act->n_damobj == HUMAN21_ATAMA 
			|| act->n_damobj == HUMAN21_KOSHI || act->n_damobj == HUMAN21_ONAKA ) {
				TAKABE_UtilPuppetIK_Piku( act->new_ik, -1 );
			} else {
				TAKABE_UtilPuppetIK_Piku( act->new_ik, act->n_damobj );
			}
		}
		work->new_ik_flag = TAKABE_ActPuppetIK( act->new_ik );
	}
}

static void ActControl( Work *work )
{
	ACTION	*act;
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;
	act = &work->action ;
	if ( !(work->status & CORPS_ST_ROTTEN) ) {
		act->old_body_height = act->body->height ;
		if ( (work->status & (CORPS_ST_ACT_CTRL|CORPS_ST_LIVE)) ||
				(act->status & ACT_STATUS_CORRECT) ||
				 !(work->new_ik_flag) ) {
			GM_ActMotion( &work->body );
#if 0
			/* 階段でめり込み防止 */
			if ( act->ctrl->flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
				float	geta ;

				geta = 0.0f ;
				if ( act->status & ACT_STATUS_DEATH ) {
					geta = 250.0f ;
				} else if ( act->status & ACT_STATUS_WAKEUP ) {
					geta = 250.0f - (float)(act->time*16) ;
					if ( geta < 0.0f ) geta = 0 ;
				}
				ctrl->height = act->body->height+ geta ;
		//printf(" geta[%f]	\n", geta ) ;
		    } else {
				ctrl->height = act->body->height ;
			}
#endif
			ctrl->height = act->body->height ;
			GM_ActControl( &work->control ) ;
			GM_ActObject2( &work->body );
			SET_FLAG( work->body.objs->flag, DG_FLAG_SHADOWMAKE) ;/* スポットライト影ＯＮ */
			
			/* 敵兵のIDで固定*/
		    MT_SetMotionSeTable( work->body.m_ctrl, GM_CurrentMap, 1, 
					 ( ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 ) ;
		} else {
			if( !(work->status & CORPS_ST_SPOT_SHADOW) ) {
				UNSET_FLAG( work->body.objs->flag, DG_FLAG_SHADOWMAKE) ;/* スポットライト影OFF */
			}
		}
#ifdef IK_ON
		NewSetIk( work ) ;
#endif
		DG_GetLightMatrix( &work->control.mov, work->lights );
	}
}

static void ProcCall( Work *work, int proc_no )
{
	ENE_ExecProc( work->proc[ proc_no ], NULL ) ;
	work->proc[ proc_no ] = 0 ;/* 一回しか呼ばない */
}

static void ActStatusCheck( Work *work )
{
	ACTION	*act ;
	
	act = &work->action ;

	if ( act->status & ACT_STATUS_DAMAGE ) {
		ProcCall( work, CORP_PROC_DAMAGE ) ;
	}
}

#define	CORP_AREA	(750.0f)
#define	CORP_AREA_H	(1250.0f)
static void PlayerCheck( Work *work )
{
	CONTROL	*ctrl ;
	float	f ;
	
	ctrl = &work->control ;
	
	f = ctrl->mov.vx - GM_PlayerPosition.vx ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return ;
	f = ctrl->mov.vz - GM_PlayerPosition.vz ;
	if ( f > CORP_AREA || f < -CORP_AREA ) return ;
	f = ctrl->mov.vy - GM_PlayerPosition.vy ;
	if ( f < -CORP_AREA_H || f > 0.0f ) return ;

	/* 壁チェック */
	if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &ctrl->mov, &GM_PlayerPosition,
			HZX_CHK_ALL, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) return ;

	GM_PlayerStatus |= PLAYER_ON_CORPSE ;
}

static void TrapCheck( Work *work )
{
	
	if( KR_CheckTrap( &work->control, CORP_TRAP_SHADOW ) ) {
		SET_FLAG( work->status, CORPS_ST_SPOT_SHADOW ) ;
//printf( "aaaaaaaaaaaaaaaaa\n") ;
	} else {
		UNSET_FLAG( work->status, CORPS_ST_SPOT_SHADOW ) ;
//printf( "bbbbbbbbbbbbbbbbbbbbb\n") ;
	}
}

static void CorpsMain( Work *work )
{
//printf("aaaaaaa\n") ;
	ActControl( work ) ;

	TrapCheck( work ) ;

	PlayerCheck( work ) ;

	/* 状態管理 */
	StateManager( work ) ;

	if ( !(work->status & CORPS_ST_ROTTEN_START) ) {
		CONTROL	*ctrl ;
		Action( work ) ;
		ActStatusCheck( work ) ;

		/* 後処理 */
		ctrl = &work->control ;
	}
}

static void Act( Work *work )
{
	CorpsMain( work ) ;
//printf("corp act: act->capture.flag [%x] \n",work->action.capture.flag);
//printf("work->body.objs->flag[0x%x]\n",work->body.objs->flag) ;
//printf("corp:control.vy[%f]  hzx_base[%f]\n",work->control.mov.vy,work->control.hzx_base ) ;
}

static void Die( Work *work )
{
	extern int NDETECT_Remove( void* pvNDetect) ;
	extern void SearchAndKillAttachment_called(OBJECT *obj) ;
	void 		(*func)( void *, OBJECT *, CONTROL * ) ;

	if ( work->die != NULL ) {
		func = work->die ;
		( *func )( work->ptr, &work->body, &work->control ) ;
	}

	TAKABE_FreePuppetIK( work->action.new_ik ) ;

	if ( !(GV_IsStageDestroy( work )) ) {
		if ( work->action.oozeblood != NULL ) GV_DestroyActorQuick( work->action.oozeblood ) ;
	}

	if ( work->d_name != NULL ) {
		NDETECT_Remove( work->d_name ) ;
	}

	SearchAndKillAttachment_called( &work->body ) ;

//    GM_FreeRadarControl( &work->rctrl ) ;
	GM_FreeZoneIntrpt( &work->z_intrpt ) ;
    GM_FreeControl( &work->control ) ;
    GM_FreeObject( &work->body ) ;
    GM_FreeTarget( &work->action.deftrg ) ;	/* 親だけで良い */
	if( work->fcanim != NULL ) FC_ReleaseFaceControl( work->fcanim ) ;
}
/*--- ------------------------------------------------------------*/
static int InitThink( work, pos, rot )
Work	*work ;
FVECTOR	*pos ;
SVECTOR	*rot ;
{
	work->think1 = TH1_WAIT ;
	work->control.mov = *pos ;
	work->control.rot = *rot ;

	GM_ConfigControlHzxHeight( &(work->control), 750.0F, work->control.mov.vy + 100.0F ) ;

	work->action.CheckDamage = DamagePad ;
	return 0 ;
}

static void SetDethMotion( work, body, motion_num, down_s )
Work	*work ;
OBJECT	*body ;
int		motion_num ;
int		down_s ;
{
	ACTION	*act ;
	int		act_num ;
	
	act = &work->action ;

	act_num = motion_num ;
	act->c_motion_num = act_num ;
	GM_ConfigObjectAction( &work->body, 0, act_num, 0, MOTION_MASK_FULL, 0 );

	if ( body != NULL ) {
		_CopyObjectWorld( body->objs, work->body.objs ) ;
		memcpy( &work->body.m_ctrl->abs_rots[0], &body->m_ctrl->abs_rots[0], sizeof( FVECTOR ) * 21 ) ;

		DG_SetPos2( &work->control.mov, &work->control.rot );
		DG_PutObjs( work->body.objs ) ;
//		work->body.height = work->control.height = body->height ;

		GM_ActMotion( &work->body );
	} else {
		work->control.hzx_base = work->control.mov.vy ;
		GM_ActMotion( &work->body );
		work->control.height = work->body.height ;
		GM_ActControl( &work->control ) ;
		GM_ActObject2( &work->body );
	}

	DG_GetLightMatrix( &work->control.mov, work->lights );

	SetMode( act, ActCorp ) ;
	act->down_s = down_s ;

}

/* 引き摺られ用 */
/* ターゲットコールバック関数 */
static	void	ChildTargCallBack( off, def, ptr )
TARGET	*off, *def ;
void	*ptr ;
{
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
		}
	}
}

static	void	InitTarget( work, down_s )
Work	*work ;
int		down_s ;
{
	TARGET			*trg, *t ;
	POWER_TARGET	*power ;
    FVECTOR	size, offset ;
    ACTION		*act ;
	CHILD_TARGETS 	*child ;
    int				i ;

	act = &work->action ;

    t = &( work->action.deftrg ) ;
    size.vx = size.vz = 1000.0F ; size.vy = 500.0F ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_CHILD, 0, ENEMY_SIDE,
		 &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, 0 ) ;
//NewTargetView( t, 32, 128, 64 ) ;

	trg = &act->deftrg ;
	power = &act->power ;
    GM_SetPowerTarget( trg, power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;

#if 1//BP - seems we should leave this in even though they only added it for the Windows build.
//#ifdef KP_WINDOWS
	offset.vz = 1.0f ;	// Nan対策
#endif

	for( i=0; i<CHILD_TARGET_NUM; i++ ) {
		child = &act->child[ i ] ;
		t = &(child->child_trg) ;
		child->parts_num = bodynum[i] ;
		switch ( i ) {
			case 0 :	/* 頭 */
				size.vx = size.vz = 96.0F ; size.vy = 130.0F ;
				offset.vx = offset.vz = 0.0F ; offset.vy = 30.0F ;
				child->level = 1 ;
//NewTargetView( t, 200, 34, 184 ) ;
				break ;
			case 1:	/* 右腕１ */
			case 2:	/* 右腕２ */
			case 3:	/* 左腕１ */
			case 4:	/* 左腕２ */
				size.vx = size.vz = 96.0F ; size.vy = 130.0F ;
				offset.vx = offset.vz = 0.0F ; offset.vy = 30.0F ;
				child->level = 2 ;
//NewTargetView( t, 200, 34, 184 ) ;
				break ;
			case 5 :	/* 右足１ */
			case 6 :	/* 左足１ */
				size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 200.0F ;
				offset.vx = offset.vz = -10.0F ; offset.vy = -200.0F ;
				child->level = 2 ;
				break ;
			case 7 :	/* 右足２ */
			case 8 :	/* 左足２ */
				size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 250.0F ;
				offset.vx = offset.vz = -20.0F ; offset.vy = -250.0F ;
				child->level = 2 ;
				break ;
			case 9 :	/* 胴体 */
				size.vx = 200.0F ; size.vz = 200.0F ; size.vy = 300.0F ;
				offset.vx = offset.vz = 0.0F ; offset.vy = 0.0F ;
				child->level = 3 ;
				break ;
		}
		GM_SetTarget( t, CHILD_TRG_FLAG, 0, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( t, 0 ) ;
		GM_SetPowerTarget( t, &child->child_pow, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( t, ChildTargCallBack, work ) ;
		GM_SetTargetParts( trg, t, 1, child->level ) ;
//NewTargetView( t, 0, 0, 128 ) ;
	}

	/* キャプチャーターゲット */
    GM_SetCaptureTarget( trg, &act->capture, &( work->control ), &( work->body ) ) ;
    GM_PutTarget( trg ) ;
	trg->class |= TARGET_DEAD ; /* 死んでいる */
//NewTargetView( trg, 32, 128, 64 ) ;

	CorpsMoveTarget( &work->action ) ;

    work->control.skip_flag |= CTRL_SKIP_HZX ; /* 当たり見ない */
    if ( down_s == DownFront || down_s == DownWall ) { /* 仰向け */
		work->action.capture.flag = CAPTURE_FRONT ;
    } else {	       	/* うつぶせ */
		work->action.capture.flag = CAPTURE_BACK ;
    }
}

/* 汎用呼び出しモーションは gbs.mar を使用 */
/*--- ------------------------------------------------------------*/
static int GetResources( work, 
	name, oldbody, pos, rot, model, 
	m_model, down_s, m_num, ooze, ptr, 
	set, die, status, item, m_name,
	sft_ptr, far_name )
Work 	*work ;
int		name ;		/* コントロール名 */
OBJECT	*oldbody ;		/* オブジェクト */
FVECTOR	*pos ;		/* 場所 */
SVECTOR	*rot ;		/* 方向 */
int		model ;		/* モデル名 */
int		m_model ;	/* マルチウェイトモデル名 */
int 	down_s ;	/* 仰向け １、うつ伏せ ２ */
int		m_num ;		/* モーション番号 */
void	*ooze ;		/* にじみ血 */
void	*ptr ;	/* 呼び出し側ワーク */
CORPS_CALLBACK	set ;	/* 初期化関数 */
CORPS_CALLBACK	die ;	/* ダイ関数 */
int		status ;
ITEM_PROC	*item ;
int		m_name ;	/* モーション名 */
FVECTOR	*sft_ptr ;
int		far_name ;	/* 顔アニメモーション */
{
	extern void GM_InitRadarControl( RADAR_CTRL *, FVECTOR *, int , int  );
	extern void GM_RadarSetVRange( RADAR_CTRL *, float , float );
	extern void TAKABE_OozeBloodChangeObjs( void *, DG_OBJS * ) ;
	extern void *NewOozeBlood( DG_OBJS *, int ) ;
	extern void *NewSlingGun( OBJECT *, OBJECT *, int, int * ) ;
	void 		(*func)( void *, OBJECT *, CONTROL * ) ;
	DG_OBJS		*corp ;
	int			motion_num ;
	CONTROL		*ctrl ;
	OBJECT		*body ;
	ACTION		*act ;
	int i ;

	work->status = 0 ;
	work->corp_status = status ;

	if ( status & CORP_STATUS_LIVE ) {
		work->status |= CORPS_ST_LIVE ;
	}
	if ( status & CORP_STATUS_SHADOW ) {
		work->status |= CORPS_ST_SPOT_SHADOW ;
	}

	act = &work->action ;
	ctrl = act->ctrl = &work->control ;
	body = act->body = &work->body ;

	GM_InitControl( ctrl, name, 0 ) ;
	GM_ConfigControlHazard( ctrl, 1200, 450, 550 ) ;
	ctrl->mov = *pos ;
    GM_ConfigControlHzxHeight( ctrl, 750.0F, ctrl->mov.vy + 100.0F ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;		/* トラップチェック */
	GM_ConfigControlMapCheck( ctrl ) ;		/* ゾーンを利用したマップ変更ON */
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ; /* 壁チェックは水平方向チェック */
    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
//	GM_InitRadarControl( &work->rctrl, &ctrl->mov, RADAR_VISIBLE|RADAR_SIGHT, -1 );
//	GM_RadarSetVRange( &work->rctrl, RADAR_VRANGE_UPPER , RADAR_VRANGE_LOWER );

	GM_SetZoneIntrpt( &work->z_intrpt, ctrl, 0, 0, ZONE_INTRPT_CORP ) ;
	GM_PutZoneIntrpt( &work->z_intrpt ) ;

	/* オブジェクトの初期化 */
	if ( m_model ) {
		GM_InitObject( body, model, DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE ) ;
		GM_ConfigObjectEvm( body, m_model, DG_EVMOBJ_IRREACTION ) ;
		/* 各関節のinvフラグが立たないように後でやる */
		body->objs->flag |= DG_FLAG_INVISIBLE ;
	} else {
		GM_InitObject( body, model, DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE );
	}

	GM_ConfigObjectLight( body, work->lights );
	GM_ConfigObjectMotion( body, 1, m_name, MT_FLAG_HUMAN2 );
	GM_ConfigObjectStep( body, &ctrl->step );
	GM_ConfigObjectAction( body, 0, 0, 0, 0xfffff, 0 );

	/* object control の関連付け */
	GM_ConfigControlObject( ctrl, body ) ;

	/* アクション初期化 */
	GM_ConfigObjectAction( act->body, 0, 0, 0, MOTION_MASK_FULL, 0 );

	act->w = work ;

	/* 思考パラメータ初期化 */
	if ( InitThink( work, pos, rot ) < 0 ) {
		return -1 ;
	}

	/* 死にモーション生成 */
	motion_num = ( down_s == DownFront )? EM_dead_idle_f : EM_dam_out_pause ;
	if ( m_num > 2 ) motion_num = m_num ;
	SetDethMotion( work, oldbody, motion_num, down_s ) ;

	/* ＩＫ */
	act->new_ik = TAKABE_MakePuppetIK( ctrl, body );
	act->ik_time = 0 ;
	work->new_ik_flag = 0 ;

	work->control.turn = *rot ;
	work->control.rot = work->control.turn ;
	GM_ConfigControlMapID( &(work->control) ) ;

	/* 引き摺られ用 */
	InitTarget( work, down_s ) ;

	work->rotten_count = DEF_ROTTEN_TIME ;
	work->tmp_time = 0 ;

	work->action.oozeblood = ooze ;
	corp = work->body.objs ;

	if ( ooze != NULL )	{ 
		/* 滲み血受け取る */
		TAKABE_OozeBloodChangeObjs( ooze, corp ) ;
	}
	if ( status & CORP_STATUS_OOZE ) {
		if ( ooze == NULL ) {
			work->action.oozeblood = NewOozeBlood( body->objs, model ) ;
		}
	}

	work->oldcorps_name = model ;

	work->ptr = ptr ;
	work->set = set ;
	work->die = die ;
	if ( work->set != NULL ) {
		func = set ;
		( *func )( work->ptr, body, ctrl ) ;
	}

	work->headmark = NULL ;
	if ( status & CORP_STATUS_SLEEP ) {
		GV_SetActorChild( work, 
			(work->headmark = New_Zzz(&BODYWORLD( act->body, HUMAN21_ATAMA ))) ) ;
	} else if ( status & CORP_STATUS_FAINT ) {
		GV_SetActorChild( work, 
			(work->headmark = NewPiyori(&BODYWORLD( act->body, HUMAN21_ATAMA ), &ctrl->map)) ) ;
	}

	if ( status & CORP_STATUS_SPLASH ) {
		work->splash = 1 ;
		GV_SetActorChild( work, NewBodySplash2( act->body->objs, act->ctrl, model, &work->splash ) ) ;
	}

	HZX_EnterTrap( ctrl->hzx_id, &( ctrl->evt ) ) ;
	TrapCheck( work ) ;
	if ( !(work->status & CORPS_ST_SPOT_SHADOW) ) {
		UNSET_FLAG( work->body.objs->flag, DG_FLAG_SHADOWMAKE) ;/* スポットライト影OFF */
	}

	if( item!=NULL ) {
		act->item = *item ;
	} else {
		act->item.n_proc = 0 ;
		act->item.c_proc = 0 ;
	}
	work->action.ene_status = &work->status ;

printf(" corps stary status[%x]\n",work->status ) ;

	for ( i=0; i<4; i++ ){
		act->wakeup_shift[i] = *(sft_ptr++) ;
	}
	for ( i=0; i<4; i++ ){
		act->take_shift[i] = *(sft_ptr++) ;
	}

	if ( far_name ) {
		NewFaceManager_forProg( ) ;
		if ( (work->fcanim = FC_NewFaceControl( body->evmobj, far_name, 0 )) == NULL ) {
			return -1 ;
		}
	    FC_ExecFaceAnim( work->fcanim, 0, 0, 0 ) ;
	} else {
		work->fcanim = NULL ;
	}

	GV_SetActorMessageKill( work, name ) ;

//printf("corp:start control.vy[%f]  hzx_base[%f]\n",work->control.mov.vy,work->body.height ) ;

	return (0);
}

void *NewCorp( name, body, pos, rot, model, down_s, ooze, ptr, set, die, status )
int		name ;	/* コントロール名 */
OBJECT	*body ;	/* オブジェクト */
FVECTOR	*pos ;	/* 初期位置 */
SVECTOR	*rot ;	/* 初期方向 */
int	model ;		/* モデル名 */
int down_s ;	/* 仰向け １、うつ伏せ ２ */
void	*ooze ;	/* にじみ血 */
void	*ptr ;	/* 呼び出し側ワーク */
CORPS_CALLBACK	set ;	/* 初期化関数 */
CORPS_CALLBACK	die ;	/* ダイ関数 */
int status ;
{
	Work	*work ;
	int		i ;

	OPERATOR() ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		for( i=0; i<MAX_PROC; i++ ) {
			work->proc[i] = 0 ;
		}
		if ( GetResources( work, 
				name, body, pos, rot, model, 
				0, down_s, 0, ooze, ptr, 
				set, die, status, NULL, CORP_MOTION,
				&Def_shift[0], 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return work ;
}

void	*NewCorpGcl( name, where )
int		name ;
int		where ;
{
	Work		*work ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	char		*opt ;
	int			model ;
	int			down_s ;
	int			status ;
	int			mot ;
	int			m_model ;
	void		*ooze ;
	ITEM_PROC	item ;
	int			m_name ;
	int			val ;
	FVECTOR		shift[8], *sft_ptr ;
	int			far_name ;
	int			i ;

	OPERATOR() ;

	if( (model = GCL_GetOptionValue( 'm', 0 )) == 0 ) {
		printf("corp: ERR!! NO MODEL NAME\n");
		return NULL ;
	}

#if 1
	down_s = GCL_GetOptionValue( 'd', 0 ) ;
#else
	if( (down_s = GCL_GetOptionValue( 'd', 0 )) == 0 ) {
		printf("corp: ERR!! NO DOWN POSE\n");
		return NULL ;
	}
#endif

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &pos ) ;
	} else {
		printf("corp: ERR!! NO POSITION\n");
		return NULL ;
	}

	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		GCL_GetSV( opt, (short *)&rot ) ;
	} else {
		printf("corp: ERR!! NO ROTATION\n");
		return NULL ;
	}

	if ( ( GCL_GetOption( 'f' ) ) != NULL ){
		if ( (val = ENE_GCL_GetFVs( &shift[0] )) != 7 ) {
			printf("corp: ERR!! SHIFT VALUE NUM [%d]/7\n",val);
			return NULL ;
		}
		sft_ptr = &shift[0] ;
	} else {
		sft_ptr = &Def_shift[0] ;
	}

	mot = GCL_GetOptionValue( 'n', 0 ) ;
	status = GCL_GetOptionValue( 's', 0 ) ;
	m_model = GCL_GetOptionValue( 'e', 0 ) ;
	m_name = GCL_GetOptionValue( 'l', CORP_MOTION ) ;
	far_name = GCL_GetOptionValue( 'a', 0 ) ;

#if 1
	if ( (GM_GameLevel != GM_LEVEL_E_EXTREME) &&
		 (GCL_GetOption( 'q' ) != NULL) ){
		KRTH_GetItemProc( &item, name ) ;
	} else {
		item.n_proc = 0 ;
		item.c_proc = 0 ;
	}
#else
	if ( GCL_GetOption( 'q' ) != NULL ){
		KRTH_GetItemProc( &item, name ) ;
	} else {
		item.n_proc = 0 ;
		item.c_proc = 0 ;
	}
#endif

	ooze = NULL ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;

		for( i=0; i<MAX_PROC; i++ ) {
			work->proc[i] = 0 ;
		}
		if ( GCL_GetOption( 'b' ) != NULL ){
			for( i=0; i<MAX_PROC; i++ ) {
				if( GCL_NextStr() != NULL ) {
					work->proc[i] = GCL_GetNextInt( ) ;
				} else {
					break ;
				}
			}
		}

		if ( GetResources( work, 
				name, NULL, &pos, &rot, model, 
				m_model, down_s, mot, ooze, NULL, 
				NULL, NULL, status, &item, m_name,
				sft_ptr, far_name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		{
			ACTION	*act ;
			OBJECT	*body ;

			act = &work->action ;
			body = &work->body ;
			work->d_name = NULL ;
#if 1
			if ( (GCL_GetOption( 'j' ) != NULL) &&
				 (GM_GameLevel != GM_LEVEL_E_EXTREME) ){
#else
			if ( GCL_GetOption( 'j' ) != NULL ){
#endif
				char *ptr_le, *tmp, *country ;
				int	level, birth ;
				
				level = 0 ;
				switch ( GM_GameLevel ) {
					case GM_LEVEL_VERYEASY :
					break ;
					case GM_LEVEL_EASY :
						level = 1 ;
					break ;
					case GM_LEVEL_NORMAL :
						level = 2 ;
					break ;
					case GM_LEVEL_HARD :
						level = 3 ;
					break ;
					case GM_LEVEL_EXTREME :
						level = 4 ;
					break ;
				}

				tmp = GCL_NextStr();	/* NextStr 先取り */
				act->dogtag_item.proc[ 0 ] = GCL_GetNextInt();

				ptr_le = GCL_GetNextResource_LE();
				for ( i=0; i<level; i++ ) {
					ptr_le = GCL_GetNextResource_LE();
				}

				GCL_SetArgTop( ptr_le );
				act->dogtag_id = GCL_GetNextInt();
	ASSERT( act->dogtag_id >= 0 ) ;
#ifdef DOGTAG_DOUBLE
				if ( GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) {
					for ( i=0; i<DOGTAG_RES_NUM; i++ ) {
						GCL_GetNextInt();
					}
				}
#endif

				act->dogtag_item.str = GCL_GetNextString();
				country = GCL_GetNextString();
				birth = GCL_GetNextInt();

				act->dogtag_item.probability[ 0 ] = 100 ;
				act->dogtag_item.n_proc = 1 ;
				act->dogtag_item.c_proc = 0 ;
printf("dogta ID[%d]\n",act->dogtag_id);
				if ( SIG_CheckDogTagFlag( act->dogtag_id ) ) {
					act->dogtag_item.c_proc = 1 ;	/* 取得済み */
				} else {
					act->sw_dogtag = 1 ;
					GV_SetActorChild( work, 
						NewCreateDogtag( body, E_DOGTAG_NAME, &DG_ZeroVector, &act->sw_dogtag) ) ;
				}
				{
					extern	void *NewNamePrint( OBJECT *body, char *str ,int id, int birth ) ;
					work->d_name = NewNamePrint(body,act->dogtag_item.str ,act->dogtag_id, birth) ;
				}
printf("dogtagname[%s]\n",act->dogtag_item.str);
			} else {
				act->dogtag_id = -1 ;
				act->dogtag_item.n_proc = 0 ;
				act->dogtag_item.c_proc = 0 ;
			}
		}
	}

	return work ;
}
