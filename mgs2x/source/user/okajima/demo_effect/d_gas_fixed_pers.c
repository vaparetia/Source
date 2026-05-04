//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_gas_fixed_pers.c
	テクセル流し（透視変換バージョン・位置指定型）

	2000/03/30 S.Okajima
	$Id: d_gas_fixed_pers.c,v 1.1.1.3 2002/11/19 11:46:52 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
#include	"libmt.h"
#include	"libutl.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/

#define	SCREEN_NEAR	( 51.0f )

#define	POS_VS_UVS	(1 + 1 + 2)

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	PARAM1		(MAX_FVECTOR / POS_VS_UVS)
#define	PARAM2		(PARAM1 * 16)

#define	SCR_SPACE	(SCRPAD_ADDR)
#define	SCR_POS		(SCRPAD_ADDR + PARAM2)
#define	SCR_UVS		(SCRPAD_ADDR + PARAM2 + PARAM2 )

/* スクラッチパットぎりぎりまでとる */
#define	N_VERTS2			(16)
#define	N_PRIMS2			(PARAM1/N_VERTS2)

typedef	struct	{
	GV_ACT_EX		actor ;

	int		life;

	FVECTOR bound_0;
	FVECTOR bound_1;
	FVECTOR diff;
	FVECTOR vec;
	FVECTOR col;
	int		alpha_add;
	int		alpha_sub;

	FVECTOR space_pos[N_VERTS2 * N_PRIMS2];	/* vw は angle */
	FVECTOR space_vec[N_VERTS2 * N_PRIMS2];
	int     space_alpha[N_VERTS2 * N_PRIMS2];
	float   space_size[N_VERTS2 * N_PRIMS2];

	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i,j;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	*pos;
	FVECTOR	*space;
	FVECTOR	*space_vec;
	float	screen_near_x;
	float	screen_near_y;
	int     *space_alpha;
	float   *space_size;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	OK_Mem_Scr( SCR_SPACE, work->space_pos,            sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Mem_Scr( SCR_POS,   work->prim->pos[ clock ],   sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Mem_Scr( SCR_UVS,   work->prim->uvrgb[ clock ], sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2) ;

	space   = SCR_SPACE;
	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;
	space_vec = work->space_vec;
	space_size  = work->space_size;
	space_alpha = work->space_alpha;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( j = 0 ; j < N_VERTS2 ; j++ ){
			if( vu0_CheckBoundingBox( space, &work->bound_0, &work->bound_1 ) ){
				/* バンダリ内だと明るくする */
				(*space_alpha) += work->alpha_add ;
				if( (*space_alpha) > (int)work->col.vw ) (*space_alpha) = (int)work->col.vw ;
			}else{
				if( (*space_alpha) >  work->alpha_sub ){ // 引きすぎないようにする
					/* バンダリ外だと暗くする */
					(*space_alpha) -= work->alpha_sub ;
				}else if( (*space_alpha) >  0 ){
					(*space_alpha)-- ;
				}else{
					/* これ以上暗く出来ないので位置・明るさを初期化する */
					space->vx = work->bound_0.vx + work->diff.vx*rnd();
					space->vy = work->bound_0.vy + work->diff.vy*rnd();
					space->vz = work->bound_0.vz + work->diff.vz*rnd();
					uvrgbwh->w = (int)(cosf( space->vw ) * (*space_size));
					uvrgbwh->h = (int)(sinf( space->vw ) * (*space_size));
				}
			}
			space->vx += space_vec->vx;
			space->vy += space_vec->vy;
			space->vz += space_vec->vz;
			uvrgbwh->a = (*space_alpha) ;

			space_vec++;
			space_alpha++;
			space_size++;
			space++;
			uvrgbwh++;
		}
	}
	/* 以上で３Ｄ空間の点の移動は終わり */

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	/* 座標を全部透視変換する */
	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( SCR_SPACE, SCR_POS, N_VERTS2 * N_PRIMS2 );

	pos     = SCR_POS;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( j = 0 ; j < N_VERTS2 ; j++ ){
			if( pos->vz < pos->vw ){
				pos->vw = (pos->vw  > 0.0f)? pos->vw: -pos->vw;
				pos->vz = SCREEN_NEAR;
				pos->vx = screen_near_x * pos->vx / pos->vw;
				pos->vy = screen_near_y * pos->vy / pos->vw;
			}else{
				pos->vz = 0.0f;
			}
			pos++;
		}
	}


	OK_Scr_Mem( work->space_pos,            SCR_SPACE, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( work->prim->pos[ clock ],   SCR_POS,   sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( work->prim->uvrgb[ clock ], SCR_UVS,   sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

	if( work->life > 0 ){
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, float size, int mode )
{
	FVECTOR				*pos ;
	FVECTOR				*space_vec ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	int     *space_alpha;
	float   *space_size;
	float   ftemp;

	DG_ConfigPrim2Tex( prim, tex );
	switch( mode ){
	  case 0:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	  case 1:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		break;
	  case 2:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		break;
	  default:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	}

	pos     = SCR_POS ;
	uvrgbwh = SCR_UVS ;
	space_size  = work->space_size;
	space_alpha = work->space_alpha;
	space_vec   = work->space_vec;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos->vx = 0.0f ;
			pos->vy = 0.0f ;
			pos->vz = 0.0f ;
			pos->vw = TPI * rnd();

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			(*space_size) = size * (1.0f + rnd()) * 0.5f;
			uvrgbwh->w = (int)(cosf( pos->vw ) * (*space_size));
			uvrgbwh->h = (int)(sinf( pos->vw ) * (*space_size));

			uvrgbwh->r = (int)work->col.vx ;
			uvrgbwh->g = (int)work->col.vy ;
			uvrgbwh->b = (int)work->col.vz ;
//			(*space_alpha) = (int)(work->col.vw * rnd());
			(*space_alpha) = 0;
			uvrgbwh->a = 0 ;

			ftemp = 0.1f + rnd()*0.9f;
			space_vec->vx = (work->vec.vx + work->vec.vx * 0.1f * rnd()) * ftemp;
			space_vec->vy = (work->vec.vy + work->vec.vy * 0.1f * rnd()) * ftemp;
			space_vec->vz = (work->vec.vz + work->vec.vz * 0.1f * rnd()) * ftemp;

			space_vec++;
			space_alpha++;
			space_size++;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( work->space_pos,  SCR_POS, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

	return 1;
}

static int GetResources( Work *work, FVECTOR *bound0, FVECTOR *bound1, FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	if( bound0->vx < bound1->vx ){
		work->bound_0.vx = bound0->vx;
		work->bound_1.vx = bound1->vx;
	}else{
		work->bound_0.vx = bound1->vx;
		work->bound_1.vx = bound0->vx;
	}
	if( bound0->vy < bound1->vy ){
		work->bound_0.vy = bound0->vy;
		work->bound_1.vy = bound1->vy;
	}else{
		work->bound_0.vy = bound1->vy;
		work->bound_1.vy = bound0->vy;
	}
	if( bound0->vz < bound1->vz ){
		work->bound_0.vz = bound0->vz;
		work->bound_1.vz = bound1->vz;
	}else{
		work->bound_0.vz = bound1->vz;
		work->bound_1.vz = bound0->vz;
	}

	work->diff.vx = work->bound_1.vx - work->bound_0.vx;
	work->diff.vy = work->bound_1.vy - work->bound_0.vy;
	work->diff.vz = work->bound_1.vz - work->bound_0.vz;

	DG_COPY_VEC( &work->vec, vec );
	DG_COPY_VEC( &work->col, col );
	work->life = life;

	work->alpha_add = (int)(work->col.vw / 16.0f) + 1;
	work->alpha_sub = (int)(work->col.vw / 16.0f) + 1;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS2, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

//	tex = work->tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
	tex = work->tex = DG_GetTexture( tex_name );
	if( !InitPacket2( work, prim, tex, (float)size, mode ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

/*
[ポインタ先保存なし]
FVECTOR *bound0  :
FVECTOR *bound1  :２点で発生する立方体範囲を指定
FVECTOR *vec     :進行方向ベクトル
FVECTOR *col_max :最大到達ＲＧＢα(vx,vy,vz,vw)=(r,g,b,a)
int tex_name     :strcode("texture name")を通した値
int size         :カメラ前に置いた値を基準としているため、試行錯誤が必要。遠方にあるものは小さくなる。
int life         :消滅までのフレーム数
int mode         :０：加算半透明 １：減算半透明 ２：平均半透明
*/
void *NewSpriteFogPers_Demo( FVECTOR *bound0, FVECTOR *bound1, FVECTOR *vec, FVECTOR *col, int tex_name, int size, int life, int mode )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, bound0, bound1, vec, col, tex_name, size, life, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
