//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pto_msg.c
   じゃがいも壊れ

   2000/01/15 T. Morita
   $Id: brk_pto_msg.c,v 1.1.1.3 2002/11/19 11:45:42 Yoshizawa1 Exp $
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

#include "brk_potato.h"
#include "../brk_utl/brk_utl.x"

void BRK_PTO_RegistFoot( int name, OBJECT *body )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトに足のポインタを登録する。*/
    buffer[0] = BRK_PTO_M_REGIST_FOOT ;
    buffer[1] = (u_int)body ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}

void BRK_PTO_GetBoxPos( int name, FVECTOR **v )
{
    GV_MSG msg ;
    u_int buffer[2] ;

    /* 壊れポテトの箱の位置を取得する。*/
    buffer[0] = BRK_PTO_M_GET_POTATO_POS ;
    buffer[1] = (u_int)v ;
    msg.address = name ;
    msg.message = (int*)buffer ;
    msg.message_len = 2 ;
    GV_SendMessage( &msg ) ;
}


/*
  足にポテト用の当たりをつける。
*/
static inline void BRK_PTO_SetFoot( Work *work, OBJECT *body )
{
    POTATO *foot ;

    foot = &work->foot[work->n_foot++] ;
    foot->objs       = NULL ;
    foot->work       = NULL ;
    foot->n_parts    = 0    ;
    foot->parts.foot = (FVECTOR *)&body->objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[W] ;
    foot->target.size.vx = 200 ;
    foot->act = BRK_PTO_ActOnFoot ;

    foot = &work->foot[work->n_foot++] ;
    foot->objs       = NULL ;
    foot->work       = NULL ;
    foot->n_parts    = 0    ;
    foot->parts.foot = (FVECTOR *)&body->objs->objs[HUMAN21_HIDARI_TSUMASAKI].world.m[W] ;
    foot->target.size.vx = 200 ;
    foot->act = BRK_PTO_ActOnFoot ;
}

/*
  散らかった状態を作る
*/
void BRK_PTO_AlreadyMessy( Work *work )
{
    extern short BRK_PTO_OpenDegreePlus[5*2] ;
    int     i, j ;
    POTATO *p ;
    FVECTOR pos_v ;
    DG_OBJS *box = work->box.objs ;

    /* まずは箱を倒す */
    work->box.vitality = 0 ;
    work->box.brot.vx = 1024 ; 
    work->box.brot.vy = work->box.brot.vz = 0 ;
    work->box.act     = NULL ;
    work->box.non_dmg = 0    ;
    for ( j=box->n_models ; --j>0 ; )
	work->box.rot[j] = BRK_PTO_OpenDegreePlus[j*2] ;
    BRK_PTO_MoveRotateBox( &work->box, work->where ) ;
    BRK_PTO_MoveRotateFlip( &work->box ) ;

    /*箱に蓋付きあたりを貼る*/
    for ( i=box->n_models ; --i>0 ; )
    {
	FVECTOR t_size, t_pos ;
	DG_MDL *m = &box->def->models[i] ;

	BRK_PTO_MakeSizeAndCenter( &t_size, &t_pos, &m->ux, &m->lx ) ;
	work->box_hzd[i] = BRK_MakeHazard( BRK_HZD_OUTSIDE| BRK_HZD_ROTATE,
					   &box->objs[i].world, NULL,
					   &t_size, &t_pos, NULL ) ;
    }
    work->box_hzd[0]->flag = BRK_HZD_Y_BOX| BRK_HZD_ROTATE ;/* Y軸箱当たりに変更 */

    /* ポテトを配置する */
    for ( i=0, p=work->potato ; i<work->n_potato ; p++, i++ )
    {
	/* いくつか箱の外にポテトを置く */
	if ( i < 5 || i > 15 )
	    _sceVu0ScaleVector( &p->pos, (FVECTOR*)box->world.m[Y],
				500.0f+rnd()*300.0f ) ;
	else
	    _sceVu0CopyVector( &p->pos, &DG_ZeroVector ) ;

	/* いくつか箱の中にポテトを置く(最大10個まで) */
	_sceVu0AddVector( &p->pos, &p->pos, &work->box.target.center ) ;
	_sceVu0CopyVector( &p->pos_v, &DG_ZeroVector ) ;
	p->pos.vx += frnd()*400.0f ;
	p->pos.vz += frnd()*400.0f ;
	p->n_parts = 0 ;
	if ( i > 5 )
	{
	    /* いくつかポテトをおく */
	    BRK_PTO_InitPotato( p, work->where, work->m_def,
				DG_FLAG_SHADE|DG_FLAG_ONEPIECE,
				BRK_PTO_ActNone ) ;
	    pos_v.vz = pos_v.vx = pos_v.vw = 0.0f ;
	    pos_v.vy = -2000.0f ;

	    if ( BRK_CheckHazard( work->hzd, &p->pos, &pos_v,
				  &BRK_HZD_NoBounce,
				  &BRK_PTO_Size[work->type] ) & 1 )
	    {
		if ( BRK_HZD_Flag & BRK_HZD_THROUGH )
		    continue ;
		RotateMatrixXY( &p->objs->world, &DG_UnitMatrix,
				0, irnd() & 4095 ) ;
		TransMatrix( &p->objs->world, &p->pos ) ;
		BRK_PTO_InitTarget( &p->target, &p->power, work->where, 0,
				    &work->m_def->ux, &work->m_def->lx,
				    &p->pos,
				    BRK_PTO_TargetCallBack, p ) ;
		BRK_UTL_GetLightMatrix( &p->pos, p->lights, 1.0f, work->where ) ;
	    }
	    else
	    {
		p->act     = NULL ;
		p->n_parts = 0    ;
		GM_SetTarget( &p->target, TARGET_DEFENSE, work->where,
			      BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
		DG_InvisibleObjs( p->objs ) ;
	    }
	}

	else
	{

	    /* いくつかポテトを壊しておく */
	    BRK_PTO_InitPotato( p, work->where, work->p_def,
				DG_FLAG_SHADE|DG_FLAG_FINISHCALC,
				NULL ) ;
	    p->n_parts = 0 ;/*メモリは確保しないので０にしておくこと */

	    for ( j=work->p_def->n_models ; --j>=0 ; )
	    {
		p->objs->objs[j].flag |= DG_FLAG_INVISIBLE ;
		if ( BRK_UTL_PutOnHazard( &p->objs->objs[j].world, &p->pos,
					  200.0f, 4095, 4095,
					  work->hzd,
					  &BRK_PTO_PartSize[work->type] ) )
		    if ( !(BRK_HZD_Flag & BRK_HZD_THROUGH) )
			if ( p->pos.vy > p->objs->objs[j].world.m[W][Y] )
			    p->objs->objs[j].flag &= ~DG_FLAG_INVISIBLE0 ;
	    }
	    BRK_UTL_GetLightMatrix( &p->pos, p->lights, 1.0f, work->where ) ;

	}
    }

    /* ポテトをアクティブな状態にする (ポテトが重なる可能性があるから) */
    work->flag = 1 ;
    BRK_PTO_HzdPotatoCheck( work, BRK_POTATO_RAD*BRK_POTATO_RAD ) ;
}

void BRK_PTO_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	case BRK_PTO_M_REGIST_FOOT:
	    if ( work->n_foot < work->max_foot )
		BRK_PTO_SetFoot( work, msg->message[1] ? (OBJECT*)msg->message[1] : GM_PlayerBody ) ;
	    break ;

	case BRK_PTO_M_ALREADY_MESSY:
	    BRK_PTO_AlreadyMessy( work ) ;
	    break ;

	case BRK_PTO_M_GET_POTATO_POS:
	    *((FVECTOR **)msg->message[1]) = &work->potato[msg->message[2]].pos ;
	    break ;

	case BRK_PTO_M_GET_POTATO_NUM:
	    *((int **)msg->message[1]) = &work->n_potato ;
	    break ;

	case BRK_PTO_M_GET_BOX_POS:
	    *((FVECTOR **)msg->message[1]) = &work->box.pos ;
	    break ;
	}
}
