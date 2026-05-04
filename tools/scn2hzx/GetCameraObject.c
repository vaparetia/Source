/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Camera Object
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

static int SearchAndGetCamera(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader, int nGroupNumber);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * カメラオブジェクトの取得・生成
 */
int GetCameraObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	int            i;
	char           szGroupName[_MAX_TEXT];
	HP3DOBJECT     hP3DObject;

	/* 進行状況出力 */
	printf("Create Camera Object.");

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

		/* グループがあったのでグループのすぐしたの子らから Camera を探す */
		if((hP3DObject = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
			continue;

		do
		{
			/* Camera を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Camera") != 0))
				continue;

printf("Found! Group-%d, Camera!\n", i);

			/* Camera があった */
#ifdef GROUP_CLEARING_NUMBER_CONVERT
			if(SearchAndGetCamera(hP3DModel, hP3DObject, lpScene2HZXHeader, i) == 0)
				return 0;
#else
			if(SearchAndGetCamera(hP3DModel, hP3DObject,
				lpScene2HZXHeader, i - 1) == 0)
				return 0;
#endif 
		}
		while((hP3DObject = P3DGetNextObject(hP3DObject)) != NULL);
	}

	/* 進行状況出力 */
	printf(".........Done.\n");
	return 1;
}

/******************************************************************************
 * statics
 */

/******************************************************************************
 * グループ内を検索し、カメラオブジェクトを作成する
 */
static int SearchAndGetCamera(	/* 0 以外 正常 */
								/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	HP3DOBJECT        hP3DObject,			/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader,	/* HZX 用オブジェクトヘッダー */
	int               nGroupNumber)			/* グループ番号 */
{
	int              nSize;
	void*            lpData;
	P3DXYZ           xyz1, xyz2;
	HP3DOBJECT       hP3DObjectL;
	LPGROUPOBJECT    lpGroup;
	LPCAMERAOBJECT   lpCamera;
	HZXCAMERAINFO    hzxCameraInfo;

	/* カメラオブジェクトを発見してここに入っている */

	/* 再帰的にループしてカメラを集める */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(SearchAndGetCamera(hP3DModel, hP3DObjectL, 
				lpScene2HZXHeader, nGroupNumber) == 0)
				return 0;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}

	if(hP3DObject == NULL)
		return 1;

	if(P3DObject(hP3DObject)->nPolygonCount == 0)
		return 1;


printf("Get Camera %s, Group-%d\n", P3DObject(hP3DObject)->szName, nGroupNumber);

	/* グループを得る */
	lpGroup = lpScene2HZXHeader->lpGroup + nGroupNumber;

	/* カメラの領域を確保 */
	if((lpCamera = (LPCAMERAOBJECT)reallocEx(lpGroup->lpCamera,
		sizeof(CAMERAOBJECT) * (lpGroup->nCameraCount + 1))) == NULL)
		return 0;

	lpGroup->lpCamera = lpCamera;
	lpCamera         += lpGroup->nCameraCount;
	lpGroup->nCameraCount++;
	memset(lpCamera, 0x00, sizeof(CAMERAOBJECT));

	/* バウンディングを求める */
	P3DAreaObject(&xyz1, &xyz2, hP3DObject);

	/* バウンディングを入力 */
	lpCamera->dxyzBound[0].x = xyz1.x;
	lpCamera->dxyzBound[0].y = xyz1.y;
	lpCamera->dxyzBound[0].z = xyz1.z;
	lpCamera->dxyzBound[1].x = xyz2.x;
	lpCamera->dxyzBound[1].y = xyz2.y;
	lpCamera->dxyzBound[1].z = xyz2.z;

	/* ユーザーデータを得る */
	if((lpData = P3DUserDataObject(
		hP3DObject, HZX_CAMERAKEYNAME, &nSize)) == NULL)
		return 1;

	if(nSize != sizeof(HZXCAMERAINFO))
	{
		/* 旧 タイプのユーザーデータ */
		memset(&hzxCameraInfo, 0x00, sizeof(HZXCAMERAINFO));
		memcpy(&hzxCameraInfo, lpData, __min(sizeof(HZXCAMERAINFO), nSize));
		hzxCameraInfo.xyzRightTarget.x = 0;
		hzxCameraInfo.xyzRightTarget.y = 0;
		hzxCameraInfo.xyzRightTarget.z = 0;
		hzxCameraInfo.xyzRightCamera.x = 0;
		hzxCameraInfo.xyzRightCamera.y = 0;
		hzxCameraInfo.xyzRightCamera.z = 0;
		hzxCameraInfo.xyzLeftTarget.x  = 0;
		hzxCameraInfo.xyzLeftTarget.y  = 0;
		hzxCameraInfo.xyzLeftTarget.z  = 0;
		hzxCameraInfo.xyzLeftCamera.x  = 0;
		hzxCameraInfo.xyzLeftCamera.y  = 0;
		hzxCameraInfo.xyzLeftCamera.z  = 0;
		hzxCameraInfo.nOptionDir       = 0;
		hzxCameraInfo.unOptionFlag     = 0;
	}
	else
	{
		/* ユーザーデータをコピー */
		memset(&hzxCameraInfo, 0x00, sizeof(HZXCAMERAINFO));
		memcpy(&hzxCameraInfo, lpData, __min(sizeof(HZXCAMERAINFO), nSize));
	}

	lpCamera->dxyzTarget.x      = hzxCameraInfo.xyzTarget.x;
	lpCamera->dxyzTarget.y      = hzxCameraInfo.xyzTarget.y;
	lpCamera->dxyzTarget.z      = hzxCameraInfo.xyzTarget.z;
	lpCamera->dxyzCamera.x      = hzxCameraInfo.xyzCamera.x;
	lpCamera->dxyzCamera.y      = hzxCameraInfo.xyzCamera.y;
	lpCamera->dxyzCamera.z      = hzxCameraInfo.xyzCamera.z;
	lpCamera->dxyzRightTarget.x = hzxCameraInfo.xyzRightTarget.x;
	lpCamera->dxyzRightTarget.y = hzxCameraInfo.xyzRightTarget.y;
	lpCamera->dxyzRightTarget.z = hzxCameraInfo.xyzRightTarget.z;
	lpCamera->dxyzRightCamera.x = hzxCameraInfo.xyzRightCamera.x;
	lpCamera->dxyzRightCamera.y = hzxCameraInfo.xyzRightCamera.y;
	lpCamera->dxyzRightCamera.z = hzxCameraInfo.xyzRightCamera.z;
	lpCamera->dxyzLeftTarget.x  = hzxCameraInfo.xyzLeftTarget.x;
	lpCamera->dxyzLeftTarget.y  = hzxCameraInfo.xyzLeftTarget.y;
	lpCamera->dxyzLeftTarget.z  = hzxCameraInfo.xyzLeftTarget.z;
	lpCamera->dxyzLeftCamera.x  = hzxCameraInfo.xyzLeftCamera.x;
	lpCamera->dxyzLeftCamera.y  = hzxCameraInfo.xyzLeftCamera.y;
	lpCamera->dxyzLeftCamera.z  = hzxCameraInfo.xyzLeftCamera.z;
	lpCamera->nPadData[0]       = hzxCameraInfo.nOptionDir;
	lpCamera->nPadData[1]       = (int)(hzxCameraInfo.unOptionFlag);
	return 1;
}
