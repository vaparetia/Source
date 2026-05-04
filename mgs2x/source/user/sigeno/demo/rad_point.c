//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rad_point.c
	レーダーに光点を登録
	$Id: rad_point.c,v 1.1.1.3 2002/11/19 11:49:26 Yoshizawa1 Exp $
*/


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include "gameheader.h"
#include "libutl.h"

enum {
	RADAR_MODE_RED	= 0 ,
	RADAR_MODE_NODE,
};

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	RADAR_CTRL	rctrl ;
	FVECTOR		pos ;
} Work ;


static void Act(Work *work)
{
}
static void Die(Work *work)
{
	GM_FreeRadarControl(&work->rctrl);
}
static int GetResources(Work *work, int name, int where)
{
	int buf[3] ;
	int col= 3 ,flag = RADAR_VISIBLE;
	if ( GCL_GetOption( 'p' ) != NULL ) {
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->pos ) ;
	}
	if ( GCL_GetOption( 'c' ) != NULL ) {
		switch(GCL_GetNextInt()){
			case RADAR_MODE_RED :
				col = 3 ;
				break ;
			case RADAR_MODE_NODE :
				flag |= RADAR_NODE ;
				break ;
			default :
				break ;
		}
	}
	work->rctrl.col = col ;
	GM_InitRadarControl(&work->rctrl,&work->pos,flag , where ) ;
    GM_RadarSetVRange( &work->rctrl, 3000 , -3000 );
	GV_SetActorMessageKill( work, name );
	return 1;
}

/* 初期化部メイン */
void *NewSigRadPoint( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


