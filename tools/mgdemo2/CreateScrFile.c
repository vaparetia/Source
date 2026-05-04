/******************************************************************************
 * System	: mgdemo
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Create Scr File
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <stdlib.h>

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

#include "CreateScrFile.h"

/******************************************************************************
 * defines
 */

/******************************************************************************
 * functions
 */
static int fputsEx(char* lpszString, char* lpszTarget, FILE* fFile);

/******************************************************************************
 * publics
 */
static char* lpszScrText1 = {"block "};

/******************************************************************************
 * デモに対応したscrファイルを作成する
 */
int CreateScrFile(	/* 0 正常 */
			/* 0 以外、異常 */
	char* lpszStageDirectory,	/* cdrom.img/stageディレクトリのパス */
	char* lpszListDirectory,	/* listディレクトリのパス */
	char* lpszTarget,			/* 実行するステージの名称 */
	char* lpszDemoFile,			/* 実行するデモファイル名 */
	int   nPcmFlag)				/* PCMがあるかどうか */
{
	char  szString[_MAX_TEXT];
	FILE* fFile;


	/* 指定のファイルを開く */
	sprintf(szString, "%s/demoscr/%s.scr", lpszListDirectory, lpszTarget);
	if((fFile = fopen(szString, "w")) == NULL)
	{
		printf("File open error! %s.scr", lpszTarget);
		return 0;
	}

	/* 書き込む */
	sprintf(szString, "%s%s\n{\n", lpszScrText1, lpszDemoFile);
	if(fputsEx(szString, lpszTarget, fFile) < 0)
		return 0;

	/* de2へのパス */
	sprintf(szString, "\tdemo ../cdrom.img/stage/%s/%s.de2\n", lpszTarget, lpszDemoFile);
	if(fputsEx(szString, lpszTarget, fFile) < 0)
		return 0;

	/* pcmもチャンクする必要があるのか */
	if( nPcmFlag == 1)
	{
		/* pcmへのパス */
		sprintf(szString, "\tsound ../cdrom.img/stage/%s/%s.pcm\n", lpszTarget, lpszDemoFile);
		if(fputsEx(szString, lpszTarget, fFile) < 0)
			return 0;
	}

	/* カッコとじ */
	sprintf(szString, "}\n");
	if(fputsEx(szString, lpszTarget, fFile) < 0)
		return 0;

	fclose(fFile);
	return 1;
}

/******************************************************************************
 * fputsExにこのルーチン特有のエラーテキスト処理を加えたもの
 */
static int fputsEx(
	char* lpszString,	/* 書き出す文字列 */
	char* lpszTarget,	/* エラーが発生したファイル名 */
	FILE* fFile)		/* 書き込むファイルのファイルポインタ */
{
	int nResult;

	if((nResult = fputs(lpszString, fFile)) < 0)
		printf("write error! %s.scr\n", lpszTarget);

	return nResult;
}


