/******************************************************************************
 * System	: DemoAct
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
#include <string.h>

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
#include "EditGclFile.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */
char* lppszDemo[] = {
	"chara デモアクトデバッグ demodebug ",
	"" };

char* lpszFindDemoAct    = { "chara デモアクトデバッグ" };
char* lpszDemoLoop       = { "-l " };
char* lpszDemoStringMark = { "-s " };
char* lpszViewCountMark  = { "-e " };
char* lpszViewStartModeMark  = { "-v " };
char* lpszFrameBaseTimeMark  = { "-f " };
char* lpszStartTimeMark  = { "-t " };

/******************************************************************************
 * gclファイルを編集する
 */
int EditGclFile(			/* 0 : 異常 */
	char* lpszGclFileName,	/* 開くファイル名 */
	char* lpszGclFileString,	/* 拡張子なしのGCL ファイル名 */
	char* lpszDemoFileName,		/* 読み込むde2ファイル名 */
	int   nCommand,			/* 指定されたコマンド */
	char  lpszDrawString[],	/* デモ中に表示される文字列 */
	char  lpszViewCount[],	/* エフェクトリスト表示時間 */
	char  lpszViewStartMode[],	/* デモ表示初期状態設定 */
	char  lpszFrameBaseTime[],	/* 30フレーム表記のベースタイム */
	char  lpszStartTime[])	/* 同期を取るためのタイミングカウンタ */
{
	int  i;
	int  nSize;
	int  nFileSize;
	int  nCommentFlag;
	long lPointer;
	FILE *fFile;
	FILE *fTmpFile;
	char szEndMark[_MAX_TEXT];
	char szText[_MAX_TEXT];
	char szNewFileName[_MAX_TEXT];
	char szLineString[_MAX_TEXT];

	/* ファイルを開いてチェックする */
	if((fFile = fopen(lpszGclFileName, "r")) == NULL)
	{
		printf("Error! Can\'t open gcl file.(scn/%s)\n", lpszGclFileName);
		return 0;
	}

	/* chara デモアクトのみを書き換える */
	printf("Add Demo.\n");

	/* tmpfile を作成する */
	if((fTmpFile = tmpfile()) == NULL)
	{
		printf("Error! Can\'t create tmp file.\n");
		return 0;
	}

	/* ファイルポインタを先頭に戻す */
	fseek(fFile, 0, SEEK_SET);

	while(1)
	{
		szLineString[0] = '\0';

		/* 一行読み出す */
		fgets(szLineString, _MAX_TEXT, fFile);

		/* コメント行は飛ばす */
		nCommentFlag = 0;
		for(i = 0; i < strlen(szLineString); i++)
		{
			/* 先頭の空白を飛ばす */
			if(isspace(szLineString[i]) != 0)
				continue;

			if(szLineString[i] == '/')
			{
				if( ((i+1) < strlen(szLineString)) && (szLineString[i] == '/') )
					nCommentFlag = 1;
			}
			else
			{
				break;
			}
		}

		/* デモアクト検索 */
		if( (nCommentFlag != 1) &&
			(strstr(szLineString, lpszFindDemoAct) != NULL) )
		{
			/* 読み込んだ行の最後に\マークがあるならそれを削除しないようにする */
			for(i = 0; i < strlen(szLineString); i++)
			{
				if(szLineString[i] == '\\') break;
			}
			if(i == strlen(szLineString))
				sprintf(szEndMark, " \n");
			else
				sprintf(szEndMark, " \\\n");

			/* デモの書き込み */
			for(i = 0; *lppszDemo[i] != '\0'; i++)
				sprintf(szLineString, "%s -i t:%s -d \'%s\' ",
					lppszDemo[i], lpszDemoFileName, lpszDemoFileName);

			/* ループデモか否か */
			if(nCommand & MGDEMO_LOOP)
				sprintf(szLineString, "%s%s ", szLineString, lpszDemoLoop);

			/* ストリングがあるか否か */
			if(lpszDrawString[0] != '\0')
				sprintf(szLineString, "%s%s\'%s\' ", szLineString, lpszDemoStringMark, lpszDrawString);

			/* エフェクトリスト表示時間の指定があるか */
			if(lpszViewCount[0] != '\0')
				sprintf(szLineString, "%s%s%s ", szLineString, lpszViewCountMark, lpszViewCount);

			/* デモ表示初期状態設定の指定があるか */
			if(lpszViewStartMode[0] != '\0')
				sprintf(szLineString, "%s%s%s ", szLineString, lpszViewStartModeMark, lpszViewStartMode);

			/* 30フレームカウントのベースタイムがあるか */
			if(lpszFrameBaseTime[0] != '\0')
				sprintf(szLineString, "%s%s%s ", szLineString, lpszFrameBaseTimeMark, lpszFrameBaseTime);

			/* 同期を取るためのタイミングカウンタ */
			if(lpszStartTime[0] != '\0')
				sprintf(szLineString, "%s%s%s%s", szLineString, lpszStartTimeMark, lpszStartTime, szEndMark);
			else
				sprintf(szLineString, "%s%s", szLineString, szEndMark);
		}

		/* テンポラリファイルに吐き出す */
		if(fputs(szLineString, fTmpFile) < 0)
		{
			printf("Error! Not write tmp file.\n");
			return 0;
		}

		if(feof(fFile) != 0)
			break;
	}

	/* 元のファイルは閉じる。改めて中を破壊して開く */
	fclose(fFile);
	fseek(fTmpFile, 0, SEEK_SET);
	lPointer = ftell(fTmpFile);
	fseek(fTmpFile, 0, SEEK_END);
	nFileSize =  ftell(fTmpFile) - lPointer;
	fseek(fTmpFile, 0, SEEK_SET);

	/* gclを新規に作成 */
	if((fFile = fopen(lpszGclFileName, "w")) == NULL)
	{
		printf("Error! -1. Not open gcl file.(scn/%s)\n", lpszGclFileName);
		return 0;
	}

	/* テンポラリーファイルの中身をコピーする */
	while(nFileSize != 0)
	{
		nSize = (nFileSize > _MAX_TEXT)? _MAX_TEXT : nFileSize;
		if(fread(szLineString, sizeof(char), nSize, fTmpFile) != nSize)
		{
			printf("Error! Not read tmp file.\n");
			return 0;
		}
		if(fwrite(szLineString, sizeof(char), nSize, fFile) != nSize)
		{
			printf("Error! Not write gcl file.(scn/%s)\n", lpszGclFileName);
			return 0;
		}
		nFileSize -= nSize;
	}
	fclose(fFile);
	fclose(fTmpFile);

	return 1;
}
