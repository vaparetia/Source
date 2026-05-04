#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _lang_env_c_
#include "lang_env.h"
#include "lip_english.h"
#include "lip_japanese.h"
#include "lip.h"

static Seq ** adjust_japanese(Seq ** list);
static Seq ** adjust_english(Seq ** list);

/* 母音とみなせる英語の音素リスト */
static int english_vowel[] = {
  LEN_at,
  LEN_E,
  LEN_I,
  LEN_i,
  LEN_A,
  LEN_R,
  LEN_c,
  LEN_a,
  LEN_U,
  LEN_u,
  LEN_aI,
  LEN_aU,
  LEN_EI,
  LEN_cI,
  LEN_cU,
  LEN_bar,
  LEN_x,
  LEN_X,
  LEN_S,
  -1
};
  
/*
 * 日本語の音韻記号と音素 ID の対応表
 * 音韻記号としては、訓令式ローマ字に近いものを用いる。
 */
static ConvTable conv_list_japanese[] = {
  /* 母音 */
  {"a", LJP_a}, {"i", LJP_i}, {"u", LJP_u}, {"e", LJP_e}, {"o", LJP_o},
  {"N", LJP_N},

  /* 半母音 */
  {"wa", LJP_wa}, {"ya", LJP_ya}, {"yu", LJP_yu}, {"yo", LJP_yo},

  /* 子音 */

  /* 後続母音の形状になる子音 */
  {"k", LJP_k}, {"h", LJP_h}, {"r", LJP_r}, {"g", LJP_g}, {"w", LJP_w},
  
  /* /u/ の形状から母音形状へ変化する子音 */
  {"s", LJP_s}, {"t", LJP_t}, {"n", LJP_n}, {"z", LJP_z}, {"ts", LJP_ts},
  {"d", LJP_d},

  /* 閉じ形状から母音形状へ変化する子音 */
  {"m", LJP_m}, {"b", LJP_b}, {"p", LJP_p},

  /* /i/ の形状から母音形状へ変化する子音 */
  {"y", LJP_y}, {"j", LJP_j},

  {"sil", LIP_sil},
  {NULL, LIP_unknown}
};

/*
 * 英語の音素記号->音素 ID 変換テーブル。
 * 英語の場合は音素記号に Klattese を用いる。
 */
static ConvTable conv_list_english[] = {
  {"@",  LEN_at }, {"E",  LEN_E },   {"I",  LEN_I },  {"i", LEN_i   },
  {"A",  LEN_A },  {"R",  LEN_R },   {"c",  LEN_c },  {"a", LEN_a   },
  {"U",  LEN_U },  {"u",  LEN_u },   {"aI", LEN_aI }, {"aU", LEN_aU },
  {"EI", LEN_EI }, {"cI", LEN_cI },  {"cU", LEN_cU }, {"l", LEN_l   },
  {"y",  LEN_y },  {"w",  LEN_w },   {"r",  LEN_r },  {"|", LEN_bar },
  {"x",  LEN_x },  {"X",  LEN_X },   {"L",  LEN_L },  {"M", LEN_M   },
  {"N",  LEN_N },  {"m",  LEN_m },   {"n",  LEN_n },  {"G", LEN_G   },
  {"V",  LEN_V },  {"D",  LEN_D },   {"z",  LEN_z },  {"Z", LEN_Z   },
  {"J",  LEN_J },  {"b",  LEN_b },   {"d",  LEN_d },  {"g", LEN_g   },
  {"h",  LEN_h },  {"C",  LEN_C },   {"S",  LEN_S },  {"s", LEN_s   },
  {"T",  LEN_T },  {"f",  LEN_f },   {"p",  LEN_p },  {"t", LEN_t   },
  {"k",  LEN_k },
  {"sil", LIP_sil},
  {NULL, LIP_unknown}
};


static LangENV env_japanese = {
  conv_list_japanese,
  adjust_japanese
};

static LangENV env_english = {
  conv_list_english,
  adjust_english
};


/*
 * 日本語音韻表記において,音素記号がつながっている箇所を分解する
 */
static Seq ** adjust_japanese(Seq ** list)
{
  Seq ** seq;
  Seq  * org, *tmp;
  void * vpt;
  int i, j, k, c;
  int len;
  int c_len, v_len;
  double c_step, xc_step;
  double seg_len;
  double t;
  char *label;
  int pos;

  seq = list;
  
  len = 0;
  for(i = 0; seq[i] != NULL; i++) len++;


  /* 現在の音素列の長さを測る */
  i = 0;
  while(seq[i] != NULL)
    {
      if((seq[i + 1] == NULL) || !strcmp(seq[i]->phone, "sil"))
	{
	  i++;
	  continue;
	}

      /* 音素セグメントの長さを得る */
      seg_len = seq[i + 1]->t - seq[i]->t;
      label = seq[i]->phone;
      if(strlen(label) < 2)
	{
	  i++;
	  continue;
	}
      c_len = v_len = 0;
      for(j = 0; label[j]; j++)
	{
	  switch(label[j])
	    {
	    case 'a':
	    case 'i':
	    case 'u':
	    case 'e':
	    case 'o':
	    case 'N':
	      v_len++;
	      break;
	    case 'w':  /* 半母音は、後続の母音と一緒にカウントする */
	    case 'y':
	      break;
	    case 't':
	      if(label[j + 1] != 's') c_len++;
	      break;
	    default:
	      c_len++;
	      break;
	    }
	}

      xc_step = seg_len / ((v_len * 2) + c_len);
      c_step = xc_step;
      vpt = realloc(seq, (len + v_len + c_len) * sizeof(Seq *));
      if(NULL == vpt) goto err_0;
      seq = vpt;
      org = seq[i];

      /* 挿入個数分ずらす */
      for(j = len; j > i; j--) seq[j + c_len + v_len - 1] = seq[j];

      /* ずらした結果, 空いた箇所には NULL を詰めておく */
      for(j = 0; j < c_len + v_len; j++) seq[i + j] = NULL;
      len += c_len + v_len - 1;

      /* 分割, 挿入する */
      k = 0; pos = 0;
      t = (double)org->t;
      while(label[k])
	{
	  if(NULL == (tmp = malloc(sizeof(Seq)))) goto err_1;
	  tmp->power = org->power;
	  tmp->t = (int)t;
	  j = 0;
	  do {
	    c = label[k + j];
	    j++;
	  } while(c == 'w' || c == 'y' || (c == 't' && label[k + j] == 's'));
	  if(NULL == (tmp->phone = malloc(j + 1))) goto err_2;
	  strncpy(tmp->phone, label + k, j);
	  tmp->phone[j] = 0;
	  k += j;

	  switch(c)
	    {
	    case 'a':
	    case 'i':
	    case 'u':
	    case 'e':
	    case 'o':
	    case 'N':
	      t += c_step * 2;
	      break;
	    default:
	      t += c_step;
	      break;
	    }
	  seq[i + pos] = tmp;
	  pos++;
	}
      i += pos;
    }
  return seq;

 err_2:
  free(tmp);
 err_1:
  free(org->phone);
  free(org);
 err_0:
  for(i = 0; i < len; i++)
    {
      if(seq[i] != NULL) free(seq[i]->phone);
      free(seq[i]);
    }
  free(seq);
  return NULL;
}

/*
 * 英語の結合状態音素を分解する
 */
static Seq ** adjust_english(Seq ** list)
{
  Seq ** seq;
  Seq *step, *old, *tmp;
  void * vpt;
  int list_len;
  int i, j, k;
  char * p;
  int nums;
  int len, l, phone_len;
  int phone;
  double seg_len, consonant_len, vowel_len, t;
  char * phone_label;

  /* リストの長さを得る */
  list_len = 0;
  for(i = 0; list[i] != NULL; i++) list_len++;
  list_len++;   /* 終端の NULL も長さに含める */

  seq = list;
  for(i = 0; seq[i] != NULL; i++)
    {
      step = seq[i];
      seg_len = (seq[i + 1] != NULL) ? (seq[i + 1]->t - seq[i]->t) : 0;
      
      /* そのセグメントに含まれる音素の数を数える */
      p = step->phone;
      if(!strcmp(step->phone, "sil")) continue;  /* 無音は対象としない */
      fprintf(stderr, "-%s-\n", step->phone);
      nums = 0;
      phone_len = 0;
      while(*p)
	{
	  len = 0;
	  /* 現在の点から、もっとも長い表記の音素と思われるものを抽出する */
	  for(j = 0; conv_list_english[j].label != NULL; j++)
	    {
	      l = strlen(conv_list_english[j].label);
	      if(strlen(p) < l) continue;
	      if(l >= len)
		if(!strncmp(conv_list_english[j].label, p, l))
		  {
		    len = l;
		    phone = conv_list_english[j].phone_id;
		  }
	    }
	  if(len > 0)
	    {
	      nums++;
	      /* 音素長のカウント。子音は 1, 母音は 2 の長さをもつとして
		 計算する。以下のものを母音として定義する。 */
	      phone_len++;
	      for(j = 0; english_vowel[j] >= 0; j++)
		if(english_vowel[j] == phone)
		  {
		    phone_len++;
		    break;
		  }
	    }
		 
	  p += len;
	}
      if(nums < 2) continue;   /* 音素が一つだけであれば問題無い */
      /*
       * セグメント内に、複数の音素が含まれている場合,
       * そのセグメントを音素ごとに分割する
       */
      consonant_len = seg_len / phone_len;
      vowel_len = consonant_len * 2;  /* 母音の長さは子音の2倍として計算する */

      /*
       * 分割前の音素があった箇所に, 分割後の音素セグメント構造体を挿入する
       */
      vpt = realloc(seq, sizeof(Seq *) * (list_len + nums - 1));
      if(NULL == vpt) goto err0;
      seq = vpt;

      /* 挿入箇所を空ける */
      for(j = list_len - 1; j > i; j--) seq[j + nums - 1] = seq[j];
      
      /* 分解した音素セグメントを挿入する */
      t = step->t;
      p = step->phone;
      for(j = 0; j < nums; j++)
	{
	  if(NULL == (tmp = malloc(sizeof(Seq)))) goto err1;
	  tmp->t = (int)t;
	  tmp->power = step->power;
	  
	  for(k = 0; conv_list_english[k].label != NULL; k++)
	    {
	      l = strlen(conv_list_english[k].label);
	      if(strlen(p) < l) continue;
	      if(l >= len)
		if(!strncmp(conv_list_english[k].label, p, l))
		  {
		    len = l;
		    phone_label = conv_list_english[k].label;
		    phone       = conv_list_english[k].phone_id;
		  }
	    }
	  if(NULL == (tmp->phone = malloc(len + 1))) goto err2;
	  strcpy(tmp->phone, phone_label);
	  p += len;
	  fprintf(stderr, "[%s]\n", tmp->phone);
	  seq[i + j] = tmp;
	  t += consonant_len;
	  for(k = 0; english_vowel[k] >= 0; k++)
	    if(english_vowel[k] == phone)
	      {
		t += consonant_len;
		break;
	      }
	}
      free(step->phone);
      free(step);
      list_len += nums - 1;
    }
  return seq;
 err2:
 err1:
 err0:
  exit(EXIT_FAILURE);
}

LangENV * envGetLanguageEnvironment(LANG lang)
{
  LangENV * env;

  switch(lang)
    {
    case JAPANESE:  env = &env_japanese; break;
    case ENGLISH:   env = &env_english;  break;
#if 0
    case FRENCH:
    case ITALIAN:
    case GERMANY:
#endif
    }
  return env;
}
