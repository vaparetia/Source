//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	selectscr.c
		ゲームセレクト画面のマネージャー

	2001/06/12 K.Kano
	2002/07/11 Y.Yano サブスタンス用に変更
	$Id: selectscr.c,v 1.1.1.3 2002/11/19 11:43:41 Yoshizawa1 Exp $
*/


#include "titlescr.h"

#include "bgscr.h"
#include "../transfarring_menu/transfarring_menu.h"

typedef struct {
	GV_ACT_EX actor;
	int name;

	int l2d_strcode;
	int l2d_handle;
	int frame_l2d_strcode;
	int frame_l2d_handle;
	int frame_action_strcode;
	int frame_busy_flag;

	int ex_l2d_handle;

	int mode;
	int newmode;
	int pre_gamesel_cursor;

	int proc;

	int start_mode;

	int ans;

	int bgname;

	void *bgact;
} Work;


#define DISP_CHANL			TITLE_CHANL
#define MENU_PRIORITY		TITLE_PRI_NORMAL

#define CODE_L2D_OPTION		GV_StrCode("node_menu")


#define SPECIAL_ACTORNAME	0x002ebea0		/* special_menu */

//BP_TROPHY - track game type
int gBP_PlayingMissions = 0;
//BP_TROPHY - track game type

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
	ComNodeFrameAction(sign);

#ifdef DEBUG_MODE
	printf("Layout Signal = %d ( 0x%08x )\n",sign,sign);
#endif

}

static void CallLocalProc(Work *work,int ans)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=ans;
		argv[1]=work->mode-TITLE_NEWGAME;

		GCL_ExecProc(work->proc,&arg);
	}
}

static void RetToGamesel(Work *work)
{
	work->newmode=TITLE_GAMESEL;
	work->start_mode=1;

	if(work->l2d_handle<0){
		work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,MENU_PRIORITY,0);

		/* シグナルハンドラの設定 */
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);

		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

#ifdef DEBUG_MODE
	printf("RetToGamesel\n");
#endif

}

static void ProgMessage(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		switch(command){
		case TITLE_MSG_MODEEND:
			switch(work->mode){
			case TITLE_GAMESEL:
				{
               // this should line up with menu_cursor in gamesel.c
					static const unsigned char mode[]={
						TITLE_NEWGAME,
						TITLE_LOADGAME,
						TITLE_OPTION,
						TITLE_SPECIAL,
						TITLE_MISSIONS,
						TITLE_SNAKETALES,
						TITLE_TRANSFARRING,
                  TITLE_BACKTOGAMESELECT
					};
					work->pre_gamesel_cursor=arg;
					work->newmode=mode[arg];
				}
				break;
			case TITLE_NEWGAME:
			case TITLE_LOADGAME:
				GV_DestroyActor(work);
				break;
			case TITLE_OPTION:
				RetToGamesel(work);
				break;
			case TITLE_SPECIAL:
				RetToGamesel(work);
				work->start_mode=0;
				break;
         case TITLE_TRANSFARRING:
            // create new actor for transfarring
            //RetToGamesel(work);
            GV_DestroyActor(work);
            break;
			case TITLE_MISSIONS:
			case TITLE_SNAKETALES:
			case TITLE_BACKTOGAMESELECT:
				GV_DestroyActor(work);
				break;
			}
			break;
		case TITLE_MSG_MODECANCEL:
			switch(work->mode){
			case TITLE_GAMESEL:
				CallLocalProc(work,PROC_COM_GAMESEL_CANCEL);
				GV_DestroyActor(work);
				break;
			case TITLE_LOADGAME:
			case TITLE_SPECIAL:
				RetToGamesel(work);
				work->start_mode=0;
				break;
			default:
				RetToGamesel(work);
				break;
			}
			break;
		}

		msg++;
		n_msg--;
    }
}

static void Act(Work *work)
{
	ProgMessage(work);

	switch(work->newmode){
	  case TITLE_GAMESEL:
		if(ComNodeFrameBusy()) break;

		if(work->ex_l2d_handle>=0){
			if(L2D_ActionStatus(work->ex_l2d_handle)!=L2D_STAT_ACK) break;
			
			L2D_ReleaseLayout(work->ex_l2d_handle);
			work->ex_l2d_handle=-1;
		}
		
		GV_SetActorChild(work,NewGameSelScr(work->name,work->l2d_handle,
											work->proc,work->start_mode,
											work->pre_gamesel_cursor,work->bgname));
		work->mode = work->newmode;
		work->newmode = TITLE_NONE;
		break;
	  case TITLE_NEWGAME:
		if(ComNodeFrameBusy()) break;

		GV_SetActorChild(work,NewNewGameScr(work->name,work->l2d_handle,work->proc,work->bgname));
		work->mode = work->newmode;
		work->newmode = TITLE_NONE;

		CallLocalProc(work,PROC_COM_GAMESEL);
		break;

	  case TITLE_OPTION:
	  case TITLE_SPECIAL:
	  case TITLE_LOADGAME:
	  case TITLE_MISSIONS:
	  case TITLE_SNAKETALES:
	  case TITLE_BACKTOGAMESELECT:
		if(ComNodeFrameBusy()) break;

		if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
			L2D_ReleaseLayout(work->l2d_handle);
			work->l2d_handle=-1;
			work->mode = work->newmode;
			work->newmode = TITLE_NONE;

			CallLocalProc(work,PROC_COM_GAMESEL);
		}
		break;
     case TITLE_TRANSFARRING:
        if(ComNodeFrameBusy()) break;

        GV_SetActorChild(work, NewTransfarringMenu(work->name));
        work->mode = work->newmode;
        work->newmode = TITLE_NONE;

        //Due to the changes to the mode enum, this ended up calling script for
        //the skateboard option and killing background music.
        //CallLocalProc(work,PROC_COM_GAMESEL);
        break;
	}
}

static void Die(Work *work)
{
   //BP_TROPHY - track game type
   gBP_PlayingMissions = (work->mode == TITLE_MISSIONS);
   //BP_TROPHY - track game type

	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->proc=0;
	work->bgname=0;
	work->ex_l2d_handle=-1;

#if 0
	work->pre_gamesel_cursor=0;
#else
	if(GetLoaddataID()==MC_LOADDATA_NOID){
		/* NEWGAME */
		work->pre_gamesel_cursor=0;
	}
	else{
		/* LOADGAME */
		work->pre_gamesel_cursor=1;
	}
#endif

	work->mode=work->newmode=TITLE_GAMESEL;
	work->start_mode=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('b')!=NULL){
		work->bgname=GCL_GetNextInt();
    }
    if(GCL_GetOption('s')!=NULL){
		int flag=GCL_GetNextInt();
		switch( flag ){
		  case 1:
			work->mode=TITLE_SPECIAL;
			work->newmode=TITLE_NONE;
			work->pre_gamesel_cursor=3;
			break;
		  case 2:
			work->mode=TITLE_MISSIONS;
			work->newmode=TITLE_GAMESEL;
			work->pre_gamesel_cursor=4;
			break;
		  case 3:
			work->mode=TITLE_SNAKETALES;
			work->newmode=TITLE_GAMESEL;
			work->pre_gamesel_cursor=5;
			break;
		  case 4:
			work->mode=TITLE_BACKTOGAMESELECT;
			work->newmode=TITLE_GAMESEL;
			work->pre_gamesel_cursor=6;
			break;
		}
	}
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
   //BP_TROPHY - record game type
   gBP_PlayingMissions = 0;
   //BP_TROPHY - record game type

	GetOptionValue(work);


	work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,MENU_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG_MODE
		printf("Select Scr : L2D Init Error = %d\n",work->l2d_handle);
#endif

		ASSERT(0);
	}
	else{
		/* シグナルハンドラの設定 */
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);

		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

    return 1;
}

/* 初期化部メイン */
void *NewSelScrMan(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL ;
		}

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
