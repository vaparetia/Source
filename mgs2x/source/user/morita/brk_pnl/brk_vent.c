//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_spotlgt.c 
   プットライトオブジェ

   1999/12/26 T.Morita
   $Id: brk_vent.c,v 1.1.1.3 2002/11/19 11:45:41 Yoshizawa1 Exp $
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

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"
#include "../brk_utl/brk_utl.x"


#define ANGtoRAD(_a) (RecalcDir(_a)*(float)M_PI/2048.0f)

typedef struct work_t
{
    GV_ACT       actor   ;

    DG_OBJS     *objs    ;
    int          where   ;
    int          name    ;
    DG_DEF      *def[3]  ;
    LIT_DEF     *lit_def ;
    int          time    ;
    short        stat    ;
    short        life    ;

    int          proc    ;

    HZX_D_SEGMENT *segs  ;

    TARGET       target ;
    POWER_TARGET power  ;
} Work ;

static inline int RecalcDir( int dir )
{
    dir &= 4095 ;
    if ( dir > 2048 )
	dir -= 4096 ;
    return dir ;
}

int BRK_VNT_InitModel( Work *work, DG_DEF *def, FMATRIX *mtx )
{
    DG_OBJS *objs ;

    /* モデルを作る */
    if ( !(objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot make DG_OBJS(Maybe no memory!!) : NewPutVentObject\n" ) ;
    DG_QueueObjs( objs ) ;
    if ( mtx )
	_sceVu0CopyMatrix( &objs->world, mtx ) ;
    DG_MakePreshade( objs, work->lit_def ) ;

    /* モデルがちゃんとできたら解放する */
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    work->objs = objs ;

    return 0 ;
}

#define MARGINE 30.0f
int BRK_VNT_MakeHzx( Work *work, int where, FVECTOR *size, FVECTOR *pos )
{
    IVECTOR      p[2] ;
    FVECTOR      v ;
    HZX_GROUP_ID hzx_id ;
    int          flag ;

    hzx_id = GM_GetHzxGroupID( where ) ;

    v.vx = pos->vx + size->vx ;
    v.vy = pos->vy - size->vy ;
    v.vz = pos->vz + size->vz ;
    v.vw = size->vy * 2.0f ;
    _sceVu0FTOI0Vector( &p[0], &v ) ;

    v.vx = pos->vx - size->vx ;
    v.vy = pos->vy - size->vy ;
    v.vz = pos->vz - size->vz ;
    v.vw = size->vy * 2.0f ;
    _sceVu0FTOI0Vector( &p[1], &v ) ;
    flag = HZX_SEG_NO_BULLET|HZX_SEG_NO_MISSILE ;
    if ( !(work->segs = HZX_AddDynamicSegment( hzx_id, &p[0], &p[1], flag )) )
	return -1 ;

    /*レーダーに映すため*/
    work->segs->atr &= ~HZX_SEG_NO_DISP_RADAR ;

    return 0 ;
}

static void VentBroken( Work *work )
{
    GM_FreeTarget( &work->target ) ;
    if ( work->segs )
    {
	HZX_RemoveDynamicSegment( work->segs ) ;
	work->segs = NULL ;
    }
}

static void BRK_VNT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work*)ptr ;
    static FVECTOR SoundPos = { 500.0f, 250.0f, 0.0f, 1.0f } ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->name == WP_ThrowG )
	    BRK_UTL_CallOffenceWhenThrough( off, def ) ;

	else if ( work->time != GM_StagePlayTime )
	    if ( off->weapon_type & WP_WEAPONCORE )
	    {
		FVECTOR spos ;

		_sceVu0ApplyMatrix( &spos, &work->objs->world,  &SoundPos ) ;

		/* ２度当たらないように時間を取っておく */
		work->time = GM_StagePlayTime ;

		/* 半壊状態ならライフを減らす */
		if ( work->stat == 1 )
		    if ( --work->life > 0 )
		    {
			GM_SeSetMode( SD_A_FENCEBR1, &spos, GM_SEMODE_BOMB ) ;
			return ;
		    }

		/* 状態を変更する */
		if ( ++work->stat >= 2 )
		{
		    GM_SeSetMode( SD_A_FENCEBR2, &spos, GM_SEMODE_BOMB ) ;
		    VentBroken( work ) ;
		}
		else
		    GM_SeSetMode( SD_A_FENCEBR1, &spos, GM_SEMODE_BOMB ) ;

		/* モデルを切替る */
		if ( !BRK_VNT_InitModel( work,
					 work->def[work->stat],
					 &work->objs->world ) )
		    if ( work->proc )
		    {
			int argv[] = { work->name, work->stat } ;
			GCL_ARGS arg  ;

			arg.argc = sizeof(argv)/sizeof(int) ;
			arg.argv = argv ;
			GCL_ExecProc( work->proc, &arg ) ;
		    }
	    }
    }
}


static void Die( Work *work )
{
    if ( work->segs )
	HZX_RemoveDynamicSegment( work->segs ) ;
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
    GM_FreeTarget( &work->target ) ;
}


static void BRK_VNT_InitTarget( Work *work, 
				TARGET *t, POWER_TARGET *p,
				DG_DEF *def, FMATRIX *m, int where )
{
    FVECTOR t_size, t_pos ;
    FVECTOR tv = { def->tx, def->ty, def->tz, 0.0f } ;
    FVECTOR lv = { def->lx, def->ly, def->lz, 0.0f } ;
    FVECTOR uv = { def->ux, def->uy, def->uz, 0.0f } ;

    _sceVu0AddVector( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
    _sceVu0ApplyMatrix( &t_pos, m, &t_pos ) ;
    _sceVu0AddVector( &t_pos, &t_pos, &tv ) ;
    _sceVu0AddVector( &t_pos, &t_pos, (FVECTOR *)m->m[W] ) ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0ApplyMatrix( &t_size, m, &t_size ) ;
    t_size.vx = fpu_Abs( t_size.vx ) ;
    t_size.vy = fpu_Abs( t_size.vy ) ;
    t_size.vz = fpu_Abs( t_size.vz ) ;

    BRK_VNT_MakeHzx( work, where, &t_size, &t_pos ) ;

    t_size.vx += MARGINE ;
    t_size.vy += MARGINE ;
    t_size.vz += MARGINE ;

    GM_SetTarget( t, 
		  TARGET_DEFENSE|TARGET_POWER|TARGET_CALL_CALLBACK_THROUGH_HIT,
		  where, BOTH_SIDE,
		  &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_VNT_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

#ifdef DEBUG_MODE
    //NewTargetView( t,  200, 50, 32 ) ;
#endif

}

#if DEBUG_MODE
//#define VENT_DEBUG
#ifdef VENT_DEBUG
static void Act( Work * work )
{
    if ( work->segs )
	HZX_ViewDynamicSegment( work->segs ) ;
}
#endif
#endif

static int GetResources( Work *work, int name, int where )
{
    IVECTOR buf ;
    IVECTOR rot ;
    int     flag ;
    FMATRIX mtx ;

    work->life    = GCL_GetOptionValue( 'l', 2 ) ;
    work->stat    = GCL_GetOptionValue( 's', 0 ) ;
    work->lit_def = GM_GetMap( where )->light ;
    work->name    = name  ;
    work->where   = where ;
    flag = GCL_GetOptionValue( 'f', 0 ) ;
    work->proc = GCL_GetOptionValue( 'O', 0 ) ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No model specified(-model option missing) :: NewPutVentObject\n" ) ;
    if ( !(work->def[0] = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewPutVentObject\n" ) ;
    if ( !(work->def[1] = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewPutVentObject\n" ) ;
    if ( !(work->def[2] = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "Cannot find KMS-Model( Not in data.cnf ) :: NewPutVentObject\n" ) ;

    if ( GCL_GetOption( 'r' ) )
    {
	GCL_GetNextIV( (int *)&rot ) ;
	_sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, ANGtoRAD(rot.vy) ) ;
	_sceVu0RotMatrixX( &mtx, &mtx          , ANGtoRAD(rot.vx) ) ;
	_sceVu0RotMatrixZ( &mtx, &mtx          , ANGtoRAD(rot.vz) ) ;
    }

    if ( !GCL_GetOption( 'p' ) )
	PERROR( "No position(-pos option missing) :: NewPutVentObject\n" ) ;
    GCL_GetNextIV( (int *)&buf ) ;
    vu0_IV0toFV( &buf, (FVECTOR *)mtx.m[W] ) ;
    mtx.m[W][W] = 1.0f ;

    if ( BRK_VNT_InitModel( work, work->def[work->stat], &mtx ) < 0 )
	return -1 ;

    BRK_VNT_InitTarget( work, &work->target, &work->power,
			work->def[0],  &mtx, where ) ;

    /* 既に壊れている */
    if ( work->stat >= 2 )
	VentBroken( work ) ;

    return 0 ;
}

void *NewPutVentObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
#ifdef VENT_DEBUG
        GV_SetActor( &work->actor, Act, Die ) ;
#else
        GV_SetActor( &work->actor, NULL, Die ) ;
#endif
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
