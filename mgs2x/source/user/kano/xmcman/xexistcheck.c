/*
  ファイル存在チェック
  VR/Snake Tales でセーブデータの数を返す

  2002/04/22 M.Kobayashi
  $Id: xexistcheck.c,v 1.3 2002/11/23 12:46:57 Yoshizawa1 Exp $
 */

#include	"gameheader.h"
#include	"libfs.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../mode/codec/cdc_load.h"
#include	"../../mode/codec/codec_signal.h"
#include	"../../kano/titlescr/titlescr.h"

typedef struct MCX_EXISTCHECK_WORK {
	MCX_MAN	man;
	int		end_proc;
	int		mode;
} MCX_EXISTCHECK_WORK;

static void ExitFunc( MCX_EXISTCHECK_WORK* pw )
{	// 終了関数
	extern short GM_SaveDataNum[ 4 ];
	
	ASSERT( pw->mode >= 0 && pw->mode <= MCMAN_FILE_KIND_MAX );
	GM_SaveDataNum[ pw->mode ] = pw->man.nData;
	
	if( pw->end_proc != 0 ) {
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=pw->man.nData;

		GCL_ExecProc(pw->end_proc,&arg);
	} 
}

void *NewFileExistCheckScn(int name,int where)
{
    MCX_EXISTCHECK_WORK* pw;
	
	if( (pw = (MCX_EXISTCHECK_WORK *)GV_Malloc( sizeof(MCX_EXISTCHECK_WORK)) ) == NULL ) {
		return NULL;
	}

	pw->end_proc = 0;
	
    if(GCL_GetOption('p')!=NULL){
		pw->end_proc = GCL_GetNextInt();
    }
	if(GCL_GetOption('m')!=NULL){
		pw->mode = GCL_GetNextInt();
	} else pw->mode = 0;
	
	switch( pw->mode ) {
	case MCMAN_FILE_KIND_GAME:
		// マネージャイニシャライズ
		MCX_InitManager( &pw->man, -1, NULL,
						 MCX_MAN_FLAG_FIRSTCHECK | MCX_MAN_FLAG_GAME );
		break;
	case MCMAN_FILE_KIND_VR:
		MCX_InitManager( &pw->man, -1, NULL,
						 MCX_MAN_FLAG_FIRSTCHECK | MCX_MAN_FLAG_VR );
		break;
	case MCMAN_FILE_KIND_SNAKE_TALES:
		MCX_InitManager( &pw->man, -1, NULL,
						 MCX_MAN_FLAG_FIRSTCHECK | MCX_MAN_FLAG_SNAKE_T );
		break;
	}
	MCX_SetExitFunc( ExitFunc, pw );
	GV_SetActorFreeFunc( &pw->man.actor, GV_Free );
	
//	GM_SetGameStatus(STATE_PAUSE_DISABLE);
	
	return pw;
}
