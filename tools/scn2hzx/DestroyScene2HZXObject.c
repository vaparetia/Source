/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Destroy Scene2HZX Object
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

static void DestroyGroupObject(LPGROUPOBJECT lpGroupObject, int nGroupCount);
static void DestroyRouteObject(LPROUTEOBJECT lpRouteObject);
static void DestroyCharacterObject(LPCHARACTERROUTEOBJECT lpCharacterObject, int nCharacterRouteCount);
static void DestroyClearingObject(LPCLEARINGAREAOBJECT lpClearingArea, int nClearingAreaCount);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * Scene2HZX オブジェクトの削除
 */
void DestroyScene2HZXObject(	/* 0 以外 正常 */
							/* 0 異常 */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	/* 進行状況出力 */
	printf("\nDestroy Object.");

	/* グループとそれに含まれるオブジェクトの削除 */
	if(lpScene2HZXHeader->lpGroup != NULL)
	{
		
		DestroyGroupObject(lpScene2HZXHeader->lpGroup,
			lpScene2HZXHeader->nGroupCount);
		lpScene2HZXHeader->lpGroup = NULL;
	}

	/* ルートオブジェクトの削除 */
	if(lpScene2HZXHeader->lpRoute != NULL)
	{
		DestroyRouteObject(lpScene2HZXHeader->lpRoute);
		lpScene2HZXHeader->lpRoute = NULL;
	}

	/* キャラクタールートの削除 */
	if(lpScene2HZXHeader->lpCharacterRoute != NULL)
	{
		DestroyCharacterObject(lpScene2HZXHeader->lpCharacterRoute,
			lpScene2HZXHeader->nCharacterRouteCount);
		lpScene2HZXHeader->lpCharacterRoute = NULL;
	}

	/* クリアリングエリアの削除 */
	if(lpScene2HZXHeader->lpClearingArea != NULL)
	{
		DestroyClearingObject(lpScene2HZXHeader->lpClearingArea,
			lpScene2HZXHeader->nClearingAreaCount);
		lpScene2HZXHeader->lpClearingArea = NULL;
	}

	/* 進行状況出力 */
	printf("...............Done.\n");
}

/******************************************************************************
 * statics
 */

/******************************************************************************
 * ルートを削除する
 */
static void DestroyRouteObject(
	LPROUTEOBJECT lpRouteObject)
{
	if(lpRouteObject->lpRouteCoord != NULL)
	{
		freeEx(lpRouteObject->lpRouteCoord);
		lpRouteObject->lpRouteCoord = NULL;
	}

	if(lpRouteObject->lpRoutePolygon != NULL)
	{
		freeEx(lpRouteObject->lpRoutePolygon);
		lpRouteObject->lpRoutePolygon = NULL;
	}

	freeEx(lpRouteObject);
}

/******************************************************************************
 * キャラクタを削除する
 */
static void DestroyCharacterObject(
	LPCHARACTERROUTEOBJECT lpCharacterObject,
	int                    nCharacterRouteCount)
{
	int i;

	for(i = 0; i < nCharacterRouteCount; i++)
	{
		if((lpCharacterObject + i)->lpCharacterCoord != NULL)
		{
			freeEx((lpCharacterObject + i)->lpCharacterCoord);
			(lpCharacterObject + i)->lpCharacterCoord = NULL;
		}
	}

	freeEx(lpCharacterObject);
}

/******************************************************************************
 * クリアリングを削除する
 */
static void DestroyClearingObject(
	LPCLEARINGAREAOBJECT lpClearingArea,
	int                  nClearingAreaCount)
{
	int                   i;
	LPCLEARINGROUTEOBJECT lpClearingRoute;
	
	for(i = 0; i < nClearingAreaCount; i++)
	{
		lpClearingRoute = (lpClearingArea + i)->lpClearingRoute;

		if(lpClearingRoute != NULL)
		{
			if(lpClearingRoute->lpClearingCoord != NULL)
			{
				freeEx(lpClearingRoute->lpClearingCoord);
				lpClearingRoute->lpClearingCoord = NULL;
			}

			freeEx((lpClearingArea + i)->lpClearingRoute);
			(lpClearingArea + i)->lpClearingRoute = NULL;
		}
	}
	
	freeEx(lpClearingArea);
}		

/******************************************************************************
 * ブロックを削除する
 */
static void DestroyBlockObject(
	LPBLOCKOBJECT lpBlockObject,	/* グループオブジェクト */
	int           nBlockCount)		/* グループオブジェクト数 */
{
	int           i;
	LPBLOCKOBJECT lpBlock;

	for(i = 0; i < nBlockCount; i++)
	{
		lpBlock = lpBlockObject + i;

		/* ハザードの削除 */
		if(lpBlock->lpHazard != NULL)
		{
			freeEx(lpBlock->lpHazard);
			lpBlock->lpHazard = NULL;
		}

		/* フロアの削除 */
		if(lpBlock->lpFloor != NULL)
		{
			freeEx(lpBlock->lpFloor);
			lpBlock->lpFloor = NULL;
		}

		/* トラップの削除 */
		if(lpBlock->lpTrap != NULL)
		{
			freeEx(lpBlock->lpTrap);
			lpBlock->lpTrap = NULL;
		}
	}

	/* ブロックの削除 */
	freeEx(lpBlockObject);

}

/******************************************************************************
 * グループを削除する
 */
static void DestroyGroupObject(
	LPGROUPOBJECT lpGroupObject,	/* グループオブジェクト */
	int            nGroupCount)		/* グループオブジェクト数 */
{
	int           i;
	LPGROUPOBJECT lpGroup;

	for(i = 0; i < nGroupCount; i++)
	{
		lpGroup = lpGroupObject + i;

		/* カメラの削除 */
		if(lpGroup->lpCamera != NULL)
		{
			freeEx(lpGroup->lpCamera);
			lpGroup->lpCamera = NULL;
		}

		/* ゾーンの削除 */
		if(lpGroup->lpZone != NULL)
		{
			freeEx(lpGroup->lpZone);
			lpGroup->lpZone = NULL;
		}

		/* ブロックの削除 */
		if(lpGroup->lpBlock != NULL)
		{
			DestroyBlockObject(lpGroup->lpBlock, lpGroup->nBlockCount);
			lpGroup->lpBlock = NULL;
		}

		/* ここより下には基本的にはオブジェクトはぶら下がっていないはず */
		/* 壁の削除 */
		if(lpGroup->lpHazard != NULL)
		{
			freeEx(lpGroup->lpHazard);
			lpGroup->lpHazard = NULL;
		}

		/* 床の削除 */
		if(lpGroup->lpFloor != NULL)
		{
			freeEx(lpGroup->lpFloor);
			lpGroup->lpFloor = NULL;
		}

		/* トラップの削除 */
		if(lpGroup->lpTrap != NULL)
		{
			freeEx(lpGroup->lpTrap);
			lpGroup->lpTrap = NULL;
		}
	}

	freeEx(lpGroupObject);
}

	
