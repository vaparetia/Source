/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Clearing Object
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

static int SearchAndGetClearing(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * クリアリングオブジェクトの取得・生成
 */
int GetClearingObject(		/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	HP3DOBJECT hP3DObject;

	/* 進行状況出力 */
	printf("Create Clearing Object.");

	/* クリアリングはモデル直下にあるので いきなり探す */
	if((hP3DObject = P3DGetFirstObject(hP3DModel, NULL)) != NULL)
	{
		do
		{
			/* Clearing を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Clearing") != 0))
				continue;

			if(SearchAndGetClearing(hP3DModel, hP3DObject,
				lpScene2HZXHeader) == 0)
				return 0;
		}
		while((hP3DObject = P3DGetNextObject(hP3DObject)) != NULL);
	}

	/* 進行状況出力 */
	printf(".......Done.\n");

	return 1;
}

/******************************************************************************
 * statics
 */

/******************************************************************************
 * フラグに応じた、指定した名称をもつクリアリングエリアオブジェクトを検索する
 */
static HP3DOBJECT SearchClearingAreaObject(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL  hP3DModel,				/* モデルハンドル */
	HP3DOBJECT hP3DObject,				/* オブジェクトハンドル */
	LPCSTR     lpcszName,
	int        nFlag,
	int        nVRIndexNumber)
{
	BOOL                bCheck;
	HP3DOBJECT          hP3DObjectL;
	HP3DOBJECT          hP3DObjectHit;
	LPHZXCLEARINGVRINFO lpHZXClearingVRInfo;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			bCheck = FALSE;
			if(nFlag == 0)
			{
				if(P3DUserDataObject(hP3DObjectL, HZX_CLEARING_VR_INFO, NULL) == NULL)
					bCheck = TRUE;
			}
			else
			{
				if((lpHZXClearingVRInfo = (LPHZXCLEARINGVRINFO)P3DUserDataObject(
					hP3DObjectL, HZX_CLEARING_VR_INFO, NULL)) != NULL)
				{
					if(lpHZXClearingVRInfo->nVRIndexNumber == nVRIndexNumber)
						bCheck = TRUE;
				}
			}

			if(bCheck == TRUE)
			{
				if(strcmp(P3DObject(hP3DObjectL)->szName, lpcszName) == 0)
					return hP3DObjectL;
			}
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}

	return NULL;
}

/******************************************************************************
 * フラグに応じた、指定した名称をもつクリアリングキャラクタオブジェクトを検索する
 */
static void GetVRClearingAreaCount(
	HP3DMODEL  hP3DModel,
	HP3DOBJECT hP3DObject,
	int*       lpnCount)
{
	HP3DOBJECT hP3DObjectL;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(P3DUserDataObject(hP3DObjectL, HZX_CLEARING_VR_INFO, NULL) != NULL)
				(*lpnCount)++;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}
}

 /******************************************************************************
 * クリアリングポイントオブジェクトを作成
 */
static BOOL GetClearingCoord(
	HP3DOBJECT            hP3DObject,		/* クリアリングポイント */
	LPCLEARINGROUTEOBJECT lpClearingRoute,	/* クリアリングルートオブジェクト */
	int                   nClearingRouteNumber)	/* クリアリングルート番号 */
{
	char                  szText[_MAX_TEXT];
	char                  szCoordKeyText[_MAX_TEXT];
	int                   i;
	int*                  lpnIndex;
	int                   nIndexCount;
	int*                  lpnIndexNext;
	int                   nSize;
	LPVOID                lpData;
	LPP3DVERTEX           lpP3DVertex;
	HZXCLEARINGCOORDINFO  hzxClearingCoordInfo;
	LPCLEARINGCOORDOBJECT lpClearingCoord;

	/* 頂点数を演算 */
	if(OESP3DPolygonLineVertex(&lpnIndex, &nIndexCount, hP3DObject) == 0)
	{
		printf("Error! Found no connect Clearing(%d) Vertex.\n",
			nClearingRouteNumber);
		return 0;
	}

	/* クリアリングポイントデータ領域の確保 */
	if((lpClearingCoord = (LPCLEARINGCOORDOBJECT)
		callocEx(nIndexCount, sizeof(CLEARINGCOORDOBJECT))) == NULL)
		return 0;

	lpClearingRoute->lpClearingCoord = lpClearingCoord;
	lpClearingRoute->nPoints         = nIndexCount;
	sprintf(szText, "%s1", HZX_CLEARINGCOORDKEYNAME);
	if(P3DUserDataObject(hP3DObject, szText, NULL) != NULL)
		sprintf(szCoordKeyText, "%s", HZX_CLEARINGCOORDKEYNAME);
	else
		sprintf(szCoordKeyText, "%s", HZX_CLEARINGCOORD2KEYNAME);

	/* 頂点補助情報 */
	for(i = 0, lpnIndexNext = lpnIndex;
		i < nIndexCount; i++, lpnIndexNext++, lpClearingCoord++)
	{
		lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex + (*lpnIndexNext);
		lpClearingCoord->dxyzVertex.x = lpP3DVertex->p3dXYZCalc.x;
		lpClearingCoord->dxyzVertex.y = lpP3DVertex->p3dXYZCalc.y;
		lpClearingCoord->dxyzVertex.z = lpP3DVertex->p3dXYZCalc.z;

		memset(&hzxClearingCoordInfo, 0x00, sizeof(HZXCLEARINGCOORDINFO));
		sprintf(szText, "%s%d", szCoordKeyText, i + 1);
		if((lpData = P3DUserDataObject(hP3DObject, szText, &nSize)) != NULL)
			memcpy(&hzxClearingCoordInfo,
				lpData, __min(sizeof(HZXCLEARINGCOORDINFO), nSize));

		lpClearingCoord->dxyzTarget.x = hzxClearingCoordInfo.xyzTarget.x;
		lpClearingCoord->dxyzTarget.y = hzxClearingCoordInfo.xyzTarget.y;
		lpClearingCoord->dxyzTarget.z = hzxClearingCoordInfo.xyzTarget.z;
		lpClearingCoord->nAction     = hzxClearingCoordInfo.nAction;
		lpClearingCoord->nTime       = hzxClearingCoordInfo.nTime;
		lpClearingCoord->nDirection  = hzxClearingCoordInfo.nDirection;
		lpClearingCoord->nCondition  = hzxClearingCoordInfo.nCondition;
	}

	return 1;
}

/******************************************************************************
 * クリアリングルートオブジェクトを作成する
 */
static BOOL GetClearingRoot(
	HP3DOBJECT           hP3DObject,		/* クリアリングルートオブジェクト */
	LPCLEARINGAREAOBJECT lpClearingArea)	/* クリアリングエリアオブジェクト */
{
	LPCLEARINGROUTEOBJECT lpClearingRoute;

	/* クリアリングルート領域の確保 */
	if((lpClearingRoute
		= (LPCLEARINGROUTEOBJECT)reallocEx(lpClearingArea->lpClearingRoute,
		sizeof(CLEARINGROUTEOBJECT) * (lpClearingArea->nRouteCount + 1))) == NULL)
		return 0;

	lpClearingArea->lpClearingRoute = lpClearingRoute;
	lpClearingRoute        += lpClearingArea->nRouteCount;
	lpClearingArea->nRouteCount++;
	memset(lpClearingRoute, 0x00, sizeof(CLEARINGROUTEOBJECT));

	if(GetClearingCoord(hP3DObject, lpClearingRoute,
		lpClearingArea->nRouteCount - 1) == 0)
		return 0;

	return 1;
}

/******************************************************************************
 * キャラクタ番号の最小値と最大値を求める
 */
static void GetClearingNumberRange(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL  hP3DModel,				/* モデルハンドル */
	HP3DOBJECT hP3DObject,				/* オブジェクトハンドル */
	int*       lpnMinNumber,
	int*       lpnMaxNumber)
{
	int        i, j;
	int        nLength;
	int        nNumber;
	char       szName[_MAX_TEXT];
	HP3DOBJECT hP3DObjectL;

	/* グループがあったのでグループのすぐしたの子らから Hazard を探す */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
		return;

	do
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
				nNumber += 10 * i * (szName[j] - '0');
			}
			nNumber += szName[nLength - 1] - '0';

			if(*lpnMinNumber > nNumber)
				*lpnMinNumber = nNumber;

			if(*lpnMaxNumber < nNumber)	
				*lpnMaxNumber = nNumber;
		}

		GetClearingNumberRange(hP3DModel, hP3DObjectL, lpnMinNumber, lpnMaxNumber);
	}
	while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);

}

/******************************************************************************
 * ダミーのキャラクタオブジェクトを作成する
 */
static BOOL GetClearingDummyRoot(
	HP3DOBJECT           hP3DObject,		/* クリアリングルートオブジェクト */
	LPCLEARINGAREAOBJECT lpClearingArea)	/* クリアリングエリアオブジェクト */
{
	LPCLEARINGROUTEOBJECT lpClearingRoute;

	/* クリアリングルート領域の確保 */
	if((lpClearingRoute
		= (LPCLEARINGROUTEOBJECT)reallocEx(lpClearingArea->lpClearingRoute,
		sizeof(CLEARINGROUTEOBJECT) * (lpClearingArea->nRouteCount + 1))) == NULL)
		return 0;

	lpClearingArea->lpClearingRoute = lpClearingRoute;
	lpClearingRoute        += lpClearingArea->nRouteCount;
	lpClearingArea->nRouteCount++;
	memset(lpClearingRoute, 0x00, sizeof(CLEARINGROUTEOBJECT));
	return 1;
}

/******************************************************************************
 * クリアリングルートオブジェクトを作成する。
 * 階層化対応版
 */
static BOOL GetClearingRoot2(
	HP3DMODEL            hP3DModel,
	HP3DOBJECT           hP3DObject,		/* クリアリングルートオブジェクト */
	LPSCENE2HZXHEADER    lpHeader,		/* HZX 用オブジェクトヘッダー */
	LPCLEARINGAREAOBJECT lpClearingArea,	/* クリアリングエリアオブジェクト */
	int                  nMinNumber,
	int                  nMaxNumber,
	int                  nFlag,
	int                  nOffset)
{
	int        i;
	char       szName[_MAX_TEXT];
	HP3DOBJECT hP3DObjectL;

	for(i = nMinNumber; i <= nMaxNumber; i++)
	{
		sprintf(szName, "%d", i);

printf("\tClearing Root Name = %s\n", szName);

		if((hP3DObjectL = P3DFindChildObjectName(
			hP3DModel, hP3DObject, szName, -1)) == NULL)
		{
			if(GetClearingDummyRoot(hP3DObjectL, lpClearingArea) == 0)
			{
				printf("Error! Clearing Route in Clearing Area(%d)\n",
					lpHeader->nClearingAreaCount);
				return 0;
			}
		}
		else
		{
			if(GetClearingRoot(hP3DObjectL, lpClearingArea) == 0)
			{
				printf("Error! Clearing Route in Clearing Area(%d)\n",
					lpHeader->nClearingAreaCount);
				return 0;
			}
		}
	}
}

/******************************************************************************
 * クリアリングエリアオブジェクトを作成する
 */
static int GetClearingArea(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL         hP3DModel,	/* モデルハンドル */
	HP3DOBJECT        hP3DObject,	/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpHeader)		/* HZX 用オブジェクトヘッダー */
{
	int                   nMinNumber;
	int                   nMaxNumber;
	int                   nOffset;
	int                   nVRCount;
	HP3DOBJECT            hP3DObjectL;
	P3DXYZ                xyz1, xyz2;
	LPHZXCLEARINGAREAINFO lpHZXClearingAreaInfo;
	LPCLEARINGAREAOBJECT  lpClearingArea;

	/* クリアリングエリア領域の確保 */
	if((lpClearingArea
		= (LPCLEARINGAREAOBJECT)reallocEx(lpHeader->lpClearingArea,
		sizeof(CLEARINGAREAOBJECT) * (lpHeader->nClearingAreaCount + 1))) == NULL)
		return 0;

	lpHeader->lpClearingArea = lpClearingArea;
	lpClearingArea += lpHeader->nClearingAreaCount;
	lpHeader->nClearingAreaCount++;
	memset(lpClearingArea, 0x00, sizeof(CLEARINGAREAOBJECT));

	/* クリアリングエリア情報の取得 */
	/* バウンディングの取得 */
	P3DAreaObject(&xyz1, &xyz2, hP3DObject);

	/* h にはなにが入るのだろうか */
	lpClearingArea->dxyzBound[0].x = xyz1.x;
	lpClearingArea->dxyzBound[0].y = xyz1.y;
	lpClearingArea->dxyzBound[0].z = xyz1.z;
	lpClearingArea->dxyzBound[1].x = xyz2.x;
	lpClearingArea->dxyzBound[1].y = xyz2.y;
	lpClearingArea->dxyzBound[1].z = xyz2.z;

	/* サイズは今後の動向に注意 */
	strcpy(lpClearingArea->szName, P3DObject(hP3DObject)->szName);

	/* クリアリングエリア情報の取得 */
	if(P3DUserDataObject(hP3DObject, HZX_CLEARINGAREAKEYNAME, NULL) != NULL)
	{
		/* 再確保 */
		if(P3DCreateUserDataObject(hP3DObject, HZX_CLEARINGAREAKEYNAME,
			sizeof(HZXCLEARINGAREAINFO)) == 0)
			return 0;

		lpHZXClearingAreaInfo = (LPHZXCLEARINGAREAINFO)P3DUserDataObject(
			hP3DObject, HZX_CLEARINGAREAKEYNAME, NULL);

		lpClearingArea->nTime = lpHZXClearingAreaInfo->nTime;
	}

	/* VRではないデータの取得 */
	/* クリアリングルートの取得 */
	nMinNumber = 2;
	nMaxNumber = 0;
	GetClearingNumberRange(hP3DModel, hP3DObject, &nMinNumber, &nMaxNumber);

	// 2000/02/20 より必ず0番から出力するようにしました
	nMinNumber = 0;
	if(nMinNumber >= 2)
		return 1;

	if(GetClearingRoot2(hP3DModel, hP3DObject, lpHeader, lpClearingArea,
		nMinNumber, nMaxNumber, 0, 0) == 0)
	{
		printf("Error! Clearing Route in Clearing Area(%d)\n",
			lpHeader->nClearingAreaCount);
		return 0;
	}

	return 1;
}

/******************************************************************************
 * クリアリングオブジェクトを作成する
 */
static int SearchAndGetClearing(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DMODEL         hP3DModel,	/* モデルハンドル */
	HP3DOBJECT        hP3DObject,	/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpHeader)		/* HZX 用オブジェクトヘッダー */
{
	int        i, j;
	int        nOffset;
	int        nCount;
	char       szName[_MAX_TEXT];
	HP3DOBJECT hP3DObjectL;

printf("\n");

#ifdef GROUP_CLEARING_NUMBER_CONVERT
	for(i = 0; ; i++)
#else
	for(i = 1; ; i++)
#endif 
	{
		sprintf(szName, "%d", i);
		if((hP3DObjectL = SearchClearingAreaObject(
			hP3DModel, hP3DObject, szName, 0, 0)) != NULL)
		{
printf("\nFind Clearing Object = %d\n", i);
			if(GetClearingArea(hP3DModel, hP3DObjectL, lpHeader) == 0)
				return 0;
		}
		else
			break;
	}

	nOffset = -1;
	nCount  = 0;
	GetVRClearingAreaCount(hP3DModel, hP3DObject, &nCount);
	if(nCount > 0)
	{
		for(i = 0 ; i < HZX_VR_CLEARING_GROUP_MAX; i++)
		{
			nOffset = lpHeader->nClearingAreaCount;

#ifdef GROUP_CLEARING_NUMBER_CONVERT
			for(j = 0; ; j++)
#else
			for(j = 1; ; j++)
#endif 
			{
				sprintf(szName, "%d", j);
				if((hP3DObjectL = SearchClearingAreaObject(
					hP3DModel, hP3DObject, szName, 1, i)) != NULL)
				{
printf("\nFind VR Clearing Object[%d] = %d\n", i, j + nOffset);
					if(GetClearingArea(hP3DModel, hP3DObjectL, lpHeader) == 0)
						return 0;
				}
				else
					break;
			}

			if(j == 0)
				nOffset = 0;

			lpHeader->arynVRClearingOffset[i] = nOffset;
		}
	}

printf("\n");
	for(i = 0; i < HZX_VR_CLEARING_GROUP_MAX; i++)
	{
printf("VR Clearing Area Offset[%d] = %d\n", i, lpHeader->arynVRClearingOffset[i]);
	}
	return 1;
}
