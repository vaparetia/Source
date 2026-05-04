// MsnStageDlg.h : ヘッダー ファイル
//

#if !defined(AFX_MSNSTAGEDLG_H__BF1159B6_15F1_4A11_BFE1_987C16D3F3B3__INCLUDED_)
#define AFX_MSNSTAGEDLG_H__BF1159B6_15F1_4A11_BFE1_987C16D3F3B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMsnStageDlg ダイアログ

class CMsnStageDlg : public CDialog
{
// 構築
public:
	CMsnStageDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CMsnStageDlg)
	enum { IDD = IDD_MSNSTAGE_DIALOG };
		// メモ: この位置に ClassWizard によってデータ メンバが追加されます。
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMsnStageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CMsnStageDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	//}}AFX_MSG
	afx_msg void OnDropFiles(HDROP hDropInfo);
	DECLARE_MESSAGE_MAP()
private:
	HRGN m_hRgnWnd;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MSNSTAGEDLG_H__BF1159B6_15F1_4A11_BFE1_987C16D3F3B3__INCLUDED_)
