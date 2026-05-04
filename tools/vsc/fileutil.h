#ifndef _fileutil_h_
#define _fileutil_h_

/* ビッグエンディアンデータの読み込み */
int fgetwBE(unsigned short * w, FILE *rfp);
int fgetlBE(unsigned long  * l, FILE *rfp);

/* リトルエンディアンデータの読み込み */
int fgetwLE(unsigned short * w, FILE *rfp);
int fgetlLE(unsigned long  * l, FILE *rfp);

#endif /* _fileutil_h_ */
