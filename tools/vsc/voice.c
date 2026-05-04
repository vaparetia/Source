/*
 * 音声特性の個人差をある程度緩和するための、
 * 不特定話者モジュール
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _voice_c_
#include "voice.h"
#include "analyze.h"

/* #define _DEBUG_ */

#ifdef _VOICE_DEBUG_
#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */
#endif /* _VOICE_DEBUG_ */

#include "debug.h"

/*
 * 有声音検出パラメータの閾値を、実際に計測された Vi の値を元に
 * 自動設定する。
 */
int voiAutoAdjustLevel(anaParam ** p_list, int f_nums)
{
  int i, j, v;
  double max_value;      /* 最大の極大値 */
  double min_value;      /* 最小の極小値 */
  double max_envelop;    /* 極大点平均   */
  double min_envelop;    /* 極小点平均   */
  double vi_envelop;     /* Vi の値全体の平均 */
  int max_samples;       /* 平均を求めるために採取した極大点の数 */
  int min_samples;       /* 平均を求めるために採取した極小点の数 */
  int vi_samples;        /* 平均を求めるために採取した有音部の数 */
  unsigned char * novoice;
  int begin, end, nvcnt; /* 有音部ブロックの始点と終点フレーム   */
  double Tvl, Tvh;

  /*
   * 明らかな無音部を処理対象としないために、有音、無音のチェックを行う
   */
  if(NULL == (novoice = malloc(sizeof(unsigned char) * f_nums))) return -1;
  for(i = 0; i < f_nums; i++) novoice[i] = 0;

  vi_envelop = 0;
  vi_samples = 0;
  for(i = 0; i < f_nums; i++)
    {
      v = 0;
      for(j = -4; j <= 4; j++)
	{
	  if((i + j < 0) || (i + j >= f_nums)) continue;
	  /* 有音/無音の別は,零次メルケプストラム係数によって判別する */
	  if(p_list[i + j]->Ci >= -2.0) v++;
	}
      /* 前後 2 フレーム(計5フレーム)以内に、3フレーム以上 Ci が 2.0 以上の
	 フレームが存在しなければ、処理対象から外す。 */
      novoice[i] = (v < 3) ? 1 : 0;  /* 1: 無音  / 0: 有音 */
      if(novoice[i])
	{
	  p_list[i]->Tvl = -2.0;
	  p_list[i]->Tvh = 0.0;
	}
    }

  /*
   * 人間の言葉には語調というものがあり、一連の発言でも途中で語調の強弱が
   * 変動する。多くの場合語調は一つの段落でほぼ一定しているため、認識上の
   * 有声音検出パラメータ用閾値を,各段落(具体的には、無音部ではさまれた有音部)
   * ごとに個別に求める。
   */
  begin = 0;
  while(begin < f_nums)
    {

      if(novoice[begin])
	{
	  begin++;
	  continue;
	}


      DEBUG("begin: %d  - ", begin);
      /* 
       * 有音部ブロック最終フレームを求める
       */
      end = begin;
      nvcnt = 0;
      while(end < (f_nums - 1))
	{
	  if(novoice[end + 1]) nvcnt++;
	  else nvcnt = 0;
	  if(nvcnt >= 5) break;
	  end++;
	}
      DEBUG("end: %d\n", end);

      /*
       * この時点で,
       *
       *  begin   …有音部ブロック(段落)先頭
       *  end     …有音部ブロック(段落)終端
       *
       * となっている。
       */

      /* 有音部ブロックの,有声音検出パラメータ平均値を求める */
      vi_samples = 0;
      vi_envelop = 0;
      for(i = begin; i < end; i++)
	if(!novoice[i])
	  {
	    vi_envelop += p_list[i]->Vi;
	    vi_samples++;
	  }
      vi_envelop = vi_envelop / vi_samples;

      max_value = min_value = vi_envelop;
      max_envelop = min_envelop = 0;
      max_samples = min_samples = 0;
      DEBUG("start loop\n");
      for(i = begin; i < end;  i++)
	{
	  int pre, nxt;

	  /* 無音であると判定されていれば、処理対象にしない */
	  if(novoice[i]) continue;

	  DEBUG("+++ 1\n");
	  /* 極大点の検出 */
	  /* 有声音検出パラメータ平均以上であることを条件とする */
	  pre = nxt = 0;
	  if(i - 1 >= 0) pre = (p_list[i]->Vi > p_list[i - 1]->Vi);
	  if(i + 1 < f_nums) nxt = (p_list[i]->Vi >= p_list[i + 1]->Vi);

	  DEBUG("+++ 2\n");

	  if(pre && nxt && (p_list[i]->Vi >= vi_envelop))
	    {
	      if(p_list[i]->Vi > max_value) max_value = p_list[i]->Vi;
	      max_envelop += p_list[i]->Vi;
	      max_samples++;
	    }

	  DEBUG("+++ 3\n");

	  /* 極小点の検出 */
	  /* 有声音検出パラメータ平均以下であることを条件とする */
	  pre = nxt = 0;
	  if(i - 1 >= 0) pre = (p_list[i]->Vi < p_list[i - 1]->Vi);
	  if(i + 1 < f_nums) nxt = (p_list[i]->Vi <= p_list[i + 1]->Vi);
	  if(pre && nxt &&  (p_list[i]->Vi <= vi_envelop))
	    {
	      if(p_list[i]->Vi < min_value) min_value = p_list[i]->Vi;
	      min_envelop += p_list[i]->Vi;
	      min_samples++;
	    }

	  DEBUG("+++ 4\n");


	}

      DEBUG("loop termination.\n");

      max_envelop = max_envelop / max_samples;
      min_envelop = min_envelop / min_samples;
     
      Tvh = (max_envelop + vi_envelop) / 2;
      Tvl = (min_envelop + vi_envelop) / 2;

      DEBUG("Set Tvl/Tvh(f_nums = %d, end = %d)\n", f_nums, end);

      /* その有音部ブロックを構成する全体に, Tvl, Tvh の値を設定する。 */
      for(i = begin; i <= end; i++)
	{
	  p_list[i]->Tvl = Tvl;
	  p_list[i]->Tvh = Tvh;
	}

#ifdef _DEBUG_
      fprintf(stderr,
	      "begin: %d   end: %d\n"
	      "Vi env: %8.3f  max env: %8.3f min env: %8.3f\n"
	      "max val: %8.3f   min val: %8.3f\n",
	      begin, end,
	      vi_envelop, max_envelop, min_envelop, max_value, min_value);
      
      fprintf(stderr, "Tvl: %8.3f   Tvh: %8.3f\n", Tvl, Tvh);
#endif /* _DEBUG_ */

      /* 次回は有音部ブロック最終フレームの次から処理を始める */
      begin = end + 1;

    }

  DEBUG("*** end = %d\n ***", end);
  free(novoice);

  return 0;
}
