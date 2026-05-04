#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _futil_c_
#include "futil.h"

int fgetl(unsigned long * l, FILE *rfp)
{
  unsigned char buf[4];
  int i;

  if(fread(buf, 1, 4, rfp) < 4) return EOF;
  *l = 0;
  for(i = 0; i < 4; i++)
    *l |= ((unsigned long)buf[i] << (i * 8));
  return 0;
}

int fgetw(unsigned short * w, FILE *rfp)
{
  unsigned char buf[2];
  int i;

  if(fread(buf, 1, 2, rfp) < 2) return EOF;
  *w = 0;
  for(i = 0; i < 2; i++)
    *w |= ((unsigned short)buf[i] << (i * 8));
  return 0;
}

int fputl(unsigned long l, FILE *wfp)
{
  int i, c;

  for(i = 0; i < 4; i++)
    {
      c = (int)(l >> (8 * i)) & 0xff;
      if(EOF == fputc(c, wfp)) return EOF;
    }
  return 0;
}

int fputw(unsigned short w, FILE *wfp)
{
  int i, c;

  for(i = 0; i < 2; i++)
    {
      c = (int)(w >> (8 * i)) & 0xff;
      if(EOF == fputc(c, wfp)) return EOF;
    }
  return 0;
}

int fput_zero_fill(int len, FILE *wfp)
{
  int i;

  for(i = 0; i < len; i++) if(EOF == fputc(0, wfp)) return EOF;
  return 0;
}
