/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Group Object
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
 * publics
 */

/******************************************************************************
 * グループオブジェクトの取得・生成
 */
int GetGroupObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	int            i;
	char           szGroupName[_MAX_TEXT];
	HP3DOBJECT     hP3DObject;
	LPHZXGROUPINFO lpHZXGroupInfo;
	LPGROUPOBJECT  lpGroup;

	/* 進行状況出力 */
	printf("Create Group Object.");

#ifdef GROUP_CLEARING_NUMBER_CONVERT
	for(i = 0; ; i++)
#else
	for(i = 1; ; i++)
#endif 
	{
		/* グループ */
		sprintf(szGroupName, "Group-%d", i);
		if((hP3DObject = P3DFindObjectName(hP3DModel, szGroupName)) == NULL)
			break;

		/* グループがあった */
		if((lpGroup = (LPGROUPOBJECT)reallocEx(lpScene2HZXHeader->lpGroup,
			(sizeof(GROUPOBJECT) * (lpScene2HZXHeader->nGroupCount + 1)))) == NULL)
			return 0;

		/* 領域が確保できた */
		lpScene2HZXHeader->lpGroup = lpGroup;
		/* lpGroup は新しく確保したグループ領域を指す */
		lpGroup += lpScene2HZXHeader->nGroupCount;
		(lpScene2HZXHeader->nGroupCount)++;

		/* グループの初期化 */
		/* バウンディングの最小値と１ブロックのサイズは一番最後にもう一度設定する */
		lpGroup->nxyzBoundMin.x = 0;
		lpGroup->nxyzBoundMin.y = 0;
		lpGroup->nxyzBoundMin.z = 0;

		lpGroup->nxyzSize.x = 0;
		lpGroup->nxyzSize.y = 0;
		lpGroup->nxyzSize.z = 0;

		/* 分割数の設定：グループのユーザーデータを取り出す */
		if((lpHZXGroupInfo = (LPHZXGROUPINFO)P3DUserDataObject(
			hP3DObject, HZX_GROUPKEYNAME, NULL)) == NULL)
		{
			lpGroup->nxyzDivide.x = 1;
			lpGroup->nxyzDivide.y = 1;
			lpGroup->nxyzDivide.z = 1;
		}
		else
		{
			if(lpHZXGroupInfo->nXDivide != 0)
				lpGroup->nxyzDivide.x = lpHZXGroupInfo->nXDivide;
			else
				lpGroup->nxyzDivide.x = 1;

			if(lpHZXGroupInfo->nYDivide != 0)
				lpGroup->nxyzDivide.y = lpHZXGroupInfo->nYDivide;
			else
				lpGroup->nxyzDivide.y = 1;

			if(lpHZXGroupInfo->nZDivide != 0)
				lpGroup->nxyzDivide.z = lpHZXGroupInfo->nZDivide;
			else
				lpGroup->nxyzDivide.z = 1;
		}

		/* カウンタの初期化 */
		lpGroup->nHazardCount = 0;
		lpGroup->nFloorCount  = 0;
		lpGroup->nTrapCount   = 0;
		lpGroup->nCameraCount = 0;
		lpGroup->nZoneCount   = 0;
		lpGroup->nBlockCount  = 0;

		/* ポインタの初期化 */
		lpGroup->lpHazard = NULL;
		lpGroup->lpFloor  = NULL;
		lpGroup->lpTrap   = NULL;
		lpGroup->lpCamera = NULL;
		lpGroup->lpZone   = NULL;
		lpGroup->lpBlock  = NULL;
	}

	/* 1 以上にカウンターが進まなかったのなら、エラー */
#ifdef GROUP_CLEARING_NUMBER_CONVERT
	if(i == 0)
#else
	if(i == 1)
#endif 
	{
		printf("Warning! Not Group Object.");
		return 1;
	}

	/* 進行状況出力 */
	printf("..........Done.\n");

	return 1;
}
