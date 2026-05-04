/*
 * 対数スペクトル包絡から、特徴ベクトルとなるケプストラムを求める
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>


#define _cepstrum_c_
#include "cepstrum.h"
#include "mel.h"

#define BASE 0

cepVector * cepCalcVector(cepVector * retcep, double * spectrum, int fftN)
{
  double *real, *imag;
  cepVector * cep;
  int alc_flg = 0;
  int i;

  if(NULL == (real = malloc(sizeof(double) * fftN))) goto err_0;
  if(NULL == (imag = malloc(sizeof(double) * fftN))) goto err_1;

  if(NULL == retcep)
    {
      if(NULL == (cep = malloc(sizeof(cepVector)))) goto err_2;
      alc_flg = 1;
    }
  else
    cep = retcep;
  
  /*
   * 与えられているスペクトル包絡から、ケプストラムを求める。
   * メル対数スペクトル包絡が与えられるはずなので、求められるケプストラムも
   * メルケプストラムとなる。
   */

  /* オリジナルのスペクトル包絡に影響を与えないように、コピーを作る。*/
  for(i = 0; i < fftN; i++) real[i] = spectrum[i], imag[i] = 0;

  /* スペクトル包絡のフーリエ係数を求めることにより、
     ケプストラムを求めることができる(つまり逆 FFT ) */
  fftPureFFT(real, imag, -fftN);

  /* 特徴ベクトルとして保存 */
  for(i = 0; i < cepVEC_DIM; i++) cep->c[i] = real[i];

  free(imag);
  free(real);

  return cep;

 err_3:
  free(cep);
 err_2:
  free(real);
 err_1:
  free(imag);
 err_0:

  return NULL;  
}


/*
 * メルケプストラム同士の距離を求める
 */

/* ハミング距離 */
double cepDistanceHamming(cepVector *a, cepVector *b)
{
  int m;
  double distance;

  distance = 0;
  for(m = BASE; m < cepVEC_DIM; m++)
    distance += fabs(a->c[m] - b->c[m]);

  return distance;  
}


/* ユークリッド距離 */
double cepDistanceEuclidian(cepVector *a, cepVector *b)
{
  int m;
  double tmp;
  double distance;

  distance = 0;
  for(m = BASE; m < cepVEC_DIM; m++)
    {
      tmp = fabs(a->c[m] - b->c[m]);
      distance += tmp * tmp;
    }
  return sqrt(distance);
}

/* チェビシェフ距離 */
double cepDistanceChebychev(cepVector *a, cepVector *b)
{
  int m;
  double distance, tmp;

  distance = 0;
  for(m = BASE; m < cepVEC_DIM; m++)
    {
      tmp = fabs(a->c[m] - b->c[m]);
      if(tmp > distance) distance = tmp;
    }

  return distance;
}

/*
 * 正定値対称行列を作成
 */
static void cep_set_symmetrical_matrix(double mat[cepVEC_DIM][cepVEC_DIM],
				       cepVector *v)
{
  /* yet */
}

/*
 * 2次形式の非線形距離
 */
double cepDistanceNonlinear2D(cepVector *a, cepVector *b)
{
  double matA[cepVEC_DIM][cepVEC_DIM];
  double matB[cepVEC_DIM][cepVEC_DIM];
  int i, j;

  /* ベクトルの値を、正定値対称行列として二次元配列に設定 */
  cep_set_symmetrical_matrix(matA, a);
  cep_set_symmetrical_matrix(matB, b);

  /* yet */
  return 0.0;
}

/* I will be 'Hacker' !! */
