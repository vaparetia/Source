/*  jpeg画像から raw画像へ  エンコード  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char ** argv)
{
    static unsigned char	*image,*code ;
    FILE			*fp, *fout;
    float			qn[2];		/*Y画像*//*CR，CB画像*/
    int				out;		// 符号化データのメモリ数
    int				width , height;
    
    fp = fopen(argv[1],"rb");
    if ( fp == NULL ) {
	fprintf(stderr,"ファィルオープンできない。(%s)\n",argv[1]) ;
	return -1;
    }    
    fout = fopen(argv[4],"wb");
    width =atoi( argv[2]);	/* X_SIZE */
    height =atoi( argv[3]);	/* Y_SIZE */
    qn[0] = atof( argv[5]);	/* y画像圧縮率 */
    qn[1] = atof( argv[6]);	/* c画像圧縮率 */
    //元画像のメモリ確保
    image = (unsigned char *)malloc(width * height *4);
    //符号データメモリ確保
    code  = (unsigned char *)malloc(width * height * 2);
	    
    fread (image , 4 , width * height , fp);

    out = cdct_huff(image ,width , height , code , qn);
    fwrite (code , 1 , out + 2*sizeof(int)  ,fout);

    
    
    fclose(fp);
    fclose(fout);
    
    //メモリ解放
    free(image);
    free(code);

    return out + 2 * sizeof(int);
}
