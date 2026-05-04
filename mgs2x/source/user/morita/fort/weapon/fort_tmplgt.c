//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_lnr.c 
   フォーチュン 武器関数(LNRカートリッジ)

   1999/12/18 T.Morita
   $Id: fort_tmplgt.c,v 1.1.1.3 2002/11/19 11:46:19 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
// for DG-LIB
#include "../../include/libdg_x.h"

typedef	struct work_weaplgt_t
{
    GV_ACT  actor   ;
    FVECTOR pos     ;
	    
    float   e_range ;
    float   r_range ;
	    
    int     r,g,b,a ;
    int     step    ;
} Work ;

static void Act( Work *work )
{
    int rgba ;

    work->a -= work->step ;
    if ( work->a > 0 )
    {
	rgba = (work->a << 24) | (work->a << 16) | (work->a << 8) | (work->a << 0) ;
	DG_SetTmpLight2( &work->pos, work->r_range, work->e_range, rgba, LIT_FLAG_BGONLY ) ;
	DG_SetTmpLight2( &work->pos, work->r_range, work->e_range, rgba, LIT_FLAG_CHARAONLY ) ;
    }
    else
	GV_DestroyActor( work ) ;
}

void *NewFortWeaponLight( FVECTOR *pos, float r_range, float e_range,
			  int r,int g,int b,int a, int step )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;

	_sceVu0CopyVector( &work->pos, pos ) ;

	work->r = r ;
	work->g = g ;
	work->b = b ;
	work->a = a ;
	work->step = step ;

	work->e_range = e_range ;
	work->r_range = r_range ;
    }
    return work ;
}
