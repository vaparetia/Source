/*******************************************************************************
 * mdloutline - mdloutline.c
 * モデルアウトライン
 * 2002/03/12 S.Yamashita
 * $Id: mdloutline.c,v 1.3 2002/03/19 06:18:09 usr03692 Exp $
 */

/******************************************************************************
 * include
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#include "mdloutline.h"

/******************************************************************************
 * string
 */

#define MOL_APPNAME "model outline (version 1.0)"

/******************************************************************************
 * global
 */

APPDATA appdata;

/******************************************************************************
 * prototype
 */

static void FreeMemory(void);
static int SetParameters(int argc, char* argv[]);
static int GetParameter(int argc, char* argv[], char* lpszParam, int* lpnCount);
static int TextOutput(int nFlag, FILE* fpLog, char* lpszFormat, ...);

/******************************************************************************
 * public
 */
/******************************************************************************
 * main
 */
int main(	/* 終了コード */
	int   argc,		/* 引数の数 */
	char* argv[])	/* 引数文字列配列 */
{
	HP3DMODEL hP3DModel;
	int       nOLFlag;

	memset(&appdata, 0, sizeof(appdata));
	printf(MOL_APPNAME "\n");

	/*
	 * パラメータの設定
	 */
	if(SetParameters(argc, argv) == 0)
	{
		FreeMemory();
		return 1;
	}

	/*
	 * mdl ファイルの読み込み
	 */
	TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "read mdl file...\n");
	if((hP3DModel = P3DReadFileModel(appdata.szMdlFName, NULL, 0)) == NULL)
	{
		TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "failed.\n");
		FreeMemory();
		return 0;
	}
	// マトリクス演算
	P3DInitMovementObject(hP3DModel, NULL, TRUE);
	P3DInitObjectMatrix(hP3DModel, NULL, TRUE);
	P3DObjectMatrix(hP3DModel, NULL, TRUE);
	P3DObjectMatrixCoord(hP3DModel, NULL, TRUE);
	TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "succeeded.\n\n");

	/*
	 * アウトライン
	 */
	TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "outline mdl...\n");

	nOLFlag = 0;
	if(appdata.nFlag & MFLAG_LOGOUT     ) nOLFlag |= OL_MFLAG_LOGOUT;
	if(appdata.nFlag & MFLAG_STDOUT     ) nOLFlag |= OL_MFLAG_STDOUT;
	if(appdata.nFlag & MFLAG_OLVINFO    ) nOLFlag |= OL_MFLAG_OLVINFO;
	if(appdata.nFlag & MFLAG_DELCONVNAME) nOLFlag |= OL_MFLAG_DELCONVNAME;
	if(appdata.nFlag & MFLAG_NAMEPREFIX ) nOLFlag |= OL_MFLAG_NAMEPREFIX;
	if(appdata.nFlag & MFLAG_CALCNORMAL ) nOLFlag |= OL_MFLAG_CALCNORMAL;
	if(appdata.nFlag & MFLAG_ONEOBJECT  ) nOLFlag |= OL_MFLAG_ONEOBJECT;
	if(appdata.nFlag & MFLAG_JOINTEDGE  ) nOLFlag |= OL_MFLAG_JOINTEDGE;
	if(appdata.nFlag & MFLAG_IGNOREFACES) nOLFlag |= OL_MFLAG_IGNOREFACES;

	if(    (CreateOutline(&appdata.outline, hP3DModel, nOLFlag, appdata.fpLog,
			appdata.unBlockSize, appdata.dAMargin, appdata.dPMargin,
			appdata.unSkipNameCount, appdata.lppszSkipName,
			&appdata.unColorNameCount[0], &appdata.lppszColorName[0], 0) == 0)
		|| (ScaleOutline(&appdata.outline, appdata.fScale, 0) == 0))
	{
		DestroyOutline(&appdata.outline, 0);
		P3DDestroyModel(hP3DModel);
		TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "failed.\n");
		FreeMemory();
		return 0;
	}
	TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "succeeded.\n\n");

	/*
	 * rol ファイルの出力
	 */
	TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "output rol file...\n");

	if(OutputOutline(&appdata.outline, appdata.szRolFName, 0) == 0)
	{
		DestroyOutline(&appdata.outline, 0);
		P3DDestroyModel(hP3DModel);
		TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "failed.\n");
		FreeMemory();
		return 0;
	}
	TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "succeeded.\n\n");

	/*
	 * 終了
	 */
	DestroyOutline(&appdata.outline, 0);
	P3DDestroyModel(hP3DModel);
	TextOutput(appdata.nFlag | MFLAG_STDOUT, appdata.fpLog, "done.\n");
	FreeMemory();

	return 0;
}

/******************************************************************************
 * static
 */
/******************************************************************************
 * FreeMemory
 */
static void FreeMemory(void)
{
	unsigned int i, j;

	/* 色グループ名前 */
	for(i = 0; i < 3; i++)
	{
		if(appdata.lppszColorName[i] != NULL)
		{
			for(j = 0; j < appdata.unColorNameCount[i]; j++)
			{
				if(*(appdata.lppszColorName[i] + j) != NULL)
					free(*(appdata.lppszColorName[i] + j));
			}
			free(appdata.lppszColorName[i]);
		}
	}

	/* 処理しない名前 */
	if(appdata.lppszSkipName != NULL)
	{
		for(i = 0; i < appdata.unSkipNameCount; i++)
		{
			if(appdata.lppszSkipName[i] != NULL)
				free(appdata.lppszSkipName[i]);
		}
		free(appdata.lppszSkipName);
	}

	/* ログファイル */
	if(appdata.fpLog != NULL)
		fclose(appdata.fpLog);

	memset(&appdata, 0, sizeof(APPDATA));
}

/******************************************************************************
 * SetParameters
 */
static int SetParameters(	/* 1: 続行 */
							/* 0: 終了 */
	int   argc,		/* 引数の数 */
	char* argv[])	/* 引数文字列配列 */
{
	char* c;
	char  szBuffer[OL_STRING_MAX];
	int   nParam, nCount;
	int   i, j;

	/*
	 * Usage
	 */
	if(argc < 2)
	{
		printf("[Usage]\n");
		printf("mdloutline <inputfile(*.mdl)> [<outputfile(*.rol)>] [<<option>>]\n");
		printf("\n");
		printf("<<option>>\n");
		printf("\t-L [<logfile(*.log)>]              log output to logfile\n");
		printf("\t-T                                 log output to stdout\n");
		printf("\t-V                                 log output vertex info\n");
		printf("\t-S <scale>                         scale\n");
		printf("\t-A <margin>                        angle margin\n");
		printf("\t-P <margin>                        position margin\n");
		printf("\t-B <size>                          memory alloc block size\n");
		printf("\t-K skipname [skipname] ...         skipname\n");
		printf("\t-X color02name [color02name] ...   color02name\n");
		printf("\t-Y color03name [color03name] ...   color03name\n");
		printf("\t-Z color04name [color04name] ...   color04name\n");
		printf("\t-D                                 delete convert name mode\n");
		printf("\t-N                                 name prefix mode\n");
		printf("\t-O                                 one object mode\n");
		printf("\t-J                                 joint edge mode\n");
		printf("\t-C                                 calculate normal mode\n");
		printf("\t-F                                 ignore polygon normal direction\n");

		return 0;
	}

	/*
	 * Parameters
	 */
	printf("[Parameters]\n");
	for(i = 0; i < argc; i++)
	{
		printf("\tParameter%02d: %s\n", i, argv[i]);
	}

	/*
	 * Files
	 */
	/* 入力ファイル名 */
	strcpy(appdata.szMdlFName, argv[1]);
	if((c = strrchr(argv[1], '.')) == NULL)
	{
		strcat(appdata.szMdlFName, ".mdl");
	}

	/* 出力ファイル名 */
	if(    (argc > 2)
		&& (*argv[2] != '-'))
	{
		strcpy(appdata.szRolFName, argv[2]);
		if((c = strrchr(argv[2], '.')) == NULL)
		{
			strcat(appdata.szRolFName, ".rol");
		}
	}
	else
	{
		if((c = strrchr(argv[1], '.')) == NULL)
		{
			strcpy(appdata.szRolFName, argv[1]);
			strcat(appdata.szRolFName, ".rol");
		}
		else
		{
			strncpy(appdata.szRolFName, argv[1], c - argv[1]);
			strcat(appdata.szRolFName, ".rol");
		}
	}

	/* ログファイル名 */
	if((nParam = GetParameter(argc, argv, "Ll", &nCount)) != 0)
	{
		appdata.nFlag |= MFLAG_LOGOUT;

		if(nCount != 0)
		{
			strcpy(appdata.szLogFName, argv[nParam + 1]);
			if((c = strrchr(argv[nParam + 1], '.')) == NULL)
			{
				strcat(appdata.szLogFName, ".log");
			}
		}
		else
		{
			if((c = strrchr(argv[1], '.')) == NULL)
			{
				strcpy(appdata.szLogFName, argv[1]);
				strcat(appdata.szLogFName, ".log");
			}
			else
			{
				strncpy(appdata.szLogFName, argv[1], c - argv[1]);
				strcat(appdata.szLogFName, ".log");
			}
		}

		/* ファイルのオープン */
		if((appdata.fpLog = fopen(appdata.szLogFName, "w")) == NULL)
			return 0;
		memset(szBuffer, 0, sizeof(szBuffer));
		sprintf(szBuffer, MOL_APPNAME "\n");
		if(fwrite(szBuffer, sizeof(char), strlen(szBuffer), appdata.fpLog) != strlen(szBuffer))
			return 0;
	}

	/* 標準出力 */
	if(GetParameter(argc, argv, "Tt", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_STDOUT;
	}

	TextOutput(appdata.nFlag, appdata.fpLog, "[Files]\n");
	TextOutput(appdata.nFlag, appdata.fpLog, "\tInputFile : %s\n", appdata.szMdlFName);
	TextOutput(appdata.nFlag, appdata.fpLog, "\tOutputFile: %s\n", appdata.szRolFName);
	if(appdata.nFlag & MFLAG_LOGOUT)
		TextOutput(appdata.nFlag, appdata.fpLog, "\tLogFile   : %s\n", appdata.szLogFName);

	TextOutput(appdata.nFlag, appdata.fpLog, "[Options]\n");
	if(appdata.nFlag & MFLAG_LOGOUT)
		TextOutput(appdata.nFlag, appdata.fpLog, "\tlog output to logfile\n");
	if(appdata.nFlag & MFLAG_STDOUT)
		TextOutput(appdata.nFlag, appdata.fpLog, "\tlog output to stdout\n");

	/* アウトライン頂点情報 */
	if(GetParameter(argc, argv, "Vv", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_OLVINFO;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tlog output vertex info\n");
	}

	/* スケール */
	if(    ((nParam = GetParameter(argc, argv, "Ss", &nCount)) != 0)
		&& (nCount != 0))
	{
		sscanf(argv[nParam + 1], "%f", &appdata.fScale);
		if(appdata.fScale <= 0.0)
			appdata.fScale = DEFAULT_SCALE;
	}
	else
	{
		appdata.fScale = DEFAULT_SCALE;
	}
	TextOutput(appdata.nFlag, appdata.fpLog, "\tscale: %f\n", appdata.fScale);

	/* 角度マージン */
	if(    ((nParam = GetParameter(argc, argv, "Aa", &nCount)) != 0)
		&& (nCount != 0))
	{
		sscanf(argv[nParam + 1], "%lf", &appdata.dAMargin);
		if((appdata.dAMargin < 0.0) || (1.0 < appdata.dAMargin))
			appdata.dAMargin = DEFAULT_AMARGIN;
	}
	else
	{
		appdata.dAMargin = DEFAULT_AMARGIN;
	}
	TextOutput(appdata.nFlag, appdata.fpLog, "\tangle margin: %lf\n", appdata.dAMargin);

	/* 位置マージン */
	if(    ((nParam = GetParameter(argc, argv, "Pp", &nCount)) != 0)
		&& (nCount != 0))
	{
		sscanf(argv[nParam + 1], "%lf", &appdata.dPMargin);
		if(appdata.dPMargin < 0.0)
			appdata.dPMargin = DEFAULT_PMARGIN;
	}
	else
	{
		appdata.dPMargin = DEFAULT_PMARGIN;
	}
	TextOutput(appdata.nFlag, appdata.fpLog, "\tposition margin: %lf\n", appdata.dPMargin);

	/* メモリ確保ブロックサイズ */
	if(    ((nParam = GetParameter(argc, argv, "Bb", &nCount)) != 0)
		&& (nCount != 0))
	{
		sscanf(argv[nParam + 1], "%u", &appdata.unBlockSize);
		if(appdata.unBlockSize < 1)
			appdata.unBlockSize = DEFAULT_BLOCKSIZE;
	}
	else
	{
		appdata.unBlockSize = DEFAULT_BLOCKSIZE;
	}
	TextOutput(appdata.nFlag, appdata.fpLog, "\tmemory alloc block size: %d\n", appdata.unBlockSize);

	/* 処理しない名前 */
	if(    ((nParam = GetParameter(argc, argv, "Kk", &nCount)) != 0)
		&& (nCount != 0))
	{
		if((appdata.lppszSkipName = (char**)calloc(nCount, sizeof(char*))) == NULL)
			return 0;

		/* 処理しない名前数 */
		appdata.unSkipNameCount = (unsigned)nCount;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tskipname: %u\n", appdata.unSkipNameCount);

		/* 処理しない名前配列 */
		for(i = 0; i < nCount; i++)
		{
			if((appdata.lppszSkipName[i] = (char*)calloc(OL_STRING_MAX, sizeof(char))) == NULL)
				return 0;

			strncpy(appdata.lppszSkipName[i], argv[nParam + 1 + i], OL_STRING_MAX - 1);
			TextOutput(appdata.nFlag, appdata.fpLog, "\tskipname%02d: %s\n", i, appdata.lppszSkipName[i]);
		}
	}

	/* 色グループ名前 */
	for(i = 0; i < 3; i++)
	{
		memset(szBuffer, 0, sizeof(szBuffer));
		switch(i)
		{
		case 0: strcpy(szBuffer, "Xx"); break;
		case 1: strcpy(szBuffer, "Yy"); break;
		case 2: strcpy(szBuffer, "Zz"); break;
		}
		if(    ((nParam = GetParameter(argc, argv, szBuffer, &nCount)) != 0)
			&& (nCount != 0))
		{
			if((appdata.lppszColorName[i] = (char**)calloc(nCount, sizeof(char*))) == NULL)
				return 0;

			/* 色グループ名前数の配列 */
			appdata.unColorNameCount[i] = (unsigned)nCount;
			TextOutput(appdata.nFlag, appdata.fpLog, "\tcolor%02dname: %u\n", i + 2, appdata.unColorNameCount[i]);

			/* 色グループ名前配列の配列 */
			for(j = 0; j < nCount; j++)
			{
				if((*(appdata.lppszColorName[i] + j) = (char*)calloc(OL_STRING_MAX, sizeof(char))) == NULL)
					return 0;

				strncpy(*(appdata.lppszColorName[i] + j), argv[nParam + 1 + j], OL_STRING_MAX - 1);
				TextOutput(appdata.nFlag, appdata.fpLog, "\tcolor%02dname%02d: %s\n", i + 2, j, *(appdata.lppszColorName[i] + j));
			}
		}
	}

	/* 変換定義の削除を行う */
	if(GetParameter(argc, argv, "Dd", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_DELCONVNAME;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tdelete convert name mode\n");
	}

	/* 名前比較 プリフィックスモード */
	if(GetParameter(argc, argv, "Nn", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_NAMEPREFIX;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tname prefix mode\n");
	}

	/* １オブジェクトモード */
	if(GetParameter(argc, argv, "Oo", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_ONEOBJECT;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tone object mode\n");
	}

	/* エッジの連結処理を行う */
	if(GetParameter(argc, argv, "Jj", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_JOINTEDGE;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tjoint edge mode\n");
	}

	/* ポリゴンの法線を計算する */
	if(GetParameter(argc, argv, "Cc", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_CALCNORMAL;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tcalculate normal mode\n");
	}

	/* ポリゴンの法線の向きは無視する */
	if(GetParameter(argc, argv, "Ff", NULL) != 0)
	{
		appdata.nFlag |= MFLAG_IGNOREFACES;
		TextOutput(appdata.nFlag, appdata.fpLog, "\tignore polygon normal direction\n");
	}

	TextOutput(appdata.nFlag, appdata.fpLog, "\n");

	return 1;
}

/******************************************************************************
 * パラメータの検索
 */
static int GetParameter(	/* 0 以外: 指定パラメータの引数番号 */
							/* 0     : パラメータは含まれない */
	int   argc,			/* 引数の数 */
	char* argv[],		/* 引数文字列配列 */
	char* lpszParam,	/* 検索パラメータ文字 */
	int*  lpnCount)		/* パラメータへの付加情報引数の数を取得 */
{
	int i, j;

	for(i = 2 ; i < argc; i++)
	{
		if(    (*argv[i] == '-')
			&& (strlen(argv[i]) > 1))
		{
			if(strchr(lpszParam, *(argv[i] + 1)) != NULL)
			{
				if(lpnCount != NULL)
				{
					*lpnCount = 0;
					for(j = i + 1; j < argc; j++)
					{
						if(*argv[j] == '-')
							break;
						(*lpnCount)++;
					}
				}
				return i;
			}
		}
	}

	return 0;
}

/******************************************************************************
 * テキスト出力
 */
static int TextOutput(	/* 1: 成功 */
						/* 0: 失敗 */
	int   nFlag,		/* フラグ */
	FILE* fpLog,		/* ログファイルポインタ */
	char* lpszFormat,	/* テキストの書式 */
	...)				/* 可変引数 */
{
	char    szText[OL_STRING_MAX * 4];
	va_list valist;

	/* テキストの設定 */
	memset(szText, 0, sizeof(szText));
	va_start(valist, lpszFormat);
	vsprintf(szText, lpszFormat, valist);
	va_end(valist);

	/* ログファイルに出力 */
	if((nFlag & MFLAG_LOGOUT) && (fpLog != NULL))
	{
		if(fwrite(szText, sizeof(char), strlen(szText), fpLog) != strlen(szText))
			return 0;
	}

	/* 標準出力に出力 */
	if(nFlag & MFLAG_STDOUT)
	{
		printf(szText);
	}

	return 1;
}
