#ifndef _lip_h_
#define _lip_h_

/*
 * 全言語共通の音韻 ID */
#define LIP_unknown 255
#define LIP_sil    0     /* 無音 */

/*
 * 口の形
 */
#define PHONE_unknown -1
#define PHONE_S    0  /* ニュートラルの形状(無音) */

#define PHONE_A    1  /* /a/ の形 */
#define PHONE_I    2  /* /i/ の形 */
#define PHONE_U    3  /* /u/ の形 */
#define PHONE_E    4  /* /e/ の形 */
#define PHONE_O    5  /* /o/ の形 */
#define PHONE_N    6  /* /N/ の形 */

/*
 * 日本語の口の形
 */
#ifdef JAPANESE

#define PHONE_uA   7  /* /u/ の形状から 母音形状に変化 */
#define PHONE_iA   8  /* /i/ の形状から /a/ の形状に変化 */
#define PHONE_iU   9  /* /i/ の形状から /u/ の形状に変化 */
#define PHONE_iO   10 /* /i/ の形状から /o/ の形状に変化 */

#endif /* JAPANESE */

/*
 * 英語の口の形
 */
#ifdef ENGLISH

#define PHONE_V    7  /* /V/ の形(下唇を噛むように) */
#define PHONE_ae   8  /* /@/ の形(/a/ と /e/ の中間)*/
#define PHONE_aI   9  /* /a/->/i/ に変化            */
#define PHONE_aU   10 /* /a/->/u/ に変化            */
#define PHONE_EI   11 /* /e/->/i/ に変化            */
#define PHONE_cI   12 /* /o/->/i/ に変化            */
#define PHONE_cU   13 /* /o/->/u/ に変化            */

#endif /* ENGLISH */

typedef struct bin_seq {
  unsigned short  t;
  unsigned char   phone_id;
  unsigned char   power;
} BinSeq;

#define TERM_NUMBER  (0xffff)


#endif /* _lip_h_ */

