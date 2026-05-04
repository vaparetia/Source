//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   boss_result.c
   boss rush結果表示
   
   2001/09/18	S.Kobayashi
   $Id: boss_result.c,v 1.2 2002/12/04 10:29:00 takaki Exp $
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
#include <string.h>
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
#include    "../../kano/titlescr/subtitle.h"

#ifndef KP_WINDOWS
extern int ClearCodeGenerate( u_char* pDst, int code_len, int dst_radix,
                             void* pSrcbit, int bitlen, int seed );
#else
extern int ClearCodeGenerate( int mode, u_char* pDst, int code_len, int dst_radix,
                             void* pSrcbit, int bitlen, int seed );
#endif

#define		LAYOUT		  (9468020) // boss_rush.l2d 
#define 	STR_NODE_FONT (9853956)
#define 	STR_CODE_FONT (9133060)
#define     STR_DEFAULT   (566267)
#define     STR_PROG      (2741831)
#define     STR_ROOT      (2770484)
#define 	DEFAULT_POS_X ( 33.0f )
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
#define	ACTOR_PRIO		(254)

// over write
#define CODE_MAX (27)
#define SK_CODE_R (190)
#define SK_CODE_G (24)
#define SK_CODE_B (20)

#define SK_R (80)//(56) 
#define SK_G (90)//(66) 
#define SK_B (85)//(61) 
#define SK_A (128)
#define SK_LOW_A (52)

#define SK_SUB_Y (22)

#define STR_OPENRESULT (2313120) // openBossResult
#define STR_CLOSERESULT (14500500) // closeBossResult
#define STR_CODE_FONT (9133060)
#define STR_RESULT_TOTAL (5379316)
#define STR_COLON7_1 (2825392)
#define STR_COLON7_2 (2825393)
#define STR_RAIDENTAG (16613269)
#define STR_SNAKETAG  (15175887)
#define STR_LEVEL_VERY_EASY_JPN (7206544)
#define STR_LEVEL_EASY_JPN (12120551)
#define STR_LEVEL_HARD_JPN (5470032)
#define SPR_CLEARCODEPARTS (4578517)

enum {
	SK_CLEAR_MUGENBANDANA_USED = 0 ,		/* 無限バンダナ使った */
	SK_CLEAR_STEALTH_USED  ,		/* ステルス使った */
	SK_CLEAR_MUGENWIG_USED ,		/* 無限かつら使った */
	SK_CLEAR_WIG_B_USED	   ,		/* 握力かつら使った */
	SK_CLEAR_WIG_A_USED	   ,		/* Ｏ２かつら使った */
};

enum {
	SK_UNUSED = 0 ,
	SK_USED ,
};

enum {
	SK_PLAY_TIME = 0 ,
	SK_SAVE ,
	SK_CONTINUE ,
	SK_ALERTCOUNT ,
	SK_ENEMIES ,
	SK_RATIONS ,
};


// enum -> #define  chaned by T.Morita 2002.05.23
#define	SK_NORMAL                   I64(0x0000000000000001)
#define	SK_INITIALIZE_OK      		I64(0x0000000000000002)
#define	SK_FONT_STANDBY_OK          I64(0x0000000000000004)
#define	SK_FONT_FADE_IN_START       I64(0x0000000000000800)
#define	SK_FONT_FADE_IN_OK          I64(0x0000000000001000)
#define	SK_FONT_FADE_OUT_START      I64(0x0000000000002000)
#define	SK_FONT_FADE_OUT_OK         I64(0x0000000000004000)
#define	SK_CHANCEL                  I64(0x0000000000800000)
#define	SK_ACTION_START             I64(0x0000000400000000)
#define	SK_ACTION_OK                I64(0x0000000800000000)
#define	SK_SIGNAL_OK                I64(0x0000001000000000)
#define	SK_MOVE_OK                  I64(0x0000020000000000)
#define	SK_DOGTAG_ALLGET            I64(0x0000040000000000)
#define	SK_STREAM_PLAY_OK           I64(0x0000100000000000)
#define	SK_STREAM_SET_OK            I64(0x0000200000000000)
#define	SK_END                      I64(0x0000400000000000)


#define SK_SPACE_GET (4096)
#define ASCCI_MAX (32) // plus 1 = tarminater
typedef struct _used_list {
	struct _used_list *pPrev;
	struct _used_list *pNext;
	u_char name[ ASCCI_MAX ];
	char   flag;
	u_char r , g , b , a; // 色
} Used_List;

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
	SPR_OBJ             *font[ 15 ][ ASCCI_MAX ];
	SPR_OBJ             *clear_code[ 7 ][ 4 ];
	Used_List           *used_list;
	u_long64              *used_list_top_add;
	GV_PAD              *pad; 
	int                 action;
	int                 base_u;
	int                 base_v;
	int                 base_code_u;
	int                 base_code_v;
	int                 pad_status;
	int                 pad_check;
	int                 proc_prev;
	int                 proc_next;
	u_long64            version;
	int                 name;
	int                 dogtag_raute; 
	int                 timer;
	int                 stream_handler;
	int                 retry_count;
	u_long64              flag;
	void                *dumy_ptr;
	void  ( *act )( struct _work * );
} Work ;

static char SK_WakeFlag;
static int  SK_BossStreme;
static u_int BossTime[ 6 ];
static u_int BossDie[ 6 ];

// プロトタイプ
static int AnimationAct( Work * ); // アニメーション
static void EscapeAct( Work * );
static void ClearCodeDisp( Work *pWork );

// extern 
extern void SK_Printf( int ascci , SPR_OBJ *pObj , int u , int v , int , int );
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfNormal3( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern float SK_PrintfNormalWidth( u_char , SPR_OBJ * );
extern void SK_AllHide( SPR_OBJ ** , int );
extern void SK_AllShow( SPR_OBJ ** , int );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern float SK_PrintfNormalWidth3( u_char *ascci , int max_position ); // 一括 space通常
extern void *GetLocalResource( int ref_id, int offset );
extern void CalcBitTo26Code(unsigned int *in,unsigned char *out);
extern void EncodeClearCode2(unsigned int *code);

#define STR_BLK_MASK_TOP (10752570)

static int SprInit( Work *pWork ) // Sprite初期化
{
	static int table_x[] = { 36 , 126 , 216 , 306 , 396 , 36 , 126 };
	static int table_y[] = { 257 , 257 , 257 , 257 , 257 , 277 , 277 };
	SPR_OBJ	   *spr;
	int        i,j;

	if ( pWork->flag & SK_INITIALIZE_OK ){
		return ( 0 );
	}
	// dumy 領域を解放
	if ( pWork->dumy_ptr != NULL ){
		GV_Free( pWork->dumy_ptr );
		pWork->dumy_ptr = NULL;
	}
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
	for ( i = 0 ; i < 15 ; i++ ){
		for ( j = 0 ; j < ASCCI_MAX ; j++ ){
			pWork->font[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->font[ i ][ j ]->sprite.pos.y = DEFAULT_POS_Y;
		}
		SK_PrintfNormal2( NULL , pWork->font[ i ] , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
	}
	// 数字の取得
	spr = L2D_GetObject( pWork->handle_2d, STR_CODE_FONT ) ;
	if ( spr == NULL ){
		SK_Err("node_font\0");
		return -1 ;
	}
	SPR_HIDE( spr );
	pWork->base_code_u = spr->sprite.head.tex.u;
	pWork->base_code_v = spr->sprite.head.tex.v;
	spr->sprite.dw = FONT_WIDTH;
	spr->sprite.dh = FONT_HEIGHT;
	spr->sprite.pos.x = DEFAULT_POS_X;
	spr->sprite.pos.y = DEFAULT_POS_Y;
	for ( i = 0 ; i < 7 ; i ++ ){
		for ( j = 0 ; j < 4 ; j++ ){
			pWork->clear_code[ i ][ j ] = SPR_DuplicateTree( spr );
			pWork->clear_code[ i ][ j ]->sprite.pos.x = table_x[ i ];
			pWork->clear_code[ i ][ j ]->sprite.pos.y = table_y[ i ];
		}
		SK_PrintfNormal3( NULL , pWork->clear_code[ i ] , pWork->base_code_u , pWork->base_code_v , STR_WIDTH , STR_HEIGHT , 4 );
	}
	// time color
	spr = L2D_GetObject( pWork->handle_2d , STR_RESULT_TOTAL );
	if ( spr == NULL ){
		SK_Err("clr_tag\0");
		return -1;
	}
	spr->sprite.col.r = SK_CODE_R;
	spr->sprite.col.g = SK_CODE_G;
	spr->sprite.col.b = SK_CODE_B;
	spr->sprite.col.a = SK_A;
	spr = L2D_GetObject( pWork->handle_2d , STR_COLON7_1 );
	if ( spr == NULL ){
		SK_Err("clr_tag7-1\0");
		return -1;
	}
	spr->sprite.col.r = SK_CODE_R;
	spr->sprite.col.g = SK_CODE_G;
	spr->sprite.col.b = SK_CODE_B;
	spr->sprite.col.a = SK_A;
	spr = L2D_GetObject( pWork->handle_2d , STR_COLON7_2 );
	if ( spr == NULL ){
		SK_Err("clr_tag7-2\0");
		return -1;
	}
	spr->sprite.col.r = SK_CODE_R;
	spr->sprite.col.g = SK_CODE_G;
	spr->sprite.col.b = SK_CODE_B;
	spr->sprite.col.a = SK_A;

	pWork->flag |= SK_INITIALIZE_OK;

	return( 0 );
}

static void SK_SpritePosition( SPR_OBJ **spr , float x , float y , int max ) // 場所の自動設定
{
	float ftmp;
	int i;

	ftmp = 0.0f;
	for ( i = 0 ; i < max ; i ++ ){
		spr[ i ]->sprite.pos.x = x + ftmp;
		spr[ i ]->sprite.pos.y = y;
		ftmp += spr[ i ]->sprite.dw;
	}
}

#define PLAY_TIME_MAX (10)
#define NUM_MAX (4)
#define BOSS_MAX (7)
static void PlayTimeDisp( Work *pWork , int mode ) // play time input パル対応済
{
	static int minit_max = ( 99 * 60 * 100 ) + ( 59 * 100 ) + ( 99 );
//	int hour;
	int minit;
	int sec;
	int msec;
	int tmp;
	int time;
	int i;
	char buf[ PLAY_TIME_MAX ];

	time = 0;
	// それぞれに分担
	switch ( mode ){
	case 0 : // オルガ
		time = BossTime[ 0 ];
		break;
	case 1 : // ファットマン
		time = BossTime[ 1 ];
		break;
	case 2 : // ハリアー
		time = BossTime[ 2 ];
		break;
	case 3 : // ヴァンプ
		time = BossTime[ 3 ];
		break;
	case 4 : // RAY
		time = BossTime[ 4 ];
		break;
	case 5 : // ソリダス
		time = BossTime[ 5 ];
		break;
	case 6 : // 合計
		{
			int minit_tmp;
			int sec_tmp;
			int msec_tmp;
			int time_tmp;

			minit_tmp = sec_tmp = msec_tmp = time_tmp = 0;
			for ( i = 0 ; i < 6 ; i ++ ){
				tmp = ( ( BossTime[ i ] * 100.0f ) / DIRECT_TICK( 60 ) );
				tmp = tmp >= minit_max - 1 ? minit_max : tmp;
				tmp = tmp < 0 ? minit_max : tmp;
				minit = tmp / 60 / 100;
				sec = ( tmp - ( minit * 60 * 100 ) ) / 100;
				msec = ( tmp - ( minit * 60 * 100 ) - ( sec * 100 ) );
				
				minit_tmp += minit;
				sec_tmp += sec;
				msec_tmp += msec;
			}
			time_tmp = msec_tmp / 100;
			msec_tmp %= 100;
			// sec
			sec_tmp += time_tmp;
			time_tmp = sec_tmp / 60;
			sec_tmp %= 60;
			// minit
			minit_tmp += time_tmp;
			// limit check
			if ( minit_tmp > 99 ){
				minit_tmp = 99;
				sec_tmp = 59;
				msec_tmp = 99;
			} else { // normal
				minit_tmp = ( minit_tmp > 99 ? 99 : minit_tmp );
			}
			// print
			if ( minit_tmp == 0 && sec_tmp == 0 && msec_tmp == 0 ){
				sprintf( buf , "-- -- --" );
			} else {
				sprintf( buf , "%02d %02d %02d" , minit_tmp , sec_tmp , msec_tmp );
			}
			SK_PrintfNormal( buf , pWork->font[ mode ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PLAY_TIME_MAX );
			SK_PrintfChengColor2( pWork->font[ mode ] , SK_CODE_R , SK_CODE_G , SK_CODE_B , 0 , PLAY_TIME_MAX );
		}
		break;
	default : 
		time = 0;
		break;
	}
	if ( mode != 6 ){
		time = ( ( time * 100.0f ) / DIRECT_TICK( 60 ) );
		tmp = time >= minit_max - 1 ? minit_max : time;
		tmp = tmp < 0 ? minit_max : tmp;

		minit = tmp / 60 / 100;
		sec = ( tmp - ( minit * 60 * 100 ) ) / 100;
		msec = ( tmp - ( minit * 60 * 100 ) - ( sec * 100 ) );
		if ( minit < 0 ){
			minit = 0;
		}
		if ( sec < 0 ){
			sec = 0;
		}
		if ( msec < 0 ){
			msec = 0;
		}
		if ( time == 0 ){
			sprintf( buf , "-- -- --" );
		} else {
			sprintf( buf , "%02d %02d %02d" , minit , sec , msec );
		}
		SK_PrintfNormal( buf , pWork->font[ mode ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PLAY_TIME_MAX );
		SK_PrintfChengColor2( pWork->font[ mode ] , SK_R , SK_G , SK_B , 0 , PLAY_TIME_MAX );
	}
}

#define STR_RESULT (10216234) /* result_left01 */
#define FONT_MAX (7)
static void AllAlphaControl( Work *pWork ) // all alpha parent alpha copy
{
	static float table_y[] = { 128.0f - SK_SUB_Y , 146.0f - SK_SUB_Y , 164.0f - SK_SUB_Y , 182.0f - SK_SUB_Y ,
							   200.0f - SK_SUB_Y , 218.0f - SK_SUB_Y , 236.0f - SK_SUB_Y };
	SPR_OBJ *spr;
	int 	alpha;
	int     i;

	if ( !( pWork->flag & SK_INITIALIZE_OK ) || ( pWork->flag & SK_END ) || !( pWork->flag & SK_FONT_STANDBY_OK ) ){
		return;
	}
	spr = L2D_GetObject( pWork->handle_2d , STR_RESULT );
	if ( spr == NULL ){
		return;
	}
	// 親のαを出す
	alpha = ( int )spr->sprite.col.a;

	// alpha を操作 ついでに位置も制御
	for ( i = 0 ; i < FONT_MAX ; i ++ ){
		SK_SpritePosition( pWork->font[ i ] , 240.0f , table_y[ i ] - FONT_HEIGHT , ASCCI_MAX ); // 場所の自動設定
		if ( i == 6 ){
			SK_PrintfChengColor2( pWork->font[ i ] , SK_CODE_R , SK_CODE_G , SK_CODE_B , alpha , PLAY_TIME_MAX );
		} else {
			SK_PrintfChengColor2( pWork->font[ i ] , SK_R , SK_G , SK_B , alpha , ASCCI_MAX );
		}
		SK_AllShow( pWork->font[ i ] , ASCCI_MAX );
	}
}

static void PadControlNormal( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status == L2D_STAT_BUSY ) ){
		return;
	}
	if ( ( pWork->pad->press & PAD_B ) || ( pWork->pad->press & PAD_A ) || ( pWork->pad->press & PAD_STA ) ){
		pWork->act = ( void * )AnimationAct;
		pWork->action = STR_CLOSERESULT;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->flag &= ~SK_NORMAL;

		SE_START();
	}
}

static void PadAct( Work *pWork )
{
	if ( pWork->flag & SK_NORMAL ){ // 通常
		PadControlNormal( pWork );
	}
}

#define END_TIME ( ( DIRECT_TICK( 60 ) * 4 ) )
static void EscapeAct( Work *pWork )
{
	int      tmp;
	int      status;

	tmp = 0;
	status = L2D_ActionStatus( pWork->handle_2d );
	
	// l2d ここが + 1
	if ( ( status != L2D_STAT_BUSY ) ){
		if ( ( GM_StreamStatus( pWork->stream_handler ) == GM_STREAM_STATE_END ) || ( SK_BossStreme == 0 ) ||
			 ( pWork->timer >= END_TIME ) ){
			if ( pWork->proc_next != 0 ){
				GM_ExecProc( pWork->proc_next , NULL );
			}
			GM_GameOverClear();
			GM_GameOverRestart( 1 );
			// Chanel 0 Visible
			DG_Chanl( 0 )->flag = 1;
			GV_DestroyActor( pWork );
		} else {
			pWork->timer++;
		}
	}
}

static void NormalAct( Work *pWork )
{
	PadAct( pWork );
}

#define WAIT_TIME (DIRECT_TICK(150)) 
static void SK_StreamControl( Work *pWork ) // Stream
{
	if ( ( pWork->flag & SK_STREAM_PLAY_OK ) || !( SK_WakeFlag & 0x1 ) ){
		return;
	}
	if ( ( GM_StreamStatus( pWork->stream_handler ) == GM_STREAM_STATE_PLAY ) ||
		 ( GM_StreamStatus( pWork->stream_handler ) == GM_STREAM_STATE_END ) || 
		( SK_BossStreme == 0 ) || ( pWork->timer >= WAIT_TIME ) ){
		pWork->timer = 0;
		pWork->flag |= SK_STREAM_PLAY_OK;
		pWork->act = ( void * )AnimationAct;
	} else {
		pWork->timer++;
	}
}

static void Act( Work *pWork )
{
 	pWork->act( pWork );
	AllAlphaControl( pWork );
}

static void StreamRetry( Work *pWork )
{
	if ( SK_BossStreme != 0 ){
		pWork->stream_handler = GM_VoxStream( SK_BossStreme , GM_STREAM_NO_PAUSE );
		if ( pWork->stream_handler < 0 ){
#ifdef DEBUG_MODE
			printf("Stream Retry---------- %d\n" , pWork->stream_handler );
#endif
			if ( pWork->retry_count >= DIRECT_TICK( 60 ) ){
#ifdef DEBUG_MODE
				printf("Stream Retry Time Over---------- %d\n" , pWork->stream_handler );
#endif
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
		pWork->act = ( void * )AnimationAct; // Stream 無視
	}
}

static int AnimationAct( Work *pWork ) // 通常実行
{
	int status;
	int i;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULT : // default
			pWork->action = STR_OPENRESULT;
			pWork->flag |= SK_ACTION_START;
			SprInit( pWork );
			GM_StreamStopAll();
			pWork->act = ( void * )StreamRetry;
			// Chanel 0 Invisible
			DG_Chanl( 0 )->flag = 0;
			break;
		case STR_OPENRESULT : // OPENRESULT
			pWork->act = (void *)NormalAct;
			pWork->flag |= SK_NORMAL;
			break;
		case STR_CLOSERESULT : // OPENRESULT
			pWork->act = (void *)EscapeAct;
			for ( i = 0 ; i < 15 ; i ++ ){
				SK_AllHide( pWork->font[ i ] , ASCCI_MAX );
			}
			pWork->flag |= SK_END;
			break;
		}
		pWork->flag &= ~SK_ACTION_OK;
		return ( 0 );
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			if ( !( SK_WakeFlag & 0x1 ) ){
				return ( 0 );
			}
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}

	return ( 0 );
}

static	void	Die( Work *pWork )
{
	int i;

	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
	if ( pWork->dumy_ptr != NULL ){
		GV_Free( pWork->dumy_ptr );
	}
	for ( i = 0 ; i < 6 ; i ++ ){
		BossTime[ i ] = 0;
		BossDie[ i ] = 0;
	}
	SK_BossStreme = 0;
	SK_WakeFlag = 0;
}

static long64 TimeLimitCheck( long64 time )
{
	time = time > 0x1ffff ? 0x1ffff : time;

	return ( time );
}

enum {
	SK_LEVEL_VERY_EASY_JPN = 0x1 ,
	SK_LEVEL_EASY_JPN      ,
	SK_LEVEL_HARD_JPN      ,
};

static void ClearCodeDisp( Work *pWork )
{
	SPR_OBJ *spr;
	u_long64 in[ 2 ];
	u_long64  olga , fat , har , vamp , ray , solidus;
	u_long64  olga_no_die , fat_no_die , har_no_die , vamp_no_die , ray_no_die , solidus_no_die;
	char  code[ CODE_MAX ];
	u_long64  dif , player;
	int   tmp;
	int   i;

	// 難度
#ifdef DEBUG_MODE
	printf( "%d\n" , GM_GameLevel );
#endif
	if ( GM_GameLevel == LEVEL_VERY_EASY_JPN ){
		dif = ( SK_LEVEL_VERY_EASY_JPN );
	} else if ( GM_GameLevel == LEVEL_EASY_JPN ){
		dif = ( SK_LEVEL_EASY_JPN );
	} else if ( GM_GameLevel == LEVEL_HARD_JPN ){
		dif = ( SK_LEVEL_HARD_JPN );
	} else {
		dif = 0;
	}
	// limit check
	olga = TimeLimitCheck( ( BossTime[ 0 ] ) );
	fat  = TimeLimitCheck( ( BossTime[ 1 ] ) );
	har  = TimeLimitCheck( ( BossTime[ 2 ] ) );
	vamp = TimeLimitCheck( ( BossTime[ 3 ] ) );
	ray  = TimeLimitCheck( ( BossTime[ 4 ] ) );
	solidus = TimeLimitCheck( ( BossTime[ 5 ] ) );
	olga_no_die = BossDie[ 0 ] & 0x1;
	fat_no_die  = BossDie[ 1 ] & 0x1;
	har_no_die  = BossDie[ 2 ] & 0x1;
	vamp_no_die = BossDie[ 3 ] & 0x1;
	ray_no_die  = BossDie[ 4 ] & 0x1;
	solidus_no_die = BossDie[ 5 ] & 0x1;
	in[ 0 ] = 0;
	in[ 1 ] = 0;
	tmp = ( vamp >> 13 );
	player = GM_CheckPlayerStatus( PLAYER_SNAKE ) != 0 ? 0 : 1;
	in[ 0 ] = ( ( ( vamp & 0x1fff ) << 51 ) | ( har << 34 ) | ( fat << 17 ) | olga );
	in[ 1 ] = ( ( player << 45 ) | ( dif << 43 ) | ( pWork->version << 41 ) |
			    ( solidus_no_die << 40 ) | ( vamp_no_die << 39 ) | ( fat_no_die << 38 ) |
				( solidus << 21 ) | ( ray << 4 ) | tmp );
#ifdef DEBUG_MODE
	printf("player %x\ndif %d\nversion %d\n" , player , dif , pWork->version );
	printf("code 1 = %lx\n" , in[ 1 ] );
#endif

#if 0	
	EncodeClearCode2( ( int * )in );
	CalcBitTo26Code( ( int * )in , code );
#else	// 新クリアコード 2002/08/07 M.Kobayashi
	{
#ifndef KP_WINDOWS
		int code_len = ClearCodeGenerate( code, CODE_MAX, 26,
										  in, 110, irnd() );
#else
		int code_len = ClearCodeGenerate( CLEARCODE_MODE_BOSSRUSH,
										  code, CODE_MAX, 26,
										  in, 110, irnd() );
#endif
		ASSERT( code_len <= CODE_MAX );
	}
#endif	

#ifdef DEBUG_MODE
	printf("dif %d\n" , dif );
#endif
	for ( i = 0 ; i < CODE_MAX ; i ++ ){
		code[ i ] = code[ i ] + 'A';
	}
	for ( i = 0 ; i < 7 ; i ++ ) {
		SK_PrintfNormal3( &code[ 4 * i ] , pWork->clear_code[ i ] , pWork->base_code_u , pWork->base_code_v , STR_WIDTH , STR_HEIGHT , 4 );
		SK_PrintfChengColor2( pWork->clear_code[ i ] , SK_CODE_R , SK_CODE_G , SK_CODE_B , 128 , 4 ); // 一行いっきに書き換え
		if ( solidus != 0 ){
			SK_AllShow( pWork->clear_code[ i ] , 4 );
		} else {
			spr = L2D_GetObject( pWork->handle_2d , SPR_CLEARCODEPARTS );
			if ( spr != NULL ){
				SPR_HIDE( spr );
			}
			SK_AllHide( pWork->clear_code[ i ] , 4 );
		}
	}
}

static void DifShow( Work *pWork ) // 難度表記
{
	SPR_OBJ *spr;
	int strcode;

	if ( GM_GameLevel == LEVEL_VERY_EASY_JPN ){
		strcode = ( STR_LEVEL_VERY_EASY_JPN );
	} else if ( GM_GameLevel == LEVEL_EASY_JPN ){
		strcode = ( STR_LEVEL_EASY_JPN );
	} else if ( GM_GameLevel == LEVEL_HARD_JPN ){
		strcode = ( STR_LEVEL_HARD_JPN );
	} else {
		strcode = 0;
	}
	spr = L2D_GetObject( pWork->handle_2d , strcode );
	if ( spr == NULL ){
		SK_Err("dif\0");
		return;
	}
	SPR_SHOW( spr );
}

#define SIG_SHOWPOINT (11505383)
static void Signal( void *work , int sign , int value )
{
	Work *pWork;
	SPR_OBJ *spr;
	int  strcode;
	int  tmp;

	pWork = ( Work * )work;

	switch ( sign ){
	case SIG_SHOWPOINT : // node frame action 実行
		for ( tmp = 0 ; tmp < BOSS_MAX ; tmp ++ ){
			PlayTimeDisp( pWork , tmp ); // play time input
		}
		ClearCodeDisp( pWork );
		// Snake or Raiden
		if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) != 0 ){
			strcode = STR_SNAKETAG;
		} else {
			strcode = STR_RAIDENTAG;
		}
		spr = L2D_GetObject( pWork->handle_2d , strcode );
		if ( spr == NULL ){
			SK_Err("name\0");
			return;
		}
		SPR_SHOW( spr );
		DifShow( pWork );
		pWork->flag |= SK_FONT_STANDBY_OK;
		break;
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	int		handle;
	int     i;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 0 , 0 , GV_PAUSE_STOP ) ; // 後に黒枠がくるため
	if ( handle < 0 ){
		SK_Err("clear_code\0");
		return -1 ;
	}
	work->handle_2d = handle ;

	// シグナルの登録
	// l2d
	L2D_SetSignalHandle( work->handle_2d , work , Signal );
	
	work->action = STR_DEFAULT;
	work->act = ( void * )AnimationAct;
	work->pad_status = 0;
	work->pad_check = 0;
	work->pad = &GV_PadDataDirect[ 0 ];
	work->timer = 0;
	work->retry_count = 0;
	// etc
	work->flag = SK_ACTION_START;

	// gcl 戻りはなし
	work->proc_prev = 0;
	work->proc_next = GCL_GetOptionValue( 'n' , 0 );
	work->version = GCL_GetOptionValue( 'v' , 0 );
	for ( i = 0 ; i < 6 ; i ++ ){
		BossTime[ i ] = 0;
		BossDie[ i ] = 0;
	}
	// BossTime取得
	i = 0;
	GCL_GetOption( 't' );  // 位置の取得
	while( ( GCL_NextStr() != NULL ) && ( i < 6 ) ){
		BossTime[ i ] = GCL_GetNextInt();	
		i++;
	}
	// BossDie取得
	i = 0;
	GCL_GetOption( 'd' );  // 位置の取得
	while( ( GCL_NextStr() != NULL ) && ( i < 6 ) ){
		BossDie[ i ] = GCL_GetNextInt();	
		i++;
	}
	// dumy の 確保
	work->dumy_ptr = GV_Malloc( SK_SPACE_GET );
	if ( work->dumy_ptr == NULL ){
		return ( -1 );
	}
	// スタートききません
//	GM_SetGameStatus( STATE_PAUSE_DISABLE );

	SK_BossStreme = GCL_GetOptionValue( 'c' , 0 );
	SK_WakeFlag = 0;

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewSK_BossResult( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), ACTOR_PRIO ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->name = name;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

void SK_BossResultWakeup( void )
{
	SK_WakeFlag = SK_WakeFlag ^ 0x1;
//	GM_StreamStopAll() ;
}

void *SK_BossResultWakeup_Sub( int mode )
{
	SK_BossResultWakeup();
	return ( void * )1 ;
}

void SK_BossResultChenge( void )
{
	SK_BossStreme = GCL_GetOptionValue( 'c' , 0 );
}

void SK_BossTimeAfterWrite( void ) // 後から上書き
{
	int tmp;

	// time
	tmp = GCL_GetOptionValue( 'a' , 0 );
	if ( tmp != 0 ){
		BossTime[ 0 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'b' , 0 );
	if ( tmp != 0 ){
		BossTime[ 1 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'c' , 0 );
	if ( tmp != 0 ){
		BossTime[ 2 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'd' , 0 );
	if ( tmp != 0 ){
		BossTime[ 3 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'e' , 0 );
	if ( tmp != 0 ){
		BossTime[ 4 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'f' , 0 );
	if ( tmp != 0 ){
		BossTime[ 5 ] = tmp;
	}
	// die
	tmp = GCL_GetOptionValue( 'g' , 0 );
	if ( tmp != 0 ){
		BossDie[ 0 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'h' , 0 );
	if ( tmp != 0 ){
		BossDie[ 1 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'i' , 0 );
	if ( tmp != 0 ){
		BossDie[ 2 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'j' , 0 );
	if ( tmp != 0 ){
		BossDie[ 3 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'k' , 0 );
	if ( tmp != 0 ){
		BossDie[ 4 ] = tmp;
	}
	tmp = GCL_GetOptionValue( 'l' , 0 );
	if ( tmp != 0 ){
		BossDie[ 5 ] = tmp;
	}
}
