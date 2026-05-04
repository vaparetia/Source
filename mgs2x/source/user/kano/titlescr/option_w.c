/*
	option_w.c
		サブスタンスWINDOWS専用オプション画面

	2002/07/03 Y.YANO 
	2002/11/26 Takaki Eiji 
	$Id: option_w.c,v 1.4 2003/01/13 10:07:12 takaki Exp $
*/

/*
   XBOXでは、オプション画面がPS2と全く違うものになる

   VIB:ON/OFF
   RAD:TYPE1/TYPE2/OFF
   BLD:ON/OFF
  *SE :8～16段階
  *MSC:8～16段階
   CAP:JAPANESE/ENGLISH/FRENCH/.../OFF
  *CTR 
   OWN:NORMAL/REVERSE
   ITM:GROUP/LINER
   QCK:UNEQUIP/PREVIOUS
   COL
   EXIT

   /////TODO
   ・項目の順番、追加、削除が楽なように作る。


static	inline	void	SD_SET( code )
int			code ;
Xbox用に、Music/SEの音量設定サウンドコードを追加しました。

・Music設定

0xF7000000 (min.) ～ 0xF700000F(max.)

・SE設定

0xF7000010 (min.) ～ 0xF700001F(max.)

min.設定では、ミュート状態になります。
デフォルトはmax.です。

SD_S_R_SEL01 //無線短縮ウィンドウ出す

…を、Xbox版音量ゲージ大小カーソル音に使用してみて下さい。



１）ＳＥ：音響効果の音量を設定します
SE: Adjust sound effect volume.

２）ＢＧＭ：音楽の音量を設定します
BGM: Adjust music volume.

３）ＣＯＮＴＲＯＬＳ：基本操作を設定します
CONTROLS: Set basic controls.

void *NewKeyConSel(int page_num, int *ret);

page_num ･･･ ページ数。4 を入れてください。
ret      ･･･ 結果を受け取る変数のアドレスを入れてください。
             このキャラを起動後はずっとその変数を監視してください。
             値が 0 の間は起動中なのでお待ちください。
             値が 0 以外になったら、終了したということなので、そちらで続きの処理を行ってください。
             結果 -1: キャンセル
                   1: １ページ目を選択
                   2: ２ページ目を選択
                   3: ３ページ目を選択
                   4: ４ページ目を選択
             なお、このキャラは自動的に破棄されます。
ページ数は 4 でお願いします。


データは
l2d  2D/option_controls/option_controls.l2d


*/
extern void *NewKeyConSel(
  int page_num,  /* ページ数 */
  int init_page, /* 初期ページ番号 1～ */
  int *ret);     /* 結果 -1: キャンセル
                          0: 起動中
                          1: １ページ目を選択
                          2: ２ページ目を選択
                          3: ３ページ目を選択
                          4: ４ページ目を選択 */


#include "titlescr_x.h"

#include "bgscr.h"

#include "subtitle.h"


#include "../../mode/codec/codec_signal.h"
#include "../../mode/codec/codecmem.h"
#include "../../mode/codec/cdc_load.h"

/* L2D define群 */
#include "node_menu_x.h"

#include "font.h"


#define N_MENU_ITEMS		12
#define N_VAL_ITEMS			9

//#define CODE_L2D_SCRN		GV_StrCode("scrn")
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

#define VOL_MAX 		16
#define VOL_KEY_COUNT 	DIRECT_TICK(6)
#define VOL_SCRL_BAR_UNIT 17.0f
#define VOL_NUM_TEX_U	18.0f


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

	/* SE, MUSIC */
	int key_vol_count;
	int se_last_val;
	int msc_last_val;
	int se_scrl_count;
	int msc_scrl_count;

	/* CONTROLS */
	int con_flag;	/* 山下さんのプログラムに渡す、監視変数(-1で初期化が必要) */
	int con_cnt;	/* 山下さんのプログラムを呼び出すタイミングをみるカウンタ */

	int *node_flag; /* ノードから呼び出されたときはこのフラグがON */
	int node_flag2; /* ノードから呼び出されたときはこのフラグがON */
	void *node_work;
} Work;


enum {/* 下のconfig_flagsと項目が同期している必要がある */
	ITEM_VIB=0,
	ITEM_RAD,
	ITEM_BLD,
	/* XBOXで追加項目 */
	ITEM_SE,
	ITEM_MSC,

	ITEM_CAP,
	/* XBOXで追加項目 */
	ITEM_CTR,

	ITEM_OWN,
	ITEM_ITEM,
	ITEM_QUICK,
	ITEM_COL,

	ITEM_EXIT,
	ITEM_MAX,
};
static const int config_flags[ITEM_MAX]={
	GM_CONFIG_VIBRATION_OFF,
	GM_CONFIG_RADAR_OFF,
	GM_CONFIG_BLOOD_OFF,
	0,/* se */
	0,/* music */
	GM_CONFIG_CAPTION_OFF,
	0,/* controls */
	GM_CONFIG_SHUKAN_REVERSE,
	GM_CONFIG_OLD_TYPE_MENU,
	GM_CONFIG_MENU_QCHANGE_EX,
	0,/* col */

	0,/* exit */
};

enum {
	ITEM_SEL_INDEX=0,
	ITEM_UNSEL_INDEX,
};


/* ノードから呼び出された時の項目 */
#define NODE_MENU_MAX 11
const static node_menu[NODE_MENU_MAX] = {
	ITEM_VIB,
	ITEM_RAD,
	ITEM_BLD,
	ITEM_SE,
	ITEM_MSC,
	ITEM_CAP,
	ITEM_CTR,//追加
	ITEM_OWN,
	ITEM_ITEM,
	ITEM_QUICK,
	ITEM_EXIT,
};


static const int vibstate_name[]={ SW_vibON,SW_vibOFF, };

static const int radstate_name[]=
	{ SW_radOFF1,SW_radTYPE1,SW_radTYPE2,SW_radOFF2, };

static const int bldstate_name[]={ SW_bldON,SW_bldOFF, };

#define CAPLANG_MAX	 	(LANG_MAX_NUM + 1)
static int capstate_name[CAPLANG_MAX];

static const int ownstate_name[]={ SW_ownNORM,SW_ownREV, };

static const int itemstate_name[]={ SW_itemGROUP,SW_itemLINEAR, };

static const int quickstate_name[]={ SW_quickUNEQUIP,SW_quickPREV, };

static const int controls_name[]={ SW_ctrlTYPEA, SW_ctrlTYPEB, SW_ctrlTYPEC, SW_ctrlTYPED, };

#define cur0_SE 4792406
#define optSE	2742956
#define colonSE 16342089
#define selSE	6568620
#define unSE	3949221

#define curO_MSC 2356428
#define optMSC	3882888
#define colonMSC 13521424
#define selMSC	8863631
#define unMSC	8928938

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

	/* SE */
	{ cur0_SE,
	  optSE, colonSE,  -1, 
	  { selSE, unSE, },
	  NULL,
	},

	/* MUSIC */
	{ curO_MSC,
	  optMSC, colonMSC,  -1,
	  { selMSC, unMSC, },
	  NULL,
	},

	/* 字幕 ON/OFF */
	{ POS_curCAP,
	  OBJ_optCAP,CODE_MENUCAP_COL,OBJ_swCAP,
	  { KEY_selCAP,   KEY_unCAP, },
	  capstate_name,
	},

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
	RES_COLOR_EXPLAIN=ITEM_MAX,
};


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

#define OPTION_EXPLAIN_X		SUBTITLE_X
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


#define TEXTCOLOR				SUBTITLE_TEXTCOLOR



/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void CallLocalProc(Work *work)
{
	if( work->node_flag != NULL ){
		if( *work->node_flag == 1 ){
			/* ノード呼び出しの時はプロックを呼ばない */
			//*work->node_flag = -1;
			return ;
		}
	}

	if( work->proc != 0 && work->proc != 1 ){
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

	/* ノード呼び出しの時の処理? */
	if(work->ret_signal_flag &&
	   work->ans == TITLE_MSG_MODEEND){
		
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




static void Key(Work *work)
{
	int status=GV_PadDataDirect[0].status & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);
	int press=GV_PadDataDirect[0].press & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL);

	work->key_press = press;

	if( status && (status == work->key_status) ){
		if( work->key_count > REPEAT_FIRST ){
			work->key_count -= work->key_repeat_time;
			work->key_autostatus = work->key_status;
		} else {
			work->key_autostatus = 0;
		}
		work->key_count++;
	} else {
		work->key_autostatus = work->key_status = status;
		work->key_count = 0;
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

	for( i = 0; i < ANIM_WORK_SIZE; i++ ){
		if( work->key_anim[i].count > 0 ){
			void *part;

			part = L2D_GetParts( work->l2d_handle, work->key_anim[i].part_code );
			if( part == NULL ) return;

			work->key_anim[i].p += (1.0f - work->key_anim[i].p)/(float)work->key_anim[i].count;

			L2D_MorfObject( part, work->key_anim[i].key0_code, work->key_anim[i].key1_code,
						    work->key_anim[i].p );

			work->key_anim[i].count--;
		}
	}
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


#if defined(JAPANESE) || defined(PAL)

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

#endif


static void GetConfigValue(Work *work)
{
	int i;
	for( i = 0; i < ITEM_MAX; i++ ){
		if( config_flags[i] == 0 ) continue;/* GM_configuration不使用 or GM_configuration2使用 */
		if( i == ITEM_RAD ) continue;/* 別処理 */
		if( i == ITEM_CAP ) continue;/* 別処理 */
		if( i == ITEM_SE  ) continue;/* 別処理 */
		if( i == ITEM_MSC ) continue;/* 別処理 */
		if( i == ITEM_CTR ) continue;/* 別処理 */
		work->menu_value_cursor[i] = ( (GM_Configuration & config_flags[i]) != 0 );
	}

	/* レーダーだけは例外 */
	switch( GM_Configuration & (GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE) ){
	  case 0:
		/* TYPE 1 */
		work->menu_value_cursor[ITEM_RAD] = 1;
		break;
	  case GM_CONFIG_RADAR_OFF_INTRUDE:
		/* TYPE 2 */
		work->menu_value_cursor[ITEM_RAD] = 2;
		break;
	  case GM_CONFIG_RADAR_OFF:
	  case GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE:
		/* OFF */
		work->menu_value_cursor[ITEM_RAD] = 0;
		break;
	}
	
	/* 字幕 CAPTION */
	if( GM_Configuration & config_flags[ITEM_CAP] ){
		work->menu_value_cursor[ITEM_CAP] = 0;/* OFF */
	} else {
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

	/* SE,MUSIC の設定 */
	{
		unsigned int val;
		val = GM_Configuration2 & GM_CONFIG_SE_VOLUME;
		printf("se [%d]\n", val);
		val = (VOL_MAX-1) - val;/* 0:MAX--15:MINにするため */
		work->menu_value_cursor[ITEM_SE] = val;		
		printf("se2[%d]\n", val);
	} 
	{
		unsigned int val;
		val = GM_Configuration2 & GM_CONFIG_MUSIC_VOLUME;
		val = ( val >> 4 );
		printf("bgm  [%d]\n", val);
		val = (VOL_MAX-1) - val;/* 0:MAX--15:MINにするため */
		work->menu_value_cursor[ITEM_MSC] = val;		
		printf("bgm2 [%d]\n", val);
	} 
	/* キーコンフィグ CONTROLS の設定 */
	{
		unsigned int val;
		val = GM_Configuration2 & GM_CONFIG_CONTROLS;
		val = ( val >> 8 );
		work->menu_value_cursor[ITEM_CTR] = val;
		work->con_flag = -1;/* 監視変数の初期化 */
	} 


#ifdef DEBUG_MODE
	printf("GM_Configuration = 0x%08x\n",GM_Configuration);
	printf("GM_Configuration2 = 0x%08x\n",GM_Configuration2);
#endif

}

static void SetConfigValue(Work *work)
{
	int vand=0,vor=0;
	int i;
	int gm_configuration  = GM_Configuration;
	int gm_configuration2 = GM_Configuration2;

	for( i = 0; i < ITEM_MAX; i++ ){
		if( config_flags[i] == 0 ) continue;/* GM_Configuration不使用 */
		if( i == ITEM_RAD ) continue;/* 別処理 */
		if( i == ITEM_CAP ) continue;/* 別処理 */
		if( i == ITEM_SE  ) continue;/* 別処理 */
		if( i == ITEM_MSC ) continue;/* 別処理 */
		if( i == ITEM_CTR ) continue;/* 別処理 */
		if( work->menu_value_cursor[i] ){
			vor  |= config_flags[i];
		} else {
			vand |= config_flags[i];
		}
	}

	/* レーダーだけは例外 */
	switch( work->menu_value_cursor[ITEM_RAD] ){
	  case 1:
		/* TYPE 1 */
		vand|= GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE;
		break;
	  case 2:
		/* TYPE 2 */
		vand|= GM_CONFIG_RADAR_OFF;
		vor |= GM_CONFIG_RADAR_OFF_INTRUDE;
		break;
	  case 0:
	  case 3:
		/* OFF */
		vor |= GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE;
		break;
	}

	/* 字幕 CAPTION */
	if( work->menu_value_cursor[ITEM_CAP] == 0 ){
		/* OFF */
		vor  |= config_flags[ITEM_CAP];
		GM_Language = work->lang_default;
		printf("lang[%d]\n", GM_Language );
	} else {
		vand |= config_flags[ITEM_CAP];
		GM_Language = work->lang_ref[ work->menu_value_cursor[ITEM_CAP] ];
		printf("lang[%d]\n", GM_Language );
	}

	vand =~ vand;
	GM_Configuration &= vand;
	GM_Configuration |= vor;

#ifdef DEBUG_MODE
	printf("GM_Configuration  = 0x%08x\n",GM_Configuration);
	printf("GM_Configuration2 = 0x%08x\n",GM_Configuration2);
#endif

	/* SE  */
	{
		int val;
		val = work->menu_value_cursor[ITEM_SE];
		val = (VOL_MAX-1) - val;/* 0:MAX--15:MIN にするため */
		GM_Configuration2 &= ~GM_CONFIG_SE_VOLUME;
		GM_Configuration2 |= ( val & GM_CONFIG_SE_VOLUME );
	} 
	/* MUSIC */
	{
		int val;
		val = work->menu_value_cursor[ITEM_MSC];
		val = (VOL_MAX-1) - val;/* 0:MAX--15:MIN にするため */
		GM_Configuration2 &= ~GM_CONFIG_MUSIC_VOLUME;
		GM_Configuration2 |= ( (val<<4) & GM_CONFIG_MUSIC_VOLUME );
	}
	/* キーコンフィグ CONTROLS */
	{
		int val;
		val = work->menu_value_cursor[ITEM_CTR];
		GM_Configuration2 &= ~GM_CONFIG_CONTROLS;
		GM_Configuration2 |= ( (val<<8) & GM_CONFIG_CONTROLS );
	}
	

	if( gm_configuration  != GM_Configuration ){
		printf("option changed\n");
		GM_TitleMenuStatus |= TITLE_MENU_OPTION_CHANGED;
	}
	if( gm_configuration2 != GM_Configuration2 ){
		printf("option changed2\n");
		GM_TitleMenuStatus |= TITLE_MENU_OPTION_CHANGED;
	}
	if( work->first_val != work->menu_value_cursor[ITEM_CAP] ){
		/* 字幕設定でOFFにしただけでなく、言語を変えてもオプションを変更したとするため */
		printf("option changed\n");
		GM_TitleMenuStatus |= TITLE_MENU_OPTION_CHANGED;
	}

	if( work->node_flag2 ){
		// ノードから呼び出し
		GCL_SaveLinkVar( &GM_Configuration, sizeof( GM_Configuration ) );
		GCL_SaveLinkVar( &GM_Configuration2, sizeof( GM_Configuration2 ) );
	}
}


static void SubStepCursorInit(Work *work)
{
	void *part;

	/* カーソル表示 */
	part = L2D_GetParts( work->l2d_handle, CUR_curs );
	if( part == NULL ) return;

	L2D_MorfObject( part,
				    menu_list[work->menu_cursor].cur_pos,
				    menu_list[work->menu_cursor].cur_pos, 1.0f );

	/* 項目表示 */
	part = L2D_GetParts( work->l2d_handle, menu_list[work->menu_cursor].menu_item );
	if( part == NULL ) return;

	L2D_MorfObject( part,
				    menu_list[work->menu_cursor].sel[ITEM_SEL_INDEX],
				    menu_list[work->menu_cursor].sel[ITEM_SEL_INDEX], 1.0f);
}

static void SubStepValueInit(Work *work)
{
	/* 値表示 */
	int i;

#ifndef _WINDOWS
	for( i = 0; i <= ITEM_MAX; i++ ){
#else
	for( i = 0; i < ITEM_MAX; i++ ){
#endif
		void *part;
		int val;

		val = work->menu_value_cursor[i];

		part = L2D_GetParts( work->l2d_handle, menu_list[i].menu_sw );
		if( part == NULL ) continue;
		L2D_MorfObject( part, menu_list[i].sw[val], menu_list[i].sw[val], 1.0f );
	}

}

static void CopyKeyAlpha(Work *work)
{
	static const int strcode[][2]={
		{ optCTR,colonCTR, },
		{ optCTR,OBJ_swCTR, },
		{ optSE ,colonSE, },
		{ optMSC,colonMSC, },
		{ optMSC,L2D_node_menu_OBJ_bgm_volume, },
		{ optMSC,L2D_node_menu_OBJ_bgm_vol_16, },
		{ optMSC,L2D_node_menu_OBJ_bgm_vol_0, },
		{ optMSC,L2D_node_menu_OBJ_bgm_vol_line, },
		{ optMSC,L2D_node_menu_OBJ_bgm_vol_ber, },
		{ optMSC,L2D_node_menu_OBJ_music_num_1, },
		{ optMSC,L2D_node_menu_OBJ_music_num_10, },
		{ optSE,L2D_node_menu_OBJ_se_volume, },
		{ optSE,L2D_node_menu_OBJ_se_vol_16, },
		{ optSE,L2D_node_menu_OBJ_se_vol_0, },
		{ optSE,L2D_node_menu_OBJ_se_vol_line, },
		{ optSE,L2D_node_menu_OBJ_se_vol_ber, },
		{ optSE,L2D_node_menu_OBJ_se_num_1, },
		{ optSE,L2D_node_menu_OBJ_se_num_10, },
		{ OBJ_optVIB,OBJ_swVIB, },
		{ OBJ_optRAD,OBJ_swRAD, },
		{ OBJ_optBLD,OBJ_swBLD, },
		{ OBJ_optCAP,OBJ_swCAP, },
		{ OBJ_optOWN,OBJ_swOWN, },
		{ OBJ_optITEM,OBJ_swITEM, },
		{ OBJ_optQUICK,OBJ_swQUICK, },

		{ OBJ_optVIB,CODE_MENUVIB_COL, },
		{ OBJ_optRAD,CODE_MENURAD_COL, },
		{ OBJ_optBLD,CODE_MENUBLD_COL, },
		{ OBJ_optCAP,CODE_MENUCAP_COL, },
		{ OBJ_optOWN,CODE_MENUOWN_COL, },
		{ OBJ_optITEM,CODE_MENUITEM_COL, },
		{ OBJ_optQUICK,CODE_MENUQUICK_COL, },
	};
	int i;

	if( work->l2d_handle < 0 ){
		printf("han %d\n",work->l2d_handle);
		//ASSERT( 0 );
		return;
	}

	for( i = 0; i < sizeof(strcode)/sizeof(strcode[0]); i++ ){
		SPR_OBJ *spr0,*spr1;

		spr0 = L2D_GetObject( work->l2d_handle, strcode[i][0] );
		spr1 = L2D_GetObject( work->l2d_handle, strcode[i][1] );
		
		if(spr0==NULL || spr1==NULL) {
			//ASSERT( 0 );
			continue;
		}
		spr1->sprite.col.a = spr0->sprite.col.a;
		spr1->sprite.head.head.flags = spr0->sprite.head.head.flags;
	}
}

static void CreateOptionExplain(Work *work)
{
	if( !work->explain_disp_flag ){
		char *str;
		int i;

		int cur_pos;
		if( work->node_flag2 != 1 ){
			/* タイトルオプション */
			cur_pos = work->menu_cursor;
		} else {
			/* ノード画面 */
			cur_pos = node_menu[ work->menu_cursor ];
		}

		switch( cur_pos ){
		  case ITEM_RAD:
			{
				int index = work->menu_value_cursor[ITEM_RAD]-1;

				if( index < 0 ) index = 3-1;

				str=(char *)GetLocalResource(work->radar_explain_resource[work->lang_ref[work->lang_val] - GM_LANG_ENGLISH],index);
			}
			break;
		  case ITEM_QUICK:
			{
				int index = work->menu_value_cursor[ITEM_QUICK];
				str=(char *)GetLocalResource(work->quick_explain_resource[work->lang_ref[ work->lang_val ] - GM_LANG_ENGLISH],index);
			}
			break;
		  default:
			str=(char *)GetLocalResource(work->option_explain_resource[work->lang_ref[ work->lang_val ] - GM_LANG_ENGLISH ],
										 work->menu_cursor);
			break;
		}

		work->explain_columns = 1;

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

		MENU_ClearTextTexture( work->strman );

		// spaceを無理やり設定する。
		MENU_SetTextSpace( work->strman, DOWN_MARGINE );
		MENU_CreateTextTexture(work->strman,
							   OPTION_EXPLAIN_U,
							   OPTION_EXPLAIN_V,
							   OPTION_EXPLAIN_WIDTH,
							   OPTION_EXPLAIN_HEIGHT,
							   0,0,str);
		// ほかに影響が出ないように規定値に戻す。
		MENU_SetTextSpace( work->strman, 12 );
	}

	work->explain_disp_flag++;
}

static void PrintOptionExplain(Work *work)
{
	if( work->explain_disp_flag > 0 ){
		int y = OPTION_EXPLAIN_Y;
		int h = OPTION_EXPLAIN_H;

		work->explain_disp_flag = 1;

		if( work->explain_columns > 1 ){
			y -= FONT_DISP_HEIGHT/2;
			h=h*5/6;
//			h  = h*2/3;
		}

		MENU_PutTextScreen(work->strman,
						   OPTION_EXPLAIN_X,
						   y,
						   OPTION_EXPLAIN_X+OPTION_EXPLAIN_W,
						   y+h,
						   OPTION_EXPLAIN_U,
						   OPTION_EXPLAIN_V,
						   OPTION_EXPLAIN_U+OPTION_EXPLAIN_WIDTH,
						   OPTION_EXPLAIN_V+OPTION_EXPLAIN_HEIGHT,
						   TEXTCOLOR);
	}
}

static void CreateColorExplain(Work *work)
{
	char *str=(char *)GetLocalResource(work->option_explain_resource[work->lang_ref[ work->lang_val ] - GM_LANG_ENGLISH],RES_COLOR_EXPLAIN);

	MENU_ClearTextTexture(work->strman);

	MENU_CreateTextTexture(work->strman,
						   COLOR_EXPLAIN_U,
						   COLOR_EXPLAIN_V,
						   COLOR_EXPLAIN_WIDTH,
						   COLOR_EXPLAIN_HEIGHT,
						   0,0,str);
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

static int SubStep(Work *work)
{
	extern	int	sd_set_cli( int ) ;

	switch( work->sub_step ){
	  case 0:
		if( !work->busy_flag ){
			if( work->start_flag ){
				work->action_strcode = CODE_BACKOPT;
			} else {
				work->action_strcode = CODE_SHOWOPT;
			}			
			work->sub_step ++;
			work->explain_disp_flag = 0;

			work->start_flag = 1;
			
			// HideCursor(work);			
		}
		break;
	  case 1:
		SubStepValueInit(work);
		work->sub_step++;
	  case 2:
		if( !work->busy_flag ){
			work->sub_step++;
			
			// ShowCursor(work);
			SubStepCursorInit(work);
		}
		break;
	  case 3:
		PrintOptionExplain(work);
		
		if( work->key_anim[0].count > 0 ) break;
		
		CreateOptionExplain(work);
		
		if( work->key_press & PAD_CANCEL ){
			work->action_strcode = CODE_HIDEOPT;
			work->explain_disp_flag = 0;
			
			SE_CANCEL();
						
			return -1;
		} else if( work->key_press & (PAD_OK|PAD_L|PAD_R) || work->key_status & (PAD_L|PAD_R) ){
			int cur_pos;
			if( work->node_flag2 != 1 ){
				/* タイトルオプション */
				cur_pos = work->menu_cursor;
			} else {
				/* ノード画面 */
				cur_pos = node_menu[ work->menu_cursor ];
			}
			switch( cur_pos ){
			  case ITEM_RAD:
				{
					if( work->key_press & (PAD_OK|PAD_L|PAD_R) ){
						int val0,val1;
						
						val0=work->menu_value_cursor[cur_pos];
						if( work->key_press & PAD_L ){
							work->menu_value_cursor[cur_pos]--;
						
							if(work->menu_value_cursor[cur_pos]<0){
								work->menu_value_cursor[cur_pos]=2;
								val0=3; val1=2;
							} else {
								val1=work->menu_value_cursor[cur_pos];
							}
						} else {
							work->menu_value_cursor[cur_pos]++;
							val1=work->menu_value_cursor[cur_pos];
							
							if(work->menu_value_cursor[cur_pos]>=3){
								work->menu_value_cursor[cur_pos]=0;
								val1=3;
							} else {
								val1=work->menu_value_cursor[cur_pos];
							}
						}
						
#ifdef DEBUG_MODE
						printf("Rader : %d -> %d\n",val0,val1);
#endif
						work->explain_disp_flag = -1;
						
						KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[cur_pos].menu_sw,
									menu_list[cur_pos].sw[val0],
									menu_list[cur_pos].sw[val1]);

						// SE_SEL();
						SE_SWITCHOPT();

					} else {
						break;
					}
				}
				return 0;
			case ITEM_QUICK:
				work->explain_disp_flag = -1;

			case ITEM_VIB:
			case ITEM_BLD:
			case ITEM_OWN:
			case ITEM_ITEM:
				{
					if( work->key_press & (PAD_OK|PAD_L|PAD_R) ){
						int val0,val1;
						
						val0=work->menu_value_cursor[cur_pos];
						work->menu_value_cursor[cur_pos]^=1;
						val1=work->menu_value_cursor[cur_pos];
						
						printf("val0[%d]->val1[%d]\n",val0, val1);
						
						KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[cur_pos].menu_sw,
									menu_list[cur_pos].sw[val0],
									menu_list[cur_pos].sw[val1]);

						// SE_SEL();
						SE_SWITCHOPT();
					}
				}

				return 0;
			  case ITEM_CAP:
				{
					if( work->key_press & (PAD_OK|PAD_L|PAD_R) ){
						int val0,val1;
					
						val0=work->menu_value_cursor[cur_pos];
					
						if(work->key_press & PAD_L){
							/*-- 欧州版 --*/
							work->menu_value_cursor[cur_pos] --;
							if( work->menu_value_cursor[cur_pos] < 0 ){
								work->menu_value_cursor[cur_pos] = work->lang_num;
							}
							//work->menu_value_cursor[cur_pos] = work->lang_flag[work->lang_flag_cnt];
						} else {
							/*-- 欧州版 --*/
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
			  case ITEM_COL:
				if(!(work->key_press & PAD_OK)) break;
				
				work->action_strcode = CODE_NEXTTOCOLADJ;
				SE_OK();
							  				
				return 4;
			  case ITEM_SE:
				{
					int val0,val1;
					int code;

					/* 音量UP */
					if( work->key_press & PAD_R ){						
						/* 押されたときは必ず値が変わる */
						val0 = work->menu_value_cursor[ITEM_SE];
						if( val0 >= (VOL_MAX-1) ){
							work->menu_value_cursor[ITEM_SE] = (VOL_MAX-1);
							break;
						}
						work->menu_value_cursor[ITEM_SE] ++;
						val1 = work->menu_value_cursor[ITEM_SE];
						/* キーカウントの初期化 */
						work->key_vol_count = 0;
						/* 前の値を記憶 */
						work->se_last_val = val0;

						GM_SdSet( SD_S_R_SEL01 );
						/* 音量設定 */
						printf("SE vol= %d\n",val1);
						code = val1 | 0xF7000010;
						sd_set_cli( code );
					} else if( work->key_status & PAD_R ){
						if( work->key_vol_count >= VOL_KEY_COUNT ){						
							/* 押し続けられたときにも値が変わる */
							val0 = work->menu_value_cursor[ITEM_SE];
							if( val0 >= (VOL_MAX-1) ){
								work->menu_value_cursor[ITEM_SE] = (VOL_MAX-1);
								break;
							}
							work->menu_value_cursor[ITEM_SE] ++;
							val1 = work->menu_value_cursor[ITEM_SE];
							/* キーカウントの初期化 */
							work->key_vol_count = 0;
							/* 前の値を記憶 */
							work->se_last_val = val0;

							GM_SdSet( SD_S_R_SEL01 );

						} else {
							/* statusの回数を増やしていく */
							work->key_vol_count ++;
							break;
						}
						/* 音量設定 */
						printf("SE vol= %d\n",val1);
						code = val1 | 0xF7000010;
						sd_set_cli( code );
					}
					/* 音量DOWN */
					if( work->key_press & PAD_L ){						
						/* 押されたときは必ず値が変わる */
						val0 = work->menu_value_cursor[ITEM_SE];
						if( val0 <= 0 ){
							work->menu_value_cursor[ITEM_SE] = 0;
							break;
						}
						work->menu_value_cursor[ITEM_SE] --;
						val1 = work->menu_value_cursor[ITEM_SE];
						/* キーカウントの初期化 */
						work->key_vol_count = 0;
						/* 前の値を記憶 */
						work->se_last_val = val0;

						GM_SdSet( SD_S_R_SEL01 );
						/* 音量設定 */
						printf("SE vol= %d\n",val1);
						code = val1 | 0xF7000010;
						sd_set_cli( code );
					} else if( work->key_status & PAD_L ){
						if( work->key_vol_count >= VOL_KEY_COUNT ){						
							/* 押し続けられたときにも値が変わる */
							val0 = work->menu_value_cursor[ITEM_SE];
							if( val0 <= 0 ){
								work->menu_value_cursor[ITEM_SE] = 0;
								break;
							}
							work->menu_value_cursor[ITEM_SE] --;
							val1 = work->menu_value_cursor[ITEM_SE];
							/* キーカウントの初期化 */
							work->key_vol_count = 0;
							/* 前の値を記憶 */
							work->se_last_val = val0;

							GM_SdSet( SD_S_R_SEL01 );

						} else {
							/* statusの回数を増やしていく */
							work->key_vol_count ++;
							break;
						}
						/* 音量設定 */
						printf("SE vol= %d\n",val1);
						code = val1 | 0xF7000010;
						sd_set_cli( code );
					}

				}
				return 0;
				break;
			  case ITEM_MSC:
				{
					int val0,val1;
					int code;

					/* 音量UP */
					if( work->key_press & PAD_R ){						
						/* 押されたときは必ず値が変わる */
						val0 = work->menu_value_cursor[ITEM_MSC];
						if( val0 >= (VOL_MAX-1) ){
							work->menu_value_cursor[ITEM_MSC] = (VOL_MAX-1);
							break;
						}
						work->menu_value_cursor[ITEM_MSC] ++;
						val1 = work->menu_value_cursor[ITEM_MSC];
						/* キーカウントの初期化 */
						work->key_vol_count = 0;
						/* 前の値を記憶 */
						work->msc_last_val = val0;

						/* 音量設定 */
						printf("MUSIC vol= %d\n",val1);
						code = val1 | 0xF7000000;
						sd_set_cli( code );
					} else if( work->key_status & PAD_R ){
						if( work->key_vol_count >= VOL_KEY_COUNT ){						
							printf("ssss\n");
							/* 押し続けられたときにも値が変わる */
							val0 = work->menu_value_cursor[ITEM_MSC];
							if( val0 >= (VOL_MAX-1) ){
								work->menu_value_cursor[ITEM_MSC] = (VOL_MAX-1);
								break;
							}
							work->menu_value_cursor[ITEM_MSC] ++;
							val1 = work->menu_value_cursor[ITEM_MSC];
							/* キーカウントの初期化 */
							work->key_vol_count = 0;
							/* 前の値を記憶 */
							work->msc_last_val = val0;

						} else {
							/* statusの回数を増やしていく */
							work->key_vol_count ++;
							break;
						}
						/* 音量設定 */
						printf("MUSIC vol= %d\n",val1);
						code = val1 | 0xF7000000;
						sd_set_cli( code );
					}
					/* 音量DOWN */
					if( work->key_press & PAD_L ){						
						/* 押されたときは必ず値が変わる */
						val0 = work->menu_value_cursor[ITEM_MSC];
						if( val0 <= 0 ){
							work->menu_value_cursor[ITEM_MSC] = 0;
							break;
						}
						work->menu_value_cursor[ITEM_MSC] --;
						val1 = work->menu_value_cursor[ITEM_MSC];
						/* キーカウントの初期化 */
						work->key_vol_count = 0;
						/* 前の値を記憶 */
						work->msc_last_val = val0;

						/* 音量設定 */
						printf("MUSIC vol= %d\n",val1);
						code = val1 | 0xF7000000;
						sd_set_cli( code );
					} else if( work->key_status & PAD_L ){
						if( work->key_vol_count >= VOL_KEY_COUNT ){						
							/* 押し続けられたときにも値が変わる */
							val0 = work->menu_value_cursor[ITEM_MSC];
							if( val0 <= 0 ){
								work->menu_value_cursor[ITEM_MSC] = 0;
								break;
							}
							work->menu_value_cursor[ITEM_MSC] --;
							val1 = work->menu_value_cursor[ITEM_MSC];
							/* キーカウントの初期化 */
							work->key_vol_count = 0;
							/* 前の値を記憶 */
							work->msc_last_val = val0;

						} else {
							/* statusの回数を増やしていく */
							work->key_vol_count ++;
							break;
						}
						/* 音量設定 */
						printf("MUSIC vol= %d\n",val1);
						code = val1 | 0xF7000000;
						sd_set_cli( code );
					}
				}
				return 0;
				break;
			  case ITEM_CTR:
				if( work->node_flag2 == 1 ){
					/* ノード呼び出し時はパッドの絵なしの山下さんのプログラムを介せず変更する */
					if( work->key_press & (PAD_OK|PAD_R|PAD_L) ){
						int val0,val1;
						
						val0 = work->menu_value_cursor[ cur_pos ];
					
						if( work->key_press & PAD_L ){
							work->menu_value_cursor[ cur_pos ] --;
							if( work->menu_value_cursor[ cur_pos ] < 0 ){
								work->menu_value_cursor[ cur_pos ] = 3;/* TYPEA~TYPEDまで計4個 */
							}
						} else {
							work->menu_value_cursor[ cur_pos ] ++;
							if( work->menu_value_cursor[ cur_pos ] > 3 ){
								work->menu_value_cursor[ cur_pos ] = 0;
							}
						}
					
						val1 = work->menu_value_cursor[ cur_pos ];
					
						printf("CTR:::va10[%d:%x],va11[%d:%x]\n",
							   val0, menu_list[cur_pos].sw[val0], 
							   val1, menu_list[cur_pos].sw[val1]);
						KeyAnimSetX(work,0,SW_ANIM_COUNT,menu_list[cur_pos].menu_sw,
									menu_list[cur_pos].sw[val0],
									menu_list[cur_pos].sw[val1]);
					
						// SE_SEL();
						SE_SWITCHOPT();
					}
					return 0;
				} else {
					/* タイトルからは山下さんのプログラムに移る */
					if( work->key_press & (PAD_OK|PAD_R|PAD_L) ){
						work->action_strcode = CODE_NEXTTOCOLADJ;
						SE_OK();				   
					} else {
						break;
					}
				
					return 2;/* キーコンフィグへ */
				}
				break;
			  case ITEM_EXIT:
				if(!(work->key_press & PAD_OK)) break;
				
				work->action_strcode = CODE_HIDEOPT;
				SE_CANCEL();
							  				
				return 1;
			}
		}
		
		if( work->node_flag2 != 1 ){
			/* タイトルオプション画面 */
			/* フルで項目を使う */
			int cur0,cur1;

			cur0 = work->menu_cursor;

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			  case PAD_U:
				work->menu_cursor--;
#ifdef _WINDOWS
				if( work->menu_cursor == ITEM_CTR ){ work->menu_cursor-- ; }
#endif
				if(work->menu_cursor<0){
					work->menu_cursor = N_MENU_ITEMS-1;
				}

				SE_SEL();
				break;
			case PAD_D:
				work->menu_cursor++;
#ifdef _WINDOWS
				if( work->menu_cursor == ITEM_CTR ){ work->menu_cursor++ ; }
#endif
				if(work->menu_cursor >= N_MENU_ITEMS){
					work->menu_cursor=0;
				}

				SE_SEL();
				break;
			}
			
			cur1=work->menu_cursor;

			//printf("cur0[%d] cur1[%d]\n",cur0, cur1);

			if( cur0 != cur1 ){
				/* カーソルの移動 */
				KeyAnimSetX(work,0,MV_ANIM_COUNT,CUR_curs,
							menu_list[cur0].cur_pos,
							menu_list[cur1].cur_pos);

				work->explain_disp_flag = 0;/* リソースは表示しない */
				
				/* 旧項目の色変え */
				KeyAnimSetX(work,1,MV_ANIM_COUNT,menu_list[cur0].menu_item,
							menu_list[cur0].sel[ITEM_SEL_INDEX],
							menu_list[cur0].sel[ITEM_UNSEL_INDEX]);
				
				/* 新項目の色変え */
				KeyAnimSetX(work,4,MV_ANIM_COUNT,menu_list[cur1].menu_item,
							menu_list[cur1].sel[ITEM_UNSEL_INDEX],
							menu_list[cur1].sel[ITEM_SEL_INDEX]);
				
				
			}
		} else {
			/* ノードから呼び出される画面 */
			/* CONTROLS,COLOR ADJUSTMENTがなくなる */
			int cur0,cur1;

			cur0 = node_menu[ work->menu_cursor ];

			switch(work->key_autostatus & (PAD_U|PAD_D)){
			  case PAD_U:
				work->menu_cursor--;
#ifdef _WINDOWS
				if( node_menu[work->menu_cursor] == ITEM_CTR ){ work->menu_cursor-- ; }
#endif
				if(work->menu_cursor<0){
					work->menu_cursor = NODE_MENU_MAX-1;
				}

				SE_SEL();
				break;
			case PAD_D:
				work->menu_cursor++;
#ifdef _WINDOWS
				if( node_menu[work->menu_cursor] == ITEM_CTR ){ work->menu_cursor++ ; }
#endif
				if(work->menu_cursor >= NODE_MENU_MAX){
					work->menu_cursor=0;
				}

				SE_SEL();
				break;
			}
			
			cur1=node_menu[ work->menu_cursor ];

			printf("cur0[%d] cur1[%d]\n",cur0, cur1);

			if( cur0 != cur1 ){
				/* カーソルの移動 */
				KeyAnimSetX(work,0,MV_ANIM_COUNT,CUR_curs,
							menu_list[cur0].cur_pos,
							menu_list[cur1].cur_pos);

				work->explain_disp_flag = 0;/* リソースは表示しない */
				
				/* 旧項目の色変え */
				KeyAnimSetX(work,1,MV_ANIM_COUNT,menu_list[cur0].menu_item,
							menu_list[cur0].sel[ITEM_SEL_INDEX],
							menu_list[cur0].sel[ITEM_UNSEL_INDEX]);
				
				/* 新項目の色変え */
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
		if(type){
			L2D_SetSignalHandle(work->sub_l2d_handle,work,
								(void (*)(void *,int,int))ColorLayoutSignalFunc);
		}
		else{
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

#if 0 ///
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
#endif

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

static void SetVolumeSpr( Work *work ){
	/* SE,MUSICのスクロールバー、数字 */
	int val, n01, n10;
	SPR_OBJ *bar, *num01, *num10;
	float tw;
	
	if( work->l2d_handle < 0 ){
		//ASSERT(0);
		return ;
	}

	/* SE */
	val = work->menu_value_cursor[ITEM_SE];
	/* SEのスクロールバー 			*/
	/* エルミート補完で変化させる 	*/
	if( val != work->se_last_val && work->se_scrl_count < VOL_KEY_COUNT ){
		float t, k;
		int val0 = work->se_last_val;

		k = (val0 + val)*VOL_SCRL_BAR_UNIT*0.5f;
		t = (float)work->se_scrl_count / (float)(VOL_KEY_COUNT-1);
		tw = val0*VOL_SCRL_BAR_UNIT*(1-t)*(1-t) + 2*k*t*(1-t) + val*VOL_SCRL_BAR_UNIT*t*t;
		work->se_scrl_count ++;
		if( work->se_scrl_count == VOL_KEY_COUNT-1 ){
			/* 補完終了 */
			work->se_scrl_count = 0;
			work->se_last_val = val;
		}
	} else {
		work->se_scrl_count = 0;
		tw = VOL_SCRL_BAR_UNIT * val;
	}
	bar = L2D_GetObject( work->l2d_handle, GV_StrCode("se_vol_ber") );
	bar->sprite.dw = tw;
	/* SEの数字 */
	n01 = val % 10;
	num01 = L2D_GetObject( work->l2d_handle, GV_StrCode("se_num_1") );
	num01->sprite.head.tex.u = num01->sprite.head.tex.pu + SPR_FIXED( VOL_NUM_TEX_U * (n01 + 1) );
	n10 = val / 10;
	num10 = L2D_GetObject( work->l2d_handle, GV_StrCode("se_num_10") );
	num10->sprite.head.tex.u = num10->sprite.head.tex.pu + SPR_FIXED( VOL_NUM_TEX_U * (n10 + 1) );
	/* MUSIC */
	val = work->menu_value_cursor[ITEM_MSC];
	/* MUSICのスクロールバー 		*/
	/* エルミート補完で変化させる 	*/
	if( val != work->msc_last_val && work->se_scrl_count < VOL_KEY_COUNT ){
		float t, k;
		int val0 = work->msc_last_val;

		k = (val0 + val)*VOL_SCRL_BAR_UNIT*0.5f;
		t = (float)work->msc_scrl_count / (float)(VOL_KEY_COUNT-1);
		tw = val0*VOL_SCRL_BAR_UNIT*(1-t)*(1-t) + 2*k*t*(1-t) + val*VOL_SCRL_BAR_UNIT*t*t;
		work->msc_scrl_count ++;
		if( work->msc_scrl_count == VOL_KEY_COUNT-1 ){
			/* 補完終了 */
			work->msc_scrl_count = 0;
			work->msc_last_val = val;
		}
	} else {
		work->msc_scrl_count = 0;
		tw = VOL_SCRL_BAR_UNIT * val;
	}
	bar = L2D_GetObject( work->l2d_handle, GV_StrCode("bgm_vol_ber") );
	bar->sprite.dw = tw;
	/* MUSICの数字 */
	n01 = val % 10;
	num01 = L2D_GetObject( work->l2d_handle, GV_StrCode("music_num_1") );
	num01->sprite.head.tex.u = num01->sprite.head.tex.pu + SPR_FIXED( VOL_NUM_TEX_U * (n01 + 1) );
	n10 = val / 10;
	num10 = L2D_GetObject( work->l2d_handle, GV_StrCode("music_num_10") );
	num10->sprite.head.tex.u = num10->sprite.head.tex.pu + SPR_FIXED( VOL_NUM_TEX_U * (n10 + 1) );
	
	return ;
}

enum {
	OPTION_MAIN=0,
	OPTION_BUTTON_CONFIG,
	OPTION_SCREEN_ADJUST,
	OPTION_COLOR_ADJUST,
};


static void Step(Work *work)
{
	switch( work->step ){
	  case OPTION_MAIN:
		switch( SubStep(work) ){
		  case 1:
			/* オプション画面抜け(項目変更された) */
			work->ans = TITLE_MSG_MODEEND;
			work->step = 0x10;
			GM_TitleMenuStatus |= TITLE_MENU_OPTION_CHANGED;
			break;
		  case 2:
			/* キーコンフィグ CONTROLS へ */
			work->step = OPTION_BUTTON_CONFIG;
			work->sub_step = 0;
			break;
		  case 3:
			/* SCREEN ADJUSTMENT へ（不使用） */
			work->step = OPTION_SCREEN_ADJUST;
			work->sub_step = 0;
			work->key_repeat_time = REPEAT_NEXT2;
			work->wait_count = WAIT_COUNT_SCR;
			break;
		  case 4:
			/* COLOR ADJUSTMENT へ */
			work->step=OPTION_COLOR_ADJUST;
			work->sub_step = 0;
			work->wait_count = WAIT_COUNT_CLR;
			break;
		  case -1:
			/* オプション画面抜け(項目変更されず) */
			work->ans = TITLE_MSG_MODECANCEL;
			work->step = 0x10;
			break;
		  case 0:
			/* オプション画面のまま */
			break;
		}
		break;

	case OPTION_BUTTON_CONFIG:
		///山下さんのプログラム呼び出し
		if( work->sub_step == 0 ){
			BgscrFadeout(work);
			work->sub_step = 1;
			work->con_cnt = 0;
			break;
		} else if( work->sub_step == 1 ){
			/* コントロールの呼び出しタイミングを見る */
			work->con_cnt ++;
			if( work->con_cnt >= DIRECT_TICK(60) ){
				work->sub_step = 2;
			}
			break;
		} else if( work->sub_step == 2 ){
			int val;
			val = work->menu_value_cursor[ITEM_CTR] + 1;
			NewKeyConSel( 4, val, &work->con_flag );
			printf("con_flag [%p]\n",&work->con_flag );
			work->sub_step = 3;
		}
		switch( work->con_flag ){
		  case -1:
			/* キャンセル */
			printf("a\n");
			work->step=OPTION_MAIN;
			work->sub_step=0;
			BgscrFadein(work);
			break;
		  case 0:
			/* キーコントロール起動中 */
			//printf("b\n");
			break;

		  case 1:
		  case 2:
		  case 3:
		  case 4:
			{ int val1 = work->menu_value_cursor[ITEM_CTR];
			/* 選択された */
			work->menu_value_cursor[ITEM_CTR] = work->con_flag - 1;
			work->step=OPTION_MAIN;
			work->sub_step=0;
			BgscrFadein(work);
			{
				/* 値を変更 */
				void *part;
				int val = work->menu_value_cursor[ITEM_CTR];
				
				printf("CONTROLS TYPE = %d\n",val);
				part = L2D_GetParts( work->l2d_handle, menu_list[ITEM_CTR].menu_sw );
				if( part == NULL ) break;
				L2D_MorfObject( part, menu_list[ITEM_CTR].sw[1], menu_list[ITEM_CTR].sw[1], 1.0f );
			}
		  }
			break;
		}
		break;

	case OPTION_SCREEN_ADJUST:
#if 0 ///XBOXではなし
		switch(ScreenAdjustSubStep(work)){
		case 1:
			GM_TitleMenuStatus|=TITLE_MENU_SCRADJ_CHANGED;

		case -1:
			work->step=OPTION_MAIN;
			work->sub_step=0;
			work->key_repeat_time=REPEAT_NEXT;
			break;
		}
#endif
		break;
	case OPTION_COLOR_ADJUST:
		switch( ColorAdjustSubStep(work) ){
		case 1:
		case -1:
			work->step = OPTION_MAIN;
			work->sub_step = 0;
			break;
		}
		break;

	case 0x10:/* 抜け */
		if( !work->busy_flag ){			
			SetConfigValue(work);

			printf("work->l2d_handle[%d]\n",work->l2d_handle);

#if 1
			/* work->l2d_handle == -1 になる前に入れる */
			/* volumeの数字、スクロールバー値入れ */
			SetVolumeSpr( work );
			CopyKeyAlpha(work);
#endif
			if( work->l2d_handle >= 0 ){
				L2D_ReleaseLayout(work->l2d_handle);
				work->l2d_handle=-1;
			}

			CallLocalProc(work);

			if( work->strman != NULL ){
				GV_DestroyOtherActor(work->strman);
				work->strman = NULL;
			}

			GV_DestroyActor(work);
		}
		break;
	}

	/* volumeの数字、スクロールバー値入れ */
	SetVolumeSpr( work );

	CopyKeyAlpha(work);

}

static void Act(Work *work)
{
	/* メインL2D */
	if( L2D_ActionStatus(work->l2d_handle) == L2D_STAT_ACK ){
		work->busy_flag=0;
		if( work->action_strcode != 0 ){
			int res;
			res = L2D_EvokeAction( work->l2d_handle, work->action_strcode );
			work->action_strcode = 0;
			if( res < 0 ){
				ASSERT( 0 );
			}
			work->busy_flag = 1;
		}
	} else {
		work->busy_flag = 1;
	}

	/* 画面移動中L2D */
	if( work->sub_l2d_handle < 0 ){
		work->sub_busy_flag = 0;
	} else if( L2D_ActionStatus(work->sub_l2d_handle) == L2D_STAT_ACK ){
		work->sub_busy_flag=0;
		if( work->sub_action_strcode != 0 ){
			L2D_EvokeAction( work->sub_l2d_handle, work->sub_action_strcode );
			work->sub_action_strcode = 0;
			work->sub_busy_flag = 1;
		}
	} else {
		work->sub_busy_flag = 1;
	}

	Key(work);
	Step(work);
}

static void Die(Work *work)
{
	if( work->node_flag != NULL ){
		if( *work->node_flag == 1 ){
			*work->node_flag = -1;
		}
	}
	
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	if( work->node_flag2 == 1 ){
		/* ノード呼び出し時 */
		work->l2d_strcode = GV_StrCode( "node_menu" ); 
		work->l2d_handle = -1;
		work->bgname = GV_StrCode( "バックグラウンド" );
		work->lang_num = 1;/* ON, OFF */

		{
			int i, k;

			capstate_name[0] = SW_capOFF;
			work->lang_flag[0] = 0;
			k = 1;
			for( i = 0; i < work->lang_num; i++ ){
				int lang;

				lang = GM_Language;
				if( i == 0 ){	/* 一つめの言語がデフォルト(OFFの時に表示される言語) */
					work->lang_default = lang;
					work->lang_ref[0] = lang; /* OFFの時の言語 */
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
				if( work->lang_num == 1 ){ /* 1言語の時はON,OFFのみ */
					work->lang_flag[lang] = 1;
					work->lang_ref[1] = work->lang_default;
					capstate_name[1] = SW_capON;
					break;
				}

			}
		}

		/* 「オプションの説明」 */
		work->option_explain_resource[ 0 ] = GV_StrCode("オプションの説明英語" );
		work->option_explain_resource[ 1 ] = GV_StrCode("オプションの説明フランス語" );
		work->option_explain_resource[ 2 ] = GV_StrCode("オプションの説明ドイツ語" );
		work->option_explain_resource[ 3 ] = GV_StrCode("オプションの説明イタリア語" );
		work->option_explain_resource[ 4 ] = GV_StrCode("オプションの説明スペイン語" );
		work->option_explain_resource[ 5 ] = 1;
		work->option_explain_resource[ 6 ] = GV_StrCode("オプションの説明日本語" );
		/* 「レーダーの説明」 */
		work->radar_explain_resource[ 0 ] = GV_StrCode("レーダーの説明英語" );
		work->radar_explain_resource[ 1 ] = GV_StrCode("レーダーの説明フランス語" );
		work->radar_explain_resource[ 2 ] = GV_StrCode("レーダーの説明ドイツ語" );
		work->radar_explain_resource[ 3 ] = GV_StrCode("レーダーの説明イタリア語" );
		work->radar_explain_resource[ 4 ] = GV_StrCode("レーダーの説明スペイン語" );
		work->radar_explain_resource[ 5 ] = 1;
		work->radar_explain_resource[ 6 ] = GV_StrCode("レーダーの説明日本語" );
		/* 「クイックチェンジの説明」 */
		work->quick_explain_resource[ 0 ] = GV_StrCode("クイックチェンジの説明英語" );
		work->quick_explain_resource[ 1 ] = GV_StrCode("クイックチェンジの説明フランス語" );
		work->quick_explain_resource[ 2 ] = GV_StrCode("クイックチェンジの説明ドイツ語" );
		work->quick_explain_resource[ 3 ] = GV_StrCode("クイックチェンジの説明イタリア語" );
		work->quick_explain_resource[ 4 ] = GV_StrCode("クイックチェンジの説明スペイン語" );
		work->quick_explain_resource[ 5 ] = 1;
		work->quick_explain_resource[ 6 ] = GV_StrCode("クイックチェンジの説明日本語" );
		
	} else {
		/* タイトルオプション */
		work->l2d_strcode=0;
		work->l2d_handle=-1;
		work->proc=0;
		work->bgname=0;
		
		if(GCL_GetOption('d')!=NULL){
			work->l2d_strcode=GCL_GetNextInt();
			printf("l2d [%d]\n",work->l2d_strcode);
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
#ifdef RELAREA_US
			  work->lang_num = 1;	// 米国リリース版は強制１言語
#endif
		  }
		if(GCL_GetOption('l')!=NULL){
			int i, k;

			capstate_name[0] = SW_capOFF;
			work->lang_flag[0] = 0;
			k = 1;
			for( i = 0; i < work->lang_num; i++ ){
				int lang;

				lang = GCL_GetNextInt();
				if( i == 0 ){	/* 一つめの言語がデフォルト(OFFの時に表示される言語) */
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
				if( work->lang_num == 1 ){ /* 1言語の時はON,OFFのみ */
					work->lang_flag[lang] = 1;
					work->lang_ref[1] = work->lang_default;
					capstate_name[1] = SW_capON;
					break;
				}

			}
		}
		{						/* リソース獲得 */
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
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

	if( work->node_flag2 == 1 ){
		/* ノード呼び出し */
		/* cache.darになかったらface.darを見に行く */
		void * ptr = GV_GetCache(GV_CacheID(work->l2d_strcode, 'o'));
		if(NULL == ptr) ptr = CDC_GetFileEntry(work->l2d_strcode, 'o');
		if( ptr == NULL ){
			printf("l2d nothing\n");
			return -1;
		}
		work->l2d_handle = L2D_SetupLayout2(ptr, DISP_CHANL, 5, SPR_FLAG_PRIV, 0);
	} else {
		/* タイトルオプション */
		work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,MENU_PRIORITY,0);
	}

	if(work->l2d_handle<0){
#ifdef DEBUG_MODE
		printf("Select Scr : L2D Init Error = %d\n",work->l2d_handle);
		ASSERT(0);
#endif

	} else {
		/* シグナルハンドラの設定 */
		if( work->node_flag2 != 1 ){
			L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))MainLayoutSignalFunc);
		} else {
#if 0
			/* ノード用のフレームアニメーションにシグナルを渡す */
			extern void NODFrameSignalHandleX(void * workp, int signal, int value);
			L2D_SetSignalHandle(work->l2d_handle,work->node_work,(void (*)(void *,int,int))NODFrameSignalHandleX);
#endif
		}
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

	work->menu_cursor=0;

	work->start_flag=0;

	for( i = 0; i < ANIM_WORK_SIZE; i++ ){
		work->key_anim[i].count=0;
	}

//MULTI_LANG
	work->lang_val  = work->menu_value_cursor[ITEM_CAP];
	work->first_val = work->menu_value_cursor[ITEM_CAP];
		
}

/* 初期化部メイン */
void *NewOptionScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		//work->node_flag = &work->node_flag2;


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

//#include "../../mode/node/node_config.h"
#define NODE_ACT_MODE    GV_ACTOR_MANAGER  /* 動作レベル */
#define NODE_ACT_PRIO    0xf0              /* 優先順位   */

void *NewOptionScrForNodeX( void *node_work, int *node_flag )
{
    Work *work ;

	/* パケットメモリに確保 */
#if 0
	if(( work = (Work *)codecMalloc(sizeof(Work)))==NULL ){
		return NULL;
	}
#else
    work=(Work *)GV_NewActorPrio(NODE_ACT_MODE,sizeof(Work), NODE_ACT_PRIO);
#endif

    if(work!=NULL) {
		//work->name=name;
		work->node_flag = node_flag;
		*work->node_flag = 1;/* 起動中フラグ */
		work->node_flag2 = 1;
		work->node_work = node_work;

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
		//GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			*work->node_flag = -1;
			GV_DestroyActor(work);
			return NULL ;
		}
		//if((work->strman=NewTextScreenControlForTitle())==NULL){
		if((work->strman=NewTextScreenControlForCodec())==NULL){
			*work->node_flag = -1;
			GV_DestroyActor(work);
			return NULL;
		}

		InitMenu(work);
		/* ノードから帰ってきたら、ポーズがかからなくなったので */
		//GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
	printf("OPOP end\n");
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
