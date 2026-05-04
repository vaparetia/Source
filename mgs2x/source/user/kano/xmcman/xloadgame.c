/*
  Xbox ロードゲーム入り口

  2002/04/22 M.Kobayashi
  $Id: xloadgame.c,v 1.1.1.3 2002/11/19 11:43:47 Yoshizawa1 Exp $
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


typedef struct MCX_LOADGAME_WORK {
	MCX_MAN	man;
	MCMAN_GAMEDATA	data;
	int	proc;
	int mode;
} MCX_LOADGAME_WORK;

static BOOL Load( MCX_LOADGAME_WORK* pw )
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
	LoadGameData( &pw->data, pw->mode );
	return TRUE;
}

static void ExitFunc( MCX_LOADGAME_WORK* pw )
{	// 終了関数
//	L2D_ReleaseLayout( pw->man.l2d_handle );
	if(pw->proc!=0 && pw->proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=((pw->man.result == SAVEGAME_ANS_SUCCESS ) ? 1 : 0);

		GCL_ExecProc(pw->proc,&arg);
	}
}

void *NewLoadGameScr(int name,int where)
{	// シナリオから呼ばれる
	MCX_LOADGAME_WORK* pw;
	int	l2d_strcode;
	int	l2d_handle;
	void* pfont_work;

	if( ( pw = (MCX_LOADGAME_WORK*)GV_Malloc( sizeof( MCX_LOADGAME_WORK ) ) ) == NULL ) {
		return NULL;
	}

    if(GCL_GetOption('d')!=NULL){
		l2d_strcode=GCL_GetNextInt();
	} else l2d_strcode = SAVEGAME_L2D_STRCODE;
    if(GCL_GetOption('p')!=NULL){
		pw->proc=GCL_GetNextInt();
    } else pw->proc = 0;
	if(GCL_GetOption('m')!=NULL){
		pw->mode = GCL_GetNextInt();
	} else pw->mode = 0;

	l2d_handle = L2D_LoadLayout(l2d_strcode, DISP_CHANL, L2D_PRIORITY, SPR_FLAG_PRIV );
	ASSERT( l2d_handle >= 0 );
	L2D_EvokeAction( l2d_handle, CODE_DEFAULTACTION );

	pfont_work = NewTextScreenControl();
	switch( pw->mode ) {
	case MCMAN_FILE_KIND_GAME:
		MCX_InitManager( &pw->man, l2d_handle, 
						 pfont_work, MCX_MAN_FLAG_GAME );
		break;
	case MCMAN_FILE_KIND_VR:
		MCX_InitManager( &pw->man, l2d_handle, 
						 pfont_work, MCX_MAN_FLAG_VR );
		break;
	case MCMAN_FILE_KIND_SNAKE_TALES:
		MCX_InitManager( &pw->man, l2d_handle, 
						 pfont_work, MCX_MAN_FLAG_SNAKE_T );
		break;
	}
	GV_SetActorChild( &pw->man, pfont_work );

	MCX_SetSaveLoadFunc( Load, pw );
	MCX_SetExitFunc( ExitFunc, pw );
	// ｌ２ｄカスタマイズ
	pw->man.l2d_tab.a_check_show = CODE_OPENLOAD;
	pw->man.l2d_tab.a_file_show = CODE_OPENSELFILE1;
	pw->man.l2d_tab.a_hide = CODE_CLOSEALL;
	
	GV_SetActorFreeFunc( &pw->man.actor, GV_DelayedFree );
	
	return pw;
}
