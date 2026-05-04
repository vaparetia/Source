#ifndef _c_layout_h_
#define _c_layoyt_h_

#include "codec_config.h"


/* VGA 座標を 2Dモジュール座標に変換するマクロ */
#define xV2S(x)   ((float)(x) * 512.0f / 640.0f)
#define yV2S(y)   ((float)(y) * 384.0f / 480.0f)

#define LAYOUT_PRIORITY  0   /* レイアウトパーツのスプライトプライオリティ */


/* 点灯/非点灯状態 */
#define CODEC_LIGHT_PTT     0x0001
#define CODEC_LIGHT_MEMORY  0x0002
#define CODEC_LIGHT_LEFT    0x0010
#define CODEC_LIGHT_RIGHT   0x0020
#define CODEC_LIGHT_TUNING  0x0040

/* =========================================================================
 * パーツ位置指定
 * ========================================================================= */

SPR_OBJ * codecGetObject(int name); /* 指定オブジェクト取得 */
int codecMemCallOpen(int selected);   /* メモリー呼び出しモードオープン   */

void codecLayoutDisplayStart(void); /* 表示開始 */

int codecMemCallClose(void);  /* メモリー呼び出しモードクローズ           */
int codecMemCallStatus(void); /* メモリー呼び出しモードオープン中チェック */
int codecMemCallSelect(int select);

void codecVibrationSwitch(int type);
void codecEarthQuake(int sw, int q_time, int amp);
void codecLayoutTuneLight(int stat);

void codecLayoutMovieSwitch(int sw);
int  codecLayoutMovieStatus(void);

int codecIsKeyWaitOK(void);

int codecActionStatus(void);  /* アクション再生中か停止中かを取得 */
int codecWaiting(int sw);     /* キー入力待ちアクションに移行     */
int codecSetFreq(int freq);   /* 周波数値を設定                   */
int codecOpenFrame(int fade); /* 枠を開く(接続,通話開始)          */
int codecCloseFrame(void);    /* 枠を閉じる(通話終了,切断)        */
int codecLayoutExit(void);    /* 無線終了アクション再生           */
int codecExitStatus(void);    /* 終了アクション進行状態取得       */
int codecLayoutDisp(int sw);  /* 表示/非表示切替え                */
int codecCallAction(int sw);  /* 呼び出しアクション起動           */
int codecTalkPTT(int sw);     /* PTT点灯                          */

int codecSetMaterAnimDisable(int flag);
void * NewCodecLayout(int skin, int exit_disp);
int codecErasePageIcon(void);


#define CalcMovePos(min, max, total, cnt) ((min) + (cnt) * ((max) - (min)) / total)

#endif /* _c_layout_h_ */
