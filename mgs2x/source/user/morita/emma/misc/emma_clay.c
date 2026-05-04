//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_clay.c
   セットされたＣｌａｙｍｏｒｅ
   
   2000/02/23 M.Sonoyama
   $Id: emma_clay.c,v 1.1.1.3 2002/11/19 11:46:04 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libutl.h>
#endif

#include	"gameheader.h"

/*------------------------------------------------------------*/

extern	void	*NewBlast( FVECTOR *, int, int, int, int, int, int ) ;
extern	void	*NewDispStringsWorld2Screen( FVECTOR *, char *, CVECTOR ) ;
extern  void 	*NewSKLandingSmoke( FVECTOR * , FVECTOR * , float );
extern   int	 DG_GetLightMatrixFix( FVECTOR * , FMATRIX * );

#define	BODY_NAME	(104941)	/* clm */
#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE|DG_FLAG_IRREACTION)
#define	DELAY_BLAST	(8)
#define	DIE_COUNT	(DIRECT_TICK(60))

enum {
    TYPE_SEGMENT = 0,	
    TYPE_FLOOR
} ;

enum {
    FLAG_PLAYER = 	0x0001,
    FLAG_SCENARIO = 	0x0002,
    FLAG_FREEZE =	0x0010,
    FLAG_DESTROY =	0x0020,
    FLAG_FREE =		0x0040,
	FLAG_COLLECTED = 0x0080,
	FLAG_HIT =		0x0100,
	FLAG_HITTED =	0x0200,
	FLAG_CANNOT_COLLECT =	0x0400,

	FLAG_DEBUG_DISP = 0x4000,
} ;

typedef struct _Work {
    GV_ACT			actor ;
    OBJECT			body ;
    FMATRIX			light[ 2 ] ;
	RADAR_CTRL		radar ;
    FMATRIX			world ;
    FVECTOR			center ;
    FVECTOR			center2 ;
    TARGET			touch ;
    TARGET			def ;
    POWER_TARGET	dummy ;
    HZX_HZD			seg ;
	void			*disp ;
    int				type ;
    int				side ;
    int				map ;
    int				flag ;
    int				delay ;
	int				checkN ;
	int				proc ;
	int				die_count ;
	int				name ;
	GM_BOMB			list ;
	HZX_D_CALLBACK	dhcb ;
} Work ;

extern void *NewClayBombFlag( int num , FMATRIX *world, FVECTOR *shift , FVECTOR *pPower , int life, float alpha, long64 flag );
extern void *NewClayBomb( int num , FMATRIX *world, FVECTOR *shift , FVECTOR *pPower , int life, float alpha );
extern void *NewClayBombMng( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float );
extern void	*NewBlast3( FVECTOR * , int , int , int , int , int , int , int );

/*---------------------------------------------------------------*/

/* 爆発 */
static	void	Bomb( Work *work )
{
	FMATRIX fmtmp;
	FVECTOR speed;
		
	speed = DG_ZeroVector;
	speed.vx = 700.0f;

	fmtmp = work->touch.world;
	*( FVECTOR * )fmtmp.m[ 3 ] = work->center2;
    GM_SeSetMode( SD_W_MINEEXP1 , &work->center , GM_SEMODE_BOMB ) ; // se
	GM_SetNoise( NOISE_L, &work->center , GM_CurrentMap ) ;

    NewClayBombFlag( 10 , &fmtmp, &DG_ZeroVector , &speed , 10 , 128, WP_NOPLAYER ) ;

	//NewClayBombMng( 10 , &fmtmp, &DG_ZeroVector , &speed , 10 , 128 );
}

/* ターゲット用のワールドマトリクスを作成 */
static	void	MakeTargetWorld( FMATRIX *world, Work *work )
{
	FMATRIX	tmpMat, tmpMat2 ;
	SVECTOR	rot = { 0, 0, 30 * 1024 / 90 } ;

	DG_COPY_MAT( &tmpMat, &work->body.objs->world ) ;
	GV_VecToMat( &DG_ZeroVector, &tmpMat ) ;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
	DG_GetPos( &tmpMat2 ) ;
	_sceVu0MulMatrix( &tmpMat, &tmpMat, &tmpMat2 ) ;
	GV_VecToMat( ( FVECTOR * )work->body.objs->world.m[ 3 ], &tmpMat ) ;
	DG_COPY_MAT( world, &tmpMat ) ;
}

/* 動的ハザードコールバック */
static	void	DynamicHzdCallback( void *hzd, void *ptr, void *ptr2, int flag )
{
	Work			*work ;
	FMATRIX			old, new ;

	work = ( Work * )ptr ;
	if ( flag & HZX_CALLBACK_FLAG_DESTROY ) {
		GV_DestroyActor( work ) ;
		return ;
	}
	/* 落とし穴 */
	/* その場で爆発 */
	if ( flag & HZX_CALLBACK_FLAG_PITFALL ) {
		work->flag |= FLAG_DESTROY | FLAG_HITTED ;
		return ;
	}
	
	DG_COPY_MAT( &old, &work->world ) ;
	HZX_CallbackUpdateMatrix( hzd, &old, &new ) ;
	DG_SetPos( &new ) ;
	DG_GetPos( &work->world ) ;
	DG_GetPos( &work->body.objs->world ) ;	

	MakeTargetWorld( &new, work ) ;
	GM_MoveTarget2Map( &work->touch, &new, work->map ) ;
	GM_MoveTarget2Map( &work->def, &new, work->map ) ;
}

static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;

    work = ( Work * )ptr ;
    if ( ( def->class & TARGET_NO_CLAYMORE ) ||
		 ( work->flag & FLAG_DESTROY ) ||
		 ( work->list.flag & ( GM_BMB_FLAG_NOBLAST | GM_BMB_FLAG_DESTROY ) ) ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}
	/* 間に壁がある */
	if ( HZX_OnlineHazardCheck( HZX_AllMapID, &work->center2, &def->center, HZX_CHK_ALL, 
							    HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
							    HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) {
		def->damaged &= ~TARGET_POWER ;
		def->weapon_type &= ~( off->weapon_type ) ;
		return ;
	}
	{
		Bomb( work ) ;
	}

    work->flag |= FLAG_DESTROY | FLAG_HIT ;
}

static	void	Hit2( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;

    work = ( Work * )ptr ;
    if ( work->flag & FLAG_DESTROY ) return ;
    if ( def->weapon_type == WP_LASERSIGHT ||
		 def->weapon_type == WP_COLDSPRAY ) return ;
	if ( work->list.flag & ( GM_BMB_FLAG_NOBLAST | GM_BMB_FLAG_DESTROY ) ) return ;
    work->flag |= FLAG_DESTROY ;
    if ( off->weapon_type & WP_BLAST ) work->delay = 8 ;
    else work->flag |= FLAG_HITTED ;
}

/* プロック実行 */
static	void	ExecProc( Work *work )
{
	GCL_ARGS	args ;
	int			buf[ 2 ] ;

	if ( work->proc <= 0 ) return ;
	args.argc = 2 ;
	args.argv = buf ;
	buf[ 0 ] = ( work->flag & FLAG_DESTROY ) ? 1 : 0 ;
	if ( buf[ 0 ] == 1 && ( work->flag & FLAG_COLLECTED ) ) buf[ 0 ] = 2 ;
	buf[ 1 ] = work->name ;
	/* ロード時に処理するのでForceを使用 */
	GM_ForceExecProc( work->proc, &args ) ;
}

/*---------------------------------------------------------------*/

static	void	Act( Work *work )
{
    GM_SetCurrentMap( work->map ) ;

    if ( work->list.flag & GM_BMB_FLAG_DESTROY ) {
	GV_DestroyActor( work ) ;
	return ;		
    }
    if ( work->list.flag & GM_BMB_FLAG_INVISIBLE ) {
	DG_InvisibleObjs( work->body.objs ) ;
	return ;		
    }

    /* 開始後die_countフレームからPLAYERチェックも開始 */
    if ( work->die_count < 0 ) {
	if ( ++ work->die_count >= 0 ) {
	    work->die_count = 0 ;
	    work->touch.side = BOTH_SIDE ;
	}
    }	

    /* 終了待ち */
    if ( work->die_count > 0 ) {
	if ( work->die_count == DIE_COUNT ) {
	    //if ( !( work->flag & FLAG_SCENARIO ) ) {
			work->die_count /= 4 ;
	    //}
	}
	if ( work->flag & FLAG_HITTED ) {
	    DG_InvisibleObjs( work->body.objs ) ;
	} else if ( work->flag & FLAG_HIT ) {
	    DG_VisibleObjs( work->body.objs ) ;
	}

	if ( work->die_count < DIE_COUNT / 2 ) {
	    DG_InvisibleObjs( work->body.objs ) ;
	}

	if ( -- work->die_count == 0 ) {
	    GV_DestroyActor( work ) ;
	}
	return ;		
    }

    /* 赤外線センサーで可視 */
    if ( PL_GetPlayerItem() == IT_Thermal )
	DG_VisibleObjs( work->body.objs ) ;		
    else
	DG_InvisibleObjs( work->body.objs ) ;

    {
	FVECTOR tmp;
	if ( work->delay > 0 ) {
	    if ( -- work->delay == 0 && !( work->list.flag & GM_BMB_FLAG_NOBLAST ) ) {
		if ( work->disp != NULL ) GV_DestroyOtherActorQuick( work->disp ) ;
		tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
		_sceVu0ApplyMatrix( &tmp , &work->body.objs->world , &tmp );
		_sceVu0Normalize( &tmp , &tmp );
		DG_SetPos( &work->body.objs->world );

		Bomb( work ) ;
		work->die_count = DIE_COUNT ;
		return ;
	    }
	}
	if ( work->flag & FLAG_HIT ) {
	    if ( work->disp != NULL ) GV_DestroyOtherActorQuick( work->disp ) ;
	    tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
	    _sceVu0ApplyMatrix( &tmp , &work->body.objs->world , &tmp );
	    _sceVu0Normalize( &tmp , &tmp );
	    GM_SeSetMode( SD_W_MINEEXP1 , &work->center , GM_SEMODE_BOMB ) ;
	    GM_SetNoise( NOISE_L, &work->center , GM_CurrentMap ) ;

	    work->die_count = DIE_COUNT ;
	    return ;
	} else if ( work->flag & FLAG_HITTED ) {
	    if ( work->disp != NULL ) GV_DestroyOtherActorQuick( work->disp ) ;
	    tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
	    _sceVu0ApplyMatrix( &tmp , &work->body.objs->world , &tmp );
	    _sceVu0Normalize( &tmp , &tmp );

	    Bomb( work ) ;

	    work->die_count = DIE_COUNT ;
	    return ;
	}
    }

#ifdef DEBUG_MODE	
    if ( work->flag & FLAG_DEBUG_DISP ) {
	NewTargetView2( &work->touch, 32, 230, 233 ) ;
	NewTargetView2( &work->def, 232, 30, 233 ) ;
    }
#endif

    /* 重いので4フレームに一回チェック */
    if ( ( GV_Time % 4 ) == work->checkN )
	if ( PL_PadEnable() && !PL_AttackDisable() )
	    GM_PutTarget( &work->touch ) ;

    /* 探知器装備中はレーダーに映る */
#if 0
    if ( PL_GetPlayerItem() == IT_MineDetector )
	GM_RadarSetFlag( &work->radar, RADAR_VISIBLE | RADAR_SIGHT ) ;
    else
	GM_RadarResetFlag( &work->radar, RADAR_VISIBLE | RADAR_SIGHT ) ;
#endif
}

static	void	Die( work )
Work		*work ;
{
	ExecProc( work ) ;
    GM_FreeObject( &work->body ) ;
    GM_FreeTarget( &work->def ) ;
	GM_FreeRadarControl( &work->radar ) ;
	GM_RemoveBombList( &work->list ) ;
	if ( ( work->flag & FLAG_SCENARIO ) &&
		 ( HZX_IsDynamic( &work->seg ) ) ) {
		HZX_RemoveDynamicCallback( work->seg.ptr, &work->dhcb ) ;
	}
    GM_N_Claymores -- ;
}

/*---------------------------------------------------------------*/

static	void	SetTarget( Work	*work, int name )
{
    TARGET	*t ;
    FVECTOR	size, offset ;
    FMATRIX	tWorld ;

    MakeTargetWorld( &tWorld, work ) ;

    t = &( work->touch ) ;
    size.vx = 400.0F ;
    size.vy = 400.0F ; 
    size.vz = 800.0F ;
    offset.vx = -550.0F ;
    offset.vy = -200.0F ;
    offset.vz = 400.0F ;
    if ( work->flag & FLAG_SCENARIO ) {
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_POWER | TARGET_ROTATE | 
		      TARGET_NO_CLAYMORE, 0,
		      BOTH_SIDE, &size, &offset ) ;
    } else {
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_POWER | TARGET_ROTATE |
		      TARGET_NO_CLAYMORE, 0,
		      ENEMY_SIDE, &size, &offset ) ;
	work->die_count = -DIRECT_TICK( 60 ) ;
    }
    GM_MoveTarget2( t, &tWorld ) ;
    GM_SetPowerTarget( t, &work->dummy, POWER_ONCE, 0, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, WP_THROWG|WP_NOPLAYER ) ;
    GM_SetTargetName( t, WP_Claymore ) ;
    GM_SetTargetCallBack( t, Hit, work ) ;

    _sceVu0ApplyMatrix( &offset, &t->world, &offset ) ;
    _sceVu0AddVector( &work->center, &t->center, &offset ) ;

    /* 自分に弾が当たっても爆発用ターゲット */
    t = &work->def ;
    size.vx = 25.0F ;
    size.vy = 150.0F ;
    size.vz = 80.0F ;
    offset.vx = -15.0F ;
    offset.vy = -200.0F ;
    offset.vz = 0.0F ;
    GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_ROTATE | TARGET_SEEK | TARGET_NO_CLAYMORE, 0,
		  BOTH_SIDE, &size, &offset ) ;
    GM_MoveTarget2( t, &tWorld ) ;
    GM_SetPowerTarget( t, &work->dummy, POWER_ONCE, 0, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, Hit2, work ) ;
    GM_PutTarget( t ) ;
    GM_SetTargetName( t, name ) ;

    _sceVu0ApplyMatrix( &offset, &t->world, &offset ) ;
    _sceVu0AddVector( &work->center2, &t->center, &offset ) ;
}

/* レーダーコントロール初期化 */
/* center2を設定してから呼ぶこと */
static	void	InitRadar( Work *work, FMATRIX *world, int where )
{
    FVECTOR	n = { 0.0F, 0.0F, 10.0F } ;
    int		dir ;

    DG_SetPos( world ) ;
    DG_RotVector( &n, &n, 1 ) ;
    dir = GV_VecDir2( &n );

    GM_InitRadarControl( &work->radar, &work->center2, RADAR_NOFIX_SIGHT, where ) ;
    GM_RadarSetSight( &work->radar, dir, 1024, 2200.0F, RADAR_COLOR_YELOW ) ;
    GM_RadarSetVRange( &work->radar, 2000.0F, -2000.0F );
    GM_RadarSetRangeRate( &work->radar, 1.0F );
    GM_RadarSetFlag( &work->radar, RADAR_MINE | RADAR_SIGHT | RADAR_VISIBLE ) ;
    //GM_RadarResetFlag( &work->radar, RADAR_VISIBLE ) ;
}


/*----------------------------------------------------------------*/

/* シナリオ起動 */
static	int	GetResources( Work *work, int name, int where )
{
    SVECTOR			rot ;
    FVECTOR			mov ;
    FVECTOR			adj = { -15.0F, 200.0F, 135.0F } ;
    FMATRIX			world ;
    HZX_GROUP_ID	hzx_id ;

    work->name = name ;

    PL_GetOptionFV( 'p', &mov ) ;
    PL_GetOptionSV( 'r', &rot ) ;

    work->proc = GCL_GetOptionValue( 'O', 0 ) ;

    rot.vx += -1024 ;
    rot.vy += 2048 ;
    rot.vz += -1024 - ( 30 * 1024 / 90 ) ;
    DG_SetPos2( &mov, &rot ) ;
    DG_MovePos( &adj ) ;
    DG_GetPos( &world ) ;
    GV_MatToVec( &world, &mov ) ;

    GM_InitObject( &( work->body ), BODY_NAME, BODY_FLAG ) ;
    GM_GroupObjs( work->body.objs, where ) ;
    work->body.map_name = where ;
    GM_ConfigObjectLight( &work->body, work->light ) ;

    DG_GetPos( &work->body.objs->world ) ;
    GV_MatToVec( &work->body.objs->world, &mov ) ;
    DG_GetLightMatrixFix( &mov, work->light ) ;

    work->world = world ;
    work->side = BOTH_SIDE ;

    work->type = TYPE_FLOOR ;
    work->flag = FLAG_SCENARIO ;
    hzx_id = GM_GetHzxGroupID( where ) ;
    if ( ( HZX_LevelHazardCheck( hzx_id, &mov, HZX_CHK_ALL, 
				 HZX_FLOOR_NO_CLAYMORE | HZX_FLOOR_RECOIL_TYPE ) ) & 1 ) {
	HZX_HZD		flr[ 2 ] ;
	int			atr[ 2 ] ;
		
	HZX_GetLevelHazard( flr, atr ) ;
	work->seg = flr[ 0 ] ;
	/* 今はシナリオ起動のみ動的ハザード処理 */
	if ( HZX_IsDynamic( &work->seg ) ) {
	    HZX_SetDynamicCallback( work->seg.ptr, &work->dhcb, DynamicHzdCallback,
				    ( void * )work, NULL ) ;
	}
    } 

    SetTarget( work, 13336174/*claymore*/ ) ;
    InitRadar( work, &world, where ) ;

    work->map = where ;

    work->list.weapon = WP_Claymore ;
    work->list.mov = &work->center2 ;
    work->list.flag = 0 ;
    GM_AddBombList( &work->list ) ;

    work->checkN = GM_N_Claymores % 4 ;
    GM_N_Claymores ++ ;	/* シナリオ置きもカウント */

    /* シナリオ置きは最初不可視 */
    DG_InvisibleObjs( work->body.objs ) ;

    /* デバッグ用 */
#ifdef DEBUG_MODE
    {
	int			flag ;

	flag = GCL_GetOptionValue( 'f', 0 ) ;
	if ( flag & 0x0001 ) work->flag |= FLAG_DEBUG_DISP ;
    }
#endif	
    return 0 ;
}

void	*NewEmmaClaymore( int name, int where )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
