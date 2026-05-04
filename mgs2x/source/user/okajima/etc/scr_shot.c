/*スクリーンショットをファイルに落とすルーチンスクリーンショットをファイルに落とすルーチンスクリーンショットをファイルに落とすルーチン
	scr_shot.c
	スクリーンショットをファイルに落とすルーチン
	1999/09/31 S.Okajima
	$Id: scr_shot.c,v 1.1.1.3 2002/11/19 11:47:42 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"libdg.h"


#define sceOpen( a, b )		pcOpen( a, b )
#define sceRead( a, b, c )	pcRead( a, b, c )
#define sceWrite( a, b, c ) pcWrite( a, b, c )
#define sceClose( a )		pcClose( a )
#define sceLseek( a, b, c )	pcLseek( a, b, c )

extern int sceClose( int fd );
extern int sceWrite( int fd, void *buffer, int size );
extern int sceOpen( char *filename, int mode );

sceGsStoreImage ok_gs_simage ALIGN16 ;

static void StoreImage(unsigned char *filename){


	u_long128 *dst1;
	int fd, i;

	printf("%s\n",filename);

	// to create image file. 

	fd = sceOpen( filename , SCE_WRONLY | SCE_TRUNC | SCE_CREAT);
//	fd = sceOpen( filename , SCE_RDWR | SCE_TRUNC | SCE_CREAT);
	dst1 = (u_long128*) GV_Malloc(DRAW_WIDTH * DRAW_HEIGHT * 4);


	sceGsSetDefStoreImage(&ok_gs_simage, BUFFER_PAGE(DG_Clock), DRAW_WIDTH / 64, FRAME_BUFFER_COLOR_MODE(), 0, 0, DRAW_WIDTH, DRAW_HEIGHT);
	FlushCache(0);
	while ( DG_DmaCheckEnd() == 0 );
	sceGsSyncPath( 0, 0 );
	sceGsExecStoreImage(&ok_gs_simage, dst1);


#ifndef HIGHRESO_FFI
	for(i = 0; i < DRAW_HEIGHT; i++) {
		sceWrite(fd, dst1 + i * DRAW_WIDTH / 4, DRAW_WIDTH * 4);
	}
#else
	for(i = 0; i < DRAW_HEIGHT; i++) {
		sceWrite(fd, dst1 + i * DRAW_WIDTH / 8, DRAW_WIDTH * 2);
	}
#endif

	GV_Free(dst1);
	sceClose(fd);

	printf("Finished !!\n");

	return;
}



void SaveScreenShot( unsigned char *filename )
{
	printf("Capturing...\n");
	StoreImage( filename );
}
