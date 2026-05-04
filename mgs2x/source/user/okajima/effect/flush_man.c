//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	flush_man.c
	フラッシュ管理
	2000/03/14 S.Okajima
	$Id: flush_man.c,v 1.1.1.3 2002/11/19 11:47:05 Yoshizawa1 Exp $

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
#include	"../etc/ok_util.h"

// 0:何もしていない
// 1:ＣＬＵＴを書き換え中
extern int ok_flush_status;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			decay ;
	int			effect_count ;
	int			mode;
} Work ;
Work *OK_FLUSH_WORK=NULL;




/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	if( work->effect_count==0 ){
		extern void *NewFadeInOutForce_Demo( int ini_r, int ini_g, int ini_b, int ini_a,
                              int col_r, int col_g, int col_b, int col_a, int time );
		NewFadeInOutForce_Demo( 255, 255, 255, 64,
                                255, 255, 255, 0, 10 );
		ok_flush_status = 0;
	}else if( work->effect_count < work->decay ){
//		ok_flush_status = 1;
		ok_flush_status = 0;
	}else{
		ok_flush_status = 0;
		GV_DestroyActor( work ) ;
	}
	work->effect_count++;

}

static void Die( Work *work )
{
	ok_flush_status = 0;
	OK_FLUSH_WORK = NULL;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, int decay, int mode )
{

	work->effect_count=0;

	if( decay < 4 ) decay = 4;
	work->decay   = decay;

	work->mode = mode;


	return 0 ;
}

void *NewFlush( int decay,    /* ＣＬＵＴを変更するフレーム長 */
                int mode	/* ０：ブラー無し  １：ブラー有り */
                 )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_FLUSH_WORK==NULL ){
		work = OK_FLUSH_WORK = (Work *)GV_NewEffect( GV_ACTOR_PREV, sizeof( Work ) ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )
			if ( GetResources( work, decay, mode ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		return (void *)work ;
	}else{
		return NULL ;
	}
}

