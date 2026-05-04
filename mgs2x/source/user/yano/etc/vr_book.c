//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   vr_book.c
   スネークテイルズ用紙芝居
   
   2002/05/13 YANO.Y
   $Id: vr_book.c,v 1.1.1.3 2002/11/19 11:52:00 Yoshizawa1 Exp $
*/

/*
   TODO:セーブ、ロードは小林さんのプログラムを呼び出す
   void *CODEC_NewSnakeTalesSave( void );//セーブ用

   ////memo////
   game/font.cを改良。
 
   font_draw_string(...)＆FONT_DRAW_OVER
   が真の場合、drawinfo.prev_charに、次の文字へのポインタが入っています。
   （正常終了時はNULL)
   
   #W (必ず半角大文字）も実装しましたが、
   「その場所の次のポインタ」を記録するため、
   「次のページの頭」に置いてください。
   
   {
   		hogehogehoge
   		#Wheggehehaea;
   }

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

#include "libfs.h"

#include <libutl.h>

#include	"gameheader.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../../mode/codec/cjimaku.h"
#include    "../../mode/codec/codec_signal.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include    "font.h"

#ifdef KP_XBOX
#include	"../../kano/xmcman/xmcman.h"
#endif

#include "vr_book.h" //l2d define header

extern void *CODEC_NewSnakeTalesSave( void );

#ifdef KP_XBOX
#define SPR_FTOI( _a ) (int)( _a*16.0f )
#endif

#define PAGE_NUM_MAX 128 //ひとつのリソースにつきこのページ数まで
typedef struct vr_book_RES_INFO {
	char *resource_top;
	char *page[PAGE_NUM_MAX];
	int  str[PAGE_NUM_MAX];
	int  page_num;
	char *main_str[PAGE_NUM_MAX];	/* そのページのmain stringへのポインタ */
} RES_INFO;

typedef	struct vr_book_Work 
{
	GV_ACT_EX  actor ;
	
	int 		phase;
	int 		flag;
	int 		proc_next;/* 終了プロックのID */
	
	int			handle_2d;
	SPR_OBJ 	*root;
	SPR_OBJ 	*prog;
	SPR_OBJ 	*n_font;
	SPR_OBJ 	*msg_area;
	SPR_OBJ 	*arrowR;
	SPR_OBJ 	*arrowL;
	SPR_OBJ 	*num_font[6];
	
	void		*t_work[ 1 ] ;/* テキスト表示デーモンのワークへのポインタ */
	int  		res_num;
	RES_INFO 	*resource;
	int			all_page;		 /* 全ページ数 */
	int			current_all_page;/* 現在のページ数 */
	char 		*current_res;
	int			current_res_num;
	int			current_page_num;

	int 		pic_showed_flag;

	unsigned char 	text_cnt;
	int 			text_alpha;

	SPR_OBJ *pic;
	int 	pic_handle;
	int 	pic_alpha;
	int 	pic_flag;

	SPR_OBJ *fade;
	int 	fade_alpha;

	int		cnt;	/* 背景見せるため、open_bookのタイミングを図る */

	int		msg_tex_width;
////test
	SPR_OBJ *line;
//////

} Work ;

#include "../../mode/menu/text_scn.h"

#include "mode/menu/xtextscn.h"

#define PRIM_CONTROL TextScn_PRIM_CONTROL
#define TexWork TextScn_Work

enum {
	PHASE_FADE = 0,
	PHASE_FADE_WAIT,
	PHASE_STORY_DISP,
	PHASE_L2D_ACTION2,
	PHASE_L2D_ACTION_WAIT2,
	PHASE_SHOW_BG,
	PHASE_DIE_CALL,
};
// プロトタイプ
// extern 
extern void *NewTextScreenControl( void );
/* コールバック関数を設定する */
void MENU_SetCallbackTextTexture( char *( *callback )( FONT_DRAWINFO *dr, char *now, int no ) );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreenF( void *work_ptr, float x1, float y1, float x2, float y2, int u1, int v1, int u2, int v2, int col );
extern void *NewTnkPhotoView( void ) ; 

static char* main_string = NULL;
static int proc_id = 0;
static int proc_id_p = 0;
static GCL_ARGS arg;
static int argv[2];
static int g_resource_num = 0;
static int g_picture_strcode = 0; 
/*
   //// #Cのあとの数字の対応 ////
   0   		:入れ替え文字から本文に戻る
   1-99		:本文から入れ替え文字に移る
   100 		:背景１枚絵をなにも表示しない
   101-199	:背景１枚絵表示
   200		:JPEG画像表示
   201-299	:特殊1枚絵表示(F.O. １枚絵表示 F.I.)
*/
#if BP_USE_NEW_FONT_SYSTEM()
static char* StringReplace( BP_FONT_DRAWINFO *dr, char* now, int no )
#else
static char* StringReplace( FONT_DRAWINFO *dr, char* now, int no )
#endif
{
	if( no == 200 ){
		/* JPEG画像表示 */
		g_picture_strcode = 1;/* このstrcodeはありえないので代用 */

		return now;

	} else if( no >= 201 ){
		/* 特殊1枚絵表示 */
		int num;
		num = no - 200;
		num *= -1;/* 正負逆転 */
		g_picture_strcode = num;/* 負のstrcodeはありえないので代用 */

		return now;

	} else if( no >= 100 ){/* 一枚絵表示 *//* #C100はなにも表示しない。#C101～対応する絵を表示 */
		int str, num;
		//printf("g_resource_num[%d]\n",g_resource_num);
		num = no - 100;

		if( num == 0 ){
			g_picture_strcode = 0; /* なにも表示しない */
			printf("num[%d]\n",num);
			return now;
		}
		
		argv[0] = num;		
		argv[1] = g_resource_num;
		arg.argc = 2;
		arg.argv = argv;
		str = GCL_ExecProc( proc_id_p, &arg );
		printf("g_str[%d]\n",str);
		if( str != 0 ){
			g_picture_strcode = str;
		}
		
		return now;

	} else if( no != 0 ){/* 入れ替え文字の方へ *//* 1～99まで */		
		char *res;
		argv[0] = no;
		argv[1] = g_resource_num;
		arg.argc = 2;
		arg.argv = argv;
		res = BP_GCL_LOOKUP_NEW_FONT_STRING((char*)GCL_ExecProc( proc_id, &arg ));
		main_string = now;
		return res;
	}
	if( no == 0 ){/* 本文へ */
		return main_string;
	}

	return NULL;
}
static char* StringReplace2( FONT_DRAWINFO *dr, char* now, int no ){
	if( no >= 100 ){/* 一枚絵表示用 */
		return now;

	} else if( no < 100 && no != 0 ){/* 入れ替え文字の方へ *//* 1～99まで */		
		char *res;
		argv[0] = no;
		argv[1] = g_resource_num;
		arg.argc = 2;
		arg.argv = argv;
		res = BP_GCL_LOOKUP_NEW_FONT_STRING((char*)GCL_ExecProc( proc_id, &arg ));
		main_string = now;
		return res;
	}
	if( no == 0 ){/* 本文へ */
		return main_string;
	}

	return NULL;
}
#define MSG_BASE_X ( 80 )
#define MSG_BASE_Y ( 17 )
//#define SYS_FONT_WIDTH ( 24 )
//#define SYS_FONT_HEIGHT ( 24 )

#define MSG_WIDTH (370)
#define MSG_HEIGHT (340)
#define MSG_LINE_X   (21)
#define MSG_LINE_Y   (11)

#define MSG_TEX_XTOP  0
#define MSG_TEX_YTOP  0
//#define MSG_TEX_YTOP  12
#define MSG_TEX_PITCH 1
#define MSG_TEX_SPACE 12
//#define MSG_TEX_WIDTH  (MSG_LINE_X*(FONT_SIZE_W+MSG_TEX_PITCH))
#define MSG_TEX_WIDTH  (work->msg_tex_width)

#define MSG_TEX_LINES 10

#if 0 //BP_PS2
#define MSG_TEX_HEIGHT (MSG_LINE_Y*(FONT_SIZE_H+MSG_TEX_SPACE)+DOWN_MARGINE)

#else
#define MSG_TEX_HEIGHT (BP_FONT_CONVERT_NEW_Y(BP_FONT_BUFFER_HEIGHT(MSG_LINE_Y, BP_RUBI_SIZE_H)))

#endif

#ifdef PSX2
#define PAD_OKU (PAD_R | PAD_A)
#define PAD_MOD (PAD_L | PAD_B)

#define PAD_OKU_NOT_JP (PAD_R | PAD_B)
#define PAD_MOD_NOT_JP (PAD_L | PAD_A)



#define PAD_HAYA PAD_X
#define PAD_SAVE PAD_SEL
#define PAD_EXIT PAD_Y
#define PAD_SHOW PAD_R1
#else /* KP_XBOX */
#define PAD_OKU (PAD_R | PAD_B)
#define PAD_MOD (PAD_L | PAD_A)
#define PAD_HAYA PAD_X
#define PAD_SAVE PAD_SEL
#define PAD_EXIT PAD_Y
#define PAD_SHOW PAD_R1
#endif

#define BG_SHOW_TIME DIRECT_TICK(60)

//work->pic_flag
#define PIC_SHOW 0x01
#define PIC_ON   0x02
#define PIC_OFF  0x04
#define PIC_HIDE 0x08

//work->flag
#define FLAG_SHOWTIME 0x0001
#define FLAG_NOSAVE   0x0002

static void PageOkuri( Work *work );
static void PageModori( Work *work );

static int ReceiveSignal(Work *work, int signal, int value)
{	// ADD M.Kobayashi 2002/06/15		
	// セーブキャラが出すシグナルを処理するハンドラ
	int ret = 0;

	switch(signal)
	  {
		case CDC_SIGNAL_SAVE_DIE:/* 0x02 */
		  /* セーブ画面終了 */
		  ASSERT( work->text_cnt == 5 );
		  //work->text_cnt = 2;/* 1フレ待つ処理をはさむ 2002.09.16 YANO */
		  work->text_cnt = 6;
		  GV_SetActorSignalFunc( work, NULL );
		  
		  /* L2Dの表示を見せる */
		  SPR_SHOW( work->root );
		  break;
		default:
		  ret = GV_DefaultSignalFunc(work, signal, value);
		  break;
	  }
	return ret;
}

static int ReceiveSignal2(Work *work, int signal, int value)
{
	switch( signal ){
	  case 1:
		/* JPEG表示終了 */
		ASSERT( work->text_cnt == 5 );
		work->text_cnt = 0;
		GV_SetActorSignalFunc( work, NULL );
	  
		/* L2Dの表示を見せる */
		SPR_SHOW( work->root );
	  
		/* 特殊1枚絵を見た */
		work->pic_showed_flag = 1;

		/* ページを進める */
		PageOkuri( work );
		work->text_cnt = 0;		/* 文字を消す必要がないので */
		break;
	  case 2:
		/* JPEG表示終了 */
		ASSERT( work->text_cnt == 5 );
		work->text_cnt = 0;
		GV_SetActorSignalFunc( work, NULL );
	  
		/* L2Dの表示を見せる */
		SPR_SHOW( work->root );
	  
		/* 特殊1枚絵を見た */
		work->pic_showed_flag = 1;

		/* ページを戻る */
		PageModori( work );
		work->text_cnt = 0;		/* 文字を消す必要がないので */
		break;
	}

	return 0;
}

static void PageOkuri( Work *work )
{
	work->current_page_num ++; /* 1ページ進める */
	work->current_all_page ++;
	SPR_SHOW( work->arrowL );
	SPR_SHOW( work->arrowR );

	if( work->current_page_num < work->resource[work->current_res_num].page_num ){
		/* 同じリソースのページを進める */
		work->current_res = work->resource[work->current_res_num].page[work->current_page_num];
		work->text_cnt = 4;

		GM_SdSet( SD_A_PAGEB001 );

	} else if( (work->current_res_num + 1) < work->res_num ){
		/* 次のリソースに移る */
		work->current_res_num ++; /* 1リソース進める */
		g_resource_num = work->current_res_num;	/* グローバルの変数に入れる */
		work->current_res = work->resource[work->current_res_num].page[0];
		work->current_page_num = 0;
		work->text_cnt = 4;

		GM_SdSet( SD_A_PAGEB001 );

   } else 
   {
		/* 紙芝居の終わりだった! */
		//TODO: おわりのL2Dアクション再生　SPR_SHOW
		work->current_page_num --;
		work->current_all_page --;/* ページ数をもとに戻す */
      /* 最終ページは送りボタンで */
		if( ((BP_Area_JP()&&GV_PadDataDirect[0].status&PAD_OKU)||work->pic_showed_flag==1) || 
          ((!BP_Area_JP()&&GV_PadDataDirect[0].status&PAD_OKU_NOT_JP)||work->pic_showed_flag==1) )
      {
         
         /* 特殊1枚絵の時は強制的に終了 */
			work->phase = PHASE_L2D_ACTION2;
			/* サウンドフェード開始 */
			GM_SdSet( SNG_FOUTS_S );
		}
	}

		
	SPR_SetColorSprite( work->arrowR, 10, 10, 80, 128 );/* 矢印を光らせる */
	if( work->current_all_page == work->all_page ){/* 最終ページだったら右矢印を消す */
		SPR_HIDE( work->arrowR );
	}
	if( work->current_all_page == 1 ){/* １ページ目だったら左矢印を消す */
		SPR_HIDE( work->arrowL );
	}

	/* 特殊1枚絵を見たフラグを初期化 */
	work->pic_showed_flag = 0;

	
	return;
}

static void PageModori( Work *work )
{
	work->current_page_num --; /* 1ページ戻る */
	work->current_all_page -= 1;
	SPR_SHOW( work->arrowL );
	SPR_SHOW( work->arrowR );
	 
	if( work->current_page_num >= 0 ){
		/* 同じリソースのページを戻る */
		work->current_res = work->resource[work->current_res_num].page[work->current_page_num];
		work->text_cnt = 4;
		/* main_stringの値を元に戻す */
		main_string = work->resource[work->current_res_num].main_str[work->current_page_num];

		GM_SdSet( SD_A_PAGEB001 );

	} else if( (work->current_res_num - 1) >= 0 ){
		/* 前のリソースに移る */
		int last_page = work->resource[work->current_res_num-1].page_num - 1;
		work->current_res_num --; /* 1リソース戻る */
		g_resource_num = work->current_res_num;	/* グローバルの変数に入れる */
		work->current_res = work->resource[work->current_res_num].page[last_page];
		work->current_page_num = last_page;
		work->text_cnt = 4;
		/* main_stringの値を元に戻す */
		main_string = work->resource[work->current_res_num].main_str[work->current_page_num];

		GM_SdSet( SD_A_PAGEB001 );

	} else {
		/* 紙芝居の一番最初 */
		// 変わらない。なにもしない。
		  work->current_page_num = 0; /* ページ数だけもとに戻す */
		work->current_all_page = 1;

	}
	
	SPR_SetColorSprite( work->arrowL, 10, 10, 80, 128 );/* 矢印を光らせる */
	if( work->current_all_page == 1 ){/* １ページ目だったら左矢印を消す */
		SPR_HIDE( work->arrowL );
	}
		
	/* 特殊1枚絵を見たフラグを初期化 */
	work->pic_showed_flag = 0;

	return ;
}

static void StoryDisplay( Work *work )
{
	switch( work->text_cnt ){
	  case 0:
		/* テキスト表示初期化 */
		MENU_SetCallbackTextTexture( StringReplace2 ); /* Createするときは必ず呼ぶ */
		MENU_ClearTextTexture( work->t_work[ 0 ] );
		if( GM_Language == GM_LANG_JAPANESE ){
			MENU_CreateTextTexture( work->t_work[0], MSG_TEX_XTOP, MSG_TEX_YTOP, 
								   MSG_TEX_WIDTH, MSG_TEX_HEIGHT,
								   MSG_TEX_PITCH, MSG_TEX_SPACE, MSG_TEX_LINES, work->current_res );
		} else {
			/* 日本語以外は字間を詰める */
			MENU_CreateTextTexture( work->t_work[0], MSG_TEX_XTOP, MSG_TEX_YTOP, 
								   MSG_TEX_WIDTH, MSG_TEX_HEIGHT,
								   0 /*MSG_TEX_PITCH*/, MSG_TEX_SPACE, MSG_TEX_LINES, work->current_res );
		}
		work->text_alpha = 128;
		work->text_cnt = 1;		/* カウンタの初期化(0->表示初期化 1->インターバル 2->表示) */
		break;
	  case 1:
		/* インターバル */ // 
		  work->text_cnt = 2;
		break;
	  case 2:
		/* 文字表示 */
		{
			u_int color;
				
			color = 0x005A5A5A | (work->text_alpha << 24); /* R:90 G:90 B:90 A:work->text_alpha */
			//color = 0x805A5A5A;	/* R:90 G:90 B:90 A:128 */
			MENU_PutTextScreenF( work->t_work[0], 
								 MSG_BASE_X, MSG_BASE_Y, MSG_BASE_X+MSG_WIDTH, MSG_BASE_Y+MSG_HEIGHT,
								 0 , 0 , MSG_TEX_WIDTH, MSG_TEX_HEIGHT, color );
		}


		SPR_SetColorSprite( work->arrowR, 128, 128, 128, 128 );
		SPR_SetColorSprite( work->arrowL, 128, 128, 128, 128 );


		if( work->flag & FLAG_SHOWTIME ){/* １枚絵が前面にでている時は以下の部分は不要 */
			break;
		}
		
		if( !(work->flag & FLAG_NOSAVE) ){//セーブできないときもある
			if( GV_PadDataDirect[0].press & PAD_SAVE ){ /* セーブ */
				// MODIFY M.Kobayashi 2002/06/25
				// 起動したアクターを子どもにしてシグナルハンドラを設定
				// 状態 text_cnt を 5 に移行
				GV_SetActorChild( work, CODEC_NewSnakeTalesSave() ); /* セーブ画面呼び出し */
				GV_SetActorSignalFunc( work, ReceiveSignal );
				work->text_cnt = 5;
				
				GM_SdSet( SD_S_WIN01 );

				break;
			}
		}

//      case COsContext::kBS_Japan:


		/* パッド監視 *//* 文字表示以外のフェーズではパッド情報無視 */
		if( GV_PadDataDirect[0].status & PAD_HAYA )
      { 
         /* 早送り */
			PageOkuri( work );
		
		} 
      else if( GV_PadDataDirect[0].release & PAD_HAYA )
      {
			/* 離したら通常モード */
			// なにもしません

		} else if( (BP_Area_JP()&&GV_PadDataDirect[0].press&PAD_OKU) || (!BP_Area_JP()&&GV_PadDataDirect[0].press&PAD_OKU_NOT_JP) )
      { 
         /* 次ページへ */			
         PageOkuri( work );

		} else if( (BP_Area_JP()&&GV_PadDataDirect[0].press & PAD_MOD) || (!BP_Area_JP()&&GV_PadDataDirect[0].press&PAD_MOD_NOT_JP) )
      { 
         /* 前ページへ */
  			PageModori( work );
		}

		/* 一枚絵 *//* 表示非表示管理はここで */
		//printf("[%d][%d]\n",work->current_res_num,work->current_page_num);
		if( work->resource[work->current_res_num].str[work->current_page_num] != 0 ){
			if( work->resource[work->current_res_num].str[work->current_page_num] == 1 ){
				if( work->pic_showed_flag == 0 ){
					/* JPEG表示 */					
					//重野さんのプログラム呼び出し口
					void *ptr;
					ptr = NewTnkPhotoView();
					if( ptr != NULL ){
						GV_SetActorChild( work, ptr );
						GV_SetActorSignalFunc( work, (void *)ReceiveSignal2 );
						work->text_cnt = 5; /* セーブ画面と同様に何もしない状態にする*/
					}
				}
			} else if( work->resource[work->current_res_num].str[work->current_page_num] < 0 ){
				if( work->pic_showed_flag == 0 ){
					/* 特殊1枚絵表示 */
					extern void *NewShowPictureForVRBook( int mode, int strcode );
					void *ptr;
					int code;

					code = work->resource[work->current_res_num].str[work->current_page_num];
					switch( code ){
					  case -1:
						ptr = NewShowPictureForVRBook( 0, GV_StrCode("jpnw_news_gollgon") );
						break;
					  case -2:
						ptr = NewShowPictureForVRBook( 0, GV_StrCode("jpnw_news_genora") );
						break;
					  default:
						ptr = NewShowPictureForVRBook( 0, GV_StrCode("jpnw_news_gollgon") );
						break;
					}
					GV_SetActorChild( work, ptr );
					GV_SetActorSignalFunc( work, ReceiveSignal2 );
					work->text_cnt = 5; /* セーブ画面と同様に何もしない状態にする*/
				}
			} else if( work->pic_flag & PIC_HIDE ){
				/* 表示 */
				SPR_OBJ *pic = work->pic;
				int handle = work->pic_handle;
				int strcode;
			
				//printf("ichi[%d]\n",strcode);
				strcode = work->resource[work->current_res_num].str[work->current_page_num];
				SPR_ObjSetTexture( pic, strcode, handle );	
				SPR_SHOW( pic );
				work->pic_alpha = 0;
				work->pic_flag = PIC_ON; /* ON */
			}
		} else {
			if(  work->pic_flag & PIC_SHOW ){
				/* 非表示 */
				SPR_OBJ *pic = work->pic;
				work->pic_flag = PIC_OFF; /* OFF */
				//SPR_HIDE( pic );
			}
		}
		break;
	  case 4:
		/* 消える文字はフェードしてから消す */
		{
			u_int color;
			//int 	alpha;
				
			work->text_alpha -= 30;
			if( GV_PadDataDirect[0].status & PAD_HAYA ){/* 早送り中はフェードしない */
				work->text_alpha = 0;
			}
			if( work->text_alpha <= 0 ){
				work->text_alpha = 0 ;
				work->text_cnt = 0;
			}
			color = 0x005A5A5A | (work->text_alpha << 24); /* R:90 G:90 B:90 A:work->text_alpha */
			MENU_PutTextScreenF( work->t_work[0], 
								 MSG_BASE_X, MSG_BASE_Y, MSG_BASE_X+MSG_WIDTH, MSG_BASE_Y+MSG_HEIGHT,
								 0 , 0 , MSG_TEX_WIDTH, MSG_TEX_HEIGHT, color );

		}

		break;
// ADD M.Kobayashi 2002/06/15		
	  case 5:
	    {
			// セーブ画面中処理
			// ハンドラがこの状態からの遷移を実行する
			// （ポーリングで書いたほうがわかりやすいが、
			//   オリジナル本編のセーブ処理とそろえておく）
			/* L2Dをセーブ中は消しておく */  
			SPR_HIDE( work->root );
		}
		break;
	  case 6:
	    {
			/* セーブから戻ってきた時、1フレーム待つ 2002.09.16 YANO */
			work->text_cnt = 2;
		}
		break;
	}
	return ;
}
	
static void StoryDisplayFade( Work *work )
{
	switch( work->text_cnt ){
	  case 0:
		/* テキスト表示初期化 */
		MENU_SetCallbackTextTexture( StringReplace2 ); /* Createするときは必ず呼ぶ */
		MENU_ClearTextTexture( work->t_work[ 0 ] );
		if( GM_Language == GM_LANG_JAPANESE ){
			MENU_CreateTextTexture( work->t_work[0], MSG_TEX_XTOP, MSG_TEX_YTOP, 
								   MSG_TEX_WIDTH, MSG_TEX_HEIGHT,
								   MSG_TEX_PITCH, MSG_TEX_SPACE, MSG_TEX_LINES, work->current_res );
		} else {
			/* 日本語以外は字間を詰める */
			MENU_CreateTextTexture( work->t_work[0], MSG_TEX_XTOP, MSG_TEX_YTOP, 
								   MSG_TEX_WIDTH, MSG_TEX_HEIGHT - 9 * MSG_TEX_PITCH,
								   0/*MSG_TEX_PITCH*/, MSG_TEX_SPACE, MSG_TEX_LINES, work->current_res );
		}
		work->text_cnt = 1;		/* カウンタの初期化(0->表示初期化 1->インターバル 2->表示) */
		break;
	  case 1:
		/* インターバル */ // 
		  work->text_cnt = 2;
		break;
	  case 2:
		/* 文字表示 */
		{
			u_int color;
				
			color = 0x005A5A5A | ( work->text_alpha << 24 ); 
			//color = 0x805A5A5A;	/* R:90 G:90 B:90 A:128 */
			MENU_PutTextScreenF( work->t_work[0], 
								 MSG_BASE_X, MSG_BASE_Y, MSG_BASE_X+MSG_WIDTH, MSG_BASE_Y+MSG_HEIGHT,
								 0 , 0 , MSG_TEX_WIDTH, MSG_TEX_HEIGHT, color );
		}

#if 0		
		/* 一枚絵 *//* 表示非表示管理はここで */
		//printf("[%d][%d]\n",work->current_res_num,work->current_page_num);
		if( work->resource[work->current_res_num].str[work->current_page_num] != 0 ){
			if( work->pic_flag & PIC_HIDE ){
				/* 表示 */
				SPR_OBJ *pic = work->pic;
				int handle = work->pic_handle;
				int strcode;
			
				//printf("ichi[%d]\n",strcode);
				strcode = work->resource[work->current_res_num].str[work->current_page_num];
				SPR_ObjSetTexture( pic, strcode, handle );	
				SPR_SHOW( pic );
				work->pic_alpha = 0;
				work->pic_flag = PIC_ON; /* ON */
			}
		} else {
			if(  work->pic_flag & PIC_SHOW ){
				/* 非表示 */
				SPR_OBJ *pic = work->pic;
				work->pic_flag = PIC_OFF; /* OFF */
				//SPR_HIDE( pic );
			}
		}
#endif

		break;
	}
	return ;
}	

static void PageNumberDisplay( Work *work )
{
	/* ベージ数表示 */
	{
		int k[6], tmp;
		SPR_OBJ *num;
		int i;
		k[5] = work->all_page % 10;
		tmp = work->all_page / 10;
		k[4] = tmp % 10;
		tmp = tmp / 10;
		k[3] = tmp % 10;
		tmp = tmp / 10;

		k[2] = work->current_all_page % 10;
		tmp = work->current_all_page / 10;
		k[1] = tmp % 10;
		tmp = tmp / 10;
		k[0] = tmp % 10;
		tmp = tmp / 10;

		for( i = 0; i < 6; i++ ){
			SPR_FIX pu, pv;
			SPR_FIX u, v;
			num = work->num_font[i];
			pu = num->sprite.head.tex.pu;
			pv = num->sprite.head.tex.pv;
			u = pu + SPR_FIXED( (k[i]+1)*18 + 0.5f );
			num->sprite.head.tex.u = u;
			//num->sprite.head.tex.h = SPR_FIXED( 11.0f );
			/*
			   printf("%d\n", num->sprite.head.tex.dgtex->tex_id);
			   printf("pu[%f],pv[%f]\n", num->sprite.head.tex.pu,num->sprite.head.tex.pv);
			   printf("pw[%f],ph[%f]\n", num->sprite.head.tex.pw,num->sprite.head.tex.ph);
			   printf("u[%f],v[%f]\n", num->sprite.head.tex.u,num->sprite.head.tex.v);
			   printf("w[%f],h[%f]\n", num->sprite.head.tex.w,num->sprite.head.tex.h);
			   */
		}
	}
	return ;
}

static void ShowBackground( Work *work )
{

		/* １枚絵のフェードインアウト */
		switch( work->pic_flag ){
		  case PIC_HIDE:
		  case PIC_SHOW:
			//SPR_HIDE( work->pic );//何もしない
			break;
		  case PIC_ON:
			work->pic_alpha += 50;
			if( work->pic_alpha > 128 ){
				work->pic_alpha = 128;
				work->pic_flag = PIC_SHOW;
			}
			SPR_SetColorSprite( work->pic, 128, 128, 128, work->pic_alpha );
			break;
		  case PIC_OFF:
			work->pic_alpha -= 4;
			if( work->pic_alpha < 0 ){
				work->pic_alpha = 0;
				work->pic_flag = PIC_HIDE;
				SPR_HIDE( work->pic );
				break;
			}
			SPR_SetColorSprite( work->pic, 128, 128, 128, work->pic_alpha );
			break;
		}

		/* トリガーを押すと下の絵だけが見える */
		if( work->pic_flag == PIC_SHOW ){
			if( GV_PadDataDirect[0].status & PAD_SHOW ){
				/* フェードで見える *//* -->面倒でした。スプライトをもう１枚作る必要あり */
				/* いきなり見える */
				work->flag |= FLAG_SHOWTIME;
#if 0
				work->pic_all_alpha -= 5;
				if( work->pic_all_alpha < 0 ){
					work->pic_all_alpha = 0;
					SPR_HIDE( work->prog );
					SPR_HIDE( work->n_font );
					SPR_HIDE( work->msg_area );
				}				
#endif
				SPR_HIDE( work->prog );
				SPR_HIDE( work->n_font );
				//SPR_HIDE( work->msg_area );/* 不使用になりました */
				work->text_alpha = 0;
			}
			if( GV_PadDataDirect[0].release & PAD_SHOW ){
				/* パッと現れる */
				work->flag &= ~(FLAG_SHOWTIME);
				//work->pic_all_alpha = 128;
				SPR_SHOW( work->prog );
				SPR_SHOW( work->n_font );
				//SPR_SHOW( work->msg_area );/* 不使用になりました */
				work->text_alpha = 128;
			}			
			
		}

		//printf("al[%d]\n",work->pic_alpha);

}

static void Act( Work *work )
{
	int status;
	
	SPR_SHOW( work->root );
	
	//printf("alp[%d]\n",work->fade_alpha);
	//printf("page num =%d\n",work->resource[0].page_num);
	switch( work->phase ){
	  case PHASE_SHOW_BG:
		if( work->cnt > BG_SHOW_TIME ){
			work->phase = PHASE_FADE;
		}

		work->fade_alpha -= 2;
		if( work->fade_alpha < 0 ){
			work->fade_alpha = 0;
		}
		SPR_SetColorSprite( work->fade, 0, 0, 0, work->fade_alpha );
		ShowBackground( work );
		work->cnt++;
		break;
	  case PHASE_FADE:
		if( !(work->flag & FLAG_NOSAVE) ){
			status = L2D_EvokeAction( work->handle_2d, 9790573/* "open_book" 仮！！ */);
		} else{// セーブできない
			status = L2D_EvokeAction( work->handle_2d, L2D_vr_book_ACT_open_book2 );
		}			
		if( status != L2D_STAT_ACK && status != L2D_STAT_BUSY ){
			printf("Book Err!![%d]\n",status);
			ASSERT( 0 );
		}

		work->fade_alpha -= 2;
		if( work->fade_alpha < 0 ){
			work->fade_alpha = 0;
		}
		SPR_SetColorSprite( work->fade, 0, 0, 0, work->fade_alpha );

		PageNumberDisplay( work );
		
		work->text_alpha += 2;
		StoryDisplayFade( work );

		work->phase = PHASE_FADE_WAIT;
		
		ShowBackground( work );
		SPR_HIDE( work->arrowL );

		break;
	  case PHASE_FADE_WAIT:
		work->fade_alpha -= 2;
		if( work->fade_alpha < 0 ){
			work->fade_alpha = 0;
		}
		SPR_SetColorSprite( work->fade, 0, 0, 0, work->fade_alpha );

		work->text_alpha += 3;
		printf("tex alp %d\n",work->text_alpha);
		if( work->text_alpha > 128 ){
			work->text_alpha = 128;
		}
		StoryDisplayFade( work );

		PageNumberDisplay( work );

		SPR_HIDE( work->arrowL );

#if 0		  
		if( work->fade_alpha == 0 ){
			work->fade_alpha = 0;
			work->text_alpha = 128;
			work->phase = PHASE_STORY_DISP;
		}
#else
		if( work->text_alpha == 128 ){
			work->fade_alpha = 0;
			work->text_alpha = 128;
			work->phase = PHASE_STORY_DISP;
		}
#endif
		ShowBackground( work );

		break;
	  case PHASE_STORY_DISP:
		StoryDisplay( work );
		PageNumberDisplay( work );
		ShowBackground( work );

		break;
	  case PHASE_L2D_ACTION2:
		/* 終了L2Dアクション再生 */
		//status = L2D_EvokeAction( work->handle_2d, 15884263/* "close_book" */);
		//if( status != L2D_STAT_ACK && status != L2D_STAT_BUSY ){
		//	printf("Book Err!![%d]\n",status);
		//	ASSERT( 0 );
		//}

		work->fade_alpha += 2;
		SPR_SetColorSprite( work->fade, 0, 0, 0, work->fade_alpha );
		SPR_SHOW( work->fade );

		//PageNumberDisplay( work );
		
		work->text_alpha = 128;
		//work->text_cnt = 0;
		StoryDisplayFade( work );
		  
		work->phase = PHASE_L2D_ACTION_WAIT2;
		break;
	  case PHASE_L2D_ACTION_WAIT2:
		work->fade_alpha += 2;
		if( work->fade_alpha > 128 ){
			work->fade_alpha = 128;
		}
		SPR_SetColorSprite( work->fade, 0, 0, 0, work->fade_alpha );

		work->text_alpha -= 2;
		//printf( "t_alp[%d]\n",work->text_alpha);
		if( work->text_alpha < 0 ){
			work->text_alpha = 0;
		}
		StoryDisplayFade( work );

		//PageNumberDisplay( work );

		if( work->fade_alpha >= 128 && work->text_alpha <= 0 ){
			work->fade_alpha = 128;
			work->text_alpha = 0;

#if 0 /* 1フレ早く死んでいたので遅らす 2002.09.03 */
			GCL_ExecProc( work->proc_next, NULL );/* 終了ブロック呼び出し */
			GV_DestroyActor( work );
#else
			work->phase = PHASE_DIE_CALL;
#endif
		}

		break;
	  case PHASE_DIE_CALL:
			GCL_ExecProc( work->proc_next, NULL );/* 終了ブロック呼び出し */
			GV_DestroyActor( work );
		break;
		
	  default:
		printf("ERR vr_book.c\n");
	}

	if( work->all_page == 1 ){
		/* 全ページ数が1ページだけだったら右矢印も消す */
		SPR_HIDE( work->arrowR );
	}
}

static	void	Die( Work *pWork )
{
	printf("DIE book\n");

	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
	if( pWork->resource != NULL ){
		GV_Free( pWork->resource );
	}

	MENU_SetCallbackTextTexture( NULL );
	
	/* static変数の初期化 */
	main_string = NULL;
	proc_id = 0;
	proc_id_p = 0;/* １枚絵タイミングブロック */
	g_resource_num = 0;
	//g_picture_strcode = 0;

}


static	int	GetResources( Work *work )
{
	int		handle;
	int          i;
	int 	str_l2d;
	SPR_OBJ *spr;
	int  res;


	/* L2Dデータ初期化 */
	str_l2d = GCL_GetOptionValue( 's', 2287797/* vr_book */ );
	handle = L2D_LoadLayout2( str_l2d, DG_CHANL_MENU , DG_DMAPACK_PHASE_AFTER, 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		printf("ERR handle %d\n" , handle );
		return -1 ;
	}
	printf("handle %d\n", handle);
	res = L2D_EvokeAction( handle , 566267 );
	if( res < 0 ){
		printf("!!![%d]\n",res);
		res = L2D_EvokeAction( handle , 566267/* DefaultAction */ );
		if( res < 0 ){
			//ASSERT( 0 );
		}
	}
	work->handle_2d = handle;
	spr = L2D_GetObject( work->handle_2d, 2770484/* root */ );
	if ( spr == NULL ){
		printf("root strcode違うよ!!!!\n");
		return -1 ;
	}
	work->root = spr;
	spr = L2D_GetObject( work->handle_2d, 2602428/* PA_left */ );
	if ( spr == NULL ){
		printf("PA_left strcode違うよ!!!!\n");
		return -1 ;
	}
	work->arrowL = spr;
	spr = L2D_GetObject( work->handle_2d, 5814905/* PA_right */ );
	if ( spr == NULL ){
		printf("PA_right strcode違うよ!!!!\n");
		return -1 ;
	}
	work->arrowR = spr;
	spr = L2D_GetObject( work->handle_2d, 5093615/* font_num_100_1 */ );
	if ( spr == NULL ){
		printf("font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->num_font[0] = spr;
	spr = L2D_GetObject( work->handle_2d, 4077807/* font_num_010_1 */ );
	if ( spr == NULL ){
		printf("font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->num_font[1] = spr;
	spr = L2D_GetObject( work->handle_2d, 4046063/* font_num_001_1 */ );
	if ( spr == NULL ){
		printf("font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->num_font[2] = spr;
	spr = L2D_GetObject( work->handle_2d, 3642219/* font_num_100 */ );
	if ( spr == NULL ){
		printf("font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->num_font[3] = spr;
	spr = L2D_GetObject( work->handle_2d, 3641227/* font_num_010 */ );
	if ( spr == NULL ){
		printf("font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->num_font[4] = spr;
	spr = L2D_GetObject( work->handle_2d, 3641196/* font_num_001 */ );
	if ( spr == NULL ){
		printf("font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->num_font[5] = spr;
	
	spr = L2D_GetObject( work->handle_2d, 2741831/* Prog */ );
	if ( spr == NULL ){
		printf("prog strcode違うよ!!!!\n");
		return -1 ;
	}
	work->prog = spr;

	spr = L2D_GetObject( work->handle_2d, 9866770/* null_font */ );
	if ( spr == NULL ){
		printf("null_font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->n_font = spr;

	spr = L2D_GetObject( work->handle_2d, 2480789/* msg_area */ );
	if ( spr == NULL ){
		printf("null_font strcode違うよ!!!!\n");
		return -1 ;
	}
	work->msg_area = spr;

	//L2D_SetSignalHandle( work->l2d[ i ].handle_2d , work , Signal );

	/* シナリオフラグ取得 */
	{
		int save;
		save = GCL_GetOptionValue( 'l' , 0 ); //セーブ項目を削るフラグ
		if( save != 1 ){
			save = 0;
		}
		if( save == 1 ){
			work->flag |= FLAG_NOSAVE;
		}
		
	}

	/* 終了プロック取得 */
	work->proc_next = GCL_GetOptionValue( 'p' , 0 ); // 終了procの読み込み;
	proc_id = GCL_GetOptionValue( 'e' , 0 ); // exec procの読み込み;
	proc_id_p =  GCL_GetOptionValue( 'i', 0 );
	printf("proc_id [%d]\n",proc_id);

	/*
	   TODO: リソースを全部読み込んで、全ページ数算出、タグつけを行う。
	 */
	/* リソース取得 */
	work->res_num = GCL_GetOptionValue( 'n', 0 );
	printf("res num = %d\n",work->res_num );
	work->resource = GV_Malloc( sizeof( RES_INFO ) * work->res_num );
	GCL_GetOption( 'r' );
	for( i =0; i < work->res_num; i++ ){
		//work->resource[i].resource_top = GCL_GetNextResource();
		work->resource[i].resource_top = BP_GCL_LOOKUP_NEW_FONT_STRING((char*)GCL_GetNextInt());
	}	
	/* テキスト表示デーモン初期化 */
	for( i =0; i < 1; i++ ){
		work->t_work[i] = NewTextScreenControl();
		if ( work->t_work[i] == NULL ){
			return (-1);
		}
		if( GM_Language == GM_LANG_JAPANESE )
      {
         TexWork* pWork = (TexWork*)work->t_work[i];
         pWork->bp_vinfo.max_lines = MSG_TEX_LINES;
#if BP_USE_NEW_FONT_SYSTEM()
         pWork->bp_vinfo.flag &= ~FONT_NO_KINSOKU;
#else
			pWork->vinfo.flag &= ~FONT_NO_KINSOKU;
#endif
		}

		MENU_ClearTextTexture( work->t_work[ i ] );
		GV_SetActorChild( work , work->t_work[ i ] );
	}
	/* 全ページ検索 */
	
   if( GM_Language == GM_LANG_JAPANESE )
   {
		/* 日本語の場合禁則分一文字分多くなる */
#if 0 //BP_PS2
		work->msg_tex_width = (MSG_LINE_X+1)*(FONT_SIZE_W+MSG_TEX_PITCH);
#else
      work->msg_tex_width = BP_FONT_CONVERT_NEW_X(BP_FONT_BUFFER_WIDTH(MSG_LINE_X + 1, BP_FONT_CONVERT_ORIGINAL_X(MSG_TEX_PITCH)));
#endif
	}
   else 
   {
#if 0 //BP_PS2
		work->msg_tex_width = MSG_LINE_X*(FONT_SIZE_W+MSG_TEX_PITCH);
#else
      work->msg_tex_width = BP_FONT_CONVERT_NEW_X(BP_FONT_BUFFER_WIDTH(MSG_LINE_X, BP_FONT_CONVERT_ORIGINAL_X(MSG_TEX_PITCH)));
#endif
	}

	// ひとつのリソースにつき、最大PAGE_NUM_MAXページまで
	for( i = 0; i < work->res_num; i++ )
   {
      char *ch = work->resource[i].resource_top;
      int page_num = 0;

#if BP_USE_NEW_FONT_SYSTEM()
      BP_FONT_VRAMINFO	vinfo = ((TexWork*)work->t_work[0])->bp_vinfo;			/* フォント展開ワーク */
      BP_FONT_DRAWINFO   dr;
      memset( &dr, 0, sizeof( BP_FONT_DRAWINFO ) );
      BP_font_begin_render_texture(&vinfo, 0);
#else
      FONT_DRAWINFO   dr;
		FONT_VRAMINFO	vinfo = ((TexWork*)work->t_work[0])->vinfo;			/* フォント展開ワーク */
      memset( &dr, 0, sizeof( FONT_DRAWINFO ) );
#endif

		if( GM_Language == GM_LANG_JAPANESE )
      {
#if BP_USE_NEW_FONT_SYSTEM()
			vinfo.c_skip = BP_FONT_CONVERT_ORIGINAL_X(MSG_TEX_PITCH);
#else
         vinfo.c_skip = MSG_TEX_PITCH;
#endif
		} 
      else 
      {
			/* 日本語以外は字間をつめる */
			vinfo.c_skip = 0;
		}

      dr.outinfo = &vinfo;

#if BP_USE_NEW_FONT_SYSTEM()
      vinfo.l_skip = BP_FONT_CONVERT_ORIGINAL_Y(MSG_TEX_SPACE);
      vinfo.width  = BP_FONT_CONVERT_ORIGINAL_X(MSG_TEX_XTOP + MSG_TEX_WIDTH);
      vinfo.height = BP_FONT_CONVERT_ORIGINAL_Y(MSG_TEX_YTOP + MSG_TEX_HEIGHT);
      vinfo.max_lines = MSG_TEX_LINES;
      dr.xtop = BP_FONT_CONVERT_ORIGINAL_X(MSG_TEX_XTOP);
      dr.ytop = BP_FONT_CONVERT_ORIGINAL_Y(MSG_TEX_YTOP);
      BP_font_set_draw_callback( &dr, StringReplace );/* コールバック関数設定 */
#else
		vinfo.l_skip = MSG_TEX_SPACE;
		vinfo.width  = MSG_TEX_XTOP + MSG_TEX_WIDTH;
		vinfo.height = MSG_TEX_YTOP + MSG_TEX_HEIGHT;
		dr.xtop = MSG_TEX_XTOP;
		dr.ytop = MSG_TEX_YTOP;
      font_set_draw_callback( &dr, StringReplace );/* コールバック関数設定 */
#endif

      while( 1 )
      {
			int res;
			
			//g_resource_page = page_num;
			/* ページ頭の#C0の戻り先のmain_stringのポインタを記憶させる */
			work->resource[i].main_str[page_num] = main_string;

			work->resource[i].page[page_num] = ch;

#if BP_USE_NEW_FONT_SYSTEM()
         res = BP_font_draw_string(&dr, ch);
#else
			res = font_draw_string( &dr, ch );
#endif

			work->resource[i].str[page_num] = g_picture_strcode;
			if( g_picture_strcode < 0 ){
				/* 特殊1枚絵表示のとき */
				g_picture_strcode = 0;
			}
			
			page_num ++;

			if( res & FONT_RESULT_OVER ){
				if( dr.cr_flag
				   && *dr.prev_char == '\n' ){
					dr.prev_char ++;
					dr.cr_flag = FALSE;
				}
				ch = dr.prev_char;
				printf("続く\n");
			} else {
				if( dr.prev_char == NULL ){
					break;
				}
				printf("draw_err!!\n");
				ASSERT( 0 );
			}
		}

#if BP_USE_NEW_FONT_SYSTEM()
      BP_font_end_render_texture(&vinfo);
#endif
		printf("CC[%d]i %d\n",page_num,i);
		work->resource[i].page_num = page_num;
		work->all_page += page_num;/* 全ページ数 */
	}
	
	work->current_res = work->resource[0].page[0];/* 最初のページ */
	work->current_res_num = 0;
	work->current_page_num = 0;
	work->current_all_page = 1;/* 1ページから始まる */

	/* グローバルの変数に入れる */
	g_resource_num = 0;
	g_picture_strcode = 0;

	{/* 一枚絵 */
		SPR_OBJ *pic;
		
		work->pic_handle = SPR_LoadTexture( GV_StrCode("snake_tales_bg") );
		pic = SPR_Create_2D_Object( SP_SPRITE, DG_CHANL_MENU, work->root );
		work->pic = pic;
		SPR_SET_FLAGS( pic, (SPR_FLAG_ALPHA) );
		SPR_ALPHA( pic, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		{ 
			SPR_POS pos = { 0.0f, 0.0f };
			SPR_SetPosSprite( pic, &pos );
		}
		SPR_SetSizeSprite( pic, (float)SPR_SCRN_WIDTH, (float)SPR_SCRN_HEIGHT );
		SPR_SetColorSprite( pic, 128, 128, 128, 0 );
		SPR_SetPriority( pic, 0 );
		SPR_HIDE( pic );
		work->pic_flag = PIC_HIDE;

		if( work->resource[0].str[0] != 0 ){
			/* １ページ目から背景があった場合の処理 */
			int strcode = work->resource[0].str[0];
			SPR_SHOW( pic );
			SPR_ObjSetTexture( pic, strcode, work->pic_handle );	
			work->pic_flag = PIC_ON;
			work->phase = PHASE_SHOW_BG;/* 背景を見せるフェーズに飛ぶ */
		}
	}

	{
		SPR_OBJ *fade;
		
		fade = SPR_Create_2D_Object( SP_SPRITE, DG_CHANL_MENU, work->root );
		work->fade = fade;
		SPR_SET_FLAGS( fade, (SPR_FLAG_ALPHA) );
		SPR_ALPHA( fade, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		{ 
			SPR_POS pos = { 0.0f, 0.0f };
			SPR_SetPosSprite( fade, &pos );
		}
		SPR_SetSizeSprite( fade, (float)SPR_SCRN_WIDTH, (float)SPR_SCRN_HEIGHT );
		SPR_SetColorSprite( fade, 0, 0, 0, 128 );
		SPR_SetPriority( fade, 7 );
		work->fade_alpha = 128;
		SPR_SHOW( fade );
	}
	


/////test
#if 0
	{
		SPR_OBJ *line;
		work->line = line = SPR_Create_2D_Object( SP_LINESTRIP, DG_CHANL_MENU, work->root );
		SPR_SetLineStripVertexNumber( line, 5 );
		//SPR_SET_FLAGS( line, (SPR_FLAG_ALPHA) );
		//SPR_ALPHA( line, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		SPR_SetPriority( line, 7 );
		for( i = 0; i < 5; i ++ ){
			SPR_POS pos;
			switch( i ){
			  case 0:
				pos.x = (float)work->msg_basex;
				pos.y = (float)work->msg_basey;
				break;
			  case 1:
				pos.x = (float)work->msg_basex + work->msg_width;
				pos.y = (float)work->msg_basey;
				break;
			  case 2:
				pos.x = (float)work->msg_basex + work->msg_width;
				pos.y = (float)work->msg_basey + work->msg_height;
				break;
			  case 3:
				pos.x = (float)work->msg_basex;
				pos.y = (float)work->msg_basey + work->msg_height;
				break;
			  case 4:
				pos.x = (float)work->msg_basex;
				pos.y = (float)work->msg_basey;
				break;
			}
			SPR_SetPosLineStrip( line, i, 1, &pos );
			SPR_SetColorLineStrip( line, i, 128, 0, 0, 128 );
		}
		SPR_SHOW( line );

	}
#endif
/////	

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewVRBook( int name )
{
	Work		*work ;

	printf("VR BOOK start\n");
	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
	if ( work == NULL ) return NULL ;

	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	//work->name = name ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}
