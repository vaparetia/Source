/*******************************************************************************
 * outline - create.c
 * 生成、破棄
 * 2002/03/12 S.Yamashita
 * $Id: create.c,v 1.3 2002/04/30 08:41:53 usr03692 Exp $
 */

/*******************************************************************************
 * include
 */

#include <string.h>
#include <malloc.h>
#include <math.h>

#include "_outline.h"

/*******************************************************************************
 * functions
 */

static int MergeVertex(LPOUTLINE lpoutline, HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject);
static int CreateOutlineNest(LPOUTLINE lpoutline, HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, unsigned int* lpunCount, int nFlag);
static int CreateOutlineObject(LPOUTLINE lpoutline, HP3DMODEL hP3DModel, HP3DOBJECT hP3DObject, unsigned int* lpunCount, int nFlag);
static int GetObjColorFlag(LPOUTLINE lpoutline, char* lpszName, int nFlag);
static int CheckEdge(LPOUTLINE lpoutline, LPOL_OBJECT lpol_object, HP3DOBJECT hP3DObject, LPP3DPOLYGON lpPoly, int* lpnEdgeVert, int nMode1, int nFlag);
static int AddEdge(LPOUTLINE lpoutline, LPOL_OBJECT lpol_object, HP3DOBJECT hP3DObject, unsigned int unVertCount, int* lpnEdgeVert, int nFlag);
static int JointEdge(LPOUTLINE lpoutline, int nFlag);

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * アウトラインの生成
 */
int CreateOutline(	/* 1: 成功 */
					/* 0: 失敗 */
	LPOUTLINE     lpoutline,			/* アウトライン */
	HP3DMODEL     hP3DModel,			/* モデルハンドル */
	int           nFlagA,				/* フラグ（全ての関数に有効） */
	FILE*         fpLog,				/* ログファイルポインタ */
	unsigned int  unBlockSize,			/* 頂点メモリ確保ブロックサイズ(1～) */
	double        dAMargin,				/* ２つの面の法線の角度判定マージン(0～1)  0:０度の時のみ描画しない  1:全て描画しない */
	double        dPMargin,				/* ２つのエッジの重なり誤差判定マージン(0～) */
	unsigned int  unSkipNameCount,		/* 処理しない名前数 */
	char**        lppszSkipName,		/* 処理しない名前配列 */
	unsigned int* lpunColorNameCount,	/* 色グループ名前数 */
	char***       lpppszColorName,		/* 色グループ名前配列 */
	int           nFlag)				/* フラグ２（この関数用） */
{
	unsigned int unCount;
	unsigned int i, j;

	if(    (lpoutline == NULL)
		|| (hP3DModel == NULL)
		|| (unBlockSize < 1)
		|| ((dAMargin < 0) || (1 < dAMargin))
		|| (dPMargin < 0)
		|| ((unSkipNameCount > 0) && (lppszSkipName == NULL)))
	{
		return 0;
	}

	/* 初期化 */
	DestroyOutline(lpoutline, nFlagA | nFlag);

	lpoutline->nFlagA          = nFlagA & OL_MFLAG_MASK;
	lpoutline->fpLog           = fpLog;
	lpoutline->unBlockSize     = unBlockSize;
	lpoutline->dAMargin        = dAMargin;
	lpoutline->dPMargin        = dPMargin;
	lpoutline->unSkipNameCount = unSkipNameCount;
	if(unSkipNameCount > 0)
	{
		/* 処理しない名前配列 */
		if((lpoutline->lppszSkipName = (char**)calloc(unSkipNameCount, sizeof(char*))) == NULL)
		{
			DestroyOutline(lpoutline, nFlagA | nFlag);
			return 0;
		}
		/* 処理しない名前 */
		for(i = 0; i < unSkipNameCount; i++)
		{
			if((lpoutline->lppszSkipName[i] = (char*)calloc(OL_STRING_MAX, sizeof(char))) == NULL)
			{
				DestroyOutline(lpoutline, nFlagA | nFlag);
				return 0;
			}
			strncpy(lpoutline->lppszSkipName[i], lppszSkipName[i], OL_STRING_MAX - 1);
		}
	}
	if((lpunColorNameCount != NULL) && (lpppszColorName != NULL))
	{
		/* 色グループ名前配列の配列 */
		for(i = 0; i < 3; i++)
		{
			if(lpppszColorName[i] != NULL)
			{
				lpoutline->unColorNameCount[i] = lpunColorNameCount[i];

				/* 色グループ名前配列 */
				if((lpoutline->lppszColorName[i] = (char**)calloc(lpunColorNameCount[i], sizeof(char*))) == NULL)
				{
					DestroyOutline(lpoutline, nFlagA | nFlag);
					return 0;
				}
				/* 色グループ名前 */
				for(j = 0; j < lpunColorNameCount[i]; j++)
				{
					if((*(lpoutline->lppszColorName[i] + j) = (char*)calloc(OL_STRING_MAX, sizeof(char))) == NULL)
					{
						DestroyOutline(lpoutline, nFlagA | nFlag);
						return 0;
					}
					strncpy(*(lpoutline->lppszColorName[i] + j), *(lpppszColorName[i] + j), OL_STRING_MAX - 1);
				}
			}
		}
	}

	/* テキスト出力 */
	_OL_TextOutput(nFlagA | nFlag, fpLog, "CreateOutline\n");
	_OL_TextOutput(nFlagA | nFlag, fpLog, "\tFlagA           : %08x\n", nFlagA);
	_OL_TextOutput(nFlagA | nFlag, fpLog, "\tFlag            : %08x\n", nFlag);
	_OL_TextOutput(nFlagA | nFlag, fpLog, "\tLogFile         : %d\n"  , fpLog);
	_OL_TextOutput(nFlagA | nFlag, fpLog, "\tBlockSize       : %d\n"  , unBlockSize);
	_OL_TextOutput(nFlagA | nFlag, fpLog, "\tMargin1         : %lf\n" , dAMargin);
	_OL_TextOutput(nFlagA | nFlag, fpLog, "\tMargin2         : %lf\n" , dPMargin);
	_OL_TextOutput(nFlagA | nFlag, fpLog, "\tSkipNameCount   : %d\n"  , unSkipNameCount);
	for(i = 0; i < unSkipNameCount; i++)
		_OL_TextOutput(nFlagA | nFlag, fpLog, "\tSkipName%02d      : %s\n", i, lpoutline->lppszSkipName[i]);
	for(i = 0; i < 3; i++)
	{
		_OL_TextOutput(nFlagA | nFlag, fpLog, "\tColor%02dNameCount: %d\n", i + 2, lpoutline->unColorNameCount[i]);
		for(j = 0; j < lpoutline->unColorNameCount[i]; j++)
		{
			_OL_TextOutput(nFlagA | nFlag, fpLog, "\tColor%02dName%02d   : %s\n", i + 2, j, *(lpoutline->lppszColorName[i] + j));
		}
	}

	/* 頂点修正 */
	if(MergeVertex(lpoutline, hP3DModel, NULL) == 0)
	{
		DestroyOutline(lpoutline, nFlagA | nFlag);
		return 0;
	}

	/* ネスト処理 */
	unCount = 0;
	if(CreateOutlineNest(lpoutline, hP3DModel, NULL, &unCount, nFlagA | nFlag) == 0)
	{
		DestroyOutline(lpoutline, nFlagA | nFlag);
		return 0;
	}

	/* エッジの連結処理 */
	if((nFlagA | nFlag) & OL_MFLAG_JOINTEDGE)
	{
		if(JointEdge(lpoutline, nFlagA | nFlag) == 0)
		{
			DestroyOutline(lpoutline, nFlagA | nFlag);
			return 0;
		}
	}

	return 1;
}

/*******************************************************************************
 * アウトラインの破棄
 */
int DestroyOutline(		/* 1: 成功 */
						/* 0: 失敗 */
	LPOUTLINE lpoutline,	/* アウトライン */
	int       nFlag)		/* フラグ */
{
	unsigned int i, j;

	if(lpoutline == NULL)
		return 0;

	/* テキスト出力 */
	_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "DestroyOutline\n");

	/* オブジェクト配列 */
	if(lpoutline->lpol_object != NULL)
	{
		for(i = 0; i < lpoutline->unObjCount; i++)
		{
			if(lpoutline->lpol_object[i].lpol_vertex != NULL)
			{
				free(lpoutline->lpol_object[i].lpol_vertex);

				/* テキスト出力 */
				_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tFree lpol_vertex\n");
			}
		}
		free(lpoutline->lpol_object);

		/* テキスト出力 */
		_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tFree lpol_object\n");
	}

	/* 色グループ名前配列の配列 */
	if(lpoutline->lppszColorName != NULL)
	{
		for(i = 0; i < 3; i++)
		{
			if(lpoutline->lppszColorName[i] != NULL)
			{
				for(j = 0; j < lpoutline->unColorNameCount[i]; j++)
				{
					if(*(lpoutline->lppszColorName[i] + j) != NULL)
					{
						free(*(lpoutline->lppszColorName[i] + j));

						/* テキスト出力 */
						_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tFree lppszColorName[%d] - [%d]\n", i, j);
					}
				}
				free(lpoutline->lppszColorName[i]);

				/* テキスト出力 */
				_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tFree lppszColorName[%d]\n", i);
			}
		}
	}

	/* 処理しない名前配列 */
	if(lpoutline->lppszSkipName != NULL)
	{
		for(i = 0; i < lpoutline->unSkipNameCount; i++)
		{
			if(lpoutline->lppszSkipName[i] != NULL)
			{
				free(lpoutline->lppszSkipName[i]);

				/* テキスト出力 */
				_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tFree lppszSkipName[%d]\n", i);
			}
		}
		free(lpoutline->lppszSkipName);

		/* テキスト出力 */
		_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tFree lppszSkipName\n");
	}

	memset(lpoutline, 0x00, sizeof(OUTLINE));

	return 1;
}

/*******************************************************************************
 * local
 */
/*******************************************************************************
 * 頂点の修正
 */
static int MergeVertex(		/* 1: 成功 */
							/* 0: 失敗 */
	LPOUTLINE  lpoutline,	/* アウトライン */
	HP3DMODEL  hP3DModel,	/* モデルハンドル */
	HP3DOBJECT hP3DObject)	/* オブジェクトハンドル */
{
	HP3DOBJECT       hP3DObjectL;
	LPP3DVERTEX      lpVert;
	LPP3DPOLYGON     lpPoly;
	LPP3DPOLYGONNODE lpNode;
	int              i, j, k, l;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			if(MergeVertex(lpoutline, hP3DModel, hP3DObjectL) == 0)
				return 0;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}
	if(hP3DObject == NULL)
		return 1;

	lpVert = P3DObject(hP3DObject)->lpP3DVertex;
	lpPoly = P3DObject(hP3DObject)->lpP3DPolygon;
	for(i = 0; i < P3DObject(hP3DObject)->nVertexCount; i++)
	{
		for(j = 0; j < i; j++)
		{
			/* 同じ座標の頂点が２つある */
			if(    (fabs(lpVert[i].p3dXYZCalc.x - lpVert[j].p3dXYZCalc.x) <= lpoutline->dPMargin)
				&& (fabs(lpVert[i].p3dXYZCalc.y - lpVert[j].p3dXYZCalc.y) <= lpoutline->dPMargin)
				&& (fabs(lpVert[i].p3dXYZCalc.z - lpVert[j].p3dXYZCalc.z) <= lpoutline->dPMargin))
			{
				for(k = 0; k < P3DObject(hP3DObject)->nPolygonCount; k++)
				{
					lpNode = lpPoly[k].lpP3DPolygonNode;
					for(l = 0; l < lpPoly[k].nPolygonNodeCount; l++)
					{
						/* 若いインデックスにまとめる */
						if(lpNode[l].nVertex == i)
							lpNode[l].nVertex = j;
					}
				}
				break;
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * アウトラインの生成
 */
static int CreateOutlineNest(	/* 1: 成功 */
								/* 0: 失敗 */
	LPOUTLINE     lpoutline,	/* アウトライン */
	HP3DMODEL     hP3DModel,	/* モデルハンドル */
	HP3DOBJECT    hP3DObject,	/* オブジェクトハンドル */
	unsigned int* lpunCount,	/* カウンター */
	int           nFlag)		/* フラグ */
{
	HP3DOBJECT   hP3DObjectL;
	unsigned int i;

	if((hP3DObjectL = P3DGetFirstObject(hP3DModel, hP3DObject)) != NULL)
	{
		do
		{
			/* 変換定義の削除 */
			if(nFlag & OL_MFLAG_DELCONVNAME)
				_OL_DeleteConvName(P3DObject(hP3DObjectL)->szName, sizeof(P3DObject(hP3DObjectL)->szName));

			/* 処理しない名前 */
			for(i = 0; i < lpoutline->unSkipNameCount; i++)
			{
				if(nFlag & OL_MFLAG_NAMEPREFIX)
				{
					if(strncmp(P3DObject(hP3DObjectL)->szName, lpoutline->lppszSkipName[i], strlen(lpoutline->lppszSkipName[i])) == 0)
						break;
				}
				else
				{
					if(strcmp(P3DObject(hP3DObjectL)->szName, lpoutline->lppszSkipName[i]) == 0)
						break;
				}
			}
			if(i != lpoutline->unSkipNameCount)
			{
				/* テキスト出力 */
				_OL_TextOutput(nFlag, lpoutline->fpLog, "\tSkip Object [%s]\n", P3DObject(hP3DObjectL)->szName);
				continue;
			}

			/* オブジェクトアウトラインの生成 */
			if(CreateOutlineObject(lpoutline, hP3DModel, hP3DObjectL, lpunCount, nFlag) == 0)
				return 0;

			/* ネスト処理 */
			(*lpunCount)++;
			if(CreateOutlineNest(lpoutline, hP3DModel, hP3DObjectL, lpunCount, nFlag) == 0)
				return 0;
		}
		while((hP3DObjectL = P3DGetNextObject(hP3DObjectL)) != NULL);
	}

	return 1;
}

/*******************************************************************************
 * アウトラインオブジェクトの生成
 */
static int CreateOutlineObject(		/* 1: 成功 */
									/* 0: 失敗 */
	LPOUTLINE     lpoutline,	/* アウトライン */
	HP3DMODEL     hP3DModel,	/* モデルハンドル */
	HP3DOBJECT    hP3DObject,	/* オブジェクトハンドル */
	unsigned int* lpunCount,	/* オブジェクト番号 */
	int           nFlag)		/* フラグ */
{
	LPP3DVERTEX      lpVert;
	LPOL_OBJECT      lpol_object;
	LPP3DPOLYGON     lpPoly;
	LPP3DPOLYGONNODE lpNode;
	int              nVert1, nVert2, nMode1, tmp;
	int              nEdgeVert[OL_POLYNODE_MAX][4];
	int              nEdgeVert2[OL_POLYNODE_MAX + 1];
	int              nSkip1, nSkip2;
	unsigned int     unVertCount;
	int              i, j;

	lpVert = P3DObject(hP3DObject)->lpP3DVertex;

	/* テキスト出力 */
	_OL_TextOutput(nFlag, lpoutline->fpLog, "\tOutline Object%u [%s]", *lpunCount, P3DObject(hP3DObject)->szName);

	/* アウトラインオブジェクト */
	if(nFlag & OL_MFLAG_ONEOBJECT)
	{
		if(lpoutline->unObjCount == 0)
		{
			if((lpoutline->lpol_object = (LPOL_OBJECT)calloc(1, sizeof(OL_OBJECT))) == NULL)
				return 0;
			lpoutline->unObjCount = 1;

			lpol_object        = lpoutline->lpol_object;
			lpol_object->nID   = _OL_StrCode(P3DObject(hP3DObject)->szName);
			lpol_object->nFlag = GetObjColorFlag(lpoutline, P3DObject(hP3DObject)->szName, nFlag);

			/* テキスト出力 */
			_OL_TextOutput(nFlag, lpoutline->fpLog, " (Alloc)");
		}
		else
		{
			lpol_object = lpoutline->lpol_object;
		}
	}
	else
	{
		if((lpol_object = (LPOL_OBJECT)realloc(
			lpoutline->lpol_object, sizeof(OL_OBJECT) * (lpoutline->unObjCount + 1))) == NULL)
			return 0;
		lpoutline->lpol_object = lpol_object;
		lpoutline->unObjCount++;

		lpol_object        = &lpoutline->lpol_object[lpoutline->unObjCount - 1];
		memset(lpol_object, 0x00, sizeof(OL_OBJECT));
		lpol_object->nID   = _OL_StrCode(P3DObject(hP3DObject)->szName);
		lpol_object->nFlag = GetObjColorFlag(lpoutline, P3DObject(hP3DObject)->szName, nFlag);

		/* テキスト出力 */
		_OL_TextOutput(nFlag, lpoutline->fpLog, " (Alloc)");
	}

	/* テキスト出力 */
	_OL_TextOutput(nFlag, lpoutline->fpLog, "\n\t\t");

	/* 各ポリゴン */
	for(i = 0; i < P3DObject(hP3DObject)->nPolygonCount; i++)
	{
		lpPoly = &(P3DObject(hP3DObject)->lpP3DPolygon[i]);

		/* ポリゴンノード数のチェック */
		if(lpPoly->nPolygonNodeCount > OL_POLYNODE_MAX)
			return 0;

		/* 各エッジをチェック */
		for(j = 0; j < lpPoly->nPolygonNodeCount; j++)
		{
			lpNode = &lpPoly->lpP3DPolygonNode[j];

			/* 次のノードの取得 */
			if(j != lpPoly->nPolygonNodeCount - 1)
			{
				nVert1 = lpNode->nVertex;
				nVert2 = (lpNode + 1)->nVertex;
			}
			else
			{
				nVert1 = lpNode->nVertex;
				nVert2 = (lpPoly->lpP3DPolygonNode)->nVertex;
			}

			/* Ｘ座標の小さい順、Ｙ座標の小さい順、Ｚ座標の小さい順 */
			if(lpVert[nVert1].p3dXYZCalc.x != lpVert[nVert2].p3dXYZCalc.x)
			{
				nEdgeVert[j][0] = (lpVert[nVert1].p3dXYZCalc.x < lpVert[nVert2].p3dXYZCalc.x) ? nVert1 : nVert2;
				nEdgeVert[j][1] = (lpVert[nVert1].p3dXYZCalc.x < lpVert[nVert2].p3dXYZCalc.x) ? nVert2 : nVert1;
				nMode1          = 0;
			}
			else if(lpVert[nVert1].p3dXYZCalc.y != lpVert[nVert2].p3dXYZCalc.y)
			{
				nEdgeVert[j][0] = (lpVert[nVert1].p3dXYZCalc.y < lpVert[nVert2].p3dXYZCalc.y) ? nVert1 : nVert2;
				nEdgeVert[j][1] = (lpVert[nVert1].p3dXYZCalc.y < lpVert[nVert2].p3dXYZCalc.y) ? nVert2 : nVert1;
				nMode1          = 1;
			}
			else
			{
				nEdgeVert[j][0] = (lpVert[nVert1].p3dXYZCalc.z < lpVert[nVert2].p3dXYZCalc.z) ? nVert1 : nVert2;
				nEdgeVert[j][1] = (lpVert[nVert1].p3dXYZCalc.z < lpVert[nVert2].p3dXYZCalc.z) ? nVert2 : nVert1;
				nMode1          = 2;
			}
			nEdgeVert[j][2] = nEdgeVert[j][3] = -1;

			if(CheckEdge(lpoutline, lpol_object, hP3DObject, lpPoly, nEdgeVert[j], nMode1, nFlag) == 0)
				return 0;

			/* エッジ頂点の順番を修正 */
			if(nEdgeVert[j][0] != lpNode->nVertex)
			{
				tmp             = nEdgeVert[j][0];
				nEdgeVert[j][0] = nEdgeVert[j][1];
				nEdgeVert[j][1] = tmp;
				tmp             = nEdgeVert[j][2];
				nEdgeVert[j][2] = nEdgeVert[j][3];
				nEdgeVert[j][3] = tmp;
			}
		}

		/* エッジを追加 */
		/*
		   出力テキストの意味
		     -         エッジを追加しない
		     <         エッジの前半を追加しない
		     >         エッジの後半を追加しない
		     16進数値  エッジを追加 (頂点数)
		     .         ポリゴンの終わり
			 A         メモリを確保
		 */
		nSkip1 = 0;
		nSkip2 = 0;
		for(j = 0; j < lpPoly->nPolygonNodeCount; j++)
		{
			if((nEdgeVert[j][0] == nEdgeVert[j][2]) && (nEdgeVert[j][1] == nEdgeVert[j][3]))
			{
				/* テキスト出力 */
				_OL_TextOutput(nFlag, lpoutline->fpLog, "-");
				continue;
			}

			nSkip2 = 0;
			/* １つ目の頂点 */
			if(nSkip1 == 0)
			{
				if(nEdgeVert[j][0] == nEdgeVert[j][2])
				{
					/* テキスト出力 */
					_OL_TextOutput(nFlag, lpoutline->fpLog, "<");
				}
				else
				{
					if(nEdgeVert[j][2] != -1)
					{
						/* １本で終わり */
						unVertCount = 2;
						nEdgeVert2[0] = nEdgeVert[j][0];
						nEdgeVert2[1] = nEdgeVert[j][2];
					}
					else
					{
						/* 次のエッジへ連結 */
						unVertCount = 2;
						nEdgeVert2[0] = nEdgeVert[j][0];
						nEdgeVert2[1] = nEdgeVert[j][1];

						for(j++ ; j < lpPoly->nPolygonNodeCount; j++)
						{
							if(nEdgeVert[j][0] == nEdgeVert[j][2])
							{
								/* 連結の終わり */
								j--;
								nSkip2 = 1;
								break;
							}
							else if(nEdgeVert[j][2] != -1)
							{
								/* 連結の終わり */
								nEdgeVert2[unVertCount] = nEdgeVert[j][2];
								unVertCount++;
								break;
							}

							/* 連結を続ける */
							nEdgeVert2[unVertCount] = nEdgeVert[j][1];
							unVertCount++;
						}
						if(j == lpPoly->nPolygonNodeCount)
							nSkip2 = 1;
					}

					if(AddEdge(lpoutline, lpol_object, hP3DObject, unVertCount, nEdgeVert2, nFlag) == 0)
						return 0;

					/* テキスト出力 */
					_OL_TextOutput(nFlag, lpoutline->fpLog, "%x", unVertCount);
				}
			}

			nSkip1 = 0;
			/* ２つ目の頂点 */
			if(nSkip2 == 0)
			{
				if(nEdgeVert[j][1] == nEdgeVert[j][3])
				{
					/* テキスト出力 */
					_OL_TextOutput(nFlag, lpoutline->fpLog, ">");
				}
				else
				{
					/* 次のエッジへ連結 */
					unVertCount = 2;
					nEdgeVert2[0] = nEdgeVert[j][3];
					nEdgeVert2[1] = nEdgeVert[j][1];

					for(j++ ; j < lpPoly->nPolygonNodeCount; j++)
					{
						if(nEdgeVert[j][0] == nEdgeVert[j][2])
						{
							/* 連結の終わり */
							j--;
							break;
						}
						else if(nEdgeVert[j][2] != -1)
						{
							/* 連結の終わり */
							nEdgeVert2[unVertCount] = nEdgeVert[j][2];
							unVertCount++;
							j--;
							nSkip1 = 1;
							break;
						}

						/* 連結を続ける */
						nEdgeVert2[unVertCount] = nEdgeVert[j][1];
						unVertCount++;
					}

					if(AddEdge(lpoutline, lpol_object, hP3DObject, unVertCount, nEdgeVert2, nFlag) == 0)
						return 0;

					/* テキスト出力 */
					_OL_TextOutput(nFlag, lpoutline->fpLog, "%x", unVertCount);
				}
			}
		}

		/* テキスト出力 */
		_OL_TextOutput(nFlag, lpoutline->fpLog, ".");
	}

	if(lpol_object->unVertCount == 0)
	{
		/* アウトラインオブジェクトを解放 */
		if((lpol_object = (LPOL_OBJECT)realloc(
			lpoutline->lpol_object, sizeof(OL_OBJECT) * (lpoutline->unObjCount - 1))) == NULL)
		{
			if(lpoutline->unObjCount != 1)
				return 0;
		}
		lpoutline->lpol_object = lpol_object;
		lpoutline->unObjCount--;
		(*lpunCount)--;

		/* テキスト出力 */
		_OL_TextOutput(nFlag, lpoutline->fpLog, " (Free)");
	}

	/* テキスト出力 */
	_OL_TextOutput(nFlag, lpoutline->fpLog, "\n");

	return 1;
}

/*******************************************************************************
 * 色グループの設定
 */
static int GetObjColorFlag(		/* 色グループフラグ */
	LPOUTLINE lpoutline,	/* アウトライン */
	char*     lpszName,		/* 名前 */
	int       nFlag)		/* フラグ */
{
	unsigned int i, j;

	for(i = 0; i < 3; i++)
	{
		for(j = 0; j < lpoutline->unColorNameCount[i]; j++)
		{
			if(nFlag & OL_MFLAG_NAMEPREFIX)
			{
				if(strncmp(lpszName, *(lpoutline->lppszColorName[i] + j), strlen(*(lpoutline->lppszColorName[i] + j))) == 0)
					break;
			}
			else
			{
				if(strcmp(lpszName, *(lpoutline->lppszColorName[i] + j)) == 0)
					break;
			}
		}
		if(j != lpoutline->unColorNameCount[i])
		{
			switch(i)
			{
			case 0: return OLO_FLAG_COLOR2;
			case 1: return OLO_FLAG_COLOR3;
			case 2: return OLO_FLAG_COLOR4;
			}
		}
	}

	return OLO_FLAG_COLOR1;
}

/*******************************************************************************
 * エッジを追加するかチェック
 *
 *   エッジを追加しない
 *     lpnEdgeVert[2] == lpnEdgeVert[0];
 *     lpnEdgeVert[3] == lpnEdgeVert[1];
 *
 *   エッジを完全に追加する
 *     lpnEdgeVert[2] == -1;
 *     lpnEdgeVert[3] == -1;
 *
 *   エッジの一部を追加する
 *     lpnEdgeVert[0] から lpnEdgeVert[2] の間
 *     lpnEdgeVert[1] から lpnEdgeVert[3] の間
 */
static int CheckEdge(	/* 1: 成功 */
						/* 0: 失敗 */
	LPOUTLINE    lpoutline,		/* アウトライン */
	LPOL_OBJECT  lpol_object,	/* アウトラインオブジェクト */
	HP3DOBJECT   hP3DObject,	/* オブジェクトハンドル */
	LPP3DPOLYGON lpPoly,		/* ポリゴン */
	int*         lpnEdgeVert,	/* エッジ頂点配列 */
	int          nMode1,		/* エッジのモード  0:Ｘ  1:Ｙ  2:Ｚ */
	int          nFlag)			/* フラグ */
{
	LPP3DVERTEX      lpVert;
	LPP3DPOLYGON     lpPolyL;
	LPP3DPOLYGONNODE lpNode;
	LPP3DPOLYGONNODE lpNodeNext;
	P3DXYZ           p3dxyz_1, p3dxyz_2, p3dxyz_3, p3dxyz_4;
	int              nVert1, nVert2, nMode2;
	int              nShare;
	unsigned int     u;
	int              i, j;

	lpVert = P3DObject(hP3DObject)->lpP3DVertex;

	/* 既に含まれているかチェック */
	for(u = 0; u < lpol_object->unVertCount; u++)
	{
		if(    (lpol_object->lpol_vertex[u].fx == (float)lpVert[lpnEdgeVert[0]].p3dXYZCalc.x)
			&& (lpol_object->lpol_vertex[u].fy == (float)lpVert[lpnEdgeVert[0]].p3dXYZCalc.y)
			&& (lpol_object->lpol_vertex[u].fz == (float)lpVert[lpnEdgeVert[0]].p3dXYZCalc.z))
		{
			if(u != 0)
			{
				if(    ( lpol_object->lpol_vertex[u - 1].fx    == (float)lpVert[lpnEdgeVert[1]].p3dXYZCalc.x)
					&& ( lpol_object->lpol_vertex[u - 1].fy    == (float)lpVert[lpnEdgeVert[1]].p3dXYZCalc.y)
					&& ( lpol_object->lpol_vertex[u - 1].fz    == (float)lpVert[lpnEdgeVert[1]].p3dXYZCalc.z)
					&& ((lpol_object->lpol_vertex[u    ].nFlag &  OLV_FLAG_DRAWEDGE) != 0))
				{
					lpnEdgeVert[2] = lpnEdgeVert[0];
					lpnEdgeVert[3] = lpnEdgeVert[1];
					return 1;
				}
			}
			if(u != lpol_object->unVertCount - 1)
			{
				if(    ( lpol_object->lpol_vertex[u + 1].fx    == (float)lpVert[lpnEdgeVert[1]].p3dXYZCalc.x)
					&& ( lpol_object->lpol_vertex[u + 1].fy    == (float)lpVert[lpnEdgeVert[1]].p3dXYZCalc.y)
					&& ( lpol_object->lpol_vertex[u + 1].fz    == (float)lpVert[lpnEdgeVert[1]].p3dXYZCalc.z)
					&& ((lpol_object->lpol_vertex[u + 1].nFlag &  OLV_FLAG_DRAWEDGE) != 0))
				{
					lpnEdgeVert[2] = lpnEdgeVert[0];
					lpnEdgeVert[3] = lpnEdgeVert[1];
					return 1;
				}
			}
		}
	}

	/* とがっているかチェック */
	for(i = 0; i < P3DObject(hP3DObject)->nPolygonCount; i++)
	{
		lpPolyL = &P3DObject(hP3DObject)->lpP3DPolygon[i];

		if(lpPolyL == lpPoly)
			continue;

		for(j = 0; j < lpPolyL->nPolygonNodeCount; j++)
		{
			lpNode = &lpPolyL->lpP3DPolygonNode[j];

			/* 次のノードの取得 */
			if(j != lpPolyL->nPolygonNodeCount - 1) lpNodeNext = lpNode + 1;
			else                                    lpNodeNext = lpPolyL->lpP3DPolygonNode;

			/* Ｘ座標の小さい順、Ｙ座標の小さい順、Ｚ座標の小さい順 */
			if(lpVert[lpNode->nVertex].p3dXYZCalc.x != lpVert[lpNodeNext->nVertex].p3dXYZCalc.x)
			{
				nVert1 = (lpVert[lpNode->nVertex].p3dXYZCalc.x < lpVert[lpNodeNext->nVertex].p3dXYZCalc.x) ? lpNode->nVertex     : lpNodeNext->nVertex;
				nVert2 = (lpVert[lpNode->nVertex].p3dXYZCalc.x < lpVert[lpNodeNext->nVertex].p3dXYZCalc.x) ? lpNodeNext->nVertex : lpNode->nVertex;
				nMode2 = 0;
			}
			else if(lpVert[lpNode->nVertex].p3dXYZCalc.y != lpVert[lpNodeNext->nVertex].p3dXYZCalc.y)
			{
				nVert1 = (lpVert[lpNode->nVertex].p3dXYZCalc.y < lpVert[lpNodeNext->nVertex].p3dXYZCalc.y) ? lpNode->nVertex     : lpNodeNext->nVertex;
				nVert2 = (lpVert[lpNode->nVertex].p3dXYZCalc.y < lpVert[lpNodeNext->nVertex].p3dXYZCalc.y) ? lpNodeNext->nVertex : lpNode->nVertex;
				nMode2 = 1;
			}
			else
			{
				nVert1 = (lpVert[lpNode->nVertex].p3dXYZCalc.z < lpVert[lpNodeNext->nVertex].p3dXYZCalc.z) ? lpNode->nVertex     : lpNodeNext->nVertex;
				nVert2 = (lpVert[lpNode->nVertex].p3dXYZCalc.z < lpVert[lpNodeNext->nVertex].p3dXYZCalc.z) ? lpNodeNext->nVertex : lpNode->nVertex;
				nMode2 = 2;
			}

			/* エッジの共有チェック */
			nShare = _OL_CheckEdgeShare(
				lpVert[lpnEdgeVert[0]].p3dXYZCalc, lpVert[lpnEdgeVert[1]].p3dXYZCalc,
				lpVert[nVert1        ].p3dXYZCalc, lpVert[nVert2        ].p3dXYZCalc, lpoutline->dPMargin, nMode1, nMode2);
			if(nShare != 0)
			{
				if(nFlag & OL_MFLAG_CALCNORMAL)
				{
					p3dxyz_1.x = lpVert[lpPoly->lpP3DPolygonNode[0].nVertex].p3dXYZCalc.x - lpVert[lpPoly->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.x;
					p3dxyz_1.y = lpVert[lpPoly->lpP3DPolygonNode[0].nVertex].p3dXYZCalc.y - lpVert[lpPoly->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.y;
					p3dxyz_1.z = lpVert[lpPoly->lpP3DPolygonNode[0].nVertex].p3dXYZCalc.z - lpVert[lpPoly->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.z;
					p3dxyz_2.x = lpVert[lpPoly->lpP3DPolygonNode[2].nVertex].p3dXYZCalc.x - lpVert[lpPoly->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.x;
					p3dxyz_2.y = lpVert[lpPoly->lpP3DPolygonNode[2].nVertex].p3dXYZCalc.y - lpVert[lpPoly->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.y;
					p3dxyz_2.z = lpVert[lpPoly->lpP3DPolygonNode[2].nVertex].p3dXYZCalc.z - lpVert[lpPoly->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.z;
					/* p3dxyz_3 = _OL_CrossProduct(p3dxyz_1, p3dxyz_2); (関数バージョン) */
					_OL_CROSSPRODUCT(p3dxyz_3, p3dxyz_1, p3dxyz_2);

					p3dxyz_1.x = lpVert[lpPolyL->lpP3DPolygonNode[0].nVertex].p3dXYZCalc.x - lpVert[lpPolyL->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.x;
					p3dxyz_1.y = lpVert[lpPolyL->lpP3DPolygonNode[0].nVertex].p3dXYZCalc.y - lpVert[lpPolyL->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.y;
					p3dxyz_1.z = lpVert[lpPolyL->lpP3DPolygonNode[0].nVertex].p3dXYZCalc.z - lpVert[lpPolyL->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.z;
					p3dxyz_2.x = lpVert[lpPolyL->lpP3DPolygonNode[2].nVertex].p3dXYZCalc.x - lpVert[lpPolyL->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.x;
					p3dxyz_2.y = lpVert[lpPolyL->lpP3DPolygonNode[2].nVertex].p3dXYZCalc.y - lpVert[lpPolyL->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.y;
					p3dxyz_2.z = lpVert[lpPolyL->lpP3DPolygonNode[2].nVertex].p3dXYZCalc.z - lpVert[lpPolyL->lpP3DPolygonNode[1].nVertex].p3dXYZCalc.z;
					/* p3dxyz_4 = _OL_CrossProduct(p3dxyz_1, p3dxyz_2); (関数バージョン) */
					_OL_CROSSPRODUCT(p3dxyz_4, p3dxyz_1, p3dxyz_2);
				}
				else
				{
					p3dxyz_3 = lpPoly->p3dXYZNormal;
					p3dxyz_4 = lpPolyL->p3dXYZNormal;
				}

				/* if(1.0 - (_OL_DotProduct(p3dxyz_3, p3dxyz_4) / (_OL_VectorLength(p3dxyz_3) * _OL_VectorLength(p3dxyz_4))) > lpoutline->dAMargin) (関数バージョン) */
				if(    (((nFlag & OL_MFLAG_IGNOREFACES) == 0)
						&& (1.0 - (_OL_DOTPRODUCT(p3dxyz_3, p3dxyz_4) / (_OL_VECTORLENGTH(p3dxyz_3) * _OL_VECTORLENGTH(p3dxyz_4))) > lpoutline->dAMargin))
					|| ((nFlag & OL_MFLAG_IGNOREFACES)
						&& (1.0 - fabs(_OL_DOTPRODUCT(p3dxyz_3, p3dxyz_4) / (_OL_VECTORLENGTH(p3dxyz_3) * _OL_VECTORLENGTH(p3dxyz_4))) > lpoutline->dAMargin)))
				{
					lpnEdgeVert[2] = -1;
					lpnEdgeVert[3] = -1;
					return 1;
				}
				else
				{
					switch(nShare)
					{
					case 1:		/* 完全に一致 */
					case 3:		/* エッジ２上にエッジ１の両頂点がある */
						lpnEdgeVert[2] = lpnEdgeVert[0];
						lpnEdgeVert[3] = lpnEdgeVert[1];
						break;

					case 2:		/* エッジ１上にエッジ２の両頂点がある */
						if(lpnEdgeVert[2] == -1)
							lpnEdgeVert[2] = nVert1;
						else
						{
							if     (nMode1 == 0) lpnEdgeVert[2] = (lpVert[lpnEdgeVert[2]].p3dXYZCalc.x < lpVert[nVert1].p3dXYZCalc.x) ? lpnEdgeVert[2] : nVert1;
							else if(nMode1 == 1) lpnEdgeVert[2] = (lpVert[lpnEdgeVert[2]].p3dXYZCalc.y < lpVert[nVert1].p3dXYZCalc.y) ? lpnEdgeVert[2] : nVert1;
							else                 lpnEdgeVert[2] = (lpVert[lpnEdgeVert[2]].p3dXYZCalc.z < lpVert[nVert1].p3dXYZCalc.z) ? lpnEdgeVert[2] : nVert1;
						}

						if(lpnEdgeVert[3] == -1)
							lpnEdgeVert[3] = nVert2;
						else
						{
							if     (nMode1 == 0) lpnEdgeVert[3] = (lpVert[lpnEdgeVert[3]].p3dXYZCalc.x > lpVert[nVert2].p3dXYZCalc.x) ? lpnEdgeVert[3] : nVert2;
							else if(nMode1 == 1) lpnEdgeVert[3] = (lpVert[lpnEdgeVert[3]].p3dXYZCalc.y > lpVert[nVert2].p3dXYZCalc.y) ? lpnEdgeVert[3] : nVert2;
							else                 lpnEdgeVert[3] = (lpVert[lpnEdgeVert[3]].p3dXYZCalc.z > lpVert[nVert2].p3dXYZCalc.z) ? lpnEdgeVert[3] : nVert2;
						}
						break;

					case 4:		/* エッジ１上にエッジ２の頂点１があり、エッジ２の頂点２はエッジ１直線上にある */
					case 7:		/* エッジ２上にエッジ１の頂点２があり、エッジ１の頂点１はエッジ２直線上にある */
						if(lpnEdgeVert[2] == -1)
							lpnEdgeVert[2] = nVert1;
						else
						{
							if     (nMode1 == 0) lpnEdgeVert[2] = (lpVert[lpnEdgeVert[2]].p3dXYZCalc.x < lpVert[nVert1].p3dXYZCalc.x) ? lpnEdgeVert[2] : nVert1;
							else if(nMode1 == 1) lpnEdgeVert[2] = (lpVert[lpnEdgeVert[2]].p3dXYZCalc.y < lpVert[nVert1].p3dXYZCalc.y) ? lpnEdgeVert[2] : nVert1;
							else                 lpnEdgeVert[2] = (lpVert[lpnEdgeVert[2]].p3dXYZCalc.z < lpVert[nVert1].p3dXYZCalc.z) ? lpnEdgeVert[2] : nVert1;
						}

						lpnEdgeVert[3] = lpnEdgeVert[1];
						break;

					case 5:		/* エッジ１上にエッジ２の頂点２があり、エッジ２の頂点１はエッジ１直線上にある */
					case 6:		/* エッジ２上にエッジ１の頂点１があり、エッジ１の頂点２はエッジ２直線上にある */
						lpnEdgeVert[2] = lpnEdgeVert[0];

						if(lpnEdgeVert[3] == -1)
							lpnEdgeVert[3] = nVert2;
						else
						{
							if     (nMode1 == 0) lpnEdgeVert[3] = (lpVert[lpnEdgeVert[3]].p3dXYZCalc.x > lpVert[nVert2].p3dXYZCalc.x) ? lpnEdgeVert[3] : nVert2;
							else if(nMode1 == 1) lpnEdgeVert[3] = (lpVert[lpnEdgeVert[3]].p3dXYZCalc.y > lpVert[nVert2].p3dXYZCalc.y) ? lpnEdgeVert[3] : nVert2;
							else                 lpnEdgeVert[3] = (lpVert[lpnEdgeVert[3]].p3dXYZCalc.z > lpVert[nVert2].p3dXYZCalc.z) ? lpnEdgeVert[3] : nVert2;
						}
						break;

					default:
						break;
					}
				}
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * エッジの追加
 */
static int AddEdge(		/* 1: 成功 */
						/* 0: 失敗 */
	LPOUTLINE    lpoutline,		/* アウトライン */
	LPOL_OBJECT  lpol_object,	/* アウトラインオブジェクト */
	HP3DOBJECT   hP3DObject,	/* オブジェクトハンドル */
	unsigned int unVertCount,	/* 頂点数 */
	int*         lpnEdgeVert,	/* エッジ頂点配列 */
	int          nFlag)			/* フラグ */
{
	LPP3DVERTEX  lpVert;
	LPOL_VERTEX  lpol_vertex;
	unsigned int i;

	lpVert = P3DObject(hP3DObject)->lpP3DVertex;

	/* メモリの確保 */
	while(lpol_object->unVertCount + unVertCount > lpol_object->unMemSize)
	{
		if((lpol_vertex = (LPOL_VERTEX)realloc(lpol_object->lpol_vertex,
			(lpol_object->unMemSize + lpoutline->unBlockSize) * sizeof(OL_VERTEX))) == NULL)
			return 0;

		lpol_object->lpol_vertex = lpol_vertex;
		memset(&lpol_vertex[lpol_object->unMemSize], 0x00, sizeof(OL_VERTEX) * lpoutline->unBlockSize);
		lpol_object->unMemSize += lpoutline->unBlockSize;

		/* テキスト出力 */
		_OL_TextOutput(nFlag, lpoutline->fpLog, "A");
	}
	lpol_vertex = &lpol_object->lpol_vertex[lpol_object->unVertCount];

	/* エッジの追加 */
	for(i = 0; i < unVertCount; i++)
	{
		if(i != 0)
			lpol_vertex->nFlag |= OLV_FLAG_DRAWEDGE;

		lpol_vertex->fx = (float)lpVert[lpnEdgeVert[i]].p3dXYZCalc.x;
		lpol_vertex->fy = (float)lpVert[lpnEdgeVert[i]].p3dXYZCalc.y;
		lpol_vertex->fz = (float)lpVert[lpnEdgeVert[i]].p3dXYZCalc.z;

		lpol_vertex++;
	}
	lpol_object->unVertCount += unVertCount;

	return 1;
}

/*******************************************************************************
 * エッジの連結
 */
static int JointEdge(	/* 1: 成功 */
						/* 0: 失敗 */
	LPOUTLINE lpoutline,	/* アウトライン */
	int       nFlag)		/* フラグ */
{
	LPOL_OBJECT  lpol_object;
	LPOL_VERTEX  lpol_vertex1, lpol_vertex2, lpol_vertex3, lpol_vertex4;
	unsigned int unVertCount, unStart, unVerts, unStart2, unVerts2;
	unsigned int i, j, k, l;

	for(i = 0; i < lpoutline->unObjCount; i++)
	{
		lpol_object = &lpoutline->lpol_object[i];

		/* テキスト出力 */
		_OL_TextOutput(nFlag, lpoutline->fpLog, "\tJoint Edge Object%u\n\t\t", i);

		/* ワークメモリの確保 */
		if(    ((lpol_vertex1 = (LPOL_VERTEX)malloc(sizeof(OL_VERTEX) * lpol_object->unVertCount)) == NULL)
			|| ((lpol_vertex2 = (LPOL_VERTEX)malloc(sizeof(OL_VERTEX) * lpol_object->unVertCount)) == NULL))
		{
			if(lpol_vertex1 != NULL) free(lpol_vertex1);
			return 0;
		}

		/* メモリのコピー */
		memcpy(lpol_vertex1            , lpol_object->lpol_vertex, sizeof(OL_VERTEX) * lpol_object->unVertCount);
		memset(lpol_object->lpol_vertex, 0x00                    , sizeof(OL_VERTEX) * lpol_object->unMemSize  );
		unVertCount = 0;

		/* 連続ライン１の取得 */
		unStart = 0;
		for(j = 0; j < lpol_object->unVertCount; j++)
		{
			lpol_vertex3 = &lpol_vertex1[j];

			if(    (j == lpol_object->unVertCount - 1)
				|| (((lpol_vertex3 + 1)->nFlag & OLV_FLAG_DRAWEDGE) == 0))
			{
				if(lpol_vertex1[unStart].nFlag & OLV_MFLAG_DONE)
				{
					unStart = j + 1;

					/* テキスト出力 */
					_OL_TextOutput(nFlag, lpoutline->fpLog, "x.");

					continue;
				}

				/* 連続ライン１の頂点数を取得。頂点データを lpol_vertex2 に格納 */
				unVerts = j - unStart + 1;
				memcpy(lpol_vertex2, &lpol_vertex1[unStart], sizeof(OL_VERTEX) * unVerts);

				/* 処理済みフラグをつける */
				lpol_vertex1[unStart].nFlag |= OLV_MFLAG_DONE;

				/* テキスト出力 */
				_OL_TextOutput(nFlag, lpoutline->fpLog, "%u", unVerts);

				/* 連続ライン２の取得 */
				unStart2 = j + 1;
				for(k = j + 2; k < lpol_object->unVertCount; k++)
				{
					lpol_vertex4 = &lpol_vertex1[k];

					if(    (k == lpol_object->unVertCount - 1)
						|| (((lpol_vertex4 + 1)->nFlag & OLV_FLAG_DRAWEDGE) == 0))
					{
						if(lpol_vertex1[unStart2].nFlag & OLV_MFLAG_DONE)
						{
							/* 既に処理済 */
							unStart2 = k + 1;

							continue;
						}

						/* 連続ライン２の頂点数を取得。頂点データを lpol_vertex4 に設定 */
						unVerts2     = k - unStart2 + 1;
						lpol_vertex4 = &lpol_vertex1[unStart2];

						/* ２つの連続ラインを判定 */
						if(    (lpol_vertex2->fx == lpol_vertex4->fx)
							&& (lpol_vertex2->fy == lpol_vertex4->fy)
							&& (lpol_vertex2->fz == lpol_vertex4->fz))
						{
							/* 連続ライン１の前に連続ライン２を逆順で */
							memmove(&lpol_vertex2[unVerts2 - 1], lpol_vertex2, sizeof(OL_VERTEX) * unVerts);
							for(l = 0; l < unVerts2; l++)
							{
								lpol_vertex2[l] = lpol_vertex4[unVerts2 - 1 - l];
							}
							lpol_vertex2->nFlag              &= ~OLV_FLAG_DRAWEDGE;
							lpol_vertex2[unVerts2 - 1].nFlag |=  OLV_FLAG_DRAWEDGE;

							/* テキスト出力 */
							_OL_TextOutput(nFlag, lpoutline->fpLog, "+%u(A)", unVerts2 - 1);
						}
						else if((lpol_vertex2->fx == lpol_vertex4[unVerts2 - 1].fx)
							&&  (lpol_vertex2->fy == lpol_vertex4[unVerts2 - 1].fy)
							&&  (lpol_vertex2->fz == lpol_vertex4[unVerts2 - 1].fz))
						{
							/* 連続ライン１の前に連続ライン２ */
							memmove(&lpol_vertex2[unVerts2 - 1], lpol_vertex2, sizeof(OL_VERTEX) * unVerts);
							for(l = 0; l < unVerts2; l++)
							{
								lpol_vertex2[l] = lpol_vertex4[l];
							}

							/* テキスト出力 */
							_OL_TextOutput(nFlag, lpoutline->fpLog, "+%u(B)", unVerts2 - 1);
						}
						else if((lpol_vertex2[unVerts - 1].fx == lpol_vertex4->fx)
							&&  (lpol_vertex2[unVerts - 1].fy == lpol_vertex4->fy)
							&&  (lpol_vertex2[unVerts - 1].fz == lpol_vertex4->fz))
						{
							/* 連続ライン１の後ろに連続ライン２ */
							for(l = 1; l < unVerts2; l++)
							{
								lpol_vertex2[unVerts - 1 + l] = lpol_vertex4[l];
							}

							/* テキスト出力 */
							_OL_TextOutput(nFlag, lpoutline->fpLog, "+%u(C)", unVerts2 - 1);
						}
						else if((lpol_vertex2[unVerts - 1].fx == lpol_vertex4[unVerts2 - 1].fx)
							&&  (lpol_vertex2[unVerts - 1].fy == lpol_vertex4[unVerts2 - 1].fy)
							&&  (lpol_vertex2[unVerts - 1].fz == lpol_vertex4[unVerts2 - 1].fz))
						{
							/* 連続ライン１の後ろに連続ライン２を逆順で */
							for(l = 1; l < unVerts2; l++)
							{
								lpol_vertex2[unVerts - 1 + l] = lpol_vertex4[unVerts2 - 1 - l];
							}
							lpol_vertex2[unVerts + unVerts2 - 2].nFlag |= OLV_FLAG_DRAWEDGE;

							/* テキスト出力 */
							_OL_TextOutput(nFlag, lpoutline->fpLog, "+%u(D)", unVerts2 - 1);
						}
						else
						{
							/* 処理しない */
							unStart2 = k + 1;

							continue;
						}
						unVerts += unVerts2 - 1;

						/* 処理済みフラグをつけ、最初からやり直し */
						lpol_vertex1[unStart2].nFlag |= OLV_MFLAG_DONE;
						unStart2 = j + 1;
						k = j + 1;
					}
				}
				/* for(k = j + 1; k < lpol_object->unVertCount; k++) */

				/* メモリのコピー */
				memcpy(&lpol_object->lpol_vertex[unVertCount], lpol_vertex2, sizeof(OL_VERTEX) * unVerts);
				unVertCount += unVerts;

				unStart = j + 1;

				/* テキスト出力 */
				_OL_TextOutput(nFlag, lpoutline->fpLog, "=%u.\n\t\t", unVerts);
			}
		}
		/*/for(j = 0; j < lpol_object->unVertCount; j++) */

		/* テキスト出力 */
		_OL_TextOutput(nFlag, lpoutline->fpLog, "\n\t\tVertCount: %u -> %u\n", lpol_object->unVertCount, unVertCount);

		lpol_object->unVertCount = unVertCount;

		/* ワークメモリの解放 */
		free(lpol_vertex1);
		free(lpol_vertex2);
	}
	/* for(i = 0; i < lpoutline->unObjCount; i++) */

	return 1;
}
