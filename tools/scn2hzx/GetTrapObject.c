/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Trap Object
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

static int SearchAndGetTrap(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader, int nGroupNumber);
static int SearchAndGetRadio(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader, int nGroupNumber);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * トラップオブジェクトの取得・生成
 */
int GetTrapObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	int            i;
	char           szGroupName[_MAX_TEXT];
	HP3DOBJECT     hP3DObject;

	/* 進行状況出力 */
	printf("Create Trap Object.");

#ifdef GROUP_CLEARING_NUMBER_CONVERT
	for(i = 0; i <= lpScene2HZXHeader->nGroupCount; i++)
#else
	for(i = 1; i <= lpScene2HZXHeader->nGroupCount; i++)
#endif 
	{
		/* グループ */
		sprintf(szGroupName, "Group-%d", i);
		if((hP3DObject = P3DFindObjectName(hP3DModel, szGroupName)) == NULL)
			break;

		/* グループがあったのでグループのすぐしたの子らから Trap を探す */
		if((hP3DObject = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
			continue;

		do
		{
			/* Trap を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Trap") == 0))
			{
#ifdef GROUP_CLEARING_NUMBER_CONVERT
				/* Trap があった */
				if(SearchAndGetTrap(hP3DModel, hP3DObject, lpScene2HZXHeader, i) == 0)
					return 0;
#else
				/* Trap があった */
				if(SearchAndGetTrap(hP3DModel, hP3DObject,
					lpScene2HZXHeader, i - 1) == 0)
					return 0;
#endif 
			}
			
			/* Radio を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Radio") == 0))
			{
#ifdef GROUP_CLEARING_NUMBER_CONVERT
				/* Radio があった */
				if(SearchAndGetRadio(hP3DModel, hP3DObject, lpScene2HZXHeader, i) == 0)
					return 0;
#else
				/* Radio があった */
				if(SearchAndGetRadio(hP3DModel, hP3DObject,
					lpScene2HZXHeader, i - 1) == 0)
					return 0;
#endif 
			}
		}
		while((hP3DObject = P3DGetNextObject(hP3DObject)) != NULL);
	}

	/* 進行状況出力 */
	printf("...........Done.\n");

	return 1;
}

/******************************************************************************
 * statics
 */

/******************************************************************************
 * グループ内を検索し、トラップオブジェクトを作成する
 */
static int SearchAndGetTrap(	/* 0 以外 正常 */
								/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	HP3DOBJECT        hP3DObject,			/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader,	/* HZX 用オブジェクトヘッダー */
	int               nGroupNumber)			/* グループ番号 */
{
	P3DXYZ        xyz1, xyz2;
	HP3DOBJECT    hP3DObjectL;
	LPGROUPOBJECT lpGroup;
	LPTRAPOBJECT  lpTrap;
	LPHZXTRAPINFO lpHZXTrapInfo;

	/* トラップオブジェクトを発見してここに入っている */

	/* 再帰的にループしてトラップを集める */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(SearchAndGetTrap(hP3DModel, hP3DObjectL, 
				lpScene2HZXHeader, nGroupNumber) == 0)
				return 0;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}

	if(hP3DObject == NULL)
		return 1;

	if(P3DObject(hP3DObject)->nPolygonCount == 0)
		return 1;

	/* グループを得る */
	lpGroup = lpScene2HZXHeader->lpGroup + nGroupNumber;

	/* トラップの領域を確保 */
	if((lpTrap = (LPTRAPOBJECT)reallocEx(lpGroup->lpTrap,
		sizeof(TRAPOBJECT) * (lpGroup->nTrapCount + 1))) == NULL)
		return 0;

	lpGroup->lpTrap = lpTrap;
	lpTrap         += lpGroup->nTrapCount;
	lpGroup->nTrapCount++;
	memset(lpTrap, 0x00, sizeof(TRAPOBJECT));

	/* バウンディングを求める */
	P3DAreaObject(&xyz1, &xyz2, hP3DObject);

	/* バウンディングを入力 */
	lpTrap->dxyzBound[0].x = xyz1.x;
	lpTrap->dxyzBound[0].y = xyz1.y;
	lpTrap->dxyzBound[0].z = xyz1.z;
	lpTrap->dxyzBound[1].x = xyz2.x;
	lpTrap->dxyzBound[1].y = xyz2.y;
	lpTrap->dxyzBound[1].z = xyz2.z;

	/* オブジェクト名を入力 */
	strncpy(lpTrap->szName, P3DObject(hP3DObject)->szName, _MAX_TEXT);

	/* 属性を取得 */
	/* 再確保 */
	if(P3DCreateUserDataObject(hP3DObject,
		HZX_TRAPKEYNAME, sizeof(HZXTRAPINFO)) == 0)
			return 0;

	/* トラップ属性構造体を取得 */
	lpHZXTrapInfo = (LPHZXTRAPINFO)
		P3DUserDataObject(hP3DObject, HZX_TRAPKEYNAME, NULL);

	/* ダークエリアを入力 */
	lpTrap->sAttribute |= (0x00F0 & (lpHZXTrapInfo->nDarkArea << 4));

	/* 属性を入力 */
	if(lpHZXTrapInfo->bNoCheck == TRUE) {
		lpTrap->sAttribute |= HZX_TRAP_NO_CHECK;
	}
	return 1;
}



/******************************************************************************
 * グループ内を検索し、ラジオオブジェクトを作成する
 * 現時点ではトラップとして扱うのでトラップオブジェクトの配列に組み込む
 * 将来的には別扱いになる可能性有り
 */
static int SearchAndGetRadio(	/* 0 以外 正常 */
								/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	HP3DOBJECT        hP3DObject,			/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader,	/* HZX 用オブジェクトヘッダー */
	int               nGroupNumber)			/* グループ番号 */
{
	P3DXYZ        xyz1, xyz2;
	HP3DOBJECT    hP3DObjectL;
	LPGROUPOBJECT lpGroup;
	LPTRAPOBJECT  lpRadio;

	/* ラジオオブジェクトを発見してここに入っている */

	/* 再帰的にループしてラジオを集める */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(SearchAndGetRadio(hP3DModel, hP3DObjectL, 
				lpScene2HZXHeader, nGroupNumber) == 0)
				return 0;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}

	if(hP3DObject == NULL)
		return 1;

	if(P3DObject(hP3DObject)->nPolygonCount == 0)
		return 1;

	/* グループを得る */
	lpGroup = lpScene2HZXHeader->lpGroup + nGroupNumber;

	/* ラジオの領域を確保 */
	if((lpRadio = (LPTRAPOBJECT)reallocEx(lpGroup->lpTrap,
		sizeof(TRAPOBJECT) * (lpGroup->nTrapCount + 1))) == NULL)
		return 0;

	lpGroup->lpTrap = lpRadio;
	lpRadio         += lpGroup->nTrapCount;
	lpGroup->nTrapCount++;
	memset(lpRadio, 0x00, sizeof(TRAPOBJECT));

	/* バウンディングを求める */
	P3DAreaObject(&xyz1, &xyz2, hP3DObject);

	/* バウンディングを入力 */
	lpRadio->dxyzBound[0].x = xyz1.x;
	lpRadio->dxyzBound[0].y = xyz1.y;
	lpRadio->dxyzBound[0].z = xyz1.z;
	lpRadio->dxyzBound[1].x = xyz2.x;
	lpRadio->dxyzBound[1].y = xyz2.y;
	lpRadio->dxyzBound[1].z = xyz2.z;

	/* オブジェクト名を入力 */
	strncpy(lpRadio->szName, P3DObject(hP3DObject)->szName, _MAX_TEXT);

	return 1;
}
