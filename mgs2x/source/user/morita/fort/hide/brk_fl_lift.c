//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gastank.c
   フォーチュン戦 タンク壊れ

   2000/12/18 T.Morita
   $Id: brk_fl_lift.c,v 1.1.1.3 2002/11/19 11:46:13 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

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
    int          hide_type ;

    HZD_BOX    *hzd   ;
    int         where ;
    HIDE       *hide  ;

#if MAKING
    void *wireframe ;
#endif
} BrkWork ;


#define ANGtoDEG(_a) ((_a)*45/8192)
#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

static void RotateTransMatrix( BrkWork *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(work->rot[Y]) ) ;
    _sceVu0RotMatrixX( mtx, mtx           , ANGtoRAD(work->rot[X]) ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], &work->pos ) ;
}

static inline void RotateMatrix( BrkWork *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, ANGtoRAD(work->rot[Y]) ) ;
    _sceVu0RotMatrixX( mtx, mtx           , ANGtoRAD(work->rot[X]) ) ;
}

static inline void TransMatrix( BrkWork *work )
{
    FMATRIX *mtx = &work->objs->world ;

    _sceVu0CopyVectorXYZ( (FVECTOR*)mtx->m[W], &work->pos ) ;
}


#define BRK_GRAVITY      6
#define FALL_ROT_MAX DEGtoANG(90)
#define FALL_ROT_V   15


static void BRK_LFT_ActBreak_C( BrkWork *work )
{
    if ( work->flag >= 0 )
    {
	float vz = -cosf( ANGtoRAD(work->rot[X]) ) ;
	float vy = -sinf( ANGtoRAD(work->rot[X]) ) ;

	work->rot[X] += work->rot_v[X] += FALL_ROT_V ;

	/* 倒れた時の弾み処理 */
	if ( work->rot[X] > FALL_ROT_MAX )
	{
	    work->rot[X]   =  FALL_ROT_MAX     ;
	    work->rot_v[X] = -work->rot_v[X]/8 ;
	    if ( !(work->rot_v[X]/(FALL_ROT_V/2)) )
	    {
		   // BP_WARNING - This USED to be work->rot[Z] = 0, but
         // that is invalid because rot is really just XY.  So
         // the side effect that we're keeping is that it's work->rot_v[X] = 0
         // work->rot[Z] = 0 ;
         work->rot_v[X] = 0;
		   work->flag   = -1 ;

printf( "Lift : r(%d %d) p(%f %f %f)\n",
	ANGtoDEG( work->rot[X] ),
	ANGtoDEG( work->rot[Y] ),
	work->pos.vx,
	work->pos.vy,
	work->pos.vz
	) ;

	    }
	}

	vz += cosf( ANGtoRAD(work->rot[X]) ) ;
	vy += sinf( ANGtoRAD(work->rot[X]) ) ;
	work->pos.vz -= vz * 987.0f ;
	work->pos.vy -= vy * 500.0f ;
	RotateTransMatrix( work ) ;
    }
}

static int BRK_LFT_ActBreak_D( BrkWork *work )
{
    static struct
    {
	FVECTOR pos ;
	short   rot_v ;
    } Target[] = {
	{ { 3187.0f,-43576.0f, 391.0f, 200.0f}, DEGtoANG(10) },
	{ { 6160.0f,-43876.0f,   0.0f,  60.0f}, DEGtoANG( 0) },
	{ { 6302.980957f,-43989.386719f,-19.371656f,  40.0f}, DEGtoANG(9) },
    } ;
    float   d ;

    if ( work->flag >=0 )
	switch( work->flag )
	{
	case 3:
	    /* ドシーーンと音がなる */
	    GM_SeSetMode( SD_A_FLAME001, &work->pos, GM_SEMODE_BOMB ) ;
	    work->flag = -1 ;

printf( "Lift : r(%d %d) p(%f %f %f)\n",
	ANGtoDEG( work->rot[X] ),
	ANGtoDEG( work->rot[Y] ),
	work->pos.vx,
	work->pos.vy,
	work->pos.vz
	) ;
	    RotateMatrix( work ) ;
	    TransMatrix( work ) ;

	    break ;
	case 0:
	    work->pos_v.vw = 0.0f ;

	default:
	    work->pos_v.vy -= BRK_GRAVITY ;
	    work->rot_v[Y] = (work->rot[Y]>0 ? DEGtoANG(5) : DEGtoANG(-5)) ;
	    work->rot[X] += work->rot_v[X] ;
	    work->rot[Y] += work->rot_v[Y] ;
	    if ( work->rot[Y]/DEGtoANG(175) )
		work->rot[Y] = DEGtoANG(180), work->rot_v[Y] = 0 ;
	    RotateMatrix( work ) ;
	    if ( work->flag== 2 )
	    {
		static FVECTOR TipOfLift = {-318.0f, -908.0f, -330.0f, 1.0f} ;
		static FVECTOR GroundDir = {   0.0f,   -1.0f,    0.0f, 0.0f} ;
		extern void NewSpark( FMATRIX * ) ;
		FMATRIX mat ;

		_sceVu0ApplyMatrix( (FVECTOR *)mat.m[W], &work->objs->world, &TipOfLift ) ;
		_sceVu0CopyVector( (FVECTOR *)mat.m[Y], &GroundDir ) ;
		NewSpark( &mat ) ;
	    }

	    if ( work->pos_v.vw <= 0.0f )
	    {
		/* 位置を合わせる */
		if ( work->flag > 3 )
		    _sceVu0CopyVectorXYZ( &work->pos, &Target[work->flag].pos ) ;

		work->pos_v.vy = Target[work->flag].pos.vw ;
		d = work->pos_v.vy * work->pos_v.vy - 
		    2.0f * (Target[work->flag].pos.vy - work->pos.vy) * BRK_GRAVITY ;
		if ( work->flag < 2 )
		{
		    work->pos_v.vw = (work->pos_v.vy + sceVu0Sqrt( d )) / BRK_GRAVITY  ;
		    work->pos_v.vx = (Target[work->flag].pos.vx - work->pos.vx) / work->pos_v.vw ;
		    work->pos_v.vz = (Target[work->flag].pos.vz - work->pos.vz) / work->pos_v.vw ;
		    if ( work->flag >= 1 )
			work->rot_v[X] = (short)((Target[work->flag].rot_v - work->rot[X]) / work->pos_v.vw) ;
		    else
			work->rot_v[X] =  Target[work->flag].rot_v ;
		}
		else
		{
		    work->rot[X] = Target[2].rot_v ;
		    work->rot_v[X] = 0 ;
		    work->pos.vx = Target[2].pos.vx ;
		    work->pos.vy = Target[2].pos.vy ;
		    work->pos.vz = Target[2].pos.vz ;
		    work->pos_v.vx = work->pos_v.vy = work->pos_v.vz = 0 ;

		}
		work->flag++ ;
	    }
	    else
	    {
		work->pos_v.vw -= 1.0f ;
		_sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
	    }
	    TransMatrix( work ) ;

	    GM_PutTarget( &work->target ) ;
	    GM_MoveTarget2Map( &work->target, &work->objs->world, work->where )  ;
	}
    return 0 ;
}

static void Act( BrkWork *work )
{
    if ( work->hide_type != work->hide->type )
    {
	work->hide_type = work->hide->type ;
	work->flag = 0 ;
    }
    switch( FRT_OBJ_GetTypeMasked( work->hide ) )
    {
    case FRT_TYP_FORKLIFT_C:
	BRK_LFT_ActBreak_C( work ) ;
	break ;

    case FRT_TYP_FORKLIFT_D:
	BRK_LFT_ActBreak_D( work ) ;
	break ;
    }
}

static void Die( BrkWork *work )
{
#if MAKING
    if ( work->wireframe )
	GV_DestroyOtherActor( work->wireframe ) ;
#endif

    if ( work->objs )
        DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
}

static int InitTarget( BrkWork *work, DG_OBJS *o )
{
    FVECTOR t_size, t_pos ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR uv = { o->def->ux, o->def->uy, o->def->uz, 0 } ;
    FVECTOR lv = { o->def->lx, o->def->ly, o->def->lz, 0 } ;
    static FVECTOR PushForce  = { 100.0f, 0.0f, 0.0f, 0.0f } ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.25f ) ;
    _sceVu0AddVector( &t_pos , &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.25f ) ;
    t_pos.vx += o->def->tx ;
    t_pos.vy += o->def->ty ;
    t_pos.vz += o->def->tz ;

    GM_SetTarget( t, TARGET_OFFENSE|TARGET_POWER, work->where, PLAYER_SIDE, &t_size, &t_pos ) ;
    GM_SetTargetWeaponType( t, WP_KICK ) ;
    GM_SetPowerTarget( t, p, POWER_EXPLODE, GM_Vitality, 0, 0, &PushForce ) ;
    GM_MoveTarget2Map( &work->target, &work->objs->world, work->where ) ;

    //NewTargetView( t, 200, 50, 32 ) ;

    return 0 ; 
}

static int GetResources( BrkWork *work, HIDE *hide,
			 int id, FVECTOR *pos, SVECTOR *rot )
{
    DG_DEF *def ;

    if ( !(def  = GV_GetCache( GV_CacheID( id, 'k' ) )) )
        PERROR( "No KMS found in data.cnf!! :: NewPutVanimeObject\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
        PERROR( "Can't Create DG_OBJS(Maybe no memory)!! :: NewPutVanimeObject\n" ) ;
    DG_QueueObjs( work->objs ) ;

    work->where = hide->work->where ;
    work->hzd   = hide->work->hzd   ;
    work->hide  = hide ;
    work->hide_type = work->hide->type ;

    if ( rot )
    {
	work->flag  = -1 ;
	_sceVu0CopyVector( &work->pos, pos ) ;
	work->rot[X] = rot->vx ;
	work->rot[Y] = rot->vy ;
    }
    else
    {
	work->flag  = 0 ;
	_sceVu0ApplyMatrix( &work->pos, &work->hide->objs->world, pos ) ;
	work->rot[X] = work->hide->rot.vx ;
	work->rot[Y] = work->hide->rot.vy ;
    }
    RotateTransMatrix( work ) ;

    InitTarget( work, work->objs ) ;


#if MAKING
    {
	extern void *NewDrawWireframe( DG_OBJ * ) ;
	work->wireframe = NewDrawWireframe( &work->objs->objs[0] ) ;
    }
#endif

    return 0 ;
}

int NewForkliftMainLift( HIDE *hide, int id, FVECTOR *pos, SVECTOR *rot )
{
    BrkWork *work ;

    work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        GV_SetActorChild( hide->work, work ) ;

        if( GetResources( work, hide, id, pos, rot ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return 0 ;
        }
    }
    return 1 ;
}
