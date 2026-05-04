/*
  Xbox ロードドッグタグ入り口

  2002/04/22 M.Kobayashi
  $Id: xloaddog.c,v 1.1.1.3 2002/11/19 11:43:47 Yoshizawa1 Exp $
 */

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../mode/codec/codec_config.h"
#include	"../../mode/codec/cdc_load.h"
#include	"../../kano/titlescr/titlescr.h"

#define DISP_CHANL			4
#define L2D_PRIORITY		0
#define CODE_DEFAULTACTION		0x0008a3fb		/* DefaultAction */


typedef struct MCX_LOADDOG_WORK {
	MCX_MAN	man;
	MCMAN_GAMEDATA	data;
	int name;
	int parent_name;
	int start_mode;
} MCX_LOADDOG_WORK;

static BOOL Load( MCX_LOADDOG_WORK* pw )
{	// セーブ時に呼ばれる関数
	// 別スレッドから呼ばれるので注意！
	// 成功したときのみ TRUE を返す
	HANDLE hFile;

	if( (hFile = MCX_Open( GENERIC_READ )) == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	
	if( MCX_Read( hFile, &pw->data, sizeof(pw->data) ) != sizeof(pw->data) ) {
		MCX_Close( hFile );
		return FALSE;
	}
	if( MCX_Close( hFile ) != 0 ) {
		return FALSE;
	}
	LoadGameData( &pw->data, MCMAN_FILE_KIND_GAME );
	return TRUE;
}

static void ExitFunc( MCX_LOADDOG_WORK* pw )
{	// 終了関数
	if( pw->parent_name != 0 && pw->parent_name != 1){
		GV_MSG msg;
		int message[4];

		msg.address = pw->parent_name;
		msg.message = message;
		msg.message_len = sizeof(message)/sizeof(message[0]);

		message[0] = pw->name;
		message[1] = (pw->man.result == SAVEGAME_ANS_SUCCESS ) ? TITLE_MSG_MODEEND : TITLE_MSG_MODECANCEL;
		GV_SendMessage(&msg);
	}
	pw->man.l2d_handle = -1;	// マネージャにリリースさせない
}

void *NewLoadGameScrForDogtag(int name,int parent_name,int l2d_handle,void *strman,int start_mode)
{	// プログラムから呼ばれる
	MCX_LOADDOG_WORK* pw;

	if( ( pw = (MCX_LOADDOG_WORK*)GV_Malloc( sizeof( MCX_LOADDOG_WORK ) ) ) == NULL ) {
		return NULL;
	}

	pw->name = name;
	pw->parent_name = parent_name;
	pw->start_mode = start_mode;
	
	MCX_InitManager( &pw->man, l2d_handle, 
					 strman, MCX_MAN_FLAG_GAME | MCX_MAN_FLAG_DOGTAG );
	MCX_SetSaveLoadFunc( Load, pw );
	MCX_SetExitFunc( ExitFunc, pw );
	
	// ｌ２ｄカスタマイズ
	if( !start_mode ) {
		pw->man.l2d_tab.a_check_show = CODE_OPENLOAD;
	} else {
		pw->man.l2d_tab.a_check_show = 0;
	}
	pw->man.l2d_tab.a_file_show = CODE_OPENSELFILE1;
	pw->man.l2d_tab.a_hide = CODE_TODOGTAG;
	
	GV_SetActorFreeFunc( &pw->man.actor, GV_DelayedFree );
	
	return pw;
}
