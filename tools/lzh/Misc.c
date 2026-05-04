#include <libps.h>

#include "types.h"
#include "vars.h"



/*----------------------------------------------------------------------
texture loader relatives
----------------------------------------------------------------------*/
void load_texture( u_long *addr )
{
	RECT	rect ;
	GsIMAGE  tim ;

	GsGetTimInfo( addr+1, &tim ) ;

	rect.x = tim.px ;
	rect.y = tim.py ;
	rect.w = tim.pw ;
	rect.h = tim.ph ;
	LoadImage( &rect, tim.pixel ) ;

	if ( (tim.pmode>>3) & 0x01 )
	{
		rect.x = tim.cx ;
		rect.y = tim.cy ;
		rect.w = tim.cw ;
		rect.h = tim.ch ;
		LoadImage( &rect, tim.clut ) ;
	}
}

/*----------------------------------------------------------------------
Random relatives
----------------------------------------------------------------------*/
#define RND_MASK 0x40004000L

int  rnum ()        /* The Random Groups of M */
{
    static long rnd_magic = 12345678L ;

    rnd_magic <<= 1 ;
    if ( (rnd_magic&RND_MASK) == 0 || (rnd_magic&RND_MASK) == RND_MASK )
        rnd_magic++ ;
    return rnd_magic ;
}




/*----------------------------------------------------------------------
GetPad relatives
----------------------------------------------------------------------*/
u_long get_pad1( int act )
{
	static u_long pprev ;

//	if ( *pad_buf1 )
	{
		pdata = ~( *(pad_buf1+3) | *(pad_buf1+2) << 8 ) ;
		ppush = (pdata ^ pprev) & pdata ;
	}
//	else
//		pdata = ppush = 0 ;

	return (pprev = act*pdata) ;
}
u_long get_pad2( int act )
{
	static u_long pprev ;

	if ( *pad_buf2 )
	{
		pdata = ~( *(pad_buf2+3) | *(pad_buf2+2) << 8 ) ;
		ppush = (pdata ^ pprev) & pdata ;
	}
	else
		pdata = ppush = 0 ;

	return (pprev = act*pdata) ;
}







/*----------------------------------------------------------------------

LZS 圧縮の解凍

----------------------------------------------------------------------*/
#define THRESHOLD       3			//      しきい値

#define MATCHBIT	7
#define DICBIT          10			//      辞書ビット数
#define DICSIZ          (1U << DICBIT)		//      辞書のサイズ
#define NIL             DICSIZ			//      未登録記号

#define DIC(s)          ((s) & (DICSIZ - 1))	//      辞書のサイズに制限するマクロ

#define rightbits(n,x) ((x)&((1U<<(n))-1U))

unsigned int bitbuf ;
int getcount ;


unsigned int getbit( u_char **in )
{
	if( --getcount >= 0 )
                return (bitbuf >> getcount) & 1U;

        getcount = 7;
	bitbuf = *(*in)++ ;
        return bitbuf & (1U << 7) ;
}
unsigned int getbits( u_char **in, int n )
{
	unsigned long x = bitbuf ;
	int i = 32-n ;

#if 0
	n -= getcount ;
	getcount = (32-n)%8 ;
	for ( n-- ; n>=0 ; n-=8 )
		x = (x<<8) + (bitbuf = *(*in)++) ;
#else
	while( n > getcount )
	{
		n -= getcount ;
		bitbuf = *(*in)++ ;
		x = (x<<8) + bitbuf ;
		getcount = 8 ;
	}
	getcount -= n ;
#endif
	return (x << (i-getcount)) >> i ;
}
int output_img( u_char *buf, int x, int y )
{
	static RECT	rect ={ 0, 0, BUFWDT, BUFHGT } ;

	rect.x = x ;
	rect.y = y ;
	LoadImage( &rect, (u_long *)buf ) ;
}
void lzs_decode( int count, u_char *in, u_char *out )
{
	int    i, j, k ;
	int    pos ;
	static u_char text[DICSIZ] ;		//      辞書かつバッファ

	k = 0 ;
	pos = bitbuf = getcount = 0 ;
	while( 1 )
		if( getbit( &in ) )		//      フラグを取得
                {
			text[pos] = getbits( &in, 8 ) ;	// １文字入力を出力し、辞書にも登録
			if ( !(pos = DIC(pos+1)) )	// r を辞書内に制限
				output_img( text, 320, (k++)*BUFHGT ) ;
                        if( --count == 0 )		// 残りカウントが０なら終了する
                                return ;
                }
                else
                {
                        j = getbits( &in, MATCHBIT ) + THRESHOLD-1 ;	// 一致長を得る
                        i = DIC(pos - getbits( &in, DICBIT ) - 1)  ;	// 一致場所を得る
                        for( ; j>=0 ; j--, i=DIC(i+1) )			// 一致文字列を出力し、辞書にも登録
                        {
				text[pos] = text[i] ;
				if ( !(pos = DIC(pos+1)) )
					output_img( text, 320, (k++)*BUFHGT ) ;
				if( --count == 0 )
					return ;
                        }
                }
}











/*----------------------------------------------------------------------

フィールド判定用関数

----------------------------------------------------------------------*/

int detect_field( FIELD *f, VECTOR v0, VECTOR *vel )
{
	VECTOR v1 ;
	MATRIX  m ;
	m = f->dir_mtx ;

	if ( vel )
	{
		v0.vx += vel->vx ;
		v0.vy += vel->vy ;
		v0.vz += vel->vz ;
	}
	v0.vx += m.t[X] ;
	v0.vy += m.t[Y] ;
	v0.vz += m.t[Z] ;
	ApplyMatrixLV( &m, &v0, &v1 ) ;

	if ( v1.vx/f->wdth == 0 && v1.vy/f->hght == 0 && v1.vz/f->leng == 0 )
		return -1 ;
	return 0 ;
}
