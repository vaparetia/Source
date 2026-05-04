//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_shck2.c
	量産型ＲＡＹ用エフェクト（振動衝撃波）

	2001/07/20 K.Takabe
	$Id: ef_shck2.c,v 1.1.1.3 2002/11/19 11:51:21 Yoshizawa1 Exp $

*/
/*



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
#include	"debugmenu.h"


#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
extern void PDRAY_CLOUD_DeleteDensity( FVECTOR *pos, int size );
/* ---------------------------------------------------------------- */
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/* ---------------------------------------------------------------- */
//#define POINT_ALIVE_TIME	(5*15)		/* 生成頂点の生存時間 */
#define POINT_ALIVE_FRAME	(16)	/* 生成頂点の生存フレーム数 */
#define TOTAL_DIV	(32)				/* 円の分割数 */
#define PARTS_DIV	(8)
/* ---------------------------------------------------------------- */
#define PRIM_FLAG	(DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define N_PRIMS		(TOTAL_DIV /2)
#define N_VERTS		(POINT_ALIVE_FRAME*2 *2)
//#define TEXTURE		(1084957)		/* "smoke_lp1_alp" */
//#define TEXTURE		(2133533)		/* "smoke_lp2_alp" */
#define TEXTURE		(3182109)		/* "smoke_lp3_alp" */
//#define TEXTURE		(13145893)			/* "wind01" */

#define FADE_TIME	(200)			/* 生存時間 */
/* ---------------------------------------------------------------- */
#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/* ---------------------------------------------------------------- */
typedef struct _scrpad_work{
	FVECTOR			base_pos[ TOTAL_DIV ];
	FVECTOR			points[ POINT_ALIVE_FRAME ][ TOTAL_DIV ] ;
	FVECTOR			pos[64] ;
	DG_PRIM2_UVRGB	uvrgb[64] ;
} ScrpadWork ;

/* ---------------------------------------------------------------- */
typedef struct {
	float	range ;
	float	height ;
	float	force_x ;
	float	force_y ;
} RING_INFO ;

typedef struct {
	GV_ACT_EX	actor ;
	int			map ;
	int			hzx_id ;

	FMATRIX		pos_mat ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	float		u_scale ;
	float		v_scale ;
	float		u_offset ;
	float		v_offset ;

	/* 計算省略用基礎位置データ */
	FVECTOR		base_pos[TOTAL_DIV] ;

	RING_INFO	ring_info[ POINT_ALIVE_FRAME ] ;

	DG_PRIM2_UVRGB	base_uvrgb[ N_VERTS ];

	int			frame ;			/* 起動してからの経過フレーム数 */
	int			count ;			/* 生存カウンタ */
	float		speed ;			/* 拡大スピード */
	float		length ;		/* 発生距離 */
	float		range ;			/* 半径 */
	int			rot_y ;
} Work ;


/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* メインメモリからスクラッチパッドへ転送 */
static void SprLoad( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartMemToSpr( scrpad, mem, num );
	DG_EndMemToSpr() ;
}
/* スクラッチパッドからメインメモリへ転送 */
static void SprStore( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartSprToMem( mem, scrpad, num );
	DG_EndSprToMem() ;
}

/* ---------------------------------------------------------------- */
/* スクラッチパッド上の頂点をプリミティブ用頂点に振り分ける */
static void SetPrimVerts( FVECTOR *pos )
{
#if 0
	ScrpadWork		*scrpad ;
	FVECTOR		*pos_a, *pos_b, *tmp_pos ;
	int		i, j ;

	scrpad = SCRPAD_ADDR ;
	for ( i = 0 ; i < TOTAL_DIV ; i++ ){
		pos_a = &scrpad->points[0][ i ] ;
		pos_b = ( i != ( TOTAL_DIV - 1 ) ) ? &scrpad->points[ 0 ][ i + 1 ] : &scrpad->points[ 0 ][ 0 ] ;
		tmp_pos = scrpad->pos ;
		for ( j = 0 ; j < POINT_ALIVE_FRAME ; j++ ){
			tmp_pos[0] = *pos_a ;
			tmp_pos[1] = *pos_b ;
			tmp_pos += 2 ;
			pos_a += TOTAL_DIV ;
			pos_b += TOTAL_DIV ;
		}
		FlushCache( 0 );
		SprStore( scrpad->pos, pos, POINT_ALIVE_FRAME * 2 );
		pos += POINT_ALIVE_FRAME * 2 ;
	}
#else
	ScrpadWork		*scrpad ;
	FVECTOR		*pos_a, *pos_b ;
	int		i, j ;

	scrpad = SCRPAD_ADDR ;
	for ( i = 0 ; i < TOTAL_DIV ; i++ ){
		pos_a = &scrpad->points[0][ i ] ;
		pos_b = ( i != ( TOTAL_DIV - 1 ) ) ? &scrpad->points[ 0 ][ i + 1 ] : &scrpad->points[ 0 ][ 0 ] ;
		for ( j = 0 ; j < POINT_ALIVE_FRAME ; j++ ){
			pos[0] = *pos_a ;
			pos[1] = *pos_b ;
			GV_PREFECH( &pos[2] );
			pos += 2 ;
			pos_a += TOTAL_DIV ;
			pos_b += TOTAL_DIV ;
		}
	}
#endif
}
static void ScaleVector( FVECTOR *dst, FVECTOR *src, float scale, float height, int n )
{
	FVECTOR		scale_vec ;
	GTE_SetVector( &scale_vec, scale, height, scale, 0 );
#if 1 //BP_ASM - use C version - #ifndef PSX2
	for ( ; n > 0 ; n-- ){
		//GTE_ScaleVector( dst, src, scale );
		GTE_MulVector( dst, src, &scale_vec );
		dst++ ;
		src++ ;
	}
#else
	asm volatile ("
		lqc2		vf3,0x00(%3)
		lqc2		vf2,0x00(%1)
0:
		vmul.xyzw	vf1,vf2,vf3
		addi		%2,%2,-1
		addi		%1,%1,16
		addi		%0,%0,16
		lqc2		vf2,0x00(%1)
		sqc2		vf1,-16(%0)
		bgtz		%2,0b
	":"+r"(dst),"+r"(src),"+r"(n):"r"(&scale_vec) );
#endif
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ScrpadWork		*scrpad ;
	DG_PRIM2		*prim ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int				i, j, frame ;

	GM_CurrentMap = work->map ;

	scrpad = SCRPAD_ADDR ;

	{/* 頂点データのアニメーション計算 */
		RING_INFO	*dst_info, *src_info ;
		frame = ( work->frame < POINT_ALIVE_FRAME ) ? work->frame : POINT_ALIVE_FRAME - 1 ;
		dst_info = &work->ring_info[ frame ] ;
		src_info = &work->ring_info[ frame - 1 ] ;
		for ( i = frame ; i > 0 ; i-- ){
			dst_info->range = src_info->range + src_info->force_x ;
			dst_info->height = src_info->height + src_info->force_y ;
			dst_info->force_x = src_info->force_x * 0.80f ;
			dst_info->force_y = src_info->force_y * 0.80f - 10 ;
			if ( dst_info->height < 0.0f ) dst_info->height = 0.0f ;
			dst_info-- ;
			src_info-- ;
		}
	}
	/* 新しい頂点パラメータの生成 */
	work->ring_info[ 0 ].range = work->length + work->range ;
	work->ring_info[ 0 ].height = 0 ;
	//work->ring_info[ 0 ].force_x = work->range/3 ;
	work->ring_info[ 0 ].force_x = work->speed/2 ;
	work->ring_info[ 0 ].force_y = work->range ;


	/* スクラッチパッドへ頂点データを移動 */
	FlushCache( 0 );
	SprLoad( scrpad->base_pos, work->base_pos, TOTAL_DIV );

	/* 頂点の生成 */
#if 1
	for ( i = 0, j = 0 ; i < POINT_ALIVE_FRAME ; i++ ){
		ScaleVector( scrpad->points[ i ], scrpad->base_pos,
					work->ring_info[ j ].range, work->ring_info[ j ].height, TOTAL_DIV );
		if ( ( POINT_ALIVE_FRAME - 1 - work->frame ) <= i ) j++ ;
	}
#else
	for ( i = 0 ; i < POINT_ALIVE_FRAME ; i++ ){
		ScaleVector( scrpad->points[ i ], scrpad->base_pos,
					work->ring_info[ i ].range, work->ring_info[ i ].height, TOTAL_DIV );
		if ( i >= work->frame ) break ;
	}
	for ( j = i ; j < POINT_ALIVE_FRAME ; j++ ){
		ScaleVector( scrpad->points[ j ], scrpad->base_pos,
					work->ring_info[ i ].range, work->ring_info[ i ].height, TOTAL_DIV );
	}
#endif

	/* プリミティブ表示＆ダブルバッファ切り替え＆ポインタ取得 */
	prim = work->prim ;
	prim->flag &= ~DG_PRIM2_INVISIBLE ;
	DG_SwitchBuffPrim2( prim );
	pos = prim->pos[ prim->buffer_clock ] ;
	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;

	/* 頂点データをプリミティブに設定 */
	SetPrimVerts( pos );

	FlushCache( 0 );
	SprLoad( SCRPAD_ADDR, uvrgb, N_PRIMS * N_VERTS );
	uvrgb = SCRPAD_ADDR ;
#if 0
	{
		float		t, tt, v0 ;
		int			alpha ;
		tt = 1.0f - (float)work->count / FADE_TIME ;
		tt = DG_MAX( 0.0f, tt );
		alpha = tt * 128 ;
		for ( i = 0 ; i < TOTAL_DIV ; i++ ){
			for ( j = 0 ; j < POINT_ALIVE_FRAME ; j++ ){
				t = j / (float)( POINT_ALIVE_FRAME - 1 ) ;
				t = t * 0.50f + tt * 0.50f ;
				v0 = FTOI12( t * work->v_scale + work->v_offset ) ;
				//v1 = FTOI12( t * work->v_scale + work->v_offset ) ;
				uvrgb[0].v = v0 ;
				uvrgb[1].v = v0 ;
				uvrgb[0].a = alpha ;
				uvrgb[1].a = alpha ;
				uvrgb += 2 ;
			}
		}
	}
#else
	{
		float		t, tt, v0 ;
		int			alpha, c ;
		DG_PRIM2_UVRGB	*tmp_uvrgb ;
		tt = 1.0f - (float)work->count / FADE_TIME ;
		tt = DG_MAX( 0.0f, tt );
		alpha = tt * 128 ;
		for ( j = 0 ; j < POINT_ALIVE_FRAME ; j++ ){
			t = j / (float)( POINT_ALIVE_FRAME - 1 ) ;
			t = t * 0.50f + tt * 0.50f ;
			v0 = FTOI12( t * work->v_scale + work->v_offset ) ;
			//v1 = FTOI12( t * work->v_scale + work->v_offset ) ;
			tmp_uvrgb = uvrgb ;

			c = 64 - 64.0f * j / (float)( POINT_ALIVE_FRAME - 1 ) ;
			if ( ( POINT_ALIVE_FRAME - work->frame ) > j ) c = 0 ;
			if ( j == 0 ) c = 0 ;

			for ( i = 0 ; i < TOTAL_DIV ; i++ ){
				tmp_uvrgb[0].v = v0 ;
				tmp_uvrgb[1].v = v0 ;
				tmp_uvrgb[0].r = c ;
				tmp_uvrgb[0].g = c ;
				tmp_uvrgb[0].b = c ;
				tmp_uvrgb[1].r = c ;
				tmp_uvrgb[1].g = c ;
				tmp_uvrgb[1].b = c ;
				tmp_uvrgb[0].a = alpha ;
				tmp_uvrgb[1].a = alpha ;
				tmp_uvrgb += POINT_ALIVE_FRAME * 2 ;
			}
			uvrgb += 2 ;
		}
	}
#endif
	FlushCache( 0 );
	SprStore( SCRPAD_ADDR, prim->uvrgb[ prim->buffer_clock ], N_PRIMS * N_VERTS );


	/* パラメータの更新 */
	work->length += work->speed ;
	work->speed *= 0.95f ;
	work->range *= 0.97f ;

	work->frame++ ;
	work->count += TIME_BASE ;

	{
		FMATRIX	mat ;
		GTE_UnitMatrix( &mat );
		GTE_MakeRotate( &mat, 0, 1, 0, GTE_PS2RAD( work->rot_y ) );
		GTE_MulMatrix( &prim->as_world, &work->pos_mat, &mat );
		work->rot_y += 6 ;
	}

	if ( work->count > FADE_TIME ){
		GV_DestroyActor( work );
	}
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	GM_FreePrim2( work->prim );
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *parent, float size, float speed, float height )
{
	ScrpadWork		*scrpad ;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR		*pos ;
	DG_TEX		*tex ;
	int			i, j, k ;
	int			u0, v0, u1, v1 ;

	scrpad = SCRPAD_ADDR ;

	/* 各種基礎設定 */
	work->map = GM_CurrentMap ;
	work->pos_mat = *parent ;
	{
		FMATRIX	mat ;
		GTE_UnitMatrix( &mat );
		GTE_MakeRotate( &mat, 0, 1, 0, GTE_PS2RAD( RND(4096) ) );
		GTE_MulMatrix( &work->pos_mat, &work->pos_mat, &mat );
	}

	work->length = size ;
	work->range = height ;
	work->speed = speed ;

	/* テクスチャの取得 */
	work->tex = tex = DG_GetTexture( TEXTURE );
	work->u_scale = tex->u_scale ;
	work->v_scale = tex->v_scale ;
	work->u_offset = tex->u_offset ;
	work->v_offset = tex->v_offset ;

	/* プリミティブオブジェクトの生成 */
	prim = work->prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 ) );
	prim->flag |= DG_PRIM2_INVISIBLE ;
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world = work->pos_mat ;

	/* 頂点データの初期化 */
	for ( k = 0 ; k < 2 ; k++ ){
		pos = prim->pos[ k ];
		uvrgb = prim->uvrgb[ k ];
		for ( i = 0 ; i < TOTAL_DIV ; i++ ){
			if ( i & 4 ){
				int		rate = i & 3 ;
				float	fu0, fu1 ;
				fu0 = (float)rate / 4.0f ;
				fu1 = (float)( rate + 1 ) / 4.0f ;
				u0 = FTOI12( fu0 * tex->u_scale + tex->u_offset ) ;
				u1 = FTOI12( fu1 * tex->u_scale + tex->u_offset ) ;
			} else {
				int		rate = i & 3 ;
				float	fu0, fu1 ;
				fu0 = (float)rate / 4.0f ;
				fu1 = (float)( rate + 1 ) / 4.0f ;
				u0 = FTOI12( ( 1.0f - fu0 ) * tex->u_scale + tex->u_offset ) ;
				u1 = FTOI12( ( 1.0f - fu1 ) * tex->u_scale + tex->u_offset ) ;
			}
			for ( j = 0 ; j < POINT_ALIVE_FRAME ; j++, pos+=2, uvrgb+=2 ){
				float	t ;

				t = j / (float)( POINT_ALIVE_FRAME - 1 ) ;

				//if ( j == 0 ) t = 1.0f ;
				//if ( j == 1 ) t = 0.5f ;
				/* 色の設定 */
				uvrgb[0].r = 64 - 64 * t ;
				uvrgb[0].g = 64 - 64 * t ;
				uvrgb[0].b = 64 - 64 * t ;
				uvrgb[1] = uvrgb[0] ;

				/* テクスチャ及びフラグの設定 */
				//u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
				v0 = FTOI12( t * tex->v_scale + tex->v_offset ) ;
				//u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
				v1 = FTOI12( t * tex->v_scale + tex->v_offset ) ;
				SET_UVRGB_UV( &uvrgb[0], u0, v0 );
				SET_UVRGB_UV( &uvrgb[1], u1, v1 );

				uvrgb[0].f = 0x0fff ;
				uvrgb[1].f = 0x0fff ;
				if ( j == 0 ){
					uvrgb[0].f = 0x8fff ;
					uvrgb[1].f = 0x8fff ;
				}
			}
		}
	}

	/* ベースデータの初期化 */
	for ( i = 0 ; i < TOTAL_DIV ; i++ ){
		FVECTOR		sincos ;
		GTE_SinCos( &sincos, GTE_PS2RAD( 4096 * i / TOTAL_DIV ) );
		scrpad->base_pos[ i ].vx = sincos.vx ;
		scrpad->base_pos[ i ].vy = 1.0f ;
		scrpad->base_pos[ i ].vz = sincos.vy ;
	}
	FlushCache( 0 );
	SprStore( scrpad->base_pos, work->base_pos, TOTAL_DIV );

	/* スクラッチパッド上に初期頂点データの構築を行いワークへ転送 */
	for ( i = 0 ; i < POINT_ALIVE_FRAME ; i++ ){
		work->ring_info[ i ].range = work->length ;
		work->ring_info[ i ].height = 0 ;
		work->ring_info[ i ].force_x = 0 ;
		work->ring_info[ i ].force_y = 0 ;
		//ScaleVector( scrpad->points[ i ], scrpad->base_pos, work->range, 0, TOTAL_DIV );
	}
	FlushCache( 0 );
	//SprStore( scrpad->points, work->points, POINT_ALIVE_FRAME * TOTAL_DIV );

	work->rot_y = RND( 4096 );

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewFogWave( FMATRIX *parent, int size, int speed, int height )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, parent, size, speed, height ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
#if 0
	if ( parent->m[3][1] > 3900.0f && parent->m[3][1] < 4200.0f ){
		PDRAY_CLOUD_DeleteDensity( (FVECTOR*)parent->m[3], size * 2.5f );
	}
#endif
	return (void *)work ;
}

#if 0
{
	extern void *NewFogWave( FMATRIX *parent, int size, int speed, int height );
	NewFogWave( &GM_PlayerBody->objs->world, 1000, 200, 300 );
}
#endif
