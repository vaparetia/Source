//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	special.c
		スペシャル選択画面

	2001/06/15 K.Kano
	2002/07/11 Y.Yano サブスタンス用に変更
	$Id: special.c,v 1.1.1.3 2002/11/19 11:43:41 Yoshizawa1 Exp $
*/


#include "titlescr.h"

#include "BP_TrophyLogicMGS2.h"

#define ANIM_WORK_SIZE		10

#define N_MENU_ITEMS		7

#define ANIM_SHOW_COUNT		DIRECT_TICK(20)
#define ANIM_HIDE_COUNT		DIRECT_TICK(20)

#define MV_ANIM_COUNT		DIRECT_TICK(10)


#define DISP_CHANL			TITLE_CHANL
#define MENU_PRIORITY		TITLE_PRI_NORMAL


typedef struct {
	GV_ACT_EX actor;
	int name;

	int proc;
	int ans;

	int busy_flag;
	int l2d_strcode;
	int l2d_handle;
	int action_strcode;

	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int menu_cursor;

	int start_mode;

	int wait_count;

	int sound_flag;

	int enable_items;
	int debug_flag;

	struct {
		int part_code;
		int key0_code;
		int key1_code;
		float p;
		int count;
	} key_anim[ANIM_WORK_SIZE];
} Work;


#define REPEAT_FIRST		20
#define REPEAT_NEXT			1

#define WAIT_COUNT			4


enum {
	ITEM_SEL_INDEX=0,
	ITEM_UNSEL_INDEX,
};


static struct {
	int           cur_pos;
	int           menu_item;
	int			  show,hide;
	int           sel[2];
} menu_list[] = {

	/* BASIC ACTION */
	{ CODE_CURSSPBASICACTION,
	  CODE_MENUBASICACTION,
	  CODE_SHOWSPBASICACTION,CODE_HIDESPBASICACTION,
	  { CODE_SELSPBASICACTION,CODE_UNSELSPBASICACTION, },
	},


	/* PREVIOUS STORY */
	{ CODE_CURSSPPRE,
	  CODE_MENUPRE,
	  CODE_SHOWSPPRE,CODE_HIDESPPRE,
	  { CODE_SELPRE,CODE_UNSELPRE,  },
	},

	/* PHOTO ALBUM */
	{ CODE_CURSSPPHOTO,
	  CODE_MENUPHOTO,
	  CODE_SHOWSPPHOTO,CODE_HIDESPPHOTO,
	  { CODE_SELPHOTO,CODE_UNSELPHOTO,  },
	},

	/* DOGTAG LIST */
	{ CODE_CURSSPDOG,
	  CODE_MENUDOG,
	  CODE_SHOWSPDOG,CODE_HIDESPDOG,
	  { CODE_SELDOG,CODE_UNSELDOG,  },
	},


	/* CAST CHANGE */
	{ CODE_CURSSPDEMO,
	  CODE_MENUDEMO,
	  CODE_SHOWSPDEMO,CODE_HIDESPDEMO,
	  { CODE_SELDEMO,CODE_UNSELDEMO,  },
	},

	/* BOSS SURVIVAL */
	{ CODE_CURSSPBOSS,
	  CODE_MENUBOSS,
	  CODE_SHOWSPBOSS,CODE_HIDESPBOSS,
	  { CODE_SELBOSS,CODE_UNSELBOSS,  },
	},

	/* EXIT */
	{ CODE_CURSSPEXIT,
	  CODE_MENUEXIT,
	  CODE_SHOWSPEXIT,CODE_HIDESPEXIT,
	  { CODE_SELEXIT,CODE_UNSELEXIT,  },
	},
};


enum {
	SPECIAL_MAIN_MENU=0,

	SPECIAL_BASIC_ACTIONS,

	SPECIAL_PREVIOUS_STORY,
	SPECIAL_PHOTO_ALBUM,
	SPECIAL_DOGTAG_LIST,

	SPECIAL_LOWPOLY_DEMO,
	SPECIAL_BOSS_RUSH,

	SPECIAL_EXIT,
};


enum {

	SPECIAL_ITEM_BASIC_ACTIONS=0,
	SPECIAL_ITEM_PREVIOS_STORY,

	SPECIAL_ITEM_PHOTO_ALBUM,
	SPECIAL_ITEM_DOGTAG_LIST,

	SPECIAL_ITEM_LOWPOLY_DEMO,
	SPECIAL_ITEM_BOSS_RUSH,

	SPECIAL_ITEM_EXIT,
};


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void KeyAnimSetX(Work *work,int index,int count,int part_code,int key0_code,int key1_code)
{
	work->key_anim[index].p=0.0f;
	work->key_anim[index].count=count;
	work->key_anim[index].part_code=part_code;
	work->key_anim[index].key0_code=key0_code;
	work->key_anim[index].key1_code=key1_code;
}

static void KeyAnim(Work *work)
{
	int i;

	for(i=0;i<ANIM_WORK_SIZE;i++){
		if(work->key_anim[i].count>0){
			void *part;

			part=L2D_GetParts(work->l2d_handle,work->key_anim[i].part_code);
			if(part==NULL) return;

			work->key_anim[i].p+=(1.0f-work->key_anim[i].p)/(float)work->key_anim[i].count;

			L2D_MorfObject(part,work->key_anim[i].key0_code,work->key_anim[i].key1_code,
						   work->key_anim[i].p);

			work->key_anim[i].count--;
		}
	}
}


static void ShowMenu(Work *work,int count)
{
	int i;
	for(i=0;i<N_MENU_ITEMS;i++){
		KeyAnimSetX(work,i,count,
					menu_list[i].menu_item,menu_list[i].hide,menu_list[i].show);
	}
	KeyAnimSetX(work,i,count,CODE_SELCURS,CODE_HIDECURS,CODE_SHOWCURS);
}

static void HideMenu(Work *work,int count)
{
	int i;
	for(i=0;i<N_MENU_ITEMS;i++){
		KeyAnimSetX(work,i,count,
					menu_list[i].menu_item,menu_list[i].show,menu_list[i].hide);
	}
	KeyAnimSetX(work,i,count,CODE_SELCURS,CODE_SHOWCURS,CODE_HIDECURS);
}

static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_SHOWSPMENU:
		ShowMenu(work,value/DIV_TICK_VALUE);
		break;
	case CODE_HIDESPMENU:
		HideMenu(work,value/DIV_TICK_VALUE);
		break;
	default:
		ComNodeFrameAction(sign);
		break;
	case CODE_SOUND:
		if(work->sound_flag) SE_EXPANDLINE();
		else SE_WINOPEN();
		break;
	}

#ifdef DEBUG_MODE
	printf("Special Layout Signal = %d ( 0x%08x )\n",sign,sign);
#endif

}


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


static void ProgMessage(Work *work)
{

#if 0
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		msg++;
		n_msg--;
    }
#endif

}

static void Key(Work *work)
{
	int status=GV_PadDataDirect[0].status & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);
	int press=GV_PadDataDirect[0].press & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);

	work->key_press=press;

	if(status && status==work->key_status){
		if(work->key_count>REPEAT_FIRST){
			work->key_count-=REPEAT_NEXT;
			work->key_autostatus=work->key_status;
		}
		else work->key_autostatus=0;

		work->key_count++;
	}
	else{
		work->key_autostatus=work->key_status=status;
		work->key_count=0;
	}
}


/* ------------------------------------------------------------------------ */


static void CursorPosInit(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CODE_SELCURSNULL);
	if(part==NULL) return;

	L2D_MorfObject(part,
				   menu_list[work->menu_cursor].cur_pos,
				   menu_list[work->menu_cursor].cur_pos,1.0f);
}

static void CursorSelInit(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,menu_list[work->menu_cursor].menu_item);
	if(part==NULL) return;

	L2D_MorfObject(part,
				   menu_list[work->menu_cursor].sel[ITEM_SEL_INDEX],
				   menu_list[work->menu_cursor].sel[ITEM_SEL_INDEX],1.0f);
}

static void VisibleItemControl(Work *work)
{
	int i;
	for(i=0;i<N_MENU_ITEMS;i++){
		if(!(work->enable_items & (1<<i))){
			SPR_OBJ *spr=L2D_GetObject(work->l2d_handle,menu_list[i].menu_item);
			if(spr!=NULL) SPR_HIDE(spr);
		}
	}
}

static int SubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			if(work->start_mode){
				work->action_strcode=CODE_SHOWSP;
			}
			else{
				work->action_strcode=CODE_SHOWSP;
			}
			work->sub_step++;

			CursorPosInit(work);

			SE_EXPANDLINE();
		}
		break;
	case 1:
		VisibleItemControl(work);

		if(!work->busy_flag){
			work->sub_step++;
		}
		break;
	case 2:
		VisibleItemControl(work);

		if(work->key_anim[0].count>0) break;
		CursorSelInit(work);
		work->sub_step++;

	case 3:
		VisibleItemControl(work);

		if(work->key_anim[0].count>0) break;

		if(work->key_press & PAD_CANCEL){
			SE_CANCEL();

			work->sub_step=0x10;
			work->ans=-1;
			return 0;
		}
		else if(work->key_press & PAD_OK){
			if(work->menu_cursor==SPECIAL_ITEM_EXIT) SE_CANCEL(); /* EXIT */
			else SE_OK(); /* other */

			work->sub_step=0x10;
			work->ans=work->menu_cursor;
			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->menu_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				do{
					work->menu_cursor--;
					if(work->menu_cursor<0){
						work->menu_cursor=N_MENU_ITEMS-1;
					}
				} while(!(work->enable_items & (1<<work->menu_cursor)));

				SE_SEL();
				break;
			case PAD_D:
				do{
					work->menu_cursor++;
					if(work->menu_cursor>=N_MENU_ITEMS){
						work->menu_cursor=0;
					}
				} while(!(work->enable_items & (1<<work->menu_cursor)));

				SE_SEL();
				break;
			}

			cur1=work->menu_cursor;

			if(cur0!=cur1){
				KeyAnimSetX(work,0,MV_ANIM_COUNT,CODE_SELCURSNULL,
							menu_list[cur0].cur_pos,
							menu_list[cur1].cur_pos);

				KeyAnimSetX(work,1,MV_ANIM_COUNT,menu_list[cur0].menu_item,
							menu_list[cur0].sel[ITEM_SEL_INDEX],
							menu_list[cur0].sel[ITEM_UNSEL_INDEX]);

				KeyAnimSetX(work,2,MV_ANIM_COUNT,menu_list[cur1].menu_item,
							menu_list[cur1].sel[ITEM_UNSEL_INDEX],
							menu_list[cur1].sel[ITEM_SEL_INDEX]);
			}
		}
		break;

	case 0x10:
		VisibleItemControl(work);

		work->sub_step++;

		work->action_strcode=CODE_HIDESP;
		work->wait_count=WAIT_COUNT;
		work->sound_flag=1;

		SE_WINCLOSE();
		break;
	case 0x11:
		VisibleItemControl(work);

		if(work->busy_flag) break;
		return 1;
	}

	return 0;
}

static void Step(Work *work)
{
	switch(work->step){
	case SPECIAL_MAIN_MENU:
		switch(SubStep(work)){
		case 1:
		case -1:
			work->step=SPECIAL_EXIT;
			work->sub_step=0;
			break;
		}
		break;
	case SPECIAL_PREVIOUS_STORY:
	case SPECIAL_PHOTO_ALBUM:
	case SPECIAL_DOGTAG_LIST:

	case SPECIAL_BASIC_ACTIONS:
	case SPECIAL_BOSS_RUSH:
	case SPECIAL_LOWPOLY_DEMO:

		break;
	case SPECIAL_EXIT:
		if(work->busy_flag) break;

		work->wait_count--;
		if(work->wait_count>0) break;

		if(work->l2d_handle>=0){
			L2D_ReleaseLayout(work->l2d_handle);
			work->l2d_handle=-1;
		}

      if( work->menu_cursor == SPECIAL_ITEM_LOWPOLY_DEMO || work->menu_cursor == SPECIAL_ITEM_BOSS_RUSH )
      {
         bp_trophy_check_warning_cleared_by_user();
      }
		CallLocalProc(work);
		GV_DestroyActor(work);
		break;
	}
}


/* ------------------------------------------------------------------------ */


static void Act(Work *work)
{
	if(work->l2d_handle>=0){
		if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
			work->busy_flag=0;
			if(work->action_strcode!=0){
				int stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
				printf("L2D Stat = %d\n",stat);
#endif

				work->action_strcode=0;
				work->busy_flag=1;
			}
		}
		else{
			work->busy_flag=1;
		}

		KeyAnim(work);
	}
	else{
		work->busy_flag=0;
	}


	ProgMessage(work);

	Key(work);
	Step(work);
}

static void Die(Work *work)
{
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->proc=0;
	work->start_mode=0;

	work->menu_cursor=0;

	work->debug_flag=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('m')!=NULL){
		work->start_mode=GCL_GetNextInt();
    }
	if(GCL_GetOption('s')!=NULL){
		work->menu_cursor=GCL_GetNextInt();
	}
	if(GCL_GetOption('g')!=NULL){
		work->debug_flag=GCL_GetNextInt();
	}
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
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

static void InitMenu(Work *work)
{
	int i;

	work->step=0;
	work->sub_step=0;

	work->sound_flag=0;

	for(i=0;i<ANIM_WORK_SIZE;i++){
		work->key_anim[i].count=0;
	}

	work->enable_items=0xcf;
	if((GM_TitleMenuStatus & (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD))
	   ==(TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD)){

		work->enable_items|=0x30;
	}

	if(work->debug_flag){
		work->enable_items|=0x30;
	}

}

/* 初期化部メイン */
void *NewSpecialScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->busy_flag=0;
		work->action_strcode=0;

		work->ans=0;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL ;
		}

		InitMenu(work);

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
