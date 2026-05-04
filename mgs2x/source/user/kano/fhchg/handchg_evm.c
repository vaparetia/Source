//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	handchg_evm.c
		partchg_evm.cを利用した、手の切替えキャラ

	2000/05/31 K.Kano
	$Id: handchg_evm.c,v 1.1.1.3 2002/11/19 11:43:10 Yoshizawa1 Exp $
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

    EVMPARTCHANGE_WORK *r_partchg;
    EVMPARTCHANGE_WORK *l_partchg;
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
			/* アニメーションセット */
		    {
				int ptn;

				ptn=*(msg->message+1);
				SetMtnForEvmPartChange(work->r_partchg,ptn);
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
					work->r_partchg->disp_enable=disp_sw;
					break;
				case -1:
					work->r_partchg->disp_enable^=1;
					break;
				}
			}
		    break;

		case 2:
			/* パターン＋フレーム */
		    {
				int ptn;

				ptn=*(msg->message+1);
				SetMtnForEvmPartChange(work->l_partchg,ptn);
			}
		    break;

		case 3:
			/* 表示／非表示 */
		    {
				int disp_sw;
				disp_sw=*(msg->message+1);

				switch(disp_sw){
				case 0:
				case 1:
					work->l_partchg->disp_enable=disp_sw;
					break;
				case -1:
					work->l_partchg->disp_enable^=1;
					break;
				}
			}
		    break;
		}

		msg++;
		n_msg--;
    }

    MoveEvmPartChange(work->r_partchg);
    MoveEvmPartChange(work->l_partchg);
}

static void Die(Work *work)
{
    if(work->r_partchg!=NULL) ExitEvmPartChange(work->r_partchg);
    if(work->l_partchg!=NULL) ExitEvmPartChange(work->l_partchg);
}


/* ------------------------------------------------------------------------ */
/* 初期化 */

static int GetResources_called(Work *work,DG_OBJS *human,int r_hand_model,int l_hand_model)
{
    EVM_DEF *def;

    work->r_partchg=NULL;
    work->l_partchg=NULL;

    if((def=(EVM_DEF*)GV_GetCache(GV_CacheID(r_hand_model,'e')))==NULL) return 0;

    work->r_partchg=InitEvmPartChange(human,HUMAN21_MIGI_TE,def);

    if((def=(EVM_DEF*)GV_GetCache(GV_CacheID(l_hand_model,'e')))==NULL) return 0;

    work->l_partchg=InitEvmPartChange(human,HUMAN21_HIDARI_TE,def);

    return (work->r_partchg!=NULL) && (work->l_partchg!=NULL);
}

void *NewEvmHandsChange_called(int name,DG_OBJS *human,int r_hand_model,int l_hand_model)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work)) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;

		work->address=name;

		if(!GetResources_called(work,human,r_hand_model,l_hand_model)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
