/*
  Xbox 用メモリカードマネージャ

  2002/04/10 M.Kobayashi
  $Id: xmcman.c,v 1.15 2002/09/25 06:56:00 usr03700 Exp $

 */

#include	"gameheader.h"
#include	"mts.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../titlescr/portsel.h"
#include	"../../mode/codec/codec_signal.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../kira/2D_action/layout_2d.h"

#define	THREAD_STACKSIZE	(1024 * 32)

#define TRI_STRCODE_P	GV_StrCode( "photo_save" )
#define TRI_STRCODE_G	GV_StrCode( "save_load" )

// 各ステートオブジェクト
static 	MCX_MAN*	pMcxman;

static void MCX_UpdateInterp( MCX_INTERP* pi );


static void Act( MCX_MAN* pm )
{
	MCX_STATE* ps;
	int i;
	
	{	// ＧＵＩ用線形補間サービス
		MCX_INTERP* pi;
		for( pi = pm->pitop ; pi != NULL ; pi = pi->pNext ) {
			MCX_UpdateInterp( pi );
		}
	}
	{	// パッドオートリピート処理
		u_short status = GV_PadDataDirect[ 0 ].status;
		pm->press = GV_PadDataDirect[ 0 ].press;
		
		if( status == pm->pad_status ) {
			if( pm->repeat_interval > 0
				&& --pm->repeat_ctr == 0 ) {
				pm->press |=  status & PAD_UDLR ;
				pm->repeat_ctr = pm->repeat_interval;
			}
		} else {	// パッド状態が変化
			pm->pad_status = status;
			pm->repeat_ctr = AUTOREPEAT_INTERVAL_1ST;
		}
	}
	//	文字列表示アップデート
	MCX_StringsUpdate();
	MCX_MStringsUpdate();
	MCX_L2DMorfUpdate();

   BP_TODO_BREAK;
#if 0 //BP
	WaitSema( pMcxman->hSemaChange );

	if( pm->curstate < MCX_STATE_MAX
		&& (ps = pm->pState[ pm->curstate ]) != NULL ) {	// メイン状態のアップデート
		if( ps->Handler != NULL ) {	// イベント処理
			WaitSema( pMcxman->hSemaEvent );
			for ( i = 0 ; i < pm->nev ; i++ ) {
				ps->Handler( pm, ps, pm->evq[ i ] );
			}
			pm->nev = 0;
			pm->devchange = 0;
			SignalSema( pMcxman->hSemaEvent );
		}
		if( ps->GUI != NULL ) {	// GUI起動
			ps->GUI( pm, ps );
		}
	}
	for ( i = MCX_STATE_DEV_SELECT ; i < MCX_STATE_MAX ; i ++ ) {
		// フェードアウト状態のものだけＧＵＩ起動
		if( i != pm->curstate
			&& (ps = pMcxman->pState[ i ] ) != NULL
			&& ps->gui_state == MCX_GUI_STATE_FADEOUT
			&& ps->GUI != NULL ) {
			ps->GUI( pm, ps );
		}
	}
	SignalSema( pMcxman->hSemaChange );

   if( pm->curstate == MCX_STATE_END ) {
		// スレッド終了待ち
		if( !BP_ReferThreadStatus( pm->hThread , NULL ) ) {
			if( pm->ExitFunc != NULL ) {
				pm->ExitFunc( pm->pExParam );
			}
			GV_DestroyActor(&pm->actor);
		}
		return;
	}
#endif
	if( pm->curstate == MCX_STATE_INIT ) MCX_ChangeState( NULL, MCX_STATE_FILE_CHECK );
}

static void Die( MCX_MAN* pm )
{
	int i;

	MCX_L2DMorfExit();

	for( i = 0 ; i < MCX_STATE_MAX ; i++ ) {
		if( pm->pState[ i ] != NULL ) MCX_Free( pm->pState[ i ] );
	}
	MCX_Free( pm->pFiles );
	SPR_KillTexture( pm->font_tri_handle );
   BP_TODO_BREAK;
#if 0 //BP
	DeleteSema( pm->hSemaChange );
	DeleteSema( pm->hSemaEvent );
#endif
	if( pm->l2d_handle >= 0 ) L2D_ReleaseLayout( pm->l2d_handle );

	pMcxman = NULL;
	--GM_PadResetDisable;	// パッドリセットを有効にする
}

static void L2dSignalFunc( MCX_MAN* pm, int sign, int value )
{
	int i;
	for ( i = 0 ; i < MCX_STATE_MAX ; i++ ) {	// GUI状態リセット
		if( pm->pState[ i ] != NULL && pm->pState[ i ]->L2dHandler != NULL)
			pm->pState[ i ]->L2dHandler( pm, pm->pState[ i ], sign, value );
	}
}


void	MCX_InitManager( MCX_MAN* pm, int l2d_handle, void* pFontManager, u_int flag )
{
	int i;
	// 起動チェック
	ASSERT( pMcxman == NULL );
	pMcxman = pm;
				   
	// アクター設定
	GV_SetActorClass( &pm->actor, GV_CLASS_CHARA );
	GV_ActorEX( &pm->actor );
	GV_SetActorKillLevel( &pm->actor, GV_KILL_LEVEL_NORMAL );
	GV_InsertActorPriority( GV_ACTOR_MANAGER, &pm->actor, 0x1ff );
	GV_SetActor( &pm->actor, Act, Die );
	GV_SetActorFreeFunc( &pm->actor, NULL );	// 外で確保しているので

	// メンバ設定
	pm->flag = flag;
	
	pm->pState[ MCX_STATE_DEV_SELECT ] = NULL; //MCX_GetDefaultState_DevSelect();
	pm->pState[ MCX_STATE_FILE_CHECK ] = MCX_GetDefaultState_FileCheck();
//	pm->pState[ MCX_STATE_FILE_SELECT ] = MCX_GetDefaultState_FileSelect();
	pm->pState[ MCX_STATE_SL_VERIFY ] = NULL;
//	pm->pState[ MCX_STATE_SL ] = MCX_GetDefaultState_SaveLoad();

	pm->devchange = 0;
#ifdef MOUNT_MU
	pm->devstate = XGetDevices( XDEVICE_TYPE_MEMORY_UNIT );
#else
	pm->devstate = 0;
#endif	

	pm->curdev = DEV_HDD;

	pm->pitop = NULL;

	pm->l2d_handle = l2d_handle;

	pm->nev = 0;
	pm->nre = 0;

	pm->result = -1;

	if( flag & MCX_MAN_FLAG_PHOTO ) {
//		pm->ext = 'p';
		pm->pDSFileFmt = LFILE_NAME_PHOTO L" %02d";
		pm->maxdata = PHOTO_FILE_MAX;
		if( flag & MCX_MAN_FLAG_SAVE ) {
			pm->pState[ MCX_STATE_FILE_SELECT ] = MCX_GetFileSelectPhoto();
			pm->pState[ MCX_STATE_SL ] = MCX_GetDefaultState_SaveLoad();
		} else {
			pm->pState[ MCX_STATE_FILE_SELECT ] = MCX_GetFileSelectPhotoLoad();
			pm->pState[ MCX_STATE_SL ] = MCX_GetPhotoEditState();
		}
		pm->require_block = ( MCX_PHOTO_DATA_SIZE + BLOCK_SIZE - 1 ) / BLOCK_SIZE
				+ 3;	// dir, meta, image 各１ブロックずつ
	} else if ( flag & MCX_MAN_FLAG_GAME ) {
		pm->maxdata = DATA_FILE_MAX;
//		pm->ext = 'g';
		pm->pDSFileFmt = LFILE_NAME_GAME L" %02d";
		pm->pState[ MCX_STATE_FILE_SELECT ] = MCX_GetFileSelectGame();
		pm->pState[ MCX_STATE_SL ] = MCX_GetDefaultState_SaveLoad();
		pm->require_block = ( MCX_GAME_DATA_SIZE + BLOCK_SIZE - 1 ) / BLOCK_SIZE
				+ 3;	// dir, meta, image 各１ブロックずつ
	} else if ( flag & MCX_MAN_FLAG_VR ) {
		pm->maxdata = VR_FILE_MAX;
//		pm->ext = 'v';
		pm->pDSFileFmt = LFILE_NAME_VR;
		pm->pState[ MCX_STATE_FILE_SELECT ] = MCX_GetFileSelectGame();
		pm->pState[ MCX_STATE_SL ] = MCX_GetDefaultState_SaveLoad();
		pm->require_block = ( MCX_VR_DATA_SIZE + BLOCK_SIZE - 1 ) / BLOCK_SIZE
				+ 3;	// dir, meta, image 各１ブロックずつ
	} else if ( flag & MCX_MAN_FLAG_SNAKE_T ) {
		pm->maxdata = SNAKE_TALES_FILE_MAX;
//		pm->ext = 's';
		pm->pDSFileFmt = LFILE_NAME_SNAKE_T L" %02d";
		pm->pState[ MCX_STATE_FILE_SELECT ] = MCX_GetFileSelectGame();
		pm->pState[ MCX_STATE_SL ] = MCX_GetDefaultState_SaveLoad();
		pm->require_block = ( MCX_SNAKE_T_DATA_SIZE + BLOCK_SIZE - 1 ) / BLOCK_SIZE
				+ 3;	// dir, meta, image 各１ブロックずつ
	} else {
		HANGUP();
	}

	{	// 空きサイズを計算しておく
		ULARGE_INTEGER lFreeBytesAvailable;
		ULARGE_INTEGER lTotalNumberOfBytes;
		ULARGE_INTEGER lTotalNumberOfFreeBytes;
		
		if( !GetDiskFreeSpaceEx( "U:\\",
								 &lFreeBytesAvailable,
								 &lTotalNumberOfBytes,
								 &lTotalNumberOfFreeBytes ) ) HANGUP();
		printf("available %d\ntotal %d\nfree %d\n",
			   lFreeBytesAvailable.QuadPart / BLOCK_SIZE,
			   lTotalNumberOfFreeBytes.QuadPart / BLOCK_SIZE );
		pm->free_block = (int)(lFreeBytesAvailable.QuadPart / BLOCK_SIZE);

		// 空いているかどうかの判断
		if( pm->free_block < pm->require_block ) {
			pm->flag |= MCX_MAN_FLAG_NOSPACE; 
		}
	}

	for ( i = 0 ; i < MCX_STATE_MAX ; i++ ) {	// GUI状態リセット
		if( pm->pState[ i ] != NULL ) pm->pState[ i ]->gui_state = MCX_GUI_STATE_IDLE;
	}

	pm->pFontManager = pFontManager;

	pm->hFile = pm->hSignature = INVALID_HANDLE_VALUE;

	// Ｌ２Ｄのシグナルハンドラ設定
	if( pm->l2d_handle >= 0 ) {
		L2D_SetSignalHandle( pm->l2d_handle, pm, L2dSignalFunc );
	}

	// 使用テクスチャロード
	if( (flag & MCX_MAN_FLAG_PHOTO) && (flag & MCX_MAN_FLAG_SAVE ) ) {
		pm->font_tri_handle = SPR_LoadTexture( TRI_STRCODE_P );
	} else {
		pm->font_tri_handle = SPR_LoadTexture( TRI_STRCODE_G );
	}
	// データ列挙用メモリ確保
	pm->pFiles = (MCX_FILEPROP*)MCX_Malloc( sizeof(MCX_FILEPROP) * pm->maxdata );
	ZeroMemory( pm->pFiles, sizeof( MCX_FILEPROP ) * pm->maxdata );
	pm->nData = 0;
	
	MCX_MStringsInit( pFontManager );	// MENU系機能イニシャライズ

	pm->CreateTex = NULL;
	pm->ExitFunc = NULL;

	// パッドオートリピート
	pm->repeat_interval = 0;
	pm->repeat_ctr = AUTOREPEAT_INTERVAL_1ST;
	pm->press = pm->pad_status = 0;
	
	pm->curstate = MCX_STATE_INIT;
	// セマフォ準備
   BP_TODO_BREAK;
#if 0 //BP
	{
		struct SemaParam sema;
		ZeroMemory( &sema, sizeof( sema ) );
		sema.maxCount = 1; sema.initCount = 1;
		pm->hSemaChange = CreateSema( &sema );
		pm->hSemaEvent = CreateSema( &sema );
	}
#endif
	// スレッド起動
	pm->hThread = MTS_NewThread( "Save/Load", MCX_SaveLoadThread,
								 THREAD_PRIORITY_LOWEST, NULL, THREAD_STACKSIZE, pm );

	++GM_PadResetDisable;	// パッドリセットを無効にする
}

void MCX_SetSaveLoadFunc( BOOL (*SaveLoad)( void* pw ),
						  void* pSlParam )
{	// セーブロード関数設定
	ASSERT( pMcxman != NULL );
	pMcxman->SaveLoad = SaveLoad;
	pMcxman->pSlParam = pSlParam;
}

void MCX_SetExitFunc( void (*Exit)( void* pw ),
					  void* pExParam )
{	// セーブロード関数設定
	ASSERT( pMcxman != NULL );
	pMcxman->ExitFunc = Exit;
	pMcxman->pExParam = pExParam;
}

void	MCX_QueueEvent( int ev )
{	// イベントキューにイベントを追加
	ASSERT( pMcxman != NULL );
	if( pMcxman->nev >= MCX_EVQ_MAX ) {
		printf("xmcman:Event Buffer Full\n");
	}
   BP_TODO_BREAK;
#if 0//BP
	WaitSema( pMcxman->hSemaEvent );
	pMcxman->evq[ pMcxman->nev++ ] = ev;
	SignalSema( pMcxman->hSemaEvent );
#endif
}

void	MCX_QueueRequest( int re )
{
	ASSERT( pMcxman != NULL );
	if( pMcxman->nre >= MCX_REQ_MAX ) {
		printf("xmcman:Request Buffer Full\n");
	}
   BP_TODO_BREAK;
#if 0//BP
	WaitSema( pMcxman->hSemaEvent );		// セマフォはイベントのものと共用する
	pMcxman->req[ pMcxman->nre++ ] = re;
	SignalSema( pMcxman->hSemaEvent );
#endif
}

void MCX_ChangeState( MCX_STATE* ps, int state )
{	// 状態を変更する
	if( ps != NULL ) {
		if( ps->gui_state != MCX_GUI_STATE_FADEOUT && ps->gui_state != MCX_GUI_STATE_IDLE ) {
			ps->gui_state = MCX_GUI_STATE_SUSPEND;
		}
	}
	pMcxman->curstate = state;
	if( state < MCX_STATE_MAX &&
		pMcxman->pState[ state ] != NULL &&
		pMcxman->pState[ state ]->Enter != NULL ) {
		pMcxman->pState[ state ]->Enter( pMcxman, pMcxman->pState[ state ] );
		pMcxman->pState[ state ]->gui_state = MCX_GUI_STATE_FADEIN;
	}
	if( state == MCX_STATE_END ) {	// 完全終了判定
		int i;
		for( i = MCX_STATE_DEV_SELECT ; i < MCX_STATE_MAX ; i++ ) {
			if( (ps = pMcxman->pState[ i ]) != NULL &&
				ps->gui_state != MCX_GUI_STATE_IDLE ) {
				return;
			}
		}
		MCX_QueueRequest( MCX_REQUEST_DESTROY );
	}
}

void	MCX_ExitState( MCX_STATE* ps )
{	// 現在の状態から完全に抜ける
	//
	int i;
	ASSERT( pMcxman != NULL );
	if( ps != NULL 
		&& ps->Exit != NULL ) {
		ps->Exit( pMcxman, ps );
		ps->gui_state = MCX_GUI_STATE_IDLE;
	}
	if( pMcxman->curstate == MCX_STATE_END ) {	// 完全終了判定
		for( i = MCX_STATE_DEV_SELECT ; i < MCX_STATE_MAX ; i++ ) {
			if( (ps = pMcxman->pState[ i ]) != NULL &&
				ps->gui_state != MCX_GUI_STATE_IDLE ) {
				return;
			}
		}
		MCX_QueueRequest( MCX_REQUEST_DESTROY );
	}
}

void	MCX_ClearStart( int state )
{	// state のフェードアウトを開始する
	MCX_STATE* ps;
	ASSERT( pMcxman != NULL );
	if( (ps = pMcxman->pState[ state ] ) != NULL
		&& ps->ClearStart != NULL ) {
		ps->ClearStart( pMcxman, ps );
		ps->gui_state = MCX_GUI_STATE_FADEOUT;
	}
}

MCX_FILEPROP*	MCX_GetFileInfo( int n )
{
	ASSERT( pMcxman != NULL );
#if 0	
	return pMcxman->pFiles + *(pMcxman->pfileindex + n) ;
#else
	return pMcxman->pFiles + n;
#endif	
}

//// ＧＵＩ用線形補間サービス
void MCX_AddInterpService( MCX_INTERP* pi )
{	// 線形補間サービスに登録
	
	if( pMcxman == NULL ) return;
	pi->pNext = pMcxman->pitop;
	pMcxman->pitop = pi;
}

void MCX_DeleteInterp( MCX_INTERP* pi )
{	// 線形補間サービスからはずす
	MCX_INTERP* picur;

	if( pMcxman == NULL ) return;

	if( pMcxman->pitop == pi ) {
		pMcxman->pitop = pi->pNext;
		return;
	}
	for( picur = pMcxman->pitop ; picur != NULL ; picur = picur->pNext )
	{
		if( picur->pNext == pi ) {
			picur->pNext = pi->pNext;
			pi->pNext = NULL;
			return;
		}
	}
	// リストの中に無かった
	HANGUP();
}

static void MCX_UpdateInterp( MCX_INTERP* pi )
{
	if( pi->ctr > 0 ) {
		pi->fCurrent += ( pi->target - pi->fCurrent ) / (float) pi->ctr;
		if( --pi->ctr == 0 ) pi->fCurrent = (float)pi->target;
	}
}

//// オートリピートサービス
void MCX_SetAutoRepeat( int repeat_interval )
{
	ASSERT( pMcxman != NULL );
	pMcxman->repeat_interval = repeat_interval;
	pMcxman->repeat_ctr = AUTOREPEAT_INTERVAL_1ST;
}

//// セーブロードユーティリティ

void MCX_SetFileName( MCX_FILEPROP* pFile, int id )
{
	ASSERT( pMcxman != NULL );
	pFile->id = id;
//	sprintf( pFile->finddata.cFileName, "data%03d.%c", id, pMcxman->ext );
	wsprintfW( pFile->szDashbordName, pMcxman->pDSFileFmt, id );
}

static void ErrorClose( void )
{
   BP_TODO_BREAK;
#if 0//BP_TODO
	ASSERT( pMcxman != NULL );
	if( pMcxman->hSignature != INVALID_HANDLE_VALUE ) {
		XCalculateSignatureEnd( pMcxman->hSignature, NULL );
		pMcxman->hSignature = INVALID_HANDLE_VALUE;
	}
	if( pMcxman->hFile != INVALID_HANDLE_VALUE ) {
		CloseHandle( pMcxman->hFile );
		pMcxman->hFile = INVALID_HANDLE_VALUE;
	}
#endif
}

#include "../clearcode/clearcode.h"

static void EncodeFileName( char* pname )
{	// ファイル名の生成
	// クリアコードの関数を利用する
	CODE_BIT_DATA tmp0;
	CODE_BIT_DATA tmp1;
	int len;
	int i;
	
	memcpy( tmp0.data, &pMcxman->pCurfile->info, sizeof(MCX_FILE_INFO) );
	tmp0.bitlen = sizeof( pMcxman->pCurfile->info ) * 8;
	ClearCodeEncodeEx( &tmp1, &tmp0, GV_Time );
	ClearCodeEncodeEx( &tmp0, &tmp1, GV_Time >> 5 );
	len = ClearCodeAddRedundantBit( &tmp0, 0, FILE_NAME_ENCODE_RADIX, 1 );

	ASSERT( len <= 42 - 2 );	// FATX 最大名 42 文字 2文字拡張子分
	ClearCodeChangeRadix( pname, len, FILE_NAME_ENCODE_RADIX,
						  tmp0.data, (tmp0.bitlen + 7) / 8, 256 );
	for( i = 0 ; i < len; i++ ) {
		// 0~9	: 00-09
		// A~Z	: 10-35
		// a~z	: 36-61
		if( pname[ i ] < 10 ) {
			pname[ i ] = pname[ i ] + '0';
		} else if( pname[ i ] < 36 ) {
			pname[ i ] = pname[ i ] + 'A' - 10;
		}
#ifdef EU		
		else if( pname[ i ] < 62 ) {
			pname[ i ] = pname[ i ] + 'a' - 36;
		} else {
			pname[ i ] = '_';
		}
#else
		else {
			pname[ i ] = pname[ i ] + 'a' - 36;
		}
#endif		
	}
	strcpy( pname + i, ".d" );
}


HANDLE	MCX_Open( DWORD dwDesiredAccess )
{
	char fullname[ MAX_PATH ];
	ASSERT( pMcxman != NULL );
	ASSERT( pMcxman->hFile == INVALID_HANDLE_VALUE );
	ASSERT( pMcxman->hSignature == INVALID_HANDLE_VALUE );

   BP_TODO_BREAK;
	// 先にシグネチャ計算ハンドルを開く
#if 0 //BP_TODO
	if( (pMcxman->hSignature = XCalculateSignatureBegin( 0 ) ) == INVALID_HANDLE_VALUE ) {
		return INVALID_HANDLE_VALUE;
	}
#endif
	_snprintf( fullname, MAX_PATH, "%s\\%s", pMcxman->pCurfile->szDir, pMcxman->pCurfile->finddata.cFileName );

	if( dwDesiredAccess == GENERIC_WRITE ) {	// セーブ時
		// 最初に前のファイルを消す
		if( DeleteFile( fullname ) ) {
			printf("del\n");
		}	// (エラーは無視)
		// ファイル名生成
		{
			char filename[ 43 ];
			EncodeFileName( filename );
			_snprintf( fullname, MAX_PATH, "%s\\%s", pMcxman->pCurfile->szDir, filename );
			strcpy( pMcxman->pCurfile->finddata.cFileName, filename );
		}
		
		pMcxman->bSave = TRUE;
		if( (pMcxman->hFile = CreateFile( fullname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
										  NULL ) ) == INVALID_HANDLE_VALUE ) {
			ErrorClose();
			return INVALID_HANDLE_VALUE;
		}
		// 最初のシグネチャのサイズをあけておく
		if( SetFilePointer( pMcxman->hFile, sizeof( MCX_SIGNATURE ), NULL, FILE_BEGIN ) != sizeof( MCX_SIGNATURE ) ) {
         BP_TODO_BREAK;
#if 0//BP_TODO
			XCalculateSignatureEnd( pMcxman->hSignature, NULL );
#endif
			ErrorClose();
			return INVALID_HANDLE_VALUE;
		}
#if 0		
		// ファイル情報を書き込む
		if( MCX_Write( pMcxman->hFile, &pMcxman->pCurfile->info,
					   MCX_FILE_INFO_SIZE ) != MCX_FILE_INFO_SIZE ) {
			ErrorClose();
			return INVALID_HANDLE_VALUE;
		}
#endif		
		return pMcxman->hFile;
	} else {	// ロード時
		DWORD dwSizeRead;
		pMcxman->bSave = FALSE;
		if( pMcxman->pCurfile->flag & MCX_FILEPROP_FLAG_DAMAGED ) {
			ErrorClose();
			return INVALID_HANDLE_VALUE;
		}
		if( (pMcxman->hFile = CreateFile( fullname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
										  NULL ) ) == INVALID_HANDLE_VALUE ) {
			ErrorClose();
			return INVALID_HANDLE_VALUE;
		}
		// シグネチャを読み込む
		if( !ReadFile( pMcxman->hFile, &pMcxman->sign, sizeof(pMcxman->sign), &dwSizeRead, NULL ) ) {
			ErrorClose();
			return INVALID_HANDLE_VALUE;
		}

		// ファイルサイズの確認
		{
			DWORD dwFileSizeHigh = 0;
			if( GetFileSize( pMcxman->hFile, &dwFileSizeHigh ) != pMcxman->sign.dwFileLength
				|| dwFileSizeHigh != 0 ) {
				ErrorClose();
				return INVALID_HANDLE_VALUE;
			}
		}
#if 0
		// ファイル情報を読み込む
		if( MCX_Read( pMcxman->hFile, &pMcxman->pCurfile->info,
					  MCX_FILE_INFO_SIZE ) != MCX_FILE_INFO_SIZE ) {
			ErrorClose();
			return INVALID_HANDLE_VALUE;
		}
#endif		
		return pMcxman->hFile;
	}
}
						  
int	MCX_Write( HANDLE hFile, void* pBuf, int size )
{
	DWORD dwSizeWritten;
	
	ASSERT( pMcxman != NULL );
	ASSERT( hFile == pMcxman->hFile );
	ASSERT( INVALID_HANDLE_VALUE != pMcxman->hSignature );

   BP_TODO_BREAK;
	// シグネチャの計算
#if 0//BP_TODO
	if( XCalculateSignatureUpdate( pMcxman->hSignature, pBuf, size ) != ERROR_SUCCESS ) {
		ErrorClose();
		return -1;
	}
#endif
	
	// 実際にデータを書き込む
	if( !WriteFile( hFile, pBuf, size, &dwSizeWritten, NULL ) ) {
		ErrorClose();
		return -1;
	}
	
	return dwSizeWritten;
}

int	MCX_Read( HANDLE hFile, void* pBuf, int size )
{
	DWORD dwSizeRead;
	
	ASSERT( pMcxman != NULL );
	ASSERT( hFile == pMcxman->hFile );
	ASSERT( INVALID_HANDLE_VALUE != pMcxman->hSignature );

	// 実際にデータを読み込む
	if( !ReadFile( hFile, pBuf, size, &dwSizeRead, NULL ) ) {
		ErrorClose();
		return -1;
	}

   BP_TODO_BREAK;
	// シグネチャの計算
#if 0//BP_TODO
	if( XCalculateSignatureUpdate( pMcxman->hSignature, pBuf, size ) != ERROR_SUCCESS ) {
		ErrorClose();
		return -1;
	}
#endif
	return dwSizeRead;
}

int MCX_Close( HANDLE hFile )
{
	MCX_SIGNATURE signature;
	
	ASSERT( pMcxman != NULL );
	ASSERT( hFile == pMcxman->hFile );
	ASSERT( INVALID_HANDLE_VALUE != pMcxman->hSignature );
	
   BP_TODO_BREAK;
#if 0//BP_TODO
	// シグネチャ計算の終了
	if( XCalculateSignatureEnd( pMcxman->hSignature, &signature.Signature ) != ERROR_SUCCESS ) {
		ErrorClose();
		return -1;
	}
#endif
	pMcxman->hSignature = INVALID_HANDLE_VALUE;
	
	// サイズ計算
	if( (signature.dwFileLength = SetFilePointer( hFile, 0, NULL, FILE_CURRENT )) < 0 ) {
		ErrorClose();
		return -1;
	}

	if( pMcxman->bSave ) {
		// シグネチャ書き込み
		DWORD dwSizeWritten;
		if( SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) < 0 ) {
			ErrorClose();
			return -1;
		}
		if( !WriteFile( hFile, &signature, sizeof( signature ), &dwSizeWritten, NULL ) ) {
			ErrorClose();
			return -1;
		}
		pMcxman->pCurfile->flag &= ~MCX_FILEPROP_FLAG_DAMAGED;
	} else {
		// シグネチャ比較
		if( memcmp( &signature, &pMcxman->sign, sizeof( signature ) ) != 0 ) {
			ErrorClose();
			return -1;
		}
	}

	CloseHandle( hFile );
	pMcxman->hFile = INVALID_HANDLE_VALUE;
	
	return 0;
}


// メモリユーティリティ
void* MCX_Malloc( int size )
{
	ASSERT( pMcxman != NULL );

	if( pMcxman->flag & MCX_MAN_FLAG_CODECMEM ) {
		return codecMalloc( size );
	} else {
		return GV_Malloc( size );
	}
}

void MCX_Free( void* ptr )
{
	ASSERT( pMcxman != NULL );

	if( pMcxman->flag & MCX_MAN_FLAG_CODECMEM ) {
		codecDelayedFree( ptr );
	} else {
		GV_DelayedFree( ptr );
	}
}


// ロードＩＤ管理
static short loaddata_id[ MCMAN_FILE_KIND_MAX ];

void MCX_ResetLoaddataID( void )
{
	int i;
	for( i = 0 ; i < MCMAN_FILE_KIND_MAX ; i++ ) {
		loaddata_id[ i ] = -1;
	}
}

void MCX_SetLoaddataID( u_int type, int id )
{
	ASSERT( type < MCMAN_FILE_KIND_MAX );
	loaddata_id[ type ] = id;
}

short MCX_GetLoaddataID( u_int type )
{
	return loaddata_id[ type ];
}


