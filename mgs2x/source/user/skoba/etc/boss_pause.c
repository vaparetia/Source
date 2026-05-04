//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   boss_result.c
   boss rush結果表示
   
   2001/09/18	S.Kobayashi
   $Id: boss_pause.c,v 1.1.1.3 2002/11/19 11:50:18 Yoshizawa1 Exp $
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

extern void ShowExitGameWarning(int *pResult);

#define		LAYOUT		  (15013974) // boss_pause.l2d 
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
#define SK_R (20)
#define SK_G (128)
#define SK_B (32)
#define SK_A (48)
#define SK_LOW_A (52)

#define SK_SUB_Y (22)

#define STR_OPENPAUSE (10812646)
#define STR_CLOSEPAUSE (129121)
#define STR_CURSHIDE (5826320)

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
#define	SK_END                      I64(0x0000200000000000)

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
	int                 handle_2d;
	GV_PAD              *pad; 
	int                 action;
	int                 continue_proc;
	int                 restert_proc;
	int                 exit_proc;
	int                 name;
	int                 position;
	int                 old_position;
	int                 hold_time;
	int                 pad_status;
	int                 pad_check;
	Hokan               cursor;
	Hokan               key_font[ 3 ];
	u_long64              flag;

	void  ( *act )( struct _work * );
} Work ;

static char SK_InvisibleFlag;
// 
static void WaitAct( Work * );
static int AnimationAct( Work * );
static inline int MorfAct( Work *pWork , Hokan *pHokan ); // これのみ実行
static void MorfL2d( int handle , int strcode , int code1 , int code2 , Hokan *hokan );


#if 0
static int SprInit( Work *pWork ) // Sprite初期化
{
	if ( pWork->flag & SK_INITIALIZE_OK ){
		return ( 0 );
	}
	// Sprite取得
	pWork->curs	= SPR_Create_2D_Object( SP_SPRITE , 0 , NULL );
	SPR_SetPosSprite( pWork->curs , &( SPR_POS ){ 64.0F, 64.0F } );
    SPR_SetSizeSprite( pWork->curs , 0.0F, 8.0F);
    /* アルファブレンディングの設定 */
    pWork->curs->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	pWork->curs->sprite.col.r = 20;
	pWork->curs->sprite.col.g = 230;
	pWork->curs->sprite.col.b = 32;
	pWork->curs->sprite.col.a = 48;
	pWork->curs->sprite.dh = 12.0f;
	// プライオリティ 
	SPR_SetPriority( pWork->curs , 6 );
	pWork->curs->head.flags |= SPR_FLAG_ALPHA | SPR_FLAG_PRIV;
	SPR_HIDE( pWork->curs );

	pWork->flag |= SK_INITIALIZE_OK;

	return( 0 );
}

#define SK_SPRITE_ADD_Y ( ( 32 ) )
#define SK_SPRITE_POS_X ( ( DRAW_WIDTH / 2 ) )
#define SK_SPRITE_POS_Y ( ( DRAW_HEIGHT / 2 ) )

static void FontDisp( void )
{
	static char string[ 4 ][ 32 ] = { "CONTINUE" , "RESTERT" , "EXIT" };
	int i;

	// pause
	MENU_Locate( SK_SPRITE_POS_X , SK_SPRITE_POS_Y - 42 , 2 );
	MENU_Color( 128 , 128 , 128 , 128 );
	MENU_Printf( "PAUSE" );
	for ( i = 0 ; i < 3 ; i ++ ) {
		MENU_Locate( SK_SPRITE_POS_X , SK_SPRITE_POS_Y + ( SK_SPRITE_ADD_Y * i ) , 2 );
		MENU_Color( 128 , 128 , 128 , 128 );
		MENU_Printf( string[ i ] );
	}
}
#endif

static void PadControlNormal( Work *pWork )
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( !( pWork->cursor.flag & SK_MOVE_OK ) || ( status == L2D_STAT_BUSY ) ){
		return;
	}
	if ( ( pWork->pad->press & PAD_STA ) ){
		SE_OK();
		// action
		pWork->action = STR_CLOSEPAUSE;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->cursor.code1 = STR_CURSHIDE;
		pWork->cursor.p = 0.0f;
		pWork->cursor.count = 1;
		pWork->cursor.flag = 0;
		pWork->position = 0;
		pWork->flag |= SK_END;
	} else if ( ( pWork->pad->press & PAD_OK/* 2002.05.10 yano */ ) )
   { // 決定
      int execute_action = 1; //BP added
		switch ( pWork->position ){
		case 0 : // Continue
			SE_OK();
			break;
		case 1 : // Restert
         {
            //BP - added system dialog on X360 to prevent destructive action
            //without confirmation.
            int confirmRet;
            ShowExitGameWarning( &confirmRet );
            if( confirmRet == 0 )   //yes
            {
               GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_START01 );
               if ( pWork->restert_proc != 0 ){
                  GM_ExecProc( pWork->restert_proc , NULL );
               }
            }
            else
            {
               //pretend the user did not select anything.
               execute_action = 0;
            }
         }
			break;
		case 2 : // Exit
         {
            //BP - added system dialog on X360 to prevent destructive action
            //without confirmation.
            int confirmRet;
            ShowExitGameWarning( &confirmRet );
            if( confirmRet == 0 )   //yes
            {
               GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_START01 );
               if ( pWork->exit_proc != 0 ){
                  GM_ExecProc( pWork->exit_proc , NULL );
               }
            }
            else
            {
               //pretend the user did not select anything.
               execute_action = 0;
            }
         }
			break;
		}			
      if( execute_action )
      {
		   // action
		   pWork->action = STR_CLOSEPAUSE;
		   pWork->flag |= SK_ACTION_START;
		   pWork->flag &= ~SK_ACTION_OK;
		   pWork->cursor.code1 = STR_CURSHIDE;
		   pWork->cursor.p = 0.0f;
		   pWork->cursor.count = 1;
		   pWork->cursor.flag = 0;
		   pWork->flag |= SK_END;
      }
	}
   else if ( ( pWork->pad->press & PAD_U ) || ( pWork->pad_status & PAD_U ) ){
		if ( pWork->position > 0 ){
			pWork->old_position = pWork->position;
			pWork->position--;
			SE_SEL();
		}
	} else if ( ( pWork->pad->press & PAD_D ) || ( pWork->pad_status & PAD_D ) ){
		if ( pWork->position < 2 ){
			pWork->old_position = pWork->position;
			pWork->position++;
			SE_SEL();
		}
	}
	// auto mode
	if ( pWork->hold_time >= DIRECT_TICK( 30 ) ){
		pWork->pad_status = pWork->pad->status;
		pWork->hold_time -= DIRECT_TICK( 4 );
	} else if ( pWork->pad_check & pWork->pad->status ){
		pWork->hold_time++;
	} else {
		pWork->pad_check = pWork->pad->status;
	} 
	if ( !( pWork->pad->status & ( PAD_D | PAD_U ) ) ){
		pWork->hold_time = 0;
		pWork->pad_status = 0;
	}

}

#define STR_KEY_CURSCONTINUE (2862859)
#define STR_KEY_CURSRESTART (8470944)
#define STR_KEY_CURSEXIT   (5743551)
#define STR_KEY_SELCONTINUE (8682881)
#define STR_KEY_UNCONTINUE (8813495)
#define STR_KEY_SELRESTART (3409940)
#define STR_KEY_UNRESTART  (14948357)
#define STR_KEY_SELEXIT    (14977828)
#define STR_KEY_UNEXIT     (290437)

static void SpriteControl( Work *pWork )
{
	int i;

	if ( pWork->position == pWork->old_position ){
		return;
	}

	pWork->cursor.flag = 0;
	for ( i = 0 ; i < 3 ; i ++ ){
		pWork->key_font[ i ].flag = 0;
	}
	switch ( pWork->position ){
	case 0 : // Continue
		pWork->cursor.code1 = STR_KEY_CURSCONTINUE;
		pWork->cursor.p = 0.0f;
		pWork->cursor.count = 10;
		// back
		pWork->key_font[ 0 ].code2 = pWork->key_font[ 0 ].code1;
		pWork->key_font[ 0 ].code1 = STR_KEY_SELCONTINUE;
		pWork->key_font[ 0 ].p = 0.0f;
		pWork->key_font[ 0 ].count = 10;
		// un
		pWork->key_font[ 1 ].code2 = pWork->key_font[ 1 ].code1;
		pWork->key_font[ 1 ].code1 = STR_KEY_UNRESTART;
		pWork->key_font[ 1 ].p = 0.0f;
		pWork->key_font[ 1 ].count = 10;
		pWork->key_font[ 2 ].code2 = pWork->key_font[ 2 ].code1;
		pWork->key_font[ 2 ].code1 = STR_KEY_UNEXIT;
		pWork->key_font[ 2 ].p = 0.0f;
		pWork->key_font[ 2 ].count = 10;
		break;
	case 1 : // Restert
		pWork->cursor.code1 = STR_KEY_CURSRESTART;
		pWork->cursor.p = 0.0f;
		pWork->cursor.count = 10;
		// back
		pWork->key_font[ 0 ].code2 = pWork->key_font[ 0 ].code1;
		pWork->key_font[ 0 ].code1 = STR_KEY_UNCONTINUE;
		pWork->key_font[ 0 ].p = 0.0f;
		pWork->key_font[ 0 ].count = 10;
		pWork->key_font[ 1 ].code2 = pWork->key_font[ 1 ].code1;
		pWork->key_font[ 1 ].code1 = STR_KEY_SELRESTART;
		pWork->key_font[ 1 ].p = 0.0f;
		pWork->key_font[ 1 ].count = 10;
		pWork->key_font[ 2 ].code2 = pWork->key_font[ 2 ].code1;
		pWork->key_font[ 2 ].code1 = STR_KEY_UNEXIT;
		pWork->key_font[ 2 ].p = 0.0f;
		pWork->key_font[ 2 ].count = 10;
		break;
	case 2 : // Exit
		pWork->cursor.code1 = STR_KEY_CURSEXIT;
		pWork->cursor.p = 0.0f;
		pWork->cursor.count = 10;
		// back
		pWork->key_font[ 0 ].code2 = pWork->key_font[ 0 ].code1;
		pWork->key_font[ 0 ].code1 = STR_KEY_UNCONTINUE;
		pWork->key_font[ 0 ].p = 0.0f;
		pWork->key_font[ 0 ].count = 10;
		pWork->key_font[ 1 ].code2 = pWork->key_font[ 1 ].code1;
		pWork->key_font[ 1 ].code1 = STR_KEY_UNRESTART;
		pWork->key_font[ 1 ].p = 0.0f;
		pWork->key_font[ 1 ].count = 10;
		pWork->key_font[ 2 ].code2 = pWork->key_font[ 2 ].code1;
		pWork->key_font[ 2 ].code1 = STR_KEY_SELEXIT;
		pWork->key_font[ 2 ].p = 0.0f;
		pWork->key_font[ 2 ].count = 10;
		break;
	}
	switch ( pWork->old_position ){
	case 0 : // Continue
		pWork->cursor.code2 = STR_KEY_CURSCONTINUE;
		break;
	case 1 : // Restert
		pWork->cursor.code2 = STR_KEY_CURSRESTART;
		break;
	case 2 : // Exit
		pWork->cursor.code2 = STR_KEY_CURSEXIT;
		break;
	}
	pWork->old_position = pWork->position;
}

static void NormalAct( Work *pWork )
{
	int i;

//	FontDisp();
	SpriteControl( pWork );
	PadControlNormal( pWork );
	// key
	MorfAct( pWork , &pWork->cursor );
	for ( i = 0 ; i < 3 ; i ++ ){
		MorfAct( pWork , &pWork->key_font[ i ] );
	}
}

static void SubWaitAct( Work *pWork )
{
}

static void WaitAct( Work *pWork )
{
	if ( ( GV_PauseLevelNoXMB == GV_PAUSE_PAUSE ) && !( SK_InvisibleFlag & 0x1 ) ){ // 起動   //BP_PAUSE - exclude XMB from pause check
		SE_OK();
		// スタートききません
		GM_SetGameStatus( STATE_PAUSE_DISABLE );
		DG_SetPrivilegeMode( 1 );
		// action
		pWork->action = STR_OPENPAUSE;
		pWork->flag |= SK_ACTION_START;
		pWork->flag &= ~SK_ACTION_OK;
		pWork->act = ( void * )SubWaitAct;
	}
#ifdef KP_XBOX	
	if( ( GV_PauseLevelNoXMB == GV_PAUSE_PAUSE ) && (SK_InvisibleFlag & 0x1) ) {  //BP_PAUSE - exclude XMB from pause check
		// パッド抜け表示も消さなくてはならない。
		GM_ReleasePadDisconnect();
	}
#endif	
}

static void Act( Work *pWork )
{
	if ( !( pWork->flag & SK_END ) ){
	 	pWork->act( pWork );
	}
	AnimationAct( pWork );
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

static inline void Initialize( Work *pWork )
{
	pWork->cursor.code1 = STR_KEY_CURSCONTINUE;
	pWork->cursor.code2 = STR_KEY_CURSCONTINUE;
	pWork->cursor.p = 0.0f;
	pWork->cursor.count = 1;
	pWork->cursor.flag = 0;
	// back
	pWork->key_font[ 0 ].code1 = STR_KEY_SELCONTINUE;
	pWork->key_font[ 0 ].code2 = STR_KEY_UNCONTINUE;
	pWork->key_font[ 0 ].p = 0.0f;
	pWork->key_font[ 0 ].count = 1;
	pWork->key_font[ 0 ].flag = 0;
	pWork->key_font[ 1 ].code1 = STR_KEY_UNRESTART;
	pWork->key_font[ 1 ].code2 = STR_KEY_UNRESTART;
	pWork->key_font[ 1 ].p = 0.0f;
	pWork->key_font[ 1 ].count = 10;
	pWork->key_font[ 1 ].flag = 0;
	pWork->key_font[ 2 ].code1 = STR_KEY_UNEXIT;
	pWork->key_font[ 2 ].code2 = STR_KEY_UNEXIT;
	pWork->key_font[ 2 ].p = 0.0f;
	pWork->key_font[ 2 ].count = 10;
	pWork->key_font[ 2 ].flag = 0;
}

static int AnimationAct( Work *pWork ) // 通常実行
{
	int status;

	status = L2D_ActionStatus( pWork->handle_2d );
	if ( ( status != L2D_STAT_BUSY ) && ( pWork->flag & SK_ACTION_OK ) ){
		switch( pWork->action ){
		case STR_OPENPAUSE :
			pWork->act = ( void * )NormalAct;
			pWork->cursor.code1 = STR_KEY_CURSCONTINUE;
			pWork->cursor.code2 = STR_KEY_CURSCONTINUE;
			pWork->cursor.p = 0.0f;
			pWork->cursor.count = 1;
			pWork->cursor.flag = 0;
			break;
		case STR_CLOSEPAUSE :
			pWork->action = STR_DEFAULT;
			Initialize( pWork );
			L2D_EvokeAction( pWork->handle_2d , pWork->action );
			pWork->flag = 0;
			pWork->old_position = 0;
			DG_SetPrivilegeMode( 0 );
			if ( pWork->position == 0 ){ // continue
				//GV_PauseOffActorSystem( GV_PAUSE_PAUSE ) ;
				//GM_SdSet( SNG_PAUSEOFF ); // sound
				GM_PauseRequest = GM_PAUSE_REQ_OFF ;
				GM_ResetGameStatus( STATE_PAUSE_DISABLE );
#ifdef DEBUG_MODE
				printf("pause解除\n");
#endif
			}
			pWork->position = 0;
			pWork->act = ( void * )WaitAct;
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

static	void	Die( Work *pWork )
{
	if ( pWork->handle_2d >= 0 ){
		L2D_ReleaseLayout( pWork->handle_2d );
	}
	SK_InvisibleFlag = 0;
#ifdef KP_XBOX  
	GM_ReleasePadDisconnect();
#endif  
}

/*----------------------------------------------------------------*/
// mode = 0 : ready , 1 : loop fight
#define STR_EXIT (3435924)
#define STR_RESTART (7057413)
#define STR_CONTINUE (7961512)
#define STR_CURSOR (9100063)
static	int	GetResources( Work *work )
{
	int		handle;

	work->handle_2d = -1 ;
	handle = L2D_LoadLayout2( LAYOUT , DG_CHANL_MENU , 0 , SPR_FLAG_PRIV , GV_PAUSE_STOP ) ; // 後に黒枠がくるため
	if ( handle < 0 ){
		SK_Err("clear_code\0");
		return -1 ;
	}
	work->handle_2d = handle ;

	// シグナルの登録
	// l2d
//	L2D_SetSignalHandle( work->handle_2d , work , Signal );
	work->action = STR_DEFAULT;
	work->flag = SK_ACTION_START;
	// morf
	work->cursor.morf_parts = STR_CURSOR;
	work->key_font[ 0 ].morf_parts = STR_CONTINUE;
	work->key_font[ 1 ].morf_parts = STR_RESTART;
	work->key_font[ 2 ].morf_parts = STR_EXIT;
	Initialize( work );
	// etc
	work->act = ( void * )WaitAct;
	work->pad_status = 0;
	work->pad_check = 0;
	work->pad = &GV_PadDataDirect[ 0 ];
	work->position = 0;
	work->old_position = 0;
	SK_InvisibleFlag = 0;
#if 1
	work->continue_proc = 0;
	work->restert_proc  = GCL_GetOptionValue( 'r' , 0 );
	work->exit_proc     = GCL_GetOptionValue( 'e' , 0 );
#else
	work->continue_proc = 0;
	work->restert_proc = 0;
	work->exit_proc = 0;
#endif
//	SprInit( work );

#ifdef KP_XBOX
	GM_CreatePadDisconnect();
#endif
	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewSK_BossPause( int name )
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

void SK_BossPauseVisibleInvisible( void )
{
	SK_InvisibleFlag ^= 0x1;
}
