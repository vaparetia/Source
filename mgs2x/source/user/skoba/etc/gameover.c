//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   gameover.c
   ゲームオーバー
   
   2001/06/12	S.Kobayashi
   $Id: gameover.c,v 1.2 2002/12/05 18:42:01 takaki Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
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
#include    <string.h>

#include "mode/menu/xtextscn.h"

extern void ShowExitGameWarning(int *pResult);

#define		LAYOUT		    (9668898)
#define     STR_MENU_NULL   (10036828) // <--- exit continue : empty -> sprite / sprite
#define     STR_CURSOR      (9100063)  // sprite
#define     STR_MENU_LINE   (9959061)  // line
#define     STR_DEFAULT     (566267)
#define     STR_SIROWAKU    (3196160)
#define     STR_ROOT        (2770484)

#define 	STR_GAMEOVER (9221682) // 特殊GameOver tri file

#define     STR_BG_1 (3319825)
#define     BG_MAX (4)
#define 	FADE_SPEED (3)

#define IN_FADE_SPEED (6)
#define OUT_FADE_SPEED (12)
#define SK_R (128)
#define SK_G (128)
#define SK_B (128)
#define SK_A (128)
#define DOT_HEIGHT (18)
#define SCREEN_DEFAULT_POS_X  (33+20)
#define SCREEN_DEFAULT_POS_Y  (29)
#define SCREEN_DEFAULT_WIDTH  (203)
#define SCREEN_DEFAULT_HEIGHT (152)
#define FONT_MAX (2)
#define PLANT_BOMB_TEX (11726393)
#define PLANT_BOMB16 (13768267)

#define END_LIMIT ( DIRECT_TICK(120) )

//#undef KP_XBOX

enum {
	SK_NORMAL               = 0x00001,
	SK_DOT_FADE_IN_START    = 0x00002,
	SK_DOT_FADE_IN_OK       = 0x00004,
	SK_DOT_FADE_OUT_START   = 0x00008,
	SK_DOT_FADE_OUT_OK      = 0x00010,
	SK_FONT_FADE_IN_START   = 0x00020,
	SK_FONT_FADE_IN_OK      = 0x00040,
	SK_FONT_FADE_OUT_START  = 0x00080,
	SK_FONT_FADE_OUT_OK     = 0x000100,
	SK_SCREEN_MOVE_START    = 0x000200,
	SK_RESTART              = 0x000400,
	SK_RESTART_OK           = 0x000800,
	SK_SELECT_OK            = 0x001000,
	SK_FLUSH_OK             = 0x002000,
	SK_TEXT_FADE_IN         = 0x004000,
	SK_TEXT_FADE_OUT        = 0x008000,
	SK_PIC_DISP_OK          = 0x010000,
	SK_MINI_FONT_OK         = 0x020000,
	SK_MINI_FONT_PHASE_1_OK = 0x040000,
	SK_MINI_FONT_PHASE_2_OK = 0x080000,
	SK_PIC_XY1_OK           = 0x100000,
	SK_PIC_XY2_OK           = 0x200000,
	SK_STREAM_PLAY_OK       = 0x400000,
	SK_END                  = 0x800000,
};

enum {
	SK_TANKER = 0, // タンカー編
	SK_PLANT ,     // プラント編
	SK_BOSS_RUSH , // ボスラッシュ
	SK_VR , // VR add by yano 2002.05.13
	SK_BOSS_RUSH2 , // 新ボスラッシュ
};

enum {
	GAME_OVER_NORMAL = 0,
	GAME_OVER_BOMB ,
	GAME_OVER_WATER ,
	GAME_OVER_SUB ,
};

#define SK_NON_SUPRT (0xff)

typedef struct {
	SPR_OBJ *cursor;
	u_char r , g , b , a;	
} Dot;

typedef struct {
	void   *mini_screen;
	SPR_OBJ *menu_line;
	FVECTOR xy1;
	FVECTOR xy2;
	FVECTOR last_xy1;
	FVECTOR last_xy2;
	int     raute;
	u_char r , g , b , a;
} Screen;

typedef struct {
	u_char r , g , b , a;
} Color;

typedef struct {
	SPR_OBJ *bg[ BG_MAX ];
} BackGraund;

typedef struct {
	SPR_OBJ *font;
	Color color;
} Font;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 position;
	Dot                 dot;
	Font                font[ FONT_MAX ]; // continue & exit
	Screen              screen;
	SPR_OBJ             *menu_null;
	SPR_OBJ             *sirowaku;
	BackGraund          backgraund;
	SPR_OBJ             *special_view;
	int                 tick_count; // 処理落カウンタ
	int                 tex_handle;
	short               *noise_view;
	int                 action;
	int                 action_num;
	u_int               flag;
	int                 pad_status;
	int                 pad_check;
	int                 hold_time;
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	int 				timer;
	int					stream_handler ;
	int					stream_flag;
	char          		text_buffer[ 256 ];
	int                 text_pos[ 2 ];
	int                 text_count;
	float               text_alpha;
	int                 disp_line[ 2 ];
	int                 end_limit_count;
	int                 last_num;
	int                 tick_back;
	int                 retry_count;
	void                *menuprint_work_ptr;
	void  ( *act )( struct _work * );
#ifdef KP_XBOX
	int 				confirm;	/* 確認画面とのやりとりで使う変数 */
	int					con_alp;
	int					con_cnt;
#endif
} Work ;

static short SK_GameOverMode;
static int SK_Strcode;
static int SK_TriStrcode;

// プロトタイプ
static int AnimationAct( Work * ); // アニメーション
static void StreamRetry( Work * ); // Stream Retry機構
// extern 
extern void SK_PrintfChengColor( SPR_OBJ *pObj , u_char r , u_char g , u_char b , u_char a );
extern void SK_Printf( u_char ascci , SPR_OBJ *pObj , int u , int v , int str_width , int str_height );

extern void *NewTextScreenControl( void );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern int ChangePhotoInfoStart( int , int , void * );
// 
extern void *NewReduceScreenEffect( void );
extern void TAKABE_SetReduceScreenSize( void *work_ptr, int x1, int y1, int x2, int y2, int color );
extern void MENU_DrawPicture16( int x, int y, int w, int h, int color, void *image, int image_w, int image_h, int flag );
extern void *NewPictureDrawManager( int prio );

static int SprInit( Work *pWork ) // default を a
{
	SPR_OBJ		*spr;
	int            i;

	// dotの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_CURSOR );
	if ( spr == NULL ){
		return -1 ;
	}
	pWork->dot.cursor = spr;
	// lineの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_MENU_LINE );
	if ( spr == NULL ){
		return -1 ;
	}
	pWork->screen.menu_line = spr;
	// menu_nullの取得
	spr = L2D_GetObject( pWork->handle_2d, STR_MENU_NULL );
	if ( spr == NULL ){
		return -1 ;
	}
	pWork->menu_null = spr;
	// sirowaku
	spr = L2D_GetObject( pWork->handle_2d , STR_SIROWAKU );
	if ( spr == NULL ){
		return -1 ;
	}
	pWork->sirowaku = spr;
	// backgraund
	{
		int strcode;

		strcode = STR_BG_1;

		for ( i = 0 ; i < BG_MAX ; i ++ ){
			spr = L2D_GetObject( pWork->handle_2d , strcode );
			if ( spr == NULL ){
				return -1 ;
			}
			pWork->backgraund.bg[ i ] = spr;
			strcode++;
		}
		pWork->backgraund.bg[ 0 ]->sprite.pos.x = 0.0f;
		pWork->backgraund.bg[ 1 ]->sprite.pos.y = 0.0f;
	}
	if ( SK_GameOverMode != GAME_OVER_NORMAL && SK_GameOverMode != SK_NON_SUPRT ){
		static SPR_POS spr_pos = { 64.0F, 64.0F } ;

	 	// 特殊
	    pWork->special_view = SPR_Create_2D_Object( SP_SPRITE , 4 , NULL );

	    /* 表示座標の設定 */
	    SPR_SetPosSprite( pWork->special_view , &spr_pos );

	    /* 表示サイズの設定 */
	    SPR_SetSizeSprite( pWork->special_view , 400.0F, 300.0F);

	    /* テクスチャの設定 */
	    pWork->tex_handle = SPR_LoadTexture( SK_TriStrcode );
		if ( pWork->tex_handle < 0 ){
			SK_Err("non tri\0");
			ASSERT( 0 )
		}
		if ( SPR_ObjSetTexture( pWork->special_view , SK_Strcode , pWork->tex_handle ) < 0 ){
			SK_Err("non tex\0");
			ASSERT( 0 )
		}
	    /* アルファブレンディングの設定 */
	    pWork->special_view->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		pWork->special_view->sprite.col.r = 128;
		pWork->special_view->sprite.col.g = 128;
		pWork->special_view->sprite.col.b = 128;
		pWork->special_view->sprite.col.a = 128;
		// プライオリティ 
		SPR_SetPriority( pWork->special_view , 1 );
		// Chanel 0 Invisible
		DG_Chanl( 0 )->flag = 0;
	}

	return( 0 );
}

#define TICK_DIV (80000)
static inline int TickToSub( int tick_count )
{
	float ftmp;

	ftmp = DG_TickCount / TICK_DIV;
	if ( ftmp <= 0.5f ){
		ftmp = 0.5f;
	}
	ftmp = 1.0f / ftmp;
	ftmp = ftmp < 1.0f ? 1.0f : ftmp;
	return ( ( int )ftmp );
}

static inline int TickToFrame( Work *pWork )
{
	float ftmp;

	ftmp = DG_FABS( DG_TickCount - pWork->tick_back );
	pWork->tick_back = DG_TickCount;
	if ( pWork->tick_back < 1.0f ){
		pWork->tick_back = 1.0f;
	} else if ( pWork->tick_back > 5.0f ){
		pWork->tick_back = 5.0f;
	}
	printf( "tick_back = %f\n" , ftmp );
#if 0
	// アクションスピードの変更
	L2D_SetActionPlaySpeed( pWork->handle_2d , ( int )ftmp );
#endif
	return ( ftmp );
}

static inline void TickSet( Work *pWork )
{
	pWork->tick_count = TickToFrame( pWork );
#if 0
	// アクションスピードの変更
	L2D_SetActionPlaySpeed( pWork->handle_2d , pWork->tick_count );
#endif
	pWork->tick_back = DG_TickCount;
}

static void MiniMiniFontPhase1Update( Work *pWork )
{
	static short line_count[] = { 1 , 1 , 1 };
	static short line_frame[] = { 2 , 2 , 2 };
	int      count;

	if ( pWork->text_alpha < 1.0f ){
		return;
	}
	// コンソールテキスト内容の生成
	sprintf( pWork->text_buffer,
			"GACKUROTAS\n"
			"61301891\n"
			"ONIKAMOTOYK\n"
			/* 各種表示用パラメータ */
	);
	/* コンソールを１文字づつ表示しているかのように見せる処理 */
	if ( strlen( pWork->text_buffer ) > pWork->text_pos[ 0 ] ){
//		tmp = TickToSub( DG_TickCount );
		pWork->text_count -= pWork->tick_count;
		if ( pWork->text_count < 0 ){
			count = 0;
			while ( count < line_count[ pWork->disp_line[ 0 ] ] ){
				/* 新たに表示された文字の種類によって効果音などをつける */
				GM_SeSet( GM_PAN_CENTER , 255 , SD_S_GO_TYPE1 );
				while ( pWork->text_buffer[ pWork->text_pos[ 0 ] ] != '\n' ){
					pWork->text_pos[ 0 ]++;
				}
				pWork->text_pos[ 0 ]++;
				count++;
			}
			/* 次の文字が出るまでの時間を設定 */
			pWork->text_count = DIRECT_TICK( line_frame[ pWork->disp_line[ 0 ] ] );
			pWork->disp_line[ 0 ]++;
		}
		pWork->text_buffer[ pWork->text_pos[ 0 ] ] = '\0' ;
	} else {
		pWork->flag |= SK_MINI_FONT_PHASE_1_OK;
	}
	/* コンソールの内容を画面に出力 */
#ifdef PSX2 /*yano 2002.03.20*/
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 492 , DIRECT_SCREEN_Y( 60 ) , 1 );
#else
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 615 , DIRECT_SCREEN_Y( 60 ) , 1 );
#endif
	___MENU_Color( pWork->menuprint_work_ptr, 1, 196, 196, 196, ( u_char )pWork->text_alpha );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 1, pWork->text_buffer );
}

static void MiniMiniFontPhase2Update( Work *pWork )
{
	static short line_count[] = { 1 , 1 , 1 , 1 , 2 , 2 , 6 , 1 }; 
	static short line_frame[] = { 12 , 12 , 6 , 15 , 3 , 6 , 18 , 6 }; 
	int          count;

	if ( pWork->text_alpha < 1.0f || !( pWork->flag & SK_MINI_FONT_PHASE_1_OK ) ){
		return;
	}
	// コンソールテキスト内容の生成
	sprintf( pWork->text_buffer,
			"ARMSTREN GTH%d\n"
			"PRUD ENCE%d\n"
			"AMMOUS ED%d\n"
			"ALRTNUM%d\n\n"
			"NUET%d\n"
			"DMGA MMOUNT%d\n\n"
			"CAMPOSX%d\n"
			"CAMPO SY%d\n"
			"CAM POSZ%d\n"
			"AREAMO VE%d\n\n\n" 
#ifdef PSX2
			"VANISHED POINT%-20s                           dsjw%6d\n" ,
#else
			/* MENU_Printf系は横幅が実際の画面ピクセルに依存する為 */
			"VANISHI NGPOINT%s                                  dsjw%6d\n" ,
#endif
			/* 各種表示用パラメータ */
			 GM_RaidenChin_Up , GM_SaveCount , GM_ShootCount , GM_AlertCount ,
			 GM_KillCount , GM_DamageCount , GM_CameraX , GM_CameraY , GM_CameraZ , GM_GlobalLoadCount , GM_GetArea() , pWork->last_num
			);
	/* コンソールを１文字づつ表示しているかのように見せる処理 */
	if ( strlen( pWork->text_buffer ) > pWork->text_pos[ 1 ] ){
//		tmp = TickToSub( DG_TickCount );
		pWork->text_count -= pWork->tick_count;
		if ( pWork->text_count < 0 ){
			count = 0;
			while ( count < line_count[ pWork->disp_line[ 1 ] ] ){
				/* 新たに表示された文字の種類によって効果音などをつける */
				GM_SeSet( GM_PAN_CENTER , 255 , SD_S_GO_TYPE1 );
				while ( pWork->text_buffer[ pWork->text_pos[ 1 ] ] != '\n' ){
					pWork->text_pos[ 1 ]++;
				}
				pWork->text_pos[ 1 ]++;
				count++;
			}
			/* 次の文字が出るまでの時間を設定 */
			pWork->text_count = DIRECT_TICK( line_frame[ pWork->disp_line[ 1 ] ] );
			pWork->disp_line[ 1 ]++;
		}
		pWork->text_buffer[ pWork->text_pos[ 1 ] ] = '\0' ;
	} else {
		pWork->flag |= SK_MINI_FONT_PHASE_2_OK;
		// clear
		GM_GameOverClear() ;
	}


	/* コンソールの内容を画面に出力 */
#ifdef PSX2
#ifdef PAL
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 35, DIRECT_SCREEN_Y( 280 ) + 16 , 0 );
#else
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 55, DIRECT_SCREEN_Y( 280 ) , 0 );
#endif
#else
#ifdef PAL
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 43.75, DIRECT_SCREEN_Y( 280 ) + 16 , 0 );
#else
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 43.75, DIRECT_SCREEN_Y( 280 ) , 0 );
#endif
#endif
	___MENU_Color( pWork->menuprint_work_ptr, 1, 196, 196, 196, ( u_char )pWork->text_alpha );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 1, pWork->text_buffer );
}

static void TextAlphaControl( Work *pWork )
{
	if ( pWork->flag & SK_TEXT_FADE_IN ){
		if ( pWork->text_alpha < 128.0f ){
			pWork->text_alpha += DIRECT_TICK( 1.0f );
		} else {
			pWork->text_alpha = 128.0f;
		}
	}
	if ( pWork->flag & SK_TEXT_FADE_OUT ){
		if ( pWork->text_alpha > 0.0f ){
			pWork->text_alpha -= DIRECT_TICK( 2.0f );
		} else {
			pWork->text_alpha = 0.0f;
		}
	}

}

static void Update( Work *pWork ) // update
{
	SPR_OBJ *spr;
	
	// dot position update
	switch ( pWork->position ){
	case 0 : // exit
		spr = pWork->menu_null->head.child;
		break;
	case 1 : // continue
		spr = pWork->menu_null->head.child->head.next;//
		break;
	default : 
		spr = pWork->menu_null->head.child;
		break;
	}
	pWork->dot.cursor->sprite.pos.y = spr->sprite.pos.y + 3;
}

// Japanease or Usa or Pal
#if 0
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL

#define SELECT PAD_OK      //元はPAD_A yano 2002.05.10
#define CANCEL PAD_CANCEL  //元はPAD_B

#else

#define SELECT PAD_OK      //元はPAD_A yano 2002.05.10
#define CANCEL PAD_CANCEL  //元はPAD_B

#endif

static void PadAct( Work *pWork )
{
	SPR_OBJ *pContinue;
	SPR_OBJ *pExit;
	SPR_OBJ *pNow;
	SPR_OBJ *pNoSelect;
	
	if ( !( pWork->flag & SK_SELECT_OK ) || pWork->flag & SK_END ){
		return;
	}
	// 取得
	pContinue = pWork->menu_null->head.child;
	pExit = pWork->menu_null->head.child->head.next;
	if ( GV_PadDataDirect[ 0 ].press & PAD_U ){
		if ( pWork->position > 0 ){
			// 現在選択されているものを選ぶ
			if ( pWork->position == 0 ){
				pNow = pContinue;
				pNoSelect = pExit;
			} else {
				pNow = pExit;
				pNoSelect = pContinue;
			}
			pNoSelect->sprite.col.a = ( u_char )68;
			pWork->position--;
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_CUR01 );
		}
	} else if ( GV_PadDataDirect[ 0 ].press & PAD_D ){
		if ( pWork->position < 1 ){
			// 現在選択されているものを選ぶ
			if ( pWork->position == 0 ){
				pNow = pContinue;
				pNoSelect = pExit;
			} else {
				pNow = pExit;
				pNoSelect = pContinue;
			}
			pNoSelect->sprite.col.a = ( u_char )68;
			pWork->position++;
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_CUR01 );
		}
	} else if ( GV_PadDataDirect[ 0 ].press & ( SELECT | PAD_STA ) )
   {
      int bp_doFinish = 1;
		if( pWork->position )
      {
			/* EXIT選択 */
#ifdef PSX2
         //BP - added system dialog on X360 to prevent destructive action
         //without confirmation.
         int confirmRet;
         ShowExitGameWarning( &confirmRet );
         if( confirmRet == 0 )   //yes
         {
            GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_START01 );
         }
         else
         {
            //Pretend the user did not ever hit the button.
            bp_doFinish = 0;
         }
#endif
#ifdef KP_XBOX /* add by Yano 2002.08.15 */
			{
				/* 確認画面を出す */
				/* confirm::: 0:起動前 -1:起動中 1:YES選択 2:NO選択フェード始め 3:NO選択のち確認画面死んだ */
				extern void *NewGameOverExitConfirm( int *confirm  );
				void *res;
				res = NewGameOverExitConfirm( &pWork->confirm );
				if( res != NULL ){
					GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);
					/* 文字のフェードインアウトのフラグを消しておく */
					pWork->flag &= ~SK_TEXT_FADE_OUT;
					pWork->flag &= ~SK_TEXT_FADE_IN;
				}
				return;
			}
#endif
		}else
      {
			/* CONTINUE選択 */
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_START001 );
		}

      if( bp_doFinish )
      {
		   // l2d chancel
		   L2D_BreakAction( pWork->handle_2d );
		   // etc
		   pWork->flag |= SK_RESTART;
		   pWork->flag |= SK_DOT_FADE_OUT_START;
		   pWork->flag |= SK_END;
		   pWork->flag &= ~SK_DOT_FADE_OUT_OK;
		   pWork->flag &= ~SK_DOT_FADE_IN_START;
		   pWork->flag &= ~SK_DOT_FADE_IN_OK;
		   return;
      }
	}
}

#define STR_DOT_BLINK (5959383)
static void DotControl( Work *pWork )
{
	SPR_OBJ *spr;
	int alpha;

	if ( !( pWork->flag & SK_FLUSH_OK ) ){
		return;
	}
	// 点滅
	if ( pWork->flag & SK_DOT_FADE_IN_START ){
		spr = pWork->dot.cursor;
		alpha = spr->sprite.col.a;
//		tmp = TickToSub( DG_TickCount );
		alpha += ( FADE_SPEED + pWork->tick_count );
		if ( alpha > 128 ){
			spr->sprite.col.a = ( u_char )128;
			pWork->flag |= SK_DOT_FADE_OUT_START;
			pWork->flag &= ~SK_DOT_FADE_IN_START;
		} else {
			spr->sprite.col.a = ( u_char )alpha;
		}
	}
	if ( pWork->flag & SK_DOT_FADE_OUT_START ){
		spr = pWork->dot.cursor;
		alpha = spr->sprite.col.a;
//		tmp = TickToSub( DG_TickCount );
		alpha -= ( FADE_SPEED + pWork->tick_count );
		if ( alpha < 0 ){
			spr->sprite.col.a = ( u_char )0;
			if ( pWork->flag & SK_RESTART ){
				pWork->flag &= ~SK_DOT_FADE_OUT_START;
			} else {
				pWork->flag |= SK_DOT_FADE_IN_START;
				pWork->flag &= ~SK_DOT_FADE_OUT_START;
			}
		} else {
			spr->sprite.col.a = ( u_char )alpha;
		}
	}
}

static void FontUpdate( Work *pWork )
{
	SPR_OBJ *pContinue;
	SPR_OBJ *pExit;
	SPR_OBJ *pNow;
	SPR_OBJ *pNoSelect;
	int      alpha;

	if ( !( pWork->flag & SK_MINI_FONT_PHASE_2_OK ) ){
		return;
	}
	// 取得
	pContinue = pWork->menu_null->head.child;
	pExit = pWork->menu_null->head.child->head.next;
	// 現在選択されているものを選ぶ
	if ( pWork->position == 0 ){
		pNow = pContinue;
		pNoSelect = pExit;
	} else {
		pNow = pExit;
		pNoSelect = pContinue;
	}
	alpha = ( int )pNow->sprite.col.a;
	alpha += FADE_SPEED;
	if ( alpha >= 128 ){
		alpha = 128;
	}
	pNow->sprite.col.a = ( u_char )alpha;
	// 選ばれていない方はうすく
	alpha = pNoSelect->sprite.col.a;
//	tmp = TickToSub( DG_TickCount );
	alpha -= ( FADE_SPEED + pWork->tick_count );
	if ( alpha < 48 ){
		pNoSelect->sprite.col.a = ( u_char )48;
	} else {
		pNoSelect->sprite.col.a = ( u_char )alpha;
	}
}

static void BgControl( Work *pWork )
{
	// left
	pWork->backgraund.bg[ 0 ]->sprite.dw = abs( pWork->screen.xy1.vx - pWork->backgraund.bg[ 0 ]->sprite.pos.x ) + 1.0f;
	pWork->backgraund.bg[ 0 ]->sprite.dh = pWork->screen.xy2.vy;
	// top
	pWork->backgraund.bg[ 1 ]->sprite.pos.x = pWork->screen.xy1.vx - 1.0f; // right
	pWork->backgraund.bg[ 1 ]->sprite.dh = abs ( pWork->screen.xy1.vy - pWork->backgraund.bg[ 1 ]->sprite.pos.y ) + 1.0f;
	pWork->backgraund.bg[ 1 ]->sprite.dw = pWork->screen.xy2.vx;
	// bottm
	pWork->backgraund.bg[ 2 ]->sprite.pos.y = pWork->screen.xy2.vy - 1.0f; // bottm
	pWork->backgraund.bg[ 2 ]->sprite.dw = pWork->screen.xy2.vx;
	pWork->backgraund.bg[ 2 ]->sprite.dh = abs( ( pWork->screen.xy2.vy - DRAW_HEIGHT ) );
	// right
	pWork->backgraund.bg[ 3 ]->sprite.pos.x = pWork->screen.xy2.vx - 1.0f; // right
	pWork->backgraund.bg[ 3 ]->sprite.dw = abs( ( pWork->screen.xy2.vx - ( DRAW_WIDTH + 10.0f ) ) );
	// sirowaku
	pWork->sirowaku->box.rect.begin.x = pWork->screen.xy1.vx + 1.0f;
	pWork->sirowaku->box.rect.begin.y = pWork->screen.xy1.vy;
	pWork->sirowaku->box.rect.end.x = pWork->screen.xy2.vx - 1.0f;
	pWork->sirowaku->box.rect.end.y = pWork->screen.xy2.vy;

   // BP JG - trim the box just a little so it fits the screen capture better
   pWork->sirowaku->box.rect.begin.y+=1;
   pWork->sirowaku->box.rect.end.y-=1;
}

#define PIC_SPEED_LIMIT ( 0.2f )
static void ScreenDisp( Work *pWork )
{
	FVECTOR fvtmp;
	float    sign;
	u_int   color;

	// 補完
	if ( pWork->flag & SK_SCREEN_MOVE_START ){
		if ( !( pWork->flag & SK_PIC_XY1_OK ) ){
			_sceVu0SubVector( &fvtmp , &pWork->screen.last_xy1 , &pWork->screen.xy1 );
			_sceVu0DivVector( &fvtmp , &fvtmp , 12.0f );
			sign = fvtmp.vx < 0.0f ? -1.0f : 1.0f;
			fvtmp.vx = DG_FABS( fvtmp.vx );
			if ( fvtmp.vx < PIC_SPEED_LIMIT ){
				fvtmp.vx = PIC_SPEED_LIMIT * sign;
			} else {
				fvtmp.vx *= sign;
			}
			sign = fvtmp.vy < 0.0f ? -1.0f : 1.0f;
			fvtmp.vy = DG_FABS( fvtmp.vy );
			if ( fvtmp.vy < PIC_SPEED_LIMIT ){
				fvtmp.vy = PIC_SPEED_LIMIT * sign;
			} else {
				fvtmp.vy *= sign;
			}
			_sceVu0AddVector( &pWork->screen.xy1 , &pWork->screen.xy1 , &fvtmp );
			// 範囲チェック
			if ( pWork->screen.xy1.vx > pWork->screen.last_xy1.vx )
         {
				pWork->screen.xy1.vx = pWork->screen.last_xy1.vx;
			}
			if ( pWork->screen.xy1.vy > pWork->screen.last_xy1.vy )
         {
				pWork->screen.xy1.vy = pWork->screen.last_xy1.vy;
			}
			if ( ( pWork->screen.xy1.vx >= pWork->screen.last_xy1.vx ) && ( pWork->screen.xy1.vy >= pWork->screen.last_xy1.vy ) )
         {
				pWork->screen.xy1.vx = pWork->screen.last_xy1.vx;
				pWork->screen.xy1.vy = pWork->screen.last_xy1.vy;
				pWork->flag |= SK_PIC_XY1_OK;
			}
		}
		if ( !( pWork->flag & SK_PIC_XY2_OK ) ){
			_sceVu0SubVector( &fvtmp , &pWork->screen.last_xy2 , &pWork->screen.xy2 );
			_sceVu0DivVector( &fvtmp , &fvtmp , 12.0f );
			sign = fvtmp.vx < 0.0f ? -1.0f : 1.0f;
			fvtmp.vx = DG_FABS( fvtmp.vx );
			if ( fvtmp.vx < PIC_SPEED_LIMIT ){
				fvtmp.vx = PIC_SPEED_LIMIT * sign;
			} else {
				fvtmp.vx *= sign;
			}
			sign = fvtmp.vy < 0.0f ? -1.0f : 1.0f;
			fvtmp.vy = DG_FABS( fvtmp.vy );
			if ( fvtmp.vy < PIC_SPEED_LIMIT ){
				fvtmp.vy = PIC_SPEED_LIMIT * sign;
			} else {
				fvtmp.vy *= sign;
			}
			_sceVu0AddVector( &pWork->screen.xy2 , &pWork->screen.xy2 , &fvtmp );
			// 範囲チェック
			if ( pWork->screen.xy2.vx < pWork->screen.last_xy2.vx ){
				pWork->screen.xy2.vx = pWork->screen.last_xy2.vx;
			}
			if ( pWork->screen.xy2.vy < pWork->screen.last_xy2.vy ){
				pWork->screen.xy2.vy = pWork->screen.last_xy2.vy;
			}
			if ( ( pWork->screen.xy2.vx <= pWork->screen.last_xy2.vx ) || ( pWork->screen.xy2.vy <= pWork->screen.last_xy2.vy ) ){
				pWork->screen.xy2.vx = pWork->screen.last_xy2.vx;
				pWork->screen.xy2.vy = pWork->screen.last_xy2.vy;
				pWork->flag |= SK_PIC_XY2_OK;
			}
		}
		// fix 領域の枠の移動
		BgControl( pWork );
		TickSet( pWork );
#if 0
		if ( pWork->screen.raute > 1 ){
			pWork->screen.raute--;
		}
#endif
	}
	color = ( pWork->screen.a << 24 ) | ( pWork->screen.b << 16 ) | ( pWork->screen.g << 8 ) | pWork->screen.r;
    switch ( SK_GameOverMode ){
	case GAME_OVER_NORMAL :
 	case SK_NON_SUPRT :
		TAKABE_SetReduceScreenSize( pWork->screen.mini_screen , ( int )pWork->screen.xy1.vx , ( int )pWork->screen.xy1.vy , 
									( int )pWork->screen.xy2.vx , ( int )pWork->screen.xy2.vy , color );	
		break;
	case GAME_OVER_BOMB :
	case GAME_OVER_WATER :
	case GAME_OVER_SUB :
      SPR_SHOW( pWork->special_view );
		pWork->special_view->sprite.pos.x = pWork->screen.xy1.vx;
		pWork->special_view->sprite.pos.y = pWork->screen.xy1.vy;
		pWork->special_view->sprite.dw = pWork->screen.xy2.vx - pWork->screen.xy1.vx;
		pWork->special_view->sprite.dh = pWork->screen.xy2.vy - pWork->screen.xy1.vy;
	}
}

#define WAIT_TIME (DIRECT_TICK(150)) 
static void SK_StreamControl( Work *pWork ) // Stream
{
	if ( pWork->flag & SK_STREAM_PLAY_OK ){
		return;
	}
	if ( ( GM_StreamStatus( pWork->stream_handler ) == GM_STREAM_STATE_PLAY ) ||
		 ( GM_StreamStatus( pWork->stream_handler ) == GM_STREAM_STATE_END ) || 
		 ( pWork->timer >= WAIT_TIME ) ){
		pWork->timer = WAIT_TIME;
		pWork->flag |= SK_STREAM_PLAY_OK;
		pWork->act = ( void * )AnimationAct;
	} else {
		pWork->timer++;
	}
}

static void NormalAct( Work *pWork )
{
#ifdef KP_XBOX
	if( pWork->confirm != 0 ){
		if( pWork->confirm == -1 ){
			/* 確認画面起動中 */
			/* 文字をフェードアウト */
			pWork->text_alpha -= 3.5f;
			if( pWork->text_alpha < 0.0f ){
				pWork->text_alpha = 0.0f;
			}
			___MENU_Color( pWork->menuprint_work_ptr, 1, 196, 196, 196, ( u_char )pWork->text_alpha );
			pWork->con_cnt = 0;
			return;
		} else if( pWork->confirm == 2 ){
			/* NO選択確認画面フェードアウト始め */
			pWork->con_cnt ++; 
			if( pWork->con_cnt > DIRECT_TICK(30) ){
				pWork->text_alpha += 3.5f;
				if( pWork->text_alpha > 128.0f ){
					pWork->text_alpha = 128.0f;
				}
				___MENU_Color( pWork->menuprint_work_ptr, 1, 196, 196, 196, ( u_char )pWork->text_alpha );
			}
			return;
		} else if( pWork->confirm == 3 ){
			/* NO選択 */
			/* ゲームオーバー画面に戻ってくる */
			/* 通常のNormalActに動作させるので、何もしない */
			pWork->text_alpha = 128.0f;
			___MENU_Color( pWork->menuprint_work_ptr, 1, 196, 196, 196, ( u_char )pWork->text_alpha );
			pWork->confirm = 0;
		} else if( pWork->confirm == 1 ){
			/* YESを選択 */
			return ;
		}
	}
#endif

	if( pWork->flag & SK_RESTART ){
		if ( ( GM_StreamIsPlay() == 0 ) || ( pWork->stream_handler < 0 ) || ( pWork->end_limit_count >= END_LIMIT ) ){
			pWork->action = 2;
			// l2d が 再生可能になるまでまつ
#ifdef DEBUG
			printf("l2d ready chack!!!!!\n");
#endif
			if ( L2D_ActionStatus( pWork->handle_2d ) != L2D_STAT_BUSY ){
				pWork->flag |= SK_RESTART_OK;
#ifdef DEBUG
				printf("l2d ready ok!!!!!!!\n");
#endif
			}
			if ( !( pWork->flag & SK_RESTART_OK ) ){
				return;
			}
#ifdef DEBUG
			printf("all ok!!!\n");
#endif
			pWork->flag |= SK_TEXT_FADE_OUT;
			pWork->flag &= ~SK_TEXT_FADE_IN;
			L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
			pWork->act = ( void * )AnimationAct;
			return;
		} else {
			pWork->end_limit_count++;
#ifdef DEBUG
			printf(" GM_StreamIsPlay() = %d %d\n" ,  GM_StreamIsPlay() , pWork->end_limit_count );
#endif
		}			
	}else{
#if 0
		printf("Game Over Select Start\n");
#endif
	}

	PadAct( pWork );
	Update( pWork );
	DotControl( pWork );
	FontUpdate( pWork );
}

static void Act( Work *pWork )
{
//	SK_StreamControl( pWork );
 	pWork->act( pWork );
	if ( pWork->flag & SK_PIC_DISP_OK ){
		ScreenDisp( pWork );
	}
	if ( pWork->flag & SK_MINI_FONT_OK ){
		MiniMiniFontPhase1Update( pWork );
		MiniMiniFontPhase2Update( pWork );
	}
	TextAlphaControl( pWork );
}

static int AnimationAct( Work *pWork ) // 通常実行
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( pWork->flag & SK_RESTART ){
		DotControl( pWork );
	}
	if ( status != L2D_STAT_BUSY ){
		if ( status == L2D_STAT_ACK ){
			switch ( pWork->action ){ //
			case 0 :
				pWork->act = ( void * )StreamRetry;
				L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
				pWork->action++;
				break;
			case 1 :
				SprInit( pWork );
				// screen 終点決定
				pWork->screen.last_xy1.vx = SCREEN_DEFAULT_POS_X;
				pWork->screen.last_xy1.vy = SCREEN_DEFAULT_POS_Y;
				pWork->screen.last_xy2.vx = SCREEN_DEFAULT_POS_X + SCREEN_DEFAULT_WIDTH;
				pWork->screen.last_xy2.vy = SCREEN_DEFAULT_POS_Y + SCREEN_DEFAULT_HEIGHT;
				pWork->flag |= SK_SCREEN_MOVE_START;
				pWork->act = (void *)NormalAct;
				// action ( black fade )
				L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
				pWork->action++;
				break;
			case 2 :
			    GM_GameOverRestart( pWork->position ) ;
			    GV_DestroyActor( pWork );
				break;
			default :
				L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
				pWork->action++;
				break;
			}
		}
	}
	return ( 0 );
}

#define SIG_SHOW_PIC (10813544)
#define SIG_SELECT_OK (14798363)
#define SIG_PRINT_OK (1581640)
static void Signal( void *work , int sign , int value )
{
	Work *pWork;

	pWork = ( Work * )work;

	switch ( sign ){
	case SIG_SHOW_PIC :
		pWork->flag |= SK_PIC_DISP_OK;
		break;
	case SIG_PRINT_OK :
		pWork->flag |= SK_MINI_FONT_OK;
		break;
	case SIG_SELECT_OK :
		pWork->dot.cursor->sprite.col.a = 128;
		pWork->flag |= SK_SELECT_OK | SK_FLUSH_OK | SK_DOT_FADE_IN_START;
		pWork->flag &= ~SK_SCREEN_MOVE_START;
		break;
	}
}

static void StreamRetry( Work *pWork ) // Stream Retry機構
{
	if ( GM_GameOverVox >= 0 ){
		pWork->stream_handler = GM_VoxStream( GM_GameOverVox , GM_STREAM_NO_PAUSE );
		if ( pWork->stream_handler < 0 ){
#ifdef DEBUG_MODE
			printf("Stream Retry---------- %d\n" , pWork->stream_handler );
#endif
			if ( pWork->retry_count >= DIRECT_TICK( 60 ) ){
#ifdef DEBUG_MODE
				printf("Stream Retry Time Over---------- %d\n" , pWork->stream_handler );
#endif
				pWork->stream_handler = -1 ;
				pWork->act = ( void * )AnimationAct; // Stream 無視
			} else {
				pWork->retry_count++;
			}
		} else {
#ifdef DEBUG_MODE
			printf("Stream Play---------- %d\n" , pWork->stream_handler );
#endif
			pWork->act = ( void * )SK_StreamControl; // 復帰
		}
	} else {
#ifdef DEBUG_MODE
		printf("Stream No Play---------- Err Call Koba4\n");
#endif
		pWork->stream_handler = -1 ;
		pWork->act = ( void * )AnimationAct; // Stream 無視
	}
}

static	void	Die( Work *pWork )
{
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
	switch ( SK_GameOverMode ){
	case GAME_OVER_BOMB :
	case GAME_OVER_WATER :
	case GAME_OVER_SUB :
		SPR_Destroy_2D_Object( pWork->special_view );
		break;
	}
	SK_GameOverMode = 0;
	SK_Strcode = 0;
	SK_TriStrcode = 0;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	int		handle;

	// 受け付けなくする
	if ( SK_GameOverMode == GAME_OVER_NORMAL ){
		SK_GameOverMode = SK_NON_SUPRT;
	}
	// main
	work->handle_2d = -1 ;
	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 0, 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		return -1 ;
	}
	work->handle_2d = handle ;

	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 0;
	L2D_SetSignalHandle( work->handle_2d , work , Signal );

	work->act = ( void * )AnimationAct;
	work->position = 0; // default continue
	// mini font
	work->text_count = 0;	
	work->text_pos[ 0 ] = work->text_pos[ 1 ] = 0;
	work->text_alpha = 0.0f;
	// screen
	DG_COPY_VEC( &work->screen.xy1 , &DG_ZeroVector );
	DG_COPY_VEC( &work->screen.xy2 , &DG_ZeroVector );
	DG_COPY_VEC( &work->screen.last_xy1 , &DG_ZeroVector );
	DG_COPY_VEC( &work->screen.last_xy2 , &DG_ZeroVector );
	work->screen.xy1.vx = 64;
	work->screen.xy1.vy = 64;
	work->screen.xy2.vx = 400 + 64;
	work->screen.xy2.vy = 300 + 64;
	work->screen.r = ( u_char )SK_R;
	work->screen.g = ( u_char )SK_G;
	work->screen.b = ( u_char )SK_B;
	work->screen.a = ( u_char )SK_A;
//	work->screen.raute = 20;
	// etc
	work->timer = 0;
	work->stream_flag = 0;
	work->flag = SK_TEXT_FADE_IN;
	work->end_limit_count = 0;
	work->disp_line[ 0 ] = work->disp_line[ 1 ] = 0;
	work->last_num = irnd() % 1000000;
	work->retry_count = 0;

	GV_SetActorChild( work , ( work->screen.mini_screen = NewReduceScreenEffect() )  );
	/* 偽ゲームオーバー用MENU_Printfキャラ生成 */
	work->menuprint_work_ptr = NewMenuPrintManager( 9 * 1024, DG_DMAPACK_MENU, DG_DMAPACK_PHASE_LAST, 140 );
	if ( work->menuprint_work_ptr == NULL ){
		SK_Err("menu print null -> non memory\0");
		return (-1);
	}
	GV_SetActorChild( work, work->menuprint_work_ptr );
	work->tick_back = DG_TickCount;
//	GV_SetActorChild( work , NewPictureDrawManager( 128 ) );

	return 0 ;
}

/*----------------------------------------------------------------*/
#define	ACTOR_PRIO		(254)

void *NewSKGameOver( int mode ) // mode は 飾り
{
	Work		*work ;

	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2 , GV_CLASS_OBJECT , sizeof( Work ), ACTOR_PRIO ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	GM_StreamStopAll() ;

	return work ;
}

// シナリオから特殊ゲームオーバーの設定
void SK_SpecialGameOver( void )
{
	if ( SK_GameOverMode != SK_NON_SUPRT ){
		SK_TriStrcode = GCL_GetOptionValue( 't' , 0 );
		SK_Strcode = GCL_GetOptionValue( 's' , 0 );
		SK_GameOverMode = GCL_GetOptionValue( 'm' , 0 );
	}
}

// タンカー編、プラント編呼びわけ 常駐防止 koba4 command
extern void	*NewGameOverWin2( int mode ) ;
extern void *( *SK_GameOverAct )( int );
extern void *SK_BossResultWakeup_Sub( int );
extern void *NewVRGameOver( int mode );
extern void *SK_BossResultWakeup_Sub2( int );

void GM_GameOverChoice( void )
{
	int choice;

	// default tanker
	choice = GCL_GetOptionValue( 'c' , 0 );

	switch ( choice ){
	case SK_TANKER :
		SK_GameOverAct = NewGameOverWin2;
		break;
	case SK_PLANT :
		SK_GameOverAct = NewSKGameOver;
		break;
	case SK_BOSS_RUSH :
		SK_GameOverAct = SK_BossResultWakeup_Sub;
		break;
	case SK_VR :
		SK_GameOverAct = NewVRGameOver;
		break;
	case SK_BOSS_RUSH2 :
		SK_GameOverAct = SK_BossResultWakeup_Sub2;
		break;
	default :
		SK_GameOverAct = NewGameOverWin2;
		printf("あなたの望終わりはありません\n");
	 	break;
	}
}
