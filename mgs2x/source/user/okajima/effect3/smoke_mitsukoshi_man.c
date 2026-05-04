//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	smoke_mitsukoshi_man.c
	三越ケムリ
	2001/08/22 S.Okajima
	$Id: smoke_mitsukoshi_man.c,v 1.1.1.3 2002/11/19 11:47:36 Yoshizawa1 Exp $

*/

#ifdef PSX2 ///
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
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR center;
	float	size;
	float	wind_sensivility;
	int		col;

	int		count;

} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	void *pw;

	if( work->count%16==0 ){
		extern void *NewSmokeMitsukoshi( FVECTOR *center, float height, int col, int life, float wind_sensivility );
		pw = NewSmokeMitsukoshi(
			&work->center,
			work->size,
			work->col,
			160,
			work->wind_sensivility );
		if(pw!=NULL) GV_SetActorChild(work,pw);
	}
	work->count++;
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	work->count = 0;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewSmokeMitsukoshiMan( FVECTOR *center, float size, int col, float wind_sensivility )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, center );
		work->center.vw = 1.0f;
		work->size = size;
		work->col  = col;
		work->wind_sensivility = wind_sensivility;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}


