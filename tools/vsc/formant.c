#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _formant_c_
#include "formant.h"
#include "pcmdata.h"
#include "mel.h"
#include "debug.h"

#define FORMANT_CHECK_MORE

#define BEST_MAX  5
#define BORDER 0.5

#define ABS(n)   (((n) < 0) ? -(n) : (n))

typedef struct maxpos {
  int    pos;  /* 極大点のある位置 */
  double lev;  /* 極大点の振幅     */
  double freq; /* 周波数成分[Hz] に換算した極大点の位置 */
} maxPos;

/* 2点を周波数成分の高低で比較する */
static int cmp_maxpos(const void *a, const void *b)
{
  maxPos *ia, *ib;

  ia = (maxPos *)a;  ib = (maxPos *)b;
  if(ia->pos < ib->pos) return -1;
  if(ia->pos > ib->pos) return 1;
  return 0;
}

/*
 * スペクトル包絡波形から、フォルマントを指定個数抽出する
 */
int fmtGetFormant(double * retarray, int arraymax, double *spectrum, int fftN)
{
  int nums, max, min;
  int i, j, k;
  maxPos F[ BEST_MAX ];
  double top, bottom;
  double d_pos;
  double amp_min, amp_max;
  double f;
  int cont, cnt;

  /* フォルマントを求める下限となる周波数から、下限となる要素を求める */
  /* 音韻識別に使用するのは約 20Hz ぐらいからだが、
     スペクトル包絡がメル周波数目盛に基づいているため、それに合わせて
     上限となる要素も変化する。 */
  min = (int)melFreq2MelFFT(250, fftN);  /* 可聴周波以下の音は対象としない */
  /* フォルマントを求める上限となる周波数から、上限となる要素を求める */
  /* 音韻識別に使用するのは約 3000Hz ぐらいまでだが、
     スペクトル包絡がメル周波数目盛に基づいているため、それに合わせて
     上限となる要素も変化する。 */
  max = (int)melFreq2MelFFT(3000, fftN);
  DEBUG("formant pickup by %d - %d\n", min, max);


  nums = 0;
  for(i = 0; i < BEST_MAX; i++)
    {
      F[i].pos = -1;
      F[i].lev = 0;
      F[i].freq = 0;
    }
  for(i = 0; i < arraymax; i++) retarray[i] = 0;

  DEBUG("--point 1\n");
#ifdef FORMANT_CHECK_MORE
  /*
   * 振幅の最大値、最小値を求める
   */
  amp_min = 10000, amp_max = 0;
  for(i = min; i < max; i++)
    {
      if(spectrum[i] > amp_max) amp_max = spectrum[i];
      if(spectrum[i] < amp_min) amp_min = spectrum[i];
    }
#endif

  for(i = min; i < max ; i++)
    {
      /* 左の点が、今自分がいる箇所以上であれば,
	 極大点ではないのでスキップ */
      if(i > min && spectrum[i] <= spectrum[i - 1]) continue;

      /* おなじ値が続く限り、カウントする */
      cnt = 0;
      for(j = i; j < max && spectrum[j] == spectrum[i]; j++) cnt++;

      /* カウントが終った箇所が、検査点の値より大きければ,
	 極大点はそこにはないためスキップ */
      if(spectrum[j] > spectrum[i]) continue;

      /* 検査点の位置に、おなじ値が連続している長さの
	 半分を加えた位置を極大点とする */
      i += cnt / 2;

      /* 極大点の振幅の順位が,上位 BEST_MAX 位までに
	 入っているかどうかを調べる */
      for(j = 0; j < BEST_MAX; j++)
	if(F[j].lev < spectrum[i])
	  {
	    /* 新たな要素として挿入する */
	    for(k = BEST_MAX - 2; k >=j; k--)
	      {
		F[k + 1].pos = F[k].pos;
		F[k + 1].lev = F[k].lev;
		F[k + 1].freq = F[k].freq;
	      }
	    F[j].pos = i;
	    F[j].lev = spectrum[i];
	    f = melFFT2Freq((double)i, fftN);
	    F[j].freq = f;
	    DEBUG("F[%d]: %d = %8.3f  %8.3f\n", j, i, f, F[j].freq);
	    if(nums < BEST_MAX) nums++;
	    break;
	  }
    }

  DEBUG("--point 2\n");
  
  /* 抽出したフォルマントを,周波数が低位のものから並べかえる */
  qsort(F, nums, sizeof(maxPos), cmp_maxpos);

  DEBUG("--point 3\n");

#if 0
  /*
    隣り合った極大点同士の
       (1)周波数成分の間隔が狭く、
       (2)振幅差が小さく,
       (3)間にはさまれた極小点との振幅差も小さい場合,
     それらは一つの極大点とみなす。
  */
  for(i = 0; i < nums - 1; i++)
    {
      j = i + 1;
      do
	{
	  cont = 0;
	  if(F[j].freq - F[i].freq <= ENVELOP_FREQ)
	    if(ABS(F[j].lev - F[i].lev) <= ENVELOP_LEV)
	      {
		/* 二つの極大点が十分近く,振幅差もあまりない場合,
		   二つの極大点の振幅平均と,間にはさまれた極小点との
		   差を求める */
		top = (F[j].lev + F[i].lev) / 2;
		bottom = F[i].lev;
		for(k = F[i].pos; k < F[j].pos; k++)
		  if(bottom > spectrum[k])
		    bottom = spectrum[k];  /* 間の極小点 */
		
		
		/* 求めた極小点との差が十分小さいなら,
		   その二つの極大点を,その中間にある一つの極大点とみなす。 */
		if(top - bottom < ENVELOP_SUBS)
		  {
		    /*
		     * 極大点を一つにまとめる
		     */

		    /* まとめた場合の周波数成分を計算する。
		     * 二つの極大点の中間より, やや高い方に寄った形になる。*/
		    d_pos = (double)(F[j].pos - F[i].pos) /2;
		    d_pos *= F[j].lev / F[i].lev;
		    F[i].pos = (int)(d_pos + 0.5);
		    
		    /* まとめた場合の振幅を計算する。
		     * 振幅は,もともとの二つの極大点の平均と考える。*/
		    F[i].lev = top;
		    
		    /* 周波数[Hz] に変換する */
		    F[i].freq = melFFT2Freq(F[i].pos, fftN);

		    /* 極大点リストを一つ詰める */
		    nums--;
		    for(k = j; k < nums; k++)
		      {
			F[k].pos = F[k + 1].pos;
			F[k].lev = F[k + 1].lev;
			F[k].freq = F[k + 1].freq;
		      }
		    
		    cont = 1;		    
		  }
	      }
	}
      while(cont);
      }
#endif
  DEBUG("--point 4: nums = %d\n", nums);

  for(i = 0; i < nums; i++)
    {
      DEBUG("freq: %8.3f\n", F[i].freq);
      retarray[i] = F[i].freq;
    }

  for(i = nums; i < BEST_MAX; i++) retarray[i] = 0;

  return 0;
}
