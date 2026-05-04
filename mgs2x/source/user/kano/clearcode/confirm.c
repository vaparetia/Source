//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	confirm.c
		セーブ確認画面

	2001/08/17  K.Kano
	$Id: confirm.c,v 1.1.1.3 2002/11/19 11:43:07 Yoshizawa1 Exp $
 */


#include "clearcode.h"

#include "font.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "BP_LocalizedTextByEnum.h"


#define	LOAD_CODE_J		0x230692
#define LOAD_CODE_E		0x9e5a6b
#define LOAD_CODE_F		0x9f7f6b
#define LOAD_CODE_G		0x8f2b68
#define LOAD_CODE_S		0x9463a6
#define LOAD_CODE_I		0xde18ea

static const int res_code[]={
	LOAD_CODE_E,
	LOAD_CODE_F,
	LOAD_CODE_G,
	LOAD_CODE_I,
	LOAD_CODE_S,
	0, //韓国語
	LOAD_CODE_J,
};


#define YESNO_STR_LEN		8

#ifdef KP_XBOX
#undef PAL
#endif


typedef struct {
	GV_ACT_EX actor;
	int name;

	int end_proc;

	int busy_flag;
	int l2d_handle;
	int action_strcode;

	int yesno_cursor;

   int yesLen;
   int noLen;

	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	void *strman;
	int str_strcode;

	int base_u,base_v;
	SPR_OBJ *yesno_str[YESNO_STR_LEN];

	int fade_count;
	int fade_alpha,fade_talpha;

#ifdef PAL
	int yes_width;
	int slash_width;
	int no_width;
#endif

} Work;


#define REPEAT_FIRST		20
#define REPEAT_NEXT			1


#define FONT_WIDTH				FONT_SIZE_W
#define FONT_HEIGHT				FONT_SIZE_H
#define FONT_DISP_WIDTH			16
#define FONT_DISP_HEIGHT		16

#define LINE_SPACE				RUBI_SIZE_H
#define LINE_HEIGHT				(FONT_HEIGHT+LINE_SPACE+DOWN_MARGINE)
#define LINE_DISP_HEIGHT		(LINE_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define TEXT_X			50
#define TEXT_Y			315
#define TEXT_W			(FONT_DISP_WIDTH*30)
#define TEXT_H			(LINE_DISP_HEIGHT*1)

#define TEXT_U			0
#define TEXT_V			0
#define TEXT_WIDTH		(FONT_WIDTH*30)
#define TEXT_HEIGHT		(LINE_HEIGHT*1)

#define YES_U			0
#define YES_V			TEXT_HEIGHT
#define YES_WIDTH		(FONT_WIDTH*6)
#define YES_HEIGHT		(LINE_HEIGHT*1)

#define YES_W			(FONT_DISP_WIDTH*6)
#define YES_H			(LINE_DISP_HEIGHT*1)

#define SLASH_U			(YES_U+YES_WIDTH)
#define SLASH_V			TEXT_HEIGHT
#define SLASH_WIDTH		(FONT_WIDTH*3)
#define SLASH_HEIGHT	(LINE_HEIGHT*1)

#define SLASH_W			(FONT_DISP_WIDTH*3)
#define SLASH_H			(LINE_DISP_HEIGHT*1)

#define NO_U			(SLASH_U+SLASH_WIDTH)
#define NO_V			TEXT_HEIGHT
#define NO_WIDTH		(FONT_WIDTH*6)
#define NO_HEIGHT		(LINE_HEIGHT*1)

#define NO_W			(FONT_DISP_WIDTH*6)
#define NO_H			(LINE_DISP_HEIGHT*1)

#define YESNO_STR_RX	492


#define TEXT_R				(160/2)
#define TEXT_G				(180/2)
#define TEXT_B				(170/2)

#define TEXTCOLOR			(TEXT_R | (TEXT_G<<8) | (TEXT_B<<16))


#define YESNO_STR_X			384
#define YESNO_STR_Y			320

#define	STR_WIDTH			(18) // strcode ではなく文字の長さ
#define STR_HEIGHT			(14) // strcode ではなく文字の長さ

#define SK_FONT_WIDTH		18.0f
#define SK_FONT_HEIGHT		14.0f

#define SEL_ALPHA		128
#define UNSEL_ALPHA		52

#define FADE_COUNT		DIRECT_TICK(20)


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
	int status=GV_PadDataDirect[0].status;
	int press=GV_PadDataDirect[0].press;

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


static void PutString(Work *work)
{
	MENU_PutTextScreen(work->strman,
					   TEXT_X,TEXT_Y,TEXT_X+TEXT_W,TEXT_Y+TEXT_H,
					   TEXT_U,TEXT_V,TEXT_U+TEXT_WIDTH,TEXT_V+TEXT_HEIGHT,
					   TEXTCOLOR | (work->fade_alpha<<24));

#ifdef PAL
	{
		int yes_x,slash_x,no_x;
		int yescol,nocol,slashcol;

		if(work->yesno_cursor){
			yescol=TEXTCOLOR|(((UNSEL_ALPHA*work->fade_alpha)>>7)<<24);
			nocol=TEXTCOLOR|(((SEL_ALPHA*work->fade_alpha)>>7)<<24);
		}
		else{
			yescol=TEXTCOLOR|(((SEL_ALPHA*work->fade_alpha)>>7)<<24);
			nocol=TEXTCOLOR|(((UNSEL_ALPHA*work->fade_alpha)>>7)<<24);
		}
		slashcol=TEXTCOLOR|(work->fade_alpha<<24);

		no_x=YESNO_STR_RX-work->no_width*FONT_DISP_WIDTH/FONT_WIDTH;
		slash_x=no_x-work->slash_width*FONT_DISP_WIDTH/FONT_WIDTH;
		yes_x=slash_x-work->yes_width*FONT_DISP_WIDTH/FONT_WIDTH;

		MENU_PutTextScreen(work->strman,yes_x,TEXT_Y,
						   yes_x+YES_W,TEXT_Y+YES_H,
						   YES_U,YES_V,
						   YES_U+YES_WIDTH,YES_V+YES_HEIGHT,
						   yescol);

		MENU_PutTextScreen(work->strman,slash_x,TEXT_Y,
						   slash_x+SLASH_W,TEXT_Y+SLASH_H,
						   SLASH_U,SLASH_V,
						   SLASH_U+SLASH_WIDTH,SLASH_V+SLASH_HEIGHT,
						   slashcol);

		MENU_PutTextScreen(work->strman,no_x,TEXT_Y,
						   no_x+NO_W,TEXT_Y+NO_H,
						   NO_U,NO_V,
						   NO_U+NO_WIDTH,NO_V+NO_HEIGHT,
						   nocol);
	}
#endif

}

static void SetYesNo(Work *work)
{
   char strYes[8];
   char strNo[8];
	char str[YESNO_STR_LEN];
	int i;

	strcpy(str,"YES/NO");


   strcpy(strYes,BP_GetStringForEnum(kBP_GLS_Yes));
   strcpy(strNo,BP_GetStringForEnum(kBP_GLS_No));

   work->yesLen = strlen(strYes);
   work->noLen = strlen(strNo);

   sprintf(str,"%s/%s",strYes,strNo);

	SK_PrintfNormal(NULL,work->yesno_str,work->base_u,work->base_v,STR_WIDTH,STR_HEIGHT,YESNO_STR_LEN);

	for(i=0;i<YESNO_STR_LEN;i++)
   {
		SPR_OBJ *spr=work->yesno_str[i];

		spr->sprite.pos.x=YESNO_STR_X;
		spr->sprite.pos.y=YESNO_STR_Y;
		spr->sprite.dw=SK_FONT_WIDTH;
		spr->sprite.dh=SK_FONT_HEIGHT;
	}

	SK_PrintfNormal(str,work->yesno_str,work->base_u,work->base_v,STR_WIDTH,STR_HEIGHT,YESNO_STR_LEN);
	SK_PrintfChengColor2(work->yesno_str,TEXT_R,TEXT_G,TEXT_B,UNSEL_ALPHA,YESNO_STR_LEN);
	SK_AllShow(work->yesno_str,YESNO_STR_LEN);
}

static void CurMoveYesNo(Work *work)
{
	int i;
	if(work->yesno_cursor){
		for(i=0;i<work->yesLen;i++){
			SPR_OBJ *spr=work->yesno_str[i];
			spr->sprite.col.a=UNSEL_ALPHA;
#ifdef KP_XBOX
			spr->sprite.col.r = TEXT_R;
			spr->sprite.col.g = TEXT_G;
			spr->sprite.col.b = TEXT_B;
#endif			
		}
		for(i=0;i<work->noLen;i++){
			SPR_OBJ *spr=work->yesno_str[i+(work->yesLen+1)];
			spr->sprite.col.a=SEL_ALPHA;
#ifdef KP_XBOX
			spr->sprite.col.r = GM_FOCUS_COLOR_R;
			spr->sprite.col.g = GM_FOCUS_COLOR_G;
			spr->sprite.col.b = GM_FOCUS_COLOR_B;
#endif			
		}
	}
	else{
		for(i=0;i<work->yesLen;i++){
			SPR_OBJ *spr=work->yesno_str[i];
			spr->sprite.col.a=SEL_ALPHA;
#ifdef KP_XBOX
			spr->sprite.col.r = GM_FOCUS_COLOR_R;
			spr->sprite.col.g = GM_FOCUS_COLOR_G;
			spr->sprite.col.b = GM_FOCUS_COLOR_B;
#endif			
		}
		for(i=0;i<work->noLen;i++){
			SPR_OBJ *spr=work->yesno_str[i+(work->yesLen+1)];
			spr->sprite.col.a=UNSEL_ALPHA;
#ifdef KP_XBOX
			spr->sprite.col.r = TEXT_R;
			spr->sprite.col.g = TEXT_G;
			spr->sprite.col.b = TEXT_B;
#endif			
		}
	}
}

static void HideYesNo(Work *work)
{
	SK_AllHide(work->yesno_str,YESNO_STR_LEN);
}


static void InitSpr(Work *work);

static int InitSubStep(Work *work)
{
	char *str;

	if(work->busy_flag) return 0;

	str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(work->str_strcode,0));

	MENU_ClearTextTexture(work->strman);

	MENU_CreateTextTexture(work->strman,TEXT_U,TEXT_V,TEXT_WIDTH,TEXT_HEIGHT,0,0,0,str);

#ifdef PAL
	str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(work->str_strcode,1));
	work->yes_width=MENU_CreateTextTexture(work->strman,YES_U,YES_V,
										   YES_WIDTH,YES_HEIGHT,0,0,0,str)-YES_U;

	work->slash_width=MENU_CreateTextTexture(work->strman,SLASH_U,SLASH_V,
											 SLASH_WIDTH,SLASH_HEIGHT,0,0,0," / ")-SLASH_U;

	str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(work->str_strcode,2));
	work->no_width=MENU_CreateTextTexture(work->strman,NO_U,NO_V,
										  NO_WIDTH,NO_HEIGHT,0,0,0,str)-NO_U;
#endif

	InitSpr(work);

	return 1;
}

static int OpenSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		work->action_strcode=CODE_OPEN_SAVEMSG;
		work->sub_step++;

#ifndef PAL
		SetYesNo(work);
#endif

		work->fade_talpha=128;
		work->fade_count=FADE_COUNT;

		SE_WINOPEN();
		break;
	case 1:
		if(work->busy_flag || work->fade_count>0) break;
		return 1;
	}
	return 0;
}

static int CloseSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		work->fade_talpha=0;
		work->fade_count=FADE_COUNT;

		work->sub_step++;
		// SE_WINCLOSE();
		break;
	case 1:
		if(work->fade_count>0) break;

		work->action_strcode=CODE_CLOSE_SAVEMSG;
		work->sub_step++;

#ifndef PAL
		HideYesNo(work);
#endif
		break;
	case 2:
		if(work->busy_flag) break;
		return 1;
	}
	return 0;
}

static void Kill(Work *work)
{
	GV_DestroyActor(work);

	if(work->strman!=NULL){
		GV_DestroyOtherActor(work->strman);
		work->strman=NULL;
	}

	if(work->end_proc!=0 && work->end_proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=work->yesno_cursor;
		GCL_ExecProc(work->end_proc,&arg);
	}
}

static void Step(Work *work)
{
	switch(work->step){
	case 0:
		if(InitSubStep(work)){
			work->step++;
			work->sub_step=0;
		}
		break;
	case 1:
		PutString(work);

		if(OpenSubStep(work)){
			work->step++;
			work->sub_step=0;
		}
		break;
	case 2:
		PutString(work);

		if(work->key_press & PAD_OK){
			work->step++;
			work->sub_step=0;

			if(work->yesno_cursor) SE_CANCEL();
			else SE_OK();
			return;
		}

		switch(work->key_press & (PAD_L|PAD_R)){
		case PAD_L:
		case PAD_R:
			work->yesno_cursor^=1;
			SE_SEL();
			break;
		}

#ifndef PAL
		CurMoveYesNo(work);
#endif

		break;
	case 3:
		PutString(work);

		if(CloseSubStep(work)){
			work->step++;
			work->sub_step=0;
		}
		break;
	case 4:
		Kill(work);
		break;
	}

	if(work->fade_count>0){
		work->fade_alpha+=(work->fade_talpha-work->fade_alpha)/work->fade_count;
		work->fade_count--;
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

		// KeyAnim(work);
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
	if(work->strman!=NULL){
		GV_DestroyOtherActor(work->strman);
		work->strman=NULL;
	}
	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}
}


/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_SHOW_ITEM_NAME:
		// SetFont(work);
		break;
	case CODE_HIDE_ITEM_NAME:
		// HideFont(work);
		break;
	}
}

/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_handle=-1;
	work->end_proc=0;

    if(GCL_GetOption('p')!=NULL){
		work->end_proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('s')!=NULL){
		work->str_strcode=GCL_GetNextInt();
    }
#if 1	// ADD M.Kobayashi 2002/07/11	
	else {
		work->str_strcode = res_code[ GM_Language - GM_LANG_ENGLISH ] ;
	}
#endif	
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);


	work->l2d_handle=L2D_LoadLayout(CODE_L2D_CLEAR_CODE,DISP_CHANL,DISP_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG_MODE
		printf("GetNewItem : L2D Init Error = %d\n",work->l2d_handle);
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

static void InitSpr(Work *work)
{
	int i;
	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,CODE_NODE_FONT);
	if(spr==NULL) ASSERT(0);

#if 0 //BP_PS2 def PSX2
	work->base_u=spr->sprite.head.tex.u;
	work->base_v=spr->sprite.head.tex.v;
#else
	work->base_u= (int)( spr->sprite.head.tex.u * 16 );
	work->base_v= (int)( spr->sprite.head.tex.v * 16 );
#endif

	SPR_SetPriority(spr,7);

	for(i=0;i<YESNO_STR_LEN;i++){
		work->yesno_str[i]=SPR_DuplicateTree(spr);
	}

	HideYesNo(work);
}

static void InitMenu(Work *work)
{
	work->step=0;
	work->sub_step=0;

	work->yesno_cursor=0;

	work->fade_alpha=work->fade_talpha=0;
	work->fade_count=0;
}

/* 初期化部メイン */
void *NewConfirmSaveScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->busy_flag=0;
		work->action_strcode=0;

		work->strman=NULL;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL ;
		}
		if((work->strman=NewTextScreenControl())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		InitMenu(work);

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}
