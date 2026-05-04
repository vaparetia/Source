//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   question.c
   アンケート
   
   2001/06/05	S.Kobayashi
   $Id: previous_story.c,v 1.2 2002/12/05 18:42:00 takaki Exp $
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
#include "libfs.h"

#include "mode/menu/xtextscn.h"

#if 1 //BP_XBOX def KP_XBOX
#define SPR_FTOI( _a ) (int)( _a*16.0f )
#endif

#define 	STR_MSG_AREA   (2480789)
#define     STR_DEFAULT    (566267)
#define     STR_ROOT       (2770484)
#define     STR_NODE_FONT  (3459636)
#define 	FONT_WIDTH     ( 13 )
#define 	FONT_HEIGHT    ( 14 )
#define     DOT_POS_X      ( 25.0f )
#define     DOT_POS_Y      ( 78.0f )
#define 	STR_WIDTH  (18) // strcode ではなく文字の長さ
#define 	STR_HEIGHT (14) // strcode ではなく文字の長さ

#define DEFAULT_PAGE_POS_X (324) //53
#define DEFAULT_PAGE_POS_Y (324+20)
#define DEFAULT_PAGE_NUM_POS_X (382)
#define DEFAULT_PAGE_NUM_POS_Y (324+20)
#define DEFAULT_PAGE_SURASHU_X (420)
#define DEFAULT_EXIT_POS_X (40)
#define DEFAULT_EXIT_POS_Y (320+20)
#define PAGE_FONT_MAX ( 7 ) // 000/000
#define PAGE_MAX (6)

#define MAX_POSITION (3)

#define MSG_BASE_X ( 74 )
#define MSG_BASE_Y ( 26 )
#define SYS_FONT_WIDTH ( 24 )
#define SYS_FONT_HEIGHT ( 24 )

#define MAX_L2D (4)
#define MESG_PANEL_W		(512-64+FONT_SIZE_W)
#define MESG_PANEL_H		((FONT_SIZE_H+FONT_SIZE_H/2)*3+FONT_SIZE_H/2)
#define MAX_BOOK_FADE (15)

// 
// parts
// menu 
#define STR_DARK_TXT_UNSEL (11469194)
#define STR_DARK_TXT_SEL (2073238)
#define STR_SHOCK_TXT_UNSEL (7854631)
#define STR_SHOCK_TXT_SEL (4641996)
#define STR_NEW_TXT_UNSEL (15190841)
#define STR_NEW_TXT_SEL (9138376)

// book-c
#define STR_KEY_FACE_A (786657)
#define STR_KEY_FACE_B (786658)
#define STR_KEY_FACE_C (786659)
#define STR_KEY_FACE_D (786660)
#define STR_KEY_FACE_E (786661)
#define STR_KEY_TEXRECT_2 (12681452)
#define STR_KEY_TEXRECT_3 (12681453)
#define STR_KEY_TEXRECT_4 (12681454)
#define STR_KEY_TEXRECT_5 (12681455)
#define STR_KEY_TEXRECT_6 (12681456)
#define STR_KEY_TEXRECT_7 (12681457)
#define STR_KEY_TEXRECT_8 (12681458)
#define STR_KEY_TEXRECT_9 (12681459)
#define STR_KEY_FILTER (9485746)
#define STR_FILTER     (13098468)
#define STR_FILLRECT_1 (11522436)

// book-b
#define STR_KEY_TEXRECT_10 (3153320)
#define STR_KEY_TEXRECT_11 (3153321)
#define STR_KEY_FILLRECT_12 (9171043)
#define STR_KEY_FILLRECT_13 (9171044)
#define STR_KEY_FILLRECT_14 (9171045)
#define STR_KEY_FILLRECT_15 (9171046)
// code
#define STR_SHOW_MOSES_B12 (10378572) // texrect-6
#define STR_HIDE_MOSES_B12 (8939396) // texrect-6
#define STR_SHOW_MOSES_B11 (10378571) // texrect-7
#define STR_HIDE_MOSES_B11 (8939395) // texrect-7
#define STR_SHOW_MOSES_B10 (10378569) // texrect-8
#define STR_HIDE_MOSES_B10 (8939394) // texrect-8
#define STR_SHOW_MOSES_B9 (13955824) // texrect-9
#define STR_HIDE_MOSES_B9 (9716546) // texrect-9
#define STR_SHOW_MOSES_B8 (13955823) // texrect-10
#define STR_HIDE_MOSES_B8 (9716545) // texrect-10
#define STR_SHOW_MOSES_B7 (13955822) // texrect-11
#define STR_HIDE_MOSES_B7 (9716544) // texrect-11
#define STR_SHOW_MOSES_B6 (13955821) // key_FillRect-12
#define STR_HIDE_MOSES_B6 (9716543) // key_FillRect-12
#define STR_SHOW_MOSES_B5 (13955820) // texrect-4
#define STR_HIDE_MOSES_B5 (9716542) // texrect-4
#define STR_SHOW_MOSES_B4 (13955819) // key_FillRect-13
#define STR_HIDE_MOSES_B4 (9716541) // key_FillRect-13
#define STR_SHOW_MOSES_B3 (13955818) // key_FillRect-14
#define STR_HIDE_MOSES_B3 (9716540) // key_FillRect-14
#define STR_SHOW_MOSES_B2 (13955817) // key_FillRect-15
#define STR_HIDE_MOSES_B2 (9716539) // key_FillRect-15
#define STR_SHOW_MOSES_B1 (13955816) // key_filter
#define STR_HIDE_MOSES_B1 (9716538) // key_filter

#define STR_SHOW_MOSES_C1 (13955848) // filter
#define STR_HIDE_MOSES_C1 (9716570) // filter
#define STR_SHOW_MOSES_C15 (10379599) // fillrect-1
#define STR_HIDE_MOSES_C15 (8940423) // fillrect-1
#define STR_SHOW_MOSES_C2 (13955849) // texrect-9
#define STR_HIDE_MOSES_C2 (9716571) // texrect-9
#define STR_SHOW_MOSES_C3 (13955850) // texrect-8
#define STR_HIDE_MOSES_C3 (9716572) // texrect-8
#define STR_SHOW_MOSES_C4 (13955851) // texrect-7
#define STR_HIDE_MOSES_C4 (9716573) // texrect-7
#define STR_SHOW_MOSES_C5 (13955852) // texrect-6
#define STR_HIDE_MOSES_C5 (9716574) // texrect-6
#define STR_SHOW_MOSES_C6 (13955853) // texrect-5
#define STR_HIDE_MOSES_C6 (9716575) // texrect-5
#define STR_SHOW_MOSES_C7 (13955854) // texrect-4
#define STR_HIDE_MOSES_C7 (9716576) // texrect-4
#define STR_SHOW_MOSES_C8 (13955855) // texrect-3
#define STR_HIDE_MOSES_C8 (9716577) // texrect-3
#define STR_SHOW_MOSES_C9 (13955856) // texrect-2
#define STR_HIDE_MOSES_C9 (9716578) // texrect-2
#define STR_SHOW_MOSES_C10 (10379594) // key_face_e
#define STR_HIDE_MOSES_C10 (8940418) // key_face_e
#define STR_SHOW_MOSES_C11 (10379595) // key_face_d
#define STR_HIDE_MOSES_C11 (8940419) // key_face_d
#define STR_SHOW_MOSES_C12 (10379596) // key_face_c
#define STR_HIDE_MOSES_C12 (8940420) // key_face_c
#define STR_SHOW_MOSES_C13 (10379597) // key_face_b
#define STR_HIDE_MOSES_C13 (8940421) // key_face_b
#define STR_SHOW_MOSES_C14 (10379598) // key_face_a
#define STR_HIDE_MOSES_C14 (8940422) // key_face_a
// PRESTMENU
#define STR_KEY_NEWYORK (13263029) 
#define STR_KEY_SHOCK (13897680)
#define STR_KEY_DARKNESS (7017249)
#define STR_KEY_PIC_DARKNESS (14879008)
#define STR_KEY_PIC_SHOCK (12586688)
#define STR_KEY_PIC_NEW (8215464)
#define STR_SELCURSNULL (6756848)
#define STR_SELCURS (14909507)

#define STR_NEW_PIC_HIDE (6849684)
#define STR_NEW_TXT_HIDE (6858870)
#define STR_SHOCK_PIC_HIDE (13960459)
#define STR_SHOCK_TXT_HIDE (13969645)
#define STR_DARK_PIC_HIDE (15646278)
#define STR_DARK_TXT_HIDE (15655464)
// code
#define STR_CURS1 (7100055) // selcurs_null
#define STR_CURS2 (7100056) // selcurs_null
#define STR_CURS3 (7100057) // selcurs_null
#define STR_CURS4 (7100058) // selcurs_null
#define STR_HIDECURS (7781825) // selcurs
#define STR_SHOWCURS (9901464) // selcurs
#define STR_HIDE_P_NEW (10321705) // key_pic_new
#define STR_SHOW_P_NEW (16571306) // key_pic_new
#define STR_HIDE_P_SHOCK (5116737) // key_pic_shock
#define STR_SHOW_P_SHOCK (12589246) // key_pic_shock
#define STR_HIDE_P_DARK (11195646) // key_pic_darkness
#define STR_SHOW_P_DARK (9856298) // key_pic_darkness
#define STR_HIDE_DARK (9368699) // key_darkness
#define STR_SHOW_DARK (10088287) // key_darkness
#define STR_HIDE_SHOCK (13763293) // key_shock
#define STR_SHOW_SHOCK (3235679) // key_shock
#define STR_HIDE_NEW (8691749) // key_newyork
#define STR_SHOW_NEW (10811388) // key_newyork
#define STR_NEW_PIC_UNSEL (14896889)
#define STR_NEW_PIC_SEL (8089513)
#define STR_SHOCK_PIC_UNSEL (7560679)
#define STR_SHOCK_PIC_SEL (3593133)
#define STR_DARK_PIC_UNSEL (11175242)
#define STR_DARK_PIC_SEL (1024375)

/*
5523302 シャドーモセス日本1
16323736 シャドーモセスアメリカ1
16723560 シャドーモセスフランス1
16327476 シャドーモセスイタリア1
10713433 シャドーモセスドイツ1
14079370 シャドーモセススペイン1
*/
// book
#define STR_STORY_JAPAN_1       (5523302)
#define STR_STORY_JAPAN_2       (5523303)
#define STR_STORY_JAPAN_3       (5523304)

#define STR_STORY_ENGLISH_1     (16323736)
#define STR_STORY_ENGLISH_2     (16323737)
#define STR_STORY_ENGLISH_3     (16323738)

#define STR_STORY_GERMAN_1      (10713433)
#define STR_STORY_GERMAN_2      (10713434)
#define STR_STORY_GERMAN_3      (10713435)

#define STR_STORY_FRANCE_1      (16723560)
#define STR_STORY_FRANCE_2      (16723561)
#define STR_STORY_FRANCE_3      (16723562)

#define STR_STORY_ITARY_1       (16327476)
#define STR_STORY_ITARY_2       (16327477)
#define STR_STORY_ITARY_3       (16327478)

#define STR_STORY_SPAIN_1       (14079370)
#define STR_STORY_SPAIN_2       (14079371)
#define STR_STORY_SPAIN_3       (14079372)

// action strcode
#define STR_OPENBOOK (352028)
#define STR_IDLEBOOK (7679017)
#define STR_CLOSEBOOK (12142327)
#define STR_OPENBOOK_BG (9407318)
#define STR_IDLEBOOK_BG (1459005)
#define STR_OPENSPECIAL (15448060)
#define STR_CLOSESPECIAL (14262640)
// L2D strcode
#define STR_MOSES_A     (6583398)
#define STR_MOSES_B     (6583399)
#define STR_MOSES_C     (6583400)
#define STR_MOSES_CLOSE (12456459)
#define STR_MOSES_A1    (9342205)
#define STR_MOSES_B1    (9342237)
#define STR_MOSES_C1    (9342269)
#define STR_PRESTMENU   (15453282)

#define IN_FADE_SPEED (4)
#define OUT_FADE_SPEED (4)
#define BOOK_SHOW_TIME (DIRECT_TICK(80))
// pic
#define STR_LAST_TOP_JAPAN (3383581)
#define STR_LAST_TOP_ENGLISH (15507720)
#define STR_LAST_TOP_PAL (5349683)

#define STR_LAST_BOTTOM_JAPAN (3387677)
#define STR_LAST_BOTTOM_ENGLISH (15511816)
#define STR_LAST_BOTTOM_PAL (5353779)
#define STR_PAGEARROW (10352869)

// color
#define FONT_R (90)//100)
#define FONT_G (90)
#define FONT_B (90)//110)

#define SK_HIPAGE_R (90)
#define SK_HIPAGE_G (12)
#define SK_HIPAGE_B (10)
#define SK_HIPAGE_A (128)

#define SK_NEXTPAGE_R (80)
#define SK_NEXTPAGE_G (90)
#define SK_NEXTPAGE_B (85)
#define SK_NEXTPAGE_A (128)

#define SK_NO_NEXTPAGE_R (80)
#define SK_NO_NEXTPAGE_G (90)
#define SK_NO_NEXTPAGE_B (85)
#define SK_NO_NEXTPAGE_A (52)

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define STR_TRI_CODE (3656015) // logo.tri
// exit
#define STR_EXIT_TEX (7644145)

// sound
#define SE_NPAP()       GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_OP_NPAP1)
#define SE_BOOK1()      GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_OP_BOOK1)
#define SE_BOOK2()      GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_OP_BOOK2)
#define SE_PAGEN()      GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGEN001)
#define SE_PAGEB()      GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGEB001)
#define SE_PAGE10N()    GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGE10N1)
#define SE_PAGE10B()    GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGE10B1)
#define SE_PAGECLN()    GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGECLN1)
#define SE_PAGECLB()    GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGECLB1)
#define SE_MOV()        GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_LINEMOV1)
#define SE_WINOPN()     GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINOPN01)
#define SE_WINCLS()     GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLS01)

enum {
	SK_NORMAL              = 0x00001 ,
	SK_END                 = 0x00002 ,
	SK_CANCEL              = 0x00004 ,
	SK_SPRITE_INIT         = 0x00008 ,	
	SK_ACTION_START        = 0x00010 ,
	SK_ACTION_OK           = 0x00020 ,
	SK_NORMAL_MODE         = 0x00040 ,
	SK_SELECT_MODE         = 0x00080 ,
	SK_MOVE_OK             = 0x00100 ,
	SK_POSITION_CHENGE     = 0x00200 ,
	SK_BLACK_RECT_FADE_OUT = 0x00400 ,
	SK_BLACK_RECT_FADE_IN  = 0x00800 ,
	SK_STRING_FADE_IN      = 0x01000 ,
	SK_PAGE_CHENGE         = 0x02000 ,
	SK_READ_OK             = 0x04000 ,
	SK_EXIT_FADE_IN        = 0x08000 ,
	SK_EXIT_FADE_OUT       = 0x10000 ,
	SK_SPRITE_INIT_START   = 0x20000 ,
	SK_BOOK_SHOW           = 0x40000 ,
	SK_SIGNAL_OK           = 0x80000 ,
	SK_LAST_PIC_SHOW       = 0x100000 ,
	SK_BLACK_RECT_FADE_IN_SUB = 0x200000 ,
	SK_BLACK_RECT_FADE_OUT_SUB = 0x400000 ,
	SK_SE_COUNT                = 0x800000 ,
	SK_DUMY_FADE               = 0x1000000 ,
};

typedef struct {
	float p;
	int   count;
	int   code1;
	int   code2;
	int   morf_parts;
	int   flag;
	int   handle; // l2d handle
} Hokan;

typedef struct {
	SPR_OBJ  *obj; // 描画領域計算の為必要
	u_char   r;
	u_char   g;
	u_char   b;
	u_char   a;
	char     *font; // Resource 
	void     *work[ 2 ]; // Demon Double Buffer
	int      clock;      // buffer clock
} Msg; // 表示領域管理

typedef struct {
	int position; // 現在のページ
	int max;  // 最大ページ数
	int flag;
	int r , g , b;
	int raute;
	SPR_OBJ *font[ PAGE_MAX ]; // PAGE
	SPR_OBJ *now[ PAGE_FONT_MAX ]; // 現在ページ 00/00
	SPR_OBJ *exit; // exit tex
} Page; // 本の総ページ数

typedef struct {
	int	handle_2d;
	int action; 
	int flag;
} L2d;

typedef struct {
	Hokan hokan;
	SPR_OBJ *obj;
} Black;

typedef struct {
	Hokan hokan;
} BookFade;

typedef struct {
	int a;
	Hokan hokan[ 2 ];
} Menu;

typedef struct {
	SPR_OBJ *pic_top;
	SPR_OBJ *pic_bottom;
	SPR_OBJ *black;
} LastPic;

typedef struct {
	int r , g , b , a ;
	int raute;
} Arrow;

typedef	struct _work {
	GV_ACT_EX			actor ;
	L2d                 l2d[ MAX_L2D ]; // 0 : a , 1 : b , 2 : c , 3 : close , 4 : c1
	Msg                 msg; // 表示領域の関係
	Page                page[ MAX_POSITION ];
	int                 page_alpha;
	Black               black; // fade out 専用
	LastPic             last_pic;
	BookFade            book_fade[ MAX_BOOK_FADE ]; // 本消滅専用
	Arrow               arrow_left;
	Arrow               arrow_right;
	Hokan               dot_hokan;
	Menu                menu[ MAX_POSITION ];
	int                 book_count;
	int                 base_u;
	int                 base_v;
	int                 now_story; // 現在読んでいる本
	int                 max_position;
	int                 position; // どの本を選んだか
	int                 old_position;
	int                 pad_status;
	int                 pad_check;
	int                 hold_time[ 2 ]; // auto mode 
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	int                 flag;
	int                 se_counter;
	int                 tex_handle;
	void  ( *act )( struct _work * );

	int 	msg_width;
	int 	msg_height;
	int 	msg_tex_width;
	int 	msg_tex_height;
#if 1 ///test
	SPR_OBJ *line;
#endif
} Work ;

static int SK_Story[ 3 ];

// プロトタイプ
// static
static void NormalAct( Work * );
static void AnimationAct( Work * );
static void OpenBook( Work *pWork );
static void CloseBook( Work *pWork );
static void BlackRectControl( Work *pWork );
static void BookDisp( Work *pWork );
static void PageChenge( Work *pWork );
static void DotUpdate( Work *pWork );
static int LayoutSet( Work *pWork , int strcode , int strcode2 ); // 3 , 4 にセット

static inline int MorfAct( Hokan *pHokan ); // これのみ実行
static inline void MorfSet( Hokan *pHokan , int parts , int code1 , int code2 , int value , int handle );

// extern 
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num );
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void MENU_PutTextScreenF( void *work_ptr, float x1, float y1, float x2, float y2, int u1, int v1, int u2, int v2, int col );
extern void MENU_SetTextSpace( void *work, int space );
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え

void *GetLocalResource( int ref_id, int offset );

static void LangChecge( void ) // 言語切替関数
{
	if ( GM_Language == GM_LANG_JAPANESE ){
		SK_Story[ 0 ] = STR_STORY_JAPAN_1;
		SK_Story[ 1 ] = STR_STORY_JAPAN_2;
		SK_Story[ 2 ] = STR_STORY_JAPAN_3;
#ifdef DEBUG_MODE
		printf("Japanese Book\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_ENGLISH ){
		SK_Story[ 0 ] = STR_STORY_ENGLISH_1;
		SK_Story[ 1 ] = STR_STORY_ENGLISH_2;
		SK_Story[ 2 ] = STR_STORY_ENGLISH_3;
#ifdef DEBUG_MODE
		printf("English Book\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_FRENCH ){
		SK_Story[ 0 ] = STR_STORY_FRANCE_1;
		SK_Story[ 1 ] = STR_STORY_FRANCE_2;
		SK_Story[ 2 ] = STR_STORY_FRANCE_3;
#ifdef DEBUG_MODE
		printf("French Book\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_GERMANY ){
		SK_Story[ 0 ] = STR_STORY_GERMAN_1;
		SK_Story[ 1 ] = STR_STORY_GERMAN_2;
		SK_Story[ 2 ] = STR_STORY_GERMAN_3;
#ifdef DEBUG_MODE
		printf("Germany Book\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_ITALY ){
		SK_Story[ 0 ] = STR_STORY_ITARY_1;
		SK_Story[ 1 ] = STR_STORY_ITARY_2;
		SK_Story[ 2 ] = STR_STORY_ITARY_3;
#ifdef DEBUG_MODE
		printf("Italy Book\n");
#endif
		return;
	}
	if ( GM_Language == GM_LANG_SPANISH ){
		SK_Story[ 0 ] = STR_STORY_SPAIN_1;
		SK_Story[ 1 ] = STR_STORY_SPAIN_2;
		SK_Story[ 2 ] = STR_STORY_SPAIN_3;
#ifdef DEBUG_MODE
		printf("Spanish Book\n");
#endif
		return;
	}
}

static int BlackInit( Work *pWork )
{
	SPR_POS pos;
#ifndef BP_PS2
	SPR_POS ydmy;
#endif


	// black fade 
    pWork->black.obj = SPR_Create_2D_Object( SP_SPRITE , 4 , NULL );
	if ( pWork->black.obj == NULL ){
		SK_Err("black_rect\0");
		return ( -1 );
	}
	pos.x = 0.0f;
	pos.y = 0.0f;
    /* 表示座標の設定 */
    SPR_SetPosSprite( pWork->black.obj , &pos );
    /* 表示サイズの設定 */
    SPR_SetSizeSprite( pWork->black.obj , 512.0f , 448.0f );
    /* アルファブレンディングの設定 */
    pWork->black.obj->head.alpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );
	pWork->black.obj->head.flags |= SPR_FLAG_ALPHA;
	// プライオリティ 
	SPR_SetPriority( pWork->black.obj , 6 );
	pWork->black.obj->sprite.col.r = 0;
	pWork->black.obj->sprite.col.g = 0;
	pWork->black.obj->sprite.col.b = 0;
	pWork->black.obj->sprite.col.a = 0;
	SPR_HIDE( pWork->black.obj );

	// tri load
    pWork->tex_handle = SPR_LoadTexture( STR_TRI_CODE );
	if ( pWork->tex_handle < 0 ){
		SK_Err("non tri\0");
		ASSERT( 0 )
	}
	// last pic
    pWork->last_pic.pic_top = SPR_Create_2D_Object( SP_SPRITE , 4 , NULL );
	if ( pWork->last_pic.pic_top == NULL ){
		SK_Err("pic_top\0");
		return ( -1 );
	}
//#ifdef JAPANESE_BP_IGNORE()
   if ( BP_Area_JP() )
   {
	   SPR_ObjSetTexture( pWork->last_pic.pic_top , STR_LAST_TOP_JAPAN , pWork->tex_handle );
	   printf("Japanese\n");
   }
   else if ( BP_Area_EU() )
   {
//#else 
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	   SPR_ObjSetTexture( pWork->last_pic.pic_top , STR_LAST_TOP_PAL , pWork->tex_handle );
	   printf("Pal\n");
   }
//#else
//#ifdef ENGLISH
   else
   {
	   SPR_ObjSetTexture( pWork->last_pic.pic_top , STR_LAST_TOP_ENGLISH , pWork->tex_handle );
	   printf("English\n");
   }
//#endif
//#endif
//#endif


    /* 表示座標の設定 */
// yano
#ifdef BP_PS2 /* gcc 特有の表記方法 */ 
    SPR_SetPosSprite( pWork->last_pic.pic_top , &(SPR_POS){ 256.0f - 316.0f*0.5f, 16.0f*384.0f/448.0f});
#else
	ydmy.x = 256.0f - 316.0f*0.5f;
	ydmy.y = 16.0f*384.0f/448.0f;
    SPR_SetPosSprite( pWork->last_pic.pic_top , &ydmy );	
#endif
    /* 表示サイズの設定 */
	SPR_SetSizeSprite( pWork->last_pic.pic_top, 316.0f, 240.0f*384.0f/448.0f );
    /* アルファブレンディングの設定 */
    pWork->last_pic.pic_top->head.alpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );
	pWork->last_pic.pic_top->head.flags |= SPR_FLAG_ALPHA;
	// プライオリティ 
	SPR_SetPriority( pWork->last_pic.pic_top , 6 );
	SET_COLOR_2DPRIM( pWork->last_pic.pic_top , 0 );
	pWork->last_pic.pic_top->sprite.col.r = 128;
	pWork->last_pic.pic_top->sprite.col.g = 128;
	pWork->last_pic.pic_top->sprite.col.b = 128;
	pWork->last_pic.pic_top->sprite.col.a = 0;
	SPR_HIDE( pWork->last_pic.pic_top );

	// last bottom
    pWork->last_pic.pic_bottom = SPR_Create_2D_Object( SP_SPRITE , 4 , NULL );
	if ( pWork->last_pic.pic_bottom == NULL ){
		SK_Err("pic_bottom\0");
		return ( -1 );
	}
//#ifdef JAPANESE_BP_IGNORE()
   if ( BP_Area_JP() )
	   SPR_ObjSetTexture( pWork->last_pic.pic_bottom , STR_LAST_BOTTOM_JAPAN , pWork->tex_handle );
//#else 
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
   else if ( BP_Area_EU() )
   	SPR_ObjSetTexture( pWork->last_pic.pic_bottom , STR_LAST_BOTTOM_PAL , pWork->tex_handle );
//#else 
//#ifdef ENGLISH
   else 
	   SPR_ObjSetTexture( pWork->last_pic.pic_bottom , STR_LAST_BOTTOM_ENGLISH , pWork->tex_handle );
//#endif
//#endif
//#endif

   if ( BP_Area_EU() )
   {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
    /* 表示座標の設定 */
#ifdef BP_PS2 /* gcc 特有の表記方法 */ 
   	SPR_SetPosSprite( pWork->last_pic.pic_bottom, &(SPR_POS){ 82.0f, 256.0f*384.0f/448.0f});
#else
	   ydmy.x = 82.0f;
	   ydmy.y = 256.0f*384.0f/448.0f;
      SPR_SetPosSprite( pWork->last_pic.pic_bottom , &ydmy );	
#endif
    /* 表示サイズの設定 */
   	SPR_SetSizeSprite( pWork->last_pic.pic_bottom, 352.0f, 132.0f*384.0f/448.0f );
   }
   else // NOT EU
   {
//#else 
    /* 表示座標の設定 */
#ifdef BP_PS2 /* gcc 特有の表記方法 */ 
   	SPR_SetPosSprite( pWork->last_pic.pic_bottom, &(SPR_POS){ 140.0f, 256.0f*384.0f/448.0f});
#else
	   ydmy.x = 140.0f;
	   ydmy.y = 256.0f*384.0f/448.0f;
      SPR_SetPosSprite( pWork->last_pic.pic_bottom , &ydmy );	
#endif
    /* 表示サイズの設定 */
   	SPR_SetSizeSprite( pWork->last_pic.pic_bottom, 232.0f, 132.0f*384.0f/448.0f );
   }
//#endif
    /* アルファブレンディングの設定 */
    pWork->last_pic.pic_bottom->head.alpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );
	pWork->last_pic.pic_bottom->head.flags |= SPR_FLAG_ALPHA;
	// プライオリティ 
	SPR_SetPriority( pWork->last_pic.pic_bottom , 6 );
	SET_COLOR_2DPRIM( pWork->last_pic.pic_bottom , 0 );
	pWork->last_pic.pic_bottom->sprite.col.r = 128;
	pWork->last_pic.pic_bottom->sprite.col.g = 128;
	pWork->last_pic.pic_bottom->sprite.col.b = 128;
	pWork->last_pic.pic_bottom->sprite.col.a = 0;
	SPR_HIDE( pWork->last_pic.pic_bottom );

	// black fade 
    pWork->last_pic.black = SPR_Create_2D_Object( SP_SPRITE , 4 , NULL );
	if ( pWork->last_pic.black == NULL ){
		SK_Err("black_rect\0");
		return ( -1 );
	}
	pos.x = 0.0f;
	pos.y = 0.0f;
    /* 表示座標の設定 */
    SPR_SetPosSprite( pWork->last_pic.black , &pos );
    /* 表示サイズの設定 */
    SPR_SetSizeSprite( pWork->last_pic.black , 512.0f , 448.0f );
    /* アルファブレンディングの設定 */
    pWork->last_pic.black->head.alpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );
	pWork->last_pic.black->head.flags |= SPR_FLAG_ALPHA;
	// プライオリティ 
	SPR_SetPriority( pWork->last_pic.black , 5 );
	pWork->last_pic.black->sprite.col.r = 0;
	pWork->last_pic.black->sprite.col.g = 0;
	pWork->last_pic.black->sprite.col.b = 0;
	pWork->last_pic.black->sprite.col.a = 0;
	SPR_HIDE( pWork->last_pic.black );

	return (0);
}

static void LastPageShow( Work *pWork ) // 宣伝
{
	if ( pWork->position != 0 && pWork->flag & SK_LAST_PIC_SHOW ){
		return;
	}
	// 黒板
	pWork->black.obj->sprite.col.a = 0;

	pWork->flag |= SK_BLACK_RECT_FADE_IN_SUB;
	pWork->flag &= ~SK_BLACK_RECT_FADE_OUT_SUB;
	// pic
	SPR_SHOW( pWork->last_pic.pic_top );
	SPR_SHOW( pWork->last_pic.pic_bottom );
	
	pWork->flag &= ~SK_LAST_PIC_SHOW;
}

static void LastPageHide( Work *pWork ) // 宣伝
{
	if ( pWork->position != 0 && !( pWork->flag & SK_LAST_PIC_SHOW ) ){
		return;
	}
	// 黒板
	pWork->flag |= SK_BLACK_RECT_FADE_OUT_SUB;
	pWork->flag &= ~SK_BLACK_RECT_FADE_IN_SUB;
	// pic
	SPR_HIDE( pWork->last_pic.pic_top );
	SPR_HIDE( pWork->last_pic.pic_bottom );

	pWork->flag &= ~SK_LAST_PIC_SHOW;
}

static void LastPageInit( Work *pWork ) // 初期化
{
	// 黒板
	pWork->flag &= ~SK_BLACK_RECT_FADE_OUT_SUB;
	pWork->flag &= ~SK_BLACK_RECT_FADE_IN_SUB;
	// pic
	SPR_HIDE( pWork->last_pic.black );
	SPR_HIDE( pWork->last_pic.pic_top );
	SPR_HIDE( pWork->last_pic.pic_bottom );
	// etc
	pWork->last_pic.black->sprite.col.r = 0;
	pWork->last_pic.black->sprite.col.g = 0;
	pWork->last_pic.black->sprite.col.b = 0;
	pWork->last_pic.black->sprite.col.a = 0;

	pWork->last_pic.pic_bottom->sprite.col.r = 128;
	pWork->last_pic.pic_bottom->sprite.col.g = 128;
	pWork->last_pic.pic_bottom->sprite.col.b = 128;
	pWork->last_pic.pic_bottom->sprite.col.a = 0;

	pWork->last_pic.pic_top->sprite.col.r = 128;
	pWork->last_pic.pic_top->sprite.col.g = 128;
	pWork->last_pic.pic_top->sprite.col.b = 128;
	pWork->last_pic.pic_top->sprite.col.a = 0;
}

static int SprInit( Work *pWork )
{
	SPR_OBJ *spr;
	int status;
	int i;

	status = L2D_ActionStatus( pWork->l2d[ 2 ].handle_2d );
	if ( ( pWork->flag & SK_SPRITE_INIT ) || ( status == L2D_STAT_BUSY ) || !( pWork->flag & SK_SPRITE_INIT_START )){
		return ( 1 );
	}
	// ページ用フォント取得
	spr = L2D_GetObject( pWork->l2d[ 2 ].handle_2d , STR_NODE_FONT );
	if ( spr == NULL ){
		SK_Err("node_font1\0");
		return -1 ;
	}
	SPR_SHOW( spr );
#if 0 //BP_PS2 def PSX2
	pWork->base_u = spr->sprite.head.tex.u; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v;
#else
	pWork->base_u = (int)(spr->sprite.head.tex.u * 16.0f); // テクスチャの位置取得
	pWork->base_v = (int)(spr->sprite.head.tex.v * 16.0f);
#endif
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_PAGE_NUM_POS_X;
	spr->sprite.pos.y = DEFAULT_PAGE_NUM_POS_Y;
	for ( i = 0 ; i < PAGE_FONT_MAX ; i ++ ){ // 設定
		pWork->page[ pWork->position ].now[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->page[ pWork->position ].now[ i ] == NULL ){
			SK_Err("Non Memory\0");
			return ( -1 );
		}
		pWork->page[ pWork->position ].now[ i ]->sprite.pos.y = DEFAULT_PAGE_NUM_POS_Y;
	}
	SK_PrintfNormal( NULL , pWork->page[ pWork->position ].now , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_FONT_MAX );
	SPR_HIDE( spr );
	SK_AllHide( pWork->page[ pWork->position ].now , PAGE_FONT_MAX );
	// page自体
	spr = L2D_GetObject( pWork->l2d[ 2 ].handle_2d , STR_NODE_FONT );
	if ( spr == NULL ){
  	SK_Err("node_font1\0");
		return -1 ;
	}
	SPR_SHOW( spr );
#if 0 //BP_PS2 def PSX2
	pWork->base_u = spr->sprite.head.tex.u; // テクスチャの位置取得
	pWork->base_v = spr->sprite.head.tex.v;
#else
	pWork->base_u = SPR_FTOI( spr->sprite.head.tex.u ); // テクスチャの位置取得
	pWork->base_v = SPR_FTOI( spr->sprite.head.tex.v );
#endif
	spr->sprite.dw = 14;
	spr->sprite.dh = 10;
	spr->sprite.pos.x = DEFAULT_PAGE_POS_X;
	spr->sprite.pos.y = DEFAULT_PAGE_POS_Y;
	for ( i = 0 ; i < PAGE_MAX ; i ++ ){ // 設定
		pWork->page[ pWork->position ].font[ i ] = SPR_DuplicateTree( spr );
		if ( pWork->page[ pWork->position ].font[ i ] == NULL ){
			SK_Err("Non Memory\0");
			return ( -1 );
		}
		pWork->page[ pWork->position ].font[ i ]->sprite.pos.y = DEFAULT_PAGE_POS_Y;
	}
	{
		char buf[] = "PAGE\0";
		SK_PrintfNormal( NULL , pWork->page[ pWork->position ].font , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_MAX );
		SK_PrintfNormal( buf , pWork->page[ pWork->position ].font , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_MAX );
	}
	SPR_HIDE( spr );
	SK_AllHide( pWork->page[ pWork->position ].font , PAGE_MAX );
	// exit tex
	spr = L2D_GetObject( pWork->l2d[ 3 ].handle_2d , STR_EXIT_TEX );
	if ( spr == NULL ){
		SK_Err("exit\0");
		return -1 ;
	}
	SPR_HIDE( spr );
	spr->sprite.pos.x = DEFAULT_EXIT_POS_X;
	spr->sprite.pos.y = DEFAULT_EXIT_POS_Y;
	spr->sprite.col.r = FONT_R;
	spr->sprite.col.g = FONT_G;
	spr->sprite.col.b = FONT_B;
	spr->sprite.col.a = 128;
	pWork->page[ pWork->position ].exit = spr;

	// arrow
	{
		SPR_OBJ *arrow_left;
		SPR_OBJ *arrow_right;

		spr = L2D_GetObject( pWork->l2d[ 2 ].handle_2d , STR_PAGEARROW );
		if ( spr == NULL ){
			SK_Err("page_arrow\0");
			return (-1);
		}
	
		arrow_right = spr->head.child;
		arrow_left = arrow_right->head.next;

		arrow_left->sprite.col.r = 0;
		arrow_left->sprite.col.g = 0;
		arrow_left->sprite.col.b = 0;
		arrow_left->sprite.col.a = 0;
		pWork->arrow_left.r = SK_NO_NEXTPAGE_R;
		pWork->arrow_left.g = SK_NO_NEXTPAGE_G;
		pWork->arrow_left.b = SK_NO_NEXTPAGE_B;
		pWork->arrow_left.a = SK_NO_NEXTPAGE_A;
		pWork->arrow_left.raute = 1;

		arrow_right->sprite.col.r = 0;
		arrow_right->sprite.col.g = 0;
		arrow_right->sprite.col.b = 0;
		arrow_right->sprite.col.a = 0;
		pWork->arrow_right.r = SK_NEXTPAGE_R;
		pWork->arrow_right.g = SK_NEXTPAGE_G;
		pWork->arrow_right.b = SK_NEXTPAGE_B;
		pWork->arrow_right.a = SK_NEXTPAGE_A;
		pWork->arrow_right.raute = 1;
	}
	pWork->flag |= SK_SPRITE_INIT;
	pWork->flag &= ~SK_SPRITE_INIT_START;

	return (0);
}

static void ArrowControl( Work *pWork )
{
	SPR_OBJ *spr;
	SPR_OBJ *arrow_left;
	SPR_OBJ *arrow_right;

	spr = L2D_GetObject( pWork->l2d[ 2 ].handle_2d , STR_PAGEARROW );
	if ( spr == NULL ){
		SK_Err("page_arrow\0");
		return;
	}
	
	arrow_right = spr->head.child;
	arrow_left = arrow_right->head.next;
	// left
    arrow_left->sprite.col.r += ( pWork->arrow_left.r - arrow_left->sprite.col.r ) / pWork->arrow_left.raute;
    arrow_left->sprite.col.g += ( pWork->arrow_left.g - arrow_left->sprite.col.g ) / pWork->arrow_left.raute;
    arrow_left->sprite.col.b += ( pWork->arrow_left.b - arrow_left->sprite.col.b ) / pWork->arrow_left.raute;
    arrow_left->sprite.col.a += ( pWork->arrow_left.a - arrow_left->sprite.col.a ) / pWork->arrow_left.raute;
	if ( pWork->arrow_left.raute > 1 ){
		pWork->arrow_left.raute--;
	}
	// right
    arrow_right->sprite.col.r += ( pWork->arrow_right.r - arrow_right->sprite.col.r ) / pWork->arrow_right.raute;
    arrow_right->sprite.col.g += ( pWork->arrow_right.g - arrow_right->sprite.col.g ) / pWork->arrow_right.raute;
    arrow_right->sprite.col.b += ( pWork->arrow_right.b - arrow_right->sprite.col.b ) / pWork->arrow_right.raute;
    arrow_right->sprite.col.a += ( pWork->arrow_right.a - arrow_right->sprite.col.a ) / pWork->arrow_right.raute;
	if ( pWork->arrow_right.raute > 1 ){
		pWork->arrow_right.raute--;
	}
#if 0
	// left
	for ( i = 0 ; i < 4 ; i ++ ) {
		alpha = ( int )arrow_left->poly.col[ i ].a;
		if ( pWork->page[ pWork->position ].position == 0 ){
			alpha = 52;
			arrow_left->poly.col[ i ].r = 255;
			arrow_left->poly.col[ i ].g = 255;
			arrow_left->poly.col[ i ].b = 255;
		} else {
			alpha -= IN_FADE_SPEED;
		}
		if ( alpha <= 128 ){
			alpha = 128;
			arrow_left->poly.col[ i ].r = 255;
			arrow_left->poly.col[ i ].g = 255;
			arrow_left->poly.col[ i ].b = 255;
		}
		arrow_left->poly.col[ i ].a = ( u_char )alpha;
	}
	// right
	for ( i = 0 ; i < 4 ; i ++ ) {
		alpha = ( int )arrow_right->poly.col[ i ].a;
		if ( pWork->page[ pWork->position ].position == pWork->page[ pWork->position ].max - 1 ){
			alpha = 52;
			arrow_right->poly.col[ i ].r = 255;
			arrow_right->poly.col[ i ].g = 255;
			arrow_right->poly.col[ i ].b = 255;
		} else {
			alpha -= IN_FADE_SPEED;
		}
		if ( alpha <= 128 ){
			alpha = 128;
			arrow_right->poly.col[ i ].r = 255;
			arrow_right->poly.col[ i ].g = 255;
			arrow_right->poly.col[ i ].b = 255;
		}
		arrow_right->poly.col[ i ].a = ( u_char )alpha;
	}
#endif
}

static inline void SK_Exit( Work *pWork )
{
	int value;

	value = DIRECT_TICK( 4 );
	MorfSet( &pWork->book_fade[ 0 ].hokan , STR_SELCURS , STR_SHOWCURS , STR_HIDECURS , value , pWork->l2d[ 0 ].handle_2d );
	MorfSet( &pWork->book_fade[ 1 ].hokan , STR_KEY_PIC_NEW , STR_NEW_PIC_SEL , STR_NEW_PIC_HIDE , value , pWork->l2d[ 0 ].handle_2d );
	MorfSet( &pWork->book_fade[ 2 ].hokan , STR_KEY_NEWYORK , STR_NEW_TXT_SEL , STR_NEW_TXT_HIDE , value , pWork->l2d[ 0 ].handle_2d );
	MorfSet( &pWork->book_fade[ 3 ].hokan , STR_KEY_PIC_SHOCK , STR_SHOCK_PIC_SEL , STR_SHOCK_PIC_HIDE , value , pWork->l2d[ 0 ].handle_2d );
	MorfSet( &pWork->book_fade[ 4 ].hokan , STR_KEY_SHOCK , STR_SHOCK_TXT_SEL , STR_SHOCK_TXT_HIDE , value , pWork->l2d[ 0 ].handle_2d );
	MorfSet( &pWork->book_fade[ 5 ].hokan , STR_KEY_PIC_DARKNESS , STR_DARK_PIC_SEL , STR_DARK_PIC_HIDE , value , pWork->l2d[ 0 ].handle_2d );
	MorfSet( &pWork->book_fade[ 6 ].hokan , STR_KEY_DARKNESS , STR_DARK_TXT_SEL , STR_DARK_TXT_HIDE , value , pWork->l2d[ 0 ].handle_2d );
	pWork->flag = SK_END;
	pWork->l2d[ 0 ].action = STR_CLOSESPECIAL;
	pWork->l2d[ 0 ].flag |= SK_ACTION_START;
	pWork->l2d[ 0 ].flag &= ~SK_ACTION_OK;
}

// Japanease or Usa or Pal
#define SELECT PAD_OK     //元はPAD_A yano 2002.05.10
#define CANCEL PAD_CANCEL //元はPAD_B

static void PadControlNormal( Work *pWork ) // 通常選択
{
	static int StrCode[][ 2 ] = { { STR_MOSES_A , STR_MOSES_C1 } ,
								  { STR_MOSES_B , STR_MOSES_C1 } , 
								  { STR_MOSES_C , STR_MOSES_C1 } };
 
	if ( GV_PadDataDirect[ 0 ].press & PAD_U || pWork->pad_status & PAD_U ){
		pWork->pad_status = 0;
		if ( pWork->position == 3 ){
			SE_SEL();
			pWork->old_position = pWork->position;
			pWork->position = pWork->max_position - 1;
		} else if ( pWork->position > 0 ){
			SE_SEL();
			pWork->old_position = pWork->position;
			pWork->position--;
		}
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_D || pWork->pad_status & PAD_D ){
		pWork->pad_status = 0;
		if ( pWork->position < pWork->max_position - 1 ){
			SE_SEL();
			pWork->old_position = pWork->position;
			pWork->position++;
		} else { // exit
			SE_SEL();
			pWork->old_position = pWork->position;
			pWork->position = 3;
		}
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
		if ( pWork->position == 3 ){
			SE_CANCEL();
			SK_Exit( pWork );
		} else {
			SE_OK();
			pWork->flag &= ~SK_NORMAL_MODE;
			pWork->page[ pWork->position ].position = 0;
			pWork->black.obj->sprite.col.a = 128;
			pWork->msg.a = 0;
			// layout init
			LayoutSet( pWork , StrCode[ pWork->position ][ 0 ] , StrCode[ pWork->position ][ 1 ] );
			pWork->l2d[ 3 ].action = STR_OPENBOOK_BG;
			pWork->l2d[ 3 ].flag |= SK_ACTION_START;
			pWork->l2d[ 3 ].flag &= ~SK_ACTION_OK;
			// book show time
			pWork->book_count = BOOK_SHOW_TIME;
			// book data get
			pWork->msg.font = BP_GCL_LOOKUP_NEW_FONT_STRING(( char * )GetLocalResource( pWork->now_story , pWork->page[ pWork->position ].position ));
			// flag set
			pWork->flag |= SK_POSITION_CHENGE | SK_BLACK_RECT_FADE_OUT | SK_STRING_FADE_IN | SK_BOOK_SHOW | SK_EXIT_FADE_IN;
			pWork->flag &= ~SK_NORMAL_MODE;
		}
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		SK_Exit( pWork );
		SE_CANCEL();
		return;
	}
	// auto mode
	if ( pWork->hold_time[ 0 ] >= DIRECT_TICK( 30 ) ){
		pWork->pad_status = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time[ 0 ] -= DIRECT_TICK( 4 );
	} else if ( pWork->pad_check & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 0 ]++;
	} else {
		pWork->pad_check = GV_PadDataDirect[ 0 ].status;
	} 
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_U | PAD_D ) ) ){
		pWork->hold_time[ 0 ] = 0;
		pWork->pad_status = 0;
	}
}

static inline void BookPageSe( Work *pWork , int mode ) // mode 1 Page Or 10 Page
{
	switch ( pWork->position ){
	case 0 : // book1
	case 1 : // book1
		if ( mode == 0 ){ // Page 1
			SE_PAGEN();
		} else {
			SE_PAGE10N();
		}
		break;
	case 2 : // book2
		if ( mode == 0 ){ // Page 1
			SE_PAGEB();
		} else {
			SE_PAGE10B();
		}
		break;
	}
}

static inline void BookCloseSe( Work *pWork )
{
	switch ( pWork->position ){
	case 0 : // book1
	case 1 : // book1
		SE_PAGECLN();
		break;
	case 2 : // book2
		SE_PAGECLB();
		break;
	}
}

#define HI_PAGE_READ (10)

static void PadControlSelectMode( Work *pWork )
{
	SPR_OBJ *spr;
	SPR_OBJ *arrow_left;
	SPR_OBJ *arrow_right;

	spr = L2D_GetObject( pWork->l2d[ 2 ].handle_2d , STR_PAGEARROW );
	if ( spr == NULL ){
		SK_Err("page_arrow pad\0");
		return;
	}
	// arrow	
	arrow_right = spr->head.child;
	arrow_left = arrow_right->head.next;

	if ( pWork->hold_time[ 1 ] >= 2 ){
		if ( ( pWork->pad_status & PAD_L1 ) ||
			 ( pWork->pad_status & PAD_L ) ){
			pWork->pad_status = 0;
			pWork->flag &= ~SK_LAST_PIC_SHOW; // 最後を表示して
			pWork->flag &= ~SK_DUMY_FADE;
			if ( pWork->page[ pWork->position ].position != 0 ) {			
				BookPageSe( pWork , 1 );
				pWork->flag |= SK_PAGE_CHENGE;
				if ( pWork->page[ pWork->position ].position - HI_PAGE_READ > 0 ){
					if ( pWork->page[ pWork->position ].position == pWork->page[ pWork->position ].max - 1 && pWork->position == 0 ){
						LastPageHide( pWork );
					}
					pWork->page[ pWork->position ].position -= HI_PAGE_READ;
				} else {
					pWork->page[ pWork->position ].position = 0;
					pWork->arrow_left.r = SK_NO_NEXTPAGE_R;
					pWork->arrow_left.g = SK_NO_NEXTPAGE_G;
					pWork->arrow_left.b = SK_NO_NEXTPAGE_B;
					pWork->arrow_left.a = SK_NO_NEXTPAGE_A;
					pWork->arrow_left.raute = 10;
				}
				
				arrow_left->sprite.col.r = SK_HIPAGE_R;
				arrow_left->sprite.col.g = SK_HIPAGE_G;
				arrow_left->sprite.col.b = SK_HIPAGE_B;
				arrow_left->sprite.col.a = SK_HIPAGE_A;
				pWork->arrow_left.raute = 10;
				// page
				pWork->page[ pWork->position ].r = SK_HIPAGE_R;
				pWork->page[ pWork->position ].g = SK_HIPAGE_G;
				pWork->page[ pWork->position ].b = SK_HIPAGE_B;
				pWork->page[ pWork->position ].raute = 10;
			}
		} else if ( ( pWork->pad_status & PAD_R1 ) ||
					( pWork->pad_status & PAD_R ) ||
					( pWork->pad_status & SELECT ) ){
			pWork->pad_status = 0;
			pWork->flag |= SK_LAST_PIC_SHOW; // 最後を表示して
			pWork->flag &= ~SK_DUMY_FADE;
			if ( pWork->page[ pWork->position ].position != pWork->page[ pWork->position ].max - 1 ) {
				BookPageSe( pWork , 1 );
				pWork->flag |= SK_PAGE_CHENGE;
				if ( pWork->page[ pWork->position ].position + HI_PAGE_READ < pWork->page[ pWork->position ].max - 1 ){
					pWork->page[ pWork->position ].position += HI_PAGE_READ;
				} else {
					pWork->page[ pWork->position ].position = pWork->page[ pWork->position ].max - 1;
				  	pWork->page[ pWork->position ].flag |= SK_READ_OK;
					pWork->arrow_right.r = SK_NO_NEXTPAGE_R;
					pWork->arrow_right.g = SK_NO_NEXTPAGE_G;
					pWork->arrow_right.b = SK_NO_NEXTPAGE_B;
					pWork->arrow_right.a = SK_NO_NEXTPAGE_A;
					pWork->arrow_right.raute = 10;
				}
				arrow_right->sprite.col.r = SK_HIPAGE_R;
				arrow_right->sprite.col.g = SK_HIPAGE_G;
				arrow_right->sprite.col.b = SK_HIPAGE_B;
				arrow_right->sprite.col.a = SK_HIPAGE_A;
				pWork->arrow_right.raute = 10;
				// page
				pWork->page[ pWork->position ].r = SK_HIPAGE_R;
				pWork->page[ pWork->position ].g = SK_HIPAGE_G;
				pWork->page[ pWork->position ].b = SK_HIPAGE_B;
				pWork->page[ pWork->position ].raute = 10;
			}
		}
	} else {
		if ( ( GV_PadDataDirect[ 0 ].press & PAD_L1 ) || ( pWork->pad_status & PAD_L1 ) ||
			 ( GV_PadDataDirect[ 0 ].press & PAD_L ) || ( pWork->pad_status & PAD_L ) ){
			pWork->pad_status = 0;
			pWork->flag &= ~SK_LAST_PIC_SHOW; // 最後を表示して
			pWork->flag &= ~SK_DUMY_FADE;
			if ( pWork->page[ pWork->position ].position > 0 ){
				BookPageSe( pWork , 0 );
				pWork->flag |= SK_PAGE_CHENGE;
				if ( pWork->page[ pWork->position ].position == pWork->page[ pWork->position ].max - 1 && pWork->position == 0 ){
					LastPageHide( pWork );
				}
				pWork->page[ pWork->position ].position--;
				if ( pWork->page[ pWork->position ].position > 0 ){
					// left
					arrow_left->sprite.col.r = SK_NO_NEXTPAGE_R;
					arrow_left->sprite.col.g = SK_NO_NEXTPAGE_G;
					arrow_left->sprite.col.b = SK_NO_NEXTPAGE_B;
					arrow_left->sprite.col.a = SK_NO_NEXTPAGE_A;
					pWork->arrow_left.r = SK_NEXTPAGE_R;
					pWork->arrow_left.g = SK_NEXTPAGE_G;
					pWork->arrow_left.b = SK_NEXTPAGE_B;
					pWork->arrow_left.a = SK_NEXTPAGE_A;
					pWork->arrow_left.raute = 10;
					// right
					pWork->arrow_right.r = SK_NEXTPAGE_R;
					pWork->arrow_right.g = SK_NEXTPAGE_G;
					pWork->arrow_right.b = SK_NEXTPAGE_B;
					pWork->arrow_right.a = SK_NEXTPAGE_A;
					pWork->arrow_right.raute = 10;
				} else {
					pWork->arrow_left.r = SK_NO_NEXTPAGE_R;
					pWork->arrow_left.g = SK_NO_NEXTPAGE_G;
					pWork->arrow_left.b = SK_NO_NEXTPAGE_B;
					pWork->arrow_left.a = SK_NO_NEXTPAGE_A;
					pWork->arrow_left.raute = 10;
				}
			}
		} else if ( ( GV_PadDataDirect[ 0 ].press & PAD_R1 ) || ( pWork->pad_status & PAD_R1 ) || 
					( GV_PadDataDirect[ 0 ].press & PAD_R ) || ( pWork->pad_status & PAD_R ) ||
					( GV_PadDataDirect[ 0 ].press & SELECT ) || ( pWork->pad_status & SELECT ) ){
			pWork->pad_status = 0;
			pWork->flag |= SK_LAST_PIC_SHOW; // 最後を表示して
			pWork->flag &= ~SK_DUMY_FADE;
			if ( pWork->page[ pWork->position ].position < pWork->page[ pWork->position ].max - 1 ){
				BookPageSe( pWork , 0 );
				pWork->flag |= SK_PAGE_CHENGE;
				pWork->page[ pWork->position ].position++;
				if ( pWork->page[ pWork->position ].position < pWork->page[ pWork->position ].max - 1 ){
					arrow_right->sprite.col.r = SK_NO_NEXTPAGE_R;
					arrow_right->sprite.col.g = SK_NO_NEXTPAGE_G;
					arrow_right->sprite.col.b = SK_NO_NEXTPAGE_B;
					arrow_right->sprite.col.a = SK_NO_NEXTPAGE_A;
					pWork->arrow_right.r = SK_NEXTPAGE_R;
					pWork->arrow_right.g = SK_NEXTPAGE_G;
					pWork->arrow_right.b = SK_NEXTPAGE_B;
					pWork->arrow_right.a = SK_NEXTPAGE_A;
					pWork->arrow_right.raute = 10;
					// left
					pWork->arrow_left.r = SK_NEXTPAGE_R;
					pWork->arrow_left.g = SK_NEXTPAGE_G;
					pWork->arrow_left.b = SK_NEXTPAGE_B;
					pWork->arrow_left.a = SK_NEXTPAGE_A;
					pWork->arrow_left.raute = 10;
				} else {
					pWork->arrow_right.r = SK_NO_NEXTPAGE_R;
					pWork->arrow_right.g = SK_NO_NEXTPAGE_G;
					pWork->arrow_right.b = SK_NO_NEXTPAGE_B;
					pWork->arrow_right.a = SK_NO_NEXTPAGE_A;
					pWork->arrow_right.raute = 10;
				}
			} else {
				pWork->arrow_left.r = SK_NO_NEXTPAGE_R;
				pWork->arrow_right.g = SK_NO_NEXTPAGE_G;
				pWork->arrow_right.b = SK_NO_NEXTPAGE_B;
				pWork->arrow_right.a = SK_NO_NEXTPAGE_A;
				pWork->page[ pWork->position ].flag |= SK_READ_OK;
			}
		}
	}
	// check
	if ( pWork->page[ pWork->position ].position == pWork->page[ pWork->position ].max - 1 ){
		pWork->page[ pWork->position ].flag |= SK_READ_OK;
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		switch( pWork->position ){
		case 0 : // 
			if ( pWork->page[ 0 ].flag & SK_READ_OK ){
				GM_TitleMenuStatus |= TITLE_MENU_LOOKED_PREV_STORY_1;
			}
			break;
		case 1 :
			if ( pWork->page[ 1 ].flag & SK_READ_OK ){
				GM_TitleMenuStatus |= TITLE_MENU_LOOKED_PREV_STORY_2;
			}
			break;
		}
		pWork->flag &= ~SK_SELECT_MODE;
		CloseBook( pWork );
		pWork->se_counter = DIRECT_TICK(30);
		pWork->flag |= SK_SE_COUNT;
		SE_CANCEL();
	}
	// auto mode
	pWork->pad_status = 0;
	if ( pWork->pad_check != GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 0 ] = 0;
		pWork->hold_time[ 1 ] = 0;
		pWork->pad_check = GV_PadDataDirect[ 0 ].status;
	}
	if ( pWork->hold_time[ 0 ] >= DIRECT_TICK( 30 ) ){
		pWork->pad_status = GV_PadDataDirect[ 0 ].status;
		pWork->hold_time[ 0 ] -= DIRECT_TICK( 4 );
		pWork->hold_time [ 1 ]++;
	} else if ( pWork->pad_check & GV_PadDataDirect[ 0 ].status ){
		pWork->hold_time[ 0 ]++;
	}
	if ( !( GV_PadDataDirect[ 0 ].status & ( PAD_L | PAD_R | PAD_L1 | PAD_R1 | SELECT ) ) ){
		pWork->hold_time[ 0 ] = 0;
		pWork->hold_time[ 1 ] = 0;
		pWork->pad_status = 0;
	}
}

static int PadAct( Work *pWork ) // マネージャ
{
	if ( pWork->flag & SK_NORMAL_MODE ){
	  if ( ( pWork->dot_hokan.flag & SK_MOVE_OK ) && ( pWork->flag & SK_SIGNAL_OK ) &&
		   ( pWork->book_fade[ 4 ].hokan.flag & SK_MOVE_OK ) && ( pWork->book_fade[ 5 ].hokan.flag & SK_MOVE_OK ) ){
		   
			PadControlNormal( pWork );
		}
		if ( pWork->flag & SK_END ){
			return ( -1 );
		}
		DotUpdate( pWork );
		MorfAct( &pWork->dot_hokan );
		return ( 0 );
	}
	if ( pWork->flag & SK_SELECT_MODE ){
		if ( !( pWork->flag & SK_STRING_FADE_IN ) || ( pWork->flag & SK_DUMY_FADE ) ){
			PadControlSelectMode( pWork );
			PageChenge( pWork );
		}
		ArrowControl( pWork );
	}
	BookDisp( pWork );
	return ( 0 );
}

static void MenuUpdate( Work *pWork )
{
	int code1_h , code2_h; // hilight 専用
	int code1_h_p , code2_h_p; // hilight 専用
	int strcode_h , strcode_h_p;
	int position_t , position_p;	

	code1_h = code2_h = 0;
	strcode_h = strcode_h_p = 0;

	position_t = position_p = 0;
	// set
	switch( pWork->position ){
	case 0 : //
		// txt 
		code1_h = STR_NEW_TXT_SEL;
		code2_h = STR_NEW_TXT_UNSEL;
		strcode_h = STR_KEY_NEWYORK;
		// pic
		code1_h_p = STR_NEW_PIC_SEL; 
		code2_h_p = STR_NEW_PIC_UNSEL; 
		strcode_h_p = STR_KEY_PIC_NEW;
		position_t = 3;
		position_p = 2;
		break;
	case 1 : // 
		code1_h = STR_SHOCK_TXT_SEL;
		code2_h = STR_SHOCK_TXT_UNSEL;
		strcode_h = STR_KEY_SHOCK;
		// pic
		code1_h_p = STR_SHOCK_PIC_SEL; 
		code2_h_p = STR_SHOCK_PIC_UNSEL; 
		strcode_h_p = STR_KEY_PIC_SHOCK;
		position_t = 5;
		position_p = 4;
		break;
	case 2 : // 
		code1_h = STR_DARK_TXT_SEL;
		code2_h = STR_DARK_TXT_UNSEL;
		strcode_h = STR_KEY_DARKNESS;
		// pic
		code1_h_p = STR_DARK_PIC_SEL; 
		code2_h_p = STR_DARK_PIC_UNSEL; 
		strcode_h_p = STR_KEY_PIC_DARKNESS;
		position_t = 7;
		position_p = 6;
		break;
	case 3 : // exit
		code1_h = STR_NEW_PIC_SEL; 
		code2_h = STR_NEW_PIC_UNSEL; 
		strcode_h = STR_KEY_PIC_NEW;
		position_t = 6; // 場所がないので
		break;
	default :
		return;
	}
	// exe
	MorfSet( &pWork->book_fade[ position_t ].hokan , strcode_h  , code2_h , code1_h , 16 , pWork->l2d[ 0 ].handle_2d );
//	MorfSet( &pWork->book_fade[ position_p ].hokan , strcode_h_p  , code2_h_p , code1_h_p , 10 , pWork->l2d[ 0 ].handle_2d );
	switch( pWork->old_position ){
	case 0 : //
		// txt 
		code1_h = STR_NEW_TXT_UNSEL;
		code2_h = STR_NEW_TXT_SEL;
		strcode_h = STR_KEY_NEWYORK;
		// pic
		code1_h_p = STR_NEW_PIC_UNSEL; 
		code2_h_p = STR_NEW_PIC_SEL; 
		strcode_h_p = STR_KEY_PIC_NEW;
		position_t = 3;
		position_p = 2;
		break;
	case 1 : // 
		code1_h = STR_SHOCK_TXT_UNSEL;
		code2_h = STR_SHOCK_TXT_SEL;
		strcode_h = STR_KEY_SHOCK;
		// pic
		code1_h_p = STR_SHOCK_PIC_UNSEL; 
		code2_h_p = STR_SHOCK_PIC_SEL; 
		strcode_h_p = STR_KEY_PIC_SHOCK;
		position_t = 5;
		position_p = 4;
		break;
	case 2 : // 
		code1_h = STR_DARK_TXT_UNSEL;
		code2_h = STR_DARK_TXT_SEL;
		strcode_h = STR_KEY_DARKNESS;
		// pic
		code1_h_p = STR_DARK_PIC_UNSEL; 
		code2_h_p = STR_DARK_PIC_SEL; 
		strcode_h_p = STR_KEY_PIC_DARKNESS;
		position_t = 7;
		position_p = 6;
		break;
	case 3 : // exit
		// txt 
		code1_h = STR_NEW_PIC_UNSEL; 
		code2_h = STR_NEW_PIC_SEL; 
		strcode_h = STR_KEY_PIC_NEW;
		position_t = 6;
		break;
	default :
		return;
	}
	// exe
	MorfSet( &pWork->book_fade[ position_t ].hokan , strcode_h    , code2_h   , code1_h   , 16 , pWork->l2d[ 0 ].handle_2d );
//	MorfSet( &pWork->book_fade[ position_p ].hokan , strcode_h_p  , code2_h_p , code1_h_p , 10 , pWork->l2d[ 0 ].handle_2d );
}

static void DotUpdate( Work *pWork )
{
	int code1 , code2;
	int morf_time;

	if ( ( pWork->position == pWork->old_position ) ){
		pWork->old_position = pWork->position;
		return;
	}
    pWork->dot_hokan.flag &= ~SK_MOVE_OK;
	code1 = code2 = 0;
	morf_time = 8;
	switch( pWork->position ){
	case 0 : // 
		code1 = STR_CURS1;
		pWork->now_story = SK_Story[ 0 ];
		if ( pWork->old_position == 3 ){
			morf_time = 24;
		}
		break;
	case 1 : // 
		code1 = STR_CURS2;
		pWork->now_story = SK_Story[ 1 ];
		if ( pWork->old_position == 3 ){
			morf_time = 16;
		}
		break;
	case 2 : // 
		code1 = STR_CURS3;
		pWork->now_story = SK_Story[ 2 ];
		if ( pWork->old_position == 3 ){
			morf_time = 12;
		}
		break;
	case 3 : // 
		code1 = STR_CURS4;
		pWork->now_story = 0;
		break;
	}
	switch( pWork->old_position ){
	case 0 : // 
		code2 = STR_CURS1;
		if ( pWork->position == 3 ){
			morf_time = 24;
		}
		break;
	case 1 : // 
		code2 = STR_CURS2;
		if ( pWork->position == 3 ){
			morf_time = 16;
		}
		break;
	case 2 : // 
		code2 = STR_CURS3;
		if ( pWork->position == 3 ){
			morf_time = 12;
		}
		break;
	case 3 : // 
		code2 = STR_CURS4;
		break;
	}
	MorfSet( &pWork->dot_hokan , STR_SELCURSNULL , code2 , code1 , morf_time , pWork->l2d[ 0 ].handle_2d );
	MenuUpdate( pWork );
	pWork->old_position = pWork->position;
}

static void PageChenge( Work *pWork )
{
	if ( pWork->flag & SK_PAGE_CHENGE ){
		pWork->msg.a = 0;
			pWork->msg.font = BP_GCL_LOOKUP_NEW_FONT_STRING(( char * )GetLocalResource( pWork->now_story , pWork->page[ pWork->position ].position ));
			MENU_ClearTextTexture( pWork->msg.work[ pWork->msg.clock ] );
			pWork->msg.clock = ( pWork->msg.clock ^ 0x1 ) & 0x1;
			pWork->flag |= SK_STRING_FADE_IN;
			pWork->flag &= ~SK_PAGE_CHENGE;
	}
}

#define SK_R (FONT_R)
#define SK_G (FONT_G)
#define SK_B (FONT_B)

static void ExitCoutrol( Work *pWork )
{
	int alpha;

	if ( pWork->flag & SK_EXIT_FADE_IN ){
		alpha = pWork->page[ pWork->position ].exit->sprite.col.a;	
		alpha += IN_FADE_SPEED;
		if ( alpha >= 128 ){
			alpha = 128;
			pWork->flag &= ~SK_EXIT_FADE_IN;
		}
		pWork->page[ pWork->position ].exit->sprite.col.a = ( u_char )alpha;
		SPR_SHOW( pWork->page[ pWork->position ].exit );
	}
	if ( pWork->flag & SK_EXIT_FADE_OUT ){
		alpha = pWork->page[ pWork->position ].exit->sprite.col.a;	
		alpha -= OUT_FADE_SPEED;
		if ( alpha <= 0 ){
			alpha = 0;
			pWork->flag &= ~SK_EXIT_FADE_OUT;
		}
		pWork->page[ pWork->position ].exit->sprite.col.a = ( u_char )alpha;
		SPR_SHOW( pWork->page[ pWork->position ].exit );
	}
}

static void PageUpDate( Work *pWork ) // num -> ascci そして 入力
{
	float diff;
	char  buf[ PAGE_FONT_MAX + 1];
	int   r , g , b;
	int   i;
	
	sprintf( buf , "%03d/%03d" , pWork->page[ pWork->position ].position + 1 , pWork->page[ pWork->position ].max );

	SK_PrintfNormal( NULL , pWork->page[ pWork->position ].now , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_FONT_MAX );
	SK_PrintfNormal( buf , pWork->page[ pWork->position ].now , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PAGE_FONT_MAX );
	// 位置補正
	diff = DEFAULT_PAGE_SURASHU_X - pWork->page[ pWork->position ].now[ 3 ]->sprite.pos.x; // スラッシュの場所出し
	if ( diff != 0 ){ // 補正
		for ( i = 0 ; i < PAGE_FONT_MAX ; i ++ ){
			if ( i == 3 ){ // スラッシュ
				pWork->page[ pWork->position ].now[ i ]->sprite.pos.x = DEFAULT_PAGE_SURASHU_X;
			} else {
				pWork->page[ pWork->position ].now[ i ]->sprite.pos.x += diff;
			}
		}
	}
	SK_PrintfChengColor2( pWork->page[ pWork->position ].now , SK_R , SK_G , SK_B , 128 , PAGE_FONT_MAX );
	SK_PrintfChengColor2( pWork->page[ pWork->position ].now , 
						  pWork->page[ pWork->position ].r , pWork->page[ pWork->position ].g , pWork->page[ pWork->position ].b ,
						  128 , 3 );
	SK_PrintfChengColor2( pWork->page[ pWork->position ].font , SK_R , SK_G , SK_B , 128 , PAGE_MAX );

	SK_AllShow( pWork->page[ pWork->position ].now , PAGE_FONT_MAX );
	SK_AllShow( pWork->page[ pWork->position ].font , PAGE_MAX );
	// r
	r = pWork->page[ pWork->position ].r;
	r += ( SK_R - r ) / pWork->page[ pWork->position ].raute;
	pWork->page[ pWork->position ].r = ( u_char )r;
	// g
	g = pWork->page[ pWork->position ].g;
	g += ( SK_G - g ) / pWork->page[ pWork->position ].raute;
	pWork->page[ pWork->position ].g = ( u_char )g;
	// b
	b = pWork->page[ pWork->position ].b;
	b += ( SK_B - b ) / pWork->page[ pWork->position ].raute;
	pWork->page[ pWork->position ].b = ( u_char )b;
	if ( pWork->page[ pWork->position ].raute > 1 ){
		pWork->page[ pWork->position ].raute--;
	}
}

static void BlackRectControl( Work *pWork )
{
	SPR_OBJ *spr;
	int alpha;

	spr = L2D_GetObject( pWork->l2d[ 0 ].handle_2d , STR_ROOT ); // menu l2d
	SPR_HIDE( spr );
	if ( pWork->flag & SK_BLACK_RECT_FADE_OUT ){
		alpha = pWork->black.obj->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < 0 ){
			alpha = 0;
			pWork->black.obj->sprite.col.a = 128;
			SPR_HIDE( pWork->black.obj );
			pWork->flag &= ~SK_BLACK_RECT_FADE_OUT;
			pWork->flag &= ~SK_POSITION_CHENGE;
			return;
		} 
		pWork->black.obj->sprite.col.a = ( u_char )alpha;

		SPR_SHOW( pWork->black.obj );
	}
	if ( pWork->flag & SK_BLACK_RECT_FADE_IN ){ // キャンセル時
		alpha = pWork->black.obj->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha > 128 ){
			alpha = 128;
			pWork->flag &= ~SK_BLACK_RECT_FADE_IN;
			pWork->flag &= ~SK_SIGNAL_OK;
			pWork->l2d[ 0 ].action = STR_OPENSPECIAL;
			pWork->l2d[ 0 ].flag |= SK_ACTION_START;
			// rerese
			if ( pWork->l2d[ 2 ].handle_2d >= 0 ){
				L2D_ReleaseLayout( pWork->l2d[ 2 ].handle_2d );
				pWork->l2d[ 2 ].handle_2d = -1;
			}
			if ( pWork->l2d[ 3 ].handle_2d >= 0 ){
				L2D_ReleaseLayout( pWork->l2d[ 3 ].handle_2d );
				pWork->l2d[ 3 ].handle_2d = -1;
			}
			SPR_HIDE( pWork->black.obj );
			return;
		} 
		pWork->black.obj->sprite.col.a = ( u_char )alpha;
		SPR_SHOW( pWork->black.obj );
	}
}

static void BlackRectControlSub( Work *pWork ) // sub
{
	int alpha;

	if ( pWork->flag & SK_BLACK_RECT_FADE_OUT_SUB ){
		alpha = pWork->last_pic.black->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha < 0 ){
			alpha = 0;
			pWork->last_pic.black->sprite.col.a = 0;
			pWork->last_pic.pic_top->sprite.col.a = 0;
			pWork->last_pic.pic_bottom->sprite.col.a = 0;
			SPR_HIDE( pWork->last_pic.black );
			pWork->flag &= ~SK_BLACK_RECT_FADE_OUT_SUB;
			return;
		} 
		pWork->last_pic.black->sprite.col.a = ( u_char )alpha;
		pWork->last_pic.pic_top->sprite.col.a = ( u_char )alpha;
		pWork->last_pic.pic_bottom->sprite.col.a = ( u_char )alpha;

		SPR_SHOW( pWork->last_pic.black );
	}
	if ( pWork->flag & SK_BLACK_RECT_FADE_IN_SUB ){ // キャンセル時
		alpha = pWork->last_pic.black->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha > 128 ){
			alpha = 128;
			pWork->flag &= ~SK_BLACK_RECT_FADE_IN_SUB;
			pWork->last_pic.black->sprite.col.a = 128;
			pWork->last_pic.pic_top->sprite.col.a = 128;
			pWork->last_pic.pic_bottom->sprite.col.a = 128;
			return;
		} 
		pWork->last_pic.black->sprite.col.a = ( u_char )alpha;
		pWork->last_pic.pic_top->sprite.col.a = ( u_char )alpha;;
		pWork->last_pic.pic_bottom->sprite.col.a = ( u_char )alpha;;
		SPR_SHOW( pWork->last_pic.black );
	}
}

static void BookDisp( Work *pWork )
{
	if ( !( pWork->flag & SK_POSITION_CHENGE ) || !( pWork->flag & SK_SPRITE_INIT ) ){
		return;
	}
	BlackRectControl( pWork );
}


/* 日本語サイズ */
#define J_MSG_WIDTH 		(442)
#define J_MSG_HEIGHT 		(270)
#define J_MSG_LINE_X		(25)
#define J_MSG_LINE_Y  	(10)
#define J_MSG_TEX_PITCH  (1)
#define J_MSG_TEX_SPACE  (12)
#define J_MSG_TEX_WIDTH  (J_MSG_LINE_X*(FONT_SIZE_W+J_MSG_TEX_PITCH)+5)
#define J_MSG_TEX_HEIGHT (J_MSG_LINE_Y*(FONT_SIZE_H+J_MSG_TEX_SPACE)+DOWN_MARGINE+10)

/* 仏、独、伊、スペイン語のサイズ */
#define EU_MSG_WIDTH 		(440)
#define EU_MSG_HEIGHT 		(270)
#define EU_MSG_LINE_X		(25)
#define EU_MSG_LINE_Y  	(10)
#define EU_MSG_TEX_PITCH  (1)
#define EU_MSG_TEX_SPACE  (12)
#define EU_MSG_TEX_WIDTH  (650)
#define EU_MSG_TEX_HEIGHT (EU_MSG_LINE_Y*(FONT_SIZE_H+EU_MSG_TEX_SPACE)+DOWN_MARGINE+10)

/* 英語サイズ */
#define E_MSG_WIDTH 		(440)
#define E_MSG_HEIGHT 		(270)
#define E_MSG_LINE_X		(25)
#define E_MSG_LINE_Y  	(10)
#define E_MSG_TEX_PITCH  (1)
#define E_MSG_TEX_SPACE  (12)
#define E_MSG_TEX_WIDTH  (E_MSG_LINE_X*(FONT_SIZE_W+E_MSG_TEX_PITCH))
#define E_MSG_TEX_HEIGHT (E_MSG_LINE_Y*(FONT_SIZE_H+E_MSG_TEX_SPACE)+DOWN_MARGINE+10)


#define MSG_WIDTH 		(pWork->msg_width)
#define MSG_HEIGHT 		(pWork->msg_height)
#define MSG_TEX_WIDTH	(pWork->msg_tex_width)
#define MSG_TEX_HEIGHT	(pWork->msg_tex_height)

static void StringCreate( Work *pWork )
{
	int poly_width;
	int poly_height;

	poly_width  = MSG_TEX_WIDTH;
	poly_height = MSG_TEX_HEIGHT;
	// trans
   if (GM_Language != GM_LANG_JAPANESE)
   {
      MENU_SetTextSpace(pWork->msg.work[ pWork->msg.clock ], 8);
   }
	MENU_CreateTextTexture( pWork->msg.work[ pWork->msg.clock ] , 0 ,  0 , poly_width , poly_height ,
							1 , 12, 0 , pWork->msg.font );
}

static void StringFade( Work *pWork )
{
	int alpha;

	if ( pWork->flag & SK_STRING_FADE_IN ){
		alpha = pWork->msg.a;		
		alpha += IN_FADE_SPEED;
		if ( alpha >= 128 ){
			alpha = 128;
			pWork->flag &= ~SK_STRING_FADE_IN;
		}
		pWork->msg.a = alpha;
	}
}

static void StringDisp( Work *pWork )
{
	float ftmp;
	u_int color;
	int poly_width;
	int poly_height;
	float   x1 , y1 , x2 , y2 , u1 , v1; 

	ftmp = 384.0f / 448.0f;

#if 0 ///
	poly_width = ( MSG_LINE_X ) * SYS_FONT_WIDTH;
	poly_height = MSG_LINE_Y * SYS_FONT_HEIGHT;
#else
	poly_width  = MSG_TEX_WIDTH;
	poly_height = MSG_TEX_HEIGHT;
#endif

   x1 = MSG_BASE_X;
   y1 = MSG_BASE_Y;
   x2 = x1 + MSG_WIDTH*9/10;
   y2 = y1 + MSG_HEIGHT*10/9;
	u1 = poly_width;
	v1 = poly_height;

   // color
	color = pWork->msg.r | ( pWork->msg.g << 8 ) | ( pWork->msg.b << 16 ) | ( pWork->msg.a << 24 ); 
#if 0 ///
	MENU_PutTextScreenF( pWork->msg.work[ pWork->msg.clock ], x1 , y1 , x2 , y2 ,
						1 , 0 , poly_width + 1 , poly_height + 1 ,  color );
#else
	MENU_PutTextScreenF( pWork->msg.work[ pWork->msg.clock ], x1 , y1 , x2 , y2 ,
						1 , 0 , poly_width , poly_height ,  color );
#endif

#if 0
	if ( flag ) {
		color = pWork->msg.r | ( pWork->msg.g << 8 ) | ( pWork->msg.b << 16 ) | ( ( pWork->msg.a / 2 ) << 24 ); 
		MENU_PutTextScreenF( pWork->msg.work[ pWork->msg.clock ], ( x1 + 0.5f ), ( y1 + 0.5f ) , x2 , y2 ,
							0 , 12 , poly_width , poly_height + 1 ,  color );
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_A ){
		if ( flag == 0x0 ){
			flag = 0x1;
		} else {
			flag = 0x0;
		}
	}
#endif
}

static void NormalAct( Work *pWork )
{
	int i;

	if ( SprInit( pWork ) < 0 ){
		return;
	}
	AnimationAct( pWork );
	if ( pWork->flag & SK_END ){ // morf だけはする
		for ( i = 0 ; i < MAX_BOOK_FADE ; i ++ ){
			MorfAct( &pWork->book_fade[ i ].hokan );
		}
		return;
	}
	if ( pWork->flag & SK_BOOK_SHOW ){
		pWork->book_count--;
		if ( pWork->book_count <= 0 ){
			OpenBook( pWork );
			pWork->flag &= ~SK_BOOK_SHOW;
		}
	} 
	if ( PadAct( pWork ) < 0 ){
		return;
	}
	if ( pWork->flag & SK_SPRITE_INIT && pWork->flag & SK_SELECT_MODE ){
		PageUpDate( pWork );
		ExitCoutrol( pWork );
		StringCreate( pWork );
		StringDisp( pWork );
		StringFade( pWork );
	} 
	for ( i = 0 ; i < MAX_BOOK_FADE ; i ++ ){
		MorfAct( &pWork->book_fade[ i ].hokan );
	}
	for ( i = 0 ; i < pWork->max_position ; i ++ ){
		MorfAct( &pWork->menu[ i ].hokan[ 0 ] );
		MorfAct( &pWork->menu[ i ].hokan[ 1 ] );
	}
	if ( pWork->flag & SK_SE_COUNT ){
		if ( pWork->se_counter <= 0 ){
			BookCloseSe( pWork );
			pWork->flag &= ~SK_SE_COUNT;
		} else {
			pWork->se_counter--;
		}
	}
}

static void Act( Work *pWork )
{
 	pWork->act( pWork );
	BlackRectControlSub( pWork );
}

static	void	Die( Work *pWork )
{
	int i;

	for ( i = 0 ; i < MAX_L2D ; i ++ ){
		if ( pWork->l2d[ i ].handle_2d >= 0 ){
			L2D_ReleaseLayout( pWork->l2d[ i ].handle_2d );
		}
	}
	if ( pWork->black.obj != NULL ){
		SPR_Destroy_2D_Object( pWork->black.obj );	
	}
	if ( pWork->last_pic.black != NULL ){
		SPR_Destroy_2D_Object( pWork->last_pic.black );	
	}
	if ( pWork->last_pic.pic_top != NULL ){
		SPR_Destroy_2D_Object( pWork->last_pic.pic_top );	
	}
	if ( pWork->last_pic.pic_bottom != NULL ){
		SPR_Destroy_2D_Object( pWork->last_pic.pic_bottom );	
	}
	for ( i = 0 ; i < 3 ; i ++ ){
		SK_Story[ i ] = 0;
	}
	// sound
	GM_SdSet( SNG_SYUKAN_OFF );

#if 1 ///test
	if( pWork->line != NULL ){
		SPR_Destroy_2D_Object( pWork->line );
	}
#endif
}

// action strcode
#define STR_OPENBOOK (352028)
#define STR_IDLEBOOK (7679017)
#define STR_CLOSEBOOK (12142327)
#define STR_OPENBOOK_BG (9407318)
#define STR_IDLEBOOK_BG (1459005)
#define STR_OPENSPECIAL (15448060)
static void OpenBook( Work *pWork )
{
	pWork->l2d[ 2 ].action = STR_OPENBOOK;
	pWork->l2d[ 2 ].flag |= SK_ACTION_START;
	pWork->l2d[ 2 ].flag &= ~SK_ACTION_OK;

	pWork->flag &= ~SK_NORMAL_MODE;
}

static void CloseBook( Work *pWork )
{
	pWork->l2d[ 1 ].action = STR_CLOSEBOOK;
	pWork->l2d[ 1 ].flag |= SK_ACTION_START;
	pWork->l2d[ 1 ].flag &= ~SK_ACTION_OK;
	pWork->flag &= ~SK_SELECT_MODE;
}

static void AnimationAct( Work *pWork )
{
	int status;
	int      i;

	for ( i = 0 ; i < MAX_L2D ; i ++ ){
		status = L2D_ActionStatus( pWork->l2d[ i ].handle_2d );
		if ( status == L2D_STAT_UNLOAD ){
			continue;
		}
		if ( ( status != L2D_STAT_BUSY ) && ( pWork->l2d[ i ].flag & SK_ACTION_OK ) ){
			switch ( pWork->l2d[ i ].action ){
			case STR_DEFAULT :
				if ( i == 0 ){
					pWork->l2d[ i ].action = STR_OPENSPECIAL;
					pWork->l2d[ i ].flag |= SK_ACTION_START;
					pWork->l2d[ i ].flag &= ~SK_ACTION_OK;
					pWork->flag &= ~SK_SIGNAL_OK;
				}
				break;
			case STR_OPENBOOK : // 本を開く
				pWork->l2d[ i ].action = STR_IDLEBOOK;
				pWork->l2d[ i ].flag |= SK_ACTION_START;
				break;
			case STR_IDLEBOOK : // アイドル
				pWork->l2d[ i ].action = STR_IDLEBOOK;
				pWork->flag |= SK_SELECT_MODE | SK_ACTION_START;
				break;
			case STR_CLOSEBOOK : // 本を閉じる
				pWork->l2d[ i ].action = STR_DEFAULT;
				pWork->l2d[ i ].flag |= SK_ACTION_START;
				pWork->l2d[ i ].flag &= ~SK_ACTION_OK;
				pWork->page[ pWork->position ].position = 0;
				// rerese
				SK_AllHide( pWork->page[ pWork->position ].now , PAGE_FONT_MAX );
				SK_AllHide( pWork->page[ pWork->position ].font , PAGE_MAX );
				// menu
				pWork->l2d[ 0 ].action = STR_DEFAULT;
				pWork->l2d[ 0 ].flag |= SK_ACTION_START;
				pWork->l2d[ 0 ].flag &= ~SK_ACTION_OK;
				// rerese
				if ( pWork->l2d[ 2 ].handle_2d >= 0 ){
					L2D_ReleaseLayout( pWork->l2d[ 2 ].handle_2d );
					pWork->l2d[ 2 ].handle_2d = -1;
				}
				if ( pWork->l2d[ 3 ].handle_2d >= 0 ){
					L2D_ReleaseLayout( pWork->l2d[ 3 ].handle_2d );
					pWork->l2d[ 3 ].handle_2d = -1;
				}
				for( i = 0 ; i < MAX_BOOK_FADE ; i ++ ) {
					MorfSet( &pWork->book_fade[ i ].hokan , -1 , 0 , 0 , 0 , -1 );
					pWork->book_fade[ i ].hokan.flag = SK_MOVE_OK;
				}
				// buffer clear
				MENU_ClearTextTexture( pWork->msg.work[ pWork->msg.clock ] );
				MENU_ClearTextTexture( pWork->msg.work[ 1 - pWork->msg.clock ] );
				// 宣伝
				LastPageInit( pWork );
				// flag
				pWork->flag = SK_ACTION_OK;
				pWork->flag &= ~SK_SPRITE_INIT;
				break;
			case STR_OPENBOOK_BG : // バックグラウンド
				pWork->l2d[ i ].action = STR_IDLEBOOK_BG;
				pWork->l2d[ i ].flag |= SK_ACTION_START;
				break;
			case STR_IDLEBOOK_BG : // バックグラウンド
				pWork->l2d[ i ].action = STR_IDLEBOOK_BG;
				pWork->l2d[ i ].flag |= SK_ACTION_START;
				break;
			case STR_OPENSPECIAL : // バックグラウンド
				pWork->flag |= SK_NORMAL_MODE;
				break;
			case STR_CLOSESPECIAL : // 終了
				if ( pWork->proc_prev != 0 ){
					GM_ExecProc( pWork->proc_prev , NULL );
					pWork->proc_prev = 0;
				}
				GV_DestroyActor( pWork );
				return;
			}
			pWork->l2d[ i ].flag &= ~SK_ACTION_OK;
		}
		if ( status != L2D_STAT_BUSY && pWork->l2d[ i ].flag & SK_ACTION_START ){
			if ( status == L2D_STAT_ACK ){
				L2D_BreakAction( pWork->l2d[ i ].handle_2d );
				L2D_EvokeAction( pWork->l2d[ i ].handle_2d , pWork->l2d[ i ].action );
				pWork->l2d[ i ].flag |= SK_ACTION_OK;
				pWork->l2d[ i ].flag &= ~SK_ACTION_START;
			}
		}
	}
}

// モーフィングを設定してくれる関数
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan )
{
	void *parts;

	parts = L2D_GetParts( handle , strcode );    /* パーツポインタの取得 */

	if ( parts == NULL ){
		return;
	}
	hokan->p += ( 1.0f - hokan->p ) / hokan->count;
	hokan->count--;

	L2D_MorfObject( parts , code1 , code2 , hokan->p );
	if ( ( hokan->p == 1.0f ) || ( hokan->count == 0 ) ){
		hokan->flag = SK_MOVE_OK;
	}
}

static inline int MorfAct( Hokan *pHokan ) // これのみ実行
{
	if ( !( pHokan->flag & SK_MOVE_OK ) ){
		MorfL2d( pHokan->handle , pHokan->morf_parts  , pHokan->code1  , pHokan->code2  , pHokan );
	} else {
		return ( 0 );
	}
	return( 1 );
}

static inline void MorfSet( Hokan *pHokan , int parts , int code1 , int code2 , int value , int handle )
{
	pHokan->morf_parts = parts;
	pHokan->count = value;
	pHokan->code1 = code1;
	pHokan->code2 = code2;
	pHokan->p    = 0.0f;
	pHokan->flag = 0;
	pHokan->handle = handle;
}

/*----------------------------------------------------------------*/
#define SIG_MSG_RECT_OPEN (8755684)
#define SIG_HIDE_MOSES_A (5022231)
#define SIG_HIDE_MOSES_B (5022232)
#define SIG_HIDE_MOSES_C (5022233)
#define STR_HIDE_MSG_RECT (3398789)
#define STR_SHOW_MSG_RECT (7638067)
#define SIG_SHOW_MENU (10212634)
#define SIG_SEON_MOSES_A (11904904)
#define SIG_SEON_MOSES_B (11904905)
#define SIG_SEON_MOSES_C (11904906)
#define SIG_WINDOW_OPEN1 (5338536)
#define SIG_WINDOW_OPEN2 (5338537)
#define SIG_WINDOW_CLOSE1 (15978402)
#define SIG_WINDOW_CLOSE2 (15978403)
static void Signal( void *work , int sign , int value )
{
	Work *pWork;

	pWork = ( Work * )work;

	switch ( sign ){
	case SIG_SEON_MOSES_A : 
		SE_NPAP();
		break;
	case SIG_SEON_MOSES_B : 
		SE_BOOK1();
		break;
	case SIG_SEON_MOSES_C : 
		SE_BOOK2();
		break;
	case SIG_WINDOW_OPEN1 :
	case SIG_WINDOW_CLOSE1 :
		SE_MOV();
		break;
	case SIG_WINDOW_OPEN2 :
		SE_WINOPN();
		break;
	case SIG_WINDOW_CLOSE2 :
		SE_WINCLS();
		break;
	case SIG_SHOW_MENU : // menu disp
		pWork->old_position = pWork->position;
		value = 20;
		pWork->max_position = 1;
		if ( GM_TitleMenuStatus & TITLE_MENU_LOOKED_PREV_STORY_1 ){
			pWork->max_position = 2;
		}
		if ( GM_TitleMenuStatus & TITLE_MENU_LOOKED_PREV_STORY_2 ){
			pWork->max_position = 3;
		}
		MorfSet( &pWork->book_fade[ 0 ].hokan , STR_SELCURS , STR_HIDECURS , STR_SHOWCURS , value , pWork->l2d[ 0 ].handle_2d );
		switch ( pWork->position ){
		case 0 :
			MorfSet( &pWork->book_fade[ 1 ].hokan , STR_SELCURSNULL , STR_CURS1 , STR_CURS1 , 1 , pWork->l2d[ 0 ].handle_2d );
//			MorfSet( &pWork->book_fade[ 2 ].hokan , STR_KEY_PIC_NEW , STR_NEW_PIC_HIDE , STR_NEW_PIC_SEL , value , pWork->l2d[ 0 ].handle_2d );
			MorfSet( &pWork->book_fade[ 3 ].hokan , STR_KEY_NEWYORK , STR_NEW_TXT_HIDE , STR_NEW_TXT_SEL , value , pWork->l2d[ 0 ].handle_2d );
			switch ( pWork->max_position ){
			case 3 :
//				MorfSet( &pWork->book_fade[ 6 ].hokan , STR_KEY_PIC_DARKNESS , STR_DARK_PIC_HIDE , STR_DARK_PIC_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
				MorfSet( &pWork->book_fade[ 7 ].hokan , STR_KEY_DARKNESS , STR_DARK_TXT_HIDE , STR_DARK_TXT_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
			case 2 : 
//				MorfSet( &pWork->book_fade[ 4 ].hokan , STR_KEY_PIC_SHOCK , STR_SHOCK_PIC_HIDE , STR_SHOCK_PIC_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
				MorfSet( &pWork->book_fade[ 5 ].hokan , STR_KEY_SHOCK , STR_SHOCK_TXT_HIDE , STR_SHOCK_TXT_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
				break;
			}
			break;
		case 1 :
			MorfSet( &pWork->book_fade[ 1 ].hokan , STR_SELCURSNULL , STR_CURS2 , STR_CURS2 , 1 , pWork->l2d[ 0 ].handle_2d );
//			MorfSet( &pWork->book_fade[ 2 ].hokan , STR_KEY_PIC_SHOCK , STR_SHOCK_PIC_HIDE , STR_SHOCK_PIC_SEL , value , pWork->l2d[ 0 ].handle_2d );
			MorfSet( &pWork->book_fade[ 3 ].hokan , STR_KEY_SHOCK , STR_SHOCK_TXT_HIDE , STR_SHOCK_TXT_SEL , value , pWork->l2d[ 0 ].handle_2d );
			// 非撰択
//			MorfSet( &pWork->book_fade[ 4 ].hokan , STR_KEY_PIC_NEW , STR_NEW_PIC_HIDE , STR_NEW_PIC_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
			MorfSet( &pWork->book_fade[ 5 ].hokan , STR_KEY_NEWYORK , STR_NEW_TXT_HIDE , STR_NEW_TXT_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
			switch ( pWork->max_position ){
			case 3 :
//				MorfSet( &pWork->book_fade[ 6 ].hokan , STR_KEY_PIC_DARKNESS , STR_DARK_PIC_HIDE , STR_DARK_PIC_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
				MorfSet( &pWork->book_fade[ 7 ].hokan , STR_KEY_DARKNESS , STR_DARK_TXT_HIDE , STR_DARK_TXT_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
				break;
			}
			break;
		case 2 :
			MorfSet( &pWork->book_fade[ 1 ].hokan , STR_SELCURSNULL , STR_CURS3 , STR_CURS3 , 1 , pWork->l2d[ 0 ].handle_2d );
//			MorfSet( &pWork->book_fade[ 2 ].hokan , STR_KEY_PIC_DARKNESS , STR_DARK_PIC_HIDE , STR_DARK_PIC_SEL , value , pWork->l2d[ 0 ].handle_2d );
			MorfSet( &pWork->book_fade[ 3 ].hokan , STR_KEY_DARKNESS , STR_DARK_TXT_HIDE , STR_DARK_TXT_SEL , value , pWork->l2d[ 0 ].handle_2d );
			// 非撰択
//			MorfSet( &pWork->book_fade[ 4 ].hokan , STR_KEY_PIC_NEW , STR_NEW_PIC_HIDE , STR_NEW_PIC_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
			MorfSet( &pWork->book_fade[ 5 ].hokan , STR_KEY_NEWYORK , STR_NEW_TXT_HIDE , STR_NEW_TXT_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
//			MorfSet( &pWork->book_fade[ 6 ].hokan , STR_KEY_PIC_SHOCK , STR_SHOCK_PIC_HIDE , STR_SHOCK_PIC_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
			MorfSet( &pWork->book_fade[ 7 ].hokan , STR_KEY_SHOCK , STR_SHOCK_TXT_HIDE , STR_SHOCK_TXT_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
			break;
		}
		MorfSet( &pWork->book_fade[ 2 ].hokan , STR_KEY_PIC_NEW , STR_NEW_PIC_HIDE , STR_NEW_PIC_UNSEL , value , pWork->l2d[ 0 ].handle_2d );
		pWork->flag |= SK_SIGNAL_OK;
		break;
	case SIG_HIDE_MOSES_B : // 本を消す
#if 0
		MorfSet( &pWork->book_fade[ 0 ].hokan , STR_KEY_TEXRECT_4 , STR_SHOW_MOSES_B5 , STR_HIDE_MOSES_B5 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 1 ].hokan , STR_KEY_TEXRECT_6 , STR_SHOW_MOSES_B12 , STR_HIDE_MOSES_B12 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 2 ].hokan , STR_KEY_TEXRECT_7 , STR_SHOW_MOSES_B11 , STR_HIDE_MOSES_B11 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 3 ].hokan , STR_KEY_TEXRECT_8 , STR_SHOW_MOSES_B10 , STR_HIDE_MOSES_B10 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 4 ].hokan , STR_KEY_TEXRECT_9 , STR_SHOW_MOSES_B9 , STR_HIDE_MOSES_B9 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 5 ].hokan , STR_KEY_TEXRECT_10 , STR_SHOW_MOSES_B8 , STR_HIDE_MOSES_B8 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 6 ].hokan , STR_KEY_TEXRECT_11 , STR_SHOW_MOSES_B7 , STR_HIDE_MOSES_B7 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 7 ].hokan , STR_KEY_FILLRECT_12 , STR_SHOW_MOSES_B6 , STR_HIDE_MOSES_B6 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 8 ].hokan , STR_KEY_FILLRECT_13 , STR_SHOW_MOSES_B4 , STR_HIDE_MOSES_B4 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 9 ].hokan , STR_KEY_FILLRECT_14 , STR_SHOW_MOSES_B3 , STR_HIDE_MOSES_B3 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 10 ].hokan , STR_KEY_FILLRECT_15 , STR_SHOW_MOSES_B2 , STR_HIDE_MOSES_B2 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 11 ].hokan , STR_KEY_FILTER      , STR_SHOW_MOSES_B1 , STR_HIDE_MOSES_B1 , value , pWork->l2d[ 2 ].handle_2d );
#endif
		break;
	case SIG_HIDE_MOSES_C : // 本を消す
#if 0
		MorfSet( &pWork->book_fade[ 0 ].hokan , STR_KEY_FACE_A , STR_SHOW_MOSES_C14 , STR_HIDE_MOSES_C14 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 1 ].hokan , STR_KEY_FACE_B , STR_SHOW_MOSES_C13 , STR_HIDE_MOSES_C13 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 2 ].hokan , STR_KEY_FACE_C , STR_SHOW_MOSES_C12 , STR_HIDE_MOSES_C12 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 3 ].hokan , STR_KEY_FACE_D , STR_SHOW_MOSES_C11 , STR_HIDE_MOSES_C11 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 4 ].hokan , STR_KEY_FACE_E , STR_SHOW_MOSES_C10 , STR_HIDE_MOSES_C10 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 5 ].hokan , STR_KEY_TEXRECT_2 , STR_SHOW_MOSES_C9 , STR_HIDE_MOSES_C9 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 6 ].hokan , STR_KEY_TEXRECT_3 , STR_SHOW_MOSES_C8 , STR_HIDE_MOSES_C8 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 7 ].hokan , STR_KEY_TEXRECT_4 , STR_SHOW_MOSES_C7 , STR_HIDE_MOSES_C7 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 8 ].hokan , STR_KEY_TEXRECT_5 , STR_SHOW_MOSES_C6 , STR_HIDE_MOSES_C6 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 9 ].hokan , STR_KEY_TEXRECT_6 , STR_SHOW_MOSES_C5 , STR_HIDE_MOSES_C5 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 10 ].hokan , STR_KEY_TEXRECT_7 , STR_SHOW_MOSES_C4 , STR_HIDE_MOSES_C4 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 11 ].hokan , STR_KEY_TEXRECT_8 , STR_SHOW_MOSES_C3 , STR_HIDE_MOSES_C3 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 12 ].hokan , STR_KEY_TEXRECT_9 , STR_SHOW_MOSES_C2 , STR_HIDE_MOSES_C2 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 13 ].hokan , STR_FILTER , STR_SHOW_MOSES_C1 , STR_HIDE_MOSES_C1 , value , pWork->l2d[ 2 ].handle_2d );
		MorfSet( &pWork->book_fade[ 14 ].hokan , STR_FILLRECT_1 , STR_SHOW_MOSES_C15 , STR_HIDE_MOSES_C15 , value , pWork->l2d[ 2 ].handle_2d );
#endif
		break;
	}
}

static int LayoutSet( Work *pWork , int strcode , int strcode2 ) // 3 , 4 にセット
{
	int handle;

	handle = L2D_LoadLayout2( strcode , DG_CHANL_MENU , 0, 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		printf("handle %d\n" , handle );
		SK_Err("layout set book\0");
		return -1 ;
	}
	// default action
	L2D_EvokeAction( handle , STR_DEFAULT );

	pWork->l2d[ 2 ].handle_2d = handle;
	pWork->l2d[ 2 ].action = 0;
	pWork->l2d[ 2 ].flag = 0;

	handle = L2D_LoadLayout2( strcode2 , DG_CHANL_MENU , 0, 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		printf("handle %d\n" , handle );
		SK_Err("layout set 2book\0");
		return -1 ;
	}
	// default action
	L2D_EvokeAction( handle , STR_DEFAULT );

	pWork->l2d[ 3 ].handle_2d = handle;
	pWork->l2d[ 3 ].action = 0;
	pWork->l2d[ 3 ].flag = 0;

	// l2d Signal
	L2D_SetSignalHandle( pWork->l2d[ 2 ].handle_2d , pWork , Signal );
	// l2d Signal
	L2D_SetSignalHandle( pWork->l2d[ 3 ].handle_2d , pWork , Signal );

	// flag
	pWork->flag |= SK_SPRITE_INIT_START;

	return (0);
}

#if 0
#ifdef JAPANESE_BP_IGNORE()
	static int MaxPage[] = { 8 , 136 , 269 };
#else
	static int MaxPage[] = { 11 , 129 , 324 };
#endif
#endif

static	int	GetResources( Work *work )
{
	static int strcode_layout[] = { STR_PRESTMENU , STR_MOSES_CLOSE };
	int max_page[ 3 ];
	int		handle;
	int          i;

	// Page数の取得
	GCL_GetOption( 'e' );
	for( i = 0 ; i < 3 ; i ++ ){
		max_page[ i ] = GCL_GetNextInt();
	}
	LangChecge();
	// l2d
	for ( i = 0 ; i < MAX_L2D ; i ++ ){
		if ( i < 2 ){
			handle = -1 ;
			handle = L2D_LoadLayout2( strcode_layout[ i ] , DG_CHANL_MENU , 0, 0 , GV_PAUSE_STOP ) ;
			if ( handle < 0 ){
				printf("i = %d \n" , i );
				printf("handle %d\n" , handle );
				SK_Err("book\0");
				return -1 ;
			}
			// default action
			L2D_EvokeAction( handle , STR_DEFAULT );
		} else {
			handle = -1;
		}
		if ( i == 0 ){
			work->l2d[ i ].handle_2d = handle ;
			work->l2d[ i ].action = STR_OPENSPECIAL;
			work->l2d[ i ].flag = SK_ACTION_START;
			// l2d
			L2D_SetSignalHandle( work->l2d[ i ].handle_2d , work , Signal );
		} else {
			work->l2d[ i ].handle_2d = handle ;
			work->l2d[ i ].action = 0;
			work->l2d[ i ].flag = 0;
		}
	}
	// position
	work->position = 0;
	work->old_position = 0;
	// page
	work->now_story = SK_Story[ 0 ];
	work->page_alpha = 0;
	for ( i = 0 ; i < MAX_POSITION ; i ++ ){
		work->page[ i ].max = max_page[ i ];
		work->page[ i ].position = 0;
		work->page[ i ].flag = 0;
		work->page[ i ].r = SK_R;
		work->page[ i ].g = SK_G;
		work->page[ i ].b = SK_B;
		work->page[ i ].raute = 1;
	}
	// menu
	for ( i = 0 ; i < MAX_POSITION ; i ++ ){
		work->menu[ i ].hokan[ 0 ].flag = SK_MOVE_OK;
		work->menu[ i ].hokan[ 1 ].flag = SK_MOVE_OK;
	}
	// dot
	work->dot_hokan.flag = SK_MOVE_OK;
	// black
#if 0
	work->black.hokan.p = 0.0f;
	work->black.hokan.count = 1;
	work->black.hokan.flag = SK_MOVE_OK;
	work->black.hokan.code1 = 0;
	work->black.hokan.code2 = 0;
	work->black.hokan.morf_parts = 0;
#endif
	// book fade 
	for ( i = 0 ; i < MAX_BOOK_FADE ; i ++ ){
		work->book_fade[ i ].hokan.flag = SK_MOVE_OK;
	}
	// work
	work->flag = SK_ACTION_OK;
	work->position = 0;
	work->pad_status = 0;
	work->pad_check = 0;
	// gcl
	work->proc_prev = GCL_GetOptionValue( 'p' , 0 ); // procの読み込み

	work->msg.font = BP_GCL_LOOKUP_NEW_FONT_STRING(( char * )GetLocalResource( work->now_story , work->page[ work->position ].position ));
	work->msg.r = FONT_R;
	work->msg.g = FONT_G;
	work->msg.b = FONT_B;
	work->msg.a = 128;
	// demon
	for ( i = 0 ; i < 2 ; i ++ ){
		work->msg.work[ i ] = NewTextScreenControl();
		if ( work->msg.work[ i ] == NULL ){
			return (-1);
		}
		MENU_ClearTextTexture( work->msg.work[ i ] );
		GV_SetActorChild( work , work->msg.work[ i ] );
	}
	work->msg.clock = 0;
	// etc
	work->act = ( void * )NormalAct;
	BlackInit( work ); // 黒いた

	// sound
	GM_SdSet( SNG_SYUKAN_ON );
	//

	  /* 各言語で文字の量が違うので調整 2002.09.25 YANO */
	  if( GM_Language == GM_LANG_JAPANESE ){
		  work->msg_width  = J_MSG_WIDTH;
		  work->msg_height = J_MSG_HEIGHT;
		  work->msg_tex_width  = J_MSG_TEX_WIDTH;
		  work->msg_tex_height = J_MSG_TEX_HEIGHT;
	  } else if( GM_Language == GM_LANG_ENGLISH ){
		  work->msg_width  = E_MSG_WIDTH;
		  work->msg_height = E_MSG_HEIGHT;
		  work->msg_tex_width  = E_MSG_TEX_WIDTH;
		  work->msg_tex_height = E_MSG_TEX_HEIGHT;
	  } else {
		  work->msg_width  = EU_MSG_WIDTH;
		  work->msg_height = EU_MSG_HEIGHT;
		  work->msg_tex_width  = EU_MSG_TEX_WIDTH;
		  work->msg_tex_height = EU_MSG_TEX_HEIGHT;
	  }		  
	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewPreviousStory( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->name = name ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
