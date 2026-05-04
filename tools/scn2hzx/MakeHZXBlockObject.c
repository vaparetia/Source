/******************************************************************************
 * System	: Scene To HZX File Convert exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Make HZX Block Object
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <math.h>

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
static void GetBoundingSegment(LPSEGMENTOBJECT lpSegmentObject, int nSegmentCount, LPDOUBLEXYZ lpdxyzBoundMax, LPDOUBLEXYZ lpdxyzBoundMin);
static void GetBoundingTrap(LPTRAPOBJECT lpTrapObject, int nTrapCount, LPDOUBLEXYZ lpdxyzBoundMax, LPDOUBLEXYZ lpdxyzBoundMin);
static int  DivideObject(LPGROUPOBJECT lpGroup);
static int  GetDivideCount(int nMax, int nMin, int nDivide, int nSize);
static int  GetSizeCount(int nMax, int nMin, int nDivide, int nSize);
static int  IntDropUp(double dParam);
static int  IntDropDown(double dParam);

/******************************************************************************
 * publics
 */


/******************************************************************************
 * ブロックオブジェクトの作成。
 */

int MakeHZXBlockObject(		/* 0 以外 正常 */
	LPSCENE2HZXHEADER lpScene2HZXHeader)	/* 読み込んだシーンから取り出したオブジェクト */
{
	int           i;
	LPGROUPOBJECT lpGroup;
	DOUBLEXYZ     dxyzBoundMax;
	DOUBLEXYZ     dxyzBoundMin;
	DOUBLEXYZ     dxyzTrapBoundMin, dxyzTrapBoundMax ;	
	INTXYZ        nxyzBoundMax;
	INTXYZ        nxyzBoundMin;

	/* 進行状況出力 */
	printf("\nCreate Block Object.");

	for(i = 0; i < lpScene2HZXHeader->nGroupCount; i++)
	{
		lpGroup = lpScene2HZXHeader->lpGroup + i;

		dxyzBoundMax.x = -HUGE_VAL;
		dxyzBoundMax.y = -HUGE_VAL;
		dxyzBoundMax.z = -HUGE_VAL;
		dxyzBoundMin.x = HUGE_VAL;
		dxyzBoundMin.y = HUGE_VAL;
		dxyzBoundMin.z = HUGE_VAL;

		/* ハザード・フロア・トラップがあるか */
		if((lpGroup->lpHazard == NULL) && (lpGroup->lpFloor == NULL) &&
			(lpGroup->lpTrap == NULL))
		{
			/* ブロックオブジェクトの領域確保 */
			if((lpGroup->lpBlock = (LPBLOCKOBJECT)reallocEx(NULL,
				sizeof(BLOCKOBJECT))) == NULL)
				return 0;

			memset(lpGroup->lpBlock, 0x00, sizeof(BLOCKOBJECT));

			/* グループオブジェクトメンバの初期化 */
			lpGroup->nxyzBoundMin.x = 0;
			lpGroup->nxyzBoundMin.y = 0;
			lpGroup->nxyzBoundMin.z = 0;

			lpGroup->nxyzSize.x = 0;
			lpGroup->nxyzSize.y = 0;
			lpGroup->nxyzSize.z = 0;

			lpGroup->nxyzDivideReCalc.x = lpGroup->nxyzDivide.x;
			lpGroup->nxyzDivideReCalc.y = lpGroup->nxyzDivide.y;
			lpGroup->nxyzDivideReCalc.z = lpGroup->nxyzDivide.z;

			continue;
		}
		
		/* まずバウンディングを求める */
		/* 参照するのはハザード、フロア、トラップである */
		GetBoundingSegment(lpGroup->lpHazard, lpGroup->nHazardCount,
			&dxyzBoundMax, &dxyzBoundMin);

		GetBoundingSegment(lpGroup->lpFloor, lpGroup->nFloorCount,
			&dxyzBoundMax, &dxyzBoundMin);

		/* トラップは高さのみ参照 */
		GetBoundingTrap(lpGroup->lpTrap, lpGroup->nTrapCount,
						&dxyzTrapBoundMax, &dxyzTrapBoundMin);
		dxyzBoundMax.y = __max( dxyzTrapBoundMax.y, dxyzBoundMax.y ) ;
		dxyzBoundMin.y = __min( dxyzTrapBoundMin.y, dxyzBoundMin.y ) ;
		
		if( (fabs(dxyzBoundMax.x - dxyzBoundMin.x) > 500000) ||
			(fabs(dxyzBoundMax.y - dxyzBoundMin.y) > 500000) ||
			(fabs(dxyzBoundMax.z - dxyzBoundMin.z) > 500000))
		{
			printf("Group-%d Bounding Size over! > 500000\n", i + 1);
			return 0;
		}

		/* int 型にキャスト。Max は四捨五入。Min は切り捨て */
		nxyzBoundMax.x = IntDropUp(dxyzBoundMax.x);
		nxyzBoundMax.y = IntDropUp(dxyzBoundMax.y);
		nxyzBoundMax.z = IntDropUp(dxyzBoundMax.z);
		nxyzBoundMin.x = IntDropDown(dxyzBoundMin.x);
		nxyzBoundMin.y = IntDropDown(dxyzBoundMin.y);
		nxyzBoundMin.z = IntDropDown(dxyzBoundMin.z);
printf( "[%d] %d %d %d %d %d %d - ", i, nxyzBoundMin.x, nxyzBoundMin.y, nxyzBoundMin.z,
		 nxyzBoundMax.x, nxyzBoundMax.y, nxyzBoundMax.z ) ;
printf( "%lf %lf %lf %lf %lf %lf\n", dxyzBoundMin.x, dxyzBoundMin.y, dxyzBoundMin.z,
	     dxyzBoundMax.x, dxyzBoundMax.y, dxyzBoundMax.z ) ;
		/* Max, Min が同じ値のときはちょっと値をいじることにする */
		if(nxyzBoundMax.x == nxyzBoundMin.x)
		{
			nxyzBoundMax.x++;
			nxyzBoundMin.x--;
		}
		if(nxyzBoundMax.y == nxyzBoundMin.y)
		{
			nxyzBoundMax.y++;
			nxyzBoundMin.y--;
		}
		if(nxyzBoundMax.z == nxyzBoundMin.z)
		{
			nxyzBoundMax.z++;
			nxyzBoundMin.z--;
		}

		/* 分割数の設定 */		
		lpGroup->nxyzDivideReCalc.x = lpGroup->nxyzDivide.x;
		lpGroup->nxyzDivideReCalc.y = lpGroup->nxyzDivide.y;
		lpGroup->nxyzDivideReCalc.z = lpGroup->nxyzDivide.z;

		/* サイズの設定 */
		lpGroup->nxyzSize.x   = (nxyzBoundMax.x - nxyzBoundMin.x) /
			lpGroup->nxyzDivideReCalc.x;
		lpGroup->nxyzSize.y   = (nxyzBoundMax.y - nxyzBoundMin.y) /
			lpGroup->nxyzDivideReCalc.y;
		lpGroup->nxyzSize.z   = (nxyzBoundMax.z - nxyzBoundMin.z) /
			lpGroup->nxyzDivideReCalc.z;

		/* X のサイズ設定 */
		if((lpGroup->nxyzSize.x = GetSizeCount(nxyzBoundMax.x, nxyzBoundMin.x,
			lpGroup->nxyzDivideReCalc.x, lpGroup->nxyzSize.x)) == -1)
			return 0;
		
		/* Y のサイズ設定 */
		if((lpGroup->nxyzSize.y = GetSizeCount(nxyzBoundMax.y, nxyzBoundMin.y,
			lpGroup->nxyzDivideReCalc.y, lpGroup->nxyzSize.y)) == -1)
			return 0;

		/* Z のサイズ設定 */
		if((lpGroup->nxyzSize.z = GetSizeCount(nxyzBoundMax.z, nxyzBoundMin.z,
			lpGroup->nxyzDivideReCalc.z, lpGroup->nxyzSize.z)) == -1)
			return 0;

		/* X の分割数設定 */
		if((lpGroup->nxyzDivideReCalc.x = GetDivideCount(nxyzBoundMax.x,
			nxyzBoundMin.x, lpGroup->nxyzDivideReCalc.x, lpGroup->nxyzSize.x)) == -1)
			return 0;
		
		/* Y の分割数設定 */
		if((lpGroup->nxyzDivideReCalc.y = GetDivideCount(nxyzBoundMax.y,
			nxyzBoundMin.y, lpGroup->nxyzDivideReCalc.y, lpGroup->nxyzSize.y)) == -1)
			return 0;

		/* Z の分割数設定 */
		if((lpGroup->nxyzDivideReCalc.z = GetDivideCount(nxyzBoundMax.z,
			nxyzBoundMin.z, lpGroup->nxyzDivideReCalc.z, lpGroup->nxyzSize.z)) == -1)
			return 0;

		/* 分割数が確定したのでグループの各メンバを設定 */
		lpGroup->nxyzBoundMin = nxyzBoundMin;

		lpGroup->nBlockCount = lpGroup->nxyzDivideReCalc.x * 
			lpGroup->nxyzDivideReCalc.y * lpGroup->nxyzDivideReCalc.z;
			
		/* ブロックの作成とオブジェクトの分割を行う */
		if(DivideObject(lpGroup) == 0)
			return 0;

		/* グループの壁・床・トラップを削除する */
		if(lpGroup->lpHazard != NULL)
		{
			freeEx(lpGroup->lpHazard);
			lpGroup->lpHazard     = NULL;
			lpGroup->nHazardCount = 0;
		}
		
		if(lpGroup->lpFloor != NULL)
		{
			freeEx(lpGroup->lpFloor);
			lpGroup->lpFloor     = NULL;
			lpGroup->nFloorCount = 0;
		}

		if(lpGroup->lpTrap != NULL)
		{
			freeEx(lpGroup->lpTrap);
			lpGroup->lpTrap     = NULL;
			lpGroup->nTrapCount = 0;
		}

	}

	/* 進行状況出力 */
	printf("..........Done.\n");

	return 1;
}

/******************************************************************************
 * static
 */

/******************************************************************************
 * セグメントオブジェクトのバウンディングを返す
 */
static void GetBoundingSegment(	/* 0 以外 正常 */
								/* 0 異常 */
	LPSEGMENTOBJECT lpSegmentObject,	/* セグメントオブジェクト */
	int             nSegmentCount,		/* セグメント数 */
	LPDOUBLEXYZ     lpdxyzBoundMax,		/* バウンディングを格納・最大 */
	LPDOUBLEXYZ     lpdxyzBoundMin)		/* バウンディングを格納・最小 */
{
	int             i, j;
	LPSEGMENTOBJECT lpSegment;


	for(i = 0; i < nSegmentCount; i++)
	{
		lpSegment = lpSegmentObject + i;

		for(j = 0; j < lpSegment->nVertexCount; j++)
		{
			lpdxyzBoundMax->x = __max(lpdxyzBoundMax->x,
				(lpSegment->dxyzVertex + j)->x);
			lpdxyzBoundMax->y = __max(lpdxyzBoundMax->y,
				(lpSegment->dxyzVertex + j)->y);
			lpdxyzBoundMax->z = __max(lpdxyzBoundMax->z,
				(lpSegment->dxyzVertex + j)->z);

			lpdxyzBoundMin->x = __min(lpdxyzBoundMin->x,
				(lpSegment->dxyzVertex + j)->x);
			lpdxyzBoundMin->y = __min(lpdxyzBoundMin->y,
				(lpSegment->dxyzVertex + j)->y);
			lpdxyzBoundMin->z = __min(lpdxyzBoundMin->z,
				(lpSegment->dxyzVertex + j)->z);
		}
	}
}

/******************************************************************************
 * トラップオブジェクトのバウンディングを返す
 */
static void GetBoundingTrap(	/* 0 以外 正常 */
								/* 0 異常 */
	LPTRAPOBJECT lpTrapObject,		/* トラップオブジェクト */
	int          nTrapCount,		/* トラップ数 */
	LPDOUBLEXYZ  lpdxyzBoundMax,	/* バウンディングを格納・最大 */
	LPDOUBLEXYZ  lpdxyzBoundMin)	/* バウンディングを格納・最小 */
{
	int             i;
	LPTRAPOBJECT lpTrap;

	for(i = 0; i < nTrapCount; i++)
	{
		lpTrap = lpTrapObject + i;

		lpdxyzBoundMax->x = __max(lpdxyzBoundMax->x, lpTrap->dxyzBound[1].x);
		lpdxyzBoundMax->y = __max(lpdxyzBoundMax->y, lpTrap->dxyzBound[1].y);
		lpdxyzBoundMax->z = __max(lpdxyzBoundMax->z, lpTrap->dxyzBound[1].z);

		lpdxyzBoundMin->x = __min(lpdxyzBoundMin->x, lpTrap->dxyzBound[0].x);
		lpdxyzBoundMin->y = __min(lpdxyzBoundMin->y, lpTrap->dxyzBound[0].y);
		lpdxyzBoundMin->z = __min(lpdxyzBoundMin->z, lpTrap->dxyzBound[0].z);
	}
}

/******************************************************************************
 * 分割数を返す
 */
static int GetDivideCount(	/* 分割数を返す */
							/* -1 はエラー( 0 除算) */
	int nMax,		/* バウンディング最大値 */
	int nMin,		/* バウンディング最小値 */
	int nDivide,	/* 元になる分割数 */
	int nSize)		/* 基準になるサイズ */
{
	int nWidth;

	/* 0 による除算回避 */
	if(nDivide == 0)
	{
		printf("Error! 0 divide.");
		return -1;
	}

	/* 幅が負数になるのをさける */
	if(nMax < nMin)
	{
		printf("Error! GetDivideCount BoundMax < BoundMin");
		return -1;
	}

	nWidth = nMax - nMin;

	while(1)
	{
		if((nSize * nDivide) >= nWidth)
			break;

		nDivide++;
	}

	return nDivide;
}


/******************************************************************************
 * サイズを返す
 */
static int GetSizeCount(	/* サイズを返す */
							/* -1 はエラー( 0 除算) */
	int nMax,		/* バウンディング最大値 */
	int nMin,		/* バウンディング最小値 */
	int nDivide,	/* 元になる分割数 */
	int nSize)		/* 基準になるサイズ」 */
{
	int nWidth;

	/* 0 による無限ループ回避 */
	if(nDivide == 0)
	{
		printf("Error! 0 divide.");
		return -1;
	}

	/* 0 による無限ループ回避 */
/*	if(nSize == 0)
	{
		printf("Error! 0 Size.");
		return -1;
	}
*/
//printf("size = %d\n", nSize);

	/* 幅が負数になるのをさける */
	if(nMax < nMin)
	{
		printf("Error! Function \"GetSizeCount\" BoundMax < BoundMin");
		return -1;
	}

	nWidth = nMax - nMin;

	while(1)
	{
		if(nSize > 16000)
			return 16000;

		if((nSize <= 16000) && ((nSize * nDivide) >= nWidth))
			break;

		nSize += 10;
	}

	return nSize;
}

/******************************************************************************
 * ブロックバウンディングを得る
 */
static void GetBlockBound(
	LPGROUPOBJECT lpGroup,			/* グループオブジェクト */
	int           nIndexX,			/* X のインデックス */
	int           nIndexY,			/* Y のインデックス */
	int           nIndexZ,			/* Z のインデックス */
	LPINTXYZ      lpnxyzBoundMax,	/* ブロックバウンディングの最大値 */
	LPINTXYZ      lpnxyzBoundMin)	/* ブロックバウンディングの最小値 */
{
	/* 最小値の設定 */
	lpnxyzBoundMin->x = lpGroup->nxyzBoundMin.x + (lpGroup->nxyzSize.x * nIndexX);
	lpnxyzBoundMin->y = lpGroup->nxyzBoundMin.y + (lpGroup->nxyzSize.y * nIndexY);
	lpnxyzBoundMin->z = lpGroup->nxyzBoundMin.z + (lpGroup->nxyzSize.z * nIndexZ);

	/* 最大値の設定 */
	lpnxyzBoundMax->x = lpnxyzBoundMin->x + lpGroup->nxyzSize.x;
	lpnxyzBoundMax->y = lpnxyzBoundMin->y + lpGroup->nxyzSize.y;
	lpnxyzBoundMax->z = lpnxyzBoundMin->z + lpGroup->nxyzSize.z;
}

/******************************************************************************
 * ハザードとブロックを判定
 */
static int DivideHazard(	/* 0 以外。正常 */
							/* 0 異常 */
	LPBLOCKOBJECT   lpBlock,			/* ブロックオブジェクト */
	LPINTXYZ        lpnxyzBoundMax,		/* ブロックのバウンディング最大値 */
	LPINTXYZ        lpnxyzBoundMin,		/* ブロックのバウンディング最小値 */
	LPSEGMENTOBJECT lpHazardObject,		/* ハザードオブジェクト */
	int             nHazardCount,		/* ハザードオブジェクトの数 */
	LPINTXYZ        lpnxyzDivide,		/* 分割数 */
	int             nIndexX,			/* X のインデックス */
	int             nIndexY,			/* Y のインデックス */
	int             nIndexZ)			/* Z のインデックス */
{
	int             i, j;
	DOUBLEXYZ       dxyzBoundHazardMax;
	DOUBLEXYZ       dxyzBoundHazardMin;
	LPSEGMENTOBJECT lpHazard;
	LPSEGMENTOBJECT lpHazardNew;

	for(i = 0; i < nHazardCount; i++)
	{
		lpHazard = lpHazardObject + i;

		dxyzBoundHazardMax.x = -HUGE_VAL;
		dxyzBoundHazardMax.y = -HUGE_VAL;
		dxyzBoundHazardMax.z = -HUGE_VAL;
		dxyzBoundHazardMin.x = HUGE_VAL;
		dxyzBoundHazardMin.y = HUGE_VAL;
		dxyzBoundHazardMin.z = HUGE_VAL;

		/* ハザードのバウンディングを得る */
		for(j = 0; j < lpHazard->nVertexCount; j++)
		{
			dxyzBoundHazardMax.x = __max(dxyzBoundHazardMax.x,
				lpHazard->dxyzVertex[j].x);
			dxyzBoundHazardMax.y = __max(dxyzBoundHazardMax.y,
				lpHazard->dxyzVertex[j].y);
			dxyzBoundHazardMax.z = __max(dxyzBoundHazardMax.z,
				lpHazard->dxyzVertex[j].z);
			dxyzBoundHazardMin.x = __min(dxyzBoundHazardMin.x,
				lpHazard->dxyzVertex[j].x);
			dxyzBoundHazardMin.y = __min(dxyzBoundHazardMin.y,
				lpHazard->dxyzVertex[j].y);
			dxyzBoundHazardMin.z = __min(dxyzBoundHazardMin.z,
				lpHazard->dxyzVertex[j].z);
		}

		/* バウンディングを判定する */
		/* X を判定 */
		if(nIndexX == (lpnxyzDivide->x - 1))
		{
			/* 最右端である。値の大きい方の境界線上も含む */
			if((dxyzBoundHazardMax.x < lpnxyzBoundMin->x) ||
				(lpnxyzBoundMax->x < dxyzBoundHazardMin.x))
				continue;
		}
		else
		{
			/* 最右端ではない。値の大きい方の境界線上は含まない */
			if((dxyzBoundHazardMax.x < lpnxyzBoundMin->x) ||
				(lpnxyzBoundMax->x <= dxyzBoundHazardMin.x))
				continue;
		}

		/* Y を判定 */
		if(nIndexY == (lpnxyzDivide->y - 1))
		{
			/* 最上端である。値の大きい方の境界線上も含む */
			if((dxyzBoundHazardMax.y < lpnxyzBoundMin->y) ||
				(lpnxyzBoundMax->y < dxyzBoundHazardMin.y))
				continue;
		}
		else
		{
			/* 最上端ではない。値の大きい方の境界線上は含まない */
			if((dxyzBoundHazardMax.y < lpnxyzBoundMin->y) ||
				(lpnxyzBoundMax->y <= dxyzBoundHazardMin.y))
				continue;
		}

		/* Z を判定 */
		if(nIndexZ == (lpnxyzDivide->z - 1))
		{
			/* 一番手前である。値の大きい方の境界線上も含む */
			if((dxyzBoundHazardMax.z < lpnxyzBoundMin->z) ||
				(lpnxyzBoundMax->z < dxyzBoundHazardMin.z))
				continue;
		}
		else
		{
			/* 一番手前ではない。値の大きい方の境界線上は含まない */
			if((dxyzBoundHazardMax.z < lpnxyzBoundMin->z) ||
				(lpnxyzBoundMax->z <= dxyzBoundHazardMin.z))
				continue;
		}

		/* ここまで来たのならそれは組み込む */
		/* ハザードオブジェクトの領域確保 */
		if((lpHazardNew = (LPSEGMENTOBJECT)reallocEx(
			lpBlock->lpHazard, (sizeof(SEGMENTOBJECT) * 
			(lpBlock->nHazardCount + 1)))) == NULL)
			return 0;

		lpBlock->lpHazard = lpHazardNew;
		lpHazardNew += lpBlock->nHazardCount;
		lpBlock->nHazardCount++;
		memset(lpHazardNew, 0x00, sizeof(SEGMENTOBJECT));
				
		*(lpHazardNew) = *(lpHazard);
	}
	return 1;
}

/******************************************************************************
 * フロアとブロックを判定
 */
static int DivideFloor(	/* 0 以外。正常 */
							/* 0 異常 */
	LPBLOCKOBJECT   lpBlock,			/* ブロックオブジェクト */
	LPINTXYZ        lpnxyzBoundMax,		/* ブロックのバウンディング最大値 */
	LPINTXYZ        lpnxyzBoundMin,		/* ブロックのバウンディング最小値 */
	LPSEGMENTOBJECT lpFloorObject,		/* フロアオブジェクト */
	int             nFloorCount,		/* フロアオブジェクトの数 */
	LPINTXYZ        lpnxyzDivide,		/* 分割数 */
	int             nIndexX,			/* X のインデックス */
	int             nIndexY,			/* Y のインデックス */
	int             nIndexZ)			/* Z のインデックス */
{
	int             i, j;
	DOUBLEXYZ       dxyzBoundFloorMax;
	DOUBLEXYZ       dxyzBoundFloorMin;
	LPSEGMENTOBJECT lpFloor;
	LPSEGMENTOBJECT lpFloorNew;

	for(i = 0; i < nFloorCount; i++)
	{
		lpFloor = lpFloorObject + i;

		dxyzBoundFloorMax.x = -HUGE_VAL;
		dxyzBoundFloorMax.y = -HUGE_VAL;
		dxyzBoundFloorMax.z = -HUGE_VAL;
		dxyzBoundFloorMin.x = HUGE_VAL;
		dxyzBoundFloorMin.y = HUGE_VAL;
		dxyzBoundFloorMin.z = HUGE_VAL;

		/* ハザードのバウンディングを得る */
		for(j = 0; j < lpFloor->nVertexCount; j++)
		{
			dxyzBoundFloorMax.x = __max(dxyzBoundFloorMax.x,
				lpFloor->dxyzVertex[j].x);
			dxyzBoundFloorMax.y = __max(dxyzBoundFloorMax.y,
				lpFloor->dxyzVertex[j].y);
			dxyzBoundFloorMax.z = __max(dxyzBoundFloorMax.z,
				lpFloor->dxyzVertex[j].z);
			dxyzBoundFloorMin.x = __min(dxyzBoundFloorMin.x,
				lpFloor->dxyzVertex[j].x);
			dxyzBoundFloorMin.y = __min(dxyzBoundFloorMin.y,
				lpFloor->dxyzVertex[j].y);
			dxyzBoundFloorMin.z = __min(dxyzBoundFloorMin.z,
				lpFloor->dxyzVertex[j].z);
		}

		/* バウンディングを判定する */
		/* X を判定 */
		if(nIndexX == (lpnxyzDivide->x -1))
		{
			/* 最右端である。値の大きい方の境界線上も含む */
			if((dxyzBoundFloorMax.x < lpnxyzBoundMin->x) ||
				(lpnxyzBoundMax->x < dxyzBoundFloorMin.x))
				continue;
		}
		else
		{
			/* 最右端ではない。値の大きい方の境界線上は含まない */
			if((dxyzBoundFloorMax.x < lpnxyzBoundMin->x) ||
				(lpnxyzBoundMax->x <= dxyzBoundFloorMin.x))
				continue;
		}

		/* Y を判定 */
		if(nIndexY == (lpnxyzDivide->y -1))
		{
			/* 最上端である。値の大きい方の境界線上も含む */
			if((dxyzBoundFloorMax.y < lpnxyzBoundMin->y) ||
				(lpnxyzBoundMax->y < dxyzBoundFloorMin.y))
				continue;
		}
		else
		{
			/* 最上端ではない。値の大きい方の境界線上は含まない */
			if((dxyzBoundFloorMax.y < lpnxyzBoundMin->y) ||
				(lpnxyzBoundMax->y <= dxyzBoundFloorMin.y))
				continue;
		}

		/* Z を判定 */
		if(nIndexZ == (lpnxyzDivide->z -1))
		{
			/* 一番手前である。値の大きい方の境界線上も含む */
			if((dxyzBoundFloorMax.z < lpnxyzBoundMin->z) ||
				(lpnxyzBoundMax->z < dxyzBoundFloorMin.z))
				continue;
		}
		else
		{
			/* 一番手前ではない。値の大きい方の境界線上は含まない */
			if((dxyzBoundFloorMax.z < lpnxyzBoundMin->z) ||
				(lpnxyzBoundMax->z <= dxyzBoundFloorMin.z))
				continue;
		}

		/* ここまで来たのならそれは組み込む */
		/* ハザードオブジェクトの領域確保 */
		if((lpFloorNew = (LPSEGMENTOBJECT)reallocEx(
			lpBlock->lpFloor, (sizeof(SEGMENTOBJECT) * 
			(lpBlock->nFloorCount + 1)))) == NULL)
			return 0;

		lpBlock->lpFloor = lpFloorNew;
		lpFloorNew += lpBlock->nFloorCount;
		lpBlock->nFloorCount++;
		memset(lpFloorNew, 0x00, sizeof(SEGMENTOBJECT));
				
		*(lpFloorNew) = *(lpFloor);
	}
	return 1;
}

/******************************************************************************
 * トラップとブロックを判定
 */
static int DivideTrap(	/* 0 以外。正常 */
							/* 0 異常 */
	LPBLOCKOBJECT lpBlock,			/* ブロックオブジェクト */
	LPINTXYZ      lpnxyzBoundMax,	/* ブロックのバウンディング最大値 */
	LPINTXYZ      lpnxyzBoundMin,	/* ブロックのバウンディング最小値 */
	LPTRAPOBJECT  lpTrapObject,		/* トラップオブジェクト */
	int           nTrapCount,		/* トラップオブジェクトの数 */
	LPINTXYZ      lpnxyzDivide,		/* 分割数 */
	int           nIndexX,			/* X のインデックス */
	int           nIndexY,			/* Y のインデックス */
	int           nIndexZ)			/* Z のインデックス */
{
	int          i;
	LPTRAPOBJECT lpTrap;
	LPTRAPOBJECT lpTrapNew;

	for(i = 0; i < nTrapCount; i++)
	{
		lpTrap = lpTrapObject + i;

		/* バウンディングを判定する */
		/* X を判定 */
		if(nIndexX == (lpnxyzDivide->x - 1))
		{
			/* 最右端である。値の大きい方の境界線上も含む */
			if((lpTrap->dxyzBound[1].x < lpnxyzBoundMin->x) ||
				(lpnxyzBoundMax->x < lpTrap->dxyzBound[0].x))
				continue;
		}
		else
		{
			/* 最右端ではない。値の大きい方の境界線上は含まない */
			if((lpTrap->dxyzBound[1].x < lpnxyzBoundMin->x) ||
				(lpnxyzBoundMax->x <= lpTrap->dxyzBound[0].x))
				continue;
		}

		/* Y を判定 */
		if(nIndexY == (lpnxyzDivide->y - 1))
		{
			/* 最上端である。値の大きい方の境界線上も含む */
			if((lpTrap->dxyzBound[1].y < lpnxyzBoundMin->y) ||
				(lpnxyzBoundMax->y < lpTrap->dxyzBound[0].y))
				continue;
		}
		else
		{
			/* 最上端ではない。値の大きい方の境界線上は含まない */
			if((lpTrap->dxyzBound[1].y < lpnxyzBoundMin->y) ||
				(lpnxyzBoundMax->y <= lpTrap->dxyzBound[0].y))
				continue;
		}

		/* Z を判定 */
		if(nIndexZ == (lpnxyzDivide->z - 1))
		{
			/* 一番手前である。値の大きい方の境界線上も含む */
			if((lpTrap->dxyzBound[1].z < lpnxyzBoundMin->z) ||
				(lpnxyzBoundMax->z < lpTrap->dxyzBound[0].z))
				continue;
		}
		else
		{
			/* 一番手前ではない。値の大きい方の境界線上は含まない */
			if((lpTrap->dxyzBound[1].z < lpnxyzBoundMin->z) ||
				(lpnxyzBoundMax->z <= lpTrap->dxyzBound[0].z))
				continue;
		}

		/* ここまで来たのならそれは組み込む */
		/* ハザードオブジェクトの領域確保 */
		if((lpTrapNew = (LPTRAPOBJECT)reallocEx(lpBlock->lpTrap,
			(sizeof(TRAPOBJECT) * (lpBlock->nTrapCount + 1)))) == NULL)
			return 0;

		lpBlock->lpTrap = lpTrapNew;
		lpTrapNew += lpBlock->nTrapCount;
		lpBlock->nTrapCount++;
		memset(lpTrapNew, 0x00, sizeof(TRAPOBJECT));
				
		*(lpTrapNew) = *(lpTrap);
	}
	return 1;
}

/******************************************************************************
 * ブロックオブジェクトの作成とオブジェクトの分割
 */
static int DivideObject(	/* 0 以外。正常 */
						/* 0 異常 */
	LPGROUPOBJECT lpGroup)
{
	int           i, j, k;
	int           nBlockCount;
	int           nDivideX;
	int           nDivideY;
	int           nDivideZ;
	INTXYZ        nxyzBoundMax;
	INTXYZ        nxyzBoundMin;
	LPBLOCKOBJECT lpBlock;

	/* ブロック数：カウント用 */
	nBlockCount = 0;

	/* 分割数を確保 */
	nDivideX = lpGroup->nxyzDivideReCalc.x;
	nDivideY = lpGroup->nxyzDivideReCalc.y;
	nDivideZ = lpGroup->nxyzDivideReCalc.z;

	for(i = 0; i < nDivideY; i++)	/* Y のインデックス */
	{
		for(j = 0; j < nDivideZ; j++)	/* Z のインデックス */
		{
			for(k = 0; k < nDivideX; k++)	/* X のインデックス */
			{
				/* ブロックオブジェクトの領域確保 */
				if((lpBlock = (LPBLOCKOBJECT)reallocEx(
					lpGroup->lpBlock, (sizeof(BLOCKOBJECT) * 
					(nBlockCount + 1)))) == NULL)
					return 0;

				lpGroup->lpBlock = lpBlock;
				lpBlock += nBlockCount;
				nBlockCount++;
				memset(lpBlock, 0x00, sizeof(BLOCKOBJECT));
				
				/* ブロックバウンディングを得る */
				GetBlockBound(lpGroup, k, i, j, &nxyzBoundMax, &nxyzBoundMin);

				/* ブロック中心を設定 */
				lpBlock->nxyzCenter.x = (nxyzBoundMax.x + nxyzBoundMin.x) / 2;
				lpBlock->nxyzCenter.y = (nxyzBoundMax.y + nxyzBoundMin.y) / 2;
				lpBlock->nxyzCenter.z = (nxyzBoundMax.z + nxyzBoundMin.z) / 2;

				/* 拡張ブロックは未使用 */
				lpBlock->nExtension = -1;

				/* ブロック番号 */
				lpBlock->nBlockNumber = (nDivideX * nDivideZ * i) +
					(nDivideX * j) + k;

				/* 壁とブロックを判定する */
				if(DivideHazard(lpBlock, &nxyzBoundMax, &nxyzBoundMin,
					lpGroup->lpHazard, lpGroup->nHazardCount,
					&(lpGroup->nxyzDivideReCalc), k, i, j) == 0)
					return 0;
				
				/* 床とブロックを判定する */
				if(DivideFloor(lpBlock, &nxyzBoundMax, &nxyzBoundMin,
					lpGroup->lpFloor, lpGroup->nFloorCount,
					&(lpGroup->nxyzDivideReCalc), k, i, j) == 0)
					return 0;
			
				/* トラップとブロックを判定する */
				if(DivideTrap(lpBlock, &nxyzBoundMax, &nxyzBoundMin,
					lpGroup->lpTrap, lpGroup->nTrapCount,
					&(lpGroup->nxyzDivideReCalc), k, i, j) == 0)
					return 0;
			}
		}
	}

	if(nBlockCount != lpGroup->nBlockCount)
	{
		printf("Error! Bad Block Object.");
		return 0;
	}

	return 1;
}

/******************************************************************************
 * 値が大きくなるように切り捨てる
 */
static int  IntDropUp(
	double dParam)
{
	int nDropParam;

	nDropParam = (int)ceil( dParam );
	return nDropParam ;
#if 0
	/* 引数よりも切り捨てた値の方が小さいならば１加えて返す */
	if ( nDropParam < dParam )
		return nDropParam + 1;
	else
		return nDropParam;
#endif
}

/******************************************************************************
 * 値が小さくなるように切り捨てる
 */
static int  IntDropDown(
	double dParam)
{
	int nDropParam;

	nDropParam = (int)floor( dParam );
	return nDropParam ;
#if 0
	/* 引数よりも切り捨てた値の方が小さいならば１加えて返す */
	if ( nDropParam > dParam )
		return nDropParam;
	else
		return nDropParam - 1;
#endif
}

