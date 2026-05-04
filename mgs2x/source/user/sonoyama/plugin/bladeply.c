//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bladeply.c
   ブレードプラグイン

   2001/01/17 M.Sonoyama
   $Id: bladeply.c,v 1.1.1.3 2002/11/19 11:50:46 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
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
#include	"../raiden/motion.h"
#include	"../raiden/pl_arm.h"

#if defined(BP_VITA)
#include "gesture_vta.h"
#endif

/*---------------------------------------------------------------*/

//#define	ACTION1

extern	void	NewSpark( FMATRIX * ) ;
extern	void	*NewPadVibration( char *, int ) ;

/*---------------------------------------------------------------*/

static	PL_PollingSet	PollingSet ;
static	int				PullOutFlag, InvFlag, VisFlag ;
static	int				Motion ;
static	void			*Blade2 ; 
static	int				Blade2Trig ;
static	int				StickPress ;
static	int				GuardHit, GuardLastTime, GuardSlash, GuardSlashLastTime ;
static	int				GuardSlashNoMuteki ;
static	int				SlashFillip ;
static	TARGET			BladeTarget, OffenseTarg ;
static	POWER_TARGET	PowerTarg ;
static	FVECTOR			OffCenter ;
//static	int				SlashMode ;

typedef	struct	{
	int			u, d, l, r ;
	int			flag ;
} CountWork ;

enum {
	RS_U	=	0x0001,
	RS_D	=	0x0002,
	RS_L	=	0x0004,	
	RS_R	=	0x0008,
	RS_ALL	=	0x000f,
} ;

static	CountWork		SwingCount ;

/*---------------------------------------------------------------*/

/* モーション */
static	short	BladeSet[] = {
    hfb_fire_p, hfb_fire_p, NO_ACT, hfb_walk, hfb_dash, hfb_dash, NO_ACT,
    none_stand, none_stair_walk, none_stair_run, none_walk_on,
	none_squat, m4a_crouch_idle, m4a_crouch_move_f,
    m4a_crouch_move_b, m4a_crouch_move_f_fast,
    none_caution, none_squat_caution, none_caution_r, none_caution_l,
    non_caution_r_sq, non_caution_l_sq,
    none_behind_r, none_behind_rb_start, none_behind_rb, none_behind_rb_end, none_behind_l, 
    none_behind_lb_start, none_behind_lb, none_behind_lb_end, 
    NO_ACT, NO_ACT, NO_ACT, NO_ACT,
    non_behind_r_sq, non_behind_l_sq, non_behind_rb_start_sq, non_behind_rb_sq,
    non_behind_rb_end_sq, non_behind_lb_start_sq, non_behind_lb_sq, non_behind_lb_end_sq,
    NO_ACT, NO_ACT, NO_ACT, NO_ACT, 
    non_knock_r1, non_knock_l1, non_knock_r2, non_knock_l2,
	non_knock_sq1, non_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

enum {
	BMpullout = 0,
	BMputback,
	BMstill,
	BMwalk,
	BMrun,
	BMslash_dr1,
	BMslash_dr2,
	BMslash_dr3,
	BMslash_dl1,
	BMslash_dl2,
	BMslash_dl3,
	BMslash_ur1,
	BMslash_ur2,
	BMslash_ur3,
	BMslash_ul1,
	BMslash_ul2,
	BMslash_ul3,
	BMthrust,
	BMguard,
	BMguard_p,
	BMguard_hit1,
	BMguard_hit2,
	BMguard_hit3,
	BMcombo,
	BMchange,
	BMslash_dre,
	BMslash_dle,
	BMslash_ure,
	BMslash_ule,
	BMslash_large,
	BMslash_v1,
	BMslash_v2,
	BMslash_v3,
	BMslash_r1,
	BMslash_r2,
	BMslash_r3,
	BMslash_l1,
	BMslash_l2,
	BMslash_l3,

	BMstair_walk,	/* XBOXの機能拡張 ガードしながら階段 */
	BMstair_run,
} ;

extern	WEAPON	NewBlade ;
extern	void	*NewBlade2( CONTROL *ctrl, OBJECT **body, int *unit, int *trigger, 
						    int side, int inv ) ;

static	void	BladePullOut( PlayerWork *work, int time ) ;
static	void	BladePullOutfromGround( PlayerWork *work, int time ) ;

static	void	BladePutBack( PlayerWork *work, int time ) ;
static	void	BladePutBackfromGround( PlayerWork *work, int time ) ;

//static	void	BladeReady( PlayerWork *work, int time ) ;
static	void	BladeSlash( PlayerWork *work, int time ) ;
static	void	BladeThrust( PlayerWork *work, int time ) ;
static	void	BladeGuard( PlayerWork *work, int time ) ;
static	void	BladeGuardSlash( PlayerWork *work, int time ) ;
static	void	BladeCombo( PlayerWork *work, int time ) ;
static	void	BladeReverse( PlayerWork *work, int time ) ;

static	int		DamageCheckCallback( PlayerWork *work, TARGET *off, TARGET *def ) ;

static	int		FirstAttack ;
static	int		AttackNo, NextAttackNo ;
static	int		StickDir, NextStickDir ;
//static	int		BladeAct = 0 ;
static	int		NoCheckSegment = 0 ;

static	PL_ACTION	BladeActions[] = {
	BladeSlash, BladeThrust, BladeGuard, BladeGuardSlash, BladeCombo,
	BladeReverse
} ;

enum {
	ATK_DOWN_L = 0,
	ATK_UP_L,
	ATK_DOWN_R,
	ATK_UP_R,
	ATK_VERTICAL,
	ATK_RIGHT,
	ATK_LEFT,
	ATK_THRUST,
} ;

enum {
	ATK_WHICH_BOTH = 		0x0000,
	ATK_WHICH_UP =			0x0001,
	ATK_WHICH_DOWN =		0x0002,
	ATK_WHICH_RIGHT =		0x0004,
	ATK_WHICH_LEFT =		0x0008,
} ;

static	int		AttackWhich[] = {
	ATK_WHICH_DOWN | ATK_WHICH_LEFT,
	ATK_WHICH_UP | ATK_WHICH_LEFT,
	ATK_WHICH_DOWN | ATK_WHICH_RIGHT,
	ATK_WHICH_UP | ATK_WHICH_RIGHT,
	ATK_WHICH_DOWN,
	ATK_WHICH_RIGHT,
	ATK_WHICH_LEFT
} ;

static	int		DownLeftSlashMotions[] = {
	BMslash_dl1, BMslash_dl2, BMslash_dl3, BMslash_dle
} ;

static	int		UpLeftSlashMotions[] = {
	BMslash_ul1, BMslash_ul2, BMslash_ul3, BMslash_ule
} ;

static	int		DownRightSlashMotions[] = {
	BMslash_dr1, BMslash_dr2, BMslash_dr3, BMslash_dre
} ;

static	int		UpRightSlashMotions[] = {
	BMslash_ur1, BMslash_ur2, BMslash_ur3, BMslash_ure
} ;

static	int		VerticalSlashMotions[] = {
	BMslash_v1, BMslash_v2, BMslash_v3, BMslash_dre
} ;

static	int		RightSlashMotions[] = {
	BMslash_r1, BMslash_r2, BMslash_r3, BMslash_ure
} ;

static	int		LeftSlashMotions[] = {
	BMslash_l1, BMslash_l2, BMslash_l3, BMslash_ule
} ;

static	int		*SlashMotions[] = {
	DownLeftSlashMotions, UpLeftSlashMotions,
	DownRightSlashMotions, UpRightSlashMotions,
	VerticalSlashMotions, RightSlashMotions, LeftSlashMotions
} ;

#if 0
static	int		AttackNext[] = {
	ATK_UP_R, ATK_DOWN_R, ATK_UP_L, ATK_DOWN_L, ATK_UP_L,
	ATK_LEFT, ATK_RIGHT
} ;
#endif

#define	BLADE_RSTICK_MARGIN_F	(96.0F)

extern int PL_PAD_WEAPON ;
extern int PL_PAD_LOCKON ;

#define	PAD_GUARD				(PL_PAD_LOCKON)
#define	PAD_CHANGE				(PL_PAD_WEAPON)


/*---------------------------------------------------------------*/
/* raiden/adjust.c から抜粋 T.Morita */
/* 階段床の上から２５０以内は特殊処理 */
static	float	StepHeightAdjust( HZX_FLR *flr, float level )
{
	float		max ;

	max = flr->p1.y ;
	if ( max < flr->p2.y ) max = flr->p2.y ;
	if ( max < flr->p3.y ) max = flr->p3.y ;
	if ( max < flr->p4.y ) max = flr->p4.y ;
	if ( max - level < 250.0F ) return ( max - level ) ;
	return 250.0F ;
}

/* 階段の上の方チェック */
static	int		UpsideStep()
{
	if ( !( GM_PlayerControl->flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ) return 1 ;
	if ( StepHeightAdjust( GM_PlayerControl->level[ 0 ], 
			       GM_PlayerControl->levels[ 0 ] ) < 250.0F ) return 1 ;
	return 0 ;
}

/*---------------------------------------------------------------*/

/* 右スティック入力方向 */
/* -1で入力なし */
static	float	RStickMax = 0.0F ;
static	short	RStickDir( PlayerWork *work )
{
	FVECTOR		rstick_vec ;
	int			c ;

	RStickMax = 0.0F ;
	if ( !PL_UseStickR( work ) ) return -1 ;
	c = 0 ;
	rstick_vec.vx = ( float )work->pad->right_dx - 128.0F ;
	rstick_vec.vz = ( float )work->pad->right_dy - 128.0F ;
	if ( DG_FABS( rstick_vec.vx ) < BLADE_RSTICK_MARGIN_F ) c |= 1 ;
	if ( DG_FABS( rstick_vec.vz ) < BLADE_RSTICK_MARGIN_F ) c |= 2 ;
	RStickMax = DG_FABS( rstick_vec.vx ) ;
	if ( RStickMax < DG_FABS( rstick_vec.vz ) ) {
		RStickMax = DG_FABS( rstick_vec.vz ) ;
	}

	if ( c == 3 ) return -1 ;
	return ( GV_VecDir2( &rstick_vec ) & 4095 ) ;
}

/* スティックぐるぐる判定 */
static	void	InitCountWork( void )
{
	SwingCount.u = 0 ;
	SwingCount.d = 0 ;
	SwingCount.l = 0 ;
	SwingCount.r = 0 ;
	SwingCount.u = 0 ;
	SwingCount.flag = 0 ;
}

static	void	UpdateCountWork( PlayerWork *work )
{
	CountWork		*c ;
	int				dir ;

	c = &SwingCount ;
	dir = RStickDir( work ) ;
	if ( RStickMax > 64.0F && dir >= 0 ) {
		if ( GV_DiffDirAbs( 2048, dir ) < 256 ) {
			c->u = GV_Time ;
		} else if ( GV_DiffDirAbs( 0, dir ) < 256 ) {
			c->d = GV_Time ;
		} else if ( GV_DiffDirAbs( 3072, dir ) < 256 ) {
			c->l = GV_Time ;
		} else if ( GV_DiffDirAbs( 1024, dir ) < 256 ) {
			c->r = GV_Time ;
		} 
	} else {
		c->u = c->d = c->l = c->r = 0 ;
	}
	c->flag = 0 ;

	if ( GV_Time - c->u < ( 300 / TIME_BASE ) * 5 / 4 ) c->flag |= RS_U ;
	if ( GV_Time - c->d < ( 300 / TIME_BASE ) * 5 / 4 ) c->flag |= RS_D ;
	if ( GV_Time - c->l < ( 300 / TIME_BASE ) * 5 / 4 ) c->flag |= RS_L ;
	if ( GV_Time - c->r < ( 300 / TIME_BASE ) * 5 / 4 ) c->flag |= RS_R ;
//	printf( "[%d] %d %x\n", GV_Time, dir, c->flag ) ;
}

static	int	CheckCountWork( void )
{
	CountWork	*c ;
	int			time[ 4 ], i, n ;
	int			min, cur, next ;

	c = &SwingCount ;
	if ( c->flag != RS_ALL ) return 0 ;
	/* 一番先に押されたものを探す */
	time[ 0 ] = c->u ;
	time[ 1 ] = c->r ;
	time[ 2 ] = c->d ;
	time[ 3 ] = c->l ;
	min = time[ 0 ] ; n = 0 ;
	for ( i = 0; i < 4; i ++ ) {
		if ( min > time[ i ] ) {
			min = time[ i ] ;
			n = i ;
		}
	}
	/* 時計まわりチェック */
	for ( i = 0; i < 3; i ++ ) {
		cur = n + i ;
		next = cur + 1 ;
		if ( cur >= 4 ) cur -= 4 ;
		if ( next >= 4 ) next -= 4 ;
		if ( time[ next ] < time[ cur ] ) goto clock_reverse_check ;
	}
//	printf( "clockclock\n" ) ;
	return 1 ;
clock_reverse_check :
	/* 反時計まわりチェック */
	for ( i = 0; i < 3; i ++ ) {
		cur = n - i ;
		next = cur - 1 ;
		if ( cur < 0 ) cur += 4 ;
		if ( next < 0 ) next += 4 ;
		if ( time[ next ] < time[ cur ] ) return 0 ;
	}
//	printf( "clockreverse\n" ) ;
	return 1 ;
}

/* 攻撃パターン設定 */
static	int		SetAttackNo( PlayerWork *work, int which )
{
	int			rstick_dir, diff ;
	
	rstick_dir = RStickDir( work ) ;
	NextStickDir = rstick_dir ;
	if ( rstick_dir < 0 ) return -1 ;
//	diff = GV_DiffDirS( work->control.turn.vy, rstick_dir ) ;
	diff = GV_DiffDirS( 0, rstick_dir ) ;

	if ( diff < 0 ) {
		if ( diff > -128 ) NextAttackNo = ATK_VERTICAL ;
		else if ( diff > -( 128 + 768 ) ) NextAttackNo = ATK_DOWN_L ;
		else if ( diff > -( 128 + 768 + 256 ) ) NextAttackNo = ATK_LEFT ;
		else if ( diff > -( 128 + 768 + 256 + 768 ) ) NextAttackNo = ATK_UP_L ;
		else NextAttackNo = ATK_UP_L ;
	} else {
		if ( diff < 128 ) NextAttackNo = ATK_VERTICAL ;
		else if ( diff < ( 128 + 768 ) ) NextAttackNo = ATK_DOWN_R ;
		else if ( diff < ( 128 + 768 + 256 ) ) NextAttackNo = ATK_RIGHT ;
		else if ( diff < ( 128 + 768 + 256 + 768 ) ) NextAttackNo = ATK_UP_R ;
		else NextAttackNo = ATK_UP_R ;
	}
//printf( "%d %d\n", diff, NextAttackNo ) ;
	if ( which == ATK_WHICH_BOTH ) return 0 ;
	if ( which & ATK_WHICH_UP ) {
		if ( NextAttackNo == ATK_VERTICAL ) NextAttackNo = ATK_UP_R ;
		else if ( NextAttackNo == ATK_DOWN_R ) NextAttackNo = ATK_UP_R ;
		else if ( NextAttackNo == ATK_DOWN_L ) NextAttackNo = ATK_UP_L ;
	} else if ( which & ATK_WHICH_DOWN ) {
		if ( NextAttackNo == ATK_UP_R ) NextAttackNo = ATK_DOWN_R ;
		else if ( NextAttackNo == ATK_UP_L ) NextAttackNo = ATK_DOWN_L ;
	}
	if ( which & ATK_WHICH_RIGHT ) {
		if ( NextAttackNo == ATK_DOWN_L ) NextAttackNo = ATK_DOWN_R ;
		else if ( NextAttackNo == ATK_UP_L ) NextAttackNo = ATK_UP_R ;
		else if ( NextAttackNo == ATK_LEFT ) NextAttackNo = ATK_RIGHT ;		
	} else if ( which & ATK_WHICH_LEFT ) {
		if ( NextAttackNo == ATK_DOWN_R ) NextAttackNo = ATK_DOWN_L ;
		else if ( NextAttackNo == ATK_UP_R ) NextAttackNo = ATK_UP_L ;
		else if ( NextAttackNo == ATK_RIGHT ) NextAttackNo = ATK_LEFT ;		
	}
#if 0
	if ( diff < 0 ) {
		if ( which == ATK_WHICH_BOTH ) {
//			if ( diff > -1024 ) NextAttackNo = ATK_DOWN_L ;
//			else			    NextAttackNo = ATK_UP_L ;
			if ( GV_DiffDirAbs( rstick_dir, 2048 ) > 1024 ) NextAttackNo = ATK_DOWN_L ;
			else			    NextAttackNo = ATK_UP_L ;
		} else if ( which == ATK_WHICH_UP ) {
			NextAttackNo = ATK_UP_L ;
		} else {
			NextAttackNo = ATK_DOWN_L ;
		}
	} else {
		if ( which == ATK_WHICH_BOTH ) {
//			if ( diff < 1024 ) NextAttackNo = ATK_DOWN_R ;
//			else			   NextAttackNo = ATK_UP_R ;
			if ( GV_DiffDirAbs( rstick_dir, 2048 ) > 1024 ) NextAttackNo = ATK_DOWN_R ;
			else			   NextAttackNo = ATK_UP_R ;
		} else if ( which == ATK_WHICH_UP ) {
			NextAttackNo = ATK_UP_R ;
		} else {
			NextAttackNo = ATK_DOWN_R ;
		}
	}
#endif
	return 0 ;
}

/* 持ち替えチェック */
static	int		CheckBladeReverse( PlayerWork *work )
{
#if defined(BP_VITA)
   if (PlayerPad.pad.pressure[PAD_PRESS_D]) {
#else
	if ( work->pad->press & PAD_CHANGE ) {
#endif
		SetMode( work, BladeReverse ) ;
		return 1 ;
	} 
	return 0 ;
}

/* 攻撃チェック */
static	void	BladeCheckAttack( PlayerWork *work ) 
{
   int do_stab = 0;

	/* しゃがみ、匍匐時攻撃不可 */
	if ( Status( PLAYER_SQUAT | PLAYER_GROUND | PLAYER_CAUTION ) ) {
		FirstAttack = 0 ;
		StickPress = 0 ;
		StickDir = -1 ;
		return ;
	}

	/* 持ち替え */
	if ( CheckBladeReverse( work ) ) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		return ;
	}
	/* 突き */
   
#if defined(BP_VITA)
   do_stab = work->pad->press & PS2_PAD_SQUARE;
#endif

	if ( work->pad->press & PAD_AR || do_stab) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		SetMode( work, BladeThrust ) ;
		return ;
	}

	/* 防御 */
	if ( work->pad->status & PAD_GUARD ) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		GM_SeSetMode( SD_A_SWIGUARD, &work->control.mov, GM_SEMODE_NORMAL ) ;
		SetMode( work, BladeGuard ) ;
		return ;
	}

	/* コンボ */
	if ( work->pad->press & PL_PAD_PUNCH ) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		SetMode( work, BladeCombo ) ;
		return ;
	}

	if ( !PL_UseStickR( work ) ) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		return ;
	}

	if ( StickPress == 2 ) return ;
	if ( StickPress == 1 && StickDir >= 0 &&
		 GV_DiffDirAbs( RStickDir( work ), StickDir ) < 1024 ) return ;

//	StickPress = 1 ;

	if ( SetAttackNo( work, ATK_WHICH_BOTH ) < 0 ) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		return ;
	}

	FirstAttack = 1 ;
	AttackNo = NextAttackNo ;
	StickDir = NextStickDir ;
//	SetMode( work, BladeReady ) ;
	SetMode( work, BladeSlash ) ;
}

extern void *NewBullet( FMATRIX *world , u_int type ,
			u_int    side  , u_int size , u_int damage,
			u_int    length, u_int speed, int   weapon ) ;

/* 防御コールバック */
static	void	Hitted( TARGET *off, TARGET *def, void *ptr )
{
	FMATRIX		mat ;
	SVECTOR		rot ;
	FVECTOR		diff ;

	GM_ClearTargetDamage( def ) ;
	if ( !( off->class & TARGET_ONLINE ) ) return ;

	if ( off->weapon_type & WP_BULLET ) {
		_sceVu0SubVector( &diff, &off->center, &def->hit ) ;
		GV_VecToRot( &diff, &rot ) ;
		rot.vx -= 1024 ;
		DG_SetPos2( &def->hit, &rot ) ;
		DG_GetPos( &mat ) ;
		NewSpark( &mat ) ;
		GuardHit ++ ;
		GM_SeSetMode( SD_A_SWORDRIC, &def->hit, GM_SEMODE_BOMB ) ;
	} else if ( off->weapon_type & WP_SHOTGUN_NEAR ) {
		_sceVu0SubVector( &diff, &off->offset, &off->center ) ;
		GV_VecToRot( &diff, &rot ) ;
		rot.vx -= 1024 ;
		DG_SetPos2( &GM_PlayerControl->mov, &rot ) ;
		DG_GetPos( &mat ) ;
#define SPS_DAMAGE	40 /*2001.08.18 4倍に*/
		NewBullet( &mat,
			   BUL_TYPE_NO_HZD,
			   PLAYER_SIDE,
			   25,
			   SPS_DAMAGE,
			   20000,
			   1000,
			   WP_ShotGun_Near ) ;
	}
}

/* 防御ターゲット設定 */
static	void	MoveTarget( PlayerWork *work )
{
	GM_MoveTarget2( &BladeTarget, &work->body.objs->world ) ;
	GM_TargetResetSkip( &BladeTarget ) ;
//	NewTargetView2( &BladeTarget, 32, 232, 32 ) ;
}

/* 刀受け止め用、本体コールバック */
static	int		DamageCheckCallback( PlayerWork *work, TARGET *off, TARGET *def )
{
	long64		wt ;
	int			dir ;
	FVECTOR		*offcenter ;

	wt = off->weapon_type ;
	//printf( "blade guard hit %lx\n", wt ) ;
	if ( ( wt & WP_BLADEGUARD ) ||	
		 ( wt & WP_BLADE ) || 
		 ( wt & ( WP_BODY | WP_KICK ) ) ) {
		offcenter = &off->center ;
		if ( off->name != 0 ) {
			CONTROL		*ctrl ;
			ctrl = GM_SearchWhere( off->name ) ;
			if ( ctrl != NULL ) {
				offcenter = &ctrl->mov ;
			}
		}
		dir = GV_VecDir2FromTo( &def->center, offcenter ) ;
		
		if ( GV_DiffDirAbs( dir, work->control.rot.vy ) < 768 ) {
			if ( def->weapon_type == off->weapon_type ) {
				GM_ClearTargetDamage( def ) ;
			}
			def->weapon_type &= ~off->weapon_type ;
			GuardSlash = 1 ;
			/* ソリダスコンボ受け専用 */
			GuardSlashNoMuteki = ( ( wt & WP_NOMUTEKI ) && ( wt & WP_AKS ) ) ? 1 : 0 ;
			DG_COPY_VEC( &OffCenter, &off->center ) ;
			work->control.turn.vy = dir ;
			return 1 ;
		}
	}
	return 0 ;
}

/* 武器カメラセット */
static	void	SetWeaponCamera( PlayerWork *work )
{
	PL_SetFlag( FLAG_WEAPON_CAMERA_ON ) ;
	work->vwait2 = 0 ;
}

static	void	UnsetWeaponCamera( PlayerWork *work )
{
    if ( work->vwait2 <= 0 ) work->vwait2 = 1 ;
}

/*---------------------------------------------------------------*/

#if 0
/* 構える */
static	void	BladeReady( PlayerWork *work, int time )
{
	int			*motion ;
	int			dir, diff ;

	motion = SlashMotions[ AttackNo ] ;
	SetFlag( FLAG_CANNOT_CHANGE ) ;
	if ( time == 0 ) {
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, motion[ 0 ], 6 ) ;
		if ( StickDir < 0 ) {
			PL_ReturnMotionArc( work ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;			
			return ;
		}
	}
	switch( work->data ) {
	case 0 :
		if ( EndMotion( work ) ) {
			if ( FirstAttack ) {
				SetMode( work, BladeSlash ) ;
			} else {
				PL_MotionSleep( work, 0 ) ;
				work->data = 1 ;
			}
		}
		break ;
	case 1 :
		if ( PL_UseStickR( work ) ) {
			work->data2 = 0 ;
			dir = RStickDir( work ) ;
			if ( dir < 0 ) goto slash_ready_end ;
			SetAttackNo( work, ATK_WHICH_BOTH ) ;
			PL_MotionActive( work, 0 ) ;
			SetMode( work, BladeSlash ) ;
		} else {
slash_ready_end :
			if ( ++ work->data2 > 8 ) {
				PL_MotionActive( work, 0 ) ;
				PL_ReturnMotionArc( work ) ;
				SetMode( work, PL_StillMode[ STAND ] ) ;			
			}
		}
	}
}
#endif


/*---------------------------------------------------------------*/

/* 連続斬りチェック */
static	int		ContSlash1( PlayerWork *work, int already )
{
	int		next, dir, prev, pre_which ;
	int		nbuf, sbuf ;
	
	next = ATK_WHICH_BOTH ;
	pre_which = AttackWhich[ AttackNo ] ;
	if ( pre_which & ATK_WHICH_DOWN ) next |= ATK_WHICH_UP ;
	else if ( pre_which & ATK_WHICH_UP ) next |= ATK_WHICH_DOWN ;
	if ( pre_which & ATK_WHICH_RIGHT ) next |= ATK_WHICH_LEFT ;
	else if ( pre_which & ATK_WHICH_LEFT ) next |= ATK_WHICH_RIGHT ;

//	next = ( AttackNo & 1 ) ? ATK_WHICH_DOWN : ATK_WHICH_UP ;
	prev = AttackNo ;
	dir = RStickDir( work ) ;
	nbuf = NextAttackNo ;
	sbuf = NextStickDir ;
	if ( dir >= 0 && StickDir >= 0 &&
		GV_DiffDirAbs( dir, StickDir ) > 768 &&
		SetAttackNo( work, next ) >= 0 ) {
#if 0
		if ( ( prev <= ATK_UP_L && NextAttackNo <= ATK_UP_L ) ||
			( prev >= ATK_DOWN_R && NextAttackNo >= ATK_DOWN_R ) ) {
			FirstAttack = 1 ;
		} else {
			FirstAttack = 0 ;				
		}
#endif
//		FirstAttack = 1 ;
		return 1 ;
	}
	if ( already ) {
		NextAttackNo = nbuf ;
		NextStickDir = sbuf ;
		return 1 ;
	}
	return 0 ;
}

#if 0
static	int		ContSlash2( PlayerWork *work, int already )
{
	int		next, dir, prev, rev ;
	int		nbuf, sbuf ;

	next = ATK_WHICH_BOTH ;
	prev = AttackNo ;
	dir = RStickDir( work ) ;
	nbuf = NextAttackNo ;
	sbuf = NextStickDir ;
//	rev = AttackNext[ prev ] ; 
	rev = AttackWhich[ prev ] ;
	if ( AttackNo == ATK_RIGHT || AttackNo == ATK_LEFT ) {
		rev |= ATK_WHICH_UP ;
	}
	if ( AttackNo == ATK_VERTICAL ) rev |= ATK_WHICH_RIGHT | ATK_WHICH_DOWN ;
	if ( dir >= 0 && StickDir >= 0 &&
		GV_DiffDirAbs( dir, StickDir ) > 768 &&
		SetAttackNo( work, next ) >= 0 ) {
		if ( prev == NextAttackNo ) return 0 ;
		if ( NextAttackNo == ATK_VERTICAL ) return 0 ;
//		if ( rev == NextAttackNo ) {
		if ( !( rev & AttackWhich[ NextAttackNo ] ) ) {
//			FirstAttack = 1 ;				
			return 1 ;
		}
		return 2 ;
	}
	if ( already ) {
		NextAttackNo = nbuf ;
		NextStickDir = sbuf ;
		if ( nbuf == rev ) return 1 ;
		else			   return 2 ;
	}
	return 0 ;
}
#endif

#if 0
/* 位置あわせ */
/* 左足が動かないように */
static	void	SlashAdjustPos( PlayerWork *work )
{
	FVECTOR		pos, diff ;

	PL_ObjPos( work, HUMAN21_HIDARI_TSUMASAKI, &pos ) ;
	_sceVu0SubVector( &diff, &work->fv, &pos ) ;
	work->control.step.vx = diff.vx ;
	work->control.step.vz = diff.vz ;
	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
}
#endif

/* ホーミング先との間に壁が無いかチェック */
static	HOMING_TRG	*CheckHazardBetweenItoHoming( PlayerWork *work, HOMING_TRG *homing )
{
    FVECTOR		to ;
    
    if ( homing == NULL ) return NULL ;
    GV_MatToVec( homing->world, &to ) ;
    if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov,
							   &to, HZX_CHK_ALL, 
							   HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY,
							   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_NO_ENEMY ) ) {
			       
		return NULL ;
    }
    return homing ;
}

/* 斬りホーミング */
static	void	SlashHoming( PlayerWork *work )
{
    HOMING_TRG	*homing ;
    CONTROL	*ctrl ;
    FVECTOR	pos, trg ;
    SVECTOR	adj, limit ;

	if ( SlashFillip != 0 ) return ; /* はじかれた直後 */
	//if ( work->pad->status & PAD_UDLR ) return ;
	//printf( "<%d>homing!!!\n", GV_Time ) ;
    ctrl = &( work->control ) ;
	limit.vx = 768 ; limit.vy = -1 ;
    homing = GM_GetHomingTrgInSight( &( ctrl->mov ), &ctrl->rot, &limit, 
									NULL, 0, ctrl->hzx_id ) ;    
    if ( homing == NULL ) return ;
    if ( CheckHazardBetweenItoHoming( work, homing ) == NULL ) return ;

    GV_MatToVec( &( work->body.objs->objs[ 2 ].world ), &pos ) ;
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		/* 的対策。敵兵はhom->worldにobjs[ 2 ]をいれているのでこれで良いはず */
		GV_MatToVec( homing->world, &trg ) ;
	} else {
		GV_MatToVec( &( homing->body->objs->objs[ 2 ].world ), &trg ) ;
	}

	/* とおすぎ */
	if ( GV_VecLen3F2( &pos, &trg ) > 4000.0F ) return ;

    GM_TrgToAdjRot( &pos, ctrl->rot.vy, &trg, &adj ) ;
    ctrl->turn.vy = ctrl->rot.vy + adj.vy ;    
}

/* 前方壁チェック */
static	int		CheckFrontWall( PlayerWork *work )
{
	FMATRIX		mat ;
	FVECTOR		from, hit, diff ;
	FVECTOR		to = { 13.454841F, -501.865540F, 834.200134F } ;	/* 刃先 */
	SVECTOR		rot ;

	if ( GM_AlertMode == ALERT_MODE_ALERT ) return 0 ; /* 危険モード */
	if ( GM_GameLevel >= GM_LEVEL_EXTREME ) return 0 ; 
	if ( NoCheckSegment != 0 ) return 0 ;

	PL_ObjPos( work, HUMAN21_MIGI_TE, &from ) ;
	DG_SetPos( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world ) ;
	DG_PutVector( &to, &to, 1 ) ;
    if ( HZX_OnlineHazardCheck( work->control.hzx_id, &from, &to,
				HZX_CHK_SEGMENT,
				HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY | HZX_SEG_NO_HARITSUKI, /*張りつきを追加*/
				HZX_FLOOR_NO_PLAYER | HZX_FLOOR_NO_ENEMY ) ) {
		HZX_GetOnlinePoint( &hit ) ;
		_sceVu0SubVector( &diff, &from, &to ) ;
		GV_VecToRot( &diff, &rot ) ;
		rot.vx -= 1024 ;
		DG_SetPos2( &hit, &rot ) ;
		DG_GetPos( &mat ) ;
		NewSpark( &mat ) ;		
		GM_SeSetMode( SD_A_SWORDHAD, &work->control.mov, GM_SEMODE_BOMB ) ;
		return 1 ;
	}
	return 0 ;
}

/* ブレード終了処理 */
static	void	EndBlade( PlayerWork *work, int mask )
{
	StickPress = 1 ;
	SlashFillip = 0 ;
#ifndef ACTION1
	PL_SetAction2( work, -1, 6, mask ) ;
#endif
	GM_TargetSetSkip( &BladeTarget ) ;
	if ( work->current_mar == Motion ) {
		PL_ReturnMotionArc( work ) ;
	}
	if ( PL_TargetCallbackFunc == DamageCheckCallback ) {
		PL_TargetCallbackFunc = NULL ;
	}
}

/*---------------------------------------------------------------*/

static	inline	void	BladeSetAction( PlayerWork *work, int motion, int interp, int mask )
{
#ifdef ACTION1
	PL_SetAction( work, motion, interp ) ;
#else
	PL_SetAction2( work, motion, interp, mask ) ;
#endif
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
}

static	inline	int		BladeMotionTime( PlayerWork *work )
{
#ifdef ACTION1
	return PL_MotionTime( work ) ;
#else
	return PL_MotionTime2( work ) ;
#endif
}

static	inline	int		BladeEndMotion( PlayerWork *work )
{
#ifdef ACTION1
	return EndMotion( work ) ;
#else
	return EndMotion2( work ) ;
#endif
}
 
/* 斬る */
static	void	BladeSlash( PlayerWork *work, int time ) 
{
	int			*motion ;
	int			mtime, pmask, mask ;
	int			max, to ;
	float		t ;

	motion = SlashMotions[ AttackNo ] ;
	SetFlag( FLAG_CANNOT_CHANGE_ITEM | FLAG_NO_IK ) ;
	SetWeaponCamera( work ) ;
	work->arm_trigger |= ARM_ACTOBJECT_EX | ARM_BLADE | ( ARM_BLADE_SLASH << 12 ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;

	if ( time == 0 ) {
		PL_ChangeMotionArc( work, Motion ) ;
		if ( FirstAttack ) {
			BladeSetAction( work, motion[ 0 ], 0, 0x01ffe ) ;
			if ( work->data == 0 ) {
				MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / 6.0F ) ;
			}
			FirstAttack = 0 ;
		} else {
			BladeSetAction( work, motion[ 0 ], 0, 0x01ffe ) ;
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / 1.40F ) ;
			//BladeSetAction( work, motion[ 1 ], 4, 0x01ffe ) ;
			//work->data = 1 ;
		}	
		work->data = 0 ;

		SlashHoming( work ) ;
		work->idata = -1 ;
		work->sv.vx = 0 ;
		work->sv.vy = 0 ;	/* 右スティックカウンター */
		work->sv.vz = 0 ; 	/* 回転斬りカウンター */
		work->sv.pad = 0 ;	/* 斬り走り可能フラグ */
		work->act_name = BLADE_SLASH ;
	}

	UpdateCountWork( work ) ;

	max = PL_MoveLevel( work ) ;
	if ( !Status( PLAYER_WATCH ) && max != 0 && work->data != 5 && work->sv.pad != 0 ) {
		/* しゃがみチェック、飛び込みチェックはあえてつくってない */
		/* 張り付きチェック */
		if ( Liable == FRONT && 
			 GV_DiffDirAbs( WallTo, PadTo ) < 256 ) {
			work->seNoFlr = -1 ;
			if ( PadForce > PAD_CAUTION_TH ) work->sv.vx += 2 ;
			else							 work->sv.vx ++ ;
			if ( work->sv.vx >= 12 ) {
				work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
				EndBlade( work, 0xfffff ) ;
				SetMode( work, PL_StandCautionStill ) ;
				return ;
			}
		} else {
			work->sv.vx = 0 ;
		}

		if ( work->idata < 0 ) {
			t = 0.0F ;
		} else {
			t = ( work->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
				 + work->body.m_ctrl->mt3_ctrl[ 0 ].motion_time_base )
				/ work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
		}
		if ( max == 1 ) {
			PL_SetAction4( work, BMwalk, t, 6 ) ;	
			to = work->pad->dir ;			
			SetStatus( PLAYER_WALK ) ;
			PL_FootPrintAct( work->foot_work, 0, 10, 40 ) ;
		} else {
			PL_SetAction4( work, BMrun, t, 6 ) ;
			to = PL_ReactWall( work->pad->dir ) ;
			SetStatus( PLAYER_DASH ) ;
			PL_FootPrintAct( work->foot_work, 0, 1, 21 ) ;
		}
		if ( work->sv.pad != 2 ) {
			if ( work->idata >= 0 && GV_DiffDirAbs( work->idata, to ) < 256 ) {
				work->control.turn.vy = to ;
			}
			work->idata = to ;
		}
		SetStatus( PLAYER_MOVE ) ;
	} else {
		if ( Status( PLAYER_WATCH ) ) {
			SetFlag( FLAG_CANNOT_PEEP ) ;
			PL_SubjectTurn( work ) ;
		}
		PL_SetAction( work, BMstill, 6 ) ;
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_TSUMASAKI ].world, &work->fv ) ;
		work->idata = -1 ;
	}
		
#ifndef ACTION1
	pmask = work->body.m_ctrl->mt3_ctrl[ 1 ].mask ;
	mask = ( Status( PLAYER_MOVE ) ) ? 0x01ffe : 0xfffff ;
	if ( pmask != mask ) {
		work->body.m_ctrl->mt3_ctrl[ 1 ].mask = mask ;
		if ( mask & 1 ) {
			work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			work->body.m_ctrl->root_old_height = work->body.m_ctrl->root_height ;
		}
		MT_SetMotionInterp( work->body.m_ctrl, 6 * NTSC_TIME_BASE, mask ) ;
	}
	//if ( mask & 1 ) SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	if ( mask & 1 ) SetFlag( FLAG_MOTION_STEP_OVERRIDE ) ;
#endif
//	if ( !Status( PLAYER_MOVE ) && 
//		 work->body.m_ctrl->interp_count == 0 ) SlashAdjustPos( work ) ;

	if ( work->data != 3 && work->data != 4 ) MoveTarget( work ) ;	
	mtime = BladeMotionTime( work ) ;

#if 1
//	if ( SwingCount.flag == RS_ALL ) {
	if ( CheckCountWork() ) {
		if ( work->data != 5 && ( ++ work->sv.vz > 8 ) ) {
			BladeSetAction( work, BMslash_large, 6, mask ) ;
			work->camdir.vx = GM_CameraDir.vx = 0 ;
			work->data = 5 ;
		} 
	} else {
		work->sv.vz = 0 ;
	}
#endif
	switch( work->data ) {
	case 0 :			/* 振りモーション１ */
		UnsetWeaponCamera( work ) ;
		if ( BladeEndMotion( work ) ) {
			BladeSetAction( work, motion[ 1 ], 0, mask ) ;
			GV_MatToVec( &work->body.objs->objs[ HUMAN21_HIDARI_TSUMASAKI ].world, &work->fv ) ;
			work->data = 1 ;
		}
		work->sv.pad = 2 ;
		break ;
	case 1 :			/* 振りモーション２ */
		work->trigger = BLD_TRIG_SLASH | BLD_TRIG_FIRE ;
//		work->data2 = ContSlash( work, work->data2 ) ;
		work->sv.pad = 2 ;
		if ( BladeEndMotion( work ) ) {
			BladeSetAction( work, motion[ 2 ], 0, mask ) ;
			work->data = 2 ;
			work->sv.pad = 0 ;
		}
		if ( mtime < 5 ) UnsetWeaponCamera( work ) ;
		break ;
	case 2 :			/* 振りモーション３ */
		if ( mtime < 10 && CheckFrontWall( work ) ) {
			EndBlade( work, mask ) ;
//			StickPress = 2 ;
			StickPress = 1 ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
			return ;
		} 
		if ( mtime > 24 ) {
			UnsetWeaponCamera( work ) ;
		}
		if ( mtime < 10 ) {	
			work->trigger = BLD_TRIG_SLASH | BLD_TRIG_FIRE ;
		} 
		work->data2 = ContSlash1( work, work->data2 ) ;

		if ( mtime >= 16 && work->data2 == 1 ) {
			AttackNo = NextAttackNo ;
			StickDir = NextStickDir ;
//			StickDir = RStickDir( work ) ;
			if ( mtime > 22 ) FirstAttack = 1 ;
			SetMode( work, BladeSlash ) ;
			work->data = 1 ;
			return ;
		}
		if ( PL_CheckMotionTime2( work, 18 ) && PL_UseStickR( work ) ) {
//			PL_MotionSleep( work, 1 ) ;
            if ( work->data2 == 0 ) BladeSetAction( work, motion[ 3 ], 12, mask ) ;
            else {
				int		*m ;

				m = SlashMotions[ NextAttackNo ] ;
				BladeSetAction( work, m[ 3 ], 12, mask ) ;
				AttackNo = NextAttackNo ;
				StickDir = NextStickDir ;				
			}
			work->data = 3 ;
		}

		if ( mtime > 16 ) {
			work->sv.pad = 1 ;
			if ( work->pad->status & PAD_GUARD ) {
				StickPress = 0 ;
				StickDir = -1 ;
				FirstAttack = 0 ;
				EndBlade( work, mask ) ;
				SetMode( work, BladeGuard ) ;
				GM_SeSetMode( SD_A_SWIGUARD, &work->control.mov, GM_SEMODE_NORMAL ) ;
				return ;
			}
		}

		if ( BladeEndMotion( work ) ) {
			EndBlade( work, mask ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
		}
		break ;

	case 3 :		/* 振り後構え */
	  /* 弾は、避けれないのでバリアーは、スキップさせる */
		GM_TargetSetSkip( &BladeTarget ) ;
		UnsetWeaponCamera( work ) ;
		work->data2 = ContSlash1( work, work->data2 ) ;
//		work->data2 = ContSlash2( work, work->data2 ) ;
		if ( work->data2 == 1 ) {
			AttackNo = NextAttackNo ;
			StickDir = NextStickDir ;
//			StickDir = RStickDir( work ) ;
			if ( mtime > 22 ) FirstAttack = 1 ;
//			FirstAttack = 1 ;
			SetMode( work, BladeSlash ) ;
			work->data = 1 ;
			return ;
		} else if ( work->data2 == 2 ) {
			int		*m ;

			m = SlashMotions[ NextAttackNo ] ;
			if ( m[ 3 ] != work->motion2 ) {
				BladeSetAction( work, m[ 3 ], 12, mask ) ;
				AttackNo = NextAttackNo ;
				StickDir = NextStickDir ;							
			}
		}
#if 0
		/* このときだけ回転斬りチェック */
		if ( CheckCountWork() ) {
			if ( ++ work->sv.vz > 8 ) {
				BladeSetAction( work, BMslash_large, 6, mask ) ;
				work->data = 5 ;
			} 
		} else {
			work->sv.vz = 0 ;
		}
#endif
		if ( work->pad->status & PAD_GUARD ) {
			StickPress = 0 ;
			StickDir = -1 ;
			FirstAttack = 0 ;
			EndBlade( work, mask ) ;
			SetMode( work, BladeGuard ) ;
			GM_SeSetMode( SD_A_SWIGUARD, &work->control.mov, GM_SEMODE_NORMAL ) ;
			return ;
		}
		if ( !PL_UseStickR( work ) ) {
			if ( ++ work->sv.vy > 8 ) {
//				PL_MotionActive( work, 1 ) ;
				GM_ConfigObjectAction( &work->body, 1, motion[ 2 ], 18 * NTSC_TIME_BASE,
									   mask, 6 * NTSC_TIME_BASE ) ;
                work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;
                work->motion2 = motion[ 2 ] ;
                work->body.m_ctrl->merge_flag = 0x0002 ;
                UnsetFlag( FLAG_RESET_MOTION2_AT_END ) ;
			    work->data = 4 ;
                work->data2 = 0 ;
            }
		} else {
			work->sv.vy = 0 ;
		}
		work->sv.pad = 1 ;
		break ;
	case 4 :			/* 振り後構えから戻る */
		UnsetWeaponCamera( work ) ;
		work->data2 = ContSlash1( work, work->data2 ) ;
		if ( work->data2 == 1 ) {
			AttackNo = NextAttackNo ;
			//StickDir = NextStickDir ;
			StickDir = RStickDir( work ) ;
			if ( mtime > 22 ) FirstAttack = 1 ;
			//printf( "mtime4 %d\n", mtime ) ;
			//FirstAttack = 1 ;
			SetMode( work, BladeSlash ) ;
			work->data = 1 ;
			return ;
		}	   
		if ( BladeEndMotion( work ) ) {
			EndBlade( work, mask ) ;
			StickPress = 0 ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
		}		
		work->sv.pad = 1 ;
		break ;
	case 5 :		/* 回転斬り */
		//if ( PL_CheckMotionRate( work ) < 0.85F ) {
		if ( PL_MotionTime( work ) < 98 ) {
			work->trigger = BLD_TRIG_SLASH_STRONG | BLD_TRIG_FIRE ;
		}
		work->arm_trigger |= ARM_ACTOBJECT_EX | ARM_BLADE | ( ARM_BLADE_ROLL << 12 ) ;
		if ( mtime > 60 ) UnsetWeaponCamera( work ) ;

		if ( BladeEndMotion( work ) ) {
			EndBlade( work, mask ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
		}		
		work->sv.pad = 0 ;

		if ( Status( PLAYER_WATCH ) ) {
			FVECTOR		to = { 0.0F, 0.0F, 10.0F } ;
			FVECTOR		from ;

			SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
			//work->arm_trigger |= ARM_IS_COMBO ;
			PL_ObjPos( work, HUMAN21_ATAMA, &from ) ;
			DG_SetPos( PL_ObjWorld( work, HUMAN21_ATAMA ) ) ;
			DG_PutVector( &to, &to, 1 ) ;
			work->camdir.vy = GV_VecDir2FromTo( &from, &to ) ;
		}
	}

	if ( work->weapon_camera->on == 0 && 
		 Status( PLAYER_WATCH ) ) {
		work->trigger &= ~( BLD_TRIG_SLASH | BLD_TRIG_SLASH_STRONG ) ;
	}

	/* 突き */
	if ( work->data != 5 && ( work->pad->press & PAD_AR ) ) {
//		StickPress = 0 ;
//		StickDir = -1 ;
		FirstAttack = 0 ;
		PL_SetAction2( work, -1, 6, mask ) ;
		GM_TargetSetSkip( &BladeTarget ) ;
		SetMode( work, BladeThrust ) ;
	}
	/* 持ち替え */
	if ( work->data != 5 && CheckBladeReverse( work ) ) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		PL_SetAction2( work, -1, 6, mask ) ;
		GM_TargetSetSkip( &BladeTarget ) ;
	}
}

/* 持ち替え */
static	void	BladeReverse( PlayerWork *work, int time )
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_IK ) ;
	SetWeaponCamera( work ) ;
	work->arm_trigger |= ARM_ACTOBJECT_EX | ARM_BLADE | ( ARM_BLADE_CHANGE << 12 ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;

	if ( time == 0 ) {
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, BMchange, 6 ) ;
		work->camdir.vx = GM_CameraDir.vx = 0 ;
	}
	if ( PL_CheckMotionTime( work, 16 ) ) {
		int		buf = 2 ;
		/* メッセージで持ち替え */
		PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
		InvFlag ^= 1 ;
		GM_PlayerStateFlag ^= PL_BLADE_MODE_MINEUCHI ;
	}
	if ( EndMotion( work ) ||
		( PL_MotionTime( work ) > 24 && ( PL_UseStickR( work ) || work->pad->press ) ) ) {
		EndBlade( work, 0 ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;		
	}
}

/* 突き */
static	void	BladeThrust( PlayerWork *work, int time ) 
{
	int			mtime ;

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_IK ) ;
	SetWeaponCamera( work ) ;
	work->arm_trigger |= ARM_ACTOBJECT_EX | ARM_BLADE | ( ARM_BLADE_THRUST << 12 ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;

	if ( time == 0 ) {
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, BMthrust, 6 ) ;
		SlashHoming( work ) ;
		work->idata = -1 ;
		work->sv.vx = 0 ;
		work->act_name = BLADE_SLASH ;
//		PL_StopTurn( work ) ;
	}	

	mtime = PL_MotionTime( work ) ;
	if ( mtime >= 7 && mtime <= 22 ) work->trigger = BLD_TRIG_THRUST | BLD_TRIG_FIRE ;

	if ( mtime < 4 ) {
		UnsetWeaponCamera( work ) ;
	}
	if ( mtime > 48 ) {
		UnsetWeaponCamera( work ) ;
	}

	if ( mtime > 48 && RStickDir( work ) >= 0 ) {
		EndBlade( work, 0 ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	}

	if ( mtime > 48 && ( work->pad->status & PAD_GUARD ) ) {
		StickPress = 0 ;
		StickDir = -1 ;
		FirstAttack = 0 ;
		EndBlade( work, 0 ) ;
		SetMode( work, BladeGuard ) ;
		GM_SeSetMode( SD_A_SWIGUARD, &work->control.mov, GM_SEMODE_NORMAL ) ;
		return ;
	}

	if ( CheckFrontWall( work ) || EndMotion( work ) ) {
		EndBlade( work, 0 ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	}			
}

/* 防御 */
static	void	BladeGuard( PlayerWork *work, int time ) 
{
	int			pmask, mask ;
	int			max, to ;
	float		t ;

	SetFlag( FLAG_CANNOT_CHANGE_ITEM ) ;
	SetStatus( PLAYER_ITEM_DISABLE ) ;

	SetWeaponCamera( work ) ;
	work->arm_trigger |= ARM_ACTOBJECT_EX | ARM_BLADE | ( ARM_BLADE_GUARD << 12 ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;	

	if ( time == 0 ) {
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction2( work, BMguard_p, 6, 0x01ffe ) ;
		SlashHoming( work ) ;
		SetFlag( FLAG_NO_STEP ) ;
		GuardHit = 0 ;
		GuardSlash = 0 ;
		GuardLastTime = GV_Time ;
		work->idata = -1 ;
		work->sv.vx = 0 ;
		work->act_name = BLADE_SLASH ;

		PL_TargetCallbackFunc = DamageCheckCallback ;
	}

	max = PL_MoveLevel( work ) ;
	if ( !Status( PLAYER_WATCH ) && max != 0 ) {
		/* しゃがみチェック、飛び込みチェックはあえてつくってない */
		/* 張り付きチェック */
		if ( Liable == FRONT && 
			 GV_DiffDirAbs( WallTo, PadTo ) < 256 ) {
			work->seNoFlr = -1 ;
			if ( PadForce > PAD_CAUTION_TH ) work->sv.vx += 2 ;
			else							 work->sv.vx ++ ;
			if ( work->sv.vx >= 12 ) {
				work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
				EndBlade( work, 0xfffff ) ;
				SetMode( work, PL_StandCautionStill ) ;
				return ;
			}
		} else {
			work->sv.vx = 0 ;
		}

		if ( work->idata < 0 ) {
			t = 0.0F ;
		} else {
			t = ( work->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
				 + work->body.m_ctrl->mt3_ctrl[ 0 ].motion_time_base )
				/ work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
		}

		if ( max == 1 ) {
		    /* XBOXの機能拡張 ガードしながら階段 */
		    PL_SetAction4( work, UpsideStep() ? BMwalk : BMstair_walk,
				   t, 6 ) ;	
		    to = work->pad->dir ;			
		    SetStatus( PLAYER_WALK ) ;
		    PL_FootPrintAct( work->foot_work, 0, 10, 40 ) ;
		} else {
		    /* XBOXの機能拡張 ガードしながら階段 */
		    PL_SetAction4( work, UpsideStep() ? BMrun : BMstair_run,
				   t, 6 ) ;
		    to = PL_ReactWall( work->pad->dir ) ;
		    SetStatus( PLAYER_DASH ) ;
		    PL_FootPrintAct( work->foot_work, 0, 1, 21 ) ;
		}
		if ( work->idata >= 0 && GV_DiffDirAbs( work->idata, to ) < 256 ) {
			work->control.turn.vy = to ;
		}
		work->idata = to ;
		SetStatus( PLAYER_MOVE ) ;
	} else {
		if ( Status( PLAYER_WATCH ) ) {
			SetFlag( FLAG_CANNOT_PEEP ) ;
			PL_SubjectTurn( work ) ;
		}
		PL_SetAction( work, BMstill, 6 ) ;
		work->idata = -1 ;
	}
		
	pmask = work->body.m_ctrl->mt3_ctrl[ 1 ].mask ;
	mask = ( Status( PLAYER_MOVE ) ) ? 0x01ffe : 0xfffff ;
	if ( pmask != mask ) {
		work->body.m_ctrl->mt3_ctrl[ 1 ].mask = mask ;
		if ( mask & 1 ) {
			work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			work->body.m_ctrl->root_old_height = work->body.m_ctrl->root_height ;
		}
		MT_SetMotionInterp( work->body.m_ctrl, 6 * NTSC_TIME_BASE, mask ) ;
	}
	//if ( mask & 1 ) SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	if ( mask & 1 ) SetFlag( FLAG_MOTION_STEP_OVERRIDE ) ;

	MoveTarget( work ) ;

	/* 刀防御成功 */
	if ( GuardSlash ) {
		GuardSlash = 0 ;
		GuardSlashLastTime = GV_Time ;
		SetMode( work, BladeGuardSlash ) ;
		PL_SetAction2( work, -1, 6, mask ) ;
		return ;
	}

	switch( work->data ) {
	case 0 :
		if ( GuardHit ) {
			GuardHit = 0 ;
			NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
			PL_SetAction2( work, BMguard_hit1 + work->data2, 0, mask ) ;
			if ( ++ work->data2 > 2 ) work->data2 = 0 ;
			work->data = 1 ;
		}
		break ;
	case 1 :
		work->trigger = BLD_TRIG_GUARD_SLASH ;
		if ( GuardHit ) {
			GuardHit = 0 ;
			if ( GV_Time - GuardLastTime > 8 ) {
				GuardLastTime = GV_Time ;
				NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
				PL_SetAction2( work, BMguard_hit1 + work->data2, 1, mask ) ;
				if ( ++ work->data2 > 2 ) work->data2 = 0 ;
			}
		}		
		if ( EndMotion2( work ) ) {
			work->data = 0 ;
			PL_SetAction2( work, BMguard_p, 6, mask ) ;
		}
		break ;
	}
	
	if ( !( work->pad->status & PAD_GUARD ) ) {
		EndBlade( work, mask ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
	}
}

/* 刀攻撃防御 */
static	void	BladeGuardSlash( PlayerWork *work, int time )
{
	SetFlag( FLAG_CANNOT_CHANGE_ITEM ) ;
	SetStatus( PLAYER_ITEM_DISABLE ) ;

	SetWeaponCamera( work ) ;
	work->arm_trigger |= ARM_ACTOBJECT_EX | ARM_BLADE | ( ARM_BLADE_GUARD << 12 ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;

	if ( time == 0 ) {
		PL_ChangeMotionArc( work, Motion ) ;
		work->motion1 = -1 ;
		PL_SetAction( work, BMguard, 6 ) ;
		GuardSlash = 0 ;
		work->act_name = BLADE_SLASH ;
		PL_TargetCallbackFunc = DamageCheckCallback ;
		if ( GuardSlashNoMuteki ) work->fdata = -32.0F ;
		else					  work->fdata = -128.0F ;
		GuardSlashNoMuteki = 0 ;
		GM_SeSetMode( SD_A_SWORDHAD, &work->control.mov, GM_SEMODE_BOMB ) ;
		{
			extern	void 	*NewBladeSparkEffect( FMATRIX *mat, int flag ) ;			
			FMATRIX			mat ;
			FVECTOR			p1, p2 ;
			SVECTOR			rot ;
			
			PL_ObjPos( work, HUMAN21_MIGI_TE, &p1 ) ;
			PL_ObjPos( work, HUMAN21_HIDARI_TE, &p2 ) ;
			_sceVu0AddVector( &p1, &p1, &p2 ) ;
			_sceVu0ScaleVector( &p1, &p1, 0.50F ) ;
			_sceVu0SubVector( &p2, &p1, &OffCenter ) ;
			GV_VecToRot( &p2, &rot ) ;
			DG_SetPos2( &p1, &rot ) ;
			DG_GetPos( &mat ) ;
			NewBladeSparkEffect( &mat, 2 ) ;
		}
		NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
	}

	/* 弾はじきターゲットも一応置いておく */
	MoveTarget( work ) ;
	if ( GuardHit ) GuardHit = 0 ;

	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;

	/* 刀防御成功 */
	if ( GuardSlash ) {
		int			c = 0 ;

		GuardSlash = 0 ;
		if ( GV_Time - GuardSlashLastTime > 8 ) {
			SetMode( work, BladeGuardSlash ) ;
			c = 1 ;
		}
		GuardSlashLastTime = GV_Time ;
		if ( c ) return ;
	}

	{
		FVECTOR			step ;
		int				diff ;

		GV_SetVec3( &step, 0.0F, 0.0F, work->fdata ) ;
		if ( work->control.n_touches ) {
			diff = GV_DiffDirAbs( work->control.turn.vy + 2048, 
								  GV_VecDir2( &work->control.vecs[ 0 ] ) ) ;
			if ( diff < 128 ) {
				work->control.turn.vy -= 512 ;
				step.vz *= 1.50F ;
			}
		} 
		DG_SetPos2( &DG_ZeroVector, &work->control.turn ) ;
		DG_RotVector( &step, &step, 1 ) ; step.vy = 0.0F ;
		_sceVu0AddVector( &work->control.step, &work->control.step, &step ) ;
	}
	work->fdata = GV_NearExp8F( work->fdata, 0.0F ) ;	

	if ( EndMotion( work ) ) {
		if ( work->pad->status & PAD_GUARD ) {
			SetMode( work, BladeGuard ) ;
		} else {
			EndBlade( work, 0xfffff ) ;
			PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		}
	}
}

/* ブレードコンボ */
static	void	BladeComboCallback( TARGET *off, TARGET *def, void *ptr )
{
	PlayerWork			*work ;

	work = ( PlayerWork * )ptr ;
	if ( !( def->class & TARGET_HAZARD ) ) {
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
								    &work->control.mov, &def->center,
								    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT, 
								    HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY,
								    HZX_FLOOR_NO_PLAYER | HZX_FLOOR_NO_ENEMY ) ) {
			if ( def->weapon_type == off->weapon_type ) {
				def->damaged &= ~TARGET_POWER ;
			}
			def->weapon_type &= ~( off->weapon_type ) ;
			return ;
		}
	}
	GM_VctrlResetSkip( &work->vctrl ) ;
}

static	void	BladeCombo( PlayerWork *work, int time )
{
	int				mtime, ptime, mode, faint ;
	float			len ;
	FVECTOR			v ; 
	TARGET			*t ;
	POWER_TARGET	*p ;

	SetWeaponCamera( work ) ;
	work->arm_trigger |= ARM_BLADE | ( ARM_BLADE_COMBO << 12 ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_COMBO ) ;

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_CANNOT_PEEP | FLAG_NO_TOUCH_DAMAGE ) ;
	if ( time == 0 ) {
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, BMcombo, 6 ) ;
		PL_SetArmAction( work, AMhfb_combo ) ;
		SlashHoming( work ) ;
		GM_VctrlSetSkip( &work->vctrl ) ;
		work->sv.vx = 0 ;

		work->idata = PL_CheckMovRotLenSegment( work->control.hzx_id, &work->control.mov,
											    &DG_ZeroVector, &work->control.turn, 
											    2000.0F, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
											    HZX_FLOOR_NO_PLAYER ) ;
		if ( work->idata ) work->fdata = GV_VecLen3F2( &work->control.mov, &ResultPoint ) ;
	}

	if ( work->idata ) {
		PL_GetModelLength( work, &len, NULL ) ;
		len += 50.0F ;
		if ( len > work->fdata ) {
			DG_SetPos2( &DG_ZeroVector, &work->control.turn ) ;
			GV_SetVec3( &v, 0.0F, 0.0F, work->fdata - len ) ;
			DG_RotVector( &v, &v, 1 ) ;
			PL_AdjustXZ( work, &v ) ;
		}
	}

	mtime = PL_MotionTime( work ) ;
	ptime = work->sv.vx ;
	work->sv.vx = mtime ;

	mode = 0 ;
	if ( ptime < 16 && mtime >= 16 ) {
		if ( !( work->data & PL_PAD_PUNCH ) ) {
			EndBlade( work, 0 ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
			GM_VctrlStopVibration( &work->vctrl ) ;
			return ;
		}
		work->data &= ~PL_PAD_PUNCH ;
	} else if ( ptime < 29 && mtime >= 29 ) {
		if ( !( work->data & PL_PAD_PUNCH ) ) {
			EndBlade( work, 0 ) ;
			SetMode( work, PL_StillMode[ STAND ] ) ;
			GM_VctrlStopVibration( &work->vctrl ) ;
			return ;
		}
		work->data &= ~PL_PAD_PUNCH ;
	}

	work->data |= work->pad->press ;

	mode = -1 ;
	if ( ptime < 7 && mtime >= 7 ) mode = 0 ;
	else if ( ptime < 20 && mtime >= 20 ) mode = 1 ;
	else if ( ptime < 45 && mtime >= 45 ) mode = 2 ;

	if ( mtime > 45 ) UnsetWeaponCamera( work ) ;

	if ( mode >= 0 ) {
		FVECTOR force = { 0.0F, 0.0F, -100.0F } ;
		t = &OffenseTarg ;
		p = &PowerTarg ;
		GV_SetVec3( &v, 350.0F, 350.0F, 350.0F ) ;
		GM_SetTarget( t, TARGET_OFFENSE, 0, ENEMY_SIDE, &v, &DG_ZeroVector ) ;
		DG_SetPos( &work->body.objs->world ) ;
		DG_RotVector( &force, &force, 1 ) ;
		switch( mode ) {
		case 0 :
		case 1 :
			faint = 3 ;
		    GM_SetTargetWeaponType( t, WP_PUNCHR | WP_PLAYER ) ;
			GM_MoveTarget3( t, &work->body.objs->objs[ HUMAN21_MIGI_TE ].world ) ;
			break ;
		case 2 :
		default :
			faint = 2 ;
		    GM_SetTargetWeaponType( t, WP_KICK | WP_PLAYER ) ;
			GV_SetVec3( &v, 750.0F, 750.0F, 750.0F ) ;
			GM_SetTargetSize( t, &v ) ;
			GM_MoveTarget3( t, &work->body.objs->objs[ HUMAN21_MIGI_KAKATO ].world ) ;
			break ;
		}
		GM_SetPowerTarget( t, p, POWER_CONST, 255, faint, 0, &force ) ;
		GM_SetTargetCallBack( t, BladeComboCallback, ( void * )work ) ;
		GM_PutTarget( t ) ;
		GM_VctrlSetSkip( &work->vctrl ) ;
	}

	if ( EndMotion( work ) ) {
		EndBlade( work, 0 ) ;
		SetMode( work, PL_StillMode[ STAND ] ) ;
	}
}

/*---------------------------------------------------------------*/

/* 抜く */
static	void	BladePullOut( PlayerWork *work, int time ) 
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	if ( time == 0 ) {
//		PL_ChangeStance( work, STAND ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, BMpullout, 6 ) ;
        PL_LeaveSubject( work ) ;
	}
	if ( PL_CheckMotionTime( work, 15 ) ) {
		VisFlag = 1 ;
		work->trigger = BLD_TRIG_PULLOUT ;
	}
	if ( EndMotion( work ) ) {
		PL_UnsetInvincible( work ) ;
		PL_ReturnMotionArc( work ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
	}
}

static	void	BladePullOutfromGround( PlayerWork *work, int time ) 
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		PL_ChangeMotionArc( work, work->org_motion ) ;
		PL_SetAction( work, none_crouch_stand, 6 ) ;
        PL_LeaveSubject( work ) ;
	}
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, BladePullOut ) ;
	}
}

/* 納める */
static	void	BladePutBack( PlayerWork *work, int time )
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
	if ( time == 0 ) {
//		PL_ChangeStance( work, STAND ) ;
		PL_ChangeMotionArc( work, Motion ) ;
		PL_SetAction( work, BMputback, 6 ) ;
        PL_LeaveSubject( work ) ;
	}
	if ( PL_CheckMotionTime( work, 88 ) ) {
		Blade2Trig = BLD_TRIG_PUTBACK ;
		if ( Blade2 != NULL ) {
			GV_DestroyOtherActor( Blade2 ) ;
			Blade2 = NULL ;
		}
	}
#if 0
	if ( EndMotion( work ) || 
		 ( work->data == 0 && ( work->pad->status & PAD_Y ) ) ||
		 ( work->pad->status & ( PAD_UDLR ) ) ) {
#else
	if ( EndMotion( work ) ) {
#endif
		if ( Blade2 != NULL ) {
			GV_DestroyOtherActor( Blade2 ) ;
			Blade2 = NULL ;
		}
		PL_UnsetInvincible( work ) ;
		PL_ReturnMotionArc( work ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
	}
}

static	void	BladePutBackfromGround( PlayerWork *work, int time ) 
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
	SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
	if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		PL_ChangeMotionArc( work, work->org_motion ) ;
		PL_SetAction( work, none_crouch_stand, 6 ) ;
        PL_LeaveSubject( work ) ;
	}
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, BladePutBack ) ;
	}
}

/*---------------------------------------------------------------*/

/* ダメージコールバック */
static	void	DamageCallback( PlayerWork *work, TARGET *off, TARGET *def )
{
	printf( "blade dmg callback!\n" ) ;
	if ( work->current_mar == Motion ) {
//		PL_ChangeMotionArc( work, work->org_motion ) ;
        PL_ReturnMotionArc( work ) ;
	}
	if ( PL_TargetCallbackFunc == DamageCheckCallback ) {
		PL_TargetCallbackFunc = NULL ;
	}
	GM_TargetSetSkip( &BladeTarget ) ;
	StickPress = 0 ;
	StickDir = -1 ;
}

/* ブレード解除コールバック */
static	void	UnequipCallback( PlayerWork *work, int time )
{
	/* 納め用カタナ起動 */
	if ( PullOutFlag ) {
		Blade2Trig = 0 ;
		Blade2 = NewBlade2( &work->control, &work->weapon_body, 
						    &work->weapon_unit, &Blade2Trig, ENEMY_SIDE, InvFlag ) ;
		GM_SetPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ;
	}
	if ( PL_CheckAttackFunc == BladeCheckAttack ) PL_CheckAttackFunc = NULL ;
	if ( PL_TargetCallbackFunc == DamageCheckCallback ) {
		PL_TargetCallbackFunc = NULL ;
	}
	GM_TargetSetSkip( &BladeTarget ) ;
	StickPress = 0 ;
	StickDir = -1 ;
}

/* ブレードアクション中 */
static	int		IsBladeAction( PlayerWork *work )
{
	int			i ;

	for ( i = 0; i < sizeof( BladeActions ) / sizeof( PL_ACTION ); i ++ ) {
		if ( work->action == BladeActions[ i ] ||
			 work->action2 == BladeActions[ i ] ) return 1 ;
	}
	return 0 ;
}

/* ポーリング関数 */
static	int		CheckBladePullOut( PlayerWork *work )
{
	/* ゲームオーバー中 */
	if ( GM_IsGameOver() ) {
		//printf( "blade polling cancel\n" ) ;
		return 0 ;
	}

	if ( work->weapon != WP_Blade ) {
		/* ブレードでない */
		if ( PullOutFlag ) {
			int			c = 0 ;
			GM_TargetSetSkip( &BladeTarget ) ;
			if ( work->act_name == STAND_STILL ||
				 work->act_name == SQUAT_STILL ||
				 work->act_name == STAND_RUN || 
				 ( c = IsBladeAction( work ) ) ) {
				PL_SetMode( work, BladePutBack ) ;
				if ( c ) work->data = 1 ;
				PL_SetInvincible( work, 4 ) ;
				SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
				PullOutFlag = 0 ;
				VisFlag = 0 ;
				work->dmg_callback = DamageCallback ;
				return 1 ;
			} else if ( work->act_name == GROUND_STILL ||
					    work->act_name == GROUND_MOVE_FRONT ||
					    work->act_name == GROUND_MOVE_BACK ) {
				PL_SetMode( work, BladePutBackfromGround ) ;
				PL_SetInvincible( work, 4 ) ;
				SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
				PullOutFlag = 0 ;
				VisFlag = 0 ;
				work->dmg_callback = DamageCallback ;
				return 1 ;
			} else {
				PullOutFlag = 0 ;
				VisFlag = 0 ;
				if ( work->current_mar == Motion ) {
					PL_ChangeMotionArc( work, work->org_motion ) ;
				}
			}
		}
		if ( Blade2 != NULL && 
			 work->action != BladePutBack &&
			 work->action != BladePutBackfromGround ) {
			GV_DestroyOtherActor( Blade2 ) ;
			Blade2 = NULL ;			
		}
		if ( PL_TargetCallbackFunc == DamageCheckCallback ) {
			PL_TargetCallbackFunc = NULL ;
		}
		if ( work->dmg_callback == DamageCallback && 
			 work->action != BladePutBack &&
			 work->action != BladePutBackfromGround &&
			 work->current_mar != Motion ) {
			work->dmg_callback = NULL ;
		}
#if 0
			int			c = 0 ;
			GM_TargetSetSkip( &BladeTarget ) ;
			if ( work->act_name == STAND_STILL ||
				 work->act_name == SQUAT_STILL ||
				 work->act_name == STAND_RUN || 
				 ( c = IsBladeAction( work ) ) ) {
				PL_SetMode( work, BladePutBack ) ;
				if ( c ) work->data = 1 ;
				PL_SetInvincible( work, 0 ) ;
				SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
				PullOutFlag = 0 ;
				VisFlag = 0 ;
				return 1 ;
			} 
			PullOutFlag = 0 ;
			VisFlag = 0 ;
			if ( work->current_mar == Motion ) {
				PL_ReturnMotionArc( work ) ;
			}
		}
		if ( Blade2 != NULL && work->action != BladePutBack ) {
			GV_DestroyOtherActor( Blade2 ) ;
			Blade2 = NULL ;			
		}
		if ( PL_TargetCallbackFunc == DamageCheckCallback ) {
			PL_TargetCallbackFunc = NULL ;
		}
		if ( work->dmg_callback == DamageCallback && 
			 work->action != BladePutBack &&
			 work->current_mar != Motion ) {
			work->dmg_callback = NULL ;
		}
#endif
		return 0 ;
	} 
	/* ブレード装備 */

	/* 以下のときしか装備できない */
	if ( PullOutFlag == 0 &&
		 work->act_name != STAND_STILL &&
		 work->act_name != SQUAT_STILL &&
		 work->act_name != STAND_RUN   &&
		 work->act_name != GROUND_STILL &&
		 work->act_name != GROUND_MOVE_FRONT &&
		 work->act_name != GROUND_MOVE_BACK ) {
		printf( "blade cannot equip!\n" ) ;
		return 0 ;
	}
	
	if ( Blade2 != NULL ) {
		/* 前のが残ってたら終了する */
		GV_DestroyOtherActor( Blade2 ) ;
		Blade2 = NULL ;
	}
	/* ダメージコールバック設定 */
	work->dmg_callback = DamageCallback ;

	if ( PullOutFlag ) {
		if ( VisFlag == 0 && 
			 work->action != BladePullOut && 
			 work->action != BladePullOutfromGround ) {	/* まだ表示されてない */
			int		buf = 1 ;
			/* メッセージで表示させる */
			PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
			VisFlag = 1 ;
		}
		/* 攻撃 */
		return 0 ;
	} 

	/* 攻撃チェック関数を登録 */
	PL_CheckAttackFunc = BladeCheckAttack ;	
	StickPress = 0 ;
	InitCountWork() ;
	
	/* さかさ */
	if ( InvFlag ) {
		int		buf = 2 ;
		PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
	}

	PullOutFlag = 1 ;
	if ( work->act_name == STAND_STILL ||
		 work->act_name == SQUAT_STILL ||
		 work->act_name == STAND_RUN ) {
		PL_SetMode( work, BladePullOut ) ;
		PL_SetInvincible( work, 4 ) ;
		return 1 ;
	} else if ( work->act_name == GROUND_STILL ||
			    work->act_name == GROUND_MOVE_FRONT ||
			    work->act_name == GROUND_MOVE_BACK ) {
		PL_SetMode( work, BladePullOutfromGround ) ;
		PL_SetInvincible( work, 4 ) ;
		return 1 ;		
	} else {
		int		buf = 1 ;

		/* メッセージで表示させる */
		PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
		VisFlag = 1 ;
	}
#if 0
	if ( Blade2 != NULL ) {
		/* 前のが残ってたら終了する */
		GV_DestroyOtherActor( Blade2 ) ;
		Blade2 = NULL ;
	}
	/* ダメージコールバック設定 */
	work->dmg_callback = DamageCallback ;

	if ( PullOutFlag ) {
		if ( VisFlag == 0 && 
			 work->action != BladePullOut ) {	/* まだ表示されてない */
			int		buf = 1 ;
			/* メッセージで表示させる */
			PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
			VisFlag = 1 ;
		}
		/* 攻撃 */
		return 0 ;
	} 

	/* 攻撃チェック関数を登録 */
	PL_CheckAttackFunc = BladeCheckAttack ;	
	StickPress = 0 ;
	InitCountWork() ;
	
	/* さかさ */
	if ( InvFlag ) {
		int		buf = 2 ;
		PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
	}

	PullOutFlag = 1 ;
	if ( work->act_name == STAND_STILL ||
		 work->act_name == SQUAT_STILL ||
		 work->act_name == STAND_RUN ) {
		PL_SetMode( work, BladePullOut ) ;
		PL_SetInvincible( work, 0 ) ;
		return 1 ;
	} else {
		int		buf = 1 ;

		/* メッセージで表示させる */
		PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
		VisFlag = 1 ;
	}
#endif
	return 0 ;
}

/*---------------------------------------------------------------*/

typedef	struct	{
	GV_ACT			actor ;
} BLADE_MNG_WORK ;

static	BLADE_MNG_WORK	BladeMngWork ;
static  int             BladeMngAlive = 0 ;/* 一つしか上がらないようにするため */

static	void	BladeMngWorkDie( BLADE_MNG_WORK *work )
{
    if ( BladeMngAlive == 1 ) {
	BladeMngAlive = 0 ;

	PL_WeaponSets[ WP_Blade ].func = NULL ;
	PL_WeaponSets[ WP_Blade ].action = NULL ;
	PL_WeaponSets[ WP_Blade ].type = BLADE_TYPE ;
	PL_WeaponSets[ WP_Blade ].shoot = NULL ;
	PL_WeaponSets[ WP_Blade ].callback = NULL ;
	if ( PullOutFlag ) GM_PlayerStateFlag |= PL_START_STATE_BLADE_OUT ;
	if ( InvFlag ) GM_PlayerStateFlag |= PL_START_STATE_BLADE_INV ;
	GM_FreeTarget( &BladeTarget ) ;

	/* 以下のは T.Morita 追加 途中で死んだ時のため */
	if ( VisFlag ) {
	    int buf = 1 ;
	    PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 0 ) ;
	    VisFlag = 0 ;
	}
	if ( PL_CheckAttackFunc == BladeCheckAttack ){
	    PL_CheckAttackFunc = NULL ;
	}
	if ( PL_TargetCallbackFunc == DamageCheckCallback ) {
	    PL_TargetCallbackFunc = NULL ;
	}
	if ( GM_PlayerWork->current_mar == Motion ) {
		PL_ReturnMotionArc( GM_PlayerWork ) ;
		PL_SetMode( GM_PlayerWork, PL_StillMode[ STAND ] ) ;
	}
	PL_RemovePollingFunc( &PollingSet ) ;
	if ( Blade2 != NULL ) {
	    GV_DestroyOtherActor( Blade2 ) ;
	    Blade2 = NULL ;
	}
    }
}

/* ターゲット設定 */
static	void	BladeSetTarget( void )
{
	TARGET		*t ;
	FVECTOR		size, offset ;

	t = &BladeTarget ;
	GV_SetVec3( &size, 400.0F, 1000.0F, 250.0F ) ;
	GV_SetVec3( &offset, 0.0F, 0.0F, 600.0F ) ;
	GM_SetTarget( t, TARGET_DEFENSE| TARGET_ROTATE | TARGET_POWER, 0,
				  PLAYER_SIDE, &size, &offset ) ;
	GM_SetTargetCallBack( t, Hitted, NULL ) ;
	GM_PutTarget( t ) ;
	GM_TargetSetSkip( t ) ;
}

/* プラグイン登録 */
int		NewPluginBlade( void )
{
    BLADE_MNG_WORK		*work ;

    if ( BladeMngAlive == 0 ) {
	BladeMngAlive = 1 ;

	PL_SetMotionChangeSets( WP_Blade, BladeSet ) ;
	PL_WeaponSets[ WP_Blade ].func = NewBlade ;
	PL_WeaponSets[ WP_Blade ].action = NULL ;
	PL_WeaponSets[ WP_Blade ].type = BLADE_TYPE ;
	PL_WeaponSets[ WP_Blade ].shoot = NULL ;
	PL_WeaponSets[ WP_Blade ].callback = UnequipCallback ;

	work = &BladeMngWork ;
	GV_ZeroMemory( work, sizeof( BLADE_MNG_WORK ) ) ;
	GV_SetActorFreeFunc( &work->actor, NULL ) ;
	GV_SetActorClass( &work->actor, GV_CLASS_OBJECT ) ;
	GV_InsertActorPriority( GV_ACTOR_MANAGER, &work->actor, 0 ) ;
	GV_SetActor( &work->actor, NULL, BladeMngWorkDie ) ;
	GV_SetActorKillLevel( &work->actor, GV_KILL_LEVEL_NORMAL ) ;

	if ( GM_PlayerStateFlag & PL_START_STATE_BLADE_OUT ) {
		PullOutFlag = 1 ;
		VisFlag = 1 ;
		{
			int		buf = 1 ;

			/* メッセージで表示させる */
			PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
		}
		PL_CheckAttackFunc = BladeCheckAttack ;	
		InitCountWork() ;
	} else {
		PullOutFlag = 0 ;
		VisFlag = 0 ;
	}
	if ( GM_PlayerStateFlag & PL_START_STATE_BLADE_INV ) {
		InvFlag = 1 ;
		{
			int		buf = 2 ;

			PL_SendMessage( GV_StrCode( "ブレード" ), &buf, 1 ) ;
		}
		GM_PlayerStateFlag |= PL_BLADE_MODE_MINEUCHI ;
	} else {
		InvFlag = 0 ;
		GM_PlayerStateFlag &= ~PL_BLADE_MODE_MINEUCHI ;
	}
	GM_PlayerStateFlag &= ~( PL_START_STATE_BLADE_OUT | PL_START_STATE_BLADE_INV ) ;

	PL_AddPollingFunc( &PollingSet, CheckBladePullOut ) ;

	Motion = GCL_GetOptionValue( 'm', 0 ) ;

	Blade2 = NULL ;
	FirstAttack = 0 ;
	StickDir = -1 ;
	SlashFillip = 0 ;
	GuardHit = 0 ;
	GuardLastTime = 0 ;
	GuardSlash = 0 ;
	GuardSlashLastTime = 0 ;
	NoCheckSegment = 0 ;

	BladeSetTarget() ;
    }

    return 0 ;
}

/*---------------------------------------------------------------*/

/* 刀をはじかれた */
void	PL_FillipSlash( void )
{
	SlashFillip = 1 ;	
	GM_PlayerControl->turn.vy += ( GV_Time & 1 ) ? 512 : -512 ;
}

/* 今刀振り中かチェック */
int		PL_SlashNow( void )
{
	if ( GM_PlayerWork->trigger & BLD_TRIG_FIRE ) return 1 ;
	return 0 ;
}

/* 振ったとき、壁チェックしなくする */
int		PL_COM_BladeNoCheckSegment( void )
{
	NoCheckSegment = 1 ;	
	return 0 ;
}

int		PL_COM_BladeNoCheckSegmentCancel( void )
{
	NoCheckSegment = 0 ;	
	return 0 ;
}


/* プラグインを外してしまう */
int		PL_COM_BladeKillPlugin( void )
{
        GV_DestroyOtherActor( &BladeMngWork ) ;
	return 0 ;
}


/* 振り種類を返す */
int		PL_SlashMode( void )
{
	if ( GM_PlayerWork->action == BladeThrust ) return ATK_THRUST ;
	return AttackNo ;
}

/* ガード中かどうか */
int		PL_SlashGuardNow( void )
{
	if ( GM_PlayerWork->action == BladeGuard ||
		 GM_PlayerWork->action == BladeGuardSlash ) {
		return 1 ;
	}
	return 0 ;
}

/* なんかガードしました */
void	PL_AttackGuardBlade( FVECTOR *mov )
{
	if ( !PL_SlashGuardNow() ) return ;
	GuardSlash = 1 ;
	GM_PlayerControl->turn.vy 
		= GV_VecDir2FromTo( &GM_PlayerControl->mov, mov ) ;
}

/*----------------------------------------------------------------*/

int		PL_COM_BladeSlashCheck( void )
{
	return PL_SlashNow() ;
}
