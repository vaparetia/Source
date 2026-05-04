//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	portsel_for_photosave.c
		ポートセレクト画面

	2001/07/14 K.Kano
	$Id: portsel_for_photosave.c,v 1.1.1.3 2002/11/19 11:43:40 Yoshizawa1 Exp $
*/

#if 0 //BP_PS2 def PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>
#endif

#include "BP_BuildDefines.h"
#include "BP_Font.h"

#include "gameheader.h"
#include "libutl.h"

#include "libfs.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"

#include "../mcman/mcman.h"

#include "../../mode/codec/codecmem.h"

#include "titlescr.h"
#include "portsel.h"

#include "font.h"

#include "mode/menu/xtextscn.h"

extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え


/* text_scn.c */

/* プロトタイプ宣言 */
/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work );
void MENU_ClearPartTextTexture( void *work, int start_line, int height );
/* 説明文を表示する */
void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
/* テキストテクスチャ管理デーモン */
void *NewTextScreenControl( void );
void *NewTextScreenControlForTitle( void );
void *NewTextScreenControlForCodec( void );



#define N_PORT_ITEMS		2

#define STRLEN				48


#define STR_NODE_FONT		0x0034ca34		/* font */

#define CODE_PHOTOSAVE_CURSOR		9100063			/* cursor */
#define CODE_PROG					0x0029d647		/* Prog */

#define CODE_TAG			0x0001dc87		/* tag */
#define CODE_TOP_BAR		0x00291581		/* topBar */


#define	STR_WIDTH			(18) // strcode ではなく文字の長さ
#define STR_HEIGHT			(14) // strcode ではなく文字の長さ


#define MSG_CHECKCARD		"CHECKING MEMORY CARD(PS2)"

#define MSG_SLOT1			"MEMORY CARD SLOT 1"
#define MSG_SLOT2			"MEMORY CARD SLOT 2"

#define MSG_NO_MEMORYCARD	"MEMORY CARD(PS2) NOT INSERTED"

#define MSG_SAVING			"DATA SAVE"

#define MSG_NO_FREEAREA		"INSUFFICIENT SPACE ON MEMORY CARD(PS2)"


#define FONT_WIDTH			(18.0f-2.0f)
#define FONT_HEIGHT			(14.0f-2.0f)
#define FONT_SPACE_HEIGHT	4.0f  /* DRAW_HEIGHT / 384.0f */


#if 0
#define MC_CHECK_X			20.0f
#define MC_CHECK_Y			(353.0f-(FONT_HEIGHT+0.0f))
#else
#define MC_CHECK_X			20.0f
#define MC_CHECK_Y			(310.0f+12.0f)
#endif

#define SLOT1_X				39.0f
#define SLOT1_Y				(86.0f+(FONT_HEIGHT+FONT_SPACE_HEIGHT)*3)
#define SLOT2_X				39.0f
#define SLOT2_Y				(86.0f+(FONT_HEIGHT+FONT_SPACE_HEIGHT)*4)

#define NOCARD_X			39.0f
#define NOCARD_Y			(86.0f+(FONT_HEIGHT+FONT_SPACE_HEIGHT)*3)

#define NOFREEAREA_X		39.0f
#define NOFREEAREA_Y		(86.0f+(FONT_HEIGHT+FONT_SPACE_HEIGHT)*3)

#if 0
#define DATA_SAVING_X		20.0f
#define DATA_SAVING_Y		(353.0f-(FONT_HEIGHT+0.0f))
#else
#define DATA_SAVING_X		(59.0f)
#define DATA_SAVING_Y		(310.0f+12.0f)
#endif


static const char *slotstr[]={
	MSG_SLOT1,
	MSG_SLOT2,
};

static const float slotxy[][2]={
	{ SLOT1_X,SLOT1_Y, },
	{ SLOT2_X,SLOT2_Y, },
};


enum {
	ANIM_CTRL_CHECKCARD=0,
	ANIM_CTRL_PORT0,
	ANIM_CTRL_PORT1,
	ANIM_CTRL_NO_MEMCARD,
	ANIM_CTRL_NO_FREEAREA,
	ANIM_CTRL_DATA_SAVING,

	ANIM_CTRL_MAX,
};

typedef struct __portsel_for_photosave_work {
	GV_ACT_EX actor;

	int busy_flag;
	int l2d_handle;
	int action_strcode;

	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int ans;

	int port_cursor;
	int port_mode;

	int base_u,base_v;

   int start_mode;

	SPR_OBJ *checkcard[STRLEN];
	SPR_OBJ *port[N_PORT_ITEMS][STRLEN];
	SPR_OBJ *no_memcard[STRLEN];
	SPR_OBJ *no_freearea[STRLEN];

	SPR_OBJ **data_saving;
	int data_saving_len;

	int anim_count;
	struct {
		unsigned char now_alpha;
		unsigned char tar_alpha;
	} anim_ctrl[ANIM_CTRL_MAX];

	float cursor_y,cursor_ty;

	void *strman;

	SPR_OBJ *cursor;
} Work;


#define CODE_SHOWSLOT			0x002cb484			/* showBtm */
#define CODE_SHOWFILE			0x002cfbe7			/* showTop */
#define CODE_HIDEALL			0x0073acc4			/* hideAll */


#define SEL_ALPHA		128
#define NOSEL_ALPHA		48


#define PORTSEL_ANS_MODEEND		1
#define PORTSEL_ANS_MODECANCEL	-1


#define ANIM_CTRL_COUNT			DIRECT_TICK(20)
#define CURSOR_MOVE_COUNT		DIRECT_TICK(10)
#define ANIM_CTRL_WAIT_COUNT	DIRECT_TICK(6)


#if 0
#define FONT_TEXT_WIDTH		24
#define FONT_TEXT_HEIGHT	24
#else
#define FONT_TEXT_WIDTH		FONT_SIZE_W
#define FONT_TEXT_HEIGHT	FONT_SIZE_H
#endif
#define FONT_TEXT_HEIGHT2	32
#define FONT_DISP_WIDTH		15
#define FONT_DISP_HEIGHT	15
#define LINE_SPACE			RUBI_SIZE_H
#define LINE_HEIGHT			(FONT_HEIGHT+LINE_SPACE)

#define NOCARD_U			0
#define NOCARD_V			0
#define NOCARD_WIDTH		(30*FONT_TEXT_WIDTH)
#if 0
#define NOCARD_HEIGHT		(1*FONT_TEXT_HEIGHT)
#define NOCARD_HEIGHT2		(1*FONT_TEXT_HEIGHT2)
#else
#define NOCARD_HEIGHT		(1*LINE_HEIGHT+DOWN_MARGINE)
#define NOCARD_HEIGHT2		NOCARD_HEIGHT
#endif
#define NOFREEAREA_U		0
#if 0
#define NOFREEAREA_V		(1*FONT_TEXT_HEIGHT2)
#else
#define NOFREEAREA_V		(NOCARD_V+NOCARD_HEIGHT)
#endif
#define NOFREEAREA_WIDTH	(30*FONT_TEXT_WIDTH)
#if 0
#define NOFREEAREA_HEIGHT	(1*FONT_TEXT_HEIGHT)
#define NOFREEAREA_HEIGHT2	(1*FONT_TEXT_HEIGHT2)
#else
#define NOFREEAREA_HEIGHT	(1*LINE_HEIGHT+DOWN_MARGINE)
#define NOFREEAREA_HEIGHT2	NOFREEAREA_HEIGHT
#endif

#define NOCARD_TEXT_X		((int)NOCARD_X)
#define NOCARD_TEXT_Y		((int)NOCARD_Y)
#if 0
#define NOCARD_TEXT_W		(30*FONT_DISP_WIDTH)
#define NOCARD_TEXT_H		(1*FONT_DISP_HEIGHT)
#else
#define NOCARD_TEXT_W		(NOCARD_WIDTH*FONT_DISP_WIDTH/FONT_TEXT_WIDTH)
#define NOCARD_TEXT_H		(NOCARD_HEIGHT*FONT_DISP_HEIGHT/FONT_TEXT_HEIGHT)
#endif
#define NOFREEAREA_TEXT_X	((int)NOFREEAREA_X)
#define NOFREEAREA_TEXT_Y	((int)NOFREEAREA_Y)
#if 0
#define NOFREEAREA_TEXT_W	(30*FONT_DISP_WIDTH)
#define NOFREEAREA_TEXT_H	(1*FONT_DISP_HEIGHT)
#else
#define NOFREEAREA_TEXT_W	(NOFREEAREA_WIDTH*FONT_DISP_WIDTH/FONT_TEXT_WIDTH)
#define NOFREEAREA_TEXT_H	(NOFREEAREA_HEIGHT*FONT_DISP_HEIGHT/FONT_TEXT_HEIGHT)
#endif


#define GLOBAL_SAVERES_ID	5


enum {
	RESOURCE_MCNOTFOUND=0,

	/* ロード時 : ファイルがありません */
	/* セーブ時 : セーブのための空き領域がありません */
	RESOURCE_ERROR,
};


#if 0
#define TEXTCOLOR		0x00808080
#else
#define R_VAL			80
#define G_VAL			90
#define B_VAL			85
#define TEXTCOLOR		((R_VAL)|(G_VAL<<8)|(B_VAL<<16))
#endif


#define REPEAT_FIRST		20
#define REPEAT_NEXT			3


static void InitMenu(Work *work);
static void InitIconEnv(Work *work);


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


enum {
	MCARD_START=0,
	MCARD_CHECK,
	MCARD_PORT_SEL,
	MCARD_FILE_CHECK,
	MCARD_ERROR,
};


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


static void AnimCtrlSet0(Work *work,int index,int alpha,int talpha)
{
	work->anim_ctrl[index].now_alpha=alpha;
	work->anim_ctrl[index].tar_alpha=talpha;
}

static void AnimCtrlSet(Work *work,int index,int talpha)
{
	work->anim_ctrl[index].tar_alpha=talpha;
}

static void AnimCtrl(Work *work)
{
	if(work->anim_count>0){
		int i;
		for(i=0;i<ANIM_CTRL_MAX;i++){
			work->anim_ctrl[i].now_alpha
				+=(((int)(work->anim_ctrl[i].tar_alpha))-((int)(work->anim_ctrl[i].now_alpha)))
				/work->anim_count;
		}

		work->cursor_y+=(work->cursor_ty-work->cursor_y)/(float)(work->anim_count);

		work->anim_count--;
	}

	SK_PrintfChengColor2(work->checkcard,R_VAL,G_VAL,B_VAL,
						 work->anim_ctrl[ANIM_CTRL_CHECKCARD].now_alpha,STRLEN);
	SK_PrintfChengColor2(work->port[0],R_VAL,G_VAL,B_VAL,
						 work->anim_ctrl[ANIM_CTRL_PORT0].now_alpha,STRLEN);
	SK_PrintfChengColor2(work->port[1],R_VAL,G_VAL,B_VAL,
						 work->anim_ctrl[ANIM_CTRL_PORT1].now_alpha,STRLEN);

#if 0
	SK_PrintfChengColor2(work->no_memcard,R_VAL,G_VAL,B_VAL,
						 work->anim_ctrl[ANIM_CTRL_NO_MEMCARD].now_alpha,STRLEN);

	SK_PrintfChengColor2(work->no_freearea,R_VAL,G_VAL,B_VAL,
						 work->anim_ctrl[ANIM_CTRL_NO_FREEAREA].now_alpha,STRLEN);
#else
	{
		int color;

		color=TEXTCOLOR | (work->anim_ctrl[ANIM_CTRL_NO_MEMCARD].now_alpha<<24);

		MENU_PutTextScreen(work->strman,
						   NOCARD_TEXT_X,
						   NOCARD_TEXT_Y,
						   NOCARD_TEXT_X+NOCARD_TEXT_W,
						   NOCARD_TEXT_Y+NOCARD_TEXT_H,
						   NOCARD_U,
						   NOCARD_V,
						   NOCARD_U+NOCARD_WIDTH,
						   NOCARD_V+NOCARD_HEIGHT,
						   color);

		color=TEXTCOLOR | (work->anim_ctrl[ANIM_CTRL_NO_FREEAREA].now_alpha<<24);

		MENU_PutTextScreen(work->strman,
						   NOFREEAREA_TEXT_X,
						   NOFREEAREA_TEXT_Y,
						   NOFREEAREA_TEXT_X+NOFREEAREA_TEXT_W,
						   NOFREEAREA_TEXT_Y+NOFREEAREA_TEXT_H,
						   NOFREEAREA_U,
						   NOFREEAREA_V,
						   NOFREEAREA_U+NOFREEAREA_WIDTH,
						   NOFREEAREA_V+NOFREEAREA_HEIGHT,
						   color);
	}
#endif

	SK_PrintfChengColor2(work->data_saving,R_VAL,G_VAL,B_VAL,
						 work->anim_ctrl[ANIM_CTRL_DATA_SAVING].now_alpha,work->data_saving_len);

	work->cursor->sprite.pos.y=work->cursor_y;
}

void SK_PrintfChengeY(SPR_OBJ **pObj,float y,int num)
{
	int i;
	for(i=0;i<num;i++){
		pObj[i]->sprite.pos.y=y;
	}
}


static void ShowSlotName(Work *work)
{
	switch(work->port_mode){
	case 0:
		SPR_HIDE(work->cursor);

		AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x80);
		// SK_AllShow(work->no_memcard,STRLEN);
		SK_AllHide(work->port[0],STRLEN);
		SK_AllHide(work->port[1],STRLEN);
		break;
	case 1:
		// SPR_SHOW(work->cursor);
		SPR_HIDE(work->cursor);

		AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
		// SK_AllHide(work->no_memcard,STRLEN);
		SK_AllShow(work->port[0],STRLEN);
		SK_AllHide(work->port[1],STRLEN);

		SK_PrintfChengeY(work->port[0],SLOT1_Y,STRLEN);
		break;
	case 2:
		// SPR_SHOW(work->cursor);
		SPR_HIDE(work->cursor);

		AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
		// SK_AllHide(work->no_memcard,STRLEN);
		SK_AllHide(work->port[0],STRLEN);
		SK_AllShow(work->port[1],STRLEN);

		SK_PrintfChengeY(work->port[1],SLOT1_Y,STRLEN);
		break;
	case 3:
		// SPR_SHOW(work->cursor);
		SPR_HIDE(work->cursor);

		AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
		// SK_AllHide(work->no_memcard,STRLEN);
		SK_AllShow(work->port[0],STRLEN);
		SK_AllShow(work->port[1],STRLEN);

		SK_PrintfChengeY(work->port[0],SLOT1_Y,STRLEN);
		SK_PrintfChengeY(work->port[1],SLOT2_Y,STRLEN);
		break;
	}
}


#if 0

static void HideSlotName(Work *work)
{
	AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
	// SK_AllHide(work->no_memcard,STRLEN);
	SK_AllHide(work->port[0],STRLEN);
	SK_AllHide(work->port[1],STRLEN);
}

static void ShowDataSaving(Work *work)
{
	SK_AllShow(work->data_saving,work->data_saving_len);
}

static void HideDataSaving(Work *work)
{
	SK_AllHide(work->data_saving,work->data_saving_len);
}

#endif


static void MakeTitle(Work *work)
{
	char title[0x80];

	strcpy(title,MSG_SAVING);
	strcat(title,"/");
	
   strcat(title,slotstr[work->port_cursor]);

	SK_PrintfNormal(NULL,work->data_saving,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,work->data_saving_len);
	//SK_PrintfNormal(title,work->data_saving,work->base_u,work->base_v,
	//				STR_WIDTH,STR_HEIGHT,work->data_saving_len);
	SK_PrintfChengColor2(work->data_saving,R_VAL,G_VAL,B_VAL,0x80,work->data_saving_len);
	SK_AllShow(work->data_saving,work->data_saving_len);
}



static void PortModeCheck(Work *work)
{
	work->port_mode=0;

	switch(MCManGetCardType(0)){
	case MCMAN_CARDTYPE_NOCARD:
	case MCMAN_CARDTYPE_PS1:
	case MCMAN_CARDTYPE_POCKET_STATION:

	case MCMAN_CARDTYPE_ERRORCARD:

	case MCMAN_CARDTYPE_MULTITAP:
		break;

	case MCMAN_CARDTYPE_PS2_FORMATTED:
	case MCMAN_CARDTYPE_PS2_UNFORMATTED:
		work->port_mode|=1;
		break;
	}

	switch(MCManGetCardType(1)){
	case MCMAN_CARDTYPE_NOCARD:
	case MCMAN_CARDTYPE_PS1:
	case MCMAN_CARDTYPE_POCKET_STATION:

	case MCMAN_CARDTYPE_ERRORCARD:

	case MCMAN_CARDTYPE_MULTITAP:
		break;

	case MCMAN_CARDTYPE_PS2_FORMATTED:
	case MCMAN_CARDTYPE_PS2_UNFORMATTED:
		work->port_mode|=2;
		break;
	}
}

static void HideTag(Work *work)
{
	SPR_OBJ *spr;
	spr=L2D_GetObject(work->l2d_handle,CODE_TAG);
	SPR_HIDE(spr);
	spr=L2D_GetObject(work->l2d_handle,CODE_TOP_BAR);
	SPR_HIDE(spr);
}

static int StartSubStep(Work *work)
{
	InitMenu(work);
	InitIconEnv(work);
	SPR_HIDE(work->cursor);
	return 1;
}

static int CheckSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag && work->anim_count==0){
			SPR_HIDE(work->cursor);
			// work->cursor->sprite.pos.x=SLOT1_X;
			work->cursor->sprite.col.r=R_VAL*2;
			work->cursor->sprite.col.g=G_VAL*2;
			work->cursor->sprite.col.b=B_VAL*2;
			work->cursor->sprite.col.a=128;
			work->cursor_y=work->cursor_ty=SLOT1_Y;

			work->action_strcode=CODE_SHOWSLOT;

			AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_DATA_SAVING,0x00,0x00);

			if(!MCManCheckingOrChecked()) return 1;
			if(MCManChecked()) return 1;

			work->sub_step++;

			work->anim_count=ANIM_CTRL_COUNT;
			AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x80);
#if 0
			AnimCtrlSet(work,ANIM_CTRL_PORT0,0x00);
			AnimCtrlSet(work,ANIM_CTRL_PORT1,0x00);
			AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
			AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x00);
			AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x00);
#else
			AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_DATA_SAVING,0x00,0x00);
#endif
		}
		break;
	case 1:
		if(!MCManCheckingOrChecked()){
			// SK_AllHide(work->checkcard,STRLEN);
			return 1;
		}
		if(MCManChecked()){
			// SK_AllHide(work->checkcard,STRLEN);
			return 1;
		}
		break;
	}

	return 0;
}

static int PortSelSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag && work->anim_count==0){
			work->anim_count=ANIM_CTRL_WAIT_COUNT;
			work->sub_step++;
		}
		break;
	case 1:
		if(!work->busy_flag && work->anim_count==0){
			PortModeCheck(work);
			switch(work->port_mode)
         {
         // BP DAK - If we get here and haven't selected a port, cancel out automatically
#if 1
         case 0:
            {
               // automatically press cancel
               SE_CANCEL();
               return -1;
            }
            break;
#endif
			case 1:
				work->port_cursor=0;
				break;
			case 2:
				work->port_cursor=1;
				break;
			}

			work->anim_count=ANIM_CTRL_COUNT;
			AnimCtrlSet(work,ANIM_CTRL_PORT0,NOSEL_ALPHA);
			AnimCtrlSet(work,ANIM_CTRL_PORT1,NOSEL_ALPHA);
			AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x80);
#if 0
			AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x00);
			AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x00);
#else
			AnimCtrlSet0(work,ANIM_CTRL_CHECKCARD,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
#endif
			ShowSlotName(work);

#if 0
			SK_PrintfChengColor2(work->port[0],R_VAL,G_VAL,B_VAL,NOSEL_ALPHA,STRLEN);
			SK_PrintfChengColor2(work->port[1],R_VAL,G_VAL,B_VAL,NOSEL_ALPHA,STRLEN);
#endif

			work->cursor_y=work->cursor_ty=SLOT1_Y;
			if(work->port_mode==3 && work->port_cursor){
				work->cursor_y=work->cursor_ty=SLOT2_Y;
			}

			// work->action_strcode=CODE_SHOWSLOT;
			work->sub_step++;
		}
		break;
	case 2:
		if(!work->busy_flag && work->anim_count==0){
			work->sub_step++;

			AnimCtrlSet0(work,ANIM_CTRL_PORT0+work->port_cursor,SEL_ALPHA,SEL_ALPHA);
		}
		break;
	case 3:
#if 0
		ShowSlotName(work);
		ShowDataSaving(work);
#endif

		if(MCManCheckingOrChecked()){
#if 0
			HideSlotName(work);
			HideDataSaving(work);
#endif

			return -2;
		}

		if(work->anim_count>0) break;


      //BP JG - skip port select UI by pressing OK or CANCEL.
      if ( work->start_mode == 0 )     // coming back from screen after a port select
      {
         // automatically press cancel
         SE_CANCEL();
         return -1;
      }
      else
      {
         // first time in, move forward without waiting for a button press
         if(work->port_mode)
         {
            // automatically press ok
            SE_OK();
            return 1;
         }
      }      

   /* BP JG - ^^^ don't need this anymore due to the code above ^^^
		if(work->key_press & PAD_CANCEL){
#if 0
			HideSlotName(work);
			HideDataSaving(work);
#endif

			SE_CANCEL();
			return -1;
		}
		else if(work->key_press & PAD_OK){
			if(work->port_mode){
#if 0
				HideSlotName(work);
				HideDataSaving(work);
#endif

				SE_OK();
				return 1;
			}
		}
      */

#if 0
		SK_PrintfChengColor2(work->port[work->port_cursor],R_VAL,G_VAL,B_VAL,NOSEL_ALPHA,STRLEN);

		switch(work->port_mode){
		case 1:
			SPR_SHOW(work->cursor);
			work->port_cursor=0;
			break;
		case 2:
			SPR_SHOW(work->cursor);
			work->port_cursor=1;
			break;
		case 3:
			SPR_SHOW(work->cursor);
			switch(work->key_press & (PAD_U|PAD_D)){
			case PAD_U:
				work->port_cursor--;
				if(work->port_cursor<0){
					work->port_cursor=N_PORT_ITEMS-1;
				}
				break;
			case PAD_D:
				work->port_cursor++;
				if(work->port_cursor>=N_PORT_ITEMS){
					work->port_cursor=0;
				}
				break;
			}
			break;
		}

		SK_PrintfChengColor2(work->port[work->port_cursor],R_VAL,G_VAL,B_VAL,SEL_ALPHA,STRLEN);
#else
		{
			int cur0,cur1;

			cur0=work->port_cursor;

			switch(work->port_mode){
			case 1:
				//SPR_SHOW(work->cursor);
				work->port_cursor=0;
				break;
			case 2:
				//SPR_SHOW(work->cursor);
				work->port_cursor=1;
				break;
			case 3:
				//SPR_SHOW(work->cursor);
				switch(work->key_press & (PAD_U|PAD_D)){
				case PAD_U:
					work->port_cursor--;
					if(work->port_cursor<0){
						work->port_cursor=N_PORT_ITEMS-1;
					}
					break;
				case PAD_D:
					work->port_cursor++;
					if(work->port_cursor>=N_PORT_ITEMS){
						work->port_cursor=0;
					}
					break;
				}
				break;
			}

			cur1=work->port_cursor;

			if(cur0!=cur1){
				static const float sloty[]={
					SLOT1_Y,
					SLOT2_Y,
				};
				work->anim_count=CURSOR_MOVE_COUNT;
				AnimCtrlSet(work,ANIM_CTRL_PORT0+cur0,NOSEL_ALPHA);
				AnimCtrlSet(work,ANIM_CTRL_PORT0+cur1,SEL_ALPHA);
				work->cursor_ty=sloty[cur1];

				SE_SEL();
			}
		}
#endif

		break;
	}

	return 0;
}

static int FileCheckSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		{
			int type=MCManGetCardType(work->port_cursor);

			switch(type){
			case MCMAN_CARDTYPE_NOCARD:
			case MCMAN_CARDTYPE_PS1:
			case MCMAN_CARDTYPE_POCKET_STATION:
			case MCMAN_CARDTYPE_ERRORCARD:
			case MCMAN_CARDTYPE_MULTITAP:
			  if( MCManCheckingOrChecked() ){
				  return -2;
			  }
				if(MCManFileFlagReset()){
					return -1;
				}
				break;

			case MCMAN_CARDTYPE_PS2_UNFORMATTED:
			  if( MCManCheckingOrChecked() ){
				  return -2;
			  }
				if(MCManFileFlagReset()){
					return 1;
				}
				break;

			case MCMAN_CARDTYPE_PS2_FORMATTED:
#if 0
				if(GetPhotoInfoStart(work->port_cursor)){
#else
				if(GetPhotoInfoRBGStart(work->port_cursor)){
#endif
					// SK_AllShow(work->checkcard,STRLEN);

#if 0
					work->anim_count=ANIM_CTRL_COUNT;
					AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x80);
#if 0
					AnimCtrlSet(work,ANIM_CTRL_PORT0,0x00);
					AnimCtrlSet(work,ANIM_CTRL_PORT1,0x00);
					AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
					AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x00);
					AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x00);
#else
					AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
					AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
					AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
					AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
					AnimCtrlSet0(work,ANIM_CTRL_DATA_SAVING,0x00,0x00);
#endif

#else
					work->anim_count=ANIM_CTRL_WAIT_COUNT;
#if 0
					AnimCtrlSet(work,ANIM_CTRL_PORT0,0x00);
					AnimCtrlSet(work,ANIM_CTRL_PORT1,0x00);
					AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
					AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x00);
					AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x00);
#else
					AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
					AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
					AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
					AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
					//AnimCtrlSet0(work,ANIM_CTRL_DATA_SAVING,0x00,0x00); //BP JG - don't fade out 'data save' - it used to then show the device, but seeing as we removed that, it just looks cleaner.
#endif

#endif

					work->sub_step++;
				}
				else{
					return -2;
				}
				break;
			}
		}
		break;
	case 1:
		if(work->anim_count==0){
			work->anim_count=ANIM_CTRL_COUNT;
			AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x80);
			work->sub_step++;
		}
	case 2:
		if(MCManCheckingOrChecked()){
			return -2;
		}
		switch(MCManGetResult()){
		case 1:
			return 1;
		case -1:
			return -1;
		}
		break;
	}

	return 0;
}

static int CheckError(Work *work)
{
	if(MCManGetCardType(work->port_cursor)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
		return 1;
	}
	else if(MCManNFiles()>0 ||
			MCManGetFreeSize(work->port_cursor)>=PHOTO_MC_MAXSIZE){

		return 1;
	}

	return 0;
}

static int ErrorSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag && work->anim_count==0){
			work->sub_step++;
		}
		break;
	case 1:
		if(MCManCheckingOrChecked()){
			return -2;
		}
		if(work->key_press & (PAD_CANCEL|PAD_OK)){
			SE_OK();
			return -1;
		}
		break;
	}

	return 0;
}

static void CallLocalProc(Work *work)
{
	switch(work->ans){
	case PORTSEL_ANS_MODEEND:
		GV_CallParentSignalFunc(work,SIGNAL_PORTSEL_OK,work->port_cursor);
		break;
	case PORTSEL_ANS_MODECANCEL:
		GV_CallParentSignalFunc(work,SIGNAL_PORTSEL_CANCEL,0);
		break;
	}
}

static void Step(Work *work)
{
	switch(work->step){
	case MCARD_START:
		HideTag(work);
		if(StartSubStep(work)){
			work->step=MCARD_CHECK;
			work->sub_step=0;
		}
		break;
	case MCARD_CHECK:
		HideTag(work);
		if(CheckSubStep(work)){
			work->step=MCARD_PORT_SEL;
			work->sub_step=0;
		}
		break;
	case MCARD_PORT_SEL:
		HideTag(work);
		switch(PortSelSubStep(work)){
		case 1:
			work->step=MCARD_FILE_CHECK;
			work->sub_step=0;
			SPR_HIDE(work->cursor);
			break;
		case -1:
			work->ans=PORTSEL_ANS_MODECANCEL;
			work->step=0x20;
			work->action_strcode=CODE_HIDEALL;

			work->anim_count=ANIM_CTRL_COUNT;

			AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x00);
			AnimCtrlSet(work,ANIM_CTRL_PORT0,0x00);
			AnimCtrlSet(work,ANIM_CTRL_PORT1,0x00);
			AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
			AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x00);
			AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x00);

			SPR_HIDE(work->cursor);
			break;
		case -2:
			work->step=MCARD_CHECK;
			work->sub_step=0;
			// work->action_strcode=CODE_HIDEALL;

			AnimCtrlSet0(work,ANIM_CTRL_CHECKCARD,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_DATA_SAVING,0x00,0x00);

			SPR_HIDE(work->cursor);
			break;
		}
		break;
	case MCARD_FILE_CHECK:
		HideTag(work);
		switch(FileCheckSubStep(work)){
		case 1:
			work->action_strcode=CODE_SHOWFILE;
			MakeTitle(work);

			if(CheckError(work)){
				work->ans=PORTSEL_ANS_MODEEND;
				work->step=0x10;

				// SK_AllHide(work->checkcard,STRLEN);

#if 0

				work->anim_count=ANIM_CTRL_COUNT;
#if 0
				AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x00);
				AnimCtrlSet(work,ANIM_CTRL_PORT0,0x00);
				AnimCtrlSet(work,ANIM_CTRL_PORT1,0x00);
				AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
				AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x00);
#else
				AnimCtrlSet0(work,ANIM_CTRL_CHECKCARD,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
#endif
				AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x80);

#else

				work->anim_count=ANIM_CTRL_WAIT_COUNT;
#if 0
				AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x00);
				AnimCtrlSet(work,ANIM_CTRL_PORT0,0x00);
				AnimCtrlSet(work,ANIM_CTRL_PORT1,0x00);
				AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
				AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x00);
#else
				AnimCtrlSet0(work,ANIM_CTRL_CHECKCARD,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
#endif

#endif

			}
			else{
				work->step=MCARD_ERROR;
				work->sub_step=0;

				work->anim_count=ANIM_CTRL_COUNT;
#if 0
				AnimCtrlSet(work,ANIM_CTRL_CHECKCARD,0x00);
				AnimCtrlSet(work,ANIM_CTRL_PORT0,0x00);
				AnimCtrlSet(work,ANIM_CTRL_PORT1,0x00);
				AnimCtrlSet(work,ANIM_CTRL_NO_MEMCARD,0x00);
#else
				AnimCtrlSet0(work,ANIM_CTRL_CHECKCARD,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
				AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
#endif
				AnimCtrlSet(work,ANIM_CTRL_NO_FREEAREA,0x80);
				AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x80);
			}
			break;
		case -1:
		case -2:

#if 0 //BP JG - this just avoids going back to portsel.. but i solved the problem a better way.
         work->ans=PORTSEL_ANS_MODEEND;
         work->step=0x10;

         //work->step=MCARD_FINISH;
         work->action_strcode=CODE_CLOSEALL;
         SE_WINCLOSE();
         break;
#endif
			work->step=MCARD_CHECK;
			work->sub_step=0;
			// work->action_strcode=CODE_HIDEALL;

			AnimCtrlSet0(work,ANIM_CTRL_CHECKCARD,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_PORT0,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_PORT1,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_MEMCARD,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_NO_FREEAREA,0x00,0x00);
			AnimCtrlSet0(work,ANIM_CTRL_DATA_SAVING,0x00,0x00);

			break;
		}
		break;
	case MCARD_ERROR:
		HideTag(work);
		switch(ErrorSubStep(work)){
		case 1:
		case -1:
			work->step=MCARD_PORT_SEL;
			work->sub_step=0;
			break;
		case -2:
			work->step=MCARD_CHECK;
			work->sub_step=0;
			// work->action_strcode=CODE_HIDEALL;
			break;
		}
		break;

	case 0x10:
		if(!work->busy_flag && work->anim_count==0){
			work->anim_count=ANIM_CTRL_COUNT;
			AnimCtrlSet(work,ANIM_CTRL_DATA_SAVING,0x80);
			work->step++;
		}
		break;
	case 0x11:
		if(!work->busy_flag && work->anim_count==0){
			CallLocalProc(work);
			GV_DestroyActor(work);
		}
		break;
		
	case 0x20:
		HideTag(work);
		if(!work->busy_flag && work->anim_count==0){
			CallLocalProc(work);
			GV_DestroyActor(work);
		}
		break;
	}

	AnimCtrl(work);
}

static void Act(Work *work)
{
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

	Key(work);
	Step(work);
}

static void _EndMenu(Work *work);

static void Die(Work *work)
{
	_EndMenu(work);
}


/* ------------------------------------------------------------------------ */


static void InitMenu(Work *work)
{
	SPR_OBJ *spr;
	int i,j;

	work->port_cursor=0;
	work->port_mode=0;

	spr=L2D_GetObject(work->l2d_handle,STR_NODE_FONT);
	if(spr==NULL) ASSERT(0);

	work->cursor=L2D_GetObject(work->l2d_handle,CODE_PHOTOSAVE_CURSOR);
	if(work->cursor==NULL) ASSERT(0);

#if 0 //BP_PS2 def PSX2
	work->base_u=spr->sprite.head.tex.u;
	work->base_v=spr->sprite.head.tex.v;
#else
	work->base_u=(int)(spr->sprite.head.tex.u*16.0f);
	work->base_v=(int)(spr->sprite.head.tex.v*16.0f);
#endif

	spr->sprite.pos.x=MC_CHECK_X;
	spr->sprite.pos.y=MC_CHECK_Y;
	spr->sprite.dw=FONT_WIDTH;
	spr->sprite.dh=FONT_HEIGHT;
	// SPR_SHOW(spr);

	for(i=0;i<STRLEN;i++){
		work->checkcard[i]=SPR_DuplicateTree(spr);
	}

	for(j=0;j<N_PORT_ITEMS;j++){
		spr->sprite.pos.x=slotxy[j][0];
		spr->sprite.pos.y=slotxy[j][1];

		for(i=0;i<STRLEN;i++){
			work->port[j][i]=SPR_DuplicateTree(spr);
		}
	}

	spr->sprite.pos.x=NOCARD_X;
	spr->sprite.pos.y=NOCARD_Y;

	for(i=0;i<STRLEN;i++){
		work->no_memcard[i]=SPR_DuplicateTree(spr);
	}

	spr->sprite.pos.x=NOFREEAREA_X;
	spr->sprite.pos.y=NOFREEAREA_Y;

	for(i=0;i<STRLEN;i++){
		work->no_freearea[i]=SPR_DuplicateTree(spr);
	}

	for(i=0;i<work->data_saving_len;i++){
		work->data_saving[i]->sprite.pos.x=DATA_SAVING_X;
		work->data_saving[i]->sprite.pos.y=DATA_SAVING_Y;
		work->data_saving[i]->sprite.dw=FONT_WIDTH;
		work->data_saving[i]->sprite.dh=FONT_HEIGHT;
	}

	SK_PrintfNormal(NULL,work->checkcard,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,STRLEN);
	//SK_PrintfNormal(MSG_CHECKCARD,work->checkcard,work->base_u,work->base_v,
	//				STR_WIDTH,STR_HEIGHT,STRLEN);
	SK_PrintfChengColor2(work->checkcard,R_VAL,G_VAL,B_VAL,0x00,STRLEN);
	SK_AllShow(work->checkcard,STRLEN);

	for(j=0;j<N_PORT_ITEMS;j++){
		SK_PrintfNormal(NULL,work->port[j],work->base_u,work->base_v,
						STR_WIDTH,STR_HEIGHT,STRLEN);
		//SK_PrintfNormal((char *)(slotstr[j]),work->port[j],work->base_u,work->base_v,
		//				STR_WIDTH,STR_HEIGHT,STRLEN);
		SK_PrintfChengColor2(work->port[j],R_VAL,G_VAL,B_VAL,0x00,STRLEN);
		SK_AllHide(work->port[j],STRLEN);
	}

	SK_PrintfNormal(NULL,work->no_memcard,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,STRLEN);
	SK_PrintfNormal(MSG_NO_MEMORYCARD,work->no_memcard,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,STRLEN);
	SK_PrintfChengColor2(work->no_memcard,R_VAL,G_VAL,B_VAL,0x00,STRLEN);
	SK_AllHide(work->no_memcard,STRLEN);

	SK_PrintfNormal(NULL,work->no_freearea,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,STRLEN);
	SK_PrintfNormal(MSG_NO_FREEAREA,work->no_freearea,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,STRLEN);
	SK_PrintfChengColor2(work->no_freearea,R_VAL,G_VAL,B_VAL,0x00,STRLEN);
	SK_AllHide(work->no_freearea,STRLEN);

	SK_PrintfNormal(NULL,work->data_saving,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,work->data_saving_len);
	SK_PrintfNormal(MSG_SAVING,work->data_saving,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,work->data_saving_len);
	SK_PrintfChengColor2(work->data_saving,R_VAL,G_VAL,B_VAL,0x00,work->data_saving_len);
	SK_AllShow(work->data_saving,work->data_saving_len);

	work->anim_count=0;
	for(i=0;i<ANIM_CTRL_MAX;i++) AnimCtrlSet0(work,i,0,0);

	MENU_ClearTextTexture(work->strman);

	{
		char *str;
		int resname=GLOBAL_SAVERES_ID;

		str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(resname,RESOURCE_MCNOTFOUND));

		MENU_CreateTextTexture(work->strman,
							   NOCARD_U,
							   NOCARD_V,
							   NOCARD_WIDTH,
							   NOCARD_HEIGHT2,
							   0,0,0,str);

		str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(resname,RESOURCE_ERROR));

		MENU_CreateTextTexture(work->strman,
							   NOFREEAREA_U,
							   NOFREEAREA_V,
							   NOFREEAREA_WIDTH,
							   NOFREEAREA_HEIGHT2,
							   0,0,0,str);
	}
}

static void _EndMenu(Work *work)
{
	int i,j;

	// printf("Check 1\n");

	for(i=0;i<STRLEN;i++){
		SPR_Destroy_2D_Object(work->checkcard[i]);
	}

	// printf("Check 2\n");

	for(j=0;j<N_PORT_ITEMS;j++){
		for(i=0;i<STRLEN;i++){
			SPR_Destroy_2D_Object(work->port[j][i]);
		}
	}

	// printf("Check 3\n");

	for(i=0;i<STRLEN;i++){
		SPR_Destroy_2D_Object(work->no_memcard[i]);
	}

	for(i=0;i<STRLEN;i++){
		SPR_Destroy_2D_Object(work->no_freearea[i]);
	}

	MENU_ClearTextTexture(work->strman);
}

static void InitIconEnv(Work *work)
{
	static const int bgclr[]={ 0x00,0x00,0x00,0x80, };
	static const FVECTOR dir={ 0.0f,0.0f,1.0f,0.0f, };
	static const FVECTOR rgb={ 128.0f/255.0f,128.0f/255.0f,128.0f/255.0f,0.0f, };

	int stage_num;
	char title[0x80];

	stage_num=GM_StageNum;

	strcpy(title,MGS2_PHOTO_TITLE);
	strcat(title,mc_title_stage_name[stage_num]);

	SetIconSysTitle(title,MGS2_PHOTO_TITLE_LF_LOCATE);

	if(GM_Configuration & GM_CONFIG_STORY_TANKER){
		/* 指定順序 : lu ru ld rd */
		SetIconSysBgColor(bgclr,bgclr,bgclr,bgclr);

		SetIconSysLight(&dir,&dir,&dir,&rgb,&rgb,&rgb);
		SetIconSysAmbient(&rgb);
	}
	else{
		/* 指定順序 : lu ru ld rd */
		SetIconSysBgColor(bgclr,bgclr,bgclr,bgclr);

		SetIconSysLight(&dir,&dir,&dir,&rgb,&rgb,&rgb);
		SetIconSysAmbient(&rgb);
	}
}


/* ------------------------------------------------------------------------ */


/* 初期化部メイン */
void *NewPortselForPhotosave(int l2d_handle,SPR_OBJ **sprwork,int sprwork_size,void *strman, int currentMode)
{
    Work *work ;

#ifdef DEBUG_MODE
	printf("Mem Point = %ld\n" , codecMemGetSize() );
#endif

	if((work=(Work *)codecMalloc(sizeof(Work)))==NULL){
		return NULL;
	}

	/* アクター起動 */
	GV_ZeroMemory(work,sizeof(Work));
	GV_SetActorFreeFunc(work,codecFree);
	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(ACT_MODE,work,0x1FF); /* ACT_MODE <--- user/mode/codec/codecmem.h */

	GV_SetActor(&(work->actor),Act,Die);
	GV_ActorEX(&(work->actor));

	work->l2d_handle=l2d_handle;

	work->step=0;
	work->sub_step=0;
	work->ans=0;

	work->data_saving=sprwork;
	work->data_saving_len=sprwork_size;

	work->strman=strman;

   work->start_mode = currentMode;

	// GV_SetActor(&(work->actor),Act,Die) ;
	// GV_ActorEX(&(work->actor));

	// InitMenu(work);

    return (void *)work ;
}
