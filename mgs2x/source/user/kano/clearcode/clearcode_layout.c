//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	clearcode_layout.c
		クリアコード表示

	2001/08/17  K.Kano
	$Id: clearcode_layout.c,v 1.2 2002/12/04 10:29:00 takaki Exp $
 */


#include "clearcode.h"


enum {
	CMD_END=0,

	CMD_PAD_U,
	CMD_PAD_D,
	CMD_PAD_L,
	CMD_PAD_R,

	CMD_PAD_CIR,
	CMD_PAD_CRS,
	CMD_PAD_TRI,
	CMD_PAD_SQR,

	CMD_PAD_L1,
	CMD_PAD_L2,
	CMD_PAD_L3,
	CMD_PAD_R1,
	CMD_PAD_R2,
	CMD_PAD_R3,
};


static unsigned char konami_command[]={

	CMD_PAD_U,
	CMD_PAD_U,
	CMD_PAD_D,
	CMD_PAD_D,
	CMD_PAD_L,
	CMD_PAD_R,
	CMD_PAD_L,
	CMD_PAD_R,

	CMD_PAD_CRS,
	CMD_PAD_CIR,

	CMD_END,
};


static unsigned char *hidden_commands[]={
	konami_command,
};


#define N_COMMANDS		((int)(sizeof(hidden_commands)/sizeof(hidden_commands[0])))

#define COUNT_LIMIT		60


typedef struct {
	GV_ACT_EX actor;
	int name;

	int end_proc;

	int busy_flag;
	int l2d_handle;
	int action_strcode;

	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int start_mode;

	int timer;

	unsigned int scn_code[SCN_CODE_LEN];
	unsigned char clear_code[MAX_CLR_CODE_LEN];

	SPR_OBJ *string[MAX_CLR_CODE_LEN];
	SPR_POS dot_pos;

	int hidden_step[N_COMMANDS];
	int hidden_count[N_COMMANDS];


	int codename_flag;

	int clr_code_len;	// コード長さ
	int bit_len;		// 入力ビット長さ
} Work;


#define REPEAT_FIRST		20
#define REPEAT_NEXT			1


#define SNAKEVOICE_RESOURCE		0x00384256		/* スネークおまけセリフ */

#define CODE_MASK_TOP			0x00a4123a		/* blk_mask_top */


/* ------------------------------------------------------------------------ */


static void HiddenComAct(Work *work)
{
	int pad=GV_PadDataDirect[0].press;
	int cmd=0;
	int i;

	switch(pad & (PAD_L|PAD_R|PAD_U|PAD_D|
				  PAD_A|PAD_B|PAD_X|PAD_Y|
				  PAD_L1|PAD_L2|PAD_R1|PAD_R2|
				  PAD_AL|PAD_AR)){
	case PAD_L:
		cmd=CMD_PAD_L;
		break;
	case PAD_R:
		cmd=CMD_PAD_R;
		break;
	case PAD_U:
		cmd=CMD_PAD_U;
		break;
	case PAD_D:
		cmd=CMD_PAD_D;
		break;
	case PAD_A:
		cmd=CMD_PAD_CIR;
		break;
	case PAD_B:
		cmd=CMD_PAD_CRS;
		break;
	case PAD_X:
		cmd=CMD_PAD_TRI;
		break;
	case PAD_Y:
		cmd=CMD_PAD_SQR;
		break;
	case PAD_L1:
		cmd=CMD_PAD_L1;
		break;
	case PAD_R1:
		cmd=CMD_PAD_R1;
		break;
	case PAD_L2:
		cmd=CMD_PAD_L2;
		break;
	case PAD_R2:
		cmd=CMD_PAD_R2;
		break;
	case PAD_AL:
		cmd=CMD_PAD_L3;
		break;
	case PAD_AR:
		cmd=CMD_PAD_R3;
		break;

	case 0:
		cmd=0;
		break;
	default:
		cmd=-1;
		break;
	}

	for(i=0;i<N_COMMANDS;i++){
		unsigned char *cmddata=hidden_commands[i];

		if(cmddata[work->hidden_step[i]]==CMD_END){

#ifdef DEBUG
			if(work->hidden_count[i]==0){
				printf("Hidden Command Finished : %d\n",i);
			}
#endif

			work->hidden_count[i]++;
			if(work->hidden_count[i]>=COUNT_LIMIT){
				work->hidden_step[i]=0;
				work->hidden_count[i]=0;
			}
		}
		else if((int)(cmddata[work->hidden_step[i]])==cmd){

#ifdef DEBUG
			printf("Hidden Command Step : %d : %d -> %d\n",
				   i,work->hidden_step[i],work->hidden_step[i]+1);
#endif

			work->hidden_step[i]++;
			work->hidden_count[i]=0;
		}
		else if(cmd==-1){
			work->hidden_step[i]=0;
			work->hidden_count[i]=0;
		}
		else if(work->hidden_step[i]>0){
			work->hidden_count[i]++;
			if(work->hidden_count[i]>=COUNT_LIMIT){
				work->hidden_step[i]=0;
				work->hidden_count[i]=0;
			}
		}
	}
}

static void ResetHiddenCom(Work *work)
{
	int i;
	for(i=0;i<N_COMMANDS;i++){
		work->hidden_step[i]=0;
		work->hidden_count[i]=0;
	}
}

static int CheckHiddenCom(Work *work,int index)
{
	unsigned char *cmddata;
	unsigned char code;

	if(index<0 || index>=N_COMMANDS) return 0;

	cmddata=hidden_commands[index];
	code=cmddata[work->hidden_step[index]];

	if(code==CMD_END){
		work->hidden_step[index]=0;
		work->hidden_count[index]=0;
		return 1;
	}
	else{
		return 0;
	}
}

/* ------------------------------------------------------------------------ */


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
	work->key_press=GV_PadDataDirect[0].press;

	if(GV_PadDataDirect[0].status && GV_PadDataDirect[0].status==work->key_status){
		if(work->key_count>REPEAT_FIRST){
			work->key_count-=REPEAT_NEXT;
			work->key_autostatus=work->key_status;
		}
		else work->key_autostatus=0;

		work->key_count++;
	}
	else{
		work->key_autostatus=work->key_status=GV_PadDataDirect[0].status;
		work->key_count=0;
	}
}

static void Kill(Work *work)
{
	GV_DestroyActor(work);

	if(work->end_proc!=0 && work->end_proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		GCL_ExecProc(work->end_proc,&arg);
	}
}


#define CODE_CODE_FONT		0x008B5C04		/* code_font */

#define NODE_FONT_A_U		18
#define NODE_FONT_A_V		14

#define NODE_FONT_WIDTH		18
#define NODE_FONT_HEIGHT	14

#define NODE_FONT_DW		20
#define NODE_FONT_DH		16

#define NODE_FONT_R			190
#define NODE_FONT_G			24
#define NODE_FONT_B			20


static const int fontx[]={
	37,57,77,97, 127,147,167,187, 217,237,257,277, 307,327,347,367,
};

static const int fonty[]={
//	146+8,166+8,
//	185, 205,
	203, 223,
};

static void SetFont(Work *work)
{
	int i;
	int x,y;

	for(i=0;i<work->clr_code_len;i++){
		SPR_POS pos;
		int num=work->clear_code[i];

		x=fontx[i%16];
		y=fonty[i/16];

#if 0
		if(num==('I'-'A')){
			x+=(NODE_FONT_DW-4)/2;
		}
#endif

		work->string[i]->sprite.head.tex.u+=SPR_FIXED(NODE_FONT_A_U+NODE_FONT_WIDTH*num);
		work->string[i]->sprite.head.tex.v+=SPR_FIXED(NODE_FONT_A_V);
		work->string[i]->sprite.head.tex.w=SPR_FIXED(NODE_FONT_WIDTH);
		work->string[i]->sprite.head.tex.h=SPR_FIXED(NODE_FONT_HEIGHT);

		work->string[i]->sprite.head.tex.pu+=SPR_FIXED(NODE_FONT_A_U+NODE_FONT_WIDTH*num);
		work->string[i]->sprite.head.tex.pv+=SPR_FIXED(NODE_FONT_A_V);
		work->string[i]->sprite.head.tex.pw=SPR_FIXED(NODE_FONT_WIDTH);
		work->string[i]->sprite.head.tex.ph=SPR_FIXED(NODE_FONT_HEIGHT);

		work->string[i]->sprite.dw=NODE_FONT_DW;
		work->string[i]->sprite.dh=NODE_FONT_DH;

		pos.x=(float)x;
		pos.y=(float)y;

		SPR_SetPosSprite(work->string[i],&pos);

		work->string[i]->sprite.col.r=NODE_FONT_R;
		work->string[i]->sprite.col.g=NODE_FONT_G;
		work->string[i]->sprite.col.b=NODE_FONT_B;

		SPR_HIDE(work->string[i]);
	}

	x=fontx[i%16];
	y=fonty[i/16];

   //BP JG - move the dot after the picture.. the dot used to be after the secret code.
	work->dot_pos.x=103; //(float)(x+(NODE_FONT_DW-12)/2);
	work->dot_pos.y=164; //(float)(y+NODE_FONT_DH-8);
}

static int AppearFont(Work *work)
{
	int num=DIRECT_TICK(work->timer)/8;
	int i;
	int limit;

	limit=(work->clr_code_len-(work->clr_code_len%4));

	// if(num>limit) num-=3;

	if(num>work->clr_code_len) return 1;

	if((DIRECT_TICK(work->timer-1) % 8)!=0 && (DIRECT_TICK(work->timer) % 8)==0) SE_DISP_A_FONT();

	for(i=0;i<num;i++){
		SPR_SHOW(work->string[i]);
	}

	work->timer++;

	return 0;
}

static int InitSubStep(Work *work)
{
	SPR_OBJ *font;
	SPR_OBJ *mask;
	int i;

	if(work->busy_flag) return 0;

	font=L2D_GetObject(work->l2d_handle,CODE_CODE_FONT);

	for(i=0;i<work->clr_code_len;i++){
		work->string[i]=SPR_DuplicateTree(font);
		if(work->string[i]==NULL){
			Kill(work);
			return -1;
		}
	}

	SetFont(work);

	mask=L2D_GetObject(work->l2d_handle,CODE_MASK_TOP);
	if(mask!=NULL) mask->sprite.col.a=0;

	work->timer=0;

	return 1;
}

static int WaitSubStep(Work *work,int count)
{
	if(work->timer>=count) return 1;
	work->timer++;
	return 0;
}

static int OpenSubStep(Work *work)
{
	static const int strcode[]={
		CODE_OPEN_TNK_CODE_WIN,
		CODE_OPEN_PLT_CODE_WIN,
		CODE_OPEN_TP_CODE_WIN,
	};

	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;
		// work->action_strcode=CODE_TYPETEST;
		work->sub_step++;
		break;
	case 1:
#ifdef BP_SHOW_COMPLETION_CODE
		if(AppearFont(work))
#endif
      {
			work->sub_step++;
			work->timer=0;
		}
		break;
	case 2:
		if(WaitSubStep(work,DIRECT_TICK(0.5f*60.0f))){
			SE_DISP_FINISH();
			work->sub_step++;
		}
		break;
	case 3:
		if(work->busy_flag) break;
		work->action_strcode=strcode[work->start_mode];
		work->sub_step++;
		break;
	case 4:
		if(work->busy_flag) break;
		return 1;
	}
	return 0;
}

static int IdleSubStep(Work *work)
{
	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,CODE_NODE_DOT);
	if(spr!=NULL){
		SPR_SetPosEmpty(spr,&(work->dot_pos));
	}

	if(GV_PadDataDirect[0].press & PAD_OK) return 1;

	if(work->busy_flag) return 0;

	// work->action_strcode=CODE_TESTBLINK;
	work->action_strcode=CODE_BLINKDOT;

	return 0;
}

static int CloseSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		work->action_strcode=CODE_CLOSE_CODE_WIN;
		work->sub_step++;

		if(CheckHiddenCom(work,0)){
			int *voice_le;
			int i=(irnd()>>16) % 3;
			int handle;

#ifdef DEBUG_MODE
			printf("Command Success\n");
#endif

			voice_le=(int *)GetLocalResource(SNAKEVOICE_RESOURCE,i);

			handle=GM_VoxStream(GCL_GetLong( voice_le ),0);
			if(handle>=0){
#ifdef DEBUG_MODE
				printf("Voice Success\n");
#endif
				GM_VoxStreamSetPan(handle,255,64,0.f);
			}
		}
		break;
	case 1:
		if(work->busy_flag) break;
		return 1;
	}
	return 0;
}

static void ReceiveSignal( void *pw, int signal, int value )
{
	Work *work = pw;
	switch( signal ){
	  case 0x01:
		work->codename_flag = 1;
		break;
	  default:
		break;
	}
		
}


static void Step(Work *work)
{
	switch(work->step){
	case 0:
		if(InitSubStep(work))
      {
			/*** サブスタンスTODO ***/
			/* 
			   ここからNewShowCodeName() を呼び出して子アクターにする。
			   コードネームを表示し終わったらシグナルで知らせてあとのルーチンに移行する
			 */

			extern void *NewShowCodeName( void );
			void *ptr;

			ptr = NewShowCodeName();
			if( ptr != NULL )
         {
				GV_SetActorChild( work, ptr );
				GV_SetActorSignalFunc( work, ReceiveSignal );
				work->step = 6;/* コードネーム表示が終わるまで待つフェーズ */
			} 
         else 
         {
				/* コードネーム表示が失敗したら、もとのクリアコード表示に移る */
				work->step = 1;
				work->sub_step=0;
				work->timer=0;
			}
      }
		break;
	case 1:
		if(OpenSubStep(work))
      {
			work->step++;
			work->sub_step=0;
			work->timer=0;
		}
		break;
	case 2:
		if(WaitSubStep(work,DIRECT_TICK(0.5f*60.0f)))
      {
			work->step++;
			work->sub_step=0;
			work->timer=0;
		}
		break;
	case 3:
		if(IdleSubStep(work)){
			work->step++;
			work->sub_step=0;
			work->timer=0;
		}
		break;
	case 4:
		if(CloseSubStep(work)){
			work->step++;
			work->sub_step=0;
			work->timer=0;
		}
		break;
	case 5:
		Kill(work);
		break;

	  case 6:
		/* コードネーム表示が終わるまで待つ */
		if( work->codename_flag == 1 ){
			work->step = 1;
			work->sub_step=0;
			work->timer=0;
		}			
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
				int stat;

				stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
				// printf("L2D Stat = %d\n",stat);
#endif

				work->action_strcode=0;
				work->busy_flag=1;
			}
		}
		else{
			work->busy_flag=1;
		}

		// KeyAnim(work);
	}
	else{
		work->busy_flag=0;
	}


	ProgMessage(work);

	Key(work);
	Step(work);

	HiddenComAct(work);
}

static void Die(Work *work)
{
	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	int i;

	work->l2d_handle=-1;
	work->end_proc=0;
	work->start_mode=0;

	for(i=0;i<SCN_CODE_LEN;i++){
		work->scn_code[i]=0;
	}

    if(GCL_GetOption('p')!=NULL){
		work->end_proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('m')!=NULL){
		work->start_mode=GCL_GetNextInt();
    }
	if(GCL_GetOption('c')!=NULL){
		work->scn_code[0]=GCL_GetNextInt();
		work->scn_code[1]=GCL_GetNextInt();
		work->scn_code[2]=GCL_GetNextInt();
		work->scn_code[3]=GCL_GetNextInt();
	}

	// 新仕様 ビット数と希望文字数可変
	if( GCL_GetOption('b') != NULL ) {
		work->bit_len = GCL_GetNextInt();
	} else {
		work->bit_len = 128;
	}
	if( GCL_GetOption('s') != NULL ) {
		work->clr_code_len = GCL_GetNextInt();
	} else {
		work->clr_code_len = 0;
	}
					
	

	for(i=0;i<MAX_CLR_CODE_LEN;i++){
		work->clear_code[i]=i;
	}
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

	ResetHiddenCom(work);

	work->l2d_handle=L2D_LoadLayout(CODE_L2D_CLEAR_CODE,DISP_CHANL,DISP_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG_MODE
		printf("Select Scr : L2D Init Error = %d\n",work->l2d_handle);
#endif

		ASSERT(0);
	}
	else{
		/* シグナルハンドラの設定 */
		// L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);

		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

    return 1;
}


/* 初期化部メイン */
void *NewClearCodeScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->busy_flag=0;
		work->action_strcode=0;

		work->step=0;
		work->sub_step=0;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL ;
		}

#if 0
		/* クリアコードの計算 */
		EncodeClearCode(work->scn_code);

		// printf("%08x %08x %08x %08x\n",
		//	   work->scn_code[0],work->scn_code[1],work->scn_code[2],work->scn_code[3]);

		CalcBitTo26Code(work->scn_code,work->clear_code);
#else	// 新エンコーダ 2002/08/07 M.Kobayashi
		
#ifndef KP_WINDOWS
		work->clr_code_len = ClearCodeGenerate( work->clear_code, work->clr_code_len, 26,
												work->scn_code, work->bit_len, irnd() );
#else
		work->clr_code_len = ClearCodeGenerate( CLEARCODE_MODE_SONSOFLIBERTY,
												work->clear_code, work->clr_code_len, 26,
												work->scn_code, work->bit_len, irnd() );
#endif
#endif		

#if 0
		{
			int i;
			for(i=0;i<CLR_CODE_LEN;i++){
				printf("%02x ",work->clear_code[i]);
			}
			printf("\n");
		}
#endif

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }

    return (void *)work ;
}
