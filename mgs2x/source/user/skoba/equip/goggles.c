//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   goggles.c
   ゴーグル系

   2001/02/06	M.Sonoyama
   $Id: goggles.c,v 1.1.1.3 2002/11/19 11:50:16 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
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

#define	OBJECT_FLAG	(DG_FLAG_ONEPIECE|DG_FLAG_SHADE)

extern	void	*NewThermalMode( void ) ;
extern	void	*NewIRMode( void ) ;
extern  void    *NewThermalSight( int name );
extern  void    *NewNightVisionSight( void );

/*------------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			equip ;
	CONTROL			*ctrl ;
	OBJECT			**body ;
	void			*effect ;
	int				item ;
} Work ;

#define THERMAL_SIGHT (0x1)
#define NIGHT_SIGHT   (0x2)
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int			vflag ;
	
	if ( work->effect == NULL ) {
		if ( work->item == IT_Thermal ) {
			work->effect = NewIRMode() ;
		} else if ( work->item == IT_NightVision ) {
			work->effect = NewThermalMode() ;
		}
		if ( work->effect != NULL ) {
			GV_SetActorChild( work, work->effect ) ;
		}
	}

	vflag = GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE ;

	work->equip.objs->flag &= ~DG_FLAG_INVISIBLE ;
	work->equip.objs->flag |= vflag ;

	if ( GM_PlayerBody != NULL &&
		( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) ) {
		int			weapon ;
		/* 武器によってはサイト表示不可 */
		weapon = PL_GetPlayerWeapon() ;
		if ( weapon == WP_Stinger ||
			( weapon == WP_Nikita && !GM_CheckSightStatus( SGT_Nikita ) ) ||
			 weapon == WP_Psg1 ||
			 weapon == WP_Psg1T ) {
			GM_SetSightStatus( SGT_Thermal | SGT_NightVision ) ;	
		} else {
			GM_ResetSightStatus( SGT_Thermal | SGT_NightVision ) ;	
		}
	} else {
		/* サイト表示不可 */
		GM_SetSightStatus( SGT_Thermal | SGT_NightVision ) ;
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER &&
	     GM_AnotherPlayer == GM_ANOTHER_PLAYER_BLADE ) {
	    work->equip.objs->flag |= DG_FLAG_INVISIBLE ;
	}
}

static	void	Die( Work *work )
{
	if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->equip.objs ) ;
	}
	GM_FreeObject( &work->equip ) ;
}

/*------------------------------------------------------------------*/

static	int		GetResources( Work *work, CONTROL *ctrl, OBJECT **body, int *unit,
							  u_int *trigger, int item )
{
	OBJECT		*equip ;
	int			model ;

	equip = &work->equip ;

	if ( item == IT_Thermal ){
		model = MDL_IRG ;
	} else {
		model = MDL_NVG ;
	}
	GM_InitObject( equip, model, OBJECT_FLAG ) ;
	if ( equip->objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( equip, *body, *unit ) ;
	DG_ConnectObjs( GM_PlayerBody->objs, equip->objs ) ;

	if ( item == IT_Thermal ){
		// サイトの起動
		GV_SetActorChild( work , NewThermalSight( 0 ) );
	} else {
		// サイトの起動
		GV_SetActorChild( work , NewNightVisionSight() );
	}
	
	/* 起動時はサイト表示不可 */
	GM_SetSightStatus( SGT_Thermal | SGT_NightVision ) ;

	work->ctrl = ctrl ;
	work->body = body ;
	work->item = item ;
	work->effect = NULL ;
	
	return 0 ;
}

/*------------------------------------------------------------------*/

static	void	*New( CONTROL *ctrl, OBJECT **body, int *unit, u_int *trigger, int item )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, 
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, ctrl, body, unit, trigger, item ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/* 赤外線ゴーグル */
void		*NewIRGoggles( CONTROL *ctrl, OBJECT **body, int *unit, u_int *trigger )
{
	return New( ctrl, body, unit, trigger, IT_Thermal ) ;
}

/* 暗視ゴーグル */
void		*NewNVGoggles( CONTROL *ctrl, OBJECT **body, int *unit, u_int *trigger )
{
	return New( ctrl, body, unit, trigger, IT_NightVision ) ;
}

