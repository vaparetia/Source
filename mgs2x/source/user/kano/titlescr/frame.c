//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	frame.c
		フレームの制御

	2001/06/12 K.Kano
	$Id: frame.c,v 1.1.1.3 2002/11/19 11:43:37 Yoshizawa1 Exp $
*/


#include "titlescr.h"

#include "bgscr.h"


#define DISP_CHANL			TITLE_CHANL
#define FRAME_PRIORITY		TITLE_PRI_FRAME
#define MENU_PRIORITY		TITLE_PRI_NORMAL


typedef struct {
	GV_ACT_EX actor;
	int name;

	int l2d_strcode;
	int l2d_handle;
	int action_strcode;
	int busy_flag;

	int bgname;

	int frame_mode;
} Work;


static Work *node_frame_work=NULL;


#define CODE_OPEN1			0x00458aec		/* openSlant1 */
#define CODE_OPEN2			0x00458aed		/* openSlant2 */
#define CODE_OPEN3			0x00458aee		/* openSlant3 */
#define CODE_OPEN4			0x00458aef		/* openSlant4 */

#define CODE_CLOSE1			0x00e4fa37		/* closeSlant1 */
#define CODE_CLOSE2			0x00e4fa38		/* closeSlant2 */

#define CODE_CLOSE_T		0x004873ec		/* closeToTop */
#define CODE_CLOSE_R		0x00ac9a95		/* closeToRight */
#define CODE_CLOSE_B		0x00482c89		/* closeToBtm */
#define CODE_CLOSE_L		0x000a54bd		/* closeToLeft */

#define CODE_OPEN_D			0x0007601f		/* openDown */
#define CODE_CLOSE_D		0x00da47fa		/* closeDown */


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void SignalFunc(Work *work,int sign,int value)
{
	GV_MSG msg;
	int message[1];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	switch(sign){
	case FRAME_SIGNAL_UP:
		message[0]=BG_MSG_UP;
		GV_SendMessage(&msg);
		work->frame_mode=1;
		break;
	case FRAME_SIGNAL_DOWN:
		message[0]=BG_MSG_DOWN;
		GV_SendMessage(&msg);
		work->frame_mode=1;
		break;
	case FRAME_SIGNAL_LEFT:
		message[0]=BG_MSG_LEFT;
		GV_SendMessage(&msg);
		work->frame_mode=1;
		break;
	case FRAME_SIGNAL_RIGHT:
		message[0]=BG_MSG_RIGHT;
		GV_SendMessage(&msg);
		work->frame_mode=1;
		break;
	case FRAME_SIGNAL_RIGHTUP:
		message[0]=BG_MSG_RIGHTUP;
		GV_SendMessage(&msg);
		work->frame_mode=1;
		break;
	case FRAME_SIGNAL_LEFTDOWN:
		message[0]=BG_MSG_LEFTDOWN;
		GV_SendMessage(&msg);
		work->frame_mode=1;
		break;
	}
}

static void BGMsg_Stop(Work *work)
{
	GV_MSG msg;
	int message[1];

	if(work->bgname==0 || work->bgname==1) return;
	if(!work->frame_mode) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);
	message[0]=BG_MSG_STOP;

	GV_SendMessage(&msg);

	work->frame_mode=0;
}

static void ProgMessage(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);

		if(command>0){
			work->action_strcode=command;
		}

		msg++;
		n_msg--;
    }
}

static void CallFrameSe(int strcode)
{
	switch(strcode){
	case CODE_OPEN1:
	case CODE_OPEN2:
	case CODE_OPEN_D:
		SE_WINOPENR2L();
		break;

	case CODE_OPEN3:
	case CODE_OPEN4:
		SE_WINOPENL2R();
		break;

	case CODE_CLOSE2:
	case CODE_CLOSE_L:
		SE_WINCLOSER2L();
		break;

	case CODE_CLOSE1:
	case CODE_CLOSE_R:
	case CODE_CLOSE_D:
		SE_WINCLOSEL2R();
		break;

	case CODE_CLOSE_T:
	case CODE_CLOSE_B:
		SE_WINCLOSE();
		break;
	}
}


static void Act(Work *work)
{
	ProgMessage(work);

	if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
		if(work->busy_flag) BGMsg_Stop(work);

		work->busy_flag=0;
		if(work->action_strcode!=0){
			L2D_EvokeAction(work->l2d_handle,work->action_strcode);
			CallFrameSe(work->action_strcode);
			work->action_strcode=0;
			work->busy_flag=1;
		}
	}
	else{
		work->busy_flag=1;
	}


}

static void Die(Work *work)
{
	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}

	node_frame_work=NULL;
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->action_strcode=0;
	work->busy_flag=0;
	work->bgname=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('b')!=NULL){
		work->bgname=GCL_GetNextInt();
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);


	work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,FRAME_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG_MODE
		printf("Select Scr : Frame L2D Init Error = %d\n",work->l2d_handle);
#endif
		ASSERT(0);
	}
	else{
		/* シグナルハンドラの設定 */
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))SignalFunc);

		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

	work->frame_mode=0;

    return 1;
}

/* 初期化部メイン */
void *NewNodeFrame(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}

		node_frame_work=work;
    }
    return (void *)work ;
}


/* ------------------------------------------------------------------------ */


void ComNodeFrameAction(int action_strcode)
{
	Work *work=node_frame_work;

	if(work==NULL) return;

	work->action_strcode=action_strcode;
}

int ComNodeFrameBusy(void)
{
	Work *work=node_frame_work;

	if(work==NULL) return 0;

	return work->busy_flag;
}
