//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rgbblur.c
	rgb分解したモデルをランダム表示
	2002/03/12 K.Sigeno
	$Id: rgbblur.c,v 1.1.1.3 2002/11/19 11:49:48 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
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

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	FMATRIX		*world ;
	FVECTOR		vel ;
	FVECTOR		pos ;
	FVECTOR		c_pos ;
	FVECTOR		len	;
	CVECTOR		color;
	SVECTOR		rot ;
	int			cnt ;
	int			limit ;
	int			mode	;
} Work ;
static void SetAmb(Work *work,u_char r,u_char g,u_char b){
	work->lights[1].m[ 3 ][ 0 ] = (float) r ;
	work->lights[1].m[ 3 ][ 1 ] = (float) g ;
	work->lights[1].m[ 3 ][ 2 ] = (float) b ;
}

//#define	DECAY_RATE	(0.0001f)
//#define	G_RATE	(0.00001f)
//#define	G_RATE	(0.0001f)
#define	G_RATE	(0.001f)

//#define	DECAY_RATE	(0.99985f)
//#define	DECAY_RATE	(0.99f)
//#define	DECAY_RATE	(0.985f) /*120*/
#define	DECAY_RATE	(0.980f)

static void randgr(float *vel,float pos){
	if(pos> 0.0f){
		*vel -= (pos * pos)*G_RATE;
	}else {
		*vel += (pos * pos)*G_RATE;
	}
	*vel *= DECAY_RATE;

}
static void Act(Work *work)
{
	FVECTOR	rand ,scale;
	float rate ,r,g,b,theta;
#if 0
	/*重力処理*/
	randgr(&(work->vel.vx),work->pos.vx) ;
	randgr(&(work->vel.vy),work->pos.vy) ;
	randgr(&(work->vel.vz),work->pos.vz) ;

	work->pos.vx += work->vel.vx ;
	work->pos.vy += work->vel.vy ;
	work->pos.vz += work->vel.vz ;

	DG_SetPos( work->world ) ;
	DG_MovePos(&work->pos) ;
	DG_PutObjs( work->objs );
#else

//	theta = (float)M_PI*2.0f*(float)work->cnt ;
//	theta = (float)M_PI*2.0f*(float)work->cnt/(float)work->limit ;
//	theta = (float)M_PI*4.0f*(float)work->cnt/(float)work->limit ;
//	theta = (float)M_PI*8.0f*(float)work->cnt/(float)work->limit ;
	theta = (float)M_PI*14.0f*(float)work->cnt/(float)work->limit ;

	rate = (float)(work->cnt)/(float)work->limit ;

//printf("work->len.vx[%f]\n",work->len.vx);

	work->pos.vx = work->len.vx * sinf(theta) * rate ;
	work->pos.vy = work->len.vy * sinf(theta) * rate ;
	work->pos.vz = work->len.vz * sinf(theta) * rate ;

	DG_SetPos( work->world ) ;
	DG_MovePos(&work->pos) ;
#if 0
	scale.vx = scale.vy = scale.vz = 1.25f ;
	scale.vw = 1.0f ;
	DG_ScalePos(&scale) ;
#endif
	DG_PutObjs( work->objs );

	{
		DG_SetPos2( &DG_ZeroVector, &work->rot ) ;
		DG_PutVector( &work->len, &work->len, 1 ) ;
	}

#endif

//	rate = (float)work->cnt/(float)work->limit ;
#if 0
	rate = 2.0f*(float)(work->limit-work->cnt)/(float)work->limit ;
#else 
	rate = 2.0f*(float)(work->limit-work->cnt)/(float)work->limit ;
	if((work->limit/2)<work->cnt){
		rate = 4.0f*(float)(work->limit-work->cnt)/(float)work->limit ;
	}else {
		rate = 4.0f*(float)(work->cnt)/(float)work->limit ;
	}

#endif
	r = (float)work->color.r * rate ;
	g = (float)work->color.g * rate ;
	b = (float)work->color.b * rate ;

//	if(r>255.000f) r=255.000f ;
//	if(g>255.000f) g=255.000f ;
//	if(b>255.000f) b=255.000f ;

	SetAmb(work,(u_char)r,(u_char)g,(u_char)b);


	work->cnt-- ;
	if(work->cnt <=0) {
		GV_DestroyActor(work) ;
	}
}
static void Die(Work *work)
{
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs );
}


#define ROT_LIM		64
static int GetResources(Work *work, int mdl,FMATRIX *world,float rate,int time,CVECTOR *col,int dir,int rot_y)
{
	DG_DEF	*def ;
	FVECTOR	rand ;
	SVECTOR rot ;

#if 0
	rot.vx = (irnd()>>8) & 4095 ;
	rot.vy = (irnd()>>8) & 4095 ;
	rot.vz = (irnd()>>8) & 4095 ;

	work->rot.vx = ((irnd()>>8) % ROT_LIM ) - (ROT_LIM/2) ;
	work->rot.vy = ((irnd()>>8) % ROT_LIM ) - (ROT_LIM/2) ;
	work->rot.vz = ((irnd()>>8) % ROT_LIM ) - (ROT_LIM/2) ;
#else
	rot.vx = 0 ;
//	rot.vy = rot_y;
	rot.vy = (irnd()>>8) & 4095 ;
	rot.vz = 0 ;

	work->rot.vx = 0 ;
//	work->rot.vy = dir ;
	work->rot.vy = ((irnd()>>8) % ROT_LIM ) - (ROT_LIM/2) ;
	work->rot.vz = 0 ;
#endif
	DG_SetPos2( &DG_ZeroVector, &rot ) ;

	rand.vx= rate ;
	rand.vy= 0.0f ;
	rand.vz= 0.0f ;

	DG_PutVector( &rand, &work->vel, 1 ) ;
	DG_PutVector( &rand, &work->len, 1 ) ;

	GV_MatToVec( world, &work->c_pos ) ;

	work->color.r = col->r ;
	work->color.g = col->g ;
	work->color.b = col->b ;


	SetAmb(work,0,0,0);


	def = (DG_DEF*) GV_GetCache( GV_CacheID( mdl, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE|DG_FLAG_NOFOG, DG_CHANL_MAIN ) ;
	ASSERT(work->objs != NULL ) ;
	DG_SetLightMatrix( work->objs, work->lights );

	DG_QueueObjs( work->objs ) ;
	DG_SetPos( world ) ;
	DG_PutObjs( work->objs );
	work->cnt = DIRECT_TICK(time) ;
	work->limit = DIRECT_TICK(time) ;

	work->world = world ;

	return 1;
}

void *NewSigRgbRandObj(int mdl,FMATRIX *world,float rate,int time ,	CVECTOR *col,int mode,int dir,int rot)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_EFFECT, GV_CLASS_EFFECT,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->mode = mode ;
		
		if(!GetResources(work,mdl,world,rate,time,col,dir,rot)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

