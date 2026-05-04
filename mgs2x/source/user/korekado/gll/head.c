//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	head.c
	
	2002/7/31 Y.Korekado
	$Id: head.c,v 1.3 2002/11/23 12:42:27 Yoshizawa1 Exp $
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
#include	"head.h"
#include	"gll_def.h"

extern int BP_FRAMES_PER_SEC();

extern	void	*AN_HeadMark( FMATRIX *world, int mark ) ;
void *NewGllLight(
	FMATRIX *world,	
	int *flag,		
	int mode,
	int *map		
);
/*----------------------------------------------------------------*/
#define	BODY_NAME	(7063641)	/*gno_meca_head_mt*/
#define PRIO	0x40
#define TARGET_CLASS	(TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK)
#define LIT_TARGET_CLASS	(TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE)

#define PUSH_SPEED	STEP_VMODE(300.0)
#define PUSH_COUNT	COUNT_VMODE(90)

static FVECTOR	TargetSize = { 1000.0, 1300.0, 1000.0 } ;
static FVECTOR	TargetShift = { 0.0, 500.0, 0.0 } ;
static FVECTOR	LitTargetSize = { 100.0, 100.0, 100.0 } ;
static FVECTOR	LitTargetShift = { 0.0, 1100.0, 1300.0 } ;

FVECTOR	StartPos[3] = {
	{-9000.0, 0.0, -17000.0},
	{9000.0, 0.0, -22000.0},
	{0.0, 0.0, -15500.0},
} ;


static int TurnCount[3] = { 3,3,3 } ;
static int TurnWait[3] = { 60,60,30 } ;
static int TargetTurnWait[3] = { 180,120,90 } ;

typedef	struct	{
	float	min_x ;
	float	min_z ;
	float	max_x ;
	float	max_z ;
} Goal ;

#if 1
static float DeadLine = -55000.0 ;
#else
static float DeadLine = -35000.0 ;
#endif
#define MAX_GOAL	(2)
static Goal	GoalData[MAX_GOAL] = {
	{-6000.0, -25000.0, -3000.0, -22000.0},
	{3000.0, -25000.0, 6000.0, -22000.0},
} ;

static FVECTOR	GoalCenter[MAX_GOAL] = {
	{-4500.0, 0.0, -23500.0 },
	{4500.0, 0.0, -23500.0},
} ;

#define HEAD_R	(800.0)	/*頭の半径*/

static FVECTOR spot_shift = { 0.0, 1100.0, 1300.0 } ;
/*----------------------------------------------------------------*/
extern void *NewGllBlast3( FVECTOR *, int, int ,int ,int, int, int, int ) ;
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;
	OBJECT		body ;
	
	FVECTOR		scale ;
	FMATRIX		*gl_head ;
	
	TARGET		deftrg ;	/* 防御ターゲット */
	POWER_TARGET	power ;	/* 防御属性 */
	FVECTOR		off_center ;
	TARGET		littrg ;	/* ライトターゲット */

	int			think ;
	int			think2 ;
	int			count ;
	int			force_dir ;
	int			time ;
	float		speed ;
	int			tmp_flag ;
	int			turn_dir ;

	int			pl_dir ;
	int			route ;
	SVECTOR 	drop_rot ;
	int			*flag ;

	FMATRIX		spot_world ;
	int			sw_spot ;
	int			*spot ;
	FVECTOR		out_shift ;
	SVECTOR		out_rot ;
	int			turn_count ;
	FMATRIX 	*lights ;
	int			shadow ;
} Work ;

/*----------------------------------------------------------------*/
enum {
	THINK_HEAD, THINK_DROP, THINK_WAIT, THINK_PUSH, THINK_MINI_PUSH, 
	THINK_TURN, THINK_BEEM, THINK_DEATH, THINK_BODY, THINK_OUT,
	THINK_MINI_TURN, THINK_CHAFF_TURN, THINK_HALL_DEATH, THINK_TARGET_TURN
} ;
/*----------------------------------------------------------------*/
#if 0
static void	ScaleMatrix( FMATRIX *mat, FVECTOR *vec )
{
	mat->m[0][0] *= vec->vx ;
	mat->m[1][0] *= vec->vx ;
	mat->m[2][0] *= vec->vx ;

	mat->m[0][1] *= vec->vy ;
	mat->m[1][1] *= vec->vy ;
	mat->m[2][1] *= vec->vy ;

	mat->m[0][2] *= vec->vz ;
	mat->m[1][2] *= vec->vz ;
	mat->m[2][2] *= vec->vz ;

//printf("scalemat> [%f][%f][%f]\n",mat->m[0][0],mat->m[1][1],mat->m[2][2] );
}
#endif

static void LightBreak( Work *work )
{
	extern void *NewBreakLight( FVECTOR *pos, FVECTOR *dir, int where ) ;
	FVECTOR pos ;
	TARGET	*def ;

	def = &work->littrg ;
	_sceVu0ApplyMatrix( &pos, &def->world, &def->offset ) ;
	_sceVu0AddVector( &pos, &def->center, &pos ) ;
	NewBreakLight( &pos, &DG_ZeroVector, GM_CurrentStageMap ) ;
	*work->flag |= GLL_HEAD_LIT_BREAK ;
	work->littrg.class |= TARGET_SKIP ;
	GM_SeSetMode( SD_E_SHIELD01, &pos, GM_SEMODE_BOMB ) ;
}

static void	SetSpotWorld( Work *work ) {
	DG_SetPos( &work->body.objs->world ) ;
	DG_MovePos( &spot_shift ) ;
	DG_GetPos( &work->spot_world ) ;
}

#define PARABOLA_TIME	COUNT_VMODE(60)
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

static void CallPushEffect( FVECTOR *pos ,FVECTOR *force )
{
	extern void *NewSpark2(int n_packets, FMATRIX *matrix, float min_speed, 
		float speed_wide, float gravity,SVECTOR *rot,SVECTOR *rot_wide,
		FVECTOR *color,float length, int count) ;
    SVECTOR	rot_e ;
    FMATRIX	w ;
	SVECTOR   rot ;
	SVECTOR   rot_wide ;
	FVECTOR   color ;

	rot.vx = 768 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
//	rot_wide.vx = 512 ;
//	rot_wide.vy = 4096 ;
	rot_wide.vx = 16 ;
	rot_wide.vy = 16 ;
	rot_wide.vz = 0 ;
	color.vx = 255.0F ;
	color.vy = 128.0F ;
	color.vz = 128.0F ;
	color.vw = 50.0F ;

    _FVecToRotXY( force, &rot_e ) ;
    DG_SetPos2( pos, &rot_e ) ;
    DG_GetPos( &w ) ;
//    NewSpark( &w ) ;
	NewSpark2(6,&w,1.0F,320.0F,0.0F,&rot,&rot_wide,&color,1.0F,8) ;
}

static	int	DamageCheck( Work *work )
{
	TARGET		*trg  ;
	long64		weapon ;
	
	trg = &work->deftrg ;

	if ( TARGET_POWER & trg->damaged ) {
//printf("head damage: [%lx] \n",trg->weapon_type ) ;
		weapon = trg->weapon_type ;
		trg->weapon_type = 0 ;
		trg->damaged = 0 ;
		if ( weapon & (WP_BLAST) ) {
			GM_SeSetMode( SD_E_SHIELD01, &work->control.mov, GM_SEMODE_BOMB ) ;
			return 1 ;
		}
		if ( weapon & (WP_BULLET) ) {
			GM_SeSetMode( SD_W_RICOCH01, &work->control.mov, GM_SEMODE_BOMB ) ;
			CallSpark( &trg->hit , &trg->power->force ) ;
//return 2 ;
		}
	}
	return 0 ;
}

static FVECTOR	PushEffectShift = { 0.0, 0.0, 1000,0 } ;
static void PushEffect( Work *work )
{
	FVECTOR	pos, force, ef_force ;
	SVECTOR	rotate ;
	CONTROL	*ctrl ;
	int i ;

	ctrl = &work->control ;
	force.vy = -50.0 ;
	force.vz = 50.0 ;

	if ( work->tmp_flag ) {
		force.vx = 100.0 ;
	} else {
		force.vx = -100.0 ;
	}

	rotate.vx = 0 ;
	rotate.vy = 2048 ;
	rotate.vz = 0 ;

	pos = ctrl->mov ;
	pos.vy = 0.0 ;	/* 床 */
	DG_SetPos2( &pos, &ctrl->rot ) ;

	for ( i=0; i<2; i++ ) {
		DG_PutVector( &PushEffectShift, &pos, 1 );
	    DG_RotVector( &force, &ef_force, 1 ) ;
		CallPushEffect( &pos , &ef_force ) ;
		DG_RotatePos( &rotate );
	}
}

static int ReflectionDir( int wall_dir, int dir )
{
	int dir1, dir2, wd1, wd2 ;

	wd1 = wall_dir & 4095 ;
	dir1 = GV_DiffDirAbs( dir, wd1 ) ;
	wd2 = (wall_dir+2048) & 4095 ;
	dir2 = GV_DiffDirAbs( dir, wd2 ) ;

//printf( "ref : dir1 [%d] dir2[%d]\n", dir1,dir2) ;
	
	wall_dir = ( dir1 < dir2 ) ? wd1 : wd2 ;
	wall_dir &= 4095 ;

//printf( "ref : dir [%d] wall_dir[%d]\n", dir,wall_dir) ;
	dir1 = GV_DiffDirS( wall_dir, dir ) ;

//printf( "ref : wall_dir [%d] dir[%d]\n", wall_dir,dir1) ;
	dir2 = (wall_dir - dir1) & 4095 ;
	
	return  dir2 ;
}

static void SetReflectionForce( Work *work )
{
	int	wall_dir ;
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;
//printf("ctrl->n_touches[%d]\n",ctrl->n_touches);
	if ( ctrl->n_touches > 0 ) {
		GM_SeSetMode( SD_A_MG_KWALL, &ctrl->mov, GM_SEMODE_BOMB ) ;
		wall_dir = GV_VecDir2( &ctrl->vecs[ 0 ] ) ;
		wall_dir = (wall_dir + 1024) & 4095 ;
//printf("wall_dir[%d]  work->force_dir[%d] > \n",wall_dir,work->force_dir);
		work->force_dir = ReflectionDir( wall_dir, work->force_dir ) ;
//printf("work->force_dir[%d]\n",work->force_dir);
	}
}

static	int	SetForce( Work *work )
{
	FVECTOR		vec ;
	int			turn_dir, diff ;
	
	_sceVu0SubVector(  &vec, &work->control.mov, &work->off_center ) ;
	work->force_dir = _FVecDir2( &vec ) ;
	/* 爆発に指向性がないためプレイヤーの座標を中心にして回転方向を決める */
	_sceVu0SubVector(  &vec, &work->off_center, &GM_PlayerPosition ) ;
	turn_dir = _FVecDir2( &vec ) ;

//printf("set force force_dir [%d] tunr_dir[%d]\n",work->force_dir , turn_dir ) ;

	diff = GV_DiffDirS( turn_dir, work->force_dir ) ;
	return ( diff > 0 ) ? 1 : 0 ;
}

static void	 DeadLineCheck( Work *work )
{
	int i ;
	FVECTOR	*pos ;

	if ( (work->sw_spot == 1) || (work->sw_spot == 4) ) {
		work->sw_spot = ( GM_GameStatus & STATE_CHAFF ) ? 4 : 1 ;
		if ( *work->flag & GLL_HEAD_LIT_BREAK ) {
			AN_HeadMark( &(work->body.objs->world), RED_QE_GENORA );
			work->sw_spot = 0 ;
		}
	}

	pos = &work->control.mov ;
	for ( i=0; i<MAX_GOAL; i++ ){
		if ( pos->vx < (GoalData[i].min_x+HEAD_R) ) continue ;
		if ( pos->vz < (GoalData[i].min_z+HEAD_R) ) continue ;
		if ( pos->vx > (GoalData[i].max_x-HEAD_R) ) continue ;
		if ( pos->vz > (GoalData[i].max_z-HEAD_R) ) continue ;

printf("GOOOOOOOOAl [%f][%f]\n",pos->vx, pos->vz ) ;

		GM_SeSetMode( SD_A_MG_KGOAL, &work->control.mov, GM_SEMODE_BOMB ) ;
		*work->flag |= GLL_HEAD_TRG_SKIP ;
		work->deftrg.weapon_type = 0 ;
		work->deftrg.damaged = 0 ;
		work->think = THINK_HALL_DEATH ;
		work->count = 0 ;
		work->tmp_flag = i ;	/* どの穴か */
		return ;
	}

	if ( work->control.mov.vz < DeadLine ) {
		GM_SeSetMode( SD_A_MG_KGOAL, &work->control.mov, GM_SEMODE_BOMB ) ;
		*work->flag |= GLL_HEAD_TRG_SKIP ;
		work->deftrg.weapon_type = 0 ;
		work->deftrg.damaged = 0 ;
		work->think = THINK_DEATH ;
		work->count = 0 ;

	}
}
/*----------------------------------------------------------------*/
static	void	ThinkHead( Work *work )
{
	if ( *work->flag & GLL_HEAD_OFF ) {
		*work->flag &= ~GLL_HEAD_JOIN ;
//		work->sw_spot =  ( GLL_GAME_STATUS & GLL_GS_SPOT_BREAK ) ? 0 : 1 ;
		work->sw_spot =  ( *work->flag & GLL_HEAD_LIT_BREAK ) ? 0 : 1 ;
		work->think = THINK_DROP ;
		work->count = 0 ;
		return ;
	}

	work->count ++ ;
}

static void DropRot( SVECTOR *rot, int dir )
{
	float x,z ;
	
//	x = 128.0 * _RcosF( dir ) ;
//	z = 128.0 * _RsinF( dir ) ;
	x = 64.0 * _RcosF( dir ) ;
	z = 64.0 * _RsinF( dir ) ;

printf("x[%f] z[%f]\n",_RsinF( dir ),_RcosF( dir ));
	
	rot->vx = (short)x ;
	rot->vz = -(short)z ;
	rot->vy = 0 ;

printf("drop rot dir[%d] vx[%d] vz[%d]\n",dir, rot->vx,rot->vz);
}

static void SetDropForce( Work *work ){
	FVECTOR	vec ;
	int dis ;

	_sceVu0SubVector( &vec, &StartPos[work->route], &work->control.mov ) ;
	dis = _FVecLen2( &vec ) ;
	work->force_dir = _FVecDir2( &vec ) ;
	work->speed = (float)(dis / COUNT_VMODE(90)) ;

	DropRot ( &work->drop_rot, work->force_dir ) ;

printf("work->force_dir[%d] work->speed[%f]\n",work->force_dir,work->speed);
}

static	void	ThinkDrop( Work *work )
{
	CONTROL *ctrl ;

	ctrl = &work->control ;

	if ( work->count == 0 ) {
		work->think2 = 0 ;
		work->out_shift = DG_ZeroVector ;
		work->out_rot = DG_ZeroSVector ;
		*work->flag |= GLL_HEAD_LEFT_BODY ;
//work->route = 2 ;
	}

	if ( ctrl->grounded & 1 ) {
		extern void *NewShakeCameraGLL( int chanl, int intense, int time, FVECTOR *pos ) ;
		NewShakeCameraGLL( 0, 512, 10, &ctrl->mov );
		GM_SeSetMode( SD_A_MG_KFALL, &ctrl->mov, GM_SEMODE_BOMB ) ;

		GLL_GAME_STATUS |= GLL_GS_HEAD_GROUNDED ;

		*work->flag &= ~GLL_HEAD_TRG_SKIP ;
		work->think = THINK_WAIT ;
		work->count = 0 ;
		ctrl->turn.vx = ctrl->rot.vx = 0 ;
		ctrl->turn.vz = ctrl->rot.vz = 0 ;
		ctrl->step.vx = 0.0 ;
		ctrl->step.vy = 0.0 ;
		ctrl->step.vz = 0.0 ;
		return ;
	}

	switch( work->think2 ) {
		case 0 :	/* ねじ回転 */
		 work->out_shift.vy += 10.0 ;
		 work->out_rot.vy += 256 ;
		 work->out_rot.vy &= 4095 ;
		 if ( !(work->count%COUNT_VMODE(8)) ) {
			GM_SeSetMode( SD_A_MG_KNEJI, &ctrl->mov, GM_SEMODE_BOMB ) ;
		 }
		 if ( work->count >= COUNT_VMODE(64) ) {
			GM_SeSetMode( SD_A_MG_KTORE, &ctrl->mov, GM_SEMODE_BOMB ) ;
			ctrl->turn.vx = ctrl->rot.vx = MatToXRot( &(work->body.objs->world) ) ;
			ctrl->turn.vy = ctrl->rot.vy = MatToYRot( &(work->body.objs->world) ) ;
			SetDropForce( work ) ;
			ctrl->step.vy = STEP_VMODE(90.0) ;
			*work->flag &= ~GLL_HEAD_LEFT_BODY ;
			work->think2++ ;
		 }
		 break ;
		case 1 :/* 外れて体にぶつかるまで */
		 {
			int	dev ;
			float speed ;

			dev = work->count - COUNT_VMODE(60) ;

			speed = (work->route==2) ? 1.0 : STEP_VMODE(100.0) ;
			ctrl->step.vx = (speed) * _RsinF( (int)work->force_dir ) ;
			ctrl->step.vz = (speed) * _RcosF( (int)work->force_dir ) ;
			ctrl->step.vy -= GRAVITATION ;

			ctrl->turn.vx = ctrl->rot.vx += work->drop_rot.vx*dev/BP_FRAMES_PER_SEC() ;
			ctrl->turn.vz = ctrl->rot.vz += work->drop_rot.vz*dev/BP_FRAMES_PER_SEC() ;
			ctrl->turn.vy = ctrl->rot.vy += 256 ;
		 }

		 if ( work->count >= COUNT_VMODE(120) ) {
			SetPalabolaForce( &work->control.mov, &StartPos[work->route], PARABOLA_TIME, GRAVITATION, &work->control.step ) ;
			if ( work->route == 0 ) {
				if ( work->sw_spot == 1 ) {
					LightBreak( work ) ;
					work->sw_spot = 0 ;
				}
			}
			work->think2++ ;
		 }
		 break ;
		case 2 :/* 地面に落下するまで放物線運動 */
		 ctrl->step.vy -= (float)GRAVITATION ;
		 ctrl->turn.vx = ctrl->rot.vx += work->drop_rot.vx ;
		 ctrl->turn.vz = ctrl->rot.vz += work->drop_rot.vz ;
		 ctrl->turn.vy = ctrl->rot.vy += 256 ;
		 break ;
	}
//printf("drop count [%d] think2[%d]\n",work->count,work->think2);
	work->count ++ ;
}


static	void	ThinkWait( Work *work )
{
	int	dam, wait_time ;

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		work->count = 0 ;
		return ;
	}

	wait_time = ( work->sw_spot == 0 ) ? TurnWait[work->route] : TargetTurnWait[work->route] ;
	wait_time = COUNT_VMODE(wait_time) ;
	if ( work->count > wait_time ) {
printf("work->route[%d] work->turn_count[%d]\n",work->route,work->turn_count);
		if ( work->turn_count >= TurnCount[work->route] ) {
			work->think = THINK_BEEM ;
		} else {
			if ( GM_GameStatus & STATE_CHAFF ) {
				work->think = THINK_CHAFF_TURN ;
			} else {
				if ( work->sw_spot == 0 ) {
					work->think = THINK_MINI_TURN ;
				} else {
					if ( (TurnCount[work->route] - work->turn_count) == 1 ) {
						work->think = THINK_TURN ;
					} else {
						work->think = THINK_TARGET_TURN ;
					}
				}
			}
		}
		work->count = 0 ;
		return ;
	}

	work->count ++ ;
}

static	void	ThinkMiniTurn( Work *work )
{
	int	dam ;
	CONTROL	*ctrl ;

	ctrl = &work->control ;

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		work->count = 0 ;
		return ;
	}

	if ( work->count == 0 ) {
		GM_SeSetMode( SD_E_MGNKINUZ, &ctrl->mov, GM_SEMODE_BOMB ) ;
		work->turn_count ++ ;
	}
	if ( work->count > COUNT_VMODE(30) ) {
		work->think = THINK_WAIT ;
		work->count = 0 ;
		return ;
	}

	ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(15) ;

	work->count ++ ;
}

static	void	ThinkChaffTurn( Work *work )
{
	int	dam ;
	CONTROL	*ctrl ;

	ctrl = &work->control ;

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		work->count = 0 ;
		return ;
	}

	if ( work->count == 0 ) {
		GM_SeSetMode( SD_E_MGNKINUZ, &ctrl->mov, GM_SEMODE_BOMB ) ;
		work->tmp_flag = 0 ;
		work->turn_count ++ ;
	}
	if ( work->count > COUNT_VMODE(90) ) {
		work->think = THINK_WAIT ;
		work->count = 0 ;
		return ;
	}
	if ( !(work->count%(COUNT_VMODE(15))) ) {
		work->tmp_flag = KR_RandU(2) ;
		printf("work->tmp_flag = %d\n",work->tmp_flag );
	}

	if ( work->tmp_flag ) {
		ctrl->rot.vy = ctrl->turn.vy -= STEP_VMODE(60) ;
	} else {
		ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(60) ;
	}

	work->count ++ ;
}

static	void	ThinkMiniPush( Work *work )
{
	int	dam ;
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;

	if ( work->count == 0 ) {
		work->tmp_flag = SetForce( work ) ;
	}

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		ctrl->step.vx = STEP_VMODE(80.0) * _RsinF( (int)work->force_dir ) ;
		ctrl->step.vz = STEP_VMODE(80.0) * _RcosF( (int)work->force_dir ) ;

		ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(32) ;
		work->count = 0 ;
		return ;
	}

	if ( work->count > COUNT_VMODE(1)) {
		ctrl->step.vx = 0.0 ;
		ctrl->step.vz = 0.0 ;
//		work->think = THINK_TURN ;
		work->think = THINK_WAIT ;
		work->count = 0 ;
		return ;
	}

	ctrl->step.vx = STEP_VMODE(80.0) * _RsinF( (int)work->force_dir ) ;
	ctrl->step.vz = STEP_VMODE(80.0) * _RcosF( (int)work->force_dir ) ;

	if ( work->tmp_flag ) {
		ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(32) ;
	} else {
		ctrl->rot.vy = ctrl->turn.vy -= STEP_VMODE(32) ;
	}

	work->count ++ ;
}

static	void	ThinkPush( Work *work )
{
	int	dam ;
	CONTROL	*ctrl ;

	ctrl = &work->control ;

	if ( work->count == 0 ) {
		work->tmp_flag = SetForce( work ) ;
	}

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		ctrl->step.vx = STEP_VMODE(200.0) * _RsinF( (int)work->force_dir ) ;
		ctrl->step.vz = STEP_VMODE(200.0) * _RcosF( (int)work->force_dir ) ;

		ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(256) ;
		work->count = 0 ;
		return ;
	}

	PushEffect( work ) ;

	if ( work->count > PUSH_COUNT ) {
		ctrl->step.vx = 0.0 ;
		ctrl->step.vz = 0.0 ;
//		work->think = THINK_TURN ;
		work->think = THINK_WAIT ;
		work->turn_count = 0 ;
		work->count = 0 ;
		return ;
	}

	if ( !(work->count%COUNT_VMODE(16)) ) {
		GM_SeSetMode( SD_A_MG_KSURE, &ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	SetReflectionForce( work ) ;

	ctrl->step.vx = PUSH_SPEED * _RsinF( (int)work->force_dir ) ;
	ctrl->step.vz = PUSH_SPEED * _RcosF( (int)work->force_dir ) ;

	if ( work->tmp_flag ) {
		ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(256) ;
	} else {
		ctrl->rot.vy = ctrl->turn.vy -= STEP_VMODE(256) ;
	}

	work->count ++ ;
}

static	void	ThinkTurn( Work *work )
{
	int	dam ;
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		work->count = 0 ;
		return ;
	}

	if ( work->count == 0 ) {
		GM_SeSetMode( SD_E_MGNKINUZ, &ctrl->mov, GM_SEMODE_BOMB ) ;
		work->turn_count ++ ;
	}

	if ( (GV_DiffDirAbs( ctrl->rot.vy, work->pl_dir ) <= STEP_VMODE(16) ) ||
		work->count > COUNT_VMODE(120) ) {
		ctrl->turn.vy = ctrl->rot.vy = work->pl_dir ;
		work->think = THINK_WAIT ;
		work->count = 0 ;
		return ;
	}

	ctrl->turn.vy = ctrl->rot.vy = GV_NearSpeedP( ctrl->rot.vy, work->pl_dir, STEP_VMODE(16) ) ;

	work->count ++ ;
}

static	void	ThinkTargetTurn( Work *work )
{
	int	dam ;
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		work->count = 0 ;
		return ;
	}

	if ( work->count == 0 ) {
		GM_SeSetMode( SD_E_MGNKINUZ, &ctrl->mov, GM_SEMODE_BOMB ) ;
		work->turn_count ++ ;
		work->turn_dir = ( GV_DiffDirS( ctrl->rot.vy, work->pl_dir ) > 0 ) ? 1 : -1 ;
	}

	if ( work->count > COUNT_VMODE(30) ) {
		work->think = THINK_WAIT ;
		work->count = 0 ;
		return ;
	}

	if ( (TurnCount[work->route] - work->turn_count) > 1 ) {
		if ( GV_DiffDirAbs( ctrl->rot.vy, work->pl_dir ) < STEP_VMODE(256) ) {
			ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(8*work->turn_dir) ;
		} else {
			ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(30*work->turn_dir) ;
		}
	} else {
		ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(15*work->turn_dir) ;
	}

	work->count ++ ;
}

static	void	ThinkBeem( Work *work )
{
	CONTROL	*ctrl ;
	int		dam ;
	
	ctrl = &work->control ;

	if ( work->count == 0 ) {
		extern void *NewEyeBeem(FMATRIX *world, FVECTOR *trg, int mode) ;
#if 0
		FMATRIX	mat ;
		DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
		DG_GetPos( &mat ) ;
		NewEyeBeem(&mat, &GM_PlayerPosition, 1) ;
#else
		NewEyeBeem(&(work->body.objs->world), &GM_PlayerPosition, 1 ) ;
#endif
		work->turn_count = 0 ;
		GM_SeSetMode( SD_E_MGNBRESS, &ctrl->mov, GM_SEMODE_BOMB ) ;
	}

	if ( (dam =DamageCheck( work )) ) {
		if ( dam == 1 ) {
			work->think = THINK_PUSH ;
		} else {
			work->think = THINK_MINI_PUSH ;
		}
		work->count = 0 ;
		return ;
	}

	if ( work->count > COUNT_VMODE(30) ) {
		work->think = THINK_WAIT ;
		work->count = 0 ;
		return ;
	}

	work->count ++ ;
}

static	void	ThinkDeath( Work *work )
{
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;

	GLL_GAME_STATUS |= GLL_GS_HEAD_REMOVE ;
	if ( work->count == 0 ) {
		ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;
		ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK ;
		ctrl->skip_flag |= CTRL_SKIP_ONLINE_CHECK ;
	}

	if ( work->count > COUNT_VMODE(120) ) {
		NewGllBlast3( &ctrl->mov, BOTH_SIDE, 2000, 3000, 0, FNT_BLAST, WP_C4Bomb, BLAST_TYPE_ONETARG|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER ) ;
		work->think = THINK_BODY ;
		ctrl->step.vy = 0 ;
		*work->flag |= GLL_HEAD_IN_BODY ;
		*work->flag |= GLL_HEAD_BREAK ;
		GLL_GAME_STATUS |= GLL_GS_HEAD_BREAK ;
		ctrl->skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		ctrl->skip_flag &= ~CTRL_SKIP_NEAR_CHECK ;
		ctrl->skip_flag &= ~CTRL_SKIP_ONLINE_CHECK ;
		work->sw_spot = 0 ;
		work->count = 0 ;
		work->route++ ;
		return ;
	}

	ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(256) ;
	ctrl->step.vy -= STEP_VMODE(8) ;
	work->count ++ ;
}

static	void	ThinkHallDeath( Work *work )
{
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;

	GLL_GAME_STATUS |= GLL_GS_HEAD_REMOVE ;
	if ( work->count == 0 ) {
		ctrl->skip_flag |= CTRL_SKIP_FLR_CHECK ;
		ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK ;
		ctrl->skip_flag |= CTRL_SKIP_ONLINE_CHECK ;
	}

	if ( work->count > COUNT_VMODE(120) ) {
		NewGllBlast3( &ctrl->mov, BOTH_SIDE, 2000, 3000, 0, FNT_BLAST, WP_C4Bomb, BLAST_TYPE_ONETARG|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER ) ;
		work->think = THINK_BODY ;
		ctrl->step.vy = 0 ;
		*work->flag |= GLL_HEAD_IN_BODY ;
		*work->flag |= GLL_HEAD_BREAK ;
		GLL_GAME_STATUS |= GLL_GS_HEAD_BREAK ;
		ctrl->skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
		ctrl->skip_flag &= ~CTRL_SKIP_NEAR_CHECK ;
		ctrl->skip_flag &= ~CTRL_SKIP_ONLINE_CHECK ;
		work->sw_spot = 0 ;
		work->count = 0 ;
		work->route++ ;
		return ;
	}

	{
		float dx, dz ;
		int  n ;

		n = work->count%4 ;
		
		dx = dz = 0.0 ;
		if( n==1 ) {
			dx = 100.0 * _RsinF( (int)work->force_dir ) ;
			dz = 100.0 * _RcosF( (int)work->force_dir ) ;
		} else if( n==3 ) {
			dx = -100.0 * _RsinF( (int)work->force_dir ) ;
			dz = -100.0 * _RcosF( (int)work->force_dir ) ;
		}
		ctrl->mov.vx = dx + GoalCenter[work->tmp_flag].vx ;
		ctrl->mov.vz = dz + GoalCenter[work->tmp_flag].vz ;
	}

	ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(256) ;
	ctrl->step.vy -= STEP_VMODE(4) ;
	work->count ++ ;
}

static	void	ThinkBody( Work *work )
{
	
	if ( work->count == 0 ) {
		*work->flag &= ~GLL_HEAD_BREAK ;
	}

//	if ( work->count > 120 ) {
	if ( *work->flag & GLL_HEAD_NEXT ) {
		work->think = THINK_OUT ;
		work->count = 0 ;
		*work->flag &= ~GLL_HEAD_IN_BODY ;
		*work->flag &= ~GLL_HEAD_NEXT ;
		*work->flag &= ~GLL_HEAD_LIT_BREAK ;
		*work->flag &= ~GLL_HEAD_NO_LIT ;
		work->littrg.class &= ~TARGET_SKIP ;

		return ;
	}
	work->count ++ ;
}

static	void	ThinkOut( Work *work )
{
	if ( work->count == 0 ) {
		*work->flag |= GLL_HEAD_OUT_BODY ;
		work->out_shift.vx = 0.0 ;
		work->out_shift.vy = STEP_VMODE(-128.0 * 40.0) ;
		work->out_shift.vz = STEP_VMODE(-128.0 * 20.0) ;
		work->out_rot = DG_ZeroSVector ;
	}
	
	if ( work->count == COUNT_VMODE(128) ) {
		*work->flag |= GLL_HEAD_REGENE ;
	}
	if ( work->count > COUNT_VMODE(128) ) {
		*work->flag = GLL_HEAD_JOIN|GLL_HEAD_TRG_SKIP ;	/* 初期状態 */
		work->think = THINK_HEAD ;
		work->count = 0 ;
		return ;
	}

	work->out_shift.vy += STEP_VMODE(40.0) ;
	work->out_shift.vz += STEP_VMODE(20.0) ;

	work->out_rot.vy += STEP_VMODE(128) ;
	work->out_rot.vy &= 4095 ;

	work->count ++ ;
}

/*----------------------------------------------------------------*/
static	void	Think( Work *work )
{
	switch( work->think ) {
		case THINK_HEAD :
		 ThinkHead( work ) ;
		 break ;
		case THINK_DROP :
		 ThinkDrop( work ) ;
		 break ;

		case THINK_WAIT :
		 ThinkWait( work ) ;
		 DeadLineCheck( work ) ;
		 break ;
		case THINK_MINI_TURN :
		 ThinkMiniTurn( work ) ;
		 DeadLineCheck( work ) ;
		 break ;
		case THINK_CHAFF_TURN :
		 ThinkChaffTurn( work ) ;
		 DeadLineCheck( work ) ;
		 break ;
		case THINK_PUSH :
		 ThinkPush( work ) ;
		 DeadLineCheck( work ) ;
		 break ;
		case THINK_MINI_PUSH :
		 ThinkMiniPush( work ) ;
		 DeadLineCheck( work ) ;
		 break ;
		case THINK_TURN :
		 ThinkTurn( work ) ;
		 DeadLineCheck( work ) ;
		 break ;
		case THINK_TARGET_TURN :
		 ThinkTargetTurn( work ) ;
		 DeadLineCheck( work ) ;
		 break ;
		case THINK_BEEM :
		 ThinkBeem( work ) ;
		 DeadLineCheck( work ) ;
		 break ;

		case THINK_DEATH :
		 ThinkDeath( work ) ;
		 break ;
		case THINK_HALL_DEATH :
		 ThinkHallDeath( work ) ;
		 break ;
		case THINK_BODY :
		 ThinkBody( work ) ;
		 break ;
		case THINK_OUT :
		 ThinkOut( work ) ;
		 break ;
	}
}
static	void	Notice( Work *work )
{
	FVECTOR vec ;

	_sceVu0SubVector(  &vec, &GM_PlayerPosition, &work->control.mov ) ;
	work->pl_dir = _FVecDir2( &vec ) ;
}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	CONTROL	*ctrl ;
	FMATRIX m ;

	GLL_GAME_STATUS &= ~GLL_GS_HEAD_GROUNDED ;
	GLL_GAME_STATUS &= ~GLL_GS_HEAD_REMOVE ;
	GLL_GAME_STATUS &= ~GLL_GS_HEAD_BREAK ;
	
	ctrl = &work->control ;
	if( *work->flag & GLL_HEAD_JOIN ) {
		DG_SetPos( work->gl_head ) ;
		DG_GetPos( &m ) ;
		ctrl->mov.vx =m.m[3][0] ;
		ctrl->mov.vy =m.m[3][1] ;
		ctrl->mov.vz =m.m[3][2] ;
	} else if( *work->flag & (GLL_HEAD_OUT_BODY|GLL_HEAD_LEFT_BODY) ) {
		DG_SetPos( work->gl_head ) ;
		DG_MovePos( &work->out_shift ) ;
 		DG_RotatePos( &work->out_rot ) ;
		DG_GetPos( &m ) ;
		ctrl->mov.vx =m.m[3][0] ;
		ctrl->mov.vy =m.m[3][1] ;
		ctrl->mov.vz =m.m[3][2] ;
	} else {
		GM_ActControl( ctrl ) ;
	}
	DG_PutObjs( work->body.objs );

	Notice( work ) ;
	Think( work ) ;

//printf("gll work flag[%x]\n",*work->flag);
	if( *work->flag & (GLL_HEAD_JOIN|GLL_HEAD_IN_BODY) ) {
		work->body.objs->flag |= DG_FLAG_INVISIBLE ;
	} else {
		work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
	}
	if( *work->flag & GLL_HEAD_TRG_SKIP ) {
		work->deftrg.class |= TARGET_SKIP ;
	} else {
		work->deftrg.class &= ~TARGET_SKIP ;
	}

	GM_MoveTarget( &work->deftrg, &work->control.mov ) ;
    GM_MoveTarget2Map( &work->littrg, &work->body.objs->world, ctrl->map ) ;
	SetSpotWorld( work ) ;
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeTarget( &work->deftrg ) ;
    GM_FreeTarget( &work->littrg ) ;
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

static	void	LitTargetCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;

	work = (Work *)ptr ;
	if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if ( off->weapon_type & WP_BULLET ) {
				LightBreak( work ) ;
			}
		}
//printf("1koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee [%lx] \n",off->weapon_type ) ;
		def->damaged = 0 ;
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

	trg = &work->littrg ;

	GM_SetTarget( trg, LIT_TARGET_CLASS, 1, ENEMY_SIDE,  &LitTargetSize, &LitTargetShift ) ;
	GM_SetTargetWeaponType( trg, 0 ) ;
	GM_SetPowerTarget( trg, &work->power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, LitTargetCallBack, work ) ;

	GM_MoveTarget( trg, &work->control.mov ) ;
//NewTargetView( trg, 200, 34, 184 ) ;
}

static	int	GetResources( Work *work, FMATRIX *gl_head, int *flag, int *spot, FMATRIX *lights )
{
	extern void *NewGllDropShadow( OBJECT *body, CONTROL *control, FMATRIX *lights, int *flag ) ;
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->body ;
	
	/* ctrl 初期化 */
	GM_InitControl( ctrl, 0, GM_CurrentStageMap ) ;
	GM_ConfigControlHazard( ctrl, 500, 1000, 1100 ) ;
	ctrl->mov = StartPos[0] ;
//    GM_ConfigControlHzxHeight( ctrl, 10.0F, ctrl->mov.vy + 10.0F ) ;
	ctrl->height = 1000.0 ;

    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
//	ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK ; /* 壁チェックなし */
//	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;


	/* body 初期化 */
	GM_InitObject( body, BODY_NAME, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );

	InitTarget( work ) ;

	work->think = THINK_HEAD ;
	work->count = 0 ;
	work->gl_head = gl_head ;
	work->route = 0 ;
	work->flag = flag ;
	work->spot = spot ;
	*work->flag = GLL_HEAD_JOIN|GLL_HEAD_TRG_SKIP ;	/* 初期状態 */

	DG_SetPos( work->gl_head ) ;
	DG_PutObjs( work->body.objs );
	ctrl->mov.vx = work->gl_head->m[3][0] ;
	ctrl->mov.vy = work->gl_head->m[3][1] ;
	ctrl->mov.vz = work->gl_head->m[3][2] ;

	work->sw_spot = 0 ;
	work->turn_count = 0 ;
	work->time = 0 ;
	work->lights = lights ;
	work->shadow = 1 ;

	SetSpotWorld( work ) ;
	NewGllLight( &work->spot_world, &work->sw_spot, 1, &work->control.hzx_id ) ;
	NewGllDropShadow( &work->body, &work->control, work->lights, &work->shadow ) ;

	return 0 ;
}

void	*NewGllHead( FMATRIX *gl_head, int *flag, int *spot, FMATRIX *lights )
{
	Work		*work ;

	OPERATOR() ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, gl_head, flag, spot, lights ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

