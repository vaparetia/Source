//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   photo_save.c
   写真セーブターミナルモード
   
   2001/07/12	S.Kobayashi
   $Id: photo_save.c,v 1.5 2002/12/05 18:42:01 takaki Exp $
*/

/* メモリーカードがやばいかも */

#if 0 //BP_PS2 def PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"
#include    "../../mode/codec/cjimaku.h"
#include    "../../kano/mcman/mcman.h"

#include "../../mode/codec/codec_signal.h"
#include "../../mode/codec/codecmem.h"
#include "../../mode/codec/cdc_load.h"
#include "../../kano/titlescr/subtitle.h"
#include "../../kano/titlescr/portsel.h"
#include "libfs.h"

#include "BP_LocalizedTextByEnum.h"

#include "mode/menu/xtextscn.h"

#if defined(BP_VITA)
#include "..\kano\titlescr\transfarring_save_load_menu_utils.h"
#include "BP_Misc.h"
#include "Transfarring_UI_Strings.h"
#include "TransfarringVTACGlue.h"
#else
#include "BP_Transfarring.h"
#endif

#define		LAYOUT		   (4691731)
#define 	STR_NODE_FONT  (0x0034ca34)
#define     STR_DEFAULT    (566267)
#define     STR_ROOT       (2770484)

#define OVER_WRITE (64)
#define OVER_WRITE_YES_NO (8) // plus 'yes / no'
#define OVER_WRITE_YES (3) // yes
#define OVER_WRITE_NO  (2) // yes
// etc
#define 	DEFAULT_POS_X ( 59.0f )
#define		DEFAULT_POS_Y ( 86.0f )
#define 	FONT_WIDTH ( 18.0f )
#define 	FONT_HEIGHT ( 14.0f )
#define     FONT_SPACE_HEIGHT ( 4.0f )// DRAW_HEIGHT / 384.0f )
#define 	STR_WIDTH  (18) // strcode ではなく文字の長さ
#define 	STR_HEIGHT (14) // strcode ではなく文字の長さ

#define SK_R (80)//(80) 
#define SK_G (90)//(94) 
#define SK_B (85)//(85) 
#define SK_A (52)
#define IN_FADE_SPEED ( DIRECT_TICK( 6 ) )
#define OUT_FADE_SPEED ( DIRECT_TICK( 12 ) )
// 93
#define DEFAULT_PAGE_POS_X (59)//(345)//(320)
#define DEFAULT_PAGE_POS_Y ( 42 )
#define DEFAULT_PAGE_NUM_POS_X (400)//(375)
#define DEFAULT_PAGE_NUM_POS_Y ( 42 )
#define DEFAULT_PAGE_SURASHU_X (145)//(438)//(426)
#define DEFAULT_OVER_WRITE_POS_X (39.0f)
#define DEFAULT_OVER_WRITE_POS_Y (310.0f)
#if 0
#  ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
#    define DEFAULT_OVER_WRITE_SELECT_POS_X (410.0f)
#    define DEFAULT_OVER_WRITE_SELECT_POS_Y (318.0f)
#  else
#    define DEFAULT_OVER_WRITE_SELECT_POS_X (386.0f)
#    define DEFAULT_OVER_WRITE_SELECT_POS_Y (310.0f)
#  endif
#else
//#  define DEFAULT_OVER_WRITE_SELECT_POS_X ( BP_Area_EU() ? 410.0f : 386.0f )
#  define DEFAULT_OVER_WRITE_SELECT_POS_X ( 388.0f )
#  define DEFAULT_OVER_WRITE_SELECT_POS_Y ( BP_Area_EU() ? 318.0f : 310.0f )
#endif

#define PAGE_FONT_MAX ( 5 ) // 00/00
#define PAGE_MAX (4)
#define DISP_PHOTO_MAX (10) // 一画面に表示する写真項目数の最大値
#define STR_TITLE_LEN  (64)
#define END_TIME ( DIRECT_TICK(40) )

#define JPEG_SIZE ( 720*1280 )
// Japanease or Usa or Pal
#if 0
//#ifdef JAPANESE_BP_IGNORE() // 日本

#define SELECT PAD_OK
#define CANCEL PAD_CANCEL

#else 

#define SELECT PAD_OK
#define CANCEL PAD_CANCEL

#endif 

#define R_POSITION (13)
#define G_POSITION (14)
#define B_POSITION (15)

#define MALLOC(_size)		codecMalloc(_size)
#define FREE(_addr,_size)	codecFree(_addr)

// kanoさん
#define N_FILES			MCSCR_N_FILES
#define NO_SAVEFILE		0xffff

#if defined(BP_VITA)
#define SAVERES_DELETE_X	64
#define SAVERES_DELETE_Y	345
#endif

/////////////////////////////////

// enum -> #define  chaned by T.Morita 2002.05.23
#define	SK_NORMAL                   I64(0x0000000000000001)
#define	SK_PHOTO_SAVE_START         I64(0x0000000000000002)
#define	SK_PHOTO_SAVE_OK            I64(0x0000000000000004)
#define	SK_PAGE_FADE_IN_START       I64(0x0000000000000080)
#define	SK_PAGE_FADE_IN_OK          I64(0x0000000000000100)
#define	SK_PAGE_FADE_OUT_START      I64(0x0000000000000200)
#define	SK_PAGE_FADE_OUT_OK         I64(0x0000000000000400)
#define	SK_FONT_FADE_IN_START       I64(0x0000000000000800)
#define	SK_FONT_FADE_IN_OK          I64(0x0000000000001000)
#define	SK_FONT_FADE_OUT_START      I64(0x0000000000002000)
#define	SK_FONT_FADE_OUT_OK         I64(0x0000000000004000)
#define	SK_CHANCEL                  I64(0x0000000000800000)
#define	SK_PAGE_CHANGE_L            I64(0x0000000001000000)
#define	SK_PAGE_CHANGE_R            I64(0x0000000002000000) 
#define	SK_NORMAL_SCREEN_START      I64(0x0000000040000000)
#define	SK_NORMAL_SCREEN_OK         I64(0x0000000080000000)
#define	SK_FREE_SPACE               I64(0x0000000100000000)
#define	SK_SAVE_WAIT                I64(0x0000000200000000)
#define	SK_OVER_WRITE_MODE          I64(0x0000002000000000)
#define	SK_FONT_OUT_START           I64(0x0000004000000000)
#define	SK_FONT_OUT_OK              I64(0x0000008000000000)
#define	SK_LIST_UP                  I64(0x0000010000000000)
#define	SK_FIRST_FADE_OK            I64(0x0000020000000000)
#define	SK_DOT_FADE_OUT_START       I64(0x0000040000000000)
#define	SK_DOT_FADE_OUT_OK          I64(0x0000080000000000)
#define	SK_DOT_FADE_IN_START        I64(0x0000100000000000)
#define	SK_DOT_FADE_IN_OK           I64(0x0000200000000000)
#define	SK_MOVE_OK                  I64(0x0000400000000000)
#define	SK_NOW_SAVING               I64(0x0000800000000000)
#define	SK_SAVE_START               I64(0x0001000000000000)
#define	SK_END                      I64(0x0002000000000000)

//Deleting photos disabled
#define BP_DISABLE_DELETE_FILES

enum {
	SK_DESTROY = 0x1 ,
};

enum {
	SK_FADE_IN = 0x1 ,
	SK_FADE_OUT = 0x2 ,
};

#define ASCCI_MAX (12 + 1) // plus 1 = tarminater
typedef struct _photo_list {
	struct _photo_list *pPrev;
	struct _photo_list *pNext;
	u_char name[ ASCCI_MAX ];
	int id;
	u_char r , g , b , a; // 色
} Photo_List;

typedef struct _photo_save__photo {
	void *data; // jpeg date
	MCMAN_WORK *mcman; // メモりーカード
	u_char r , g , b , a; // 色
	int   files; // ファイル総数
	int   id_max;
} Photo;

typedef struct _photo_save__Page {
	SPR_OBJ *page_now[ PAGE_FONT_MAX ]; // 現在ページ 00/00
	SPR_OBJ *page[ PAGE_MAX ]; // PAGE
	int      max_num; // 最大ページ数
	int      num;     // 現在のページ
	int      now_page_max; // 現在のページの枚数
	u_char   a;
} Page;

typedef struct _photo_save__Overwrite {
	int a[ 2 ]; // Alpha 0 : Yes , 1 : No
	int position;
	SPR_OBJ *obj[ OVER_WRITE_YES_NO ];
	SPR_OBJ *hold_obj[ 2 ][ OVER_WRITE ]; // 変化することのないフォント 2 行
} OverWrite;

typedef struct __photo_save {
	int r;
	int g;
	int b;
	int a;
	void *work;
	char *data;
	int position; // resource 専用
} Font;

typedef struct _photo_save__Dot {
	SPR_OBJ             *dot_curs; // sprite
	SPR_OBJ             *menu_line;
	u_char r , g , b , a;
} Dot;

typedef struct _photo_save__Hokan_Program {
	int     count;
	int     end_y;
	u_long64  flag;
} Hokan_Program;

#if defined(BP_VITA)
typedef enum
{
   kDelete_None,
   kDelete_Clicked,
   kDelete_Selected,
   kDelete_Looping,
   kDelete_Finished,
   kDelete_Error,
}
EDeleteStates;
#endif

typedef	struct _photo_save_work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	SPR_OBJ             *font[ DISP_PHOTO_MAX ][ ASCCI_MAX ];
	SPR_OBJ             **kano_work;  // かのさんのwork
	OverWrite           over_write; // 上書き
	Photo               photo;    // 画像データ
	Photo_List          *photo_list; // 項目リスト
	Page                page;
	Dot                 dot;
	Hokan_Program       dot_hokan;
	u_long64              *photo_top_addres;
	int                 position; // ページ中のどこか？
	int                 pre_position; // edit screenのどこか
	int                 main_position;
	int                 save_id;
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
	void                *save_data;
	int                 save_data_size;
	int                 port;
	int                 end_time;
	int                 format_flag;
	char                *jpeg_data;
	char                cancel_flag;
	u_char              *font_string;
	int                 file_counter;
	int                 num_counter;
	Font                jpn_font;
	int  ( *act )( struct _work * );

#if defined(BP_VITA)
   EDeleteStates mDeleteFlag;
#endif

   MCMAN_WORK* mMemoryCardActor;
} Work ;

// プロトタイプ
static void PadAct( Work * );
static void PageFade( Work *pWork );
static void FontFade( Work *pWork );
static void SearchSaveFile( Work *pWork);

static inline Photo_List *PhotoListForward( Photo_List * ); // リストを安全に進める
static inline Photo_List *PhotoListSearch( Photo_List * , int ); // リストを安全に進める
static inline Photo_List *PhotoListBack( Photo_List * ); // リストを安全に進める
// extern my function
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void SK_PrintfChengColor( SPR_OBJ *pObj , u_char r , u_char g , u_char b , u_char a );

// font
extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え

#define STR_CURSOR (9100063)
#define STR_MENU_LINE (9959061)

/*----------------------------------------------------------------*/
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
	spr = L2D_GetObject( pWork->handle_2d, STR_CURSOR );
	if ( spr == NULL ){
		SK_Err("dot\0");
		return -1 ;
	}
	SPR_HIDE( spr );
	pWork->dot.dot_curs = spr;
	pWork->dot.dot_curs->sprite.col.a = 0;
	SPR_HIDE( pWork->dot.dot_curs );
   pWork->dot.dot_curs->sprite.dw = 7;       //BP JG - put the cursor back to aspect correct - this also aligns it with the border.
   pWork->dot.dot_curs->sprite.pos.x = 40;

	// menu_lineの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_MENU_LINE );
	if ( spr == NULL ){
		SK_Err("menu_line\0");
		return -1 ;
	}
	SPR_HIDE( spr );
	pWork->dot.menu_line = spr;
   pWork->dot.menu_line->box.rect.begin.x = 47;
   pWork->dot.menu_line->box.rect.end.x = 47;

   // ページ用フォント取得
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
	pWork->base_v = spr->sprite.head.tex.v*16.0f; // テクスチャの位置取得
#endif
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_PAGE_NUM_POS_X;
	spr->sprite.pos.y = DEFAULT_PAGE_NUM_POS_Y;
	pWork->page.page_now[ 0 ] = SPR_DuplicateTree( spr );
	for ( i = 1 ; i < PAGE_FONT_MAX ; i ++ ){ // 設定
		pWork->page.page_now[ i ] = SPR_DuplicateTree( spr );
		pWork->page.page_now[ i ]->sprite.pos.x = 0;
		pWork->page.page_now[ i ]->sprite.pos.y = DEFAULT_PAGE_NUM_POS_Y;
	}
	SK_PrintfNormal( NULL , pWork->page.page_now , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_FONT_MAX );
	SPR_HIDE( spr );
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
	pWork->base_v = spr->sprite.head.tex.v*16.0f; // テクスチャの位置取得
#endif
	spr->sprite.dw = 14;
	spr->sprite.dh = 10;
	spr->sprite.pos.x = DEFAULT_PAGE_POS_X;
	spr->sprite.pos.y = DEFAULT_PAGE_POS_Y;
	pWork->page.page[ 0 ] = SPR_DuplicateTree( spr );
	for ( i = 1 ; i < PAGE_MAX ; i ++ ){ // 設定
		pWork->page.page[ i ] = SPR_DuplicateTree( spr );
		pWork->page.page[ i ]->sprite.pos.x = 0;
		pWork->page.page[ i ]->sprite.pos.y = DEFAULT_PAGE_POS_Y;
	}
	{
		char buf[] = "PAGE\0";

		SK_PrintfNormal( NULL , pWork->page.page , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_MAX );
		SK_PrintfNormal( buf , pWork->page.page , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_MAX );
		SK_PrintfChengColor2( pWork->page.page , 0 , 0 , 0 , 0 , PAGE_MAX );
	}
	SPR_HIDE( spr );
	// 文字の取得
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("node_font2\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH - 2;
	spr->sprite.dh = FONT_HEIGHT - 2;
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
		pWork->font[ i ][ 0 ]->sprite.pos.y += ( ( FONT_HEIGHT + FONT_SPACE_HEIGHT ) * i );
		for ( j = 1 ; j < ASCCI_MAX ; j++ ){
			pWork->font[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->font[ i ][ j ]->sprite.pos.x = 0;
			pWork->font[ i ][ j ]->sprite.pos.y += ( ( FONT_HEIGHT + FONT_SPACE_HEIGHT ) * i );
		}
		// font初期化
		SK_PrintfNormal( NULL , pWork->font[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
		SK_PrintfChengColor2( pWork->font[ i ] , photo_list->r , photo_list->g , photo_list->b , photo_list->a , ASCCI_MAX );
	}
	SPR_HIDE( spr );
	// dot init
	pWork->dot_hokan.end_y = pWork->font[ pWork->position ][ 0 ]->sprite.pos.y;
	pWork->dot_hokan.count = 1;
	pWork->dot_hokan.flag = 0;
	// over write
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("node_font3\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH - 2;
	spr->sprite.dh = FONT_HEIGHT - 2;
	spr->sprite.pos.x = DEFAULT_OVER_WRITE_POS_X;
	// main process
	for ( i = 0 ; i < 2 ; i ++ ){
		spr->sprite.pos.y = DEFAULT_OVER_WRITE_POS_Y + ( ( spr->sprite.dh ) * i );
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
	spr->sprite.dw = FONT_WIDTH - 2;
	spr->sprite.dh = FONT_HEIGHT - 2;
	spr->sprite.pos.x = DEFAULT_OVER_WRITE_SELECT_POS_X;
	spr->sprite.pos.y = DEFAULT_OVER_WRITE_SELECT_POS_Y + spr->sprite.dh + 2;
	// main process
	for ( i = 0 ; i < OVER_WRITE_YES_NO ; i ++ ){
		pWork->over_write.obj[ i ] = SPR_DuplicateTree( spr );
	}
	// font初期化
	{
		char buf[] = "YES / NO\0";

		SK_PrintfNormal( NULL , pWork->over_write.obj , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE_YES_NO );
		SK_PrintfNormal( buf , pWork->over_write.obj , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE_YES_NO );
	}
	SK_PrintfChengColor2( pWork->over_write.obj , SK_R , SK_G , SK_B , 0 , OVER_WRITE_YES_NO );
	SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
	SPR_HIDE( spr );
	// flag set
	pWork->flag |= SK_NORMAL;

  	return( 0 );
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
			SPR_HIDE( pWork->dot.menu_line );
		} else {
			pWork->dot.a = ( u_char )alpha;
		}
		SK_PrintfChengColor( pWork->dot.dot_curs , pWork->dot.r , pWork->dot.g , pWork->dot.b , pWork->dot.a );
		pWork->dot.menu_line->line.col[ 0 ].a = pWork->dot.a;
		pWork->dot.menu_line->line.col[ 1 ].a = pWork->dot.a;
		SPR_SHOW( pWork->dot.menu_line );
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
		SK_PrintfChengColor( pWork->dot.dot_curs , pWork->dot.r , pWork->dot.g , pWork->dot.b , pWork->dot.a );
		pWork->dot.menu_line->line.col[ 0 ].a = pWork->dot.a;
		pWork->dot.menu_line->line.col[ 1 ].a = pWork->dot.a;
		SPR_SHOW( pWork->dot.menu_line );
	}
}

static inline void DotUpdate( Work *pWork ) // update
{
	if ( !( pWork->flag & SK_NORMAL ) )
   {
		return;
	}
	// dot position update
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) ){ 
		pWork->dot.dot_curs->sprite.pos.y += ( pWork->dot_hokan.end_y - pWork->dot.dot_curs->sprite.pos.y ) / pWork->dot_hokan.count;
		pWork->dot_hokan.count--;
		if ( pWork->dot_hokan.count <= 0 ){
			pWork->dot_hokan.count = 1;
		}
		if ( pWork->dot.dot_curs->sprite.pos.y == pWork->dot_hokan.end_y ){
			pWork->dot_hokan.flag |= SK_MOVE_OK;
		}
	}
}

static inline int FontPositionY( int position )
{
	switch ( position ){
	case 4 : // 改行がある
#if 0
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		if ( GM_Language == GM_LANG_FRENCH ){
			return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT ) - 1 );
		} else if ( GM_Language == GM_LANG_GERMANY ){
			return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT ) - 1 );
		} else if ( GM_Language == GM_LANG_ITALY ){
			return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT ) - 1 );
		} else if ( GM_Language == GM_LANG_SPANISH ){
			return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT ) - 1 );
		} else { // english
			return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT ) - 1 );
		}
#else
		return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT * 2 ) - 8 );
#endif
#else
		if( GM_Language == GM_LANG_JAPANESE ) {
			return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT * 2 ) - 8 );
		} else {
			return ( DEFAULT_OVER_WRITE_POS_Y - ( SUBTITLE_FONT_DISP_HEIGHT ) - 1 );
		}
#endif
	default :
#if 0
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		return ( DEFAULT_OVER_WRITE_POS_Y + SUBTITLE_FONT_DISP_HEIGHT - 9 );
#else
		return ( DEFAULT_OVER_WRITE_POS_Y + SUBTITLE_FONT_DISP_HEIGHT - 8 );
#endif
#else
		if( GM_Language == GM_LANG_JAPANESE ) {
			return ( DEFAULT_OVER_WRITE_POS_Y + SUBTITLE_FONT_DISP_HEIGHT - 8 );
		} else {
			return ( DEFAULT_OVER_WRITE_POS_Y + SUBTITLE_FONT_DISP_HEIGHT - 9 );
		}
#endif
	}
}

static inline int FontPositionEndY( int position )
{
	switch ( position ){
	case 4 : // 改行がある
      if ( BP_Area_EU() )
      {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		   if ( GM_Language == GM_LANG_FRENCH ){
			   return ( SUBTITLE_FONT_DISP_HEIGHT * 5 );
		   } else if ( GM_Language == GM_LANG_GERMANY ){
			   return ( SUBTITLE_FONT_DISP_HEIGHT * 5 );
		   } else if ( GM_Language == GM_LANG_ITALY ){
			   return ( SUBTITLE_FONT_DISP_HEIGHT * 5 );
		   } else if ( GM_Language == GM_LANG_SPANISH ){
			   return ( SUBTITLE_FONT_DISP_HEIGHT * 5 );
		   } else { // english
			   return ( SUBTITLE_FONT_DISP_HEIGHT * 5 );
		   }
      }
      else
      {
//#else 
		   return ( SUBTITLE_FONT_DISP_HEIGHT * 5 );
      }
//#endif
	default :
		return ( SUBTITLE_FONT_DISP_HEIGHT * 5 );
	}
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
	y1 = FontPositionY( pWork->jpn_font.position );
	x2 = x1 + disp_width;
	y2 = y1 + FontPositionEndY( pWork->jpn_font.position );
	u1 = poly_width;
	v1 = SUBTITLE_FONT_HEIGHT * 5;
	MENU_PutTextScreen( pWork->jpn_font.work , x1 , y1 , x2 , y2 ,
						1 , 1  , u1 + 1 , v1 + 1 , color );
}

#if 0
#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
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
#endif
#endif

#define ENGLISH_STAGE (29)
// 文字の形成
static void StringCleate( Work *pWork )
{
	int poly_width;

	MENU_ClearTextTexture( pWork->jpn_font.work );
	// create
	poly_width = 30 * SUBTITLE_FONT_WIDTH;
#if 0
#ifdef JAPANESE_BP_IGNORE()
	pWork->jpn_font.data = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( 5 , pWork->jpn_font.position ));
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT * 5 + 1 ,
							0 , 0 , pWork->jpn_font.data );
#else // 海外
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	pWork->jpn_font.data = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( 5 , pWork->jpn_font.position ));
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT * 5 + 1 ,
							0 , 0 , pWork->jpn_font.data );
#else 
	pWork->jpn_font.data  = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( 5 , pWork->jpn_font.position ));
	SK_ToUpper( pWork->font_string , pWork->jpn_font.data );
	SK_PrintfNormal( pWork->font_string , SetSpriteY( pWork ) , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );
	GV_ZeroMemory( pWork->font_string , sizeof( u_char ) * 128 );
	pWork->jpn_font.data = NULL;
#endif
#endif
#else
	pWork->jpn_font.data = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( 5 , pWork->jpn_font.position ));
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT * 5 + 1 ,
							0 , 0 , 0 , pWork->jpn_font.data );
#endif
}

#if defined(BP_VITA)
static void DeleteStringDisp ( Work *pWork )
{
   if ( pWork->jpn_font.data != NULL )
   {
      int const disp_width = 30 * SUBTITLE_FONT_DISP_WIDTH;	
      int const poly_width = 30 * SUBTITLE_FONT_WIDTH;

      MENU_PutTextScreen( pWork->jpn_font.work, DEFAULT_OVER_WRITE_POS_X, FontPositionY( pWork->jpn_font.position ),
         DEFAULT_OVER_WRITE_POS_X + disp_width, FontPositionY( pWork->jpn_font.position )
            + FontPositionEndY( pWork->jpn_font.position ), 1, 1, poly_width + 1, SUBTITLE_FONT_HEIGHT * 5 + 1,
               ( 128 << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R );
   }
}

static void DeleteStringCleate( Work *pWork )
{
   if(pWork->mDeleteFlag != kDelete_None)
   {
      int const poly_width = 30 * SUBTITLE_FONT_WIDTH;
      
      if(pWork->mDeleteFlag == kDelete_Clicked)
      {
         pWork->jpn_font.data = BP_GetCustomOverrideString( "SAVELOAD", "DELETE_QUESTION" );
      }
      else
      {
         pWork->jpn_font.data = BP_GetCustomOverrideString( "SAVELOAD", "DELETE_PROGRESS" );
      }
      MENU_ClearTextTexture( pWork->jpn_font.work );

      MENU_CreateTextTexture( pWork->jpn_font.work, 1, 1, poly_width, SUBTITLE_FONT_HEIGHT * 5 + 1,
         0, 0, 0, pWork->jpn_font.data );
   }
}

static void DeleteUpdate( Work *pWork )
{
   if(pWork->flag & SK_NORMAL_SCREEN_OK
      && ( pWork->position + ( ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ) != ( pWork->photo.files - 1 ) ))
   {
      int const color = ( 128 << 24 ) | ( SK_B << 16 ) | ( SK_G << 8 ) | SK_R; 
      int const poly_width = 30 * SUBTITLE_FONT_WIDTH;
      char const * const deleteLabel = BP_GetCustomOverrideString( "SAVELOAD", "DELETE_LABEL" );

      Transfarring_RenderIconSprite(pWork->jpn_font.work, SAVERES_DELETE_X - 15, SAVERES_DELETE_Y,
         TRANSFARRING_SQ_BUTTON_COLOR, kTransfarring_SqButtonIcon, kSaveLoadScreen_Photos);

      MENU_CreateTextTexture( pWork->jpn_font.work, 0, SUBTITLE_FONT_HEIGHT * 6 + 1, poly_width,
         SUBTITLE_FONT_HEIGHT, 0 , 0 , 0 , deleteLabel );
      MENU_PutTextScreen( pWork->jpn_font.work, SAVERES_DELETE_X, SAVERES_DELETE_Y, SAVERES_DELETE_X + FONT_WIDTH * 30,
         SAVERES_DELETE_Y + FONT_HEIGHT, 0, SUBTITLE_FONT_HEIGHT * 6, poly_width, SUBTITLE_FONT_HEIGHT * 7, color );
   }
}
#endif

static void FontUpDate( Work *pWork )
{
	Photo_List *photo_list;
	int  i;
	int  alpha;
	int  tmp;

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
			if ( pWork->position == i ){
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
					alpha -= OUT_FADE_SPEED;;
					if ( alpha <= SK_A ){
						photo_list->a = SK_A;
						tmp++;
					} else {
						photo_list->a = ( u_char )alpha;
					}
				}
			}
			// 更新
			if ( pWork->main_position + DISP_PHOTO_MAX > i && i >= pWork->main_position ){
				SK_PrintfNormal2( NULL , pWork->font[ i - pWork->main_position ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
				SK_PrintfNormal2( photo_list->name , pWork->font[ i - pWork->main_position ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
				SK_PrintfChengColor2( pWork->font[ i - pWork->main_position ] , SK_R , SK_G , SK_B , photo_list->a , ASCCI_MAX );
				SK_AllShow( pWork->font[ i - pWork->main_position ] , ASCCI_MAX );
			}
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
		}
	}
}

static void PageUpDate( Work *pWork ) // num -> ascci そして 入力
{
	float diff;
	char  buf[ PAGE_FONT_MAX + 2 ];
	int   i;

	// new file は いれない
	sprintf( buf , "%02d/%02d" , pWork->page.num , pWork->page.max_num );
#if 0
	if ( pWork->position == pWork->page.now_page_max - 1 ){
		tmp = pWork->page.now_page_max - 1;	
	} else {
		tmp = pWork->position + 1;
	}
	sprintf( buf , "%02d/%02d" , tmp , ( pWork->page.now_page_max - 1 ) );
#endif

	SK_PrintfNormal( NULL , pWork->page.page_now , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_FONT_MAX);
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
	pWork->photo_list = MALLOC( sizeof( Photo_List ) );
	if ( pWork->photo_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->photo_list , sizeof( Photo_List ) );
	pWork->photo_list->id = -1;
	pWork->photo_list->pPrev = NULL;
	pWork->photo_list->pNext = NULL;
	pWork->photo_top_addres = ( u_long64 * )pWork->photo_list;

	return ( 0 );
}

static int PhotoListDataInput( Work *pWork , int id , int mode )
{
#if 0
#  ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	//static u_char break_name[] = "DAMAGED DATA\0";
	static u_char *break_name = (u_char *)"DAMAGED DATA\0";
#  else 
	//static u_char break_name[] = "DAMAGED FILE\0";
	static u_char *break_name = (u_char *)"DAMAGED FILE\0";
#  endif
#endif
   static u_char *break_name_pal = (u_char *)"DAMAGED DATA\0";
   static u_char *break_name_nopal = (u_char *)"DAMAGED FILE\0";
	Photo_List *photo_list;
	u_char *pTmp;
	int i;
   u_char const *break_name;

   break_name = BP_Area_EU() ? break_name_pal : break_name_nopal;

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
	} else {
		for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
			photo_list->name[ i ] = break_name[ i ];
		}
		photo_list->r = 128;
		photo_list->g = 128;
		photo_list->b = 128;
		photo_list->a = 0;
	}

	return ( 0 );
}

// リストに追加する
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
			pNewPhotoList = MALLOC( sizeof( Photo_List ) );
			if ( pNewPhotoList == NULL ){
				printf( " memory malloc failed\n" );
				return ( -1 );
			}
			GV_ZeroMemory( pNewPhotoList , sizeof( Photo_List ) );
			// data input
			pNewPhotoList->id = id;
#if 0
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
	pWork->photo_list->pNext = MALLOC( sizeof( Photo_List ) );
	if ( pWork->photo_list->pNext == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->photo_list->pNext , sizeof( Photo_List ) );
	pWork->photo_list->pNext->pPrev = pWork->photo_list;
	pWork->photo_list = pWork->photo_list->pNext;
	pWork->photo_list->pNext = NULL;
	// data input
	pWork->photo_list->id = id;
#if 0
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
	return ( NULL );
}

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
	while ( pWork->photo_list != NULL ) {
		pTmp = pWork->photo_list;
		pWork->photo_list = pTmp->pNext;
		FREE( pTmp , sizeof( Photo_List ) );
	}
}

static void PhotoListFreeElement( Photo_List *pPhotoList)
{
   if( pPhotoList )
   {
      if( pPhotoList->pPrev != NULL )
      {
         pPhotoList->pPrev->pNext = pPhotoList->pNext;
      }
      FREE( pPhotoList , sizeof( Photo_List ) );
   }
}

static void PhotoListAddNewFile( Work *pWork )
{
#if 0
#  ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	char buf[] = "NEW DATA\0";
#  else
	char buf[] = "NEW FILE\0";
#  endif
#else
   char *buf = BP_Area_EU() ? "NEW DATA\0" : "NEW FILE\0";
#endif

	pWork->photo_list->pNext = MALLOC( sizeof( Photo_List ) );
	if ( pWork->photo_list->pNext == NULL ){
		printf( " memory malloc failed\n" );
		return;
	}
	GV_ZeroMemory( pWork->photo_list->pNext , sizeof( Photo_List ) );
	pWork->photo_list->pNext->pPrev = pWork->photo_list;
	pWork->photo_list = pWork->photo_list->pNext;
	pWork->photo_list->pNext = NULL;
	// data input
	strncpy( pWork->photo_list->name , buf , 9 );
	pWork->photo_list->id = pWork->save_id;
	pWork->photo_list->r = 128;
	pWork->photo_list->g = 128;
	pWork->photo_list->b = 128;
	pWork->photo_list->a = 0;

	pWork->photo.files++;
}

static void SearchSaveFile(Work *pWork)
{
	int i=0;

	if( MCManGetCardType( pWork->port ) == MCMAN_CARDTYPE_PS2_UNFORMATTED ){
		pWork->save_id = 0;
		return;
	}

	while( i < FILE_MAX ){
		if( !MCManCheckExactFileFlag( i ) ){
			pWork->save_id = i;
			return;
		}
		i++;
	}
	pWork->save_id = NO_SAVEFILE;
}

static int ListAutoInsert( Work *pWork ) // バックグラウンド動作
{
	if ( pWork->file_counter < 0 ){
		return ( 0 );
	}
	if ( MCManCheckingID() >= pWork->file_counter ){
		return ( 0 );
	}
//	GetPhotoPageInfoStart( pWork->port , pWork->file_counter , 10 ); // 何回もやっていいのか微妙
	switch ( MCManCheckExactFileFlag( pWork->file_counter ) ){
	case 1 : // 正常
		if ( PhotoListDataInput( pWork , pWork->file_counter , 1 ) < 0 ){
			pWork->file_counter--;
			return ( -1 );
		}
		pWork->num_counter++;
		break;
	case -1 : // 異常
		if ( PhotoListDataInput( pWork , pWork->file_counter , -1 ) < 0 ){
			pWork->file_counter--;
			return ( -1 );
		}
		pWork->num_counter++;
		break;
	case 0 :
		break;
	}
	pWork->file_counter--;

	return ( 0 );
}

#if 0
static int ListNewFile( Work *pWork ) // New File を入れる
{

	// 最後にNEWファイルを入れる
	if( ( MCManGetFreeSize( pWork->port ) >= PHOTO_MC_MAXSIZE ) || 
		( MCManGetCardType( pWork->port ) == MCMAN_CARDTYPE_PS2_UNFORMATTED ) ){
		SearchSaveFile( pWork );
		PhotoListAddNewFile( pWork );
		pWork->flag |= SK_FREE_SPACE; // 保存領域あり
	}
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres; // 先頭に戻す
	if ( pWork->photo_list->pNext != NULL ){
		pWork->photo_list = pWork->photo_list->pNext;
		if ( pWork->photo_list->pNext == NULL ){
			printf( "なにもないっすよ\n");
		}
	} else {
		return ( -1 );
	}
	return ( 0 );
}
#endif

static int ListAll( Work *pWork ) // idを全て登録
{
	int count;
	int i;

	count = 0;
	if( MCManGetCardType( pWork->port ) != MCMAN_CARDTYPE_PS2_UNFORMATTED ){
		for ( i = 0 ; i <= pWork->photo.id_max ; i ++ ){
			if( MCManCheckExactFileFlag( i ) != 0 ){
				if ( PhotoListAdd( pWork , i ) < 0 ){
					return ( -1 );
				}
				count++;
			}
		}
	} else {
		pWork->file_counter = -1;
	}
	pWork->photo.files = count;
	// 最後にNEWファイルを入れる
   // DKR: Looks like every version of the game shipped with this bug and no one noticed.
   //if( pWork->photo.files != pWork->photo.id_max 
   if( pWork->photo.files < PHOTO_FILE_MAX )
   {
      if( ( MCManGetFreeSize( pWork->port ) >= PHOTO_MC_MAXSIZE ) || 
         ( MCManGetCardType( pWork->port ) == MCMAN_CARDTYPE_PS2_UNFORMATTED ) ){
            SearchSaveFile( pWork );
            if ( pWork->save_id != NO_SAVEFILE ){
               PhotoListAddNewFile( pWork );
               pWork->flag |= SK_FREE_SPACE; // 保存領域あり
            }
      }
   }
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres; // 先頭に戻す
	if ( pWork->photo_list->pNext != NULL ){
		pWork->photo_list = pWork->photo_list->pNext;
		if ( pWork->photo_list->pNext == NULL ){
			printf( "なにもないっすよ\n");
		}
	} else {
		return ( -1 );
	}
	return ( 0 );
}

static void OverWriteAlphaControl( Work *pWork )
{
   int i;
//#ifdef AREA_EU_BP_IGNORE()	// #ifndef PAL
//#endif

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
			   center_width = SUBTITLE_FONT_WIDTH * 1;
			   no_width = SUBTITLE_FONT_WIDTH * 3;
			   pWork->jpn_font.data = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( 5 , 12 ));
			   MENU_CreateTextTexture( pWork->jpn_font.work , DEFAULT_OVER_WRITE_SELECT_POS_X , SUBTITLE_FONT_HEIGHT * 5 + 1 ,
									   yes_width , SUBTITLE_FONT_HEIGHT , 0 , 0 , 0 , pWork->jpn_font.data );
			   MENU_CreateTextTexture( pWork->jpn_font.work , DEFAULT_OVER_WRITE_SELECT_POS_X + yes_width , SUBTITLE_FONT_HEIGHT * 5 + 1 ,
									   center_width , SUBTITLE_FONT_HEIGHT , 0 , 0 , 0 , string );
			   pWork->jpn_font.data = BP_GCL_LOOKUP_NEW_FONT_STRING(GM_GetResource( 5 , 13 ));
			   MENU_CreateTextTexture( pWork->jpn_font.work , DEFAULT_OVER_WRITE_SELECT_POS_X + yes_width + center_width , SUBTITLE_FONT_HEIGHT * 5 + 1 ,
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
			   v1 = SUBTITLE_FONT_HEIGHT * 5 + 1;
			   u2 = DEFAULT_OVER_WRITE_SELECT_POS_X + poly_width;
			   v2 = SUBTITLE_FONT_HEIGHT * 6;
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
}

static void FormatMemory( Work *pWork )
{
	int i;

	pWork->format_flag = 0;
	// font初期化
	for ( i = 0 ; i < 2 ; i ++ ){
		SK_PrintfNormal( NULL , pWork->over_write.hold_obj[ i ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE );
	}
#if defined(BP_VITA)
   if(pWork->mDeleteFlag == kDelete_Clicked)
   {
      DeleteStringCleate(pWork);
   }
	else
#endif
   if( MCManGetCardType( pWork->port ) == MCMAN_CARDTYPE_PS2_UNFORMATTED ){
		pWork->format_flag = 1;
		pWork->jpn_font.position = 4;
		StringCleate( pWork );
	} else {
		if ( ( pWork->flag & SK_FREE_SPACE ) &&  
			 ( pWork->position + ( ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ) == ( pWork->photo.files - 1 ) ) ){ // new file
			pWork->jpn_font.position = 9;
		} else { // over write
			pWork->jpn_font.position = 10;
		}
		StringCleate( pWork );
	}
}

static inline void FormatSave( Work *pWork )
{
	if ( pWork->format_flag == 1 ){
		pWork->jpn_font.position = 6;
		StringCleate( pWork );
	}
}

static void OverWriteData( Work *pWork )
{
	Photo_List *photo_list;
	u_char info[ 16 ];
	int            id;
	int             i;

	photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( photo_list->pNext != NULL ){
		photo_list = photo_list->pNext;
	}
	photo_list = PhotoListSearch( photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	for ( i = 0 ; i < pWork->position ; i ++ ){
		if ( photo_list == NULL ){
			break;
		}
		photo_list = PhotoListForward( photo_list );
	}
	if ( photo_list == NULL ){
		id = pWork->save_id;
	} else {
		id = photo_list->id;
	}
	// info
	info[ 0 ] = 'P';
	info[ 1 ] = 'H';
	info[ 2 ] = 'O';
	info[ 3 ] = 'T';
	info[ 4 ] = 'O';
	info[ 5 ] = 0x80;
	info[ 6 ] = 0x80;
	info[ 7 ] = 0x80;
	info[ 8 ] = 0x80;
	info[ 9 ] = 0x80;
	info[ 10 ] = 0x80;
	info[ 11 ] = 0x80;
	info[ 12 ] = 0x80; // 終端
	info[ R_POSITION ] = 0x80; // r 
	info[ G_POSITION ] = 0x80; // g
	info[ B_POSITION ] = 0x80; // b

	// number 刻む
	sprintf( &info[ 5 ] , " %03d" , id + 1 );
	pWork->over_write.a[ 0 ] = 128;
	pWork->over_write.a[ 1 ] = 64;

	SK_PrintfNormal2( info , pWork->font[ pWork->position ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );

	SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );

	// メモりーカードに書きこみ

	{
		u_char title[0x10];

		memcpy(title,info,12);
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

	if ( SavePhotoStart( pWork->port , id , pWork->jpeg_data , JPEG_SIZE , NULL , 0 , info ) != 0 ){
		pWork->flag |= SK_SAVE_START;
	} else {
#ifdef DEBUG_MODE
		printf("Try Agein\n");
#endif
	}

	pWork->flag |= SK_NOW_SAVING;

	FormatSave( pWork );
// save_data , pWork->save_data_size ,
//	ChangePhotoInfoStart( pWork->port , pWork->photo_list->id , tmp );
}

static void SaveCheck( Work *pWork )
{
	if ( pWork->flag & SK_NOW_SAVING ){
		if( !MCManFormatting() ){
			pWork->format_flag = 0;
		}
		if ( pWork->format_flag == 0 ){
			pWork->jpn_font.position = kBP_PLS_SavingShort;
			StringCleate( pWork );
			pWork->flag &= ~SK_NOW_SAVING;
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
//		printf("chenge l\n");
		photo_list = PhotoListSearch( photo_list , ( pWork->page.num ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	} else if ( pWork->flag & SK_PAGE_CHANGE_R ){
//		printf("chenge r\n");
		photo_list = PhotoListSearch( photo_list , ( pWork->page.num - 2 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	} else {
//		printf("chenge normal\n");
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
			// 更新
			if ( pWork->main_position + DISP_PHOTO_MAX > i && i >= pWork->main_position ){
				SK_PrintfChengColor2( pWork->font[ i - pWork->main_position ] , SK_R , SK_G , SK_B , photo_list->a , ASCCI_MAX );
			}
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
	tmp = 0;
	if ( pWork->flag & SK_FONT_OUT_START ){
		for ( i = 0 ; i < pWork->page.now_page_max ; i ++ ){
			photo_list->a = 0;
			tmp++;
			// 更新
			if ( pWork->main_position + DISP_PHOTO_MAX > i && i >= pWork->main_position ){
				SK_PrintfChengColor2( pWork->font[ i - pWork->main_position ] , SK_R , SK_G , SK_B , photo_list->a , ASCCI_MAX );
			}
			// next
			if ( photo_list->pNext == NULL ){
			  	i++;
				break;
			}
			photo_list = PhotoListForward( photo_list );
		}
		// jude
		if ( tmp >= i ){
			pWork->flag |= SK_FONT_OUT_OK;
			pWork->flag &= ~SK_FONT_OUT_START;
		}
	}
}

#if 0
static void PageChange2( Work *pWork ) // 10ライン飛ばし
{
	pWork->flag |= SK_FONT_FADE_IN_START;
	pWork->flag &= ~SK_FONT_FADE_OUT_OK;
	pWork->flag &= ~SK_FONT_FADE_OUT_START;
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( pWork->photo_list->pNext != NULL ){
		pWork->photo_list = pWork->photo_list->pNext;
	}
	pWork->photo_list = PhotoListSearch( pWork->photo_list , pWork->main_position );
	if ( pWork->photo_list == NULL ){
		return;
	}
	pWork->flag |= SK_NORMAL_SCREEN_OK;
	pWork->flag &= ~SK_PAGE_CHANGE_L;
	pWork->flag &= ~SK_PAGE_CHANGE_R;
}
#endif

static void PageChange( Work *pWork ) // ページの切替
{
	if ( pWork->flag & SK_LIST_UP ){
		pWork->position = DISP_PHOTO_MAX - 1; // 初期位置へ
		pWork->flag &= ~SK_LIST_UP;
	} else {
		pWork->position = 0; // 初期位置へ
	}
#if 0
	// dot
	pWork->flag |= SK_DOT_FADE_IN_START;
	pWork->flag &= ~SK_DOT_FADE_OUT_START;
	pWork->flag &= ~SK_DOT_FADE_IN_OK;
#endif
	// flag
	pWork->flag |= SK_FONT_FADE_IN_START;
	pWork->flag &= ~SK_FONT_FADE_OUT_OK;
	pWork->flag &= ~SK_FONT_FADE_OUT_START;
	pWork->flag &= ~SK_FIRST_FADE_OK;
	pWork->photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( pWork->photo_list->pNext != NULL ){
		pWork->photo_list = pWork->photo_list->pNext;
	}
	if ( pWork->flag & SK_PAGE_CHANGE_L ){
		pWork->photo_list = PhotoListSearch( pWork->photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	} else if ( pWork->flag & SK_PAGE_CHANGE_R ){
		pWork->photo_list = PhotoListSearch( pWork->photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	}
	if ( pWork->photo_list == NULL ){
		return;
	}
	pWork->flag |= SK_NORMAL_SCREEN_OK;
	pWork->flag &= ~SK_PAGE_CHANGE_L;
	pWork->flag &= ~SK_PAGE_CHANGE_R;
}

static void PadControlNormal( Work *pWork )
{
	int page_min;
	int tmp;

	// そのページのphoto数を出す
	if ( pWork->page.num < pWork->page.max_num || 
		 ( ( pWork->photo.files != 0 ) && ( pWork->photo.files % DISP_PHOTO_MAX ) == 0 ) ){
		pWork->page.now_page_max = DISP_PHOTO_MAX;
	} else {
		pWork->page.now_page_max = pWork->photo.files % DISP_PHOTO_MAX;
	}
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) || !( pWork->flag & SK_FONT_FADE_IN_OK ) ) {
		return;
	}
	page_min = ( pWork->page.max_num ) - ( ( pWork->num_counter ) / 10 );
	if ( pWork->flag & SK_FREE_SPACE ){
		tmp = 1;
	} else {
		tmp = 0;
	}
	if ( ( page_min < 0 ) || ( pWork->num_counter == pWork->photo.files - tmp ) ){
		page_min = 0;
	}
	// main
	if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status & PAD_U ){
		pWork->pad_status = 0;
		if ( pWork->position > 0 ){
			pWork->position_sign--;
			pWork->position--; // <-- リストを進める数の事 いるのか？
			// dot
			pWork->dot_hokan.end_y = pWork->font[ pWork->position ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 5;
			pWork->dot_hokan.flag = 0;
			// flag
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			// sound
			SE_SEL();
			return;
		} else if ( pWork->page.num - 1 > page_min ){
			pWork->page.num--;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ DISP_PHOTO_MAX - 1 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
#if 0
			pWork->flag |= SK_DOT_FADE_OUT_START;
			pWork->flag &= ~SK_DOT_FADE_IN_START;
			pWork->flag &= ~SK_DOT_FADE_OUT_OK;
#endif
			// flag
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_L;
			pWork->flag |= SK_LIST_UP;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_R;
			// sound
			SE_TYPE();
			return;
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status & PAD_D ){
		pWork->pad_status = 0;
		if ( pWork->position < pWork->page.now_page_max - 1 ){
			pWork->position_sign++;
			pWork->position++; // <-- リストを進める数の事
			// dot
			pWork->dot_hokan.end_y = pWork->font[ pWork->position ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 5;
			pWork->dot_hokan.flag = 0;
			// flag
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			// sound
			SE_SEL();
			return;
		} else if ( pWork->page.num < pWork->page.max_num ){
			pWork->page.num++;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ 0 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
#if 0
			pWork->flag |= SK_DOT_FADE_OUT_START;
			pWork->flag &= ~SK_DOT_FADE_IN_START;
			pWork->flag &= ~SK_DOT_FADE_OUT_OK;
#endif
			// flag
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_R;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_L;
			// sound
			SE_TYPE();
			return;
		}		  
	}
	// page 切替
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_L1 || pWork->pad_status & PAD_L1 ) ||
		 ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status & PAD_L ) ){
		pWork->pad_status = 0;
		if ( pWork->page.num - 1 > page_min ){
			pWork->page.num--;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ 0 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
#if 0
			pWork->flag |= SK_DOT_FADE_OUT_START;
			pWork->flag &= ~SK_DOT_FADE_IN_START;
			pWork->flag &= ~SK_DOT_FADE_OUT_OK;
#endif
			// flag
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_L;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_R;
			// sound
			SE_TYPE();
			return;
		}
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_R1 || pWork->pad_status & PAD_R1 ) || 
		 ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status & PAD_R ) ){
		pWork->pad_status = 0;
		if ( pWork->page.num < pWork->page.max_num ){
			pWork->page.num++;
			// dot
			pWork->dot_hokan.end_y = pWork->font[ 0 ][ 0 ]->sprite.pos.y;
			pWork->dot_hokan.count = 1;
			pWork->dot_hokan.flag = 0;
			SPR_HIDE( pWork->dot.dot_curs );
#if 0
			pWork->flag |= SK_DOT_FADE_OUT_START;
			pWork->flag &= ~SK_DOT_FADE_IN_START;
			pWork->flag &= ~SK_DOT_FADE_OUT_OK;
#endif
			// flag
			pWork->flag |= SK_FONT_FADE_OUT_START;
			pWork->flag |= SK_PAGE_CHANGE_R;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_PAGE_CHANGE_L;
			// sound
			SE_TYPE();
			return;
		} 
	}
	// etc
#if defined(BP_VITA) && !defined(BP_DISABLE_DELETE_FILES)
   if ( ( GV_PadDataDirect[ 0 ].press & PAD_Y )
         && ( pWork->position + ( ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ) != ( pWork->photo.files - 1 ) ) )
   { // normal screen -> pre normal screen
      pWork->pre_position = 0;
      // flag control
      pWork->mDeleteFlag = kDelete_Clicked;
      pWork->flag |= SK_OVER_WRITE_MODE;
      FormatMemory( pWork ); // format が必要かどうか

      pWork->flag &= ~SK_NORMAL_SCREEN_OK;
      SK_AllShow( pWork->over_write.obj , OVER_WRITE_YES_NO );
      SK_AllShow( pWork->over_write.hold_obj[ 0 ] , OVER_WRITE );
      SK_AllShow( pWork->over_write.hold_obj[ 1 ] , OVER_WRITE );
      // kano
      SK_AllHide( pWork->kano_work , STR_TITLE_LEN );
      // sound
      SE_OK();
      return;
   }
#endif
   if ( GV_PadDataDirect[ 0 ].press & SELECT ){ // normal screen -> pre normal screen
		pWork->pre_position = 0;
		// flag control
		pWork->flag |= SK_OVER_WRITE_MODE;
		FormatMemory( pWork ); // format が必要かどうか

		pWork->flag &= ~SK_NORMAL_SCREEN_OK;
		SK_AllShow( pWork->over_write.obj , OVER_WRITE_YES_NO );
		SK_AllShow( pWork->over_write.hold_obj[ 0 ] , OVER_WRITE );
		SK_AllShow( pWork->over_write.hold_obj[ 1 ] , OVER_WRITE );
		// kano
	    SK_AllHide( pWork->kano_work , STR_TITLE_LEN );
		// sound
		SE_OK();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){ // back exit proc call
		pWork->flag |= SK_FONT_FADE_OUT_START | SK_PAGE_FADE_OUT_START | SK_DOT_FADE_OUT_START;
		pWork->flag |= SK_CHANCEL;
		pWork->flag &= ~SK_FONT_FADE_IN_START;
		pWork->flag &= ~SK_FONT_FADE_OUT_OK;
		pWork->flag &= ~SK_DOT_FADE_IN_START;
		pWork->flag &= ~SK_DOT_FADE_OUT_OK;
		pWork->flag &= ~SK_PAGE_FADE_IN_START;
		pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
		pWork->cancel_flag = 0x1;
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
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_U | PAD_D | PAD_L1 | PAD_R1 | PAD_R | PAD_L ) ) ){
		pWork->hold_time = 0;
		pWork->pad_status = 0;
	}
}

static void PadControlOverWrite( Work *pWork )
{
	if ( GV_PadDataDirect[ 0 ].press & PAD_L ){
		if ( pWork->over_write.position == 1 ){
			pWork->over_write.position--;
		}
		SE_SEL();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_R ){
		if ( pWork->over_write.position == 0 ){
			pWork->over_write.position++;
		}
		SE_SEL();
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		switch( pWork->over_write.position ){
		case 0 : // ok
#if defined(BP_VITA)
         if(pWork->mDeleteFlag == kDelete_Clicked)
         {
            pWork->mDeleteFlag = kDelete_Selected;
            DeleteStringCleate(pWork);
         }
         else
#endif
         {
            OverWriteData( pWork );
            pWork->flag |= SK_SAVE_WAIT;
         }
			SE_OK();
			return;
		case 1 : // cancel
#if defined(BP_VITA)         
         if(pWork->mDeleteFlag == kDelete_Clicked)
         {
            pWork->mDeleteFlag = kDelete_None;
         }
#endif
         pWork->over_write.position = 1;
			pWork->flag |= SK_NORMAL_SCREEN_OK;
			pWork->flag &= ~SK_OVER_WRITE_MODE;
			SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
			SK_AllHide( pWork->over_write.hold_obj[ 0 ] , OVER_WRITE );
			SK_AllHide( pWork->over_write.hold_obj[ 1 ] , OVER_WRITE );
			SK_AllShow( pWork->kano_work , STR_TITLE_LEN );
			// 領域の初期化
			MENU_ClearTextTexture( pWork->jpn_font.work );
			SE_CANCEL();
			break;
		}
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
#if defined(BP_VITA)
      if(pWork->mDeleteFlag == kDelete_Clicked)
      {
         pWork->mDeleteFlag = kDelete_None;
      }
#endif
      pWork->over_write.position = 1;
		pWork->flag |= SK_NORMAL_SCREEN_OK;
		pWork->flag &= ~SK_OVER_WRITE_MODE;
		pWork->over_write.a[ 0 ] = 64;
		pWork->over_write.a[ 1 ] = 128;
		SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
		SK_AllHide( pWork->over_write.hold_obj[ 0 ] , OVER_WRITE );
		SK_AllHide( pWork->over_write.hold_obj[ 1 ] , OVER_WRITE );
		SK_AllShow( pWork->kano_work , STR_TITLE_LEN );
		// 領域の初期化
		MENU_ClearTextTexture( pWork->jpn_font.work );
		// sound
		SE_CANCEL();

		return;
	}
}

static inline void FormatOrSaveComp( Work *pWork )
{
	pWork->jpn_font.position = 7;
	StringCleate( pWork );
	GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_SAVEOK01 ); // SE
}

static inline void FormatOrSaveFaild( Work *pWork )
{
	Photo_List *photo_list;
	int            id;
	int             i;

	// damaged 
	photo_list = ( Photo_List * )pWork->photo_top_addres;
	if ( photo_list->pNext != NULL ){
		photo_list = photo_list->pNext;
	}
	photo_list = PhotoListSearch( photo_list , ( pWork->page.num - 1 ) * DISP_PHOTO_MAX ); // pWork->position の 値まで移動
	for ( i = 0 ; i < pWork->position ; i ++ ){
		if ( photo_list == NULL ){
			break;
		}
		photo_list = PhotoListForward( photo_list );
	}
	if ( photo_list == NULL ){
		id = pWork->save_id;
	} else {
		id = photo_list->id;
	}
	// 振り分けformat_err or normal_err
	if ( pWork->format_flag == 1 ){
		pWork->jpn_font.position = 11;
		StringCleate( pWork );
	} else {
		pWork->jpn_font.position = 8;
		StringCleate( pWork );
		// damaged file 
		MCManSetDamagedFlag( id );
	}
}

static void PadAct( Work *pWork ) // パッドコントロール
{
	// main
	if ( pWork->flag & SK_NORMAL_SCREEN_OK ){
		PadControlNormal( pWork );
	}
}

static int NormalAct( Work *pWork )
{
   STransfarringError deleteError;
   SET_TRANSFARRING_ERROR_INVALID( deleteError );

	FontFade( pWork );
	PageFade( pWork );
	FontUpDate( pWork );
	PageUpDate( pWork ); // num -> ascci そして 入力
	DotUpdate( pWork );
	DotFade( pWork );

#if defined(BP_VITA) && !defined(BP_DISABLE_DELETE_FILES)
   DeleteUpdate( pWork );

   //TODO:
   switch(pWork->mDeleteFlag)
   {
   case kDelete_Selected:
      SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
      SK_AllHide( pWork->over_write.hold_obj[ 0 ] , OVER_WRITE );
      SK_AllHide( pWork->over_write.hold_obj[ 1 ] , OVER_WRITE );

      pWork->mDeleteFlag = kDelete_Looping;

      Transfarring_PostWork_DeleteNonGameFile(pWork->mMemoryCardActor, kNGST_Photo, pWork->position );

      return 1;
   case kDelete_Looping:
      StringDisp(pWork);

      deleteError = Transfarring_ProcessWork();
      if(deleteError.mHighLevelError != kTE_Processing )
      {
         if (deleteError.mHighLevelError == kTE_Success)
         {
            pWork->mDeleteFlag = kDelete_Finished;
         }
         else
         {
            // TODO: Handle error
            pWork->mDeleteFlag = kDelete_Finished;
         }
      }
      return 1;
   case kDelete_Finished:
      //PhotoListFreeElement(PhotoListSearch(pWork->photo_list, pWork->position));
      pWork->over_write.position = 1;
      pWork->flag |= SK_NORMAL_SCREEN_OK;
      pWork->flag &= ~SK_OVER_WRITE_MODE;
      MENU_ClearTextTexture( pWork->jpn_font.work );

      pWork->mDeleteFlag = kDelete_None;
      return 1;
   case kDelete_Error:
      return 1;
   }
#endif
  
   if ( pWork->flag & SK_END ){
#ifdef DEBUG_MODE
		printf("%d\n" , pWork->end_time );
#endif
		if ( pWork->end_time <= 0 )
      {
			// 親に送信
#if defined(BP_PS3)
         if ( pWork->cancel_flag )     // check and see if there was a save error
            GV_CallParentSignalFunc( pWork , SIGNAL_PORTSEL_AGAIN , 0 );         // BP JG - force the photo save to run again as this time the save failed.
         else
            GV_CallParentSignalFunc( pWork , SK_DESTROY , pWork->cancel_flag );
#else
         GV_CallParentSignalFunc( pWork , SK_DESTROY , pWork->cancel_flag );
#endif
			GV_DestroyActor( pWork );
		}
		pWork->end_time--;
		return ( -1 );
	}
	if ( pWork->flag & SK_CHANCEL ){ // 終了
		PageFade( pWork );
		if ( ( pWork->flag & SK_FONT_FADE_OUT_OK ) && ( pWork->flag & SK_PAGE_FADE_OUT_OK ) && ( pWork->flag & SK_DOT_FADE_OUT_OK ) ){
			// 親に送信
			GV_CallParentSignalFunc( pWork , SK_DESTROY , pWork->cancel_flag );
			GV_DestroyActor( pWork );
		}
		return ( -1 );
	}
	// over_write 操作
	if ( pWork->flag & SK_OVER_WRITE_MODE ){
		if ( ( pWork->flag & SK_SAVE_WAIT ) ){
#ifdef DEBUG_MODE
			printf("check --------> %d\n" , MCManGetResult() );
#endif
			if ( !( pWork->flag & SK_SAVE_START ) ){
				OverWriteData( pWork );
			} else {
				switch ( MCManGetResult() ){
				case 1 : // save wait
					FormatOrSaveComp( pWork );
					pWork->end_time = END_TIME;
					pWork->flag |= SK_FONT_FADE_OUT_START | SK_PAGE_FADE_OUT_START | SK_DOT_FADE_OUT_START;
					pWork->flag |= SK_CHANCEL;
					pWork->flag &= ~SK_FONT_FADE_IN_START;
					pWork->flag &= ~SK_FONT_FADE_OUT_OK;
					pWork->flag &= ~SK_DOT_FADE_IN_START;
					pWork->flag &= ~SK_DOT_FADE_OUT_OK;
					pWork->flag &= ~SK_PAGE_FADE_IN_START;
					pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
					pWork->cancel_flag = 0;
					pWork->flag |= SK_END;
					return ( 1 );
				case -1 : // faild
					FormatOrSaveFaild( pWork );
					pWork->end_time = END_TIME;
					pWork->flag |= SK_FONT_FADE_OUT_START | SK_PAGE_FADE_OUT_START | SK_DOT_FADE_OUT_START;
					pWork->flag |= SK_CHANCEL;
					pWork->flag &= ~SK_FONT_FADE_IN_START;
					pWork->flag &= ~SK_FONT_FADE_OUT_OK;
					pWork->flag &= ~SK_DOT_FADE_IN_START;
					pWork->flag &= ~SK_DOT_FADE_OUT_OK;
					pWork->flag &= ~SK_PAGE_FADE_IN_START;
					pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
					pWork->cancel_flag = 0x1;
					pWork->flag |= SK_END;
					return ( 1 );
				} 
				SaveCheck( pWork );
			}
		} else {
			PadControlOverWrite( pWork );
			OverWriteAlphaControl( pWork );
		}
		return ( 1 );
	}
	if ( ( pWork->flag & SK_FONT_FADE_OUT_OK ) && ( pWork->flag & SK_PAGE_CHANGE_L || pWork->flag & SK_PAGE_CHANGE_R ) ){
		PageChange( pWork );
		return ( 1 );
	}
	PadAct( pWork );
	return ( 1 );
}

static void Act( Work *pWork )
{
	int type;

	StringDisp( pWork );
	if( MCManChecked() ){
		type = MCManIsMCChanged( pWork->port );
#ifdef DEBUG_MODE
		printf("type = %d\n" , type );
#endif
		if( type != 0 ){
			if ( ( pWork->flag & SK_OVER_WRITE_MODE ) && ( pWork->flag & SK_SAVE_WAIT ) ){
				FormatOrSaveFaild( pWork );
				pWork->end_time = END_TIME;
				pWork->flag |= SK_FONT_FADE_OUT_START | SK_PAGE_FADE_OUT_START | SK_DOT_FADE_OUT_START;
				pWork->flag |= SK_CHANCEL;
				pWork->flag &= ~SK_FONT_FADE_IN_START;
				pWork->flag &= ~SK_FONT_FADE_OUT_OK;
				pWork->flag &= ~SK_DOT_FADE_IN_START;
				pWork->flag &= ~SK_DOT_FADE_OUT_OK;
				pWork->flag &= ~SK_PAGE_FADE_IN_START;
				pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
				pWork->cancel_flag = 0x1;
				pWork->flag |= SK_END;
			}  else {
				pWork->flag |= SK_FONT_FADE_OUT_START | SK_PAGE_FADE_OUT_START | SK_DOT_FADE_OUT_START;
				pWork->flag |= SK_CHANCEL;
				pWork->flag &= ~SK_FONT_FADE_IN_START;
				pWork->flag &= ~SK_FONT_FADE_OUT_OK;
				pWork->flag &= ~SK_DOT_FADE_IN_START;
				pWork->flag &= ~SK_DOT_FADE_OUT_OK;
				pWork->flag &= ~SK_PAGE_FADE_IN_START;
				pWork->flag &= ~SK_PAGE_FADE_OUT_OK;
				pWork->cancel_flag = 0x1;
			}
			// 親に送信
			if ( !( ( pWork->file_counter < pWork->photo.files - 20 ) || ( pWork->file_counter <= 0 ) ) ){
				GV_CallParentSignalFunc( pWork , SK_DESTROY , pWork->cancel_flag );
				GV_DestroyActor( pWork );
			}
			return;
		}
	} else if ( ( pWork->file_counter < pWork->photo.files - 20 ) || ( pWork->file_counter <= 0 ) ){
		if ( pWork->act( pWork ) < 0 ){
			return;
		}
	}
	ListAutoInsert( pWork );
}

static	void	Die( Work *pWork )
{
	int i , j;

#ifdef DEBUG_MODE
	printf("skoba free memsize first = %ld\n" , codecMemGetSize() );
#endif
	PhotoListFree( pWork ); // リストの解放
	codecFree( pWork->jpeg_data );
	codecFree( pWork->font_string );
	// memory
	for(i = 0 ; i < PAGE_MAX ; i++ ){
		SPR_Destroy_2D_Object( pWork->page.page[ i ] );
	}
	for(i = 0 ; i < PAGE_FONT_MAX ; i++ ){
		SPR_Destroy_2D_Object( pWork->page.page_now[ i ] );
	}
	for(j = 0 ; j < DISP_PHOTO_MAX ; j++ ){
		for(i = 0 ; i < ASCCI_MAX ; i++ ){
			SPR_Destroy_2D_Object( pWork->font[ j ][ i ] );
		}
	}
	for(j = 0 ; j < 2 ; j++ ){
		for(i = 0 ; i < OVER_WRITE ; i++ ){
			SPR_Destroy_2D_Object( pWork->over_write.hold_obj[ j ][ i ] );
		}
	}
	for(i = 0 ; i < OVER_WRITE_YES_NO ; i++ ){
		SPR_Destroy_2D_Object( pWork->over_write.obj[ i ] );
	}
#ifdef DEBUG_MODE
	printf("skoba free memsize = %ld\n" , codecMemGetSize() );
#endif
#if 0
	SK_FreeMemory( pWork->page.page , PAGE_MAX );
	SK_FreeMemory( pWork->page.page_now , PAGE_FONT_MAX );
	for ( i = 0 ; i < DISP_PHOTO_MAX ; i ++ ){
		SK_FreeMemory( pWork->font[ i ] , ASCCI_MAX );
	}
	for ( i = 0 ; i < 2 ; i ++ ){
		SK_FreeMemory( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
	}
	SK_FreeMemory( pWork->over_write.obj , OVER_WRITE_YES_NO );
#endif
#if 0 // 使いまわしをしているので書換えなくてもよい
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
#endif
}

static void JpegCopy( Work *pWork ) // Jpeg to Memory and ZeroPading
{
	int i;

   memcpy( pWork->jpeg_data, &pWork->save_data_size, sizeof(int) );
	memcpy( pWork->jpeg_data + sizeof(int), pWork->save_data, pWork->save_data_size );

	i = pWork->save_data_size + sizeof(int) + 1;
	while( i <= JPEG_SIZE ){ // ZeroPading
		pWork->jpeg_data[ i++ ] = 0;
	}	
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	float     ftmp;

#ifdef DEBUG_MODE
	printf("skoba memsize = %ld\n" , codecMemGetSize() );
#endif
	ftmp = ( DRAW_HEIGHT / 384.0f ); 

	work->act = NormalAct;
	work->over_write.position = 1;
	work->pad_status = 0;
	work->pad_check = 0;
	work->position = 0;
	work->position_sign = 0;
	work->main_position = 0;
	work->cancel_flag = 0;
	work->file_counter = MCManMaxId();
	work->num_counter = 0;
	work->flag = 0;
	work->flag |= ( SK_PAGE_FADE_IN_START | SK_NORMAL_SCREEN_OK | SK_FONT_FADE_IN_START | SK_DOT_FADE_IN_START );
	work->over_write.a[ 0 ] = 64;
	work->over_write.a[ 1 ] = 128;
	// jpeg
#ifdef DEBUG_MODE
	printf("skoba get memsize jpeg first= %ld\n" , codecMemGetSize() );
#endif
   // TODO: PHOTE_SAVE get rid of this fix file size logic and use varaible size files
   assert( work->save_data_size + sizeof(int) < JPEG_SIZE );// BP_NOTE: +sizeof(int) is for filesize of JPEG, this will change once PHOTE_SAVE above is done
	work->jpeg_data = codecMalloc( JPEG_SIZE );
	if ( work->jpeg_data == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( work->jpeg_data , JPEG_SIZE );
#ifdef DEBUG_MODE
	printf("skoba get memsize jpeg first= %ld\n" , codecMemGetSize() );
#endif
	JpegCopy( work );
	// font
#ifdef DEBUG_MODE
	printf("skoba get memsize font first= %ld\n" , codecMemGetSize() );
#endif
	work->font_string = codecMalloc( sizeof( u_char ) * 128 );
	if ( work->font_string == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( work->font_string , sizeof( u_char ) * 128 );
#ifdef DEBUG_MODE
	printf("skoba get memsize font first= %ld\n" , codecMemGetSize() );
#endif
	// dot
	work->dot.r = SK_R * 2;
	work->dot.g = SK_G * 2;
	work->dot.b = SK_B * 2;
	work->dot.a = 0;
	work->dot_hokan.flag = 0;
	work->dot_hokan.end_y = 0;
	work->dot_hokan.count = 0;
	// page
	work->page.max_num = 1;
	work->page.num = 1;
	work->page.a = 128;
	// photo list
	if ( PhotoListInitialize( work ) < 0 ){
		return ( -1 );
	}
	// mc
	work->photo.id_max = MCManMaxId();
	work->photo.files = MCManExactNFiles();
	if ( ListAll( work ) < 0 ){ // default で 一番先頭のものになる
		return ( -1 );
	}
	work->page.max_num = ( int )( ( float )( work->photo.files + 9 ) / ( float )DISP_PHOTO_MAX ); // 最大ページ数を出す
	work->page.max_num = work->page.max_num == 0 ? 1 : work->page.max_num;
	work->page.now_page_max = work->photo.files;
	// そのページのphoto数を出す
	work->page.num = work->page.max_num;
	if ( work->page.num < work->page.max_num || 
		 ( ( work->photo.files != 0 ) && ( work->photo.files % DISP_PHOTO_MAX ) == 0 ) ){
		work->page.now_page_max = DISP_PHOTO_MAX;
		work->position = work->page.now_page_max - 1;
	} else {
		work->page.now_page_max = work->photo.files % DISP_PHOTO_MAX;
		work->position = work->page.now_page_max - 1;
	}

#ifdef DEBUG_MODE
	printf("skoba get memsize = %ld\n" , codecMemGetSize() );
#endif
	SprInit( work ); // 文字の初期化

#ifdef DEBUG_MODE
	printf("skoba get memsize spr plus = %ld\n" , codecMemGetSize() );
#endif

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewPhotoSaveMode( SPR_OBJ **kano_work , int port_sel , void *save_data , int save_data_size , int handle , void *font_work , MCMAN_WORK* mcman )
{
	Work		*work ;

	if((work=(Work *)codecMalloc( sizeof( Work ) )) == NULL){
		return NULL;
	}

	/* アクター起動 */
	GV_ZeroMemory(work,sizeof(Work));
	GV_SetActorFreeFunc( work , codecFree);
	GV_SetActorClass( work , GV_CLASS_CHARA);
	GV_SetActorKillLevel( work , GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority( GV_ACTOR_MANAGER , work , 0xFF); /* ACT_MODE <--- user/mode/codec/codecmem.h */

#if 0
	work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 0xff ) ;
#endif
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->port = port_sel;
	work->kano_work = kano_work;	
	work->save_data = save_data;
	work->save_data_size = save_data_size;
	work->jpn_font.work = font_work;
	work->jpn_font.data = NULL;
   work->mMemoryCardActor = mcman;
	if ( handle < 0 ){ // handle の 確保に失敗している場合はさよなら
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	work->handle_2d = handle;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

