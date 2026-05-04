/*******************************************************************************
 * outline - scale.c
 * スケーリング
 * 2002/03/12 S.Yamashita
 * $Id: scale.c,v 1.1 2002/03/19 06:17:49 usr03692 Exp $
 */

/*******************************************************************************
 * include
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <string.h>
//#include <malloc.h>
//#include <math.h>

#include "_outline.h"

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * アウトラインのスケーリング
 */
int ScaleOutline(	/* 1: 成功 */
					/* 0: 失敗 */
	LPOUTLINE lpoutline,	/* アウトライン */
	float     fScale,		/* スケール */
	int       nFlag)		/* フラグ */
{
	LPOL_OBJECT  lpol_object;
	LPOL_VERTEX  lpol_vertex;
	unsigned int i, j;

	if(lpoutline == NULL)
		return 0;

	/* テキスト出力 */
	_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "ScaleOutline\n");
	_OL_TextOutput(lpoutline->nFlagA | nFlag, lpoutline->fpLog, "\tScale: %f\n", fScale);

	for(i = 0; i < lpoutline->unObjCount; i++)
	{
		lpol_object = &lpoutline->lpol_object[i];

		for(j = 0; j < lpol_object->unVertCount; j++)
		{
			lpol_vertex = &lpol_object->lpol_vertex[j];

			lpol_vertex->fx *= fScale;
			lpol_vertex->fy *= fScale;
			lpol_vertex->fz *= fScale;
		}
	}

	return 1;
}
