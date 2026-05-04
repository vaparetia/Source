/*******************************************************************************
 * outline - _outline.c
 * 内部用関数
 * 2002/03/12 S.Yamashita
 * $Id: _outline.c,v 1.1 2002/03/19 06:17:49 usr03692 Exp $
 */

/*******************************************************************************
 * include
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "_outline.h"

/*******************************************************************************
 * functions
 */

static int CheckVertexOnEdge(P3DXYZ p3dxyz_E0, P3DXYZ p3dxyz_E1, P3DXYZ p3dxyz_V, double dPMargin, int nMode);
static int CheckVertexOnEdgeLine(P3DXYZ p3dxyz_E0, P3DXYZ p3dxyz_E1, P3DXYZ p3dxyz_V, double dPMargin, int nMode);

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * 変換定義の削除
 */
int _OL_DeleteConvName(		/* 1: 成功 */
							/* 0: 失敗 */
	char*        lpszName,	/* 名前 */
	unsigned int unSize)	/* バッファのサイズ */
{
	char  szName[OL_STRING_MAX];
	char* p, * p2;

	memset(szName, 0x00, sizeof(szName));
	strncpy(szName, lpszName, sizeof(szName) - 1);

	/* １つ目と２つ目はスキップ、３つ目がオブジェクト名 */
	if(    ((p = strtok(szName, "-"))  == NULL)
		|| ((p = strtok(NULL  , "-"))  == NULL)
		|| ((p = strtok(NULL  , "-"))  == NULL)
		|| (strncmp(szName, "CONV", 4) != 0   ))
		return 0;

	memset(lpszName, 0x00, unSize);
	unSize--;
	if((p2 = strtok(NULL, "-")) == NULL)
		strncpy(lpszName, p, (strlen(p) < unSize) ? strlen(p) : unSize);
	else
		strncpy(lpszName, p, (p2 - p - 1 < unSize) ? p2 - p - 1 : unSize);

	return 1;
}

/*******************************************************************************
 * 文字列ＩＤ
 */
#define ___BIT_LEN (24)

int _OL_StrCode(	/* 文字列ＩＤ */
	char* szString)		/* 文字列 */
{
	unsigned char  c;
	unsigned char* p;
	unsigned int   unID, unMask;

	p      = (unsigned char*)szString;
	unID   = 0;
	unMask = (1 << ___BIT_LEN) - 1;

	while((c = *(p++)) != '\0')
	{
		unID  = (unID << 5) | (unID >> (___BIT_LEN - 5));
		unID += c;
		unID &= unMask;
	}
	if(unID == 0)
		unID = 1;

	return (signed)unID;
}

/*******************************************************************************
 * エッジの共有チェック
 */
int _OL_CheckEdgeShare(		/* 1: 完全に一致 */
							/* 2: エッジ１上にエッジ２の両頂点がある */
							/* 3: エッジ２上にエッジ１の両頂点がある */
							/* 4: エッジ１上にエッジ２の頂点１があり、エッジ２の頂点２はエッジ１直線上にある */
							/* 5: エッジ１上にエッジ２の頂点２があり、エッジ２の頂点１はエッジ１直線上にある */
							/* 6: エッジ２上にエッジ１の頂点１があり、エッジ１の頂点２はエッジ２直線上にある */
							/* 7: エッジ２上にエッジ１の頂点２があり、エッジ１の頂点１はエッジ２直線上にある */
							/* 0: 共有しない */
	P3DXYZ p3dxyz_00,	/* エッジ１の頂点１ */
	P3DXYZ p3dxyz_01,	/* エッジ１の頂点２ */
	P3DXYZ p3dxyz_10,	/* エッジ２の頂点１ */
	P3DXYZ p3dxyz_11,	/* エッジ２の頂点２ */
	double dPMargin,	/* ２つのエッジの重なり誤差判定マージン(0～) */
	int    nMode1,		/* エッジ１のモード  0:Ｘ方向  1:Ｙ方向  2:Ｚ方向 */
	int    nMode2)		/* エッジ２のモード  0:Ｘ方向  1:Ｙ方向  2:Ｚ方向 */
{
	/* 完全に一致しているか */
	if(    (!memcmp(&p3dxyz_00, &p3dxyz_10, sizeof(P3DXYZ)) && !memcmp(&p3dxyz_01, &p3dxyz_11, sizeof(P3DXYZ)))
		|| (!memcmp(&p3dxyz_00, &p3dxyz_11, sizeof(P3DXYZ)) && !memcmp(&p3dxyz_01, &p3dxyz_10, sizeof(P3DXYZ))))
		return 1;
/*
	if(    (   ((p3dxyz_00.x == p3dxyz_10.x) && (p3dxyz_00.y == p3dxyz_10.y) && (p3dxyz_00.z == p3dxyz_10.z))
			&& ((p3dxyz_01.x == p3dxyz_11.x) && (p3dxyz_01.y == p3dxyz_11.y) && (p3dxyz_01.z == p3dxyz_11.z)))
		|| (   ((p3dxyz_00.x == p3dxyz_11.x) && (p3dxyz_00.y == p3dxyz_11.y) && (p3dxyz_00.z == p3dxyz_11.z))
			&& ((p3dxyz_01.x == p3dxyz_10.x) && (p3dxyz_01.y == p3dxyz_10.y) && (p3dxyz_01.z == p3dxyz_10.z))))
			return 1;
*/

	/* 重なっているか */
	if          (CheckVertexOnEdge    (p3dxyz_00, p3dxyz_01, p3dxyz_10, dPMargin, nMode1) == 1) {
		if      (CheckVertexOnEdge    (p3dxyz_00, p3dxyz_01, p3dxyz_11, dPMargin, nMode1) == 1) return 2;
		else if (CheckVertexOnEdgeLine(p3dxyz_00, p3dxyz_01, p3dxyz_11, dPMargin, nMode1) == 1) return 4;
		else                                                                                    return 0;
	} else if   (CheckVertexOnEdge    (p3dxyz_00, p3dxyz_01, p3dxyz_11, dPMargin, nMode1) == 1) {
		if      (CheckVertexOnEdgeLine(p3dxyz_00, p3dxyz_01, p3dxyz_10, dPMargin, nMode1) == 1) return 5;
		else                                                                                    return 0;
	} else if   (CheckVertexOnEdge    (p3dxyz_10, p3dxyz_11, p3dxyz_00, dPMargin, nMode2) == 1) {
		if      (CheckVertexOnEdge    (p3dxyz_10, p3dxyz_11, p3dxyz_01, dPMargin, nMode2) == 1) return 3;
		else if (CheckVertexOnEdgeLine(p3dxyz_10, p3dxyz_11, p3dxyz_01, dPMargin, nMode2) == 1) return 6;
		else                                                                                    return 0;
	} else if   (CheckVertexOnEdge    (p3dxyz_10, p3dxyz_11, p3dxyz_01, dPMargin, nMode2) == 1) {
		if      (CheckVertexOnEdgeLine(p3dxyz_10, p3dxyz_11, p3dxyz_00, dPMargin, nMode2) == 1) return 7;
		else                                                                                    return 0;
	}

	return 0;
}

/*******************************************************************************
 * ベクトルの内積
 */
double _OL_DotProduct(	/* 内積 */
	P3DXYZ p3dxyz1,		/* ベクトル１ */
	P3DXYZ p3dxyz2)		/* ベクトル２ */
{
	return ((p3dxyz1.x * p3dxyz2.x) + (p3dxyz1.y * p3dxyz2.y) + (p3dxyz1.z * p3dxyz2.z));
}

/*******************************************************************************
 * ベクトルの外積
 */
P3DXYZ _OL_CrossProduct(	/* 外積 */
	P3DXYZ p3dxyz1,		/* ベクトル１ */
	P3DXYZ p3dxyz2)		/* ベクトル２ */
{
	P3DXYZ p3dxyz;

	p3dxyz.x = p3dxyz1.y * p3dxyz2.z - p3dxyz1.z * p3dxyz2.y;
	p3dxyz.y = p3dxyz1.z * p3dxyz2.x - p3dxyz1.x * p3dxyz2.z;
	p3dxyz.z = p3dxyz1.x * p3dxyz2.y - p3dxyz1.y * p3dxyz2.x;

	return p3dxyz;
}

/*******************************************************************************
 * ベクトルの長さ
 */
double _OL_VectorLength(	/* 長さ */
	P3DXYZ p3dxyz)	/* ベクトル */
{
	return sqrt((p3dxyz.x * p3dxyz.x) + (p3dxyz.y * p3dxyz.y) + (p3dxyz.z * p3dxyz.z));
}

/*******************************************************************************
 * テキスト出力
 */
int _OL_TextOutput(		/* 1: 成功 */
						/* 0: 失敗 */
	int   nFlag,		/* フラグ */
	FILE* fpLog,		/* ログファイルポインタ */
	char* lpszFormat,	/* テキストの書式 */
	...)				/* 可変引数 */
{
	char    szText[OL_STRING_MAX * 4];
	va_list valist;

	/* テキストの設定 */
	memset(szText, 0x00, sizeof(szText));
	va_start(valist, lpszFormat);
	vsprintf(szText, lpszFormat, valist);
	va_end(valist);

	/* ログファイルに出力 */
	if((nFlag & OL_MFLAG_LOGOUT) && (fpLog != NULL))
	{
		if(fwrite(szText, sizeof(char), strlen(szText), fpLog) != strlen(szText))
			return 0;
	}

	/* 標準出力に出力 */
	if(nFlag & OL_MFLAG_STDOUT)
	{
		printf(szText);
	}

	return 1;
}

/*******************************************************************************
 * local
 */
/*******************************************************************************
 * エッジ上の頂点かチェック
 *   エッジの頂点と頂点が重なるときは 0 を返す
 */
static int CheckVertexOnEdge(	/* 1: エッジ上にある */
								/* 0: エッジ上にない */
	P3DXYZ p3dxyz_E0,	/* エッジの頂点１ */
	P3DXYZ p3dxyz_E1,	/* エッジの頂点２ */
	P3DXYZ p3dxyz_V,	/* 頂点 */
	double dPMargin,	/* ２つのエッジの重なり誤差判定マージン(0～) */
	int    nMode)		/* エッジのモード  0:Ｘ方向  1:Ｙ方向  2:Ｚ方向 */
{
	double d;

	switch(nMode)
	{
	case 0:
		if((p3dxyz_E0.x < p3dxyz_V.x) && (p3dxyz_V.x < p3dxyz_E1.x))
		{
			d = (p3dxyz_V.x - p3dxyz_E0.x) / (p3dxyz_E1.x - p3dxyz_E0.x);

			if(    (fabs((p3dxyz_E0.y + d * (p3dxyz_E1.y - p3dxyz_E0.y)) - p3dxyz_V.y) <= dPMargin)
				&& (fabs((p3dxyz_E0.z + d * (p3dxyz_E1.z - p3dxyz_E0.z)) - p3dxyz_V.z) <= dPMargin))
				return 1;
		}
		else if((p3dxyz_E1.x < p3dxyz_V.x) && (p3dxyz_V.x < p3dxyz_E0.x))
		{
			d = (p3dxyz_V.x - p3dxyz_E1.x) / (p3dxyz_E0.x - p3dxyz_E1.x);

			if(    (fabs((p3dxyz_E1.y + d * (p3dxyz_E0.y - p3dxyz_E1.y)) - p3dxyz_V.y) <= dPMargin)
				&& (fabs((p3dxyz_E1.z + d * (p3dxyz_E0.z - p3dxyz_E1.z)) - p3dxyz_V.z) <= dPMargin))
				return 1;
		}
		break;

	case 1:
		if((p3dxyz_E0.y < p3dxyz_V.y) && (p3dxyz_V.y < p3dxyz_E1.y))
		{
			d = (p3dxyz_V.y - p3dxyz_E0.y) / (p3dxyz_E1.y - p3dxyz_E0.y);

			if(    (fabs( p3dxyz_E0.x                                    - p3dxyz_V.x) <= dPMargin)
				&& (fabs((p3dxyz_E0.z + d * (p3dxyz_E1.z - p3dxyz_E0.z)) - p3dxyz_V.z) <= dPMargin))
				return 1;
		}
		else if((p3dxyz_E1.y < p3dxyz_V.y) && (p3dxyz_V.y < p3dxyz_E0.y))
		{
			d = (p3dxyz_V.y - p3dxyz_E1.y) / (p3dxyz_E0.y - p3dxyz_E1.y);

			if(    (fabs( p3dxyz_E1.x                                    - p3dxyz_V.x) <= dPMargin)
				&& (fabs((p3dxyz_E1.z + d * (p3dxyz_E0.z - p3dxyz_E1.z)) - p3dxyz_V.z) <= dPMargin))
				return 1;
		}
		break;

	case 2:
		if((p3dxyz_E0.z < p3dxyz_V.z) && (p3dxyz_V.z < p3dxyz_E1.z))
		{
			if(    (fabs(p3dxyz_E0.x - p3dxyz_V.x) <= dPMargin)
				&& (fabs(p3dxyz_E0.y - p3dxyz_V.y) <= dPMargin))
				return 1;
		}
		else if((p3dxyz_E1.z < p3dxyz_V.z) && (p3dxyz_V.z < p3dxyz_E0.z))
		{
			if(    (fabs(p3dxyz_E1.x - p3dxyz_V.x) <= dPMargin)
				&& (fabs(p3dxyz_E1.y - p3dxyz_V.y) <= dPMargin))
				return 1;
		}
		break;
	}
	return 0;
}

/*******************************************************************************
 * エッジ直線上の頂点かチェック
 */
static int CheckVertexOnEdgeLine(	/* 1: エッジ直線上にある */
									/* 0: エッジ直線上にない */
	P3DXYZ p3dxyz_E0,	/* エッジの頂点１ */
	P3DXYZ p3dxyz_E1,	/* エッジの頂点２ */
	P3DXYZ p3dxyz_V,	/* 頂点 */
	double dPMargin,	/* ２つのエッジの重なり誤差判定マージン(0～) */
	int    nMode)		/* エッジのモード  0:Ｘ方向  1:Ｙ方向  2:Ｚ方向 */
{
	double d;

	switch(nMode)
	{
	case 0:
		d = (p3dxyz_V.x - p3dxyz_E0.x) / (p3dxyz_E1.x - p3dxyz_E0.x);

		if(    (fabs((p3dxyz_E0.y + d * (p3dxyz_E1.y - p3dxyz_E0.y)) - p3dxyz_V.y) <= dPMargin)
			&& (fabs((p3dxyz_E0.z + d * (p3dxyz_E1.z - p3dxyz_E0.z)) - p3dxyz_V.z) <= dPMargin))
			return 1;
		break;

	case 1:
		d = (p3dxyz_V.y - p3dxyz_E0.y) / (p3dxyz_E1.y - p3dxyz_E0.y);

		if(    (fabs( p3dxyz_E0.x                                    - p3dxyz_V.x) <= dPMargin)
			&& (fabs((p3dxyz_E0.z + d * (p3dxyz_E1.z - p3dxyz_E0.z)) - p3dxyz_V.z) <= dPMargin))
			return 1;
		break;

	case 2:
		if(    (fabs(p3dxyz_E0.x - p3dxyz_V.x) <= dPMargin)
			&& (fabs(p3dxyz_E0.y - p3dxyz_V.y) <= dPMargin))
			return 1;
		break;
	}
	return 0;
}
