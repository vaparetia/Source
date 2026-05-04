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
#include "SearchDemoHeader.h"
#include "EditInitGcl.h"
#include "RunCommand.h"
#include "TargetList.h"
#include "CreateScrFile.h"
#include "GetResident.h"

/******************************************************************************
 * defines
 */

/******************************************************************************
 * functions
 */
static void InitDemoAct();
static void EndDemoAct(char* lpszCurrentDirectory);

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
	int   nResult;
	int   nLength;
	FILE* fFile;
	char  szMGS2Directory[_MAX_TEXT];
	char  szCDRomImgDirectory[_MAX_TEXT];
	char  szStageDirectory[_MAX_TEXT];
	char  szScnDirectory[_MAX_TEXT];
	char  szListDirectory[_MAX_TEXT];
	char  szCurrentDirectory[_MAX_TEXT];
	char  szLockFilePath[_MAX_TEXT] = { "/usr/local/develop/demo/._LOCKFILE" };

	int   nCommand;						/* 実行コマンド */
	char  szTarget[_MAX_TEXT];			/* 作業ディレクトリ */
	char  szDemoFile[_MAX_TEXT];		/* 実行するデモファイル */
	char  szDrawString[_MAX_TEXT];		/* デモ中に表示する文字列 */
	char  szViewCount[_MAX_TEXT];		/* エフェクト表示時間のテキスト */
	char  szViewStartMode[_MAX_TEXT];	/* デモ表示初期状態設定のテキスト */
	char  szFrameBaseTime[_MAX_TEXT];	/* 30フレームカウントのベースタイム */
	char  szStartTime[_MAX_TEXT];		/* 同期を取るためのタイミングカウンタ */
	int   nPCMStream;					/* PCM ストリーミングの有無 */
	char  szResident[_MAX_TEXT];		/* 常駐ステージ名 */
	char  szAddGclFile[_MAX_TEXT];		/* 追加検索ステージ名 */

	int   nPCMFlag;

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
	nPCMStream            = 1;
	szResident[0]         = '\0';
	szAddGclFile[0]       = '\0';
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
		szStartTime, &nPCMStream, szAddGclFile) == 0)
		return 1;

	if(nCommand & MGDEMO_MAKE_HD_MODULE)
	{
		/* ハードディスク用データの作成 */
		sprintf(szString, "%s/_LOCK", szCDRomImgDirectory);
		if(CheckFile(szString, 0) == 0)
		{
			/* ロックファイルは無かった */
			printf("\n Make HD data!");
			sprintf(szString, "echo \"Make HD Data!\" > %s/_LOCK", szCDRomImgDirectory);
			if(system(szString) != 0)
				return 1;
		}
		else
		{
			/* ロックファイルがあった */
			printf("\n Make HD data!");
		}
	}

	if(nCommand & MGDEMO_HD_FORMAT)
	{
		/* ハードディスクフォーマットを行うか */
		printf("\n Set HD format!\n\n");
	}
	else
	{
		printf("\n\n");
	}

	if(szTarget[0] != '\0')
	{
		/* ディレクトリチェック */
		/* ../cdrom.img/stage/$$$ があるかどうか */
		sprintf(szString, "%s/%s", szStageDirectory, szTarget);
		if(CheckDirectory(szString, 1) == 0)
			return 1;

		/* ../scn があるかどうか */
		sprintf(szString, "%s/scn", szMGS2Directory);
		if(CheckDirectory(szString, 1) == 0)
			return 1;

		/* ../list があるかどうか */
		sprintf(szString, "%s/list", szMGS2Directory);
		if(CheckDirectory(szString, 1) == 0)
			return 1;

		/* ../list/demolist があるかどうか */
		sprintf(szString, "%s/list/demolist", szMGS2Directory);
		if(CheckDirectory(szString, 0) == 0)
		{
			/* demolist はなければ作る */
			sprintf(szString, "mkdir %s/list/demolist", szMGS2Directory);
			if(system(szString) != 0)
				return 1;
		}

		/* ../list/demoscr があるかどうか */
		sprintf(szString, "%s/list/demoscr", szMGS2Directory);
		if(CheckDirectory(szString, 0) == 0)
		{
			/* demoscr はなければ作る */
			sprintf(szString, "mkdir %s/list/demoscr", szMGS2Directory);
			if(system(szString) != 0)
				return 1;
		}

		/* ファイルチェック */
		/* ../cdrom.img/stage/$$$/$$$.de2 があるかどうか */
		sprintf(szString, "%s/%s/%s.de2", szStageDirectory, szTarget, szDemoFile);
		if(CheckFile(szString, 1) == 0)
			return 1;

		/* ../cdrom.img/stage/$$$/$$$.lst があるかどうか */
		sprintf(szString, "%s/%s/%s.lst", szStageDirectory, szTarget, szDemoFile);
		if(CheckFile(szString, 1) == 0)
			return 1;

		/* ../cdrom.img/stage/$$$/$$$.pcm があるかどうか */
		sprintf(szString, "%s/%s/%s.pcm", szStageDirectory, szTarget, szDemoFile);
		if(CheckFile(szString, 0) == 0) nPCMFlag = 0;
		else {
			if(nPCMStream == 0)	nPCMFlag = 0;
			else				nPCMFlag = 1;
		}

		/* ../scn/init.gcl があるかどうか */
		sprintf(szString, "%s/init.gcl", szScnDirectory);
		if(CheckFile(szString, 1) == 0)
			return 1;

		/* ../scn/$$$.gcl があるかどうか */
		sprintf(szString, "%s/%s.gcl", szScnDirectory, szTarget);
		if(CheckFile(szString, 1) == 0)
			return 1;

		/* ../list/$$$.src があるかどうか */
		sprintf(szString, "%s/%s.src", szListDirectory, szTarget);
		if(CheckFile(szString, 1) == 0)
			return 1;

		/* 常駐データチェック */
		sprintf(szString, "%s/%s.src", szListDirectory, szTarget);
		if(GetResident(szString, szResident) == 0)
			return 1;

		/* 常駐データに関する情報を出力 */
		if(szResident[0] == '\0')
		{
			printf("No set resident.(%s)\n", szString);
			printf("mgdemo set \"r_tnk0\".\n");
			sprintf(szResident, "r_tnk0\0");
		}
		else
		{
			printf("Set resident \"%s\".\n", szResident);
		}

		/* 環境設定 */
		setenv("MGS2RESIDENT", szResident, 1);

		/* init.gcl ファイルを編集する */
		if(nCommand & MGDEMO_MAKE_HD_MODULE)
		{
			/* ハードディスク用のステージを指定(固定) */
			sprintf(szString, "%s/init.gcl", szScnDirectory);
			if(EditInitGcl(szString, DEMO_HD_STAGE, szResident) == 0)
				return 1;
		}
		else
		{
			sprintf(szString, "%s/init.gcl", szScnDirectory);
			if(EditInitGcl(szString, szTarget, szResident) == 0)
				return 1;
		}

		/* gcl ファイルを編集する */
		sprintf(szString, "%s/%s.gcl", szScnDirectory, szTarget);
		if(EditGclFile(szString, szTarget, szDemoFile, nCommand,
			szDrawString, szViewCount, szViewStartMode, szFrameBaseTime,
			szStartTime) == 0)
			return 1;

		/* gcl ファイル内にデモ用のヘッダーファイルがあった場合はそちらも編集対象にする。
		   またヘッダーファイルは gcl ファイルと同じ階層にあるものとする。
		   デモ用ヘッダーファイルは "polydemo_*****.h"という命名規則にしたがっている */
		sprintf(szString, "%s/%s.gcl", szScnDirectory, szTarget);
		if(SearchDemoHeader(szString, szScnDirectory) != 0)
		{
			printf("\nFound Demo header file! \n\t\"%s\"\n", szString);

			/* ヘッダーファイルが存在した */
			/* szStringにはヘッダーファイルのパスが格納されている */
			if(EditGclFile(szString, szTarget, szDemoFile, nCommand,
				szDrawString, szViewCount, szViewStartMode, szFrameBaseTime,
				szStartTime) == 0)
				return 1;
		}

		/* 追加ファイルが指定されていたときは、それも編集対象にする */
		if(strlen(szAddGclFile) != 0)
		{
			/* gcl ファイルを編集する */
			printf("Add gcl %s\n", szAddGclFile);
			sprintf(szString, "%s/%s", szScnDirectory, szAddGclFile);
			if(EditGclFile(szString, szTarget, szDemoFile, nCommand,
				szDrawString, szViewCount, szViewStartMode, szFrameBaseTime,
				szStartTime) == 0)
				return 1;
		}

		/* リストファイルをcdrom.img/stage/***からlist/demolistにコピーする */
		sprintf(szString, "cp %s/%s/%s.lst %s/demolist/.",
			szStageDirectory, szTarget, szDemoFile, szListDirectory);
		if(system(szString) != 0)
			return 1;

		/* デモに対応するscrファイルを作成する */
		if(CreateScrFile(szStageDirectory, szListDirectory,
			szTarget, szDemoFile, nPCMFlag) == 0)
		{
			printf("Can not create %s.scr file.\n", szDemoFile);
			return 1;
		}

		if((nCommand & MGDEMO_CUT_CHECK) == 0)
		{
			/* src ファイルを編集する */
			/* モデル等の追加に応じてEditScrFile関数内でmakeを行なうので、 */
			/* 実行を後回しにする */
			sprintf(szString, "%s/%s/%s.lst", szStageDirectory,
				szTarget, szDemoFile);
			sprintf(szString2, "%s/%s.src", szListDirectory, szTarget);
			if((nResult = EditSrcFile(szString, szString2, szTarget,
				szListDirectory)) == 0)
			{
				printf("Error Error Error!\n");
				return 1;
			}
			else if(nResult == 2)
			{
				/* srcのファイルが編集され、ステージを構築した */
				/* cvs commit を促して終了。 */
				printf("\n\nNow, \'mgdemo\' edited %s.src file.\n", szTarget);
				printf("Please fix up, run \'cvs commit\'\n");
				printf("Please run mgdemo, once more.\n\n");
				EndDemoAct(szCurrentDirectory);
				return 0;
			}
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
	if(CheckFile(szLockFilePath, 0) != 0) {
		printf("Making system data, Now!\n");
		printf("Sorry, please wait.\n");
		return 1;
	}

	/* コマンドを実行 */
	if(nCommand & MGDEMO_HELP)
	{
		PrintHelpMenu();
		return 1;
	}
	else if( (nCommand & MGDEMO_DEMO) || (nCommand & MGDEMO_LOOP))
	{
		if(RunCommand(nCommand,
			szTarget, szResident, szCDRomImgDirectory) == 0)
			return 1;
	}
	else
	{
		printf("Error! No Command.\n");
		PrintHelpMenu();
		return 1;
	}

	/* ロックファイルがあったら削除します */
	if(CheckFile("cdrom.img/_LOCK", 0) == 1)
	{
		if(system("rm cdrom.img/_LOCK") != 0)
			return 0;
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
	char* lpszFilePath,
	int   nErrorPrintFlag)
{
	FILE* fFile;

	/* ファイルのオープンを試みる */
	if((fFile = fopen(lpszFilePath, "r")) == NULL)
	{
		if(nErrorPrintFlag == 1)
			printf("Error! No such file.(%s)\n", lpszFilePath);
		return 0;
	}

	fclose(fFile);

	return 1;
}

/******************************************************************************
 * 指定されたディレクトリがあるかどうか調べる
 */
int CheckDirectory(	/* 0:存在しない */
							/* 1:存在する */
	char* lpszDirectoryPath,
	int   nErrorPrintFlag)
{
	DIR* lpDir;

	/* ディレクトリのオープンを試みる */
	if((lpDir = opendir(lpszDirectoryPath)) == NULL)
	{
		if(nErrorPrintFlag == 1)
			printf("Error! No such directory.(%s)\n", lpszDirectoryPath);
		return 0;
	}
	closedir(lpDir);
	return 1;
}



