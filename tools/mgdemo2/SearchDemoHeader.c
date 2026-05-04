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
#include "SearchDemoHeader.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */

/******************************************************************************
 * gclファイルを編集する
 */
int SearchDemoHeader(			/* 0 : インクルード文はなかった */
	char lpszGclFileName[],
	char lpszScnDirectory[])
{
	int  i;
	int  nCommentFlag;
	int  nCheck;
	FILE *fFile;
	char szFileName[_MAX_TEXT];
	char szLineString[_MAX_TEXT];

	/* ファイルを開いてチェックする */
	if((fFile = fopen(lpszGclFileName, "r")) == NULL)
	{
		printf("Error! Can\'t open gcl file.(scn/%s)\n", lpszGclFileName);
		return 0;
	}

	/* ファイルポインタを先頭に戻す */
	fseek(fFile, 0, SEEK_SET);

	nCheck = 0;
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

		/* インクルード文検索 */
		if( (nCommentFlag != 1) &&
			(strstr(szLineString, "#include") != NULL) )
		{
			char* lpszKeyWord;

			/* インクルード文を発見 */
			/* 命名規則は "polydemo_*****.h" */ 
			if((lpszKeyWord = strstr(szLineString, "polydemo_")) != NULL)
			{
				/* キーワードを発見 */
				for(i = 0; ; i++, lpszKeyWord++)
				{
					szFileName[i] = *lpszKeyWord;
					if((*lpszKeyWord == '.') && (*(lpszKeyWord+1) == 'h'))
					{
						szFileName[i+1] = *(lpszKeyWord + 1);
						szFileName[i+2] = '\0';
						sprintf(lpszGclFileName, "%s/%s\0", 
							lpszScnDirectory, szFileName);
						nCheck = 1;
						break;
					}
				}
			}
		}

		if(feof(fFile) != 0)
			break;
	}

	fclose(fFile);

	return nCheck;
}
