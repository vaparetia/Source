//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_fil_ini.c
   紙舞い壊れ 初期化

   2000/04/25 T. Morita
   $Id: brk_fil_ini.c,v 1.1.1.3 2002/11/19 11:45:28 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_file.h"
#include "../brk_utl/brk_utl.x"


void BRK_FIL_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv )
{
    FVECTOR u = { uv[X],uv[Y],uv[Z],0 }, l = { lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( t_size, &u, &l ) ;
    _sceVu0ScaleVector( t_size, t_size, 0.5f ) ;
    _sceVu0AddVector( t_pos, &u, &l ) ;
    _sceVu0ScaleVector( t_pos , t_pos , 0.5f ) ;
}

int BRK_FIL_InitTarget( PILED *fil , int map )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &fil->target ;
    POWER_TARGET *p = &fil->power ;

    BRK_FIL_MakeSizeAndCenter( &t_size, &t_pos, &fil->objs->def->ux, &fil->objs->def->lx ) ;
    GM_SetTarget( t, TARGET_DEFENSE| TARGET_POWER| TARGET_SEEK, map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_FIL_TargetCallBack, fil ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget( t, (FVECTOR *)&fil->objs->world.m[W] ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

int BRK_FIL_InitHazard( Work *work )
{
    int hzd_id = GCL_GetOptionValue( 'h', BRK_HZD_W01D_DINING_L ) ;

    if ( !(work->hzd = BRK_InitHazard( hzd_id )) )
	return -1 ;
    return 0 ;
}


/* 破片の埃 効果  */
int BRK_FIL_InitPrimitive( Work *work )
{
    if ( !(work->dust = BRK_UTL_MakeSPRT( BRK_FIL_N_DUST,
					  GV_StrCode( "splash04_alp" ),
					  0 )) )
	PERROR( "No cannot make DG_PRIM2(work->dust)(May be no memory) :: NewPutFileObject\n" ) ;
    work->n_dust = BRK_FIL_INACTIVE ;

    return 0 ;
}


int BRK_FIL_InitPiece( Work *work, int where )
{
    int    i ;
    PIECE  *p ;
    DG_COMDL_POS *pos_s, *pos_l ;

    if ( work->p_def )
    {
        if ( !(work->piece_l = DG_MakeComdl( work->p_def->models[0].packs,
                                             DG_COMDL_NOFOG|DG_COMDL_SEMITRANS,
					     BRK_FIL_N_PIECE/2, 0)) )
            PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutFileObject\n" ) ;
        DG_QueueComdlObjs( work->piece_l ) ;
	GM_GroupObject( work->piece_l, where ) ;
        if ( !(work->piece_s = DG_MakeComdl( work->p_def->models[1].packs,
                                             DG_COMDL_NOFOG|DG_COMDL_SEMITRANS,
					     BRK_FIL_N_PIECE/2, 0)) )
            PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutFileObject\n" ) ;
        DG_QueueComdlObjs( work->piece_s ) ;
	GM_GroupObject( work->piece_s, where ) ;

	/* 子画面には見えないようにする */
	work->piece_s->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;
	work->piece_l->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;

        pos_l = work->piece_l->pos ;
        pos_s = work->piece_s->pos ;
        for ( i=BRK_FIL_N_PIECE/2, p=work->piece ; --i>=0 ; p+=2 )
        {
            p[0].pos  = pos_s++ ;
            p[0].flag = 0 ;
            p[1].pos  = pos_l++ ;
            p[1].flag = 0 ;
        }
    }
    work->n_piece = BRK_FIL_INACTIVE ;

    return 0 ;
}


void BRK_FIL_InitEneFind( Work *work )
{
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;
    int      grp, zone ;
    int      addr ;
    FVECTOR *pos ;

    pos = (FVECTOR *)work->piled->objs->world.m[W] ;
    HZX_Pos2Zone( pos, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, pos, addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}

void BRK_FIL_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_FIL_AlreadyMessy( work ) ;
}


int BRK_FIL_GetOptions( Work *work, int where )
{
    int      i ;
    PILED   *p ;
    IVECTOR  buf ;
    FVECTOR  pos ;
    SVECTOR  rot ;
    DG_DEF  *def ;
    LIT_DEF *lit ;

    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( !(i=GCL_GetOptionValue( 'm', 0 )) )
	PERROR( "No File Model ID (-model option missing) :: NewPutFileObject\n" ) ;
    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( i, 'k' ))) )
	PERROR( "Cannot find PILED FILER KMS-MODEL( not in data.cnf ) :: NewPutFileObject\n" ) ;
    if ( !(work->m1_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannot find FILER KMS-MODEL( not in data.cnf ) :: NewPutFileObject\n" ) ;
    if ( !(work->m2_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannot find FILER KMS-MODEL( not in data.cnf ) :: NewPutFileObject\n" ) ;
    if ( !(work->p_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannot find BROKEN FILER KMS-COMODEL( not in data.cnf ) :: NewPutFileObject\n" ) ;

    lit = GM_GetMap( where )->light ;
    if ( GCL_GetOption( 'l' ) )
	if ( !(lit = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'l' ))) )
	    PERROR( "Cannot find light data( not in data.cnf ) :: NewPutFileObject\n" ) ;

    if ( !(work->n_piled = GCL_GetOptionValue( 'n', 0 )) )
	PERROR( "The number must be more than one.(-number option maybe missing) :: NewPutFileObject\n" ) ;
    if ( !(work->piled = GV_Malloc( sizeof(PILED) * work->n_piled )) )
	PERROR( "Cannot allocate memory for <work->piled> :: NewPutFileObject\n" ) ;
    for ( i=work->n_piled, p=work->piled ; --i>=0 ; p++ )
	p->objs = NULL ; /* mallocからの初期化なのでNULLにする */

    p = work->piled ;
    while( (i = GCL_GetNextOption()) )
        switch( i )
	{
	case 'p':
	    GCL_GetIV( GCL_NextStr(), (int  *)&buf ) ;
	    vu0_IV0toFV( &buf, &pos ) ;
	    break ;

	case 'r':
	    GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;

	    if ( !(p->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
		PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutFileObject\n" ) ;
	    p->objs->world.m[X][X] =
	    p->objs->world.m[Y][Y] =
	    p->objs->world.m[Z][Z] = 0.5f ;
	    RotateMatrix( &p->objs->world, &p->objs->world, &rot ) ;
	    _sceVu0CopyVectorXYZ( (FVECTOR *)p->objs->world.m[W], &pos ) ;
	    p->work =work ;

	    BRK_FIL_InitTarget( p, where ) ;

	    DG_SetLightMatrix( p->objs, p->lights ) ;
	    DG_QueueObjs( p->objs ) ;
	    GM_GroupObjs( p->objs, where ) ;
	    //BRK_UTL_GetLightMatrix( &pos, p->lights, 1.0f, where ) ;
	    DG_GetLightMatrix( &pos, p->lights ) ;
	}

    return 0 ;
}

