//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gll_ef.c
	chara ゴルルゴン水柱
	2002/07/10 K.Sigeno
	$Id: gll_ef.c,v 1.1.1.3 2002/11/19 11:49:15 Yoshizawa1 Exp $
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

/* ワーク */
//#define		POS_MAX (4)
#define		POS_MAX (1)

typedef	struct _Work {
	GV_ACT_EX	actor ;
	FVECTOR		ef_pos[POS_MAX] ;	/*表示座標*/
	FVECTOR		center ;
	int			cnt ;
	int			cycle ;
	int			limit ;
	float		len ;
	float		height ;
	int			sp_flag ;
} Work ;




//extern void *NewAutoSplush( FVECTOR *pos, float radius ) ;
extern void *NewGllSplush( FVECTOR *pos, float radius ,int *sw) ;

extern void *NewWaterSurfaceMountain( FVECTOR *center, float radius, float intense, int flag ) ;

extern float GM_WaterLevel;

static float isin( d )
int	d ;
{
	float f, t ;

	f = ( float )M_PI * d / 2048.0F ;
	t = sinf( f ) ;
	return t ;
}
static void Act(Work *work)
{
#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif
	int i;
//	if(work->cnt == work->limit){
	if(work->cnt <= 0){
		GV_DestroyActor(work) ;
		return ;
	}
	for(i=0;i<POS_MAX;i++){
		work->ef_pos[i].vy = 
			(work->center.vy) + isin((work->cnt*work->cycle + (i*(4096/POS_MAX)) )&4095 ) 
				* ((float)(i+1)/(float)POS_MAX) 
				* (work->height*work->cnt/work->limit) ;
	}


//	work->cnt++;
	work->cnt--;
}
static void Die(Work *work)
{
}
static int GetResources(Work *work, int name, int where)
{
	int i ;
	work->len = (float) GCL_GetOptionValue( 'w', 0 ) ;
	work->height = (float) GCL_GetOptionValue( 'h', 0 ) ;
	work->cycle = (4095/DIRECT_TICK(60)) * GCL_GetOptionValue( 'c', 1 )  ;
	if ( GCL_GetOption( 'p' ) ){
		work->center.vx = (float) GCL_GetNextInt() ; 
		work->center.vy = (float) GCL_GetNextInt() ; 
		work->center.vz = (float) GCL_GetNextInt() ; 
	}
	for(i=0;i<POS_MAX;i++){
		work->ef_pos[i] =  work->center ; 
	}
	if ( GCL_GetOption( 't' ) ){
		work->limit = GCL_GetNextInt() ; 
	}
//	work->cnt = 0 ;
	work->cnt = work->limit ;

	work->sp_flag = 1 ;
	for(i=0;i<POS_MAX;i++){
		if(1){
//			GV_SetActorChild( work, NewAutoSplush( &work->ef_pos[i], work->len * (float)(i+1)/(float)POS_MAX) );
			GV_SetActorChild( work, 
				NewGllSplush(&work->ef_pos[i],work->len*(float)(i+1)/(float)POS_MAX,&work->sp_flag));
		}
	}


	return 1;
}

void *NewGllEffectCall( name , where )
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

