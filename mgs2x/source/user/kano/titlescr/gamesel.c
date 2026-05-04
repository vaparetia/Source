//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gamesel.c
		新規ゲームスタート

	2001/06/12 K.Kano
	2002/07/11 Y.Yano サブスタンス用に変更
	$Id: gamesel.c,v 1.4 2002/11/23 12:24:51 Yoshizawa1 Exp $
*/

#include "titlescr.h"

#include "bgscr.h"

#include "BP_BuildDefines.h"
#include "../../../Builds/DiskBuilds/UseDiskBuild.h"
#include "BP_Misc.h"

//#define N_MENU_ITEMS		6

#define ANIM_COUNT			DIRECT_TICK(10)

#define ANIM_WORK_SIZE		2

#define CODE_MENUGAMESELECT (0x0088de24)       //BP JG - revive an old unused option to be replaced with 'return to game select'

#if defined(BP_VITA) || defined(BP_PS3)
//HACK to deal with MGSTWO-3242
extern int gGameClearChangeDetected;
#endif

enum
{
	P_NEWGAME = 0,
	P_LOADGAME,
	P_OPTIONS,
	P_SPECIAL,
	P_MISSIONS,
	P_SNAKETALES,
   P_TRANSFARRING,
   P_GAMESELECT,

	N_MAX_MENU_ITEMS,
};

int N_MENU_ITEMS()
{
   if( BP_IsDownloadableVersion() )
   {
      return N_MAX_MENU_ITEMS - 1;
   }
   else
   {
      return N_MAX_MENU_ITEMS;
   }
}

typedef struct _gamesel_Work {
	GV_ACT_EX actor;
	int parent_name;

	int busy_flag;
	int l2d_handle;
	int action_strcode;

	int proc;
	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int ans;
	int menu_cursor;

	SPR_OBJ *cursor;
	SPR_OBJ *cursor_spr;
	SPR_OBJ *menu[N_MAX_MENU_ITEMS];

	float t_ypos;

	int anim_count;
	struct {
		SPR_OBJ *target;
		unsigned char talpha;
	} key_anim[ANIM_WORK_SIZE];

	int bgname;
} GameselWork;


#define REPEAT_FIRST		20
#define REPEAT_NEXT			3


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void Key(GameselWork *work)
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

static void LocalShowCursor(GameselWork *work)
{
	SPR_SHOW(work->cursor);
	SPR_SHOW(work->cursor_spr);
}

static void HideCursor(GameselWork *work)
{
	SPR_HIDE(work->cursor);
	SPR_HIDE(work->cursor_spr);
}

static void BgscrFadein(GameselWork *work)
{
	GV_MSG msg;
	int message[3];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_FADE;
	message[1]=0;
	message[2]=DIRECT_TICK(60);

	GV_SendMessage(&msg);
}

static void BgscrFadeout(GameselWork *work)
{
	GV_MSG msg;
	int message[3];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_FADE;
	message[1]=128;
	message[2]=DIRECT_TICK(60);

	GV_SendMessage(&msg);
}

static void BgscrOpenGrd(GameselWork *work)
{
	GV_MSG msg;
	int message[1];

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_OPENGRD;

	GV_SendMessage(&msg);
}

static void BgscrCloseGrd(GameselWork *work)
{
	GV_MSG msg;
	int message[1];

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_CLOSEGRD;

	GV_SendMessage(&msg);
}

static void BgscrOpenPlmap(GameselWork *work)
{
	GV_MSG msg;
	int message[1];

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_OPENPLMAP;

	GV_SendMessage(&msg);
}

static void BgscrClosePlmap(GameselWork *work)
{
	GV_MSG msg;
	int message[1];

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_CLOSEPLMAP;

	GV_SendMessage(&msg);
}


static void AnimCtrlSet(GameselWork *work,int index,SPR_OBJ *spr,int talpha)
{
	work->key_anim[index].target=spr;
	work->key_anim[index].talpha=talpha;
}

static void AnimCtrl(GameselWork *work)
{
	int i;

	if(work->anim_count>0){
		for(i=0;i<ANIM_WORK_SIZE;i++){
			if(work->key_anim[i].target==NULL) continue;

			work->key_anim[i].target->sprite.col.a
				+=((int)(work->key_anim[i].talpha)-(int)(work->key_anim[i].target->sprite.col.a))
				/work->anim_count;
		}

		if(work->cursor!=NULL){
			work->cursor->empty.pos.y
				+=(work->t_ypos-work->cursor->empty.pos.y)/(float)(work->anim_count);
		}

		work->anim_count--;
	}
	else{
		for(i=0;i<ANIM_WORK_SIZE;i++){
			work->key_anim[i].target=NULL;
		}
	}
}

#if defined(BP_360)
extern int MGS_Get360SpaceForFileType(const char* saveType);
extern void DoDeviceSelectorLogic_ForSpace(int requiredSpace);

void DoDeviceSelectorLogic(int gameMode)
{
   int requiredSpace = 0;

   switch( gameMode )
   {
   case P_NEWGAME:
      requiredSpace = MGS_Get360SpaceForFileType("G");
      break;
   case P_SNAKETALES:
      requiredSpace = MGS_Get360SpaceForFileType("S");
      break;
   case P_MISSIONS:
      requiredSpace = MGS_Get360SpaceForFileType("V");
      break;
   case P_LOADGAME:
   case P_SPECIAL:
   case P_OPTIONS:
   case P_TRANSFARRING:
   case P_GAMESELECT:
      // We don't need to do anything for these modes
      return;
      break;
   default:
      return;
      break;
   }

   DoDeviceSelectorLogic_ForSpace(requiredSpace);
}
#else
void DoDeviceSelectorLogic(int gameMode)
{
}
#endif
static int SubStep(GameselWork *work)
{
	switch(work->sub_step){
	  case 0:
		if(!work->busy_flag){
			work->sub_step++;

			LocalShowCursor(work);

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->menu[work->menu_cursor]->sprite.pos.y;
			work->menu[work->menu_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	  case 1:
		if(work->anim_count>0) break;

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_HIDEMAIN;
			HideCursor(work);
			BgscrCloseGrd(work);
			BgscrClosePlmap(work);

			SE_CANCEL();

			return -1;
		}
		else if(work->key_press & PAD_OK)
      {
			static const int strcode[N_MAX_MENU_ITEMS]=
         {
				CODE_TONEWGAME,
				CODE_TOLOADGAME,
				CODE_TOOPTIONS,
				CODE_TOSPECIAL,
				CODE_TOMISSIONS,
				CODE_TOSNAKETALES,
            CODE_TOTRANSFARRING,
				CODE_TOGAMESELECT,
			};
			work->action_strcode=strcode[work->menu_cursor];
			HideCursor(work);

			if( work->menu_cursor == P_LOADGAME || 
			    work->menu_cursor == P_SPECIAL  || 
			    work->menu_cursor == P_MISSIONS || 
			    work->menu_cursor == P_SNAKETALES ||
             (!BP_IsDownloadableVersion() && work->menu_cursor == P_GAMESELECT)
			   ){ 
				BgscrFadeout( work );
			}
         //BP_SaveLoad Support - Check storage device for enough space for separate game modes
         DoDeviceSelectorLogic( work->menu_cursor );

			SE_OK();

			return 1;
		}

		{
			int cur0,cur1;

			cur0=work->menu_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:

#if BP_TGS_DEMO()
         do 
#endif
         {
               work->menu_cursor--;
   
				   if(work->menu_cursor<0)
               {
					   work->menu_cursor=N_MENU_ITEMS() - 1;
				   }
         }
#if BP_TGS_DEMO()
         while ( work->menu_cursor==P_LOADGAME || work->menu_cursor==P_SPECIAL || work->menu_cursor==P_MISSIONS || work->menu_cursor==P_SNAKETALES );
#endif

				break;
			case PAD_D:

#if BP_TGS_DEMO()
            do 
#endif
            {
               work->menu_cursor++;

               if(work->menu_cursor >= N_MENU_ITEMS())
               {
                  work->menu_cursor=0;
               }

            }
#if BP_TGS_DEMO()
           while ( work->menu_cursor==P_LOADGAME || work->menu_cursor==P_SPECIAL || work->menu_cursor==P_MISSIONS || work->menu_cursor==P_SNAKETALES );
#endif
            break;
			}

			cur1=work->menu_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;

				work->t_ypos=work->menu[work->menu_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->menu[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->menu[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;
	}

	return 0;
}

static void Step(GameselWork *work)
{





	switch(work->step){
	case 0:
		switch(SubStep(work)){
		case 1:
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;
			break;
		case -1:
			work->ans=TITLE_MSG_MODECANCEL;
			work->step=0x10;
			break;
		}
		break;

	case 0x10:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag)
      {

			GV_DestroyActor(work);

         if ( !BP_IsDownloadableVersion() && work->ans==TITLE_MSG_MODEEND && work->menu_cursor==P_GAMESELECT )     // if we're going back to the boot menu, force a reset.
         {
            BP_RelaunchTheBootLoaderFromUI();
         }
      }
		break;
	}

	AnimCtrl(work);
}

static void Act(GameselWork *work)
{
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

#if defined(BP_VITA) || defined(BP_PS3)
   if(gGameClearChangeDetected)
   {
      gGameClearChangeDetected = 0;
      BP_RelaunchTheGameFromUINoPrompt();
   }
#endif

	Key(work);
	Step(work);

   // AS(JM) - If this is "downloadable", kill the game select sprite object
   // Doing this every frame so that no action can possibly make it visible.
   if ( BP_IsDownloadableVersion() )
   {
      SPR_OBJ *pGameSelect = L2D_GetObject( work->l2d_handle, CODE_MENUGAMESELECT );

      SPR_HIDE( pGameSelect );
   }

}

static void Die(GameselWork *work)
{
	if(work->parent_name!=0 &&
	   work->parent_name!=1){

		GV_MSG msg;
		int message[2];

		msg.address=work->parent_name;
		msg.message=message;
		msg.message_len=sizeof(message)/sizeof(message[0]);

		message[0]=work->ans;
		message[1]=work->menu_cursor;

		GV_SendMessage(&msg);
	}

#if 0
	if(work->proc!=0 && work->proc!=1 &&
	   work->ans==TITLE_MSG_MODEEND){

		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=PROC_COM_GAMESEL;
		argv[1]=work->menu_cursor;

		GCL_ExecProc(work->proc,&arg);
	}
#endif

}


/* ------------------------------------------------------------------------ */

static void InitMenu(GameselWork *work)
{
   int i;
   static int skMenuItemStrCodes[N_MAX_MENU_ITEMS] =
   {
      CODE_MENUNEW, CODE_MENULOAD, CODE_MENUOPT, CODE_MENUSP, CODE_MENUMIS, CODE_MENUSNA, CODE_MENUXFAR, CODE_MENUGAMESELECT
   };

   work->cursor=L2D_GetObject(work->l2d_handle,CODE_SELCURSNULL);
	work->cursor_spr=L2D_GetObject(work->l2d_handle,CODE_SELCURS);

   for (i = 0; i < N_MENU_ITEMS(); ++i)
   {
      work->menu[i] = L2D_GetObject(work->l2d_handle, skMenuItemStrCodes[i]);
   }

#if BP_TGS_DEMO()
   //BP JG - force the cursor to start on an available option.
   work->menu_cursor = P_NEWGAME;
#endif
}


/* 初期化部メイン */
void *NewGameSelScr(int parent_name,int l2d_handle,int proc,int start_mode,int cursor,int bgname)
{
    GameselWork *work ;

    work=(GameselWork *)GV_NewActor(GV_ACTOR_USER,sizeof(GameselWork));

    if(work!=NULL) {
		work->parent_name=parent_name;
		work->proc=proc;
		work->l2d_handle=l2d_handle;
		work->bgname=bgname;
		if(start_mode){
			printf("Mode 1\n");
			work->action_strcode=CODE_BACKMAIN;
		}
		else{
			printf("Mode 0\n");
			work->action_strcode=CODE_SHOWMAIN;
			BgscrOpenGrd(work);
			BgscrOpenPlmap(work);
		}

		work->busy_flag=0;

		work->step=0;
		work->sub_step=0;
		work->ans=0;

		work->menu_cursor=cursor;

#ifdef EU_TRIAL
		/* LOAD GAMEを飛ばす */
		work->menu_cursor=0;
#endif

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		InitMenu(work);
		HideCursor(work);

		BgscrFadein(work);

    }
    return (void *)work ;

}
