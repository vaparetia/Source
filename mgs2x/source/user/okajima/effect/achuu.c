//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	achuu.c
	くしゃみ飛沫（呼び出し部）
	2000/03/31 S.Okajima
	$Id: achuu.c,v 1.1.1.3 2002/11/19 11:46:57 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"


/*----------------------------------------------------------------*/
#define	LIFE_TIME		(15)

#define	SHIFT0_X		(0.0f)
#define	SHIFT0_Y		(-30.0f)
#define	SHIFT0_Z		(80.0f)
#define	ROTATION0_X		(0)
#define	ROTATION0_Y		(0)
#define	ROTATION0_Z		(0)

#define	SHIFT1_X		(10.0f)
#define	SHIFT1_Y		(-10.0f)
#define	SHIFT1_Z		(110.0f)
#define	ROTATION1_X		(1024-128)
#define	ROTATION1_Y		( 128)
#define	ROTATION1_Z		(0)

#define	SHIFT2_X		(-10.0f)
#define	SHIFT2_Y		(-10.0f)
#define	SHIFT2_Z		(110.0f)
#define	ROTATION2_X		(1024-128)
#define	ROTATION2_Y		(-128)
#define	ROTATION2_Z		(0)

extern void *New_ACHUU_Parts( FMATRIX *world, int mode );

typedef	struct	{
	GV_ACT_EX		actor ;
	FMATRIX		*world;
	int			life;
} Work ;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FMATRIX	mat;
	FVECTOR	shift;
	SVECTOR	rot;

	if( work->life > 0 ){

		/* 口 */
		shift.vx = SHIFT0_X;
		shift.vy = SHIFT0_Y;
		shift.vz = SHIFT0_Z;
		rot.vx = ROTATION0_X;
		rot.vy = ROTATION0_Y;
		rot.vz = ROTATION0_Z;
		DG_SetPos( work->world );
		DG_MovePos( &shift );
		DG_RotatePos( &rot );
		DG_GetPos( &mat );
		New_ACHUU_Parts( &mat, 0 );

		/* 鼻孔（左） */
		shift.vx = SHIFT1_X;
		shift.vy = SHIFT1_Y;
		shift.vz = SHIFT1_Z;
		rot.vx = ROTATION1_X;
		rot.vy = ROTATION1_Y;
		rot.vz = ROTATION1_Z;
		DG_SetPos( work->world );
		DG_MovePos( &shift );
		DG_RotatePos( &rot );
		DG_GetPos( &mat );
		New_ACHUU_Parts( &mat, 1 );

		/* 鼻孔（右） */
		shift.vx = SHIFT2_X;
		shift.vy = SHIFT2_Y;
		shift.vz = SHIFT2_Z;
		rot.vx = ROTATION2_X;
		rot.vy = ROTATION2_Y;
		rot.vz = ROTATION2_Z;
		DG_SetPos( work->world );
		DG_MovePos( &shift );
		DG_RotatePos( &rot );
		DG_GetPos( &mat );
		New_ACHUU_Parts( &mat, 1 );

		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
}


static int GetResources( Work *work, FMATRIX *world )
{
	if( world==NULL ) return -1;
	work->world = world;
	work->life = LIFE_TIME;

	return 0 ;
}

void *New_ACHUU( FMATRIX *world )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
