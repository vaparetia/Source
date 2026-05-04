//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mw_hukuyure.c
		マルチウェイトの服揺らし

	2000/04/06 K.Kano
	$Id: mw_hukuyure.c,v 1.1.1.3 2002/11/19 11:43:20 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include "mw_hukuyure.h"


/* ワーク */
typedef	struct _Work {
    GV_ACT_EX		actor ;		/* リンク */

    MWHUKUYURE_WORK	mp;
} Work ;


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{

#ifdef DEBUG
	{
		int cnt;
		GV_SET_PRFC_CLOCK();
#endif

		MoveMWHukuyure(&(work->mp));

#ifdef DEBUG
		cnt=GV_GET_PRFC_CLOCK();
		if(DEBUG_BUTTON_CHECK()){
			float p;
			p=(float)cnt/CLOCK_PER_HSYNC;
			printf("MWHukuyure : %6.3f ( %d )\n",p,cnt);
		}
	}
#endif

}

static void Die(Work *work)
{
    ExitMWHukuyure(&(work->mp));
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */

static void GetOptionValue(MWHUKUYURE_WORK *mp)
{
    mp->target=NULL;
    mp->tobjnum=0;

    mp->stock_size=6;
    mp->limity=CVC2N(2.0f);

    /* 特定charaの特定objに付いていく */
    if( GCL_GetOption( 'y' ) != NULL ){
		CONTROL *ctrl;
		int name;

		name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
		if(ctrl!=NULL) mp->target=((OBJECT *)(ctrl+1))->evmobj;
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		mp->tobjnum=GCL_GetNextInt();
    }

    /* 遅れフレーム数 */
    if( GCL_GetOption( 'f' ) != NULL ){
		mp->stock_size=GCL_GetNextInt();
    }

    /* 上下幅 */
    if( GCL_GetOption( 'l' ) != NULL ){
		mp->limity=fpu_Abs((float)GCL_GetNextInt());
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    GetOptionValue(&(work->mp));
    if(work->mp.target==NULL) return 0;
    if(!InitMWHukuyure(&(work->mp))) return 0;
    return 1;
}

/* 初期化部メイン */
void *NewMWHukuyure(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


static void MWHukuyure_GetOptionValue_called(MWHUKUYURE_WORK *mp,OBJECT *target,
											 int objnum,int frames,float limity)
{
    mp->target=NULL;
    mp->tobjnum=0;

    mp->stock_size=6;
    mp->limity=CVC2N(2.0f);

    /* 特定charaの特定objに付いていく */
    if(target!=NULL){
		mp->target=target->evmobj;
    }
    mp->tobjnum=objnum;

    /* 遅れフレーム数 */
    mp->stock_size=frames;

    mp->limity=limity;
}

/* 資源を獲得 */
static int GetResources_called(Work *work,OBJECT *target,int objnum,int frames,float limity)
{
    MWHukuyure_GetOptionValue_called(&(work->mp),target,objnum,frames,limity);
    if(work->mp.target==NULL) return 0;
    if(!InitMWHukuyure(&(work->mp))) return 0;
    return 1;
}

/* 初期化部メイン */
void *NewMWHukuyure_called(OBJECT *target,int objnum,int frames,float limity)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources_called(work,target,objnum,frames,limity)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
