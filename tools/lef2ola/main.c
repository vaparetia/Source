/*
   main.c

   2002/12/11 Nobumitsu Tanaka
   $Id: main.c,v 1.5 2003/01/06 07:36:21 usr04761 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"

#ifdef DEBUG
int nMemorySize = 0;
int nAllocCount = 0;
AllocTable allocTbl[2048];

void* Malloc(size_t size){
	long lPointer;

	nMemorySize += size;

	lPointer = (long)malloc(size);

printf("alloc %d(%d)\n", size, nAllocCount);

	allocTbl[nAllocCount].nID      = nAllocCount;
	allocTbl[nAllocCount].nSize    = size;
	allocTbl[nAllocCount].lPointer = lPointer;
	nAllocCount++;

	return (void*)lPointer;
}

void Free(void* p){
	int i;

	for(i = 0; i < nAllocCount; i++){
		if(allocTbl[i].lPointer == (long)p){
			nMemorySize -= allocTbl[i].nSize;
			allocTbl[i].lPointer = 0;
printf("free %d(%d)\n", allocTbl[i].nSize, allocTbl[i].nID);
			break;
		}
	}

	free(p);
}
#endif

/* 使用法 */
void Usage( void )
{
	printf( "\nlef2ola : LEF Scene - file -> mgs3 - ola converter\n" ) ;
	printf( "usage : lef2ola input(*.lef) output(*.ola)\n" ) ;
	printf( "\t<options>\n" ) ;
}

/* ConvertInfo構造体の初期化 */
void InitConvertInfo(
	ConvertInfo *lpInfo)
{
	lpInfo->szInputPath[0]  = '\0';
	lpInfo->szOutputPath[0] = '\0';

	lpInfo->scnData.unVersion = 0;

	lpInfo->scnData.nFieldWidth = 0;
	lpInfo->scnData.nFieldHeight = 0;

	lpInfo->scnData.unObjectCount = 0;
	lpInfo->scnData.lpObject = NULL;

	lpInfo->scnData.unActionCount = 0;
	lpInfo->scnData.lpAction = NULL;

	lpInfo->scnData.unPictureCount = 0;
	lpInfo->scnData.lpPicture = NULL;
}

/* メイン */
int main(int argc, char *argv[])
{
	ConvertInfo  info;

	/* コンバート情報を初期化 */
	InitConvertInfo(&info);

	/* 引数解釈 */
	if(Analyze(argc, argv, &info) != 0){
		/* 引数が不正 */
		Usage();
		return 1;
	}

	/* lef読み込み、および中間データ生成 */
	if(Lef2SceneData(&info) != 0){
		DeleteSceneData(&(info.scnData));
		return 1;
	}

	/* 中間データのダンプ */
//	DumpSceneData(&info);
	

	/* 中間データのlaバイナリデータの生成・出力 */
	SceneData2Ola( &info );

	/* 中間データの削除 */
	DeleteSceneData(&(info.scnData));

#ifdef DEBUG
	printf("Alloc Memory = %d\n", nMemorySize);
#endif

	return 0 ;
}
