//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	poscheck.c
	マップ上の座標取得
	$Id: poscheck.c,v 1.1.1.3 2002/11/19 11:49:07 Yoshizawa1 Exp $
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
#include	"gameheader.h"
#include	"libutl.h"

//extern void *NewBladeEft( FVECTOR *pos0, FVECTOR *pos1, int n_disp, int init_col, int *alpha );
//NewBladeEft( &pos[0], &pos[1], 8, 0x80808000, &debug_alpha );


typedef	struct	{
	GV_ACT_EX			actor ;
	FVECTOR		pos ;
}Work ;

#define SPEED	(25.0f)
#define PAD_NUM	(0)
extern void PosBox(FVECTOR *,float ,SVECTOR *) ;

static void Act(Work *work)
{
	SVECTOR	rgb;
	float speed = SPEED;


	if(GV_PadData[ PAD_NUM ].status & PAD_R2){
		speed *= 2.0f ;
	}
	
//		if(GV_PadData[ PAD_NUM ].press & 
	if(GV_PadData[ PAD_NUM ].status & PAD_L2){
		work->pos.vy -= SPEED ;
	}
	if(GV_PadData[ PAD_NUM ].status & PAD_L1){
		work->pos.vy += SPEED ;
	}

	if(GV_PadData[ PAD_NUM ].status & PAD_U){
		work->pos.vz -= SPEED ;
	}
	if(GV_PadData[ PAD_NUM ].status & PAD_D){
		work->pos.vz += SPEED ;
	}

	if(GV_PadData[ PAD_NUM ].status & PAD_L){
		work->pos.vx -= SPEED ;
	}
	if(GV_PadData[ PAD_NUM ].status & PAD_R){
		work->pos.vx += SPEED ;
	}

	if(GV_PadData[ PAD_NUM ].press & PAD_R1){
		printf("CHECK POS X[%f]Y[%f]Z[%f]\n",work->pos.vx,work->pos.vy,work->pos.vz) ;
	}
	rgb.vy = 127 ;
	
	PosBox(&work->pos,250.0f ,&rgb) ;
}
static void Die(Work *work)
{
}
/*資源確保と各種初期設定*/
static int GetResources( Work *work){
	work->pos  = GM_PlayerPosition ;
	return 1;
}
void	*NewPosChecker( void )
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources(work) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

