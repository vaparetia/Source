//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	missile.c
	
	2002/7/16 Y.Korekado
	$Id: missile.c,v 1.1.1.3 2002/11/19 11:44:14 Yoshizawa1 Exp $
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
#include	"debugmenu.h"

#include	"missile.h"
#include	"gll_def.h"

/*----------------------------------------------------------------*/
#define	BODY_NAME	(11398892)
#define	PARA_NAME	(2942725)
#define PRIO	0x40
#define EYE_LENGTH	(4000)
//#define EYE_RANGE	(256)
#define EYE_RANGE	(512)
#define EYE_X_RANGE	(256)
#define SIGHT_OUT	(0)
#define SIGHT_IN	(1)
#define DMG_MISSILE	(30)
#define DEF_LIFE	(3)
#define TARGET_CLASS	(TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK)
#define PARABOLA_TIME	COUNT_VMODE(90)
#define JUMP_TIME		COUNT_VMODE(30)
#define GRAVITATION		STEP_VMODE(4)
#define GRAVI_JUMP		STEP_VMODE(8)


static FVECTOR	Force = { 0.0, 0.0, 250.0 } ;
static FVECTOR	TargetSize = { 200.0, 250.0, 200.0 } ;
extern void *NewGllBlast3( FVECTOR *, int, int ,int ,int, int, int, int ) ;
extern void *AN_HeadMark( FMATRIX *world, int flag );
extern int GLL_GAME_STATUS ;
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;
	OBJECT		body ;
	OBJECT		para ;
	
	NAVIGATE 	navi ;
	NAVITARGET	n_trg ;
	
	int			trg_point ;
	float		speed ;
	int			dir ;
	int			think ;
	int			count ;
	int			id ;

	int			c_route ;
	int			eye_flag ;
	int			pl_dir ;
	FVECTOR		force ;
	SVECTOR		force_rot ;
	int			thk_status ;
	FVECTOR		old_pos ;
	FVECTOR		para_scale ;

    TARGET		off ;
    int			trg_flag ;
	TARGET		deftrg ;	/* 防御ターゲット */
	POWER_TARGET	power ;	/* 防御属性 */
    int			life ;
    HOMING_TRG 	homing ;

	FMATRIX		mark_world ;
	int			piku_time ;
	int			head_jump_time ;
	int			sw_spot ;
} Work ;

static int	MissileID ;
static FVECTOR MarkShift = { 0.0, 150.0, 0.0 } ;
/*----------------------------------------------------------------*/
#define	THK_STATUS_FLY		0x00000001
#define	THK_STATUS_ATTACK	0x00000002
#define	THK_STATUS_BOMB		0x00000004
#define	THK_STATUS_DESTROY	0x00000008
#define	THK_STATUS_START	0x00000010
#define	THK_STATUS_MOVE		0x00000020

/*----------------------------------------------------------------*/
enum {
	THINK_START, THINK_JUMP, THINK_MOVE,THINK_READY,THINK_ATTACK,
	THINK_PARA, THINK_ALL_READY, THINK_BREAK, THINK_HANABI
} ;
/*----------------------------------------------------------------*/
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
static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;

    work = ( Work * )ptr ;
    work->trg_flag |= 1 ;
    DG_COPY_VEC( &work->control.mov, &off->hit ) ;
}
static	void	SetTarget( Work *work )
{
    TARGET	*t ;

    t = &work->off ;
    GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN |
				 TARGET_POWER | TARGET_THROUGH, GM_CurrentStageMap, PLAYER_SIDE,
				 &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_WEAPONCORE ) ;
    GM_SetTargetName( t, WP_Rgb6 ) ;
    GM_SetTargetCallBack( t, Hit, work ) ;

//NewTargetView( t, 200, 34, 184 ) ;
    work->trg_flag = 0 ;
}

static void PosToMatrix( FMATRIX *w, SVECTOR *rot, FVECTOR *from, FVECTOR *to )
{
	extern void	_FVecToRotXY( FVECTOR *vec, SVECTOR *rot ) ;
	FVECTOR vec ;

	_sceVu0SubVector(  &vec, to, from ) ;
	_FVecToRotXY( &vec, rot ) ;
//printf(" vec vx[%f] vy[%f] vz[%f]\n",vec.vx,vec.vy,vec.vz ) ;
//printf(" rotvx[%d] vy[%d] vz[%d]\n",rot.vx,rot.vy,rot.vz ) ;
	rot->vx -= 1028 ;
	DG_SetPos2( from, rot ) ;
	DG_GetPos( w ) ;
}

static int RotPlayerX( FVECTOR *pos )
{
	FVECTOR vec ;
	
	_sceVu0SubVector(  &vec, &GM_PlayerPosition, pos ) ;
	return _FVecDirX( &vec ) ;
}

static void SetForce( Work *work )
{
	FMATRIX w ;
	FVECTOR	f ;
	
	PosToMatrix( &w, &work->force_rot, &work->control.mov, &GM_PlayerPosition ) ;

	DG_SetPos( &w );
	DG_PutVector( &Force, &f, 1 );

	_sceVu0SubVector(  &work->force, &f, &work->control.mov ) ;
}

static	int	EyeCheck( Work *work )
{
	CONTROL	*ctrl ;
	FVECTOR	vec ;
	int dis, dir, diff ;

	ctrl = &work->control ;

	_sceVu0SubVector(  &vec, &GM_PlayerPosition, &ctrl->mov ) ;
	dis = _FVecLen3( &vec ) ;
	work->pl_dir = dir = _FVecDir2( &vec ) ;

	if ( dis > EYE_LENGTH ) return 0 ;
	if ( _DiffDirAbs( ctrl->rot.vy, dir ) > EYE_RANGE ) return 0 ;

	dir = _FVecDirX( &vec ) - 1024 ;
	diff = GV_DiffDirS( ctrl->rot.vx, dir ) ;
	if ( diff > EYE_X_RANGE || diff < - EYE_X_RANGE ) return 0 ;

	if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &GM_PlayerPosition, &ctrl->mov,
		HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES | HZX_FLOOR_IK ) ) {
		return 0 ;
	}

	return 1 ;
}

/*----------------------------------------------------------------*/
static	void	NextTrgPoint( Work *work )
{
	work->trg_point ++ ;
	if ( work->trg_point >= MoveRoute[work->c_route].point_num ) work->trg_point = 0 ;
}
/*----------------------------------------------------------------*/
static void SetPalabolaForce( Work *work, FVECTOR *from, FVECTOR *to, int time, int gv )
{
	FVECTOR vec ;
	int dir, x_dis, y_dis ;
	float	s, ss ;

//printf( " form [%f] [%f] [%f] \n",from->vx, from->vy, from->vz ) ;
//printf( " to [%f] [%f] [%f] \n",to->vx, to->vy, to->vz ) ;

	_sceVu0SubVector(  &vec, to, from ) ;
	dir = _FVecDir2( &vec ) ;
	x_dis = _FVecLen2( &vec ) ; 
	y_dis = to->vy - from->vy ;

	work->speed = (float)x_dis / (float)time ;
	s = (float)y_dis/(float)time ;
	ss = (float)gv*(float)time/2.0 ;

	work->force.vx = work->speed * _RsinF( dir ) ;
	work->force.vz = work->speed * _RcosF( dir ) ;
	work->force.vy = s + ss ;
}

static	void	ThinkStart( Work *work )
{
	if ( work->count == 0 ) {
//		SetPalabolaForce( work, &work->control.mov, &StartRoute[work->c_route], PARABOLA_TIME,GRAVITATION ) ;
		SetPalabolaForce( work, &work->control.mov, &ParaRoute[work->c_route], COUNT_VMODE(60),GRAVITATION ) ;
		GM_SeSetMode( SD_A_MG_CSHOT, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->count == COUNT_VMODE(50) ) {
		GM_SeSetMode( SD_A_MG_CPARA, &work->control.mov, GM_SEMODE_BOMB ) ;
	}
	if ( (work->count >= COUNT_VMODE(50)) && (work->count < COUNT_VMODE(60)) ) {
		float s ;

		work->para.objs->flag &= ~DG_FLAG_INVISIBLE ;
		s = (float)COUNT_VMODE(60) - (float)work->count ;
		s = 1.0/s ;
		work->para_scale.vx = work->para_scale.vy = work->para_scale.vz = s ;
//printf("parachute scale[%f] \n",s);
	}
	ScaleMatrix( &work->para.objs->world, &work->para_scale ) ;

	if ( work->count > COUNT_VMODE(60) ) {
		work->think = THINK_PARA ;
		work->count = 0 ;
		return ;
	}

	work->force.vy -= GRAVITATION ;

	work->thk_status |= THK_STATUS_START ;
	work->count ++ ;
}

static	void	ThinkParachute( Work *work )
{
	if ( work->count == 0 ) {
		work->para.objs->flag &= ~DG_FLAG_INVISIBLE ;
		work->force.vx = 0.0 ;
		work->force.vz = 0.0 ;
		work->para_scale.vx = work->para_scale.vy = work->para_scale.vz = 1.0 ;
	}

	if ( (work->count >= COUNT_VMODE(130)) && (work->count < COUNT_VMODE(160))) {
		float s ;

		s = ((float)work->count - (float)COUNT_VMODE(128))/2.0 ;
		s = 1.0/s ;
		work->para_scale.vx = work->para_scale.vy = work->para_scale.vz = s ;
	} else if ( work->count == COUNT_VMODE(160) ) {
		work->para.objs->flag |= DG_FLAG_INVISIBLE ;
	}
	ScaleMatrix( &work->para.objs->world, &work->para_scale ) ;

	if ( work->count < COUNT_VMODE(140) ) {/*残り3000 */
		work->force.vy = STEP_VMODE(-50.0) ;
	} else {
		work->force.vy -= GRAVITATION ;
	}

	if ( work->count > COUNT_VMODE(160) ) {
		work->think = THINK_JUMP ;
		work->count = 0 ;
		return ;
	}

	work->thk_status |= THK_STATUS_START ;
	work->count ++ ;
}

static	void	ThinkJump( Work *work )
{
	if ( work->count == 0 ) {
		SetPalabolaForce( work, &work->control.mov, &MoveRoute[work->c_route].route[0], JUMP_TIME, GRAVI_JUMP ) ;
		GM_SeSetMode( SD_A_MG_CCHAK, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->count > JUMP_TIME ) {
		GM_ConfigControlAddressCheck( &work->control ) ;/* ゾーンアドレス取得 */
		GM_SeSetMode( SD_A_MG_CCHAK, &work->control.mov, GM_SEMODE_BOMB ) ;
		work->think = THINK_MOVE ;
		work->count = 0 ;
		return ;
	}

	work->force.vy -= GRAVI_JUMP ;

	work->thk_status |= THK_STATUS_START ;
	work->count ++ ;
}

static	void	ThinkMove( Work *work )
{
	work->thk_status |= THK_STATUS_MOVE ;

	if ( work->count == 0 ) {
		GM_SetNaviTargetFromPos( &work->n_trg, &MoveRoute[work->c_route].route[work->trg_point], work->control.hzx_id ) ;
		NextTrgPoint( work ) ;
	}

	if ( GLL_GAME_STATUS & GLL_GS_HEAD_REMOVE ) {
		work->think = THINK_BREAK ;
		work->count = 0 ;
		return ;
	}

	if ( GLL_GAME_STATUS & GLL_GS_HEAD_GROUNDED ) {
		GM_SeSetMode( SD_A_MG_CJUMP, &work->control.mov, GM_SEMODE_BOMB ) ;
		work->head_jump_time = 2 ;
	}

	if ( work->count > work->id * 5 ) {
		if ( GM_GameStatus & STATE_CHAFF ) {
			CONTROL *ctrl ;

			ctrl = &work->control ;
			ctrl->rot.vy = ctrl->turn.vy += STEP_VMODE(32) ;
			return ;
		}
	}

#if 1
	if ( !(GM_GameStatus & STATE_CHAFF) ) {
		if ( work->eye_flag == SIGHT_IN ) {
			work->think = THINK_READY ;
			work->count = 0 ;
			return ;
		}
		if ( GLL_GAME_STATUS & GLL_GS_SEARCH_LIGHT_IN ) {
			work->think = THINK_ALL_READY ;
			work->count = 0 ;
			return ;
		}
	}
#endif

#if 1
	if ( GM_Navi( &work->navi, &work->n_trg, 1000 ) ) {
		work->count = 0 ;
		return ;
	}
//printf("dir[%d]\n",work->navi.next_dir );

	work->speed = STEP_VMODE(25.0F) ;
	work->dir = work->navi.next_dir ;
#endif

	work->count ++ ;
}

static	void	ThinkReady( Work *work )
{
	CONTROL	*ctrl ;
	int		rot_x ;
	
	ctrl = &work->control ;
	work->thk_status |= THK_STATUS_FLY ;

	if ( work->count == 0 ) {
		work->force = DG_ZeroVector;
		work->force_rot = DG_ZeroSVector ;
		AN_HeadMark( &work->mark_world, 0x10 );/*赤！ミニ*/
		GM_SeSetMode( SD_A_MG_CBIKU, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->count > COUNT_VMODE(90) ) {
		work->think = THINK_ATTACK ;
		work->count = 0 ;
		return ;
	}

	if ( work->count > COUNT_VMODE(30) ) {
		if ( work->count < COUNT_VMODE(60) ) {
			work->force.vy = STEP_VMODE(20.0) ;
		} else {
			work->force.vy = 0.0 ;
			rot_x = RotPlayerX( &ctrl->mov ) ;
//			work->force_rot.vx = GV_NearSpeedP( work->force_rot.vx, rot_x, 24 ) ;
			work->force_rot.vx = GV_NearSpeedP( work->force_rot.vx, rot_x, STEP_VMODE(32) ) ;
		}
	}
	work->force_rot.vy = work->pl_dir ;
	work->count ++ ;
}

static	void	ThinkAllReady( Work *work )
{
	CONTROL	*ctrl ;
	int		rot_x ;
	
	ctrl = &work->control ;
	work->thk_status |= THK_STATUS_FLY ;

	if ( work->count == 0 ) {
		work->force = DG_ZeroVector;
		work->force_rot = DG_ZeroSVector ;
		AN_HeadMark( &work->mark_world, 0x10 );/*赤！ミニ*/
	}

	if ( work->count > (COUNT_VMODE(90) + (work->id*15)) ) {
		work->think = THINK_ATTACK ;
		work->count = 0 ;
		return ;
	}

	if ( work->count > (COUNT_VMODE(30) + COUNT_VMODE(work->id*15)) ) {
		work->force.vy = STEP_VMODE(30.0) ;/*少し高く*/
		rot_x = RotPlayerX( &ctrl->mov ) ;
		work->force_rot.vx = GV_NearSpeedP( work->force_rot.vx, rot_x, STEP_VMODE(24) ) ;
	}
	work->force_rot.vy = work->pl_dir ;
	work->count ++ ;
}

static	void	ThinkAttack( Work *work )
{
	if ( work->count == 0 ) {
		SetForce( work ) ;
		SetTarget( work ) ;
		work->force_rot.vx = RotPlayerX( &work->control.mov ) ;
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		GM_SeSetMode( SD_A_MG_CFIRE, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	if ( work->count > COUNT_VMODE(120) ) {
		work->thk_status |= THK_STATUS_BOMB ;
	}

	work->thk_status |= THK_STATUS_FLY ;
	work->thk_status |= THK_STATUS_ATTACK ;
	work->count ++ ;
}

static	void	ThinkBreak( Work *work )
{
	CONTROL	*ctrl ;

	ctrl = &work->control ;
	work->thk_status |= THK_STATUS_FLY ;

	if ( work->count == 0 ) {
		work->force = DG_ZeroVector;
		work->force_rot = DG_ZeroSVector ;
		AN_HeadMark( &work->mark_world, 0x2c );/*赤？ミニ*/
	}

	if ( work->count > (COUNT_VMODE(120) + (work->id*4)) ) {
		work->think = THINK_HANABI ;
		work->count = 0 ;
		return ;
	}

	if ( work->count < COUNT_VMODE(60) ) {
		work->force_rot.vy += STEP_VMODE(work->count*4) ;
		work->force_rot.vx = GV_NearSpeedP( work->force_rot.vx, 1024, STEP_VMODE(24) ) ;
	} else {
		work->force_rot.vy += STEP_VMODE(240) ;
	}

	work->count ++ ;
}

static	void	ThinkHanabi( Work *work )
{
	if ( work->count == 0 ) {
		SetTarget( work ) ;
		work->force.vy = STEP_VMODE(200.0) ;
	}

	if ( work->count > COUNT_VMODE(30) ) {
		work->thk_status |= THK_STATUS_BOMB ;
	}

	work->force.vy -= STEP_VMODE(8.0) ;
	work->force_rot.vy += STEP_VMODE(240) ;
	work->thk_status |= THK_STATUS_FLY ;
	work->thk_status |= THK_STATUS_ATTACK ;
	work->count ++ ;
}

/*----------------------------------------------------------------*/
static	void	Think( Work *work )
{
	work->dir = -1 ;
	work->thk_status = 0 ;

	switch( work->think ) {
		case THINK_START :
		 ThinkStart( work ) ;
		 break ;
		case THINK_PARA :
		 ThinkParachute( work ) ;
		 break ;
		case THINK_JUMP :
		 ThinkJump( work ) ;
		 break ;
		case THINK_MOVE :
		 ThinkMove( work ) ;
		 break ;
		case THINK_READY :
		 ThinkReady( work ) ;
		 break ;
		case THINK_ALL_READY :
		 ThinkAllReady( work ) ;
		 break ;
		case THINK_ATTACK :
		 ThinkAttack( work ) ;
		 break ;
		case THINK_BREAK :
		 ThinkBreak( work ) ;
		 break ;
		case THINK_HANABI :
		 ThinkHanabi( work ) ;
		 break ;
	}
}
/*----------------------------------------------------------------*/
static	void	Notice( Work *work )
{
	work->eye_flag = ( EyeCheck( work ) ) ? SIGHT_IN : SIGHT_OUT ;
}

static	void	Move( Work *work )
{
	NAVIGATE 	*navi ;
	CONTROL		*ctrl ;

	ctrl = &work->control ;
	navi = &work->navi ;

if ( work->eye_flag){
//AN_Test_Eye2(&ctrl->mov,2);
//AN_Test_Eye2(&work->n_trg.pos,2);
}

	if ( work->thk_status & THK_STATUS_FLY ) {
		ctrl->step = work->force ;
		ctrl->turn = work->force_rot ;
	} else if ( work->thk_status & THK_STATUS_START ) {
		ctrl->step = work->force ;
		ctrl->turn = work->force_rot ;
	} else if ( work->dir >= 0 ) {
		ctrl->step.vx = work->speed * _RsinF( (int)work->dir ) ;
		ctrl->step.vz = work->speed * _RcosF( (int)work->dir ) ;
		ctrl->turn.vy = work->dir ;
		if ( work->head_jump_time == 0 ) {
			ctrl->step.vy = 0.0F ;
		}
	} else if ( work->head_jump_time > 0 ) {
	} else {
		ctrl->step.vx = 0.0 ;
		ctrl->step.vz = 0.0 ;
		ctrl->step.vy = 0.0F ;
	}

	GM_MoveTarget( &work->deftrg, &work->control.mov ) ;
}

static	void	Attack( Work *work )
{
	CONTROL	*ctrl ;
	
	ctrl = &work->control ;
	if ( work->thk_status & THK_STATUS_ATTACK ) {
		if  ( ctrl->n_touches > 0 ||
			( work->thk_status & THK_STATUS_BOMB ) ||
			( work->trg_flag ) ) {
			work->thk_status |= THK_STATUS_DESTROY ;
			return ;
		}
	    GM_MoveOnlineTargetMap( &work->off, &work->old_pos, &work->control.mov, GM_StageMapAll ) ;
	    GM_PutTarget( &work->off ) ;
	}
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
static	void	Damage( Work *work )
{
	TARGET		*trg  ;
	
	trg = &work->deftrg ;

	if ( TARGET_POWER & trg->damaged ) {
		if ( trg->weapon_type  & (WP_BLAST) ) {
			work->life -= 10 ;
		}
		if ( trg->weapon_type  & (WP_BULLET) ) {
			work->life -= 1 ;
			CallSpark( &trg->hit , &trg->power->force ) ;
			GM_SeSetMode( SD_W_RICOCH02, &work->control.mov, GM_SEMODE_BOMB ) ;
			work->piku_time = 4 ;
		}
//printf("damage: [%lx] life[%d]\n",trg->weapon_type, work->life ) ;
		trg->weapon_type = 0 ;
		trg->damaged = 0 ;
		if ( work->life <= 0 ) {
			work->thk_status |= THK_STATUS_DESTROY ;
			return ;
		}
	}
}

static int PikuRot[4] = { -32, 0, 0, 0 } ;
static	void	Piku( Work *work )
{
	if ( work->piku_time > 0 ) {
		work->piku_time -- ;
		work->control.rot.vx += PikuRot[work->piku_time] ;
	}

	if ( work->thk_status & THK_STATUS_MOVE ) {
		/*いいかげんに作っているので注意！！*/
		if ( work->head_jump_time > 0 ) {
			if ( work->head_jump_time == 2 ) {
				work->head_jump_time -- ;
				work->control.step.vy = STEP_VMODE(150.0) ;
			} else {
				work->control.step.vy -= STEP_VMODE(8.0) ;
				if ( work->control.mov.vy < 501.0 ) {
					work->head_jump_time = 0 ;
					work->control.mov.vy = 500.0 ;
					work->control.step.vy -= 0.0 ;
				}
			}
//	printf("head drop[%d] control.mov.vy = %f \n",work->head_jump_time,work->control.mov.vy ) ;
		}
	}
}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	HZX_ChangeRouteCourse( HZX_ROOT_COURSE1|HZX_ROOT_COURSE2 ) ;

	ctrl = &work->control ;
	body = &work->body ;
	work->old_pos = ctrl->mov ;
	GM_ActControl( ctrl ) ;
	DG_PutObjs( body->objs );
	DG_PutObjs( work->para.objs );

	Notice( work ) ;
	Think( work ) ;
	Move( work ) ;
	Damage( work ) ;
	Attack( work ) ;
	Piku( work ) ;

	if ( work->thk_status & THK_STATUS_DESTROY ) {
		NewGllBlast3( &ctrl->mov, BOTH_SIDE, 2000, 3000, DMG_MISSILE, FNT_BLAST, WP_C4Bomb, BLAST_TYPE_ONETARG|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER ) ;
		GM_SeSetMode( SD_A_MG_COUTV, &work->control.mov, GM_SEMODE_BOMB ) ;
		GV_DestroyActor( work );
		return ;
	}

	{
		DG_SetPos( &work->body.objs->world ) ;
		DG_MovePos( &MarkShift ) ;
		DG_GetPos( &work->mark_world ) ;
	}

	HZX_ClearRouteCourse( 0 ) ;

}

static	void	Die( work )
Work		*work ;
{
	GM_FreeHomingTrg( &work->homing ) ;
    GM_FreeTarget( &work->deftrg ) ;
    GM_FreeControl( &work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_FreeObject( &work->para ) ;
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
//			work->off_center = off->center ;
//printf("koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee [%lx] \n",off->weapon_type ) ;
		}
	}
}

static void	InitTarget( Work *work )
{
	TARGET		*trg  ;
	FVECTOR 	shift ;

	trg = &work->deftrg ;

	shift.vx = 0.0 ;	shift.vy = 0.0 ;	shift.vz = 0.0 ;

	GM_SetTarget( trg, TARGET_CLASS, 1, ENEMY_SIDE,  &TargetSize, &shift ) ;
	GM_SetTargetWeaponType( trg, 0 ) ;
	GM_SetPowerTarget( trg, &work->power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargetCallBack, work ) ;

	GM_MoveTarget( trg, &work->control.mov ) ;
//NewTargetView( trg, 200, 34, 184 ) ;
}

static void InitHomingTarget( Work *work )
{
    HOMING_TRG 	*hom ;
	
	hom = &work->homing ;

	GM_SetHomingTrg( hom, &work->body.objs->world, NULL, &(work->control.hzx_id), &work->control, 0 ) ;
	GM_SetHomingTrgTarget( hom, &work->deftrg ) ;
	GM_PutHomingTrg( hom ) ;
}

static	int	GetResources( Work *work, FVECTOR *pos, int route )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	
	ctrl = &work->control ;
	body = &work->body ;
	
	/* ctrl 初期化 */
	GM_InitControl( ctrl, 0, GM_CurrentStageMap ) ;
	GM_ConfigControlHazard( ctrl, 500, 100, 110 ) ;
	ctrl->mov = *pos ;
	ctrl->rot = DG_ZeroSVector ;
//	ctrl->mov = MoveRoute[route].route[0] ;
//    GM_ConfigControlHzxHeight( ctrl, 1000.0F, ctrl->mov.vy + 100.0F ) ;

    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
	ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK ; /* 壁チェックなし */
//	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

	/* body 初期化 */
	GM_InitObject( body, BODY_NAME, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );

	/* para 初期化 */
	GM_InitObject( &work->para, PARA_NAME, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	work->para.objs->flag |=  DG_FLAG_INVISIBLE ;

	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_PutObjs( body->objs );
	DG_PutObjs( work->para.objs );

	/* navi 初期化 */
	GM_SetNavi( &work->navi, ctrl ) ;

	InitTarget( work ) ;

	InitHomingTarget( work ) ;

	work->think = THINK_START ;
	work->count = 0 ;
	work->c_route = route ;
	work->trg_point = 0 ;
	work->life = DEF_LIFE ;
	work->id = MissileID ++ ;
	work->piku_time = 0 ;
	work->head_jump_time = 0 ;

	work->sw_spot = 1 ;
	{
		extern void *NewGllMiniLight( FMATRIX *, int *, int, int * ) ;
		GV_SetActorChild( work, NewGllMiniLight( &work->body.objs->world, &work->sw_spot, 0, &work->control.hzx_id ) );
	}
{
	extern void *NewEyeView2( FMATRIX *world, int len, int range, int upper, int under ) ;
//	NewEyeView2( &(body->objs->world), EYE_LENGTH, EYE_LENGTH, EYE_X_RANGE, -EYE_X_RANGE ) ;
}
	return 0 ;
}

void	*NewGllMissile( FVECTOR	*pos, int route )
{
	Work		*work ;

	OPERATOR() ;

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, pos, route ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


void	GllMissileInitID( void )
{
	MissileID = 0 ;
}
