//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bgscr.c
		タイトル等のバックグランド画面のアニメーション

	2001/06/12 K.Kano
	$Id: late_control.c,v 1.1.1.3 2002/11/19 11:43:14 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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


typedef struct {
	GV_ACT_EX actor;
	OBJECT *object;
	int name;
	int flag;
} Work;

enum {
	LATE_CONTROL_ON=0,
	LATE_CONTROL_OFF,
	LATE_CONTROL_REV,
};

static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);

		switch(command){
		case LATE_CONTROL_ON:
			work->flag=1;
			break;
		case LATE_CONTROL_OFF:
			work->flag=0;
			break;
		case LATE_CONTROL_REV:
			work->flag^=1;
			break;
		}

		msg++;
		n_msg--;
	}

	if(work->object==NULL) return;
	if(work->object->evmobj==NULL) return;

	if(work->flag){
		work->object->evmobj->flag|=DG_EVMOBJ_LATTERDRAW;
	}
	else{
		work->object->evmobj->flag&=~DG_EVMOBJ_LATTERDRAW;
	}
}

static void Die(Work *work)
{
}

void *NewEvmLateControl(int name,OBJECT *object)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->object=object;
		work->flag=1;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);
    }

    return (void *)work ;
}
