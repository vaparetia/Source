#ifndef _vec_h_
#define _vec_h_

#include "token.h"

typedef enum _vVecMode {
  vec_ALL,      /* 全体型   */
  vec_CENTER,   /* 定常部型 */
  vec_BORDER    /* 過渡部型 */
} vVecMode;

/*
 * 特徴ベクトルとして用いるメルケプストラムのブロック
 */
typedef struct _vecCep {
  int      dim;  /* メルケプストラムの次数 */
  double * cep;  /* メルケプストラムの実体 */
} vecCep;


typedef struct _vecVector {
  char *label;         /* 音素ラベル                       */

  int begin;           /* 音素セグメントの開始分析フレーム */
  int end;             /* 音素セグメントの終了分析フレーム */

  vecCep all[5];       /* 全体型特徴ベクトル               */
  vecCep center[5];    /* 定常部型特徴ベクトル             */
  vecCep border[5];    /* 過渡部型特徴ベクトル             */

} vecVector;

vecVector ** vecReadVector(TOKEN * tkn);
void         vecReleaseVector(vecVector ** v_list);

#endif /* _vec_h_ */
