/*
   DumpSceneData.c

   2002/12/11 Nobumitsu Tanaka
   $Id: DumpSceneData.c,v 1.2 2002/12/11 06:28:36 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"

/* データのダンプ */
void DumpData(
	Data *lpData)
{
	int i;
	while(lpData != NULL){
		printf("\t\t[%s]\n", lpData->szName);

		printf("\t\t\tID = %d\n", lpData->unID);

		if(lpData->nVertexFlag == 1){
			printf("\t\t\tVertex\n");
			printf("\t\t\t\tVertex Count = %d\n", lpData->unVertexCount);
			for(i = 0; i < lpData->unVertexCount; i++){
				printf("\t\t\t\t(%d, %d)\n",
					(lpData->lpvtVertex + i)->x, (lpData->lpvtVertex + i)->y);
			}
		}

		if(lpData->nColorFlag == 1){
			printf("\t\t\tColor\n");
			for(i = 0; i < lpData->unVertexCount; i++){
				printf("\t\t\t\t(%d, %d, %d, %d)\n",
					(lpData->lpclrColor + i)->r, (lpData->lpclrColor + i)->g,
					(lpData->lpclrColor + i)->b, (lpData->lpclrColor + i)->a);
			}
		}

		if(lpData->nViewFlag == 1){
			printf("\t\t\tView = %d\n", lpData->nView);
		}

		if(lpData->nMagniFlag == 1){
			printf("\t\t\tMagni = %f\n", lpData->fMagni);
		}

		if(lpData->nBlendFlag == 1){
			printf("\t\t\tBlend = %d\n", lpData->nBlend);
		}

		if(lpData->nPictureFlag == 1){
			printf("\t\t\tPicture Name = %s\n", lpData->szPictureName);
			printf("\t\t\tTRI Name = %s\n", lpData->szTRIName);
		}

		if(lpData->nUVFlag == 1){
			printf("\t\t\tUV\n");
			for(i = 0; i < lpData->unVertexCount; i++){
				printf("\t\t\t\t(%d, %d)\n",
					(lpData->lpUV + i)->u, (lpData->lpUV + i)->v);
			}
		}

		if(lpData->nRotateFlag){
			printf("\t\t\tRotate = %d\n", lpData->nRotate);
		}

		if(lpData->nRotateModeFlag == 1){
			printf("\t\t\tRotateMode = %d\n", lpData->nRotateMode);
		}

		if(lpData->nSpinCenterFlag == 1){
			printf("\t\t\tSpinCenter = (%d, %d)\n",
				lpData->vtSpinCenter.x, lpData->vtSpinCenter.y);
		}

		lpData = lpData->lpNext;
	}
}

/* オブジェクトのダンプ */
void DumpObject(
	Object *lpObject,
	int     nLevel)
{
	while(lpObject != NULL){
		printf("\n");
		printf("\t<%s><%d>\n", lpObject->szName, nLevel);

		printf("\t\tType = %d\n", lpObject->unType);
		printf("\t\tData Count = %d\n", lpObject->unDataCount);

		DumpData(lpObject->lpData);

		DumpObject(lpObject->lpChild, nLevel + 1);
		lpObject = lpObject->lpNext;
	}
}

/* キーフレーム情報のダンプ */
void DumpKey(
	ActObj *lpActObj)
{
	int  i;
	Key *lpKey;

	for(i = 0; i < lpActObj->unKeyCount; i++){
		lpKey = lpActObj->lpKey + i;

		printf("\t\t\t\tFrame = %d\n", lpKey->nFrame);
		printf("\t\t\t\tType  = %d\n", lpKey->unType);
		printf("\n");
	}

}

/* アクションオブジェクトのダンプ */
void DumpActObj(
	Action *lpAction)
{
	int     i;
	ActObj *lpActObj;

	for(i = 0; i < lpAction->unActObjCount; i++){
		lpActObj = lpAction->lpActObj + i;

		printf("\t\t- %s -\n", lpActObj->lpObject->szName);
		printf("\t\t\tKey Count = %d\n", lpActObj->unKeyCount);
		DumpKey(lpActObj);
	}
}

/* アクションのダンプ */
void DumpAction(
	SceneData *lpSceneData)
{
	int     i;
	Action *lpAction;

	for(i = 0; i < lpSceneData->unActionCount; i++){
		lpAction = lpSceneData->lpAction + i;

		printf("\n");
		printf("\t<%s>\n", lpAction->szName);
		printf("\t\tFrame[%d - %d]\n",
			lpAction->nFrameMin, lpAction->nFrameMax);

		printf("\t\tActObj Count = %d\n", lpAction->unActObjCount);

		DumpActObj(lpAction);
	}
}

/* ピクチャーのダンプ */
void DumpPicture(
	SceneData *lpSceneData)
{
	int     i;
	Picture *lpPicture;

	for(i = 0; i < lpSceneData->unPictureCount; i++){
		lpPicture = lpSceneData->lpPicture + i;

		printf("\n");
		printf("\t<%s>\n", lpPicture->szName);
	}
}

/* 中間データのダンプ */
void DumpSceneData(
	ConvertInfo *lpInfo)
{
	SceneData *lpSceneData;

	lpSceneData = &(lpInfo->scnData);

	printf("\n");
	printf("Version - %d\n", lpSceneData->unVersion);

	printf("\n");
	printf("Field Width  = %d\n", lpSceneData->nFieldWidth);
	printf("Field Height = %d\n", lpSceneData->nFieldHeight);

	printf("\n");
	printf("Object\n");
	printf("\tObjectCount = %d\n", lpSceneData->unObjectCount);

	DumpObject(lpSceneData->lpObject, 1);

	printf("\n");
	printf("Action\n");
	printf("\tActionCount = %d\n", lpSceneData->unActionCount);
	DumpAction(lpSceneData);

	printf("\n");
	printf("Picture\n");
	printf("\tPictureCount = %d\n", lpSceneData->unPictureCount);
	DumpPicture(lpSceneData);
}
