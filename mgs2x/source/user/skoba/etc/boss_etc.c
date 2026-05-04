//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   boss_result.c
   boss rush結果表示
   
   2001/09/18	S.Kobayashi
   $Id: boss_etc.c,v 1.1.1.3 2002/11/19 11:50:18 Yoshizawa1 Exp $
*/

/* メモリーカードがやばいかも */

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

#define		LAYOUT		  (2552802) // boss_telop.l2d 
#define 	STR_NODE_FONT (9853956)
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
#define SK_R (56) 
#define SK_G (66) 
#define SK_B (61) 
#define SK_A (128)
#define SK_LOW_A (52)

#define SK_SUB_Y (22)

#define STR_OPENRESULT (2313120) // openBossResult
#define STR_CLOSERESULT (14500500) // closeBossResult
#define STR_CODE_FONT (9133060)
#define KEY_MAX (4)

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
#define	SK_FADE_IN                  I64(0x0000040000000000)
#define	SK_FONT_COLOR_SET           I64(0x0000100000000000)
#define	SK_END                      I64(0x0000200000000000)


#define STR_PLAY_NAME        (12040484)
#define STR_PLAY_FACE        (11778020)
#define STR_BOSS_FACE        (1907431)
#define STR_BOSS_NAME        (2169895)
#define STR_KEY_SNAKE        (6550633)
#define STR_KEY_RAIDEN       (6549201)
#define STR_KEY_BOSS_OLGA    (6546479)
#define STR_KEY_BOSS_FATMAN  (6536924)
#define STR_KEY_BOSS_HARRIER (6538970)
#define STR_KEY_BOSS_VAMP    (6553301)
#define STR_KEY_BOSS_RAY     (6549217)
#define STR_KEY_BOSS_SOLIDUS (6550676)
#define STR_KEY_NAME_SNAKE   (259690)
#define STR_KEY_NAME_RAIDEN  (258258)
#define STR_KEY_BOSS_NAME_OLGA    (255536)
#define STR_KEY_BOSS_NAME_FATMAN  (245981)
#define STR_KEY_BOSS_NAME_HARRIER (248027)
#define STR_KEY_BOSS_NAME_VAMP    (262358)
#define STR_KEY_BOSS_NAME_RAY     (258274)
#define STR_KEY_BOSS_NAME_SOLIDUS (259733)

typedef struct {
	float   p;
	int     count;
	int     code1;
	int     code2;
	int     morf_parts;
	u_long64  flag;
} Hokan;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	GV_PAD              *pad; 
	int                 action;
	int                 base_u;
	int                 base_v;
	int                 pad_status;
	int                 pad_check;
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	u_long64              flag;

	Hokan               boss_pic[ 4 ];
	int                 boss_name;
	int                 disp_mode;
	int                 loop_count;
	int                 player;               
	void  ( *act )( struct _work * );
} Work ;

static char SK_WakeFlag;

// プロトタイプ
static int AnimationAct( Work * ); // アニメーション
static inline int MorfAct( Work *pWork , Hokan *pHokan ); // これのみ実行
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan );
static void KeySetPic( Work *pWork ); // キーを設定する関数
static void KeySetName( Work *pWork ); // キーを設定する関数

// extern 
extern void SK_Printf( int ascci , SPR_OBJ *pObj , int u , int v , int , int );
extern void SK_MenuPrintf( char *str , int x , int y , CVECTOR *color , char mode );
extern void SK_PrintfNormal( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern float SK_PrintfNormalWidth( u_char , SPR_OBJ * );
extern void SK_AllHide( SPR_OBJ ** , int );
extern void SK_AllShow( SPR_OBJ ** , int );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern float SK_PrintfNormalWidth3( u_char *ascci , int max_position ); // 一括 space通常
extern void *GetLocalResource( int ref_id, int offset );

#define STR_BLK_MASK_TOP (10752570)

#define STR_VS_OLGA    (2710445)
#define STR_VS_FATMAN  (12939552)
#define STR_VS_HARRIER (9487338)
#define STR_VS_VAMP    (2928764)
#define STR_VS_RAY     (6378892)
#define STR_VS_SOLIDUS (2897095)

static void ShowHideControl( int handle_2d , int strcode  , int mode )
{
	SPR_OBJ		*spr;

	spr = L2D_GetObject( handle_2d , strcode );
	if ( spr == NULL ){
		return;
	}
	switch ( mode ){
	case 0 : // hide
		SPR_HIDE( spr );
		break;
	case 1 : // show
		SPR_SHOW( spr );
		break;
	}
}

static void BossNameDisp( Work *pWork , int flag )
{
	switch ( pWork->boss_name ){
	case 0 : // オルガ
		ShowHideControl( pWork->handle_2d , STR_VS_OLGA , flag );
		break;
	case 1 : // ファットマン
		ShowHideControl( pWork->handle_2d , STR_VS_FATMAN , flag );
		break;
	case 2 : // ハリアー
		ShowHideControl( pWork->handle_2d , STR_VS_HARRIER , flag );
		break;
	case 3 : // ヴァンプ
		ShowHideControl( pWork->handle_2d , STR_VS_VAMP , flag );
		break;
	case 4 : // RAY
		ShowHideControl( pWork->handle_2d , STR_VS_RAY , flag );
		break;
	case 5 : // ソリダス
		ShowHideControl( pWork->handle_2d , STR_VS_SOLIDUS , flag );
		break;
	default : 
		break;
	}
}

static int SprInit( Work *pWork ) // Sprite初期化
{
	if ( pWork->flag & SK_INITIALIZE_OK ){
		return ( 0 );
	}
	BossNameDisp( pWork , 1 );
	pWork->flag |= SK_INITIALIZE_OK;

	return( 0 );
}

#if 0
static void AlphaControl( Work *pWork )
{
	SPR_OBJ *spr;
	int     r , g , b;
	int     alpha;

	if ( pWork->flag & SK_FADE_IN ){
		spr = L2D_GetObject( pWork->handle_2d , STR_BOSS_NAME );
		if ( spr == NULL ){
			return;
		}
		alpha = ( int )spr->sprite.col.a;
		if ( alpha < 128 ){
			alpha += IN_FADE_SPEED;
		} else {
			alpha = 128;
			pWork->flag &= ~SK_FADE_IN;
		}
		spr->sprite.col.a = ( char )alpha;
	}
	if ( pWork->flag & SK_FONT_COLOR_SET ){
		spr = L2D_GetObject( pWork->handle_2d , STR_BOSS_NAME );
		if ( spr == NULL ){
			return;
		}
		r = ( int )spr->sprite.col.r;
		if ( r < 42 ){
			r -= OUT_FADE_SPEED;
		} else {
			r = 42;
		}
		spr->sprite.col.r = ( char )r;

		g = ( int )spr->sprite.col.g;
		if ( g < 42 ){
			g -= OUT_FADE_SPEED;
		} else {
			g = 42;
		}
		spr->sprite.col.g = ( char )g;

		b = ( int )spr->sprite.col.b;
		if ( b < 42 ){
			b -= OUT_FADE_SPEED;
		} else {
			b = 42;
		}
		spr->sprite.col.b = ( char )b;
		if ( r == 42 && g == 42 && b == 42 ){
			pWork->flag &= ~SK_FONT_COLOR_SET;
		}
	}
}
#endif

static void Act( Work *pWork )
{
	int i;

 	pWork->act( pWork );
	if ( pWork->disp_mode == 0 ){
		// key
		for ( i = 0 ; i < KEY_MAX ; i ++ ){
			MorfAct( pWork , &pWork->boss_pic[ i ] );
		}
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

	if ( strcode == 0 ){
		printf("return\n");
		return;
	}
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

#define STR_SHOWREADY (13266348)
#define STR_LOOPFIGHT (376345)
#define STR_HIDEFIGHT (101187)
#define STR_SHOWBOSSNAME (11663840)
#define STR_SHOWWIN (2950949)
#define STR_LOOPWIN (2721139)
#define STR_HIDEWINNORM (9439987)
#define STR_HIDEWINFINISH (13225207)
static void DefaultNextAction( Work *pWork )
{
	switch ( pWork->disp_mode ){
	case 0 :
		DG_Chanl( 0 )->flag = 0;
		pWork->action = STR_SHOWBOSSNAME;
		KeySetPic( pWork ); // キーを設定する関数
		KeySetName( pWork ); // キーを設定する関数
		break;
	case 1 :
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_B_FIGHT1 );
		pWork->action = STR_LOOPFIGHT;
		break;
	case 2 :
	case 3 :
		pWork->action = STR_SHOWWIN;
		break;
	}		
}

static void KeySetPic( Work *pWork ) // キーを設定する関数
{
	pWork->boss_pic[ 0 ].morf_parts = STR_PLAY_FACE;
	pWork->boss_pic[ 2 ].morf_parts = STR_BOSS_FACE;
	// Snake or Raiden
	if ( pWork->player == 0 ){
		pWork->boss_pic[ 0 ].code1 = STR_KEY_SNAKE;
		pWork->boss_pic[ 0 ].code2 = STR_KEY_SNAKE;
	} else {
		pWork->boss_pic[ 0 ].code1 = STR_KEY_RAIDEN;
		pWork->boss_pic[ 0 ].code2 = STR_KEY_RAIDEN;
	}
	// boss
	switch ( pWork->boss_name ){
	case 0 : // オルガ
		pWork->boss_pic[ 2 ].code1 = STR_KEY_BOSS_OLGA;
		pWork->boss_pic[ 2 ].code2 = STR_KEY_BOSS_OLGA;
		break;
	case 1 : // ファットマン
		pWork->boss_pic[ 2 ].code1 = STR_KEY_BOSS_FATMAN;
		pWork->boss_pic[ 2 ].code2 = STR_KEY_BOSS_FATMAN;
		break;
	case 2 : // ハリアー
		pWork->boss_pic[ 2 ].code1 = STR_KEY_BOSS_HARRIER;
		pWork->boss_pic[ 2 ].code2 = STR_KEY_BOSS_HARRIER;
		break;
	case 3 : // ヴァンプ
		pWork->boss_pic[ 2 ].code1 = STR_KEY_BOSS_VAMP;
		pWork->boss_pic[ 2 ].code2 = STR_KEY_BOSS_VAMP;
		break;
	case 4 : // RAY
		pWork->boss_pic[ 2 ].code1 = STR_KEY_BOSS_RAY;
		pWork->boss_pic[ 2 ].code2 = STR_KEY_BOSS_RAY;
		break;
	case 5 : // ソリダス
		pWork->boss_pic[ 2 ].code1 = STR_KEY_BOSS_SOLIDUS;
		pWork->boss_pic[ 2 ].code2 = STR_KEY_BOSS_SOLIDUS;
		break;
	}
	pWork->boss_pic[ 0 ].flag = 0;
	pWork->boss_pic[ 0 ].p = 0.0f;
	pWork->boss_pic[ 0 ].count = 1;
	pWork->boss_pic[ 2 ].flag = 0;
	pWork->boss_pic[ 2 ].p = 0.0f;
	pWork->boss_pic[ 2 ].count = 1;
}

static void KeySetName( Work *pWork ) // キーを設定する関数
{
	pWork->boss_pic[ 1 ].morf_parts = STR_PLAY_NAME;
	pWork->boss_pic[ 3 ].morf_parts = STR_BOSS_NAME;
	// Snake or Raiden
	if ( pWork->player == 0 ){
		pWork->boss_pic[ 1 ].code1 = STR_KEY_NAME_SNAKE;
		pWork->boss_pic[ 1 ].code2 = STR_KEY_NAME_SNAKE;
	} else {
		pWork->boss_pic[ 1 ].code1 = STR_KEY_NAME_RAIDEN;
		pWork->boss_pic[ 1 ].code2 = STR_KEY_NAME_RAIDEN;
	}
	// boss
	switch ( pWork->boss_name ){
	case 0 : // オルガ
		pWork->boss_pic[ 3 ].code1 = STR_KEY_BOSS_NAME_OLGA;
		pWork->boss_pic[ 3 ].code2 = STR_KEY_BOSS_NAME_OLGA;
		break;
	case 1 : // ファットマン
		pWork->boss_pic[ 3 ].code1 = STR_KEY_BOSS_NAME_FATMAN;
		pWork->boss_pic[ 3 ].code2 = STR_KEY_BOSS_NAME_FATMAN;
		break;
	case 2 : // ハリアー
		pWork->boss_pic[ 3 ].code1 = STR_KEY_BOSS_NAME_HARRIER;
		pWork->boss_pic[ 3 ].code2 = STR_KEY_BOSS_NAME_HARRIER;
		break;
	case 3 : // ヴァンプ
		pWork->boss_pic[ 3 ].code1 = STR_KEY_BOSS_NAME_VAMP;
		pWork->boss_pic[ 3 ].code2 = STR_KEY_BOSS_NAME_VAMP;
		break;
	case 4 : // RAY
		pWork->boss_pic[ 3 ].code1 = STR_KEY_BOSS_NAME_RAY;
		pWork->boss_pic[ 3 ].code2 = STR_KEY_BOSS_NAME_RAY;
		break;
	case 5 : // ソリダス
		pWork->boss_pic[ 3 ].code1 = STR_KEY_BOSS_NAME_SOLIDUS;
		pWork->boss_pic[ 3 ].code2 = STR_KEY_BOSS_NAME_SOLIDUS;
		break;
	}
	pWork->boss_pic[ 1 ].flag = 0;
	pWork->boss_pic[ 1 ].p = 0.0f;
	pWork->boss_pic[ 1 ].count = 1;
	pWork->boss_pic[ 3 ].flag = 0;
	pWork->boss_pic[ 3 ].p = 0.0f;
	pWork->boss_pic[ 3 ].count = 1;
}

static int AnimationAct( Work *pWork ) // 通常実行
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULT : // default
			pWork->flag |= SK_ACTION_START;
			DefaultNextAction( pWork );
			break;
		case STR_SHOWBOSSNAME : // default
			BossNameDisp( pWork , 0 );
			pWork->action = STR_SHOWREADY;
			pWork->flag |= SK_ACTION_START;
			break;
		case STR_SHOWWIN : // default
			pWork->action = STR_LOOPWIN;
			pWork->flag |= SK_ACTION_START;
			break;
		case STR_LOOPFIGHT : // default
			if ( pWork->loop_count < 1 ){
				pWork->action = STR_LOOPFIGHT;
				pWork->flag |= SK_ACTION_START;
				pWork->loop_count++;
			} else {
				pWork->action = STR_HIDEFIGHT;
				pWork->flag |= SK_ACTION_START;
			}				
			break;
		case STR_LOOPWIN : // default
			if ( pWork->loop_count < 1 ){
				pWork->action = STR_LOOPWIN;
				pWork->flag |= SK_ACTION_START;
				pWork->loop_count++;
			} else {
				if ( pWork->disp_mode == 2 ){ // normal
					pWork->action = STR_HIDEWINNORM;
					pWork->flag |= SK_ACTION_START;
				} else { // finish
					pWork->action = STR_HIDEWINFINISH;
					pWork->flag |= SK_ACTION_START;
				}
			}				
			break;
		case STR_HIDEFIGHT :
		case STR_HIDEWINNORM :
		case STR_HIDEWINFINISH :
			if ( pWork->proc_next != 0 ){
				GM_ExecProc( pWork->proc_next , NULL );
			} else {
				printf("proc none\n");
			}
			GV_DestroyActor( pWork );
			break;
		case STR_SHOWREADY :
			if ( pWork->proc_next != 0 ){
//				GV_PauseOffActorSystem( GV_PAUSE_PAUSE ) ;
				GM_ExecProc( pWork->proc_next , NULL );
			} else {
				printf("proc none\n");
			}
			GV_DestroyActor( pWork );
			break;
		}
		pWork->flag &= ~SK_ACTION_OK;
		return ( 0 );
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}

	return ( 0 );
}

#define SIG_SHOWPOINT (11505383)
#define SIG_SD_S_N_START1 (2666147)
#define SIG_SD_S_B_READY1 (3675968)
#define SIG_SD_S_B_FIGHT1 (8070792)
#define SIG_SD_S_TWINKY01 (13497358)
#define SIG_SD_S_TWINKY02 (13497359)
#define SIG_SD_S_LINEMOV1 (4099416)
static void Signal( void *work , int sign , int value )
{
	Work *pWork;

	pWork = ( Work * )work;

	switch ( sign ){
	case SIG_SHOWPOINT : // node frame action 実行
		SprInit( pWork );
		break;
	case SIG_SD_S_N_START1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_N_START1 );
		break;
	case SIG_SD_S_B_READY1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_B_READY1 );
		break;
	case SIG_SD_S_B_FIGHT1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_B_FIGHT1 );
		break;
	case SIG_SD_S_TWINKY01 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_TWINKY01 );
		break;
	case SIG_SD_S_TWINKY02 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_TWINKY02 );
		break;
	case SIG_SD_S_LINEMOV1 : // node frame action 実行
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_LINEMOV1 );
		break;
	}
}

static	void	Die( Work *pWork )
{
	DG_Chanl( 0 )->flag = 1;
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
}

/*----------------------------------------------------------------*/
// mode = 0 : ready , 1 : loop fight
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

	// etc
	work->action = STR_DEFAULT;
	work->act = ( void * )AnimationAct;
	work->pad_status = 0;
	work->pad_check = 0;
	work->pad = &GV_PadDataDirect[ 0 ];
	work->loop_count = 0;
#if 1
	work->disp_mode = GCL_GetOptionValue( 'm' , 0 );
	work->boss_name = GCL_GetOptionValue( 'b' , 0 );
	work->proc_next = GCL_GetOptionValue( 'n' , 0 );
	work->player    = GCL_GetOptionValue( 'p' , 0 );
#else
	work->disp_mode = 0;
	work->boss_name = 4;
	work->proc_next = 0;
	work->player    = 0;
#endif
	// etc
	for ( i = 0 ; i < KEY_MAX ; i ++ ){
		work->boss_pic[ i ].flag = 0;	
		work->boss_pic[ i ].p = 0.0f;	
		work->boss_pic[ i ].code1 = 0;	
		work->boss_pic[ i ].code2 = 0;
	}
	work->flag = SK_ACTION_START;

	SK_WakeFlag = 0;

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewSK_BossTelop( int name )
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
