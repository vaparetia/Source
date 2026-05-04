//=============================================================================
// 2002/11/14
//		OptionIO.cpp : オプション 入出力用 いぬぷりめんと
//												copyright(C) 2002 inuniku
//=============================================================================
//-----------------------------------------------------------------------------
// イヌクルード
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include "OptionIO.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//	S_MGS2SGetModuleDirectory
//	[内容]
//		メタルギアソリッドのインストール先フォルダを調べる
//
//	[引数]
//		pszBuf		インストール先の場所名を格納するためのバッファ
//		size		バッファサイズ
//	[戻り値]
//		TRUE  : 成功
//		FALSE : 失敗
//				失敗すると、pszBufには今実行しているファイルのパスが格納されます
//
//-----------------------------------------------------------------------------
BOOL S_MGS2SGetModuleDirectory( char* pszBuf, size_t size )
{
	BOOL	bResult = FALSE;
	DWORD	dwType;
	DWORD	dwBufSize = ( DWORD )size;
	HKEY	hOpenKey;

	if( ERROR_SUCCESS ==
		RegOpenKeyEx( HKEY_CURRENT_USER, "software\\MGS2S",
					  0, KEY_EXECUTE, &hOpenKey ) )
	{
		if( ERROR_SUCCESS ==
			RegQueryValueEx( hOpenKey,
							 "InstallDir", 0, &dwType,
							 ( LPBYTE )pszBuf, &dwBufSize ) )
		{
			bResult = TRUE;
		}
		RegCloseKey( hOpenKey );
	}
	if( bResult == FALSE )
	{
		char* p;

		GetModuleFileName( NULL, pszBuf, ( DWORD )size );
		p = strrchr( pszBuf, '\\' );
		if( p )	*p = '\0';
	}
	return bResult;
}
// end of S_MGS2SGetModuleDirectory
//-----------------------------------------------------------------------------
//	S_MGS2SSaveRankingPassword
//	[内容]
//		メタルギアソリッド ランキングをオプション用初期化ファイルに
//		保存します
//
//	[引数]
//		iMode			モード
//						0 : SONS OF LIBERTY
//						1 : BOSS SURVIVAL
//						2 : TOP RECORD CODE
//		pszPassword		終端にNULLを含んだ パスワードの文字列
//						SONS OF LIBERTY : 28文字
//						BOSS SURVIVAL	: 35文字
//						TOP RECORD CODE	: 20文字
//	[戻り値]
//		なし
//
//-----------------------------------------------------------------------------
void S_MGS2SSaveRankingPassword( const int iMode, const char* pszPassword )
{
	char	szFilePath[MAX_PATH], szKey[64], szBuf[128];
	sInt	iCnt, iCur;
	time_t	t;
	struct tm*	pTm;

	GetModuleFileName( NULL, szFilePath, MAX_PATH );
	*( strrchr( szFilePath, '\\' ) + 1 ) = '\0';
	strcat( szFilePath, OPTIONFILENAME );

	iCnt = ( sInt )GPI( SEC_RANKING, KEY_RANKNUM, DEF_RANKNUM, szFilePath );
	iCur = iCnt;

	// 重複コードを探す（あったら(´･ω･`)ｼｮﾎﾞｰｿです）
	{
		sInt i, y, m, d, h, mi, mode;
		char szCode[64];

		for( i = 0 ; i < iCnt ; i++ )
		{
			wsprintf( szKey, KEY_RANKDATA, i );
			GPS( SEC_RANKING, szKey, "", szBuf,
				 sizeof( szBuf ), szFilePath );
			sscanf( szBuf, fmtRANKINGDATA,
					&y, &m, &d, &h, &mi, &mode, szCode );
			// !!(*'-')重複 汁
			if( strcmp( szCode, pszPassword ) == 0 )
			{
				iCur = i;
			}
		}
	}

	if( iCnt == iCur )
		WPSM( SEC_RANKING, KEY_RANKNUM,	iCnt+1, szBuf, szFilePath );

	time( &t );
	pTm = localtime( &t );
	wsprintf( szKey, KEY_RANKDATA, iCur );
	wsprintf( szBuf, fmtRANKINGDATA,
			  pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday,
			  pTm->tm_hour, pTm->tm_min, iMode,  pszPassword );
	WPS( SEC_RANKING, szKey, szBuf, szFilePath );
}
// end of S_MGS2SSaveRankingPassword
//-----------------------------------------------------------------------------
//	S_MGS2SSettingLoad
//	[内容]
//		メタルギアソリッド 設定ファイルのロードします
//		各項目に対してロードできなかった
//		もしくは、ファイルそのものがなかった場合は
//		デフォルト値が入るようになっています
//
//	[引数]
//		pO			SOptionIO構造体（結果を格納します）
//		pszFilePath	設定ファイル名を含んだ絶対パスを指定
//	[戻り値]
//		なし
//
//-----------------------------------------------------------------------------
void S_MGS2SSettingLoad( SOptionIO* pO, const char* pszFilePath )
{
	memset( pO, 0, sizeof( SOptionIO ) );

/*
	Drawing QualityとEffect Qualityのデフォルト設定は
	
	const Byte c_aucDrawingCustom[16][3];	// DlgDrawing.cpp
	const Byte c_aucProductCustom[16][7];	// DlgProduction.cpp

	の値を参照する
*/
	// 起動設定のロード
	pO->m_setting.m_bSSECPU			= GPI( SEC_STARTING, KEY_SSECPU,	DEF_SSECPU,		pszFilePath );
	pO->m_setting.m_bWinMode		= GPI( SEC_STARTING, KEY_WINMODE,	DEF_WINMODE,	pszFilePath );
	pO->m_setting.m_b32bitZBuff		= GPI( SEC_STARTING, KEY_32ZBUFF,	DEF_32ZBUFF,	pszFilePath );
	pO->m_setting.m_ucBpp			= GPI( SEC_STARTING, KEY_BPP,		DEF_BPP,		pszFilePath );
	pO->m_setting.m_wWidth			= GPI( SEC_STARTING, KEY_WIDTH,		DEF_WIDTH,		pszFilePath );
	pO->m_setting.m_wHeight			= GPI( SEC_STARTING, KEY_HEIGHT,	DEF_HEIGHT,		pszFilePath );
	GPS( SEC_STARTING, KEY_DRIVERNAME, "", pO->m_setting.m_szDriverName,
		 sizeof( pO->m_setting.m_szDriverName ), pszFilePath );
	// 描画設定のロード
	pO->m_drawing.m_ucRenderSize	= GPI( SEC_DRAWING, KEY_RENDERSIZE,	DEF_RENDERSIZE,	pszFilePath );
	pO->m_drawing.m_ucDetail		= GPI( SEC_DRAWING, KEY_DETAIL,		DEF_DETAIL,		pszFilePath );
	pO->m_drawing.m_ucQuality		= GPI( SEC_DRAWING, KEY_QUALITY,	DEF_QUALITY,	pszFilePath );
	pO->m_drawing.m_ucClearness		= GPI( SEC_DRAWING, KEY_CLEANESS,	DEF_CLEANESS,	pszFilePath );
	// 演出効果設定のロード
	pO->m_product.m_bFocus			= GPI( SEC_PRODUCT, KEY_FOCUS,		 DEF_FOCUS,		pszFilePath );
	pO->m_product.m_bMotionBlur		= GPI( SEC_PRODUCT, KEY_MOTIONBLUR,  DEF_MOTIONBLUR,pszFilePath );
	pO->m_product.m_bVRMode			= GPI( SEC_PRODUCT, KEY_VRMODE,		 DEF_VRMODE,	pszFilePath );
	pO->m_product.m_bStealth		= GPI( SEC_PRODUCT, KEY_STEALTH,	 DEF_STEALTH,	pszFilePath );
	pO->m_product.m_bRadioFocus		= GPI( SEC_PRODUCT, KEY_RADIOFOCUS,	 DEF_RADIOFOCUS,pszFilePath );
	pO->m_product.m_ucCrossFade		= GPI( SEC_PRODUCT, KEY_CROSSFADE,	 DEF_CROSSFADE,	pszFilePath );
	pO->m_product.m_ucEfQuantity	= GPI( SEC_PRODUCT, KEY_EFFQUANTITY, DEF_EFFQUANTITY, pszFilePath );
	// サウンド設定ロード
	pO->m_sound.m_ucQuality			= GPI( SEC_SOUND,	KEY_QUALITY,	 DEF_SNDQUALITY,pszFilePath );
	pO->m_sound.m_ucSE				= GPI( SEC_SOUND,	KEY_SE,			 DEF_SE,		pszFilePath );
	pO->m_sound.m_uc3DQuality		= GPI( SEC_SOUND,	KEY_3DQUALITY,	 DEF_3DSOUND,	pszFilePath );
	GPS( SEC_SOUND, KEY_DRIVERNAME, "", pO->m_sound.m_szDriverName,
		 sizeof( pO->m_sound.m_szDriverName ), pszFilePath );
}
// end of S_MGS2SSettingLoad
//=============================================================================
// end of OptionIO.cpp
//=============================================================================
