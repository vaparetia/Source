//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_fl_frame.c
   フォーチュン戦 フォークリフト上部フレームのふっ飛び

   2000/12/18 T.Morita
   $Id: brk_fl_stick.c,v 1.1.1.3 2002/11/19 11:46:14 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#define BRK_VEL_R        0.85f
#define BRK_ROT_R        8
#define BRK_ROT_PART_R   8
#define BRK_GRAVITY      6
#define BRK_BOUNCE       1.38f
#define BRK_BOUNCE_WALL  1.28f

#define BRK_FRM_N_MDL    7

typedef struct work_brk_t
{
    GV_ACT_EX    actor ;

    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR      size   ;
    FVECTOR      center ;
    FVECTOR      pos   ;
    FVECTOR      pos_v ;
    short        rot[XYZ]   ;
    short        rot_v[XY] ;

    DG_OBJ      *obj    ;

    short        se_tic ;
    short        se_vol ;

    int          type   ;
    int          flag   ;
    HZD_BOX     *hzd    ;
    int          where  ;
} BrkWork ;

#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)
static void RotateMatrix( FMATRIX *out, short rot[XY] )
{
    FMATRIX *in = &DG_UnitMatrix ;

    _sceVu0RotMatrixX( out, in , ANGtoRAD(rot[X]) ) ;
    _sceVu0RotMatrixZ( out, out, ANGtoRAD(rot[Z]) ) ;
    //_sceVu0RotMatrixZ( out, out, ANGtoRAD(DEGtoANG(90)) ) ;
    _sceVu0RotMatrixY( out, out, ANGtoRAD(rot[Y]) ) ;
}
static void TransMatrix( BrkWork *p )
{
    FMATRIX *mtx = &p->obj->world ;

    _sceVu0ApplyMatrix( (FVECTOR*)mtx->m[W], mtx, &p->center ) ;
    _sceVu0SubVector( (FVECTOR*)mtx->m[W], &p->pos, (FVECTOR*)mtx->m[W] ) ;
    mtx->m[W][W] = 1.0f ;
}

static void MakePosition( BrkWork *work )
{
    work->rot[X] += work->rot_v[X] = work->rot_v[X]*(BRK_ROT_PART_R-1)/BRK_ROT_PART_R ;
    work->rot[Y] += work->rot_v[Y] = work->rot_v[Y]*(BRK_ROT_PART_R-1)/BRK_ROT_PART_R ;
}



static inline void ApplyMatrixMinMax( FVECTOR *a, FMATRIX *m, FVECTOR *v,
				      FVECTOR *min, FVECTOR *max )
{
#ifdef BP_PSX2_ASM
    asm volatile  ("
    lqc2        vf1,0x00(%1)
    lqc2        vf2,0x00(%0)
    lqc2        vf3,0x10(%0)
    lqc2        vf4,0x20(%0)
    lqc2        vf5,0x00(%3)
    lqc2        vf6,0x00(%4)
    vmulax.xyz  ACC,vf2,vf1x
    vmadday.xyz ACC,vf3,vf1y
    vmaddz.xyz  vf2,vf4,vf1z
    vmini.xyz   vf5, vf5, vf2
    vmax.xyz    vf6, vf6, vf2
    sqc2        vf2,0x00(%2)
    sqc2        vf5,0x00(%3)
    sqc2        vf6,0x00(%4)
    " : : "r"(m), "r"(v), "r"(a), "r"(min), "r"(max) ) ;
#else
	_sceVu0ApplyMatrix( a, m, v ) ;
	MinMaxVector( min, max, a ) ;
#endif
}

static void SizeOfBound( FVECTOR *org, FMATRIX *m, FVECTOR *size )
{
    int     i ;
    FVECTOR f, min, max ;

    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;
    f.vw = 0.0f ;
    for ( i=8 ; --i>=0 ; )
    {
	f.vx = i&1 ? org->vx : -org->vx ;
	f.vy = i&2 ? org->vy : -org->vy ;
	f.vz = i&4 ? org->vz : -org->vz ;
	//ApplyMatrixXYZ( &f, m, &f ) ;
	_sceVu0ApplyMatrix( &f, m, &f ) ;
	if ( min.vx > f.vx ) min.vx = f.vx ;
	if ( min.vy > f.vy ) min.vy = f.vy ;
	if ( min.vz > f.vz ) min.vz = f.vz ;
	if ( max.vx < f.vx ) max.vx = f.vx ;
	if ( max.vy < f.vy ) max.vy = f.vy ;
	if ( max.vz < f.vz ) max.vz = f.vz ;
	//MinMaxVector( &min, &max, &f ) ;
    }
    _sceVu0SubVector( size, &max, &min ) ;
    _sceVu0ScaleVector( size, size, 0.5f ) ;
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
    RotateMatrix( &p->obj->world, p->rot ) ;
    if ( check_hzd )
    {
	SizeOfBound( &p->size, &p->obj->world, &size ) ;
	//BRK_UTL_SizeOfMDL( p->obj->model, &p->obj->world, &size ) ;
	flag = BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v, &bounce, &size ) ;
    }

    switch( flag )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R ) ;
	if ( BRK_UTL_PutCenterHazard( p->hzd, &p->pos, &p->pos_v, 1000.0f, 25.0f ) )
	{
	    p->pos_v.vy = 10.0f ;
	    p->rot_v[X] = irnd()&10 ? 200 : -200 ;
	    flag = 3 ;
	}
	else
	{
	    p->rot_v[X] = ( (p->rot[X]>0 ? DEGtoANG(90): -DEGtoANG(90)) - p->rot[X]) / 16 ;
	    flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;
	    if ( flag == 2 )
		if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		    p->rot_v[X]=0, flag = 1 ;
	}

	if ( p->se_tic > 0 )
	    p->rot_v[X]=0/*, flag = 1*/ ;
	else
	{
	    int pan ;
       float bp_angle;

	    pan = GM_SeGetPan( &p->pos, GM_SEMODE_BOMB, &bp_angle ) ;
	    if ( p->se_vol )
	    {
		p->se_vol = p->se_vol * 7 / 10 ;
		GM_SeSet3D( pan, p->se_vol, SD_A_FLAME001, bp_angle ) ;
	    }
	    else
	    {
		p->se_vol = GM_SeGetVol( &p->pos, GM_SEMODE_BOMB ) ;
		GM_SeSet3D( pan, p->se_vol, SD_A_FLAME001, bp_angle ) ;
	    }
	}
	p->se_tic = 5 ;
        break ;

    case 0:
	p->rot[X] += p->rot_v[X] ;
	p->rot[Y] += p->rot_v[Y] ;
	if ( p->type )
	    p->rot[Z] += (DEGtoANG(90) - p->rot[Z]) / 4 ;
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	break ;
    }
    TransMatrix( p ) ;
    //MakePosition( p ) ;
    if ( p->se_tic > 0 )
	p->se_tic-- ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( BrkWork *p )
{
    int     flag = 1, r ;
    int     hzd_flg ;
    FVECTOR size ;

    p->pos_v.vy = -BRK_GRAVITY*5 ;

    if ( p->type )
	p->rot[Z] += (DEGtoANG(90) - p->rot[Z]) / 4 ;
    p->rot[Y] += p->rot_v[Y] = p->rot_v[Y] * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->rot_v[X] += (p->rot[X]>0 ? DEGtoANG(5) : -DEGtoANG(5)) * (p->rot[X]/DEGtoANG(90) ? -1 : 1) ;
    r = p->rot[X] + p->rot_v[X] ;
    if ( (!(p->rot[X]/DEGtoANG(90)) &&  (r/DEGtoANG(90))) ||
	 ( (p->rot[X]/DEGtoANG(90)) && !(r/DEGtoANG(90))) /*||
	 p->rot[X] == DEGtoANG(90) || p->rot[X] == DEGtoANG(-90)*/ )
    {
        r = p->rot[X]>0 ? DEGtoANG(90) : DEGtoANG(-90) ;
	p->rot_v[X] = 0 ;
	if ( !p->rot_v[Y] )
	    flag = 0 ;
    }
    p->rot[X] = r ;

    RotateMatrix( &p->obj->world, p->rot ) ;
    SizeOfBound( &p->size, &p->obj->world, &size ) ;
    //BRK_UTL_SizeOfMDL( p->obj->model, &p->obj->world, &size ) ;
    hzd_flg = BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) ;
    if ( !(hzd_flg & 1) )
	p->pos.vy += p->pos_v.vy, flag = 1 ;
    TransMatrix( p ) ;
    MakePosition( p ) ;

    return flag ;
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
	    //GM_PutTarget( &work->target ) ;
	    //GM_MoveTarget2Map( &work->target, &work->obj->world, work->where ) ;
	}
	//GM_MoveTarget2Map( &work->target, &work->obj->world, work->where ) ;
	//AN_Test_Eye2( &work->pos, 2 ) ;
	break ;
    }
}


static int InitCenterSize( BrkWork *work, DG_OBJ *o )
{
    FVECTOR uv = { o->model->ux, o->model->uy, o->model->uz, 0 } ;
    FVECTOR lv = { o->model->lx, o->model->ly, o->model->lz, 0 } ;

    _sceVu0AddVector( &work->center, &uv, &lv ) ;
    _sceVu0ScaleVector( &work->center, &work->center, 0.5f ) ;
    _sceVu0SubVector( &work->size, &uv, &lv ) ;
    _sceVu0ScaleVector( &work->size, &work->size, 0.5f ) ;

#if DEBUG_MODE
#if 0
    {
	GM_SetTarget( &work->target, TARGET_OFFENSE|TARGET_ROTATE,
		      work->where, PLAYER_SIDE, &work->size, &work->center ) ;
	GM_MoveTarget2Map( &work->target, &work->obj->world, work->where ) ;
	NewTargetView( &work->target, 200, 50, 32 ) ;
    }
#endif
#endif

    return 0 ; 
}

#if 0
static int InitTarget( BrkWork *work, DG_OBJ *o )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR uv = { o->model->ux, o->model->uy, o->model->uz, 0 } ;
    FVECTOR lv = { o->model->lx, o->model->ly, o->model->lz, 0 } ;
    static FVECTOR PushForce = { 100.0f, 0.0f, 0.0f, 0.0f } ;

    t_size.vx = (uv.vx - lv.vx)*0.25f ;
    t_size.vy = (uv.vy - lv.vy)*0.25f ;
    t_size.vz = (uv.vz - lv.vz)*0.25f ;
    t_pos.vx  = (uv.vx + lv.vx)*0.25f + o->model->tx ;
    t_pos.vy  = (uv.vy + lv.vy)*0.25f + o->model->ty ;
    t_pos.vz  = (uv.vz + lv.vz)*0.25f + o->model->tz ;

    GM_SetTarget( t, TARGET_OFFENSE|TARGET_POWER, work->where, PLAYER_SIDE, &t_size, &t_pos ) ;
    GM_SetTargetWeaponType( t, WP_KICK ) ;
    GM_SetPowerTarget( t, p, POWER_EXPLODE, GM_Vitality, 10, 5, &PushForce ) ;
    GM_MoveTarget2Map( &work->target, &work->obj->world, work->where ) ;
    //NewTargetView( t, 200, 50, 32 ) ;

    return 0 ; 
}
#endif

static int GetResources( BrkWork *work, HZD_BOX *hzd, int type, 
			 DG_OBJ *obj,
			 short rot_x, short rot_y )
{
    work->obj   = obj ;
    //work->where = hide->work->where ;
    work->hzd   = hzd   ;
    work->flag  = 2+4 ;

#define BRK_POPUP  130.0f
#if 1
    work->pos_v.vx = work->obj->world.m[Z][X]*100.0f ;
    work->pos_v.vy = BRK_POPUP ;
    work->pos_v.vz = work->obj->world.m[Z][Z]*100.0f ;
    if ( GM_StagePlayTime & 1 )
	work->pos_v.vx *= -1.0f ;
    work->pos_v.vz = fpu_Abs( work->pos_v.vz ) ;
#else
    work->pos_v.vx = 0.0f;
    work->pos_v.vy = BRK_POPUP ;
    work->pos_v.vz = 0.0f ;
    {
	FVECTOR v = { -2400.0f, -43800.0f, 4500.0f, 1.0f } ;

	_sceVu0CopyVector( (FVECTOR *)work->obj->world.m[W], &v ) ;
    }
    
#endif
    work->rot[X] = rot_x ;
    work->rot[Y] = rot_y ;
    work->rot[Z] = 0 ;
    work->rot_v[X] = irnd()&1023 ;
    work->rot_v[Y] = irnd()&1023 ;

    work->type = type ;

    InitCenterSize( work, work->obj ) ;

    _sceVu0ApplyMatrix( &work->pos, &work->obj->world, &work->center ) ;
    _sceVu0AddVector( &work->pos,
		      (FVECTOR *)work->obj->world.m[W], &work->pos ) ;

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    BrkWork *work = pwork ;

    switch( signal )
    {
    case FRT_OBJ_Destroyed:
	/* 床より上にあり,消えていないもののみ動かす */
	if ( work->pos.vy > -44900.0f )
	{
	    work->pos_v.vy = 50.0f ;
	    work->flag = 2 ;
	}
	break ;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}


void *NewForkliftStick( HZD_BOX *hzd, int type, DG_OBJ *obj, short rot_x, short rot_y )
{
    BrkWork *work ;

    work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;
        GV_ActorEX( &work->actor ) ;
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

        if( GetResources( work, hzd, type, obj, rot_x, rot_y ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
