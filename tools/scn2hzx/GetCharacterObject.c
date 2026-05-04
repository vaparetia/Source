/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Character Object
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <ctype.h>

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

static void GetCharecterNumberRange(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, int* lpnMinNumber, int* lpnMaxNumber, int nFlag, int nVRIndexNumber);
static int SearchAndGetDummyCharacter(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpHeader, int nCharacterRouteNumber);
static int SearchAndGetCharacter(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader, int nGroupNumber);
static HP3DOBJECT SearchCharacterObject(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPCSTR lpcszName, int nFlag, int nVRIndexNumber);
static void GetVRCharacter(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, int* lpnMin, int* lpnMax, int* lpnCount);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * キャラクタオブジェクトの取得・生成
 */
int GetCharacterObject(		/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	int        i, j;
	int        nMinNumber;
	int        nMaxNumber;
	int        nVRMinNumber;
	int        nVRMaxNumber;
	int        nOffset;
	int        nVRCount;
	char       szName[_MAX_TEXT];
	HP3DOBJECT hP3DObject;
	HP3DOBJECT hP3DObjectL;

	/* 進行状況出力 */
	printf("Create Character Object.");

	/* キャラクタはモデル直下にあるので いきなり探す */
	if((hP3DObject = P3DGetFirstObject(hP3DModel, NULL)) != NULL)
	{
		do
		{
			/* Character を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Character") != 0))
				continue;

			// キャラクタの最小番号と最大番号を得る
			// 最小値は０か１でなければならない。
			// ２のままのときはエラーか、オブジェクトが存在しないかのどちらか
// 02/04/10追加：最初はＶＲのフラグが無いものを取得する
			nMinNumber = 2;
			nMaxNumber = 0;
			GetCharecterNumberRange(hP3DModel, hP3DObject,
					&nMinNumber, &nMaxNumber, 0, 0);

			// 2000/02/20 より必ず０番から出力するようにしました。
			nMinNumber = 0;
			if(nMinNumber >= 2)
				continue;

printf("\nObject Route Number : %d - %d\n", nMinNumber, nMaxNumber);

			/* 階層化、0番から、に対応している */
// 02/04/10追加：最初はＶＲのフラグが無いものを取得する
			/* Character があった */
			for(i = nMinNumber; i <= nMaxNumber; i++)
			{
				sprintf(szName, "%d", i);
				if((hP3DObjectL = SearchCharacterObject(
					hP3DModel, hP3DObject, szName, 0, 0)) == NULL)
				{
printf("Not Found \"%s\" Object. Craete Dummy.\n", szName);
					if(SearchAndGetDummyCharacter(hP3DModel, hP3DObjectL,
						lpScene2HZXHeader, i) == 0)
						return 0;
				}
				else
				{
					if(SearchAndGetCharacter(hP3DModel, hP3DObjectL,
						lpScene2HZXHeader, i) == 0)
						return 0;
				}
			}
// VRオブジェクトの数を数える
			nVRCount     = 0;
			nVRMinNumber = 0;
			nVRMaxNumber = -1;
			GetVRCharacter(hP3DModel, hP3DObject,
				&nVRMinNumber, &nVRMaxNumber, &nVRCount);

			if(nVRCount > 0)
			{
				for(i = 0; i < HZX_VR_CHARACTER_GROUP_MAX; i++)
				{
// 02/04/10追加：オフセット値を追加する
					nOffset = 0;

				// キャラクタの最小番号と最大番号を得る
				// 最小値は０か１でなければならない。
				// ２のままのときはエラーか、オブジェクトが存在しないかのどちらか
// 02/04/10追加：次はＶＲのフラグが無いものを取得する
					nMinNumber = 2;
					nMaxNumber = -1;
					GetCharecterNumberRange(hP3DModel, hP3DObject,
							&nMinNumber, &nMaxNumber, 1, i);

					// 2000/02/20 より必ず０番から出力するようにしました。
					nMinNumber = 0;
					if(nMinNumber >= 2)
						continue;

					if(nMaxNumber >= 0)
					{
						// オブジェクトがあったので
						nOffset = lpScene2HZXHeader->nCharacterRouteCount;

printf("\nObject VR[%d] Route Number : %d - %d\n", i, nMinNumber + nOffset, nMaxNumber  + nOffset);

					/* 階層化、0番から、に対応している */
// 02/04/10追加：最初はＶＲのフラグが無いものを取得する
						/* Character があった */
						for(j = nMinNumber; j <= nMaxNumber; j++)
						{
							sprintf(szName, "%d", j);
							if((hP3DObjectL = SearchCharacterObject(
								hP3DModel, hP3DObject, szName, 1, i)) == NULL)
							{
printf("Not Found \"%s\" Object. Craete Dummy.\n", szName);
								if(SearchAndGetDummyCharacter(hP3DModel, hP3DObjectL,
									lpScene2HZXHeader, j + nOffset) == 0)
								return 0;
							}
							else
							{
								if(SearchAndGetCharacter(hP3DModel, hP3DObjectL,
									lpScene2HZXHeader, j + nOffset) == 0)
									return 0;
							}
						}
					}
					lpScene2HZXHeader->arynVRCharacterOffset[i] = nOffset;
				}
			}
		}
		while((hP3DObject = P3DGetNextObject(hP3DObject)) != NULL);
	}

printf("\n");
	for(i = 0; i < HZX_VR_CHARACTER_GROUP_MAX; i++)
	{
printf("VR Character Offset[%d] = %d\n", i,
			lpScene2HZXHeader->arynVRCharacterOffset[i]);
	}


	/* 進行状況出力 */
	printf("......Done.\n");

	return 1;
}

/******************************************************************************
 * statics
 */
/******************************************************************************
 * キャラクタ番号の最小値と最大値を求める
 */
static void GetCharecterNumberRange(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL  hP3DModel,				/* モデルハンドル */
	HP3DOBJECT hP3DObject,				/* オブジェクトハンドル */
	int*       lpnMinNumber,
	int*       lpnMaxNumber,
	int        nFlag,			/* 0 : 通常のオブジェクト  1 : VRのオブジェクト */
	int        nVRIndexNumber)	/* 取得すべきVRIndexNumber */
{
	int                  i, j;
	int                  nLength;
	int                  nNumber;
	char                 szName[_MAX_TEXT];
	BOOL                 bCheck;
	HP3DOBJECT           hP3DObjectL;
	LPHZXCHARACTERVRINFO lpHZXCharacterVRInfo;

	/* グループがあったのでグループのすぐしたの子らから Hazard を探す */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
		return;

	do
	{
		bCheck = FALSE;
		if(nFlag == 0)
		{
			if(P3DUserDataObject(hP3DObjectL, HZX_CHARACTER_VR_INFO, NULL) == NULL)
				bCheck = TRUE;
		}
		else
		{
			if( (lpHZXCharacterVRInfo = (LPHZXCHARACTERVRINFO)P3DUserDataObject(
				hP3DObjectL, HZX_CHARACTER_VR_INFO, NULL)) != NULL)
			{
				if(lpHZXCharacterVRInfo->nVRIndexNumber == nVRIndexNumber)
					bCheck = TRUE;
			}
		}

		if(bCheck == TRUE)
		{
			sprintf(szName, "%s\0", P3DObject(hP3DObjectL)->szName);
			nLength = strlen(szName);

			for(i = 0; i < nLength; i++)
			{
				if(isdigit(szName[i]) == 0)
					break;
			}

			// 文字列を数字に直して比較する
			if(i == nLength)
			{
				nNumber = 0;
				for(i = nLength - 1, j = 0; i > 0; i--, j++)
				{
					nNumber *= 10;
					nNumber += szName[j] - '0';
				}
				nNumber *= 10;
				nNumber += szName[nLength - 1] - '0';

				if(*lpnMinNumber > nNumber)
					*lpnMinNumber = nNumber;

				if(*lpnMaxNumber < nNumber)
					*lpnMaxNumber = nNumber;
			}
		}

		GetCharecterNumberRange(hP3DModel, hP3DObjectL,
			lpnMinNumber, lpnMaxNumber, nFlag, nVRIndexNumber);
	}
	while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);

}

/******************************************************************************
 * ダミーのキャラクタオブジェクトを作成する
 */
static int SearchAndGetDummyCharacter(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL         hP3DModel,				/* モデルハンドル */
	HP3DOBJECT        hP3DObject,				/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpHeader,		/* HZX 用オブジェクトヘッダー */
	int               nCharacterRouteNumber)	/* キャラクタルート番号 */
{
	LPCHARACTERROUTEOBJECT lpCharacterRoute;

	/* 領域確保 */
	if((lpCharacterRoute = (LPCHARACTERROUTEOBJECT)reallocEx(
		lpHeader->lpCharacterRoute, sizeof(CHARACTERROUTEOBJECT) *
		(lpHeader->nCharacterRouteCount + 1))) == NULL)
		return 0;

	lpHeader->lpCharacterRoute = lpCharacterRoute;
	lpCharacterRoute += lpHeader->nCharacterRouteCount;
	lpHeader->nCharacterRouteCount++;
	memset(lpCharacterRoute, 0x00, sizeof(CHARACTERROUTEOBJECT));
	return 1;
}

/******************************************************************************
 * キャラクタオブジェクトを作成する
 */
static int SearchAndGetCharacter(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL         hP3DModel,				/* モデルハンドル */
	HP3DOBJECT        hP3DObject,				/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpHeader,		/* HZX 用オブジェクトヘッダー */
	int               nCharacterRouteNumber)	/* キャラクタルート番号 */
{
	
	char                   szText[_MAX_TEXT];
	char                   szInfoKeyText[_MAX_TEXT];
	char                   szCoordKeyText[_MAX_TEXT];
	int                    i;
	int*                   lpnIndex;
	int                    nIndexCount;
	int*                   lpnIndexNext;
	int                    nSize;
	LPVOID                 lpData;
	LPP3DVERTEX            lpP3DVertex;
	HZXCHARACTERINFO       hzxCharacterInfo;
	HZXCHARACTERCOORDINFO  hzxCharacterCoordInfo;
	LPCHARACTERCOORDOBJECT lpCharacterCoord;
	LPCHARACTERROUTEOBJECT lpCharacterRoute;

	/* 頂点数を演算 */
	if(OESP3DPolygonLineVertex(&lpnIndex, &nIndexCount, hP3DObject) == 0)
	{
		printf("Error! Found no connect Character(%d) Vertex.", nCharacterRouteNumber);
		return 0;
	}

	/* 領域確保 */
	if((lpCharacterRoute = (LPCHARACTERROUTEOBJECT)reallocEx(
		lpHeader->lpCharacterRoute, sizeof(CHARACTERROUTEOBJECT) *
		(lpHeader->nCharacterRouteCount + 1))) == NULL)
		return 0;

	lpHeader->lpCharacterRoute = lpCharacterRoute;
	lpCharacterRoute += lpHeader->nCharacterRouteCount;
	lpHeader->nCharacterRouteCount++;
	memset(lpCharacterRoute, 0x00, sizeof(CHARACTERROUTEOBJECT));

	/* 2:ループするかもしれないので予め倍の領域を確保しておく */
	if((lpCharacterRoute->lpCharacterCoord = (LPCHARACTERCOORDOBJECT)
		callocEx(nIndexCount * 2, sizeof(CHARACTERCOORDOBJECT))) == NULL)
		return 0;

	if(P3DUserDataObject(
		hP3DObject, HZX_CHARACTERKEYNAME, NULL) != NULL)
	{
		sprintf(szInfoKeyText, "%s", HZX_CHARACTERKEYNAME);
		sprintf(szCoordKeyText, "%s", HZX_CHARACTERCOORDKEYNAME);
	}
	else
	{
		sprintf(szInfoKeyText, "%s", HZX_CHARACTER2KEYNAME);
		sprintf(szCoordKeyText, "%s", HZX_CHARACTERCOORD2KEYNAME);
	}

	/* 補助情報 */
	memset(&hzxCharacterInfo, 0x00, sizeof(HZXCHARACTERINFO));
	if((lpData = P3DUserDataObject(
		hP3DObject, szInfoKeyText, &nSize)) != NULL)
		memcpy(&hzxCharacterInfo,
			lpData, __min(sizeof(HZXCHARACTERINFO), nSize));

	lpCharacterRoute->nCoordCount = nIndexCount;
	lpCharacterRoute->nFlag       = hzxCharacterInfo.nFlag;

	/* 頂点補助情報 */
	for(i = 0, lpnIndexNext = lpnIndex,
		lpCharacterCoord = lpCharacterRoute->lpCharacterCoord;
		i < nIndexCount; i++, lpnIndexNext++, lpCharacterCoord++)
	{
		lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex + (*lpnIndexNext);
		lpCharacterCoord->dxyzVertex.x = lpP3DVertex->p3dXYZCalc.x;
		lpCharacterCoord->dxyzVertex.y = lpP3DVertex->p3dXYZCalc.y;
		lpCharacterCoord->dxyzVertex.z = lpP3DVertex->p3dXYZCalc.z;

		memset(&hzxCharacterCoordInfo, 0x00, sizeof(HZXCHARACTERCOORDINFO));
		sprintf(szText, "%s%d", szCoordKeyText, i + 1);
		if((lpData = P3DUserDataObject(hP3DObject, szText, &nSize)) != NULL)
			memcpy(&hzxCharacterCoordInfo,
				lpData, __min(sizeof(HZXCHARACTERCOORDINFO), nSize));

		lpCharacterCoord->dxyzTarget.x = hzxCharacterCoordInfo.xyzTarget.x;
		lpCharacterCoord->dxyzTarget.y = hzxCharacterCoordInfo.xyzTarget.y;
		lpCharacterCoord->dxyzTarget.z = hzxCharacterCoordInfo.xyzTarget.z;
		lpCharacterCoord->nAction     = hzxCharacterCoordInfo.nAction;
		lpCharacterCoord->nTime       = hzxCharacterCoordInfo.nTime;
		lpCharacterCoord->nDirection  = hzxCharacterCoordInfo.nDirection;
		lpCharacterCoord->nFlag       = hzxCharacterCoordInfo.nFlag;
	}

	printf("Found \"%s\" Object. Vetex Count = %d, Loop = %d\n",
		P3DObject(hP3DObject)->szName, lpCharacterRoute->nCoordCount, hzxCharacterInfo.bLoop);

	if(hzxCharacterInfo.bLoop == FALSE)
			return 1;

	/* ループする */
	for(i -= 2, lpnIndexNext -= 2; i > 0; i--, lpCharacterCoord++,
		lpnIndexNext--, lpCharacterRoute->nCoordCount++)
	{
		lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex + (*lpnIndexNext);
		lpCharacterCoord->dxyzVertex.x = lpP3DVertex->p3dXYZCalc.x;
		lpCharacterCoord->dxyzVertex.y = lpP3DVertex->p3dXYZCalc.y;
		lpCharacterCoord->dxyzVertex.z = lpP3DVertex->p3dXYZCalc.z;

		memset(&hzxCharacterCoordInfo, 0x00, sizeof(HZXCHARACTERCOORDINFO));
		sprintf(szText, "%s%d", szCoordKeyText,
						lpCharacterRoute->nCoordCount + 1);
		if((lpData = P3DUserDataObject(hP3DObject, szText, &nSize)) != NULL)
			memcpy(&hzxCharacterCoordInfo,
				lpData, __min(sizeof(HZXCHARACTERCOORDINFO), nSize));

		lpCharacterCoord->dxyzTarget.x = hzxCharacterCoordInfo.xyzTarget.x;
		lpCharacterCoord->dxyzTarget.y = hzxCharacterCoordInfo.xyzTarget.y;
		lpCharacterCoord->dxyzTarget.z = hzxCharacterCoordInfo.xyzTarget.z;
		lpCharacterCoord->nAction     = hzxCharacterCoordInfo.nAction;
		lpCharacterCoord->nTime       = hzxCharacterCoordInfo.nTime;
		lpCharacterCoord->nDirection  = hzxCharacterCoordInfo.nDirection;
		lpCharacterCoord->nFlag       = hzxCharacterCoordInfo.nFlag;
	}

	return 1;
}

/******************************************************************************
 * フラグに応じた、指定した名称をもつキャラクタオブジェクトを検索する
 */
static HP3DOBJECT SearchCharacterObject(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL  hP3DModel,				/* モデルハンドル */
	HP3DOBJECT hP3DObject,				/* オブジェクトハンドル */
	LPCSTR     lpcszName,
	int        nFlag,			/* 0 : 通常のオブジェクト  1 : VRオブジェクト */
	int        nVRIndexNumber)	/* VRオブジェクトのインデックス */
{
	BOOL                 bCheck;
	HP3DOBJECT           hP3DObjectL;
	HP3DOBJECT           hP3DObjectHit;
	LPHZXCHARACTERVRINFO lpHZXCharacterVRInfo;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			bCheck = FALSE;
			if(nFlag == 0)
			{
				if(P3DUserDataObject(hP3DObjectL, HZX_CHARACTER_VR_INFO, NULL) == NULL)
					bCheck = TRUE;
			}
			else
			{
				if( (lpHZXCharacterVRInfo = (LPHZXCHARACTERVRINFO)P3DUserDataObject(
					hP3DObjectL, HZX_CHARACTER_VR_INFO, NULL)) != NULL)
				{
					if(lpHZXCharacterVRInfo->nVRIndexNumber == nVRIndexNumber)
						bCheck = TRUE;
				}
			}

			if(bCheck == TRUE)
			{
				if(strcmp(P3DObject(hP3DObjectL)->szName, lpcszName) == 0)
					return hP3DObjectL;
			}

			if((hP3DObjectHit = SearchCharacterObject(
				hP3DModel, hP3DObjectL, lpcszName, nFlag, nVRIndexNumber)) != NULL)
				return hP3DObjectHit;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}

	return NULL;
}

/******************************************************************************
 * フラグに応じた、指定した名称をもつキャラクタオブジェクトを検索する
 * VRオブジェクト数と、VRIndexの最小値と最大値を取得する
 */
static void GetVRCharacter(
	HP3DMODEL  hP3DModel,
	HP3DOBJECT hP3DObject,
	int*       lpnMin,
	int*       lpnMax,
	int*       lpnCount)
{
	HP3DOBJECT           hP3DObjectL;
	LPHZXCHARACTERVRINFO lpHZXCharacterVRInfo;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if( (lpHZXCharacterVRInfo = (LPHZXCHARACTERVRINFO)P3DUserDataObject(
				hP3DObjectL, HZX_CHARACTER_VR_INFO, NULL)) != NULL)
			{
				(*lpnCount)++;

				if(*lpnMin > lpHZXCharacterVRInfo->nVRIndexNumber)
					*lpnMin = lpHZXCharacterVRInfo->nVRIndexNumber;

				if(*lpnMax < lpHZXCharacterVRInfo->nVRIndexNumber)
					*lpnMax = lpHZXCharacterVRInfo->nVRIndexNumber;
			}

			GetVRCharacter(hP3DModel, hP3DObjectL, lpnMin, lpnMax, lpnCount);
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}
}
