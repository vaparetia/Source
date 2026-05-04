//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * sy_util - sy_memcpy.c
 * メモリコピー
 * 2002/03/20 S.Yamashita
 * $Id: sy_memcpy.c,v 1.1.1.3 2002/11/19 11:51:53 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include "sy_util.h"

/*******************************************************************************
 * fuctions
 */

extern void UTL_StartSprToMem(void *dst, void *src, int size);
extern void UTL_EndSprToMem(void);
extern void UTL_StartMemToSpr(void *dst, void *src, int size);
extern void UTL_EndMemToSpr(void);

/*******************************************************************************
 * public
 */
/******************************************************************************
 * スクラッチパッドからメインメモリへ転送
 */
void sy_memcpy(
	void *dst,	/* 転送先アドレス */
	void *src,	/* 転送元アドレス */
	int  size,	/* 要素サイズ */
	int  num)	/* 要素数 */
{
	UTL_StartSprToMem(dst, src, size * num / sizeof(u_long128));
	UTL_EndSprToMem();
}

/******************************************************************************
 * メインメモリからスクラッチパッドへ転送
 */
void sy_memcpy2(
	void *dst,	/* 転送先アドレス */
	void *src,	/* 転送元アドレス */
	int  size,	/* 要素サイズ */
	int  num)	/* 要素数 */
{
	UTL_StartMemToSpr(dst, src, size * num / sizeof(u_long128));
	UTL_EndMemToSpr();
}
