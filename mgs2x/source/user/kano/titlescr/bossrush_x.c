/*
	bossrush_x.c
		ボスラッシュ選択画面(XBOX用)

	2002/08/13  Y.Yano
	$Id: bossrush_x.c,v 1.4 2002/12/13 11:50:52 takaki Exp $
*/

#include "titlescr_x.h"

#include "subtitle.h"

#include "font.h"

#include "xtextscn.h"

#define CODE_ITEM_EXIT				0x0077efb7		/* menuExit */
#define CODE_KEY_HIDEEXIT			0x0077c8a2		/* hideExit */
#define CODE_KEY_SHOWEXIT			0x00982079		/* showExit */
#define CODE_KEY_SELEXIT			0x00e48b24		/* selExit */

#define CODE_ITEM_OPTION			0x00444041		/* menuOption */
#define CODE_KEY_HIDEOPT			0x0073e54c		/* hideOpt */
#define CODE_KEY_SHOWOPT			0x002ce80b		/* showOpt */
#define CODE_KEY_SELOPT				0x00874b60		/* selOpt */

#define CODE_ITEM_SNAKE				0x00d8d634		/* menuSnake */
#define CODE_KEY_HIDESNAKE			0x0073f503		/* hideSnk */
#define CODE_KEY_SHOWSNAKE			0x002cf7c2		/* showSnk */
#define CODE_KEY_SELSNAKE			0x00875b17		/* selSnk */

#define CODE_ITEM_RAIDEN			0x004eab06		/* menuRaiden */
#define CODE_KEY_HIDERAIDEN			0x0073ef61		/* hideRai */
#define CODE_KEY_SHOWRAIDEN			0x002cf220		/* showRai */
#define CODE_KEY_SELRAIDEN			0x00875575		/* selRai */

#define CODE_CURSOR_LOCATE			0x006719f0		/* selCursNull */
#define CODE_KEY_CURSRAIDEN			0x005aee3a		/* cursRai */
#define CODE_KEY_CURSSNAKE			0x005af3dc		/* cursSnk */
#define CODE_KEY_CURSOPT			0x005ae425		/* cursOpt */
#define CODE_KEY_CURSEXIT			0x0057a3bf		/* cursExit */

#define CODE_CURSOR_DISP			0x00e38043		/* selCurs */
#define CODE_KEY_HIDECURS			0x0076bdc1		/* hideCurs */
#define CODE_KEY_SHOWCURS			0x00971598		/* showCurs */

#define CODE_OPEN_BOSS				0x00065fa4		/* openBoss */
#define CODE_CLOSE_BOSS				0x00d9477f		/* closeBoss */
#define CODE_OPEN_BOSSOPT			0x00d39bf4		/* openBossOption */
#define CODE_CLOSE_BOSSOPT			0x008d92e9		/* closeBossOption */
#define CODE_OPEN_DIFFICULTY		0x00d320b5		/* openBossDif */
#define CODE_CLOSE_DIFFICULTY		0x00c10a29		/* closeBossDif */

#define CODE_SIGNAL_SHOWMENU		0x009bd51a		/* showMenu */
#define CODE_SIGNAL_HIDEMENU		0x007b7d43		/* hideMenu */
#define CODE_SIGNAL_SOUND			0x0069622b		/* sound */

#define CODE_QUICKON				0x00c3535e		/* quickON */
#define CODE_QUICKOFF				0x006a6b1e		/* quickOFF */

#define CODE_ITEM_EASY				0x007794fc		/* menuEasy */
#define CODE_ITEM_NORMAL			0x00334e7d		/* menuNormal */
#define CODE_ITEM_HARD				0x007914c7		/* menuHard */

#define CODE_KEY_HIDEEASY			0x00776de7		/* hideEasy */
#define CODE_KEY_HIDENORMAL			0x0096fa7c		/* hideNormal */
#define CODE_KEY_HIDEHARD			0x0078edb2		/* hideHard */
#define CODE_KEY_SHOWEASY			0x0097c5be		/* showEasy */
#define CODE_KEY_SHOWNORMAL			0x00f656fd		/* showNormal */
#define CODE_KEY_SHOWHARD			0x00994589		/* showHard */
#define CODE_KEY_UNSELEASY			0x0097c5be		/* showEasy */
#define CODE_KEY_UNSELNORMAL		0x00f656fd		/* showNormal */
#define CODE_KEY_UNSELHARD			0x00994589		/* showHard */
#define CODE_KEY_SELEASY			0x00e43069		/* selEasy */
#define CODE_KEY_SELNORMAL			0x00a1042f		/* selNormal */
#define CODE_KEY_SELHARD			0x00e5b034		/* selHard */
#define CODE_KEY_CUREASY			0x00574904		/* cursEasy */
#define CODE_KEY_CURNORMAL			0x00036dfc		/* cursNormal */
#define CODE_KEY_CURHARD			0x0058c8cf		/* cursHard */


#define MOVE_CURSOR_COUNT			DIRECT_TICK(10)


#define ANIM_WORK_SIZE		5


#define DISP_CHANL			TITLE_CHANL
#define MENU_PRIORITY		TITLE_PRI_NORMAL

#define REPEAT_FIRST		20
#define REPEAT_NEXT			1
#define REPEAT_NEXT2		3

#define SW_ANIM_COUNT		1


enum {
	MENU_ITEM_RAIDEN=0,
	MENU_ITEM_SNAKE,
	MENU_ITEM_OPTION,
	MENU_ITEM_EXIT,

	N_MENU_ITEMS,
};

enum {
	OPTION_ITEM_VIB,
	OPTION_ITEM_RAD,
	OPTION_ITEM_BLOOD,
	// OPTION_ITEM_SND,
	// OPTION_ITEM_5_1CH,
	// OPTION_ITEM_CAP,
	OPTION_ITEM_CTR,

	OPTION_ITEM_REV,
	OPTION_ITEM_ITEMMENU,
	OPTION_ITEM_QUICK,

	OPTION_ITEM_EXIT,

	N_OPTION_ITEMS,
};

enum {
	DIFFICULTY_ITEM_EASY=0,
	DIFFICULTY_ITEM_NORMAL,
	DIFFICULTY_ITEM_HARD,

	N_DIFFICULTY_ITEMS,
};


enum {
	START_STEP=0,
	MAIN_MENU,
	LEVEL_MENU,
	OPTION_MENU,
	CONTROLS_SELECT,

	EXIT_STEP,
};

#define LANG_NUM_MAX 7

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
	int option_cursor;
	int option_menu_cursor[N_OPTION_ITEMS];
	int difficulty_cursor;

	int start_mode;

	int sound_flag;

	struct {
		int part_code;
		int key0_code;
		int key1_code;
		float p;
		int count;
	} key_anim[ANIM_WORK_SIZE];

	int explain_disp_flag;
	int explain_columns;

	void *strman;

	float t_y_pos;
	int y_pos_count;

	struct {
		int spr_code;
		int t_alpha;
		int count;
	} alphaanim[2];

	//int base_v[N_OPTION_ITEMS],base_pv[N_OPTION_ITEMS];
	SPR_FIX base_v[N_OPTION_ITEMS],base_pv[N_OPTION_ITEMS];

	int con_flag;
} Work;


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
#define OPTION_EXPLAIN_HEIGHT	(1*FONT_HEIGHT)
#else
#define OPTION_EXPLAIN_HEIGHT	(3*LINE_HEIGHT+DOWN_MARGINE)
#endif

#define OPTION_EXPLAIN_X		SUBTITLE_X
#define OPTION_EXPLAIN_Y		SUBTITLE_Y
#if 0
#define OPTION_EXPLAIN_W		(30*FONT_DISP_WIDTH)
#define OPTION_EXPLAIN_H		(1*FONT_DISP_HEIGHT)
#else
#define OPTION_EXPLAIN_W		(OPTION_EXPLAIN_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define OPTION_EXPLAIN_H		(OPTION_EXPLAIN_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define TEXTCOLOR				SUBTITLE_TEXTCOLOR


#define OPTION_EXPLAIN_RESOURCE		(option_resource_name[GM_Language-GM_LANG_ENGLISH])/* オプションの説明 */
#define RADAR_EXPLAIN_RESOURCE		(radar_resource_name[GM_Language-GM_LANG_ENGLISH])	/* レーダーの説明 */
#define QUICK_EXPLAIN_RESOURCE		(quick_resource_name[GM_Language-GM_LANG_ENGLISH])	/* クイックチェンジの説明 */

#define E_OPTION_EXPLAIN_RESOURCE	0x0006591e		/* オプションの説明 */
#define E_RADAR_EXPLAIN_RESOURCE	0x001bface		/* レーダーの説明 */
#define E_QUICK_EXPLAIN_RESOURCE	0x006d6db2		/* クイックチェンジの説明 */

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

#define J_OPTION_EXPLAIN_RESOURCE	0x001dd032		/* オプションの説明スペイン語 */
#define J_RADAR_EXPLAIN_RESOURCE	0x00a49088		/* レーダーの説明スペイン語 */
#define J_QUICK_EXPLAIN_RESOURCE	0x007021ce		/* クイックチェンジの説明スペイン語 */

static const int option_resource_name[]={
	E_OPTION_EXPLAIN_RESOURCE,
	F_OPTION_EXPLAIN_RESOURCE,
	G_OPTION_EXPLAIN_RESOURCE,
	I_OPTION_EXPLAIN_RESOURCE,
	S_OPTION_EXPLAIN_RESOURCE,
	0,
	J_OPTION_EXPLAIN_RESOURCE,
};

static const int radar_resource_name[]={
	E_RADAR_EXPLAIN_RESOURCE,
	F_RADAR_EXPLAIN_RESOURCE,
	G_RADAR_EXPLAIN_RESOURCE,
	I_RADAR_EXPLAIN_RESOURCE,
	S_RADAR_EXPLAIN_RESOURCE,
	0,
	J_RADAR_EXPLAIN_RESOURCE,
};

static const int quick_resource_name[]={
	E_QUICK_EXPLAIN_RESOURCE,
	F_QUICK_EXPLAIN_RESOURCE,
	G_QUICK_EXPLAIN_RESOURCE,
	I_QUICK_EXPLAIN_RESOURCE,
	S_QUICK_EXPLAIN_RESOURCE,
	0,
	J_QUICK_EXPLAIN_RESOURCE,
};




static const int config_flags[]={
	GM_CONFIG_VIBRATION_OFF,
	GM_CONFIG_RADAR_OFF,
	GM_CONFIG_BLOOD_OFF,
	// GM_CONFIG_SOUND_MONAURAL,
	// GM_CONFIG_SOUND_5_1CHANL,
	// GM_CONFIG_CAPTION_OFF,

	0,/* controls */

	GM_CONFIG_SHUKAN_REVERSE,
	GM_CONFIG_OLD_TYPE_MENU,
	GM_CONFIG_MENU_QCHANGE_EX,
};

enum {
	ITEM_SEL_INDEX=0,
	ITEM_UNSEL_INDEX,
};

/* titlescr_x.h と定義が食い違っているもの */
#define SW_ownNORM	0x086342	/* "ownNORM" */
#define SW_ownREV	0xa851db	/* "ownREV" */
#define SW_itemLINEAR2	0x4f605c	/* "itemLINEAR" */
#define SW_itemGROUP2	0x07010f		/* "itemGROUP" */

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

static const int capstate_name[]={ SW_capON,SW_capOFF, };

static const int ownstate_name[]={ SW_ownNORM,SW_ownREV, };

static const int itemstate_name[]={ SW_itemGROUP2,SW_itemLINEAR2, };

static const int quickstate_name[]={ CODE_QUICKON,CODE_QUICKOFF, };

static const int controls_name[]={ SW_ctrlTYPEA, SW_ctrlTYPEB, SW_ctrlTYPEC, SW_ctrlTYPED, };

#define curO_CTR 2346235
#define optCTR	3872695
#define selCTR	8853438
#define unCTR	8918745	

static const struct {
	int cur_pos;
	int menu_item;
	int menu_colon;
	int menu_sw;
	int sel[2];
	const int *sw;
} option_menu_list[] = {
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

#if 0
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
#endif

	/* キーコンフィグCONTROLS */
	{ curO_CTR,
	  optCTR, colonCTR, OBJ_swCTR,             
	  { selCTR, unCTR, },
	  controls_name,
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

	{ POS_curEXIT,
	  OBJ_optEXIT,  -1, -1,              /* 脱出 */
	  { KEY_selEXIT, KEY_unEXIT, },
	  NULL,
	},
};

static const float option_sw_v[][3]={
	{  0.0f,12.0f, 0.0f, }, /* vib */
	{  0.0f,12.0f,24.0f, }, /* rad */
	{  0.0f,12.0f, 0.0f, }, /* bld */
	//{  0.0f,12.0f, 0.0f, }, /* snd */
	//{ 12.0f, 0.0f, 0.0f, }, /* 5_1ch */
	//{  0.0f,12.0f, 0.0f, }, /* cap */
	{  0.0f, 0.0f, 0.0f, }, /* ctr(dumy) */

	{  0.0f,12.0f, 0.0f, }, /* own */
	{  0.0f,12.0f, 0.0f, }, /* item */
	{  0.0f,12.0f, 0.0f, }, /* quick */
};

static const struct {
	int cur_pos;

	int item;

	int hide;
	int unsel;
	int sel;
} main_menu_list[]={
	{
		CODE_KEY_CURSRAIDEN,

		CODE_ITEM_RAIDEN,

		CODE_KEY_HIDERAIDEN,
		CODE_KEY_SHOWRAIDEN,
		CODE_KEY_SELRAIDEN,
	},
	{
		CODE_KEY_CURSSNAKE,

		CODE_ITEM_SNAKE,

		CODE_KEY_HIDESNAKE,
		CODE_KEY_SHOWSNAKE,
		CODE_KEY_SELSNAKE,
	},
	{
		CODE_KEY_CURSOPT,

		CODE_ITEM_OPTION,

		CODE_KEY_HIDEOPT,
		CODE_KEY_SHOWOPT,
		CODE_KEY_SELOPT,
	},
	{
		CODE_KEY_CURSEXIT,

		CODE_ITEM_EXIT,

		CODE_KEY_HIDEEXIT,
		CODE_KEY_SHOWEXIT,
		CODE_KEY_SELEXIT,
	},
};

static const struct {
	int cur_pos;

	int item;

	int hide;
	int unsel;
	int sel;
} difficulty_menu_list[]={
	{
		CODE_KEY_CUREASY,

		CODE_ITEM_EASY,

		CODE_KEY_HIDEEASY,
		CODE_KEY_SHOWEASY,
		CODE_KEY_SELEASY,
	},
	{
		CODE_KEY_CURNORMAL,

		CODE_ITEM_NORMAL,

		CODE_KEY_HIDENORMAL,
		CODE_KEY_SHOWNORMAL,
		CODE_KEY_SELNORMAL,
	},
	{
		CODE_KEY_CURHARD,

		CODE_ITEM_HARD,

		CODE_KEY_HIDEHARD,
		CODE_KEY_SHOWHARD,
		CODE_KEY_SELHARD,
	},
};


int sd_set_cli(int);


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

static void KeyAnimSet0(Work *work,int part_code,int key_code)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,part_code);
	if(part==NULL) return;

	L2D_MorfObject(part,key_code,key_code,1.0f);
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
					main_menu_list[i].item,main_menu_list[i].hide,main_menu_list[i].unsel);
	}
	KeyAnimSetX(work,i,count,CODE_CURSOR_DISP,CODE_KEY_HIDECURS,CODE_KEY_SHOWCURS);
}

static void ShowDifficulty(Work *work,int count)
{
	int i;
	for(i=0;i<N_DIFFICULTY_ITEMS;i++){
		KeyAnimSetX(work,i,count,
					difficulty_menu_list[i].item,
					difficulty_menu_list[i].hide,difficulty_menu_list[i].unsel);
	}
	KeyAnimSetX(work,i,count,CODE_CURSOR_DISP,CODE_KEY_HIDECURS,CODE_KEY_SHOWCURS);
}

static void LocalShowCursor(Work *work,int count)
{
	KeyAnimSetX(work,0,count,CODE_CURSOR_DISP,CODE_KEY_HIDECURS,CODE_KEY_SHOWCURS);
}

static void HideMenu(Work *work,int count)
{
	int i;
	for(i=0;i<N_MENU_ITEMS;i++){
		KeyAnimSetX(work,i,count,
					main_menu_list[i].item,main_menu_list[i].unsel,main_menu_list[i].hide);
	}
	KeyAnimSetX(work,i,count,CODE_CURSOR_DISP,CODE_KEY_SHOWCURS,CODE_KEY_HIDECURS);
}

static void HideDifficulty(Work *work,int count)
{
	int i;
	for(i=0;i<N_DIFFICULTY_ITEMS;i++){
		KeyAnimSetX(work,i,count,
					difficulty_menu_list[i].item,
					difficulty_menu_list[i].unsel,difficulty_menu_list[i].hide);
	}
	KeyAnimSetX(work,i,count,CODE_CURSOR_DISP,CODE_KEY_SHOWCURS,CODE_KEY_HIDECURS);
}

static void HideCursor(Work *work,int count)
{
	KeyAnimSetX(work,0,count,CODE_CURSOR_DISP,CODE_KEY_SHOWCURS,CODE_KEY_HIDECURS);
}

static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_SIGNAL_SHOWMENU:
		switch(work->step){
		case MAIN_MENU:
			ShowMenu(work,value/DIV_TICK_VALUE);
			break;
		case LEVEL_MENU:
			ShowDifficulty(work,value/DIV_TICK_VALUE);
			break;
		case OPTION_MENU:
			LocalShowCursor(work,value/DIV_TICK_VALUE);
			break;
		}
		break;
	case CODE_SIGNAL_HIDEMENU:
		switch(work->step){
		case MAIN_MENU:
			HideMenu(work,value/DIV_TICK_VALUE);
			break;
		case LEVEL_MENU:
			HideDifficulty(work,value/DIV_TICK_VALUE);
			break;
		case OPTION_MENU:
			HideCursor(work,value/DIV_TICK_VALUE);
			break;
		}
		break;
	case CODE_SIGNAL_SOUND:
		if(work->sound_flag) SE_EXPANDLINE();
		else SE_WINOPEN();
		break;
	}

#ifdef DEBUG_MODE
	printf("Bossrush Layout Signal = %d ( 0x%08x )\n",sign,sign);
#endif

}

static void CopyKeyAlpha(Work *work)
{
	static const int strcode[][2]={
		{ optCTR,colonCTR, },
		{ optCTR,OBJ_swCTR, },
		{ OBJ_optVIB,OBJ_swVIB, },
		{ OBJ_optRAD,OBJ_swRAD, },
		{ OBJ_optBLD,OBJ_swBLD, },
		{ OBJ_optSND,OBJ_swSND, },
		// { OBJ_opt5_1CH,OBJ_sw5_1CH, },
		{ OBJ_optCAP,OBJ_swCAP, },
		{ OBJ_optOWN,OBJ_swOWN, },
		{ OBJ_optITEM,OBJ_swITEM, },
		{ OBJ_optQUICK,OBJ_swQUICK, },

		{ OBJ_optVIB,CODE_MENUVIB_COL, },
		{ OBJ_optRAD,CODE_MENURAD_COL, },
		{ OBJ_optBLD,CODE_MENUBLD_COL, },
		{ OBJ_optSND,CODE_MENUSND_COL, },
		// { OBJ_opt5_1CH,CODE_MENU5_1CH_COL, },
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
}

static void AlphaAnimSet0(Work *work,int strcode,int t_alpha)
{
	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,strcode);
	if(spr!=NULL){
		spr->sprite.col.a=t_alpha;
	}
}

static void AlphaAnimSet(Work *work,int index,int strcode,int t_alpha,int count)
{
	work->alphaanim[index].spr_code=strcode;
	work->alphaanim[index].t_alpha=t_alpha;
	work->alphaanim[index].count=count;
}

static void YPosAnimSet0(Work *work,int index)
{
	SPR_OBJ *spr;
	SPR_OBJ *cur;

	spr=L2D_GetObject(work->l2d_handle,option_menu_list[index].menu_item);
	if(spr==NULL) return;

	cur=L2D_GetObject(work->l2d_handle,CODE_CURSOR_LOCATE);
	if(cur!=NULL){
		cur->empty.pos.y=spr->sprite.pos.y;
	}
}

static void YPosAnimSet(Work *work,int index,int count)
{
	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,option_menu_list[index].menu_item);
	if(spr!=NULL){
		work->t_y_pos=spr->sprite.pos.y;
		work->y_pos_count=count;
	}
}

static void AlphaAnim(Work *work)
{
	int i;
	for(i=0;i<2;i++){
		if(work->alphaanim[i].count>0){
			SPR_OBJ *spr;

			spr=L2D_GetObject(work->l2d_handle,work->alphaanim[i].spr_code);
			if(spr!=NULL){
				spr->sprite.col.a
					+=(work->alphaanim[i].t_alpha-spr->sprite.col.a)/work->alphaanim[i].count;
			}

			work->alphaanim[i].count--;
		}
	}

	if(work->y_pos_count>0){
		SPR_OBJ *spr;

		spr=L2D_GetObject(work->l2d_handle,CODE_CURSOR_LOCATE);
		if(spr!=NULL){
			spr->empty.pos.y
				+=(work->t_y_pos-spr->empty.pos.y)/(float)(work->y_pos_count);
		}

		work->y_pos_count--;
	}
}

static void ChangeVValue(Work *work,int strcode,float v,SPR_FIX base_v,SPR_FIX base_pv)
{
	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,strcode);
	if(spr!=NULL){
		spr->sprite.head.tex.v=base_v+SPR_FIXED(v);
		spr->sprite.head.tex.pv=base_pv+SPR_FIXED(v);
	}
}


/* ------------------------------------------------------------------------ */


static void CreateOptionExplain(Work *work)
{
	if(!work->explain_disp_flag){
		char *str;
		int i;

		switch(work->option_cursor){
		case OPTION_ITEM_RAD:
			{
				int index=work->option_menu_cursor[OPTION_ITEM_RAD]-1;

				if(index<0) index=3-1;
				
				/* レーダーの説明が他の部分での説明と違うので場所をスキップ */
				index += 3;

				str=(char *)GetLocalResource(RADAR_EXPLAIN_RESOURCE,index);
			}
			break;
		case OPTION_ITEM_QUICK:
			{
				int index=work->option_menu_cursor[OPTION_ITEM_QUICK];
				str=(char *)GetLocalResource(QUICK_EXPLAIN_RESOURCE,index);
			}
			break;
		default:
			{
				int index=work->option_cursor;

				if(index==OPTION_ITEM_EXIT){
					/* SND,5.1ch,CAP,COLOR ADJを飛ばす */
					index+=4;
				}
				else if(index>OPTION_ITEM_BLOOD){
					/* SND,5.1ch,CAPを飛ばす */
					index+=3;
				}

				str=(char *)GetLocalResource(OPTION_EXPLAIN_RESOURCE,index);
			}
			break;
		}

		work->explain_columns=1;

		i=0;
		while(*(str+i)!='\0'){
			if(*(str+i)=='|'){
				work->explain_columns++;
			}
			i++;
		}

		MENU_ClearTextTexture(work->strman);

		// spaceを無理やり設定する。
		MENU_SetTextSpace( work->strman, 0 );
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
	if(work->explain_disp_flag>0){
		int y=OPTION_EXPLAIN_Y;
		int h=OPTION_EXPLAIN_H;

		work->explain_disp_flag=1;

		if(work->explain_columns>1){
			y-=FONT_DISP_HEIGHT/2;
			h=h*5/6;
		}

		MENU_PutTextScreen(work->strman,
						   OPTION_EXPLAIN_X,y,
						   OPTION_EXPLAIN_X+OPTION_EXPLAIN_W,y+h,
						   OPTION_EXPLAIN_U,
						   OPTION_EXPLAIN_V,
						   OPTION_EXPLAIN_U+OPTION_EXPLAIN_WIDTH,
						   OPTION_EXPLAIN_V+OPTION_EXPLAIN_HEIGHT,
						   TEXTCOLOR);
	}
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
		argv[1]=work->difficulty_cursor;

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


static void GetConfigValue(Work *work)
{
	int i;
	for(i=OPTION_ITEM_VIB;i<=OPTION_ITEM_QUICK;i++){
		if(i==OPTION_ITEM_RAD) continue;
		if(i==OPTION_ITEM_CTR) continue;
		work->option_menu_cursor[i]=((GM_Configuration & config_flags[i])!=0);
	}

	/* レーダーだけは例外 */
	switch(GM_Configuration & (GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE)){
	case 0:
		/* TYPE 1 */
		work->option_menu_cursor[OPTION_ITEM_RAD]=1;
		break;
	case GM_CONFIG_RADAR_OFF_INTRUDE:
		/* TYPE 2 */
		work->option_menu_cursor[OPTION_ITEM_RAD]=2;
		break;
	case GM_CONFIG_RADAR_OFF:
	case GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE:
		/* OFF */
		work->option_menu_cursor[OPTION_ITEM_RAD]=0;
		break;
	}

	/* キーコンフィグ CONTROLS の設定 */
	{
		unsigned int val;
		val = GM_Configuration2 & GM_CONFIG_CONTROLS;
		val = ( val >> 8 );
		work->option_menu_cursor[OPTION_ITEM_CTR] = val;
		work->con_flag = -1;/* 監視変数の初期化 */
	} 
#ifdef DEBUG_MODE
	printf("GM_Configuration = 0x%08x\n",GM_Configuration);
#endif

}

static void SetConfigValue(Work *work)
{
	int vand=0,vor=0;
	int i;

	for(i=OPTION_ITEM_VIB;i<=OPTION_ITEM_QUICK;i++){
		if(i==OPTION_ITEM_RAD) continue;
		if(i==OPTION_ITEM_CTR) continue;
		// if(i==OPTION_ITEM_5_1CH) continue;
		if(work->option_menu_cursor[i]) vor|=config_flags[i];
		else vand|=config_flags[i];
	}

#if 0
	/* 5.1chは、ステレオの場合のみ */
	if(!work->option_menu_cursor[OPTION_ITEM_SND]){
		if(work->option_menu_cursor[OPTION_ITEM_5_1CH]) vor|=config_flags[OPTION_ITEM_5_1CH];
		else vand|=config_flags[OPTION_ITEM_5_1CH];
	}
	else{
		vand|=config_flags[OPTION_ITEM_5_1CH];
	}
#endif

	/* レーダーだけは例外 */
	switch(work->option_menu_cursor[OPTION_ITEM_RAD]){
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

	/* キーコンフィグ CONTROLS */
	{
		int val;
		val = work->option_menu_cursor[OPTION_ITEM_CTR];
		GM_Configuration2 &= ~GM_CONFIG_CONTROLS;
		GM_Configuration2 |= ( (val<<8) & GM_CONFIG_CONTROLS );
	}

	vand=~vand;
	GM_Configuration&=vand;
	GM_Configuration|=vor;

#ifdef DEBUG_MODE
	printf("GM_Configuration = 0x%08x\n",GM_Configuration);
#endif

}


/* ------------------------------------------------------------------------ */


static int mainStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;
		work->action_strcode=CODE_OPEN_BOSS;
		work->sound_flag=0;
		work->sub_step++;

		KeyAnimSet0(work,CODE_CURSOR_LOCATE,main_menu_list[work->menu_cursor].cur_pos);

		SE_EXPANDLINE();
		break;
	case 1:
		if(work->busy_flag) break;
		work->sub_step++;

		KeyAnimSet0(work,main_menu_list[work->menu_cursor].item,
					main_menu_list[work->menu_cursor].sel);
	case 2:
		if(work->key_anim[0].count>0) break;

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_CLOSE_BOSS;
			work->sound_flag=1;

			work->sub_step=0x10;
			work->ans=-1;

			SE_CANCEL();
			return 0;
		}
		else if(work->key_press & PAD_OK){
			work->action_strcode=CODE_CLOSE_BOSS;
			work->sound_flag=1;

			work->sub_step=0x10;

			switch(work->menu_cursor){
			case MENU_ITEM_RAIDEN:
			case MENU_ITEM_SNAKE:
				SE_OK();
				work->ans=1;
				break;
			case MENU_ITEM_OPTION:
				SE_OK();
				work->ans=2;
				break;
			case MENU_ITEM_EXIT:
				SE_CANCEL();
				work->ans=-1;
				break;
			}
			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->menu_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->menu_cursor--;
				if(work->menu_cursor<0) work->menu_cursor=N_MENU_ITEMS-1;

				SE_SEL();
				break;
			case PAD_D:
				work->menu_cursor++;
				if(work->menu_cursor>=N_MENU_ITEMS) work->menu_cursor=0;

				SE_SEL();
				break;
			}

			cur1=work->menu_cursor;

			if(cur0!=cur1){
				KeyAnimSetX(work,0,MOVE_CURSOR_COUNT,CODE_CURSOR_LOCATE,
							main_menu_list[cur0].cur_pos,main_menu_list[cur1].cur_pos);
				KeyAnimSetX(work,1,MOVE_CURSOR_COUNT,main_menu_list[cur0].item,
							main_menu_list[cur0].sel,main_menu_list[cur0].unsel);
				KeyAnimSetX(work,2,MOVE_CURSOR_COUNT,main_menu_list[cur1].item,
							main_menu_list[cur1].unsel,main_menu_list[cur1].sel);
			}
		}
		break;

	case 0x10:
		if(work->busy_flag) break;
		return work->ans;
	}
	return 0;
}

static int difficultyStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;
		work->action_strcode=CODE_OPEN_DIFFICULTY;
		work->sound_flag=0;
		work->sub_step++;

		KeyAnimSet0(work,CODE_CURSOR_LOCATE,difficulty_menu_list[work->difficulty_cursor].cur_pos);

		SE_EXPANDLINE();
		break;
	case 1:
		if(work->busy_flag) break;
		work->sub_step++;

		KeyAnimSet0(work,difficulty_menu_list[work->difficulty_cursor].item,
					difficulty_menu_list[work->difficulty_cursor].sel);
	case 2:
		if(work->key_anim[0].count>0) break;

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_CLOSE_DIFFICULTY;
			work->sound_flag=1;

			work->sub_step=0x10;
			work->ans=-1;

			SE_CANCEL();
			return 0;
		}
		else if(work->key_press & PAD_OK){
			work->action_strcode=CODE_CLOSE_DIFFICULTY;
			work->sound_flag=1;

			work->sub_step=0x10;

			SE_OK();
			work->ans=1;

			return 0;
		}

		{
			int cur0,cur1;

			cur0=work->difficulty_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->difficulty_cursor--;
				if(work->difficulty_cursor<0) work->difficulty_cursor=N_DIFFICULTY_ITEMS-1;

				SE_SEL();
				break;
			case PAD_D:
				work->difficulty_cursor++;
				if(work->difficulty_cursor>=N_DIFFICULTY_ITEMS) work->difficulty_cursor=0;

				SE_SEL();
				break;
			}

			cur1=work->difficulty_cursor;

			if(cur0!=cur1){
				KeyAnimSetX(work,0,MOVE_CURSOR_COUNT,CODE_CURSOR_LOCATE,
							difficulty_menu_list[cur0].cur_pos,difficulty_menu_list[cur1].cur_pos);
				KeyAnimSetX(work,1,MOVE_CURSOR_COUNT,difficulty_menu_list[cur0].item,
							difficulty_menu_list[cur0].sel,difficulty_menu_list[cur0].unsel);
				KeyAnimSetX(work,2,MOVE_CURSOR_COUNT,difficulty_menu_list[cur1].item,
							difficulty_menu_list[cur1].unsel,difficulty_menu_list[cur1].sel);
			}
		}
		break;

	case 0x10:
		if(work->busy_flag) break;
		return work->ans;
	}
	return 0;
}

static int optionStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(work->busy_flag) break;
		work->action_strcode=CODE_OPEN_BOSSOPT;
		work->sound_flag=0;
		work->sub_step++;
		work->option_cursor=0;

		work->explain_disp_flag=0;

#if 1
		KeyAnimSet0(work,CODE_CURSOR_LOCATE,option_menu_list[work->option_cursor].cur_pos);
#else
		YPosAnimSet0(work,work->option_cursor);
#endif

		{
			int i;
			for(i=0;i<N_OPTION_ITEMS;i++){
				if(option_menu_list[i].menu_sw==-1) continue;

#if 1
				KeyAnimSet0(work,option_menu_list[i].menu_sw,
							option_menu_list[i].sw[work->option_menu_cursor[i]]);
#else
				ChangeVValue(work,option_menu_list[i].menu_sw,
							 option_sw_v[i][work->option_menu_cursor[i]],
							 work->base_v[i],work->base_pv[i]);
#endif
			}
		}

		SE_EXPANDLINE();
		break;
	case 1:
		CopyKeyAlpha(work);

		if(work->busy_flag) break;
		work->sub_step++;

#if 1
		KeyAnimSet0(work,option_menu_list[work->option_cursor].menu_item,
					option_menu_list[work->option_cursor].sel[ITEM_SEL_INDEX]);
#else
		AlphaAnimSet0(work,option_menu_list[work->option_cursor].menu_item,SEL_ALPHA);
#endif

	case 2:
		AlphaAnim(work);
		CopyKeyAlpha(work);

		PrintOptionExplain(work);

#if 1
		if(work->key_anim[0].count>0) break;
#else
		if(work->y_pos_count>0) break;
#endif

		CreateOptionExplain(work);

		if(work->key_press & PAD_CANCEL){
			work->action_strcode=CODE_CLOSE_BOSSOPT;
			work->sound_flag=1;

			work->sub_step=0x10;
			work->ans=-1;

			work->explain_disp_flag=0;

			SE_CANCEL();
			return 0;
		}
		else if(work->key_press & (PAD_OK|PAD_L|PAD_R)){
			const float *sw_v=option_sw_v[work->option_cursor];

			switch(work->option_cursor){
			case OPTION_ITEM_RAD:
#if 1
				{
					int val0,val1;

					val0=work->option_menu_cursor[work->option_cursor];

					if(work->key_press & PAD_L){
						work->option_menu_cursor[work->option_cursor]--;

						if(work->option_menu_cursor[work->option_cursor]<0){
							work->option_menu_cursor[work->option_cursor]=2;
							val0=3; val1=2;
						}
						else{
							val1=work->option_menu_cursor[work->option_cursor];
						}
					}
					else{
						work->option_menu_cursor[work->option_cursor]++;
						val1=work->option_menu_cursor[work->option_cursor];

						if(work->option_menu_cursor[work->option_cursor]>=3){
							work->option_menu_cursor[work->option_cursor]=0;
							val1=3;
						}
						else{
							val1=work->option_menu_cursor[work->option_cursor];
						}
					}

#ifdef DEBUG_MODE
					printf("Rader : %d -> %d\n",val0,val1);
#endif

					KeyAnimSetX(work,0,SW_ANIM_COUNT,option_menu_list[work->option_cursor].menu_sw,
								option_menu_list[work->option_cursor].sw[val0],
								option_menu_list[work->option_cursor].sw[val1]);
				}
#else
				if(work->key_press & PAD_L){
					work->option_menu_cursor[OPTION_ITEM_RAD]--;
					if(work->option_menu_cursor[OPTION_ITEM_RAD]<0){
						work->option_menu_cursor[OPTION_ITEM_RAD]=2;
					}
				}
				else{
					work->option_menu_cursor[OPTION_ITEM_RAD]++;
					if(work->option_menu_cursor[OPTION_ITEM_RAD]>=3){
						work->option_menu_cursor[OPTION_ITEM_RAD]=0;
					}
				}
				ChangeVValue(work,option_menu_list[work->option_cursor].menu_sw,
							 sw_v[work->option_menu_cursor[work->option_cursor]],
							 work->base_v[work->option_cursor],
							 work->base_pv[work->option_cursor]);
#endif

				work->explain_disp_flag=-1;

				// SE_SEL();
				SE_SWITCHOPT();
				return 0;


#if 0
			case OPTION_ITEM_5_1CH:
				if(work->option_menu_cursor[OPTION_ITEM_SND]) break;

#if 1
				{
					int val0,val1;

					val0=work->option_menu_cursor[work->option_cursor];
					work->option_menu_cursor[work->option_cursor]^=1;
					val1=work->option_menu_cursor[work->option_cursor];

					KeyAnimSetX(work,0,SW_ANIM_COUNT,option_menu_list[work->option_cursor].menu_sw,
								option_menu_list[work->option_cursor].sw[val0],
								option_menu_list[work->option_cursor].sw[val1]);
				}
#else
				work->option_menu_cursor[work->option_cursor]^=1;
				ChangeVValue(work,option_menu_list[work->option_cursor].menu_sw,
							 sw_v[work->option_menu_cursor[work->option_cursor]],
							 work->base_v[work->option_cursor],
							 work->base_pv[work->option_cursor]);
#endif
				// SE_SEL();
				SE_SWITCHOPT();
				return 0;
#endif

#ifndef KP_WINDOWS	// Windows版では無し
			case OPTION_ITEM_CTR:
#if 0 ///
				if( work->key_press & (PAD_OK|PAD_R|PAD_L) ){
					work->action_strcode = CODE_NEXTTOCOLADJ;
					SE_OK();				   
				} else {
					break;
				}
				
				return 2;/* キーコンフィグへ */
#else
				if( work->key_press & (PAD_OK|PAD_R|PAD_L) ){
					int val0,val1;
					int cur_pos = work->option_cursor;
					
					val0 = work->option_menu_cursor[ cur_pos ];
					
					if( work->key_press & PAD_L ){
						work->option_menu_cursor[ cur_pos ] --;
						if( work->option_menu_cursor[ cur_pos ] < 0 ){
							work->option_menu_cursor[ cur_pos ] = 3;/* TYPEA~TYPEDまで計4個 */
						}
					} else {
						work->option_menu_cursor[ cur_pos ] ++;
						if( work->option_menu_cursor[ cur_pos ] > 3 ){
							work->option_menu_cursor[ cur_pos ] = 0;
						}
					}
					
					val1 = work->option_menu_cursor[ cur_pos ];
					
					printf("CTR:::va10[%d:%x],va11[%d:%x]\n",
						   val0, option_menu_list[cur_pos].sw[val0], 
						   val1, option_menu_list[cur_pos].sw[val1]);
					KeyAnimSetX(work,0,SW_ANIM_COUNT,option_menu_list[cur_pos].menu_sw,
								option_menu_list[cur_pos].sw[val0],
								option_menu_list[cur_pos].sw[val1]);
					
					// SE_SEL();
					SE_SWITCHOPT();
				}
				return 0;
#endif

			case OPTION_ITEM_QUICK:
				work->explain_disp_flag=-1;

			case OPTION_ITEM_VIB:
			case OPTION_ITEM_BLOOD:
			// case OPTION_ITEM_CAP:
			case OPTION_ITEM_REV:
			case OPTION_ITEM_ITEMMENU:
#if 1
#endif	// KP_WINDOWS
				{
					int val0,val1;

					val0=work->option_menu_cursor[work->option_cursor];
					work->option_menu_cursor[work->option_cursor]^=1;
					val1=work->option_menu_cursor[work->option_cursor];
					printf("[%d] %d->%d\n",work->option_cursor, val0, val1);

					KeyAnimSetX(work,0,SW_ANIM_COUNT,option_menu_list[work->option_cursor].menu_sw,
								option_menu_list[work->option_cursor].sw[val0],
								option_menu_list[work->option_cursor].sw[val1]);
				}
#else
				work->option_menu_cursor[work->option_cursor]^=1;
				ChangeVValue(work,option_menu_list[work->option_cursor].menu_sw,
							 sw_v[work->option_menu_cursor[work->option_cursor]],
							 work->base_v[work->option_cursor],
							 work->base_pv[work->option_cursor]);
#endif
				// SE_SEL();
				SE_SWITCHOPT();
				return 0;

#if 0

			case OPTION_ITEM_SND:
#if 1
				{
					int val0,val1;

					val0=work->option_menu_cursor[work->option_cursor];
					work->option_menu_cursor[work->option_cursor]^=1;
					val1=work->option_menu_cursor[work->option_cursor];

					KeyAnimSetX(work,0,SW_ANIM_COUNT,option_menu_list[work->option_cursor].menu_sw,
								option_menu_list[work->option_cursor].sw[val0],
								option_menu_list[work->option_cursor].sw[val1]);

					if(val1){
						/* モノラル */
						sd_set_cli(SD_MONORAL);
					}
					else{
						/* ステレオ */
						sd_set_cli(SD_STEREO);
					}

#if 0
					if(work->option_menu_cursor[work->option_cursor]){
						/* モノラルならば、5.1CHをOFF */
						val0=work->option_menu_cursor[OPTION_ITEM_5_1CH];
						work->option_menu_cursor[OPTION_ITEM_5_1CH]=0;
						val1=work->option_menu_cursor[OPTION_ITEM_5_1CH];

						KeyAnimSetX(work,1,SW_ANIM_COUNT,option_menu_list[OPTION_ITEM_5_1CH].menu_sw,
									option_menu_list[OPTION_ITEM_5_1CH].sw[val0],
									option_menu_list[OPTION_ITEM_5_1CH].sw[val1]);
					}
#endif

				}
#else
				work->option_menu_cursor[work->option_cursor]^=1;
				ChangeVValue(work,option_menu_list[work->option_cursor].menu_sw,
							 sw_v[work->option_menu_cursor[work->option_cursor]],
							 work->base_v[work->option_cursor],
							 work->base_pv[work->option_cursor]);

				if(work->option_menu_cursor[work->option_cursor]){
					/* モノラル */
					sd_set_cli(SD_MONORAL);
				}
				else{
					/* ステレオ */
					sd_set_cli(SD_STEREO);
				}

				if(work->option_menu_cursor[work->option_cursor]){
					/* モノラルならば、5.1CHをOFF */
					work->option_menu_cursor[OPTION_ITEM_5_1CH]=0;
					ChangeVValue(work,option_menu_list[OPTION_ITEM_5_1CH].menu_sw,
								 option_sw_v[OPTION_ITEM_5_1CH]
								 	[work->option_menu_cursor[OPTION_ITEM_5_1CH]],
								 work->base_v[OPTION_ITEM_5_1CH],
								 work->base_pv[OPTION_ITEM_5_1CH]);
				}
#endif
				// SE_SEL();
				SE_SWITCHOPT();
				return 0;
#endif

			case OPTION_ITEM_EXIT:
				if(!(work->key_press & PAD_OK)) break;

				work->action_strcode=CODE_CLOSE_BOSSOPT;
				work->sound_flag=1;

				work->sub_step=0x10;
				work->ans=1;

				work->explain_disp_flag=0;

				SE_CANCEL();
				return 0;
			}
		}

#if 1
		{
			int cur0,cur1;

			cur0=work->option_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->option_cursor--;
#ifdef KP_WINDOWS
				if(work->option_cursor==OPTION_ITEM_CTR) work->option_cursor--;
#endif
				if(work->option_cursor<0) work->option_cursor=N_OPTION_ITEMS-1;

				SE_SEL();
				break;
			case PAD_D:
				work->option_cursor++;
#ifdef KP_WINDOWS
				if(work->option_cursor==OPTION_ITEM_CTR) work->option_cursor++;
#endif
				if(work->option_cursor>=N_OPTION_ITEMS) work->option_cursor=0;

				SE_SEL();
				break;
			}

			cur1=work->option_cursor;

			if(cur0!=cur1){
				KeyAnimSetX(work,0,MOVE_CURSOR_COUNT,CODE_CURSOR_LOCATE,
							option_menu_list[cur0].cur_pos,option_menu_list[cur1].cur_pos);
				work->explain_disp_flag=0;

#if 0
				if(cur0!=OPTION_ITEM_5_1CH ||
				   work->option_menu_cursor[OPTION_ITEM_SND]==0){

					KeyAnimSetX(work,1,MOVE_CURSOR_COUNT,option_menu_list[cur0].menu_item,
								option_menu_list[cur0].sel[ITEM_SEL_INDEX],
								option_menu_list[cur0].sel[ITEM_UNSEL_INDEX]);
				}
				if(cur1!=OPTION_ITEM_5_1CH ||
				   work->option_menu_cursor[OPTION_ITEM_SND]==0){

					KeyAnimSetX(work,2,MOVE_CURSOR_COUNT,option_menu_list[cur1].menu_item,
								option_menu_list[cur1].sel[ITEM_UNSEL_INDEX],
								option_menu_list[cur1].sel[ITEM_SEL_INDEX]);
				}
#else
				KeyAnimSetX(work,1,MOVE_CURSOR_COUNT,option_menu_list[cur0].menu_item,
							option_menu_list[cur0].sel[ITEM_SEL_INDEX],
							option_menu_list[cur0].sel[ITEM_UNSEL_INDEX]);
				KeyAnimSetX(work,2,MOVE_CURSOR_COUNT,option_menu_list[cur1].menu_item,
							option_menu_list[cur1].sel[ITEM_UNSEL_INDEX],
							option_menu_list[cur1].sel[ITEM_SEL_INDEX]);
#endif

			}
		}
#else
		{
			int cur0,cur1;

			cur0=work->option_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			case PAD_U:
				work->option_cursor--;
				if(work->option_cursor<0) work->option_cursor=N_OPTION_ITEMS-1;

				SE_SEL();
				break;
			case PAD_D:
				work->option_cursor++;
				if(work->option_cursor>=N_OPTION_ITEMS) work->option_cursor=0;

				SE_SEL();
				break;
			}

			cur1=work->option_cursor;

			if(cur0!=cur1){
				YPosAnimSet(work,cur1,MOVE_CURSOR_COUNT);

#if 0
				if(cur0!=OPTION_ITEM_5_1CH ||
				   work->option_menu_cursor[OPTION_ITEM_SND]==0){

					AlphaAnimSet(work,0,option_menu_list[cur0].menu_item,
								 UNSEL_ALPHA,MOVE_CURSOR_COUNT);
				}
				if(cur1!=OPTION_ITEM_5_1CH ||
				   work->option_menu_cursor[OPTION_ITEM_SND]==0){

					AlphaAnimSet(work,1,option_menu_list[cur1].menu_item,
								 SEL_ALPHA,MOVE_CURSOR_COUNT);
				}
#else
				AlphaAnimSet(work,0,option_menu_list[cur0].menu_item,
							 UNSEL_ALPHA,MOVE_CURSOR_COUNT);
				AlphaAnimSet(work,1,option_menu_list[cur1].menu_item,
							 SEL_ALPHA,MOVE_CURSOR_COUNT);
#endif

			}
		}
#endif

		break;

	case 0x10:
		CopyKeyAlpha(work);

		if(work->busy_flag) break;
		return work->ans;
	}
	return 0;
}


static void InitMenu(Work *work);

static void Step(Work *work)
{
	switch(work->step){
	case START_STEP:
		InitMenu(work);
		work->step++;
		work->sub_step=0;
		break;
	case MAIN_MENU:
		switch(mainStep(work)){
		case 1:
			work->step=LEVEL_MENU;
			work->sub_step=0;
			break;
		case 2:
			work->step=OPTION_MENU;
			work->sub_step=0;
			break;
		case -1:
			work->step=EXIT_STEP;
			work->sub_step=0;
			work->ans=-1;
			break;
		}
		break;
	case LEVEL_MENU:
		switch(difficultyStep(work)){
		case 1:
			work->step=EXIT_STEP;
			work->sub_step=0;
			work->ans=work->menu_cursor;
			break;
		case -1:
			work->step=MAIN_MENU;
			work->sub_step=0;
			break;
		}
		break;
	case OPTION_MENU:
		switch(optionStep(work)){
		case 1:
		case -1:
			work->step=MAIN_MENU;
			work->sub_step=0;
			break;
		  case 2:
			/* キーコンフィグが呼ばれた */
			work->step=CONTROLS_SELECT;
			work->sub_step=0;
			break;
		}
		break;
	  case CONTROLS_SELECT:
		if( work->sub_step == 0 ){
			extern void *NewKeyConSel(
									  int page_num,  /* ページ数 */
									  int init_page, /* 初期ページ番号 1～ */
									  int *ret);     /* 結果 -1: キャンセル
														0: 起動中
														1: １ページ目を選択
														2: ２ページ目を選択
														3: ３ページ目を選択
														4: ４ページ目を選択 */
			int val;
			val = work->option_menu_cursor[OPTION_ITEM_CTR] + 1;
			NewKeyConSel( 4, val, &work->con_flag );
			printf("con_flag [%p]\n",&work->con_flag );
			//BgscrFadeout(work);
			work->action_strcode = CODE_CLOSE_BOSSOPT;
			work->sub_step = 1;
		} else {
			SPR_OBJ *root;
			root = L2D_GetObject( work->l2d_handle, GV_StrCode("ROOT") );
			SPR_HIDE( root );
		}
		switch( work->con_flag ){
		  case -1:
			/* キャンセル */
			printf("a\n");
			work->step=OPTION_MENU;
			work->sub_step=0;
			//BgscrFadein(work);
			break;
		  case 0:
			/* キーコントロール起動中 */
			//printf("b\n");
			break;

		  case 1:
		  case 2:
		  case 3:
		  case 4:
			{ 
				/* 選択された */
				int val1 = work->option_menu_cursor[OPTION_ITEM_CTR];
				//SPR_OBJ *root;
				//root = L2D_GetObject( work->l2d_handle, GV_StrCode("ROOT") );
				//SPR_SHOW( root );
				work->option_menu_cursor[OPTION_ITEM_CTR] = work->con_flag - 1;
				work->step=OPTION_MENU;
				work->sub_step=0;
				//work->action_strcode = CODE_OPEN_BOSSOPT;
				//BgscrFadein(work);
				{
					/* 値を変更 */
					void *part;
					int val = work->option_menu_cursor[OPTION_ITEM_CTR];
					
					printf("CONTROLS TYPE = %d\n",val);
					part = L2D_GetParts( work->l2d_handle, option_menu_list[OPTION_ITEM_CTR].menu_sw );
					if( part == NULL ) break;
					L2D_MorfObject( part, option_menu_list[OPTION_ITEM_CTR].sw[val], option_menu_list[OPTION_ITEM_CTR].sw[val], 1.0f );
				}
			}
			break;
		}
		break;
		
	case EXIT_STEP:
		if(work->busy_flag) break;

		GV_DestroyActor(work);
		break;
	}
}

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
	SetConfigValue(work);

	CallLocalProc(work);

	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}

	if(work->strman!=NULL){
		GV_DestroyOtherActor(work->strman);
		work->strman=NULL;
	}
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	int i;

	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->proc=0;
	work->start_mode=0;

	work->menu_cursor=0;
	work->option_cursor=0;
	work->difficulty_cursor=DIFFICULTY_ITEM_NORMAL;

	for(i=0;i<N_OPTION_ITEMS;i++){
		work->option_menu_cursor[i]=0;
	}

	GetConfigValue(work);

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
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

	work->y_pos_count=0;

	for(i=0;i<2;i++){
		work->alphaanim[i].count=0;
	}

	for(i=0;i<N_OPTION_ITEMS;i++){
		int strcode=option_menu_list[i].menu_sw;
		SPR_OBJ *spr;

		if(strcode==-1) continue;

		spr=L2D_GetObject(work->l2d_handle,strcode);
		if(spr==NULL) continue;

		work->base_v[i]=spr->sprite.head.tex.v;
		work->base_pv[i]=spr->sprite.head.tex.pv;
	}
	
	work->con_flag = -1;/* 監視変数の初期化 */
}

/* 初期化部メイン */
void *NewBossrushScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->busy_flag=0;
		work->action_strcode=0;

		work->ans=0;
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

		// InitMenu(work);

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}


/* XBOXでは GM_Configuration2 も同時に操作する */
extern short Stock_GM_Configuration;
extern int 	 Stock_GM_Configuration2;

int ComStoreOptions(void)
{
	short ans=Stock_GM_Configuration;
	Stock_GM_Configuration=GM_Configuration;
	Stock_GM_Configuration2=GM_Configuration2;
	return ans;
}

int ComLoadOptions(void)
{
	short ans=GM_Configuration;
	GM_Configuration=Stock_GM_Configuration;
	GM_Configuration2=Stock_GM_Configuration2;
	return ans;
}
