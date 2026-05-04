//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_floor.c
   揺れ落下

   1999/12/13 T. Morita
   $Id: brk_flr_act.c,v 1.1.1.3 2002/11/19 11:45:47 Yoshizawa1 Exp $
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

#include "brk_floor.h"


// for DG-LIB
#include "../include/libdg_x.h"
#include "../brk_utl/brk_utl.x"

extern void *SetSplushSequence2( FVECTOR *pos ) ;

/*

  汎用関数群

 */
void BRK_FLR_MakeObjWorld( Work *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0CopyMatrix( mtx, &DG_UnitMatrix ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)mtx->m[W], &work->center ) ;
    _sceVu0RotMatrixZ( mtx, mtx, ANGtoRAD(work->rot_z) ) ;
    _sceVu0RotMatrixX( mtx, mtx, ANGtoRAD(work->rot_x) ) ;
    _sceVu0RotMatrixY( mtx, mtx, ANGtoRAD(work->rot_y) ) ;
    _sceVu0AddVector( (FVECTOR *)mtx->m[W], (FVECTOR *)mtx->m[W], &work->pos ) ;
    mtx->m[W][W] = 1.0f ;
    //GM_MoveTarget2Map( &work->target, mtx, work->where ) ;
}

void BRK_FLR_ChangeCenter( Work *work, FVECTOR *center )
{
    FVECTOR v ;

    _sceVu0SubVector( &v, &work->center, center ) ;
    v.vw = 0.0f ;
    _sceVu0ApplyMatrix( &v, &work->objs->world, &v ) ;
    _sceVu0AddVector( &work->pos, &work->pos, &v ) ;
    _sceVu0CopyVectorXYZ( &work->center, center ) ;
}

void BRK_FLR_PanelBottom( Work *work, float rate )
{
    FVECTOR size ;

    BRK_UTL_SizeOfBound( work->objs, &work->objs->world, &size ) ;
    rate = rate<0.0f ? 0.0f : rate>1.0f ? 1.0f : rate ;
    work->objs->world.m[W][Y] += size.vy * rate ;
}

static inline int RotSpeed( int i, int r )
{
    for ( ; r/2 && --i>=0 ; r/=2 ) ;
    return r ;
}

static inline void LayDown( short *rot, short *rot_v, int axis )
{
    short r ;

#if 0
    *rot_v += (*rot/DEGtoANG(90) ? DEGtoANG(1) : -DEGtoANG(1)) * (*rot>=0 ? 1 : -1) ;
    r = *rot + *rot_v ;
    if ( (r<0 && *rot>=0) || (r>=0 && *rot<0) )
    {
	r = *rot/DEGtoANG(90) ? DEGtoANG(180) : DEGtoANG(0) ;
	*rot_v = 0 ;
    }
    *rot = r ;

#else
    if ( axis )
	*rot_v += (*rot<0 ? -DEGtoANG(1) : DEGtoANG(1)) ;
    else
	*rot_v += (*rot>0 ? -DEGtoANG(1) : DEGtoANG(1)) ;
    r = *rot + *rot_v ;
    if ( (r <0 && *rot>=0) || (r>=0 && *rot< 0) )
    {
	r = axis ? DEGtoANG(180) :  0 ;
	*rot_v = 0 ;
    }
    *rot = r ;

#endif
}



/*

  それぞれのアクト関数群

 */
static inline void BRK_FLR_ActLaydownStart( Work *work )
{
    work->rot_vx = work->rot_vz = 0 ;
    work->mode = BRK_FLR_ModeLaydown ;
    GM_SetNoise( NOISE_MM, &work->pos, work->where ) ;
}

static inline void BRK_FLR_ActLaydown( Work *work )
{
    /* X,Y,Z の角度を落ち着かせる */
    LayDown( &work->rot_x, &work->rot_vx, 1 ) ;
    LayDown( &work->rot_z, &work->rot_vz, 0 ) ;
    work->rot_y += work->rot_vy = work->rot_vy*15/16 ;

    /* 回転が止まったら終り */
    if ( !work->rot_vx && !work->rot_vy && !work->rot_vz )
	work->mode = BRK_FLR_ModeNone ;

    /* ワールドを計算 */
    BRK_FLR_MakeObjWorld( work ) ;
    BRK_FLR_PanelBottom( work, 1.0f ) ;
}


static inline void BRK_FLR_ActFallStart( Work *work )
{
    if ( !work->value )
    {
	work->rot_vx = (irnd() & 1023) - 512 ;
	work->rot_vy = (irnd() &  511) - 256 ;
	work->rot_vz = (irnd() & 1023) - 512 ;
    }
    work->length = work->pos.vy - work->level ;
    work->mode = BRK_FLR_ModeFall ;

    GM_SeSetMode( SD_A_FLOORTK1, &work->pos, GM_SEMODE_NORMAL ) ;

    BRK_FLR_ChangeCenter( work, &DG_ZeroVector ) ;
}

static inline void BRK_FLR_ActFall( Work *work )
{
    work->rot_x += work->rot_vx ;
    work->rot_y += work->rot_vy ;
    work->rot_z += work->rot_vz ;

    if ( work->pos.vy+work->pos_vy <  GM_WaterLevel &&
	 work->pos.vy              >= GM_WaterLevel )
	SetSplushSequence2( &work->pos ) ;

    work->pos_vy -= BRK_GRAVITY ;
    if ( work->pos_vy > BRK_MAX_SPEED )
	work->pos_vy = BRK_MAX_SPEED ;
    work->pos.vy += work->pos_vy ;

    if ( work->pos.vy < work->level )
    {
	if ( work->level > GM_WaterLevel )
	    GM_SeSetMode( SD_A_FLOORFL1, &work->pos, GM_SEMODE_BOMB ) ;
	work->pos.vy = work->level ;
	work->mode = BRK_FLR_ModeLaydownStart ;
    }

    /* ワールドを計算 */
    BRK_FLR_MakeObjWorld( work ) ;
    BRK_FLR_PanelBottom( work, 1.0f - (work->pos.vy - work->level)/work->length ) ;
}


static inline void BRK_FLR_ActShakeStart( Work *work )
{
    work->mode = BRK_FLR_ModeShake ;
    work->pos.vy = work->pos_d.vy - 50.0f ;
    work->rot_vx =  50 * (1 - (irnd()&2)) ;
    work->rot_vz = 100 * (1 - (irnd()&2)) ;

    NewPadVibration2( 14071622/*board_fall.vib*/, 0 ) ;

    GM_SeSetMode( SD_A_FLOORON1, &work->pos, GM_SEMODE_BOMB ) ;
}

static inline void BRK_FLR_ActShake( Work *work )
{
    extern void *NewFortFallDust( FVECTOR *pos, int n_dust,
				  float gravity, int latency ) ;
    extern void *NewFallFragile( FVECTOR *pos, int n_piece, int life,
				 float width, int interval ) ;
    FVECTOR pos, out ;
    float   sx, sz ;

    /* エフェクト */
    if ( --work->value <= 0 )
    {
	work->mode  = BRK_FLR_ModeSwingStart ;
	work->value = work->falltype ;
    }
    else if ( !(work->value & 15) )
    {
	GM_CurrentMap = work->where ;

	/* モデルのサイズ及び中心位置 */
	sx     = (work->objs->def->ux - work->objs->def->lx) * 0.5f ;
	sz     = (work->objs->def->uz - work->objs->def->lz) * 0.5f ;
	pos.vx = (work->objs->def->ux + work->objs->def->lx) * 0.5f ;
	pos.vy = (work->objs->def->uy + work->objs->def->ly) * 0.5f ;
	pos.vz = (work->objs->def->uz + work->objs->def->lz) * 0.5f ;

	/* モデルの端を計算する(右側) */
	out.vx = pos.vx + sx ;
	out.vy = pos.vy      ;
	out.vz = pos.vz + sz * frnd() ;
	out.vw = 1.0f ;
	_sceVu0ApplyMatrix( &out, &work->objs->world, &out ) ;
	/* 埃をだす */
	NewFallFragile( &out, 4, 120, 100.0f, 0 ) ;
	NewFortFallDust( &out, 4, 4.0f, TIME_BASE ) ;

	/* モデルの端を計算する(左側) */
	out.vx = pos.vx - sx ;
	out.vy = pos.vy      ;
	out.vz = pos.vz + sz * frnd() ;
	out.vw = 1.0f ;
	_sceVu0ApplyMatrix( &out, &work->objs->world, &out ) ;
	/* 埃をだす */
	NewFallFragile( &out, 4, 120, 100.0f, 0 ) ;
	NewFortFallDust( &out, 4, 4.0f, TIME_BASE ) ;

	GM_SeSetMode( (work->se_mode ? SD_A_FLOORVB1 : SD_A_FLOORVB2),
		      &work->pos, GM_SEMODE_NORMAL ) ;
    }

    /*各軸の揺れ計算*/
    work->rot_x += work->rot_vx -= RotSpeed( 6, work->rot_vx + work->rot_x - work->rot_dx ) ;
    work->rot_z += work->rot_vz -= RotSpeed( 6, work->rot_vz + work->rot_z - work->rot_dz ) ;

    /* ワールドを計算 */
    BRK_FLR_MakeObjWorld( work ) ;
}



static inline void BRK_FLR_ActSwingStart( Work *work )
{
    FVECTOR center ;
    static short stable[] = {
	DEGtoANG(-45),DEGtoANG(  0),DEGtoANG( 80),/* 0 */
	DEGtoANG(-45),DEGtoANG(  0),DEGtoANG(-80),
	DEGtoANG( 45),DEGtoANG(  0),DEGtoANG( 80),/* 2 */
	DEGtoANG( 45),DEGtoANG(  0),DEGtoANG(-80),
	DEGtoANG(  0),DEGtoANG(  0),DEGtoANG( 90),/* 4 */
	DEGtoANG(-90),DEGtoANG(  0),DEGtoANG(  0),
	DEGtoANG( 90),DEGtoANG(  0),DEGtoANG(  0),/* 6 */
	DEGtoANG(  0),DEGtoANG(  0),DEGtoANG(-90),
    } ;
    work->mode = BRK_FLR_ModeSwing ;
    work->rot_vx = 0 ;
    work->rot_vy = 0 ;
    work->rot_vz = 0 ;

    if ( stable[work->value*XYZ + X] )
	work->rot_dx = stable[work->value*XYZ + X] ;
    if ( stable[work->value*XYZ + Z] )
	work->rot_dz = stable[work->value*XYZ + Z] ;
    if ( stable[work->value*XYZ + Y] )
	work->rot_dy = stable[work->value*XYZ + Y] ;

    center.vx = work->value&1 ? work->objs->def->ux : work->objs->def->lx ;
    center.vy = 0.0f ;
    center.vz = work->value&2 ? work->objs->def->uz : work->objs->def->lz ;
    if ( work->value & 4 )
    {
	if ( ((work->value >> 1)^work->value) & 1 )
	    center.vx = 0.0f ;
	else
	    center.vz = 0.0f ;
    }


    GM_SeSetMode( (work->se_mode ? SD_A_FLOOROU1 : SD_A_FLOOROU2),
		  &work->pos, GM_SEMODE_NORMAL ) ;

    //printf( "%d(%.1f %.1f %.1f)(%d %d %d)\n", work->value, center.vx,center.vy,center.vz, ANGtoDEG(work->rot_dx), ANGtoDEG(work->rot_dy), ANGtoDEG(work->rot_dz) );

    BRK_FLR_ChangeCenter( work, &center ) ;

    work->value = 0 ;
}

static inline void BRK_FLR_ActSwing( Work *work )
{
    int prv, cur ;

    prv = work->rot_vx ;
    /*各軸の揺れ計算*/
    work->rot_x += work->rot_vx -= RotSpeed( 6, work->rot_vx + work->rot_x - work->rot_dx ) ;
    work->rot_y += work->rot_vy -= RotSpeed( 6, work->rot_vy + work->rot_y - work->rot_dy ) ;
    work->rot_z += work->rot_vz -= RotSpeed( 6, work->rot_vz + work->rot_z - work->rot_dz ) ;
    cur = work->rot_vx ;

    /*揺れ終り*/
    if ( cur * prv <= 0 )
	if ( ++work->value > 3 )
	    work->mode = BRK_FLR_ModeFallStart ;
#if 0
    if ( !(work->rot_vx/4) && !(work->rot_vy/4) && !(work->rot_vz/4) )
	work->mode = BRK_FLR_ModeFallStart ;
#endif

    /* ワールドを計算 */
    BRK_FLR_MakeObjWorld( work ) ;
}


static inline void BRK_FLR_ActReset( Work *work )
{
    work->mode = BRK_FLR_ModeNone ;
    _sceVu0CopyVector( &work->pos, &work->pos_d ) ;
    work->rot_x = work->rot_dx ;
    work->rot_y = work->rot_dy ;
    work->rot_z = work->rot_dz ;

    /* ワールドを計算 */
    BRK_FLR_MakeObjWorld( work ) ;
}

/*

  メインアクト関数群

 */
void BRK_FLR_Act( Work *work )
{
    switch( work->mode )
    {
    case BRK_FLR_ModeReset:
	BRK_FLR_ActReset( work ) ;

    case BRK_FLR_ModeNone:
	break ;


    case BRK_FLR_ModeShakeStart:
	BRK_FLR_ActShakeStart( work ) ;

    case BRK_FLR_ModeShake:
	BRK_FLR_ActShake( work ) ;
	break ;


    case BRK_FLR_ModeSwingStart:
	BRK_FLR_ActSwingStart( work ) ;

    case BRK_FLR_ModeSwing:
	BRK_FLR_ActSwing( work ) ;
	break ;


    case BRK_FLR_ModeFallStart:
	BRK_FLR_ActFallStart( work ) ;

    case BRK_FLR_ModeFall:
	BRK_FLR_ActFall( work ) ;
	break ;


    case BRK_FLR_ModeLaydownStart:
	BRK_FLR_ActLaydownStart( work ) ;

    case BRK_FLR_ModeLaydown:
	BRK_FLR_ActLaydown( work ) ;
	break ;
    }
}
