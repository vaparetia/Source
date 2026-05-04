//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
   brk_rdl_ini.c
   レードル揺れ 初期化

   2000/09/08 T. Morita
   $Id: brk_rdl_ini.c,v 1.2 2002/12/12 05:44:47 takaki Exp $
*/
#endif
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

#include "brk_radle.h"


int BRK_RDL_InitTarget( RADLE *radle, int where )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &radle->target ;
    POWER_TARGET *p = &radle->power  ;
    DG_DEF *def = radle->objs->def ;
    FVECTOR uv = { def->ux, def->uy, def->uz, 0 } ;
    FVECTOR lv = { def->lx, def->ly, def->lz, 0 } ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size,  &t_size, 0.5f ) ;
    _sceVu0MulVector( &t_size,  &t_size, &radle->scale ) ;
    _sceVu0AddVector( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos,  &t_pos, 0.5f ) ;
    _sceVu0MulVector( &t_pos,  &t_pos, &radle->scale ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_THROUGH,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_RDL_TargetCallBack, radle ) ;
    GM_PutTarget( t ) ;

#if MAKING
    NewTargetView( t,  200, 50, 32 ) ;
#endif
    return 0 ;
}

int BRK_RDL_GetOptions( Work *work, int name, int where )
{
    int      buf[3] ;
    SVECTOR  rot ;
    int     i   ;
    DG_DEF *def ;
    RADLE *radle ;

    work->where = where ;
    work->n_radle = 0 ;
    while( (i = GCL_GetNextOption()) )
	if ( i=='m' )
	    work->n_radle++ ;
    if ( !work->n_radle )
	PERROR( "No Model Specified(-m option missing) :: NewPutRadleObject\n" ) ;
    if ( !(work->radle = GV_Malloc( work->n_radle * sizeof(RADLE) )) )
	PERROR( "No memory for RADLE :: NewPutRadleObject\n" ) ;

    work->flag = GCL_GetOptionValue( 'f', 0 ) ;

    radle = work->radle ;
    while( (i = GCL_GetNextOption()) )
	switch( i )
	{
        case 'm':
	    if ( !(i = GCL_GetNextInt()) )
		PERROR( "No model ID(-m option missing) :: NewPutRadleObject\n" ) ;
	    if ( !(def = GV_GetCache( GV_CacheID( i, 'k' ) )) )
		PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewPutRadleObject\n" ) ;
	    if ( !(radle->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
		PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewPutRadleObject\n" ) ;
	    DG_QueueObjs( radle->objs ) ;
	    GM_GroupObjs( radle->objs, where ) ;

	    radle->rot_vx = 0 ;
	    radle->rot_vz = 0 ;

#if 0 //BP
//#ifndef KP_WINDOWS
	    radle->scale.vx = radle->scale.vy = radle->scale.vz = 1.0f ;
#else
		// Nan対策
	    radle->scale.vx = radle->scale.vy = radle->scale.vz = radle->scale.vw = 1.0f ;
#endif
	    radle->mode  = BRK_MOD_FINISHED ;
	    radle->work  = work ;
	    radle->se_id = 0 ;
#if MAKING
	    radle->wireframe = NULL ;
#endif
	    break ;

        case 'n':
		radle->name = GCL_GetNextInt() ;
		radle->where = where ;
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			extern int	C4MAN_Regist( int name, int *map, DG_OBJS *objs, FMATRIX *world, FVECTOR *shift, SVECTOR *rot ) ;

			static FVECTOR	shift={0.0, 0.0, 0.0} ;
			static SVECTOR	rot={-1024, 1024, 0} ;
			C4MAN_Regist( radle->name, &radle->where, radle->objs, &radle->objs->world, &shift, &rot ) ;
		}
	    break ;

	case 'D':
	    radle->se_id = GCL_GetNextInt() ;
	    break ;

        case 'r':
	    GCL_GetNextSV( (short*)&rot ) ;
	    radle->rot_x = radle->rot_dx = rot.vx*65536/4096 ;
	    radle->rot_y = radle->rot_dy = rot.vy*65536/4096 ;
	    radle->rot_z = radle->rot_dz = rot.vz*65536/4096 ;
	    break ;

        case 's':
	    GCL_GetNextIV( buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, &radle->scale ) ;
	    _sceVu0ScaleVector( &radle->scale,&radle->scale, 1.0f/100.0f ) ;
	    break ;

        case 'p':
	    GCL_GetNextIV( buf ) ;
	    vu0_IV0toFV( (IVECTOR *)buf, &radle->pos ) ;
	    break ;

        case 'e':
	    BRK_RDL_InitTarget( radle, where ) ;
	    MakeObjWorld( &radle->objs->world, radle, where ) ;

	    /* レードル同士での当たりの中心 */
	    radle->col_p = radle->target.offset ;
	    radle->col_p.vw = 1.0f ;
	    radle->col_p.vy += radle->col_p.vy + radle->target.size.vx ;
	    _sceVu0ApplyMatrix( &radle->col_p, &radle->objs->world, &radle->col_p ) ;
	    /*非当たりIDをクリア*/
	    radle->col_id = -1 ;
#if MAKING
	    {
		void *NewDrawWireframe( DG_OBJ * ) ;
		radle->wireframe = NewDrawWireframe( &radle->objs->objs[0] ) ;
	    }
#endif

	    radle++ ;
	    break ;
	}


    return 0 ;
}
