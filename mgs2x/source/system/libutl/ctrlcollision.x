/*
	ctrlcollision.x
		CONTROL構造体をチェックすることで、人間に当たっているかどうかを
		チェックする。チェックは頭から足先までの円筒を基準にする。

	2000/6/1 K.Kano
	$Id: ctrlcollision.x,v 1.1.1.3 2002/11/19 11:42:54 Yoshizawa1 Exp $
*/


#ifndef _ctrlcollision_x_
#define _ctrlcollision_x_


#include "gameheader.h"


int ControlsCollision(FVECTOR *r,FVECTOR *x,float paramf);
int ControlCollision(FVECTOR *r,FVECTOR *x,CONTROL *ctrl,float paramf);

#endif
