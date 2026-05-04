#ifndef _util_h_
#define _util_h_

/* Big Endian データの読み書き */
int utReadWordBE(unsigned short * w, FILE *rfp);
int utReadLongBE(unsigned long  * l, FILE *rfp);

int utWriteWordBE(unsigned short w, FILE *wfp);
int utWriteLongBE(unsigned long  l, FILE *wfp);

/* Little Endian データの読み書き */
int utReadWordLE(unsigned short * w, FILE *rfp);
int utReadLongLE(unsigned long  * l, FILE *rfp);

int utWriteWordLE(unsigned short w, FILE *wfp);
int utWriteLongLE(unsigned long  l, FILE *wfp);

#endif /* _util_h_ */
