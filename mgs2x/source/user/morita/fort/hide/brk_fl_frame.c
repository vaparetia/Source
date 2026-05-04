//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_fl_frame.c
   フォーチュン戦 フォークリフト上部フレームのふっ飛び

   2000/12/18 T.Morita
   $Id: brk_fl_frame.c,v 1.1.1.3 2002/11/19 11:46:13 Yoshizawa1 Exp $
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
    GV_ACT_EX    actor  ;

    TARGET       target ;
    POWER_TARGET power  ;

    FVECTOR      pos    ;
    FVECTOR      pos_v  ;
    short        rot[BRK_FRM_N_MDL][XY]   ;
    short        rot_v[BRK_FRM_N_MDL][XY] ;
    int          flag   ;
    DG_OBJS     *objs   ;
    FVECTOR     *trgt   ;
    short        se_tic ;
    short        se_vol ;
    short        mtx_flag ;
    short        mtx_cnt  ;

    HZD_BOX     *hzd    ;
    int          where  ;
    HIDE        *hide   ;

#if MAKING
    void        *wireframe[8] ;
#endif

} BrkWork ;


#define DEGtoANG(_a) (short)((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)
static void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot[XY] )
{
    _sceVu0RotMatrixX( out, in , ANGtoRAD(rot[X]) ) ;
    _sceVu0RotMatrixY( out, out, ANGtoRAD(rot[Y]) ) ;
}
static void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    _sceVu0AddVector( (FVECTOR*)&mtx->m[W], (FVECTOR*)&mtx->m[W], pos ) ;
    mtx->m[W][W] = 1.0f ;
}
static void MakePosition( BrkWork *work )
{
    FMATRIX *parent ;
    DG_OBJ *o ;
    int i ;

    _sceVu0CopyMatrix( &work->objs->objs->world, &work->objs->world ) ;
    for ( i=1 ; i<work->objs->n_models ; i++ )
	if ( work->mtx_flag & (1<<i) )
	{
	    work->rot_v[i][X] = work->rot_v[i][X]*(BRK_ROT_PART_R-1)/BRK_ROT_PART_R ;
	    work->rot_v[i][Y] = work->rot_v[i][Y]*(BRK_ROT_PART_R-1)/BRK_ROT_PART_R ;
	    work->rot[i][X] += work->rot_v[i][X] ;
	    work->rot[i][Y] += work->rot_v[i][Y] ;

	    o = &work->objs->objs[i] ;
	    parent = o->parent<0 ? &work->objs->world : &work->objs->objs[o->parent].world ;
	    RotateMatrixXY( &o->world, parent, work->rot[i] ) ;
	    _sceVu0ApplyMatrix( (FVECTOR *)o->world.m[W], parent, &o->trans ) ;
	}
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
    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot[0] ) ;
    if ( check_hzd )
    {
	BRK_UTL_SizeOfMDL( p->objs->objs->model, &p->objs->world, &size ) ;
	flag = BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v, &bounce, &size ) ;
    }
    switch( flag )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R ) ;
	if ( BRK_UTL_PutCenterHazard( p->hzd, &p->pos, &p->pos_v,
				      1000.0f, 25.0f ) )
	{
	    p->pos_v.vy = 10.0f ;
	    p->rot_v[0][X] = irnd()&10 ? 200 : -200 ;
	    flag = 3 ;
	}
	else
	{
	    p->rot_v[0][X] = ( (p->rot[0][X]>0 ? DEGtoANG(180):	-DEGtoANG(179)) - p->rot[0][X]) / 16 ;
	    flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;
	    if ( flag == 2 )
		if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
		    p->rot_v[0][X]=0, flag = 1 ;
	}
	if ( p->se_tic > 0 )
	    p->rot_v[0][X]=0, flag = 1 ;
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
	p->rot[0][X] += p->rot_v[0][X] ;
	p->rot[0][Y] += p->rot_v[0][Y] ;
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	break ;
    }
    TransMatrix( &p->objs->world, &p->pos ) ;
    MakePosition( p ) ;
    if ( p->se_tic > 0 )
	p->se_tic-- ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( BrkWork *p )
{
    int     flag = 1 ;
    int     hzd_flg ;
    short   r    ;
    FVECTOR size ;

    p->pos_v.vy = -BRK_GRAVITY*5 ;

    p->rot[0][Y] += p->rot_v[0][Y] = p->rot_v[0][Y] * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->rot_v[0][X] += (p->rot[0][X]>0 ? DEGtoANG(5) : -DEGtoANG(5)) ;
    r = p->rot[0][X] + p->rot_v[0][X] ;
    if ( r/DEGtoANG(90) && ((r< 0 && p->rot[0][X]>=0) ||
			    (r>=0 && p->rot[0][X]< 0)) )
    {
        r = DEGtoANG(180) ;
	p->rot_v[0][X] = 0 ;
	if ( !p->rot_v[0][Y] )
	    flag = 0 ;
    }
    p->rot[0][X] = r ;

    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot[0] ) ;
    BRK_UTL_SizeOfMDL( p->objs->objs->model, &p->objs->world, &size ) ;
    hzd_flg = BRK_CheckHazard( p->hzd, &p->pos, &p->pos_v,
			       &BRK_HZD_NoBounce, &size ) ;
    if ( !(hzd_flg & 1) )
	p->pos.vy += p->pos_v.vy, flag = 1 ;
    TransMatrix( &p->objs->world, &p->pos ) ;
    MakePosition( p ) ;

    return flag ;
}



static void Die( BrkWork *work )
{

#if MAKING
    int i ;

    for ( i=0 ; i<work->objs->n_models ; i++ )
	if ( work->wireframe[i] )
	    GV_DestroyOtherActor( work->wireframe[i] ) ;
#endif

    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
}

/* バラバラにするための処理 */    
static int ActStick( BrkWork *work )
{
    int mtx_type ;

#if 0
    if ( work->mtx_cnt==4 || work->mtx_cnt==6 )/* この関節番号は,分解しない */
	work->mtx_cnt++ ;
#endif

    if ( work->mtx_cnt < BRK_FRM_N_MDL )
    {
	mtx_type = work->mtx_cnt<=2 ? 0 : 1 ;/* １は９０°Ｚ回転して落ち着く棒 */
	work->mtx_flag &= ~(1 << work->mtx_cnt) ;

	GV_SetActorChild( work->hide->work,
			  NewForkliftStick( work->hzd, mtx_type,
					    &work->objs->objs[work->mtx_cnt],
					    work->rot[0][X] + work->rot[work->mtx_cnt][X],
					    work->rot[0][Y] + work->rot[work->mtx_cnt][Y] ) ) ;
	work->mtx_cnt++ ;
	return 1 ;
    }
    return 0 ;
}

/* フレームのメインアクト */
static void Act( BrkWork *work )
{
    int act_flag ;

    act_flag = 0 ;
    switch ( work->flag )
    {
    case 1:
	if ( !FallDownObject( work ) )
	{
	    while( ActStick( work ) ) ;
	    work->flag = 0 ;
	}
    case 0:
	break ;
    default:
	if ( work->flag > 2 )
	{
	    work->flag-- ;
	    act_flag = MoveObject( work, 0 ) ;
	}
	else
	{
	    act_flag = MoveObject( work, 1 ) ;
	    if ( act_flag == 1 )
		work->flag = 1 ;
	    else if ( work->mtx_cnt < BRK_FRM_N_MDL )
	    {
		GM_MoveTarget2Map( &work->target, &work->objs->world,
				   work->where ) ;
		GM_PutTarget( &work->target ) ;
	    }
	}
	break ;
    }

    if ( act_flag )
    {
	ActStick( work ) ;
	ActStick( work ) ;
    }
}

/* 攻撃用ターゲット設定 */
static int InitTarget( BrkWork *work, DG_OBJS *o )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR uv = { o->def->ux, o->def->uy, o->def->uz, 0 } ;
    FVECTOR lv = { o->def->lx, o->def->ly, o->def->lz, 0 } ;
    static FVECTOR PushForce = { 200.0f, 0.0f, 0.0f, 0.0f } ;

    t_size.vx = (uv.vx - lv.vx)*0.25f ;
    t_size.vy = (uv.vy - lv.vy)*0.25f ;
    t_size.vz = (uv.vz - lv.vz)*0.25f ;
    t_pos.vx  = (uv.vx + lv.vx)*0.25f + o->def->tx ;
    t_pos.vy  = (uv.vy + lv.vy)*0.25f + o->def->ty ;
    t_pos.vz  = (uv.vz + lv.vz)*0.25f + o->def->tz ;

    GM_SetTarget( t, TARGET_OFFENSE|TARGET_POWER, work->where, PLAYER_SIDE, &t_size, &t_pos ) ;
    GM_SetTargetWeaponType( t, WP_KICK ) ;
    GM_SetPowerTarget( t, p, POWER_EXPLODE, GM_Vitality, 10, 5, &PushForce ) ;
    GM_MoveTarget2Map( &work->target, &work->objs->world, work->where ) ;

    //NewTargetView( t, 200, 50, 32 ) ;

    return 0 ; 
}


static int GetResources( BrkWork *work, HIDE *hide, FVECTOR *pos, int id, TARGET *trgt )
{
    DG_DEF *def ;
    int     i ;
    static short rot_v[][XY] = 
    {
	{ DEGtoANG(  2 ),-DEGtoANG(  8 ) }, { DEGtoANG(  4 ), DEGtoANG(0.5f) },
	{ DEGtoANG(  3 ), DEGtoANG(  1 ) }, { DEGtoANG(  1 ), DEGtoANG(  0 ) },
	{ DEGtoANG(  0 ),-DEGtoANG(0.2f) }, { DEGtoANG(  1 ), DEGtoANG(  3 ) },
	{ DEGtoANG(  0 ), DEGtoANG(0.1f) },
    } ;
    FVECTOR enlarge = { 6000.0f, 2000.0f, 6000.0f, 0.0f } ;

    if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS found in data.cnf!! :: NewForkliftFrame\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewForkliftFrame\n" ) ;
    DG_QueueObjs( work->objs ) ;

    /* バウンドを大きくする */
    _sceVu0AddVector( &work->objs->bound_max,
		      &work->objs->bound_max, &enlarge ) ;
    _sceVu0SubVector( &work->objs->bound_min,
		      &work->objs->bound_min, &enlarge ) ;
    for( i=def->n_x_models ; --i>=0 ; )
    {
	_sceVu0AddVector( &work->objs->objs[i].bound_max,
			  &work->objs->objs[i].bound_max, &enlarge ) ;
	_sceVu0SubVector( &work->objs->objs[i].bound_min,
			  &work->objs->objs[i].bound_min, &enlarge ) ;
    }

    /* 各種パラメータを変更 */
    work->where = hide->work->where ;
    work->hzd   = hide->work->hzd   ;
    work->hide  = hide ;
    work->flag  = 2+14 ;
    work->mtx_flag = 0x00fe ; /* このキャラで面倒を見るオブジェクトのマスク(5つ) */
    work->mtx_cnt  = 1 ;

#define BRK_POPUP  40.0f
#define BRK_SPEED 100.0f
    _sceVu0ApplyMatrix( &work->pos, &work->hide->objs->world, pos ) ;

#if 0
    d = BRK_SPEED / sceVu0Sqrt( trgt->power->force.vx * trgt->power->force.vx +
				trgt->power->force.vz * trgt->power->force.vz ) ;
    work->pos_v.vx = trgt->power->force.vx * d ;
    work->pos_v.vy = BRK_POPUP ;
    work->pos_v.vz = trgt->power->force.vz * d ;
#else
    work->pos_v.vx = 60.0f ;
    work->pos_v.vy = BRK_POPUP ;
    work->pos_v.vz = 100.0f ;
#endif

    for( i=BRK_FRM_N_MDL ; --i>=0 ; )
    {
	work->rot_v[i][X] = rot_v[i][X] ;
	work->rot_v[i][Y] = rot_v[i][Y] ;
    }
    RotateMatrixXY( &work->objs->world, &DG_UnitMatrix, work->rot[0] ) ;
    TransMatrix( &work->objs->world, &work->pos ) ;
    MakePosition( work ) ;
    InitTarget( work, work->objs ) ;


#if MAKING
    {
	extern void *NewDrawWireframe( DG_OBJ * ) ;
	for ( i=0 ; i<work->objs->n_models ; i++ )
	    work->wireframe[i] = NewDrawWireframe( &work->objs->objs[i] ) ;
    }
#endif

    return 0 ;
}


static int ReceiveSignal( void *pwork, int signal, int value )
{
    BrkWork *work = pwork ;

    switch( signal )
    {
    case FRT_OBJ_Destroyed:
	if ( work->pos.vy < -44950.0f )
	    if ( work->flag == 0 )
		work->flag = 2 ;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

int NewForkliftFrame( HIDE *hide, FVECTOR *pos, int id, TARGET *trgt )
{
    BrkWork *work ;

    work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;
	GV_SetActorSignalFunc( work, ReceiveSignal ) ;

        if( GetResources( work, hide, pos, id, trgt ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return 0 ;
        }
    }
    return 1 ;
}
