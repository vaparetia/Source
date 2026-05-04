//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_floor.c
   揺れ落下

   1999/12/13 T. Morita
   $Id: brk_floor.c,v 1.1.1.3 2002/11/19 11:45:46 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "../include/libdg_x.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_floor.h"


static void Die( Work *work )
{
    if ( work->objs )
    {
	work->objs->flag |=  DG_FLAG_PAINT ;
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
    if ( work->thermal )
    {
	DG_DequeueObjs( work->thermal ) ;
	DG_FreeObjs( work->thermal ) ;
    }
    //GM_FreeTarget( &work->target ) ;
}

static void Act( Work *work )
{
    if ( PL_GetPlayerItem() == IT_Thermal )
    {
	DG_InvisibleObjs( work->objs ) ;
	if ( work->thermal )
	    DG_VisibleObjs( work->thermal ) ;
    }
    else
    {
	DG_VisibleObjs( work->objs ) ;
	if ( work->thermal )
	    DG_InvisibleObjs( work->thermal ) ;
    }
    if ( work->thermal )
	_sceVu0CopyMatrix( &work->thermal->world, &work->objs->world ) ;

    if ( work->mode )
	BRK_FLR_Act( work ) ;
}

static int GetResources( Work *work,
			 int model, int thermal,
			 FVECTOR *pos, SVECTOR *rot )
{
    DG_DEF  *def ;

    _sceVu0CopyVector( &work->offset, &DG_ZeroVector ) ;
    _sceVu0CopyVector( &work->center, &DG_ZeroVector ) ;
    _sceVu0CopyVector( &work->pos   , pos ) ;
    _sceVu0CopyVector( &work->pos_d , pos ) ;

    work->rot_x = work->rot_dx = rot->vx*16 ; /* 入力は4096ラジアン,ここは65536ラジアン */
    work->rot_y = work->rot_dy = rot->vy*16 ;
    work->rot_z = work->rot_dz = rot->vz*16 ;
    work->rot_vx = 0 ;
    work->rot_vy = 0 ;
    work->rot_vz = 0 ;

    work->mode   = 0 ;
    work->value  = 0 ;
    work->where = GM_CurrentMap ;
    work->falltype = irnd() & 7 ;

    
    if ( !(def = GV_GetCache( GV_CacheID( model, 'k' ) )) )
	PERROR( "Cannot find KMS-Model<%d>( Not in data.cnf ) :: NewFallingFloor\n", model ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewFallingFloor\n" ) ;
    DG_QueueObjs( work->objs ) ;
    GM_GroupObjs( work->objs, work->where ) ;
    //BRK_FLR_InitTarget( work, work->where ) ;
    BRK_FLR_MakeObjWorld( work ) ;
    DG_MakePreshade( work->objs, GM_GetMap( work->where )->light ) ;

    if ( thermal )
    {
	if ( !(def = GV_GetCache( GV_CacheID( model, 'k' ) )) )
	    PERROR( "Cannot find KMS-Thermal<%d>( Not in data.cnf ) :: NewFallingFloor\n", thermal ) ;
	if ( !(work->thermal = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_IRREACTION|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewFallingFloor\n" ) ;
	DG_QueueObjs( work->thermal ) ;
	GM_GroupObjs( work->thermal, work->where ) ;
	DG_InvisibleObjs( work->thermal ) ;
	_sceVu0CopyMatrix( &work->thermal->world, &work->objs->world ) ;
    }

    if ( ( HZX_LevelHazardCheck( HZX_AllMapID, &work->pos, HZX_CHK_F_FLOOR, 0 ) & 1 ) )
	work->level = HZX_GetFloorLevel() ;
    else
	work->level = GM_WORLD_LIMIT_BOTTOM ;


    return 0 ;
}

/* 
   BRK_FLR_ModeReset: 最初の位置に戻り 静止する
   BRK_FLR_ModeShake: valueのフレーム数で BRK_FLR_ModeFall に移る 
       value: フレーム数   このフレーム後に落ちる
   BRK_FLR_ModeFall : 現在の状態(位置,回転)からそのまま落ちる(ハザードを見る)
       value: 落ち方       踏んだ場所を設定して頂く
*/
static int ReceiveSignal( void *pwork, int signal, int value )
{
    Work *work = pwork ;

    //printf( "NewFallingFloor : %x Recieved Signal %d %d mod%d\n", work, signal, value, work->mode ) ;

    switch( signal )
    {
    case BRK_FLR_ModeReset:
    case BRK_FLR_ModeFall :
    case BRK_FLR_ModeFallStart :
    case BRK_FLR_ModeShakeStart:
    case BRK_FLR_ModeSwingStart:
	if ( work->mode >= BRK_FLR_ModeSwingStart )/* 既に落ちているので受け付けない */
	    break ;
	work->mode  = signal ;
	work->value = value  ;
	break ;
    case BRK_FLR_SetSeMode:
	work->se_mode = value ;
	break ;
    case BRK_FLR_SetFallType:
	work->falltype = value ;
	break ;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

void *NewFallingFloor( int model, int thermal,  FVECTOR *pos, SVECTOR *rot )
{
    Work *work ;

    work = (Work *)GV_NewActorPrio( GV_ACTOR_USER, sizeof(Work), 0x10 ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

        if( GetResources( work, model, thermal, pos, rot ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
