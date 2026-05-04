#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _util_c_
#include "debug.h"
#include "my_malloc.h"
#include "util.h"

/* 指数のバイアス値（2^0となる指数値） */
#define E_BIAS	(8)		/* メタルギアでは大きい値を重視する */

#define BIT_LEN  24    /* StrCode 生成で使用する */

int search_char(int c, char *str)
{
  char *ptr;

  for(ptr = str; *ptr; ptr++)
    if(c == *ptr) return 1;
  return 0;
}

/*
 * 数字からなる文字列を整数値に変換する。
 * エラー検出可能。
 */
int str_to_int(int * num, char * str)
{
  char * pt;
  int sign = 1;
  int ans = 0;
  int base = 10;
  int c, n;

  pt = str;
  /* 符号の処理があれば、それを処理する */
  if(*pt == '-' || *pt == '+')
    {
      sign = (*pt == '-' ) ? -1 : 1;
      pt++;
    }

  if(*pt == '0')
    {
      pt++;
      if(toupper(*pt) == 'X')
	{
	  base = 16;  /* "0x" で開始されていれば16進数として解釈 */
	  pt++;
	}
      else
	base = 8;     /* "0" で開始されていれば 8進数として解釈  */
    }

  while((c = *pt++))
    {
      ans *= base;
      if(c >= '0' && c <= '9') n = c - '0';
      else
	if(tolower(c) >= 'a' && tolower(c) <= 'f')
	  n = tolower(c) - 'a' + 10;
	else
	  return -1;

      if(n < 0 || n >= base) return -1;

      ans += n;
    }
  
  *num = ans * sign;

  return 0;
}

int str_to_float(float * fnum, char * str)
{
  char * pt;
  float sign = 1.0F;
  float ans = 0.0F;
  float fn, ax, nx, xx;
  int c, n;

  pt = str;
  /* 符号の処理があれば、それを処理する */
  if(*pt == '-' || *pt == '+')
    {
      sign = (*pt == '-' ) ? -1.0F : 1.0F;
      pt++;
    }

  ax = 10.0F;
  nx = 1.0F;
  xx = 1.0F;
  while((c = *pt++))
    {
      if(c == '.')
	{
	  ax = 1.0F;
	  xx = nx = 0.1F;
	  continue;
	}
      if(c >= '0' && c <= '9') n = c - '0';
      else
	return -1;

      if(n < 0 || n >= 10) return -1;

      fn = nx * (float)n;
      ans *= ax;
      nx *= xx;
      ans += fn;
    }
  
  *fnum = ans * sign;

  return 0;
}




/*
 * 与えられた文字列の StrCode を求め、返す
 */
int strcode(char * string)
{
  unsigned char c;
  unsigned char *p;
  unsigned int id, mask;
  
  p = ( unsigned char * )string;
  id = 0;
  mask = ( 1 << BIT_LEN ) - 1;
  
  while( ( c = *( p++ ) ) != '\0' ){
    id = ( id << 5 ) | ( id >> (BIT_LEN-5) );
    id += c;
    id &= mask;
  }
  if( id == 0 ) id = 1;
  
  return id;
}

/*
 * long 型をリトルエンディアンでファイルに書き出す
 */
int fputl(long l, FILE * wfp)
{
  unsigned char buf[4];
  int i;

  for(i = 0; i < 4; i++)
    buf[i] = ((unsigned long)l >> (8 * i)) & 0xff;

  if(4 != fwrite(buf, 1, 4, wfp)) return EOF;
  return 0;
}

/*
 * short 型をリトルエンディアンでファイルに書き出す
 */
int fputw(short s, FILE * wfp)
{
  unsigned char buf[2];
  int i;

  for(i = 0; i < 2; i++)
    buf[i] = ((unsigned short)s >> (8 * i)) & 0xff;
  if(2 != fwrite(buf, 1, 2, wfp)) return EOF;
  return 0;
}

/*
 * float 型を、16bit 浮動小数点数値に変換する
 */
unsigned short short_float(float f)
{
  unsigned int	data, s_dat, e_dat, f_dat ;
  unsigned short	mf ;
  
  data = *(unsigned int*)&f ;
  /* 符合取りだし */
  s_dat = ( data & 0x80000000 ) >> 16 ;
  /* 指数取りだし */
  e_dat = ( data & 0x7f800000 ) ;
  if ( e_dat != 0 ){
    if ( e_dat <= ( 127 - E_BIAS ) << 23 ){
      //printf("convert error!!\n");
      return ( 0x0000 | s_dat );
    }
    if ( e_dat > ( 127 - E_BIAS + 31 ) << 23 ){
      //printf("convert error!!\n");
      return ( 0x7fff | s_dat );
    }
    e_dat -= ( 127 - E_BIAS ) << 23 ;
    e_dat >>= 13 ;
  }
  /* 仮数部取りだし */
  f_dat = ( data & 0x007fffff ) >> 13 ;
  
  /* ビット合成 */
  mf = s_dat | e_dat | f_dat ;
  
  return ( mf );
}

