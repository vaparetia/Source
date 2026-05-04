/*
   lockon_sight.c
   ロックオンサイト
   
   2001/02/09	M.Sonoyama
   $Id: lockon_sight.c,v 1.1.1.3 2002/11/19 11:50:28 Yoshizawa1 Exp $
*/

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

#include	"gameheader.h"
#include	"camera.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

#define		SIGHT		(2308909)	/* lockon.o2d */
#define		MAX_SIGHTS	(2)

#define		DEFAULT_SIGHT_SIZE		(15)

/*----------------------------------------------------------------*/

/* グローバル */
TARGET		*PL_LockonTarget = NULL ;

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX			actor ;
	int					handle_2d ;
	void				*layout_player ;
	SPR_OBJ				*root ;
	SPR_EMPTY			*sight[ MAX_SIGHTS ] ;
	int					count ;
	int					lockon_count ;
} Work ;

typedef	struct	{
	FMATRIX				world ;
	FVECTOR				center ;
	FVECTOR				disp_pos ;
	SPR_POS				sight_pos ;
	float				scale ;
	TARGET				*lockonTarget ;
	int					newlock ;
} ScrPad ;

#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)
#define	WORLD		(&(SCRPAD->world))
#define	CENTER		(&(SCRPAD->center))
#define	DISP_POS	(&(SCRPAD->disp_pos))
#define	SIGHT_POS		(&(SCRPAD->sight_pos))
#define	SCALE		(SCRPAD->scale)
#define	LOCKONTRG	(SCRPAD->lockonTarget)
#define NEWLOCK		(SCRPAD->newlock)

/*----------------------------------------------------------------*/

/* 画面内チェック */
static	int		CheckTargetInSight( TARGET *t )
{
	FVECTOR		max, min, offset ;

	if ( t->class & TARGET_ROTATE ) {
		_sceVu0ApplyMatrix( &offset, &t->world, &t->offset ) ;
		DG_COPY_MAT( WORLD, &t->world ) ;
	} else {
		DG_COPY_VEC( &offset, &t->offset ) ;
		DG_COPY_MAT( WORLD, &DG_UnitMatrix ) ;
	}
	_sceVu0AddVector( CENTER, &t->center, &offset ) ;
	GV_VecToMat( CENTER, WORLD ) ;
	DG_COPY_VEC( &max, &t->size ) ; max.vw = 1.0F ;
	_sceVu0SubVector( &min, &DG_ZeroVector, &max ) ; min.vw = 1.0F ;
	if ( DG_BoundCheck( WORLD, &max, &min ) & 0x01 ) return 0 ;
	DG_TransPersOne( DISP_POS, CENTER ) ;

//DumpVec( DISP_POS ) ;
	return 1 ;
}

/* サイトの表示 */
static	void	DisplaySight( SPR_EMPTY *emp )
{
	float			len ;

	SIGHT_POS->x = DISP_POS->vx ;
	SIGHT_POS->y = DISP_POS->vy * 384.0F / 448.0F ;
//printf( "<%d> %f %f\n", GV_Time, SIGHT_POS->x, SIGHT_POS->y ) ;
	SPR_SetPosEmpty( ( SPR_OBJ * )emp, SIGHT_POS ) ;
	len = GV_VecLen3F2( CENTER, &( GM_GetCurrentCamera( 0 )->position ) ) ;
	SCALE = 8.0F - len / 2000.0F ;
	if ( SCALE < 1.0F ) SCALE = 1.0F ;
	SPR_MAG( emp, SCALE ) ;
	SPR_SHOW( emp ) ;
}

/* ロックオンできるか？ */
static	void	CheckLockonAvailable( TARGET *t )
{
	float		size ;
	FVECTOR		center ;

	/* ミサイル飛行中は新たにロックオンしない */
	if ( GM_WeaponAlive & WP_ALIVE_STINGER ) return ;
	size = DEFAULT_SIGHT_SIZE * SCALE ;
	center.vx = ( float )( DRAW_WIDTH / 2 ) - DISP_POS->vx ; 
	center.vy = ( float )( DRAW_HEIGHT / 2 + 32 ) - DISP_POS->vy ; 
	if ( center.vx < - size || center.vx > size ||
		 center.vy < - size || center.vy > size ) return ;
	LOCKONTRG = t ;
	NEWLOCK = 1 ;
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int			time, n_targets, i ;
	SPR_EMPTY	*cur_sight ;
	TARGET		*t, *tnext, *prelock ;

//	if ( work->count == 0 ) SPR_HIDE( work->root ) ;
	time = work->count ;
	work->count ++ ;

	prelock = PL_LockonTarget ;
	if ( prelock != NULL && !( GM_WeaponAlive & WP_ALIVE_STINGER ) ) {
		if ( ++ work->lockon_count > 60 ) {	/* 時間切れ */
			prelock = NULL ;
			work->lockon_count = 0 ;
		}
	}
	LOCKONTRG = prelock ;
	NEWLOCK = 0 ;
	n_targets = 0 ;
	tnext = GM_TrgDefListGetTop() ;
	for ( i = 0; i < MAX_SIGHTS; i ++ ) SPR_HIDE( work->sight[ i ] ) ;
	while( tnext != NULL ) {
		if ( n_targets >= MAX_SIGHTS ) break ;
		t = tnext ;
		tnext = t->next ;
		cur_sight = work->sight[ n_targets ] ;
		if ( !( t->class & TARGET_LOCKON ) ) continue ;
		if ( !( t->map & GM_CurrentStageMap ) ) continue ;
		if ( t->side == PLAYER_SIDE ) continue ;
		if ( !CheckTargetInSight( t ) ) continue ;
		DisplaySight( cur_sight ) ;
		CheckLockonAvailable( t ) ;
		n_targets ++ ;
	}

	if ( LOCKONTRG != NULL && NEWLOCK == 0 ) {
		int		exist = 0 ;
		/* 以前からのロック先が選ばれた場合、
		   それがＴＡＲＧＥＴリストに載っているか調べる */
		tnext = GM_TrgDefListGetTop() ;		
		while( tnext != NULL ) {
			t = tnext ;
			tnext = t->next ;
			if ( t == LOCKONTRG ) {
				exist = 1 ;
				break ;
			}
		}
		if ( exist == 0 ) LOCKONTRG = NULL ;
	}

	if ( LOCKONTRG != NULL && prelock != LOCKONTRG ) {
		/* 違う奴にロックした */
		work->lockon_count = 0 ;
	}
	PL_LockonTarget = LOCKONTRG ;
//	if ( PL_LockonTarget != NULL ) printf( "<%d> %x \n", work->lockon_count, LOCKONTRG ) ;
}

static	void	Die( Work *work )
{
	if ( work->handle_2d >= 0 ) L2D_ReleaseLayout( work->handle_2d ) ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work )
{
	void		*layout_player ;
	int			i, handle, strcode ;
	SPR_OBJ		*spr ;
#if 0
	/* 常駐で起動される */
	layout_player = NewLayout2D_Player_for_Prog() ;
	if ( layout_player != NULL ) {
		work->layout_player = layout_player ;
		GV_SetActorChild( work, layout_player ) ;
	}
#endif
	work->handle_2d = -1 ;
	handle = L2D_LoadLayout( SIGHT, DG_CHANL_MENU, 0, 0 ) ;
printf( "handle %d\n", handle ) ;
	if ( handle < 0 ) return -1 ;
	work->handle_2d = handle ;
	L2D_EvokeAction( handle, GV_StrCode( "FirstAction" ) ) ;

	spr = L2D_GetObject( handle, GV_StrCode( "ROOT-1" ) ) ;
	if ( spr == NULL ) return -1 ;
	work->root = spr ;

	strcode = GV_StrCode( "sight1" ) ;
	for ( i = 0; i < MAX_SIGHTS; i ++, strcode ++ ) {
		spr = L2D_GetObject( handle, strcode ) ;
		if ( spr == NULL ) return -1 ;
		work->sight[ i ] = ( SPR_EMPTY * )spr ;
	}

	PL_LockonTarget = NULL ;	/* 起動時は初期化 */

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewLockonSight( void )
{
	Work		*work ;

	PL_LockonTarget = NULL ;
	work = GV_NewActorPrio( GV_ACTOR_PLAYER, sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work ) < 0 ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	return work ;
}


