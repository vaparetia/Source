//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wave2.c
	新波オブジェクト

	2000/07/19 K.Takabe
	$Id: wave2.c,v 1.1.1.3 2002/11/19 11:51:16 Yoshizawa1 Exp $

*/
/*


・ループ接続処理

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"



/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#ifdef PSX2
#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )
#else
#define _RND(_n)		( (int)( ( (u_long64)rand()*(_n) ) / ((u_long64)RAND_MAX+1) ) )
#endif

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define	MESH_X	( 16 )				/* Ｘ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define MESH_Z	( 16 )				/* Ｚ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define	MESH_DIV_X	( MESH_X - 1 )	/* パッチの分割数（Ｘ軸方向） */
#define MESH_DIV_Z	( MESH_Z - 1 )	/* パッチの分割数（Ｚ軸方向） */
#define MESH_WIDTH_X	( 8000 )	/* メッシュ間隔（Ｘ軸方向） */
#define MESH_WIDTH_Z	( 8000 )	/* メッシュ間隔（Ｚ軸方向） */

#define TRI_ID	(GV_StrCode("wave"))
#define TEX_ID	(GV_StrCode("wave06"))
#define TEX_ID2	(GV_StrCode("wave03"))

#define TEXTURE_SCROLL	(0.0100f)

/* ---------------------------------------------------------------- */
typedef struct _wave_work{
	short	count ;
	short	dcount ;
	short	count2 ;
	short	dcount2 ;
	float	sin ;
	float	blend ;
} WaveWork;

typedef struct _verts_work{
	FVECTOR	pos ;
	FVECTOR	ds ;
	FVECTOR	dt ;
	FVECTOR	uv ;
} VertsWork ;

typedef struct _wave_splash_work{
	int			flag ;
	int			name ;
	CONTROL		*ctrl ;
	int			pad ;
} SPLASH_WORK ;
#define MAX_SPLASH		(8)

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		light[2] ;
	FVECTOR		center ;
	DG_PATCH_VERT	wverts[ MESH_X * MESH_Z ];
	int			name ;
	int			map ;
	DG_PATCH	*patch ;
	DG_PATCH	*patch2 ;
	int			tri_id ;
	int			tex_id ;
	int			tex_id2 ;
	WaveWork	waves[ MESH_X * MESH_Z ] ;
	int			mesh_x, mesh_z ;
	int			div_x, div_y ;
	int			mesh_size_x, mesh_size_z ;
	float		tex_scroll ;
	float		tex_scroll_offset ;
	FVECTOR		tex_scale ;
	FVECTOR		tex_scale2 ;
	FVECTOR		invisible_bound[2]	;			/* 非表示領域指定 */
	int			invisible_enabled ;				/* 非表示領域指定許可フラグ */
	int			count ;

	/* 水飛沫関連 */
	int			n_splash ;
	SPLASH_WORK	splash[MAX_SPLASH];
} Work ;

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/
/* 頂点データ初期化 */
static void InitVertex( Work *work, DG_PATCH_VERT *verts )
{
	WaveWork	*waves ;
	int				i, j ;

	waves = work->waves ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
#if 0
			verts->pos.vx = j * MESH_WIDTH_X - ( MESH_X * MESH_WIDTH_X ) / 2 +
			  _RND( MESH_WIDTH_X/8 ) - MESH_WIDTH_X/16 ;
			verts->pos.vz = i * MESH_WIDTH_Z - ( MESH_Z * MESH_WIDTH_Z ) / 2 +
			  _RND( MESH_WIDTH_Z/2 ) - MESH_WIDTH_Z/4 ;
#else
			verts->pos.vx = j * MESH_WIDTH_X - ( ( MESH_X - 1 ) * MESH_WIDTH_X ) / 2 ;
			verts->pos.vz = i * MESH_WIDTH_Z - ( ( MESH_Z - 1 ) * MESH_WIDTH_Z ) / 2 ;
			if ( j < 7 || j > 8 ){
				verts->pos.vx += _RND( MESH_WIDTH_X/8 ) - MESH_WIDTH_X/16 ;
				verts->pos.vz += _RND( MESH_WIDTH_Z/2 ) - MESH_WIDTH_Z/4 ;
			}
#endif
			verts->pos.vy = 0.0f ;
			verts->pos.vw = 1.0f ;
			verts->uv.vx = j / 4.0f ;
			verts->uv.vy = i / 4.0f  ;
			verts->uv.vz = 1.0f ;
			verts->uv.vw = 128.0f ;
			verts++ ;
			waves++ ;
		}
	}
}
/* アニメーション用パラメータ更新 */
static void ActAnimeParam( Work *work )
{
	WaveWork	*waves ;
	FVECTOR		tmp ;
	int			i, c ;
	float		angle ;

	/* 最終的には計算済みテーブル参照に置き換える予定 */
	waves = work->waves ;
	for ( i = MESH_X * MESH_Z ; i > 1 ; waves += 2, i-=2 ){
		waves[0].count = ( waves[0].count + waves[0].dcount ) & 4095 ;
		waves[0].count2 = ( waves[0].count2 + waves[0].dcount2 ) & 4095 ;
		angle = ( waves[0].count - 2048 ) / 2048.0f * 3.14159265f ;
		tmp.vx = angle ;
#if 0
		//angle = ( waves[0].count2 - 2048 ) / 2048.0f * 3.14159265f ;
#else
		c = waves[0].count2 ; if ( c < 1024 ){ c = ( c - 256 ) & 1023 ; if ( c & 512 ) c -= 1024 ; } else c = -256 ;
		angle = ( c ) / 512.0f * 3.14159265f ;
#endif
		tmp.vy = angle ;
		waves[1].count = ( waves[1].count + waves[1].dcount ) & 4095 ;
		waves[1].count2 = ( waves[1].count2 + waves[1].dcount2 ) & 4095 ;
		angle = ( waves[1].count - 2048 ) / 2048.0f * 3.14159265f ;
		tmp.vz = angle ;
#if 0
		//angle = ( waves[1].count2 - 2048 ) / 2048.0f * 3.14159265f ;
#else
		c = waves[1].count2 ; if ( c < 1024 ){ c = ( c - 256 ) & 1023 ; if ( c & 512 ) c -= 1024 ; } else c = -256 ;
		angle = ( c ) / 512.0f * 3.14159265f ;
#endif
		tmp.vw = angle ;
		MT_SinX4( &tmp, &tmp );
		waves[0].sin = tmp.vx ;
		waves[0].blend = tmp.vy * 0.5f + 0.5f ;
		waves[1].sin = tmp.vz ;
		waves[1].blend = tmp.vw * 0.5f + 0.5f ;
		//waves[0].blend = DG_MAX( 0.0f, tmp.vy * 6.0f - 5.0f ) ;
		//waves[1].blend = DG_MAX( 0.0f, tmp.vw * 6.0f - 5.0f ) ;
		//waves[0].blend = DG_MAX( 0.0f, tmp.vy * 9.0f - 3.0f ) ;
		//waves[1].blend = DG_MAX( 0.0f, tmp.vw * 9.0f - 3.0f ) ;
		//waves[0].blend = DG_MIN( 1.0f, waves[0].blend );
		//waves[1].blend = DG_MIN( 1.0f, waves[1].blend );
	}
	for (  ; i > 0 ; waves++, i-- ){
		waves->count = ( waves->count + waves->dcount ) & 4095 ;
		waves->count2 = ( waves->count2 + waves->dcount2 ) & 4095 ;
		angle = ( waves->count - 2048 ) / 2048.0f * 3.14159265f ;
		tmp.vx = angle ;
#if 0
		//angle = ( waves->count2 - 2048 ) / 2048.0f * 3.14159265f ;
#else
		c = waves->count2 ; if ( c < 1024 ){ c = ( c - 256 ) & 1023 ; if ( c & 512 ) c -= 1024 ; } else c = -256 ;
		angle = ( c ) / 512.0f * 3.14159265f ;
#endif
		tmp.vy = angle ;
		MT_SinX4( &tmp, &tmp );
		waves->sin = tmp.vx ;
		waves->blend = tmp.vy ;
	}
}
/* 頂点アニメーション処理 */
static void ActVertex( Work *work, DG_PATCH_VERT *verts )
{
	WaveWork	*waves ;
	int			i, j ;

	waves = work->waves ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			verts->pos.vy = waves->sin * MESH_WIDTH_X / 10 ;
			verts->uv.vx = j * work->tex_scale.vx * 0.5f ;
			verts->uv.vy = i * work->tex_scale.vy * 0.5f - work->tex_scroll ;
			verts->uv.vw = 128.0f ;
			verts++ ;
			waves++ ;
		}
	}
}
/* 頂点アニメーション処理 */
static void ActVertex2( Work *work, DG_PATCH_VERT *verts )
{
	WaveWork	*waves ;
	int			i, j ;
	static int	a ;
	float		uo, vo ;

	a = ( a + 1 ) & 3 ;
	//uo = (float)( a ) / 4.0f ;
	uo = (float)( _RND( 256 ) ) / 256.0f ;
	vo = (float)( _RND( 256 ) ) / 256.0f ;
	//vo = 0.5f * work->tex_scale2.vy ;
	waves = work->waves ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			verts->uv.vx = j * work->tex_scale2.vx + uo ;
			verts->uv.vy = i * work->tex_scale2.vy + vo ;
			if ( (i+j) & 1 ){
				verts->uv.vw = DG_MIN( 128.0f, ( waves->blend ) * 256.0f ) ;
			} else {
				verts->uv.vw = 0.0f ;
			}
			verts->uv.vw = 100.0f ;
			verts++ ;
			waves++ ;
		}
	}
}
/* 頂点間の傾き計算 */
static void CalcVertsInclination( Work *work, DG_PATCH_VERT *mesh_verts )
{
	int		x, z ;

#if 1//BP_ASM - use C version always - #if 0 || !defined(PSX2)
	/* メッシュの補間パラメータ計算処理 */
	for ( z = 0 ; z < MESH_Z ; z++ ){
		for ( x = 0 ; x < MESH_X ; x++ ){
			DG_PATCH_VERT	*verts, *prev, *next ;
			verts = &mesh_verts[ x + z * ( MESH_DIV_X + 1 ) ] ;
			if ( x == 0 ){
				prev = &mesh_verts[ ( x - 0 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_ds, &next->pos, &prev->pos );
			} else if ( x == MESH_DIV_X ){
				prev = &mesh_verts[ ( x - 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 0 ) + z * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_ds, &next->pos, &prev->pos );
			} else {
				prev = &mesh_verts[ ( x - 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_ds, &next->pos, &prev->pos );
				_sceVu0ScaleVector( &verts->pos_ds, &verts->pos_ds, 0.5f );	
			}
			if ( z == 0 ){
				prev = &mesh_verts[ x + ( z - 0 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 1 ) * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_dt, &next->pos, &prev->pos );
			} else if ( z == MESH_DIV_Z ){
				prev = &mesh_verts[ x + ( z - 1 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 0 ) * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_dt, &next->pos, &prev->pos );
			} else {
				prev = &mesh_verts[ x + ( z - 1 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 1 ) * ( MESH_DIV_X + 1 ) ] ;
				_sceVu0SubVector( &verts->pos_dt, &next->pos, &prev->pos );
				_sceVu0ScaleVector( &verts->pos_dt, &verts->pos_dt, 0.5f );	
			}
		}
	}
#else
	FVECTOR		scale = {0.5,0,0,0};
	asm volatile("lqc2	vf03,0x00(%0)"::"r"(&scale) );

	/* メッシュの補間パラメータ計算処理 */
	for ( z = 0 ; z < MESH_Z ; z++ ){
		for ( x = 0 ; x < MESH_X ; x++ ){
			DG_PATCH_VERT	*verts, *prev, *next ;
			verts = &mesh_verts[ x + z * ( MESH_DIV_X + 1 ) ] ;
			if ( x == 0 ){
				prev = &mesh_verts[ ( x - 0 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				asm volatile("
					lqc2		vf01,0x00(%1)
					lqc2		vf02,0x00(%2)
					vsub.xyzw	vf01,vf01,vf02
					sqc2		vf01,0x00(%0)
				"::"r"(&verts->pos_ds),"r"(&next->pos),"r"(&prev->pos));
			} else if ( x == MESH_DIV_X ){
				prev = &mesh_verts[ ( x - 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 0 ) + z * ( MESH_DIV_X + 1 ) ] ;
				asm volatile("
					lqc2		vf01,0x00(%1)
					lqc2		vf02,0x00(%2)
					vsub.xyzw	vf01,vf01,vf02
					sqc2		vf01,0x00(%0)
				"::"r"(&verts->pos_ds),"r"(&next->pos),"r"(&prev->pos));
			} else {
				prev = &mesh_verts[ ( x - 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ ( x + 1 ) + z * ( MESH_DIV_X + 1 ) ] ;
				asm volatile("
					lqc2		vf01,0x00(%1)
					lqc2		vf02,0x00(%2)
					vmulax.xyzw	ACC ,vf01,vf03
					vmsubx.xyzw	vf01,vf02,vf03
					sqc2		vf01,0x00(%0)
				"::"r"(&verts->pos_ds),"r"(&next->pos),"r"(&prev->pos));
			}
			if ( z == 0 ){
				prev = &mesh_verts[ x + ( z - 0 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 1 ) * ( MESH_DIV_X + 1 ) ] ;
				asm volatile("
					lqc2		vf01,0x00(%1)
					lqc2		vf02,0x00(%2)
					vsub.xyzw	vf01,vf01,vf02
					sqc2		vf01,0x00(%0)
				"::"r"(&verts->pos_dt),"r"(&next->pos),"r"(&prev->pos));
			} else if ( z == MESH_DIV_Z ){
				prev = &mesh_verts[ x + ( z - 1 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 0 ) * ( MESH_DIV_X + 1 ) ] ;
				asm volatile("
					lqc2		vf01,0x00(%1)
					lqc2		vf02,0x00(%2)
					vsub.xyzw	vf01,vf01,vf02
					sqc2		vf01,0x00(%0)
				"::"r"(&verts->pos_dt),"r"(&next->pos),"r"(&prev->pos));
			} else {
				prev = &mesh_verts[ x + ( z - 1 ) * ( MESH_DIV_X + 1 ) ] ;
				next = &mesh_verts[ x + ( z + 1 ) * ( MESH_DIV_X + 1 ) ] ;
				asm volatile("
					lqc2		vf01,0x00(%1)
					lqc2		vf02,0x00(%2)
					vmulax.xyzw	ACC ,vf01,vf03
					vmsubx.xyzw	vf01,vf02,vf03
					sqc2		vf01,0x00(%0)
				"::"r"(&verts->pos_dt),"r"(&next->pos),"r"(&prev->pos));
			}
		}
	}
#endif
}
/* 波頭ブレンド処理 */
static void CrestBlending( Work *work, DG_PATCH_VERT *verts )
{
	WaveWork	*waves ;
	int			i, j ;
	float		t ;
	FVECTOR		tmp, crest, crest_dt ;

#if 0 || !defined(BP_PSX2_ASM)
	waves = work->waves ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			t = waves->blend ;
			if ( (i+j) & 1 ){
				verts->pos.vy = verts->pos.vy * ( 1.0f - t ) + (MESH_WIDTH_Z / 10.0f) * t ;
				verts->pos_dt.vy = verts->pos_dt.vy * ( 1.0f - t ) + (-10.0f) * t ;
				verts->pos_dt.vz = verts->pos_dt.vz * ( 1.0f - t ) + (MESH_WIDTH_Z / 40.0f) * t  ;
			} else {
				verts->pos.vy = verts->pos.vy * ( 1.0f - t ) + (-MESH_WIDTH_Z / 10.0f) * t ;
				verts->pos_dt.vy = verts->pos_dt.vy * ( 1.0f - t ) + (MESH_WIDTH_Z / 4.0f) * t ;
				verts->pos_dt.vz = verts->pos_dt.vz * ( 1.0f - t ) + (MESH_WIDTH_Z) * t  ;
			}
			//verts->uv.vw = 128 * t ;
			verts++ ;
			waves++ ;
		}
	}
#else
	/* 混ぜ合わせるベクトルをＶＵレジスタへ設定 */
	crest.vy = MESH_WIDTH_Z / 10.0f ;
	crest_dt.vy = -10.0f ;
	crest_dt.vz = MESH_WIDTH_Z / 40.0f ;
	asm volatile("
		lqc2		vf04,0x00(%0)
		lqc2		vf05,0x00(%1)
	"::"r"(&crest),"r"(&crest_dt):"memory");
	crest.vy = -MESH_WIDTH_Z / 10.0f ;
	crest_dt.vy = MESH_WIDTH_Z / 4.0f ;
	crest_dt.vz = MESH_WIDTH_Z ;
	asm volatile("
		lqc2		vf06,0x00(%0)
		lqc2		vf07,0x00(%1)
	"::"r"(&crest),"r"(&crest_dt):"memory");

	waves = work->waves ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			t = waves->blend ;
			tmp.vx = t ;
			if ( (i+j) & 1 ){
				asm volatile("
					lqc2		vf01,0x00(%0)
					lqc2		vf02,0x00(%1)
					lqc2		vf03,0x00(%2)
					vaddax.y	ACC ,vf01,vf00
					vmsubax.y	ACC ,vf01,vf03
					vmaddx.y	vf01,vf04,vf03
					vaddax.yz	ACC ,vf02,vf00
					vmsubax.yz	ACC ,vf02,vf03
					vmaddx.yz	vf02,vf05,vf03
					sqc2		vf01,0x00(%0)
					sqc2		vf02,0x00(%1)
				"::"r"(&verts->pos),"r"(&verts->pos_dt),"r"(&tmp) );
			} else {
				asm volatile("
					lqc2		vf01,0x00(%0)
					lqc2		vf02,0x00(%1)
					lqc2		vf03,0x00(%2)
					vaddax.y	ACC ,vf01,vf00
					vmsubax.y	ACC ,vf01,vf03
					vmaddx.y	vf01,vf06,vf03
					vaddax.yz	ACC ,vf02,vf00
					vmsubax.yz	ACC ,vf02,vf03
					vmaddx.yz	vf02,vf07,vf03
					sqc2		vf01,0x00(%0)
					sqc2		vf02,0x00(%1)
				"::"r"(&verts->pos),"r"(&verts->pos_dt),"r"(&tmp) );
			}
			//verts->uv.vw = 128 * t ;
			verts++ ;
			waves++ ;
		}
	}
#endif
}

/* テクスチャ情報をリージョンリピートテクスチャに設定する */
static void TextureRepeatSet( DG_TEX *tex )
{
	int		tw, th, w, h, tx, ty, ofx, ofy, tmp ;
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	w = 1 << tw ;
	h = 1 << th ;
	tmp = w * tex->u_scale ;
	tx = 1 ; while ( tx < tmp ) tx <<= 1 ;
	tmp = h * tex->v_scale ;
	ty = 1 ; while ( ty < tmp ) ty <<= 1 ;
	ofx = w * tex->u_offset ;
	ofx &= ~( tx - 1 );
	ofy = h * tex->v_offset ;
	ofy &= ~( ty - 1 );
	tex->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 3, 3, tx-1, ofx, ty-1, ofy );
}
/* テクスチャのスケーリング値を取得する */
static void GetTextureScale( DG_TEX *tex, FVECTOR *scale )
{
	int		tw, th, w, h, tx, ty, tmp ;
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	w = 1 << tw ;
	h = 1 << th ;
	tmp = w * tex->u_scale ;
	tx = 1 ; while ( tx < tmp ) tx <<= 1 ;
	tmp = h * tex->v_scale ;
	ty = 1 ; while ( ty < tmp ) ty <<= 1 ;
	scale->vx = 1.0f / ( w / tx ) ;
	scale->vy = 1.0f / ( h / ty ) ;
}

static void InvisiblePatchParts( Work *work, DG_PATCH *patch )
{
	static FVECTOR	min_def = {99999999999,99999999999,99999999999,1.0} ;
	static FVECTOR	max_def = {-99999999999,-99999999999,-99999999999,1.0} ;
	FVECTOR		max, min, verts[8], *v ;
	DG_PATCH_PARTS	*parts ;
	int			i, j ;

	if ( !work->invisible_enabled ) return ;
	//printf("%f %f %f %f %f %f\n",
	//	   work->invisible_bound[0].vx, work->invisible_bound[0].vy, work->invisible_bound[0].vz,
	//	   work->invisible_bound[1].vx, work->invisible_bound[1].vy, work->invisible_bound[1].vz );

	if ( patch->root == NULL ){
		DG_SetPos( &patch->world );
	} else {
		DG_SetPos( patch->root );
	}

	/* パッチ毎に非表示領域に入っているかチェックする */
	parts = patch->parts ;
	for ( i = 0 ; i < patch->n_patch ; parts++, i++ ){
		/* ８頂点に分解して座標変換を行なう */
		v = verts ;
		for ( j = 0 ; j < 8 ; v++, j++ ){
			v->vx = ( j & 1 ) ? parts->max.vx : parts->min.vx ;
			v->vy = ( j & 2 ) ? parts->max.vy : parts->min.vy ;
			v->vz = ( j & 4 ) ? parts->max.vz : parts->min.vz ;
			v->vw = 1.0f ;
		}
		DG_PutVector( verts, verts, 8 );
		/* 変換後の最大・最小座標を求める */
		max = max_def ;
		min = min_def ;
		v = verts ;
		for ( j = 0 ; j < 8 ; v++, j++ ){
			max.vx = DG_MAX( max.vx, v->vx );
			max.vy = DG_MAX( max.vy, v->vy );
			max.vz = DG_MAX( max.vz, v->vz );
			min.vx = DG_MIN( min.vx, v->vx );
			min.vy = DG_MIN( min.vy, v->vy );
			min.vz = DG_MIN( min.vz, v->vz );
		}
		/* チェック */
		if ( work->invisible_bound[0].vx < min.vx &&
			work->invisible_bound[0].vy < min.vy &&
			work->invisible_bound[0].vz < min.vz &&
			work->invisible_bound[1].vx > max.vx &&
			work->invisible_bound[1].vy > max.vy &&
			work->invisible_bound[1].vz > max.vz ){
			parts->flag |= 0x0100 ;
		}
	}
}

/* エルミート補間用マトリクス生成 */
static void MakeHermiteMatrix( FMATRIX *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1 )
{
	static FMATRIX c_mat2 = {{
		{ 2.0f,-2.0f, 1.0f, 1.0f},
		{-3.0f, 3.0f,-2.0f,-1.0f},
		{ 0.0f, 0.0f, 1.0f, 0.0f},
		{ 1.0f, 0.0f, 0.0f, 0.0f}
	}};
	FMATRIX	data_mat ;
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;
	_sceVu0MulMatrix( res, &data_mat, &c_mat2 );
	//_sceVu0ApplyMatrix( &ans, res, &tvec );
}
static void LerpVecFpu( FVECTOR *res, FVECTOR *v1, FVECTOR *v2, float t )
{
#if 0
	res->vx = ( v2->vx - v1->vx ) * t + v1->vx ;
	res->vy = ( v2->vy - v1->vy ) * t + v1->vy ;
	res->vz = ( v2->vz - v1->vz ) * t + v1->vz ;
#else
	/* X = x1 - ( x1 * t ) + ( x2 * t ) = ( x2 - x1 ) * t + x1 */
	DG_ADDA( v1->vx, 0.0f );
	DG_MSUBA( v1->vx, t );
	res->vx = DG_MADD( v2->vx, t );
	/* Y */
	DG_ADDA( v1->vy, 0.0f );
	DG_MSUBA( v1->vy, t );
	res->vy = DG_MADD( v2->vy, t );
	/* Z */
	DG_ADDA( v1->vz, 0.0f );
	DG_MSUBA( v1->vz, t );
	res->vz = DG_MADD( v2->vz, t );
#endif
}

/* 指定した座標における水面のおおよその位置を取得する */
static int GetWaveHeight( FVECTOR *res, FMATRIX *world, DG_PATCH_PARTS *parts, DG_PATCH_VERT *wverts, FVECTOR *org_pos )
{
	static FVECTOR	t_vec[] = {
		{ 1.0,            1.0       , 1.0   , 1.0 },		/* t=16/16*/
		{ 0.823974609375, 0.87890625, 0.9375, 1.0 },		/* t=15/16 */
		{ 0.669921875   , 0.765625  , 0.875 , 1.0 },		/* t=14/16 */
		{ 0.536376953125, 0.66015625, 0.8125, 1.0 },		/* t=13/16 */
		{ 0.421875      , 0.5625    , 0.75  , 1.0 },		/* t=12/16 */
		{ 0.324951171875, 0.47265625, 0.6875, 1.0 },		/* t=11/16 */
		{ 0.244140625   , 0.390625  , 0.625 , 1.0 },		/* t=10/16 */
		{ 0.177978515625, 0.31640625, 0.5625, 1.0 },		/* t= 9/16 */
		{ 0.125         , 0.25      , 0.5   , 1.0 },		/* t= 8/16 */
		{ 0.083740234375, 0.19140625, 0.4375, 1.0 },		/* t= 7/16 */
		{ 0.052734375   , 0.140625  , 0.375 , 1.0 },		/* t= 6/16 */
		{ 0.030517578125, 0.09765625, 0.3125, 1.0 },		/* t= 5/16 */
		{ 0.015625      , 0.0625    , 0.25  , 1.0 },		/* t= 4/16 */
		{ 0.006591796875, 0.03515625, 0.1875, 1.0 },		/* t= 3/16 */
		{ 0.001953125   , 0.015625  , 0.125 , 1.0 },		/* t= 2/16 */
		{ 0.000244140625, 0.00390625, 0.0625, 1.0 },		/* t= 1/16 */
		{ 0.0           , 0.0       , 0.0   , 1.0 },		/* t= 0/16 */
	};
	int			i, j, ts, tt ;
	DG_PATCH_VERT	*v0, *v1, *v2, *v3 ;
	FVECTOR		pv0, pv1, pv2, pv3 ;
	FVECTOR		p0, p1, dp0, dp1, *t0, *t1 ;
	FMATRIX		t0_interp_mat, t1_interp_mat, s0_interp_mat, s1_interp_mat ;
	FVECTOR		pos ;

	/* warningを取るための変数初期化（内容は意味無し） */
	s1_interp_mat = DG_UnitMatrix ;
	pv1 = pv3 = DG_ZeroVector ;

	{/* 相対座標取得 */
		FMATRIX	inv_mat ;
		pos = *org_pos ;
		pos.vw = 1.0f ;
		_sceVu0InversMatrix( &inv_mat, world );
		_sceVu0ApplyMatrix( &pos, &inv_mat, &pos );
	}
	for ( i = 0 ; i < ( MESH_Z - 1 ) ; i++ ){
		v0 = &wverts[ ( i + 0 ) * MESH_X + ( 0 ) ] ;
		v1 = &wverts[ ( i + 0 ) * MESH_X + ( 1 ) ] ;
		v2 = &wverts[ ( i + 1 ) * MESH_X + ( 0 ) ] ;
		v3 = &wverts[ ( i + 1 ) * MESH_X + ( 1 ) ] ;
		for ( j = 0 ; j < ( MESH_X - 1 ) ; parts++, v0++, v1++, v2++, v3++, j++ ){
			/* バウンディングチェック */
			if ( parts->max.vx > pos.vx /* && parts->max.vy > pos.vy */ && parts->max.vz > pos.vz &&
				parts->min.vx < pos.vx /* && parts->min.vy < pos.vy */ && parts->min.vz < pos.vz ){
				/* ある程度の粗さで分割した各微少面毎に判定を行なう */
				for ( tt = 0 ; tt <= 16 ; tt+=4 ){
					s0_interp_mat = s1_interp_mat ;
					MakeHermiteMatrix( &t0_interp_mat, &v0->pos, &v2->pos, &v0->pos_dt, &v2->pos_dt );
					MakeHermiteMatrix( &t1_interp_mat, &v1->pos, &v3->pos, &v1->pos_dt, &v3->pos_dt );
					t0 = &t_vec[ tt ] ;
					_sceVu0ApplyMatrix( &p0, &t0_interp_mat, t0 );/* エルミート補間 */
					_sceVu0ApplyMatrix( &p1, &t1_interp_mat, t0 );/* エルミート補間 */
					LerpVecFpu( &dp0, &v0->pos_ds, &v2->pos_ds, t0->vz );
					LerpVecFpu( &dp1, &v1->pos_ds, &v3->pos_ds, t0->vz );
					MakeHermiteMatrix( &s1_interp_mat, &p0, &p1, &dp0, &dp1 );
					if ( tt == 0 ) continue ;
					for ( ts = 0 ; ts <= 16 ; ts+=4 ){
						float	f ;
						FVECTOR	l0, l1, n ;
						t1 = &t_vec[ ts ] ;
						pv0 = pv1 ;
						pv2 = pv3 ;
						_sceVu0ApplyMatrix( &pv1, &s0_interp_mat, t1 );/* エルミート補間 */
						_sceVu0ApplyMatrix( &pv3, &s1_interp_mat, t1 );/* エルミート補間 */
						if ( ts == 0 ) continue ;
						/* ＸＺ平面におけるチェックを行なう（手抜き処理） */
						_sceVu0SubVector( &l0, &pv1, &pv0 );
						_sceVu0SubVector( &l1, &pos, &pv0 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &l0, &pv3, &pv1 );
						_sceVu0SubVector( &l1, &pos, &pv1 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &l0, &pv2, &pv3 );
						_sceVu0SubVector( &l1, &pos, &pv3 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &l0, &pv0, &pv2 );
						_sceVu0SubVector( &l1, &pos, &pv2 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &n, &pv1, &pv0 );
						_sceVu0OuterProduct( &n, &n, &l0 );
						_sceVu0Normalize( &n, &n );
						f = _sceVu0InnerProduct( &n, &l1 );
						pos.vy -= f ;
						pos.vw = 1.0f ;
						_sceVu0ApplyMatrix( res, world, &pos );
						//printf("wave2.c: f = %f\n", f);
						if ( f < 0 ){
							/* 水面下 */
							return ( 1 );
						} else {
							/* 水面上 */
							return ( 0 );
						}
					}
				}
			}
		}
	}
	//printf("wave2.c:region error\n");
	return ( 0 );
}

/* ---------------------------------------------------------------- */
/* 指定した名前のコントロールを検索 */
static CONTROL* SearchControl( Work *work, int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		if ( ( control->map & work->map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("wave2.c:search faild control !!\n");
	return ( NULL );
}


/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	DG_PATCH_VERT	*wverts = (void*)SCRPAD_ADDR ;
	GV_MSG		*msg ;
	int			time, n_msg, i, j ;

	/* 頂点をスクラッチパッドへ */
	//GV_SET_PRFC_CLOCK();
	DG_StartMemToSpr( wverts, work->wverts, MESH_X * MESH_Z * 4 );
	DG_EndMemToSpr();
	//time = GV_GET_PRFC_CLOCK();
	//printf("wave2.c: %d ", time);

	/* 通常アニメーション用パラメータ変更 */
	//GV_SET_PRFC_CLOCK();
	ActAnimeParam( work );
	//time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	/* 通常上下運動アニメーションの頂点計算 */
	//GV_SET_PRFC_CLOCK();
	ActVertex( work, wverts );
	//time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	/* 頂点変化傾き計算 */
	//GV_SET_PRFC_CLOCK();
	CalcVertsInclination( work, wverts );
	//time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	/* 波頭部分のブレンディング */
	//GV_SET_PRFC_CLOCK();
	CrestBlending( work, wverts );
	//time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);
	wverts[ 7 + 10 * MESH_X ].pos.vx = -5000.0f ;
	wverts[ 7 + 10 * MESH_X ].pos.vy =  0.0f ;
	wverts[ 7 + 10 * MESH_X ].pos.vz = 19500.0f ;
	wverts[ 7 + 10 * MESH_X ].pos_ds.vx = 3000.0f ;
	wverts[ 7 + 10 * MESH_X ].pos_ds.vy = -3000.0f ;
	wverts[ 7 + 10 * MESH_X ].pos_ds.vz = 0.0f ;
	wverts[ 7 + 10 * MESH_X ].pos_dt.vx = 1000.0f ;
	wverts[ 7 + 10 * MESH_X ].pos_dt.vy = 0.0f ;
	wverts[ 7 + 10 * MESH_X ].pos_dt.vz = 8000.0f ;
	wverts[ 7 + 11 * MESH_X ].pos.vx = -5000.0f ;
	//wverts[ 7 + 11 * MESH_X ].pos.vy =  0.0f ;
	wverts[ 7 + 11 * MESH_X ].pos.vz = 28000.0f ;

	wverts[ 8 + 10 * MESH_X ].pos.vx =  5000.0f ;
	wverts[ 8 + 10 * MESH_X ].pos.vy =  0.0f ;
	wverts[ 8 + 10 * MESH_X ].pos.vz = 19500.0f ;
	wverts[ 8 + 10 * MESH_X ].pos_ds.vx = 3000.0f ;
	wverts[ 8 + 10 * MESH_X ].pos_ds.vy = 0.0f ;
	wverts[ 8 + 10 * MESH_X ].pos_ds.vz = 0.0f ;
	wverts[ 8 + 10 * MESH_X ].pos_dt.vx = -1000.0f ;
	wverts[ 8 + 10 * MESH_X ].pos_dt.vy = 0.0f ;
	wverts[ 8 + 10 * MESH_X ].pos_dt.vz = 8000.0f ;
	wverts[ 8 + 11 * MESH_X ].pos.vx =  5000.0f ;
	//wverts[ 8 + 11 * MESH_X ].pos.vy =  0.0f ;
	wverts[ 8 + 11 * MESH_X ].pos.vz = 28000.0f ;

	DG_SwitchBuffPatch( work->patch );
	DG_SwitchBuffPatch( work->patch2 );

	/* 頂点データの書き戻し */
	//GV_SET_PRFC_CLOCK();
	FlushCache( 0 );
	DG_StartSprToMem( work->patch->verts[ work->patch->buffer_clock ], wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	//time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	/* 波頭プレーン用のアニメーション処理 */
	ActVertex2( work, wverts );
	FlushCache( 0 );
	DG_StartSprToMem( work->patch2->verts[ work->patch->buffer_clock ], wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();

	/* 頂点データの書き戻し */
	//GV_SET_PRFC_CLOCK();
	FlushCache( 0 );
	DG_StartSprToMem( work->wverts, wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	//time = GV_GET_PRFC_CLOCK();
	//printf(" %d \n", time );

	/* テクスチャスクロールオフセット更新 */
	work->tex_scroll += TEXTURE_SCROLL * work->tex_scale.vy ;
	if ( work->tex_scroll > 1.0f ) work->tex_scroll -= 1.0f ;

	/*
		水飛沫関連処理
	*/
	/* メッセージチェック */
	if ( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		CONTROL	*ctrl ;
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
			  case 0:/* チェック開始 */
				if ( ( ctrl = SearchControl( work, msg->message[1] ) ) == NULL ) break ;
				/* 名前重複チェック */
				for ( i = 0 ; i < work->n_splash ; i++ ){
					if ( work->splash[i].name == msg->message[1] ) break ;
				}
				/* ワークオーバーチェック */
				if ( work->n_splash >= MAX_SPLASH ){
					/* オーバーしていたら古いものから消す */
					for ( i = 1 ; i < work->n_splash ; i++ ){
						work->splash[i-1] = work->splash[i] ;
					}
				}
				work->splash[work->n_splash].ctrl = ctrl ;
				work->splash[work->n_splash].name = msg->message[1] ;
				work->n_splash++ ;
				printf("wave2.c:add chara %d\n", msg->message[1]);
				break ;
			}
		}
	}
	//printf("chech start\n");
	/* 水しぶき監視メイン処理 */
	for ( i = 0 ; i < work->n_splash ; i++ ){
		FVECTOR		pos ;
		CONTROL		*ctrl ;
		/* 念のため毎回コントロールを検索する */
		ctrl = SearchControl( work, work->splash[i].name );
		//printf("wave2.c: no=%d ctrl=%d(%08x)\n", i, work->splash[i].name, ctrl );
		if ( ctrl != NULL ){
			/* 水面との当りをチェック */
			if ( GetWaveHeight( &pos, &work->patch->world, work->patch->parts, wverts, &ctrl->mov ) ){
				printf("wave2.c: splash on!!(%d)\n", work->splash[i].name );
				/* 水飛沫発生処理 */
				{
					extern void *NewDiveSplash( FVECTOR *center, int pattern );
					NewDiveSplash( &pos, 0 );
					NewDiveSplash( &pos, 0 );
					NewDiveSplash( &pos, 0 );
					NewDiveSplash( &pos, 0 );
					NewDiveSplash( &pos, 1 );
					NewDiveSplash( &pos, 1 );
					NewDiveSplash( &pos, 1 );
					NewDiveSplash( &pos, 1 );
				}
				
				/* ワークの開放処理 */
				for ( j = i ; j < ( work->n_splash - 1 ) ; j++ ){
					//printf("  %d,%d = %d\n", j, work->splash[j].name, work->splash[j+1].name );
					work->splash[j] = work->splash[j+1] ;
				}
				/* 参照位置の補正 */
				i-- ;
				work->n_splash-- ;
			}
			//printf("wave2.c: %d (y = %f) pos:%f,%f,%f\n", ctrl->name, ctrl->mov.vy, pos.vx, pos.vy, pos.vz );
		} else {
			/* コントロールが見つからなかった場合には監視チェックをはずす */
			/* ワークの開放処理 */
			for ( j = i ; j < ( work->n_splash - 1 ) ; j++ ){
				work->splash[j] = work->splash[j+1] ;
			}
			/* 参照位置の補正 */
			i-- ;
			work->n_splash-- ;
		}
	}

#ifdef KP_XBOX
	/* XBOXでは重くてゲームに支障をきたすためＬＯＤレベルを変更 */
	if ( GM_GameStatus & STATE_DEMO ){
		/* デモ中なら従来と同じに */
		DG_ConfigPatchLOD( work->patch, 13 );
		DG_ConfigPatchLOD( work->patch2, 13 );
		work->patch->max_level = 0 ;
		work->patch2->max_level = 0 ;
	} else {
		/* ゲーム中なら荒く */
		DG_ConfigPatchLOD( work->patch, 8 );
		DG_ConfigPatchLOD( work->patch2, 8 );
		work->patch->max_level = 3 ;
		work->patch2->max_level = 3 ;
	}
#endif

}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	DG_DequeuePatchObjs( work->patch );
	DG_FreePatch( work->patch );
	DG_DequeuePatchObjs( work->patch2 );
	DG_FreePatch( work->patch2 );
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_TEX		*tex ;
	DG_PATCH	*patch ;
	int		i, j ;
	FVECTOR		pos ;
	DG_PATCH_VERT	*wverts = (void*)SCRPAD_ADDR ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	/* テクスチャ設定読み込み */
	if ( GCL_GetOption( 't' ) != NULL ) {
		work->tri_id = GCL_GetNextInt() ;
		work->tex_id = GCL_GetNextInt() ;
		work->tex_id2 = GCL_GetNextInt() ;
	} else {
		work->tri_id = TRI_ID ;
		work->tex_id = TEX_ID ;
		work->tex_id2 = TEX_ID2 ;
	}

	/* 設置座標読み込み */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		pos.vx = (float)GCL_GetNextInt() ;
		pos.vy = (float)GCL_GetNextInt() ;
		pos.vz = (float)GCL_GetNextInt() ;
	} else {
		pos = DG_ZeroVector ;
	}

	/* 非表示領域設定 */
	if ( GCL_GetOption( 'i' ) != NULL ) {
		work->invisible_bound[0].vx = (float)GCL_GetNextInt() ;
		work->invisible_bound[0].vy = (float)GCL_GetNextInt() ;
		work->invisible_bound[0].vz = (float)GCL_GetNextInt() ;
		work->invisible_bound[1].vx = (float)GCL_GetNextInt() ;
		work->invisible_bound[1].vy = (float)GCL_GetNextInt() ;
		work->invisible_bound[1].vz = (float)GCL_GetNextInt() ;
		work->invisible_bound[0].vw = work->invisible_bound[1].vw = 1.0f ;
		work->invisible_enabled = 1 ;
	}

	/* テクスチャ取得 */
	tex = DG_GetTexture2( work->tri_id, work->tex_id );
	/* 強制的にリピートテクスチャにする */
	//tex->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 3, 3, 0x7f, 0x00, 0x7f, 0x00 );
	TextureRepeatSet( tex );
	GetTextureScale( tex, &work->tex_scale );

	/* オブジェクト作成 */
	work->patch = patch = DG_MakePatchMesh( 0, MESH_X, MESH_Z );
	GM_GroupObject( patch, work->map );
	DG_QueuePatchObjs( patch );
	DG_ConfigPatchLOD( patch, 13 );
	/* テクスチャ設定 */
	patch->tri_id = work->tri_id ;
	patch->tex = tex ;
	/* 座標設定 */
	patch->world.m[3][0] = pos.vx ;
	patch->world.m[3][1] = pos.vy ;
	patch->world.m[3][2] = pos.vz ;
	/* ライト設定 */
	work->light[0] = patch->light[0] ;
	work->light[1] = patch->light[1] ;
	patch->light = work->light ;

	/* ローカルライトカラー設定 */
	if ( GCL_GetOption( 'c' ) != NULL ) {
		work->light[1].m[0][0] = (float)GCL_GetNextInt() ;
		work->light[1].m[0][1] = (float)GCL_GetNextInt() ;
		work->light[1].m[0][2] = (float)GCL_GetNextInt() ;
	}
	/* ローカルライト方向設定 */
	if ( GCL_GetOption( 'd' ) != NULL ) {
		FVECTOR		tmp ;
		tmp.vx = (float)GCL_GetNextInt();
		tmp.vy = (float)GCL_GetNextInt();
		tmp.vz = (float)GCL_GetNextInt();
		_sceVu0Normalize( &tmp, &tmp );
		work->light[0].m[0][0] = tmp.vx ;
		work->light[0].m[1][0] = tmp.vy ;
		work->light[0].m[2][0] = tmp.vz ;
	}
	/* ローカルアンビエント設定 */
	if ( GCL_GetOption( 'a' ) != NULL ) {
		work->light[1].m[3][0] = (float)GCL_GetNextInt() ;
		work->light[1].m[3][1] = (float)GCL_GetNextInt() ;
		work->light[1].m[3][2] = (float)GCL_GetNextInt() ;
	}
#if 0
	work->light[1].m[0][0] /= 4.0f ;
	work->light[1].m[0][1] /= 4.0f ;
	work->light[1].m[0][2] /= 4.0f ;
	work->light[1].m[3][0] = 0.0f ;
	work->light[1].m[3][1] = 0.0f ;
	work->light[1].m[3][2] = 0.0f ;
#endif

	{/* 波頂点アニメーションパラメータ初期化 */
		WaveWork	*waves ;
		waves = work->waves ;
		for ( i = MESH_X * MESH_Z ; i > 0 ; waves++, i-- ){
			waves->count = _RND( 4096 );
			waves->dcount = _RND( 10 ) + 22 ;
			waves->count2 = _RND( 4096 );
			//waves->dcount2 = _RND( 20 ) + 30 ;
			waves->dcount2 = _RND( 2 ) + 2 ;
			//waves->dcount2 = _RND( 8 ) + 6 ;
		}
		for ( i = 0 ; i < MESH_Z ; i++ ){
			for ( j = 0 ; j < MESH_X ; j++ ){
				if ( i == 0 ){
					if ( !( ( i + j ) & 1 ) ){
						work->waves[ j + i * MESH_X ].count2 = work->waves[ j + ( MESH_Z - 1 ) * MESH_X ].count2 ;
						work->waves[ j + i * MESH_X ].dcount2 = work->waves[ j + ( MESH_Z - 1 ) * MESH_X ].dcount2 ;
					}
				} else {
					if ( !( ( i + j ) & 1 ) ){
						work->waves[ j + i * MESH_X ].count2 = work->waves[ j + ( i - 1 ) * MESH_X ].count2 ;
						work->waves[ j + i * MESH_X ].dcount2 = work->waves[ j + ( i - 1 ) * MESH_X ].dcount2 ;
					}
				}
			}
		}
	}

	/* 頂点初期化 */
	InitVertex( work, wverts );
	CalcVertsInclination( work, wverts );
	FlushCache( 0 );
	DG_StartSprToMem( work->patch->verts[ 0 ], wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	DG_StartSprToMem( work->patch->verts[ 1 ], wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	DG_StartSprToMem( work->wverts, wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	DG_SetupPatchMeshBounding( patch );


	/* テクスチャ取得 */
	tex = DG_GetTexture2( work->tri_id, work->tex_id2 );
	/* 強制的にリピートテクスチャにする */
	TextureRepeatSet( tex );
	tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0,1,0,1,0 );
	GetTextureScale( tex, &work->tex_scale2 );

	/* オブジェクト作成 */
	work->patch2 = patch = DG_MakePatchMesh( 0, MESH_X, MESH_Z );
	GM_GroupObject( patch, work->map );
	DG_QueuePatchObjs( patch );
	DG_ConfigPatchLOD( patch, 13 );
	patch->tri_id = work->tri_id ;
	patch->tex = tex ;
	patch->world.m[3][0] = pos.vx ;
	patch->world.m[3][1] = pos.vy ;
	patch->world.m[3][2] = pos.vz ;
	patch->light = work->light ;

	FlushCache( 0 );
	DG_StartSprToMem( work->patch2->verts[ 0 ], wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	DG_StartSprToMem( work->patch2->verts[ 1 ], wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	DG_SetupPatchMeshBounding( patch );

	InvisiblePatchParts( work, work->patch );
	InvisiblePatchParts( work, work->patch2 );

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewStormyRiverWaveSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

#if 0
	/*
		プログラム起動インターフェイス
	*/
void *NewStormyRiverWave( int name, FMATRIX *world, int model )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, GM_CurrentMap, 1, world, model ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
#endif
