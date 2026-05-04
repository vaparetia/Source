#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _util_c_
#include "util.h"

/* 16bit 書き込み */
int utWriteWord(unsigned short w, FILE *wfp)
{
  if(EOF == fputc(w & 0xff, wfp)) return EOF;
  if(EOF == fputc(w >> 8, wfp)) return EOF;

  return 0;
}

/* 32bit 書き込み */
int utWriteLong(unsigned long l, FILE *wfp)
{
  if(EOF == utWriteWord(l & 0xffff, wfp)) return EOF;
  if(EOF == utWriteWord(l >> 16, wfp)) return EOF;

  return 0;
}

/* Big Endian 16bit 書き込み */
int utWriteWordBE(unsigned short w, FILE *wfp)
{
  if(EOF == fputc(w >> 8, wfp)) return EOF;
  if(EOF == fputc(w & 0xff, wfp)) return EOF;

  return 0;
}

/* Big Endian 32bit 書き込み */
int utWriteLongBE(unsigned long l, FILE *wfp)
{
  if(EOF == utWriteWordBE(l >> 16, wfp)) return EOF;
  if(EOF == utWriteWordBE(l & 0xffff, wfp)) return EOF;

  return 0;
}
