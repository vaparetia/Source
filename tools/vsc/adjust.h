#ifndef _adjust_h_
#define _adjust_h_

#include "analyze.h"

typedef struct _adj_list {

  int      t;      /* 音素境界が登場するフレーム         */
  int      dt;     /* 直後の音素境界までの検出フレーム数 */

  char   * label;  /* 割り当てられている音素ラベル       */
  vGroup   group;  /* 割り当てられている音素群           */

  int      status; /* その音素割り当てのステータス       */

} adjList





#endif /* _adjust_h_ */
