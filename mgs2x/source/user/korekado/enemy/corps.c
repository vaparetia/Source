//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	corps.c
	死体

	1999/07/28 Y.Korekado
	$Id: corps.c,v 1.2 2003/01/09 14:32:13 Yoshizawa1 Exp $
	
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
#include <string.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"enemy.h"
#ifdef PSX2
#include	"../../kano/corpse/corpse_ik.x"
#endif

#define TAKABE_IK	(1)
#define NO_KANO_IK	(1)

#define DAM_BUG	(1)


//#define	DYNAMIC_ON
#define	PUT_ITEM (1)

/*-----		-----*/
#define	DEF_ROTTEN_TIME		COUNT_VMODE(60*60*5) 	/* 腐る時間:３分 */
//#define	DEF_ROTTEN_TIME		COUNT_VMODE(300)

#define	SHORT_ROTTEN_TIME		COUNT_VMODE(300)
#define DYNAMIC_HEIGHT	(125)
#define DYNAMIC_WIDTH	(300)
#define DYNAMIC_WIDTH2	(200)
#define FLOOR_N			(5)

//#define CORP_MOT_CHECK	(1)	//引き摺りモーションがモデルに合うかチェックする

#ifdef CORP_MOT_CHECK
#define	CORP_MODEL	GV_StrCode("org_sgl")
#endif

/*----- ワーク定義 -----*/
typedef	struct	{
	GV_ACT_EX		actor ;
	CONTROL		control ;
	OBJECT		body ;
	OBJECT		weapon ;
	FMATRIX		lights[2] ;

	NEWCORP		newcorp ;
	OLDCORP		oldcorp ;

	/* 暫定モーションデータ */
	int			motion_data[ MOTION_MAX ];

	/* アクション */
	ACTION		action ;

	/* 敵兵思考 */
	ENETHINK	enethink ;

	void		*ik ;
	int			new_ik_flag ;
	int			oldcorps_name ;
	u_int		uni_name_no ;

#ifdef	DYNAMIC_ON
	HZX_D_FLOOR	*d_floor[FLOOR_N] ;
	IVECTOR		pos[4] ;
	FVECTOR		d_floor_center ;
#endif
} Work ;

/*-----	----*/
extern void SearchAndKillAttachment_called(OBJECT *obj) ;
/*-----	----*/
static	void ActCorp( ACTION *, int ) ;
static	void ActCorpWakeup( ACTION *, int ) ;
static	void ActCorpTake( ACTION *, int ) ;
static	void ActCorpMove( ACTION *, int ) ;
static	void ActCorpDown( ACTION *, int ) ;
static	void ActCorpIntoLocker( ACTION *, int ) ;
static	void ActCorpIntoToilet( ACTION *, int ) ;
void ActCorpsStandLocker( ACTION *, int ) ;
static	void ActCorpDownLocker( ACTION *, int ) ;

/*-----	----*/

#if 0
#define DYNAMIC_FLOOR_FLAG	(HZX_FLOOR_NO_BULLET|HZX_FLOOR_NO_MISSILE|HZX_FLOOR_NO_C4| \
	HZX_FLOOR_NO_PLAYER| \
	HZX_FLOOR_NO_RECOIL|HZX_FLOOR_NO_CLAYMORE|HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_SPRAY)
#else
#define DYNAMIC_FLOOR_FLAG	(HZX_FLOOR_NO_BULLET|HZX_FLOOR_NO_MISSILE|HZX_FLOOR_NO_C4| \
	HZX_FLOOR_NO_RECOIL|HZX_FLOOR_NO_CLAYMORE|HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_SPRAY)
#endif


#ifdef	DYNAMIC_ON

static void MoveDynamic( work )
Work	*work ;
{
	IVECTOR	shift ;
	FVECTOR	*center ;
	int i ;

	center = &work->control.mov ;

	shift.vx = (int)center->vx - work->d_floor_center.vx ;
	shift.vy = ( int )work->control.levels[ 0 ] - work->d_floor_center.vy ;
	shift.vz = (int)center->vz - work->d_floor_center.vz ;

	for ( i=0; i<FLOOR_N; i++ ) {
		HZX_ShiftDynamicFloor( work->d_floor[i], &shift ) ;
	}

   	work->d_floor_center.vx = (int)center->vx ;
   	work->d_floor_center.vy = (int)work->control.levels[ 0 ] ;
   	work->d_floor_center.vz = (int)center->vz ;
}

static void UnSetFlagDynamic( work, flag )
Work	*work ;
{
	int i ;
	for ( i=0; i<FLOOR_N; i++ ) {
		UNSET_FLAG( work->d_floor[i]->atr, flag ) ;
	}
}

static void SetFlagDynamic( work, flag )
Work	*work ;
{
	int i ;
	for ( i=0; i<FLOOR_N; i++ ) {
		SET_FLAG( work->d_floor[i]->atr, flag ) ;
	}
}

static void UnSetDynamic( work )
Work	*work ;
{
	int i ;

	for ( i=0; i<FLOOR_N; i++ ) {
		HZX_RemoveDynamicFloor( work->d_floor[i] ) ;
		work->d_floor[i] = NULL ;
	}
}

static void SetDynamic( work )
Work	*work ;
{
	float	levels[2] ;
	FVECTOR	*center ;
	IVECTOR	*pos, pos_buff[4] ;
	int		floor, flag ;

    GM_CurrentMap = work->control.map ;

	center = &work->control.mov ;

    flag = HZX_LevelHazardCheck( work->control.hzx_id, center,
				 work->control.hzx_check_type, work->control.flr_flag ) ;
	if ( flag != 0 ) {
		HZX_GetLevelHeight( levels ) ; 
		if ( !( 1 & flag ) ) levels[ 0 ] = 0.0F ;
	} else {
		levels[ 0 ] = 0.0f ;
	}

	floor = ( int )levels[ 0 ] ;

   	work->d_floor_center.vx = (int)center->vx ;
   	work->d_floor_center.vy = floor ;
   	work->d_floor_center.vz = (int)center->vz ;

	pos = &work->pos[0] ;

	pos->vx = (int)center->vx - DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx - DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH2 ;

	pos = &work->pos[0] ;
	work->d_floor[0] = HZX_AddDynamicFloor( work->control.hzx_id,
							pos, pos+1, pos+2, pos+3,
						     4, DYNAMIC_FLOOR_FLAG ) ;

	pos = &pos_buff[0] ;
	pos->vx = (int)center->vx - DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx - DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH2 ;

	pos = &pos_buff[0] ;
	work->d_floor[1] = HZX_AddDynamicFloor( work->control.hzx_id,
							pos, pos+1, pos+2, pos+3,
						     4, DYNAMIC_FLOOR_FLAG ) ;

	pos = &pos_buff[0] ;
	pos->vx = (int)center->vx - DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH ;
	pos++ ;

	pos->vx = (int)center->vx - DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx - DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx - DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH ;

	pos = &pos_buff[0] ;
	work->d_floor[2] = HZX_AddDynamicFloor( work->control.hzx_id,
							pos, pos+1, pos+2, pos+3,
						     4, DYNAMIC_FLOOR_FLAG ) ;

	pos = &pos_buff[0] ;
	pos->vx = (int)center->vx + DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz - DYNAMIC_WIDTH ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH2 ;

	pos = &pos_buff[0] ;
	work->d_floor[3] = HZX_AddDynamicFloor( work->control.hzx_id,
							pos, pos+1, pos+2, pos+3,
						     4, DYNAMIC_FLOOR_FLAG ) ;

	pos = &pos_buff[0] ;
	pos->vx = (int)center->vx - DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH2 ;
	pos->vy = floor + DYNAMIC_HEIGHT ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH2 ;
	pos++ ;

	pos->vx = (int)center->vx + DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH ;
	pos++ ;

	pos->vx = (int)center->vx - DYNAMIC_WIDTH ;
	pos->vy = floor ;
	pos->vz = (int)center->vz + DYNAMIC_WIDTH ;

	pos = &pos_buff[0] ;
	work->d_floor[4] = HZX_AddDynamicFloor( work->control.hzx_id,
							pos, pos+1, pos+2, pos+3,
						     4, DYNAMIC_FLOOR_FLAG ) ;

}
#endif

static int FloorCheck( ACTION	*act )
{
	CONTROL	*ctrl ;
	float f ;

	ctrl = act->ctrl ;
	f = KR_GetGRotFromPos( &ctrl->mov, ctrl->hzx_id, HZX_CHK_ALL, ctrl->flr_flag ) ;
//printf("mov[%f] f[%f]\n",ctrl->mov.vy, f);
	if ( (ctrl->mov.vy-f) > 300.0f ) return 1 ;
	return 0 ;
}

static void SetSlopeRotX( ctrl )
CONTROL	*ctrl ;
{
	int x ;
	x = ENE_GetGRot( ctrl, 1000.0f ) ;
//printf( " slope Rot [%d]\n",x);	
	ctrl->turn.vx = x ;
}
static void ReSetSlopeRotX( ctrl )
CONTROL	*ctrl ;
{
	ctrl->turn.vx = 0 ;
}

static void CorpsMoveTarget( act )
ACTION *act ;
{
	BODYPARAM		*bodyp ;
	OBJECT		*body ;

	bodyp = &act->bodyp ;
	body = act->body ;

//printf("CorpsMoveTarget act->ctrl->map[%x]\n",act->ctrl->map) ;

    GM_CurrentMap = act->ctrl->map ;

    GM_MoveTarget( &act->bodyp.deftrg, &act->ctrl->mov ) ;

	/* Level 0 */
    GM_MoveTarget2( &( bodyp->def_child1[ PTARGET_HEAD ] ), &(BODYWORLD( body, HUMAN21_ATAMA )) ) ;
	/* Level 2 */
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARMR1 ] ), &(BODYWORLD( body, HUMAN21_MIGI_UDE1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARMR2 ] ), &(BODYWORLD( body, HUMAN21_MIGI_UDE2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARML1 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_UDE1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARML2 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_UDE2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGR1 ] ), &(BODYWORLD( body, HUMAN21_MIGI_ASHI1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGR2 ] ), &(BODYWORLD( body, HUMAN21_MIGI_ASHI2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGL1 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_ASHI1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGL2 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_ASHI2 )) ) ;
	/* Level 3 */
    GM_MoveTarget2( &( bodyp->def_child3[ PTARGET_BODY ] ), &(BODYWORLD( body, HUMAN21_MUNE )) ) ;

}

static void FloorThrough( ACTION *act )
{
	act->ctrl->step = DG_ZeroVector ;
	if ( act->bodyp.capture.capture != NULL ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
		act->bodyp.capture.capture = NULL ;	
		act->bodyp.capture.flag &= ~CAPTURE_FREE ;
	}
	AT_SetMode( act, ActCorpDown ) ;
}

static	void CorpSetPosition( act, mov )
ACTION	*act ;
FVECTOR	*mov ;
{
#if 1	//00.10.1
    static FVECTOR 	Shift[] = {
	{ 0.0F, -382.115F, 422.438F },	/* 仰向け足 */
	{ 0.0F, -113.487F, 291.057F },	/* 仰向け頭 */
	{ 0.0F, -294.030F, 463.501F },	/* うつぶせ足 */
	{ -35.0F, 0.0F, 372.0F }	/* うつぶせ頭 */ /* いらない */
    } ;
#else
    static FVECTOR 	Shift[] = {
	{ -10.0F, -465.0F, 263.0F },	/* 仰向け足 */
	{ 60.0F, -68.0F, 400.0F },	/* 仰向け頭 */
	{ -6.7F, -430.0F, 516.0F },	/* うつぶせ足 */
	{ -35.0F, 0.0F, 372.0F }	/* うつぶせ頭 */ /* いらない */
    } ;
#endif
    CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov2, *sft ;
    float	     	len ;

	cap = &(act->bodyp.capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;
	/* 当たりチェック高さはプレイヤーと同じにする */
//	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;
	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				sft = Shift + 1 ;
		    } else {
				sft = Shift + 1 ;
		    }
	} else {
	    if ( act->down_s == DownFront || act->down_s == DownWall ) {
			sft = Shift + 0 ;
	    } else {
			sft = Shift + 2 ;
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
    static FVECTOR 	Shift[] = {
#if 1
		/* モーションが始まって１６フレーム目のシフト座標 */
	{ 0.0F, -421.119F, 1192.692F },	/* 仰向け足 */
	{ 26.483F, -393.755F, 940.002F },	/* 仰向け頭 */
	{ 0.0F, -346.867F, 1192.629F },	/* うつぶせ足 */
	{ 45.761F, -317.266F, 946.003F },	/* うつ伏せ頭 */
#else
	{ -10.0F, -465.0F, 303.0F },	/* 仰向け足 */
	{ 60.0F, -68.0F, 440.0F },	/* 仰向け頭 */
	{ -6.7F, -430.0F, 566.0F },	/* うつぶせ足 */
#endif
    } ;
    CAPTURE_TARGET	*cap ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov2, *sft ;
    float	     	len ;

	cap = &(act->bodyp.capture) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;
	/* 当たりチェック高さはプレイヤーと同じにする */
//	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;

	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				sft = Shift + 1 ;
		    } else {
				sft = Shift + 3 ;
		    }
	} else {
	    if ( act->down_s == DownFront || act->down_s == DownWall ) {
			sft = Shift + 0 ;
	    } else {
			sft = Shift + 2 ;
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
int 	time ;
{
	FVECTOR	mov, to ;
	CONTROL	*ctrl ;
	
	ctrl = act->ctrl ;
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	CorpSetPositionNear( act, &mov ) ;
//	CorpSetPosition( act, &mov ) ;
	
	to.vx = GV_NearTimeF( act->ctrl->mov.vx, mov.vx, time ) ;
	to.vz = GV_NearTimeF( act->ctrl->mov.vz, mov.vz, time ) ;

	/* 持ち上げ時 */
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
int 	time ;
{
	FVECTOR	mov, to ;
	CONTROL	*ctrl ;
	int		pose ;
	
	ctrl = act->ctrl ;
	act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	CorpSetPosition( act, &mov ) ;

	/* 持ち上げた後 */
	pose = 0 ;
	if ( (act->c_motion_num[0] == EM_dead_carry_leg_idle_f) ||
		 (act->c_motion_num[0] == EM_dead_carry_leg_start_f) ||
		 (act->c_motion_num[0] == EM_dead_carry_leg_walk_f) ) {
		pose = 1 ;	/* 階段補正反対 */
	} else if ( (act->c_motion_num[0] == EM_dead_carry_leg_idle_b) ||
		 (act->c_motion_num[0] == EM_dead_carry_leg_walk_b) ) {
		pose = 2 ;	/* 階段補正反対 */
	}
	if ( time >= 0 ) {
		if ( act->c_motion_num[0] == EM_dead_carry_leg_idle_f || 
			act->c_motion_num[0] == EM_dead_carry_leg_walk_f ) {
		    act->ctrl->turn.vy = act->ctrl->rot.vy = 
			act->bodyp.capture.capture->ctrl->rot.vy + 2048 ;
		} else if (
			act->c_motion_num[0] == EM_dead_carry_body_idle ||
			act->c_motion_num[0] == EM_dead_carry_leg_idle_b ||
			act->c_motion_num[0] == EM_dead_carry_body_walk ||
			act->c_motion_num[0] == EM_dead_carry_leg_walk_b ){
		    act->ctrl->turn.vy = act->ctrl->rot.vy = 
			act->bodyp.capture.capture->ctrl->rot.vy ;	    
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
		
		pl = act->bodyp.capture.capture->ctrl ;
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

	cap = &(act->bodyp.capture) ;
	if ( cap->flag & CAPTURE_FREE ) { /* ひきずられ終わり */
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
		AT_SetMode( act, ActCorpDown ) ;
	    return 1 ;
	}
	if ( cap->flag & CAPTURE_LOCKER ) { /* ロッカー入れられる */
		CorpSetStep( act, 0 ) ;
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
		act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
		AT_SetMode( act, ActCorpIntoLocker ) ;
	    return 1 ;
	}

	return 0 ;
}

/*-------- ------------------*/
static	void ActCorp( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DEATH | ACT_STATUS_IK_DOWN ) ;

//printf("ActCorp %d \n",time ) ;
	if ( time == 0 ) {
printf("corps:down_s = %d \n",act->down_s ) ;
		if ( act->down_s == DownFront ) {
			AT_SetAction( act, 0, EM_dead_idle_f, 0, MOTION_MASK_FULL, 0 ) ;
		} else if ( act->down_s == DownWall ) {
			AT_SetAction( act, 0, EM_dam_wall_down_p, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
			AT_SetAction( act, 0, EM_dam_out_pause, 0, MOTION_MASK_FULL, 0 ) ;
		}
#if 1
	    act->ctrl->turn.vy += act->body->m_ctrl->rot_correct ;
	    act->ctrl->rot.vy = act->ctrl->turn.vy ;
	    act->body->m_ctrl->rot_correct = 0 ;
#endif
		SET_FLAG( act->ctrl->skip_flag, CTRL_SKIP_HZX ) ;
	}
	
	if ( FloorCheck( act ) ) {
		UNSET_FLAG( act->ctrl->skip_flag, CTRL_SKIP_HZX ) ;
		CorpsMoveTarget( act ) ;
	} else {
		act->ctrl->step = DG_ZeroVector ;
		SET_FLAG( act->ctrl->skip_flag, CTRL_SKIP_HZX ) ;
	}

	if ( act->CheckDamage( act ) ) {
	    return ;
	}
}

static	void ActCorpDamage( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActCorpDamage %d \n",time ) ;
	AT_SetActStatus( act, ACT_STATUS_DEATH|ACT_STATUS_IK_DOWN|ACT_STATUS_DAMAGE ) ;
	act->ctrl->step = DG_ZeroVector ;

	if ( time == 0 ) {
	    AT_SetActStatus( act, ACT_STATUS_IK_PIKU ) ;
		act->ctrl->skip_flag &= ~CTRL_SKIP_HZX ;	/* あたりチェック開始 */
	}

	CorpsMoveTarget( act ) ;

	if ( act->CheckDamage( act ) ) {
	    return ;
	}

	if ( time > COUNT_VMODE(12) ) {
	    AT_SetMode( act, ActCorp ) ;
	}
}

static	void ActCorpWakeup( act, time )
ACTION	*act ;
int		time ;
{
	int	left ;

	AT_SetActStatus( act, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_WAKEUP ) ;

    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		AT_SetActStatus( act, ACT_STATUS_IK_FOOT ) ;
	} else {
		AT_SetActStatus( act, ACT_STATUS_IK_HAND ) ;
	}


	if ( time == 0 ) {
		FVECTOR	vec ;
		int		dir ;

		_sceVu0SubVector(  &vec, &act->bodyp.capture.capture->ctrl->mov, &act->ctrl->mov ) ;
		dir = _FVecDir2( &vec ) ;

		if ( act->down_s == DownFront ) {
		    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
				AT_SetAction( act, 0, EM_dead_carry_body_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		} else if ( act->down_s == DownWall ) {
		    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
				AT_SetAction( act, 0, EM_dead_carry_body_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir + 2048 ;
				act->tmp_time = COUNT_VMODE(50) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_start_f, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(50) ;
		    }
		} else {
		    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
				AT_SetAction( act, 0, EM_dead_carry_body_start_b, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
			    act->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(90) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_start_b, 0, MOTION_MASK_FULL, ACT_INTERP_SLOWLY ) ;
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
#if 1
	if ( time < 16 ) {
		if ( CorpSetStepNear( act, 16-time ) ) return ;
	} else {
		left = KR_LeftMotion( &act->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
		if ( CorpSetStep( act, left ) ) return ;
	}
#else
	if ( time < 16 ) CorpSetStepNear( act, 16-time ) ;

	left = KR_LeftMotion( &act->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
	if ( left < 16 ) CorpSetStep( act, left ) ;
#endif
//	else			AT_SetActStatus( act, ACT_STATUS_WAKEUP ) ;	/* 途中までＩＫしない */

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		AT_SetMode( act, ActCorpTake ) ;
		return ;
	}
}

static	void ActCorpTake( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP ) ;
    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		AT_SetActStatus( act, ACT_STATUS_IK_FOOT ) ;
	} else {
		AT_SetActStatus( act, ACT_STATUS_IK_HAND ) ;
	}


	if ( time == 0 ) {
//		AT_SetAction( act, 0, EM_dead_carry_body_idle, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		    AT_SetAction( act, 0, EM_dead_carry_body_idle, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    act->ctrl->turn.vy += act->body->m_ctrl->rot_correct ;
		    act->ctrl->rot.vy = act->ctrl->turn.vy ;
		    act->body->m_ctrl->rot_correct = 0 ;
//printf( "%d\n", act->body->m_ctrl->rot_correct ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_idle_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_idle_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
		act->ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;	/* 床の当たりチェックしない */
	}
	if ( CorpCaptureCheck( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		if ( CorpSetStep( act, -1 ) ) return  ;
	} else {
		if ( CorpSetStep( act, 0 ) ) return  ;
	}

	if ( act->bodyp.capture.flag & CAPTURE_MOVE ) {
	    AT_SetMode( act, ActCorpMove ) ;
	}
}

static	void ActCorpMove( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP ) ;
    if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		AT_SetActStatus( act, ACT_STATUS_IK_FOOT ) ;
	} else {
		AT_SetActStatus( act, ACT_STATUS_IK_HAND ) ;
	}


	if ( time == 0 ) {
//		AT_SetAction( act, 0, EM_dead_carry_body_walk, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
		    AT_SetAction( act, 0, EM_dead_carry_body_walk, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_walk_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_walk_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}
	}

	if ( CorpCaptureCheck( act ) ) {
		return ;
	}

	if ( CorpSetStep( act, 0 ) ) return ;

	if ( !( act->bodyp.capture.flag & CAPTURE_MOVE ) ) {
	    AT_SetMode( act, ActCorpTake ) ;
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

//printf("m [%f] w [%f] \n", mat.m[3][1], w->m[3][1] ) ;
	if ( mat.m[3][1] > w->m[3][1] ) {
		return 1 ;
	}

	return 0 ;
}


static	void ActCorpDown( act, time )
ACTION	*act ;
int		time ;
{
//printf("ActCorpDown %d \n",time ) ;
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_IK_DOWN|ACT_STATUS_TARGET_SKIP ) ;
	AT_SetActStSt( act, ACT_STST_UNDER_NEARCHECK ) ;

	if ( time == 0 ) {
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
			AT_SetAction( act, 0, EM_dead_carry_body_end, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_end_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_end_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
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
		float n ;

		n = ( act->bodyp.capture.flag & CAPTURE_HEAD )? 1.0f : 2.0f ;
		KRTH_PutItemProc( &act->item, &act->ctrl->mov, *(act->ene_status), n ) ;
	}
#endif

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
#if 1
	    if ( FrontCheck( act ) ) {
			act->bodyp.capture.flag = CAPTURE_FRONT ; /* 一度つかまれると仰向け */
			act->down_s = DownFront ;
		} else {
			act->bodyp.capture.flag = CAPTURE_BACK ;
			act->down_s = DownBack ;
		}
#else
	    if ( cap->flag & CAPTURE_HEAD ) {
			act->bodyp.capture.flag = CAPTURE_FRONT ; /* 一度つかまれると仰向け */
			act->down_s = DownFront ;
	    } else {
			if ( act->down_s == DownFront || act->down_s == DownWall ) cap->flag = CAPTURE_FRONT ;
			else                            cap->flag = CAPTURE_BACK ;
			if ( act->down_s == DownWall ) act->down_s = DownFront ;
	    }
#endif
	    AT_SetMode( act, ActCorp ) ;
	    return ;
	}
}


static	void ActCorpIntoLocker( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_LOCKER ) ;
	act->ctrl->skip_flag |= CTRL_SKIP_HZX ;

	if ( time == 0 ) {
		if ( act->bodyp.capture.flag & CAPTURE_HEAD ) {
			AT_SetAction( act, 0, EM_dead_carry_body_put_locker,0,MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		} else {
		    if ( act->down_s == DownFront || act->down_s == DownWall ) {
				AT_SetAction( act, 0, EM_dead_carry_leg_put_locker_f, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    } else {
				AT_SetAction( act, 0, EM_dead_carry_leg_put_locker_b, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		    }
		}

//		act->ctrl->skip_flag &= ~CTRL_SKIP_HZX ;	/* あたりチェック開始 */

		ReSetSlopeRotX( act->ctrl ) ;
	}

	if ( !(act->bodyp.capture.flag & CAPTURE_LOCKER) ) { /* ロッカー入れられる */
	    AT_SetMode( act, ActCorpDownLocker ) ;
	    return ;
	}

	if ( ENE_GameStatus & ENE_GMSTATUS_TOILET ) {
		if ( act->c_motion_num[0] == EM_dead_carry_body_put_locker ) {
			if ( time>74 ) {
			    AT_SetMode( act, ActCorpIntoToilet ) ;
			    return ;
			}
		} else if ( act->c_motion_num[0] == EM_dead_carry_leg_put_locker_f ) {
			if ( time>130 ) {
			    AT_SetMode( act, ActCorpIntoToilet ) ;
			    return ;
			}
		} else if ( act->c_motion_num[0] == EM_dead_carry_leg_put_locker_b ) {
			if ( time>138 ) {
			    AT_SetMode( act, ActCorpIntoToilet ) ;
			    return ;
			}
		}
	} else {
		if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
			act->act_end = 1 ;
		    AT_SetMode( act, ActCorpsStandLocker ) ;
		    return ;
		}
	}
}

static	void ActCorpIntoToilet( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP|ACT_STATUS_LOCKER  ) ;

	AT_SetActStatus( act, ACT_STATUS_SEG_OFF|ACT_STATUS_FLR_OFF) ;
//	AT_SetActStatus( act, ACT_STATUS_SEG_OFF) ;

	if ( time == 0 ) {
		if ( (act->c_motion_num[0] == EM_dead_carry_leg_put_locker_f) ) {
			AT_SetAction( act, 0, EM_dead_carry_put_toilet, 0, MOTION_MASK_FULL, 0 ) ;
		    act->ctrl->turn.vy += act->body->m_ctrl->rot_correct ;
		    act->ctrl->rot.vy = act->ctrl->turn.vy ;
		    act->body->m_ctrl->rot_correct = 0 ;
	    } else {
			AT_SetAction( act, 0, EM_dead_carry_put_toilet,0,MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
		}
	}

	if ( !(act->bodyp.capture.flag & CAPTURE_LOCKER) ) { /* ロッカー出る */
	    AT_SetMode( act, ActCorpDownLocker ) ;
	    return ;
	}

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
	    AT_SetMode( act, ActCorpsStandLocker ) ;
	    return ;
	}
}

void ActCorpsStandLocker( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_TARGET_SKIP|ACT_STATUS_LOCKER  ) ;
	if ( !(ENE_GameStatus & ENE_GMSTATUS_TOILET) ) {
		AT_SetActStatus( act, ACT_STATUS_INVISIBLE ) ;
	}

	act->ctrl->step = DG_ZeroVector ;

	act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
	if ( time == 0 ) {
		if ( act->c_motion_num[0] == EM_dead_carry_leg_put_locker_f ) {
			AT_SetAction( act, 0, EM_dead_carry_locker_idle, 0, MOTION_MASK_FULL, 0 ) ;
		    act->ctrl->turn.vy += act->body->m_ctrl->rot_correct ;
		    act->ctrl->rot.vy = act->ctrl->turn.vy ;
		    act->body->m_ctrl->rot_correct = 0 ;
		} else {
			AT_SetAction( act, 0, EM_dead_carry_locker_idle, 0, MOTION_MASK_FULL, 0 ) ;
		}

//printf("corps:down_s = %d \n",act->down_s ) ;


//	    act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
	}

	if ( !(act->bodyp.capture.flag & CAPTURE_LOCKER) ) { /* ロッカー入れられる */
	    AT_SetMode( act, ActCorpDownLocker ) ;
	    return ;
	}
}

static	void ActCorpDownLocker( act, time )
ACTION	*act ;
int		time ;
{
	AT_SetActStatus( act, ACT_STATUS_DOWN|ACT_STATUS_TARGET_SKIP ) ;

	if ( time > COUNT_VMODE(140) ) {
		AT_SetActStatus( act, ACT_STATUS_IK_DOWN ) ;
//		act->ctrl->skip_flag &= ~CTRL_SKIP_HZX ;	/* あたりチェック開始 */
	} else {
		act->ctrl->skip_flag |= CTRL_SKIP_HZX ;
	}

	if ( time == 0 ) {
		AT_SetAction( act, 0, EM_dead_carry_locker_surprise, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}
	CorpsMoveTarget( act ) ;


	if ( (act->ctrl->mov.vy - act->ctrl->levels[0]) < 500 ) {
		SetSlopeRotX( act->ctrl ) ;
	}

#ifdef PUT_ITEM
	if ( time == COUNT_VMODE(130) ) {
		float n ;

		n = ( act->bodyp.capture.flag & CAPTURE_HEAD )? 1.0f : 2.0f ;
		KRTH_PutItemProc( &act->item, &act->ctrl->mov, *(act->ene_status), n ) ;
	}
#endif

	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
	    if ( FrontCheck( act ) ) {
			act->bodyp.capture.flag = CAPTURE_FRONT ; /* 一度つかまれると仰向け */
			act->down_s = DownFront ;
		} else {
			act->bodyp.capture.flag = CAPTURE_BACK ;
			act->down_s = DownBack ;
		}
	    AT_SetMode( act, ActCorp ) ;
	    return ;
	}
}

static int ChildTargetCheck( child, n )
TARGET	*child ;
int		n ;
{
	int i ;

	for( i=0; i<n; i++ ) {
		if ( TARGET_POWER & child->damaged ) {
			return i ;
		}
		child++ ;
	}
	return -1 ;
}

/* 全てのダメージフラグをクリア */
static void DamageFlagClear( ACTION *act )
{
	TARGET	*def ;
	int i ;
	
	def = &(act->bodyp.def_child1[0]) ;
	for( i=0; i<PTARGET_LEVEL1_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def++ ;
	}
	def = &(act->bodyp.def_child2[0]) ;
	for( i=0; i<PTARGET_LEVEL2_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def++ ;
	}
	def = &(act->bodyp.def_child3[0]) ;
	for( i=0; i<PTARGET_LEVEL3_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def++ ;
	}
	def = &(act->bodyp.deftrg) ;
	def->damaged = FLAG_CLEAR ;

	def = &(act->bodyp.pushtrg) ;
	def->damaged = FLAG_CLEAR ;

}

#if 0//no use
static void CaptureFlagClear( ACTION *act )
{
	if ( act->bodyp.capture.capture != NULL ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
		act->bodyp.capture.capture = NULL ;	
	}
	act->bodyp.capture.flag = 0 ;
}

static void DamageCaptureFlagClear( ACTION *act )
{
	DamageFlagClear( act ) ;
	CaptureFlagClear( act ) ;
}
#endif

static void _DamageCaptureFlagClear( ACTION *act )
{
	DamageFlagClear( act ) ;
	if ( act->bodyp.capture.capture != NULL ) {
		act->bodyp.capture.capture->flag |= CAPTURE_FREE ;
		act->bodyp.capture.capture = NULL ;	
		act->bodyp.capture.flag = 0 ;
	}
}

static void SetBlood( ACTION *act )
{
	extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
	extern void TAKABE_OozeBloodAdd( void *, int, FVECTOR *, float ) ;

	TARGET	*dam ;
	FMATRIX	*world ;

	dam = act->bodyp.damtrg ;
	world = &(BODYWORLD( act->body, act->bodyp.n_damobj )) ;
	GV_SetActorChild( act->w, NewBlood( world, &dam->hit, &dam->power->force, 0, 0 ) ) ;
	if ( act->oozeblood != NULL ) {
//printf("kroerrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");
		TAKABE_OozeBloodAdd( act->oozeblood, act->bodyp.n_damobj, &dam->hit, 50.0F ) ;
	}
}

static int PowerHitCheck( act )
ACTION	*act ;
{
	TARGET	*def ;
	int	child_num ;
	long64	weapon ;
	PTARGET_INFO	*pinfo ;

	def = &(act->bodyp.def_child1[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL1] ;
	if ( (child_num = ChildTargetCheck( def, 1 )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.n_damobj = pinfo->obj_num ;
		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
#ifdef DAM_BUG
		_DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif
		if ( weapon & WP_THROWG ) {
			return 0 ;
		} else if ( weapon & (WP_M92) ) {
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
		} else if ( weapon & (WP_BULLET) ) {
			SetBlood( act ) ;
		}

		return 1 ;
	}

	def = &(act->bodyp.def_child2[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL2] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL2_NUM )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.n_damobj = pinfo->obj_num ;
		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
#ifdef DAM_BUG
		_DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif

		if ( weapon & WP_THROWG ) {
			return 0 ;
		} else if ( weapon & (WP_M92) ) {
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
		} else if ( weapon & (WP_BULLET) ) {
			SetBlood( act ) ;
		}

		return 1 ;
	}

	def = &(act->bodyp.def_child3[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL3] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL3_NUM )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		act->bodyp.n_damobj = pinfo->obj_num ;
		def->weapon_type = 0 ;
		act->bodyp.damtrg = def ;
#ifdef DAM_BUG
		_DamageCaptureFlagClear( act ) ;
#else
		DamageCaptureFlagClear( act ) ;
#endif

		if ( weapon & (WP_THROWG|WP_WALLCRASH) ) {
			return 0 ;
		} else if ( weapon & (WP_M92) ) {
			GM_SeSetMode( SD_A_M_NINE01, &act->ctrl->mov, GM_SEMODE_BOMB ) ;
			ENE_SetNeedl( act->body, act->bodyp.n_damobj, &act->bodyp.damtrg->hit ) ;
		} else if ( weapon & (WP_BULLET) ) {
			SetBlood( act ) ;
		}

		return 1 ;
	}
	
	return 0 ;
}

static int	EnemyDamagePad( act )
ACTION	*act ;
{
    CAPTURE_TARGET	*cap ;
	TARGET	*def ;

	cap = &(act->bodyp.capture) ;
	def = &(act->bodyp.deftrg) ;
    if ( cap->capture != NULL ) { /* 捕まった */
		def->damaged = FLAG_CLEAR ;
//printf("korecorps [%d]\n",def->damaged ) ;
		DamageFlagClear( act ) ; /* 同時ダメージは捕まり優先なのでダメージクリア */
		AT_SetMode( act, ActCorpWakeup ) ;
	    AT_SetActStatus( act, ACT_STATUS_CAPTURE|ACT_STATUS_TARGET_SKIP) ;
		return 1 ;
	}

	if ( PowerHitCheck( act ) ){
//printf("power hit [%d]\n",def->damaged ) ;
		AT_SetMode( act, ActCorpDamage ) ;
		AT_SetActStatus( act, ACT_STATUS_DAMAGE|ACT_STATUS_TARGET_SKIP ) ;
		return 1 ;
	}

	return 0 ;
}

static	void	Action ( work )
Work	*work ;
{
	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	OBJECT		*body ;
	ACTION		*act ;

	entk = &work->enethink ;
	ctrl = &work->control ;
	body = &work->body ;
	act = &work->action ;

	/* 行動処理 */
	body->flag = 0 ;				/* オブジェクトフラグ初期化 */

	AT_Action( act ) ;

	if ( !(ctrl->skip_flag & CTRL_SKIP_FLR_CHECK) ) {
//		ctrl->height = body->height ;
		if ( ctrl->grounded & 1 ) ctrl->step.vy = 0.0F ;
		ctrl->step.vy -= 64.0F ;
	} else {
		/* Y移動量はモーションに任せる */
		ctrl->step.vy = act->body->height - act->old_body_height ;
	}

	if ( act->status & ACT_STATUS_TARGET_SKIP ) {
		AT_SetTargetClass( act, TARGET_SKIP ) ;
	} else {
		AT_UnSetTargetClass( act, TARGET_SKIP ) ;
	}

	if ( act->status & ACT_STATUS_INVISIBLE ) {
		SET_FLAG( act->body->objs->flag, DG_FLAG_INVISIBLE ) ;
		SET_FLAG( entk->weapon->objs->flag, DG_FLAG_INVISIBLE ) ;
	} else {
		UNSET_FLAG( act->body->objs->flag, DG_FLAG_INVISIBLE ) ;
		UNSET_FLAG( entk->weapon->objs->flag, DG_FLAG_INVISIBLE ) ;
	}

}

/*-----	----*/

enum {
	TH1_WAIT, TH1_DAMAGE, TH1_TAKE, TH1_MOVE, TH1_ROTTEN, TH1_ROTTEN_WAIT
} ;

static void Think1_Wait( Work *work )
{
	ENETHINK	*entk ;
	ACTION		*act ;
	
	entk = &work->enethink ;
	act = &work->action ;

#if 1
	if ( work->newcorp.count <= 0 ) {
		_DamageCaptureFlagClear( act ) ;
		AT_SetTargetClass( &work->action, TARGET_SKIP ) ;
		work->newcorp.flag |= CORPS_ST_ROTTEN_START ;
		entk->think1 = TH1_ROTTEN ;
		entk->count3 = 0 ;
		return ;
	}
#endif
	if ( act->status & ACT_STATUS_DAMAGE ) {
		work->newcorp.flag |= CORPS_ST_ACT_CTRL ;
		entk->think1 = TH1_DAMAGE ;
		entk->count3 = 0 ;
		return ;
	} else if ( act->status & ACT_STATUS_CAPTURE ) {
#ifdef	DYNAMIC_ON
		SetFlagDynamic( work, HZX_SEG_SKIP ) ;
#endif
		work->newcorp.flag |= CORPS_ST_ACT_CTRL ;
		entk->think1 = TH1_MOVE ;
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}

static void Think1_Damage( Work *work )
{
	ENETHINK	*entk ;
	ACTION		*act ;
	
	entk = &work->enethink ;
	act = &work->action ;

	if ( !(act->status & ACT_STATUS_DAMAGE) ) {
		work->newcorp.flag &= ~CORPS_ST_ACT_CTRL ;
		entk->think1 = TH1_WAIT ;
		entk->count3 = 0 ;
		return ;
	}
	if ( act->status & ACT_STATUS_CAPTURE ) {
#ifdef	DYNAMIC_ON
		SetFlagDynamic( work, HZX_SEG_SKIP ) ;
#endif
		entk->think1 = TH1_MOVE ;
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}

static void Think1_Move( Work *work )
{
	ENETHINK	*entk ;
	ACTION		*act ;

	entk = &work->enethink ;
	act = &work->action ;

	if ( act->status & ACT_STATUS_DOWN && act->act_end == 1 ) {
		work->newcorp.flag &= ~CORPS_ST_ACT_CTRL ;
		entk->think1 = TH1_WAIT ;
		entk->count3 = 0 ;
#ifdef	DYNAMIC_ON
		MoveDynamic( work ) ;
		UnSetFlagDynamic( work, HZX_SEG_SKIP ) ;
#endif

		return ;
	}
	entk->count3 ++ ;
}

static void Think1_Rotten( Work *work )
{
	ENETHINK	*entk ;
	ACTION		*act ;

	entk = &work->enethink ;
	act = &work->action ;
	if ( entk->count3 < COUNT_VMODE(60) ) {
		work->lights[1].m[3][0] -= 1.0F ;
		if ( work->lights[1].m[3][0] < 0 ) work->lights[1].m[3][0] = 0.0F ;
		work->lights[1].m[3][1] -= 1.0F ;
		if ( work->lights[1].m[3][1] < 0 ) work->lights[1].m[3][1] = 0.0F ;
		work->lights[1].m[3][2] -= 2.0F ;
		if ( work->lights[1].m[3][2] < 0 ) work->lights[1].m[3][2] = 0.0F ;
		DG_SetLightMatrix( work->body.objs, work->lights );
	}

	if ( entk->count3 > COUNT_VMODE(60) ) {
		extern void SearchAndFallAttachment_called(OBJECT *,int func_type,
									int delay_count,int rnd_delay_count) ;
		extern void *NewOldCorp( int, CONTROL *, OBJECT *, FMATRIX *, int ) ;
		int	size ;

		CP_FreeCorp( &work->newcorp ) ;
		work->newcorp.flag |= CORPS_ST_ROTTEN ;
		entk->think1 = TH1_ROTTEN_WAIT ;
		entk->count3 = 0 ;
		SearchAndKillAttachment_called(act->body) ;
#if 1
		if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH ) {
			int mode = 2;
			if ( ENE_BreakBody( act, mode, act->oozeblood ) != NULL ) {
				act->oozeblood = NULL ;
			}
		} else {
			size = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY ) ;
			if ( size > GV_MEM_EFFECT_LIMIT_SIZE ) {
				NewOldCorp( work->oldcorps_name, &work->control, &work->body, work->lights, act->name_id->weapon ) ;
#ifdef KP_WINDOWS
				if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH_OFF ) {
					GM_SeSetMode( SD_A_V_SHITAI, (FVECTOR *)& act->body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
				}
#endif
			}
		}
		GV_DestroyActor( work ) ;
#if 0	//subsutans kara //kowainode yame!
	{
		ENEMEM *deadenm ;

		deadenm = ENEMEM_DeadLoad( GM_CurrentStage, work->enethink.alive_name ) ;
		if ( deadenm != NULL ) {
			/* もし、敵兵メモリーに登録されていたら削除 */
			deadenm->stage = 0 ;
		}
	}
#endif
		work->enethink.alive_name = 0 ;
#else
		CP_SetOldCorp( &work->oldcorp, &work->control.mov ) ;
		CP_PutOldCorp( &work->oldcorp ) ;
#endif
		return ;
	}

	entk->count3 ++ ;
}

static void StateManager( Work *work )
{
	ENETHINK	*entk ;

	entk = &work->enethink ;
	switch( entk->think1 ) {
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
	ENETHINK	*entk  ;
	ACTION	*act ;

	entk = &work->enethink ;
	act = entk->act ;


	if ( work->newcorp.flag & CORPS_ST_ACT_CTRL || !(work->new_ik_flag) ) {
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
			if ( act->bodyp.n_damobj == HUMAN21_MUNE || act->bodyp.n_damobj == HUMAN21_ATAMA 
			|| act->bodyp.n_damobj == HUMAN21_KOSHI || act->bodyp.n_damobj == HUMAN21_ONAKA ) {
				TAKABE_UtilPuppetIK_Piku( act->new_ik, -1 );
			} else {
				TAKABE_UtilPuppetIK_Piku( act->new_ik, act->bodyp.n_damobj );
			}
		}
		work->new_ik_flag = TAKABE_ActPuppetIK( act->new_ik );
	}


}

static void ActControl( Work *work )
{
	CONTROL	*ctrl ;
	ACTION	*act ;

	ctrl = &work->control ;
	act = &work->action ;

	if ( !(work->newcorp.flag & CORPS_ST_ROTTEN) ) {
		act->old_body_height = act->body->height ;
#ifdef TAKABE_IK
		if ( work->newcorp.flag & CORPS_ST_ACT_CTRL ||
			!(work->new_ik_flag) ) {
#else
		if ( work->newcorp.flag & CORPS_ST_ACT_CTRL ) {
#endif
			/* 倒れたときの低い床抜けチェック */
			if ( act->status_status & (ACT_STST_UNDER_NEARCHECK) ) {
				GM_ConfigControlNearCheck2( ctrl, 195 ) ;
			}

			GM_ActMotion( &work->body );

			/* 階段でめり込み防止 */
#if 0
			if ( act->ctrl->flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
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
#else
			ctrl->height = act->body->height ;
#endif

			GM_ActControl( &work->control ) ;

#ifndef NO_KANO_IK
			if ( work->action.status & ACT_STATUS_WAKEUP ) {
				DummyFreeCorpse( work->ik ) ;
				work->enethink.tmp_time = 0 ;
			} else if ( work->action.status & ACT_STATUS_IK_DOWN ) {
				if ( work->enethink.tmp_time ) {
					work->enethink.tmp_time = AfterJumpCorpse( work->ik ) ;
				} else {
					FreeCorpse( work->ik ) ;
				}
			} else if ( work->action.bodyp.capture.flag & CAPTURE_HEAD ) {
				PullCorpseWithShoulder( work->ik ) ;
				work->enethink.tmp_time = 0 ;
			} else {
				PullCorpseWithLegs( work->ik ) ;
				work->enethink.tmp_time = 0 ;
			}
#endif
			GM_ActObject2( &work->body );
			SET_FLAG( work->body.objs->flag, DG_FLAG_SHADOWMAKE) ;/* スポットライト影ＯＮ */
		} else {
#ifndef NO_KANO_IK
			if ( work->enethink.tmp_time ) {
			    GM_CurrentMap = work->control.map ;
				DG_SetPos( &(work->body.objs->world) ) ;
				work->enethink.tmp_time = AfterJumpCorpse( work->ik ) ;
				GM_ActObject2( &work->body );
			} else {
				DummyFreeCorpse( work->ik ) ;
			}
#endif
			UNSET_FLAG( work->body.objs->flag, DG_FLAG_SHADOWMAKE) ;/* スポットライト影OFF */
		}
#ifdef TAKABE_IK
		NewSetIk( work ) ;
#endif

		DG_GetLightMatrix( &work->control.mov, work->lights );

#ifdef DEBUG_MODE
		if( GM_DebugModeEnable ){
			ENETHINK	*entk ;
			
			entk = &work->enethink ;
			if ( entk->com->status & CMST_ENEMY_BODY_LIGHT ) {
				entk->lights[1].m[3][0] = 128 ;
				entk->lights[1].m[3][1] = 128 ;
				entk->lights[1].m[3][2] = 128 ;
			}
		}
#endif
	}

}

static void ActStatusCheck( Work *work )
{
	ACTION	*act ;
	
	act = &work->action ;
#ifndef NO_KANO_IK
	if ( act->status & ACT_STATUS_IK_PIKU ) {
		if ( act->bodyp.n_damobj == HUMAN21_MUNE ) {
			JumpFreeCorpse( work->ik ) ;
		} else {
			JumpFreeCorpse2( work->ik,act->bodyp.n_damobj) ;
		}
		if ( act->bodyp.n_damobj == HUMAN21_ATAMA ) {
			work->enethink.sw.eye_anim = 6 ;
		}
		work->enethink.tmp_time = 1 ;
	}
#endif
	if ( act->status & ACT_STATUS_CAPTURE ) {
		SET_FLAG( work->newcorp.flag, CORPS_ST_CAPTURE ) ;
	} else {
		UNSET_FLAG( work->newcorp.flag, CORPS_ST_CAPTURE ) ;
	}
}

#define ROTTEN_TRAP	(13434271)	/* CORPROTTEN */
static int TrapCheck( Work *work )
{
    static u_int     	Traps[] = { ROTTEN_TRAP } ;
    int			res ;

    res = GM_CheckInsideTrap( &work->control, Traps, 1, 0 ) ;
    if ( res & 1 ) return 1 ;
    return 0 ;
}

static void CorpsMain( Work *work )
{
	ActControl( work ) ;

	ENE_PlayerOnCorp( &work->control.mov ) ;
	
	/* 状態管理 */
	StateManager( work ) ;

	if ( !(work->newcorp.flag & CORPS_ST_ROTTEN_START) ) {
		CONTROL	*ctrl ;
		Action( work ) ;
		ActStatusCheck( work ) ;

		/* 後処理 */
		ctrl = &work->control ;

		if ( !(work->action.bodyp.type & ENE_TYPE_NO_ROTTEN) ) {
			if ( (GM_AlertMode == ALERT_MODE_ALERT) ||
				TrapCheck( work ) ) {
				work->newcorp.count -= STEP_VMODE(128) ;
			} else {
				work->newcorp.count -- ;
			}
			if ( work->newcorp.count < 0 ) work->newcorp.count = 0 ;
		}
	}
}

static int KillCheck( Work *work )
{
	if ( work->newcorp.flag & CORPS_ST_KILL ) {
		SearchAndKillAttachment_called(&(work->body)) ;
		CP_FreeCorp( &work->newcorp ) ;
		GV_DestroyActor( work ) ;
#if 1	//subsutans kara
	{
		ENEMEM *deadenm ;

		deadenm = ENEMEM_DeadLoad( GM_CurrentStage, work->enethink.alive_name ) ;
		if ( deadenm != NULL ) {
			/* もし、敵兵メモリーに登録されていたら削除 */
			deadenm->stage = 0 ;
		}
	}
#endif
		work->enethink.alive_name = 0 ;
		return 1 ;
	}
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ACTION		*act ;

		act = &work->action ;
		SearchAndKillAttachment_called(&(work->body)) ;
		CP_FreeCorp( &work->newcorp ) ;
		work->enethink.alive_name = 0 ;
		if ( GM_VRStatus & GM_VR_ENEMY_POLY_VANISH ) {
			if ( ENE_BreakBody( act, 4, act->oozeblood ) != NULL ) {
				act->oozeblood = NULL ;
			}
		} else {
			int size ;
			size = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY ) ;
			if ( size > GV_MEM_EFFECT_LIMIT_SIZE ) {
				extern void *NewOldCorp( int, CONTROL *, OBJECT *, FMATRIX *, int ) ;
				NewOldCorp( work->oldcorps_name, &work->control, &work->body, work->lights, act->name_id->weapon ) ;
			}
		}
		GV_DestroyActor( work ) ;
		return 1 ;
	}

	return 0 ;
}

static void Act( Work *work )
{
	if ( KillCheck( work ) ) return ;

	CorpsMain( work ) ;

	work->action.bodyp.faint_time = work->newcorp.count ;
#ifdef DYNAMIC_ON
{
	int i ;
    GM_CurrentMap = work->control.map ;
	for ( i=0; i<FLOOR_N; i++ ) {
		HZX_ViewDynamicFloor( work->d_floor[i], 4 ) ;
	}
}
#endif

}

static void Die( Work *work )
{
	ENETHINK	*entk ;
	
	entk = &work->enethink ;

	FreeUniqName( work->uni_name_no ) ;

	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;
#ifndef NO_KANO_IK
	ExitCorpseIK( work->ik ) ;
#endif
	ENE_FreeResources( entk ) ;
#ifdef DYNAMIC_ON
//	UnSetDynamic( work ) ;
#endif
}

static void Die2( Work *work )
{
	extern void ExitCorpseIK(void *work);
	ENETHINK	*entk ;
	
	entk = &work->enethink ;

#ifndef NO_KANO_IK
	ExitCorpseIK( entk->act->ik ) ;
#endif
	if ( !(GV_IsStageDestroy( work )) ) {
		if ( entk->act->oozeblood != NULL ) GV_DestroyActorQuick( entk->act->oozeblood ) ;
	}
#ifdef TAKABE_IK
	TAKABE_FreePuppetIK( entk->act->new_ik ) ;
#endif

	SearchAndKillAttachment_called( entk->act->body ) ;
	GM_FreeZoneIntrpt( &(entk->z_intrpt) ) ;
	GM_FreeHomingTrg( &(entk->hom) ) ;
    GM_FreeRadarControl( &( entk->sense.rctrl ) ) ;
    GM_FreeControl( entk->ctrl ) ;
    GM_FreeObject( entk->act->body ) ;
    GM_FreeTarget( &entk->act->bodyp.deftrg ) ;	/* 親だけで良い */
#ifdef DYNAMIC_ON
	UnSetDynamic( work ) ;
#endif
}
/*--- ------------------------------------------------------------*/
static int InitThink( work, pos, rot )
Work	*work ;
FVECTOR	*pos ;
SVECTOR	*rot ;
{
	work->enethink.think1 = TH1_WAIT ;
	work->control.mov = *pos ;
	work->control.rot = *rot ;

	GM_ConfigControlHzxHeight( &(work->control), 750.0F, work->control.mov.vy + 100.0F ) ;

	work->action.CheckDamage = EnemyDamagePad ;

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
	act->c_motion_num[0] = act_num ;
	GM_ConfigObjectAction( act->body, 0, act->motion_table[act_num], 0, MOTION_MASK_FULL, 0 );

	_CopyObjectWorld( body->objs, work->body.objs ) ;
	memcpy( &work->body.m_ctrl->abs_rots[0], &body->m_ctrl->abs_rots[0], sizeof( FVECTOR ) * 21 ) ;

	KR_GroupObject( &work->body, body->map_name ) ;

	DG_SetPos2( &work->control.mov, &work->control.rot );
	DG_PutObjs( work->body.objs ) ;
	DG_GetLightMatrix( &work->control.mov, work->lights );

	AT_SetMode( act, ActCorp ) ;
	act->down_s = down_s ;
}

static int SetDethMotionNoBody( work, motion_num, down_s )
Work	*work ;
int		motion_num ;
int		down_s ;
{
	ACTION	*act ;
	ENETHINK *entk ;
	int		act_num ;
	
	entk = &work->enethink ;
	act = &work->action ;

	act_num = motion_num ;
	act->c_motion_num[0] = act_num ;
	GM_ConfigObjectAction( act->body, 0, act->motion_table[act_num], 0, MOTION_MASK_FULL, 0 );

	DG_SetPos2( &work->control.mov, &work->control.rot );
	DG_PutObjs( work->body.objs ) ;

	/*高さ調整*/
	act->old_body_height = entk->act->body->height = 
		MT_GetMotionStartHeight( entk->act->body->m_ctrl, entk->act->c_motion_num[0] ) ;
    GM_ActMotion( entk->act->body ) ;

	DG_GetLightMatrix( &work->control.mov, work->lights );

	if ( motion_num  == EM_dead_carry_locker_idle ) {
		SET_FLAG( act->ctrl->skip_flag,  (CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK) ) ;
	    AT_SetMode( act, ActCorpsStandLocker ) ;
	} else {
		AT_SetMode( act, ActCorp ) ;
	}
	act->down_s = down_s ;

	return entk->act->body->height ;
}

/* 引き摺られ用 */
/* ターゲットコールバック関数 */
static	void	ChildTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	ENETHINK	*entk ;

	entk = ( ENETHINK * )ptr ;

    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = off->power->force ;
			GM_DamageTarget( off, def ) ;
			entk->act->bodyp.off_center = off->center ;
//printf("call def->weapon_type[%d]\n",def->weapon_type);
//printf("[%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}

static	void	InitTarget( work, down_s )
Work		*work ;
int		down_s ;
{
	BODYPARAM		*bodyp ;
	TARGET			*trg, *child1, *child2, *child3 ;
	POWER_TARGET	*power, *power_child1, *power_child2, *power_child3 ;
    TARGET	*t ;
    FVECTOR	size, offset ;
    int				i ;

    t = &( work->action.bodyp.deftrg ) ;
    size.vx = size.vz = 1100.0F ; size.vy = 300.0F ;
#if 0
    GM_SetTarget( t, TARGET_SEEK, 1, ENEMY_SIDE,
		 &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, 0 ) ;
#else
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_CHILD, 1, ENEMY_SIDE,
		 &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, 0 ) ;

	bodyp = &work->action.bodyp ;
	trg = &bodyp->deftrg ;
	power = &bodyp->power ;

    GM_SetPowerTarget( trg, power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;

	child1 = bodyp->def_child1 ;
	power_child1 = &bodyp->power_child1[0] ;
	child2 = bodyp->def_child2 ;
	power_child2 = &bodyp->power_child2[0] ;
	child3 = bodyp->def_child3 ;
	power_child3 = &bodyp->power_child3[0] ;

	for( i=0; i<1; i++ ) {
		size.vx = size.vz = 96.0F ; size.vy = 130.0F ;
		offset.vx = offset.vz = 0.0F ; offset.vy = 30.0F ;
		GM_SetTarget( child1, CHILD_TRG_FLAG, 1, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child1, 0 ) ;
		GM_SetPowerTarget( child1, power_child1, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child1, ChildTargCallBack, &work->enethink ) ;

//		NewTargetView( child1, 200, 34, 184 ) ;

		child1++ ;
		power_child1++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child1, PTARGET_LEVEL1_NUM, 1 ) ;
	for( i=0; i<PTARGET_LEVEL2_NUM; i++ ) {
		switch ( i ) {
			case PTARGET_ARMR1:
			case PTARGET_ARMR2:
			case PTARGET_ARML1:
			case PTARGET_ARML2:
			size.vx = size.vz = 75.0F ; size.vy = 150.0F ;
			offset.vx = offset.vz = 0.0F ; offset.vy = -130.0F ;
			break ;

			case PTARGET_LEGL1:
			case PTARGET_LEGR1:
			size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 200.0F ;
			offset.vx = offset.vz = -10.0F ; offset.vy = -200.0F ;
			break ;

			case PTARGET_LEGL2:
			case PTARGET_LEGR2:
			size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 250.0F ;
			offset.vx = offset.vz = -20.0F ; offset.vy = -250.0F ;
			break ;
		}
		GM_SetTarget( child2, CHILD_TRG_FLAG, 1, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child2, 0 ) ;
		GM_SetPowerTarget( child2, power_child2, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child2, ChildTargCallBack, &work->enethink ) ;

//		NewTargetView( child2, 34, 184, 200 ) ;

		child2++ ;
		power_child2++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child2, PTARGET_LEVEL2_NUM, 2 ) ;

	for( i=0; i<PTARGET_LEVEL3_NUM; i++ ) {
		size.vx = 200.0F ; size.vz = 200.0F ; size.vy = 300.0F ;
		offset.vx = offset.vz = 0.0F ; offset.vy = 0.0F ;
		GM_SetTarget( child3, CHILD_TRG_FLAG, 1, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child3, 0 ) ;
		GM_SetPowerTarget( child3, power_child3, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child3, ChildTargCallBack,  &work->enethink ) ;

//		NewTargetView( child3, 184, 200, 34 ) ;

		child3++ ;
		power_child3++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child3, PTARGET_LEVEL3_NUM, 3 ) ;
#endif
    GM_SetCaptureTarget( t, &( work->action.bodyp.capture ), 
			 &( work->control ), &( work->body ) ) ;
    GM_PutTarget( t ) ;
    t->class |= TARGET_DEAD ; /* 死んでいる */

	CorpsMoveTarget( &work->action ) ;

    work->control.skip_flag |= CTRL_SKIP_HZX ; /* 当たり見ない */
    if ( down_s == DownFront || down_s == DownWall ) { /* 仰向け */
		work->action.bodyp.capture.flag = CAPTURE_FRONT ;
    } else {	       	/* うつぶせ */
		work->action.bodyp.capture.flag = CAPTURE_BACK ;
    }

	{
		CONTROL		*ctrl ;
		OBJECT		*body ;
		ENETHINK	*entk ;
		
		entk = &work->enethink ;
		body = &work->body ;
		ctrl = &work->control ;
		/* body ctrl target 検索用にホーミングリストに追加 */
		GM_SetHomingTrg( &entk->hom, &(BODYWORLD( body, HUMAN21_MUNE )), body, 
			&ctrl->hzx_id, ctrl, (HOMING_SKIP|HOMING_DEAD) ) ;
		GM_SetHomingTrgTarget( &(entk->hom), &bodyp->deftrg ) ;
		GM_PutHomingTrg( &(entk->hom) ) ;
	}

//	NewTargetView( t, 32, 128, 64 ) ;
}

#define	CORPSNAME	(2583726) /* GV_StrCode("敵兵死体")*/

static int GetResources( work, name_id, pos, rot, body, motion_num, 
	down_s, corp, status, act, corp_flag, rotten_time )
Work 	*work ;
NAME_ID	*name_id ;
FVECTOR	*pos ;
SVECTOR	*rot ;
OBJECT	*body ;
int		motion_num ;
int		down_s ;
DG_OBJS	**corp ;
int		status ;
ACTION	*act ;
int		corp_flag ;
int		rotten_time ;
{
	extern void *NewSlingGun( OBJECT *, OBJECT *, int, int * ) ;
	ENETHINK	*entk ;
	float		body_height ;

	entk = &work->enethink ;
	entk->alive_name = act->ctrl->name ;
	entk->status = status | ENE_STATUS_DEAD ;
	{/*暫定ステータス修正 */
		if ( act->sw->n_sight == 2 ) {
			entk->status &= ~ENE_STATUS_NIGHT_SIGHT ;
			entk->status |= ENE_STATUS_NIGHT_SIGHT2 ;
		} else if ( act->sw->n_sight == 1 ) {
			entk->status |= ENE_STATUS_NIGHT_SIGHT ;
			entk->status &= ~ENE_STATUS_NIGHT_SIGHT2 ;
		}
	}

	entk->com =	COM_GetCommander() ;
	entk->name_id = *name_id ;
	entk->w = work->action.w = ( void * )work ;

	/* コントロールの初期化 */
	work->uni_name_no = GetUniqName( ) ;
	if ( work->uni_name_no >= 64 ) return -1 ;

	ENE_InitControl( entk, &work->control, (CORPSNAME|work->uni_name_no<<24) ) ;
printf("corp: ctrl->name [%d] \n",work->control.name ) ;

	/* オブジェクトの初期化 */
	entk->status |= ENE_STATUS_SHADOW_OFF ; /* 足跡影ＯＦＦ */
#ifndef CORP_MOT_CHECK
	ENE_InitObject( entk, &work->body, entk->name_id.body, &work->control, work->lights ) ;
#else
	ENE_InitObject( entk, &work->body, CORP_MODEL, &work->control, work->lights ) ;
#endif
	UNSET_FLAG( work->body.objs->flag, DG_FLAG_SHADOWMAKE) ;/* スポットライト影ＯＦＦ */
	{
		int r ;
		
		r=GV_Time%4 ;
		switch(r) {
			case 0:
			r = 5 ;
			break ;
			case 1:
			r = 4 ;
			break ;
			case 2:
			r = 3 ;
			break ;
			case 3:
			r = 0 ;
			break ;
		}
		entk->sw.eye_anim = r ;
	}

	/* アクション初期化 */
	ENE_InitAction( entk, &work->action, &work->control, &work->body, DEF_LIFE, DEF_FAINT, DEF_BLOOD ,NULL) ;
	entk->ctrl = &work->control ;
	entk->act = &work->action ;

#ifdef PUT_ITEM
	/* アイテムプロック受け渡し */
	entk->act->item = act->item ;
#endif

	/* 思考パラメータ初期化 */
	if ( InitThink( work, pos, rot ) < 0 ) {
		return -1 ;
	}

	body_height = act->body->height ;
	/* 死にモーション生成 */
	if ( body != NULL ) {
		SetDethMotion( work, body, motion_num, down_s ) ;
	} else {
		body_height = SetDethMotionNoBody( work, motion_num, down_s ) ;
	}

#ifdef TAKABE_IK
printf("corps takabe ik set\n");
	TAKABE_UtilPuppetIK_CopyStatus( work->action.new_ik, act->new_ik );
	work->new_ik_flag = 0 ;
#endif
#ifndef NO_KANO_IK
	work->ik = StartCorpseIK( &work->control, &work->body ) ;
	DummyFreeCorpse( work->ik ) ;
#endif
	entk->act->old_body_height = entk->act->body->height = body_height ;
	printf("corps height vy[%f]\n",act->body->height ) ;

	/* 手持ち武器の初期化 */
	UNSET_FLAG( entk->status, ENE_STATUS_GUNLIGHT ) ;
//	if ( entk->act->bodyp.type & ENE_TYPE_KATANA ) {
	if ( act->bodyp.type & ENE_TYPE_KATANA ) {
		entk->name_id.weapon = MDL_TNG_KANATA_G ;
	}
	ENE_InitWeapon( entk, &work->body, &work->weapon, entk->name_id.weapon, NULL ) ;

	entk->sw_gun = (SW_FLAG_VISIBLE|SW_FLAG_SWITCH2) ;
	if(entk->name_id.body == ENE_MDL_NAME_TNG){
		/*天狗刀は消したまま*/
		if ( act->bodyp.type & ENE_TYPE_KATANA ) {
			entk->sw_gun &= ~SW_FLAG_VISIBLE ;
		}
		/*天狗は無理矢理シングルテクスチャ*/
#if 0
		/*マルチテクスチャ解除*/
		work->body.flag &= ~DG_FLAG_MULTITEX ;
		work->body.objs->flag &= ~DG_FLAG_MULTITEX ;
		KR_UnsetAllObjsFlag( work->body.objs, DG_FLAG_MULTITEX ) ;
#endif
	}
	/* 引き摺られ用 */
	InitTarget( work, down_s ) ;
	work->control.turn = *rot ;
	work->control.rot = work->control.turn ;
	GM_ConfigControlMapID( &(work->control) ) ;

	/* ctrl->mapにセットされたあとターゲット移動 */
	if ( body == NULL ) {	/* ステージスタート死体 */
		GM_ActControl( entk->ctrl ) ;
		if ( motion_num != EM_dead_carry_locker_idle ) {
			CONTROL	*ctrl ;

			ctrl = entk->act->ctrl ;
			ctrl->turn.vx = ENE_GetGRotFromPos( &ctrl->mov, &ctrl->rot, 1000.0f, 
						ctrl->hzx_id, ctrl->hzx_check_type, ctrl->flr_flag ) ;
			ctrl->rot.vx = ctrl->turn.vx ;
			DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
printf("corps slope rot vx = [%d]\n",ctrl->rot.vx ) ;
		}
		GM_ActObject2( entk->ctrl->object );
	}
	CorpsMoveTarget( &work->action ) ;

	CP_SetCorp( &work->newcorp, &work->control, &work->control.addr, name_id->uniq_id, entk ) ;
	CP_PutCorp( &work->newcorp ) ;

	work->newcorp.count = rotten_time ;
	work->enethink.tmp_time = 0 ;

//	work->oldcorps_name = GV_StrCode("gbs_def") ;
	work->oldcorps_name = name_id->body ;

#ifdef TAKABE_IK
	if ( motion_num  == EM_dead_carry_locker_idle ) {
		SET_FLAG( work->action.bodyp.capture.flag, CAPTURE_LOCKER ) ;
		work->newcorp.flag |= CORPS_ST_ACT_CTRL ;
		entk->think1 = TH1_MOVE ;
		entk->count3 = 0 ;
	    ActCorpsStandLocker( &work->action, 0 ) ;
	} else {
		ActCorp( &work->action, 0 ) ;	/* モーションとアクションステータス、セット */
	}
#endif

#ifdef DYNAMIC_ON
	SetDynamic( work ) ;
#endif

	entk->act->head_blood = NULL ;
	if ( corp_flag & CORPS_FLAG_HEADBLOOD ) {
		extern void *NewHiTechFaceBlood( DG_OBJS *objs ) ;
		GV_SetActorChild( work, entk->act->head_blood = NewHiTechFaceBlood( entk->act->body->objs ) ) ;
	}

//	*corp = &work->body ;

/*呼び出し元の情報をコピー*/
	work->action.bodyp.type = act->bodyp.type ;

	return (0);
}

/* 敵兵用 */
Work *NewMakeCorps( name_id, pos, rot, body, motion_num, down_s, ooze, status, act, corp_flag )
NAME_ID	*name_id ;
FVECTOR	*pos ;
SVECTOR	*rot ;
OBJECT	*body ;
int		motion_num ;
int		down_s ;
void	*ooze ;
int		status ;
ACTION	*act ;
int		corp_flag ;
{
	extern void TAKABE_OozeBloodChangeObjs( void *, DG_OBJS * ) ;
	Work		*work ;
	DG_OBJS		*corp ;

	OPERATOR() ;

#ifdef CORP_MOT_CHECK
ooze = NULL ;
#endif
	corp = NULL ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, name_id, pos, rot, body, motion_num, down_s, 
				&corp, status, act, corp_flag, DEF_ROTTEN_TIME ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->action.oozeblood = ooze ;
		corp = work->body.objs ;

printf("corrrrrps: act->oozeblood[%x]\n",ooze);
		if ( ooze != NULL )	{ 
			/* 滲み血受け取る */
			TAKABE_OozeBloodChangeObjs( ooze, corp ) ;
		}
	}

	return work ;
}

Work *NewMakeCorpsDead( name_id, pos, rot, body, motion_num, down_s, ooze, status, act, corp_flag, rotten_time )
NAME_ID	*name_id ;
FVECTOR	*pos ;
SVECTOR	*rot ;
OBJECT	*body ;
int		motion_num ;
int		down_s ;
void	*ooze ;
int		status ;
ACTION	*act ;
int		corp_flag ;
int		rotten_time ;
{
	extern void *NewOozeBlood( DG_OBJS *, int ) ;
	Work		*work ;
	DG_OBJS		*corp ;

	OPERATOR() ;

#ifdef CORP_MOT_CHECK
ooze = NULL ;
#endif
	corp = NULL ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, name_id, pos, rot, body, motion_num, down_s, 
				&corp, status, act, corp_flag, rotten_time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->action.oozeblood = NewOozeBlood( work->body.objs, name_id->body ) ;
		corp = work->body.objs ;
	}

	return work ;
}

/* 汎用呼び出しモーションは gbs.mar を使用 */
/*--- ------------------------------------------------------------*/
static int GetResources2( work, name, body, pos, rot, model, down_s, ooze, uniq_id )
Work 	*work ;
int		name ;		/* コントロール名 */
OBJECT	*body ;		/* オブジェクト */
FVECTOR	*pos ;		/* 場所 */
SVECTOR	*rot ;		/* 方向 */
int		model ;		/* モデル名 */
int 	down_s ;	/* 仰向け １、うつ伏せ ２ */
void	*ooze ;		/* にじみ血 */
int		uniq_id ;	/* 個別ＩＤ */
{
	extern void TAKABE_OozeBloodChangeObjs( void *, DG_OBJS * ) ;
	extern void *NewSlingGun( OBJECT *, OBJECT *, int, int * ) ;
	DG_OBJS		*corp ;
	ENETHINK	*entk ;
	int			motion_num ;

	entk = &work->enethink ;
	entk->status = ENE_STATUS_DEAD|ENE_STATUS_NO_HEADMARK|ENE_STATUS_NO_EYEANIM|ENE_STATUS_NO_FINGER|ENE_STATUS_NO_VANIM ;
	entk->name_id.body = model ;

	entk->com =	COM_GetCommander() ;

	entk->w = work->action.w = ( void * )work ;

	/* コントロールの初期化 */
	ENE_InitControl( entk, &work->control, name ) ;

	/* オブジェクトの初期化 */
	entk->status |= ENE_STATUS_SHADOW_OFF ; /* 足跡影ＯＦＦ */
	ENE_InitObject( entk, &work->body, entk->name_id.body, &work->control, work->lights ) ;
	UNSET_FLAG( work->body.objs->flag, DG_FLAG_SHADOWMAKE) ;/* スポットライト影ＯＦＦ */
	entk->sw.eye_anim = 0 ;

	/* アクション初期化 */
	ENE_InitAction( entk, &work->action, &work->control, &work->body, DEF_LIFE, DEF_FAINT, DEF_BLOOD,NULL ) ;
	entk->ctrl = &work->control ;
	entk->act = &work->action ;

	/* 思考パラメータ初期化 */
	if ( InitThink( work, pos, rot ) < 0 ) {
		return -1 ;
	}

	/* 死にモーション生成 */
	motion_num = ( down_s == DownFront )? EM_dead_idle_f : EM_dam_out_pause ;
	SetDethMotion( work, body, motion_num, down_s ) ;
#ifndef NO_KANO_IK
	work->ik = StartCorpseIK( &work->control, &work->body ) ;
	DummyFreeCorpse( work->ik ) ;
#endif

	/* 引き摺られ用 */
	InitTarget( work, down_s ) ;
	work->control.turn = *rot ;
	work->control.rot = work->control.turn ;
	GM_ConfigControlMapID( &(work->control) ) ;

	CP_SetCorp( &work->newcorp, &work->control, &work->control.addr, uniq_id, entk ) ;
	CP_PutCorp( &work->newcorp ) ;

	work->newcorp.count = DEF_ROTTEN_TIME ;
	work->enethink.tmp_time = 0 ;

	work->action.oozeblood = ooze ;
	corp = work->body.objs ;

	if ( ooze != NULL )	{ 
		/* 滲み血受け取る */
		TAKABE_OozeBloodChangeObjs( ooze, corp ) ;
	}
	
	work->oldcorps_name = model ;

#ifdef DYNAMIC_ON
	SetDynamic( work ) ;
#endif

	return (0);
}

void *NewCorps( name, body, pos, rot, model, down_s, ooze )
int		name ;	/* コントロール名 */
OBJECT	*body ;	/* オブジェクト */
FVECTOR	*pos ;	/* 初期位置 */
SVECTOR	*rot ;	/* 初期方向 */
int	model ;		/* モデル名 */
int down_s ;	/* 仰向け １、うつ伏せ ２ */
void	*ooze ;	/* にじみ血 */
{
	Work		*work ;

	OPERATOR() ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {

		GV_SetActor( &( work->actor ), Act, Die2 ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources2( work, name, body, pos, rot, model, down_s, ooze, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return work ;
}


void *MekeNewStageCorps( ENETHINK *entk )
{
	ENEMEM *enm ;
	FVECTOR pos ;
	SVECTOR rot ;
	ACTION *act ;
	int		keep_mot, down_s ;

	enm = ENEMEM_DeadLoad( GM_CurrentStage, entk->ctrl->name ) ;
	if ( enm == NULL ) return NULL ;
printf("enemy:load dead enemy memory[%d] name[%d]\n",GM_CurrentStage, entk->ctrl->name ) ;
printf("dead load:stage[%d]\n",enm->stage) ;
printf("dead load:hzx_id[%d]\n",enm->hzx_id) ;
printf("dead load:name[%d]\n",enm->name) ;
printf("dead load:alive_name[%d]\n",enm->alive_name) ;
printf("dead load:x[%f]\n",enm->x) ;
printf("dead load:y[%f]\n",enm->y) ;
printf("dead load:z[%f]\n",enm->z) ;
printf("dead load:dir[%d]\n",enm->dir) ;
printf("dead load:status[%d]\n",enm->status) ;
printf("dead load:pose[%d]\n",enm->pose) ;
printf("dead load:time[%d]\n",enm->time) ;
printf("dead load:life[%d]\n",enm->life) ;
printf("dead load:faint[%d]\n",enm->faint) ;
printf("dead load:pbreak[%d]\n",enm->pbreak) ;
printf("dead load:item[%d]\n",enm->item) ;

	act = entk->act ;

	act->bodyp.life = 0 ;
	act->bodyp.faint = enm->faint ;
	act->item.c_proc = ENEMEM_ITEM1(enm->item) ;
	act->hold_item.c_proc = ENEMEM_ITEM2(enm->item) ;

	if ( enm->status != ENEMEM_ST_DEAD ) return NULL ;
	if ( enm->time < COUNT_VMODE(300) ) return NULL ;

	/* 場所移動 */
	pos.vx = enm->x ;
	pos.vy = enm->y ;
	pos.vz = enm->z ;
	rot.vx = 0 ;
	rot.vy = enm->dir ;
	rot.vz = 0 ;

	/* ポーズ */
	keep_mot = EM_dead_idle_f ;
	down_s = DownFront ;
	switch( enm->pose ) {
		case ENEMEM_PS_FRONT :
			keep_mot = EM_dead_idle_f ;
			down_s = DownFront ;
		break ;
		case ENEMEM_PS_BACK :
			keep_mot = EM_dam_out_pause ;
			down_s = DownBack ;
		break ;
		case ENEMEM_PS_WALL :
			keep_mot = EM_dam_wall_down_p ;
			down_s = DownWall ;
		break ;
		case ENEMEM_PS_LOCKER :
			keep_mot = EM_dead_carry_locker_idle ;
			down_s = DownFront ;
		break ;
		default :
			printf( " dead load pose err !!\n");
			ASSERT( 1 ) ;
		break ;
	}

	return NewMakeCorpsDead( act->name_id, &pos, &rot, NULL,
		keep_mot, down_s, NULL, *(act->ene_status), act, 0, enm->time ) ;
}


