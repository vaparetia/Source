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

void initLoadImage( char *image );


void SetDisplayMode( int width, int height, int offset,
		     int col_mode, int v_zoom, int page_offset ){
	u_long	dispfb ;
	u_long	display ;

	switch ( col_mode ){
	  case 0:
		col_mode = SCE_GS_PSMCT32 ;
		break ;
	  case 1:
		col_mode = SCE_GS_PSMCT16 ;
		break ;
	  case 2:
		col_mode = SCE_GS_PSMCT16S ;
		break ;
	  case 3:
		col_mode = SCE_GS_PSMZ32 ;
		break ;
	  case 4:
		col_mode = SCE_GS_PSMZ16 ;
		break ;
	  case 5:
		col_mode = SCE_GS_PSMZ16S ;
		break ;
	}
	offset = ( width / 64 ) * offset + page_offset ;
	if ( offset >= ( 1024*1024/2048 ) ) offset = 1024*1024/2048 - 1 ;
	offset &= 511 ;
	dispfb = (col_mode<<15)|((width>>6)<<9)|(offset);
	display = ( (unsigned long)(height*1-1) << 44 ) | ((unsigned long)0x9ff<<32)
	  | ((( 2560+width-1)/width-1) << 23 ) | ((50)<<12) | (0x28c+(0*(2560/width) | (v_zoom<<27) )) ;

	/* PMODE */
	*(unsigned long*)0x12000000 = 0x000000000000ff21 ;
	/* PMODE2 */
	*(unsigned long*)0x12000020 = 0x03 ;
	/* PMODE2 */
	*(unsigned long*)0x12000020 = 0x01 ;
	/* DISPFB1 */
	*(unsigned long*)0x12000070 = dispfb ;
	/* DISPLAY1 */
	*(unsigned long*)0x12000080 = display ;
	/* BGCOLOR */
	*(unsigned long*)0x120000e0 = 0 ;
}
/* 幅高さを指定してデータをロード */
void LoadImage( char *image, int width, int height )
{
    sceGsLoadImage	li ;
    int i;
    
    sceGsSyncPath( 0, 0 );
    for ( i = 0 ; i < height ; i += 32 ){
    sceGsSetDefLoadImage( &li,
			  0, 512/64,
			  0,
			  (512-width)/2, (448-height)/2 + i, width , 32 );
    FlushCache(0);
    sceGsExecLoadImage( &li, image + i * width * 4 );
    sceGsSyncPath( 0, 0 );
    }
}


void initLoadImage( char *image )
{
	sceGsLoadImage	li ;
	int				i ;

	sceGsSyncPath( 0, 0 );
	for ( i = 0 ; i < 448 ; i += 32 ){
		sceGsSetDefLoadImage( &li,
							  0, 512/64,
							  0,
							  0, i, 512, 32 );
		FlushCache(0);
		sceGsExecLoadImage( &li, image + i * 512 * 4 );
		sceGsSyncPath( 0, 0 );
	}

}

