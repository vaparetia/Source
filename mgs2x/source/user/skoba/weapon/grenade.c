//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   grenade.c
   Grenade系
   
   2000/02/07 M.Sonoyama
   $Id: grenade.c,v 1.1.1.3 2002/11/19 11:50:28 Yoshizawa1 Exp $
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

#include	"BP_Debug.h"//BP_INPUT

/*------------------------------------------------------------------*/

/* 外部関数・変数宣言 */
extern	void	*NewBulletGrenade( FMATRIX *, FVECTOR *, int, int, int, int ) ;
extern	void	*NewBlast2( FMATRIX *, int, int, int, int, int, int ) ;
extern	void	*NewBlast3( FVECTOR *, int, int, int, int, int, int, int ) ;
extern	void	*NewBlastChaff( FMATRIX * ) ;
extern	void	*NewBlastStun( FMATRIX * ) ;

extern	void	*NewFallingBulletEX( FMATRIX *world, FVECTOR *shift, FVECTOR *force, 
								  int model, float front, float back, int type, int height ) ;

/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SUB	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC)

#define	BOMB_COUNT	(DIRECT_TICK(60 * 5)) /* 5秒 */
#define	WAIT_COUNT	(DIRECT_TICK(30))

enum {
    THIS_IS_CHAFF = 0,
    THIS_IS_STUN,
    THIS_IS_MAGAZINE,
    THIS_IS_GRENADE
} ;

static	int	WpNum[] = {
    WP_ChaffGrenade,
    WP_StunGrenade,
    WP_Magazine,
    WP_Grenade
} ;

static	int	*BodyDatas ;

static	int	BodyDatasSna[] = {
	MDL_CHAFF,
	MDL_CHAFF_SUB,
	MDL_STUN,
	MDL_STUN_SUB,
	MDL_USP_AMO,
	MDL_USP_AMO,
	MDL_GRENADE,
	MDL_GRENADE_SUB,
} ;

static	int	BodyDatasRai[] = {
	MDL_CHAFF_RAI,
	MDL_CHAFF_SUB_RAI,
	MDL_STUN_RAI,
	MDL_STUN_SUB_RAI,
	MDL_USP_AMO,
	MDL_USP_AMO,
	MDL_GRENADE_RAI,
	MDL_GRENADE_SUB_RAI,
} ;

static	int	*PinDatas ;

static	int	PinDatasSna[] = {
	MDL_CHAFF_PIN,
	MDL_CHAFF_LEVER,
	MDL_STUN_PIN,
	MDL_STUN_LEVER,
	0,
	0,
	MDL_GRENADE_PIN,
	MDL_GRENADE_LEVER,
} ;

static	int	PinDatasRai[] = {
	MDL_CHAFF_PIN_RAI,
	MDL_CHAFF_LEVER_RAI,
	MDL_STUN_PIN_RAI,
	MDL_STUN_LEVER_RAI,
	0,
	0,
	MDL_GRENADE_PIN_RAI,
	MDL_GRENADE_LEVER_RAI,
} ;

//static	FVECTOR	Force = { 0.0F, 96.0F, 140.0F } ;
static	FVECTOR	Force = { 0.0F, 48.0F, 70.0F } ;
static	FVECTOR	GllForce = { 0.0F, 57.0F, 82.0F } ;	/* VSゴルルゴン専用 */

static	FVECTOR	FallForce = { 0.0F, 16.0F, 32.0F } ;

static	FVECTOR	*MagShift ;

static	FVECTOR	MagShiftSna = { 17.5F, -115.9F, -1.2F } ;
static	FVECTOR	MagShiftRai = { 17.5F, -85.9F, -1.2F } ;

static	FVECTOR	LeftShift = { -35.0F, 0.0F, 0.0F } ;

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT_EX	actor ;
    OBJECT		weapon ;
    OBJECT		weapon_sub ;

	DG_OBJS		*pin ;
	DG_OBJS		*lever ;

    CONTROL		*ctrl ;
    OBJECT		**body ;
    int			*unit ;
    u_int		*trigger ;

    short		side ;
    short		this ;
    short		count ;
    short		wait ;
	int			flag ;
    GM_CameraSet	*camera ;

	int			chanl ;
	int			vflag ;
} Work ;

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	int		chanl, vflag, left ;
    int		this, inv ;
    u_int	trg, trg2 ;
    //float	adjx, adjz ;
    FVECTOR	mov, force, from ;
    SVECTOR	rot ;
    FMATRIX	*root, world ;

	chanl = work->chanl ;
	vflag = work->vflag ;
    GM_SetCurrentMap( work->ctrl->map ) ;
    GM_GroupObjs( work->weapon.objs, work->ctrl->map ) ;
    this = work->this ;

    trg = *( work->trigger ) ; 
	trg2 = ( trg >> 16 ) & 0xffff ;
	trg &= 0xffff ;

	left = ( trg2 == TRIG_MAG_LEFT_HAND ) ? 1 : 0 ;

	if ( trg == TRIG_SET && work->wait > 0 ) work->wait = 0 ;
	if ( work->wait > 0 ) -- work->wait ;

	GM_ResetPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ;

    if ( work->wait > 0 ||
		GM_WeaponNum( WpNum[ this ] ) == 0 ) {
		DG_InvisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjs( work->weapon_sub.objs ) ;
		if ( work->pin != NULL ) DG_InvisibleObjs( work->pin ) ;
		if ( work->lever != NULL ) DG_InvisibleObjs( work->lever ) ;
		return ;
    }

    if ( GM_PlayerBody->objs->flag & vflag ) {
		GM_ConfigObjectRoot( &( work->weapon_sub ), 
							*( work->body ), *( work->unit ) ) ;
		if ( left ) {
			GM_ConfigObjectRoot( &( work->weapon_sub ), 
								*( work->body ), HUMAN21_HIDARI_TE ) ;
		}

		root = work->weapon_sub.objs->root ;
		DG_InvisibleObjsChanl( work->weapon.objs, chanl ) ;
		DG_VisibleObjsChanl( work->weapon.objs, 1 ) ;
		DG_VisibleObjsChanl( work->weapon_sub.objs, chanl ) ;
		rot = work->camera->rotate ;
		inv = 1 ;
    } else {
		GM_ConfigObjectRoot( &( work->weapon ), 
							*( work->body ), *( work->unit ) ) ;
		if ( left ) {
			GM_ConfigObjectRoot( &( work->weapon ), 
								*( work->body ), HUMAN21_HIDARI_TE ) ;
		}
		root = work->weapon.objs->root ;
		DG_VisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjsChanl( work->weapon_sub.objs, chanl ) ;
		rot = work->ctrl->rot ;
		inv = 0 ;
    }

	if ( work->lever != NULL ) {
		DG_COPY_MAT( &work->lever->world, root ) ;
		GM_GroupObjs( work->lever, work->ctrl->map ) ;
		DG_VisibleObjsChanl( work->lever, 0 ) ;
	}

	if ( this == THIS_IS_MAGAZINE ) {
		int		obj = HUMAN21_MIGI_TE ;
		FVECTOR	shift ;

		DG_COPY_VEC( &shift, MagShift ) ;
		if ( left ) {
			shift.vx *= -1.0F ;	obj = HUMAN21_HIDARI_TE ;
		}

		DG_SetPos( &GM_PlayerBody->objs->objs[ obj ].world ) ;
		DG_MovePos( &shift ) ;
		DG_GetPos( &work->weapon.objs->world ) ;

		DG_SetPos( &GM_PlayerArmBody->objs->objs[ obj ].world ) ;
		DG_MovePos( &shift ) ;
		DG_GetPos( &work->weapon_sub.objs->world ) ;

		work->weapon.objs->root = NULL ;
		work->weapon_sub.objs->root = NULL ;
	} else if ( left ) {
		FVECTOR	shift ;
		
		DG_COPY_VEC( &shift, &LeftShift ) ;

		DG_SetPos( &GM_PlayerBody->objs->objs[ HUMAN21_HIDARI_TE ].world ) ;
		DG_MovePos( &shift ) ;
		DG_GetPos( &work->weapon.objs->world ) ;

		DG_SetPos( &GM_PlayerArmBody->objs->objs[ HUMAN21_HIDARI_TE ].world ) ;
		DG_MovePos( &shift ) ;
		DG_GetPos( &work->weapon_sub.objs->world ) ;

		work->weapon.objs->root = NULL ;
		work->weapon_sub.objs->root = NULL ;
	}

	/* ピン＆レバー */
	if ( work->pin != NULL ) {
		if ( work->flag == 1 ||
			( work->flag == 0 && trg == TRIG_YET && inv ) ) {
			DG_InvisibleObjsChanl( work->pin, 0 ) ;
		} else {
			if ( left ) {
				if ( inv ) {
					DG_COPY_MAT( &work->pin->world, &work->weapon_sub.objs->world ) ;
				} else {
					DG_COPY_MAT( &work->pin->world, &work->weapon.objs->world ) ;
				}
			} else {
				DG_COPY_MAT( &work->pin->world, root ) ;
			}
			GM_GroupObjs( work->pin, work->ctrl->map ) ;
			DG_VisibleObjsChanl( work->pin, 0 ) ;
		}
	}
	if ( work->lever != NULL ) {
		if ( work->flag == 0 && trg == TRIG_YET && inv ) {
			DG_InvisibleObjsChanl( work->lever, 0 ) ;
		} else {
			if ( left ) {
				if ( inv ) {
					DG_COPY_MAT( &work->lever->world, &work->weapon_sub.objs->world ) ;
				} else {
					DG_COPY_MAT( &work->lever->world, &work->weapon.objs->world ) ;
				}
			} else {
				DG_COPY_MAT( &work->lever->world, root ) ;
			}
			GM_GroupObjs( work->lever, work->ctrl->map ) ;
			DG_VisibleObjsChanl( work->lever, 0 ) ;
		}
	}
	
    switch ( trg ) {
	case TRIG_YET :
		if ( work->flag == 1 ) {
			/* もし強制モードに入ったらなかったことにする */
			/* デモも */
			if ( GM_CheckPlayerStatus( PLAYER_FORCE ) ||
				 !PL_PadEnable() ||
				 GM_CheckGameStatus( STATE_PLAY_DEMO ) ) {
				work->wait = 0 ;
				work->count = BOMB_COUNT ;
				work->flag = 0 ;
				break ;
			}
			if ( this != THIS_IS_MAGAZINE &&
				 !PL_AttackDisable() && !GM_CheckPlayerStatus( PLAYER_INVINCIBLE_ALL ) && 
				 !GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ) 
         {
            //BP_INPUT - disable grenade exploding in hand?
            if(!gBP_Game_GrenadeExplodeInHand_Enable)
            {
               work->count = BOMB_COUNT;
            }
            //BP_INPUT - disable grenade exploding in hand?

				if ( -- work->count == 0 ) 
            {
					FVECTOR			pos ;

					GV_MatToVec( root, &pos ) ;
					if ( HZX_OnlineHazardCheck( work->ctrl->hzx_id, &work->ctrl->mov, &pos,
											    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
											    0, 0 ) ) 
               {
						FVECTOR		hit ;
						float		len ;
						
						HZX_GetOnlineVector( &hit ) ;
						len = GV_VecLen3F( &hit ) ;
						if ( len > 5.0F ) {
							GV_LenVec3F( &hit, &hit, 0.0F, len - 4.0F ) ;
							_sceVu0AddVector( &pos, &work->ctrl->mov, &hit ) ;
						}
					}
					switch( this ) {
					case THIS_IS_GRENADE :
						NewBlast3( &pos, BOTH_SIDE, 1000, 
								   2000, DMG_BLAST, FNT_BLAST, WpNum[ this ], 0 ) ;
						GM_SetPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ;
						break ;
					case THIS_IS_CHAFF :
						NewBlastChaff( root ) ;
						NewBlast3( &pos, BOTH_SIDE, 1000, 
								   2000, DMG_BLAST / 4, FNT_BLAST / 4, WpNum[ this ],
								   BLAST_TYPE_NO_FIRE | BLAST_TYPE_NO_NOISE | BLAST_TYPE_NO_SE ) ;
						GM_SetPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ;
						break ;
					case THIS_IS_STUN :
						NewBlastStun( root ) ;
						NewBlast3( &pos, BOTH_SIDE, 10000, 
								   10000, 0, 0, WP_StunGrenade, 
								   BLAST_TYPE_NO_FIRE | BLAST_TYPE_ONETARG ) ;
						NewBlast3( (FVECTOR *)root->m[3], BOTH_SIDE, 20000, 
								   20000, 0, 0, WP_StunFar, 
								   BLAST_TYPE_NO_FIRE | BLAST_TYPE_ONETARG ) ;
						/* プレイヤーにダメージを与える */
						NewBlast3( &pos, PLAYER_SIDE, 1000, 2000,
								   DMG_BLAST / 4, FNT_BLAST / 4, WP_ChaffGrenade,
								   BLAST_TYPE_NO_FIRE | BLAST_TYPE_NO_NOISE | BLAST_TYPE_NO_SE ) ;
						GM_SetPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ;
						break ;
					default :
					  ;
					}
					GM_DecrementWeapon( WpNum[ this ], 1 ) ;
					if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
					work->wait = WAIT_COUNT ;
					work->count = BOMB_COUNT ;
					work->flag = 0 ;
				}
			}
		}
		break ;
    case TRIG_SET :
		work->flag = 1 ; /* ピンぬいた */
		if ( work->pin != NULL ) {	/* ピンとばし */
			FVECTOR		force = { 128.0F, 12.0F, 16.0F } ;

			/* イントルードのときは出さないでおく */
			if ( !GM_CheckPlayerStatus( PLAYER_INTRUDE ) ) {
				DG_SetPos( &GM_PlayerBody->objs->objs[ HUMAN21_ATAMA ].world ) ;
				DG_RotVector( &force, &force, 1 ) ;
				NewFallingBulletEX( &work->pin->world, &DG_ZeroVector, &force, 
								   PinDatas[ this * 2 ],
								   0.0F, 0.0F, 
								   FALLBUL_TYPE_YDOWN | FALLBUL_TYPE_ENDDESTROY | 
								   FALLBUL_TYPE_RANDOM | FALLBUL_TYPE_CHANL0ONLY |
								   FALLBUL_TYPE_NONEARCHECK, 0 ) ;
			}
			DG_InvisibleObjsChanl( work->pin, 0 ) ;
		}
		break ;
    case TRIG_FIRE_LEVEL0 :
    case TRIG_FIRE_LEVEL1 :
    case TRIG_FIRE_LEVEL2 :
    case TRIG_FIRE_LEVEL3 :
    case TRIG_FIRE_LEVEL4 :
	case TRIG_FALL :
		/* 投げるべし */
		if ( trg != TRIG_FALL ) {
			if ( work->flag == 0 ) break ; /* もう持ってない（保険） */
			if ( GM_GameStatus & STATE_GLL ) {
				DG_COPY_VEC( &force, &GllForce ) ;
			} else {
				DG_COPY_VEC( &force, &Force ) ;
			}
			if ( GM_CheckPlayerStatus( PLAYER_GROUND ) ) {
				/* 匍匐時 */
				force.vz *= 2.0f + 0.8f * ( float )( trg - TRIG_FIRE_LEVEL0 ) ;
				force.vy *= 0.4f + 0.05f * ( float )( trg - TRIG_FIRE_LEVEL0 ) ;
			} else {
				force.vz *= 1.0f + 1.0f * ( float )( trg - TRIG_FIRE_LEVEL0 ) ;
				force.vy *= 1.0f + 0.13f * ( float )( trg - TRIG_FIRE_LEVEL0 ) ;
			}
			/* ビハインドなげ時 */
			if ( GM_CheckPlayerStatus( PLAYER_BEHIND ) ) {
				force.vz *= -1.0F ;
			}
		} else {
			if ( work->flag == 0 ) break ; /* まだピンぬいてない */
			DG_COPY_VEC( &force, &FallForce ) ;
		}
		switch( this ) {
		case THIS_IS_CHAFF :
			break ;
		default :
		  ;
		}
		if ( this != THIS_IS_MAGAZINE ) {
			mov.vx = root->m[ 3 ][ 0 ] ;
			mov.vy = root->m[ 3 ][ 1 ] ;
			mov.vz = root->m[ 3 ][ 2 ] ;
		} else {
			DG_SetPos( root ) ;
			DG_MovePos( MagShift ) ;			
			DG_GetPos( &world ) ;
			GV_MatToVec( &world, &mov ) ;
		}
		
		/* 主観のときはカメラ位置から出す */
//		if ( inv ) {
//			DG_COPY_VEC( &mov, &work->camera->position ) ;
//			mov.vy -= 250.0F ;	/* ちょっと下 */
//		} else {
			/* なげ位置との間に壁がある場合 */
			if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
				DG_COPY_VEC( &from, &GM_PlayerSubjectCamera[ 0 ]->position ) ;
			} else {
				GV_MatToVec( &GM_PlayerBody->objs->objs[ HUMAN21_KUBI ].world, &from ) ;
			}
	        if ( HZX_OnlineHazardCheck( work->ctrl->hzx_id, &from,
									    &mov, HZX_CHK_ALL,
									    HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
									    HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
				FVECTOR		vec ;
				float		len ;
				HZX_GetOnlineVector( &vec ) ;
				len = GV_VecLen3F( &vec ) ;
				if ( len > 10.0F ) {
					GV_LenVec3F( &vec, &vec, 0.0F, len - 10.0F ) ;
				}
				_sceVu0AddVector( &mov, &from, &vec ) ;
				//printf( "grenade.c : mov mov online\n" ) ;
			}
//		}
#if 0
		adjx = GM_PlayerBody->m_ctrl->step.vx ;
		if ( adjx < 0.0F ) adjx = - adjx ;
		adjz = GM_PlayerBody->m_ctrl->step.vz ;
		if ( adjz < 0.0F ) adjz = - adjz ;
		force.vz += adjx + adjz ;
#endif
		DG_SetPos2( &mov, &rot ) ;
		DG_GetPos( &world ) ;
		/* ちょっとランダム */
		if ( !GM_CheckGameStatus( STATE_PAD_DEMO ) ) {
			_sceVu0ScaleVector( &force, &force, 1.0F + 0.03F * ( float )( GV_Time % 6 - 4 ) ) ;
		}
		NewBulletGrenade( &world, &force, BOTH_SIDE, WpNum[ this ], 
						 BodyDatas[ this * 2 ], work->count ) ;
		if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;

		if ( work->lever != NULL ) {
			/* レバー */
			DG_SetPos( &world ) ;
			DG_RotVector( &force, &force, 1 ) ;
			force.vx *= 0.90F ;
			force.vy *= 0.75F ;
			force.vz *= 0.90F ;
			NewFallingBulletEX( &world, &DG_ZeroVector, &force, PinDatas[ this * 2 + 1 ],
							   0.0F, 0.0F, 
							   FALLBUL_TYPE_YDOWN | FALLBUL_TYPE_ENDDESTROY | 
							   FALLBUL_TYPE_RANDOM | FALLBUL_TYPE_ENDXROTZERO | FALLBUL_TYPE_CHANL0ONLY |
							   FALLBUL_TYPE_NONEARCHECK, 0 ) ;
			DG_InvisibleObjsChanl( work->lever, 0 ) ;
		}

		GM_DecrementWeapon( WpNum[ this ], 1 ) ;
		work->wait = WAIT_COUNT ;
		work->count = BOMB_COUNT ;
		work->flag = 0 ;
		/* なげたフレームでは表示ＯＦＦ */
		DG_InvisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjs( work->weapon_sub.objs ) ;
		break ;
    default :
	  ;

    }
	if ( GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ) {
		if ( work->pin != NULL ) DG_InvisibleObjs( work->pin ) ;
		if ( work->lever != NULL ) DG_InvisibleObjs( work->lever ) ;
	}	
}

static	void	Die( work )
Work		*work ;
{
    if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
    }
    GM_FreeObject( &( work->weapon ) ) ;
    GM_FreeObject( &( work->weapon_sub ) ) ;
	GM_PlayerWeaponBody = NULL ;
	GM_PlayerSubWeaponBody = NULL ;

	if ( work->pin != NULL ) {
		DG_DequeueObjs( work->pin ) ;
		DG_FreeObjs( work->pin ) ;
	}
	if ( work->lever != NULL ) {
		DG_DequeueObjs( work->lever ) ;
		DG_FreeObjs( work->lever ) ;
	}
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( work, body, unit, this )
Work		*work ;
OBJECT		**body ;
int		*unit ;
int		this ;
{
    OBJECT	*weapon ;

    weapon = &( work->weapon ) ;
    GM_InitObject( weapon, BodyDatas[ this * 2 ], BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
    GM_ConfigObjectRoot( weapon, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
    weapon = &( work->weapon_sub ) ;
    GM_InitObject( weapon, BodyDatas[ this * 2 + 1 ], BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
    GM_ConfigObjectRoot( weapon, *body, *unit ) ;
    DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
    work->camera = GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] ;
	work->flag = 0 ;

	if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
		GM_SetPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ;
	}

	DG_InvisibleObjs( work->weapon_sub.objs ) ;

	GM_PlayerWeaponModel = BodyDatas[ this * 2 ] ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon_sub ;

	/* ピン＆レバー */
	{
		int		pin, lever ;
		DG_DEF	*def ;
		DG_OBJS	*objs ;

		work->pin = work->lever = NULL ;
 		pin = PinDatas[ this * 2 ] ;
		lever = PinDatas[ this * 2 + 1 ] ;
		if ( pin != 0 ) {
			def = ( DG_DEF * )GV_GetCache( GV_CacheID( pin, 'k' ) ) ;
			ASSERT( def != NULL ) ;
			objs = work->pin = DG_MakeObjs( def, DG_FLAG_SHADE | DG_FLAG_ONEPIECE, 0 ) ;
			if ( objs == NULL ) return -1 ;
			DG_COPY_MAT( &objs->world, work->weapon.objs->root ) ;
			DG_SetLightMatrix( objs, work->weapon.objs->light ) ;
			GM_GroupObjs( objs, ( *body )->map_name ) ;
			DG_QueueObjs( objs ) ;
			DG_InvisibleObjs( objs ) ;
			DG_VisibleObjsChanl( objs, 0 ) ;
		}
		if ( lever != 0 ) {
			def = ( DG_DEF * )GV_GetCache( GV_CacheID( lever, 'k' ) ) ;
			ASSERT( def != NULL ) ;
			objs = work->lever = DG_MakeObjs( def, DG_FLAG_SHADE | DG_FLAG_ONEPIECE, 0 ) ;
			if ( objs == NULL ) return -1 ;
			DG_COPY_MAT( &objs->world, work->weapon.objs->root ) ;
			DG_SetLightMatrix( objs, work->weapon.objs->light ) ;
			GM_GroupObjs( objs, ( *body )->map_name ) ;
			DG_QueueObjs( objs ) ;
			DG_InvisibleObjs( objs ) ;
			DG_VisibleObjsChanl( objs, 0 ) ;
		}
	}

    return 0 ;
}

static	void	*New( ctrl, body, unit, trigger, side, this )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side, this ;
{
    Work	*work ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
			BodyDatas = BodyDatasSna ;
			PinDatas = PinDatasSna ;
			MagShift = &MagShiftSna ;
		} else {
			BodyDatas = BodyDatasRai ;
			PinDatas = PinDatasRai ;
			MagShift = &MagShiftRai ;
		}
	} else if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
		BodyDatas = BodyDatasSna ;
		PinDatas = PinDatasSna ;
		MagShift = &MagShiftSna ;
	} else {
		BodyDatas = BodyDatasRai ;
		PinDatas = PinDatasRai ;
		MagShift = &MagShiftRai ;
	}

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, body, unit, this ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->side = side ;
		work->count = BOMB_COUNT ;
		work->this = this ;
		work->chanl = GM_CurrentCameraChanl ;
		work->vflag = ( DG_FLAG_INVISIBLE0 << work->chanl ) ;
    }
    return work ;
}

/*------------------------------------------------------------------*/

/* Chaff */
void	*NewChaff( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
    return New( ctrl, body, unit, trigger, side, THIS_IS_CHAFF ) ;
}

/* Stun */
void	*NewStun( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
    return New( ctrl, body, unit, trigger, side, THIS_IS_STUN ) ;
}

/* Magazine */
void	*NewMagazine( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
    return New( ctrl, body, unit, trigger, side, THIS_IS_MAGAZINE ) ;
}

/* Grenade */
void	*NewGrenade( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
    return New( ctrl, body, unit, trigger, side, THIS_IS_GRENADE ) ;
}
