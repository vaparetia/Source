//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	NewSonic.c
	ごるるごん超音波
   
	2002/07/10 Y.Korekado

	$Id: sonic.c,v 1.1.1.3 2002/11/19 11:44:15 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/define.h"

//#include	"../util/ts_util.h"
#define	VERT_KICK_CODE	(0x8fff)
#define	DRAW_KICK_CODE	(0x0fff)
/* ------------------------------------------------------ */
           /* 
               補助マクロ
           */

#define   FTOI12(_f)        ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define		N_PRIMS		(1)
#define		N_VERTS		(1)
#define		PRIM_TYPE	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

#define		TEX_CODE 	(7744599)	// hamon09_add_alp

//#define		SHIFT_X	(0.0f)
//#define		SHIFT_Y	(37.5f)
//#define		SHIFT_Z	(124.5f) //(117.5)

//#define		SIZE_W	(100.0f)
//#define		SIZE_H	(25.0f)

#define		SHIFT_X	(0.0f)
#define		SHIFT_Y	(375.0f)
#define		SHIFT_Z	(1245.0f) //(117.5)

#define		SIZE_W	(200.0f)
#define		SIZE_H	(100.0f)

#define		SIZE_ZOOM	(30.0f)
#define		SONIC_SPEED	STEP_VMODE(300.0f)

//#define		LIFE	(8)
#define		LIFE	COUNT_VMODE(120)

typedef struct {
    GV_ACT_EX	actor;

	CONTROL		control ;

    DG_PRIM2	*prim;
    FMATRIX	    world;
    FMATRIX	    s_world;
    FMATRIX	    *p_world;
	FVECTOR		shift;

	FVECTOR		trg_pos ;

    int			timer ;
    int			flag ;

	FVECTOR		now_pos[4] ;
} Work;

static FVECTOR		init_pos[4] = {
	{ SHIFT_X - SIZE_W, SHIFT_Y - SIZE_H, SHIFT_Z, 1.0f },
	{ SHIFT_X + SIZE_W, SHIFT_Y - SIZE_H, SHIFT_Z, 1.0f },
	{ SHIFT_X - SIZE_W, SHIFT_Y + SIZE_H, SHIFT_Z, 1.0f },
	{ SHIFT_X + SIZE_W, SHIFT_Y + SIZE_H, SHIFT_Z, 1.0f },
};

static FVECTOR		center_pos = {
	SHIFT_X, SHIFT_Y, SHIFT_Z, 1.0f
};

/*--------------------------------------------------------------------*/
#define SONIC_FLAG_PL_FREEZ	0x00000001
/*--------------------------------------------------------------------*/
#define ATK_LEN	(1000.0)
static float	InnerProduct( FVECTOR *p, FVECTOR *v1, FVECTOR *v2 )
{
    FVECTOR	d1, d2 ;
    float f ;

    _sceVu0SubVector( &d1, p, v1 ) ;
    _sceVu0SubVector( &d2, p, v2 ) ;
    _sceVu0Normalize( &d1, &d1 ) ;
    _sceVu0Normalize( &d2, &d2 ) ;
    f = _sceVu0InnerProduct( &d1, &d2) ;

	return f ;
}
static int HitCheck( Work *work )
{
	FVECTOR *center, *at_center, *at_range, *trgpos ;
	float len_at, len_trg, at_trg, ip_at, ip_trg ;

	center = (FVECTOR *)&work->s_world.m[3][0] ;
	at_center = &work->control.mov ;
	at_range = work->prim->pos[work->prim->buffer_clock] ;
	trgpos = &GM_PlayerPosition ;

	len_at = GV_VecLen3F2( center, at_center ) ;
	len_trg = GV_VecLen3F2( center, trgpos ) ;
	at_trg = len_at - len_trg ;

//printf("sonic : c[%f][%f][%f] a[%f][%f][%f]\n",center->vx, center->vy, center->vz, at_center->vx,at_center->vy,at_center->vz) ;
//printf("sonic : t[%f][%f][%f] len_at[%f] len_trg[%f]\n",trgpos->vx, trgpos->vy, trgpos->vz, len_at, len_trg ) ;
//printf("sonic : len [%f] \n",at_trg ) ;
	/* 距離判定 */
	if ( at_trg > ATK_LEN ) return 0 ;
	if ( at_trg < -ATK_LEN ) return 0 ;

	ip_at = InnerProduct( center, at_center, at_range ) ;
	ip_trg = InnerProduct( center, at_center, trgpos ) ;

//printf("sonic : innner p at[%f] trg[%f]\n",ip_at, ip_trg ) ;
	if ( ip_trg < ip_at ) return 0 ;
	
	return 1 ;	/* あたり！ */
}

static	void SonicAttack( Work *work )
{
	extern void	PL_StartKageshibari( int time ) ;
	extern int		PL_KageshibariNow( void ) ;

#if 1
	if ( HitCheck( work ) ) {
		PL_StartKageshibari( COUNT_VMODE(240) ) ;
	}
#else
	FVECTOR	vec ;
	float len, range ;

	len = GV_VecLen3F2( &GM_PlayerPosition, &work->control.mov ) ;
	range = (work->now_pos[1].vx - work->now_pos[0].vx) / 2.0 ;

	if ( len < range ) {
		PL_StartKageshibari( COUNT_VMODE(240) ) ;
	}
#endif

	if ( work->flag & SONIC_FLAG_PL_FREEZ ) {
		if ( !PL_KageshibariNow() ) {
			work->flag &= ~SONIC_FLAG_PL_FREEZ ;
		}
	} else {
		if ( PL_KageshibariNow() ) {
			work->flag |= SONIC_FLAG_PL_FREEZ ;
			GM_SeSetMode( SD_E_V_KAGE02, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
	}
}
/*--------------------------------------------------------------------*/
static  void  Act(Work *work)
{
	int					clock ;
	FVECTOR				*pos, from, to ;
	DG_PRIM2_UVRGB		*uvrgb ;

	DG_VisiblePrim2(work->prim);
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = work->prim->pos[clock];
	uvrgb = work->prim->uvrgb[clock];
	DG_VisiblePrim2(work->prim);

	if ( work->timer == COUNT_VMODE(0) ) {
		extern void	_FVecToRotXY( FVECTOR *vec, SVECTOR *rot ) ;
		FVECTOR vec ;
		SVECTOR rot ;
		_sceVu0SubVector(  &vec, &work->trg_pos, (FVECTOR *)(&work->world.m[3][0]) ) ;
		_FVecToRotXY( &vec, &rot ) ;
//printf(" vec vx[%f] vy[%f] vz[%f]\n",vec.vx,vec.vy,vec.vz ) ;
//printf(" rotvx[%d] vy[%d] vz[%d]\n",rot.vx,rot.vy,rot.vz ) ;
		rot.vx -= 1028 ;
		DG_SetPos2( (FVECTOR *)(&work->world.m[3][0]), &rot ) ;
		DG_GetPos( &work->world ) ;
#if 0
{
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = work->trg_pos ;
	KR_FMatToFvec( &work->world, &lin[1] ) ;
	NewLineView(  &lin[0],1,0,0,255) ;
	AN_Test_Eye2(&work->world.m[3][0],2);
}
#endif
	}

	DG_SetPos( &work->world );

	/* 移動 */
	if ( work->timer >= COUNT_VMODE(0) ) {
		DG_PutVector( &center_pos, &from, 1 );

		work->shift.vz = SONIC_SPEED ;
		DG_MovePos( &work->shift ) ;
		DG_GetPos( &work->world );

		DG_PutVector( &center_pos, &to, 1 );
		
		work->now_pos[0].vx -= SIZE_ZOOM ;
		work->now_pos[0].vy -= SIZE_ZOOM ;

		work->now_pos[1].vx += SIZE_ZOOM ;
		work->now_pos[1].vy -= SIZE_ZOOM ;

		work->now_pos[2].vx -= SIZE_ZOOM ;
		work->now_pos[2].vy += SIZE_ZOOM ;

		work->now_pos[3].vx += SIZE_ZOOM ;
		work->now_pos[3].vy += SIZE_ZOOM ;
	}
	DG_PutVector( work->now_pos, pos, 4 );

	if ( work->timer >= COUNT_VMODE(0) ) {
#if 1
		/* 壁判定無し */
		DG_COPY_VEC( &work->control.mov, &to ) ;
#else
		CONTROL *ctrl ;
		
		ctrl = &work->control ;
		DG_COPY_VEC( &from, &ctrl->mov ) ;
		_sceVu0SubVector( &ctrl->step, &to, &from ) ;
		GM_ActControl( &( work->control ) ) ;
#endif
	}

	SonicAttack( work ) ;

	if ( !(work->timer%COUNT_VMODE(15)) ) {
		GM_SeSetMode( SD_A_LIPPLE01, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	//AN_Test_Eye2(pos,2);

	if(++work->timer >= LIFE ){
		DG_InvisiblePrim2(work->prim);
		GV_DestroyActor( work );
	}
}

    FMATRIX	    world;
static  void  Die(Work *work)
{
    GM_FreeControl( &work->control ) ;
    if(work->prim != NULL) GM_FreePrim2(work->prim) ;
}

static int InitControl( Work *work )
{
    CONTROL	*ctrl ;
    FMATRIX	*world ;
	
	ctrl = &( work->control ) ;
	world = work->p_world ;

    if ( GM_InitControl( ctrl, 0, GM_CurrentStageMap ) < 0 ) return -1 ;
	GM_ConfigControlHazard( ctrl, 1200, 4500, 5000 ) ;
    ctrl->mov.vx = world->m[ 3 ][ 0 ] ;
    ctrl->mov.vy = world->m[ 3 ][ 1 ] ;
    ctrl->mov.vz = world->m[ 3 ][ 2 ] ;
    ctrl->rot = ctrl->turn = DG_ZeroSVector ;

    ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
    ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
    ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;

	return 0 ;
//SET_FLAG( ctrl->skip_flag, CTRL_SKIP_SEG_CHECK ) ;
}

static int GetResources( Work *work )
{
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGB		*uvrgb0,*uvrgb1;
	int					i;

	work->flag = 0 ;
	work->timer = 0;
	work->s_world = *work->p_world ;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
									  1, 4);
	if(prim == NULL) return -1 ;
	tex = DG_GetTexture(TEX_CODE);
	if(tex == NULL) return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 )) ;

	DG_COPY_VEC( &work->now_pos[0], &init_pos[0] ) ;
	DG_COPY_VEC( &work->now_pos[1], &init_pos[1] ) ;
	DG_COPY_VEC( &work->now_pos[2], &init_pos[2] ) ;
	DG_COPY_VEC( &work->now_pos[3], &init_pos[3] ) ;

	//pos init
	DG_SetPos(&work->world);
	DG_PutVector(work->now_pos,prim->pos[0],4);
	DG_PutVector(work->now_pos,prim->pos[1],4);

	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];
	for( i = 0; i < 4; i++ ){
		uvrgb0->u = (i&1)?FTOI12(tex->u_scale + tex->u_offset) : FTOI12(tex->u_offset);	
		uvrgb0->v = (i<2)?FTOI12(tex->v_offset):FTOI12(tex->v_scale + tex->v_offset);

		uvrgb0->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb0->q = 4096;

		uvrgb0->r = 196 - 32;
		uvrgb0->g = 196 - 32;
		uvrgb0->b = 128 - 16;
		uvrgb0->a = 128;
	
		*uvrgb1 = *uvrgb0;

		uvrgb1++; uvrgb0++;
		
	}

	work->shift = DG_ZeroVector ;

	if ( InitControl( work ) < 0 ) return -1 ;

	work->trg_pos = GM_PlayerPosition ;

	return 0;
}


void *NewSonic(FMATRIX *world)
{
	Work		*work ;

	//printf("キュピーン\n");
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) 
	{
	    work->world = *world ;
	    work->p_world = world ;
	    GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
	    if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
	    }
	}
	return (void *)work ;
}
