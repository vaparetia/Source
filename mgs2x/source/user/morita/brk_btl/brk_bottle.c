//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_bottle.c
   瓶壊れ

   1999/12/02 T. Morita
   $Id: brk_bottle.c,v 1.1.1.3 2002/11/19 11:45:23 Yoshizawa1 Exp $
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

#include "brk_bottle.h"


void BRK_BTL_FreeBottle( BOTTLE *b )
{

#if MAKING
    if ( b->wireframe )
    {
	GV_DestroyOtherActor( b->wireframe ) ;
	b->wireframe = NULL ;
    }

    if ( b->target )
    {
	int i ;

	for( i=b->objs->n_models ; --i>=0 ; )
	    if ( b->parts[i].wireframe )
	    {
		GV_DestroyOtherActor( b->parts[i].wireframe ) ;
		b->parts[i].wireframe = NULL ;
	    }
    }
#endif

    if ( b->target )
    {
	GM_FreeTarget( b->target ) ;
	GV_Free( b->target ) ;
	b->target = NULL ;
    }
    if ( b->objs )
    {
	DG_DequeueObjs( b->objs ) ;
	DG_FreeObjs( b->objs ) ;
	DG_FreePreshade( b->objs ) ;
	b->objs = NULL ;
    }
}

static void Die( Work *work )
{
    int     i ;
    BOTTLE *b = work->bottles ;

    work->n_bottles &= ~BRK_BOTTLE_INACTIVE ;
    for ( i=work->n_bottles ; --i>=0 ; b++ )
	BRK_BTL_FreeBottle( b ) ;
    if ( work->bottles )
	GV_Free( work->bottles ) ;
    if ( work->piece_l )
	DG_DequeueComdlObjs( work->piece_l ), DG_FreeComdl( work->piece_l ) ;
    if ( work->piece_s )
	DG_DequeueComdlObjs( work->piece_s ), DG_FreeComdl( work->piece_s ) ;
    if ( work->hzd )
	GV_Free( work->hzd ) ;
    if ( work->ene_find.type & (EF_TYPE_USING|EF_TYPE_FOUND) )
	  GM_FreeEneFind( &work->ene_find ) ;

}

static void Act( Work *work )
{
    int     i, flag ;
    BOTTLE *b ;

	/* コールドスプレーを装備して噴射したらターゲットを効かなくする */
	if ( GM_Weapon == WP_ColdSpray ){
		i = work->n_bottles & ~BRK_BOTTLE_INACTIVE ;
		b = work->bottles ;
		while( --i>=0 ){
			if ( b->target ) {
				if ( GM_WeaponFire == WP_ColdSpray ){
					b->target->class |= TARGET_SKIP ;
				} else {
					b->target->class &= ~TARGET_SKIP ;
				}
			}
			b++ ;
		}
	}

	/* メッセージ処理 */
    BRK_BTL_ReceiveMessage( work ) ;

	/* 本体処理 */
    if ( !(work->n_bottles & BRK_BOTTLE_INACTIVE) )
    {
	flag = 0 ;
	for( b=work->bottles, i=work->n_bottles ; --i>=0 ; b++ )
	    if ( b->act && b->act != BRK_BTL_ActFinished )
		flag |= (*b->act)( work, b ) ;
	if ( !flag )
	    work->n_bottles |= BRK_BOTTLE_INACTIVE ;
    }

	/* ごみカス処理 */
    if ( work->n_piece & BRK_PIECE_ACTIVE )
	BRK_BTL_ActPieces( work ) ;
}

static int GetResources( Work *work, int name, int where )
{
    work->hzx   = GM_GetHzxGroupID( where ) ;
    work->name  = name  ;
    work->where = where ;

    BRK_BTL_InitBottleType() ;
    if ( BRK_BTL_GetOptions( work, where ) )
	return -1 ;
    if ( BRK_BTL_InitPieces( work, where ) )
        return -1 ;
    if ( BRK_BTL_InitHazard( work, where ) )
	return -1 ;

    BRK_BTL_InitBroken( work ) ;
	BRK_BTL_InitEneFind( work ) ;

    return 0 ;
}

void *NewPutBottleObject( int name, int where )
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
