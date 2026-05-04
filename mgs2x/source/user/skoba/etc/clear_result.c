//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   clear_result.c
   結果表示
   
   2001/09/07	S.Kobayashi
   $Id: clear_result.c,v 1.2 2002/12/05 18:42:01 takaki Exp $
*/

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

#define		LAYOUT		  (15895067)
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

// over write
#define SK_R (80)//(56) 
#define SK_G (90)//(66) 
#define SK_B (85)//(61) 
#define SK_A (128)
#define SK_LOW_A (52)

#define SK_SUB_Y (40)
// Japanease or Usa or Pal
#if 0
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL

#define SELECT PAD_OK
#define CLEAR_RESULT_DELETE PAD_Y
#define CANCEL PAD_CANCEL
#define START  PAD_STA

#else

#define SELECT PAD_OK
#define CLEAR_RESULT_DELETE PAD_Y
#define CANCEL PAD_CANCEL
#define START  PAD_STA

#endif

#define STR_OPENRESULT (14591532)
#define STR_CLOSERESULT (8263032)
#define STR_CODE_FONT (9133060)

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
#define	SK_END                      I64(0x0000200000000000)


#define ASCCI_MAX (32 + 1) // plus 1 = tarminater
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
	Used_List           *used_list;
	u_long64              *used_list_top_add;
	GV_PAD              *pad; 
	int                 action;
	int                 base_u;
	int                 base_v;
	int                 pad_status;
	int                 pad_check;
	int                 proc_prev;
	int                 proc_next;
	int                 name;
	int                 dogtag_raute; 
	u_long64              flag;
	void  ( *act )( struct _work * );
} Work ;

// プロトタイプ
static int AnimationAct( Work * ); // アニメーション
static void EscapeAct( Work * );

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

static int SprInit( Work *pWork ) // Sprite初期化
{
	SPR_OBJ		*spr;
	int          i,j;

	if ( pWork->flag & SK_INITIALIZE_OK ){
		return ( 0 );
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
			SPR_HIDE( pWork->font[ i ][ j ] );
		}
		SK_PrintfNormal2( NULL , pWork->font[ i ] , pWork->base_u , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
	}

	pWork->flag |= SK_INITIALIZE_OK;

	return( 0 );
}

static int UsedListInitialize( Work *pWork ) // 初期化
{
	pWork->used_list = GV_Malloc( sizeof( Used_List ) );
	if ( pWork->used_list == NULL ){
		return ( -1 );
	}
	pWork->used_list->pPrev = NULL;
	pWork->used_list->pNext = NULL;

	GV_ZeroMemory( pWork->used_list->name , ASCCI_MAX );
	pWork->used_list->flag = SK_UNUSED; // Unused
	pWork->used_list->r = pWork->used_list->g = pWork->used_list->b = pWork->used_list->a = 0;

	pWork->used_list_top_add = ( u_long64 * )pWork->used_list;
	return ( 0 );
}

#define STR_RESOURCE (16608425)

static void UsedListAdd( Work *pWork , int position ) // Sort なし
{
	char *code1;

	code1 = NULL;
	pWork->used_list = ( Used_List * )pWork->used_list_top_add;

	while( pWork->used_list->pNext != NULL ){
		pWork->used_list = pWork->used_list->pNext;
	}
	// 最後に足す
	pWork->used_list->pNext = GV_Malloc( sizeof( Used_List ) );
	if ( pWork->used_list->pNext == NULL ){
		return;
	}
	pWork->used_list->pNext->pPrev = pWork->used_list;
	pWork->used_list->pNext->pNext = NULL;
	
	pWork->used_list = pWork->used_list->pNext;

	GV_ZeroMemory( pWork->used_list->name , ASCCI_MAX );

	code1 = ( char * )GetLocalResource( STR_RESOURCE , position );
	if ( code1 == NULL ) {
		return;
	}
	SK_ToUpper( pWork->used_list->name , code1 );
	pWork->used_list->flag = SK_USED; // Unused
	pWork->used_list->r = pWork->used_list->g = pWork->used_list->b = pWork->used_list->a = 0;
}

static void UsedListAddMng( Work *pWork , int num ) // リストに追加
{
	char *ptr;

	ptr = NULL;
	switch ( num ){
	case SK_CLEAR_MUGENBANDANA_USED : 
		UsedListAdd( pWork , SK_CLEAR_MUGENBANDANA_USED ); // Sort なし
		break;
	case SK_CLEAR_MUGENWIG_USED : 
		UsedListAdd( pWork , SK_CLEAR_MUGENWIG_USED ); // Sort なし
		break;
	case SK_CLEAR_WIG_A_USED : 
		UsedListAdd( pWork , SK_CLEAR_WIG_A_USED ); // Sort なし
		break;
	case SK_CLEAR_WIG_B_USED : 
		UsedListAdd( pWork , SK_CLEAR_WIG_B_USED ); // Sort なし
		break;
	case SK_CLEAR_STEALTH_USED : 
		UsedListAdd( pWork , SK_CLEAR_STEALTH_USED ); // Sort なし
		break;
	}
}

static void UsedListAutoInsert( Work *pWork ) // リストに追加
{
	if ( GM_ClearCodeFlag & GM_CLEAR_MUGENBANDANA_USED ){
		UsedListAddMng( pWork , SK_CLEAR_MUGENBANDANA_USED ); // リストに追加
	}
	if ( GM_ClearCodeFlag & GM_CLEAR_STEALTH_USED ){
		UsedListAddMng( pWork , SK_CLEAR_STEALTH_USED ); // リストに追加
	}
	if ( GM_ClearCodeFlag & GM_CLEAR_MUGENWIG_USED ){
		UsedListAddMng( pWork , SK_CLEAR_MUGENWIG_USED ); // リストに追加
	}
	if ( GM_ClearCodeFlag & GM_CLEAR_WIG_B_USED ){
		UsedListAddMng( pWork , SK_CLEAR_WIG_B_USED ); // リストに追加
	}
	if ( GM_ClearCodeFlag & GM_CLEAR_WIG_A_USED ){
		UsedListAddMng( pWork , SK_CLEAR_WIG_A_USED ); // リストに追加
	}
}

static void UsedListFree( Work *pWork ) // 解放
{
	Used_List *ptr;

	pWork->used_list = ( Used_List * )pWork->used_list_top_add;
	while ( pWork->used_list != NULL ) {
		ptr = pWork->used_list->pNext;
		GV_DelayedFree( pWork->used_list );
		pWork->used_list = ptr;
	}
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

static void UsedListDisp( Work *pWork ) // 使用済の表示
{
	Used_List *used_list;
	static float table_y[] = { 236.0f - SK_SUB_Y , 254.0f - SK_SUB_Y , 272.0f - SK_SUB_Y , 290.0f - SK_SUB_Y , 308.0f - SK_SUB_Y };
	int tmp;

	tmp = 0;
	used_list = ( Used_List * )pWork->used_list_top_add;
	used_list = used_list->pNext;
	while( used_list != NULL ){
		if ( used_list->flag == SK_USED ){
			SK_SpritePosition( pWork->font[ 6 + tmp ] , 240.0f , table_y[ tmp ] - FONT_HEIGHT , ASCCI_MAX ); // 場所の自動設定
			SK_PrintfNormal2( used_list->name , pWork->font[ 6 + tmp ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , ASCCI_MAX );
			SK_PrintfChengColor2( pWork->font[ 6 + tmp ] , SK_R , SK_G , SK_B , 128 , ASCCI_MAX );
			SK_AllShow( pWork->font[ 6 + tmp ] , ASCCI_MAX );
			tmp++;
		}
		used_list = used_list->pNext;
	}
}

#define STR_URL (9033814) /* alldogtag */
static void UrlDisp( Work *pWork ) // Dogtags Tanker & Plant Complete --> Mgs2 Dogtag HomePage Disp 
{
	SPR_OBJ *spr;

	if ( pWork->flag & SK_DOGTAG_ALLGET ){ // 取得済
		spr = L2D_GetObject( pWork->handle_2d , STR_URL );
		if ( spr == NULL ){
			return;
		}
		SPR_SHOW( spr );
	}
}

#define PLAY_TIME_MAX (10)
#define NUM_MAX (4)
static void PlayTimeDisp( Work *pWork ) // play time input パル対応済
{
	static int minit_max;
	int hour;
	int minit;
	int sec;
	int tmp;
	char buf[ PLAY_TIME_MAX ];

   minit_max = ( 99 * 60 * 60 * DIRECT_TICK( 60 ) ) + ( 59 * 60 * DIRECT_TICK( 60 ) ) + ( 59 * DIRECT_TICK( 60 ) );	

   tmp = GM_PlayTime < minit_max + 1 ? GM_PlayTime : minit_max;
	tmp = tmp < 0 ? minit_max : tmp;

	
	hour = tmp / 60 / 60 / DIRECT_TICK( 60 );
	minit = ( tmp - ( hour * 60 * 60 * DIRECT_TICK( 60 ) ) ) / 60 / DIRECT_TICK( 60 );
	sec = ( tmp - ( ( hour * 60 * 60 * DIRECT_TICK( 60 ) ) + ( minit * 60 * DIRECT_TICK( 60 ) ) ) ) / DIRECT_TICK( 60 );
	sprintf( buf , "%02d %02d %02d" , hour , minit , sec );
   SK_PrintfNormal( buf , pWork->font[ 0 ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PLAY_TIME_MAX );
	SK_PrintfChengColor2( pWork->font[ 0 ] , SK_R , SK_G , SK_B , 0 , PLAY_TIME_MAX );
}

static void NumCodinate( char *buf ) // 文字整形
{
	char flag;
	int  i;

	flag = 0;
	for ( i = 0 ; i < NUM_MAX ; i ++ ){ // 文字整形
		if ( ( flag == 0 ) && ( buf[ i ] != '0' ) ){
			flag = 1;
		}
		if ( ( flag == 0 ) && ( buf[ i ] == '0' ) && ( i != ( NUM_MAX - 1 ) ) ){
			buf[ i ] = 0x20; // space
			flag = 1;
		}
	}
}

static void NumDisp( Work *pWork , int number , int data ) // save times input
{
	char buf[ NUM_MAX + 1 ];/* 4文字+'\0' by yano 2002.07.11 */
	int  tmp;

	tmp = data < 10000 ? data : 9999;
	tmp = tmp < 0 ? 9999 : tmp;
	sprintf( buf , "%4d" , tmp );

	NumCodinate( buf );

	SK_PrintfNormal2( buf , pWork->font[ number ] , pWork->base_u  , pWork->base_v , STR_WIDTH , STR_HEIGHT , PLAY_TIME_MAX );
	SK_PrintfChengColor2( pWork->font[ number ] , SK_R , SK_G , SK_B , 0 , NUM_MAX );
}

static void NumSetupAllinOne( Work *pWork ) // 4桁数字のSetUp
{
	NumDisp( pWork , SK_SAVE        , GM_SaveCount );
	NumDisp( pWork , SK_CONTINUE    , GM_ContinueCount );
	NumDisp( pWork , SK_ALERTCOUNT  , GM_AlertCount ); // 危険モード
	NumDisp( pWork , SK_ENEMIES     , GM_KillCount );
	NumDisp( pWork , SK_RATIONS     , GM_RationUseCount );
}

#define STR_RESULT_ITEM (4006608)
static void UsedItemList( Work *pWork ) // 最上位UsedItem追加関数
{
	SPR_OBJ *spr;

	if ( GM_ClearCodeFlag & GM_CLEAR_SPECIAL_ITEM_USED ) { // 特殊アイテム使ってクリア
		UsedListAutoInsert( pWork );
	} else {
		spr = L2D_GetObject( pWork->handle_2d , STR_RESULT_ITEM );
		if ( spr == NULL ){
			return;
		}
		SPR_HIDE( spr );
	}
}

#define STR_RESULT (10216234) /* result_left01 */
#define FONT_MAX (6)
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
		SK_SpritePosition( pWork->font[ i ] , 240.0f , (table_y[ i ]-4) - FONT_HEIGHT , ASCCI_MAX ); // 場所の自動設定
		SK_PrintfChengColor2( pWork->font[ i ] , SK_R , SK_G , SK_B , alpha , ASCCI_MAX );
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

static void EscapeAct( Work *pWork )
{
	int      tmp;
	int      status;

	tmp = 0;
	status = L2D_ActionStatus( pWork->handle_2d );
	
	// l2d ここが + 1
	if ( ( status != L2D_STAT_BUSY ) ){
		if ( pWork->proc_next != 0 ){
			GM_ExecProc( pWork->proc_next , NULL );
		}
		GV_DestroyActor( pWork );
	}
}

static void NormalAct( Work *pWork )
{
	PadAct( pWork );
}

static void Act( Work *pWork )
{
   //BP JG - align the colons in the time.
   SPR_OBJ *pTestObject;
   pTestObject = L2D_GetObject(pWork->handle_2d,2607807);
   if ( pTestObject ) pTestObject->sprite.pos.x = 313;
   if ( pTestObject ) pTestObject->sprite.pos.y = 112;
   pTestObject = L2D_GetObject(pWork->handle_2d,2607806);
   if ( pTestObject ) pTestObject->sprite.pos.x = 270;
   if ( pTestObject ) pTestObject->sprite.pos.y = 112;

   // move and resize the RESULTS text.
   pTestObject = L2D_GetObject(pWork->handle_2d,0x9be32a);
   if ( pTestObject ) pTestObject->sprite.pos.y = 81;
   if ( pTestObject ) pTestObject->sprite.dw = 148;
   if ( pTestObject ) pTestObject->sprite.dh = 12;

   // BP JG - correct the lines on the screen, changes to these in the layout editor had no effect.
   /*
   pTestObject = L2D_GetObject(pWork->handle_2d,0xc89a71);
   if ( pTestObject ) pTestObject->line.pos[1].y = 75;
   if ( pTestObject ) pTestObject->line.pos[1].y = 100;
   pTestObject = L2D_GetObject(pWork->handle_2d,0xc89a70);
   if ( pTestObject ) pTestObject->line.pos[0].x = 27;
   if ( pTestObject ) pTestObject->line.pos[1].x = 485;
   pTestObject = L2D_GetObject(pWork->handle_2d,0x33d378);
   if ( pTestObject ) pTestObject->line.pos[0].y = 100;
   if ( pTestObject ) pTestObject->line.pos[1].y = 342;
   pTestObject = L2D_GetObject(pWork->handle_2d,0x69c8f3);
   if ( pTestObject ) pTestObject->line.pos[0].x = 27;
   if ( pTestObject ) pTestObject->line.pos[1].x = 485;
   pTestObject = L2D_GetObject(pWork->handle_2d,0x69ccf3);
   if ( pTestObject ) pTestObject->line.pos[0].y = 342;
   if ( pTestObject ) pTestObject->line.pos[1].y = 383;
   */

   pWork->act( pWork );
	AllAlphaControl( pWork );
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
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			switch ( pWork->action ){
			case STR_OPENRESULT :
				PlayTimeDisp( pWork ); // play time input
				NumSetupAllinOne( pWork );
				UsedItemList( pWork );
				UsedListDisp( pWork );
				UrlDisp( pWork ); // Dogtags Tanker & Plant Complete --> Mgs2 Dogtag HomePage Disp 
				pWork->flag |= SK_FONT_STANDBY_OK;
				break;
			}
			pWork->flag |= SK_ACTION_OK;
			pWork->flag &= ~SK_ACTION_START;
		}
	}

	return ( 0 );
}

static	void	Die( Work *pWork )
{
	UsedListFree( pWork ); // 解放

	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
}

/*----------------------------------------------------------------*/
static	int	GetResources( Work *work )
{
	int		handle;
	int     layout ;

	work->handle_2d = -1 ;

	layout = GCL_GetOptionValue( 'l' , LAYOUT ) ;
	handle = L2D_LoadLayout( layout, DG_CHANL_MENU , 0 , 0 ) ; // 後に黒枠がくるため
	if ( handle < 0 ){
		SK_Err("clear_code\0");
		return -1 ;
	}
	work->handle_2d = handle ;

	work->action = STR_DEFAULT;
	work->act = ( void * )AnimationAct;
	work->pad_status = 0;
	work->pad_check = 0;
	work->pad = &GV_PadDataDirect[ 0 ];
	// etc
	work->flag = SK_ACTION_START;
	// gcl 戻りはなし
	work->proc_prev = 0;
	work->proc_next = GCL_GetOptionValue( 'n' , 0 );
	work->dogtag_raute = GCL_GetOptionValue( 'd' , 0 );
#ifdef DEBUG_MODE
	printf("dogtag_raute = %d\n" , work->dogtag_raute );
#endif
	if ( work->dogtag_raute >= 100 ){ // 保険で > にしています
		work->flag |= SK_DOGTAG_ALLGET;
	}

	UsedListInitialize( work ); // 初期化

	// スタートききません
	GM_SetGameStatus( STATE_PAUSE_DISABLE );

	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewSK_Result( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
//	work = GV_NewActorPrio( GV_ACTOR_MANAGER , sizeof( Work ), 30 ) ;
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
