//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	homsight.c
	ホーミングカーソル
	2002/03/14 K.Sigeno
	$Id: homsight.c,v 1.1.1.3 2002/11/19 11:49:47 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif


#include	"gameheader.h"
#include	"libutl.h"
typedef	struct	{
	GV_ACT_EX	actor ;
	FMATRIX	*world ;
	void	*child ;
	float hankei ;
	float y_len ;
	float y_low ;
	int		*flag ;
} Work ;

extern void *NewHomingSight2( int *flag,FMATRIX *world,float hankei ,float y_len,float y_low) ;

static	void	Act( work )
Work		*work ;
{
//	int model, i ;
	if(*work->flag == 0){
		if(work->child != NULL){
			 GV_DestroyOtherActor( work->child ) ;
			work->child = NULL ;
		}
	}else {
		if(work->child == NULL){
//printf("work->y_len[%f] work->y_low[%f]\n",work->y_len,work->y_low) ;
			GV_SetActorChild( work, work->child = 
				(void *)NewHomingSight2( work->flag,work->world,work->hankei ,work->y_len,work->y_low)) ;
		}
	}
}

static	void	Die( work )
Work		*work ;
{
}




/*----------------------------------------------------------------*/
static	int	GetResources( Work	*work ,FMATRIX *world,int	*flag,float hankei ,float y_len,float y_low){

	work->flag = flag ;
	work->world = world ;
	work->hankei = hankei ;

	work->y_len = y_len ;
	work->y_low = y_low ;
//printf("work->y_len[%f] work->y_low[%f]\n",work->y_len,work->y_low) ;
	work->child = NULL ;

	return 0 ;
}
void	*NewHomingSight( int *flag,FMATRIX *world,float hankei ,float y_len,float y_low)
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, world,flag,hankei , y_len,y_low) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

void VR_CheckHomingStatus(HOMING_TRG *hom_trg ,int *sw ){
	if(hom_trg->status & HOMING_YOU){
		/*維持*/
		*sw = 1 ;
	}else if((!(hom_trg->status & HOMING_YOU))) {
		/*終了*/
		*sw = 0 ;
	}
	hom_trg->status &= ~HOMING_YOU ;
}

