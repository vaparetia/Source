//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_spc_ini.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_spc_ini.c,v 1.1.1.3 2002/11/19 11:45:45 Yoshizawa1 Exp $
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

#include "brk_speech.h"


void BRK_SPC_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    work->piece_l->flag &= ~DG_FLAG_INVISIBLE ;
	    work->piece_s->flag &= ~DG_FLAG_INVISIBLE ;

	    GM_SetNoise( NOISE_S, &def->hit, work->where ) ;

	    work->objs->objs[1].flag |= DG_FLAG_INVISIBLE ;

	    /* 音がなる */
	    GM_SeSetMode( SD_A_BOTTLE02, &def->hit, GM_SEMODE_BOMB ) ;

	    /* 破片が出る */
	    BRK_SPC_StartActPieces( work, &off->power->force, 40 ) ;

	    GM_FreeTarget( &work->target ) ;
	}
    }
}

int BRK_SPC_InitTarget( Work *work, int where )
{
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
#if 0
    FVECTOR       uv = { work->objs->def->models->ux,
			 work->objs->def->models->uy,
			 work->objs->def->models->uz, 0.0f } ;
    FVECTOR       lv = { work->objs->def->models->lx,
			 work->objs->def->models->ly,
			 work->objs->def->models->lz, 0.0f } ;
#else
    FVECTOR       tv = { work->objs->def->tx,
			 work->objs->def->ty,
			 work->objs->def->tz, 0.0f } ;
    FVECTOR       uv = { work->objs->def->ux,
			 work->objs->def->uy,
			 work->objs->def->uz, 0.0f } ;
    FVECTOR       lv = { work->objs->def->lx,
			 work->objs->def->ly,
			 work->objs->def->lz, 0.0f } ;
#endif
    FVECTOR       t_size, t_pos ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0AddVector( &t_pos , &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;
    _sceVu0AddVector( &t_pos , &t_pos , &tv ) ;

    GM_SetTarget( t, TARGET_DEFENSE| TARGET_POWER| TARGET_ROTATE,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality,
		       0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_SPC_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget2Map( t, &work->objs->world, where ) ;

#if DEBUG_MODE
    if ( GCL_GetOptionValue( 'f', 0 ) )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    return 0 ;
}

int BRK_SPC_InitHazard( Work *work, int where )
{
    work->hzd = BRK_InitHazard( GCL_GetOptionValue( 'D', BRK_HZD_W24C_SPEECH ) ) ;

    return 0 ;
}

int BRK_SPC_InitPieces( Work *work, int where )
{
    int           i   ;
    DG_DEF       *def ;
    DG_COMDL_POS *pos_s, *pos_l, *pos ;

    if ( !(def = GV_GetCache( GV_CacheID( work->lpiece_nm, 'k' ) )) )
	PERROR( "No Model for COMDL(%d) in data.cnf : NewPutSpeechObject\n", work->lpiece_nm ) ;
    if ( !(work->piece_l = DG_MakeComdl( def->models[0].packs,
					 DG_COMDL_SEMITRANS,
					 BRK_N_LPIECE, 0 )) )
	PERROR( "No memory for COMDL(work->piece_l) : NewPutSpeechObject\n" ) ;
    DG_QueueComdlObjs( work->piece_l ) ;
    GM_GroupObject( work->piece_l, where ) ;
    work->piece_l->flag |= DG_FLAG_INVISIBLE ;

    if ( !(work->piece_s = DG_MakeComdl( def->models[1].packs,
					 DG_COMDL_SEMITRANS,
					 BRK_N_SPIECE, 0)) )
	PERROR( "No memory for COMDL(work->piece_s) : NewPutSpeechObject\n" ) ;
    DG_QueueComdlObjs( work->piece_s ) ;
    GM_GroupObject( work->piece_s, where ) ;
    work->piece_s->flag |= DG_FLAG_INVISIBLE ;

    pos_s = work->piece_s->pos ;
    pos_l = work->piece_l->pos ;
    for ( i=0 ; i<BRK_N_PIECE ; i++ )
    {
	if ( !(i&3) )
	    pos = pos_l++, work->piece[i].scale = 2.0f + 2.0f*rnd() ;
	else
	    pos = pos_s++, work->piece[i].scale = 2.0f ;
	work->piece[i].comdl = pos ;
	work->piece[i].flag  = 0   ;
	_sceVu0CopyMatrix( &pos->world, &DG_UnitMatrix ) ;
        pos->color.vx = 35 ;
	pos->color.vy = 80 ;
	pos->color.vz = 64 ;
        pos->color.vw = 0 ;
    }
    work->n_piece = BRK_SPC_INACTIVE ;

    return 0 ;
}


int BRK_SPC_GetOptions( Work *work, int where )
{
    IVECTOR  buf ;
    DG_DEF  *def ;
    FMATRIX *mtx ;
    int      id  ;

    if ( !(id  = GCL_GetOptionValue( 'm', 0 )) )
	PERROR( "No model option(-model missing) : NewPutSpeechObject\n" ) ;
    work->lpiece_nm =  GCL_GetNextInt() ;
    if ( !(def = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "Can't find KMS(%d) :: NewPutSpeechObject\n", id ) ;
    if ( !(work->objs = DG_MakeObjs( def,
				     DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't Create DG_OBJS(no memory)!! : NewPutSpeechObject\n" ) ;
    DG_QueueObjs( work->objs ) ;
    mtx = &work->objs->world ;
    if ( GCL_GetOption( 'r' ) )
    {
	GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	_sceVu0RotMatrixZ( mtx, mtx, DEGtoRAD(buf.vz) ) ;
	_sceVu0RotMatrixX( mtx, mtx, DEGtoRAD(buf.vx) ) ;
	_sceVu0RotMatrixY( mtx, mtx, DEGtoRAD(buf.vy) ) ;
    }
    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	vu0_IV0toFV( &buf, (FVECTOR *)mtx->m[W] ) ;
	mtx->m[W][W] = 1.0f ;
    }
    DG_MakePreshade( work->objs, GM_GetMap( where )->light ) ;

    return 0 ;
}
