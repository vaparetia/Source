/*
 * 音声認識環境条件の定義ヘッダ
 */
#ifndef _env_h_
#define _env_h_

/* =========================================================================
 * Macros
 * ========================================================================= */
#define FRAME_STEP     160  /* 16[KHz] データ中 10[ms]分に相当するフレーム数 */
#define FRAME_WINDOW   400  /* 16[KHz] データ中 25[ms]分に相当するフレーム数 */

#define DEF_MFCCDIM    12
#define DEF_FBANK      24
#define DEF_SMPPERIOD  625
#define DEF_PREEMPH    ((double)0.97)
#define DEF_CEPLIF     22
#define DEF_DELWIN     2

#endif /* _env_h_ */
