//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	option.c
		オプション画面

	2001/06/15 K.Kano
	2002/06/08 Y.YANO 多言語対応
	$Id: option.c,v 1.2 2002/12/05 18:41:59 takaki Exp $
*/


#include "titlescr.h"

#include "bgscr.h"

#include "subtitle.h"


#include "../../mode/codec/codec_signal.h"
#include "../../mode/codec/codecmem.h"
#include "../../mode/codec/cdc_load.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "font.h"
#include "BP_LocalizedTextByEnum.h"

extern void MENU_SetTextSpace( void *work_ptr, int space );

#ifdef PSX2
int sd_set_cli(int);
#endif

// BP JG - new option flags.
//int g_RealTimeCutscenes = 0;
//int g_FullFrameHUD = 0;

#define N_MENU_ITEMS		   12
#define N_VAL_ITEMS			9

#define CODE_L2D_SCRN		GV_StrCode("scrn")
#define CODE_L2D_COLOR		GV_StrCode("color")

#define MV_ANIM_COUNT		DIRECT_TICK(10)
#if 0
#define SW_ANIM_COUNT		DIRECT_TICK(10)
#else
#define SW_ANIM_COUNT		1
#endif


#define ANIM_WORK_SIZE		7


#define DISP_CHANL			TITLE_CHANL
#define MENU_PRIORITY		TITLE_PRI_NORMAL

#define WAIT_COUNT			DIRECT_TICK(2)

#define WAIT_COUNT_SCR		DIRECT_TICK(16)
#define WAIT_COUNT_CLR		DIRECT_TICK(16)


typedef struct {
	GV_ACT_EX actor;
	int name;
	int parent_name;

	int busy_flag;
	int l2d_strcode;
	int l2d_handle;
	int action_strcode;

	int sub_busy_flag;
	int sub_l2d_handle;
	int sub_action_strcode;

	int proc;
	int ret_signal_flag;
	int step;
	int sub_step;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;
	int key_repeat_time;

	int wait_count;

	int ans;
	int menu_cursor;
   int menu_cursor_screen;
	int menu_value_cursor[N_VAL_ITEMS];
	int old_x,old_y;
	int x,y;
	int explain_disp_flag;
	int explain_columns;

	int fade_count;
	int fade_alpha;

	struct {
		int part_code;
		int key0_code;
		int key1_code;
		float p;
		int count;
	} key_anim[ANIM_WORK_SIZE];

	SPR_OBJ *cursor;

	SPR_OBJ *numxy[2][3];

	void *strman;

	int bgname;

	int start_flag;


	// 以下、多言語対応用変数
	int option_explain_resource[LANG_MAX_NUM];
	int radar_explain_resource[LANG_MAX_NUM];
	int quick_explain_resource[LANG_MAX_NUM];
	int lang_num;
	int lang_flag[LANG_MAX_NUM+1];
	int lang_ref[LANG_MAX_NUM+1];
	//int lang_flag_cnt;
	int lang_default;
	int lang_val;

	int first_val;

	int stat_5_1ch;
	int x_pos_5_1ch;
	int y_pos_5_1ch;
	int line_5_1ch;
	int ch5_1_explain_height;
	int ch5_1_explain_h;
} Work;

static const int config_flags[]={
	GM_CONFIG_VIBRATION_OFF,
	GM_CONFIG_RADAR_OFF,
	GM_CONFIG_BLOOD_OFF,
	GM_CONFIG_CUTSCENES_LETTERBOXED,
	GM_CONFIG_SOUND_5_1CHANL,
	GM_CONFIG_CAPTION_OFF,

	GM_CONFIG_SHUKAN_REVERSE,
	GM_CONFIG_OLD_TYPE_MENU,
	GM_CONFIG_MENU_QCHANGE_EX,
};

enum {
	ITEM_SEL_INDEX=0,
	ITEM_UNSEL_INDEX,
};


static const int vibstate_name[]={ SW_vibON,SW_vibOFF, };

static const int radstate_name[]=
#if 0
	{ SW_radON,SW_radOFF, };
#else
	{ SW_radOFF1,SW_radTYPE1,SW_radTYPE2,SW_radOFF2, };
#endif


static const int bldstate_name[]={ SW_bldON,SW_bldOFF, };

static const int sndstate_name[]={ SW_sndSTEREO,SW_sndMONO, };

static const int p5_1chstate_name[]={ SW_5_1chOFF,SW_5_1chON, };

#define CAPLANG_MAX	 	(LANG_MAX_NUM + 1)
static int capstate_name[CAPLANG_MAX];

static const int ownstate_name[]={ SW_ownNORM,SW_ownREV, };

static const int itemstate_name[]={ SW_itemGROUP,SW_itemLINEAR, };

static const int quickstate_name[]={ SW_quickUNEQUIP,SW_quickPREV, };


static struct {

   int cur_pos;
	int menu_item;
	int menu_colon;
	int menu_sw;
	int sel[2];
	const int *sw;
} menu_list[] = {
	/* 振動切替え  */
	{ POS_curVIB,
	  OBJ_optVIB,CODE_MENUVIB_COL,OBJ_swVIB,
	  { KEY_selVIB,   KEY_unVIB,  },
	  vibstate_name,
	},

	/* レーダー    */
	{ POS_curRAD,
	  OBJ_optRAD,CODE_MENURAD_COL,OBJ_swRAD,
	  { KEY_selRAD,   KEY_unRAD, },
	  radstate_name,
	},

	/* 出血        */
	{ POS_curBLD,
	  OBJ_optBLD,CODE_MENUBLD_COL,OBJ_swBLD,
	  { KEY_selBLD,   KEY_unBLD, },
	  bldstate_name,
	},

	/* サウンド切替え */
	{ POS_curSND,
	  OBJ_optSND,CODE_MENUSND_COL,OBJ_swSND,
	  { KEY_selSND,   KEY_unSND, },
	  sndstate_name,
	},

	/* 部分 5.1ch 対応 */
	{ POS_cur5_1CH,
	  OBJ_opt5_1CH,CODE_MENU5_1CH_COL,OBJ_sw5_1CH,
	  { KEY_sel5_1CH, KEY_un5_1CH, },
	  p5_1chstate_name,
	},

	/* 字幕 ON/OFF */
	{ POS_curCAP,
	  OBJ_optCAP,CODE_MENUCAP_COL,OBJ_swCAP,
	  { KEY_selCAP,   KEY_unCAP, },
	  capstate_name,
	},

	/* 主観上下挙動 */
	{ POS_curOWN,
	  OBJ_optOWN,CODE_MENUOWN_COL,OBJ_swOWN,
	  { KEY_selOWN,   KEY_unOWN, },
	  ownstate_name,
	},

	/* アイテムウィンドウ */
	{ POS_curITEM,
	  OBJ_optITEM,CODE_MENUITEM_COL,OBJ_swITEM,
	  { KEY_selITEM,   KEY_unITEM, },
	  itemstate_name,
	},

	/* クィックチェンジ */
	{ POS_curQUICK,
	  OBJ_optQUICK,CODE_MENUQUICK_COL,OBJ_swQUICK,
	  { KEY_selQUICK,  KEY_unQUICK, },
	  quickstate_name,
	},

	{ POS_curSCRN,
	  OBJ_optSCRN,  -1, -1,              /* スクリーン位置調整 */
	  { KEY_selSCRN, KEY_unSCRN, },
	  NULL,
	},

	{ POS_curCOL,
	  OBJ_optCOL,  -1,  -1,              /* カラー調整 */
	  { KEY_selCOL, KEY_unCOL, },
	  NULL,
	},

	{ POS_curEXIT,
	  OBJ_optEXIT,  -1, -1,              /* 脱出 */
	  { KEY_selEXIT, KEY_unEXIT, },
	  NULL,
	},
};


enum {
	ITEM_VIB=0,
	ITEM_RAD,
	ITEM_BLD,
	ITEM_SND,
	ITEM_5_1CH,
	ITEM_CAP,

	ITEM_OWN,
	ITEM_ITEM,
	ITEM_QUICK,

	ITEM_SCRN,
	ITEM_COL,

	ITEM_EXIT,

	ITEM_MAX,
};

enum {
	RES_COLOR_EXPLAIN=ITEM_MAX,
};

// AS Steve: make this much easier to move around

typedef struct
{
   int item;
   int offset;
   int cur_pos;
}
SMenuOptionReposition;

// to rearrange, just change the first column - the ITEM_s
SMenuOptionReposition skOptionReposition[] =
{
   { ITEM_RAD,   0, POS_curVIB   },
   { ITEM_BLD,   0, POS_curRAD   },
   { ITEM_CAP,   0, POS_curBLD   },
   { ITEM_OWN,   0, POS_curSND   },
   { ITEM_QUICK, 0, POS_cur5_1CH },
   { -1,         0, POS_curCAP   },
   { -1,         0, POS_curOWN   },
   { -1,         0, POS_curITEM  },
   { -1,         0, POS_curQUICK },
   { -1,         0, POS_curSCRN  },
   { -1,         0, POS_curCOL   },
   { ITEM_EXIT, -4, POS_curEXIT  }
};
#define REPOSITION_COUNT (sizeof(skOptionReposition)/sizeof(skOptionReposition[0]))


#define NUM_FONT_WIDTH		18


#define REPEAT_FIRST		20
#define REPEAT_NEXT			1
#define REPEAT_NEXT2		3

#define SCRN_ADJUST_X_LIMIT		20
#define SCRN_ADJUST_Y_LIMIT		20


#define FONT_WIDTH				SUBTITLE_FONT_WIDTH
#define FONT_HEIGHT				SUBTITLE_FONT_HEIGHT
#define FONT_DISP_WIDTH			SUBTITLE_FONT_DISP_WIDTH
#define FONT_DISP_HEIGHT		SUBTITLE_FONT_DISP_HEIGHT
#define LINE_SPACE				RUBI_SIZE_H
#define LINE_HEIGHT				(FONT_HEIGHT+LINE_SPACE)

#define OPTION_EXPLAIN_U		0
#define OPTION_EXPLAIN_V		0
#define OPTION_EXPLAIN_WIDTH	(30*FONT_WIDTH)
#if 0
#define OPTION_EXPLAIN_HEIGHT	(3*FONT_HEIGHT)
#else
#define OPTION_EXPLAIN_HEIGHT	(3*LINE_HEIGHT+DOWN_MARGINE)
#endif

#define OPTION_EXPLAIN_X		(SUBTITLE_X-10)
#define OPTION_EXPLAIN_Y		SUBTITLE_Y
#if 0
#define OPTION_EXPLAIN_W		(30*FONT_DISP_WIDTH)
#define OPTION_EXPLAIN_H		(3*FONT_DISP_HEIGHT)
#else
#define OPTION_EXPLAIN_W		(OPTION_EXPLAIN_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define OPTION_EXPLAIN_H		(OPTION_EXPLAIN_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define COLOR_EXPLAIN_U			0
#define COLOR_EXPLAIN_V			0
#define COLOR_EXPLAIN_WIDTH		(30*FONT_WIDTH)
#if 0
#define COLOR_EXPLAIN_HEIGHT	(10*FONT_HEIGHT)
#else
#define COLOR_EXPLAIN_HEIGHT	(10*LINE_HEIGHT+DOWN_MARGINE)
#endif

#define COLOR_EXPLAIN_X			56
#define COLOR_EXPLAIN_Y			200
#if 0
#define COLOR_EXPLAIN_W			(30*FONT_DISP_WIDTH)
#define COLOR_EXPLAIN_H			(10*FONT_DISP_HEIGHT)
#else
#define COLOR_EXPLAIN_W			(COLOR_EXPLAIN_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define COLOR_EXPLAIN_H			(COLOR_EXPLAIN_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define CH5_1_EXPLAIN_U			0
#define CH5_1_EXPLAIN_V			0
#define CH5_1_EXPLAIN_WIDTH		(17*FONT_WIDTH+20)		
#define CH5_1_EXPLAIN_HEIGHT	(8 *(FONT_HEIGHT+FONT_HEIGHT/2)+DOWN_MARGINE)		
#define CH5_1_EXPLAIN_W			(CH5_1_EXPLAIN_WIDTH *FONT_DISP_WIDTH /FONT_WIDTH)
#define CH5_1_EXPLAIN_H			((CH5_1_EXPLAIN_HEIGHT)*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define TEXTCOLOR				SUBTITLE_TEXTCOLOR

//#ifdef NTSC

#define OPTION_EXPLAIN_RESOURCE_NOEU		0x00a7d31a		/* オプションの説明 */
#define RADAR_EXPLAIN_RESOURCE_NOEU		0x0001ee1c		/* レーダーの説明 */
#define QUICK_EXPLAIN_RESOURCE_NOEU		0x00191c61		/* クイックチェンジの説明 */

//#endif

//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL

#define OPTION_EXPLAIN_RESOURCE_EU		(work->option_explain_resource)		/* オプションの説明 */
#define RADAR_EXPLAIN_RESOURCE_EU		(work->radar_explain_resource)		/* レーダーの説明 */
#define QUICK_EXPLAIN_RESOURCE_EU		(work->quick_explain_resource)		/* クイックチェンジの説明 */

#define E_OPTION_EXPLAIN_RESOURCE	0x00a7d31a		/* オプションの説明 */
#define E_RADAR_EXPLAIN_RESOURCE	0x0001ee1c		/* レーダーの説明 */
#define E_QUICK_EXPLAIN_RESOURCE	0x00191c61		/* クイックチェンジの説明 */

#define F_OPTION_EXPLAIN_RESOURCE	0x009f2c05		/* オプションの説明フランス語 */
#define F_RADAR_EXPLAIN_RESOURCE	0x0007980b		/* レーダーの説明フランス語 */
#define F_QUICK_EXPLAIN_RESOURCE	0x0064511f		/* クイックチェンジの説明フランス語 */

#define G_OPTION_EXPLAIN_RESOURCE	0x00b5ab53		/* オプションの説明ドイツ語 */
#define G_RADAR_EXPLAIN_RESOURCE	0x00b7056e		/* レーダーの説明ドイツ語 */
#define G_QUICK_EXPLAIN_RESOURCE	0x00fc1c9c		/* クイックチェンジの説明ドイツ語 */

#define I_OPTION_EXPLAIN_RESOURCE	0x00ddc584		/* オプションの説明イタリア語 */
#define I_RADAR_EXPLAIN_RESOURCE	0x0046318a		/* レーダーの説明イタリア語 */
#define I_QUICK_EXPLAIN_RESOURCE	0x00a2ea9e		/* クイックチェンジの説明イタリア語 */

#define S_OPTION_EXPLAIN_RESOURCE	0x00941040		/* オプションの説明スペイン語 */
#define S_RADAR_EXPLAIN_RESOURCE	0x00fc7c45		/* レーダーの説明スペイン語 */
#define S_QUICK_EXPLAIN_RESOURCE	0x0059355a		/* クイックチェンジの説明スペイン語 */

static const int option_resource_name[]={
	E_OPTION_EXPLAIN_RESOURCE,
	F_OPTION_EXPLAIN_RESOURCE,
	G_OPTION_EXPLAIN_RESOURCE,
	I_OPTION_EXPLAIN_RESOURCE,
	S_OPTION_EXPLAIN_RESOURCE,
};

static const int radar_resource_name[]={
	E_RADAR_EXPLAIN_RESOURCE,
	F_RADAR_EXPLAIN_RESOURCE,
	G_RADAR_EXPLAIN_RESOURCE,
	I_RADAR_EXPLAIN_RESOURCE,
	S_RADAR_EXPLAIN_RESOURCE,
};

static const int quick_resource_name[]={
	E_QUICK_EXPLAIN_RESOURCE,
	F_QUICK_EXPLAIN_RESOURCE,
	G_QUICK_EXPLAIN_RESOURCE,
	I_QUICK_EXPLAIN_RESOURCE,
	S_QUICK_EXPLAIN_RESOURCE,
};

//#endif

#define OPTION_EXPLAIN_RESOURCE ( BP_Area_EU() ? OPTION_EXPLAIN_RESOURCE_EU : OPTION_EXPLAIN_RESOURCE_NOEU )
#define RADAR_EXPLAIN_RESOURCE ( BP_Area_EU() ? RADAR_EXPLAIN_RESOURCE_EU : RADAR_EXPLAIN_RESOURCE_NOEU )
#define QUICK_EXPLAIN_RESOURCE ( BP_Area_EU() ? QUICK_EXPLAIN_RESOURCE_EU : QUICK_EXPLAIN_RESOURCE_NOEU )

void ReArrangeOptions(Work *work);


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[N_PROC_ARGS];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

#if 0
		argv[0]=PROC_COM_OPTION;
		argv[1]=0;
		argv[1]|=(work->menu_value_cursor[0] ? 0x00 : 0x01);
		argv[1]|=(work->menu_value_cursor[1] ? 0x00 : 0x02);
		argv[1]|=(work->menu_value_cursor[2] ? 0x00 : 0x04);
		argv[1]|=(work->menu_value_cursor[3] ? 0x00 : 0x08);
		argv[1]|=(work->menu_value_cursor[4] ? 0x00 : 0x10);
		argv[2]=work->x;
		argv[3]=work->y;
#endif

		GCL_ExecProc(work->proc,&arg);
	}
	if(work->ret_signal_flag &&
	   work->ans==TITLE_MSG_MODEEND){

		GV_CallParentSignalFunc(work,0,0);
	}
}


static void MainLayoutSignalFunc(Work *work,int sign,int value)
{
	ComNodeFrameAction(sign);

#ifdef DEBUG_MODE
	printf("Layout Signal = %d ( 0x%08x )\n",sign,sign);
#endif

}


#if 0

static inline void ShowCursor(Work *work)
{
	SPR_SHOW(work->cursor);
}

static inline void HideCursor(Work *work)
{
	SPR_HIDE(work->cursor);
}

#endif


static void Key(Work *work)
{
	int status=GV_PadDataDirect[0].status & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);
	int press=GV_PadDataDirect[0].press & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);

	work->key_press=press;

	if(status && status==work->key_status){
		if(work->key_count>REPEAT_FIRST){
			work->key_count-=work->key_repeat_time;
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
   ReArrangeOptions(work);
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


//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)

static void BgscrShowGenziOption(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_SHOW_OPTION;

	GV_SendMessage(&msg);
}

static void BgscrHideGenziOption(Work *work)
{
	GV_MSG msg;
	int message[2];

	if(work->bgname==0 || work->bgname==1) return;

	msg.address=work->bgname;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	message[0]=BG_MSG_GENZI;
	message[1]=GENZI_MSG_HIDE_OPTION;

	GV_SendMessage(&msg);
}

//#endif


static void GetConfigValue(Work *work)
{
	int i;
	for(i=ITEM_VIB;i<=ITEM_QUICK;i++){
		if(i==ITEM_RAD) continue;
		if(i==ITEM_CAP) continue;
		work->menu_value_cursor[i]=((GM_Configuration & config_flags[i])!=0);
	}

	/* レーダーだけは例外 */
	switch(GM_Configuration & (GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE)){
	case 0:
		/* TYPE 1 */
		work->menu_value_cursor[ITEM_RAD]=1;
		break;
	case GM_CONFIG_RADAR_OFF_INTRUDE:
		/* TYPE 2 */
		work->menu_value_cursor[ITEM_RAD]=2;
		break;
	case GM_CONFIG_RADAR_OFF:
	case GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE:
		/* OFF */
		work->menu_value_cursor[ITEM_RAD]=0;
		break;
	}

	if (GM_Configuration & config_flags[ITEM_CAP])
   {
		work->menu_value_cursor[ITEM_CAP]=0;
	}
   else
   {
		switch(GM_Language){
		case GM_LANG_JAPANESE:
			work->menu_value_cursor[ITEM_CAP] = work->lang_flag[GM_LANG_JAPANESE];
			break;
		case GM_LANG_ITALY:
			work->menu_value_cursor[ITEM_CAP] = work->lang_flag[GM_LANG_ITALY];
			break;
		case GM_LANG_SPANISH:
			work->menu_value_cursor[ITEM_CAP] = work->lang_flag[GM_LANG_SPANISH];
			break;
		case GM_LANG_FRENCH:
			work->menu_value_cursor[ITEM_CAP] = work->lang_flag[GM_LANG_FRENCH];
			break;
		case GM_LANG_GERMANY:
			work->menu_value_cursor[ITEM_CAP] = work->lang_flag[GM_LANG_GERMANY];
			break;
		case GM_LANG_ENGLISH:
		default:
			work->menu_value_cursor[ITEM_CAP] = work->lang_flag[GM_LANG_ENGLISH];
			break;
		}
	}

#ifdef DEBUG_MODE
	printf("GM_Configuration = 0x%08x\n",GM_Configuration);
#endif

	work->x=work->old_x=GM_ScrAdjX;
	work->y=work->old_y=GM_ScrAdjY;
}

static void SetConfigValue(Work *work)
{
	int vand=0,vor=0;
	int i;
	int gm_configuration=GM_Configuration;

	for(i=ITEM_VIB;i<=ITEM_QUICK;i++){
		if(i==ITEM_RAD || i==ITEM_5_1CH) continue;
		if(i==ITEM_CAP) continue;
		if(work->menu_value_cursor[i]) vor|=config_flags[i];
		else vand|=config_flags[i];
	}

	/* 5.1chは、ステレオの場合のみ */
	if(!work->menu_value_cursor[ITEM_SND]){
		if(work->menu_value_cursor[ITEM_5_1CH]) vor|=config_flags[ITEM_5_1CH];
		else vand|=config_flags[ITEM_5_1CH];
	}
	else{
		vand|=config_flags[ITEM_5_1CH];
	}

	/* レーダーだけは例外 */
	switch(work->menu_value_cursor[ITEM_RAD]){
	case 1:
		/* TYPE 1 */
		vand|=GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE;
		break;
	case 2:
		/* TYPE 2 */
		vand|=GM_CONFIG_RADAR_OFF;
		vor|=GM_CONFIG_RADAR_OFF_INTRUDE;
		break;
	case 0:
	case 3:
		/* OFF */
		vor|=GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE;
		break;
	}

	if(work->menu_value_cursor[ITEM_CAP]==0)
   {
		vor|=config_flags[ITEM_CAP];
		GM_Language = work->lang_default;
		printf("lang[%d]\n", GM_Language );
	}
	else
   {
		vand|=config_flags[ITEM_CAP];
		GM_Language= work->lang_ref[ work->menu_value_cursor[ITEM_CAP] ];
		printf("lang futu[%d]\n", GM_Language );
	}

	vand=~vand;
	GM_Configuration&=vand;
	GM_Configuration|=vor;

#ifdef DEBUG_MODE
	printf("GM_Configuration = 0x%08x\n",GM_Configuration);
#endif

	GM_ScrAdjX=work->x;
	GM_ScrAdjY=work->y;

	if(gm_configuration!=GM_Configuration){
		printf("option changed\n");
		GM_TitleMenuStatus|=TITLE_MENU_OPTION_CHANGED;
	}

	if( work->first_val != work->menu_value_cursor[ITEM_CAP] ){
		printf("option changed\n");
		GM_TitleMenuStatus|=TITLE_MENU_OPTION_CHANGED;
	}		

}


static void SubStepCursorInit(Work *work)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,CUR_curs);
	if(part==NULL) return;

	L2D_MorfObject(part,
				   menu_list[work->menu_cursor].cur_pos,
				   menu_list[work->menu_cursor].cur_pos,1.0f);

	part=L2D_GetParts(work->l2d_handle,menu_list[work->menu_cursor].menu_item);
	if(part==NULL) return;

	L2D_MorfObject(part,
				   menu_list[work->menu_cursor].sel[ITEM_SEL_INDEX],
				   menu_list[work->menu_cursor].sel[ITEM_SEL_INDEX],1.0f);
}

static void SubStepValueInit(Work *work)
{
	int i;

	for(i=ITEM_VIB;i<=ITEM_QUICK;i++){
		void *part;
		int val;

		val=work->menu_value_cursor[i];

		part=L2D_GetParts(work->l2d_handle,menu_list[i].menu_sw);
		if(part==NULL) continue;

		L2D_MorfObject(part,menu_list[i].sw[val],menu_list[i].sw[val],1.0f);
	}
}

static void CopyKeyAlpha(Work *work)
{
	static const int strcode[][2]={
      { OBJ_optVIB,OBJ_swVIB, },
		{ OBJ_optRAD,OBJ_swRAD, },
		{ OBJ_optBLD,OBJ_swBLD, },
      { OBJ_optSND,OBJ_swSND, },
		//{ OBJ_opt5_1CH,OBJ_sw5_1CH, },
		{ OBJ_optCAP,OBJ_swCAP, },
		{ OBJ_optOWN,OBJ_swOWN, },
		{ OBJ_optITEM,OBJ_swITEM, },
		{ OBJ_optQUICK,OBJ_swQUICK, },

      { OBJ_optVIB,CODE_MENUVIB_COL, },
		{ OBJ_optRAD,CODE_MENURAD_COL, },
		{ OBJ_optBLD,CODE_MENUBLD_COL, },
      { OBJ_optSND,CODE_MENUSND_COL, },
		//{ OBJ_opt5_1CH,CODE_MENU5_1CH_COL, },
		{ OBJ_optCAP,CODE_MENUCAP_COL, },
		{ OBJ_optOWN,CODE_MENUOWN_COL, },
		{ OBJ_optITEM,CODE_MENUITEM_COL, },
		{ OBJ_optQUICK,CODE_MENUQUICK_COL, },
	};
	int i;

	if(work->l2d_handle<0) return;

	for(i=0;i<sizeof(strcode)/sizeof(strcode[0]);i++){
		SPR_OBJ *spr0,*spr1;

		spr0=L2D_GetObject(work->l2d_handle,strcode[i][0]);
		spr1=L2D_GetObject(work->l2d_handle,strcode[i][1]);

		if(spr0==NULL || spr1==NULL) continue;

		spr1->sprite.col.a=spr0->sprite.col.a;
	}

#if 0
	/* node画面 */
	{
		SPR_OBJ *spr;

		spr=L2D_GetObject(work->l2d_handle,OBJ_optSCRN);
		SPR_HIDE(spr);
		spr=L2D_GetObject(work->l2d_handle,OBJ_optCOL);
		SPR_HIDE(spr);
	}
#endif

}



static void CreateOptionExplain(Work *work)
{
	if(!work->explain_disp_flag){
		char *str;
		int i;

		switch(work->menu_cursor){
		case ITEM_RAD:
			{
				int index=work->menu_value_cursor[ITEM_RAD]-1;

				if(index<0) index=3-1;

				str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(work->radar_explain_resource[work->lang_ref[work->lang_val] - GM_LANG_ENGLISH],index));
			}
			break;
		case ITEM_QUICK:
			{
				int index=work->menu_value_cursor[ITEM_QUICK];
				str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(work->quick_explain_resource[work->lang_ref[ work->lang_val ] - GM_LANG_ENGLISH],index));
			}
			break;

      case ITEM_SND:
         {
            str=BP_GetStringForEnum(kBP_GLS_Options_ExplainRealTimeCutScenes);
         }
         break;

		default:
			str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(work->option_explain_resource[work->lang_ref[ work->lang_val ] - GM_LANG_ENGLISH ],work->menu_cursor));
			break;
		}

		work->explain_columns=1;

#if BP_USE_NEW_FONT_SYSTEM()
      {
         unsigned char* temp = (unsigned char *)str;
         
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

		MENU_ClearTextTexture(work->strman);

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
	}

	work->explain_disp_flag++;
}

static void PrintOptionExplain(Work *work)
{
	if (work->explain_disp_flag > 0)
   {
      int x = OPTION_EXPLAIN_X;
		int y = OPTION_EXPLAIN_Y;
      int w = OPTION_EXPLAIN_W; // * 3 / 4; // correct for 16:9
		int h = OPTION_EXPLAIN_H;

		work->explain_disp_flag = 1;

		if (work->explain_columns > 1)
      {
			y -= FONT_DISP_HEIGHT / 2;
			h = h * 5 / 6;
         // w = w * 5 / 6; // keep font approximately the same aspect ratio
		}

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
}

static void CreateColorExplain(Work *work)
{
	char *str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(work->option_explain_resource[work->lang_ref[ work->lang_val ] - GM_LANG_ENGLISH],RES_COLOR_EXPLAIN));

	MENU_ClearTextTexture(work->strman);

	MENU_CreateTextTexture(work->strman,
						   COLOR_EXPLAIN_U,
						   COLOR_EXPLAIN_V,
						   COLOR_EXPLAIN_WIDTH,
						   COLOR_EXPLAIN_HEIGHT,
						   0,0,0,str);
}

static void PrintColorExplain(Work *work)
{
	MENU_PutTextScreen(work->strman,
					   COLOR_EXPLAIN_X,
					   COLOR_EXPLAIN_Y,
					   COLOR_EXPLAIN_X+COLOR_EXPLAIN_W,
					   COLOR_EXPLAIN_Y+COLOR_EXPLAIN_H,
					   COLOR_EXPLAIN_U,
					   COLOR_EXPLAIN_V,
					   COLOR_EXPLAIN_U+COLOR_EXPLAIN_WIDTH,
					   COLOR_EXPLAIN_V+COLOR_EXPLAIN_HEIGHT,
					   TEXTCOLOR);
}
static void Create5_1chExplain(Work *work)
{
	char *str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(work->option_explain_resource[work->lang_ref[ work->lang_val ] - GM_LANG_ENGLISH],RES_COLOR_EXPLAIN+1));/* COLOR ADJUSTMENTの説明の次 */

	MENU_ClearTextTexture(work->strman);

	/* 注！言語によって行数が違うので調整 */
	if( GM_Language == GM_LANG_JAPANESE || GM_Language == GM_LANG_SPANISH ){
		/* 日、西 */
		work->line_5_1ch = 6;
	} else if( GM_Language == GM_LANG_ENGLISH || GM_Language == GM_LANG_ITALY ){
		/* 英、伊 */
		work->line_5_1ch = 7;		
	} else{
		/* 仏、独 */
		work->line_5_1ch = 8;
	}
	work->ch5_1_explain_height = work->line_5_1ch * (FONT_HEIGHT*3/2) + DOWN_MARGINE;
	work->ch5_1_explain_h = work->ch5_1_explain_height * FONT_DISP_HEIGHT / FONT_HEIGHT; 

	MENU_CreateTextTexture(work->strman,
						   CH5_1_EXPLAIN_U,
						   CH5_1_EXPLAIN_V,
						   CH5_1_EXPLAIN_WIDTH,
						   work->ch5_1_explain_height,
						   0,8,0,str);
}

static void Print5_1chExplain(Work *work)
{
	MENU_PutTextScreen(work->strman,
					   work->x_pos_5_1ch-CH5_1_EXPLAIN_W/2,
					   work->y_pos_5_1ch-work->ch5_1_explain_h/2,
					   work->x_pos_5_1ch+CH5_1_EXPLAIN_W/2,
					   work->y_pos_5_1ch+work->ch5_1_explain_h/2,
					   0,
					   0,
					   COLOR_EXPLAIN_U+CH5_1_EXPLAIN_WIDTH,
					   COLOR_EXPLAIN_V+work->ch5_1_explain_height,
					   TEXTCOLOR);
}

static int SubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			if(work->start_flag) work->action_strcode=CODE_BACKOPT;
			else work->action_strcode=CODE_SHOWOPT;

			work->sub_step++;
			work->explain_disp_flag=0;

			work->start_flag=1;

			// HideCursor(work);


//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrShowGenziOption(work);
//#endif

		}
		break;
	case 1:
		SubStepValueInit(work);
		work->sub_step++;
	case 2:
		if(!work->busy_flag){
			work->sub_step++;

			// ShowCursor(work);
			SubStepCursorInit(work);
		}
		break;
	case 3:
		PrintOptionExplain(work);

		if(work->key_anim[0].count>0) break;

		CreateOptionExplain(work);

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_HIDEOPT;
			work->explain_disp_flag=0;

			SE_CANCEL();


//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
			// BgscrHideGenziOption(work);
//#endif

			return -1;
		}
		else if(work->key_press & (PAD_OK|PAD_L|PAD_R)){
			switch(work->menu_cursor){
			case ITEM_RAD:
				{
					int val0,val1;

					val0=work->menu_value_cursor[work->menu_cursor];
#if 0
					work->menu_value_cursor[work->menu_cursor]^=1;
					val1=work->menu_value_cursor[work->menu_cursor];
#else
					if(work->key_press & PAD_L){
						work->menu_value_cursor[work->menu_cursor]--;

						if(work->menu_value_cursor[work->menu_cursor]<0){
							work->menu_value_cursor[work->menu_cursor]=2;
							val0=3; val1=2;
						}
						else{
							val1=work->menu_value_cursor[work->menu_cursor];
						}
					}
					else{
						work->menu_value_cursor[work->menu_cursor]++;
						val1=work->menu_value_cursor[work->menu_cursor];

						if(work->menu_value_cursor[work->menu_cursor]>=3){
							work->menu_value_cursor[work->menu_cursor]=0;
							val1=3;
						}
						else{
							val1=work->menu_value_cursor[work->menu_cursor];
						}
					}
#endif

#ifdef DEBUG_MODE
					printf("Rader : %d -> %d\n",val0,val1);
#endif
					work->explain_disp_flag=-1;

					KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[work->menu_cursor].menu_sw,
								menu_list[work->menu_cursor].sw[val0],
								menu_list[work->menu_cursor].sw[val1]);
				}

				// SE_SEL();
				SE_SWITCHOPT();
				return 0;
			case ITEM_QUICK:
				work->explain_disp_flag=-1;

			case ITEM_VIB:
			case ITEM_BLD:
			case ITEM_OWN:
			case ITEM_ITEM:
				{
					int val0,val1;

					val0=work->menu_value_cursor[work->menu_cursor];
					work->menu_value_cursor[work->menu_cursor]^=1;
					val1=work->menu_value_cursor[work->menu_cursor];

					KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[work->menu_cursor].menu_sw,
								menu_list[work->menu_cursor].sw[val0],
								menu_list[work->menu_cursor].sw[val1]);
				}

				// SE_SEL();
				SE_SWITCHOPT();
				return 0;
			case ITEM_SND:
				{
					int val0,val1;

					val0=work->menu_value_cursor[work->menu_cursor];
					work->menu_value_cursor[work->menu_cursor]^=1;
					val1=work->menu_value_cursor[work->menu_cursor];

					KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[work->menu_cursor].menu_sw,
								menu_list[work->menu_cursor].sw[val0],
								menu_list[work->menu_cursor].sw[val1]);

					if(val1){
						/* モノラル */
#ifdef PSX2						
						//sd_set_cli(SD_MONORAL);
#else
						//GM_SdSet(SD_MONORAL);
#endif						
					}
					else{
						/* ステレオ */
#ifdef PSX2												
						//sd_set_cli(SD_STEREO);
#else
						//GM_SdSet(SD_STEREO);
#endif						
					}

#if 1
					if(work->menu_value_cursor[work->menu_cursor]){
						/* モノラルならば、5.1CHをOFF */

						val0=work->menu_value_cursor[ITEM_5_1CH];
						work->menu_value_cursor[ITEM_5_1CH]=0;
						val1=work->menu_value_cursor[ITEM_5_1CH];

						KeyAnimSetX(work,1,SW_ANIM_COUNT,menu_list[ITEM_5_1CH].menu_sw,
									menu_list[ITEM_5_1CH].sw[val0],
									menu_list[ITEM_5_1CH].sw[val1]);
					}
#endif

				}

				// SE_SEL();
				SE_SWITCHOPT();
				return 0;
			case ITEM_5_1CH:
				if(work->menu_value_cursor[ITEM_SND]){
					/* モノラルであれば選択できない */
					break;
				}
				
				
#if 1 /* 5.1chの説明文は次の画面に表示 */
				work->action_strcode = CODE_NEXTTOCOLADJ;
				SE_OK();				   
				
				return 5;/* 5.1ch画面へ */

#else
				{
					int val0,val1;

					val0=work->menu_value_cursor[work->menu_cursor];
					work->menu_value_cursor[work->menu_cursor]^=1;
					val1=work->menu_value_cursor[work->menu_cursor];

					KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[work->menu_cursor].menu_sw,
								menu_list[work->menu_cursor].sw[val0],
								menu_list[work->menu_cursor].sw[val1]);

					if(work->menu_value_cursor[work->menu_cursor]){
						/* 5.1CHがONならば、ステレオ */

						val0=work->menu_value_cursor[ITEM_SND];
						work->menu_value_cursor[ITEM_SND]=0;
						val1=work->menu_value_cursor[ITEM_SND];

						KeyAnimSetX(work,1,SW_ANIM_COUNT,menu_list[ITEM_SND].menu_sw,
									menu_list[ITEM_SND].sw[val0],
									menu_list[ITEM_SND].sw[val1]);
					}
				}

				// SE_SEL();
				SE_SWITCHOPT();
				return 0;
#endif

			case ITEM_CAP:
				{
					int cur_pos;
					cur_pos = work->menu_cursor;

					if( work->key_press & (PAD_OK|PAD_L|PAD_R) ){
						int val0,val1;
					
						val0=work->menu_value_cursor[cur_pos];
					
						if(work->key_press & PAD_L){
							work->menu_value_cursor[cur_pos] --;
							if( work->menu_value_cursor[cur_pos] < 0 ){
								work->menu_value_cursor[cur_pos] = work->lang_num;
							}
							//work->menu_value_cursor[cur_pos] = work->lang_flag[work->lang_flag_cnt];
						} else {
							work->menu_value_cursor[cur_pos] ++;
							if( work->menu_value_cursor[cur_pos] > work->lang_num ){
								work->menu_value_cursor[cur_pos] = 0;
							}
							//work->menu_value_cursor[cur_pos] = work->lang_flag[work->lang_flag_cnt];
						}
					
						val1=work->menu_value_cursor[cur_pos];
					
						printf("va10[%d:%x],va11[%d:%x]\n",
							   val0, menu_list[cur_pos].sw[val0], 
							   val1, menu_list[cur_pos].sw[val1]);
						KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[cur_pos].menu_sw,
									menu_list[cur_pos].sw[val0],
									menu_list[cur_pos].sw[val1]);
					
						work->explain_disp_flag=-1;
						work->lang_val = val1;
					
						// SE_SEL();
						SE_SWITCHOPT();

						GM_Language = work->lang_ref[ val1 ];	/* この時点で言語を変えてしまう */
						//if( work->first_lang != GM_Language ){
							//	GM_TitleMenuStatus|=TITLE_MENU_OPTION_CHANGED;
							//}
					}
				}
				return 0;

			case ITEM_SCRN:
				if(!(work->key_press & PAD_OK)) break;

				work->action_strcode=CODE_NEXTTOSCRADJ;
				SE_OK();


//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
				// BgscrHideGenziOption(work);
//#endif

				return 3;
			case ITEM_COL:
				if(!(work->key_press & PAD_OK)) break;

				work->action_strcode=CODE_NEXTTOCOLADJ;
				SE_OK();


//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
				// BgscrHideGenziOption(work);
//#endif

				return 4;
			case ITEM_EXIT:
				if(!(work->key_press & PAD_OK)) break;

				work->action_strcode=CODE_HIDEOPT;
				SE_CANCEL();


//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
				// BgscrHideGenziOption(work);
//#endif

				return 1;
			}
		}

		{
			int cur0,cur1;

			cur0=work->menu_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
            do 
            {
               work->menu_cursor_screen--;
               if (work->menu_cursor_screen < 0)
               {
                  work->menu_cursor_screen = REPOSITION_COUNT - 1;
               }
               work->menu_cursor = skOptionReposition[work->menu_cursor_screen].item;

#ifdef GERMAN_TRIAL
               /* ドイツ語体験版は、血の項目を選択できない */
               if(work->menu_cursor == ITEM_BLD)
               {
                  work->menu_cursor = -1;
               }
#endif
            }
            while (work->menu_cursor == -1);

				SE_SEL();
				break;
			case PAD_D:
            do 
            {
               work->menu_cursor_screen++;

               if (work->menu_cursor_screen >= REPOSITION_COUNT)
               {
                  work->menu_cursor_screen = 0;
               }
               work->menu_cursor = skOptionReposition[work->menu_cursor_screen].item;

#ifdef GERMAN_TRIAL
               /* ドイツ語体験版は、血の項目を選択できない */
               if(work->menu_cursor == ITEM_BLD)
               {
                  work->menu_cursor = -1;
               }
#endif
            }
            while (work->menu_cursor == -1);

				SE_SEL();
				break;
			}

			cur1=work->menu_cursor;

			if(cur0!=cur1){
				KeyAnimSetX(work,0,MV_ANIM_COUNT,CUR_curs,
							menu_list[cur0].cur_pos,
							menu_list[cur1].cur_pos);
				work->explain_disp_flag=0;

				KeyAnimSetX(work,1,MV_ANIM_COUNT,menu_list[cur0].menu_item,
							menu_list[cur0].sel[ITEM_SEL_INDEX],
							menu_list[cur0].sel[ITEM_UNSEL_INDEX]);

            KeyAnimSetX(work,4,MV_ANIM_COUNT,menu_list[cur1].menu_item,
							menu_list[cur1].sel[ITEM_UNSEL_INDEX],
							menu_list[cur1].sel[ITEM_SEL_INDEX]);
			}
		}
		break;
	}

	KeyAnim(work);

	return 0;
}

static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_NUM_FADEIN:
		work->fade_count=value/DIV_TICK_VALUE;
		work->fade_alpha=0x80;

		// SE_WINOPEN();
		break;
	case CODE_NUM_FADEOUT:
		work->fade_count=value/DIV_TICK_VALUE;
		work->fade_alpha=0x00;
		break;
	case CODE_SOUND:
		SE_EXPANDLINE();
		break;
	}

#ifdef DEBUG_MODE
	printf("Option Layout Signal = %d ( 0x%08x )\n",sign,sign);
#endif
}

static void ColorLayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_SOUND:
		if(work->sub_step<0x10) SE_EXPANDLINE();
		else SE_WINCLOSEL2R();
		break;
	}

#ifdef DEBUG_MODE
	printf("Option Layout Signal = %d ( 0x%08x )\n",sign,sign);
#endif
}

static void StartSubL2D(Work *work,int strcode,int type)
{
	if(work->sub_l2d_handle<0){
		work->sub_l2d_handle=L2D_LoadLayout(strcode,DISP_CHANL,MENU_PRIORITY,0);

		/* シグナルハンドラの設定 */
		if(type == 1){
			L2D_SetSignalHandle(work->sub_l2d_handle,work,
								(void (*)(void *,int,int))ColorLayoutSignalFunc);
		}
		else if(type == 0){
			L2D_SetSignalHandle(work->sub_l2d_handle,work,
								(void (*)(void *,int,int))LayoutSignalFunc);
		}

		L2D_EvokeAction(work->sub_l2d_handle,CODE_DEFAULTACTION);

		work->sub_action_strcode=0;
		work->sub_busy_flag=1;
	}
}

static void GetScrnNum(Work *work)
{
	work->numxy[0][0]=L2D_GetObject(work->sub_l2d_handle,CODE_X_MINUS);
	work->numxy[0][1]=L2D_GetObject(work->sub_l2d_handle,CODE_NUM_X_10);
	work->numxy[0][2]=L2D_GetObject(work->sub_l2d_handle,CODE_NUM_X_01);

	work->numxy[1][0]=L2D_GetObject(work->sub_l2d_handle,CODE_Y_MINUS);
	work->numxy[1][1]=L2D_GetObject(work->sub_l2d_handle,CODE_NUM_Y_10);
	work->numxy[1][2]=L2D_GetObject(work->sub_l2d_handle,CODE_NUM_Y_01);

	work->fade_count=0;
	work->fade_alpha=0;
}

static void AnimScrnNum(Work *work)
{
	int x,y;
	int num10,num01;

	if(work->fade_count>0){
		int alpha;

		alpha=work->numxy[0][0]->sprite.col.a;
		alpha+=(work->fade_alpha-alpha)/work->fade_count;
		work->fade_count--;

		work->numxy[0][0]->sprite.col.a=alpha;
		work->numxy[0][1]->sprite.col.a=alpha;
		work->numxy[0][2]->sprite.col.a=alpha;
		work->numxy[1][0]->sprite.col.a=alpha;
		work->numxy[1][1]->sprite.col.a=alpha;
		work->numxy[1][2]->sprite.col.a=alpha;
	}
	else{
		int alpha;

		alpha=work->fade_alpha;

		work->numxy[0][0]->sprite.col.a=alpha;
		work->numxy[0][1]->sprite.col.a=alpha;
		work->numxy[0][2]->sprite.col.a=alpha;
		work->numxy[1][0]->sprite.col.a=alpha;
		work->numxy[1][1]->sprite.col.a=alpha;
		work->numxy[1][2]->sprite.col.a=alpha;
	}

	x=work->x;
	y=work->y;

	if(x<0){
		SPR_SHOW(work->numxy[0][0]);
		x=-x;
	}
	else{
		SPR_HIDE(work->numxy[0][0]);
	}

	num10=(x/10) % 10;
	num01=x % 10;

	if(num10>0){
		SPR_SHOW(work->numxy[0][1]);
		work->numxy[0][1]->sprite.head.tex.u=SPR_FIXED(num10*NUM_FONT_WIDTH);
		work->numxy[0][0]->sprite.pos.x=work->numxy[0][1]->sprite.pos.x
			-SPR_SCALE_X(0,NUM_FONT_WIDTH);
	}
	else{
		SPR_HIDE(work->numxy[0][1]);
		work->numxy[0][0]->sprite.pos.x=work->numxy[0][1]->sprite.pos.x;
	}
	work->numxy[0][2]->sprite.head.tex.u=SPR_FIXED(num01*NUM_FONT_WIDTH);
	SPR_SHOW(work->numxy[0][2]);

	if(y<0){
		SPR_SHOW(work->numxy[1][0]);
		y=-y;
	}
	else{
		SPR_HIDE(work->numxy[1][0]);
	}

	num10=(y/10) % 10;
	num01=y % 10;

	if(num10>0){
		SPR_SHOW(work->numxy[1][1]);
		work->numxy[1][1]->sprite.head.tex.u=SPR_FIXED(num10*NUM_FONT_WIDTH);
		work->numxy[1][0]->sprite.pos.x=work->numxy[1][1]->sprite.pos.x
			-SPR_SCALE_X(0,NUM_FONT_WIDTH);
	}
	else{
		SPR_HIDE(work->numxy[1][1]);
		work->numxy[1][0]->sprite.pos.x=work->numxy[1][1]->sprite.pos.x;
	}
	work->numxy[1][2]->sprite.head.tex.u=SPR_FIXED(num01*NUM_FONT_WIDTH);
	SPR_SHOW(work->numxy[1][2]);
}

static void EndSubL2D(Work *work)
{
	if(work->sub_l2d_handle>=0){
		L2D_ReleaseLayout(work->sub_l2d_handle);
		work->sub_l2d_handle=-1;
	}
}

static int ButtonConfigSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			work->sub_step++;
		}
		break;
	case 1:
		if(!work->sub_busy_flag){
			work->sub_step++;
		}
		break;
	case 2:
		if(work->key_press & PAD_CANCEL){
			SE_CANCEL();
			work->sub_step=0x20;
			break;
		}
		else if(work->key_press & PAD_OK){
			SE_OK();
			work->sub_step=0x10;
			break;
		}
		break;

	case 0x10:
		if(!work->sub_busy_flag){
			EndSubL2D(work);
			return 1;
		}
		break;
	case 0x20:
		if(!work->sub_busy_flag){
			EndSubL2D(work);
			return -1;
		}
		break;
	}

	return 0;
}

static int ScreenAdjustSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;

		work->wait_count--;
		if(work->wait_count>0) break;
		work->sub_step++;

		StartSubL2D(work,CODE_L2D_SCRN,0);
		GetScrnNum(work);

		work->old_x=work->x;
		work->old_y=work->y;

		work->sub_action_strcode=CODE_SHOWSCRADJ;

		// SE_WINOPEN();
		break;
	case 1:
		AnimScrnNum(work);

		if(!work->sub_busy_flag){
			work->sub_step++;
		}
		break;
	case 2:
		AnimScrnNum(work);

		if(work->key_press & PAD_CANCEL){
			work->sub_step=0x20;
			work->wait_count=DIRECT_TICK(6);

			SE_CANCEL();
			break;
		}
		else if(work->key_press & PAD_OK){
			// work->sub_action_strcode=CODE_HIDESCRADJ;
			work->sub_step=0x10;
			// work->wait_count=WAIT_COUNT;
			work->wait_count=DIRECT_TICK(6);

			SE_OK();
			// SE_WINCLOSE();
			break;
		}

		switch(work->key_autostatus & (PAD_U|PAD_D)){
		case PAD_U:
			work->y--;
			if(work->y<-SCRN_ADJUST_Y_LIMIT) work->y=-SCRN_ADJUST_Y_LIMIT;
			else{
				// SE_SEL();
				SE_SCRMOVE();
			}
			break;
		case PAD_D:
			work->y++;
			if(work->y>SCRN_ADJUST_Y_LIMIT) work->y=SCRN_ADJUST_Y_LIMIT;
			else{
				// SE_SEL();
				SE_SCRMOVE();
			}
			break;
		}
		switch(work->key_autostatus & (PAD_L|PAD_R)){
		case PAD_L:
			work->x--;
			if(work->x<-SCRN_ADJUST_X_LIMIT) work->x=-SCRN_ADJUST_X_LIMIT;
			else{
				// SE_SEL();
				SE_SCRMOVE();
			}
			break;
		case PAD_R:
			work->x++;
			if(work->x>SCRN_ADJUST_X_LIMIT) work->x=SCRN_ADJUST_X_LIMIT;
			else{
				// SE_SEL();
				SE_SCRMOVE();
			}
			break;
		}
		DG_SetDisplayOffset(work->x,work->y,0);

		break;

	case 0x10:
		AnimScrnNum(work);

		if(work->wait_count>0){
			work->wait_count--;
		}
		else{
			work->sub_action_strcode=CODE_HIDESCRADJ;
			work->sub_step++;
			work->wait_count=WAIT_COUNT;

			// SE_WINCLOSE();
		}
		break;
	case 0x11:
		AnimScrnNum(work);

		if(work->sub_busy_flag) break;

		work->wait_count--;
		if(work->wait_count>0) break;

		EndSubL2D(work);
		return 1;

	case 0x20:
		AnimScrnNum(work);

		if(work->wait_count>0){
			work->x+=(work->old_x-work->x)/work->wait_count;
			work->y+=(work->old_y-work->y)/work->wait_count;
			DG_SetDisplayOffset(work->x,work->y,0);

			work->wait_count--;
		}
		else{
			work->sub_action_strcode=CODE_HIDESCRADJ;
			work->sub_step++;
			work->wait_count=WAIT_COUNT;

			// SE_WINCLOSE();
		}
		break;
	case 0x21:
		AnimScrnNum(work);

		if(work->sub_busy_flag) break;

		work->wait_count--;
		if(work->wait_count>0) break;

		EndSubL2D(work);
		return -1;
	}

	return 0;
}

static int ColorAdjustSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;

		work->wait_count--;
		if(work->wait_count>0) break;
		work->sub_step++;

		StartSubL2D(work,CODE_L2D_COLOR,1);

		BgscrFadeout(work);

		CreateColorExplain(work);
		break;
	case 1:
		if(!work->sub_busy_flag){
			work->sub_action_strcode=CODE_SHOWCLRADJ;
			work->sub_step++;			

			work->wait_count=DIRECT_TICK(12);
		}
		break;
	case 2:
		if(work->wait_count>0){
			work->wait_count--;
			if(work->wait_count==0) SE_WINOPENR2L();
		}
		if(!work->sub_busy_flag){
			work->sub_step++;
		}
		break;
	case 3:
		PrintColorExplain(work);

		if(work->key_press & (PAD_OK|PAD_CANCEL)){
			work->sub_action_strcode=CODE_HIDECLRADJ;
			work->sub_step=0x10;
			work->wait_count=WAIT_COUNT;

			SE_OK();

			BgscrFadein(work);
			break;
		}
		break;

	case 0x10:
		if(work->sub_busy_flag) break;

		work->wait_count--;
		if(work->wait_count>0) break;

		EndSubL2D(work);
		return 1;
	}

	return 0;
}

static int Explain5_1chSubStep( Work *work ){
	switch( work->sub_step ){
	  case 0:
		if(work->busy_flag) break;

		work->wait_count--;
		if(work->wait_count>0) break;
		work->sub_step++;
		
		/* カーソルがどちらに合っているか */
		work->stat_5_1ch = work->menu_value_cursor[ITEM_5_1CH];
		StartSubL2D(work,GV_StrCode( "5_1ch" ),2);/* DefaultAction再生 */

		BgscrFadeout(work);

		Create5_1chExplain(work);

		break;
	case 1:
		if(!work->sub_busy_flag){
			{
				/* 文字表示位置をL2Dから取得 */
				SPR_OBJ *spr;
				spr = L2D_GetObject( work->sub_l2d_handle, GV_StrCode("NULL_moji") );
				if( spr != NULL ){
					/* 表示領域の真中 */
					work->x_pos_5_1ch = (int)spr->empty.pos.x;
					work->y_pos_5_1ch = (int)spr->empty.pos.y;
					printf("X:%d Y:%d\n",work->x_pos_5_1ch,work->y_pos_5_1ch);
					printf("X:%f Y:%f\n",spr->empty.pos.x,spr->empty.pos.y);
				}
			}


			if( work->stat_5_1ch == 0 ){
				work->sub_action_strcode=GV_StrCode("show_off");
			} else {
				work->sub_action_strcode=GV_StrCode("show_on");
			}
			//GM_SdSet( SD_S_WINOPNL1 );
			work->sub_step++;			

			work->wait_count=DIRECT_TICK(90);
		}
		break;
	case 2:
		if(work->wait_count>0){
			work->wait_count --;
			if(work->wait_count==DIRECT_TICK(50)){
				GM_SdSet( SD_S_WINOPNL1 );
			}
			if(work->wait_count==DIRECT_TICK(32)){
				GM_SdSet( SD_S_LINEMOV1 );
			}
			if(work->wait_count==DIRECT_TICK(2)){
				GM_SdSet( SD_S_WINOPN01 );
			}
		}
		if(!work->sub_busy_flag){
			work->sub_step++;
		}
		break;
	case 3:
		Print5_1chExplain(work);

		if(!work->sub_busy_flag){
			if(work->key_press & (PAD_L)){
				if( work->stat_5_1ch == 0 ){/* OFF-->ON */
					work->stat_5_1ch = 1;
					work->sub_action_strcode=GV_StrCode("off_on");
					GM_SdSet( SD_S_CUR01 );
				}
			}
			if(work->key_press & (PAD_R)){
				if( work->stat_5_1ch == 1 ){/* ON-->OFF */
					work->stat_5_1ch = 0;
					work->sub_action_strcode=GV_StrCode("on_off");
					GM_SdSet( SD_S_CUR01 );
				}
			}

			if(work->key_press & (PAD_OK|PAD_CANCEL)){
				if( work->key_press & (PAD_OK) ){
					/* メイン画面にも反映させる */
					int val;
					val = work->menu_value_cursor[ITEM_5_1CH] = work->stat_5_1ch;
					KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[ITEM_5_1CH].menu_sw,
								menu_list[ITEM_5_1CH].sw[val],
								menu_list[ITEM_5_1CH].sw[val]);
					GM_SdSet( SD_S_WIN01 );
				} else {
					GM_SdSet( SD_S_V_CANS02 );
				}
				if( work->stat_5_1ch == 0 ){
					work->sub_action_strcode=GV_StrCode("hide_off");
				} else {
					work->sub_action_strcode=GV_StrCode("hide_on");
				}
				work->sub_step=0x10;
				work->wait_count=DIRECT_TICK(35);
				
				BgscrFadein(work);
				break;
			}
		}
		break;

	case 0x10:
		work->wait_count--;
		if( work->wait_count == DIRECT_TICK(30) ){
			//GM_SdSet( SD_S_WINCLS01 );
		}
		if( work->wait_count == DIRECT_TICK(13) ){
			GM_SdSet( SD_S_LINEMOV1 );
		}
		if( work->wait_count == 0 ){
			GM_SdSet( SD_S_WINCLSL1 );
		}
		if(work->wait_count>0) break;

		if(work->sub_busy_flag) break;

		EndSubL2D(work);
		return 1;
	}

	return 0;
}

enum {
	OPTION_MAIN=0,
	OPTION_BUTTON_CONFIG,
	OPTION_SCREEN_ADJUST,
	OPTION_COLOR_ADJUST,
	OPTION_5_1CH_EXPLAIN,
};


static void Step(Work *work)
{
	switch(work->step){
	case OPTION_MAIN:
		switch(SubStep(work)){
		case 1:
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;
			GM_TitleMenuStatus|=TITLE_MENU_OPTION_CHANGED;
			break;
		case 2:
			work->step=OPTION_BUTTON_CONFIG;
			work->sub_step=0;
			break;
		case 3:
			work->step=OPTION_SCREEN_ADJUST;
			work->sub_step=0;
			work->key_repeat_time=REPEAT_NEXT2;
			work->wait_count=WAIT_COUNT_SCR;
			break;
		case 4:
			work->step=OPTION_COLOR_ADJUST;
			work->sub_step=0;
			work->wait_count=WAIT_COUNT_CLR;
			break;
		case 5:
			work->step=OPTION_5_1CH_EXPLAIN;
			work->sub_step=0;
			work->wait_count=WAIT_COUNT_CLR;
			break;
		case -1:
			work->ans=TITLE_MSG_MODECANCEL;
			work->step=0x10;
			break;
		}
		break;

	case OPTION_BUTTON_CONFIG:
		switch(ButtonConfigSubStep(work)){
		case 1:
		case -1:
			work->step=OPTION_MAIN;
			work->sub_step=0;
			break;
		}
		break;
	case OPTION_SCREEN_ADJUST:
		switch(ScreenAdjustSubStep(work)){
		case 1:
			GM_TitleMenuStatus|=TITLE_MENU_SCRADJ_CHANGED;

		case -1:
			work->step=OPTION_MAIN;
			work->sub_step=0;
			work->key_repeat_time=REPEAT_NEXT;
			break;
		}
		break;
	case OPTION_COLOR_ADJUST:
		switch(ColorAdjustSubStep(work)){
		case 1:
		case -1:
			work->step=OPTION_MAIN;
			work->sub_step=0;
			break;
		}
		break;

	case OPTION_5_1CH_EXPLAIN:
		switch(Explain5_1chSubStep(work)){
		case 1:
		case -1:
			work->step=OPTION_MAIN;
			work->sub_step=0;
			break;
		}
		break;

	case 0x10:
		if(!work->busy_flag){
			SetConfigValue(work);

			if(work->l2d_handle>=0){
				L2D_ReleaseLayout(work->l2d_handle);
				work->l2d_handle=-1;
			}

			CallLocalProc(work);

			if(work->strman!=NULL){
				GV_DestroyOtherActor(work->strman);
				work->strman=NULL;
			}

			GV_DestroyActor(work);
		}
		break;
	}

	CopyKeyAlpha(work);
}


static void Act(Work *work)
{
   ReArrangeOptions(work);

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

	if(work->sub_l2d_handle<0){
		work->sub_busy_flag=0;
	}
	else if(L2D_ActionStatus(work->sub_l2d_handle)==L2D_STAT_ACK){
		work->sub_busy_flag=0;
		if(work->sub_action_strcode!=0){
			L2D_EvokeAction(work->sub_l2d_handle,work->sub_action_strcode);
			work->sub_action_strcode=0;
			work->sub_busy_flag=1;
		}
	}
	else{
		work->sub_busy_flag=1;
	}

	Key(work);
	Step(work);
   ReArrangeOptions(work);
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
	work->bgname=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('b')!=NULL){
		work->bgname=GCL_GetNextInt();
    }

	// 以下、多言語対応追加
    if(GCL_GetOption('n')!=NULL){
		work->lang_num = GCL_GetNextInt();

      //BP_LANGUAGE - force available "Caption" options to "Off/On" only. 
      //NOTE: 
      // This overrides "Off/Japanese/English" Japanese options which are not available in 
      // "Sons Of Liberty". According to Ted, having the Japanese build include English and
      // Japanese caption data would be a lot of work. Both Japanese/English data would have
      // to be included and the function "NewGclLangUpdate" would have to updated
      work->lang_num = 1;
      //BP_LANGUAGE - force available "Caption" options to "Off/On" only. 
	}
    if(GCL_GetOption('l')!=NULL){
		int i, k;

		capstate_name[0] = SW_capOFF;
		work->lang_flag[0] = 0;
		k = 1;
		for( i = 0; i < work->lang_num; i++ ){
			int lang;

			lang = GCL_GetNextInt();
			if( i == 0 ){ /* 一つめの言語がデフォルト(OFFの時に表示される言語) */
				work->lang_default = GM_Language;
				work->lang_ref[0] = GM_Language;/* OFFの時の言語:
												   入ってきた時の言語を採用。
												 */
			}
			switch( lang ){
			  case GM_LANG_JAPANESE:
				work->lang_flag[lang] = k;
				work->lang_ref[k] = GM_LANG_JAPANESE; 
				capstate_name[k] = SW_capJPN;
				k ++;
				break;
			  case GM_LANG_ENGLISH:
			  default:
				work->lang_flag[lang] = k;
				work->lang_ref[k] = GM_LANG_ENGLISH;
				capstate_name[k] = SW_capENG;
				k ++;
				break;
			  case GM_LANG_FRENCH:
				work->lang_flag[lang] = k;
				work->lang_ref[k] = GM_LANG_FRENCH;
				capstate_name[k] = SW_capFRE;
				k ++;
				break;
			  case GM_LANG_GERMANY:
				work->lang_flag[lang] = k;
				work->lang_ref[k] = GM_LANG_GERMANY;
				capstate_name[k] = SW_capGER;
				k ++;
				break;
			  case GM_LANG_ITALY:
				work->lang_flag[lang] = k;
				work->lang_ref[k] = GM_LANG_ITALY;
				capstate_name[k] = SW_capITA;
				k ++;
				break;
			  case GM_LANG_SPANISH:
				work->lang_flag[lang] = k;
				work->lang_ref[k] = GM_LANG_SPANISH;
				capstate_name[k] = SW_capSPA;
				k ++;
				break;
			  case GM_LANG_KOREAN:
				work->lang_flag[lang] = k;
				work->lang_ref[k] = GM_LANG_KOREAN;
				capstate_name[k] = SW_capKRA;
				k ++;
				break;
			}
			if( work->lang_num == 1 ){/* 1言語の時はON,OFFのみ */
				work->lang_flag[lang] = 1;
				work->lang_ref[1] = work->lang_default;
				capstate_name[1] = SW_capON;
				break;
			}

		}
	}
    {/* リソース獲得 */
		int i;
		if(GCL_GetOption('r')!=NULL){			
			for( i = 0; i < LANG_MAX_NUM; i ++ ){
				/* 「オプションの説明」 */
				work->option_explain_resource[ i ] = GCL_GetNextInt();
				/* 「レーダーの説明」 */
				work->radar_explain_resource[ i ] = GCL_GetNextInt();
				/* 「クイックチェンジの説明」 */
				work->quick_explain_resource[ i ] = GCL_GetNextInt();
			}
		}
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
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))MainLayoutSignalFunc);

		/* layoutの初期化 */
		L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	}

    return 1;
}

static void InitMenu(Work *work)
{
	int i;

	work->cursor=L2D_GetObject(work->l2d_handle,CODE_SELCURSNULL);

	GetConfigValue(work);

   work->menu_cursor_screen = 0;
   work->menu_cursor = skOptionReposition[work->menu_cursor_screen].item;

	work->start_flag=0;

	for(i=0;i<ANIM_WORK_SIZE;i++){
		work->key_anim[i].count=0;
	}

//MULTI_LANG
	work->lang_val = work->menu_value_cursor[ITEM_CAP];
	work->first_val = work->menu_value_cursor[ITEM_CAP];
		
}

/* 初期化部メイン */
void *NewOptionScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->action_strcode=0;
		work->busy_flag=0;

		work->ret_signal_flag=0;

		work->sub_l2d_handle=-1;
		work->sub_action_strcode=0;
		work->sub_busy_flag=0;

		work->step=0;
		work->sub_step=0;
		work->ans=0;

		work->strman=NULL;

		work->key_repeat_time=REPEAT_NEXT;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL ;
		}
		if((work->strman=NewTextScreenControlForTitle())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		InitMenu(work);

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}

void *NewOptionScrForNode(int parent_name,int l2d_handle,void *strman)
{
    Work *work ;

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

	work->parent_name=parent_name;
	work->proc=0;
	work->l2d_handle=l2d_handle;
	work->action_strcode=0;
	work->busy_flag=0;

	work->ret_signal_flag=1;

	work->sub_l2d_handle=-1;
	work->sub_action_strcode=0;
	work->sub_busy_flag=0;

	work->step=0;
	work->sub_step=0;
	work->ans=0;

	work->key_repeat_time=REPEAT_NEXT;

	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX(&(work->actor));

	InitMenu(work);

	work->strman=strman;

    return (void *)work ;
}

static void _hide_object(Work *work, int strcode)
{
   if (strcode != -1)
   {
      SPR_OBJ *pObj = L2D_GetObject(work->l2d_handle, strcode);
      if (pObj)
      {
         SPR_HIDE(pObj);
      }
   }
}

static void _set_object_y(Work *work, int strcode, int y)
{
   if (strcode != -1)
   {
      SPR_OBJ *pObj = L2D_GetObject(work->l2d_handle, strcode);
      if (pObj)
      {
         pObj->sprite.pos.y = y;
      }
   }
}

static void _hide_menu_item(Work *work, int item)
{
   _hide_object(work, menu_list[item].menu_item);
   _hide_object(work, menu_list[item].menu_colon);
   _hide_object(work, menu_list[item].menu_sw);
}

static void _set_menu_item_y(Work *work, int item, int y)
{
   _set_object_y(work, menu_list[item].menu_item, y);
   _set_object_y(work, menu_list[item].menu_colon, y);
   _set_object_y(work, menu_list[item].menu_sw, y);
}

void ReArrangeOptions(Work *work)
{
   //BP JG - This code is designed to move the menu items around that are build in script.
   //AS SM - Why didn't you do this in the data? WHY?
   static float skFirstItemY = 71;
   static float skItemSpacing = 22;
   SPR_OBJ *pObject;
   int i;

   // adjust the title underline.
   pObject = L2D_GetObject(work->l2d_handle,0x008dd8e0);
   if ( pObject ) pObject->sprite.dh = 1;
   if ( pObject ) pObject->sprite.pos.y = 48;

   // hide items
   _hide_menu_item(work, ITEM_SND);
   _hide_menu_item(work, ITEM_VIB);
   _hide_menu_item(work, ITEM_5_1CH);
   _hide_menu_item(work, ITEM_SCRN);
   _hide_menu_item(work, ITEM_ITEM);
   _hide_menu_item(work, ITEM_COL);

   for (i = 0; i < REPOSITION_COUNT; ++i)
   {
      if (skOptionReposition[i].item != -1)
      {
         _set_menu_item_y(work, skOptionReposition[i].item, skFirstItemY + i*skItemSpacing + skOptionReposition[i].offset);
         menu_list[skOptionReposition[i].item].cur_pos = skOptionReposition[i].cur_pos;
      }
   }
}
