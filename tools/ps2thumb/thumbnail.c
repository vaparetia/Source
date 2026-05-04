/* デジカメ画像保存用サムネイル */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T_WIDTH  48
#define T_HEIGHT 32

int main(int argc , char ** argv)
{
    FILE                  *fin , *fout ;
    static unsigned char  *image , *thumbnail ;
    int  width , height ;

    fin = fopen(argv[1],"rb");
    if ( fin == NULL ) {
	fprintf(stderr,"ファィルオープンできない。(%s)\n",argv[1]) ;
	return -1;
    }    
    fout = fopen(argv[4],"wb");
    if ( fout == NULL ) {
	fprintf(stderr,"ファィルオープンできない。(%s)\n",argv[4]) ;
	return -1;
    }    
    width  = atoi( argv[2]);	/* 元画像のX_SIZE */
    height = atoi( argv[3]);	/* 元画像のY_SIZE */
    
    //元画像のメモリ確保
    image = (unsigned char *)malloc(width * height *4);
    //サムネイルのメモリ確保
    thumbnail  = (unsigned char *)malloc(T_WIDTH * T_HEIGHT * 4) ;

    //画像読み込み
    fread (image , 4 , width * height , fin); 

    //縮小
    transform( image , width , height , thumbnail , T_WIDTH , T_HEIGHT );
    fwrite ( thumbnail , 4 , T_WIDTH * T_HEIGHT , fout );

    fclose(fin);
    fclose(fout);
    
    //メモリ解放
    free(image);
    free(thumbnail);

    return 0;
}
