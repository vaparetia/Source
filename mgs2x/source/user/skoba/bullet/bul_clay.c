//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_clay.c
   セットされたＣｌａｙｍｏｒｅ
   
   2000/02/23 M.Sonoyama
   $Id: bul_clay.c,v 1.1.1.3 2002/11/19 11:50:00 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
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
    GV_ACT_EX		actor ;
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
	int				dir ;
	GM_BOMB			list ;
	HZX_D_CALLBACK	dhcb ;
} Work ;

extern void *NewClayBomb( int num , FMATRIX *world, FVECTOR *shift , FVECTOR *pPower , int life, float alpha );
extern void *NewClayBombNormal( int num , FMATRIX *world , FVECTOR *shift , FVECTOR *pPower , int life , float alpha );
extern void *NewClayBombFlag( int num , FMATRIX *world , FVECTOR *shift , FVECTOR *pPower , int life , float alpha,
							  long64 flag );
extern void	*NewBlast3( FVECTOR * , int , int , int , int , int , int , int );

/*---------------------------------------------------------------*/

/* 爆発 */
static	void	Bomb( Work *work )
{
	FMATRIX fmtmp;
	FVECTOR speed;
	
	GM_SetCurrentMap( work->map ) ;

	speed = DG_ZeroVector;
	speed.vx = 700.0f;
	
	fmtmp = work->touch.world;
	*( FVECTOR * )fmtmp.m[ 3 ] = work->center2;
    GM_SeSetMode( SD_W_MINEEXP1 , &work->center , GM_SEMODE_BOMB ) ; // se
	GM_SetNoise( NOISE_L, &work->center , GM_CurrentMap ) ;
	if ( work->flag & FLAG_SCENARIO ) {
		/* シナリオ置きは、NOPLAYER */
		NewClayBombFlag( 10 , &fmtmp, &DG_ZeroVector , &speed , 10 , 128, WP_NOPLAYER ) ;
	} else {
		NewClayBombNormal( 10 , &fmtmp, &DG_ZeroVector , &speed , 10 , 128 ) ;
	}
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
		HZX_RemoveDynamicCallback( work->seg.ptr, &work->dhcb ) ;
		work->seg.ptr = NULL ;
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
	/* しゃがみ、匍匐中＆強制中は当たらない */
	if ( GM_PlayerControl != NULL && 
		 def->name == GM_PlayerControl->name && 
		( GM_PlayerStance != 0 || GM_CheckPlayerStatus( PLAYER_FORCE ) ) ) {
		if ( off->weapon_type == def->weapon_type ) {
			def->damaged &= ~TARGET_POWER ;
		}
		def->weapon_type &= ~( off->weapon_type ) ;
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
    if ( off->weapon_type == WP_LASERSIGHT ||
		 off->weapon_type == WP_STUNGRENADE ||
		 off->weapon_type == WP_STUNFAR ||
		 off->weapon_type == WP_CHAFFGRENADE ||
		 off->weapon_type == WP_COLDSPRAY ) return ;
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

/* 回収処理 */
static	int	Collection( Work *work )
{
	int		res ;
	CVECTOR	col ;
   int pan;
   float bp_angle;

	if ( GM_IsGameOver() ) return 0 ;
	if ( GM_PlayerControl != NULL && GM_PlayerStance == 2 ) {
		if ( GV_VecLen3F2( &work->center2, 
						  ( FVECTOR * )GM_PlayerBody->objs->world.m[ 3 ] ) < 650.0F ) {
			res = GM_IncrementWeapon( WP_Claymore, 1 ) ; 
			if ( res == GM_ITEMGET_OK ) {
				col.r = col.g = col.b = 196 ;
				col.cd = 128 ;
				GV_DestroyChild( work ) ;
				//GV_SetActorChild( work, NewDispStringsWorld2Screen( &work->center2, "CLAYMORE", col ) ) ;
				/* 直後にデストロイされるのでＣＨＩＬＤにはしない */
				NewDispStringsWorld2Screen( &work->center2, "CLAYMORE", col ) ;
            pan = GM_SeGetPan( &work->center2, GM_SEMODE_BOMB, &bp_angle );
				GM_SeSet3D( pan, GM_MAX_VOL, SD_S_IGET02, bp_angle ) ;
				return 1 ;
			} else if ( res == GM_ITEMGET_FULL ) {
				col.r = 196 ; col.g = col.b = 64 ;
				col.cd = 128 ;				
				if ( !( work->flag & FLAG_CANNOT_COLLECT ) ) {
					GV_DestroyChild( work ) ;
					GV_SetActorChild( work, NewDispStringsWorld2Screen( &work->center2, "FULL", col ) ) ;
               pan = GM_SeGetPan( &work->center2, GM_SEMODE_BOMB, &bp_angle );
					GM_SeSet3D( pan, GM_MAX_VOL, SD_S_FULL0005, bp_angle ) ;
				}
				DG_VisibleObjs( work->body.objs ) ;
				work->flag |= FLAG_CANNOT_COLLECT ;
			}
		} else {
			work->flag &= ~FLAG_CANNOT_COLLECT ;
		}
	}
	return 0 ;
}

/*---------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
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

	//NewTargetView2( &work->touch, 32, 32, 232 ) ;
	//NewPointView( &work->center2, 32, 232, 32 ) ;

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
				/* 全部１／４ */
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
	if ( work->flag & FLAG_SCENARIO ) {
		if ( work->die_count <= 0 && PL_GetPlayerItem() == IT_Thermal ) {
			DG_VisibleObjs( work->body.objs ) ;		
		} else {
			DG_InvisibleObjs( work->body.objs ) ;
		}
	}

	{
		FVECTOR tmp;
	    if ( work->delay > 0 ) {
			if ( -- work->delay == 0 && !( work->list.flag & GM_BMB_FLAG_NOBLAST ) ) {
				GV_DestroyChild( work ) ;
				tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
				_sceVu0ApplyMatrix( &tmp , &work->body.objs->world , &tmp );
				_sceVu0Normalize( &tmp , &tmp );
				DG_SetPos( &work->body.objs->world );

				Bomb( work ) ;

//				NewBlast3( &work->center, work->side, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Claymore, BLAST_TYPE_NO_FIRE );
//				NewSKLandingSmoke( &work->center, &tmp , 600.f );
//				NewBlast( &work->center, work->side, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Claymore ) ;
				work->die_count = DIE_COUNT ;
				return ;
			}
    	}
		if ( work->flag & FLAG_HIT ) {
			GV_DestroyChild( work ) ;
			tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
			_sceVu0ApplyMatrix( &tmp , &work->body.objs->world , &tmp );
			_sceVu0Normalize( &tmp , &tmp );
		    GM_SeSetMode( SD_W_MINEEXP1 , &work->center , GM_SEMODE_BOMB ) ; // se
			GM_SetNoise( NOISE_L, &work->center , GM_CurrentMap ) ;
//			NewBlast3( &work->center, work->side, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Claymore , BLAST_TYPE_NO_FIRE );
//			NewSKLandingSmoke( &work->center , &tmp , 600.f );
//			NewBlast( &work->center, work->side, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Claymore ) ; 
			work->die_count = DIE_COUNT ;
			return ;
		} else if ( work->flag & FLAG_HITTED ) {
			GV_DestroyChild( work ) ;
			tmp.vx = tmp.vy = tmp.vz = tmp.vw = 1.0f;
			_sceVu0ApplyMatrix( &tmp , &work->body.objs->world , &tmp );
			_sceVu0Normalize( &tmp , &tmp );

			Bomb( work ) ;

//			NewBlast3( &work->center, work->side, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Claymore, BLAST_TYPE_NO_FIRE );
//			NewSKLandingSmoke( &work->center , &tmp , 600.f );
//			NewBlast( &work->center2, work->side, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Claymore ) ; 
			work->die_count = DIE_COUNT ;
			return ;
		}
	}
	/* 回収処理 */
	if ( Collection( work ) ) {
		work->flag |= FLAG_DESTROY | FLAG_COLLECTED ;
		GV_DestroyActor( work ) ;
		work->die_count = DIE_COUNT ;
		return ;
	}
#ifdef DEBUG_MODE	
	if ( work->flag & FLAG_DEBUG_DISP ) {
		NewTargetView2( &work->touch, 32, 230, 233 ) ;
		NewTargetView2( &work->def, 232, 30, 233 ) ;
	}
#endif
    /* 重いので4フレームに一回チェック */
    if ( ( GV_Time % 4 ) == work->checkN ) {
		if ( PL_PadEnable() && !PL_AttackDisable() ) GM_PutTarget( &work->touch ) ;
	}
	/* 探知器装備中はレーダーに映る */
#if 0
	if ( PL_GetPlayerItem() == IT_MineDetector ) {
		GM_RadarSetFlag( &work->radar, RADAR_VISIBLE | RADAR_SIGHT ) ;
	} else {
		GM_RadarResetFlag( &work->radar, RADAR_VISIBLE | RADAR_SIGHT ) ;
	}
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
	if ( HZX_IsDynamic( &work->seg ) ) {
		HZX_RemoveDynamicCallback( work->seg.ptr, &work->dhcb ) ;
	}
    GM_N_Claymores -- ;
}

/*---------------------------------------------------------------*/

static	void	SetTarget( work )
Work		*work ;
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
    offset.vz = 450.0F ;
	if ( work->flag & FLAG_SCENARIO ) {
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_POWER | TARGET_ROTATE |
					  TARGET_THROUGH |
					  TARGET_NO_CLAYMORE, 0,
					  BOTH_SIDE, &size, &offset ) ;
	} else {
		offset.vx = -850.0F ;
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_POWER | TARGET_ROTATE |
					  TARGET_THROUGH |
					  TARGET_NO_CLAYMORE, 0,
					  ENEMY_SIDE, &size, &offset ) ;
		work->die_count = -DIRECT_TICK( 60 ) ;
	}
    GM_MoveTarget2( t, &tWorld ) ;
    GM_SetPowerTarget( t, &work->dummy, POWER_ONCE, 0, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, WP_THROWG ) ;

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

    _sceVu0ApplyMatrix( &offset, &t->world, &offset ) ;
    _sceVu0AddVector( &work->center2, &t->center, &offset ) ;

	work->dir = GV_VecDir2FromTo( &work->center2, &work->center ) ;
}

/* レーダーコントロール初期化 */
/* center2, dirを設定してから呼ぶこと */
static	void	InitRadar( Work *work, FMATRIX *world, int where )
{
	GM_InitRadarControl( &work->radar, &work->center2, RADAR_NOFIX_SIGHT, where ) ;
	GM_RadarSetSight( &work->radar, work->dir, 1024, 2200.0F, RADAR_COLOR_YELOW ) ;
	GM_RadarSetVRange( &work->radar, 2000.0F, -2000.0F );
	GM_RadarSetRangeRate( &work->radar, 1.0F );
	GM_RadarSetFlag( &work->radar, RADAR_MINE | RADAR_SIGHT | RADAR_VISIBLE ) ;
	//GM_RadarResetFlag( &work->radar, RADAR_VISIBLE ) ;
}

static	int	GetResources( work, side, world, seg, type ) 
Work		*work ;
int		side ;
FMATRIX		*world ;
HZX_HZD		*seg ;
u_int		type ;
{
    //SVECTOR	rot ;
    FVECTOR	mov ;

    work->world = *world ;
	//    work->side = side ;
    work->side = BOTH_SIDE ;

    work->type = TYPE_FLOOR ;
    work->flag = FLAG_PLAYER ;
	work->seg = *seg ;
    GM_InitObject( &( work->body ), BODY_NAME, BODY_FLAG ) ;
	if ( work->body.objs == NULL ) return -1 ;
    GM_ConfigObjectLight( &work->body, work->light ) ;
    DG_SetPos( world ) ;
#if 0
    rot.vx = -1024 ; 
    rot.vy = 2048 ; 
    rot.vz = -1024 ;
    DG_RotatePos( &rot ) ;	
#endif
    DG_GetPos( &work->body.objs->world ) ;
    GV_MatToVec( &work->body.objs->world, &mov ) ;
    DG_GetLightMatrixFix( &mov, work->light ) ;

    SetTarget( work ) ;
	InitRadar( work, world, GM_CurrentMap ) ;

	{
		HZX_ZONE_ADD	zone[ 2 ] ;
		int				hzx_id, map ;

		HZX_GetInterruptZone( &work->center2, 0, 100, &zone[ 0 ] ) ;
		hzx_id = GV_GetBit( HZX_ZoneMapNo( zone[ 0 ] ) ) ;
		hzx_id |= GV_GetBit( HZX_ZoneMapNo( zone[ 1 ] ) ) ;
		map = GM_GetMapIDfromHzxGroupID( hzx_id ) ;
		GM_GroupObjs( work->body.objs, map ) ;
	}


	/* 動的ハザード処理 */
	if ( HZX_IsDynamic( &work->seg ) ) {
		HZX_SetDynamicCallback( work->seg.ptr, &work->dhcb, DynamicHzdCallback,
							    ( void * )work, NULL ) ;
	}

    work->map = GM_CurrentMap ;

	work->list.weapon = WP_Claymore ;
	work->list.mov = &work->center2 ;
	work->list.flag = 0 ;
	GM_AddBombList( &work->list ) ;

	work->checkN = GM_N_Claymores % 4 ;
    GM_N_Claymores ++ ;
    return 0 ;
}

/* Ｃｌａｙｍｏｒｅ起動 */
void	*NewBulletClaymore( world, side, seg, type )
FMATRIX		*world ;
int			side ;
HZX_HZD		*seg ;
int			type ;
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, side, world, seg, type ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		if ( GM_ShootCount < 30000 ) ++ GM_ShootCount ;
		GM_SetWeaponFire( WP_Claymore ) ;
    }
    return work ;
}

/*----------------------------------------------------------------*/

/* シナリオ起動 */
static	int	GetResourcesScn( Work *work, int name, int where )
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
	if ( work->body.objs == NULL ) return -1 ;
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
		/* 動的ハザード処理 */
		if ( HZX_IsDynamic( &work->seg ) ) {
			HZX_SetDynamicCallback( work->seg.ptr, &work->dhcb, DynamicHzdCallback,
								    ( void * )work, NULL ) ;
		}
	} 

    SetTarget( work ) ;
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

void	*NewScenarioClaymore( int name, int where )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResourcesScn( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
