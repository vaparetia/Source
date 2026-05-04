//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wp_mng.c
   武器管理者
   
   2000/04/10 M.Sonoyama
   $Id: wp_mng.c,v 1.1.1.3 2002/11/19 11:50:32 Yoshizawa1 Exp $
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

#define	ACTOR_PRIO	(254)

static	signed char	WeaponFire ;

typedef	struct	{
    GV_ACT	actor ;	
} Work ;

static	void	Act( work )
Work		*work ;
{
	DG_OBJS		*objs ;
	DG_OBJ		*obj ;
	int			i, vflag ;
	long64		inv ;

	if ( GV_PauseLevel != 0 ) return ;

    if ( GM_PlayerBody == NULL || 
		 GM_PlayerArmBody == NULL ) return ;

	/* 発射トリガーの管理 */
	GM_WeaponFire = -1 ;
	if ( WeaponFire >= 0 ) GM_WeaponFire = WeaponFire ;
	WeaponFire = -1 ;

	inv = GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ;
    /* 非表示の管理 */
	if ( GM_PlayerWeaponBody != NULL ) {
		objs = GM_PlayerWeaponBody->objs ;
		obj = objs->objs ;
		vflag = GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE ;
		for ( i = 0; i < objs->n_models; i ++, obj ++ ) {
			if ( inv ) obj->flag |= DG_FLAG_INVISIBLE ;
			else       obj->flag &= ~DG_FLAG_INVISIBLE ;
			obj->flag |= vflag ;
		}
		if ( inv ) objs->flag |= DG_FLAG_SHADOWMAKE ;
		else 	   objs->flag &= ~DG_FLAG_SHADOWMAKE ;
	}

	if ( GM_PlayerSubWeaponBody != NULL ) {
		objs = GM_PlayerSubWeaponBody->objs ;
		obj = objs->objs ;
		vflag = GM_PlayerArmBody->evmobj->flag & DG_EVMOBJ_INVISIBLE ;
		vflag >>= 8 ;	/* DG_EVMOBJ_INVISIBLE0 = 0x0100 */
		vflag <<= 12 ;	/* DG_FLAG_INVISIBLE0 = 0x1000 */
		for ( i = 0; i < objs->n_models; i ++, obj ++ ) {
			if ( inv ) obj->flag |= DG_FLAG_INVISIBLE ;
			else       obj->flag &= ~DG_FLAG_INVISIBLE ;
			obj->flag |= vflag ;
		}	
	}

	/* 腕非表示 */
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ) {
		GM_PlayerArmBody->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
	}

	/* 主観腕のＫＭＳは消す */
	DG_InvisibleObjs( GM_PlayerArmBody->objs ) ;
}

/* 武器管理者 */
/* 武器のアクトの後に実行されるようにする */
void	*NewWeaponManager( void )
{
    Work	*work ;

    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT, 
								   sizeof( Work ), PLAYER_WPMNG_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, NULL ) ;
    }   
	WeaponFire = -1 ;
    return work ;
}

/*----------------------------------------------------------------*/

/* 武器発射の通知 */
void	GM_SetWeaponFire( int wp ) 
{
	WeaponFire = wp ;
}

void	GM_ResetWeaponFire( void )
{
	GM_WeaponFire = -1 ;
}
