//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blurpoint.c
	残像点
	$Id: blurpoint.c,v 1.1.1.3 2002/11/19 11:49:51 Yoshizawa1 Exp $
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

#define		B_SIZE	(35)
#define		B_RISE	(10.0f)

enum {
	LIGHT_OFF = 0,
	LIGHT_ON
};

typedef	struct	{
	GV_ACT_EX		actor ;
	FVECTOR		ef_pos ;
	int			rgba ;
	int			b_mode ;
	FMATRIX *world ;
	FVECTOR shift ;
	CVECTOR col ;
	int code ;
	int *sw ;
	/*子アクター保持*/
	void	*blur ;
	void	*point ;
} Work ;


extern void *NewGeneralSprite(int,FVECTOR *,float,int *,int,int,int,int * );
extern void *NewInterPoly_Gr( FVECTOR *, float , CVECTOR );

static void SetRGBA(int r ,int g ,int b ,int a,int *rgba){
    *rgba = ((a<<24)|(b<<16)|(g<<8)|(r)) ;
} 
static void Act(Work *work)
{

//extern void PosBox(FVECTOR * ,float ,SVECTOR * );

	DG_SetPos( work->world );
	DG_PutVector( &work->shift, &work->ef_pos, 1 );
//printf("HEAD EF POS X[%f] Y[%f] Z[%f]\n",work->ef_pos.vx,work->ef_pos.vy,work->ef_pos.vz) ;
GM_CurrentMap = GM_CurrentStageMap ;
//PosBox(&work->ef_pos,250.0f,NULL);

	if((*work->sw)<1){
		/** LIGHT_OFF **/
		if ( work->blur != NULL ) {
			GV_DestroyOtherActor( work->blur ) ;
			work->blur = NULL ;
		}
		if ( work->point != NULL ) {
			GV_DestroyOtherActor( work->point ) ;
			work->point = NULL ;
		}
	}else {
		/* LIGHT_ON */
		if ( work->blur == NULL ) {
			/*残像*/
			work->blur = NewInterPoly_Gr( &work->ef_pos, 5.0f , work->col );
			if(work->blur != NULL) {
				GV_SetActorChild( work , work->blur );
			}
		}
		if ( work->point == NULL ) {
			/*雪洞*/
			work->b_mode = (0x80010001) ;
			work->point = NewGeneralSprite(work->code,&work->ef_pos,
				B_RISE,&work->rgba,B_SIZE,B_SIZE,0,&work->b_mode );
//				100.0f,&work->rgba,100,100,0,&work->b_mode );
			if(work->point != NULL) {
				GV_SetActorChild( work , work->point );
			}
		}
	}
}
static void Die(Work *work)
{
}
static int GetResources( Work *work,FMATRIX *world,FVECTOR *shift,CVECTOR *col ,int *sw){
	work->world = world;
	work->shift = *shift ;
	work->col = *col;
	work->sw = sw ;
	work->blur = NULL ;
	work->point = NULL ;
	work->code = GV_StrCode("svc_bonbori") ;
	SetRGBA(work->col.r ,work->col.g ,work->col.b ,work->col.cd, &work->rgba);
	work->b_mode = (0x00010000)|(0x80000001) ;

	return 1;
}
void *NewSIG_BlurPoint(FMATRIX *world,FVECTOR *shift,CVECTOR *col ,int *sw){
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0x40 ) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX (&work->actor ) ;
		if(!GetResources( work,world,shift,col ,sw )){
		    GV_DestroyActor(work) ;
		    return NULL ;
		}
    }
    return (void *)work ;
}

