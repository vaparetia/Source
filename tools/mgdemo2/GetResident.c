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
#include "GetResident.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */
char* lpszBlock = { "block" };
char* lpszUse   = { ".use" };

/******************************************************************************
 * Src ファイルからレジデントファイルの情報を取得する
 * block .use ***** の情報を取得する
 */
int GetResident(			/* 0 : 異常 */
	char* lpszSrcFileName,	/* 開くファイル名 */
	char* lpszResident)		/* 常駐名 */
{
	int   nCount;
	int   nSize;
	int   nFileSize;
	long  lPointer;
	char  szLineString[_MAX_TEXT];	/* 一行分だけ読み出す */
	char* lpszString;
	FILE* fFile;		/* init.gclのファイルポインタ */

	/* ファイルを開く */
	if((fFile = fopen(lpszSrcFileName, "r")) == NULL)
	{
		printf("Error! -0. Can\'t open src File.(scn/%s)\n", lpszSrcFileName);
		return 0;
	}

	while(1)
	{
		szLineString[0] = '\0';

		/* 一行読み出す */
		fgets(szLineString, _MAX_TEXT, fFile);

		/* block文検索 */
		if(strstr(szLineString, lpszBlock) != NULL)
		{
			if((lpszString = strstr(szLineString, lpszUse)) != NULL)
			{
				/* レジデントに関する記述があった。 */
				/* lpszString には".use" の先頭アドレスがあるので、*/
				/* そこからレジデントのフィールドまで進めて値を拾う */

				/* .use 記述を飛ばす */
				while(isspace((int)(*lpszString)) == 0)
					lpszString++;

				/* レジデントまでの空白を飛ばす */
				while(isspace((int)(*lpszString)) != 0)
					lpszString++;

				nCount = 0;
				while(isspace((int)(*lpszString)) == 0)
				{
					*(lpszResident + nCount) = *lpszString;
					nCount++;
					lpszString++;
				}
				lpszString[nCount + 1] = '\0';
				break;
			}
		}

		if(feof(fFile) != 0)
			break;
	}

	fclose(fFile);

	return 1;
}
