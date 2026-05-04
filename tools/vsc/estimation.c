/*
 * estimation.c 
 *
 * 対数スペクトルの不偏推定法を使用して,
 * 波形からメル対数スペクトル包絡を算出する。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _estimation_c_
#include "estimation.h"
#include "pcmdata.h"
#include "frame.h"

/* 音声信号の変形ピリオドグラム I(Ω)を求める */
static double * est_I(double * wav, int fnums, int fftN)
{
  double * real, *imag;
  double a, b, omega, sum;
  int i;

  if(NULL == (real = malloc(sizeof(double) * fftN))) return NULL;
  if(NULL == (imag = malloc(sizeof(double) * fftN))) goto err_0;

  a = 0;
  for(i = 0; i < fftN; i++)
    {
      real[i] = wav[i], imag[i] = 0;
      a += real[i];
    }

  /*
   * ハミング窓など、時間範囲外で値が 0 になるような窓関数
   * $\omega_N[n]$ をかけ、フーリエ変換する。
   */
  a = 2 * M_PI / fnums;
  b = 4 * M_PI / fnums;
  for(i = 0; i < fnums; i++)
    real[i] *= 0.42 - 0.5 * cos(a * i) + 0.08 * cos(b * i);

  fftPureFFT(real, imag, fftN);

  /* 結果の絶対値の 2乗を求める */
  for(i = 0; i < fftN; i++)
    {
      real[i] = real[i] * real[i] + imag[i] * imag[i];
      imag[i] = 0;
    }

  /* 
   * \[ \left( \sum_{n=0}^{N-1} \omega_N^2[n] \right)^{-1} \]
   *
   * の部分の値を求める。
   */
  sum = 0;
  for(i = 0; i < fftN; i++)
    {
      omega = 0.42 - 0.5 * cos(i * a) + 0.08 * cos(i * b);
      sum += omega * omega;
    }
  omega = 1 / sum;

  /* omega = ブラックマン窓の自乗合計分の1 */

  /* 変形ピリオドグラムを求める */
  for(i = 0; i < fftN; i++)
    {
      real[i] = real[i] * omega;
      if(real[i] == 0) fprintf(stderr, "real[%d] == 0\n", i);
    }

  free(imag);

  return real;   /* 求めた変形ピリオドグラムを返す */

 err_0:
  free(real);
  return NULL;
}

static double est_Walpha(int mel_omega)
{
  double Wa;
  double a2;

  a2 = MEL_ALPHA * MEL_ALPHA;

  Wa = (1 - a2) / (1 + 2 * MEL_ALPHA * cos(mel_omega) + a2);
  return Wa;
}

/* 直線周波数目盛の配列をメル化する(スペクトル包絡,ピリオドグラムなど) */
static double * est_Mel(double *dst, double *src, int fftN)
{
  int i;
  double p;
  double v1, v2;
  double omega;
  double par;

  par = 2 * M_PI / fftN;

  for(i = 0; i <= fftN / 2; i++)
    {
      omega = i * 2 * M_PI / fftN;
      p = omega - 2 * atan((MEL_ALPHA * sin(omega))
			   / (1 + MEL_ALPHA * cos(omega)));
      p = p / par;  /* 直線周波数目盛上の位置 */
      
      v1 = src[(int)p];
      v2 = src[(int)p + 1] - v1;
      dst[i] = v1 + v2 * (p - (int)p);

      v1 = src[fftN - (int)p - 1];
      v2 = src[fftN - (int)p - 2] - v1;
      dst[fftN - i - 1] = v1 + v2 * (p - (int)p);
    }

  return dst;
}


/*
 * 対数スペクトルの不偏推定法を使用して、メル対数スペクトル包絡を求める
 *
 * 注) 現時点においては、不変推定法は機能していない。
 */
double *  estEstimation(double *wav, int fnums)
{
  double *real, *imag;
  double *ln_f, *I, *delC, *delG;
  double *xi;
  double  R, J, preJ;
  int fftN, n;
  int i, j, k;
  double a1, a2;


  n = 1, fftN = 2;
  while(fftN < fnums) fftN *= 2, n++;

  if(NULL == (real = malloc(sizeof(double) * fftN))) goto err_0;
  if(NULL == (imag = malloc(sizeof(double) * fftN))) goto err_1;
  if(NULL == (ln_f = malloc(sizeof(double) * fftN))) goto err_3;
  if(NULL == (delC = malloc(sizeof(double) * fftN))) goto err_4;
  if(NULL == (delG = malloc(sizeof(double) * fftN))) goto err_5;


  for(i = 0; i < fnums; i++) real[i] = (double)wav[i], imag[i] = 0;
  for(i = fnums; i < fftN; i++) real[i] = imag[i] = 0;

  /* 変形ピリオドグラムを求める */
  I = est_I(real, fnums, fftN);

  /* Hamming */
  a1 = 2 * M_PI / (double)fnums;
  a2 = 4 * M_PI / (double)fnums;
  for(i = 0; i < fnums; i++)
    /* real[i] *= 0.42 - 0.5 * cos((double)i * a1) + 0.08 * cos((double)i * a2); */
    real[i] *= 0.54 - 0.46 * cos((double)i * a1);

  /* FFT 処理 */
  fftPureFFT(real, imag, fftN);

  /* 対数スペクトルを求める */
  for(i = 0; i < fftN; i++)
    {
      real[i] = log(real[i] * real[i] + imag[i] * imag[i]);
      imag[i] = 0;
    }

  fftPureFFT(real, imag, -fftN);

  for(i = 80; i <= fftN / 2; i++)
    {
      real[i] = real[fftN - i - 1] = 0;
      imag[i] = imag[fftN - i - 1] = 0;
    }

  /* スペクトル包絡 */
  fftPureFFT(real, imag, fftN);


  /*
   * ここで求められた包絡を, $G(\Omega)$ の初期値として取り扱う。
   * (つまり、「適当な方法によって求めた $\ln f(\Omega)$ の推定値」)
   */
  for(i = 0; i < fftN; i++) ln_f[i] = real[i];

  /*
   * 不偏推定法に必要なパラメータは以下のもの。
   *
   * スペクトル密度関数 f(Ω) 
   * 変形ピリオドグラム I(Ω)
   * 対数スペクトル ln f(Ω)
   * 対数ピリオドグラム ln I(Ω)$
   * 適当な方法によって求めた ln f(Ω) の推定値 G(Ω)
   *
   * 上記から、f(Ω) の推定値は exp G(Ω) となる。
   *
   * これらの値は,ここまで計算した値のうち,以下に該当する。
   *
   * G(Ω):  ln_f[] に該当
   * I(Ω):  I[] に該当
   *
   */
#if 1
  /* $G(\Omega)$ の値を収束させる */
  preJ = 100;
  while(1)
    {
      /* 残差評価関数を用いて,残差が充分小さくなっているかを求める */
      j = 0;
      for(i = 0; i < fftN; i++)
	{
	  if(I[i] == 0)
	    {
	      fprintf(stderr, "I(Ω) == 0!!!!!!!!!!\n");
	      exit(EXIT_FAILURE);
	    }
	  R = log(I[i]) - ln_f[i];
	  real[i] = exp(R) - 1;
	  imag[i] = 0;
	  J += exp(R) - R - 1;
	}
      J = J / fftN;   /* J{G(Ω)} の値 */
      if(preJ - J <= 0.001) break;
      preJ = J;

      /* J の値が充分小さくなったら、ループを抜ける */
      if(J <= 0.001) break;

      /*
       * 推定残差 ΔG(Ω) を求める
       */
      /* ΔC[m] を求める */
      fftPureFFT(real, imag, -fftN);

      for(i = 80; i <= fftN / 2; i++)
	{
	  real[i] = real[fftN - i - 1] = 0;
	  imag[i] = imag[fftN - i - 1] = 0;
	}

      fftPureFFT(real, imag, fftN);

      /* real[] の内容がΔG(Ω) になっている(はず) */
      
      /* G(Ω) + ΔG(Ω) を、あらたな推定値とする。*/
      for(i = 0; i < fftN; i++) ln_f[i] += real[i];
    }
#endif

  /* この段階で,直線周波数メモリに対する
     対数スペクトル包絡の推定値が ln_f[i] に得られている。 */
  
  /* ここでの real, imag の用途は,あくまでテンポラリ */
  for(i = 0; i < fftN; i++)
    {
      real[i] = ln_f[i];
      imag[i] = I[i];
    }

  est_Mel(ln_f, real, fftN);  /* 対数化スペクトル包絡をメル化する */
  est_Mel(I, imag, fftN);     /* メル化ピリオドグラムの値を作成する */

  
#if 1
  /* $G(\tilde{\Omega})$ の値を収束させる */
  preJ = 100;
  while(1)
    {
      /* 残差評価関数を用いて,残差が充分小さくなっているかを求める */
      j = 0;
      for(i = 0; i < fftN; i++)
	{
	  R = log(I[i]) - ln_f[i];
	  real[i] = exp(R) - 1;
	  imag[i] = 0;
	  J += est_Walpha(i) * (exp(R) - R - 1);
	}
      J = J / fftN;   /* J{G(Ω)} の値 */
      if(preJ - J <= 0.001) break;
      preJ = J;

      /* J の値が充分小さくなったら、ループを抜ける */
      if(J <= 0.001) break;

      /*
       * 推定残差 ΔG(Ω) を求める
       */
      /* ΔC[m] を求める */
      fftPureFFT(real, imag, -fftN);

      for(i = 80; i <= fftN / 2; i++)
	{
	  real[i] = real[fftN - i - 1] = 0;
	  imag[i] = imag[fftN - i - 1] = 0;
	}

      fftPureFFT(real, imag, fftN);

      /* real[] の内容がΔG(Ω) になっている(はず) */
      
      /* G(Ω) + ΔG(Ω) を、あらたな推定値とする。*/
      for(i = 0; i < fftN; i++) ln_f[i] += real[i];
    }
#endif
  free(I);
  free(delG);
  free(delC);
  free(imag);
  free(real);

  return ln_f;

 err_5:
  free(delC);
 err_4:
  free(ln_f);
 err_3:
  free(imag);
 err_1:
  free(real);
 err_0:
  return NULL;
}
