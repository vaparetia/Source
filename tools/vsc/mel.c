/*
 * 周波数目盛変換を行う関数群
 *
 * o Hz 単位
 * o 角速度単位
 * o FFT 周波数目盛単位
 *
 * の,それぞれの周波数単位間での変換を行う。
 * また、メル周波数目盛と直線周波数目盛の変換も扱う
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define _mel_c_
#include "mel.h"
#include "pcmdata.h"

/*
 * 周波数を,角速度Ωに変換する
 */
double melFreq2Omega(double freq, int fftN)
{
  return 2 * M_PI * freq / PCM_OUTFREQ;
}

/*
 * 角速度ωを, Hz 単位の周波数に変換する
 */
double melOmega2Freq(double omega, int fftN)
{
  return omega * PCM_OUTFREQ / (2 * M_PI);
}

/*
 * ω を,FFT周波数目盛で表す
 */
double melOmega2FFT(double omega, int fftN)
{
  return omega * fftN / (2 * M_PI);
}

/*
 * FFT 周波数目盛を,ωに直す
 */
double melFFT2Omega(double fft, int fftN)
{
  return fft * 2 * M_PI / fftN;
}

/*
 * FFT 周波数目盛から、Hz 単位の周波数に変換する
 */
double melFFT2Freq(double n, int fftN)
{
  double freq;
  freq = n * PCM_OUTFREQ / (double)fftN;
  return freq;

}

/*
 * Hz 単位の周波数を、FFT 要素に変換する
 */
double melFreq2FFT(double freq, int fftN)
{
  return freq * fftN / PCM_OUTFREQ;
}

/*
 * 直線周波数のΩを、メル周波数のΩに変換する
 */
double melOmega2MelOmega(double omega)
{
  return omega + 2 * atan((MEL_ALPHA * sin(omega))
			  / (1 - MEL_ALPHA * cos(omega)));
}

/*
 * メル周波数目盛のΩを、直線周波数目盛のΩに変換する
 */
double melMelOmega2Omega(double mel_omega)
{
  return mel_omega - 2 * atan((MEL_ALPHA * sin(mel_omega))
			      / (1 + MEL_ALPHA * cos(mel_omega)));
}


/*
 * 直線周波数から、メル周波数目盛の FFT 単位における要素番号を算出する
 */
double melFreq2MelFFT(double freq, int fftN)
{
  double omega, mel_omega;
  double fft_n;

  /* Hz 単位の周波数を,角速度ωの単位に直す */
  omega = melFreq2Omega(freq, fftN);

  /* 直線周波数から、メル周波数を算出する */
  mel_omega = melOmega2MelOmega(omega);

  /* 算出されたメル周波数は角速度なので、FFT 周波数目盛に変換する */
  fft_n = melOmega2FFT(mel_omega, fftN);

  return fft_n;
}

/* 
 * メルFFT周波数目盛から、直線周波数への変換を行う
 */
double melMelFFT2Freq(double melFFT, int fftN)
{
  double mel_omega, omega, freq;

  mel_omega = melFFT2Omega(melFFT, fftN);
  omega = melMelOmega2Omega(mel_omega);
  freq = melOmega2Freq(omega, fftN);
  return freq;
}

/*
 * 直線周波数を、メル周波数に変換
 */
double melFreq2MelFreq(double freq, int fftN)
{
  double omega, mel_omega, mel_freq;

  omega = melFreq2Omega(freq, fftN);
  mel_omega = melOmega2MelOmega(omega);
  mel_freq = melOmega2Freq(mel_omega, fftN);

  return mel_freq;
}
