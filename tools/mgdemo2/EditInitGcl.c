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
#include "EditInitGcl.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */
char* lpszLoad1 = { "load " };
char* lpszLoad2 = { "load\t" };

/******************************************************************************
 * init.gclファイルを編集する
 */
int EditInitGcl(			/* 0 : 異常 */
	char* lpszInitGclName,	/* 開くファイル名 */
	char* lpszTargetStage,	/* ロードするステージ */
	char* lpszResident)		/* レジデント設定 */
{
	int   nSize;
	int   nFileSize;
	long  lPointer;
	char  szLineString[_MAX_TEXT];	/* 一行分だけ読み出す */
	FILE* fFile;		/* init.gclのファイルポインタ */
	FILE* fTmpFile;	/* テンポラリファイルのファイルポインタ */

	/* ファイルを開く */
	if((fFile = fopen(lpszInitGclName, "r")) == NULL)
	{
		printf("Error! -0. Can\'t open gcl File.(scn/%s)\n", lpszInitGclName);
		return 0;
	}

	/* tmpfile を作成する */
	if((fTmpFile = tmpfile()) == NULL)
	{
		printf("Error! Can\'t create Tmp File.\n");
		return 0;
	}

	while(1)
	{
		szLineString[0] = '\0';

		/* 一行読み出す */
		fgets(szLineString, _MAX_TEXT, fFile);

		/* Load文検索 */
		if( (strstr(szLineString, lpszLoad1) != NULL) ||
			(strstr(szLineString, lpszLoad2) != NULL))
		{
			/* Load文の書き込み */
			sprintf(szLineString, "%s \'%s\' -r \'%s\'\n",
				lpszLoad1, lpszTargetStage, lpszResident);
		}

		/* テンポラリファイルに吐き出す */
		if(fputs(szLineString, fTmpFile) < 0)
		{
			printf("Error! Not write Tmp File.\n");
			return 0;
		}

		if(feof(fFile) != 0)
			break;
	}

	/* 元のファイルは閉じる。改めて中を破壊して開く */
	fclose(fFile);

	/* テンポラリファイルのサイズを計算 */
	fseek(fTmpFile, 0, SEEK_SET);
	lPointer = ftell(fTmpFile);
	fseek(fTmpFile, 0, SEEK_END);
	nFileSize =  ftell(fTmpFile) - lPointer;
	fseek(fTmpFile, 0, SEEK_SET);

	/* gclを新規に作成 */
	if((fFile = fopen(lpszInitGclName, "w")) == NULL)
	{
		printf("Error! -1. Not open gcl File.(scn/%s)\n", lpszInitGclName);
		return 0;
	}

	/* テンポラリーファイルの中身をコピーする */
	while(nFileSize != 0)
	{
		nSize = (nFileSize > _MAX_TEXT)? _MAX_TEXT : nFileSize;
		if(fread(szLineString, sizeof(char), nSize, fTmpFile) != nSize)
		{
			printf("Error! Not read Tmp File.\n");
			return 0;
		}
		if(fwrite(szLineString, sizeof(char), nSize, fFile) != nSize)
		{
			printf("Error! Not write gcl File.(scn/%s)\n", lpszInitGclName);
			return 0;
		}
		nFileSize -= nSize;
	}
	fclose(fFile);
	fclose(fTmpFile);

	return 1;
}
