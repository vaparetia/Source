/*
  マネージャ別スレッド部分
  2002/04/10 M.Kobayashi
  $Id: slthread.c,v 1.8 2002/12/26 11:04:36 Yoshizawa1 Exp $

 */

#include	"gameheader.h"
#include	"mts.h"
#include	"xmcman.h"
#include 	"private.h"

#if 0//BP_TODO

#include "bp_threading.h"

#ifdef MOUNT_MU	
static char chMUDrives[NUM_PORTS][NUM_SLOTS];
#endif

#ifdef KP_WINDOWS

static char save_root_path[MAX_PATH];
static char root_path[MAX_PATH];
static void GetSaveDirectry( void )
{
	WIN32_FIND_DATA wfd;
	int len;
	HANDLE fh;
	if( (len = GetModuleFileName(NULL, root_path, MAX_PATH)) > 0 ){
		char	*ptr ;
	
		len-- ;
		ptr = root_path + len ;
		for(; len>0; len--, ptr--)
		{
			if( *ptr == '\\' )	// 最後端の"\"
			{
				*ptr = 0 ;	// パスの終端
				break ;
			}
		}
		ptr--;
		len-- ;
		for(; len>0; len--, ptr--)
		{
			if( *ptr == '\\' )	// 最後端の"\"
			{
				*ptr = 0 ;	// パスの終端
				break ;
			}
		}

		sprintf(save_root_path, "%s\\savedata", root_path) ;
								// Data Root Path(仮)
	}
	
	
	if((fh = FindFirstFile( save_root_path , &wfd )) == INVALID_HANDLE_VALUE ){
		SECURITY_ATTRIBUTES SecurityAttributes;
		
		SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES); 
		SecurityAttributes.lpSecurityDescriptor = NULL; 
		SecurityAttributes.bInheritHandle = 0; 
		
		if(!CreateDirectory(save_root_path,&SecurityAttributes )){
			
		}
	}else{
		FindClose(fh);
	}
	sprintf(save_root_path, "%s\\savedata\\", root_path) ;
}  





static int XCreateSaveGame( char *root , WCHAR*	path,int flag, int flag2 , char *chCDir, int size) {

	WIN32_FIND_DATA wfd;
	char	fname[MAX_PATH];
	char	fname2[MAX_PATH];
	HANDLE fh;
	
//@	wcstombs( fname2, path, MAX_PATH );
	strcpy(fname2,path);
	sprintf(fname,"%s%s",save_root_path,fname2);
	
	if((fh = FindFirstFile( fname , &wfd )) == INVALID_HANDLE_VALUE ){
		SECURITY_ATTRIBUTES SecurityAttributes;
	
		SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES); 
		SecurityAttributes.lpSecurityDescriptor = NULL; 
		SecurityAttributes.bInheritHandle = 0; 
	
		if(!CreateDirectory(fname,&SecurityAttributes )){
			return(ERROR_SUCCESS+1);
		}
	}else{
		FindClose(fh);
	}
	sprintf(chCDir,"%s%s\\",save_root_path,fname2);
//	strncpy(chCDir, fname, MAX_PATH );
	return(ERROR_SUCCESS);
}
#endif


static int	DecodeFileName( MCX_FILE_INFO* pinfo, const char* filename );
void MCX_SaveLoadThread( int id, MCX_MAN* pm )
{
   BP_TODO_BREAK;
#if 0//BP
//#ifdef KP_WINDOWS

	while( 1 ) {
		// メインスレッドからのリクエストチェック
		int nre;
		int req;
		Sleep( 100 );	// 0.1s 間隔で動く
		
		WaitSema( pm->hSemaEvent );
		nre = pm->nre;
		SignalSema( pm->hSemaEvent );
		if( nre != 0 ) {
			// リクエスト有り
			int i;
			WaitSema( pm->hSemaEvent );
			// キューから取り出して整合性をとる
			req = pm->req[0];
			for ( i = 1 ; i < nre ; i++ ) {
				pm->req[ i - 1 ] = pm->req[ i ];
			}
			--pm->nre;
			SignalSema( pm->hSemaEvent );

			// リクエスト処理
			switch( req ) {
			case MCX_REQUEST_FIND_GAME:
			{	// セーブゲームを見つける
				// TODO:セーブゲームが作れないときの処理
				// ロード時：「データがありません」
				// セーブ時：エラーメッセージ？
				
				char data_path[MAX_PATH];
				HANDLE	hFindGame;
				WIN32_FIND_DATA xfd;
				int		nSaveGame = 0;

				printf("req find game\n");
				for( i = 0 ; i < pm->maxdata ; i++ ) {
					(pm->pFiles + i)->id = -1; // 空いてるよ印
				}
				GetSaveDirectry();
				sprintf(data_path,"%s*.*",save_root_path);
				hFindGame = FindFirstFile(data_path , &xfd );
				while( hFindGame != INVALID_HANDLE_VALUE ) {
					// ファイル判定
					int nFileNameLen;
					int type = -1;
					int id = -1;

					++nSaveGame;
					
					
#if DEBUG_MODE
					printf("%s%s == %s \n",save_root_path,xfd.cFileName,FILE_NAME_GAME);
#endif
					if( strncmp( xfd.cFileName, FILE_NAME_GAME,
								 nFileNameLen = strlen( FILE_NAME_GAME ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_GAME ) {
							type = MCMAN_FILE_KIND_GAME;
						}
					} else if ( strncmp( xfd.cFileName, FILE_NAME_PHOTO,
										 nFileNameLen = strlen( FILE_NAME_PHOTO ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_PHOTO ) {
							type = MCMAN_FILE_KIND_PHOTO;
						}
					} else if ( strncmp( xfd.cFileName, FILE_NAME_VR,
										  nFileNameLen = strlen( FILE_NAME_VR ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_VR ) {
							type = MCMAN_FILE_KIND_VR;
						}
					} else if ( strncmp( xfd.cFileName, FILE_NAME_SNAKE_T,
										 nFileNameLen = strlen( FILE_NAME_SNAKE_T ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
							type = MCMAN_FILE_KIND_SNAKE_TALES;
						}
					}
					if( type != -1 ) {
						// id 取得
						if( nFileNameLen + 3 == strlen( xfd.cFileName ) ) {
							id = (xfd.cFileName[ nFileNameLen + 1 ] - '0') * 10	+ (xfd.cFileName[ nFileNameLen + 2 ] - '0');
						} else if( (nFileNameLen == strlen( xfd.cFileName )) &&
//@02/12/26(木)@								   (nFileNameLen == lstrlenW( pm->pDSFileFmt)) 
								   (nFileNameLen == strlen( pm->pDSFileFmt)) 
								   ) {
							id = 0;	// １つしかファイルがない（ＶＲ）
						}
					}
					if( id != -1 ) {
						char buf[MAX_PATH];
						HANDLE	hFindFile;
//						HANDLE	hFile;
						WIN32_FIND_DATA	wfd;
						MCX_FILE_INFO	info;
						MCX_FILEPROP* pFile;

						// id が取得できたので、この時点でファイルブラウザに表示できるのは確定
						// ただしデフォルトでは「壊れたファイル」としておく
						
						if( pm->flag & MCX_MAN_FLAG_FIRSTCHECK ) {
							// ソートしないので順番に入れる
							pFile = pm->pFiles + pm->nData;
						} else {
							// id の箇所に入れる（後でつめる）
							pFile = pm->pFiles + id;
						}
						memcpy( pFile->szDir, xfd.cFileName, MCX_MAX_PATH );

//@						MultiByteToWideChar( CP_ACP, 0, xfd.cFileName, -1,
//@											 pFile->szDashbordName, MCX_MAX_GAMENAME * sizeof( WCHAR ) );

						memcpy( pFile->szDashbordName, xfd.cFileName,
								MCX_MAX_GAMENAME * sizeof( WCHAR ) );
						pFile->id = id;
						++pm->nData;
						pFile->flag = MCX_FILEPROP_FLAG_DAMAGED;
						ZeroMemory( &pFile->info, sizeof( MCX_FILE_INFO ) );
						ZeroMemory( &pFile->finddata, sizeof( WIN32_FIND_DATA ) );
						
						sprintf(buf,"%s%s\\*.d",save_root_path,xfd.cFileName);
						hFindFile = FindFirstFile( buf, &wfd);

#if DEBUG_MODE
						printf("%s \n",buf);
#endif

						if( hFindFile != INVALID_HANDLE_VALUE ) {
							pFile->finddata = wfd;

#if DEBUG_MODE
							printf("%s%s \n",buf,wfd.cFileName);
#endif

							if( DecodeFileName( &info, wfd.cFileName ) ){
								pFile->info = info;
								pFile->flag &= ~MCX_FILEPROP_FLAG_DAMAGED;
								printf("%s\n", xfd.cFileName );
							}
							FindClose( hFindFile ) ;
						}
						
					}
					
					// 次のセーブゲームを探す
					if( !FindNextFile( hFindGame, &xfd ) || pm->nData >= pm->maxdata ) {
						FindClose( hFindGame );
						break;
					}
				}
				printf("findend0\n");
				if( nSaveGame == 4096 ) pm->flag |= MCX_MAN_FLAG_MANYDIRS;	// 一応 4096 チェック
				
				if( !(pm->flag & MCX_MAN_FLAG_FIRSTCHECK) ){	// データを詰める
					int dst = 0, src = 0;
					ULARGE_INTEGER	latesttime;
					latesttime.QuadPart = 0;
					pm->latestfile = 0;
					
					for( dst = 0 ; dst < pm->nData; ++dst ) {
						if( pm->pFiles[dst].id >= 0 ) {
							++src;
							continue;
						}
						while( pm->pFiles[src].id < 0 ) src++;
						pm->pFiles[dst] = pm->pFiles[src];
						pm->pFiles[src].id = -1;
						++src;
					}
					for( i = 0 ; i < pm->nData; i++ ) {	
						// 最新ファイルチェック
						ULARGE_INTEGER ulLastWriteTime;
						if( pm->pFiles[i].flag & MCX_FILEPROP_FLAG_DAMAGED ) continue;
						memcpy( &ulLastWriteTime, &pm->pFiles[i].finddata.ftLastWriteTime, sizeof( ULARGE_INTEGER ) );
						if( ulLastWriteTime.QuadPart > latesttime.QuadPart ) {
							pm->latestfile = i;
							latesttime.QuadPart = ulLastWriteTime.QuadPart;
						}
					}
				}
				MCX_QueueEvent( MCX_EVENT_FIND_SUCCESS );
				printf("findend1\n" );
				break;
			}
			case MCX_REQUEST_SAVE_GAME:
			{	// ゲームデータ作成
				if( ERROR_SUCCESS != XCreateSaveGame( "U:\\", pm->pCurfile->szDashbordName, OPEN_ALWAYS, 0,
													  pm->pCurfile->szDir, MCX_MAX_PATH ) ) {
					MCX_QueueEvent( MCX_EVENT_SL_ERROR );
				}
				if( pm->CreateTex != NULL ) {
					pm->CreateTex( pm->pSlParam );
				}
				if( pm->SaveLoad != NULL && !pm->SaveLoad( pm->pSlParam ) ) {
					MCX_QueueEvent( MCX_EVENT_SL_ERROR );
#if DEBUG_MODE
					printf("save ERROR %s \n",pm->pCurfile->szDir);
#endif
				} else {
#if DEBUG_MODE
					printf("save SUCCESS %s \n",pm->pCurfile->szDir);
#endif
					MCX_QueueEvent( MCX_EVENT_SL_SUCCESS );
				}
				break;
			}
			case MCX_REQUEST_DESTROY:
			{	// スレッドを終了する
				printf("req end\n");
				return;	// スレッド終了return
			}
			}
		} 
	}
#else		以降 X-BOX

#ifdef MOUNT_MU	
	ZeroMemory( &chMUDrives[ 0 ][ 0 ], sizeof( chMUDrives ));
	{	// 最初に必要なものをマウントする
		int iPort, iSlot;
		for( iPort = 0; iPort < NUM_PORTS; iPort++ ) {
			for( iSlot = 0; iSlot < NUM_SLOTS; iSlot++ ) {
				DWORD dwMask;
				// ポート・スロットからマスク取得
				dwMask = iPort + (iSlot ? 16 : 0);
				dwMask = 1 << dwMask;
				// 挿されたデバイスをマウント
				if( dwMask & pm->devstate ) {
					XMountMU( iPort, iSlot, chMUDrives[iPort] + iSlot );
				}
			}
		}
	}
#endif	
	while( 1 ) {
		// メインスレッドからのリクエストチェック
		int nre;
		int req;

		Sleep( 100 );	// 0.1s 間隔で動く
		
		WaitSema( pm->hSemaEvent );
		nre = pm->nre;
		SignalSema( pm->hSemaEvent );
		if( nre != 0 ) {
			// リクエスト有り
			int i;
			WaitSema( pm->hSemaEvent );
			// キューから取り出して整合性をとる
			req = pm->req[0];
			for ( i = 1 ; i < nre ; i++ ) {
				pm->req[ i - 1 ] = pm->req[ i ];
			}
			--pm->nre;
			SignalSema( pm->hSemaEvent );

			// リクエスト処理
			switch( req ) {
			case MCX_REQUEST_FIND_GAME:
			{	// セーブゲームを見つける
				
				// セーブゲームが作れないときの処理
				// ロード時：「データがありません」
				// セーブ時：エラーメッセージ？

				HANDLE	hFindGame;
				XGAME_FIND_DATA xfd;
				int		nSaveGame = 0;
				
				printf("req find game\n");
				for( i = 0 ; i < pm->maxdata ; i++ ) {
					(pm->pFiles + i)->id = -1; // 空いてるよ印
				}
				hFindGame = XFindFirstSaveGame( "U:\\", &xfd );
				while( hFindGame != INVALID_HANDLE_VALUE ) {
					// ファイルを探してヘッダ以外の先頭 MCX_FILE_INFO_SIZE バイトを読み込み
					// （署名計算しないといけない？）
#if 0					
					char buf[MAX_PATH];
					HANDLE	hFindFile;
					HANDLE	hFile;
					WIN32_FIND_DATA	wfd;
					MCX_FILE_INFO	info;

					++nSaveGame;

					_snprintf( buf, MAX_PATH, "%sdata*.%c", xfd.szSaveGameDirectory, pm->ext );
					
					hFindFile = FindFirstFile( buf, &wfd);
					while( hFindFile != INVALID_HANDLE_VALUE ) {
						// ファイルの先頭を読み込む
						char fullpath[ MAX_PATH ];
						_snprintf( fullpath, MAX_PATH, "%s\\%s", xfd.szSaveGameDirectory, wfd.cFileName );
						if( INVALID_HANDLE_VALUE ==
							( hFile = CreateFile( fullpath,
												  GENERIC_READ, 0, NULL, OPEN_EXISTING,
												  FILE_ATTRIBUTE_NORMAL, NULL ) ) ) {
							// ファイル読み込み失敗:ゴミファイルなので削除するべき？
							// 普通ここを通ることはないが。。。
							printf("Find Game Data: Can't Open Data" );
						} else {
							DWORD dwSizeRead;
							// ファイルの最初のほうの名前を読む
							if( INVALID_SET_FILE_POINTER
								== SetFilePointer( hFile, sizeof( MCX_SIGNATURE ), NULL, FILE_BEGIN ) ) {
								printf("Find Game Data: Can't SetPos" );
							} else if ( !ReadFile( hFile, &info,
												   MCX_FILE_INFO_SIZE, &dwSizeRead, NULL ) ) {
								printf("Find Game Data: Can't Read" );
							} else {
								// 読み取り成功
													
								int id = ((wfd.cFileName[ 4 ] - '0' ) * 100
										  + (wfd.cFileName[ 5 ] - '0' ) * 10
										  + (wfd.cFileName[ 6 ] - '0' ) ) % pm->maxdata;
								MCX_FILEPROP* pFile;
								if( pm->flag & MCX_MAN_FLAG_FIRSTCHECK ) {
									// ソートしないので順番に入れる
									pFile = pm->pFiles + pm->nData;
								} else {
									// id の箇所に入れる（後でつめる）
									pFile = pm->pFiles + id;
								}
								pFile->finddata = wfd;
								memcpy( pFile->szDir, xfd.szSaveGameDirectory, MCX_MAX_PATH );
								memcpy( pFile->szDashbordName, xfd.szSaveGameName,
										MCX_MAX_GAMENAME * sizeof( WCHAR ) );
								pFile->info = info;
								pFile->id = id;

								// 仮判定
								if( wfd.cFileName[ 8 ] == 'g' ) pFile->type = MCMAN_FILE_KIND_GAME;
								else if ( wfd.cFileName[ 8 ] == 'p' ) pFile->type = MCMAN_FILE_KIND_PHOTO;
								else if ( wfd.cFileName[ 8 ] == 'v' ) pFile->type = MCMAN_FILE_KIND_VR;
								else if ( wfd.cFileName[ 8 ] == 's' ) pFile->type = MCMAN_FILE_KIND_SNAKE_TALES;
								else pFile->type = -1;
								
								pm->nData++;
								printf("%s\n", fullpath );
							}
							CloseHandle( hFile );
						}
						// 次のファイルを探す
						if( FindNextFile( hFindFile, &wfd ) == 0 || pm->nData >= pm->maxdata ) {
							FindClose( hFindFile ) ;
							break;
						}
					}
#else	// １セーブゲーム１ファイルとする

					// ファイル判定
					int nFileNameLen;
					int type = -1;
					int id = -1;
					char name[MAX_PATH];

					++nSaveGame;
					
//					WideCharToMultiByte( CP_ACP, 0, xfd.szSaveGameName, -1,
//										 name, MAX_PATH, NULL, NULL );
//
					strcpy(xfd.szSaveGameName,name);
					
					if( strncmp( name, FILE_NAME_GAME,
								 nFileNameLen = strlen( FILE_NAME_GAME ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_GAME ) {
							type = MCMAN_FILE_KIND_GAME;
						}
					} else if ( strncmp( name, FILE_NAME_PHOTO,
										 nFileNameLen = strlen( FILE_NAME_PHOTO ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_PHOTO ) {
							type = MCMAN_FILE_KIND_PHOTO;
						}
					} else if ( strncmp( name, FILE_NAME_VR,
										  nFileNameLen = strlen( FILE_NAME_VR ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_VR ) {
							type = MCMAN_FILE_KIND_VR;
						}
					} else if ( strncmp( name, FILE_NAME_SNAKE_T,
										 nFileNameLen = strlen( FILE_NAME_SNAKE_T ) ) == 0 ) {
						if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
							type = MCMAN_FILE_KIND_SNAKE_TALES;
						}
					}
					if( type != -1 ) {
						// id 取得
						if( nFileNameLen + 3 == strlen( name ) ) {
							id = (name[ nFileNameLen + 1 ] - '0') * 10	+ (name[ nFileNameLen + 2 ] - '0');
						} else if( (nFileNameLen == strlen( name )) &&
								   (nFileNameLen == lstrlenW( pm->pDSFileFmt)) ) {
							id = 0;	// １つしかファイルがない（ＶＲ）
						}
					}
					if( id != -1 ) {
						char buf[MAX_PATH];
						HANDLE	hFindFile;
//						HANDLE	hFile;
						WIN32_FIND_DATA	wfd;
						MCX_FILE_INFO	info;
						MCX_FILEPROP* pFile;

						// id が取得できたので、この時点でファイルブラウザに表示できるのは確定
						// ただしデフォルトでは「壊れたファイル」としておく
						
						if( pm->flag & MCX_MAN_FLAG_FIRSTCHECK ) {
							// ソートしないので順番に入れる
							pFile = pm->pFiles + pm->nData;
						} else {
							// id の箇所に入れる（後でつめる）
							pFile = pm->pFiles + id;
						}
						memcpy( pFile->szDir, xfd.szSaveGameDirectory, MCX_MAX_PATH );
						memcpy( pFile->szDashbordName, xfd.szSaveGameName,
								MCX_MAX_GAMENAME * sizeof( WCHAR ) );
						pFile->id = id;
						++pm->nData;
						pFile->flag = MCX_FILEPROP_FLAG_DAMAGED;
						ZeroMemory( &pFile->info, sizeof( MCX_FILE_INFO ) );
						ZeroMemory( &pFile->finddata, sizeof( WIN32_FIND_DATA ) );
						
						_snprintf( buf, MAX_PATH, "%s*.d", xfd.szSaveGameDirectory );
						hFindFile = FindFirstFile( buf, &wfd);
						if( hFindFile != INVALID_HANDLE_VALUE ) {
#if 0								
							// ファイルの先頭を読み込む
							char fullpath[ MAX_PATH ];
							pFile->finddata = wfd;
							_snprintf( fullpath, MAX_PATH, "%s\\%s", xfd.szSaveGameDirectory, wfd.cFileName );
							if( INVALID_HANDLE_VALUE ==
								( hFile = CreateFile( fullpath,
													  GENERIC_READ, 0, NULL, OPEN_EXISTING,
													  FILE_ATTRIBUTE_NORMAL, NULL ) ) ) {
								// ファイル読み込み失敗:ゴミファイルなので削除するべき？
								// 普通ここを通ることはないが。。。
								printf("Find Game Data: Can't Open Data" );
							} else {
								DWORD dwSizeRead;
								// ファイルの最初のほうの名前を読む
								if( INVALID_SET_FILE_POINTER
									== SetFilePointer( hFile, sizeof( MCX_SIGNATURE ), NULL, FILE_BEGIN ) ) {
									printf("Find Game Data: Can't SetPos" );
								} else if ( !ReadFile( hFile, &info,
													   MCX_FILE_INFO_SIZE, &dwSizeRead, NULL ) ) {
									printf("Find Game Data: Can't Read" );
								} else {
									// 読み取り成功
									pFile->info = info;
									pFile->flag &= ~MCX_FILEPROP_FLAG_DAMAGED;
									printf("%s\n", fullpath );
								}
								CloseHandle( hFile );
							}
#else
							pFile->finddata = wfd;
							if( DecodeFileName( &info, wfd.cFileName ) ){
								pFile->info = info;
								pFile->flag &= ~MCX_FILEPROP_FLAG_DAMAGED;
								printf("%s\n", xfd.szSaveGameDirectory );
							}
#endif								
							FindClose( hFindFile ) ;
						}
						
					}
					
#endif					
					// 次のセーブゲームを探す
					if( !XFindNextSaveGame( hFindGame, &xfd ) || pm->nData >= pm->maxdata ) {
						CloseHandle( hFindGame );
						break;
					}
				}

				if( nSaveGame == 4096 ) pm->flag |= MCX_MAN_FLAG_MANYDIRS;	// 一応 4096 チェック
				
				if( !(pm->flag & MCX_MAN_FLAG_FIRSTCHECK) ){	// データを詰める
					int dst = 0, src = 0;
					ULARGE_INTEGER	latesttime;
					latesttime.QuadPart = 0;
					pm->latestfile = 0;
					
					for( dst = 0 ; dst < pm->nData; ++dst ) {
						if( pm->pFiles[dst].id >= 0 ) {
							++src;
							continue;
						}
						while( pm->pFiles[src].id < 0 ) src++;
						pm->pFiles[dst] = pm->pFiles[src];
						pm->pFiles[src].id = -1;
						++src;
					}
					for( i = 0 ; i < pm->nData; i++ ) {	
						// 最新ファイルチェック
						ULARGE_INTEGER ulLastWriteTime;
						if( pm->pFiles[i].flag & MCX_FILEPROP_FLAG_DAMAGED ) continue;
						memcpy( &ulLastWriteTime, &pm->pFiles[i].finddata.ftLastWriteTime, sizeof( ULARGE_INTEGER ) );
						if( ulLastWriteTime.QuadPart > latesttime.QuadPart ) {
							pm->latestfile = i;
							latesttime.QuadPart = ulLastWriteTime.QuadPart;
						}
					}
				}
				MCX_QueueEvent( MCX_EVENT_FIND_SUCCESS );

				break;
			}
			case MCX_REQUEST_SAVE_GAME:
			{	// ゲームデータ作成
				if( ERROR_SUCCESS != XCreateSaveGame( "U:\\", pm->pCurfile->szDashbordName, OPEN_ALWAYS, 0,
													  pm->pCurfile->szDir, MCX_MAX_PATH ) ) {
					MCX_QueueEvent( MCX_EVENT_SL_ERROR );
				}
				
				if( pm->CreateTex != NULL ) {
					pm->CreateTex( pm->pSlParam );
				}
				if( pm->SaveLoad != NULL && !pm->SaveLoad( pm->pSlParam ) ) {
					MCX_QueueEvent( MCX_EVENT_SL_ERROR );
				} else {
					MCX_QueueEvent( MCX_EVENT_SL_SUCCESS );
				}
				break;
			}
			case MCX_REQUEST_DESTROY:
			{	// スレッドを終了する
#ifdef MOUNT_MU				
				// マウントしたドライブをアンマウント
				int iPort, iSlot;
				for( iPort = 0; iPort < NUM_PORTS; iPort++ ) {
					for( iSlot = 0; iSlot < NUM_SLOTS; iSlot++ ) {
						DWORD dwMask;
						// ポート・スロットからマスク取得
						dwMask = iPort + (iSlot ? 16 : 0);
						dwMask = 1 << dwMask;
						if( dwMask & pm->devstate ) {
							XUnmountMU( iPort, iSlot );
						}
					}
				}
#endif				
				printf("req end\n");
				return;	// スレッド終了return
			}
			}
		} else {
#ifdef MOUNT_MU	
			// リクエストが無い
			// ポートの監視
			DWORD	dwInsertions;
			DWORD	dwRemovals;
			int	iPort, iSlot;
			
			XGetDeviceChanges( XDEVICE_TYPE_MEMORY_UNIT, &dwInsertions, &dwRemovals );
			for( iPort = 0; iPort < NUM_PORTS; iPort++ ) {
				for( iSlot = 0; iSlot < NUM_SLOTS; iSlot++ ) {
					DWORD dwMask;
					// ポート・スロットからマスク取得
					dwMask = iPort + (iSlot ? 16 : 0);
					dwMask = 1 << dwMask;
					// 抜かれたデバイスを閉じる
					if( (dwMask & dwRemovals) && chMUDrives[iPort][iSlot] ) {
						XUnmountMU( iPort, iSlot );
						chMUDrives[iPort][iSlot] = 0;
					}
					// 挿されたデバイスをマウント
					if( dwMask & dwInsertions ) {
						XMountMU( iPort, iSlot, chMUDrives[iPort] + iSlot );
					}
				}
			}
			// ここからマネージャ状態変更
			WaitSema( pm->hSemaChange );
			pm->devchange |= dwInsertions | dwRemovals;
			pm->devstate |= dwInsertions;
			pm->devstate &= ~dwRemovals;

			// 状態が変わっているときはイベント
			if ( dwInsertions | dwRemovals ) {
				printf("mcx_status %x\n", pm->devstate);
				MCX_QueueEvent( MCX_EVENT_MC_STATUS );
			}
		
			// カレントデバイスが抜かれたときはイベント
			if ( pm->curdev != DEV_HDD
				 && ( ( GetDeviceMask( pm->curdev ) & pm->devstate ) == 0 ) ) {
				pm->curdev = DEV_HDD;
				printf("mcx_null\n");
				MCX_QueueEvent( MCX_EVENT_MC_NULL );
			}
			SignalSema( pm->hSemaChange );
#endif			
		}
	}
#endif	// KP_WINDOWS
}

#include "../clearcode/clearcode.h"

static int	DecodeFileName( MCX_FILE_INFO* pinfo, const char* pname )
{
	// ファイル名から MCX_FILE_INFO を生成
	// クリアコードの関数を利用する
	CODE_BIT_DATA tmp0;
	CODE_BIT_DATA tmp1;
	int len;
	int i;
	char buf[ 42 ] ;

	ZeroMemory( buf, sizeof( buf ) );
	len = strlen( pname ) - 2;	// ".d" を抜く
	if( len > 42 - 2 ) return FALSE;
	
	memcpy( buf, pname, len );
	for(i = 0 ; i < len ; i++ ) {
		// 0~9	: 00-09
		// A~Z	: 10-35
		// a~z	: 36-61
		// _	: 62  ( Europe でエンコード方法を変えるときのみ )
		if( '0' <= buf[ i ] && buf[ i ] <= '9' ) {
			buf[ i ] -= '0';
		} else if( 'A' <= buf[ i ] && buf[ i ] <= 'Z' ) {
			buf[ i ] = buf[ i ] - 'A' + 10;
		} else if( 'a' <= buf[ i ] && buf[ i ] <= 'z' ) {
			buf[ i ] = buf[ i ] - 'a' + 36;
		}
#ifdef EU
		else if( buf[ i ] == '_' ) {
			buf[ i ] = 62;
		}
#endif		
		else {
			buf[ i ] = 0;
		}
	}

	// 基数変換
	ClearCodeChangeRadix( tmp0.data, sizeof( tmp0.data) , 256,
						  buf, len, FILE_NAME_ENCODE_RADIX );
	tmp0.bitlen = sizeof( MCX_FILE_INFO) * 8 + CODE_ADD_LEN * 2;
	if( !ClearCodeDecodeEx( &tmp1, &tmp0 ) ) return FALSE;
	if( !ClearCodeDecodeEx( &tmp0, &tmp1 ) ) return FALSE;

	memcpy( pinfo, tmp0.data, sizeof( MCX_FILE_INFO ) );
	return TRUE;
}

#endif
