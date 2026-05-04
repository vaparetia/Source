//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	NewEyeBeem.c
   
	2002/04/16 Y.Korekado

	$Id: eyebeem.c,v 1.1.1.3 2002/11/19 11:44:10 Yoshizawa1 Exp $
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

#include	"gll_def.h"

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
//#define		DMG_EYEBEEM	DMG_BLAST
#define		DMG_EYEBEEM	(30)

#define		N_PRIMS		(1)
#define		N_VERTS		(1)
#define		PRIM_TYPE	(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

#define		TEX_CODE 	(9300995)	// kirari_msk
//#define		TEX_CODE 	(13276037)	// kirari

//#define		SHIFT_X	(0.0f)
//#define		SHIFT_Y	(37.5f)
//#define		SHIFT_Z	(124.5f) //(117.5)

//#define		SIZE_W	(100.0f)
//#define		SIZE_H	(25.0f)

#define		SHIFT_X	(0.0f)
#define		SHIFT_Y	(375.0f)
#define		SHIFT_Z	(1245.0f) //(117.5)

#define		SIZE_W	(1000.0f)
#define		SIZE_H	(250.0f)

//#define		LIFE	(8)
#define		LIFE	COUNT_VMODE(180)

typedef struct {
    GV_ACT_EX		actor;

	CONTROL			control ;

    DG_PRIM2	    *prim;
    FMATRIX	    	world;
    FMATRIX	    	*p_world;
	FVECTOR			shift;

	FVECTOR			trg_pos ;

	short			u0[3];
	short			u1[3];

    short			timer;
	short			now_anime;

    TARGET		off ;
    int			flag ;
    int			mode ;
    int			mabataki_time ;
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
#define EYE_FLAG_HIT			0x00000001
#define EYE_FLAG_MABATAKI		0x00000002
#define EYE_FLAG_BOOK_MABATAKI	0x00000004

/*--------------------------------------------------------------------*/
static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;

	if(0){
		FVECTOR		dc ;

		GM_TargetGetCenter( &dc, def ) ;
		if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &off->hit, &dc, 
								    HZX_CHK_ALL,
								    HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
								    HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
			GM_TargetHitCancel( off, def ) ;
			return ;
		}
	}

    work = ( Work * )ptr ;
    work->flag |= EYE_FLAG_HIT ;
    DG_COPY_VEC( &work->control.mov, &off->hit ) ;
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

	if ( work->timer < COUNT_VMODE(60) ) {
		work->world = *work->p_world ;
		if ( work->timer%0x8 ) DG_InvisiblePrim2(work->prim);
	}

	if ( work->timer == COUNT_VMODE(60) ) {
		extern void	_FVecToRotXY( FVECTOR *vec, SVECTOR *rot ) ;
		FVECTOR vec ;
		SVECTOR rot ;

		if ( work->mode == 0 ) {
			_sceVu0SubVector(  &vec, &work->trg_pos, (FVECTOR *)(&work->world.m[3][0]) ) ;
			_FVecToRotXY( &vec, &rot ) ;
//printf(" vec vx[%f] vy[%f] vz[%f]\n",vec.vx,vec.vy,vec.vz ) ;
//printf(" rotvx[%d] vy[%d] vz[%d]\n",rot.vx,rot.vy,rot.vz ) ;
			rot.vx -= 1028 ;
			DG_SetPos2( (FVECTOR *)(&work->world.m[3][0]), &rot ) ;
			DG_GetPos( &work->world ) ;
		}
		GM_SeSetMode( SD_A_EYEBON01, &work->control.mov, GM_SEMODE_BOMB ) ;

#ifdef DEBUG_MODE
#if 0
if(0){
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = work->trg_pos ;
	KR_FMatToFvec( &work->world, &lin[1] ) ;
	NewLineView(  &lin[0],1,0,0,255) ;
	AN_Test_Eye2(&work->world.m[3][0],2);
}
#endif
#endif
	}
	if ( GllEyebeemFlag & (GLL_EYEBEEM_ORDER_STOP|GLL_EYEBEEM_ORDER_BOOK_STOP) ) {
		work->flag |= EYE_FLAG_MABATAKI ;
		if ( GllEyebeemFlag & GLL_EYEBEEM_ORDER_BOOK_STOP ) {
			work->flag |= EYE_FLAG_BOOK_MABATAKI ;
		}
	}

	if ( work->flag & EYE_FLAG_MABATAKI ) {
		FVECTOR	shift_pos[4] ;
		float	dy ;
		int	m, i ;

		m = work->mabataki_time ;
		if ( ((m > COUNT_VMODE(420)) && (m <= COUNT_VMODE(421))) || ((m > COUNT_VMODE(430)) && (m <= COUNT_VMODE(431))) ) {
			dy = STEP_VMODE(100.0) ;
		} else if ( ((m > COUNT_VMODE(421)) && (m <= COUNT_VMODE(422))) || ((m > COUNT_VMODE(431)) && (m <= COUNT_VMODE(432))) ) {
			dy = STEP_VMODE(150.0) ;
		} else if ( ((m > COUNT_VMODE(422)) && (m <= COUNT_VMODE(423))) || ((m > COUNT_VMODE(432)) && (m <= COUNT_VMODE(433))) ) {
			dy = STEP_VMODE(200.0) ;
		} else if ( ((m > COUNT_VMODE(423)) && (m <= COUNT_VMODE(424))) || ((m > COUNT_VMODE(433)) && (m <= COUNT_VMODE(434))) ) {
			dy = STEP_VMODE(100.0) ;
		}

		for ( i=0; i<4; i++ ) {
			shift_pos[i].vx = init_pos[i].vx ;
			shift_pos[i].vy = init_pos[i].vy + dy ;
			shift_pos[i].vz = init_pos[i].vz ;
			if ( i==1 ) dy *= -1 ;
		}
		DG_SetPos( &work->world );
		DG_PutVector( shift_pos, pos, 4 );

		if ( work->flag & EYE_FLAG_BOOK_MABATAKI ) {
			if ( ++work->mabataki_time > COUNT_VMODE(482) ) {
				work->flag &= ~EYE_FLAG_MABATAKI ;
				work->timer = LIFE ;
			}
		} else {
			if ( ++work->mabataki_time > COUNT_VMODE(660) ) {
				work->flag &= ~EYE_FLAG_MABATAKI ;
				work->timer = LIFE ;
			}
		}
	} else {
		DG_SetPos( &work->world );
		if ( work->timer >= COUNT_VMODE(60) ) {
			DG_PutVector( &center_pos, &from, 1 );

			work->shift.vz = STEP_VMODE(600.0) ;
			DG_MovePos( &work->shift ) ;
			DG_GetPos( &work->world );

			DG_PutVector( &center_pos, &to, 1 );
		}
		DG_PutVector( init_pos, pos, 4 );
		GllEyebeemWorld = work->world ;

		if ( work->timer >= COUNT_VMODE(60) ) {
			CONTROL *ctrl ;
			
			ctrl = &work->control ;
			DG_COPY_VEC( &from, &ctrl->mov ) ;
			_sceVu0SubVector( &ctrl->step, &to, &from ) ;
			GM_ActControl( &( work->control ) ) ;
//	AN_Test_Eye2(&ctrl->mov,2);
			if  ( ctrl->n_touches > 0 ||
				( ctrl->mov.vy < 500.0) ||
				( work->flag & EYE_FLAG_HIT ) ) {
				extern void *NewGllBlast3( FVECTOR *, int, int ,int ,int, int, int, int ) ;
				static FVECTOR	blast_shift = { 0.0, 0.0, SHIFT_Z } ;
				FMATRIX			blast_mat ;

				DG_SetPos( &work->world );
				DG_MovePos( &blast_shift ) ;
				DG_GetPos( &blast_mat );

				NewGllBlast3( (FVECTOR *)&blast_mat.m[3][0], BOTH_SIDE, 2000, 3000, DMG_EYEBEEM, FNT_BLAST, WP_C4Bomb, BLAST_TYPE_ONETARG|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER ) ;
				GV_DestroyActor( work );
				return ;
			}

		    GM_MoveOnlineTargetMap( &work->off, &from, &work->control.mov, GM_StageMapAll ) ;
		    GM_PutTarget( &work->off ) ;
			GllEyebeemFlag |= GLL_EYEBEEM_MOVE ;
		}
	}

	if(work->timer == 2 || work->timer == 3){
		uvrgb[0].u = work->u0[1];
		uvrgb[1].u = work->u1[1];
		uvrgb[2].u = work->u0[1];
		uvrgb[3].u = work->u1[1];
	}else if(work->timer == 4 && work->timer == 5 ){
		uvrgb[0].u = work->u0[2];
		uvrgb[1].u = work->u1[2];
		uvrgb[2].u = work->u0[2];
		uvrgb[3].u = work->u1[2];
	}

	if(++work->timer >= LIFE ){
		if ( !(work->flag & EYE_FLAG_MABATAKI) ) {
			DG_InvisiblePrim2(work->prim);
			GV_DestroyActor( work );
		}
	}
}

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

//SET_FLAG( ctrl->skip_flag, CTRL_SKIP_SEG_CHECK ) ;
	return 0 ;
}

static	void	SetTarget( work, name )
Work		*work ;
int		name ;
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
    
    work->flag = 0 ;
}

static int GetResources( Work *work, FVECTOR *trg, int mode )
{
	DG_TEX				*tex ;
	DG_PRIM2			*prim ;
	DG_PRIM2_UVRGB		*uvrgb0,*uvrgb1;
	int					add_u,off_u,tri_w,i;

	work->timer = 0;
	work->mabataki_time = 0;
	work->now_anime = 0;
	work->mode = mode ;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
									  1, 4);
	if(prim == NULL) return -1 ;
	tex = DG_GetTexture(TEX_CODE);
	if(tex == NULL) return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
//	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 )) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 )) ;

	tri_w = 1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f);
	DG_GetTexelInfo( &add_u, NULL, &off_u, NULL, tex);
	add_u /= 3;

	work->u0[0] = FTOI12(((float)(off_u + 0) + 0.5f)/(float)tri_w);
	work->u0[1] = FTOI12(((float)(off_u + add_u) + 0.5f)/(float)tri_w);
	work->u0[2] = FTOI12(((float)(off_u + add_u*2) + 0.5f)/(float)tri_w);

	work->u1[0] = FTOI12(((float)(off_u + add_u) - 0.5f)/(float)tri_w);
	work->u1[1] = FTOI12(((float)(off_u + add_u*2) - 0.5f)/(float)tri_w);
	work->u1[2] = FTOI12(((float)(off_u + add_u*3) - 0.5f)/(float)tri_w);

	//pos init
	DG_SetPos(&work->world);
	DG_PutVector(init_pos,prim->pos[0],4);
	DG_PutVector(init_pos,prim->pos[1],4);

	uvrgb0 = prim->uvrgb[0];
	uvrgb1 = prim->uvrgb[1];
	for( i = 0; i < 4; i++ ){
		uvrgb0->u = (i&1)?work->u1[0]:work->u0[0];
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
	SetTarget( work, work->control.name ) ;

	GM_SeSetMode( SD_A_EYEBRI01, &work->control.mov, GM_SEMODE_BOMB ) ;

	work->trg_pos = *trg ;

	return 0;
}


void *NewEyeBeem(FMATRIX *world, FVECTOR *trg, int mode)
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
	    if ( GetResources( work, trg, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
	    }
	}
	return (void *)work ;
}
