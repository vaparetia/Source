//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wave3.c
	新波オブジェクト

	2001/01/15 K.Takabe
	$Id: wave3.c,v 1.1.1.3 2002/11/19 11:51:16 Yoshizawa1 Exp $

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



/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#ifdef PSX2
#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )
#else
#define _RND(_n)		( (int)( ( (unsigned int)rand()*(_n) ) / ((unsigned int)RAND_MAX+1) ) )
#endif

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define	MESH_X	( 16 )				/* Ｘ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define MESH_Z	( 16 )				/* Ｚ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define	MESH_DIV_X	( MESH_X - 1 )	/* パッチの分割数（Ｘ軸方向） */
#define MESH_DIV_Z	( MESH_Z - 1 )	/* パッチの分割数（Ｚ軸方向） */
#define MESH_WIDTH_X	( 1250/2 )	/* メッシュ間隔（Ｘ軸方向） */
#define MESH_WIDTH_Z	( 1250/2 )	/* メッシュ間隔（Ｚ軸方向） */
#define PATCH_X	(3)					/* パッチ配置数 */
#define PATCH_Z	(3)					/* パッチ配置数 */

#define TRI_ID	(GV_StrCode("wave"))
#define TEX_ID	(GV_StrCode("wave06"))
#define TEX_ID2	(GV_StrCode("wave03"))

#define TEXTURE_SCROLL	(0.0100f)

/* ---------------------------------------------------------------- */
typedef struct _wave_work{
	float		height ;
	float		force ;
} WaveWork;

typedef struct _patch_work{
	DG_PATCH		*patch ;
	DG_PATCH_VERT	*verts_backup[2] ;
	DG_PATCH_PARTS	*parts_backup ;
} PatchWork ;

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
	float			force[ MESH_X * MESH_Z ] ;
	int			name ;
	int			map ;
	DG_PATCH	*patch ;
	PatchWork	patchs[ PATCH_X * PATCH_Z ] ;
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
	int				i, j ;

	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			verts->pos.vx = j * MESH_WIDTH_X - ( ( MESH_X - 1 ) * MESH_WIDTH_X ) / 2 ;
			verts->pos.vz = i * MESH_WIDTH_Z - ( ( MESH_Z - 1 ) * MESH_WIDTH_Z ) / 2 ;
			verts->pos.vy = 0.0f ;
			verts->pos.vw = 1.0f ;
			verts->pos_ds.vx = MESH_WIDTH_X ;
			verts->pos_ds.vz = 0.0f ;
			verts->pos_ds.vy = 0.0f ;
			verts->pos_ds.vw = 1.0f ;
			verts->pos_dt.vx = 0.0f ;
			verts->pos_dt.vz = MESH_WIDTH_Z ;
			verts->pos_dt.vy = 0.0f ;
			verts->pos_dt.vw = 1.0f ;
			verts->uv.vx = j / 4.0f ;
			verts->uv.vy = i / 4.0f  ;
			verts->uv.vz = 1.0f ;
			verts->uv.vw = 128.0f ;
			work->force[ j + i * ( MESH_X ) ] = _RND(MESH_WIDTH_Z/32) - MESH_WIDTH_Z/64 ;
			work->waves[ j + i * ( MESH_X ) ].force = _RND(MESH_WIDTH_Z/32) - MESH_WIDTH_Z/64 ;
			verts++ ;
		}
	}
}
/* アニメーション用パラメータ更新 */
static void ActAnimeParam( Work *work )
{
	WaveWork	*waves ;
	int		i, j, x, z, offset ;
	int	px, nx, pz, nz, flag ;
	float	ave ;

	waves = work->waves ;
	offset = MESH_X ;

	ave = 0 ;
	pz = ( MESH_DIV_Z - 1 ) * offset ;	/* １つ前の頂点オフセット */
	z = 0  ;					/* 処理中の頂点オフセット */
	nz = offset ;				/* １つ次の頂点オフセット */
	for ( i = 0, z = 0 ; i < ( MESH_DIV_Z ) ; i++ ){
		px = MESH_DIV_X - 1;		/* １つ前の頂点オフセット */
		x = 0 ;					/* 処理中の頂点オフセット */
		nx = 1 ;				/* １つ次の頂点オフセット */
		for ( j = 0 ; j < ( MESH_DIV_X ) ; j++ ){
			float		h, f ;

			/* 隣接する高さからの影響を計算する */
#if !defined( PSX2 )
			{
				float	h1, h2 ;
				h1 = ( waves[ x + nz ].height + waves[ x + pz ].height +
					  waves[ nx + z ].height + waves[ px + z ].height ) * ( 2.0f/12.0f ) ;
				h2 = ( waves[ nx + nz ].height + waves[ nx + pz ].height +
					  waves[ px + nz ].height + waves[ px + pz ].height ) * ( 1.0f/12.0f ) ;
				h = h1 + h2 ;
			}
#else
			DG_MULA(  2.0f/12.0f, waves[ x + nz ].height );		/* 垂直隣接成分 */
			DG_MADDA( 2.0f/12.0f, waves[ x + pz ].height );		/* 垂直隣接成分 */
			DG_MADDA( 2.0f/12.0f, waves[ nx + z ].height );		/* 垂直隣接成分 */
			DG_MADDA( 2.0f/12.0f, waves[ px + z ].height );		/* 垂直隣接成分 */
			DG_MADDA( 1.0f/12.0f, waves[ nx + nz ].height );	/* 斜め隣接成分 */
			DG_MADDA( 1.0f/12.0f, waves[ nx + pz ].height );	/* 斜め隣接成分 */
			DG_MADDA( 1.0f/12.0f, waves[ px + nz ].height );	/* 斜め隣接成分 */
			h = DG_MADD( 1.0f/12.0f, waves[ px + pz ].height );	/* 斜め隣接成分 */
#endif
			f = h - waves[ x + z ].height ;
			f = waves[ x + z ].force + f * 0.20f ;
			if ( f > 3.0f ) f *= 0.90f ;
			waves[ x + z ].force = f ;

			/* 平均値取得用計算 */
			ave += waves[ x + z ].height ;

			/* 頂点オフセットインクリメント */
			px++ ;
			x++ ;
			nx++ ;
			if ( px >= MESH_DIV_X ) px = 0 ;
			if ( nx >= MESH_DIV_X ) nx = 0 ;
		}
		/* 頂点オフセットインクリメント */
		pz += offset ;
		z += offset ;
		nz += offset ;
		if ( pz >= MESH_DIV_Z * offset ) pz = 0 ;
		if ( nz >= MESH_DIV_Z * offset ) nz = 0 ;
	}
	ave *= 1.0f / ( MESH_DIV_X * MESH_DIV_Z ) ;

	/* 高さの更新 */
	z = 0 ;
	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		x = 0 ;
		for ( j = 0 ; j < MESH_DIV_X ; j++ ){
			waves[ x + z ].height += waves[ x + z ].force - ave ;
			x++ ;
		}
		z += offset ;
	}

	/* データループ処理 */
	z = 0 ;
	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		waves[ z + MESH_DIV_X ] = waves[ z + 0 ] ;
		z += offset ;
	}
	for ( i = 0 ; i < MESH_X ; i++ ){
		waves[ i + z ] = waves[ i ] ;
	}


}
/* 頂点アニメーション処理 */
static void ActVertex( Work *work, DG_PATCH_VERT *verts )
{
	int			i, j ;

	if ( GV_PadData[0].press & PAD_X )work->waves[  _RND( MESH_X ) + _RND( MESH_Z ) * ( MESH_X ) ].force = 1000.0f ;
#if 0
	/* 波アニメーションテスト */
	{
		int		x, z ;
		float	height[ ( MESH_X ) * ( MESH_Z ) ], ave;
		ave = 0 ;
		for ( z = 0 ; z < ( MESH_Z ) ; z++ ){
			for ( x = 0 ; x < ( MESH_X ) ; x++ ){
				DG_PATCH_VERT	*vert ;
				vert = &verts[ x + z * ( MESH_X ) ] ;
				height[ x + z * ( MESH_X ) ] = vert->pos.vy ;
				ave += vert->pos.vy ;
			}
		}
		ave *= 1.0f / ( MESH_X * MESH_Z ) ;
		for ( z = 0 ; z < ( MESH_Z ) ; z++ ){
			for ( x = 0 ; x < ( MESH_X ) ; x++ ){
				int	px, nx, pz, nz, flag ;
				DG_PATCH_VERT	*vert ;
				float		h, f ;
				px = x - 1 ;
				nx = x + 1 ;
				pz = z - 1 ;
				nz = z + 1 ;
#if 0
				if ( px < 0 ) px = MESH_DIV_X ;
				if ( pz < 0 ) pz = MESH_DIV_Z ;
				if ( nx >= ( MESH_X ) ) nx = 0 ;
				if ( nz >= ( MESH_Z ) ) nz = 0 ;
#else
				flag = 0 ;
				if ( px < 0 ) px = 0, flag = 1 ;
				if ( pz < 0 ) pz = 0, flag = 1 ;
				if ( nx >= ( MESH_X ) ) nx = MESH_DIV_X, flag = 1 ;
				if ( nz >= ( MESH_Z ) ) nz = MESH_DIV_Z, flag = 1 ;
#endif
				h = ( height[ x + nz * MESH_X ] +
					 height[ x + pz * MESH_X ] +
					 height[ nx + z * MESH_X ] +
					 height[ px + z * MESH_X ] ) * (2.0f/12.0f) +
					   ( height[ nx + nz * MESH_X ] +
						height[ nx + pz * MESH_X ] +
						height[ px + nz * MESH_X ] +
						height[ px + pz * MESH_X ] ) * (1.0f/12.0f) ;
				f = h - height[ x + z * MESH_X ] ;
				f = work->force[ x + z * MESH_X ] + f * 0.50f ;
				vert = &verts[ x + z * MESH_X ] ;
				if ( flag == 0 ){
					vert->pos.vy += f - ave ;
				} else {
					vert->pos.vy += f - ave ;
					vert->pos.vy *= 0.5f ;
				}
				if ( f > 3.0f ) f *= 0.50f ;
				work->force[ x + z * MESH_X ] = f ;
			}
		}
	}
#endif

	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			verts->pos.vy = work->waves[ j + i * MESH_X ].height;
			verts++ ;
		}
	}
}
/* 頂点間の傾き計算 */
static void CalcVertsInclination( Work *work, DG_PATCH_VERT *mesh_verts )
{
	int		i, j, x, z, offset ;
	int	px, nx, pz, nz ;

#if 1 || !defined(PSX2)
	/* メッシュの補間パラメータ計算処理 */

	offset = MESH_X ;

	pz = ( MESH_DIV_Z - 1 ) * offset ;	/* １つ前の頂点オフセット */
	z = 0  ;					/* 処理中の頂点オフセット */
	nz = offset ;				/* １つ次の頂点オフセット */
	for ( i = 0, z = 0 ; i < ( MESH_DIV_Z ) ; i++ ){
		px = MESH_DIV_X - 1;		/* １つ前の頂点オフセット */
		x = 0 ;					/* 処理中の頂点オフセット */
		nx = 1 ;				/* １つ次の頂点オフセット */
		for ( j = 0 ; j < ( MESH_DIV_X ) ; j++ ){
			DG_PATCH_VERT	*verts, *prev, *next ;

			verts = &mesh_verts[ x + z ] ;

			prev = &mesh_verts[ px + z ] ;
			next = &mesh_verts[ nx + z ] ;
			_sceVu0SubVector( &verts->pos_ds, &next->pos, &prev->pos );
			_sceVu0ScaleVector( &verts->pos_ds, &verts->pos_ds, 0.5f );	
			verts->pos_ds.vx = MESH_WIDTH_X ;
			verts->pos_ds.vz = 0.0f ;

			prev = &mesh_verts[ x + pz ] ;
			next = &mesh_verts[ x + nz ] ;
			_sceVu0SubVector( &verts->pos_dt, &next->pos, &prev->pos );
			_sceVu0ScaleVector( &verts->pos_dt, &verts->pos_dt, 0.5f );	
			verts->pos_dt.vx = 0.0f ;
			verts->pos_dt.vz = MESH_WIDTH_Z ;

			/* 頂点オフセットインクリメント */
			px++ ;
			x++ ;
			nx++ ;
			if ( px >= MESH_DIV_X ) px = 0 ;
			if ( nx >= MESH_DIV_X ) nx = 0 ;
		}
		/* 頂点オフセットインクリメント */
		pz += offset ;
		z += offset ;
		nz += offset ;
		if ( pz >= MESH_DIV_Z * offset ) pz = 0 ;
		if ( nz >= MESH_DIV_Z * offset ) nz = 0 ;
	}

	/* データループ処理 */
	z = 0 ;
	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		mesh_verts[ z + MESH_DIV_X ].pos_ds = mesh_verts[ z + 0 ].pos_ds ;
		mesh_verts[ z + MESH_DIV_X ].pos_dt = mesh_verts[ z + 0 ].pos_dt ;
		z += offset ;
	}
	for ( i = 0 ; i < MESH_X ; i++ ){
		mesh_verts[ i + z ].pos_ds = mesh_verts[ i ].pos_ds ;
		mesh_verts[ i + z ].pos_dt = mesh_verts[ i ].pos_dt ;
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
	DG_PATCH_VERT	*wverts = SCRPAD_ADDR ;
	GV_MSG		*msg ;
	int			time, n_msg, i, j ;

	/* 頂点をスクラッチパッドへ */
	GV_SET_PRFC_CLOCK();
	DG_StartMemToSpr( wverts, work->wverts, MESH_X * MESH_Z * 4 );
	DG_EndMemToSpr();
	time = GV_GET_PRFC_CLOCK();
	//printf("wave2.c: %d ", time);

	/* 通常アニメーション用パラメータ変更 */
	GV_SET_PRFC_CLOCK();
	ActAnimeParam( work );
	time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	/* 通常上下運動アニメーションの頂点計算 */
	GV_SET_PRFC_CLOCK();
	ActVertex( work, wverts );
	time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	/* 頂点変化傾き計算 */
	GV_SET_PRFC_CLOCK();
	CalcVertsInclination( work, wverts );
	time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	DG_SwitchBuffPatch( work->patch );

	/* 頂点データの書き戻し */
	GV_SET_PRFC_CLOCK();
	FlushCache( 0 );
	DG_StartSprToMem( work->patch->verts[ work->patch->buffer_clock ], wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	time = GV_GET_PRFC_CLOCK();
	//printf(" %d ", time);

	/* 頂点データの書き戻し */
	GV_SET_PRFC_CLOCK();
	FlushCache( 0 );
	DG_StartSprToMem( work->wverts, wverts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	time = GV_GET_PRFC_CLOCK();
	//printf(" %d \n", time );


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
				printf("wave3.c:add chara %d\n", msg->message[1]);
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
			pos = ctrl->mov ;
			pos.vy = work->patch->world.m[3][1] ;
			if ( pos.vy > ctrl->mov.vy ){
				printf("wave3.c: splash on!!(%d)\n", work->splash[i].name );
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


}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	int		i, j ;
	//DG_DequeuePatchObjs( work->patch );
	DG_FreePatch( work->patch );
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			PatchWork	*p_work ;
			p_work = &work->patchs[ j + i * PATCH_X ] ;
			DG_DequeuePatchObjs( p_work->patch );
			DG_FreePatch( p_work->patch );
		}
	}
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
	DG_PATCH_VERT	*wverts = SCRPAD_ADDR ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	/* テクスチャ設定読み込み */
	if ( GCL_GetOption( 't' ) != NULL ) {
		work->tri_id = GCL_GetNextInt() ;
		work->tex_id = GCL_GetNextInt() ;
		//work->tex_id2 = GCL_GetNextInt() ;
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
	tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 48 );

	/* オブジェクト作成 */
	work->patch = patch = DG_MakePatchMesh( DG_PATCH_REFLECTPLANE, MESH_X, MESH_Z );
	//DG_QueuePatchObjs( patch );
	GM_GroupObject( patch, work->map );
	DG_ConfigPatchLOD( patch, 10 );
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


	InvisiblePatchParts( work, work->patch );

	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			PatchWork	*p_work ;
			p_work = &work->patchs[ j + i * PATCH_X ] ;
			p_work->patch = patch = DG_MakePatchMesh( DG_PATCH_REFLECTPLANE, MESH_X, MESH_Z );
			GM_GroupObject( patch, work->map );
			DG_QueuePatchObjs( patch );
			DG_ConfigPatchLOD( patch, 10 );
			patch->tri_id = work->tri_id ;
			patch->tex = tex ;
			patch->world.m[3][0] = pos.vx + (float)( MESH_WIDTH_X * MESH_DIV_X ) * ( j - PATCH_X * 0.5f + 0.5f ) ;
			patch->world.m[3][1] = pos.vy ;
			patch->world.m[3][2] = pos.vz + (float)( MESH_WIDTH_Z * MESH_DIV_Z ) * ( i - PATCH_Z * 0.5f + 0.5f ) ;
			p_work->verts_backup[0] = patch->verts[0] ;
			p_work->verts_backup[1] = patch->verts[1] ;
			p_work->parts_backup = patch->parts ;
			patch->verts[0] = work->patch->verts[0] ;
			patch->verts[1] = work->patch->verts[1] ;
			patch->parts = work->patch->parts ;
		}
	}


	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewReflectionWaterSurfaceSet( int name, int where )
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
