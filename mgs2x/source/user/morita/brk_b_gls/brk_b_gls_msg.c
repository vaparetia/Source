//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_b_gls_msg.c
   ガラス壊れ メッセージ関数

   2000/04/25 T. Morita
   $Id: brk_b_gls_msg.c,v 1.1.1.3 2002/11/19 11:45:21 Yoshizawa1 Exp $
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
#include "def_dma.h"
#include "gameheader.h"

#include "brk_big_glass.h"


void BRK_BGLS_BreakGlass( int name )
{
    GV_MSG msg ;
    u_int buffer = 0 ;

    /* 大ガラスを一気に壊す。*/
    msg.address = name ;
    msg.message = (int*)&buffer ;
    msg.message_len = 1 ;
    GV_SendMessage( &msg ) ;
}

static void BRK_BGLS_BreakDown( Work *work )
{
    int i ;
    FVECTOR v ;

    if ( !BRK_BGLS_InitPart( work ) )
    {
	/* */
	for ( i=10-work->n_frac ; --i>=0 ; )
	{
	    v.vx = work->target.offset.vx + work->target.size.vx*frnd() ;
	    v.vy = work->target.offset.vy + work->target.size.vy*frnd() ;
	    v.vz = work->target.offset.vz + work->target.size.vz*frnd() ;
	    BRK_BGLS_PutGlassWeb( work, &v ) ;
	}
	work->life_acs = 0 ;
	work->life = 4 ;
	work->act = BRK_BGLS_ActFracture ;
	//if ( work->pr_name ) GCL_ExecProc( work->pr_name, NULL ) ;
	GM_FreeTarget( &work->target ) ;
    }

}

int BRK_BGLS_AlreadyMessy( Work *work )
{
    static IVECTOR def_color = { 64, 64, 64, 64 } ;
    int     i, j ;
    DG_DEF *def = work->def ;
    FMATRIX *m ;

    work->n_comdl = def->n_models ;
    /* 散らばせモデルのメモリーを確保する */
    if ( !(work->comdl = GV_Malloc( sizeof(DG_COMDL *) * work->n_comdl )) )
	PERROR( "Can't alocate memory :: NewPutBigGlass\n" ) ;
    for ( i=0 ; i<work->n_comdl ; i++ )
	work->comdl[i] = NULL ;
    for ( i=0 ; i<work->n_comdl ; i+=2 )
    {
	if ( !(work->comdl[i] = DG_MakeComdl( def->models[i].packs, DG_COMDL_SEMITRANS,
					      BRK_BGLS_N_MESSY, 0 )) )
	{
	    BRK_BGLS_FreeComdl( work ) ;
	    PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutBigGlass\n" ) ;
	}
	DG_QueueComdlObjs( work->comdl[i] ) ;
	GM_GroupObject( work->comdl[i], work->where ) ;

	/* 破片を散らばせる */
	for ( j=BRK_BGLS_N_MESSY ; --j>=0 ; )
	{
	    m = &work->comdl[i]->pos[j].world ;
	    work->comdl[i]->pos[j].color = def_color ;
	    _sceVu0CopyMatrix( m, &BRK_BGLS_MatrixTable[1024/256][(/*irnd()&4095*/0)/256] ) ;
	    if ( j & 0x01 )
	    {
		m->m[W][X] = work->target.offset.vx + work->target.size.vx*frnd() ;
		if ( irnd()&0x10000 )
		{
		    m->m[W][Y] = work->target.offset.vy - work->target.size.vy ;
		    m->m[W][Z] = work->target.offset.vz + 50.0f*frnd() ;
		}
		else
		{
		    m->m[W][Y] = work->target.offset.vy - work->target.size.vy - 100.0f ;
		    m->m[W][Z] = work->target.offset.vz +
			(irnd()&0x100 ? -100.0f : 100.0f) + 50.0f*frnd() ;
		}
	    }
	    else
	    {
		m->m[W][X] = work->target.offset.vx + work->target.size.vx*frnd() ;
		m->m[W][Y] = work->target.offset.vy - work->target.size.vy - 850.0f ;
		m->m[W][Z] = work->target.offset.vz +
		    (irnd()&0x100 ? -700.0f : 700.0f) + 250.0f*frnd() ;
	    }
	}
    }

    /* 壊れ前モデル や 細かい破片は解放する */
    BRK_BGLS_FreePiece( work ) ;
    if ( work->frac[0] )
	GM_FreePrim2( work->frac[0] ), work->frac[0] = NULL ;
    if ( work->frac[1] )
	GM_FreePrim2( work->frac[1] ), work->frac[1] = NULL ;
    work->life = work->life_acs = 0 ;
    work->n_piece |= BRK_BGLS_INACTIVE ;

    /* 当たりを外す */
    GM_FreeTarget( &work->target ) ;

    return 0 ;
}


void BRK_BGLS_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case BRK_BGLS_M_BREAKDOWN:
	    BRK_BGLS_BreakDown( work ) ;
	    break ;

	case BRK_BGLS_M_ALREADY_MESSY:
	    BRK_BGLS_AlreadyMessy( work ) ;
	    break ;
	}
}
