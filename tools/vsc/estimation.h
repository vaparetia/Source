#ifndef _estimation_h_
#define _estimation_h_

#include "mel.h"

/* 直線周波数目盛における 3HKz 前後の位置 */
#define EST_Line3KHz  (3000 * 512 / 16000)

double * estEstimation(double *wav, int fnums);

#endif /* _estimation_h_ */
