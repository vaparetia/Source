#ifndef _ref_load_h_
#define _ref_load_h_

#include "cepstrum.h"

#define CEP_M  22


/* リファレンスパターン列 */
typedef struct _refR {
  char *label;  /* そのリファレンスパターン列を持つ音素のラベル */
  cepVector r[5];
} refR;

typedef struct _refBlock {
  int    nums;
  refR ** refs;
} refBlock;

/* 
 * パターンマッチング用の辞書
 */
typedef struct _refDic {
  refBlock  W;
  refBlock  C;
  refBlock  U;
  refBlock  SV;
  refBlock  CW;
  refBlock  UW;
  refBlock  F;
  refBlock  X;
} refDic;

#ifdef _ref_load_c_
#define EXT
#define INIT(n)  = n
#else
#define EXT extern
#define INIT(n)
#endif /* _ref_load_c_ */

EXT refDic  refDICT;

#undef EXT
#undef INIT

/* =========================================================================
 *                        Function prototype
 * ========================================================================= */

int refLoadRef(char *fname);


#endif /* _ref_load_h_ */
