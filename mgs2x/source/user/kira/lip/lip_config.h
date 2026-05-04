#ifndef _lip_config_h_
#define _lip_config_h_

/*
 * デバッグメッセージを表示させる場合は定義する
 */
/* #define FACE_DEBUG */

/*
 * 英語用モーションをチェックする場合に有効にする
 */
#if  0
#undef JAPANESE_BP_IGNORE()
#define ENGLISH    /* 英語モーションテスト用 */
#endif

/*
 * 発話中の無音部で,口を半開きのままにしたい場合は定義する
 */
/* #define SILENT_NO_CLOSE_MOUTH */

#endif /* _lip_config_h_ */
