//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  sceMc* -> Xbox むりやりエミュレート
  
  2002/03/15 M.Kobayashi

  $Id: sceemux.c,v 1.5 2002/11/23 12:16:40 Yoshizawa1 Exp $
 */


#include "libgv.h"

#ifdef BP_WIN32 //BP_TODO

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef	struct sceMcTblGetDir {
	FILETIME _Create;
	FILETIME _Modify;
	unsigned FileSizeByte;
	unsigned short AttrFile;
	unsigned short Reserve1;
	unsigned Reserve2;
	unsigned PdaAplNo;
	unsigned char EntryName[32];
} sceMcTblGetDir;



#include "sceemux.h"


#define	NUM_PORTS	4
#define NUM_SLOTS	2

static char chMUDrives[NUM_PORTS][NUM_SLOTS];

static char chCurDir[ MAX_PATH ];
static char chDirName[ MAX_PATH ];



#define		SCE_RDONLY	0x0001
#define		SCE_WRONLY	0x0002
#define		SCE_RDWR	0x0003
#define		SCE_CREAT	0x0200

enum {	// 作業中の仕事 sceMcFuncNo* に対応
	CMD_NULL,
	CMD_INFO,
	CMD_OPEN,
	CMD_CLOSE,
	CMD_SEEK,
	CMD_READ,
	CMD_WRITE,
	CMD_FLUSH = 10,
	CMD_MKDIR,
	CMD_CHDIR,
	CMD_GETDIR,
	CMD_FILEINFO,
	CMD_DELETE,
	CMD_FORMAT,
	CMD_UNFORMAT,
	CMD_ENTSPACE,
	CMD_RENAME,
	CMD_CHGPRIOR,
};
static int cmd;
static int result;
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
		CloseHandle(fh);
	}
}  







static char* GetCurrentDir( int port, int slot )
{	// カレントディレクトリを取得
	strcpy(chCurDir ,save_root_path);
	chDirName[ 0 ] = '\0';
	return chCurDir;
}

static int XCreateSaveGame( char *root , char* path,int flag, int flag2 , char *chCDir, int size) {

	WIN32_FIND_DATA wfd;
	char	fname[MAX_PATH];
	HANDLE fh;
	sprintf(fname,"%s%s",save_root_path,path);
	
	if((fh = FindFirstFile( save_root_path , &wfd )) == INVALID_HANDLE_VALUE ){
		SECURITY_ATTRIBUTES SecurityAttributes;
	
		SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES); 
		SecurityAttributes.lpSecurityDescriptor = NULL; 
		SecurityAttributes.bInheritHandle = 0; 
	
		if(!CreateDirectory(save_root_path,&SecurityAttributes )){
			return(ERROR_SUCCESS+1);
		}
	}else{
		CloseHandle(fh);
	}
	strncpy(chCDir, fname, MAX_PATH );
	return(ERROR_SUCCESS);
}
#else
static char* GetCurrentDir( int port, int slot )
{	// カレントディレクトリを取得
	if( chCurDir[ 0 ] != chMUDrives[ port ][ slot ] ) {
		strcpy( chCurDir + 1, ":\\" );
		chCurDir[ 0 ] = chMUDrives[ port ][ slot ];
		chDirName[ 0 ] = '\0';
	}
	return chCurDir;
}
#endif

int sceMcGetInfo( int slot, int port, int *type, int *free, int *format )
{	// 数の関係でとりあえず slot と port を入れ替えている
	// メモリカードの抜き差しを見る
#if 0	
	DWORD	dwInsertions;
	DWORD	dwRemovals;
	int	iPort, iSlot;
	BOOL	bInsert = FALSE;

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
				if( slot == iSlot && port == iPort ) bInsert = TRUE;
			}
		}
	}
	if( bInsert ) {
		result = -1;	//sceMcResChangedCard
	} else if( chMUDrives[port][slot] != 0 ) {
		result = 0;		//sceMcResSucceed
	} else {
		result = -11;	// ささってない
	}

	if( result != -11 ) {
		static char drive_name[] = "X:\\";
		ULARGE_INTEGER	uliFreeAvail, uliFree, uliTotal;
		char* szDrive = drive_name;
		
		if( type != NULL ) *type = 2;	// sceMcTypePS2
		if( free != NULL ) {
			szDrive[0] = chMUDrives[port][slot];
			GetDiskFreeSpaceEx( szDrive, &uliFreeAvail, &uliTotal, &uliFree );
			*free = uliFree.LowPart;
		}
		if( format != NULL ) *format = 1;	// フォーマット済み Xでは未フォーマットはマウントできない
	}
	
	cmd = CMD_INFO;
#endif
	return 0;
}

int sceMcChdir( int slot, int port, const char* path, char* pwd )
{	// カレントディレクトリ移動
	char* pCur = GetCurrentDir( port, slot );

	if( *pCur == 0 ) {
		return -1;	//err
	}
	
	if( strcmp( path, chDirName ) != 0 ) {

		if( ERROR_SUCCESS == XCreateSaveGame( GetCurrentDir( port, slot ) ,
											  path,
											  OPEN_ALWAYS,
											  0,	
											  chCurDir,
											  MAX_PATH ) ) {
			strncpy( chDirName, path, MAX_PATH );
		} else return -1;
	}
										  

	if( pwd != NULL ) {
		strcpy( pwd, pCur );
	}
	cmd = CMD_CHDIR;
	result = 0;
			
	return 0;	// Succeed
}

int sceMcMkdir( int slot, int port, const char* name )
{	// ディレクトリを作成。Ｘの場合は XCreateGame で代用
	// 内部的には Chdir までやってしまう

	if( ERROR_SUCCESS == XCreateSaveGame( GetCurrentDir( port, slot ) ,
										  name,
										  OPEN_ALWAYS,
										  0,
										  chCurDir,
										  MAX_PATH ) ) {
		strncpy( chDirName, name, MAX_PATH );

		cmd = CMD_MKDIR;
		result = 0;
		
		return 0;
	} else return -1;

	
}

int sceMcOpen( int slot, int port, const char* name, int mode )
{
	HANDLE hFile;
	char chFile[ MAX_PATH ];
	DWORD	dwAccess = 0;

	if( chCurDir[ 0 ] != chMUDrives[ port ] [ slot ] ) {
		return result = -11;
	}
	strcpy( chFile, chCurDir );
	strcat( chFile, name );
	if( mode & SCE_RDONLY ) dwAccess |= GENERIC_READ;
	if( mode & SCE_WRONLY ) dwAccess |= GENERIC_WRITE;
	
	if( (hFile = CreateFile( chFile, dwAccess, 0, NULL,
							 (mode & SCE_CREAT)? CREATE_ALWAYS : OPEN_ALWAYS,
							 FILE_ATTRIBUTE_NORMAL, NULL ) ) == INVALID_HANDLE_VALUE ) {
		return result = -11;
	}

	cmd = CMD_OPEN;
	result = (int)hFile;

	return 0;
}

int sceMcClose( int fd ){
	CloseHandle( (HANDLE)fd );
	cmd = CMD_CLOSE;
	result = 0;
	return 0;
}

int sceMcSeek( int fd, int offset, int mode )
{
	DWORD	dwNewptr;
#ifdef KP_WINDOWS
	if( ( dwNewptr = SetFilePointer( (HANDLE)fd, (LONG)offset, NULL,
									 (mode == 0) ? FILE_BEGIN : ( ( mode == 1 )? FILE_CURRENT : FILE_END ) ) )
		== 0 ) {
		return -11;
	}
#else
	if( ( dwNewptr = SetFilePointer( (HANDLE)fd, (LONG)offset, NULL,
									 (mode == 0) ? FILE_BEGIN : ( ( mode == 1 )? FILE_CURRENT : FILE_END ) ) )
		== INVALID_SET_FILE_POINTER ) {
		return -11;
	}
#endif
	cmd = CMD_SEEK;
	result = dwNewptr;

	return 0;
}

int	sceMcRead( int fd, void* pBuf, int size )
{
	DWORD	dwRead;
	
	if( !ReadFile( (HANDLE)fd, pBuf, size, &dwRead, NULL ) ) {
		return -11;
	}

	cmd = CMD_READ;
	result = dwRead;

	return 0;
}

int sceMcWrite( int fd, void* pBuf, int size )
{
	DWORD	dwWrite;
	
	if( !WriteFile( (HANDLE)fd, pBuf, size, &dwWrite, NULL ) ) {
		return -11;
	}

	cmd = CMD_WRITE;
	result = dwWrite;

	return 0;
}


int sceMcRename( int slot, int port, const char* org, const char* dst )
{
	char chNameOrg[ MAX_PATH ];
	char chNameDst[ MAX_PATH ];


	GetCurrentDir( port, slot );
	
	if( chCurDir[ 0 ] != chMUDrives[ port ] [ slot ] ) {
		return -1;
	}

	strcpy( chNameOrg, chCurDir );
	strcpy( chNameDst, chCurDir );
	strcat( chNameOrg, org );
	strcat( chNameDst, dst );

	if( !MoveFile( chNameOrg, chNameDst ) ) {
		return -1;
	}

	cmd = CMD_RENAME;
	result = 0;

	return 0;
}

int sceMcDelete( int slot, int port, const char* name )
{
	char chName[ MAX_PATH ];

	GetCurrentDir( port, slot );
	
	if( chCurDir[ 0 ] != chMUDrives[ port ] [ slot ] ) {
		return -1;
	}

	strcpy( chName, chCurDir );
	strcat( chName, name );

	if( !DeleteFile( chName ) ) {
		return -1;
	}

	cmd = CMD_DELETE;
	result = 0;

	return 0;
}


int sceMcSetFileInfo ( int slot, int port, const char* name, const char* info, unsigned valid )
{
	cmd = CMD_FILEINFO;
	result = 0;
	return 0;
}

int sceMcGetDir( int slot, int port, const char* name, unsigned mode, int maxent, sceMcTblGetDir* table )
{
	cmd = CMD_GETDIR;
	result = 0;
	return 0;
}




int sceMcFormat( int slot, int port )
{	// Ｘではフォーマットの必要なし
	cmd = CMD_FORMAT;
	result = 0;
	return 0;
}

int sceMcUnformat( int slot, int port )
{	// Ｘではフォーマットの必要なし
	cmd = CMD_UNFORMAT;
	result = 0;
	return 0;
}


int	sceMcSync( int mode, int *pcmd, int *presult )
{
	switch( cmd ) {
	case CMD_NULL:
		return -1;	// sceMcExecIdle
	default:
		if( pcmd != NULL ) *pcmd = cmd;
		if( presult != NULL ) *presult = result;
		cmd = CMD_NULL;
		return 1;	// sceMcExecFinish
	}
	return 0;
}




//////////////////////////////////////
void	sceMcEmulateInit( void )
{	// マウント情報のリセット
	ZeroMemory( &chMUDrives[0][0], sizeof( chMUDrives ) );
	ZeroMemory( chCurDir, sizeof( chCurDir ) );
	cmd = CMD_NULL;
#ifdef KP_WINDOWS
	GetSaveDirectry( );
#endif
}

void	sceMcEmulateStop( void )
{	// マウントされているドライブがあればアンマウントする
#if 0	
	int iPort, iSlot;
	for( iPort = 0 ; iPort < NUM_PORTS; iPort++ ) {
		for( iSlot = 0 ; iSlot < NUM_SLOTS ; iSlot++ ) {
			if( chMUDrives[iPort][iSlot] != 0 ) {
				DWORD err = XUnmountMU( iPort, iSlot );
				ASSERT( err == ERROR_SUCCESS );
				chMUDrives[iPort][iSlot] = 0;
			}
		}
	}
#endif	
}

#endif //BP_WIN32
