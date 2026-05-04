/*
  Xbox セーブゲーム入り口

  2002/04/22 M.Kobayashi
  $Id: xsavegame.c,v 1.3 2002/11/23 12:46:57 Yoshizawa1 Exp $
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


#if 0
#define TANKER_IMAGE_NAME	"sitanker.row"
#define PLANT_IMAGE_NAME	"siplant.row"
#define IMAGE_PATH			"D:\\image\\"
//#define VR_IMAGE_NAME		"siother.row"
#else
#define TANKER_IMAGE_NAME	5008448 // "sitanker"
#define PLANT_IMAGE_NAME	3649485	// "siplant"
#define VR_IMAGE_NAME		3879730	// "sivr"
#define ST_IMAGE_NAME		3879636	// "sist"
#endif


#define DISP_CHANL			4
#define L2D_PRIORITY		0
#define CODE_DEFAULTACTION		0x0008a3fb		/* DefaultAction */

#define IMAGE_BUFFER_SIZE	(1024 * 16)

typedef struct MCX_SAVEGAME_WORK {
	MCX_MAN	man;
	MCMAN_GAMEDATA	data;
	int		end_proc;
	int		mode;
	int	buf[ IMAGE_BUFFER_SIZE / sizeof(int) ];  // セーブイメージ用バッファ
} MCX_SAVEGAME_WORK;

static BOOL Save( MCX_SAVEGAME_WORK* pw )
{	// セーブ時に呼ばれる関数
	// 別スレッドから呼ばれるので注意！
	// 成功したときのみ TRUE を返す
	HANDLE hFile;
	int save_ctr;

	// セーブカウントの処理
	save_ctr = GM_SaveCount;
	if( pw->man.flag & ( MCX_MAN_FLAG_GAME | MCX_MAN_FLAG_SNAKE_T ) ) {
		GM_SaveCount++;
	}
		
	SaveGameData( &pw->data, 0, pw->mode);

	if( (hFile = MCX_Open( GENERIC_WRITE )) == INVALID_HANDLE_VALUE ) {
		GM_SaveCount = save_ctr;
		return FALSE;
	}
	
	if( MCX_Write( hFile, &pw->data, sizeof(pw->data) ) != sizeof(pw->data) ) {
		MCX_Close( hFile );
		GM_SaveCount = save_ctr;
		return FALSE;
	}
	if( MCX_Close( hFile ) != 0 ) {
		GM_SaveCount = save_ctr;
		return FALSE;
	}
	return TRUE;
}

static BOOL	CreateImage( MCX_SAVEGAME_WORK* pw )
{	// サムネール画像を作成
#ifdef KP_WINDOWS
pw;
	return TRUE;
#else	
#if 0
	int hFileSrc;
	HANDLE hFileDst;
	int size;
	char fullname[ MAX_PATH ];
	char* imagename;

	if( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
		imagename = IMAGE_PATH TANKER_IMAGE_NAME;
	} else {
		imagename = IMAGE_PATH PLANT_IMAGE_NAME;
	}

//	if( (hFileSrc = pcOpen( "D:\\image\\gameimage0.row", O_RDONLY )) == -1 ) return FALSE;
	if( (hFileSrc = pcOpen( imagename, O_RDONLY )) == -1 ) return FALSE;
	size = pcLseek( hFileSrc, 0, SEEK_END );
	pcLseek( hFileSrc, 0, SEEK_SET );
	
	_snprintf( fullname, MAX_PATH, "%s%s", pw->man.pCurfile->szDir, "saveimage.xbx" );
	if( (hFileDst = CreateFile( fullname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
								NULL ) ) == INVALID_HANDLE_VALUE ) {
		pcClose( hFileSrc );
		return FALSE;
	}

	while( size > 0 ) {
		int curread = min( size, IMAGE_BUFFER_SIZE );
		DWORD dwSizeWritten;

		if( pcRead( hFileSrc, pw->buf, curread ) != curread ) {
			pcClose( hFileSrc ); CloseHandle( hFileDst );
			return FALSE;
		}
		if( !WriteFile( hFileDst, pw->buf, curread, &dwSizeWritten, NULL ) ) {
			pcClose( hFileSrc ); CloseHandle( hFileDst );
			return FALSE;
		}
		size -= curread;
	}
	pcClose( hFileSrc ); CloseHandle( hFileDst );
	return TRUE;
#else
	// サムネールは常駐部に存在するようにした。

//	int hFileSrc;
	HANDLE hFileDst;
	int size;
	char fullname[ MAX_PATH ];
	char *ptr;
	int imagename;

	if( pw->man.flag & MCX_MAN_FLAG_GAME ) {
		if( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
			imagename = TANKER_IMAGE_NAME;
		} else {
			imagename = PLANT_IMAGE_NAME;
		}
	} else if( pw->man.flag & MCX_MAN_FLAG_VR ){
		imagename = VR_IMAGE_NAME;
	} else {
		imagename = ST_IMAGE_NAME;
	}
	ptr = GV_GetCache( GV_CacheID( imagename /* gameimage0 */, 'r' ) );
	ASSERT( ptr != NULL );
	size = GV_GetResidentDataSize( ptr );

	_snprintf( fullname, MAX_PATH, "%s%s", pw->man.pCurfile->szDir, "saveimage.xbx" );
	if( (hFileDst = CreateFile( fullname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
								NULL ) ) == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	while( size > 0 ) {
		int curread = min( size, IMAGE_BUFFER_SIZE );
		DWORD dwSizeWritten;

		if( !WriteFile( hFileDst, ptr, curread, &dwSizeWritten, NULL ) ) {
			CloseHandle( hFileDst );
			return FALSE;
		}
		size -= curread;
		ptr += curread;
	}
	CloseHandle( hFileDst );
	return TRUE;
#endif
#endif
}

static void ExitFunc( MCX_SAVEGAME_WORK* pw )
{	// 終了関数
//	L2D_ReleaseLayout( pw->man.l2d_handle );

	if( pw->end_proc != 0 ) {
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=((pw->man.result == SAVEGAME_ANS_SUCCESS ) ? 1 : 0);

		GCL_ExecProc(pw->end_proc,&arg);
	} else {
		GV_CallParentSignalFunc( &pw->man.actor, CDC_SIGNAL_SAVE_DIE, pw->man.result );
	}
}

void *CODEC_XNewMCSave(int l2d_strcode)
{
	MCX_SAVEGAME_WORK* pw;
	void* l2d_data;
	int	l2d_handle;
	void* pfont_work;

	if( ( pw = (MCX_SAVEGAME_WORK*) codecMalloc( sizeof( MCX_SAVEGAME_WORK ) ) ) == NULL ) {
		return NULL;
	}

	if( (l2d_data = CDC_GetFileEntry(l2d_strcode,'o') ) ==NULL ) {
		HANGUP();
	}
	l2d_handle = L2D_SetupLayout(l2d_data, DISP_CHANL, L2D_PRIORITY, SPR_FLAG_PRIV );
	ASSERT( l2d_handle >= 0 );
	L2D_EvokeAction( l2d_handle, CODE_DEFAULTACTION );

	pfont_work = NewTextScreenControlForCodec();
	MCX_InitManager( &pw->man, l2d_handle, 
					 pfont_work, MCX_MAN_FLAG_SAVE | MCX_MAN_FLAG_GAME | MCX_MAN_FLAG_CODECMEM);

	GV_SetActorChild( &pw->man, pfont_work );

	MCX_SetSaveLoadFunc( Save, pw );
	pw->man.CreateTex = CreateImage;
	MCX_SetExitFunc( ExitFunc, pw );
	// ｌ２ｄカスタマイズ
	pw->man.l2d_tab.a_check_show = CODE_OPENSAVE;
	pw->man.l2d_tab.a_file_show = CODE_OPENSELFILE1;
	pw->man.l2d_tab.a_hide = CODE_CLOSEALL;
	pw->end_proc = 0;
	
	GV_SetActorFreeFunc( &pw->man.actor, codecDelayedFree );
	
	pw->mode = MCMAN_FILE_KIND_GAME;
	
	return pw;
}

void *NewSaveGameScr(int name,int where)
{
    MCX_SAVEGAME_WORK* pw;
	int l2d_handle;
	int l2d_strcode;
	void* pfont_work;
	
	if( (pw = (MCX_SAVEGAME_WORK *)GV_Malloc( sizeof(MCX_SAVEGAME_WORK)) ) == NULL ) {
		return NULL;
	}

	pw->end_proc = 0;
	l2d_handle = 0;
	
    if(GCL_GetOption('d')!=NULL){
		l2d_strcode = GCL_GetNextInt();
		l2d_handle = L2D_LoadLayout( l2d_strcode, DISP_CHANL, L2D_PRIORITY, 0 );
		ASSERT( l2d_handle >= 0 );
		L2D_EvokeAction( l2d_handle, CODE_DEFAULTACTION );
    }
	
    if(GCL_GetOption('p')!=NULL){
		pw->end_proc = GCL_GetNextInt();
    }
	if(GCL_GetOption('m')!=NULL){
		pw->mode = GCL_GetNextInt();
	} else pw->mode = 0;
	
	pfont_work = NewTextScreenControlForTitle();
	switch( pw->mode ) {
	case MCMAN_FILE_KIND_GAME:
		MCX_InitManager( &pw->man, l2d_handle, 
						 pfont_work, MCX_MAN_FLAG_SAVE | MCX_MAN_FLAG_GAME );
		break;
	case MCMAN_FILE_KIND_VR:
		MCX_InitManager( &pw->man, l2d_handle, 
						 pfont_work, MCX_MAN_FLAG_SAVE | MCX_MAN_FLAG_VR );
		break;
	case MCMAN_FILE_KIND_SNAKE_TALES:
		MCX_InitManager( &pw->man, l2d_handle, 
						 pfont_work, MCX_MAN_FLAG_SAVE | MCX_MAN_FLAG_SNAKE_T );
		break;
	}
	
	GV_SetActorChild( &pw->man, pfont_work );

	MCX_SetSaveLoadFunc( Save, pw );
	pw->man.CreateTex = CreateImage;
	MCX_SetExitFunc( ExitFunc, pw );
	// ｌ２ｄカスタマイズ
	pw->man.l2d_tab.a_check_show = CODE_OPENSAVE;
	pw->man.l2d_tab.a_file_show = CODE_OPENSELFILE1;
	pw->man.l2d_tab.a_hide = CODE_CLOSEALL;
	
	GV_SetActorFreeFunc( &pw->man.actor, GV_Free );
	GM_SetGameStatus(STATE_PAUSE_DISABLE);
	
	return pw;
}
