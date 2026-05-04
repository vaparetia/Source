//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_anydust.c
   フォーチュン戦  屑飛び

   2000/12/15 T.Morita
   $Id: brk_gas_cap.c,v 1.1.1.3 2002/11/19 11:46:14 Yoshizawa1 Exp $
*/
#include "fort_obj.h"


#define BRK_VEL_R        0.85f
#define BRK_ROT_R        64
#define BRK_GRAVITY      5.68f
#define BRK_BOUNCE       1.68f
#define BRK_BOUNCE_WALL  1.38f

typedef struct work_brk_t
{
    GV_ACT_EX   actor    ;

    FVECTOR pos   ;
    FVECTOR pos_v ;

    short   rot_x,  rot_y  ;
    short   rot_vx, rot_vy ;

    int          flag   ;
    DG_OBJS      *objs  ;
    HZD_BOX      *hzd   ;
    int           n_bound ;
} BrkWork ;



static void RotateMatrixXY( BrkWork *p )
{
    int r ;

    r = p->rot_x & 0x0fff ;
    _sceVu0RotMatrixX( &p->objs->world, &DG_UnitMatrix  , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = p->rot_y & 0x0fff ;
    _sceVu0RotMatrixY( &p->objs->world, &p->objs->world, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static void TransMatrix( BrkWork *p )
{
    _sceVu0CopyVectorXYZ( (FVECTOR *)p->objs->world.m[W], &p->pos ) ;
    //p->objs->world.m[W][Y] += p->push_up ;
}



static void ActSetSound( BrkWork *work )
{
    int     pan, vol ;
    float   dis ;
    FVECTOR v   ;

    /* 回転速度の符号が反転したら音がなる */
    if ( work->n_bound > 0 )
    {
	_sceVu0SubVector( &v, &GM_PlayerControl->mov, &work->pos ) ;
	dis = _sceVu0InnerProduct( &v, &v ) ;
	dis = sceVu0Sqrt( dis ) ;

	vol = (int)((work->n_bound*64/5) * 4000.0f / dis) ;
	vol = vol>64 ? 64 : vol ;

	if ( vol )
	{
      float bp_angle;
	    pan = GM_SeGetPan( &work->pos, GM_SEMODE_BOMB, &bp_angle ) ;
	    GM_SeSet3D( pan, vol, SD_A_DRUMCAN1, bp_angle ) ;
	}
	work->n_bound-- ;
    }
}



/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( BrkWork *work )
{
    int flag = 0 ;
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    FVECTOR size ;

    work->pos_v.vy -= BRK_GRAVITY ;
    RotateMatrixXY( work ) ;

    BRK_UTL_SizeOfBound( work->objs, &work->objs->world, &size ) ;
    flag = BRK_CheckHazard( work->hzd, &work->pos, &work->pos_v, &bounce, &size ) ;
    switch( flag )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &work->pos_v, &work->pos_v, BRK_VEL_R + frnd()*0.1f ) ;
	ActSetSound( work ) ;

	if ( BRK_UTL_PutCenterHazard( work->hzd, &work->pos, &work->pos_v, 3000.0f, 25.0f ) )
	{
//printf( "Illeagal Bounce x%.2f z%.2f\n", work->pos_v.vx,work->pos_v.vz ) ;
	    work->pos_v.vy = 10.0f ;
	    flag = 3 ;
	}
	else
	{
	    flag = (int)(work->pos_v.vy/16.0f) + 2 ;
	    work->rot_vx = -work->rot_vx * (BRK_ROT_R-1) / BRK_ROT_R ;
	    if ( flag == 2 )
		if ( !(int)work->pos_v.vx && !(int)work->pos_v.vz )
		    work->rot_vx=0, flag = 1 ;
	}
        break ;
    case 0:
	work->rot_x = (work->rot_x + work->rot_vx)&4095 ;
	work->rot_y = (work->rot_y + work->rot_vy)&4095 ;
	work->rot_x = work->rot_x>2048 ? work->rot_x-4096 : work->rot_x ;
	work->rot_y = work->rot_y>2048 ? work->rot_y-4096 : work->rot_y ;
        _sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
    }
    TransMatrix( work ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( BrkWork *work )
{
    int     i, r ;
    int     flag ;
    FVECTOR size ;

    /* 各軸の揺れ計算 */
    r = work->rot_vx + work->rot_x + (work->rot_x>1024 ? 2048 : work->rot_x<-1024 ?-2048 : 0) ;
    for ( i=4 ; r/2 && --i>=0 ; r/=2 ) ;
    work->rot_x += work->rot_vx -= r ;
    work->rot_y += work->rot_vy = work->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R ;

    /* 終了条件       */
    flag = !work->rot_vx && !work->rot_x && !work->rot_vy ? 0 : 2 ;

    /* 移動 */
    work->pos_v.vy = -0.5f * BRK_GRAVITY ;
    RotateMatrixXY( work ) ;
    BRK_UTL_SizeOfBound( work->objs, &work->objs->world, &size ) ;
    if ( !BRK_CheckHazard( work->hzd, &work->pos, &work->pos_v, &BRK_HZD_NoBounce, &size ) )
	work->pos.vy += work->pos_v.vy, flag = 1 ;
    TransMatrix( work ) ;

    return flag ;
}

static void Die( BrkWork *work )
{
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
}

static void Act( BrkWork *work )
{
    if ( work->flag )
	switch( work->flag )
	{
	case 2:
	    if ( MoveObject( work ) == 1 )
		work->flag = 1 ;
	    break ;
	case 1:
	    if ( !FallDownObject( work ) )
		work->flag = 0 ;
	case 0:
	    break ;
	default:
	    work->flag-- ;
	    break ;
	}
    //if ( !work->flag ) GV_DestroyActor( work ) ;
}

static int GetResources( BrkWork *work, HIDE *hide, FVECTOR *pos,
			 short rot_x, short rot_y, int flag )
{
    DG_DEF  *def ;
    static FVECTOR top    = { 0.0f,  500.0f, 0.0f, 1.0f } ;
    static FVECTOR bottom = { 0.0f, -100.0f, 0.0f, 1.0f } ;


    if ( !(def  = GV_GetCache( GV_CacheID( 9556851 /*w11c2_drum_futa.kms*/, 'k' ) )) )
	PERROR( "No KMS found in data.cnf!! :: NewForkliftFrame\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewForkliftFrame\n" ) ;
    DG_QueueObjs( work->objs ) ;

    work->n_bound  = 5 ;
    work->pos_v.vx = 50.0f*frnd() ;
    work->pos_v.vz = 50.0f*frnd() ;
    work->pos_v.vy = 150.0f ;
    work->rot_x    = rot_x ;
    work->rot_y    = rot_y ;
    work->rot_vx   = 100 ;
    work->rot_vy   = (irnd()&512)-256 ;

    work->hzd  = hide->work->hzd ;
    if ( pos )
    {
	work->flag = 0 ;
	_sceVu0CopyVector( &work->pos, pos ) ;
    }
    else
    {
	work->flag = 2 ;
	_sceVu0ApplyMatrix( &work->pos, &hide->objs->world,
			    flag ? &top : &bottom ) ;
    }
    RotateMatrixXY( work ) ;
    TransMatrix( work ) ;

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    BrkWork *work = pwork ;

    switch( signal )
    {
    case FRT_OBJ_Destroyed:
	if ( work->flag >= 0 )
	    if ( work->pos.vy > -44900.0f )
		work->flag = 2 ;
	if ( work->flag == 0 )
	    work->flag = -1 ;
	break ;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

int NewGasCap( HIDE *hide, FVECTOR *pos, short rot_x, short rot_y, int flag )
{
    BrkWork *work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;

    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

	if( GetResources( work, hide, pos, rot_x, rot_y, flag ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return 0 ;
	}
	return 1 ;
    }
    return 0 ;
}
