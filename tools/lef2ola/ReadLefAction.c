/*
	ReadLefAction.c

   2002/12/11 Nobumitsu Tanaka
   $Id: ReadLefAction.c,v 1.2 2002/12/11 06:28:36 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"
#include "LESceneStruct20.h"

/* アクションの読み込み、および中間データを作成する */
int ReadLefAction(
	FILE      *fp,
	SceneData *lpSceneData,
	LEFLESCENEHEADER *lpLEFSceneHeader,
	IdDataTable      *lpActorTable)
{
	int                   i;
	char                  szName[MAX_TEXT];
	Action               *lpAction;
	LEFLEANIMEACTOR       lefActor;
	LEFLEANIMETREEHEADER  lefActorHeader;

	/* アクターヘッダー読み込み */
	fseek(fp, lpLEFSceneHeader->lLEAnime, SEEK_SET);
	fread(&lefActorHeader, sizeof(LEFLEANIMETREEHEADER), 1, fp);

#ifdef DEBUG_DUMP
	printf("\n");
	printf("Action\n");
	printf("\tAction Count = %d\n", lefActorHeader.nAnimeActorCount);
#endif

	/* 中間データアクション配列作成 */
	if((lpSceneData->lpAction = (Action*)Malloc(
		sizeof(Action) * lefActorHeader.nAnimeActorCount)) == NULL){
		printf("ReadLefAction malloc error(0)!\n");
		return 1;
	}

	/* IDと名称の対応テーブルの生成 */
	if((lpActorTable->lpSell = (Sell*)Malloc(
		sizeof(Sell) * lefActorHeader.nAnimeActorCount)) == NULL){
		printf("ReadLefAction malloc error(1)!\n");
		return 1;
	}

	/* アクター読み込み */
	for(i = 0; i < lefActorHeader.nAnimeActorCount; i++){
		/* LEFアクター情報の読み込み */
		fread(&lefActor, sizeof(LEFLEANIMEACTOR), 1, fp);
		/* ファイル名読み込み */
		fread(szName, sizeof(char), lefActor.nActorNameLength, fp);
		szName[lefActor.nActorNameLength] = '\0';

		/* 文字列長さチェック */
		if(lefActor.nActorNameLength >= MAX_NAME){
			printf("Action Name length over!\n");
			return 1;
		}
	
		/* データ設定 */
		lpAction = lpSceneData->lpAction + i;

		lpAction->nFrameMin = lefActor.nRangeStart;
		lpAction->nFrameMax = lefActor.nRangeEnd;
		strcpy(lpAction->szName, szName);

		lpAction->unActObjCount = 0;
		lpAction->lpActObj      = NULL;

#ifdef DEBUG_DUMP
		printf("\n");
		printf("\tAction Name = %s\n", lpAction->szName);
		printf("\tFrame Min   = %d\n", lpAction->nFrameMin);
		printf("\tFrame Max   = %d\n", lpAction->nFrameMax);
#endif

		lpSceneData->unActionCount++;

		/* テーブル設定 */
		(lpActorTable->lpSell + lpActorTable->nCount)->nID    = lefActor.nAnimeID;
		(lpActorTable->lpSell + lpActorTable->nCount)->lpData = lpAction;;
		lpActorTable->nCount++;
	}

	return 0;
}
