/*
	objcollision.c
		バウンダリボックスを使って、楕円球計算で当たり判定を
		行なう。

	1999/10/18 K.Kano
	$Id: objcollision.h,v 1.1.1.3 2002/11/19 11:43:26 Yoshizawa1 Exp $
*/

#ifndef _objcollision_h_
#define _objcollision_h_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <float.h>
#include <math.h>

#if PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

int CalcObjsCollision(FVECTOR *r,FVECTOR *x,OBJECT *obj,float paramf);
int CalcObjCollision(FVECTOR *r,FVECTOR *x,OBJECT *obj,int objnum,float paramf);
int CalcObjsBoundary(FVECTOR *r,FVECTOR *x,OBJECT *obj,float paramf);
int CalcObjBoundary(FVECTOR *r,FVECTOR *x,OBJECT *obj,int objnum,float paramf);

#endif
