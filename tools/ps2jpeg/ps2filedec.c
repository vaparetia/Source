/*
	PS2FILEDEC.C
		JPEG画像をRAW画像に変換して表示する
		                     YANO.YUKO
	$ID$
*/

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


/* ---------------------------------------------------------------------- */
/*                       メインルーチン                                   */
/* ---------------------------------------------------------------------- */


int main( int argc, char *argv[] )
{
    char 	hdnamein[ 128 ], hdnameout[ 128 ];
    int  	fp,fout;
    int  	size , jsize;
    int 	width, height;
    int i , j ;

 
    //元画像のメモリ確保
    //符号データメモリ確保
    static unsigned char image[448][512*4] __attribute__((aligned(64)));
    static unsigned char code[448][512*4];

    sceCdInit( SCECdINIT );

    width = atoi( argv[2]);	/* X_SIZE */
    height= atoi( argv[3]);	/* Y_SIZE */

    sprintf( hdnamein, "host0:./%s", argv[1] );
    sprintf( hdnameout, "host0:./%s", argv[4] );
    printf( "FROM %s TO %s\n", hdnamein, hdnameout );
    if( ( fp = sceOpen( hdnamein, SCE_RDONLY ) ) < 0 ){
	printf( "ERROR:FILE (%s) OPEN ERROR\n", hdnamein );
	return -1;
    }
    if( ( fout = sceOpen( hdnameout, SCE_WRONLY | SCE_CREAT ) ) < 0 ){	
	printf( "ERROR:FILE %s OPEN ERROR\n", hdnameout );
	return -1;
    }

    jsize = sceLseek( fp , 0 , SCE_SEEK_END);
    sceLseek( fp , 0, SCE_SEEK_SET);
printf("サイズ%d\n" , jsize );
    sceRead( fp, code , jsize );
    size = cdct_huff(image ,width, height, code );
    sceWrite( fout, image , width*height*4 );

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

    SetDisplayMode( 512, 448, 0, 0, 0, 0 );

    //VRAMに、0を吐いておく
    for(i = 0 ; i < 448 ; i ++){
	for (j = 0 ; j < 512*4 ; j ++){
	    code[i][j] = 0 ;
	}
    }
    initLoadImage( code );

    LoadImage( image  , width , height );    
    
    //ファイル閉じる
    sceClose(fp);
    sceClose(fout);


    return width*height*4 ;
}
