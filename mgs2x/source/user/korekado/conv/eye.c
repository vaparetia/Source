//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	eye.c
	視界
	
	2000/5/16 Y.Korekado
	$Id: eye.c,v 1.1.1.3 2002/11/19 11:44:03 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"define.h"
#define 	TARGET_ON (1)

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	FMATRIX		world ;

	FVECTOR pos ;
	SVECTOR rot ;
	int length ;
	int range ;
	OBJECT	*body ;
	int findbody ;
	int *flag ;
} Work ;


/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */

/* flag */
#define FLAG_EYECHECK	0x80000000
#define FLAG_EYE_IN		0x00000001
/*----------------------------------------------------------------*/
static	int	EyeCheck( work, bodynum )
Work		*work ;
int			bodynum ;
{
	FVECTOR	pos, vec ;
	int dis, dir, diff ;

	KR_FMatToFvec( &BODYWORLD( work->body, 12 ), &pos ) ;

	_sceVu0SubVector(  &vec, &pos, &work->pos ) ;
	dis = _FVecLen3( &vec ) ;
	if ( dis > work->length ) return 0 ;

	dir = _FVecDir2( &vec ) ;
	if ( _DiffDirAbs( work->rot.vy, dir ) > work->range ) return 0 ;

	dir = _FVecDirX( &vec ) - 1024 ;
	diff = GV_DiffDirS( work->rot.vx, dir ) ;
	if ( diff > work->range || diff < - work->range ) return 0 ;

	return 1 ;
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	UNSET_FLAG( *work->flag, FLAG_EYE_IN ) ;
	if ( *(work->flag) & FLAG_EYECHECK ) {
		if ( EyeCheck( work, 12 ) ) {
			SET_FLAG( *work->flag, FLAG_EYE_IN ) ;
		}
	}
}

static	void	Die( work )
Work		*work ;
{
}

/*----------------------------------------------------------------*/
static	int	GetResources( work,  pos, rot, length, range, body, findbody, flag  )
Work	*work ;
FVECTOR *pos ;
SVECTOR *rot ;
int length ;
int range ;
OBJECT *body ;
int findbody ;
int *flag ;
{
	work->pos = *pos ;
	work->rot = *rot ;
	work->length = length ;
	work->range = range ;
	work->body = body ;
	work->findbody = findbody ;
	work->flag = flag ;

	DG_SetPos2( pos, rot ) ;
	DG_GetPos( &work->world ) ;

//	NewEyeView( &work->world, work->length, work->range, work->range, -work->range ) ;

	return 0 ;
}

void	*NewEye( FVECTOR *pos, SVECTOR *rot, int length, int range, OBJECT *body, int findbody, int *flag )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, pos, rot, length, range, body, findbody, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
