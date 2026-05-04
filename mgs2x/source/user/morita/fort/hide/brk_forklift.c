//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gastank.c
   フォーチュン戦 タンク壊れ

   2000/12/18 T.Morita
   $Id: brk_forklift.c,v 1.1.1.3 2002/11/19 11:46:14 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#include "BP_Misc.h"

typedef struct work_brk_t
{
    GV_ACT_EX  actor ;

    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR    pos   ;
    FVECTOR    pos_v ;
    short      rot[XYZ] ;
    short      rot_v[XYZ] ;

    int        type  ;
    int        flag  ;
    int        where ;
    int        used  ;
    HIDE      *hide  ;
    HZD_BOX   *hzd   ;
} BrkWork ;


#define ANGtoDEG(_a) ((int)(_a)*45/8192)
#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

static void RotateTransMatrix( BrkWork *work )
{
    FMATRIX *mtx = &work->hide->objs->world ;

    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(work->rot[Y]) ) ;
    _sceVu0RotMatrixX( mtx, mtx           , ANGtoRAD(work->rot[X]) ) ;
    _sceVu0RotMatrixZ( mtx, mtx           , ANGtoRAD(work->rot[Z]) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], &work->pos ) ;
}

static inline void RotateMatrix( BrkWork *work )
{
    FMATRIX *mtx = &work->hide->objs->world ;

    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(work->rot[Y]) ) ;
    _sceVu0RotMatrixX( mtx, mtx           , ANGtoRAD(work->rot[X]) ) ;
    _sceVu0RotMatrixZ( mtx, mtx           , ANGtoRAD(work->rot[Z]) ) ;
}

static inline void TransMatrix( BrkWork *work )
{
    FMATRIX *mtx = &work->hide->objs->world ;

    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], &work->pos ) ;
}


#define BRK_GRAVITY      6
#define FALL_ROT_MAX DEGtoANG(90)
#define FALL_ROT_V   16

#define TILT_ROT_MAX 710
#define TILT_ROT_V   32
#define FLAT_ROT_MAX 1200
#define FLAT_STEP    4
#define FLAT_RATE    (TILT_ROT_MAX/FLAT_STEP)
#define FLAT_POS_VY  (10.0f/FLAT_RATE)
#define FLAT_ROT_VX  ((FLAT_ROT_MAX-TILT_ROT_MAX)/FLAT_RATE)
#define FLAT_ROT_VZ  FLAT_STEP


static FVECTOR SparkPos0  = { -200.0f,  1000.0f,  620.0f, 1.0f } ;
static FVECTOR SparkVel0  = {    0.0f,   100.0f,   20.5f, 1.0f } ;
static FVECTOR SparkPos1  = { -500.0f,  1000.0f, -620.0f, 1.0f } ;
static FVECTOR SparkVel1  = {   50.0f,   100.0f,  -20.5f, 1.0f } ;
static FVECTOR SparkPos2  = { 1700.0f,  1000.0f, -620.0f, 1.0f } ;
static FVECTOR SparkVel2  = {   40.0f,    80.0f,    8.5f, 1.0f } ;

static FVECTOR SideSmokePos1 = { -300.0f,  2000.0f, -620.0f, 1.0f } ;
static FVECTOR SideSmokeDir1 = {  -10.0f,     0.0f,    3.5f, 1.0f } ;
static FVECTOR SideSmokePos2 = {    0.0f,  1700.0f, -620.0f, 1.0f } ;
static FVECTOR SideSmokeDir2 = {    0.0f,     0.0f,   10.5f, 1.0f } ;
static FVECTOR SideSmokePos3 = {  300.0f,  2000.0f, -620.0f, 1.0f } ;
static FVECTOR SideSmokeDir3 = {   10.0f,     0.0f,    3.5f, 1.0f } ;

static FVECTOR SideSmokePos4 = { -800.0f,  -100.0f, -620.0f, 1.0f } ;
static FVECTOR SideSmokeDir4 = {   10.0f,     0.0f,    0.0f, 1.0f } ;

static FVECTOR SmokePos1 = { -340.0f,  730.0f, -500.0f, 1.0f } ;
static FVECTOR SmokeDir1 = {   10.0f,     0.0f,    3.5f, 1.0f } ;

void BRK_FRT_Spark( FVECTOR *sp_pos, FVECTOR *sp_vel, HIDE *hide )
{
    FVECTOR pos ;
#if 0
    FVECTOR dir ;

    _sceVu0ApplyMatrix( &pos, &hide->objs->world, sp_pos ) ;
    _sceVu0ApplyMatrix( &dir, &hide->objs->world, sp_vel ) ;
    NewFortSpark( &pos, &dir, hide->work->hzd, 0 ) ;
#else
    _sceVu0ApplyMatrix( &pos, &hide->objs->world, sp_pos ) ;
    NewFortSpark( &pos, sp_vel, hide->work->hzd, 0 ) ;
#endif
}

void BRK_FRT_SideDust( FVECTOR *sp_pos, FVECTOR *sp_vel, HIDE *hide, int n_dust )
{
    FVECTOR pos ;

    _sceVu0ApplyMatrix( &pos, &hide->objs->world, sp_pos ) ;
    NewFortSideDust( &pos, sp_vel, n_dust ) ;
}

static inline void BRK_FRT_Smoke( FVECTOR *sp_pos, FVECTOR *sp_vel, HIDE *hide )
{
    GV_SetActorChild( hide->work,
		      NewFortSmoke( &hide->objs->world, sp_pos, sp_vel, NULL ) ) ;
}

static void BRK_LFT_ActBreak_B( BrkWork *work )
{
    switch( work->flag )
    {
    case 1:
	/*
	work->pos.vy -= FLAT_POS_VY ;
	work->rot[X] += FLAT_ROT_VX ;
	work->rot[Z] -= FLAT_ROT_VZ ;
	if ( work->rot[Z] <= 0 )*/
	{
	    //work->rot[X] = FLAT_ROT_MAX ;
	    //work->rot[Z] = 0 ;
	    work->flag   = 0 ;
	    work->used   = 2 ;
	}
	break ;

    case 3:
	work->rot[X] += work->rot_v[X] += TILT_ROT_V ;
	if ( work->rot[X] > TILT_ROT_MAX )
	{
	    work->rot[X] = TILT_ROT_MAX ;
	    work->rot_v[X] = -work->rot_v[X]/4 ;

	    /* 風を出す(ライトを揺らす) */
	    FRT_LGT_SetMagnitude( &work->pos, 6000.0f ) ;
	    /* カメラを揺らす */
	    NewShakeCamera( 0, 512, 10 ) ;
	    GV_CallChildSignalFunc( work->hide->work, FRT_OBJ_FlatTire, 0 ) ;

	    printf( "ForkLiftB: r(%d %d %d) p(%f %f %f)\n",ANGtoDEG( work->rot[X] ),ANGtoDEG( work->rot[Y] ),ANGtoDEG( work->rot[Z] ),work->pos.vx,work->pos.vy,work->pos.vz ) ;

	    work->flag = 1 ;
	}
	work->rot[Z] = work->rot[X] ;
	break ;
    }
    RotateTransMatrix( work ) ;
}

static void BRK_LFT_ActBreak_C( BrkWork *work )
{
    if ( work->flag )
    {
	float vz = -cosf( ANGtoRAD(work->rot[X]) ) ;
	float vy = -sinf( ANGtoRAD(work->rot[X]) ) ;

	/* エフェクトを出す */
	if ( work->rot[X] < DEGtoANG(62) )
	{
	    if ( work->rot[X] > DEGtoANG(58) )
		BRK_FRT_SideDust( &SideSmokePos2, &SideSmokeDir2,
				  work->hide, 8 ) ;
	    else if ( work->rot[X] > DEGtoANG(54) )
		BRK_FRT_Spark( &SparkPos1, &SparkVel1, work->hide ) ;
	    else if ( work->rot[X] > DEGtoANG(50) )
		BRK_FRT_Spark( &SparkPos2, &SparkVel2, work->hide ) ;
	    else if ( work->rot[X] > DEGtoANG(48) )
		if ( work->used == 0 )
		    work->used = 1 ;
	}

	/* 倒れる角度の計算 */
	if ( work->rot[Z] < 0 )
	    work->rot[Z] -= 2 ;
	else
	    work->rot[Z] = 0 ;
	work->rot[X] += work->rot_v[X] += FALL_ROT_V ;

	/* 倒れた時の弾み処理 */
	if ( work->rot[X] > FALL_ROT_MAX )
	{
	    /* 最初の倒れた時のみ */
	    if ( work->flag == 3 )
	    {
		/* 倒れた音 */
		GM_SeSetMode( SD_A_FLAME002,
			      (FVECTOR *)work->hide->objs->world.m[W],
			      GM_SEMODE_BOMB );
		/* パッドを振動させる */

   if ( BP_IsPAL()==TRUE )
		NewFortVibrate( 112-32, 4*5/TIME_BASE, 15*5/TIME_BASE ) ;
   else
		NewFortVibrate( 160-32, 6*5/TIME_BASE, 15*5/TIME_BASE ) ;

		work->flag++ ;
	    }

	    work->rot[X] = FALL_ROT_MAX ;
	    work->rot_v[X] = -work->rot_v[X]/8 ;
	    if ( !(work->rot_v[X]/(FALL_ROT_V/2)) )
	    {
		/* 風を出す(ライトを揺らす) */
		FRT_LGT_SetMagnitude( &work->pos, 10000.0f ) ;
		/* カメラを揺らす */
		NewShakeCamera2( 0, 128, 3, &work->pos ) ;

		/* 埃エフェクト */
		BRK_FRT_SideDust( &SideSmokePos1, &SideSmokeDir1, work->hide, 3 ) ;
		BRK_FRT_SideDust( &SideSmokePos3, &SideSmokeDir3, work->hide, 3 ) ;

#if DEBUG_MODE		
		printf( "ForkLiftC: r(%d %d %d) p(%f %f %f)\n",ANGtoDEG( work->rot[X] ),ANGtoDEG( work->rot[Y] ),ANGtoDEG( work->rot[Z] ),work->pos.vx,work->pos.vy,work->pos.vz ) ;
#endif

		work->rot[Z] = 0 ;
		work->flag   = 0 ;/* アクトを止める */
	    }
	}

	vz += cosf( ANGtoRAD(work->rot[X]) ) ;
	vy += sinf( ANGtoRAD(work->rot[X]) ) ;
	work->pos.vz -= vz * /*542.25f*/100.0f ;
	work->pos.vy += vy * 410.0f ;
#if 0
	{
	    int  i ;
	    for ( i=3 ; --i>=0 ; )
		work->hide->segs[i]->atr |= HZX_SEG_SKIP ;
	}
#else
	/* ハザードを動かす */
	{
	    IVECTOR shift = { 0, 0, (int)(-vz*600.0f), 0 } ;
	    int  i ;
	    
	    for ( i=3 ; --i>=0 ; )
		HZX_ShiftDynamicSegment( work->hide->segs[i], &shift ) ;
	}
#endif
    }
    RotateTransMatrix( work ) ;
}

static void BRK_LFT_ActBreak_D( BrkWork *work )
{
    static struct
    {
	FVECTOR pos ;
	int     rot_vy, rot_vz ;
    } Target[] = {
	{ {-4187.0f,-44056.0f,-191.0f, 140.0f}, DEGtoANG(30), DEGtoANG(45) },
	{ {-4860.0f,-44076.0f,   0.0f,  60.0f}, DEGtoANG( 0), DEGtoANG(60) },
	{ {-5586.0f,-43926.0f,-174.0f,  40.0f}, DEGtoANG(10), DEGtoANG(95) },
	{ {-5586.0f,-43926.0f,-114.0f, 120.0f}, DEGtoANG(10), DEGtoANG(95) },
    } ;
    float d ;

    switch( work->flag )
    {
    case 7:
	NewShakeCamera2( 0, 256, 8, &work->pos ) ;
	BRK_FRT_SideDust( &SideSmokePos4, &SideSmokeDir4, work->hide, 5 ) ;
	printf( "ForkLiftD: r(%d %d %d) p(%f %f %f)\n",ANGtoDEG( work->rot[X] ),ANGtoDEG( work->rot[Y] ),ANGtoDEG( work->rot[Z] ),work->pos.vx,work->pos.vy,work->pos.vz ) ;
	work->used = 1 ;
	work->flag = 8 ;
    case 8:
	if ( work->used == 2 )
	    work->flag = 0 ;
    case 0:
	break ;
    case 3:
	work->pos_v.vw = 0.0f ;
    default:
	if ( work->pos_v.vw <= 0.0f )
	{
	    /* このときだけ高く上がっているのでカメラを揺らす */
	    if ( work->flag == 4 )
		NewShakeCamera2( 0, 256, 8, &work->pos ) ;

	    /* ドシーーンと音がなる */
	    GM_SeSetMode( SD_A_FLAME002, &work->pos, GM_SEMODE_BOMB ) ;

	    /* 位置を合わせる */
	    if ( work->flag > 3 )
		_sceVu0CopyVectorXYZ( &work->pos, &Target[work->flag-4].pos ) ;

	    work->pos_v.vy = Target[work->flag-3].pos.vw ;
	    d = work->pos_v.vy * work->pos_v.vy - 
		2.0f * (Target[work->flag-3].pos.vy - work->pos.vy) * BRK_GRAVITY ;
	    work->pos_v.vw = (work->pos_v.vy + sceVu0Sqrt( d )) / BRK_GRAVITY -1.0f ;
	    work->pos_v.vx = (Target[work->flag-3].pos.vx - work->pos.vx) / work->pos_v.vw ;
	    work->pos_v.vz = (Target[work->flag-3].pos.vz - work->pos.vz) / work->pos_v.vw ;
	    work->rot_v[X] = 0 ;
	    work->rot_v[Y] = (short)((Target[work->flag-3].rot_vy - work->rot[Y]) / work->pos_v.vw) ;
	    work->rot_v[Z] = (short)((Target[work->flag-3].rot_vz - work->rot[Z]) / work->pos_v.vw) ;
	    work->flag++ ;
	}
	else
	{
	    work->pos_v.vy -= BRK_GRAVITY ;
	    work->rot[X] += work->rot_v[X] ;
	    work->rot[Y] += work->rot_v[Y] ;
	    work->rot[Z] += work->rot_v[Z] ;
	    work->pos_v.vw -= 1.0f ;
	    _sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
	}
	RotateMatrix( work ) ;
	TransMatrix( work ) ;

	GM_MoveTarget2Map( &work->target, &work->hide->objs->world, work->where ) ;
	GM_PutTarget( &work->target ) ;
    }
}

static void Act( BrkWork *work )
{
    switch( work->type )
    {
    case FRT_TYP_FORKLIFT_B:
	if ( work->type == FRT_OBJ_GetTypeMasked( work->hide ) )
	    BRK_LFT_ActBreak_B( work ) ;
	else
	    work->used = 2, work->flag = 0 ;
	break ;

    case FRT_TYP_FORKLIFT_C:
	if ( work->type == FRT_OBJ_GetTypeMasked( work->hide ) )
	    BRK_LFT_ActBreak_C( work ) ;
	else if ( work->used == 0 )
	    work->used = 1, work->flag = 0 ;
	break ;

    case FRT_TYP_FORKLIFT_D:
	BRK_LFT_ActBreak_D( work ) ;
	break ;
    }

    switch( work->used )
    {
    case 1:
	if ( !PL_PlayerResetInvincible() )
	    break ;
	GM_PutTarget( &work->target ) ;

    case 3:
	if ( work->hide->procs[FRT_OBJ_P_CHANGED] )
	    GCL_ExecProc( work->hide->procs[FRT_OBJ_P_CHANGED], NULL ) ;
	work->used = 2 ;

    case 2:
	if ( !work->flag )
	    GV_DestroyActor( work ) ;
    }
}


static int InitTarget( BrkWork *work, int type, FVECTOR *force )
{
    FVECTOR  t_pos ;
    DG_OBJS      *o =  work->hide->objs ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR uv = { o->def->ux, o->def->uy, o->def->uz, 0 } ;
    FVECTOR lv = { o->def->lx, o->def->ly, o->def->lz, 0 } ;
    static FVECTOR TargetSize = { 1000.0f, 1000.0f, 1000.0f, 0.0f } ;

    _sceVu0AddVector( &t_pos , &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
    t_pos.vx += o->def->tx ;
    t_pos.vy += o->def->ty ;
    t_pos.vz += o->def->tz ;

    GM_SetTarget( t, TARGET_OFFENSE|TARGET_POWER, work->where, PLAYER_SIDE,
		  &TargetSize, &t_pos ) ;
    GM_SetTargetWeaponType( t, WP_KICK ) ;
    GM_SetPowerTarget( t, p, type, GM_Vitality, 20, 20, force ) ;
    GM_MoveTarget2Map( t, &work->hide->objs->world, work->where ) ;

    //NewTargetView( t, 200, 50, 32 ) ;

    return 0 ; 
}

static FVECTOR BRK_FRT_OBJ_MainLift    = {  780.0f,   950.0f, 0.0f, 1.0f } ;
static FVECTOR BRK_FRT_OBJ_OilPoolPos1 = {-1500.0f,-44995.0f, 0.0f, 1.0f } ;

static int GetResources( BrkWork *work, HIDE *hide, TARGET *trgt, int flag )
{
    work->where = hide->work->where ;
    work->hide  = hide ;

    _sceVu0CopyVector( &work->pos, (FVECTOR *)hide->objs->world.m[W] ) ;
    work->rot[X] = hide->rot.vx ;
    work->rot[Y] = hide->rot.vy ;
    work->rot[Z] = hide->rot.vz ;
    work->flag = 3 ;

    hide->type++ ;
    work->type = FRT_OBJ_GetTypeMasked( hide ) ;
    if ( FRT_OBJ_InitModel( hide, &hide->objs->world ) < 0 )
	PERROR( "Couldn't initialize New Model(%d) : NewForkliftBrokenUpper\n", --hide->type ) ;
    else if ( FRT_OBJ_GetTypeMasked( hide ) == FRT_TYP_FORKLIFT_B )
    {
	static FVECTOR FrontWheel = {  605.0f,   170.0f,  420.0f, 1.0f } ;
	static FVECTOR RearWheel  = { -625.0f,    90.0f,  420.0f, 1.0f } ;
	static FVECTOR Frame      = { -240.0f,  1810.0f,    0.0f, 1.0f } ;
	static FVECTOR Handle     = {   64.0f,  1100.0f,    0.0f, 1.0f } ; //rotZ:32 ;
	static FVECTOR RearTarget = {-5500.0f,-45000.0f, 1000.0f, 0.0f } ;

	GM_CurrentMap = work->where ;
	NewIronBoxDust( hide, trgt, 20 ) ;

	NewForkliftFrontWheel( hide, &FrontWheel, GV_StrCode( "w11c2_forklift_fwheel" ), FLAT_RATE ) ;
	NewForkliftRearWheel( hide, &RearWheel, GV_StrCode( "w11c2_forklift_bwheel" ), &RearTarget ) ;
	NewForkliftFrame( hide, &Frame, GV_StrCode( "w11c2_forklift_frame" ), trgt ) ;
	/*NewForkliftHandle( hide, &Handle, GV_StrCode( "w11c2_forklift_handle" ), 32 ) ;*/

	/* パッドを振動させる */
	NewFortVibrate( 128-32, 4*5/TIME_BASE, 15*5/TIME_BASE ) ;

	/*着弾音*/
	GM_SeSetMode( SD_A_FLAME001, &trgt->hit, GM_SEMODE_BOMB ) ;

	BRK_FRT_Spark( &SparkPos0, &SparkVel0, hide ) ;
	BRK_FRT_Spark( &SparkPos1, &SparkVel1, hide ) ;
	BRK_FRT_Spark( &SparkPos2, &SparkVel2, hide ) ;
    }
    else if ( FRT_OBJ_GetTypeMasked( hide ) == FRT_TYP_FORKLIFT_C )
    {
	static FVECTOR BRK_FRT_OBJ_FallForce = { 0.0f, 0.0f, 200.0f, 0.0f } ;

	InitTarget( work, POWER_ONCE, &BRK_FRT_OBJ_FallForce ) ;
	NewForkliftMainLift( hide, 6604695, /*w11c2_forklift_front.kms*/
			     &BRK_FRT_OBJ_MainLift, NULL ) ;
	BRK_FRT_Smoke( &SmokePos1, &SmokeDir1, hide ) ;
	work->rot_v[X] = 50 ;

	/*着弾音*/
	GM_SeSetMode( SD_A_FLAME001, &trgt->hit, GM_SEMODE_BOMB ) ;

	NewOilSpreadCalled( &BRK_FRT_OBJ_OilPoolPos1, 240,
			    work->where, -1 ) ;
    }
    else if ( FRT_OBJ_GetTypeMasked( hide ) == FRT_TYP_FORKLIFT_D )
    {
	static FVECTOR PushForce  = { 200.0f, 0.0f, 0.0f, 0.0f } ;

	InitTarget( work, POWER_ONCE, &PushForce ) ;
	NewIronBoxDust( hide, trgt, 40 ) ;
	NewFortBlast( (FVECTOR *)hide->objs->world.m[W], 
		      PLAYER_SIDE, 2500.0f, 2000.0f, 25, 10, WP_Grenade, 2 ) ;
	NewBombEffect( &work->pos, 2 ) ;
	work->rot_v[X] = -50 ;
    }

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    //BrkWork *work = pwork ;

    switch( signal )
    {
    case FRT_OBJ_Destroyed:
	//GV_DestroyActor( work ) ;
	break;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

int NewForkliftBrokenUpper( HIDE *hide, TARGET *trgt, int flag )
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
	else
	    GV_SetActorSignalFunc( work, ReceiveSignal ) ;
    }
    return 0 ;
}

int NewForkliftBrokenFall( HIDE *hide, TARGET *trgt, int flag )
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
    }
    return 1 ;
}

int BRK_FRT_OBJ_BreakInitForklift( HIDE *hide, FVECTOR *pos )
{
    BrkWork work ;
    SVECTOR lf_rot ;
    FVECTOR lf_pos ;

    work.where = hide->work->where ;
    work.hide  = hide ;
    work.type = FRT_OBJ_GetTypeMasked( hide ) ;
    switch( work.type )
    {
    case FRT_TYP_FORKLIFT_B:
        work.pos.vx = -1000.0f  ;
        work.pos.vy = -44900.0f ;
        work.pos.vz = -1350.0f  ;
        work.rot[X] = DEGtoANG(4) ;
        work.rot[Y] = DEGtoANG(0) ;
        work.rot[Z] = DEGtoANG(4) ;
	break ;

    case FRT_TYP_FORKLIFT_C:
	work.pos.vx = -273.816345f   ;
	work.pos.vy = -44414.023438f ;
	work.pos.vz = -282.466736f   ;
	work.rot[X] = DEGtoANG(90) ;
	work.rot[Y] = work.rot[Z] = 0 ;

	lf_pos.vx = -273.816345f   ;
	lf_pos.vy = -44414.023438f ;
	lf_pos.vz = -282.466736f   ;
	lf_rot.vx = DEGtoANG(90)   ;
	lf_rot.vy = DEGtoANG(0)    ;
	NewForkliftMainLift( hide, 6604695, /*w11c2_forklift_front.kms*/
			     &lf_pos, &lf_rot ) ;
	NewOilStained( &BRK_FRT_OBJ_OilPoolPos1, work.where, -1 ) ;
	break ;

    case FRT_TYP_FORKLIFT_D:
        work.pos.vx = -5586.0f  ;
        work.pos.vy = -43926.0f ;
        work.pos.vz = -174.0f   ;
        work.rot[X] = DEGtoANG(0)  ;
        work.rot[Y] = DEGtoANG(11) ;
        work.rot[Z] = DEGtoANG(99) ;

	lf_pos.vx =  6302.980957f  ;
	lf_pos.vy = -43989.386719f ;
	lf_pos.vz = -19.371656f    ;
	lf_rot.vx = DEGtoANG(9)    ;
	lf_rot.vy = DEGtoANG(-180) ;
	NewForkliftMainLift( hide, 6604695, /*w11c2_forklift_front.kms*/
			     &lf_pos, &lf_rot ) ;
	NewOilStained( &BRK_FRT_OBJ_OilPoolPos1, work.where, -1 ) ;
	break ;
    }
    RotateMatrix( &work ) ;
    TransMatrix( &work ) ;

    return 1 ;
}

