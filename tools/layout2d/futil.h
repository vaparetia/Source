#ifndef _futil_h_
#define _futil_h_

#ifndef _futil_c_
#define EXT extern
#else
#define EXT
#endif /* _futil_c_ */



#undef EXT

int fgetl(unsigned long * l, FILE *rfp);
int fgetw(unsigned short * w, FILE *rfp);

int fputl(unsigned long l, FILE *wfp);
int fputw(unsigned short w, FILE *wfp);

int fput_zero_fill(int len, FILE *wfp);

#endif /* _futil_h_ */
