//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tr_option_frame.c
		体験版用オプションフレーム

	2003/06/20 Y.Yano
	$Id: tr_option_frame.c,v 1.1.1.3 2002/11/19 11:43:42 Yoshizawa1 Exp $
*/


#include "../../kano/titlescr/titlescr.h"

//#include "bgscr.h"


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

	//int bgname;
	SPR_OBJ *root;
	int 	cnt;
	int 	sd_flag;

	int frame_mode;
} Work;


static Work *node_frame_work=NULL;



#define OPENCODE   863270 //openFrame
#define CLOSECODE  6956960 //closeFrame
#define OPENFLAG   4557550 //openSlant3
#define OPENFLAG1  4557551 //openSlant4
#define CLOSEFLAG  15006263 //closeSlant1
#define CLOSEFLAG1 15006264 //closeSlant2


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


#if 0
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
#endif

#if 0
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
#endif
#if 0
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
#endif


int node_frame_flag;

static void Act(Work *work)
{
	//ProgMessage(work);
	int stat;

	if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
		//if(work->busy_flag) BGMsg_Stop(work);

		work->busy_flag=0;
		if(work->action_strcode!=0){

			// SD
			if( work->action_strcode == OPENCODE ){
				work->sd_flag = 1;
				GM_SdSet( SD_S_LINEMOV1 );
			} else if( work->action_strcode == CLOSECODE ){
				GM_SdSet( SD_S_WINCLS01 );
				work->sd_flag = 2;
			} else {
				printf("frame sd err\n");
				ASSERT( 0 );
			}

			stat = L2D_EvokeAction(work->l2d_handle,work->action_strcode);
			printf("action[%d] stat[%d]\n",work->action_strcode, stat );
			//CallFrameSe(work->action_strcode);
			work->action_strcode=0;
			work->busy_flag=1;
			work->cnt = 0;
		}
	}
	else{
		SPR_SHOW( work->root );

		if( work->cnt == 10 ){
			// SD
			if( work->sd_flag == 1 ){
				GM_SdSet( SD_S_WINOPN01 );
			} else if( work->sd_flag == 2 ){
				GM_SdSet( SD_S_LINEMOV1 );
			} else {
				printf("frame sd err\n");
				ASSERT( 0 );
			}
		}
	
		work->busy_flag=1;
		work->cnt ++;
	}

	node_frame_flag = work->busy_flag;

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
	//work->bgname=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
	/*
	   if(GCL_GetOption('b')!=NULL){
	   work->bgname=GCL_GetNextInt();
	   }
	*/
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	SPR_OBJ *spr;

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
		//L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))SignalFunc);

		printf("l2d handle = %d\n",work->l2d_handle );
		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

	spr = L2D_GetObject( work->l2d_handle, 2770484/* root */ );
	if ( spr == NULL ){
		printf("root strcode違うよ!!!!\n");
		return -1 ;
	}
	work->root = spr;
	SPR_HIDE( spr );

	work->frame_mode=0;

    return 1;
}

/* 初期化部メイン */
void *NewNodeFrameTrial(int name,int where)
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

void ComNodeFrameActionTrial(int action_strcode)
{
	Work *work=node_frame_work;

	if(work==NULL) return;

	if( action_strcode == OPENFLAG || action_strcode == OPENFLAG1 ){		
		work->action_strcode = OPENCODE;
		printf("open sign\n");
	} else if( action_strcode == CLOSEFLAG || action_strcode == CLOSEFLAG1 ){
		work->action_strcode = CLOSECODE;
		printf("close sign\n");
	} else {
		printf("FRAME err!!(bad strcode)[%d]\n", action_strcode);
	}
}

int ComNodeFrameBusyTrial(void)
{
	Work *work=node_frame_work;

	if(work==NULL) return 0;

	return work->busy_flag;
}
