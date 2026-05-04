/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Route Object
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

static int SearchAndGetRoute(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * ルートオブジェクトの取得・生成：ルートは階層構造はサポートしない。兄弟も不可
 */
int GetRouteObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	HP3DOBJECT     hP3DObject;

	/* 進行状況出力 */
	printf("Create Route Object.");

	/* ルートはモデル直下にあるので いきなり探す */
	if((hP3DObject = P3DGetFirstObject(hP3DModel, NULL)) != NULL)
	{
		do
		{
			/* Route を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Route") != 0))
				continue;

			/* Route があった */
			if(SearchAndGetRoute(hP3DModel, hP3DObject,
				lpScene2HZXHeader) == 0)
				return 0;
	
		}
		while((hP3DObject = P3DGetNextObject(hP3DObject)) != NULL);
	}

	/* 進行状況出力 */
	printf("..........Done.\n");

	return 1;
}

/******************************************************************************
 * 旧 HZX シーンからルートオブジェクトの取得・生成
   ルートは階層構造はサポートしない。兄弟も不可
 */
int GetOldRouteObject(	/* 0 以外 正常 */
						/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	HP3DOBJECT hP3DObject;
	HP3DOBJECT hP3DObjectL;

	/* 進行状況出力 */
	printf("Create Route Object.");

	/* ルートはゾーンの下にあるのでまずゾーンを探す */
	if((hP3DObject = P3DGetFirstObject(hP3DModel, NULL)) != NULL)
	{
		do
		{
			/* Zone を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Zone") != 0))
				continue;

			/* Zone があった。Route を探す */
			if((hP3DObjectL = P3DFindChildObjectName(hP3DModel, hP3DObject,
				"Route", 1)) == NULL)
				return 1;

			/* Route があった */
			if(SearchAndGetRoute(hP3DModel, hP3DObjectL,
				lpScene2HZXHeader) == 0)
				return 0;
		}
		while((hP3DObject = P3DGetNextObject(hP3DObject)) != NULL);
	}

	/* 進行状況出力 */
	printf("..........Done.\n");

	return 1;
}

/******************************************************************************
 * statics
 */

/******************************************************************************
 * ルート頂点オブジェクトを作成する
 */
static int GetRouteCoordObject(	/* 0 以外 正常 */
								/* 0 異常 */
	HP3DOBJECT    hP3DObject,		/* オブジェクトハンドル */
	LPROUTEOBJECT lpRouteObject)	/* ルートオブジェクト */
{
	int                i;
	LPP3DVERTEX        lpP3DVertex;
	LPHZXZONEINFO      lpHZXZoneInfo;
	LPROUTECOORDOBJECT lpRouteCoord;

	/* ルート頂点配列を作成する */
	/* ルートオブジェクト内の頂点データを逐次チェック */
	for(i = 0, lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex;
		i < P3DObject(hP3DObject)->nVertexCount; i++, lpP3DVertex++)
	{
		/* ユーザーデータがない頂点は不正なルートデータ */
		if(P3DUserDataVertex(lpP3DVertex, HZX_ZONEKEYNAME, NULL) != NULL)
		{
			/* 再確保 */
			if(P3DCreateUserDataVertex(lpP3DVertex, HZX_ZONEKEYNAME,
				sizeof(HZXZONEINFO)) == 0)
				return 0;

			lpHZXZoneInfo = (LPHZXZONEINFO)
				P3DUserDataVertex(lpP3DVertex, HZX_ZONEKEYNAME, NULL);
		}
		else
		{
			printf("Error! Bad Route Vertex.");
			return 0;
		}

		/* ルート頂点の領域を確保 */
		if((lpRouteCoord = (LPROUTECOORDOBJECT)reallocEx(lpRouteObject->lpRouteCoord,
			sizeof(ROUTECOORDOBJECT) * (lpRouteObject->nVertexCount + 1))) == NULL)
			return 0;

		lpRouteObject->lpRouteCoord = lpRouteCoord;
		lpRouteCoord += lpRouteObject->nVertexCount;
		lpRouteObject->nVertexCount++;	
		memset(lpRouteCoord, 0x00, sizeof(ROUTECOORDOBJECT));
		
		/* 頂点座標の格納 */
		lpRouteCoord->dxyzVertex.x = lpP3DVertex->p3dXYZCalc.x;
		lpRouteCoord->dxyzVertex.y = lpP3DVertex->p3dXYZCalc.y;
		lpRouteCoord->dxyzVertex.z = lpP3DVertex->p3dXYZCalc.z;

		/* 逐次番号の格納 */
		lpRouteCoord->nIncremental = lpHZXZoneInfo->nIncremental;

		/* フラグの格納 */
		lpRouteCoord->nFlag        = lpHZXZoneInfo->nFlag;
	}

	return 1;
}

/******************************************************************************
 * ルートポリゴンオブジェクトを作成する
 */
static int GetRoutePolygonObject(	/* 0 以外 正常 */
									/* 0 異常 */
	HP3DOBJECT        hP3DObject,	/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpHeader)		/* HZX 用オブジェクトヘッダー */
{
	int                  i, j, k;
	LPP3DVERTEX          lpP3DVertex;
	LPP3DPOLYGON         lpP3DPolygon;
	LPP3DPOLYGONNODE     lpP3DPolygonNode;
	LPHZXZONEINFO        lpHZXZoneInfo;
	LPHZXZONETOZONEINFO  lpHZXZoneToZoneInfo;
	LPZONETOZONEOBJECT   lpZoneToZoneObject;
	LPROUTEOBJECT        lpRouteObject;
	LPROUTEPOLYGONOBJECT lpRoutePolygon;

	lpRouteObject = lpHeader->lpRoute;

	/* ルートポリゴン配列を作成する */
	/* ルートオブジェクト内のポリゴンデータを逐次チェック */
	for(i = 0, lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon;
		i < P3DObject(hP3DObject)->nPolygonCount; i++, lpP3DPolygon++)
	{
		/* ２頂点以下のポリゴンは不可 */
		if(lpP3DPolygon->nPolygonNodeCount < 3)
		{
			printf("Error! Bad Route Polygon.");
			return 0;
		}

		/* ルートポリゴンの領域を確保 */
		if((lpRoutePolygon = (LPROUTEPOLYGONOBJECT)reallocEx(
			lpRouteObject->lpRoutePolygon, (sizeof(ROUTEPOLYGONOBJECT) *
			(lpRouteObject->nPolygonCount + 1)))) == NULL)
			return 0;

		lpRouteObject->lpRoutePolygon = lpRoutePolygon;
		lpRoutePolygon += lpRouteObject->nPolygonCount;
		lpRouteObject->nPolygonCount++;	
		memset(lpRoutePolygon, 0x00, sizeof(ROUTEPOLYGONOBJECT));
		
		for(j = 0, lpP3DPolygonNode = lpP3DPolygon->lpP3DPolygonNode;
			j < 2; j++, lpP3DPolygonNode++)
		{
			/* 頂点データを得る */
			lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex +
				lpP3DPolygonNode->nVertex;

			/* ユーザーデータがない頂点は不正なルートデータ */
			if(P3DUserDataVertex(lpP3DVertex, HZX_ZONEKEYNAME, NULL) != NULL)
			{
				/* 再確保 */
				if(P3DCreateUserDataVertex(lpP3DVertex, HZX_ZONEKEYNAME,
					sizeof(HZXZONEINFO)) == 0)
					return 0;

				lpHZXZoneInfo = (LPHZXZONEINFO)
					P3DUserDataVertex(lpP3DVertex, HZX_ZONEKEYNAME, NULL);
			}
			else
			{
				printf("Error! Bad Route Polygon.");
				return 0;
			}
		
			/* ルート頂点インデックスを得る */
			for(k = 0; k < lpRouteObject->nVertexCount; k++)
			{
				if((lpRouteObject->lpRouteCoord + k)->nIncremental ==
					lpHZXZoneInfo->nIncremental)
					break;
			}

			if(k == lpRouteObject->nVertexCount)
			{
				printf("Error! Bad Route Polygon.");
				return 0;
			}

			/* ルート頂点へのポインタを格納 */
			lpRoutePolygon->lpRouteCoord[j] = lpRouteObject->lpRouteCoord + k;
		}

#if 0
		/* ゾーン間フラグが設定されているか */
		if(P3DUserDataPolygon(lpP3DPolygon, HZX_ZONETOZONEKEYNAME, NULL) != NULL)
		{
			/* 再確保 */
			if(P3DCreateUserDataPolygon(lpP3DPolygon, HZX_ZONETOZONEKEYNAME,
				sizeof(HZXZONETOZONEINFO)) == 0)
				return 0;

			/* ユーザーデータ取得 */
			lpHZXZoneToZoneInfo = (LPHZXZONETOZONEINFO)P3DUserDataPolygon(
				lpP3DPolygon, HZX_ZONETOZONEKEYNAME, NULL);

			/* フラグが０以外の時だけ確保する */
			if(lpHZXZoneToZoneInfo->nFlag == 0)
				continue;

			/* ゾーン間フラグオブジェクト領域を再確保 */
			if((lpZoneToZoneObject = (LPZONETOZONEOBJECT)reallocEx(
				lpHeader->lpZoneToZone, (sizeof(ZONETOZONEOBJECT) *
				(lpHeader->nZoneToZoneCount + 1)))) == NULL)
				return 0;

			lpHeader->lpZoneToZone = lpZoneToZoneObject;
			lpZoneToZoneObject += lpHeader->nZoneToZoneCount;
			lpHeader->nZoneToZoneCount++;	
			memset(lpZoneToZoneObject, 0x00, sizeof(ZONETOZONEOBJECT));

			/* フラグの内容を入力 */
			lpZoneToZoneObject->nRouteIndex = lpRouteObject->nPolygonCount - 1;
			lpZoneToZoneObject->nFlag       = lpHZXZoneToZoneInfo->nFlag;
		}
#else
		/* ゾーン間フラグが設定されているか */
		if(P3DUserDataPolygon(lpP3DPolygon, HZX_ZONETOZONEKEYNAME, NULL) != NULL)
		{
			/* 再確保 */
			if(P3DCreateUserDataPolygon(lpP3DPolygon, HZX_ZONETOZONEKEYNAME,
				sizeof(HZXZONETOZONEINFO)) == 0)
				return 0;

			/* ユーザーデータ取得 */
			lpHZXZoneToZoneInfo = (LPHZXZONETOZONEINFO)P3DUserDataPolygon(
				lpP3DPolygon, HZX_ZONETOZONEKEYNAME, NULL);

			/* 設定 */
			lpRoutePolygon->nZoneToZoneFlag = lpHZXZoneToZoneInfo->nFlag;
		}
		else
		{
			lpRoutePolygon->nZoneToZoneFlag = 0;
		}
#endif
	}

	return 1;
}

/******************************************************************************
 * ルートオブジェクトを作成する
 */
static int SearchAndGetRoute(	/* 0 以外 正常 */
								/* 0 異常 */
	HP3DMODEL         hP3DModel,	/* モデルハンドル */
	HP3DOBJECT        hP3DObject,	/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpHeader)		/* HZX 用オブジェクトヘッダー */
{
	LPROUTEOBJECT lpRoute;
	
	/* ルートオブジェクトを発見してここに入っている */

	if(hP3DObject == NULL)
		return 1;

	/* ルートの領域を確保 */
	if(lpHeader->lpRoute == NULL)
	{
		if((lpRoute = (LPROUTEOBJECT)callocEx(1,
			sizeof(ROUTEOBJECT))) == NULL)
			return 0;

		lpHeader->lpRoute       = lpRoute;

		/* メンバの初期化 */
		lpRoute->nVertexCount   = 0;
		lpRoute->nPolygonCount  = 0;
		lpRoute->lpRouteCoord   = NULL;
		lpRoute->lpRoutePolygon = NULL;
	}

	/* ルート頂点を格納する */
	if(GetRouteCoordObject(hP3DObject, lpHeader->lpRoute) == 0)
	{
		if(lpRoute->lpRouteCoord != NULL)
			freeEx(lpRoute->lpRouteCoord);

		if(lpRoute->lpRoutePolygon != NULL)
			freeEx(lpRoute->lpRoutePolygon);

		return 0;
	}

	/* ルートポリゴンを格納する*/
	if(GetRoutePolygonObject(hP3DObject, lpHeader) == 0)
	{
		if(lpRoute->lpRouteCoord != NULL)
			freeEx(lpRoute->lpRouteCoord);

		if(lpRoute->lpRoutePolygon != NULL)
			freeEx(lpRoute->lpRoutePolygon);

		return 0;
	}

	return 1;
}
