//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	script.c
		game 進行用シナリオ関数記述

	1999/06/24 K.Uehara
	$Id: script.c,v 1.1.1.3 2002/11/19 11:41:55 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <string.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "libfs.h"
#include "libgcl.h"
#include "libhzx.h"

#include "gameheader.h"

#define START_IN_STAGE_SELECT_STICK_THRESHOLD_Y 20

//BP - set this to desired stage to start the game in that stage instead of title sequence.
//( Activated with command line parameter -stage )
extern const char * gBP_InitialStage;

//#undef D
//#define D( a ) a

/* ---------------------------------------------------------------- */
/*
	マップ識別子
*/

int GM_ScriptCurrentMap;

/* ---------------------------------------------------------------- */
/*
	関数宣言
*/

static int PreseekCmd( char *top )
{
	char *stage;

	stage = GCL_GetNextString();

#ifdef PSX2	
	FS_LoadStagePreseek( stage );
#endif	

	return 0;
}

// ステージ移動
static int LoadCmd( char *top )
{
	char *stage;

	/* エリア名が文字列で入っている */
	stage = GCL_GetNextString();

	/* ゲームオーバー中は無効 */
	if ( GM_IsGameOver() ) {
		if ( !( GM_GameOverFlag & GM_OVERFLAG_LOADOK ) ) {
			return GCL_OK ;
		} else {
			if ( GCL_GetOption( 'g' ) != NULL ) {
				GM_GameOverClear() ;
				GM_GameOverFlag &= ~GM_OVERFLAG_LOADOK ;
				printf( "[%d] game over load!!!\n", GV_Time ) ;
			} else {
				return GCL_OK ;
			}
		}
	}

	/* ステージ名が空ならロードのみおこなう */
	if( stage[ 0 ] == '\0' ){
		GM_LoadRequest = 1;
		/* 常駐入れ替えチェックまで飛ぶ */
printf( "stage name is empty\n" ) ;
		goto change_resident_check ;
//		return GCL_OK;
	}

D( printf( "load %s\n", stage ); );
	memmove( &GM_AreaHistory[ 1 ], &GM_AreaHistory[ 0 ], sizeof( int ) * 3 ) ;
	GM_PrevArea = GM_AreaHistory[ 0 ] = GM_SaveArea ;
#ifdef DEBUG_MODE
	{
		int		i ;
		for ( i = 0; i < 4; i ++ ) {
			printf( "AreaHistory[ %d ] = %d\n", i, GM_AreaHistory[ i ] ) ;
		}
	}
#endif

#ifndef GOLD_VERSION
#if MGS_VERSION == 2
   {
      GV_PAD * const pad = &GV_PadDataDirect[0];

      //Push both sticks up to start in stage select
      if(pad->left_dy < START_IN_STAGE_SELECT_STICK_THRESHOLD_Y
         && pad->right_dy < START_IN_STAGE_SELECT_STICK_THRESHOLD_Y)
      {
         gBP_InitialStage = "select";
      }
   }
#endif
#endif

   //BP
   if( gBP_InitialStage )
   {
      //Circumvent the first jump to title screen
      //(this is where the init script would jump to 'select' instead in debug mode)
      if( !strcmp( stage, "n_title" ) )
      {
         stage = gBP_InitialStage;
      }
      gBP_InitialStage = 0;
   }

//	GM_SaveArea = GV_StrCode( stage );
	GM_SetArea( GM_SaveArea, stage );

//D( printf( "m" ) );
	if( GCL_GetOption( 'm' ) != NULL ){
		GM_SaveMap = GCL_GetNextInt();
	}
//D( printf( "p" ) );
	if( GCL_GetOption( 'p' ) != NULL ){
		IVECTOR vec;
		GCL_GetNextIV( ( int * )&vec );
		GM_SaveX = vec.vx;
		GM_SaveY = vec.vy;
		GM_SaveZ = vec.vz;
	}
	if( GCL_GetOption( 's' ) != NULL ){
		/*
		   -s 0: 画面を消す 1:画面を消さない 2:エレベータ
		*/
		GM_LoadRequest = ( GCL_GetNextInt() );
		if( GM_LoadRequest != 0 )	/* こっちが本当 */
//		if( GM_LoadRequest == 2 )
		{
			/* デバッグ用にエレベータのみ画面を残すようにする */
			GM_LoadRequest |= GM_REQ_SCREEN_ON;
		}
	} else {
		GM_LoadRequest = 1;
	}

change_resident_check :

	if( GCL_GetOption( 'n' ) == NULL ){
		/* nosave option がない */
		GM_LoadRequest |= GM_REQ_SAVE_VAR;
	} else {
		if ( GCL_NextStr() != NULL && GCL_GetNextInt() == 0 ) {
			GM_LoadRequest |= GM_REQ_SAVE_VAR ;
		}
	}

	/* 常駐データを入れ替える */
	if ( GCL_GetOption( 'r' ) != NULL ) {
		GM_ChangeResidentArea( GCL_GetNextString() ) ;
#ifdef DEBUG_MODE
		printf( "RESIDENT DATA CHANGE!! %s\n", GM_GetArea() ) ;
#endif
	}

	if( GCL_GetOption( 'c' ) != NULL ){
		int code;

		code = GCL_GetNextInt();
		GCL_ChangeSenerioCode( code );
	}

D( printf( "LoadCmd end\n" ); );
	return GCL_OK ;
}


// キャラクタ実行
static int CharaCmd( char *top )
{
	NEWCHARA *new;
	void *work;
	int id, name, map;
	int		mapbuf1 ;
#ifdef DEBUG_MODE
	int save_id = GCL_DEBUG_CurrentCommandID;
#endif

	id = GCL_GetNextInt();
#ifdef DEBUG_MODE
	GCL_DEBUG_CurrentCommandID = id;
#endif
	if( ( new = GM_GetCharaID( id ) ) == NULL ){
		return GCL_ERROR;
	}
D( printf( "chara %X %X\n", id, new ) );

	name = GCL_GetNextInt();

	map = GM_ScriptCurrentMap ;
	mapbuf1 = GM_CurrentMap ;
	work = ( *new )( name, map );

D( printf( "chara init end\n" ) );
//	ASSERT( work != NULL );
#ifdef DEBUG_MODE
	if( work == NULL ){
	    printf( "CHARA INIT ERROR AD:%X ID:%X NM:%X MP:%X\n", new, id, name, map );
	    HANGUP();
	}
#endif
#ifdef DEBUG_MODE
	GCL_DEBUG_CurrentCommandID = save_id;
#endif

	/* キャラの初期化時にマップが書き変わる危険があるのでもどす */
	GM_ScriptCurrentMap = map;
	GM_CurrentMap = mapbuf1 ;

	return GCL_OK;
}

// プログラム実行

typedef int (*COMMAND_FUNC)( void );

static int CommandCmd( char *top )
{
	NEWCHARA 	*new;
	int 		id;
	int			mapbuf1 ;
#ifdef DEBUG_MODE
	int save_id = GCL_DEBUG_CurrentCommandID;
#endif

	id = GCL_GetNextInt();
#ifdef DEBUG_MODE
	GCL_DEBUG_CurrentCommandID = id;
#endif
	if( ( new = GM_GetCharaID( id ) ) == NULL ){
		return GCL_ERROR;
	}
D( printf( "command %X %X\n", id, new ) );
	mapbuf1 = GM_CurrentMap ;

	GCL_Status = ( *( COMMAND_FUNC )new )();
#ifdef DEBUG_MODE
	GCL_DEBUG_CurrentCommandID = save_id;
#endif
	/* キャラの初期化で変更されるのを防ぐ */
	GM_CurrentMap = mapbuf1 ;

	return GCL_OK;
}

// メッセージコマンド

static int MesgCmd( char *top )
{
	GV_MSG msg;
	int buffer[ 16 ];
	int *p;
	int num;

	msg.address = GCL_GetNextInt();
//printf( "MESG %X:", msg.address );
	p = msg.message = buffer;
	num = 0;
	while( GCL_NextStr() != NULL ){
		*( p ++ ) = GCL_GetNextInt();
//printf( "%d ", *( p - 1 ) );
		num ++;
	}
	GCL_ASSERT( num < 16 );
	msg.message_len = num;
//printf( "(%d)\n", num );
	return ( GV_SendMessage( &msg ) < 0 ) ? GCL_ERROR : GCL_OK;
}

// Map コマンド

static int MapCmd( char *top )
{
	// ダミー
	return GCL_OK;
}

// Trap コマンド
static	int	TrapCmd( char *top )
{
    HZX_BND	bnd ;
    u_short	type, check ;
    int		n ;
//    extern int	GM_CurrentMap ;
	
    bnd.name = GCL_GetNextInt() ;
    bnd.chara = GCL_GetNextInt() ;
    bnd.event = GCL_GetOptionValue( 'm', EV_ENTER ) ;
    bnd.id = GM_GetHzxGroupID( GM_ScriptCurrentMap );
    bnd.time = GCL_GetOptionValue( 'd', 0 ) ;
    check = type = 0 ;
	
    /* ? -> ？, * -> ＊ */
    if ( bnd.chara == EV_CHARA_EVERY_HALF ) bnd.chara = EV_CHARA_EVERY ;
    if ( bnd.event == EV_BOTH_HALF ) bnd.event = EV_BOTH ;
    else if ( bnd.event == EV_BOTH2_HALF ) bnd.event = EV_BOTH2 ;
	
    /* 論理積モード */
    if ( GCL_GetOption( 'a' ) != NULL ) check |= HZX_BND_CHECK_AND ;

	/* 方向 */
    if ( GCL_GetOption( 'd' ) != NULL ) {
		check |= HZX_BND_CHECK_DIR ;	
		n = 0 ;
		while( GCL_NextStr() != NULL ) {
			bnd.dir[ n + 1 ] = GCL_GetNextInt() ;
			bnd.dir_w[ n + 1 ] = GCL_GetNextInt() ;
			if ( ++ n == 3 ) break ;
		}
		bnd.dir[ 0 ] = n ;
    }
	/* ボタン */
    if ( GCL_GetOption( 'b' ) != NULL ) {
		check |= HZX_BND_CHECK_BUTTON ;
		n = 0 ;
		while( GCL_NextStr() != NULL ) {
			bnd.button[ n + 1 ] = GCL_GetNextInt() ;
			if ( ++ n == 3 ) break ;
		}
		bnd.button[ 0 ] = n ;
		/* どのパッドを見るか？ */
		bnd.which = 0 ;
		if ( GCL_GetOption( 'H' ) != NULL ) {
			bnd.which = GCL_GetNextInt() ;
		}
    } else if ( GCL_GetOption( 'k' ) != NULL ) {
		extern u_short	HZX_ButtonMask[] ;
		int				i ;
		u_short			key_bit ;

		check |= HZX_BND_CHECK_BUTTON ;
		n = 0 ;
		while( GCL_NextStr() != NULL ) {
			key_bit = GCL_GetNextInt() & 0xffff ;
			for ( i = 0; i < 16; i ++ ) {
				if ( key_bit == HZX_ButtonMask[ i ] ) break ;
			}
			if ( i == 16 ) i = -1 ;
			bnd.button[ n + 1 ] = i ;
			if ( ++ n == 3 ) break ;
		}
		bnd.button[ 0 ] = n ;
		/* どのパッドを見るか？ */
		bnd.which = 0 ;
		if ( GCL_GetOption( 'H' ) != NULL ) {
			bnd.which = GCL_GetNextInt() ;
		}		
	}

	/* プレイヤー状態 */
    if ( GCL_GetOption( 's' ) != NULL ) {
		check |= HZX_BND_CHECK_STATE ;
		n = 0 ;
		while( GCL_NextStr() != NULL ) {
			bnd.state[ n + 1 ] = GCL_GetNextInt() ;
			if ( ++ n == 3 ) break ;
		}
		bnd.state[ 0 ] = n ;
    }
	/* アイテム */
    if ( GCL_GetOption( 'i' ) != NULL ) {
		check |= HZX_BND_CHECK_ITEM ;
		n = 0 ;
		while( GCL_NextStr() != NULL ) {
			bnd.item[ n + 1 ] = GCL_GetNextInt() ;
			if ( ++ n == 3 ) break ;
		}
		bnd.item[ 0 ] = n ;
    }
	/* 武器 */
    if ( GCL_GetOption( 'w' ) != NULL ) {
		check |= HZX_BND_CHECK_WEAPON ;
		n = 0 ;
		while( GCL_NextStr() != NULL ) {
			bnd.weapon[ n + 1 ] = GCL_GetNextInt() ;
			if ( ++ n == 3 ) break ;
		}
		bnd.weapon[ 0 ] = n ;
    }
	/* アラートモード */
	if ( GCL_GetOption( 'L' ) != NULL ) {
		check |= HZX_BND_CHECK_ALERT ;
		bnd.alert = GCL_GetNextInt() ;
	}
	/* カメラ */
    if ( GCL_GetOption( 'c' ) != NULL ) {
		type |= HZX_BND_TYPE_CAMERA ;
    }
	/* ミサイル専用 */
	if ( GCL_GetOption( 'I' ) != NULL ) {
		type |= HZX_BND_TYPE_MISSILE ;
	}
	/* マップ接合 */
	if ( GCL_GetOption( 'A' ) != NULL ) {
		type |= HZX_BND_TYPE_MAPCONNECT ;
	}	
	/* 実行ブロック */
    if ( GCL_GetOption( 'e' ) != NULL ) {
		int	gtype, value ;

		GCL_GetNextValue( GCL_NextStr(), &gtype, &value ) ;
		GCL_ASSERT( gtype == GCL_BLOCK ) ;
		bnd.command = ( char * )value ;
    } else if ( GCL_GetOption( 'p' ) != NULL ) {
		type |= HZX_BND_TYPE_PROC ;
		bnd.command = ( char * )GCL_GetNextInt() ;
    } 
    bnd.type = type ;
    bnd.check = check ;
    bnd.flag = HZX_BND_FLAG_OUT ;
	
    HZX_SetBind( &bnd, top ) ;
	
    return GCL_OK ;
}

// Restartコマンド
static	int	RestartCmd( char *top )
{
	if ( GM_IsGameOver() ) return GCL_OK ;
    GM_LoadRequest = GM_REQ_RESET ;
	if ( GCL_GetOption( 's' ) != NULL ) {
		/* 現在の変数状態をセーブ */
		GM_LoadRequest |= GM_REQ_SAVE_VAR ;
	}
    return GCL_OK ;
}

/* ---------------------------------------------------------------- */
/*
	外部への定義
*/

static GCL_COMMANDLIST Commands[] = {
	{ 0x3822C7, MesgCmd },
	{ 0x082BC9, CommandCmd },
	{ 0x6592A7, CharaCmd },
	{ 0x3BD490, TrapCmd },
	{ 0x37C884, LoadCmd },
	{ 0x8B3DF5, PreseekCmd },
	{ 0x01C090, MapCmd },
	{ 0x6BB005, RestartCmd },
};

static GCL_COMMANDDEF script_commands = GCL_COMMANDS( Commands );

void GM_ResetScript( void )
{
}

int GM_InitScript( void )
{
	GM_ResetScript();

	return GCL_AddCommMulti( &script_commands );
}

