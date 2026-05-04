/*
 * 特徴ベクトルの算出
 *
 * 音韻認識用の特徴ベクトルを算出する
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _vector_c_
#include "vector.h"
#include "window.h"
#include "analyze.h"

#define VEC_ALPHA 0.41


/*
 * 音声の波形から、特徴ベクトルを抽出する
 *
 * ※不偏ケプストラム分析
 */
void vecCalcVector(anaParam * param, double *wav)
{
  int begin, end;
  int fftN, n;
  double * real, *imag;
  int i, j, k;
  double * G_dash;   /* mel scaled log spectrum */
  double * G_alpha;  /* mel log spectrum        */
  double * G;        /* log spectrum            */
  double * I;        /* logarithmic periodogram */

  /* 分析フレームに相当する開始 PCM フレームと,終了 PCM フレームの計算 */
  begin = param->i * FRAME_STEP;
  end = begin + FRAME_WINDOW;

  /* 
   * スペクトル計算
   */

  /* FFT 用バッファの確保 */
  n = 1; fftN = 2;
  while(fftN < FRAME_WINDOW) n++, fftN *= 2;
  if(NULL == (real = malloc(sizeof(double) * fftN))) goto err_0;
  if(NULL == (imag = malloc(sizeof(double) * fftN))) goto err_1;

  /* FFT のため、波形をバッファにコピーする */
  for(i = 0; i < FRAME_WINDOW; i++) real[i] = wav[begin + i], imag[i] = 0.0;
  for(i = FRAME_WINDOW; i < fftN; i++) real[i] = imag[i] = 0.0;

  /* 波形にハミング窓をかける */
  wndHamming(real, FRAME_WINDOW);
  /* wndHanning(real, FRAME_WINDOW); */
  /* wndBlackman(real, FRAME_WINDOW); */

  /* FFT の実行 */
  fftPureFFT(real, imag, fftN);

  /* この時点で, real[], imag[] の内容は フーリエ変換結果になっている */

  /* 対数振幅スペクトルの計算 */
  for(i = 0; i < fftN; i++)
    {
      real[i] = log(real[i] * real[i] + imag[i] * imag[i]);
      imag[i] = 0;
    }

  /*
   * 不偏ケプストラム分析法により、スペクトル包絡を求める
   */

  /* 対数ピリオドグラムの算出 */
  if(NULL == (I = vec_logarithmic_periodogram(real, fftN, VEC_ALPHA)))
    goto err_2;

  
  


  



    

  

  
  
  
  
  

  
  
 err_2:
  free(imag);
  
 err_1:
  free(real);

 err_0:
  return -1;
}

static double * vec_logarithmic_periodogram(double *f, int fftN, double alpha)
{
  int i;
  double *I;
  double omega, tilde_omega;
  
  /* 対数ピリオドグラムを求める */
  /*
    定常過程 $x[n]$ の変形ピリオドグラム $I(\Omega)$ は,
    以下のように表現される。
    
    $I(\Omega) = (1 + \xi(\Omega))f(\Omega)$
    
    ここで
    $E\{\xi(\Omega)\} = 0$
    
    であるが、 $x[n]$ が正規定常過程に対しては
    
    $E\{\xi^2(\Omega)\} \simeq 1$
    
    となる。
  */
  
  if(NULL == (I = malloc(sizeof(double) * fftN))) return NULL;
  for(i = 0; i < fftN; i++)
    {
      tilde_omega = 2 * M_PI * i / fftN;
      
      omega = tilde_omega -
	2 * atan(alpha * sin(tilde_omega) / (1 + alpha * cos(tilde_omega)));
      
      I[i] = (1 + _xi(omega)) * f[i];
    }
  return I;
}

/*
 * 確率関数 ξ(Ω)
 *
 * 確率関数 $\xi(\Omega)$ は、
 *
 * $E\{\xi(\Omega)\} = 0$ であり、
 * $E\{\xi^2(\Omega)} \simeq 1$ の条件を満たすものである
 */
static double _xi(double omega)
{
  double xi;

  /* 確率関数の内容は調査中 */


  return xi;
}


/* メル化対数スペクトル推定値を求める */
static double * vec_mel_scaled_log_spectrum(double * real, int fftN,
					    double alpha)
{
  double *G_dash;
  double tilde_omega;
  double omega;
  
  int i, b;

  if(NULL == (G_dash = malloc(sizeof(double) * fftN))) return NULL;
  
  for(i = 0; i < fftN; i++)
    {
      tilde_omega = 2 * M_PI * i / fftN;
      omega = tilde_omega -
	2 * atan(alpha * sin(tilde_omega) / (1 + alpha * cos(tilde_omega)));

      b = (int)omega;

      G_dash[i] = real[b] + (real[b + 1] - real[b]) * (omega - (double)b);
    }
  return G_dash;
}


/*
 * 残差評価関数(メル対数スペクトル用)
 *
 * I[]   対数ピリオドグラム
 * G[]   メル対数スペクトルの値
 * alpha alpha の値 (通常 0.41 を使用)
 *
 * ※ I[] は,必ず正の値で、0 は入らない(入れると ln(I[Ω]) が∞になる)。
 */
static double func_J(double *I, double *G, double alpha, int fftN)
{
  int i;
  double R;
  double Jsum;

  Jsum = 0;
  for(i = 0; i < fftN; i++)
    {
      R = ln(I[i]) - G[i];
      Jsum += exp(R) - R - 1;
    }
  return Jsum / fftN;
}

/*
 * 残差評価関数(メル化対数スペクトル用)
 *
 * I[]   対数ピリオドグラム
 * G[]   メル化対数スペクトルの値
 * alpha alpha の値 (通常 0.41 を使用)
 */
static double func_J_dash(double *I, double *G, double alpha, int fftN)
{
  int i;
  double R;
  double omega;
  double Walpha;
  double Jsum;

  Jsum = 0;
  for(i = 0; i < fftN; i++)
    {
      omega = 2 * M_PI * i / fftN;
      R = ln(I[i]) - G[i];
      Walpha = (1 - alpha * alpha) /
	(1 + 2 * alpha * cos(omega) + alpha * alpha);
      Jsum += Walpha * exp(R) - R - 1;
    }
  return Jsum / fftN;
}
