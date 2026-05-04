//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ctrlcollision.c
		CONTROL構造体をチェックすることで、人間に当たっているかどうかを
		チェックする。チェックは頭から足先までの円筒を基準にする。

	2000/6/1 K.Kano
	$Id: ctrlcollision.c,v 1.1.1.3 2002/11/19 11:42:54 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>

#ifndef KP_XBOX
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


/*
  r      ... 移動位置
  x      ... チェックすべき点
  paramf ... チェック半径
 */

int ControlsCollision(FVECTOR *r,FVECTOR *x,float paramf)
{
	int ans=0;
	int i=GM_N_WhereList;

	fpu_CopyVector(r,x);

	while(i>0){
		OBJECT *obj;
		FVECTOR p,q;
		float hmax,hmin;
		float l;

		i--;
#if 0
		obj=(OBJECT *)(GM_WhereList[i]+1);
#else	
		/* 修正 2000/06/02 M.Sonoyama */
		obj = GM_WhereList[ i ]->object ;
		if ( obj == NULL ) continue ;
#endif
		if(obj->objs==NULL) continue;
		if(obj->objs->n_models<21) continue;

		fpu_CopyVector(&p,(FVECTOR *)&(obj->objs->world.m[3][0]));
		hmax=hmin=p.vy;
		if(hmin>obj->objs->objs[HUMAN21_ATAMA].world.m[3][1]){
			hmin=obj->objs->objs[HUMAN21_ATAMA].world.m[3][1];
		}
		else if(hmax<obj->objs->objs[HUMAN21_ATAMA].world.m[3][1]){
			hmax=obj->objs->objs[HUMAN21_ATAMA].world.m[3][1];
		}
		if(hmin>obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1]){
			hmin=obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1];
		}
		else if(hmax<obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1]){
			hmax=obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1];
		}
		if(hmin>obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1]){
			hmin=obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1];
		}
		else if(hmax<obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1]){
			hmax=obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1];
		}

		if(r->vy>hmin && r->vy<hmax){
			q.vx=r->vx-p.vx;
			q.vy=0.0f;
			q.vz=r->vz-p.vz;
			p.vy=r->vy;
			q.vw = 1.0f ;

			vu0_Ldv0(&q);
			vu0_Ldv1(&p);
			l=vu0_VectorLength2v0();
			l=fpu_Rsqrt(l,paramf);
			if(l>1.0f){
				vu0_Mulv0a(l);
				ans=1;
			}
			vu0_Addv0v1();
			vu0_Stv0(r);
		}
	}

	return ans;
}


int ControlCollision(FVECTOR *r,FVECTOR *x,CONTROL *ctrl,float paramf)
{
	int ans=0;

	fpu_CopyVector(r,x);

	{
		OBJECT *obj;
		FVECTOR p,q;
		float hmax,hmin;
		float l;

		obj=(OBJECT *)(ctrl+1);

		if(obj->objs==NULL) return 0;
		if(obj->objs->n_models<21) return 0;

		fpu_CopyVector(&p,(FVECTOR *)&(obj->objs->world.m[3][0]));
		hmax=hmin=p.vy;
		if(hmin>obj->objs->objs[HUMAN21_ATAMA].world.m[3][1]){
			hmin=obj->objs->objs[HUMAN21_ATAMA].world.m[3][1];
		}
		if(hmin>obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1]){
			hmin=obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1];
		}
		if(hmin>obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1]){
			hmin=obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1];
		}
		if(hmax<obj->objs->objs[HUMAN21_ATAMA].world.m[3][1]){
			hmax=obj->objs->objs[HUMAN21_ATAMA].world.m[3][1];
		}
		if(hmax<obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1]){
			hmax=obj->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1];
		}
		if(hmax<obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1]){
			hmax=obj->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[3][1];
		}

		if(r->vy>hmin && r->vy<hmax){
			q.vx=r->vx-p.vx;
			q.vy=0.0f;
			q.vz=r->vz-p.vz;
			p.vy=r->vy;

			vu0_Ldv0(&q);
			vu0_Ldv1(&p);
			l=vu0_VectorLength2v0();
			l=fpu_Rsqrt(l,paramf);
			if(l>1.0f){
				vu0_Mulv0a(l);
				ans=1;
			}
			vu0_Addv0v1();
			vu0_Stv0(r);
		}
	}

	return ans;
}
