/*
	fsxdev.c
	
	XBOX用開発環境用ファイルシステム
*/

#include <xtl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "libgv.h"
#include "libfs.h"
#include "cdbios.h"

/* -------------------------------------------- */
// Windows追加
#ifdef KP_WINDOWS

#define	DATA_PATH_STR_MAX	(256)
#define	PATH_CTGR_INI_FILE		"sys.ini"
#define	PATH_CTGR_INI_SECTION	"LIBFS"

typedef	struct	FS_PATH_CTGR_INFO_
{
	char	path[DATA_PATH_STR_MAX] ;
} FS_PATH_CTGR_INFO ;


static	char	data_root_path[DATA_PATH_STR_MAX] ;
static	char	path_ctgr_bin[DATA_PATH_STR_MAX] ;

static	FS_PATH_CTGR_INFO	path_ctgr_info_tbl[MAX_FS_PATH_CTGR_NUM] ;

static	void	devFsPathCtgrInit( void ) ;

#endif

/* -------------------------------------------- */
//開発環境用 ファイル関数

#include "xbdm.h"

#define XBFS_COMMAND "XBFS"
#define XBFS_TIMEOUT 100000

static HANDLE event_h;
static int xbfs_result;


static struct {
	void *buf;
	int size;
} xbfs_rd;

static HRESULT __stdcall xbfs_read_handler( PDM_CMDCONT pdmcc
										   , LPSTR szResponse, DWORD cchResponse )
{
#ifdef KP_WINDOWS
	return S_OK;
#else
	( char * )xbfs_rd.buf += pdmcc->DataSize;
	xbfs_rd.size -= pdmcc->DataSize;

	( char * )pdmcc->Buffer += pdmcc->DataSize;
	pdmcc->BufferSize -= pdmcc->DataSize;

	pdmcc->BytesRemaining = pdmcc->BufferSize;
	return XBDM_NOERR;
#endif
}


static HRESULT __stdcall xbfs_handler( LPCSTR szCommand, LPSTR szResponse
									  , DWORD cchResponse, PDM_CMDCONT pdmcc)
{
#ifdef KP_WINDOWS
	return S_OK;
#else
	switch( szCommand[ 5 ] ){
	  case 'r':
		{
			xbfs_result = ( int )strtoul( szCommand + 6, NULL, 10 );

			SetEvent( event_h );
		}
		break;
	  case 'b':
		{
			// binary recieve
			int size;

			size = strtol( szCommand + 6, NULL, 10 );
			xbfs_result = size;

			pdmcc->HandlingFunction = xbfs_read_handler;
			pdmcc->Buffer = xbfs_rd.buf;
			pdmcc->BufferSize = size;
			pdmcc->BytesRemaining = size;

			return XBDM_READYFORBIN;

		}
	}

	return XBDM_NOERR;
#endif
}

void devFsInit( void )
{
#ifdef KP_WINDOWS
	char	strbuff[DATA_PATH_STR_MAX] ;
	int		len ;

	if( (len = GetModuleFileName(NULL, strbuff, DATA_PATH_STR_MAX)) > 0 )
	{
		char	*ptr ;

		len-- ;
		ptr = strbuff + len ;
		for(; len>0; len--, ptr--)
		{
				if( *ptr == '\\' )	// 最後端の"\"
				{
					*ptr = 0 ;	// パスの終端
					break ;
				}
		}

		sprintf(path_ctgr_bin, "%s", strbuff) ;

#ifdef DEBUG_MODE
//		sprintf(data_root_path, "e:\\cdrom.img\\", strbuff) ;
		sprintf(data_root_path, "%s\\..\\cdrom.img\\", strbuff) ;
								// Data Root Path(仮)
#else
		sprintf(data_root_path, "%s\\..\\cdrom.img\\", strbuff) ;
								// Data Root Path(仮)
#endif
	}

	/* File Path設定 */
	devFsPathCtgrInit() ;

#else
	HRESULT res;
	res = DmRegisterCommandProcessor( XBFS_COMMAND, xbfs_handler );
	if( res != XBDM_NOERR ){
		printf( "can't regist %X\n", res );
		ASSERT( 0 );
	}
	event_h = CreateEvent( NULL, FALSE, FALSE, NULL );
#endif
}

#ifdef KP_WINDOWS
static	char	*path_ctgr_key_name_tbl[MAX_FS_PATH_CTGR_NUM] =
{
	"FS_PATH_CTGR_INI",		// 設定ファイルパス
	"FS_PATH_CTGR_STAGE",	// STAGEパス
	"FS_PATH_CTGR_STREAM",	// ストリームデータファイル
} ;

	/* デフォルトパス(実行モジュールパス相対) */
static	char	*path_ctgr_def_path_tbl[DATA_PATH_STR_MAX] =
{
	".",			// FS_PATH_CTGR_INI
	"../cdrom.img",	// FS_PATH_CTGR_STAGE
	"../cdrom.img",	// FS_PATH_CTGR_STREAM
} ;

static	void	devFsPathCtgrInit( void )
{
	int					i ;
	char				def_path[DATA_PATH_STR_MAX] ;
	FS_PATH_CTGR_INFO	*info ;
	char				**key_name ;
	char				**path ;
	char				ini_file[DATA_PATH_STR_MAX] ;

	/* 設定ファイルパス作成 */
	sprintf(ini_file, "%s\\" PATH_CTGR_INI_FILE, path_ctgr_bin) ;

	/* 設定取得 */
	path     = path_ctgr_def_path_tbl ;
	key_name = path_ctgr_key_name_tbl ;
	info     = path_ctgr_info_tbl ;
	for(i=MAX_FS_PATH_CTGR_NUM; i>0; i--, path++, key_name++, info++)
	{
		/* Default Path 作成 */
		sprintf(def_path, "%s/%s", path_ctgr_bin, *path) ;
		//printf("def_path:%s\n", path_ctgr_bin) ;

		/* 設定Path取得 */
		GetPrivateProfileString(PATH_CTGR_INI_SECTION, *key_name,
					def_path, info->path, DATA_PATH_STR_MAX,
					ini_file) ;
		ErrorLogPrintf("[ %s ]: %s\n", *key_name, info->path) ;
	}
}
#endif

static void WaitEvent( void )
{
	DWORD res;

	res = WaitForSingleObject( event_h, XBFS_TIMEOUT );
	if( res == WAIT_TIMEOUT ){
		printf( "XBFS:TIMEOUT!!\n" );
	}
}
#ifdef KP_WINDOWS

char* pcGetFilePath(void)
{
	return(path_ctgr_info_tbl[FS_PATH_CTGR_STAGE].path) ;
}

char* pcGetStreamFilePath(void)
{
	return(path_ctgr_info_tbl[FS_PATH_CTGR_STREAM].path) ;
}

char* pcGetErrorLogFilePath(void)
{
	return(path_ctgr_bin) ;
}

char* pcGetIniFilePath(void)
{
	return(path_ctgr_info_tbl[FS_PATH_CTGR_INI].path) ;
}

char* pcGetFullPathFilename(char *buff, int buff_size, char *name)
{
	DWORD	path_ctgr ;
	char	str[256] ;
	char	*str_ptr ;

	path_ctgr = FS_PATH_CTGR_STAGE ;
	str_ptr = str ;
	while( *name )
	{
		switch( *name )
		{
		  case ':' :	// ":"がファイル名についていたらSTREAMデータと認識
			path_ctgr = FS_PATH_CTGR_STREAM ;
			break ;

		  default :
			*str_ptr = *name ;
			str_ptr++ ;
			break ;
		}
		name++ ;
	}
	*str_ptr = 0 ;

#if TRUE
	{
		FS_PATH_CTGR_INFO	*info ;

		info = &path_ctgr_info_tbl[path_ctgr] ;
		sprintf( buff, "%s/%s", info->path, str );
	}
#else
	switch( mode )
	{
	  case 1 :
		/* STREAM */
		path = pcGetStreamFilePath();
		break ;

	  default :
		/* Other Data */
		path = pcGetFilePath();
		break ;
	}
	sprintf( buff, "%s%s", path, str );
#endif

#ifdef DEBUG_MODE
	printf("buff:%s\n", buff) ;
#endif
	return(buff) ;
}
#endif

#ifdef KP_WINDOWS
int pcOpenDirectPath( char *filename, int flag )
{
	HANDLE	fh;

	if( X2W_OverlappedReadFileEnable() )
	{
		// OVERLAPPED使用
		fh = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED ,NULL);
	}
	else
	{
		// OVERLAPPED使用不可能
		fh = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL);
	}

	return( (int)fh ) ;
}
#endif	// KP_WINDOWS

int pcOpen( char *filename, int flag )
{
#ifdef KP_WINDOWS
	//HANDLE	fh;
	char	full[256] ;

	pcGetFullPathFilename(full, 256, filename) ;
#if TRUE
	return( pcOpenDirectPath(full, flag) ) ;
#else
	if( X2W_OverlappedReadFileEnable() )
	{
		// OVERLAPPED使用
		fh = CreateFile(full,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED ,NULL);
	}
	else
	{
		// OVERLAPPED使用不可能
		fh = CreateFile(full,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL);
	}
#ifdef DEBUG_MODE
	printf("pcOpen -> %s (%d) (%d) \n",full,fh, flag );
//	if(flag&O_RDONLY){
//	}else{
//		printf("Not Read Only\n");
//	}
	printf("file size(%d) -> %d\n",fh,GetFileSize(fh,NULL));
#endif

	return( (int)fh ) ;
#endif

#else
	cprintf( "%s!Open,%s,%X\n", XBFS_COMMAND, filename, flag );
	WaitEvent();
//printf( "pcOpen:%d\n", xbfs_result );
	return xbfs_result;
#endif
}

int pcClose( int fd )
{
#ifdef KP_WINDOWS
#ifdef DEBUG_MODE
//@	printf("pcClose(%d) \n",fd);
#endif
	return( !CloseHandle((HANDLE)fd) ) ;
#else
	cprintf( "%s!Close,%X\n", XBFS_COMMAND, fd );
	WaitEvent();
//printf( "pcClose:%d\n", xbfs_result );
	return xbfs_result;
#endif
}

int pcRead( int fd, void *buf, int nbyte )
{
#ifdef KP_WINDOWS
	int	size;
	HANDLE	eh;
	OVERLAPPED		overlap;
	
	size  = 0;

	if( X2W_OverlappedReadFileEnable() )
	{
		// OVERLAPPED使用
		eh = CreateEvent( NULL, FALSE, FALSE, "pcRead" );
		overlap.Internal 	= 0; 
		overlap.InternalHigh	= 0; 
		overlap.Offset		= pcLseek( fd,0,SEEK_CUR);
		overlap.OffsetHigh	= 0;
		overlap.hEvent		= eh;

		ReadFile((HANDLE)fd
				,buf
				,nbyte
				,(DWORD*)&size
				,&overlap);

		GetOverlappedResult((HANDLE)fd	// ファイル、パイプ、通信デバイスのハンドル
   							,&overlap	// OVERLAPPED 構造体へのポインタ
							,&size   	// 転送されたバイト数を格納する変数へのポインタ
							,TRUE		// 待機フラグ
							);
							
		pcLseek( fd,size,SEEK_CUR);

		CloseHandle(eh);
	}
	else
	{
		// OVERLAPPED使用不可能
		ReadFile((HANDLE)fd
				,buf
				,nbyte
				,(DWORD*)&size
				,NULL);
		//while( size < nbyte ){ printf("(%d/%d)\n", size, nbyte) ; Sleep(1) ; }
	}

#ifdef DEBUG_MODE
//@	printf("pcRead(%d) -> (%d/%d) \n",fd,size,nbyte);
#endif
	return( size ) ;

#else
	xbfs_rd.size = nbyte;
	xbfs_rd.buf = buf;

	cprintf( "%s!Read,%X,%X,%X\n", XBFS_COMMAND, fd, buf, nbyte );

	WaitEvent();
//printf( "pcRead:%d\n", xbfs_result );

	return xbfs_result;
#endif
}

int pcWrite( int fd, void *buf, int nbyte )
{
#ifdef KP_WINDOWS
	return(WriteFile((HANDLE)fd,buf,nbyte,NULL,NULL));
#else
	cprintf( "%s!Write,%X,%X,%X\n", XBFS_COMMAND, fd, buf, nbyte );
	WaitEvent();
//printf( "pcWrite:%d\n", xbfs_result );
	return xbfs_result;
#endif
}

int pcLseek( int fd, unsigned int offset, int whence )
{
#ifdef KP_WINDOWS
	int	after;

	switch(whence){
	case	SEEK_CUR:
		after = SetFilePointer((HANDLE)fd
						,offset
						,NULL
						,FILE_CURRENT);
		break;

	case	SEEK_END:
		after = SetFilePointer((HANDLE)fd
						,offset
						,NULL
						,FILE_END);
		break;
	case	SEEK_SET:
		after = SetFilePointer((HANDLE)fd
						,offset
						,NULL
						,FILE_BEGIN);
		break;
	}
	return after ;
#ifdef DEBUG_MODE
//@	printf("pcLseek(%d) -> (%d/%d) \n",fd,after,offset);
#endif
#else
	cprintf( "%s!Seek,%X,%X,%X\n", XBFS_COMMAND, fd, offset, whence );
	WaitEvent();
	
//printf( "pcLseek:%d\n", xbfs_result );
	return xbfs_result;
#endif
}

/* -------------------------------------------- */

//#define LOAD_UNIT	(8196*16*16)
#define LOAD_UNIT	(0x7FFFFFFF)

static char current_dir[ 64 ];

typedef struct _HDFILE {
	int fd;
	int size;
	int unit;
	void *buf;
} HDFILE;

static HDFILE finfo;

void FS_ChangeDirectory( char *dir )
{
	extern void sd_change_directory( const char* path ) ;
	
	sprintf( current_dir, "stage/%s/", dir );
	// サウンドにディレクトリを通知
//	sd_set_cli( 0x80000000 | ( int )current_dir );
	sd_change_directory( current_dir );
}

int FS_LoadRequest( char *name )
{
	char full[ 128 ];
	HDFILE *fn;
	int fd;
	int size;

	fn = &finfo;
	ASSERT( fn->fd == -2 );	// 読み込み中ではないか

	sprintf( full, "%s%s", current_dir, name );
	while( ( fd = pcOpen( full, O_RDONLY ) ) < 0 ){
		char	fullfull[256] ;
		int		sel ;

		printf( "FILE %s not found\n", full );

		pcGetFullPathFilename(fullfull, sizeof(fullfull), full) ;

		/* 再試行意思確認 */
		sel = X2W_ErrorPrintfID(X2W_MB_RETRYCANCEL,
							X2WERR_ID_ASK_RETRY_FILEREAD,
							fullfull) ;
		switch( sel )
		{
		  case IDCANCEL :	// キャンセル
			/* 本当に止めるのか意思確認 */
			sel = X2W_ErrorPrintfID(X2W_MB_OKCANCEL | MB_DEFBUTTON2,	// デフォルトCANCEL
								X2WERR_ID_ASK_ERROR_EXIT) ;
			switch( sel )
			{
			  case IDOK :	// OK選択
				X2W_ErrorAbort() ;		// 異常終了手続き
				break ;
			}
		}
	}

	size = pcLseek( fd, 0, SEEK_END );
	pcLseek( fd, 0, SEEK_SET );

	fn->buf = NULL;
	fn->fd = fd;

	return size;
}

#ifdef KP_WINDOWS
int FS_LoadRequestDirectPath( char *name )
{
	HDFILE *fn;
	int fd;
	int size;

	fn = &finfo;
	ASSERT( fn->fd == -2 );	// 読み込み中ではないか

	while( ( fd = pcOpenDirectPath( name, O_RDONLY ) ) < 0 ){
		int	sel ;

		printf( "FILE %s not found\n", name );

		/* 再試行意思確認 */
		sel = X2W_ErrorPrintfID(X2W_MB_RETRYCANCEL,
							X2WERR_ID_ASK_RETRY_FILEREAD,
							name) ;
		switch( sel )
		{
		  case IDCANCEL :	// キャンセル
			/* 本当に止めるのか意思確認 */
			sel = X2W_ErrorPrintfID(X2W_MB_OKCANCEL | MB_DEFBUTTON2,	// デフォルトCANCEL
								X2WERR_ID_ASK_ERROR_EXIT) ;
			switch( sel )
			{
			  case IDOK :	// OK選択
				X2W_ErrorAbort() ;		// 異常終了手続き
				break ;
			}
		}
	}

	size = pcLseek( fd, 0, SEEK_END );
	pcLseek( fd, 0, SEEK_SET );

	fn->buf = NULL;
	fn->fd = fd;

	return size;
}
#endif

void FS_LoadSet( void *buf, int size )
{
	HDFILE *fn;
	fn = &finfo;

	ASSERT( fn->fd >= 0 );
#ifdef DEBUG_MODE
	printf( "LoadSet %X size %d\n", buf, size );
#endif
	fn->buf = buf;
	fn->unit = LOAD_UNIT;
	fn->size = size;
}

void FS_LoadStop( void )
{
	HDFILE *fn;
	fn = &finfo;

	ASSERT( fn->fd >= 0 );

	pcClose( fn->fd );
	fn->fd = -2;
	fn->buf = NULL;
}

int FS_LoadSync( void )
{
	HDFILE *fn;
	int size;

	fn = &finfo;

	ASSERT( fn->buf != NULL );

	size = ( fn->size < LOAD_UNIT ) ? fn->size : LOAD_UNIT;
	pcRead( fn->fd, fn->buf, size );
	( char * )fn->buf += size;
	if( ( fn->size -= size ) <= 0 ){
		FS_LoadStop();
		return -1;
	}
	return fn->size;
}

void FS_LoadFileRequest( int fileno, int offset, int size, void *buffer )
{
	while( cdbios_get_status() != 0 ){
		Sleep( 1 );
	}

	cdbios_read( buffer, FS_GET_FILE_POS( fileno ) + offset, size );
}

int  FS_LoadFileSync( void )
{
	return cdbios_get_status();
}

/* ----------------------------------------------------- */

void FS_DevInit( void )
{
	HDFILE *fn;

	fn = &finfo;
	fn->fd = -2;
	fn->buf = NULL;

	printf( "FS_DevInit\n" );

	devFsInit();
}
