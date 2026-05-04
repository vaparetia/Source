//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   blade.c
   愛国ブレード
   
   2000/05/23 M.Sonoyama
   $Id: blade.c,v 1.1.1.3 2002/11/19 11:50:26 Yoshizawa1 Exp $
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
#include	"camera.h"



#define	BODY_FLAG		(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
						 DG_FLAG_ONEPIECE)
#define	BODY_DATA			MDL_BLADE
#define	BODY_DATA_INV		MDL_BLADE_INV
#define	BLADE_LENGTH	(1000.0F)

extern	void 	*NewInterPoly_Demo( FVECTOR *pos0, FVECTOR *pos1, int disp_f, SVECTOR *col ) ;
extern 	void 	*NewBladeEft( FVECTOR *pos0, FVECTOR *pos1, int n_disp, int init_col, int *alpha ) ;
extern void *NewKatanaAppear(FMATRIX *world,int mode ,int *sw ) ;

extern int	DG_ReverseOrderDrawMode ; /*libdg/xchanl.c*/


typedef	struct {
    GV_ACT_EX		actor ;
    OBJECT			weapon ;
	OBJECT			weapon_inv ;
    OBJECT			weapon_sub ;
	OBJECT			weapon_inv_sub ;
    OBJECT			weapon_mir ;      /*鏡の映りこみとしてのみ入れる*/
    OBJECT			weapon_inv_mir ;  /**/
	TARGET			target ;
	POWER_TARGET	power ;
	FVECTOR			prev_pos[ 3 ] ;
	FVECTOR			pos[ 3 ] ;
	void			*int_poly ;
    CONTROL			*ctrl ;
    OBJECT			**body ;
    int				*unit ;
    u_int			*trigger ;	
	int				side ;
	int				flag ;
	int				faint_atk ;
	int				faint_atk_count ;
	int				eff_alpha ;

    int efct_sw ;/* エフェクト用のスイッチ */
} Work ;

enum {
	BLD_FLAG_NONE =		0x0000,
	BLD_FLAG_PULLOUT =	0x0001,
	BLD_FLAG_INV =		0x0002,		/* 逆手 */
	BLD_FLAG_PULLOUT_MSG =	0x0004,	/* メッセージで強制表示 */
	BLD_FLAG_REAL =		0x1000,		/* 本体 */
} ;

static	FVECTOR			Blade0 = { 10.0F, -118.0F, 75.0F } ;	/* 鍔元 */

/* 昔の短い奴 */
//static	FVECTOR			Blade1 = { 10.0F, -373.0F, 620.0F } ;	/* 刃先 */
//static	FVECTOR			Blade2 = { 10.0F, -651.0F, 783.0F } ;	/* 刃先あたり */

static	FVECTOR			Blade1 = { 13.454841F, -501.865540F, 834.200134F } ;	/* 刃先 */
static	FVECTOR			Blade2 = { 13.454841F, -501.865540F, 834.200134F } ;	/* 刃先あたり */
//static	FVECTOR			Blade2 = { 16.909681F, -630.731079F, 1048.400269F } ;	/* 刃先あたり */


//static	long64			FaintAtk[] = { WP_PUNCHR, WP_PUNCHL, WP_KICK } ;

static	int				BladeEffectScn = 0 ;
 
/*----------------------------------------------------------------*/

static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	work = ( Work * )ptr ;
	if ( work->flag & BLD_FLAG_INV ) {
		/* 天狗兵の刀防御ターゲットだったら無視する */
		if ( def->name == WP_Psg1 ||
			 def->name == WP_m92 ) return ;
		work->faint_atk_count = 16 ;
	}
}

/*----------------------------------------------------------------*/

/* 鞘柄の表示きり変え */
static	inline	void	BladeGripOnOff( int *which )
{
	extern int	PL_SendMessage( int, int *, int ) ;	/* raiden/routine.c */
	PL_SendMessage( PL_EQUIPMENT_MANAGER_NAME, which, 1 ) ;
}

static	void	BladeGripOn( void )
{
	int			buf = 2 ;
	BladeGripOnOff( &buf ) ;
}

static	void	BladeGripOff( void )
{
	int			buf = 3 ;
	BladeGripOnOff( &buf ) ;
}

static	void	UpdateMap( Work *work )
{
	int			map ;

	map = GM_CurrentMap = work->ctrl->map ;
	work->weapon.map_name = map ;
	GM_GroupObjs( work->weapon.objs, map ) ;
}

static	void	CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, visible ;
	int			code ;

	visible = 0 ;
	n_msg = GV_ReceiveMessage( GV_StrCode( "ブレード" ), &msg ) ;
	if ( n_msg ) {
		while( -- n_msg >= 0 ) {
			code = msg->message[ 0 ] ;
			if ( code == 1 ) {
				work->flag |= BLD_FLAG_PULLOUT_MSG ;
			} else if ( code == 2 ) {
				work->flag ^= BLD_FLAG_INV ;
printf( "inv %x\n", work->flag & BLD_FLAG_INV ) ;
			}
			msg ++ ;
		}
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int			trig, fire ;
	OBJECT		*weapon, *weapon_sub, *weapon_mir ;
	FVECTOR		force ;
	FMATRIX		*root ;

	UpdateMap( work ) ;
	CheckMessage( work ) ;

	if ( work->flag & BLD_FLAG_INV ) {
		weapon = &work->weapon_inv ;
		weapon_sub = &work->weapon_inv_sub ;
		weapon_mir = &work->weapon_inv_mir ;
		DG_InvisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjs( work->weapon_sub.objs ) ;
		DG_InvisibleObjs( work->weapon_mir.objs ) ;
	} else {
		weapon = &work->weapon ;
		weapon_sub = &work->weapon_sub ;
		weapon_mir = &work->weapon_mir ;
		DG_InvisibleObjs( work->weapon_inv.objs ) ;
		DG_InvisibleObjs( work->weapon_inv_sub.objs ) ;
		DG_InvisibleObjs( work->weapon_inv_mir.objs ) ;
	}
	if ( DG_ReverseOrderDrawMode == 0 ) {
		weapon_mir = NULL ;
	}

	if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
		if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_COMBO ) ) {
			/* コンボのときだけ、柄ありモデル */
			GM_ConfigObjectRoot( weapon, *work->body, *work->unit ) ;
			if ( weapon_mir ) {/*鏡の時の処理*/
				GM_ConfigObjectRoot( weapon_mir, GM_PlayerBody, *work->unit ) ;
				DG_InvisibleObjsChanl( weapon->objs    , 1 ) ;
				DG_VisibleObjs( weapon_mir->objs ) ;
				DG_InvisibleObjsChanl( weapon_mir->objs, 0 ) ;
			}
			DG_InvisibleObjs( weapon_sub->objs ) ;
			root = weapon->objs->root ;
			if ( work->flag & BLD_FLAG_REAL ) {
				GM_PlayerSubWeaponBody = weapon ;
				GM_PlayerWeaponBody = weapon_sub ;
			}
		} else {
			GM_ConfigObjectRoot( weapon_sub, *work->body, *work->unit ) ;
			GM_ConfigObjectRoot( weapon    , *work->body, *work->unit ) ;
			if ( weapon_mir ) {/*鏡の時の処理*/
				GM_ConfigObjectRoot( weapon_mir, GM_PlayerBody, *work->unit ) ;
				DG_InvisibleObjsChanl( weapon_sub->objs, 1 ) ;
				DG_VisibleObjs( weapon_mir->objs ) ;
				DG_InvisibleObjsChanl( weapon_mir->objs, 0 ) ;
			}
			DG_InvisibleObjs( weapon->objs ) ;
			root = weapon_sub->objs->root ;
			if ( work->flag & BLD_FLAG_REAL ) {
				GM_PlayerSubWeaponBody = weapon_sub ;
				GM_PlayerWeaponBody = weapon ;
			}
		}
	} else {
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			GM_ConfigObjectRoot( weapon, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
		} else {
			GM_ConfigObjectRoot( weapon, *work->body, *work->unit ) ;
		}
		DG_InvisibleObjs( weapon_sub->objs ) ;		
		root = weapon->objs->root ;
		if ( weapon_mir ) {/*鏡の時の処理*/
			DG_InvisibleObjs( weapon_mir->objs ) ;
		}
		if ( work->flag & BLD_FLAG_REAL ) {
			GM_PlayerSubWeaponBody = weapon_sub ;
			GM_PlayerWeaponBody = weapon ;
		}
	}

	DG_COPY_VEC( &work->prev_pos[ 0 ], &work->pos[ 0 ] ) ;
	DG_COPY_VEC( &work->prev_pos[ 1 ], &work->pos[ 1 ] ) ;
	DG_COPY_VEC( &work->prev_pos[ 2 ], &work->pos[ 2 ] ) ;
	DG_SetPos( root ) ;
	DG_PutVector( &Blade1, &work->pos[ 0 ], 1 ) ;
	DG_PutVector( &Blade0, &work->pos[ 1 ], 1 ) ;
	DG_PutVector( &Blade2, &work->pos[ 2 ], 1 ) ;

	_sceVu0SubVector( &force, &work->pos[ 1 ], &work->prev_pos[ 1 ] ) ;

	trig = ( *work->trigger ) & 0xf ;
	fire = ( *work->trigger ) & BLD_TRIG_FIRE ;
	if ( work->flag & BLD_FLAG_REAL ) {
		if ( !( work->flag & BLD_FLAG_PULLOUT ) ) {
			if ( trig == BLD_TRIG_PULLOUT || 
				 ( work->flag & BLD_FLAG_PULLOUT_MSG ) ) {
				work->flag |= BLD_FLAG_PULLOUT ;
				work->flag &= ~BLD_FLAG_PULLOUT_MSG ;

				DG_VisibleObjs( weapon->objs ) ;
				DG_VisibleObjs( weapon_sub->objs ) ;
				if ( weapon_mir ) {/*鏡の時の処理*/
					DG_InvisibleObjsChanl( weapon->objs    , 1 ) ;
					DG_InvisibleObjsChanl( weapon_sub->objs, 1 ) ;
					DG_VisibleObjs( weapon_mir->objs ) ;
					DG_InvisibleObjsChanl( weapon_mir->objs, 0 ) ;
				}
				BladeGripOff() ;
			} else {
				DG_InvisibleObjs( weapon->objs ) ;
				DG_InvisibleObjs( weapon_sub->objs ) ;
				if ( weapon_mir ) {/*鏡の時の処理*/
					DG_InvisibleObjs( weapon_mir->objs ) ;
				}
			}
		} else {
		    DG_VisibleObjs( weapon->objs ) ;
		    DG_VisibleObjs( weapon_sub->objs ) ;
			if ( weapon_mir ) {/*鏡の時の処理*/
				DG_InvisibleObjsChanl( weapon->objs    , 1 ) ;
				DG_InvisibleObjsChanl( weapon_sub->objs, 1 ) ;
				DG_VisibleObjs( weapon_mir->objs ) ;
				DG_InvisibleObjsChanl( weapon_mir->objs, 0 ) ;
			}
		}

		//GM_PlayerWeaponBody = weapon ;
		//GM_PlayerSubWeaponBody = weapon_sub ;
		if ( work->faint_atk_count > 0 ) -- work->faint_atk_count ;
//		if ( trig == BLD_TRIG_SLASH || trig == BLD_TRIG_THRUST || 
//			 trig == BLD_TRIG_SLASH_STRONG ) {
	    if ( fire ) {
			FVECTOR		from ;
			FVECTOR		to ;

			GV_MatToVec( root, &from ) ;
			DG_SetPos( root ) ;
			DG_PutVector( &Blade2, &to, 1 ) ;

			/* ちょっと射程伸ばす */
			_sceVu0SubVector( &to, &to, &from ) ;
			GV_LenVec3F( &to, &to, 0.0F, GV_VecLen3F( &to ) + 150.0F ) ;
			_sceVu0AddVector( &to, &from, &to ) ;
			/* 手前にも伸ばす */
			_sceVu0SubVector( &from, &from, &to ) ;
			GV_LenVec3F( &from, &from, 0.0F, GV_VecLen3F( &from ) + 500.0F ) ;
			_sceVu0AddVector( &from, &to, &from ) ;			

			GM_MoveOnlineTarget( &work->target, &from, &to ) ;
			//ViewFromTo( &from, &to, 32, 232, 32 ) ;
			if ( ( work->flag & BLD_FLAG_INV ) &&
				 ( trig != BLD_TRIG_THRUST ) ) {
				if ( work->faint_atk_count <= 0 ) {
					GM_SetPowerTarget( &work->target, &work->power, POWER_ONCE, 255, 3, 
									   0, &force ) ;
#if 0
					if ( trig == BLD_TRIG_SLASH_STRONG ) {
						GM_SetTargetWeaponType( &work->target, WP_KICK ) ;
					} else {
						GM_SetTargetWeaponType( &work->target, FaintAtk[ work->faint_atk ] ) ;
					}
#else
					GM_SetTargetWeaponType( &work->target, WP_BLADEFAINT ) ;
#endif
					GM_PutTarget( &work->target ) ;
					if ( ++ work->faint_atk > 2 ) work->faint_atk = 0 ;
				}
			} else {
				GM_SetPowerTarget( &work->target, &work->power, POWER_ONCE, 255, 0, 
								   DMG_BLAST, &force ) ;
				if ( trig != BLD_TRIG_THRUST ) {
					GM_SetTargetWeaponType( &work->target, WP_BLADE ) ;
				} else {
					GM_SetTargetWeaponType( &work->target, WP_BLADESTAB ) ;
				}
				GM_PutTarget( &work->target ) ;
			}
		}
		if ( BladeEffectScn == 1 ||
			 trig == BLD_TRIG_SLASH || 
			 trig == BLD_TRIG_SLASH_STRONG || 
			 trig == BLD_TRIG_THRUST ||
			 trig == BLD_TRIG_GUARD_SLASH ) {
			DG_SetPos( root ) ;
			DG_PutVector( &Blade1, &work->pos[ 0 ], 1 ) ;
			DG_PutVector( &Blade0, &work->pos[ 1 ], 1 ) ;
			work->eff_alpha = 128 ;
			if ( work->int_poly == NULL ) {
				SVECTOR			col = { 32, 96, 196, 32 } ;
				SVECTOR			col2 = { 32, 196, 96, 32 } ;
				SVECTOR			*use_col ;

				if ( trig != BLD_TRIG_THRUST && ( work->flag & BLD_FLAG_INV ) ) {
					use_col = &col2 ;
				} else {
					use_col = &col ;
				}
				work->int_poly = NewBladeEft( &work->pos[ 0 ], &work->pos[ 1 ], 8, 
											  0x040a0e00, &work->eff_alpha ) ;
//				work->int_poly = NewInterPoly_Demo( &work->pos[ 0 ], &work->pos[ 1 ],
//													16, use_col ) ;
				GV_SetActorChild( work, work->int_poly ) ;
			}
		} else {
			if ( work->int_poly != NULL ) {
				DG_SetPos( root ) ;
				DG_PutVector( &Blade1, &work->pos[ 0 ], 1 ) ;
				DG_PutVector( &Blade0, &work->pos[ 1 ], 1 ) ;
				work->eff_alpha = GV_NearSpeed( work->eff_alpha, 0, 3 ) ;
//				GV_DestroyOtherActor( work->int_poly ) ;
//				work->int_poly = NULL ;
			}
		}
	} else {
		if ( trig == BLD_TRIG_PUTBACK ) {
			BladeGripOn() ;
			work->flag &= ~BLD_FLAG_PULLOUT ;
			DG_InvisibleObjs( weapon->objs ) ;
		}
	}

	if ( ( weapon->objs->flag & DG_FLAG_INVISIBLE0 ) &&
		 ( weapon_sub->objs->flag & DG_FLAG_INVISIBLE0 ) ) {
		BladeGripOn() ;
	} else {
		BladeGripOff() ;
	}


	/* T.Morita 追加  -------ここから--------- */
	/* VR用出現エフェクト */
	if ( work->efct_sw > 1 ) {
	    if ( --work->efct_sw == 1 ) {
		work->efct_sw = 0 ;
		GV_SetActorChild( work, 
				  NewKatanaAppear( &weapon->objs->objs->world,
						   work->flag & BLD_FLAG_REAL ? 1 : 0,
						   &work->efct_sw ) ) ;
	    }
	}
	if ( (!(work->flag & BLD_FLAG_REAL) && work->efct_sw==1) ||
	     work->efct_sw == 0 ){
	    /* エフェクト起動中は刀を消す */
	    DG_InvisibleObjs( work->weapon_inv.objs ) ;
	    DG_InvisibleObjs( work->weapon_inv_sub.objs ) ;
	    DG_InvisibleObjs( work->weapon.objs ) ;
	    DG_InvisibleObjs( work->weapon_sub.objs ) ;
		DG_InvisibleObjs( work->weapon_mir.objs ) ;
		DG_InvisibleObjs( work->weapon_inv_mir.objs ) ;
	}		
	/* T.Morita 追加  -------ここまで--------- */
}

static	void	Die( work )
Work		*work ;
{
    if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
    }
    GM_FreeObject( &( work->weapon ) ) ;
    GM_FreeObject( &( work->weapon_sub ) ) ;
    GM_FreeObject( &( work->weapon_inv ) ) ;
    GM_FreeObject( &( work->weapon_inv_sub ) ) ;
	GM_FreeObject( &( work->weapon_mir ) ) ;
	GM_FreeObject( &( work->weapon_inv_mir ) ) ;

	if ( work->flag & BLD_FLAG_REAL ) {
		GM_PlayerWeaponBody = NULL ;
		GM_PlayerSubWeaponBody = NULL ;
	} 
	BladeGripOn() ;
	work->efct_sw = 1 ; /*エフェクトが上がった場合は殺す*/
}

/*------------------------------------------------------------------*/

static	int	GetResources( work, body, unit, mode, inv )
Work		*work ;
OBJECT		**body ;
int			*unit ;
int			mode ;
int			inv ;
{
    OBJECT	*weapon ;

	/* 殺しうち用 */
    weapon = &( work->weapon ) ;
    GM_InitObject( weapon, BODY_DATA, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
    DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
	GM_ConfigObjectRoot( weapon, *body, *unit ) ;

    weapon = &( work->weapon_sub ) ;
    GM_InitObject( weapon, MDL_BLADE_SUB, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( weapon, *body, *unit ) ;

	weapon = &( work->weapon_mir ) ;
	GM_InitObject( weapon, BODY_DATA, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( weapon, *body, *unit ) ;

	/* 峰うち用 */
    weapon = &( work->weapon_inv ) ;
    GM_InitObject( weapon, BODY_DATA_INV, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( weapon, *body, *unit ) ;
	DG_InvisibleObjs( weapon->objs ) ;

    weapon = &( work->weapon_inv_sub ) ;
    GM_InitObject( weapon, MDL_BLADE_INV_SUB, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( weapon, *body, *unit ) ;
	DG_InvisibleObjs( weapon->objs ) ;

	weapon = &( work->weapon_inv_mir ) ;
	GM_InitObject( weapon, BODY_DATA_INV, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( weapon, *body, *unit ) ;

	GM_PlayerWeaponModel = BODY_DATA ;

	/* T.Morita 追加  -------ここから--------- */
	/* VR用出現エフェクトのための初期化 */
	work->efct_sw = -1 ;
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
	    /*消え刀モデルがあったらやる 暫定*/
	    if ( GV_GetCache( GV_CacheID( 15363829, 'k' ) ) ) {
		if ( mode == 0 ) {
		    work->efct_sw = 2 ;
		} else {
		    work->efct_sw = 40 ;
		}
	    }
	}
	/* T.Morita 追加  -------ここまで--------- */

	if ( mode == 0 ) {
		work->flag |= BLD_FLAG_REAL ;
		GM_PlayerWeaponBody = &work->weapon ;
		GM_PlayerSubWeaponBody = &work->weapon_sub ;
		GM_SetTarget( &work->target, TARGET_OFFENSE | 
					  TARGET_ONLINE | TARGET_POWER | TARGET_CHILD, 0,
					  ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
		GM_SetPowerTarget( &work->target, &work->power, POWER_ONCE, 255, 0, 
						   DMG_BLAST, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( &work->target, Hit, work ) ;
		GM_SetTargetWeaponType( &work->target, WP_BLADE ) ;
//		GM_SetTargetWeaponType( &work->target, WP_USP ) ;
		DG_InvisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjs( work->weapon_sub.objs ) ;
		BladeGripOn() ;
	} else {
		if ( inv ) {
			DG_InvisibleObjs( work->weapon.objs ) ;
			DG_InvisibleObjs( work->weapon_sub.objs ) ;
			DG_VisibleObjs( work->weapon_inv.objs ) ;
			DG_VisibleObjs( work->weapon_inv_sub.objs ) ;
			work->flag |= BLD_FLAG_INV ;
		} else {
			DG_VisibleObjs( work->weapon.objs ) ;
			DG_VisibleObjs( work->weapon_sub.objs ) ;
			DG_InvisibleObjs( work->weapon_inv.objs ) ;
			DG_InvisibleObjs( work->weapon_inv_sub.objs ) ;
		}
		BladeGripOff() ;
	}
	DG_InvisibleObjs( work->weapon_mir.objs ) ;
	DG_InvisibleObjs( work->weapon_inv_mir.objs ) ;

	DG_SetPos( work->weapon.objs->root ) ;
	DG_PutVector( &Blade1, &work->pos[ 0 ], 1 ) ;
	DG_PutVector( &Blade0, &work->pos[ 1 ], 1 ) ;
	DG_PutVector( &Blade2, &work->pos[ 2 ], 1 ) ;
	DG_COPY_VEC( &work->prev_pos[ 0 ], &work->pos[ 0 ] ) ;
	DG_COPY_VEC( &work->prev_pos[ 1 ], &work->pos[ 1 ] ) ;
	DG_COPY_VEC( &work->prev_pos[ 2 ], &work->pos[ 2 ] ) ;

#if 0
	{
		TARGET		*t ;
		FVECTOR		size, offset ;

		t = &( work->def ) ;
		size.vx = 500.0F ;
		size.vy = 1000.0F ;
		size.vz = 200.0F ;
		offset.vx = offset.vy = 0.0F ;
		offset.vz = 450.0F ;
		GM_SetTarget( t, TARGET_DEFENSE | TARGET_ROTATE | TARGET_POWER, 0,
					  PLAYER_SIDE, &size, &offset ) ;
		GM_SetTargetCallBack( t, Hit, work ) ;
		GM_PutTarget( t ) ;
		GM_TargetSetSkip( t ) ;
	}
#endif	
    return 0 ;
}

/*------------------------------------------------------------------*/

static	void	*New( ctrl, body, unit, trigger, side, mode, inv )
CONTROL			*ctrl ;
OBJECT			**body ;
int				*unit ;
u_int			*trigger ;
int				side, mode, inv ;
{
    Work	*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, unit, mode, inv ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->side = side ;
#if 0
		GV_LenVec3F( &Blade1, &Blade1, 0.0F, GV_VecLen3F( &Blade1 ) + 250.0F ) ;
		DumpVec( &Blade1 ) ;
		GV_LenVec3F( &Blade1, &Blade2, 0.0F, GV_VecLen3F( &Blade1 ) + 250.0F ) ;
		DumpVec( &Blade2 ) ;
#endif
		BladeEffectScn = 0 ;
    }
    return work ;
}

/*----------------------------------------------------------------*/

/* 高周波ブレード */
void	*NewBlade( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
int			side ;
{
	return New( ctrl, body, unit, trigger, side, 0, 0 ) ;
}

/* 鞘にしまうとき用 */
void	*NewBlade2( ctrl, body, unit, trigger, side, inv )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
int			side, inv ;
{
	return New( ctrl, body, unit, trigger, side, 1, inv ) ;
}

/*----------------------------------------------------------------*/

/* シナリオでエフェクトを呼ぶ */
void	PL_COM_BladeEffectOn( void )
{
	BladeEffectScn = 1 ;
}

void	PL_COM_BladeEffectOff( void )
{
	BladeEffectScn = 0 ;
}	
