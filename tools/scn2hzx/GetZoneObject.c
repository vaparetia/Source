/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Zone Object
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

static int SearchAndGetZone(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader, int nGroupNumber);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * ゾーンオブジェクトの取得・生成
 */
int GetZoneObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	int            i;
	char           szGroupName[_MAX_TEXT];
	HP3DOBJECT     hP3DObject;

	/* 進行状況出力 */
	printf("Create Zone Object.");

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

		/* グループがあったのでグループのすぐしたの子らから Zone を探す */
		if((hP3DObject = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
			continue;

		do
		{
			/* Zone を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Zone") != 0))
				continue;

#ifdef GROUP_CLEARING_NUMBER_CONVERT
			/* Zone があった */
			if(SearchAndGetZone(hP3DModel, hP3DObject, lpScene2HZXHeader, i) == 0)
				return 0;
#else
			/* Zone があった */
			if(SearchAndGetZone(hP3DModel, hP3DObject,
				lpScene2HZXHeader, i - 1) == 0)
				return 0;
#endif 
		}
		while((hP3DObject = P3DGetNextObject(hP3DObject)) != NULL);
	}

	/* 進行状況出力 */
	printf("...........Done.\n");

	return 1;
}

/******************************************************************************
 * ゾーンオブジェクトの取得・生成
 */
int GetOldZoneObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	HP3DOBJECT     hP3DObject;

	/* 進行状況出力 */
	printf("Create Zone Object.");

	/* 旧 HZX 形態のシーンではモデルの直下に HZX がある */
	if((hP3DObject = P3DGetFirstObject(hP3DModel, NULL)) != NULL)
	{
		do
		{
			/* Zone を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Zone") != 0))
				continue;

			/* Zone があった */
			if(SearchAndGetZone(hP3DModel, hP3DObject,
				lpScene2HZXHeader, -1) == 0)
				return 0;
	
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
 * グループ内を検索し、ゾーンオブジェクトを作成する
 */
static int SearchAndGetZone(	/* 0 以外 正常 */
								/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	HP3DOBJECT        hP3DObject,			/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader,	/* HZX 用オブジェクトヘッダー */
	int               nGroupNumber)			/* グループ番号 負数は旧 HZX 形式のゾーン読み*/
{
	int               i, j;
	int               nSize;
	void*             lpData;
	LPGROUPOBJECT     lpGroup;
	LPZONEOBJECT      lpZone;
	LPP3DPOLYGON      lpP3DPolygon;
	LPP3DPOLYGONNODE  lpP3DPolygonNode;
	HP3DOBJECT        hP3DObjectL;
	HZXZONEINFO       hzxZoneInfo;

	/* ゾーンオブジェクトを発見してここに入っている */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(SearchAndGetZone(hP3DModel, hP3DObjectL,
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
	if(nGroupNumber < 0)
		lpGroup = lpScene2HZXHeader->lpGroup + 0;
	else	
		lpGroup = lpScene2HZXHeader->lpGroup + nGroupNumber;
	
	/* オブジェクトからポリゴンデータを取り出す */
	for(i = 0, lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon;
		i < P3DObject(hP3DObject)->nPolygonCount;
		i++, lpP3DPolygon++)
	{
		/* 頂点数チェック */
		if(lpP3DPolygon->nPolygonNodeCount != 4)
		{
			printf("Error! Found Zone Polygon != 4 Vertex : %d\n",
			        lpP3DPolygon->nPolygonNodeCount );
#if 1
			{
			    int			n, vn ;
			    LPP3DVERTEX		lpP3DVertex, v ;
			    LPP3DPOLYGONNODE	lpP3DPolygonNode ;
			    lpP3DPolygonNode = lpP3DPolygon->lpP3DPolygonNode ;
			    lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex ;
			    for ( n = 0; n < lpP3DPolygon->nPolygonNodeCount; n ++ ) {
				vn = ( lpP3DPolygonNode + n )->nVertex ;
				v =  lpP3DVertex + vn ;
				printf( "%lf %lf %lf\n", v->p3dXYZ.x, v->p3dXYZ.y, v->p3dXYZ.z ) ;
			    }
			}
#endif
			return 0;
		}

		/* ゾーンの領域を確保 */
		if((lpZone = (LPZONEOBJECT)reallocEx(lpGroup->lpZone,
			sizeof(ZONEOBJECT) * (lpGroup->nZoneCount + 1))) == NULL)
			return 0;

		lpGroup->lpZone = lpZone;
		lpZone         += lpGroup->nZoneCount;
		lpGroup->nZoneCount++;	
		memset(lpZone, 0x00, sizeof(ZONEOBJECT));
		
		/* 頂点数の格納 */
		lpZone->nVertexCount = lpP3DPolygon->nPolygonNodeCount;

		/* 確保した領域に頂点データを格納する */
		for(j = 0, lpP3DPolygonNode = lpP3DPolygon->lpP3DPolygonNode;
			j < lpP3DPolygon->nPolygonNodeCount; j++, lpP3DPolygonNode++)
		{
			lpZone->dxyzVertex[j].x = (P3DObject(hP3DObject)->lpP3DVertex
				+ lpP3DPolygonNode->nVertex)->p3dXYZCalc.x;

			lpZone->dxyzVertex[j].y = (P3DObject(hP3DObject)->lpP3DVertex
				+ lpP3DPolygonNode->nVertex)->p3dXYZCalc.y;

			lpZone->dxyzVertex[j].z = (P3DObject(hP3DObject)->lpP3DVertex
				+ lpP3DPolygonNode->nVertex)->p3dXYZCalc.z;
		}
	
		/* ユーザーデータを得る */
		if((lpData = P3DUserDataPolygon(
			lpP3DPolygon, HZX_ZONEKEYNAME, &nSize)) == NULL)
			return 1;

		memset(&hzxZoneInfo, 0x00, sizeof(HZXZONEINFO));
		memcpy(&hzxZoneInfo, lpData, __min(sizeof(HZXZONEINFO), nSize));

		/* ユーザーデータを格納 */
		lpZone->nIncremental = hzxZoneInfo.nIncremental;	/* 逐次番号 */
		lpZone->nFlag        = hzxZoneInfo.nFlag;			/* フラグ */
	}

	return 1;
}

