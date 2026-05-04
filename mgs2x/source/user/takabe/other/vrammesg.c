//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	putspot.c
	静止スポットライト設置

	1999/10/25 K.Takabe
	$Id: vrammesg.c,v 1.1.1.3 2002/11/19 11:51:19 Yoshizawa1 Exp $

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"


/* ---------------------------------------------------------------- */
#if 0
static void LoadImageMessage( void )
{
	sceGsLoadImage	li ;
	int				*image ;
	unsigned char	*buff ;
	int				i ;

	buff = GV_GetCache( GV_CacheID( 10129879/*"vram0"*/, 'r' ) );
	if ( buff == NULL ) return ;
	image = GV_Malloc( 256*32*4 );

	for ( i = 0 ; i < (256*32) ; i++ ){
		int		c = buff[i] >> 1 ;
		image[i] = c | ( c << 8 ) | ( c << 16 ) ;
	}

	/* 描画が終了するまでウェイトを入れる */
	while ( DG_DmaCheckEnd() == 0 );
	sceGsSyncPath( 0, 0 );			/* 念のため */

	/* vram0 */
	sceGsSetDefLoadImage( &li,
						 BUFFER_PAGE(0)/64, DRAW_WIDTH/64,
						 FRAME_BUFFER_COLOR_MODE(),
						 0, DRAW_HEIGHT, 256, 16 );
	FlushCache(0);
	sceGsExecLoadImage( &li, (void*)image );
	sceGsSyncPath( 0, 0 );

	/* vram1 */
	sceGsSetDefLoadImage( &li,
						 BUFFER_PAGE(1)/64, DRAW_WIDTH/64,
						 FRAME_BUFFER_COLOR_MODE(),
						 0, DRAW_HEIGHT, 256, 16 );
	FlushCache(0);
	sceGsExecLoadImage( &li, (void*)( image + ( 256 * 16 ) ) );
	sceGsSyncPath( 0, 0 );

	GV_Free( image );
}
#endif

/* ---------------------------------------------------------------- */
int NewVramInitialize( void )
{
#if 0
#ifdef NTSC
	LoadImageMessage();
#endif
#endif
	return ( 0 );
}
