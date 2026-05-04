//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_inter_poly.c
	毎フレーム更新される座標を補完しつつ繋ぐ：グレネード用
	2001/01/17 S.Okajima
	$Id: d_inter_poly_grenade.c,v 1.1.1.3 2002/11/19 11:46:53 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	SCR_POS		(SCRPAD_ADDR)
#define N_VERTS	(64)
#define N_POLYS	(N_VERTS/4)
#define	N_PRIMS	(16)

#define N_INTER	(N_POLYS)
#define N_INTER_DIV	(1.0f/(float)N_INTER)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	SVECTOR		col;
	FVECTOR		*center;
	FVECTOR		before;
	FVECTOR		pos[2];
	int			rot_z;
	float		width;
} Work ;

/* ---------------------------------------------------------------- */
static void CalcEdge( Work *work )
{
	SVECTOR	rot;

	OK_DirVecXY( &work->before, work->center, &rot );
	work->rot_z += 32;
	rot.vz = (work->rot_z&4095);

	work->pos[0].vx = work->width;
	work->pos[0].vy = 0;
	work->pos[0].vz = 0;
	work->pos[1].vx =-work->width;
	work->pos[1].vy = 0;
	work->pos[1].vz = 0;

	DG_SetPos2( work->center, &rot );
	DG_PutVector( work->pos, work->pos, 2 );

}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	CalcEdge( work );
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	DG_COPY_VEC( &work->before, work->center );

	CalcEdge( work );

	{
		void	*p0;
		extern void *NewInterPoly_Demo( FVECTOR *pos0, FVECTOR *pos1, int disp_f, SVECTOR *col );
		p0 = NewInterPoly_Demo( &work->pos[0], &work->pos[1], 16, &work->col );
		if(p0)GV_SetActorChild( work, p0 );
	}

	return (0);
}

/* ---------------------------------------------------------------- */
/*
col.r :Ｒ
col.g :Ｇ
col.b :Ｂ
col.cd:α
*/

void *NewInterPoly_Gr( FVECTOR *center, float width, CVECTOR col )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->center = center;
		work->width  = width;
		work->col.vx  = col.r;
		work->col.vy  = col.g;
		work->col.vz  = col.b;
		work->col.pad = col.cd;


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

