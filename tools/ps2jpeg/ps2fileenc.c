/*
	PS2FILE.C
		T10000からCD上のファイルを読み込む
	2000/05/09	K.Uehara
	$Id: ps2fileenc.c,v 1.1 2001/05/07 01:54:14 usr04761 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>
#include <libdma.h>
#include <sifdev.h>
#include <sifrpc.h>
#include <libcdvd.h>

/* ---------------------------------------------------------------------- */
/*
	メインルーチン
*/

int main( int argc, char *argv[] )
{
     float 	qn[2];
     char 	hdnamein[ 128 ], hdnameout[ 128 ];
     int  	fp,fout;
     int 	size;
     int 	width,height;
         
    //元画像のメモリ確保
    static unsigned char image[448][512*4];
    //符号データメモリ確保
    static unsigned char code[448][512*2];

    sceCdInit( SCECdINIT );

    width =atoi( argv[2]);	/* X_SIZE */
    height =atoi( argv[3]);	/* Y_SIZE */
    qn[0] = atof( argv[5]);	/* y画像圧縮率 */
    qn[1] = atof( argv[6]);	/* c画像圧縮率 */

    sprintf( hdnamein, "host:./%s", argv[1] );
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

    sceRead( fp, image ,4*width*height );
    size = cdct_huff(image ,width,height , code , qn );
    sceWrite( fout, code , size+ 2*sizeof(int)  );
    //ファイル閉じる
    sceClose(fp);
    sceClose(fout);
    printf("end\n");
    return  size + 2 * sizeof(int);
}
