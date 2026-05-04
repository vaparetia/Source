/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Get Hazard Object
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

static int SearchAndGetHazard(HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, LPSCENE2HZXHEADER lpScene2HZXHeader, int nGroupNumber);

/******************************************************************************
 * publics
 */

/******************************************************************************
 * 壁オブジェクトの取得・生成
 */
int GetHazardObject(	/* 0 以外 正常 */
							/* 0 異常 */
	HP3DMODEL         hP3DModel,			/* モデルハンドル */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* HZX 用オブジェクトヘッダー */
{
	int            i;
	char           szGroupName[_MAX_TEXT];
	HP3DOBJECT     hP3DObject;

	/* 進行状況出力 */
	printf("Create Hazard Object.");

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

		/* グループがあったのでグループのすぐしたの子らから Hazard を探す */
		if((hP3DObject = P3DGetFirstObject(hP3DModel, hP3DObject)) == NULL)
			continue;

		do
		{
			/* Hazard を探す */
			if( (hP3DObject == NULL) || 
				(strcmp(P3DObject(hP3DObject)->szName, "Hazard") != 0))
				continue;

#ifdef GROUP_CLEARING_NUMBER_CONVERT
			/* Hazard があった */
			if(SearchAndGetHazard(hP3DModel, hP3DObject, lpScene2HZXHeader, i) == 0)
				return 0;
#else
			/* Hazard があった */
			if(SearchAndGetHazard(hP3DModel, hP3DObject,
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
 * ねじれチェックと小さい順並びを行う
 */
static BOOL CheckHazard(	/* 0 以外 正常 */
							/* 0 異常 */
	LPP3DXYZ     lpP3DXYZVertex,	/* 余分な頂点削除済み頂点配列。こちらを使う */
	int          nVertexCount)		/* 余分な頂点削除済み頂点数。こちらを使う */
{
	int              i, j;
	double           dTmp1, dTmp2;
	BOOL             bLoop;
	BOOL             bStatus;
	BOOL             bVertexStatus[4];
	P3DXYZ           xyz[4];
	P3DXYZ           xyzTmp;

	for(i = 0; i < nVertexCount; i++)
		bVertexStatus[i] = FALSE;

	/* 第一頂点 */
	i = 0;
	bVertexStatus[0] = TRUE;
	xyz[0] = *(lpP3DXYZVertex + 0);
	xyz[1] = xyz[0];

	for(i = 1; (i < nVertexCount) && ((bVertexStatus[i] == TRUE)
		|| (__fequal((lpP3DXYZVertex + i)->x, xyz[0].x, HZXCALCERROR) == FALSE)
		|| (__fequal((lpP3DXYZVertex + i)->z, xyz[0].z, HZXCALCERROR) == FALSE)); i++);

	if(i < nVertexCount)
	{
//printf( "<0:%d/%d>", i, nVertexCount ) ;
		bVertexStatus[i] = TRUE;
		xyz[1] = *(lpP3DXYZVertex + i);
	}

	/* 第二頂点 */
	for(i = 0; (i < nVertexCount) && (bVertexStatus[i] == TRUE); i++);
//printf( "<<%d>>\n", i ) ;
	bVertexStatus[i] = TRUE;
	xyz[2] = *(lpP3DXYZVertex + i);
	xyz[3] = xyz[2];

	for(i++; (i < nVertexCount) && ((bVertexStatus[i] == TRUE)
		|| (__fequal((lpP3DXYZVertex + i)->x, xyz[2].x, HZXCALCERROR) == FALSE)
		|| (__fequal((lpP3DXYZVertex + i)->z, xyz[2].z, HZXCALCERROR) == FALSE)); i++) {
//	    printf( "%d : %f %f\n", i, (lpP3DXYZVertex + i)->x, xyz[ 2 ].x ) ;
//	    printf( "%f %f\n", (lpP3DXYZVertex + i)->z, xyz[ 2 ].z ) ;
	}

	if(i < nVertexCount)
	{
//printf( "<1:%d>", i ) ;
		bVertexStatus[i] = TRUE;
		xyz[3] = *(lpP3DXYZVertex + i);
	}

	for(i = 0; (i < nVertexCount) && (bVertexStatus[i] == TRUE); i++);

//printf( "%f %f %f\n", xyz[ 0 ].x, xyz[ 0 ].y, xyz[ 0 ].z ) ;
//printf( "%f %f %f\n", xyz[ 1 ].x, xyz[ 1 ].y, xyz[ 1 ].z ) ;
//printf( "%f %f %f\n", xyz[ 2 ].x, xyz[ 2 ].y, xyz[ 2 ].z ) ;
//printf( "%f %f %f\n", xyz[ 3 ].x, xyz[ 3 ].y, xyz[ 3 ].z ) ;
//printf( "\n" ) ;
	if(i < nVertexCount)
	{
		printf("Error! Found Twist Polygon.");
		return 0;
	}

	// TRUE : x 座標で比較 FALSE : z 座標で比較
	if( (__abs(xyz[1].x - xyz[0].x) < 0.0001) &&
		(__abs(xyz[2].x - xyz[0].x) < 0.0001))
		bLoop = FALSE;
	else
		bLoop = TRUE;

	// 頂点を小さい順に並び替え
	bStatus = TRUE;
	if ( bLoop == TRUE ) {
	    if ( xyz[ 0 ].x > xyz[ 2 ].x ) {
		xyzTmp = xyz[ 0 ] ; xyz[ 0 ] = xyz[ 2 ] ; xyz[ 2 ] = xyzTmp ;
		xyzTmp = xyz[ 1 ] ; xyz[ 1 ] = xyz[ 3 ] ; xyz[ 3 ] = xyzTmp ;
	    }
	} else {
	    if ( xyz[ 0 ].z > xyz[ 2 ].z ) {
		xyzTmp = xyz[ 0 ] ; xyz[ 0 ] = xyz[ 2 ] ; xyz[ 2 ] = xyzTmp ;
		xyzTmp = xyz[ 1 ] ; xyz[ 1 ] = xyz[ 3 ] ; xyz[ 3 ] = xyzTmp ;
	    }
	}
	if ( xyz[ 0 ].y > xyz[ 1 ].y ) {
		xyzTmp = xyz[ 0 ] ; xyz[ 0 ] = xyz[ 1 ] ; xyz[ 1 ] = xyzTmp ;
	}
	if ( xyz[ 2 ].y > xyz[ 3 ].y ) {
		xyzTmp = xyz[ 2 ] ; xyz[ 2 ] = xyz[ 3 ] ; xyz[ 3 ] = xyzTmp ;
	}
#if 0
	while(bStatus)
	{
		bStatus = FALSE;
		for(i = 0; i < 3; i++)
		{
			// TRUE : x 座標で比較　FALSE : z 座標で比較
			if(bLoop == TRUE)
			{
				dTmp1 = xyz[i].x;
				dTmp2 = xyz[i + 1].x;
			}
			else
			{
				dTmp1 = xyz[i].z;
				dTmp2 = xyz[i + 1].z;
			}

			if(dTmp1 > dTmp2)
			{
				// もう一度ループする
				bStatus = TRUE;
				
				// 配列を回す
				xyzTmp = xyz[0];
				for(j = 0; j < 3; j++) xyz[j] = xyz[j + 1];
				xyz[3] = xyzTmp;
				break;
			}
		}
	}
#endif
	/* 修正した頂点を戻す */
	for(i = 0; i < 4; i++)
		*(lpP3DXYZVertex + i) = xyz[i];
	
	return 1;
}

/******************************************************************************
 * グループ内を検索し、ハザードオブジェクトを作成する
 */
static int SearchAndGetHazard(	/* 0 以外 正常 */
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
	HZXHAZARDINFO    hzxHazardInfo;
	LPGROUPOBJECT    lpGroup;
	LPSEGMENTOBJECT  lpHazard;

	/* ハザードオブジェクトを発見してここに入っている */

	/* 再帰的にループしてハザードを集める */
	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(SearchAndGetHazard(hP3DModel, hP3DObjectL, 
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
			printf("Error! Found Hazard Polygon < 3 Vertex");
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
			printf("Error! Found Hazard Polygon > 4 Vertex");

			for(j = 0; j < nVertexCount; j++)
			{
				printf("Vertex%d = (%f, %f, %f)\n", j, 
				(lpP3DXYZVertex + j)->x, (lpP3DXYZVertex + j)->y, (lpP3DXYZVertex + j)->z);
			}

			freeEx(lpP3DXYZVertex);
			return 0;
		}
		// ３頂点は４頂点領域に確保し直す
		if(nVertexCount == 3)
		{
			if((lpP3DXYZVertex = (LPP3DXYZ)reallocEx(lpP3DXYZVertex, 
				(sizeof(P3DXYZ) * 4))) == NULL)
			{
				freeEx(lpP3DXYZVertex);
				return 0;
			}
		}
		/* ねじれチェックと小さい順並び */
		if(CheckHazard(lpP3DXYZVertex, nVertexCount) == 0)
		{
			freeEx(lpP3DXYZVertex);
			return 0;
		}
		// 3頂点のもので最初の２頂点が同じものは前詰め
		if(nVertexCount == 3)
		{
			if(OESCheckSameVertex((lpP3DXYZVertex + 0),
				(lpP3DXYZVertex + 1), 0.01) == TRUE)
				nVertexCount = OESDeleteVertex(lpP3DXYZVertex, 4, 0);
		}
		/* ハザードデータ領域を確保 */
		lpGroup = lpScene2HZXHeader->lpGroup + nGroupNumber;
		
		if((lpHazard = (LPSEGMENTOBJECT)reallocEx(lpGroup->lpHazard,
			(sizeof(SEGMENTOBJECT) * (lpGroup->nHazardCount + 1)))) == NULL)
		{
			freeEx(lpP3DXYZVertex);
			return 0;
		}
		/* ハザードデータ領域を確保できた */
		lpGroup->lpHazard = lpHazard;
		lpHazard += lpGroup->nHazardCount;
		(lpGroup->nHazardCount)++;
		
		/* 新規確保した領域の初期化 */
		memset(lpHazard, 0x00, sizeof(SEGMENTOBJECT));
		
		/* 頂点数の設定 */
		lpHazard->nVertexCount = nVertexCount;
		/* ポリゴンの頂点データを取り出す */
		for(j = 0; j < nVertexCount; j++)
		{
			lpHazard->dxyzVertex[j].x = (lpP3DXYZVertex + j)->x;
			lpHazard->dxyzVertex[j].y = (lpP3DXYZVertex + j)->y;
			lpHazard->dxyzVertex[j].z = (lpP3DXYZVertex + j)->z;
		}
		/* ユーザーデータからアトリビュートを取り出す */	
		if((lpData = P3DUserDataPolygon(
			lpP3DPolygon, HZX_HAZARDKEYNAME, &nSize)) == NULL)
		{
			freeEx(lpP3DXYZVertex);
			continue;
		}

		memset(&hzxHazardInfo, 0x00, sizeof(HZXHAZARDINFO));
		memcpy(&hzxHazardInfo, lpData, __min(sizeof(HZXHAZARDINFO), nSize));
		
		if(hzxHazardInfo.bNoPlayer == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_PLAYER;
		if(hzxHazardInfo.bNoEnemy == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_ENEMY;
		if(hzxHazardInfo.bNoBullet == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_BULLET;
		if(hzxHazardInfo.bNoMissile == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_MISSILE;
		if(hzxHazardInfo.bNoC4 == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_C4;
		if(hzxHazardInfo.bNoRecoil == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_RECOIL;
		if(hzxHazardInfo.bNoHarituki == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_HARITSUKI;
		if(hzxHazardInfo.bNoDispRadar == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_DISP_RADAR;
		if(hzxHazardInfo.bNoBulletHole == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_BULLETHOLE;
		if(hzxHazardInfo.bNoSpray == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_SPRAY;
		if(hzxHazardInfo.bRecoilOnly == TRUE)
			lpHazard->nAttribute |= HZX_SEG_RECOIL_TYPE;
		if(hzxHazardInfo.bNoEnemyEyes == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_ENEMY_EYES;
		if(hzxHazardInfo.bNoKnockSE == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_KNOCK_SE;
		if(hzxHazardInfo.bCypher == TRUE)
			lpHazard->nAttribute |= HZX_SEG_CYPHER;
		if(hzxHazardInfo.bNoEnemyIK == TRUE)
			lpHazard->nAttribute |= HZX_SEG_NO_ENEMY_IK;
		if(hzxHazardInfo.bCliff == TRUE)
			lpHazard->nAttribute |= HZX_SEG_CLIFF;

		lpHazard->nAttribute |= (hzxHazardInfo.nSound & 0x000f) << 28 ;

		/* 頂点データの開放 */
		freeEx(lpP3DXYZVertex);
	}

	return 1;
}


