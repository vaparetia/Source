//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	utl_dma.c
	ＰＳ２ＤＭＡ制御ユーティリティ

	1999/09/09 K.Takabe
	$Id: utl_dma.c,v 1.1.1.3 2002/11/19 11:42:59 Yoshizawa1 Exp $

*/
/*
	ＰＳ２ＤＭＡ制御ユーティリティ

	void UTL_StartSprToMem( void *dst, void *src, int size );
	void	*dst ;		転送先メインメモリアドレス（qwordに揃っていること）
	void	*src ;		転送元スクラッチパッドアドレス（qwordに揃っていること）
	int		size ;		転送サイズ（qword単位）

		スクラッチからメインメモリへの転送を開始する


	void UTL_EndSprToMem( void );

		スクラッチパッドからメインメモリへの転送終了を待つ


	void UTL_StartMemToSpr( void *dst, void *src, int size );
	void	*dst ;		転送先スクラッチパッドアドレス（qwordに揃っていること）
	void	*src ;		転送元メインメモリアドレス（qwordに揃っていること）
	int		size ;		転送サイズ（qword単位）

		メモリからからスクラッチパッドへの転送を開始する


	void UTL_EndMemToSpr( void );

		メインメモリからスクラッチパッドへの転送終了を待つ




*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libdg.h"

#ifdef PSX2
#include	"def_dma.h"
#endif


	/*
		スクラッチパッドからメインメモリへの転送
	*/
void UTL_StartSprToMem( void *dst, void *src, int size )
{
#ifdef BP_PSX2_ASM
	/* ＣＰＵによる転送バージョン */
	//u_long128	*ret_addr, *exec_addr ;
	u_long128	*copy_src, *copy_dst ;
	int			i ;

	copy_src = (u_long128*)src ;
	copy_dst = (u_long128*)dst ;

	/* アドレスが６４バイト境界になるように端数を転送する */
	i = ( (int)copy_dst/16 ) & 3 ;
	if ( i != 0 ){
		for ( i = 4 - i ; i > 0 ; i-- ){
			*copy_dst++ = *copy_src++ ;
			size-- ;
			if ( size <= 0 ) return ;
		}
	}
	/* キャッシュを考慮して転送する（６４バイト単位） */
	GV_PREFECH( copy_dst );
	while ( size > 4 ){
		asm( "pref 0,64(%0)"::"r"(copy_dst) );
		copy_dst[0] = copy_src[0] ;
		copy_dst[1] = copy_src[1] ;
		copy_dst[2] = copy_src[2] ;
		copy_dst[3] = copy_src[3] ;
		copy_dst += 4 ;
		copy_src += 4 ;
		size -= 4 ;
	}
	for (  ; size > 0 ; size-- ){
		*copy_dst++ = *copy_src++ ;
	}
#else
	memcpy( dst, src, size * sizeof(u_long128) );
#endif
}

	/*
		スクラッチパッドからメインメモリへの転送終了待ち
	*/
void UTL_EndSprToMem( void )
{
}


	/*
		メインメモリからスクラッチパッドへの転送
	*/
void UTL_StartMemToSpr( void *dst, void *src, int size )
{
#ifdef BP_PSX2_ASM
	/* ＣＰＵによる転送バージョン */
	//u_long128	*ret_addr, *exec_addr ;
	u_long128	*copy_src, *copy_dst ;
	int			i ;

	copy_src = (u_long128*)src ;
	copy_dst = (u_long128*)dst ;

	/* アドレスが６４バイト境界になるように端数を転送する */
	i = ( (int)copy_dst/16 ) & 3 ;
	if ( i != 0 ){
		for ( i = 4 - i ; i > 0 ; i-- ){
			*copy_dst++ = *copy_src++ ;
			size-- ;
			if ( size <= 0 ) return ;
		}
	}
	/* キャッシュを考慮して転送する（６４バイト単位） */
	GV_PREFECH( copy_src );
	while ( size > 4 ){
		asm( "pref 0,64(%0)"::"r"(copy_src) );
		copy_dst[0] = copy_src[0] ;
		copy_dst[1] = copy_src[1] ;
		copy_dst[2] = copy_src[2] ;
		copy_dst[3] = copy_src[3] ;
		copy_dst += 4 ;
		copy_src += 4 ;
		size -= 4 ;
	}
	for (  ; size > 0 ; size-- ){
		*copy_dst++ = *copy_src++ ;
	}
#else
	memcpy( dst, src, size * sizeof(u_long128) );
#endif
}

	/*
		メインメモリからスクラッチパッドへの転送終了待ち
	*/
void UTL_EndMemToSpr( void )
{
}

