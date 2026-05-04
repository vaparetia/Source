//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ironbox.c
   フォーチュン戦 鉄箱壊れ

   2000/12/15 T.Morita
   $Id: brk_slide.c,v 1.1.1.3 2002/11/19 11:46:15 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#define BRK_IRON_GRAVITY 6

typedef struct work_brk_t
{
    GV_ACT_EX   actor  ;

    short       rot  [XYZ] ;
    short       rot_v[XYZ] ;

    float       vel   ;
    HIDE       *hide  ;
} BrkWork ;


static inline void RotateMatrixXY( BrkWork *work )
{
    int r ;
    FMATRIX *mtx = &work->hide->objs->world ;

    r = work->rot[X] & 0x0fff ;
    _sceVu0RotMatrixX( mtx, &DG_UnitMatrix, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = work->rot[Y] & 0x0fff ;
    _sceVu0RotMatrixY( mtx,  mtx          , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}

static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], pos ) ;
}

static void Act( BrkWork *work )
{
    FVECTOR size ;
    FVECTOR *pos = (FVECTOR *)work->hide->objs->world.m[W] ;
    FVECTOR *slide = &work->hide->slide ;
    float x, z, d ;

    x = slide->vx - pos->vx ;
    z = slide->vz - pos->vz ;
    d = x*x + z*z ;
    if ( d > 200.0f*200.0f )
    {
	/*ゆっくり滑る*/
	d = 10.0f / sceVu0Sqrt( d ) ;
	pos->vx += x * d ;
	pos->vz += z * d ;
    }
    else
    {
	/*落下する*/
	BRK_UTL_SizeOfBound( work->hide->objs, &work->hide->objs->world, &size ) ;
	pos->vx += (slide->vx - pos->vx)*0.25f ;
	pos->vz += (slide->vz - pos->vz)*0.25f ;
	pos->vy += work->vel -= BRK_IRON_GRAVITY ;
	if ( pos->vy < slide->vy )
	{
	    pos->vy = slide->vy ;
	    NewShakeCamera2( 0, 64, 8, pos ) ;

	    /*無敵解除*/
	    work->hide->target.class &= ~TARGET_SKIP ;
	    /* 当たり関係を貼り直す */
	    FRT_OBJ_InitTargetHzxHzd( work->hide ) ;

	    GV_DestroyActor( work ) ;
	}
    }
}

static int GetResources( BrkWork *work, HIDE *hide )
{
    work->hide  = hide ;
    work->vel   = 0.0f ;

    /* しばらく無敵（こうしないと2度きてしまうから） */
    hide->target.class |= TARGET_SKIP ;

    /* しばらく当たりを解除 */
    FRT_OBJ_FreeHzxHzd( work->hide, 0 ) ;

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    BrkWork *work = pwork ;

    switch( signal )
    {
    case FRT_OBJ_Destroyed:
	GV_DestroyActor( work ) ;
	break;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

int NewSlideBox( HIDE *hide )
{
    BrkWork *work ;

    if ( (work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) )) )
    {
	GV_SetActor( &work->actor, Act, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;
	if( GetResources( work, hide ) < 0 )
	    GV_DestroyActor( work ) ;
    }
    return 0 ;
}
