#ifndef _formant_h_
#define _formant_h_

#define ENVELOP_FREQ  400
#define ENVELOP_LEV   10
#define ENVELOP_SUBS  10



#ifdef _formant_c_
#define EXT
#define INIT(n) = n
#else
#define EXT extern
#define INIT(n)
#endif /* _formant_c_ */



#undef EXT
#undef INIT

int fmtGetFormant(double * retarray, int arraymax, double *spectrum, int fftN);

#endif /* _formant_h_ */
