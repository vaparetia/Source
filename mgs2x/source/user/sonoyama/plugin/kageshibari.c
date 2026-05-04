//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   kageshibari.c
   影縛りプラグイン
   
   2001/05/18	M.Sonoyama
   $Id: kageshibari.c,v 1.1.1.3 2002/11/19 11:50:48 Yoshizawa1 Exp $
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

/*----------------------------------------------------------------*/

/* 止まり影 */
typedef	struct	{
	GV_ACT			actor ;
	OBJECT			body ;
	OBJECT			append ;
	OBJECT			weapon ;
} Work ;

OBJECT			*GM_PlayerKageshibariBody = NULL ;

static	void	*MakeStopShadowWork = NULL ;

static	void	CopyMatrix_DG_OBJS( DG_OBJS * new, DG_OBJS * org )
{
	int			i ;

	new->group_id = org->group_id ;
	DG_COPY_MAT( &new->world, &org->world ) ;
	new->root = &org->world ;
	for ( i = 0; i < new->n_models; i ++ ) {
		DG_COPY_MAT( &new->objs[ i ].world, &org->objs[ i ].world ) ;
	}
}

static	void	CopyMatrixRoot( DG_OBJS * new, DG_OBJS * org, int unit )
{
	int			i ;

	new->group_id = org->group_id ;
	DG_COPY_MAT( &new->world, &org->objs[ unit ].world ) ;
	new->root = &org->objs[ unit ].world ;
	for ( i = 0; i < new->n_models; i ++ ) {
		DG_COPY_MAT( &new->objs[ i ].world, &org->objs[ unit ].world ) ;
	}
}

static	void	CopyObjectFlag( DG_OBJS * new, DG_OBJS * org )
{
	int			i ;

	for ( i = 0; i < org->n_models; i ++ ) {
		new->objs[ i ].flag = org->objs[ i ].flag ;
	}
}

static	void	Act( Work *work )
{
	GM_PlayerBody->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
	/* ダンボールはダメージを受けるかもしれないので */
	if ( ( GM_ItemTypes[ PL_GetPlayerItem() ] & IT_TYPE_CBBOX ) &&
		 GM_PlayerBoxBody != NULL ) {
		CopyObjectFlag( work->append.objs, GM_PlayerBoxBody->objs ) ;
	}
}

static	void	Die( Work *work )
{
	GM_FreeObject( &work->body ) ;
	GM_FreeObject( &work->append ) ;
	if ( GM_PlayerBody != NULL ) {
		GM_PlayerBody->objs->flag |= DG_FLAG_SHADOWMAKE ;
	}
	if ( work->weapon.objs != NULL ) {
		GM_FreeObject( &work->weapon ) ;
	}
	MakeStopShadowWork = NULL ;
	GM_PlayerKageshibariBody = NULL ;
}

static	int		GetResources( Work *work )
{
	OBJECT		*body ;

	body = &work->body ;
	if ( PL_GetPlayerItem() == IT_Uniform ) {
		GM_InitObject( body, GV_StrCode( "rai_gbs_mt" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC|
					   DG_FLAG_SHADOWMAKE ) ;
	} else {
		if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
			GM_InitObject( body, GV_StrCode( "sna_shadow" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC|
						   DG_FLAG_SHADOWMAKE ) ;		
		} else {
			GM_InitObject( body, GV_StrCode( "rai_shadow" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC|
						   DG_FLAG_SHADOWMAKE ) ;		
		}
	}
	if ( body->objs == NULL ) return -1 ;
	DG_InvisibleObjs( body->objs ) ;
	CopyMatrix_DG_OBJS( body->objs, GM_PlayerBody->objs ) ;
	GM_PlayerKageshibariBody = body ;

	body = &work->append ;
	if ( ( GM_ItemTypes[ PL_GetPlayerItem() ] & IT_TYPE_CBBOX ) &&
		 GM_PlayerBoxBody != NULL ) {
		GM_InitObject( body, GV_StrCode( "cbx_a_mt" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC|
					   DG_FLAG_SHADOWMAKE ) ;
		if ( body->objs == NULL ) return -1 ;
		CopyMatrixRoot( body->objs, GM_PlayerBody->objs, HUMAN21_KOSHI ) ;
		CopyObjectFlag( body->objs, GM_PlayerBoxBody->objs ) ;
	} else {
		if ( !GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
			GM_InitObject( body, GV_StrCode( "rai_hair_shadow_mt" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC|
						   DG_FLAG_SHADOWMAKE ) ;		
			if ( body->objs == NULL ) return -1 ;
			CopyMatrixRoot( body->objs, GM_PlayerBody->objs, HUMAN21_ATAMA ) ;
		}
	}
	if ( body->objs != NULL ) DG_InvisibleObjs( body->objs ) ;

	body = &work->weapon ;
	if ( GM_PlayerWeaponBody != NULL ) {
		GM_InitObject( body, GM_PlayerWeaponModel, DG_FLAG_SHADE|DG_FLAG_FINISHCALC|
					   DG_FLAG_SHADOWMAKE ) ;		
		if ( body->objs == NULL ) return -1 ;
		CopyMatrixRoot( body->objs, GM_PlayerBody->objs, HUMAN21_MIGI_TE ) ;
		CopyObjectFlag( body->objs, GM_PlayerWeaponBody->objs ) ;
		DG_InvisibleObjs( body->objs ) ;
	}

	GM_PlayerBody->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
	MakeStopShadowWork = work ;
	return 0 ;
}

static	void	*MakeStopShadow( void )
{
	Work 	*work ;

	if ( MakeStopShadowWork != NULL ) {
		GV_DestroyOtherActorQuick( MakeStopShadowWork ) ;
		MakeStopShadowWork = NULL ;
	}
	work = GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( work, Act, Die ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

static	void	DestroyStopShadow( void )
{
	if ( MakeStopShadowWork != NULL ) {
		GV_DestroyOtherActorQuick( MakeStopShadowWork ) ;
		MakeStopShadowWork = NULL ;
	}
}

/*----------------------------------------------------------------*/

#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

static	PL_PluginSet	PluginSet ;
static	PL_PollingSet	PollingSet ;

static	int				ShibariTime = 0 ;

static	void	KageshibariGround( PlayerWork *work, int time ) ;
static	void	KageshibariSquat( PlayerWork *work, int time ) ;
static	void	KageshibariStand( PlayerWork *work, int time ) ;
static	void	KageshibariRun( PlayerWork *work, int time ) ;

/* raiden.hより */
#define	TURN_MARGIN			(256)
#define	TURN_COUNT_VALUE	(1024)

/*----------------------------------------------------------------*/

static	void	ClearReturnMode( void )
{
	PL_ReturnModes[ 0 ] = NULL ;
	PL_ReturnModes[ 1 ] = NULL ;
	PL_ReturnModes[ 2 ] = NULL ;
	UnsetFlag( FLAG_RETURNMODE_SET ) ;
	DestroyStopShadow() ;
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_NO_KICK ) ;
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_KAGESHIBARI_NOW ) ;
}

static	void	SetReturnMode( void )
{
	PL_ReturnModes[ 0 ] = KageshibariStand ;
	PL_ReturnModes[ 1 ] = KageshibariSquat ;
	PL_ReturnModes[ 2 ] = ( GM_GameStatus & STATE_GLL ) ? KageshibariGround : NULL ;
	SetFlag( FLAG_RETURNMODE_SET ) ;
}

static	void	KageshibariGround( PlayerWork *work, int time ) 
{
	int			diff ;

    if ( time == 0 ) {
		PL_ChangeStance( work, GROUND ) ;
		PL_SetAction( work, PL_MotionSet->change[ Mcrouch ], 6 ) ;	
		SetReturnMode() ;
    }

	if ( ShibariTime <= 0 ) {
		ClearReturnMode() ;
		SetMode( work, PL_StillMode[ GROUND ] ) ;
		return ;
	}

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_SPECIAL_MODE ) ;
	GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;

    if ( Status( PLAYER_NEED_NEW_PRESS ) ) return ;

    if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
		if ( work->pad->press & PL_PAD_SQUAT ) {
			SetMode( work, KageshibariSquat ) ;
			return ;
		}
    } else {
		if ( work->pad->press & PL_PAD_SQUAT ) {
			SetMode( work, KageshibariSquat ) ;	
			return ;
		} else if ( work->pad->status & PAD_UDLR ) {
			if ( work->data != 0 ) {
				-- work->data ; 
				return ;
			}
			diff = GV_DiffDirAbs( work->control.turn.vy, PadTo ) ;
			if ( diff > TURN_MARGIN ) {
				work->data = diff / TURN_COUNT_VALUE + 1 ;
			} 
			//work->control.turn.vy = PadTo ;
			work->control.turn.vy = GV_NearSpeedP( work->control.rot.vy, PadTo, 128 ) ;
    	}
    }
	PL_CheckAttack( work ) ;
}

static	void	KageshibariSquat( PlayerWork *work, int time ) 
{
	int			diff ;

    if ( time == 0 ) {
		PL_ChangeStance( work, SQUAT ) ;
		PL_SetAction( work, PL_MotionSet->change[ Msquat ], 6 ) ;	
		SetReturnMode() ;
    }

	if ( ShibariTime <= 0 ) {
		ClearReturnMode() ;
		SetMode( work, PL_StillMode[ SQUAT ] ) ;
		return ;
	}

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_SPECIAL_MODE ) ;
	GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;

    if ( Status( PLAYER_NEED_NEW_PRESS ) ) return ;

    if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
		if ( work->pad->press & PL_PAD_SQUAT ) {
			SetMode( work, KageshibariStand ) ;
			return ;
		}
    } else {
		if ( work->pad->press & PL_PAD_SQUAT ) {
			SetMode( work, KageshibariStand ) ;	
			return ;
		} else if ( work->pad->status & PAD_UDLR ) {
			if ( work->data != 0 ) {
				-- work->data ; 
				return ;
			}
			diff = GV_DiffDirAbs( work->control.turn.vy, PadTo ) ;
			if ( diff > TURN_MARGIN ) {
				work->data = diff / TURN_COUNT_VALUE + 1 ;
			} 
			//work->control.turn.vy = PadTo ;
			work->control.turn.vy = GV_NearSpeedP( work->control.rot.vy, PadTo, 128 ) ;
    	}
    }
	PL_CheckAttack( work ) ;
}

static	void	KageshibariStand( PlayerWork *work, int time ) 
{
	int			diff ;

    if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		PL_SetAction( work, PL_MotionSet->change[ Mstand ], 6 ) ;	
		SetReturnMode() ;
    }

	if ( ShibariTime <= 0 ) {
		ClearReturnMode() ;
		SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	}

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_SPECIAL_MODE ) ;
	GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;

    if ( Status( PLAYER_NEED_NEW_PRESS ) ) return ;

    if ( Status( PLAYER_WATCH ) ) {
		PL_SubjectTurn( work ) ;
		if ( work->pad->press & PL_PAD_SQUAT ) {
			SetMode( work, KageshibariSquat ) ;
			return ;
		}
    } else {
		if ( work->pad->status & PAD_UDLR ) {
			if ( work->data != 0 ) {
				-- work->data ; 
				return ;
			}
			diff = GV_DiffDirAbs( work->control.turn.vy, PadTo ) ;
			if ( diff > TURN_MARGIN ) {
				work->data = diff / TURN_COUNT_VALUE + 1 ;
			} else {
				SetMode( work, KageshibariRun ) ;
				return ;
			}
			//work->control.turn.vy = PadTo ;
			work->control.turn.vy = GV_NearSpeedP( work->control.rot.vy, PadTo, 128 ) ;
		} else {
			if ( work->pad->press & PL_PAD_SQUAT ) {
				SetMode( work, KageshibariSquat ) ;	
				return ;
			}
    	}
    }
	PL_CheckAttack( work ) ;
}

static	void	KageshibariRun( PlayerWork *work, int time ) 
{
	int		max, to ;
	float	t ;

	SetStatus( PLAYER_MOVE ) ;

    if ( time == 0 ) {
		PL_ChangeStance( work, STAND ) ;
		work->idata = -1 ;
		work->data2 = 0 ;
		SetReturnMode() ;
    }

	if ( ShibariTime <= 0 ) {
		ClearReturnMode() ;
		SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	}

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_SPECIAL_MODE ) ;
	GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;

    if ( Status( PLAYER_WATCH ) ) {
		SetMode( work, KageshibariStand ) ;
		return ;
    } else {
		if ( work->pad->press & PL_PAD_SQUAT ) {
			SetMode( work, KageshibariSquat ) ;
			return ;
		} else if ( work->pad->status & PAD_UDLR ) {
			if ( PadForce > PAD_WALK_TH ) {
				to = PL_ReactWall( PadTo ) ;
			} else {
				to = PadTo ;
			}
			/* チャタリング防止策 */
			if ( work->idata >= 0 && GV_DiffDirAbs( work->idata, to ) < 256 ) {
				//work->control.turn.vy = to ;
				work->control.turn.vy = GV_NearSpeedP( work->control.rot.vy, to, 128 ) ;
			}	
			work->idata = to ;
			work->control.turn.vz = 0 ;
		} else {
			SetMode( work, KageshibariStand ) ;
			return ;
		}

		if ( work->data2 == 0 ) {
			work->data2 = 1 ;
			t = 0.0F ; 
		} else {
			t = ( work->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
				 + work->body.m_ctrl->mt3_ctrl[ 0 ].motion_time_base )
				/ work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
		}
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.0F ) ;

		max = PL_MoveLevel( work ) ;
		if ( max != 0 ) {
			if ( max == 1 ) {				
				PL_SetAction4( work, MS.change[ Mwalk ], t, 6 ) ;
			} else {
				PL_SetAction4( work, MS.change[ Mwalk ], t, 6 ) ;
				//PL_SetAction4( work, MS.change[ Mrun ], t, 6 ) ;
			}
			SetFlag( FLAG_NO_IK ) ; /* 通常走りはＩＫなし */
		}
		//if ( max == 1 || PL_MotionTime( work ) < 16 ) SetStatus( PLAYER_WALK ) ;
		//else 			SetStatus( PLAYER_DASH ) ;
	}
	PL_CheckAttack( work ) ;
}

/*----------------------------------------------------------------*/

static	int	PollingFuncKageshibari( PlayerWork *work )
{
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_KAGESHIBARI_NOW ) ;
	if ( ShibariTime > 0 ) {
		if ( MakeStopShadowWork == NULL ) MakeStopShadow() ;
		if ( ( -- ShibariTime <= 0 ) || 
			GM_CheckPlayerStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ) {
			ClearReturnMode() ;
			ShibariTime = 0  ;
			return 0 ;
		}
#if 0
		/* しばられてなさそう */
		if ( !( PL_ItemType( work ) & IT_TYPE_CBBOX ) &&
			 work->action != KageshibariStand &&
			 work->action != KageshibariSquat &&
			 work->action != KageshibariSquat &&
			 work->action != KageshibariRun ) {
			printf( "shibararete nai??\n" ) ;
			ClearReturnMode() ;
			ShibariTime = 0  ;
			return 0 ;
		}
#endif
		/* ダンボール特殊 */
		if ( PL_ItemType( work ) & IT_TYPE_CBBOX ) {
			//printf( "danbo-ru kageshibari!\n" ) ;
			if ( work->def.damaged & TARGET_POWER ) {
				ClearReturnMode() ;
				ShibariTime = 0  ;
				return 0 ;				
			}
			GM_SetPlayerStatusEX( I64(0), PLAYER2_NO_MOTION_STEP_XZ ) ;
		}
		/* コンボ特殊 */
		if ( work->act_name == COMBO ) {
			GM_SetPlayerStatusEX( I64(0), PLAYER2_NO_KICK | PLAYER2_NO_MOTION_STEP_XZ ) ;
		}
		GM_SetPlayerStatusEX( I64(0), PLAYER2_KAGESHIBARI_NOW ) ;
		GM_SetPlayerStatusEX( I64(0), PLAYER2_MENU_DISABLE ) ;
	}
	return 0 ;
}

static int VrKageshibari( PlayerWork *work ) {
	if ( GM_GameStatus & STATE_GLL ) {	/* ゴルルゴンステージのみ !!*/
		if ( PL_GetPlayerWeapon( ) == WP_Stinger ) return 0 ;
		if ( PL_GetPlayerWeapon( ) == WP_Psg1 ) return 0 ;
		if ( PL_GetPlayerWeapon( ) == WP_Psg1T ) return 0 ;
		if ( PL_GetPlayerWeapon( ) == WP_ColdSpray ) return 0 ;
		if ( PL_GetPlayerWeapon( ) == WP_Blade ) return 0 ;
	}
	
	return 1 ;
}

static int VrKageshibariPose( PlayerWork *work ) {
	if ( GM_GameStatus & STATE_GLL ) {	/* ゴルルゴンステージのみ !!*/
		if ( work->act_name == GROUND_STILL ) return 0 ;
		if ( work->act_name == GROUND_MOVE_FRONT ) return 0 ;
		if ( work->act_name == GROUND_MOVE_BACK ) return 0 ;
	}
	
	return 1 ;
}

static	int	SetKageshibariAct( PlayerWork *work, GV_MSG *msg, int len )
{
#if 1
	if ( !( PL_ItemType( work ) & IT_TYPE_CBBOX ) &&	
		 ( VrKageshibari( work ) ) &&
		 ( VrKageshibariPose( work ) ) &&
		 work->act_name != STAND_STILL &&
		 work->act_name != SQUAT_STILL &&
		 work->act_name != STAND_RUN &&
		 work->act_name != STAND_CAUTION_STILL &&
		 work->act_name != STAND_CAUTION_RIGHT &&
		 work->act_name != STAND_CAUTION_LEFT &&
		 work->act_name != SQUAT_CAUTION_RIGHT &&
		 work->act_name != SQUAT_CAUTION_LEFT &&
		 work->act_name != SQUAT_CAUTION_STILL ) {
		return -1 ;
	}

	if ( !VrKageshibari( work ) ) {
		if ( PL_GetPlayerWeapon( ) != WP_Blade ) {
			PL_NoWeapon( work ) ;
		}
	}
#else
	if ( !( PL_ItemType( work ) & IT_TYPE_CBBOX ) &&	
		 work->act_name != STAND_STILL &&
		 work->act_name != SQUAT_STILL &&
		 work->act_name != STAND_RUN &&
		 work->act_name != STAND_CAUTION_STILL &&
		 work->act_name != STAND_CAUTION_RIGHT &&
		 work->act_name != STAND_CAUTION_LEFT &&
		 work->act_name != SQUAT_CAUTION_RIGHT &&
		 work->act_name != SQUAT_CAUTION_LEFT &&
		 work->act_name != SQUAT_CAUTION_STILL ) {
		return -1 ;
	}
#endif
	if ( GM_CheckPlayerStatus( PLAYER_STEALTH ) ) return -1 ;

printf("aaaaaaaaaaa kagesibariiiiiii\n");

	PL_LeaveCaution( work ) ;
	PL_ClearCaptureTarget( work ) ;
	PL_SetMode2( work, NULL ) ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_SPECIAL_MODE ) ;
	GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_KAGESHIBARI_NOW ) ;

	ShibariTime = msg->message[ 1 ] ;
	MakeStopShadow() ;
	if ( PL_ItemType( work ) & IT_TYPE_CBBOX ) {
		/* ダンボール */
		/* なにもしないでＯＫ */
printf( "danbo-ru kageshibari!\n" ) ;
		return 1 ;
	} else if ( work->stance == SQUAT ) {
		SetMode( work, KageshibariSquat ) ;
	} else if ( (GM_GameStatus & STATE_GLL) && (work->stance == GROUND) ) {
		SetMode( work, KageshibariGround ) ;
	} else {
		SetMode( work, KageshibariStand ) ;
	}
	SetReturnMode() ;
	printf( "[%d] set kageshibari!!\n", GV_Time ) ;
	return 1 ;
}

int		NewPluginKageshibari( void )
{
	PL_AddPlugin( &PluginSet, PL_MSG_KAGESHIBARI, SetKageshibariAct, NULL ) ;
	PL_AddPollingFunc( &PollingSet, PollingFuncKageshibari ) ;
	ShibariTime = 0 ;
	MakeStopShadowWork = NULL ;
	GM_PlayerKageshibariBody = NULL ;
	return 0 ;
}

/*----------------------------------------------------------------*/

/* 影縛り開始 */
void	PL_StartKageshibari( int time )
{
	int			buf[ 2 ] ;

	buf[ 0 ] = PL_MSG_KAGESHIBARI ;
	buf[ 1 ] = time ;
	PL_SendMessage( GM_PlayerControl->name, buf, 2 ) ;
	printf( "[%d] send kageshibari mesg\n", GV_Time ) ;
}

/* 影縛りチェック */
int		PL_KageshibariNow( void )
{
	if ( ShibariTime > 0 ) return 1 ;
	return 0 ;
}

/* 影縛り解除 */
void	PL_FreeKageshibari( void )
{
	ClearReturnMode() ;
	ShibariTime = 0 ;
	printf( "[%d] free kageshibari\n", GV_Time ) ;
}

/* 影縛り時間短縮 */
void	PL_KageshibariTimeDecrease( void )
{
	if ( ShibariTime > 0 ) 	ShibariTime -- ;
}
