#ifndef _outrange_h_
#define _outrange_h_

#include "g_struct.h"

#define OR_ARROW_R  128
#define OR_ARROW_G  128
#define OR_ARROW_B  128


/*
 * OUTRANGE_CTRL は RADAR_CTRL にキャスト可能。
 */
typedef struct _outrange_ctrl {
  RADAR_CTRL   radar;
  SPR_OBJ    * arrow;   /* 範囲外の場合の方向矢印 */
} OUTRANGE_CTRL;

void * NewRadarOutRangeTarget(short * base_rot, FVECTOR *zoom_rate
							  , SPR_OBJ * parent, float offset_y);

int RDR_InitOutrangeTarget(OUTRANGE_CTRL * orange,
			   FVECTOR * mov, int flag, int map);
int RDR_FinishOutrangeTarget(OUTRANGE_CTRL * orange);


#endif /* _outrange_h_ */
