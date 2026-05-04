/*
  rch_act.c
  ゴキブリ 行動関数

  2000/04/23 T. Morita
  $Id: roach_act.c,v 1.1.1.3 2002/11/19 11:46:33 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "roach.h"


/***

  アクト補助関数

  ***/
/* 直線と点の距離 */
static float CheckDestanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}

/* XZ 2次元距離 */
static float RCH_SqLength2D( FVECTOR *a, FVECTOR *b )
{
    float x = a->vx - b->vx ;
    float z = a->vz - b->vz ;

    return x*x + z*z ;
}

/* 3次元距離 */
static float RCH_SqLength3D( FVECTOR *a, FVECTOR *b )
{
    float x = a->vx - b->vx ;
    float y = a->vy - b->vy ;
    float z = a->vz - b->vz ;

    return x*x + y*y + z*z ;
}

/* 移動と向きの調整 */
static void MoveToDirection( Roach *r, int speed )
{
    FVECTOR v ;

    _sceVu0ScaleVector( &v, r->objs->world.m[Z], speed  ) ;
    _sceVu0SubVector( &v, &v, &r->pos_v ) ;
    _sceVu0ScaleVector( &v, &v, 0.5f ) ;
    _sceVu0AddVector( &r->pos_v, &r->pos_v, &v ) ;
}

/* 表示用マトリックス計算 */
static void RCH_DisplayModel( Roach *r )
{
    int  i ;
    FMATRIX lft, rgt ;
    DG_MDL *m = r->objs->def->models ;
    DG_OBJ *o = r->objs->objs ;

    r->rot_x += r->rot_vx ;
    r->rot_y += r->rot_vy ;
    _sceVu0AddVector( &r->pos, &r->pos, &r->pos_v ) ;

    RotateMatrixXY( &r->objs->world, &DG_UnitMatrix, r->rot_x, r->rot_y ) ;
    TransMatrix( &r->objs->world, (float*)&r->pos ) ;

    /* 本体 */
    r->objs->objs[0].world = r->objs->world ;

    /* 羽 */
    rgt = lft = DG_UnitMatrix ;
    if ( r->flag & RCH_F_MOVE_WINGS )/* 羽をバタバタする */
    {
	i = GV_Time & 7, i = i>4 ? 8-i : i ;
	_sceVu0RotMatrixY( &lft, &lft, -i * ((float)M_PI/16.0f) ) ;
	_sceVu0RotMatrixX( &lft, &lft,  i * ((float)M_PI/16.0f) ) ;
	_sceVu0RotMatrixY( &rgt, &rgt,  i * ((float)M_PI/16.0f) ) ;
	_sceVu0RotMatrixX( &rgt, &rgt,  i * ((float)M_PI/16.0f) ) ;
    }
    TransMatrix( &lft, &m[1].tx ) ;
    _sceVu0MulMatrix( &o[1].world, &r->objs->objs[0].world, &lft ) ;
    TransMatrix( &rgt, &m[2].tx ) ;
    _sceVu0MulMatrix( &o[2].world, &r->objs->objs[0].world, &rgt ) ;

    /* 触覚 */
    rgt = lft = DG_UnitMatrix ;
    if ( r->flag & RCH_F_MOVE_PEEIR )/* 触覚を動かす */
    {
	i = GV_Time & 127, i = i>64 ? 128-i : i ;
	_sceVu0RotMatrixY( &lft, &lft, -i * ((float)M_PI/256.0f) ) ;
	_sceVu0RotMatrixY( &rgt, &rgt,  i * ((float)M_PI/256.0f) ) ;
    }
    TransMatrix( &lft, &m[3].tx ) ;
    _sceVu0MulMatrix( &o[3].world, &r->objs->objs[0].world, &lft ) ;
    TransMatrix( &rgt, &m[4].tx ) ;
    _sceVu0MulMatrix( &o[4].world, &r->objs->objs[0].world, &rgt ) ;

    /* 足 */
    rgt = lft = DG_UnitMatrix ;
    TransMatrix( &lft, &m[5].tx ) ;
    _sceVu0MulMatrix( &o[5].world, &r->objs->objs[0].world, &lft ) ;
    TransMatrix( &rgt, &m[6].tx ) ;
    _sceVu0MulMatrix( &o[6].world, &r->objs->objs[0].world, &rgt ) ;

    DG_GetLightMatrix( &r->pos, r->lights ) ;

    GM_MoveTarget2( &r->target, &r->objs->world ) ;

    r->flag &= ~(RCH_F_MOVE_WINGS|RCH_F_MOVE_PEEIR) ;
}

/***

  アクト関数

  ***/
void RCH_ActBrake( Work *work, Roach *r, int id )
{
    _sceVu0ScaleVector( &r->pos_v, &r->pos_v, 0.6f ) ;
    if ( !(int)r->pos_v.vx && !(int)r->pos_v.vy && !(int)r->pos_v.vz )
    {
	r->rot_vy = 0 ;
	r->pos_v.vx = r->pos_v.vy = r->pos_v.vz = 0.0f ;
	r->act = RCH_ActThink ;
    }
    RCH_DisplayModel( r ) ;
}

void RCH_ActHiding( Work *work, Roach *r, int id )
{
    if ( RCH_SqLength2D( &r->pos, &GM_PlayerControl->mov ) > RCH_DANGER_RAD * RCH_DANGER_RAD )
	r->act = RCH_ActThink ;
}

void RCH_ActWander( Work *work, Roach *r, int id )
{
    MoveToDirection( r, RCH_WANDER_SPEED ) ;

    //BRK_CheckHazard( work->hzd, &r->pos, &r->pos_v, &DG_ZeroVector, &DG_ZeroVector ) ;

    if ( (GV_Time & 127) == r->tics )
	r->act = RCH_ActBrake, r->tics = irnd() & 127 ;
    RCH_DisplayModel( r ) ;
}

void RCH_ActRunaway( Work *work, Roach *r, int id )
{
    FVECTOR v, *h = NULL ;
    float min, tmp ;
    int i ;

    MoveToDirection( r, RCH_RUNAWAY_SPEED ) ;

    min = 60000.0f * 60000.0f ;
    for ( i=work->n_h_spot ; --i>=0 ; )
	if ( (tmp = RCH_SqLength3D( &r->pos, &work->h_spot[i] )) < min )
	    min = tmp, h = &work->h_spot[i] ;

#if DEBUG_MODE
    if ( h==NULL )
	PERROR( "TOO Far for me :: NewCockRoach\n" ) ;
#endif
    _sceVu0SubVector( &v, &r->pos_v, h ) ;
    r->rot_vy += ( (short)(2048.0f / M_PI * atan2f( v.vx, v.vz )) - r->rot_vy )/8 ;

    if ( min < RCH_HIDE_RAD * RCH_HIDE_RAD )
	r->act = RCH_ActHiding ;
    RCH_DisplayModel( r ) ;
}


void RCH_ActStopAtEye( Work *work, Roach *r, int id )
{
    printf( "stop at eye\n" ) ;
    r->rot_vy = 0 ;
    if ( !(r->rot_vx = ( -1024 - r->rot_x ) / 8) )
	r->flag |= RCH_F_MOVE_PEEIR ;

    if ( GM_PlayerStatus & PLAYER_WATCH )
    {
	_sceVu0ScaleVector( &r->pos, (FVECTOR *)&DG_Chanl( 0 )->eye.m[Z], 250.0f ) ;
	_sceVu0AddVector( &r->pos, &r->pos, (FVECTOR *)&DG_Chanl( 0 )->eye.m[W] ) ;
    }
    else if ( !(--r->tics) )
	r->tics = 30, r->act = RCH_ActThink ;

    RCH_DisplayModel( r ) ;
}

void RCH_ActFlyAtEye( Work *work, Roach *r, int id )
{
    FVECTOR v ;

printf( "fly at eye\n" ) ;

    r->flag |= RCH_F_MOVE_WINGS ;

    if ( GM_PlayerStatus & PLAYER_WATCH )
    {
	_sceVu0ScaleVector( &v, (FVECTOR *)&DG_Chanl( 0 )->eye.m[Z], 250.0f ) ;
	_sceVu0AddVector( &v, &v, (FVECTOR *)&DG_Chanl( 0 )->eye.m[W] ) ;
	_sceVu0SubVector( &v, &v, &r->pos ) ;
    }
    else
	_sceVu0SubVector( &v, (FVECTOR *)BODYWORLD(GM_PlayerBody, HUMAN21_ATAMA).m[W], &r->pos ) ;

    r->rot_vy = ( (short)(2048.0f / M_PI * atan2f( v.vx, v.vz )) - r->rot_y )/8 ;
    r->rot_vx = ( -512 - r->rot_x ) / 8 ;
    if ( !(int)(v.vx/256.0f) && !(int)(v.vy/256.0f) && !(int)(v.vz/256.0f) )
	r->act = RCH_ActStopAtEye, r->tics = 30, r->flag &= ~RCH_F_MOVE_WINGS ;

    MoveToDirection( r, RCH_FLY_SPEED ) ;
    r->pos_v.vy = v.vy * 0.625f ;
    r->pos_v.vy = r->pos_v.vy>2.0f ? 2.0f : r->pos_v.vy<-2.0f ? -2.0f : r->pos_v.vy ;

    RCH_DisplayModel( r ) ;
}

void RCH_ActFlyToFront( Work *work, Roach *r, int id )
{
    FVECTOR v ;

    r->flag |= RCH_F_MOVE_WINGS ;
printf( "fly to front\n" ) ;

    _sceVu0ScaleVector( &v, (FVECTOR *)&DG_Chanl( 0 )->eye.m[Z], 300.0f ) ;
    _sceVu0AddVector( &v, &v, (FVECTOR *)&DG_Chanl( 0 )->eye.m[W] ) ;
    _sceVu0SubVector( &v, &v, &r->pos ) ;
    r->rot_vy = ( (short)(2048.0f / M_PI * atan2f( v.vx, v.vz )) - r->rot_y )/8 ;
    r->rot_vx = ( -512 - r->rot_x ) / 8 ;
    if ( !(int)(v.vx/256.0f) && !(int)(v.vy/256.0f) && !(int)(v.vz/256.0f) )
	r->act = RCH_ActFlyAtEye ;

    MoveToDirection( r, RCH_FLY_SPEED ) ;
    r->pos_v.vy = v.vy * 0.625f ;
    r->pos_v.vy = r->pos_v.vy>2.0f ? 2.0f : r->pos_v.vy<-2.0f ? -2.0f : r->pos_v.vy ;

    RCH_DisplayModel( r ) ;
}

void RCH_ActWowowRun( Work *work, Roach *r, int id )
{
    if ( !r->tics )
	AN_HeadMark( &r->pos, 0 ) ;
    else if ( r->tics >= 30 )
	r->act = RCH_ActRunaway ;
    r->tics++ ;
}

void RCH_ActWowowFly( Work *work, Roach *r, int id )
{
    if ( !r->tics )
	AN_HeadMark( &r->pos, 0 ) ;
    else if ( r->tics >= 30 )
    {
	if ( GM_PlayerStatus & PLAYER_WATCH )
	    r->act = RCH_ActFlyToFront ;
	else
	    r->act = RCH_ActFlyAtEye ;
    }
    r->tics++ ;
}

void RCH_ActThink( Work *work, Roach *r, int id )
{
    static void (*ActList[])( Work *, Roach *, int ) =  { RCH_ActWander,RCH_ActFlyAtEye,} ;

#if 0
    if ( RCH_SqLength2D( &r->pos, &GM_PlayerControl->mov ) < RCH_DANGER_RAD * RCH_DANGER_RAD )
    {
	if ( CheckDestanceLineAndPoint( (FVECTOR*)&GM_PlayerArmBody->objs->objs[4].world.m[W],
					(FVECTOR*)&GM_PlayerArmBody->objs->objs[4].world.m[Y],
					&r->pos ) < RCH_AIMED_RAD )
	    r->act = RCH_ActWowowFly ;
	else
	    r->act = RCH_ActWowowFly ;
	r->tics = 0 ;
    }
#endif
    if ( GM_PlayerStatus & PLAYER_WATCH )
	r->tics = 0, r->act = RCH_ActWowowFly ;

    if ( (GV_Time & 127) == r->tics )
    {
	r->act = RCH_ActWander ;
	r->tics = irnd() & 127 ;
	r->rot_vy  = r->tics & 0x01 ? 12 : 0 ;
	r->rot_vy -= r->tics & 0x10 ? 12 : 0 ;
    }

    RCH_DisplayModel( r ) ;
}
