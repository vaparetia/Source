//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	savegame.c
		セーブゲーム画面

	2001/06/15 K.Kano
	$Id: savegame.c,v 1.5 2002/12/05 18:41:59 takaki Exp $
*/


#include "titlescr.h"


#include "../../mode/codec/codec_signal.h"
#include "../../mode/codec/codecmem.h"
#include "../../mode/codec/cdc_load.h"


#include "font.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

#include "BP_LocalizedTextByEnum.h"
#include "BP_SaveLoadMGS.h"

#include "libfs.h"

#ifdef BP_PS3
#include "Transfarring_PS3CGlue.h"
#else
#include "TransfarringVTACGlue.h"
#endif

#include "transfarring_save_load_menu_utils.h"
#include "Transfarring_UI_Strings.h"

#include "BP_Network.h"
#include "BP_TUS.h"

#include "BP_Misc.h"

#include "BP_CommonDialog.h"

extern void ShowWrongUserWarning(int *pResult);

#include "BP_MD5.h"

#if defined(BP_VITA)
//#define BP_DISABLE_DELETE_FILES
#endif

#define N_YESNO_ITEMS		2
#define N_SLOT_ITEMS		2
#define N_FILES				10

#define NO_SAVEFILE		0xffff


#define PAGE_STR_LEN		8
#define PAGE_NUM_STR_LEN	8
#define YESNO_STR_LEN		8

extern void DecodeVRInfo( MCMAN_VR_INFODATA* pDst, const MCMAN_INFODATA* pSrc );
extern void EncodeVRInfo( MCMAN_INFODATA* pDst, const MCMAN_VR_INFODATA* pSrc );

#ifdef BP_360
extern void bp_set_current_savegame( const char * const dirsuffix );
extern int gBP_CanSelectNewDevice;
#endif


static const char* tales_title_name[] = {
	MCMAN_TALES_TITLE_A,
	MCMAN_TALES_TITLE_B,
	MCMAN_TALES_TITLE_C,
	MCMAN_TALES_TITLE_D,
	MCMAN_TALES_TITLE_E,
};

typedef struct _savegame_Work {
	GV_ACT_EX actor;

	MCMAN_WORK mcman;
	MCMAN_GAMEDATA savedata;
	MCMAN_INFODATA info;
	void *icon;
	int iconsize;

	void *strman;

	int name;
	int parent_name;

	int busy_flag;
	int l2d_handle;
	int action_strcode;

	int step;
	int sub_step;
	int timer;
	int end_proc;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int ans;
	int ans2;

	int save_count;

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
	int save_index;
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

	int l2d_strcode;
	int icon_strcode;

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

	unsigned char codec_flag;
	unsigned char format_flag;
	unsigned char mcchanged_flags;

#if defined(BP_VITA)
   unsigned char delete_flag;
#endif

//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
	int yes_width;
	int slash_width;
	int no_width;
//#endif

	int nlines_confirm;	// 下に出すフォントの行数

   int bp_savegame_error;   //BP - record whether player failed to save due to wrong user error

   int needs_cloud_upload;

#if defined(BP_VITA)
   STransfarringError mTransfarringManagerError;
#endif

   unsigned char mHidePsnWidget;

   int mSyncSlotsRemaining;
} Work;


#define PORTSEL_NAME		0x005bf5f1		/* portsel */

#define SAVEGAME_RESOURCE		5		/* セーブゲームリソース */
#define STAGENAME_RESOURCE		6		/* ステージ名 */
#define DIFFICULTY_RESOURCE		7		/* 難易度名 */


#define MAX_LEVEL				5

#if 0
#define FONT_WIDTH				24
#define FONT_HEIGHT				24
#else
#define FONT_WIDTH				FONT_SIZE_W
#define FONT_HEIGHT				FONT_SIZE_H
#endif
#define FONT_DISP_WIDTH			(15)
#define FONT_DISP_HEIGHT		(16)

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


#define SAVERES_CONFIRM_U			0
#define SAVERES_CONFIRM_V			(FILE_HEIGHT*(N_FILES+1))
#define SAVERES_CONFIRM_WIDTH		(FONT_WIDTH*30)
#if 0
#define SAVERES_CONFIRM_HEIGHT		(FONT_HEIGHT*5)
#else
#define SAVERES_CONFIRM_HEIGHT		(LINE_HEIGHT*3+DOWN_MARGINE)
#endif
#define SAVERES_SAVING_U			0
#if 0
#define SAVERES_SAVING_V			(FONT_HEIGHT*(N_FILES+6))
#else
#define SAVERES_SAVING_V			(SAVERES_CONFIRM_V+SAVERES_CONFIRM_HEIGHT)
#endif
#define SAVERES_SAVING_WIDTH		(FONT_WIDTH*30)
#define SAVERES_SAVING_HEIGHT		FILE_HEIGHT
#define SAVERES_FORMATTING_U		0
#if 0
#define SAVERES_FORMATTING_V		(FONT_HEIGHT*(N_FILES+7))
#else
#define SAVERES_FORMATTING_V		(SAVERES_SAVING_V+SAVERES_SAVING_HEIGHT)
#endif
#define SAVERES_FORMATTING_WIDTH	(FONT_WIDTH*30)
#define SAVERES_FORMATTING_HEIGHT	FILE_HEIGHT
#define SAVERES_SUCCESS_U			0
#define SAVERES_SUCCESS_V			(FILE_HEIGHT*(N_FILES+1))
#define SAVERES_SUCCESS_WIDTH		(FONT_WIDTH*30)
#define SAVERES_SUCCESS_HEIGHT		FILE_HEIGHT
#define SAVERES_FAILED_U			0
#define SAVERES_FAILED_V			(FILE_HEIGHT*(N_FILES+1))
#define SAVERES_FAILED_WIDTH		(FONT_WIDTH*30)
#define SAVERES_FAILED_HEIGHT		FILE_HEIGHT

#define SAVERES_FILEINFO_U			0
#if 0
#define SAVERES_FILEINFO_V			(FONT_HEIGHT*(N_FILES+8))
#else
#define SAVERES_FILEINFO_V			(SAVERES_FORMATTING_V+SAVERES_FORMATTING_HEIGHT)
#endif
#define SAVERES_FILEINFO_WIDTH		(FONT_WIDTH*30)
#define SAVERES_FILEINFO_HEIGHT		FILE_HEIGHT
#define SAVERES_FILEINFO_U2			0
#if 0
#define SAVERES_FILEINFO_V2			(FONT_HEIGHT*(N_FILES+9))
#else
#define SAVERES_FILEINFO_V2			(SAVERES_FILEINFO_V+SAVERES_FILEINFO_HEIGHT)
#endif
#define SAVERES_FILEINFO_WIDTH2		(FONT_WIDTH*30)
#define SAVERES_FILEINFO_HEIGHT2	FILE_HEIGHT


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
#define TEXT_X				108
#define TEXT_Y				ID_Y
#if 0
#define TEXT_W				(FILE_FONT_DISP_WIDTH*10)
#define TEXT_H				FILE_FONT_DISP_HEIGHT
#else
#define TEXT_W				(TEXT_WIDTH*FILE_FONT_DISP_WIDTH/FONT_WIDTH)
#define TEXT_H				(TEXT_HEIGHT*FILE_FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define TEXT_LINE_HEIGHT	ID_LINE_HEIGHT
#define DATE_X				240
#define DATE_Y				TEXT_Y
#if 0
#define DATE_W				(FILE_FONT_DISP_WIDTH*10)
#define DATE_H				FILE_FONT_DISP_HEIGHT
#else
#define DATE_W				(DATE_WIDTH*FILE_FONT_DISP_WIDTH/FONT_WIDTH)
#define DATE_H				(DATE_HEIGHT*FILE_FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define DATE_LINE_HEIGHT	TEXT_LINE_HEIGHT
#define TIME_X				384
#define TIME_Y				TEXT_Y
#if 0
#define TIME_W				(FILE_FONT_DISP_WIDTH*10)
#define TIME_H				FILE_FONT_DISP_HEIGHT
#else
#define TIME_W				(TIME_WIDTH*FILE_FONT_DISP_WIDTH/FONT_WIDTH)
#define TIME_H				(TIME_HEIGHT*FILE_FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define TIME_LINE_HEIGHT	TEXT_LINE_HEIGHT

#define SAVED_ID_U			0
#define SAVED_ID_V			(FILE_HEIGHT*(N_FILES+0))
#define SAVED_TEXT_U		(SAVED_ID_U+ID_WIDTH)
#define SAVED_TEXT_V		SAVED_ID_V
#define SAVED_DATE_U		(SAVED_TEXT_U+TEXT_WIDTH)
#define SAVED_DATE_V		SAVED_TEXT_V
#define SAVED_TIME_U		(SAVED_DATE_U+DATE_WIDTH)
#define SAVED_TIME_V		SAVED_TEXT_V


#if 0
#define MESSAGE_Y				72
#else
#define MESSAGE_Y3				(ID_Y+ID_LINE_HEIGHT*7+0)
#define MESSAGE_Y2				(ID_Y+ID_LINE_HEIGHT*8+2)
#define MESSAGE_Y				(ID_Y+ID_LINE_HEIGHT*9+4)
#endif



#define SAVERES_CONFIRM_X			(56-4)
#define SAVERES_CONFIRM_Y			(work->nlines_confirm * ( MESSAGE_Y3 - MESSAGE_Y2 ) + MESSAGE_Y )
#define SAVERES_FORMAT_CONFIRM_X	(56-4)
#define SAVERES_FORMAT_CONFIRM_Y	(work->nlines_confirm * ( MESSAGE_Y3 - MESSAGE_Y2 ) + MESSAGE_Y )


#if 0
#  ifdef JAPANESE_BP_IGNORE()
#    if 0
#      define SAVERES_CONFIRM_W		(FONT_DISP_WIDTH*30)
#      define SAVERES_CONFIRM_H		(FONT_DISP_HEIGHT*5)
#    endif
#    if 0
#      define SAVERES_FORMAT_CONFIRM_W		(FONT_DISP_WIDTH*30)
#      define SAVERES_FORMAT_CONFIRM_H		(FONT_DISP_HEIGHT*5)
#    endif
#  endif
#  ifdef ENGLISH
#    if 0
#      define SAVERES_CONFIRM_W		(FONT_DISP_WIDTH*30)
#      define SAVERES_CONFIRM_H		(FONT_DISP_HEIGHT*5)
#    endif
#    if 0
#      define SAVERES_FORMAT_CONFIRM_W		(FONT_DISP_WIDTH*30)
#      define SAVERES_FORMAT_CONFIRM_H		(FONT_DISP_HEIGHT*5)
#    endif
#  endif
#endif

#define SAVERES_CONFIRM_W		(SAVERES_CONFIRM_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_CONFIRM_H		(SAVERES_CONFIRM_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#define SAVERES_FORMAT_CONFIRM_W		(SAVERES_CONFIRM_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_FORMAT_CONFIRM_H		(SAVERES_CONFIRM_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)


#define SAVERES_SAVING_X		(56-4)
#define SAVERES_SAVING_Y		MESSAGE_Y
#if 0
#define SAVERES_SAVING_W		(FONT_DISP_WIDTH*30)
#define SAVERES_SAVING_H		FONT_DISP_HEIGHT
#else
#define SAVERES_SAVING_W		(SAVERES_SAVING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_SAVING_H		(SAVERES_SAVING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define SAVERES_FORMATTING_X	(56-4)
#define SAVERES_FORMATTING_Y	MESSAGE_Y
#if 0
#define SAVERES_FORMATTING_W	(FONT_DISP_WIDTH*30)
#define SAVERES_FORMATTING_H	FONT_DISP_HEIGHT
#else
#define SAVERES_FORMATTING_W	(SAVERES_FORMATTING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_FORMATTING_H	(SAVERES_FORMATTING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define SAVERES_SUCCESS_X		(56-4)
#define SAVERES_SUCCESS_Y		MESSAGE_Y
#if 0
#define SAVERES_SUCCESS_W		(FONT_DISP_WIDTH*30)
#define SAVERES_SUCCESS_H		FONT_DISP_HEIGHT
#else
#define SAVERES_SUCCESS_W		(SAVERES_SUCCESS_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_SUCCESS_H		(SAVERES_SUCCESS_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define SAVERES_FAILED_X		(56-4)
#define SAVERES_FAILED_Y		MESSAGE_Y
#if 0
#define SAVERES_FAILED_W		(FONT_DISP_WIDTH*30)
#define SAVERES_FAILED_H		FONT_DISP_HEIGHT
#else
#define SAVERES_FAILED_W		(SAVERES_FAILED_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_FAILED_H		(SAVERES_FAILED_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define SAVERES_FILEINFO_X		(56-4)
#define SAVERES_FILEINFO_Y		(92-1)
#if 0
#define SAVERES_FILEINFO_W		(FONT_DISP_WIDTH*30)
#define SAVERES_FILEINFO_H		FONT_DISP_HEIGHT
#else
#define SAVERES_FILEINFO_W		(SAVERES_FILEINFO_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_FILEINFO_H		(SAVERES_FILEINFO_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define LISTLOAD_U				0
#define LISTLOAD_V				(FILE_HEIGHT*(N_FILES+1))
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
#define YES_V					(SAVERES_FILEINFO_V2+SAVERES_FILEINFO_HEIGHT2)
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

#if defined(BP_VITA)
#define SAVERES_DELETING_U		(NO_U + NO_WIDTH)
#define SAVERES_DELETING_V		SLASH_V
#define SAVERES_DELETING_WIDTH	(FONT_WIDTH*12)
#define SAVERES_DELETING_HEIGHT	FILE_HEIGHT
#define SAVERES_DELETING_X	(56-4)
#define SAVERES_DELETING_Y	MESSAGE_Y
#define SAVERES_DELETING_W	(SAVERES_DELETING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_DELETING_H	(SAVERES_DELETING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define SAVERES_DELETE_U		0
#define SAVERES_DELETE_V		(YES_V + YES_HEIGHT)
#define SAVERES_DELETE_WIDTH	(FONT_WIDTH*30)
#define SAVERES_DELETE_HEIGHT	FILE_HEIGHT
#define SAVERES_DELETE_X	64
#define SAVERES_DELETE_Y	(MESSAGE_Y + LINE_HEIGHT)
#define SAVERES_DELETE_W	(SAVERES_DELETE_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define SAVERES_DELETE_H	(SAVERES_DELETE_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

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
	RES_INDEX_CONFIRM_OVERWRITE,
	RES_INDEX_CONFIRM_FORMAT,

	RES_INDEX_SAVING,
	RES_INDEX_FORMATTING,
	RES_INDEX_SUCCESS,
	RES_INDEX_FAILED,

	RES_INDEX_FORMAT_FAILED=RES_INDEX_FAILED+3,

	RES_INDEX_YES,
	RES_INDEX_NO,
};


#define DISP_RESULT_COUNT	DIRECT_TICK((int)(3.0f*60.0f))		/* セーブの結果の表示時間 */
#define DISP_RESULT_COUNT2	DIRECT_TICK((int)(0.5f*60.0f))		/* セーブの結果の表示時間２ */


#define FILE_ALPHA_TIME		DIRECT_TICK(10)
#define PAGE_MOVE_TIME		DIRECT_TICK(3)
#define CURSOR_MOVE_TIME	DIRECT_TICK(5)


#define DISP_CHANL			DG_CHANL_MENU
#define SAVEGAME_PRIORITY	0


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


#if 0
#  ifndef	AREA_EU_BP_IGNORE()		// #ifndef PAL
#    define NEW_SAVEFILE		"NEW FILE"
#    define DAMAGED_FILE		"DAMAGED FILE"
#  else
#    define NEW_SAVEFILE		"NEW DATA"
#    define DAMAGED_FILE		"DAMAGED DATA"
#  endif
#else
#  define NEW_SAVEFILE_NOEU		"NEW FILE"
#  define DAMAGED_FILE_NOEU		"DAMAGED FILE"
#  define NEW_SAVEFILE_EU		"NEW DATA"
#  define DAMAGED_FILE_EU		"DAMAGED DATA"
#  define DAMAGED_FILE ( BP_Area_EU() ? DAMAGED_FILE_EU : DAMAGED_FILE_NOEU )
#  define NEW_SAVEFILE ( BP_Area_EU() ? NEW_SAVEFILE_EU : NEW_SAVEFILE_NOEU )
#endif

#ifdef MGS2_VRTRIAL
#define VR_TRAINING_FILE	"VR Training Data"
#else
#define VR_TRAINING_FILE	"Missions Data:"
#endif

#define REPEAT_FIRST		20
#define REPEAT_NEXT			1


#define CURSOR_ADJUST		3.0f


enum {
	SAVEGAME_ANS_SUCCESS=0,
	SAVEGAME_ANS_FAILED_CANCEL,
	SAVEGAME_ANS_FAILED_NO_FREEAREA,
	SAVEGAME_ANS_FAILED_NO_CARD,
	SAVEGAME_ANS_FAILED_ERROR_CARD,

	SAVEGAME_ANS_FAILED_UNFORMATTED,
};


extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え

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

         MENU_ClearPartTextTexture(work->strman, SAVERES_CONFIRM_V, SAVERES_CONFIRM_HEIGHT);

         work->nlines_confirm = MENU_CreateTextTextureGetLines(work->strman, SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
            SAVERES_CONFIRM_WIDTH,SAVERES_CONFIRM_HEIGHT, 0, 0, 0, tmpString);
      }

      MENU_PutTextScreenNoTR(work->strman,
         SAVERES_CONFIRM_X,SAVERES_CONFIRM_Y,
         SAVERES_CONFIRM_X+SAVERES_CONFIRM_W,
         SAVERES_CONFIRM_Y+SAVERES_CONFIRM_H,
         SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
         SAVERES_CONFIRM_U+SAVERES_CONFIRM_WIDTH,
         SAVERES_CONFIRM_V+SAVERES_CONFIRM_HEIGHT,
         TEXTCOLOR);
   }
#endif
}

static void Key(Work *work)
{
	int status=GV_PadDataDirect[0].status & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL|PAD_X|PAD_Y);
	int press=GV_PadDataDirect[0].press & (PAD_U|PAD_D|PAD_L|PAD_R|PAD_L1|PAD_R1|PAD_OK|PAD_CANCEL|PAD_X|PAD_Y);

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

static void ReturnValueToParent(Work *work)
{
	int val=-1;
	int port_val[2];
	int port;

	for(port=0;port<2;port++){
		switch(MCManGetCardType(port)){
		case MCMAN_CARDTYPE_NOCARD:
		case MCMAN_CARDTYPE_PS1:
		case MCMAN_CARDTYPE_POCKET_STATION:

		case MCMAN_CARDTYPE_MULTITAP:
			port_val[port]=SAVEGAME_ANS_FAILED_NO_CARD;
			break;

		case MCMAN_CARDTYPE_ERRORCARD:
			port_val[port]=SAVEGAME_ANS_FAILED_ERROR_CARD;
			break;

		case MCMAN_CARDTYPE_PS2_FORMATTED:
			if(MCManGetFreeSize(port)>=GAME_MC_MAXSIZE ||
			   MCManNFilesE(port)>0){

				port_val[port]=SAVEGAME_ANS_SUCCESS;
			}
			else{
				port_val[port]=SAVEGAME_ANS_FAILED_NO_FREEAREA;
			}
			break;

		case MCMAN_CARDTYPE_PS2_UNFORMATTED:
			port_val[port]=SAVEGAME_ANS_SUCCESS;
			break;
		}
	}

	if(work->ans==TITLE_MSG_MODEEND){
		val=SAVEGAME_ANS_SUCCESS;
	}
	else if(port_val[0]==SAVEGAME_ANS_SUCCESS || port_val[1]==SAVEGAME_ANS_SUCCESS){
		/* ただのキャンセル */
		val=SAVEGAME_ANS_FAILED_CANCEL;
	}
	else if(port_val[0]==SAVEGAME_ANS_FAILED_NO_FREEAREA ||
			port_val[1]==SAVEGAME_ANS_FAILED_NO_FREEAREA){

		/* 空きがない */
		val=SAVEGAME_ANS_FAILED_NO_FREEAREA;
	}
	else if(port_val[0]==SAVEGAME_ANS_FAILED_ERROR_CARD ||
			port_val[1]==SAVEGAME_ANS_FAILED_ERROR_CARD){

		/* エラーのカードのみがささっている */
		val=SAVEGAME_ANS_FAILED_ERROR_CARD;
	}
	else{
		/* カードがささっていない */
		val=SAVEGAME_ANS_FAILED_NO_CARD;
	}

   //BP - override any other specific error if we got this error.
   //This basically affects just the voice that otacon or rose plays back
   //after you get back to the codec. (generic "I couldn't save the game" line)
   if( work->bp_savegame_error )
   {
      val = work->bp_savegame_error;
   }

	if(work->end_proc==0 ||
	   work->end_proc==1){

		GV_CallParentSignalFunc(work,CDC_SIGNAL_SAVE_DIE,val);
	}
}

/* ------------------------------------------------------------------------ */

static void SearchSaveFile(Work *work)
{
	int i=0;

	if(MCManGetCardType(work->port_sel)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
		work->save_index=0;
		return;
	}

	while(i< MCMAN_GetDataFileMax( work->mcman.file_kind ) ) {
		// if(!MCManCheckFileFlag(i)){
		if(!MCManCheckExactFileFlag(i)){
			work->save_index=i;
			return;
		}
		i++;
	}
	work->save_index=NO_SAVEFILE;
}

static int SearchUpFile(Work* work, int start)
{
	int i=start;
	while( i < MCMAN_GetDataFileMax(work->mcman.file_kind) ){
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

	if(MCManGetCardType(work->port_sel)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
		work->n_indexes=0;
		return;
	}

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

	if(MCManGetCardType(work->port_sel)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
		work->n_indexes=0;
		return;
	}

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

/* ------------------------------------------------------------------------ */


static void CreateFilenameTexture(Work *work)
{
	char tdatestr[32];
	char ttimestr[32];
	char tid[32];
	int i;

#ifdef BP_PS3
char *savingstr=BP_GetStringForEnum(kBP_PLS_SavingShort);
#else
char *savingstr=BP_GetStringForEnum(kBP_PLS_Saving);
#endif
	
	char *formattingstr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_FORMATTING));
   char *yesstr = NULL;
   char *nostr = NULL;

   if ( BP_Area_EU() )
   {
//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
	   yesstr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_YES));
	   nostr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_NO));
   }
//#endif

	MENU_ClearTextTexture(work->strman);

	MENU_CreateTextTexture(work->strman,
						   SAVERES_SAVING_U,SAVERES_SAVING_V,
						   SAVERES_SAVING_WIDTH,SAVERES_SAVING_HEIGHT,0,0,0,savingstr);
	MENU_CreateTextTexture(work->strman,
						   SAVERES_FORMATTING_U,SAVERES_FORMATTING_V,
						   SAVERES_FORMATTING_WIDTH,SAVERES_FORMATTING_HEIGHT,0,0,0,formattingstr);
   
#if defined(BP_VITA)
   {
      char const * const deletingStr = BP_GetCustomOverrideString("SAVELOAD", "DELETE_PROGRESS");
      char const * const deleteStr = BP_GetCustomOverrideString("SAVELOAD", "DELETE_LABEL");

      MENU_CreateTextTexture(work->strman,
         SAVERES_DELETING_U,SAVERES_DELETING_V,
         SAVERES_DELETING_WIDTH,SAVERES_DELETING_HEIGHT, 0, 0, 0, (char*)deletingStr);
      
      MENU_CreateTextTexture(work->strman,
         SAVERES_DELETE_U,SAVERES_DELETE_V,
         SAVERES_DELETE_WIDTH,SAVERES_DELETE_HEIGHT, 0, 0, 0, (char*)deleteStr);
   }
#endif

   if ( BP_Area_EU() )
   {
//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
      work->yes_width=MENU_CreateTextTexture(work->strman,
									         YES_U,YES_V,
									         YES_WIDTH,YES_HEIGHT,0,0,0,yesstr)-YES_U;
      work->slash_width=MENU_CreateTextTexture(work->strman,
										       SLASH_U,SLASH_V,
										       SLASH_WIDTH,SLASH_HEIGHT,0,0,0," / ")-SLASH_U;
      work->no_width=MENU_CreateTextTexture(work->strman,
									        NO_U,NO_V,
									        NO_WIDTH,NO_HEIGHT,0,0,0,nostr)-NO_U;

	// printf("V = %d\n%s,%s\n",YES_V,yesstr,nostr);
   }
//#endif


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

		switch ( work->mcman.file_kind ) {
		case MCMAN_FILE_KIND_GAME:
			diff=(diff-GM_LEVEL_VERYEASY)/10;
			if(MCManCheckExactFileFlag(id)<0){
				str="----------";
			}
			else if(diff>=0 && diff<=MAX_LEVEL){
				str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(DIFFICULTY_RESOURCE,diff));
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
			} else if( info->snake_tales_no >= 1 && info->snake_tales_no <= MCMAN_MAX_TALES ) {
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
	if(work->n_indexes<N_FILES && work->save_index!=NO_SAVEFILE){
		int id=work->save_index;
		char *str;
		int x,y;

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

		str=NEW_SAVEFILE;

		x=TEXT_U;
		y=TEXT_V+TEXT_HEIGHT*i;

		MENU_CreateTextTexture(work->strman,x,y,TEXT_WIDTH,TEXT_HEIGHT,0,0,0,str);
	}
}

static void CreateSavedFilenameTexture(Work *work)
{
	char tdatestr[32];
	char ttimestr[32];
	char tid[32];
	int id;

	if(work->file_cursor<work->n_indexes){
		id=work->indexes[work->file_cursor];
	}
	else{
		id=work->save_index;
	}

	/* ファイル名や日時の展開 */
	{
		MCMAN_INFODATA *info=MCManGetFileInfo(id);
		MCMAN_VR_INFODATA vrinfo;
		int ptime=info->mgs2_playtime/FRAMES_PER_SEC;
		int diff=info->difficulty;

		char *str;
		unsigned char *data;
		int x,y;

		char buffer[32];

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

		x=SAVED_ID_U;
		y=SAVED_ID_V;

		MENU_CreateTextTexture(work->strman,x,y,ID_WIDTH,ID_HEIGHT,0,0,0,tid);

		switch ( work->mcman.file_kind ) {
		case MCMAN_FILE_KIND_GAME:
			diff=(diff-GM_LEVEL_VERYEASY)/10;
			if(diff>=0 && diff<=MAX_LEVEL){
				str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(DIFFICULTY_RESOURCE,diff));
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
			} else if( info->snake_tales_no >= 1 && info->snake_tales_no <= MCMAN_MAX_TALES ) {
				str = (char*)tales_title_name[ info->snake_tales_no - 1 ];
			} else {
				str = "?";
			}
			break;
		}

		x=SAVED_TEXT_U;
		y=SAVED_TEXT_V;

		MENU_CreateTextTexture(work->strman,x,y,TEXT_WIDTH,TEXT_HEIGHT,0,0,0,str);

		if( work->mcman.file_kind == MCMAN_FILE_KIND_SNAKE_TALES ) {
			/* クリアフラグ */
			int j;
			int n;

			char clrstr[ MCMAN_MAX_TALES + 1 ];

			x=SAVED_TIME_U;
			y=SAVED_TIME_V;

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
			x=SAVED_DATE_U;
			y=SAVED_DATE_V;

			data=(unsigned char *)MCManGetDateTime(id);

			strcpy(tdatestr,"2000 00 00");

			tdatestr[2]+=data[DATETIME_INDEX_YEAR]/10;
			tdatestr[3]+=data[DATETIME_INDEX_YEAR]%10;
			tdatestr[5]+=data[DATETIME_INDEX_MONTH]/10;
			tdatestr[6]+=data[DATETIME_INDEX_MONTH]%10;
			tdatestr[8]+=data[DATETIME_INDEX_DAY]/10;
			tdatestr[9]+=data[DATETIME_INDEX_DAY]%10;

			MENU_CreateTextTexture(work->strman,x,y,DATE_WIDTH,DATE_HEIGHT,0,0,0,tdatestr);

			/* プレイ時間 */
			x=SAVED_TIME_U;
			y=SAVED_TIME_V;

			if(ptime>=60*60*10000){
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

static void CreateListLoading(Work *work)
{
	MENU_ClearPartTextTexture(work->strman,LISTLOAD_V,LISTLOAD_HEIGHT);

	MENU_CreateTextTexture(work->strman,LISTLOAD_U,LISTLOAD_V,
						   LISTLOAD_WIDTH,LISTLOAD_HEIGHT,0,0,0,"Loading ... Please wait .");
}

static void DispListLoading(Work *work)
{
	MENU_PutTextScreenNoTR(work->strman,
						   LISTLOAD_X,LISTLOAD_Y,LISTLOAD_X+LISTLOAD_W,LISTLOAD_Y+LISTLOAD_H,
						   LISTLOAD_U,LISTLOAD_V,LISTLOAD_U+LISTLOAD_WIDTH,LISTLOAD_V+LISTLOAD_HEIGHT,
						   TEXTCOLOR);
}


/* ------------------------------------------------------------------------ */

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
					work->ans2=0;
					work->mcchanged_flags=0;

					HideCursor(work);
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

static int SignalFunc(Work *work,int signal,int value)
{
#if defined(BP_PS3) || defined(BP_360)
   // BP JG - if there was an error during save, don't quit this save screen, in case it was your only chance to save. TRC violation.
   // BP DAK - if there was an error during save (besides port select), don't quit this save screen, in case it was your only chance to save. TRC violation.
   if ( 
      signal==SIGNAL_PORTSEL_CANCEL && work->bp_savegame_error != SAVEGAME_ANS_SUCCESS
#if defined(BP_360)
      && work->port_sel != 0
#endif
      )
   {
      signal = SIGNAL_PORTSEL_OK;
      work->bp_savegame_error = SAVEGAME_ANS_SUCCESS;
   }
#endif
	switch(signal)
   {
	   case SIGNAL_PORTSEL_OK:
		   if(work->step==MCARD_FILE_CHECK)
         {
			   work->step=MCARD_FILE_SEL;
			   work->sub_step=0;
			   work->port_sel=value;
			   work->start_mode=1;
			   work->ans2=0;
			   work->mcchanged_flags=0;

			// HideCursor(work);
		}
		break;
	case SIGNAL_PORTSEL_CANCEL:
		if(work->step==MCARD_FILE_CHECK){
			work->ans=TITLE_MSG_MODECANCEL;
			work->step=0x10;
         // BP - Dak, whenever exiting because of a port cancel, set the error message to the save canceled version
         // all the system dialogs are telling you if you don't select a device you will not be able to save so this seems
         // fitting
#if defined(BP_360)
         if( work->bp_savegame_error == SAVEGAME_ANS_SUCCESS )
            work->bp_savegame_error = SAVEGAME_ANS_FAILED_CANCEL;
#endif
			// work->action_strcode=CODE_CLOSEALL;
		}
		break;
	default:
		return GV_DefaultSignalFunc(work,signal,value);
	}
	return 0;
}

/* ------------------------------------------------------------------------ */

static void DispFileInfo(Work *work,int alpha)
{
#if 0
	static const int uvwh[2][4]={
		{ SAVERES_FILEINFO_U ,SAVERES_FILEINFO_V ,SAVERES_FILEINFO_WIDTH ,SAVERES_FILEINFO_HEIGHT , },
		{ SAVERES_FILEINFO_U2,SAVERES_FILEINFO_V2,SAVERES_FILEINFO_WIDTH2,SAVERES_FILEINFO_HEIGHT2, },
	};
#endif

	int disp_width;
	int color=TEXTCOLOR_NOSEL | (alpha<<24);


	if(work->update_fileinfo){
		int id;
		MCMAN_INFODATA *info;
		int stage_num;
		char *str;
		char buf[64];	
		int flag=0;

		if(work->save_index!=NO_SAVEFILE){
			flag=MCManCheckExactFileFlag(work->save_index);
		}

		if(work->file_cursor<work->n_indexes || flag>0){
			if(flag) id=work->save_index;
			else id=work->indexes[work->file_cursor];

			if(MCManCheckExactFileFlag(id)<0){
				str=DAMAGED_FILE;
			}
			else{
				if( work->mcman.file_kind == MCMAN_FILE_KIND_VR ) {
					MCMAN_VR_INFODATA vrinfo;
					info=MCManGetFileInfo(id);
					DecodeVRInfo( &vrinfo, info );
					GV_ZeroMemory( buf, sizeof( buf ) );
					str = buf;
					strcpy( buf, VR_TRAINING_FILE );
					memcpy( buf + strlen( buf ), vrinfo.name, MCMAN_VR_NAME_MAX );
				} else {
					info=MCManGetFileInfo(id);
					stage_num=info->stage_num;
					
					if(stage_num>=0 && stage_num<MC_N_STAGES){
						str=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(STAGENAME_RESOURCE,stage_num));
					}
					else{
						str="NO STAGENAME";
					}
				}
			}
		}
		else{
			if( work->mcman.file_kind == MCMAN_FILE_KIND_VR ) {
				GV_ZeroMemory( buf, sizeof( buf ) );
				str = buf;
				strcpy( buf, NEW_SAVEFILE );
            strcat( buf, ":" );
				memcpy( buf + strlen( buf ), GM_MyName, MCMAN_VR_NAME_MAX );
			} else {
				str=NEW_SAVEFILE;
			}
		}

		if(work->i_fileinfo){
			MENU_ClearPartTextTexture(work->strman,SAVERES_FILEINFO_V2,SAVERES_FILEINFO_HEIGHT2);

			work->fileinfo_width
				=MENU_CreateTextTexture(work->strman,
										SAVERES_FILEINFO_U2,SAVERES_FILEINFO_V2,
										SAVERES_FILEINFO_WIDTH2,SAVERES_FILEINFO_HEIGHT2,
										0,0,0,str);
		}
		else{
			MENU_ClearPartTextTexture(work->strman,SAVERES_FILEINFO_V,SAVERES_FILEINFO_HEIGHT);

			work->fileinfo_width
				=MENU_CreateTextTexture(work->strman,
										SAVERES_FILEINFO_U,SAVERES_FILEINFO_V,
										SAVERES_FILEINFO_WIDTH,SAVERES_FILEINFO_HEIGHT,
										0,0,0,str);
		}

		work->update_fileinfo=0;
	}

	disp_width=work->fileinfo_width*FONT_DISP_WIDTH/FONT_WIDTH;

	if(work->i_fileinfo){
		MENU_PutTextScreenNoTR(work->strman,
							   SAVERES_FILEINFO_X,
							   SAVERES_FILEINFO_Y,
							   SAVERES_FILEINFO_X+disp_width,
							   SAVERES_FILEINFO_Y+SAVERES_FILEINFO_H,
							   SAVERES_FILEINFO_U2,
							   SAVERES_FILEINFO_V2,
							   SAVERES_FILEINFO_U2+work->fileinfo_width,
							   SAVERES_FILEINFO_V2+SAVERES_FILEINFO_HEIGHT2,
							   color);
	}
	else{
		MENU_PutTextScreenNoTR(work->strman,
							   SAVERES_FILEINFO_X,
							   SAVERES_FILEINFO_Y,
							   SAVERES_FILEINFO_X+disp_width,
							   SAVERES_FILEINFO_Y+SAVERES_FILEINFO_H,
							   SAVERES_FILEINFO_U,
							   SAVERES_FILEINFO_V,
							   SAVERES_FILEINFO_U+work->fileinfo_width,
							   SAVERES_FILEINFO_V+SAVERES_FILEINFO_HEIGHT,
							   color);
	}
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
					STR_WIDTH,STR_HEIGHT,PAGE_NUM_STR_LEN);
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
#if defined(BP_VITA)
static void DispDelete(Work *work)
{
   Transfarring_RenderIconSprite(work->strman, SAVERES_DELETE_X - 15, SAVERES_DELETE_Y + 1,
      TRANSFARRING_SQ_BUTTON_COLOR, kTransfarring_SqButtonIcon, kSaveLoadScreen_Normal);

   MENU_PutTextScreenNoTR(work->strman,SAVERES_DELETE_X,SAVERES_DELETE_Y,
      SAVERES_DELETE_X+SAVERES_DELETE_W,
      SAVERES_DELETE_Y+SAVERES_DELETE_H,
      SAVERES_DELETE_U,SAVERES_DELETE_V,
      SAVERES_DELETE_U+SAVERES_DELETE_WIDTH,
      SAVERES_DELETE_V+SAVERES_DELETE_HEIGHT,
      TEXTCOLOR);
}
#endif

static void DispFilename(Work *work)
{
	int i;

   if(!work->mHidePsnWidget)
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

#if defined(BP_VITA) && !defined(BP_DISABLE_DELETE_FILES)
      if( work->file_cursor == i && work->step == MCARD_FILE_SEL && !(isLocked || isLinked || isCloud) )
      {
         DispDelete(work);
      }
#endif

		if(MCManCheckExactFileFlag(id)<0){
			col=(work->file_alpha[i]<<24) | TEXTCOLOR_NOSEL;
		}
		else{
			MCMAN_INFODATA *info;

			col=(work->file_alpha[i]<<24) | TEXTCOLOR_NOSEL;
			if( work->mcman.file_kind == MCMAN_FILE_KIND_GAME ) {
				info=(MCMAN_INFODATA *)MCManGetFileInfo(id);
				if(info->clear_count>0){
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

	if(work->n_indexes<N_FILES && work->save_index!=NO_SAVEFILE){
		int x1,y1,x2,y2;
		int u1,v1,u2,v2;
		int col;

		col=(work->file_alpha[i]<<24) | TEXTCOLOR_NOSEL;

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
	}

	DispFileInfo(work,work->file_alpha[work->file_cursor]);
}

static void DispSavedFilename(Work *work)
{
	int i;

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

	i=work->file_cursor;

	{
		int x1,y1,x2,y2;
		int u1,v1,u2,v2;
		int col;
		MCMAN_INFODATA *info;

		col=(work->file_alpha[i]<<24) | TEXTCOLOR_NOSEL;

		if( work->mcman.file_kind == MCMAN_FILE_KIND_GAME ) {
			if(work->file_cursor<work->n_indexes){
				info=(MCMAN_INFODATA *)MCManGetFileInfo(work->indexes[work->file_cursor]);
			}
			else{
				info=(MCMAN_INFODATA *)MCManGetFileInfo(work->save_index);
			}
			if(info->clear_count>0){
				col=(work->file_alpha[i]<<24) | TEXTCOLOR_CLEARED;
			}
		}

      //Transfarring - Do not execute for a new save.
      if(i < work->n_indexes)
      {
         int const id = work->indexes[i];

         ESaveType const saveType = Transfarring_GetTransfarringType(work->mcman.file_kind);

         int const isLocked = Transfarring_IsFileLocked(saveType, id);
         int const isCloud = Transfarring_IsFileCloud(saveType, id);
         int const isLinked = Transfarring_IsFileLinked(saveType, id);

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
      }

		if( MCMAN_GetDataFileMax( work->mcman.file_kind ) != 1 ) {
			x1=ID_X;
			y1=work->file_y_pos[i];
			x2=x1+ID_W;
			y2=y1+ID_H;

			u1=SAVED_ID_U;
			v1=SAVED_ID_V;
			u2=u1+ID_WIDTH;
			v2=v1+ID_HEIGHT;

			MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);
		}

		x1=TEXT_X;
		y1=work->file_y_pos[i];
		x2=x1+TEXT_W;
		y2=y1+TEXT_H;

		u1=SAVED_TEXT_U;
		v1=SAVED_TEXT_V;
		u2=u1+TEXT_WIDTH;
		v2=v1+TEXT_HEIGHT;

		MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);

		x1=DATE_X;
		y1=work->file_y_pos[i];
		x2=x1+DATE_W;
		y2=y1+DATE_H;

		u1=SAVED_DATE_U;
		v1=SAVED_DATE_V;
		u2=u1+DATE_WIDTH;
		v2=v1+DATE_HEIGHT;

		MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);

		x1=TIME_X;
		y1=work->file_y_pos[i];
		x2=x1+TIME_W;
		y2=y1+TIME_H;

		u1=SAVED_TIME_U;
		v1=SAVED_TIME_V;
		u2=u1+TIME_WIDTH;
		v2=v1+TIME_HEIGHT;

		MENU_PutTextScreenNoTR(work->strman,x1,y1,x2,y2,u1,v1,u2,v2,col);
	}

	DispFileInfo(work,work->file_alpha[work->file_cursor]);
}


/* ------------------------------------------------------------------------ */

static void CreateConfirm(Work *work)
{
	char *confirmstr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_CONFIRM));

	MENU_ClearPartTextTexture(work->strman,SAVERES_CONFIRM_V,SAVERES_CONFIRM_HEIGHT);

	work->nlines_confirm 
		= MENU_CreateTextTextureGetLines(work->strman,
										 SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
										 SAVERES_CONFIRM_WIDTH,SAVERES_CONFIRM_HEIGHT,0,0,0,confirmstr);
}

static void CreateConfirmOverwrite(Work *work)
{
	char *confirmovstr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_CONFIRM_OVERWRITE));

	MENU_ClearPartTextTexture(work->strman,SAVERES_CONFIRM_V,SAVERES_CONFIRM_HEIGHT);

	work->nlines_confirm 
		= MENU_CreateTextTextureGetLines(work->strman,
										 SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
										 SAVERES_CONFIRM_WIDTH,SAVERES_CONFIRM_HEIGHT,0,0,0,confirmovstr);
}

static void CreateConfirmFormat(Work *work)
{
	char *confirmfmtstr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_CONFIRM_FORMAT));

	MENU_ClearPartTextTexture(work->strman,SAVERES_CONFIRM_V,SAVERES_CONFIRM_HEIGHT);

	work->nlines_confirm 
		= MENU_CreateTextTextureGetLines(work->strman,
										 SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
										 SAVERES_CONFIRM_WIDTH,SAVERES_CONFIRM_HEIGHT,0,0,0,confirmfmtstr);
}

static void CreateConfirmTUSOverride(Work *work)
{
   char const * const confirmstr = GetTransfarringString(kTString_ConflictSaveMessage);

   MENU_ClearPartTextTexture(work->strman, SAVERES_CONFIRM_V, SAVERES_CONFIRM_HEIGHT);

   work->nlines_confirm = MENU_CreateTextTextureGetLines(work->strman, SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
      SAVERES_CONFIRM_WIDTH,SAVERES_CONFIRM_HEIGHT, 0, 0, 0, (char*)confirmstr);
}

#if defined(BP_VITA)
static void CreateConfirmDelete(Work *work)
{
   char const * const confirmDelStr = BP_GetCustomOverrideString("SAVELOAD", "DELETE_QUESTION");
   MENU_ClearPartTextTexture(work->strman,SAVERES_CONFIRM_V,SAVERES_CONFIRM_HEIGHT);

   work->nlines_confirm = MENU_CreateTextTextureGetLines(work->strman, SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
      SAVERES_CONFIRM_WIDTH, SAVERES_CONFIRM_HEIGHT, 0, 0, 0, (char*)confirmDelStr);
}
#endif

static void DispConfirm(Work *work)
{
	// HideSlot(work);

	if(work->format_flag){
		MENU_PutTextScreenNoTR(work->strman,
							   SAVERES_FORMAT_CONFIRM_X,SAVERES_FORMAT_CONFIRM_Y,
							   SAVERES_FORMAT_CONFIRM_X+SAVERES_FORMAT_CONFIRM_W,
							   SAVERES_FORMAT_CONFIRM_Y+SAVERES_FORMAT_CONFIRM_H,
							   SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
							   SAVERES_CONFIRM_U+SAVERES_CONFIRM_WIDTH,
							   SAVERES_CONFIRM_V+SAVERES_CONFIRM_HEIGHT,
							   TEXTCOLOR);
	}
	else{
		MENU_PutTextScreenNoTR(work->strman,
                        SAVERES_CONFIRM_X,SAVERES_CONFIRM_Y,
							   SAVERES_CONFIRM_X+SAVERES_CONFIRM_W,
							   SAVERES_CONFIRM_Y+SAVERES_CONFIRM_H,
							   SAVERES_CONFIRM_U,SAVERES_CONFIRM_V,
							   SAVERES_CONFIRM_U+SAVERES_CONFIRM_WIDTH,
							   SAVERES_CONFIRM_V+SAVERES_CONFIRM_HEIGHT,
							   TEXTCOLOR);
	}

//#ifdef	AREA_EU_BP_IGNORE()		// #ifdef PAL
	if ( BP_Area_EU() )
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

		MENU_PutTextScreenNoTR(work->strman,yes_x,MESSAGE_Y,
							   yes_x+YES_W,MESSAGE_Y+YES_H,
							   YES_U,YES_V,
							   YES_U+YES_WIDTH,YES_V+YES_HEIGHT,
							   yescol);

		MENU_PutTextScreenNoTR(work->strman,slash_x,MESSAGE_Y,
							   slash_x+SLASH_W,MESSAGE_Y+SLASH_H,
							   SLASH_U,SLASH_V,
							   SLASH_U+SLASH_WIDTH,SLASH_V+SLASH_HEIGHT,
							   slashcol);

		MENU_PutTextScreenNoTR(work->strman,no_x,MESSAGE_Y,
							   no_x+NO_W,MESSAGE_Y+NO_H,
							   NO_U,NO_V,
							   NO_U+NO_WIDTH,NO_V+NO_HEIGHT,
							   nocol);
	}
//#endif

}

static void DispSaving(Work *work)
{
	// HideSlot(work);

	MENU_PutTextScreenNoTR(work->strman,SAVERES_SAVING_X,SAVERES_SAVING_Y,
						   SAVERES_SAVING_X+SAVERES_SAVING_W,
						   SAVERES_SAVING_Y+SAVERES_SAVING_H,
						   SAVERES_SAVING_U,SAVERES_SAVING_V,
						   SAVERES_SAVING_U+SAVERES_SAVING_WIDTH,
						   SAVERES_SAVING_V+SAVERES_SAVING_HEIGHT,
						   TEXTCOLOR);
}

static void DispFormatting(Work *work)
{
	// HideSlot(work);

	MENU_PutTextScreenNoTR(work->strman,SAVERES_FORMATTING_X,SAVERES_FORMATTING_Y,
						   SAVERES_FORMATTING_X+SAVERES_FORMATTING_W,
						   SAVERES_FORMATTING_Y+SAVERES_FORMATTING_H,
						   SAVERES_FORMATTING_U,SAVERES_FORMATTING_V,
						   SAVERES_FORMATTING_U+SAVERES_FORMATTING_WIDTH,
						   SAVERES_FORMATTING_V+SAVERES_FORMATTING_HEIGHT,
						   TEXTCOLOR);
}

#if defined(BP_VITA)
static void DispDeleting(Work *work)
{
   // HideSlot(work);

   MENU_PutTextScreenNoTR(work->strman,SAVERES_DELETING_X,SAVERES_DELETING_Y,
      SAVERES_DELETING_X+SAVERES_DELETING_W,
      SAVERES_DELETING_Y+SAVERES_DELETING_H,
      SAVERES_DELETING_U,SAVERES_DELETING_V,
      SAVERES_DELETING_U+SAVERES_DELETING_WIDTH,
      SAVERES_DELETING_V+SAVERES_DELETING_HEIGHT,
      TEXTCOLOR);
}
#endif

static void CreateSuccess(Work *work)
{
	char *successstr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_SUCCESS));

	MENU_ClearPartTextTexture(work->strman,SAVERES_SUCCESS_V,SAVERES_SUCCESS_HEIGHT);

	MENU_CreateTextTexture(work->strman,
						   SAVERES_SUCCESS_U,SAVERES_SUCCESS_V,
						   SAVERES_SUCCESS_WIDTH,SAVERES_SUCCESS_HEIGHT,0,0,0,successstr);
}

static void DispSuccess(Work *work)
{
	// HideSlot(work);

	MENU_PutTextScreenNoTR(work->strman,SAVERES_SUCCESS_X,SAVERES_SUCCESS_Y,
						   SAVERES_SUCCESS_X+SAVERES_SUCCESS_W,
						   SAVERES_SUCCESS_Y+SAVERES_SUCCESS_H,
						   SAVERES_SUCCESS_U,SAVERES_SUCCESS_V,
						   SAVERES_SUCCESS_U+SAVERES_SUCCESS_WIDTH,
						   SAVERES_SUCCESS_V+SAVERES_SUCCESS_HEIGHT,
						   TEXTCOLOR);
}

static void CreateFailed(Work *work)
{
   char const * messageStr = 0;

   if(work->format_flag)
   {
      messageStr = BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_FORMAT_FAILED));
   }
#if defined(BP_VITA)
   else if(work->delete_flag)
   {
      messageStr = GetTransfarringStringForErrorCode(work->mTransfarringManagerError.mHighLevelError);
   }
#endif
   else
   {
      messageStr = BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GM_GetResource(SAVEGAME_RESOURCE,RES_INDEX_FAILED));
   }

   if(messageStr)
   {
      MENU_ClearPartTextTexture(work->strman,SAVERES_FAILED_V,SAVERES_FAILED_HEIGHT);
      MENU_CreateTextTexture(work->strman, SAVERES_FAILED_U, SAVERES_FAILED_V, SAVERES_FAILED_WIDTH, SAVERES_FAILED_HEIGHT, 0, 0, 0, (char*)messageStr);
   }
}

static void DispFailed(Work *work)
{
	// HideSlot(work);

	MENU_PutTextScreenNoTR(work->strman,SAVERES_FAILED_X,SAVERES_FAILED_Y,
						   SAVERES_FAILED_X+SAVERES_FAILED_W,
						   SAVERES_FAILED_Y+SAVERES_FAILED_H,
						   SAVERES_FAILED_U,SAVERES_FAILED_V,
						   SAVERES_FAILED_U+SAVERES_FAILED_WIDTH,
						   SAVERES_FAILED_V+SAVERES_FAILED_HEIGHT,
						   TEXTCOLOR);
}


/* ------------------------------------------------------------------------ */

static int FileCheckSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(work->codec_flag){
			GV_SetActorChild(work,NewPortselForCodec(PORTSEL_NAME,work->name,work->l2d_handle,
													 1,work->start_mode,work->strman,
													 SAVEGAME_RESOURCE,&work->mcman));
		}
		else{
			GV_SetActorChild(work,NewPortsel2(PORTSEL_NAME,work->name,work->l2d_handle,
											  1,work->start_mode,work->strman,
											  SAVEGAME_RESOURCE, &work->mcman));
		}
		work->sub_step++;
		break;
	case 1:
      DisplaySynchingMessage(work);
		break;
	}

	return 0;
}

enum {
	FILESEL_INIT=0,

#if 1
	FILESEL_START_GETINFO,
	FILESEL_WAIT_GETINFO,
#endif

	FILESEL_START_FADEIN0,
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

#ifdef DEBUG_MODE
	// printf("sub_step = %d\n",work->sub_step);
#endif

	if(MCManChecked()){

#ifdef DEBUG_MODE
		printf("mcchanged_flag = Hit 1\n");
#endif

		if(MCManIsMCChanged(work->port_sel)){

#ifdef DEBUG_MODE
			printf("mcchanged_flag = Hit 2\n");
#endif

			work->mcchanged_flags=1;
		}
	}

	switch(work->sub_step){
	case FILESEL_INIT:
		work->file_cursor=0;
		work->page_cursor=0;

		work->start_index=0;

		/* 空き容量の確認 */
		if(MCManGetCardType(work->port_sel)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
			work->save_index=0;
			work->n_pages=1;
			work->n_indexes=0;

			work->sub_step=FILESEL_START_FADEIN0;
		}
		else{
			int n_files=MCManExactNFiles();
			int i;
			int id=GetLoaddataID();

			SearchUpFiles(work);

			if(MCManGetFreeSize(work->port_sel)>=GAME_MC_MAXSIZE) SearchSaveFile(work);
			else work->save_index=NO_SAVEFILE;

			if(work->save_index!=NO_SAVEFILE) n_files++;
			work->n_pages=(n_files+N_FILES-1)/N_FILES;

			if(GetLoaddataPort()!=work->port_sel || id==MC_LOADDATA_NOID){
				/* 最初は一番下にカーソルを合わせる */
				work->page_cursor=work->n_pages-1;
				work->file_cursor=n_files-work->page_cursor*N_FILES-1;

				// printf("Cursor = %d %d ( %d )\n",work->page_cursor,work->file_cursor,n_files);

				i=work->page_cursor;

				while(i>0){
					work->start_index=work->indexes[N_FILES-1]+1;
					i--;

					SearchUpFiles(work);
				}
			}
			else{
				while(id>work->indexes[work->n_indexes-1]){
					work->page_cursor++;

					work->start_index=work->indexes[N_FILES-1]+1;
					SearchUpFiles(work);
				}
				while(id!=work->indexes[work->file_cursor]) work->file_cursor++;
			}

			work->sub_step++;
		}

		HidePage(work);
		break;

#if 1
	case FILESEL_START_GETINFO:
		if(work->mcchanged_flags){
			return -1;
		}
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
		if(work->mcchanged_flags){
			return -1;
		}
		switch(MCManGetResult()){
		case 1:
			work->sub_step++;
			break;
		case -1:
		case -2:
			return -1;
		}
		break;
#endif

	case FILESEL_START_FADEIN0:
		if(work->port_sel){
			work->action_strcode=CODE_OPENSELFILE2;
		}
		else{
			work->action_strcode=CODE_OPENSELFILE1;
		}
		work->sub_step++;
		break;

	case FILESEL_FADEIN:
		if(work->mcchanged_flags){
			work->action_strcode=CODE_CLOSELIST;
			return -1;
		}

		if(work->busy_flag) break;

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

		if(work->n_indexes || work->save_index!=NO_SAVEFILE){
			LocalShowCursor(work);
		}

		if(work->mcchanged_flags){
			FileSelErrorEnd(work);
			return 0;
		}

		if(work->key_press & PAD_CANCEL){
			SE_CANCEL();
			FileSelErrorEnd(work);
			return 0;
		}
		else if(work->key_press & PAD_OK)
      {
         if((work->file_cursor < work->n_indexes
            && !Transfarring_IsFileLocked(Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor]))
            || (work->file_cursor >= work->n_indexes && work->save_index != NO_SAVEFILE))
         {
            HideCursor(work);
            SE_OK();
            work->file_anim_count=FILE_ALPHA_TIME;
            SetFileYPosAll1(work);
            SetFileAlphaAll1(work,SEL_ALPHA,0);
            return 1;
         }
         else
         {
            SE_CANCEL();
         }
      }
#if defined(BP_VITA)
#if !defined(BP_DISABLE_DELETE_FILES)
      else if((work->key_press & PAD_Y) && work->file_cursor < work->n_indexes)
      {
         if(!Transfarring_IsFileLocked(Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor])
            && !Transfarring_IsFileLinked(Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor])
            && !Transfarring_IsFileCloud(Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor]))
         {
            work->delete_flag = 1;
            HideCursor(work);
            SE_OK();
            work->file_anim_count=FILE_ALPHA_TIME;
            SetFileYPosAll1(work);
            SetFileAlphaAll1(work,SEL_ALPHA,0);
            return 1;
         }
         else
         {
            SE_CANCEL();
         }
      }
#endif
#endif
      else if(!work->mHidePsnWidget && (work->key_press & PAD_X) && !BP_Network_IsSignedIn())
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
				if(work->n_indexes) work->start_index=work->indexes[0]-1;
				else work->start_index--;
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

				{
					int n_files=work->n_indexes;

					if(work->save_index!=NO_SAVEFILE) n_files++;

					if(work->file_cursor>=n_files){
						work->file_cursor=n_files-1;
					}
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
						if(work->n_indexes) work->start_index=work->indexes[0]-1;
						else work->start_index--;
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
				else{
					int n_files=work->n_indexes;

					if(work->save_index!=NO_SAVEFILE) n_files++;

					if(work->file_cursor>=n_files){
						work->file_cursor=n_files-1;
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
			}
			break;
		}
		break;
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

	case 0x20:
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
		if(!work->busy_flag && work->file_anim_count <= 0)
      {
			work->format_flag = 0;

			if(MCManGetCardType(work->port_sel) == MCMAN_CARDTYPE_PS2_UNFORMATTED)
         {
				work->format_flag = 1;
				CreateConfirmFormat(work);
			}
			else if(work->file_cursor < work->n_indexes)
         {
#if defined(BP_VITA)
            if(work->delete_flag)
            {
               CreateConfirmDelete(work);
            }
            else
#endif
            if(Transfarring_HasCloudConflict(Transfarring_GetTransfarringType(work->mcman.file_kind),
               work->indexes[work->file_cursor]))
            {
               CreateConfirmTUSOverride(work);
            }
            else
            {
               CreateConfirmOverwrite(work);
            }
         }
			else
         {
            CreateConfirm(work);
         }

//#ifndef	AREA_EU_BP_IGNORE()		// #ifndef PAL
         if ( !BP_Area_EU() )
         {
			   SetYesNo(work);
         }
//#endif
			work->sub_step++;
		}
		break;
	case 3:
		DispConfirm(work);

//#ifndef	AREA_EU_BP_IGNORE()		// #ifndef PAL
      if ( !BP_Area_EU() )
      {
         CurMoveYesNo(work);
      }
//#endif

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
#if defined(BP_VITA)         
         work->delete_flag = 0;
#endif
			
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
#if defined(BP_VITA)
             work->delete_flag = 0;
#endif            
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

#if defined(BP_VITA)      
typedef enum
{
   kDelete_Start = 0,
   kDelete_Loop,
   kDelete_Error,
   kDelete_Done,
}
EDeleteStepSubStep;

typedef enum
{
   kDelete_ShouldLoop = 0,
   kDelete_Finished = 2,
   kDelete_ReportError = 3,
}
EDeleteStepReturnValue;

static int ProcessDeleteSubStep(Work * const work)
{
   EDeleteStepReturnValue returnValue = kDelete_ShouldLoop;

   switch(work->sub_step)
   {
   case kDelete_Start:
      if(work->file_cursor < work->n_indexes)
      {
         work->mTransfarringManagerError = Transfarring_PostWork_DeleteSaveFile(&work->mcman,
            Transfarring_GetTransfarringType(work->mcman.file_kind), work->indexes[work->file_cursor]);

         if(work->mTransfarringManagerError.mHighLevelError == kTE_Success)
         {
            work->sub_step = kDelete_Loop;         
         }
         else
         {
            work->sub_step = kDelete_Error;
         }
      }
      else
      {
         work->sub_step = kDelete_Done;
      }
      break;
   case kDelete_Loop:
      {
         work->mTransfarringManagerError = Transfarring_ProcessWork();
         if(work->mTransfarringManagerError.mHighLevelError != kTE_Processing)
         {
            if( work->mTransfarringManagerError.mHighLevelError == kTE_Success)
            {
               work->sub_step = kDelete_Done;

            }
            else
            {
               work->sub_step = kDelete_Error;
            }
         }
      }
      break;
   case kDelete_Error:
      returnValue = kDelete_ReportError;
      break;
   case kDelete_Done:
      returnValue = kDelete_Finished;
      break;
   }

   return returnValue;
}
#endif

static int ProcessSubStep(Work *work)
{
   ESaveType saveType = kST_MaxSaveTypes;

	DispFilename(work);
	
   if(work->format_flag)
   {
      DispFormatting(work);
   }
#if defined(BP_VITA)       
	else if(work->delete_flag)
   {
      DispDeleting(work);
   }
#endif
   else
   {
      DispSaving(work);
   }

	PageAlphaCtrl(work);

	if(MCManChecked())
   {
		if(MCManIsMCChanged(work->port_sel))
      {
			work->mcchanged_flags=1;
		}
	}
#if defined(BP_VITA)      
   if(work->delete_flag)
   {
      return ProcessDeleteSubStep(work);
   }
   else
#endif
   {
      switch(work->sub_step)
      {
      case 0:
         /* シナリオからセーブするデータを取得 */
         work->save_count=GM_SaveCount;

         GM_SaveCount++;
         if(GM_SaveCount<0) GM_SaveCount=32767;

         if(work->file_cursor<work->n_indexes)
         {
            SaveGameData(&(work->savedata),work->indexes[work->file_cursor], work->mcman.file_kind );
            MC_InitIconEnv( work->indexes[work->file_cursor], work->mcman.file_kind );
         }
         else{
            SaveGameData(&(work->savedata),work->save_index, work->mcman.file_kind );
            MC_InitIconEnv( work->save_index, work->mcman.file_kind );
         }

         // 拡張されたデータ部分を埋める
         switch(work->mcman.file_kind)
         {
         case MCMAN_FILE_KIND_VR:
         {
            MCMAN_VR_INFODATA vrinfo;
            SaveVRInfo(&vrinfo);
            printf("name = %16s\n", vrinfo.name );
            EncodeVRInfo( &work->info, &vrinfo );
            break;
         }
         case MCMAN_FILE_KIND_SNAKE_TALES:
         {
            int i;
            SaveGameInfo(&(work->info));
            work->info.snake_tales_no
               = GCL_ReadVarRef( &TTL_ReferenceVariable[0], 0 );
            work->info.st_clear_flag = 0;
            for( i = 0 ; i < MCMAN_MAX_TALES ; i++ ) {
               work->info.st_clear_flag |=
                  GCL_ReadVarRef( &TTL_ReferenceVariable[1 + i], 0 ) ?
                  ( 1 << i ) : 0;
            }
            break;
         }
         default:
            SaveGameInfo(&(work->info));
            break;
         }

         saveType = Transfarring_KonamiSaveTypeToBPSaveType(work->mcman.file_kind);

         // Mark as having offline work
         if( work->file_cursor<work->n_indexes && Transfarring_IsFileCloud(saveType, work->indexes[work->file_cursor]) )
         {
            Transfarring_SetOfflineWork(saveType, work->indexes[work->file_cursor], 1);
            work->needs_cloud_upload = 1;
         }
         else
         {
            work->needs_cloud_upload = 0;
         }

         // TODO: Display a message earlier in the chain to give them the opportunity to cancel out
         if( work->file_cursor<work->n_indexes )
         {
            {
               // If the originator data is compatible, clear the originator data and rewrite it.
               // We do this because otherwise overwrite saves would end up using the PSN hashes
               // of the last saved game (wrong tid).
               
               // If the PSN hash is zero, then keep it as zero so that trophies are locally enabled but get
               // disabled in case of transfarring the file.
               unsigned char tid[16];
               Transfarring_GetLoadedTransfarringID( tid );
               if( Transfarring_CheckTrophyCompatibilityTID( &bp_linkvars, tid )
                  && !BP_IsMD5HashZero( bp_linkvars.mOriginalPSNAccount ))
               {
                  Transfarring_ClearOriginatorData( &bp_linkvars );
               }
            }
            
            //Transfarring_SetTrophyInvalidFlagIfApplicable( &bp_linkvars, saveType, work->indexes[work->file_cursor] );
         }
         else
         {
            {
               // If the originator data is compatible, clear the originator data and rewrite it.
               // We have to do this because by making a new TransfarringID for the new save, the PSN account hash changes

               // If the PSN hash is zero, then keep it as zero so that trophies are locally enabled but get
               // disabled in case of transfarring the file.
               unsigned char tid[16];
               Transfarring_GetLoadedTransfarringID( tid );
               if( Transfarring_CheckTrophyCompatibilityTID( &bp_linkvars, tid )
                  && !BP_IsMD5HashZero( bp_linkvars.mOriginalPSNAccount ) )
               {
                  Transfarring_ClearOriginatorData( &bp_linkvars );
               }
            }

            // Make a new TransfarringID for this empty slot
            Transfarring_MakeNewTransfarringID( 
               Transfarring_KonamiSaveTypeToBPSaveType(work->mcman.file_kind), 
               work->save_index );

            //Transfarring_SetTrophyInvalidFlagIfApplicable( &bp_linkvars, saveType, work->save_index );
         }

         {
            // If the loaded save's originator data is not compatible, then disable trophies.
            // Use this instead of Transfarring_SetTrophyInvalidFlagIfApplicable() because when
            // overriding a save, the tid in the bp_linkvars is the currently loaded tid, not
            // the one of the file being overriden.
            unsigned char tid[16];
            Transfarring_GetLoadedTransfarringID( tid );
            if( !Transfarring_CheckTrophyCompatibilityTID( &bp_linkvars, tid ) )
            {
               Transfarring_SetTrophyInvalidFlag( &bp_linkvars );
            }
         }

         Transfarring_LockTransfarringIDUpdates();

         work->sub_step++;
      case 1:
         if(work->mcchanged_flags)
         {
            /* セーブが正常に行われなかった場合は戻す */
            GM_SaveCount=work->save_count;
            Transfarring_AllowTransfarringIDUpdates();
            return -1;
         }

         {
         int curFileIndex = work->file_cursor < work->n_indexes ? work->indexes[work->file_cursor] : work->save_index;

         // Write the PSN account information if it hasn't been written yet
         Transfarring_TryWriteOriginatorData( 
            &bp_linkvars, 
            Transfarring_KonamiSaveTypeToBPSaveType(work->mcman.file_kind),
            curFileIndex );
         }

         if(work->file_cursor<work->n_indexes)
         {
            if(SaveGameStart(work->port_sel,work->indexes[work->file_cursor],
               &(work->savedata),sizeof(work->savedata),
               work->icon,work->iconsize,&(work->info)))
            {

                  work->sub_step++;
            }
         }
         else
         {
            if(SaveGameStart(work->port_sel,work->save_index,
               &(work->savedata),sizeof(work->savedata),
               work->icon,work->iconsize,&(work->info)))
            {

                  work->sub_step++;
            }
         }
         break;
      case 2:
         if(work->mcchanged_flags)
         {
            /* セーブが正常に行われなかった場合は戻す */
            GM_SaveCount=work->save_count;
            Transfarring_AllowTransfarringIDUpdates();
            return -1;
         }

         switch(MCManGetResult())
         {
         case 1:
            Transfarring_AllowTransfarringIDUpdates();

            // If we've already saved the file, and we're signed in, push the file to the server and re-save
            if (work->needs_cloud_upload && BP_Network_IsSignedIn())
            {
               // Sync to server
               work->sub_step = 3;
            }
            else
            {
               return 1;
            }
            break;
         case -1:
            /* セーブが正常に行われなかった場合は戻す */
            GM_SaveCount=work->save_count;
            Transfarring_AllowTransfarringIDUpdates();
            return -1;
         }

         if(!MCManFormatting()) work->format_flag=0;

         break;
      case 3: // Post sync work
         Transfarring_PostWork_SyncSingleCloudFile(&work->mcman, 
            Transfarring_KonamiSaveTypeToBPSaveType(work->mcman.file_kind),
            work->indexes[work->file_cursor],
            kCCR_UseLocalSaveFile,
            1 );
         work->sub_step = 4;
         break;
      case 4: // Consume sync work
      {
         STransfarringError error = Transfarring_ProcessWork();
         switch(error.mHighLevelError)
         {
         case kTE_Processing:
            break;
         case kTE_Success:
            // Sync operation re-saves file, no need to do it again
            return 1;
         default:
            if(error.mLowLevelError)
            {
               BP_CommonDialog_WantsErrorDialog(error.mLowLevelError);
            }
            else
            {
               BP_CommonDialog_WantsMessageDialog(GetTransfarringStringForErrorCode(error.mHighLevelError), kMDL_OK);
            }
            return 1;
         }
      }
      break;
      }
   }
	return 0;
}

static int FinishSubStep(Work *work)
{
	// DispFilename(work);
	PageAlphaCtrl(work);

	switch(work->sub_step){
	case 0:
		work->timer=0;
		work->sub_step++;
		work->update_fileinfo=1;

		CreateSuccess(work);
		CreateSavedFilenameTexture(work);

		SE_SAVE_FINISH();

	case 1:
		DispSavedFilename(work);
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
		DispSavedFilename(work);
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
		CreateFailed(work);

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
	ReturnValueToParent(work);
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
			break;
		}
		break;
	case MCARD_CONFIRM:
		work->mHidePsnWidget = 1;
      switch(ConfirmSubStep(work)){
		case 1:
         work->mHidePsnWidget = 0;
			work->step=MCARD_PROCESS;
			work->sub_step=0;
			break;
		case -1:
         work->mHidePsnWidget = 0;
			work->step=MCARD_FILE_SEL;
			work->sub_step=0x10;
			if(MCManGetCardType(work->port_sel)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
				work->ans2=SAVEGAME_ANS_FAILED_UNFORMATTED;
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
		BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 1);
      switch(ProcessSubStep(work))
      {
      case 1:
         BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
         //BP - no new error is flagged if the user tried to save while playing another user's
         //save file.  The save system quietly punted out.  Handle wrong user at this step.
#ifdef BP_PS3
         if( MGS_SaveStatus_WrongUser() )
         {
            int result = 0;
            ShowWrongUserWarning(&result);
            work->bp_savegame_error = SAVEGAME_ANS_FAILED_ERROR_CARD;
            work->step=MCARD_FAILED;
            work->sub_step=0;
         }
         else
#endif
         {
            work->step=MCARD_FINISH;
			   work->sub_step=0;
         }
			break;
		case -1:
		case -2:
#if defined(BP_PS3) || defined(BP_360) || defined(BP_VITA)
         // BP Dak - No matter what the error lets just throw up the generic error message
         {
            //PS3 system dialog has already been displayed at this point if necessary.
            //Just make sure that error is returned properly to the codec screen so an appropriate
            //voice line plays.
            work->bp_savegame_error = SAVEGAME_ANS_FAILED_ERROR_CARD;
         }
#endif
         BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
			work->step = MCARD_FAILED;
			work->sub_step = 0;
			break;
#if defined(BP_VITA)   
      case kDelete_Finished:
         BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
         work->delete_flag = 0;
         work->step = MCARD_FILE_SEL;
         work->sub_step = 0;
         ResetLoaddataID();
         break;
      case kDelete_ReportError:
         BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
         work->step = MCARD_FAILED;
         work->sub_step = 0;
         break;
#endif
		}
		break;
	case MCARD_FINISH:
		if(FinishSubStep(work)){
			if(work->file_cursor<work->n_indexes){
				SetLoaddataID(work->port_sel,work->indexes[work->file_cursor]);
			}
			else{
				SetLoaddataID(work->port_sel,work->save_index);
			}
			work->ans=TITLE_MSG_MODEEND;
			work->step=0x10;
		}
		break;

	case MCARD_FAILED:
		if(FailedSubStep(work))
      {
#if defined(BP_VITA)  
         if(work->delete_flag)
         {
            work->delete_flag = 0;
            work->step = MCARD_FILE_SEL;
            work->sub_step = 0;
         }
         else
#endif
         {
            // ShowSlot(work);
            work->step=MCARD_FILE_CHECK;
            work->sub_step=0;
         }
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
		Kill(work);
		break;
	}
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
	TagAnim(work);

	Key(work);
	Step(work);
}

static void Die(Work *work)
{
	int i;

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
		if(work->strman!=NULL){
			GV_DestroyOtherActor(work->strman);
			work->strman=NULL;
		}
	}
	if(work->end_proc!=0 && work->end_proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=((work->ans==TITLE_MSG_MODEEND) ? 1 : 0);

		GCL_ExecProc(work->end_proc,&arg);
	}

	if(work->codec_flag) SPR_ResetMemoryManager();
}


/* ------------------------------------------------------------------------ */


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
	work->base_u=(int)(spr->sprite.head.tex.u*16.0f);
	work->base_v=(int)(spr->sprite.head.tex.v*16.0f);
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

	TagAnimSet0(work,CODE_HIDE_SAVE);
}

static void InitMenu(Work *work)
{
	work->cursor=L2D_GetObject(work->l2d_handle,CODE_SELCURSNULL);
	work->cursor_spr=L2D_GetObject(work->l2d_handle,CODE_SELCURS);

	work->slot[0]=L2D_GetObject(work->l2d_handle,CODE_SLOT1);
	work->slot[1]=L2D_GetObject(work->l2d_handle,CODE_SLOT2);

	// work->yesno[0]=L2D_GetObject(work->l2d_handle,CODE_MENU_OK);
	// work->yesno[1]=L2D_GetObject(work->l2d_handle,CODE_MENU_CANCEL);
}

static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_HIDE_MODE:
		TagAnimSet(work,CODE_SHOW_SAVE,CODE_HIDE_SAVE,value/DIV_TICK_VALUE);
		break;
	case CODE_SHOW_MODE:
		TagAnimSet(work,CODE_HIDE_SAVE,CODE_SHOW_SAVE,value/DIV_TICK_VALUE);
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


/* ------------------------------------------------------------------------ */


/* 資源を獲得 */
static int CODEC_GetResources(Work *work,int l2d_strcode)
{
   float safeZoneOffsetY = 0;
	void *l2d_data;

	if( work->codec_flag ) {
		if((l2d_data=CDC_GetFileEntry(l2d_strcode,'o'))==NULL){
			return 0;
		}
		work->l2d_handle=-1;
		work->l2d_handle=L2D_SetupLayout(l2d_data,DISP_CHANL,SAVEGAME_PRIORITY,SPR_FLAG_PRIV,safeZoneOffsetY );

		SPR_SetMemoryManager(codecMalloc, codecFree);

	} else {
		work->l2d_handle=L2D_LoadLayout( l2d_strcode, DISP_CHANL,SAVEGAME_PRIORITY,SPR_FLAG_PRIV);
		SPR_ResetMemoryManager();
	}

	work->port_sel=0;
	work->start_mode=0;


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
void *CODEC_NewMCSave(int iconname,int l2d_strcode)
{
	Work *work;
	void *icon;
	int iconsize;

#ifdef BP_360
   bp_set_current_savegame( "G" );
   gBP_CanSelectNewDevice = 1;
#endif

	// printf("Height Limit= %d\n",SAVERES_FILEINFO_V2+SAVERES_FILEINFO_HEIGHT2);

	if((icon=CDC_GetFileEntry(iconname,'r'))==NULL){
#ifdef DEBUG
		printf("CODEC_NewMCSave : icon not found\n");
#endif
		return NULL;
	}
	iconsize=MCAccessCalcIconSize(icon);

	if((work=(Work *)codecMalloc(sizeof(Work)))==NULL){
#ifdef DEBUG
		printf("CODEC_NewMCSave : memory not enough\n");
#endif
		return NULL;
	}

	/* アクター起動 */
	GV_ZeroMemory(work,sizeof(Work));
	GV_SetActorFreeFunc(work,codecDelayedFree);
	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(ACT_MODE,work,0x1FF); /* ACT_MODE <--- user/mode/codec/codecmem.h */

	GV_SetActor(&(work->actor),Act,Die);
	GV_ActorEX(&(work->actor));
	GV_SetActorSignalFunc(work,(int (*)(void *,int,int))SignalFunc);

	work->name=0;
	work->step=0;
	work->sub_step=0;
	work->ans=0;
	work->ans2=0;
	work->strman=NULL;
	work->finish_mode=0;
	work->end_proc=0;
	work->codec_flag=1;

	work->sound_flag=0;

	work->i_fileinfo=0;
	work->update_fileinfo=0;


	work->icon=icon;
	work->iconsize=iconsize;


	NewMCMan(&(work->mcman));

	GV_SetActorChild(work,&(work->mcman));

	if(!CODEC_GetResources(work,l2d_strcode)){
		GV_DestroyActor(work);
		return NULL ;
	}
	if((work->strman=NewTextScreenControlForCodec())==NULL){
		GV_DestroyActor(work);
		return NULL;
	}

	InitMenu(work);

	return work;
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->l2d_strcode=0;
	work->l2d_handle=-1;
	work->end_proc=0;
	work->port_sel=0;
	work->start_mode=0;
	work->l2d_strcode=0;
	work->icon_strcode=0;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->end_proc=GCL_GetNextInt();
    }

#if 0
    if(GCL_GetOption('b')!=NULL){
		work->bgname=GCL_GetNextInt();
    }
#endif

    if(GCL_GetOption('i')!=NULL){
		work->icon_strcode=GCL_GetNextInt();
    }

    if(GCL_GetOption('m')!=NULL){
		work->mcman.file_kind = GCL_GetNextInt();
    }

}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	GetOptionValue(work);

	work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,SAVEGAME_PRIORITY,0);

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


	if((work->icon=GV_GetCache(GV_CacheID(work->icon_strcode,'r')))==NULL){
#ifdef DEBUG
		printf("NewSaveGameScr : icon not found\n");
#endif
		return NULL;
	}
	work->iconsize=MCAccessCalcIconSize(work->icon);

    return 1;
}


#ifdef PSX2
/* 初期化部メイン */
void *NewSaveGameScr(int name,int where)
{
    Work *work ;

#ifdef BP_360
    bp_set_current_savegame( "G" );
    gBP_CanSelectNewDevice = 1;
#endif

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->step=0;
		work->sub_step=0;
		work->ans=0;
		work->ans2=0;
		work->strman=NULL;
		work->finish_mode=0;
		work->end_proc=0;
		work->codec_flag=0;

		work->sound_flag=0;

		work->i_fileinfo=0;
		work->update_fileinfo=0;

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
      work->mHidePsnWidget = 0;
    }
    return (void *)work ;
}

#define ST_ICONNAME	0x4e430a	// sub_sticon
#define ST_L2DNAME	0x9deed4	// save_load

/* 初期化部メイン */
void *CODEC_NewSnakeTalesSave( void )
{
	Work *work;
	void *icon;
	int iconsize;

#ifdef BP_360
   bp_set_current_savegame( "S" );
   gBP_CanSelectNewDevice = 1;
#endif

	// printf("Height Limit= %d\n",SAVERES_FILEINFO_V2+SAVERES_FILEINFO_HEIGHT2);

	if((icon=GV_GetCache( GV_CacheID( ST_ICONNAME, 'r')))==NULL){
#ifdef DEBUG
		printf("CODEC_NewMCSave : icon not found\n");
#endif
		return NULL;
	}
	iconsize=MCAccessCalcIconSize(icon);

	if((work=(Work *)GV_Malloc(sizeof(Work)))==NULL){
#ifdef DEBUG
		printf("CODEC_NewMCSave : memory not enough\n");
#endif
		return NULL;
	}

	/* アクター起動 */
	GV_ZeroMemory(work,sizeof(Work));
	GV_SetActorFreeFunc(work,GV_DelayedFree);
	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(ACT_MODE,work,0x1FF); /* ACT_MODE <--- user/mode/codec/codecmem.h */

	GV_SetActor(&(work->actor),Act,Die);
	GV_ActorEX(&(work->actor));
	GV_SetActorSignalFunc(work,(int (*)(void *,int,int))SignalFunc);

	//	work->name=0;
	work->name=ST_ICONNAME;	// 名前を適当につける
	work->step=0;
	work->sub_step=0;
	work->ans=0;
	work->ans2=0;
	work->strman=NULL;
	work->finish_mode=0;
	work->end_proc=0;
	work->codec_flag=0;

	work->sound_flag=0;

	work->i_fileinfo=0;
	work->update_fileinfo=0;


	work->icon=icon;
	work->iconsize=iconsize;

	NewMCMan(&(work->mcman));
	work->mcman.file_kind = MCMAN_FILE_KIND_SNAKE_TALES;

	GV_SetActorChild(work,&(work->mcman));

	if(!CODEC_GetResources(work, ST_L2DNAME )){
		GV_DestroyActor(work);
		return NULL ;
	}
	if((work->strman=NewTextScreenControlForTitle())==NULL){
		GV_DestroyActor(work);
		return NULL;
	}

	InitMenu(work);
   
   work->mHidePsnWidget = 0;
	
   return work;
}

#endif









