#ifndef _english_h_
#define _english_h_

#include "text.h"

/*
 * 英単語発音 Klattese 列辞書構造体
 */
typedef struct _engDIC {
  char  * word;   /* 英単語の表記 */

  int     len;    /* Klattese 列のステップ数 */
  char ** klat;   /* Klattese 列 */
} engDIC








#endif /* _english_h_ */
