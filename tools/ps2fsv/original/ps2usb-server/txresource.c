#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _txresource_c_
#include "txresource.h"

#define BUF_STEP 256

static char * txr_read_line(FILE *rfp)
{
  void *vpt;
  char *linbuf;
  int siz;

  if(NULL == (linbuf = malloc(BUF_STEP))) return NULL;
  siz = BUF_STEP;

  if(NULL == fgets(linbuf, siz, rfp))
    {
      free(linbuf);
      return NULL;
    }

  while(*(linbuf + strlen(linbuf) - 1) != '\n')
    {
      if(NULL == (vpt = realloc(linbuf, siz + BUF_STEP)))
	{
	  free(linbuf);
	  return NULL;
	}
      linbuf = vpt;
      if(NULL == fgets(linbuf + strlen(linbuf),
		       siz + BUF_STEP - strlen(linbuf), rfp))
	break;
    }

  /* 最終的には改行は不要であるため、改行のあるところに 0 を書き込む */
  if(*(linbuf + strlen(linbuf) - 1) == '\n')
    *(linbuf + strlen(linbuf) - 1) = 0;

  if(NULL == (vpt = realloc(linbuf, strlen(linbuf) + 1)))
    {
      free(linbuf);
      return NULL;
    }

  linbuf = vpt;
  return linbuf;
}

static char * fips(char *str)
{
  int i;
  int top, tail;
  int len;

  for(i = 0; str[i] != 0; i++)
    if(str[i] != ' ' && str[i] != '\t')
      {
	top = i;
	break;
      }

  for(i = strlen(str) - 1; i >= 0; i++)
    if(str[i] != ' ' && str[i] != '\t')
      {
	tail = i;
	break;
      }
  len = tail - top + 1;

  for(i = 0; i < len; i++) str[i] = str[top + i];
  str[len] = 0;
  return str;
}

txRes * txrGetResource(FILE *rfp)
{
  char *line;
  int nums;
  txRes   * res;
  txEntry * entry;
  int i;
  int begin, len, end;
  void *vpt;
  char *tag, *value;

  nums = 0;
  while(NULL != (line = txr_read_line(rfp)))
    {
      /* '#' 以降は注釈として切り捨てる */
      for(i = 0; line[i] != 0; i++)
	if(line[i] == '#')
	  {
	    line[i] = 0;
	    break;
	  }

      /* 行の先頭からの空白は切り捨てる */
      for(i = 0; line[i] != 0; i++)
	if(line[i] != ' ' && line[i] != '\t')
	  {
	    begin = i;
	    len = strlen(line + i);
	    break;
	  }
      if(!len) continue;  /* 結果として行の長さが 0 になった場合は処理しない */

      /* ':' のある箇所で左右に分断する */
      for(i = begin; line[i] != 0; i++)
	if(line[i] == ':') break;

      if(line[i] == 0) continue;  /* ':' が見付からない場合は中断 */

      if(NULL == (tag = malloc(i - begin + 1))) goto err_0;
      if(NULL == (value = malloc(strlen(line + i))))
	{
	  free(tag);
	  goto err_0;
	}
      strncpy(tag, line + begin, i - begin);
      tag[i - begin] = 0;
      strcpy(value, line + i + 1);


      /* 前後の空白を除去 */
      fips(tag);
      fips(value);

      printf("tag:[%s] value:[%s]\n", tag, value);

      tag = realloc(tag, strlen(tag) + 1);
      value = realloc(value, strlen(value) + 1);
      
      /* txEntry 配列を成長させる */
      vpt = (!nums) ? malloc(sizeof(txEntry))
	: realloc(entry, sizeof(txEntry) * (nums + 1));

      if(NULL == vpt)
	{
	  free(tag);
	  free(value);
	  goto err_0;
	}
      entry = vpt;

      entry[nums].tag = tag;
      entry[nums].value = value;
      nums++;
      
      free(line);
    }

  if(NULL == (res = malloc(sizeof(txRes)))) goto err_1;

  res->entry = entry;
  res->nums = nums;
  return res;

 err_0:
  free(line);
 err_1:
  for(i = 0; i < nums; i++)
    {
      free(entry[i].tag);
      free(entry[i].value);
    }
  free(entry);

  return NULL;
}

/*
 * 取得したリリース構造体領域の開放
 */
void txrRelease(txRes * res)
{
  int i;

  for(i = 0; i < res->nums; i++)
    {
      free(res->entry[i].tag);
      free(res->entry[i].value);
    }
  free(res->entry);

  free(res);
}

char * txrRef(txRes * res, char * tag)
{
  int i;

  for(i = 0; i < res->nums; i++)
    if(!strcmp(res->entry[i].tag, tag))
      return res->entry[i].value;

  return NULL;
}
