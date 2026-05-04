//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	datacopy.c 
	データコピー用。

	1999/11/25 H.Tanaka
	2000/10/18 S.Okajima
	$Id: datacopy.c,v 1.1.1.3 2002/11/19 11:47:47 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include        "utl_dma.h"

/* プログラム使用サブルーチン */

/* スクラッチパットからメインメモリへの転送 */
/* FVECTOR DG_PRIM2_UVRGB DG_PRIM2_UVRGBWH ともに 128bit 単位 */

/* メインメモリからスクラッチパッドへの転送 */
void  _BigScrCopy( void *dst, void *src, int size, int num)
{
    UTL_StartMemToSpr(dst, src, size * num / sizeof(u_long128)) ;
    UTL_EndMemToSpr() ;
}

/* スクラッチパッドからメインメモリへの転送 */
void  _BigMemCopy( void *dst, void *src, int size, int num )
{
    UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) ) ;
    UTL_EndSprToMem() ;
}


