//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gastank.c
   フォーチュン戦 タンク壊れ

   2000/12/18 T.Morita
   $Id: brk_gastank.c,v 1.1.1.3 2002/11/19 11:46:14 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#define  BRK_N_WOODS 64 
#define  BRK_N_PIECE 64
#define  BRK_N_MOVES (BRK_N_WOODS+BRK_N_PIECE)

#define BRK_VEL_R        0.85f
#define BRK_ROT_R        16
#define BRK_GRAVITY      6
#define BRK_BOUNCE       1.38f
#define BRK_BOUNCE_WALL  1.38f
#define BRK_GAS_GRAVITY 6

static int GasSound[] = { SD_A_METLOTI1, SD_A_METLOTI2 } ;
static int n_GasSound = sizeof(GasSound) / sizeof(int) ;

typedef struct move_t
{
    FVECTOR pos   ;
    FVECTOR pos_v ;

    short   rot_x  ;
    short   rot_y  ;
    short   rot_vx ;
    short   rot_vy ;

    int          flag ;
    DG_OBJS     *objs ;
} MOVE ;


typedef struct work_brk_t
{
    GV_ACT_EX   actor ;

    MOVE        move  ;

    HZD_BOX    *hzd   ;
    int         where ;
    HIDE       *hide  ;
} BrkWork ;


static FVECTOR OilLocalPos = { 0.0f, 0.0f, -800.0f, 1.0f } ;



static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixX( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}
static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], pos ) ;
}

/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( HZD_BOX *hzd, MOVE *p )
{
    int flag = 0 ;
    FVECTOR size ;

    p->rot_x = (p->rot_x + p->rot_vx)&4095 ;
    p->rot_y = (p->rot_y + p->rot_vy)&4095 ;
    p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
    p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;
    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    BRK_UTL_SizeOfBound( p->objs, &p->objs->world, &size ) ;
    p->pos.vy += p->pos_v.vy -= BRK_GRAVITY ;
    if ( p->pos.vy - size.vy < -45000.0f )
    {
	p->pos.vy = -45000.0f + size.vy ;
	p->pos_v.vy *= -0.65f ;
	p->rot_vx = ( (p->rot_x>0 ? 1024 : -1024) - p->rot_x) / 16 ;
	flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;
	if ( flag == 2 )
	    if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		p->rot_vx=0, flag = 1 ;
    }
    TransMatrix( &p->objs->world, &p->pos ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( HZD_BOX *hzd, MOVE *p )
{
    int r, flag ;
    FVECTOR size ;

    r = p->rot_x + (p->rot_vx += ( p->rot_x/1024 ? -10 : 10) * (p->rot_x>0 ? 1 : -1) ) ;
    if ( (!(p->rot_x/1024) && r/1024) || (p->rot_x/1024 && !(r/1024)) )
        flag = 0, r = p->rot_x>0 ? 1024 : -1024 ;
    else
        flag = 2 ;
    p->rot_x = r ;
    p->rot_y += p->rot_vy = p->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R ;
    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    BRK_UTL_SizeOfBound( p->objs, &p->objs->world, &size ) ;
    p->pos.vy = -45000.0f + size.vy ;
    TransMatrix( &p->objs->world, &p->pos ) ;

    return flag ;
}

/* 揺れて 0°に落ち着く */
static int ShakingObject( HZD_BOX *hzd, MOVE *p )
{
#if 0
    int j, r ;
    FVECTOR size ;

    r = p->rot_vx + p->rot_x ;
    for ( j=4 ; r/2 && --j>=0 ; )
	r /= 2 ;
    p->rot_x += p->rot_vx -= r ;
    p->rot_y += p->rot_vy = p->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->pos_v.vy = -BRK_GRAVITY*5 ;
    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    BRK_UTL_SizeOfBound( p->objs, &p->objs->world, &size ) ;
    r = BRK_CheckHazard( hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) ;

    {
	extern int      BRK_HZD_Flag    ;
	extern float    BRK_HZD_HitDist ;
	extern float    BRK_HZD_Velocity ;
	printf( "%d FLG%x HitDist%f Vel%f Y%f VY%f\n", r, BRK_HZD_Flag,
		BRK_HZD_HitDist, BRK_HZD_Velocity, p->pos.vy,p->pos_v.vy ) ;
    }
    p->pos.vy += p->pos_v.vy ;
    TransMatrix( &p->objs->world, &p->pos ) ;

    if ( !p->rot_x && !p->rot_vx && !p->rot_vy && p->pos_v.vy<0.01f )
	return 0 ;
    return 1 ;
#else
	return 0 ;
#endif

}

static int ChangeModel( MOVE *m, HIDE *hide, int flag )
{
    FVECTOR v ;

    hide->type++ ;
    if ( FRT_OBJ_InitModel( hide, &hide->objs->world ) < 0 )
	PERROR( "Couldn't initialize New Model(%d) : NewGasTankBroken\n", --hide->type ) ;

    m->objs = hide->objs ;
    m->flag = flag ;

    _sceVu0CopyVector( &m->pos, (FVECTOR *)hide->objs->world.m[W] ) ;
    m->rot_x = hide->rot.vx ;
    m->rot_y = hide->rot.vy ;
    m->pos_v.vx = 0.0f ;
    m->pos_v.vz = 0.0f ;

    /* 無敵にする */
    hide->target.class |= TARGET_SKIP ;

    if ( m->flag == 2 )
    {
	m->pos_v.vy = 200.0f ;
	m->rot_vx = (irnd()&0x30)-64 ;
	m->rot_vy = (irnd()&0x18)-32 ;

	/* 蓋を飛ばす */
	NewGasCap( hide, NULL, m->rot_x, m->rot_y, 1 ) ;
	/* 破片を飛ばす */
	NewTankDust( hide, NULL, 8,
		     13849007,   /* GV_StrCode( "w11c2_drum_frg1_cm" ) */
		     BRK_GAS_GRAVITY,
		     50.0f, 60*10,
		     GasSound, n_GasSound ) ;
    }
    else
    {
	_sceVu0ApplyMatrix( &v, &hide->objs->world, &OilLocalPos ) ;
	if ( m->flag == 1 )
	    NewOilSpreadCalled( &v, 180, hide->work->where, -1 ) ;
	else
	    NewOilSpreadCalled( &v, 180, hide->work->where, 10*60*5/TIME_BASE ) ;
    }
    return 0 ;
}

static void Act( BrkWork *work )
{
    switch ( work->move.flag )
    {
    case 2:
	if ( MoveObject( work->hzd, &work->move ) == 1 )
	{
	    /* 落ちた音 */
	    //GM_SeSetMode( SD_A_DRUMCAN1, (FVECTOR *)work->hide->objs->world.m[W], GM_SEMODE_BOMB ) ;
	    work->move.flag = 1 ;
	}
	break ;
    case 1:
	if ( !FallDownObject( work->hzd, &work->move ) )
	{
	    /* 倒れる音 */
	    //GM_SeSetMode( SD_A_DRUMCAN1, (FVECTOR *)work->hide->objs->world.m[W], GM_SEMODE_BOMB ) ;

	    /* 当たりを復活させて死ぬ */
	    work->hide->target.class &= ~TARGET_SKIP ;
	    GV_DestroyActor( work ) ;
	}
	break ;

    case 0:
	if ( !ShakingObject( work->hzd, &work->move ) )
	    work->move.flag = -1 ;
	break ;

    default:
	if ( work->move.flag < -150*TIME_BASE/6 )
	{
	    GV_CallChildSignalFunc( work->hide->work,
				    FRT_OBJ_Destroyed,
				    (int)work->hide ) ;

	    ChangeModel( &work->move, work->hide, 2 ) ;
	    /* 風を出す(ライトを揺らす) */
	    FRT_LGT_SetMagnitude( &work->move.pos, 8000.0f ) ;
	    /* パッドを振動させる */
	    NewFortVibrate( 192-32, 12*5/TIME_BASE, 15*5/TIME_BASE ) ;
	    /* カメラを揺らす */
	    NewShakeCamera( 0, 256, 30 ) ;
	    /* 爆発エフェクトと攻撃ターゲット出す */
	    if ( FRT_OBJ_GetTypeAimFlag( work->hide ) & FRT_AIM_TypeGasCan )
		NewFortBlast( &work->move.pos,
			      PLAYER_SIDE, 2000.0f, 1500.0f, 10, 10, WP_Grenade, 3 ) ;
	    else
		NewFortBlast( &work->move.pos,
			      PLAYER_SIDE, 2500.0f, 2000.0f, 25, 10, WP_Grenade, 3 ) ;
	}
	work->move.flag-- ;
	break ;
    }
}

static int GetResources( BrkWork *work, HIDE *hide, TARGET *trgt, int flag )
{
    work->where = hide->work->where ;
    work->hzd   = hide->work->hzd   ;
    work->hide  = hide ;

    if ( ChangeModel( &work->move, work->hide, flag ) < 0 )
	return -1 ;

    /* 貫通音 */
    GM_SeSetMode( SD_A_METLANA1, (FVECTOR *)hide->objs->world.m[W], GM_SEMODE_BOMB ) ;

    return 0 ;
}

int NewGasTankBroken( HIDE *hide, TARGET *trgt, int flag )
{
    BrkWork *work ;

    work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;
        GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;

        if( GetResources( work, hide, trgt, flag ) < 0 )
            GV_DestroyActor( work ) ;
    }
    return 0 ;
}


int NewBombTankBroken( HIDE *hide, TARGET *trgt, int flag )
{
    BrkWork *work ;

    work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;
        GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;

        if( GetResources( work, hide, trgt, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return 0 ;
        }
	return 1 ;
    }
    return 0 ;
}


int NewGasBottomBroken( HIDE *hide, TARGET *trgt, int flag )
{
    /* 蓋を飛ばす */
    NewGasCap( hide, NULL, hide->rot.vx, hide->rot.vy, 0 ) ;
    /* 破片を飛ばす */
    NewTankDust( hide, trgt, 8,
		 13849007,   /* w11c2_drum_frg1_cm */
		 BRK_GAS_GRAVITY,
		 50.0f, 60*10,
		 GasSound, n_GasSound ) ;
    /* 貫通音 */
    GM_SeSetMode( SD_A_METLANA1, (FVECTOR *)hide->objs->world.m[W], GM_SEMODE_BOMB ) ;

    FRT_OBJ_FreeObject( hide, 1 ) ;
    hide->type++ ;

    return 1 ;
}

int BRK_FRT_OBJ_BreakInitBombTank( HIDE *hide, FVECTOR *pos )
{
    FVECTOR c_pos ;

    /* オイルを出す */
    _sceVu0ApplyMatrix( &c_pos, &hide->objs->world, &OilLocalPos ) ;
    NewOilStained( &c_pos, hide->work->where, -1 ) ;

    /* タンクを傾ける */
#if 0
    _sceVu0CopyVector( &c_pos, pos ) ;
    c_pos.vy -= 300.f ;
    if ( hide->objs )
    {
	RotateMatrixXY( &hide->objs->world, &DG_UnitMatrix,
			1024, irnd()&4095 ) ;
	TransMatrix( &hide->objs->world, &c_pos ) ;
    }
#else
    FRT_OBJ_FreeObject( hide, 1 ) ;
#endif

    /* 蓋を置く */
    c_pos.vx += 500.0f*frnd() ;
    c_pos.vy += 300.f ;
    c_pos.vz += 500.0f*frnd() ;
    NewGasCap( hide, &c_pos, irnd()&2048, irnd()&4095, 1 ) ;

    return  1 ;
}

int BRK_FRT_OBJ_BreakInitGasTank( HIDE *hide, FVECTOR *pos )
{
    FVECTOR c_pos ;
    FMATRIX mtx ;
    int i ;

    /* オイルを出す */
    i = hide->rot.vy & 0x0fff ;
    _sceVu0RotMatrixY( &mtx, &DG_UnitMatrix,
		       (i-4096*(i>>11))*(float)M_PI/2048.0F ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)mtx.m[W], pos ) ;
    _sceVu0ApplyMatrix( &c_pos, &mtx, &OilLocalPos ) ;
    NewOilStained( &c_pos, hide->work->where, 0 /*10*60*5/TIME_BASE*/ ) ;

    /* タンクを傾ける */
    _sceVu0CopyVector( &c_pos, pos ) ;
    c_pos.vy -= 300.f ;
    if ( hide->objs )
    {
	RotateMatrixXY( &hide->objs->world, &DG_UnitMatrix,
			1024, irnd()&4095 ) ;
	TransMatrix( &hide->objs->world, &c_pos ) ;
    }

    NewAnythingDustScaleInit( hide, pos,
			      6 , 13849007,   /* w11c2_drum_frg1_cm*/
			      1.0f ) ;
    

    /* 蓋を置く */
    c_pos.vx += 500.0f*frnd() ;
    c_pos.vy = -45000.0f + 50.f ;
    c_pos.vz += 500.0f*frnd() ;
    NewGasCap( hide, &c_pos, irnd()&2048, irnd()&4095, 1 ) ;

    return  1 ;
}

int BRK_FRT_OBJ_BreakInitGasTankBroken( HIDE *hide, FVECTOR *pos )
{
    FVECTOR c_pos ;

    BRK_FRT_OBJ_BreakInitGasTank( hide, pos ) ;
    
    /* 蓋を置く */
    _sceVu0CopyVector( &c_pos, pos ) ;
    c_pos.vx += 500.0f*frnd() ;
    c_pos.vy = -45000.0f + 50.f ;
    c_pos.vz += 500.0f*frnd() ;
    NewGasCap( hide, &c_pos, irnd()&2048, irnd()&4095, 1 ) ;

    return 0 ;
}
