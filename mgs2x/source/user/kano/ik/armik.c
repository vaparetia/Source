//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	armik.c

	2000/04/06 K.Kano
	$Id: armik.c,v 1.1.1.3 2002/11/19 11:43:13 Yoshizawa1 Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <sys/types.h>

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


typedef struct {
    OBJECT *obj;
    int objnum;

    OBJECT *target;
    int tobjnum;
    FVECTOR *tpoint;

    FVECTOR abs_rots[3];
    int count;
} ARMIK_WORK;


#define FRAMES		8


void ExitArmIK(ARMIK_WORK *work)
{
    GV_Free(work);
}

ARMIK_WORK *InitArmIK(OBJECT *obj,int objnum,OBJECT *target,int tobjnum,FVECTOR *tpoint)
{
    ARMIK_WORK *work;

    if((work=(ARMIK_WORK *)GV_Malloc(sizeof(ARMIK_WORK)))==NULL) return NULL;
    GV_ZeroMemory(work,sizeof(ARMIK_WORK));

    work->obj=obj;
    work->objnum=objnum;
    work->target=target;
    work->tobjnum=tobjnum;
    work->tpoint=tpoint;

    return work;
}

void ReregistTarget(ARMIK_WORK *work,OBJECT *target,int tobjnum,FVECTOR *tpoint)
{
    if ( target != NULL ) work->target=target;
    if ( tobjnum >= 0 ) work->tobjnum=tobjnum;
    if ( tpoint != NULL ) work->tpoint=tpoint;

    work->count=0;
}

int MoveArmIK(ARMIK_WORK *work,int flag)
{
    OBJECT *obj=work->obj;
    int objnum=work->objnum;

    FVECTOR abs_rots[3];

    fpu_CopyVector(abs_rots+0,&(obj->m_ctrl->abs_rots[objnum-2]));
    fpu_CopyVector(abs_rots+1,&(obj->m_ctrl->abs_rots[objnum-1]));
    fpu_CopyVector(abs_rots+2,&(obj->m_ctrl->abs_rots[objnum-0]));

    if(flag){
	int Arm_IKcalc(OBJECT *obj,int objnum,OBJECT *target,int tobjnum,FVECTOR *tpoint);

	int ik_on=Arm_IKcalc(obj,objnum,work->target,work->tobjnum,work->tpoint);

	if(ik_on){
	    work->count++;
	    if(work->count>=FRAMES) work->count=FRAMES;
	    else{
		float interp;

		interp=(float)work->count/(float)FRAMES;

		MT_QuatSlerp(&(obj->m_ctrl->abs_rots[objnum-2]),
			     abs_rots+0,
			     &(obj->m_ctrl->abs_rots[objnum-2]),
			     interp);
		MT_QuatNormalize(&(obj->m_ctrl->abs_rots[objnum-2]),
				 &(obj->m_ctrl->abs_rots[objnum-2]));
		MT_QuatSlerp(&(obj->m_ctrl->abs_rots[objnum-1]),
			     abs_rots+1,
			     &(obj->m_ctrl->abs_rots[objnum-1]),
			     interp);
		MT_QuatNormalize(&(obj->m_ctrl->abs_rots[objnum-1]),
				 &(obj->m_ctrl->abs_rots[objnum-1]));
		MT_QuatSlerp(&(obj->m_ctrl->abs_rots[objnum-0]),
			     abs_rots+2,
			     &(obj->m_ctrl->abs_rots[objnum-0]),
			     interp);
		MT_QuatNormalize(&(obj->m_ctrl->abs_rots[objnum-0]),
				 &(obj->m_ctrl->abs_rots[objnum-0]));
	    }

	    fpu_CopyVector(work->abs_rots+0,&(obj->m_ctrl->abs_rots[objnum-2]));
	    fpu_CopyVector(work->abs_rots+1,&(obj->m_ctrl->abs_rots[objnum-1]));
	    fpu_CopyVector(work->abs_rots+2,&(obj->m_ctrl->abs_rots[objnum-0]));

	    return 1;
	}
    }

    work->count--;
    if(work->count<=0) work->count=0;
    else{
	float interp;

	interp=(float)work->count/(float)FRAMES;

	MT_QuatSlerp(&(obj->m_ctrl->abs_rots[objnum-2]),
		     abs_rots+0,
		     work->abs_rots+0,
		     interp);
	MT_QuatNormalize(&(obj->m_ctrl->abs_rots[objnum-2]),
			 &(obj->m_ctrl->abs_rots[objnum-2]));
	MT_QuatSlerp(&(obj->m_ctrl->abs_rots[objnum-1]),
		     abs_rots+1,
		     work->abs_rots+1,
		     interp);
	MT_QuatNormalize(&(obj->m_ctrl->abs_rots[objnum-1]),
			 &(obj->m_ctrl->abs_rots[objnum-1]));
	MT_QuatSlerp(&(obj->m_ctrl->abs_rots[objnum-0]),
		     abs_rots+2,
		     work->abs_rots+2,
		     interp);
	MT_QuatNormalize(&(obj->m_ctrl->abs_rots[objnum-0]),
			 &(obj->m_ctrl->abs_rots[objnum-0]));
    }

    return 0;
}
