#ifndef _cdc_noise_h_
#define _cdc_noise_h_

#define VFAST_NOISE 4       /* 分割ノイズの本数(画面の分割数は + 1) */
#define VFAST_SPLIT (VFAST_NOISE - 1)
#define VFAST_DIST_LOW 3    /* ゆがみ(緩) */
#define VFAST_DIST_HIGH 5   /* ゆがみ(急) */

/* ノイズの寸法は,表示領域の大きさに対する比率で表現 */
#define VFAST_DIST_NOISE 0.04F  /* ノイズ自体のゆがみ */
#define VFAST_HEIGHT_NOISE 1  /* ノイズ領域一本あたりの高さ(ラスタ数) */

#define UnsyncV_HEIGHT 10
enum {
  /*
   * アナログ波 CRT のノイズ(先にあるものほど優先度が高い)
   */
  CDC_NOISE_SAND,    /* サンドノイズ(砂嵐)                                   */
  CDC_NOISE_VFAST,   /* ビデオの早送りのようなノイズ                         */
  CDC_NOISE_GHOST,   /* ゴースト(反射波による二重像)                         */
  CDC_NOISE_BIGWAV,  /* ビッグウェーブ(大規模な同期ズレによる映像の乱れ)     */

  CDC_NOISE_V_UNSYNC,/* 垂直同期ずれ(上下方向にスクロールするような同期ズレ) */
  CDC_NOISE_H_UNSYNC,/* 水平同期ずれ(一部ラスタの表示開始がずれる)           */
  CDC_NOISE_UNCOLOR, /* カラーバースト信号不良による色の不良(彩度落ち)       */

  /*
   * デジタルストリーミング動画のノイズ
   */
  CDC_NOISE_BLOCK,   /* デジタルブロックノイズ                               */
  CDC_NOISE_FRAMEOUT,/* フレーム落ち(途中コマ落ち)                           */

  CDC_NOISE_MAX      /* この値以上ならば無効                                 */
};


void * NewCodecNoise(int side, int type, int time_len);
void * NewCodecStaticNoise(int side);

#endif /* _cdc_noise_h_ */
