/* 何にせよ、FFT 無くば始まらない */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define _fft_c_
#include "fft.h"

#ifdef _FFT_DEBUG_
#ifndef _DEBUG_
#define _DEBUG_
#endif /* _DEBUG_ */
#endif /* _FFT_DEBUG_ */


static void core_sintbl(int n, double *sintbl)
{
  int i, n2, n4, n8;
  double c, s, dc, ds, t;

  n2 = n / 2;  n4 = n / 4;  n8 = n / 8;
  t = sin(M_PI / n);
  dc = 2 * t * t; ds = sqrt(dc * (2 - dc));

  t = 2 * dc; c = sintbl[n4] = 1; s = sintbl[0] = 0;
  for(i = 1; i < n8; i++)
    {
      c -= dc; dc += t * c;
      s += ds; ds -= t * s;
      sintbl[i] = s;
      sintbl[n4 - i] = c;
    }
  if(n8 != 0) sintbl[n8] = sqrt(0.5);
  for(i = 0; i < n4; i++)      sintbl[n2 - i] = sintbl[i];
  for(i = 0; i < n2 + n4; i++) sintbl[i + n2] = -sintbl[i];
}

static void core_bitrev(int n, int *bitrev)
{
  int i, j, k, n2;

  n2 = n / 2; i = j = 0;

  for(;;)
    {
      bitrev[i] = j;
      if(++i >= n) break;
      k = n2;
      while(k <= j) { j -= k; k /= 2; }
      j += k;
    }
}

static int core_fft(double *real, double *imag, int n)
{
  static int      last_n = 0;
  static int    * bitrev = NULL;
  static double * sintbl = NULL;

  int i, j, k;
  int ik, h, d, k2, n4;
  int inverse;
  double t, s, c, dx, dy;

  if(n < 0)
    {
      n = -n;
      inverse = 1;
    }
  else inverse = 0;

  n4 = n / 4;
  if(n != last_n || n == 0)
    {
      last_n = n;
      if(sintbl != NULL) free(sintbl);
      if(bitrev != NULL) free(bitrev);
      if(n == 0) return 0;

      sintbl = malloc((n + n4) * sizeof(double));
      bitrev = malloc(n * sizeof(int));
      if(sintbl == NULL || bitrev == NULL) return -1;

      core_sintbl(n, sintbl);
      core_bitrev(n, bitrev);
    }

  for(i = 0; i < n; i++)
    {
      j = bitrev[i];

      if(i < j)
	{
	  t = real[i];   real[i] = real[j];  real[j] = t;
	  t = imag[i];   imag[i] = imag[j];  imag[j] = t;
	}
    }

  for(k = 1; k < n; k = k2)
    {
      h = 0; k2 = k + k; d = n / k2;
      for(j = 0; j < k; j++)
	{
	  c = sintbl[h + n4];
	  s = (inverse) ? -sintbl[h] : sintbl[h];
	  for(i = j; i < n; i += k2)
	    {
	      ik = i + k;
	      dx = s * imag[ik] + c * real[ik];
	      dy = c * imag[ik] - s * real[ik];
	      real[ik] = real[i] - dx; real[i] += dx;
	      imag[ik] = imag[i] - dy; imag[i] += dy;
	    }
	  h += d;
	}
    }
  if(!inverse)
    for(i = 0; i < n; i++) { real[i] /= n; imag[i] /= n; }
  return 0;  
}

/*
 * PCM データを double 表現にする
 */
double * fftFloatWav(double *buf, short *wav, int len)
{
  double *dbuf;
  int i;

  dbuf = (NULL == buf) ? malloc(sizeof(double) * len) : buf;
  if(NULL == dbuf) return NULL;

  for(i = 0; i < len; i++) dbuf[i] = (double)wav[i];
  return dbuf;
}

/*
 * もっとも単純な FFT 関数を提供する
 */
int fftPureFFT(double *real, double *imag, int n)
{
  return core_fft(real, imag, n);
}

/*
 * PCM 波形に FFT をかける
 */
int fftFFT(double *pcm, int len, double **re, double **im, int *fN)
{
  double *real, *imag;
  int fftN, n;
  int i;

  fftN = 2; n = 1;
  while(fftN < len) fftN *= 2, n++;

  if(NULL == (real = malloc(sizeof(double) * fftN))) goto err_entry0;
  if(NULL == (imag = malloc(sizeof(double) * fftN))) goto err_entry1;

  /* 波形データを格納する */
  for(i = 0; i < len; i++) real[i] = pcm[i], imag[i] = 0.0;
  for(i = len; i < fftN; i++) real[i] = imag[i] = 0.0;

  /* FFT を行なう */
  core_fft(real, imag, fftN);

  *re = real;
  *im = imag;
  *fN = fftN;

  return 0;
  

 err_entry1:
  free(real);
 err_entry0:
  return -1;
}

/*
 * 逆 FFT(IFFT)を行なう
 */
int fftIFFT(int len, double *re, double *im,
	    double **r_re, double **r_im, int *fN)
{
  double *real, *imag;
  int fftN, n;
  int i;

  fftN = 2; n = 1;
  while(fftN < len) fftN *= 2, n++;

  if(NULL == (real = malloc(sizeof(double) * fftN))) goto err_entry0;
  if(NULL == (imag = malloc(sizeof(double) * fftN))) goto err_entry1;

  for(i = 0; i < len; i++) real[i] = re[i], imag[i] = im[i];
  for(i = len; i < fftN; i++) real[i] = 0, imag[i] = 0;

  /* FFT を行なう */
  core_fft(real, imag, -fftN);

  *r_re = real;
  *r_im = imag;
  *fN = fftN;

  return 0;
  

 err_entry1:
  free(real);
 err_entry0:
  return -1;
}
