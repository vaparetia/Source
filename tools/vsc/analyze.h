#ifndef _analyze_h_
#define _analyze_h_

#include "env.h"
#include "pcmdata.h"
#include "mel.h"

#define MEL_CEPSTRUM_M 12  /* メルケプストラムの次数 */

#define ST_VOICED  0x0001  /* 有声音フレーム                            */
#define ST_VSEG    0x0002  /* 音素境界マーク                            */
#define ST_SILENT  0x0004  /* 無音区間(声として認識すべき音が無い)      */

#define ST_BORDER  (ST_VSEG)

/* 無声音かどうかを判定するマクロ */
#define IsNonVoiced(s)  (!((s) & (ST_VOICE | ST_SILENT)))

/* 音素境界かどうかを判定するマクロ */
#define IsVoiceSegment(s)  ((s) & ST_VSEG)

/* フォルマント計算のための、スペクトル包絡保持最大要素数 */
#define FORMANT_BUF  176  /* メル周波数目盛に変換した 3000Hz 付近の値 */


/* Vi の値を算出する際に使用する、主要周波数帯域の、下限と上限 */
#define Vi_UNDER   75
#define Vi_UPPER   300

#define Vi_MIN  5
#define Vi_MAX  22
#define Vi_PAR  (Vi_MAX - Vi_MIN + 1)

typedef enum {
  V_W, /* 母音的セグメント    : /a/ /i/ /u/ /e/ /o/ /N/ /ya/ /yu/ /yo/ /wa/ */
  V_D, /* 有声子音セグメント  : /d/ /b/ /g/                                 */
  V_C, /* 有声子音的セグメント: /y/ /w/ /g/ /m/ /n/ /z/ V_D                 */
  V_V, /* 有声音的セグメント  : V_W, V_C                                    */
  V_F, /* 無声摩擦音的セグメント: /k/ /h/ /s/ /c/ /x/                       */
  V_U, /* 無声子音的セグメント: /p/ /t/ V_F                                 */
  V_S, /* 無音セグメント      : /g/ 無音                                    */

  V_unknown  /* 不明(未判別) */
} vGroup;


typedef struct _ana_param {
  int    i;   /* 検出フレーム番号                  */

  double Ci;  /* 零次メルケプストラム係数          */
  double Zi;  /* 零交差数(音声の波が 0 を通る回数) */
  double Vi;  /* 有声音検出パラメータ              */
  double Ai;  /* 零次メルケプストラム時間変化      */
  double Fi;  /* 零交差数時間変化                  */
  double Ei;  /* スペクトル包絡時間変化(基準値)    */

  double Si;  /* スペクトル包絡時間変化            */

  double gi[MEL_CEPSTRUM_M];  /* メルケプストラム  */

  unsigned short status;  /* セグメンテーションの際に使用するステータス */

  vGroup  group;  /* 音素群ラベル(音素の属するグループ)            */
  char  * label;  /* 音素ラベル                                    */

  int     V;      /* 有声音/非有声音の区分を行なうためのパラメータ */

  double  w_max;  /* 分析フレームの先頭 10[ms] 中における最大値    */
  double  w_min;  /* 分析フレームの先頭 10[ms] 中における最小値    */

  double  *spectrum; /* スペクトル包絡自体                     */

  double Tvh, Tvl;  /* そのフレームが有声音か無声音か判別するための閾値 */

} anaParam;


anaParam * anaParamNew(int i);
int        anaCalcParam(anaParam * param, short *wav, int N);
int        anaLatticeParameter(anaParam ** p_list, int nums);

#endif /* _analyze_h_ */
