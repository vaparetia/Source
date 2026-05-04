//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachments.c
		装備品揺らし

	1999/11/22 K.Kano
	$Id: attachments.c,v 1.1.1.3 2002/11/19 11:43:05 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif


#include "attachment.h"
#include "attachment2.h"
#include "attachment3.h"
#include "attachments.h"
#include "attachment_debug.h"

#include "../../mode/demo/libdemo.h"


/* ワーク */
typedef	struct _attachments_Work {
    GV_ACT_EX		actor ;		/* リンク */
    DG_OBJS		**objs ;

    OBJECT		*target;

    MODEL_PARAMETER	*mp;
    int attachment_size;

    MODEL_PARAMETER2	*mp2;
    int attachment2_size;

    MODEL_PARAMETER3	*mp3;
    int attachment3_size;

	int name;
	int invisible_flag;

} Work ;


#define	ATTACHMENTS_NAME	GV_StrCode( "装備品Ｓ" )
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

#define E3VIDEO_MODE


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    DG_OBJS **objs;
    int msize=work->attachment_size+work->attachment2_size+work->attachment3_size;
    int i;

	int init_flag;


    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(ATTACHMENTS_NAME,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int name=*(msg->message+1);

		if(name==work->name){
			switch(command){
			case 0:
				/* 表示／非表示 */
				{
					int disp_sw;
					disp_sw=*(msg->message+2);

					switch(disp_sw){
					case 0:
					case 1:
						work->invisible_flag=!disp_sw;
						break;
					case -1:
						work->invisible_flag^=1;
						break;
					}
				}
				break;
			}
		}

		msg++;
		n_msg--;
    }


	objs=work->objs;
	for(i=0;i<msize;i++,objs++){
		if(*objs){
			DG_VisibleObjs(*objs);
			(*objs)->group_id=work->target->objs->group_id;
		}
	}

	objs=work->objs;

	{
		int flag=0x000f;

		if(work->target->evmobj!=NULL){
			flag&=((work->target->evmobj->flag &
					(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
		}

		flag&=((work->target->objs->flag &
				(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3))>>12);
		flag<<=12;

		for(i=0;i<msize;i++,objs++){
			if(*objs){
				(*objs)->flag=((*objs)->flag &
							   ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3)) | flag;
			}
		}
	}

	if(work->invisible_flag){
		objs=work->objs;
		for(i=0;i<msize;i++,objs++){
			if(*objs) (*objs)->flag|=(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3);
		}
	}

	init_flag=(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT));

	objs=work->objs;
	for(i=0;i<work->attachment_size;i++,objs++){
		if(*objs==NULL) continue;

		if((work->mp+i)->first_flag || init_flag){
			InitCalcAttachment(work->mp+i);
			(work->mp+i)->first_flag=0;
		}

		MoveAttachment(*objs,work->mp+i);
	}
	for(i=0;i<work->attachment2_size;i++,objs++){
		if(*objs==NULL) continue;

		if((work->mp2+i)->first_flag){
			(work->mp2+i)->first_flag=0;
		}

		MoveAttachment2(*objs,work->mp2+i);
	}
	for(i=0;i<work->attachment3_size;i++,objs++){
		if(*objs==NULL) continue;

		if((work->mp3+i)->first_flag || init_flag){
			InitCalcAttachment3(work->mp3+i);
			(work->mp3+i)->first_flag=0;
		}

		MoveAttachment3(*objs,work->mp3+i);
	}
}

static void Die(Work *work)
{
    if(work->objs!=NULL){
		int msize=work->attachment_size+work->attachment2_size+work->attachment3_size;
		int i;

		for(i=0;i<msize;i++){
			if(*(work->objs+i)!=NULL){
				DG_DequeueObjs(*(work->objs+i));
				DG_FreeObjs(*(work->objs+i));
			}
		}
		GV_Free(work->objs);
    }
    if(work->mp!=NULL) GV_Free(work->mp);
    if(work->mp2!=NULL) GV_Free(work->mp2);
    if(work->mp3!=NULL){
		int i;
		for(i=0;i<work->attachment3_size;i++){
			ExitAttachment3(work->mp3+i);
		}
		GV_Free(work->mp3);
    }
}

/* ------------------------------------------------------------------------ */

FVECTOR *GetNowQFromAttachment2(void *param,int index)
{
	Work *work=(Work *)param;

	if(work->mp2==NULL) return NULL;
	if(index>=work->attachment2_size) return NULL;
	return &((work->mp2+index)->now_q);
}

/* ------------------------------------------------------------------------ */
/* 初期化部 */

static int InitObject(DG_OBJS **objs,int name)
{
    DG_DEF *def ;

    def = (DG_DEF*) GV_GetCache( GV_CacheID( name, 'k' ) );
    if((*objs = DG_MakeObjs( def, OBJECT_FLAG, 0 ))==NULL) return 0;
    DG_QueueObjs(*objs);
    DG_InvisibleObjs(*objs);

    return 1;
}

static int GetResources_called(Work *work,OBJECT *target,const ATTACHMENT_ARGUMENT *arg,int size,
							   const ATTACHMENT_ARGUMENT2 *arg2,int size2,
							   const ATTACHMENT_ARGUMENT3 *arg3,int size3)
{
    DG_OBJS **objs;
    int msize=size+size2+size3;
    int i;

    work->objs=NULL;
    work->target=target;
    work->mp=NULL;
    work->attachment_size=0;
    work->mp2=NULL;
    work->attachment2_size=0;
    work->mp3=NULL;
    work->attachment3_size=0;

    if(target==NULL) return 0;

	work->name=((CONTROL *)(target-1))->name;

    if((work->objs=(DG_OBJS **)GV_Malloc(sizeof(DG_OBJS *)*msize))==NULL) return 0;
    GV_ZeroMemory(work->objs,sizeof(DG_OBJS *)*msize);

    work->attachment_size=size;
    work->attachment2_size=size2;
    work->attachment3_size=size3;

    if(size>0){
		if((work->mp
			=(MODEL_PARAMETER *)GV_Malloc(sizeof(MODEL_PARAMETER)*size))==NULL) return 0;
    }
    if(size2>0){
		if((work->mp2
			=(MODEL_PARAMETER2 *)GV_Malloc(sizeof(MODEL_PARAMETER2)*size2))==NULL) return 0;
    }
    if(size3>0){
		if((work->mp3
			=(MODEL_PARAMETER3 *)GV_Malloc(sizeof(MODEL_PARAMETER3)*size3))==NULL) return 0;
    }

    objs=work->objs;
    for(i=0;i<size;i++,objs++){
		if(!InitObject(objs,(arg+i)->model_name)) continue;
		if(!InitAttachment(*objs,work->mp+i)) continue;
		Attachment_GetOptionValue_called(work->mp+i,*objs,(arg+i)->v,(arg+i)->r,
										 target,(arg+i)->objnum,
										 (arg+i)->x,(arg+i)->a,(arg+i)->angle_limit,
										 (arg+i)->oval_param);
    }
    for(i=0;i<size2;i++,objs++){
		if(!InitObject(objs,(arg2+i)->model_name)) continue;
		if(!InitAttachment2(*objs,work->mp2+i)) continue;
		Attachment2_GetOptionValue_called(work->mp2+i,*objs,(arg2+i)->v,(arg2+i)->r,target,
										  (arg2+i)->objnum,(arg2+i)->x,
										  (arg2+i)->objnum2,(arg2+i)->p);
    }
    for(i=0;i<size3;i++,objs++){
		if(!InitObject(objs,(arg3+i)->model_name)) continue;
		Attachment3_GetOptionValue_called(work->mp3+i,*objs,(arg3+i)->r,target,
										  (arg3+i)->objnum,(arg3+i)->x,(arg3+i)->frames);
		if(!InitAttachment3(work->mp3+i)) continue;
    }

	if(work->target!=NULL){
		work->name=((CONTROL *)(work->target)-1)->name;
	}

    return 1;
}

void *NewAttachments_called(OBJECT *target,const ATTACHMENT_ARGUMENT *arg,int size,
							const ATTACHMENT_ARGUMENT2 *arg2,int size2,
							const ATTACHMENT_ARGUMENT3 *arg3,int size3)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work)) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		work->name=0;
		work->invisible_flag=0;

		if(!GetResources_called(work,target,arg,size,arg2,size2,arg3,size3)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
