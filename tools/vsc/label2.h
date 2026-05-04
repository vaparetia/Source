#ifndef _label_h_
#define _label_h_

#include "analyze.h"
#include "text.h"

/*
 * 音素境界正規化用構造体
 */
typedef struct _lab_norm {
  vGroup  group;  /* その境界以降、次の境界があらわれるまでの音素群 */
  int     t;      /* 境界が存在する分析フレーム番号                 */
} labVList;

/* 母音的セグメントの記録用構造体 */
typedef struct _lab_vowel {
  double position;  /* 単語内の位置   */
  int    id;        /* 対応するデータ */
} labVowel;

typedef labVowel labConsonant;


typedef struct _lab_score {
  char   * label;   /* 音素ラベル                           */
  double   score;   /* その音素ラベルに割り当てられたスコア */
} labScore;


typedef struct _lab_lattice {
  int        t;        /* 音素境界の分析フレーム                         */
  int        dt;       /* セグメントの長さ(次の音素境界までのフレーム数) */
  vGroup     group;    /* 音素群                                         */

  int        sc_nums;  /* 候補の数                                       */
  labScore * score;    /* セグメントに割り当てる音素の候補               */
  
} labLattice;

/*
 * 音素群判定の閾値
 */
#define LBL_V_minAi -0.2
#define LBL_V_maxEi 10.0
#define LBL_V_maxFi 10.0
#define LBL_V_minFi -4.0
#define LBL_N_maxFi 10.0
#define LBL_N_maxZi 230.0

typedef struct  lab_param {
  double VminAi;

  double VmaxEi;
  double VmaxFi;
  double VminFi;

  double NmaxFi;
  double NmaxZi;

} labParam;

#ifdef _label_c_
#define EXT
#define INIT(n) = n
#else
#define EXT extern
#define INIT(n)
#endif /* _label_c_ */

EXT labParam  labP;

#undef EXT
#undef INIT

void labInit(void);
int  labLabeling(double skip_time,
		 txtPhono **tx_list, anaParam **p_list, int nums);

#endif /* _label_h_ */
