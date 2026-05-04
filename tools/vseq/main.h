#ifndef _main_h_
#define _main_h_

#define MIN_STEP (5.5)

typedef struct seq {
  int    t;       /* 時刻   */
  char * phone;   /* 音素   */
  float  power;   /* パワー */
} Seq;
  
/*
 * 対応言語モード
 * 現時点で対応しているのは日本語と英語のみ。
 */
typedef enum {
  JAPANESE,    /* 日本語     */
  ENGLISH,     /* 英語       */
  FRENCH,      /* フランス語 */
  ITALIAN,     /* イタリア語 */
  GERMANY,     /* ドイツ語   */
  unknown
} LANG;

typedef enum {
  FALSE,        /* 偽 */
  TRUE          /* 真 */
} BOOLEAN;

#endif /* _main_h_ */
