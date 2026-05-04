/*
 * ファイル名を加工するためのユーティリティー関数群
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _fname_c_
#include "my_malloc.h"
#include "fname.h"


/*
 * suffix とみなされる部分の開始インデックスを返す。
 * suffix が無い場合は、文字列の最後の文字の、次の位置を返す。
 */
static int fnm_suffix_pos(char * name)
{
  size_t  len;
  int i;

  len = strlen(name);

  for(i = len - 1; i >= 0; i--)
    {
      if(name[i] == '/') return (int)len;
      if(name[i] == '.') return i;
    }
  return (int)len;
}

/*
 * suffix を置き換えたと仮定した際のファイル名の長さを得る。
 * 実際の置き換えは行わない。
 */
int fnmLengthSuffixExchange(char * name, char * suffix)
{
  int pos;

  pos = fnm_suffix_pos(name);
  pos += strlen(suffix);
  if(*suffix != '.') pos++;
  return pos;
}

/*
 * 与えられたファイル名の suffix を置き換え、所定のバッファに格納する。
 */
char * fnmExchangeSuffix(char * buf, int maxlen, char * name, char * suffix)
{
  int len;
  int pos;

  len = fnmLengthSuffixExchange(name, suffix) + 1;
  if(len > maxlen) return NULL;

  pos = fnm_suffix_pos(name);
  strncpy(buf, name, pos);
  if(*suffix != '.') buf[pos++] = '.';
  strcpy(buf + pos, suffix);

  return buf;
}

/*
 * suffix の置き換えと、メモリ確保を同時に行うコンビニエンス関数
 */
char * fnmMakeSuffixExchanged(char * name, char * suffix)
{
  int max;
  char * buf;
  char * ret;

  max = fnmLengthSuffixExchange(name, suffix) + 1;
  if(NULL == (buf = MALLOC(max))) return NULL;
  if(NULL == (ret = fnmExchangeSuffix(buf, max, name, suffix))) FREE(buf);
  return ret;
}
