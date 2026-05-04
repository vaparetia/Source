//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef KP_XBOX
#include <libgraph.h>
#endif

#include "mgs_type.h"
#include "libgv.h"
#include "libdg.h"

#include "jpeg.h"


static int *bigbuff , *bigbuff_top;

#if 0
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
#endif

/* qt.c */
static void dct_qt(float g[8][8], int q[8][8], float qn);

/* jpeg.c */
static int cdct_huff(unsigned char *image , int width , int height ,
	      unsigned char *code , float qn[2]);
static void rgb_yc(unsigned char  *image , int width , int height);

/*  prb.c */
static void prb_get(unsigned char *image ,int width , int height ,
		    float prb_dc[16], float prb_ac[16][16],
		    float qn ,int ycflag);
static void grp(int val, int *pgno);

/* mono.c */
static int dct_huff(unsigned char *image , int width , int height ,
		    unsigned char *code,float qn, int ycflag );
static int dct_huff_code(unsigned char *image ,int width , int height ,
			 unsigned char *code , int pc_dc[16],
			 int bits_dc[16], int pc_ac[16][16],
		         int bits_ac[16][16], float qn , int ycflag);
static int byte_copy(unsigned char *ps , unsigned char *pd , int n);

/* huff.h  */
static int huffman_get(float prb[], int n, int pc[], int bits[]);
static int huffman_save(char buf[], int *nbyte, int *nbit, int code, int bits);

/* cosin.h   */
static void dct(float f[8][8], float g[8][8]);
extern void tablemk(float cos[8][8] ,float T_cos[8][8]);

static int calc(int width ,int height , int xs ,int ys ,int ycflag , unsigned char *image  ,float  f[8][8] );

static void block_copy(int  *image , int* q );
/*void image_clear_color(unsigned char *image , int width , int height);*/

extern float 	c[8][8], ct[8][8] ;
extern int    	scan[64] ;
extern int 	q_table[8][8] ;

float q_table_inv[8][8] = { /*量子化テーブル */
    {  1.0f/16,	1.0f/11,	1.0f/10,	1.0f/16,	1.0f/24,	1.0f/40,	1.0f/51,	1.0f/61  },
    {  1.0f/12,	1.0f/12,	1.0f/14,	1.0f/19,	1.0f/26,	1.0f/58,	1.0f/60,	1.0f/55  },
    {  1.0f/14,	1.0f/13,	1.0f/16,	1.0f/24,	1.0f/40,	1.0f/57,	1.0f/69,	1.0f/56  },
    {  1.0f/14,	1.0f/17,	1.0f/22,	1.0f/29,	1.0f/51,	1.0f/87,	1.0f/80,	1.0f/62  },
    {  1.0f/18,	1.0f/22,	1.0f/37,	1.0f/56,	1.0f/68,	1.0f/109,	1.0f/103,	1.0f/77  },
    {  1.0f/24,	1.0f/35,	1.0f/55,	1.0f/64,	1.0f/81,	1.0f/104,	1.0f/113,	1.0f/92  },
    {  1.0f/49,	1.0f/64,	1.0f/78,	1.0f/87,	1.0f/103,	1.0f/121,	1.0f/120,	1.0f/101 },
    {  1.0f/72,	1.0f/92,	1.0f/95,	1.0f/98,	1.0f/112,	1.0f/100,	1.0f/103,	1.0f/99  }
    };

/*****************************************************************************
                     cdct_huff     DCTとハフマン符合化
image:    画像データ
code:       符合データ
qn:         量子化テーブルの乗数
*****************************************************************************/
static int cdct_huff(unsigned char *image , int width , int height ,
	      unsigned char *code , float qn[2])
{
    int nbyte0, nbyte1;
    unsigned char *codebody;
	int			time ;

    bigbuff  = bigbuff_top ;
    

    tablemk(c, ct);

    if (qn[0] < 0.01f || qn[1] < 0.01f) {
        printf("qnちいさすぎ。%f ,%f\n",qn[0],qn[1]);
	return -1;
    } 
    rgb_yc( image , width , height);

    codebody = code + sizeof( int ) * 2;
    nbyte0 = dct_huff(image , width , height ,
		      (unsigned char *) codebody , qn[0] , Y );
//printf( "Y:qn = %f nbyte = %d\n", qn[ 0 ], nbyte0 );
    nbyte1 = dct_huff(image ,width , height ,
		      (unsigned char *)( codebody + nbyte0 ),qn[1], C );
//printf( "C:qn = %f nbyte = %d\n", qn[ 1 ], nbyte1 );
    //printf(" %d , %d \n", nbyte0 ,nbyte1);
    //Yのcodeと、Cのcodeをcodeにまとめる。
    *((int*)(code)) = nbyte0;
    *((int*)(code + sizeof(int))) = nbyte1;
    //printf("cdct_huff end\n");

    if ((nbyte0 != -1) && (nbyte1 != -1)) {
	return nbyte0 + nbyte1 ;
    }else {
	printf("不正終了 dct_huff変");
	return -1;
    }
}

/******************************************************************************
         rgb_yc      RGB画像から Y，CR，CB画像を作る
image:		RGB画像データ
image_y:	Y画像データ
image_c:	CR,CB画像データ
******************************************************************************/
static void rgb_yc(unsigned char  *image , int width , int height)
{
#if 0
    int i,j ;
    int r,g,b,y,cr,cb;

#if BLEND
    int r1 ,r2 ,r3 ,g1, g2 ,g3 ,b1,b2,b3 ;
#endif
    
    for (i = 0 ; i < height ; i++){
	for (j = 0; j < width ; j++){
	    r = *(image + i*width*4 + j*4)   ;
	    g = *(image + i*width*4 + j*4+1) ;
	    b = *(image + i*width*4 + j*4+2) ;
	    y = (int)(0.3f * r + 0.59f * g + 0.11f * b);
	    // *(image + i*width*4 + j*4)  = y;
	    *(image + i*width*4 + j*4)  = y;
	    if (  (!(j % 2)) && (!(i % 2))  ){
#if BLEND
		r1 =  *(image + i*width*4 + (j+1)*4);
		r2 =  *(image + (i+1)*width*4 + j*4);
		r3 =  *(image + (i+1)*width*4 + (j+1)*4);
		r = (r + r1 + r2 + r3)/4 ;
		    
		g1 =  *(image + i*width*4 + (j+1)*4 +1);
		g2 =  *(image + (i+1)*width*4 + j*4 +1);
		g3 =  *(image + (i+1)*width*4 + (j+1)*4 +1);
		g = (g + g1 + g2 + g3 )/4 ;
		
		b1 =  *(image + i*width*4 + (j+1)*4 +2);
		b2 =  *(image + (i+1)*width*4 + j*4 +2);
		b3 =  *(image + (i+1)*width*4 + (j+1)*4 +2);
		b = (b + b1 + b2 + b3)/4 ;
#endif	

		cr = (int)(( r - y)*0.713f + 128);
		cb = (int)(( b - y)*0.564f + 128);

		if (cr < 0){ cr =   0;}		
		if (cr > 255){ cr = 255;}		
		if (cb < 0){ cb =   0;}		
		if (cb > 255){ cb = 255;}
		*(image + i*width*4 + j*4+1 ) = cr;
		*(image + i*width*4 + j*4+2 ) = cb;
	    }
	}
    }
#else

	/* 高速化バージョン */
    int i,j ;
    int	r,g,b,y,cr,cb;
    for (i = 0 ; i < height ; i++){
		for (j = 0; j < width ; j++, image += 4 ){
#ifdef PSX2
			r = image[0] ;
			g = image[1] ;
			b = image[2] ;
#else
			b = image[0] ;
			g = image[1] ;
			r = image[2] ;
#endif
			//y = (int)(0.3f * r + 0.59f * g + 0.11f * b);
			y = ( r * 19660 + g * 38666 + b * 7208 ) >> 16 ;
			image[0] = y ;
			if (  (!(j % 2)) && (!(i % 2))  ){

				//cr = (int)(( r - y)*0.713f + 128);
				//cb = (int)(( b - y)*0.564f + 128);
				cr = (int)( ( ( ( r - y) * 46727 ) >> 16 ) + 128);
				cb = (int)( ( ( ( b - y) * 36962 ) >> 16 ) + 128);

				if (cr < 0){ cr =   0;}
				if (cr > 255){ cr = 255;}
				if (cb < 0){ cb =   0;}
				if (cb > 255){ cb = 255;}

				image[1] = cr ;
				image[2] = cb ;
			}
		}
    }

#endif
}

/*****************************************************************************
		dct_huff	DCT+ハフマン符号(モノクロ)
image:	画像データ
code:	符号データ
qn:	量子化テーブルの乗数
nbyte:	符号データのバイト数
mode:	1:符号 2:復号
*****************************************************************************/
static int dct_huff(unsigned char *image , int width , int height ,
		    unsigned char *code,float qn , int ycflag)
{
    float prb_dc[16], prb_ac[16][16];		/* シンボルの出現確率	*/
    int pc_dc[16], pc_ac[16][16];		/* ハフマン符号		*/
    int bits_dc[16], bits_ac[16][16];		/* ハフマン符号の符号長 */
    float qns;					/* 量子化テーブルの乗数	*/
    int posi, n_prb_dc,n_prb_ac, n_pc_dc, n_pc_ac;
    int n_bits_dc, n_bits_ac, n_qns;
    int kbyte;

    int popo;
    
    n_prb_dc = sizeof(prb_dc);
    n_prb_ac = sizeof(prb_ac);
    n_pc_dc = sizeof(pc_dc);
    n_pc_ac = sizeof(pc_ac);
    n_bits_dc = sizeof(bits_dc);
    n_bits_ac = sizeof(bits_ac);
    n_qns = sizeof(qns);
    
    posi = n_prb_dc + n_prb_ac + n_pc_dc + n_pc_ac
	+ n_bits_dc + n_bits_ac + n_qns;
    popo = posi;
    qns = qn ;
    prb_get(image ,width , height , prb_dc, prb_ac, qns, ycflag );
    huffman_get(prb_dc, 16 , pc_dc, bits_dc);
    huffman_get(( float *)prb_ac,16*16, (int *)pc_ac, (int *)bits_ac);

    kbyte = dct_huff_code (image ,width ,height , &code[posi], pc_dc,
			   bits_dc, pc_ac , bits_ac, qns , ycflag);
//printf( "kbyte = %d\n", kbyte );
    if (kbyte <= 0){
	printf("kbyteが0以下\n ");
	return -1;
    }
    posi = 0;
    n_prb_dc = byte_copy((char *)&prb_dc[0], (char *)&code[posi], n_prb_dc);
    //printf("n_prb_dc\n ");
    //dump(prb_dc , sizeof(prb_dc)); 
    posi += n_prb_dc;
    n_prb_ac = byte_copy((char *)&prb_ac[0][0], (char *)&code[posi], n_prb_ac);
    posi += n_prb_ac;
    //printf("n_prb_ac\n ");
    //dump(prb_ac , sizeof(prb_ac)); 
    n_pc_dc = byte_copy((char *)&pc_dc[0], (char *)&code[posi], n_pc_dc);
    posi += n_pc_dc;
    //printf("n_pc_dc\n ");
    //dump(pc_dc , sizeof(pc_dc)); 
    n_pc_ac = byte_copy((char *)&pc_ac[0][0], (char *)&code[posi], n_pc_ac);
    posi += n_pc_ac;
    //printf("n_pc_ac\n ");
    //dump(pc_ac , sizeof(pc_ac)); 
    n_bits_dc = byte_copy((char *)&bits_dc[0], (char *)&code[posi], n_bits_dc);
    posi += n_bits_dc;
    //printf("n_bits_dc\n ");
    //dump(bits_dc , sizeof(bits_dc)); 
    n_bits_ac = byte_copy((char *)&bits_ac[0][0], (char *)&code[posi], n_bits_ac);
    posi += n_bits_ac;
    //printf("n_bits_ac\n ");
    //dump(bits_ac ,sizeof(bits_ac)); 
    n_qns = byte_copy((char *)&qns, (char *)&code[posi], n_qns);
    posi += n_qns ;
    //printf("qns\n %f\n ",qns); 
    //dump(&qns , sizeof(qns)); 
    //printf("%d ,posi =%d\n ",kbyte +posi , popo);
    memmove(code+posi , code+popo , kbyte);
    return kbyte + posi;
}


/****************************************************************************
		dct_huff_code	DCT+ハフマン符号化による符号化
image:	画像データ
code:	符号データ用メモリ
qn:	量子化テーブルの乗数
pc_dc:	ハフマン符号と符号長
bits_dc:  (直流成分)
pc_ac:	ハフマン符号と符号長
bits_ac:  (直流成分)
*****************************************************************************/
static int dct_huff_code(unsigned char *image ,int width , int height ,
			 unsigned char *code , int pc_dc[16],
			 int bits_dc[16], int pc_ac[16][16],
		         int bits_ac[16][16], float qn , int ycflag)
{
    int		k,xs,ys,run,val,gno,pre;
    int		q[8*8];
    int 	kbyte;
    int		kbit;
    int		ret;
    int HEIGHT , WIDTH , startys ;
    
    kbyte = kbit = 0;
    pre = (int) (64/qn);

    if (ycflag == C){// C画像の縦が半分のための処理
	HEIGHT = height/2 -Y_WAKU ;
	//startys = Y_WAKU ;
	//startxs = X_WAKU ;
    }else{
	HEIGHT = height-Y_WAKU*2 ;
	//startys = Y_WAKU ;
	//startxs = X_WAKU ;
    }

    bigbuff = bigbuff_top;//bigbuffの最初のアドレスに戻る
    for (ys = 0 ; ys < HEIGHT ; ys += 8){
	for (xs = 0 ; xs < width-X_WAKU*2 ; xs += 8){
	    block_copy( q , bigbuff );
	    bigbuff += 64 ;
	    /*	    if(xs == width-8 && ys == HEIGHT-8){
		printf("最後のブロック\n");
		dump(q ,64*4);
		}*/
	    run = 0;
	    for (k=0; k < 64 ;k++){
		val = q[scan[k]];
		if ( k == 0){//直流成分
		    val -= pre;
		    pre += val;
		    grp(val,&gno);
		    ret = huffman_save(code , &kbyte, &kbit, pc_dc[gno],
				       bits_dc[gno]);
		    if (ret == -1) {
			printf("retが -1 \n ");
			return -1;
		    }
		    if (val < 0 ) {
			val -= -( (1 << gno) -1);
		    }
		    if (gno != 0) {
			ret = huffman_save(code , &kbyte, &kbit,
					   (int)val, gno );
			if (ret == 1) {
			    printf("retが -1 2 個目\n ");
			    return -1;}
		    }
		    continue;
		}

		//交流成分
		if (val == 0){
		    run++;
		}else {
		    while (run >15){
			ret = huffman_save(code , &kbyte, &kbit,
					   pc_ac[15][0],bits_ac[15][0]);
			if (ret == -1) {
			    printf("retが -1   3個目\n ");
			    return -1;
			}
			run -= 16;
			//printf("while文のなかrun = %d\n ", run);

		    }
		    grp(val, &gno);
		    ret = huffman_save(code , &kbyte, &kbit,
				       pc_ac[run][gno],bits_ac[run][gno]);
		    if (ret == -1) {
			printf("retが-1   4 個目\n ");
			return -1;
		    }
		    if(val < 0) {val  -= -((1 << gno) -1);}
		    ret = huffman_save(code , &kbyte, &kbit,(int)val,
				       gno);
		    if (ret == -1) {
			printf("retが-1   5 個目\n ");
			return -1;
		    }
		    run = 0;
		}
	    }
	    if (run != 0){
		//printf("run= %d ,ac[0][0]= %d" , run , bits_ac[0][0]);
		//if( run == 63 ){bits_ac[0][0]= 1;}//単一色圧縮用
		//	if(flag == 0 && run == 63 ){bits_ac[0][0]= 1;}//単一色圧縮用
		
		ret = huffman_save(code , &kbyte, &kbit,
				   pc_ac[0][0],bits_ac[0][0]);
		if (ret == -1 ) {
		    printf("retが-1   6 個目\n ");
		    return -1;
		}
	    }
	    
	}
    }
    //最後のビットをフラッシュ
    if(kbit != 0){
	kbyte ++ ;
    }
    return kbyte ;
}

/*****************************************************************************
									      byte_copy	データコピー
									      ps :	転送元のメモリのポインタ
									      pd:	転送先のメモリのポインタ
									      n:	転送するバイト数
*****************************************************************************/
static int byte_copy(unsigned char *ps,unsigned char *pd, int n)
{
    int i  , n_pd=0;
    unsigned char run = 0;

    for (i=0 ; i < n; i++){
	if( run == 0 && *ps == 0 ){
	    *pd++ =  0x00 ;
	    n_pd ++ ;
	    ps++ ;
	    run++ ;
	}else if( run != 0 ){
	    if ( *ps != 0 ){//0の直後に数字が来た時
		pd[0] = run ;
		pd[1] = *ps++;
		pd += 2;
		n_pd +=2 ;
		run = 0;
	    }else if( run >= 255){//0が255より多くなった時の処理
		pd[0] = run ;
		pd[1] = 0x00 ;
		pd += 2 ;
		n_pd += 2 ;
		ps++ ;
		run = 1 ;
	    }else {//増える 0
		run ++ ;
		ps++ ;		
	    }
	}else {
	    //通常の処理
	    *pd++ = *ps++;
	    n_pd++;
	}
    }
    if(run != 0){
	*pd++ = run;
	n_pd ++ ;
    }
    return n_pd;
}
 

/****************************************************************************
		prb_get		シンボルの出現確率を求める
image:	入力画像配列
prb_dc:	シンボルの出現確率
prb_ac:	シンボルの出現確率
qn:	量子化テーブルの乗数
*****************************************************************************/
static void prb_get(unsigned char  *image, int width , int height ,
		    float prb_dc[16],float prb_ac[16][16], float qn , int ycflag)
{
#if 0
    int i, j, k, xs, ys, run, val, gno, pre;
    float f[8][8], g[8][8];
    float total_dc, total_ac;
    int q[8*8];

    int HEIGHT , startxs , startys;

    /* prb_dc 直流成分(u=0,v=0) を初期化 */
    for	(i= 0; i<16 ; i++) {
	prb_dc[i] = 0;
    }
    /* prb_ac 交流成分 を初期化 */
    for (i= 0; i<16 ; i++){
	for (j= 0; j<16 ; j++){
	    prb_ac[i][j] = 0;
	}
    }
    /* total_dc , total_ac を初期化 */
    total_dc = 0 ;
    total_ac = 0;
    /* ????? */
    pre = (int)(64/qn);

    if (ycflag == C){// C画像の縦が半分のための処理
	HEIGHT =  height/2 ;
	//	startys = Y_WAKU ;
	//startxs = X_WAKU  ;
    }else{
	HEIGHT = height ;
	//startys = Y_WAKU ;
	//startxs = X_WAKU ;

    }

    bigbuff = bigbuff_top;//bigbuffの最初のアドレスに戻る    
    for (ys = 0 ; ys < HEIGHT ; ys +=8){
	for (xs = 0 ; xs < width ; xs += 8){
	    // 8x8のブロックにわけて gに入れて渡す
	    calc(width ,height , xs, ys , ycflag ,
		 image , f);

	    //枠の処理
	    if(ycflag == Y){
		if(ys < Y_WAKU || (ys >= HEIGHT - Y_WAKU) ){
		    continue ;
		}else if ( xs < X_WAKU || xs >= width - X_WAKU ){
		    continue ;
		}
	    }else {//Cのとき
		if( ys < Y_WAKU/4 || (ys >= HEIGHT - Y_WAKU/4) ||
		    ((ys >= HEIGHT/2-Y_WAKU/4 ) && ( ys < HEIGHT/2+Y_WAKU/4 ))){
		    continue ;
		}else if( xs < X_WAKU/2 || (xs >= width - X_WAKU/2) ||
		    ((xs >= width/2-X_WAKU/2 ) && ( xs < width/2+X_WAKU/2 ))){
		    continue ;
		}
	    }

	    dct(f,g);
	    //printf("f\n");dump(f, 64*4);
	    //printf("g\n");dump(g, 64*4);
	    dct_qt(g, (int (*)[8])q, qn);//Qテーブルで割って、q に入れる
	    //printf("q\n");dump(q, 64*4);
	    //imageに、8*8ぶろっくデータをほうりこんでおく。
	    block_copy(bigbuff , q );
	    bigbuff += 64;
	    
	    run =0;	//ランレングス 初期化
	    for (k=0; k<64; k++){
		val = q[scan[k]];// バラバラの順番で要素を valにいれる
		
		if(k == 0){/*直流成分の処理*/
		    val -= pre;//pre 前の値(val)
		    pre += val;//val 前の値との差分
		    grp(val, &gno);//グループ分け
		    prb_dc[gno]++; total_dc++;//直流成分をカウント   
		    continue;
		}

		/*交流成分の処理*/
		if(val == 0){//値が0の時は ランレングスを +1
		    run++;
		}else{
		    while(run > 15 ){//ラン長は 16が最大 その処理
			//prb_ac[15][0]は 全部 0 のグループ
			prb_ac[15][0]++; total_ac++; run -= 16;
		    }
		    // 0が15個以内の処理 グループわけ
		    // prb_ac[ラン長][グループ番号]   となっている
		    grp(val , &gno);
		    prb_ac[run][gno]++; total_ac++;
		    run = 0;
		}
	    }// k のループ
	    if (run != 0) {
		prb_ac[0][0] ++ ;
		total_ac ++ ;
	    }
	}// x のループ
    }// y のループ
    
    // 全体数で割って 確率にする
    for (i =0; i <16; i++){//16個まで見る
	prb_dc[i] /= total_dc;
	//printf("dc %f\n" , prb_dc[i]);
    }
    
    for (i =0; i <16; i++){//16*16個までみる
	for (j =0; j <16; j++){
	    prb_ac[i][j] /= total_ac;
	}
    }
#else
    int i, j, k, xs, ys, run, val, gno, pre;
    float f[8][8], g[8][8];
    float total_dc, total_ac, div_param;
    int q[8*8];

    int HEIGHT , startxs , startys;

    /* prb_dc 直流成分(u=0,v=0) を初期化 */
    for	(i= 0; i<16 ; i++) {
		prb_dc[i] = 0;
    }
    /* prb_ac 交流成分 を初期化 */
    for (i= 0; i<16 ; i++){
		for (j= 0; j<16 ; j++){
			prb_ac[i][j] = 0;
		}
    }
    /* total_dc , total_ac を初期化 */
    total_dc = 0 ;
    total_ac = 0;
    /* ????? */
    pre = (int)(64/qn);

    if (ycflag == C){// C画像の縦が半分のための処理
		HEIGHT =  height/2 ;
		//	startys = Y_WAKU ;
		//startxs = X_WAKU  ;
    }else{
		HEIGHT = height ;
		//startys = Y_WAKU ;
		//startxs = X_WAKU ;
    }

    bigbuff = bigbuff_top;//bigbuffの最初のアドレスに戻る    
	for (ys = 0 ; ys < HEIGHT ; ys +=8){
		for (xs = 0 ; xs < width ; xs += 8){
			// 8x8のブロックにわけて gに入れて渡す
			calc(width ,height , xs, ys , ycflag , image, f);

			//枠の処理
			if(ycflag == Y){
				if(ys < Y_WAKU || (ys >= HEIGHT - Y_WAKU) ){
					continue ;
				}else if ( xs < X_WAKU || xs >= width - X_WAKU ){
					continue ;
				}
			}else {//Cのとき
				if( ys < Y_WAKU/4 || (ys >= HEIGHT - Y_WAKU/4) ||
				   ((ys >= HEIGHT/2-Y_WAKU/4 ) && ( ys < HEIGHT/2+Y_WAKU/4 ))){
					continue ;
				}else if( xs < X_WAKU/2 || (xs >= width - X_WAKU/2) ||
						 ((xs >= width/2-X_WAKU/2 ) && ( xs < width/2+X_WAKU/2 ))){
					continue ;
				}
			}

			dct(f,g);
			//printf("f\n");dump(f, 64*4);
			//printf("g\n");dump(g, 64*4);
			dct_qt(g, (int (*)[8])q, qn);//Qテーブルで割って、q に入れる
			//printf("q\n");dump(q, 64*4);
			//imageに、8*8ぶろっくデータをほうりこんでおく。
			block_copy(bigbuff , q );
			bigbuff += 64;
	    
			run =0;	//ランレングス 初期化
			for (k=0; k<64; k++){
				val = q[scan[k]];// バラバラの順番で要素を valにいれる
		
				if(k == 0){/*直流成分の処理*/
					val -= pre;//pre 前の値(val)
					pre += val;//val 前の値との差分
					grp(val, &gno);//グループ分け
					prb_dc[gno]++; total_dc++;//直流成分をカウント   
					continue;
				}

				/*交流成分の処理*/
				if(val == 0){//値が0の時は ランレングスを +1
					run++;
				}else{
					while(run > 15 ){//ラン長は 16が最大 その処理
						//prb_ac[15][0]は 全部 0 のグループ
						prb_ac[15][0]++; total_ac++; run -= 16;
					}
					// 0が15個以内の処理 グループわけ
					// prb_ac[ラン長][グループ番号]   となっている
					grp(val , &gno);
					prb_ac[run][gno]++; total_ac++;
					run = 0;
				}
			}// k のループ
			if (run != 0) {
				prb_ac[0][0] ++ ;
				total_ac ++ ;
			}
		}// x のループ
    }// y のループ
    
	// 全体数で割って 確率にする
	div_param = 1.0f / total_dc ;
	for (i =0; i <16; i++){//16個まで見る
		prb_dc[i] *= div_param ;
		//printf("dc %f\n" , prb_dc[i]);
	}
    
	div_param = 1.0f / total_ac ;
    for (i =0; i <16; i++){//16*16個までみる
		for (j =0; j <16; j++){
			prb_ac[i][j] *= div_param ;
		}
	}
#endif
}




/***************************************************************************
		grp	有効係数グループ番号を求める
val:	入力有効係数
pgno:	出力グループ番号
****************************************************************************/
static void grp(int val, int *pgno)
{
#if 0
    if		(val == 0)				{*pgno = 0;}
    else if	(     -1 <= val && val <=        1)	{*pgno = 1;}    
    else if	(     -3 <= val && val <=        3)	{*pgno = 2;}    
    else if	(     -7 <= val && val <=        7)	{*pgno = 3;}    
    else if	(    -15 <= val && val <=       15)	{*pgno = 4;}    
    else if	(    -31 <= val && val <=       31)	{*pgno = 5;}    
    else if	(    -63 <= val && val <=       63)	{*pgno = 6;}    
    else if	(   -127 <= val && val <=      127)	{*pgno = 7;}    
    else if	(   -255 <= val && val <=      255)	{*pgno = 8;}    
    else if	(   -511 <= val && val <=      511)	{*pgno = 9;}    
    else if	(  -1023 <= val && val <=     1023)	{*pgno = 10;}    
    else if	(  -2047 <= val && val <=     2047)	{*pgno = 11;}    
    else if	(  -4095 <= val && val <=     4095)	{*pgno = 12;}    
    else if	(  -8191 <= val && val <=     8191)	{*pgno = 13;}    
    else if	( -16383 <= val && val <=    16383)	{*pgno = 14;}
    else						{*pgno = 15;}
#else
#if 1
	if ( val < 0 ) val = -val ;
    if (val == 0)			{*pgno = 0;}
	else if ( !( val & ~0x0001 ) )	{*pgno = 1;}
	else if ( !( val & ~0x0003 ) )	{*pgno = 2;}
	else if ( !( val & ~0x0007 ) )	{*pgno = 3;}
	else if ( !( val & ~0x000f ) )	{*pgno = 4;}
	else if ( !( val & ~0x001f ) )	{*pgno = 5;}
	else if ( !( val & ~0x003f ) )	{*pgno = 6;}
	else if ( !( val & ~0x007f ) )	{*pgno = 7;}
	else if ( !( val & ~0x00ff ) )	{*pgno = 8;}
	else if ( !( val & ~0x01ff ) )	{*pgno = 9;}
	else if ( !( val & ~0x03ff ) )	{*pgno = 10;}
	else if ( !( val & ~0x07ff ) )	{*pgno = 11;}
	else if ( !( val & ~0x1fff ) )	{*pgno = 12;}
	else if ( !( val & ~0x3fff ) )	{*pgno = 13;}
	else if ( !( val & ~0x7fff ) )	{*pgno = 14;}
    else							{*pgno = 15;}
#else
	if ( val < 0 ) val = -val ;
	*pgno = 31 - GV_GetNo( val ) ;
#endif
#endif
}

/****************************************************************************
		dct_qt		DCT係数の量子化
g:	入力DCT係数
q:	出力量子化符号データ
qn:	量子化テーブルの乗数
*************************************************************************** */
static void dct_qt(float g[8][8], int q[8][8], float qn)
{
#ifndef PSX2
#if 0
	/* これがオリジナル */
    int i,j;

    for (i=0; i<8 ;i++){
	for (j=0; j<8; j++){
	    if (g[i][j] >=  0.0f){
		q[i][j] = (int)(g[i][j]/((float)q_table[i][j] *qn) + 0.5f);
	    }else{
		q[i][j] = (int)(g[i][j]/((float)q_table[i][j] *qn) - 0.5f);
	    }
	}
    }
#else
	/* ＰＳ２用に最適化してあるものをＸＢＯＸ用に移植 */
    int i,j;
	float	d0, d1 ;

	qn = 1.0f / qn ;
	for (i=0; i<8 ;i++){
		d0 = g[i][0] ;
		for (j=0; j<8; j++){
			d1 = q_table_inv[i][j] * qn ;
			if ( d0 >=  0.0f ){
				d1 = d0 * d1 + 0.5f ;
			}else{
				d1 = d0 * d1 - 0.5f ;
			}
			q[i][j] = d1 ;
			d0 = g[i][j+1] ;
		}
	}
#endif
#else
	/* ＰＳ２用に最適化したもの */
    int i,j, tmp;
	float	d0, d1 ;

	qn = 1.0f / qn ;
	for (i=0; i<8 ;i++){
		d0 = g[i][0] ;
		for (j=0; j<8; j++){
			d1 = q_table_inv[i][j] * qn ;
			if ( d0 >=  0.0f ){
				// q[i][j] = (int)(g[i][j]*( q_table_inv[i][j] * qn ) + 0.5f);
				DG_ADDA( 0.0f, 0.5 );
				d1 = DG_MADD( d0, d1 );
			}else{
				//q[i][j] = (int)(g[i][j]*( q_table_inv[i][j] * qn ) - 0.5f);
				DG_SUBA( 0.0f, 0.5 );
				d1 = DG_MADD( d0, d1 );
			}
			q[i][j] = d1 ;
			d0 = g[i][j+1] ;
		}
	}
#endif
}

/****************************************************************************
		dct	離散コサイン変換
f:	入力画像配列
g:	出力コサイン関数
*****************************************************************************/
static void dct(float f[8][8], float g[8][8])
{
#ifndef PSX2
    int i , j , k;
    float temp1 , temp[8][8];

    for ( i=0; i < 8 ; i++){
	for ( j = 0 ; j < 8 ; j++){
	    temp[i][j] = 0.0f;
	    for( k=0 ; k < 8 ; k++ ){
		temp[i][j] += ( f[i][k] -128.0f) * ct[k][j];
	    }
	}
    }

    for (i=0 ; i < 8 ; i++){
	for ( j = 0 ; j < 8 ; j++){
	    temp1 = 0.0f ;
	    for( k=0 ; k < 8 ; k++ ){
		temp1 += c[i][k] * temp[k][j] ;
	    }
	    g[i][j] = temp1;
	}
    }
#else
	int i , j , k;
	float temp1 , temp[8][8];
	float	f0, f1, f2, f3, f4, f5, f6, f7 ;
	float	d0, d1, d2, d3, d4, d5, d6, d7 ;

	for ( i=0; i < 8 ; i++){
		f0 = f[i][0] - 128.0f ;
		f1 = f[i][1] - 128.0f ;
		f2 = f[i][2] - 128.0f ;
		f3 = f[i][3] - 128.0f ;
		f4 = f[i][4] - 128.0f ;
		f5 = f[i][5] - 128.0f ;
		f6 = f[i][6] - 128.0f ;
		f7 = f[i][7] - 128.0f ;
		for ( j = 0 ; j < 8 ; j++){
			//temp[i][j] = f0 * ct[0][j] + f1 * ct[1][j] + f2 * ct[2][j] + f3 * ct[3][j] +
			//  f4 * ct[4][j] + f5 * ct[5][j] + f6 * ct[6][j] + f7 * ct[7][j] ;
#if 0
			DG_MULA( f0, ct[0][j] );
			DG_MADDA( f1, ct[1][j] );
			DG_MADDA( f2, ct[2][j] );
			DG_MADDA( f3, ct[3][j] );
			DG_MADDA( f4, ct[4][j] );
			DG_MADDA( f5, ct[5][j] );
			DG_MADDA( f6, ct[6][j] );
			temp[i][j] = DG_MADD( f7, ct[7][j] );
#else
			d0 = ct[0][j] ;
			d1 = ct[1][j] ;
			d2 = ct[2][j] ;
			d3 = ct[3][j] ;
			DG_MULA( f0, d0 );
			d4 = ct[4][j] ;
			DG_MADDA( f1, d1 );
			d5 = ct[5][j] ;
			DG_MADDA( f2, d2 );
			d6 = ct[6][j] ;
			DG_MADDA( f3, d3 );
			d7 = ct[7][j] ;
			DG_MADDA( f4, d4 );
			DG_MADDA( f5, d5 );
			DG_MADDA( f6, d6 );
			temp[i][j] = DG_MADD( f7, d7 );
#endif
		}
	}

	for (i=0 ; i < 8 ; i++){
		f0 = c[i][0] ;
		f1 = c[i][1] ;
		f2 = c[i][2] ;
		f3 = c[i][3] ;
		f4 = c[i][4] ;
		f5 = c[i][5] ;
		f6 = c[i][6] ;
		f7 = c[i][7] ;
		for ( j = 0 ; j < 8 ; j++){
			//g[i][j] = f0 * temp[0][j] + f1 * temp[1][j] + f2 * temp[2][j] + f3 * temp[3][j] +
			//  f4 * temp[4][j] + f5 * temp[5][j] + f6 * temp[6][j] + f7 * temp[7][j]  ;
#if 0
			DG_MULA( f0, temp[0][j] );
			DG_MADDA( f1, temp[1][j] );
			DG_MADDA( f2, temp[2][j] );
			DG_MADDA( f3, temp[3][j] );
			DG_MADDA( f4, temp[4][j] );
			DG_MADDA( f5, temp[5][j] );
			DG_MADDA( f6, temp[6][j] );
			g[i][j] = DG_MADD( f7, temp[7][j] );
#else
			d0 = temp[0][j] ;
			d1 = temp[1][j] ;
			d2 = temp[2][j] ;
			d3 = temp[3][j] ;
			DG_MULA( f0, d0 );
			d4 = temp[4][j] ;
			DG_MADDA( f1, d1 );
			d5 = temp[5][j] ;
			DG_MADDA( f2, d2 );
			d6 = temp[6][j] ;
			DG_MADDA( f3, d3 );
			d7 = temp[7][j] ;
			DG_MADDA( f4, d4 );
			DG_MADDA( f5, d5 );
			DG_MADDA( f6, d6 );
			g[i][j] = DG_MADD( f7, d7 );
#endif
		}
	}
#endif
}


/***************************************************************************
	huffman_get	ハフマン符号を求める
prb:	シンボルの出現確率
n:	シンボル数
pc:	ハフマン符号
bits:	ハフマン符号の符号長
****************************************************************************/
static int huffman_get( float prb[] , int n , int pc[] , int bits[] )
{
    // prb_ が 一個の時も割り当てる 変数を作っておく
    
    float pmin1, pmin2;
    int nmin1 = 0 , nmin2 = 0 ;
    int i, j, ps, pd;
    int count = 0;
#ifdef BP_PSX2_GCC
    int ptr[n] , pbits[n];  /* removed by T.Morita 2002.02.01*/
#else
    int *ptr , *pbits;
	ptr   = STACK_ALLOC( sizeof(int) * n ) ; /* gcc拡張表現から Mallocに変更 T.Morita 2002.02.01 */
	if ( ptr == NULL )
	  return -1 ;
	pbits = STACK_ALLOC( sizeof(int) * n ) ; /* gcc拡張表現から Mallocに変更 T.Morita 2002.02.01 */
	if ( pbits == NULL )
	  return -1 ;	
#endif

    //初期化
    for (i= 0; i<n; i++){
		ptr[i]  = -1;
		pc[i]   = 0 ;
		bits[i] = 0 ;
    }
    //根から、木へ
	  while(1){	
		  pmin1 = pmin2 = 2.0f;
		  for (i=0; i<n ; i++){
			  if( prb[i] == 0 ){
				  continue ;
			  }
			  if( pmin1 > prb[i] ){
				  pmin1 = prb[i] ;
				  nmin1 = i;
			  }
		  }
		  //if(( n==16 ) && (pmin1 == 1.0f)){//実験
											   //  pc[nmin1] = 1;
											 // bits = 1;
											 //break ;
											 //}
		  prb[nmin1] = 10.0f;
		  for (i = 0; i<n ; i++){
			  if (prb[i] == 0) {
				  continue;
			  }
			  if (pmin2 > prb[i]){
				  pmin2 = prb[i];
				  nmin2 = i;
			  }
	}
		  if (pmin2 == 2.0f) {
			  
			  break;
		  }
		  //pc[nmin1] |= (int)1 << bits[nmin1];
		  pc[nmin2] |= (int)1 << bits[nmin2];
		  prb[nmin2] = pmin1 + pmin2;
		  ptr[nmin1] = nmin2;
		  pbits[nmin1] = bits[nmin2];
		  bits[nmin1]++; bits[nmin2]++;
		  
		  count ++ ;
	  }
    //printf("count %d\n" , count);
	if(count == 0 ){
	    bits[0] = 1 ;
	}
    //全体像を作る
	  for(i=0; i<n; i++){
		  ps = i;
		  while(1){
			  if ((pd = ptr[ps] ) == -1) {
				  break;
			  }
			  for (j = pbits[ps]+1 ; j< bits[pd] ; j++){
				  if (pc[pd] & ((int)1 << j ))  {
					  pc[i] |= ((int)1<<bits[i]);
				  }
				  bits[i]++;
			  }
			  ps =pd;
		  }
		  ptr[i] = -1;
	  }
	
    /*    {int i;
		  for (i=0 ; i < n ; i ++){
		  printf("pc[%d] = %x\n",i , pc[i]);
		  }
		  }*/
    return 0;
}


/***************************************************************************
		huffman_save	ハフマン符号をメモリに書き出す
buf:	メモリ
nbyte:	メモリ内のバイト位置を指すポインタ
nbits:	バイト内のメモリ位置を指すポインタ
code:	ハフマン符号
bits:	ハフマン符号の符号長
****************************************************************************/
static int huffman_save(char buf[], int  *nbyte, int *nbit, int code, int bits)
{
    int i;

    if(bits == 0){
	printf("bitsが 0  huffmansave \n ");
	return -1;
    }
    for (i = bits -1; i >= 0 ; i--){
	if (code & ((int)1 <<i )){ buf[*nbyte] |= (char)( 1 << *nbit);}
	else buf[*nbyte] &= (char)(~(1 << *nbit));
	*nbit += 1;
	if ( *nbit == 8) {
	    *nbit = 0; *nbyte += 1;
	}
    }
    return 0;
}

/*面倒な計算関数*/
static int calc(int width ,int height , int xs ,int ys ,int ycflag , unsigned char *image  ,float f[8][8] )
{
    unsigned char *p;
    int yys , cell , step ,istep;
    int i, j;

    if(ycflag == Y){
	p = image + ys*width*4 + xs*4;
	step =4;
	istep =1;
    }   
    else { // ycflag == C の時
	step = 8 ;
	istep =2 ;
	if(ys < height/4){
	    yys = ys;
	    cell =1;
	} else {
	    yys = ys - height/4 ;
	    cell = 2;
	}
	p = image + (xs % (width/2))*8 + width*4*(4*yys + 2*(xs/(width/2))*8)+cell;
    }
    for(i=0 ; i < 8 ; i++){
	for(j=0 ; j < 8 ; j++){
	    f[i][j] = (float)(*p);
	    p = p + step;
	}
	p = p + width*4*istep - 8*step ;
    }
    return 0;
}

/* 8*8ブロックをimageい書き込み  */
static void block_copy(int *image , int *q  )
{
#if 0
    int i ;

    for ( i = 0 ; i < 64 ; i++){
	*image++ =  *q++ ;
    }
#else
	int i ;

	for ( i = 0 ; i < 64/4 ; i++){
		image[0] = q[0] ;
		image[1] = q[1] ;
		image[2] = q[2] ;
		image[3] = q[3] ;
		image += 4 ;
		q += 4 ;
	}
#endif
}

/******************************************************************************
		image_clear_color	カラー画像データをクリアする
image:	入力画像ファイル
******************************************************************************/
/*void image_clear_color(unsigned char *image , int width , int height)
{

}*/

/* ---------------------------------------------------------------------- */
/*
	外部とのインターフェース
*/

int UTL_JpegEncode( UTL_JPEGWORK *jpegwork, int quality )
{
	static float q_base[ 2 ] = { 0.4F, 0.5F };
	float q[ 2 ];

	bigbuff_top = jpegwork->work;

	q[ 0 ] = q_base[ 0 ] + quality * 0.2F;
	q[ 1 ] = q_base[ 1 ] + quality * 0.2F;
printf( "Q = %f %f\n", q[ 0 ], q[ 1 ] );
	return cdct_huff( jpegwork->vram, jpegwork->width, jpegwork->height
					  , jpegwork->code, q ) + sizeof( int ) * 2;
}

