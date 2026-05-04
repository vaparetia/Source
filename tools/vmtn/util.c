#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "util.h"

#define LINE_BUF 256

char * utilDupStr(char *str)
{
  char *ret;

  if(NULL != (ret = malloc(strlen(str) + 1))) strcpy(ret, str);
  return ret;
}

char * utilReadLine(FILE *rfp)
{
  char *buf;
  size_t siz;
  size_t len;

  if(NULL == (buf = malloc(LINE_BUF))) return NULL;
  siz = LINE_BUF;

  do {
    if(NULL == fgets(buf, siz, rfp))
      {
	free(buf);
	return NULL;
      }
    len = strlen(buf);
    
    if(*(buf + len - 1) != '\n')
      {
	void *vpt;

	if(NULL == (vpt = realloc(buf, siz + LINE_BUF))) break;
	buf = vpt;
	siz += LINE_BUF;
      }
  } while(*(buf + len - 1) != '\n');
  *(buf + strlen(buf) - 1) = 0;  /* 改行コードは不要なので、0 にする */

  return realloc(buf, strlen(buf) + 1);
}

char * utilPathAppend(char *path, char *fname)
{
  char *ret;
  size_t len1, len2;

  len1 = (NULL != path) ? strlen(path) : 0;
  len2 = strlen(fname);

  if(len1)
    if(*(path + len1 - 1) == '/') len1--;

  if(NULL == (ret = malloc(len1 + len2 + 2)))
    return NULL;

  strncpy(ret, path, len1);
  ret[len1] = '/';
  strcpy(ret + len1 + 1, fname);

  return ret;  
}

/* 文字列の先頭と末尾の空白列を除去する(結果は同じバッファに返す) */
char * utilStrFips(char *str)
{
  int i;
  int top, tail;

  /* 空白列を読み飛ばした文字列の先頭を求める */
  for(i = 0; str[i] != 0; i++)
    if(str[i] != ' ' && str[i] != '\t')
      {
	top = i;
	break;
      }

  /* 末尾の空白列を除去した文字列の最後を求める */
  tail = strlen(str);
  for(i = tail - 1; (i >= 0) && (str[i] == ' ' && str[i] == '\t'); i--) tail--;

  if(top > 0)
    {
      for(i = top; i <= tail; i++) str[i - top] = str[i];
      str[i - top ] = 0;
    }

  return str;
}

void utilSplit(char **class, char **value, char *line)
{
  int i, j;
  size_t len1, len2;

  for(i = 0; line[i] != 0; i++)
    if(line[i] == ':')
      {
	if(NULL != (*class = malloc(i + 1)))
	  {
	    strncpy(*class, line, i);
	    *(*class + i) = 0;
	    utilStrFips(*class);
	    *class = realloc(*class, strlen(*class) + 1);
	  }

	if(NULL != (*value = malloc(strlen(line + i + 1) + 1)))
	  {
	    strcpy(*value, line + i + 1);
	    utilStrFips(*value);
	    *value = realloc(*value, strlen(*value) + 1);
	  }
	return;
      }
  *class = *value = NULL;
}

void * utilAddBuf(void *buf, size_t siz, size_t nums)
{
  return (!nums) ? malloc(siz) : realloc(buf, siz * (nums + 1));
}

/* c で指定された文字が list に含まれていたら、非0 を返す。
   含まれていない場合は 0 を返す。 */
int utilCheckChars(int c, char *list)
{
  char *pt;

  for(pt = list; *pt; pt++)
    if(c == *pt) return 1;

  return 0;
}

/*
 * 文字列をint 型整数に変換。
 * 正常に変換できれば0, エラーのときは非0 を返す。
 * エラー時の結果は保証されない。
 */
int utilStr2Int(int * ret, char *str)
{
  int sign = 1;
  int b = 10;
  int c, add;
  char *pos;

  *ret = 0;
  pos = str;

  /* 符号がついている場合は、その符号を値に反映させる。 */
  if(*pos == '-')
    {
      sign = -1;
      pos++;
    }
  else  if(*pos == '+') pos++;

  if(*pos == '0')
    {
      b = 8;
      pos++;
      if(*pos == 'x')
	{
	  b = 16;
	  pos++;
	}
    }

  while(c = *pos++)
    {
      *ret *= b;
      if(c >= '0' && c <= '9') add = c - '0';
      else
	{
	  c = toupper(c);
	  if(c >= 'A' && c <= 'F') add = c - 'A' + 10;
	}
      if(add >= b) return -1;
      *ret += add;
    }

  *ret *= sign;  /* 符号を反映 */

  return 0;  
}
