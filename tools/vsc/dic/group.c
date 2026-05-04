#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _group_c_
#include "group.h"
#include "token.h"


#define BUF_SIZE 256

static char tkn_buf[ BUF_SIZE ];
static struct {
  char *label;
  vGroup group;
} gr_labels[] = {
  { "W", V_W },
  { "D", V_D },
  { "C", V_C },
  { "V", V_V },
  { "F", V_F },
  { "U", V_U },
  { "S", V_S },
  {NULL, V_unknown}
};

char * gr_read_token(TOKEN *tkn)
{
  if(NULL == tknReadToken(tkn, tkn_buf, BUF_SIZE, ":")) return NULL;
  return tkn_buf;
}

grGroup **grReadGroup(TOKEN *tkn)
{
  grGroup ** g_list;
  grGroup  * gr;
  char *token;
  int nums;
  void *vpt;
  int i;

  nums = 0;
  g_list = NULL;
  while(NULL != (token = gr_read_token(tkn)))
    {
      /* 取得したトークンが, 数字のみから構成されているかどうかをチェック */
      for(i = 0; token[i]; i++)
	if(token[i] < '0' || token[i] > '9') goto err_0;
      
      if(NULL == (gr = malloc(sizeof(grGroup)))) goto err_0;
      gr->label = NULL;
      gr->begin = atoi(token);   /* 開始時刻として登録 */

      /* 間に入る ':' を取得 */
      if(NULL == (token = gr_read_token(tkn))) goto err_1;
      if(strcmp(token, ":")) goto err_1;

      /* 音素群ラベルを取得 */
      if(NULL == (token = gr_read_token(tkn))) goto err_1;

      /* 取得した音素群ラベルを元に、音素群属性を格納する */
      gr->group = V_unknown;
      for(i = 0; gr_labels[i].label != NULL; i++)
	if(!strcmp(gr_labels[i].label, token))
	  {
	    gr->group = gr_labels[i].group;
	    break;
	  }
      
      /* 人間の手で付けられた分のラベルを取得する */
      if(NULL == (token = gr_read_token(tkn))) goto err_1;

      /* ラベルの文字列を二重化して登録 */
      if(NULL == (gr->label = malloc(strlen(token) + 1))) goto err_1;
      strcpy(gr->label, token);


      /*
       * 一応のところ完成した構造体をリストに登録
       */
      vpt = (!nums)
	? malloc(sizeof(grGroup *))
	: realloc(g_list, sizeof(grGroup *) * (nums + 1));
      if(vpt == NULL) goto err_2;
      g_list = vpt;
      g_list[nums] = gr;
      nums++;
    }
  
  /* 終点となる NULL を書き込む */
  vpt = (!nums)
    ? malloc(sizeof(grGroup *))
    : realloc(g_list, sizeof(grGroup *) * (nums + 1));
  if(vpt == NULL) goto err_2;
  g_list = vpt;
  
  g_list[nums] = NULL;

  /* 全ての構造体において、終了時刻を生成する */
  for(i = 0; g_list[i] != NULL; i++)
    g_list[i]->end =(g_list[i + 1] != NULL)
      ? ( g_list[i + 1]->begin - 1 )
      : -1;

  return g_list;  /* 生成したリストを返す */

 err_2:
  if(NULL == gr->label) free(gr->label);
 err_1:
  free(gr);
 err_0:
  for(i = 0; i < nums; i++)
    if(g_list[i]->label != NULL) free(g_list[i]->label);
      
  free(g_list[i]);
  free(g_list);

  return NULL;
}


void grReleaseGroup(grGroup **g_list)
{
  int i;

  for(i = 0; g_list[i] != NULL; i++)
    {
      if(g_list[i]->label != NULL) free(g_list[i]->label);
      free(g_list[i]);
    }
  free(g_list);
}
