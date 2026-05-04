/*
   brk_rol_ini.c
   転がりオブジェクト

   1999/12/13 T. Morita
   $Id: brk_rol_ini.c,v 1.1.1.3 2002/11/19 11:45:44 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_roll.h"

#if DEBUG_MODE
static int DebugFlag = 0 ;
#endif


int BRK_ROL_InitHazard( Work *work )
{
    if ( !GCL_GetOption( 'h' ) )
	PERROR( "No Hazard ID (-hazard option missing) :: NewPutRollObject\n" ) ;
    if ( !(work->hzd = BRK_InitHazard( GCL_GetNextInt() )) )
	PERROR( "Box Hazard wrong ID :: NewPutRollObject\n" ) ;
    return 0 ;
}

static void BRK_ROL_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    ROLL *p = (ROLL *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	GM_SeSetMode( SD_A_RICDAN01, &def->hit, GM_SEMODE_BOMB ) ;

	p->mov.rot_vx = (irnd()&0x100)-128  ;/* 揺らす力 強め */
	p->mov.rot_vy = (irnd()&0x100)-128  ;
	p->mov.roll_v = (irnd()&0x100)-128  ;

	_sceVu0ScaleVector( &p->mov.pos_v, &off->power->force, 0.01f ) ;
	p->mov.pos.vy += p->mov.pos_v.vy = 50.0f ;
	p->flag = 1 ;
	p->work->n_roll &= ~BRK_ROL_INACTIVE ;
    }
}

static inline void BRK_ROL_InitTarget( ROLL *p, int where, float *uv, float *lv )
{
    FVECTOR t_size, t_pos ;
    FVECTOR u={ uv[X],uv[Y],uv[Z],0 }, l={ lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( &t_size, &u, &l ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &u, &l ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    GM_SetTarget( &p->target, TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|TARGET_ROTATE,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( &p->target, &p->power, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &p->target, BRK_ROL_TargetCallBack, p ) ;
    GM_PutTarget( &p->target ) ;

#if DEBUG_MODE
    if ( DebugFlag || 1 )
	NewTargetView( &p->target,  200, 50, 32 ) ;
#endif
}

int BRK_ROL_GetOptions( Work *work, int where )
{
    int      i   ;
    ROLL    *p   ;

    if ( !(work->n_roll = GCL_GetOptionValue( 'n', 0 )) )
	PERROR( "Number must be more than one( or maybe no -number option) :: NewPutRollObject\n" ) ;
    if ( !(work->roll = GV_Malloc( sizeof(ROLL) * work->n_roll )) )
	PERROR( "Cannot allocate memory for <work->roll> :: NewPutRollObject\n" ) ;

    for ( i=work->n_roll, p=work->roll ; --i>=0 ; p++ )
    {
        _sceVu0CopyVector( &p->mov.pos  , &DG_ZeroVector ) ;
        _sceVu0CopyVector( &p->mov.pos_v, &DG_ZeroVector ) ;
	p->objs = NULL ; /* mallocからの初期化なのでNULLにする */
	p->mov.roll = 0 ;
	//p->flag = -1 ;
	p->flag = 1 ;
	p->work = work ;
    }    
    for( p=work->roll ; (i = GCL_GetNextOption()) ; )
        switch( i )
        {
	    DG_DEF  *def ;
	    IVECTOR  buf ;
	    SVECTOR  rot ;
        case 'm':
	    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
		PERROR( "No Roll KMS-MODEL (not in data.cnf) :: NewPutRollObject\n" ) ;
	    if ( !(p->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
		PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutRollObject\n" ) ;
	    DG_QueueObjs( p->objs ) ;
	    GM_GroupObjs( p->objs, where ) ;
	    break ;

	case 'r':
	    GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
	    p->mov.rot_x = rot.vx ;
	    p->mov.rot_y = rot.vy ;
	    break ;

	case 'p':
	    GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	    vu0_IV0toFV( &buf, &p->mov.pos ) ;
	    break ;

	case 'l':
	    GCL_GetSV( GCL_NextStr(), p->mov.limit ) ;

	    RotateMatrixYXY( &p->objs->world, &DG_UnitMatrix, 0, p->mov.rot_x, p->mov.rot_y ) ;
	    TransMatrix( &p->objs->world, &p->mov.pos ) ;

	    BRK_ROL_InitTarget( p, where, &p->objs->def->ux, &p->objs->def->lx ) ;
	    GM_MoveTarget2( &p->target, &p->objs->world ) ;
	    p++ ;

	    break ;
	}
    //work->n_roll |= BRK_ROL_INACTIVE ;

    return 0 ;
}
