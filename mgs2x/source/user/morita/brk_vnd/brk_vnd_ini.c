//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_vending.c 
   プット自動販売機

   1999/12/26 T.Morita
   $Id: brk_vnd_ini.c,v 1.1.1.3 2002/11/19 11:45:53 Yoshizawa1 Exp $
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

#include "brk_vending.h"


void BRK_VND_InitTarget( TARGET *t, POWER_TARGET *p, int where,
			 DG_OBJS *objs,
			 void *callback, void *arg )
{
    FVECTOR t_size, t_pos ;
    FVECTOR lv = { objs->def->lx, objs->def->ly, objs->def->lz, 0 } ;
    FVECTOR uv = { objs->def->ux, objs->def->uy, objs->def->uz, 0 } ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos , &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|TARGET_ROTATE,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, callback, arg ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget2Map( t, &objs->world, where ) ;

    //NewTargetView( t, 200, 50, 32 ) ;
}

DG_OBJS *BRK_VND_ChangeModel( Work *work, DG_DEF *def, DG_OBJS *old, FMATRIX *lights )
{
    DG_OBJS *new  ;
    int      flag ;

    /* フラグを反映 */
    flag = (lights ? DG_FLAG_SHADE : DG_FLAG_PAINT) | DG_FLAG_ONEPIECE ;
    if ( (new = DG_MakeObjs( def, flag, 0 )) )
    {
	DG_QueueObjs( new ) ;
	GM_GroupObjs( new, work->where ) ;

	if ( old )
	{
	    _sceVu0CopyMatrix( &new->world, &old->world ) ;
	    DG_DequeueObjs( old ) ;
	    DG_FreeObjs( old ) ;
	}
	if ( flag & DG_FLAG_PAINT )
	    DG_MakePreshade( new, work->lit ) ;
	else
	{
	    DG_SetLightMatrix( new, lights ) ;
	    DG_GetLightMatrix( (FVECTOR *)new->world.m[W], lights ) ;
	}
    }

    return new ;
}

int BRK_VND_InitHazard( Work *work )
{
    int hzd_id = GCL_GetOptionValue( 'h', BRK_HZD_W01A1_TV_SPACE2 ) ;

    if ( !(work->hzd = BRK_InitHazard( hzd_id )) )
	PERROR( "Undefined Hazard ID!! :: NewPutFedralObject\n" ) ;

    return 0 ;
}

int BRK_VND_InitCan( Work *work, int where )
{
    /* 缶モデル設定 */
    if ( !GCL_GetOption( 'M' ) )
	PERROR( "No Can-Model option found(-can option missing) :: NewVendingMachine\n" ) ;
    if ( !(work->can_nrm  = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "No Normal-Can KMS-MODEL (not in data.cnf) :: NewPutFedralObject\n" ) ;
    if ( !(work->can_brk  = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "No Broken-Can KMS-MODEL (not in data.cnf) :: NewPutFedralObject\n" ) ;

    /* 缶モデルメモリ確保 */
    work->n_can = GCL_GetOptionValue( 'N', 3 ) ;
    if ( !(work->can = GV_Malloc( sizeof(CAN) * work->n_can )) )
	PERROR( "No memory avaiable for can :: NewPutFedralObject\n" ) ;
    GV_ZeroMemory( work->can, sizeof(CAN) * work->n_can ) ;
    work->n_can |= BRK_VND_INACTIVE ;

    return 0 ;
}

int BRK_VND_InitVending( Work *work, int where )
{
    IVECTOR buf ;
    SVECTOR rot ;
    FVECTOR pos ;
    int     id  ;
    VENDER *p   ;
    int     i   ;
    DG_DEF *def ;

    /* 自動販売機 */
    work->n_vender = GCL_GetOptionValue( 'n', 3 ) ;
    if ( !(work->vender = GV_Malloc( sizeof(VENDER) * work->n_vender )) )
	PERROR( "No memory avaiable for vender :: NewPutFedralObject\n" ) ;
    GV_ZeroMemory( work->vender, sizeof(VENDER) * work->n_vender ) ;

    i = 0 ;
    p = work->vender ;
    while( (id = GCL_GetNextOption()) )
        switch( id )
	{
	case 'm':
	    /* 自動販売機のモデルを作る */
	    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
		PERROR( "No Vender KMS-MODEL (not in data.cnf) :: NewPutFedralObject\n" ) ;
	    if ( !(p->objs = BRK_VND_ChangeModel( work, def, NULL, NULL )) )
		PERROR( "Can't initialize DG_OBJS for Vender :: NewPutFedralObject\n" ) ;

	    if ( !(p->vnd_brk = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
		PERROR( "No Broken-Vender KMS-MODEL (not in data.cnf) :: NewPutFedralObject\n" ) ;
	    break ;

	case 'r':
	    GCL_GetSV( GCL_NextStr(), (short *)&rot ) ;
	    break ;

	case 'p':
	    GCL_GetIV( GCL_NextStr(), (int   *)&buf ) ;
	    vu0_IV0toFV( &buf, &pos ) ;
	    break ;

	case 'C':
	    p->proc = GCL_GetNextInt() ;
	    break ;

	case 'e':
	    _sceVu0CopyVectorXYZ( (FVECTOR *)p->objs->world.m[W], &pos ) ;
	    BRK_VND_InitTarget( &p->target, &p->power,
				where, p->objs,
				BRK_VND_VenderCallBack, p ) ;
	    p->work    = work ;
	    p->can_num = 0    ;
	    p->life    = BRK_VND_LIFE ;

	    p++ ;
	    i++ ;
	    break ;
	}

    return 0 ;
}

int BRK_VND_GetOptions( Work *work, int where )
{
    int     flag ;

    flag = GCL_GetOptionValue( 'f', 0 ) ;

    /* ライトデータを取得 */
    work->lit = GM_GetMap( where )->light ;
    if ( GCL_GetOption( 'l' ) )
	if ( !(work->lit = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'l' ))) )
	    PERROR( "Cannot find Light data (not in data.cnf) :: NewPutFedralObject\n" ) ;

    return 0 ;
}
