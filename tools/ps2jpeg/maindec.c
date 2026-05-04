/* raw画像から jpeg画像へ デコード */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char ** argv)
{
    static unsigned char *image,*code ;
    FILE *fp, *fout;
    int out,in;		// 符号化データのメモリ数
    int width ,height ;
    
    fp = fopen(argv[1],"rb");
    if ( fp == NULL ) {
	fprintf(stderr,"ファィルオープンできない。(%s)\n",argv[1]) ;
	return -1;
    }    
    fout = fopen(argv[4],"wb");
    width =atoi( argv[2]);	/* X_SIZE */

    height =atoi( argv[3]);	/* Y_SIZE */

        //元画像のメモリ確保
    image = (unsigned char *)malloc(width * height *4);
    //符号データメモリ確保
    code  = (unsigned char *)malloc(width * height * 2);

     memset(image , 0 , width*height*4 );
        memset(code , 0 , width*height*2 );
	    
    fseek(fp,0,SEEK_END);
    in = ftell(fp);
    fseek(fp,0,SEEK_SET);

	fread (code ,1 , in ,fp);
	out = cdct_huff(image, width , height ,code );
	fwrite (image,4, width * height ,fout);

    fclose(fp);
    fclose(fout);

    //メモリ解放
    free(image);
    free(code);

    return 4 * width * height;
}
