#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _fileutil_c_
#include "fileutil.h"

/* ビッグエンディアン形式での 16bit データのファイルからの読み込み */
int fgetwBE(unsigned short * w, FILE *rfp)
{
  int l, h;

  if(EOF == (h = fgetc(rfp))) return EOF;
  if(EOF == (l = fgetc(rfp))) return EOF;
  *w = ((h & 0xff) << 8) | (l & 0xff);
  return 0;
}

/* ビッグエンディアン形式での 32bit データのファイルからの読み込み */
int fgetlBE(unsigned long * l, FILE *rfp)
{
  unsigned short lw, hw;

  if(EOF == (fgetwBE(&hw, rfp))) return EOF;
  if(EOF == (fgetwBE(&lw, rfp))) return EOF;
  *l = ((unsigned long)hw << 16) | (unsigned long)lw;
  return 0;
}


/* リトルエンディアン形式での 16bit データのファイルからの読み込み */
int fgetwLE(unsigned short * w, FILE *rfp)
{
  int l, h;

  if(EOF == (l = fgetc(rfp))) return EOF;
  if(EOF == (h = fgetc(rfp))) return EOF;
  *w = ((h & 0xff) << 8) | (l & 0xff);
  return 0;
}

/* リトルエンディアン形式での 32bit データのファイルからの読み込み */
int fgetlLE(unsigned long * l, FILE *rfp)
{
  unsigned short lw, hw;

  if(EOF == (fgetwLE(&lw, rfp))) return EOF;
  if(EOF == (fgetwLE(&hw, rfp))) return EOF;
  *l = ((unsigned long)hw << 16) | (unsigned long)lw;
  return 0;
}
