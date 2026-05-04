//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	radio_break.c
	ラジオブレイク  

	2000/01/13 H.TANAKA
	2000/10/18 S.Okajima
	$Id: radio_break.c,v 1.1.1.3 2002/11/19 11:47:44 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

/* ----------------------------------------------------- */
#define     MAX_COUNT   50

typedef struct
{
    GV_ACT_EX  actor ;

    FMATRIX  *world ;

    int     all_count ;
    int     fire_count ;
} Work ;

extern void *NewSpark2(int n_prims, FMATRIX  *world,
     float min_speed, float speed_wide, float gravity,
     SVECTOR *rot, SVECTOR *rot_wide, FVECTOR  *color, 
     float length, int count 
) ;

//extern void  Big_TmpLight2(FVECTOR *pos,float r_range,float e_range,int color,int flag) ;

/* --------------------------------------------------------- */
static  void  Fire_Spark(Work *work)
{
    SVECTOR   local_rot ;
    SVECTOR   rot_wide ;
    FVECTOR   color ;
    FMATRIX   mat ;
    FVECTOR   shift ;
    int       n ;

    if((work->fire_count -- < 0) || (work->all_count == MAX_COUNT))
    {
//	local_rot.vx = irnd() % 4096 ;
//	local_rot.vy = irnd() % 4096 ;
	local_rot.vx = -128 + irnd() % 256 ;
	local_rot.vy = 1536 + irnd() % 1024 ;
       
	local_rot.vz = 0 ;

	rot_wide.vx = 512 ;
	rot_wide.vy = 512 ;
	color.vx = 255.0F ;
	color.vy = 200.0F ;
	color.vz = 128.0F ;
	color.vw = 30.0F ;
	
        DG_SetPos(work->world) ;
        shift.vx = - 6.0F + rnd() * 12.0F ;
        shift.vy = 30.0F + rnd() * 60.0F ;
        shift.vz = 5.0F + rnd() * 12.0F ;
        DG_MovePos(&shift) ;
        DG_GetPos(&mat) ;
	n = 15 + irnd() % 5 ;
	NewSpark2(n,&mat,9.0F,19.0F,(rnd() * 1.0F),&local_rot,&rot_wide,&color,1.0F,15) ;

//	Big_TmpLight2((FVECTOR *)(&mat),300.0F, 600.0F, 0x406080,3) ;
	DG_SetTmpLight2(
		(FVECTOR *)(mat.m[3]),
		300.0f,
		600.0f,
		80 | 60<<8 | 40 <<16,
		LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY ) ;

	work->fire_count = irnd() % 10 + 5 ;
    }
}

static  void  Act(Work *work)
{
    
    if(work->all_count > MAX_COUNT)
    {
	GV_DestroyActor( work ) ;
	return  ;
    }

    /* 火花のエフェクト */
    Fire_Spark(work) ;
    work->all_count ++ ;
}

static  void Die(Work *work)
{
}

static  int  GetResources(Work *work, FMATRIX *world)
{
    work->world = world ;
    work->all_count = 0 ;
    work->fire_count = 0;
    
    return 0 ;
}

void *Radio_BreakEffect(FMATRIX *world)
{
	Work *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER, sizeof(Work)) ;
	if( work != NULL ){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if( GetResources( work, world ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return work ;

}
