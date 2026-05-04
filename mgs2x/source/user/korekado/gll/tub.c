//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tub.c
	
	2002/8/15 Y.Korekado
	$Id: tub.c,v 1.3 2002/11/23 12:46:56 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/conv/define.h"
#include	"gll_def.h"

/*----------------------------------------------------------------*/
#define	BODY_NAME	(2685052)	/*gll_tarai_mt*/
#define PRIO	0x40
#define TARGET_CLASS	(TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK)

static FVECTOR	TargetSize = { 2000.0, 300.0, 2000.0 } ;
static FVECTOR	TargetShift = { 0.0, 0.0, 0.0 } ;

static FVECTOR	StartPos = { 0.0, 13300.0, -16500.0 } ;
#define GROUND_NUM	(0)	/*最終目標地点*/
static FVECTOR	GroundPos[3] = {
	{ 0.0, 2000.0, -12500.0 },
	{ 4500.0, 0.0, -16000.0 },
	{ -4500.0, 0.0, -16000.0 },
} ;
#define MAX_SHELF (4)
static FVECTOR ShlefShift[MAX_SHELF] = {
	{ 0.0, -400.0, 2000.0 },
	{ 2000.0, -400.0, 0.0 },
	{ 0.0, -400.0, -2000.0 },
	{ -2000.0, -400.0, 0.0 }
} ;
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;
	OBJECT		body ;
	OBJECT		*gll_body ;
	FMATRIX 	*lights ;
	
	TARGET		deftrg ;	/* 防御ターゲット */
	POWER_TARGET	power ;	/* 防御属性 */
	FVECTOR		off_center ;

	int			damaged ;
	int			think ;
	int			count ;
	int			tmp_time ;
	int			tmp_flag ;
	int			shadow ;
	int			fall_count ;
	int			map ;
	int			shelf[4] ;
	int			dam_shelf ;

	FVECTOR		head_top ;
} Work ;

/*- damaged ------------------------------------------------------*/
#define	TUB_DAM_DROP		0x00000001
#define	TUB_DAM_BULLET		0x00000002
#define	TUB_DAM_BREAK		0x00000004
#define	TUB_DAM_BOMB		0x00000008
#define	TUB_DAM_BOMB_TUB	0x00000010
#define	TUB_DAM_BULLET_TUB	0x00000020

/*----------------------------------------------------------------*/
extern void  *NewSpreadFlour(FMATRIX *world ) ;
extern void  *NewFlour_Splay(FMATRIX  *world) ;
extern void *NewFlour_Fall( int map, int name, FVECTOR *hit, FVECTOR *force );
extern void *NewFlour_Down( int map, int name, FVECTOR *hit, FVECTOR *force );
extern void *NewGllFlourSara( int map, int name, FVECTOR *hit, FVECTOR *force, int mode );
extern void *NewFlour_Gas3( FVECTOR *center );
/*----------------------------------------------------------------*/
enum {
	THINK_IDLE, THINK_DROP, THINK_DROP_2, THINK_GROUND, THINK_BREAK,
	THINK_BOMB, THINK_BULLET, THINK_BOMB_TUB, THINK_BULLET_TUB
} ;
/*----------------------------------------------------------------*/
//#define PARABOLA_TIME	(60)
#define PARABOLA_TIME	COUNT_VMODE(120)
#define GRAVITATION		(4)
static float SetPalabolaForce( FVECTOR *from, FVECTOR *to, int time, int gv, FVECTOR *out_step )
{
	FVECTOR vec ;
	int dir, x_dis, y_dis ;
	float	s, ss, speed ;

//printf( " form [%f] [%f] [%f] \n",from->vx, from->vy, from->vz ) ;
//printf( " to [%f] [%f] [%f] \n",to->vx, to->vy, to->vz ) ;

	_sceVu0SubVector(  &vec, to, from ) ;
	dir = _FVecDir2( &vec ) ;
	x_dis = _FVecLen2( &vec ) ; 
	y_dis = to->vy - from->vy ;

	speed = (float)x_dis / (float)time ;
	s = (float)y_dis/(float)time ;
	ss = (float)gv*(float)time/2.0 ;

	out_step->vx = speed * _RsinF( dir ) ;
	out_step->vz = speed * _RcosF( dir ) ;
	out_step->vy = s + ss ;

	return speed ;
}

static void  VecToMat(FMATRIX *world, FVECTOR *force, FVECTOR *hit, int mode)
{
    FVECTOR   base = {0.0F, -1.0F, 0.0F,0.0F} ;
    FVECTOR   ftmp[3] ;
    int       tmp ;

    /* 1つめのベクトル */
    _sceVu0Normalize(&ftmp[0],force) ;
    
    /* force と base の外せきを求める */
    _sceVu0OuterProduct(&ftmp[1],&base,force) ;
    /* 2つめのベクトル */
    _sceVu0Normalize(&ftmp[1],&ftmp[1]) ;
    /* 3つめのベクトル */
    _sceVu0OuterProduct(&ftmp[2],&ftmp[1],&ftmp[0]) ;

    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[2]) = ftmp[tmp] ;
    
    /* 2ビットずらす */
    mode /= 4 ;
    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[1]) = ftmp[tmp] ;
    
    /* 2ビットずらす */
    mode /= 4 ;
    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[0]) = ftmp[tmp] ; 

    *((FVECTOR *)world->m[3]) = *hit ;
      world->m[3][3] = 1.0F ;
}

/* 跳弾 */
static void CallSpark( FVECTOR *pos ,FVECTOR *force )
{
	extern void *NewSpark( FMATRIX *world ) ;
    SVECTOR	rot ;
    FMATRIX	w ;

    _FVecToRotXY( force, &rot ) ;
    DG_SetPos2( pos, &rot ) ;
    DG_GetPos( &w ) ;
    NewSpark( &w ) ;
}

/* 落ちる粉 */
//static FVECTOR	 FlourFallShift= { 0.0, 1000.0, 2550.0 } ;
static FVECTOR	 FlourFallShift= { 0.0, 500.0, 2750.0 } ;
static FVECTOR	 FlourFallForce= { 0.0, 300.0, -200.0 } ;
static void CallFlourFall( Work *work, FVECTOR	*pos )
{
	CONTROL	*ctrl ;
	FMATRIX	mat ;
	SVECTOR rot ;
	FVECTOR vec, f_pos, f_force ;
	float	speed, y_speed ;
	int dir, f_dir ;
	
	ctrl = &work->control ;
	_sceVu0SubVector(  &vec, pos, &ctrl->mov ) ;
	dir = _FVecDir2( &vec ) ;
	
printf(" floureeeeeeeeeee dir [%d]\n",dir ) ;
	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	f_dir = 0 ;
	if ( (dir > 512) && (dir <= 1536) ) {
		f_dir = 1024 ;
	} else if ( (dir > 1536) && (dir <= 2560) ) {
		f_dir = 2048 ;
	} else if ( (dir > 2560) && (dir <= 3584) ) {
		f_dir = 3072 ;
	}


	speed = STEP_VMODE(-1.0) ;
	y_speed = STEP_VMODE(-300.0) ;
	DG_SetPos2( &ctrl->mov, &DG_ZeroSVector ) ;
	DG_GetPos( &mat ) ;

	rot.vy = f_dir - 64 ;
	DG_RotatePos( &rot );
	DG_PutVector( &FlourFallShift, &f_pos, 1 );
	f_force.vx = speed * _RsinF( (int)rot.vy ) ;
	f_force.vy = y_speed ;
	f_force.vz = speed * _RcosF( (int)rot.vy ) ;
//printf("f_pos[%f][%f][%f]\n",f_pos.vx,f_pos.vy,f_pos.vz ) ;
	NewGllFlourSara( work->map,0, &f_pos,&f_force, 1 );//垂直粉
/*
	DG_SetPos ( &mat ) ;
	rot.vy = f_dir - 32 ;
	DG_RotatePos( &rot );
	DG_PutVector( &FlourFallShift, &f_pos, 1 );
	f_force.vx = speed * _RsinF( (int)rot.vy ) ;
	f_force.vy = y_speed ;
	f_force.vz = speed * _RcosF( (int)rot.vy ) ;
	NewGllFlourSara( work->map,0, &f_pos,&f_force, 1 );//垂直粉

	DG_SetPos ( &mat ) ;
	rot.vy = f_dir + 64 ;
	DG_RotatePos( &rot );
	DG_PutVector( &FlourFallShift, &f_pos, 1 );
	f_force.vx = speed * _RsinF( (int)rot.vy ) ;
	f_force.vy = y_speed ;
	f_force.vz = speed * _RcosF( (int)rot.vy ) ;
	NewGllFlourSara( work->map,0, &f_pos,&f_force, 1 );//垂直粉
*/
	DG_SetPos ( &mat ) ;
	rot.vy = f_dir + 128 ;
	DG_RotatePos( &rot );
	DG_PutVector( &FlourFallShift, &f_pos, 1 );
	f_force.vx = speed * _RsinF( (int)rot.vy ) ;
	f_force.vy = y_speed ;
	f_force.vz = speed * _RcosF( (int)rot.vy ) ;
	NewGllFlourSara( work->map,0, &f_pos,&f_force, 1 );//垂直粉
}

static	int	DamageCheck( Work *work )
{
	FMATRIX	   world ; 
	FVECTOR	   hit ;
	FVECTOR	   t_force ;
	int		   mode, i, num ;
	float		 tmp ;
	TARGET		*trg  ;
	long64		weapon ;
	
	trg = &work->deftrg ;
	work->damaged = 0 ;

	/* 棚から影響するダメージ */
	{
		num = 0 ;
		for ( i=0; i<MAX_SHELF; i++) {
			if ( work->shelf[i] & GLL_SHELF_DESTROY ) num++ ;
		} 
		if ( num >= 3 ) {
			SET_FLAG( GLL_GAME_STATUS, GLL_GS_TUB_DORP ) ;
			work->damaged = TUB_DAM_DROP ;
			return 1 ;
		}
		for ( i=0; i<MAX_SHELF; i++) {
			if ( work->shelf[i] & GLL_SHELF_BOMB )  {
				work->damaged = TUB_DAM_BOMB ;
				work->dam_shelf = i ;
				GLL_ACHOO_TIME += COUNT_VMODE(400) ;
				return 1 ;
			}
		} 
		for ( i=0; i<MAX_SHELF; i++) {
			if ( work->shelf[i] & GLL_SHELF_BREAK )  {
	GM_SeSetMode( SD_A_TARAIKRO, &work->control.mov, GM_SEMODE_BOMB ) ;
				work->damaged = TUB_DAM_BREAK ;
				work->dam_shelf = i ;
				GLL_ACHOO_TIME += COUNT_VMODE(500) ;
				return 1 ;
			}
		} 
		for ( i=0; i<MAX_SHELF; i++) {
			if ( work->shelf[i] & GLL_SHELF_DAMAGE )  {
				work->damaged = TUB_DAM_BULLET ;
				work->dam_shelf = i ;
				GLL_ACHOO_TIME += COUNT_VMODE(30) ;
				return 1 ;
			}
		}
	}

	/* タライ自身のダメージ */
	if ( TARGET_POWER & trg->damaged ) {
//printf("tub damage: [%lx] \n",trg->weapon_type ) ;
		weapon = trg->weapon_type ;
		trg->weapon_type = 0 ;
		trg->damaged = 0 ;
		t_force = trg->power->force ;

		if ( weapon & (WP_BLAST) ) {
			GM_SeSetMode( SD_A_TARAIKRO, &work->control.mov, GM_SEMODE_BOMB ) ;
			GM_SeSetMode( SD_A_TARAIREC, &work->control.mov, GM_SEMODE_BOMB ) ;
			work->damaged = TUB_DAM_BOMB_TUB ;
			GLL_ACHOO_TIME += COUNT_VMODE(800) ;
			return 1 ;
		}
		if ( weapon & (WP_BULLET) ) {
			GM_SeSetMode( SD_A_RICOCH02, &work->control.mov, GM_SEMODE_BOMB ) ;
			work->damaged = TUB_DAM_BULLET_TUB ;
			CallSpark( &trg->hit , &trg->power->force ) ;

			/* worldを求める */
			/* 小麦粉 拡散 */					
			if( weapon & WP_BLOW ){
				_sceVu0AddVector( &hit, &trg->center, &trg->offset );
			}else{
				DG_COPY_VEC( &hit, &trg->hit );
			}
			GM_SeSetMode( SD_A_RICKOM01, &hit, GM_SEMODE_BOMB ) ;
			world = DG_UnitMatrix ;
			DG_COPY_VEC( (FVECTOR *)world.m[3], &trg->hit );
			NewSpreadFlour(&world) ;

			/* 飛び散りの向きを測定 */
			if(t_force.vy < 0.0F)
			t_force.vy = - t_force.vy ;
			tmp = bp_sqrtf(t_force.vx * t_force.vx + t_force.vz * t_force.vz) ;  //BP_MATH - emulate PS2 sqrtf
			if(t_force.vy < tmp)
			t_force.vy = tmp ;
			mode = 0x0012 ;
			VecToMat(&world,&t_force,&hit,mode) ;

			if( work->fall_count == 0 ){
				work->fall_count = COUNT_VMODE(30) ;
//				NewFlour_Fall( work->map,0, &hit,&t_force );
				NewGllFlourSara( work->map,0, &hit,&trg->power->force, 0 );//垂直粉
				GLL_ACHOO_TIME += COUNT_VMODE(300) ;
			}
			NewFlour_Gas3(&hit);
			/* 飛び散りの小麦粉 */
			NewFlour_Splay( &world ) ;

			GLL_ACHOO_TIME += COUNT_VMODE(100) ;
			return 1 ;
		}
	}
	return 0 ;
}

/*----------------------------------------------------------------*/
static void ThinkIdle( Work *work )
{
	if ( DamageCheck( work ) ) {
		if ( work->damaged & TUB_DAM_DROP ) {
			work->think = THINK_DROP ;
		} else if ( work->damaged & TUB_DAM_BREAK ) {
			work->think = THINK_BREAK ;
		} else if ( work->damaged & TUB_DAM_BOMB ) {
			work->think = THINK_BOMB ;
		} else if ( work->damaged & TUB_DAM_BOMB_TUB ) {
			work->think = THINK_BOMB_TUB ;
		} else if ( work->damaged & TUB_DAM_BULLET_TUB ) {
			work->think = THINK_BULLET_TUB ;
		} else {
			work->think = THINK_BULLET ;
		}
		work->count = 0 ;
		return ;
	}
	work->count ++ ;
}

static void ThinkBullet( Work *work )
{
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;
	if ( DamageCheck( work ) ) {
		if ( work->damaged & TUB_DAM_DROP ) {
			work->think = THINK_DROP ;
		} else if ( work->damaged & TUB_DAM_BREAK ) {
			work->think = THINK_BREAK ;
		} else if ( work->damaged & TUB_DAM_BOMB ) {
			work->think = THINK_BOMB ;
		} else if ( work->damaged & TUB_DAM_BOMB_TUB ) {
			work->think = THINK_BOMB_TUB ;
		} else if ( work->damaged & TUB_DAM_BULLET_TUB ) {
			work->think = THINK_BULLET_TUB ;
		} else {
			work->think = THINK_BULLET ;
		}
		work->count = 0 ;
		return ;
	}

	if ( ctrl->rot.vz == 16 ) {
		ctrl->rot.vz = ctrl->turn.vz = -16 ;
		ctrl->mov.vx = StartPos.vx - 32.0 ;
	} else {
		ctrl->rot.vz = ctrl->turn.vz = 16 ;
		ctrl->mov.vx = StartPos.vx + 32.0 ;
	} 

	if ( work->count == 3 ) {
		ctrl->rot.vz = ctrl->turn.vz = 0 ;
		ctrl->mov.vx = StartPos.vx ;
		work->think = THINK_IDLE ;
		work->count = 0 ;
		return ;
	}

	work->count ++ ;
}

static void ThinkBomb( Work *work )
{
	CONTROL	*ctrl ;
	float	h ;
	
	ctrl = &work->control ;
	if ( DamageCheck( work ) ) {
		if ( work->damaged & TUB_DAM_DROP ) {
			work->think = THINK_DROP ;
			work->count = 0 ;
			return ;
		} else if ( work->damaged & TUB_DAM_BREAK ) {
			work->think = THINK_BREAK ;
			work->count = 0 ;
			return ;
		} else if ( work->damaged & TUB_DAM_BOMB ) {
			work->think = THINK_BOMB ;
			work->count = 0 ;
			return ;
		} else if ( work->damaged & TUB_DAM_BOMB_TUB ) {
			work->think = THINK_BOMB_TUB ;
			work->count = 0 ;
			return ;
		}
	}

	if ( work->count == 0 ) {
		ctrl->mov.vy = StartPos.vy + 600.0 ;
		if ( (work->dam_shelf == 1) || (work->dam_shelf == 3) ) {
			ctrl->rot.vz = ctrl->turn.vz = 256 ;
			work->tmp_flag = 0 ;
		} else {
			ctrl->rot.vx = ctrl->turn.vx = 256 ;
			work->tmp_flag = 2 ;
		}
	}

	h = 0.0 ;
	if ( work->tmp_flag == 0 ) {
		ctrl->rot.vz = ctrl->turn.vz -= STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 1 ;
		}
	} else if ( work->tmp_flag == 1 ) {
		ctrl->rot.vz = ctrl->turn.vz += STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz+2048 ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 0 ;
		}
	} else if ( work->tmp_flag == 2 ) {
		ctrl->rot.vx = ctrl->turn.vx -= STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vx ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 3 ;
		}
	} else if ( work->tmp_flag == 3 ) {
		ctrl->rot.vx = ctrl->turn.vx += STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vx+2048 ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 2 ;
		}
	}

//printf("bomb [%d] rot vz[%d] vy[%f] h[%f]\n",work->count, ctrl->rot.vz, ctrl->mov.vy, h ) ;
	if ( work->count >= COUNT_VMODE(60) ) {
		ctrl->mov.vy = StartPos.vy ;
		ctrl->rot.vx = ctrl->turn.vx = 0 ;
		ctrl->rot.vz = ctrl->turn.vz = 0 ;
		work->tmp_flag = 0 ;
		work->think = THINK_IDLE ;
		work->count = 0 ;
		return ;
	}

	ctrl->mov.vy -= STEP_VMODE(10.0) ;
	work->count ++ ;
}

static void ThinkBulletTub( Work *work )
{
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;
	if ( DamageCheck( work ) ) {
		if ( work->damaged & TUB_DAM_DROP ) {
			work->think = THINK_DROP ;
		} else if ( work->damaged & TUB_DAM_BREAK ) {
			work->think = THINK_BREAK ;
		} else if ( work->damaged & TUB_DAM_BOMB ) {
			work->think = THINK_BOMB ;
		} else if ( work->damaged & TUB_DAM_BOMB_TUB ) {
			work->think = THINK_BOMB_TUB ;
		} else if ( work->damaged & TUB_DAM_BULLET_TUB ) {
			work->think = THINK_BULLET_TUB ;
		} else {
			work->think = THINK_BULLET ;
		}
		work->count = 0 ;
		return ;
	}

	if ( ctrl->rot.vz == 16 ) {
		ctrl->rot.vz = ctrl->turn.vz = -16 ;
		ctrl->mov.vy = StartPos.vy - 32.0 ;
	} else {
		ctrl->rot.vz = ctrl->turn.vz = 16 ;
		ctrl->mov.vy = StartPos.vy + 32.0 ;
	} 

	if ( work->count == 3 ) {
		ctrl->rot.vz = ctrl->turn.vz = 0 ;
		ctrl->mov.vy = StartPos.vy ;
		work->think = THINK_IDLE ;
		work->count = 0 ;
		return ;
	}

	work->count ++ ;
}

static void ThinkBombTub( Work *work )
{
	FVECTOR vec ;
	CONTROL	*ctrl ;
	float	h ;
	
	ctrl = &work->control ;
	if ( DamageCheck( work ) ) {
		if ( work->damaged & TUB_DAM_DROP ) {
			work->think = THINK_DROP ;
			work->count = 0 ;
			return ;
		} else if ( work->damaged & TUB_DAM_BREAK ) {
			work->think = THINK_BREAK ;
			work->count = 0 ;
			return ;
		} else if ( work->damaged & TUB_DAM_BOMB ) {
			work->think = THINK_BOMB ;
			work->count = 0 ;
			return ;
		}
	}

	if ( work->count == 0 ) {
		ctrl->mov.vy = StartPos.vy + 1200.0 ;
		if ( (work->dam_shelf == 1) || (work->dam_shelf == 3) ) {
			ctrl->rot.vz = ctrl->turn.vz = 256 ;
			work->tmp_flag = 0 ;
		} else {
			ctrl->rot.vx = ctrl->turn.vx = 256 ;
			work->tmp_flag = 2 ;
		}
		if ( work->dam_shelf < 4 ) {
			_sceVu0AddVector( &vec, &ShlefShift[work->dam_shelf], &ctrl->mov ) ;
			CallFlourFall( work, &vec ) ;
		}
	}

	if ( work->count == COUNT_VMODE(15) ) {
		int	sh ;
		
		sh = work->dam_shelf + 2 ;
		if ( sh >= 4 ) sh %= 4 ;
		_sceVu0AddVector( &vec, &ShlefShift[sh], &ctrl->mov ) ;
		CallFlourFall( work, &vec ) ;
	}

	h = 0.0 ;
	if ( work->tmp_flag == 0 ) {
		ctrl->rot.vz = ctrl->turn.vz -= STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 1 ;
		}
	} else if ( work->tmp_flag == 1 ) {
		ctrl->rot.vz = ctrl->turn.vz += STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz+2048 ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 0 ;
		}
	} else if ( work->tmp_flag == 2 ) {
		ctrl->rot.vx = ctrl->turn.vx -= STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vx ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 3 ;
		}
	} else if ( work->tmp_flag == 3 ) {
		ctrl->rot.vx = ctrl->turn.vx += STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vx+2048 ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 2 ;
		}
	}

//printf("bomb [%d] rot vz[%d] vy[%f] h[%f]\n",work->count, ctrl->rot.vz, ctrl->mov.vy, h ) ;
	if ( work->count >= COUNT_VMODE(120) ) {
		ctrl->mov.vy = StartPos.vy ;
		ctrl->rot.vx = ctrl->turn.vx = 0 ;
		ctrl->rot.vz = ctrl->turn.vz = 0 ;
		work->tmp_flag = 0 ;
		work->think = THINK_IDLE ;
		work->count = 0 ;
		return ;
	}

	ctrl->mov.vy -= STEP_VMODE(10.0) ;
	work->count ++ ;
}

static void ThinkBreak( Work *work )
{
	FVECTOR vec ;
	CONTROL	*ctrl ;
	float	h ;
	
	ctrl = &work->control ;
	if ( DamageCheck( work ) ) {
		if ( work->damaged & TUB_DAM_DROP ) {
			work->think = THINK_DROP ;
			work->count = 0 ;
			return ;
		} else if ( work->damaged & TUB_DAM_BREAK ) {
			work->think = THINK_BREAK ;
			work->count = 0 ;
			return ;
		} else if ( work->damaged & TUB_DAM_BOMB ) {
			work->think = THINK_BOMB ;
			work->count = 0 ;
			return ;
		}
	}

	if ( work->count == 0 ) {
		ctrl->mov.vy = StartPos.vy + 600.0 ;
		if ( (work->dam_shelf == 1) || (work->dam_shelf == 3) ) {
			ctrl->rot.vz = ctrl->turn.vz = 256 ;
			work->tmp_flag = 0 ;
		} else {
			ctrl->rot.vx = ctrl->turn.vx = 256 ;
			work->tmp_flag = 2 ;
		}

		if ( work->dam_shelf < 4 ) {
			_sceVu0AddVector( &vec, &ShlefShift[work->dam_shelf], &ctrl->mov ) ;
			CallFlourFall( work, &vec ) ;
		}
	}

	h = 0.0 ;
	if ( work->tmp_flag == 0 ) {
		ctrl->rot.vz = ctrl->turn.vz -= STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 1 ;
		}
	} else if ( work->tmp_flag == 1 ) {
		ctrl->rot.vz = ctrl->turn.vz += STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz+2048 ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 0 ;
		}
	} else if ( work->tmp_flag == 2 ) {
		ctrl->rot.vx = ctrl->turn.vx -= STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vx ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 3 ;
		}
	} else if ( work->tmp_flag == 3 ) {
		ctrl->rot.vx = ctrl->turn.vx += STEP_VMODE(32) ;
		h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vx+2048 ) ;
		if ( h < StartPos.vy ) {
			work->tmp_flag = 2 ;
		}
	}

//printf("bomb [%d] rot vz[%d] vy[%f] h[%f]\n",work->count, ctrl->rot.vz, ctrl->mov.vy, h ) ;
	if ( work->count >= COUNT_VMODE(60) ) {
		ctrl->mov.vy = StartPos.vy ;
		ctrl->rot.vx = ctrl->turn.vx = 0 ;
		ctrl->rot.vz = ctrl->turn.vz = 0 ;
		work->tmp_flag = 0 ;
		work->think = THINK_IDLE ;
		work->count = 0 ;
		return ;
	}

	ctrl->mov.vy -= STEP_VMODE(10.0) ;
	work->count ++ ;
}


static void ThinkDropToHead( Work *work )
{
	FVECTOR vec ;
	CONTROL	*ctrl ;
	int	i, d ;

	ctrl = &work->control ;

	if ( work->count == 0 ) {
		work->tmp_flag = 0 ;
		for ( i=0; i<MAX_SHELF; i++) {
			if ( !(work->shelf[i] & GLL_SHELF_DESTROY) ) {
				work->tmp_flag = i ;
				break ;
			}
		}
		
		d = work->tmp_flag + 2 ;
		if ( d >= 4 ) d -= 4 ;
		_sceVu0AddVector( &vec, &ShlefShift[d], &ctrl->mov ) ;
		CallFlourFall( work, &vec ) ;
GM_SeSetMode( SD_A_TARAIKRO, &ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->count == COUNT_VMODE(60) ) {
GM_SeSetMode( SD_A_TARAIKAT, &ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->count < COUNT_VMODE(120) ) {
		if ( (work->count > COUNT_VMODE(60)) ) {
			if ( !(work->count % COUNT_VMODE(5)) ) {
				extern void  *NewGllSpreadFlour(FMATRIX *world ) ;
				FMATRIX	world ;

				world = DG_UnitMatrix ;
				vec.vx = ctrl->mov.vx + (float)KR_RandS( 1500 ) ;
				vec.vy = ctrl->mov.vy ;
				vec.vz = ctrl->mov.vz + (float)KR_RandS( 1500 ) ;
				DG_COPY_VEC( (FVECTOR *)world.m[3], &vec );
				NewGllSpreadFlour(&world) ;
			}
		}

		d = work->count / 4 ;
		d = STEP_VMODE(d) ;
		switch ( work->tmp_flag ) {
			case 1 :
			 ctrl->rot.vx = ctrl->turn.vz -= d ;
			 break ;
			case 2 :
			 ctrl->rot.vx = ctrl->turn.vx = d ;
			 break ;
			case 3 :
			 ctrl->rot.vx = ctrl->turn.vz = d ;
			 break ;
			default :
			 ctrl->rot.vx = ctrl->turn.vx -= d ;
			 break ;
		}
		ctrl->step.vy += STEP_VMODE(-0.2) ;
	} else {
		if (ctrl->mov.vy < work->head_top.vy ) { 
			GM_SeSetMode( SD_A_TARAIOTI, &ctrl->mov, GM_SEMODE_BOMB ) ;
			work->think = THINK_DROP_2 ;
			ctrl->rot.vx = ctrl->turn.vx = 0 ;
			work->count = 0 ;
			return ;
		}

		d = STEP_VMODE(32) ;
		switch ( work->tmp_flag ) {
			case 1 :
			 ctrl->rot.vx = ctrl->turn.vz -= d ;
			 break ;
			case 2 :
			 ctrl->rot.vx = ctrl->turn.vx = d ;
			 break ;
			case 3 :
			 ctrl->rot.vx = ctrl->turn.vz = d ;
			 break ;
			default :
			 ctrl->rot.vx = ctrl->turn.vx -= d ;
			 break ;
		}
		ctrl->step.vy += STEP_VMODE(-2) ;
	}
	work->count ++ ;
}

static void ThinkHeadToGround( Work *work )
{
	CONTROL	*ctrl ;

	ctrl = &work->control ;

	if ( work->count == 0 ) {
		SetPalabolaForce( &work->control.mov, &GroundPos[GROUND_NUM], PARABOLA_TIME, GRAVITATION, &work->control.step ) ;
		GLL_GAME_STATUS |=  GLL_GS_TUB_HEAD ;
	}

	if ( ctrl->grounded ) { 
		GM_SeSetMode( SD_A_TARAIKRO, &ctrl->mov, GM_SEMODE_BOMB ) ;
		work->think = THINK_GROUND ;
		ctrl->step.vx = 0.0 ;
		ctrl->step.vy = 0.0 ;
		ctrl->step.vz = 0.0 ;
		work->count = 0 ;
		return ;
	}

	ctrl->step.vy += STEP_VMODE(-4) ;
	work->count ++ ;
}

static void ThinkGround( Work *work )
{
	FVECTOR vec ;
	CONTROL	*ctrl ;
	float	h ;

	ctrl = &work->control ;

	if ( work->count == 0 ) {
		ctrl->mov.vy = GroundPos[GROUND_NUM].vy + 600.0 ;
//		ctrl->step.vy = 100.0 ;
		ctrl->rot.vz = ctrl->turn.vz = 256 ;
		work->tmp_flag = 0 ;
	}

	if ( work->count == COUNT_VMODE(60) ) {
		ctrl->mov.vy = GroundPos[GROUND_NUM].vy ;
		ctrl->rot.vx = ctrl->turn.vx = 0 ;
		ctrl->rot.vz = ctrl->turn.vz = 0 ;
		work->tmp_flag = 0 ;
	} 
	if ( work->count < COUNT_VMODE(60) ) {
		h = 0.0 ;
		if ( work->tmp_flag == 0 ) {
			ctrl->rot.vz = ctrl->turn.vz -= STEP_VMODE(32) ;
			h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz ) ;
			if ( h < GroundPos[GROUND_NUM].vy ) {
				work->tmp_flag = 1 ;
			}
		} else if ( work->tmp_flag == 1 ) {
			ctrl->rot.vz = ctrl->turn.vz += STEP_VMODE(32) ;
			h = ctrl->mov.vy + 2000.0 * _RsinF( ctrl->rot.vz+2048 ) ;
			if ( h < GroundPos[GROUND_NUM].vy ) {
				work->tmp_flag = 0 ;
			}
		}
		ctrl->mov.vy -= STEP_VMODE(10.0) ;
//		ctrl->step.vy -= 4.0 ;
	}

	work->count ++ ;
}

/*----------------------------------------------------------------*/
static	void	Think( Work *work )
{
	switch( work->think ) {
		case THINK_IDLE :
		 ThinkIdle( work ) ;
		 break ;
		case THINK_BREAK :
		 ThinkBreak( work ) ;
		 break ;
		case THINK_BOMB :
		 ThinkBomb( work ) ;
		 break ;
		case THINK_BULLET :
		 ThinkBullet( work ) ;
		 break ;
		case THINK_BOMB_TUB :
		 ThinkBombTub( work ) ;
		 break ;
		case THINK_BULLET_TUB :
		 ThinkBulletTub( work ) ;
		 break ;

		case THINK_DROP :
		 ThinkDropToHead( work ) ;
		 break ;
		case THINK_DROP_2 :
		 ThinkHeadToGround( work ) ;
		 break ;
		case THINK_GROUND :
		 ThinkGround( work ) ;
		 break ;
	}
}
static	void	HeadHeight( Work *work )
{
	FVECTOR	*pos ;
	
	pos = (FVECTOR *)&work->gll_body->objs->objs[12].world.m[3][0] ;
	work->head_top.vx = pos->vx ;
	work->head_top.vy = pos->vy + 1700.0 ;
	work->head_top.vz = pos->vz ;

//AN_Test_Eye2(&work->head_top,2);
}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int i ;

	if ( GLL_GAME_STATUS & GLL_GS_TUB_KILL ) {
//		work->body.objs->flag |= DG_FLAG_INVISIBLE ;
		if ( !(GLL_GAME_STATUS & GLL_GS_TUB_EFE_KILL) ) {
			extern void *NewSigBreakObj6(DG_OBJS *,float ,int ,int ) ;

			GV_SetActorChild( work, NewSigBreakObj6((work->body.objs),5000.0f ,30 ,15) ) ; 
			GLL_GAME_STATUS |= GLL_GS_TUB_EFE_KILL ;
			for ( i=0; i<MAX_SHELF; i++) {
				work->shelf[i] |= GLL_SHELF_TUB_KILL ;
			}
		}
		return ;
	}
	
	GM_ActControl( &work->control ) ;
	DG_PutObjs( work->body.objs );

	HeadHeight( work ) ;
	Think( work ) ;

	if ( work->fall_count > 0 ) work->fall_count-- ;
	if ( GLL_ACHOO_TIME > 0 ) GLL_ACHOO_TIME-- ;
	
	GM_MoveTarget( &work->deftrg, &work->control.mov ) ;
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeControl( &work->control ) ;
	GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/
static	void	TargetCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	
	work = (Work *)ptr ;

	if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;
			work->off_center = off->center ;
//printf("koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee [%lx] \n",off->weapon_type ) ;
		}
	}
}

static void	InitTarget( Work *work )
{
	TARGET		*trg  ;

	trg = &work->deftrg ;

	GM_SetTarget( trg, TARGET_CLASS, 1, ENEMY_SIDE,  &TargetSize, &TargetShift ) ;
	GM_SetTargetWeaponType( trg, 0 ) ;
	GM_SetPowerTarget( trg, &work->power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargetCallBack, work ) ;

	GM_MoveTarget( trg, &work->control.mov ) ;
//NewTargetView( trg, 200, 34, 184 ) ;
}

static void SetShelf( Work *work )
{
	extern void	*NewGllShelf( int *flag, FVECTOR *pos, SVECTOR *rot, int life ) ;
	CONTROL *ctrl ;
	FVECTOR	shelf_pos[MAX_SHELF] ;
	SVECTOR rot ;
	int		i ;

	ctrl = &work->control ;
	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_PutVector( ShlefShift, shelf_pos, 4 ) ;
	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 1024 ;

	for ( i=0; i<MAX_SHELF; i++) {
		NewGllShelf( &work->shelf[i], &shelf_pos[i], &rot, 100 ) ;
		rot.vy += 1024 ;
	}
}


static	int	GetResources( Work *work, OBJECT *gll_body, int map, FMATRIX *lights )
{
	extern void *NewGllDropShadow( OBJECT *body, CONTROL *control, FMATRIX *lights, int *flag ) ;
	CONTROL	*ctrl ;
	OBJECT	*body ;

	work->gll_body = gll_body ;

	ctrl = &work->control ;
	body = &work->body ;
	
	/* ctrl 初期化 */
	GM_InitControl( ctrl, 0, GM_CurrentStageMap ) ;
	GM_ConfigControlHazard( ctrl, 500, 1000, 1100 ) ;
	ctrl->mov = StartPos ;
//    GM_ConfigControlHzxHeight( ctrl, 10.0F, ctrl->mov.vy + 10.0F ) ;
	ctrl->height = 0.0 ;

    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
	ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK ; /* 壁チェックなし */
//	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;


	/* body 初期化 */
	GM_InitObject( body, BODY_NAME, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );

	InitTarget( work ) ;

	work->think = THINK_IDLE ;
	work->count = 0 ;
	work->lights = lights ;
	work->fall_count = 0 ;
	work->map = map ;

	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_PutObjs( work->body.objs );
	work->shadow = 1 ;

	/*棚セット*/
	SetShelf( work ) ;

	/* 影セット */
//	NewGllDropShadow( body, ctrl, work->lights, &work->shadow ) ;

	return 0 ;
}

void	*NewGllTub( OBJECT *gll_body, int map, FMATRIX *lights )
{
	Work		*work ;

	OPERATOR() ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, gll_body, map, lights ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

