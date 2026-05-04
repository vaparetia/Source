/* デジカメ画像保存用サムネイル */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <math.h>

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>
#include <libdma.h>
#include <sifdev.h>
#include <sifrpc.h>
#include <libcdvd.h>

#include <libgraph.h>
#include <libdev.h>
#include <libvu0.h>

#define T_WIDTH  48
#define T_HEIGHT 36

void SetDisplayMode( int width, int height, int offset, int col_mode, int v_zoom, int page_offset ){
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
    
    sceGsSyncPath( 0, 0 );
    sceGsSetDefLoadImage( &li,
			  0, 512/64,
			  2,
			  (512-width)/2, (448-height)/2, width, height );
    FlushCache(0);
    sceGsExecLoadImage( &li, image );
    sceGsSyncPath( 0, 0 );
    
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


thumbnail[T_HEIGHT][T_WIDTH*2]  __attribute__((aligned(64))) ;

int main(int argc , char ** argv)
{
    int                   *fin , *fout ;
    static unsigned char  image[448][512*4] ;
    int  width , height ;
    char 	hdnamein[ 128 ], hdnameout[ 128 ];

    static unsigned char zero[448][512*4] = { 0 };//VRAMに0を食べさせるための配列

    sceCdInit( SCECdINIT );    

    sprintf( hdnamein, "host0:./%s", argv[1] );
    sprintf( hdnameout, "host0:./%s", argv[4] );
    printf( "FROM %s TO %s\n", hdnamein, hdnameout );
    if( ( fin = sceOpen( hdnamein, SCE_RDONLY ) ) < 0 ){
	printf( "ERROR:FILE (%s) OPEN ERROR\n", hdnamein );
	return -1;
    }
    if( ( fout = sceOpen( hdnameout, SCE_WRONLY | SCE_CREAT ) ) < 0 ){	
	printf( "ERROR:FILE %s OPEN ERROR\n", hdnameout );
	return -1;
    }

    width  = atoi( argv[2]);	/* 元画像のX_SIZE */
    height = atoi( argv[3]);	/* 元画像のY_SIZE */
    
    //画像読み込み
    sceRead ( fin , image , width * height * 4 ); 

    //縮小
    transform( image , width , height , thumbnail , T_WIDTH , T_HEIGHT );

    sceWrite ( fout , thumbnail ,  T_WIDTH * T_HEIGHT * 4 );

    //画像表示
    /* ＰＳ２初期化 */
    sceDevVif0Reset();
    sceDevVif1Reset();
    sceDevVu0Reset();
    sceDevVu1Reset();
    sceGsResetPath();
    sceDmaReset( 1 );

    /* グラフィック初期化 */
    sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FIELD);

    /* 画面モードの設定 */
    SetDisplayMode( 512, 448, 0, 1, 0, 0 );

    initLoadImage( zero );//VRAMに、0を吐いておく

    LoadImage( thumbnail  , T_WIDTH , T_HEIGHT );    
    
    sceClose(fin);
    sceClose(fout);
    
    return 0;
}

