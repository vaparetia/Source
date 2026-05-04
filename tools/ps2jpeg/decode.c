#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define PI 3.1415926f
#define Y 0
#define C 1

void dump( char *top, int len )
{
	int i;
	unsigned char *p;

	p = ( unsigned char * )top;

	printf( "%08X: ", p );

	for( i = 0; i < len; i++ ){
		printf( "%02X ", *p );
		p++;
		if( i % 16 == 15 ){
			printf( "\n%08X: ", p );
		}
	}
	printf( "\n" );
}

/* qt.c */
static void dct_iqt(int q[8][8],float g[8][8],float qn);

/* jpeg.c */
int cdct_huff(unsigned char *image,int width , int height ,
	       unsigned char *code );
static void yc_rgb(unsigned char  *image , int width , int height);
//static void image_clear_color(unsigned char *image);


/* mono.c */
static int dct_huff(unsigned char *image,int width , int height ,
		    unsigned char *code ,  int ycflag);
static int dct_huff_decode(unsigned char *code,int pc_dc[16],int bits_dc[16],
		   int pc_ac[16][16], int bits_ac[16][16],
		   unsigned char *image ,float qn , int ycflag ,
			   int width ,int height);
static int add_load(unsigned char *buf, int *nbyte, int *nbit, int bits);

/* huff.h  */
static int huffman_load(char buf[],int *nbyte, int *nbit, int code[],
		 int bits[], int n);

/* cosin.h   */
static void idct(float g[8][8], float f[8][8]);
static int byte_copy(unsigned char *ps,unsigned char *pd, int n);
extern void tablemk(float cos[8][8] ,float T_cos[8][8]);

extern float 	c[8][8], ct[8][8];
extern int     	scan[64] ;
extern int 	q_table[8][8];

/*****************************************************************************
                     cdct_huff     DCTとハフマン符合化
imageyc:    画像データ
code:       符合データ
qn:         量子化テーブルの乗数
fname:      符合データのファイル名
mode:       1:符合化 ， 2:復号
*****************************************************************************/
int cdct_huff(unsigned char *image , int width , int height ,
	      unsigned char *code )
{
    int nbyte0, nbyte1;


    tablemk(c, ct);
    //codeをYとCにわける。
    nbyte0 = *((int*)(code));
    nbyte1 = *((int*)(code + sizeof(int)));
    memmove (code , code +sizeof(int)*2 , nbyte0  );
    memmove (code + width*height , code +sizeof(int)*2 + nbyte0 , nbyte1);
    nbyte0 = dct_huff(image ,width , height , code , Y);
    nbyte1 = dct_huff(image ,width , height , code + width*height ,C);
    if(( nbyte0 != -1) && (nbyte1 != -1)){
	yc_rgb(image , width , height);
	return 0;
    }
    else {
	printf("デコード失敗\n");
	return -1;
    }
}

/******************************************************************************
         yc_rgb    Y，CR，CB画像から，RGB画像に変換
image_y:	Y画像データ
image_c:	CR,CB画像データ
image:		RGB画像
******************************************************************************/
static void yc_rgb(unsigned char *image ,int width , int height)
{
    int i, j ;
    int  r, g, b, y, cr, cb;
    int ccr[2][width+1],ccb[2][width+1];

    for (i =0; i< height ; i++){
	    for (j = 0; j < width ; j++) {
		y = *(image + i*width*4 + j*4 );
		if( (!(i % 2)) ){//偶数行のとき
		    if( (!(j%2)) ){//偶数列
			cr = (*(image + i*width*4 + j*4+1  ) -128) ;
			cb = (*(image + i*width*4 + j*4+2  ) -128) ;
			ccr[1][j] = (*(image + (i+2)*width*4 + j*4+1  ) -128) ;
			ccb[1][j] = (*(image + (i+2)*width*4 + j*4+2  ) -128) ;
			ccr[0][j+2] = (*(image + i*width*4 + (j+2)*4+1  ) -128) ;
			ccb[0][j+2] = (*(image + i*width*4 + (j+2)*4+2  ) -128) ;
			ccr[1][j+2] = (*(image + (i+2)*width*4 + (j+2)*4+1  ) -128) ;
			ccb[1][j+2] = (*(image + (i+2)*width*4 + (j+2)*4+2  ) -128) ;
			cb = (*(image + i*width*4 + j*4+2  ) -128) ;
			ccr[0][j] = cr;
			ccb[0][j] = cb;
		    }else {//奇数列
			if(j == width-1){//右端の処理
			    cr = ccr[0][j-1];
			    cb = ccb[0][j-1];
			}
			cr = (ccr[0][j-1]+ccr[0][j+1])/2;
			cb = (ccb[0][j-1]+ccb[0][j+1])/2;
		    }
		}
		else{ //奇数行の時
		    if( j%2 ){ // 奇数列
			if(j == width-1 || i == height -1){//右端と、下端の処理
			    cr = ccr[0][j-1];
			    cb = ccb[0][j-1];
			}
			/*			if( j == 0 ){ //左端の処理
			    cr = ccr[0][j+1];
			    cb = ccb[0][j+1];
			    }*/
			
			cr = ( ccr[0][j-1] + ccr[1][j+1] + ccr[0][j+1] + ccr[1][j-1] )/4 ;
			cb = ( ccb[0][j-1] + ccb[1][j+1] + ccb[0][j+1] + ccb[1][j-1] )/4 ;
		    }else {//偶数列
			if(j == width-1 || i == height -1){//下端の処理
			    cr = ccr[0][j-1];
			    cb = ccb[0][j-1];
			}
			cr = ( ccr[0][j] + ccr[1][j] )/2 ;
			cb = ( ccb[0][j] + ccb[1][j] )/2 ;
		    }
		}
		r = (int)(cr /0.713f + y);
		b = (int)(cb /0.564f + y);	    
		g = (int)((y - 0.3f*r - 0.11f*b) /0.59f);
		if (r <	0) {r = 0;}
		if (r > 255) {r = 255;}
		if (g < 	0){ g =   0;}		
		if (g > 255){ g = 255;}		
		if (b <  	0){ b =   0;}	
		if (b > 255){ b = 255;}
#if 0
		*(image + i*width*4 + j*4   ) = y;
		*(image + i*width*4 + j*4+1 ) = 0;
		*(image + i*width*4 + j*4+2 ) = 0;
#else
		*(image + i*width*4 + j*4   ) = r;
		*(image + i*width*4 + j*4+1 ) = g;
		*(image + i*width*4 + j*4+2 ) = b;
#endif
		*(image + i*width*4 + j*4+3 ) = 255;
	    }
    }
}

/*****************************************************************************
		dct_huff	DCT+ハフマン符号(モノクロ)
image:	画像データ
code:	符号データ
qn:	量子化テーブルの乗数
nbyte:	符号データのバイト数
mode:	1:符号s 2:復号
*****************************************************************************/
static int dct_huff(unsigned char *image, int width , int height ,
		    unsigned char *code, int ycflag)
{
    static float prb_dc[16], prb_ac[16][16];		/* シンボルの出現確率	*/
    static int pc_dc[16], pc_ac[16][16];		/* ハフマン符号		*/
    static int bits_dc[16], bits_ac[16][16];		/* ハフマン符号の符号長 */
    float qns;					        /* 量子化テーブルの乗数	*/
    int posi, n_prb_dc,n_prb_ac, n_pc_dc, n_pc_ac;
    int n_bits_dc, n_bits_ac, n_qns;

    int a , b;
    n_prb_dc = sizeof(prb_dc);
    n_prb_ac = sizeof(prb_ac);
    n_pc_dc = sizeof(pc_dc);
    n_pc_ac = sizeof(pc_ac);
    n_bits_dc = sizeof(bits_dc);
    n_bits_ac = sizeof(bits_ac);
    n_qns = sizeof(qns);
    
	posi = 0;
	a = byte_copy((char *)&code[posi], (char *)&prb_dc[0], n_prb_dc);
	posi += a; b += n_prb_dc;
	//printf("prb_dc\n");dump( prb_dc , n_prb_dc);
	a = byte_copy((char *)&code[posi], (char *)&prb_ac[0][0], n_prb_ac);
	posi += a; b += n_prb_ac;
	//printf("prb_ac\n");dump( prb_ac , n_prb_ac);
	a = byte_copy((char *)&code[posi], (char *)&pc_dc[0], n_pc_dc);
	posi += a; b += n_pc_dc;
	//printf("pc_dc\n");dump( pc_dc , n_pc_dc);
	a = byte_copy((char *)&code[posi], (char *)&pc_ac[0][0], n_pc_ac);
	posi += a; b += n_pc_ac;
	//printf("pc_ac\n");dump( pc_ac , n_pc_ac);
	a = byte_copy((char *)&code[posi], (char *)&bits_dc[0], n_bits_dc);
	posi += a; b += n_bits_dc;
	//printf("bits_dc\n");dump( bits_dc , n_bits_dc);
	a = byte_copy((char *)&code[posi], (char *)&bits_ac[0][0], n_bits_ac);
	posi += a; b += n_bits_ac;
	//printf("bits_ac\n");dump( bits_ac , n_bits_ac);
	a = byte_copy((char *)&code[posi], (char *)&qns, n_qns);
	posi += a ; b += n_qns;
	//printf("qns\n");dump( &qns , n_qns);
	//printf("(%f)\n",qns );
	if (qns <= 0.0f) {
	    printf("qnsが0以下だよ。(%f)(%d)\n",qns ,bits_ac[15][15]);
	    return -1;
	}
	return dct_huff_decode(&code[posi], pc_dc,
			       bits_dc, pc_ac, bits_ac,image, qns ,ycflag,
			       width , height);

}


/****************************************************************************
		dct_huff_decode	DCT+ハフマン符号化による複号化
image:	画像データ
code:	符号データ用メモリ
qn:	量子化テーブルの乗数
pc_dc:	ハフマン符号と符号長
bits_dc:  (直流成分)
pc_ac:	ハフマン符号と符号長
bits_ac:  (直流成分)
*****************************************************************************/
static int dct_huff_decode(unsigned char *code , int pc_dc[16],int bits_dc[16],
		   int pc_ac[16][16], int bits_ac[16][16],
		   unsigned char *image , float qn , int ycflag ,
		   int width ,int height)
{
    int i,j, k, xs, ys, run, gno, pre;
    float f[8][8],g[8][8];
    int q[8*8];
    int kbyte;
    int kbit;
    unsigned char *p;
    int step ,istep , yys , cell ;

    int HEIGHT;
    //
    if (ycflag == C){
	HEIGHT = height/2;
    }else{
	HEIGHT = height ;
    }
    //
    
    kbyte = kbit = 0;
    pre = (int) (64/qn);
    for (ys = 0 ; ys < HEIGHT ; ys += 8){
	for (xs = 0; xs < width ; xs += 8){
	    gno = huffman_load(code,&kbyte,&kbit,pc_dc,bits_dc,16);
	    if(gno == -1){return -1;}
	    if(gno != 0){
		i = add_load(code ,&kbyte,&kbit,gno);
		if( i< (1 << (gno -1))) { i += -(( 1 << gno ) -1 );}
	    }
	    else {
		i = 0;
	    }
	    q[0] = i;
	    for (k=1; k<64 ;){
		i = huffman_load(code , &kbyte, &kbit, pc_ac,
				 (int *)bits_ac, 16*16);
		if (i == -1) {
		    return -1;
		}
		if ( i == 0){
		    while (k < 64 ){
			q[scan[k++]] = 0;
		    }
		}
		else if (i == 15*16){
		    for (j =0; j <16; j++){
			q[scan[k++]] = 0;
		    }
		}
	    
		else {
		    run = i/16; gno = ( i % 16);
		    i = add_load(code , &kbyte, &kbit, gno);
		    if (run > 0)
			for (j=0; j < run; j++) q[scan[k++]] = 0;
		    if (i < ( 1 << (gno -1))) i += - (( 1 << gno )-1);
		    q[scan[k++]] = i;
		}
	    }
	
	    q[0] += pre;
	    pre = q[0];
	    if(xs == width-8 && ys == HEIGHT-8){
		//printf("最後のブロック\n");
		//dump(q ,64*4);
	    }
	    
	    dct_iqt((int (*)[8])q, g, qn);
	    idct(g,f);
	    if(ycflag == Y){
		p = image + ys *width*4 + xs*4;
		step =4;
		istep = 1;
	    } else {
		step = 8 ;
		istep = 2;
		if(ys < height/4){
		    yys = ys;
		    cell =1;
		} else {
		    yys = ys - height/4;
		    cell = 2;
		}
		p = image + (xs % (width/2))*8 + width*4*(4*yys + 2*(xs/(width/2))*8)+cell;
	    }
	    for(i=0 ; i < 8 ; i++){
		for(j=0 ; j <8 ; j++){
		    if (f[i][j] >255) f[i][j] =255;
		    if (f[i][j] <  0) f[i][j] =  0;
		    *p = (int)(f[i][j] +0.5f);
		    p = p + step;
		}
		p = p + width*4*istep - 8*step ;
	    }
	}
    }
    return 0;
}

/****************************************************************************
		add_load	メモりから付加データを読み込む
buf:	符号データメモリ
nbyte:	メモリ内のバイト位置を指すポインタ
nbit:	バイト内のビット位置を指すポインタ
bits:	付加データのビット数
****************************************************************************/
static int add_load(unsigned char *buf, int *nbyte, int *nbit, int bits)
{
    int i,k;

    k = 0;
    for (i= bits-1; i >= 0; i--){
	if (buf[*nbyte] & (1 << *nbit)) k |= 1<<i;
	*nbit += 1;
	if (*nbit == 8){
	    *nbyte += 1; *nbit =0;
	}
    }
    return k;
}
    
/***************************************************************************
		dct_iqt		DCT係数の復号
g:	入力DCT係数
q:	出力量子化符号データ
qn:	量子化テーブルの乗数
******************************************************************************/
static void dct_iqt(int q[8][8],float g[8][8],float qn)
{
    int i,j;

    for ( i=0; i<8 ; i++)
	for ( j=0; j<8 ; j++)
	    g[i][j] = (int)( q[i][j] * (q_table[i][j]*qn) );
}

/****************************************************************************
		idct	離散コサイン変換
f:	入力画像配列
g:	出力コサイン関数
*****************************************************************************/
static void idct(float g[8][8], float f[8][8])
{
    int i , j , k;
    float temp1 , temp[8][8];

    for ( i=0; i < 8 ; i++){
	for ( j = 0 ; j < 8 ; j++){
	    temp[i][j] = 0.0f;
	    for( k=0 ; k < 8 ; k++ ){
		temp[i][j] += g[i][k] *c[k][j];
	    }
	}
    }

    for (i=0 ; i < 8 ; i++){
	for ( j = 0 ; j < 8 ; j++){
	    temp1 = 0.0f ;
	    for( k=0 ; k < 8 ; k++ ){
		temp1 += ct[i][k] * temp[k][j] ;
	    }
	    temp1 += 128.0f;
	    f[i][j] = temp1;
	}
    }
}

/***************************************************************************
		huffman_load	ハフマン符号をメモリから読み出す
buf:	メモリ
nbyte:	メモリ内のバイト位置を指すポインタ
nbits:	バイト内のメモリ位置を指すポインタ
code:	ハフマン符号
bits:	ハフマン符号の符号長
n:	シンボル数
******************************************************************************/
static int huffman_load(char buf[],int *nbyte, int *nbit, int code[],
		 int bits[], int n)
{
    int kbyte;
    int i,j,kbit,flg1,flg2;

    for(i=0; i<n; i++){
	if (bits[i] == 0) {
	    continue;
	}
	kbyte = *nbyte; kbit = *nbit;
	for (j = bits[i]-1; j >= 0; j--){
	    if (buf[kbyte] & (char)(1 << kbit)) {flg1 = 1;}
	    else				{flg1 = 0;}
	    if (code[i] & ((int)1 << j))	{flg2 = 1;}
	    else				{flg2 = 0;}
	    if (flg1 ^ flg2) {break;}
	    kbit++;
	    if (kbit == 8) {
		kbit = 0; kbyte++;
	    }
	}
	if (j == -1) {break;}
    }
    if (i >= n){
	return -1;
    }
    *nbyte = kbyte; *nbit = kbit;
    return i;	/* シンボルの番号を返す */
}

/******************************************************************************
		image_clear_color	カラー画像データをクリアする
image:	入力画像ファイル
******************************************************************************/
/*static void image_clear_color(unsigned char *image)
{
    int i;

    for (i = 0 ; i< X_SIZE*Y_SIZE*2 ; i++){
		image[i] = 0;
    }
}*/

/*****************************************************************************
		byte_copy	データコピー
ps :	転送元のメモリのポインタ
pd:	転送先のメモリのポインタ
n:	転送するバイト数
*****************************************************************************/
static int byte_copy(unsigned char *ps,unsigned char *pd, int n)
{
    int  j , n_pd=0 , n_ps=0 ;

    //printf("%d \n", n);    
    while(1){
	if ( *ps == 0x00 ){//0の次の数字だけ0を打ち出す
	    //printf("loop%d\n" , ps[1]);
	    for(j = 0 ; j < ps[1] ; j++ ){
		*pd++ = 0x00 ;
		n_pd++ ;
		//printf("%d," ,n_pd);
	    }
	    ps += 2;
	    n_ps += 2;
	}else{
	    *pd++ = *ps++;
	    n_ps++;
	    n_pd++ ;
	    //printf("%d ,," , n_pd);
	}
        if(n_pd == n){//終了条件
	    //printf("\n");	    
	    return n_ps;
	}
    }
}

