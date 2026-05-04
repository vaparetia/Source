//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ironbox.c
   フォーチュン戦 鉄箱壊れ

   2000/12/15 T.Morita
   $Id: brk_ironbox.c,v 1.1.1.3 2002/11/19 11:46:14 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#define BRK_IRON_GRAVITY 6

typedef struct work_brk_t
{
    GV_ACT_EX   actor  ;

    DG_VERTS_ANIME front_uv  ;
    DG_VERTS_ANIME back_uv   ;
    FMATRIX        lights[2] ;

    HIDE       *hide  ;
    HZD_BOX    *hzd   ;
    int         where ;

    int         non_dmg ;
} BrkWork ;

static int IronSound[] = { SD_A_METLOTI1, SD_A_METLOTI2 } ;
static int n_IronSound = sizeof(IronSound) / sizeof(int) ;

static void Die( BrkWork *work )
{
    if ( work->front_uv.mem[0] )
	DG_FreeAnimVertsBuffer( &work->front_uv ) ;
    if ( work->back_uv.mem[0]  )
	DG_FreeAnimVertsBuffer( &work->back_uv  ) ;
}

static void Act( BrkWork *work )
{
    if ( work->non_dmg >= 0 )
	work->non_dmg-- ;
    if ( !work->non_dmg )
	work->hide->target.class &= ~TARGET_SKIP ;
}

static void InitThroughUVS( DG_OBJ *obj, DG_VERTS_ANIME *anm, float u, float v, float scale  )
{
    int            i   ;
    int            add_u, add_v ;
    DG_MDLPACK    *mdlpack ;
    DG_OBJ_PACKET *obj_packet;
    DG_TEX        *tex ;
#ifdef KP_XBOX
	DG_VERTEX_KMSS *kmss ;
#else
    short	  *uv_read, *uv_write ;
    SVECTOR       *verts ;
#endif

    DG_MakeAnimVertsBuffer( anm, obj, DG_VANIME_UVS|DG_VANIME_SINGLE ) ;

    mdlpack    = obj->model->packs ;
    obj_packet = obj->packets ;
    tex = (DG_TEX*)mdlpack->tex_id[0] ;


#ifdef PSX2
    add_u = DG_FTOI( tex->u_scale * (0.5f - u) * 2048.0f ) ;
    add_v = DG_FTOI( tex->v_scale * (0.5f - v) * 4096.0f ) ;
    uv_write = (short*)obj->uvs[0]  ;
    for( i=obj->model->n_packs ; --i>=0 ; ) {
		int j ;

		uv_read = mdlpack->uvs[0] ;
		verts   = (SVECTOR *)mdlpack->verts ;

		/* UVデータ生成 */
		for( j=mdlpack->n_verts ; --j>=0 ; uv_read+=2, uv_write+=2, verts++ ) {
			if ( scale>2048.0f ? verts->pad>=4095 : verts->pad<=1 ) {
				uv_write[0] = uv_read[0] + add_u ;
				uv_write[1] = uv_read[1] + add_v ;
			}
		}
		uv_write = (short*)((long128 *)obj->uvs[0] + obj_packet->uvs_offset[0]) ;
		mdlpack++ ;
		obj_packet++ ;
    }
#else
    add_u = DG_FTOI( tex->u_scale * (0.5f - u) * 4096.0f ) * 16.0f ;
    add_v = DG_FTOI( tex->v_scale * (0.5f - v) * 4096.0f ) * 16.0f*8 ;

	kmss = anm->verts_top[0] ;/*シングルなので、バッファはどっちでもいいのだ*/
	for( i=obj->model->n_verts ; --i>=0 ; ) {
		if ( scale>2048.0f ? kmss->wt>=4095 : kmss->wt<=1 ) {
			kmss->u0 += add_u ;
			kmss->v0 += add_v ;
		}
		kmss = (DG_VERTEX_KMSS *)( (char *)kmss + obj->model->stride ) ;
	}
#endif

}

static inline void InitThroughMatrix( BrkWork *work, DG_OBJS *objs,
				      FVECTOR *lpos, FVECTOR *ldir, FMATRIX *mtx )
{
    int      i ;
    DG_OBJ  *o ;
    FVECTOR from, to ;
    FMATRIX inv ;

    _sceVu0CopyVector( &from, lpos ) ;
    from.vw = 1.0f ;
    _sceVu0Normalize( &to, ldir ) ;
    to.vw   = 0.0f ;
    _sceVu0InversMatrix( &inv, mtx ) ;
    _sceVu0ApplyMatrix( &from, &inv, &from ) ;
    _sceVu0ApplyMatrix( &to  , &inv, &to   ) ;
    _sceVu0ScaleVector( &to, &to, 1000.0f/to.vz ) ;
    _sceVu0AddVector( &to, &to, &from ) ;
    from.vx = from.vx>400.0f ? 400.0f : from.vx<-400.0f ?-400.0f : from.vx ;
    from.vy = from.vy>900.0f ? 900.0f : from.vy< 100.0f ? 100.0f : from.vy ;
    from.vz =-500.0f ;
    to.vx   = to.vx> 350.0f ? 350.0f : to.vx<-350.0f ?-350.0f : to.vx ;
    to.vy   = to.vy> 790.0f ? 790.0f : to.vy< 200.0f ? 200.0f : to.vy ;
    to.vz   = 500.0f ;

    /* モデルのマトリックスを初期化 */
    _sceVu0CopyMatrix( &objs->world, mtx ) ;
    for( i=0, o=objs->objs ; i<objs->n_models ; i++, o++ )
    {
	_sceVu0CopyMatrix( &o->world, &DG_UnitMatrix ) ;
	switch( i )
	{
	case 1:
	    _sceVu0ScaleVector( &o->bound_min, &o->bound_min, 2.0f ) ;
	    _sceVu0ClampVector( &o->bound_min, &o->bound_min,-1500.0f, -1500.0f ) ;
	    _sceVu0ScaleVector( &o->bound_max, &o->bound_max, 2.0f ) ;
	    _sceVu0ClampVector( &o->bound_max, &o->bound_max, 1500.0f,  1500.0f ) ;
	    _sceVu0CopyVectorXYZ( (FVECTOR *)o->world.m[W], &from ) ;
	    InitThroughUVS( o, &work->front_uv, (from.vx+500.0f)/1000.0f, from.vy/1000.0f, 4096.0f ) ;
	    break ;
	case 2:
	    _sceVu0ScaleVector( &o->bound_min, &o->bound_min, 2.0f ) ;
	    _sceVu0ClampVector( &o->bound_min, &o->bound_min,-1500.0f, -1500.0f ) ;
	    _sceVu0ScaleVector( &o->bound_max, &o->bound_max, 2.0f ) ;
	    _sceVu0ClampVector( &o->bound_max, &o->bound_max, 1500.0f,  1500.0f ) ;
	    _sceVu0CopyVectorXYZ( (FVECTOR *)o->world.m[W], &to   ) ;
	    break ;
	case 3:
	    _sceVu0ScaleVector( &o->bound_min, &o->bound_min, 2.0f ) ;
	    _sceVu0ClampVector( &o->bound_min, &o->bound_min, -1500.0f, -1500.0f ) ;
	    _sceVu0ScaleVector( &o->bound_max, &o->bound_max, 2.0f ) ;
	    _sceVu0ClampVector( &o->bound_max, &o->bound_max,  1500.0f,  1500.0f ) ;
	    o->world.m[W][Z] = 10.0f ;
	    InitThroughUVS( o, &work->back_uv , (to.vx+500.0f)/1000.0f, to.vy/1000.0f, 2048.0f ) ;
	    break ;
	}
	_sceVu0MulMatrix( &o->world, &objs->world, &o->world ) ;
    }
}

static inline int InitThroughModel( BrkWork *work, HIDE *h, TARGET *t )
{
    int i ;
    DG_DEF  *def ;
    DG_OBJS *objs ;
    FVECTOR hit   = { h->objs->world.m[W][X]+100.0f*frnd(),
		      h->objs->world.m[W][Y]+100.0f*frnd(),
		      h->objs->world.m[W][Z]-500.0f,
		      h->objs->world.m[W][W] } ;
    FVECTOR force = {100.0f*frnd(), 100.0f*frnd(), 1000.0f, 0.0f };

    /* モデルを初期化 */
    i = GV_StrCode( FRT_OBJ_GetTypeModel( h ) ) ;
    if ( !(def = GV_GetCache( GV_CacheID( i, 'k' ) )) )
	PERROR( "No KMS<%s> in data.cnf!! : NewIronBoxBroken\n", FRT_OBJ_GetTypeModel( h ) ) ;
    if ( !(objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Cannot make DG_OBJS(Maybe no memory!!) : NewIronBoxBroken\n" ) ;
    DG_QueueObjs( objs ) ;

    if ( t )
	InitThroughMatrix( work, objs,
			   &h->target.hit, &t->power->force,
			   &h->objs->world ) ;
    else
	InitThroughMatrix( work, objs,
			   &hit, &force,
			   &h->objs->world ) ;

    /* モデルがちゃんとできたら解放する */
    FRT_OBJ_FreeObject( h, 0 ) ;
    h->objs = objs ;

    /* 当たり関係を貼り直す */
    FRT_OBJ_InitTargetHzxHzd( h ) ;

    ASSERT( !h->shadow && h->objs ) ;
    FRT_OBJ_InitShadow( h ) ;

    return 0 ;
}

static int ReceiveSignal( void *pwork, int signal, int value )
{
    BrkWork *work = pwork ;

    switch( signal )
    {
    case FRT_OBJ_FreeObjs:
	if ( value == (int)work->hide )
	{
	    if ( work->front_uv.mem[0] )
		DG_FreeAnimVertsBuffer( &work->front_uv ) ;
	    work->front_uv.mem[0] = NULL ;
	    if ( work->back_uv.mem[0]  )
		DG_FreeAnimVertsBuffer( &work->back_uv  ) ;
	    work->back_uv.mem[0] = NULL ;
	    GV_DestroyActor( work ) ;
	}
	break;
    default:
	return GV_DefaultSignalFunc( pwork, signal, value ) ;
    }
    return 0 ;
}

static int GetResources( BrkWork *work, HIDE *hide, TARGET *trgt, int flag )
{
    int     i ;
    FVECTOR v ;

    /* 火花が散る */
    if ( flag )
    {
	NewCrushLine( &trgt->hit, &trgt->power->force, 100.0f  ) ;
	NewCrushLine( &trgt->hit, &trgt->power->force, 1000.0f ) ;
    }
    work->hide = hide ;
    hide->type++ ;
    NewIronBoxDust( hide, trgt, 20 ) ;
    if ( InitThroughModel( work, hide, trgt ) < 0 )
	PERROR( "Couldn't initialize New Model(%d) : NewIronBoxBroken\n", --hide->type ) ;

    if ( flag )
    {
	/* しばらく無敵（こうしないと2度きてしまうから） */
	hide->target.class |= TARGET_SKIP ;
	work->non_dmg = 4 ;

	/* 穴が熱くなって焼けている */
	_sceVu0ScaleVector( &v, &trgt->power->force, 0.05f ) ;
	GV_SetActorChild( work,
			  NewFortHeatIron( work->hide->objs ) ) ;
	GV_SetActorChild( work,
			  NewFortSmoke( &work->hide->objs->objs[2].world,
					&DG_ZeroVector, &v, hide->work->hzd ) ) ;

	/* 貫通音 */
	GM_SeSetMode( SD_A_METLANA1, (FVECTOR *)work->hide->objs->world.m[W], GM_SEMODE_BOMB ) ;

	v.vy = 500.0f ;
	v.vw = 1.0f   ;
	for ( i=6 ; --i>=0 ; )
	{
	    v.vx = 500.0f*frnd() ;
	    v.vz = 500.0f*(1-2*(i&1)) ;
	    GV_SetActorChild( work, NewLineSmoke( &work->hide->objs->world, &v, 60*10 ) ) ;
	}
    }
    else
    {
	work->non_dmg = 0 ;

	/* 次は貫通する */
	hide->target.class |= TARGET_THROUGH ;
    }

    /* InitThroughModel()より後にしないとこのキャラが死ぬから */
    GV_SetActorSignalFunc( work, ReceiveSignal ) ;

    return 0 ;
}


/* 貫通する箱 */
int NewIronBoxThrough( HIDE *hide, TARGET *trgt, int flag )
{
    BrkWork *work ;

    if ( GM_GameLevel >= GM_LEVEL_EXTREME )
    {
	hide->type &= FRT_TYP_FLAGMASK ;
	hide->type |= FRT_TYP_IRONBOX1M2_C ;
	return NewIronBoxComplete( hide, trgt, flag ) ;
    }
    else
    {
	work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) ) ;
	if ( work )
	{
	    GV_SetActor( &work->actor, Act, Die ) ;
	    GV_ActorEX( &work->actor ) ;
	    GV_SetActorChild( hide->work, work ) ;
	    if( GetResources( work, hide, trgt, flag ) < 0 )
		GV_DestroyActor( work ) ;
	}
	return 0 ;
    }
    return 0 ;
}


/* 壊れ箱 */
int NewIronBoxBroken( HIDE *hide, TARGET *trgt, int flag )
{
    hide->type++ ;
    NewIronBoxDust( hide, trgt, 20 ) ;
    if ( FRT_OBJ_InitModel( hide, &hide->objs->world ) < 0 )
	PERROR( "Couldn't initialize New Model(%d) : NewIronBoxBroken\n", --hide->type ) ;

    return 0 ;
}


/* 箱の粉砕 */
int NewIronBoxComplete( HIDE *hide, TARGET *trgt, int flag )
{
    FMATRIX mtx ;

    /* 破壊音 */
    GM_SeSetMode( SD_A_METLBRE1, (FVECTOR *)hide->objs->world.m[W], GM_SEMODE_BOMB ) ;

    NewIronBoxDust( hide, trgt, 30 ) ;
    NewAnythingDustScale( hide, trgt, 8,
			  6348906, /* w11c2_ironbox_frg1_cm*/
			  BRK_IRON_GRAVITY,
			  100.0f, 2.0f,
			  NULL, 0 ) ;

    /* 爆破 */
    NewFortBlast( (FVECTOR *)hide->objs->world.m[W], 
		  PLAYER_SIDE, 2000.0f, 1500.0f, 10, 10, WP_Kick, 2 ) ;

    if ( hide->type & FRT_TYP_FLAG_UPPER_DECK )/*残らず崩壊*/
	FRT_OBJ_FreeObject( hide, 0 ) ;
    else  /*壊れ残り*/
    {
	hide->type++ ;
	if ( FRT_OBJ_InitModel( hide, &hide->objs->world ) < 0 )
	    PERROR( "Couldn't initialize New Model(%d) : NewIronBoxBroken\n", --hide->type ) ;
	_sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, ((u_int)hide & 2)*M_PI ) ;
	_sceVu0MulMatrix( &hide->objs->world, &hide->objs->world, &mtx ) ;
    }
    return 1 ;
}


/* 小物 */
int NewIronBoxDust( HIDE *hide, TARGET *trgt, int n_dust )
{
    return NewAnythingDust( hide, trgt, n_dust,
			    6381674, /* w11c2_ironbox_frg2_cm */
			    BRK_IRON_GRAVITY,
			    50.0f, 60*10,
			    IronSound, n_IronSound ) ;
}


/* 壊れ箱(現在未使用) */
int NewIronBoxBurn( HIDE *hide, TARGET *trgt, int flag )
{
    hide->type++ ;
    NewIronBoxDust( hide, trgt, 20 ) ;

    return 0 ;
}


int BRK_FRT_OBJ_BreakInitIronThrough( HIDE *hide, FVECTOR *pos )
{
    BrkWork *work ;

    if ( (work = (BrkWork *)GV_NewActor( GV_ACTOR_USER, sizeof(BrkWork) )) )
    {
	GV_SetActor( &work->actor, NULL, Die ) ;
	GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( hide->work, work ) ;
	work->hide = hide ;
	if ( InitThroughModel( work, hide, NULL ) < 0 )
	    GV_DestroyActor( work ) ;
	else
	    GV_SetActorSignalFunc( work, ReceiveSignal ) ;
    }
    return 0 ;
}

int BRK_FRT_OBJ_BreakInitIronBox( HIDE *hide, FVECTOR *pos )
{
    NewAnythingDustScaleInit( hide, 
			      pos,
			      16,
			      6381674,/*w11c2_ironbox_frg2_cm*/
			      50.0f ) ;
	return 0 ;
}
