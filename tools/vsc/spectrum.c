#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _spectrum_c_
#include "spectrum.h"
#include "env.h"
#include "fft.h"

#define M  DEF_FBANK     /* フィルタ C[m] の次数 */
#define SPEC_CEPW 80

/*
 * メル対数スペクトルを計算する
 *
 * Prototype: double * specMelLogSpectrum(double alpha, int fftN, double *dwav,
 *                                        double *real, double *imag)
 *
 * Args:      alpha    mel 近似のためのアルファ値
 *            fftN     FFT の際の、角速度分解能
 *            dwav     実数表現の波形
 *            real     FFT 結果の実数部
 *            imag     FFT 結果の虚数部
 *
 * Return:    double * 計算結果の G (メル対数スペクトル)
 *
 * ※ メル周波数におけるフーリエ変換結果の求め方に直線補間を使うという
 *    かなりいい加減なことをやっているので、そこは後でもっと精度の高い
 *    近似方法に置き換える。
 */
specSpectrum * specMelLogSpectrum(double alpha, int fftN, double *dwav,
				  double *real, double *imag)
{
  specSpectrum * G;
  double omega, omega2;
  double o_step;
  double k2, k3;
  double re, im;
  int k;
  int n;

  if(NULL == (G = malloc(sizeof(specSpectrum) * fftN))) return NULL;

  o_step = 2 * M_PI / fftN;
  for(k = 0; k < fftN; k++)
    {
      omega = o_step * (double)k;
      omega2 = omega - 2 * atan(alpha * sin(omega) / (1 + alpha * cos(omega)));
      k2 = omega2 / o_step;
      k3 = k2 - (double)(int)k2;

      if((k2 < 0) || (k3 < 0))
	{
	  fprintf(stderr, "GGGGGGGGGGGGGG\n");
	  exit(EXIT_FAILURE);
	}
      /* メルスケール上の値を線形補間で求める */
      re = real[(int)k2] + (real[(int)k2 + 1] - real[(int)k2]) * k3;
      im = imag[(int)k2] + (imag[(int)k2 + 1] - imag[(int)k2]) * k3;

      G[k].G = log(re * re + im * im);
    }  

  /*
   * スペクトル包絡を求める
   */
  /* スペクトルの計算 */
  for(k = 0; k < fftN; k++)
    {
      real[k] = 10.0 * G[k].G;
      imag[k] = 0;
    }

  /* 逆 FFT をかけて、ケプストラムを計算する */
  fftPureFFT(real, imag, -fftN);

  /* スペクトル包絡計算 */
  for(k = SPEC_CEPW; k <= fftN / 2; k++)
    real[k] = real[fftN - k] = 0;  /* ピッチ成分除去 */

  fftPureFFT(real, imag, fftN);

  /* 計算されたスペクトル包絡を確保したバッファにコピー */
  for(k = 0; k < fftN; k++)
    G[k].envG = real[k];
  
  return G;
}

