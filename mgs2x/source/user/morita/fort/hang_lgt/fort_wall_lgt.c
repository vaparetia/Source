//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_wall_lgt.c
   フォーチュン戦専用 壁ライト

   1999/12/13 T. Morita
   $Id: fort_wall_lgt.c,v 1.1.1.3 2002/11/19 11:46:12 Yoshizawa1 Exp $
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


// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_hzd/brk_hazard.h"
#include "../../brk_utl/brk_utl.x"
#include "../include/fort_flag.h"


#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_GRAVITY      6
#define BRK_BOUNCE       1.68f
#define BRK_BOUNCE_WALL  1.38f

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_t
{
    GV_ACT    actor  ;

    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR   pos       ;
    FVECTOR   pos_v     ;
    short     rot[XY]   ;
    short     rot_v[XY] ;
    short     tic       ;
    short     se_tic    ;

    //DG_COMDL *gloom  ;
    DG_OBJS  *gloom  ;
    DG_OBJS  *objs   ;
    HZD_BOX  *hzd    ;

    int       where  ;
    int       flag   ;

    DG_DEF   *bf_def  ;/* モデルID */
    DG_DEF   *af_def  ;
    DG_DEF   *dr_def  ;
} Work ;



#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

extern void *NewFortSpark( FVECTOR *pos, FVECTOR *dir, HZD_BOX *hzd, int mode ) ;
extern void  OK_RemoveDynamicLight( FVECTOR *pos ) ;
extern void  OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot ) ;
extern int   FRT_AIM_AddAimSpot( void *aim_id, void *nxt_id, int phase,
				 FVECTOR *center, FVECTOR *front, FVECTOR *rear, u_int flag ) ;
extern int   FRT_AIM_RemoveAimSpot( void *work ) ;
extern void *NewCrushWithForce( FVECTOR *pos, FVECTOR *force ) ;


static void RotateTransMatrix( Work *work, FMATRIX *mtx )
{
    _sceVu0CopyMatrix( mtx, &DG_UnitMatrix ) ;
    if ( work->flag & 0x10 )
	mtx->m[X][X] *= -1 ;
    _sceVu0RotMatrixX( mtx, mtx, ANGtoRAD(work->rot[X]) ) ;
    _sceVu0RotMatrixY( mtx, mtx, ANGtoRAD(work->rot[Y]) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], &work->pos ) ;
    GM_MoveTarget2Map( &work->target, mtx, work->where ) ;
}

#if 0
static void RotateMatrixXY( Work *work )
{
    FMATRIX *mtx = &work->objs->world ;
    int r ;

    _sceVu0CopyMatrix( mtx, &DG_UnitMatrix ) ;
    if ( work->flag & 0x10 )
	mtx->m[X][X] *= -1 ;
    r = work->rot[X] & 0x0fff ;
    _sceVu0RotMatrixX( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = work->rot[Y] & 0x0fff ;
    _sceVu0RotMatrixY( mtx, mtx, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
#else
static void RotateMatrixXY( Work *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0CopyMatrix( mtx, &DG_UnitMatrix ) ;
    if ( work->flag & 0x10 )
	mtx->m[X][X] *= -1 ;
    _sceVu0RotMatrixX( mtx, mtx, ANGtoRAD(work->rot[X]) ) ;
    _sceVu0RotMatrixY( mtx, mtx, ANGtoRAD(work->rot[Y]) ) ;
}
#endif

static inline void TransMatrix( Work *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], &work->pos ) ;
    GM_MoveTarget2Map( &work->target, mtx, work->where ) ;
}


int FRT_WLGT_InitModel( Work *work, DG_DEF  *def )
{
    DG_OBJS *objs ;
    LIT_DEF *lit = GM_GetMap( work->where )->light ;

    if ( !(objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot make DG_OBJS(Maybe no memory!!) :NewFortWallLight" ) ;
    DG_QueueObjs( objs ) ;
    RotateTransMatrix( work, &objs->world ) ;
    DG_MakePreshade( objs, lit ) ;

    if ( work->objs )
    {
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
    work->objs = objs ;

    return 0 ;
}

static void FRT_WLGT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    static FVECTOR OffenseSize = { 500.0f, 500.0f, 500.0f, 0.0f } ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_NIKITA|WP_GRENADE) )
	{
	    DG_DestroyLightSphere( &work->pos, 2000.0f ) ;
	    FRT_WLGT_InitModel( work, work->af_def ) ;
#if 0
	    //work->gloom->pos[0].color.vw = 0 ;
#else
	    DG_InvisibleObjs( work->gloom ) ;
#endif
	    /*割れ音*/
	    GM_SeSetMode( SD_A_LLIGHT01, &work->pos, GM_SEMODE_BOMB ) ;
	    work->se_tic = 0 ;
	}
	if ( off->weapon_type & (WP_NIKITA|WP_GRENADE) )
	{
	    work->flag = 1 ;
	    work->rot_v[X] = (irnd()&63) - 32 ;
	    work->rot_v[Y] = (irnd()&31) - 16 ;
	    _sceVu0ScaleVector( &work->pos_v, &off->power->force, 0.05f ) ;

	    /*防御ターゲットを取りさって,攻撃ターゲットに変える*/
	    GM_FreeTarget( def ) ;
	    GM_SetTarget( def, TARGET_OFFENSE|TARGET_POWER,
			  work->where, PLAYER_SIDE, &OffenseSize, &DG_ZeroVector ) ;
	    GM_SetTargetWeaponType( def, WP_KICK ) ;
	    GM_SetPowerTarget( def, &work->power, POWER_DECREASE, GM_Vitality,
			       10, 10, &DG_ZeroVector ) ;

	    /* エフェクト */
	    NewFortSpark( &def->hit, &off->power->force, work->hzd, 2 ) ;
	    NewCrushWithForce( &def->hit, &off->power->force ) ;

	    /* ターゲットシステムから外す */
	    FRT_AIM_RemoveAimSpot( &work->target ) ;
#if 0
	    work->gloom->pos[0].color.vw = 0 ;
#else
	    DG_InvisibleObjs( work->gloom ) ;
#endif
	}
    }
}

static int FRT_WLGT_InitTarget( Work *work, DG_OBJ *obj, int where )
{
    FVECTOR       t_size, t_pos ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;

    t_size.vx = (obj->model->ux - obj->model->lx) * 0.5f ;
    t_size.vy = (obj->model->uy - obj->model->ly) * 0.5f ;
    t_size.vz = (obj->model->uz - obj->model->lz) * 0.5f ;
    t_pos.vx  = (obj->model->ux + obj->model->lx) * 0.5f ;
    t_pos.vy  = (obj->model->uy + obj->model->ly) * 0.5f ;
    t_pos.vz  = (obj->model->uz + obj->model->lz) * 0.5f ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_SEEK,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, FRT_WLGT_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

static void Die( Work *work )
{
    if ( work->objs )
    {
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }
    if ( work->gloom )
    {
#if 0
	DG_DequeueComdlObjs( work->gloom ) ;
	DG_FreeComdl( work->gloom ) ;
#else
	DG_DequeueObjs( work->gloom ) ;
	DG_FreeObjs( work->gloom ) ;
#endif
    }
    if ( work->target.class & TARGET_DEFENSE )
	GM_FreeTarget( &work->target ) ;
    if ( !(work->flag & 0x0020) )
	FRT_AIM_RemoveAimSpot( &work->target ) ;
    //OK_RemoveDynamicLight( &work->pos ) ;
}

static int FRT_WLGT_ActDrop( Work *work )
{
    int flag = 0 ;
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    FVECTOR size ;

    work->pos_v.vy -= BRK_GRAVITY ;
    work->rot[X] += work->rot_v[X] ;
    work->rot[Y] += work->rot_v[Y] ;
    RotateMatrixXY( work ) ;
    BRK_UTL_SizeOfBound( work->objs, &work->objs->world, &size ) ;
    switch( BRK_CheckHazard( work->hzd, &work->pos, &work->pos_v, &bounce, &size ) )
    {
    case 3:
    case 1:
	work->pos_v.vx *= BRK_VEL_R ;
	work->pos_v.vz *= BRK_VEL_R ;
	if ( BRK_UTL_PutCenterHazard( work->hzd, &work->pos, &work->pos_v, 1000.0f, 25.0f ) )
	{
	    work->pos_v.vy = 30.0f ;
	    work->rot_v[X] = irnd()&10 ? 200 : -200 ;
	}
	else
	{
	    work->rot_v[X] = ((work->rot[X]> 0 ? DEGtoANG(90) : -DEGtoANG(90)) - work->rot[X]) / 16 ;
	    if ( !(int)work->pos_v.vx && !(int)work->pos_v.vz &&
		 !(int)(work->pos_v.vy/BRK_GRAVITY) )
		work->rot_v[X]=0, flag = 1 ;
	}
	/* ガタガタしないようにする */
	if ( work->tic > 0 )
	    work->rot_v[X]=0, flag = 1 ;
	else
	{
	    int pan, vol ;
       float bp_angle;

	    vol = GM_SeGetVol( &work->pos, GM_SEMODE_NORMAL ) ;
	    vol = (4 - work->se_tic++) * vol / 4 ;
	    vol = vol>10 ? vol&0x3f : 10 ;
	    pan = GM_SeGetPan( &work->pos, GM_SEMODE_NORMAL, &bp_angle ) ;
	    GM_SeSet3D( pan, vol, SD_A_LANPOTI1, bp_angle ) ;
	}
	work->tic = 10 ;
	break ;
    case 0:
	_sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
	break ;
    }
    TransMatrix( work ) ;
    GM_MoveTarget2Map( &work->target, &work->objs->world, work->where ) ;
    GM_PutTarget( &work->target ) ;
    if ( work->tic > 0 )
	work->tic-- ;

    return flag ;
}

static int FRT_WLGT_ActFalldown( Work *work )
{
    int flag = 0 ;
    FVECTOR size ;

    work->pos_v.vy = -BRK_GRAVITY*5 ;

    work->rot[X] += work->rot_v[X] += work->rot[X]>0 ? DEGtoANG(5) : -DEGtoANG(5) ;
    work->rot[Y] += work->rot_v[Y]  = work->rot_v[Y] * (BRK_ROT_R-1) / BRK_ROT_R ;
    if ( work->rot[X]/DEGtoANG(90) )
    {
        work->rot[X] = work->rot[X]>0 ? DEGtoANG(90) : -DEGtoANG(90) ;
	work->rot_v[X] = 0 ;
	if ( !work->rot_v[Y] )
	    flag = 1 ;
    }
    RotateMatrixXY( work ) ;
    BRK_UTL_SizeOfBound( work->objs, &work->objs->world, &size ) ;
    if ( !BRK_CheckHazard( work->hzd, &work->pos, &work->pos_v, &BRK_HZD_NoBounce, &size ) )
	work->pos.vy += work->pos_v.vy, flag=0 ;
    TransMatrix( work ) ;

    return flag ;
}

static void Act( Work *work )
{
    switch( work->flag & 0x000f )
    {
    case 1:
	if ( FRT_WLGT_ActDrop( work ) )
	   work->flag |= 2 ;
	break ;
    case 3 :
	if ( FRT_WLGT_ActFalldown( work ) )
	   work->flag |= 4 ;
    }
}

/* 明るくする */
#if 0

static DG_COMDL *InitGloomComdl( Work *work, DG_DEF *def )
{
    DG_COMDL *comdl ;

    comdl = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, 1, 0 ) ;
    if ( comdl )
    {
	DG_QueueComdlObjs( comdl ) ;
	GM_GroupObject( comdl, work->where ) ;
	RotateTransMatrix( work, &comdl->pos[0].world ) ;

	/* 色の指定 */
	comdl->pos[0].color.vx = comdl->pos[0].color.vy = comdl->pos[0].color.vz = 
	    comdl->pos[0].color.vw = 128 ;
    }
    return comdl ;
}
#else

static DG_OBJS *InitGloomComdl( Work *work, DG_DEF *def )
{
    DG_OBJS *objs ;

    if ( (objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
    {
	DG_QueueObjs( objs ) ;
	RotateTransMatrix( work, &objs->world ) ;
    }
    return objs ;
}

#endif

static int GetResources( Work *work, int name, int where )
{
    IVECTOR  buf ;
    int      id  ;
    DG_DEF  *def ;

    work->where = where ;
    work->flag  = GCL_GetOptionValue( 'f', 0 ) ;

    if ( !(id = GCL_GetOptionValue( 'h', 0 )) )
	PERROR( "No Brk-Hazard-ID specified in GCL : NewFortWallLight\n" ) ;
    if ( !(work->hzd = BRK_InitHazard( id )) )
	PERROR( "No such Brk-Hazard-ID<%d> : NewFortWallLight\n", id ) ;

    if ( GCL_GetOption( 'r' ) != NULL )
    {
	work->rot[X] = DEGtoANG( GCL_GetNextInt() ) ;
	work->rot[Y] = DEGtoANG( GCL_GetNextInt() ) ;
    }
    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	vu0_IV0toFV( &buf, &work->pos ) ;
    }

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No Model-ID specified in GCL : NewFortWallLight\n" ) ;
    if ( !(work->bf_def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "No BeforeBreak KMS<%d> in data.cnf!! : NewFortWallLight", id ) ;
    if ( !(work->af_def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "No afterBreak KMS<%d> in data.cnf!! : NewFortWallLight", id ) ;
    if ( !(work->dr_def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "No Drop KMS<%d> in data.cnf!! : NewFortWallLight", id ) ;
    if ( !(def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "No Gloom Comdl(kms)<%d> in data.cnf!! : NewFortWallLight", id ) ;
    if ( !(work->gloom = InitGloomComdl( work, def )) )
	PERROR( "No Memory for Gloom : NewFortLight" ) ;
    if ( FRT_WLGT_InitModel( work, work->bf_def ) < 0 )
	return -1 ;
    if ( FRT_WLGT_InitTarget( work, work->objs->objs, where ) < 0 )
	return -1 ;
    _sceVu0CopyVector( &work->pos_v, &DG_ZeroVector ) ;

    //OK_SetDynamicLight( &work->pos, &DG_ZeroSVector ) ;
    if ( !(work->flag & 0x0020) )
	FRT_AIM_AddAimSpot( &work->target, NULL, 0x0000, &work->pos, NULL, NULL, FRT_AIM_TypeWallLight ) ;

    return 0 ;
}

void *NewFortWallLight( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if ( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
