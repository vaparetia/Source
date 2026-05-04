#ifndef _vmotion_h_
#define _vmotion_h_

#include "mtnlib.h"

/* =========================================================================
 *                                Macro
 * ========================================================================= */

#define VOWEL_S   0   /* 無音(閉じ) */
#define VOWEL_A   1   /* /a/ あ     */
#define VOWEL_I   2   /* /i/ い     */
#define VOWEL_U   3   /* /u/ う     */
#define VOWEL_E   4   /* /e/ え     */
#define VOWEL_O   5   /* /o/ お     */
#define VOWEL_N   6   /* /N/ ん     */
#define VOWEL_V   7
#define VOWEL_NUMS 8  /* 母音の種類数 */

#define VOWEL_unknown -1 /* 母音としては認識しえないものはこの値 */

/*
 * 顔のモーションの関節数
 */
#define FACE_JOINTS  32
#define BODY_JOINTS  21

/* =========================================================================
 *                               Type define
 * ========================================================================= */

/* 各母音に対応するモーションを保持する構造体 */
typedef struct _vmtn_vowel {
  struct _vmtn_vowel * prev;
  struct _vmtn_vowel * next;

  char      * set_label;
  int         joints;    /* 関節数 (顔32, 全身21) */
  int         mtn_len[ VOWEL_NUMS ];
  MTN_MOTION *mtn_vowel[ VOWEL_NUMS ];
} vmtnVowel;

/* 各表情に対応したモーションセットをまとめる構造体 */
typedef struct _vmtn_set {
  int         speaker_code;  /* 話者名コード */
  char      * speaker_name;  /* 話者名文字列 */
  float       bias;          /* パワーバイアス値 */
  vmtnVowel * begin;
  vmtnVowel * end;
} vmtnSet;




/* vowel = 母音 */



#ifndef _vmotion_c_
#define EXT extern
#define INIT(n)
#else
#define EXT
#define INIT(n) = n
#endif /* _vmotion_c_ */



#undef EXT
#undef INIT

/* =========================================================================
 *                                Prototype
 * ========================================================================= */

vmtnSet * vmtnReadMotion(char *infoname);
void      vmtnReleaseMotion(vmtnSet * Set);

#endif /* _vmotion_h_ */
