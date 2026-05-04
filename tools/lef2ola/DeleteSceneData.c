/*
   DeleteSceneData.c

   2002/12/11 Nobumitsu Tanaka
   $Id: DeleteSceneData.c,v 1.2 2002/12/11 06:28:36 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"

/* データのデリート */
void DeleteData(
	Data *lpData)
{
	Data *lpDataNext;

	if(lpData == NULL) return;

	while(lpData != NULL){
		lpDataNext = lpData->lpNext;

		if(lpData->lpvtVertex != NULL) Free(lpData->lpvtVertex);
		if(lpData->lpclrColor != NULL) Free(lpData->lpclrColor);
		if(lpData->lpUV != NULL) Free(lpData->lpUV);

		Free(lpData);

		lpData = lpDataNext;
	}
}

/* オブジェクトのデリート */
void DeleteObject(
	Object *lpObject)
{
	Object *lpObjectNext;

	while(lpObject != NULL){
		DeleteObject(lpObject->lpChild);
		lpObjectNext = lpObject->lpNext;

		DeleteData(lpObject->lpData);
		Free(lpObject);

		lpObject = lpObjectNext;
	}
}

/* アクションオブジェクトのデリート */
void DeleteActObj(
	Action *lpAction)
{
	int     i;
	ActObj *lpActObj;

	if(lpAction->lpActObj == NULL) return;

	for(i = 0; i < lpAction->unActObjCount; i++){
		lpActObj = lpAction->lpActObj + i;

		if(lpActObj->lpKey != NULL) Free(lpActObj->lpKey);
	}
	Free(lpAction->lpActObj);
}

/* アクションのデリート */
void DeleteAction(
	SceneData *lpSceneData)
{
	int     i;
	Action *lpAction;

	if(lpSceneData->lpAction == NULL) return;

	for(i = 0; i < lpSceneData->unActionCount; i++){
		lpAction = lpSceneData->lpAction + i;

		DeleteActObj(lpAction);
	}
	Free(lpSceneData->lpAction);

}

/* ピクチャーのデリート */
void DeletePicture(
	SceneData *lpSceneData)
{
	int     i;
	Picture *lpPicture;

	if(lpSceneData->lpPicture == NULL) return;

	for(i = 0; i < lpSceneData->unPictureCount; i++){
		lpPicture = lpSceneData->lpPicture + i;

		if(lpPicture->lpFile != NULL) Free(lpPicture->lpFile);
	}
	Free(lpSceneData->lpPicture);
}

/* シーンデータのデリート */
void DeleteSceneData(
	SceneData *lpSceneData)
{
	DeleteObject(lpSceneData->lpObject);
	DeleteAction(lpSceneData);
	DeletePicture(lpSceneData);
}
