//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	hitech_face_blood.c
	物体落下跳ね返り
	2001/06/21 S.Okajima
	$Id: hitech_face_blood.c,v 1.1.1.3 2002/11/19 11:47:32 Yoshizawa1 Exp $

*/

#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"

//#define		GRAVITY		( 6.0f ) 
#define		FALL_TIME	( 60*60*1 ) 

#define		MAX_PAT		( 1 ) 
#define		MAX_MAG		( 32 ) 

typedef	struct	{
	GV_ACT_EX	actor ;
	DG_OBJS		*hitech_objs ;
	DG_OBJS		*objs ;
} Work ;


static	void Act( Work *work )
{
//	work->objs->flag     = work->hitech_objs->flag;
	DG_VisibleObjs( work->objs) ;
	work->objs->group_id = work->hitech_objs->group_id;
}

static	void Die( Work *work )
{
	if( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
}

static	int GetResources( Work *work, DG_OBJS *objs )
{
	DG_DEF		*def ;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( 11245660 /*"htc_blood_face"*/, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 );
	if( work->objs == NULL ) return -1;
	DG_QueueObjs( work->objs );

	work->hitech_objs = objs;
	work->objs->root        =&work->hitech_objs->objs[HUMAN21_ATAMA].world;
	DG_SetLightMatrix( work->objs, work->hitech_objs->objs[HUMAN21_ATAMA].light );

	return 0 ;
}

void *NewHiTechFaceBlood( DG_OBJS *objs )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, objs ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
