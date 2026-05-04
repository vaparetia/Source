/*
	ReadLefObject.c

   2002/12/11 Nobumitsu Tanaka
   $Id: ReadLefObject.c,v 1.2 2002/12/11 06:28:37 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"
#include "LESceneStruct20.h"

/* Object検索 */
/* ID・データテーブルを利用して見つける */
Object* SearchObject(
	IdDataTable *lpObjectTable,
	int          nID)
{
	int   i;
	Sell *lpSell;

	for(i = 0; i < lpObjectTable->nCount; i++){
		lpSell = lpObjectTable->lpSell + i;

		if(lpSell->nID == nID) return (Object*)lpSell->lpData;
	}

	return NULL;
}

/* ピクチャーオブジェクト検索 */
Picture* SearchPicture(
	SceneData *lpSceneData,
	char      *lpszTexName)
{
	int i;

	for(i = 0; i < lpSceneData->unPictureCount; i++){
		if(strcmp((lpSceneData->lpPicture + i)->szName, lpszTexName) == 0)
			return (lpSceneData->lpPicture + i);
	}

	return NULL;
}

/* オブジェクトCommon情報を設定 */
void SetObjectCommon(
	Object          *lpObject,
	Data            *lpData,
	LEFOBJECTCOMMON *lpLEFCommon,
	char            *lpszName)
{
	lpObject->unType = lpLEFCommon->nObjectType;
	strcpy(lpObject->szName, lpszName);

	lpData->unType = 0;
	sprintf(lpData->szName, "default");

	lpData->nView  = lpLEFCommon->nVisible;
	lpData->nViewFlag = 1;

	lpData->fMagni = lpLEFCommon->fMagni;
	lpData->nMagniFlag = 1;

	/* ブレンド情報はCommonにあるのでここで設定 */
	lpData->nBlend = lpLEFCommon->nBlend;
	lpData->nBlendFlag = 1;

#ifdef DEBUG_DUMP
	printf("\n");
	printf("\t<%s>\n", lpObject->szName);
	printf("\tType = %d\n", lpObject->unType);

	printf("\t\t%s\n", lpData->szName);
	printf("\t\tView  = %d\n", lpData->nView);
	printf("\t\tMagni = %f\n", lpData->fMagni);
	printf("\t\tBlend = %d\n", lpData->nBlend);
#endif

}

/* オブジェクトVertex情報を設定 */
int SetObjectVertex(
	Data   *lpData,
	LEFOBJECTCOMMON *lpLEFCommon,
	LEFOBJECTVERTEX *lpLEFVertex,
	LEFOBJECTSTRIP  *lpLEFStrip,
	int             *lpnVertex)
{
	int          i;
	unsigned int unVertexCount;
	Vertex *lpVertex;
	Color  *lpColor;

	/* 頂点数決定 */
	switch(lpLEFCommon->nObjectType){
	case LEPRIM_NULL:
		unVertexCount = 1;
		break;

	case LEPRIM_LINE:
		unVertexCount = 2;
		break;

	case LEPRIM_RECT:
	case LEPRIM_FILLRECT:
	case LEPRIM_TEXRECT:
	case LEPRIM_SPINRECT:
	case LEPRIM_FILLPOLYGON:
		unVertexCount = 4;
		break;

	case LEPRIM_STRIP:
	case LEPRIM_FILLSTRIP:
		unVertexCount = lpLEFVertex->nVertexCount;
		if(lpLEFStrip->nCloseFlag == 1) unVertexCount++;
		break;
	}

	/* 頂点領域の確保 */
	if((lpVertex = (Vertex*)Malloc(sizeof(Vertex) * unVertexCount)) == NULL){
		printf("SetObjectVertex malloc error(0)!\n");
		return 1;
	}
	/* カラー領域の確保 */
	if((lpColor = (Color*)Malloc(sizeof(Color) * unVertexCount)) == NULL){
		printf("SetObjectVertex malloc error(1)!\n");
		return 1;
	}

	/* 頂点設定 */
	switch(lpLEFCommon->nObjectType){
	case LEPRIM_NULL:
		/* NULLの頂点はMove値 */
		lpVertex->x = lpLEFCommon->nMoveX;
		lpVertex->y = lpLEFCommon->nMoveY;

		lpColor->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 24);
		lpColor->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 16);
		lpColor->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 8);
		lpColor->a = 0x000000ff & lpLEFCommon->uiColorRGBA[0];
		break;

	case LEPRIM_LINE:
		(lpVertex + 0)->x = *(lpnVertex + 0) + lpLEFCommon->nMoveX;
		(lpVertex + 0)->y = *(lpnVertex + 1) + lpLEFCommon->nMoveY;
		(lpVertex + 1)->x = *(lpnVertex + 4) + lpLEFCommon->nMoveX;
		(lpVertex + 1)->y = *(lpnVertex + 5) + lpLEFCommon->nMoveY;

		(lpColor + 0)->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 24);
		(lpColor + 0)->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 16);
		(lpColor + 0)->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 8);
		(lpColor + 0)->a = 0x000000ff & lpLEFCommon->uiColorRGBA[0];

		(lpColor + 1)->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[1] >> 24);
		(lpColor + 1)->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[1] >> 16);
		(lpColor + 1)->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[1] >> 8);
		(lpColor + 1)->a = 0x000000ff & lpLEFCommon->uiColorRGBA[1];
		break;

	case LEPRIM_RECT:
	case LEPRIM_FILLRECT:
	case LEPRIM_TEXRECT:
	case LEPRIM_SPINRECT:
		for(i = 0; i < unVertexCount; i++){
			(lpVertex + i)->x = *(lpnVertex + (i * 2)) + lpLEFCommon->nMoveX;
			(lpVertex + i)->y = *(lpnVertex + (i * 2) + 1) + lpLEFCommon->nMoveY;

			(lpColor + i)->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 24);
			(lpColor + i)->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 16);
			(lpColor + i)->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 8);
			(lpColor + i)->a = 0x000000ff & lpLEFCommon->uiColorRGBA[0];
		}
		break;

	case LEPRIM_FILLPOLYGON:
		for(i = 0; i < unVertexCount; i++){
			(lpVertex + i)->x = *(lpnVertex + (i * 2)) + lpLEFCommon->nMoveX;
			(lpVertex + i)->y = *(lpnVertex + (i * 2) + 1) + lpLEFCommon->nMoveY;

			(lpColor + i)->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[i] >> 24);
			(lpColor + i)->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[i] >> 16);
			(lpColor + i)->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[i] >> 8);
			(lpColor + i)->a = 0x000000ff & lpLEFCommon->uiColorRGBA[i];
		}
		break;


	case LEPRIM_STRIP:
	case LEPRIM_FILLSTRIP:
		if(lpLEFStrip->nCloseFlag == 1){
			for(i = 0; i < unVertexCount - 1; i++){
				(lpVertex + i)->x = *(lpnVertex + (i * 2)) + lpLEFCommon->nMoveX;
				(lpVertex + i)->y = *(lpnVertex + (i * 2) + 1) + lpLEFCommon->nMoveY;

				(lpColor + i)->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 24);
				(lpColor + i)->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 16);
				(lpColor + i)->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 8);
				(lpColor + i)->a = 0x000000ff & lpLEFCommon->uiColorRGBA[0];
			}
			(lpVertex + i)->x = *(lpnVertex + 0) + lpLEFCommon->nMoveX;
			(lpVertex + i)->y = *(lpnVertex + 1) + lpLEFCommon->nMoveY;

			(lpColor + i)->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 24);
			(lpColor + i)->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 16);
			(lpColor + i)->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 8);
			(lpColor + i)->a = 0x000000ff & lpLEFCommon->uiColorRGBA[0];
		}else{
			for(i = 0; i < unVertexCount; i++){
				(lpVertex + i)->x = *(lpnVertex + (i * 2)) + lpLEFCommon->nMoveX;
				(lpVertex + i)->y = *(lpnVertex + (i * 2) + 1) + lpLEFCommon->nMoveY;

				(lpColor + i)->r = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 24);
				(lpColor + i)->g = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 16);
				(lpColor + i)->b = 0x000000ff & (lpLEFCommon->uiColorRGBA[0] >> 8);
				(lpColor + i)->a = 0x000000ff & lpLEFCommon->uiColorRGBA[0];
			}
		}
		break;
	}

	lpData->unVertexCount = unVertexCount;
	lpData->lpvtVertex   = lpVertex;
	lpData->nVertexFlag  = 1;
	lpData->lpclrColor   = lpColor;
	lpData->nColorFlag   = 1;

#ifdef DEBUG_DUMP
	printf("\t\tVertex Count = %d\n", lpData->unVertexCount);
	printf("\t\tVertex\n");
	for(i = 0; i < lpData->unVertexCount; i++){
		printf("\t\t\t(%d, %d)\n", (lpData->lpvtVertex + i)->x, (lpData->lpvtVertex + i)->y);
	}
	printf("\t\tColor\n");
	for(i = 0; i < lpData->unVertexCount; i++){
		printf("\t\t\t(%d, %d, %d, %d)\n", (lpData->lpclrColor + i)->r, 
			(lpData->lpclrColor + i)->g, (lpData->lpclrColor + i)->b, (lpData->lpclrColor + i)->a);
	}
#endif
	return 0;
}

/* オブジェクトテクスチャ情報を設定 */
int SetObjectTex(
	SceneData    *lpSceneData,
	Data         *lpData,
	LEFOBJECTTEX *lpLEFTex,
	char         *lpszTexName)
{
	Picture *lpPicture;

	/* ピクチャーを取得 */
	if((lpPicture = SearchPicture(lpSceneData, lpszTexName)) == NULL){
		printf("\t\tNot found picture!(%s)\n", lpszTexName);
		lpData->nUVFlag = 0;
		lpData->nPictureFlag = 0;
		return 0;
	}

	/* データ設定 */
	strcpy(lpData->szPictureName, lpszTexName);
	lpData->nPictureFlag = 1;

	strcpy(lpData->szTRIName, lpPicture->szTRIName);

	/* UV値メモリ確保 */
	if((lpData->lpUV = (UV*)Malloc(sizeof(UV) * lpData->unVertexCount)) == NULL){
		printf("SetObjectTex malloc error(0)!\n");
		return 1;
	}
	(lpData->lpUV + 0)->u = (float)lpLEFTex->arynUV[0] / (float)lpPicture->nWidth;
	(lpData->lpUV + 0)->v = (float)lpLEFTex->arynUV[1] / (float)lpPicture->nHeight;

	(lpData->lpUV + 2)->u = (float)(lpLEFTex->arynUV[0] + lpLEFTex->arynUVSize[0]) / (float)lpPicture->nWidth;
	(lpData->lpUV + 2)->v = (float)(lpLEFTex->arynUV[1] + lpLEFTex->arynUVSize[1]) / (float)lpPicture->nHeight;

	(lpData->lpUV + 1)->u = (lpData->lpUV + 2)->u;
	(lpData->lpUV + 1)->v = (lpData->lpUV + 0)->v;

	(lpData->lpUV + 3)->u = (lpData->lpUV + 0)->u;
	(lpData->lpUV + 3)->v = (lpData->lpUV + 2)->v;

	lpData->nUVFlag = 1;

#ifdef DEBUG_DUMP
	printf("\t\tPicture Name = %s\n", lpData->szPictureName);
	printf("\t\tTRI Name = %s\n", lpData->szTRIName);
	{
		int i;
		for(i = 0; i < lpData->unVertexCount; i++){
			printf("\t\t\t(%f, %f)\n", (lpData->lpUV + i)->u, (lpData->lpUV + i)->v);
		}
	}
#endif

	return 0;
}

/* オブジェクトスピン情報を設定 */
void SetObjectSpin(
	Data          *lpData,
	LEFOBJECTSPIN *lpLEFSpin)
{
	/* データ設定 */
	lpData->nRotate     = lpLEFSpin->nRotate;
	lpData->nRotateFlag = 1;

	lpData->nRotateMode = lpLEFSpin->nRotateMode;
	lpData->nRotateModeFlag = 1;

	lpData->vtSpinCenter.x = lpLEFSpin->nSpinCenterX;
	lpData->vtSpinCenter.y = lpLEFSpin->nSpinCenterY;
	lpData->nSpinCenterFlag = 1;

#ifdef DEBUG_DUMP
	printf("\t\tRotate = %d\n", lpData->nRotate);
	printf("\t\tRotateMode = %d\n", lpData->nRotateMode);
	printf("\t\tCenter = (%d, %d)\n", lpData->vtSpinCenter.x, lpData->vtSpinCenter.y);

#endif

}

/* オブジェクト読み込み */
int ReadLefObjectData(
	FILE        *fp,
	SceneData   *lpSceneData,
	Object      *lpObjectParent,
	IdDataTable *lpActorTable,
	IdDataTable *lpObjectTable,
	ObjectDummy *lpObjectDummy)
{
	int              i;
	int              nVertexCount;
	int             *lpnVertex;
	char             szName[MAX_TEXT];
	char             szTexName[MAX_TEXT];
	Data            *lpData;
	Object          *lpObject;
	Object          *lpObjectPrev;
	LEFOBJECTCOMMON    lefObjectCommon;
	LEFOBJECTVERTEX    lefObjectVertex;
	LEFOBJECTTEX       lefObjectTex;
	LEFOBJECTSPIN      lefObjectSpin;
	LEFOBJECTSTRIP     lefObjectStrip;
	LEFOBJECTFILLSTRIP lefObjectFillStrip;

	/* 共通オブジェクトデータ読み込み */
	fread(&lefObjectCommon, sizeof(LEFOBJECTCOMMON), 1, fp);
	/* オブジェクト名読み込み */
	fread(szName, sizeof(char), lefObjectCommon.nNameLength, fp);
	szName[lefObjectCommon.nNameLength] = '\0';

	/* 文字列長さチェック */
	if(lefObjectCommon.nNameLength >= MAX_NAME){
		printf("Object Name length over!\n");
		return 1;
	}

	/* NULLオブジェクト以外はすべて頂点を持つｓ */
	if(lefObjectCommon.nObjectType != LEPRIM_NULL){
		/* 頂点列の読み込み */
		fread(&lefObjectVertex, sizeof(LEFOBJECTVERTEX), 1, fp);
		if((lpnVertex = (int*)Malloc(
			sizeof(int) * lefObjectVertex.nVertexCount * 2)) == NULL){
			printf("ReadLefObjectData malloc error(0)!\n");
			return 1;
		}
		fread(lpnVertex, sizeof(int), lefObjectVertex.nVertexCount * 2, fp);
	}

	/* テクスチャオブジェクト */
	if( (lefObjectCommon.nObjectType == LEPRIM_TEXRECT) ||
		(lefObjectCommon.nObjectType == LEPRIM_SPINRECT) ||
		(lefObjectCommon.nObjectType == LEPRIM_TEXPOLYGON)){
		/* テクスチャ情報読み込み */
		fread(&lefObjectTex, sizeof(LEFOBJECTTEX), 1, fp);

		/* テクスチャ名読み込み */
		fread(szTexName, sizeof(char), lefObjectTex.nTexNameLength, fp);
		szTexName[lefObjectTex.nTexNameLength] = '\0';

		/* 文字列長さチェック */
		if(lefObjectTex.nTexNameLength >= MAX_NAME){
			printf("Object Tex Name length over!\n");
			return 1;
		}
	}

	/* スピンオブジェクト */
	if(lefObjectCommon.nObjectType == LEPRIM_SPINRECT){
		/* テクスチャ情報読み込み */
		fread(&lefObjectSpin, sizeof(LEFOBJECTSPIN), 1, fp);
	}

	/* ラインストリップオブジェクト */
	if( (lefObjectCommon.nObjectType == LEPRIM_STRIP) ||
		(lefObjectCommon.nObjectType == LEPRIM_FILLSTRIP) ){
		/* ラインストリップ情報読み込み */
		fread(&lefObjectStrip, sizeof(LEFOBJECTSTRIP), 1, fp);
	}

	/* 塗り潰しラインストリップオブジェクト */
	if(lefObjectCommon.nObjectType == LEPRIM_FILLSTRIP){
		/* 塗り潰しラインストリップ情報読み込み */
		fread(&lefObjectFillStrip, sizeof(LEFOBJECTFILLSTRIP), 1, fp);
	}

	/* 中間データオブジェクト構造体作成 */
	if((lpObject = (Object*)Malloc(sizeof(Object))) == NULL){
		printf("ReadLefObjectData malloc error(0)!\n");
		return 1;
	}
	/* 中間データ構造体作成 */
	if((lpData = (Data*)Malloc(sizeof(Data))) == NULL){
		printf("ReadLefObjectData malloc error(1)!\n");
		return 1;
	}
	memset(lpData, 0x00, sizeof(Data));

	/* データ設定 */
	lpObject->lpNext      = NULL;
	lpObject->lpChild     = NULL;
	lpObject->nObjectID   = 0;
	lpObject->unDataCount = 0;
	lpObject->lpData      = NULL;

	lpData->lpNext  = NULL;
	lpData->nOffset = 0;
	lpData->unID    = 0;	/* 識別用ID。"default"は必ず0 */

	/* 共通データ設定 */
	SetObjectCommon(lpObject, lpData, &lefObjectCommon, szName);

	/* 頂点データ設定 */
	if(SetObjectVertex(lpData, &lefObjectCommon, &lefObjectVertex,
		&lefObjectStrip, lpnVertex) != 0){
		/* エラー */
		return 1;
	}
	// 使用した頂点データの開放
	Free(lpnVertex);

	/* テクスチャデータ設定 */
	/* テクスチャオブジェクト以外は無視 */
	if( (lefObjectCommon.nObjectType == LEPRIM_TEXRECT) ||
		(lefObjectCommon.nObjectType == LEPRIM_SPINRECT) ||
		(lefObjectCommon.nObjectType == LEPRIM_TEXPOLYGON)){
		if(SetObjectTex(lpSceneData, lpData, &lefObjectTex, szTexName) != 0){
			/* エラー */
			return 1;
		}
	}else{
		lpData->nPictureFlag = 0;
		lpData->nBlendFlag = 0;
		lpData->nUVFlag = 0;
	}

	/* スピンデータ設定 */
	/* スピンオブジェクト以外は無視 */
	if(lefObjectCommon.nObjectType == LEPRIM_SPINRECT){
		SetObjectSpin(lpData, &lefObjectSpin);
	}else{
		lpData->nRotateFlag = 0;
		lpData->nRotateModeFlag = 0;
		lpData->nSpinCenterFlag = 0;
	}

#if 0
	/* ラインストリップ系はこちらのフラグの扱いが決まっていないので保留 */
	SetObjectStrip(lpObject, lpData, &lefObjectStrip);
	SetObjectFillStrip(lpObject, lpData, &lefObjectFillStrip);
#endif

	/* リンク設定 */
	if(lpObjectParent != NULL){
		if(lpObjectParent->lpChild != NULL){
			Object *lpPrev;

			/* 兄弟の最後まで行く */
			lpPrev = lpObjectParent->lpChild;
			while(lpPrev->lpNext != NULL){
				lpPrev = lpPrev->lpNext;
			}

			/* このオブジェクトの後につなぐ */
			lpPrev->lpNext = lpObject;
		}else{
			/* 最初の子だった */
			lpObjectParent->lpChild = lpObject;
		}
	}else{
		/* 親オブジェクトがNULLなので、ルートに設定 */
		lpSceneData->lpObject = lpObject;
	}
	lpSceneData->unObjectCount++;

	/* Dataの登録 */
	lpObject->lpData = lpData;
	lpObject->unDataCount++;

	/* 検索用オブジェクトIDテーブルの登録 */
	(lpObjectTable->lpSell + lpObjectTable->nCount)->nID = lefObjectCommon.nOriginalNumber;
	(lpObjectTable->lpSell + lpObjectTable->nCount)->lpData = lpObject;
	lpObjectTable->nCount++;

	/* アニメ用ダミーデータに登録 */
	lpObjectDummy->nCount   = 0;
	lpObjectDummy->lpAnmObj = NULL;
	lpObjectDummy->lpObject = lpObject;

	/* アニメオブジェクトの読み込み */
	if(lefObjectCommon.nAnimeObjectCount != 0){
		ReadLefAnime(fp, lpSceneData, lpObject, lpObjectDummy, lefObjectCommon.nAnimeObjectCount);
	}

	return 0;
}

/* LEFオブジェクト読み込み */
int ReadLefObject(
	FILE      *fp,
	SceneData *lpSceneData,
	LEFLESCENEHEADER  *lpLEFSceneHeader,
	IdDataTable       *lpActorTable,
	AnimeObjectHeader *lpAnmObjHeader)
{
	int                    i, j;
	ObjectDummy           *lpObjectDummy;
	IdDataTable            ObjectTable;
	LEFLEOGROUPHEADER      lefObjectGroup;
	LEFLEOBJECTTREEHEADER  lefObjectHeader;

	/* オブジェクトヘッダー読み込み */
	fseek(fp, lpLEFSceneHeader->lLEObject, SEEK_SET);
	fread(&lefObjectHeader, sizeof(LEFLEOBJECTTREEHEADER), 1, fp);

	/* オブジェクトが無ければ終了 */
	if(lefObjectHeader.nTotalObjectCount == 0) return 0;

	/* ルートオブジェクトはカウントされなていないのでその分を足す */
	lefObjectHeader.nTotalObjectCount++;

#ifdef DEBUG_DUMP
	printf("\n");
	printf("Object\n");
	printf("\tObject Count = %d\n", lefObjectHeader.nTotalObjectCount);
#endif

	/* オブジェクトID・名称用テーブル */
	if((ObjectTable.lpSell = (Sell*)Malloc(
		sizeof(Sell) * lefObjectHeader.nTotalObjectCount)) == NULL){
		printf("ReadLefObject malloc error(0)!\n");
		return 1;
	}
	ObjectTable.nCount = 0;

	/* アニメオブジェクト用 */
	if((lpAnmObjHeader->lpObjList = (ObjectDummy*)Malloc(
		sizeof(ObjectDummy) * lefObjectHeader.nTotalObjectCount)) == NULL){
		printf("ReadLefObject malloc error(1)!\n");
		return 1;
	}

	for(i = 0; i < lefObjectHeader.nTotalObjectCount; ){
		Object *lpObjectParent;

		/* オブジェクトグループ読み込み */
		fread(&lefObjectGroup, sizeof(LEFLEOGROUPHEADER), 1, fp);

		/* 親オブジェクト取得 */
		lpObjectParent = SearchObject(&ObjectTable, lefObjectGroup.nParentObjectNumber);

		for(j = 0; j < lefObjectGroup.nGroupObjectCount; j++){
			/* オブジェクト読み込み */
			lpObjectDummy = lpAnmObjHeader->lpObjList + lpAnmObjHeader->nCount;
			if( ReadLefObjectData(fp, lpSceneData, lpObjectParent,
				lpActorTable, &ObjectTable, lpObjectDummy ) != 0){
				/* エラー */
				return 1;
			}
			lpAnmObjHeader->nCount++;
			i++;
		}
	}

	/* 領域開放 */
	Free(ObjectTable.lpSell);

	return 0;
}


