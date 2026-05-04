/******************************************************************************
 * System	: Merge Scene exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Kelnel
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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

#include "Kernel.h"
#include "AnalyzeCommand.h"
#include "PrintHelpMenu.h"
#include "EditGclFile.h"
#include "EditInitGcl.h"
#include "EditMakeFile.h"
#include "RunCommand.h"
#include "TargetList.h"

/******************************************************************************
 * defines
 */

/******************************************************************************
 * functions
 */
static void InitDemoAct();
static void EndDemoAct(char* lpszCurrentDirectory);
static int CheckDirectory(char* lpszDirectoryPath);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * 引数のチェックとディレクトリ・ファイルの有無を調べ、makeコマンドを実行します
 */
/******************************************************************************
 * メイン
 */
int main(	/* 0 正常 */
			/* 0 以外、異常 */
	int   argc,		/* 引数数 */
	char* argv[])	/* 引数 */
{
	int   i;
	int   nLength;
	FILE* fFile;
	char  szMGS2Directory[_MAX_TEXT];
	char  szCDRomImgDirectory[_MAX_TEXT];
	char  szStageDirectory[_MAX_TEXT];
	char  szScnDirectory[_MAX_TEXT];
	char  szListDirectory[_MAX_TEXT];
	char  szCurrentDirectory[_MAX_TEXT];
	char  szLockFilePath[_MAX_TEXT] = { "/usr/local/develop/demo/._LOCKFILE" };

	int   nCommand;		/* 実行コマンド */
	char  szTarget[_MAX_TEXT];		/* 作業ディレクトリ */
	char  szDemoFile[_MAX_TEXT];	/* 実行するデモファイル */
	char  szDrawString[_MAX_TEXT];	/* デモ中に表示する文字列 */
	char  szViewCount[_MAX_TEXT];	/* エフェクト表示時間のテキスト */
	char  szViewStartMode[_MAX_TEXT];	/* デモ表示初期状態設定のテキスト */
	char  szFrameBaseTime[_MAX_TEXT];	/* 30フレームカウントのベースタイム */
	char  szStartTime[_MAX_TEXT];	/* 同期を取るためのタイミングカウンタ */
	char  szString[_MAX_TEXT];		/* 汎用 */
	char  szString2[_MAX_TEXT];		/* 汎用 */

	/* 初期化 */
	InitDemoAct();

	szCurrentDirectory[0] = '\0';
	nCommand              = 0;
	szTarget[0]           = '\0';
	szDemoFile[0]         = '\0';
	szDrawString[0]       = '\0';
	szViewCount[0]        = '\0';
	szViewStartMode[0]    = '\0';
	szFrameBaseTime[0]    = '\0';
	szStartTime[0]        = '\0';
	szString[0]           = '\0';

	/* 現在のカレントディレクトリを得る */
	if(getcwd(szMGS2Directory, _MAX_TEXT) == NULL)
		return 1;

	nLength = strlen(szMGS2Directory);
	if(strcmp(&szMGS2Directory[nLength - 4], "mgs2") != 0)
		printf("Please run on mgs2/source directory.\n");

	/* szMGS2Directory には mgs2 ディレクトリのパスが入っている */
	szMGS2Directory[nLength] = '\0';
	/* 各種ディレクトリの作成 */
	/* CDRomImgパス */
	sprintf(szCDRomImgDirectory, "%s/cdrom.img", szMGS2Directory);
	/* ステージパス */
	sprintf(szStageDirectory, "%s/cdrom.img/stage", szMGS2Directory);
	/* シナリオパス */
	sprintf(szScnDirectory, "%s/scn", szMGS2Directory);
	/* srcパス */
	sprintf(szListDirectory, "%s/list", szMGS2Directory);

	/* 環境設定 */
	setenv("MGS2_DEMO", "1", 1);
	setenv("DEMO_DATA", szCDRomImgDirectory, 1);
	setenv("MGS2_SCN", "2", 1);

	/* 引数解析 */
	if(AnalyzeCommand(argc, argv, &nCommand, szTarget, szDemoFile,
		szDrawString, szViewCount, szViewStartMode, szFrameBaseTime,
		szStartTime) == 0)
		return 1;

	if(szTarget[0] != '\0')
	{
		/* Target DemoFile チェック */

		/* ../cdrom.img/stage/$$$ があるかどうか */
		sprintf(szString, "%s/%s", szStageDirectory, szTarget);
		if(CheckDirectory(szString) == 0)
		{
			printf("Error! No target directory.(%s)\n", szString);
			return 1;
		}

		/* ../cdrom.img/stage/$$$/$$$.de2 があるかどうか */
		sprintf(szString, "%s/%s/%s.de2", szStageDirectory, szTarget, szDemoFile);
		if(CheckFile(szString) == 0)
		{
			printf("Error Not found de2 file.(%s)\n", szString);
			return 1;
		}

		/* ../cdrom.img/stage/$$$/$$$.lst があるかどうか */
		sprintf(szString, "%s/%s/%s.lst", szStageDirectory, szTarget, szDemoFile);
		if(CheckFile(szString) == 0)
		{
			printf("Error Not found lst file.(%s)\n", szString);
			return 1;
		}

		/* ../src があるかどうか */
		sprintf(szString, "%s/scn", szMGS2Directory);
		if(CheckDirectory(szString) == 0)
		{
			printf("Error! No src directory.(%s)\n", szString);
			return 1;
		}

		/* ../src/init.gcl があるかどうか */
		sprintf(szString, "%s/init.gcl", szScnDirectory);
		if(CheckFile(szString) == 0)
		{
			printf("Error Not found gcl file.(%s)\n", szString);
			return 1;
		}

		/* ../src/$$$.gcl があるかどうか */
		sprintf(szString, "%s/%s.gcl", szScnDirectory, szTarget);
		if(CheckFile(szString) == 0)
		{
			printf("Error Not found gcl file.(%s)\n", szString);
			return 1;
		}

		/* init.gcl ファイルを編集する */
		sprintf(szString, "%s/init.gcl", szScnDirectory);
		if(EditInitGcl(szString, szTarget) == 0)
			return 1;

		/* gcl ファイルを編集する */
		sprintf(szString, "%s/%s.gcl", szScnDirectory, szTarget);
		if(EditGclFile(szString, szTarget, szDemoFile, nCommand,
			szDrawString, szViewCount, szViewStartMode, szFrameBaseTime,
			szStartTime) == 0)
			return 1;

		/* src ファイルを編集する */
		sprintf(szString, "%s/%s/%s.lst", szStageDirectory,
			szTarget, szDemoFile);
		sprintf(szString2, "%s/%s.src", szListDirectory, szTarget);
		if(EditSrcFile(szString, szString2, szTarget, szListDirectory) == 0)
		{
			printf("Error Error Error!\n");
			
			return 1;
		}
	}

	/* 第二引数のチェック */
	if(nCommand == 0)
	{
		printf("Error No Command.\n");
		PrintHelpMenu();
		return 1;
	}

	/* ロックファイルチェック */
	if(CheckFile(szLockFilePath) != 0) {
		printf("making system data, Now!\n");
		printf("Sorry, please wait.\n");
		return 1;
	}

	/* コマンドを実行 */
	if(nCommand & MGDEMO_HELP)
	{
		PrintHelpMenu();
		return 1;
	}
	else if(nCommand & MGDEMO_GO)
	{
		if(RunCommand(MAKE_TYPE_GO, szTarget) == 0)
			return 1;
	}
	else if( (nCommand & MGDEMO_DEMO) || (nCommand & MGDEMO_LOOP))
	{
		if(RunCommand(MAKE_TYPE_ALL_ONE_STAGE, szTarget) == 0)
			return 1;
	}
	else
	{
		printf("Error! No Command.\n");
		PrintHelpMenu();
		return 1;
	}

	/* 終了処理：EndDemoAct内でExitしています。 */
	EndDemoAct(szCurrentDirectory);
	return 0;
}

static void InitDemoAct()
{
#ifdef _WINDOWS
	AttachSfdWin((HINSTANCE)GetModuleHandle("DemoAct.exe"));
#endif
#ifdef _UNIX
	AttachSfdUnix("dact");
#endif
#ifdef _LINUX
	AttachSfdLinux("dact");
#endif
}

static void EndDemoAct(
	char* lpszCurrentDirectory)
{
#ifdef _WINDOWS
	DetachSfdWin();
#endif
#ifdef _UNIX
	DetachSfdUnix();
#endif
#ifdef _LINUX
	DetachSfdLinux();
#endif

	exit(0);
}

/******************************************************************************
 * 指定されたファイルがあるかどうか調べる
 */
int CheckFile(	/* 0:存在しない */
						/* 1:存在する */
	char* lpszFilePath)
{
	FILE* fFile;

	/* ファイルのオープンを試みる */
	if((fFile = fopen(lpszFilePath, "r")) == NULL)
		return 0;

	fclose(fFile);

	return 1;
}

/******************************************************************************
 * 指定されたディレクトリがあるかどうか調べる
 */
static int CheckDirectory(	/* 0:存在しない */
							/* 1:存在する */
	char* lpszDirectoryPath)
{
	DIR* lpDir;

	/* ディレクトリのオープンを試みる */
	if((lpDir = opendir(lpszDirectoryPath)) == NULL)
		return 0;

	closedir(lpDir);
	return 1;
}



