/*
	objcollision.c
		バウンダリボックスを使って、楕円球計算で当たり判定を
		行なう。

	1999/10/18 K.Kano
	$Id: objcollision.x,v 1.1.1.3 2002/11/19 11:42:57 Yoshizawa1 Exp $
*/

#ifndef _objcollision_h_
#define _objcollision_h_

#include "gameheader.h"

/* 楕円球の当たりに当たっていた場合、
   楕円球の中心と現在位置を結んだ直線と楕円球の交点に
   点を移動させる。 */
int CalcObjsCollision(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf);
int CalcObjCollision(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,int objnum,float paramf);
int CalcObjsCollisionWithInvM(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf,FMATRIX *inv_m);
int CalcObjsCollisionWithInvM2(FVECTOR *r,FVECTOR *x,DG_DEF *def,DG_OBJS *objs,
			       float paramf,FMATRIX *inv_m);
int CalcObjCollisionWithInvM(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,int objnum,
			     float paramf,FMATRIX *inv_m);
int CalcObjCollisionWithInvM2(FVECTOR *r,FVECTOR *x,DG_DEF *def,DG_OBJS *objs,int objnum,
			      float paramf,FMATRIX *inv_m);

/* 楕円球の当たりに当たっていた場合、
   楕円球のローカルの軸と現在位置を最短で結んだ直線と楕円球の交点に
   点を移動させる。 */
int CalcObjsCollisionV(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf,int axis);
int CalcObjsCollisionVWithInvM(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,
			       float paramf,int axis,FMATRIX *inv_m);
int CalcObjCollisionV(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,int objnum,float paramf,int axis);


/* 楕円球の当たりに当たっていた場合、
   楕円球の中心と指定の基点を結んだ直線と楕円球の交点に
   点を移動させる。 */
int CalcObjsCollisionB(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,float paramf);
int CalcObjsCollisionBWithInvM(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,
			       float paramf,FMATRIX *inv_m);
int CalcObjsCollisionBWithInvM2(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_DEF *def,DG_OBJS *objs,
				float paramf,FMATRIX *inv_m);
int CalcObjCollisionB(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,int objnum,float paramf);
int CalcObjCollisionBWithInvM(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,int objnum,
							  float paramf,FMATRIX *inv_m);
int CalcObjCollisionBWithInvM2(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_DEF *def,DG_OBJS *objs,int objnum,
							   float paramf,FMATRIX *inv_m);


/* バウンダリボックスをそのまま当たりとして用いて、
   バウンダリの縁でもっとも現在位置に近い点に
   点を移動させる。*/
int CalcObjsBoundary(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf);
int CalcObjsBoundaryWithInvM(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf,FMATRIX *inv_m);
int CalcObjBoundary(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,int objnum,float paramf);


#endif
