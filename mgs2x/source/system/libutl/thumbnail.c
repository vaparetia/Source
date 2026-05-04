//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>

#include "libgv.h"
#include "jpeg.h"

#define BLEND  0
#define SENKEI 1

static void transform(unsigned char *image , int width , int height , unsigned char *thumbnail , int T_WIDTH , int T_HEIGHT );
#if BLEND
static int getcolor(unsigned char * image , float x , float y , int width , int height);
#endif

#if 0
/****************周辺の画素を平均して色を取得*******************/
static int getcolor2(unsigned char * image , int intX , int intY , int width , int height ,
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
    a = (intX - b_x)*(intY - b_y) ;
    b = sum / a ;
    
    return b ;
    
}

static void transform(unsigned char *image , int width , int height , unsigned char *thumbnail , int T_WIDTH , int T_HEIGHT )
{
    int     i , j ;
    int     r , g, b , transbit;
    float   x , y , xtimes , ytimes ;
    int     b_x=0 , b_y=0 , intX , intY=0 ;
    
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

#endif


static void transform2(unsigned char *image , int width , int height , unsigned char *thumbnail , int T_WIDTH , int T_HEIGHT )
{
	int     i, j, x, y ;
	int     r , g, b , transbit;
	int		x0, x1, y0, y1, xx, yy ;/* (1.15.16)形式固定小数点として使用する */
	unsigned short	*write_data ;

	write_data = (unsigned short*)thumbnail ;
	xx = 65536 * width / T_WIDTH ;		/* オフセット量を固定小数点で求める */
	yy = 65536 * height / T_HEIGHT ;	/* オフセット量を固定小数点で求める */
	y1 = 0 ;
	for ( i = 0 ; i < T_HEIGHT ; i++ ){
		x1 = 0 ;
		y0 = y1 ; y1 += yy ;
		for ( j = 0 ; j < T_WIDTH ; j++ ){
			x0 = x1 ; x1 += xx ;

			{/* 周辺の色の平均をとる */
				/* x0 <= x < x1 */
				/* y0 <= y < y1 */
				int		ex, ey, count ;
				unsigned char	*data ;
				ex = x1 >> 16 ;
				ey = y1 >> 16 ;
				count = 0 ;
				r = g = b = 0 ;
				for ( y = ( y0 >> 16 ) ; y < ey ; y++ ){
					x = x0 >> 16 ;
					data = image + ( x + y * width ) * 4 ;
					for ( ; x < ex ; x++ ){
						r += data[0] ;
						g += data[1] ;
						b += data[2] ;
						data += 4 ;
						count++ ;
					}
				}
#if 0
				r /= count ;
				g /= count ;
				b /= count ;
#else
				count = 65536 / count ;
				r = ( r * count ) >> 16 ;
				g = ( g * count ) >> 16 ;
				b = ( b * count ) >> 16 ;
#endif
			}

			//32bit -> 16bit に変換
			r >>= 3 ;
			g >>= 3 ;
			b >>= 3 ;
			transbit = r | (g<<5) | (b<<10) | (1<<15);/*yano alpha bit 追加 2002.04.05yano*/
			*write_data++ = transbit ;
		}
	}
}

#if BLEND
static int getcolor(unsigned char * image , float x , float y , int width , int height)
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
#endif


/* ---------------------------------------------------------------------- */
/*
	外部とのインターフェース
*/

void UTL_MakeThumbnail( UTL_JPEGWORK *jpegwork
					   , void *thumb_buf, int width, int height )
{
#if 0
	int		time ;
	GV_SET_PRFC_CLOCK();
	if ( !( GV_PadData[0].status & PAD_X ) ){
		printf("start 1\n");
		transform( jpegwork->vram, jpegwork->width, jpegwork->height
				   , thumb_buf, width, height );
	} else {
		printf("start 2\n");
		transform2( jpegwork->vram, jpegwork->width, jpegwork->height
				   , thumb_buf, width, height );
	}
	time = GV_GET_PRFC_CLOCK();
	printf("time = %d\n", time );
#else
	transform2( jpegwork->vram, jpegwork->width, jpegwork->height
				, thumb_buf, width, height );
#endif
}
