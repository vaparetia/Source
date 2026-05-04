/*
	Analyze.c

   2002/12/11 Nobumitsu Tanaka
   $Id: Analyze.c,v 1.2 2002/12/11 06:28:36 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"

/* 引数解析 */
int Analyze(int argc, char *argv[], ConvertInfo *lpInfo)
{
	int nLength;

	/* 引数が無いときは無条件に終了 */
	if(argc <= 1) return 1;

	/* 入力ファイル読み込み */
	/* 今は仮。最終的には拡張子の有無。拡張子の正当性もチェック */
	strcpy(lpInfo->szInputPath, argv[1]);

	if(argc == 2){
		/* 出力ファイルが指定されていない */
		/* 今は仮。拡張子有りが前提 */
		strcpy(lpInfo->szOutputPath, argv[1]);

		nLength = strlen(lpInfo->szOutputPath);
		lpInfo->szOutputPath[nLength - 3] = 'o';
		lpInfo->szOutputPath[nLength - 2] = 'l';
		lpInfo->szOutputPath[nLength - 1] = 'a';
	}else{
		/* 出力ファイル指定 */
		strcpy(lpInfo->szOutputPath, argv[2]);
	}

printf("Input  = %s\n", lpInfo->szInputPath);
printf("Output = %s\n", lpInfo->szOutputPath);

	return 0;
}
