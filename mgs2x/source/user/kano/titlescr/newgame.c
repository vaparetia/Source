//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	newgame.c
		新規ゲームスタート

	2001/06/12 K.Kano
	$Id: newgame.c,v 1.6 2002/12/05 18:42:00 takaki Exp $
*/


#include "titlescr.h"

#include "bgscr.h"

#include "subtitle.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "font.h"
#include "BP_LocalizedTextByEnum.h"

#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

#include "Transfarring_UI_Strings.h"
#include "transfarring_save_load_menu_utils.h"
#include "BP_Network.h"
#include "BP_TUS.h"

#include "libfs.h"

extern void MENU_SetTextSpace( void *work_ptr, int space );

#define N_GAME_ITEMS		3 /* サブスタンスはアンケート付きスタートがない */

#define N_LEVEL_ITEMS		6 /* VE, E, N, H, EX, EE */

#define N_YESNO_ITEMS		2

#define N_RADAR_TYPES		3
#define N_GAMEOVER_ITEMS	2

#ifdef PSX2
#define N_DOGTAGS_ITEMS		2
#endif


#define FOUND_STR_LEN		16
#define YESNO_STR_LEN		8


#define ANIM_COUNT			DIRECT_TICK(10)

#define ANIM_WORK_SIZE		2


#define OPTION_EXPLAIN_U		0
#define OPTION_EXPLAIN_V		0
#define OPTION_EXPLAIN_WIDTH	(30*FONT_WIDTH)
#define OPTION_EXPLAIN_HEIGHT	(3*LINE_HEIGHT+DOWN_MARGINE)

#define OPTION_EXPLAIN_X		(SUBTITLE_X-10)
#define OPTION_EXPLAIN_Y		SUBTITLE_Y
#define OPTION_EXPLAIN_W		(OPTION_EXPLAIN_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define OPTION_EXPLAIN_H		(OPTION_EXPLAIN_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define COLOR_EXPLAIN_U			0
#define COLOR_EXPLAIN_V			0
#define COLOR_EXPLAIN_WIDTH		(30*FONT_WIDTH)
#define COLOR_EXPLAIN_HEIGHT	(10*LINE_HEIGHT+DOWN_MARGINE)

#define COLOR_EXPLAIN_X			56
#define COLOR_EXPLAIN_Y			200
#define COLOR_EXPLAIN_W			(COLOR_EXPLAIN_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define COLOR_EXPLAIN_H			(COLOR_EXPLAIN_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define TEXTCOLOR				SUBTITLE_TEXTCOLOR


#define TROPHIES_X 60
#define TROPHIES_Y 260
#define TROPHIES_EXPLAIN_FONT_DISP_WIDTH 12
#define TROPHIES_EXPLAIN_FONT_DISP_HEIGHT 12
#define TROPHIES_EXPLAIN_U		0
#define TROPHIES_EXPLAIN_V		(OPTION_EXPLAIN_HEIGHT * 2)
#define TROPHIES_EXPLAIN_WIDTH	(30*FONT_WIDTH)
#define TROPHIES_EXPLAIN_HEIGHT	(3*LINE_HEIGHT+DOWN_MARGINE)
#define TROPHIES_EXPLAIN_W		(TROPHIES_EXPLAIN_WIDTH*TROPHIES_EXPLAIN_FONT_DISP_WIDTH/FONT_WIDTH)
#define TROPHIES_EXPLAIN_H		(TROPHIES_EXPLAIN_HEIGHT*TROPHIES_EXPLAIN_FONT_DISP_HEIGHT/FONT_HEIGHT)

#define PSN_JP_X 0
#define PSN_X 60
#define PSN_Y 245

#if defined(BP_PS3)
   #define TROPHIES_FONT_COLOR (0x80373f32)
#else
   #define TROPHIES_FONT_COLOR (0x806e7f64)
#endif


#if 0
#define ENABLE_ALL_GAME_SELECT
#endif


typedef struct {
	GV_ACT_EX actor;
	int parent_name;
	int name;

	int busy_flag;
	int l2d_strcode;
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
	short game_cursor;
	short level_cursor;
	short yesno_cursor;
	short question_cursor;
	short radar_cursor;
	short gameover_cursor;

	unsigned char disp_game_item;
	unsigned char level_u,level_l;
	unsigned char level_mode;
	unsigned char question_first_flag;

	SPR_OBJ *cursor;
	SPR_OBJ *cursor_spr;
	SPR_OBJ *game[N_GAME_ITEMS + 1 ];
	SPR_OBJ *level[N_LEVEL_ITEMS];
	SPR_OBJ *yesno[N_YESNO_ITEMS];
	SPR_OBJ *radar[N_RADAR_TYPES];
	SPR_OBJ *gameover[N_GAMEOVER_ITEMS];
#ifdef PSX2
	SPR_OBJ *dogtags[N_DOGTAGS_ITEMS];
	short	dogtags_cursor;
#endif

	float t_ypos;

	int anim_count;
	struct {
		SPR_OBJ *target;
		unsigned char talpha;
	} key_anim[ANIM_WORK_SIZE];

	int bgname;

	void *strman;

	int found_fade_count;
	int base_u,base_v;
	SPR_OBJ *found_str[FOUND_STR_LEN];
	SPR_OBJ *yesno_str[YESNO_STR_LEN];

	int start_flag;
	int explain_disp_flag;
	int explain_columns;


} Work;

extern void PrintDifficultyExplain(Work *work);
extern void CreateDifficultyExplain(Work *work, int whichText);

#define DISP_CHANL			TITLE_CHANL
#define MENU_PRIORITY		TITLE_PRI_NORMAL


#define QUESTION_ACTORNAME		0x005f6796			/* 井上喜久子アンケート */

#define NEWGAME_ACTORNAME		0x00952656			/* NewGame */

#define REPEAT_FIRST		20
#define REPEAT_NEXT			1


#define STR_NODE_FONT		0x00965c04		/* node_font */

#define	STR_WIDTH			(18) // strcode ではなく文字の長さ
#define STR_HEIGHT			(14) // strcode ではなく文字の長さ

#define SK_FONT_WIDTH			(18.0f-2.0f)
#define SK_FONT_HEIGHT			(14.0f-2.0f)
#define SK_FONT_SPACE_HEIGHT	4.0f  /* DRAW_HEIGHT / 384.0f */


#define MESSAGE_Y				(118+21*9+4)

#define FOUND_STR_X				(60+2)
#define FOUND_STR_Y				(MESSAGE_Y+5)
#define YESNO_STR_X				(384+10-4)
#define YESNO_STR_Y				(MESSAGE_Y+5)


enum {
	GAME_SELECT=0,
	QUESTION,
	LEVEL_SELECT,
	YESNO_SELECT,

#ifdef PSX2
	DOGTAGS_SELECT,
#endif

	RADAR_SELECT,
	GAMEOVER_SELECT,
};

enum {
	INIT_FOR_2ND=0,
	GAME_SELECT_FOR_2ND,
	LEVEL_SELECT_FOR_2ND,
	YESNO_SELECT_FOR_2ND,

	RADAR_SELECT_FOR_2ND,
	GAMEOVER_SELECT_FOR_2ND,
};

extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え


#define TEXT_R				(160/2)
#define TEXT_G				(180/2)
#define TEXT_B				(170/2)


#define FONT_WIDTH				SUBTITLE_FONT_WIDTH
#define FONT_HEIGHT				SUBTITLE_FONT_HEIGHT
#define FONT_DISP_WIDTH			SUBTITLE_FONT_DISP_WIDTH
#define FONT_DISP_HEIGHT		SUBTITLE_FONT_DISP_HEIGHT
#define LINE_SPACE				RUBI_SIZE_H
#define LINE_HEIGHT				(FONT_HEIGHT+LINE_SPACE)


#define EXPLAIN_U		0
#define EXPLAIN_V		0
#define EXPLAIN_WIDTH	(30*FONT_WIDTH)
#if 0
#define EXPLAIN_HEIGHT	(3*FONT_HEIGHT)
#else
#define EXPLAIN_HEIGHT	(3*LINE_HEIGHT+DOWN_MARGINE)
#endif

#define EXPLAIN_X		SUBTITLE_X
#define EXPLAIN_Y		SUBTITLE_Y
#if 0
#define EXPLAIN_W		(30*FONT_DISP_WIDTH)
#define EXPLAIN_H		(3*FONT_DISP_HEIGHT)
#else
#define EXPLAIN_W		(EXPLAIN_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define EXPLAIN_H		(EXPLAIN_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define TEXTCOLOR				SUBTITLE_TEXTCOLOR



#define RADAR_EXPLAIN_RESOURCE		(radar_resource_name[GM_Language-GM_LANG_ENGLISH])
#define GAMEOVER_EXPLAIN_RESOURCE	(gameover_resource_name[GM_Language-GM_LANG_ENGLISH])

#define J_RADAR_EXPLAIN_RESOURCE		0x00a49088		/* レーダーの説明日本語 */
#define J_GAMEOVER_EXPLAIN_RESOURCE		0x00c5f91d		/* 見つかったらゲームオーバーの説明日本語 */

#define E_RADAR_EXPLAIN_RESOURCE		0x001bface		/* レーダーの説明英語 */
#define E_GAMEOVER_EXPLAIN_RESOURCE		0x00414328		/* 見つかったらゲームオーバーの説明英語 */

#define F_RADAR_EXPLAIN_RESOURCE		0x0007980b		/* レーダーの説明フランス語 */
#define F_GAMEOVER_EXPLAIN_RESOURCE		0x0059ae94		/* 見つかったらゲームオーバーの説明フランス語 */

#define G_RADAR_EXPLAIN_RESOURCE		0x00b7056e		/* レーダーの説明ドイツ語 */
#define G_GAMEOVER_EXPLAIN_RESOURCE		0x005959f4		/* 見つかったらゲームオーバーの説明ドイツ語 */

#define I_RADAR_EXPLAIN_RESOURCE		0x0046318a		/* レーダーの説明イタリア語 */
#define I_GAMEOVER_EXPLAIN_RESOURCE		0x00984813		/* 見つかったらゲームオーバーの説明イタリア語 */

#define S_RADAR_EXPLAIN_RESOURCE		0x00fc7c45		/* レーダーの説明スペイン語 */
#define S_GAMEOVER_EXPLAIN_RESOURCE		0x004e92cf		/* 見つかったらゲームオーバーの説明スペイン語 */

static const int radar_resource_name[]={
	E_RADAR_EXPLAIN_RESOURCE,
	F_RADAR_EXPLAIN_RESOURCE,
	G_RADAR_EXPLAIN_RESOURCE,
	I_RADAR_EXPLAIN_RESOURCE,
	S_RADAR_EXPLAIN_RESOURCE,
	0, //韓国語
	J_RADAR_EXPLAIN_RESOURCE,
};

static const int gameover_resource_name[]={
	E_GAMEOVER_EXPLAIN_RESOURCE,
	F_GAMEOVER_EXPLAIN_RESOURCE,
	G_GAMEOVER_EXPLAIN_RESOURCE,
	I_GAMEOVER_EXPLAIN_RESOURCE,
	S_GAMEOVER_EXPLAIN_RESOURCE,
	0, //韓国語
	J_GAMEOVER_EXPLAIN_RESOURCE,
};



/* サブスタンスでは難易度は米版／ヨーロッパ版に準拠 */
enum {
	DIF_CURSOR_POS_VEASY=0,
	DIF_CURSOR_POS_EASY,
	DIF_CURSOR_POS_NORMAL,
	DIF_CURSOR_POS_HARD,
	DIF_CURSOR_POS_EXTREME,

	DIF_CURSOR_POS_E_EXTREME,
};



/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


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
	message[2]=DIRECT_TICK(50);

	GV_SendMessage(&msg);
}



//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)

static void BgscrShowGenziNewgame(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_SHOW_NEWGAME;

	GV_SendMessage(&msg);
}

static void BgscrHideGenziNewgame(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_HIDE_NEWGAME;

	GV_SendMessage(&msg);
}

static void BgscrShowGenziQuestion(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_SHOW_QUESTION;

	GV_SendMessage(&msg);
}

static void BgscrHideGenziQuestion(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_HIDE_QUESTION;

	GV_SendMessage(&msg);
}

static void BgscrShowGenziDifficulty(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_SHOW_DIFFICULTY;

	GV_SendMessage(&msg);
}

static void BgscrHideGenziDifficulty(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_HIDE_DIFFICULTY;

	GV_SendMessage(&msg);
}

//#endif

static void CreateTrophiesExplain(Work *work)
{
   char const * const str = GetTransfarringString(kString_StartGameTrophyMessage);

   MENU_CreateTextTexture(work->strman, TROPHIES_EXPLAIN_U, TROPHIES_EXPLAIN_V,
      TROPHIES_EXPLAIN_WIDTH, TROPHIES_EXPLAIN_HEIGHT, 0,0,0, (char*)str);
}

static void PrintTrophiesExplain(Work *work)
{
   MENU_PutTextScreen(work->strman,
         TROPHIES_X,
         TROPHIES_Y,
         TROPHIES_X + TROPHIES_EXPLAIN_W,
         TROPHIES_Y + TROPHIES_EXPLAIN_H,
         TROPHIES_EXPLAIN_U,
         TROPHIES_EXPLAIN_V,
         TROPHIES_EXPLAIN_U + TROPHIES_EXPLAIN_WIDTH,
         TROPHIES_EXPLAIN_V + TROPHIES_EXPLAIN_HEIGHT,
         TROPHIES_FONT_COLOR);

   Transfarring_RenderPsnConnectionSprite(work->strman, BP_Area_JP() ? PSN_JP_X : PSN_X, PSN_Y,
      BP_Network_IsSignedIn() ? kTransfarring_PSN_Connected : kTransfarring_PSN_Disconnected);
}

static void CreateRadarExplain(Work *work)
{
	if(!work->explain_disp_flag){
		char *str;
		int  i;

		str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(RADAR_EXPLAIN_RESOURCE,work->radar_cursor));

		work->explain_columns=1;

#if BP_USE_NEW_FONT_SYSTEM()
      {
         unsigned char* temp = str;

         for( ;; )
         {
            int code;

            temp = BP_font_decode_utf8_character(&code, temp);

            if( code == 0 )
               break;

            if( code == '|')
               work->explain_columns++;
         }
      }
#else

      i=0;		
      while(*(str+i)!='\0'){
         if( *(str+i) & 0x80 ){ /* 漢字コードだお☆ */
            i += 2;
            continue;
         }
         if(*(str+i)=='|'){
            work->explain_columns++;
         }
         i++;
      }
#endif

		//MENU_ClearTextTexture(work->strman);
      MENU_ClearPartTextTexture(work->strman, 0, EXPLAIN_HEIGHT);

		// spaceを無理やり設定する。
		MENU_SetTextSpace( work->strman, 0 );
		MENU_CreateTextTexture(work->strman,
							   EXPLAIN_U,
							   EXPLAIN_V,
							   EXPLAIN_WIDTH,
							   EXPLAIN_HEIGHT,
							   0,0,0,str);
		// ほかに影響が出ないように規定値に戻す。
		MENU_SetTextSpace( work->strman, 12 );
	}

	work->explain_disp_flag=1;
}

static void PrintExplain(Work *work)
{
	if(work->explain_disp_flag){
		int y=EXPLAIN_Y;
		int h=EXPLAIN_H;

		work->explain_disp_flag=1;

		if(work->explain_columns>1){
			y-=FONT_DISP_HEIGHT/2;
			h=h*5/6;
		}

		MENU_PutTextScreen(work->strman,
						   EXPLAIN_X,
						   y,
						   EXPLAIN_X+EXPLAIN_W,
						   y+h,
						   EXPLAIN_U,
						   EXPLAIN_V,
						   EXPLAIN_U+EXPLAIN_WIDTH,
						   EXPLAIN_V+EXPLAIN_HEIGHT,
						   TEXTCOLOR);
	}
}

static void CreateGameoverExplain(Work *work)
{
	if(!work->explain_disp_flag){
		char *str;
		int i;

		str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(GAMEOVER_EXPLAIN_RESOURCE,work->gameover_cursor));

		work->explain_columns=1;

#if BP_USE_NEW_FONT_SYSTEM()
      {
         unsigned char* temp = str;

         for( ;; )
         {
            int code;

            temp = BP_font_decode_utf8_character(&code, temp);

            if( code == 0 )
               break;

            if( code == '|')
               work->explain_columns++;
         }
      }
#else

      i=0;		
      while(*(str+i)!='\0'){
         if( *(str+i) & 0x80 ){ /* 漢字コードだお☆ */
            i += 2;
            continue;
         }
         if(*(str+i)=='|'){
            work->explain_columns++;
         }
         i++;
      }
#endif

		//MENU_ClearTextTexture(work->strman);
      MENU_ClearPartTextTexture(work->strman, 0, EXPLAIN_HEIGHT);

		MENU_CreateTextTexture(work->strman,
							   EXPLAIN_U,
							   EXPLAIN_V,
							   EXPLAIN_WIDTH,
							   EXPLAIN_HEIGHT,
							   0,0,0,str);
	}

	work->explain_disp_flag=1;
}


static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1 &&
	   work->ans==TITLE_MSG_MODEEND){

		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=PROC_COM_NEWGAME;
		argv[1]=work->game_cursor;
		argv[2]=work->level_cursor;
		argv[3]=work->yesno_cursor;
		argv[4]=work->question_cursor;

		GCL_ExecProc(work->proc,&arg);
	}
}

static void CallLocalProcFor2nd(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		if(work->ans==TITLE_MSG_MODEEND){
			argv[0]=1;
			argv[1]=work->game_cursor;
			argv[2]=work->level_cursor;
			argv[3]=work->yesno_cursor;
		}
		else{
			argv[0]=0;
		}

		GCL_ExecProc(work->proc,&arg);
	}
}

static void ProgMessage(Work *work)
{
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		if(command==QUESTION_ACTORNAME){
			if(work->step==QUESTION){
				work->question_cursor=arg;
				work->sub_step=0x10;
			}
		}

		msg++;
		n_msg--;
    }
}

static void Key(Work *work)
{
	int status=GV_PadDataDirect[0].status & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL|PAD_X);
	int press=GV_PadDataDirect[0].press & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL|PAD_X);

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

static void LocalShowCursor(Work *work)
{
	SPR_SHOW(work->cursor);
	SPR_SHOW(work->cursor_spr);
}

static void HideCursor(Work *work)
{
	SPR_HIDE(work->cursor);
	SPR_HIDE(work->cursor_spr);
}


static void AnimCtrlSet(Work *work,int index,SPR_OBJ *spr,int talpha)
{
	work->key_anim[index].target=spr;
	work->key_anim[index].talpha=talpha;
}

static void AnimCtrl(Work *work)
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


static void ShowGameMenu(Work *work)
{
	switch(work->disp_game_item){
	case 1:
		// SPR_SHOW(work->game[0]);
		SPR_HIDE(work->game[1]);
		SPR_HIDE(work->game[2]);
		SPR_HIDE(work->game[3]);
		break;
	case 3:
		// SPR_SHOW(work->game[0]);
		// SPR_SHOW(work->game[1]);
		SPR_HIDE(work->game[2]);
		SPR_HIDE(work->game[3]);
		break;
	case 5:
		// SPR_SHOW(work->game[0]);
		SPR_HIDE(work->game[1]);
		// SPR_SHOW(work->game[2]);
		SPR_HIDE(work->game[3]);

		work->game[2]->sprite.pos.y=work->game[1]->sprite.pos.y;
		break;
	default:
		/* TANKER-PALNTの表示を消す */
		SPR_HIDE(work->game[0]);
		// SPR_SHOW(work->game[0]);
		// SPR_SHOW(work->game[1]);
		// SPR_SHOW(work->game[2]);
		// SPR_SHOW(work->game[3]);
		break;
	}

}

static int GameSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			work->action_strcode=CODE_SHOWSELGAME;
			work->question_cursor=-1;
			work->sub_step++;


//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrShowGenziNewgame(work);
//#endif

		}
		break;
	case 1:
		ShowGameMenu(work);

		/* spr[0]は不使用*/
		if(!work->busy_flag){
			int cur = work->game_cursor;
			if( cur == 0 ){
				cur = 3;
			}
			
			work->sub_step++;

			LocalShowCursor(work);

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->game[work->game_cursor]->sprite.pos.y;
			work->game[cur]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:
      PrintTrophiesExplain(work);
		ShowGameMenu(work);

		if(work->anim_count>0) break;

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_HIDESELGAME;
			HideCursor(work);

			work->sub_step=0x20;
			SE_CANCEL();


//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrHideGenziNewgame(work);
//#endif

			break;
		}
		else if(work->key_press & PAD_OK){
			work->action_strcode=CODE_NEXTFROMSELGAME;
			HideCursor(work);

			work->sub_step=0x10;
			SE_OK();


//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrHideGenziNewgame(work);
//#endif

			break;
		}

		{
			int cur0,cur1;

			/* spr[0]は不使用*/
			if( work->game_cursor == 0 ){
				cur0 = 3;
			} else {
				cur0 = work->game_cursor;
			}
			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
#if !BP_TGS_DEMO()
				do
            {
					work->game_cursor--;
					if(work->game_cursor<0)
               {
						work->game_cursor=N_GAME_ITEMS-1;
					}
				} while(!(work->disp_game_item & (1<<work->game_cursor)));
#endif
				break;
			case PAD_D:
#if !BP_TGS_DEMO()
				do
            {

					work->game_cursor++;
					if(work->game_cursor>=N_GAME_ITEMS)
               {
						work->game_cursor=0;
					}
				} while(!(work->disp_game_item & (1<<work->game_cursor)));
#endif
				break;
			}

         if( (work->key_press & PAD_X) && !BP_Network_IsSignedIn() )
         {
            BP_TUS_WantsSignin();
            return 0;
         }

			/* spr[0]は不使用*/
			if( work->game_cursor == 0 ){
				cur1 = 3;
			} else {
				cur1=work->game_cursor;
			}

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;

				work->t_ypos=work->game[work->game_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->game[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->game[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
      PrintTrophiesExplain(work);
		ShowGameMenu(work);
		if(work->busy_flag) break;
		return 1;
	case 0x20:
      //PrintTrophiesExplain(work);
		ShowGameMenu(work);
		if(work->busy_flag) break;
		return -1;
	}

	return 0;
}

static int QuestionSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			work->action_strcode=CODE_SHOWQST;
			work->sub_step++;


//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrShowGenziQuestion(work);
//#endif

		}
		break;
	case 1:
		if(!work->busy_flag){
			GV_SetActorChild(work,NewQuestion(work->name,QUESTION_ACTORNAME,
											  work->question_first_flag));

			work->question_first_flag=1;
			work->sub_step++;
		}
		break;
	case 2:
		break;

	case 0x10:
		if(work->question_cursor<0){
			work->action_strcode=CODE_HIDEQST;


//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrHideGenziQuestion(work);
//#endif

			return -1;
		}
		else{
			work->action_strcode=CODE_NEXTFROMQST;


//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrHideGenziQuestion(work);
//#endif

			return 1;
		}
		break;
	}

	return 0;
}

static int CheckExtreme(Work *work)
{
	switch(GM_TitleMenuStatus & (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD)){
	case 0:
		return 0;
	case TITLE_MENU_TANKER_CLEARD:
		if(work->game_cursor!=1){
			/* Tanker以外 */
			return 0;
		}
		break;
	case TITLE_MENU_PLANT_CLEARD:
		if(work->game_cursor!=2){
			/* Plant以外 */
			return 0;
		}
		break;
	case TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD:
		break;
	}
	return 1;
}

static inline void DispExtreme(Work *work)
{
	if(!CheckExtreme(work)){
#if 0 ///
		SPR_HIDE(work->level[4]);
#else
		/* EX, EE を消す */
		SPR_HIDE(work->level[4]);
		SPR_HIDE(work->level[5]);
#endif
	}
	else{

		SPR_OBJ *eex=L2D_GetObject(work->l2d_handle,CODE_SWEEX);
		SPR_SHOW(eex);

	}

}

static void ShowLevel(Work *work)
{
   if(work->question_cursor<0){
		/* 全て選択可 */
#if 0
		SPR_SHOW(work->level[0]);
		SPR_SHOW(work->level[1]);
		SPR_SHOW(work->level[2]);
		SPR_SHOW(work->level[3]);
		SPR_SHOW(work->level[4]);
#else
		work->action_strcode=CODE_SHOWDIFALL;


#endif

		work->level_l=0;
		if(CheckExtreme(work)){
			work->level_u=5;
		}
		else work->level_u=3;
		work->level_mode=0;
	}
	else if(work->question_cursor==2){

		/* 全て選択可 */
#if 0
		SPR_SHOW(work->level[0]);
		SPR_SHOW(work->level[1]);
		SPR_SHOW(work->level[2]);
		SPR_SHOW(work->level[3]);
		SPR_SHOW(work->level[4]);
#else
		work->action_strcode=CODE_SHOWDIFALLFROMQST;


#endif

		work->level_l=0;
		if(CheckExtreme(work)){
			work->level_u=5;
		}
		else work->level_u=3;
		work->level_mode=0;
	}
	else if(work->question_cursor==0 ||
			work->question_cursor==4){

		/* アクションは得意 */
#if 0
		SPR_HIDE(work->level[0]);
		SPR_HIDE(work->level[1]);
		SPR_SHOW(work->level[2]);
		SPR_SHOW(work->level[3]);
		SPR_SHOW(work->level[4]);
#else
		work->action_strcode=CODE_SHOWDIFHARD;
#endif

		work->level_l=DIF_CURSOR_POS_NORMAL;
		if(CheckExtreme(work)){
			work->level_u=5;
		}
		else work->level_u=3;
		work->level_mode=2;
	}
	else{
		/* アクションは苦手 */
#if 0
		SPR_SHOW(work->level[0]);
		SPR_SHOW(work->level[1]);
		SPR_SHOW(work->level[2]);
		SPR_HIDE(work->level[3]);
		SPR_HIDE(work->level[4]);
#else
		work->action_strcode=CODE_SHOWDIFEASY;
#endif

		work->level_l=0;
		work->level_u=DIF_CURSOR_POS_NORMAL;
		work->level_mode=1;
   }

	if(work->level_cursor<work->level_l) work->level_cursor=work->level_l;
	if(work->level_cursor>work->level_u) work->level_cursor=work->level_u;
}

static int LevelSubStep(Work *work)
{


	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			ShowLevel(work);

			// work->action_strcode=CODE_SHOWDIF;

			work->sub_step++;


//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrShowGenziDifficulty(work);
//#endif

		}
		break;
	case 1:
      DispExtreme(work);
		// ShowLevel(work);

		if(!work->busy_flag){
			SPR_OBJ *parent;

			work->sub_step++;

         work->explain_disp_flag=1;

			LocalShowCursor(work);

			parent=work->level[work->level_cursor]->head.parent;

			work->anim_count=0;

			work->t_ypos=work->level[work->level_cursor]->sprite.pos.y+5;
			while(parent!=NULL){
				work->t_ypos+=parent->empty.pos.y;
				parent=parent->head.parent;
			}
			work->cursor->empty.pos.y=work->t_ypos;

			work->level[work->level_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:
      CreateDifficultyExplain(work,work->level_cursor);
      PrintDifficultyExplain(work);

      DispExtreme(work);

		if(work->anim_count>0) break;

		if(work->key_press & PAD_CANCEL)
      {
#if 0 ///サブ
			if(work->game_cursor==0){
				switch(work->level_mode){
				case 0:
					work->action_strcode=CODE_HIDEDIFALLTOQST;
					break;
				case 1:
					work->action_strcode=CODE_HIDEDIFEASYTOQST;
					break;
				case 2:
					work->action_strcode=CODE_HIDEDIFHARDTOQST;
					break;
				}



			}
			else
#endif
			  {

				switch(work->level_mode){
				case 0:
					work->action_strcode=CODE_HIDEDIFALLTOSEL;
					break;
				case 1:
					work->action_strcode=CODE_HIDEDIFEASYTOSEL;
					break;
				case 2:
					work->action_strcode=CODE_HIDEDIFHARDTOSEL;
					break;
				}
			}
			// work->action_strcode=CODE_HIDEDIF;

			HideCursor(work);

			SE_CANCEL();

			work->sub_step=0x10;
			work->ans=-1;



//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrHideGenziDifficulty(work);
//#endif

			return 0;
		}
		else if(work->key_press & PAD_OK){

#if 0
			if(work->level_cursor<=DIF_CURSOR_POS_NORMAL){
				if(work->game_cursor==0){
					switch(work->level_mode){
					case 0:
						work->action_strcode=CODE_HIDEDIFALLTONEXT;
						break;
					case 1:
						work->action_strcode=CODE_HIDEDIFEASYTONEXT;
						break;
					case 2:
						work->action_strcode=CODE_HIDEDIFHARDTONEXT;
						break;
					}
				}
				else{
					switch(work->level_mode){
					case 0:
						work->action_strcode=CODE_HIDEDIFALLTONEXT1;
						break;
					case 1:
						work->action_strcode=CODE_HIDEDIFEASYTONEXT1;
						break;
					case 2:
						work->action_strcode=CODE_HIDEDIFHARDTONEXT1;
						break;
					}
				}
			}
#else

#if 0 ///サブ
			if(work->game_cursor==0){
				switch(work->level_mode){
				case 0:
					work->action_strcode=CODE_HIDEDIFALLTONEXT;
					break;
				case 1:
					work->action_strcode=CODE_HIDEDIFEASYTONEXT;
					break;
				case 2:
					work->action_strcode=CODE_HIDEDIFHARDTONEXT;
					break;
				}
			}
			else
#endif
			  {
				switch(work->level_mode){
				case 0:
					work->action_strcode=CODE_HIDEDIFALLTONEXT1;
					break;
				case 1:
					work->action_strcode=CODE_HIDEDIFEASYTONEXT1;
					break;
				case 2:
					work->action_strcode=CODE_HIDEDIFHARDTONEXT1;
					break;
				}
			}


#endif

			HideCursor(work);

			SE_OK();

			work->sub_step=0x10;
			work->ans=1;



//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrHideGenziDifficulty(work);
//#endif

			return 0;
		}

#if 0
		work->level[work->level_cursor]->sprite.col.a=UNSEL_ALPHA;

		switch(work->key_autostatus & (PAD_U|PAD_D)){
		case PAD_U:
			work->level_cursor--;
			if(work->level_cursor<work->level_l){
				work->level_cursor=work->level_u;
			}

			SE_SEL();
			break;
		case PAD_D:
			work->level_cursor++;
			if(work->level_cursor>work->level_u){
				work->level_cursor=work->level_l;
			}

			SE_SEL();
			break;
		}

		work->cursor->empty.pos.y=work->level[work->level_cursor]->sprite.pos.y;
		work->level[work->level_cursor]->sprite.col.a=SEL_ALPHA;
#else
		{
			int cur0,cur1;

			cur0=work->level_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D))
         {
			case PAD_U:
				work->level_cursor--;
				if(work->level_cursor<work->level_l){
					work->level_cursor=work->level_u;
				}
				break;
			case PAD_D:
				work->level_cursor++;
				if(work->level_cursor>work->level_u){
					work->level_cursor=work->level_l;
				}
				break;
			}

			cur1=work->level_cursor;

   		if(cur0!=cur1)
         {
				SPR_OBJ *parent;

            work->explain_disp_flag=1;

				work->anim_count=ANIM_COUNT;

				parent=work->level[work->level_cursor]->head.parent;
				work->t_ypos=work->level[work->level_cursor]->sprite.pos.y+5;        // offset the cursor to match the text.
				while(parent!=NULL)
            {
					work->t_ypos+=parent->empty.pos.y;
					parent=parent->head.parent;
				}

				AnimCtrlSet(work,0,work->level[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->level[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
#endif
		break;

	case 0x10:
      DispExtreme(work);

		if(work->busy_flag) break;
		return work->ans;
	}

	return 0;
}


static void SetRadarType(Work *work)
{
	switch(work->radar_cursor){
	case 0:
		/* TYPE 1 */
		GM_Configuration&=~(GM_CONFIG_RADAR_OFF_INTRUDE|GM_CONFIG_RADAR_OFF);
		break;
	case 1:
		/* TYPE 2 */
		GM_Configuration|=GM_CONFIG_RADAR_OFF_INTRUDE;
		GM_Configuration&=~GM_CONFIG_RADAR_OFF;
		break;
	case 2:
		/* OFF */
		GM_Configuration&=~GM_CONFIG_RADAR_OFF_INTRUDE;
		GM_Configuration|=GM_CONFIG_RADAR_OFF;
		break;
	}
}

static void GetRadarType(Work *work)
{
	switch(GM_Configuration & (GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE)){
	case 0:
		/* TYPE 1 */
		work->radar_cursor=0;
		break;
	case GM_CONFIG_RADAR_OFF_INTRUDE:
		/* TYPE 2 */
		work->radar_cursor=1;
		break;
	case GM_CONFIG_RADAR_OFF:
	case GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE:
		/* OFF */
		work->radar_cursor=2;
		break;
	}
}

static int RadTypeSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			  {
				work->action_strcode=CODE_SHOWRADTYPE1;
			}


			GetRadarType(work);

			work->explain_disp_flag=0;

			work->sub_step++;
		}
		break;
	case 1:
		if(!work->busy_flag){
			work->sub_step++;

			LocalShowCursor(work);

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->radar[work->radar_cursor]->sprite.pos.y;
			work->radar[work->radar_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:
		PrintExplain(work);

		if(work->anim_count>0) break;

		CreateRadarExplain(work);

		if(work->key_press & PAD_CANCEL){
			  {
				work->action_strcode=CODE_BACKFROMRADTYPE1;
			}


			HideCursor(work);

			SE_CANCEL();

			work->sub_step=0x10;
			work->ans=-1;

			return 0;
		}
		else if(work->key_press & PAD_OK){
			if(work->level_cursor<=DIF_CURSOR_POS_NORMAL){
				/* VERY EASY, EASY, NORMAL */
#ifdef PSX2 /* DOGTAGS仕様 */
				if((GM_TitleMenuStatus & (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD))
			   ==(TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD)){

					/* 次にドックタグ選択画面が来る */
					work->action_strcode=CODE_HIDERADTYPE1_TO_GAMEOVER;
				} else {
					work->action_strcode=CODE_HIDERADTYPE1;
				}

#else
				{
					work->action_strcode=CODE_HIDERADTYPE1;
				}
#endif
			}
			else{
				/* HARD, EXTREME */
				  {
					work->action_strcode=CODE_HIDERADTYPE1_TO_GAMEOVER;
				}


			}
			HideCursor(work);

			SE_OK();

			work->sub_step=0x10;
			work->ans=1;

			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->radar_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->radar_cursor--;
				if(work->radar_cursor<0){
					work->radar_cursor=N_RADAR_TYPES-1;
				}
				break;
			case PAD_D:
				work->radar_cursor++;
				if(work->radar_cursor>=N_RADAR_TYPES){
					work->radar_cursor=0;
				}
				break;
			}

			cur1=work->radar_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;
				work->explain_disp_flag=0;

				work->t_ypos=work->radar[work->radar_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->radar[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->radar[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
      if(work->busy_flag) break;
      return work->ans;
	}

	return 0;
}

static int EndIfFoundSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
#if 0 ///
			if(work->game_cursor==0){
				work->action_strcode=CODE_SHOWGAMEOVER1;
				//work->action_strcode=CODE_SHOWGAMEOVER;
			}
			else
#endif
			  {
				work->action_strcode=CODE_SHOWGAMEOVER1;
			}


			work->gameover_cursor=0;
			work->sub_step++;

			work->explain_disp_flag=0;
		}
		break;
	case 1:
		if(!work->busy_flag){
			work->sub_step++;

			LocalShowCursor(work);

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->gameover[work->gameover_cursor]->sprite.pos.y;
			work->gameover[work->gameover_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:

		PrintExplain(work);
		
		if(work->anim_count>0) break;

		CreateGameoverExplain(work);

		if(work->key_press & PAD_CANCEL){
			{
				work->action_strcode=CODE_BACKFROMGAMEOVER1;
			}


			HideCursor(work);

			SE_CANCEL();

			work->sub_step=0x10;
			work->ans=-1;

			return 0;
		}
		else if(work->key_press & PAD_OK){
#ifdef PSX2 /* DOGTAGS仕様 */
			if((GM_TitleMenuStatus & (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD))
			   ==(TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD)){
				/* 次にDOGTAGS選択画面がくる */
				work->action_strcode=CODE_HIDEGAMEOVER1TODOGTAGS;
			} else {
				work->action_strcode=CODE_HIDEGAMEOVER1;
			}
#else
			{
				work->action_strcode=CODE_HIDEGAMEOVER1;
			}
#endif


			HideCursor(work);

			SE_OK();

			work->sub_step=0x10;
			work->ans=1;

			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->gameover_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->gameover_cursor--;
				if(work->gameover_cursor<0){
					work->gameover_cursor=N_GAMEOVER_ITEMS-1;
				}
				break;
			case PAD_D:
				work->gameover_cursor++;
				if(work->gameover_cursor>=N_GAMEOVER_ITEMS){
					work->gameover_cursor=0;
				}
				break;
			}

			cur1=work->gameover_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;
				work->explain_disp_flag=0;

				work->t_ypos=work->gameover[work->gameover_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->gameover[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->gameover[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
		if(work->busy_flag) break;
		return work->ans;
	}

	return 0;
}

#ifdef PSX2
/**** 新旧ドックタグ選択はPS2版のみの仕様 ****/
static int DogTagsSubStep( Work *work ){
	switch( work->sub_step ){
	  case 0:
		if( ComNodeFrameBusy() ){
			break;
		}
		if( !work->busy_flag ){
			if(work->level_cursor<=DIF_CURSOR_POS_NORMAL){
				/* レーダー画面から */
				work->action_strcode = CODE_SHOWDOGTAGS2;
			} else {
				/* ゲームオーバー条件画面から */
				work->action_strcode = CODE_SHOWDOGTAGS3;
			}
		}
		work->sub_step ++;
		break;
	  case 1:
		if( !work->busy_flag ){
			work->sub_step ++;

			LocalShowCursor( work );

			work->anim_count = 0;
			work->t_ypos=work->cursor->empty.pos.y = work->dogtags[work->dogtags_cursor]->sprite.pos.y;
			work->dogtags[work->dogtags_cursor]->sprite.col.a = SEL_ALPHA;
		}
		break;
	  case 2:
		if(work->anim_count>0) break;

		if(work->key_press & PAD_CANCEL){
			if(work->level_cursor<=DIF_CURSOR_POS_NORMAL){
				/* レーダー画面に戻る */
				work->action_strcode=CODE_BACK_TO_RADAR;
			} else {
				/* ゲームオーバー条件画面に戻る */
				work->action_strcode=CODE_BACK_TO_GAMEOVER;
			}
			HideCursor(work);

			SE_CANCEL();

			work->sub_step = 0x10;
			work->ans = -1;

			return 0;
		} else if(work->key_press & PAD_OK){
			if(work->level_cursor<=DIF_CURSOR_POS_NORMAL){
				/* レーダー画面から来た */
				work->action_strcode=CODE_HIDEDOGTAGS1;
			} else {
				/* ゲームオーバー条件画面から来た */
				work->action_strcode=CODE_HIDEDOGTAGS2;
			}
			HideCursor(work);

			if( work->dogtags_cursor == 1 ){
				/* 2002 */
				GM_Configuration2 |= GM_CONFIG_DOGTAGS_2002;
				printf("2002 DOGTAGS\n");
			} else {
				/* 2001 */
				GM_Configuration2 &= ~GM_CONFIG_DOGTAGS_2002;
				printf("2001 DOGTAGS\n");
			}				

			SE_OK();

			work->sub_step = 0x10;
			work->ans = 1;

			return 0;
		}

		{
			int cur0,cur1;

			cur0 = work->dogtags_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->dogtags_cursor--;
				if(work->dogtags_cursor < 0){
					work->dogtags_cursor = ( N_DOGTAGS_ITEMS - 1 );
				}
				break;
			case PAD_D:
				work->dogtags_cursor++;
				if(work->dogtags_cursor >= N_DOGTAGS_ITEMS){
					work->dogtags_cursor = 0;
				}
				break;
			}

			cur1=work->dogtags_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;
				work->explain_disp_flag=0;

				work->t_ypos=work->dogtags[work->dogtags_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->dogtags[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->dogtags[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;
	  case 0x10:
		if(work->busy_flag) break;
		return work->ans;
	}
	return 0;
}
#endif

#if 0


/* ------------------------------------------------------------------------ */


static void SetFound(Work *work)
{
	char str[FOUND_STR_LEN];
	int i;

	strcpy(str,"END IF FOUND ?");

	SK_PrintfNormal(NULL,work->found_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,FOUND_STR_LEN);

	for(i=0;i<FOUND_STR_LEN;i++){
		SPR_OBJ *spr=work->found_str[i];

		spr->sprite.pos.x=FOUND_STR_X;
		spr->sprite.pos.y=FOUND_STR_Y;
	}

	SK_PrintfNormal(str,work->found_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,FOUND_STR_LEN);
	SK_PrintfChengColor2(work->found_str,TEXT_R,TEXT_G,TEXT_B,SEL_ALPHA,FOUND_STR_LEN);
	SK_AllShow(work->found_str,FOUND_STR_LEN);
}

static void HideFound(Work *work)
{
	SK_AllHide(work->found_str,FOUND_STR_LEN);
}

static void SetYesNo(Work *work)
{
	char str[YESNO_STR_LEN];
	int i;

	strcpy(str,"YES/NO");

	SK_PrintfNormal(NULL,work->yesno_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,YESNO_STR_LEN);

	for(i=0;i<YESNO_STR_LEN;i++){
		SPR_OBJ *spr=work->yesno_str[i];

		spr->sprite.pos.x=YESNO_STR_X;
		spr->sprite.pos.y=YESNO_STR_Y;
	}

	SK_PrintfNormal(str,work->yesno_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,YESNO_STR_LEN);
	SK_PrintfChengColor2(work->yesno_str,TEXT_R,TEXT_G,TEXT_B,UNSEL_ALPHA,YESNO_STR_LEN);
	SK_AllShow(work->yesno_str,YESNO_STR_LEN);
}

static void CurMoveYesNo(Work *work)
{
	int i;
	if(work->yesno_cursor){
		for(i=0;i<3;i++){
			SPR_OBJ *spr=work->yesno_str[i];
			spr->sprite.col.a=UNSEL_ALPHA;
		}
		for(i=0;i<3;i++){
			SPR_OBJ *spr=work->yesno_str[i+4];
			spr->sprite.col.a=SEL_ALPHA;
		}
	}
	else{
		for(i=0;i<3;i++){
			SPR_OBJ *spr=work->yesno_str[i];
			spr->sprite.col.a=SEL_ALPHA;
		}
		for(i=0;i<3;i++){
			SPR_OBJ *spr=work->yesno_str[i+4];
			spr->sprite.col.a=UNSEL_ALPHA;
		}
	}
}

static void HideYesNo(Work *work)
{
	SK_AllHide(work->yesno_str,YESNO_STR_LEN);
}

static int FadeFound(Work *work)
{
	if(work->found_fade_count>0){
		SPR_OBJ *spr;
		int a;
		int i;

		spr=work->found_str[0];
		a=spr->sprite.col.a;

		a+=(0-a)/work->found_fade_count;

		SK_PrintfChengColor2(work->found_str,TEXT_R,TEXT_G,TEXT_B,a,FOUND_STR_LEN);


		spr=work->yesno_str[0];
		a=spr->sprite.col.a;

		a+=(0-a)/work->found_fade_count;

		for(i=0;i<3;i++){
			spr=work->yesno_str[i];
			spr->sprite.col.a=a;
		}


		spr=work->yesno_str[3];
		a=spr->sprite.col.a;

		a+=(0-a)/work->found_fade_count;

		spr->sprite.col.a=a;


		spr=work->yesno_str[4];
		a=spr->sprite.col.a;

		a+=(0-a)/work->found_fade_count;

		for(i=0;i<3;i++){
			spr=work->yesno_str[i+4];
			spr->sprite.col.a=a;
		}

		work->found_fade_count--;

		return 0;
	}
	else return 1;
}


/* ------------------------------------------------------------------------ */


#define FOUND_FADE_COUNT		DIRECT_TICK(20)

static int YesNoSubStep(Work *work)
{
	DispExtreme(work);

	switch(work->sub_step){

#if 0
	case 0:
		if(!work->busy_flag){
			work->action_strcode=CODE_SHOWFIND;
			work->sub_step++;
			work->yesno_cursor=N_YESNO_ITEMS-1;
		}
		break;
	case 1:
		if(!work->busy_flag){
			work->action_strcode=CODE_SHOWYESNO;
			work->sub_step++;
		}
		break;
	case 2:
		if(!work->busy_flag){
			work->sub_step++;

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->yesno[work->yesno_cursor]->sprite.pos.y;
			work->yesno[work->yesno_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 3:
		if(work->anim_count>0) break;

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_BACKFROMFIND;

			SE_CANCEL();
			return -1;
		}
		else if(work->key_press & PAD_OK){
			work->action_strcode=CODE_HIDEFIND;

			SE_OK();
			return 1;
		}

#if 0
		work->yesno[work->yesno_cursor]->sprite.col.a=UNSEL_ALPHA;

		switch(work->key_press & (PAD_U|PAD_D)){
		case PAD_U:
			work->yesno_cursor--;
			if(work->yesno_cursor<0){
				work->yesno_cursor=N_YESNO_ITEMS-1;
			}

			SE_SEL();
			break;
		case PAD_D:
			work->yesno_cursor++;
			if(work->yesno_cursor>=N_YESNO_ITEMS){
				work->yesno_cursor=0;
			}

			SE_SEL();
			break;
		}

		work->cursor->empty.pos.y=work->yesno[work->yesno_cursor]->sprite.pos.y;
		work->yesno[work->yesno_cursor]->sprite.col.a=SEL_ALPHA;
#else
		{
			int cur0,cur1;

			cur0=work->yesno_cursor;

			switch(work->key_press & (PAD_U|PAD_D)){
			case PAD_U:
				work->yesno_cursor--;
				if(work->yesno_cursor<0){
					work->yesno_cursor=N_YESNO_ITEMS-1;
				}
				break;
			case PAD_D:
				work->yesno_cursor++;
				if(work->yesno_cursor>=N_YESNO_ITEMS){
					work->yesno_cursor=0;
				}
				break;
			}

			cur1=work->yesno_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;

				work->t_ypos=work->yesno[work->yesno_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->yesno[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->yesno[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
#endif

		break;
#else
	case 0:
		if(work->busy_flag) break;

		work->sub_step++;
		work->yesno_cursor=N_YESNO_ITEMS-1;
		SetFound(work);
		SetYesNo(work);

	case 1:
		if(work->key_press & PAD_CANCEL){
			HideFound(work);
			HideYesNo(work);

			SE_CANCEL();
			return -1;
		}
		else if(work->key_press & PAD_OK){
			if(work->game_cursor==0){
				switch(work->level_mode){
				case 0:
					work->action_strcode=CODE_HIDEDIFALLTONEXT;
					break;
				case 1:
					work->action_strcode=CODE_HIDEDIFALLTONEXT;
					break;
				case 2:
					work->action_strcode=CODE_HIDEDIFALLTONEXT;
					break;
				}
			}
			else{
				switch(work->level_mode){
				case 0:
					work->action_strcode=CODE_HIDEDIFALLTONEXT1;
					break;
				case 1:
					work->action_strcode=CODE_HIDEDIFALLTONEXT1;
					break;
				case 2:
					work->action_strcode=CODE_HIDEDIFALLTONEXT1;
					break;
				}
			}

			work->found_fade_count=FOUND_FADE_COUNT;
			work->sub_step++;
			SE_OK();
			return 0;
		}

		switch(work->key_press & (PAD_L|PAD_R)){
		case PAD_L:
			work->yesno_cursor--;
			if(work->yesno_cursor<0){
				work->yesno_cursor=N_YESNO_ITEMS-1;
			}

			SE_SEL();
			break;
		case PAD_R:
			work->yesno_cursor++;
			if(work->yesno_cursor>=N_YESNO_ITEMS){
				work->yesno_cursor=0;
			}

			SE_SEL();
			break;
		}

		CurMoveYesNo(work);
		break;
	case 2:
		if(!FadeFound(work)) break;
		return 1;
#endif

	}

	return 0;
}

#endif



static void Step(Work *work)
{
   switch(work->step){
	case GAME_SELECT:
		switch(GameSubStep(work)){
		case 1:
			/* サブスタンスは1ST TIMEがない */
			if(work->game_cursor==0) work->step=LEVEL_SELECT;
			else work->step=LEVEL_SELECT;
			
#if BP_TGS_DEMO()          //BP JG - skip passed the difficulty select screen if it's TGS demo.
         /* TYPE 2 */
         GM_Configuration|=GM_CONFIG_RADAR_OFF_INTRUDE;
         GM_Configuration&=~GM_CONFIG_RADAR_OFF;

         GM_Configuration&=~GM_CONFIG_END_IF_FOUND;
         work->ans=TITLE_MSG_MODEEND;
         work->step=0x10;
         work->sub_step=0;
         BgscrFadeout(work);

#endif
			work->sub_step=0;

			/* Normalが標準位置 */
			work->level_cursor=DIF_CURSOR_POS_NORMAL;
			break;
		case -1:
			work->ans=TITLE_MSG_MODECANCEL;
			work->step=0x10;
			break;
		}
		break;
	case QUESTION:
		switch(QuestionSubStep(work)){
		case 1:
			work->step=LEVEL_SELECT;
			work->sub_step=0;

			/* Normalが標準位置 */
			work->level_cursor=DIF_CURSOR_POS_NORMAL;
			break;
		case -1:
			work->step=GAME_SELECT;
			work->sub_step=0;
			break;
		}
		break;
	case LEVEL_SELECT:
		switch(LevelSubStep(work)){
		case 1:
			/* レーダー選択画面へ */
			work->step=RADAR_SELECT;
			work->sub_step=0;
			/* TYPE 2が標準 */
			work->radar_cursor=1;
			break;
		case -1:
			/* サブスタンス1ST TIMEがない */
			if(work->game_cursor==0) work->step=GAME_SELECT;
			else work->step=GAME_SELECT;
			
			work->sub_step=0;
			break;
		}
		break;

	case RADAR_SELECT:
		switch(RadTypeSubStep(work)){
		case 1:
			if(work->level_cursor>DIF_CURSOR_POS_NORMAL){
				work->step=GAMEOVER_SELECT;
				work->sub_step=0;
			}
			else{
#ifdef PSX2 /* DOGTAGS仕様 */
				SetRadarType(work);
				GM_Configuration&=~GM_CONFIG_END_IF_FOUND;

				if((GM_TitleMenuStatus & (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD))
				   ==(TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD)){
					/* T&Pクリアーしているとドックタグ選択画面へ */
					work->step = DOGTAGS_SELECT;
					work->sub_step=0;
					
				} else {
					work->ans=TITLE_MSG_MODEEND;
					work->step=0x10;
					work->sub_step=0;

					BgscrFadeout(work);
				}
#else
				work->ans=TITLE_MSG_MODEEND;
				work->step=0x10;
				work->sub_step=0;

				SetRadarType(work);

				GM_Configuration&=~GM_CONFIG_END_IF_FOUND;

				BgscrFadeout(work);
#endif
			}
			break;
		case -1:
			work->step=LEVEL_SELECT;
			work->sub_step=0;
			break;
		}
		break;
	case GAMEOVER_SELECT:
		switch(EndIfFoundSubStep(work)){
		case 1:
			SetRadarType(work);			

			if(work->gameover_cursor) GM_Configuration|=GM_CONFIG_END_IF_FOUND;
			else GM_Configuration&=~GM_CONFIG_END_IF_FOUND;

#ifdef PSX2 /* DOGTAGS仕様 */
			if((GM_TitleMenuStatus & (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD))
			   ==(TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD)){
				/* T&Pクリアーしているとドックタグ選択画面へ */
				work->step = DOGTAGS_SELECT;
				work->sub_step=0;
				
			} else {
				work->ans=TITLE_MSG_MODEEND;
				work->step=0x10;
				work->sub_step=0;
				
				BgscrFadeout(work);
			}
#else
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;
			work->sub_step=0;

			SetRadarType(work);

			if(work->gameover_cursor) GM_Configuration|=GM_CONFIG_END_IF_FOUND;
			else GM_Configuration&=~GM_CONFIG_END_IF_FOUND;

			BgscrFadeout(work);
#endif
			break;
		case -1:
			work->step=RADAR_SELECT;
			work->sub_step=0;
			break;
		}
		break;

#ifdef PSX2 /**** 新旧ドックタグ選択はPS2版のみの仕様 ****/
	  case DOGTAGS_SELECT:
		switch( DogTagsSubStep(work) ){
		  case 1:
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;
			work->sub_step=0;			

			BgscrFadeout(work);
			break;
		  case -1:
			/* 前の画面へ */
			if(work->level_cursor>DIF_CURSOR_POS_NORMAL){
				/* ハード以上はGMAEOVER選択画面がある */
				work->step=GAMEOVER_SELECT;
				work->sub_step=0;
			} else {
				/* ノーマル以下はレーダー選択画面に戻る */
				work->step = RADAR_SELECT;
				work->sub_step = 0;
			}
			break;
		}
		break;
#endif

#if 0
	case YESNO_SELECT:
		switch(YesNoSubStep(work)){
		case 1:
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;

			if(work->yesno_cursor){
				GM_Configuration&=~GM_CONFIG_END_IF_FOUND;
			}
			else{
				GM_Configuration|=GM_CONFIG_END_IF_FOUND;
			}

			BgscrFadeout(work);
			break;
		case -1:
			/* 特殊 */
			work->step=LEVEL_SELECT;
			work->sub_step=1;
			break;
		}
		break;
#endif

	case 0x10:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			GV_DestroyActor(work);

			if(work->strman!=NULL){
				GV_DestroyOtherActor(work->strman);
				work->strman=NULL;
			}
		}
		break;
	}

	AnimCtrl(work);
}

static void Act(Work *work)
{
   SPR_OBJ *pTestObject;


   // make sure the title underline is the correct thickness and position.
   pTestObject = L2D_GetObject(work->l2d_handle,0x08dd8e0);
   pTestObject->sprite.pos.x= 32;
   pTestObject->sprite.pos.y= 49;
   pTestObject->sprite.dh= 1;



#if BP_TGS_DEMO()
   {
      SPR_OBJ *pSampleAlphaObject;
      char greyedOutAlpha = 164;     // current value is 52, so we need to lower this
      pSampleAlphaObject = L2D_GetObject(work->l2d_handle,1833025);   //tanker-plant
      if ( pSampleAlphaObject )
      {
         if ( pSampleAlphaObject->sprite.col.a < 52 )    // 52 is the standard unselected alpha value
         {
            greyedOutAlpha = pSampleAlphaObject->sprite.col.a;
         }
      }

      // grey out the options we don't need for TGS.
      pTestObject = L2D_GetObject(work->l2d_handle,5326606);   //tanker
      if ( pTestObject )
      {
         pTestObject->sprite.col.a = greyedOutAlpha;
         pTestObject->sprite.col.r = (5);
         pTestObject->sprite.col.g = (10);
         pTestObject->sprite.col.b = (10);
      }
      pTestObject = L2D_GetObject(work->l2d_handle,10999459);   //plant
      if ( pTestObject )
      {
         pTestObject->sprite.col.a = greyedOutAlpha;
         pTestObject->sprite.col.r = (5);
         pTestObject->sprite.col.g = (10);
         pTestObject->sprite.col.b = (10);
      }
   }
#endif




#ifdef DEBUG_MODE ///debug
	if( GV_PadData[1].status & PAD_L1 && GV_PadData[1].status & PAD_R1 && GV_PadData[1].status & PAD_X ){
		GM_TitleMenuStatus |= (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD);
	}
	if( GV_PadData[1].status & PAD_L2 && GV_PadData[1].status & PAD_R2  && GV_PadData[1].status & PAD_X ){
		GM_TitleMenuStatus &= ~(TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD);
	}
#endif

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

	ProgMessage(work);

	Key(work);
	Step(work);
}

static void EndStr(Work *work);

static void Die(Work *work)
{
#ifdef DEBUG_MODE
	printf("GM_Configuration = 0x%04x\n",GM_Configuration);
#endif

	EndStr(work);


	if(work->ans==TITLE_MSG_MODEEND){
		int val=work->game_cursor;

		/* 選択されたゲームタイプを保存 */
		GM_TitleMenuStatus&=~TITLE_MENU_STORY_SEL_MASK;

		if(val==0){
#if 0
			if(work->question_cursor<3) val=3;
			else val=2;
#else
			val = 3;/* TANKER-PLANT */
#endif
		}
		GM_TitleMenuStatus|=val;

		ResetLoaddataID();
	}

	CallLocalProc(work);

	if(work->parent_name!=0 &&
	   work->parent_name!=1){

		GV_MSG msg;
		int message[5];

		msg.address=work->parent_name;
		msg.message=message;
		msg.message_len=sizeof(message)/sizeof(message[0]);

		message[0]=work->ans;
		message[1]=work->game_cursor;
		message[2]=work->level_cursor;
		message[3]=work->yesno_cursor;
		message[4]=work->question_cursor;

		GV_SendMessage(&msg);
	}
}


/* ------------------------------------------------------------------------ */


static void InitStr(Work *work)
{
	int i;

	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,STR_NODE_FONT);
	if(spr==NULL) ASSERT(0);

#ifdef PSX2
	work->base_u=spr->sprite.head.tex.u;
	work->base_v=spr->sprite.head.tex.v;
#else
	work->base_u=(int)(spr->sprite.head.tex.u *16.0f);
	work->base_v=(int)(spr->sprite.head.tex.v *16.0f);
#endif

	spr->sprite.pos.x=FOUND_STR_X;
	spr->sprite.pos.y=FOUND_STR_Y;
	spr->sprite.dw=SK_FONT_WIDTH;
	spr->sprite.dh=SK_FONT_HEIGHT;

	for(i=0;i<FOUND_STR_LEN;i++){
		work->found_str[i]=SPR_DuplicateTree(spr);
	}

	for(i=0;i<YESNO_STR_LEN;i++){
		work->yesno_str[i]=SPR_DuplicateTree(spr);
	}

	// HideYesNo(work);
	// HideFound(work);

   CreateTrophiesExplain(work);
}

static void EndStr(Work *work)
{
	int i;

	for(i=0;i<FOUND_STR_LEN;i++){
		if(work->found_str[i]!=NULL) SPR_Destroy_2D_Object(work->found_str[i]);
	}
	for(i=0;i<YESNO_STR_LEN;i++){
		if(work->yesno_str[i]!=NULL) SPR_Destroy_2D_Object(work->yesno_str[i]);
	}
}

static void InitMenu(Work *work)
{
	work->cursor=L2D_GetObject(work->l2d_handle,CODE_SELCURSNULL);
	work->cursor_spr=L2D_GetObject(work->l2d_handle,CODE_SELCURS);

	work->game[0]=L2D_GetObject(work->l2d_handle,CODE_MENU1ST);
	work->game[1]=L2D_GetObject(work->l2d_handle,CODE_MENUTANKER);
	work->game[2]=L2D_GetObject(work->l2d_handle,CODE_MENUPLANT);
	work->game[3]=L2D_GetObject(work->l2d_handle,CODE_MENUT_P);

	work->level[0]=L2D_GetObject(work->l2d_handle,CODE_MENUVEASY);
	work->level[1]=L2D_GetObject(work->l2d_handle,CODE_MENUEASY);
	work->level[2]=L2D_GetObject(work->l2d_handle,CODE_MENUNORMAL);
	work->level[3]=L2D_GetObject(work->l2d_handle,CODE_MENUHARD);
	work->level[4]=L2D_GetObject(work->l2d_handle,CODE_MENUVHARD);

	work->level[5]=L2D_GetObject(work->l2d_handle,CODE_MENUEEX);


	work->yesno[0]=L2D_GetObject(work->l2d_handle,CODE_MENUON);
	work->yesno[1]=L2D_GetObject(work->l2d_handle,CODE_MENUOFF);

	work->radar[0]=L2D_GetObject(work->l2d_handle,CODE_RADARTYPE1);
	work->radar[1]=L2D_GetObject(work->l2d_handle,CODE_RADARTYPE2);
	work->radar[2]=L2D_GetObject(work->l2d_handle,CODE_RADARTYPE3);

	work->gameover[0]=L2D_GetObject(work->l2d_handle,CODE_NOTGAMEOVER);
	work->gameover[1]=L2D_GetObject(work->l2d_handle,CODE_GAMEOVER);

#ifdef PSX2 /* DOGTAGS仕様 */
	/* デフォルトが2002版 */
	GM_Configuration2 |= GM_CONFIG_DOGTAGS_2002;
	work->dogtags[0]=L2D_GetObject(work->l2d_handle,CODE_DOGTAGS2001);
	work->dogtags[1]=L2D_GetObject(work->l2d_handle,CODE_DOGTAGS2002);
#endif

	work->game_cursor=0;

	/* Normalが標準位置 */
	work->level_cursor=DIF_CURSOR_POS_NORMAL;

	work->yesno_cursor=0;
	work->question_cursor=-1;

	/* TYPE 2が標準 */
	work->radar_cursor=1;

	work->gameover_cursor=0;

#ifndef ENABLE_ALL_GAME_SELECT
	work->disp_game_item=1;

#if 1
#if 1
	/* サブスタンス仕様 */
	/* クリアーしていなくても好きなところから遊べる */
	work->disp_game_item = 0x0f;

#else 
	/* 日本版以降はタンカーもしくはプラントをクリアしていると
	   全てのメニューを出す */
	if(GM_TitleMenuStatus & (TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD)){
		work->disp_game_item|=2|4;
	}

	if(work->disp_game_item==7) work->disp_game_item|=8;

	work->disp_game_item & ~( 0x01 );
#endif
#else
	work->disp_game_item|=4;
#endif

#else

	GM_TitleMenuStatus|=TITLE_MENU_TANKER_CLEARD|TITLE_MENU_PLANT_CLEARD;

	work->disp_game_item=0x0f;

#endif

	InitStr(work);
}

/* 初期化部メイン */
void *NewNewGameScr(int parent_name,int l2d_handle,int proc,int bgname)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->parent_name=parent_name;
		work->name=NEWGAME_ACTORNAME;
		work->proc=proc;
		work->bgname=bgname;
		work->l2d_handle=l2d_handle;
		work->action_strcode=0;
		work->busy_flag=0;

		work->step=0;
		work->sub_step=0;
		work->ans=0;
		work->question_first_flag=0;
		work->explain_disp_flag=0;


		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if((work->strman=NewTextScreenControlForTitle())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		InitMenu(work);


    }
    return (void *)work ;
}


/* ------------------------------------------------------------------------ */


static void *GetLinkvariableAddr(void *addr)
{
	void *linkvar=GCL_GetLinkvarSaveAreaTop();
	return (void *)((int)linkvar+(((int)addr)-((int)linkvarbuf)));
}


static void ShowGameMenuFor2nd(Work *work)
{
	switch(work->disp_game_item){
	case 1:
		/* 一気にDifficultyに飛ぶはずなので、特に関係ない */
		SPR_HIDE(work->game[0]);
		SPR_HIDE(work->game[1]);
		SPR_HIDE(work->game[2]);
		// SPR_SHOW(work->game[3]);

		work->game[3]->sprite.col.a=work->game[0]->sprite.col.a;
		work->game[3]->sprite.pos.y=work->game[0]->sprite.pos.y;
		break;
	case 3:
		SPR_HIDE(work->game[0]);
		// SPR_SHOW(work->game[1]);
		SPR_HIDE(work->game[2]);
		// SPR_SHOW(work->game[3]);

		work->game[3]->sprite.col.a=work->game[0]->sprite.col.a;
		work->game[3]->sprite.pos.y=work->game[0]->sprite.pos.y;
		break;
	case 5:
		SPR_HIDE(work->game[0]);
		SPR_HIDE(work->game[1]);
		// SPR_SHOW(work->game[2]);
		// SPR_SHOW(work->game[3]);

		work->game[3]->sprite.col.a=work->game[0]->sprite.col.a;
		work->game[2]->sprite.pos.y=work->game[1]->sprite.pos.y;
		work->game[3]->sprite.pos.y=work->game[0]->sprite.pos.y;
		break;
	default:
		SPR_HIDE(work->game[0]);
		// SPR_SHOW(work->game[1]);
		// SPR_SHOW(work->game[2]);
		// SPR_SHOW(work->game[3]);

		work->game[3]->sprite.col.a=work->game[0]->sprite.col.a;
		//work->game[3]->sprite.pos.y=work->game[0]->sprite.pos.y;
		break;
	}
}

static int GameSubStepFor2nd(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			if(work->start_flag) work->action_strcode=CODE_SHOWSELGAME_FROMDIF_AFTERCLEAR;
			else work->action_strcode=CODE_SHOWSELGAME_AFTERCLEAR;

			work->start_flag=1;
			work->question_cursor=-1;
			work->sub_step++;
		}
		break;
	case 1:
		ShowGameMenuFor2nd(work);

		if(!work->busy_flag){
			work->sub_step++;

			LocalShowCursor(work);

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->game[work->game_cursor]->sprite.pos.y;
			work->game[work->game_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:
		ShowGameMenuFor2nd(work);

		if(work->anim_count>0) break;

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_BACKLOADGAME_AFTERCLEAR;
			HideCursor(work);

			work->sub_step=0x20;
			SE_CANCEL();

			BgscrFadeout(work);
			break;
		}
		else if(work->key_press & PAD_OK){
			work->action_strcode=CODE_HIDESELGAME_AFTERCLEAR;
			HideCursor(work);

			work->sub_step=0x10;
			SE_OK();
			break;
		}

		{
			int cur0,cur1;

			cur0=work->game_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				do{
					work->game_cursor--;
					if(work->game_cursor<0){
						work->game_cursor=N_GAME_ITEMS-1;
					}
				} while(!(work->disp_game_item & (1<<work->game_cursor)));
				break;
			case PAD_D:
				do{
					work->game_cursor++;
					if(work->game_cursor>=N_GAME_ITEMS){
						work->game_cursor=0;
					}
				} while(!(work->disp_game_item & (1<<work->game_cursor)));
				break;
			}

			cur1=work->game_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;

				work->t_ypos=work->game[work->game_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->game[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->game[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
		ShowGameMenuFor2nd(work);
		if(work->busy_flag) break;
		return 1;
	case 0x20:
		ShowGameMenuFor2nd(work);
		if(work->busy_flag) break;
		return -1;
	}

	return 0;
}

static int LevelSubStepFor2nd(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			ShowLevel(work);

			work->action_strcode=CODE_SHOWDIFALL_AFTERCLEAR;

			work->sub_step++;
		}
		break;
	case 1:
		DispExtreme(work);
		// ShowLevel(work);

		if(!work->busy_flag){
			SPR_OBJ *parent;

         work->explain_disp_flag=1;

			work->sub_step++;

			LocalShowCursor(work);

			parent=work->level[work->level_cursor]->head.parent;

			work->anim_count=0;

			work->t_ypos=work->level[work->level_cursor]->sprite.pos.y+5;
			while(parent!=NULL){
				work->t_ypos+=parent->empty.pos.y;
				parent=parent->head.parent;
			}
			work->cursor->empty.pos.y=work->t_ypos;

			work->level[work->level_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:

      CreateDifficultyExplain(work,work->level_cursor);
      PrintDifficultyExplain(work);

		DispExtreme(work);

		if(work->anim_count>0) break;

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_HIDEDIFALLTOSEL_AFTERCLEAR;
			HideCursor(work);

			SE_CANCEL();

			work->sub_step=0x10;
			work->ans=-1;

			return 0;
		}
		else if(work->key_press & PAD_OK){
#if 0
			if(work->level_cursor<=DIF_CURSOR_POS_NORMAL){
				work->action_strcode=CODE_HIDEDIFALL_AFTERCLEAR;
			}
#else
			work->action_strcode=CODE_HIDEDIFALL_AFTERCLEAR;
#endif
			HideCursor(work);

			SE_OK();

			work->sub_step=0x10;
			work->ans=1;

			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->level_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->level_cursor--;
				if(work->level_cursor<work->level_l){
					work->level_cursor=work->level_u;
				}
				break;
			case PAD_D:
				work->level_cursor++;
				if(work->level_cursor>work->level_u){
					work->level_cursor=work->level_l;
				}
				break;
			}

			cur1=work->level_cursor;

			if(cur0!=cur1)
         {
				SPR_OBJ *parent;

            work->explain_disp_flag=1;

				work->anim_count=ANIM_COUNT;

				parent=work->level[work->level_cursor]->head.parent;
				work->t_ypos=work->level[work->level_cursor]->sprite.pos.y+5;
				while(parent!=NULL){
					work->t_ypos+=parent->empty.pos.y;
					parent=parent->head.parent;
				}

				AnimCtrlSet(work,0,work->level[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->level[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
		DispExtreme(work);

		if(work->busy_flag) break;
		return work->ans;
	}

	return 0;
}


static void SetRadarTypeFor2nd(Work *work)
{
	short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);

	switch(work->radar_cursor){
	case 0:
		/* TYPE 1 */
		*gm_config&=~(GM_CONFIG_RADAR_OFF_INTRUDE|GM_CONFIG_RADAR_OFF);
		break;
	case 1:
		/* TYPE 2 */
		*gm_config|=GM_CONFIG_RADAR_OFF_INTRUDE;
		*gm_config&=~GM_CONFIG_RADAR_OFF;
		break;
	case 2:
		/* OFF */
		*gm_config&=~GM_CONFIG_RADAR_OFF_INTRUDE;
		*gm_config|=GM_CONFIG_RADAR_OFF;
		break;
	}
}

static void GetRadarTypeFor2nd(Work *work)
{
	short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);

	switch(*gm_config & (GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE)){
	case 0:
		/* TYPE 1 */
		work->radar_cursor=0;
		break;
	case GM_CONFIG_RADAR_OFF_INTRUDE:
		/* TYPE 2 */
		work->radar_cursor=1;
		break;
	case GM_CONFIG_RADAR_OFF:
	case GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE:
		/* OFF */
		work->radar_cursor=2;
		break;
	}
}

static int RadTypeSubStepFor2nd(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			work->action_strcode=CODE_SHOWRADTYPE_AFTERCLEAR;

			work->sub_step++;

			GetRadarTypeFor2nd(work);
		}
		break;
	case 1:
		if(!work->busy_flag){
			work->sub_step++;

			LocalShowCursor(work);

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->radar[work->radar_cursor]->sprite.pos.y;
			work->radar[work->radar_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:
		PrintExplain(work);

		if(work->anim_count>0) break;

		CreateRadarExplain(work);

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_BACKFROMRADTYPE_AFTERCLEAR;

			HideCursor(work);

			SE_CANCEL();

			work->sub_step=0x10;
			work->ans=-1;

			return 0;
		}
		else if(work->key_press & PAD_OK){
			if(work->level_cursor<=DIF_CURSOR_POS_NORMAL){
				/* VERY EASY, EASY, NORMAL */
				work->action_strcode=CODE_HIDERADTYPE_AFTERCLEAR;
			}
			else{
				/* HARD, EXTREME */
				work->action_strcode=CODE_HIDERADTYPE_TO_GAMEOVER_AFTERCLEAR;
			}

			HideCursor(work);

			SE_OK();

			work->sub_step=0x10;
			work->ans=1;

			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->radar_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->radar_cursor--;
				if(work->radar_cursor<0){
					work->radar_cursor=N_RADAR_TYPES-1;
				}
				break;
			case PAD_D:
				work->radar_cursor++;
				if(work->radar_cursor>=N_RADAR_TYPES){
					work->radar_cursor=0;
				}
				break;
			}

			cur1=work->radar_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;
				work->explain_disp_flag=0;

				work->t_ypos=work->radar[work->radar_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->radar[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->radar[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
		if(work->busy_flag) break;
		return work->ans;
	}

	return 0;
}

static int EndIfFoundSubStepFor2nd(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			work->action_strcode=CODE_SHOWGAMEOVER_AFTERCLEAR;

			work->gameover_cursor=0;
			work->sub_step++;

			work->explain_disp_flag=0;
		}
		break;
	case 1:
		if(!work->busy_flag){
			work->sub_step++;

			LocalShowCursor(work);

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y=work->gameover[work->gameover_cursor]->sprite.pos.y;
			work->gameover[work->gameover_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:

		PrintExplain(work);

		if(work->anim_count>0) break;

		CreateGameoverExplain(work);

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_BACKFROMGAMEOVER_AFTERCLEAR;

			HideCursor(work);

			SE_CANCEL();

			work->sub_step=0x10;
			work->ans=-1;

			return 0;
		}
		else if(work->key_press & PAD_OK){
			work->action_strcode=CODE_HIDEGAMEOVER_AFTERCLEAR;

			HideCursor(work);

			SE_OK();

			work->sub_step=0x10;
			work->ans=1;

			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->gameover_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->gameover_cursor--;
				if(work->gameover_cursor<0){
					work->gameover_cursor=N_GAMEOVER_ITEMS-1;
				}
				break;
			case PAD_D:
				work->gameover_cursor++;
				if(work->gameover_cursor>=N_GAMEOVER_ITEMS){
					work->gameover_cursor=0;
				}
				break;
			}

			cur1=work->gameover_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;
				work->explain_disp_flag=0;

				work->t_ypos=work->gameover[work->gameover_cursor]->sprite.pos.y;

				AnimCtrlSet(work,0,work->gameover[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->gameover[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
		if(work->busy_flag) break;
		return work->ans;
	}

	return 0;
}

static void StepFor2nd(Work *work)
{
	switch(work->step){
	case INIT_FOR_2ND:
		InitMenu(work);

		/* メニューは最大でも三つまでしか出ない。*/
		work->disp_game_item&=~8;

		BgscrFadein(work);
		work->step++;
		break;
	case GAME_SELECT_FOR_2ND:
		switch(GameSubStepFor2nd(work)){
		case 1:
			work->step=LEVEL_SELECT_FOR_2ND;
			work->sub_step=0;

			/* Normalが標準位置 */
			work->level_cursor=DIF_CURSOR_POS_NORMAL;
			break;
		case -1:
			work->ans=TITLE_MSG_MODECANCEL;
			work->step=0x10;
			break;
		}
		break;
	case LEVEL_SELECT_FOR_2ND:
		switch(LevelSubStepFor2nd(work)){
		case 1:
#if 0
			if(work->level_cursor>DIF_CURSOR_POS_NORMAL){
				work->step=YESNO_SELECT_FOR_2ND;
				work->sub_step=0;
			}
			else{
				work->ans=TITLE_MSG_MODEEND;
				work->step=0x10;

				{
					short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);
					*gm_config&=~GM_CONFIG_END_IF_FOUND;
				}

				BgscrFadeout(work);
			}
#else
			work->step=RADAR_SELECT_FOR_2ND;
			work->sub_step=0;
#endif
			break;
		case -1:
			work->step=GAME_SELECT_FOR_2ND;
			work->sub_step=0;
			break;
		}
		break;

	case RADAR_SELECT_FOR_2ND:
		switch(RadTypeSubStepFor2nd(work)){
		case 1:
			if(work->level_cursor>DIF_CURSOR_POS_NORMAL){
				work->step=GAMEOVER_SELECT_FOR_2ND;
				work->sub_step=0;
			}
			else{
				work->ans=TITLE_MSG_MODEEND;
				work->step=0x10;
				work->sub_step=0;

				SetRadarTypeFor2nd(work);

				{
					short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);
					*gm_config&=~GM_CONFIG_END_IF_FOUND;
				}

				BgscrFadeout(work);
			}
			break;
		case -1:
			work->step=LEVEL_SELECT_FOR_2ND;
			work->sub_step=0;
			break;
		}
		break;
	case GAMEOVER_SELECT_FOR_2ND:
		switch(EndIfFoundSubStepFor2nd(work)){
		case 1:
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;
			work->sub_step=0;

			SetRadarTypeFor2nd(work);

			{
				short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);

				if(work->gameover_cursor) *gm_config|=GM_CONFIG_END_IF_FOUND;
				else *gm_config&=~GM_CONFIG_END_IF_FOUND;
			}

			BgscrFadeout(work);
			break;
		case -1:
			work->step=RADAR_SELECT_FOR_2ND;
			work->sub_step=0;
			break;
		}
		break;

#if 0
	case YESNO_SELECT_FOR_2ND:
		switch(YesNoSubStep(work)){
		case 1:
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;

			if(work->yesno_cursor){
				short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);
				*gm_config&=~GM_CONFIG_END_IF_FOUND;
			}
			else{
				short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);
				*gm_config|=GM_CONFIG_END_IF_FOUND;
			}

			BgscrFadeout(work);
			break;
		case -1:
			/* 特殊 */
			work->step=LEVEL_SELECT_FOR_2ND;
			work->sub_step=1;
			break;
		}
		break;
#endif

	case 0x10:
		if(ComNodeFrameBusy()) break;

		if(!work->busy_flag){
			GV_DestroyActor(work);

			if(work->strman!=NULL){
				GV_DestroyOtherActor(work->strman);
				work->strman=NULL;
			}
		}
		break;
	}

	AnimCtrl(work);
}


/* ------------------------------------------------------------------------ */


static void ActFor2nd(Work *work)
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

	ProgMessage(work);

	Key(work);
	StepFor2nd(work);
}

static void DieFor2nd(Work *work)
{
#ifdef DEBUG_MODE
	{
		short *gm_config=(short *)GetLinkvariableAddr(&GM_Configuration);
		printf("GM_Configuration = 0x%04x\n",*gm_config);
	}
#endif

	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}

	if(work->game_cursor==0) work->game_cursor=3;

	if(work->ans==TITLE_MSG_MODEEND){
		short *titlemenu=(short *)GetLinkvariableAddr(&GM_TitleMenuStatus);
		short *level=(short *)GetLinkvariableAddr(&GM_GameLevel);
		static const short val_diffculty[]={
			GM_LEVEL_VERYEASY,
			GM_LEVEL_EASY,
			GM_LEVEL_NORMAL,
			GM_LEVEL_HARD,
			GM_LEVEL_EXTREME,
		};

		/* 選択されたゲームタイプを保存 */
		*titlemenu&=~TITLE_MENU_STORY_SEL_MASK;
		*titlemenu|=work->game_cursor;

		*level=val_diffculty[work->level_cursor];
	}

	CallLocalProcFor2nd(work);
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->proc=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('b')!=NULL){
		work->bgname=GCL_GetNextInt();
    }
}

static void LayoutSignalFunc(Work *work,int sign,int value)
{
	ComNodeFrameAction(sign);

#ifdef DEBUG_MODE
	printf("Layout Signal = %d ( 0x%08x )\n",sign,sign);
#endif

}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

	work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,MENU_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG
		printf("Select Scr : L2D Init Error = %d\n",work->l2d_handle);
#endif
		ASSERT(0);
	}

	/* シグナルハンドラの設定 */
	L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);

	/* layoutの初期化 */
	L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);

    return 1;
}

/* 初期化部メイン */
void *NewClearedGameMenuScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->parent_name=0;
		work->name=name;

		work->action_strcode=0;
		work->busy_flag=0;

		work->step=0;
		work->sub_step=0;
		work->ans=0;
		work->question_first_flag=1;
		work->start_flag=0;
		work->explain_disp_flag=0;

		GV_SetActor(&(work->actor),ActFor2nd,DieFor2nd);
		GV_ActorEX(&(work->actor));

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL;
		}
		if((work->strman=NewTextScreenControlForTitle())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		// InitMenu(work);
    }
    return (void *)work ;
}



void CreateDifficultyExplain(Work *work, int whichText)
{
   unsigned char* temp;
   int columns;
   char *str;
   int i;

   if ( !work->explain_disp_flag )
      return;

   switch(whichText)
   {
      case DIF_CURSOR_POS_VEASY:
         {
            str=BP_GetStringForEnum(kBP_GLS_Options_ExplainDifficultyVeryEasy);
         }
         break;
      case DIF_CURSOR_POS_EASY:
         {
            str=BP_GetStringForEnum(kBP_GLS_Options_ExplainDifficultyEasy);
         }
         break;
      case DIF_CURSOR_POS_NORMAL:
         {
            str=BP_GetStringForEnum(kBP_GLS_Options_ExplainDifficultyNormal);
         }
         break;
      case DIF_CURSOR_POS_HARD:
         {
            str=BP_GetStringForEnum(kBP_GLS_Options_ExplainDifficultyHard);
         }
         break;
      case DIF_CURSOR_POS_EXTREME:
         {
            str=BP_GetStringForEnum(kBP_GLS_Options_ExplainDifficultyExtreme);
         }
         break;
      case DIF_CURSOR_POS_E_EXTREME:
         {
            str=BP_GetStringForEnum(kBP_GLS_Options_ExplainDifficultyExtremeExtreme);
         }
         break;

      default:
         BP_BREAK;
   }

      columns=1;
      temp = str;

      for( ;; )
      {
         int code;

         temp = BP_font_decode_utf8_character(&code, temp);

         if( code == 0 )
            break;

         //if( code == '|')
         //   work->explain_columns++;
      }

      //MENU_ClearTextTexture(work->strman);
      MENU_ClearPartTextTexture(work->strman, 0, EXPLAIN_HEIGHT);

      // spaceを無理やり設定する。
      MENU_SetTextSpace( work->strman, DOWN_MARGINE );
      MENU_CreateTextTexture(work->strman,
         OPTION_EXPLAIN_U,
         OPTION_EXPLAIN_V,
         OPTION_EXPLAIN_WIDTH,
         OPTION_EXPLAIN_HEIGHT,
         0,0,0,str);
      // ほかに影響が出ないように規定値に戻す。
      MENU_SetTextSpace( work->strman, 12 );

      work->explain_disp_flag=0;
}

void PrintDifficultyExplain(Work *work)
{
      int x = OPTION_EXPLAIN_X;
      int y = OPTION_EXPLAIN_Y;
      int w = OPTION_EXPLAIN_W; // * 3 / 4; // correct for 16:9
      int h = OPTION_EXPLAIN_H;

      MENU_PutTextScreen(work->strman,
         x,
         y,
         x + w,
         y + h,
         OPTION_EXPLAIN_U,
         OPTION_EXPLAIN_V,
         OPTION_EXPLAIN_U + OPTION_EXPLAIN_WIDTH,
         OPTION_EXPLAIN_V + OPTION_EXPLAIN_HEIGHT,
         TEXTCOLOR);
}