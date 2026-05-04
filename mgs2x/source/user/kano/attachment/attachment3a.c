//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachment3a.c
		装備品揺らし

	1999/11/05 K.Kano
	$Id: attachment3a.c,v 1.1.1.3 2002/11/19 11:43:04 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif


#include "attachment.h"
#include "attachment2.h"
#include "attachment3.h"
#include "attachment4.h"
#include "attachmentf.h"
#include "attachment_debug.h"

#include "../../mode/demo/libdemo.h"


typedef union {
    MODEL_PARAMETER	mp;
    MODEL_PARAMETER2	mp2;
    MODEL_PARAMETER3	mp3;
    MODEL_PARAMETER4	mp4;
    MODEL_PARAMETERF	mpf;
} MODEL_PARAMETERS;

typedef struct {
	int func_type;
	int delay_count;
} MPS_PARAMS;


/* ワーク */
typedef	struct _Work {
    GV_ACT		actor ;		/* リンク */

    DG_OBJS		**objs ;
    MODEL_PARAMETERS	*mps;
    unsigned int	*ids;

	MPS_PARAMS *mps_params;

	unsigned char *visible_flag;

    int nowid;
    int nowp;
    int size;
} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)



static Work *allocated_work=NULL ;

static void ExitObject(Work *work,int index);

void FallAttachment_called(int id,int func_type,int delay_count,int rnd_delay_count);


/* okajima */
void *NewBlood_M9( FMATRIX *world, FVECTOR *mov, FVECTOR *norm );


/* ------------------------------------------------------------------------ */
/* メイン処理 */
static void Act(Work *work)
{
    int size=work->size;
    int i;
	int init_flag;

	init_flag=(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT));

	for(i=0;i<size;i++){
		if(*(work->objs+i)!=NULL){
			switch(*(work->ids+i)>>24){
			case 0x01:
				DG_VisibleObjs(*(work->objs+i));

				if((work->mps+i)->mp.target!=NULL){
					int flag=0x000f;

					(*(work->objs+i))->group_id=(work->mps+i)->mp.target->objs->group_id;

					if(!(*(work->visible_flag+i))){
						if((work->mps+i)->mp.target->evmobj!=NULL){
							flag&=(((work->mps+i)->mp.target->evmobj->flag &
									(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
						}

						flag&=(((work->mps+i)->mp.target->objs->flag &
								(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3))>>12);
						flag<<=12;

						(*(work->objs+i))->flag=((*(work->objs+i))->flag &
												 ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|
												   DG_FLAG_INVISIBLE3)) | flag;
					}
				}

				if((work->mps+i)->mp.first_flag || init_flag){
					InitCalcAttachment(&((work->mps+i)->mp));
					(work->mps+i)->mp.first_flag=0;
				}

				MoveAttachment(*(work->objs+i),&((work->mps+i)->mp));

				break;
			case 0x02:
				DG_VisibleObjs(*(work->objs+i));

				if((work->mps+i)->mp2.target!=NULL){
					int flag=0x000f;

					(*(work->objs+i))->group_id=(work->mps+i)->mp2.target->objs->group_id;

					if(!(*(work->visible_flag+i))){
						if((work->mps+i)->mp2.target->evmobj!=NULL){
							flag&=(((work->mps+i)->mp2.target->evmobj->flag &
									(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
						}

						flag&=(((work->mps+i)->mp2.target->objs->flag &
								(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3))>>12);
						flag<<=12;

						(*(work->objs+i))->flag=((*(work->objs+i))->flag &
												 ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|
												   DG_FLAG_INVISIBLE3)) | flag;
					}
				}

				(work->mps+i)->mp2.first_flag=0;

				MoveAttachment2(*(work->objs+i),&((work->mps+i)->mp2));

				break;
			case 0x03:
				DG_VisibleObjs(*(work->objs+i));

				if((work->mps+i)->mp3.target!=NULL){
					int flag=0x000f;

					(*(work->objs+i))->group_id=(work->mps+i)->mp3.target->objs->group_id;

					if(!(*(work->visible_flag+i))){
						if((work->mps+i)->mp3.target->evmobj!=NULL){
							flag&=(((work->mps+i)->mp3.target->evmobj->flag &
									(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
						}

						flag&=(((work->mps+i)->mp3.target->objs->flag &
								(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3))>>12);
						flag<<=12;

						(*(work->objs+i))->flag=((*(work->objs+i))->flag &
												 ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|
												   DG_FLAG_INVISIBLE3)) | flag;
					}
				}

				if((work->mps+i)->mp3.first_flag || init_flag){
					InitCalcAttachment3(&((work->mps+i)->mp3));
					(work->mps+i)->mp3.first_flag=0;
				}

				MoveAttachment3(*(work->objs+i),&((work->mps+i)->mp3));

				break;
			case 0x13:
				DG_VisibleObjs(*(work->objs+i));

				if((work->mps+i)->mp3.target!=NULL){
					int flag=0x000f;

					(*(work->objs+i))->group_id=(work->mps+i)->mp3.target->objs->group_id;

					if(!(*(work->visible_flag+i))){
						if((work->mps+i)->mp3.target->evmobj!=NULL){
							flag&=(((work->mps+i)->mp3.target->evmobj->flag &
									(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
						}

						flag&=(((work->mps+i)->mp3.target->objs->flag &
								(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3))>>12);
						flag<<=12;

						(*(work->objs+i))->flag=((*(work->objs+i))->flag &
												 ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|
												   DG_FLAG_INVISIBLE3)) | flag;
					}
				}

				if((work->mps+i)->mp3.first_flag || init_flag){
					InitCalcAttachment3(&((work->mps+i)->mp3));
					(work->mps+i)->mp3.first_flag=0;
				}

				MoveAttachment3A(*(work->objs+i),&((work->mps+i)->mp3));

				break;
			case 0x04:
				DG_VisibleObjs(*(work->objs+i));

				if((work->mps+i)->mp4.target!=NULL){
					int flag=0x000f;

					(*(work->objs+i))->group_id=(work->mps+i)->mp4.target->objs->group_id;

					if(!(*(work->visible_flag+i))){
						if((work->mps+i)->mp4.target->evmobj!=NULL){
							flag&=(((work->mps+i)->mp4.target->evmobj->flag &
									(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
						}

						flag&=(((work->mps+i)->mp4.target->objs->flag &
								(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3))>>12);
						flag<<=12;

						(*(work->objs+i))->flag=((*(work->objs+i))->flag &
												 ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|
												   DG_FLAG_INVISIBLE3)) | flag;
					}
				}

				if((work->mps+i)->mp4.first_flag || init_flag){
					InitCalcAttachment4(&((work->mps+i)->mp4));
					(work->mps+i)->mp4.first_flag=0;
				}

				MoveAttachment4(*(work->objs+i),&((work->mps+i)->mp4));

				if((work->mps_params+i)->delay_count>0){
					(work->mps_params+i)->delay_count--;
				}
				if((work->mps_params+i)->delay_count==0){
					FVECTOR *base;

					if((work->mps+i)->mp4.target!=NULL){
						base=(FVECTOR *)&((work->mps+i)->mp4.target->objs->world.m[3][0]);
					}
					else{
						base=(FVECTOR *)&((*(work->objs+i))->world.m[3][0]);
					}

					switch((work->mps_params+i)->func_type){
					case 1:
						NewBlood_M9((work->mps+i)->mp4.tmat,
									(work->mps+i)->mp4.x,
									(FVECTOR *)&((work->mps+i)->mp4.rotm.m[2][0]));
						break;
					}

					ExitAttachment4(&((work->mps+i)->mp4));

					*(work->ids+i)&=((1<<24)-1);
					*(work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(work->objs+i),
									   &((work->mps+i)->mpf),base);

					(work->mps_params+i)->func_type=0;
					(work->mps_params+i)->delay_count=-1;
				}
				break;

			case 0x0f:
				if(!MoveAttachmentFall(*(work->objs+i),&((work->mps+i)->mpf))){
					ExitObject(work,i);
					*(work->ids+i)=0;
				}
				break;
			}
		}
	}
}

static void Die(Work *work)
{
    int i;
    for(i=0;i<work->size;i++){
		if(*(work->objs+i)!=NULL ){
			DG_DequeueObjs(*(work->objs+i)) ;
			DG_FreeObjs(*(work->objs+i));
			*(work->objs+i)=NULL;

			switch(*(work->ids+i)>>24){
			case 0x01:
			case 0x02:
			case 0x0f:
				*(work->ids+i)=0;
				break;
			case 0x03:
			case 0x13:
				ExitAttachment3(&((allocated_work->mps+i)->mp3));
				*(work->ids+i)=0;
				break;
			case 0x04:
				ExitAttachment4(&((allocated_work->mps+i)->mp4));
				*(work->ids+i)=0;
				break;
			}
		}
    }
    GV_Free(work->mps);
    allocated_work=NULL;
}

/* ------------------------------------------------------------------------ */
/* 初期化部 */

static void GetOptionValue(Work *work)
{
    int msize;

    work->objs=NULL;
    work->ids=NULL;

    work->size=40;
    work->nowp=0;
    work->nowid=0;

    /* ワークのサイズを決定 */
    if( GCL_GetOption( 's' ) != NULL ){
		work->size=GCL_GetNextInt();
    }

    msize=(sizeof(MODEL_PARAMETERS)+sizeof(DG_OBJS *)+sizeof(int)+sizeof(MPS_PARAMS)+
		   sizeof(unsigned char))*work->size;

    work->mps=(MODEL_PARAMETERS *)GV_Malloc(msize);
    if(work->mps!=NULL){
		GV_ZeroMemory(work->mps,msize);
		work->objs=(DG_OBJS **)(work->mps+work->size);
		work->ids=(int *)(work->objs+work->size);
		work->mps_params=(MPS_PARAMS *)(work->ids+work->size);
		work->visible_flag=(unsigned char *)(work->mps_params+work->size);
    }
}

/* 初期化部メイン */
void *NewAttachment_Server(void)
{
    if(allocated_work!=NULL) return NULL;

    allocated_work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(allocated_work!=NULL) {
		GV_SetActor(&(allocated_work->actor),Act,Die) ;
		GetOptionValue(allocated_work);
    }
    return (void *)allocated_work ;
}


/* ----------------------------------------------------------------------- */
/* マネージャーを駆動するインターフェイス部                                */

/* オブジェクトを初期化 */
static int InitObject(Work *work,int name)
{
    DG_DEF *def ;
    int index=work->nowp;

    def = (DG_DEF*) GV_GetCache( GV_CacheID( name, 'k' ) );
    if((*(work->objs+index) = DG_MakeObjs( def, OBJECT_FLAG, 0 ))==NULL) return 0;
    DG_QueueObjs( *(work->objs+index) );
	DG_InvisibleObjs( *(work->objs+index) );

    return 1;
}

static void ExitObject(Work *work,int index)
{
    if(*(work->objs+index)!=NULL){
		DG_DequeueObjs(*(work->objs+index)) ;
		DG_FreeObjs(*(work->objs+index));
		*(work->objs+index)=NULL;
    }
}

static void SetIR(Work *work,int index,int onoff)
{
	DG_OBJS	*objs ;
	int		i ;
    if(*(work->objs+index)!=NULL){
		objs = *(work->objs+index);
		if(onoff){
			//(*(work->objs+index))->flag|=DG_FLAG_IRREACTION;
			objs->flag |= DG_FLAG_IRREACTION ;
			for ( i = 0 ; i < objs->n_models ; i++ ){
				objs->objs[i].flag |= DG_FLAG_IRREACTION ;
			}
		} else {
			//(*(work->objs+index))->flag&=~DG_FLAG_IRREACTION;
			objs->flag &= ~DG_FLAG_IRREACTION ;
			for ( i = 0 ; i < objs->n_models ; i++ ){
				objs->objs[i].flag &= ~DG_FLAG_IRREACTION ;
			}
		}
    }
}

static void KillAttachment(Work *work)
{
    int index=work->nowp;

    switch(*(work->ids+index)>>24){
    case 0x01:
    case 0x02:
    case 0x0f:
		ExitObject(work,work->nowp);
		*(work->ids+index)=0;
		(work->mps_params+index)->func_type=0;
		(work->mps_params+index)->delay_count=-1;
		break;
    case 0x03:
    case 0x13:
		ExitObject(work,index);
		ExitAttachment3(&((work->mps+index)->mp3));
		*(work->ids+index)=0;
		(work->mps_params+index)->func_type=0;
		(work->mps_params+index)->delay_count=-1;
		break;
    case 0x04:
		ExitObject(work,index);
		ExitAttachment4(&((work->mps+index)->mp4));
		*(work->ids+index)=0;
		(work->mps_params+index)->func_type=0;
		(work->mps_params+index)->delay_count=-1;
		break;
    }
}

DG_OBJS	*GetAttachmentDGobjs(int id)
{
	Work	*work ;
	
    if(allocated_work!=NULL){
		int index=id & 0xffff;
		if(*(allocated_work->ids+index)==id){
			work = allocated_work ;
		    if(*(work->objs+index)!=NULL){
				return *(work->objs+index) ;
			}
		}
	}
	return NULL ;
}


void KillAttachment_called(int id)
{
    if(allocated_work!=NULL){
		int index=id & 0xffff;
		if(*(allocated_work->ids+index)==id){
			switch(*(allocated_work->ids+index)>>24){
			case 0x01:
			case 0x02:
			case 0x0f:
				ExitObject(allocated_work,index);
				*(allocated_work->ids+index)=0;
				break;
			case 0x03:
			case 0x13:
				ExitObject(allocated_work,index);
				ExitAttachment3(&((allocated_work->mps+index)->mp3));
				*(allocated_work->ids+index)=0;
				break;
			case 0x04:
				ExitObject(allocated_work,index);
				ExitAttachment4(&((allocated_work->mps+index)->mp4));
				*(allocated_work->ids+index)=0;
				break;
			}
		}
    }
}


void SearchAndKillAttachment_called2(int type,OBJECT *obj)
{
    if(allocated_work!=NULL){
		int i;

		for(i=0;i<allocated_work->size;i++){

			if(type>0){
				if((*(allocated_work->ids+i)>>24)!=type) continue;
			}

			switch(*(allocated_work->ids+i)>>24){
			case 0x01:
				if((allocated_work->mps+i)->mp.target==obj){
					ExitObject(allocated_work,i);
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			case 0x02:
				if((allocated_work->mps+i)->mp2.target==obj){
					ExitObject(allocated_work,i);
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			case 0x03:
			case 0x13:
				if((allocated_work->mps+i)->mp3.target==obj){
					ExitObject(allocated_work,i);
					ExitAttachment3(&((allocated_work->mps+i)->mp3));
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			case 0x04:
				if((allocated_work->mps+i)->mp4.target==obj){
					ExitObject(allocated_work,i);
					ExitAttachment4(&((allocated_work->mps+i)->mp4));
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			}
		}
    }
}

void SearchAndKillAttachment_called(OBJECT *obj)
{

#if 0
    if(allocated_work!=NULL){
		int i;

		for(i=0;i<allocated_work->size;i++){
			switch(*(allocated_work->ids+i)>>24){
			case 0x01:
				if((allocated_work->mps+i)->mp.target==obj){
					ExitObject(allocated_work,i);
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			case 0x02:
				if((allocated_work->mps+i)->mp2.target==obj){
					ExitObject(allocated_work,i);
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			case 0x03:
			case 0x13:
				if((allocated_work->mps+i)->mp3.target==obj){
					ExitObject(allocated_work,i);
					ExitAttachment3(&((allocated_work->mps+i)->mp3));
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			case 0x04:
				if((allocated_work->mps+i)->mp4.target==obj){
					ExitObject(allocated_work,i);
					ExitAttachment4(&((allocated_work->mps+i)->mp4));
					*(allocated_work->ids+i)=0;
					(allocated_work->mps_params+i)->func_type=0;
					(allocated_work->mps_params+i)->delay_count=-1;
				}
				break;
			}
		}
    }
#else
	SearchAndKillAttachment_called2(-1,obj);
#endif

}


#if 1

void FallAttachment_called(int id,int func_type,int delay_count,int rnd_delay_count)
{
    if(allocated_work!=NULL){
		int index=id & 0xffff;
		if(*(allocated_work->ids+index)==id){
			FVECTOR *base;

			switch(*(allocated_work->ids+index)>>24){
			case 0x01:
				if((allocated_work->mps+index)->mp.target!=NULL){
					base=(FVECTOR *)&((allocated_work->mps+index)->mp.target->objs->world.m[3][0]);
				}
				else{
					base=(FVECTOR *)&((*(allocated_work->objs+index))->world.m[3][0]);
				}
				*(allocated_work->ids+index)&=((1<<24)-1);
				*(allocated_work->ids+index)|=(0x0f<<24);
				InitAttachmentFall(*(allocated_work->objs+index),
								   &((allocated_work->mps+index)->mpf),base);
				break;
			case 0x02:
				if((allocated_work->mps+index)->mp2.target!=NULL){
					base=(FVECTOR *)&((allocated_work->mps+index)->mp2.target->objs->world.m[3][0]);
				}
				else{
					base=(FVECTOR *)&((*(allocated_work->objs+index))->world.m[3][0]);
				}
				*(allocated_work->ids+index)&=((1<<24)-1);
				*(allocated_work->ids+index)|=(0x0f<<24);
				InitAttachmentFall(*(allocated_work->objs+index),
								   &((allocated_work->mps+index)->mpf),base);
				break;
			case 0x03:
			case 0x13:
				if((allocated_work->mps+index)->mp3.target!=NULL){
					base=(FVECTOR *)&((allocated_work->mps+index)->mp3.target->objs->world.m[3][0]);
				}
				else{
					base=(FVECTOR *)&((*(allocated_work->objs+index))->world.m[3][0]);
				}

				ExitAttachment3(&((allocated_work->mps+index)->mp3));

				*(allocated_work->ids+index)&=((1<<24)-1);
				*(allocated_work->ids+index)|=(0x0f<<24);
				InitAttachmentFall(*(allocated_work->objs+index),
								   &((allocated_work->mps+index)->mpf),base);
				break;
			case 0x04:
#if 1
				(allocated_work->mps_params+index)->func_type=func_type;
				(allocated_work->mps_params+index)->delay_count
					=delay_count+(int)((float)rnd_delay_count*rnd());
#else
				if((allocated_work->mps+index)->mp4.target!=NULL){
					base=(FVECTOR *)&((allocated_work->mps+index)->mp4.target->objs->world.m[3][0]);
				}
				else{
					base=(FVECTOR *)&((*(allocated_work->objs+index))->world.m[3][0]);
				}

				switch(func_type){
				case 1:
					NewBlood_M9((allocated_work->mps+index)->mp4.tmat,
								(allocated_work->mps+index)->mp4.x,
								(FVECTOR *)&((allocated_work->mps+index)->mp4.rotm.m[2][0]));
					break;
				}

				ExitAttachment4(&((allocated_work->mps+index)->mp4));

				*(allocated_work->ids+index)&=((1<<24)-1);
				*(allocated_work->ids+index)|=(0x0f<<24);
				InitAttachmentFall(*(allocated_work->objs+index),
								   &((allocated_work->mps+index)->mpf),base);
#endif
				break;
			}
		}
    }
}

void SearchAndFallAttachment_called2(int type,OBJECT *obj,int func_type,
									 int delay_count,int rnd_delay_count)
{
    if(allocated_work!=NULL){
		int i;

		for(i=0;i<allocated_work->size;i++){

			if(type>0){
				if((*(allocated_work->ids+i)>>24)!=type) continue;
			}

			switch(*(allocated_work->ids+i)>>24){
			case 0x01:
				if((allocated_work->mps+i)->mp.target==obj){
					FVECTOR *base;

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
				}
				break;
			case 0x02:
				if((allocated_work->mps+i)->mp2.target==obj){
					FVECTOR *base;

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
				}
				break;
			case 0x03:
			case 0x13:
				if((allocated_work->mps+i)->mp3.target==obj){
					FVECTOR *base;

					ExitAttachment3(&((allocated_work->mps+i)->mp3));

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
				}
				break;
			case 0x04:
				if((allocated_work->mps+i)->mp4.target==obj){

#if 1
					(allocated_work->mps_params+i)->func_type=func_type;
					(allocated_work->mps_params+i)->delay_count
						=delay_count+(int)((float)rnd_delay_count*rnd());
#else
					FVECTOR *base;

					switch(func_type){
					case 1:
						NewBlood_M9((allocated_work->mps+i)->mp4.tmat,
									(allocated_work->mps+i)->mp4.x,
									(FVECTOR *)&((allocated_work->mps+i)->mp4.rotm.m[2][0]));
						break;
					}

					ExitAttachment4(&((allocated_work->mps+i)->mp4));

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
#endif

				}
				break;
			}
		}
    }
}

void SearchAndFallAttachment_called(OBJECT *obj,int func_type,int delay_count,int rnd_delay_count)
{

#if 0
    if(allocated_work!=NULL){
		int i;

		for(i=0;i<allocated_work->size;i++){
			switch(*(allocated_work->ids+i)>>24){
			case 0x01:
				if((allocated_work->mps+i)->mp.target==obj){
					FVECTOR *base;

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
				}
				break;
			case 0x02:
				if((allocated_work->mps+i)->mp2.target==obj){
					FVECTOR *base;

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
				}
				break;
			case 0x03:
			case 0x13:
				if((allocated_work->mps+i)->mp3.target==obj){
					FVECTOR *base;

					ExitAttachment3(&((allocated_work->mps+i)->mp3));

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
				}
				break;
			case 0x04:
				if((allocated_work->mps+i)->mp4.target==obj){

#if 1
					(allocated_work->mps_params+i)->func_type=func_type;
					(allocated_work->mps_params+i)->delay_count
						=delay_count+(int)((float)rnd_delay_count*rnd());
#else
					FVECTOR *base;

					switch(func_type){
					case 1:
						NewBlood_M9((allocated_work->mps+i)->mp4.tmat,
									(allocated_work->mps+i)->mp4.x,
									(FVECTOR *)&((allocated_work->mps+i)->mp4.rotm.m[2][0]));
						break;
					}

					ExitAttachment4(&((allocated_work->mps+i)->mp4));

					base=(FVECTOR *)&(obj->objs->world.m[3][0]);
					*(allocated_work->ids+i)&=((1<<24)-1);
					*(allocated_work->ids+i)|=(0x0f<<24);
					InitAttachmentFall(*(allocated_work->objs+i),
									   &((allocated_work->mps+i)->mpf),base);
#endif

				}
				break;
			}
		}
    }
#else
	SearchAndFallAttachment_called2(-1,obj,func_type,delay_count,rnd_delay_count);
#endif

}

#endif


void TurnOnOff_IR_called(int id,int onoff)
{
    if(allocated_work!=NULL){
		int index=id & 0xffff;
		if(*(allocated_work->ids+index)==id){
			switch(*(allocated_work->ids+index)>>24){
			case 0x01:
			case 0x02:
			case 0x0f:
			case 0x03:
			case 0x13:
			case 0x04:
				SetIR(allocated_work,index,onoff);
				break;
			}
		}
    }
}

void SearchAndTurnOnOff_IR_called(OBJECT *obj,int onoff)
{
    if(allocated_work!=NULL){
		int i;

		for(i=0;i<allocated_work->size;i++){
			switch(*(allocated_work->ids+i)>>24){
			case 0x01:
				if((allocated_work->mps+i)->mp.target==obj){
					SetIR(allocated_work,i,onoff);
				}
				break;
			case 0x02:
				if((allocated_work->mps+i)->mp2.target==obj){
					SetIR(allocated_work,i,onoff);
				}
				break;
			case 0x03:
			case 0x13:
				if((allocated_work->mps+i)->mp3.target==obj){
					SetIR(allocated_work,i,onoff);
				}
				break;
			case 0x04:
				if((allocated_work->mps+i)->mp4.target==obj){
					SetIR(allocated_work,i,onoff);
				}
				break;
			}
		}
    }
}


static int GetResources_called(Work *work,
							   int model_name,FVECTOR *v,SVECTOR *r,
							   OBJECT *target,int objnum,FVECTOR *x,
							   int *a,int angle_limit,float oval_param,int visible_flag)
{
    int ans;

    KillAttachment(work);
    if(!InitObject( work,model_name )) return -1;
    if(!InitAttachment(*(work->objs+work->nowp),&((work->mps+work->nowp)->mp))){
		ExitObject(work,work->nowp);
		return -1;
    }
    Attachment_GetOptionValue_called(&((work->mps+work->nowp)->mp),*(work->objs+work->nowp),
									 v,r,target,objnum,
									 x,a,angle_limit,oval_param);

    ans=*(work->ids+work->nowp)=(0x01<<24) | work->nowp | (work->nowid & 0x00ff0000);
	(work->mps_params+work->nowp)->func_type=0;
	(work->mps_params+work->nowp)->delay_count=-1;
	*(work->visible_flag+work->nowp)=visible_flag;

    work->nowp++;
    if(work->nowp>=work->size){
		work->nowp=0;
		work->nowid+=0x10000;
    }

    return ans;
}

/* 初期化部メイン */
int MakeAttachment_called(int model_name,FVECTOR *v,SVECTOR *r,
						  OBJECT *target,int objnum,FVECTOR *x,
						  int *a,int angle_limit,float oval_param)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources_called(allocated_work,model_name,v,r,target,objnum,x,
								a,angle_limit,oval_param,0);
    }
    return ans;
}

int MakeAttachmentV_called(int model_name,FVECTOR *v,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,
						   int *a,int angle_limit,float oval_param)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources_called(allocated_work,model_name,v,r,target,objnum,x,
								a,angle_limit,oval_param,1);
    }
    return ans;
}

static int GetResources2_called(Work *work,
								int model_name,FVECTOR *v,SVECTOR *r,
								OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p,
								int visible_flag)
{
    int ans;

    KillAttachment(work);
    if(!InitObject( work,model_name )) return -1;
    if(!InitAttachment2(*(work->objs+work->nowp),&((work->mps+work->nowp)->mp2))){
		ExitObject(work,work->nowp);
		return -1;
    }
    Attachment2_GetOptionValue_called(&((work->mps+work->nowp)->mp2),*(work->objs+work->nowp),
									  v,r,target,objnum,x,objnum2,p);

    ans=*(work->ids+work->nowp)=(0x02<<24) | work->nowp | (work->nowid & 0x00ff0000);
	(work->mps_params+work->nowp)->func_type=0;
	(work->mps_params+work->nowp)->delay_count=-1;
	*(work->visible_flag+work->nowp)=visible_flag;

    work->nowp++;
    if(work->nowp>=work->size){
		work->nowp=0;
		work->nowid+=0x10000;
    }

    return ans;
}

int MakeAttachment2_called(int model_name,FVECTOR *v,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources2_called(allocated_work,model_name,v,r,target,objnum,x,objnum2,p,0);
    }
    return ans;
}

int MakeAttachment2V_called(int model_name,FVECTOR *v,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources2_called(allocated_work,model_name,v,r,target,objnum,x,objnum2,p,1);
    }
    return ans;
}


/* 資源を獲得 */
static int GetResources3_called(Work *work,
								int model_name,SVECTOR *r,
								OBJECT *target,int objnum,FVECTOR *x,int frames,int visible_flag)
{
    int ans;

    KillAttachment(work);
    if(!InitObject(work,model_name)) return -1;

    Attachment3_GetOptionValue_called(&((work->mps+work->nowp)->mp3),*(work->objs+work->nowp),
									  r,target,objnum,x,frames);
    if(!InitAttachment3(&((work->mps+work->nowp)->mp3))){
		ExitObject(work,work->nowp);
		return -1;
    }

    ans=*(work->ids+work->nowp)=(0x03<<24) | work->nowp | (work->nowid & 0x00ff0000);
	(work->mps_params+work->nowp)->func_type=0;
	(work->mps_params+work->nowp)->delay_count=-1;
	*(work->visible_flag+work->nowp)=visible_flag;

    work->nowp++;
    if(work->nowp>=work->size){
		work->nowp=0;
		work->nowid+=0x10000;
    }
    return ans;
}

/* 初期化部メイン */
int MakeAttachment3_called(int model_name,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,int frames)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources3_called(allocated_work,model_name,r,target,objnum,x,frames,0);
    }
    return ans;
}

int MakeAttachment3V_called(int model_name,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int frames)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources3_called(allocated_work,model_name,r,target,objnum,x,frames,1);
    }
    return ans;
}

/* 資源を獲得 */
static int GetResources3A_called(Work *work,
								 int model_name,SVECTOR *r,
								 OBJECT *target,int objnum,FVECTOR *x,int frames,int visible_flag)
{
    int ans;

    KillAttachment(work);
    if(!InitObject(work,model_name)) return -1;

    Attachment3_GetOptionValue_called(&((work->mps+work->nowp)->mp3),*(work->objs+work->nowp),
									  r,target,objnum,x,frames);
    if(!InitAttachment3(&((work->mps+work->nowp)->mp3))){
		ExitObject(work,work->nowp);
		return -1;
    }

    ans=*(work->ids+work->nowp)=(0x13<<24) | work->nowp | (work->nowid & 0x00ff0000);
	(work->mps_params+work->nowp)->func_type=0;
	(work->mps_params+work->nowp)->delay_count=-1;
	*(work->visible_flag+work->nowp)=visible_flag;

    work->nowp++;
    if(work->nowp>=work->size){
		work->nowp=0;
		work->nowid+=0x10000;
    }
    return ans;
}

/* 初期化部メイン */
int MakeAttachment3A_called(int model_name,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int frames)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources3A_called(allocated_work,model_name,r,target,objnum,x,frames,0);
    }
    return ans;
}

int MakeAttachment3AV_called(int model_name,SVECTOR *r,
							 OBJECT *target,int objnum,FVECTOR *x,int frames)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources3A_called(allocated_work,model_name,r,target,objnum,x,frames,1);
    }
    return ans;
}

/* 資源を獲得 */
static int GetResources4_called(Work *work,
								int model_name,FVECTOR *v,SVECTOR *r,
								OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames,
								int visible_flag)
{
    int ans;

    KillAttachment(work);
    if(!InitObject( work,model_name )) return -1;

    InitAttachment4(*(work->objs+work->nowp),&((work->mps+work->nowp)->mp4));
    Attachment4_GetOptionValue_called(&((work->mps+work->nowp)->mp4),*(work->objs+work->nowp),
									  v,r,target,objnum,x,angle_limit,frames);
    if(!Attachment4_GetStock(&((work->mps+work->nowp)->mp4))){
		ExitObject(work,work->nowp);
		return -1;
    }

    ans=*(work->ids+work->nowp)=(0x04<<24) | work->nowp | (work->nowid & 0x00ff0000);
	(work->mps_params+work->nowp)->func_type=0;
	(work->mps_params+work->nowp)->delay_count=-1;
	*(work->visible_flag+work->nowp)=visible_flag;

    work->nowp++;
    if(work->nowp>=work->size){
		work->nowp=0;
		work->nowid+=0x10000;
    }

    return ans;
}

/* 初期化部メイン */
int MakeAttachment4_called(int model_name,FVECTOR *v,SVECTOR *r,
						   OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources4_called(allocated_work,model_name,v,r,target,objnum,x,angle_limit,frames,0);
    }
    return ans;
}

int MakeAttachment4V_called(int model_name,FVECTOR *v,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames)
{
    int ans=-1;
    if(allocated_work!=NULL){
		ans=GetResources4_called(allocated_work,model_name,v,r,target,objnum,x,angle_limit,frames,1);
    }
    return ans;
}
