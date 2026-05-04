#ifndef _segmentation_h_
#define _segmentation_h_

#include "analyze.h"


#define U(n)  (((n) > 0) ? 1 : 0)


/* =========================================================================
 * -------------------------------------------------------------------------
 *                          各種閾値の初期値
 * -------------------------------------------------------------------------
 * ========================================================================= */
#define SEG_Tvl       0.5
#define SEG_Tvh       2

#define SEG_Ta1       0.12
#define SEG_Ta2       0.18
#define SEG_Ta3       0.25

#define SEG_Te1       0.6
#define SEG_Te2       1.5

#define SEG_Td        0.3

#define SEG_Tcl      -3.5
#define SEG_Tch      -2.5
#define SEG_Tzl      55
#define SEG_Tzh     100

#define SEG_Ts        2.5
#define SEG_Tf        0.5
#define SEG_Tf2       8.0

/*
 * 極大/極小を求めるためのパラメータ
 */
typedef struct _seg_peek {
  double border;  /* 閾値     */
  double sub;     /* 誤差範囲 */
} segPeek;

/*
 * セグメンテーションを行なうためのパラメータをまとめた構造体
 */
typedef struct _seg_param {
  /*
   * 有声音区間識別値
   */
  double   Tvl, Tvh;  /* 有声音部、非有声音部を分離する、Vi に対する閾値 */

  /*
   * 音素境界識別値
   */
  /* 新たに実装された部分用の閾値保持部分 */
  double Ta1;  /* 有声音の音素境界判別用の閾値(高) */
  double Ta2;  /* 有声音の音素境界判別用の閾値(低) */
  double Ta3;  /* 有声子音の後続部における修正に使用する閾値 */
  
  double Te1;  /* 有声音の音素境界判別用の閾値 */
  double Te2;  /* 有声子音の後続部における修正に使用する閾値 */
  
  double Td;   /* 有声子音的/有声子音セグメントを区別する閾値 */
  
  double Tcl;  /* 非有声音区間の、無声音、無音の区間分けに使用($c_i$, 低) */
  double Tch;  /* 非有声音区間の、無声音、無音の区間分けに使用($c_i$, 高) */
  double Tzl;  /* 非有声音区間の、無声音、無音の区間分けに使用($z_i$, 低) */
  double Tzh;  /* 非有声音区間の、無声音、無音の区間分けに使用($z_i$, 高) */

  double Ts;   /* 有音、無音論理値の閾値(2.5固定)                         */
  double Tf;   /* 摩擦音判別論理値の閾値(0.5固定)                         */
  double Tf2;  /* 無声音区間セグメンテーションに使用する $f_i$ の閾値     */
  
} segParam;


#ifdef _segmentation_c_
#define EXT
#define INIT(n)  = n
#else
#define EXT extern
#define INIT(n)
#endif /* _segmentation_c_ */

EXT segParam  segP;  /* セグメンテーション用パラメータ保持変数 */

#undef EXT
#undef INIT

void segInit(void);
int  segDitectMute(anaParam *param);
void segSegmentation(anaParam **p_list, int nums);
int  segLoadParams(char *opt, char *home, char *global);

#endif /* _segmentation_h_ */
