/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Kelnel
 */

/******************************************************************************
 * included
 */

#include <stdio.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include "Scene2HZX.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */

/******************************************************************************
 * メイン
 */
int main(	/* 0以外	正常 */
			/* 0 */
	int   argc,		/* 引数数 */
	char* argv[])	/* 引数 */
{
    SCENE2HZXINFO   scene2HZXInfo;	
    SCENE2HZXHEADER scene2HZXHeader;

		/* 初期化 */
#ifdef _WINDOWS
	AttachSfdWin((HINSTANCE)GetModuleHandle("Scene2HZX.exe"));
#endif
#ifdef _UNIX
	AttachSfdUnix();
#endif
#ifdef _LINUX
	AttachSfdLinux();
#endif

	/* コマンドライン解析 */
	if(AnalyzeCommand(argc, argv, &scene2HZXInfo) == 0)
		return 0;

	/* シーンからオブジェクトを取り出す */
	if(GetHZXObject(&scene2HZXInfo, &scene2HZXHeader) == 0)
	{
		DestroyScene2HZXObject(&scene2HZXHeader);
		return 0;
	}

	/* ブロック分割を行う */
	if(MakeHZXBlockObject(&scene2HZXHeader) == 0)
	{
		DestroyScene2HZXObject(&scene2HZXHeader);
		return 0;
	}

	/* ここに HZX への処理を入れてください */
	



	/* 作成したオブジェクトを破棄する */
	DestroyScene2HZXObject(&scene2HZXHeader);

	/* 終了 */
#ifdef _WINDOWS
	DetachSfdWin();
#endif
#ifdef _UNIX
	DetachSfdUnix();
#endif
#ifdef _LINUX
	DetachSfdLinux();
#endif
	return 1;
}







