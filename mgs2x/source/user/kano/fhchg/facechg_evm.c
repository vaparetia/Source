/*
	facechg_evm.c
		partchg_evm.cを利用した、顔の切替えキャラ

	2000/05/31 K.Kano
	$Id: facechg_evm.c,v 1.1.1.3 2002/11/19 11:43:10 Yoshizawa1 Exp $
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

    EVMPARTCHANGE_WORK *partchg;
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
				int ptn;

				ptn=*(msg->message+1);
				SetMtnForEvmPartChange(work->partchg,ptn);
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

    MoveEvmPartChange(work->partchg);
}

static void Die(Work *work)
{
    if(work->partchg!=NULL) ExitEvmPartChange(work->partchg);
}


/* ------------------------------------------------------------------------ */
/* 初期化 */

static int GetResources_called(Work *work,DG_OBJS *human,int face_model)
{
    EVM_DEF *def;

    work->partchg=NULL;

    if((def=(EVM_DEF*)GV_GetCache(GV_CacheID(face_model,'e')))==NULL) return 0;

    work->partchg=InitEvmPartChange(human,HUMAN21_ATAMA,def);

    return (work->partchg!=NULL);
}

void *NewFaceChange_called(int name,DG_OBJS *human,int face_model)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work)) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;

		work->address=name;

		if(!GetResources_called(work,human,face_model)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
