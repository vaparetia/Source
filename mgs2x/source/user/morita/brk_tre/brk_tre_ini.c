//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tre_ini.c
   植物揺れ

   1999/12/17 T. Morita
   $Id: brk_tre_ini.c,v 1.1.1.3 2002/11/19 11:45:48 Yoshizawa1 Exp $
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

#include "brk_tree.h"




void BRK_TRE_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_TRE_AlreadyMessy( work ) ;
}

int BRK_TRE_InitTarget( Work *work, int where )
{
    int     i ;
    FVECTOR t_size ;
    FVECTOR t_pos  ;
    PART   *p = work->parts ;
    TARGET       *tr ;
    POWER_TARGET *pw ;

    work->target = tr = GV_Malloc( (sizeof(TARGET)+sizeof(POWER_TARGET))*(work->n_parts+1) ) ;
    work->power  = pw = (POWER_TARGET *)(tr + work->n_parts + 1) ;

    t_size.vx = (work->objs->def->ux - work->objs->def->lx)*0.5f ;
    t_size.vy = (work->objs->def->uy - work->objs->def->ly)*0.5f ;
    t_size.vz = (work->objs->def->uz - work->objs->def->lz)*0.5f ;

    t_pos.vx = work->objs->world.m[3][X] + work->objs->def->lx + t_size.vx ;
    t_pos.vy = work->objs->world.m[3][Y] + work->objs->def->ly + t_size.vy ;
    t_pos.vz = work->objs->world.m[3][Z] + work->objs->def->lz + t_size.vz ;

    GM_SetTarget( tr, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH|TARGET_CHILD|TARGET_CHILD_ALWAYS,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( tr, pw++, POWER_DECREASE, GM_Vitality, 0, 0,&DG_ZeroVector) ;
    GM_SetTargetCallBack( tr, BRK_TRE_TargetCallBack, p ) ;
    GM_PutTarget( tr++ ) ;
    GM_SetTargetParts( tr-1, tr, work->n_parts, 0 ) ;
    for ( i=work->n_parts ; --i>=0 ; p++, tr++, pw++ )
    {
	t_size.vx = (p->mdl->ux - p->mdl->lx)*0.5f ;
	t_size.vy = (p->mdl->uy - p->mdl->ly)*0.5f ;
	t_size.vz = (p->mdl->uz - p->mdl->lz)*0.5f ;

	t_pos.vx = work->objs->world.m[3][X] + p->mdl->tx + p->mdl->lx + t_size.vx ;
	t_pos.vy = work->objs->world.m[3][Y] + p->mdl->ty + p->mdl->ly + t_size.vy ;
	t_pos.vz = work->objs->world.m[3][Z] + p->mdl->tz + p->mdl->lz + t_size.vz ;

	GM_SetTarget( tr, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH|TARGET_CHILD_ALWAYS|TARGET_ONLINE, where, BOTH_SIDE, &t_size, &t_pos ) ;
	GM_SetPowerTarget( tr, pw, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( tr, BRK_TRE_TargetCallBack, p ) ;

	//NewTargetView( tr,  50, 200, 32 ) ;
    }

    return 0 ;
}

int BRK_TRE_InitHazard( Work *work, int where )
{
    work->hzx = GM_GetHzxGroupID( where ) ;

    if ( HZX_LevelHazardCheck( work->hzx, (FVECTOR *)work->objs->world.m[3],
			       HZX_CHK_ALL, HZX_FLOOR_ALL ) )
	HZX_GetLevelHeight( work->floor ) ;
    else
	work->floor[0] = work->floor[1] = 0.0f ;

    return 0 ;
}

static int BRK_TRE_InitModel( Work *work, int id, int where )
{
    DG_DEF  *def ;

    if ( !id )
	return -1 ;
    def = (DG_DEF*) GV_GetCache( GV_CacheID( id, 'k' ) ) ;

    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_FINISHCALC, 0 )) )
	return -1 ;
    DG_QueueObjs( work->objs ) ;
    GM_GroupObjs( work->objs, where ) ;

    return 0 ;
}


static int BRK_TRE_InitParts( Work *work )
{
    int      i ;
    DG_MDL  *m ;
    DG_OBJ  *o ;
    PART    *p ;

    work->n_parts = work->objs->def->n_models ;
    if ( !(work->parts = p = GV_Malloc( sizeof(PART)*work->n_parts ) )  )
	return -1 ;

    o = work->objs->objs ;
    m = work->objs->def->models ;
    for ( i=work->n_parts ; --i>=0 ; o++, m++, p++ )
    {
	p->obj  = o ;
	p->mdl  = m ;
	p->act  = BRK_TRE_ActFinished ;
	p->work = work ;
	p->rot_x = p->rot_z = p->rot_vx = p->rot_vz = 0 ;
	BRK_TRE_MakePosition( work, p ) ;
    }

    return 0 ;
}

static int BRK_TRE_InitCoModel( Work *work, int id, int where )
{
    int i ;
    DG_DEF  *def ;
    FMATRIX lgt[2] ;
    FVECTOR c = { 0.5f, 0.5f, 0.5f, 1.0f } ;

    if ( !(def = (DG_DEF *)GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "Can't find comdel ID(Not in data.cnf) :: NewPutTreeObject\n" ) ;
    if ( !(work->leaf = DG_MakeComdl( def->models[0].packs, DG_COMDL_DEFAULT, BRK_N_LEAVES, 0 )) )
	PERROR( "Can't make DG_COMDL(Maybe no memory) :: NewPutTreeObject\n" ) ;
    DG_QueueComdlObjs( work->leaf ) ;
    DG_GetLightMatrixFix( (FVECTOR *)&work->objs->world.m[W], lgt ) ;
    GM_GroupObject( work->leaf, where ) ;

    c.vx *= GV_VecLen3F( (FVECTOR*)&lgt[0].m[X] ) ;
    c.vy *= GV_VecLen3F( (FVECTOR*)&lgt[0].m[Y] ) ;
    c.vz *= GV_VecLen3F( (FVECTOR*)&lgt[0].m[Z] ) ;
    _sceVu0ApplyMatrix( &c, &lgt[1], &c ) ;

    for ( i=BRK_N_LEAVES ; --i>=0 ; )
    {
	work->leaf->pos[i].color.vx = (int)c.vx ;
	work->leaf->pos[i].color.vy = (int)c.vy ;
	work->leaf->pos[i].color.vz = (int)c.vz ;
	work->leaf->pos[i].color.vw = 0 ;
    }
    return 0 ;
}

int BRK_TRE_GetOptions( Work *work, int where )
{
    int      i ;
    IVECTOR  buf ;
    SVECTOR  rot ;
    FMATRIX *mtx ;
    FVECTOR  scale ;

    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;
    work->tend    = GCL_GetOptionValue( 't', 12 ) ;

    if ( BRK_TRE_InitModel( work, GCL_GetOptionValue( 'm', 0 ), where ) )
	return -1 ;

    mtx = &work->objs->world ;
    if ( GCL_GetOption( 's' ) != NULL )
    {
        GCL_GetNextIV( (int *)&buf ) ;
        vu0_IV0toFV( &buf, &scale ) ;
	mtx->m[X][X] *= scale.vx/100.0F ;
	mtx->m[Y][Y] *= scale.vy/100.0F ;
	mtx->m[Z][Z] *= scale.vz/100.0F ;
    }
    if ( GCL_GetOption( 'r' ) != NULL )
    {
	GCL_GetNextSV( (short*)&rot ) ;
	RotateMatrix( mtx, mtx, &rot ) ;
    }
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetNextIV( (int *)&buf ) ;
        vu0_IV0toFV( &buf, (FVECTOR *)mtx->m[W] ) ;
	mtx->m[W][W] = 1.0f ;
    }

    if ( BRK_TRE_InitParts( work ) )
	return -1 ;
    if ( BRK_TRE_InitCoModel( work, GCL_GetOptionValue( 'b', 0 ), where ) )
	return -1 ;

    if ( (i = GCL_GetOptionValue( 'l', 0 )) )
    {
	LIT_DEF *l ;
	if ( !(l = GV_GetCache( GV_CacheID( i, 'l' )) ) )
	    PERROR( "Can't find comdel ID(Not in data.cnf) :: NewPutTreeObject\n" ) ;
        DG_MakePreshade( work->objs, l ) ;
    }
    return 0 ;
}
