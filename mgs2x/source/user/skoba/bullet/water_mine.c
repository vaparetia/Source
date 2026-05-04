//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   water_mine.c
   水中機雷
   
   2001/02/20 M.Sonoyama
   $Id: water_mine.c,v 1.1.1.3 2002/11/19 11:50:02 Yoshizawa1 Exp $
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
#endif

#include	"gameheader.h"
#include	"camera.h"

/*------------------------------------------------------------*/

extern	void	*NewBlastWater( FVECTOR *, int, int, int, int, int, int ) ;

#define	BODY_NAME	(GV_StrCode( "watermine" ))
#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_IRREACTION)

typedef struct _Work {
    GV_ACT_EX		actor ;
    OBJECT			body ;
    FMATRIX			light[ 2 ] ;
	RADAR_CTRL		radar ;
    FMATRIX			world ;
	FVECTOR			pos ;
	FVECTOR			step ;
    TARGET			attack ;
    POWER_TARGET	power ;
    int				side ;
    int				map ;
    int				flag ;
	HZX_PAT			*pat ;
	int				curNo ;
	int				nextNo ;
	HZX_PTP			*cur_ptp ;
	HZX_PTP			*next_ptp ;
	float			speed ;
	int				name ;
	int				proc ;
} Work ;

enum {
	FLAG_NONE =			0x0000,
	FLAG_HIT =			0x0001,
	FLAG_ROUTE =		0x0002,		/* ルート移動あり */
	FLAG_ROUTE_REV =	0x0004,		/* ルート逆移動中 */
} ;

/*---------------------------------------------------------------*/

static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;

    work = ( Work * )ptr ;
	/* 間に壁がある */
	if ( HZX_OnlineHazardCheck( 0, &work->pos, &def->center, HZX_CHK_ALL, 
							    HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET ) ) {
		def->damaged &= ~TARGET_POWER ;
		def->weapon_type &= ~( off->weapon_type ) ;
		return ;
	}
	work->flag |= FLAG_HIT ;
}

/*---------------------------------------------------------------*/

static	void	MoveRoute( Work *work )
{
	FVECTOR		mov, step, aim ;
	int			n_points ;

	if ( !( work->flag & FLAG_ROUTE ) ) return ;
	n_points = work->pat->n_points ;
	if ( n_points < 2 ) return ;
	GV_MatToVec( &work->body.objs->world, &mov ) ;
	GV_SetVec3( &aim, work->next_ptp->x, work->next_ptp->y, work->next_ptp->z ) ;
	if ( GV_VecLen3F2( &aim, &mov ) < work->speed ) {
		/* 次のポイント設定 */
		work->curNo = work->nextNo ;
		work->cur_ptp = work->next_ptp ;
		if ( work->flag & FLAG_ROUTE_REV ) {
			if ( -- work->nextNo < 0 ) {
				work->nextNo = 1 ;
			}
		} else {
			if ( ++ work->nextNo >= n_points ) {
				work->nextNo = n_points - 2 ;
			}			
		}
		work->next_ptp = work->pat->points + work->nextNo ;
	} 
	step.vx = work->next_ptp->x - work->cur_ptp->x ;
	step.vy = work->next_ptp->y - work->cur_ptp->y ;
	step.vz = work->next_ptp->z - work->cur_ptp->z ;
	GV_LenVec3F( &step, &step, 0.0F, work->speed ) ;
	work->body.objs->world.m[ 3 ][ 0 ] += step.vx ;
	work->body.objs->world.m[ 3 ][ 1 ] += step.vy ;
	work->body.objs->world.m[ 3 ][ 2 ] += step.vz ;
}

/* ゲームオーバー時、プレイヤー等の近くなら爆発 */
static	void	CheckGameOverBlast( Work *work ) 
{
	CONTROL		*ctrl ;
	int			i ;
	float		len ;

	if ( !GM_IsGameOver() ) return ;

	GM_ResetPlayerStatusEX( I64(0), PLAYER2_WATERMINE_NEAR ) ;
	for ( i = 0; i < GM_N_WhereList; i ++ ) {
		ctrl = GM_WhereList[ i ] ;
		if ( ctrl == GM_PlayerControl ||
			( ctrl->attribute & CTRL_ATR_NPC ) ) {
			len = GV_VecLen3F2( &work->pos, &ctrl->mov ) ;
			if ( len < 3000.0F ) GM_SetPlayerStatusEX( I64(0), PLAYER2_WATERMINE_NEAR ) ;
			if ( len < 1000.0F ) {
				work->flag |= FLAG_HIT ;
				break ;
			}
		}
	}
}

/*---------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    GM_SetCurrentMap( work->map ) ;

	/* ゲームオーバー時、プレイヤー等の近くなら爆発 */
	CheckGameOverBlast( work ) ;

	if ( work->flag & FLAG_HIT ) {
		NewBlastWater( &work->pos, BOTH_SIDE, 2000, 4000, DMG_BLAST, FNT_BLAST, WP_C4Bomb ) ;
		GV_DestroyActor( work ) ;
		return ;
	}

	/* ルート移動処理 */
	MoveRoute( work ) ;

//    NewTargetView2( &work->attack, 32, 230, 233 ) ;
	DG_GetLightMatrix( &work->pos, work->light ) ;
	if ( PL_PadEnable() && !PL_AttackDisable() ) {
		GM_MoveTarget2( &work->attack, &work->body.objs->world ) ;
		GM_PutTarget( &work->attack ) ;
	}
	/* 探知器装備中はレーダーに映る */
	if ( PL_GetPlayerItem() == IT_MineDetector ) {
		GM_RadarSetFlag( &work->radar, RADAR_VISIBLE ) ;
	} else {
		GM_RadarResetFlag( &work->radar, RADAR_VISIBLE ) ;
	}

	/* カメラ近くで消える */
	{
		GM_CameraSet	*cur ;

		cur = GM_GetCurrentCamera( 0 ) ;
		if ( GV_VecLen3F2( &cur->position, &work->pos ) < 500.0F ) {
			DG_InvisibleObjsChanl( work->body.objs, 0 ) ;
		} else {
			DG_VisibleObjsChanl( work->body.objs, 0 ) ;
		}
	}
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &work->body ) ;
	GM_FreeRadarControl( &work->radar ) ;

	if ( work->proc > 0 ) {
		GCL_ARGS	args ;
		int			buf[ 2 ] ;
		
		args.argv = buf ;
		args.argc = 2 ;
		buf[ 0 ] = work->name ;
		if ( work->flag & FLAG_HIT ) {		
			buf[ 1 ] = 1 ;
		} else {
			buf[ 1 ] = 0 ;
		}
		GM_ForceExecProc( work->proc, &args ) ;
	}
	/* 消えるのは爆発かステージ終了なので、
	   ここでリセットしてもいいはず */
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_WATERMINE_NEAR ) ;
}

/*---------------------------------------------------------------*/

static	void	SetTarget( work )
Work		*work ;
{
    TARGET	*t ;
    FVECTOR	size, offset ;

    t = &( work->attack ) ;
    size.vx = 150.0F ;	/* ちょっと小さ目 */
    size.vy = 150.0F ; 
    size.vz = 150.0F ;
    offset.vx = 0.0F ;
    offset.vy = 0.0F ;
    offset.vz = 0.0F ;

    GM_SetTarget( t, TARGET_OFFENSE | TARGET_POWER, 0,
				  BOTH_SIDE, &size, &offset ) ;
    GM_MoveTarget2( t, &work->body.objs->world ) ;
    GM_SetPowerTarget( t, &work->power, POWER_ONCE, 0, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, WP_THROWG ) ;
    GM_SetTargetCallBack( t, Hit, work ) ;
}

/* レーダーコントロール初期化 */
static	void	InitRadar( Work *work, FMATRIX *world, int where )
{
	FVECTOR	n = { 0.0F, 0.0F, 10.0F } ;
	int		dir ;

	DG_SetPos( world ) ;
	DG_RotVector( &n, &n, 1 ) ;
	dir = GV_VecDir2( &n ) ;

	GM_InitRadarControl( &work->radar, &work->pos, 0, where ) ;
	GM_RadarSetSight( &work->radar, dir, 768, 1000.0F, RADAR_COLOR_YELOW ) ;
	GM_RadarResetFlag( &work->radar, RADAR_VISIBLE ) ;
}

static	int	GetResources( Work *work, int name, int where )
{
	FMATRIX		world ;
    SVECTOR		rot ;
    FVECTOR		mov ;
	
	if ( !PL_GetOptionFV( 'p', &mov ) ) return -1 ;
	if ( !PL_GetOptionSV( 'r', &rot ) ) return -1 ;
	DG_SetPos2( &mov, &rot ) ;
	DG_GetPos( &world ) ;
	DG_COPY_MAT( &work->world, &world ) ;
	DG_COPY_VEC( &work->pos, &mov ) ;
    work->side = BOTH_SIDE ;
	work->map = where ;

    GM_InitObject( &( work->body ), BODY_NAME, BODY_FLAG ) ;
    GM_ConfigObjectLight( &work->body, work->light ) ;

	DG_PutObjs( work->body.objs ) ;
	work->body.map_name = where ;
	GM_GroupObjs( work->body.objs, where ) ;

    SetTarget( work ) ;
	InitRadar( work, &world, where ) ;

	/* ルート読み込み */
	if ( GCL_GetOption( 'O' ) != NULL ) {
		int		patNo ;

		work->flag |= FLAG_ROUTE ;
		patNo = GCL_GetNextInt() ;
		work->pat = HZX_GetCurrentHzx()->def->patrols + patNo ;
		work->curNo = 0 ; 
		work->nextNo = 1 ;
		work->cur_ptp = work->pat->points + 0 ;
		work->next_ptp = work->pat->points + 1 ;
		work->speed = ( float )GCL_GetOptionValue( 's', 16.0F ) ;
	}

	{
		extern void *NewWaterMineLamp( FMATRIX *world );
		GV_SetActorChild( work, NewWaterMineLamp( &work->body.objs->world ) );
	}

	/* 終了プロック */	
	work->name = name ;
	work->proc = GCL_GetOptionValue( 'R', 0 ) ;

    return 0 ;
}

/* 水中機雷起動 */
void	*NewWaterMine( int name, int where )
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}
