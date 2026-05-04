/******************************************************************************
 * System	: Scene To HZX File Convert exe exe
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: AnalyzeCommand
 */

/******************************************************************************
 * included
 */
#include <stdio.h>

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

#include "Scene2HZX.h"

/******************************************************************************
 * コマンド引数解析
 */

extern	int	EasyStrip ;

int AnalyzeCommand(		/* 0 以外 正常 */
	int             argc,				/* コマンドライン引数の数 */
	char            **argv,			/* コマンドライン引数 */
	LPSCENE2HZXINFO lpScene2HZXInfo)	/* 読み出すシーンリスト */
{
	int i;
	int nJobType;	/* 0 : 処理を行わず */
					/* 1 : 読み込みを行うファイル名 */
					/* 2 : 書き込みを行うファイル名 */

	/* 初期化 */
	lpScene2HZXInfo->szInputFileName[0]  = '\0';
	lpScene2HZXInfo->szOutputFileName[0] = '\0';
	strcpy( lpScene2HZXInfo->szOutputDirName, "." ) ;
	lpScene2HZXInfo->nOldHZXSceneFlag     = 0;
	lpScene2HZXInfo->DebugMode     = 0;
	EasyStrip = 1 ;
	for(i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 'i':
				nJobType = 1;
				break;

			case 'o':
				nJobType = 2;
				break;
			case 'd' :
			        nJobType = 3 ;
				break ;
			case 'k':
				lpScene2HZXInfo->nOldHZXSceneFlag = 1;
				break;
			case 'v' :
				nJobType = 0;
			        lpScene2HZXInfo->DebugMode = 1 ;
				break ;
			case 'l' :
				nJobType = 0;
			        lpScene2HZXInfo->DebugMode = 2 ;
				break ;
			case 'e' :
			        nJobType = 0;
				EasyStrip = 0 ;
				break ;
			default:
				nJobType = 0;
				break;
			}

			if(nJobType != 0)
			{
				if( (argc > (i + 1)) && (argv[i + 1][0] != '-') && 
					(argv[i + 1] != '\0'))
				{
					if(nJobType == 1)
						strcpy(lpScene2HZXInfo->szInputFileName, argv[i + 1]);
					else if ( nJobType == 2 ) 
						strcpy(lpScene2HZXInfo->szOutputFileName, argv[i + 1]);
					else 
					        strcpy(lpScene2HZXInfo->szOutputDirName, argv[i + 1]);
				}
			}
		}
	}

	if(lpScene2HZXInfo->szInputFileName[0] == '\0')
	{
		printf("Error! No Input File Name.");
		return 0;
	}
	if ( lpScene2HZXInfo->szOutputFileName[0] == '\0' ) {
	    char	*cp ;

	    strcpy( lpScene2HZXInfo->szOutputFileName, lpScene2HZXInfo->szInputFileName ) ;
	    cp = strrchr( lpScene2HZXInfo->szOutputFileName, '.' ) ;
	    if ( cp != NULL ) {
		cp ++ ;
		strcpy( cp, "hzx" ) ;
	    }
	}
	return 1;
}
