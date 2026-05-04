//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	weapon_sph2.c
	主観時の武器の水飛沫・わさわさタイプ

	1999/12/15 S.Okajima
	$Id: weapon_sph2.c,v 1.1.1.3 2002/11/19 11:47:16 Yoshizawa1 Exp $
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

#define	INNER_LIMIT			(0.1f)
#define	SHIFT_ROT			(256.0f)
#define	RAND_WIDTH			(8.0f)
#define	SIZE_RND			(2.0f)
#define	SIZE_MIN			(5.0f)

#define	MAX_ALPHA			(8.0f)

#define	SCALE				(1.05f)

/* ＣＶ２モデル内の部分モデル数 */
#define	MAX_MODEL_NUM		(3)


extern SVECTOR G_wind_rot;	/* 風向 */

/* ---------------------------------------------------------------- */
#define	SCR_LENGTH			( 0x4000 )
#define	FVEC_NUM			(SCR_LENGTH/16)
#define	FVEC_NUM_PART		(FVEC_NUM/2)

#define	NUM_POS		(512)


#define	SCR_LENGTH			( 0x4000 )
#define	FVEC_NUM			(SCR_LENGTH/16)

#define	SCR_NOR		(SCRPAD_ADDR)
#define	SCR_VER		(SCRPAD_ADDR + 0x2000)
#define	NUM_MAX		(0x2000 / 16)

#define	N_VERTS				(32)
#define	N_PRIMS				(NUM_MAX/N_VERTS)

typedef	struct	{
	GV_ACT_EX		actor;

	DG_PRIM2	*prim ;

	CV2_DEF		*cvd_def;
	int			n_models;
	int			n_num[MAX_MODEL_NUM];

	int			player_weapon;
	int			set_flag;

	SVECTOR		before_rot;

} Work ;

static inline void OK_ScaleMatrix( FMATRIX *mat )
{
	_sceVu0ScaleVector( (FVECTOR *)mat->m[0], (FVECTOR *)mat->m[0], SCALE );
	_sceVu0ScaleVector( (FVECTOR *)mat->m[1], (FVECTOR *)mat->m[1], SCALE );
	_sceVu0ScaleVector( (FVECTOR *)mat->m[2], (FVECTOR *)mat->m[2], SCALE );
}

static inline float OK_InnerProduct( FVECTOR *a, FVECTOR *b)
{
    float ans;
#ifdef BP_PSX2_ASM
    asm volatile ("
    lwc1	$f1,0(%1)
    lwc1	$f2,0(%2)
    lwc1	$f3,4(%1)
    lwc1	$f4,4(%2)
    lwc1	$f5,8(%1)
    lwc1	$f6,8(%2)
    mula.s	$f1,$f2
    madda.s	$f3,$f4
    madd.s	%0,$f5,$f6
    " : "=f"(ans) : "r"(a), "r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6");
#else
	ans = BP_Vec3_InnerProduct(a,b);
#endif
    return ans;
}

/* ---------------------------------------------------------------- */
static void	OK_InnerVector( FVECTOR *norms, FVECTOR *direction, int num )
{
	while ( -- num >= 0 ) {
		norms->vw = OK_InnerProduct( norms, direction );
		norms ++ ;
	}
}

static int SetUpCvd( Work *work )
{
	int		i;
	int		wp_cv2_id;

	/* モデルの共有頂点データ取得 */
	/* ＣＶ２データ用に全シェーディングをかけ、
	   頂点と法線の数を一致させたものを使う */

	switch( work->player_weapon ){
	  case WP_m92:
		wp_cv2_id = 5338657;//5338657 /*"m92_spl"*/;
		break;
	  case WP_Usp:
		wp_cv2_id = 3242137;//3242137 /*"usp_spl"*/;
		break;
	  case WP_Famas:
		wp_cv2_id = 6386893 /*"fms_spl"*/;
		break;
	  default:
		return 0;
	}



	work->cvd_def = GV_GetCache( GV_CacheID( wp_cv2_id, 'c' ) );
	if( work->cvd_def==NULL ){
		printf("ERR:CV2 data was NULL!!! :: %d\n",wp_cv2_id);
		return 0;
	}

	printf("n_models:%d\n",work->cvd_def->n_models);
	work->n_models=work->cvd_def->n_models;
	for( i=0; i<work->n_models; i++ ){
		work->n_num[i] = work->cvd_def->models[i].n_verts;
		if( work->n_num[i] > FVEC_NUM_PART ) work->n_num[i] = FVEC_NUM_PART;
	}

	return 1;
}


static void Act( Work *work )
{
	int	i,clock;
	int	num;
	int	pos_limit_num;
	FVECTOR	direction;
	SVECTOR	rot;
	FVECTOR	*verts;
	FVECTOR	*norms;
	FVECTOR	*pos;
	FMATRIX	mat;
	int		now_player_weapon;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;

	if( GM_PlayerControl != NULL ){
		if( (GM_PlayerControl->level_found & 2) ){
			DG_InvisiblePrim2( work->prim );
			return;
		}else{
			DG_VisiblePrim2( work->prim );
		}
	}

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


	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;


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

//-----------------
	pos   = work->prim->pos[clock];
	pos_limit_num = NUM_POS;
	uvrgbwh=work->prim->uvrgb[clock];
	for( i=0; i<work->n_models; i++ ){
		num   = work->n_num[i];
		/* スクラッチパットに転送 */
		OK_Mem_Scr( SCR_NOR, work->cvd_def->models[i].norms,  sizeof(FVECTOR), num ) ;
		OK_Mem_Scr( SCR_VER, work->cvd_def->models[i].verts,  sizeof(FVECTOR), num ) ;

		DG_COPY_MAT( &mat, &GM_PlayerSubWeaponBody->objs->objs[i].world );

		DG_SetPos( &mat );
		DG_RotVector( SCR_NOR, SCR_NOR, num );
		OK_ScaleMatrix( &mat );
		DG_SetPos( &mat );
		DG_PutVector( SCR_VER, SCR_VER, num );
		OK_InnerVector( SCR_NOR, &direction, num );	/* vw に 結果を入れる */

		verts = SCR_VER;
		norms = SCR_NOR;
		while ( -- num >= 0 ) {
			if( norms->vw > INNER_LIMIT && pos_limit_num >= 0){
				DG_COPY_VEC( pos, verts );
				pos->vx += rnd()*RAND_WIDTH-RAND_WIDTH*0.5f;
				pos->vy += rnd()*RAND_WIDTH-RAND_WIDTH*0.5f;
				pos->vz += rnd()*RAND_WIDTH-RAND_WIDTH*0.5f;
				pos->vw = 1.0f;
				if( --pos_limit_num<=0 ) break;
				uvrgbwh->a=MAX_ALPHA;
				pos++;
				uvrgbwh++;
			}
			verts++;
			norms++;
		}
	}

	while( --pos_limit_num >= 0){
		uvrgbwh->a=0;
		uvrgbwh++;
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
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh = SCRPAD_ADDR ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
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
//			uvrgbwh->a = 127 ;

			uvrgbwh ++ ;
		}
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGBWH), N_VERTS * N_PRIMS ) ;

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

	return 1;
}

static int GetResources( Work *work, int name, int where )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( 7745660 /*"splash06_msk"*/ );

	if( !InitPacket2( work, prim, tex ) ) return -1;
	DG_InvisiblePrim2( work->prim );

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewWeaponSplash2( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

//printf("NewWeaponSplash2\n");

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
