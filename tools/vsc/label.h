#ifndef _label_h_
#define _label_h_

#include "analyze.h"
#include "text.h"
#include "vowel.h"

#define _VPOS_WITH_TIME_

#define VMODE_MALE   0
#define VMODE_FEMALE 1

#define DITECT_WAIT  0.5

#define DIST_MAX  500  /* フォルマントから抽出されたベクトルと、
			  基準ベクトルの距離がこの値以上であれば、
			  音韻識別の結果には信頼性が無いとして無視する */

/*
 * 音素境界正規化用構造体
 */
typedef struct _labS_norm {
  char  * label;        /* ラベルメンバ。ラベリングを終えるまでは NULL    */
  vGroup  group;        /* その境界以降、次の境界があらわれるまでの音素群 */
  int     t;            /* 境界が存在する分析フレーム番号                 */
  int     time;         /* セグメントのフレーム数                         */
  double  power;        /* セグメント中のパワー平均
			   (零次メルケプストラム係数平均)                 */
  vowelDistance male;   /* 母音の場合,男声とした場合の候補と距離          */
  vowelDistance female; /* 母音の場合,女声とした場合の候補と距離          */
} labVList;

/* 母音的セグメントの記録用構造体 */
typedef struct _lab_vowel {
  double position;  /* 単語内の位置   */
  int    id;        /* 対応するデータ */
} labVowel;

typedef labVowel labConsonant;
typedef labVowel labMute;

/*
 * 音素群判定の閾値
 */
#define LBL_W_maxEi 0.2
#define LBL_C_maxAi 0.2
#define LBL_D_maxFi 10.0
#define LBL_N_maxZi 230.0
#define LBL_N_maxFi 10.0

typedef struct  lab_param {

  double WmaxEi;    /* 母音的セグメントの条件     */
  double CmaxAi;    /* 有声子音的セグメントの条件 */
  double DmaxFi;    /* 有声子音セグメントの条件   */

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
int  labLabeling(int skip, txtPhono **tx_list,
		 anaParam **p_list, int nums, int voice_mode);

#endif /* _label_h_ */
