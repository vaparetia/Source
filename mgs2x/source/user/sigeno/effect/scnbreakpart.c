//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scnbreakpart.c
	多間接モデルの1部のみ変形 シナリオ呼び版
	2002/08/16 K.Sigeno
	$Id: scnbreakpart.c,v 1.3 2002/11/23 12:46:55 Yoshizawa1 Exp $
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
#include "../effect/sig_vanim.h"

#define MAX_BODY_PARTS	(21)
#define STATUS_DBROKEN	(02)

#define LEN1	(10.0f)
#define LEN2	(13.0f)
#define TIME_A1	(10)	
#define TIME_A2	(180)
#define SCALE1	(0.4f)
#define SCALE2	(0.8f)
#define RE_TIME	(120)

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

extern void *NewGolBreakPart2(DG_OBJ *obj,int *sw,float len1 ,float len2 ,int time_a1,int time_a2,
	float scale1,float scale2,int re_time) ;

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	int			parts_bit ;
	int			sw[MAX_BODY_PARTS] ;
} Work ;
static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}

	return ( NULL );
}

static void Act(Work *work){
}
static void Die(Work *work){
}
static int GetResources(Work *work ,int name,int where){
	int i;
	CONTROL *ctrl ;
	OBJECT *body ;
	DG_OBJS *objs ;
//	work->sw = STATUS_DBROKEN ;
	if ( GCL_GetOption( 'n' ) != NULL ){
		ctrl = SearchControl( GCL_GetNextInt() ) ;
		if(ctrl == NULL ){
			return 0 ;
		}
		body = ctrl->object ;
		if(body == NULL ){
			return 0 ;
		}
		objs = body->objs;
	}else {
		return 0 ;
	}
	if ( GCL_GetOption( 'b' ) != NULL ){
		work->parts_bit = GCL_GetNextInt() ;
	}else {
		work->parts_bit = 0 ;
	}
	for(i=0 ; i<MAX_BODY_PARTS; i++){
		if(work->parts_bit & (1<<i)){
			if(objs->n_models > i){
				work->sw[i] = STATUS_DBROKEN ;
				GV_SetActorChild( &work->actor, (void *)NewGolBreakPart2(
					&(objs->objs[i]),&work->sw[i],LEN1,LEN2 ,TIME_A1,TIME_A2,SCALE1,SCALE2,RE_TIME) );
			}
		}
	}
	return 1;
}

void *NewSigBreakPartScn( int name, int where )
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,name,where)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
