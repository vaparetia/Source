//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_object.c
   一般 物壊れ

   2000/02/15 T. Morita
   $Id: brk_object.c,v 1.1.1.3 2002/11/19 11:45:38 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_object.h"


int BRK_OBJ_NoDropSound = 0 ;
int BRK_OBJ_HowManyActs = 0 ;


static void BRK_OBJ_FreeBox( Work *work, BOX *b )
{
    if ( b )
    {
	if ( b->hzd )
	    BRK_FreeHazard( b->hzd ) ;
	if ( b->objs )
	{
	    if ( !(work->flag & BRK_F_NO_BOXSHOW) )
		DG_DequeueObjs( b->objs ) ;
	    DG_FreeObjs( b->objs ) ;
	}
	GM_FreeTarget( &b->target ) ;
	GV_Free( b ) ;
    }
}

void BRK_OBJ_FreeObj( BRK_OBJ *o )
{
    if ( o->n_parts > 0 )
	GV_Free( o->parts.prof ) ;
    else
	GM_FreeTarget( &o->target ) ;
    if ( o->objs )
    {
	if ( o->objs->flag & DG_FLAG_PAINT )
	    DG_FreePreshade( o->objs ) ;
	DG_DequeueObjs( o->objs ) ;
	DG_FreeObjs( o->objs ) ;
	o->objs = NULL ;
    }
    o->n_parts = 0 ;
}
static void BRK_OBJ_FreeObjs( Work *work, BRK_OBJ *obj, int i, int n_foot )
{
    BRK_OBJ *o ;

    if ( obj )
    {
	i = (i & ~BRK_OBJ_INACTIVE) - n_foot ;
	for ( o=obj ; --i>=0 ; o++ )
	    BRK_OBJ_FreeObj( o ) ;
	GV_Free( obj ) ;
    }
}

static void Die( Work *work )
{
    /* 箱のメモリー解放 */
    BRK_OBJ_FreeBox( work, work->box ) ;
    /* オブジェのメモリー解放 */
    BRK_OBJ_FreeObjs( work, work->obj, work->n_obj, work->n_foot ) ;

    BRK_OBJ_HowManyActs = 0 ;/* 次のために初期化 */
    BRK_OBJ_NoDropSound = 0 ;

    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	GM_FreeEneFind( &work->ene_find ) ;

    /* コモデルのメモリー解放 */
    if ( work->comdl[0] )
        DG_DequeueComdlObjs( work->comdl[0] ), DG_FreeComdl( work->comdl[0] ) ;
    if ( work->comdl[1] )
        DG_DequeueComdlObjs( work->comdl[1] ), DG_FreeComdl( work->comdl[1] ) ;
}

static void Act( Work *work )
{
    BRK_OBJ_ActSound( work ) ;
    BRK_OBJ_ReceiveMessage( work ) ;
    if ( work->counter > 0 )
    {
	work->counter-- ;
	if ( !(work->n_obj & BRK_OBJ_INACTIVE) )
	    BRK_OBJ_ActObject( work ) ;
	if ( !(work->n_piece & BRK_OBJ_INACTIVE) )
	    BRK_OBJ_ActPiece( work, work->piece, BRK_N_PIECE ) ;
    }
}

static int GetResources( Work *work, int name, int where )
{
    work->name  = name  ;
    work->where = where ;
    if ( BRK_OBJ_InitHazard( work, where ) < 0 )
	return -1 ;
    if ( BRK_OBJ_GetOptions( work, where ) < 0 )
	return -1 ;

    BRK_OBJ_InitEneFind( work ) ;
    BRK_OBJ_InitBroken( work ) ;
    BRK_OBJ_HowManyActs++ ;

    return 0 ;
}


void *NewPutBreakObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
