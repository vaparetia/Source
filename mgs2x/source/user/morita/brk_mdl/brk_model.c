//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_spotlgt.c 
   プットライトオブジェ

   1999/12/26 T.Morita
   $Id: brk_model.c,v 1.1.1.3 2002/11/19 11:45:34 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

enum
{
    BRK_MDL_F_TARGETSHOW  = 0x00000001,
    BRK_MDL_F_BROKEN      = 0x40000000,
    BRK_MDL_F_PUT_C4      = 0x80000000,
} ;

enum
{
    BRK_MDL_M_BREAKDOWN,
    BRK_MDL_M_INVISBLE ,
    BRK_MDL_M_VISIBLE  ,
    BRK_MDL_M_CHANGFLAG,
} ;


typedef struct work_t
{
    GV_ACT       actor  ;

    DG_DEF      *model  ;
    DG_DEF      *broken ;
    DG_OBJS     *objs   ;

    int          where   ;
    int          name    ;
    int          flag    ;
    int          pr_name ;
    GCL_ARGS     pr_arg  ; /* プロック引数*/

    FVECTOR      trgt   ;
    float        radius ;

    LIT_DEF     *lit    ;

    TARGET       trg_bllt ;
    POWER_TARGET pow_bllt ;
    TARGET       trg_bomb ;
    POWER_TARGET pow_bomb ;
} Work ;


static int ChangeModel( Work *work, DG_DEF *model, FMATRIX *mtx )
{
    DG_OBJS *objs ;

    if ( (objs = DG_MakeObjs( model, 
			      DG_FLAG_ONEPIECE| DG_FLAG_PAINT| DG_FLAG_AUTOREPAINT, 0 )) )
    {
	if ( mtx ) {
	    _sceVu0CopyMatrix( &objs->world, mtx ) ;
	} else if ( work->objs ) {
	    _sceVu0CopyMatrix( &objs->world, &work->objs->world ) ;
	    objs->flag  = work->objs->flag  ;/* 天井君によって返られるかも知れないから */
	    DG_DequeueObjs( work->objs ) ;
	    DG_FreeObjs( work->objs ) ;
	}
	work->objs = objs ;
	DG_QueueObjs( work->objs ) ;
	DG_MakePreshade( work->objs, work->lit ) ;
	GM_GroupObjs( work->objs, work->where ) ;
	return 0 ;
    }
    return -1 ;
}

static void BRK_SPLGT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work      *work = (Work*)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if( (&work->trg_bllt == def && (off->weapon_type & (WP_BULLET|WP_M92)) ) ||
	    (&work->trg_bomb == def && (off->weapon_type & WP_BLAST))  )
	    if ( !ChangeModel( work, work->broken, NULL ) )
	    {
		work->flag |= BRK_MDL_F_BROKEN ;

		if ( work->pr_name )
		    GCL_ExecProc( work->pr_name, &work->pr_arg ) ;
	    }
    }
}

static inline int BRK_SPLGT_InitTarget( Work *work, TARGET *t, POWER_TARGET *p,
					int flag, int where, float size ) 
{
    FVECTOR t_size = { size, size, size, 0 } ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|flag, where, BOTH_SIDE, &t_size, &work->trgt ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_SPLGT_TargetCallBack, work ) ;

#ifdef DEBUG_MODE
    if ( work->flag & BRK_MDL_F_TARGETSHOW )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    return 0 ;
}


static void Die( Work *work )
{
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    if ( !(work->flag & BRK_MDL_F_BROKEN) )
    {
	GM_FreeTarget( &work->trg_bllt ) ;
	if ( work->flag & BRK_MDL_F_PUT_C4 )
	    GM_FreeTarget( &work->trg_bomb ) ;
    }
}

static void Act( Work *work )
{
    GV_MSG  *msg  ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch ( msg->message[0] )
	{
	case BRK_MDL_M_BREAKDOWN:
	    if ( !(work->flag & BRK_MDL_F_BROKEN) )
		ChangeModel( work, work->broken, NULL ) ;
	    break ;

        case BRK_MDL_M_INVISBLE:
	    DG_InvisibleObjs( work->objs ) ;
            break ;
        case BRK_MDL_M_VISIBLE:
	    DG_VisibleObjs( work->objs ) ;
            break ;

        case BRK_MDL_M_CHANGFLAG:
	    work->objs->flag &= ~DG_FLAG_INVISIBLE ;
	    work->objs->flag |=  msg->message[1] ;
            break ;
	}
}

static int GetResources( Work *work, int name, int where )
{
    IVECTOR buf ;
    FMATRIX mtx ;

    work->name    = name  ;
    work->where   = where ;
    work->flag    = GCL_GetOptionValue( 'f', 0 ) ;
    work->pr_name = GCL_GetOptionValue( 'b', 0 ) ;
    work->pr_arg.argc = 1 ;
    work->pr_arg.argv = &work->name ;

    /* ライトを設定 */
    work->lit = GM_GetMap( where )->light ;
    if ( GCL_GetOption( 'l' ) )
	if ( !(work->lit = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'l' ))) )
	    PERROR( "Cannot find Light data (not in data.cnf) :: NewPutSpotLightObject\n" ) ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No model name(-m option missing) :: NewPutSpotLightObject\n" ) ;
    if ( !(work->model  = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k') )) )
	PERROR( "No normal model in data.cnf(%d) :: NewPutSpotLightObject\n", work->model  ) ;
    if ( !(work->broken = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k') )) )
	PERROR( "No broken model in data.cnf(%d) :: NewPutSpotLightObject\n", work->broken ) ;

    if ( !GCL_GetOption( 't' ) )
	PERROR( "No target size(-t option missing) :: NewPutSpotLightObject\n" ) ;
    GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
    vu0_IV0toFV( &buf, &work->trgt ) ;
    if ( GCL_NextStr() )
	BRK_SPLGT_InitTarget( work, &work->trg_bllt, &work->pow_bllt,
			      0, where, (float)GCL_GetNextInt() ) ;
    work->radius = !GCL_NextStr() ? 1000.0f : (float)GCL_GetNextInt() ;
    if ( GCL_NextStr() )
    {
	BRK_SPLGT_InitTarget( work, &work->trg_bomb, &work->pow_bomb,
			      TARGET_THROUGH, where, (float)GCL_GetNextInt() ) ;
	work->flag |= BRK_MDL_F_PUT_C4 ;
    }

    _sceVu0CopyMatrix( &mtx, &DG_UnitMatrix ) ;
    if ( GCL_GetOption( 'r' ) )
    {
	int r ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixX( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixY( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	r = GCL_GetNextInt() & 0x0fff ;
	_sceVu0RotMatrixZ( &mtx, &mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    }
    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	vu0_IV0toFV( &buf, (FVECTOR *)mtx.m[W] ) ;
	mtx.m[W][W] = 1.0f ;
    }

    /* モデルを初期化する */
    ChangeModel( work, work->model, &mtx ) ;

    return 0 ;
}

void *NewModelChangeObject( int name, int where )
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
