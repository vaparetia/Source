/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Floor Object
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

static int SearchAndGetFloor(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader, int nGroupNumber);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * 床オブジェクトの取得・生成
 */
int GetFloorObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	int            i;
	char           szGroupName[_MAX_TEXT];
	HP3DOBJECT     hP3DObject;

	/* 進行状況出力 */
	printf("Create Floor Object.");

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

		/* グループがあったのでグループのすぐしたの子らから Floor を探す */
		if((hP3DObject = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
			continue;

		do
		{
			/* Floor を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Floor") != 0))
				continue;

#ifdef GROUP_CLEARING_NUMBER_CONVERT
			/* Floor があった */
			if(SearchAndGetFloor(hP3DModel, hP3DObject, lpScene2HZXHeader, i) == 0)
				return 0;
#else
			/* Floor があった */
			if(SearchAndGetFloor(hP3DModel, hP3DObject,
				lpScene2HZXHeader, i - 1) == 0)
				return 0;
#endif 
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
 * グループ内を検索し、フロアオブジェクトを作成する
 */
static int SearchAndGetFloor(	/* 0 以外 正常 */
								/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	HP3DOBJECT        hP3DObject,			/* オブジェクトハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader,	/* HZX 用オブジェクトヘッダー */
	int               nGroupNumber)			/* グループ番号 */
{
	int              i, j;
	int              nSize;
	int              nVertexCount;
	void*            lpData;
	LPP3DXYZ         lpP3DXYZVertex;
	HP3DOBJECT       hP3DObjectL;
	LPP3DPOLYGON     lpP3DPolygon;
	LPP3DPOLYGONNODE lpP3DPolygonNode;
	HZXFLOORINFO     hzxFloorInfo;
	LPGROUPOBJECT    lpGroup;
	LPSEGMENTOBJECT  lpFloor;

	/* フロアオブジェクトを発見してここに入っている */

	/* 再帰的にループしてフロアを集める */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(SearchAndGetFloor(hP3DModel, hP3DObjectL, 
				lpScene2HZXHeader, nGroupNumber) == 0)
				return 0;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}

	if(hP3DObject == NULL)
		return 1;

	if(P3DObject(hP3DObject)->nPolygonCount == 0)
		return 1;

	/* オブジェクトからポリゴンデータを取り出す */
	for(i = 0, lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon;
		i < P3DObject(hP3DObject)->nPolygonCount;
		i++, lpP3DPolygon++)
	{
		/* ２頂点以下は不可 */
		if(lpP3DPolygon->nPolygonNodeCount < 3)
		{
			printf("Error! Found Floor Polygon < 3 Vertex");
			return 0;
		}

		/* 頂点データを取り出す */
		if((lpP3DXYZVertex = (LPP3DXYZ)calloc(lpP3DPolygon->nPolygonNodeCount,
			sizeof(P3DXYZ))) == NULL)
			return 0;

		/* 確保した領域に頂点データを格納する */
		for(j = 0, lpP3DPolygonNode = lpP3DPolygon->lpP3DPolygonNode;
			j < lpP3DPolygon->nPolygonNodeCount; j++, lpP3DPolygonNode++)
		{
			*(lpP3DXYZVertex + j) = (P3DObject(hP3DObject)->lpP3DVertex
				+ lpP3DPolygonNode->nVertex)->p3dXYZCalc;
		}

		/* 同一直線上に３つ以上ならぶ頂点があったなら削除する */
		if((nVertexCount = OESDeleteOverVertexOnLine(lpP3DXYZVertex,
			lpP3DPolygon->nPolygonNodeCount, 0.01)) < 0)
		{
			freeEx(lpP3DXYZVertex);
			continue;
		}
	
		/* 以降、頂点数は nVertexCount。頂点は lpP3DXYZVertex を参照 */

		/* ５頂点以上は不可 */
		if(nVertexCount > 4)
		{
			printf("Error! Found Floor Polygon > 4 Vertex\n");

			for(j = 0; j < nVertexCount; j++)
			{
				printf("Vertex%d = (%f, %f, %f)\n", j, 
				(lpP3DXYZVertex + j)->x, (lpP3DXYZVertex + j)->y, (lpP3DXYZVertex + j)->z);
			}

			freeEx(lpP3DXYZVertex);
			return 0;
		}

		/* ハザードデータ領域を確保 */
		lpGroup = lpScene2HZXHeader->lpGroup + nGroupNumber;
		
		if((lpFloor = (LPSEGMENTOBJECT)reallocEx(lpGroup->lpFloor,
			(sizeof(SEGMENTOBJECT) * (lpGroup->nFloorCount + 1)))) == NULL)
		{
			freeEx(lpP3DXYZVertex);
			return 0;
		}

		/* フロアデータ領域を確保できた */
		lpGroup->lpFloor = lpFloor;
		lpFloor += lpGroup->nFloorCount;
		(lpGroup->nFloorCount)++;
		
		/* 新規確保した領域の初期化 */
		memset(lpFloor, 0x00, sizeof(SEGMENTOBJECT));
		
		/* 頂点数の設定 */
		lpFloor->nVertexCount = nVertexCount;
		
		/* ポリゴンの頂点データを取り出す */
		for(j = 0; j < nVertexCount; j++)
		{
			lpFloor->dxyzVertex[j].x = (lpP3DXYZVertex + j)->x;
			lpFloor->dxyzVertex[j].y = (lpP3DXYZVertex + j)->y;
			lpFloor->dxyzVertex[j].z = (lpP3DXYZVertex + j)->z;
		}

		/* ユーザーデータからアトリビュートを取り出す */	
		if((lpData = P3DUserDataPolygon(
			lpP3DPolygon, HZX_FLOORKEYNAME, &nSize)) == NULL)
		{
			freeEx(lpP3DXYZVertex);
			continue;
		}

		memset(&hzxFloorInfo, 0x00, sizeof(HZXFLOORINFO));
		memcpy(&hzxFloorInfo, lpData, __min(sizeof(HZXFLOORINFO), nSize));
		
		/* 新属性追加 */
		if(hzxFloorInfo.bNoPlayer == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_PLAYER;
		if(hzxFloorInfo.bNoEnemy == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_ENEMY;
		if(hzxFloorInfo.bNoBullet == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_BULLET;
		if(hzxFloorInfo.bNoMissile == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_MISSILE;
		if(hzxFloorInfo.bNoC4 == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_C4;
		if(hzxFloorInfo.bNoRecoil == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_RECOIL;
		if(hzxFloorInfo.bNoClaymore == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_CLAYMORE;
		if(hzxFloorInfo.bNoBlood == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_BLOOD;
		if(hzxFloorInfo.bNoBulletHole == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_BULLETHOLE;
		if(hzxFloorInfo.bNoSpray == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_SPRAY;
		if(hzxFloorInfo.bInverseKinematics == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_INVERSE_KINEMATICS;
		if(hzxFloorInfo.bStep == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_STEP;
		if(hzxFloorInfo.bRecoilOnly == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_RECOILE_TYPE;
		if(hzxFloorInfo.bNoEnemyEyes == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_ENEMY_EYES;
		if(hzxFloorInfo.bNoObject == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_NO_OBJECT;
		if(hzxFloorInfo.bCypher == TRUE)
			lpFloor->nAttribute |= HZX_FLOOR_CYPHER;

		lpFloor->nAttribute |= (hzxFloorInfo.nSound & 0x0000000f) << 28;

		/* 頂点データの開放 */
		freeEx(lpP3DXYZVertex);
	}

	return 1;
}