//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	weapon_sph.c
	主観時の武器の水飛沫

	1999/12/15 S.Okajima
	$Id: weapon_sph.c,v 1.1.1.3 2002/11/19 11:47:16 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	SHIFT_ROT		(256.0f)

#define	GRAVITY				(0.5f)
#define	SPEED_MAX			(3.0f)
#define	SPEED_RAND			(1.0f)

#define	LOW_LEVEL_SPEED		(-100.0f)
#define	RAND_WIDTH			(20.0f)

#define	INNER_LIMIT			(0.01f)

#define	SIZE_RND			(1.0f)
#define	SIZE_MIN			(1.8f)

#define	MAX_ALPHA			(30.0f)

extern void DG_ReflectVector( FVECTOR *pole, FVECTOR *vec1, FVECTOR *vec2 );

extern SVECTOR G_wind_rot;	/* 風向 */
extern int G_wind_intense;	/* 強さ */

/* ---------------------------------------------------------------- */
#define	SCR_LENGTH			( 0x4000 )
#define	FVEC_NUM			(SCR_LENGTH/16)

#define	SCR_NOR		(SCRPAD_ADDR)
#define	SCR_VER		(SCRPAD_ADDR + 0x2000)
#define	NUM_MAX		(0x2000 / 16)

#define	N_VERTS				(32)
#define	N_PRIMS				(4)

typedef	struct	{
	FVECTOR		pos[   N_PRIMS * N_VERTS ];
	FVECTOR		speed[ N_PRIMS * N_VERTS ];
} Unit ;


typedef	struct	{
	GV_ACT_EX		actor;

	DG_PRIM2	*prim ;

	CV2_DEF		*cvd_def;
	int			n_num;
	int			wp_cv2_id;
	int			player_weapon;
	int			set_flag;

	SVECTOR		before_rot;

	Unit		unit;
} Work ;

/* ---------------------------------------------------------------- */
static void	OK_InnerVector( FVECTOR *norms, FVECTOR *direction, int num )
{
	while ( -- num >= 0 ) {
		norms->vw = _sceVu0InnerProduct( norms, direction );
		norms ++ ;
	}
}


static int SetUpCvd( Work *work )
{

	/* モデルの共有頂点データ取得 */
	/* ＣＶ２データ用に全シェーディングをかけ、
	   頂点と法線の数を一致させたものを使う */

	switch( work->player_weapon ){
	  case WP_m92:
		work->wp_cv2_id = 5338657;//5338657 /*"m92_spl"*/;
		break;
	  case WP_Usp:
		work->wp_cv2_id = 3242137;//3242137 /*"usp_spl"*/;
		break;
//	  case WP_Famas:
//		work->wp_cv2_id = 108051;//108051 /*"fms"*/;
//		break;
	  default:
		return 0;
	}

	work->cvd_def = GV_GetCache( GV_CacheID( work->wp_cv2_id, 'c' ) );
	if( work->cvd_def==NULL ){
		printf("ERR:CV2 data was NULL!!! :: %d\n");
		return 0;
	}

	work->n_num = work->cvd_def->models[0].n_verts;
//printf("work->n_num::%d\n",work->n_num);
	if( work->n_num > NUM_MAX ) work->n_num = NUM_MAX;
//printf("work->n_num::%d\n",work->n_num);

	return 1;
}


static void Act( Work *work )
{
	static FVECTOR speed_source={ SPEED_MAX, SPEED_MAX, SPEED_MAX, SPEED_MAX };
	int	i,num,clock;
	FVECTOR	direction;
	SVECTOR	rot;
	FVECTOR	*verts;
	FVECTOR	*norms;
	FVECTOR	*prim_pos;
	FVECTOR	*pos;
	FVECTOR	*speed;
	int		now_player_weapon;

	if( GM_PlayerControl != NULL ){
		if( (GM_PlayerControl->level_found & 2) ){
			DG_InvisiblePrim2( work->prim );
			return;
		}else{
			DG_VisiblePrim2( work->prim );
		}
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	now_player_weapon = Ply_GetPlayerWeapon();

	if( work->player_weapon != now_player_weapon ){
		work->player_weapon = now_player_weapon;
		if( !SetUpCvd( work ) ){
			work->set_flag=0;
		}else{
			work->set_flag=1;
		}
	}

	if( now_player_weapon == WP_None
	 || !GM_CheckPlayerStatus(PLAYER_INTRUDE|PLAYER_WATCH)
	 || work->set_flag==0
	 || GM_PlayerSubWeaponBody==NULL ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else if( GM_PlayerSubWeaponBody->objs->flag & DG_FLAG_INVISIBLE0 ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
		GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	}

	direction.vx = 0.0f;
	direction.vy = 0.0f;
	direction.vz = -1.0f;
	direction.vw = 1.0f;

	rot.vx = (G_wind_rot.vx + 512-256) & 4095;
	rot.vy = (G_wind_rot.vy + 2048 + (short)(rnd()*SHIFT_ROT - SHIFT_ROT*0.5f)) & 4095;
	rot.vz = 0;
	OK_DirectionSmoother( &work->before_rot, &rot, 0.95f );

	DG_SetPos2( &DG_ZeroVector, &work->before_rot );
	DG_RotVector( &direction, &direction, 1 );

	/* スクラッチパットに転送 */

	OK_Mem_Scr( SCR_NOR, work->cvd_def->models[0].norms,  sizeof(FVECTOR), work->n_num ) ;
	OK_Mem_Scr( SCR_VER, work->cvd_def->models[0].verts,  sizeof(FVECTOR), work->n_num ) ;
//	DG_SetPos( &GM_PlayerArmBody->objs->objs[4].world );
	DG_SetPos( &GM_PlayerSubWeaponBody->objs->objs[0].world );
	DG_RotVector( SCR_NOR, SCR_NOR, work->n_num );
	DG_PutVector( SCR_VER, SCR_VER, work->n_num );
	OK_InnerVector( SCR_NOR, &direction, work->n_num );	/* vw に 結果を入れる */

	pos   = work->unit.pos;
	speed = work->unit.speed;
	norms = SCR_NOR;
	verts = SCR_VER;
	num   = work->n_num;
	i     = N_PRIMS * N_VERTS ;
	while ( --num >= 0 ) {
		if( (norms->vw > INNER_LIMIT && speed->vy <= LOW_LEVEL_SPEED && rnd()<0.2f) || (rnd()<0.001f) ){
			DG_COPY_VEC( pos, verts );
/*
			pos->vx += rnd()*RAND_WIDTH-RAND_WIDTH*0.5f;
			pos->vy += rnd()*RAND_WIDTH-RAND_WIDTH*0.5f;
			pos->vz += rnd()*RAND_WIDTH-RAND_WIDTH*0.5f;
*/
			_sceVu0MulVector( speed, &speed_source, &direction );
			speed->vx += rnd()*SPEED_RAND-SPEED_RAND*0.5f;
			speed->vy += rnd()*SPEED_RAND-SPEED_RAND*0.5f;
			speed->vz += rnd()*SPEED_RAND-SPEED_RAND*0.5f;
			DG_ReflectVector( norms, speed, speed );
			if(--i<=0) break;
			speed++;
			pos++;
		}
		verts ++ ;
		norms ++ ;
	}

	prim_pos   = work->prim->pos[clock];
	pos        = work->unit.pos;
	speed      = work->unit.speed;
	i          = N_PRIMS * N_VERTS ;
	while ( -- i >= 0 ){
		speed->vy -= GRAVITY;
		_sceVu0AddVector( pos, pos, speed );
		DG_COPY_VEC( prim_pos, pos );
		speed++;
		pos++;
		prim_pos++;
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

/* ---------------------------------------------------------------- */
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*speed ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	speed = work->unit.speed;
	uvrgbwh = SCRPAD_ADDR ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			speed->vy = LOW_LEVEL_SPEED - 1.0f;	/* 初期化を促す */

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->h = uvrgbwh->w = rnd()*SIZE_RND + SIZE_MIN ;

			uvrgbwh->r = 255 ;
			uvrgbwh->g = 255 ;
			uvrgbwh->b = 255 ;
			uvrgbwh->a = MAX_ALPHA ;

			speed++;
			uvrgbwh ++ ;
		}
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS * N_PRIMS ) ;

	return 1;
}

static int GetResources( Work *work, int name, int where )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->player_weapon=-1;
	work->set_flag=0;


	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	if( !InitPacket2( work, prim, tex ) ) return -1;
	DG_InvisiblePrim2( work->prim );


	{
		extern void *NewWeaponSplash2( int name, int where );
		NewWeaponSplash2( name, where );
	}

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewWeaponSplash( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

//printf("NewWeaponSplash\n");

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
