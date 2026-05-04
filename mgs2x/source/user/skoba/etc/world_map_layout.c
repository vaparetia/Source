//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   world_map_layout.c
   全体マップL2D

   2001/07/11 S.Kobayashi
   $Id: world_map_layout.c,v 1.1.1.3 2002/11/19 11:50:23 Yoshizawa1 Exp $
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
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"
#include    "../test/etc.h"

#define		LAYOUT		(114832)	/* map.l2d */
#define 	STR_BLOCK1       (16325211)
#define     STR_CURSOR_TRI (2755675) // tri ではない
#define 	STR_ROOT         (2770484)

#define CENTER_MAX (153)
#define CENTER_SPEED_X (153)
#define BLOCK_MAX (3)
#define SK_CLOSE_LAYOUT (0x1)
#define STR_DEFAULT (566267)
#define STR_DISPSTART (11728985)
#define STR_DISPLAY (6934130)
#define STR_DISPEND (6894589)

#define SK_PARENT_CALL (0x2)
#define SK_PARENT_MOVE_OK (0x80)

#define BOMB_MAX (13)

// stage
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

#define OTHER (2862361)
#define S1_CENTER_W24 (2177614)
#define S2_CENTER_W31 (2177771)

enum {
  SK_TANKER = 0x1 ,
  SK_PLANT = 0x2 ,
  SK_BUG = 0x4 ,
};

enum {
	SK_ACTION_START = 0x1 ,
	SK_ACTION_OK    = 0x2 ,
};

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_VISIBLE          = 0x10,
	SK_NORMAL           = 0x20,
	SK_BUG_MAP          = 0x40,
};

typedef struct {
	FVECTOR     point;  // 選択中の場所
	FVECTOR     all_point[ 13 ]; // bomb専用
	int position;       // どこのモデルか？
} Pos;

typedef struct {
	SPR_OBJ             *obj;
	float               end_position;
	float               div;
	float               add;
	int                 raute;
} Block;

typedef struct {
	SPR_OBJ *obj;
	FVECTOR *pos;
} Bomb; 

typedef struct {
	int bomb;
} DupCount;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;

	Pos                 *pos;
	Block               block[ BLOCK_MAX ];
	DupCount            dup_count;
	Bomb                bomb[ BOMB_MAX ];
	SPR_OBJ             *cursor;
	int                 block_counter;
	int                 action;
	int                 flag;
	FVECTOR             *point;
	SVECTOR             cam_rot;
	int                 *position;
    char                *show_flag;
	int                 invers;
  	int                 raute;
	void  ( *act )( struct _work * );
} Work ;

static u_int SK_BombShow; // 表示

// my
static int AnimationAct( Work *pWork ); // 通常実行
// extern
extern void *NewWorldMapSubLayout( int *position );
extern void *NewWorldMapBugLayout( void );

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;
	int  i;

	switch ( signal ){
	case SK_CLOSE_LAYOUT :
 		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->action = STR_DISPEND;
		// アクションの強制ストップ
		L2D_BreakAction( pWork->handle_2d );
		pWork->act = ( void * )AnimationAct;
		if ( pWork->cursor != NULL ){
			SPR_HIDE( pWork->cursor );
		}
		for ( i = 0 ; i < BOMB_MAX ; i ++ ){
			if ( pWork->bomb[ i ].obj != NULL ){
				SPR_HIDE( pWork->bomb[ i ].obj );
			}
		}
	    GV_CallChildSignalFunc( pWork , SK_CLOSE_LAYOUT , 0 );
		break;
	case SK_PARENT_CALL :
		GV_CallParentSignalFunc( pWork , SK_PARENT_CALL , 1 );
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return 1;
	}

	return ( 0 );
}

#define STR_BOMBTEX (194173)
static int SprInit( Work *pWork )
{
	SPR_OBJ	   *spr;
	int     strcode;
	int           i;

	if ( pWork->flag & SK_INITIALIZE_OK ){
		return (0);
	}
	// bomb
	spr = L2D_GetObject( pWork->handle_2d , STR_BOMBTEX );
	if ( spr == NULL ){
		SK_Err( "bombtex\n");
		return (-1);
	}
	SPR_HIDE( spr );
	for ( i = pWork->dup_count.bomb ; i < BOMB_MAX ; i ++ ){
		pWork->bomb[ i ].obj = SPR_DuplicateTree( spr );
		if ( pWork->bomb[ i ].obj == NULL ){
			SK_Err( "bombtex\0" );
			pWork->bomb[ 0 ].obj = NULL;
			break;
		} else {
			pWork->dup_count.bomb++;
		}
	}
	// blockの取得
	strcode = STR_BLOCK1;
	for ( i = 0 ; i < BLOCK_MAX ; i ++ ){
		spr = L2D_GetObject( pWork->handle_2d, strcode );
		if ( spr == NULL ){
			SK_Err( "block\0" );
			return (-1);
		}
		SPR_SHOW( spr );
		pWork->block[ i ].obj = spr;
		pWork->block[ i ].end_position = irnd() % 161;
		strcode++;
	}
	// cursor
	spr = L2D_GetObject( pWork->handle_2d , STR_CURSOR_TRI );
	if ( spr == NULL ){
		SK_Err( "cursor\n");
		return (-1);
	}
	SPR_SHOW( spr );
	pWork->cursor = spr;

	// 初期化終了
	pWork->flag |= SK_INITIALIZE_OK;	
	return (0);
}

#define IN_FADE_SPEED (4)
#define OUT_FADE_SPEED (8)
static void BombControl( Work *pWork )
{
	int alpha;
	int tmp;
	int i ;

	tmp = 0;
	if ( ( pWork->pos != NULL ) && ( pWork->action != STR_DISPEND ) && ( pWork->bomb[ 0 ].obj != NULL ) ){
		for ( i = 0 ; i < BOMB_MAX ; i ++ ){
			pWork->bomb[ i ].obj->sprite.pos.x = pWork->pos->all_point[ i ].vx - 11;
			pWork->bomb[ i ].obj->sprite.pos.y = pWork->pos->all_point[ i ].vy - 12;
			if ( pWork->show_flag != NULL ){
				alpha = pWork->bomb[ i ].obj->sprite.col.a;
				if ( *pWork->show_flag & 0x1 ){ // fade in
					alpha += IN_FADE_SPEED * pWork->invers;
					if ( pWork->invers > 0 ){
						if ( alpha >= 200 ){
							alpha = 200;
							tmp++;
						}
					} else {
						if ( alpha <= 48 ){
							alpha = 48;
							tmp++;
						}
					}
					pWork->bomb[ i ].obj->sprite.col.a = alpha;
				} else {
					alpha -= OUT_FADE_SPEED;
					if ( alpha <= 0 ){
						alpha = 0;
					}
					pWork->bomb[ i ].obj->sprite.col.a = alpha;
				}
			}
			if ( tmp == BOMB_MAX ){
				pWork->invers *= -1;
			}
			if ( SK_BombShow & ( 0x1 << i ) ){
				SPR_SHOW( pWork->bomb[ i ].obj );
			} else {
				SPR_HIDE( pWork->bomb[ i ].obj );
			}
		}
	}
}

static void CursorControl( Work *pWork )
{
	if ( ( pWork->pos != NULL ) && !( pWork->flag & SK_BUG_MAP ) ){
		if ( pWork->cursor->empty.pos.x != pWork->pos->point.vx ){
			pWork->raute = 60;
		}
		pWork->cursor->empty.pos.x = pWork->pos->point.vx;
		pWork->cursor->empty.pos.y = pWork->pos->point.vy;
		pWork->cursor->head.child->box.col.a = 90;
		pWork->cursor->head.child->head.next->sprite.col.a = 90;
		SPR_SHOW( pWork->cursor );
	}
}

static	void	NormalAct( Work *pWork )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
//	BlockControl( pWork );
	CursorControl( pWork );
	BombControl( pWork );
	AnimationAct( pWork );
	// 更新
	pWork->cam_rot = pCam->rotate;
}

static void Act( Work *pWork )
{
	SPR_OBJ *spr;

	if ( ( GM_MenuStatus & MENU_RADIO_ON ) || ( GM_MenuStatus & MENU_NODE_ON ) ||
		 ( GM_CheckGameStatus( STATE_PLAY_DEMO ) ) ){ // 無線が始まろうとしたら all hide
		spr = L2D_GetObject( pWork->handle_2d , STR_ROOT );
		if ( spr == NULL ){
			return;
		}
		SPR_HIDE( spr );
	}
 	pWork->act( pWork );
}

static int AnimationAct( Work *pWork ) // 通常実行
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch ( pWork->action ){
		case STR_DEFAULT : 
	 		pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->action = STR_DISPSTART;
			break;
		case STR_DISPSTART :
			pWork->action = STR_DISPLAY;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			pWork->act = ( void * )NormalAct;
			if ( SprInit( pWork ) < 0 ){
				return ( -1 );
			}
			GV_CallParentSignalFunc( pWork , SK_PARENT_MOVE_OK , 0 );
			break;
		case STR_DISPLAY :
			pWork->action = STR_DISPLAY;
			pWork->flag |= SK_ACTION_START;
			pWork->flag &= ~SK_ACTION_OK;
			break;
		case STR_DISPEND :
			GV_CallParentSignalFunc( pWork , SK_PARENT_CALL , 2 );
			GV_DestroyActor( pWork );
			break;
		}
	}
	if ( status != L2D_STAT_BUSY && pWork->flag & SK_ACTION_START ){
		if ( status == L2D_STAT_ACK ){
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag &= ~SK_ACTION_START;
			pWork->flag |= SK_ACTION_OK;
		}
	}

	return ( 0 );
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ) L2D_ReleaseLayout( work->handle_2d ) ;
}

/*----------------------------------------------------------------*/
extern void *NewWorldMapBugLayout( void );

static	int	GetResources( Work *work , char mode )
{
	void        *ptr;
	int			handle;
	int         i;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 0, 0 , GV_PAUSE_STOP ) ;
	if ( handle < 0 ){
		return -1 ;
	}
	work->handle_2d = handle ;
#if 0
	memory = GV_Malloc( sizeof( SPR_OBJ ) * BOMB_MAX );
	if ( memory == NULL ){
		return -1 ;
	}
#endif
	// obj
	work->cursor = NULL;
	for ( i = 0 ; i < BOMB_MAX ; i ++ ){
		work->bomb[ i ].obj = NULL;
	}
	// etc
	work->action = STR_DEFAULT;
	work->flag = SK_ACTION_START;
	work->act = (void *)AnimationAct;
	work->dup_count.bomb = 0;
	// シグナルの登録
	GV_SetActorSignalFunc( work , ReceiveSignal );

	if ( mode & SK_BUG ){
		ptr = NewWorldMapBugLayout();
		if ( ptr == NULL ){
			return ( -1 );
		}
		GV_SetActorChild( work , ptr );
		// sub
		ptr = NewWorldMapSubLayout( work->position );
		if ( ptr == NULL ){
			return ( -1 );
		}
		GV_SetActorChild( work , ptr );
		work->flag |= SK_BUG_MAP;
	} else { // Normal
		// sub
		ptr = NewWorldMapSubLayout( work->position );
		if ( ptr == NULL ){
			return ( -1 );
		}
		GV_SetActorChild( work , ptr );
		work->invers = 1;
	}
	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewWorldMapLayout( Pos *pos , int *position , char mode , char *show_flag )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 200 ) ;
	if ( work == NULL ){
		return NULL ;
	}
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	work->pos = pos;
	work->position = position;
	work->show_flag = show_flag;
	if ( GetResources( work , mode ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}

static int StrcodeToEnum( int strcode ) // ステージ名から判断
{
	switch( strcode ){
	case A_W12 : // Ａ脚
		return ( 0x1 );
	case AB_W13 : // ＡＢ連絡橋
		return ( 0x2 );
	case B_W14 : // Ｂ脚変電室
		return ( 0x4 );
	case BC_W15 : // ＢＣ連絡橋
		return ( 0x8 );
	case C_W16 : // Ｃ脚食堂
		return ( 0x10 );
	case CD_W17 : // ＣＤ連絡橋
		return ( 0x20 );
	case D_W18 : // Ｄ脚第一沈殿池
		return ( 0x40 );
	case DE_W19 : // ＤＥ連絡橋
		return ( 0x80 );
	case E_W20 : // Ｅ脚集配場
		return ( 0x100 );
	case EF_W21 : // ＥＦ連絡橋
		return ( 0x200 );
	case F_W22 : // Ｆ脚倉庫
		return ( 0x400 );
	case AF_W23 : // ＦＡ連絡橋
		return ( 0x800 );
	case S1_CENTER_W24 : // シェル１中央棟
		return ( 0x1000 );
	default :
		return ( 0xff );
	}
	return (0);
}

void SK_ScnBombInit( void ) // ステージに入るたび再セットが必要
{
	SK_BombShow = 0 ; 
}

void SK_ScnBombSet( void ) // ステージに入るたび再セットが必要
{
	int strcode;

	strcode = GCL_GetOptionValue( 'b' , 0 );

	if ( strcode == 0 ){
		return;
	}
	strcode = StrcodeToEnum( strcode );
	if ( strcode == 0xff ){
#ifdef DEBUG
		printf("そんなステージに爆弾おけへん\n");
#endif
		return;
	}
	SK_BombShow |= strcode; 
}

void SK_ScnBombReset( void ) // ステージに入るたび再セットが必要
{
	int strcode;

	strcode = GCL_GetOptionValue( 'b' , 0 );

	if ( strcode == 0 ){
		return;
	}
	strcode = StrcodeToEnum( strcode );
	if ( strcode == 0xff ){
#ifdef DEBUG
		printf("そんなステージに爆弾おけへん\n");
#endif
		return;
	}
	SK_BombShow &= ~strcode; 
}
