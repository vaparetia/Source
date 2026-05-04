//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	asiato.c
	アイテム

	2000/06/16 Y.Korekado
	$Id: asiato.c,v 1.1.1.3 2002/11/19 11:44:26 Yoshizawa1 Exp $
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
typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		body ;

	ENEFINDLIST	*efl ;
	ENEFIND		enefind ;
	int			life ;
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */
#define ASIATO_LIFE	60*20
#define BODY_NAME	(104754)
/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	if( (work->life--) <= 0 ) GV_DestroyActor( work ) ;
}

static	void	Die( work )
Work		*work ;
{
	GM_FreeObject( &work->body ) ;
	GM_FreeEneFindFromList( work->efl, &work->enefind ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work, ENEFINDLIST *efl, FVECTOR *pos, SVECTOR *rot, int zaddr )
{
	work->efl = efl ;

	GM_SetEneFind( &work->enefind, pos, zaddr, EF_TYPE_FOOTSTAMP ) ;
	GM_PutEneFindToList( efl, &work->enefind ) ;

	GM_InitObject( &work->body, BODY_NAME, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	DG_SetPos2( pos, rot ) ;
	DG_PutObjs( work->body.objs );

	work->life = ASIATO_LIFE ;
	return 0 ;
}

void	*NewAsiato( ENEFINDLIST *efl, FVECTOR *pos, SVECTOR *rot, int zaddr )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, efl, pos, rot, zaddr ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
