#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _scedule_c_
#include "scedule.h"
#include "util.h"


/*
 * 時系列表モジュール初期化
 */
void scInit(void)
{
  
}

/*
 * テキスト形式の音素時系列表を、すでに開かれているストリームから読み込む。
 * 例) scReadSceduleByStream(stdin);
 */
scScedule ** scReadSceduleByStream(FILE *rfp)
{
  char *line;
  char *time_s, *param, *pow;
  scScedule * sc;
  scScedule **sc_list;
  void *vpt;
  int nums;
  int i, j;
  int finish_time;

  nums = 0;
  sc_list = NULL;
  while(NULL != (line = utilReadLine(rfp)))
    {
      /*
       * 読み込んだ行を、時系列、音素記号、その他情報に分割する。
       */

      /* [1] 時系列とその他の情報は、':' で区切られる */
      for(i = 0; line[i]; i++)
	if(line[i] == ':') break;
      if(!line[i]) goto err_1;

      time_s = malloc(i + 1);
      param = malloc(strlen(line) - i);
      if(time_s == NULL || param == NULL) goto err_2;

      /* 分割した両辺を、別の文字列として作成 */
      strncpy(time_s, line, i); time_s[i] = 0;
      strcpy(param, line + i + 1);

      /* 文字列の左右に入る空白を除去 */
      utilStrFips(time_s);
      utilStrFips(param);

      /* 音素記号とパワーは空白またはタブで区切られる */
      pow = utilDupStr(param);  /* 二重化 */

      /* 間に入る空白またはタブを探す */
      for(i = 0; pow[i] != 0; i++)
	if(pow[i] == ' ' || pow[i] == '\t') break;

      /* 音素記号のほうは見つけた空白以降、
	 パワーのほうは見つけた空白までを、空白でうめる */
      for(j = i; param[j] != 0; j++) param[j] = ' ';
      for(j = 0; j < i; j++) pow[j] = ' ';

      /* 前後の空白を除去することにより、
	 音素記号のみ、パワーのみにすることができる */
      utilStrFips(param);
      utilStrFips(pow);

      /* 音素記号より右側を除去 */
      for(i = 0; param[i]; i++)
	if(param[i] == ' ' || param[i] == '\t')
	  {
	    /* 空白とみなせる文字があったら、そこを文字列の終端とする。
	       (文字列始端の空白は、直前の utilStrFips() で除去される。*/
	    param[i] = 0;
	    break;
	  }

      /* 構造体領域を作成 */
      if(NULL == (sc = malloc(sizeof(scScedule)))) goto err_2;

      /* 音素時刻の登録 */
      if(utilStr2Int(&(sc->t), time_s)) goto err_3;
      finish_time = sc->t;

      /* 音素ラベルの登録 */
      if(NULL == (sc->label = malloc(strlen(param) + 1))) goto err_3;
      strcpy(sc->label, param);

      /* パワーの登録 */
      sc->power = DEF_POWER * atof(pow);
      if(sc->power < MIN_POWER) sc->power = MIN_POWER;


      /* 完成した scScedule 構造体をリストに登録 */
      if(NULL == (vpt = utilAddBuf(sc_list, sizeof(scScedule *), nums)))
	goto err_4;

      sc_list = vpt;

      sc_list[nums] = sc;
      nums++;

      free(pow);
      free(time_s);
      free(param);
      free(line);
    }

  /* 構造体領域を作成(最後の無音領域) */
  if(NULL == (sc = malloc(sizeof(scScedule)))) goto err_2;
  sc->t = finish_time + CLOSE_SPEED;
      
  /* 音素ラベルの登録 */
  if(NULL == (sc->label = malloc(4))) goto err_3;
  strcpy(sc->label, "sil");

  /* パワーの登録 */
  sc->power = DEF_POWER;  /* 現時点では、パワーには定数が入る。 */

  if(NULL == (vpt = utilAddBuf(sc_list, sizeof(scScedule *), nums)))
    goto err_4;
  sc_list = vpt;
  sc_list[nums] = sc;

  nums++;

  /* リストの最後は、終端を表す NULL で締める */
  if(NULL == (vpt = utilAddBuf(sc_list, sizeof(scScedule *), nums)))
    goto err_4;
  sc_list = vpt;
  sc_list[nums] = NULL;

  return sc_list;

 err_4:
  free(sc->label);
 err_3:
  free(sc);
 err_2:
  if(time_s != NULL) free(time_s);
  if(param != NULL) free(param);
 err_1:
  free(line);
 err_0:
  /*
   * エラーが発生した場合、作成途中のリストは必ず開放する
   */
  if(nums)
    {
      for(i = 0; i < nums; i++)
	{
	  free(sc_list[i]->label);
	  free(sc_list[i]);
	}
      free(sc_list);
    }
  return NULL;  
}

/*
 * 音素時系列表を、指定された名前を持つファイルから読み込む
 */
scScedule ** scReadScedule(char *fname)
{
  FILE *rfp;
  scScedule **sc;

  rfp = fopen(fname, "r");   /* スケジュールデータは、テキストで与えられる */
  if(NULL == rfp) return NULL;

  sc = scReadSceduleByStream(rfp);
  fclose(rfp);
  return sc;
}

/*
 * スケジュールリストに使用しているメモリ領域を開放する
 */
void scReleaseScedule(scScedule ** sc_list)
{
  int i;

  for(i = 0; sc_list[i] != NULL; i++)
    {
      free(sc_list[i]->label);
      free(sc_list[i]);
    }
  free(sc_list);  
}

/*
 * 指定された、音素時系列スケジュールの要素数を数える。
 * 最後の NULL　は含まない。
 */
int scSceduleCount(scScedule ** sc_list)
{
  int cnt;
  cnt = 0;

  /* 余計なオプティマイズするんじゃねえぞ > コンパイラ */
  while(sc_list[cnt] != NULL) cnt++;

  return cnt;
}


/*
 * まとまった音素を分解し、スケジュールを調整する
 */
scScedule ** scAdjustScedule(scScedule **sc_list)
{
  scScedule ** tmp;
  scScedule *sc, *old;
  int i, j, k, p;
  int sc_len;
  int elem_cnt;
  char *label;
  int c;
  int v_cnt; /* vowel counter     */
  int c_cnt; /* consonant counter */
  double c_len;
  double t;
  double power;

  sc_len = scSceduleCount(sc_list);
  i = 0;
  while(i < sc_len)
    {
      if(!strcmp(sc_list[i]->label, "sil"))
	{
	  i++;
	  continue;
	}

      c_len = sc_list[i + 1]->t - sc_list[i]->t;
      if(strlen(sc_list[i]->label) > 1)
	{
	  label = sc_list[i]->label;

	  /* 音素セグメントの長さを取得 */
	  /* 母音と子音を数える */
	  elem_cnt = 0;
	  v_cnt = c_cnt = 0;
	  while(0 != (c = label[elem_cnt++]))
	    {
	      /* 半母音は後続の母音と同じ扱いとする。 */
	      if(c == 'w' || c == 'y') continue;
	      switch(c)
		{
		case 'a':
		case 'i':
		case 'u':
		case 'e':
		case 'o':
		case 'N':
		  v_cnt++;
		  break;
		default:
		  c_cnt++;
		  break;
		}
	    }
	  if((c_cnt + v_cnt) == 1) /* 長さが 1 ならば、今までと変わらない */
	    {
	      i++;
	      continue;
	    }
	  
	  /* 音素セグメント全体の長さを v * 2 + c として等分する */
	  c_len = c_len / (v_cnt + 2 + c_cnt);

	  /* 音素セグメントリストを、
	     (v + c - 1) 個の挿入が可能なように拡張する */
	  tmp = realloc(sc_list, sizeof(scScedule *)*(sc_len + v_cnt + c_cnt));
	  if(NULL == tmp) goto err;
	  sc_list = tmp;

	  old = sc_list[i];
	  t = old->t;
	  power = old->power;
	  /*
	   * 挿入のため、該当個所のデータ以降を後ろにずらす
	   */
	  for(j = sc_len; j > i; j--)
	    sc_list[j + c_cnt + v_cnt - 1] = sc_list[j];
	  sc_len += v_cnt + c_cnt - 1;
	  k = 0;
	  /* 音素ごとに分解し、新たなスケジュールとして組みなおす */
	  for(j = 0; j < v_cnt + c_cnt; j++)
	    {
	      if(NULL == (sc = malloc(sizeof(scScedule)))) goto err;
	      sc->t = (int)(t + 0.5);
	      sc->power = power;
	      elem_cnt = 0;
	      p = k;
	      do {
		elem_cnt++;
	      } while(c = old->label[p++], (c == 'w' || c == 'y'));
	      if(NULL == (sc->label = malloc(elem_cnt + 1))) goto err;
	      for(p = 0; p < elem_cnt; p++)
		c = sc->label[p] = old->label[k + p];
	      sc->label[p] = 0;
	      switch(c)
		{
		case 'a':
		case 'i':
		case 'u':
		case 'e':
		case 'o':
		case 'N':
		  t += c_len * 2;   /* 母音は子音の2倍の長さとして扱う */
		  break;
		default:
		  t += c_len;
		  break;
		}
	      k += elem_cnt;
	      sc_list[i + j] = sc;
	    }
	  /*
	   * 分解前の構造体を開放する
	   */
	  free(old->label);
	  free(old);
	  i += v_cnt + c_cnt;
	  continue;
	}
      i++;
    }

  return sc_list;

 err:
  fprintf(stderr, "not enough memory.\n");
  exit(EXIT_FAILURE);
}
