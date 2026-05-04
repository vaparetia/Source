//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  全体マップ表示の 3D 表示
  map_3d.c

  2001/04/16  Y.Kira
  $Id: map_3d.c,v 1.1.1.3 2002/11/19 11:43:55 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "g_struct.h"

#include "font.h"
#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "utl_dma.h"
#include "vu0_pack.h"
#include "fpu_pack.h"
#include "fmt_demo.h"
#include "dmapack.h"
#include "def_dma.h"
#include "sprite_2d.h"
#include "debugmenu.h"
#include "../../skoba/test/etc.h"
#include "../../mode/codec/codecmem.h"
#include "../../yamashita/2D/msn.h"

#include "mode/menu/xtextscn.h"

#define _map_3d_c_
#include "map_3d.h"

#include "BP_EndianSupport.h"
#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "BP_Camera.h"

#ifdef DEBUG
#define DBG(args)   printf(args)
#else
#define DBG(args)
#endif /* DEBUG */

/* スクラッチパッド先頭 */
#define SCR_POS   ((FVECTOR *)SCRPAD_ADDR)
#define F_PI  ((float)M_PI)

// koba4
#define FONT_WIDTH (16)
#define FONT_HEIGHT (17)
#define SYS_FONT_WIDTH ( 24 )
#define SYS_FONT_HEIGHT ( 24 )
#define STAGE_FONT_X ( 46 )
#define STAGE_FONT_Y ( 50 )
#define DEFAULT_ROT_X ( -64 )// -98 )
#define DEFAULT_ROT_Y ( -384 )//-60 )
#define DEFAULT_ROT_X_TANKER ( 0 )
#define DEFAULT_ROT_Y_TANKER ( -1024 )
#define SK_R (118)
#define SK_G (134)
#define SK_B (74)
#define SK_A (128)

#define SK_POS_R (110)
#define SK_POS_G (130)
#define SK_POS_B (120)

#define NORMAL_STAGE_ENUM (43)
#define SK_CLOSE_LAYOUT (0x1)

enum {
  SK_TANKER = 0x1 ,
  SK_PLANT = 0x2 ,
  SK_BUG = 0x4 ,
  SK_BREAK_MODE = 0x8 ,
};

enum {
  SK_PARENT_CALL = 0x2,
  SK_PARENT_MOVE_OK = 0x80 ,
  SK_KILL_FLAG      = 0x100 ,
  SK_LOAD_MISS      = 0x200 ,
};

typedef struct {
	int r;
	int g;
	int b;
	int a;
	void *work;
	
   char *mTitleString;
   char *mSubTitleString;
} Font;

typedef struct {
	FVECTOR     point;  // 選択中の場所
	FVECTOR     all_point[ 13 ]; // bomb専用
	int position;       // どこのモデルか？
	int position_x; // 二次元座標上
	int position_y; // 二次元座標上
} Pos;
///

typedef struct {
  DG_DEF  * def;
  DG_OBJS * objs;
  int       queue;
  int       strcode;
} MODEL_SET;


typedef struct _position_work {
	struct _position_work *pUp;
	struct _position_work *pDown;
	struct _position_work *pLeft;
	struct _position_work *pRight;
	int                   position; 
} PositionWork;

typedef struct _map_3d_Work {
  GV_ACT_EX   actor;
  
  int         name;      /* Actor 名 */

  int         handle;    /* 全体マップ装飾用 2D レイアウトのハンドル */
  SPR_OBJ   * point_marker;  /* プレイヤ位置マーキング用基準点       */

  int         model_cnt;
  MODEL_SET * models;

  int         step;      /* 処理ステップ(0は初期化、1～は各 Act 依存) */

  GV_PAD    * pad;       /* 現在のパッド値 */
  void  (*act_func)(struct Work * work);  /* 現在有効な Act のメイン部分 */

  /* 描画チャネル状態保存用 */
  int         ch_flag;
  int         ch_width;
  int         ch_height;
  int         ch_bg_clear_flag;

  SPR_OBJ   * fade;
  int         fade_cnt;   /* フェードタイムカウンタ */

  float       map_zoom;

  FMATRIX     newtral;    /* 非回転状態のマトリクス      */

  float       rot_x;      /* マップモデルの x 軸方向回転 */
  float       rot_y;      /* マップモデルの y 軸方向回転 */
  float       target_x;   /* 目標回転量                  */
  float       target_y;   /* 目標回転量                  */


  FMATRIX     light_active[2];  /* 有効箇所の光源   */
  FMATRIX     light_broken[2];  /* 破壊されている   */
  FMATRIX     light_player[2];  /* プレイヤ現在位置 */
  FMATRIX     light_select[2];  /* 選択している場所 */
  
  int         light_cnt;
  int         player_r;
  int         player_g;
  int         player_b;

  int         light_cnt_select;
  int         select_r;
  int         select_g;
  int         select_b;

  int         parts;

  // koba4
  Font        stage_font; // ステージ文字
  Pos         pos; // つなぎ
  PositionWork pad_position[ 29 ]; // パッドコントロール専用
	
  int         bug_pos;
  int         move_hold;     // Pad Auto Mode Control
  int         root_position; // Player Position
  int         mode;          // Tanker or Plant
  int         flag;

  int         child_kill_count;
  int         sk_codec_flag;
  char        show_flag;     // bomb
  char        sk_2d_flag;

} Work;

static int SK_RootPosition = 0xff;
static void *SK_NodeON; // 既に設定されていないかどうか
char SK_MapStatus;

#ifdef DEBUG_MODE

static int allmap_flag = 1;

static char *debugmenu_items[]  = { "ON", "OFF" } ;
static int   debugmenu_values[] = {    1,    0  } ;
static GM_DEBUG_MENU debug_menu = {
	NULL, "ALLMAP","DISPLAY",
	debugmenu_items,
	debugmenu_values,
	(int *)&allmap_flag, /*target */
	0x01, /*mask*/
	NULL, /*func */
	0   , /*strid*/
	0   , /*type */
	2   , /*max  */
	0,0
};

#endif /* DEBUG_MODE */

// global -> linkvar になるかも
static void act_waiting(Work * work);
static void act_pause(Work * work);
static void set_ambient(FMATRIX * mat, int r, int g, int b);
static void change_light_matrix(DG_OBJS * objs, FMATRIX * mat);
static int StrcodeToEnum( int strcode );
static void StageBreakLightSet( Work *pWork );
// koba4
extern void *NewWorldMapLayout( Pos * , int * , char , char * );
extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void MENU_PutTextScreenF( void *work_ptr, float x1, float y1, float x2, float y2, int u1, int v1, int u2, int v2, int col );
extern void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag );
// etc
extern void GM_JimakuHide( void );
extern void TS_SubWindowWake();
extern void TS_SubWindowSleep();
extern void CODEC_Block(int sw);
extern void CODEC_CallIconSwitch(int sw);

// 操作系
// 28 , 13 27 は はりあー後
// up down left right 
static int position_tabele[ 29 ][ 4 ] = { {   12 , 0xff ,    1 ,   11 } , /* a */
										  {    2 ,    0 ,    2 ,    0 } , /* ab */
										  {    3 ,    1 , 0xff ,    1 } , /* b */
										  {    4 ,    2 , 0xff ,   12 } , /* bc */
										  {   28 ,    3 ,   28 ,    5 } , /* c */
										  {    6 ,    4 ,    4 ,    6 } , /* cd */
										  {   13 ,   12 ,    5 ,    7 } , /* d */
										  {    6 ,    8 ,    6 ,    8 } , /* de */
										  {   27 ,    9 ,    7 ,   27 } , /* e */
										  {    8 ,   10 ,   12 , 0xff } , /* ef */
										  {    9 ,   11 ,   11 , 0xff } , /* f */
										  {   10 ,    0 ,    0 ,   10 } , /* fa */
										  {    6 ,    0 ,    3 ,    9 } , /* sh1 */
										  {   14 ,    6 ,   28 ,   27 } , /* dg */ 
										  {   26 ,   13 ,   15 ,   25 } , /* g */ 
										  {   16 ,   14 ,   16 ,   14 } , /* gh */ 
										  {   17 ,   28 ,   28 ,   15 } , /* h */ 
										  {   18 ,   16 , 0xff ,   26 } , /* hi */ 
										  {   19 ,   17 , 0xff ,   19 } , /* i */ 
										  {   20 ,   18 ,   18 ,   20 } , /* ij */ 
										  { 0xff ,   26 ,   19 ,   21 } , /* j */ 
										  {   20 ,   22 ,   20 ,   22 } , /* jk */ 
										  {   21 ,   23 ,   21 , 0xff } , /* k */ 
										  {   22 ,   24 ,   26 , 0xff } , /* kl */ 
										  {   23 ,   27 ,   25 ,   27 } , /* l */ 
										  {   24 ,   14 ,   14 ,   24 } , /* gl */
										  {   20 ,   14 ,   17 ,   23 } , /* sh2 */ 
										  {   24 ,    8 ,   13 , 0xff } , /* l oil */
										  {   16 ,    4 , 0xff ,   13 } , /* h oil */ };

static void PositionWorkInitialize( Work *pWork )
{
	PositionWork *pPosition;
	int          i;

	for ( i = 0 ; i < 29 ; i ++ ){
		pPosition = &pWork->pad_position[ i ];
		// input
		pPosition->pUp      = ( position_tabele[ i ][ 0 ] == 0xff ? NULL : &pWork->pad_position[ position_tabele[ i ][ 0 ] ] );
		pPosition->pDown    = ( position_tabele[ i ][ 1 ] == 0xff ? NULL : &pWork->pad_position[ position_tabele[ i ][ 1 ] ] );
		pPosition->pLeft    = ( position_tabele[ i ][ 2 ] == 0xff ? NULL : &pWork->pad_position[ position_tabele[ i ][ 2 ] ] );
		pPosition->pRight   = ( position_tabele[ i ][ 3 ] == 0xff ? NULL : &pWork->pad_position[ position_tabele[ i ][ 3 ] ] );
		pPosition->position = i;
	}
}

static void PositionVaturlToReal( Work *pWork , int add_x , int add_y ) // 格納関数
{
	PositionWork *pPosition;
	int          pos_back;
	char         flag;

	pos_back = pWork->pos.position;
	flag = 0x0;
	switch( add_x ){
	case -1 : //
		pPosition = pWork->pad_position[ pWork->pos.position ].pLeft;
		if ( pPosition == NULL ){
			break;
		}
        pWork->pos.position = pPosition->position;
		flag = 0x1;
		break;
	case 1 : //
		pPosition = pWork->pad_position[ pWork->pos.position ].pRight;
		if ( pPosition == NULL ){
			break;
		}
        pWork->pos.position = pPosition->position;
		flag = 0x1;
		break;
	}
	switch( add_y ){
	case -1 : //
		pPosition = pWork->pad_position[ pWork->pos.position ].pUp;
		if ( pPosition == NULL ){
			break;
		}
        pWork->pos.position = pPosition->position;
		flag = 0x1;
		break;
	case 1 : //
		pPosition = pWork->pad_position[ pWork->pos.position ].pDown;
		if ( pPosition == NULL ){
			break;
		}
        pWork->pos.position = pPosition->position;
		flag = 0x1;
		break;
	}
	if ( pWork->pos.position > GM_SelectStageLimit ){ // limit check
		pWork->pos.position = pos_back;
	} else if ( flag & 0x1 ){
		SE_SEL();
	}
}

// signal
static int SignalFunc( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_PARENT_CALL :
		pWork->child_kill_count -= value;
		break;
	case SK_PARENT_MOVE_OK :
		pWork->flag |= SK_PARENT_MOVE_OK;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		return ( -1 );
	}
	return ( 0 );
}

/* あるポイントが画面のどの位置にくるかを返すのを
   つくってみた */
static void SK_TransPersOneChanl( FVECTOR *res, FVECTOR *pos, int chanl )
{
	FMATRIX		*eye_pers ;

	ASSERT( chanl >= 0 && chanl < DG_MAX_CHANLS ) ;
	eye_pers = &DG_Chanl( chanl )->eye_pers ;
	pos->vw = 1.0F ;
	_sceVu0ApplyMatrix( res, eye_pers, pos ) ;
}

static void SK_TransPersOne( FVECTOR *res, FVECTOR *pos )
{
	float		w ;

	SK_TransPersOneChanl( res, pos, MAP_CHANL ) ;
	w = DG_FABS( res->vw ) ;
	res->vx /= w ;
	res->vy /= w ;
	res->vz /= w ;
	res->vx = res->vx * ( float )( DRAW_WIDTH / 2 ) + ( float )( DRAW_WIDTH / 2 ) ;
	res->vy = res->vy * ( float )( DRAW_HEIGHT / 2 ) + ( float )( DRAW_HEIGHT / 2 ) ;
}

static void map_chanl_on(Work * work)
{
  /* 現状の全体マップ表示用チャネルの状態を保存 */
  work->ch_flag          = DG_Chanls[ MAP_CHANL ].flag;
  work->ch_width         = DG_Chanls[ MAP_CHANL ].width;
  work->ch_height        = DG_Chanls[ MAP_CHANL ].height;
  work->ch_bg_clear_flag = DG_Chanls[ MAP_CHANL ].bg_clear_flag;
  
  /* 必要な状態に設定 */
  DG_SetDrawEnv(&DG_Chanls[ MAP_CHANL ],
		0, 0, DRAW_WIDTH, DRAW_HEIGHT);
  DG_Chanls[ MAP_CHANL ].flag          = 1;
  DG_Chanls[ MAP_CHANL ].bg_clear_flag = 2;
  // DG_Chanls[ MAP_CHANL ].bg_clear_flag = 0;
}

static void map_chanl_off(Work * work)
{
  /* マップ表示用チャネルの状態を元に戻す */
  DG_Chanls[ MAP_CHANL ].flag          = work->ch_flag;
  DG_Chanls[ MAP_CHANL ].width         = work->ch_width;
  DG_Chanls[ MAP_CHANL ].height        = work->ch_height;
  DG_Chanls[ MAP_CHANL ].bg_clear_flag = work->ch_bg_clear_flag;
}

static void change_act(Work * work, void (*act_func)(Work * work))
{
  work->act_func = act_func;
  work->step = 0;   /* ステップの初期化 */
}

static void camera_setup(Work * work)
{
  FVECTOR eye;
  FVECTOR base;

  /* カメラ位置を指定する */
  if ( work->mode & SK_TANKER ){
	eye.vx = MAP_CAMERA_X_TANKER ;
	eye.vy = MAP_CAMERA_Y_TANKER ;
	eye.vz = MAP_CAMERA_Z_TANKER ;
	eye.vw = 0.0F ;
  } else { // Plant 
	eye.vx = MAP_CAMERA_X ;
	eye.vy = MAP_CAMERA_Y ;
	eye.vz = MAP_CAMERA_Z ;
	eye.vw = 0.0F ;
  }

  /* 注視点 */
  if ( work->mode & SK_TANKER ){
	  base.vx = MAP_LOOKUP_X_TANKER ;
	  base.vy = MAP_LOOKUP_Y_TANKER ;
	  base.vz = MAP_LOOKUP_Z_TANKER ;
	  base.vw = 0.0f ;
  } else { // Plant
	  base.vx = MAP_LOOKUP_X ;
	  base.vy = MAP_LOOKUP_Y ;
	  base.vz = MAP_LOOKUP_Z ;
	  base.vw = 0.0f ;
  }
  /* カメラマトリクスの設定 */
  BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
  DG_SetCamera2(&DG_Chanls[ MAP_CHANL ], &eye, &base, work->map_zoom);
}

/*
 * マップのモデルを表示状態にする
 */
static void model_setup(Work * work)
{
  int i;

  for(i = 0; i < work->model_cnt; i++)
    {
      DG_QueueObjs(work->models[i].objs);
      work->models[i].queue = 1;
      /* 表示グループを、マップオブジェクトと同じものにする */
      DG_AddCurrentGroup2(MAP_CHANL, work->models[i].objs->group_id);
    }
}

static inline int FontColor( Work *pWork , int raute )
{
	u_int color;

	if ( pWork->pos.position == pWork->root_position ){
		color = SK_POS_R | ( SK_POS_G << 8 ) | ( SK_POS_B << 16 ) | ( ( pWork->stage_font.a / raute ) << 24 ); 
	} else {
		color = pWork->stage_font.r | ( pWork->stage_font.g << 8 ) | ( pWork->stage_font.b << 16 ) | ( ( pWork->stage_font.a / raute ) << 24 ); 
	}
	return ( color );
}

// 文字の表示
static void StringDisp( Work *pWork )
{
	u_int color;
	float x1 , y1 , x2 , y2 , u1 , v1;
	int   disp_width; 
	int   poly_width;

//#ifdef JAPANESE_BP_IGNORE() // 日本
	disp_width = 17 * FONT_WIDTH;	
	poly_width = 16 * SYS_FONT_WIDTH;
	// color
	x1 = (float)(STAGE_FONT_X);
	y1 = (float)(STAGE_FONT_Y + FONT_HEIGHT - 4);
	x2 = (float)(STAGE_FONT_X + disp_width) ;
	y2 = y1 + FONT_HEIGHT + 4;
	u1 = (float)poly_width;
	v1 = SYS_FONT_HEIGHT;

	color = FontColor( pWork , 1 );
	MENU_PutTextScreenF( pWork->stage_font.work , x1 , y1 , x2 , y2 ,
						1 , 0  , (int)u1 + 1 , (int)v1 + 4 , color );
	color = FontColor( pWork , 2 );
	MENU_PutTextScreenF( pWork->stage_font.work , ( x1 + 0.5f ) , ( y1 + 0.5f ) , ( x2 + 0.5f ) , ( y2 + 0.5f ) , 
						1 , 0  , (int)u1 + 1 , (int)v1 + 4 , color );
	// english
	y1 = STAGE_FONT_Y;
	x2 = STAGE_FONT_X + ( 22 * ( FONT_WIDTH - 3 ) );
	y2 = y1 + FONT_HEIGHT - 3;
	u1 = 22 * SYS_FONT_WIDTH;
	color = FontColor( pWork , 1 );
	MENU_PutTextScreenF( pWork->stage_font.work , x1 , y1 , x2 , y2 ,
						1 , (int)v1 + 6  , (int)u1 + 1 , ( (int)v1 * 2 ) + 6 , color );
	color = FontColor( pWork , 2 );
	MENU_PutTextScreenF( pWork->stage_font.work , ( x1 + 0.5f ) , ( y1 + 0.5f ) , ( x2 + 0.5f ) , ( y2 + 0.5f ) ,
						1 , (int)v1 + 6  , (int)u1 + 1 , ( (int)v1 * 2 ) + 6 , color );
/*
#else // 海外
	disp_width = 21 * FONT_WIDTH;	
	poly_width = 22 * SYS_FONT_WIDTH;
	// color
	x1 = STAGE_FONT_X;
	y1 = STAGE_FONT_Y;
	x2 = STAGE_FONT_X + disp_width;
	y2 = STAGE_FONT_Y + FONT_HEIGHT + 2;
	u1 = poly_width;
	v1 = SYS_FONT_HEIGHT;
	color = pWork->stage_font.r | ( pWork->stage_font.g << 8 ) | ( pWork->stage_font.b << 16 ) | ( pWork->stage_font.a << 24 ); 
	MENU_PutTextScreenF( pWork->stage_font.work , x1 , y1 , x2 , y2 ,
						0 , 1  , u1 , v1 + 3 , color );
	color = pWork->stage_font.r | ( pWork->stage_font.g << 8 ) | ( pWork->stage_font.b << 16 ) | ( ( pWork->stage_font.a / 2 ) << 24 ); 
	MENU_PutTextScreenF( pWork->stage_font.work , ( x1 + 0.5f ) , ( y1 + 0.5f ) , ( x2 + 0.5f ) , ( y2 + 0.5f ) ,
						0 , 1  , u1 , v1 + 3 , color );
#endif
*/
}

#define ENGLISH_STAGE (39) // 35 <- 29
// 文字の形成
static void StringCleate( Work *pWork )
{
	int strcode;

	if ( pWork->mode & SK_BUG )
   {
		strcode = StrcodeToEnum( pWork->bug_pos );
	}
   else
   {
		strcode = StrcodeToEnum( pWork->models[ pWork->pos.position ].strcode );
	}
	
	if ( strcode == 0xff )
   {
      MENU_ClearTextTexture( pWork->stage_font.work );
	}
   else
   {
      char * const subTitle = BP_GCL_LOOKUP_NEW_FONT_STRING( GM_GetResource( 6 , strcode ) );
      char * const title = BP_GCL_LOOKUP_NEW_FONT_STRING( GM_GetResource( 6 , strcode + ENGLISH_STAGE ) );

      if( pWork->stage_font.mTitleString != title || pWork->stage_font.mSubTitleString != subTitle )
      {
         pWork->stage_font.mTitleString = title;
         pWork->stage_font.mSubTitleString = subTitle;

         // clear
         MENU_ClearTextTexture( pWork->stage_font.work );
         // create
         MENU_CreateTextTexture( pWork->stage_font.work, 1, 0, 16 * SYS_FONT_WIDTH, SYS_FONT_HEIGHT + 6,
            0, 6, 0, subTitle );
         // english
         MENU_CreateTextTexture( pWork->stage_font.work, 1, 4 + SYS_FONT_HEIGHT, 22 * SYS_FONT_WIDTH, SYS_FONT_HEIGHT,
            0, 0, 0, title );
      }
   }
}

#define BOMB_X_PLUS (-50.0f)
#define BOMB_Y_PLUS (850.0f)
#define BOMB_A_PLUS (1025.0f)
static void PosBombUpdate( Work *pWork )
{
	static float table_y[ 13 ] = { BOMB_A_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , 
								   BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS , BOMB_Y_PLUS };
	FVECTOR pos;
	FVECTOR tmp;
	int       i;
	
	for ( i = 0 ; i < 13 ; i ++ ){
		if ( pWork->models[ i ].objs == NULL ){
			return;
		}
		DG_SetPos( &pWork->models[ i ].objs->world );

		tmp.vx = pWork->models[ i ].def->lx - pWork->models[ i ].def->ux;
		tmp.vy = pWork->models[ i ].def->ly - pWork->models[ i ].def->uy;
		tmp.vz = pWork->models[ i ].def->lz - pWork->models[ i ].def->uz;
		_sceVu0DivVector( &tmp , &tmp , 2.0f );
	
		tmp.vx += pWork->models[ i ].def->ux + BOMB_X_PLUS;
		tmp.vy += pWork->models[ i ].def->uy + table_y[ i ];
		tmp.vz += pWork->models[ i ].def->uz;
		// 実際の場所を求める
		DG_RotVector( &tmp , &tmp , 1 );
		_sceVu0AddVector( &tmp , &tmp , ( FVECTOR * )pWork->models[ i ].objs->world.m[ 3 ] );
		// 透視変換
		SK_TransPersOne( &pos , &tmp );
		pos.vx = ( pos.vx ) * 512.0F / ( float )DRAW_WIDTH ;
		pos.vy = ( pos.vy ) * 384.0F / ( float )DRAW_HEIGHT;

		// 格納
		pWork->pos.all_point[ i ].vx = pos.vx;
		pWork->pos.all_point[ i ].vy = pos.vy;
	}
}

// 終点を求める
static void PosControl( Work *pWork )
{
	FVECTOR pos;
	FVECTOR tmp;

	if ( pWork->models[ pWork->pos.position ].objs == NULL ){
		return;
	}
	DG_SetPos( &pWork->models[ pWork->pos.position ].objs->world );
	// バウンディングから中心を求める
	if ( pWork->mode & SK_TANKER ){
		tmp.vx = pWork->models[ pWork->pos.position ].def->lx - pWork->models[ pWork->pos.position ].def->ux;
		tmp.vy = 0.0f;
		tmp.vz = pWork->models[ pWork->pos.position ].def->lz - pWork->models[ pWork->pos.position ].def->uz;
		_sceVu0DivVector( &tmp , &tmp , 2.0f );
	} else { // Plant
		tmp.vx = pWork->models[ pWork->pos.position ].def->lx - pWork->models[ pWork->pos.position ].def->ux;
		tmp.vy = pWork->models[ pWork->pos.position ].def->ly - pWork->models[ pWork->pos.position ].def->uy;
		tmp.vz = pWork->models[ pWork->pos.position ].def->lz - pWork->models[ pWork->pos.position ].def->uz;
		_sceVu0DivVector( &tmp , &tmp , 2.0f );
	}
	tmp.vx += pWork->models[ pWork->pos.position ].def->ux;
	tmp.vy += pWork->models[ pWork->pos.position ].def->uy;
	tmp.vz += pWork->models[ pWork->pos.position ].def->uz;
	// 実際の場所を求める
	DG_RotVector( &tmp , &tmp , 1 );
	_sceVu0AddVector( &tmp , &tmp , ( FVECTOR * )pWork->models[ pWork->pos.position ].objs->world.m[ 3 ] );
	// 透視変換
	SK_TransPersOne( &pos , &tmp );
	pos.vx = ( pos.vx ) * 512.0F / ( float )DRAW_WIDTH ;
	pos.vy = ( pos.vy ) * 384.0F / ( float )DRAW_HEIGHT;
	// 格納
	pWork->pos.point.vx = pos.vx;
	pWork->pos.point.vy = pos.vy;
}

#define HOLD_TIME (DIRECT_TICK(10))
// 場所選び
static void PositionCheng(Work * pWork)
{
	int dx;
	int dy;
	int sign;
	int add_x , add_y;
	int buf;

	if ( !( pWork->flag & SK_PARENT_MOVE_OK ) ){
		return;
	}
	buf = 0xff;
	add_x = add_y = 0;
	if ( pWork->move_hold > 0 ){
		pWork->move_hold--;
		if ( pWork->mode & SK_TANKER ){
			pWork->models[ pWork->pos.position ].objs->world.m[ 3 ][ 2 ] = 50.0f;
			pWork->models[ pWork->root_position ].objs->world.m[ 3 ][ 2 ] = 50.0f;
		}
		return;
	}
	// analog
	if ( ( DG_FABS( pWork->pad->right_dx - 128 ) >= 64 ) || ( DG_FABS( pWork->pad->right_dy - 128 ) >= 64 ) ){ // 動けません
		pWork->show_flag = 0x0;
		return;
	}
	pWork->show_flag = 0x1;
	dx = pWork->pad->left_dx - 128;
	sign = 1;
	if(dx < 0){
		sign = -1;
	}
	if((dx * sign) > 64){
		buf = pWork->pos.position;
		add_x = sign;
		pWork->move_hold = HOLD_TIME;
	}

	if ( buf == 0xff ){
		dy = pWork->pad->left_dy - 128;
		sign = 1;
		if(dy < 0){
			sign = -1;
		}
		if((dy * sign) > 64){
			buf = pWork->pos.position;
			add_y = sign;
			pWork->move_hold = HOLD_TIME;
		}
	}
	// digi
	if ( pWork->pad->status & PAD_L && buf == 0xff ){
		buf = pWork->pos.position;
		add_x = -1;
		pWork->move_hold = HOLD_TIME;
	}
	if ( pWork->pad->status & PAD_R && buf == 0xff ){
		buf = pWork->pos.position;
		add_x = 1;
		pWork->move_hold = HOLD_TIME;
	}
	if ( pWork->pad->status & PAD_U && buf == 0xff ){
		buf = pWork->pos.position;
		add_y = -1;
		pWork->move_hold = HOLD_TIME;
	}
	if ( pWork->pad->status & PAD_D && buf == 0xff ){
		buf = pWork->pos.position;
		add_y = 1;
		pWork->move_hold = HOLD_TIME;
	}
	// main
	if ( pWork->mode & SK_TANKER ){
		if ( pWork->pos.position > 12 ){
			pWork->pos.position = 1;
		}
		if ( pWork->pos.position < 1 ){
			pWork->pos.position = 12;
		}
	} else { // Plant
		if ( GM_SelectStageLimit != 13 && GM_SelectStageLimit != 28 ){ // 補正
			SK_Err("Map Limit Non Set!!! Default Limit IN\0");
			GM_SelectStageLimit = 13;
		}
		PositionVaturlToReal( pWork , add_x , add_y );
	}
	if ( buf != 0xff ){
		change_light_matrix( pWork->models[ buf ].objs , pWork->light_active );
		StageBreakLightSet( pWork );
		change_light_matrix( pWork->models[ pWork->pos.position ].objs , pWork->light_select );
		change_light_matrix( pWork->models[ pWork->root_position ].objs , pWork->light_player );
	}
	if ( pWork->mode & SK_TANKER ){
		pWork->models[ pWork->pos.position ].objs->world.m[ 3 ][ 2 ] = 50.0f;
		pWork->models[ pWork->root_position ].objs->world.m[ 3 ][ 2 ] = 50.0f;
	}
}

/*
 * モデルの回転
 */
static void model_rot(Work * work)
{
  FVECTOR pos;
  SVECTOR rot;
  int dx;
  int dy;
  int sign;
  float add_x;
  float add_y;
  int i;

  if ( work->mode & SK_BUG ){
	return;
  }
  if ( work->mode & SK_TANKER ){
	for(i = 0; i < work->model_cnt; i++) {
	  pos.vx = MAP_OBJ_X ;
	  pos.vy = MAP_OBJ_Y ;
	  pos.vz = MAP_OBJ_Z ;
	  pos.vw = 0.0f ;
      rot.vx = DEFAULT_ROT_X_TANKER ;
	  rot.vy = DEFAULT_ROT_Y_TANKER ;
	  rot.vz = rot.pad = 0 ;
	  DG_SetPos2( &pos , &rot );
	  DG_GetPos( &work->models[ i ].objs->world );
	}
  } else { // Plant
    /* レバーの値によって、回転目標点を変更する */
	dx = work->pad->right_dx - 128;
	sign = 1;
	if(dx < 0) sign = -1;
	if((dx * sign) < 64) dx = 0;
	if ( dx > 128 ){
	  dx = 128;
	}
	if ( dx < -112 ){
	  dx = -112;
	}

	work->target_y = (float)( dx * 10 + DEFAULT_ROT_Y );
	add_y = (work->target_y - work->rot_y) / 10.0F;
	work->rot_y += add_y;
	dy = work->pad->right_dy - 128;
	sign = 1;
	if(dy < 0) sign = -1;
	if (( dy * sign ) <= 64 ){
	  dy = 0;
	}
	if ( dy > 28 ){
	  dy = 28;
	}
	if ( dy < -10 ){
	  dy = -10;
	}

	work->target_x = (float)( dy * 20 + DEFAULT_ROT_X );
	add_x = (work->target_x - work->rot_x) / 10.0F;
	work->rot_x += add_x;
	for(i = 0; i < work->model_cnt; i++) {
	  pos.vx = MAP_OBJ_X ;
	  pos.vy = MAP_OBJ_Y ;
	  pos.vz = MAP_OBJ_Z ;
	  pos.vw = 0.0F ;
      rot.vx = ( int )work->rot_x ;
	  rot.vy = ( int )work->rot_y ;
	  rot.vz = rot.pad = 0 ;
	  DG_SetPos2( &pos , &rot );
	  DG_GetPos( &work->models[ i ].objs->world );
	}
  }
}

/*
 * プレイヤ箇所のアンビエント補間アニメーション
 */
static void player_pos_anim(Work * work)
{
  int rate;

  if ( work->mode & SK_BUG ){ // Bug Mode 
	return;
  }
  rate = (work->light_cnt += TIME_BASE);
  if(rate > MAP_LIGHT_ANIM_TIME)
    {
      rate = MAP_LIGHT_ANIM_TIME;
      work->light_cnt -= MAP_LIGHT_ANIM_TIME;
    }

  work->player_r = (MAP_AMB_ACT_R - MAP_AMB_PLY_R) * rate
    / MAP_LIGHT_ANIM_TIME + MAP_AMB_PLY_R;
  
  work->player_g = (MAP_AMB_ACT_G - MAP_AMB_PLY_G) * rate
    / MAP_LIGHT_ANIM_TIME + MAP_AMB_PLY_G;
  
  work->player_b = (MAP_AMB_ACT_B - MAP_AMB_PLY_B) * rate
    / MAP_LIGHT_ANIM_TIME + MAP_AMB_PLY_B;

  set_ambient(work->light_player,
	      work->player_r, work->player_g, work->player_b);
//  set_light_color(work->light_player, MAP_COLOR_R / 2 , MAP_COLOR_G / 2 , MAP_COLOR_B / 2 );

}

static inline void StatusReset( Work *pWork ) // by koba4
{
	int tmp;

	tmp = GM_CheckMenuStatus( MENU_STREAM_CH_0 );
	tmp |= GM_CheckMenuStatus( MENU_STREAM_CH_1 );
//	DG_SetPrivilegeMode( 0 );
	CODEC_CallIconSwitch( 1 );

	GM_PopGameStatus();
	GM_PopMenuStatus();

	// 特殊
	if ( tmp & MENU_STREAM_CH_0 ){
		GM_SetMenuStatus( MENU_STREAM_CH_0 );
	} else {
		GM_ResetMenuStatus( MENU_STREAM_CH_0 );
	}
	if ( tmp & MENU_STREAM_CH_1 ){
		GM_SetMenuStatus( MENU_STREAM_CH_1 );
	} else {
		GM_ResetMenuStatus( MENU_STREAM_CH_1 );
	}
	TS_SubWindowWake();

	if ( pWork->sk_codec_flag & 0x1 ){
	  	GM_ResetMenuStatus( MENU_RADIO_DISABLE );
		pWork->sk_codec_flag = 0;
	}
	CODEC_Block( 0 ); // 強制無線禁止解除
	pWork->flag &= ~SK_PARENT_MOVE_OK;
}

/*
 * 選択個所のアンビエント補間アニメーション
 */
static void select_pos_anim(Work * work)
{
  int rate;

  if ( work->mode & SK_BUG ){ // Bug Mode 
	return;
  }
  rate = (work->light_cnt_select += TIME_BASE);
  if(rate > MAP_LIGHT_ANIM_TIME)
    {
      rate = MAP_LIGHT_ANIM_TIME;
      work->light_cnt_select -= MAP_LIGHT_ANIM_TIME;
    }

  work->select_r = (MAP_AMB_ACT_R - MAP_AMB_SEL_R) * rate
    / MAP_LIGHT_ANIM_TIME + MAP_AMB_SEL_R;
  
  work->select_g = (MAP_AMB_ACT_G - MAP_AMB_SEL_G) * rate
    / MAP_LIGHT_ANIM_TIME + MAP_AMB_SEL_G;
  
  work->select_b = (MAP_AMB_ACT_B - MAP_AMB_SEL_B) * rate
    / MAP_LIGHT_ANIM_TIME + MAP_AMB_SEL_B;

  set_ambient(work->light_select,
	      work->select_r, work->select_g, work->select_b);
}

// Bug Mode Pause
static void act_bug_pause( Work *work )
{
  enum {
    STEP_INIT,
    STEP_FDIN,
    STEP_PAUSE,
    STEP_FINISH,
  };

  switch(work->step)
    {
    case STEP_INIT:
      /* マップ表示用チャネルを有効に */
      map_chanl_on(work);
      /* フェードイン準備 */
      work->fade->sprite.col.a = 0;
      SPR_SHOW(work->fade);
      work->fade_cnt = 0;
      work->parts = 0;
      /* 初期化フレームでは、そのまま STEP_PAUSE フェーズになだれ込む */
      work->step = STEP_FDIN;
      /* break は不要(bug ではない) */
    case STEP_FDIN:
      {
	int alpha;
	
	work->fade_cnt += TIME_BASE;
	alpha = work->fade_cnt * MAP_FADE_MAX / MAP_FADE_TIME;
	if(alpha > MAP_FADE_MAX) alpha = MAP_FADE_MAX;
	work->fade->sprite.col.a = alpha;
	if(alpha < MAP_FADE_MAX) break;
	work->step = STEP_PAUSE;
      }
    case STEP_PAUSE:
	  StringCleate( work );
	  StringDisp( work );
      /* このフレーム中にやることがあれば、ここで処理 */
      break;

    case STEP_FINISH:
      {
	int alpha;

	work->fade_cnt -= TIME_BASE;
	alpha = work->fade_cnt * MAP_FADE_MAX / MAP_FADE_TIME;
	if(alpha < 0) alpha = 0;
	work->fade->sprite.col.a = alpha;
	if(alpha > 0) break;
      }
      {
	int i;

	for(i = 0; i < work->model_cnt; i++)
	  {
	    DG_DequeueObjs(work->models[i].objs);
	    work->models[i].queue = 0;
	  }
      }
      map_chanl_off(work);
      SPR_HIDE( work->fade );
      change_act(work, act_waiting);
	  GM_ResetSightStatus( SGT_Invisible );
	  work->sk_2d_flag = 0;
	  // 子供を殺す by koba4
	  GV_CallChildSignalFunc( work , SK_CLOSE_LAYOUT , 0 );
	  work->flag |= SK_KILL_FLAG;
      break;
    }
  /* たとえ何かの途中でも、ポーズが解除されたら全体マップ消去フェーズに入る */
  if((GV_PauseLevelNoXMB != GV_PAUSE_PAUSE) && (work->step == STEP_PAUSE)){      //BP_PAUSE - exclude XMB from pause check
      work->step = STEP_FINISH;
	  SE_CANCEL();
	  // 特権
	  StatusReset( work );
  }
}

/*
 * ポーズ中状態
 */
static void act_pause(Work * work)
{
  enum {
    STEP_INIT,
    STEP_FDIN,
    STEP_PAUSE,
    STEP_FINISH,
  };
  static float zm;

  player_pos_anim( work );
  select_pos_anim( work );
  PosControl( work );
  PosBombUpdate( work );
  /* たとえ何かの途中でも、ポーズが解除されたら全体マップ消去フェーズに入る */
  switch(work->step)
    {
    case STEP_INIT:

      work->map_zoom = 0.0F;

      /* マップ表示用チャネルを有効に */
      map_chanl_on(work);
      
      /* モデルの表示を有効に
	 (モデルの表示フラグ設定、カメラの設定など) */
      model_setup(work);
	  model_rot(work) ;/*マトリックスを準備し忘れてたので入れた　修正T.Morita*/

      /* フェードイン準備 */
      work->fade->sprite.col.a = 0;
      SPR_SHOW(work->fade);

      work->fade_cnt = 0;

      work->parts = 0;
	  StageBreakLightSet( work );
	  change_light_matrix( work->models[ work->root_position ].objs , work->light_player );
      /* 初期化フレームでは、そのまま STEP_PAUSE フェーズになだれ込む */
      work->step = STEP_FDIN;
      /* break は不要(bug ではない) */
    case STEP_FDIN:
      {
	int alpha;
	
	GM_JimakuHide(); // 割込みを考慮して
	work->fade_cnt += TIME_BASE;
	alpha = work->fade_cnt * MAP_FADE_MAX / MAP_FADE_TIME;
	if ( work->mode & SK_TANKER ){
		work->map_zoom = (float)(work->fade_cnt * MAP_CAMERA_ZOOM_TANKER ) / (float)MAP_FADE_TIME;
	} else { // Plant
		work->map_zoom = (float)(work->fade_cnt * MAP_CAMERA_ZOOM) / (float)MAP_FADE_TIME;
	}
#ifdef DEBUG
	//printf("map_zoom = %f\n", work->map_zoom);
#endif
	if(alpha > MAP_FADE_MAX) alpha = MAP_FADE_MAX;
	work->fade->sprite.col.a = alpha;
	if(alpha < MAP_FADE_MAX) break;
	work->step = STEP_PAUSE;
      }
    case STEP_PAUSE:
	  model_rot( work );
	  PositionCheng( work );
	  StringCleate( work );
	  StringDisp( work );
      /* このフレーム中にやることがあれば、ここで処理 */
      break;

    case STEP_FINISH:
      {
	int alpha;

	work->fade_cnt -= TIME_BASE;
	alpha = work->fade_cnt * MAP_FADE_MAX / MAP_FADE_TIME;
	work->map_zoom = (float)(work->fade_cnt * zm) / (float)MAP_FADE_TIME;
#ifdef DEBUG
	//printf("map_zoom = %f\n", work->map_zoom);
#endif
	if(alpha < 0) alpha = 0;
	work->fade->sprite.col.a = alpha;
	camera_setup(work);
	if(alpha > 0) break;
      }
      {
	int i;

	for(i = 0; i < work->model_cnt; i++)
	  {
	    DG_DequeueObjs(work->models[i].objs);
	    work->models[i].queue = 0;
	  }
      }
      map_chanl_off(work);
      SPR_HIDE( work->fade );
      change_act(work, act_waiting);
	  change_light_matrix( work->models[ work->pos.position ].objs , work->light_active );
	  change_light_matrix( work->models[ work->root_position ].objs , work->light_player );
	  work->pos.position = work->root_position;	
	  GM_ResetSightStatus( SGT_Invisible );
	  work->sk_2d_flag = 0;
	  // 子供を殺す by koba4
	  GV_CallChildSignalFunc( work , SK_CLOSE_LAYOUT , 0 );
	  work->flag |= SK_KILL_FLAG;
      break;
    }

  if((GV_PauseLevelNoXMB != GV_PAUSE_PAUSE) && ( work->step == STEP_PAUSE || work->step == STEP_FDIN ) ){   //BP_PAUSE - exclude XMB from pause check
      work->step = STEP_FINISH;
      zm = work->map_zoom;
	  SE_CANCEL();
	  // 特権
	  StatusReset( work );
    }

}


/*
 * ポーズ待ち状態
 */
static void act_waiting(Work * work)
{
  enum {
    STEP_INIT,
    STEP_WAIT
  };
  FVECTOR pos;
  SVECTOR rot;
  void    *ptr;
  int i;

  switch(work->step)
    {
    case STEP_INIT:
      work->step = STEP_WAIT;
      work->light_cnt = 0;
      work->light_cnt_select = 0;
  	  work->rot_x = DEFAULT_ROT_X;
  	  work->rot_y = DEFAULT_ROT_Y;
	  for(i = 0; i < work->model_cnt; i++) {
		  pos.vx = MAP_OBJ_X ;
		  pos.vx = MAP_OBJ_Y ;
		  pos.vx = MAP_OBJ_Z ;
		  pos.vw = 0.0F ;
	      rot.vx = ( int )work->rot_x ;
		  rot.vy = ( int )work->rot_y ;
		  rot.vz = rot.pad = 0 ;
		  DG_SetPos2( &pos , &rot );
		  DG_GetPos( &work->models[ i ].objs->world );
	  }
      /* break 不要 */
    case STEP_WAIT:
#ifdef DEBUG_MODE
      if(!allmap_flag) break;  /* 表示禁止であれば表示しない */
#endif /* DEBUG_MODE */
	  SK_MapStatus = 0;
	  if ( SK_RootPosition != 0xff ){
		change_light_matrix( work->models[ work->root_position ].objs , work->light_active );
	  	work->root_position = work->pos.position = SK_RootPosition;
		change_light_matrix( work->models[ work->pos.position ].objs , work->light_select );
		change_light_matrix( work->models[ work->root_position ].objs , work->light_player );
		SK_RootPosition = 0xff;
	  }

      /* ポーズ状態になっていたら、ポーズ中表示用 Act に移行する */
	  if ( work->child_kill_count <= 0 ){
#if 0
		  if ( work->flag & SK_KILL_FLAG ){ // 無線許可
			if ( work->sk_codec_flag & 0x1 ){
			  	GM_ResetMenuStatus( MENU_RADIO_DISABLE );
				work->sk_codec_flag = 0;
			}
			CODEC_Block( 0 ); // 強制無線禁止解除
			work->flag &= ~SK_KILL_FLAG;
		  }
#endif
	      if( ( GV_PauseLevelNoXMB == GV_PAUSE_PAUSE ) && !( work->flag & SK_LOAD_MISS ) &&   //BP_PAUSE - exclude XMB from pause check   
			  !( GV_PadDataDirect[ 0 ].flag & GV_PAD_RELEASE ) ){ // ポーズ中は基本的にOK 但し、特殊な場合はNG
		    
			if( GM_GameStatus & STATE_VR_ANOTHER && MSN_2DSTATUS & MSN_2DSTAT_WINDOW_PAUSE ){
				/* ＶＲのボーズメニューがでている時はマップはでない */
				return;
			}

			// 特権
			TS_SubWindowSleep();
			GM_PushGameStatus(); // 退避
			if ( !( GM_MenuStatus & MENU_RADIO_DISABLE ) ){
				work->sk_codec_flag = 0x1;
			} else {
				work->sk_codec_flag = 0;
			}
			GM_SetMenuStatus( MENU_RADIO_DISABLE );
			GM_PushMenuStatus();
			// 字幕消し
			GM_JimakuHide();
			GM_SetMenuStatus( MENU_GAGE_OFF );
			GM_SetMenuStatus( MENU_RADAR_OFF );
			GM_SetMenuStatus( MENU_SUBWIN_OFF );
			GM_SetMenuStatus( MENU_MENU_OFF );
//			DG_SetPrivilegeMode( 1 );
			CODEC_CallIconSwitch( 0 );
			// サイトの非表示
			GM_SetSightStatus( SGT_Invisible );
			work->sk_2d_flag = 0x1;
			// にぎやかし
			ptr = NewWorldMapLayout( &work->pos , &work->pos.position , work->mode , &work->show_flag );
			if ( ptr == NULL ){
			  GM_ResetSightStatus( SGT_Invisible );
			  work->sk_2d_flag = 0;
			  StatusReset( work );
			  work->flag |= SK_KILL_FLAG | SK_LOAD_MISS;
			  return;
			}
			GV_SetActorChild( work , ptr );
			CODEC_Block( 1 ); // 強制無線禁止
			work->child_kill_count = 2;
			SK_MapStatus = 1;
			if ( work->mode & SK_BUG ){ // Bug Mode
			  change_act(work, act_bug_pause);
			} else {  // 通常
			  change_act(work, act_pause);
			}
			SE_OK();
		  } else {
			work->flag &= ~SK_LOAD_MISS;
		  }
	  }
      break;
    }
}

static void Act(Work * work)
{
  int i;

  work->pad = &(GV_PadDataDirect[0]);
  (work->act_func)(work);   /* 現在有効な Act 関数を呼び出す */
  if(GV_PauseLevelNoXMB == GV_PAUSE_PAUSE) camera_setup(work); //BP_PAUSE - exclude XMB from pause check

  if ( ( GM_MenuStatus & MENU_RADIO_ON ) || ( GM_MenuStatus & MENU_NODE_ON ) ||
		( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) ){ // 無線が始まろうとしたら all hide
	for(i = 0; i < work->model_cnt; i++){
	    DG_DequeueObjs(work->models[i].objs);
	    work->models[i].queue = 0;
    }
  }
}

static void Die(Work * work)
{
  int i;

  /*
    GM_FreeControl(&(work->control));
    GM_FreeObject(&(work->object));
  */
  SPR_Destroy_2D_Object(work->fade);
  for(i = 0; i < work->model_cnt; i++)
    {
      if(work->models[i].queue)
	DG_DequeueObjs(work->models[i].objs);
      DG_FreeObjs(work->models[i].objs);
    }
  if ( work->sk_2d_flag & 0x1 ){
	GM_ResetSightStatus( SGT_Invisible );
  }
  if ( work->sk_codec_flag & 0x1 ){
	GM_ResetMenuStatus( MENU_RADIO_DISABLE );
  }
  CODEC_Block( 0 ); // 強制無線禁止解除
  CODEC_CallIconSwitch( 1 );

  SK_MapStatus = 0;
  SK_NodeON = NULL;

  GV_Free(work->models);

#ifdef KP_XBOX  
  GM_ReleasePadDisconnect();
#endif  
}

/* 環境光色を設定 */
static void set_ambient(FMATRIX * mat, int r, int g, int b)
{
  mat[1].m[3][0] = (float)r;
  mat[1].m[3][1] = (float)g;
  mat[1].m[3][2] = (float)b;
}

/* 主光源色を設定 */
static void set_light_color(FMATRIX * mat,  int r, int g, int b)
{
  mat[1].m[0][0] = (float)r;
  mat[1].m[0][1] = (float)g;
  mat[1].m[0][2] = (float)b;
}

/* 主光源ベクトルを設定 */
static void set_light_vec(FMATRIX * mat, float x, float y, float z)
{
  FVECTOR vec;

  vec.vx = x;
  vec.vy = y;
  vec.vz = z;

  /* ベクトルを正規化 */
  _sceVu0Normalize(&vec, &vec);

  /* 光源ベクトルを設定 */
  mat[0].m[0][0] = vec.vx;
  mat[0].m[1][0] = vec.vy;
  mat[0].m[2][0] = vec.vz;
}

/*
 * 色変更用の光源マトリクスを用意する
 */
static void set_light_matrixes(Work * work)
{
  /* 光源ベクトルは同じ */
  if ( work->mode & SK_TANKER ){
	set_light_vec( work->light_active, MAP_LIGHT_X_TANKER, MAP_LIGHT_Y_TANKER, MAP_LIGHT_Z_TANKER );
	set_light_vec( work->light_broken, MAP_LIGHT_X_TANKER, MAP_LIGHT_Y_TANKER, MAP_LIGHT_Z_TANKER );
	set_light_vec( work->light_player, MAP_LIGHT_X_TANKER, MAP_LIGHT_Y_TANKER, MAP_LIGHT_Z_TANKER );
	set_light_vec( work->light_select, MAP_LIGHT_X_TANKER, MAP_LIGHT_Y_TANKER, MAP_LIGHT_Z_TANKER );
  } else { // Plant
	set_light_vec( work->light_active, MAP_LIGHT_X, MAP_LIGHT_Y, MAP_LIGHT_Z );
	set_light_vec( work->light_broken, MAP_LIGHT_X, MAP_LIGHT_Y, MAP_LIGHT_Z );
	set_light_vec( work->light_player, MAP_LIGHT_X, MAP_LIGHT_Y, MAP_LIGHT_Z );
	set_light_vec( work->light_select, MAP_LIGHT_X, MAP_LIGHT_Y, MAP_LIGHT_Z );
  }
  /* 光源色は共通 */
  if ( work->mode & SK_TANKER ){
	set_light_color( work->light_active, MAP_COLOR_R_TANKER, MAP_COLOR_G_TANKER, MAP_COLOR_B_TANKER );
	set_light_color( work->light_broken, MAP_COLOR_R_TANKER, MAP_COLOR_G_TANKER, MAP_COLOR_B_TANKER );
	set_light_color( work->light_player, MAP_COLOR_R_TANKER, MAP_COLOR_G_TANKER, MAP_COLOR_B_TANKER );
	set_light_color( work->light_select, MAP_COLOR_R_TANKER, MAP_COLOR_G_TANKER, MAP_COLOR_B_TANKER );
  } else { // Plant
	set_light_color( work->light_active, MAP_COLOR_R, MAP_COLOR_G, MAP_COLOR_B );
	set_light_color( work->light_broken, MAP_COLOR_R, MAP_COLOR_G, MAP_COLOR_B );
	set_light_color( work->light_player, MAP_COLOR_R, MAP_COLOR_G, MAP_COLOR_B );
	set_light_color( work->light_select, MAP_COLOR_R, MAP_COLOR_G, MAP_COLOR_B );
  }
  /*
   * 環境光色の設定
   */
  set_ambient(work->light_active, MAP_AMB_ACT_R, MAP_AMB_ACT_G, MAP_AMB_ACT_B);
  set_ambient(work->light_broken, MAP_AMB_BRK_R, MAP_AMB_BRK_G, MAP_AMB_BRK_B);
  set_ambient(work->light_player, MAP_AMB_PLY_R, MAP_AMB_PLY_G, MAP_AMB_PLY_B);
  set_ambient(work->light_select, MAP_AMB_SEL_R, MAP_AMB_SEL_G, MAP_AMB_SEL_B);

}

/*
 * オブジェクトに与える光源マトリクスを変更する
 */
static void change_light_matrix(DG_OBJS * objs, FMATRIX * mat)
{
  int i;

  for(i = 0; i < objs->n_models; i++)  objs->objs[i].light = mat;
}

/*
 * マップの全てをアクティブ状態にする
 */
static void map_all_active(Work * work, DG_OBJS * objs)
{
  change_light_matrix(objs, work->light_active);
}


static void objs_clear_flag(DG_OBJS * objs, int clear_flag)
{
  int mask = ~clear_flag;
  int i;

  objs->flag &= mask;
  for(i = 0; i < objs->n_models; i++) objs->objs[i].flag &= mask;
}


static int GetResourcesP(Work * work, int name, int where,
			 int * table_le, int model_cnt, int player_pos)
{
  DG_DEF * def;
  DG_OBJS * objs;
  FVECTOR mov;
  SVECTOR rot;
  int i;
  static SPR_POS pos = { 0.0f, 0.0f } ;

  if(table_le == NULL) return -1;


#ifdef DEBUG_MODE
  GM_AddDebugMenu(&debug_menu);
#endif /* DEBUG_MODE */

  work->model_cnt = model_cnt;
  DBG("map_3d.c: [0] model parts = \n");

  /* 光源マトリクスを三種類用意する */
  set_light_matrixes(work);

  if(NULL == (work->models = GV_Malloc(sizeof(MODEL_SET) * model_cnt))){
	SK_Err("Non Memory\0");
    return -1;
  }

  /*
   * フェードアウト用マスクスプライトの用意
   */
  work->fade = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
  SPR_SetPosSprite(work->fade, &pos );
  SPR_SetSizeSprite(work->fade, 512.0F, 384.0F);
  work->fade->sprite.col.r = 8;
  work->fade->sprite.col.g = 24;
  work->fade->sprite.col.b = 8;
  work->fade->sprite.col.a = 0;
  work->fade->head.flags |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
  work->fade->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
  SPR_SetPriority( work->fade , 1 );
  SPR_HIDE(work->fade);
  /* モデルからオブジェクトを生成(モデル個数分繰り返す) */
  for(i = 0; i < model_cnt; i++)
    {
      work->models[i].queue = 0;
	  work->models[ i ].strcode = BP_LE_SwapUInt(*(table_le + i));
      def = work->models[i].def = GV_GetCache(GV_CacheID(BP_LE_SwapUInt(*(table_le + i)), 'k'));
	  if ( def == NULL ){
		return ( -1 );
	  }
//      ASSERT(NULL != def);
      DBG("map_3d.c: [1]\n");
  
	  if ( work->mode & SK_TANKER ) {
#if 0
		if ( i == 0 ){ // 半透明
		  objs = work->models[i].objs = DG_MakeObjs(def, MAP_DG_FLAG | DG_FLAG_SEMITRANS | DG_FLAG_NOFOG , MAP_CHANL);
		} else { // normal
#endif
		  objs = work->models[i].objs = DG_MakeObjs(def, MAP_DG_FLAG | DG_FLAG_NOFOG , MAP_CHANL);
//		}
	  } else { // Plant
	    objs = work->models[i].objs = DG_MakeObjs(def, MAP_DG_FLAG | DG_FLAG_NOFOG , MAP_CHANL);
	  }
	  if ( objs == NULL ){
		return ( -1 );
	  }
//      ASSERT(NULL != objs);
      DBG("map_3d.c: [2]\n");
      
      /* モデル作成直後は、必ず (DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3) が
	 OR されているので、これをクリアしてやらねばならない。 */
      //objs->flag &= ~DG_FLAG_INVISIBLE2;
      objs_clear_flag(objs, DG_FLAG_INVISIBLE2);

      /* objs->flag = (DG_FLAG_INVISIBLE3 | DG_FLAG_ONEPIECE);*/
  
	  if ( work->mode & SK_TANKER ){
	    mov.vx = MAP_OBJ_X ;
		mov.vy = MAP_OBJ_Y ;
		mov.vz = MAP_OBJ_Z ;
		mov.vw = 0.0F ;
        rot.vx = DEFAULT_ROT_X_TANKER ;
		rot.vy = DEFAULT_ROT_Y_TANKER ;
		rot.vz = rot.pad = 0 ;
	  } else { // Plant
	    mov.vx = MAP_OBJ_X ;
		mov.vy = MAP_OBJ_Y ;
		mov.vz = MAP_OBJ_Z ;
		mov.vw = 0.0F ;
        rot.vx = DEFAULT_ROT_X ;
		rot.vy = DEFAULT_ROT_Y ;
		rot.vz = rot.pad = 0 ;
	  }
      DG_SetPos2(&mov, &rot);
      DG_GetPos(&(objs->world));
      work->newtral = objs->world;   /* 初期マトリクスを定義 */

      /* マップの全オブジェクトをアクティブ状態にする */
      map_all_active(work, objs);

      /* プレイヤの位置であれば位置表示用マトリクスを割り当てる */
      if(BP_LE_SwapUInt(*(table_le + i)) == player_pos)
	{
	  DBG("Player Position: \n");
	  work->pos.position = i;
	  work->root_position = i;
//	  PositionUpdate( work ); // by koba4
	  PositionWorkInitialize( work );
//	  change_light_matrix(objs, work->light_player);
	}
    }


  if ( work->mode & SK_TANKER ){
	work->rot_x = DEFAULT_ROT_X_TANKER;
	work->rot_y = DEFAULT_ROT_Y_TANKER;
  } else {
	work->rot_x = DEFAULT_ROT_X;
	work->rot_y = DEFAULT_ROT_Y;
  }
  DBG("map_3d.c: [3]\n");

  /* 初期 Act の設定 */
  change_act(work, act_waiting);

  // シグナルの登録
  GV_SetActorSignalFunc( work , SignalFunc );


  return 0;
}
#define STR_TMAP_BASE (10097643)
#define STR_TMAP_W00A_KANPAN (8725064)
#define STR_TMAP_W00B_OKUJYOU (1223354)
#define STR_TMAP_W01A_1F (13980777)
#define STR_TMAP_W01B_2F (14013577)
#define STR_TMAP_W01C_3F (14046377)
#define STR_TMAP_W01D_4F (14079177)
#define STR_TMAP_W01E_5F (14111977)
#define STR_TMAP_W02A_ENGINEROOM (8389322)
#define STR_TMAP_W03A_NAGAROUKA (3777525)
#define STR_TMAP_W04A_SENSOU1 (2491434)
#define STR_TMAP_W04B_SENSOU2 (2556971)
#define STR_TMAP_W04C_SENSOU3 (2622508)

#define A_W12 (4285016)
#define AB_W13 (5333787)
#define AF_W23 (9528123)
#define B_W14 (5333594)
#define BC_W15 (6382367)
#define C_W16 (6382172)
#define CD_W17 (7430947)
#define D_W18 (7430750)
#define DE_W19 (8479527)
#define DG_W25 (10576707)
#define E_W20 (8479350)
#define EF_W21 (9528129)
#define F_W22 (9527928)
#define G_W25 (10576507)
#define GL_W25 (15819593)
#define L_W25 (15819387)
#define L_W32 (15819416)
#define LK_W25 (14771027)
#define GH (3400)
#define H (104)
#define HI (3433)
#define I (105)
#define IJ (3466)
#define J (106)
#define JK (3499)
#define K (107)
#define W41A (3954305)
#define W42A (3954337)
#define W43A (3954369)
#define W44A (3954401)
#define W45A (3954433)
#define W46A (3954465)
#define AZABU      (4066457)
#define HUNABASHI  (9212239)
#define AOYAMA     (1937000)
#define NEWYORK    (10345114)

#define OTHER (2862361)
#define S1_CENTER_W24 (2177614)
#define S2_CENTER_W31 (2177771)

//#define DEFAULT_PLUS (52)
// MODIFY M.Kobayashi 2002/07/16 スネークテイルズクリアのステージが加わったので 
//#define DEFAULT_PLUS (52+5)	
// MODIFY M.Kobayashi 2002/09/24 スネークテイルズＶＲのステージが加わったので 
#define DEFAULT_PLUS (52+5+5)
  
static int StrcodeToEnum( int strcode ) // ステージ名から判断
{
	switch( strcode ){
/* たんかー */
	case STR_TMAP_W00A_KANPAN : // 船尾甲板
	  return ( 0 + DEFAULT_PLUS );
	case STR_TMAP_W00B_OKUJYOU : // 船尾甲板
	  return ( 7 + DEFAULT_PLUS );
	case STR_TMAP_W01A_1F : // 船橋一階リフレッシュルーム
	  return ( 1 + DEFAULT_PLUS );
	case STR_TMAP_W01B_2F : // 船橋二階居住区
	  return ( 3 + DEFAULT_PLUS );
	case STR_TMAP_W01C_3F : // 船橋三階居住区
	  return ( 4 + DEFAULT_PLUS );
	case STR_TMAP_W01D_4F : // 船橋四階居住区
	  return ( 5 + DEFAULT_PLUS );
	case STR_TMAP_W01E_5F : // 船橋五階操舵室
	  return ( 6 + DEFAULT_PLUS );
	case STR_TMAP_W02A_ENGINEROOM : // 機関室
	  return ( 8 + DEFAULT_PLUS );
	case STR_TMAP_W03A_NAGAROUKA : // 第二甲板右舷
	  return ( 9 + DEFAULT_PLUS );
	case STR_TMAP_W04A_SENSOU1 : // 第一船倉
	  return ( 10 + DEFAULT_PLUS );
	case STR_TMAP_W04B_SENSOU2 : // 第二船倉
	  return ( 11 + DEFAULT_PLUS );
	case STR_TMAP_W04C_SENSOU3 : // 第三船倉
	  return ( 12 + DEFAULT_PLUS );
/* プラント */
	case A_W12 : // Ａ脚
		return ( 13 + DEFAULT_PLUS );
	case AB_W13 : // ＡＢ連絡橋
		return ( 14 + DEFAULT_PLUS );
	case B_W14 : // Ｂ脚変電室
		return ( 15 + DEFAULT_PLUS );
	case BC_W15 : // ＢＣ連絡橋
		return ( 16 + DEFAULT_PLUS );
	case C_W16 : // Ｃ脚食堂
		return ( 17 + DEFAULT_PLUS );
	case CD_W17 : // ＣＤ連絡橋
		return ( 18 + DEFAULT_PLUS );
	case D_W18 : // Ｄ脚第一沈殿池
		return ( 19 + DEFAULT_PLUS );
	case DE_W19 : // ＤＥ連絡橋
		return ( 20 + DEFAULT_PLUS );
	case E_W20 : // Ｅ脚集配場
		return ( 21 + DEFAULT_PLUS );
	case EF_W21 : // ＥＦ連絡橋
		return ( 22 + DEFAULT_PLUS );
	case F_W22 : // Ｆ脚倉庫
		return ( 23 + DEFAULT_PLUS );
	case AF_W23 : // ＦＡ連絡橋
		return ( 24 + DEFAULT_PLUS );
	case S1_CENTER_W24 : // シェル１中央棟
		return ( 25 + DEFAULT_PLUS );
	case DG_W25 : // ＤＧ連絡橋
		return ( 26 + DEFAULT_PLUS );
	case G_W25 : // Ｇ脚
		return ( 27 + DEFAULT_PLUS );
	case GH : // 
		return ( 28 + DEFAULT_PLUS );
	case H : //
		return ( 29 + DEFAULT_PLUS );
	case HI : //
		return ( 30 + DEFAULT_PLUS );
	case I : //
		return ( 31 + DEFAULT_PLUS );
	case IJ : //
		return ( 32 + DEFAULT_PLUS );
	case J : //
		return ( 33 + DEFAULT_PLUS );
	case JK : //
		return ( 34 + DEFAULT_PLUS );
	case K : //
		return ( 35 + DEFAULT_PLUS );
	case LK_W25 : // ＫＬ連絡橋
		return ( 36 + DEFAULT_PLUS );
	case L_W25 : // Ｌ脚外周～ＫＬ連絡橋
		return ( 37 + DEFAULT_PLUS );
	case GL_W25 : 
		return ( 38 + DEFAULT_PLUS );
	case S2_CENTER_W31 : // シェル２中央棟
		return ( 39 + DEFAULT_PLUS );
	case L_W32 : // Ｌ脚下部オイルフェンス
		return ( 40 + DEFAULT_PLUS );
	case OTHER : // 
		return ( 41 + DEFAULT_PLUS );
	case W41A : // 
		return ( 42 + DEFAULT_PLUS );
	case W42A : // 
		return ( 43 + DEFAULT_PLUS );
	case W43A : // 
		return ( 44 + DEFAULT_PLUS );
	case W44A : // 
		return ( 45 + DEFAULT_PLUS );
	case W45A : // 
		return ( 46 + DEFAULT_PLUS );
	case W46A : // 
		return ( 47 + DEFAULT_PLUS );
	case AZABU : // 
		return ( 48 + DEFAULT_PLUS );
	case HUNABASHI : // 
		return ( 49 + DEFAULT_PLUS );
	case AOYAMA : // 
		return ( 50 + DEFAULT_PLUS );
	case NEWYORK : // 
		return ( 51 + DEFAULT_PLUS );
	default :
		return ( 0xff );
	}
	return (0);
}

static int GetResources(Work * work, int name, int where)
{
	int *table_le;
	int model_cnt;
	int player_pos = -1;

	if(NULL == GCL_GetOption('m')) return -1;  /* 'map' */
		table_le = GCL_GetNextResource_LE();   /* テーブルへのポインタを取得 */

	if(NULL != GCL_GetOption('p')) /* 'player' */
  	  	player_pos = GCL_GetNextInt();

	// stage から Tanker or Plant を割出す
	if ( StrcodeToEnum( player_pos ) < 13 + DEFAULT_PLUS ){
		work->mode = SK_TANKER;
	} else if ( StrcodeToEnum( player_pos ) < 42 + DEFAULT_PLUS ) {
		work->mode = SK_PLANT;
	} else {
		work->mode = SK_BUG;
		work->bug_pos = player_pos;
		work->root_position = work->pos.position = StrcodeToEnum( player_pos ) - (DEFAULT_PLUS+13);
	}
	/* モデルの数をカウントする */
	model_cnt = 0;
	while( !( work->mode & SK_BUG ) && ( BP_LE_SwapUInt(*(table_le + model_cnt)) != 0x20 ) ){
	  model_cnt++;
	}
	work->name = name;

	work->stage_font.work = NewTextScreenControlEx( 24 * SYS_FONT_WIDTH , ( SYS_FONT_HEIGHT * 2 ) + 14 , 240, 0x2, 1/*bufferedTextFlag*/ );
	if ( work->stage_font.work == NULL ){
		return ( -1 );
	}
	MENU_ClearTextTexture( work->stage_font.work );
	GV_SetActorChild( work , work->stage_font.work );

	work->stage_font.r = SK_R;
	work->stage_font.g = SK_G;
	work->stage_font.b = SK_B;
	work->stage_font.a = SK_A;

	work->child_kill_count = 0;
	work->sk_codec_flag = 0;
	work->sk_2d_flag = 0;
	SK_RootPosition = 0xff;

	// 二重きどう防止
	SK_NodeON = work;

#ifdef KP_XBOX
	GM_CreatePadDisconnect();
#endif
	return GetResourcesP(work, name, where, table_le, model_cnt, player_pos);
}

void * NewMap3D(int name, int where)
{
  Work * work;

  if ( SK_NodeON != NULL ){ // 二重きどう防止
	return ( SK_NodeON );
  }
  if( NULL == ( work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ) , 180 ) ) )
    return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, name, where)){
      GV_DestroyActor(work);
      return NULL;
  }

  return work;

}

/////////////// シナリオ
#define PLANT_POSITION_MAX (29)
#define MAX_BREAK (5)
static void StageBreakLightSet( Work *pWork ) // 現在登録できるのは4つのみ
{
	static int break_table[] = { BC_W15 , H , G_W25 , DG_W25 , GH };
    static int break_point[ MAX_BREAK ];
	int str_bit;
	int count;
	int i , j;

	count = 0;
	for ( i = 0 ; i < MAX_BREAK ; i ++ ){
		str_bit = StrcodeToEnum( break_table[ i ] );
		str_bit = I64(1) << ( str_bit - (DEFAULT_PLUS+13) );
		if ( GM_StageBreakPoint & str_bit ){
			break_point[ count ] = break_table[ i ];
			count++;
			count %= ( MAX_BREAK + 1 );
		}
	}
	if ( count == 0 ){
		return;
	}
	for ( i = 0 ; i < PLANT_POSITION_MAX ; i ++ ){
		for ( j = 0 ; j < count ; j ++ ){
			if ( pWork->models[ i ].strcode == break_point[ j ] ){
				change_light_matrix( pWork->models[ i ].objs , pWork->light_broken );
			}
		}
	}
}

void SK_RootPositionChange( void )
{
	int position;

	position = GCL_GetOptionValue( 'p' , 0xff );
	// command からの操作があったかどうか
	if ( position != 0xff ){
		SK_RootPosition = StrcodeToEnum( position ) - (DEFAULT_PLUS+13);
		if ( SK_RootPosition <= 0 ){
			SK_RootPosition = 0;
		}
		if ( SK_RootPosition >= 28 ){
			SK_RootPosition = 28;
		}
	} else {
		SK_RootPosition = 0xff;
	}
}

// GM_StageBreakPoint 
void SK_ScnBreakPointSet( void ) // 指定した場所の光源を破壊後光源に切り換える ( mode を) 
{
	int position;

	position = GCL_GetOptionValue( 'p' , 0xff );
	if ( position == 0xff ){
		return;
	}
	position = StrcodeToEnum( position );
	GM_StageBreakPoint |= I64(1) << ( position - (DEFAULT_PLUS+13) );
}

#if 0
void SK_ScnSelectStageLimitSet( void ) // 選択できる最大ステージを決める 
{
	int limit;

	limit = GCL_GetOptionValue( 'l' , 0 );
	if ( limit == 0 ){
		return;
	}
	GM_SelectStageLimit = limit;
}
#endif

void SK_ScnSelectStageLimitAutoSet( void ) // 選択できる最大ステージを決める 
{
	int limit;

	limit = GCL_GetOptionValue( 'l' , 0 );
	switch( limit ){
	case 0 : // normal
		limit = 13;
		break;
	case 1 : // all
		limit = 28;
		break;
	}
	GM_SelectStageLimit = limit;
}

int SK_CheckMapStatus( void ) // map状態をかえす 0 : 待機 1 : 使用中
{
	return ( ( int )SK_MapStatus );
}
