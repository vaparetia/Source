#ifndef _codec_config_h_
#define _codec_config_h_

#include "g_sound.h"

/* 無線内で使用する数少ない GV_Malloc() 用の領域として、
   よやくしておくヒープのサイズ */
#define CODEC_RESERVE_SIZE   (1024 * 128)

/* セーブ用アイコン名 */
#define CODEC_SAVE_ICON				0x0079d47b		/* testicon */
#define CODEC_SAVE_ICON_TNK			0x00e63830		/* tnkicon */
#define CODEC_SAVE_ICON_PLT			0x0076372d		/* plticon */

#define SAVEGAME_L2D_STRCODE		0x009deed4		/* save_load */

#define AUX_CALL_ENABLE   /* 強制/任意コールでアイコン表示を使い分ける */
// #undef AUX_CALL_ENABLE

#ifdef AUX_CALL_ENABLE

#define CALL_AUX     1
#define CALL_FORCE   0

#define CALL_ALARM_MAX 4   /* 呼び出し音の回数制限 */



/* AUX CALL 呼び分けバージョン */
#define CALL_L2D    0x003311ec   /* call */

#define CALL_forceCallP  0x0094e76b
#define CALL_forceCloseP 0x004e8c63
#define CALL_auxCallP    0x00656404
#define CALL_auxCloseP   0x005e1f7d

#define CALL_forceCallT  0x0094e76f
#define CALL_forceCloseT 0x004e8c67
#define CALL_auxCallT    0x00656408
#define CALL_auxCloseT   0x005e1f81

#else
/* 従来バージョン */
#define CALL_L2D    0x003311ec   /* call */

#define CALL_openCallPlant   0x0044dfb3
#define CALL_closeCallPlant  0x00eaaf6a
#define CALL_openCallTanker  0x00f26901
#define CALL_closeCallTanker 0x00ac5ff6

#endif

/* アイドル時のアクションは共通 */
#define CALL_idleCall        0x007673ca

#define CALL_NULL_1         0x00773672  /* NULL-1 */
#define CALL_ROOT           0x002a4634  /* ROOT   */


#define CODEC_PARTS_LEVEL  136  /* 166 */

// #define DUMMY_BONE  4274856    // sna_def
#define DUMMY_BONE  12663374   // rai_def

/* 非 0 のとき、髪の毛の遅延描画を行わない */
#define DELAY_DRAW  0

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

#define FACE_LIGHT_CHANGE
// #undef FACE_LIGHT_CHANGE


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

/* ノイズエフェクト */
#define EFFECT_NOISE
//#undef EFFECT_NOISE

/* 定常的ノイズエフェクト */
#define EFFECT_STATIC_NOISE
//#undef EFFECT_STATIC_NOISE



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
#define CODEC_SE_CAMERA_CHG_L SD_S_R_FACE01   /* 人物切替え(左)         */
#define CODEC_SE_CAMERA_CHG_R SD_S_R_FACE02   /* 人物切替え(右)         */
#define CODEC_SE_CANCEL       SD_S_R_CANCEL   /* キャンセル音           */
#define CODEC_SE_SEL_PARSON   SD_S_R_CURSOR   /* 人物選択(メモリー呼出) */
#define CODEC_SE_CONNECT      SD_S_R_DISP01   /* 接続ノイズ             */
#define CODEC_SE_SEND         SD_S_R_SND01    /* 呼出中音               */
#define CODEC_SE_TUNING       SD_S_R_TUNE01   /* チューニングされ～て～
						 黙っているのか～♪     */
#define CODEC_SE_WNDOPEN      SD_S_R_WINDW1   /* ウィンドウ開く         */
#define CODEC_SE_WNDCLOSE     SD_S_R_WINDW2   /* ウィンドウ閉じ         */

#define CODEC_SE_VTRFAST      SD_S_R_VTR_FF   /* ビデオ早送り音         */

#define CODEC_MEM_WNDOPEN     SD_S_R_SEL01    /* メモリーコール開き     */
#define CODEC_MEM_WNDCLOSE    SD_S_R_CANCEL   /* メモリーコール閉じ     */


/* 無線の SE は、デモ中であっても必ず鳴らなければならない。 */
#define CodecSE(_senum)  sd_set_cli(_senum)

extern int sd_set_cli(int);

#endif /* _codec_config_h_ */
