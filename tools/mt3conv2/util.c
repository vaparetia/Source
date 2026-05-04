#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<math.h>
#include	<ctype.h>


/* その他のルーチン */
/* その数字の符号を返す */
int GetSign( int a )
{
	if ( a > 0 ) return ( 1);
	if ( a < 0 ) return (-1);
	else return (0);
}
/* 有効ビット数を返す */
int GetEffectiveBit( int data )
{
	unsigned int mask,bit,sign;
	bit = 16;
	mask = 0x0008000;
	sign = data & mask ;
	do {
		sign >>= 1;
		mask >>= 1;
		if ( sign != (unsigned int)(data & mask) ) return ( bit );
	} while ( --bit );
	return ( bit );
}

/* 指定された数のshort型データをリトルエンディアンに揃える */
void ChangeLittleEndianShort( unsigned short *src, int num )
{
	unsigned short tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		*dst++ = tmp & 0xff;
		*dst++ = (tmp >> 8) & 0xff;
	}
}
/* 指定された数のlong型データをリトルエンディアンに揃える */
void ChangeLittleEndianLong( unsigned long *src, int num )
{
	unsigned long tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		*dst++ = tmp & 0xff;
		tmp >>= 8;
		*dst++ = tmp & 0xff;
		tmp >>= 8;
		*dst++ = tmp & 0xff;
		tmp >>= 8;
		*dst++ = tmp & 0xff;
	}
}

/* 指定された数のリトルエンディアンデータをshort型に揃える */
void RecoverLittleEndianShort( unsigned short *dst, int num )
{
	unsigned short tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = *src++;
		tmp |= (*src++) << 8;
		*dst++ = tmp;
	}
}
/* 指定された数のリトルエンディアンデータをlong型に揃える */
void RecoverLittleEndianLong( unsigned long *dst, int num )
{
	unsigned long tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = *src++;
		tmp |= (*src++) << 8;
		tmp |= (*src++) << 16;
		tmp |= (*src++) << 24;
		*dst++ = tmp;
	}
}


/* 指定された数のshort型データをビッグエンディアンに揃える */
void ChangeBigEndianShort( unsigned short *src, int num )
{
	unsigned short tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		*dst++ = (tmp >> 8) & 0xff;
		*dst++ = tmp & 0xff;
	}
}
/* 指定された数のlong型データをビッグエンディアンに揃える */
void ChangeBigEndianLong( unsigned long *src, int num )
{
	unsigned long tmp;
	unsigned char  *dst;

	dst = (unsigned char *) src;
	while ( num-- ){
		tmp = *src++;
		dst[3] = tmp & 0xff;
		tmp >>= 8;
		dst[2] = tmp & 0xff;
		tmp >>= 8;
		dst[1] = tmp & 0xff;
		tmp >>= 8;
		dst[0] = tmp & 0xff;
		dst += 4 ;
	}
}

/* 指定された数のビッグエンディアンデータをshort型に揃える */
void RecoverBigEndianShort( unsigned short *dst, int num )
{
	unsigned short tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = src[1];
		tmp |= src[0] << 8;
		*dst++ = tmp;
		src += 2 ;
	}
}
/* 指定された数のビッグエンディアンデータをlong型に揃える */
void RecoverBigEndianLong( unsigned long *dst, int num )
{
	unsigned long tmp;
	unsigned char  *src;

	src = (unsigned char *) dst;
	while ( num-- ){
		tmp = src[3];
		tmp |= src[2] << 8;
		tmp |= src[1] << 16;
		tmp |= src[0] << 24;
		*dst++ = tmp;
		src += 4 ;
	}
}
