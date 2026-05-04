#ifndef _text_h_
#define _text_h_

#include "analyze.h"

/* 平仮名、カタカナと、音素列の対応表をつくるための構造体 */
typedef struct _txt_list {
  char * hira;
  char * kata;

  char * phono[5];  /* 一つの文字に、音素は最大4つまで(4 + 終了コードの 5つ) */
} txtList;

/* テキストを音素単位に分解した、その音素一つ分をあらわす構造体 */
typedef struct _txt_phono {
  char   * label;      /* 音素ラベル     */

  int      gr_nums;    /* 音素群候補の数 */
  vGroup   group[3];   /* 音素群ラベル   */

} txtPhono;

/*
   一つの音素につき、音素群は複数考えられる。
   たとえば /g/ などは、

       無音セグメント(S)
       有声子音的セグメント(C)
       有声子音セグメント(D) 

   の三通りが考えられる。重複する C,D は別として、重複しない S としても
   考えられるように、音素群は最大 3 つまでの候補がある。
   */
   
   
txtPhono ** txtSplitText(char *fname);

#endif /* _text_h_ */

