//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   kwt_mst.c
   共和刀＆民主刀

   2001/03/23	M.Sonoyama
   $Id: kwt_mst.c,v 1.1.1.3 2002/11/19 11:51:05 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"sol_common.h"
#include	"rand.h"

#define	KWT_NAME		(3014850)	/* kwt_r_mt */
#define	MST_NAME		(2822082)	/* mst_l_mt */
#define	BODY_FLAG		(DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

enum {
	THIS_IS_KWT		= 0,
	THIS_IS_MST,
} ;

static	FVECTOR	KwtShift[] = {
	{ 18.0F, -135.0F, 87.0F },
	{ 18.0F, -320.0F, 800.0F }
} ;

static	FVECTOR	MstShift[] = {
	{ -18.0F, -135.0F, 87.0F },
	{ -18.0F, -285.0F, 607.0F }
} ;

static	int		Bodys[] = {
	KWT_NAME,
	MST_NAME
} ;

static	FVECTOR	*Shifts[] = {
	KwtShift, 
	MstShift,
} ;

typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			body ;
	TARGET			offense ;
	POWER_TARGET	of_power ;
	FVECTOR			pos[ 2 ] ;
	void			*int_poly ;
	CONTROL			*pctrl ;
	OBJECT			*pbody ;
	int				unit ;
	int				*trigger ;
	int				this ;
	int				flag ;
	int				trg ;
	int				eff_alpha ;
    int		attack_flag;
} Work ;

extern void *NewInterPoly_Demo( FVECTOR *pos0, FVECTOR *pos1, int disp_f, SVECTOR *col ) ;
extern void *NewBladeEft( FVECTOR *pos0, FVECTOR *pos1, int n_disp, int init_col, int *alpha ) ;
extern void *NewSolidusBladeLight( FMATRIX *root, int kind, int *flag );
extern void *NewSolidusBladeWind( FVECTOR *pos0, FVECTOR *pos1, int *alpha );

extern	void	PL_SetHitSE( int se ) ;

static	int			Damage = 0 ;
static	float		BladeLenPlus = 0.0F ;
static	long64		WeaponTypeTmp = I64(0) ;
static	float		ForceTmp = -1.0F ;

/*----------------------------------------------------------------*/

static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	work = ( Work * )ptr ;

	if ( GM_PlayerControl != NULL && 
		 def->name == GM_PlayerControl->name ) {
		/* ローリング中は当たらない */
		if ( GM_CheckPlayerStatus( PLAYER_ROLLING ) ) {
			if ( def->weapon_type == off->weapon_type ) {
				def->damaged &= ~TARGET_POWER ;
			}
			def->weapon_type &= ~off->weapon_type ;
		}
	}
	if ( !PL_SlashNow() && !PL_SlashGuardNow() &&
		 !GM_CheckPlayerStatus( PLAYER_INVINCIBLE | PLAYER_ROLLING ) ) {
		if ( work->trg == BLD_TRIG_THRUST ) {
			GM_SeSetMode( SD_E_TKENBIT1, &work->pctrl->mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_E_TKENCUT1, &work->pctrl->mov, GM_SEMODE_BOMB ) ;
		}
	}
}

/*----------------------------------------------------------------*/

static	inline	void	UpdateMap( Work *work )
{
	GM_SetCurrentMap( work->pctrl->map ) ;
	work->body.map_name = GM_CurrentMap ;
	GM_GroupObjs( work->body.objs, GM_CurrentMap ) ;
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int			trg, fire ;
	int			damage ;

	UpdateMap( work ) ;

	trg = work->trg = ( *work->trigger ) & ( ~BLD_TRIG_FIRE_ALL ) ;
	fire = ( *work->trigger ) & BLD_TRIG_FIRE_ALL ;
	if ( fire ) {
		FVECTOR		from, to, *shift ;
		FVECTOR		diff ;

		DG_SetPos( work->body.objs->root ) ;
		shift = Shifts[ work->this ] ;
		DG_PutVector( &shift[ 0 ], &from, 1 ) ;
		DG_PutVector( &shift[ 1 ], &to, 1 ) ;
		_sceVu0SubVector( &diff, &to, &from ) ;
		GV_LenVec3F( &diff, &diff, 0.0F, GV_VecLen3F( &diff ) + BladeLenPlus ) ;
		_sceVu0AddVector( &to, &from, &diff ) ;

		/* from を５０ｃｍさげて当たりやすく */
		_sceVu0SubVector( &diff, &from, &to ) ;
		GV_LenVec3F( &diff, &diff, 0.0F, GV_VecLen3F( &diff ) + 500.0F ) ; 
		_sceVu0AddVector( &from, &to, &diff ) ;

		{
			FMATRIX		world ;
			FVECTOR		center, size ;
			SVECTOR		rot ;
			
			_sceVu0AddVector( &center, &to, &from ) ;
			_sceVu0ScaleVector( &center, &center, 0.50F ) ;
			_sceVu0SubVector( &diff, &to, &from ) ;
			size.vz = GV_VecLen3F( &diff ) / 2.0F ;
			size.vx = size.vy = 250.0F ;
			GM_SetTargetSize( &work->offense, &size ) ;
			GV_VecToRot( &diff, &rot ) ;
			DG_SetPos2( &center, &rot ) ;
			DG_GetPos( &world ) ;
			GM_MoveTarget2( &work->offense, &world ) ;
			//NewTargetView2( &work->offense, 32, 232, 32 ) ;			
		}
#if 0
		GM_MoveOnlineTarget( &work->offense, &from, &to ) ;
#endif
		//_sceVu0SubVector( &work->offense.power->force, &to, &from ) ;
		//GV_LenVec3F( &work->offense.power->force, &work->offense.power->force, 0.0F, 128.0F ) ;

		{
			SVECTOR		rot ;
			float		force ;

			force = ( ForceTmp > 0.0F ) ? ForceTmp : 128.0F ;
			ForceTmp = -1.0F ;
			GV_SetVec3( &work->offense.power->force, 0.0F, 0.0F, 128.0F ) ;
			rot.vy = GV_VecDir2FromTo( &work->pctrl->mov, &GM_PlayerPosition ) ;
			rot.vx = rot.vz = 0 ;
			if ( GV_DiffDirAbs( rot.vy, work->pctrl->rot.vy ) > 1024 ) {
				rot.vy += 2048 ;
			}
			DG_SetPos2( &work->pctrl->mov, &rot ) ;
			DG_RotVector( &work->offense.power->force, &work->offense.power->force, 1 ) ;
		}

		if ( WeaponTypeTmp != I64(0) ) {
			GM_SetTargetWeaponType( &work->offense, WeaponTypeTmp ) ;
		} else {
			GM_SetTargetWeaponType( &work->offense, WP_SHOTGUN_NEAR | WP_BLADEGUARD ) ;
		}
		WeaponTypeTmp = I64(0) ;

		damage = Damage ;
		/* 難易度別 */
		switch( SOL_GameLevel ) {
		case GM_LEVEL_VERYEASY :
			damage -= 2 ;
			break ;
		case GM_LEVEL_EASY :
			damage -= 1 ;
			break ;
		case GM_LEVEL_HARD :
			damage += 1 ;
			break ;
		case GM_LEVEL_EXTREME :
			damage += 2 ;
			break ;
		case GM_LEVEL_NORMAL :
		default :
		  ;
		}
		if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
		if ( damage <= 0 ) damage = 1 ;
		work->of_power.damage = damage ;

		if ( trg == BLD_TRIG_SLASH ) {
			PL_SetHitSE( SD_E_TKENCUT1 ) ;
		} else if ( trg == BLD_TRIG_THRUST ) {
			PL_SetHitSE( SD_E_TKENBIT1 ) ;
		}

		work->offense.class &= ~TARGET_DIRECT_ATTACK ;
		GM_PutTarget( &work->offense ) ;
		/* もしエルード中ならダイレクトであてる */
		if ( GM_PlayerTarget != NULL &&
			 GM_CheckPlayerStatus( PLAYER_BEYOND ) &&
			 !GM_CheckPlayerStatus( PLAYER_FORCE ) &&
			GV_VecLen3F2( &GM_PlayerPosition, &work->offense.center ) < 2000.0F ) {
			GM_TargetSetDirectAttack( &work->offense, GM_PlayerTarget ) ;
		}
	}

	{
		FVECTOR		*shift ;

		DG_SetPos( work->body.objs->root ) ;
		shift = Shifts[ work->this ] ;
		DG_PutVector( &shift[ 0 ], &work->pos[ 0 ], 1 ) ;
		DG_PutVector( &shift[ 1 ], &work->pos[ 1 ], 1 ) ;
	}

	if ( trg == BLD_TRIG_SLASH ||
		 trg == BLD_TRIG_THRUST ) {
		work->eff_alpha = 16;
		if ( work->int_poly == NULL ) {
			SVECTOR			col = { 32, 96, 196, 32 } ;			

			work->int_poly = NewInterPoly_Demo( &work->pos[ 0 ], &work->pos[ 1 ],
			 								    4, &col ) ;
			//work->int_poly = NewBladeEft( &work->pos[ 0 ], &work->pos[ 1 ], 8, 
			//							  0x040a0e00, &work->eff_alpha ) ;			
			GV_SetActorChild( work, work->int_poly ) ;
		} 
		work->attack_flag = 1;
	} else {
		if ( work->int_poly != NULL ) {
			GV_DestroyOtherActor( work->int_poly ) ;
			work->int_poly = NULL ;
			//work->eff_alpha = GV_NearSpeed( work->eff_alpha, 0, 3 ) ;
		}		
		work->eff_alpha = GV_NearSpeed( work->eff_alpha, 0, 4 ) ;
		work->attack_flag = 0;
	}
}

static	void	Die( Work *work )
{
	GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/

static	void	InitTarget( Work *work )
{
	TARGET		*t ;

	t = &work->offense ;
//	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE, 0,
//				  PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ROTATE, 0,
				  PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetPowerTarget( t, &work->of_power, POWER_ONCE, 255, 0,
					   5, &DG_ZeroVector ) ;
	GM_SetTargetName( t, work->pctrl->name ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
	GM_SetTargetWeaponType( t, WP_SHOTGUN_NEAR | WP_BLADEGUARD ) ;
}

static	int		GetResources( Work *work )
{
	OBJECT		*body ;

	body = &work->body ;
	GM_InitObject( body, Bodys[ work->this ], BODY_FLAG ) ;
	ASSERT( body->objs != NULL ) ;
	DG_ConnectObjs( work->pbody->objs, body->objs ) ;
	GM_ConfigObjectRoot( body, work->pbody, work->unit ) ;

	InitTarget( work ) ;

	Damage = 5 ; 
	WeaponTypeTmp = I64(0) ;
	ForceTmp = -1.0F ;

	/* ブレード光呼び出し */
	work->attack_flag = 0;
	NewSolidusBladeLight( &work->pbody->objs->objs[work->unit].world, work->this, &work->attack_flag );

	/* ブレード風呼び出し */
	NewSolidusBladeWind( &work->pos[ 0 ], &work->pos[ 1 ], &work->eff_alpha );

	return 0 ;
}

static	void	*New( CONTROL *ctrl, OBJECT *body, int unit, int *trigger, int this )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT,
									 sizeof( Work ), 16 ) ; /* ソリダス本体より後 */
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->pctrl = ctrl ;
		work->pbody = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->this = this ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/*----------------------------------------------------------------*/

/* 共和刀 */
void	*NewKyouwaBlade( CONTROL *ctrl, OBJECT *body, int unit, int *trigger )
{
	return New( ctrl, body, unit, trigger, THIS_IS_KWT ) ;
}

/* 民主刀 */
void	*NewMinshuBlade( CONTROL *ctrl, OBJECT *body, int unit, int *trigger )
{
	return New( ctrl, body, unit, trigger, THIS_IS_MST ) ;
}

/* ダメージ値設定 */
void	SOL_SetBladeDamage( int damage, float lenplus )
{
	Damage = damage ; 	
	BladeLenPlus = lenplus ;
}

/* 武器タイプ設定 */
void	SOL_SetBladeWeaponType( long64 weapon_type )
{
	WeaponTypeTmp = weapon_type ;
}

/* フォース設定 */
void	SOL_SetBladeForce( float v )
{
	ForceTmp = v ;
}
