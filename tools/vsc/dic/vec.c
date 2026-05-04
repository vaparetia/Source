#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _vec_c_
#include "vec.h"
#include "token.h"

/* #define _DEBUG_ */

#ifdef _DEBUG_
#define DEBUG(args...)    fprintf(stderr, args); fflush(stderr)
#else
#define DEBUG(args...)
#endif /* _DEBUG_ */


#define BUF_LEN  256

static char tkn_buf[ BUF_LEN ];

/*
 * トークンを一つ読み込む
 */
char * vec_read_token(TOKEN *tkn)
{
  if(NULL == tknReadToken(tkn, tkn_buf, BUF_LEN, "<>")) return NULL;

  /*
   * タグであればタグ全体を読み込む
   */
  if(tkn_buf[0] == '<')
    if(NULL == tknReadWhileTerminator(tkn, tkn_buf + 1, BUF_LEN - 1, ">"))
      return NULL;

  return tkn_buf;
}

/*
 * メルケプストラムブロックを一つ読み込む
 * ※先頭の <vec> は既に読み込まれているものとする
 */
static vecCep * vec_read_cepstrum(TOKEN *tkn, vecCep *cep)
{
  char *token;
  void *vpt;

  DEBUG("vec_read_cepstrum()\n");

  cep->dim = 0;
  while(NULL != (token = vec_read_token(tkn)))
    {
      if(!strcmp(token, "</vec>")) break;

      vpt = (!cep->dim)
	? malloc(sizeof(double))
	: realloc(cep->cep, sizeof(double) * (cep->dim + 1));
      if(vpt == NULL) return NULL;
      cep->cep = vpt;
      cep->cep[cep->dim] = atof(token);
      cep->dim++;
    }

  DEBUG("end: vec_read_cepstrum()\n");

  return cep;
}

/*
 * 特徴ベクトルブロックの読み込み
 * 全体型,定常部型、過渡部型などの単位での読み込みを行う。
 */
vecCep * vec_read_vec_block(TOKEN *tkn, vecCep * vec,
			    int max_nums, vVecMode mode)
{
  static char * term[] = {"</all>", "</static>", "</border>"};
  int nums;
  int term_num;
  char *token;
  
  DEBUG("vec_read_vec_block()\n");

  switch(mode)
    {
    case vec_ALL:    term_num = 0; break;
    case vec_CENTER: term_num = 1; break;
    case vec_BORDER: term_num = 2; break;
    }

  nums = 0;
  while(NULL != (token = vec_read_token(tkn)))
    {
      if(!strcmp(token, term[term_num])) break;

      if(!strcmp(token, "<vec>"))
	{
	  if(nums >= max_nums) return NULL;  /* 最大数を越えている場合 */
	  if(NULL == vec_read_cepstrum(tkn, &vec[nums])) return NULL;
	  nums++;
	  continue;
	}
      return NULL;
    }
  DEBUG("end: vec_read_vec_block()\n");

  return vec;
}

/*
 * 音素セグメントの時刻を得る
 */
int vec_read_time(TOKEN *tkn, int *begin,  int *end)
{
  char *token;
  int er;
  int i, pos;

  DEBUG("vec_read_time()\n");

  while(NULL != (token = vec_read_token(tkn)))
    {
      if(!strcmp(token, "</time>")) break;

      /* 時刻を表すトークンが,数字とハイフン以外を含んでいたらエラー */
      er = 0;
      for(i = 0; token[i]; i++)
	if(token[i] != '-' &&
	   (token[i] < '0' || token[i] > '9')) er = 1;
      if(er) return -1;

      /* ハイフンのある位置で分ける */
      for(i = 0; token[i]; i++)
	if(token[i] == '-')
	  {
	    pos = i;
	    break;
	  }

      token[pos] = 0;
      *begin = atoi(token);
      *end   = atoi(token + pos + 1);
    }

  DEBUG("end: vec_read_time()\n");

  return 0;
}

/*
 * vecVector 構造体に、特徴ベクトルを読み込む
 */
static vecVector * vec_read_vector(TOKEN *tkn, vecVector *vector)
{
  char *token;

  DEBUG("vec_read_vector()\n");
  while(NULL != (token = vec_read_token(tkn)))
    {
      if(!strcmp(token, "</vector>")) break;
      if(!strcmp(token, "<time>"))
	{
	  if(vec_read_time(tkn, &(vector->begin), &(vector->end))) return NULL;
	  continue;
	}

      if(!strcmp(token, "<all>"))
	{
	  if(NULL == vec_read_vec_block(tkn, vector->all, 5, vec_ALL))
	    return NULL;
	  continue;
	}

      if(!strcmp(token, "<static>"))
	{
	  if(NULL == vec_read_vec_block(tkn, vector->center, 5, vec_CENTER))
	    return NULL;
	  continue;
	}

      if(!strcmp(token, "<border>"))
	{
	  if(NULL == vec_read_vec_block(tkn, vector->border, 5, vec_BORDER))
	    return NULL;
	  continue;
	}
    }
  DEBUG("end: vec_read_vector()\n");

  return vector;
}

/*
 * vecVector 構造体に使用しているメモリヒープを開放する
 */
static void vec_release_vector(vecVector *vec)
{
  int i;

  if(vec->label != NULL) free(vec->label);
  for(i = 0; i < 5; i++)
    {
      free(vec->all[i].cep);
      free(vec->center[i].cep);
      free(vec->border[i].cep);
    }

  free(vec);
}


/*
 * vsc の -V オプション時の出力から、
 * vecVector 構造体へのポインタの配列を作成する
 */
vecVector ** vecReadVector(TOKEN *tkn)
{
  vecVector ** v_list;
  vecVector  * vec;
  char       * token;
  int          nums;
  void       * vpt;
  int i;

  nums = 0;
  v_list = NULL;
  while(NULL != (token = vec_read_token(tkn)))
    {
      if(!strcmp(token, "<vector>"))
	{
	  if(NULL == (vec = malloc(sizeof(vecVector)))) goto err0;
	  vec->label = NULL;
	  if(NULL == vec_read_vector(tkn, vec))
	    {
	      free(vec);
	      goto err0;
	    }

	  vpt = (!nums)
	    ? malloc(sizeof(vecVector *))
	    : realloc(v_list, sizeof(vecVector *) * (nums + 1));
	  if(vpt == NULL) goto err1;
	  v_list = vpt;
	  v_list[nums] = vec;
	  nums++;
	}
    }

  DEBUG("end: vecReadVector()\n");

  vpt = (!nums)
    ? malloc(sizeof(vecVector *))
    : realloc(v_list, sizeof(vecVector *) * (nums + 1));
  if(vpt == NULL) goto err1;
  v_list = vpt;
  v_list[nums] = NULL;

  DEBUG("terimation.\n");
  
  return v_list;

 err0:
  for(i = 0; i < nums; i++)
    vec_release_vector(v_list[i]);
 err1:
  free(v_list);
  return NULL;
}

void vecReleaseVector(vecVector **v_list)
{
  int i, j;

  for(i = 0; v_list[i] != NULL; i++)
    vec_release_vector(v_list[i]);
  free(v_list);
}
