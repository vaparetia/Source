//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	delay_tmplight.c
	水飛沫発生点
	ストリップで曲面形成

	2001/04/09 S.Okajima
	$Id: delay_tmplight.c,v 1.1.1.3 2002/11/19 11:47:19 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		pos;
	CVECTOR		col;

	int			life;
	int			life_max;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	CVECTOR		col;
	int			life_half;

	life_half = work->life_max/2;
	if( work->life > life_half ){
		col.r = work->col.r + (255 - work->col.r) * (work->life - life_half) / life_half;
		col.g = work->col.g + (255 - work->col.g) * (work->life - life_half) / life_half;
		col.b = work->col.b + (255 - work->col.b) * (work->life - life_half) / life_half;
	}else{
		col.r = work->col.r * work->life / life_half;
		col.g = work->col.g * work->life / life_half;
		col.b = work->col.b * work->life / life_half;
	}

//printf("%d %d %d\n",col.r,col.g,col.b);

	DG_SetTmpLight2(
		&work->pos,
		2000.0f,
		4000.0f,
		col.r | col.g<<8 | col.b<<16,
		LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;

	if( work->life >= 0 ){
		work->life--;
		if( work->life <= 0 ) GV_DestroyActor( work ) ;
	}
}


/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}


static int GetResources( Work *work )
{
	return 0 ;
}

/* ---------------------------------------------------------------- */
/*
・全てずっと参照する！！
・world の 位置・方向（Ｚ方向）に width の 厚さで intense の強さの水が出る
*/
void *NewDelayTmpLight( FVECTOR *pos, CVECTOR col, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		DG_COPY_VEC( &work->pos, pos );
		work->col.r = col.r;
		work->col.g = col.g;
		work->col.b = col.b;
		work->life = work->life_max = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

