//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/* 
	cigar.c
	2000/10/18 S.Okajima
	$Id: cigar.c,v 1.1.1.3 2002/11/19 11:47:03 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"gameheader.h"
#ifdef PSX2
#include	"def_dma.h"
#endif

/*------------------------------------------------------------------*/
//extern void   *Cigar_Smoke(FMATRIX   *world) ;
/*------------------------------------------------------------------*/

#define	BODY_FLAG	( DG_FLAG_SHADE | DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE )
#define	BODY_NAME	(104839)	/* cig.kms */
#define INTERVAL		 (290)
#define TOP_INTERVAL	 (30)
/*------------------------------------------------------------------*/

typedef	struct {
	GV_ACT_EX		actor ;
	OBJECT		equip ;

	CONTROL		*ctrl ;
	OBJECT		**body ;
	int			*unit ;
	u_int		*trigger ;
	FVECTOR			 color ;

	FMATRIX			 light[2] ;
	int				 count ;
	short			   flag ;
} Work ;

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
static FVECTOR Cigar_Color[] =
{
//	{100.0F,70.0F,70.0F, 0.0F}
	  {120.0F,90.0F,90.0F, 0.0F}

} ;

static	void	Act( work )
Work		*work ;
{
	u_int	trg ;

	if(GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0){
		DG_InvisibleObjsChanl(work->equip.objs, 0) ;
		DG_VisibleObjsChanl(work->equip.objs, 1) ;
	}else{
		DG_VisibleObjs(work->equip.objs) ;
	}

	GM_GroupObjs( work->equip.objs, GM_CurrentStageMap );

	/* たばこに火を付ける */
	work->light[0] = work->equip.objs->objs[1].light[0] ;
	work->light[1] = work->equip.objs->objs[1].light[1] ;
   
	if(work->count == 0){
		work->color.vx += 2.0F * work->flag ;
		if(work->color.vx >= 250.0F){
			if(work->flag == 1){
				work->count = TOP_INTERVAL ;
			}
			work->flag = -1 ;
		}

		if(work->color.vx <= 100.0F){
			if(work->flag == -1){
				//	Cigar_Smoke(&work->equip.objs->world) ;
				work->count = INTERVAL ;
			}
			work->flag = 1 ;
		
		}
	} else {
		work->count -- ;
	}

	work->light[1].m[3][0] = work->color.vx ;
	work->light[1].m[3][1] = work->color.vy ;
	work->light[1].m[3][2] = work->color.vz ;

	GM_ConfigObjectRoot( &( work->equip ), 
			 *( work->body ), *( work->unit ) ) ;
	trg = *( work->trigger ) ;

	work->equip.objs->objs[1].light = work->light ;
}

static	void	Die( work )
Work		*work ;
{
	if ( GM_PlayerBody != NULL ) {
	DG_DisconnectObjs( GM_PlayerBody->objs, work->equip.objs ) ;
	}
	GM_FreeObject( &( work->equip ) ) ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( work, body, unit )
Work		*work ;
OBJECT		**body ;
int		*unit ;
{
	OBJECT	*equip ;

	equip = &( work->equip ) ;
	GM_InitObject( equip, BODY_NAME, BODY_FLAG ) ;
	GM_ConfigObjectRoot( equip, *body, *unit ) ;
	DG_ConnectObjs( GM_PlayerBody->objs, work->equip.objs ) ;

	work->color = Cigar_Color[0] ;
	work->count = 0 ;
	work->flag = 1 ;

	return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewCigar( ctrl, body, unit, trigger )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
{
	Work	*work ;

//	work = ( Work * )GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
	if ( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, body, unit ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
	}
	return work ;
}

