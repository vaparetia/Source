//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	asiato.c
	アイテム

	2000/06/16 Y.Korekado
	$Id: asitest.c,v 1.1.1.3 2002/11/19 11:44:26 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include	"korekado/conv/define.h"
/*----------------------------------------------------------------*/
#define	MAX_ASIATO	64
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	ENEFINDLIST	asiato_efl ;
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */
/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	if( GV_PadData[ 0 ].status & PAD_Y ){
		if( !(GV_Time%15) ) {
			extern void *NewAsiato( ENEFINDLIST *efl, FVECTOR *pos, SVECTOR *rot, int zaddr ) ;
			FVECTOR	pos ;
			
			pos = GM_PlayerControl->mov ;
			pos.vy = GM_PlayerControl->levels[0] + 100.0f ;
			NewAsiato( &work->asiato_efl, &pos, &GM_PlayerControl->rot, GM_PlayerControl->addr ) ;
		}
	}
}

static	void	Die( work )
Work		*work ;
{
}

/*----------------------------------------------------------------*/
static	int	GetResources( work )
Work	*work ;
{
	GM_InitEneFindList( &work->asiato_efl, EF_LIST_TYPE_LINK|EF_LIST_TYPE_FOOT ) ;
	GM_PutEneFindList( &work->asiato_efl ) ;

	return 0 ;
}

void	*NewAsiatoControl( )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


