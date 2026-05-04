//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   albam.c
   アルバムモード
   
   2001/06/07	S.Kobayashi
   $Id: albam.c,v 1.5 2002/12/05 18:42:02 takaki Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#if 0 //BP_PS2 def PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"gameheader.h"
#include <libutl.h>
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"
#include    "../../mode/codec/cjimaku.h"
#include    "../../kano/mcman/mcman.h"
#include "../../kano/titlescr/subtitle.h"
#include "libfs.h"

#include "mode/menu/xtextscn.h"

#define		LAYOUT		   (4691731)
#define 	STR_NODE_FONT  (10045112)
#define     STR_CURS       (6756848)
#define     STR_DEFAULT    (566267)
#define     STR_ROOT       (2770484)
#define 	STR_R_BAR      (5313689)
#define 	STR_G_BAR      (10556568)
#define		STR_B_BAR      (5400341)
#define 	STR_RGB_SELECT (15807010)
#define 	STR_SAVE_LOAD (10350292)
#define 	STR_SUB_BAR_NULL (2163524)
// action
#define STR_OPENRGB (8941000)
#define STR_CLOSERGB (6751495)
#define STR_OPEN_LIST (739237)
#define STR_OPENEDIT (504421)
#define STR_CLOSEEDIT_BACKUP (1398383)
#define STR_CLOSEEDIT (14326336)
#define STR_SELECTTITLE (8757611)
#define STR_SELECTCOLOR (7897559)
#define STR_SELECTEXIT (2942896)
#define STR_OPENNOWLOAD (20148)
#define STR_CLOSENOWLOAD (15611943)
#define STR_PRE_SCREEN_TO_FULL_SCREEN_OK (10624623)
#define STR_FULL_SCREEN_TO_PRE_SCREEN_OK (13667387)
#define STR_OPENOVERWRITE (12887728)
#define STR_CLOSEOVERWRITE (6977128)
#define STR_OPENOVERWRITE (12887728)
#define STR_CLOSEOVERWRITE (6977128)
#define STR_OPENLISTSLOT1 (8280576)
#define STR_OPENLISTSLOT2 (8280577)
#define STR_CLOSEEDITSLOT1 (1900344)
#define STR_CLOSEEDITSLOT2 (1900345)
#define STR_CLOSELIST (14561152)

// etc
#define 	DEFAULT_POS_X ( 58.0f )
#define		DEFAULT_POS_Y ( 100.0f )
#define 	FONT_WIDTH ( 18.0f )
#define 	FONT_HEIGHT ( 14.0f )
#define     FONT_SPACE_HEIGHT ( 8.0f )// DRAW_HEIGHT / 384.0f )
#define     DOT_POS_X ( 16.0f )
#define     DOT_POS_Y ( 10.0f )
#define 	DEFAULT_DOT_POS_X ( DEFAULT_POS_X - DOT_POS_X )
#define 	DEFAULT_DOT_POS_Y ( DEFAULT_POS_Y + DOT_POS_Y )
#define		DEFAULT_EDIT_FONT_POS_X (43)
#define 	DEFAULT_EDIT_FONT_POS_Y (317)
#define 	STR_WIDTH  (18) // strcode ではなく文字の長さ
#define 	STR_HEIGHT (14) // strcode ではなく文字の長さ
#define     RGB_DEFAULT_POS_X ( 246.0f )
#define     RGB_DEFAULT_POS_Y ( 245.0f )
#define     RGB_NUM_DEFAULT_POS_X ( 210.0f )
#define     RGB_SPACE_HEIGHT ( 20.0f )
#define     RGB_POS_Y ( 10.0f )

#define STR_MENU_OK (1844828)
#define STR_MENU_CANCEL (5921625)

// key
#define STR_ALBUM_MARUBATSU_RECT (11297303)
#define STR_SHOW_ALB_FILTER (15610570)
#define STR_HIDE_ALB_FILTER (3111368)
#define STR_SHOW_RGB_FILTER (602829)
#define STR_HIDE_RGB_FILTER (4880842)

#define STR_ALBAM_MARUBATSU (12637854)
#define STR_ALBAM_MARUBATSU_RGB (1925804)
#define STR_SHOW_ALB_MARUBATU (14265529)
#define STR_HIDE_ALB_MARUBATU (16097726)
#define STR_SHOW_RGB_MARUBATU (14267685)
#define STR_HIDE_RGB_MARUBATU (16099882)
#define HOKAN_SPEED (20)
// etc
#define STR_LOADING_TEX (15047160)
// color
#define SK_R (80) 
#define SK_G (90) 
#define SK_B (85) 
#define SK_A (52)
#define SK_LOW_A (52)
#define IN_FADE_SPEED ( DIRECT_TICK( 6 ) )
#define OUT_FADE_SPEED ( DIRECT_TICK( 12 ) )

#define OVER_WRITE (64)
#define OVER_WRITE_YES_NO (8) // plus 'yes / no'
#define OVER_WRITE_YES (3) // yes
#define OVER_WRITE_NO  (2) // yes

#define DEFAULT_OVER_WRITE_POS_X (41.0f)
#define DEFAULT_OVER_WRITE_POS_Y (33.0f)
#if 0
#  ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
#  define DEFAULT_OVER_WRITE_SELECT_POS_X (390.0f)
#  define DEFAULT_OVER_WRITE_SELECT_POS_Y (38.0f)
#  else
#  define DEFAULT_OVER_WRITE_SELECT_POS_X (390.0f)
#  define DEFAULT_OVER_WRITE_SELECT_POS_Y (33.0f)
#  endif
#else
#  define DEFAULT_OVER_WRITE_SELECT_POS_X (388.0f)
#  define DEFAULT_OVER_WRITE_SELECT_POS_Y ( BP_Area_EU() ? 38.0f : 33.0f )
#endif
#define DEFAULT_PAGE_FONT_POS_X (324)//(320)
#define DEFAULT_PAGE_FONT_POS_Y (72)
#define DEFAULT_PAGE_POS_X (391)
#define DEFAULT_PAGE_SURASHU_X (429)
#define DEFAULT_PAGE_POS_Y (72)
#define PAGE_FONT_MAX ( 5 ) // 00/00
#define PAGE_MAX (4)

#define DISP_PHOTO_MAX (10) // 一画面に表示する写真項目数の最大値
#define JPEG_DECODE_START_TIME (40) // 10 frame before start to the JPEG decodeing

// size
#define JPEG_ENCODE_SIZE_LIMIT ( 30720 ) // 30 * 1024
#define JPEG_SIZE_LIMIT		( 720*1280 )
#define CAPTURE_MEM_SIZE	(3*1024*1024)

// Japanease or Usa or Pal

//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
#if 0

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define CANCEL PAD_CANCEL //元はPAD_B

#else 

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define CANCEL PAD_CANCEL //元はPAD_B

#endif 


// kanoさん
#define N_FILES			MCSCR_N_FILES
#define NO_SAVEFILE		0xffff

enum {
	MCSCR_CHECK_CARD=0,
	MCSCR_GET_CARDINFO,
	MCSCR_WAIT_GET_CARDINFO,
	MCSCR_LIST_FILE,  // by koba4
	MCSCR_SEL_FILE,

	MCSCR_CONFIRM,
	MCSCR_START_SAVELOAD,
	MCSCR_WAIT_SAVELOAD,

	MCSCR_SUCCESS_SAVELOAD,
	MCSCR_FAILED_SAVELOAD,
};
/////////////////////////////////

// enum -> #define  chaned by T.Morita 2002.05.23
#define	SK_NORMAL                   I64(0x0000000000000001)
#define	SK_PHOTO_LOAD_OK            I64(0x0000000000000002)
#define	SK_PHOTO_SAVE_OK            I64(0x0000000000000004)
#define	SK_PHOTO_FADE_IN_START      I64(0x0000000000000008)
#define	SK_PHOTO_FADE_IN_OK         I64(0x0000000000000010)
#define	SK_PHOTO_FADE_OUT_START     I64(0x0000000000000020)
#define	SK_PHOTO_FADE_OUT_OK        I64(0x0000000000000040)
#define	SK_PAGE_FADE_IN_START       I64(0x0000000000000080)
#define	SK_PAGE_FADE_IN_OK          I64(0x0000000000000100)
#define	SK_PAGE_FADE_OUT_START      I64(0x0000000000000200)
#define	SK_PAGE_FADE_OUT_OK         I64(0x0000000000000400)
#define	SK_FONT_FADE_IN_START       I64(0x0000000000000800)
#define	SK_FONT_FADE_IN_OK          I64(0x0000000000001000)
#define	SK_FONT_FADE_OUT_START      I64(0x0000000000002000)
#define	SK_FONT_FADE_OUT_OK         I64(0x0000000000004000)
#define	SK_DOT_FADE_IN_START        I64(0x0000000000008000)
#define	SK_DOT_FADE_IN_OK           I64(0x0000000000010000)
#define	SK_DOT_FADE_OUT_START       I64(0x0000000000020000)
#define	SK_DOT_FADE_OUT_OK          I64(0x0000000000040000)
#define	SK_RGB_FADE_IN_START        I64(0x0000000000080000)
#define	SK_RGB_FADE_IN_OK           I64(0x0000000000100000)
#define	SK_RGB_FADE_OUT_START       I64(0x0000000000200000)
#define	SK_RGB_FADE_OUT_OK          I64(0x0000000000400000)
#define	SK_CHANCEL                  I64(0x0000000000800000)
#define	SK_PAGE_CHANGE_L            I64(0x0000000001000000)
#define	SK_PAGE_CHANGE_R            I64(0x0000000002000000) 
#define	SK_FULL_SCREEN_START        I64(0x0000000004000000) 
#define	SK_FULL_SCREEN_OK           I64(0x0000000008000000)
#define	SK_PRE_NORMAL_SCREEN_START  I64(0x0000000010000000)
#define	SK_PRE_NORMAL_SCREEN_OK     I64(0x0000000020000000)
#define	SK_NORMAL_SCREEN_START      I64(0x0000000040000000)
#define	SK_NORMAL_SCREEN_OK         I64(0x0000000080000000)
#define	SK_NAME_ENTRY               I64(0x0000000100000000)
#define	SK_RGB_CHENG                I64(0x0000000200000000)
#define	SK_ACTION_START             I64(0x0000000400000000)
#define	SK_ACTION_OK                I64(0x0000000800000000)
#define	SK_SIGNAL_OK                I64(0x0000001000000000)
#define	SK_OVER_WRITE_MODE          I64(0x0000002000000000)
#define	SK_MEMORY_FONT_FADE         I64(0x0000004000000000)
#define	SK_SAVE_WAIT                I64(0x0000008000000000)
#define	SK_SAVE_TIME                I64(0x0000010000000000)
#define	SK_MOVE_OK                  I64(0x0000020000000000)
#define	SK_NAME_ENTRY_END           I64(0x0000040000000000)
#define	SK_LIST_UP                  I64(0x0000080000000000)
#define	SK_FIRST_FADE_OK            I64(0x0000100000000000)
#define	SK_OPEN_NOW_LOAD            I64(0x0000200000000000)
#define	SK_DELAY                    I64(0x0000400000000000)
#define	SK_SAVE_START               I64(0x0000800000000000)
#define	SK_END                      I64(0x0001000000000000)


enum {
	SK_DESTROY = 0x1 ,
	SK_BREAK_FILE = 0x2 ,
};

enum {
	SK_FADE_IN = 0x1 ,
	SK_FADE_OUT = 0x2 ,
};

typedef struct {
	void      *work;
	u_char        r;
	u_char        g;
	u_char        b;
	u_char        a;
} Code ;

#define ASCCI_MAX (12 + 1) // plus 1 = tarminater
typedef struct _photo_list {
	struct _photo_list *pPrev;
	struct _photo_list *pNext;
	u_char name[ ASCCI_MAX ];
	int id;
	u_char r , g , b , a; // 色
	char flag;
} Photo_List;

typedef struct _albam_photo{
	void *data; // jpeg date
	MCScrWork *mcscr; // メモりーカード
	float x , y; // 始点
	float w , h; // 幅
	u_char r , g , b , a; // 色
	int   files; // ファイル総数
	int   id_max;
} Photo;

typedef struct {
	SPR_OBJ *page_now[ PAGE_FONT_MAX ]; // 現在ページ 00/00
	SPR_OBJ *page[ PAGE_MAX ]; // PAGE
	int      max_num; // 最大ページ数
	int      num;     // 現在のページ
	int      now_page_max; // 現在のページの枚数
	u_char   a;
} Page;

typedef struct {
	SPR_OBJ    *rgb[ 3 ]; // RGBチェンジ時のRGB 固定
	SPR_OBJ    *rgb_num[ 3 ][ 3 ]; // RGBチェンジ時のRGB 数字３桁
	SPR_OBJ    *rgb_bar[ 3 ]; // 動くやつ
	SPR_OBJ    *rgb_select; // 選択中の物
	u_char bak_r , bak_g , bak_b , bak_a; // 色
	u_char r , g , b , a; // 色
} Rgb;
	
typedef struct {
	SPR_OBJ             *dot_curs; // sprite
	u_char r , g , b , a;
} Dot;

typedef struct {
	int r;
	int g;
	int b;
	int a;
	void *work;
	char *data;
	int position; // resource 専用
} Font;

typedef struct {
	SPR_OBJ *ok;	
	SPR_OBJ *no;
	SPR_OBJ *dot;
	int      a[ 2 ];
	int      position;	
	SPR_OBJ *obj[ OVER_WRITE_YES_NO ];
	SPR_OBJ *hold_obj[ 2 ][ OVER_WRITE ]; // 変化することのないフォント 2 行
} OverWriteWork;

typedef struct {
	float alpha;
	float alpha_add;
} MemoryFont;

typedef struct {
	float p;
	int   count;
	int   code1;
	int   code2;
	int   morf_parts;
	u_long64  flag;
} Hokan;

typedef struct {
	float   p;
	int     count;
	int     start_y;
	int     end_y;
	u_long64  flag;
} Hokan_Program;

typedef	struct _albam_work {
	GV_ACT_EX			actor ;
	int					handle; // 外部
	int					handle_2d;
	SPR_OBJ             *font[ DISP_PHOTO_MAX ][ ASCCI_MAX ];
	SPR_OBJ             *edit_font[ ASCCI_MAX ];
	OverWriteWork       over_write; // 上書き
	Dot                 dot; // ドット
	Rgb                 rgb_menu; // rgb 変更
	Photo               photo;    // 画像データ
	Photo_List          *photo_list; // 項目リスト
	Page                page;
	MemoryFont          memory;
	u_long64              *photo_top_addres;
	Code                font_work;
	Hokan_Program       dot_hokan;
	int                 position; // ページ中のどこか？
	int                 pre_position; // edit screenのどこか
    int                 color_mode_position;
	int                 position_sign; // 何方に進んだか
	int                 action;
	int                 action_num;
	int                 base_u;
	int                 base_v;
	u_long64              flag;
	int                 pad_status;
	int                 pad_check;
	int                 hold_time;
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	int                 time;      // jpeg 展開までの時間
	int                 port;
	int                 end_time;
	int                 file_counter;
	int                 num_counter;
	int                 delay;
	int                 album_resource;
	Font                jpn_font;
	Hokan               hokan[ 2 ];
	void  ( *act )( struct _work * );
} Work ;

// プロトタイプ
static void PadAct( Work * );
static void AnimetionAct( Work * );
static void PhotoFade( Work *pWork ); // fade out = black
static void PageFade( Work *pWork );
static void FontFade( Work *pWork );
static void RGBFade( Work *pWork );
static void DotFade( Work *pWork );
static inline Photo_List *PhotoListForward( Photo_List * ); // リストを安全に進める
static inline Photo_List *PhotoListSearch( Photo_List * , int ); // リストを安全に進める
static inline Photo_List *PhotoListBack( Photo_List * ); // リストを安全に進める
static void OverWriteEnd( Work *pWork );
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan * );
static inline int MorfAct( Work *pWork , Hokan *pHokan );
static int PhotoListDataInput( Work *pWork , int id , int mode );

// extern my function
extern void *NewAlbamName( char *name_data , u_long64 *flag , int port , Photo_List * , int handle , void * , int );
extern void *NewBlackRect( void );
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_PrintfChengColor( SPR_OBJ *pObj , u_char r , u_char g , u_char b , u_char a );
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void SK_PintfChengColor( SPR_OBJ *pObj , u_char r , u_char g , u_char b , u_char a );
extern void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag );

// kano
extern void *GetLocalResource( int ref_id, int offset );

// font
extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
// jpeg
extern void BP_MENU_DrawPictureTexEX( int x, int y, int w, int h, int color,
                                     int photoNum, int u0, int v0, int u1, int v1, int flag );
extern void MENU_DrawPicture32( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );
extern int UTL_JpegDecode( void *image, int width, int height, void *code );
extern void *NewPictureDrawManager( int prio );
// mc
extern void *NewPortsel(int name,int parent_name,int l2d_handle,int title_mode,int start_mode);
// bp
extern int BP_LoadScreenShot_DigitalCamera_FromJPEG_MGS2(void* pJPEG_);

/*----------------------------------------------------------------*/
#if 0
#define SK_ALBUM_JAPAN (16727656) /* セーブゲームリソース日本  */
#define SK_ALBUM_ENGLISH (16727656) /* セーブゲームリソースアメリカ */
#define SK_ALBUM_FRANCE (16570686) /* セーブゲームリソースフランス語 */
#define SK_ALBUM_GERMAN (262847) /* セーブゲームリソースドイツ語 */
#define SK_ALBUM_ITARY (3895998) /* セーブゲームリソースイタリア語 */
#define SK_ALBUM_SPAIN (15842681) /* セーブゲームリソーススペイン語 */
#else
#define SK_ALBUM_JAPAN (16294855) /* セーブゲームリソース日本語 */
#define SK_ALBUM_ENGLISH (15454162) /* セーブゲームリソース英語 */
#define SK_ALBUM_FRANCE (16570686) /* セーブゲームリソースフランス語 */
#define SK_ALBUM_GERMAN (262847) /* セーブゲームリソースドイツ語 */
#define SK_ALBUM_ITARY (3895998) /* セーブゲームリソースイタリア語 */
#define SK_ALBUM_SPAIN (15842681) /* セーブゲームリソーススペイン語 */
#endif
static void LangChecge( Work *pWork ) // 言語切替関数
{
	pWork->album_resource = SK_ALBUM_JAPAN;
	if ( GM_Language == GM_LANG_JAPANESE ){
		pWork->album_resource = SK_ALBUM_JAPAN;
#ifdef DEBUG_MODE
		printf("Japanese\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_ENGLISH ){
		pWork->album_resource = SK_ALBUM_ENGLISH;
#ifdef DEBUG_MODE
		printf("English\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_FRENCH ){
		pWork->album_resource = SK_ALBUM_FRANCE;
#ifdef DEBUG_MODE
		printf("French\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_GERMANY ){
		pWork->album_resource = SK_ALBUM_GERMAN;
#ifdef DEBUG_MODE
		printf("Germany\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_ITALY ){
		pWork->album_resource = SK_ALBUM_ITARY;
#ifdef DEBUG_MODE
		printf("Italy\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_SPANISH ){
		pWork->album_resource = SK_ALBUM_SPAIN;
#ifdef DEBUG_MODE
		printf("Spanish\n");
#endif
		return;
	}
}

static int SprInit( Work *pWork ) // default を a
{
	SPR_OBJ		*spr;
	Photo_List *photo_list;
	int        i , j;
	u_long64      flag;

	if ( pWork->flag & SK_NORMAL ){
		return ( 0 );
	}
	// dotの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_CURS );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_HIDE( spr );
	pWork->dot.dot_curs = pWork->over_write.dot = spr;
	// page自体
	spr = L2D_GetObject( pWork->handle_2d , STR_NODE_FONT );
	if ( spr == NULL ){
		SK_Err("node_font1\0");
		return -1 ;
	}
	SPR_SHOW( spr );
#if 0 //BP_PS2 def PSX2
	pWork->base_u = spr->sprite.head.tex.u; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v;
#else
	pWork->base_u = spr->sprite.head.tex.u*16.0f; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v*16.0f;
#endif
	spr->sprite.dw = 14;
	spr->sprite.dh = 10;
	spr->sprite.pos.x = DEFAULT_PAGE_FONT_POS_X;
	spr->sprite.pos.y = DEFAULT_PAGE_FONT_POS_Y;
	pWork->page.page[ 0 ] = SPR_DuplicateTree( spr );
	for ( i = 1 ; i < PAGE_MAX ; i ++ ){ // 設定
		pWork->page.page[ i ] = SPR_DuplicateTree( spr );
		pWork->page.page[ i ]->sprite.pos.x = 0;
		pWork->page.page[ i ]->sprite.pos.y = DEFAULT_PAGE_FONT_POS_Y;
	}
	{
		char buf[] = "PAGE\0";

		SK_PrintfNormal( NULL , pWork->page.page , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_MAX );
		SK_PrintfNormal( buf , pWork->page.page , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_MAX );
	}
	SPR_HIDE( spr );
	// ページ用フォント取得
	spr = L2D_GetObject( pWork->handle_2d , STR_NODE_FONT );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
#if 0 //BP_PS2 def PSX2
	pWork->base_u = spr->sprite.head.tex.u; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v;
#else
	pWork->base_u = spr->sprite.head.tex.u*16.0f; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v*16.0f;
#endif
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_PAGE_POS_X;
	spr->sprite.pos.y = DEFAULT_PAGE_POS_Y-3;
	pWork->page.page_now[ 0 ] = SPR_DuplicateTree( spr );
	if ( pWork->page.page_now[ 0 ] == NULL ){
		SK_Err("Non Memory\0");
		return ( -1 );
	}
	for ( i = 1 ; i < PAGE_FONT_MAX ; i ++ ){ // 設定
		pWork->page.page_now[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->page.page_now[ i ] == NULL ){
			SK_Err("Non Memory\0");
			return ( -1 );
		}
		pWork->page.page_now[ i ]->sprite.pos.x = 0;
		pWork->page.page_now[ i ]->sprite.pos.y = DEFAULT_PAGE_POS_Y;
	}
	SK_PrintfNormal( NULL , pWork->page.page_now , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_FONT_MAX );
	SPR_HIDE( spr );
	// RGB
	spr = L2D_GetObject( pWork->handle_2d , STR_NODE_FONT );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = RGB_DEFAULT_POS_X;
	spr->sprite.pos.y = RGB_DEFAULT_POS_Y;
	for ( i = 0 ; i < 3 ; i ++ ){ // 設定 文字
		for ( j = 0 ; j < 3 ; j ++ ){ // 数字
			pWork->rgb_menu.rgb_num[ i ][ j ] = SPR_DuplicateTree( spr );
			if ( pWork->rgb_menu.rgb_num[ i ][ j ] == NULL ){
				SK_Err("Non Memory\0");
				return ( -1 );
			}
			pWork->rgb_menu.rgb_num[ i ][ j ]->sprite.pos.x = RGB_NUM_DEFAULT_POS_X;
			pWork->rgb_menu.rgb_num[ i ][ j ]->sprite.pos.y += ( RGB_SPACE_HEIGHT * i );
		}
		SK_PrintfNormal( NULL , pWork->rgb_menu.rgb_num[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , 3 );
	}
	SPR_HIDE( spr );
	// edit font
	spr = L2D_GetObject( pWork->handle_2d , STR_NODE_FONT );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
#if 0 //BP_PS2 def PSX2
	pWork->base_u = spr->sprite.head.tex.u; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v;
#else
	pWork->base_u = spr->sprite.head.tex.u*16.0f; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v*16.0f;
#endif
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_EDIT_FONT_POS_X;
	spr->sprite.pos.y = DEFAULT_EDIT_FONT_POS_Y;
	pWork->edit_font[ 0 ] = SPR_DuplicateTree( spr );
	if ( pWork->edit_font[ 0 ] == NULL ){
		SK_Err("Non Memory\0");
		return ( -1 );
	}
	for ( i = 1 ; i < ASCCI_MAX ; i ++ ){ // 数字
		pWork->edit_font[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->edit_font[ i ] == NULL ){
			SK_Err("Non Memory\0");
			return ( -1 );
		}
		pWork->edit_font[ i ]->sprite.pos.x = 0;
	}
	SK_PrintfNormal2( NULL , pWork->edit_font , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
	SPR_HIDE( spr );
	// Rgb_bar
	spr = L2D_GetObject( pWork->handle_2d , STR_R_BAR );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
	pWork->rgb_menu.rgb_bar[ 0 ] = spr;
	// rGb_bar
	spr = L2D_GetObject( pWork->handle_2d , STR_G_BAR );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
	pWork->rgb_menu.rgb_bar[ 1 ] = spr;
	// rgB_bar
	spr = L2D_GetObject( pWork->handle_2d , STR_B_BAR );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
	pWork->rgb_menu.rgb_bar[ 2 ] = spr;
	// RGB_Select
	spr = L2D_GetObject( pWork->handle_2d , STR_RGB_SELECT );
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
	pWork->rgb_menu.rgb_select = spr;
	// 文字の取得
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH - 1;
	spr->sprite.dh = FONT_HEIGHT - 1;
	spr->sprite.pos.x = DEFAULT_POS_X;
	spr->sprite.pos.y = DEFAULT_POS_Y;
	// forward process
	flag = SK_NORMAL;
	photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( photo_list->pNext != NULL ){
		photo_list = photo_list->pNext;
	}
	// main process
	for ( i = 0 ; i < DISP_PHOTO_MAX ; i ++ ){
		pWork->font[ i ][ 0 ] = SPR_DuplicateTree( spr );
		if ( pWork->font[ i ][ 0 ] == NULL ){
			SK_Err("Non Memory\0");
			return ( -1 );
		}
		pWork->font[ i ][ 0 ]->sprite.pos.y += ( ( FONT_HEIGHT + FONT_SPACE_HEIGHT ) * i );
		for ( j = 1 ; j < ASCCI_MAX ; j++ ){
			pWork->font[ i ][ j ] = SPR_DuplicateTree( spr );
			if ( pWork->font[ i ][ j ] == NULL ){
				SK_Err("Non Memory\0");
				return ( -1 );
			}
			pWork->font[ i ][ j ]->sprite.pos.x = 0;
			pWork->font[ i ][ j ]->sprite.pos.y += ( ( FONT_HEIGHT + FONT_SPACE_HEIGHT ) * i );
		}
		// font初期化
		SK_PrintfNormal2( NULL , pWork->font[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
		SK_PrintfChengColor2( pWork->font[ i ] , photo_list->r , photo_list->g , photo_list->b , photo_list->a , ASCCI_MAX );
	}
	SPR_HIDE( spr );
	// dot init
	pWork->dot_hokan.start_y = 0;
	pWork->dot_hokan.end_y = pWork->font[ 0 ][ 0 ]->sprite.pos.y;
	pWork->dot_hokan.count = 1;
	// over write
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("node_font3\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH - 3;
	spr->sprite.dh = FONT_HEIGHT - 3;
	spr->sprite.pos.x = DEFAULT_OVER_WRITE_POS_X;
	// main process
	for ( i = 0 ; i < 2 ; i ++ ){
		spr->sprite.pos.y = DEFAULT_OVER_WRITE_POS_Y + ( ( spr->sprite.dh + 2 ) * i );
		for ( j = 0 ; j < OVER_WRITE ; j ++ ){
			pWork->over_write.hold_obj[ i ][ j ] = SPR_DuplicateTree( spr );
		}
		// font初期化
		{
			char buf[] = "OVER WRITE OK?\0";

			SK_PrintfNormal( NULL , pWork->over_write.hold_obj[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );
			SK_PrintfNormal( buf , pWork->over_write.hold_obj[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );
		}
		SK_PrintfChengColor2( pWork->over_write.hold_obj[ i ] , SK_R , SK_G , SK_B , 128 , OVER_WRITE );
		SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
	}
	SPR_HIDE( spr );
	// over write yes/no
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("node_font3\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH - 3;
	spr->sprite.dh = FONT_HEIGHT - 3;
	spr->sprite.pos.x = DEFAULT_OVER_WRITE_SELECT_POS_X;
//#ifdef JAPANESE_BP_IGNORE()
//	spr->sprite.pos.y = DEFAULT_OVER_WRITE_SELECT_POS_Y + 13;
//#else
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	spr->sprite.pos.y = DEFAULT_OVER_WRITE_SELECT_POS_Y + 13;
//#else 
//	spr->sprite.pos.y = DEFAULT_OVER_WRITE_SELECT_POS_Y + 13;
//#endif
//#endif
	// main process
	for ( i = 0 ; i < OVER_WRITE_YES_NO ; i ++ ){
		pWork->over_write.obj[ i ] = SPR_DuplicateTree( spr );
	}
	// font初期化
	{
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
		char buf[] = "YES / NO\0";
//#endif
      
		SK_PrintfNormal( NULL , pWork->over_write.obj , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE_YES_NO );
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
      if ( !BP_Area_EU() )
      {
		   SK_PrintfNormal( buf , pWork->over_write.obj , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE_YES_NO );
      }
//#endif
	}
	SK_PrintfChengColor2( pWork->over_write.obj , SK_R , SK_G , SK_B , 0 , OVER_WRITE_YES_NO );
	SPR_HIDE( spr );
	SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );

	// over write ok
#if 0
	spr = L2D_GetObject( pWork->handle_2d , STR_MENU_OK );
	if ( spr == NULL ){
		SK_Err("ok\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	pWork->over_write.ok = spr;
	// over write no
	spr = L2D_GetObject( pWork->handle_2d , STR_MENU_CANCEL );
	if ( spr == NULL ){
		SK_Err("non\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	pWork->over_write.no = spr;
#endif
	// root hide -> show
	spr = L2D_GetObject( pWork->handle_2d , STR_ROOT );
	if ( spr == NULL ){
		SK_Err("spr\0");	
		return ( -1 );
	}
	SPR_SHOW( spr ); // 復活

	// flag set
	pWork->flag |= SK_NORMAL;

	return( 0 );
}

static void EditNameDisp( Work *pWork )
{
   // BP JG - change the colour of the hilight so the text is visible.
   SPR_OBJ *spr;
   spr = L2D_GetObject( pWork->handle_2d, 0x07f3b88 );
   spr->sprite.col.a = 0x40;

	SK_AllShow( pWork->edit_font , ASCCI_MAX ); // 一行いっきに書き換え
	SK_PrintfNormal2( pWork->photo_list->name , pWork->edit_font , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
	SK_PrintfChengColor2( pWork->edit_font , SK_R , SK_G , SK_B , 128 , ASCCI_MAX );
}

static void FontUpDate( Work *pWork )
{
	Photo_List *photo_list;
	int  i;
	int  alpha;
	int      tmp;

	// check
	if ( pWork->flag & SK_FONT_FADE_IN_START ){
		// forward process 
		tmp = 0;
		photo_list = ( Photo_List * )pWork->photo_top_addres;
		if ( photo_list->pNext != NULL ){
			photo_list = photo_list->pNext;
		}
		if ( pWork->flag & SK_PAGE_CHANGE_L ){
			photo_list = PhotoListSearch( photo_list , ( pWork->page.num ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
		} else if ( pWork->flag & SK_PAGE_CHANGE_R ){
			photo_list = PhotoListSearch( photo_list , ( pWork->page.num - 2 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
		} else {
			photo_list = PhotoListSearch( photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
		}
		if ( photo_list == NULL ){
			return;
		}
		// main process 
		for ( i = 0 ; i < pWork->page.now_page_max ; i ++ ){
			// font color
			if ( i == pWork->position ){
				if ( pWork->flag & SK_FIRST_FADE_OK ) {
					photo_list->a = 128;
					tmp++;
				} else {
					alpha = ( int )photo_list->a;
					if ( alpha < 128 ){
						alpha += IN_FADE_SPEED;
					} else {
						alpha -= IN_FADE_SPEED;
					}
					if ( alpha >= 128 ){
						photo_list->a = 128;
						pWork->flag |= SK_FIRST_FADE_OK; 
						tmp++;
					} else {
						photo_list->a = ( u_char )alpha;
					}
				}
			} else {
				alpha = ( int )photo_list->a;
				if ( alpha < SK_A ){
					alpha += OUT_FADE_SPEED;
					if ( alpha >= SK_A ){
						photo_list->a = SK_A;
						tmp++;
					} else {
						photo_list->a = ( u_char )alpha;
					}
				} else {
					alpha -= OUT_FADE_SPEED;
					if ( alpha <= SK_A ){
						photo_list->a = SK_A;
						tmp++;
					} else {
						photo_list->a = ( u_char )alpha;
					}
				}
//				alpha += ( 64 - photo_list->a ) / 10;
			}
			SK_PrintfNormal2( photo_list->name , pWork->font[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
			SK_PrintfChengColor2( pWork->font[ i ] , SK_R , SK_G , SK_B , photo_list->a , ASCCI_MAX );
			if ( photo_list->pNext == NULL ){
				i++;
				break;
			}		
			photo_list = PhotoListForward( photo_list );
		}
		if ( tmp >= i ){
			pWork->flag |= SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			SPR_SHOW( pWork->dot.dot_curs );
			printf("fadein ok %d\n" , i);
		}
	}
}

static void PageUpDate( Work *pWork ) // num -> ascci そして 入力
{
	float diff;
	char  buf[ PAGE_FONT_MAX+1 ];
	int   i;
	
	sprintf( buf , "%02d/%02d" , pWork->page.num , pWork->page.max_num );

	SK_PrintfNormal( buf , pWork->page.page_now , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_FONT_MAX);
	// 位置補正
	diff = DEFAULT_PAGE_SURASHU_X - pWork->page.page_now[ 2 ]->sprite.pos.x; // スラッシュの場所出し
	if ( diff != 0 ){ // 補正
		for ( i = 0 ; i < PAGE_FONT_MAX ; i ++ ){
			if ( i == 2 ){ // スラッシュ
				pWork->page.page_now[ i ]->sprite.pos.x = DEFAULT_PAGE_SURASHU_X;
			} else {
				pWork->page.page_now[ i ]->sprite.pos.x += diff;
			}
		}
	}
	SK_PrintfChengColor2( pWork->page.page_now , SK_R , SK_G , SK_B , pWork->page.a , PAGE_FONT_MAX );
	SK_PrintfChengColor2( pWork->page.page , SK_R , SK_G , SK_B , pWork->page.a , PAGE_MAX );
}

// リストを初期化する
static int PhotoListInitialize( Work *pWork ) // root の 取得
{
	pWork->photo_list = GV_Malloc( sizeof( Photo_List ) );
	if ( pWork->photo_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->photo_list , sizeof( Photo_List ) );
	pWork->photo_list->pPrev = NULL;
	pWork->photo_list->pNext = NULL;
	pWork->photo_top_addres = ( u_long64 * )pWork->photo_list;

	return ( 0 );
}

// リストに追加する
#define R_POSITION (13)
#define G_POSITION (14)
#define B_POSITION (15)
static int PhotoListDataInput( Work *pWork , int id , int mode )
{
#if 0
#  ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	//static u_char break_name[] = "DAMAGED DATA\0";
	static u_char *break_name = (u_char *)"DAMAGED DATA\0";	// Complier Error Taisaku
#  else
	//static u_char break_name[] = "DAMAGED FILE\0";
	static u_char *break_name = (u_char *)"DAMAGED FILE\0";	// Complier Error Taisaku
#  endif
#endif
	Photo_List *photo_list;
	u_char *pTmp;
	int i;
   const u_char *break_name = BP_Area_EU() ? (u_char *)"DAMAGED DATA\0" : (u_char *)"DAMAGED FILE\0";	// Complier Error Taisaku

	photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( photo_list->pNext != NULL ){
		photo_list = photo_list->pNext;
	}

	while ( photo_list->id != id ){
		if ( photo_list->pNext == NULL ){
			return ( -1 );
		}
		photo_list = PhotoListForward( photo_list );
	}
	// datain
	if ( mode == 1 ){
		pTmp = ( u_char * )MCManGetFileInfo( id );
		for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
			photo_list->name[ i ] = pTmp[ i ];
		}
		photo_list->r = pTmp[ R_POSITION ];
		photo_list->g = pTmp[ G_POSITION ];
		photo_list->b = pTmp[ B_POSITION ];
		photo_list->a = 0;
		photo_list->flag = 0;
	} else {
		for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
			photo_list->name[ i ] = break_name[ i ];
		}
		photo_list->r = 128;
		photo_list->g = 128;
		photo_list->b = 128;
		photo_list->a = 0;
		photo_list->flag = SK_BREAK_FILE;
	}

	return ( 0 );
}

static int PhotoListAdd( Work *pWork , int id ) // id sort plus
{
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
	while ( pWork->photo_list->pNext != NULL ){ // sort insert
		if ( pWork->photo_list->pPrev == NULL ){ // root は 判定しません
			pWork->photo_list = pWork->photo_list->pNext;
			continue;
		}
		if ( pWork->photo_list->id < id ){
			pWork->photo_list = pWork->photo_list->pNext;
			continue;
		}
		// insert
		{
			Photo_List *pNewPhotoList;

			// memory malloc
			pNewPhotoList = GV_Malloc( sizeof( Photo_List ) );
			if ( pNewPhotoList == NULL ){
				printf( " memory malloc failed\n" );
				return ( -1 );
			}
			GV_ZeroMemory( pNewPhotoList , sizeof( Photo_List ) );
			pNewPhotoList->id = id;
#if 0
			// data input
			pTmp = ( char * )MCManGetFileInfo( id );
			for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
				pNewPhotoList->name[ i ] = pTmp[ i ];
			}
			pNewPhotoList->r = pTmp[ R_POSITION ];
			pNewPhotoList->g = pTmp[ G_POSITION ];
			pNewPhotoList->b = pTmp[ B_POSITION ];
			pNewPhotoList->a = 0;
#endif
			// chain
			pNewPhotoList->pPrev = pWork->photo_list->pPrev;
			pNewPhotoList->pNext = pWork->photo_list;
			pWork->photo_list->pPrev->pNext = pNewPhotoList;
			pWork->photo_list->pPrev = pNewPhotoList;
		}
		return ( 0 );
	}
	// bottom plus
	pWork->photo_list->pNext = GV_Malloc( sizeof( Photo_List ) );
	if ( pWork->photo_list->pNext == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->photo_list->pNext , sizeof( Photo_List ) );
	pWork->photo_list->pNext->pPrev = pWork->photo_list;
	pWork->photo_list = pWork->photo_list->pNext;
	pWork->photo_list->pNext = NULL;
	pWork->photo_list->id = id;
#if 0
	// data input
	pTmp = ( char * )MCManGetFileInfo( id );
	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
	  pWork->photo_list->name[ i ] = pTmp[ i ];
	}
	pWork->photo_list->r = pTmp[ R_POSITION ];
	pWork->photo_list->g = pTmp[ G_POSITION ];
	pWork->photo_list->b = pTmp[ B_POSITION ];
	pWork->photo_list->a = 0;
#endif

	return ( 0 );
}

static inline Photo_List *PhotoListBack( Photo_List *pPhoto_list ) // リストを安全に戻す
{
	if ( pPhoto_list->pPrev != NULL ){
		return ( pPhoto_list->pPrev );
	}
	return ( pPhoto_list );
}

static inline Photo_List *PhotoListForward( Photo_List *pPhoto_list ) // リストを安全に進める
{
	if ( pPhoto_list->pNext != NULL ){
		return ( pPhoto_list->pNext );
	}
	return ( pPhoto_list );
}

#if 0
static void ListDebug( Work *pWork ) // 内容を全表示
{
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
	while ( pWork->photo_list->pNext != NULL ){
		printf("prev = %p my = %p next %p id = %d data = %d %d %d \n" , pWork->photo_list->pPrev , pWork->photo_list , pWork->photo_list->pNext , pWork->photo_list->id , pWork->photo_list->r , pWork->photo_list->g , pWork->photo_list->b );
		pWork->photo_list = pWork->photo_list->pNext;
	}
	printf("prev = %p my = %p next %p id = %d data = %d %d %d \n" , pWork->photo_list->pPrev , pWork->photo_list , pWork->photo_list->pNext , pWork->photo_list->id , pWork->photo_list->r , pWork->photo_list->g , pWork->photo_list->b );
	// 初期化
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( pWork->photo_list->pNext != NULL ){
		pWork->photo_list = pWork->photo_list->pNext;
	}
}
#endif

static inline Photo_List *PhotoListSearch( Photo_List *pPhoto_list , int position ) // pWork->position の 値まで移動
{
	Photo_List *tmp;
	int count;

	tmp = pPhoto_list;
	count = 0;

	// 先行チェック
	if ( tmp->pPrev == NULL && tmp->pNext == NULL ){
		return ( NULL );
	}
	while ( count < position ){
		if ( tmp->pNext == NULL ){
			return ( NULL );
		}
		tmp = tmp->pNext;
		count++;
	}
	return ( tmp );
}

// 全てのリストを解放
static void PhotoListFree( Work *pWork )
{
	Photo_List *pTmp;

	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
	while ( pWork->photo_list->pNext != NULL ) {
		pTmp = pWork->photo_list;
		pWork->photo_list = pTmp->pNext;
		GV_DelayedFree( pTmp );
	}
	GV_DelayedFree( pWork->photo_list );
}

static void MCScrBackCheckCard(MCScrWork *work)
{
#if 0
	work->step=MCSCR_CHECK_CARD;
	work->flags=0;
	work->n_indexes=0;
	work->save_index=NO_SAVEFILE;
#endif
#ifdef DEBUG_MODE
	printf("Next is MCSCR_CHECK_CARD %d\n" , work->step );
#endif
}

#if 0
static void MCScrBackGetCardInfo(MCScrWork *work)
{
	work->step=MCSCR_GET_CARDINFO;
	work->flags=0;
	work->n_indexes=0;
	work->save_index=NO_SAVEFILE;

#ifdef DEBUG_MODE
	printf("Next is MCSCR_GET_CARDINFO\n");
#endif
}
#endif
#if 0
static void SearchSaveFile(MCScrWork *work)
{
	int i=0;
	while(i<FILE_MAX){
		if(!MCManCheckFileFlag(i)){
			work->save_index=i;
			return;
		}
		i++;
	}
	work->save_index=NO_SAVEFILE;
}

static int SearchUpFile(int start)
{
	int i=start;
	while(i<FILE_MAX){
		if(MCManCheckFileFlag(i)) return i;
		i++;
	}
	return -1;
}


static int SearchDownFile(int start)
{
	int i=start;
	while(i>=0){
		if(MCManCheckFileFlag(i)) return i;
		i--;
	}
	return -1;
}

static void InitSearchFile(MCScrWork *work)
{
	int i;
	int start=0;

	for(i=0;i<N_FILES;i++){
		int j;
		j=SearchUpFile(start);
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
#endif

static int ListAutoInsert( Work *pWork ) // バックグラウンド動作
{
	if ( pWork->file_counter > MCManMaxId() ){
		return ( 0 );
	}
	if ( MCManCheckingID() <= pWork->file_counter ){
		return ( 0 );
	}

	switch ( MCManCheckExactFileFlag( pWork->file_counter ) ){
	case 1 : // 正常
		if ( PhotoListDataInput( pWork , pWork->file_counter , 1 ) < 0 ){
			pWork->file_counter++;
			return ( -1 );
		}
		pWork->num_counter++;
		break;
	case -1 : // 異常
		if ( PhotoListDataInput( pWork , pWork->file_counter , -1 ) < 0 ){
			pWork->file_counter++;
			return ( -1 );
		}
		pWork->num_counter++;
		break;
	case 0 :
		break;
	}
	pWork->file_counter++;

	return ( 0 );
}

static int ListAll( Work *pWork ) // idを全て登録
{
	int count;
	int i;

	count = 0;
	for ( i = 0 ; i <= pWork->photo.id_max ; i ++ ){
		if( MCManCheckExactFileFlag( i ) != 0 ){
			if ( PhotoListAdd( pWork , i ) < 0 ){
				return ( -1 );
			}
			count++;
		}
	}
	pWork->photo.files = count;
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres; // 先頭に戻す
	if ( pWork->photo_list->pNext != NULL ){
		pWork->photo_list = pWork->photo_list->pNext;
		if ( pWork->photo_list->pNext == NULL ){
			printf("なにもない\n");
		}
	} else {
		return ( -1 );
	}
	return ( 0 );
}

static int MemoryAct( Work *pWork )
{
	MCScrWork *work;

	work = pWork->photo.mcscr;
	switch( work->step ){
	case MCSCR_LIST_FILE:
	case MCSCR_SEL_FILE:
		if(MCManCheckingOrChecked()){
			MCScrBackCheckCard(work);
			break;
		}
		break;
	case MCSCR_START_SAVELOAD:
		if( work->file_cursor == work->n_indexes ){
			work->id = work->save_index;
		}

		switch(work->mode){
		case MCSCR_MODE_LOADPHOTO:
			if( MCManChecked() ){
				if( MCManIsMCChanged( pWork->port ) ){
					if ( pWork->flag & SK_NORMAL ){
						// 親に送信
						GV_CallParentSignalFunc( pWork , SK_DESTROY , 0 );
						GV_DestroyActor( pWork );
						pWork->flag |= SK_END;
						return (-1);
					}
					MCScrBackCheckCard(work);
					return -1;
				}
			}
			printf("load start\n");
			if(LoadPhotoStart( pWork->port , pWork->photo_list->id , work->data ) != 0 ){
				work->step++;
			}
			break;
		case MCSCR_MODE_SAVEPHOTO:
			/* アイコン関連の設定を入れる予定 */
			MCMan_PhotoIconEncode(work->icondata);

			if(SavePhotoStart(pWork->port,work->id,work->data,work->size,
							  NULL,0,work->info)){

				work->step++;
			}
			else{
				MCScrBackCheckCard(work);
			}
			break;
		}
		break;
	case MCSCR_WAIT_SAVELOAD:
		switch(MCManGetResult()){
		case 1:
			work->step=MCSCR_SUCCESS_SAVELOAD;
			work->counter=0;
			break;
		case -1:
			work->step=MCSCR_FAILED_SAVELOAD;
			work->counter=0;
			pWork->delay = 60;
			pWork->flag |= SK_DELAY;
			break;
		}
		break;

	case MCSCR_SUCCESS_SAVELOAD:
		if(MCManCheckingOrChecked()){
			MCScrBackCheckCard(work);
			break;
		}
		work->counter++;
		if(work->counter>3*60){
			switch(work->mode){
			case MCSCR_MODE_LOADPHOTO: /* Load Photo */
			case MCSCR_MODE_SAVEPHOTO: /* Save Photo */
				;
			}
			return 1;
		}
		break;
	case MCSCR_FAILED_SAVELOAD:
		work->counter++;
		if(work->counter>3*60){
			MCScrBackCheckCard(work);
		}
		break;
	}
	return 0;
}

static void DamagedCheck( Work *pWork )
{
	Photo_List *photo_list;

	// 20010907追加
	photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( photo_list->pNext != NULL ){
		photo_list = photo_list->pNext;
	}
	photo_list = PhotoListSearch( photo_list , ( ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ) + pWork->position ); // pWork->position の 値まで移動
	if ( photo_list == NULL ){
		return;
	}
	if ( pWork->flag & SK_DELAY ){
		if ( pWork->delay <= 0 ){
			MCManSetDamagedFlag( photo_list->id );
			PhotoListDataInput( pWork , photo_list->id , -1 );
			SK_PrintfNormal2( photo_list->name , pWork->font[ pWork->position ] ,
							  pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
			photo_list->a = 128;
			SK_PrintfChengColor2( pWork->font[ pWork->position ] , SK_R , SK_G , SK_B , photo_list->a , ASCCI_MAX );
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->action = STR_CLOSENOWLOAD;
			pWork->flag &= ~SK_DELAY;
		} else {
			pWork->delay--;
		}
	}
}

static void MemoryLoad( Work *pWork ) // サムネイルにメモりーカードからデータを読み込む
{
	int type;

	// 差さってるか
	if ( MCManChecked() ) {
		type = MCManIsMCChanged( pWork->port );
		printf("type %d\n" , type );
		if ( type != 0 ){
			// 親に送信
			GV_CallParentSignalFunc( pWork , SK_DESTROY , 0 );
			GV_DestroyActor( pWork );
			pWork->flag |= SK_END;
			return;
		}
	}
	MemoryAct( pWork );

	// file数の取得
	switch ( pWork->photo.mcscr->step ){
	case MCSCR_SEL_FILE :
		if ( ( pWork->time >= DIRECT_TICK( JPEG_DECODE_START_TIME ) ) && ( pWork->flag & SK_PHOTO_FADE_OUT_OK ) && 
			 ( pWork->flag & SK_NORMAL ) && !( pWork->flag & SK_OPEN_NOW_LOAD ) && ( pWork->flag & SK_NORMAL_SCREEN_OK ) && 
			 !( pWork->photo_list->flag & SK_BREAK_FILE ) ){
			pWork->action = STR_OPENNOWLOAD;
			pWork->flag |= SK_ACTION_START | SK_OPEN_NOW_LOAD;
			pWork->flag &= ~SK_ACTION_OK;
		}
		break;
	case MCSCR_SUCCESS_SAVELOAD :
		break;
	default :
		break;
	}
	if ( pWork->photo.mcscr->step > MCSCR_LIST_FILE && pWork->flag & SK_NORMAL ){ // 
		PhotoFade( pWork ); // load ok なら 始動
		PageFade( pWork );
		FontFade( pWork );
		RGBFade( pWork ); // update
		DotFade( pWork );
	}
}

static void PhotoToJpeg( Work *pWork )
{
	if ( ( pWork->photo.mcscr->step == MCSCR_SUCCESS_SAVELOAD ) && ( pWork->flag & SK_PHOTO_FADE_OUT_OK ) && !( pWork->flag & SK_PHOTO_LOAD_OK ) && 
		 ( pWork->dot_hokan.flag & SK_MOVE_OK ) && !( pWork->photo_list->flag & SK_BREAK_FILE ) )
   {
      BP_LoadScreenShot_DigitalCamera_FromJPEG_MGS2( pWork->photo.mcscr->data );
		//UTL_JpegDecode( pWork->photo.data , DRAW_WIDTH , DRAW_HEIGHT , pWork->photo.mcscr->data); // decode
		pWork->flag |= SK_PHOTO_LOAD_OK;
		pWork->flag |= SK_PHOTO_FADE_IN_START;
		pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
		pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->action = STR_CLOSENOWLOAD;
#ifdef DEBUG
		printf("decode _ ok !\n");
#endif
	}
}


#define PHOTO_MAIN_WIDTH ( 720 )
#define PHOTO_MAIN_HEIGHT ( 128 )

#define PHOTO_FULL_WIDTH ( 1280 )
#define PHOTO_FULL_HEIGHT ( 720 )

static void PhotoDisp( Work *pWork ) // 写真の描画
{
	u_char        *pTmp;
	int       mem_count;
	int          height;
	int     trance_size;
	int               i;
	int           add_y;
	int         photo_y;
	float         raute;
	float          ftmp;
	u_int         color;

	pTmp = ( u_char * )pWork->photo.data;
	// err chack
	if ( ( pTmp == NULL ) || ( pWork->photo_list == NULL ) || ( pWork->photo_list->pPrev == NULL ) ){ // root の 場合もはじく
		return;
	}
	mem_count = 0;
	photo_y = pWork->photo.y;
	height = PHOTO_MAIN_HEIGHT;
	trance_size = ( int )( ( PHOTO_FULL_HEIGHT + 127 ) / height );
	add_y = ( int )( pWork->photo.h ) / ( trance_size );
	color = ( pWork->photo.a << 24 ) | ( pWork->photo_list->b << 16 ) | ( pWork->photo_list->g << 8 ) | pWork->photo_list->r; 
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL	
   if ( BP_Area_EU() )
   {
	   raute = 0;
   }
   else
   {
//#else 
	   raute = ( pWork->photo.h - add_y * ( trance_size - 1 ) );
	   raute /= ( float )( trance_size );
   }
//#endif
#if 1 // BP_REDNER
   BP_MENU_DrawPictureTexEX((int)pWork->photo.x, (int)pWork->photo.y,
      pWork->photo.w + 1, pWork->photo.h, color, 
      -2, 
      0, 0, PHOTO_FULL_WIDTH, PHOTO_FULL_HEIGHT , 0 );
#else
	for( i = 0 ; i < trance_size ; i ++ )
   {
		MENU_DrawPicture32( ( int )pWork->photo.x, photo_y,
							 pWork->photo.w , add_y + raute , color , pTmp , PHOTO_MAIN_WIDTH , height , 0 );
		mem_count += height;
		photo_y += add_y + raute;
		pTmp += ( PHOTO_MAIN_WIDTH * height * 4 );
		if ( PHOTO_FULL_HEIGHT - mem_count < height )
      {
			add_y = ( pWork->photo.h - ( ( add_y + raute ) * ( trance_size - 1 ) ) );
			height = PHOTO_FULL_HEIGHT - mem_count;
		}
	}
#endif
}

// 文字の表示
static void StringDisp( Work *pWork )
{
	u_int color;
	int   x1 , y1 , x2 , y2 , u1 , v1;
	int   disp_width; 
	int   poly_width;

	if ( pWork->jpn_font.data == NULL ){
		return;
	}
	disp_width = 30 * SUBTITLE_FONT_DISP_WIDTH;	
	poly_width = 30 * SUBTITLE_FONT_WIDTH;
	// color
	color = ( 128 << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R; 
	x1 = DEFAULT_OVER_WRITE_POS_X;
	y1 = DEFAULT_OVER_WRITE_POS_Y + 7;
	x2 = x1 + disp_width;
	y2 = y1 + SUBTITLE_FONT_DISP_HEIGHT;
	u1 = poly_width;
   if ( BP_Area_EU() )
   {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   	v1 = SUBTITLE_FONT_HEIGHT + 6;
   }
   else
   {
//#else
   	v1 = SUBTITLE_FONT_HEIGHT;
   }
//#endif
	MENU_PutTextScreen( pWork->jpn_font.work , x1 , y1 , x2 , y2 ,
						1 , 1  , u1 + 1 , v1 + 1 , color );
}

//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
static SPR_OBJ **SetSpriteY( Work *pWork )
{
	if ( pWork->jpn_font.position == 4 ){
		SK_PrintfNormal( NULL , pWork->over_write.hold_obj[ 1 ]  , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );
		return ( pWork->over_write.hold_obj[ 0 ] );
	} else { // 通常
		SK_PrintfNormal( NULL , pWork->over_write.hold_obj[ 0 ]  , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );
		return ( pWork->over_write.hold_obj[ 1 ] );
	}
}
//#endif

#define ENGLISH_STAGE (29)
// 文字の形成
static void StringCleate( Work *pWork )
{
	int poly_width;
//#ifndef JAPANESE_BP_IGNORE()
	u_char ptr[ 64 ];
//#endif

	MENU_ClearTextTexture( pWork->jpn_font.work );
	if ( pWork->jpn_font.position == 0xff ){
		return;
	}
	// create
	poly_width = 30 * SUBTITLE_FONT_WIDTH;

#if 0
#ifdef JAPANESE_BP_IGNORE()
	pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , pWork->jpn_font.position ));
//	GM_GetResource( pWork->album_resource , pWork->jpn_font.position );
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT + 1 ,
							0 , 0 , pWork->jpn_font.data );
#else // 海外
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , pWork->jpn_font.position ));
//	pWork->jpn_font.data = GM_GetResource( pWork->album_resource , pWork->jpn_font.position );
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT + 1 ,
							0 , 0 , pWork->jpn_font.data );
#else 
	pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , pWork->jpn_font.position ));
//	pWork->jpn_font.data  = GM_GetResource( pWork->album_resource , pWork->jpn_font.position );
	SK_ToUpper( ptr , pWork->jpn_font.data );
	SK_PrintfNormal( ptr , SetSpriteY( pWork ) , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );

	SK_AllShow( pWork->over_write.hold_obj[ 0 ] , OVER_WRITE );
	SK_AllShow( pWork->over_write.hold_obj[ 1 ] , OVER_WRITE );
	SK_PrintfChengColor2( pWork->over_write.hold_obj[ 0 ] , SK_R , SK_G , SK_B , 128 , OVER_WRITE );
	SK_PrintfChengColor2( pWork->over_write.hold_obj[ 1 ] , SK_R , SK_G , SK_B , 128 , OVER_WRITE );
#endif
#endif

#else
	pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , pWork->jpn_font.position ));
//	GM_GetResource( pWork->album_resource , pWork->jpn_font.position );
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT + 1 ,
							0 , 0 , 0 , pWork->jpn_font.data );
#endif
}

enum {
  SET_COLOR_R = 0x0 ,
  SET_COLOR_G = 0x1 ,
  SET_COLOR_B = 0x2 ,
};

static void OverWrite( Work *pWork )
{
	u_char tmp[ 16 ];
	int            i;

	// メモりーカードに書きこみ
	// memory card 
	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
		tmp[ i ] = pWork->photo_list->name[ i ];
	}
	tmp[ R_POSITION ] = ( u_char )pWork->photo_list->r;
	tmp[ G_POSITION ] = ( u_char )pWork->photo_list->g;
	tmp[ B_POSITION ] = ( u_char )pWork->photo_list->b;
	pWork->jpn_font.position = 5;

	StringCleate( pWork );
	SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
#if 0
	// 文字書き換え
	{
		// セーブ中
		char buf[][64] = { "NOW SAVING." , "DO NOT REMOVE THE MEMORY CARD (PS2)." };
		int  i;

		for ( i = 0 ; i < 2 ; i ++ ){
			SK_PrintfNormal( NULL , pWork->over_write.hold_obj[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );
		}
		SK_PrintfNormal( buf[ 0 ] , pWork->over_write.hold_obj[ 0 ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , 11 );
		SK_PrintfNormal( buf[ 1 ] , pWork->over_write.hold_obj[ 1 ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , 36 );
		SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
		// 色をもどす
		pWork->over_write.a[ 0 ] = SK_LOW_A;
		pWork->over_write.a[ 1 ] = 128;
		for( i = 0 ; i < OVER_WRITE_YES ; i ++ ){
			pWork->over_write.obj[ i ]->sprite.col.a = pWork->over_write.a[ 0 ];
		}
		pWork->over_write.obj[ OVER_WRITE_YES + 1 ]->sprite.col.a = 128;
		for( i = OVER_WRITE_YES + 3 ; i < OVER_WRITE_YES_NO ; i ++ ){
			pWork->over_write.obj[ i ]->sprite.col.a = pWork->over_write.a[ 1 ];
		}
	}
#endif

	{
		u_char title[0x10];

		memcpy(title,tmp,12);
		title[12]=' ';

		for(i=0;i<12;i++){
			if(title[i]==0x80) title[i]=' ';
		}
		for(i=11;i>=0;i--){
			if(title[i]!=' '){
				title[i+1]='\0';
				break;
			}
		}
		if(i==-1) title[0]='\0';

		MCMan_SetPhotoTitle(title);
	}

	if ( ChangePhotoInfoStart( pWork->port , pWork->photo_list->id , tmp ) != 0 ){
		pWork->flag |= SK_SAVE_START;
	} else {
#ifdef DEBUG_MODE
		printf("Try Agein\n");
#endif
	}
}

#if 0
static inline void OverWriteDotUpdate( Work *pWork )
{
	switch ( pWork->over_write.position ){
	case 0 : // ok
		pWork->over_write.dot->empty.pos.y = -22;
		break;
	case 1 : // cancel
		pWork->over_write.dot->empty.pos.y = 0;
		break;
	}
}
#endif

static void OverWriteFontControl( Work *pWork ) // over write menu control
{
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
	int i;
//#endif

	if ( !( pWork->flag & SK_OVER_WRITE_MODE ) ){
		return;
	}
	switch ( pWork->over_write.position ){
	case 0 : // Yes
		if ( pWork->over_write.a[ 0 ] < 128 ){
			pWork->over_write.a[ 0 ] += IN_FADE_SPEED;
			if ( pWork->over_write.a[ 0 ] > 128 ){
				pWork->over_write.a[ 0 ] = 128;
			}
		} else {
			pWork->over_write.a[ 0 ] -= OUT_FADE_SPEED;
			if ( pWork->over_write.a[ 0 ] < 128 ){
				pWork->over_write.a[ 0 ] = 128;
			}
		}
		if ( pWork->over_write.a[ 1 ] < 64 ){
			pWork->over_write.a[ 1 ] += IN_FADE_SPEED;
			if ( pWork->over_write.a[ 1 ] > 64 ){
				pWork->over_write.a[ 1 ] = 64;
			}
		} else {
			pWork->over_write.a[ 1 ] -= OUT_FADE_SPEED;
			if ( pWork->over_write.a[ 1 ] < 64 ){
				pWork->over_write.a[ 1 ] = 64;
			}
		}
		break;
	case 1 : // No
		if ( pWork->over_write.a[ 0 ] < 64 ){
			pWork->over_write.a[ 0 ] += IN_FADE_SPEED;
			if ( pWork->over_write.a[ 0 ] > 64 ){
				pWork->over_write.a[ 0 ] = 64;
			}
		} else {
			pWork->over_write.a[ 0 ] -= OUT_FADE_SPEED;
			if ( pWork->over_write.a[ 0 ] < 64 ){
				pWork->over_write.a[ 0 ] = 64;
			}
		}
		if ( pWork->over_write.a[ 1 ] < 128 ){
			pWork->over_write.a[ 1 ] += IN_FADE_SPEED;
			if ( pWork->over_write.a[ 1 ] > 128 ){
				pWork->over_write.a[ 1 ] = 128;
			}
		} else {
			pWork->over_write.a[ 1 ] -= OUT_FADE_SPEED;
			if ( pWork->over_write.a[ 1 ] < 128 ){
				pWork->over_write.a[ 1 ] = 128;
			}
		}
		break;
	}
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   if ( BP_Area_EU() )
   {
	   if ( !( pWork->over_write.obj[ 0 ]->head.flags & SPR_FLAG_HIDDEN ) ){
		   {
   #ifndef KP_WINDOWS
			   u_char string[ 2 ] = "/\0";
   #else
			   char string[ 2 ] = "/\0";
   #endif
			   int    yes_width;
			   int    center_width;
			   int    no_width;
			   int    x1 , y1 , x2 , y2 , u1 , v1 , u2 , v2;
			   int    disp_width , poly_width;
			   u_int  color;

			   yes_width = SUBTITLE_FONT_WIDTH * 3;
			   center_width = SUBTITLE_FONT_WIDTH;
			   no_width = SUBTITLE_FONT_WIDTH * 3;
			   pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , 12 ));
			   MENU_CreateTextTexture( pWork->jpn_font.work , DEFAULT_OVER_WRITE_SELECT_POS_X , SUBTITLE_FONT_HEIGHT + 4 ,
									   yes_width , SUBTITLE_FONT_HEIGHT , 0 , 0 , 0 , pWork->jpn_font.data );
			   MENU_CreateTextTexture( pWork->jpn_font.work , DEFAULT_OVER_WRITE_SELECT_POS_X + yes_width , SUBTITLE_FONT_HEIGHT + 4 ,
									   center_width , SUBTITLE_FONT_HEIGHT , 0 , 0 , 0 , string );
			   pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , 13 ));
			   MENU_CreateTextTexture( pWork->jpn_font.work , DEFAULT_OVER_WRITE_SELECT_POS_X + yes_width + center_width , SUBTITLE_FONT_HEIGHT + 4 ,
									   no_width , SUBTITLE_FONT_HEIGHT , 0 , 0 , 0 , pWork->jpn_font.data );
			   // put
			   yes_width = SUBTITLE_FONT_DISP_WIDTH * 3;
			   center_width = SUBTITLE_FONT_DISP_WIDTH * 1;
			   no_width = SUBTITLE_FONT_DISP_WIDTH * 3;
			   disp_width = 3 * SUBTITLE_FONT_DISP_WIDTH;	
			   poly_width = 3 * SUBTITLE_FONT_WIDTH;
			   x1 = DEFAULT_OVER_WRITE_SELECT_POS_X;
			   y1 = DEFAULT_OVER_WRITE_SELECT_POS_Y;
			   x2 = x1 + yes_width;
			   y2 = y1 + SUBTITLE_FONT_DISP_HEIGHT;
			   u1 = DEFAULT_OVER_WRITE_SELECT_POS_X;
			   v1 = SUBTITLE_FONT_HEIGHT + 4;
			   u2 = DEFAULT_OVER_WRITE_SELECT_POS_X + poly_width;
			   v2 = ( SUBTITLE_FONT_HEIGHT * 2 ) + 4;
			   color = ( pWork->over_write.a[ 0 ] << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R; 
			   MENU_PutTextScreen( pWork->jpn_font.work , x1 , y1 , x2 , y2 ,
								   u1 , v1  , u2 , v2 + 1 , color );
   			
			   disp_width = SUBTITLE_FONT_DISP_WIDTH;	
			   poly_width = SUBTITLE_FONT_WIDTH;
			   x1 = x2 - 8;
			   x2 = x1 + center_width;
			   u1 = u2;
			   u2 = u1 + poly_width;
			   color = ( 128 << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R; 
			   MENU_PutTextScreen( pWork->jpn_font.work , x1 , y1 , x2 , y2 ,
								   u1 , v1 , u2 , v2 + 1 , color );
			   disp_width = 3 * SUBTITLE_FONT_DISP_WIDTH;	
			   poly_width = 3 * SUBTITLE_FONT_WIDTH;
			   x1 = x2;
			   x2 = x1 + no_width;
			   u1 = u2;
			   u2 = u1 + poly_width;
			   color = ( pWork->over_write.a[ 1 ] << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R; 
			   MENU_PutTextScreen( pWork->jpn_font.work , x1 , y1 , x2 , y2 ,
								   u1 , v1 , u2 , v2 + 1 , color );
		   }
	   }
   }
   else
   {
//#else
	   for( i = 0 ; i < OVER_WRITE_YES ; i ++ ){
		   pWork->over_write.obj[ i ]->sprite.col.a = pWork->over_write.a[ 0 ];
	   }
	   pWork->over_write.obj[ OVER_WRITE_YES + 1 ]->sprite.col.a = 128;
	   for( i = OVER_WRITE_YES + 3 ; i < OVER_WRITE_YES_NO ; i ++ ){
		   pWork->over_write.obj[ i ]->sprite.col.a = pWork->over_write.a[ 1 ];
	   }
   }
//#endif
#if 0
	// 対象オブジェクトの選択
	switch( pWork->over_write.position ){
	case 0 : // select is ok
		select = pWork->over_write.ok;
		non_select = pWork->over_write.no;
		break;
	case 1 : // select is no
		select = pWork->over_write.no;
		non_select = pWork->over_write.ok;
		break;
	default :
		return;
	}
	// alpha control
	// select
	alpha = ( int )select->sprite.col.a;
	if ( alpha < 128 ){
		alpha += IN_FADE_SPEED;
	} else {
		alpha -= OUT_FADE_SPEED;
	}
	select->sprite.col.a = ( u_char )alpha;
	// non_select
	alpha = ( int )non_select->sprite.col.a;
	if ( alpha < SK_A ){
		alpha += IN_FADE_SPEED;
	} else {
		alpha -= OUT_FADE_SPEED;
	}
	non_select->sprite.col.a = ( u_char )alpha;
	OverWriteDotUpdate( pWork );
#endif
}

static int SK_MCManGetResult( Work *pWork ) // 最低でも30frame
{
	switch ( MCManGetResult() ){
	case 1 : // 成功
		if ( pWork->end_time >= DIRECT_TICK( 30 ) ){
			pWork->end_time = 0;
			return( 1 );
		}
		break;
	case -1 : // 失敗
	  return( -1 );
	}
	pWork->end_time++;

	return ( 0 );
}

#define END_TIME (DIRECT_TICK(60))
static void OverWriteEnd( Work *pWork )
{
	// over_write 操作
	if ( pWork->flag & SK_OVER_WRITE_MODE ){
		if ( ( pWork->flag & SK_SAVE_WAIT ) ){
			if ( !( pWork->flag & SK_SAVE_START ) ){
				OverWrite( pWork );
			} else {
				switch ( SK_MCManGetResult( pWork ) ){
				case 1 : // save wait
					pWork->jpn_font.position = 7;
					StringCleate( pWork );
					pWork->end_time = END_TIME;
					pWork->flag |= SK_SAVE_TIME;
					pWork->flag &= ~SK_SAVE_WAIT;
					break;
				case -1 : // faild
					pWork->jpn_font.position = 8;
					StringCleate( pWork );
					pWork->end_time = END_TIME;
					pWork->flag |= SK_SAVE_TIME;
					pWork->flag &= ~SK_SAVE_WAIT;
					break;
				} 
			}
		}
	}
}

static inline void SoundRgb( int tmp , int tmp2 ) // 音をならすかならさないか
{
	if ( tmp != tmp2 ){
		SE_RGB_TUNE();
	}
}

#define STR_RGB_MASK1 (7842518)
#define STR_RGB_MASK2 (7842519)
#define STR_RGB_MASK3 (7842520)
static inline void RgbMask( Work *pWork , int mode )
{
	SPR_OBJ *rgb[ 3 ];
	int     strcode;
	int     i;

	strcode = STR_RGB_MASK1;
	for ( i = 0 ; i < 3 ; i ++ , strcode++ ){
		rgb[ i ] = L2D_GetObject( pWork->handle_2d , strcode );
		if ( rgb[ i ] == NULL ){
			SK_Err("rgb new\0");
			return;
		}
		if ( mode == 0 ){
			SPR_SHOW( rgb[ i ] );
		} else {
			SPR_HIDE( rgb[ i ] );
		}
	}
	if ( mode == 1 ){
		return;
	}
	switch ( pWork->color_mode_position ){
	case 0 : // r
		SPR_HIDE( rgb[ 0 ] );
		break;
	case 1 : // g
		SPR_HIDE( rgb[ 1 ] );
		break;
	case 2 : // b
		SPR_HIDE( rgb[ 2 ] );
		break;
	}
}

static void ColorChange( Work *pWork )
{
	int status;
	int tmp;
	char flag;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( !( pWork->flag & SK_RGB_FADE_IN_OK ) || ( pWork->flag & SK_OVER_WRITE_MODE ) || ( status == L2D_STAT_BUSY ) ){
		return;
	}
	flag = 0;
	if ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status & PAD_L ){ // 減少
		switch ( pWork->color_mode_position ) {
		case SET_COLOR_R :
			tmp = ( int )pWork->photo_list->r;
			tmp--;
			if ( tmp < 0 ){
				pWork->photo_list->r = 0;
			} else {
				SoundRgb( pWork->photo_list->r , tmp );
				pWork->photo_list->r = ( u_char )tmp;
			}
			break;
		case SET_COLOR_G :
			tmp = ( int )pWork->photo_list->g;
			tmp--;
			if ( tmp < 0 ){
				pWork->photo_list->g = 0;
			} else {
				SoundRgb( pWork->photo_list->g , tmp );
				pWork->photo_list->g = ( u_char )tmp;
			}
			break;
		case SET_COLOR_B :
			tmp = ( int )pWork->photo_list->b;
			tmp--;
			if ( tmp < 0 ){
				pWork->photo_list->b = 0;
			} else {
				SoundRgb( pWork->photo_list->b , tmp );
				pWork->photo_list->b = ( u_char )tmp;
			}
			break;
		}
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status & PAD_R ) && !( flag & 0x1 ) ){ // 減少
		switch ( pWork->color_mode_position ) {
		case SET_COLOR_R :
			tmp = ( int )pWork->photo_list->r;
			tmp++;
			if ( tmp > 255 ){
				pWork->photo_list->r = 255;
			} else {
				SoundRgb( pWork->photo_list->r , tmp );
				pWork->photo_list->r = ( u_char )tmp;
			}
			break;
		case SET_COLOR_G :
			tmp = ( int )pWork->photo_list->g;
			tmp++;
			if ( tmp > 255 ){
				pWork->photo_list->g = 255;
			} else {
				SoundRgb( pWork->photo_list->g , tmp );
				pWork->photo_list->g = ( u_char )tmp;
			}
			break;
		case SET_COLOR_B :
			tmp = ( int )pWork->photo_list->b;
			tmp++;
			if ( tmp > 255 ){
				pWork->photo_list->b = 255;
			} else {
				SoundRgb( pWork->photo_list->b , tmp );
				pWork->photo_list->b = ( u_char )tmp;
			}
			break;
		}
		flag = 0x1;
	}
	RgbMask( pWork , 0 );
	// move
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status & PAD_U ) && !( flag & 0x1 ) ){
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->color_mode_position > 0 ){
			pWork->color_mode_position--; // <-- リストを進める数の事 いるのか？
			SE_SEL();
		}
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status & PAD_D ) && !( flag & 0x1 ) ){
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->color_mode_position < 2 ){
			pWork->color_mode_position++; // <-- リストを進める数の事 いるのか？
			SE_SEL();
		}
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_OK/* 元PAD_A */ ) && !( flag & 0x1 ) ){ // ok exit
		pWork->action = STR_OPENOVERWRITE;
		pWork->flag |= SK_OVER_WRITE_MODE;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		SE_OK();
		flag = 0x1;
	}

	if ( ( GV_PadDataDirect[ 0 ].press & PAD_CANCEL/* 元PAD_B */ ) && !( flag & 0x1 ) ){ // no exit
		pWork->flag |= SK_RGB_FADE_OUT_START;
		pWork->flag &= ~SK_RGB_FADE_IN_START;
		pWork->flag &= ~SK_RGB_FADE_IN_OK;
		pWork->flag &= ~SK_RGB_FADE_OUT_OK;
		// 色を変更前にもどす
		pWork->photo_list->r = pWork->rgb_menu.bak_r;
		pWork->photo_list->g = pWork->rgb_menu.bak_g;
		pWork->photo_list->b = pWork->rgb_menu.bak_b;

		// value で menu 非表示
		pWork->hokan[ 0 ].p = 0.0f;
		pWork->hokan[ 0 ].count = HOKAN_SPEED;
		pWork->hokan[ 0 ].code1 = STR_HIDE_RGB_MARUBATU;
		pWork->hokan[ 0 ].code2 = STR_SHOW_RGB_MARUBATU;
		pWork->hokan[ 0 ].morf_parts = STR_ALBAM_MARUBATSU_RGB;
		pWork->hokan[ 0 ].flag = 0;

		pWork->hokan[ 1 ].p = 0.0f;
		pWork->hokan[ 1 ].count = HOKAN_SPEED;
		pWork->hokan[ 1 ].code1 = STR_HIDE_RGB_FILTER;
		pWork->hokan[ 1 ].code2 = STR_SHOW_RGB_FILTER;
		pWork->hokan[ 1 ].morf_parts = STR_ALBUM_MARUBATSU_RECT;
		pWork->hokan[ 1 ].flag = 0;
		SE_CANCEL();
		flag = 0x1;
	}
	// auto mode
	if ( pWork->hold_time >= DIRECT_TICK( 30 ) ){
		pWork->pad_status = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time -= DIRECT_TICK( 4 );
	} else if ( pWork->pad_check & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time++;
	} else {
		pWork->pad_check = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_U | PAD_D | PAD_L | PAD_R ) ) ){
		pWork->hold_time = 0;
		pWork->pad_status = 0;
	}
	// hold time
	if ( !( GV_PadDataDirect[ 0 ].press & ( PAD_U | PAD_D | PAD_L | PAD_R | SELECT | CANCEL ) ) ){
		pWork->time++;
		pWork->time %= ( JPEG_DECODE_START_TIME + 1 );
	}
}

static void RGB_Menu( Work *pWork )
{
	char ascci[ 3+1 ];
	char       flag;
	int         tmp;
	int           i;

	// 色変換
	if ( !( pWork->flag & SK_RGB_FADE_IN_START || pWork->flag & SK_OVER_WRITE_MODE || pWork->action == STR_CLOSEOVERWRITE ) ){
		ColorChange( pWork );
	}
	// 情報を取得
	flag = 0;
	tmp = 0;
	sprintf( ascci , "%03d" , pWork->photo_list->r );
	for ( i = 0 ; i < 3 ; i ++ ){
		if ( ascci[ i ] == '0' && flag == 0 && i != 2 ){
			ascci[ i ] = 0x20;
			tmp++;
		} else {
			flag = 1;
		}
	}
	for ( i = 0 ; i < tmp ; i ++ ){
		ascci[ 0 ] = ascci[ 1 ];
		ascci[ 1 ] = ascci[ 2 ];
		ascci[ 2 ] = 0x20;
	}
	SK_PrintfNormal( ascci , pWork->rgb_menu.rgb_num[ 0 ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , 3 );

	flag = 0;
	tmp = 0;
	sprintf( ascci , "%03d" , pWork->photo_list->g );
	for ( i = 0 ; i < 3 ; i ++ ){
		if ( ascci[ i ] == '0' && flag == 0 && i != 2 ){
			ascci[ i ] = 0x20;
			tmp++;
		} else {
			flag = 1;
		}
	}
	for ( i = 0 ; i < tmp ; i ++ ){
		ascci[ 0 ] = ascci[ 1 ];
		ascci[ 1 ] = ascci[ 2 ];
		ascci[ 2 ] = 0x20;
	}
	SK_PrintfNormal( ascci , pWork->rgb_menu.rgb_num[ 1 ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , 3 );
	flag = 0;
	tmp = 0;
	sprintf( ascci , "%03d" , pWork->photo_list->b );
	for ( i = 0 ; i < 3 ; i ++ ){
		if ( ascci[ i ] == '0' && flag == 0 && i != 2 ){
			ascci[ i ] = 0x20;
			tmp++;
		} else {
			flag = 1;
		}
	}
	for ( i = 0 ; i < tmp ; i ++ ){
		ascci[ 0 ] = ascci[ 1 ];
		ascci[ 1 ] = ascci[ 2 ];
		ascci[ 2 ] = 0x20;
	}
	SK_PrintfNormal( ascci , pWork->rgb_menu.rgb_num[ 2 ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , 3 );
}

static void FullScreen( Work *pWork )
{
	float ftmp;
	int tmp , i;

	ftmp = ( 384.0f / 448.0f );
	if ( pWork->flag & SK_FULL_SCREEN_START )
   { // full screen mode に 移項
		if ( pWork->flag & SK_PHOTO_FADE_OUT_OK && pWork->flag & SK_SIGNAL_OK )
      { 
         if ( BP_Area_EU() )
         {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
			   pWork->photo.x = 0.0f;
			   pWork->photo.y = 0.0f;
			   pWork->photo.w = DRAW_WIDTH;
			   pWork->photo.h = 384.0f;
         }
         else
         {
//#else
			   pWork->photo.x = -42.0f;
			   pWork->photo.y = 0.0f;
			   pWork->photo.w = 596.0f;
			   pWork->photo.h = DRAW_HEIGHT * ftmp;
         }
//#endif
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag |= SK_PHOTO_FADE_IN_START;
		}
		if ( ( pWork->flag & SK_PHOTO_FADE_IN_OK ) )
      {
			pWork->flag |= SK_FULL_SCREEN_OK;
			pWork->flag &= ~SK_FULL_SCREEN_START;
			pWork->flag &= ~SK_PRE_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_SIGNAL_OK;
		}
	}
	if ( pWork->flag & SK_PRE_NORMAL_SCREEN_START )
   { // full screen になる手前の状態に戻す
		if ( pWork->flag & SK_PHOTO_FADE_OUT_OK && pWork->flag & SK_FONT_FADE_OUT_OK && pWork->flag & SK_PAGE_FADE_OUT_OK &&
			 pWork->flag & SK_SIGNAL_OK )
      { 
			// 前処理
			tmp = pWork->position_sign < 0 ? -pWork->position_sign : pWork->position_sign;
			if ( tmp != 0 )
         { // 移動
				for ( i = 0 ; i < tmp ; i ++ )
            {
					if ( pWork->position_sign < 0 )
               {
						pWork->photo_list = PhotoListBack( pWork->photo_list );
					} 
               else 
               {
						pWork->photo_list = PhotoListForward( pWork->photo_list );
					}
				}
				pWork->position_sign = 0;
			}
			if ( pWork->flag & SK_PHOTO_LOAD_OK )
         { // ロードが終るまでまつ
            if ( BP_Area_EU() )
            {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
				   pWork->photo.x = 19.0f;
				   pWork->photo.y = 14.0f;//29.0f;
				   pWork->photo.w = 472.0f;
				   pWork->photo.h = 413 * ftmp;//378.0f * ftmp; // 332
            } 
            else
            {
//#else
				   pWork->photo.x = 19.0f;
				   pWork->photo.y = 29.0f;
				   pWork->photo.w = 472.0f;
				   pWork->photo.h = 354.0f * ftmp;
            }
//#endif
				pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
				pWork->flag &= ~SK_FONT_FADE_OUT_OK;
				pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
				pWork->flag |= SK_PHOTO_FADE_IN_START;
			}
		}
		if ( ( pWork->flag & SK_PHOTO_FADE_IN_OK ) )
      {
			pWork->flag &= ~SK_PRE_NORMAL_SCREEN_START;
			pWork->flag &= ~SK_FULL_SCREEN_OK;
			pWork->flag &= ~SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_SIGNAL_OK;
		}
	}
	if ( pWork->flag & SK_NORMAL_SCREEN_START )
   {
		SPR_HIDE( pWork->dot.dot_curs );
		if ( pWork->flag & SK_PHOTO_FADE_OUT_OK && pWork->flag & SK_SIGNAL_OK )
      { 
			pWork->photo.x = 281.0f;
			pWork->photo.y = 101.5f;
			pWork->photo.w = 198.0f-2;
         if ( BP_Area_EU() )
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
			   pWork->photo.h = 150.0f * ftmp;
         else
//#else
			   pWork->photo.h = 144.0f * ftmp;
//#endif
         pWork->photo.h -=2;

			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag |= SK_PHOTO_FADE_IN_START;
		}
		if ( ( pWork->flag & SK_PHOTO_FADE_IN_OK ) )
      {
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag |= SK_PAGE_FADE_IN_START;
			pWork->flag |= SK_DOT_FADE_IN_START;
			pWork->flag |= SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_NORMAL_SCREEN_START;
			pWork->flag &= ~SK_PRE_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_FULL_SCREEN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_SIGNAL_OK;
		}
	}
}

static void PhotoFade( Work *pWork ) // fade out = black // rgb は 未對應？
{
	float a;
	int tmp;
	int   i;

	if ( pWork->flag & SK_PHOTO_FADE_IN_START ){
		a = ( float )pWork->photo.a;
		a += IN_FADE_SPEED;
		if ( a > 128.0f ){
			pWork->photo.a = 128;
		} else {
			pWork->photo.a = ( u_char )a;
		}
		if ( pWork->photo.a >= 128 ){
			pWork->flag |= SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
		}
	}
	if ( pWork->flag & SK_PHOTO_FADE_OUT_START ){
		a = ( float )pWork->photo.a;
		a -= OUT_FADE_SPEED;
		if ( a < 0.0f ){
			pWork->photo.a = 0;
		} else {
			pWork->photo.a = ( u_char )a;
		}
		if ( pWork->photo.a <= 0 ){
			tmp = pWork->position_sign < 0 ? -pWork->position_sign : pWork->position_sign;
			if ( tmp != 0 ){ // 移動
				for ( i = 0 ; i < tmp ; i ++ ){
					if ( pWork->position_sign < 0 ){
						pWork->photo_list = PhotoListBack( pWork->photo_list );
					} else {
						pWork->photo_list = PhotoListForward( pWork->photo_list );
					}
				}
				pWork->position_sign = 0;
			}
			pWork->flag |= SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_START;
		}
	}
}

static void PageFade( Work *pWork )
{
	int alpha;

	if ( pWork->flag & SK_PAGE_FADE_IN_START ){ 
		alpha = ( int )pWork->page.a;
		alpha += IN_FADE_SPEED;
		if ( alpha > 128 ){
			pWork->page.a = 128;
		} else {
			pWork->page.a = ( u_char )alpha;
		}
		SK_PrintfChengColor2( pWork->page.page_now , SK_R , SK_G , SK_B , pWork->page.a , PAGE_FONT_MAX );
		SK_PrintfChengColor2( pWork->page.page , SK_R , SK_G , SK_B , pWork->page.a , PAGE_MAX );
		if ( pWork->page.a >= 128 ){
			pWork->flag |= SK_PAGE_FADE_IN_OK;
			pWork->flag &= ~SK_PAGE_FADE_IN_START;
		}
	}
	if ( pWork->flag & SK_PAGE_FADE_OUT_START ){
		alpha = ( int )pWork->page.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < 0 ){
			pWork->page.a = 0;
		} else {
		  pWork->page.a = ( u_char )alpha;
		}
		SK_PrintfChengColor2( pWork->page.page_now , SK_R , SK_G , SK_B , pWork->page.a , PAGE_FONT_MAX );
		SK_PrintfChengColor2( pWork->page.page , SK_R , SK_G , SK_B , pWork->page.a , PAGE_MAX );
		if ( pWork->page.a <= 0 ){
			pWork->flag |= SK_PAGE_FADE_OUT_OK;
			pWork->flag &= ~SK_PAGE_FADE_OUT_START;
		}
	}
}

static void FontFade( Work *pWork )
{
	Photo_List *photo_list;
	int alpha;
	int   tmp;
	int     i;

	tmp = 0;
	photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( photo_list->pNext != NULL ){
		photo_list = photo_list->pNext;
	}
	if ( pWork->flag & SK_PAGE_CHANGE_L ){
		photo_list = PhotoListSearch( photo_list , ( pWork->page.num ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	} else if ( pWork->flag & SK_PAGE_CHANGE_R ){
		photo_list = PhotoListSearch( photo_list , ( pWork->page.num - 2 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	} else {
		photo_list = PhotoListSearch( photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	}
	if ( photo_list == NULL ){
		return;
	}
	if ( pWork->flag & SK_FONT_FADE_OUT_START ){
		for ( i = 0 ; i < pWork->page.now_page_max ; i ++ ){
			alpha = ( int )photo_list->a;
			alpha -= OUT_FADE_SPEED;
			if ( alpha < 0 ){
				photo_list->a = 0;
				tmp++;
			} else {
				photo_list->a = ( u_char )alpha;
			}
			SK_PrintfChengColor2( pWork->font[ i ] , SK_R , SK_G , SK_B , photo_list->a , ASCCI_MAX );
			// next
			if ( photo_list->pNext == NULL ){
			  	i++;
				break;
			}
			photo_list = PhotoListForward( photo_list );
		}
		// jude
		if ( tmp >= i ){
			pWork->flag |= SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_START;
		}
	}
}

static void DotFade( Work *pWork ) // update
{
	int alpha ;

	if ( pWork->flag & SK_DOT_FADE_OUT_START ){
		alpha = ( int )pWork->dot.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < 0 ){
			pWork->dot.a = 0;
			// 終了
			pWork->flag |= SK_DOT_FADE_OUT_OK;
			pWork->flag &= ~SK_DOT_FADE_OUT_START;
		} else {
			pWork->dot.a = ( u_char )alpha;
		}
		SK_PrintfChengColor( pWork->dot.dot_curs->empty.head.child , pWork->dot.r , pWork->dot.g , pWork->dot.b , pWork->dot.a );
	}
	if ( pWork->flag & SK_DOT_FADE_IN_START ){
		alpha = ( int )pWork->dot.a;
		alpha += IN_FADE_SPEED;
		if ( alpha > 128 ){
			pWork->dot.a = 128;
			// 終了
			pWork->flag |= SK_DOT_FADE_IN_OK;
			pWork->flag &= ~SK_DOT_FADE_IN_START;
		} else {
			pWork->dot.a = ( u_char )alpha;
		}
		SK_PrintfChengColor( pWork->dot.dot_curs->empty.head.child , pWork->dot.r , pWork->dot.g , pWork->dot.b , pWork->dot.a );
	}
}

#define SK_R_POSITION (275) // 移動量20
#define SK_G_POSITION (295)
#define SK_B_POSITION (315)

static void RGB_SelectUpdate( Work *pWork )
{
	switch ( pWork->color_mode_position ) {
	case SET_COLOR_R :
		pWork->rgb_menu.rgb_select->line.pos[ 0 ].y = SK_R_POSITION;
		break;
	case SET_COLOR_G :
		pWork->rgb_menu.rgb_select->line.pos[ 0 ].y = SK_G_POSITION;
		break;
	case SET_COLOR_B :
		pWork->rgb_menu.rgb_select->line.pos[ 0 ].y = SK_B_POSITION;
		break;
	}
	pWork->rgb_menu.rgb_select->line.pos[ 1 ].y = pWork->rgb_menu.rgb_select->line.pos[ 0 ].y;
}

#define BAR_MAX (128.0f)
static void RGB_BarMove( Work *pWork ) // バーの移動（慣性補間）
{
	Photo_List *photo_list;
	SPR_OBJ *spr;
	float   tmp;

	tmp = 0;
	photo_list = pWork->photo_list;
	if ( photo_list == NULL ){
		return;
	}
	// 加工
	// r
	spr = pWork->rgb_menu.rgb_bar[ 0 ]->head.child; // edgeの取り出し
	tmp = ( BAR_MAX / 256.0f );
	pWork->rgb_menu.rgb_bar[ 0 ]->line.pos[ 1 ].x = pWork->rgb_menu.rgb_bar[ 0 ]->line.pos[ 0 ].x + ( float )( photo_list->r * tmp );
	spr->line.pos[ 0 ].x = pWork->rgb_menu.rgb_bar[ 0 ]->line.pos[ 1 ].x - 95.0f;
	spr->line.pos[ 1 ].x = spr->line.pos[ 0 ].x;
   spr->line.pos[ 0 ].x+=1;
   spr->line.pos[ 1 ].x+=1;
	// g
	spr = pWork->rgb_menu.rgb_bar[ 1 ]->head.child; // edgeの取り出し
	tmp = ( BAR_MAX / 256.0f );
	pWork->rgb_menu.rgb_bar[ 1 ]->line.pos[ 1 ].x = pWork->rgb_menu.rgb_bar[ 1 ]->line.pos[ 0 ].x + ( float )( photo_list->g * tmp );
	spr->line.pos[ 0 ].x = pWork->rgb_menu.rgb_bar[ 1 ]->line.pos[ 1 ].x - 95.0f;
	spr->line.pos[ 1 ].x = spr->line.pos[ 0 ].x;
   spr->line.pos[ 0 ].x+=1;
   spr->line.pos[ 1 ].x+=1;
	// b
	spr = pWork->rgb_menu.rgb_bar[ 2 ]->head.child; // edgeの取り出し
	tmp = ( BAR_MAX / 256.0f );
	pWork->rgb_menu.rgb_bar[ 2 ]->line.pos[ 1 ].x = pWork->rgb_menu.rgb_bar[ 2 ]->line.pos[ 0 ].x + ( float )( photo_list->b * tmp );
	spr->line.pos[ 0 ].x = pWork->rgb_menu.rgb_bar[ 2 ]->line.pos[ 1 ].x - 95.0f;
	spr->line.pos[ 1 ].x = spr->line.pos[ 0 ].x;
   spr->line.pos[ 0 ].x+=1;
   spr->line.pos[ 1 ].x+=1;
#if 0
	// g
	spr = pWork->rgb_menu.rgb_bar[ 1 ]->head.child; // edgeの取り出し
	tmp = pWork->rgb_menu.rgb_bar[ 1 ]->line.pos[ 1 ].x - pWork->rgb_menu.rgb_bar[ 1 ]->line.pos[ 0 ].x;
	pWork->rgb_menu.rgb_bar[ 1 ]->line.pos[ 1 ].x += ( ( photo_list->g / 2.0f ) - tmp ) / 10.0f;
	spr->line.pos[ 0 ].x = pWork->rgb_menu.rgb_bar[ 1 ]->line.pos[ 1 ].x - 93.0f;
	spr->line.pos[ 1 ].x = spr->line.pos[ 0 ].x;
	// b
	spr = pWork->rgb_menu.rgb_bar[ 2 ]->head.child; // edgeの取り出し
	tmp = pWork->rgb_menu.rgb_bar[ 2 ]->line.pos[ 1 ].x - pWork->rgb_menu.rgb_bar[ 2 ]->line.pos[ 0 ].x;
	pWork->rgb_menu.rgb_bar[ 2 ]->line.pos[ 1 ].x += ( ( photo_list->b / 2.0f ) - tmp ) / 4.0f;
	spr->line.pos[ 0 ].x = pWork->rgb_menu.rgb_bar[ 2 ]->line.pos[ 1 ].x - 93.0f;
	spr->line.pos[ 1 ].x = spr->line.pos[ 0 ].x;
#endif
}

static void RGBFade( Work *pWork ) // update
{
	int alpha ;

	if ( pWork->flag & SK_RGB_FADE_OUT_START ){
		alpha = ( int )pWork->rgb_menu.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < 0 ){
			pWork->rgb_menu.a = 0;
			// 終了
			pWork->action = STR_CLOSERGB;
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_RGB_FADE_OUT_OK;
			pWork->flag &= ~SK_RGB_FADE_OUT_START;
			pWork->flag &= ~SK_ACTION_OK;
		} else {
			pWork->rgb_menu.a = ( u_char )alpha;
		}
	}
	if ( pWork->flag & SK_RGB_FADE_IN_START ){
		alpha = ( int )pWork->rgb_menu.a;
		alpha += IN_FADE_SPEED;
		if ( alpha > 128 ){
			pWork->rgb_menu.a = 128;
			// 終了
			pWork->flag |= SK_RGB_FADE_IN_OK;
			pWork->flag &= ~SK_RGB_FADE_IN_START;
		} else    {
			pWork->rgb_menu.a = ( u_char )alpha;
		}
	}
}

static inline void DotUpdate( Work *pWork ) // update
{
	if ( !( pWork->flag & SK_NORMAL ) ){
		return;
	}
	// dot position update
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) ){ 
		pWork->dot.dot_curs->empty.pos.y += ( pWork->dot_hokan.end_y - pWork->dot.dot_curs->empty.pos.y ) / pWork->dot_hokan.count;
		pWork->dot_hokan.count--;
		if ( pWork->dot_hokan.count <= 0 ){
			pWork->dot_hokan.count = 1;
		}
		if ( pWork->dot.dot_curs->empty.pos.y == pWork->dot_hokan.end_y ){
			pWork->dot_hokan.flag |= SK_MOVE_OK;
		}
	}
}

static inline void RGBUpdate( Work *pWork )
{
	int  i ;

	for ( i = 0 ; i < 3 ; i ++ ){
//		SK_PrintfChengColor( pWork->rgb_menu.rgb[ i ] , pWork->rgb_menu.r , pWork->rgb_menu.g , pWork->rgb_menu.b , pWork->rgb_menu.a );
		SK_PrintfChengColor2( pWork->rgb_menu.rgb_num[ i ] , pWork->rgb_menu.r , pWork->rgb_menu.g , pWork->rgb_menu.b , pWork->rgb_menu.a , 3 );
	}
}

static void PrePositionUpdate( Work *pWork ) // 反転アクションの呼び出し
{
   // BP JG - change the colour of the hilight so the text is visible.
   SPR_OBJ *spr;
	spr = L2D_GetObject( pWork->handle_2d, 0x07f3b88 );
   spr->sprite.col.a = 0x40;
   spr = L2D_GetObject( pWork->handle_2d, 0x04260c8 );
   spr->sprite.col.a = 0x10;
   spr = L2D_GetObject( pWork->handle_2d, 0x065c762 );
   spr->sprite.col.a = 0x10;

	if ( pWork->flag & SK_ACTION_START ){
		return;
	}
	switch ( pWork->pre_position )
   {
	case 0 : // name entry
		pWork->action = STR_SELECTTITLE;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		break;
	case 1 : // color
		pWork->action = STR_SELECTCOLOR;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		break;
	case 2 : // exit
		pWork->action = STR_SELECTEXIT;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		break;
	}
}

static void PageChange( Work *pWork ) // ページの切替
{
	pWork->flag |= SK_FONT_FADE_IN_START;
	pWork->flag &= ~SK_FONT_FADE_OUT_OK;
	pWork->flag &= ~SK_FONT_FADE_OUT_START;
	pWork->flag &= ~SK_FIRST_FADE_OK;
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( pWork->photo_list->pNext != NULL ){
		pWork->photo_list = pWork->photo_list->pNext;
	}
	if ( pWork->flag & SK_LIST_UP ){
		pWork->position = DISP_PHOTO_MAX - 1; // 初期位置へ
		pWork->flag &= ~SK_LIST_UP;
		pWork->photo_list = PhotoListSearch( pWork->photo_list , ( ( pWork->page.num ) * DISP_PHOTO_MAX ) - 1 ); // pWork->position の 値まで移動
	} else {
		pWork->position = 0; // 初期位置へ
		if ( pWork->flag & SK_PAGE_CHANGE_L ){
			pWork->photo_list = PhotoListSearch( pWork->photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
		} else if ( pWork->flag & SK_PAGE_CHANGE_R ){
			pWork->photo_list = PhotoListSearch( pWork->photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
		}
	}
	if ( pWork->photo_list == NULL ){
		return;
	}
	pWork->flag &= ~SK_PAGE_CHANGE_L;
	pWork->flag &= ~SK_PAGE_CHANGE_R;
}

#define STR_SLOT1 (6802104)
#define STR_SLOT2 (6802105)
static void MemoryCardFade( Work *pWork ) // メモりーカードのalpha色がえ
{
	SPR_OBJ *spr , *spr2;

   spr = L2D_GetObject( pWork->handle_2d , STR_SLOT1 );
   if (spr)  spr->sprite.pos.x = 57;       // make sure this is lined up perfectly with the title and list of photos to be loaded.
   spr = L2D_GetObject( pWork->handle_2d , STR_SLOT2 );
   if (spr)  spr->sprite.pos.x = 57;       // make sure this is lined up perfectly with the title and list of photos to be loaded.



	if ( !( pWork->flag & SK_MEMORY_FONT_FADE ) ){
		return;
	}
	spr = L2D_GetObject( pWork->handle_2d , STR_SLOT1 );
	if ( spr == NULL ){
		return;
	}
	spr2 = L2D_GetObject( pWork->handle_2d , STR_SLOT2 );
	if ( spr2 == NULL ){
		return;
	}
	pWork->memory.alpha -= pWork->memory.alpha_add;
//	printf("alpha = %f %f\n" , pWork->memory.alpha , pWork->memory.alpha_add );
	if ( pWork->memory.alpha <= 0 ){
		spr->sprite.col.a = ( u_char )0;
		spr2->sprite.col.a = ( u_char )0;
	} else {
		spr->sprite.col.a = ( u_char )pWork->memory.alpha;
		spr2->sprite.col.a = ( u_char )pWork->memory.alpha;
	}
	if ( spr->sprite.col.a == 0 ){
		pWork->flag &= ~SK_MEMORY_FONT_FADE;
	}
}

static inline void ForceMorf( int handle , int strcode , int code1 , int code2 )
{
	void *parts;

	parts = L2D_GetParts( handle , strcode );    /* パーツポインタの取得 */

	if ( parts == NULL ){
		SK_Err("morf failed\0");
		return;
	}
	L2D_MorfObject( parts , code1 , code2 , 1.0f );
}

static void PadControlFullScreen( Work *pWork )
{
	if ( pWork->flag & SK_FULL_SCREEN_OK ){ // full screen mode -> pre normal mode
		if ( GV_PadDataDirect[ 0 ].press ){
			pWork->action = STR_FULL_SCREEN_TO_PRE_SCREEN_OK;
			pWork->pre_position = 0;
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_FADE_OUT_START;
			pWork->flag |= SK_PRE_NORMAL_SCREEN_START;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_FULL_SCREEN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_ACTION_OK;
		}
		return;
	}
}

#define PRE_POSITION_MAX (2)
static void PadControlPreNormal( Work *pWork )
{
	if ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status & PAD_L ){
		if ( pWork->pre_position > 0 ){
			pWork->pre_position--;
			SE_SEL();
		}
	} else if ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status & PAD_R ){
		if ( pWork->pre_position < PRE_POSITION_MAX ){
			pWork->pre_position++;
			SE_SEL();
		} 
	} else if ( ( GV_PadDataDirect[ 0 ].press & PAD_OK/* 元PAD_A */ ) ){
		switch ( pWork->pre_position ){
		case 0 : // name_change
			GV_SetActorChild( pWork , NewAlbamName( pWork->photo_list->name , &pWork->flag , pWork->port , pWork->photo_list ,
													pWork->handle_2d , pWork->jpn_font.work , pWork->album_resource ) );
			pWork->flag |= SK_NAME_ENTRY;
			pWork->flag &= ~SK_RGB_CHENG;
			// 初期化
			ForceMorf( pWork->handle_2d , STR_ALBAM_MARUBATSU , STR_HIDE_ALB_MARUBATU , STR_HIDE_ALB_MARUBATU );
			ForceMorf( pWork->handle_2d , STR_ALBUM_MARUBATSU_RECT , STR_HIDE_ALB_FILTER , STR_HIDE_ALB_FILTER );
			// value で menu 非表示
			pWork->hokan[ 0 ].p = 0.0f;
			pWork->hokan[ 0 ].count = HOKAN_SPEED;
			pWork->hokan[ 0 ].code1 = STR_SHOW_ALB_MARUBATU;
			pWork->hokan[ 0 ].code2 = STR_HIDE_ALB_MARUBATU;
			pWork->hokan[ 0 ].morf_parts = STR_ALBAM_MARUBATSU;
			pWork->hokan[ 0 ].flag = 0;

			pWork->hokan[ 1 ].p = 0.0f;
			pWork->hokan[ 1 ].count = HOKAN_SPEED;
			pWork->hokan[ 1 ].code1 = STR_SHOW_ALB_FILTER;
			pWork->hokan[ 1 ].code2 = STR_HIDE_ALB_FILTER;
			pWork->hokan[ 1 ].morf_parts = STR_ALBUM_MARUBATSU_RECT;
			pWork->hokan[ 1 ].flag = 0;
			SE_OK();
			break;
		case 1 : // rgb change
			pWork->action = STR_OPENRGB;
			pWork->over_write.position = 1;
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_RGB_CHENG;
			pWork->flag &= ~SK_RGB_FADE_IN_OK;
			pWork->flag &= ~SK_RGB_FADE_OUT_OK;
			pWork->flag &= ~SK_RGB_FADE_OUT_START;
			pWork->flag &= ~SK_NAME_ENTRY;
			pWork->flag &= ~SK_ACTION_OK;
			// 初期値保存
			pWork->rgb_menu.bak_r = pWork->photo_list->r;
			pWork->rgb_menu.bak_g = pWork->photo_list->g;
			pWork->rgb_menu.bak_b = pWork->photo_list->b;
			// 初期化
			ForceMorf( pWork->handle_2d , STR_ALBAM_MARUBATSU_RGB , STR_HIDE_RGB_MARUBATU , STR_HIDE_RGB_MARUBATU );
			ForceMorf( pWork->handle_2d , STR_ALBUM_MARUBATSU_RECT , STR_HIDE_RGB_FILTER , STR_HIDE_RGB_FILTER );
			// value で menu 非表示
			pWork->hokan[ 0 ].p = 0.0f;
			pWork->hokan[ 0 ].count = HOKAN_SPEED;
			pWork->hokan[ 0 ].code1 = STR_SHOW_RGB_MARUBATU;
			pWork->hokan[ 0 ].code2 = STR_HIDE_RGB_MARUBATU;
			pWork->hokan[ 0 ].morf_parts = STR_ALBAM_MARUBATSU_RGB;
			pWork->hokan[ 0 ].flag = 0;

			pWork->hokan[ 1 ].p = 0.0f;
			pWork->hokan[ 1 ].count = HOKAN_SPEED;
			pWork->hokan[ 1 ].code1 = STR_SHOW_RGB_FILTER;
			pWork->hokan[ 1 ].code2 = STR_HIDE_RGB_FILTER;
			pWork->hokan[ 1 ].morf_parts = STR_ALBUM_MARUBATSU_RECT;
			pWork->hokan[ 1 ].flag = 0;
			SE_OK();
			break;
		case 2 : // exit
			switch( pWork->port ){
			case 0 : // Memory Card Slot 1
				pWork->action = STR_CLOSEEDITSLOT1;
				break;
			case 1 : // Memory Card Slot 2
				pWork->action = STR_CLOSEEDITSLOT2;
				break;
			case 2 : // hard disk
				break;		
			}
			SK_AllHide( pWork->edit_font , ASCCI_MAX ); // 一行いっきに書き換え
			pWork->flag |= SK_NORMAL_SCREEN_START;
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_START;
			pWork->flag &= ~SK_PAGE_FADE_OUT_START;
			pWork->flag &= ~SK_ACTION_OK;
			SE_CANCEL();
			break;
		}
	} else if ( ( GV_PadDataDirect[ 0 ].press & PAD_R1 ) ){ // full_screen
		pWork->action = STR_PRE_SCREEN_TO_FULL_SCREEN_OK;
		SK_AllHide( pWork->edit_font , ASCCI_MAX ); // 一行いっきに書き換え
		pWork->flag |= SK_ACTION_START;
		pWork->flag |= SK_FULL_SCREEN_START;
		pWork->flag |= SK_PHOTO_FADE_OUT_START;
		pWork->flag &= ~SK_PHOTO_FADE_IN_START;
		pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
		pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
		pWork->flag &= ~SK_ACTION_OK;
	} else if ( ( GV_PadDataDirect[ 0 ].press & CANCEL ) ){
		switch( pWork->port ){
		case 0 : // Memory Card Slot 1
			pWork->action = STR_CLOSEEDITSLOT1;
			break;
		case 1 : // Memory Card Slot 2
			pWork->action = STR_CLOSEEDITSLOT2;
			break;
		case 2 : // hard disk
			break;		
		}
		SK_AllHide( pWork->edit_font , ASCCI_MAX ); // 一行いっきに書き換え
		pWork->flag |= SK_NORMAL_SCREEN_START;
		pWork->flag |= SK_PHOTO_FADE_OUT_START;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_PHOTO_FADE_IN_START;
		pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
		pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
		pWork->flag &= ~SK_FONT_FADE_OUT_OK;
		pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
		pWork->flag &= ~SK_FONT_FADE_OUT_START;
		pWork->flag &= ~SK_PAGE_FADE_OUT_START;
		pWork->flag &= ~SK_ACTION_OK;
		SE_CANCEL();
		return;
	}
	// auto mode
	if ( pWork->hold_time >= DIRECT_TICK( 10 ) ){
		pWork->pad_status = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time -= DIRECT_TICK( 4 );
	} else if ( pWork->pad_check & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time++;
	} else {
		pWork->pad_check = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_L | PAD_R ) ) ){
		pWork->hold_time = 0;
		pWork->pad_status = 0;
	}
	// hold time
	if ( !( GV_PadDataDirect[ 0 ].press & ( PAD_L | PAD_R ) ) ){
		pWork->time++;
		pWork->time %= ( JPEG_DECODE_START_TIME + 1 );
	}
	PrePositionUpdate( pWork );
}

static void PadControlNormal( Work *pWork )
{
	SPR_OBJ *spr;
	int  page_max;

	// そのページのphoto数を出す
	if ( pWork->num_counter == pWork->photo.files ){
		page_max = ( int )( ( float )( pWork->photo.files + 9 ) / ( float )DISP_PHOTO_MAX ); // 最大ページ数を出す
	} else {
		page_max = ( int )( ( float )( pWork->num_counter ) / ( float )DISP_PHOTO_MAX ); // 最大ページ数を出す
	}
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) || !( pWork->flag & SK_FONT_FADE_IN_OK ) ) {
		return;
	}
	// main
	if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status & PAD_U ){
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->position > 0 ){
			pWork->position_sign--;
			pWork->position--; // <-- リストを進める数の事 いるのか？
			pWork->dot_hokan.end_y = pWork->font[ pWork->position ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 5;
			pWork->dot_hokan.flag = 0;
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_PHOTO_LOAD_OK;
			// close nowload
			if ( ( pWork->action == STR_OPENNOWLOAD ) && !( pWork->photo_list->flag & SK_BREAK_FILE ) ){
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->action = STR_CLOSENOWLOAD;
			}
			pWork->photo.mcscr->step = MCSCR_SEL_FILE;
			SE_SEL();
		} else if ( pWork->page.num - 1 > 0 ){
			pWork->page.num--;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ DISP_PHOTO_MAX - 1 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
			// etc
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_L;
			pWork->flag |= SK_LIST_UP;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_LOAD_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_R;
			// close nowload
			if ( ( pWork->action == STR_OPENNOWLOAD ) && !( pWork->photo_list->flag & SK_BREAK_FILE ) ){
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->action = STR_CLOSENOWLOAD;
			}
			pWork->photo.mcscr->step = MCSCR_SEL_FILE;
			SE_TYPE();
		}
	} else if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status & PAD_D ){
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->position < pWork->page.now_page_max - 1 ){
			pWork->position_sign++;
			pWork->position++; // <-- リストを進める数の事
			// dot
			pWork->dot_hokan.end_y = pWork->font[ pWork->position ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 5;
			pWork->dot_hokan.flag = 0;
			// etc
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_PHOTO_LOAD_OK;
			// close nowload
			if ( ( pWork->action == STR_OPENNOWLOAD ) && !( pWork->photo_list->flag & SK_BREAK_FILE ) ){
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->action = STR_CLOSENOWLOAD;
			}
			pWork->photo.mcscr->step = MCSCR_SEL_FILE;
			SE_SEL();
		} else if ( pWork->page.num < page_max ){
			pWork->page.num++;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ 0 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
			// etc
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_R;
			pWork->flag &= ~SK_PHOTO_LOAD_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_L;
			// close nowload
			if ( ( pWork->action == STR_OPENNOWLOAD ) && !( pWork->photo_list->flag & SK_BREAK_FILE ) ){
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->action = STR_CLOSENOWLOAD;
			}
			pWork->photo.mcscr->step = MCSCR_SEL_FILE;
			SE_TYPE();
		}
	} else if ( ( ( GV_PadDataDirect[ 0 ].press & PAD_L1 || pWork->pad_status & PAD_L1 ) ||
		   ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status & PAD_L ) ) ){ // page 切替
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->page.num > 1 ){
			pWork->page.num--;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ 0 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
			// etc
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_L;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_LOAD_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_R;
			// close nowload
			if ( ( pWork->action == STR_OPENNOWLOAD ) && !( pWork->photo_list->flag & SK_BREAK_FILE ) ){
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->action = STR_CLOSENOWLOAD;
			}
			pWork->photo.mcscr->step = MCSCR_SEL_FILE;
			SE_TYPE();
		}
	} else if ( ( ( GV_PadDataDirect[ 0 ].press & PAD_R1 || pWork->pad_status & PAD_R1 ) || 
		   ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status & PAD_R ) ) ){
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->page.num < page_max ){
			pWork->page.num++;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ 0 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
			// etc
			pWork->flag |= SK_PHOTO_FADE_OUT_START;
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_R;
			pWork->flag &= ~SK_PHOTO_LOAD_OK;
			pWork->flag &= ~SK_PHOTO_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_L;
			// close nowload
			if ( ( pWork->action == STR_OPENNOWLOAD ) && !( pWork->photo_list->flag & SK_BREAK_FILE ) ){
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->action = STR_CLOSENOWLOAD;
			}
			pWork->photo.mcscr->step = MCSCR_SEL_FILE;
			SE_TYPE();
		} 
	} else if ( ( GV_PadDataDirect[ 0 ].press & SELECT ) && ( pWork->flag & SK_PHOTO_LOAD_OK ) && 
		 !( pWork->photo_list->flag & SK_BREAK_FILE ) && ( pWork->action != STR_OPENNOWLOAD ) ){ // normal screen -> pre normal screen
		pWork->action = STR_OPENEDIT;
		pWork->pre_position = 0;
		// 強制
	 	pWork->photo.mcscr->step = MCSCR_SEL_FILE;//   <--- 2001/06/29
		pWork->time = DIRECT_TICK( JPEG_DECODE_START_TIME );
		// flag control
		pWork->flag |= SK_PRE_NORMAL_SCREEN_START;
		pWork->flag |= SK_PHOTO_FADE_OUT_START;
		pWork->flag |= SK_PAGE_FADE_OUT_START;
		pWork->flag |= SK_FONT_FADE_OUT_START;
		pWork->flag |= SK_DOT_FADE_OUT_START;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_PHOTO_FADE_IN_START;
		pWork->flag &= ~SK_PAGE_FADE_IN_START;
		pWork->flag &= ~SK_FONT_FADE_IN_START;
		pWork->flag &= ~SK_DOT_FADE_IN_START;
		pWork->flag &= ~SK_FULL_SCREEN_OK;
		pWork->flag &= ~SK_NORMAL_SCREEN_OK;
		pWork->flag &= ~SK_PHOTO_FADE_IN_OK;
		pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
		pWork->flag &= ~SK_FONT_FADE_IN_OK;
		pWork->flag &= ~SK_FONT_FADE_OUT_OK;
		pWork->flag &= ~SK_PAGE_FADE_IN_OK;
		pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
		pWork->flag &= ~SK_DOT_FADE_IN_OK;
		pWork->flag &= ~SK_DOT_FADE_OUT_OK;
		pWork->flag &= ~SK_ACTION_OK;
		SE_OK();
		return;
	} else if ( ( GV_PadDataDirect[ 0 ].press & CANCEL ) ){ // back exit proc call
		switch( pWork->port ){
		case 0 : // Memory Card Slot 1
			pWork->action = STR_CLOSELIST;//STR_CLOSEEDITSLOT1;
			break;
		case 1 : // Memory Card Slot 2
			pWork->action = STR_CLOSELIST;//STR_CLOSEEDITSLOT2;
			break;
		case 2 : // hard disk
			break;		
		}
		SPR_HIDE( pWork->dot.dot_curs );
		spr = L2D_GetObject( pWork->handle_2d , STR_LOADING_TEX );
		if ( spr != NULL ){
			SPR_HIDE( spr );
		}
		pWork->flag |= SK_PHOTO_FADE_OUT_START | SK_FONT_FADE_OUT_START | SK_PAGE_FADE_OUT_START;
		pWork->flag &= ~SK_PHOTO_LOAD_OK;
		pWork->flag &= ~SK_PHOTO_FADE_IN_START;
		pWork->flag &= ~SK_FONT_FADE_IN_START;
		pWork->flag &= ~SK_PAGE_FADE_IN_START;
		pWork->flag &= ~SK_PHOTO_FADE_OUT_OK;
		pWork->flag &= ~SK_FONT_FADE_OUT_OK;
		pWork->flag &= ~SK_PAGE_FADE_IN_OK;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->flag |= SK_CHANCEL;
		SE_CANCEL();
	}
	// auto mode
	if ( pWork->hold_time >= DIRECT_TICK( 10 ) ){
		pWork->pad_status = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time -= DIRECT_TICK( 4 );
	} else if ( pWork->pad_check & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time++;
	} else {
		pWork->pad_check = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_U | PAD_D | PAD_L1 | PAD_R1 | PAD_L | PAD_R ) ) ){
		pWork->hold_time = 0;
		pWork->pad_status = 0;
	}
	// hold time
	if ( !( GV_PadDataDirect[ 0 ].press & ( PAD_U | PAD_D | PAD_L1 | PAD_R1 | SELECT | CANCEL ) ) ){
		pWork->time++;
		pWork->time %= ( JPEG_DECODE_START_TIME + 1 );
	}
}

static inline void OverWriteColorRe( Work *pWork )
{
	int i;

	// 色をもどす
	pWork->over_write.a[ 0 ] = SK_LOW_A;
	pWork->over_write.a[ 1 ] = 128;
	for( i = 0 ; i < OVER_WRITE_YES ; i ++ ){
		pWork->over_write.obj[ i ]->sprite.col.a = pWork->over_write.a[ 0 ];
	}
	pWork->over_write.obj[ OVER_WRITE_YES + 1 ]->sprite.col.a = 128;
	for( i = OVER_WRITE_YES + 3 ; i < OVER_WRITE_YES_NO ; i ++ ){
		pWork->over_write.obj[ i ]->sprite.col.a = pWork->over_write.a[ 1 ];
	}
}

#define STR_MARUBATSU (12747845)
static void MarubatsuHideShow( Work *pWork , char mode ) // 説明文章の制御
{
	SPR_OBJ *spr;

	spr = L2D_GetObject( pWork->handle_2d , STR_MARUBATSU );
	if ( spr == NULL ){
		return;
	}
	if ( mode == 0 ){
		SPR_HIDE( spr );
	} else {
		SPR_SHOW( spr );
	}
}

static void PadControlOverWrite( Work *pWork )
{
	int status;
	int i;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( status == L2D_STAT_BUSY ){
		return;
	}	
	if ( GV_PadDataDirect[ 0 ].press & PAD_L ){
		if ( pWork->over_write.position == 1 ){
			pWork->over_write.position--;
		}
		SE_SEL();
	} else if ( GV_PadDataDirect[ 0 ].press & PAD_R ){
		if ( pWork->over_write.position == 0 ){
			pWork->over_write.position++;
		}
		SE_SEL();
	} else if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		switch( pWork->over_write.position ){
		case 0 : // ok
			OverWrite( pWork );
			pWork->flag |= SK_SAVE_WAIT;
			pWork->end_time = 0;
			MarubatsuHideShow( pWork , 0 );
			SE_OK();
			break;
		case 1 : // cancel
			pWork->over_write.position = 1;
			SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
			for ( i = 0 ; i < 2 ; i ++ ){
				SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
			}
			pWork->jpn_font.position = 0xff;
			StringCleate( pWork );
			// 色をもどす
			OverWriteColorRe( pWork );
			// action
			pWork->action = STR_CLOSEOVERWRITE;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			MarubatsuHideShow( pWork , 1 );
			SE_CANCEL();
			break;
		}
	} else if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		pWork->over_write.position = 1;
		SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
		for ( i = 0 ; i < 2 ; i ++ ){
			SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
		}
		pWork->jpn_font.position = 0xff;
		StringCleate( pWork );
		// 色をもどす
		OverWriteColorRe( pWork );
		// action
		pWork->action = STR_CLOSEOVERWRITE;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->flag &= ~SK_OVER_WRITE_MODE;
		MarubatsuHideShow( pWork , 1 );
		SE_CANCEL();
	}
}

static void PadAct( Work *pWork ) // パッドコントロール
{
	int status;

	// page 
	if ( pWork->page.num < pWork->page.max_num || 
		 ( ( pWork->photo.files != 0 ) && ( pWork->photo.files % DISP_PHOTO_MAX ) == 0 ) ){
		pWork->page.now_page_max = DISP_PHOTO_MAX;
	} else {
		pWork->page.now_page_max = pWork->photo.files % DISP_PHOTO_MAX;
	}
	if ( !( pWork->hokan[ 0 ].flag & SK_MOVE_OK ) || !( pWork->hokan[ 1 ].flag & SK_MOVE_OK ) ){
		return;
	}
	// main
	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( pWork->flag & SK_NORMAL_SCREEN_OK ) && ( status != L2D_STAT_BUSY ) && !( pWork->flag & SK_DELAY ) ){
		PadControlNormal( pWork );
	}
#if 0
	// loding 中は操作不能
	if ( ( MCManAccessing() != 0 ) ){
		return;
	}
#endif
	if ( pWork->flag & SK_PRE_NORMAL_SCREEN_OK ){
		PadControlPreNormal( pWork );
	}
	if ( pWork->flag & SK_FULL_SCREEN_OK ){
		PadControlFullScreen( pWork );
	}
}

static void NormalAct( Work *pWork )
{
	int status;
	int i;

	status = L2D_ActionStatus( pWork->handle_2d );
	// 初期化
	if ( pWork->flag & SK_PHOTO_FADE_OUT_OK ){
		pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
		if ( pWork->photo_list->pNext != NULL ){
			pWork->photo_list = pWork->photo_list->pNext;
		}
		pWork->photo_list = PhotoListSearch( pWork->photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
		if ( pWork->photo_list == NULL ){
			return;
		}
		for ( i = 0 ; i < pWork->position ; i ++ ){
			pWork->photo_list = PhotoListForward( pWork->photo_list );
		}
	}

	MemoryLoad( pWork ); // memoryにメモりーカードからデータを読み込む
	if ( pWork->flag & SK_END ){
		return;
	}
	AnimetionAct( pWork );
	MemoryCardFade( pWork );
	if ( pWork->flag & SK_CHANCEL ){ // 終了
		SPR_HIDE( pWork->dot.dot_curs );
		if ( ( pWork->flag & SK_FONT_FADE_OUT_OK ) && ( pWork->flag & SK_PHOTO_FADE_OUT_OK ) && ( pWork->flag & SK_PAGE_FADE_OUT_OK ) &&
			 ( status != L2D_STAT_BUSY ) ){
			// 親に送信
			GV_CallParentSignalFunc( pWork , SK_DESTROY , 0 );
			GV_DestroyActor( pWork );
		}
		return;
	}
	PhotoToJpeg( pWork );
	PhotoDisp( pWork );
	if ( pWork->flag & SK_RGB_CHENG ){ // color change
		if ( pWork->flag & SK_SAVE_TIME ){
			if ( pWork->end_time <= 0 ){
				pWork->action = STR_CLOSEOVERWRITE;
				pWork->flag |= SK_ACTION_START;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->flag &= ~SK_SAVE_TIME;
				pWork->flag &= ~SK_OVER_WRITE_MODE;
				for ( i = 0 ; i < 2 ; i ++ ){
					SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
				}
				pWork->jpn_font.position = 0xff;
				StringCleate( pWork );
				// 色をもどす
				OverWriteColorRe( pWork );
				return;
			} else {
				pWork->end_time--;
			}
			return;
		}
		// over_write 操作
		if ( pWork->flag & SK_OVER_WRITE_MODE ){
			if ( !( pWork->flag & SK_SAVE_WAIT ) ){
				OverWriteFontControl( pWork );
				PadControlOverWrite( pWork );
			}
			OverWriteEnd( pWork );
		}
		RGB_Menu( pWork );
		RGBUpdate( pWork );
		RGB_BarMove( pWork );
		RGB_SelectUpdate( pWork );
		return;
	}
	if ( pWork->flag & SK_FULL_SCREEN_START || pWork->flag & SK_PRE_NORMAL_SCREEN_START || pWork->flag & SK_NORMAL_SCREEN_START ){
		FullScreen( pWork );
		return;
	}
	if ( pWork->flag & SK_PRE_NORMAL_SCREEN_OK ){
		EditNameDisp( pWork );
	}
	if ( pWork->flag & SK_FONT_FADE_OUT_OK && pWork->flag & SK_PHOTO_FADE_OUT_OK &&
		 ( pWork->flag & SK_PAGE_CHANGE_L || pWork->flag & SK_PAGE_CHANGE_R ) ){
		PageChange( pWork );
	}

	// morf set
	if ( pWork->flag & SK_NAME_ENTRY_END ){
		// value で menu 非表示
		pWork->hokan[ 0 ].p = 0.0f;
		pWork->hokan[ 0 ].count = HOKAN_SPEED;
		pWork->hokan[ 0 ].code1 = STR_HIDE_ALB_MARUBATU;
		pWork->hokan[ 0 ].code2 = STR_SHOW_ALB_MARUBATU;
		pWork->hokan[ 0 ].morf_parts = STR_ALBAM_MARUBATSU;
		pWork->hokan[ 0 ].flag = 0;

		pWork->hokan[ 1 ].p = 0.0f;
		pWork->hokan[ 1 ].count = HOKAN_SPEED;
		pWork->hokan[ 1 ].code1 = STR_HIDE_ALB_FILTER;
		pWork->hokan[ 1 ].code2 = STR_SHOW_ALB_FILTER;
		pWork->hokan[ 1 ].morf_parts = STR_ALBUM_MARUBATSU_RECT;
		pWork->hokan[ 1 ].flag = 0;

		pWork->flag &= ~SK_NAME_ENTRY_END;
	}

	if ( pWork->photo.mcscr->step > MCSCR_LIST_FILE ){
		// ネームエントリー中はゆうこときかない
		if ( pWork->flag & SK_NAME_ENTRY || pWork->flag & SK_PAGE_CHANGE_L || pWork->flag & SK_PAGE_CHANGE_R ){
			return;
		}
		PadAct( pWork );
		DotUpdate( pWork );
		RGBUpdate( pWork );
		FontUpDate( pWork );
		PageUpDate( pWork ); // num -> ascci そして 入力
		DamagedCheck( pWork );
	}
}

static inline int MorfAct( Work *pWork , Hokan *pHokan ) // これのみ実行
{
	if ( !( pHokan->flag & SK_MOVE_OK ) ){
		MorfL2d( pWork->handle_2d , pHokan->morf_parts  , pHokan->code2  , pHokan->code1  , pHokan );
	} else {
		return ( 0 );
	}
	return( 1 );
}

// モーフィングを設定してくれる関数
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan )
{
	void *parts;

	parts = L2D_GetParts( handle , strcode );    /* パーツポインタの取得 */

	if ( parts == NULL ){
		SK_Err("morf failed\0");
		return;
	}
	hokan->p += ( 1.0f - hokan->p ) / hokan->count;
	hokan->count--;

	L2D_MorfObject( parts , code1 , code2 , hokan->p );
	if ( ( hokan->p == 1.0f ) || ( hokan->count == 0 ) ){
		hokan->flag = SK_MOVE_OK;
	}
}

static void Act( Work *pWork )
{
	int type;
	int i;

   SPR_OBJ *spr;
   spr = L2D_GetObject( pWork->handle_2d, 0x07f3b88 );
   spr->sprite.col.a = 0x40;


	if ( ( pWork->num_counter > 30 ) || ( pWork->num_counter == pWork->photo.files ) ){
	 	pWork->act( pWork );
	} else if ( MCManChecked() ) { // メモリーカードチェック
		type = MCManIsMCChanged( pWork->port );
		printf("Load before type %d\n" , type );
		if ( type != 0 ){
			// 親に送信
			GV_CallParentSignalFunc( pWork , SK_DESTROY , 0 );
			GV_DestroyActor( pWork );
			pWork->flag |= SK_END;
			return;
		}
	}

	for ( i = 0 ; i < 2 ; i ++ ){
		MorfAct( pWork , &pWork->hokan[ i ] );
	}
	ListAutoInsert( pWork );
	StringDisp( pWork );
}

static	void	Die( Work *pWork )
{
	SPR_OBJ *spr;

	PhotoListFree( pWork ); // リストの解放
	if ( pWork->photo.data != NULL ){
		GV_DelayedFree( pWork->photo.data );
	}
	// layout
	if ( pWork->handle >= 0 ){
		spr = L2D_GetObject( pWork->handle , STR_ROOT );
		if ( spr == NULL ){
			return;
		}
		SPR_SHOW( spr );
	}
#if 0
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
#endif
}

static void AnimetionAct( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULT :
			pWork->action = 0;
			break;
		case STR_OPENEDIT :
		case STR_FULL_SCREEN_TO_PRE_SCREEN_OK :
			pWork->flag |= SK_PRE_NORMAL_SCREEN_OK;
			pWork->action = 0;
			break;
		case STR_OPENOVERWRITE : // over write view
			pWork->jpn_font.position = 10;
			StringCleate( pWork );
			SK_AllShow( pWork->over_write.obj , OVER_WRITE_YES_NO );

			pWork->flag |= SK_OVER_WRITE_MODE;
			pWork->action = 0;
			break;
		case STR_CLOSEOVERWRITE : // over write view
			if ( pWork->over_write.position == 0 ){
	 			pWork->flag |= SK_RGB_FADE_OUT_START;
				pWork->flag &= ~SK_RGB_FADE_IN_START;
				pWork->flag &= ~SK_RGB_FADE_IN_OK;
				pWork->flag &= ~SK_RGB_FADE_OUT_OK;
				if ( pWork->pre_position == 0 ){
					// value で menu 非表示
					pWork->hokan[ 0 ].p = 0.0f;
					pWork->hokan[ 0 ].count = HOKAN_SPEED;
					pWork->hokan[ 0 ].code1 = STR_HIDE_ALB_MARUBATU;
					pWork->hokan[ 0 ].code2 = STR_SHOW_ALB_MARUBATU;
					pWork->hokan[ 0 ].morf_parts = STR_ALBAM_MARUBATSU;
					pWork->hokan[ 0 ].flag = 0;
	
					pWork->hokan[ 1 ].p = 0.0f;
					pWork->hokan[ 1 ].count = HOKAN_SPEED;
					pWork->hokan[ 1 ].code1 = STR_HIDE_ALB_FILTER;
					pWork->hokan[ 1 ].code2 = STR_SHOW_ALB_FILTER;
					pWork->hokan[ 1 ].morf_parts = STR_ALBUM_MARUBATSU_RECT;
					pWork->hokan[ 1 ].flag = 0;
				} else {
					// value で menu 非表示
					pWork->hokan[ 0 ].p = 0.0f;
					pWork->hokan[ 0 ].count = HOKAN_SPEED;
					pWork->hokan[ 0 ].code1 = STR_HIDE_RGB_MARUBATU;
					pWork->hokan[ 0 ].code2 = STR_SHOW_RGB_MARUBATU;
					pWork->hokan[ 0 ].morf_parts = STR_ALBAM_MARUBATSU_RGB;
					pWork->hokan[ 0 ].flag = 0;
	
					pWork->hokan[ 1 ].p = 0.0f;
					pWork->hokan[ 1 ].count = HOKAN_SPEED;
					pWork->hokan[ 1 ].code1 = STR_HIDE_RGB_FILTER;
					pWork->hokan[ 1 ].code2 = STR_SHOW_RGB_FILTER;
					pWork->hokan[ 1 ].morf_parts = STR_ALBUM_MARUBATSU_RECT;
					pWork->hokan[ 1 ].flag = 0;
				}
			} else { // cancel
			}
			pWork->flag &= ~SK_OVER_WRITE_MODE;
			pWork->action = 0;
			break;
		case STR_CLOSERGB :
			pWork->flag &= ~SK_RGB_CHENG;
			pWork->color_mode_position = 0;
			pWork->action = 0;
			break;
		case STR_OPENNOWLOAD :
			pWork->photo.mcscr->step = MCSCR_START_SAVELOAD;
			pWork->flag &= ~SK_OPEN_NOW_LOAD;
			break;
		}
		pWork->flag &= ~SK_ACTION_OK;
	}
	if ( pWork->flag & SK_SIGNAL_OK && !( pWork->flag & SK_NORMAL ) ){
		if ( SprInit( pWork ) < 0 ){ // 文字の初期化
			GV_DestroyActor( pWork );
			return;
		}
		pWork->act = ( void * )NormalAct;
		pWork->flag &= ~SK_SIGNAL_OK;
	}
	// 特殊
	switch ( pWork->action ){
	case STR_OPENLISTSLOT1 :
	case STR_OPENLISTSLOT2 :
		// layout
		{
			SPR_OBJ *spr;

			if ( pWork->flag & SK_ACTION_OK ){
				spr = L2D_GetObject( pWork->handle , STR_ROOT );

				if ( spr == NULL ){
					return;
				}
				SPR_HIDE( spr );
			}
		}
		break;
	}
	// 実行
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}
}

#define SIG_FULL_SCREEN_FADE_IN (14194161)
#define SIG_OPEN_LIST_FADE_IN (1845022)
#define SIG_EDIT_FADE_IN (737268)
#define SIG_HIDEMEMORY (16024742)
#define SIG_RGB_FADE_IN (10034471)
#define SIG_RGB_FADE_OUT (2342471)

static void Signal( void *work , int sign , int value )
{
	Work *pWork;

	pWork = ( Work * )work;

	switch ( sign ){
	case SIG_OPEN_LIST_FADE_IN : // openlistのタイミングを計る
		pWork->flag |= SK_SIGNAL_OK;
		break;
	case SIG_EDIT_FADE_IN : // openlistのタイミングを計る
	case SIG_FULL_SCREEN_FADE_IN :
		pWork->flag |= SK_SIGNAL_OK;
		break;
	case SIG_HIDEMEMORY : // Memory Card
		pWork->memory.alpha = 128.0f;
		pWork->memory.alpha_add = ( 128.0f * 2 ) / ( float )value; // 上乗せJPEGの都合
		pWork->flag |= SK_MEMORY_FONT_FADE;
		break;
	case SIG_RGB_FADE_IN :
		pWork->rgb_menu.a = 128;
		pWork->flag |= SK_RGB_FADE_IN_START;
		break;
	case SIG_RGB_FADE_OUT :
		RgbMask( pWork , 1 );
		break;
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	void      *ptr;
	float     ftmp;
	int       i;

	ftmp = ( 384.0f / 448.0f );

	work->act = (void *)AnimetionAct;
	work->position = 0;
	work->color_mode_position = 0;
	work->position_sign = 0;
	work->pad_status = 0;
	work->pad_check = 0;
	work->time = 0;
	work->num_counter = 0;
	work->flag = 0;
	work->flag |= ( SK_PAGE_FADE_IN_START | SK_NORMAL_SCREEN_OK | SK_PHOTO_FADE_OUT_OK | SK_FONT_FADE_IN_START | 
					SK_RGB_FADE_OUT_OK | SK_ACTION_START );
	// l2d
	L2D_SetSignalHandle( work->handle_2d , work , Signal );
	switch( work->port ){
	case 0 : // Memory Card Slot 1
		work->action = STR_OPENLISTSLOT1;
		break;
	case 1 : // Memory Card Slot 2
		work->action = STR_OPENLISTSLOT2;
		break;
	case 2 : // hard disk
		break;		
	}
	// hokan
	for ( i = 0 ; i < 2 ; i ++ ){
		work->hokan[ i ].flag = SK_MOVE_OK;
	}
	LangChecge( work ); // 言語切替関数
	// dot
	work->dot.r = SK_R * 2;
	work->dot.g = SK_G * 2;
	work->dot.b = SK_B * 2;
	work->dot.a = 128;
	work->dot_hokan.p = 0.0f;
	work->dot_hokan.flag = 0;
	work->dot_hokan.start_y = 0;
	work->dot_hokan.end_y = 0;
	work->dot_hokan.count = 0;
	// rgb
	work->rgb_menu.r = SK_R;
	work->rgb_menu.g = SK_G;
	work->rgb_menu.b = SK_B;
	work->rgb_menu.a = 0;
	// page
	work->page.max_num = 1;
	work->page.num = 1;
	work->page.a = 128;
	// photo
	work->photo.x = 281.0f;
	work->photo.y = 101.5f;
	work->photo.w = 198.0f-2;
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   if ( BP_Area_EU() )
	   work->photo.h = 150.0f * ftmp;
//#else
   else
      work->photo.h = 144.0f * ftmp;
//#endif

   work->photo.h -=2;

	work->photo.r = 0;
	work->photo.g = 0;
	work->photo.b = 0;
	work->photo.a = 0;
	work->photo.files = 0;
	// photo list
	if ( PhotoListInitialize( work ) < 0 ){
		return ( -1 );
	}
	// mc
	work->file_counter = 0;
	work->photo.id_max = MCManMaxId();
	work->photo.files = MCManExactNFiles();
	work->page.max_num = ( int )( ( float )( work->photo.files + 9 ) / ( float )DISP_PHOTO_MAX ); // 最大ページ数を出す
	if ( ListAll( work ) < 0 ){ // default で 一番先頭のものになる
		return ( -1 );
	}
#if 0
	printf("list ok");
	ListDebug( work );
#endif
	// work
   // TODO: PHOTE_SAVE get rid of this fix file size logic and use varaible size files
	work->photo.data = GV_Malloc( JPEG_SIZE_LIMIT );
	if ( work->photo.data == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( work->photo.data , JPEG_SIZE_LIMIT );

	ptr = NewTextScreenControlEx( 30 * SUBTITLE_FONT_WIDTH , SUBTITLE_FONT_HEIGHT * 2 + 16 , 240, 0, 0/*bufferedTextFlag*/ );
	if ( ptr == NULL ){
		return (-1);
	}
	GV_SetActorChild( work , ptr );
	work->jpn_font.work = ptr;
	work->jpn_font.position = 0xff;
	StringCleate( work );

	ptr = NewPictureDrawManager( 140 );
	if ( ptr == NULL ){
		return (-1);
	}
	GV_SetActorChild( work , ptr );
//	GV_SetActorChild( work , NewBlackRect() );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewAlbamMode( MCScrWork *mcscr , int port , int handle , int handle_out )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->port = port;
	work->photo.mcscr = mcscr;
	work->handle = handle;
	work->handle_2d = handle_out;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

