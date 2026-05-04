#ifndef _group_h_
#define _group_h_

#include "token.h"

typedef enum {
  V_W,         /* 母音的セグメント       */
  V_D,         /* 有声子音セグメント     */
  V_C,         /* 有声子音的セグメント   */
  V_V,         /* 有声音的セグメント     */
  V_F,         /* 無声摩擦音的セグメント */
  V_U,         /* 無声子音的セグメント   */
  V_S,         /* 無音セグメント         */
  V_unknown    /* 不明,もしくは未判別    */
} vGroup;



typedef struct _grGroup {
  char * label;     /* 音素ラベル                   */
  int    begin;     /* 音素セグメント開始フレーム   */
  int    end;       /* 音素セグメント終了フレーム   */

  vGroup group;     /* 音素セグメントの音素群ラベル */

} grGroup;


grGroup ** grReadGroup(TOKEN *tkn);
void       grReleaseGroup(grGroup ** g_list);

#endif /* _group_h_ */
