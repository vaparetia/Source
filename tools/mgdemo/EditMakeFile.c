/******************************************************************************
 * System	: mgdemo
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
#include "EditMakeFile.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */
char* lpszMakeStageInList = "\t-(for i in $(SCENARIOS:.gcl=); do gclconv $(GCLOPTIONS) $$i.gcl -m $(SOURCEPATH) -l $(STAGEPATH)/$$i; done)
";

char* lppszCheckWordList[] = {
	"stage",
	"for",
	"gclconv",
	"" };

/******************************************************************************
 * makefileを編集する
 */
int EditMakeFile(			/* 0 : 異常 */
	char* lpszFileName)	/* 開くファイル名 */
{
	int   i;
	int   nSize;
	int   nFileSize;
	char* lpcString;
	long  lPointer;
	FILE* fFile;
	FILE* fTmpFile;
	char  szLineString[_MAX_TEXT];
	char  szTmpString[_MAX_TEXT];

	/* ファイルを開いてチェックする */
	if((fFile = fopen(lpszFileName, "r")) == NULL)
	{
		printf("Error! -0. Can\'t open makefile.(../scn/%s)\n", lpszFileName);
		return 0;
	}

	/* tmpfile を作成する */
	if((fTmpFile = tmpfile()) == NULL)
	{
		printf("Error! Can\'t create Tmp file.\n");
		return 0;
	}

	while(feof(fFile) == 0)
	{
		szLineString[0] = '\0';

		/* 一行読み出す */
		fgets(szLineString, _MAX_TEXT, fFile);

		/* 一つ目のキーワードを検索 */
		if(strstr(szLineString, lppszCheckWordList[0]) != NULL)
		{
			/* 一つ目のキーワードがあった */
			/* テンポラリファイルに書き出す */
			if(fputs(szLineString, fTmpFile) < 0)
			{
				printf("Error! Not write Tmp file.\n");
				return 0;
			}

			if(feof(fFile) != 0)
				break;

			/* 一行読み出す。これに残りのキーワードがあるかチェック */
			fgets(szLineString, _MAX_TEXT, fFile);

			/* コメント行以外の二つ目のキーワードをチェック */
			if((szLineString[0] != '#') &&
				(lpcString = strstr(szLineString, lppszCheckWordList[1])) != NULL)
			{
				/* 二つ目のキーワードもあった */ 
				/* 文字列を進めて、最後のチェック */
				lpcString += sizeof(char) * strlen(lppszCheckWordList[1]);
				if(strstr(lpcString, lppszCheckWordList[2]) != NULL)
				{
					/* キーワードが全部あったがすでに書き換えたやつかもしれない */
					if(strcmp(szLineString, lpszMakeStageInList) != 0)
					{
						/* もともとあった文字列はコメント文にする */
						sprintf(szTmpString, "#%s", szLineString);

						/* テンポラリファイルに書き出す */
						if(fputs(szTmpString, fTmpFile) < 0)
						{
							printf("Error! Not write Tmp file.\n");
							return 0;
						}

						/* 書き換える */
						strcpy(szLineString, lpszMakeStageInList);
					}
				}
			}

			/* テンポラリファイルに書き出す */
			if(fputs(szLineString, fTmpFile) < 0)
			{
				printf("Error! Not write Tmp file.\n");
				return 0;
			}
		}
		else
		{
			/* テンポラリファイルに書き出す */
			if(fputs(szLineString, fTmpFile) < 0)
			{
				printf("Error! Not write Tmp file.\n");
				return 0;
			}
		}
	}

	/* 元のファイルは閉じる。改めて中を破壊して開く */
	fclose(fFile);

	fseek(fTmpFile, 0, SEEK_SET);
	lPointer = ftell(fTmpFile);
	fseek(fTmpFile, 0, SEEK_END);
	nFileSize =  ftell(fTmpFile) - lPointer;
	fseek(fTmpFile, 0, SEEK_SET);

	/* makefileを新規に作成 */
	if((fFile = fopen(lpszFileName, "w")) == NULL)
	{
		printf("Error! -1. Not open makefile.(../scn/%s)\n", lpszFileName);
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
			printf("Error! Not write makefile.(../scn/%s)\n", lpszFileName);
			return 0;
		}
		nFileSize -= nSize;
	}
	fclose(fFile);
	fclose(fTmpFile);

	return 1;
}
