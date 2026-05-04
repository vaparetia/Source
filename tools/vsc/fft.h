#ifndef _fft_h_
#define _fft_h_

#include "env.h"


/* =========================================================================
 * Function prototypes
 * ========================================================================= */

int      fftPureFFT(double *real, double *imag, int n);

int      fftFFT(double *pcm, int len, double **re, double **im, int *fN);
int      fftIFFT(int len, double *r, double *i,
		 double **re, double **im, int *fN);
double * fftFloatWav(double * buf, short *pcm, int len);

#endif /* _fft_h_ */
