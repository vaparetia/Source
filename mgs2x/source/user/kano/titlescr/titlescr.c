//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	titlescr.c
		タイトル画面の表示マネージャー

	2001/06/12 K.Kano
	$Id: titlescr.c,v 1.3 2002/12/11 14:08:18 takaki Exp $
*/
#endif

#include "titlescr.h"

#include "bgscr.h"
#ifdef KP_XBOX
#include "../xmcman/xmcman.h"
#endif


#define TITLE_COUNT		DIRECT_TICK((int)(60.0f*30.0f))		/* 30秒 */

#ifdef KP_WINDOWS
#define TITLE_PAD_START		( PAD_ABXY | PAD_STA | PAD_SEL )
#endif

typedef struct {
	GV_ACT_EX actor;
	int name;

	int busy_flag;
	int busy_flag2;

	int l2d_strcode;
	int l2d_strcode2;
	int l2d_handle;
	int l2d_handle2;
	int action_strcode;
	int action_strcode2;

	int proc;
	int step;
	int face_flag;

	int ans;
	int gcounter;

	int bgname;
} Work;


#if 0

#define DISP_CHANL		TITLE_CHANL
#define DISP_PRIORITY	(TITLE_PRI_NORMAL+1)

#define DISP_CHANL2		TITLE_CHANL
#define DISP_PRIORITY2	TITLE_PRI_NORMAL

#else

#define DISP_CHANL		TITLE_CHANL
#define DISP_PRIORITY	TITLE_PRI_NORMAL

#define DISP_CHANL2		3
#define DISP_PRIORITY2	6

#endif


#define CODE_FACEOPEN			0x009843aa		/* showFace */
#define CODE_FACECLOSE			0x0077ebd3		/* hideFace */

#define CODE_SNAKE_FACEOPEN		0x00deee78		/* showSnake */
#define CODE_SNAKE_FACECLOSE	0x00d3f394		/* hideSnake */
#define CODE_RAIDEN_FACEOPEN	0x0011b387		/* showRaiden */
#define CODE_RAIDEN_FACECLOSE	0x00b25705		/* hideRaiden */

#define CODE_SNAKE_SETFACE		0x006c43f2		/* setSnake */
#define CODE_RAIDEN_SETFACE		0x00bc62b8		/* setRaiden */


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
		break;
	case FRAME_SIGNAL_DOWN:
		message[0]=BG_MSG_DOWN;
		GV_SendMessage(&msg);
		break;
	case FRAME_SIGNAL_LEFT:
		message[0]=BG_MSG_LEFT;
		GV_SendMessage(&msg);
		break;
	case FRAME_SIGNAL_RIGHT:
		message[0]=BG_MSG_RIGHT;
		GV_SendMessage(&msg);
		break;
	case FRAME_SIGNAL_RIGHTUP:
		message[0]=BG_MSG_RIGHTUP;
		GV_SendMessage(&msg);
		break;
	case FRAME_SIGNAL_LEFTDOWN:
		message[0]=BG_MSG_LEFTDOWN;
		GV_SendMessage(&msg);
		break;

	case CODE_FACEOPEN:
		if(work->face_flag) work->action_strcode2=CODE_RAIDEN_FACEOPEN;
		else work->action_strcode2=CODE_SNAKE_FACEOPEN;
		break;
	case CODE_FACECLOSE:
		if(work->face_flag) work->action_strcode2=CODE_RAIDEN_FACECLOSE;
		else work->action_strcode2=CODE_SNAKE_FACECLOSE;
		break;
	}
}

static void BgscrChangeFace(Work *work)
{
	GV_MSG msg;
	int message[3];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_CHGFACE;
	message[1]=work->face_flag;

	GV_SendMessage(&msg);
}

static void BgscrFadein(Work *work)
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

static void BgscrFadein2(Work *work)
{
	GV_MSG msg;
	int message[3];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_FADE;
	message[1]=0;
	message[2]=1;

	GV_SendMessage(&msg);
}

static void BgscrFadeout(Work *work)
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

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=work->ans;

		GCL_ExecProc(work->proc,&arg);
	}
}

static void Step(Work *work)
{
#ifdef KP_XBOX
	// XBOX ではどのパッドでもスタートできなくてはならない
	// 一度決定したら変える必要はない？
	u_short press =
			GV_PadDataDirect[0].press |
			GV_PadDataDirect[1].press |
			GV_PadDataDirect[2].press |
			GV_PadDataDirect[3].press;
			
#else
	u_short press =	GV_PadDataDirect[0].press;
#endif
	
	switch(work->step){
	case 0:
		if(!work->busy_flag){
			if(work->face_flag) work->action_strcode=CODE_SHOWLOGO2;
			else work->action_strcode=CODE_SHOWLOGO;
			work->step++;

			BgscrFadein(work);
			BgscrChangeFace(work);
		}
		break;
	case 1:
		if(work->busy_flag){
#ifndef KP_WINDOWS
			if(press & PAD_STA){
#else
			/* Windows版では、いろんなボタンが効く様にしました */
			if(press & TITLE_PAD_START ){
#endif
				L2D_BreakAction(work->l2d_handle);
				L2D_BreakAction(work->l2d_handle2);

				if(work->face_flag){
					work->action_strcode=CODE_SETLOGO2;
					work->action_strcode2=CODE_RAIDEN_SETFACE;
				}
				else{
					work->action_strcode=CODE_SETLOGO;
					work->action_strcode2=CODE_SNAKE_SETFACE;
				}

				BgscrFadein2(work);

				work->step++;
			}
			break;
		}
		work->step++;

	case 2:
		if(!work->busy_flag){
			if(work->gcounter>TITLE_COUNT){
				if(work->face_flag) work->action_strcode=CODE_HIDELOGO2;
				else work->action_strcode=CODE_HIDELOGO;
				work->step=0x10;

				BgscrFadeout(work);
			}
			else{
				work->action_strcode=CODE_BLINKSTART;
			}
		}
#ifndef KP_WINDOWS
		if( press & PAD_STA){
#else
		/* Windows版では、いろんなボタンが効く様にしました */
		if( press & TITLE_PAD_START ){
#endif
#ifdef KP_XBOX
			{ // ポート入れ替え処理
				int i;
				for( i = 0 ; i < GV_PAD_MAX ; i++ ) {
					if( GV_PadDataDirect[ i ].press & PAD_STA ) {
						// スタートの押されたパッドをプライマリ（０）にする
						GV_ChangePadPortMapping( i, 0 );
					}
				}
			}
			MCX_ResetLoaddataID();	// セーブ位置記憶をリセット
#endif			
			L2D_BreakAction(work->l2d_handle);

			work->ans=PROC_COM_OK;
			if(work->face_flag) work->action_strcode=CODE_HIDELOGO2;
			else work->action_strcode=CODE_HIDELOGO;
			work->step=0x10;

#ifdef TGS2001F
			BgscrFadeout(work);
#endif

			SE_START();
		}
		work->gcounter++;
		break;

	case 0x10:
		if(!work->busy_flag && !work->busy_flag2){
			if(work->l2d_handle>=0){
				L2D_ReleaseLayout(work->l2d_handle);
				work->l2d_handle=-1;
			}

			if(work->l2d_handle2>=0){
				L2D_ReleaseLayout(work->l2d_handle2);
				work->l2d_handle2=-1;
			}

			CallLocalProc(work);
			GV_DestroyActor(work);
		}
		break;
	}
}

static void ProgMessage(Work *work)
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

}

static void Act(Work *work)
{
	ProgMessage(work);

	if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
		work->busy_flag=0;
		if(work->action_strcode!=0){
			L2D_EvokeAction(work->l2d_handle,work->action_strcode);
			work->action_strcode=0;
			work->busy_flag=1;
		}
	}
	else{
		work->busy_flag=1;
	}

	if(work->l2d_handle2>=0){
		if(L2D_ActionStatus(work->l2d_handle2)==L2D_STAT_ACK){
			work->busy_flag2=0;
			if(work->action_strcode2!=0){
				L2D_EvokeAction(work->l2d_handle2,work->action_strcode2);
				work->action_strcode2=0;
				work->busy_flag2=1;
			}
		}
		else{
			work->busy_flag2=1;
		}
	}
	else{
		work->action_strcode2=0;
		work->busy_flag2=0;
	}

	Step(work);
}

static void Die(Work *work)
{
	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}
	if(work->l2d_handle2>=0){
		L2D_ReleaseLayout(work->l2d_handle2);
		work->l2d_handle2=-1;
	}
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->busy_flag=0;
	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->l2d_strcode2=0;
	work->l2d_handle2=-1;
	work->action_strcode=0;
	work->action_strcode2=0;
	work->proc=0;
	work->bgname=0;
	work->face_flag=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
		if(GCL_NextStr()!=NULL){
			work->l2d_strcode2=GCL_GetNextInt();
		}
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('b')!=NULL){
		work->bgname=GCL_GetNextInt();
    }
    if(GCL_GetOption('f')!=NULL){
		work->face_flag=GCL_GetNextInt();
    }
}

static void NullAct(Work *work){ GV_DestroyActor(work); }

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

	work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,DISP_PRIORITY,0);
	if(work->l2d_handle<0){
#ifdef DEBUG
		printf("Title Scr : L2D Init Error = %d\n",work->l2d_handle);
#endif
		GV_ChangeActFunc(&(work->actor),NullAct);
	}
	else{
		/* シグナルハンドラの設定 */
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))SignalFunc);

		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

	work->action_strcode=0;

	if(work->l2d_strcode2!=0){
		work->l2d_handle2=L2D_LoadLayout(work->l2d_strcode2,DISP_CHANL2,DISP_PRIORITY2,0);
		if(work->l2d_handle2<0){
#ifdef DEBUG
			printf("Title Scr : L2D Init Error = %d\n",work->l2d_handle2);
#endif
			GV_ChangeActFunc(&(work->actor),NullAct);
		}
		else{
			/* layoutの初期化 */
			L2D_EvokeAction(work->l2d_handle2,CODE_DEFAULTACTION);
		}
	}

	work->action_strcode2=0;

    return 1;
}

/* 初期化部メイン */
void *NewTitleScrMan(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->step=0;
		work->ans=PROC_COM_CANCEL;

		work->gcounter=0;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL;
		}

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
