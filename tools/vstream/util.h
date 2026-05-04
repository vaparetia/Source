#ifndef _util_h_
#define _util_h_

int utWriteWord(unsigned short w, FILE *wfp);
int utWriteLong(unsigned long l, FILE *wfp);
int utWriteWordBE(unsigned short w, FILE *wfp);
int utWriteLongBE(unsigned long l, FILE *wfp);

#endif /* _util_h_ */
