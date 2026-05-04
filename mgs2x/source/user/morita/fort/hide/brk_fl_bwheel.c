//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_fl_bwheel.c
   フォーチュン戦 フォークリフト後輪のふっ飛び

   2000/12/18 T.Morita
   $Id: brk_fl_bwheel.c,v 1.1.1.3 2002/11/19 11:46:13 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_GRAVITY      6
#define BRK_BOUNCE       1.68f
#define BRK_BOUNCE_WALL  1.38f

typedef struct work_brk_t
{
    GV_ACT_EX   actor ;

    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR      pos   ;
    FVECTOR      pos_v ;
    short        rot[XY]   ;
    short        rot_v[XY] ;
    int          flag ;
    DG_OBJS     *objs ;
    FVECTOR     *trgt ;

    HZD_BOX    *hzd   ;
    int         where ;
    HIDE       *hide  ;
} BrkWork ;


static void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot[XY] )
{
    int r ;
    r = rot[X] & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot[Y] & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0CopyVectorXYZ( (FVECTOR*)&mtx->m[W], pos ) ;
}

/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( BrkWork *p, int check_hzd )
{
    int flag = 0 ;
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    FVECTOR size ;

    p->pos_v.vy -= BRK_GRAVITY ;

    p->rot[X] = (p->rot[X] + p->rot_v[X])&4095 ;
    p->rot[Y] = (p->rot[Y] + p->rot_v[Y])&4095 ;
    p->rot[X] = p->rot[X]>2048 ? p->rot[X]-4096 : p->rot[X] ;
    p->rot[Y] = p->rot[Y]>2048 ? p->rot[Y]-4096 : p->rot[Y] ;
    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot ) ;
    if ( check_hzd )
    {
	BRK_UTL_SizeOfBound( p->objs, &p->objs->world, &size ) ;
	flag = BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v, &bounce, &size ) ;
    }
    switch( flag )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R ) ;
	if ( BRK_UTL_PutCenterHazard( p->hzd, &p->pos, &p->pos_v, 1000.0f, 25.0f ) )
	{
	    p->pos_v.vy = 30.0f ;
	    p->rot_v[X] = irnd()&10 ? 200 : -200 ;
	    flag = 3 ;
	}
	else
	{
	    p->rot_v[X] = ( (p->rot[X]>0 ? 1024 : -1024) - p->rot[X]) / 16 ;
	    flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;
	    if ( flag == 2 )
		if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		    p->rot_v[X]=0, flag = 1 ;
	}
        break ;
    case 0:
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	break ;
    }
    TransMatrix( &p->objs->world, &p->pos ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( BrkWork *p )
{
    int r, flag ;
    FVECTOR size ;

    r = p->rot[X] + (p->rot_v[X] += ( p->rot[X]/1024 ? -10 : 10) * (p->rot[X]>0 ? 1 : -1) ) ;
    if ( (!(p->rot[X]/1024) && r/1024) || (p->rot[X]/1024 && !(r/1024)) )
        flag = 0, r = p->rot[X]>0 ? 1024 : -1024 ;
    else
        flag = 2 ;
    p->rot[X] = r ;
    p->rot[Y] += p->rot_v[Y] = p->rot_v[Y] * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->pos_v.vy = -BRK_GRAVITY*5 ;
    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot ) ;
    BRK_UTL_SizeOfBound( p->objs, &p->objs->world, &size ) ;
    if ( !BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) )
	p->pos.vy += p->pos_v.vy, flag = 1 ;
    TransMatrix( &p->objs->world, &p->pos ) ;

    return flag ;
}



static void Die( BrkWork *work )
{
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
}

static void Act( BrkWork *work )
{
    switch ( work->flag )
    {
    case 1:
	if ( !FallDownObject( work ) )
	    work->flag = 0 ;
    case 0:
	break ;
    default:
	if ( work->flag > 2 )
	{
	    work->flag-- ;
	    MoveObject( work, 0 ) ;
	}
	else if ( MoveObject( work, 1 ) == 1 )
	    work->flag = 1 ;
	else
	{
	    GM_PutTarget( &work->target ) ;
	    GM_MoveTarget2Map( &work->target, &work->objs->world, work->where ) ;
	}
	break ;
    }
}

static int InitTarget( BrkWork *work, DG_OBJS *o )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR uv = { o->def->ux, o->def->uy, o->def->uz, 0 } ;
    FVECTOR lv = { o->def->lx, o->def->ly, o->def->lz, 0 } ;

    t_size.vx = (uv.vx - lv.vx)*0.25f ;
    t_size.vy = (uv.vy - lv.vy)*0.25f ;
    t_size.vz = (uv.vz - lv.vz)*0.25f ;
    t_pos.vx  = (uv.vx + lv.vx)*0.25f + o->def->tx ;
    t_pos.vy  = (uv.vy + lv.vy)*0.25f + o->def->ty ;
    t_pos.vz  = (uv.vz + lv.vz)*0.25f + o->def->tz ;

    GM_SetTarget( t, TARGET_OFFENSE, work->where, PLAYER_SIDE, &t_size, &t_pos ) ;
    GM_SetTargetWeaponType( t, WP_KICK ) ;
    GM_SetPowerTarget( t, p, POWER_ONCE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_MoveTarget2Map( &work->target, &work->objs->world, work->where ) ;

    //NewTargetView( t, 200, 50, 32 ) ;

    return 0 ; 
}

static int GetResources( BrkWork *work, HIDE *hide, FVECTOR *pos, int id, FVECTOR *trgt )
{
    DG_DEF *def ;
    float d ;

    if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS found in data.cnf!! :: NewPutVanimeObject\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewPutVanimeObject\n" ) ;
    DG_QueueObjs( work->objs ) ;

    work->where = hide->work->where ;
    work->hzd   = hide->work->hzd   ;
    work->hide  = hide ;
    work->flag  = 2+15 ;

#define BRK_POPUP 100.0f
#define BRK_SPEED 50.0f
    _sceVu0ApplyMatrix( &work->pos, &work->hide->objs->world, pos ) ;
    work->pos_v.vx = trgt->vx - pos->vx ;
    work->pos_v.vz = trgt->vz - pos->vz ;
    d = BRK_SPEED / sceVu0Sqrt( work->pos_v.vx*work->pos_v.vx + work->pos_v.vz*work->pos_v.vz ) ;
    work->pos_v.vx *= d ;
    work->pos_v.vz *= d ;
    work->pos_v.vy = BRK_POPUP ;

    work->rot_v[X] = (irnd()&0x70)-64 ;
    work->rot_v[Y] = (irnd()&0x38)-32 ;
    RotateMatrixXY( &work->objs->world, &DG_UnitMatrix, work->rot ) ;
    TransMatrix( &work->objs->world, &work->pos ) ;

    InitTarget( work, work->objs ) ;

    return 0 ;
}

int NewForkliftRearWheel( HIDE *hide, FVECTOR *pos, int id, FVECTOR *trgt )
{
    BrkWork *work ;

    work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;

        if( GetResources( work, hide, pos, id, trgt ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return 0 ;
        }
    }
    return 1 ;
}
