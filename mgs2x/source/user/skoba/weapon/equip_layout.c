//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   equip_layout.c
   装備説明
   
   2001/07/02	S.Kobayashi
   $Id: equip_layout.c,v 1.1.1.3 2002/11/19 11:50:28 Yoshizawa1 Exp $
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

#define		LAYOUT	     	 (9068950)	// equip.l2d 
#define     STR_DEFAULT      (566267)
#define 	STR_BLOCK1       (16325211)
#define 	STR_ROOT         (2770484)
#define     DEFAULT_Y        (-26)
#define 	FONT_ALPHA_MAX   (20.0f)

#define BLOCK_MAX (3)
#define VISIBLE_FLAG    ( 0x2 )
#define INVISIBLE_FLAG  ( 0x4 )
#define FONT_ALPHA_FLAG ( 0x8 )
#define FONT_FADE_OUT_FLAG ( 0x10 )

enum {
	SK_UN_INITIALIZE    = 0x1,
	SK_INITIALIZE_START = 0x2,
	SK_INITIALIZE_OK    = 0x4,
	SK_INVISIBLE        = 0x8,
	SK_VISIBLE          = 0x10,
	SK_NORMAL           = 0x20,
};

typedef struct {
	SPR_OBJ             *obj;
	float               end_position;
	float               div;
	float               add;
} Block;

typedef	struct _work {
	GV_ACT_EX			actor ;
	int					handle_2d;
	int                 action_num;
	int                 action;
	int                 time;
	char                visible_flag;
	int                 flag;
	float               font_alpha;
	float               font_alpha_add;
	int                 name;

	Block               block[ BLOCK_MAX ];
	int                 block_counter;
	void                *menuprint_work_ptr;
	void  ( *act )( struct _work * );
} Work ;

enum {
	SK_FADE_IN = 0x1 ,
	SK_FADE_OUT = 0x2 ,
};

enum {
	SK_DEMO_FONT_FADE_OUT = 0x20,
	SK_DEMO_FONT_ALPHA = 0x80,
};

// プロトタイプ
static int AnimetionAct( Work * ); // アニメーション
// extern
extern void *NewRasterEffect( int alpha, int color );

static int MsgChack( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int          num;
	int         name;
	float       ftmp;

	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	name = 0;
	while( --msg_num >= 0 ){
		num = msg->message[ 0 ];
		if( num == VISIBLE_FLAG ){
			GM_ResetSightStatus( SGT_Invisible );
		}
		if( num == INVISIBLE_FLAG ){
			GM_SetSightStatus( SGT_Invisible );
		}
		if ( num == FONT_ALPHA_FLAG ){
			pWork->font_alpha = 0.0f;
			if ( msg->message[ 1 ] <= 0 ){
				ftmp = 1.0f;
			} else {
				ftmp = ( float )msg->message[ 1 ];
			}
			pWork->font_alpha_add = FONT_ALPHA_MAX / ftmp;
			pWork->flag |= SK_FADE_IN;
			pWork->flag &= ~SK_FADE_OUT;
		}
		if ( num == FONT_FADE_OUT_FLAG ){
			ftmp = msg->message[ 1 ]? ( float )msg->message[ 1 ] : 0.0001f ; // T.Morita Added 2002.03.12
			pWork->font_alpha_add = -( FONT_ALPHA_MAX / ftmp );
			pWork->flag |= SK_FADE_OUT;
			pWork->flag &= ~SK_FADE_IN;
		}
		msg--;
	}
	return ( 0 );
}

static void SprInit( Work *pWork )
{
	SPR_OBJ	   *spr;
	int     strcode;
	int           i;

	if ( pWork->flag & SK_INITIALIZE_OK ){
		return;
	}
	// blockの取得
	strcode = STR_BLOCK1;
	for ( i = 0 ; i < BLOCK_MAX ; i ++ ){
		spr = L2D_GetObject( pWork->handle_2d, strcode );
		if ( spr == NULL ){
			SK_Err( "block\0" );
			return;
		}
		SPR_SHOW( spr );

		pWork->block[ i ].obj = spr;
		pWork->block[ i ].obj->box.col.a = 128;
		pWork->block[ i ].end_position = spr->box.rect.begin.y;
		strcode++;
	}
	// 初期化終了
	pWork->flag |= SK_INITIALIZE_OK;	
}

static void BlockMove( Work *pWork ) // 賑やかし
{
	SPR_OBJ *spr;
	int i;

	if ( !( pWork->flag & SK_INITIALIZE_OK ) ){
		return;
	}
	for ( i = 0 ; i < BLOCK_MAX ; i ++ ){
		spr = pWork->block[ i ].obj;
		if ( ( ( pWork->block[ i ].end_position > spr->box.rect.begin.y - 1.0f ) && 
			   ( pWork->block[ i ].end_position < spr->box.rect.begin.y + 1.0f ) ) ||
			   spr->box.rect.begin.y == spr->box.rect.end.y ){
			pWork->block[ i ].end_position = spr->box.rect.end.y - ( rnd() * 16.0f ) - 119.0f;
			pWork->block[ i ].end_position = pWork->block[ i ].end_position < 153.0f ? 153.0f : pWork->block[ i ].end_position;
			pWork->block[ i ].div = ( rnd() * 1.0f ) + 1.6f;
			pWork->block[ i ].add = ( pWork->block[ pWork->block_counter ].obj->box.rect.end.y - 
									  pWork->block[ pWork->block_counter ].obj->box.rect.begin.y ) / 12.0f;
			pWork->block[ i ].end_position += pWork->block[ i ].add;
			if ( i == 0 ){
				pWork->block[ i ].end_position += rnd() * 20.0f;
			}
		} else {
			// 更新
			spr->box.rect.begin.y += ( pWork->block[ i ].end_position - spr->box.rect.begin.y ) / pWork->block[ i ].div;
			// はみ出し制御
			if ( spr->box.rect.begin.y <= 153.0f ){
				spr->box.rect.begin.y = 153.0f;
			}
			if ( spr->box.rect.begin.y >= spr->box.rect.end.y ){
				spr->box.rect.begin.y =  spr->box.rect.end.y;
			}
		}
	}
	pWork->block_counter++;
	pWork->block_counter %= BLOCK_MAX;
}

static void FontUpdate( Work *pWork )
{
	if ( pWork->flag & SK_FADE_IN ){
		if ( pWork->font_alpha < FONT_ALPHA_MAX ){
			pWork->font_alpha += pWork->font_alpha_add;
		} else {
			pWork->font_alpha = FONT_ALPHA_MAX;
			pWork->flag &= ~SK_FADE_IN;
		}
	}
	if ( pWork->flag & SK_FADE_OUT ){
		if ( pWork->font_alpha > 0.0f ){
			pWork->font_alpha += pWork->font_alpha_add;
		} else {
			pWork->font_alpha = 0.0f;
			pWork->flag &= ~SK_FADE_OUT;
		}
	}
}

static void NumberMove( Work *pWork )
{
	char text_buffer[ 10 ];
	float parent_x , parent_y;

	if ( pWork->flag & SK_INVISIBLE ){//|| pWork->font_alpha < 1.0f || pWork->font_alpha > 255.0f ){
		return;
	}
	parent_x = 53;
	parent_y = DIRECT_SCREEN_Y( 309.f );
	sprintf( text_buffer , "%04d %04d" , irnd() % 10000 , irnd() % 5000 + irnd() % 5001 );
	___MENU_Locate( pWork->menuprint_work_ptr, 2, parent_x , parent_y  , 0 );
	___MENU_Color( pWork->menuprint_work_ptr, 2, 180, 250, 90, 60 );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 2, text_buffer );
#if 0
	parent_x = 53;
	parent_y = 309.f;
	MENU_S_Locate( parent_x , parent_y , 0 );
	MENU_S_Color( 180 , 250 , 90 , ( u_char )pWork->font_alpha );
	MENU_S_Printf( "%04d %04d" , irnd() % 10000 , irnd() % 5000 + irnd() % 5001 );
#endif
}

static int NormalAct( Work *pWork )
{
    if ( pWork->flag & SK_INVISIBLE ){
		return ( 1 );
	}
	AnimetionAct( pWork );
	NumberMove( pWork );
	FontUpdate( pWork );
	BlockMove( pWork );
	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return( 1 );
	}

	return ( 0 );
}

static int AnimetionAct( Work *pWork ) // 通常実行
{
	int status;

	if ( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ){
		return ( -1 );
	}
	status = L2D_ActionStatus( pWork->handle_2d );
	if ( pWork->action < pWork->action_num ){
		if ( status != L2D_STAT_BUSY ){
 			L2D_EvokeActionByNumber( pWork->handle_2d , pWork->action );
			SprInit( pWork );
		}
	}
	return ( 0 );
}

static void Act( Work *pWork )
{
	MsgChack( pWork );
	// デモ等のサイトの表示非表示管理
	if ( SthingerSightVisibleInvisible( pWork->handle_2d , STR_ROOT , ( pWork->flag & SK_INVISIBLE ? 1 : 0 ) ) > 0){
		pWork->flag &= ~SK_INVISIBLE;
	} else {
		pWork->flag |= SK_INVISIBLE;
	}
 	pWork->act( pWork );
#if 0
	if ( pWork->visible_flag == SK_INVISIBLE ){
		return;
	}
#endif
}

static	void	Die( Work *work )
{
	GM_ResetSightStatus( SGT_Invisible ); // 元にもどす

	if ( work->handle_2d >= 0 ){
		L2D_ReleaseLayout( work->handle_2d );
	}
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	void *ptr;
	int			handle;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( LAYOUT , DG_CHANL_MAIN , 0, 0 ) ;
	if ( handle < 0 ){
		return -1 ;
	}
	work->handle_2d = handle ;
	L2D_EvokeActionByNumber( work->handle_2d , 0 ); // default action
	// etc
	work->action_num = L2D_GetActionNumber( work->handle_2d );
	work->action = 1;
	work->flag = 0;
	work->act = (void *)NormalAct;
	work->block_counter = 0;

	work->font_alpha = 0.0f;
	work->font_alpha_add = 0.0f;//FONT_ALPHA_MAX / ( float )360;

	/* MENU_Printfキャラ生成 */
	work->menuprint_work_ptr = NewMenuPrintManager( ( 8 * 1024 ) , DG_DMAPACK_NORMAL , DG_DMAPACK_PHASE_NORMAL , 255 );
	if ( work->menuprint_work_ptr == NULL ){
		return ( -1 );
	}
	GV_SetActorChild( work , work->menuprint_work_ptr );

	ptr = NewRasterEffect( SCE_GS_SET_ALPHA(  0 , 1 , 0 , 1 , 0x00 ) , 0x36002000 );
	if ( ptr == NULL ){
		return ( -1 );
	}
	GV_SetActorChild( work , ptr );

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewDEMO_Equip( int name )
{
	Work		*work ;

	work = GV_NewActorPrio( GV_ACTOR_AFTER2, sizeof( Work ), 32 ) ;
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
