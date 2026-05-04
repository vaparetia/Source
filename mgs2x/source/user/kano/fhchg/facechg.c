/*
	facechg.c
		partchg.cを利用した、顔の切替えキャラ

	2000/03/28 K.Kano
	$Id: facechg.c,v 1.1.1.3 2002/11/19 11:43:10 Yoshizawa1 Exp $
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
    GV_ACT	actor;

    int		address;

    PARTCHANGE_WORK *partchg;
} Work;


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->address,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);

		switch(command){
		case 0:
			/* パターン＋フレーム */
	    {
			int ptn,cnt;

			ptn=*(msg->message+1);
			cnt=*(msg->message+2);

			SetParamForPartChange(work->partchg,ptn,cnt);
	    }
	    break;

		case 1:
			/* 表示／非表示 */
	    {
			int disp_sw;
			disp_sw=*(msg->message+1);

			switch(disp_sw){
			case 0:
			case 1:
				work->partchg->disp_enable=disp_sw;
				break;
			case -1:
				work->partchg->disp_enable^=1;
				break;
			}
	    }
	    break;
		}

		msg++;
		n_msg--;
    }
    MovePartChange(work->partchg);
}

static void Die(Work *work)
{
    if(work->partchg!=NULL) ExitPartChange(work->partchg);
}


/* ------------------------------------------------------------------------ */
/* 初期化 */

static int GetResources_called(Work *work,DG_OBJS *human,
							   int face_model,int *cv2_models,int cv2_size)
{
    DG_DEF *def;
    CV2_DEF **cv2def;
    int i;

    work->partchg=NULL;

    if((def=(DG_DEF*)GV_GetCache(GV_CacheID(face_model,'k')))==NULL) return 0;

    if((cv2def=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*cv2_size))==NULL) return 0;

    for(i=0;i<cv2_size;i++){
		if((*(cv2def+i)=(CV2_DEF*)GV_GetCache(GV_CacheID(*(cv2_models+i),'c')))==NULL){
			GV_Free(cv2def);
			return 0;
		}
    }

    work->partchg=InitPartChange(human,HUMAN21_ATAMA,def,cv2def,cv2_size);

    GV_Free(cv2def);

    return (work->partchg!=NULL);
}

void *NewFaceChange_called(int name,DG_OBJS *human,int face_model,int *cv2_models,int cv2_size)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work)) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;

		work->address=name;

		if(!GetResources_called(work,human,face_model,cv2_models,cv2_size)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
