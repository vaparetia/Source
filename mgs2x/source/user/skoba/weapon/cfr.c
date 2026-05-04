//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   cfr.c
   C4Bomb
   
   1999/12/02 M.Sonoyama
   $Id: cfr.c,v 1.1.1.3 2002/11/19 11:50:26 Yoshizawa1 Exp $
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


/*------------------------------------------------------------------*/

/* 外部関数・変数宣言 */
extern	void	*NewBulletC4( FMATRIX *, int, HZX_HZD *, int ) ;
extern	void	*NewBulletClaymore( FMATRIX *, int, HZX_HZD *, int ) ;

/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SUB	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC)

#define	INVISIBLE_COUNT	(120)

enum {
    THIS_IS_C4 = 0,
    THIS_IS_CLAY
} ;

static	int	WpNum[] = {
    WP_C4Bomb,
    WP_Claymore
} ;

static	int	BodyDatas[] = {
    MDL_C4,						/* cfr */
    MDL_C4_SUB,					/* cfr */
    MDL_CLAYMORE,				/* clm */
    MDL_CLAYMORE_SUB			/* clm_sub */
} ;

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT_EX			actor ;
    OBJECT				weapon ;
    OBJECT				weapon_sub ;
    
    CONTROL				*ctrl ;
    OBJECT				**body ;
    int					*unit ;
    PL_StickWPWork		*trigger ;
    int					side ;
	int					count ;

    int					this ;
	int					chanl ;
	int					vflag ;

	int					clay_line_flag ;
} Work ;

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	int		chanl, vflag ;
    int		this ;
    u_int	trg ;
    FMATRIX	*root, world ;

	chanl = work->chanl ;
	vflag = work->vflag ;

    GM_SetCurrentMap( work->ctrl->map ) ;
    GM_GroupObjs( work->weapon.objs, work->ctrl->map ) ;

#if 1
	if ( work->count > 0 ) {
		DG_InvisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjs( work->weapon_sub.objs ) ;
		-- work->count ;
		return ;
	}
#endif

    this = work->this ;
    if ( GM_PlayerBody->objs->flag & vflag ) {
		GM_ConfigObjectRoot( &( work->weapon_sub ), 
							*( work->body ), *( work->unit ) ) ;
		root = work->weapon_sub.objs->root ;

		DG_InvisibleObjsChanl( work->weapon.objs, 0 ) ;
		DG_VisibleObjsChanl( work->weapon.objs, 1 ) ;

		DG_VisibleObjsChanl( work->weapon_sub.objs, 0 ) ;
    } else {
		GM_ConfigObjectRoot( &( work->weapon ), 
							*( work->body ), *( work->unit ) ) ;
		root = work->weapon.objs->root ;

		DG_VisibleObjsChanl( work->weapon.objs, 0 ) ;
		DG_VisibleObjsChanl( work->weapon.objs, 1 ) ;

		DG_InvisibleObjsChanl( work->weapon_sub.objs, 0 ) ;
    }
	/* 主観腕ができるまでの仮 */
	/* できないかも */
	root = &GM_PlayerBody->objs->objs[ HUMAN21_MIGI_TE ].world ;
	//root = work->weapon.objs->root ;

    trg = work->trigger->trigger ;
	work->clay_line_flag = 0 ;

    switch ( trg ) {
	case TRIG_SET :
		if ( this == THIS_IS_CLAY ) {
			work->clay_line_flag = 1 ;
		}
		break ;
    case TRIG_FIRE :
	case TRIG_FIRE_C4ENEMY :
	case TRIG_FIRE_C4WALL :
		if ( GM_WeaponNum( WpNum[ this ] ) == 0 ) break ;
		/* セット */
		switch( this ) {
		case THIS_IS_C4 :
			if ( GM_N_C4Bombs == GM_C4_MAX ) goto set_unable ;
			GM_SeSetMode( SD_W_C4PUT02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			DG_COPY_MAT( &world, root ) ;
			if ( trg == TRIG_FIRE_C4WALL ) {
				FVECTOR		chk = { 0.0F, 0.0F, -250.0F }, pos, vec ;
				float		len ;

				GV_MatToVec( root, &pos ) ;
				DG_SetPos2( &pos, &work->ctrl->rot ) ;
				DG_PutVector( &chk, &chk, 1 ) ;
				if ( HZX_OnlineHazardCheckOneSegment( ( HZX_HZD * )&work->trigger->hzd,
													  &pos, &chk ) ) {
					HZX_GetOnlineVector( &vec ) ;
					len = GV_VecLen3F( &vec ) ;
					printf( "len %f\n", len ) ;
					if ( len > 50.0F ) {	
						GV_LenVec3F( &vec, &vec, 0.0F, len - 50.0F ) ;
						_sceVu0AddVector( &pos, &pos, &vec ) ;
						GV_VecToMat( &pos, &world ) ;
					}
				}
			} else if ( trg == TRIG_FIRE ) {
				FVECTOR			shift = { 47.5F, -131.5F, 7.5F } ;
				float			level ;
				
				DG_SetPos( root ) ;
				DG_PutVector( &shift, &shift, 1 ) ;
				HZX_SlopeFloorLevel( &level, &shift, ( HZX_HZD * )&work->trigger->hzd ) ;
				printf( "%f\n", shift.vy - level ) ;
				if ( shift.vy - level > 20.0F ) world.m[ 3 ][ 1 ] = level + 20.0F ;
			}
			if ( trg == TRIG_FIRE ) {
				NewBulletC4( &world, BOTH_SIDE, ( HZX_HZD * )&work->trigger->hzd, C4_TYPE_FLOOR ) ;
			} else if ( trg == TRIG_FIRE_C4WALL ) {
				NewBulletC4( &world, BOTH_SIDE, ( HZX_HZD * )&work->trigger->hzd, C4_TYPE_SEGMENT ) ;
			} else {
				NewBulletC4( &world, BOTH_SIDE, ( HZX_HZD * )work->trigger->capture, C4_TYPE_CHARA ) ;
			}
			work->count = DIRECT_TICK( 32 ) ;
			break ;
		case THIS_IS_CLAY :
			if ( GM_N_Claymores == GM_CLAYMORE_MAX ) goto set_unable ;
			GM_SeSetMode( SD_W_MINEPUT1 , &GM_PlayerPosition , GM_SEMODE_BOMB );
			NewBulletClaymore( root, BOTH_SIDE, ( HZX_HZD * )&work->trigger->hzd, 1 ) ;
			work->count = DIRECT_TICK( 60 ) ;
			break ;	    
		default :
		  ;
		}
		DG_InvisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjs( work->weapon_sub.objs ) ;
		GM_DecrementWeapon( WpNum[ this ], 1 ) ;
#if 0
		work->count = INVISIBLE_COUNT ;
#endif
		break ;
    default :
	  ;

    }
	set_unable :
	  ;
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
    GM_ConfigObjectRoot( weapon, *body, *unit ) ;
    weapon = &( work->weapon_sub ) ;
    GM_InitObject( weapon, BodyDatas[ this * 2 + 1 ], BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
    GM_ConfigObjectRoot( weapon, *body, *unit ) ;

    DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;

	GM_PlayerWeaponModel = BodyDatas[ this * 2 ] ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon_sub ;

	if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
		GM_SetPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ;
	}

	DG_InvisibleObjs( work->weapon_sub.objs ) ;

	/* クレイモア線 */
	if ( this == THIS_IS_CLAY ) {
		extern	void	*NewClaymoreLine( FMATRIX *, int * ) ;
		work->clay_line_flag = 0 ;
		GV_SetActorChild( work, 
						  NewClaymoreLine( &GM_PlayerBody->objs->objs[ HUMAN21_MIGI_TE ].world, 
										   &work->clay_line_flag ) ) ;
	}

    return 0 ;
}

/*------------------------------------------------------------------*/

static	void	*New( ctrl, body, unit, trigger, side, this )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
int			side, this ;
{
    Work	*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, unit, this ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = ( PL_StickWPWork * )trigger ;
		work->side = side ;
		work->this = this ;
		work->chanl = GM_CurrentCameraChanl ;
		work->vflag = ( DG_FLAG_INVISIBLE0 << work->chanl ) ;
    }
    return work ;
}

/*------------------------------------------------------------------*/

/* Ｃ４ */
void	*NewC4Bomb( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
int		side ;
{
    return New( ctrl, body, unit, trigger, side, THIS_IS_C4 ) ;
}

/* Ｃｌａｙｍｏｒｅ */
void	*NewClaymore( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
int		side ;
{
    return New( ctrl, body, unit, trigger, side, THIS_IS_CLAY ) ;
}
