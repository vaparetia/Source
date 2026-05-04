//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachment4.c
		装備品揺らし

	1999/12/03 K.Kano
	$Id: attachment4.c,v 1.1.1.3 2002/11/19 11:43:04 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include "attachment4.h"
#include "attachment_debug.h"

#include "../../mode/demo/libdemo.h"


/* ワーク */
typedef	struct _Work {
    GV_ACT_EX		actor ;		/* リンク */
    DG_OBJS		*objs ;

    MODEL_PARAMETER4	mp;

	int name;
	int invisible_flag;

} Work ;


#define	ATTACHMENT4_NAME	GV_StrCode( "装備品Ｄ" )
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(/* ATTACHMENT4_NAME */ work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);

#if 0
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
#else
		switch(command){
		case 0:
			/* 表示／非表示 */
			{
				int disp_sw;
				disp_sw=*(msg->message+1);

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
#endif

		msg++;
		n_msg--;
    }


	DG_VisibleObjs(work->objs);

	if(work->mp.target!=NULL){
		int flag=0x000f;

		work->objs->group_id=work->mp.target->objs->group_id;

		if(work->mp.target->evmobj!=NULL){
			flag&=((work->mp.target->evmobj->flag &
					(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
		}

		flag&=((work->mp.target->objs->flag &
				(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3))>>12);
		flag<<=12;

		work->objs->flag=(work->objs->flag &
						  ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3)) | flag;
	}

	if(work->invisible_flag){
		work->objs->flag|=(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3);
	}

	if(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT)){
		InitCalcAttachment4(&(work->mp));
	}

	MoveAttachment4(work->objs,&(work->mp));
}

static void Die(Work *work)
{
    ExitAttachment4(&(work->mp));
    if(work->objs!=NULL){
		DG_DequeueObjs(work->objs);
		DG_FreeObjs(work->objs);
    }
}

static void InitAct(Work *work)
{
	InitCalcAttachment4(&(work->mp));
	GV_ChangeActFunc(&(work->actor),Act);
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */

/* オブジェクトを初期化 */
static int InitObject(Work *work,int name)
{
    DG_DEF *def ;

    def = (DG_DEF*) GV_GetCache( GV_CacheID( name, 'k' ) );
    if((work->objs = DG_MakeObjs( def, OBJECT_FLAG, 0 ))==NULL) return 0;
    DG_QueueObjs( work->objs );
	DG_InvisibleObjs( work->objs );

    return 1;
}

/* 初期設定値を取得 */
static int GetModelName(int *name)
{
#if 0
    char *x;

    if( GCL_GetOption( 'n' ) != NULL ){
		x=GCL_GetNextString();
    }
    else return 0;

    for( ;x!='\0';x++,name++) *name=*x;
    *name='\0';
#else
    if( GCL_GetOption( 'n' ) != NULL ){
		*name=GCL_GetNextInt();
    }
    else return 0;
#endif

    return 1;
}

static void GetOptionValue(MODEL_PARAMETER4 *mp,DG_OBJS *objs)
{
    mp->tmat=NULL;
    fpu_CopyUnitMatrix(&(mp->rotm));
    fpu_ClearVector(&(mp->x[0]));

    mp->target=NULL;
    mp->tobjnum=0;

    mp->stockv=NULL;
    mp->stockp=0;
    mp->stock_size=6;

    mp->sin_limit=0.258819045f; /* sin 30/2 */
    mp->cos_limit=0.965925826f; /* cos 30/2 */


    /* 初期位置 */
    if( GCL_GetOption( 'x' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf,&(mp->x[0]) ) ;
    } 
    mp->x[0].vw=1.0f;

    /* 特定charaの特定objに付いていく */
    if( GCL_GetOption( 'y' ) != NULL ){
		CONTROL *ctrl;
		int name;

		name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
		if(ctrl!=NULL){
			if((mp->target=ctrl->object)==NULL){
				mp->target=(OBJECT *)(ctrl+1);
			}
		}
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		mp->tobjnum=GCL_GetNextInt();
    }
    if(mp->target!=NULL){
		mp->tmat=&(mp->target->objs->objs[mp->tobjnum].world);
		DG_SetLightMatrix(objs,mp->target->objs->light);
    }
    else{
		DG_GetLightMatrix(&(mp->x[0]),mp->light);
		DG_SetLightMatrix(objs,mp->light);
    }

    /* 動きを記録する点 */
    if( GCL_GetOption( 'v' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf,&(mp->lastobjx) ) ;
    } 
    mp->lastobjx.vw=1.0f;

	/* 回転角の制限 */
    if( GCL_GetOption( 'l' ) != NULL ){
		int l=GCL_GetNextInt();
		float fl;

		fl=(float)l*(float)M_PI/2048.0f*0.5f;
		mp->sin_limit=sinf(fl);
		mp->cos_limit=cosf(fl);
    }

    /* 遅れフレーム数 */
    if( GCL_GetOption( 'f' ) != NULL ){
		mp->stock_size=GCL_GetNextInt();
    }

    if( GCL_GetOption( 'r' ) != NULL ){
		int buf[ 3 ];
		SVECTOR r;

		GCL_GetIV(GCL_NextStr(),buf);
		r.vx=buf[0];
		r.vy=buf[1];
		r.vz=buf[2];
		DG_SetPos(&DG_UnitMatrix);
		DG_RotatePosZYX(&r);
		DG_GetPos(&(mp->rotm));
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    int name;

    if(!GetModelName(&name)) return 0;
    if(!InitObject( work,name )) return 0;
    InitAttachment4(work->objs,&(work->mp));
    GetOptionValue(&(work->mp),work->objs);
    if(!Attachment4_GetStock(&(work->mp))) return 0;

#if 0
	if(work->mp.target!=NULL){
		work->name=((CONTROL *)(work->mp.target-1))->name;
	}
#endif

    return 1;
}

/* 初期化部メイン */
void *NewAttachment4(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		work->name=name;
		work->invisible_flag=0;

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


void Attachment4_GetOptionValue_called(MODEL_PARAMETER4 *mp,DG_OBJS *objs,
									   FVECTOR *v,SVECTOR *r,OBJECT *target,int objnum,FVECTOR *x,
									   int angle_limit,int frames)
{
    mp->tmat=NULL;
    fpu_CopyUnitMatrix(&(mp->rotm));
    fpu_ClearVector(&(mp->x[0]));

    mp->target=NULL;
    mp->tobjnum=0;

    mp->stockv=NULL;
    mp->stockp=0;

    mp->stock_size=6;
    if(frames>0) mp->stock_size=frames;

    mp->sin_limit=0.258819045f; /* sin 30/2 */
    mp->cos_limit=0.965925826f; /* cos 30/2 */


    /* 初期位置 */
    if(x!=NULL){
		fpu_CopyVector(&(mp->x[0]),x);
    } 
    mp->x[0].vw=1.0f;

    /* 特定charaの特定objに付いていく */
    mp->target=target;
    mp->tobjnum=objnum;

    if(mp->target!=NULL){
		mp->tmat=&(mp->target->objs->objs[mp->tobjnum].world);
		DG_SetLightMatrix(objs,mp->target->objs->light);
    }
    else{
		DG_GetLightMatrix(&(mp->x[0]),mp->light);
		DG_SetLightMatrix(objs,mp->light);
    }

    /* 当たりをみる点 */
    if(v!=NULL){
		fpu_CopyVector(&(mp->lastobjx),v);
    } 
    mp->lastobjx.vw=1.0f;

    {
		float fl;

		fl=(float)angle_limit*(float)M_PI/2048.0f*0.5f;
		mp->sin_limit=sinf(fl);
		mp->cos_limit=cosf(fl);
    }

    if(r!=NULL){
		DG_SetPos(&DG_UnitMatrix);
		DG_RotatePosZYX(r);
		DG_GetPos(&(mp->rotm));
    }
}

/* 資源を獲得 */
static int GetResources_called(Work *work,
							   int model_name,FVECTOR *v,SVECTOR *r,
							   OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames)
{
    if(!InitObject(work,model_name)) return 0;
    InitAttachment4(work->objs,&(work->mp));
    Attachment4_GetOptionValue_called(&(work->mp),work->objs,
									  v,r,target,objnum,x,angle_limit,frames);
    if(!Attachment4_GetStock(&(work->mp))) return 0;

#if 0
	if(work->mp.target!=NULL){
		work->name=((CONTROL *)(work->mp.target-1))->name;
	}
#endif

    return 1;
}

/* 初期化部メイン */
void *NewAttachment4_called(int model_name,FVECTOR *v,SVECTOR *r,
							OBJECT *target,int objnum,FVECTOR *x,int angle_limit,int frames)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));

		work->name=0;
		work->invisible_flag=0;

		if(!GetResources_called(work,model_name,v,r,target,objnum,x,angle_limit,frames)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
