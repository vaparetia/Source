/*
	SetKeyData.c

   2002/12/11 Nobumitsu Tanaka
   $Id: SetKeyData.c,v 1.2 2002/12/11 06:28:37 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"
#include "LESceneStruct20.h"

Action* SearchAction(
	IdDataTable *lpTblActor,
	int          nID)
{
	int i;

	for(i = 0; i < lpTblActor->nCount; i++){
		Sell *lpSell;

		lpSell = lpTblActor->lpSell + i;

		if(lpSell->nID == nID) return (Action*)lpSell->lpData;
	}
	return NULL;
}


/* Actionにキー情報を割り当てる */
int SetKeyData(
	SceneData         *lpSceneData,
	IdDataTable       *lpTblActor,
	AnimeObjectHeader *lpAnmObjectHeader)
{
	int     i, j;
	Action *lpAction;

	/* まずアクションオブジェクトを確保してしまう */
	for(i = 0; i < lpSceneData->unActionCount; i++){
		lpAction = lpSceneData->lpAction + i;

		if((lpAction->lpActObj = (ActObj*)Malloc(
			sizeof(ActObj) * lpSceneData->unObjectCount)) == NULL){
			printf("SetKeyData malloc error(0)!\n");
			return 1;
		}
	}

	/* ダミーデータを参照して、Actionにキーデータを割り当てていく */
	for(i = 0; i < lpAnmObjectHeader->nCount; i++){
		ObjectDummy *lpObjectDummy;

		lpObjectDummy = lpAnmObjectHeader->lpObjList + i;

		for(j = 0; j < lpObjectDummy->nCount; j++){
			ActObj           *lpActObj;
			AnimeObjectDummy *lpAnimeDummy;

			/* Animeオブジェクトのダミー取得 */
			lpAnimeDummy = lpObjectDummy->lpAnmObj + j;

			/* IDからアクションを取得 */
			if((lpAction = SearchAction(lpTblActor, lpAnimeDummy->nID)) == NULL){
				printf("Error! not found Action!\n");
				return 1;
			}

			lpActObj = lpAction->lpActObj + lpAction->unActObjCount;

			lpActObj->lpObject = (Object*)lpObjectDummy->lpObject;
			lpActObj->unKeyCount = lpAnimeDummy->nCount;
			lpActObj->lpKey      = (Key*)lpAnimeDummy->lpKey;

			lpAnimeDummy->lpKey = NULL;

			lpAction->unActObjCount++;
		}
	}

	return 0;
}
