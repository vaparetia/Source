#include <stdio.h>
#include <stdlib.h>

#define _util_c_
#include "util.h"

int utReadWordBE(unsigned short *w, FILE *rfp)
{
  int l, h;

  if(EOF == (h = fgetc(rfp))) return EOF;
  if(EOF == (l = fgetc(rfp))) return EOF;
  *w = ((h & 0xff) << 8) | (l & 0xff);
  return 0;
}

int utReadLongBE(unsigned long *l, FILE *rfp)
{
  unsigned short lw, hw;

  if(EOF == utReadWordBE(&hw, rfp)) return EOF;
  if(EOF == utReadWordBE(&lw, rfp)) return EOF;
  *l = ((unsigned long)hw << 16) | (unsigned long)lw;
  return 0;
}

int utWriteWordBE(unsigned short w, FILE *wfp)
{
  if(EOF == fputc(w >> 8,   wfp)) return EOF;
  if(EOF == fputc(w & 0xff, wfp)) return EOF;
  return 0;
}

int utWriteLongBE(unsigned long l, FILE *wfp)
{
  if(EOF == utWriteWordBE(l >> 16,    wfp)) return EOF;
  if(EOF == utWriteWordBE(l & 0xffff, wfp)) return EOF;
  return 0;
}




int utReadWordLE(unsigned short *w, FILE *rfp)
{
  int l, h;

  if(EOF == (l = fgetc(rfp))) return EOF;
  if(EOF == (h = fgetc(rfp))) return EOF;
  *w = ((h & 0xff) << 8) | (l & 0xff);
  return 0;
}

int utReadLongLE(unsigned long *l, FILE *rfp)
{
  unsigned short lw, hw;

  if(EOF == utReadWordLE(&lw, rfp)) return EOF;
  if(EOF == utReadWordLE(&hw, rfp)) return EOF;
  *l = ((unsigned long)hw << 16) | (unsigned long)lw;
  return 0;
}

int utWriteWordLE(unsigned short w, FILE *wfp)
{
  if(EOF == fputc(w & 0xff, wfp)) return EOF;
  if(EOF == fputc(w >> 8,   wfp)) return EOF;
  return 0;
}

int utWriteLongLE(unsigned long l, FILE *wfp)
{
  if(EOF == utWriteWordLE(l & 0xffff, wfp)) return EOF;
  if(EOF == utWriteWordLE(l >> 16,    wfp)) return EOF;
  return 0;
}
