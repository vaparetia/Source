//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   albam_name.c
   ネームエントリー
   
   2001/06/05	S.Kobayashi
   $Id: albam_name.c,v 1.2 2002/12/05 18:42:02 takaki Exp $
*/

/* メモリーカードがやばいかも */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include    <libutl.h>
#include    "font.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"
#include    "../../mode/codec/cjimaku.h"
#include    "../../kano/mcman/mcman.h"
#include    "../../kano/titlescr/subtitle.h"
#include "libfs.h"

#include "mode/menu/xtextscn.h"

#define		LAYOUT		  (467533)
#define 	STR_NODE_FONT (10045112)
#define     STR_DOT_CURS  (2531169)
#define     STR_DEFAULT   (566267)
#define     STR_PROG      (2741831)
#define     STR_ROOT      (2770484)
#define     STR_FONT_HILIGHT (1666338)
#define     STR_CURS       (6756848)
#define 	DEFAULT_POS_X ( 33.0f +10.0f)
#define		DEFAULT_POS_Y ( 317.0f )
#define 	FONT_WIDTH ( 18.0f )
#define 	FONT_HEIGHT ( 14.0f )
#define     DOT_POS_X ( 18.0f )
#define     DOT_POS_Y ( 14.0f )
#define 	DEFAULT_DOT_POS_X ( DEFAULT_POS_X + DOT_POS_X )
#define 	DEFAULT_DOT_POS_Y ( DEFAULT_POS_Y + DOT_POS_Y )
#define 	STR_WIDTH  (18) // strcode ではなく文字の長さ
#define 	STR_HEIGHT (14) // strcode ではなく文字の長さ

#define IN_FADE_SPEED (6)
#define OUT_FADE_SPEED (4)
// 外部L2D strcode
#define STR_MENU_OK (1844828)
#define STR_MENU_CANCEL (5921625)
// action 
#define STR_OPENOVERWRITE (12887728)
#define STR_CLOSEOVERWRITE (6977128)
#define STR_DEFAULTACTION (566267)

// over write
#define OVER_WRITE (64)
#define OVER_WRITE_YES_NO (8) // plus 'yes / no'
#define OVER_WRITE_YES (3) // yes
#define OVER_WRITE_NO  (2) // yes

#define DEFAULT_OVER_WRITE_POS_X (41.0f)
#define DEFAULT_OVER_WRITE_POS_Y (33.0f)
#if 0
#  ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
#    define DEFAULT_OVER_WRITE_SELECT_POS_X (390.0f)
#    define DEFAULT_OVER_WRITE_SELECT_POS_Y (38.0f)
#  else
#    define DEFAULT_OVER_WRITE_SELECT_POS_X (390.0f)
#    define DEFAULT_OVER_WRITE_SELECT_POS_Y (33.0f) // 33.0f
#  endif
#else
#  define DEFAULT_OVER_WRITE_SELECT_POS_X (388.0f)
#  define DEFAULT_OVER_WRITE_SELECT_POS_Y (BP_Area_EU() ? 38.0f : 33.0f) // 33.0f
#endif
#define SK_R (80) 
#define SK_G (90) 
#define SK_B (85) 
#define SK_A (52)
#define SK_LOW_A (52)

// Japanease or Usa or Pal
#if 0
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define ALBAM_NAME_DELETE PAD_Y
#define CANCEL PAD_CANCEL //元はPAD_B
#define START  PAD_STA

#else

#define SELECT PAD_OK //元はPAD_A yano 2002.05.10
#define ALBAM_NAME_DELETE PAD_Y
#define CANCEL PAD_CANCEL //元はPAD_B
#define START  PAD_STA

#endif



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
#define	SK_HILIGHT_FADE_OUT         I64(0x0000080000000000)
#define	SK_HILIGHT_FADE_IN          I64(0x0000100000000000)
#define	SK_SAVE_START               I64(0x0000800000000000)
#define	SK_END                      I64(0x0000200000000000)


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
} Photo_List;

typedef struct {
	SPR_OBJ *ok;	
	SPR_OBJ *no;
	SPR_OBJ *dot;
	int      a[ 2 ];
	int      position;	
	SPR_OBJ *obj[ OVER_WRITE_YES_NO ];
	SPR_OBJ *hold_obj[ 2 ][ OVER_WRITE ];
} OverWriteWork;

typedef struct {
	int r;
	int g;
	int b;
	int a;
	void *work;
	char *data;
	int position; // resource 専用
} Font;
	
typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int					handle; // 外部ハンドル
	SPR_OBJ             *font[ ASCCI_MAX ];
	SPR_OBJ             *dot_curs; // sprite
	SPR_OBJ             *hilight;
#if 0
	SPR_OBJ             *ok; // over write
	SPR_OBJ             *no; // over write
	SPR_OBJ             *dot; // over write
#endif
	Photo_List          *photo_list;
	Code                font_work;
	u_char              *name_data; // 外部データ
	char                name_data_copy[ ASCCI_MAX ]; // 外部データバックUP
	OverWriteWork       over_write;
	Font                jpn_font;
	int                 position;
//	int                 over_write_position;
	SPR_COLOR           color;
	int                 action;
	int                 action_num;
	int                 base_u;
	int                 base_v;
	u_long64              flag;
	u_long64              *out_flag; // 外部フラグ
	int                 pad_status;
	int                 pad_check;
	int                 hold_time;
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	int                 port;
	int                 end_time;
	int                 album_resource;
	void  ( *act )( struct _work * );
} Work ;

// プロトタイプ
static int AnimetionAct( Work * ); // アニメーション
static void OverWrite( Work * ); // 上書き
static void EscapeAct( Work * );
static void StringCleate( Work *pWork );
static void StringDisp( Work *pWork );
static inline void OverWriteColorRe( Work *pWork );

// extern 
extern void SK_Printf( int ascci , SPR_OBJ *pObj , int u , int v , int , int );
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern int ChangePhotoInfoStart( int , int , void * );
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern float SK_PrintfNormalWidth( u_char , SPR_OBJ * );
extern void SK_AllHide( SPR_OBJ ** , int );
extern void SK_AllShow( SPR_OBJ ** , int );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern float SK_PrintfNormalWidth3( u_char *ascci , int max_position ); // 一括 space通常

// kano
extern void *GetLocalResource( int ref_id, int offset );

static int SprInit( Work *pWork ) // default を a
{
	SPR_OBJ		*spr;
	int          i,j;

	// 数字の取得
	spr = L2D_GetObject( pWork->handle_2d, STR_NODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("node_font\0");
		return -1 ;
	}
	SPR_HIDE( spr );
#if 0 //BP_PS2 def PSX2
	pWork->base_u = spr->sprite.head.tex.u;
	pWork->base_v = spr->sprite.head.tex.v;
#else
	pWork->base_u = spr->sprite.head.tex.u*16.0f;
	pWork->base_v = spr->sprite.head.tex.v*16.0f;
#endif
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_POS_X;
	spr->sprite.pos.y = DEFAULT_POS_Y;
	for ( i = 0 ; i < ASCCI_MAX ; i++ ){
		pWork->font[ i ] = SPR_DuplicateTree( spr );
		pWork->font[ i ]->sprite.pos.y = DEFAULT_POS_Y+4;     //BP JG - offset to line up the cursor with the text.
	}
	SK_PrintfNormal2( pWork->name_data , pWork->font , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );

	// dot out
	spr = L2D_GetObject( pWork->handle_2d , STR_DOT_CURS );
	if ( spr == NULL ){
		SK_Err("dot\0");
		return -1 ;
	}
	SPR_HIDE( spr );
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
//   if ( BP_Area_EU() )
//   	spr->sprite.pos.y = DEFAULT_OVER_WRITE_SELECT_POS_Y + 13;
//#else
//   else
   spr->sprite.pos.y = DEFAULT_OVER_WRITE_SELECT_POS_Y + 13;
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
      if ( !BP_Area_EU() )
      {
//#ifndef AREA_EU_BP_IGNORE()	// #ifndef PAL
   		SK_PrintfNormal( buf , pWork->over_write.obj , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , OVER_WRITE_YES_NO );
      }
//#endif
	}
	SK_PrintfChengColor2( pWork->over_write.obj , SK_R , SK_G , SK_B , 0 , OVER_WRITE_YES_NO );
	SPR_HIDE( spr );
	// hilight
	spr = L2D_GetObject( pWork->handle_2d , STR_FONT_HILIGHT ) ;
	if ( spr == NULL ){
		SK_Err("hilight\0");
		return -1 ;
	}
	SPR_SHOW( spr );
	spr->sprite.col.a -= 30;
	pWork->hilight = spr;
	// prog
	spr = L2D_GetObject( pWork->handle_2d , STR_PROG );
	if ( spr == NULL ){
		SK_Err("prog\0");
		return -1 ;
	}
	SPR_SHOW( spr );

	// 色をもどす
	OverWriteColorRe( pWork );
	SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );

	return( 0 );
}

#define RAUTE (4.0f)
static void Update( Work *pWork ) // update
{
	int width;
	int tmp;

	// hilight
	SK_PrintfNormal2( pWork->name_data , pWork->font , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
	pWork->hilight->sprite.pos.x = pWork->font[ 0 ]->sprite.pos.x - 1;
	pWork->hilight->sprite.pos.x += SK_PrintfNormalWidth3( pWork->name_data , pWork->position ); // 一括 space通常
#if 0
	for ( i = 0 ; i < pWork->position ; i ++ ){
		tmp = SK_PrintfNormalWidth( pWork->name_data[ i ] , pWork->font[ i ] );
		tmp = ( tmp == 0xff ? 14 : tmp );
		pWork->hilight->sprite.pos.x += tmp;
	}
#endif
	pWork->hilight->sprite.pos.y =  pWork->font[ pWork->position ]->sprite.pos.y - 2;
	tmp = SK_PrintfNormalWidth( pWork->name_data[ pWork->position ] , pWork->font[ pWork->position ] );
	tmp = ( tmp == 0xff ? 14 : tmp );
	width = tmp + 1;
	pWork->hilight->sprite.dw += ( float )( width - pWork->hilight->sprite.dw ) / RAUTE;
	pWork->hilight->sprite.dh = pWork->font[ pWork->position ]->sprite.dh;
}

static void NoneName( Work *pWork ) // 名無の時？にする
{
	u_char *ascci;
	int  i;

	ascci = pWork->name_data;

	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){ // 文字整形
		if ( ascci[ i ] == 0x60 || ascci[ i ] == 0x61 ){
			ascci[ i ] = 0x80;
//			pWork->position = ( i - 1 < 0 ? 0 : i - 1 );
			break;
		}
	}
	for ( i = 0 ; i <= pWork->position ; i ++ ) {	
		if ( ascci[ i ] != 0x20 && ascci[ i ] != 0x80 ){
			return;
		}
	}
	i -= 1;
	if ( i == pWork->position || ( pWork->position == 0 && ( ascci[ 0 ] == 0x20 || ascci[ 0 ] == 0x80 ) ) ){
		ascci[ 0 ] = '?';
		ascci[ 1 ] = 0x80;
	}
}

static inline void OverWriteIn( Work *pWork )
{
	pWork->jpn_font.position = 10;
	StringCleate( pWork );
	SK_AllShow( pWork->over_write.obj , OVER_WRITE_YES_NO );
}

static void PadControlNormal( Work *pWork )
{
	static char ForWardOffset[]  = { 33 , 5 , 1 , 1 , 1 , 1 , 8 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 12 , 
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 38 , 1 , 4 , 1 , 1 , 1 , 33 ,
 									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , -42 , 1 , 1 , 1 , 1 , 33 ,
									 1 , -65 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,  
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 5 , 1 , 1 , 1 , 2 , 1 };

	static char BackWardOffset[] = { 31 , 33 , 1 , 1 , 1 , 1 , 5 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 8 , 1 , 
									 -42 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 12 , 1 , 1 , 1 , 4 ,  
									 1 , 33 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,  
									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 38 ,
									 33 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 ,
 									 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 31 , 1 , };
	u_char *ascci;
	char   flag;
	int    status;
	int    tmp;

	status = L2D_ActionStatus( pWork->handle );
	if ( ( status == L2D_STAT_BUSY ) ){
		return;
	}

	ascci = pWork->name_data;
	flag = 0;
	if ( pWork->position == ASCCI_MAX - 1 ){ // ラスト
		if ( GV_PadDataDirect[ 0 ].press & SELECT ){
			if ( ascci[ pWork->position ] == 0x61 ){ // end
				ascci[ pWork->position ] = 0x80;
				L2D_BreakAction( pWork->handle );
				pWork->action = STR_OPENOVERWRITE;
				pWork->flag |= SK_ACTION_START;
				pWork->flag |= SK_HILIGHT_FADE_OUT;
				pWork->flag &= ~SK_ACTION_OK;
				pWork->flag &= ~SK_NORMAL;
				// 文字整形
				NoneName( pWork );
				SE_OK();
				return;
			}
		}
		if ( ( GV_PadDataDirect[ 0 ].press & CANCEL ) ){ 
			ascci[ pWork->position ] = 0x80; // space 
			pWork->position--;
			SE_CANCEL();
			return;
		}
		if ( GV_PadDataDirect[ 0 ].press & START ){
			ascci[ pWork->position ] = 0x80; // space 
			L2D_BreakAction( pWork->handle );
			pWork->action = STR_OPENOVERWRITE;
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_HILIGHT_FADE_OUT;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->flag &= ~SK_NORMAL;
			// 文字整形
			NoneName( pWork );
			SE_OK();
			return;
		}
		if ( GV_PadDataDirect[ 0 ].press & PAD_SEL ){
			pWork->act = ( void * )EscapeAct;
			pWork->flag &= ~SK_NORMAL;
			pWork->flag = SK_CHANCEL;

			pWork->jpn_font.position = 0xff;
			StringCleate( pWork );

			SE_CANCEL();
		}
		return;
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_L || pWork->pad_status & PAD_L ){
		pWork->pad_status = 0;
		tmp = ascci[ pWork->position ];
		ascci[ pWork->position ] -= BackWardOffset[ ascci[ pWork->position ] - 0x20 ];
		if ( ascci[ pWork->position ] < 0x20 ){
			ascci[ pWork->position ] = 0x80 + ( ascci[ pWork->position ] - tmp );
		}
		SE_SEL();
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_R || pWork->pad_status & PAD_R ) && !( flag & 0x1 ) ){
		pWork->pad_status = 0;
		ascci[ pWork->position ] += ForWardOffset[ ascci[ pWork->position ] - 0x20 ];
		if ( ascci[ pWork->position ] >= 0x7f ){ // loop
			ascci[ pWork->position ] = 0x20 + ( ascci[ pWork->position ] - 0x7f );
		}
		SE_SEL();
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & SELECT ) && !( flag & 0x1 ) ){
		if ( ascci[ pWork->position ] == 0x60 ){ // back space
			if ( pWork->position > 0 ){
				ascci[ pWork->position ] = 0x80; // space 
				pWork->position--;
				SE_CANCEL();
			}
		} else if ( ascci[ pWork->position ] == 0x61 ){ // end
			ascci[ pWork->position ] = 0x80;
			L2D_BreakAction( pWork->handle );
			pWork->action = STR_OPENOVERWRITE;
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_HILIGHT_FADE_OUT;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->flag &= ~SK_NORMAL;
			// 文字整形
			NoneName( pWork );
			SE_OK();
			return;
		} else if ( pWork->position < ASCCI_MAX - 2 ){
			ascci[ pWork->position + 1 ] = 0x20;//ascci[ pWork->name.position ]; 
			pWork->position++;
			SE_TYPE();
		} else {
			ascci[ pWork->position + 1 ] = 0x61;
			pWork->position++;
			SE_TYPE();
		}
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & CANCEL ) && !( flag & 0x1 ) ){ // 削除
		if ( pWork->position > 0 ){
			ascci[ pWork->position ] = 0x80; // space 
			pWork->position--;
			SE_CANCEL();
		} else if ( ascci[ pWork->position ] != 0x20 ){
			ascci[ pWork->position ] = 0x20; // space 
			SE_CANCEL();
		}
		flag = 0x1;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & START ) && !( flag & 0x1 ) ){
		ascci[ pWork->position + 1 ] = 0x80; // space 
		L2D_BreakAction( pWork->handle );
		pWork->action = STR_OPENOVERWRITE;
		pWork->flag |= SK_ACTION_START;
		pWork->flag |= SK_HILIGHT_FADE_OUT;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->flag &= ~SK_NORMAL;
		// 文字整形
		NoneName( pWork );
		SE_OK();
		return;
	}
	if ( ( GV_PadDataDirect[ 0 ].press & PAD_SEL ) && !( flag & 0x1 ) ){
		pWork->act = ( void * )EscapeAct;
		pWork->flag &= ~SK_NORMAL;
		pWork->flag = SK_CHANCEL;

		pWork->jpn_font.position = 0xff;
		StringCleate( pWork );

		SE_CANCEL();
		flag = 0x1;
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

static void HilightFade( Work *pWork )
{
	int alpha;

	if ( pWork->flag & SK_HILIGHT_FADE_OUT ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= OUT_FADE_SPEED;
		if ( alpha <= 0 ){
			pWork->hilight->sprite.col.a = 0;
			pWork->flag &= ~SK_HILIGHT_FADE_OUT;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
	if ( pWork->flag & SK_HILIGHT_FADE_IN ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha += IN_FADE_SPEED;
		if ( alpha >= 42 ){
			pWork->hilight->sprite.col.a = 42;
			pWork->flag &= ~SK_HILIGHT_FADE_IN;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	}
}

#define STR_MARUBATSU (12747845)
static void MarubatsuHideShow( Work *pWork , char mode ) // 説明文章の制御
{
	SPR_OBJ *spr;

	spr = L2D_GetObject( pWork->handle , STR_MARUBATSU );
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
	u_char *ascci;
	int status;
	int i;

	status = L2D_ActionStatus( pWork->handle );
	if ( ( status == L2D_STAT_BUSY ) ){
		return;
	}

	ascci = pWork->name_data;
	if ( GV_PadDataDirect[ 0 ].press & PAD_L ){
		if ( pWork->over_write.position == 1 ){
			pWork->over_write.position--;
		}
		SE_SEL();
	}
	if ( GV_PadDataDirect[ 0 ].press & PAD_R ){
		if ( pWork->over_write.position == 0 ){
			pWork->over_write.position++;
		}
		SE_SEL();
	}
	if ( GV_PadDataDirect[ 0 ].press & SELECT ){
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
			for ( i = 0 ; i < ASCCI_MAX ; i++ ){
				if ( pWork->name_data[ i ] == 0x80 || pWork->name_data[ i ] == '\0' ){
					ascci[ i ] = 0x80;
					if ( i != 0 ) {
			 			pWork->position = i - 1;
					} else {
//					ascci[ pWork->position ] = 0x41;
						pWork->position = 0;
					}
					break;
				}
			}
			SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
			for ( i = 0 ; i < 2 ; i ++ ){
				SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
			}
			pWork->jpn_font.position = 0xff;
			StringCleate( pWork );
			// 色をもどす
			OverWriteColorRe( pWork );
			L2D_BreakAction( pWork->handle );
			pWork->action = STR_CLOSEOVERWRITE;
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_HILIGHT_FADE_IN;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->flag &= ~SK_OVER_WRITE_MODE;
			MarubatsuHideShow( pWork , 1 );
			SE_CANCEL();
			break;
		}
	}
	if ( GV_PadDataDirect[ 0 ].press & CANCEL ){
		pWork->over_write.position = 1;
		for ( i = 0 ; i < ASCCI_MAX ; i++ ){
			if ( pWork->name_data[ i ] == 0x80 || pWork->name_data[ i ] == '\0' ){
				ascci[ i ] = 0x80;
				if ( i != 0 ) {
		 			pWork->position = i - 1;
				} else {
//					ascci[ pWork->position ] = 0x41;
					pWork->position = 0;
				}
				break;
			}
		}
		SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
		for ( i = 0 ; i < 2 ; i ++ ){
			SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
		}
		pWork->jpn_font.position = 0xff;
		StringCleate( pWork );
		// 色をもどす
		OverWriteColorRe( pWork );
		// action
		L2D_BreakAction( pWork->handle );
		pWork->action = STR_CLOSEOVERWRITE;
		pWork->flag |= SK_ACTION_START;
		pWork->flag |= SK_HILIGHT_FADE_IN;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->flag &= ~SK_OVER_WRITE_MODE;
		MarubatsuHideShow( pWork , 1 );
		SE_CANCEL();
	}
}

static void PadAct( Work *pWork )
{
	if ( pWork->flag & SK_END ){
		return;
	}
	if ( pWork->flag & SK_NORMAL ){ // 通常
		PadControlNormal( pWork );
	}
	if ( pWork->flag & SK_OVER_WRITE_MODE ){
		if ( !( ( pWork->flag & SK_SAVE_WAIT ) || ( pWork->flag & SK_SAVE_TIME ) ) ){
			PadControlOverWrite( pWork );
		}
	}
}

static void EscapeAct( Work *pWork )
{
	GCL_ARGS args;
	int     alpha;
	int       tmp;
	int       buf;


	tmp = 0;

	// l2d ここが + 1
	if ( pWork->hilight->sprite.col.a > 0 ){
		alpha = ( int )pWork->hilight->sprite.col.a;
		alpha -= 0x04;
		if ( alpha < 0 ){
			pWork->hilight->sprite.col.a = 0;
			tmp++;
		} else {
			pWork->hilight->sprite.col.a = ( u_char )alpha;
		}
	} else {
		tmp++;
	}
	if ( pWork->flag & SK_CHANCEL ){
		if ( tmp >= 1 ){
			if ( pWork->proc_prev != 0 ){
				args.argc = 1;
				args.argv = &buf;
				buf = pWork->name;
				GM_ExecProc( pWork->proc_prev , &args );
			}
			// バックアップの物に戻す
			memcpy( pWork->name_data , pWork->name_data_copy , ASCCI_MAX );
			GV_DestroyActor( pWork );
		}
	} else 	if ( tmp >= 1 ) {
		if ( pWork->proc_next != 0 ){
			args.argc = 1;
			args.argv = &buf;
			buf = pWork->name;

			GM_ExecProc( pWork->proc_next , &args );
		}
		GV_DestroyActor( pWork );
	}
}

#if 0
static inline void OverWriteDotUpdate( Work *pWork )
{
	switch ( pWork->over_write.position ){
	case 0 : // ok
		pWork->dot->empty.pos.y = -22;
		break;
	case 1 : // cancel
		pWork->dot->empty.pos.y = 0;
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
	SPR_OBJ *select;
	SPR_OBJ *non_select;
	// 対象オブジェクトの選択
	switch( pWork->over_write.position ){
	case 0 : // select is ok
		select = pWork->ok;
		non_select = pWork->no;
		break;
	case 1 : // select is no
		select = pWork->no;
		non_select = pWork->ok;
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
	if ( alpha < 64 ){
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

static void NormalAct( Work *pWork )
{
	int i;

	AnimetionAct( pWork );
	
	OverWriteEnd( pWork );
	HilightFade( pWork );
	if ( pWork->flag & SK_SAVE_TIME ){
		if ( pWork->end_time <= 0 ){
			SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
			for ( i = 0 ; i < 2 ; i ++ ){
				SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
			}
			pWork->jpn_font.position = 0xff;
			StringCleate( pWork );
			// action
			L2D_BreakAction( pWork->handle );
			pWork->action = STR_CLOSEOVERWRITE;
			pWork->flag |= SK_ACTION_START;
			pWork->flag |= SK_END;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->flag &= ~SK_SAVE_TIME;
			return;
		} else {
			pWork->end_time--;
		}
	}
	OverWriteFontControl( pWork );
	PadAct( pWork );
	Update( pWork );
	StringDisp( pWork );
}

static void Act( Work *pWork )
{
 	pWork->act( pWork );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	int status;
	int i;

	status = L2D_ActionStatus( pWork->handle );
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULTACTION : // default
			SprInit( pWork );
			pWork->act = (void *)NormalAct;
			break;
		case STR_OPENOVERWRITE : // over write view
			// over write
			OverWriteIn( pWork );
			pWork->flag |= SK_OVER_WRITE_MODE;
			break;
		case STR_CLOSEOVERWRITE : // over write view
			if ( pWork->flag & SK_END ){
				pWork->act = ( void * )EscapeAct;
				SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );
				for ( i = 0 ; i < 2 ; i ++ ){
					SK_AllHide( pWork->over_write.hold_obj[ i ] , OVER_WRITE );
				}
				pWork->jpn_font.position = 0xff;
				StringCleate( pWork );
				// 色をもどす
				OverWriteColorRe( pWork );
			} else {
				pWork->flag |= SK_NORMAL;
			}
			pWork->flag &= ~SK_OVER_WRITE_MODE;
			break;
		}
		pWork->flag &= ~SK_ACTION_OK;
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			switch ( pWork->action ){
			case STR_DEFAULTACTION : // 内部L2Dの操作
				L2D_EvokeAction( pWork->handle_2d , pWork->action );
				break;
			default : // 外部L2Dの操作
				L2D_EvokeAction( pWork->handle , pWork->action );
				break;
			}
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}

	return ( 0 );
}

#define R_POSITION (13)
#define G_POSITION (14)
#define B_POSITION (15)

static void OverWrite( Work *pWork ) // 上書き
{
	char tmp[ 16 ];
	int i;

	// メモりーカードに書きこみ
	// memory card 
	for ( i = 0 ; i < ASCCI_MAX ; i ++ ){
		tmp[ i ] = pWork->name_data[ i ];
	}
	tmp[ R_POSITION ] = ( u_char )pWork->photo_list->r;
	tmp[ G_POSITION ] = ( u_char )pWork->photo_list->g;
	tmp[ B_POSITION ] = ( u_char )pWork->photo_list->b;

	pWork->jpn_font.position = 5;

	StringCleate( pWork );
	SK_AllHide( pWork->over_write.obj , OVER_WRITE_YES_NO );

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

	if ( ChangePhotoInfoStart( pWork->port , pWork->photo_list->id , tmp ) != 0 ){ // <-- かのさん馬具ってる
		pWork->flag |= SK_SAVE_START;
	} else {
#ifdef DEBUG_MODE
		printf("Try Agein\n");
#endif
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
	y1 = DEFAULT_OVER_WRITE_POS_Y + 7;
	x2 = x1 + disp_width;
	y2 = y1 + SUBTITLE_FONT_DISP_HEIGHT;
	u1 = poly_width;
   if ( BP_Area_EU() )
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	   v1 = SUBTITLE_FONT_HEIGHT + 6;
   else
//#else
	   v1 = SUBTITLE_FONT_HEIGHT;
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
//	GM_GetResource( 5 , pWork->jpn_font.position );
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT + 1 ,
							0 , 0 , pWork->jpn_font.data );
#else // 海外
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , pWork->jpn_font.position ));
//	GM_GetResource( 5 , pWork->jpn_font.position );
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT + 1 ,
							0 , 0 , pWork->jpn_font.data );
#else
	pWork->jpn_font.data = ( char * )BP_GCL_LOOKUP_NEW_FONT_STRING(GetLocalResource( pWork->album_resource , pWork->jpn_font.position ));
//	pWork->jpn_font.data = GM_GetResource( 5 , pWork->jpn_font.position );
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
//	GM_GetResource( 5 , pWork->jpn_font.position );
	MENU_CreateTextTexture( pWork->jpn_font.work , 1 , 1 , poly_width , SUBTITLE_FONT_HEIGHT + 1 ,
							0 , 0 , 0 , pWork->jpn_font.data );
#endif
}

static	void	Die( Work *pWork )
{
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
	*pWork->out_flag |= SK_NAME_ENTRY_END; // ネームエントリー終了
	*pWork->out_flag &= ~SK_NAME_ENTRY; // ネームエントリー終了
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int		handle;
	int          i;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MENU , 1 , 0 ) ;
	if ( handle < 0 ){
		SK_Err("dot_and_font\0");
		return -1 ;
	}
	work->handle_2d = handle ;

	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = STR_DEFAULTACTION;
	work->act = ( void * )AnimetionAct;
	work->pad_status = 0;
	work->pad_check = 0;
	work->position = 0;
	work->proc_prev = 0;
	work->proc_next = 0;
	work->over_write.position = 1;
	for ( i = 0 ; i < ASCCI_MAX ; i++ ){
		if ( work->name_data[ i ] == 0x80 || work->name_data[ i ] == '\0' ){
//			work->position = 0x80;
			work->position = i - 1;
			if ( work->position < 0 ){
				work->name_data[ 0 ] = 0x41;
				work->name_data[ 1 ] = 0x80;
				work->position = 0;
			}
			break;
		}
	}
	memcpy( work->name_data_copy , work->name_data , ASCCI_MAX );
	// etc
	work->flag = SK_NORMAL | SK_ACTION_START;
	work->end_time = 0;
	work->jpn_font.position = 0xff;
	StringCleate( work );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewAlbamName( char *name_data , u_long64 *flag , int port , Photo_List *photo_list , int handle , void *jpn_font , int resource )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 30 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->name_data = name_data;
	work->out_flag = flag;
	work->photo_list = photo_list;
	work->handle = handle;
	work->port = port;
	work->jpn_font.work = jpn_font;
	work->album_resource = resource;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
