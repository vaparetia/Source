#include <stdio.h>

#define BLEND  0
#define SENKEI 1


void transform(unsigned char *image , int width , int height , unsigned char *thumbnail , int T_WIDTH , int T_HEIGHT );
int getcolor(unsigned char * image , float x , float y , int width , int height);

/****************周辺の画素を平均して色を取得*******************/
int getcolor2(unsigned char * image , int intX , int intY , int width , int height ,
		     int b_x , int b_y)
{
    int i , j ;
    int sum = 0 ;
    int a , b ;

    for( i = b_y ; i < intY ; i++ ){
	for( j = b_x ; j < intX ; j++ ){
	    sum += *(image + i*width*4 + j*4) ;
	}
    }
    printf("intX %d, intY %d, b_x %d, b_y %d\n" , intX , intY , b_x , b_y );
    a = (intX - b_x)*(intY - b_y) ;
    b = sum / a ;

    return b ;    
}

void transform(unsigned char *image , int width , int height , unsigned char *thumbnail , int T_WIDTH , int T_HEIGHT )
{
    int     i , j ;
    int     r , g, b , transbit;
    float   x , y , xtimes , ytimes ;
    int     b_x=0 , b_y=0 , intX , intY ;
    
    xtimes = (float)T_WIDTH  / (float)width ;    //xの倍率
    ytimes = (float)T_HEIGHT / (float)height ;   //yの倍率
    //printf("x倍率=%f ,y倍率= %f \n",xtimes , ytimes);    
#if BLEND
    for (i = 0 ; i < T_HEIGHT ; i++){
	for (  j = 0 ; j < T_WIDTH*2 ; j += 2 ){
	    x = (j/2) * (1/xtimes) ;  //元画像でのx座標
	    y =   i   * (1/ytimes) ;  //元画像でのy座標
	    //線形補完法で色を取得
	    r = getcolor( image    , x , y , width , height);
	    g = getcolor( image +1 , x , y , width , height);    
	    b = getcolor( image +2 , x , y , width , height);
#else
    for (i = 1 ; i < T_HEIGHT+1 ; i++){
	for (  j = 2 ; j < T_WIDTH*2+2 ; j += 2 ){
	    x = (j/2) * (1/xtimes) ;  //元画像でのx座標
	    y =   i   * (1/ytimes) ;  //元画像でのy座標
	    /*周辺の画素値の平均で*/
	    intX = (int)( x + 0.5F ) ;/*四捨五入*/
	    intY = (int)( y + 0.5F ) ;/*        */

	    r = getcolor2( image    , intX , intY , width , height , b_x , b_y);
	    g = getcolor2( image +1 , intX , intY , width , height , b_x , b_y);
	    b = getcolor2( image +2 , intX , intY , width , height , b_x , b_y);

	    b_x = intX ;/*値を記憶*/

#endif
	    //32bit -> 16bit に変換
	    r >>= 3;
	    g >>= 3;
	    b >>= 3;
	    transbit = r | (g<<5) | (b<<10) ;
	    
#if BLEND
	    *( thumbnail + i*T_WIDTH*2 + j   ) = transbit;
	    *( thumbnail + i*T_WIDTH*2 + j+1 ) = transbit >> 8;
	}

#else
	    *( thumbnail + (i-1)*T_WIDTH*2 + (j-2)   ) = transbit;
	    *( thumbnail + (i-1)*T_WIDTH*2 + (j-2)+1 ) = transbit >> 8;
	}
	b_x = 0 ;
	b_y = intY ;
#endif
    }
}

int getcolor(unsigned char * image , float x , float y , int width , int height)
{
    int   c1 , c2 , c3 , c4 ;
    float xr , yr ;
    int   intx , inty ;
    float d , e , F ;
    
    intx = (int)x ;
    inty = (int)y ;

    xr = x - intx ;
    yr = y - inty ;

    c1 = *(image +  inty   *width*4 +  intx   *4 );
    c2 = *(image +  inty   *width*4 + (intx+1)*4 );
    c3 = *(image + (inty+1)*width*4 +  intx   *4 );
    c4 = *(image + (inty+1)*width*4 + (intx+1)*4 );

    d = (float)c1 * ( 1.0f - xr ) + (float)c2 * xr ;
    e = (float)c3 * ( 1.0f - xr ) + (float)c4 * xr ;
    F = d * ( 1.0f - yr ) + e * yr ; 

    return (int)F + 0.5f ;
}
