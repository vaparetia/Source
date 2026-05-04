#ifndef _matching_h_
#define _matching_h_

#include "analyze.h"
#include "label.h"
#include "ref_load.h"
#include "cepstrum.h"

typedef enum {
  SS_W,  /* W, /N/, /w/ の定常部                    */
  SS_C,  /* C, D, V(Wを除く)の定常部                */
  SS_U,  /* U, F の定常部                           */
  SS_SV, /* S-(V,W) の境界の過渡部                  */
  SS_CW, /* (C,D)-(W,V), V-W の境界の過渡部         */
  SS_UW, /* (U,F)-(W,V) の境界の過渡部              */
  SS_F,  /* 後続音がある 6フレーム以上の F,U の全体 */
  SS_X   /* 後続音がない 6フレーム以上の F,U の全体 */
} matchSubset;

typedef enum  {
  vec_NONE,    /* ベクトル不要         */
  vec_ALL,     /* 全体型特徴ベクトル   */
  vec_STAY,    /* 定常部型特徴ベクトル */
  vec_BORDER   /* 過渡部型特徴ベクトル */
} vecPosition;


typedef struct _mSubset {
  matchSubset subset;
  vecPosition position;
} mSubset;


/* 特徴ベクトル列 */
typedef struct _vecA {
  cepVector a[7];
} vecA;

/* 音素ラティスの候補構造体 */
typedef struct _matLattice {
  char      * label;   /* 音素ラベル                       */
  matchSubset subset;  /* リファレンスパターンのサブセット */
  double      D;       /* 距離                             */
  double      U;       /* 類似度                           */
} matLattice;

/* 音素記号ごとのスコア */
typedef struct _matScore {
  char   * label;   /* 音素ラベル */
  double   score;   /* スコア     */
  double   Uj;      /* 類似度     */
} matScore;


/* =========================================================================
 *                           Function prototype
 * ========================================================================= */
int matMatching(int skip, anaParam **p_list, int p_nums,
		labVList * v_list, int top, int len);




#endif /* _matching_h_ */
