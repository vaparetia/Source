//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_mgz_ini.c
   雑誌壊れ 初期化

   2000/06/20 T. Morita
   $Id: brk_mgz_ini.c,v 1.1.1.3 2002/11/19 11:45:35 Yoshizawa1 Exp $
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

#include "brk_magazine.h"
#include "../brk_utl/brk_utl.x"


void BRK_MGZ_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->magazine->target->center, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->magazine->target->center, addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}

void BRK_MGZ_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_MGZ_AlreadyMessy( work ) ;
}

void BRK_MGZ_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv )
{
    FVECTOR u = { uv[X],uv[Y],uv[Z],0 }, l = { lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( t_size, &u, &l ) ;
    _sceVu0ScaleVector( t_size, t_size, 0.5f ) ;
    _sceVu0AddVector( t_pos, &u, &l ) ;
    _sceVu0ScaleVector( t_pos , t_pos , 0.5f ) ;
}

int BRK_MGZ_InitTarget( MAGAZINE *mgz, TARGET *t, POWER_TARGET *p, int where, int flag,
			float *lx, float *ux, FVECTOR *pos )
{
    FVECTOR t_size, t_pos ;

    GM_FreeTarget( t ) ;

    BRK_MGZ_MakeSizeAndCenter( &t_size, &t_pos, ux, lx ) ;
    GM_SetTarget( t, TARGET_DEFENSE| TARGET_POWER| TARGET_NO_CLAYMORE| flag, where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_MGZ_TargetCallBack, mgz ) ;
    GM_PutTarget( t ) ;
    if ( pos )
	GM_MoveTarget( t, pos ) ;

    return 0 ;
}


int BRK_MGZ_InitHazard( Work *work )
{
    work->hzd_id = GCL_GetOptionValue( 'h', BRK_HZD_W01A1_TV_SPACE2 ) ;

    if ( !(work->hzd = BRK_InitHazard( work->hzd_id )) )
	PERROR( "Undefined Hazard ID!! :: NewPutMagazineObject\n" ) ;

    return 0 ;
}


/* 破片の埃 効果  */
int BRK_MGZ_InitPrimitive( Work *work )
{
    if ( !(work->dust = BRK_UTL_MakeSPRT( BRK_MGZ_N_DUST,
					  GV_StrCode( "blood_2bw_msk" ),
					  SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) )) )
	PERROR( "No cannot make DG_PRIM2(work->dust)(May be no memory) :: NewPutMagazineObject\n" ) ;
    work->n_dust = BRK_MGZ_INACTIVE ;

    return 0 ;
}


int BRK_MGZ_InitPiece( Work *work, int where )
{
    int     i ;
    PIECE  *p ;
    DG_COMDL_POS *pos_s, *pos_l ;

    if ( work->p_def )
    {
        if ( !(work->piece_l = DG_MakeComdl( work->p_def->models[0].packs,
                                             DG_COMDL_SEMITRANS, BRK_MGZ_N_PIECE/2, 0)) )
            PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutMagazineObject\n" ) ;
        DG_QueueComdlObjs( work->piece_l ) ;
	GM_GroupObject( work->piece_l, where ) ;
	work->piece_l->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;
        if ( !(work->piece_s = DG_MakeComdl( work->p_def->models[1].packs,
                                             DG_COMDL_SEMITRANS, BRK_MGZ_N_PIECE/2, 0)) )
            PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutMagazineObject\n" ) ;
        DG_QueueComdlObjs( work->piece_s ) ;
	GM_GroupObject( work->piece_s, where ) ;
	work->piece_s->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;

        pos_l = work->piece_l->pos ;
        pos_s = work->piece_s->pos ;
        for ( i=BRK_MGZ_N_PIECE/2, p=work->piece ; --i>=0 ; p+=2 )
        {
            p[0].pos  = pos_s++ ;
            p[0].flag = 0 ;
            p[1].pos  = pos_l++ ;
            p[1].flag = 0 ;
        }
    }
    work->n_piece = BRK_MGZ_INACTIVE ;

    return 0 ;
}


int BRK_MGZ_InitPage( Work *work )
{
    int   i ;
    PAGE *p ;

    for ( i=BRK_MGZ_N_PAGE, p=work->page ; --i>=0 ; p++ )
    {
	p->flag = -BRK_MGZ_SPOOLTIME-1 ;
	p->anime = NULL ;
	p->objs  = NULL ;
    }
    work->n_page |= BRK_MGZ_INACTIVE ;

    return 0 ;
}


int BRK_MGZ_GetOptions( Work *work, int where )
{
    int       i, id ;
    MAGAZINE *p   ;
    IVECTOR   buf ;
    SVECTOR   rot ;

    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( !GCL_GetOption( 'b' ) )
	PERROR( "-break option maybe missing :: NewPutMagazineObject\n" ) ;

    id = GCL_GetNextInt() ;
    if ( !(work->n_def = (DG_DEF*)GV_GetCache( GV_CacheID( id, 'k' ))) )
	PERROR( "No Page KMS-MODEL<%d> (not in data.cnf) :: NewPutMagazineObject\n", id ) ;
    if ( !(work->c_def = (CV2_DEF*)GV_GetCache( GV_CacheID( id, 'c' ))) )
	PERROR( "No Page CV2-MODEL<%d> (not in data.cnf) :: NewPutMagazineObject\n", id ) ;
    id = GCL_GetNextInt() ;
    if ( !(work->p_def = (DG_DEF*)GV_GetCache( GV_CacheID( id, 'k' ))) )
	PERROR( "No Dust KMS-MODEL<%d> (not in data.cnf) :: NewPutMagazineObject\n", id ) ;
    work->n_def->n_models = work->n_def->n_x_models = 1 ;
	

    if ( !(work->n_magazine = GCL_GetOptionValue( 'n', 0 ) ) )
	PERROR( "number must not be Zero(-number option maybe missing) :: NewPutMagazineObject\n" ) ;
    if ( !(p = work->magazine = GV_Malloc( sizeof(MAGAZINE) * work->n_magazine )) )
	PERROR( "No memory for <work->magazine> :: NewPutMagazineObject\n" ) ;
    for ( i=work->n_magazine ; --i>=0 ; p++ )
    {
	_sceVu0CopyVector( &p->mov.pos  , &DG_ZeroVector ) ;
	_sceVu0CopyVector( &p->mov.pos_v, &DG_ZeroVector ) ;
	p->mov.rot_x  = p->mov.rot_y  = 0 ;
	p->mov.rot_vx = p->mov.rot_vy = 0 ;
	p->objs = NULL ; /* mallocからの初期化なのでNULLにする */
	p->mgz_type = 0;
	p->y_offset = 8.0f * (float)(i%5);
	/* ひとまずNULLに初期化しておく */
	p->v_anime[0].mem[0] =
	    p->v_anime[1].mem[0] =
	    p->v_anime[2].mem[0] = NULL ;
	p->flag = -1 ;
	p->non_damg = 0 ;
	p->work = work ;
    }


    p = work->magazine ;
    while( (i = GCL_GetNextOption()) )
        switch( i )
        {
	    DG_DEF  *def ;
	    LIT_DEF *lit ;
        case 'm':
	    id = GCL_GetNextInt() ;
	    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( id, 'k' ))) )
		PERROR( "No Magazine KMS-MODEL<%d> (not in data.cnf) :: NewPutMagazineObject\n", id ) ;
	    if ( !(p->objs = DG_MakeObjs( def, DG_FLAG_PAINT| DG_FLAG_ONEPIECE, 0 )) )
		PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutMagazineObject\n" ) ;
	    DG_QueueObjs( p->objs ) ;
	    GM_GroupObjs( p->objs, where ) ;

	    id = GCL_GetNextInt() ;
	    if ( !(p->def = (DG_DEF*)GV_GetCache( GV_CacheID( id, 'k' ))) )
		PERROR( "No OpenMagazine KMS-MODEL<%d> (not in data.cnf) :: NewPutMagazineObject\n", id ) ;
	    break ;

	case 'H':
	    p->y_offset = 8.0f * 4.0f + 1.0f ;
	    break ;

	case 'r':
	    GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
	    p->mov.rot_x = rot.vx ;
	    p->mov.rot_y = rot.vy ;
	    break ;

	case 'p':
	    GCL_GetIV( GCL_NextStr(), (int  *)&buf ) ;
	    vu0_IV0toFV( &buf, &p->mov.pos ) ;
	    break ;

	case 't':
	    p->mgz_type = GCL_GetNextInt();
	    if( p->mgz_type < 0 || 9 <= p->mgz_type )
		p->mgz_type = 0 ;

	    BRK_MGZ_ChangeBook( p, 0 ) ;
	    break ;

	case 'l':
	    id = GCL_GetNextInt() ;
	    if ( !(lit = GV_GetCache( GV_CacheID( id, 'l' ))) )
		PERROR( "Cannot find Light<%d> data (not in data.cnf) :: NewPutMagazineObject\n", id ) ;

	    RotateMatrixZY( &p->objs->world, &DG_UnitMatrix, p->mov.rot_x, p->mov.rot_y ) ;
	    TransMatrix( &p->objs->world, &p->mov.pos ) ;
	    DG_MakePreshade( p->objs, lit ) ;

	    BRK_MGZ_InitTarget( p, p->target, p->power, work->where, TARGET_ROTATE,
				&p->objs->def->lx, &p->objs->def->ux, &p->mov.pos ) ;
	    GM_MoveTarget2( p->target, &p->objs->world ) ;

#if MAKING
	    p->view[0] = NewTargetView( p->target,  200, 50, 32 ) ;
	    p->view[1] = p->view[2] = NULL ;
#endif

	    p++ ;

	    break ;
	}
    work->n_magazine |= BRK_MGZ_INACTIVE ;

    return 0 ;
}
