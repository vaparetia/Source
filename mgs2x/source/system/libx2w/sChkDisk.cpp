//=============================================================================
// 2002/12/26
//		sChkDisk.cpp : 
//=============================================================================
#include <windows.h>
#ifdef __cplusplus	// Takaki付加
extern "C" {
#endif

BOOL IsChkMGS2DvdDrive();

#ifdef __cplusplus	// Takaki付加
}
#endif
//-----------------------------------------------------------------------------
//	IsChkMGS2DvdDrive
//	[内容]
//		DVDドライブを見に行って
//		メタルギアソリッドメディアが入ってるかどうかチェックする
//		ほとんど直値ですいませんって感じ
//
//	[引数]
//		なし
//	[戻り値]
//		TRUE  : メタルギアソリッドメディアが挿入されている
//		FALSE : メタルギアソリッドメディアがささってない
//
//-----------------------------------------------------------------------------
BOOL IsChkMGS2DvdDrive()
{
	BOOL	bResult;
	char	szPath[MAX_PATH];
	UINT	uiDriveType;
	DWORD	dwType, dwBufSize;
	HKEY	hOpenKey;
	// ファイル検索対象パス
	// あまりに多いと時間がかかるかもしれないので…
	const char* pszFindFile[] =
	{
		"setup.exe",
		"engine32.cab",
		"Autorun.inf",
		"setup.inx",
		"setup.ini",
		"setup.boot",
		"data1.cab",
		"data1.hdr",
		"layout.bin",
		"MGS2SSetup.exe",
		"mgs2.exe",
		"mgs2_sse.exe",
		NULL,
	};

	bResult   = FALSE;
	dwBufSize = MAX_PATH;
	if( ERROR_SUCCESS ==
		RegOpenKeyEx( HKEY_CURRENT_USER, "software\\KONAMI\\MGS2S",
					  0, KEY_EXECUTE, &hOpenKey ) )
	{
		if( ERROR_SUCCESS ==
			RegQueryValueEx( hOpenKey,
							 "SrcDrive", 0, &dwType,
							 ( LPBYTE )szPath, &dwBufSize ) )
		{
			bResult = TRUE;
		}
		RegCloseKey( hOpenKey );
	}
	if( bResult != FALSE )
	{
		int		i;
		char*	p;
		HANDLE	hFile;
		DWORD	dwSizeHigh, dwSizeLow;
		char	szFilePath[MAX_PATH];
		//=====================================================================
		//
		// CD(DVDドライブ)かどうか判別
		//
		p = strchr( szPath, ':' );	// ドライブのみを抽出
		if( p )	*( p+1 ) = '\0';
		uiDriveType = GetDriveType( szPath );
		if( uiDriveType != DRIVE_CDROM )	return FALSE;
		//=====================================================================
		//=====================================================================
		//
		// あるファイルを開いて、ファイルサイズを取得する
		//
		strcpy( szFilePath, szPath );
		strcat( szFilePath, "\\data2.cab" );
		hFile = CreateFile( szFilePath, GENERIC_READ, 0,
							NULL, OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL );
		if( hFile == INVALID_HANDLE_VALUE )
		{
			return FALSE;
		}
		dwSizeLow = GetFileSize( hFile, &dwSizeHigh );
		CloseHandle( hFile );
		// ３G以上？
		if( dwSizeLow < 0xB2D05E00 )
		{
			return FALSE;
		}
		//=====================================================================
		//=====================================================================
		//
		// ファイルがあるかを調べる
		//
		for( i = 0 ; pszFindFile[i] != NULL ; i++ )
		{
			strcpy( szFilePath, szPath );
			strcat( szFilePath, pszFindFile[i] );
			// 他アプリも読み取り許可にして開く
			if( INVALID_HANDLE_VALUE == 
				( hFile = CreateFile( szFilePath, GENERIC_READ,
									  FILE_SHARE_READ, NULL, OPEN_EXISTING,
									  FILE_ATTRIBUTE_NORMAL,NULL ) ) )
			{
				return FALSE;
			}
			CloseHandle( hFile );
		}
		//=====================================================================

		return TRUE;
	}

	return FALSE;
}
//=============================================================================
// end of sChkDisk.cpp
//=============================================================================
