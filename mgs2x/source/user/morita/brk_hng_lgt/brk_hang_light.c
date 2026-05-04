//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_hang_light.c
   ライト揺れ

   1999/12/13 T. Morita
   $Id: brk_hang_light.c,v 1.1.1.3 2002/11/19 11:45:30 Yoshizawa1 Exp $
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

#include "brk_hang_light.h"

/* ベクトルからマトリックスを計算する（ベクトルがZ軸を向くようなマトリックス） */
static void VectorToMatatix( FMATRIX *m, FVECTOR *v )
{
    float sht, phi ;

    _sceVu0Normalize( v, v ) ;
    phi = atan2f( v->vz, v->vx ) ;
    sht = atan2f( v->vy, sceVu0Sqrt( v->vx*v->vx + v->vz*v->vz ) ) ;
    _sceVu0RotMatrixY( m, &DG_UnitMatrix, phi ) ;
    _sceVu0RotMatrixZ( m, m, -sht ) ; 
}

/* 先祖と子孫（つまりライトの根元）を結ぶベクトルを得る */
static void GetTipOfPos( int i, DG_OBJ *o, FVECTOR *pos )
{
    *pos = DG_ZeroVector ;
    for ( ; i!=-1 ; i=o[i].parent )
    {
        _sceVu0ApplyMatrix( pos, &o[i].world, pos ) ;
        pos->vx += o[i].model->tx ;
        pos->vy += o[i].model->ty ;
        pos->vz += o[i].model->tz ;
    }
}

static void MakeObjWorld( Work *work )
{
    FMATRIX *mtx, base ;
    FVECTOR pos ;
    DG_OBJ  *o = work->objs->objs ;
    int      i ;

    /* まず全ての間接のマトリックスを計算する */
    for ( i=work->objs->n_models ; --i>=0 ; )
	RotateMatrixXYZ( &o[i].world, &DG_UnitMatrix, work->rot_xyz[i] ) ;

    /* 先祖と子孫（つまりライトの根元）を結ぶベクトルを得る */
    GetTipOfPos( work->objs->n_models-1, o, &pos ) ;

    /* そのベクトルから逆マトリックスを得る */
    VectorToMatatix( &base, &pos ) ;

    /* そのマトリックスを間接全てに掛けることで ライトの根元は動かないように見える */
    RotateMatrix( &work->objs->world, &base, &work->rot ) ;
    TransMatrix( &work->objs->world, &work->pos ) ;
    for ( i=0 ; i<work->objs->n_models ; i++ )
    {
	TransMatrix( mtx = &o[i].world, (FVECTOR *)&o[i].trans ) ;
	if ( o[i].parent == -1 )
	    _sceVu0MulMatrix( mtx, &work->objs->world   , mtx ) ;
	else
	    _sceVu0MulMatrix( mtx, &o[o[i].parent].world, mtx ) ;
    }

    GM_MoveTarget2Map( &work->target, &o[1].world, work->where ) ;
}



static void BRK_HLGT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    FMATRIX mat ;
    FVECTOR offs, frc ;
    float scale ;
    Work *work = (Work *)ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;
	if ( !(off->weapon_type & (WP_BULLET|WP_M92)) )
	    return ;

	work->mode = BRK_MOD_MOVE ;

	/*ローカルの位置と力積のの向きを計算*/
        _sceVu0CopyVector( &offs, &def->hit ) ;
        offs.vw = 1.0f ;
        _sceVu0CopyVector( &frc, &off->power->force ) ;
        frc.vw = 0.0f ;
	_sceVu0InversMatrix( &mat, &work->objs->world ) ;
        _sceVu0ApplyMatrix( &offs, &mat, &offs ) ;
        _sceVu0ApplyMatrix( &frc , &mat, &frc ) ;
	/*力積の位置と方向から揺れる向きを計算*/
	_sceVu0ScaleVector( &frc, &frc, 0.05f ) ;
	offs.vx -= 350.0f ;

	work->rotv_xyz[0][Z] = (short) frc.vx ;
	work->rotv_xyz[1][Z] = (short)-frc.vx ;
	work->rotv_xyz[3][Z] = (short) frc.vx ;
	if ( !(int)(frc.vx/frc.vz*0.8f) )
	{
	    scale =  offs.vx > 0.0f ? -0.5f : 0.5f ;
	    _sceVu0ScaleVector( &frc, &frc, scale ) ;
	    work->rotv_xyz[0][X] =(short) -frc.vz ;
	    work->rotv_xyz[0][Y] =(short) -frc.vx ;
	    work->rotv_xyz[1][X] =(short) frc.vz ;
	    work->rotv_xyz[1][Y] =(short) frc.vx ;
		work->rotv_xyz[3][X] =(short)  frc.vz ;
		work->rotv_xyz[3][Y] =(short) -frc.vx ;
	}
    }
}

static int BRK_HLGT_InitTarget( Work *work, DG_OBJ *obj, int where )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;

    t_size.vx = (obj->model->ux - obj->model->lx) * 0.5f ;
    t_size.vy = (obj->model->uy - obj->model->ly) * 0.5f ;
    t_size.vz = (obj->model->uz - obj->model->lz) * 0.5f ;
    t_pos.vx  = (obj->model->ux + obj->model->lx) * 0.5f ;
    t_pos.vy  = (obj->model->uy + obj->model->ly) * 0.5f ;
    t_pos.vz  = (obj->model->uz + obj->model->lz) * 0.5f ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_SEEK, where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector) ;
    GM_SetTargetCallBack( t, BRK_HLGT_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}


static inline void BRK_HLGT_ReceiveMessage( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch( msg->message[0] )
        {
        case BRK_HLGT_M_INVISBLE:
	    work->objs->flag &= ~DG_FLAG_INVISIBLE ;
	    work->objs->flag |=  msg->message[1] ;
            break ;
        case BRK_HLGT_M_VISIBLE:
            break ;
        }
}

static void Die( Work *work )
{
    if ( work->objs )
    {
	DG_FreePreshade( work->objs ) ;
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
	GM_FreeTarget( &work->target ) ;
	
}

static void Act( Work *work )
{
    int  i, j, k, r, flag = 0 ;

    BRK_HLGT_ReceiveMessage( work ) ;
    if ( work->mode )
    {
		for ( i=BRK_MAX_MDL ; --i>=0 ; )
		  for( k=XYZ ; --k>=0 ; )
			{
				r = work->rotv_xyz[i][k] + work->rot_xyz[i][k] ;
				for ( j=6 ; r/2 && --j>=0 ; )
				  r /= 2 ;
				work->rot_xyz[i][k] += work->rotv_xyz[i][k] -= r ;
				flag |= work->rot_xyz[i][k] | work->rotv_xyz[i][k] ;
			}
		if ( !flag )
		  work->mode = BRK_MOD_FINISHED ;
		MakeObjWorld( work ) ;
    }
}

static int GetResources( Work *work, int name, int where )
{
    int      i, buf[3] ;
    DG_DEF  *def ;
    LIT_DEF *lit ;

    work->name  = name ;
	work->where = where ;
    work->mode  = BRK_MOD_FINISHED ;

    if ( (i = GCL_GetOptionValue( 'l', 0 )) )
	lit = GV_GetCache( GV_CacheID( i, 'l' ) ) ;

    if ( GCL_GetOption( 'r' ) )
	GCL_GetSV( GCL_NextStr(), (short*)&work->rot ) ;
    if ( GCL_GetOption( 'p' ) )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &work->pos ) ;
    }

    if ( !(i = GCL_GetOptionValue( 'm', 0 )) )
	PERROR( "No model ID(-m option missing) :: NewPutHangLight\n" ) ;
    if ( !(def = GV_GetCache( GV_CacheID( i, 'k' ) )) )
	PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewPutHangLight\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewPutHangLight\n" ) ;
    DG_QueueObjs( work->objs ) ;
    GM_GroupObjs( work->objs, where ) ;
    MakeObjWorld( work ) ;
    DG_MakePreshade( work->objs, GM_GetMap( where )->light ) ;

    BRK_HLGT_InitTarget( work, &work->objs->objs[1], where ) ;

    return 0 ;
}


void *NewPutHangLight( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
