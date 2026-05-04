/*
   brk_fed_msg.c
   雑誌壊れ

   2000/01/15 T. Morita
   $Id: brk_fed_msg.c,v 1.1.1.3 2002/11/19 11:45:27 Yoshizawa1 Exp $
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

#include "brk_fedralex.h"
#include "../brk_utl/brk_utl.x"


void BRK_FED_AlreadyMessy( Work *work )
{
    FEDRAL *m    ;
    FVECTOR   size, pos ;
    PIECE    *p    ;
    static FVECTOR  pce_size = { BRK_DUST_SPHERE, BRK_DUST_SPHERE, BRK_DUST_SPHERE } ;
    int       i, j ;

    work->n_fedral  &= ~BRK_FED_INACTIVE ;
    work->n_piece     &= ~BRK_FED_INACTIVE ;
    m = work->fedral ;
    for ( i=work->n_fedral ; --i>=0 ; m++ )
	if ( irnd() & 0x01100 )
	{
	    BRK_FED_ChangeBook( m ) ;
	    m->mov.rot_x  = (irnd() & 0x0010)  ? 1024 : -1024 ;
	    m->mov.rot_y  = irnd() & 4095 ;
	    m->mov.rot_vx = m->mov.rot_vy = 0 ;
	    _sceVu0CopyVector( &pos, &m->mov.pos ) ;
	    m->flag = 0 ;
	    /* 雑誌の降りる場所 自分の4方から検索 */
	    //for ( j=4 ; --j>=0 ; )
	    {
		m->mov.pos_v.vx = m->mov.pos_v.vz = 0.0f ;
		m->mov.pos_v.vy = -1000.0f ;
#if 0
		m->mov.pos.vx = pos.vx + (j==0 ? 1.0f : j==1 ? -1.0f : 0.0f)*(800.0f + 200.0f*frnd()) ;
		m->mov.pos.vy = pos.vy ;
		m->mov.pos.vz = pos.vz + (j==2 ? 1.0f : j==3 ? -1.0f : 0.0f)*(800.0f + 200.0f*frnd()) ;
#else
		m->mov.pos.vx = pos.vx + (800.0f + 200.0f*frnd()) ;
		m->mov.pos.vy = pos.vy ;
		m->mov.pos.vz = pos.vz ;
#endif

		RotateMatrixZY( &m->objs->world, &DG_UnitMatrix, m->mov.rot_x, m->mov.rot_y ) ;
		BRK_MGS_GetCurrentSize( m, &m->objs->world, &size, &pos ) ;
#if 0
		BRK_CheckHazard( work->hzd, &m->mov.pos, &m->mov.pos_v, &BRK_HZD_NoBounce, &size ) ;
#else
		if ( BRK_CheckHazard( work->hzd, &m->mov.pos, &m->mov.pos_v,
				      &BRK_HZD_NoBounce, &size ) & 1 )
#endif
		{
		    TransMatrix( &m->objs->world, &m->mov.pos ) ;
		    m->objs->world.m[W][Y] += m->y_offset;
		    BRK_UTL_GetLightMatrix( &m->mov.pos, m->lights, 0.6f, work->where ) ;
		    GM_MoveTarget2Map( &m->target, &m->objs->world, work->where ) ;
		    //break ;
		}
	    }

	    /* ゴミを散らせる */
	    for ( j=20 ; --j>=0 ; )
	    {
		p = &work->piece[work->n_piece] ;
		p->mov.pos_v.vx =    0.0f ;
		p->mov.pos_v.vy = -100.0f ;
		p->mov.pos_v.vz =    0.0f ;
		p->mov.pos.vx = m->mov.pos.vx + frnd() * 400.0f ;
		p->mov.pos.vy = m->mov.pos.vy ;
		p->mov.pos.vz = m->mov.pos.vz + frnd() * 400.0f ;

		RotateMatrixZY( &p->pos->world, &DG_UnitMatrix, 1024, irnd()&4095 ) ;
		if ( BRK_CheckHazard( work->hzd, &p->mov.pos, &p->mov.pos_v,
				      &BRK_HZD_NoBounce, &pce_size ) & 1 )
		{
		    BRK_UTL_ComdlColor( p->pos, 0.9f, work->where ) ;
		    TransMatrix( &p->pos->world, &p->mov.pos ) ;
		    if ( ++work->n_piece >= BRK_FED_N_PIECE )
			work->n_piece = 0 ;
		}
	    }
	}
    work->n_fedral |= BRK_FED_INACTIVE ;
    work->n_piece  |= BRK_FED_INACTIVE ;
}

void BRK_FED_ReceiveMessage( Work *work )
{
    int       i    ;
    GV_MSG   *msg  ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case 1:
	    BRK_FED_AlreadyMessy( work ) ;
	    break ;
	}
}



int ComGetFedralPos( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    extern Work *FED_Work ;
    int type, yes ;

    if ( FED_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    type = GCL_GetNextInt() ; /* テクスチャーの種類の取得 */

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FED_Work->fedral[type].mov.pos.vx ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FED_Work->fedral[type].mov.pos.vy ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)FED_Work->fedral[type].mov.pos.vz ) ;

    /* 表向いて 開いているかどうかの判定 */
    yes = 0 ;
    if ( FED_Work->fedral[type].mov.rot_x >= DEG2RAD(178.0f) )
	yes = 1 ;
    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, yes ) ;

    return 1 ;
}

