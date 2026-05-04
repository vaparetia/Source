#ifndef _mel_h_
#define _mel_h_

#define MEL_ALPHA 0.47

/*
  関数名に使用されている語句の意味

   Freq      Hz 単位の周波数

   Omega     角速度
   MelOmega  メル周波数目盛における角速度 ( $\tilde{\Omega}$ )

   FFT       FFT 単位
   MelFFT    メル周波数目盛における FFT 単位
*/
double melFreq2Omega(double freq, int fftN);
double melOmega2Freq(double omega, int fftN);
double melOmega2FFT(double omega, int fftN);
double melFFT2Omega(double fft, int fftN);
double melFFT2Freq(double fft, int fftN);
double melFreq2FFT(double freq, int fftN);
double melOmega2MelOmega(double omega);
double melMelOmega2Omega(double mel_omega);
double melFreq2MelFFT(double freq, int fftN);
double melMelFFT2Freq(double melFFT, int fftN);
double melFreq2MelFreq(double freq, int fftN);

#endif /* _mel_h_ */
