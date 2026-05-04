#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define _analyze_c_
#include "main.h"
#include "analyze.h"
#include "spectrum.h"
#include "window.h"
#include "pcmdata.h"
#include "env.h"
#include "estimation.h"
#include "debug.h"

#define L  4
#define MEL_ALPHA 0.47
#define SGN(n)  (((n) < 0) ? -1 : 1)


/*
 * セグメンテーションパラメータ構造体の作成
 */
anaParam * anaParamNew(int i)
{
  anaParam * param;

  if(NULL == (param = malloc(sizeof(anaParam)))) return NULL;

  param->i  = i;
  param->Ci = 0.0;
  param->Vi = 0.0;
  param->Zi = 0.0;
  param->Ai = 0.0;
  param->Fi = 0.0;
  param->Ei = 0.0;
  param->Si = 0.0;

  param->status = 0;  /* 作成時の段階で、ステータスを初期化 */
  param->group  = V_unknown;
  param->label  = NULL;

  param->V      = -1;
  return param;
}


/*
 * セグメンテーションパラメータの計算
 *
 * Prototype: void anaCalcParam(anaParam * param, double *dwav, int N)
 *
 * Args:      param    値を作成するセグメンテーションパラメータ構造体
 *            dwav     実数化した波形を格納した配列のポインタ
 *            N        波形データの要素数
 *
 * Return:    void     (処理結果は, param で渡された構造体の内部に作成)
 */
int anaCalcParam(anaParam * param, short *wav, int N)
{ 
  double * dwav;
  double * real, * imag;  /* FFT 結果受取り用のバッファポインタ     */
  int      ex, fftN;      /* FFT 結果の際に用いたウィンドウのサイズ */

  double *G;
  double g[ MEL_CEPSTRUM_M ];
  double a, b;
  double omega, omega2;
  double o_cos, o_sin;
  int i, j, k;
  int m;
  double z;
  double max;

  /*
   * 波形を、FFT などが可能なサイズの double 型配列に格納する
   */
  ex = 1, fftN = 2;
  while(fftN < N) fftN *= 2, ex++;
  if(NULL == (dwav = malloc(sizeof(double) * fftN))) return -1;

#if 0
  max = 0;
  for(i = 0; i < N; i++)
    if((double)abs(wav[i]) > max) max = (double)abs(wav[i]);
#endif

  param->w_max = param->w_min = 0;
  for(i = 0; i < N; i++)
    {
      dwav[i] = (double)wav[i] * DEF_PREEMPH;
      if(i < FRAME_STEP)
	{
	  if(dwav[i] < param->w_min) param->w_min = dwav[i];
	  if(dwav[i] > param->w_max) param->w_max = dwav[i];
	}
    }

  for(i = N; i < fftN; i++) dwav[i] = 0.0;

  /*
   * 純粋に波形データを処理した場合,問題が発生することがある。
   * たとえば、標本化値全てが 0 であるような、人為的データとして
   * 生み出された完全な無音は自然界に存在せず、これをフーリエ変換後
   * 対数化すると結果が負の無限大になり、計算に不都合を来たす。
   * そこで、おおよその波形にほとんど影響を与えない程度の振幅をもった
   * 高周波バイアスをかけてやることにより、計算上の不都合を減らすことが
   * できる。
   */
  /* その分析フレームが全て 0 であることを確認する */
  {
    /*
      double x = 0.0;
      for(i = 0; i < fftN; i++) x += dwav[i] * dwav[i];
      if(x == 0.0)
    */
    /* 分析フレームが全て 0 であるなら、バイアスをかけ、不都合を無くす。 */
    for(i = 0; i < fftN; i++)
      dwav[i] += 0.00001 * cos(239 * (double)i * 2 * M_PI / fftN);
  }

  /* 波形そのままで検出可能なものとして、零交差数を求める */
  z = 0;
  for(i = 0; i < N; i++)
    z += 1 - SGN(dwav[i]) * SGN(dwav[i + 1]);
  param->Zi = (double)z / 2;
  
  G = estEstimation(dwav, N);

  /* 平均値を求め,零次メルケプストラム係数とする。 */
  /*
   * 零時メルケプストラム係数は、メル対数スペクトル包絡の平均値
   */
  z = 0;
  for(i = 0; i < fftN / 2; i++) z += G[i];
  param->Ci = param->gi[0] = z / (fftN / 2);

  /* 有声音検出パラメータを求める */
  /*
   * Jun. 8, 2000 
   *   話者によって、無声音のスペクトル平均が異なることがあるので注意。
   */
  {
    int vi_min, vi_max, vi_par;

    vi_min = (int)melFreq2MelFFT(Vi_UNDER, 512);
    vi_max = (int)melFreq2MelFFT(Vi_UPPER, 512);
    DBG("min = %d,  max = %d\n", vi_min, vi_max);
    vi_par = vi_max - vi_min + 1;
    z = 0;
    for(k = vi_min; k <= vi_max; k++) z += G[k];
    param->Vi = z / (float)vi_par;
  }

  /* gi[1] ～ gi[ MEL_CqL_CEPSTRUM - 1 ] は、メル対数スペクトルから
     求められるメルケプストラムである。 */

  /*
   * ケプストラムは対数スペクトルのフーリエ係数であるから、
   * メルケプストラムはメル対数スペクトルのフーリエ係数である。
   */
  if(NULL == (real = malloc(sizeof(double) * fftN)))
    {
      free(dwav);
      return -1;
    }
  if(NULL == (imag = malloc(sizeof(double) * fftN)))
    {
      free(real);
      free(dwav);
      return -1;
    }

  /* スペクトル包絡に 逆FFT をかけ,ケプストラムを求める
   * (※逆FFT = フーリエ係数を求める。
   *    ケプストラムは、スペクトルのフーリエ係数)  */
  for(i = 0; i < fftN; i++) real[i] = G[i], imag[i] = 0;
  fftPureFFT(real, imag, -fftN);

  /* 求められたメルケプストラムを格納 */
  for(m = 1; m < MEL_CEPSTRUM_M; m++)  param->gi[m] = real[m];

  /* スペクトル包絡を保管する */
  param->spectrum = G;
  
  free(imag);
  free(real);
  free(dwav);
  return 0;
}

/*
 * 時間変化パラメータ(零次メルケプストラム時間変化、零交差時間変化)を
 * 求める際の関数
 */
static double ana_blackman(double *buf, int m)
{
  int i, k;
  double window;
  double p1, p2;
  double N;

  N = 2 * m + 1;

  /* あつめたバッファの値に、ブラックマン窓をかける */
  p1 = p2 = 0.0;
  for(i = 0; i <= N; i++)
    {
      k = i - m;
      window = W_Blackman(i, N);
      p1 += k * k * window;
      p2 += buf[i] * k * window;
    }
  if(p1 == 0.0)
    {
      fprintf(stderr, "division by 0\n");
      exit(EXIT_FAILURE);
    }

  return p2 / p1;
}

/*
 * 連続性をもったパラメータを検出する。
 * ここで検出されるパラメータは、
 *    零次メルケプストラム時間変化
 *
 * である。
 */
int anaLatticeParameter(anaParam ** p_list, int nums)
{
  int i, j, k;
  double a_buf[L * 2 + 1];
  double f_buf[L * 2 + 1];
  double g_buf[L * 2 + 1];
  double s_buf[L * 2 + 1];
  double s;
  int m;
  double kw, sw, ww;
  double window, p1, p2;
  double a, b;

  for(i = 0; i < nums; i++)
    {
      /* Ci, Zi を中心に C i+k と Z i+k (|k| <= M) の値をバッファに集める */
      for(j = 0; j < L * 2 + 1; j++)
	{
	  if((i + j - L < 0) || (i + j - L >= nums))
	    {
	      a_buf[j] = f_buf[j] = 0.0;
	      continue;
	    }
	  a_buf[j] =  p_list[i + j - L]->Ci;
	  f_buf[j] =  p_list[i + j - L]->Zi;
	}
      
      p_list[i]->Fi = ana_blackman(f_buf, L);/* 零交差数時間変化             */
      p_list[i]->Ai = ana_blackman(a_buf, L);/* 零次メルケプストラム時間変化 */
      
      /* ======================
       * スペクトル包絡時間変化
       * ====================== */
      
      /* Ei を求めるために、すべての Si を求めておく。 */
      p_list[i]->Si = 0.0;
      for(m = 1; m <= 7; m++)
	{
	  ww = 0;
	  for(j = 0; j < L * 2 + 1; j++)
	    {
	      if((i + j - L < 0) || (i + j - L >= nums))
		{
		  g_buf[j] = 0.0;
		  continue;
		}
	      g_buf[j] = p_list[i + j - L]->gi[m];
	    }
	  s = ana_blackman(g_buf, L);
  	  p_list[i]->Si += s * s;
	}
      p_list[i]->Si = sqrt(p_list[i]->Si);
    }

 
  /*
   * 最後にスペクトル包絡時間変化 Ei の値を求める
   *
   * Ei は、全フレームの Si を求めた後でなければ計算できず、
   * その Si は全フレームの gi[m] を求めたあとでなければ計算できない。
   */
  for(i = 0; i < nums; i++)
    {
      for(j = 0; j < L * 2 + 1; j++)
	{
	  if((i + j - L < 0) || (i + j - L >= nums))
	    {
	      s_buf[j] = 0.0;
	      continue;
	    }
	  s_buf[j] = p_list[i + j - L]->Si;
	}

      kw = sw = ww = 0.0;
      for(j = 0; j < L * 2 + 1; j++)
	{
	  k = j - L;
	  window = W_Blackman(j, L * 2 + 1);
	  kw += window * k * k;
	  sw += window * s_buf[j];
	  ww += window;            /* w~    */
	}
      kw = kw / ww; /* k~w^2 */
      sw = sw / ww; /* s~w   */

      p_list[i]->Ei = 0;
      p1 = p2 = 0.0;
      for(j = 0; j < L * 2 + 1; j++)
	{
	  k = j - L;
	  window = W_Blackman(j, L * 2 + 1);
	  p1 = window * (k * k - kw) * (k * k - kw);
	  p2 = window * (k * k - kw) * (s_buf[j] - sw);
	}
      p_list[i]->Ei = - p2 / p1;

    }

  return 0;

 err_entry0:
  return -1;
}
