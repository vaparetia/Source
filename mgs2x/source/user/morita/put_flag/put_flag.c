//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_vanime.c
   プット頂点アニメ

   1999/12/13 T. Morita
   $Id: put_flag.c,v 1.1.1.3 2002/11/19 11:46:31 Yoshizawa1 Exp $
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

// for DG-LIB
#include "../include/libdg_x.h"
#include "../include/util.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_t
{
    GV_ACT    actor ;
    TARGET       target ;
    POWER_TARGET power  ;

    FMATRIX   lights[2] ;
    FVECTOR   pos    ;
    short     rot_y  ;
    short     rot_vy ;
    short     rot_dy ;
    short     rot_my ;

    short     rot_bul ;
    short     rot_msg ;

    short     key   ; /* キーフレーム */
    short     n_key ;
    short     mode  ;
    int       name  ;
    int       where ;

    DG_OBJS  *objs  ;
    VERTEX_ANIME_WORK *anime ;

} Work ;

#define DEGtoANG(_a) ((_a)*32768/180)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)


static int PUT_FLG_Count[] = {
    5,5,5,5,5,8, 4,8
} ;
static int PUT_FLG_NextKey[] = {
    1,2,3,4,5,0, 7,0
} ;


static void Die( Work *work )
{
    if ( work->anime )
	ExitVertexAnimation( work->anime ) ;
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs );
}

static void Act( Work *work )
{
    int     i   ;
    GV_MSG *msg ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	if ( !msg->message[0] )
	   work->rot_vy = -work->rot_msg ;
#if 1
    if ( GV_PadData[1].press & PAD_L1 )
	work->rot_vy = -work->rot_msg ;
#endif

    if ( work->rot_vy || work->rot_my || work->rot_y != work->rot_dy )
    {
	short   r   ;

	if ( work->rot_my )
	{
	    work->rot_my = work->rot_my*14/16 ; 
#if 0
	    r = work->rot_my - work->rot_y ;
	    for ( i=6 ; r/2 && --i>=0 ; r/=2 ) ;
	    work->rot_y += work->rot_vy += r ;
#else
	    work->rot_y = work->rot_my ;
	    work->rot_vy = 0 ;
#endif
	}
	else
	{
	    r = work->rot_vy + work->rot_y - (work->rot_dy + work->rot_my) ;
	    for ( i=6 ; r/2 && --i>=0 ; r/=2 ) ;
	    work->rot_y += work->rot_vy -= r ;
	}
	_sceVu0RotMatrixY( &work->objs->world, &DG_UnitMatrix,
			   ANGtoRAD(work->rot_y) ) ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)work->objs->world.m[W], &work->pos ) ;

	GM_MoveTarget2Map( &work->target, &work->objs->world, work->where ) ;
    }

    DG_GetLightMatrix( &work->pos, work->lights ) ;
    SimpleVertexAnimation( work->anime ) ;

    if ( work->anime->count == 0 )
    {
	if ( !work->rot_my )
	    if ( work->key == 3 )
		work->rot_vy = irnd()&255 ;

	work->target.class &= ~TARGET_SKIP ;
	work->anime->count = PUT_FLG_Count[work->key] ;
	work->anime->p[work->key] = 0.0f ;
	work->key = PUT_FLG_NextKey[work->key] ;
	work->anime->p[work->key] = 1.0f ;
    }
}


static void PUT_FLG_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = ptr ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;
	
	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	{
	    work->anime->count = 2 ;
	    work->anime->p[work->key] = 0.0f ;
	    work->key = 6 ;
	    work->anime->p[work->key] = 1.0f ;

	    work->rot_my = work->rot_bul ;

	    def->class |= TARGET_SKIP ;
	}
    }
}

static void PUT_FLG_InitTarget( Work *work, int where, DG_MDL *m )
{
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR       t_size,  t_pos ;
    FVECTOR       uv = { m->ux,m->uy,m->uz, 0.0f } ;
    FVECTOR       lv = { m->lx,m->ly,m->lz, 0.0f } ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
    t_size.vx -= 300.0f ;
    t_size.vy -= 100.0f ;
    t_size.vz  = 10.0f ;

    GM_SetTarget( t,
		  TARGET_DEFENSE| TARGET_POWER| TARGET_ROTATE| TARGET_THROUGH,
		  where,
		  BOTH_SIDE,
		  &t_size,
		  &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality,
		       0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, PUT_FLG_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget2Map( t, &work->objs->world, where ) ;

    //if ( flag & 1 )
    //NewTargetView( t,  200, 50, 32 ) ;
}

static int GetResourcesCalled( Work *work, int name, int where,
			       int id,
			       IVECTOR *rot, FVECTOR *pos )
{
    int      i ;
    FMATRIX *mtx ;
    CV2_DEF *cdef ;
    DG_DEF  *def  ;

    if ( !id )
	PERROR( "No model ID!!(-m option missing) :: NewPutFlagObject\n" ) ;
    if ( !(cdef = GV_GetCache( GV_CacheID( id, 'c' ) )) )
	PERROR( "No CV2<%d> found in data.cnf!! :: NewPutFlagObject\n", id ) ;
    if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS<%d> found in data.cnf!! :: NewPutFlagObject\n", id ) ;
    def->n_models = def->n_x_models = 1 ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0)) )
	PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewPutFlagObject\n" ) ;
    DG_QueueObjs( work->objs ) ;
    /*オブジェクトのバウンド変更*/
    work->objs->objs->bound_min.vx = def->lx ;
    work->objs->objs->bound_min.vy = def->ly ;
    work->objs->objs->bound_min.vz = def->lz ;
    work->objs->objs->bound_max.vx = def->ux ;
    work->objs->objs->bound_max.vy = def->uy ;
    work->objs->objs->bound_max.vz = def->uz ;

    work->rot_y  = 0 ;
    work->rot_vy = 0 ;
    work->rot_dy = 0 ;
    work->rot_my = 0 ;
    work->rot_bul = DEGtoANG( 80 ) ;
    work->rot_msg = DEGtoANG( 32 ) ;
    mtx = &work->objs->world ;
    if ( rot )
    {
	work->rot_bul = rot->vz*16 ;
	work->rot_msg = rot->vy    ;
	work->rot_y   = rot->vx*16 ;
	work->rot_dy  = rot->vx*16 ;
	_sceVu0RotMatrixY( mtx, mtx, ANGtoRAD(work->rot_y) ) ;
    }
    _sceVu0CopyVector( &work->pos, pos ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)mtx->m[W], pos ) ;

    /* ライトの設定 */
    DG_SetLightMatrix( work->objs, work->lights ) ; /* シェード */
    DG_GetLightMatrix( &work->pos, work->lights ) ;

    /* フレームコントロール初期化 */
    work->mode = 0 ;

    /* アニメーション初期化 */
    if ( !(work->anime = InitVertexAnimation( work->objs->objs,
					      cdef->models,
					      DG_VANIME_VERTS|DG_VANIME_NORMS,
					      cdef->n_models ) ) )
	PERROR( "InitVertexAnimation() returns NULL!! :: NewPutFlagObject\n" ) ;

    for ( i=0 ; i<cdef->n_models ; i++ )
	if ( cdef->models[i].n_verts == cdef->models[0].n_verts )
	    work->anime->key[i] = &cdef->models[i], work->anime->p[i] = 0.0f ;
	else
	    PERROR( "Animation Vertexies are not same!! :: NewPutFlagObject\n" ) ;
    work->anime->p[0] = 1.0f ;
    work->key   = 0 ;
    work->n_key = 6 /*cdef->n_models -1*/ ;
    work->name  = name  ;
    work->where = where ;

    PUT_FLG_InitTarget( work, where, &work->objs->def->models[3] ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    IVECTOR buf ;
    IVECTOR rot = { 0,0,0,0 } ;
    FVECTOR pos ;

    if (  GCL_GetOption( 'r' ) )
	GCL_GetNextIV( (int *)&rot ) ;
    if ( !GCL_GetOption( 'p' ) )
	PERROR( "No position!! :: NewPutFlagObject\n" ) ;
    GCL_GetNextIV( (int *)&buf ) ;
    vu0_IV0toFV( &buf, &pos ) ;

    if ( GetResourcesCalled( work, name, where,
			     GCL_GetOptionValue( 'm', 0 ),
			     &rot, &pos ) < 0 )
	return -1 ;
    return 0 ;
}


void *NewPutFlagObject( int name, int where )
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
