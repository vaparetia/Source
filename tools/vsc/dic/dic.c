#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _dic_c_
#include "vec.h"
#include "group.h"
#include "dic.h"
#include "token.h"
#include "util.h"

/* #define _DEBUG_ */

#ifdef _DEBUG_
#define DEBUG(args...)  fprintf(stderr, args);  fflush(stderr)
#else
#define DEBUG(args...)
#endif /* _DEBUG_ */


#define BUF_SIZ 256

static struct {
  char *label;
  vSubset subset;
} subset_list[] = {
  {"W",  SS_W  },
  {"C",  SS_C  },
  {"U",  SS_U  },
  {"SV", SS_SV },
  {"CW", SS_CW },
  {"UW", SS_UW },
  {"UU", SS_UU },
  {"F",  SS_F  },
  {NULL, SS_unknown}
};

static char tkn_buf[ BUF_SIZ ];


static dicREF ** dic_append_list(dicREF ** r_list, int *nums,
				 vSubset subset, vVecMode mode,
				 vecVector * vec, grGroup * group)
{
  dicREF * ref;
  vecCep * cep;
  void   * vpt;
  int i, j;

  if(NULL == (ref = malloc(sizeof(dicREF)))) return NULL;

  ref->subset = subset;
  switch(mode)
    {
    case vec_ALL:     /* 全体型のベクトルを使用する場合   */
      cep = vec->all;
      break;
    case vec_CENTER:  /* 定常部型のベクトルを使用する場合 */
      cep = vec->center;
      break;
    case vec_BORDER:  /* 過渡部型のベクトルを使用する場合 */
      cep = vec->border;
      break;
    }

  ref->subset = subset;
  for(i = 0; i < 5; i++)
    {
      ref->vector[i].dim = cep[i].dim;
      if(NULL == (ref->vector[i].cep = malloc(sizeof(double) * cep[i].dim)))
	{
	  free(ref);
	  return NULL;
	}
      for(j = 0; j < cep[i].dim; j++)
	ref->vector[i].cep[j] = cep[i].cep[j];
    }
  ref->label = utilDupString(group->label);

#ifdef _DEBUG_
  fprintf(stderr, "Number: %d\n", *nums);
  fprintf(stderr, "subset: ");
  switch(ref->subset)
    {
    case SS_W:   fprintf(stderr, "W\n");  break;
    case SS_C:   fprintf(stderr, "C\n");  break;
    case SS_U:   fprintf(stderr, "U\n");  break;
    case SS_SV:  fprintf(stderr, "SV\n"); break;
    case SS_CW:  fprintf(stderr, "CW\n"); break;
    case SS_UU:  fprintf(stderr, "UU\n"); break;
    case SS_F:   fprintf(stderr, "F\n");  break;
    }
  fprintf(stderr, "label: %s\n", ref->label);
#endif /* _DEBUG_ */

  vpt = (!*nums)
    ? malloc(sizeof(dicREF *))
    : realloc(r_list, sizeof(dicREF *) * (*nums + 1));
  if(vpt == NULL)
    {
      free(ref);
      return NULL;
    }

  r_list = vpt;
  r_list[*nums] = ref;
  (*nums)++;
  return r_list;
}


dicREF ** dicMatchVectorAndGroup(vecVector **v_list, grGroup **g_list)
{
  dicREF ** r_list;
  int nums;
  int i, j;
  int begin, end;
  int v_len, g_len;
  void * vpt;

  nums = 0;
  r_list = NULL;
  for(i = 0; g_list[i] != NULL; i++)
    {
      DEBUG("start-----\n");
      for(j = 0; v_list[j] != NULL; j++)
	{
		 
	  /* ラベル側の区間との重複領域を調べ,
	     その長さがラベル側の 65% を越えていたら,
	     対応する音素セグメントであるとみなす。 */
	  
	  /* あきらかに領域が重複しない場合の処理 */
	  if(v_list[j]->begin > g_list[i]->end) continue;
	  if(v_list[j]->end < g_list[i]->begin) continue;

	  /* 少しでも領域が重複するなら、重複している領域を割出す */
	  begin = (g_list[i]->begin < v_list[j]->begin)
	    ? v_list[j]->begin : g_list[i]->begin;

	  end = (g_list[i]->end > v_list[j]->end)
	    ? v_list[j]->end : g_list[i]->end;

	  if(begin > end) continue;

	  g_len = (g_list[i]->end - g_list[i]->begin) + 1;
	  v_len = (end - begin) + 1;

	  /* 重複部分の長さが,グループラベル側の
	     65% 未満であれば処理対象としない。 */
	  if((v_len * 100 / g_len) < 65) continue;

	  DEBUG("begin: %d  end: %d\n", begin, end);
	  DEBUG("---  1  ---\n");


	  /* グループによって、そのベクトルを
	     どのリファレンスサブセットに振り分けるかを決定する */
	  switch(g_list[i]->group)
	    {
	    case V_W:
	      DEBUG("V_W\n");
	      r_list = dic_append_list(r_list, &nums, SS_W,
				       vec_CENTER, v_list[j], g_list[i]);
	      break;

	    case V_D:
	    case V_C:
	      DEBUG("V_D or V_C\n");
	      if(g_list[i+1] != NULL)
		if(g_list[i + 1]->group == V_W || g_list[i + 1]->group == V_V)
		  r_list = dic_append_list(r_list, &nums, SS_CW,
					   vec_BORDER, v_list[j], g_list[i]);

	      /* D, C, V はおなじカテゴリとして扱うので,
		 この箇所の break は不要 */

	    case V_V:
	      DEBUG("V_V\n");

	      /* C, D, V(W 除く) は、必ずこの処理を行う */
	      r_list = dic_append_list(r_list, &nums, SS_C,
				       vec_CENTER, v_list[j], g_list[i]);
	      break;

	    case V_F:
	    case V_U:
	      DEBUG("V_F or V_U\n");

	      r_list = dic_append_list(r_list, &nums, SS_U,
				       vec_CENTER, v_list[j], g_list[i]);
	      
	      if(g_list[i+1] != NULL)
		if(g_list[i + 1]->group == V_W ||
		   g_list[i + 1]->group == V_V)
		  {
		    r_list = dic_append_list(r_list, &nums, SS_UW,
					     vec_BORDER, v_list[j], g_list[i]);
		  }
	      
	      DEBUG("{1}\n");
	      if(g_list[i+1] != NULL)
		if(g_list[i + 1]->group == V_U ||
		   g_list[i + 1]->group == V_F)
		  {
		    r_list = dic_append_list(r_list, &nums, SS_UU,
					     vec_BORDER, v_list[j], g_list[i]);
		  }

	      DEBUG("{2}\n");

	      /* 長さが 6 フレーム以上あり、後続音がある F, U は F サブセット
		 としても扱う   */
	      if(g_list[i + 1] != NULL)
		if(((g_list[i]->end - g_list[i]->begin + 1) >= 6) &&
		   (g_list[i + 1]->group != V_S))
		  {
		    DEBUG("-1\n");
		    r_list = dic_append_list(r_list, &nums, SS_F,
					     vec_ALL, v_list[j], g_list[i]);
		  }
	      DEBUG("{3}\n");

	      break;

	    case V_S:

	      DEBUG("V_S\n");

	      if(g_list[i+1] != NULL)
		if(g_list[i + 1]->group == V_W ||
		   g_list[i + 1]->group == V_V)
		  r_list = dic_append_list(r_list, &nums, SS_SV,
					   vec_BORDER, v_list[j], g_list[i]);
	      break;
	    }
	  DEBUG("---  2  ---\n");
	  break;
	}
      DEBUG("------next\n");
    }

  vpt = (!nums)
    ? malloc(sizeof(dicREF *))
    : realloc(r_list, sizeof(dicREF *) * (nums + 1));
  if(vpt == NULL) ;
  r_list = vpt;
  r_list[nums] = NULL;

  return r_list;

 err:
  for(i = 0; i < nums; i++)
    free(r_list[i]);
  if(i > 0) free(r_list);
  return NULL;
}

static int dic_append_subset(dicSubSET *sub, dicREF *ref)
{
  void *vpt;

  vpt = (!sub->nums)
    ? malloc(sizeof(dicREF *))
    : realloc(sub->ref, sizeof(dicREF *) * (sub->nums + 1));
  if(vpt == NULL) return -1;
  sub->ref = vpt;
  sub->ref[sub->nums] = ref;
  sub->nums++;
  return 0;
}


/* 作成したリファレンスを,既に読み込まれている dicDicROOT 構造体に追加する */
int dicAppendDictionaly(dicDicROOT *root, dicREF **r_list)
{
  int i;
  int ret;
  dicSubSET *set;

  ret = 0;
  for(i = 0; r_list[i] != NULL; i++)
    {
      set = NULL;
      switch(r_list[i]->subset)
	{
	case SS_W:  set = &(root->W);  break;
	case SS_C:  set = &(root->C);  break;
	case SS_U:  set = &(root->U);  break;
	case SS_SV: set = &(root->SV); break;
	case SS_CW: set = &(root->CW); break;
	case SS_UW: set = &(root->UW); break;
	case SS_UU: set = &(root->UU); break;
	case SS_F:  set = &(root->F);  break;
	}
      if(NULL == set)
	{
	  fprintf(stderr, "unknown group.\n");
	  exit(EXIT_FAILURE);
	}
      if(dic_append_subset(set, r_list[i])) ret = -1;
    }
  return -1;
}

static char * dic_read_token(TOKEN *tkn)
{
  if(NULL == tknReadToken(tkn, tkn_buf, BUF_SIZ, "<>")) return NULL;

  if(tkn_buf[0] == '<')
    if(NULL == tknReadWhileTerminator(tkn, tkn_buf + 1, BUF_SIZ - 1, ">"))
      return NULL;

  return tkn_buf;
}

static vecCep * dic_read_vector(TOKEN *tkn, vecCep *cep)
{
  char *token;
  void *vpt;

  cep->dim = 0;
  while(NULL != (token = dic_read_token(tkn)))
    {
      if(!strcmp(token, "</vec>")) break;

      vpt = (!cep->dim)
	? malloc(sizeof(double))
	: realloc(cep->cep, sizeof(double) * (cep->dim + 1));
      if(NULL == vpt)
	{
	  free(cep->cep);
	  return NULL;
	}
      cep->cep = vpt;
      cep->cep[cep->dim] = atof(token);
      cep->dim++;
    }
  return cep;
}

static dicREF * dic_read_ref(TOKEN *tkn, vSubset subset)
{
  dicREF *ref;
  char *token;
  int vec_num;
  int dim;
  void *vpt;


  if(NULL == (ref = malloc(sizeof(dicREF)))) return NULL;
  ref->subset = subset;
  vec_num = 0;

  while(NULL != (token = dic_read_token(tkn)))
    {
      if(!strcmp(token, "</ref>")) break;

      if(!strcmp(token, "<label>"))
	{
	  if(NULL == (token = dic_read_token(tkn))) goto err0;
	  if(NULL == (vpt = malloc(strlen(token) + 1))) goto err0;
	  ref->label = vpt;
	  strcpy(ref->label, token);
	  
	  if(NULL == (token = dic_read_token(tkn))) goto err1;
	  if(strcmp(token, "</label>")) goto err1;	  
	  continue;
	}
      if(!strcmp(token, "<vec>"))
	{
	  if(vec_num >= 5) goto err1;
	  if(NULL == dic_read_vector(tkn, &(ref->vector[vec_num]))) goto err1;
	  vec_num++;
	}
    }
  return ref;

 err1:
  free(ref->label);
 err0:
  free(ref);
  return NULL;
}

/*
 * サブセットを読み込む
 */
static dicSubSET * dic_read_subset(TOKEN *tkn, dicSubSET *sub, vSubset subset)
{
  char   * token;
  dicREF * ref;
  void   * vpt;

  while(NULL != (token = dic_read_token(tkn)))
    {
      if(!strcmp(token, "</subset>")) break;

      if(!strcmp(token, "<ref>"))
	{
	  if(NULL == (ref = dic_read_ref(tkn, subset))) goto err0;

	  vpt = (!sub->nums)
	    ? malloc(sizeof(dicREF *))
	    : realloc(sub->ref, sizeof(dicREF *) * (sub->nums + 1));

	  if(NULL == vpt) goto err1;
	  sub->ref = vpt;
	  sub->ref[sub->nums] = ref;
	  sub->nums++;
	}
    }

  return sub;

 err1:
 err0:
  return NULL;
}

/*
 * 現在の音韻辞書を読み込み,dicDicROOT 構造体に格納する
 */
dicDicROOT * dicReadDictionaly(TOKEN *tkn)
{
  dicDicROOT *dic;
  dicSubSET  *set;
  char *token;
  int i, j;

  /* 読み込み先の構造体を用意 */
  if(NULL == (dic = malloc(sizeof(dicDicROOT)))) return NULL;

  dic->W.nums  = 0;
  dic->C.nums  = 0;
  dic->U.nums  = 0;
  dic->SV.nums = 0;
  dic->CW.nums = 0;
  dic->UW.nums = 0;
  dic->UU.nums = 0;
  dic->F.nums  = 0;

  /* 辞書の読み込み */
  while(NULL != (token = dic_read_token(tkn)))
    {
      if(!strcmp(token, "<subset>"))
	{
	  if(NULL == (token = dic_read_token(tkn))) break;
	  
	  for(i = 0; subset_list[i].label != NULL; i++)
	    if(!strcmp(subset_list[i].label, token))
	      {
		switch(subset_list[i].subset)
		  {
		  case SS_W:  set = &(dic->W);  break;
		  case SS_C:  set = &(dic->C);  break;
		  case SS_U:  set = &(dic->U);  break;
		  case SS_SV: set = &(dic->SV); break;
		  case SS_CW: set = &(dic->CW); break;
		  case SS_UW: set = &(dic->UW); break;
		  case SS_UU: set = &(dic->UU); break;
		  case SS_F:  set = &(dic->F);  break;
		  default:
		    fprintf(stderr, "bad subset.\n");
		    exit(EXIT_FAILURE);
		  }
		dic_read_subset(tkn, set, subset_list[i].subset);
		break;
	      }
	}
    }

  return dic;
}


static int dic_write_vector(FILE *wfp, vecCep * cep)
{
  int i;

  fprintf(wfp, "    <vec>");
  for(i = 0; i < cep->dim; i++)
    {
      if(!(i % 5)) fprintf(wfp, "\n    ");
      fprintf(wfp, "  %8.3f", cep->cep[i]);
    }
  fprintf(wfp, "\n    </vec>\n");
  return 0;
}


static int dic_write_ref(FILE *wfp, dicREF *ref)
{
  int i;

  fprintf(wfp, "  <ref>\n");
  fprintf(wfp, "    <label>%s</label>\n", ref->label);
  for(i = 0; i < 5; i++) dic_write_vector(wfp, &(ref->vector[i]));
  fprintf(wfp, "  </ref>\n\n");
  return 0;
}

static int dic_write_subset(FILE *wfp, dicSubSET *sub, char *sublabel)
{
  int i;

  fprintf(wfp, "<subset>%s\n", sublabel);
  for(i = 0; i <  sub->nums; i++)
    dic_write_ref(wfp, sub->ref[i]);
  fprintf(wfp, "</subset>\n\n");
  return 0;
}

int dicWriteDictionaly(FILE *wfp, dicDicROOT *dic)
{
  dic_write_subset(wfp, &(dic->W),  "W");
  dic_write_subset(wfp, &(dic->C),  "C");
  dic_write_subset(wfp, &(dic->U),  "U");
  dic_write_subset(wfp, &(dic->SV), "SV");
  dic_write_subset(wfp, &(dic->CW), "CW");
  dic_write_subset(wfp, &(dic->UW), "UW");
  dic_write_subset(wfp, &(dic->UU), "UU");
  dic_write_subset(wfp, &(dic->F),  "F");

  return 0;
}

dicDicROOT * dicNewDictionaly(void)
{
  dicDicROOT * dic;

  if(NULL == (dic = malloc(sizeof(dicDicROOT)))) return NULL;
  
  dic->W.nums  = 0;
  dic->C.nums  = 0;
  dic->U.nums  = 0;
  dic->SV.nums = 0;
  dic->CW.nums = 0;
  dic->UW.nums = 0;
  dic->UU.nums = 0;
  dic->F.nums  = 0;

  return dic;
}
