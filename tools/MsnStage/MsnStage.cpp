// MsnStage.cpp : アプリケーション用クラスの定義を行います。
//

#include "stdafx.h"
#include "MsnStage.h"
#include "MsnStageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsnStageApp

BEGIN_MESSAGE_MAP(CMsnStageApp, CWinApp)
	//{{AFX_MSG_MAP(CMsnStageApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsnStageApp クラスの構築

CMsnStageApp::CMsnStageApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CMsnStageApp オブジェクト

CMsnStageApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMsnStageApp クラスの初期化

BOOL CMsnStageApp::InitInstance()
{
	AfxEnableControlContainer();

	// 標準的な初期化処理

#ifdef _AFXDLL
	Enable3dControls();			// 共有 DLL 内で MFC を使う場合はここをコールしてください。
#else
	Enable3dControlsStatic();	// MFC と静的にリンクする場合はここをコールしてください。
#endif

	CMsnStageDlg dlg;
	m_pMainWnd = &dlg;

	if(m_lpCmdLine[0] == _T('\0'))
	{
		// 通常起動

		int nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
		}
		else if (nResponse == IDCANCEL)
		{
		}
	}
	else
	{
		// コマンドライン起動

		char szPath[_MAX_PATH];
		UINT unStartPos, unEndPos;

		if(*((char*)m_lpCmdLine) == '"')
		{
			// Windows2000など未確認
			unStartPos = 1;
			unEndPos   = unStartPos + 1;

			while(unEndPos < strlen(m_lpCmdLine))
			{
				while(*((char*)m_lpCmdLine + unEndPos) != '"')
				{
					unEndPos++;
				}
				memset(szPath, 0x00, sizeof(szPath));
				strncpy(szPath, ((char*)m_lpCmdLine + unStartPos), unEndPos - unStartPos);
				Execute(szPath);

				unStartPos = unEndPos + 3;
				unEndPos   = unStartPos + 1;
			}
		}
		else
		{
			// Windows98SEなど
			char szLongPath[_MAX_PATH];

			unStartPos = 0;
			unEndPos   = unStartPos + 1;

			while(unEndPos < strlen(m_lpCmdLine))
			{
				while(*((char*)m_lpCmdLine + unEndPos) != ' ')
				{
					unEndPos++;
				}
				memset(szPath, 0x00, sizeof(szPath));
				memset(szLongPath, 0x00, sizeof(szLongPath));
				strncpy(szPath, ((char*)m_lpCmdLine + unStartPos), unEndPos - unStartPos);
				GetLongPathName(szPath, szLongPath, sizeof(szLongPath));
				Execute(szLongPath);

				unStartPos = unEndPos + 1;
				unEndPos   = unStartPos + 1;
			}
		}
	}

	// ダイアログが閉じられてからアプリケーションのメッセージ ポンプを開始するよりは、
	// アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

HRGN CMsnStageApp::CreateRgnFromBitmap(HBITMAP hBitmap)
{
	BITMAP           bm;
	BITMAPINFOHEADER bmih;

	// メモリデバイスコンテキストの作成
	HDC hDC = CreateCompatibleDC(NULL);
	if(hDC == NULL)
	{
		return NULL;
	}

	// ビットマップ情報の取得
	GetObject(hBitmap, sizeof(BITMAP), &bm);

	// リージョン用バッファ
	RGNDATA* pRgnData = (RGNDATA*)malloc(
		sizeof(RGNDATAHEADER) + sizeof(RECT) * bm.bmWidth * bm.bmHeight);
	if(pRgnData == NULL)
	{
		DeleteDC(hDC);
		return NULL;
	}
	RECT* pRect     = (RECT*)pRgnData->Buffer;
	int   rectCount = 0;

	// 1行解析用
	COLORREF* pScanData = (COLORREF*)malloc(sizeof(COLORREF) * bm.bmWidth);
	if(pScanData == NULL)
	{
		DeleteDC(hDC);
		free(pRgnData);
		return NULL;
	}
	COLORREF clrTransparent;

	// ビットマップ情報の設定
	memset(&bmih, 0x00, sizeof(BITMAPINFOHEADER));
	bmih.biSize        = sizeof(BITMAPINFOHEADER);
	bmih.biWidth       = bm.bmWidth;
	bmih.biHeight      = bm.bmHeight;
	bmih.biPlanes      = 1;
	bmih.biBitCount    = 32;
	bmih.biCompression = BI_RGB;

	// 1行ずつビットマップを解析
	for(int y = 0; y < bm.bmHeight; y ++)
	{
		// 1行取得
		GetDIBits(hDC, hBitmap, bm.bmHeight - y - 1, 1,
			pScanData, (BITMAPINFO*)&bmih, DIB_RGB_COLORS);

		for(int x = 0; x < bm.bmWidth; x ++)
		{
			// 左上の色を透過色に
			if((y == 0) && (x == 0))
				clrTransparent = pScanData[x];

			// 透過色でないならリージョン追加
			if(pScanData[x] != clrTransparent)
			{
				// 透過色にあたるまで検索
				int start  = x;
				int length = 1;
				for(x++; x < bm.bmWidth; x++, length++)
				{
					if(pScanData[x] == clrTransparent)
						break;
				}
				pRect->left   = start;
				pRect->right  = start + length;
				pRect->top    = y;
				pRect->bottom = y + 1;
				pRect++;
				rectCount++;
			}
		}
	}
	free(pScanData);
	DeleteDC(hDC);

	// リージョンを作成
	RGNDATAHEADER& rdh  = pRgnData->rdh;
	rdh.dwSize          = sizeof(RGNDATAHEADER);
	rdh.iType           = RDH_RECTANGLES;
	rdh.nRgnSize        = sizeof(RGNDATAHEADER) + sizeof(RECT) * rectCount;
	rdh.nCount          = rectCount;
	rdh.rcBound.left    = 0;
	rdh.rcBound.top     = 0;
	rdh.rcBound.right   = bm.bmWidth;
	rdh.rcBound.bottom  = bm.bmHeight;
	HRGN hRgn = ExtCreateRegion(NULL, rdh.nRgnSize, pRgnData);
	free(pRgnData);
	return hRgn;
}

/////////////////////////////////////////////////////////////////////////////
// Execute

enum {
	NONE = 0,
	NONE_CRLF,
	DIGIT,
	DIGIT_CRLF,
	STRING,
	STRING_CRLF,
};

#define IS_NONE(ret)   (((ret) == NONE      ) || ((ret) == NONE_CRLF  ))
#define IS_DIGIT(ret)  (((ret) == DIGIT     ) || ((ret) == DIGIT_CRLF ))
#define IS_STRING(ret) (((ret) == STRING    ) || ((ret) == STRING_CRLF))
#define IS_CRLF(ret)   (((ret) == NONE_CRLF ) || ((ret) == DIGIT_CRLF ) || ((ret) == STRING_CRLF))

enum {
	MISSIONS_VR = 0,
	MISSIONS_ALTERNATIVE,
	MISSIONS_MAX,
};

enum {
	MODE_SNEAKING = 0,
	MODE_WEAPON,
	MODE_FIRST_PERSON_VIEW,
	MODE_VARIETY,
	MODE_STREAKING,
	MODE_BOMB_DISPOSAL,
	MODE_ELIMINATE,
	MODE_HOLD_UP,
	MODE_PHOTOGRAPH,
	MODE_MAX,
};

enum {
	WEAPON_SNEAKING = 0,
	WEAPON_ELIMINATE_ALL,
	WEAPON_HANDGUN,
	WEAPON_ASSAULT_RIFLE,
	WEAPON_C4_CLAYMORE,
	WEAPON_GRENADE,
	WEAPON_PSG1,
	WEAPON_SINGER,
	WEAPON_NIKITA,
	WEAPON_HF_BLADE,
	WEAPON_MAX,
};

enum {
	PLAYER_RAIDEN = 0,
	PLAYER_NINJA,
	PLAYER_SNAKE,
	PLAYER_PLISKIN,
	PLAYER_TUXEDO,
	PLAYER_PREVIOUS,
	PLAYER_MAX,
};

char _lpszMissions[MISSIONS_MAX][32] = {
	"VR MISSIONS",
	"ALTERNATIVE MISSIONS",
};

char _lpszMode[MODE_MAX][32] = {
	"SNEAKING MODE",
	"WEAPON MODE",
	"FIRST PERSON VIEW MODE",
	"VARIETY MODE",
	"STREAKING MODE",
	"BOMB DISPOSAL MODE",
	"ELIMINATE MODE",
	"HOLD UP MODE",
	"PHOTOGRAPH MODE",
};

char _lpszWeapon[WEAPON_MAX][32] = {
	"SNEAKING",
	"ELIMINATE ALL",
	"HANDGUN",
	"ASSAULT RIFLE",
	"C4/CLAYMORE",
	"GRENADE",
	"PSG-1",
	"STINGER",
	"NIKITA",
	"HF. BLADE/NO WEAPON",
};

char _lpszPlayer[PLAYER_MAX][32] = {
	"_RAI",
	"_NIN",
	"_SNA",
	"_PLI",
	"_TUX",
	"_PRE",
};

BOOL CMsnStageApp::Execute(LPCTSTR lpszPath)
{
	HANDLE hFileCsv;
	HANDLE hFileID;
	HANDLE hFileID2;
	HANDLE hFileHS;
	char   szPath[_MAX_PATH];
	char   szDrive[_MAX_DRIVE];
	char   szDir[_MAX_DIR];
	char   szFName[_MAX_FNAME];
	char   szExt[_MAX_EXT];
	char   szMessage[_MAX_PATH + 64];

	// 開始
	HCURSOR hCursor = SetCursor(LoadCursor(IDC_WAIT));

	// msn_stage.csv を開く
	if((hFileCsv = CreateFile(lpszPath, GENERIC_READ,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		SetCursor(hCursor);
		sprintf(szMessage, "%s\nファイルが開けません。", lpszPath);
		MessageBox(NULL, szMessage, "エラー", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	// msn_stage_id.h を開く
	_splitpath(lpszPath, szDrive, szDir, szFName, szExt);
	_makepath(szPath, szDrive, szDir, "msn_stage_id", ".h");
	if((hFileID = CreateFile(szPath, GENERIC_WRITE,
		0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		if((hFileID = CreateFile(szPath, GENERIC_WRITE,
			0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFileCsv);
			SetCursor(hCursor);
			sprintf(szMessage, "%s\nファイルが開けません。", szPath);
			MessageBox(NULL, szMessage, "エラー", MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	// msn_stage_id2.h を開く
	_splitpath(lpszPath, szDrive, szDir, szFName, szExt);
	_makepath(szPath, szDrive, szDir, "msn_stage_id2", ".h");
	if((hFileID2 = CreateFile(szPath, GENERIC_WRITE,
		0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		if((hFileID2 = CreateFile(szPath, GENERIC_WRITE,
			0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFileID);
			CloseHandle(hFileCsv);
			SetCursor(hCursor);
			sprintf(szMessage, "%s\nファイルが開けません。", szPath);
			MessageBox(NULL, szMessage, "エラー", MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	// msn_def_hiscore.h を開く
	_splitpath(lpszPath, szDrive, szDir, szFName, szExt);
	_makepath(szPath, szDrive, szDir, "msn_def_hiscore", ".h");
	if((hFileHS = CreateFile(szPath, GENERIC_WRITE,
		0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		if((hFileHS = CreateFile(szPath, GENERIC_WRITE,
			0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFileID2);
			CloseHandle(hFileID);
			CloseHandle(hFileCsv);
			SetCursor(hCursor);
			sprintf(szMessage, "%s\nファイルが開けません。", szPath);
			MessageBox(NULL, szMessage, "エラー", MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	// csv 解析
	{
		int  i;
		int  ret, ret2, ret3, nNum, nNum2, nNum3;
		char szBuffer[1024], szBuffer2[1024], szBuffer3[1024], szID[1024];
		int  nLineSkip[2];
		char cPlayer[MODE_MAX + WEAPON_MAX][PLAYER_MAX];
		int  nModeCount;
		int  nMissions, nMode, nWeapon;
		BOOL bNewMode;

		// ヘッダー行の取得
		while(1)
		{
			ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));
			if(IS_NONE(ret))
			{
				goto exe_error;
			}
			else if(IS_STRING(ret))
			{
				if(szBuffer[0] != '#')
					goto exe_error;

				// コメント行 － 行末までスキップ
				while(!IS_CRLF(ret))
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));
			}
			else
			{
				// コメント欄数１の取得
				if(nNum < 0)
					goto exe_error;
				nLineSkip[0] = nNum;

				// コメント欄数２の取得
				for(i = 0; i < 3 + nLineSkip[0]; i++)
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));
				if(!IS_DIGIT(ret))
					goto exe_error;
				if(nNum < 0)
					goto exe_error;
				nLineSkip[1] = nNum;

				// 行末までスキップ
				while(!IS_CRLF(ret))
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));

				break;
			}
		}
		TRACE("Get Header Succeeded.\n");

		nMissions  = -1;
		nMode      = -1;
		nWeapon    = -1;
		nModeCount = -1;

		// ファイル出力
		sprintf(szBuffer, "enum{\n");
		if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
			|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
			goto exe_error;

		// 各行の処理
		while(1)
		{
			TRACE("Scan Line - ");

			// コメント欄
			for(i = 0; i < nLineSkip[0]; i++)
				ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));

			// ３つの欄を取得
			ret  = GetNext(hFileCsv, &nNum , szBuffer , sizeof(szBuffer ));
			ret2 = GetNext(hFileCsv, &nNum2, szBuffer2, sizeof(szBuffer2));
			ret3 = GetNext(hFileCsv, &nNum3, szBuffer3, sizeof(szBuffer3));

			if(IS_DIGIT(ret) || IS_DIGIT(ret2) || IS_DIGIT(ret3))
			{
				goto exe_error;
			}
			else if(IS_NONE(ret) && IS_NONE(ret2) && IS_NONE(ret3))
			{
				////////////////////////////////////////
				// データ行
				TRACE("Data Line\n");

				// データ行
				if(    (nMissions == -1)
					|| (nMode     == -1)
					|| (nWeapon   == -1))
					goto exe_error;

				// コメント欄
				for(i = 0; i < nLineSkip[1]; i++)
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));

				// タグ取得
				ret = GetNext(hFileCsv, &nNum , szID, sizeof(szID));
				if(!IS_STRING(ret))
					goto exe_error;

				if(bNewMode == FALSE)
				{
					for(i = 0; i < PLAYER_MAX; i++)
					{
						if(cPlayer[nModeCount][i] == 1)
						{
							// ファイル出力
							sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
							if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
								|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
								goto exe_error;
						}
					}
				}
				else
				{
					// 先頭行
					if(nMissions == MISSIONS_VR)
					{
						if(nMode == MODE_SNEAKING)
						{
							if(nWeapon == WEAPON_SNEAKING)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_SNK_SNK_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_SNK_SNK_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_ELIMINATE_ALL)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_SNK_ELM_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_SNK_ELM_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else
							{
								goto exe_error;
							}
						}
						else if(nMode == MODE_WEAPON)
						{
							if(nWeapon == WEAPON_HANDGUN)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_HGN_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_HGN_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_ASSAULT_RIFLE)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_ASR_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_ASR_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_C4_CLAYMORE)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_C4C_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_C4C_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_GRENADE)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_GRN_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_GRN_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_PSG1)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_PSG_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_PSG_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_SINGER)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_STG_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_STG_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_NIKITA)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_NKT_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_NKT_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else if(nWeapon == WEAPON_HF_BLADE)
							{
								for(i = 0; i < PLAYER_MAX; i++)
								{
									if(cPlayer[nModeCount][i] == 1)
									{
										if(bNewMode == TRUE)
										{
											bNewMode = FALSE;

											// ファイル出力
											sprintf(szBuffer, "%s%s = d:VRM_WPN_HFB_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID, szBuffer) == FALSE)
												goto exe_error;
											sprintf(szBuffer, "%s%s = VRM_WPN_HFB_START,\n", szID, _lpszPlayer[i]);
											if(WriteBuffer(hFileID2, szBuffer) == FALSE)
												goto exe_error;
										}
										else
										{
											// ファイル出力
											sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
											if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
												|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
												goto exe_error;
										}
									}
								}
							}
							else
							{
								goto exe_error;
							}
						}
						else if(nMode == MODE_FIRST_PERSON_VIEW)
						{
							for(i = 0; i < PLAYER_MAX; i++)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									if(bNewMode == TRUE)
									{
										bNewMode = FALSE;

										// ファイル出力
										sprintf(szBuffer, "%s%s = d:VRM_FPV_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID, szBuffer) == FALSE)
											goto exe_error;
										sprintf(szBuffer, "%s%s = VRM_FPV_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID2, szBuffer) == FALSE)
											goto exe_error;
									}
									else
									{
										// ファイル出力
										sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
										if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
											|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
											goto exe_error;
									}
								}
							}
						}
						else if(nMode == MODE_VARIETY)
						{
							for(i = 0; i < PLAYER_MAX; i++)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									if(bNewMode == TRUE)
									{
										bNewMode = FALSE;

										// ファイル出力
										sprintf(szBuffer, "%s%s = d:VRM_VRT_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID, szBuffer) == FALSE)
											goto exe_error;
										sprintf(szBuffer, "%s%s = VRM_VRT_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID2, szBuffer) == FALSE)
											goto exe_error;
									}
									else
									{
										// ファイル出力
										sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
										if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
											|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
											goto exe_error;
									}
								}
							}
						}
						else if(nMode == MODE_STREAKING)
						{
							for(i = 0; i < PLAYER_MAX; i++)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									if(bNewMode == TRUE)
									{
										bNewMode = FALSE;

										// ファイル出力
										sprintf(szBuffer, "%s%s = d:VRM_STR_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID, szBuffer) == FALSE)
											goto exe_error;
										sprintf(szBuffer, "%s%s = VRM_STR_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID2, szBuffer) == FALSE)
											goto exe_error;
									}
									else
									{
										// ファイル出力
										sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
										if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
											|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
											goto exe_error;
									}
								}
							}
						}
						else
						{
							goto exe_error;
						}
					}
					else if(nMissions == MISSIONS_ALTERNATIVE)
					{
						if(nMode == MODE_BOMB_DISPOSAL)
						{
							for(i = 0; i < PLAYER_MAX; i++)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									if(bNewMode == TRUE)
									{
										bNewMode = FALSE;

										// ファイル出力
										sprintf(szBuffer, "%s%s = d:ALT_BMB_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID, szBuffer) == FALSE)
											goto exe_error;
										sprintf(szBuffer, "%s%s = ALT_BMB_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID2, szBuffer) == FALSE)
											goto exe_error;
									}
									else
									{
										// ファイル出力
										sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
										if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
											|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
											goto exe_error;
									}
								}
							}
						}
						else if(nMode == MODE_ELIMINATE)
						{
							for(i = 0; i < PLAYER_MAX; i++)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									if(bNewMode == TRUE)
									{
										bNewMode = FALSE;

										// ファイル出力
										sprintf(szBuffer, "%s%s = d:ALT_ELM_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID, szBuffer) == FALSE)
											goto exe_error;
										sprintf(szBuffer, "%s%s = ALT_ELM_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID2, szBuffer) == FALSE)
											goto exe_error;
									}
									else
									{
										// ファイル出力
										sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
										if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
											|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
											goto exe_error;
									}
								}
							}
						}
						else if(nMode == MODE_HOLD_UP)
						{
							for(i = 0; i < PLAYER_MAX; i++)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									if(bNewMode == TRUE)
									{
										bNewMode = FALSE;

										// ファイル出力
										sprintf(szBuffer, "%s%s = d:ALT_HLD_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID, szBuffer) == FALSE)
											goto exe_error;
										sprintf(szBuffer, "%s%s = ALT_HLD_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID2, szBuffer) == FALSE)
											goto exe_error;
									}
									else
									{
										// ファイル出力
										sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
										if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
											|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
											goto exe_error;
									}
								}
							}
						}
						else if(nMode == MODE_PHOTOGRAPH)
						{
							for(i = 0; i < PLAYER_MAX; i++)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									if(bNewMode == TRUE)
									{
										bNewMode = FALSE;

										// ファイル出力
										sprintf(szBuffer, "%s%s = d:ALT_PHT_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID, szBuffer) == FALSE)
											goto exe_error;
										sprintf(szBuffer, "%s%s = ALT_PHT_START,\n", szID, _lpszPlayer[i]);
										if(WriteBuffer(hFileID2, szBuffer) == FALSE)
											goto exe_error;
									}
									else
									{
										// ファイル出力
										sprintf(szBuffer, "%s%s,\n", szID, _lpszPlayer[i]);
										if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
											|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
											goto exe_error;
									}
								}
							}
						}
						else
						{
							goto exe_error;
						}
					}
					else
					{
						goto exe_error;
					}
				}

				// ３つの欄ずつ取得
				for(i = 0; i < PLAYER_MAX; i++)
				{
					ret  = GetNext(hFileCsv, &nNum , szBuffer , sizeof(szBuffer ));
					ret2 = GetNext(hFileCsv, &nNum2, szBuffer2, sizeof(szBuffer2));
					ret3 = GetNext(hFileCsv, &nNum3, szBuffer3, sizeof(szBuffer3));

					if(cPlayer[nModeCount][i] == 1)
					{
						sprintf(szBuffer , "MSN_HISCORE(%s%s, 1) = %6d;\n", szID, _lpszPlayer[i], nNum );
						sprintf(szBuffer2, "MSN_HISCORE(%s%s, 2) = %6d;\n", szID, _lpszPlayer[i], nNum2);
						sprintf(szBuffer3, "MSN_HISCORE(%s%s, 3) = %6d;\n", szID, _lpszPlayer[i], nNum3);

						if(    (WriteBuffer(hFileHS, szBuffer ) == FALSE)
							|| (WriteBuffer(hFileHS, szBuffer2) == FALSE)
							|| (WriteBuffer(hFileHS, szBuffer3) == FALSE))
							goto exe_error;
					}
				}

				// 行末までスキップ
				while(!(IS_CRLF(ret) || IS_CRLF(ret2) || IS_CRLF(ret3)))
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));
			}
			else if(IS_STRING(ret) && (szBuffer[0] == '#'))
			{
				TRACE("Comment Line\n");

				////////////////////////////////////////
				// コメント行 － 行末までスキップ
				while(!IS_CRLF(ret))
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));
			}
			else if(IS_STRING(ret) && (strcmp(szBuffer, "DATA_END") == 0))
			{
				TRACE("DATA_END Line\n");

				////////////////////////////////////////
				// データの終わり
				break;
			}
			else if(IS_STRING(ret) && (strcmp(szBuffer, "END") == 0))
			{
				TRACE("END Line\n");

				////////////////////////////////////////
				// データの区切り
				if(nMissions == MISSIONS_VR)
				{
					if(nMode == MODE_SNEAKING)
					{
						if(nWeapon == WEAPON_SNEAKING)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_SNK_SNK_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_SNK_SNK_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_ELIMINATE_ALL)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_SNK_ELM_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_SNK_ELM_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									// ファイル出力
									sprintf(szBuffer, "VRM_SNK_END = d:VRM_SNK_ELM_END,\n");
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_SNK_END = VRM_SNK_ELM_END,\n");
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else
						{
							goto exe_error;
						}
					}
					else if(nMode == MODE_WEAPON)
					{
						if(nWeapon == WEAPON_HANDGUN)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_HGN_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_HGN_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_ASSAULT_RIFLE)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_ASR_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_ASR_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_C4_CLAYMORE)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_C4C_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_C4C_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_GRENADE)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_GRN_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_GRN_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_PSG1)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_PSG_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_PSG_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_SINGER)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_STG_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_STG_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_NIKITA)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_NKT_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_NKT_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else if(nWeapon == WEAPON_HF_BLADE)
						{
							for(i = PLAYER_MAX - 1; i >= 0; i--)
							{
								if(cPlayer[nModeCount][i] == 1)
								{
									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_HFB_END = d:%s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_HFB_END = %s%s,\n", szID, _lpszPlayer[i]);
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									// ファイル出力
									sprintf(szBuffer, "VRM_WPN_END = d:VRM_WPN_HFB_END,\n");
									if(WriteBuffer(hFileID, szBuffer) == FALSE)
										goto exe_error;
									sprintf(szBuffer, "VRM_WPN_END = VRM_WPN_HFB_END,\n");
									if(WriteBuffer(hFileID2, szBuffer) == FALSE)
										goto exe_error;

									break;
								}
							}
						}
						else
						{
							goto exe_error;
						}
					}
					else if(nMode == MODE_FIRST_PERSON_VIEW)
					{
						for(i = PLAYER_MAX - 1; i >= 0; i--)
						{
							if(cPlayer[nModeCount][i] == 1)
							{
								// ファイル出力
								sprintf(szBuffer, "VRM_FPV_END = d:%s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "VRM_FPV_END = %s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								break;
							}
						}
					}
					else if(nMode == MODE_VARIETY)
					{
						for(i = PLAYER_MAX - 1; i >= 0; i--)
						{
							if(cPlayer[nModeCount][i] == 1)
							{
								// ファイル出力
								sprintf(szBuffer, "VRM_VRT_END = d:%s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "VRM_VRT_END = %s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								break;
							}
						}
					}
					else if(nMode == MODE_STREAKING)
					{
						for(i = PLAYER_MAX - 1; i >= 0; i--)
						{
							if(cPlayer[nModeCount][i] == 1)
							{
								// ファイル出力
								sprintf(szBuffer, "VRM_STR_END = d:%s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "VRM_STR_END = %s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								// ファイル出力
								sprintf(szBuffer, "VRM_END = d:VRM_STR_END,\n");
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "VRM_END = VRM_STR_END,\n");
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								break;
							}
						}
					}
					else
					{
						goto exe_error;
					}
				}
				else if(nMissions == MISSIONS_ALTERNATIVE)
				{
					if(nMode == MODE_BOMB_DISPOSAL)
					{
						for(i = PLAYER_MAX - 1; i >= 0; i--)
						{
							if(cPlayer[nModeCount][i] == 1)
							{
								// ファイル出力
								sprintf(szBuffer, "ALT_BMB_END = d:%s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "ALT_BMB_END = %s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								break;
							}
						}
					}
					else if(nMode == MODE_ELIMINATE)
					{
						for(i = PLAYER_MAX - 1; i >= 0; i--)
						{
							if(cPlayer[nModeCount][i] == 1)
							{
								// ファイル出力
								sprintf(szBuffer, "ALT_ELM_END = d:%s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "ALT_ELM_END = %s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								break;
							}
						}
					}
					else if(nMode == MODE_HOLD_UP)
					{
						for(i = PLAYER_MAX - 1; i >= 0; i--)
						{
							if(cPlayer[nModeCount][i] == 1)
							{
								// ファイル出力
								sprintf(szBuffer, "ALT_HLD_END = d:%s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "ALT_HLD_END = %s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								break;
							}
						}
					}
					else if(nMode == MODE_PHOTOGRAPH)
					{
						for(i = PLAYER_MAX - 1; i >= 0; i--)
						{
							if(cPlayer[nModeCount][i] == 1)
							{
								// ファイル出力
								sprintf(szBuffer, "ALT_PHT_END = d:%s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "ALT_PHT_END = %s%s,\n", szID, _lpszPlayer[i]);
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								// ファイル出力
								sprintf(szBuffer, "ALT_END = d:ALT_PHT_END,\n");
								if(WriteBuffer(hFileID, szBuffer) == FALSE)
									goto exe_error;
								sprintf(szBuffer, "ALT_END = ALT_PHT_END,\n");
								if(WriteBuffer(hFileID2, szBuffer) == FALSE)
									goto exe_error;

								break;
							}
						}
					}
					else
					{
						goto exe_error;
					}
				}
				else
				{
					goto exe_error;
				}

				// 行末までスキップ
				while(!IS_CRLF(ret))
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));
			}
			else
			{
				TRACE("New Mode Line\n");

				////////////////////////////////////////
				// 新しいモード
				bNewMode = FALSE;

				// ミッションズ
				if(IS_STRING(ret) && (strcmp(szBuffer, _lpszMissions[MISSIONS_VR]) == 0))
				{
					if(nMissions != MISSIONS_VR - 1)
						goto exe_error;
					nMissions++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_START = 0,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret) && (strcmp(szBuffer, _lpszMissions[MISSIONS_ALTERNATIVE]) == 0))
				{
					if(nMissions != MISSIONS_ALTERNATIVE - 1)
						goto exe_error;
					nMissions++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "ALT_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}

				// モード
				if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_SNEAKING]) == 0))
				{
					if(nMode != MODE_SNEAKING - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_SNK_START = d:VRM_START,\n");
					if(WriteBuffer(hFileID, szBuffer) == FALSE)
						goto exe_error;
					sprintf(szBuffer, "VRM_SNK_START = VRM_START,\n");
					if(WriteBuffer(hFileID2, szBuffer) == FALSE)
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_WEAPON]) == 0))
				{
					if(nMode != MODE_WEAPON - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_FIRST_PERSON_VIEW]) == 0))
				{
					if(nMode != MODE_FIRST_PERSON_VIEW - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_FPV_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_VARIETY]) == 0))
				{
					if(nMode != MODE_VARIETY - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_VRT_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_STREAKING]) == 0))
				{
					if(nMode != MODE_STREAKING - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_STR_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_BOMB_DISPOSAL]) == 0))
				{
					if(nMode != MODE_BOMB_DISPOSAL - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "ALT_BMB_START = d:ALT_START,\n");
					if(WriteBuffer(hFileID, szBuffer) == FALSE)
						goto exe_error;
					sprintf(szBuffer, "ALT_BMB_START = ALT_START,\n");
					if(WriteBuffer(hFileID2, szBuffer) == FALSE)
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_ELIMINATE]) == 0))
				{
					if(nMode != MODE_ELIMINATE - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "ALT_ELM_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_HOLD_UP]) == 0))
				{
					if(nMode != MODE_HOLD_UP - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "ALT_HLD_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret2) && (strcmp(szBuffer2, _lpszMode[MODE_PHOTOGRAPH]) == 0))
				{
					if(nMode != MODE_PHOTOGRAPH - 1)
						goto exe_error;
					nMode++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "ALT_PHT_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}

				// 武器
				if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_SNEAKING]) == 0))
				{
					if(nWeapon != WEAPON_SNEAKING - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_SNK_SNK_START = d:VRM_SNK_START,\n");
					if(WriteBuffer(hFileID, szBuffer) == FALSE)
						goto exe_error;
					sprintf(szBuffer, "VRM_SNK_SNK_START = VRM_SNK_START,\n");
					if(WriteBuffer(hFileID2, szBuffer) == FALSE)
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_ELIMINATE_ALL]) == 0))
				{
					if(nWeapon != WEAPON_ELIMINATE_ALL - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_SNK_ELM_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_HANDGUN]) == 0))
				{
					if(nWeapon != WEAPON_HANDGUN - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_HGN_START = d:VRM_WPN_START,\n");
					if(WriteBuffer(hFileID, szBuffer) == FALSE)
						goto exe_error;
					sprintf(szBuffer, "VRM_WPN_HGN_START = VRM_WPN_START,\n");
					if(WriteBuffer(hFileID2, szBuffer) == FALSE)
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_ASSAULT_RIFLE]) == 0))
				{
					if(nWeapon != WEAPON_ASSAULT_RIFLE - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_ASR_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_C4_CLAYMORE]) == 0))
				{
					if(nWeapon != WEAPON_C4_CLAYMORE - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_C4C_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_GRENADE]) == 0))
				{
					if(nWeapon != WEAPON_GRENADE - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_GRN_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_PSG1]) == 0))
				{
					if(nWeapon != WEAPON_PSG1 - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_PSG_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_SINGER]) == 0))
				{
					if(nWeapon != WEAPON_SINGER - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_STG_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_NIKITA]) == 0))
				{
					if(nWeapon != WEAPON_NIKITA - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_NKT_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}
				else if(IS_STRING(ret3) && (strcmp(szBuffer3, _lpszWeapon[WEAPON_HF_BLADE]) == 0))
				{
					if(nWeapon != WEAPON_HF_BLADE - 1)
						goto exe_error;
					nWeapon++;
					bNewMode = TRUE;

					// ファイル出力
					sprintf(szBuffer, "VRM_WPN_HFB_START,\n");
					if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
						|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
						goto exe_error;
				}

				if(    (bNewMode  == FALSE)
					|| (nMissions == -1   )
					|| (nMode     == -1   )
					|| (nWeapon   == -1   ))
					goto exe_error;

				// コメント欄
				for(i = 0; i < nLineSkip[1]; i++)
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));

				// 読み飛ばし
				ret  = GetNext(hFileCsv, &nNum , szBuffer , sizeof(szBuffer ));

				nModeCount++;

				// ３つの欄ずつ取得
				for(i = 0; i < PLAYER_MAX; i++)
				{
					ret  = GetNext(hFileCsv, &nNum , szBuffer , sizeof(szBuffer ));
					ret2 = GetNext(hFileCsv, &nNum2, szBuffer2, sizeof(szBuffer2));
					ret3 = GetNext(hFileCsv, &nNum3, szBuffer3, sizeof(szBuffer3));

					if(IS_DIGIT(ret) && (nNum == 1)) cPlayer[nModeCount][i] = 1;
					else                             cPlayer[nModeCount][i] = 0;
				}

				// 行末までスキップ
				while(!(IS_CRLF(ret) || IS_CRLF(ret2) || IS_CRLF(ret3)))
					ret = GetNext(hFileCsv, &nNum, szBuffer, sizeof(szBuffer));
			}
		}	// 各行の処理 終了

		// ファイル出力
		sprintf(szBuffer, "MSN_STAGE_ID_MAX\n};\n");
		if(    (WriteBuffer(hFileID , szBuffer) == FALSE)
			|| (WriteBuffer(hFileID2, szBuffer) == FALSE))
			goto exe_error;

		// プレイヤー定義
		sprintf(szBuffer, "\n");                                               if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_RAIDEN   (0x00000001)\n");       if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_NINJA    (0x00000002)\n");       if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_SNAKE    (0x00000004)\n");       if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_PLISKIN  (0x00000008)\n");       if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_TUXEDO   (0x00000010)\n");       if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_PREVIOUS (0x00000020)\n");       if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_ALL      (0x0000003f)\n");       if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "\n");                                               if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define MSN_PLAYER_NUM(player) (\\\n");             if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "   ((player) & MSN_PLAYER_RAIDEN  )       + \\\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "  (((player) & MSN_PLAYER_NINJA   ) >> 1) + \\\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "  (((player) & MSN_PLAYER_SNAKE   ) >> 2) + \\\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "  (((player) & MSN_PLAYER_PLISKIN ) >> 3) + \\\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "  (((player) & MSN_PLAYER_TUXEDO  ) >> 4) + \\\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "  (((player) & MSN_PLAYER_PREVIOUS) >> 5)   \\\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "  )\n");                                            if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "\n");                                               if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;

		for(i = 0; i <= nModeCount; i++)
		{
			switch(i)
			{
			case  0: sprintf(szBuffer, "#define VRM_SNK_SNK_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  1: sprintf(szBuffer, "#define VRM_SNK_ELM_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  2: sprintf(szBuffer, "#define VRM_WPN_HGN_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  3: sprintf(szBuffer, "#define VRM_WPN_ASR_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  4: sprintf(szBuffer, "#define VRM_WPN_C4C_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  5: sprintf(szBuffer, "#define VRM_WPN_GRN_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  6: sprintf(szBuffer, "#define VRM_WPN_PSG_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  7: sprintf(szBuffer, "#define VRM_WPN_STG_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  8: sprintf(szBuffer, "#define VRM_WPN_NKT_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case  9: sprintf(szBuffer, "#define VRM_WPN_HFB_PLAYER (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case 10: sprintf(szBuffer, "#define VRM_FPV_PLAYER     (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case 11: sprintf(szBuffer, "#define VRM_VRT_PLAYER     (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case 12: sprintf(szBuffer, "#define VRM_STR_PLAYER     (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case 13: sprintf(szBuffer, "#define ALT_BMB_PLAYER     (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case 14: sprintf(szBuffer, "#define ALT_ELM_PLAYER     (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case 15: sprintf(szBuffer, "#define ALT_HLD_PLAYER     (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			case 16: sprintf(szBuffer, "#define ALT_PHT_PLAYER     (0x00"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; break;
			default: goto exe_error;
			}
			if(cPlayer[i][PLAYER_RAIDEN  ] == 1) { sprintf(szBuffer, "|MSN_PLAYER_RAIDEN"  ); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; }
			if(cPlayer[i][PLAYER_NINJA   ] == 1) { sprintf(szBuffer, "|MSN_PLAYER_NINJA"   ); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; }
			if(cPlayer[i][PLAYER_SNAKE   ] == 1) { sprintf(szBuffer, "|MSN_PLAYER_SNAKE"   ); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; }
			if(cPlayer[i][PLAYER_PLISKIN ] == 1) { sprintf(szBuffer, "|MSN_PLAYER_PLISKIN" ); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; }
			if(cPlayer[i][PLAYER_TUXEDO  ] == 1) { sprintf(szBuffer, "|MSN_PLAYER_TUXEDO"  ); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; }
			if(cPlayer[i][PLAYER_PREVIOUS] == 1) { sprintf(szBuffer, "|MSN_PLAYER_PREVIOUS"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error; }
			sprintf(szBuffer, ")\n");                                                         if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		}

		sprintf(szBuffer, "\n");                                                                    if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_SNK_SNK_PLAYER_NUM (MSN_PLAYER_NUM(VRM_SNK_SNK_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_SNK_ELM_PLAYER_NUM (MSN_PLAYER_NUM(VRM_SNK_ELM_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_HGN_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_HGN_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_ASR_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_ASR_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_C4C_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_C4C_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_GRN_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_GRN_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_PSG_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_PSG_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_STG_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_STG_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_NKT_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_NKT_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_WPN_HFB_PLAYER_NUM (MSN_PLAYER_NUM(VRM_WPN_HFB_PLAYER))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_FPV_PLAYER_NUM     (MSN_PLAYER_NUM(VRM_FPV_PLAYER    ))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_VRT_PLAYER_NUM     (MSN_PLAYER_NUM(VRM_VRT_PLAYER    ))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define VRM_STR_PLAYER_NUM     (MSN_PLAYER_NUM(VRM_STR_PLAYER    ))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define ALT_BMB_PLAYER_NUM     (MSN_PLAYER_NUM(ALT_BMB_PLAYER    ))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define ALT_ELM_PLAYER_NUM     (MSN_PLAYER_NUM(ALT_ELM_PLAYER    ))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define ALT_HLD_PLAYER_NUM     (MSN_PLAYER_NUM(ALT_HLD_PLAYER    ))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;
		sprintf(szBuffer, "#define ALT_PHT_PLAYER_NUM     (MSN_PLAYER_NUM(ALT_PHT_PLAYER    ))\n"); if(WriteBuffer(hFileID2, szBuffer) == FALSE) goto exe_error;

	}	// csv 解析 終了

	// ファイルを閉じる
	CloseHandle(hFileHS);
	CloseHandle(hFileID2);
	CloseHandle(hFileID);
	CloseHandle(hFileCsv);

	// 終了
	SetCursor(hCursor);
	sprintf(szMessage, "%s\n処理に成功しました。", lpszPath);
	MessageBox(NULL, szMessage, "結果", MB_OK|MB_ICONINFORMATION);
	return TRUE;

exe_error:
	// ファイルを閉じる
	CloseHandle(hFileHS);
	CloseHandle(hFileID2);
	CloseHandle(hFileID);
	CloseHandle(hFileCsv);

	// 終了
	SetCursor(hCursor);
	sprintf(szMessage, "%s\n処理に失敗しました。", lpszPath);
	MessageBox(NULL, szMessage, "結果", MB_OK|MB_ICONINFORMATION);
	return FALSE;
}

int CMsnStageApp::GetNext(	// NONE       : 何もない
							// NONE_CRLF  : 何もない＋行末
							// DIGIT      : 数値
							// DIGIT_CRLF : 数値＋行末
							// STRING     : 文字列
							// STRING_CRLF: 文字列＋行末
	HANDLE hFile,
	int    *lpnNum,
	char   *lpStr,
	int    nStrSize)
{
	char  szBuffer[1024];
	int   n;
	BOOL  bLineEnd;

	// 次の "," か行末まで読み込み
	memset(szBuffer, 0x00, sizeof(szBuffer));
	n        = 0;
	bLineEnd = FALSE;
	do
	{
		DWORD dwBytes;

		if(n > 1023)
			ASSERT(0);

		// 一文字読み込み
		if(    (ReadFile(hFile, &szBuffer[n], 1, &dwBytes, NULL) == FALSE)
			|| (dwBytes != 1))
			ASSERT(0);
		n++;

		if(szBuffer[n - 1] == 0x0d)
		{
			// 一文字飛ばす
			SetFilePointer(hFile, 1, 0, FILE_CURRENT);
			bLineEnd = TRUE;
			break;
		}
	}
	while(szBuffer[n - 1] != ',');

	// 最後の文字を消す
	n--;
	szBuffer[n] = '\0';

	if(n == 0)
	{
		// 何もない
		if(bLineEnd == FALSE)
			return NONE;
		else
			return NONE_CRLF;
	}
	else if(isdigit(szBuffer[0]) != 0)
	{
		// 数値
		sscanf(szBuffer, "%d", lpnNum);

		if(bLineEnd == FALSE)
			return DIGIT;
		else
			return DIGIT_CRLF;
	}
	else
	{
		// 文字列
		memset(lpStr, 0x00, nStrSize);
		strcpy(lpStr, szBuffer);

		if(bLineEnd == FALSE)
			return STRING;
		else
			return STRING_CRLF;
	}
}

BOOL CMsnStageApp::WriteBuffer(HANDLE hFile, LPCTSTR lpszBuffer)
{
	DWORD dwBytes;

	if(    (WriteFile(hFile, lpszBuffer, strlen(lpszBuffer), &dwBytes, NULL) == FALSE)
		|| (dwBytes != strlen(lpszBuffer)))
		return FALSE;

	return TRUE;
}
