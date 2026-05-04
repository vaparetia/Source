/*
	ReadLefAnime.c

   2002/12/11 Nobumitsu Tanaka
   $Id: ReadLefAnime.c,v 1.2 2002/12/11 06:28:36 usr03635 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "func.h"
#include "LESceneStruct20.h"

/* 共通データを設定 */
void SetKeyCommon(
	Data              *lpData,
	LEFANIMEKEYCOMMON *lpLEFKeyCommon,
	char              *lpszName)
{
	strcpy(lpData->szName, lpszName);

	lpData->nView = (int)lpLEFKeyCommon->sVisible;
	lpData->nViewFlag = (int)lpLEFKeyCommon->cVisibleOutFlag;

	lpData->fMagni = lpLEFKeyCommon->fMagni;
	lpData->nMagniFlag = (int)lpLEFKeyCommon->cMagniOutFlag;

	if((int)lpLEFKeyCommon->cAnimeCommonOutFlag == 0){
		/* 表示・拡縮・頂点・サイズ・色のフラグを寝かす */
		lpData->nViewFlag = 0;
		lpData->nMagniFlag = 0;
		lpData->nVertexFlag = 0;
		lpData->nColorFlag = 0;
	}

#ifdef DEBUG_DUMP
	printf("\t<%s>\n", lpData->szName);
	printf("\t\tView = %d\n", lpData->nView);
	printf("\t\tViewFlag = %d\n", lpData->nViewFlag);
	printf("\t\tMagni = %f\n", lpData->fMagni);
	printf("\t\tMagniFlag = %d\n", lpData->nMagniFlag);
#endif
}

/* 頂点データを設定 */
int SetKeyVertex(
	Object            *lpObject,
	Data              *lpData,
	LEFANIMEKEYCOMMON *lpLEFKeyCommon,
	LEFANIMEKEYVERTEX *lpLEFKeyVertex,
	int               *lpnVertex)
{
	int          i;
	unsigned int unVertexCount;
	Vertex *lpVertex;
	Color  *lpColor;

	/* 頂点数決定 */
	switch(lpObject->unType){
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
		/* ストリップ系は頂点に関するキー情報は無し */
		unVertexCount = 0;
		break;
	}

	/* 頂点メモリ確保 */
	if((lpVertex = (Vertex*)Malloc(sizeof(Vertex) * unVertexCount)) == NULL){
		printf("SetKeyVertex malloc error(0)!\n");
		return 1;
	}
	lpData->unVertexCount = unVertexCount;

	/* カラーメモリ確保 */
	if((lpColor = (Color*)Malloc(sizeof(Color) * unVertexCount)) == NULL){
		printf("SetKeyVertex malloc error(1)!\n");
		return 1;
	}

	/* 頂点設定 */
	switch(lpObject->unType){
	case LEPRIM_NULL:
		/* NULLの頂点はMove値 */
		lpVertex->x = *(lpnVertex + 0);
		lpVertex->y = *(lpnVertex + 1);

		lpColor->r = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 24);
		lpColor->g = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 16);
		lpColor->b = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 8);
		lpColor->a = 0x000000ff & lpLEFKeyCommon->uiColorRGBA[0];
		break;

	case LEPRIM_LINE:
		(lpVertex + 0)->x = *(lpnVertex + 0);
		(lpVertex + 0)->y = *(lpnVertex + 1);
		(lpVertex + 1)->x = *(lpnVertex + 4);
		(lpVertex + 1)->y = *(lpnVertex + 5);

		(lpColor + 0)->r = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 24);
		(lpColor + 0)->g = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 16);
		(lpColor + 0)->b = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 8);
		(lpColor + 0)->a = 0x000000ff & lpLEFKeyCommon->uiColorRGBA[0];

		(lpColor + 1)->r = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[1] >> 24);
		(lpColor + 1)->g = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[1] >> 16);
		(lpColor + 1)->b = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[1] >> 8);
		(lpColor + 1)->a = 0x000000ff & lpLEFKeyCommon->uiColorRGBA[1];
		break;

	case LEPRIM_RECT:
	case LEPRIM_FILLRECT:
	case LEPRIM_TEXRECT:
	case LEPRIM_SPINRECT:
		for(i = 0; i < unVertexCount; i++){
			(lpVertex + i)->x = *(lpnVertex + (i * 2));
			(lpVertex + i)->y = *(lpnVertex + (i * 2) + 1);

			(lpColor + i)->r = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 24);
			(lpColor + i)->g = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 16);
			(lpColor + i)->b = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[0] >> 8);
			(lpColor + i)->a = 0x000000ff & lpLEFKeyCommon->uiColorRGBA[0];
		}
		break;

	case LEPRIM_FILLPOLYGON:
		for(i = 0; i < unVertexCount; i++){
			(lpVertex + i)->x = *(lpnVertex + (i * 2));
			(lpVertex + i)->y = *(lpnVertex + (i * 2) + 1);

			(lpColor + i)->r = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[i] >> 24);
			(lpColor + i)->g = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[i] >> 16);
			(lpColor + i)->b = 0x000000ff & (lpLEFKeyCommon->uiColorRGBA[i] >> 8);
			(lpColor + i)->a = 0x000000ff & lpLEFKeyCommon->uiColorRGBA[i];
		}
		break;


	case LEPRIM_STRIP:
	case LEPRIM_FILLSTRIP:
		break;
	}
	lpData->lpvtVertex = lpVertex;
	lpData->lpclrColor = lpColor;

	if( (lpObject->unType == LEPRIM_STRIP) ||
		(lpObject->unType == LEPRIM_FILLSTRIP) ){
		/* アニメのストリップキーは出力不可 */
		lpData->nVertexFlag = 0;
	}else{
		lpData->nVertexFlag = (int)lpLEFKeyCommon->cVertexOutFlag;
	}
	lpData->nColorFlag = (int)lpLEFKeyCommon->cRGBAOutFlag;

	if((int)lpLEFKeyCommon->cAnimeCommonOutFlag == 0){
		lpData->nVertexFlag = 0;
		lpData->nColorFlag  = 0;
	}

#ifdef DEBUG_DUMP
	printf("\t\tVertexCount = %d\n", lpData->unVertexCount);

	if( (lpObject->unType != LEPRIM_STRIP) &&
		(lpObject->unType != LEPRIM_FILLSTRIP) ){
		printf("\t\tVertex\n");
		for(i = 0; i < lpData->unVertexCount; i++){
			printf("\t\t\t(%d, %d)\n", (lpData->lpvtVertex + i)->x, (lpData->lpvtVertex + i)->y);
		}
		printf("\t\tColor\n");
		for(i = 0; i < lpData->unVertexCount; i++){
			printf("\t\t\t(%d, %d, %d, %d)\n", (lpData->lpclrColor + i)->r, 
				(lpData->lpclrColor + i)->g, (lpData->lpclrColor + i)->b, (lpData->lpclrColor + i)->a);
		}
	}
#endif

	return 0;
}

/* キーテクスチャデータを設定 */
int SetKeyTex(
	SceneData      *lpSceneData,
	Data           *lpData,
	LEFANIMEKEYTEX *lpLEFKeyTex,
	char           *lpszTexName)
{
	Picture *lpPicture;

	/* キーからはブレンドは編集させない */
	lpData->nBlendFlag = 0;

	/* ピクチャー検索 */
	if((lpPicture = SearchPicture(lpSceneData, lpszTexName)) == NULL){
		printf("\t\tNot found picture!(%s)\n", lpszTexName);
		lpData->nUVFlag = 0;
		lpData->nPictureFlag = 0;
		return 0;
	}

	/* テクスチャ名 */
	strcpy(lpData->szPictureName, lpszTexName);
	lpData->nPictureFlag = (int)lpLEFKeyTex->cTextureNameOutFlag;

	strcpy(lpData->szTRIName, lpPicture->szTRIName);

	/* UV値 */
	/* UV値メモリ確保 */
	if((lpData->lpUV = (UV*)Malloc(sizeof(UV) * lpData->unVertexCount)) == NULL){
		printf("SetKeyTex malloc error(0)!\n");
		return 1;
	}
	(lpData->lpUV + 0)->u = (float)lpLEFKeyTex->arynUV[0] / (float)lpPicture->nWidth;
	(lpData->lpUV + 0)->v = (float)lpLEFKeyTex->arynUV[1] / (float)lpPicture->nHeight;

	(lpData->lpUV + 2)->u = (float)(lpLEFKeyTex->arynUV[0] + lpLEFKeyTex->arynUVSize[0]) / (float)lpPicture->nWidth;
	(lpData->lpUV + 2)->v = (float)(lpLEFKeyTex->arynUV[1] + lpLEFKeyTex->arynUVSize[1]) / (float)lpPicture->nHeight;

	(lpData->lpUV + 1)->u = (lpData->lpUV + 2)->u;
	(lpData->lpUV + 1)->v = (lpData->lpUV + 0)->v;

	(lpData->lpUV + 3)->u = (lpData->lpUV + 0)->u;
	(lpData->lpUV + 3)->v = (lpData->lpUV + 2)->v;

	lpData->nUVFlag = (int)lpLEFKeyTex->cUVOutFlag;

	if((int)lpLEFKeyTex->cAnimeTextureOutFlag == 0){
		lpData->nPictureFlag = 0;
		lpData->nUVFlag      = 0;
	}

#ifdef DEBUG_DUMP
	printf("\t\tPicture Name = %s\n", lpData->szPictureName);
	printf("\t\tPicture Flag = %d\n", lpData->nPictureFlag);
	printf("\t\tTRI Name = %s\n", lpData->szTRIName);
	{
		int i;
		printf("\t\tUV\n");
		for(i = 0; i < lpData->unVertexCount; i++){
			printf("\t\t\t(%f, %f)\n", (lpData->lpUV + i)->u, (lpData->lpUV + i)->v);
		}
		printf("\t\tUV Flag = %d\n", lpData->nUVFlag);
	}
#endif

	return 0;
}

/* キースピンデータを設定 */
void SetKeySpin(
	Data           *lpData,
	LEFANIMEKEYSPIN *lpLEFKeySpin)
{
	lpData->nRotate = lpLEFKeySpin->nRotate;
	lpData->nRotateFlag = (int)lpLEFKeySpin->cRotateOutFlag;

	lpData->nRotateMode = lpLEFKeySpin->nRotateMode;

	lpData->vtSpinCenter.x = lpLEFKeySpin->nSpinCenterX;
	lpData->vtSpinCenter.y = lpLEFKeySpin->nSpinCenterY;
	lpData->nSpinCenterFlag = (int)lpLEFKeySpin->cSpinCenterOutFlag;

	if((int)lpLEFKeySpin->cAnimeSpinOutFlag == 0){
		lpData->nRotateFlag = 0;
		lpData->nSpinCenterFlag = 0;
	}

#ifdef DEBUG_DUMP
	printf("\t\tRotate = %d\n", lpData->nRotate);
	printf("\t\tRotateFlag = %d\n", lpData->nRotateFlag);
	printf("\t\tRotateMode = %d\n", lpData->nRotateMode);
	printf("\t\tCenter = (%d, %d)\n", lpData->vtSpinCenter.x, lpData->vtSpinCenter.y);
	printf("\t\tCenterFlag = %d\n", lpData->nSpinCenterFlag);
#endif

}

/* LEFアニメキー読み込み */
int ReadLefKey(
	FILE      *fp,
	SceneData *lpSceneData,
	Object    *lpObject,
	int        nKeyCount,
	AnimeObjectDummy *lpAnimeDummy)
{
	int   i;
	int  *lpnVertex;
	char  szName[MAX_TEXT];
	char  szTexName[MAX_TEXT];
	Data *lpData;
	Data *lpDataPrev;
	Key  *lpKey;
	LEFANIMEKEYCOMMON lefKeyCommon;
	LEFANIMEKEYVERTEX lefKeyVertex;
	LEFANIMEKEYTEX    lefKeyTex;
	LEFANIMEKEYSPIN   lefKeySpin;

	/* Key列の確保 */
	if((lpAnimeDummy->lpKey = Malloc(sizeof(Key) * nKeyCount)) == NULL){
		printf("ReadLefKey malloc error(0)!\n");
		return 1;
	}

	for(i = 0; i < nKeyCount; i++){
		// キーの読み込み

		/* キー共通データ読み込み */
		fread(&lefKeyCommon, sizeof(LEFANIMEKEYCOMMON), 1, fp);
		/* キー名読み込み */
		fread(szName, sizeof(char), lefKeyCommon.nKeyNameLength, fp);
		szName[lefKeyCommon.nKeyNameLength] = '\0';

		// 頂点の読み込み
		fread(&lefKeyVertex, sizeof(LEFANIMEKEYVERTEX), 1, fp);
		// 頂点列の領域確保
		if((lpnVertex = (int*)Malloc(
			sizeof(int) * lefKeyVertex.nVertexCount * 2)) == NULL){
			printf("ReadLefKey malloc error(1)!\n");
			return 1;
		}
		fread(lpnVertex, sizeof(int), lefKeyVertex.nVertexCount * 2, fp);

		/* テクスチャデータの読み込み */
		if( (lpObject->unType == LEPRIM_TEXRECT) ||
			(lpObject->unType == LEPRIM_SPINRECT) ||
			(lpObject->unType == LEPRIM_TEXPOLYGON)){
			/* キーテクスチャデータ読み込み */
			fread(&lefKeyTex, sizeof(LEFANIMEKEYTEX), 1, fp);
			/* テクスチャ名読み込み */
			fread(szTexName, sizeof(char), lefKeyTex.nTexNameLength, fp);
			szTexName[lefKeyTex.nTexNameLength] = '\0';
		}

		/* スピンデータの読み込み */
		if(lpObject->unType == LEPRIM_SPINRECT){
			/* キースピンデータ読み込み */
			fread(&lefKeySpin, sizeof(LEFANIMEKEYSPIN), 1, fp);
		}

		/* Data領域の確保 */
		if((lpData = (Data*)Malloc(sizeof(Data))) == NULL){
			printf("ReadLefKey malloc error(2)!\n");
			return 1;
		}
		memset(lpData, 0x00, sizeof(Data));
		lpData->lpNext = NULL;
		lpData->unID   = lpObject->unDataCount;

		/* Keyデータ設定 */
		lpKey = (Key*)lpAnimeDummy->lpKey + lpAnimeDummy->nCount;
		lpKey->nFrame = lefKeyCommon.nFrame;
		lpKey->unType = lefKeyCommon.sType;
		lpKey->lpData = lpData;
		lpAnimeDummy->nCount++;

#ifdef DEBUG_DUMP
	printf("\n");
	printf("\t[KEY] Frame = %d\n", lpKey->nFrame);
#endif

		/* データ登録 */
		SetKeyCommon(lpData, &lefKeyCommon, szName);
		if(SetKeyVertex(lpObject, lpData,
			&lefKeyCommon, &lefKeyVertex, lpnVertex) != 0){
			/* エラー */
			return 1;
		}
		if( (lpObject->unType == LEPRIM_TEXRECT) ||
			(lpObject->unType == LEPRIM_SPINRECT) ||
			(lpObject->unType == LEPRIM_TEXPOLYGON)){
			if(SetKeyTex(lpSceneData, lpData, &lefKeyTex, szTexName) != 0){
				/* エラー */
				return 1;
			}
		}
		if(lpObject->unType == LEPRIM_SPINRECT){
			SetKeySpin(lpData, &lefKeySpin);
		}

		/* Data自体の登録 */
		lpDataPrev = lpObject->lpData;
		while(lpDataPrev->lpNext != NULL){
			lpDataPrev = lpDataPrev->lpNext;
		}

		Free(lpnVertex);
		lpDataPrev->lpNext = lpData;
		lpObject->unDataCount++;
	}

	return 0;
}

/* 宣言キーの読み込み */
/* 読み込むだけ */
int ReadLefDefineKey(
	FILE   *fp,
	Object *lpObject,
	int     nKeyCount)
{
	int   i;
	int  *lpnVertex;
	char  szName[MAX_TEXT];
	char  szTexName[MAX_TEXT];
	LEFANIMEKEYCOMMON lefKeyCommon;
	LEFANIMEKEYVERTEX lefKeyVertex;
	LEFANIMEKEYTEX    lefKeyTex;
	LEFANIMEKEYSPIN   lefKeySpin;

	for(i = 0; i < nKeyCount; i++){
		// キーの読み込み

		/* キー共通データ読み込み */
		fread(&lefKeyCommon, sizeof(LEFANIMEKEYCOMMON), 1, fp);
		/* キー名読み込み */
		fread(szName, sizeof(char), lefKeyCommon.nKeyNameLength, fp);
		szName[lefKeyCommon.nKeyNameLength] = '\0';

		// 頂点の読み込み
		fread(&lefKeyVertex, sizeof(LEFANIMEKEYVERTEX), 1, fp);
		// 頂点列の領域確保
		if((lpnVertex = (int*)Malloc(
			sizeof(int) * lefKeyVertex.nVertexCount * 2)) == NULL){
			printf("ReadLefKey malloc error(1)!\n");
			return 1;
		}
		fread(lpnVertex, sizeof(int), lefKeyVertex.nVertexCount * 2, fp);
		Free(lpnVertex);

		/* テクスチャデータの読み込み */
		if( (lpObject->unType == LEPRIM_TEXRECT) ||
			(lpObject->unType == LEPRIM_SPINRECT) ||
			(lpObject->unType == LEPRIM_TEXPOLYGON)){
			/* キーテクスチャデータ読み込み */
			fread(&lefKeyTex, sizeof(LEFANIMEKEYTEX), 1, fp);
			/* テクスチャ名読み込み */
			fread(szTexName, sizeof(char), lefKeyTex.nTexNameLength, fp);
			szTexName[lefKeyTex.nTexNameLength] = '\0';
		}

		/* スピンデータの読み込み */
		if(lpObject->unType == LEPRIM_SPINRECT){
			/* キースピンデータ読み込み */
			fread(&lefKeySpin, sizeof(LEFANIMEKEYSPIN), 1, fp);
		}
	}

	return 0;
}

/* イベントキーの読み込み */
/* 読み込むだけ */
int ReadLefEventKey(
	FILE   *fp,
	Object *lpObject,
	int     nKeyCount)
{
	int               i;
	char              szName[MAX_TEXT];
	char             *lpszText;
	LEFANIMEKEYEVENT  lefKeyEvent;

	for(i = 0; i < nKeyCount; i++){
		// キーの読み込み
		fread(&lefKeyEvent, sizeof(LEFANIMEKEYEVENT), 1, fp);

		// 名称の読み込み
		fread(szName, sizeof(char), lefKeyEvent.nKeyNameLength, fp);
		szName[lefKeyEvent.nKeyNameLength] = '\0';

		if((lpszText = (char*)Malloc(
			sizeof(char) * lefKeyEvent.nMessageLength)) == NULL){
			printf("ReadLefEventKey malloc error(0)!\n");
			return 1;
		}
		fread(lpszText, sizeof(char), lefKeyEvent.nMessageLength, fp);

		Free(lpszText);
	}

	return 0;
}

/* LEFアニメオブジェクト読み込み */
/* アニメオブジェクト自体のデータは、実機側には要らないので */
/* 参照アクターの情報以外は破棄 */
int ReadLefAnime(
	FILE        *fp,
	SceneData   *lpSceneData,
	Object      *lpObject,
	ObjectDummy *lpObjectDummy,
	int          nAnimeObjectCount)
{
	int                   i;
	int                  *lpnVertex;
	char                  szName[MAX_TEXT];
	char                  szTexName[MAX_TEXT];
	LEFANIMEOBJECTCOMMON  lefAnimeCommon;
	LEFANIMEOBJECTVERTEX  lefAnimeVertex;
	LEFANIMEOBJECTTEX     lefAnimeTex;
	LEFANIMEOBJECTSPIN    lefAnimeSpin;

	/* ダミーデータ確保 */
	if((lpObjectDummy->lpAnmObj = (AnimeObjectDummy*)Malloc(
		sizeof(AnimeObjectDummy) * nAnimeObjectCount)) == NULL){
		printf("ReadLefAnime malloc error(0)!\n");
		return 1;
	}

	for(i = 0; i < nAnimeObjectCount; i++){
		AnimeObjectDummy *lpAnimeDummy;

		/* アニメオブジェクト読み込み */

		/* 共通アニメデータ読み込み */
		fread(&lefAnimeCommon, sizeof(LEFANIMEOBJECTCOMMON), 1, fp);
		/* アニメオブジェクト名読み込み */
		fread(szName, sizeof(char), lefAnimeCommon.nNameLength, fp);
		szName[lefAnimeCommon.nNameLength] = '\0';

		/* アニメオブジェクト頂点データの読み込み */
		fread(&lefAnimeVertex, sizeof(LEFANIMEOBJECTVERTEX), 1, fp);
		/* 頂点領域確保 */
		if((lpnVertex = (int*)Malloc(
			sizeof(int) * lefAnimeVertex.nVertexCount * 2)) == NULL){
			printf("ReadLefAnime malloc error(1)!\n");
			return 1;
		}
		/* 頂点データ読み込み */
		/* Define と Moveの２回読み込む */
		fread(lpnVertex, sizeof(int), lefAnimeVertex.nVertexCount * 2, fp);
		fread(lpnVertex, sizeof(int), lefAnimeVertex.nVertexCount * 2, fp);
		/* 次の読み込みのために破棄しておく */
		Free(lpnVertex);

		if( (lpObject->unType == LEPRIM_TEXRECT) ||
			(lpObject->unType == LEPRIM_SPINRECT) ||
			(lpObject->unType == LEPRIM_TEXPOLYGON)){
			/* アニメテクスチャデータ読み込み */
			fread(&lefAnimeTex, sizeof(LEFANIMEOBJECTTEX), 1, fp);
			/* アニメオブジェクト名読み込み */
			fread(szTexName, sizeof(char), lefAnimeTex.nTexNameLength, fp);
			szTexName[lefAnimeTex.nTexNameLength] = '\0';
		}

		if(lpObject->unType == LEPRIM_SPINRECT){
			/* アニメ回転データ読み込み */
			fread(&lefAnimeSpin, sizeof(LEFANIMEOBJECTSPIN), 1, fp);
		}

		/* ダミーデータ設定 */
		lpAnimeDummy = (lpObjectDummy->lpAnmObj + lpObjectDummy->nCount);
		lpAnimeDummy->nID = lefAnimeCommon.nAnimeID;
		lpAnimeDummy->nCount = 0;
		lpAnimeDummy->lpKey  = NULL;
		lpObjectDummy->nCount++;

		if(strcmp(lpObject->szName, "ROOT") == 0){
			/* ルートの時はイベントキーを読み込むことになる */
			if(lefAnimeCommon.nDefineKeyCount != 0){
				if(ReadLefEventKey(fp, lpObject, lefAnimeCommon.nDefineKeyCount) != 0){
					/* エラー */
					return 0;
				}
			}

			/* アニメキー読み込み */
			if(lefAnimeCommon.nKeyCount != 0){
				if(ReadLefEventKey(fp, lpObject, lefAnimeCommon.nKeyCount) != 0){
					/* エラー */
					return 0;
				}
			}

		}else{
			/* 宣言アニメキーの読み込み */
			/* 使用しないので読み込むだけ */
			if(lefAnimeCommon.nDefineKeyCount != 0){
				if(ReadLefDefineKey(fp, lpObject, lefAnimeCommon.nDefineKeyCount) != 0){
					/* エラー */
					return 0;
				}
			}

			/* アニメキー読み込み */
			if(lefAnimeCommon.nKeyCount != 0){
				if(ReadLefKey(fp, lpSceneData, lpObject, lefAnimeCommon.nKeyCount, lpAnimeDummy) != 0){
					/* エラー */
					return 0;
				}
			}
		}
	}

	return 0;
}
