///////////////////////////////////////////////////////////////////////////////
// System	: Object Exchange Service for Windows
// Computer : DOS/V
// OS		: Microsoft Windows
// Compiler : Microsoft Visual C++
// Module	: Math Public 

///////////////////////////////////////////////////////////////////////////////
// 汎用演算関数群

///////////////////////////////////////////////////////////////////////////////
// included

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

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */

/******************************************************************************
 * 同一頂点かチェックする
 */
Dllexport BOOL WINAPI OESCheckSameVertex(	/* TRUE  : 同一頂点 */
											/* FALSE : 異なる頂点 */
	LPP3DXYZ lpP3DXYZ1,		/* 比較する頂点１ */
	LPP3DXYZ lpP3DXYZ2,		/* 比較する頂点２ */
	double   dCalcError)	/* 誤差 */
{
	if((lpP3DXYZ1 == NULL) || (lpP3DXYZ2 == NULL))
		return FALSE;

	return (((__abs(lpP3DXYZ1->x - lpP3DXYZ2->x) < dCalcError)
		  && (__abs(lpP3DXYZ1->y - lpP3DXYZ2->y) < dCalcError)
		  && (__abs(lpP3DXYZ1->z - lpP3DXYZ2->z) < dCalcError)) == TRUE);
}

/******************************************************************************
 * 頂点配列から任意の点を削除する。ただし、配列内の頂点を前詰めするだけで、
 * 領域の再確保は行わないので、返り値 + 1 にもデータが残っていることに注意
 */
Dllexport int WINAPI OESDeleteVertex(	/* 変更後の頂点数を返す（１減るだけだが）*/
										/* 負数はエラー (頂点配列が NULL か頂点数が 0 */
	LPP3DXYZ lpP3DXYZVertex,	/* 削除する頂点を持つリスト */
	int      nVertexCount,		/* リストの全頂点数 */
	int      nDeleteVertexIndex)	/* 削除する頂点のインデックス */
{
	int i;

	/* 頂点配列が NULL か、頂点数が 0 ならばエラー */
	if((lpP3DXYZVertex == NULL) || (nVertexCount == 0))
		return -1;

	if(nDeleteVertexIndex == (nVertexCount - 1))
		return nVertexCount - 1;
	else
	{
		/* 前に詰める */
		for(i = nDeleteVertexIndex + 1; i < nVertexCount; i++)
			*(lpP3DXYZVertex + i - 1) = *(lpP3DXYZVertex + i);
	}

	return nVertexCount - 1;
}

/******************************************************************************
 * 渡された２頂点が構成するベクトルを返す。ベクトルは第二頂点から第一頂点に
 * 引かれる。
 */
Dllexport P3DXYZ WINAPI OESGetVector(	/* ベクトルを返す */
	LPP3DXYZ lpP3DXYZVertex1,		/* 第一頂点 */
	LPP3DXYZ lpP3DXYZVertex2)		/* 第二頂点 */
{
	P3DXYZ xyzVector;

	/* 最初の頂点と２番目の頂点が形成するベクトル */
	xyzVector.x = lpP3DXYZVertex2->x - lpP3DXYZVertex1->x;
	xyzVector.y = lpP3DXYZVertex2->y - lpP3DXYZVertex1->y;
	xyzVector.z = lpP3DXYZVertex2->z - lpP3DXYZVertex1->z;
	
	return xyzVector;
}

/******************************************************************************
 * 同一直線上にある複数の頂点を２頂点に減らします。引数の頂点配列を新しい頂点配列
 * として構成し直すので注意してください
 */
Dllexport int WINAPI OESDeleteOverVertexOnLine(	/* 0 以上 新しい頂点数 */
											/* 負数  ２頂点以下、配列が NULL */
	LPP3DXYZ lpP3DXYZVertex,	/* 調べる頂点配列 */
	int      nVertexCount,		/* 頂点数 */
	double   dErrorRenge)		/* 誤差範囲 ０～１の範囲 ０に近いほど厳密な判定になる */
{
	int    i;
	int    nCount;
	int    nSecond, nThird;
	double dCalcError;
	P3DXYZ xyzVector1, xyzVector2;

	/* ２頂点以下、頂点配列が NULL ならエラー */
	if((nVertexCount < 3) || (lpP3DXYZVertex == NULL))
		return -1;

	nCount = nVertexCount;
	dCalcError = 1 - dErrorRenge;
	
	for(i = 0; i < (nCount - 1); i++)
	{
		// i は最初の点。２番目、３番目の点のインデックスを決定
		if(i == (nCount - 1))
		{
			nSecond = 0;
			nThird  = 1;
		}
		else if(i == (nCount - 2))
		{
			nSecond = i + 1;
			nThird  = 0;
		}
		else
		{
			nSecond = i + 1;
			nThird  = i + 2;
		}

		/* 3 頂点からベクトルを形成 */
		xyzVector1 = OESGetVector((lpP3DXYZVertex + i), (lpP3DXYZVertex + nSecond));
		xyzVector2 = OESGetVector((lpP3DXYZVertex + nSecond), (lpP3DXYZVertex + nThird));

		/* ３つの頂点が同一直線上にある */
		if(P3DVectorCrossCos(&xyzVector1, &xyzVector2) >= dCalcError)
		{
			nCount = OESDeleteVertex(lpP3DXYZVertex, nCount, nSecond);
			break;
		}
	}

	if(nCount != nVertexCount)
		nCount = OESDeleteOverVertexOnLine(lpP3DXYZVertex, nCount, dErrorRenge);

	return nCount;
}


/******************************************************************************
 * オブジェクトに含まれるポリゴンが線ポリゴンのみであるとして、その接続順頂点インデックス配列を返す
 * 戻り値の int ポインタは freeEx 関数で開放して下さい。
 */
Dllexport BOOL WINAPI OESP3DPolygonLineVertex(	/* 0以外	接続順解析成功 */
												/* 0		接続順解析不能 */
	int**      lppnIndex,		/* 接続順頂点インデックス配列 */
	int*       lpnIndexCount,	/* 接続順頂点インデックス配列数 */
	HP3DOBJECT hP3DObject)		/* オブジェクト */
{
	int              i, j, j2;
	int              nNext;
	int              nVertexNext;
	int*             lpnIndex;
	int*             lpnIndex2;
	BOOL             bScarch;
	LPP3DVERTEX      lpP3DVertex;
	LPP3DPOLYGON     lpP3DPolygon;
	LPP3DPOLYGONNODE lpP3DPolygonNode;
	LPP3DPOLYGONNODE lpP3DPolygonNode2;
	LPP3DPOLYGONNODE lpP3DPolygonNode3;
	LPP3DPOLYGONNODE lpP3DPolygonNode4;

	if(P3DObject(hP3DObject)->nVertexCount <= 0)
		return 0;

	/* 頂点順序を格納する配列 */
	if((*lppnIndex = (int*)callocEx(1, sizeof(int))) == NULL)
		return 0;

	if(P3DObject(hP3DObject)->nVertexCount == 1)
	{
		/* 一頂点のみ */
		*(*lppnIndex + 0) = 0;
		*lpnIndexCount = 1;
		return 1;
	}

	/* 一番始めのポリゴンのポリゴンノード */
	for(i = 0, lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon;
		i < P3DObject(hP3DObject)->nPolygonCount; i++, lpP3DPolygon++)
	{
		for(j = 0, lpP3DPolygonNode = lpP3DPolygon->lpP3DPolygonNode;
			(j < lpP3DPolygon->nPolygonNodeCount)
			&& (lpP3DPolygonNode->nVertex == -1); j++, lpP3DPolygonNode++);
		if(j < lpP3DPolygon->nPolygonNodeCount)
			break;
	}
	/* 頂点を使用しているポリゴンが存在しない */
	if(i >= P3DObject(hP3DObject)->nPolygonCount)
	{
		freeEx(*lppnIndex);
		return 0;
	}

	/* 指定されたポリゴンノードの次のポリゴンノードインデックス */
	if((nNext = (lpP3DPolygonNode - lpP3DPolygon->lpP3DPolygonNode + 1))
		>= lpP3DPolygon->nPolygonNodeCount)
		nNext = 0;
	/* 接続順の第二頂点インデックス */
	lpP3DPolygonNode2 = lpP3DPolygon->lpP3DPolygonNode + nNext;
	nVertexNext = lpP3DPolygonNode2->nVertex;

	/* 第一頂点を使用し、第二頂点を使用していないポリゴンノード */
	for(i = P3DObject(hP3DObject)->nPolygonCount,
		lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon
			+ (P3DObject(hP3DObject)->nPolygonCount - 1);
		i > 0; i--, lpP3DPolygon--)
	{
		for(j = lpP3DPolygon->nPolygonNodeCount,
			lpP3DPolygonNode3 = lpP3DPolygon->lpP3DPolygonNode
				+ (lpP3DPolygon->nPolygonNodeCount - 1),
			lpP3DPolygonNode4 = lpP3DPolygon->lpP3DPolygonNode; (j > 0)
			&& !(((lpP3DPolygonNode3->nVertex != nVertexNext)
			   && (lpP3DPolygonNode4->nVertex == lpP3DPolygonNode->nVertex))
			  || ((lpP3DPolygonNode3->nVertex == lpP3DPolygonNode->nVertex)
			   && (lpP3DPolygonNode4->nVertex != nVertexNext)));
			j--, lpP3DPolygonNode4 = lpP3DPolygonNode3, lpP3DPolygonNode3--);
		if(j > 0)
			break;
	}

	/* 始点頂点インデックス */
	if(i <= 0)
		nVertexNext = lpP3DPolygonNode->nVertex;
	else
	{
		/* 第一頂点から伸びるノードが2つある */
		/* 頂点補助情報を初期化 */
		for(i = 0, lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex;
			i < P3DObject(hP3DObject)->nVertexCount; i++, lpP3DVertex++)
			lpP3DVertex->wParam = FALSE;

		if(lpP3DPolygonNode3->nVertex == lpP3DPolygonNode->nVertex)
			nNext = lpP3DPolygonNode4->nVertex;
		else
			nNext = lpP3DPolygonNode3->nVertex;

		lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex
			+ lpP3DPolygonNode->nVertex;
		lpP3DVertex->wParam = TRUE;
		lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex + nNext;
		lpP3DVertex->wParam = TRUE;

		/* 後に発見したノードの最終頂点を求める */
		do
		{
			bScarch = FALSE;

			for(i = P3DObject(hP3DObject)->nPolygonCount,
				lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon
					+ (P3DObject(hP3DObject)->nPolygonCount - 1);
				i > 0; i--, lpP3DPolygon--)
			{
				for(j = lpP3DPolygon->nPolygonNodeCount,
					lpP3DPolygonNode3 = lpP3DPolygon->lpP3DPolygonNode
						+ (lpP3DPolygon->nPolygonNodeCount - 1),
					lpP3DPolygonNode4 = lpP3DPolygon->lpP3DPolygonNode;
					j > 0; j--,
					lpP3DPolygonNode4 = lpP3DPolygonNode3, lpP3DPolygonNode3--)
				{
					if(lpP3DPolygonNode3->nVertex == nNext)
						lpP3DPolygonNode2 = lpP3DPolygonNode4;
					else if(lpP3DPolygonNode4->nVertex == nNext)
						lpP3DPolygonNode2 = lpP3DPolygonNode3;
					else
						continue;

					lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex
						+ lpP3DPolygonNode2->nVertex;
					if(lpP3DVertex->wParam == TRUE)
						continue;

					lpP3DVertex->wParam = TRUE;
					nNext = lpP3DPolygonNode2->nVertex;

					bScarch = TRUE;
				}
			}
		}
		while(bScarch == TRUE);

		/* 接続順の第二頂点インデックスと後に発見したノードの最終頂点が等しいときはループしている */
		if(nNext == nVertexNext)
			nVertexNext = lpP3DPolygonNode->nVertex;
		else
			nVertexNext = nNext;
	}

	/* 頂点補助情報を初期化 */
	for(i = 0, lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex;
		i < P3DObject(hP3DObject)->nVertexCount; i++, lpP3DVertex++)
		lpP3DVertex->wParam = FALSE;

	/* 頂点順序を格納する配列 */
	*(*lppnIndex + 0) = nVertexNext;
	*lpnIndexCount = 1;

	lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex + nVertexNext;
	lpP3DVertex->wParam = TRUE;

	do
	{
		bScarch = FALSE;

		/* 頂点を使用している一番始めのポリゴンのポリゴンノード */
		for(i = 0, lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon;
			i < P3DObject(hP3DObject)->nPolygonCount; i++, lpP3DPolygon++)
		{
			for(j = 0, lpP3DPolygonNode3 = lpP3DPolygon->lpP3DPolygonNode,
				lpP3DPolygonNode4 = lpP3DPolygon->lpP3DPolygonNode
					+ (lpP3DPolygon->nPolygonNodeCount - 1);
				j < lpP3DPolygon->nPolygonNodeCount; j++,
				lpP3DPolygonNode4 = lpP3DPolygonNode3, lpP3DPolygonNode3++)
			{
				if(lpP3DPolygonNode3->nVertex == nVertexNext)
					lpP3DPolygonNode2 = lpP3DPolygonNode4;
				else if(lpP3DPolygonNode4->nVertex == nVertexNext)
					lpP3DPolygonNode2 = lpP3DPolygonNode3;
				else
					continue;

				lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex
					+ lpP3DPolygonNode2->nVertex;
				if(lpP3DVertex->wParam == TRUE)
					continue;

				/* 頂点順序を格納する配列 */
				if((lpnIndex = (int*)reallocEx(
					*lppnIndex, sizeof(int) * (*lpnIndexCount + 1))) == NULL)
				{
					freeEx(*lppnIndex);
					return 0;
				}

				lpP3DVertex->wParam = TRUE;
				nVertexNext = lpP3DPolygonNode2->nVertex;

				*lppnIndex = lpnIndex;
				 lpnIndex += *lpnIndexCount;
				 *lpnIndex = nVertexNext;
				(*lpnIndexCount)++;

				bScarch = TRUE;
			}
		}
	}
	while(bScarch == TRUE);

	/* 頂点数と配列数が一致しない */
	if(*lpnIndexCount != P3DObject(hP3DObject)->nVertexCount)
	{
		freeEx(*lppnIndex);
		return 0;
	}

	/* 使用していない頂点がある */
	for(i = 0, lpP3DVertex = P3DObject(hP3DObject)->lpP3DVertex;
		(i < P3DObject(hP3DObject)->nVertexCount)
		&& (lpP3DVertex->wParam == TRUE); i++, lpP3DVertex++);
	if(i < P3DObject(hP3DObject)->nVertexCount)
	{
		freeEx(*lppnIndex);
		return 0;
	}

	/* 使用していないポリゴンノードがある */
	for(i = 0, lpP3DPolygon = P3DObject(hP3DObject)->lpP3DPolygon;
		i < P3DObject(hP3DObject)->nPolygonCount; i++, lpP3DPolygon++)
	{
		for(j = 0, lpP3DPolygonNode3 = lpP3DPolygon->lpP3DPolygonNode,
			lpP3DPolygonNode4 = lpP3DPolygon->lpP3DPolygonNode
				+ (lpP3DPolygon->nPolygonNodeCount - 1);
			j < lpP3DPolygon->nPolygonNodeCount; j++,
			lpP3DPolygonNode4 = lpP3DPolygonNode3, lpP3DPolygonNode3++)
		{
			for(j2 = 0, lpnIndex = *lppnIndex,
				lpnIndex2 = *lppnIndex + (*lpnIndexCount - 1);
				(j2 < *lpnIndexCount)
				&& !(((lpP3DPolygonNode3->nVertex == *lpnIndex)
				   && (lpP3DPolygonNode4->nVertex == *lpnIndex2))
				  || ((lpP3DPolygonNode3->nVertex == *lpnIndex2)
				   && (lpP3DPolygonNode4->nVertex == *lpnIndex)));
				j2++, lpnIndex2 = lpnIndex, lpnIndex++);
			if(j2 >= *lpnIndexCount)
			{
				freeEx(*lppnIndex);
				return 0;
			}
		}
	}

	return 1;
}

/******************************************************************************
 * statics
 */

