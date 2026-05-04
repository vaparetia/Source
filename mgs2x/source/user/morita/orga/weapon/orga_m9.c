/*
   orga_m9.c 
   オルガ 武器関数(USP)

   1999/12/18 T.Morita
   $Id: orga_m9.c,v 1.1.1.3 2002/11/19 11:46:28 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libutl.h"
#include "gameheader.h"

#include "orga_wep.h"
#include "orga_se.h"

typedef	struct work_usp_t Work ;
struct work_usp_t
{
    GV_ACT	 actor ;
    DG_OBJS     *objs  ;
    FMATRIX     *world ;

    FVECTOR    *trigger ;
    short      *bullet  ;
} ;



#define ORGA_USP_FRAMES  7
#define ORGA_USP_SCALE   7

static void Die( Work *work )
{
    if ( work->weapon )
	DG_DequeueObjs( work->weapon ), DG_FreeObjs( work->weapon ) ;
}

static void Act( Work *work )
{
    extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
    extern void  AN_MazzleUSP( FMATRIX *, int ) ;
    extern void  AN_CartridgeUSP_E( FMATRIX *, OBJECT *, CONTROL * ) ;
    int se ;

    work->weapon->world         = work->weapon->objs[0].world =
    work->weapon->objs[1].world = work->weapon->objs[2].world = *work->world ;
    if ( work->trigger->vw == -1.0f )
	GV_DestroyActor( work ) ;
    else if ( work->trigger->vw != 0.0f )
    {
	if ( work->trigger->vw == 1.0f )
	{
	    if ( --(*work->bullet) > 0 )
	    {
		FMATRIX dir = DG_UnitMatrix ;
		_sceVu0CopyVector( (FVECTOR*)&dir.m[W], (FVECTOR*)&work->world->m[W] ) ;
		_sceVu0SubVector( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[W], work->trigger ) ;
		_sceVu0Normalize( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[Y] ) ;
		NewBullet( &dir, BUL_TYPE_SPARK|BUL_TYPE_SCAR, PLAYER_SIDE,
			   25, 5, 20000, 1000, WP_Usp ) ;
		AN_MazzleUSP( work->world, 0 ) ;
		se = ORGA_SE_USP_FIRE ;
	    }
	    else
		se = ORGA_SE_USP_FIRE_E ;
	    GM_SeSetMode( se, (FVECTOR *)&work->world->m[3], GM_SEMODE_NORMAL ) ; 
	}

	/* USP Blow Back */
	ScaleAndAddVector( (FVECTOR*)&work->weapon->objs[2].world.m[W],
			   (FVECTOR*)&work->weapon->objs[2].world.m[Y],
			   (ORGA_USP_FRAMES - work->trigger->vw) * ORGA_USP_SCALE ) ;

	if ( (work->trigger->vw += 1.0f) > ORGA_USP_FRAMES )
	    work->trigger->vw = 0.0f ;
    }
}

static int GetResources( Work *work, FMATRIX *world, FVECTOR *trigger, short *bullet )
{
    int      i ;
    DG_OBJ  *o ;
    DG_MDL  *m ;
    DG_DEF  *def = GV_GetCache( GV_CacheID( GV_StrCode( "usp" ), 'k' ) ) ;

    DG_QueueObjs( work->weapon = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 ) ) ;
    work->world   = world   ;
    work->trigger = trigger ;
    work->bullet  = bullet  ;

    o = work->weapon->objs ;
    m = def->models ;
    for ( i=def->n_models ; --i>=0 ; o++, m++ )
	AddVector( (FVECTOR*)&o->world.m[3], (FVECTOR*)&work->world->m[3], (FVECTOR*)&m->tx ) ;
    return 0 ;
}

void *NewOrgaWeaponUSP( FMATRIX *world, FVECTOR *trigger, short *bullet )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, world, trigger, bullet ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
