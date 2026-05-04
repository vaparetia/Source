#ifndef _codec_config_h_
#define _codec_config_h_

#include "g_sound.h"

#define CODEC_PARTS_LEVEL  136  /* 166 */

// #define DUMMY_BONE  4274856    // sna_def
#define DUMMY_BONE  12663374   // rai_def

/* 非 0 のとき、髪の毛の遅延描画を行わない */
#define DELAY_DRAW  1

/* 顔の上の緑フィルタをかけない */
#define NO_GREEN_FILTER
#undef NO_GREEN_FILTER

/* 背景無し */
#define NO_BACKGROUND
#undef NO_BACKGROUND

/* 背景の RGB 値 */
#define FACEBG_R   0
#define FACEBG_G   0
#define FACEBG_B   0


/* 操作したカメラ位置が残るよう設定 */
#define FACE_CAMERA_CHANGE
// #undef  FACE_CAMERA_CHANGE

/* カメラが下を向かないよう設定 */
#define CAMERA_NO_DOWN
#ifdef FACE_CAMERA_CHANGE
#undef CAMERA_NO_DOWN
#endif

/* デフォルトモーション再生を行うよう設定 */
#define FACE_DEFAULT_MOTION
/* #undef FACE_DEFAULT_MOTION */

/* カメラ操作 */
#define CAMERA_NO_SHIFT     /* 平行移動の禁止 */
#undef CAMERA_NO_SHIFT
#ifdef FACE_CAMERA_CHANGE
/* カメラ位置取得バージョンの場合は、常に平行移動を許可 */
#undef CAMERA_NO_SHIFT
#endif /* FACE_CAMERA_CHANGE */

#if 1
#ifdef DEBUG
#define _DEBUG_
#endif /* DEBUG */
#endif

#define CodecID 6888711	// "Codec"


/* ルビの高さ */
#define RUBI_HEIGHT  0


/* 背景の有無 */
#define CODEC_BACKGROUND
/* #undef CODEC_BACKGROUND */

/* 走査線エフェクト */
#define EFFECT_CRT
// #undef EFFECT_CRT

/* 顔表示窓縁のぼかし */
#define EFFECT_FACE_EDGE
#undef EFFECT_FACE_EDGE

/* 水平同期ずれエフェクト */
#define EFFECT_HSYNC
#undef EFFECT_HSYNC

/* 手ブレ効果 */
#define EFFECT_HANDYCAM
// #undef EFFECT_HANDYCAM

/* フォーカシング */
#define EFFECT_FOCUS
// #undef EFFECT_FOCUS

/* 髪の毛アニメーション(物理計算) */
#define EFFECT_HAIR
// #undef EFFECT_HAIR



/*
 * 無線中の SE
 */
#define CODEC_SE_CALL         SD_S_R_CALL01   /* 受信呼出音             */
#define CODEC_SE_CAMERA_CHG   SD_S_R_FACE01   /* 人物切替え             */
#define CODEC_SE_CANCEL       SD_S_R_CANCEL   /* キャンセル音           */
#define CODEC_SE_SEL_PERSON   SD_S_R_CURSOR   /* 人物選択(メモリー呼出) */
#define CODEC_SE_CONNECT      SD_S_R_DISP01   /* 接続ノイズ             */
#define CODEC_SE_SEND         SD_S_R_SND01    /* 呼出中音               */
#define CODEC_SE_TUNING       SD_S_R_TUNE01   /* チューニングされ～て～
						 黙っているのか～♪     */
#define CODEC_SE_WNDOPEN      SD_S_R_WINDW1   /* ウィンドウ開く         */
#define CODEC_SE_WNDCLOSE     SD_S_R_WINDW2   /* ウィンドウ閉じ         */

#define CODEC_MEM_WNDOPEN     SD_S_R_SEL01    /* メモリーコール開き     */
#define CODEC_MEM_WNDCLOSE    SD_S_R_CANCEL   /* メモリーコール閉じ     */

#endif /* _codec_config_h_ */
