//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cergei_eri.c
		マルチウェイトのセルゲイの襟揺らし

	2001/03/19 K.Kano
	$Id: cergei_eri.c,v 1.1.1.3 2002/11/19 11:43:06 Yoshizawa1 Exp $
*/

#include "cergei_eri.h"

#include "../../mode/demo/libdemo.h"


/* ワーク */
typedef	struct _Work {
    GV_ACT_EX		actor ;		/* リンク */

	int name;
    CERGEI_ERI_WORK	mp;
} Work ;


enum {
	CERGEI_ERI_MSG_CALCON=0,
	CERGEI_ERI_MSG_CALCOFF,
	CERGEI_ERI_MSG_CALCREV,
};


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		// int arg=*(msg->message+1);

		switch(command){
		case CERGEI_ERI_MSG_CALCON:
			work->mp.flag&=~CERGEI_ERI_FLAG_CALC_DISABLE;
			break;
		case CERGEI_ERI_MSG_CALCOFF:
			work->mp.flag|=CERGEI_ERI_FLAG_CALC_DISABLE;
			break;
		case CERGEI_ERI_MSG_CALCREV:
			work->mp.flag^=CERGEI_ERI_FLAG_CALC_DISABLE;
			break;
		}

		msg++;
		n_msg--;
	}

	if(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT)){
		CergeiEriFirstCalc(&(work->mp));
	}

	MoveCergeiEri(&(work->mp));
}

static void InitAct(Work *work)
{
	CergeiEriFirstCalc(&(work->mp));
	GV_ChangeActFunc(&(work->actor),Act);
}

static void Die(Work *work)
{
    ExitCergeiEri(&(work->mp));
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


static void GetOptionValue(CERGEI_ERI_WORK *mp)
{
	mp->n_objs=1;

    mp->target=NULL;
    mp->tobjnum[0]=0;

	mp->pa=250*0.000001f;
	mp->k=-1000*0.000001f;
	mp->deg_param=500*0.01f;

	mp->cos_limit=DEFAULT_COS_LIMIT;
	mp->sin_limit=DEFAULT_SIN_LIMIT;

	mp->mov_rate=0.0f;
	mp->ymov_rate=0.0f;
	mp->flag=0;

    /* 特定charaの特定objに付いていく */
    if( GCL_GetOption( 'y' ) != NULL ){
		CONTROL *ctrl;
		int name;

		name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
		if(ctrl!=NULL){
			OBJECT *object=ctrl->object;
			if(object==NULL) object=(OBJECT *)(ctrl+1);
			mp->target=object->evmobj;
		}
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		char *str;
		int cnt=0;

		while((str=GCL_NextStr())!=NULL){
			mp->tobjnum[cnt]=GCL_GetInt(str);
			cnt++;
		}
		mp->n_objs=cnt;
    }

	/* 動きの想定点 */
    if( GCL_GetOption( 't' ) != NULL ){
		int buf[ 3 ];
		int i;

		for(i=0;i<mp->n_objs;i++){
			GCL_GetIV( GCL_NextStr(), buf );
			vu0_IV0toFV((IVECTOR *)buf, &(mp->lastobjx[i]));
			mp->lastobjx[i].vw=1.0f;
		}
	}

	/* 角度の限界値 */
    if( GCL_GetOption( 'l' ) != NULL ){
		int deg;

		deg=GCL_GetNextInt();
		mp->cos_limit=cosf(((float)deg/4096.0f)*M_PI);
		mp->sin_limit=sinf(((float)deg/4096.0f)*M_PI);
	}

    /* 風によって受ける力 */
    if( GCL_GetOption('p') != NULL ){
		int p=GCL_GetNextInt();
		float fp;

		fp=(float)p*0.000001f;

		mp->pa=fp;
    }

    /* 速度に対する抵抗値 */
    if( GCL_GetOption('k') != NULL ){
		int k=GCL_GetNextInt();
		float fk;

		fk=(float)k*0.000001f;
		mp->k=-fk;
    }

    /* 曲がりを正すパラメータ */
    if( GCL_GetOption('a') != NULL ){
		int a=GCL_GetNextInt();
		float fa;

		fa=(float)a*0.01f;
		mp->deg_param=fa;
    }

	/* 全体の移動の何割を無視するかのパラメータ */
    if( GCL_GetOption('w') != NULL ){
		int w=GCL_GetNextInt();
		float fw;

		fw=(float)w*0.01f;
		mp->mov_rate=fw;
    }
    if( GCL_GetOption('j') != NULL ){
		int w=GCL_GetNextInt();
		float fw;

		fw=(float)w*0.01f;
		mp->ymov_rate=fw;
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    GetOptionValue(&(work->mp));
    if(work->mp.target==NULL){
		// printf("Check 1\n");
		return 0;
	}
    if(!InitCergeiEri(&(work->mp))) return 0;
    return 1;
}

/* 初期化部メイン */
void *NewCergeiEri(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));

		work->name=name;

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


static void GetOptionValue_called(CERGEI_ERI_WORK *mp,DG_EVMOBJ *target,
								  int n_targets,int *target_nums,FVECTOR *imgpnt,
								  int deg_limit,float pa,float k,float a,float w,float yw)
{
	mp->n_objs=1;

    mp->target=NULL;
    mp->tobjnum[0]=0;

	mp->pa=250*0.000001f;
	mp->k=-1000*0.000001f;
	mp->deg_param=500*0.01f;

	mp->cos_limit=DEFAULT_COS_LIMIT;
	mp->sin_limit=DEFAULT_SIN_LIMIT;

	mp->mov_rate=0.0f;
	mp->ymov_rate=0.0f;

	mp->n_objs=n_targets;

    /* 特定charaの特定objに付いていく */
	// mp->target=target->evmobj;
	mp->target=target;

    {
		int cnt=0;

		while(cnt<n_targets){
			mp->tobjnum[cnt]=target_nums[cnt];
			cnt++;
		}
    }

	/* 動きの想定点 */
    {
		int i;

		for(i=0;i<mp->n_objs;i++){
			fpu_CopyVector(&(mp->lastobjx[i]),&(imgpnt[i]));
			mp->lastobjx[i].vw=1.0f;
		}
	}

	/* 角度の限界値 */
    {
		int deg;

		deg=deg_limit;
		mp->cos_limit=cosf(((float)deg/4096.0f)*M_PI);
		mp->sin_limit=sinf(((float)deg/4096.0f)*M_PI);
	}

    /* 風によって受ける力 */
    {
		float fp=pa;
		mp->pa=fp;
    }

    /* 速度に対する抵抗値 */
    {
		float fk=k;
		mp->k=-fk;
    }

    /* 曲がりを正すパラメータ */
    {
		float fa=a;
		mp->deg_param=fa;
    }

	/* 全体の移動の何割を無視するかのパラメータ */
    {
		float fw=w;
		mp->mov_rate=fw;
    }
    {
		float fw=yw;
		mp->ymov_rate=fw;
    }
}

/* 資源を獲得 */
static int GetResources_called(Work *work,DG_EVMOBJ *target,
							   int n_targets,int *target_nums,FVECTOR *imgpnt,
							   int deg_limit,float pa,float k,float a,float w,float yw)
{
    GetOptionValue_called(&(work->mp),target,
						  n_targets,target_nums,imgpnt,
						  deg_limit,pa,k,a,w,yw);

    if(work->mp.target==NULL){
		// printf("Check 1\n");
		return 0;
	}
    if(!InitCergeiEri(&(work->mp))) return 0;
    return 1;
}

/* 初期化部メイン */
void *NewCergeiEri_called(int name,DG_EVMOBJ *target,
						  int n_targets,int *target_nums,FVECTOR *imgpnt,
						  int deg_limit,float pa,float k,float a,float w,float yw)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));

		work->name=name;

		if(!GetResources_called(work,target,
								n_targets,target_nums,imgpnt,
								deg_limit,pa,k,a,w,yw)){

			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



void *NewCergeiEri_demo(int sample_num,DG_EVMOBJ *object,int name)
{
	switch(sample_num){
	case 0:
		/* セルゲイの襟揺れ */
		{
			static const int target_nums[]={
				53,54,55,
			};
			static const FVECTOR imgpnt[]={
				{ -100.0f,   0.0f, 100.0f,1.0f, },
				{  100.0f,   0.0f, 100.0f,1.0f, },
				{    0.0f,   0.0f,-150.0f,1.0f, },
			};

			return NewCergeiEri_called(name,object,sizeof(target_nums)/sizeof(target_nums[0]),
									   (int *)target_nums,(FVECTOR *)imgpnt,512,
									   500.0f*0.000001f,55000.0f*0.000001f,30.0f*0.01f,
									   100.0f*0.01f,0.0f*0.01f);
		}
	default:
		return NULL;
	}
}

