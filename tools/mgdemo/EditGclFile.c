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
char* lpszCheckCodeDemo = "// This File is Only demo.\n";

char* lpszCheckEUCCode = "\n//蘭(EUC判定用文字)\n";

char* lppszStandardChara[] = {
	"\n#include \"stdch.h\"\n",
	"@常駐キャラ設定\n",
	"",
	};

char* lppszCommentDefine[] = {
	"\n// 各種define\n",
	"// ------------------------------------\n",
	"// マップ関係 \n",
	"" };

char* lppszDefine[] = {
	"#define HZX_NAME ",
	"#define KMS_NAME ",
	"#define LT2_NAME ",
	"" };

char* lppszCommentMap[] = {
	"\n// マップ設定 \n",
	"" };

char* lppszMap[] = {
	"chara マップ map -k d:KMS_NAME -h d:HZX_NAME,0 -l d:LT2_NAME\n",
	"command マップ表示 -show map\n",
	"" } ;

char* lppszCommentShadow[] = {
	"\n// 影投影モデル \n",
	"" };

char* lpszShadow1 = { "chara 影投影モデル shadow -model" };
char* lpszShadow2 = { "-pos 0 0 0\n" };

char* lppszCommentDemo[] = {
	"\n// デモ設定 \n",
	"" } ;

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

	/* 先頭の１行を取り出す */
	if(fgets(szLineString, _MAX_TEXT, fFile) == NULL)
	{
		printf("Error! wrong gcl file.(scn/%s)\n", lpszGclFileName);
		return 0;
	}

	if(strcmp(szLineString, lpszCheckCodeDemo) == 0)
	{
		/* これはすでにDemoActが編集したファイルなので */
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

			/* デモアクト検索 */
			if(strstr(szLineString, lpszFindDemoAct) != NULL)
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
					sprintf(szLineString, "%s -d \'%s\' ", lppszDemo[i], lpszDemoFileName);

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
	}
	else
	{
		printf("New Demo.\n");

		/* DemoAct以外で編集されたファイルなのでrenameする */
		fclose(fFile);
		for(i = 0; ; i++)
		{
			sprintf(szNewFileName, "scn/%s.gcl.org%d", lpszGclFileString, i);
			if(rename(lpszGclFileName, szNewFileName) == 0)
				break;
		}

		/* gclを新規に作成 */
		if((fFile = fopen(lpszGclFileName, "w")) == NULL)
		{
			printf("Error! -2. Not open gcl file.(scn/%s)\n", lpszGclFileName);
			return 0;
		}

		/* ファイルの書き込みを行う */
		/* チェックコードの書き込み */
		if(fputs(lpszCheckCodeDemo, fFile) < 0)
		{
			printf("Error! -0. Not write gcl file.(scn/%s)\n", lpszGclFileName);
			return 0;
		}

		/* チェックコードの書き込み */
		if(fputs(lpszCheckEUCCode, fFile) < 0)
		{
			printf("Error! -1. Not write gcl file.(scn/%s)\n", lpszGclFileName);
			return 0;
		}

		/* 常駐キャラのインクルード文と呼び出しの書き込み */
		for(i = 0; *lppszStandardChara[i] != '\0'; i++)
		{
			if(fputs(lppszStandardChara[i], fFile) < 0)
			{
				printf("Error! -1.5. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* defineのコメント行の書き込み */
		for(i = 0; *lppszCommentDefine[i] != '\0'; i++)
		{
			if(fputs(lppszCommentDefine[i], fFile) < 0)
			{
				printf("Error! -2. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* defineの書き込み */
		for(i = 0; *lppszDefine[i] != '\0'; i++)
		{
			sprintf(szLineString, "%s%s\n", lppszDefine[i], lpszGclFileString);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -3. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* マップのコメント行の書き込み */
		for(i = 0; *lppszCommentMap[i] != '\0'; i++)
		{
			if(fputs(lppszCommentMap[i], fFile) < 0)
			{
				printf("Error! -4. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* マップの書き込み */
		for(i = 0; *lppszMap[i] != '\0'; i++)
		{
			if(fputs(lppszMap[i], fFile) < 0)
			{
				printf("Error! -5. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* 影投影モデルのコメント行の書き込み */
		for(i = 0; *lppszCommentShadow[i] != '\0'; i++)
		{
			if(fputs(lppszCommentShadow[i], fFile) < 0)
			{
				printf("Error! -6. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* キャラ影投影モデルの書き込み */
		sprintf(szLineString, "%s %s %s\n", lpszShadow1, lpszGclFileString, lpszShadow2);
		if(fputs(szLineString, fFile) < 0)
		{
			printf("Error! -7. Not write gcl file.(scn/%s)\n", lpszGclFileName);
			return 0;
		}

		/* デモのコメント行の書き込み */
		for(i = 0; *lppszCommentDemo[i] != '\0'; i++)
		{
			if(fputs(lppszCommentDemo[i], fFile) < 0)
			{
				printf("Error! -8. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* デモの書き込み */
		for(i = 0; *lppszDemo[i] != '\0'; i++)
		{
			sprintf(szLineString, "%s -d \'%s\' ", lppszDemo[i], lpszDemoFileName);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -9. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* ループデモか否か */
		if(nCommand & MGDEMO_LOOP)
		{
			sprintf(szLineString, "%s ", lpszDemoLoop);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -10. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* ストリングがあるか否か */
		if(lpszDrawString[0] != '\0')
		{
			sprintf(szLineString, "%s\'%s\' ", lpszDemoStringMark, lpszDrawString);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -10. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* エフェクトリストがあるか否か */
		if(lpszViewCount[0] != '\0')
		{
			sprintf(szLineString, "%s%s ", lpszViewCountMark, lpszViewCount);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -10. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* デバッグモード初期状態指定があるか */
		if(lpszViewStartMode[0] != '\0')
		{
			sprintf(szLineString, "%s%s ", lpszViewStartModeMark, lpszViewStartMode);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -10. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* 30フレームカウントのベースタイム指定があるか */
		if(lpszFrameBaseTime[0] != '\0')
		{
			sprintf(szLineString, "%s%s ", lpszFrameBaseTimeMark, lpszFrameBaseTime);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -10. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		/* 同期を取るためのタイミングカウンタ */
		if(lpszStartTime[0] != '\0')
		{
			sprintf(szLineString, "%s%s\n", lpszStartTimeMark, lpszStartTime);
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -10. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}
		else
		{
			sprintf(szLineString, "\n");
			if(fputs(szLineString, fFile) < 0)
			{
				printf("Error! -11. Not write gcl file.(scn/%s)\n", lpszGclFileName);
				return 0;
			}
		}

		fclose(fFile);
	}

	return 1;
}
