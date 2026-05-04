/*******************************************************************************
 * mdloutline - output.c
 * ファイル出力
 * 2002/03/12 S.Yamashita
 * $Id: output.c,v 1.1 2002/03/19 06:17:49 usr03692 Exp $
 */

/*******************************************************************************
 * include
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
//#include <malloc.h>
//#include <math.h>

#include "_outline.h"

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * アウトラインのファイル出力
 *
 * rol ファイルフォーマット
 *   =========================================================================
 *   | Outline Flag (4)                                                      |
 *   | Outline ObjCount (4)                                                  |
 *   |-----------------------------------------------------------------------|
 *   | ObjCount | OutlineObject ID (4)                                       |
 *   |          | OutlineObject Flag (4)                                     |
 *   |          | OutlineObject Prims (4)                                    |
 *   |          | OutlineObject Verts (4)                                    |
 *   |          | [OutlineObject Verts2 (4)] ※１                            |
 *   |          |------------------------------------------------------------|
 *   |          | Prims * Verts                     | OutlineVertex Flag (4) |
 *   |          |   or                              | OutlineVertex XYZ (12) |
 *   |          | (Prims - 1) * Verts + Verts2 ※１ |                        |
 *   =========================================================================
 * ※１ OutlineObject の Prims の値が 1 でない時
 */
int OutputOutline(	/* 1: 成功 */
					/* 0: 失敗 */
	LPOUTLINE lpoutline,	/* アウトライン */
	char*     lpszFName,	/* ファイル名 */
	int       nFlag)		/* フラグ */
{
	FILE*        fp;
	LPOL_OBJECT  lpol_object;
	LPOL_VERTEX  lpol_vertex;
	OL_VERTEX    ol_vertex;
	unsigned int unPrims, unVerts, unVerts2;
	unsigned int i, j, k;
	int          nFlagTmp;	/* 現在のバージョンだけ */

	/* テキスト出力 */
	_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "OutputOutline\n");

	/* ファイルオープン */
	if((fp = fopen(lpszFName, "wb")) == NULL)
		return 0;

	/* アウトラインの出力 */
	/* フラグ、オブジェクト数 */
	nFlagTmp = 0;
	if(    (fwrite(&nFlagTmp             , sizeof(int         ), 1, fp) != 1)
//	if(    (fwrite(&lpoutline->nFlag     , sizeof(int         ), 1, fp) != 1)
		|| (fwrite(&lpoutline->unObjCount, sizeof(unsigned int), 1, fp) != 1))
	{
		fclose(fp);
		return 0;
	}
	/* テキスト出力 */
	_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tOutline\n");
	_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tFlag    : %08x\n", lpoutline->nFlagA    );
	_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tObjCount: %u\n"  , lpoutline->unObjCount);

	/* アウトラインオブジェクトの出力 */
	for(i = 0; i < lpoutline->unObjCount; i++)
	{
		lpol_object = &lpoutline->lpol_object[i];

		/* オブジェクトＩＤ、フラグ */
		if(    (fwrite(&lpol_object->nID  , sizeof(int), 1, fp) != 1)
			|| (fwrite(&lpol_object->nFlag, sizeof(int), 1, fp) != 1))
		{
			fclose(fp);
			return 0;
		}
		/* テキスト出力 */
		_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tOutlineObject%u\n", i);
		_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tID    : %d\n"  , lpol_object->nID  );
		_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tFlag  : %08x\n", lpol_object->nFlag);

		if(lpol_object->unVertCount <= 64)
		{
			/* プリミティブ数、頂点数 */
			unPrims = 1;
			unVerts = lpol_object->unVertCount;
			if(    (fwrite(&unPrims, sizeof(unsigned int), 1, fp) != 1)
				|| (fwrite(&unVerts, sizeof(unsigned int), 1, fp) != 1))
			{
				fclose(fp);
				return 0;
			}
			/* テキスト出力 */
			_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tPrims : %u\n", unPrims);
			_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tVerts : %u\n", unVerts);

			/* 頂点配列 */
			if(fwrite(lpol_object->lpol_vertex, sizeof(OL_VERTEX), lpol_object->unVertCount, fp) != lpol_object->unVertCount)
			{
				fclose(fp);
				return 0;
			}
			/* テキスト出力 */
			if((lpoutline->nFlagA | nFlag) & OL_MFLAG_OLVINFO)
			{
				_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tPrimitive0\n");
				for(j = 0; j < lpol_object->unVertCount; j++)
					_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\t\tVertex%02u: %08x\t%8f\t%8f\t%8f\n", j, lpol_object->lpol_vertex[j].nFlag, lpol_object->lpol_vertex[j].fx, lpol_object->lpol_vertex[j].fy, lpol_object->lpol_vertex[j].fz);
			}
		}
		else
		{
			/* プリミティブ数、頂点数 */
			unPrims = 2;
			while(lpol_object->unVertCount > 63 * unPrims)
				unPrims++;
			if((lpol_object->unVertCount % unPrims) == 0)
				unVerts = (lpol_object->unVertCount / unPrims) + 1;
			else
				unVerts = (lpol_object->unVertCount / unPrims) + 2;
			unVerts2 = (lpol_object->unVertCount - ((unVerts - 1) * (unPrims - 1))) + 1;
			if(    (fwrite(&unPrims , sizeof(unsigned int), 1, fp) != 1)
				|| (fwrite(&unVerts , sizeof(unsigned int), 1, fp) != 1)
				|| (fwrite(&unVerts2, sizeof(unsigned int), 1, fp) != 1))
			{
				fclose(fp);
				return 0;
			}
			/* テキスト出力 */
			_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tPrims : %u\n", unPrims );
			_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tVerts : %u\n", unVerts );
			_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tVerts2: %u\n", unVerts2);

			/* 頂点配列 */
			for(j = 0; j < unPrims; j++)
			{
				lpol_vertex = &lpol_object->lpol_vertex[j * (unVerts - 1)];

				/* 先頭要素 */
				if(j == 0) memset(&ol_vertex, 0x00, sizeof(OL_VERTEX));
				else       ol_vertex = *(lpol_vertex - 1);
				if(fwrite(&ol_vertex, sizeof(OL_VERTEX), 1, fp) != 1)
				{
					fclose(fp);
					return 0;
				}
				/* テキスト出力 */
				_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\tPrimitive%u\n", j);
				if((lpoutline->nFlagA | nFlag) & OL_MFLAG_OLVINFO)
					_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\t\tVertex00: %08x\t%8f\t%8f\t%8f\n", ol_vertex.nFlag, ol_vertex.fx, ol_vertex.fy, ol_vertex.fz);

				/* 残りの要素 */
				if(j != unPrims - 1)
				{
					if(fwrite(lpol_vertex, sizeof(OL_VERTEX), unVerts - 1, fp) != unVerts - 1)
					{
						fclose(fp);
						return 0;
					}
					/* テキスト出力 */
					if((lpoutline->nFlagA | nFlag) & OL_MFLAG_OLVINFO)
					{
						for(k = 1; k < unVerts; k++)
							_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\t\tVertex%02u: %08x\t%8f\t%8f\t%8f\n", k, lpol_vertex[k - 1].nFlag, lpol_vertex[k - 1].fx, lpol_vertex[k - 1].fy, lpol_vertex[k - 1].fz);
					}
				}
				else
				{
					if(fwrite(lpol_vertex, sizeof(OL_VERTEX), unVerts2 - 1, fp) != unVerts2 - 1)
					{
						fclose(fp);
						return 0;
					}
					/* テキスト出力 */
					if((lpoutline->nFlagA | nFlag) & OL_MFLAG_OLVINFO)
					{
						for(k = 1; k < unVerts2; k++)
							_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\t\t\tVertex%02u: %08x\t%8f\t%8f\t%8f\n", k, lpol_vertex[k - 1].nFlag, lpol_vertex[k - 1].fx, lpol_vertex[k - 1].fy, lpol_vertex[k - 1].fz);
					}
				}
			}
		}
	}

	/* ファイルクローズ */
	fclose(fp);

	return 1;
}
