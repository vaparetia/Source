#ifndef _util_2d_h_
#define _util_2d_h_

#include "sprite_2d.h"

int U2D_TreeAlpha(SPR_OBJ * obj, int alpha);
int U2D_TreeMulAlpha(SPR_OBJ * obj, int alpha);
void U2D_TreeProc(void * workp, SPR_OBJ * obj,
		  void (*proc)(void * workp, SPR_OBJ * obj));

#endif /* _util_2d_h_ */

