// MsnStageDlg.cpp : インプリメンテーション ファイル
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
// アプリケーションのバージョン情報で使われている CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// メッセージ ハンドラがありません。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsnStageDlg ダイアログ

CMsnStageDlg::CMsnStageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMsnStageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsnStageDlg)
		// メモ: この位置に ClassWizard によってメンバの初期化が追加されます。
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_hRgnWnd = NULL;
}

void CMsnStageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsnStageDlg)
		// メモ: この場所には ClassWizard によって DDX と DDV の呼び出しが追加されます。
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMsnStageDlg, CDialog)
	//{{AFX_MSG_MAP(CMsnStageDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsnStageDlg メッセージ ハンドラ

BOOL CMsnStageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "バージョン情報..." メニュー項目をシステム メニューへ追加します。

	// IDM_ABOUTBOX はコマンド メニューの範囲でなければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンを設定
	
	// TODO: 特別な初期化を行う時はこの場所に追加してください。
	
	// ドラッグアンドドロップを許可
	DragAcceptFiles(TRUE);

	// アイコンからリージョンの作成
	ICONINFO iconinfo;
	HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 32, 32, 0); 
	GetIconInfo(hIcon, &iconinfo);
	HRGN hRgn1 = lpApp->CreateRgnFromBitmap(iconinfo.hbmColor);
	// ウィンドウリージョンの作成
	RECT rcWnd;
	GetWindowRect(&rcWnd);
	HRGN hRgn2 = CreateRectRgn(
		rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);
	OffsetRgn(hRgn1,
		((rcWnd.right - rcWnd.left) * 34) / 40,
		((rcWnd.bottom - rcWnd.top) * 17) / 20);
	m_hRgnWnd = CreateRectRgn(0, 0, 1, 1);
	CombineRgn(m_hRgnWnd, hRgn2, hRgn1, RGN_DIFF);
	DeleteObject(hRgn1);
	DeleteObject(hRgn2);
	// ウィンドウリージョンの設定
	SetWindowRgn(m_hRgnWnd, TRUE);

	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

void CMsnStageDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void CMsnStageDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CMsnStageDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMsnStageDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT i, unCount;
	char szPath[_MAX_PATH];

	unCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	for(i = 0; i < unCount; i++)
	{
		memset(szPath, 0x00, sizeof(szPath));
		DragQueryFile(hDropInfo, i, szPath, sizeof(szPath));
		lpApp->Execute(szPath);
	}
}

void CMsnStageDlg::OnButton1() 
{
	char szFileList[_MAX_PATH + (_MAX_FNAME + _MAX_EXT) * 99];
	memset(szFileList, 0x00, sizeof(szFileList));

	// 開くダイアログ
	CFileDialog fileDlg(
		TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT,
		"All Files (*.*)|*.*||", this);
	fileDlg.m_ofn.lpstrTitle = "msn_stage.csv ファイルを開く";
	fileDlg.m_ofn.lpstrFile  = szFileList;
	fileDlg.m_ofn.nMaxFile   = sizeof(szFileList);

	if(fileDlg.DoModal() == IDOK)
	{
		POSITION pos;
		CString  str;

		UpdateWindow();
		pos = fileDlg.GetStartPosition();
		while(pos != NULL)
		{
			str = fileDlg.GetNextPathName(pos);
			lpApp->Execute(str);
		}
	}
}
