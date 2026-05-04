//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*-----------------------------------------------------
  target.c 
  キャラクタ当たり判定システム (character collision detection system)
  
  1999/07/07 M.Sonoyama
  $Id: target.c,v 1.4 2002/12/12 06:00:56 takaki Exp $			   
  
  *** PROJECT OF METAL GEAR SOLID 2 ***
  
  -----------------------------------------------------
  
  ### ターゲットシステム起動 ###
  void	*GM_StartTargetSystem( void * )
  
  ### ターゲットシステム初期化 ###
  void	GM_InitTargetSystem( void * )
  
  ### 防御ターゲットをリストから削除 ###
  void	GM_FreeTarget( def )
  TARGET		*def ;
  
  ### ターゲット構造体を初期化 ###
  void	GM_SetTarget( targ, class, map, side, size, offset ) 
  TARGET		*targ ;		ターゲットへのポインタ
  u_int		class ;		判定クラス
  u_int		map ;		マップ
  u_int		side ;		敵味方種別
  FVECTOR		*size ;		ターゲットサイズ
  FVECTOR		*offset ;	ターゲット中心補正
  
  ### ターゲット登録 ###
  void	GM_PutTarget( targ )
  TARGET		*targ ;		ターゲットへのポインタ
  
  ### ターゲットを移動 ###
  void	GM_MoveTarget( targ, mov )
  TARGET		*targ ;		ターゲットへのポインタ
  FVECTOR		*mov ;		中心位置
  
  ### ターゲットを移動（マトリクス指定） ###
  void	GM_MoveTarget2( targ, mov )
  TARGET		*targ ;		ターゲットへのポインタ
  FMATRIX		*mov ;		中心位置（回転角度つき）
  
  ### ターゲットを移動（オンライン攻撃用）###
  void	GM_MoveOnlineTarget( targ, from, to )
  TARGET		*targ ;		ターゲットへのポインタ
  FVECTOR		*from ;		検索直線開始位置
  FVECTOR		*to ;		検索直線終端位置
  
  ### 子ターゲットを設定 ###
  void	GM_SetTargetParts( targ, child, n, level )
  TARGET		*targ ;		ターゲットへのポインタ
  TARGET		*child ;	子ターゲットへのポインタ
  int		n ;		子ターゲット数
  int		level ;		子ターゲット優先順位
  
  ### 武器タイプを設定 ###
  void	GM_SetTargetWeaponType( targ, type )
  TARGET		*targ ;		ターゲットへのポインタ
  long64		type ;		判定武器種類
  
  ### コールバック設定 ###
  void	GM_SetTargetCallBack( targ, func, ptr )
  TARGET		*targ ;		ターゲットへのポインタ
  TARGET_CALLBACK	func ;		コールバック関数
  void		*ptr ;		第３引数指定
  
  ### 打撃系ターゲット設定 ###
  void	GM_SetPowerTarget( targ, power, p_type, vital, faint, damage, force )
  TARGET			*targ ;		ターゲットへのポインタ
  POWER_TARGET		*power ;	打撃ターゲットへのポインタ
  u_int			p_type ;	打撃タイプ       
  u_short			vital ;		耐久力
  u_short			faint ;		気絶値
  u_short			damage ;	攻撃値
  FVECTOR			*force ;	力積
  
  ### つかみターゲット設定 ###
  void	GM_SetCaptureTarget( targ, capture, ctrl, body )
  TARGET		*targ ;			ターゲットへのポインタ
  CAPTURE_TARGET	*capture ;		つかみターゲットへのポインタ
  CONTROL		*ctrl ;			自分のCONTROL
  OBJECT		*body ;			自分のOBJECT
  
  ### プッシュターゲット設定 ###
  void	GM_SetPushTarget( targ, push, depth )
  TARGET		*targ ;			ターゲットへのポインタ
  PUSH_TARGET	*push ;			プッシュターゲットへのポインタ
  float		depth ;			プッシュ量比率
  
  ### くっつけターゲット設定 ###
  void	GM_SetStickTarget( targ, stick, world )
  TARGET		*targ ;			ターゲットへのポインタ
  STICK_TARGET	*stick ;		くっつけターゲットへのポインタ
  FMATRIX		*world ;		くっつけ位置
  
  ### ダメージ計算 ###
  int	GM_DamageTarget( off, def )
  TARGET		*off ;			攻撃ターゲット
  TARGET		*def ;			防御ターゲット
  
  ### プッシュ計算 ###
  int	GM_PushTarget( off, def )
  TARGET		*off ;			攻撃ターゲット
  TARGET		*def ;			防御ターゲット
  
  ### つかみ処理 ###
  void	GM_CaptureTarget( off, def ) 
  TARGET		*off ;			攻撃ターゲット
  TARGET		*def ;			防御ターゲット
  
  ### 打撃ダメージクリア ###
  void	GM_ClearTargetDamage( def )
  TARGET		*def ;			ターゲット
  
  ### プッシュ量クリア ###
  void	GM_ClearTargetPush( def )
  TARGET		*def ;			ターゲット
  
  ---------------------------------------------------*/
#endif

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libutl.h"

#include "bp_matrix.h"

//#define	DEBUG_FLAG

#ifdef DEBUG_FLAG
static	int	TrgDebugFlag ;
#endif

/*----------------------------------------------------------------*/

typedef struct	_Work {
    GV_ACT		actor ;
    TARGET		*DefenseList ;
    TARGET		*OffenseList ;
    TARGET		*DefListLast ;
    TARGET		*OffListLast ;

    TARGET		FirstDefTarget ;
    TARGET		FirstOffTarget ;
    int			N_DefenseTargets ;
    int			N_OffenseTargets ;
#ifdef DEBUG_MODE
	int			callbackflag ;
#endif
} Work ;

static	Work	*TargetWork ;
//static	Work	TargetWorkStatic ;

static	POWER_TARGET  	DummyPowerTarget ;

typedef struct	{
    FVECTOR	from ;
    FVECTOR	to ;
    FVECTOR	min_hit ;
    FVECTOR	min_hit2 ;
    FVECTOR	tmp_hit ;
    FVECTOR	normal ;
    FVECTOR	min_hit_normal ;
    FVECTOR	ov[ 8 ] ;
    FVECTOR	dv[ 8 ] ;    
    TARGET	min_target ;
    TARGET	min_target2 ;
    TARGET	tmp_target ;
    TARGET	tmp_target2 ;
    TARGET	*min_def_targ ;
    TARGET	*min_def_targ2 ;
	int		flag ;
} ScrPad ;

static	ScrPad	ScrBuf ;

//#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)
#define	SCRPAD		(&ScrBuf)

#define	FROM		(&(SCRPAD->from))
#define	TO		(&(SCRPAD->to))
#define	MIN_HIT		(&(SCRPAD->min_hit))
#define	TMP_HIT		(&(SCRPAD->tmp_hit))
#define	NORMAL		(&(SCRPAD->normal))
#define	MIN_NORMAL	(&(SCRPAD->min_hit_normal))

#define	MIN_TARG	(&(SCRPAD->min_target))
#define	MIN_DEF_TARG	(SCRPAD->min_def_targ)

#define	MIN_HIT2	(&(SCRPAD->min_hit2))
#define	MIN_TARG2	(&(SCRPAD->min_target2))
#define	MIN_DEF_TARG2	(SCRPAD->min_def_targ2)

#define	OV		(SCRPAD->ov)
#define	DV		(SCRPAD->dv)
#define	TMP_TARG	(&(SCRPAD->tmp_target))
#define	TMP_TARG2	(&(SCRPAD->tmp_target2))

#define	FLAG	(SCRPAD->flag)

#define	MAX_DISTANCE	(1000000.0F)

/*-----------------------------------------------------------------------------*/

/* ターゲットシステム初期化 */
void	GM_InitTargetSystem( void )
{
    ASSERT( TargetWork != NULL ) ;
    TargetWork->DefenseList = &TargetWork->FirstDefTarget ;
    TargetWork->OffenseList = &TargetWork->FirstOffTarget ;
    TargetWork->DefenseList->next = NULL ;
    TargetWork->DefenseList->next = NULL ;
    TargetWork->DefListLast = &TargetWork->FirstDefTarget ;
    TargetWork->OffListLast = &TargetWork->FirstOffTarget ;
    TargetWork->N_DefenseTargets = 0 ;
    TargetWork->N_OffenseTargets = 0 ;

	GV_ZeroMemory( &DummyPowerTarget, sizeof( POWER_TARGET ) ) ;
}

/* 防御ターゲットをリストに登録 */
static	void	GM_AddDefTarget( def )
TARGET	*def ;
{
    TARGET	*list ;

//    ASSERT( TargetWork != NULL ) ;
	if ( TargetWork == NULL ) return ;
    /* 二重リンクをチェック */
    list = TargetWork->DefenseList->next ;
    while( list != NULL ) {
		if ( list == def ) {
			printf( "target %x is already linked!!\n", def ) ;
#ifdef DEBUG_MODE
			ASSERT( 0 ) ;
#else
			return ;
#endif
		}
		list = list->next ;
    }
    list = TargetWork->DefListLast ;
    list->next = ( void * )def ;
    def->next = NULL ;
    TargetWork->DefListLast = def ;    
    TargetWork->N_DefenseTargets ++ ;
}

/* 攻撃ターゲットをリストに登録 */
static	void	GM_AddOffTarget( off )
TARGET	*off ;
{
    TARGET	*list ;

//    ASSERT( TargetWork != NULL ) ;
	if ( TargetWork == NULL ) return ;
    /* 二重リンクをチェック */
    list = TargetWork->OffenseList->next ;
    while( list != NULL ) {
		if ( list == off ) {
			printf( "target %x is already linked!!\n", off ) ;
#ifdef DEBUG_MODE
			ASSERT( 0 ) ;
#else
			return ;
#endif
		}
		list = list->next ;
    }
#if 0
    list = TargetWork->OffListLast ;
    list->next = ( void * )off ;
    off->next = NULL ;
    TargetWork->OffListLast = off ;    
#else
    list = TargetWork->OffenseList ;
    off->next = list->next ;
    list->next = ( void * )off ;
#endif
    TargetWork->N_OffenseTargets ++ ;
}

/* 防御ターゲットをリストから削除 */
void	GM_FreeTarget( def )
TARGET	*def ;
{
    TARGET	*list, *cur ;
    TARGET_PARTS	*parts ;

	if ( TargetWork == NULL ) return ;
#ifdef DEBUG_MODE
	if ( def->class & TARGET_OFFENSE ) {
		printf( "warning : offense target doesn't need free.\n" ) ;
	}
#endif
    ASSERT( TargetWork != NULL ) ;
    list = TargetWork->DefenseList ;
    while ( ( cur = ( TARGET * )list->next ) != NULL ) {
		if ( ( u_int )cur == ( u_int )def ) {
			if ( def == TargetWork->DefListLast ) {
				TargetWork->DefListLast = list ;
			}
			parts = def->parts ;
			while( parts != NULL ) {
				GV_DelayedFree( parts ) ;
				parts = parts->next ;
			}
			def->parts = NULL ;
			list->next = def->next ;
			TargetWork->N_DefenseTargets -- ;
			break ;
		}
		list = cur ;
    }
#if 0
    list = TargetWork->DefenseList ;
    printf( "--- defense list %d ---\n", TargetWork->N_DefenseTargets ) ;
    cur = list->next ;
    while( cur != NULL ) {
		printf( "%x\n", cur ) ;
		cur = cur->next ;
    }
#endif
}

/*-----------------------------------------------------------------------------*/

/* ターゲット構造体を初期化 */
void	GM_SetTarget( targ, class, map, side, size, offset ) 
TARGET		*targ ;
u_int		class ;
u_int		map ;
u_int		side ;
FVECTOR		*size ;
FVECTOR		*offset ;
{

    if ( map == 0 ) map = GM_CurrentMap ;
    targ->world = DG_UnitMatrix ;
	targ->center = DG_ZeroVector ;
    targ->class = class ;
    targ->map = map ;
    targ->side = side ;
    targ->size = *size ;
    targ->offset = *offset ;

#ifdef KP_WINDOWS
#ifdef DEBUG_MODE
    if(  (((*(DWORD *)&(targ->offset.vw)) & 0x7f800000) == 0x7f800000)	// Nan Test
       && (targ->offset.vw != 1.0f) )	// とりあえず調べておく
    {
       ASSERT(0) ;
    }
#else
    if( ((*(DWORD *)&(targ->offset.vw)) & 0x7f800000) == 0x7f800000 )	// Nan Test
    {
       targ->offset.vw = 1.0f ;
    }
#endif
#endif

    targ->offset.vw = 1.0f ; // BP_MATH - fix unitized value

    targ->damaged = 0 ;
    targ->weapon_type = 0 ;
    targ->power = NULL ;
    targ->capture = NULL ;
    targ->push = NULL ;
    targ->stick = NULL ;
    targ->callback = NULL ;
    targ->parts = NULL ;
    /* size.vwに中心からターゲット壁までの最大距離を入れておく */
    targ->size.vw = DG_SQRT( targ->size.vx * targ->size.vx +
							targ->size.vy * targ->size.vy +
							targ->size.vz * targ->size.vz ) ;
}	

void	GM_SetTargetName( targ, name )
TARGET		*targ ;
int		name ;
{
    targ->class |= TARGET_HAS_NAME ;
    targ->name = name ;
}

/* ターゲット登録 */
void	GM_PutTarget( targ )
TARGET		*targ ;
{
#ifdef DEBUG_MODE
	if ( TargetWork->callbackflag == 1 ) {
		printf( "warning : GM_PutTarget is executed in a callback proc.\n" ) ;
	}
#endif
    if ( targ->class & TARGET_OFFENSE ) {
		GM_AddOffTarget( targ ) ;
    } else {
		GM_AddDefTarget( targ ) ;
    }
}

/* ターゲットのサイズを変更 */
void	GM_SetTargetSize( targ, size )
TARGET	*targ ;
FVECTOR	*size ;
{
    targ->size = *size ;
    targ->size.vw = DG_SQRT( size->vx * size->vx +
							size->vy * size->vy +
							size->vz * size->vz ) ;
}

/* ターゲットを移動 */
void	GM_MoveTargetMap( targ, mov, map )
TARGET		*targ ;
FVECTOR		*mov ;
int		map ;
{
    targ->center.vx = mov->vx ;
    targ->center.vy = mov->vy ;
    targ->center.vz = mov->vz ;
    targ->map = map ;
}

void	GM_MoveTarget2Map( targ, mov, map )
TARGET		*targ ;
FMATRIX		*mov ;
int			map ;
{
    targ->world = *mov ;
    targ->world.m[ 3 ][ 0 ] = 0.0F ;
    targ->world.m[ 3 ][ 1 ] = 0.0F ;
    targ->world.m[ 3 ][ 2 ] = 0.0F ;
    targ->center.vx = mov->m[ 3 ][ 0 ] ;
    targ->center.vy = mov->m[ 3 ][ 1 ] ;
    targ->center.vz = mov->m[ 3 ][ 2 ] ;
    targ->map = map ;
}

void	GM_MoveTarget3Map( targ, mov, map )
TARGET		*targ ;
FMATRIX		*mov ;
int			map ;
{
    targ->center.vx = mov->m[ 3 ][ 0 ] ;
    targ->center.vy = mov->m[ 3 ][ 1 ] ;
    targ->center.vz = mov->m[ 3 ][ 2 ] ;
    targ->world = DG_UnitMatrix ;
    targ->map = map ;
}

/* ターゲットを移動（オンライン攻撃用） */
void	GM_MoveOnlineTargetMap( targ, from, to, map )
TARGET		*targ ;
FVECTOR		*from ;
FVECTOR		*to ;
int		map ;
{
    targ->center = *from ;
    targ->offset = *to ;	
    targ->map = map ;
}

/* 子ターゲットを設定 */
void	GM_TargetConnectChild( targ, child, n, level )
TARGET		*targ ;
TARGET		*child ;
int		n, level ;
{	
    TARGET_PARTS	*parts, *list, *prev ;

    targ->class |= TARGET_CHILD ;
    
    parts = GV_Malloc( sizeof( TARGET_PARTS ) ) ;
    if ( parts == NULL ) {
		printf( "cannot alloc memory for target_parts\n" ) ;
		ASSERT( 0 ) ;
    }
    parts->flag = 0 ;
    parts->n_parts = n ;
    parts->parts = ( void * )child ;
    parts->level = level ;

    prev = NULL ;
    list = targ->parts ;
    while( list != NULL ) {
		if ( list->level >= level ) break ;
		prev = list ;
		list = list->next ;
    }
    if ( prev == NULL ) {
		parts->next = targ->parts ;
		targ->parts = parts ;
    } else {
		parts->next = prev->next ;
		prev->next = parts ;
    }
}

/* 子ターゲット切り離し */
void	GM_TargetDisconnectChild( targ, child )
TARGET	*targ, *child ;
{
    TARGET_PARTS	*list, *prev ;

    prev = NULL ;
    list = targ->parts ;
    while( list != NULL ) {
		if ( list->parts == child ) break ;
		prev = list ;
		list = list->next ;
    }
    if ( list == NULL ) printf( "disconnect child : not found\n" ) ;
    if ( prev == NULL ) {
		targ->parts = list->next ;
    } else {
		prev->next = list->next ;
    }
	GV_DelayedFree( list ) ;
}

/* 武器タイプを設定 */
#if 0 /* yano 2002.03.11 */
void	GM_SetTargetWeaponType( targ, type )
TARGET		*targ ;
long64		type ;
#else
void	GM_SetTargetWeaponType( TARGET *targ, long64 type )
#endif
{
    targ->weapon_type = type ;
}

/* コールバック設定 */
void	GM_SetTargetCallBack( targ, func, ptr )
TARGET		*targ ;
TARGET_CALLBACK	func ;
void		*ptr ;
{
    targ->callback = func ;
    targ->work = ptr ;
}

/*-----------------------------------------------------------------------------*/

/* 攻撃用ターゲットの設定 */

/* ダメージ系 */
void	GM_SetPowerTarget( targ, power, p_type, vital, faint, damage, force )
TARGET			*targ ;
POWER_TARGET		*power ;
u_int			p_type ;
u_short			vital ;
u_short			faint ;
u_short			damage ;
FVECTOR			*force ;
{
    targ->class |= TARGET_POWER ;
    targ->power = power ;
    power->type = p_type ;
    power->vital = vital ;
    power->faint = faint ;
    power->damage = damage ;
    power->force = *force ;
}

/* つかみ系 */
void	GM_SetCaptureTarget( targ, capture, ctrl, body )
TARGET		*targ ;
CAPTURE_TARGET	*capture ;
CONTROL		*ctrl ;
OBJECT		*body ;
{
    targ->class |= TARGET_CAPTURE ;
    targ->capture = capture ;
    capture->ctrl = ctrl ;
    capture->body = body ;
    capture->capture = NULL ;
    capture->flag = 0 ;
}

/* プッシュ */
void	GM_SetPushTarget(
		TARGET		*targ ,
		PUSH_TARGET	*push ,
		float		depth )
{
    targ->class |= TARGET_PUSH ;
    targ->push = push ;
    push->depth = depth ;
    push->force = DG_ZeroVector ;
}

/* くっつき系 */
void	GM_SetStickTarget( targ, stick, world )
TARGET		*targ ;
STICK_TARGET	*stick ;
FMATRIX		*world ;
{
    targ->class |= TARGET_STICK ;
    targ->stick = stick ;
    stick->world = world ;
    stick->free = 0 ;
}

/*-----------------------------------------------------------------------------*/

/* 接触時、パラメータ更新関数群 */

/* 耐久力更新 */
static	void	UpdatePowerParam( power, op )
POWER_TARGET	*power, *op ;
{
    switch ( POWER_CLASS & power->type ) {
    case POWER_ONCE :
		power->vital = 0 ;
		break ;
    case POWER_DECREASE :
		power->vital -= op->damage ;
		break ;
    case POWER_THRESHOLD :
		if ( power->vital <= op->damage ) {
			power->vital = 0 ;
		}
		break ;
    case POWER_CONST :
    case POWER_EXPLODE :
    default :
		;
    }
}

/* 力積計算 */
void		GM_MakeTargetForce( off, def )
TARGET		*off, *def ;
{
    float		dir ;    
    FVECTOR		diff, d1, o1, offset ;
    POWER_TARGET	*op, *dp ;

    op = off->power ; 
    dp = def->power ;
	if ( op == NULL || dp == NULL ) return ;
    if ( op->type & POWER_EXPLODE ) {
		/* 爆発型 */
		if ( def->class & TARGET_ROTATE ) {
			_sceVu0ApplyMatrix( &offset, &def->world, &def->offset ) ;
		} else {
			DG_COPY_VEC( &offset, &DG_ZeroVector ) ;
		}
		_sceVu0AddVector( &d1, &( def->center ), &offset ) ;
		if ( off->class & TARGET_ROTATE ) {
			_sceVu0ApplyMatrix( &offset, &off->world, &off->offset ) ;
		} else {
			DG_COPY_VEC( &offset, &DG_ZeroVector ) ;
		}
		_sceVu0AddVector( &o1, &( off->center ), &offset ) ;
		_sceVu0SubVector( &diff, &d1, &o1 ) ;
		dir = atan2f( diff.vx, diff.vz ) ;
		dp->force.vx += op->force.vx * sinf( dir ) ;
		dp->force.vz += op->force.vx * cosf( dir ) ;
//printf( "dir %d : %f %f\n", ( int )( dir * 2048.0F / 3.14F ), dp->force.vx, dp->force.vz ) ;
    } else {
		/* 指向性 */
		_sceVu0AddVector( &( dp->force ), &( dp->force ), &( op->force ) ) ;
    }
}

/* ダメージ計算 */
int	GM_DamageTarget( off, def )
TARGET		*off, *def ;
{
    POWER_TARGET	*op, *dp ;

    op = off->power ; 
    if ( op == NULL ) return def->power->vital ;
    dp = def->power ;
    dp->damage += op->damage ;
    dp->faint += op->faint ;
    UpdatePowerParam( op, op ) ;
    UpdatePowerParam( dp, op ) ;
    GM_MakeTargetForce( off, def ) ;
    return dp->vital ;
}

/* １次元プッシュ量計算 */
static	float	Push( float center1, float size1, float center2, float size2 )
{
    size1 += size2 ;
    size2 = center2 - center1 ;
    if ( size2 >= 0.0F ) {
		size1 -= size2 ;
		if ( size1 < 0.0F ) size1 = 0.0F ;
		return size1 ;
    } else {
		size2 += size1 ;
		if ( size2 < 0.0F ) size2 = 0.0F ;
		return - size2 ;
    }
}

/* プッシュ計算 */
int	GM_PushTarget( off, def )
TARGET		*off, *def ;
{
    float	x, z, x2, z2, d ;
    float	y ;

    if ( off->push == NULL ) return 0 ;

    d = off->push->depth ;
    y = Push( def->center.vy + def->offset.vy, def->size.vy,
			 off->center.vy + off->offset.vy, off->size.vy ) ;
    def->push->force.vy = - y * d ;
    off->push->force.vy = y * ( 1.0F - d ) ;

    x = Push( def->center.vx + def->offset.vx, def->size.vx,
			 off->center.vx + off->offset.vx, off->size.vx ) ;
    if ( x == 0.0F ) return 0 ;
    z = Push( def->center.vz + def->offset.vz, def->size.vz,
			 off->center.vz + off->offset.vz, off->size.vz ) ;
    if ( z == 0.0F ) return 0 ;

    x2 = ( x >= 0.0F ) ? x : -x ;
    z2 = ( z >= 0.0F ) ? z : -z ;
#if 0
    if ( x2 <= z2 ) {
		def->push->force.vx -= x * d ;
		off->push->force.vx += x * ( 1.0F - d ) ;
    } else {
		def->push->force.vz -= z * d ;
		off->push->force.vz += z * ( 1.0F - d ) ;
    }
#else
    if ( x2 <= z2 ) {
		def->push->force.vx = - x * d ;
		off->push->force.vx = x * ( 1.0F - d ) ;
    } else {
		def->push->force.vz = - z * d ;
		off->push->force.vz = z * ( 1.0F - d ) ;
    }
#endif
    return 1 ;
}

/* つかみ関係 */
void	GM_CaptureTarget( off, def ) 
TARGET		*off, *def ;
{
    def->capture->capture = off->capture ;
    off->capture->capture = def->capture ;
}

/* ダメージクリア */
void	GM_ClearTargetDamage( def )
TARGET		*def ;
{
    def->damaged &= ~TARGET_POWER ;
	def->weapon_type = 0 ;
    if ( def->power != NULL ) {
		POWER_TARGET	*p ;
	
		p = def->power ;
		p->force = DG_ZeroVector ;
		p->damage = 0 ;
    }
}

/* プッシュクリア */
void	GM_ClearTargetPush( def )
TARGET		*def ;
{
    if ( def->push != NULL ) {
		def->damaged &= ~TARGET_PUSH ;
		def->push->force = DG_ZeroVector ;
    }	
}

/* スキップ設定 */
void	GM_TargetSetSkip( def )
TARGET	*def ;
{
    def->class |= TARGET_SKIP ;
}

void	GM_TargetResetSkip( def )
TARGET	*def ;
{
    def->class &= ~TARGET_SKIP ;
}

/* ターゲットの中心を求める */
void	GM_TargetGetCenter( FVECTOR *center, TARGET *t )
{
	FVECTOR		offset ;

	if ( t->class & TARGET_ROTATE ) {
		_sceVu0ApplyMatrix( &offset, &t->world, &t->offset ) ;
	} else {
		DG_COPY_VEC( &offset, &t->offset ) ;
	}
	_sceVu0AddVector( center, &t->center, &offset ) ;
}

/* 当たらなかった事にする。
   攻撃ターゲットコールバック内で実行 */
void	GM_TargetHitCancel( TARGET *off, TARGET *def )
{
	if ( def->weapon_type == off->weapon_type ) {
		GM_ClearTargetDamage( def ) ;
	}
	def->weapon_type &= ~off->weapon_type ;
}

/* 中心間チェック */
int		GM_TargetCheckCenter2Center( TARGET *off, TARGET *def, 
									 int chk_type, int seg_flag, int flr_flag )
{
	FVECTOR			oc, dc ;

	GM_TargetGetCenter( &oc, off ) ;
	GM_TargetGetCenter( &dc, def ) ;
	if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &oc, &dc, 
							    chk_type, seg_flag, flr_flag ) ) {
		return 1 ;
	}
	return 0 ; 
}


/*-----------------------------------------------------------------------------*/

/* 攻撃ターゲットリストをクリア */
static	inline	void	ClearOffenseTargets( Work *work )
{
    work->N_OffenseTargets = 0 ;
    work->OffenseList->next = NULL ;
    work->OffListLast = work->OffenseList ;
}

/* 衝突判定（１次元）*/
static	inline	int	Conflict( float center1, float size1, float center2, float size2 )
{
    size1 += size2 ;
    return center1 - size1 <= center2 && center2 - size1 <= center1 ;
}

/* 検索直線対角線とする直方体ターゲットを作成 */
static	void		SetFromToTarget( from, to, targ )
FVECTOR			*from, *to ;
TARGET			*targ ;
{
    float	f, t, s ;

    f = from->vx ; t = to->vx ; s = ( f - t ) / 2.0F ;
    targ->center.vx = ( f + t ) / 2.0F ; 
    targ->size.vx = ( s >= 0.0F ) ? s : -s ;
    f = from->vy ; t = to->vy ; s = ( f - t ) / 2.0F ;
    targ->center.vy = ( f + t ) / 2.0F ; 
    targ->size.vy = ( s >= 0.0F ) ? s : -s ;
    f = from->vz ; t = to->vz ; s = ( f - t ) / 2.0F ;
    targ->center.vz = ( f + t ) / 2.0F ; 
    targ->size.vz = ( s >= 0.0F ) ? s : -s ;
    targ->offset = DG_ZeroVector ;
}

/* 検索直線とターゲットの交差判定 */
static	int	OnlineTarget( from, to, def, hit )
FVECTOR		*from, *to ;
TARGET		*def ;
FVECTOR		*hit ;
{
    float	ccx, ccy, ccz ;
    float	cx, cy, cz, sx, sy, sz, df, f0 ;
    float	c1, c2 ;

    cx = ccx = def->center.vx + def->offset.vx ;
    cy = ccy = def->center.vy + def->offset.vy ;
    cz = ccz = def->center.vz + def->offset.vz ;
    sx = def->size.vx ;
    sy = def->size.vy ;
    sz = def->size.vz ;
    
    *NORMAL = DG_ZeroVector ;

    /* Ｘ切断を試みる */
    f0 = from->vx ;
    if ( ( df = to->vx - f0 ) == 0.0F ) goto cutx_failed ;
    if ( cx < f0 ) {
		cx += sx ;
		if ( cx > f0 ) cx = f0 ;
    } else {
		cx -= sx ;
		if ( cx < f0 ) cx = f0 ;
    }
    c1 = from->vy + ( to->vy - from->vy ) * ( cx - f0 ) / df ;
    if ( c1 < cy - sy || c1 > cy + sy ) {
		goto cutx_failed ;
    }
    c2 = from->vz + ( to->vz - from->vz ) * ( cx - f0 ) / df ;
    if ( c2 < cz - sz || c2 > cz + sz ) {
		goto cutx_failed ;
    }
    hit->vx = cx ;
    hit->vy = c1 ;
    hit->vz = c2 ;
    if ( cx == ccx - sx ) NORMAL->vx = 1.0F ;
    else if ( cx == ccx + sx ) NORMAL->vx = -1.0F ;
    else if ( c1 == ccy + sy ) NORMAL->vy = 1.0F ;
    else if ( c1 == ccy - sy ) NORMAL->vy = -1.0F ;
    else if ( c2 == ccy + sz ) NORMAL->vz = 1.0F ;
    else if ( c2 == ccy - sz ) NORMAL->vz = -1.0F ;
    return 1 ;
cutx_failed :
    /* Ｙ切断を試みる */
    cx = ccx ;
    cy = ccy ;
    cz = ccz ;
    f0 = from->vy ;
    if ( ( df = to->vy - f0 ) == 0.0F ) goto cuty_failed ;
    if ( cy < f0 ) {
		cy += sy ;
		if ( cy > f0 ) cy = f0 ;
    } else {
		cy -= sy ;
		if ( cy < f0 ) cy = f0 ;
    }
    c1 = from->vz + ( to->vz - from->vz ) * ( cy - f0 ) / df ;
    if ( c1 < cz - sz || c1 > cz + sz ) {
		goto cuty_failed ;
    }
    c2 = from->vx + ( to->vx - from->vx ) * ( cy - f0 ) / df ;
    if ( c2 < cx - sx || c2 > cx + sx ) {
		goto cuty_failed ;
    }
    hit->vy = cy ;
    hit->vz = c1 ;
    hit->vx = c2 ;
    if ( cy == ccy - sy ) NORMAL->vy = 1.0F ;
    else if ( cx == ccx + sy ) NORMAL->vy = -1.0F ;
    else if ( c1 == ccy + sz ) NORMAL->vz = 1.0F ;
    else if ( c1 == ccy - sz ) NORMAL->vz = -1.0F ;
    else if ( c2 == ccy + sx ) NORMAL->vx = 1.0F ;
    else if ( c2 == ccy - sx ) NORMAL->vx = -1.0F ;
    return 1 ;
cuty_failed :
    /* Ｚ切断を試みる */
    cx = ccx ;
    cy = ccy ;
    cz = ccz ;
    f0 = from->vz ;
    if ( ( df = to->vz - f0 ) == 0.0F ) goto cutz_failed ;
    if ( cz < f0 ) {
		cz += sz ;
		if ( cz > f0 ) cz = f0 ;
    } else {
		cz -= sz ;
		if ( cz < f0 ) cz = f0 ;
    }
    c1 = from->vx + ( to->vx - from->vx ) * ( cz - f0 ) / df ;
    if ( c1 < cx - sx || c1 > cx + sx ) {
		goto cutz_failed ;
    }
    c2 = from->vy + ( to->vy - from->vy ) * ( cz - f0 ) / df ;
    if ( c2 < cy - sy || c2 > cy + sy ) {
		goto cutz_failed ;
    }
    hit->vz = cz ;
    hit->vx = c1 ;
    hit->vy = c2 ;
    if ( cz == ccz - sz ) NORMAL->vz = 1.0F ;
    else if ( cx == ccz + sz ) NORMAL->vz = -1.0F ;
    else if ( c1 == ccx + sx ) NORMAL->vx = 1.0F ;
    else if ( c1 == ccx - sx ) NORMAL->vx = -1.0F ;
    else if ( c2 == ccy + sy ) NORMAL->vy = 1.0F ;
    else if ( c2 == ccy - sy ) NORMAL->vy = -1.0F ;
    return 1 ;
cutz_failed :
    return 0 ;
}

#if 0
/* 検索直線とターゲットの交差判定２
   ＦＲＯＭは必ずターゲット内部。
   外直方体面との接点を交差点とする。
   ＦＲＯＭ・ＴＯ共に内部の場合は０を返す。 */
static	int	OnlineTarget2( from, to, def, hit )
FVECTOR		*from, *to ;
TARGET		*def ;
FVECTOR		*hit ;
{
    float	ccx, ccy, ccz ;
    float	cx, cy, cz, sx, sy, sz, df, f0, t0, dt ;
    int		c ;

    c = 0 ;
    cx = ccx = def->center.vx + def->offset.vx ;
    cy = ccy = def->center.vy + def->offset.vy ;
    cz = ccz = def->center.vz + def->offset.vz ;
    sx = def->size.vx ;
    sy = def->size.vy ;
    sz = def->size.vz ;
    *hit = *to ;

    /* Ｘ切断を試みる */
    f0 = from->vx ; t0 = to->vx ;
    if ( ( df = t0 - f0 ) == 0.0F ) goto cutx_failed ;
    if ( t0 >= ( cx - sx ) && t0 <= ( cx + sx ) ) goto cutx_failed ;
    if ( t0 < ( cx - sx ) ) dt = cx - sx ;
    else		    dt = cx + sx ;
    hit->vx = dt ;
    hit->vy = from->vy + ( to->vy - from->vy ) * ( dt - f0 ) / df ;
    hit->vz = from->vz + ( to->vz - from->vz ) * ( dt - f0 ) / df ;
    c ++ ;
cutx_failed :
    /* Ｙ切断を試みる */
    f0 = from->vy ; t0 = hit->vy ;
    if ( ( df = t0 - f0 ) == 0.0F ) goto cuty_failed ;
    if ( t0 >= ( cy - sy ) && t0 <= ( cy + sy ) ) goto cuty_failed ;
    if ( t0 < ( cy - sy ) ) dt = cy - sy ;
    else		    dt = cy + sy ;
    hit->vx = from->vx + ( hit->vx - from->vx ) * ( dt - f0 ) / df ;
    hit->vy = dt ;
    hit->vz = from->vz + ( hit->vz - from->vz ) * ( dt - f0 ) / df ;
    c ++ ;
cuty_failed :
    /* Ｚ切断を試みる */
    f0 = from->vz ; t0 = hit->vz ;
    if ( ( df = t0 - f0 ) == 0.0F ) goto cutz_failed ;
    if ( t0 >= ( cz - sz ) && t0 <= ( cz + sz ) ) goto cutz_failed ;
    if ( t0 < ( cz - sz ) ) dt = cz - sz ;
    else		    dt = cz + sz ;
    hit->vx = from->vx + ( hit->vx - from->vx ) * ( dt - f0 ) / df ;
    hit->vy = from->vy + ( hit->vy - from->vy ) * ( dt - f0 ) / df ;
    hit->vz = dt ;
    c ++ ;
cutz_failed :
	//if ( c == 0 ) printf( "inside\n" ) ;
    return ( c > 0 ) ? 1 : 0 ;
}
#endif

/*-----------------------------------------------------------------------------*/

/* クラスチェック */
static	inline	int	CheckClass( oc, dc )
int		oc, dc ;
{
    if ( dc & TARGET_SKIP ) {
		//	printf( "skip\n" ) ;
		return 0 ;
    }
    oc &= TARGET_CHECK_CLASS ;
    dc &= TARGET_CHECK_CLASS ;
//printf( "%x %x\n", oc, dc ) ;
    return ( oc & dc ) ;
}

/* 箱判定 */
static	int	BoxCheckTarget( off, def ) 
TARGET		*off, *def ;
{
    return ( Conflict( off->center.vx + off->offset.vx, off->size.vx, 
					  def->center.vx + def->offset.vx, def->size.vx ) &&
			Conflict( off->center.vy + off->offset.vy, off->size.vy, 
					 def->center.vy + def->offset.vy, def->size.vy ) &&
			Conflict( off->center.vz + off->offset.vz, off->size.vz, 
					 def->center.vz + def->offset.vz, def->size.vz ) ) ;
}

#if 1
/* 回転あり箱判定 */
static	int	BoxCheckTargetRotate( off, def, from )
TARGET		*off, *def ;
FVECTOR		*from ;
{
    static int	Lines[] = { 
		0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7, 
		7, 6, 6, 4, 0, 4, 1, 5, 3, 7, 2, 6
    } ;
    FVECTOR	to, size, v, v2, s ;
    FVECTOR	b1, b2 ;
    float	len ;
    FMATRIX    	*mat, inv ;
    FVECTOR	offset, hit ;
    int		i, dr ;

    dr = def->class & TARGET_ROTATE ;
    /* まず中心－中心の距離を求める */
    to = def->center ;
    if ( dr ) {
		mat = &( def->world ) ;
		_sceVu0ApplyMatrix( &offset, mat, &( def->offset ) ) ;
    } else {
		DG_COPY_VEC( &offset, &def->offset ) ;
    }
    _sceVu0AddVector( &to, &to, &offset ) ;
    len = GV_VecLen3F2( from, &to ) ;
    /* 中心間距離が、size.vw の和よりも大きい時ははずれ */
    if ( len > off->size.vw + def->size.vw ) {
		return 0 ;
    }

    /* 攻撃側ターゲットの頂点が
       防御側ターゲットに侵入しているかチェック */
    DG_COPY_VEC( &b2, &def->size ) ;
    _sceVu0SubVector( &b1, &DG_ZeroVector, &b2 ) ;
    _sceVu0TransposeMatrix( &inv, &def->world ) ;
    DG_COPY_VEC( &size, &off->size ) ;
    size.vw = 1.0F ;
    DG_SetPos( &off->world ) ;
    for ( i = 0; i < 8; i ++ ) {
		s.vx = ( i & 1 ) ? size.vx : - size.vx ;
		s.vy = ( i & 2 ) ? size.vy : - size.vy ;	
		s.vz = ( i & 4 ) ? size.vz : - size.vz ;
		DG_PutVector( &s, &s, 1 ) ;
		_sceVu0AddVector( &v, from, &s ) ;
		_sceVu0SubVector( &v2, &v, &to ) ;
		v.vw = v2.vw = 1.0F ;
		_sceVu0ApplyMatrix( &v2, &inv, &v2 ) ;
		if ( vu0_CheckBoundingBox( &v2, &b1, &b2 ) ) {
			//if ( GV_PadData->status & PAD_L1 ) printf( "offverts inside\n" ) ;
			return 1 ;
		}
		DG_COPY_VEC( &OV[ i ], &v2 ) ;
    }
    /* 防御側ターゲットの頂点が
       攻撃側ターゲットに侵入しているかチェック */
    DG_COPY_VEC( &b2, &off->size ) ;
    _sceVu0ScaleVector( &b1, &b2, -1.0F ) ;
    _sceVu0TransposeMatrix( &inv, &off->world ) ;
    DG_COPY_VEC( &size, &def->size ) ;
    size.vw = 1.0F ;
    DG_SetPos( &def->world ) ;
    for ( i = 0; i < 8; i ++ ) {
		s.vx = ( i & 1 ) ? size.vx : - size.vx ;
		s.vy = ( i & 2 ) ? size.vy : - size.vy ;	
		s.vz = ( i & 4 ) ? size.vz : - size.vz ;
		DG_PutVector( &s, &s, 1 ) ;
		_sceVu0AddVector( &v, &to, &s ) ;
		_sceVu0SubVector( &v, &v, from ) ;
		v.vw = 1.0F ;
		_sceVu0ApplyMatrix( &v, &inv, &v ) ;
		if ( vu0_CheckBoundingBox( &v, &b1, &b2 ) ) {
			//if ( GV_PadData->status & PAD_L1 ) printf( "defverts inside\n" ) ;
			return 1 ;
		}
		DG_COPY_VEC( &DV[ i ], &v ) ;
    }
    
    /* なかなかしぶとい */

    /* 攻撃側ターゲットを構成する１２本の直線と
       防御側ターゲットの交差判定をする */
    DG_COPY_VEC( &TMP_TARG->center, &DG_ZeroVector ) ;
    DG_COPY_VEC( &TMP_TARG->offset, &DG_ZeroVector ) ;
    DG_COPY_VEC( &TMP_TARG->size, &def->size ) ;
    for ( i = 0; i < 24; i += 2 ) {
		SetFromToTarget( &OV[ Lines[ i ] ], &OV[ Lines[ i + 1 ] ], TMP_TARG2 ) ;
		if ( !BoxCheckTarget( TMP_TARG2, TMP_TARG ) ) continue ;
		if ( OnlineTarget( &OV[ Lines[ i ] ], &OV[ Lines[ i + 1 ] ], TMP_TARG, &hit ) ) {
			//if ( GV_PadData->status & PAD_L1 ) printf( "offline touch\n" ) ;
			return 1 ;
		}
    }
    /* 防御側ターゲットを構成する１２本の直線と
       攻撃側ターゲットの交差判定をする */
    DG_COPY_VEC( &TMP_TARG->size, &off->size ) ;
    for ( i = 0; i < 24; i += 2 ) {
		SetFromToTarget( &DV[ Lines[ i ] ], &DV[ Lines[ i + 1 ] ], TMP_TARG2 ) ;
		if ( !BoxCheckTarget( TMP_TARG2, TMP_TARG ) ) continue ;
		if ( OnlineTarget( &DV[ Lines[ i ] ], &DV[ Lines[ i + 1 ] ], TMP_TARG, &hit ) ) {
			//if ( GV_PadData->status & PAD_L1 ) printf( "defline touch\n" ) ;
			return 1 ;
		}
    }
    /* こんなにチェックしたのに外れですか */
    return 0 ;
}
#endif

/* 回転ありオンライン判定 */
static	int	OnlineTargetRotate( off, def )
TARGET		*off, *def ;
{
    TARGET	def2 ;
    FMATRIX	*mat, inv ;
    FVECTOR	offset, mov, from, to ;
    
    mat = &( def->world ) ;
    _sceVu0ApplyMatrix( &offset, &( def->world ), &( def->offset ) ) ;
    /* 防御側ターゲットのセンター中心の座標に変換して
       計算 */
    _sceVu0AddVector( &mov, &( def->center ), &offset ) ;
    _sceVu0SubVector( &from, FROM, &mov ) ;
    _sceVu0SubVector( &to, TO, &mov ) ;
    _sceVu0UnitMatrix( &inv ) ;
    _sceVu0TransposeMatrix( &inv, mat ) ;
	to.vw = from.vw = 1.0f ;
    _sceVu0ApplyMatrix( &from, &inv, &from ) ;
    _sceVu0ApplyMatrix( &to, &inv, &to ) ;
    def2.center = DG_ZeroVector ;
    def2.offset = DG_ZeroVector ;
    _sceVu0CopyVector( &( def2.size ), &( def->size ) ) ;
    SetFromToTarget( &from, &to, MIN_TARG2 ) ;
    if ( !BoxCheckTarget( MIN_TARG2, &def2 ) ) return 0 ;
    if ( !OnlineTarget( &from, &to, &def2, MIN_HIT2 ) ) return 0 ;
    if ( off->class & TARGET_GET_NORMAL ) {
		*NORMAL = DG_ZeroVector ;
		if ( DG_FABS( MIN_HIT2->vx + def2.size.vx ) < 1.0F ) NORMAL->vx = 1.0F ;
		else if ( DG_FABS( MIN_HIT2->vx - def2.size.vx ) < 1.0F ) NORMAL->vx = -1.0F ;
		else if ( DG_FABS( MIN_HIT2->vy + def2.size.vy ) < 1.0F ) NORMAL->vy = 1.0F ;
		else if ( DG_FABS( MIN_HIT2->vy - def2.size.vy ) < 1.0F ) NORMAL->vy = -1.0F ;
		else if ( DG_FABS( MIN_HIT2->vz + def2.size.vz ) < 1.0F ) NORMAL->vz = 1.0F ;
		else if ( DG_FABS( MIN_HIT2->vz - def2.size.vz ) < 1.0F ) NORMAL->vz = -1.0F ;
		_sceVu0ApplyMatrix( NORMAL, mat, NORMAL ) ;    
    }
    _sceVu0ApplyMatrix( MIN_HIT2, mat, MIN_HIT2 ) ;
    _sceVu0AddVector( MIN_HIT, MIN_HIT2, &mov ) ;
    return 1 ;
}

/* オンライン判定 */
static	int	OnlineCheckTarget( off, def ) 
TARGET		*off, *def ;
{
    *TMP_HIT = *MIN_HIT ;
    if ( def->class & TARGET_ROTATE ) {
		if ( !OnlineTargetRotate( off, def ) ) return 0 ;
    } else {
		if ( !BoxCheckTarget( MIN_TARG, def ) ) return 0 ;	
		if ( !OnlineTarget( FROM, TO, def, MIN_HIT ) ) return 0 ;
		if ( off->class & TARGET_GET_NORMAL ) {
			FVECTOR	b1, b2 ;

			*NORMAL = DG_ZeroVector ;
			{
				FVECTOR		center ;
				GM_TargetGetCenter( &center, def )	;
				_sceVu0SubVector( &b1, &center, &def->size ) ;
				_sceVu0AddVector( &b2, &center, &def->size ) ;
			}
			if ( DG_FABS( MIN_HIT->vx - b1.vx ) < 1.0F ) NORMAL->vx = 1.0F ;
			else if ( DG_FABS( MIN_HIT->vx - b2.vx ) < 1.0F ) NORMAL->vx = -1.0F ;
			else if ( DG_FABS( MIN_HIT->vy - b1.vy ) < 1.0F ) NORMAL->vy = 1.0F ;
			else if ( DG_FABS( MIN_HIT->vy - b2.vy ) < 1.0F ) NORMAL->vy = -1.0F ;
			else if ( DG_FABS( MIN_HIT->vz - b1.vz ) < 1.0F ) NORMAL->vz = 1.0F ;
			else if ( DG_FABS( MIN_HIT->vz - b2.vz ) < 1.0F ) NORMAL->vz = -1.0F ;
		}
    }
    if ( ( ( off->class & TARGET_CHILD ) &&
		   ( def->class & TARGET_CHILD ) &&
		   !( off->class & TARGET_CHILD_SKIP ) ) ||
		 ( def->class & TARGET_CHILD_ALWAYS ) ) {
		*MIN_HIT = *TMP_HIT ;
		return 1 ;				/* 子ターゲットありのときはここでreturn */
	}
    if ( !( def->class & TARGET_THROUGH ) ) {
		if ( off->class & TARGET_ONLINE_MIN ) {
			/* 検索直線ターゲットを更新 */
			SetFromToTarget( FROM, MIN_HIT, MIN_TARG ) ;
			*TO = *MIN_HIT ;
			*MIN_NORMAL = *NORMAL ;
			MIN_DEF_TARG = def ;
			return 0 ;			/* 0 を返す */
		} 
		def->hit = *MIN_HIT ;
		off->hit = *MIN_HIT ;
		off->size = *NORMAL ;
		return 1 ;
    } else {
		/* 防御側がスルーターゲット */
		def->hit = *MIN_HIT ;
		*MIN_HIT = *TMP_HIT ;	/* MIN_HITを更新してはならない */
		return 1 ;
    }
}

/* オンラインチェック子ターゲット版 */
static	int	ChildTargetHit ;

static	int	OnlineCheckChildTarget( off, def )
TARGET		*off, *def ;
{
    *TMP_HIT = *MIN_HIT ;
    if ( def->class & TARGET_ROTATE ) {
		if ( !OnlineTargetRotate( off,def ) ) return 0 ;
    } else {
		if ( !BoxCheckTarget( MIN_TARG, def ) ) return 0 ;
		if ( !OnlineTarget( FROM, TO, def, MIN_HIT ) ) return 0 ;
		if ( off->class & TARGET_GET_NORMAL ) {
			FVECTOR	b1, b2 ;

			*NORMAL = DG_ZeroVector ;
			{
				FVECTOR		center ;
				GM_TargetGetCenter( &center, def )	;
				_sceVu0SubVector( &b1, &center, &def->size ) ;
				_sceVu0AddVector( &b2, &center, &def->size ) ;
			}
			if ( DG_FABS( MIN_HIT->vx - b1.vx ) < 1.0F ) NORMAL->vx = 1.0F ;
			else if ( DG_FABS( MIN_HIT->vx - b2.vx ) < 1.0F ) NORMAL->vx = -1.0F ;
			else if ( DG_FABS( MIN_HIT->vy - b1.vy ) < 1.0F ) NORMAL->vy = 1.0F ;
			else if ( DG_FABS( MIN_HIT->vy - b2.vy ) < 1.0F ) NORMAL->vy = -1.0F ;
			else if ( DG_FABS( MIN_HIT->vz - b1.vz ) < 1.0F ) NORMAL->vz = 1.0F ;
			else if ( DG_FABS( MIN_HIT->vz - b2.vz ) < 1.0F ) NORMAL->vz = -1.0F ;
		}
    }
    /* 子ターゲットに当たった */
    ChildTargetHit = 1 ;
    if ( !( def->class & TARGET_THROUGH ) ) {
		if ( off->class & TARGET_ONLINE_MIN ) {
			/* 検索直線ターゲットを更新 */
			SetFromToTarget( FROM, MIN_HIT, MIN_TARG ) ;
			MIN_DEF_TARG = def ;
			*MIN_NORMAL = *NORMAL ;
			*TO = *MIN_HIT ;
			return 0 ;			/* 0 を返す */
		} 
		off->hit = *MIN_HIT ;
		off->size = *NORMAL ;
		def->hit = *MIN_HIT ;
		return 1 ;    
    } else {
		/* 防御側がスルーターゲット */
		def->hit = *MIN_HIT ;
		*MIN_HIT = *TMP_HIT ;	/* MIN_HITを更新してはならない */
		return 1 ;
    }
}

/* ターゲット接触 */
static	void	ConflictTarget( off, def )
TARGET		*off, *def ;
{
    void	( *func )( TARGET *, TARGET *, void * ) ;
    int		class ;
	int		dummy = 0 ;

#ifdef DEBUG_MODE
	TargetWork->callbackflag = 1 ;
#endif
    class = ( off->class & TARGET_CHECK_CLASS ) ;
	if ( !( off->class & TARGET_THROUGH ) || 
		 ( def->class & TARGET_CALL_CALLBACK_THROUGH_HIT ) ) {
		def->damaged |= class ;
		def->weapon_type |= off->weapon_type ;
	}

	if ( off->power == NULL ) {
		off->power = &DummyPowerTarget ;
		dummy = 1 ;
	}

    /* コールバック呼び出しは攻撃側が先 */
    if ( !( def->class & TARGET_THROUGH ) ) {
		/* プッシュはやっといた方が都合がいい */
		if ( class & TARGET_PUSH ) GM_PushTarget( off, def ) ;
		off->damaged |= class ;
		func = off->callback ;
		if ( func != NULL ) {
			( *func )( off, def, off->work ) ;
		}
    } 
	if ( !( off->class & TARGET_THROUGH ) || 
		( def->class & TARGET_CALL_CALLBACK_THROUGH_HIT ) ) {
		/* offenseでクリアされているかもしれないので再チェック */
		if ( ( def->damaged & class ) && ( def->weapon_type & off->weapon_type ) ) {
			func = def->callback ;
			if ( func != NULL ) {
				( *func )( off, def, def->work ) ;
			}
		}
	}

	if ( dummy == 1 ) off->power = NULL ;
#ifdef DEBUG_MODE
	TargetWork->callbackflag = 0 ;
#endif
}

/* ターゲット計算 */
static	inline	void	ExecTargetSystem( n_offs, n_defs, offs, defs )
int		n_offs, n_defs ;
TARGET		*offs, *defs ;
{
    TARGET		*off, *def, *child ;
	TARGET		*offnext, *defnext ;
    TARGET_PARTS	*parts ;
    FVECTOR		off_center ;
    int			n, class ;

	offnext = offs->next ;
    while( offnext != NULL ) {
		off = offnext ;
#ifdef DEBUG_MODE
		/* size.vw チェック */
		if ( !( off->class & ( TARGET_DIRECT_ATTACK | TARGET_ONLINE ) ) )
		{
			float	len ;

			len = DG_SQRT( off->size.vx * off->size.vx +
						   off->size.vy * off->size.vy +
						   off->size.vz * off->size.vz ) ;
			//ASSERT( DG_FABS( len - off->size.vw ) > 1.0F ) ;
			if ( DG_FABS( len - off->size.vw ) > 1.0F ) {
				printf( "warning : target size is not calculated.[%x/%f/%f]\n",
					   off, off->size.vw, len ) ;
				off->size.vw = len ;
			}
		}
#endif
		offnext = off->next ;
#if 0	/* ＯＦＦマップでも当たるように */
		/* マップ判定 */
		if ( !( off->map & GM_CurrentStageMap ) &&
			 !( off->class & TARGET_DIRECT_ATTACK ) ) goto offense_skip ;
#endif
		class = off->class ;
		if ( class & TARGET_ONLINE ) {
			/* 検索直線ターゲットを作成 */
			SetFromToTarget( &( off->center ), &( off->offset ), MIN_TARG ) ;
			*FROM = off->center ;
			*TO = off->offset ;
            *MIN_HIT = off->offset ;
			MIN_DEF_TARG = MIN_DEF_TARG2 = NULL ;
		}
		if ( class & TARGET_ROTATE ) {
			FVECTOR	offset ;
			/* 回転ターゲットの場合、
			   オフセットを加算したセンター位置を計算しておく */
			_sceVu0ApplyMatrix( &offset, &( off->world ), &( off->offset ) ) ;
			_sceVu0AddVector( &off_center, &( off->center ), &offset ) ;
		} else {
			_sceVu0AddVector( &off_center, &( off->center ), &( off->offset ) ) ;
		}
		defnext = defs->next ;
		while( defnext != NULL ) {
			def = defnext ;
#ifdef DEBUG_MODE
			/* size.vw チェック */
			{
				float	len ;

				len = DG_SQRT( def->size.vx * def->size.vx +
							  def->size.vy * def->size.vy +
							  def->size.vz * def->size.vz ) ;
				//ASSERT( DG_FABS( len - def->size.vw ) > 1.0F ) ;
				if ( DG_FABS( len - def->size.vw ) > 1.0F ) {
					printf( "warning : target size is not calculated.[%x/%f/%f]\n",
						   def, def->size.vw, len ) ;
					def->size.vw = len ;
				}
			}
#endif
			defnext = def->next ;
			/* ダイレクト */
			if ( off->class & TARGET_DIRECT_ATTACK ) {
				TARGET		*direct_def ;
				
				direct_def = ( TARGET * )off->parts ;
				if ( !( direct_def->class & TARGET_SKIP ) ) {
					ConflictTarget( off, direct_def ) ;
				}
				break ;
			}
			/* マップ判定 */
//            if ( !( def->map & off->map ) ) continue ;
            if ( !( def->map & off->map ) &&
				 ( !( def->map & GM_CurrentStageMap ) ||
				   !( off->map & GM_CurrentStageMap ) ) ) {
				continue ;
			}

			/* サイド判定 */
			if ( !( off->side & def->side ) ) continue ;
			/* クラス判定 */
			if ( !CheckClass( class, def->class ) ) continue ;
			/* 名前チェック */
			if ( ( off->class & TARGET_HIT_SAMENAME ) &&
				 ( off->class & TARGET_HAS_NAME ) ) {
				/* 同じ名前しか当たらないモード */
				if ( !( def->class & TARGET_HIT_SAMENAME ) ||
					 !( def->class & TARGET_HAS_NAME ) ) {
					continue ;
				}
				if ( off->name != def->name ) continue ;
			} else if ( ( def->class & TARGET_HIT_SAMENAME ) &&
					    ( def->class & TARGET_HAS_NAME ) ) {
				continue ;
			} else if ( ( off->class & TARGET_HAS_NAME ) && 
					    ( def->class & TARGET_HAS_NAME ) &&
					    ( off->name == def->name ) ) {
				/* 同じ名前には当たらないモード */
				continue ;
			} 

			/* 箱チェック ＯＲ オンラインチェック */
			if ( class & TARGET_ONLINE ) {
				if ( !OnlineCheckTarget( off, def ) ) continue ;
			} else {	
#if 1
				if ( off->class & TARGET_ROTATE ||
					def->class & TARGET_ROTATE ) {
					if ( !BoxCheckTargetRotate( off, def, &off_center ) ) continue ;
				} else {
					if ( !BoxCheckTarget( off, def ) ) continue ;
				}
#else
				if ( !BoxCheckTarget( off, def ) ) continue ;			
#endif
			}
			/* 子ターゲットのチェック */
			/* 子チェックはオンラインのみ */
			if ( ( ( !( class & TARGET_CHILD_SKIP ) && ( class & TARGET_CHILD ) ) ||
				  ( def->class & TARGET_CHILD_ALWAYS ) ) &&
				( def->class & TARGET_CHILD ) &&
				( class & TARGET_ONLINE ) ) {
				ChildTargetHit = 0 ;
				parts = def->parts ;
				while( parts != NULL ) {
					if ( parts->flag & TARGET_SKIP ) goto child_link_next ;
					n = parts->n_parts ;
					child = ( TARGET * )parts->parts ;
					while( -- n >= 0 ) {
						/* クラス判定 */
						if ( !CheckClass( class, child->class ) ) 
							goto child_skip ;
						if ( !OnlineCheckChildTarget( off, child ) ) goto child_skip ;
						ConflictTarget( off, child ) ;
child_skip :		    
						child ++ ;
					}
					/* このレベルでの子ターゲット当たりがあれば終了 */
					if ( ChildTargetHit ) break ;
child_link_next :
                    parts = parts->next ;
				}
			} else {
				ConflictTarget( off, def ) ;
				if ( class & TARGET_CHECK_ONE ) break ;
			}
		}
		if ( class & TARGET_ONLINE_MIN && 
			MIN_DEF_TARG != NULL ) {
			off->hit = *MIN_HIT ;
			off->size = *MIN_NORMAL ;
			MIN_DEF_TARG->hit = *MIN_HIT ;
			ConflictTarget( off, MIN_DEF_TARG ) ;
		}
offense_skip :
//		off = off->next ;
				;
    }
}

/* アクト */
static	void	Act( work )
Work		*work ;
{
    TARGET	*offs, *defs ;
    int		n_offs, n_defs ;
#ifdef DEBUG_FLAG
    if ( GV_PadData[ 0 ].press & PAD_X ) TrgDebugFlag = 1 ;
#endif
    n_offs = work->N_OffenseTargets ;
    if ( n_offs == 0 ) return ;
	if ( PL_AttackDisable() || GV_PauseLevel != 0 ) {
		ClearOffenseTargets( work ) ;
		return ;
	}
    n_defs = work->N_DefenseTargets ;
    if ( n_defs != 0 ) {
		offs = work->OffenseList ;
		defs = work->DefenseList ;
		ExecTargetSystem( n_offs, n_defs, offs, defs ) ;
    }
    ClearOffenseTargets( work ) ;

//	ExecDirectAttackList( work ) ;

#ifdef DEBUG_FLAG
    if ( TrgDebugFlag ) {
		TrgDebugFlag = 0 ;
    }
#endif
}

static	void	Die( Work *work )
{
	TargetWork = NULL ;
}

/*-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------*/

/* ターゲットシステム起動 */
void	*GM_StartTargetSystem( void )
{
    Work	*work ;
#if 0
    work = &TargetWorkStatic ;
	GV_ZeroMemory( &work->actor, sizeof( Work ) ) ;	
	GV_SetActorFreeFunc( &work->actor, NULL ) ;
	GV_SetActorClass( &work->actor, GV_CLASS_OBJECT ) ;
	GV_SetActorKillLevel( &work->actor, GV_KILL_LEVEL_NORMAL ) ; 
	GV_InsertActorPriority( GV_ACTOR_AFTER2, &work->actor, GM_TARGET_ACTOR_PRIO ) ;
#else
	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), GM_TARGET_ACTOR_PRIO ) ;
#endif
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_SetActorKillLevel( &( work->actor ), GV_KILL_LEVEL_NORMAL ) ;
		TargetWork = work ;
		GM_InitTargetSystem() ;
    }
    return ( void * )work ;
}


/*-----------------------------------------------------------------------------*/

/* 防御用ターゲット列の先頭を返す */
TARGET	*GM_TrgDefListGetTop( void )
{	
	if ( TargetWork == NULL ) return NULL ;
    return TargetWork->DefenseList->next ;
}

/*-----------------------------------------------------------------------------*/

/* 直接指定で特定のターゲットに攻撃を当てる */
void		GM_TargetSetDirectAttack( atk, def )
TARGET		*atk, *def ;
{
	atk->class |= TARGET_DIRECT_ATTACK ;
	atk->parts = ( TARGET_PARTS * )def ;
}

/*-----------------------------------------------------------------------------*/

/* 動的ハザードにターゲット登録 */
void	GM_SetTargettoDynamicHazard( void *seg, TARGET *target, int type )
{
	if ( type == HZX_TYPE_SEGMENT ) {
		( ( HZX_D_SEGMENT * )seg )->target = target ;
	} else {
		( ( HZX_D_FLOOR * )seg )->target = target ;
	}
}

/* 動的ハザードにターゲットがヒット */
/* 動的ハザードにターゲットがセットしてある場合、
   攻撃ターゲットを登録して、ターゲット処理フェイズで
   コールバックを呼ばせる */
void		GM_TargetHitDynamicHazard( void	*ptr, TARGET *off, int type )
{
	TARGET			*def ;

	if ( type == HZX_TYPE_SEGMENT ) {
		if ( ( ( HZX_SEG * )ptr )->ptr == NULL ) return ;
		def = ( TARGET * )( ( ( HZX_D_SEGMENT * )( ( ( HZX_SEG * )ptr )->ptr ) )->target ) ;
	} else {
		if ( ( ( HZX_FLR * )ptr )->ptr == NULL ) return ;
		def = ( TARGET * )( ( ( HZX_D_FLOOR * )( ( ( HZX_FLR * )ptr )->ptr ) )->target ) ;
	}
	if ( def == NULL ) return ;
	GM_TargetSetDirectAttack( off, def ) ;
	GM_PutTarget( off ) ;
}
