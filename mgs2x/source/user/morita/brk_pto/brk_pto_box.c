//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pto_box.c
   じゃがいも壊れの箱

   2000/01/15 T. Morita
   2000/10/16 1.17 T.Morita 
   $Id: brk_pto_box.c,v 1.1.1.3 2002/11/19 11:45:42 Yoshizawa1 Exp $
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



static void StickPartsOnFloor( BOX *b, FVECTOR *min, float floor )
{
    int         i ;
    DG_MDLPACK *o = b->objs->objs->model->packs ;
    FVECTOR     f ;
#ifdef PSX2
    SVECTOR    *s = (SVECTOR*)o->verts ;
    min->vy = o->verts[Y] ;
	i       = o->n_verts ;
#else
	DG_VERTEX_KMSS *s = b->objs->objs->model->vbuff ;
    min->vy = s->vy ;
	i       = o->n_indices ;
#endif

    for ( ; --i>=0 ; s++ ) {
#ifdef PSX2
		vu0_SV0toFV( s, &f ) ;
#else
		f.vx = s->vx ;
		f.vy = s->vy ;
		f.vz = s->vz ;
#endif
		_sceVu0ApplyMatrix( &f, &b->objs->world, &f ) ;
		if ( f.vy < min->vy )
		  min->vy = f.vy ;
    }
    min->vy = floor - min->vy ;
}

short BRK_PTO_OpenDegreePlus[5*2] = {
     0,0,   1620,Z,  1024,X,   -400,X,  -1815,Z,
} ;
short BRK_PTO_OpenDegreeMinus[5*2] = {
     0,0,   1424,Z,   400,X,  -1024,X,  -1624,Z,
} ;
short *BRK_PTO_OpenDegree ;



void BRK_PTO_MoveRotateBox( BOX *b, int where )
{
    int   i, r ;

    r = b->brot.vz + b->brot_v.vz ;
    for ( i=2 ; r/2 && --i>=0 ; r/=2 ) ;
    b->brot.vz += b->brot_v.vz -= r ;

    b->brot.vy += b->brot_v.vy = b->brot_v.vy*7/8 ;
    b->brot.vy = b->brot.vy>b->r_max ? b->r_max : b->brot.vy<b->r_min ? b->r_min : b->brot.vy ;

    RotateMatrix( &b->objs->world, &DG_UnitMatrix, &b->brot ) ;
    StickPartsOnFloor( b, &b->pos, b->floor ) ;
    TransMatrix( &b->objs->world, &b->pos ) ;
    GM_MoveTarget2Map( &b->target, &b->objs->world, where ) ;
}


void BRK_PTO_MoveRotateFlip( BOX *b )
{
    int i, r ;
    FMATRIX *mtx ;

    BRK_PTO_OpenDegree = b->brot.vx>0 ? BRK_PTO_OpenDegreePlus : BRK_PTO_OpenDegreeMinus ;
    for ( i=b->objs->n_models ; --i>0 ; )
    {
	mtx = &b->objs->objs[i].world ;
	r = b->rot[i] & 0x0fff ;
	if ( BRK_PTO_OpenDegree[i*2+1] == X ) {
	    _sceVu0RotMatrixX( mtx, &DG_UnitMatrix, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	} else {
	    _sceVu0RotMatrixZ( mtx, &DG_UnitMatrix, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	}
	mtx->m[W][X] = b->objs->def->models[i].tx ;
	mtx->m[W][Y] = b->objs->def->models[i].ty ;
	mtx->m[W][Z] = b->objs->def->models[i].tz ;
	_sceVu0MulMatrix( mtx, &b->objs->world, mtx ) ;
    }
    b->objs->objs[i].world = b->objs->world ;
}


void BRK_PTO_ActOpenedBox( Work *work )
{
    BOX    *b = &work->box ;
    int     i, r ;
    int     flag = 0, t ;
    POTATO *p ;

    /*揺れ処理*/
    r = b->brot.vx + b->brot_v.vx - (b->brot.vx>0 ? 1024 : -1024) ;
    for ( i=2 ; r/2 && --i>=0 ; r/=2 ) ;
    b->brot.vx += b->brot_v.vx -= r ;
    r = b->brot.vz + b->brot_v.vz ;
    for ( i=2 ; r/2 && --i>=0 ; r/=2 ) ;
    b->brot.vz += b->brot_v.vz -= r ;
    b->brot_v.vy = 0 ;

    /*蓋を動かす*/
    BRK_PTO_OpenDegree = b->brot.vx>0 ? BRK_PTO_OpenDegreePlus : BRK_PTO_OpenDegreeMinus ;
    for ( i=b->objs->n_models ; --i>0 ; flag |= t )
	if ( (t = (b->rot[i]-BRK_PTO_OpenDegree[i*2])/8 ) )
	    b->rot[i] += b->rot_v[i] -= (b->rot_v[i] + b->rot[i] - BRK_PTO_OpenDegree[i*2])/8 ;
	else
	    b->rot[i] = BRK_PTO_OpenDegree[i*2] ;

    BRK_PTO_MoveRotateBox( b, work->where ) ;
    BRK_PTO_MoveRotateFlip( b ) ;

    /*箱の動きは止まったのか？*/
    if ( !flag && !b->brot_v.vx && !b->brot_v.vz )
	b->act = NULL, b->non_dmg = 0 ;
    if ( b->non_dmg )
	b->non_dmg-- ;

    /*箱が動いたらポテトも動く*/
    for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
	if ( p->act == BRK_PTO_ActNone )
	    p->act = BRK_PTO_Act ;
    work->flag = 1 ;
}

void BRK_PTO_ActOpenBox( Work *work )
{
    BOX    *b = &work->box ;
    int     i, flag = 0, t ;

    BRK_PTO_OpenDegree = b->brot.vx>0 ? BRK_PTO_OpenDegreePlus : BRK_PTO_OpenDegreeMinus ;
    for ( i=b->objs->n_models ; --i>0 ; flag |= t )
	if ( (t = (b->rot[i]-BRK_PTO_OpenDegree[i*2])/8 ) )
	    b->rot[i] += b->rot_v[i] -= (b->rot_v[i] + b->rot[i] - BRK_PTO_OpenDegree[i*2])/8 ;
	else
	    b->rot[i] = BRK_PTO_OpenDegree[i*2] ;
    BRK_PTO_MoveRotateFlip( b ) ;
    if ( !flag )
	b->act = NULL, b->non_dmg = 0 ;
}


void BRK_PTO_ActFallingDownBox( Work *work )
{
    BOX    *b = &work->box ;
    POTATO *p ;
    int     i ;

    /*倒れ処理  倒れてからも揺れる*/
    if ( b->brot.vx/1024 )
	b->brot.vx /= 1024, b->brot.vx *= 1024 ;
    else
	b->brot.vx += b->brot_v.vx += b->brot_v.vx/8 ;

    /*蓋を開く処理*/
    BRK_PTO_OpenDegree = b->brot.vx>0 ? BRK_PTO_OpenDegreePlus : BRK_PTO_OpenDegreeMinus ;
    for ( i=b->objs->n_models ; --i>0 ; )
	if ( i == 2 || i == 3 || b->brot.vx/800 )
	{
	    if ( !(b->rot[i]/BRK_PTO_OpenDegree[i*2]) )
		b->rot[i] += b->rot_v[i] -= (b->rot_v[i] + b->rot[i] - BRK_PTO_OpenDegree[i*2])/32 ;
	    else
		b->rot[i] = BRK_PTO_OpenDegree[i*2] ;
	}

    BRK_PTO_MoveRotateBox( b, work->where ) ;
    BRK_PTO_MoveRotateFlip( b ) ;

    /*ポテトを表示し,ポテトの動きを開始*/
    if ( b->brot.vx/800 && b->vitality )
    {
	GM_SeSetMode( SD_A_VGTFAL01, &b->pos, GM_SEMODE_BOMB ) ;
	for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
	{
	    /* じゃがいもの初期位置,速度,当たりを設定 */
	    _sceVu0ScaleVector( &p->pos_v,
				(FVECTOR*)b->objs->world.m[Y],
				120.0f+rnd()*30.0f ) ;
	    _sceVu0ApplyMatrix( &p->pos, &b->objs->world, &p->pos ) ;
	    _sceVu0AddVector( &p->pos, &p->pos, &b->pos ) ;
	    BRK_CheckHazard( work->hzd, &p->pos, &p->pos_v,
			     &BRK_PTO_Bounce, &BRK_PTO_Size[work->type] ) ;
	    DG_VisibleObjs( p->objs ) ;
	    BRK_PTO_InitTarget( &p->target, &p->power, work->where, 0,
				&work->m_def->ux, &work->m_def->lx, &p->pos,
				BRK_PTO_TargetCallBack, p ) ;
	}

#if DEBUG_MODE
#if MAKING
	{
	    int i ;
	    void *NewNameNum( DG_OBJS *body, int i ) ;
	    for ( i=0 ; i<work->n_potato ; i++ )
		NewNameNum( work->potato[i].objs, i ) ;
	}
#endif
#endif

	/*じゃがいも活動開始*/
	work->flag  = 1 ;
	b->vitality = 0 ;
    }
    /*箱が倒れたので蓋が開くアクトに切替える*/
    if ( (b->brot.vx == 1024 || b->brot.vx == -1024) && !b->brot.vz )
	b->act = BRK_PTO_ActOpenBox ;

    /*箱が動いたらポテトが動く*/
    for ( i=work->n_potato, p=work->potato ; --i>=0 ; p++ )
	if ( p->act == BRK_PTO_ActNone )
	    p->act = BRK_PTO_Act ;
    work->flag = 1 ;
}



void BRK_PTO_ActBox( Work *work )
{
    BOX  *b = &work->box ;
    int i, r ;

    /*揺れ処理*/
    r = b->brot.vx + b->brot_v.vx ;
    for ( i=2 ; r/2 && --i>=0 ; r/=2 ) ;
    b->brot.vx += b->brot_v.vx -= r ;

    BRK_PTO_MoveRotateBox( b, work->where ) ;
    BRK_PTO_MoveRotateFlip( b ) ;

    if ( !b->brot.vx && !b->brot.vz )
	work->box.act = NULL, b->non_dmg = 0 ;
    if ( b->non_dmg )
	b->non_dmg-- ;
}

