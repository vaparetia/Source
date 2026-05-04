#ifndef _floor_h_
#define _floor_h_

#define FLOOR_CHECK_ENABLE

#define SQRT_3  1.7320508F      /* SQRT(3) の値 */

typedef struct HEX_FLOOR_INFO {
  struct HEX_FLOOR_INFO * prev;
  struct HEX_FLOOR_INFO * next;

  CONTROL * ctrl;       /* コントロール構造体のポインタ */
  FVECTOR   pre_mov;    /* 直前フレームの位置           */
  int       flags;
} HEX_FLOOR_INFO;

#define HEX_FLAG_UNDRAW   0x0001   /* プリミティブの追加を行わない */

int HEX_RegistControlInfo(HEX_FLOOR_INFO * info, CONTROL * ctrl);
int HEX_RemoveControlInfo(HEX_FLOOR_INFO * info);

#endif /* _floor_h_ */
