//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_noise.c
	ＶＲ的破壊エエフェクト
	2002/03/29 K.Sigeno
	$Id: vr_noise.c,v 1.1.1.3 2002/11/19 11:49:50 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include	<string.h>

#include "libutl.h"
#include "gameheader.h"
#include "../vr/vr.h"

extern void NewSIG_3DTex(FVECTOR *pos ,int mode,int time,int code);


#define	VR_NOISE_MAX	(6)
static int	VR_NOISE_EF[VR_NOISE_MAX]	= {
	TEX_VR_NOISE_A0,
	TEX_VR_NOISE_A1,
	TEX_VR_NOISE_A2,
	TEX_VR_NOISE_B0,
	TEX_VR_NOISE_B1,
	TEX_VR_NOISE_B2
};

#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define N_PRIMS	(4)
#define N_VERTS	(1)

//extern void PosBox(FVECTOR * ,float ,SVECTOR * );

#define	VR_EF_TEX_MAX (3)
static int VR_EF_TEX[VR_EF_TEX_MAX] ={
	TEX_VR_NOISE_C0,
	TEX_VR_NOISE_C1,
	TEX_VR_NOISE_C2
};
typedef	struct _Work {
	GV_ACT_EX	actor ;
	int		time ;
	int		mode ;
	int		limit ;
	FVECTOR	pos;
	FVECTOR	shift;
	DG_PRIM2		*prim_sprt ;
} Work ;

static void Act(Work *work)
{
	FVECTOR *pos ;
#if 0
	{
		SVECTOR	rgb ;
		rgb.vx = 120;
		rgb.vy = 120;
		rgb.vz = 0;
		PosBox(&work->pos,30.0f ,&rgb);
	}
#endif
//	work->pos = *work->posbuf ;
//	GV_MatToVec( work->posbuf, &work->pos ) ;

//	work->pos.vx += work->shift.vx ;
//	work->pos.vy += work->shift.vy ;

	if(work->time  ==0){
//printf("EFE CALL!!!!\n");
//		NewSIG_3DTex(&work->pos,work->mode,60,VR_EF_TEX[((irnd()>>8)%VR_EF_TEX_MAX)] ) ;
		NewSIG_3DTex(&work->pos,work->mode,30,VR_EF_TEX[((irnd()>>8)%VR_EF_TEX_MAX)] ) ;
//		NewSIG_3DTex(&work->pos,work->mode,60,VR_NOISE_EF[((irnd()>>8)%VR_NOISE_MAX)]) ;
	}

	work->time++ ;
	if(work->limit <= work->time){
		GV_DestroyActor(work) ;
	}
}
static void Die(Work *work)
{
}
static int GetResources(Work *work,FVECTOR	*pos ,int mode,int time)
{
	work->mode = mode ;
	work->pos = *pos ;
	work->limit = time ;
	work->time = 0 ;
	return 1 ;
}
void NewSIG_VR_Noise(FVECTOR *pos ,int mode,int time)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,pos ,mode,time)){
			GV_DestroyActor(work) ;
		}
	}
}

