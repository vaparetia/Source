/*******************************************************************************
 * outline - _outline.h
 * 内部用ヘッダー
 * 2002/03/12 S.Yamashita
 * $Id: _outline.h,v 1.1 2002/03/19 06:17:49 usr03692 Exp $
 */

#ifndef __INC__OUTLINE__
#define __INC__OUTLINE__

#ifdef __cplusplus
extern "C" {		/* Assume C declarations for C++ */
#endif				/* __cplusplus */

/*******************************************************************************
 * include
 */

#include <outline.h>

/*******************************************************************************
 * definitions and typedefs and structures
 */

/* ベクトルの内積 (v1 ･ v2) */
#define _OL_DOTPRODUCT(v1, v2) ((v1).x * (v2).x + (v1).y * (v2).y + (v1).z * (v2).z)

/* ベクトルの外積 (v1 * v2) */
#define _OL_CROSSPRODUCT(v3, v1, v2) {\
	(v3).x = (v1).y * (v2).z - (v1).z * (v2).y; \
	(v3).y = (v1).z * (v2).x - (v1).x * (v2).z; \
	(v3).z = (v1).x * (v2).y - (v1).y * (v2).x; }

/* ベクトルの長さ (math.h) */
#define _OL_VECTORLENGTH(v1) (sqrt(_OL_DOTPRODUCT(v1, v1)))

/*******************************************************************************
 * functions
 */

int    _OL_DeleteConvName(char* lpszName, unsigned int unSize);
int    _OL_StrCode       (char* szString);
int    _OL_CheckEdgeShare(P3DXYZ p3dxyz_00, P3DXYZ p3dxyz_01, P3DXYZ p3dxyz_10, P3DXYZ p3dxyz_11, double dPMargin, int nMode1, int nMode2);
double _OL_DotProduct    (P3DXYZ p3dxyz1, P3DXYZ p3dxyz2);
P3DXYZ _OL_CrossProduct  (P3DXYZ p3dxyz1, P3DXYZ p3dxyz2);
double _OL_VectorLength  (P3DXYZ p3dxyz);
int    _OL_TextOutput    (int nFlag, FILE* fpLog, char* lpszFormat, ...);

/*******************************************************************************
 */

#ifdef __cplusplus
}					/* End of extern "C" { */
#endif				/* __cplusplus */

#endif	/* __INC__OUTLINE__ */
