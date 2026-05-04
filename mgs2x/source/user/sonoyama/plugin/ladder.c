//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   ladder.c
   
   はしごモード
   2000/03/22 M.Sonoyama
   $Id: ladder.c,v 1.1.1.3 2002/11/19 11:50:48 Yoshizawa1 Exp $
   */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX 
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

enum {
    Mladstill = 0,
    Mladstill_l,
    Mladup,
    Mladdown,
    Mladstart,
    Mladend,
	Mladdmg_r,
	Mladdmg_l,
	Mladfall_s_r,
	Mladfall_s_l,
	Mladfall_l,
	Mladfall_e,
	MAX_LADDER_MOTIONS
} ;

typedef	struct	{
    PL_PluginSet	plugin ;
    FVECTOR		start_mov ;
    FVECTOR		end_mov ;
    FVECTOR		center ;
    float		limit[ 2 ] ;
    short		start_dir ;
    short		end_dir ;
    int			flag ;
    u_char		motions[ ( MAX_LADDER_MOTIONS + 3 ) / 4 * 4 ] ;
} LADDER_SET ;

enum {
    LAD_FLAG_ACTIVE = 0x0001,
    LAD_FLAG_DOWN = 0x0002,
    LAD_FLAG_R_UP = 0x0010,
} ;

static	LADDER_SET	LadderSet ;
static	float		LadderHeight, LadderDiff ;
static	FVECTOR		LadderAdj ;
static	int			LadderNo ;

static	void	LadderStart( PlayerWork *, int ) ;
static	void	LadderEnd( PlayerWork *, int ) ;
static	void	LadderStill( PlayerWork *, int ) ;
static	void	LadderUp( PlayerWork *, int ) ;
static	void	LadderDown( PlayerWork *, int ) ;
static	void	LadderShrink( PlayerWork *, int ) ;
static	void	LadderFall( PlayerWork *, int ) ;

static	void	LadderCheckDamage( PlayerWork *work, long64 weapon_type, int dead ) ;

extern	void	*NewPadVibration( char *, int ) ;

/*------------------------------------------------------------------
  
  プレイヤー行動
  
  ------------------------------------------------------------------*/

#define	UP_RIGHT_UP	(1)
#define	UP_LEFT_UP     	(35)
#define	DOWN_RIGHT_UP	(1)
#define	DOWN_LEFT_UP	(40)

#define	LADDER_SPACE	(500.0F)

/* はしご高さセット */
static	void	ResetLadderHeight( work )
PlayerWork	*work ;
{
    LadderHeight = work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 1 ] + 50.0F ;
    LadderDiff = LadderHeight - work->control.mov.vy ;
//printf( "%f %f\n", LadderHeight, LadderDiff ) ;
}

static	void	SetLadderHeight( work, updown, flag )
PlayerWork	*work ;
int		updown, flag ;
{
    float	hand ;
    FVECTOR	mov ;

    if ( flag & LAD_FLAG_R_UP ) {
		hand = work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 1 ] + 50.0F ;	
    } else {
		hand = work->body.objs->objs[ HUMAN21_HIDARI_TE ].world.m[ 3 ][ 1 ] + 50.0F ;
    }
    LadderDiff = hand - work->control.mov.vy ;
    if ( updown == 0 ) {		/* up */
		LadderHeight += LADDER_SPACE ;
    } else {
		LadderHeight -= LADDER_SPACE ;
    }
    DG_COPY_VEC( &mov, &work->control.mov ) ;
    mov.vy = LadderHeight - LadderDiff ;
	//    GM_ResetControlPosition( &work->control, &mov ) ;
    LadderAdj.vy = mov.vy - work->control.mov.vy ;
}

static	void	SetLadderAdj( PlayerWork *work, int flag )
{
    float	hand ;
    FVECTOR	mov ;

    if ( flag & LAD_FLAG_R_UP ) {
		hand = work->body.objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ][ 1 ] + 50.0F ;	
    } else {
		hand = work->body.objs->objs[ HUMAN21_HIDARI_TE ].world.m[ 3 ][ 1 ] + 50.0F ;
    }
    LadderDiff = hand - work->control.mov.vy ;
    DG_COPY_VEC( &mov, &work->control.mov ) ;
    mov.vy = LadderHeight - LadderDiff ;
    LadderAdj.vy = mov.vy - work->control.mov.vy ;
}

static	void	SetLadderAdjXZ( PlayerWork *work, int flag )
{
    FVECTOR	diff, center ;
	FVECTOR	left = { 220.0F, 0.0F, 0.0F } ;

	DG_COPY_VEC( &center, &LadderSet.center ) ;
	if ( !( LadderSet.flag & LAD_FLAG_R_UP ) ) {
		DG_SetPos2( &center, &work->control.rot ) ;
		DG_PutVector( &left, &center, 1 ) ;
	}
    _sceVu0SubVector( &diff, &center, &work->control.mov ) ;
	LadderAdj.vx = diff.vx ;
	LadderAdj.vz = diff.vz ;
}

static	void	AdjustHeight( work )
PlayerWork	*work ;
{
    float	v ;
	
    v = LadderAdj.vy ;    
    LadderAdj.vy = GV_NearExp4F( LadderAdj.vy, 0.0F ) ;
    v = v - LadderAdj.vy ;
    work->control.mov.vy += v ;
}

static	void	_AdjustXZ( work )
PlayerWork	*work ;
{
    float	v ;
	
    v = LadderAdj.vx ;    
    LadderAdj.vx = GV_NearExp4F( LadderAdj.vx, 0.0F ) ;
    v = v - LadderAdj.vx ;
    work->control.mov.vx += v ;
    v = LadderAdj.vz ;    
    LadderAdj.vz = GV_NearExp4F( LadderAdj.vz, 0.0F ) ;
    v = v - LadderAdj.vz ;
    work->control.mov.vz += v ;
}

#if 0
/* モーション設定開始フレームあり */
static	void	SetAction3( work, action, time, interp )
PlayerWork	*work ;
int		time ;
int		action, interp ;
{
    int		rc ;

    if ( work->motion1 == action ) return ;
    GM_ConfigObjectAction( &( work->body ), 0, action,
						  time * ONE_CLOCK, 0xfffff, interp * ONE_CLOCK ) ;
    work->motion1 = action ;
    if ( ( rc = work->body.m_ctrl->rot_correct ) != 0 ) {
		work->control.turn.vy += rc ;
		work->control.rot.vy = work->control.turn.vy ;
    }
    work->root_diff = DG_ZeroVector ;
    /* 腰補完なし */
    if ( PL_Flag( FLAG_NO_WAIST_INTERP ) ) {
		work->body.m_ctrl->interp_flag &= ~0x01 ;
		PL_UnsetFlag( FLAG_NO_WAIST_INTERP ) ;
    }
}
#endif

/* 中心に戻す */
static	void	Centering( work )
PlayerWork	*work ;
{
    FVECTOR	diff, center ;
#ifdef PAL
	FVECTOR	left = { 250.0F, 0.0F, 0.0F } ;
#else
	FVECTOR	left = { 220.0F, 0.0F, 0.0F } ;
#endif
	DG_COPY_VEC( &center, &LadderSet.center ) ;
	if ( !( LadderSet.flag & LAD_FLAG_R_UP ) ) {
		DG_SetPos2( &center, &work->control.rot ) ;
		DG_PutVector( &left, &center, 1 ) ;
	}
    _sceVu0SubVector( &diff, &center, &work->control.mov ) ;
    diff.vy = work->root_diff.vy ;
    _sceVu0AddVector( &work->control.step, &work->control.step, &diff ) ;
    SetFlag( FLAG_NO_MOTION_STEP ) ;
	LadderAdj.vx = 0.0F ;
	LadderAdj.vz = 0.0F ;
}

/* カメラ位置セット */
static	inline	void	SetCamera( PlayerWork *work )
{
	FVECTOR		prev, now ;
	FVECTOR		cus = { 500.0F, 150.0F, 500.0F } ;

	if ( Status( PLAYER_WATCH ) ) {
		PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
		return ;
	} 
	DG_COPY_VEC( &prev, &GM_CameraTarget ) ;
	DG_COPY_VEC( &now, &work->control.mov ) ;
//	now.vy = PL_ObjHeight( work, HUMAN21_ATAMA ) ;
	GV_NearRangeVF( &prev, &now, &cus, 3 ) ;
	DG_COPY_VEC( &work->camera, &prev ) ;
}

/*----------------------------------------------------------------*/

/* はしごモード開始 */
static	void	LadderStart( work, time )
PlayerWork	*work ;
int		time ;
{
    LADDER_SET	*l ;
	int			end_flag ;

	end_flag = 0 ;
    SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | 
			FLAG_NO_IK | FLAG_NO_GRAVITY ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    l = &LadderSet ;
    if ( time == 0 ) {
		PL_LeaveSubject( work ) ;
		/* モーションファイルチェンジ */
		PL_ChangeMotionArc( work, PL_Force->marfile ) ;
		SetAction3( work, l->motions[ Mladstart ], 0, 0 ) ;
		SetFlag( FLAG_NO_MOTION_STEP_Y ) ;
		GM_SetMenuStatus( MENU_WEAPON_OFF ) ;
    }
#if 0	
	if ( work->ftime < 16 ) {
		printf( "[%d] : ", work->ftime ) ;
		DumpVec( &work->control.mov ) ;
	}
#endif
	SetCamera( work ) ;
#if 1
	if ( EndMotion( work ) ) {
		/* 強制位置あわせ */
		work->control.mov.vy -= 40.0F ;
	}
#endif
    if ( EndMotionAll( work ) ) {
		end_flag = 1 ;
		//	PL_UnsetInvincible( work ) ;
        UnsetStatus( PLAYER_FORCE ) ;
		SetMode( work, LadderStill ) ;
		work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
        DG_COPY_VEC( &l->center, &work->control.mov ) ;
        SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
        SetAction3( work, l->motions[ Mladup ], 0, 6 ) ;
        work->control.step.vy = 0.0F ;
        l->flag &= ~LAD_FLAG_DOWN ;
        l->flag |= LAD_FLAG_R_UP ;
		ResetAddress( work ) ;
        ResetLadderHeight( work ) ;
        LadderNo = 0 ;
		DG_COPY_VEC( &LadderAdj, &DG_ZeroVector ) ;
		PL_UnsetInvincible( work ) ;
		PL_DamageFunc = LadderCheckDamage ;
		GM_ResetMenuStatus( MENU_ITEM_OFF ) ;
    }
	work->control.step.vy = 0.0F ;
	PL_ExecForceActProc( work, FA_PROCMODE_LADDER_IN, time, end_flag ) ;
}

/* はしご終了 */
static	void	LadderEnd( work, time )
PlayerWork	*work ;
int		time ;
{
    LADDER_SET	*l ;
	int			end_flag ;

	end_flag = 0 ;
    SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | 
			FLAG_NO_IK | FLAG_NO_GRAVITY ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
    l = &LadderSet ;
    if ( time == 0 ) {
		PL_LeaveSubject( work ) ;
		work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
		SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
		SetAction3( work, l->motions[ Mladend ], 0, 6 ) ;
    }
	SetCamera( work ) ;
    if ( EndMotion( work ) ) {
		end_flag = 1 ;
		SetFlag( FLAG_FORCE_END ) ;
		PL_UnsetInvincible( work ) ;
        UnsetStatus( PLAYER_LADDER ) ;
		SetMode( work, PL_StillMode[ 0 ] ) ;
		PL_Force->e_turn = l->end_dir ;
		l->flag &= ~LAD_FLAG_ACTIVE ;
//		PL_Force->e_turn = -1 ;
		ResetAddress( work ) ;
		GM_ResetMenuStatus( MENU_MENU_OFF ) ;
		PL_DamageFunc = NULL ;
    }
	PL_ExecForceActProc( work, FA_PROCMODE_LADDER_OUT, time, end_flag ) ;
}

/* はしご静止 */
static	void	LadderStill( work, time )
PlayerWork	*work ;
int		time ;
{
    LADDER_SET	*l ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_IK | FLAG_CANNOT_PEEP ) ;
    SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_GRAVITY ) ;
//	SetFlag( FLAG_RECOVER_ENABLE ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
	PL_SetNoUseItemType( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ;

    l = &LadderSet ;
    if ( time == 0 ) {
		//	work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
        SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
        if ( l->flag & LAD_FLAG_R_UP ) {
			SetAction3( work, l->motions[ Mladstill ], 0, 12 ) ;
		} else {
			SetAction3( work, l->motions[ Mladstill_l ], 0, 12 ) ;
		}
		SetLadderAdj( work, l->flag ) ;
		SetLadderAdjXZ( work, l->flag ) ;
//printf( "%f\n", LadderHeight ) ;
    }
	SetCamera( work ) ;
    if ( Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		PL_SubjectTurn( work ) ;
    } else {
		if ( ( work->pad->status & PAD_U ) &&
			( work->control.mov.vy < ( l->limit[ 1 ] - LADDER_SPACE + 50.0F ) ) ) {
			SetMode( work, LadderUp ) ;
			Centering( work ) ;
		} else if ( ( work->pad->status & PAD_D ) &&
				   ( work->control.mov.vy > l->limit[ 0 ] + LADDER_SPACE - 50.0F ) ) {
			SetMode( work, LadderDown ) ;
			Centering( work ) ;
		}
    }
    work->control.step.vy = 0.0F ;
    AdjustHeight( work ) ;
    _AdjustXZ( work ) ;
}

/* はしご登る */
static	void	LadderUp( work, time )
PlayerWork	*work ;
int		time ;
{
    LADDER_SET		*l ;
    int			mtime, interp ;
	int			rc, lc ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_IK | FLAG_NO_GRAVITY ) ;
    SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_CANNOT_PEEP ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
	PL_SetNoUseItemType( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ;

    l = &LadderSet ;
    if ( time == 0 ) {
		//	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
		if ( l->flag & LAD_FLAG_DOWN ) {
			//	    SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
			l->flag &= ~LAD_FLAG_DOWN ;
		} 
		if ( l->flag & LAD_FLAG_R_UP ) {
			mtime = UP_RIGHT_UP ;
			interp = 6 ;
			Centering( work ) ;
		} else {
			mtime = UP_LEFT_UP ;
			interp = 6 ;
		}
        SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
		SetAction3( work, l->motions[ Mladup ], mtime, interp ) ;
    }
	SetCamera( work ) ;
	/* 小さいほうが先 */
	rc = PL_CheckMotionTime( work, UP_RIGHT_UP ) ;
	lc = PL_CheckMotionTime( work, UP_LEFT_UP ) ;
    if ( time != 0 && ( rc || lc ) ) {
		if ( rc ) {
			l->flag |= LAD_FLAG_R_UP ;
		} else {
			l->flag &= ~LAD_FLAG_R_UP ;
		}	
		SetLadderHeight( work, 0, l->flag ) ;
		if ( !( work->pad->status & PAD_U ) ||
			( work->control.mov.vy >= ( l->limit[ 1 ] - LADDER_SPACE + 50.0F ) ) ||
			Status( PLAYER_WATCH ) ) {
			//	    work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			SetMode( work, LadderStill ) ;
			return ;
		}
		//printf( "[%d] up %d %d\n", GV_Time, rc, lc ) ;
    }
#ifdef PAL
    if ( EndMotionAll( work ) ) {
		//printf( "[%d] up end\n", GV_Time ) ;
		l->flag |= LAD_FLAG_R_UP ;
        Centering( work ) ;
		work->control.step.vy = 0.0F ;
		//SetFlag( FLAG_NO_MOTION_STEP ) ;
	}
    if ( EndMotionAll( work ) ) {
		/* rootずれの防止 */
		if ( work->body.m_ctrl->mt3_ctrl->root->prev.vy > 
			 work->body.m_ctrl->mt3_ctrl->root->next.vy ) {
			work->body.m_ctrl->mt3_ctrl->root->prev.vy 
				= work->body.m_ctrl->mt3_ctrl->root->next.vy ;
		}
    }
#else
    if ( EndMotionAll( work ) ) {
		//printf( "[%d] up end\n", GV_Time ) ;
        Centering( work ) ;
		work->control.step.vy = 0.0F ;
//		SetFlag( FLAG_NO_MOTION_STEP ) ;
    }
#endif

#if 0
	printf( "%f %f / %f %f %f . %f %f\n",
		     work->body.m_ctrl->mt3_ctrl->move->step.vw,
		     work->body.m_ctrl->mt3_ctrl->root->step.vw, 
		     work->body.m_ctrl->mt3_ctrl->root->time,
		     work->body.m_ctrl->mt3_ctrl->root->interp_time,
		     work->body.m_ctrl->mt3_ctrl->root->inv_time,
		     work->body.m_ctrl->mt3_ctrl->root->prev.vy,
		     work->body.m_ctrl->mt3_ctrl->root->next.vy ) ;
#endif
    AdjustHeight( work ) ;
}

/* はしご降りる */
static	void	LadderDown( work, time )
PlayerWork	*work ;
int		time ;
{
    LADDER_SET	*l ;
    int		mtime, interp ;
	int		rc, lc ;
	static	float	Height ;
	float			v ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_IK | FLAG_CANNOT_PEEP ) ;
    SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_GRAVITY ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
	PL_SetNoUseItemType( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ;

    l = &LadderSet ;
    if ( time == 0 ) {
		//	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
		if ( !( l->flag & LAD_FLAG_DOWN ) ) {
			//          SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
			l->flag |= LAD_FLAG_DOWN ;
		} 
		if ( l->flag & LAD_FLAG_R_UP ) {
			mtime = DOWN_RIGHT_UP ;
			interp = 6 ;
			Centering( work ) ;
		} else {
			mtime = DOWN_LEFT_UP ;
			interp = 6 ;
		}
        SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
		SetAction3( work, l->motions[ Mladdown ], mtime, interp ) ;	
		Height = 50.0F ;
    }

	SetCamera( work ) ;
	v  = Height ;
	Height = GV_NearExp8F( Height, 0.0F ) ;
	work->control.mov.vy += v - Height ;

	/* 小さいほうが先 */
	rc = PL_CheckMotionTime( work, DOWN_RIGHT_UP ) ;
	lc = PL_CheckMotionTime( work, DOWN_LEFT_UP ) ;

//    mtime = work->body.m_ctrl->mt3_ctrl->time ;
    if ( time != 0 && ( rc || lc ) ) {
		if ( rc ) {
			l->flag |= LAD_FLAG_R_UP ;
		} else {
			l->flag &= ~LAD_FLAG_R_UP ;
		}	    
		SetLadderHeight( work, 1, l->flag ) ;
		if ( !( work->pad->status & PAD_D ) ||
			( work->control.mov.vy <= l->limit[ 0 ] + LADDER_SPACE - 50.0F ) ||
			Status( PLAYER_WATCH ) ) {
			//	    work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			SetMode( work, LadderStill ) ;
			return ;
		}
		Height = 50.0F ;
		printf( "[%d] down %d %d\n", GV_Time, rc, lc ) ;
    }
#ifdef PAL
    if ( EndMotionAll( work ) ) {
		printf( "[%d] down end\n", GV_Time ) ;
		l->flag |= LAD_FLAG_R_UP ;
		Centering( work ) ;
		work->control.step.vy = 0.0F ;
//		SetFlag( FLAG_NO_MOTION_STEP ) ;
	}
	if ( EndMotionAll( work ) ) {
		/* rootずれの防止 */
		if ( work->body.m_ctrl->mt3_ctrl->root->prev.vy < 
			work->body.m_ctrl->mt3_ctrl->root->next.vy ) {
			work->body.m_ctrl->mt3_ctrl->root->prev.vy 
				= work->body.m_ctrl->mt3_ctrl->root->next.vy ;
		}
    }
#else
    if ( EndMotionAll( work ) ) {
		printf( "[%d] down end\n", GV_Time ) ;
		Centering( work ) ;
		work->control.step.vy = 0.0F ;
//		SetFlag( FLAG_NO_MOTION_STEP ) ;
	}
#endif

#if 0
	printf( "%f %f / %f %f %f . %f %f\n",
		     work->body.m_ctrl->mt3_ctrl->move->step.vw,
		     work->body.m_ctrl->mt3_ctrl->root->step.vw, 
		     work->body.m_ctrl->mt3_ctrl->root->time,
		     work->body.m_ctrl->mt3_ctrl->root->interp_time,
		     work->body.m_ctrl->mt3_ctrl->root->inv_time,
		     work->body.m_ctrl->mt3_ctrl->root->prev.vy,
		     work->body.m_ctrl->mt3_ctrl->root->next.vy ) ;
#endif
    AdjustHeight( work ) ;
}

/* はしごのけぞり */
static	void	LadderShrink( PlayerWork *work, int time )
{
    LADDER_SET	*l ;
	FVECTOR		pos, diff ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_IK ) ;
    SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_GRAVITY | FLAG_CANNOT_PEEP ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
	PL_SetNoUseItemType( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ;

    l = &LadderSet ;
    if ( time == 0 ) {
        SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP | FLAG_NO_STEP ) ;
		if ( work->motion1 == l->motions[ Mladup ] ) {
			/* 登り中 */
			/* 一段登ったことにする */
			if ( l->flag & LAD_FLAG_R_UP ) l->flag &= ~LAD_FLAG_R_UP ;
			else						   l->flag |= LAD_FLAG_R_UP ;
			work->data = 1 ;
		} else if ( work->motion1 == l->motions[ Mladdown ] ) {
			/* 降り中 */
			work->data = 2 ;
		}
        if ( l->flag & LAD_FLAG_R_UP ) {
			SetAction3( work, l->motions[ Mladdmg_r ], 0, 6 ) ;
			/* 左手位置保存 */
			PL_ObjPos( work, HUMAN21_HIDARI_TE, &work->fv ) ;
		} else {
			SetAction3( work, l->motions[ Mladdmg_l ], 0, 6 ) ;
			/* 右手位置保存 */
			PL_ObjPos( work, HUMAN21_MIGI_TE, &work->fv ) ;
		}
    }
	SetCamera( work ) ;
	/* 手の位置を変えないようにモーションを変化させる */
	if ( l->flag & LAD_FLAG_R_UP ) {	/* 右手上 */
		PL_ObjPos( work, HUMAN21_HIDARI_TE, &pos ) ;
		_sceVu0SubVector( &diff, &work->fv, &pos ) ;
		_sceVu0AddVector( &pos, &work->control.mov, &diff ) ;
		GM_ResetControlPosition( &work->control, &pos ) ;
		SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_STEP ) ;
	} else {							/* 左手上 */
		PL_ObjPos( work, HUMAN21_MIGI_TE, &pos ) ;
		_sceVu0SubVector( &diff, &work->fv, &pos ) ;
		_sceVu0AddVector( &pos, &work->control.mov, &diff ) ;
		GM_ResetControlPosition( &work->control, &pos ) ;
		SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_STEP ) ;
	}

    if ( Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		PL_SubjectTurn( work ) ;
    } 
	if ( EndMotion( work ) ) {
		PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;	/* 通常と同じにすること */
		UnsetStatus( PLAYER_DAMAGED ) ;
        SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP ) ;
		if ( work->data == 1 ) {
			/* 一段上がった */
			SetLadderHeight( work, 0, l->flag ) ;
		} 
		SetMode( work, LadderStill ) ;
		Centering( work ) ;
	}
    work->control.step.vy = 0.0F ;
//    AdjustHeight( work ) ;
}

/* はしご落下 */
static	void	LadderFall( PlayerWork *work, int time )
{
    LADDER_SET	*l ;
	FVECTOR		pos, diff ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_IK ) ;
    SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_GRAVITY |
			 FLAG_DONOT_CHECK_WATCH ) ;
    SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
	PL_SetNoUseItemType( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ;

    l = &LadderSet ;
    if ( time == 0 ) {
		PL_LeaveSubject( work ) ;
        SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_WAIST_INTERP | FLAG_NO_STEP ) ;
		if ( work->motion1 == l->motions[ Mladup ] ) {
			/* 登り中 */
			/* 一段登ったことにする */
			if ( l->flag & LAD_FLAG_R_UP ) l->flag &= ~LAD_FLAG_R_UP ;
			else						   l->flag |= LAD_FLAG_R_UP ;
		} else if ( work->motion1 == l->motions[ Mladdown ] ) {
			/* 降り中 */
		}
        if ( l->flag & LAD_FLAG_R_UP ) {
			SetAction3( work, l->motions[ Mladfall_s_r ], 0, 6 ) ;
			/* 左手位置保存 */
			PL_ObjPos( work, HUMAN21_HIDARI_TE, &work->fv ) ;
		} else {
			SetAction3( work, l->motions[ Mladfall_s_l ], 0, 6 ) ;
			/* 右手位置保存 */
			PL_ObjPos( work, HUMAN21_MIGI_TE, &work->fv ) ;
		}
		work->fdata = work->control.mov.vy ;
    }
	SetCamera( work ) ;
	switch( work->data ) {
	case 0 :	/* 落下開始 */
		if ( PL_MotionTime( work ) < 16 ) {
			/* 手の位置を変えないようにモーションを変化させる */
			if ( l->flag & LAD_FLAG_R_UP ) {	/* 右手上 */
				PL_ObjPos( work, HUMAN21_HIDARI_TE, &pos ) ;
				_sceVu0SubVector( &diff, &work->fv, &pos ) ;
				_sceVu0AddVector( &pos, &work->control.mov, &diff ) ;
				GM_ResetControlPosition( &work->control, &pos ) ;
				SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_STEP ) ;
			} else {							/* 左手上 */
				PL_ObjPos( work, HUMAN21_MIGI_TE, &pos ) ;
				_sceVu0SubVector( &diff, &work->fv, &pos ) ;
				_sceVu0AddVector( &pos, &work->control.mov, &diff ) ;
				GM_ResetControlPosition( &work->control, &pos ) ;
				SetFlag( FLAG_NO_MOTION_STEP | FLAG_NO_STEP ) ;
			}
		}
		if ( EndMotion( work ) ) {
			SetAction( work, l->motions[ Mladfall_l ], 6 ) ;
			work->data = 1 ;
			work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
			work->control.grounded = 0 ;
			work->data2 = -96 ;
		}
		break ;
	case 1 :	/* 落下中 */
		work->control.step.vy = ( float )work->data2 ;
		work->data2 -= 16 ;
		if ( work->control.grounded & 1 ) {
			SetAction( work, l->motions[ Mladfall_e ], 6 ) ;
			work->data = 2 ;
			NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
			NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;
			work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
			if ( !Status( PLAYER_DEAD ) ) {
				if ( work->fdata - work->control.mov.vy > 6400.0F ) {	/* エルードと一緒 */
					work->power.vital = 0 ;
					SetStatus( PLAYER_DEAD ) ;
					PL_SetDeadFlag( work ) ;
					/* 血 */
					{
						extern void	*NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int ) ;
						FVECTOR		pos, force ;
						
						GV_MatToVec( &work->body.objs->objs[ 11 ].world, &pos ) ;
						force.vx = force.vz = 0.0F ;
						force.vy = -320.0F ;
						NewBlood( &( work->body.objs->objs[ 11 ].world ), 
								 &pos, &force, 1, 0 ) ;
					}
					GM_SeSetMode( SD_V_POUT0001, &work->control.mov, GM_SEMODE_BOMB ) ;
				} else {
					GM_SeSetMode( SD_V_PDMG02, &work->control.mov, GM_SEMODE_BOMB ) ;
				}
			}
		}
		break ;
	case 2 :	/* べちゃ */
		PL_GroundIK2( work, 250.0F ) ;
		PL_AvoidSink( work, 200.0F, 0.0F ) ;
		if ( EndMotion( work ) ) {
			work->down_dir = DOWN_BACK ;
			if ( Status( PLAYER_DEAD ) ) {
				SetMode( work, PL_Dead ) ;
			} else {
				SetMode( work, PL_Down ) ;
			}
			SetFlag( FLAG_FORCE_END ) ;
			SetStatus( PLAYER_GROUND | PLAYER_DOWNED ) ;
			PL_SetInvincible( work, DIRECT_TICK( 60 ) ) ;
			UnsetStatus( PLAYER_LADDER | PLAYER_DAMAGED ) ;
			PL_Force->e_turn = -1 ;
			PL_ResetAddress( work ) ;
			GM_ResetMenuStatus( MENU_WEAPON_OFF ) ;
			PL_DamageFunc = NULL ;
			l->flag &= ~LAD_FLAG_ACTIVE ;
			PL_Force->flag &= ~FA_USE_DEFAULT ;
			PL_Force->flag &= ~FA_NO_RECHECK_TRP ;
		}
	} 
}

/*----------------------------------------------------------------*/

/* ダメージチェック */
static	void	LadderCheckDamage( PlayerWork *work, long64 weapon_type, int dead )
{
	int			seNo ;

	if ( dead || ( weapon_type & ( WP_BLOW | WP_SOFTBLOW ) ) ) {
		/* 吹っ飛びＯＲ死亡 */
		NewPadVibration( PL_DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib2L, 2 | VAR_FLAG_FORCE ) ;		
		seNo = ( dead ) ? SD_V_POUT0001 : SD_V_PDMG01 ;
		SetMode( work, LadderFall ) ;
	} else {
		/* のけぞり */
		NewPadVibration( PL_DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( PL_DamageVib1L, 2 | VAR_FLAG_FORCE ) ;
		seNo = SD_V_PDMG02 ;
		SetMode( work, LadderShrink ) ;
	}
	GM_SeSetMode( seNo, &work->control.mov, GM_SEMODE_BOMB ) ;
	PL_SetInvincible( work, 0 ) ;
	GM_VctrlStopVibration( &work->vctrl ) ;
	GM_ClearTargetDamage( &( work->def ) ) ;
	GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
}

/*----------------------------------------------------------------*/

/* はしごモード設定 */
static	int	SetLadderAct( work, msg, len )
PlayerWork	*work ;
GV_MSG		*msg ;
int		len ;
{
    LADDER_SET	*l ;

    l = &LadderSet ;
    if ( !( l->flag & LAD_FLAG_ACTIVE ) ) {
		/* 以下の時は無効 */
		if ( Status( PLAYER_SQUAT | PLAYER_GROUND | PLAYER_DAMAGED | PLAYER_DOWNED |
					PLAYER_INTRUDE | PLAYER_FORCE | PLAYER_BEYOND | PLAYER_CB_BOX |
					PLAYER_DEAD | PLAYER_LOCKER ) || 
			( WeaponType( work ) & ( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ) ||
			( ItemType( work ) & ( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ) ) return -1 ;
		if ( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) return -1 ;
		/* 回転付きのモーションが終了したフレームで来るとバグる */
		/* 回転付きは強制モーションとビハインド攻撃のみと仮定してある */
		if ( Flag( FLAG_FORCE_END | FLAG_BEHIND_ATTACK ) ) {
			return -1 ;
		}
		GM_ResetControlPosition( &work->control, &l->start_mov ) ;
		work->control.turn.vy = l->start_dir ;
		work->control.rot.vy = l->start_dir ;
		PL_SetInvincible( work, 0 ) ;
		PL_LeaveCaution( work ) ;
		PL_ClearCaptureTarget( work ) ;
		PL_UnequipSpecials() ;
		SetMode2( work, NULL ) ;
		work->trigger = TRIG_FALL ;	/* グレネード落とし */

		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
		SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_STEP | FLAG_NO_MOTION_STEP |
				 FLAG_NO_WAIST_INTERP | FLAG_NO_GRAVITY | FLAG_DONOT_CHECK_WATCH ) ;
		SetStatus( PLAYER_FORCE ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		SetMode( work, LadderStart ) ;
		l->flag |= LAD_FLAG_ACTIVE ;
    } else {
		if ( Status( PLAYER_DAMAGED ) ) return -1 ;
		if ( Status( PLAYER_FORCE ) || Flag( FLAG_FORCE ) ) return -1 ;
		UnsetStatus( PLAYER_DAMAGED ) ;
		GM_ResetControlPosition( &work->control, &l->end_mov ) ;
		PL_SetInvincible( work, 0 ) ;
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ;
		SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_STEP | FLAG_NO_MOTION_STEP |
				 FLAG_NO_WAIST_INTERP | FLAG_DONOT_CHECK_WATCH ) ;
		SetStatus( PLAYER_FORCE ) ;
		GM_SetMenuStatus( MENU_MENU_OFF ) ;
		SetMode( work, LadderEnd ) ;
//		l->flag &= ~LAD_FLAG_ACTIVE ; 
		PL_Force->flag &= ~FA_USE_DEFAULT ;
		PL_Force->flag &= ~FA_NO_RECHECK_TRP ;
    }
    SetStatus( PLAYER_LADDER ) ;
    return 1 ;
}

/*---------------------------------------------------------------*/

/* プラグインはしご登録 */
int	NewPluginLadder( void )
{
    PL_AddPlugin( &LadderSet.plugin, PL_MSG_LADDER, SetLadderAct, NULL ) ;
    LadderSet.flag = 0 ;
    return 0 ;
}

/* はしご設定 */
int	NewSetupLadder( void )
{
    LADDER_SET	*l ;
    IVECTOR	iv ;
    u_char	*m ;

    l = &LadderSet ;
    m = l->motions ;
    if ( GCL_GetOption( 'l' ) != NULL ) {
		l->limit[ 0 ] = ( float )GCL_GetNextInt() ;
		l->limit[ 1 ] = ( float )GCL_GetNextInt() ;
    }
    if ( GCL_GetOption( 'b' ) != NULL ) {
		m[ Mladstill ] = GCL_GetNextInt() ;
		m[ Mladstill_l ] = GCL_GetNextInt() ;
		m[ Mladup ] = GCL_GetNextInt() ;
		m[ Mladdown ] = GCL_GetNextInt() ;
		if ( GCL_NextStr() != NULL ) m[ Mladdmg_r ] = GCL_GetNextInt() ;
		if ( GCL_NextStr() != NULL ) m[ Mladdmg_l ] = GCL_GetNextInt() ;
		if ( GCL_NextStr() != NULL ) m[ Mladfall_s_r ] = GCL_GetNextInt() ;
		if ( GCL_NextStr() != NULL ) m[ Mladfall_s_l ] = GCL_GetNextInt() ;
		if ( GCL_NextStr() != NULL ) m[ Mladfall_l ] = GCL_GetNextInt() ;
		if ( GCL_NextStr() != NULL ) m[ Mladfall_e ] = GCL_GetNextInt() ;
    }
    if ( GCL_GetOption( 's' ) != NULL ) {
		m[ Mladstart ] = GCL_GetNextInt() ;
		GCL_GetNextIV( ( int * )&iv ) ;
		GV_IVtoFV( &iv, &l->start_mov, 3 ) ;
		l->start_dir = GCL_GetNextInt() ;
    }
    if ( GCL_GetOption( 'e' ) != NULL ) {
		m[ Mladend ] = GCL_GetNextInt() ;
		GCL_GetNextIV( ( int * )&iv ) ;
		GV_IVtoFV( &iv, &l->end_mov, 3 ) ;
		l->end_dir = GCL_GetNextInt() ;
    }
    return 0 ;
}

/*------------------------------------------------------------------*/

/* 手モーフィング管理 */

#define	HAND_CHANGE_ACT			GV_StrCode( "プレイヤー手アニメはしご" ) 
#define	N_LAD_MOTIONS			(6)

extern	void *NewHandsChange_called( int name,DG_OBJS *human,
									 int r_hand_model,int *r_cv2_models,int r_cv2_size,
									 int l_hand_model,int *l_cv2_models,int l_cv2_size ) ;

static	void	*VanimeWork = NULL ;
static	void	*HandAnimeActWork = NULL ;

typedef	struct	{
	u_char	time, pattern ;
} TIMING ;

typedef	struct	{
	GV_ACT		actor ;
	void		*vanime_work ;
	int			flag ;
	int			count ;
	int			ptime ;
	TIMING		rtiming[ N_LAD_MOTIONS ][ 16 ] ;
	TIMING		ltiming[ N_LAD_MOTIONS ][ 16 ] ;
} HandAnimeWork ;

static	void	CheckTiming( work, mode, ptime, mtime, timing )
HandAnimeWork	*work ;
int				mode ;
int				ptime ;
int				mtime ;
TIMING			*timing ;
{
	int			i, buf[ 4 ] ;

	for ( i = 0; i < 16; i ++ ) {
		if ( ptime < timing[ i ].time && mtime >= timing[ i ].time ) break ;
		else if ( mtime < timing[ i ].time ) return ;
	}
	ASSERT( i < 16 ) ;
	buf[ 0 ] = mode ;
	buf[ 1 ] = timing[ i ].pattern ;
	buf[ 2 ] = 4 ;
	PL_SendMessage( HAND_CHANGE_ACT, buf, 3 ) ;
}

static	void	HandAnimeAct( work )
HandAnimeWork	*work ;
{
	int				msgbuf[ 4 ], motion, mtime ;
	TIMING			*rtiming, *ltiming ;
	PlayerWork		*pwork ;
	LADDER_SET		*l ;

	l = &LadderSet ;
	pwork = GM_PlayerWork ;

	if ( !Status( PLAYER_LADDER ) ) {
		if ( work->flag == 1 ) {
			work->flag = 0 ;
			work->count = 6 ;
		}
		if ( work->count > 0 ) {
			if ( -- work->count == 0 ) {
				msgbuf[ 1 ] = 0 ;	/* 非表示 */
				msgbuf[ 0 ] = 1 ;
				PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
				msgbuf[ 0 ] = 3 ;
				PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
			}
		}
		return ;
	}

	if ( work->flag == 0 ) {
		/* 表示 */
		msgbuf[ 1 ] = 1 ;
		msgbuf[ 0 ] = 1 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
		msgbuf[ 0 ] = 3 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
		work->ptime = -1 ;
	}

	work->flag = 1 ;
	motion = GM_PlayerMotion ;
	mtime = PL_MotionTime( pwork ) ;
	if ( motion == l->motions[ Mladstill ] ) {
		rtiming = work->rtiming[ 0 ] ;
		ltiming = work->ltiming[ 0 ] ;
	} else if ( motion == l->motions[ Mladstill_l ] ) {
		rtiming = work->rtiming[ 1 ] ;
		ltiming = work->ltiming[ 1 ] ;
	} else if ( motion == l->motions[ Mladup ] ) {
		rtiming = work->rtiming[ 2 ] ;
		ltiming = work->ltiming[ 2 ] ;
	} else if ( motion == l->motions[ Mladdown ] ) {
		rtiming = work->rtiming[ 3 ] ;
		ltiming = work->ltiming[ 3 ] ;
	} else if ( motion == l->motions[ Mladstart ] ) {
		rtiming = work->rtiming[ 4 ] ;
		ltiming = work->ltiming[ 4 ] ;
	} else if ( motion == l->motions[ Mladend ] ) {
		rtiming = work->rtiming[ 5 ] ;
		ltiming = work->ltiming[ 5 ] ;
	} else return ;

	CheckTiming( work, 0, work->ptime, mtime, rtiming ) ;
	CheckTiming( work, 2, work->ptime, mtime, ltiming ) ;
	work->ptime = mtime ;
}

static	void	HandAnimeDie( work )
HandAnimeWork	*work ;
{
	GV_DestroyActor( work->vanime_work ) ;
	VanimeWork = NULL ;
	HandAnimeActWork = NULL ;
}

static	int		HandAnimeGetResources( work )
HandAnimeWork	*work ;
{
	int		rhandmdls[ 16 ], lhandmdls[ 16 ] ;
	int		n_r_models, n_l_models ;
	int		i, j ;
	int		motion, cur[ N_LAD_MOTIONS ] ;

	if ( GCL_GetOption( 'p' ) != NULL ) {
		n_r_models = GCL_GetNextInt() ;
		n_l_models = GCL_GetNextInt() ;

		if ( GCL_GetOption( 'r' ) != NULL ) {
			for ( i = 0; i < n_r_models; i ++ ) {
				rhandmdls[ i ] = GCL_GetNextInt() ;
			}
		}
		if ( GCL_GetOption( 'l' ) != NULL ) {
			for ( i = 0; i < n_l_models; i ++ ) {
				lhandmdls[ i ] = GCL_GetNextInt() ;
			}
		}
		if ( VanimeWork == NULL ) {
			work->vanime_work = NewHandsChange_called( HAND_CHANGE_ACT, GM_PlayerBody->objs,
													  rhandmdls[ 0 ], rhandmdls, n_r_models,
													  lhandmdls[ 0 ], lhandmdls, n_l_models ) ;
			VanimeWork = work->vanime_work ;
			if ( work->vanime_work == NULL ) return -1 ;
		}
	}
	
	/* タイミングデータの読み込み */
	for ( i = 0; i < N_LAD_MOTIONS; i ++ ) {
		cur[ i ] = 0 ;
	}

	if ( GCL_GetOption( 't' ) != NULL ) {
		while( GCL_NextStr() != NULL ) {
			motion = GCL_GetNextInt() ;
			work->rtiming[ motion ][ cur[ motion ] ].time = GCL_GetNextInt() ;
			work->rtiming[ motion ][ cur[ motion ] ].pattern = GCL_GetNextInt() ;
			cur[ motion ] ++ ; 
		}
	}

	/* あまりを２５５でうめる */
	for ( i = 0; i < N_LAD_MOTIONS; i ++ ) {
		if ( HandAnimeActWork != NULL && cur[ i ] == 0 ) continue ;
		for ( j = cur[ i ]; j < 16; j ++ ) {
			work->rtiming[ i ][ j ].time = 255 ;
		}
	}

	for ( i = 0; i < N_LAD_MOTIONS; i ++ ) cur[ i ] = 0 ;
	if ( GCL_GetOption( 'I' ) != NULL ) {
		while( GCL_NextStr() != NULL ) {
			motion = GCL_GetNextInt() ;
			work->ltiming[ motion ][ cur[ motion ] ].time = GCL_GetNextInt() ;
			work->ltiming[ motion ][ cur[ motion ] ].pattern = GCL_GetNextInt() ;
			cur[ motion ] ++ ;
		}
	}

	/* あまりを２５５でうめる */
	for ( i = 0; i < N_LAD_MOTIONS; i ++ ) {
		if ( HandAnimeActWork != NULL && cur[ i ] == 0 ) continue ;
		for ( j = cur[ i ]; j < 16; j ++ ) {
			work->ltiming[ i ][ j ].time = 255 ;
		}
	}

	if ( HandAnimeActWork == NULL ) {
		int		msgbuf[ 2 ] ;
		/* 最初は非表示 */
		msgbuf[ 1 ] = 0 ;
		msgbuf[ 0 ] = 1 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
		msgbuf[ 0 ] = 3 ;
		PL_SendMessage( HAND_CHANGE_ACT, msgbuf, 2 ) ;
		work->flag = 0 ;
	}
	return 0 ;
}

void	*NewLadderPlayerHandMofingManager( name, where )
int		name, where ;
{	
    HandAnimeWork	*work ;

	if ( HandAnimeActWork != NULL ) {
		if ( VanimeWork != NULL ) GV_DestroyOtherActorQuick( VanimeWork ) ;
		GV_DestroyOtherActorQuick( HandAnimeActWork ) ;		
		HandAnimeActWork = VanimeWork = NULL ;
	}

    work = ( HandAnimeWork * )GV_NewActor( GV_ACTOR_AFTER, sizeof( HandAnimeWork ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), HandAnimeAct, HandAnimeDie ) ;
		if ( HandAnimeGetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	HandAnimeActWork = work ;
    return work ;
}

int		NewSetLadderPlayerHandMofingTiming( void )
{
	if ( HandAnimeActWork == NULL ) return -1 ;
	HandAnimeGetResources( HandAnimeActWork ) ;
	return 0 ;
}
