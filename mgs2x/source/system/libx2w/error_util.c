/*--------------------------------------------------------------*/
/*	error_util.c												*/
/*					Error Utility								*/
/*--------------------------------------------------------------*/
#define	__ERROR_UTIL_C__

#include <stdio.h>
#include <stdlib.h>

#include <xtl.h>
#include <dxerr8.h>

#include "libdg.h"
#include "libfs.h"

/*--------------------------------------------------------------*/
/*	typedef														*/
/*--------------------------------------------------------------*/
#define	ERROR_LOG_BUFF_SIZE			(0x1000)
#define	ERROR_LOG_BUFF_WATERLINE	(ERROR_LOG_BUFF_SIZE - 0x100)	// 危険なサイズ

#define	ERROR_LOG_FILENAME_SIZE		(MAX_PATH+1)

typedef	struct	ERROR_LOG_MAN_
{
	BOOL	create_file ;

	char	*ptr ;
	char	buff[ERROR_LOG_BUFF_SIZE] ;

	char	*waterline ;

	char	filename[ERROR_LOG_FILENAME_SIZE] ;

	BOOL	err ;
} ERROR_LOG_MAN ;

/*--------------------------------------------------------------*/
/*	static														*/
/*--------------------------------------------------------------*/
static	ERROR_LOG_MAN	_log ;

/*--------------------------------------------------------------*/
/*	ローカル関数プロトタイプ宣言								*/
/*--------------------------------------------------------------*/
static	void	ErrorLogSetStatus(BOOL is_err) ;

/*--------------------------------------------------------------*/
/*	X2W_ErrorPrintf												*/
/*--------------------------------------------------------------*/
int	X2W_ErrorPrintf(DWORD type, char *fmt, ...)
{
	va_list	ap ;
	char	buff[1024] ;

	/*-- 文字列展開 --------------------------------------------*/

	va_start( ap, fmt ) ;
	vsprintf(buff, fmt, ap) ;
	va_end( ap ) ;
	/*----------------------------------------------------------*/

	/*-- 出力 --------------------------------------------------*/

	return( X2W_ErrorPuts(type, buff) ) ;
	/*----------------------------------------------------------*/
}


/*--------------------------------------------------------------*/
/*	X2W_ErrorPuts												*/
/*--------------------------------------------------------------*/
int	X2W_ErrorPuts(DWORD type, char *str)
{
	int	ret ;

	ErrorLogPrintf("%s\n", str) ;

	X2W_ErrorDisplayStatus = TRUE ;	// 表示中フラグOn

	DG_ShowWindowCursorForce(TRUE) ;

	ret = MessageBoxEx(DG_hWnd,							// オーナーウィンドウのハンドル
					  str,  							// メッセージボックス内のテキスト
					  "METAL GEAR SOLID 2:SUBSTANCE",	// メッセージボックスのタイトル
					  type,								// メッセージボックスのスタイル
					   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)) ;
														// 言語識別子

	X2W_ErrorDisplayStatus = FALSE ;	// 表示中フラグOn

	return( ret ) ;
}

/*--------------------------------------------------------------*/
/*	X2W_ErrorPrintfID	エラー文字列付き						*/
/*--------------------------------------------------------------*/
int	X2W_ErrorPrintfID(DWORD type, int err_id, ...)
{
	va_list	ap ;
	char	str[1024] ;
	char	buff[1024] ;

	ErrorLogPrintf("X2W_ErrorPrintfID:ID %d\n", err_id) ;	// Log

	X2W_GetErrorIDMess(str, sizeof(str), err_id) ;

	/*-- 文字列展開 --------------------------------------------*/

	va_start( ap, err_id ) ;
	vsprintf(buff, str, ap) ;
	va_end( ap ) ;
	/*----------------------------------------------------------*/

	return( X2W_ErrorPuts(type, buff) ) ;
}

/*--------------------------------------------------------------*/
/*	X2W_ErrorPutsID		エラー文字列付き						*/
/*--------------------------------------------------------------*/
int	X2W_ErrorPutsID(DWORD type, int err_id)
{
	char	str[1024] ;

	ErrorLogPrintf("X2W_ErrorPutsID:ID %d\n", err_id) ;	// Log

	X2W_GetErrorIDMess(str, sizeof(str), err_id) ;

	return( X2W_ErrorPuts(type, str) ) ;
}


/*--------------------------------------------------------------*/
/*	X2W_ErrorGetMess	エラー文字列取得						*/
/*--------------------------------------------------------------*/
char	*X2W_ErrorGetMess(char *str, DWORD size, HRESULT hr)
{
	char	*err_str ;
	DWORD	len ;
	BOOL	err_str_alloc ;

	/*-- エラー文字列取得 --------------------------------------*/

	err_str_alloc = TRUE ;
	len = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
				| FORMAT_MESSAGE_FROM_SYSTEM
				| FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語
				(LPTSTR)&err_str,
				0,
				NULL ) ;
	if( !len )
	{
		err_str = (char *)DXGetErrorString8(hr) ;
		err_str_alloc = FALSE ;

		len = strlen(err_str) ;
	}
	/*--------------------------------------------------------------*/

	/*-- 文字列複写 ------------------------------------------------*/

	if( len > (size-1) ){ len = (size-1) ; }
	memcpy(str, err_str, size) ;
	/*--------------------------------------------------------------*/

	/*-- エラー文字列解放 ------------------------------------------*/

	if( err_str_alloc ){ LocalFree(err_str) ; }
	/*--------------------------------------------------------------*/

	return(str) ;
}

/*--------------------------------------------------------------*/
/*	X2W_ErrorGetLastMess	最新エラー文字列取得				*/
/*--------------------------------------------------------------*/
char	*X2W_ErrorGetLastMess(char *str, DWORD size)
{
	return(str) ;
}

/*--------------------------------------------------------------*/
/*	X2W_ErrorAbort		異常終了手続き(内部で終了)				*/
/*--------------------------------------------------------------*/
extern BOOL	RestoreWindowsPowerOffSetting(void) ;	// mainw.c

void	X2W_ErrorAbort(void)
{
	/*-- Window消去 --------------------------------------------*/

	ShowWindow(DG_hWnd, SW_HIDE) ;	// Window非表示
	/*----------------------------------------------------------*/

	/*-- Windows省電力設定を元に戻す ---------------------------*/

	RestoreWindowsPowerOffSetting() ;
	/*----------------------------------------------------------*/

	/*-- ErrorLog吐き出し --------------------------------------*/

	ErrorLogFlush() ;
	/*----------------------------------------------------------*/

	/*-- メッセージ表示 ----------------------------------------*/

	X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_ERROR_ABORT) ;
	/*----------------------------------------------------------*/

	/*-- 次のプロセスの実行を試みる ----------------------------*/

	X2W_NextProcess() ;
	/*----------------------------------------------------------*/

	/*-- ErrorLog吐き出し --------------------------------------*/

	ErrorLogSetStatus(TRUE) ;
	ReleaseErrorLog() ;
	/*----------------------------------------------------------*/

	ASSERT(0) ;					// DEBUG中は死んだほうが良い？
	exit(EXIT_FAILURE) ;		// 異常終了
}

/*--------------------------------------------------------------*/
/*	X2W_ErrorPrintfHResult	HRESULT内容表示						*/
/*--------------------------------------------------------------*/
int	X2W_ErrorPutsHResult(DWORD type, HRESULT hr)
{
	char	str[1024] ;
	int		err_id ;

	ErrorLogPrintf("X2W_ErrorPutsHResult:HRESULT %d\n", hr) ;	// Log

	/*-- 対応しているERROR IDがあればそちらに変更 --------------*/

	err_id = X2W_GetErrorIDFromHResult(hr) ;
	if( err_id >= 0 )
	{
		return( X2W_ErrorPutsID(type, err_id) ) ;
	}
	/*----------------------------------------------------------*/

	X2W_ErrorGetMess(str, sizeof(str), hr) ;

	return( X2W_ErrorPuts(type, str) ) ;
}

/*--------------------------------------------------------------*/
/*	InitErrorLog	エラーログ管理初期化						*/
/*--------------------------------------------------------------*/
void	InitErrorLog(void)
{
	char	path[ERROR_LOG_FILENAME_SIZE] ;

	ZeroMemory(&_log, sizeof(_log)) ;

	_log.create_file = FALSE ;
	strcpy(_log.filename, "log.txt") ;	// Default Name

	_log.ptr       = _log.buff ;
	_log.waterline = &_log.buff[ERROR_LOG_BUFF_WATERLINE] ;

	/*-- ファイル名作成 ----------------------------------------*/

	GetTempPath(ERROR_LOG_FILENAME_SIZE, path) ;	// テンポラリディレクトリを取得
	GetTempFileName(path, "log", 0, _log.filename) ;
	/*----------------------------------------------------------*/

	/*-- とりあえず出力 ----------------------------------------*/

	ErrorLogPrintf("Log Name:%s\n", _log.filename) ;
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	ReleaseErrorLog	エラーログ管理終了処理						*/
/*--------------------------------------------------------------*/
void	ReleaseErrorLog(void)
{
	/*-- 残りログの吐き出し ------------------------------------*/

	ErrorLogFlush() ;
	/*----------------------------------------------------------*/

	/*-- TempLOGの複写＆消去 -----------------------------------*/

	{
		char		filename[512] ;

		/*-- 最新Log -------------------------------------------*/

		sprintf(filename, "%s/last.log", pcGetErrorLogFilePath()) ;
		CopyFile(_log.filename, filename, FALSE) ;	// 複写
		/*------------------------------------------------------*/

		/*-- ERROR LOG -----------------------------------------*/

		if( _log.err )
		{

#ifdef DEBUG_MODE
			SYSTEMTIME	systime ;

			GetSystemTime(&systime) ;

			sprintf(filename, "%s/err.%04X%02X%X%02X%02X%02X%02X.log",
						pcGetErrorLogFilePath(),
						systime.wYear,
						systime.wMonth,
						systime.wDayOfWeek,
						systime.wDay,
						systime.wHour,
						systime.wMinute,
						systime.wSecond) ;
#else
			sprintf(filename, "%s/err.log", pcGetErrorLogFilePath()) ;
#endif
			CopyFile(_log.filename, filename, FALSE) ;	// 複写
		}
		/*------------------------------------------------------*/

		DeleteFile(_log.filename) ;
	}
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	ErrorLogSetStatus											*/
/*--------------------------------------------------------------*/
static	void	ErrorLogSetStatus(BOOL is_err)
{
	_log.err = is_err ;
}

/*--------------------------------------------------------------*/
/*	ErrorLogPrintf	エラーログPrintf							*/
/*--------------------------------------------------------------*/
void	ErrorLogPrintf(char *fmt, ...)
{
	va_list	ap ;

	/*-- 文字列展開 --------------------------------------------*/

	va_start( ap, fmt ) ;
	vsprintf(_log.ptr, fmt, ap) ;
	va_end( ap ) ;

#ifdef DEBUG_MODE
	printf("%s", _log.ptr) ;
#endif
	/*----------------------------------------------------------*/

	/*-- パラメータ補正 ----------------------------------------*/

	_log.ptr += strlen(_log.ptr) ;
	/*----------------------------------------------------------*/

	/*-- 危険領域の場合出力 ------------------------------------*/

	if( _log.ptr > _log.waterline ){ ErrorLogFlush() ; }
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	ErrorLogPuts	エラーログPuts								*/
/*--------------------------------------------------------------*/
void	ErrorLogPuts(char *str)
{
	int	len ;

#ifdef DEBUG_MODE
	printf("%s", str) ;
#endif
	len = strlen(str) ;
	
	/*-- 危険判定 ----------------------------------------------*/

	if( (_log.ptr + len) > _log.waterline ){ ErrorLogFlush() ; }
	/*----------------------------------------------------------*/

	/*-- Puts --------------------------------------------------*/

	sprintf(_log.ptr, "%s", str) ;
	_log.ptr += strlen(_log.ptr) ;
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	ErrorLogFlush	エラーログ吐き出し							*/
/*--------------------------------------------------------------*/
void	ErrorLogFlush(void)
{
	char	filename[256] ;
	HANDLE	fh ;
	DWORD	size ;

	/*-- 出力 --------------------------------------------------*/

	size = (DWORD)(_log.ptr - _log.buff) ;
	if( size )
	{
#if FALSE
		sprintf(filename, "%s/%s", pcGetErrorLogFilePath(), _log.filename) ;
#else
		sprintf(filename, "%s", _log.filename) ;
#endif

		if( !_log.create_file )
		{
			fh = CreateFile(filename,
							GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL) ;
			_log.create_file = TRUE ;
		}
		else
		{
			fh = CreateFile(filename,
							GENERIC_WRITE,
							0,
							NULL,
							OPEN_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL) ;
			SetFilePointer(fh ,0 ,NULL ,FILE_END) ;	// 追加書き込み
		}
		ASSERT( fh ) ;

		if( fh )
		{
			DWORD	dst_size ;
			WriteFile(fh, _log.buff, size, &dst_size, NULL) ;
			CloseHandle(fh) ;
		}
		else
		{
			printf("Can't Create %s\n", filename) ;
		}
	}
	/*----------------------------------------------------------*/

	/*-- パラメータ初期化 --------------------------------------*/

	_log.ptr = _log.buff ;
	/*----------------------------------------------------------*/
}
/*-- End Of File --*/
