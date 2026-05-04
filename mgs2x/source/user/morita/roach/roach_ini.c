/*
  roach.c
  ゴキブリ

  2000/04/23 T. Morita
  $Id: roach_ini.c,v 1.1.1.3 2002/11/19 11:46:33 Yoshizawa1 Exp $
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

FMATRIX RCH_MatrixTable[16][16] ;
int RCH_InitMatrixTable( Work *work )
{
    int      x, z ;
    FMATRIX *m ;

    for ( z=16 ; --z>=0 ; )
	for ( x=16 ; --x>=0 ; )
	{
	    m = &RCH_MatrixTable[x][z] ;
	    _sceVu0RotMatrixX( m, &DG_UnitMatrix, (float)(2.0 * M_PI * x /16 - M_PI) ) ;
	    _sceVu0RotMatrixZ( m, m             , (float)(2.0 * M_PI * z /16 - M_PI) ) ;
	}
    return 0 ;
}

static int RCH_InitTarget( Roach *r, int map )
{
    FVECTOR ub = { r->objs->def->models[0].ux, r->objs->def->models[0].uy, r->objs->def->models[0].uz } ;
    FVECTOR lb = { r->objs->def->models[0].lx, r->objs->def->models[0].ly, r->objs->def->models[0].lz } ;
    FVECTOR t_size, t_pos ;
    TARGET       *t = &r->target ;
    POWER_TARGET *p = &r->power  ;

    _sceVu0SubVector( &t_size, &ub, &lb ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &ub, &lb ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;

    GM_SetTarget( t, TARGET_DEFENSE, map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_ONCE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, RCH_TargetCallBack, r ) ;
    GM_PutTarget( t ) ;
    NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}


int RCH_InitHazard( Work *work )
{
    if ( !(work->hzd = BRK_InitHazard( GCL_GetOptionValue( 'h', BRK_HZD_W01D_STORERAGE ) )) )
	PERROR( "Illeagal HAZARD-ID !! :: NewCockRoach\n" ) ;
    return 0 ;
}

int RCH_InitRoaches( Work *work, int map )
{
    int    i ;
    Roach *r ;

    if ( !(work->roach = GV_Malloc( sizeof(Roach) * work->n_roach )) )
	PERROR( "Cannot alloc memory :: NewCockRoach\n" ) ;
    for ( r=work->roach, i=work->n_roach ; --i>=0 ; r++ )
	r->objs = NULL ;
    for ( r=work->roach, i=work->n_roach ; --i>=0 ; r++ )
    {
	if ( !(r->objs = DG_MakeObjs( work->def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 )) )
	    PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewCockRoach\n" ) ;
	DG_QueueObjs( r->objs ) ;
	DG_SetLightMatrix( r->objs, r->lights ) ;

	_sceVu0CopyVector( &r->pos, &work->e_spot[irnd() % work->n_e_spot] ) ;

	r->tics  = irnd() & 63 ;
	r->act   = RCH_ActThink ;
	r->floor = DG_UnitMatrix ;
	RCH_InitTarget( r, map ) ;
    }
    return 0 ;
}

int RCH_GetOptions( Work *work, int name, int where )
{
    IVECTOR buf ;
    int   i ;
    char *c ;

    work->flag = GCL_GetOptionValue( 'f', 0 ) ;

    if ( !(work->n_roach = GCL_GetOptionValue( 'n', 0 )) )
	PERROR( "Must specify number of roaches(-n option missing) :: NewCockRoach\n" ) ;

    if ( !(i = GCL_GetOptionValue( 'm', 0 )) )
	PERROR( "No model ID(-m option missing) :: NewPutRadleObject\n" ) ;
    if ( !(work->def = GV_GetCache( GV_CacheID( i, 'k' ) )) )
	PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewCockRoach\n" ) ;

    if ( !GCL_GetOption( 'e' ) )
	PERROR( "Emerging Spot not specified( -e option missing ) :: NewCockRoach\n" ) ;
    for ( work->n_e_spot=0 ; (c=GCL_NextStr()) ; )
    {
	GCL_GetIV( c, (int *)&buf ) ;
	vu0_IV0toFV( &buf, &work->e_spot[work->n_e_spot++] ) ;
    }
    if ( !GCL_GetOption( 'h' ) )
	PERROR( "Hiding Spot not specified( -h option missing ) :: NewCockRoach\n" ) ;
    for ( work->n_h_spot=0 ; (c=GCL_NextStr()) ; )
    {
	GCL_GetIV( c, (int *)&buf ) ;
	vu0_IV0toFV( &buf, &work->h_spot[work->n_h_spot++] ) ;
    }

    return 0 ;
}
