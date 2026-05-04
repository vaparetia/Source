//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	enecorps.c
	Å¨Ê¼»àÂÎ

	2000/05/30 Y.Korekado
	$Id: enecorps.c,v 1.1.1.3 2002/11/19 11:44:08 Yoshizawa1 Exp $
	
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

#include	"korekado/conv/corps.h"

typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		weapon ;
	
	int			wp_name ;
	int			sw_gun ;
	int			sw_radio ;
	int			sw_nsight ;
	int			sw_magg ;
	int			status ;
} Work ;

static void Act( Work *work )
{
}

static void Die( Work *work )
{
}
/*--- ------------------------------------------------------------*/
static	void	ENE_CorpsDieCallBack( ptr, body, ctrl )
void		*ptr ;
OBJECT		*body ;
CONTROL		*ctrl ;
{
	Work	*work ;

	work = ( Work * )ptr ;
}

void	ENE_CorpsSetCallBack( ptr, body, ctrl )
void		*ptr ;
OBJECT		*body ;
CONTROL		*ctrl ;
{
	extern void *NewSlingGun( OBJECT *, OBJECT *, int, int * ) ;
	extern void *NewCartridgeControl( int *, OBJECT *, OBJECT *, int );
	extern void	*NewRadio( OBJECT *, int, int *, TARGET * ) ;
	extern void	*NewNightSight( OBJECT *, int, int, int * ) ;
	Work	*work ;

	work = ( Work * )ptr ;

	work->sw_magg = SW_FLAG_VISIBLE ;
	work->sw_radio = SW_FLAG_VISIBLE ;
	work->sw_gun = SW_FLAG_VISIBLE ;

	GV_SetActorChild( work, NewSlingGun( body, &work->weapon, work->wp_name, &work->sw_gun ) ) ;
	UNSET_FLAG( work->sw_gun, SW_FLAG_SWITCH1 ) ;
	ENE_InitSling( work, body, &work->weapon, body->objs->light ) ;

	GV_SetActorChild( work, NewCartridgeControl( &work->sw_magg, body, &work->weapon, 0 ) ) ;
	GV_SetActorChild( work, NewRadio( body, GV_StrCode("rad"), &work->sw_radio, NULL ) ) ;

	if ( work->status & (ENE_STATUS_NIGHT_SIGHT|ENE_STATUS_NIGHT_SIGHT2) ) {
		GV_SetActorChild( work,NewNightSight( body, GV_StrCode("gbs_nvg2"),GV_StrCode("gbs_nvg1"),&work->sw_nsight)) ;
		work->sw_nsight = (work->status & ENE_STATUS_NIGHT_SIGHT)?1:2 ;
	}

	ENE_SetAttachment( ptr, body ) ;
}


#define	CORPSNAME	GV_StrCode("»àÂÎ")

static int GetResources( work, name_id, pos, rot, body, motion_num, down_s, status, act )
Work 	*work ;
NAME_ID	*name_id ;
FVECTOR	*pos ;
SVECTOR	*rot ;
OBJECT	*body ;
int		motion_num ;
int		down_s ;
int		status ;
ACTION	*act ;
{
	extern void *NewCorp( int name, OBJECT *body, FVECTOR *pos, SVECTOR *rot, int model, 
		int down_s, void *ooze, void *ptr, CORPS_CALLBACK set, CORPS_CALLBACK die, int status ) ;

	work->status = status ;
	work->wp_name = name_id->weapon ;

	NewCorp( CORPSNAME, body, pos, rot, name_id->body, 
		down_s, act->oozeblood, (void *)work, ENE_CorpsSetCallBack, ENE_CorpsDieCallBack, status ) ;

//	GM_SetTargetCallBack( off, ComboCallBack, ( void * )work ) ;

	return (0);
}

/* Å¨Ê¼ÍÑ */
Work *NewEneCorps( name_id, pos, rot, body, motion_num, down_s, ooze, status, act )
NAME_ID	*name_id ;
FVECTOR	*pos ;
SVECTOR	*rot ;
OBJECT	*body ;
int		motion_num ;
int		down_s ;
void	*ooze ;
int		status ;
ACTION	*act ;
{
	extern void TAKABE_OozeBloodChangeObjs( void *, DG_OBJS * ) ;
	Work		*work ;

	OPERATOR() ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, name_id, pos, rot, body, motion_num, down_s, status, act ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return work ;
}
