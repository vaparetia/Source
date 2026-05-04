//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dogtag2.c
   ドッグタグモード
   
   2001/06/14	S.Kobayashi
   $Id: dogtag2.c,v 1.2 2002/12/05 18:42:01 takaki Exp $
*/
#ifdef PSX2
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
#include "libfs.h"

#include "mode/menu/xtextscn.h"

#define		LAYOUT		  (2448726)
#define 	STR_FONT      (10045111)
#define     STR_DOT_CURS  (2531169)
#define     STR_DEFAULT   (566267)
#define     STR_ROOT      (2770484)
#define 	FONT_WIDTH ( 16.0f )
#define 	FONT_HEIGHT ( 14.0f )
#define     FONT_HEL_WIDTH  (14) // helvetika
#define     FONT_HEL_HEIGHT (18)
#define     FONT_SPACE_HEIGHT ( 8.0f )// DRAW_HEIGHT / 384.0f )
#define     DOT_POS_X ( 16.0f )
#define     DOT_POS_Y ( 10.0f )
#define 	DEFAULT_DOT_POS_X ( DEFAULT_POS_X - DOT_POS_X )
#define 	DEFAULT_DOT_POS_Y ( DEFAULT_POS_Y + DOT_POS_Y )
#define 	STR_WIDTH  (18) // strcode ではなく文字の長さ
#define 	STR_HEIGHT (14) // strcode ではなく文字の長さ
#define     STR_HEL_WIDTH  (16) // helvetika
#define     STR_HEL_HEIGHT (20)
#define     RGB_DEFAULT_POS_X ( 36.0f )
#define     RGB_DEFAULT_POS_Y ( 246.0f )
#define     RGB_NUM_DEFAULT_POS_X ( 90.0f )
#define     RGB_SPACE_HEIGHT ( 22.0f )
#define     RGB_POS_Y ( 10.0f )
#define 	STR_TEST_BG (7054968)
#define     STR_SHOW_TOP (2948071)
#define     STR_SELECT_PLANT (4642234)
#define     STR_SELECT_TANKER (3221986)
#define     STR_SHOW_VIEWER_TANKER (4759204)
#define     STR_SHOW_VIEWER_PLANT (5738848)
#define     STR_CURS_NULL (6035479)
#define     STR_DIF (105862)
#define		STR_LEFT_ARROW (12196035)
#define		STR_RIGHT_ARROW (16197969)
#define		STR_TOP_ARROW (5149435)
#define		STR_BOTTOM_ARROW (10654859)
#define     STR_HIDE_VIEWER_TANKER (7437900)
#define     STR_HIDE_VIEWER_PLANT (10016861)
#define		STR_HILITE_NULL (12077417)
#define DEFAULT_ID_POS_X (56)
#define	DEFAULT_POS_X ( DEFAULT_ID_POS_X + 48.0f )
#if 0
#  ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
#    if 1 //BP
//#ifndef _WINDOWS
#      define	DEFAULT_POS_Y ( 98.0f * 448.0f / 384.0f ) // 間違ってるよー by koba4
#    else
#      define	DEFAULT_POS_Y ( 98.0f * DRAW_HEIGHT / 384.0f ) // ちと変更
#    endif
#  else
#    define	DEFAULT_POS_Y ( 98.0f * DRAW_HEIGHT / 384.0f ) // 間違ってるよー by koba4
#  endif
#else
#  define	DEFAULT_POS_Y ( 98.0f * ( BP_Area_EU() ? 448.0f : DRAW_HEIGHT ) / 384.0f ) // 間違ってるよー by koba4
#endif
#define DEFAULT_PERCENT_POS_X (426)
#define DEFAULT_PERCENT_POS_Y (72)
#define DEFAULT_LOCATION_POS_X (58)
#define DEFAULT_LOCATION_POS_Y (92)
#define DEFAULT_BARTHDAY_POS_X ( DEFAULT_POS_X + 291 )
#define DEFAULT_BLOOD_POS_X ( DEFAULT_BARTHDAY_POS_X + 57 )
#define DEFAULT_REG_POS_X ( DEFAULT_BLOOD_POS_X + 42 )
#define TEST_MAX_DATA (1000)
#define SK_R (80)
#define SK_G (90) 
#define SK_B (85) 
#define SK_A (52)
#define IN_FADE_SPEED ( DIRECT_TICK( 6 ) )
#define OUT_FADE_SPEED ( DIRECT_TICK( 12 ) )

#define STR_BLD (9251795)

#define DEFAULT_PAGE_POS_X (62)
#define DEFAULT_PAGE_POS_Y (52)
#define PAGE_FONT_MAX ( 5 ) // 00/00

#define DISP_DOGTAG_MAX (11) // 一画面に表示する写真項目数の最大値
#define JPEG_DECODE_START_TIME (20) // 10 frame before start to the JPEG decodeing

#define MODE_MAX ( 2 ) // tanker , plant
#define PAGE_MAX ( 5 ) // very_easy , easy , normal , hard , exterm

#define ID_MAX       (3)
#define BLOOD_MAX    (3)
#define PERCENT_MAX  (4)
#define REGION_MAX   (3)
#define YEAR_MAX     (4)
#define MANTH_MAX    (2)
#define DAY_MAX      (2)
#define LOCATION_MAX (35)
#define ASCCI_MAX    (32) 

#define LOCATION_FONT_WIDTH  (16)
#define LOCATION_FONT_HEIGHT (18)
#define SYS_FONT_WIDTH ( 24 )
#define SYS_FONT_HEIGHT ( 24 )

#define LOCATION_DMA_WAIT ( DIRECT_TICK(10) )
#define SK_LAYOUT_RERESE (0x10)

// Japanease or Usa or Pal
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
#if 0

#define SELECT PAD_OK
#define CANCEL PAD_CANCEL

#else 

#define SELECT PAD_OK
#define CANCEL PAD_CANCEL

#endif 
#define SK_REPEAT (2)

#ifdef ENGLISH
   #if 0
//   #ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		#define SK_DEFAULT_PAGE (2)
	#else
		#define SK_DEFAULT_PAGE (2)
	#endif
#else
#define SK_DEFAULT_PAGE (1)
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

enum {
	SK_PAGE_VERY_EASY = 0,
	SK_PAGE_EASY ,
	SK_PAGE_NORMAL ,
	SK_PAGE_HARD ,
	SK_PAGE_EXTERM ,
	SK_PAGE_SPACIAL ,
};

// enum -> #define  chaned by T.Morita 2002.05.23
#define	SK_NORMAL                      I64(0x0000000000000001)
#define	SK_TAG_LOAD_OK                 I64(0x0000000000000002)
#define	SK_TAG_SAVE_OK                 I64(0x0000000000000004)
#define	SK_FONT_FADE_IN_START          I64(0x0000000000000008)
#define	SK_FONT_FADE_IN_OK             I64(0x0000000000000010)
#define	SK_FONT_FADE_OUT_START         I64(0x0000000000000020)
#define	SK_FONT_FADE_OUT_OK            I64(0x0000000000000040)
#define	SK_RIGHT_ARROW_FADE_OUT_START  I64(0x0000000000000080)
#define	SK_LEFT_ARROW_FADE_OUT_START   I64(0x0000000000000100)
#define	SK_TOP_ARROW_FADE_OUT_START    I64(0x0000000000000200)
#define	SK_BOTTOM_ARROW_FADE_OUT_START I64(0x0000000000000400)
#define	SK_CHANCEL                     I64(0x0000000000000800)
#define	SK_PAGE_CHANGE_L               I64(0x0000000000001000)
#define	SK_PAGE_CHANGE_R               I64(0x0000000000002000) 
#define	SK_SELECT_SCREEN_START         I64(0x0000000000004000)
#define	SK_SELECT_SCREEN_OK            I64(0x0000000000008000)
#define	SK_NORMAL_SCREEN_START         I64(0x0000000000010000)
#define	SK_NORMAL_SCREEN_OK            I64(0x0000000000020000)
#define	SK_ACTION_START                I64(0x0000000000040000)
#define	SK_ACTION_OK                   I64(0x0000000000080000)
#define	SK_SELECT                      I64(0x0000000000100000)
#define	SK_SELECT_TANKER               I64(0x0000000000200000)
#define	SK_FONT_ALL_BLACK              I64(0x0000000000400000)
#define	SK_LOCATION_FADE_IN            I64(0x0000000000800000)
#define	SK_LOCATION_FADE_OUT           I64(0x0000000001000000)
#define	SK_HILIGHT_FADE_IN_START       I64(0x0000000002000000)
#define	SK_HILIGHT_FADE_OUT_START      I64(0x0000000004000000)
#define	SK_DIF_FADE_OUT_START          I64(0x0000000008000000)
#define	SK_MOVE_OK                     I64(0x0000000010000000)
#define	SK_DOT_FADE_OUT_START          I64(0x0000000020000000)
#define	SK_END                         I64(0x0000000040000000)


enum {
	SK_DESTROY = 0x1 ,
	SK_NEW_ACT  = 0x2 ,
};

enum {
	A = 0,
	B ,
	O ,
	AB ,
	BLOOD_NONE ,
	BLOOD_GACKT ,
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

enum {
	DOGTAG_LOCATION_OK = 0x1,
	DOGTAG_LOCATION_NG = 0x2,
};

enum {
  SK_TANKER = 0,
  SK_PLANT ,
};

typedef struct _photo_list {
	struct _photo_list *pPrev;
	struct _photo_list *pNext;
	u_char name[ ASCCI_MAX ]; // my name is ...
	u_char region[ REGION_MAX ]; // 国
//	u_char location[ LOCATION_MAX ]; // 取得場所
	int location;
	int manth;       // 月
	int day;           // 日
	int blood; // 血液型
	int id; // 番号
	int flag;
	u_char r , g , b , a; // 色
} Dogtag_List;

typedef struct {
	SPR_OBJ *page_now[ PAGE_FONT_MAX ]; // 不定
	int      num;     // 現在のページ
	int      bufnum;  // 進んだ値の保存
	int      now_page_max[ MODE_MAX ][ PAGE_MAX ]; // 現在のページの枚数
	int      get_count[ MODE_MAX ][ PAGE_MAX ]; // このページでの取得数
	u_char   a; // α
} Page;

typedef struct {
	SPR_OBJ             *curs_null; // empty
	float               plus;
	int                 count;
	u_char r , g , b , a;
} Dot;

typedef struct {
//	void *data; // date
	MCScrWork mcscr; // メモりーカード
	int   files; // ファイル総数
} Memory;

typedef struct {
	SPR_OBJ *curs;
	SPR_OBJ *tanker;
	SPR_OBJ *plant;
} Select;

typedef struct {
	void     *work[ 2 ];
	int      buffer;
} Location;	

typedef struct {
	int     count;
	int     end_y;
	u_long64  flag;
} Hokan_Program;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle;
	SPR_OBJ             *font[ DISP_DOGTAG_MAX ][ ASCCI_MAX ];
	Location            location;
	SPR_OBJ             *region[ DISP_DOGTAG_MAX ][ REGION_MAX ];
	SPR_OBJ             *id[ DISP_DOGTAG_MAX ][ ID_MAX ];
	SPR_OBJ             *blood[ DISP_DOGTAG_MAX ][ BLOOD_MAX ];
	SPR_OBJ             *percent[ PERCENT_MAX ];
#if 0
	SPR_OBJ             *year[ DISP_DOGTAG_MAX ][ YEAR_MAX ];
	SPR_OBJ             *manth[ DISP_DOGTAG_MAX ][ MANTH_MAX ];
	SPR_OBJ             *day[ DISP_DOGTAG_MAX ][ DAY_MAX ];
#endif
	SPR_OBJ             *barthday[ DISP_DOGTAG_MAX ][ MANTH_MAX + DAY_MAX ];
	SPR_OBJ             *hilight_null;
	SPR_OBJ             *left_arrow;
	SPR_OBJ             *right_arrow;
	SPR_OBJ             *top_arrow;
	SPR_OBJ             *bottom_arrow;
	SPR_OBJ             *dif;
	Dot                 dot; // ドット
	Hokan_Program       dot_hokan;
	Select              select; // セレクト画面用
	Dogtag_List         *dogtag_list; // 項目リスト
	Page                page;
	Memory              memory;
	u_long64              *dogtag_tanker_very_easy_top_addres;
	u_long64              *dogtag_tanker_easy_top_addres;
	u_long64              *dogtag_tanker_normal_top_addres;
	u_long64              *dogtag_tanker_hard_top_addres;
	u_long64              *dogtag_tanker_exterm_top_addres;
	u_long64              *dogtag_plant_very_easy_top_addres;
	u_long64              *dogtag_plant_easy_top_addres;
	u_long64              *dogtag_plant_normal_top_addres;
	u_long64              *dogtag_plant_hard_top_addres;
	u_long64              *dogtag_plant_exterm_top_addres;
	int                 main_position; // ページ中の一番上の場所を示す
	int                 alpha;  // 全体α
	int                 location_alpha;  // location用alpha
	int                 location_wait_frame; // DMA Initialize Wait
	int                 position; // ページ中のどこか？
	int                 position_sign; // 何方に進んだか
	int                 action;
	int                 action_num;
	int                 base_node_u;  // フォント用ベースUV値
	int                 base_node_v;  // フォント用ベースUV値
	int                 base_u;      // フォント用ベースUV値
	int                 base_v;      // フォント用ベースUV値
	u_long64              flag;        // 全体管理フラグ
	int                 pad_status;  // パッド用
	int                 pad_check;   // パッド用
	int                 hold_time;   // パッドオートモード用
	int                 proc_prev;   // キャンセル時前のproc
	int                 proc_next;   // 終了時次のproc
	int                 time;        // 時間
	int                 name;        // 名前
	int                 select_mode; // 何方のモード tanker / plant
	float               fade_plus;   // フェードの増加分
	int                 select_position; // tanker or plant
	void  ( *act )( struct _work * );

   char const * mPrevStageString;
#ifdef PSX2
	char 	spr_flag;
	SPR_OBJ *spr2001_2;
#endif
} Work ;

// プロトタイプ
static void PadAct( Work * );
static void FontFade( Work *pWork );
static void FontUpdate( Work *pWork );
static void SelectAct( Work *pWork );
static void NormalAct( Work *pWork );
static inline Dogtag_List *PageChange( Work *pWork );
static inline Dogtag_List *DogtagLinker( Work *pWork );
static inline Dogtag_List *DogtagListForward( Dogtag_List * ); // リストを安全に進める
static inline Dogtag_List *DogtagListSearch( Dogtag_List * , int ); // リストを安全に進める
static inline Dogtag_List *DogtagListBack( Dogtag_List * ); // リストを安全に進める
static inline void NameUpdate( Work *pWork , Dogtag_List * , int point );
static inline void NumToReg( Work *pWork , Dogtag_List * , int point );
static inline void NumToId( Work *pWork , Dogtag_List * , int point );
static inline void NumToBlood( Work *pWork , Dogtag_List *pDogtag_list , int point ); // データを文字に変換
static inline void NumToLocation( Work *pWork , Dogtag_List *pDogtag_list , int point ); // データを文字に変換
static inline void NumToBarthday( Work *pWork , Dogtag_List *pDogtag_list , int point ); // データを文字に変換
static inline void PercentDisp( Work * );
static inline void HilightUpdate( Work * );
static inline void DotUpdate( Work *pWork ); // update
static int DogtagListAdd( Work *pWork ); // id sort plus
// extern my function
extern void SK_Printf( int ascci , SPR_OBJ *pObj , int u , int v , int , int );
extern void SK_Printf2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num ); // helvetica ver
extern void SK_PrintfChengColor( SPR_OBJ *pObj , u_char r , u_char g , u_char b , u_char a );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // helvetics ver
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_AllShow( SPR_OBJ ** , int );

// etc
extern int SIG_GetTotalDogTagNum( void );
extern int SIG_CheckDogTagFlag2( int num );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_ClearPartTextTexture( void *work_ptr, int start_line, int height );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void *GetLocalResource( int ref_id, int offset );
extern void *NewTextScreenControlForTitle( void );

/*----------------------------------------------------------------*/
static int SprInit( Work *pWork ) // default を a
{
	SPR_OBJ		*spr;
	int        i , j;

	if ( pWork->flag & SK_NORMAL ){
		return ( 0 );
	}
	// difの取得
	spr = L2D_GetObject( pWork->handle, STR_DIF ) ;
	if ( spr == NULL ){
		SK_Err("dif\0");
		return -1 ;
	}
	pWork->dif = spr;
	// Left_arrowの取得
	spr = L2D_GetObject( pWork->handle, STR_LEFT_ARROW ) ;
	if ( spr == NULL ){
		SK_Err("left\0");
		return -1 ;
	}
	pWork->left_arrow = spr;
	// right_arrowの取得
	spr = L2D_GetObject( pWork->handle, STR_RIGHT_ARROW ) ;
	if ( spr == NULL ){
		SK_Err("right\0");
		return -1 ;
	}
	pWork->right_arrow = spr;
	// hilight_nullの取得
	spr = L2D_GetObject( pWork->handle, STR_HILITE_NULL ) ;
	if ( spr == NULL ){
		SK_Err("hilight_null\0");
		return -1 ;
	}
	pWork->hilight_null = spr;
	pWork->hilight_null->head.child->sprite.col.a = 0;
	SPR_SHOW( pWork->hilight_null );
	// top_arrowの取得
	spr = L2D_GetObject( pWork->handle, STR_TOP_ARROW ) ;
	if ( spr == NULL ){
		SK_Err("up\0");
		return -1 ;
	}
	pWork->top_arrow = spr;
	// bottom_arrowの取得
	spr = L2D_GetObject( pWork->handle, STR_BOTTOM_ARROW ) ;
	if ( spr == NULL ){
		SK_Err("down\0");
		return -1 ;
	}
	pWork->bottom_arrow = spr;
	// name文字の取得
	spr = L2D_GetObject( pWork->handle, STR_FONT ) ;
	if ( spr == NULL ){
		SK_Err("font\0");
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
	spr->sprite.dw = FONT_HEL_WIDTH;
	spr->sprite.dh = FONT_HEL_HEIGHT;
	spr->sprite.pos.x = DEFAULT_POS_X;
	// main process
	for ( i = 0 ; i < DISP_DOGTAG_MAX ; i ++ ){
		spr->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		pWork->font[ i ][ 0 ] = SPR_DuplicateTree( spr );
		for ( j = 1 ; j < ASCCI_MAX ; j++ ){
			pWork->font[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->font[ i ][ j ]->sprite.pos.x = 0;
			pWork->font[ i ][ j ]->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		}
		SK_Printf2( NULL , pWork->font[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , ASCCI_MAX ); // helvetica ver
	}
	SPR_HIDE( spr );
#if 0
	// location文字の取得
	spr = L2D_GetObject( pWork->handle, STR_FONT ) ;
	if ( spr == NULL ){
		SK_Err("location\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_HEL_WIDTH;
	spr->sprite.dh = FONT_HEL_HEIGHT;
	spr->sprite.pos.x = DEFAULT_LOCATION_POS_X;
	spr->sprite.pos.y = DEFAULT_LOCATION_POS_Y;
	// main process
	pWork->location[ 0 ] = SPR_DuplicateTree( spr );
	for ( j = 1 ; j < LOCATION_MAX ; j++ ){
		pWork->location[ j ] = SPR_DuplicateTree( spr );
		pWork->location[ j ]->sprite.pos.x = 0;
		pWork->location[ j ]->sprite.pos.y = DEFAULT_LOCATION_POS_Y;
	}		
	SK_Printf2( NULL , pWork->location , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , LOCATION_MAX ); // helvetica ver
#endif
	// id文字の取得
	spr = L2D_GetObject( pWork->handle, STR_FONT ) ;
	if ( spr == NULL ){
		SK_Err("id\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_HEL_WIDTH;
	spr->sprite.dh = FONT_HEL_HEIGHT;
	spr->sprite.pos.x = DEFAULT_ID_POS_X;
	// main process
	for ( i = 0 ; i < DISP_DOGTAG_MAX ; i ++ ){
		spr->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		pWork->id[ i ][ 0 ] = SPR_DuplicateTree( spr );
		for ( j = 1 ; j < ID_MAX ; j++ ){
			pWork->id[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->id[ i ][ j ]->sprite.pos.x = 0;
			pWork->id[ i ][ j ]->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		}
		SK_Printf2( NULL , pWork->id[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , ID_MAX ); // helvetica ver
	}
	SPR_HIDE( spr );
	// reg文字の取得
	spr = L2D_GetObject( pWork->handle, STR_FONT ) ;
	if ( spr == NULL ){
		SK_Err("reg\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_HEL_WIDTH;
	spr->sprite.dh = FONT_HEL_HEIGHT;
	spr->sprite.pos.x = DEFAULT_REG_POS_X;
	// main process
	for ( i = 0 ; i < DISP_DOGTAG_MAX ; i ++ ){
		spr->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		pWork->region[ i ][ 0 ] = SPR_DuplicateTree( spr );
		for ( j = 1 ; j < REGION_MAX ; j++ ){
			pWork->region[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->region[ i ][ j ]->sprite.pos.x = 0;
			pWork->region[ i ][ j ]->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		}
		SK_Printf2( NULL , pWork->region[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , REGION_MAX ); // helvetica ver
	}
	SPR_HIDE( spr );
	// blood文字の取得
	spr = L2D_GetObject( pWork->handle, STR_FONT ) ;
	if ( spr == NULL ){
		SK_Err("reg\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_HEL_WIDTH;
	spr->sprite.dh = FONT_HEL_HEIGHT;
	spr->sprite.pos.x = DEFAULT_BLOOD_POS_X;
	// main process
	for ( i = 0 ; i < DISP_DOGTAG_MAX ; i ++ ){
		spr->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		pWork->blood[ i ][ 0 ] = SPR_DuplicateTree( spr );
		for ( j = 1 ; j < BLOOD_MAX ; j++ ){
			pWork->blood[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->blood[ i ][ j ]->sprite.pos.x = 0;
			pWork->blood[ i ][ j ]->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		}
		SK_Printf2( NULL , pWork->blood[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX ); // helvetica ver
	}
	SPR_HIDE( spr );
	// percent
	spr = L2D_GetObject( pWork->handle, ( STR_FONT + 1 ) ) ;
	if ( spr == NULL ){
		SK_Err("percent\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_PERCENT_POS_X;
	spr->sprite.pos.y = DEFAULT_PERCENT_POS_Y;
#if 0 //BP_PS2 def PSX2	
	pWork->base_node_u = spr->sprite.head.tex.u; // テクスチャの位置取得
	pWork->base_node_v = spr->sprite.head.tex.v;
#else
	pWork->base_node_u = spr->sprite.head.tex.u * 16.0f; // テクスチャの位置取得
	pWork->base_node_v = spr->sprite.head.tex.v * 16.0f;
#endif	
	// main process
	  /* 基点のスプライトが出たままになっていたため修正 2002.07.14 Yano */
	for ( j = 0 ; j < PERCENT_MAX-1 ; j++ ){
		pWork->percent[ j ] = SPR_DuplicateTree( spr );
		pWork->percent[ j ]->sprite.pos.y = DEFAULT_PERCENT_POS_Y;
	}
	pWork->percent[ PERCENT_MAX-1 ] = spr;
	

	SK_PrintfNormal( NULL , pWork->percent , pWork->base_node_u , pWork->base_node_v , STR_WIDTH , STR_HEIGHT , PERCENT_MAX ); // helvetica ver

	// barthday文字の取得
	spr = L2D_GetObject( pWork->handle, STR_FONT ) ;
	if ( spr == NULL ){
		SK_Err("reg\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_HEL_WIDTH;
	spr->sprite.dh = FONT_HEL_HEIGHT;
	spr->sprite.pos.x = DEFAULT_BARTHDAY_POS_X;
	// main process
	for ( i = 0 ; i < DISP_DOGTAG_MAX ; i ++ ){
		spr->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		pWork->barthday[ i ][ 0 ] = SPR_DuplicateTree( spr );
		for ( j = 1 ; j < MANTH_MAX + DAY_MAX ; j++ ){
			pWork->barthday[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->barthday[ i ][ j ]->sprite.pos.x = 0;
			pWork->barthday[ i ][ j ]->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		}
		SK_Printf2( NULL , pWork->barthday[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , MANTH_MAX + DAY_MAX ); // helvetica ver
	}
	SPR_HIDE( spr );
#if 0
	// day文字の取得
	spr = L2D_GetObject( pWork->handle, STR_FONT ) ;
	if ( spr == NULL ){
		SK_Err("reg\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.dw = FONT_HEL_WIDTH;
	spr->sprite.dh = FONT_HEL_HEIGHT;
	spr->sprite.pos.x = DEFAULT_BARTHDAY_POS_X + 16;
	// main process
	for ( i = 0 ; i < DISP_DOGTAG_MAX ; i ++ ){
		spr->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		pWork->day[ i ][ 0 ] = SPR_DuplicateTree( spr );
		for ( j = 1 ; j < DAY_MAX ; j++ ){
			pWork->day[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->day[ i ][ j ]->sprite.pos.x = 0;
			pWork->day[ i ][ j ]->sprite.pos.y = DEFAULT_POS_Y + ( FONT_HEL_HEIGHT * i );
		}
		SK_Printf2( NULL , pWork->day[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , DAY_MAX ); // helvetica ver
	}
	SPR_HIDE( spr );
#endif
	// flag set
	pWork->flag |= SK_NORMAL;

	return( 0 );
}

#define DOGTAG_TANKER_VERY_EASY_MAX (100)
#define DOGTAG_TANKER_EASY_MAX      (100)
#define DOGTAG_TANKER_NORMAL_MAX    (100)
#define DOGTAG_TANKER_HARD_MAX      (100)
#define DOGTAG_TANKER_EXTERM_MAX    (40)
#define DOGTAG_PLANT_VERY_EASY_MAX (100)
#define DOGTAG_PLANT_EASY_MAX      (100)
#define DOGTAG_PLANT_NORMAL_MAX    (100)
#define DOGTAG_PLANT_HARD_MAX      (100)
#define DOGTAG_PLANT_EXTERM_MAX    (40)

#if 0
static void FontFadein( Work *pWork )
{
	Dogtag_List *dogtag_list;
	int  i;
	int  alpha;
	u_long64   flag;
	int      tmp;

	// check
	if ( pWork->flag & SK_FONT_FADE_IN_START ){
		// forward process 
		flag = SK_NORMAL;
		tmp = 0;
		dogtag_list = DogtagLinker( pWork );
		// main process 
		for ( i = 0 ; i < pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] ; i ++ ){
			alpha = ( int )dogtag_list->a;
			alpha += pWork->fade_plus;
			if ( alpha > 128 ){
				dogtag_list->a = 128;
				tmp++;
			} else {
				dogtag_list->a = ( u_char )alpha;
			}
			dogtag_list = DogtagListForward( dogtag_list );
			if ( dogtag_list->pNext == NULL && flag == SK_NORMAL ){
				flag = SK_CHANCEL; // 特殊
			}
			if ( flag == SK_END ){
				break;
			}		
		}
		if ( tmp >= i ){
		  pWork->flag |= SK_FONT_FADE_IN_OK;
		  pWork->flag &= ~SK_FONT_FADE_IN_START;
		}
	}
}

static void FontAlphaControl( Work *pWork ) // 未
{
	Dogtag_List *dogtag_list;
	u_long64 flag;
	int  alpha;
	int  i;

	dogtag_list = DogtagLinker( pWork );
	flag = SK_NORMAL;
	for ( i = 0 ; i < pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] ; i ++ ){
		alpha = ( int )dogtag_list->a;
		if ( flag == SK_CHANCEL ){ // 特殊
			flag = SK_END;
		}
		if ( pWork->position == i ){
			// 128付近 にする
			alpha += ( 128 - alpha ) / 10.0f;
		} else {
			// 64付近 にする
			alpha += ( 64 - alpha ) / 4.0f;
		}
		dogtag_list = DogtagListForward( dogtag_list );
		if ( dogtag_list->pNext == NULL && flag == SK_NORMAL ){
			flag = SK_CHANCEL; // 特殊
		}
		if ( flag == SK_END ){
			break;
		}		
	}
}
#endif

static void FontUpdate( Work *pWork )
{
	Dogtag_List *dogtag_list;
	char flag;
	int  i;

	i = 0;
	flag = 0;
	dogtag_list = DogtagLinker( pWork );
	if ( ( dogtag_list = DogtagListSearch( dogtag_list , pWork->main_position ) ) != NULL ){
//	    printf("next = %p %d\n" , dogtag_list , pWork->main_position );
		for ( i = 0 ; i < DISP_DOGTAG_MAX ; i ++ ){
			// id からネームをもってくる！
//			if ( dogtag_list->flag != 0 ){ // 表示可能（取得済）
				NameUpdate( pWork , dogtag_list , i );
				NumToReg( pWork , dogtag_list , i );
				NumToId( pWork , dogtag_list , i );
				NumToBlood( pWork , dogtag_list , i );
				NumToBarthday( pWork , dogtag_list , i );
				if ( i + pWork->main_position == pWork->position ){
					NumToLocation( pWork , dogtag_list , i );
					flag = 1;
				}
//			}
			if ( dogtag_list->pNext == NULL ){
				i++;
				break;
			} else {
				dogtag_list = DogtagListForward( dogtag_list );
			}
		}
	}
	for ( ; i < DISP_DOGTAG_MAX ; i ++ ){  // 空白にする
		SK_Printf2( NULL , pWork->font[ i ] , pWork->base_u  , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , ASCCI_MAX );
		SK_Printf2( NULL , pWork->region[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , REGION_MAX );
		SK_Printf2( NULL , pWork->id[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , ID_MAX );
		SK_Printf2( NULL , pWork->blood[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
		SK_Printf2( NULL , pWork->barthday[ i ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , MANTH_MAX + DAY_MAX );
	}
#if 0
	if ( flag != 1 ){
		SK_Printf2( NULL , pWork->location , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , LOCATION_MAX );
	}
#endif
	// αの制御
//	FontAlphaControl( pWork );
}

// リストを初期化する
static int DogtagListInitialize( Work *pWork ) // root の 取得
{
	// tanker
	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_tanker_very_easy_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_tanker_easy_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_tanker_normal_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_tanker_hard_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_tanker_exterm_top_addres = ( u_long64 * )pWork->dogtag_list;

	// plant
	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_plant_very_easy_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_plant_easy_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_plant_normal_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_plant_hard_top_addres = ( u_long64 * )pWork->dogtag_list;

	pWork->dogtag_list = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pWork->dogtag_list == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pWork->dogtag_list , sizeof( Dogtag_List ) );
	pWork->dogtag_list->pPrev = NULL;
	pWork->dogtag_list->pNext = NULL;
	pWork->dogtag_plant_exterm_top_addres = ( u_long64 * )pWork->dogtag_list;

	// default tanker_very_easy
	pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_very_easy_top_addres;

	return ( 0 );
}

static inline Dogtag_List *DogtagListPageSet( Work *pWork , int select_mode , int page )
{
	Dogtag_List *pDogtag_list;

	switch ( page ){
	case SK_PAGE_VERY_EASY :
		if ( select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_very_easy_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_very_easy_top_addres;
		}
		break;
	case SK_PAGE_EASY :
		if ( select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_easy_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_easy_top_addres;
		}
		break;
	case SK_PAGE_NORMAL :
		if ( select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_normal_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_normal_top_addres;
		}
		break;
	case SK_PAGE_HARD :
		if ( select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_hard_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_hard_top_addres;
		}
		break;
	case SK_PAGE_EXTERM :
		if ( select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_exterm_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_exterm_top_addres;
		}
		break;
	default : 
#ifdef DEBUG
		printf("無効なレベル設定です %d\n" , pWork->page.num );
		printf("end guy");
#endif
		pDogtag_list = NULL;
		break;
	}
	return ( pDogtag_list );
}

#define DOGTAG_TANKER_VERY_EASY_MAX (100)
#define DOGTAG_TANKER_EASY_MAX      (100)
#define DOGTAG_TANKER_NORMAL_MAX    (100)
#define DOGTAG_TANKER_HARD_MAX      (100)
#define DOGTAG_TANKER_EXTERM_MAX    (40)
#define DOGTAG_PLANT_VERY_EASY_MAX (100)
#define DOGTAG_PLANT_EASY_MAX      (100)
#define DOGTAG_PLANT_NORMAL_MAX    (100)
#define DOGTAG_PLANT_HARD_MAX      (100)
#define DOGTAG_PLANT_EXTERM_MAX    (40)

// データは全て resource から 抽出
static inline int IdToPage( int id )
{
	int *level;

	if( GCL_GetOption('V') != NULL ){ // level
		level = ( int * )GCL_GetNextInt();
		level += id;
	} else {
		printf("データが不正です。\n");
		level = NULL;
	}
	return ( *level );
}

static inline int TankerOrPlant( int id )
{
	int *where;

	if( GCL_GetOption('w') != NULL ){ // level
		where = ( int * )GCL_GetNextInt();
		where += id;
		return ( *where );
	} else {
		printf("データが不正です。\n");
		where = NULL;
		ASSERT ( 0 );
	}
	return ( 0 );
}

#define STR_RESOURCE (12477798) /* "ドックタグデータ" */
//#define STR_DOGTAG_MAX (368)
static void DogtagListAddMng( Work *pWork )
{
	void *ptr;

	// set
	ptr = ( void * )GetLocalResource( STR_RESOURCE , 0 );
	GCL_SetArgTop( ptr );

	// リストに登録
	while( 1 ){
		if ( DogtagListAdd( pWork ) < 0 ) {
			break;
		}
	}
}

static int DogtagListAddSub( Work *pWork , int id , int location , u_char *name , int manth , int day ,
							 int blood , u_char *region , int where , int level , Dogtag_List *pDogtag_list ) // 実際に挿入
{
	int len;

	while ( pDogtag_list->pNext != NULL ){ // sort insert
		if ( pDogtag_list->pPrev == NULL ){ // root は 判定しません
			pDogtag_list = pDogtag_list->pNext;
			continue;
		}
		if ( pDogtag_list->id < id ){
			pDogtag_list = pDogtag_list->pNext;
			continue;
		}
		// insert
		{
			Dogtag_List *pNewDogtagList;

			// memory malloc
			pNewDogtagList = GV_Malloc( sizeof( Dogtag_List ) );
			if ( pNewDogtagList == NULL ){
				printf( " memory malloc failed\n" );
				return ( -1 );
			}
			GV_ZeroMemory( pNewDogtagList , sizeof( Dogtag_List ) );
			// data input
			// name
			if ( name != NULL ){
				len = strlen( name );
				strncpy( pNewDogtagList->name , name , len );
			} else { 
				pNewDogtagList->name[ 0 ] = ' ';
			}
			// region
			if ( region != NULL ){
				len = strlen( region );
				strncpy( pNewDogtagList->region   , region   , len );
			} else { 
				pNewDogtagList->region[ 0 ] = ' ';
			}
			// manth
			pNewDogtagList->manth = manth;
			// day
			pNewDogtagList->day = day;
			// blood
			pNewDogtagList->blood = blood;
			// id			// page num count up
			pNewDogtagList->id = pWork->page.now_page_max[ where ][ level ]++;
			// location
			pNewDogtagList->location = location;
			// etc
			pNewDogtagList->r = SK_R;
			pNewDogtagList->g = SK_G;
			pNewDogtagList->b = SK_B;
			pNewDogtagList->a = 0;
			pNewDogtagList->flag = 0;
			if ( ( pNewDogtagList->flag = SIG_CheckDogTagFlag2( id ) ) == 1 ){ // 表示してよいのか
				pWork->page.get_count[ where ][ level ]++; // パーセント出しに必要
			}
			// chain
			pNewDogtagList->pPrev = pDogtag_list->pPrev;
			pDogtag_list->pPrev->pNext = pNewDogtagList;
			pDogtag_list->pPrev = pNewDogtagList;
			pNewDogtagList->pNext = pDogtag_list;
		}
		return ( 0 );
	}
	// bottom plus
	pDogtag_list->pNext = GV_Malloc( sizeof( Dogtag_List ) );
	if ( pDogtag_list->pNext == NULL ){
		printf( " memory malloc failed\n" );
		return ( -1 );
	}
	GV_ZeroMemory( pDogtag_list->pNext , sizeof( Dogtag_List ) );
	pDogtag_list->pNext->pPrev = pDogtag_list;
	pDogtag_list = pDogtag_list->pNext;
	pDogtag_list->pNext = NULL;
	// data input
	// name
	if ( name != NULL ){
		len = strlen( name );
		strncpy( pDogtag_list->name , name , len );
	} else { 
		pDogtag_list->name[ 0 ] = ' ';
	}
	// region
	if ( region != NULL ){
		len = strlen( region );
		strncpy( pDogtag_list->region   , region   , len );
	} else { 
		pDogtag_list->region[ 0 ] = ' ';
	}
	// manth
	pDogtag_list->manth = manth;
	// day
	pDogtag_list->day = day;
	// blood
	pDogtag_list->blood = blood;
	// id	// page num count up
	pDogtag_list->id = pWork->page.now_page_max[ where ][ level ]++;
	// location
	pDogtag_list->location = location;
	// etc
	pDogtag_list->r = SK_R;
	pDogtag_list->g = SK_G;
	pDogtag_list->b = SK_B;
	pDogtag_list->a = 0;
	pDogtag_list->flag = 0;
	if ( ( pDogtag_list->flag = SIG_CheckDogTagFlag2( id ) ) == 1 ){ // 表示してよいのか
		pWork->page.get_count[ where ][ level ]++;
	}
	return ( 0 );
}

// リストに追加する
static int DogtagListAdd( Work *pWork ) // id sort plus
{
	Dogtag_List *pDogtag_list;
	u_char *name , *region;
	int id , manth , day , where , level , blood , location;

#ifdef PSX2
	/* PS2版だけ　ドックタグに2001,2002版の二通り持っている */
	int 	*config2;
	void 	*linkvar = GCL_GetLinkvarSaveAreaTop();
	config2 = (int *)( (int)(linkvar)+ (((u_int)&GM_Configuration2)-((u_int)linkvarbuf)) );
	
	if( *config2 & GM_CONFIG_DOGTAGS_2002 ){		
		/* 2002版 */
		id = GCL_GetNextInt();
		location = GCL_GetNextInt();
		/* 2001版の情報を空回しする */
		GCL_GetNextString();
		GCL_GetNextInt();
		GCL_GetNextInt();
		GCL_GetNextString();
		GCL_GetNextInt();
		GCL_GetNextInt();
		GCL_GetNextInt();		
		/* 2002版の情報を取得 */
		name = GCL_GetNextString();
		manth = GCL_GetNextInt();
		day = GCL_GetNextInt();
		region = GCL_GetNextString();
		where = GCL_GetNextInt();
		level = GCL_GetNextInt();
		blood = GCL_GetNextInt();

		/* l2dの2001表示を2002表示に書き換えるフラグ */
		pWork->spr_flag = 1;
		
	} else {
		/* 2001版 */
		id = GCL_GetNextInt();
		location = GCL_GetNextInt();
		name = GCL_GetNextString();
		manth = GCL_GetNextInt();
		day = GCL_GetNextInt();
		region = GCL_GetNextString();
		where = GCL_GetNextInt();
		level = GCL_GetNextInt();
		blood = GCL_GetNextInt();
		/* 2002版の情報を空回しする */
		GCL_GetNextString();
		GCL_GetNextInt();
		GCL_GetNextInt();
		GCL_GetNextString();
		GCL_GetNextInt();
		GCL_GetNextInt();
		GCL_GetNextInt();		

		/* l2dの2001表示を2002表示に書き換えるフラグ */
		pWork->spr_flag = 0;/* 書き換えない */
	}
	{
		/* 2001,2002表示スプライト  */
		SPR_OBJ *spr;
		spr = L2D_GetObject( pWork->handle, GV_StrCode("2001_2") );
		if( spr == NULL ){
			printf("2002 sprite err\n");
			return (-1);
		}
		pWork->spr2001_2 = spr;
	}
	
#if 0
printf("name %s\n",name);
printf("manth %d\n",manth);
printf("day %d\n",day);
printf("region %s\n",region);
printf("where %d\n",where);
printf("level %d\n",level);
printf("blood %d\n",blood);
#endif

#else
	id = GCL_GetNextInt();
	location = GCL_GetNextInt();
	name = GCL_GetNextString();
	manth = GCL_GetNextInt();
	day = GCL_GetNextInt();
	region = GCL_GetNextString();
	where = GCL_GetNextInt();
	level = GCL_GetNextInt();
	blood = GCL_GetNextInt();
#endif

	if ( id < 0 ){ // 特殊
#if 0
		for ( i = 0 ; i < 5 ; i ++ ){
			pDogtag_list = DogtagListPageSet( pWork , where , i );
			DogtagListAddSub( pWork , id , location , name , manth , day , blood , region , where , level , pDogtag_list );
		}
#endif
	} else {
		if ( ( pDogtag_list = DogtagListPageSet( pWork , where , level ) ) == NULL ){
		  return ( -1 );
		}
		DogtagListAddSub( pWork , id , location , name , manth , day , blood , region , where , level , pDogtag_list );
	}

	return ( 0 );
}

static inline Dogtag_List *DogtagListBack( Dogtag_List *pDogtag_list ) // リストを安全に戻す
{
	if ( pDogtag_list->pPrev != NULL ){
		return ( pDogtag_list->pPrev );
	}
	return ( pDogtag_list );
}

static inline Dogtag_List *DogtagListForward( Dogtag_List *pDogtag_list ) // リストを安全に進める
{
	if ( pDogtag_list->pNext != NULL ){
		return ( pDogtag_list->pNext );
	}
	return ( pDogtag_list );
}

#if 0
static void ListDebug( Work *pWork , int level ) // 内容を全表示
{
	int i;

	i = 0;
	switch( level ){
	case SK_PAGE_VERY_EASY :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_very_easy_top_addres;
		break;
	case SK_PAGE_EASY :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_easy_top_addres;
		break;
	case SK_PAGE_NORMAL :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_normal_top_addres;
		break;
	case SK_PAGE_HARD :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_hard_top_addres;
		break;
	case SK_PAGE_EXTERM :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_exterm_top_addres;
		break;
	}
	while ( pWork->dogtag_list->pNext != NULL ){
	  printf("%d : prev = %p my = %p next %p id = %d \n" , i , pWork->dogtag_list->pPrev , pWork->dogtag_list , pWork->dogtag_list->pNext , pWork->dogtag_list->id );
		pWork->dogtag_list = pWork->dogtag_list->pNext;
		i ++;
	}
	printf("%d : prev = %p my = %p next %p id = %d\n" , i , pWork->dogtag_list->pPrev , pWork->dogtag_list , pWork->dogtag_list->pNext , pWork->dogtag_list->id );
	printf("end\n" );
	// 初期化
	switch( level ){
	case SK_PAGE_VERY_EASY :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_very_easy_top_addres;
		break;
	case SK_PAGE_EASY :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_easy_top_addres;
		break;
	case SK_PAGE_NORMAL :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_normal_top_addres;
		break;
	case SK_PAGE_HARD :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_hard_top_addres;
		break;
	case SK_PAGE_EXTERM :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_exterm_top_addres;
		break;
	}
	if ( pWork->dogtag_list->pNext != NULL ){
		pWork->dogtag_list = pWork->dogtag_list->pNext;
	}
}
#endif

static inline Dogtag_List *DogtagListSearch( Dogtag_List *pDogtag_list , int position ) // pWork->position の 値まで移動
{
	Dogtag_List *tmp;
	int count;

	tmp = pDogtag_list;
	count = 0;
	// 先行チェック
	if ( tmp->pPrev == NULL && tmp->pNext == NULL ){
		return ( NULL );
	}
	// main
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
static void DogtagListFree( Work *pWork , int select_mode , int level )
{
	Dogtag_List *pTmp;

	pWork->dogtag_list = DogtagListPageSet( pWork , select_mode , level );

	while ( pWork->dogtag_list->pNext != NULL ) {
		pTmp = pWork->dogtag_list;
		pWork->dogtag_list = pTmp->pNext;
		GV_DelayedFree( pTmp );
	}
	GV_DelayedFree( pWork->dogtag_list );
}

#if 0
static int SK_PAGE_MAX[] = { DOGTAG_VERY_EASY_MAX , DOGTAG_EASY_MAX , DOGTAG_NORMAL_MAX , DOGTAG_HARD_MAX , DOGTAG_EXTERM_MAX };
static void LocationInit( Dogtag_List *pDogtag_list , int location_max );

static void ListAll( Work *pWork ) // idを全て登録
{
	static char test[9][ ASCCI_MAX ] = { "Satoru Kobayashi" , "Yumie Akatsuka" , "Yuko kitamura" , "Chiaki Hirano" , "Kie Suga" , "Junko Sato" ,
										 "Hisano Hashizume" , "Hisami Okamoto" , "Megumi Wtanabe" };
	int i , j;

	for ( i = 0 ; i < PAGE_MAX ; i ++ ){//pWork->memory.files ; i ++ ){
		for ( j = 0 ; j < SK_PAGE_MAX[ i ] ; j ++ ){//pWork->memory.files ; i ++ ){		
			DogtagListAdd2( pWork , irnd() % 1000 , test[ irnd() % 9 ] , i ); //pWork->memory.mcscr.indexes[ i ] );
		}
		switch( i ){
		case SK_PAGE_VERY_EASY :
			pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_very_easy_top_addres;
			break;
		case SK_PAGE_EASY :
			pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_easy_top_addres;
			break;
		case SK_PAGE_NORMAL :
			pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_normal_top_addres;
			break;
		case SK_PAGE_HARD :
			pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_hard_top_addres;
			break;
		case SK_PAGE_EXTERM :
			pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_exterm_top_addres;
			break;
		}
		pWork->dogtag_list = pWork->dogtag_list->pNext;

		LocationInit( pWork->dogtag_list , SK_PAGE_MAX[ i ] );

	}
	// 先頭に戻す
	switch( pWork->page.num ){
	case SK_PAGE_VERY_EASY :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_very_easy_top_addres;
		break;
	case SK_PAGE_EASY :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_easy_top_addres;
		break;
	case SK_PAGE_NORMAL :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_normal_top_addres;
		break;
	case SK_PAGE_HARD :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_hard_top_addres;
		break;
	case SK_PAGE_EXTERM :
		pWork->dogtag_list = ( Dogtag_List * )pWork->dogtag_exterm_top_addres;
		break;
	}
	if ( pWork->dogtag_list->pNext != NULL ){
		pWork->dogtag_list = pWork->dogtag_list->pNext;
		if ( pWork->dogtag_list->pNext == NULL ){
			printf( "なにもないっすよ\n");
		}
	}
}
#endif

#define LOCATION_LOW_A (64)

static void LocationControl( Work *pWork ) // DMA 初期化 まち
{
	if ( ( pWork->flag & SK_LOCATION_FADE_IN ) && ( pWork->location_wait_frame <= 0 ) ){
		pWork->location_alpha += IN_FADE_SPEED;
		if ( pWork->location_alpha >= 128 ){
			pWork->location_alpha = 128;
			pWork->flag &= ~SK_LOCATION_FADE_IN;
		}
	}
	if ( pWork->flag & SK_LOCATION_FADE_OUT ){
		pWork->location_alpha -= OUT_FADE_SPEED;
		if ( pWork->location_alpha <= 0 )
      {
         MENU_ClearPartTextTexture(pWork->location.work[ pWork->location.buffer ], 0, SYS_FONT_HEIGHT + 1);		
			pWork->location_alpha = 0;
			pWork->flag &= ~SK_LOCATION_FADE_OUT;
			return;
		}
	}
	if ( ( pWork->location_alpha <= LOCATION_LOW_A ) && !( pWork->flag & SK_CHANCEL ) ){
		// flag set
		pWork->flag |= SK_LOCATION_FADE_IN;
	}
	if ( pWork->location_wait_frame > 0 ){
		pWork->location_wait_frame--;
	}
#if 0
	if ( ( pWork->location_alpha <= 0 ) && !( pWork->flag & SK_CHANCEL ) ){
		MENU_ClearTextTexture( pWork->location.work[ pWork->location.buffer ] );
		// flag set
		pWork->flag |= SK_LOCATION_FADE_IN;
	}
	if ( pWork->location_wait_frame > 0 ){
		pWork->location_wait_frame--;
	}
#endif

}

static void ArrowFade( Work *pWork )
{
	int alpha;

	if ( pWork->flag & SK_LEFT_ARROW_FADE_OUT_START ){
		alpha = pWork->left_arrow->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < SK_A ){
			pWork->left_arrow->sprite.col.a = ( u_char )SK_A;
			pWork->flag &= ~SK_LEFT_ARROW_FADE_OUT_START;
		} else {
			pWork->left_arrow->sprite.col.a = ( u_char )alpha;
		}
	}
	if ( pWork->flag & SK_RIGHT_ARROW_FADE_OUT_START ){
		alpha = pWork->right_arrow->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < SK_A ){
			pWork->right_arrow->sprite.col.a = ( u_char )SK_A;
			pWork->flag &= ~SK_RIGHT_ARROW_FADE_OUT_START;
		} else {
			pWork->right_arrow->sprite.col.a = ( u_char )alpha;
		}
	}
	if ( pWork->flag & SK_TOP_ARROW_FADE_OUT_START ){
		alpha = pWork->top_arrow->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < SK_A ){
			pWork->top_arrow->sprite.col.a = ( u_char )SK_A;
			pWork->flag &= ~SK_LEFT_ARROW_FADE_OUT_START;
		} else {
			pWork->top_arrow->sprite.col.a = ( u_char )alpha;
		}
	}
	if ( pWork->flag & SK_BOTTOM_ARROW_FADE_OUT_START ){
		alpha = pWork->bottom_arrow->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < SK_A ){
			pWork->bottom_arrow->sprite.col.a = ( u_char )SK_A;
			pWork->flag &= ~SK_RIGHT_ARROW_FADE_OUT_START;
		} else {
			pWork->bottom_arrow->sprite.col.a = ( u_char )alpha;
		}
	}
#if 0
	// 表示非表示
	if ( SIG_GetTotalDogTagNum() == 0 ){
		SPR_HIDE( pWork->top_arrow );
		SPR_HIDE( pWork->bottom_arrow );
		return;
	}
#endif
	if ( pWork->position == pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] - 1 ){
		SPR_SHOW( pWork->top_arrow );
		SPR_HIDE( pWork->bottom_arrow );
		return;
	}
	switch ( pWork->position ){
	case 0 :
		SPR_SHOW( pWork->bottom_arrow );
		SPR_HIDE( pWork->top_arrow );
		break;
	default :	
		SPR_SHOW( pWork->top_arrow );
		SPR_SHOW( pWork->bottom_arrow );
		break;
	}
}

static void FontFade( Work *pWork )
{
	Dogtag_List *dogtag_list;
	int alpha;
	int   tmp;
	int     i;

	tmp = 0;
	dogtag_list = DogtagLinker( pWork );

	if ( pWork->flag & SK_FONT_FADE_OUT_START ){
		for ( i = 0 ; i < pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] ; i ++ ){
			alpha = ( int )dogtag_list->a;
			alpha -= OUT_FADE_SPEED;
			if ( alpha < 0 ){
				alpha = 0;
				dogtag_list->a = 0;
				tmp++;
			} else {
				dogtag_list->a = ( u_char )alpha;
			}
			// next
			if ( dogtag_list->pNext == NULL ){
				pWork->alpha = alpha;
				break;
			}
			dogtag_list = DogtagListForward( dogtag_list );
		}
		// jude
		if ( tmp >= i ){
			pWork->page.num = pWork->page.bufnum;
			pWork->flag |= SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_OUT_START;
		}
	}
	// check
	if ( pWork->flag & SK_FONT_FADE_IN_START ){
		// forward process 
		tmp = 0;
		dogtag_list = DogtagLinker( pWork );
		// main process 
		for ( i = 0 ; i < pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] ; i ++ ){
			alpha = ( int )dogtag_list->a;
			alpha += pWork->fade_plus;
			if ( alpha > 128 ){
				alpha = 128;
				dogtag_list->a = 128;
				tmp++;
			} else {
				dogtag_list->a = ( u_char )alpha;
			}
			if ( dogtag_list->pNext == NULL ){
				pWork->alpha = alpha;
				break;
			}
			dogtag_list = DogtagListForward( dogtag_list );
		}
		if ( tmp >= i ){
		  pWork->flag |= SK_FONT_FADE_IN_OK;
		  pWork->flag &= ~SK_FONT_FADE_IN_START;
		}
	}
	if ( pWork->flag & SK_FONT_ALL_BLACK ){
		// forward process 
		tmp = 0;
		dogtag_list = DogtagLinker( pWork );
		// main process 
		for ( i = 0 ; i < pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] ; i ++ ){
			dogtag_list->a = 0;
			if ( dogtag_list->pNext == NULL ){
				pWork->alpha = 0;
				break;
			}		
			dogtag_list = DogtagListForward( dogtag_list );
		}
		pWork->page.num = pWork->page.bufnum;
		pWork->flag &= ~SK_FONT_ALL_BLACK;
	}
}

static inline void PercentDisp( Work *pWork )
{
	char buf[ PERCENT_MAX + 10 ];/* bufのサイズ足りませんでしたyano 2002.07.11 */
	int percent;
	int       i;

	percent = ( int )( ( ( float )pWork->page.get_count[ pWork->select_mode ][ pWork->page.num ] /
						  ( pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] ) ) * 100.0f );
	sprintf( buf , "%03d%%" , percent );
	// 再構成 
	for ( i = 0 ; i < PERCENT_MAX ; i ++ ){
		if ( buf[ i ] == '0' && i != PERCENT_MAX - 2 ){
			buf[ i ] = 0x20;
		} else {
			break;
		}
	}
	SK_PrintfNormal( buf , pWork->percent , pWork->base_node_u , pWork->base_node_v , STR_WIDTH , STR_HEIGHT , PERCENT_MAX );
	SK_PrintfChengColor2( pWork->percent , SK_R , SK_G , SK_B , pWork->alpha , PERCENT_MAX );
	SK_AllShow( pWork->percent , PERCENT_MAX );
}

static inline void NumToId( Work *pWork , Dogtag_List *pDogtag_list , int point ) // データを文字に変換
{
	char buf[ ID_MAX + 1 ];

	sprintf( buf , "%03d" , pDogtag_list->id );
	SK_Printf2( buf , pWork->id[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , ID_MAX );

   //HACK to fix MGSTWO-2986. Gets rid of the extra pixel from the next character in the font sheet.
   pWork->id[point][ID_MAX - 1]->sprite.head.tex.w -= 1;

	SK_PrintfChengColor2( pWork->id[ point ] , SK_R , SK_G , SK_B , pDogtag_list->a , ID_MAX );
}

static inline void NumToBlood( Work *pWork , Dogtag_List *pDogtag_list , int point ) // データを文字に変換
{
	char buf[ BLOOD_MAX + 1 + 1 ];
	int alpha;

	if ( pDogtag_list->flag == 0 ){
		sprintf( buf , "--" );
		alpha = ( int )pDogtag_list->a / 2;
		SK_Printf2( buf , pWork->blood[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
	} else {
		alpha = ( int )pDogtag_list->a;
		switch ( pDogtag_list->blood ){
		case A :
			sprintf( buf , " A " );
			pWork->blood[ point ][ 0 ]->sprite.pos.x = DEFAULT_BLOOD_POS_X;
			SK_Printf2( buf , pWork->blood[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
			break;
		case B :
			sprintf( buf , " B " );
			pWork->blood[ point ][ 0 ]->sprite.pos.x = DEFAULT_BLOOD_POS_X;
			SK_Printf2( buf , pWork->blood[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
			break;
		case AB :
			sprintf( buf , "AB " );
			pWork->blood[ point ][ 0 ]->sprite.pos.x = DEFAULT_BLOOD_POS_X;
			SK_Printf2( buf , pWork->blood[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
			break;
		case O : 
			sprintf( buf , " O " );
			pWork->blood[ point ][ 0 ]->sprite.pos.x = DEFAULT_BLOOD_POS_X;
			SK_Printf2( buf , pWork->blood[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
			break;
		case BLOOD_NONE : // 不明
			sprintf( buf , " ? " );
			pWork->blood[ point ][ 0 ]->sprite.pos.x = DEFAULT_BLOOD_POS_X;
			SK_Printf2( buf , pWork->blood[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
			break;
		case BLOOD_GACKT : // がっくん
			sprintf( buf , "AAA" );
			pWork->blood[ point ][ 0 ]->sprite.pos.x = DEFAULT_BLOOD_POS_X - 6;
			SK_Printf2( buf , pWork->blood[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , BLOOD_MAX );
			break;
		}
	}
	SK_PrintfChengColor2( pWork->blood[ point ] , SK_R , SK_G , SK_B , alpha , BLOOD_MAX );
}

static inline void NumToReg( Work *pWork , Dogtag_List *pDogtag_list , int point ) // データを文字に変換
{
	char buf[ ASCCI_MAX ];
	u_char r , g , b;
	int alpha;

	if ( pDogtag_list->flag == 0 ){	 // 未取得
		sprintf( buf , "---" );
		alpha = ( int )pDogtag_list->a / 2;
		SK_Printf2( buf , pWork->region[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , REGION_MAX );
		r = SK_R;
		g = SK_G;
		b = SK_B;
	} else {
		alpha = ( int )pDogtag_list->a;
		if ( pDogtag_list->region[ 0 ] == '0' ){ // KCEJ
			sprintf( buf , "MGS" );
			SK_Printf2( buf , pWork->region[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , REGION_MAX );
			r = 90;
			g = 24;
			b = 24;
		} else { // 通常国名
			SK_Printf2( pDogtag_list->region , pWork->region[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , REGION_MAX );
			r = SK_R;
			g = SK_G;
			b = SK_B;
		}
	}
//	SK_PrintfChengColor2( pWork->region[ point ] , r , g , b , alpha , REGION_MAX );
	SK_PrintfChengColor2( pWork->region[ point ] , r , g , b , 0 , REGION_MAX );
}

static inline void NameUpdate( Work *pWork , Dogtag_List *pDogtag_list , int point )
{
	char buf[ ASCCI_MAX + 1 ];
	int alpha;

	if ( pDogtag_list->flag == 0 ){ // 取得していない
		sprintf( buf , "------------" );
		alpha = ( int )pDogtag_list->a / 2;
		SK_Printf2( buf , pWork->font[ point ] , pWork->base_u  , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , ASCCI_MAX );
	} else {
		alpha = ( int )pDogtag_list->a;
		SK_Printf2( pDogtag_list->name , pWork->font[ point ] , pWork->base_u  , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , ASCCI_MAX );
	}
	SK_PrintfChengColor2( pWork->font[ point ] , SK_R , SK_G , SK_B , ( u_char )alpha , ASCCI_MAX );
}

static void *GetLinkvariableAddr(void *addr)
{
	void *linkvar=GCL_GetLinkvarSaveAreaTop();
	return (void *)((int)linkvar+(((int)addr)-((int)linkvarbuf)));
}


#define STR_LOCATION_RESOURCE (7591201)
static inline void NumToLocation( Work *pWork , Dogtag_List *pDogtag_list , int point ) // データを文字に変換
{
	int width;
	int width2;
	float ftmp;
	u_int color;
	int   x1 , y1 , x2 , y2 , u1 , v1; 
	int   tmp;
	u_char *font_data;
   int redrawStageString = 0;

	short *SK_GM_TankerClearCount = ( short * )GetLinkvariableAddr( &GM_TankerClearCount );
	short *SK_GM_PlantClearCount  = ( short * )GetLinkvariableAddr( &GM_PlantClearCount );
#if 0
	short *SK_GM_GameClearCount   = ( short * )GetLinkvariableAddr( &GM_GameClearCount );
	short *SK_GM_StageNum         = ( short * )GetLinkvariableAddr( &GM_StageNum );
	short *SK_GM_TitleMenuStatus  = ( short * )GetLinkvariableAddr( &GM_TitleMenuStatus );
#endif
#if 0
	tmp = ( *SK_GM_TankerClearCount + *SK_GM_PlantClearCount ) - *SK_GM_GameClearCount;
	if ( ( *SK_GM_TitleMenuStatus & TITLE_MENU_STORY_SEL_MASK ) == TITLE_MENU_STORY_SEL_TANDP ){ // 特殊
		if ( *SK_GM_StageNum >= 14 && *SK_GM_StageNum <= 49 ) {
			tmp--;
		}
	}
#endif
	tmp = 0;
	switch ( pWork->select_position ){
	case 0 : // Tanker
		tmp = *SK_GM_TankerClearCount;
#ifdef DEBUG_MODE
		//printf("Tanker clear count %d\n" , tmp );
#endif
		break;
	case 1 : // Plant
		tmp = *SK_GM_PlantClearCount;
#ifdef DEBUG_MODE
		//printf("Plant clear count %d\n" , tmp );
#endif
		break;
	}
	if ( ( tmp <= 0 ) && ( pDogtag_list->flag != 0x1 ) ) {
		return;
	}
	// resource
	if( GM_Language == GM_LANG_JAPANESE ){
		font_data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( GV_StrCode("ステージ名日本語") , pDogtag_list->location ));
	} else {
		/* ステージ名が英語 */
		font_data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( STR_LOCATION_RESOURCE , pDogtag_list->location ));
	}

   if( pWork->mPrevStageString != font_data )
   {
      pWork->mPrevStageString = font_data;
      redrawStageString = 1;
   }

	ftmp = 384.0f / 448.0f;
	width = SYS_FONT_WIDTH * LOCATION_MAX;
	width2 = LOCATION_FONT_WIDTH * LOCATION_MAX;
	// color
	color = ( SK_R + 10 ) | ( ( SK_G + 10 ) << 8 ) | ( ( SK_B + 10 ) << 16 ) | ( pWork->location_alpha << 24 ); 
	x1 = DEFAULT_LOCATION_POS_X;
	y1 = DEFAULT_LOCATION_POS_Y;
	x2 = DEFAULT_LOCATION_POS_X + width2;
	y2 = ( int )( DEFAULT_LOCATION_POS_Y + LOCATION_FONT_HEIGHT * ftmp );
	u1 = width;
	v1 = ( int )( LOCATION_FONT_HEIGHT * ftmp );

   if ( redrawStageString )
   {
      MENU_ClearPartTextTexture(pWork->location.work[ pWork->location.buffer ], 0, SYS_FONT_HEIGHT + 1);
   }

   if ( BP_Area_JP() )
   {
      if ( redrawStageString )
      {
         MENU_CreateTextTexture( pWork->location.work[ pWork->location.buffer ] , 1 , 0 , width * 16 , SYS_FONT_HEIGHT + 1, 0 , 1 , 0 , font_data );
      }
      MENU_PutTextScreen( pWork->location.work[ pWork->location.buffer ] , x1 , y1 , x2 , y2, 1 , 1 , width + 1 , SYS_FONT_HEIGHT + 1, color );
   }
   else
   {
      if ( redrawStageString )
      {
         MENU_CreateTextTexture( pWork->location.work[ pWork->location.buffer ] , 1 , 0 , width * 16 , SYS_FONT_HEIGHT + 3 ,  0 , 1 , 0 , font_data );
      }
	   MENU_PutTextScreen( pWork->location.work[ pWork->location.buffer ] , x1 , y1 , x2 , y2, 1 , 1 , width + 1 , SYS_FONT_HEIGHT + 3, color );
   }

   // 反転
   //pWork->location.buffer ^= 1;   //BP dont bother with the attempted double buffer.

   // trans
}

static inline void NumToBarthday( Work *pWork , Dogtag_List *pDogtag_list , int point ) // データを文字に変換
{
 	char buf[ 4 + 1 ]; // 0316
	int alpha;

	if ( pDogtag_list->manth == 0 || pDogtag_list->flag == 0 ){
		if ( ( pDogtag_list->manth == 0 ) && ( pDogtag_list->flag != 0 ) ){
			sprintf( buf , "???" , pDogtag_list->manth );
			alpha = ( int )pDogtag_list->a;
		} else {
			sprintf( buf , "---" , pDogtag_list->manth );
			alpha = ( int )pDogtag_list->a / 2;
		}
	} else {
		sprintf( buf , "%02d%02d" , pDogtag_list->manth , pDogtag_list->day );
		alpha = ( int )pDogtag_list->a;
	}
	SK_Printf2( buf , pWork->barthday[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , MANTH_MAX + DAY_MAX );
	SK_PrintfChengColor2( pWork->barthday[ point ] , SK_R , SK_G , SK_B , alpha , MANTH_MAX + DAY_MAX );

#if 0
	if ( pDogtag_list->day == 0 || pDogtag_list->flag == 0 ){
		sprintf( buf , "--" , pDogtag_list->day );
		alpha = ( int )pDogtag_list->a / 2;
	} else {
		sprintf( buf , "%02d" , pDogtag_list->day );
		alpha = ( int )pDogtag_list->a;
	}
	SK_Printf2( buf , pWork->day[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , DAY_MAX );
	SK_PrintfChengColor2( pWork->day[ point ] , SK_R , SK_G , SK_B , alpha , DAY_MAX );
#endif
}

#if 0
static void LocationDisp( Work *pWork , Dogtag_List *pDogtag_list , int point )
{
	char buf[ LOCATION_MAX ];

	if ( pDogtag_list->flag & DOGTAG_LOCATION_OK ){
		NumToLocation( pWork , pDogtag_list , point );
	} else { // 空白
		sprintf( buf , "              " );
		SK_Printf2( buf , pWork->location[ point ] , pWork->base_u , pWork->base_v , STR_HEL_WIDTH , STR_HEL_HEIGHT , LOCATION_MAX );
		SK_PrintfChengColor2( pWork->location[ point ] , SK_R , SK_G , SK_B , pDogtag_list->a , LOCATION_MAX );
	}
}

static void LocationInit( Dogtag_List *pDogtag_list , int location_max )
{
	int location_back;
	int i;

	location_back = 0;
	for ( i = 0 ; i < location_max ; i ++ ){
		if ( location_back != pDogtag_list->location ){
			pDogtag_list->flag |= DOGTAG_LOCATION_OK;
			pDogtag_list->flag &= ~DOGTAG_LOCATION_NG;
			location_back = pDogtag_list->location;
		} else { // 空白
			pDogtag_list->flag |= DOGTAG_LOCATION_NG;
			pDogtag_list->flag &= ~DOGTAG_LOCATION_OK;
		}
		pDogtag_list = DogtagListForward( pDogtag_list ); 
	}
}
#endif

static inline Dogtag_List *DogtagLinker( Work *pWork )
{
	Dogtag_List *pDogtag_list;
	// そのページのpage先頭アドレスを出す
	// そのページのpage数を出す
	pDogtag_list = NULL;
	switch ( pWork->page.num ){
	case SK_PAGE_VERY_EASY :
		if ( pWork->select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_very_easy_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_very_easy_top_addres;
		}
		break;
	case SK_PAGE_EASY :
		if ( pWork->select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_easy_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_easy_top_addres;
		}
		break;
	case SK_PAGE_NORMAL :
		if ( pWork->select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_normal_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_normal_top_addres;
		}
		break;
	case SK_PAGE_HARD :
		if ( pWork->select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_hard_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_hard_top_addres;
		}
		break;
	case SK_PAGE_EXTERM :
		if ( pWork->select_mode == SK_TANKER ){
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_tanker_exterm_top_addres;
		} else {
			pDogtag_list = ( Dogtag_List * )pWork->dogtag_plant_exterm_top_addres;
		}
		break;
	default : 
		printf("無効なレベル設定です %d\n" , pWork->page.num );		
		ASSERT(0);
	}
	if ( pDogtag_list->pNext != NULL ){
		pDogtag_list = pDogtag_list->pNext;
	}
	return( pDogtag_list );
}

// BP FIX - was 16*12 but the UVs aren't fixed point internally any more
#define DIF_HEIGHT 12

static inline void DifControl( Work *pWork )
{
	int alpha;
	pWork->dif->sprite.head.tex.v = DIF_HEIGHT * pWork->page.num;

	if ( pWork->flag & SK_DIF_FADE_OUT_START ){
		alpha = ( int )pWork->dif->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha <= 0 ){
			pWork->dif->sprite.col.a = 0;
			pWork->flag &= ~SK_DIF_FADE_OUT_START;
		} else {
			pWork->dif->sprite.col.a = ( u_char )alpha;
		}
	}
}

static inline Dogtag_List *PageChange( Work *pWork ) // ページの切替 <-- レベルの変更
{
	// 今のページの情報が残りっぱなし
	if ( pWork->flag & SK_FONT_ALL_BLACK ){
		return (NULL);
	}
	pWork->position = 0; // 初期位置へ
	pWork->main_position = 0;
	HilightUpdate( pWork );
	pWork->flag |= SK_FONT_FADE_IN_START;
	pWork->flag &= ~SK_FONT_FADE_OUT_OK;
	pWork->flag &= ~SK_FONT_FADE_OUT_START;
	// そのページのpage先頭アドレスを出す
	// そのページのpage数を出す
	DogtagListPageSet( pWork , pWork->select_mode , pWork->page.num );

	if ( pWork->dogtag_list->pNext != NULL ){
		pWork->dogtag_list = pWork->dogtag_list->pNext;
	}
#if 0
	// ドット移動量を算出
	pWork->dot.plus = ( float )pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] / 206.0f;
#endif
	// flag 管理
	pWork->flag &= ~SK_PAGE_CHANGE_L;
	pWork->flag &= ~SK_PAGE_CHANGE_R;

	return( pWork->dogtag_list );
}

static inline void HilightUpdate( Work *pWork )
{
	int alpha;

	pWork->hilight_null->empty.pos.y = pWork->font[ pWork->position - pWork->main_position ][ 0 ]->sprite.pos.y - 3;
	// fade out
	if ( pWork->flag & SK_HILIGHT_FADE_OUT_START ){
		alpha = ( int )pWork->hilight_null->head.child->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha <= 0 ){
			pWork->hilight_null->head.child->sprite.col.a = 0;
			pWork->flag &= ~SK_HILIGHT_FADE_OUT_START;
			SPR_HIDE( pWork->hilight_null );
		} else {
			pWork->hilight_null->head.child->sprite.col.a = ( u_char )alpha;	
			SPR_SHOW( pWork->hilight_null );
		}
	}
	// fade in
	if ( pWork->flag & SK_HILIGHT_FADE_IN_START ){
		alpha = ( int )pWork->hilight_null->head.child->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha >= 72 ){
			pWork->hilight_null->head.child->sprite.col.a = 72;
			pWork->flag &= ~SK_HILIGHT_FADE_IN_START;
		} else {
			pWork->hilight_null->head.child->sprite.col.a = ( u_char )alpha;	
		}
		SPR_SHOW( pWork->hilight_null );
	}
}

static void PadControlNormal( Work *pWork )
{
	char flag;

	flag = 0;
	// main
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status & PAD_U )) 
   {
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->position > 0 ){
			pWork->position_sign--;
			pWork->position--; // <-- リストを進める数の事 いるのか？
			if ( pWork->position < pWork->main_position ){
				pWork->main_position--;
				pWork->main_position = pWork->main_position < 0 ? 0 : pWork->main_position;
				pWork->position = pWork->main_position;
			}
#if 0
			// top arrow の αを引き上げる
			pWork->top_arrow->sprite.col.a = 255;
			pWork->bottom_arrow->sprite.col.a = SK_A;
#endif
			SE_SEL();
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			pWork->flag &= ~SK_TAG_LOAD_OK;
			// location
			pWork->location_alpha = LOCATION_LOW_A;
			pWork->location_wait_frame = LOCATION_DMA_WAIT;
			pWork->memory.mcscr.step = MCSCR_SEL_FILE;
			return;
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status & PAD_D )
   {
      pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->position < pWork->page.now_page_max[ pWork->select_mode ][ pWork->page.num ] - 1 ){
			pWork->position_sign++;
			pWork->position++; // <-- リストを進める数の事
			if ( pWork->position == pWork->main_position + DISP_DOGTAG_MAX ){
				pWork->main_position = pWork->position - DISP_DOGTAG_MAX;
				pWork->main_position = pWork->main_position < 0 ? 0 : pWork->main_position + 1;
			}
#if 0
			// bottom arrow の αを引き上げる
			pWork->bottom_arrow->sprite.col.a = 255;
			pWork->top_arrow->sprite.col.a = SK_A;
#endif
			SE_SEL();
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			// location
			pWork->location_alpha = LOCATION_LOW_A;
			pWork->location_wait_frame = LOCATION_DMA_WAIT;
			pWork->memory.mcscr.step = MCSCR_SEL_FILE;
			return;
		}		  
	}
	// page 切替
	if ( ( ( GV_PadDataDirect[ 0 ].press & PAD_L ) || ( GV_PadDataDirect[ 0 ].press & PAD_L1 ) )  && ( flag == 0 ) )
   {
      if ( pWork->page.num > 0 )
      {
			pWork->page.bufnum = pWork->page.num - 1;
			// location alpha black
			pWork->location_alpha = 0;
			pWork->location_wait_frame = LOCATION_DMA_WAIT;
			// hilight
			pWork->hilight_null->head.child->sprite.col.a = 0;
			pWork->flag |= SK_HILIGHT_FADE_IN_START;
			// etc
			pWork->left_arrow->sprite.col.a = 255;
			pWork->right_arrow->sprite.col.a = SK_A;
			pWork->flag |= SK_FONT_ALL_BLACK;
			pWork->flag |= SK_PAGE_CHANGE_L;
			pWork->flag |= SK_LEFT_ARROW_FADE_OUT_START;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			SE_TYPE();
		}
		flag = 0x1;
	}
	if ( ( ( GV_PadDataDirect[ 0 ].press & PAD_R ) || ( GV_PadDataDirect[ 0 ].press & PAD_R1 ) ) && ( flag == 0 ) ) 
   {
      if ( pWork->page.num < PAGE_MAX - 1 ){
			pWork->page.bufnum = pWork->page.num + 1;
			// location alpha black
			pWork->location_alpha = 0;
			pWork->location_wait_frame = LOCATION_DMA_WAIT;
			// hilight
			pWork->hilight_null->head.child->sprite.col.a = 0;
			pWork->flag |= SK_HILIGHT_FADE_IN_START;
			// etc
			pWork->right_arrow->sprite.col.a = 255;
			pWork->left_arrow->sprite.col.a = SK_A;
			pWork->flag |= SK_FONT_ALL_BLACK;
			pWork->flag |= SK_PAGE_CHANGE_R;
			pWork->flag |= SK_RIGHT_ARROW_FADE_OUT_START;
			pWork->flag &= ~SK_FONT_FADE_OUT_OK;
			pWork->flag &= ~SK_FONT_FADE_IN_START;
			SE_TYPE();
		} 
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & CANCEL ) && ( flag == 0 ) ){ // back exit proc call
		if ( pWork->flag & SK_SELECT_TANKER ){
			pWork->action = STR_HIDE_VIEWER_TANKER;
		} else {
			pWork->action = STR_HIDE_VIEWER_PLANT;
		}
		// 初期に戻す
		pWork->flag = 0;
		pWork->flag |= ( SK_SELECT_SCREEN_START | SK_ACTION_START | SK_FONT_FADE_OUT_START | SK_LOCATION_FADE_OUT |
						 SK_HILIGHT_FADE_OUT_START | SK_DIF_FADE_OUT_START | SK_CHANCEL | SK_NORMAL );
		SE_CANCEL();

		return;
	}
	// auto mode
	if ( GV_PadDataDirect[ 0 ].status != pWork->pad_check ){ //( PAD_U | PAD_D | PAD_L | PAD_R | PAD_L1 | PAD_R1 ) ) ){
		pWork->hold_time = 0;
		pWork->pad_status = 0;
	}
	if ( pWork->hold_time >= DIRECT_TICK( 22 ) ){
		pWork->pad_status = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time -= DIRECT_TICK( SK_REPEAT );
	} else if ( pWork->pad_check & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time++;
	} else {
		pWork->pad_check = GV_PadDataDirect[ 0 ].status;
		pWork->pad_status = 0;
	} 
}

#define STR_HIDETOP (7600424)
static void PadControlSelect( Work *pWork )
{
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) ) {
		return;
	}
	// main
	if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status & PAD_U )
   {
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->select_position > 0 ){
			pWork->select_position--; // <-- リストを進める数の事 いるのか？
			// dot
			pWork->dot_hokan.end_y = pWork->select.tanker->sprite.pos.y - 50;
			pWork->dot_hokan.count = 5;
			pWork->dot_hokan.flag = 0;
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_CUR01 );
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			SE_SEL();
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status & PAD_D )
   {
		pWork->pad_status = 0;
		pWork->time = 0;
		if ( pWork->select_position < 1 ){
			pWork->select_position++; // <-- リストを進める数の事
			// dot
			pWork->dot_hokan.end_y = pWork->select.plant->sprite.pos.y - 50;
			pWork->dot_hokan.count = 5;
			pWork->dot_hokan.flag = 0;
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_CUR01 );
			pWork->flag |= SK_FONT_FADE_IN_START;
			pWork->flag &= ~SK_FONT_FADE_IN_OK;
			SE_SEL();
		}		  
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){ // back exit proc call
		switch ( pWork->select_position ){// koba4
		case 0 : // tanker
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_SELECT_TANKER;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->action = STR_SELECT_TANKER;
			break;
		case 1 : // plant
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_SELECT_TANKER;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->action = STR_SELECT_PLANT;
			break;
		}
		pWork->flag |= SK_SELECT | SK_NORMAL_SCREEN_START;
		pWork->flag &= ~SK_SELECT_SCREEN_OK;
		SE_OK();

		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){ // back exit proc call
		pWork->action = STR_HIDETOP;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_SELECT_SCREEN_OK;
		pWork->flag &= ~SK_ACTION_OK;
		SE_CANCEL();

		return;
	}
}

static void PadAct( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle );
	// アクション再生中はさよなら
	if ( status == L2D_STAT_BUSY ){
		return;
	}
	if ( pWork->flag & SK_NORMAL_SCREEN_OK ){
		PadControlNormal( pWork );
	}
	if ( pWork->flag & SK_SELECT_SCREEN_OK ){
		PadControlSelect( pWork );
		DotUpdate( pWork );
	}
}

static void NormalAct( Work *pWork )
{
	if ( !( pWork->flag & SK_NORMAL ) ){
		SprInit( pWork ); // 文字の初期化
		return;
	}
	// hilight
	HilightUpdate( pWork );
	
	// cancel
	if ( pWork->flag & SK_CHANCEL ){
		if ( ( pWork->flag & SK_FONT_FADE_OUT_OK ) && !( pWork->flag & SK_LOCATION_FADE_OUT ) && !( pWork->flag & SK_HILIGHT_FADE_OUT_START ) && 
			 !( pWork->flag & SK_DIF_FADE_OUT_START ) ){
			pWork->flag &= ~SK_CHANCEL;
			pWork->position = 0;
			pWork->main_position = 0;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->act = ( void * )SelectAct;
		}
		FontUpdate( pWork );
		FontFade( pWork );
		PercentDisp( pWork );
		LocationControl( pWork );
		DifControl( pWork );
		return;
	}
	// page change
	if ( pWork->flag & SK_PAGE_CHANGE_L || pWork->flag & SK_PAGE_CHANGE_R ){
		PageChange( pWork );
	}
	// main
	PadAct( pWork );
	LocationControl( pWork );
	FontUpdate( pWork );
	FontFade( pWork );
	PercentDisp( pWork );
	DifControl( pWork );
	ArrowFade( pWork );
}

static void Act( Work *pWork )
{
 	pWork->act( pWork );

#ifdef PSX2
	/* ドックタグのバージョンを表示 */
	if( pWork->spr_flag == 1 ){
		/* l2dの2001表示を2002表示にuv値を書き換える */
		SPR_OBJ *spr;
		spr = pWork->spr2001_2;
		spr->ex_hd.tex.v  = spr->ex_hd.tex.pv + SPR_FIXED(12.0f);
	}
#endif
}

static	void	Die( Work *pWork )
{
	int i , j;

	for ( i = 0 ; i < 2 ; i ++ ){ // tanker 
		for ( j = 0 ; j < PAGE_MAX ; j ++ ){
			DogtagListFree( pWork , i , j ); // リストの解放
		}
	}
#if 0
	if ( pWork->memory.mcscr.data != NULL ){
		GV_DelayedFree( pWork->memory.mcscr.data );
	}
	if ( pWork->memory.data != NULL ){
		GV_DelayedFree( pWork->memory.data );
	}
	if ( pWork->handle >= 0 ){ // 上で解放するからいい
		L2D_ReleaseLayout( pWork->handle );
	}
#endif
}

static inline void DotUpdate( Work *pWork ) // update
{
	// dot position update
	if ( !( pWork->dot_hokan.flag & SK_MOVE_OK ) ){ 
		pWork->select.curs->empty.pos.y += ( pWork->dot_hokan.end_y - pWork->select.curs->empty.pos.y ) / pWork->dot_hokan.count;
		pWork->dot_hokan.count--;
		if ( pWork->dot_hokan.count <= 0 ){
			pWork->dot_hokan.count = 1;
		}
		if ( pWork->select.curs->empty.pos.y == pWork->dot_hokan.end_y ){
			pWork->dot_hokan.flag |= SK_MOVE_OK;
		}
	}
	SPR_SHOW( pWork->select.curs );
}

#define STR_PLANT (3641915)
#define STR_TANKER_PLANTNULL (14631708)
static void SelectInitialize( Work *pWork )
{
	SPR_OBJ *spr;

	spr = L2D_GetObject( pWork->handle , STR_CURS_NULL );
	if ( spr == NULL ){
		SK_Err("str_curs not ready\0");
		return;
	}
	pWork->select.curs = spr;

	spr = L2D_GetObject( pWork->handle , STR_TANKER_PLANTNULL );
	if ( spr == NULL ){
		SK_Err("TANKER_PLANTNULL not ready\0");
		return;
	}
	pWork->select.tanker = spr->empty.head.child;

	spr = L2D_GetObject( pWork->handle , STR_PLANT );
	if ( spr == NULL ){
		SK_Err("str_plant not ready\0");
		return;
	}
	pWork->select.plant = spr;
	// dot init
	pWork->dot_hokan.end_y = pWork->select.tanker->sprite.pos.y - 50;
	pWork->dot_hokan.count = 1;
}

static void DotFade( Work *pWork ) // ドットのfade
{
	int alpha;
	int tmp;

	tmp = 0;
	if ( pWork->flag & SK_DOT_FADE_OUT_START ){
		// tanker
		alpha = ( int )pWork->select.tanker->sprite.col.a;
		alpha += ( 0 - alpha ) / pWork->dot.count;
		if ( alpha <= 0 ) {
			alpha = 0;
			tmp++;
		}
		pWork->select.tanker->sprite.col.a = alpha;
		// plant
		alpha = ( int )pWork->select.plant->sprite.col.a;
		alpha += ( 0 - alpha ) / pWork->dot.count;
		if ( alpha <= 0 ) {
			alpha = 0;
			tmp++;
		}
		pWork->select.plant->sprite.col.a = alpha;
		pWork->dot.count = pWork->dot.count > 2 ? pWork->dot.count - 1 : 1;
		if ( tmp >= 2 ){
			pWork->flag &= ~SK_DOT_FADE_OUT_START;
		}
	}
}

static void SelectUpdate( Work *pWork )
{
	int alpha;

	switch ( pWork->select_position ){
	case 0 : // tanker
		// dot position
//		pWork->select.curs->empty.pos.y = pWork->select.tanker->sprite.pos.y - 50.0f;
		// font alpha
		alpha = ( int )pWork->select.tanker->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha > 128 ){
			pWork->select.tanker->sprite.col.a = ( u_char )128;
		} else {
			pWork->select.tanker->sprite.col.a = ( u_char )alpha;
		}
		alpha = ( int )pWork->select.plant->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < 64 ){
			pWork->select.plant->sprite.col.a = ( u_char )64;
		} else {
			pWork->select.plant->sprite.col.a = ( u_char )alpha;
		}
		break;
	case 1 : // plant
		// dot position
//		pWork->select.curs->empty.pos.y = pWork->select.plant->sprite.pos.y - 50.0f;
		// font alpha
		alpha = ( int )pWork->select.plant->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha > 128 ){
			pWork->select.plant->sprite.col.a = ( u_char )128;
		} else {
			pWork->select.plant->sprite.col.a = ( u_char )alpha;
		}
		alpha = ( int )pWork->select.tanker->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < 64 ){
			pWork->select.tanker->sprite.col.a = ( u_char )64;
		} else {
			pWork->select.tanker->sprite.col.a = ( u_char )alpha;
		}
		break;
	}
}

static void SelectAct( Work *pWork )
{
	int status;

	// good by
	if ( pWork->flag & SK_CHANCEL ){
		GV_CallParentSignalFunc( pWork , SK_DESTROY , 0 );
		GV_DestroyActor( pWork );
		return;
	}
	// dot
	DotFade( pWork );

	// main
	status = L2D_ActionStatus( pWork->handle );
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
//			L2D_BreakAction( pWork->handle );
			L2D_EvokeAction( pWork->handle , pWork->action );
			if ( pWork->flag & SK_SELECT_SCREEN_START ){
				pWork->flag |= SK_ACTION_OK | SK_SELECT_SCREEN_OK;
				pWork->flag &= ~SK_ACTION_START;
				pWork->flag &= ~SK_SELECT_SCREEN_START;
			} else {
				pWork->flag |= SK_ACTION_OK;
				pWork->flag &= ~SK_ACTION_START;
			}
		}
		return;
	}
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch( pWork->action ){
		case STR_HIDETOP :
			pWork->flag |= SK_CHANCEL;
			pWork->flag &= ~SK_ACTION_OK;
			return;
		}
	}
	if ( pWork->flag & SK_SELECT ){
		pWork->flag |= SK_NORMAL_SCREEN_START | SK_ACTION_START;
		pWork->flag &= ~SK_SELECT;
		pWork->flag &= ~SK_ACTION_OK;
		switch ( pWork->select_position ){
		case SK_TANKER : // tanker
			pWork->select_mode = SK_TANKER;
			pWork->action = STR_SHOW_VIEWER_TANKER;
			break;
		case SK_PLANT : // plant
			pWork->select_mode = SK_PLANT;
			pWork->action = STR_SHOW_VIEWER_PLANT;
			break;
		}
	} else if ( !( pWork->flag & SK_NORMAL_SCREEN_START ) && ( pWork->flag & SK_SELECT_SCREEN_OK ) ){
		if ( status != L2D_STAT_BUSY ){
			PadAct( pWork );
			SelectUpdate( pWork );
		}
	}
	if ( pWork->flag & SK_SELECT_SCREEN_OK ){
		GV_CallParentSignalFunc( pWork ,  SK_LAYOUT_RERESE , 0 );
	}
}

#define SIG_FIX_FADE_IN (990588)
#define SIG_SHOW_TOP (2948071)
#define SIG_FADEOUT (7595356)
#define SIG_HIDE_MENU (8092995)
static void Signal( void *work , int sign , int value )
{
	Work *pWork;

	pWork = ( Work * )work;

	switch ( sign ){
	case SIG_FIX_FADE_IN : // fix の fade in	
		pWork->flag &= ~SK_SELECT_SCREEN_OK;
		pWork->flag &= ~SK_FONT_FADE_OUT_START;
		pWork->flag &= ~SK_FONT_FADE_OUT_OK;
		pWork->flag |= ( SK_NORMAL_SCREEN_OK | SK_FONT_FADE_IN_START | SK_LOCATION_FADE_IN | SK_HILIGHT_FADE_IN_START );
		pWork->position = 0;
		pWork->main_position = 0;
		pWork->page.num = SK_DEFAULT_PAGE;
		pWork->page.bufnum = SK_DEFAULT_PAGE;
		pWork->page.a = 128;
		pWork->alpha = 0;
		pWork->location_alpha = 0;
		pWork->position_sign = 0;
		pWork->pad_status = 0;
		pWork->pad_check = 0;
		pWork->time = 0;
		pWork->fade_plus = 128.0f / ( float )value; // 0 割禁止
		pWork->act = ( void * )NormalAct;

		break;
	case SIG_SHOW_TOP : // キャンセルのときもの画面を出すタイミング
		pWork->flag |= ( SK_SELECT_SCREEN_START | SK_ACTION_START );
		pWork->action = STR_SHOW_TOP;
		break;
	case SIG_HIDE_MENU : // 
		pWork->flag |= SK_DOT_FADE_OUT_START;
		pWork->dot.count = DIRECT_TICK( ( int )( ( ( float )value / 300.0f ) * 60.0f ) );
		break;
	}
}

/*----------------------------------------------------------------*/
extern void SIG_SetDogTagFlag(int num);
static	int	GetResources( Work *work )
{
	float     ftmp;
	int      i , j;

	ftmp = ( DRAW_HEIGHT / 384.0f );

	L2D_SetSignalHandle( work->handle , work , Signal );
	work->action = STR_SHOW_TOP;
	work->act = (void *)SelectAct;//AnimetionAct;
	work->main_position = 0;
	work->position = 0;
	work->position_sign = 0;
	work->pad_status = 0;
	work->pad_check = 0;
	work->time = 0;
	work->flag = 0;
	work->flag |= ( SK_SELECT_SCREEN_START | SK_ACTION_START );
	work->alpha = 0;
	work->location_alpha = 0;
	work->select_position = 0;

	// page
	work->page.num = SK_DEFAULT_PAGE;
	work->page.bufnum = 1;
	work->page.a = 128;
	for ( i = 0 ; i < MODE_MAX ; i ++ ){
		for ( j = 0 ; j < PAGE_MAX ; j ++ ){
			work->page.get_count[ i ][ j ] = 0;
			work->page.now_page_max[ i ][ j ] = 0;
		}
	}
	// location
	work->location.buffer = 0;
	work->location.work[ 1 ] = NewTextScreenControlForTitle();
	if ( work->location.work[ 1 ] == NULL ){
		return ( -1 );
	}
	GV_SetActorChild( work , work->location.work[ 1 ] ); // 貴方の子供よ
	for ( i = 0 ; i < 2 ; i ++ ){
		MENU_ClearTextTexture( work->location.work[ i ] );
	}
	// select
	SelectInitialize( work );
#if 0
	// test
	//	for ( i = 0 ; i < 316 ; i++ ){
	  	SIG_SetDogTagFlag( 11 );
	  	SIG_SetDogTagFlag( 50 );
	  	SIG_SetDogTagFlag( 27 );
	  	SIG_SetDogTagFlag( 42 );
		//	}
#endif

	// dogtag list
	DogtagListInitialize( work );
  	DogtagListAddMng( work );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewDogtagMode( void *font_work , int handle )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->location.work[ 0 ] = font_work;
	work->handle = handle; // 先に確保しておいたものをもらう
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

