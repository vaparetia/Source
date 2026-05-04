//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	loadgame.c
		ロードゲーム画面

	2001/06/15 K.Kano
	$Id: loadgame.c,v 1.6 2002/12/05 18:42:00 takaki Exp $
*/


#include "titlescr.h"

#include "font.h"
#include "libfs.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "BP_SaveLoadMGS.h"

#ifdef BP_PS3
#include "Transfarring_PS3CGlue.h"
#else
#include "TransfarringVTACGlue.h"
#endif
#include "transfarring_save_load_menu_utils.h"

#include "BP_Network.h"
#include "BP_TUS.h"

#include "Transfarring_UI_Strings.h"

#include "BP_CommonDialog.h"

#include "BP_Misc.h                                                                              "

extern void ShowWrongUserWarning(int *pResult);

#define N_YESNO_ITEMS		2
#define N_SLOT_ITEMS		2
#define N_FILES				10

#define DISP_PAGE_NUM		5


#define PAGE_STR_LEN		8
#define PAGE_NUM_STR_LEN	8
#define YESNO_STR_LEN		8

static const char* tales_title_name[] = {
	MCMAN_TALES_TITLE_A,
	MCMAN_TALES_TITLE_B,
	MCMAN_TALES_TITLE_C,
	MCMAN_TALES_TITLE_D,
	MCMAN_TALES_TITLE_E,
};

#ifdef BP_360
extern void bp_set_current_savegame( const char * const dirsuffix );
extern int gBP_CanSelectNewDevice;
#endif

typedef struct _loadgame_Work {
	GV_ACT_EX actor;

	MCMAN_WORK mcman;
	MCMAN_GAMEDATA savedata;

	void *strman;

	int name;
	int parent_name;
	int bgname;

	int busy_flag;
	int l2d_strcode;
	int l2d_handle;
	int action_strcode;

	int proc;
	int step;
	int sub_step;
	int timer;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int ans;

	int file_cursor;
	int page_cursor;
	int n_pages;
	int fileinfo_width;
	float cursor_y_pos;
	float cursor_t_y_pos;

	int yesno_cursor;

	int start_index;
	int n_indexes;
	int indexes[N_FILES];
	int file_y_pos[N_FILES];
	int file_alpha[N_FILES];
	int file_t_y_pos[N_FILES];
	int file_t_alpha[N_FILES];
	int file_anim_count;

	int page_alpha;
	int page_talpha;
	int page_alpha_count;

	SPR_OBJ *cursor;
	SPR_OBJ *cursor_spr;
	SPR_OBJ *slot[N_SLOT_ITEMS];

	// SPR_OBJ *yesno[N_YESNO_ITEMS];

	int base_u,base_v;

	SPR_OBJ *page_str[PAGE_STR_LEN];
	SPR_OBJ *page_num_str[PAGE_NUM_STR_LEN];
	SPR_OBJ *yesno_str[YESNO_STR_LEN];

	float tag_anim_p;
	int tag_anim0;
	int tag_anim1;
	int tag_count;

	unsigned char port_sel;
	unsigned char start_mode;
	unsigned char finish_mode;
	unsigned char sound_flag;

	unsigned char i_fileinfo;
	unsigned char update_fileinfo;
	unsigned char search_dir;

	unsigned char mcchanged_flags;

   unsigned char mCloudConflictFlag;
   unsigned char mDisplayTrophiesDisabled;
   unsigned char mDogTagLoad;
   unsigned char mHidePsnWidget;
   ECloudConflictResolution mCloudConflictResolution;
   
#if 0 //yano
#ifdef PAL
	int loadgame_resource;
	int stagename_resource;
	int difficulty_resource;

	int yes_width;
	int slash_width;
	int no_width;
#endif
#else
	int loadgame_resource;
	int stagename_resource;
	int difficulty_resource;

	int yes_width;
	int slash_width;
	int no_width;

   int wasSignedIn;

   #if defined(BP_VITA)
   int mSystemResumeCount;
   #endif

   int mSyncSlotsRemaining;
#endif
} Work;


#define MAX_LEVEL				5


//#ifdef AREA_EU_BP_IGNORE()	// #ifndef PAL
#define DAMAGED_FILE		( BP_Area_EU() ? "DAMAGED FILE" : "DAMAGED DATA" )
//#else
//#define DAMAGED_FILE		"DAMAGED DATA"
//#endif
#ifdef MGS2_VRTRIAL
#define VR_TRAINING_FILE	"VR Training Data"
#else
#define VR_TRAINING_FILE	"Missions Data:"
#endif


#define DISP_CHANL			TITLE_CHANL
#define LOAD_PRIORITY		TITLE_PRI_NORMAL


#define PORTSEL_NAME		0x005bf5f1		/* portsel */

#if 0 //yano 多言語
#ifdef NTSC

#define LOADGAME_RESOURCE	0x00873e66		/* ロードゲームリソース */
#define STAGENAME_RESOURCE	0x0073d521		/* ステージ名 */
#define DIFFICULTY_RESOURCE	0x00dc6beb		/* 難易度名 */

#endif

#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL

#define LOADGAME_RESOURCE	(work->loadgame_resource)		/* ロードゲームリソース */
#define STAGENAME_RESOURCE	(work->stagename_resource)		/* ステージ名 */
#define DIFFICULTY_RESOURCE	(work->difficulty_resource)		/* 難易度名 */

#define E_LOADGAME_RESOURCE		0x00873e66		/* ロードゲームリソース */
#define E_STAGENAME_RESOURCE	0x0073d521		/* ステージ名 */
#define E_DIFFICULTY_RESOURCE	0x00dc6beb		/* 難易度名 */

#define F_LOADGAME_RESOURCE		0x001cd935		/* ロードゲームリソースフランス語 */
#define G_LOADGAME_RESOURCE		0x00018abf		/* ロードゲームリソースドイツ語 */
#define I_LOADGAME_RESOURCE		0x005b72b4		/* ロードゲームリソースイタリア語 */
#define S_LOADGAME_RESOURCE		0x0011bd70		/* ロードゲームリソーススペイン語 */

static const int loadgame_resource_name[]={
	E_LOADGAME_RESOURCE,
	F_LOADGAME_RESOURCE,
	G_LOADGAME_RESOURCE,
	I_LOADGAME_RESOURCE,
	S_LOADGAME_RESOURCE,
};

#endif
#else

//static int loadgame_resource_name[LANG_MAX_NUM];
#define LOADGAME_RESOURCE	(work->loadgame_resource)		/* ロードゲームリソース */
#define STAGENAME_RESOURCE	(work->stagename_resource)		/* ステージ名 */
#define DIFFICULTY_RESOURCE	(work->difficulty_resource)		/* 難易度名 */

#define E_STAGENAME_RESOURCE	0x0073d521		/* ステージ名 */
#define J_STAGENAME_RESOURCE	0x001e51e5		/* ステージ名日本語 */
#define E_DIFFICULTY_RESOURCE	0x00dc6beb		/* 難易度名 */

#define J_LOADGAME_RESOURCE		0x00f8a329		/* ロードゲームリソース日本語 */
#define E_LOADGAME_RESOURCE		0x00c44fd2		/* ロードゲームリソース英語 */
#define F_LOADGAME_RESOURCE		0x001cd935		/* ロードゲームリソースフランス語 */
#define G_LOADGAME_RESOURCE		0x00018abf		/* ロードゲームリソースドイツ語 */
#define I_LOADGAME_RESOURCE		0x005b72b4		/* ロードゲームリソースイタリア語 */
#define S_LOADGAME_RESOURCE		0x0011bd70		/* ロードゲームリソーススペイン語 */

static const int loadgame_resource_name[]={
	E_LOADGAME_RESOURCE,
	F_LOADGAME_RESOURCE,
	G_LOADGAME_RESOURCE,
	I_LOADGAME_RESOURCE,
	S_LOADGAME_RESOURCE,
	0, //韓国語
	J_LOADGAME_RESOURCE,
};

static const int stagename_resource_name[]={
	E_STAGENAME_RESOURCE,
	E_STAGENAME_RESOURCE,
	E_STAGENAME_RESOURCE,
	E_STAGENAME_RESOURCE,
	E_STAGENAME_RESOURCE,
	0, //韓国語
	J_STAGENAME_RESOURCE,
};

#endif


#if 0
#define FONT_WIDTH				24
#define FONT_HEIGHT				24
#else
#define FONT_WIDTH				FONT_SIZE_W
#define FONT_HEIGHT				FONT_SIZE_H
#endif

#define FONT_DISP_WIDTH			16
#define FONT_DISP_HEIGHT		16

#if defined(BP_VITA)
   #define LINE_SPACE				9
#else
   #define LINE_SPACE				RUBI_SIZE_H
#endif

#define LINE_HEIGHT				(FONT_HEIGHT+LINE_SPACE)

#if 0
#define FILE_HEIGHT				FONT_HEIGHT
#else
#define FILE_HEIGHT				(LINE_HEIGHT+DOWN_MARGINE)
#endif



#define ID_U				0
#define ID_V				0
#define ID_WIDTH			(FONT_WIDTH*3)
#define ID_HEIGHT			FILE_HEIGHT
#define TEXT_U				(ID_U+ID_WIDTH)
#define TEXT_V				ID_V
#define TEXT_WIDTH			(FONT_WIDTH*10)
#define TEXT_HEIGHT			FILE_HEIGHT
#define DATE_U				(TEXT_U+TEXT_WIDTH)
#define DATE_V				TEXT_V
#define DATE_WIDTH			(FONT_WIDTH*10)
#define DATE_HEIGHT			FILE_HEIGHT
#define TIME_U				(DATE_U+DATE_WIDTH)
#define TIME_V				TEXT_V
#define TIME_WIDTH			(FONT_WIDTH*10)
#define TIME_HEIGHT			FILE_HEIGHT

#define LOADRES_CONFIRM_U		0
#define LOADRES_CONFIRM_V		(FILE_HEIGHT*N_FILES)
#define LOADRES_CONFIRM_WIDTH	(FONT_WIDTH*30)
#define LOADRES_CONFIRM_HEIGHT	FILE_HEIGHT
#define LOADRES_LOADING_U		0
#define LOADRES_LOADING_V		(FILE_HEIGHT*(N_FILES+1))
#define LOADRES_LOADING_WIDTH	(FONT_WIDTH*30)
#define LOADRES_LOADING_HEIGHT	FILE_HEIGHT
#define LOADRES_SUCCESS_U		0
#define LOADRES_SUCCESS_V		(FILE_HEIGHT*(N_FILES+2))
#define LOADRES_SUCCESS_WIDTH	(FONT_WIDTH*30)
#define LOADRES_SUCCESS_HEIGHT	FILE_HEIGHT
#define LOADRES_FAILED_U		0
#define LOADRES_FAILED_V		(FILE_HEIGHT*(N_FILES+3))
#define LOADRES_FAILED_WIDTH	(FONT_WIDTH*30)
#define LOADRES_FAILED_HEIGHT	FILE_HEIGHT

#define LOADRES_FILEINFO_U			0
#define LOADRES_FILEINFO_V			(FILE_HEIGHT*(N_FILES+4))
#define LOADRES_FILEINFO_WIDTH		(FILE_HEIGHT*30)
#define LOADRES_FILEINFO_HEIGHT		FILE_HEIGHT
#define LOADRES_FILEINFO_U2			0
#define LOADRES_FILEINFO_V2			(FILE_HEIGHT*(N_FILES+5))
#define LOADRES_FILEINFO_WIDTH2		(FONT_WIDTH*30)
#define LOADRES_FILEINFO_HEIGHT2	FILE_HEIGHT



#define FILE_FONT_DISP_WIDTH		(15+3)
#define FILE_FONT_DISP_HEIGHT		(16+4)

#define ID_X				(56 - 4 + TRANSFARRING_X_OFFSET)
#define ID_Y				(118-4)
#if 0
#define ID_W				(FILE_FONT_DISP_WIDTH*3)
#define ID_H				FILE_FONT_DISP_HEIGHT
#else
#define ID_W				(ID_WIDTH*FILE_FONT_DISP_WIDTH/FONT_WIDTH)
#define ID_H				(ID_HEIGHT*FILE_FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define ID_LINE_HEIGHT		21
#define TEXT_X				(108)
#define TEXT_Y				ID_Y
#if 0
#define TEXT_W				(FONT_DISP_WIDTH*10)
#define TEXT_H				FONT_DISP_HEIGHT
#else
#define TEXT_W				(TEXT_WIDTH*FILE_FONT_DISP_WIDTH/FONT_WIDTH)
#define TEXT_H				(TEXT_HEIGHT*FILE_FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define TEXT_LINE_HEIGHT	ID_LINE_HEIGHT
#define DATE_X				(240)
#define DATE_Y				TEXT_Y
#if 0
#define DATE_W				(FILE_FONT_DISP_WIDTH*10)
#define DATE_H				FILE_FONT_DISP_HEIGHT
#else
#define DATE_W				(DATE_WIDTH*FILE_FONT_DISP_WIDTH/FONT_WIDTH)
#define DATE_H				(DATE_HEIGHT*FILE_FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define DATE_LINE_HEIGHT	TEXT_LINE_HEIGHT
#define TIME_X				(384)
#define TIME_Y				TEXT_Y
#if 0
#define TIME_W				(FILE_FONT_DISP_WIDTH*10)
#define TIME_H				FILE_FONT_DISP_HEIGHT
#else
#define TIME_W				(TIME_WIDTH*FILE_FONT_DISP_WIDTH/FONT_WIDTH)
#define TIME_H				(TIME_HEIGHT*FILE_FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define TIME_LINE_HEIGHT	TEXT_LINE_HEIGHT


#if 0
#define MESSAGE_Y				72
#else
#define MESSAGE_Y				(ID_Y+ID_LINE_HEIGHT*9+4)
#endif

#define LOADRES_CONFIRM_X		(56-4)
#define LOADRES_CONFIRM_Y		MESSAGE_Y
#if 0
#define LOADRES_CONFIRM_W		(FONT_DISP_WIDTH*30)
#define LOADRES_CONFIRM_H		FONT_DISP_HEIGHT
#else
#define LOADRES_CONFIRM_W		(LOADRES_CONFIRM_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define LOADRES_CONFIRM_H		(LOADRES_CONFIRM_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define LOADRES_LOADING_X		(56-4)
#define LOADRES_LOADING_Y		MESSAGE_Y
#if 0
#define LOADRES_LOADING_W		(FONT_DISP_WIDTH*30)
#define LOADRES_LOADING_H		FONT_DISP_HEIGHT
#else
#define LOADRES_LOADING_W		(LOADRES_LOADING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define LOADRES_LOADING_H		(LOADRES_LOADING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define LOADRES_SUCCESS_X		(56-4)
#define LOADRES_SUCCESS_Y		MESSAGE_Y
#if 0
#define LOADRES_SUCCESS_W		(FONT_DISP_WIDTH*30)
#define LOADRES_SUCCESS_H		FONT_DISP_HEIGHT
#else
#define LOADRES_SUCCESS_W		(LOADRES_SUCCESS_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define LOADRES_SUCCESS_H		(LOADRES_SUCCESS_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define LOADRES_FAILED_X		(56-4)
#define LOADRES_FAILED_Y		MESSAGE_Y
#if 0
#define LOADRES_FAILED_W		(FONT_DISP_WIDTH*30)
#define LOADRES_FAILED_H		FONT_DISP_HEIGHT
#else
#define LOADRES_FAILED_W		(LOADRES_FAILED_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define LOADRES_FAILED_H		(LOADRES_FAILED_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define LOADRES_FILEINFO_X		(56-4)
#define LOADRES_FILEINFO_Y		92
#if 0
#define LOADRES_FILEINFO_W		(FONT_DISP_WIDTH*30)
#define LOADRES_FILEINFO_H		FONT_DISP_HEIGHT
#else
#define LOADRES_FILEINFO_W		(LOADRES_FILEINFO_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define LOADRES_FILEINFO_H		(LOADRES_FILEINFO_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define LISTLOAD_U				0
#define LISTLOAD_V				(FILE_HEIGHT*(N_FILES+6))
#define LISTLOAD_WIDTH			(FONT_WIDTH*30)
#define LISTLOAD_HEIGHT			FILE_HEIGHT

#define LISTLOAD_X				(56-4)
#define LISTLOAD_Y				168
#if 0
#define LISTLOAD_W				(FONT_DISP_WIDTH*30)
#define LISTLOAD_H				FONT_DISP_HEIGHT
#else
#define LISTLOAD_W				(LISTLOAD_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define LISTLOAD_H				(LISTLOAD_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define LISTLOAD_COUNT			DIRECT_TICK(1.0f*60.0f)


#define YES_U					0
#define YES_V					(FILE_HEIGHT*(N_FILES+7))
#define YES_WIDTH				(FONT_WIDTH*6)
#define YES_HEIGHT				FILE_HEIGHT

#define YES_W					(YES_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define YES_H					(YES_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define SLASH_U					(YES_U+YES_WIDTH)
#define SLASH_V					YES_V
#define SLASH_WIDTH				(FONT_WIDTH*3)
#define SLASH_HEIGHT			FILE_HEIGHT

#define SLASH_W					(SLASH_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SLASH_H					(SLASH_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define NO_U					(SLASH_U+SLASH_WIDTH)
#define NO_V					SLASH_V
#define NO_WIDTH				(FONT_WIDTH*6)
#define NO_HEIGHT				FILE_HEIGHT

#define NO_W					(NO_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define NO_H					(NO_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define TRANSFARRING_TEXT_X		(56-4)
#define TRANSFARRING_TEXT_Y		(MESSAGE_Y - ID_LINE_HEIGHT)

#define TRANSFARRING_TEXT_U					0
#define TRANSFARRING_TEXT_V					(YES_V + FILE_HEIGHT)
#define TRANSFARRING_TEXT_WIDTH				(FONT_WIDTH * 30)
#define TRANSFARRING_TEXT_HEIGHT				(FILE_HEIGHT * 2)

#define TRANSFARRING_TEXT_W					(TRANSFARRING_TEXT_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define TRANSFARRING_TEXT_H					(TRANSFARRING_TEXT_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#if 0
#define PAGE_STR_X				(340-16)
#define PAGE_STR_Y				75
#else
#define PAGE_STR_X				324
#define PAGE_STR_Y				74
#endif

#define PAGE_NUM_STR_X			429
#define PAGE_NUM_STR_Y			73

#define YESNO_STR_X				(384-8)
#define YESNO_STR_Y				(MESSAGE_Y+5)


#define YESNO_STR_RX			470


#define STR_NODE_FONT		0x009946b8		/* font2 */

#define PAGE_STR_WIDTH		(14)
#define PAGE_STR_HEIGHT		(10)

#define	STR_WIDTH			(18) // strcode ではなく文字の長さ
#define STR_HEIGHT			(14) // strcode ではなく文字の長さ

#if 0
#define SK_FONT_WIDTH			(18.0f-2.0f)
#define SK_FONT_HEIGHT			(14.0f-2.0f)
#else
#define SK_FONT_WIDTH			18.0f
#define SK_FONT_HEIGHT			14.0f
#endif

#define SK_FONT_SPACE_HEIGHT	4.0f  /* DRAW_HEIGHT / 384.0f */



enum {
	RES_INDEX_CONFIRM=2,
	RES_INDEX_LOADING,
	RES_INDEX_SUCCESS,
	RES_INDEX_FAILED,

	RES_INDEX_YES,
	RES_INDEX_NO,
};


#define DISP_RESULT_COUNT	DIRECT_TICK((int)(3.0f*60.0f))		/* ロードの結果の表示時間 */
#define DISP_RESULT_COUNT2	DIRECT_TICK((int)(0.5f*60.0f))		/* ロードの結果の表示時間２ */


#define FILE_ALPHA_TIME		DIRECT_TICK(10)
#define PAGE_MOVE_TIME		DIRECT_TICK(3)
#define CURSOR_MOVE_TIME	DIRECT_TICK(5)


#if 0

#define TEXTCOLOR			0x80808080
#define TEXTCOLOR_NOSEL		0x00808080

#else

#define TEXT_R				(160/2)
#define TEXT_G				(180/2)
#define TEXT_B				(170/2)

#define TEXTCOLOR			(TEXT_R | (TEXT_G<<8) | (TEXT_B<<16) | (0x80<<24))
#define TEXTCOLOR_NOSEL		(TEXT_R | (TEXT_G<<8) | (TEXT_B<<16))

#define TEXTCOLOR_CLEARED	(0x80 | (0x10<<8) | (0x10<<16))

#endif


#define REPEAT_FIRST		20
#define REPEAT_NEXT			1


#define CURSOR_ADJUST		3.0f


extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え

extern void DecodeVRInfo( MCMAN_VR_INFODATA* pDst, const MCMAN_INFODATA* pSrc );

extern void BackupLoadedTransfarringID();

#if defined(BP_VITA) || defined(BP_PS3)
extern int gSyncSlotsRemaining;
#endif

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

enum {
	MCARD_INIT=0,
	MCARD_FILE_CHECK,
	MCARD_FILE_SEL,
	MCARD_CONFIRM,
	MCARD_PROCESS,
	MCARD_FINISH,
	MCARD_FAILED,
};


static void DisplaySynchingMessage(Work* work)
{
#if defined(BP_VITA) || defined(BP_PS3)
   int const syncSlotsRemaining = gSyncSlotsRemaining;

   if(syncSlotsRemaining)
   {
      if(work->mSyncSlotsRemaining != syncSlotsRemaining)
      {
         char tmpString[512] = { 0 };

         work->mSyncSlotsRemaining = syncSlotsRemaining;

         sprintf(tmpString, BP_GetCustomOverrideString("SAVELOAD", "SYNCHING_CLOUD_FILES"), syncSlotsRemaining);

         MENU_ClearPartTextTexture(work->strman, TRANSFARRING_TEXT_V, TRANSFARRING_TEXT_HEIGHT);

         MENU_CreateTextTexture(work->strman, TRANSFARRING_TEXT_U, TRANSFARRING_TEXT_V,
            TRANSFARRING_TEXT_WIDTH, TRANSFARRING_TEXT_HEIGHT, 0, 0, 0, tmpString);
      }

      MENU_PutTextScreenNoTR(work->strman, TRANSFARRING_TEXT_X, TRANSFARRING_TEXT_Y,
         TRANSFARRING_TEXT_X + TRANSFARRING_TEXT_W, TRANSFARRING_TEXT_Y + TRANSFARRING_TEXT_H,
         TRANSFARRING_TEXT_U, TRANSFARRING_TEXT_V, TRANSFARRING_TEXT_U + TRANSFARRING_TEXT_WIDTH,
         TRANSFARRING_TEXT_V + TRANSFARRING_TEXT_HEIGHT, TEXTCOLOR);
   }
#endif
}

static void CallLocalMsg(Work *work)
{
	if(work->parent_name!=0 && work->parent_name!=1){
		GV_MSG msg;
		int message[4];

		msg.address=work->parent_name;
		msg.message=message;
		msg.message_len=sizeof(message)/sizeof(message[0]);

		message[0]=work->name;
		message[1]=work->ans;

		GV_SendMessage(&msg);
	}
}

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=((work->ans==TITLE_MSG_MODEEND) ? 1 : 0);

		GCL_ExecProc(work->proc,&arg);
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


/* ------------------------------------------------------------------------ */

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


/* ------------------------------------------------------------------------ */

static const int tag_part_name[]={
	CODE_SRASH,
	CODE_DOGTAG,
	CODE_ALBUMTAG,
	CODE_DATASAVETAG,
	CODE_DATALOADTAG,
	CODE_UNDERBAR,
};

#define PART_SIZE		(sizeof(tag_part_name)/sizeof(tag_part_name[0]))


static void TagAnimSet0(Work *work,int tag_anim)
{
	int i;

	work->tag_anim0=tag_anim;
	work->tag_anim1=tag_anim;
	work->tag_count=0;
	work->tag_anim_p=1.0f;

	for(i=0;i<PART_SIZE;i++){
		void *part=L2D_GetParts(work->l2d_handle,tag_part_name[i]);
		if(part==NULL) continue;
		L2D_MorfObject(part,work->tag_anim1,work->tag_anim1,1.0f);
	}
}

static void TagAnimSet(Work *work,int tag_anim0,int tag_anim1,int count)
{
	work->tag_anim0=tag_anim0;
	work->tag_anim1=tag_anim1;
	work->tag_count=count;
	work->tag_anim_p=0.0f;
}

static void TagAnim(Work *work)
{
	if(work->tag_count>0){
		int i;

		work->tag_anim_p+=(1.0f-work->tag_anim_p)/(float)(work->tag_count);
		work->tag_count--;

		for(i=0;i<PART_SIZE;i++){
			void *part=L2D_GetParts(work->l2d_handle,tag_part_name[i]);
			if(part==NULL) continue;
			L2D_MorfObject(part,work->tag_anim0,work->tag_anim1,work->tag_anim_p);
		}
	}
}

/* ------------------------------------------------------------------------ */

#if 0

static void ShowSlot(Work *work)
{
	SPR_SHOW(work->slot[0]);
	SPR_SHOW(work->slot[1]);
}

static void HideSlot(Work *work)
{
	SPR_HIDE(work->slot[0]);
	SPR_HIDE(work->slot[1]);
}

#endif


/* ------------------------------------------------------------------------ */

static int SearchUpFile(Work* work, int start)
{
	int i=start;
	while(i< MCMAN_GetDataFileMax( work->mcman.file_kind ) ) {
		// if(MCManCheckFileFlag(i)) return i;
		if(MCManCheckExactFileFlag(i)) return i;
		i++;
	}
	return -1;
}

static int SearchDownFile(int start)
{
	int i=start;
	while(i>=0){
		// if(MCManCheckFileFlag(i)) return i;
		if(MCManCheckExactFileFlag(i)) return i;
		i--;
	}
	return -1;
}

static void SearchUpFiles(Work *work)
{
	int i;
	int start=work->start_index;

	for(i=0;i<N_FILES;i++){
		int j;
		j=SearchUpFile(work, start);
		if(j==-1){
			work->n_indexes=i;
			return;
		}
		else{
			work->indexes[i]=j;
			start=j+1;
		}
	}
	work->n_indexes=N_FILES;
}

static void SearchDownFiles(Work *work)
{
	int i;
	int start=work->start_index;

	for(i=N_FILES-1;i>=0;i--){
		int j;
		j=SearchDownFile(start);
		if(j==-1){
			work->n_indexes=i;
			return;
		}
		else{
			work->indexes[i]=j;
			start=j-1;
		}
	}
	work->n_indexes=N_FILES;
}

static inline int DateCompare(unsigned char *a,unsigned char *b)
{
	if(a[DATETIME_INDEX_YEAR]<b[DATETIME_INDEX_YEAR]) return -1;
	else if(a[DATETIME_INDEX_YEAR]>b[DATETIME_INDEX_YEAR]) return 1;

	if(a[DATETIME_INDEX_MONTH]<b[DATETIME_INDEX_MONTH]) return -1;
	else if(a[DATETIME_INDEX_MONTH]>b[DATETIME_INDEX_MONTH]) return 1;

	if(a[DATETIME_INDEX_DAY]<b[DATETIME_INDEX_DAY]) return -1;
	else if(a[DATETIME_INDEX_DAY]>b[DATETIME_INDEX_DAY]) return 1;

	if(a[DATETIME_INDEX_HOUR]<b[DATETIME_INDEX_HOUR]) return -1;
	else if(a[DATETIME_INDEX_HOUR]>b[DATETIME_INDEX_HOUR]) return 1;

	if(a[DATETIME_INDEX_MIN]<b[DATETIME_INDEX_MIN]) return -1;
	else if(a[DATETIME_INDEX_MIN]>b[DATETIME_INDEX_MIN]) return 1;

	if(a[DATETIME_INDEX_SEC]<b[DATETIME_INDEX_SEC]) return -1;
	else if(a[DATETIME_INDEX_SEC]>b[DATETIME_INDEX_SEC]) return 1;

	return 0;
}

static int SearchLatestFile(void)
{
	int i=0,cnt=MCManMaxId();
	int rid=cnt;
	unsigned char *rdate=NULL;

	// printf("Search Latest ----------------------\n");

	// printf("Max ID = %d\n",cnt);

	while(cnt>=0){
		if(MCManCheckExactFileFlag(i)){
			unsigned char *date=(unsigned char *)MCManGetDateTime(i);

			if(rdate==NULL){
				rdate=date;
				rid=i;
			}
			else{
				if(DateCompare(date,rdate)>=0){
					rdate=date;
					rid=i;
				}
			}
		}
		i++; cnt--;
	}

	return rid;
}


/* ------------------------------------------------------------------------ */

static void CreateFilenameTexture(Work *work)
{
	char tdatestr[32];
	char ttimestr[32];
	char tid[32];
	int i;

	char *confirmstr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_CONFIRM));
	char *loadingstr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_LOADING));
	char *successstr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_SUCCESS));
	char *failedstr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_FAILED));
   
#if 0 //yano
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	char *yesstr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_YES));
	char *nostr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_NO));
#endif
#else
	char *yesstr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_YES));
	char *nostr=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(LOADGAME_RESOURCE,RES_INDEX_NO));
#endif
printf("?????\n");

	MENU_ClearTextTexture(work->strman);

	MENU_CreateTextTexture(work->strman,
						   LOADRES_CONFIRM_U,LOADRES_CONFIRM_V,
						   LOADRES_CONFIRM_WIDTH,LOADRES_CONFIRM_HEIGHT,0,0,0,confirmstr);
	MENU_CreateTextTexture(work->strman,
						   LOADRES_LOADING_U,LOADRES_LOADING_V,
						   LOADRES_LOADING_WIDTH,LOADRES_LOADING_HEIGHT,0,0,0,loadingstr);
	MENU_CreateTextTexture(work->strman,
						   LOADRES_SUCCESS_U,LOADRES_SUCCESS_V,
						   LOADRES_SUCCESS_WIDTH,LOADRES_SUCCESS_HEIGHT,0,0,0,successstr);
	MENU_CreateTextTexture(work->strman,
						   LOADRES_FAILED_U,LOADRES_FAILED_V,
						   LOADRES_FAILED_WIDTH,LOADRES_FAILED_HEIGHT,0,0,0,failedstr);

#if 0 //yano
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	work->yes_width=MENU_CreateTextTexture(work->strman,
										   YES_U,YES_V,
										   YES_WIDTH,YES_HEIGHT,0,0,yesstr)-YES_U;
	work->slash_width=MENU_CreateTextTexture(work->strman,
											 SLASH_U,SLASH_V,
											 SLASH_WIDTH,SLASH_HEIGHT,0,0," / ")-SLASH_U;
	work->no_width=MENU_CreateTextTexture(work->strman,
										  NO_U,NO_V,
										  NO_WIDTH,NO_HEIGHT,0,0,nostr)-NO_U;
#endif
#else
	work->yes_width=MENU_CreateTextTexture(work->strman,
										   YES_U,YES_V,
										   YES_WIDTH,YES_HEIGHT,0,0,0,yesstr)-YES_U;
	work->slash_width=MENU_CreateTextTexture(work->strman,
											 SLASH_U,SLASH_V,
											 SLASH_WIDTH,SLASH_HEIGHT,0,0,0," / ")-SLASH_U;
	work->no_width=MENU_CreateTextTexture(work->strman,
										  NO_U,NO_V,
										  NO_WIDTH,NO_HEIGHT,0,0,0,nostr)-NO_U;
#endif


	/* ファイル名や日時の展開 */
#if 1
	for(i=0;i<work->n_indexes;i++){
		int id=work->indexes[i];
#else
	for(i=0;i<N_FILES;i++){
		int id=work->indexes[0];
#endif
		MCMAN_INFODATA *info=MCManGetFileInfo(id);
		MCMAN_VR_INFODATA vrinfo;
		int ptime=info->mgs2_playtime/FRAMES_PER_SEC;
		int diff=info->difficulty;

		char *str;
		unsigned char *data;
		int x,y;

		char buffer[16];

		if( work->mcman.file_kind == MCMAN_FILE_KIND_VR ) {
			DecodeVRInfo( &vrinfo, info );
			ptime = vrinfo.vr_playtime / FRAMES_PER_SEC;
		} 
#if 0
		strcpy(tid,"000");

		tid[0]+=(id/100) % 10;
		tid[1]+=(id/10) % 10;
		tid[2]+=(id/1) % 10;
#else
		strcpy(tid,"00");

		tid[0]+=(id/10) % 10;
		tid[1]+=(id/1) % 10;
#endif

		x=ID_U;
		y=ID_V+ID_HEIGHT*i;

		MENU_CreateTextTexture(work->strman,x,y,ID_WIDTH,ID_HEIGHT,0,0,0,tid);
		

		diff=(diff-GM_LEVEL_VERYEASY)/10;
		switch ( work->mcman.file_kind ) {
		case MCMAN_FILE_KIND_GAME:
			if(MCManCheckExactFileFlag(id)<0){
				str="----------";
			}
			else if(diff>=0 && diff<=MAX_LEVEL){
printf("o!!!!\n");
				str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(DIFFICULTY_RESOURCE,diff));
printf("!!!!\n");
			}
			else{
				str="NO LEVEL";
			}
			break;
		case MCMAN_FILE_KIND_VR:
			str = buffer;
			if(MCManCheckExactFileFlag(id)<0){
				str = "------";
			} else {
				sprintf( str, "%3.1f%%", vrinfo.achieve_high + 0.1f * vrinfo.achieve_low );
			}
			break;
		case MCMAN_FILE_KIND_SNAKE_TALES:
			str = buffer;
			if(MCManCheckExactFileFlag(id)<0){
				str = "----------";
			} else if ( info->snake_tales_no >= 1 && info->snake_tales_no <= MCMAN_MAX_TALES ) {
				str = (char*)tales_title_name[ info->snake_tales_no - 1 ];
			} else {
				str = "?";
			}
			break;
		}

		x=TEXT_U;
		y=TEXT_V+TEXT_HEIGHT*i;

		MENU_CreateTextTexture(work->strman,x,y,TEXT_WIDTH,TEXT_HEIGHT,0,0,0,str);

		if( work->mcman.file_kind == MCMAN_FILE_KIND_SNAKE_TALES ) {
			/* クリアフラグ */
			int j;
			int n;
			char clrstr[ MCMAN_MAX_TALES + 1 ];

			x=TIME_U;
			y=TIME_V+TIME_HEIGHT*i;

			for( j = 0, n = 0 ; j < MCMAN_MAX_TALES ; j ++ ) {
				if( info->st_clear_flag & ( 1 << j ) ) {
					clrstr[ n++ ] = 'A' + j;
				} 
			}
			clrstr[ n ] = '\0';

			if(MCManCheckExactFileFlag(id)<0){
				strcpy(clrstr,"-----");
			}
			MENU_CreateTextTexture(work->strman,x,y,TIME_WIDTH,TIME_HEIGHT,0,0,0,clrstr);
		} else {
			/* 日付 */
			x=DATE_U;
			y=DATE_V+DATE_HEIGHT*i;

			data=(unsigned char *)MCManGetDateTime(id);

			if(MCManCheckExactFileFlag(id)<0){
				strcpy(tdatestr,"---- -- --");
			}
			else{
				strcpy(tdatestr,"2000 00 00");

				tdatestr[2]+=data[DATETIME_INDEX_YEAR]/10;
				tdatestr[3]+=data[DATETIME_INDEX_YEAR]%10;
				tdatestr[5]+=data[DATETIME_INDEX_MONTH]/10;
				tdatestr[6]+=data[DATETIME_INDEX_MONTH]%10;
				tdatestr[8]+=data[DATETIME_INDEX_DAY]/10;
				tdatestr[9]+=data[DATETIME_INDEX_DAY]%10;
			}

			MENU_CreateTextTexture(work->strman,x,y,DATE_WIDTH,DATE_HEIGHT,0,0,0,tdatestr);

			/* プレイ時間 */
			x=TIME_U;
			y=TIME_V+TIME_HEIGHT*i;

			if(MCManCheckExactFileFlag(id)<0){
				strcpy(ttimestr,"----:--:--");
			}
			else if(ptime>=60*60*10000){
				strcpy(ttimestr,"9999:59:59");
			}
			else{
				int hour,min,sec;

				hour=ptime/3600;
				ptime%=3600;
				min=ptime/60;
				sec=ptime%60;

				strcpy(ttimestr,"0000:00:00");

				ttimestr[0]+=(hour/1000) % 10;
				ttimestr[1]+=(hour/100) % 10;
				ttimestr[2]+=(hour/10) % 10;
				ttimestr[3]+=(hour/1) % 10;

				ttimestr[5]+=(min/10) % 10;
				ttimestr[6]+=(min/1) % 10;

				ttimestr[8]+=(sec/10) % 10;
				ttimestr[9]+=(sec/1) % 10;
			}

			MENU_CreateTextTexture(work->strman,x,y,TIME_WIDTH,TIME_HEIGHT,0,0,0,ttimestr);
		}
	}
}

static void DispFileInfo(Work *work,int alpha)
{
	int disp_width;
	int color=TEXTCOLOR_NOSEL | (alpha<<24);

	if(work->update_fileinfo){
		int id=work->indexes[work->file_cursor];
		MCMAN_INFODATA *info=MCManGetFileInfo(id);
		int stage_num=info->stage_num;
		char *str;
		char buf[64];

		work->i_fileinfo^=1;

		if(MCManCheckExactFileFlag(id)<0){
			str=DAMAGED_FILE;
		}
		else {
			if( work->mcman.file_kind == MCMAN_FILE_KIND_VR ) {
				MCMAN_VR_INFODATA vrinfo;
				info=MCManGetFileInfo(id);
				DecodeVRInfo( &vrinfo, info );
				GV_ZeroMemory( buf, sizeof( buf ) );
				str = buf;
				strcpy( buf, VR_TRAINING_FILE );
				memcpy( buf + strlen( buf ), vrinfo.name, MCMAN_VR_NAME_MAX );
			} else {
				if(stage_num>=0 && stage_num<MC_N_STAGES){
printf("o#####\n");
					str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(STAGENAME_RESOURCE,stage_num));
printf("#####\n");
				}
				else{
					str="NO STAGENAME";
				}
			}
		}

		if(work->i_fileinfo){
			MENU_ClearPartTextTexture(work->strman,LOADRES_FILEINFO_V2,LOADRES_FILEINFO_HEIGHT2);

			work->fileinfo_width
				=MENU_CreateTextTexture(work->strman,
										LOADRES_FILEINFO_U2,LOADRES_FILEINFO_V2,
										LOADRES_FILEINFO_WIDTH2,LOADRES_FILEINFO_HEIGHT2,
										0,0,0,str);
		}
		else{
			MENU_ClearPartTextTexture(work->strman,LOADRES_FILEINFO_V,LOADRES_FILEINFO_HEIGHT);

			work->fileinfo_width
				=MENU_CreateTextTexture(work->strman,
										LOADRES_FILEINFO_U,LOADRES_FILEINFO_V,
										LOADRES_FILEINFO_WIDTH,LOADRES_FILEINFO_HEIGHT,
										0,0,0,str);
		}

		work->update_fileinfo=0;
	}

	disp_width=work->fileinfo_width*FONT_DISP_WIDTH/FONT_WIDTH;

	if(work->i_fileinfo){
		MENU_PutTextScreenNoTR(work->strman,
							   LOADRES_FILEINFO_X,
							   LOADRES_FILEINFO_Y,
							   LOADRES_FILEINFO_X+disp_width,
							   LOADRES_FILEINFO_Y+LOADRES_FILEINFO_H,
							   LOADRES_FILEINFO_U2,
							   LOADRES_FILEINFO_V2,
							   LOADRES_FILEINFO_U2+work->fileinfo_width,
							   LOADRES_FILEINFO_V2+LOADRES_FILEINFO_HEIGHT2,
							   color);
	}
	else{
		MENU_PutTextScreenNoTR(work->strman,
							   LOADRES_FILEINFO_X,
							   LOADRES_FILEINFO_Y,
							   LOADRES_FILEINFO_X+disp_width,
							   LOADRES_FILEINFO_Y+LOADRES_FILEINFO_H,
							   LOADRES_FILEINFO_U,
							   LOADRES_FILEINFO_V,
							   LOADRES_FILEINFO_U+work->fileinfo_width,
							   LOADRES_FILEINFO_V+LOADRES_FILEINFO_HEIGHT,
							   color);
	}
}

static void CreateListLoading(Work *work)
{
	MENU_ClearPartTextTexture(work->strman,LISTLOAD_V,LISTLOAD_HEIGHT);

	MENU_CreateTextTexture(work->strman,LISTLOAD_U,LISTLOAD_V,
						   LISTLOAD_WIDTH,LISTLOAD_HEIGHT,0,0,0,"Loading. Please wait.");
}

static void DispListLoading(Work *work)
{
	MENU_PutTextScreenNoTR(work->strman,
						   LISTLOAD_X,LISTLOAD_Y,LISTLOAD_X+LISTLOAD_W,LISTLOAD_Y+LISTLOAD_H,
						   LISTLOAD_U,LISTLOAD_V,LISTLOAD_U+LISTLOAD_WIDTH,LISTLOAD_V+LISTLOAD_HEIGHT,
						   TEXTCOLOR);
}


/* ------------------------------------------------------------------------ */

static void SetFileYPos0(Work *work,int index,int y_pos)
{
	work->file_y_pos[index]=y_pos;
	work->file_t_y_pos[index]=y_pos;
}

static void SetFileYPos(Work *work,int index,int y_pos)
{
	work->file_t_y_pos[index]=y_pos;
}

static void SetFileYPosAll0(Work *work)
{
	int i;
	for(i=0;i<N_FILES;i++){
		SetFileYPos0(work,i,ID_Y+ID_LINE_HEIGHT*i);
	}
}

static void SetFileYPosAll1(Work *work)
{
	int i;
	for(i=0;i<N_FILES;i++){
		SetFileYPos(work,i,ID_Y+ID_LINE_HEIGHT*3);
	}
}

static void SetFileYPosAll2(Work *work)
{
	int i;
	for(i=0;i<N_FILES;i++){
		SetFileYPos(work,i,ID_Y+ID_LINE_HEIGHT*i);
	}
}


/* ------------------------------------------------------------------------ */

static void SetFileAlpha0(Work *work,int index,int alpha)
{
	work->file_alpha[index]=alpha;
	work->file_t_alpha[index]=alpha;
}

static void SetFileAlpha(Work *work,int index,int alpha)
{
	work->file_t_alpha[index]=alpha;
}

static void SetFileAlphaAll0(Work *work,int alpha0,int alpha1)
{
	int i;
	for(i=0;i<N_FILES;i++){
		if(i==work->file_cursor) SetFileAlpha0(work,i,alpha0);
		else SetFileAlpha0(work,i,alpha1);
	}
}

static void SetFileAlphaAll1(Work *work,int alpha0,int alpha1)
{
	int i;
	for(i=0;i<N_FILES;i++){
		if(i==work->file_cursor) SetFileAlpha(work,i,alpha0);
		else SetFileAlpha(work,i,alpha1);
	}
}


/* ------------------------------------------------------------------------ */

static void SetCursorYPos0(Work *work)
{
	float y=TEXT_Y+TEXT_LINE_HEIGHT*work->file_cursor+CURSOR_ADJUST;
	work->cursor_t_y_pos=work->cursor_y_pos=y;
}

static void SetCursorYPos(Work *work)
{
	float y=TEXT_Y+TEXT_LINE_HEIGHT*work->file_cursor+CURSOR_ADJUST;
	work->cursor_t_y_pos=y;
}


/* ------------------------------------------------------------------------ */

static void SetPage(Work *work)
{
	char str[PAGE_STR_LEN];
	int i,j;
	float d;

	if( MCMAN_GetDataFileMax( work->mcman.file_kind ) <= N_FILES ) return;

	strcpy(str,"PAGE");

	SK_PrintfNormal(NULL,work->page_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,PAGE_STR_LEN);

	for(i=0;i<PAGE_STR_LEN;i++){
		SPR_OBJ *spr=work->page_str[i];

		spr->sprite.pos.x=PAGE_STR_X;
		spr->sprite.pos.y=PAGE_STR_Y;
		spr->sprite.dw=PAGE_STR_WIDTH;
		spr->sprite.dh=PAGE_STR_HEIGHT;
	}

	SK_PrintfNormal(str,work->page_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,PAGE_STR_LEN);
	SK_PrintfChengColor2(work->page_str,TEXT_R,TEXT_G,TEXT_B,work->page_alpha,PAGE_STR_LEN);
	SK_AllShow(work->page_str,PAGE_STR_LEN);


	i=0;
	if(work->page_cursor+1>=100){
		str[i]='0'+(((work->page_cursor+1)/100) % 10);
		i++;
	}
	str[i]='0'+(((work->page_cursor+1)/10) % 10);
	i++;
	str[i]='0'+((work->page_cursor+1) % 10);
	i++;
	str[i]='/';
	j=i;
	i++;
	if(work->n_pages>=100){
		str[i]='0'+((work->n_pages/100) % 10);
		i++;
	}
	str[i]='0'+((work->n_pages/10) % 10);
	i++;
	str[i]='0'+(work->n_pages % 10);
	i++;
	str[i]='\0';

	SK_PrintfNormal(NULL,work->page_num_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,PAGE_NUM_STR_LEN);

	for(i=0;i<PAGE_NUM_STR_LEN;i++){
		SPR_OBJ *spr=work->page_num_str[i];

		spr->sprite.pos.x=PAGE_NUM_STR_X;
		spr->sprite.pos.y=PAGE_NUM_STR_Y;
		spr->sprite.dw=SK_FONT_WIDTH;
		spr->sprite.dh=SK_FONT_HEIGHT;
	}

	SK_PrintfNormal(str,work->page_num_str,work->base_u,work->base_v,
					STR_WIDTH,STR_HEIGHT,PAGE_STR_LEN);
	SK_PrintfChengColor2(work->page_num_str,TEXT_R,TEXT_G,TEXT_B,work->page_alpha,PAGE_NUM_STR_LEN);
	SK_AllShow(work->page_num_str,PAGE_NUM_STR_LEN);

	i=j;
	d=PAGE_NUM_STR_X-work->page_num_str[j]->sprite.pos.x;
	for(i=0;i<PAGE_NUM_STR_LEN;i++){
		SPR_OBJ *spr=work->page_num_str[i];
		spr->sprite.pos.x+=d;
	}
}

static void PageAlphaSet(Work *work,int alpha,int count)
{
	work->page_talpha=alpha;
	work->page_alpha_count=count;
}

static void PageAlphaCtrl(Work *work)
{
	if(work->page_alpha_count>0){
		work->page_alpha+=(work->page_talpha-work->page_alpha)/work->page_alpha_count;
		work->page_alpha_count--;
	}
	SK_PrintfChengColor2(work->page_str,TEXT_R,TEXT_G,TEXT_B,work->page_alpha,PAGE_STR_LEN);
	SK_PrintfChengColor2(work->page_num_str,TEXT_R,TEXT_G,TEXT_B,work->page_alpha,PAGE_NUM_STR_LEN);
}

static void HidePage(Work *work)
{
	work->page_alpha_count=0;
	work->page_alpha=0;
	work->page_talpha=0;
	SK_AllHide(work->page_str,PAGE_STR_LEN);
	SK_AllHide(work->page_num_str,PAGE_NUM_STR_LEN);
}


/* ------------------------------------------------------------------------ */

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


/* ------------------------------------------------------------------------ */
static void CreateConflictMessage(Work *work)
{
   char const * const cloudConflictStr = GetTransfarringString(kTString_ConflictLoadMessage);

   MENU_ClearPartTextTexture(work->strman, TRANSFARRING_TEXT_V, TRANSFARRING_TEXT_HEIGHT);

   MENU_CreateTextTexture(work->strman, TRANSFARRING_TEXT_U, TRANSFARRING_TEXT_V,
      TRANSFARRING_TEXT_WIDTH, TRANSFARRING_TEXT_HEIGHT, 0, 0, 0, (char*)cloudConflictStr);
}

static void CreateTrophiesDisabledMessage(Work *work)
{
   char const * const cloudConflictStr = GetTransfarringString(kString_TrophiesDisabled);

   MENU_ClearPartTextTexture(work->strman, TRANSFARRING_TEXT_V, TRANSFARRING_TEXT_HEIGHT);

   MENU_CreateTextTexture(work->strman, TRANSFARRING_TEXT_U, TRANSFARRING_TEXT_V,
      TRANSFARRING_TEXT_WIDTH, TRANSFARRING_TEXT_HEIGHT, 0, 0, 0, (char*)cloudConflictStr);
}

static void DispFilename(Work *work)
{
	int i;

   if(!BP_Network_IsSignedIn())
   {
      work->wasSignedIn = 0;
   }

   if(!work->mHidePsnWidget && !work->mDogTagLoad)
   {
      Transfarring_RenderPsnConnectionSprite(work->strman, TRANSFARRING_PSN_X, TRANSFARRING_PSN_Y,
         BP_Network_IsSignedIn() ? kTransfarring_PSN_Connected : kTransfarring_PSN_Disconnected);
   }

	if(work->file_anim_count>0){
		work->cursor_y_pos+=(work->cursor_t_y_pos-work->cursor_y_pos)/work->file_anim_count;

		for(i=0;i<N_FILES;i++){
			work->file_y_pos[i]
				+=(work->file_t_y_pos[i]-work->file_y_pos[i])/work->file_anim_count;
			work->file_alpha[i]
				+=(work->file_t_alpha[i]-work->file_alpha[i])/work->file_anim_count;
		}
		work->file_anim_count--;
	}

	work->cursor->empty.pos.y=work->cursor_y_pos;

#if 1
	for(i=0;i<work->n_indexes;i++){
#else
	for(i=0;i<N_FILES;i++){
#endif
		int x1,y1,x2,y2;
		int u1,v1,u2,v2;
		int col;
		int id=work->indexes[i];

      ESaveType saveType = Transfarring_GetTransfarringType(work->mcman.file_kind);

      int isLocked = Transfarring_IsFileLocked(saveType, id);
      int isCloud = Transfarring_IsFileCloud(saveType, id);
      int isLinked = Transfarring_IsFileLinked(saveType, id);

		if(MCManCheckExactFileFlag(id)<0){
			col=(work->file_alpha[i]<<24) | TEXTCOLOR_NOSEL;
		}
		else{
			MCMAN_INFODATA *info;
			col=(work->file_alpha[i]<<24) | TEXTCOLOR_NOSEL;
			
			if( work->mcman.file_kind == MCMAN_FILE_KIND_GAME )
         {
            info=(MCMAN_INFODATA *)MCManGetFileInfo(id);
				if(info->clear_count > 0)
            {
					col=(work->file_alpha[i]<<24) | TEXTCOLOR_CLEARED;
				}
			}
		}

      //Transfarring
      if(isLocked)
      {
         if(work->file_alpha[i] <= UNSEL_ALPHA)
         {
            col = TRANSFARRING_TEXT_COLOR | (work->file_alpha[i] << 24);
         }
         else
         {
            col = TRANSFARRING_SELECTED_TEXT_COLOR;
         }

         Transfarring_RenderIconSprite(work->strman, TRANSFARRING_LOCK_X, work->file_y_pos[i] + TRANSFARRING_LOCK_Y_OFFSET,
            TRANSFARRING_LOCK_COLOR | (work->file_alpha[i] << 24), kTransfarring_LockIcon, kSaveLoadScreen_Normal);
      }
      else if(isCloud)
      {
         Transfarring_RenderIconSprite(work->strman, TRANSFARRING_LOCK_X, work->file_y_pos[i] + TRANSFARRING_LOCK_Y_OFFSET,
            TRANSFARRING_CLOUD_COLOR | (work->file_alpha[i] << 24), kTransfarring_CloudIcon, kSaveLoadScreen_Normal);
      }
      else if(isLinked)
      {
         Transfarring_RenderIconSprite(work->strman, TRANSFARRING_LOCK_X, work->file_y_pos[i] + TRANSFARRING_LOCK_Y_OFFSET,
            TRANSFARRING_LINK_COLOR | (work->file_alpha[i] << 24), kTransfarring_LinkIcon, kSaveLoadScreen_Normal);
      }

		if( MCMAN_GetDataFileMax( work->mcman.file_kind ) != 1 ) {
			x1=ID_X;
			y1=work->file_y_pos[i];
			x2=x1+ID_W;
			y2=y1+ID_H;

			u1=ID_U;
			v1=ID_V+ID_HEIGHT*i;
			u2=u1+ID_WIDTH;
			v2=v1+ID_HEIGHT;

			MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);
		}

      x1=TEXT_X;
		y1=work->file_y_pos[i];
		x2=x1+TEXT_W;
		y2=y1+TEXT_H;

		u1=TEXT_U;
		v1=TEXT_V+TEXT_HEIGHT*i;
		u2=u1+TEXT_WIDTH;
		v2=v1+TEXT_HEIGHT;

		MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);

		x1=DATE_X;
		y1=work->file_y_pos[i];
		x2=x1+DATE_W;
		y2=y1+DATE_H;

		u1=DATE_U;
		v1=DATE_V+DATE_HEIGHT*i;
		u2=u1+DATE_WIDTH;
		v2=v1+DATE_HEIGHT;

		MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);

		x1=TIME_X;
		y1=work->file_y_pos[i];
		x2=x1+TIME_W;
		y2=y1+TIME_H;

		u1=TIME_U;
		v1=TIME_V+TIME_HEIGHT*i;
		u2=u1+TIME_WIDTH;
		v2=v1+TIME_HEIGHT;

		MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);
	}

	if(work->n_indexes) DispFileInfo(work,work->file_alpha[work->file_cursor]);
}

static void DispConfirm(Work *work)
{
	// HideSlot(work);

   if(work->mCloudConflictFlag)
   {
      MENU_PutTextScreenNoTR(work->strman, TRANSFARRING_TEXT_X, TRANSFARRING_TEXT_Y,
         TRANSFARRING_TEXT_X + TRANSFARRING_TEXT_W, TRANSFARRING_TEXT_Y + TRANSFARRING_TEXT_H,
         TRANSFARRING_TEXT_U, TRANSFARRING_TEXT_V, TRANSFARRING_TEXT_U + TRANSFARRING_TEXT_WIDTH,
         TRANSFARRING_TEXT_V + TRANSFARRING_TEXT_HEIGHT, TEXTCOLOR);
   }
   else
   {
      MENU_PutTextScreenNoTR(work->strman,LOADRES_CONFIRM_X,LOADRES_CONFIRM_Y,
         LOADRES_CONFIRM_X+LOADRES_CONFIRM_W,LOADRES_CONFIRM_Y+LOADRES_CONFIRM_H,
         LOADRES_CONFIRM_U,LOADRES_CONFIRM_V,
         LOADRES_CONFIRM_U+LOADRES_CONFIRM_WIDTH,
         LOADRES_CONFIRM_V+LOADRES_CONFIRM_HEIGHT,
         TEXTCOLOR);

   }
	
#if 0 //yano
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	{
		int yes_x,slash_x,no_x;
		int yescol,nocol,slashcol;

		if(work->yesno_cursor){
			yescol=TEXTCOLOR_NOSEL|(UNSEL_ALPHA<<24);
			nocol=TEXTCOLOR_NOSEL|(SEL_ALPHA<<24);
		}
		else{
			yescol=TEXTCOLOR_NOSEL|(SEL_ALPHA<<24);
			nocol=TEXTCOLOR_NOSEL|(UNSEL_ALPHA<<24);
		}
		slashcol=TEXTCOLOR;

		no_x=YESNO_STR_RX-work->no_width*FONT_DISP_WIDTH/FONT_WIDTH;
		slash_x=no_x-work->slash_width*FONT_DISP_WIDTH/FONT_WIDTH;
		yes_x=slash_x-work->yes_width*FONT_DISP_WIDTH/FONT_WIDTH;

		MENU_PutTextScreen(work->strman,yes_x,MESSAGE_Y,
						   yes_x+YES_W,MESSAGE_Y+YES_H,
						   YES_U,YES_V,
						   YES_U+YES_WIDTH,YES_V+YES_HEIGHT,
						   yescol);

		MENU_PutTextScreen(work->strman,slash_x,MESSAGE_Y,
						   slash_x+SLASH_W,MESSAGE_Y+SLASH_H,
						   SLASH_U,SLASH_V,
						   SLASH_U+SLASH_WIDTH,SLASH_V+SLASH_HEIGHT,
						   slashcol);

		MENU_PutTextScreen(work->strman,no_x,MESSAGE_Y,
						   no_x+NO_W,MESSAGE_Y+NO_H,
						   NO_U,NO_V,
						   NO_U+NO_WIDTH,NO_V+NO_HEIGHT,
						   nocol);
	}
#endif
#else
	{
		int yes_x,slash_x,no_x;
		int yescol,nocol,slashcol;

		if(work->yesno_cursor){
			yescol=TEXTCOLOR_NOSEL|(UNSEL_ALPHA<<24);
			nocol=TEXTCOLOR_NOSEL|(SEL_ALPHA<<24);
		}
		else{
			yescol=TEXTCOLOR_NOSEL|(SEL_ALPHA<<24);
			nocol=TEXTCOLOR_NOSEL|(UNSEL_ALPHA<<24);
		}
		slashcol=TEXTCOLOR;

		no_x=YESNO_STR_RX-work->no_width*FONT_DISP_WIDTH/FONT_WIDTH;
		slash_x=no_x-work->slash_width*FONT_DISP_WIDTH/FONT_WIDTH;
		yes_x=slash_x-work->yes_width*FONT_DISP_WIDTH/FONT_WIDTH;

		MENU_PutTextScreen(work->strman,yes_x,MESSAGE_Y,
						   yes_x+YES_W,MESSAGE_Y+YES_H,
						   YES_U,YES_V,
						   YES_U+YES_WIDTH,YES_V+YES_HEIGHT,
						   yescol);

		MENU_PutTextScreen(work->strman,slash_x,MESSAGE_Y,
						   slash_x+SLASH_W,MESSAGE_Y+SLASH_H,
						   SLASH_U,SLASH_V,
						   SLASH_U+SLASH_WIDTH,SLASH_V+SLASH_HEIGHT,
						   slashcol);

		MENU_PutTextScreen(work->strman,no_x,MESSAGE_Y,
						   no_x+NO_W,MESSAGE_Y+NO_H,
						   NO_U,NO_V,
						   NO_U+NO_WIDTH,NO_V+NO_HEIGHT,
						   nocol);
	}
#endif

}

static void DispLoading(Work *work)
{
	// HideSlot(work);

   MENU_PutTextScreenNoTR(work->strman,LOADRES_LOADING_X,LOADRES_LOADING_Y,
      LOADRES_LOADING_X+LOADRES_LOADING_W,LOADRES_LOADING_Y+LOADRES_LOADING_H,
      LOADRES_LOADING_U,LOADRES_LOADING_V,
      LOADRES_LOADING_U+LOADRES_LOADING_WIDTH,
      LOADRES_LOADING_V+LOADRES_LOADING_HEIGHT,
      TEXTCOLOR);
}

static void DispSuccess(Work *work)
{
	// HideSlot(work);

   if(work->mDisplayTrophiesDisabled)
   {
      MENU_PutTextScreenNoTR(work->strman, TRANSFARRING_TEXT_X, TRANSFARRING_TEXT_Y,
         TRANSFARRING_TEXT_X + TRANSFARRING_TEXT_W, TRANSFARRING_TEXT_Y + TRANSFARRING_TEXT_H,
         TRANSFARRING_TEXT_U, TRANSFARRING_TEXT_V, TRANSFARRING_TEXT_U + TRANSFARRING_TEXT_WIDTH,
         TRANSFARRING_TEXT_V + TRANSFARRING_TEXT_HEIGHT, TEXTCOLOR);
   }
   else
   {
      MENU_PutTextScreenNoTR(work->strman,LOADRES_SUCCESS_X,LOADRES_SUCCESS_Y,
         LOADRES_SUCCESS_X+LOADRES_SUCCESS_W,LOADRES_SUCCESS_Y+LOADRES_SUCCESS_H,
         LOADRES_SUCCESS_U,LOADRES_SUCCESS_V,
         LOADRES_SUCCESS_U+LOADRES_SUCCESS_WIDTH,
         LOADRES_SUCCESS_V+LOADRES_SUCCESS_HEIGHT,
         TEXTCOLOR);
   }
}

static void DispFailed(Work *work)
{
	// HideSlot(work);

	MENU_PutTextScreenNoTR(work->strman,LOADRES_FAILED_X,LOADRES_FAILED_Y,
						   LOADRES_FAILED_X+LOADRES_FAILED_W,LOADRES_FAILED_Y+LOADRES_FAILED_H,
						   LOADRES_FAILED_U,LOADRES_FAILED_V,
						   LOADRES_FAILED_U+LOADRES_FAILED_WIDTH,
						   LOADRES_FAILED_V+LOADRES_FAILED_HEIGHT,
						   TEXTCOLOR);
}

/* ------------------------------------------------------------------------ */

static int FileCheckSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		{
			int title_mode=0;
			if(work->finish_mode) title_mode=4;
         GV_SetActorChild(work, NewPortsel(PORTSEL_NAME,work->name,work->l2d_handle, title_mode,work->start_mode,
            work->strman,LOADGAME_RESOURCE,&work->mcman));

         work->mSyncSlotsRemaining = 0;
    	}
		work->sub_step++;
		break;
	case 1:
      {
         DisplaySynchingMessage(work);
      }
      break;
	}

	return 0;
}

enum {
   FILESEL_BEGINSYNC=-2,
   FILESEL_DOSYNC=-1,
	FILESEL_INIT=0,

#if 1
	FILESEL_START_GETINFO,
	FILESEL_WAIT_GETINFO,
#endif

	FILESEL_FADEIN,
	FILESEL_MAIN,
#if 0
	FILESEL_FADEOUT_AND_START_GETINFO,
#endif
	FILESEL_FADEOUT,
	FILESEL_BLACK,
#if 1
	FILESEL_WAIT_PAGELOAD,
#endif
	FILESEL_START_FADEIN,

	FILESEL_ERROR=0x20,
};

static void FileSelErrorEnd(Work *work)
{
	work->action_strcode=CODE_CLOSELIST;

	work->file_anim_count=FILE_ALPHA_TIME;
	SetFileAlphaAll1(work,0,0);
	PageAlphaSet(work,0,FILE_ALPHA_TIME);

	work->sub_step=FILESEL_ERROR;
}

static int FileSelSubStep(Work *work)
{
#if 0
	SPR_OBJ *rarrow,*larrow;

	rarrow=L2D_GetObject(work->l2d_handle,CODE_RARROW);
	larrow=L2D_GetObject(work->l2d_handle,CODE_LARROW);
#endif

	if(MCManChecked()){
		if(MCManIsMCChanged(work->port_sel)){
			work->mcchanged_flags=1;
		}
	}

#if defined(BP_VITA)
   if( !work->mDogTagLoad && work->sub_step == FILESEL_MAIN && BP_Network_IsSignedIn()
      && ( !work->wasSignedIn || ( work->mSystemResumeCount != Transfarring_GetSystemResumeCount() ) ) )
   {
      work->wasSignedIn = 1;
      work->mSystemResumeCount = Transfarring_GetSystemResumeCount();
      // Normally, syncing is done in portsel, but we can also sync here if we connect to PSN
      work->sub_step = FILESEL_BEGINSYNC;
   }
#else
   if( !work->mDogTagLoad && work->sub_step == FILESEL_MAIN && BP_Network_IsSignedIn() && !work->wasSignedIn )
   {
      work->wasSignedIn = 1;
      // Normally, syncing is done in portsel, but we can also sync here if we connect to PSN
      work->sub_step = FILESEL_BEGINSYNC;
   }
#endif

	switch(work->sub_step){
   case FILESEL_BEGINSYNC:
      {
      STransfarringError error = Transfarring_PostWork_SyncCloudFiles( &work->mcman, 0, 0 );
      if( error.mHighLevelError == kTE_Success )
      {
         work->sub_step = FILESEL_DOSYNC;
         work->start_index=0;
         work->file_cursor=0;
         work->page_cursor=0;
         HidePage( work );
         HideCursor( work );
         work->action_strcode=CODE_CLOSELIST;
      }
      else
      {
         work->sub_step = FILESEL_INIT;
      }
      }
      break;
   case FILESEL_DOSYNC:
      {
         STransfarringError error = Transfarring_ProcessWork();
         
         DisplaySynchingMessage(work);
         
         switch( error.mHighLevelError )
         {
         case kTE_Success:
            work->sub_step = FILESEL_INIT;
            break;
         case kTE_Processing:
            break;
         default:
            if(error.mLowLevelError)
            {
               BP_CommonDialog_WantsErrorDialog(error.mLowLevelError);
            }
            else
            {
               BP_CommonDialog_WantsMessageDialog(GetTransfarringStringForErrorCode(error.mHighLevelError), kMDL_OK);
            }
            work->sub_step = FILESEL_INIT;
            break;
         }
      }
      break;
	case FILESEL_INIT:
		work->start_index=0;
		SearchUpFiles(work);

		work->file_cursor=0;
		work->page_cursor=0;
		work->n_pages=(MCManExactNFiles()+N_FILES-1)/N_FILES;

		{
			int id;

			id=SearchLatestFile();

			// printf("ID = %d\n",id);
			// if(id==-1) ASSERT(0);

			while(id>work->indexes[work->n_indexes-1]){
				work->page_cursor++;

				work->start_index=work->indexes[N_FILES-1]+1;
				SearchUpFiles(work);
			}
			while(id!=work->indexes[work->file_cursor]) work->file_cursor++;
		}

		// printf("cursor = %d %d\n",work->page_cursor,work->file_cursor);

		HidePage(work);

		work->sub_step++;

#if 1
	case FILESEL_START_GETINFO:
		if(work->mcchanged_flags) return -1;

		if((work->page_cursor<<1)<work->n_pages){
			/* 正方向 */
			if(GetGameInfoOnlyBGStart(work->port_sel)){
				work->sub_step++;
			}
			work->search_dir=0;
		}
		else{
			/* 逆方向 */
			if(GetGameInfoOnlyRBGStart(work->port_sel)){
				work->sub_step++;
			}
			work->search_dir=1;
		}
		break;
	case FILESEL_WAIT_GETINFO:
		if(work->mcchanged_flags) return -1;

		switch(MCManGetResult()){
		case 1:
			work->sub_step++;
			if(work->port_sel){
				work->action_strcode=CODE_OPENSELFILE2;
			}
			else{
				work->action_strcode=CODE_OPENSELFILE1;
			}
			break;
		case -1:
		case -2:
			return -1;
		}
		break;
#endif

	case FILESEL_FADEIN:
		if(work->mcchanged_flags){
			work->action_strcode=CODE_CLOSELIST;
			return -1;
		}

		if(work->busy_flag) break;

		printf("o?????\n");

		CreateFilenameTexture(work);
		work->update_fileinfo=1;

		work->file_anim_count=FILE_ALPHA_TIME;
		SetFileYPosAll0(work);
		SetFileAlphaAll0(work,0,0);
		SetFileAlphaAll1(work,SEL_ALPHA,UNSEL_ALPHA);
		SetCursorYPos0(work);
		SetPage(work);
		PageAlphaSet(work,SEL_ALPHA,FILE_ALPHA_TIME);

	case FILESEL_MAIN:
		work->sub_step=FILESEL_MAIN;

#if 0
		rarrow->sprite.col.a=UNSEL_ALPHA;
		larrow->sprite.col.a=UNSEL_ALPHA;
#endif

		DispFilename(work);

		if(work->file_anim_count>0) break;

		if(work->n_indexes)
      {
			LocalShowCursor(work);
		}

		if(work->mcchanged_flags)
      {
			FileSelErrorEnd(work);
			return 0;
		}

		if(work->key_press & PAD_CANCEL)
      {
			SE_CANCEL();
			FileSelErrorEnd(work);
			return 0;
		}
		else if((work->key_press & PAD_OK)
         && work->n_indexes
         &&	MCManCheckExactFileFlag(work->indexes[work->file_cursor]) > 0)
      {
         if(Transfarring_IsFileLocked(Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor]))
         {
            SE_CANCEL();
         }
         else
         {
            HideCursor(work);
            SE_OK();
            work->file_anim_count=FILE_ALPHA_TIME;
            SetFileYPosAll1(work);
            SetFileAlphaAll1(work,SEL_ALPHA,0);
            return 1;
         }
		}
      else if(!work->mHidePsnWidget && !work->mDogTagLoad && (work->key_press & PAD_X) && !BP_Network_IsSignedIn())
      {
         BP_TUS_WantsSignin();
         return 0;
      }
		switch(work->key_autostatus & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1)){
		case PAD_L:
		case PAD_L1:
			work->page_cursor--;
			if(work->page_cursor<0){
				work->page_cursor=0;
			}
			else{
				work->sub_step++;
				work->start_index=work->indexes[0]-1;
				SearchDownFiles(work);

				// larrow->sprite.col.a=SEL_ALPHA;

				work->file_anim_count=PAGE_MOVE_TIME;
				SetFileAlphaAll1(work,0,0);
				PageAlphaSet(work,0,PAGE_MOVE_TIME);

				SE_SEL();
				return 0;
			}
			break;
		case PAD_R:
		case PAD_R1:
			work->page_cursor++;
			if(work->page_cursor>=work->n_pages){
				work->page_cursor=work->n_pages-1;
			}
			else{
				work->sub_step++;
				work->start_index=work->indexes[N_FILES-1]+1;
				SearchUpFiles(work);

				if(work->file_cursor>=work->n_indexes){
					work->file_cursor=work->n_indexes-1;
					SetCursorYPos(work);
				}

				// rarrow->sprite.col.a=SEL_ALPHA;

				work->file_anim_count=PAGE_MOVE_TIME;
				SetFileAlphaAll1(work,0,0);
				PageAlphaSet(work,0,PAGE_MOVE_TIME);

				SE_SEL();
				return 0;
			}
			break;
		case PAD_U:
			{
				int cur0,cur1;

				cur0=work->file_cursor;
				work->file_cursor--;
				cur1=work->file_cursor;

				if(work->file_cursor<0){
					work->file_cursor=N_FILES-1;

					work->page_cursor--;
					if(work->page_cursor<0){
						work->page_cursor=0;
						work->file_cursor=0;
					}
					else{
						work->sub_step++;
						work->start_index=work->indexes[0]-1;
						SearchDownFiles(work);

						work->file_anim_count=PAGE_MOVE_TIME;
						SetFileAlphaAll1(work,0,0);
						PageAlphaSet(work,0,PAGE_MOVE_TIME);

						SE_SEL();
						return 0;
					}
				}
				else{
					work->file_anim_count=CURSOR_MOVE_TIME;
					SetFileAlpha(work,cur0,UNSEL_ALPHA);
					SetFileAlpha(work,cur1,SEL_ALPHA);
					SetCursorYPos(work);

					work->update_fileinfo=1;

					SE_SEL();
				}
			}
			break;
		case PAD_D:
			{
				int cur0,cur1;

				cur0=work->file_cursor;
				work->file_cursor++;
				cur1=work->file_cursor;

				if(work->file_cursor>=N_FILES){
					work->file_cursor=0;

					work->page_cursor++;
					if(work->page_cursor>=work->n_pages){
						work->page_cursor=work->n_pages-1;
						work->file_cursor=N_FILES-1;
					}
					else{
						work->sub_step++;
						work->start_index=work->indexes[N_FILES-1]+1;
						SearchUpFiles(work);

						work->file_anim_count=PAGE_MOVE_TIME;
						SetFileAlphaAll1(work,0,0);
						PageAlphaSet(work,0,PAGE_MOVE_TIME);

						SE_SEL();
						return 0;
					}
				}
				else if(work->file_cursor>=work->n_indexes){
					work->file_cursor=work->n_indexes-1;
				}
				else{
					work->file_anim_count=CURSOR_MOVE_TIME;
					SetFileAlpha(work,cur0,UNSEL_ALPHA);
					SetFileAlpha(work,cur1,SEL_ALPHA);
					SetCursorYPos(work);

					work->update_fileinfo=1;

					SE_SEL();
				}
			}
			break;
		}
		break;

#if 0
	case FILESEL_FADEOUT_AND_START_GETINFO:
		HideCursor(work);
		DispFilename(work);

		if(work->mcchanged_flags){
			work->action_strcode=CODE_CLOSELIST;
			work->sub_step=FILESEL_ERROR;
			return 0;
		}

		if(GetGamePageInfoStart(work->port_sel,work->indexes[0],N_FILES)){
			work->sub_step++;
		}
		break;
#endif

	case FILESEL_FADEOUT:
		HideCursor(work);
		DispFilename(work);

		if(work->file_anim_count>0) break;

		work->sub_step++;
		break;

	case FILESEL_BLACK:
		work->sub_step++;
		work->timer=0;
		break;

#if 1
	case FILESEL_WAIT_PAGELOAD:
		if(work->timer==LISTLOAD_COUNT) CreateListLoading(work);
		else if(work->timer>LISTLOAD_COUNT) DispListLoading(work);
		work->timer++;

		if(work->mcchanged_flags){
			work->action_strcode=CODE_CLOSELIST;
			return -1;
		}

#if 0
		switch(MCManGetResult()){
		case 1:
			work->sub_step++;
			break;
		case -1:
		case -2:
			work->action_strcode=CODE_CLOSELIST;
			return -1;
		}
#else
		if(work->search_dir){
			/* 逆方向 */
			if(MCManCheckingID()<work->indexes[0]){
				work->sub_step++;
			}
		}
		else{
			/* 正方向 */
			if(MCManCheckingID()>work->indexes[work->n_indexes-1]){
				work->sub_step++;
			}
		}
#endif
		break;
#endif

	case FILESEL_START_FADEIN:
		printf("oo?????\n");
		CreateFilenameTexture(work);
		work->update_fileinfo=1;

	case 0x10:
		work->file_anim_count=PAGE_MOVE_TIME;
		SetFileYPosAll2(work);
		SetFileAlphaAll1(work,SEL_ALPHA,UNSEL_ALPHA);
		SetCursorYPos0(work);
		SetPage(work);
		PageAlphaSet(work,SEL_ALPHA,PAGE_MOVE_TIME);

		work->sub_step=FILESEL_MAIN;

		DispFilename(work);

		break;

	case FILESEL_ERROR:
		HideCursor(work);
		DispFilename(work);

		if(work->file_anim_count>0) break;

		HidePage(work);

		return -1;
	}

	PageAlphaCtrl(work);

	return 0;
}

static int ConfirmSubStep(Work *work)
{
	DispFilename(work);
	PageAlphaCtrl(work);

	if(MCManChecked()){
		if(MCManIsMCChanged(work->port_sel)){
			work->mcchanged_flags=1;
		}
	}

	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			// work->action_strcode=CODE_OPENCONFIRM;
			work->sub_step++;
		}
		break;
	case 1:
		work->yesno_cursor=1;
		work->sub_step++;
		break;
	case 2:
		if(!work->busy_flag && work->file_anim_count<=0)
      {
         CreateConflictMessage(work);
         work->sub_step++;
#if 0 //yano
#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
			SetYesNo(work);
#endif
#else
			//			SetYesNo(work);
#endif
		}
		break;
	case 3:
         DispConfirm(work);
#if 0 //yano
#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
		CurMoveYesNo(work);
#endif
#else
		CurMoveYesNo(work);
#endif

		if(work->mcchanged_flags){
			work->action_strcode=CODE_CLOSELIST;

			work->file_anim_count=FILE_ALPHA_TIME;
			SetFileAlphaAll1(work,0,0);
			PageAlphaSet(work,0,FILE_ALPHA_TIME);
			HideYesNo(work);

			work->sub_step=0x20;
			break;
		}

		if(work->key_press & PAD_CANCEL){
			// work->action_strcode=CODE_CLOSECONFIRM;

			HideYesNo(work);

			SE_CANCEL();
			return -1;
		}
		else if(work->key_press & PAD_OK){
			// work->action_strcode=CODE_CLOSECONFIRM;

			HideYesNo(work);

			if(work->yesno_cursor==0){
				SE_OK();
				return 1;
			}
			else{
				SE_CANCEL();
				return -1;
			}
		}

		// work->yesno[work->yesno_cursor]->sprite.col.a=UNSEL_ALPHA;

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

		// work->cursor->empty.pos.y=work->yesno[work->yesno_cursor]->sprite.pos.y+CURSOR_ADJUST;
		// work->yesno[work->yesno_cursor]->sprite.col.a=SEL_ALPHA;
		break;

	case 0x20:
		HideCursor(work);

		if(work->file_anim_count>0) break;

		HidePage(work);

		return -2;
	}

	return 0;
}

typedef enum
{
   kPSS_CloudConflictCheck = 0,
   kPSS_CloudSyncStart,
   kPSS_CloudSyncLoop,
   kPSS_LoadGameStart,
   kPSS_LoadGame,
}
EProcessSubStepStep;

static int ProcessSubStep(Work *work)
{
   char typeCode;

	DispFilename(work);
	DispLoading(work);
	PageAlphaCtrl(work);

	if(MCManChecked()){
		if(MCManIsMCChanged(work->port_sel)){
			work->mcchanged_flags=1;
		}
	}

   if (!Transfarring_CanLoadFile(Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor]))
   {
      return -1;
   }

	switch(work->sub_step)
   {
	case kPSS_CloudConflictCheck:
      {
         if(work->mCloudConflictFlag)
         {
            work->sub_step = kPSS_CloudSyncStart;
         }
         else if(Transfarring_HasCloudConflict(Transfarring_GetTransfarringType(work->mcman.file_kind),
            work->indexes[work->file_cursor]))
         {
            work->mCloudConflictFlag = 1;
            work->step = MCARD_CONFIRM;
            work->sub_step = 0;
         }
         else
         {
            work->sub_step = kPSS_LoadGameStart;
         }
      }
      break;
   case kPSS_CloudSyncStart:
      {
         if(work->mCloudConflictResolution == kCCR_UseLocalSaveFile)
         {
            work->mCloudConflictFlag = 0;
            work->sub_step = kPSS_LoadGameStart;
         }
         else
         {
            STransfarringError const error = Transfarring_PostWork_SyncSingleCloudFile(&work->mcman,
               Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor],
               work->mCloudConflictResolution,
               0 );

            if(error.mHighLevelError == kTE_Success)
            {
               work->sub_step = kPSS_CloudSyncLoop;         
            }
            else
            {
               work->mCloudConflictFlag = 0;
               return -1;
            }
         }
      }
      break;
   case kPSS_CloudSyncLoop:
      {
         STransfarringError const error = Transfarring_ProcessWork();
         
         if(error.mHighLevelError != kTE_Processing)
         {
            if(error.mHighLevelError == kTE_Success)
            {
               work->mCloudConflictFlag = 0;
               work->sub_step = kPSS_LoadGameStart;
            }
            else
            {
               work->mCloudConflictFlag = 0;
               return -1;
            }
         }
      }
      break;
   case kPSS_LoadGameStart:
		if(work->mcchanged_flags)
      {
			return -1;
		}
		if(LoadGameStart(work->port_sel,work->indexes[work->file_cursor],
						 &(work->savedata))){

			work->sub_step = kPSS_LoadGame;
		}
		break;
	case kPSS_LoadGame:
		if(work->mcchanged_flags)
      {
			return -1;
		}
		switch(MCManGetResult())
      {
		case 1:
			if(LoadGameData(&(work->savedata), work->mcman.file_kind)) return 1;
			else return -1;
		case -1:
			return -1;
		}
		break;
	}

	return 0;
}

static int FinishSubStep(Work *work)
{
	DispFilename(work);
	PageAlphaCtrl(work);

	switch(work->sub_step){
	case 0:
      //BP - game data has just been successfully loaded and the UI waits a moment
      //before closing automatically.  This is the place to handle "Wrong User" case.
      //Need to display a warning message, but not stop the user from proceeding.
#ifdef BP_PS3
      if( MGS_SaveStatus_WrongUser() )
      {
         int result = 0;
         ShowWrongUserWarning(&result);
      }
#endif
      work->mDisplayTrophiesDisabled = 0;
      
      if( Transfarring_CheckTrophyValidFlag( &sv_bp_linkvars ) )
      {
         if( !Transfarring_CheckTrophyCompatibility( 
            &sv_bp_linkvars, 
            Transfarring_KonamiSaveTypeToBPSaveType(work->mcman.file_kind),
            work->indexes[work->file_cursor] )
            )
         {
            // TODO: Display a message telling the user their trophies will not unlock
            work->mDisplayTrophiesDisabled = 1;
            CreateTrophiesDisabledMessage(work);
            BP_TrophySystem_Disable();
         }
      }
      else
      {
         work->mDisplayTrophiesDisabled = 1;
         CreateTrophiesDisabledMessage(work);
         BP_TrophySystem_Disable();
      }

      Transfarring_SetLoadedTransfarringID( 
         Transfarring_KonamiSaveTypeToBPSaveType(work->mcman.file_kind), 
         work->indexes[work->file_cursor] 
      );

      //Need to update the backup of the TID of the currently loaded save because the current
      //value is stomped by the backup when the load is finished.
      BackupLoadedTransfarringID();

		work->timer=0;
		work->sub_step++;

	case 1:
		DispSuccess(work);

		if((work->key_press & (PAD_OK|PAD_CANCEL)) &&
		   work->timer>=DISP_RESULT_COUNT2){

			work->timer=DISP_RESULT_COUNT;
		}
		if(work->timer>=DISP_RESULT_COUNT){
			switch(work->finish_mode){
			case 0:
				work->action_strcode=CODE_CLOSEALL;
				SE_WINCLOSE();
				break;
			case 1:
				work->action_strcode=CODE_TODOGTAG;
				break;
			}
			work->file_anim_count=FILE_ALPHA_TIME;
			SetFileAlphaAll1(work,0,0);
			PageAlphaSet(work,0,FILE_ALPHA_TIME);

			work->sub_step++;
		}
		work->timer++;
		break;
	case 2:
		if(work->file_anim_count>0) break;
		return 1;
	}

	return 0;
}

static int FailedSubStep(Work *work)
{
	DispFilename(work);
	PageAlphaCtrl(work);

	switch(work->sub_step){
	case 0:
		work->timer=0;
		work->sub_step++;

	case 1:
		DispFailed(work);

		if((work->key_press & (PAD_OK|PAD_CANCEL)) &&
		   work->timer>=DISP_RESULT_COUNT2){

			work->timer=DISP_RESULT_COUNT;
		}
		if(work->timer>=DISP_RESULT_COUNT){
			work->action_strcode=CODE_CLOSELIST;

			work->file_anim_count=FILE_ALPHA_TIME;
			SetFileAlphaAll1(work,0,0);
			PageAlphaSet(work,0,FILE_ALPHA_TIME);

			work->sub_step++;
		}
		work->timer++;
		break;
	case 2:
		if(work->file_anim_count>0) break;
		return 1;
	}

	return 0;
}

static void Kill(Work *work)
{
	if(work->finish_mode==0){
		if(work->strman!=NULL){
			GV_DestroyOtherActor(work->strman);
			work->strman=NULL;
		}
	}

	GV_DestroyActor(work);
}

static void InitSpr(Work *work);

static void Step(Work *work)
{
	MENU_TransTextTexture(work->strman);
	switch(work->step){
	case MCARD_INIT:
		if(work->busy_flag) break;
		InitSpr(work);
		work->step++;
		break;
	case MCARD_FILE_CHECK:
		FileCheckSubStep(work);
		break;
	case MCARD_FILE_SEL:
		switch(FileSelSubStep(work)){
		case 1:
			work->step=MCARD_CONFIRM;
			work->sub_step=0;
			break;
		case -1:
		case -2:
			work->step=MCARD_FILE_CHECK;
			work->sub_step=0;

         // BP JG - working progress for removing storage screen
         //work->action_strcode=CODE_CLOSEALL;
         //SE_WINCLOSE();

			break;
		}
		break;
	case MCARD_CONFIRM:
		work->mHidePsnWidget = 1;

      switch(ConfirmSubStep(work)){
		case 1:
         work->mHidePsnWidget = 0;
         if(work->mCloudConflictFlag)
         {
            work->mCloudConflictResolution = kCCR_UseCloudSaveFile;
         }
         work->step=MCARD_PROCESS;
			work->sub_step=0;
			break;
		case -1:
         work->mHidePsnWidget = 0;
			if(work->mCloudConflictFlag)
         {
            work->mCloudConflictResolution = kCCR_UseLocalSaveFile;
            work->step=MCARD_PROCESS;
            work->sub_step=0;
         }
         else
         {
            work->step=MCARD_FILE_SEL;
            work->sub_step=0x10;
         }
			break;
		case -2:
         work->mHidePsnWidget = 0;
			work->step=MCARD_FILE_CHECK;
			work->sub_step=0;
			break;
		}
		break;
	case MCARD_PROCESS:
		switch(ProcessSubStep(work)){
		case 1:
			work->step=MCARD_FINISH;
			work->sub_step=0;
			break;
		case -1:
		case -2:
			work->step=MCARD_FAILED;
			work->sub_step=0;
			break;
		}
		break;
	case MCARD_FINISH:
		if(FinishSubStep(work)){
			SetLoaddataID(work->port_sel,work->indexes[work->file_cursor]);
			SetLoaddataTime(MCManGetDateTime(work->indexes[work->file_cursor]));

			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;
		}
		break;

	case MCARD_FAILED:
		if(FailedSubStep(work)){
			// ShowSlot(work);
			work->step=MCARD_FILE_CHECK;
			work->sub_step=0;
		}
		break;

	case 0x10:
		if(!work->busy_flag){
			work->step++;
		}
		break;

		/* フレーム待ち */
	case 0x11:
		work->step++;
		break;
	case 0x12:
		CallLocalMsg(work);
		Kill(work);
		break;
	}
}


static void ProgMessage(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);
		int port=*(msg->message+2);

		if(command==PORTSEL_NAME){
			switch(arg){
			case TITLE_MSG_MODEEND:
				if(work->step==MCARD_FILE_CHECK){
					work->step=MCARD_FILE_SEL;
					work->sub_step=0;
					work->port_sel=port;
					work->start_mode=1;
					work->mcchanged_flags=0;

					// HideCursor(work);
				}
				break;
			case TITLE_MSG_MODECANCEL:
				if(work->step==MCARD_FILE_CHECK){
					work->ans=TITLE_MSG_MODECANCEL;
					work->step=0x10;
					// work->action_strcode=CODE_CLOSEALL;
				}
				break;
			}
		}

		msg++;
		n_msg--;
    }
}

static void Act(Work *work)
{
#if 0
   //BP - hack to dump all difficulty and stage name resources
   //for a given language to a text file so we can build them into the
   //executable.  mcman needs to be able to encode / decode any language
   //without dependencies on loaded data.
   {
      int i;
      const char * str;
      char fullPath[FILENAME_MAX];
      FILE * fp;
      sprintf( fullPath, "bp_fileparam_resource_%d.c", GM_Language );
      fp = fopen( fullPath, "wt" );
      fprintf( fp, "static const char * const bp_diffstr_%d[] =\n{\n", GM_Language );
      for(i=0;i<=MAX_LEVEL;++i)
      {
         str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(DIFFICULTY_RESOURCE,i));
         fprintf( fp, "\t\"%s\",\n", str );
      }
      fprintf( fp, "};\n\n" );

      fprintf( fp, "static const char * const bp_stagestr_%d[] =\n{\n", GM_Language );
      for(i=0;i<MC_N_STAGES;++i)
      {
         str=(char *)BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource(STAGENAME_RESOURCE,i));
         fprintf( fp, "\t\"%s\",\n", str );
      }
      fprintf( fp, "};\n\n" );
      fclose(fp);
      BP_BREAK;
   }
#endif

	ProgMessage(work);

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
	TagAnim(work);

	Key(work);
	Step(work);
}

static void Die(Work *work)
{
	int i;

	CallLocalProc(work);

	for(i=0;i<PAGE_STR_LEN;i++){
		if(work->page_str[i]!=NULL) SPR_Destroy_2D_Object(work->page_str[i]);
	}
	for(i=0;i<PAGE_NUM_STR_LEN;i++){
		if(work->page_num_str[i]!=NULL) SPR_Destroy_2D_Object(work->page_num_str[i]);
	}
	for(i=0;i<YESNO_STR_LEN;i++){
		if(work->yesno_str[i]!=NULL) SPR_Destroy_2D_Object(work->yesno_str[i]);
	}

	if(work->finish_mode==0){
		if(work->l2d_handle>=0){
			L2D_ReleaseLayout(work->l2d_handle);
			work->l2d_handle=-1;
		}
	}
}


/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_HIDE_MODE:
		if(work->finish_mode) TagAnimSet(work,CODE_SHOW_DOGTAGLOAD,CODE_HIDE_DOGTAGLOAD,
										 value/DIV_TICK_VALUE);
		else TagAnimSet(work,CODE_SHOW_LOAD,CODE_HIDE_LOAD,
						value/DIV_TICK_VALUE);
		break;
	case CODE_SHOW_MODE:
		if(work->finish_mode) TagAnimSet(work,CODE_HIDE_DOGTAGLOAD,CODE_SHOW_DOGTAGLOAD,
										 value/DIV_TICK_VALUE);
		else TagAnimSet(work,CODE_HIDE_LOAD,CODE_SHOW_LOAD,
						value/DIV_TICK_VALUE);
		break;
	case CODE_SOUND:
		if(work->sound_flag) SE_EXPANDLINE();
		else{
			SE_WINOPEN();
			work->sound_flag=1;
		}
		break;
	}
}

static void InitSpr(Work *work)
{
	int i;
	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,STR_NODE_FONT);
	if(spr==NULL) ASSERT(0);

#if 0 //BP_PS2 def PSX2
	work->base_u=spr->sprite.head.tex.u;
	work->base_v=spr->sprite.head.tex.v;
#else
	work->base_u=(int)( spr->sprite.head.tex.u * 16.0f );
	work->base_v=(int)( spr->sprite.head.tex.v * 16.0f );
#endif

	spr->sprite.pos.x=PAGE_STR_X;
	spr->sprite.pos.y=PAGE_STR_Y;
	spr->sprite.dw=SK_FONT_WIDTH;
	spr->sprite.dh=SK_FONT_HEIGHT;

	for(i=0;i<PAGE_STR_LEN;i++){
		work->page_str[i]=SPR_DuplicateTree(spr);
	}
	for(i=0;i<PAGE_NUM_STR_LEN;i++){
		work->page_num_str[i]=SPR_DuplicateTree(spr);
	}
	for(i=0;i<YESNO_STR_LEN;i++){
		work->yesno_str[i]=SPR_DuplicateTree(spr);
	}

	HidePage(work);
	HideYesNo(work);

	if(!work->start_mode){
		if(work->finish_mode) TagAnimSet0(work,CODE_HIDE_DOGTAGLOAD);
		else TagAnimSet0(work,CODE_HIDE_LOAD);
	}
}

static void InitMenu(Work *work)
{
	work->cursor=L2D_GetObject(work->l2d_handle,CODE_SELCURSNULL);
	work->cursor_spr=L2D_GetObject(work->l2d_handle,CODE_SELCURS);

	work->slot[0]=L2D_GetObject(work->l2d_handle,CODE_SLOT1);
	work->slot[1]=L2D_GetObject(work->l2d_handle,CODE_SLOT2);

	// work->yesno[0]=L2D_GetObject(work->l2d_handle,CODE_MENU_OK);
	// work->yesno[1]=L2D_GetObject(work->l2d_handle,CODE_MENU_CANCEL);

	/* シグナルハンドラの設定 */
	L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->proc=0;
	work->port_sel=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
    if(GCL_GetOption('b')!=NULL){
		work->bgname=GCL_GetNextInt();
    }
#if 0
	if(GCL_GetOption('r')!=NULL){
		int i;
		for( i = 0; i < LANG_MAX_NUM; i ++ ){
			loadgame_resource_name[i] = GCL_GetNextInt();
			printf("[%d:%x]\n",i, loadgame_resource_name[i]);
		}
	}
#endif
    if(GCL_GetOption('m')!=NULL){
		work->mcman.file_kind = GCL_GetNextInt();
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

	work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,LOAD_PRIORITY,0);

	if(work->l2d_handle<0){
#ifdef DEBUG
		printf("Select Scr : L2D Init Error = %d\n",work->l2d_handle);
#endif
		ASSERT(0);
	}

	/* layoutの初期化 */
	L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);

    return 1;
}

#ifdef PSX2 //-->XBOXは /kano/xmcman/ へ
/* 初期化部メイン */

void *NewLoadGameScr(int name,int where)
{
    Work *work ;

#ifdef BP_360
    bp_set_current_savegame( "G" );
    gBP_CanSelectNewDevice = 1;
#endif

    //BP TED - it is necessary to clear the wrong user flag every time this screen is entered,
    //since it's possible to back out of a "new game" off another user's cleared save file.
    //There are a couple more screens following where you select the chapter to play, etc.
    //so in this case the player hasn't committed to *playing* the save file he just loaded.
    MGS_SaveStatus_ClearWrongUser();

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->parent_name=0;

		work->step=0;
		work->sub_step=0;
		work->ans=0;
		work->strman=NULL;
		work->finish_mode=0;

		work->sound_flag=0;

		work->i_fileinfo=0;
		work->update_fileinfo=0;
		work->start_mode=0;

		work->bgname=0;

#if 0 //yano
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		work->loadgame_resource=loadgame_resource_name[GM_Language-GM_LANG_ENGLISH];
		work->stagename_resource=E_STAGENAME_RESOURCE;
		work->difficulty_resource=E_DIFFICULTY_RESOURCE;
#endif
#else
		work->loadgame_resource=loadgame_resource_name[GM_Language-GM_LANG_ENGLISH];
		work->stagename_resource=stagename_resource_name[GM_Language-GM_LANG_ENGLISH];
		work->difficulty_resource=E_DIFFICULTY_RESOURCE;
#endif

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		NewMCMan(&(work->mcman));

		GV_SetActorChild(work,&(work->mcman));
		
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

      work->mDogTagLoad = 0;
      work->mHidePsnWidget = 0;
      work->wasSignedIn = BP_Network_IsSignedIn();

#if defined(BP_VITA)
      work->mSystemResumeCount = Transfarring_GetSystemResumeCount();
#endif
    }
    return (void *)work ;
}

void *NewLoadGameScrForDogtag(int name,int parent_name,int l2d_handle,void *strman,int start_mode)
{
    Work *work ;

#ifdef BP_360
    bp_set_current_savegame( "G" );
    gBP_CanSelectNewDevice = 1;
#endif

    MGS_SaveStatus_ClearWrongUser();

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->parent_name=parent_name;

		work->step=0;
		work->sub_step=0;
		work->ans=0;
		work->strman=strman;
		work->finish_mode=1;

		work->sound_flag=start_mode;

		work->i_fileinfo=0;
		work->update_fileinfo=0;
		work->start_mode=start_mode;

		printf("Start_mode = %d\n",start_mode);

		work->bgname=0;

#if 0 //yano
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		work->loadgame_resource=loadgame_resource_name[GM_Language-GM_LANG_ENGLISH];
		work->stagename_resource=E_STAGENAME_RESOURCE;
		work->difficulty_resource=E_DIFFICULTY_RESOURCE;
#endif
#else
		work->loadgame_resource=loadgame_resource_name[GM_Language-GM_LANG_ENGLISH];
		work->stagename_resource=stagename_resource_name[GM_Language-GM_LANG_ENGLISH];
		work->difficulty_resource=E_DIFFICULTY_RESOURCE;
#endif

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		NewMCMan(&(work->mcman));

		GV_SetActorChild(work,&(work->mcman));

		work->l2d_handle=l2d_handle;

		InitMenu(work);

      work->mDogTagLoad = 1;
      work->mHidePsnWidget = 1;
      work->wasSignedIn = BP_Network_IsSignedIn();

#if defined(BP_VITA)
      work->mSystemResumeCount = Transfarring_GetSystemResumeCount();
#endif
    }
    return (void *)work ;
}

#endif	// PSX2
