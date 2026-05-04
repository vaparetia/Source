/******************************************************************************
 * System	: mgdemo exe
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
#include "PrintHelpMenu.h"
#include "TargetList.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */


/******************************************************************************
 * 引数のチェックをし、実行に必要なデータを作成します。
 */
/******************************************************************************
 * メイン
 */
int AnalyzeCommand(	/* 0以外	正常 */
			/* 0 */
	int   argc,		/* 引数数 */
	char* argv[],	/* 引数 */
	int*  lpnCommand,		/* 実行コマンド */
	char* lpszTarget,		/* 作業ディレクトリ */
	char* lpszDemoFile,		/* 実行するデモファイル */
	char  lpszString[],		/* デモ中に表示する文字列 */
	char  lpszViewCount[],	/* エフェクトリスト表示時間 */
	char  lpszViewStartMode[],	/* デモ表示初期設定 */
	char  lpszFrameBaseTime[],	/* 30フレーム表記のベースタイム */
	char  lpszStartTime[],	/* 同期を取るためのタイミングカウンタ */
	int*  lpnPCMStream,	/* PCM ストリーミングの有無 */
	char  lpszAddGclFile[])
{
	int  i, j;
	int  nCommandIndex;		/* コマンド文字列があったargvのインデックス */
	int  nDemoFileIndex;		/* デモファイルのあったargvのインデックス */
	char szString[_MAX_TEXT];

	/* 引数があるかのチェック */
	if(argc == 1)
	{
		printf("\nError! Please input command or demo file name.\n");
		PrintHelpMenu();
		return 0;
	}

	/* 引数をすべてチェック。コマンドを決定する */
	for(i = 1; i < argc; i++)
	{
		/* コマンド解析 */
		if(strcmp(argv[i], COMMAND_HELP) == 0)
		{
			/* 引数がhelpコマンドだった */
			*lpnCommand |= MGDEMO_HELP;
			argv[i][0] = '\0';
		}
		else if(strcmp(argv[i], COMMAND_DEMO) == 0)
		{
			/* 引数がdemoコマンドだった */
			*lpnCommand |= MGDEMO_DEMO;
			argv[i][0] = '\0';
		}
		else if(strcmp(argv[i], COMMAND_DEMOLOOP) == 0)
		{
			/* 引数がloopコマンドだった */
			*lpnCommand |= MGDEMO_LOOP;
			argv[i][0] = '\0';
		}
		else if(strcmp(argv[i], COMMAND_CUT_CHECK) == 0)
		{
			/* 引数がカットコマンドだった */
			*lpnCommand |= MGDEMO_CUT_CHECK;
			argv[i][0] = '\0';
		}
		else if(strcmp(argv[i], COMMAND_MAKE_HD_MODULE) == 0)
		{
			/* HD用データ作成コマンドが指定された */
			*lpnCommand |= MGDEMO_MAKE_HD_MODULE;
			*lpnCommand |= MGDEMO_HD_FORMAT;
			argv[i][0] = '\0';
		}
		else if(strcmp(argv[i], COMMAND_MAKE_HD_NO_FORMAT) == 0)
		{
			/* HDフォーマットしないデモＨＤコマンドが指定された */
			*lpnCommand |= MGDEMO_MAKE_HD_MODULE;
			argv[i][0] = '\0';
		}
	}

	/* ループコマンドかデモコマンドのどちらかが指定されていること */
	if((*lpnCommand & MGDEMO_LOOP) == 0)
		*lpnCommand |= MGDEMO_DEMO;

	/* デモファイルの決定 : 最初の文字列をデモファイルとする */
	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] != '\0')
		{
			strcpy(lpszDemoFile, argv[i]);
			argv[i][0] = '\0';
			break;
		}
	}

	/* エフェクト表示時間の検索 */
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-e") == 0)
		{
			/* 最後の引数だったので続きはない */
			if(i == (argc - 1))
			{
				lpszViewCount[0] = '3';
				lpszViewCount[1] = '0';
				lpszViewCount[2] = '\0';
				argv[i][0]     = '\0';
				break;
			}

			/* 引数が文字列だった */
			if((lpszViewCount[0] == '\0') && ( i != (argc - 1)))
			{
				/* まだ文字列が設定されていない */
				if( isdigit((int)(argv[i+1][0])) != 0)
				{			
					strcpy(lpszViewCount, argv[i + 1]);
					argv[i][0]     = '\0';
					argv[i + 1][0] = '\0';
					break;
				}
				else
				{
					lpszViewCount[0] = '3';
					lpszViewCount[1] = '0';
					lpszViewCount[2] = '\0';
					argv[i][0]     = '\0';
					break;
				}
			}
		}
	}

	/* デモ表示初期状態設定の検索 */
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-v") == 0)
		{
			/* 最後の引数だったので続きはない */
			if(i == (argc - 1))
			{
				lpszViewStartMode[0] = '0';
				lpszViewStartMode[1] = '\0';
				argv[i][0]     = '\0';
				break;
			}

			/* 引数が文字列だった */
			if(lpszViewStartMode[0] == '\0')
			{
				/* まだ文字列が設定されていない */
				if( isdigit((int)(argv[i+1][0])) != 0)
				{			
					strcpy(lpszViewStartMode, argv[i + 1]);
					argv[i][0]     = '\0';
					argv[i + 1][0] = '\0';
					break;
				}
				else
				{
					lpszViewStartMode[0] = '0';
					lpszViewStartMode[1] = '\0';
					argv[i][0]     = '\0';
					break;
				}
			}
		}
	}

	/* 30フレーム表記のベースタイムの設定 */
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-f") == 0)
		{
			/* 最後の引数だったので続きはない */
			if(i == (argc - 1))
			{
				lpszFrameBaseTime[0] = '0';
				lpszFrameBaseTime[1] = '\0';
				argv[i][0]     = '\0';
				break;
			}
		
			/* 引数が文字列だった */
			if(lpszFrameBaseTime[0] == '\0')
			{
				/* まだ文字列が設定されていない */
				if((isdigit((int)(argv[i+1][0])) != 0) || (argv[i+1][0] == '-'))
				{
					strcpy(lpszFrameBaseTime, argv[i + 1]);
					argv[i][0]     = '\0';
					argv[i + 1][0] = '\0';
					break;
				}
				else
				{
					lpszFrameBaseTime[0] = '0';
					lpszFrameBaseTime[1] = '\0';
					argv[i][0]     = '\0';
					break;
				}
			}
		}
	}

	/* 同期を取るためのタイミングカウンタ */
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-t") == 0)
		{
			/* 最後の引数だったので続きはない */
			if(i == (argc - 1))
			{
				lpszStartTime[0] = '5';
				lpszStartTime[1] = '\0';
				argv[i][0]     = '\0';
				break;
			}
		
			/* 引数が文字列だった */
			if(lpszStartTime[0] == '\0')
			{
				/* まだ文字列が設定されていない */
				if((isdigit((int)(argv[i+1][0])) != 0) || (argv[i+1][0] == '-'))
				{
					strcpy(lpszStartTime, argv[i + 1]);
					argv[i][0]     = '\0';
					argv[i + 1][0] = '\0';
					break;
				}
				else
				{
					lpszStartTime[0] = '5';
					lpszStartTime[1] = '\0';
					argv[i][0]     = '\0';
					break;
				}
			}
		}
	}

	/* PCM のストリーミングの有無を指定 */
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-p") == 0)
		{
			/* 最後の引数だったので続きはない */
			if(i == (argc - 1))
			{
				*lpnPCMStream = 1;
				argv[i][0]     = '\0';
				break;
			}
		
			/* 引数が文字列だった */
			if((isdigit((int)(argv[i+1][0])) != 0) || (argv[i+1][0] == '-'))
			{
				if( strcmp("0", argv[i + 1]) == 0 )
					*lpnPCMStream = 0;
				else
					*lpnPCMStream = 1;
				argv[i][0]     = '\0';
				break;
			}
			else
			{
				*lpnPCMStream = 1;
				argv[i][0]     = '\0';
				break;
			}
		}
	}

	/* 任意のファイルを検索対象に加える */
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-a") == 0)
		{
			/* 引数が文字列だった */
			if( (i == (argc - 1)) || (argv[i + 1][0] == '\0') )
			{
				/* 文字列が無い、もしくは最後にオプションが指定された */ 
				lpszAddGclFile[0] = '\0';
			}
			else
			{
				/* 何らかの文字列があった */
				sprintf(lpszAddGclFile, "%s\0", argv[i + 1]);
				argv[i + 1][0]     = '\0';
			}
			argv[i][0]     = '\0';
			break;
		}
	}

	/* テキスト文字列の検索 : 文字列確定のためにオプションでは最後に実行 */
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-s") == 0)
		{
			if(lpszString[0] == '\0')
			{
				/* まだ文字列が設定されていない */
				if( (i == (argc - 1)) || (argv[i + 1][0] == '\0') )
				{
					/* 文字列が無い、もしくは最後にオプションが指定された */ 
					sprintf(lpszString, "%s\0", lpszDemoFile);
				}
				else
				{
					/* 何らかの文字列があった */
					sprintf(lpszString, "%s : %s\0", lpszDemoFile, argv[i + 1]);
					argv[i + 1][0] = '\0';
				}
				argv[i][0]     = '\0';
				break;
			}
		}
	}

	/* lpszDemoFileから、作業ディレクトリを得る */
	for(i = 0; targetList[i].szDemo[0] != '\0' ; i++)
	{
		if(strcmp(lpszDemoFile, targetList[i].szDemo) == 0)
		{
			/* ターゲットをコピー */
			strcpy(lpszTarget, targetList[i].szTarget);
			break;
		}
	}

	/* 引数をチェックし、コマンドでもファイル以外の文字列があるか調べる */
	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] != '\0')
		{
			strcpy(lpszTarget, argv[i]);
			break;
		}
	}

	if(*lpnCommand & MGDEMO_HELP)
		return 1;

	/* 引数はコマンドのみでデモファイルの指定がなかった */
	if(*lpszDemoFile == '\0')
	{
		printf("Error! Please input demo file name.\n");
		return 0;
	}

	/* Targetの指定もなく、デモファイルも登録されていない */
	if(*lpszTarget == '\0')
	{
		printf("Error! No entry demo file.(%s)\n", lpszDemoFile);
		return 0;
	}

	return 1;
}
