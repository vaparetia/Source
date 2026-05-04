/*
	Lef2ScenData.c

   2002/12/11 Nobumitsu Tanaka
   $Id: Lef2SceneData.c,v 1.2 2002/12/11 06:28:36 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"
#include "LESceneStruct20.h"

/* LEFファイルチェック、およびバージョンチェック */
int CheckLef(FILE *fp)
{
	char         szFileType[32];
	unsigned int unVersion;

	fread(szFileType, sizeof(char), 32, fp);
	fread(&unVersion, sizeof(unsigned int), 1, fp);

	/* ファイルタイプ確認 */
	if(strcmp(szFileType, FILE_TYPE) != 0){
		printf("This file is not LEF format file!\n");
		return 1;
	}
	/* ファイルバージョン確認 */
	if(unVersion != LEF_VERSION){
		printf("This version do not support.\n");
		printf("Supported version is \"19\".\n");
		return 1;
	}

#ifdef DEBUG_DUMP
	printf("\n");
	printf("File Type    = %s\n", szFileType);
	printf("File Version = %d\n", unVersion);
#endif

	return 0;
}

/* LEFヘッダー部・シーン情報読み込み */
int ReadLefHeader(
	FILE      *fp,
	SceneData *lpSceneData,
	LEFLESCENEHEADER *lpLEFSceneHeader)
{
	LEFLESCENEINFO   lefSceneInfo;

	/* ヘッダ読み込み */
	fseek(fp, 0, SEEK_SET);
	fread(lpLEFSceneHeader, sizeof(LEFLESCENEHEADER), 1, fp);

	lpSceneData->unVersion = lpLEFSceneHeader->uiVersion;

	/* シーン情報取得 */
	fseek(fp, lpLEFSceneHeader->lLEScene, SEEK_SET);
	fread(&lefSceneInfo, sizeof(LEFLESCENEINFO), 1, fp);

	lpSceneData->nFieldWidth  = lefSceneInfo.nFieldWidth;
	lpSceneData->nFieldHeight = lefSceneInfo.nFieldHeight;

#ifdef DEBUG_DUMP
	printf("\n");
	printf("SceneData Header\n");
	printf("\tVersion = %d\n", lpSceneData->unVersion);
	printf("\tFieldWidth  = %d\n", lpSceneData->nFieldWidth);
	printf("\tFieldHeight = %d\n", lpSceneData->nFieldHeight);
#endif

	return 0;
}

/* オブジェクトのタイプを実機用にあわせる */
void ChangeObjectType(
	Object *lpObject)
{
	while(lpObject != NULL){
		/* LEのオブジェクトタイプをLAにあわせる */
		switch(lpObject->unType){
		case LEPRIM_NULL:
			lpObject->unType = LA_EMPTY;
			break;

		case LEPRIM_LINE:
			lpObject->unType = LA_LINE;
			break;

		case LEPRIM_FILLRECT:
			lpObject->unType = LA_BOX;
			break;

		case LEPRIM_TEXRECT:
		case LEPRIM_SPINRECT:
			lpObject->unType = LA_SPRT;
			break;

		case LEPRIM_FILLPOLYGON:
			lpObject->unType = LA_QUAD;
			break;

		case LEPRIM_RECT:
		case LEPRIM_STRIP:
		case LEPRIM_FILLSTRIP:
			lpObject->unType = LA_LINESTRIP;
			break;
		}

		ChangeObjectType(lpObject->lpChild);

		lpObject = lpObject->lpNext;
	}
}

/* LEFファイルを読み込み、中間データを作成する */
int ReadLefFile(
	FILE      *fp,
	SceneData *lpSceneData)
{
	int               i, j;
	IdDataTable       tblActor;
	AnimeObjectHeader anmObjectHeader;
	LEFLESCENEHEADER  lefSceneHeader;

	/* ヘッダ部読み込み・中間データ設定 */
	if(ReadLefHeader(fp, lpSceneData, &lefSceneHeader) != 0){
		/* エラー */
		return 1;
	}

	/* ピクチャーファイル読み込み、および中間データ作成 */
	if(ReadLefPicture(fp, lpSceneData, &lefSceneHeader) != 0){
		/* エラー */
		return 1;
	}

	/* アクション読み込み、および中間データの作成 */
	tblActor.nCount = 0;
	tblActor.lpSell = NULL;
	if(ReadLefAction(fp, lpSceneData, &lefSceneHeader, &tblActor) != 0){
		/* エラー */
		return 1;
	}

	/* オブジェクト読み込み、および中間データの作成 */
	anmObjectHeader.nCount    = 0;
	anmObjectHeader.lpObjList = NULL;
	if(ReadLefObject(fp, lpSceneData, &lefSceneHeader, &tblActor, &anmObjectHeader) != 0){
		/* エラー */
		return 1;
	}

	/* アクションデータに、キーデータを割り当てる */
	if(SetKeyData(lpSceneData, &tblActor, &anmObjectHeader) != 0){
		/* エラー */
		return 1;
	}

	/* データを開放 */
	if(tblActor.lpSell != NULL) Free(tblActor.lpSell);

	for(i = 0; i < anmObjectHeader.nCount; i++){
		ObjectDummy *lpObjectDummy;

		lpObjectDummy = anmObjectHeader.lpObjList + i;

		for(j = 0; j < lpObjectDummy->nCount; j++){
			if((lpObjectDummy->lpAnmObj + j)->lpKey != NULL) Free((lpObjectDummy->lpAnmObj + j)->lpKey);
		}
		if(lpObjectDummy->lpAnmObj != NULL) Free(lpObjectDummy->lpAnmObj);
	}
	if(anmObjectHeader.lpObjList != NULL) Free(anmObjectHeader.lpObjList);

	/* オブジェクトのタイプを変換 */
	ChangeObjectType(lpSceneData->lpObject);

	return 0;
}

/* LEFファイルを読み込み、中間データを作成する */
int Lef2SceneData(
	ConvertInfo *lpInfo)
{
	FILE *fp;

	/* 引数存在チェック */
	if(lpInfo->szInputPath[0] == '\0') return 1;

	/* ファイルオープン */
	if((fp = fopen(lpInfo->szInputPath, "rb")) == NULL){
		printf("Cannot open file!(%s)\n", lpInfo->szInputPath);
		return 1;
	}

	/* ファイルチェック */
	if(CheckLef(fp) != 0){
		/* LEFファイルじゃないか、バージョンが異なっている */
		fclose(fp);
		return 1;
	}

	/* LEFファイル読み込み、および中間データ作成 */
	if(ReadLefFile(fp, &(lpInfo->scnData)) != 0){
		/* 失敗 */
		/* 中間データの開放はmain側で行う */
		fclose(fp);
		return 1;
	}

	/* ファイルクローズ */
	fclose(fp);

	return 0;
}
