#ifndef _spectrum_h_
#define _spectrum_h_

typedef struct spec_spectrum {
  double envG;  /* スペクトル包絡     */
  double G;     /* メル対数スペクトル */
}specSpectrum;

specSpectrum * specMelLogSpectrum(double alpha, int fftN, double *dwav,
				  double *real, double *imag);


#endif /* _spectrum_h_ */
