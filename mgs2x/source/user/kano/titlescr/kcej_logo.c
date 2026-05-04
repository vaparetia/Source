//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	kcej_logo.c
		KCEJロゴの表示

	2001/07/01 K.Kano
	$Id: kcej_logo.c,v 1.1.1.3 2002/11/19 11:43:37 Yoshizawa1 Exp $
*/


#include "titlescr.h"


typedef struct {
	GV_ACT_EX actor;
	int name;
	int timer;

	int fadein_time;
	int disp_time;
	int fadeout_time;

	int proc;
	int ans;

	SPR_OBJ *title_kcej;
} Work;


#define TEX_CODE			3656015			/* logo.tri */
#define TEX_CODE_KCEJ		(233616)		/* logo_japan.bmp */


#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) \
	SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))


#define DISP_CHANL			4


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=work->ans;

		GCL_ExecProc(work->proc,&arg);
	}
}

static int KcejAct(Work *work)
{
	if(work->timer==0){
		/* 開始 */
		SPR_SHOW(work->title_kcej);
	}
	else if(work->timer>work->fadeout_time){
		/* 終了 */
		SPR_HIDE(work->title_kcej);
		return 1;
	}

	if(work->timer<work->fadein_time){
		int alpha=work->timer*128/work->fadein_time;
		SET_COLOR_2DPRIM2(work->title_kcej,128,128,128,alpha);
	}
	else if(work->timer<work->disp_time){
		SET_COLOR_2DPRIM2(work->title_kcej,128,128,128,128);
	}
	else if(work->timer<work->fadeout_time){
		int alpha=(work->fadeout_time-work->timer)*128/(work->fadeout_time-work->disp_time);
		SET_COLOR_2DPRIM2(work->title_kcej,128,128,128,alpha);
	}
	else{
		SET_COLOR_2DPRIM2(work->title_kcej,128,128,128,0);
	}

	return 0;
}

static void Act(Work *work)
{

#if 0
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		msg++;
		n_msg--;
    }
#endif


	if(KcejAct(work)){
		if(work->title_kcej) SPR_Destroy_2D_Object(work->title_kcej);
		CallLocalProc(work);
		GV_DestroyActor(work);
	}

	work->timer++;
}

static void Die(Work *work)
{
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('t')!=NULL){
		work->fadein_time=DIRECT_TICK(GCL_GetNextInt());
		work->disp_time=work->fadein_time+DIRECT_TICK(GCL_GetNextInt());
		work->fadeout_time=work->disp_time+DIRECT_TICK(GCL_GetNextInt());
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

	SPR_LoadTexture(TEX_CODE);

	work->title_kcej = SPR_Create_2D_Object(SP_SPRITE, DISP_CHANL, NULL);
	if(work->title_kcej == NULL){ printf("ERR!! Make2DObj[work->title_kcej]\n"); return -1; }
	SPR_ObjSetTexture(work->title_kcej, TEX_CODE_KCEJ, 0);
#if 0 //BP_PS2
//#ifdef PSX2	
	SPR_SetPosSprite(work->title_kcej, &(SPR_POS){ 0.0f, 0.0f});
#else
	SPR_SetPosSprite(work->title_kcej, (SPR_POS*)&DG_ZeroVector );
#endif	
	SPR_SetSizeSprite(work->title_kcej, 512.0f, 384.0f );
	work->title_kcej->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_kcej->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->title_kcej,0x00000000);

    return 1;
}

/* 初期化部メイン */
void *NewTitleKcej(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_MANAGER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->title_kcej=NULL;
		work->timer=0;
		work->proc=0;
		work->ans=PROC_COM_OK;

		work->fadein_time=64;
		work->disp_time=128;
		work->fadeout_time=192;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
