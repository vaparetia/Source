// MsnStage.h : MSNSTAGE アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_MSNSTAGE_H__375CF0CB_0F00_4FB0_9D3F_903D26EE011E__INCLUDED_)
#define AFX_MSNSTAGE_H__375CF0CB_0F00_4FB0_9D3F_903D26EE011E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CMsnStageApp:
// このクラスの動作の定義に関しては MsnStage.cpp ファイルを参照してください。
//
#define lpApp ((CMsnStageApp*)AfxGetApp())

class CMsnStageApp : public CWinApp
{
public:
	BOOL Execute(LPCTSTR lpszPath);
	HRGN CreateRgnFromBitmap(HBITMAP hBitmap);
	CMsnStageApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMsnStageApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CMsnStageApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL WriteBuffer(HANDLE hFile, LPCTSTR lpszBuffer);
	int GetNext(HANDLE hFile, int* lpnNum, char* lpStr, int nStrSize);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MSNSTAGE_H__375CF0CB_0F00_4FB0_9D3F_903D26EE011E__INCLUDED_)
