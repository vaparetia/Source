#ifndef _cepstrum_h_
#define _cepstrum_h_

#define cepVEC_DIM   10    /* 特徴ベクトルとして保持する次数 */

/*
 * メルケプストラムで表される特徴ベクトル
 */
typedef struct _cepVector {
  double c[ cepVEC_DIM ];  /* メルケプストラムの値 */
} cepVector;


#ifdef _cepstrum_c_
#define EXT
#define INIT(n) = n
#else
#define EXT extern
#define INIT(n)
#endif /* _cepstrum_c_ */


#undef EXT
#undef INIT

/*
 * 特徴ベクトル抽出(メルケプストラム)
 */
cepVector * cepCalcVector(cepVector * retbuf, double * spectrum, int fftN);

/*
 * メルケプストラム同士の距離尺度
 */
double  cepDistanceHamming(cepVector *a, cepVector *b);
double  cepDistanceEuclidian(cepVector *a, cepVector *b);
double  cepDistanceChebychev(cepVector *a, cepVector *b);

double  cepDistanceNonlinear2D(cepVector *a, cepVector *b);

#endif /* _cepstrum_h_ */
