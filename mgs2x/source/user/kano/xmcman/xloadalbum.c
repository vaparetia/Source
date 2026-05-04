/*
  Xbox 写真ロード入り口

  2002/04/22 M.Kobayashi
  $Id: xloadalbum.c,v 1.1.1.3 2002/11/19 11:43:47 Yoshizawa1 Exp $
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

#define		STR_LAYOUT		   (4691731)

#define JPEG_W		(DRAW_WIDTH)
#define JPEG_H		(DRAW_HEIGHT)
#define JPEG_SIZE_LIMIT		( JPEG_W * JPEG_H * 2 * sizeof(short) + 256) //( 24*1024 * sizeof(short) )
#define JPEG_DATA_LIMIT		( 24*1024 * sizeof(short) )

extern void *NewPictureDrawManager( int prio );
extern int UTL_JpegDecode( void *image, int width, int height, void *code );

typedef struct MCX_LOADPHOTO_WORK {
	MCX_MAN	man;
	MCMAN_GAMEDATA	data;
	int		prev_proc;
	int		next_proc;
} MCX_LOADPHOTO_WORK;

static BOOL Load( MCX_LOADPHOTO_WORK* pw )
{	// セーブ時に呼ばれる関数
	// 別スレッドから呼ばれるので注意！
	// 成功したときのみ TRUE を返す
	HANDLE hFile;
	int size;

	if( (hFile = MCX_Open( GENERIC_READ )) == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

//	size = pw->man.sign.dwFileLength - sizeof( MCX_SIGNATURE ) - MCX_FILE_INFO_SIZE;
	size = pw->man.sign.dwFileLength - sizeof( MCX_SIGNATURE );
	
	if( MCX_Read( hFile, pw->man.pLoadBuffer, size ) != size ) {
		MCX_Close( hFile );
		return FALSE;
	}
	if( MCX_Close( hFile ) != 0 ) {
		return FALSE;
	}
	// JPEG をデコードする（時間がかかるのでこちらのスレッドでやってしまう）
	UTL_JpegDecode( (void*)(((long64)pw->man.pDataBuffer + 0xff) & ~0xff), JPEG_W , JPEG_H , pw->man.pLoadBuffer );
	
	return TRUE;
}

static void ExitFunc( MCX_LOADPHOTO_WORK* pw )
{	// 終了関数
	MCX_Free( pw->man.pLoadBuffer );
	MCX_Free( pw->man.pDataBuffer );

//	L2D_ReleaseLayout( pw->man.l2d_handle );

	if( pw->prev_proc != 0) GM_ExecProc( pw->prev_proc, NULL );
}

void *NewAlbumMng( int name )
{	// シナリオから呼ばれる
	MCX_LOADPHOTO_WORK* pw;
	void* pfont_work;
	int l2d_handle;

	if( ( pw = (MCX_LOADPHOTO_WORK*)GV_Malloc( sizeof( MCX_LOADPHOTO_WORK ) ) ) == NULL ) {
		return NULL;
	}

	l2d_handle = L2D_LoadLayout( STR_LAYOUT , DG_CHANL_MENU , 0, 0 ) ; // 後に黒枠がくるため
	
	L2D_EvokeAction( l2d_handle, CODE_DEFAULTACTION );

	pw->prev_proc = GCL_GetOptionValue( 'p' , 0 ); // procの読み込み
	pw->next_proc = GCL_GetOptionValue( 'n' , 0 ); // procの読み込み
	
	pfont_work = NewTextScreenControl();
	MCX_InitManager( &pw->man, l2d_handle, 
					 pfont_work, MCX_MAN_FLAG_PHOTO );
	GV_SetActorChild( &pw->man, pfont_work );
	GV_SetActorChild( &pw->man, NewPictureDrawManager( 140 ));

	// 使用メモリの確保
	pw->man.pDataBuffer = MCX_Malloc( JPEG_SIZE_LIMIT );	// データのデコード先
	pw->man.pLoadBuffer = MCX_Malloc( JPEG_DATA_LIMIT );	// データを読み込むバッファ
	
	MCX_SetSaveLoadFunc( Load, pw );
	MCX_SetExitFunc( ExitFunc, pw );
	// ｌ２ｄカスタマイズ
	pw->man.l2d_tab.a_check_show = CODE_OPENLOAD;
	pw->man.l2d_tab.a_file_show = CODE_OPENSELFILE1;
	pw->man.l2d_tab.a_hide = CODE_CLOSEALL;
	
	GV_SetActorFreeFunc( &pw->man.actor, GV_DelayedFree );
	
	return pw;
}
