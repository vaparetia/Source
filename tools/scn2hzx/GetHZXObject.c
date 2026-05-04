/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get HZX Object
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

#ifdef _WINDOWS
#include "..\VirtualRealityStudio\LocalDLL\METAL2\HZXStruct2.h"
#endif
#ifdef _LINUX
#include "HZXStruct2.h"
#endif

/******************************************************************************
 * functions
 */

static LRESULT   Interrupt(WPARAM wParam, LPARAM lParam);
static HCNMSCENE ReadSceneFile(char* lpszFileName);
static int       MakeHZXObject(HP3DMODEL hP3DModel, LPSCENE2HZXHEADER lpScene2HZXHeader, int nOldHZXSceneFlag);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * シーン読み込み、及び HZX オブジェクトの取り出し
 */

int GetHZXObject(		/* 0 以外 正常 */
	LPSCENE2HZXINFO   lpscene2HZXInfo,		/* 読み込み、作成情報 */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* 読み込んだシーンから取り出したオブジェクト */
{
	HCNMSCENE      hCnmScene;
	HP3DSCENE      hP3DScene;
	HP3DSCENEACTOR hP3DSceneActor;
	HP3DACTOR      hP3DActor;
	HP3DMODEL      hP3DModel;

	/* 進行状況出力 */
	printf("reading.....");
	
	hCnmScene = NULL;

	/* CnmReadFileScene はバージョン検査で、Ｃでは通らない可能性有り */
	if((hCnmScene = ReadSceneFile(lpscene2HZXInfo->szInputFileName)) == 0)
		return 0;

	/* 読み込み終了出力 */
	printf("Done.\n");

	hP3DScene = CnmGetSceneP3DScene(hCnmScene);

	if((hP3DSceneActor = P3DGetFirstSceneActor(hP3DScene)) != NULL)
	{
		do
		{
			if((hP3DActor = P3DGetSceneActorActor(hP3DSceneActor)) == NULL)
				continue;

			if((hP3DModel = P3DGetActorModel(hP3DActor)) == NULL)
				continue;

			P3DInitMovementObject(hP3DModel, NULL, TRUE);
			P3DInitObjectMatrix(hP3DModel, NULL, TRUE);
			P3DObjectMatrix(hP3DModel, NULL, TRUE);
			P3DObjectMatrixCoord(hP3DModel, NULL, TRUE);

			/* HZX 用オブジェクトの取り出し */
			if(MakeHZXObject(hP3DModel, lpScene2HZXHeader, 
				lpscene2HZXInfo->nOldHZXSceneFlag) == 0)
				return 0;

		}
		while((hP3DSceneActor = P3DGetNextSceneActor(hP3DSceneActor)) != NULL);
	}

	/* 読み込んだシーンの削除 */
	if(hCnmScene != NULL)
		CnmDestroyScene(hCnmScene);

	return 1;
}


/******************************************************************************
 * statics
 */

/******************************************************************************
 * シーンフィルの読み込み
 */
static HCNMSCENE ReadSceneFile(	/* NULL 異常 */
	char* lpszFileName)
{
	int   i;
	int   nStringLength;	/* 文字列の長さ */
	int   nExtIndex;		/* 拡張子の始まる直前の '.' のインデックス */
	char  szExt[3] = "scn";

	nExtIndex     = 0;
	nStringLength = strlen(lpszFileName);

	/* ファイル名拡張子検査 */
	for(i = 0; i < nStringLength; i++)
	{
		if(lpszFileName[i] == '.')
			nExtIndex = i;
	}

	/* ピリオドが発見できなかったか、一番最後に発見した */
	if((nExtIndex == 0) && (nExtIndex == nStringLength))
	{
		printf("Error! This File is not VRS Scene File.");
		return 0;
	}

	/* 拡張子がシーンファイルじゃない */
	if(strncmp((lpszFileName + (nExtIndex + 1)), (const char*)szExt, 3) != 0)
	{
		printf("Error! This File is not VRS Scene File.");
		return 0;
	}

	/* 注意！ この段階では test.scntt も通ってしまうことを忘れずに */


	/* シーンファイルを読み込む */
	return CnmReadFileScene(lpszFileName, Interrupt, (LPARAM)"reading");
}

/******************************************************************************
 * HZX 用オブジェクトの作成
 */
static int MakeHZXObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader,	/* HZX 用オブジェクトヘッダー */
	int               nOldHZXFileFlag)		/* 旧 HZX 形態のシーンか */
{
	/* 進行状況出力 */
	printf("\nCreate HZX Object.............Start.\n");

	/* ヘッダーの初期化 */

	/* バージョン番号の設定 */
	lpScene2HZXHeader->nVersion = 3;

	/* カウンタの初期化 */
	lpScene2HZXHeader->nGroupCount          = 0;
	lpScene2HZXHeader->nCharacterRouteCount = 0;
	lpScene2HZXHeader->nClearingAreaCount   = 0;
//	lpScene2HZXHeader->nZoneToZoneCount     = 0;

	/* オブジェクトへのポインタの初期化 */
	lpScene2HZXHeader->lpGroup          = NULL;
	lpScene2HZXHeader->lpRoute          = NULL;
	lpScene2HZXHeader->lpCharacterRoute = NULL;
	lpScene2HZXHeader->lpClearingArea   = NULL;
//	lpScene2HZXHeader->lpZoneToZone     = NULL;

	/* グループの読み込み */
	if(GetGroupObject(hP3DModel, lpScene2HZXHeader) == 0)
		return 0;

	/* ハザードの読み込み */
	if(GetHazardObject(hP3DModel, lpScene2HZXHeader) == 0)
		return 0;

	/* フロアの読み込み */
	if(GetFloorObject(hP3DModel, lpScene2HZXHeader) == 0)
		return 0;

	/* トラップ・ラジオの読み込み */
	/* 現状ではラジオはトラップとして処理 */
	/* ラジオは別オブジェクトだがトラップの後に組み込む */
	if(GetTrapObject(hP3DModel, lpScene2HZXHeader) == 0)
		return 0;

	/* カメラの読み込み */
	if(GetCameraObject(hP3DModel, lpScene2HZXHeader) == 0)
		return 0;

	/* 旧 バージョンの HZX 形式のシーン */
//	if(nOldHZXFileFlag == 1)
	if(0)
	{
		/* ゾーンの読み込み */
		if(GetOldZoneObject(hP3DModel, lpScene2HZXHeader) == 0)
			return 0;

		/* ルートの読み込み */
		if(GetOldRouteObject(hP3DModel, lpScene2HZXHeader) == 0)
			return 0;
	}
	else
	{
		/* ゾーンの読み込み */
		if(GetZoneObject(hP3DModel, lpScene2HZXHeader) == 0)
			return 0;

		/* ルートの読み込み */
		if(GetRouteObject(hP3DModel, lpScene2HZXHeader) == 0)
			return 0;
	}

	/* キャラクタの読み込み */
	if(GetCharacterObject(hP3DModel, lpScene2HZXHeader) == 0)
		return 0;
	
	/* クリアリングの読み込み */
	if(GetClearingObject(hP3DModel, lpScene2HZXHeader) == 0)
		return 0;

	/* 進行状況出力 */
	printf("Create HZX Object.............End.\n");

	return 1;
}

/******************************************************************************
 * 
 */
static LRESULT Interrupt(
	WPARAM wParam,	
	LPARAM lParam)	
{
//	printf("%s = %d/100\n", (char*)lParam, wParam);
	printf(".");

	return 1;
}
