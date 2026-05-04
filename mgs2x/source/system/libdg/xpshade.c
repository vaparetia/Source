//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xpshade.c
	プレシェイド（ＲＧＢ前処理計算）ルーチン

	2002/03/13 K.Takabe
	$Id: xpshade.c,v 1.1.1.3 2002/11/19 11:42:37 Yoshizawa1 Exp $

*/
/*
	プレシェイド（ＲＧＢ前処理計算）ルーチン

	int		DG_MakePreshade( objs, lights, n_lights )
	DG_OBJS		*objs ;		物体ハンドラ
	DG_LIT		*lights ;	光源配列
	int		n_lights ;	光源数

		プレシェイドデータを計算
		プレシェイド用バッファがなければ確保する

	void		DG_FreePreshade( objs )
	DG_OBJS		*objs ;		物体ハンドラ

		プレシェイド用バッファを解放する


*/

#ifdef KP_XBOX //BP

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

//#define NEW_KMX_FORMAT	/* XBOX用新フォーマット対応 */

#define NO_CACHED(a) a

/*----------------------------------------------------------------*/

typedef struct {
	FVECTOR			w_vert ;
	FVECTOR			w_vec ;
	FVECTOR			w_param ;
	FVECTOR			w_tmpvec ;
	int				n_point ;
	int				n_spot ;
	int				n_line ;
	int				n_black ;
	int				hit_flag ;
	LIT_POINT		*points ;
	LIT_SPOT		*spot ;
	LIT_LINE		*line ;
	LIT_BLACKPOINT	*blackpoints ;
	int				pad[3] ;
	LIT_POINT		buffer[0] ;
} ScrpadWork ;


/*----------------------------------------------------------------*/

/* ベクトルの長さを求める */
#define VEC_LEN(_x,_y,_z) (bp_sqrtf((_x) * (_x) + (_y) * (_y) + (_z) * (_z))) //BP_MATH - emulate PS2 sqrtf

/* 絶対値を取る */
#define FABS(_x) DG_FABS(_x)

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

/*----------------------------------------------------------------*/
static inline void __SetMatrix( FMATRIX *m )
{
	__asm {
        mov		edx, m
		movups	xmm4, [edx+00h]
		movups	xmm5, [edx+10h]
		movups	xmm6, [edx+20h]
		movups	xmm7, [edx+30h]
	}
}

static inline void _RotTrans( FVECTOR *res, FVECTOR *v )
{
	__asm {
        mov		ecx, v
        mov		eax, res
		movups	xmm0, [ecx+00h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		movaps	xmm3, xmm7
		addps	xmm1, xmm0
		addps	xmm3, xmm2
		addps	xmm3, xmm1
		movups	[eax+00h], xmm3 
	}
}

static inline void _RotVector( FVECTOR *res, FVECTOR *v )
{
	__asm {
        mov		ecx, v
        mov		eax, res
		movups	xmm0, [ecx+00h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		addps	xmm1, xmm0
		addps	xmm2, xmm1
		movups	[eax+00h], xmm2 
	}
}


/*----------------------------------------------------------------*/
static FMATRIX	pshade_mat;	/* vf4～vf7 */
static FVECTOR	bound_max;	/* vf9 */
static FVECTOR	bound_min;	/* vf10 */

static inline void _SetMatrix( FMATRIX *m )
{
	pshade_mat = *m ;
}

static inline void _SetDefaultBound( FVECTOR *max, FVECTOR *min )
{
	bound_max = *max ;
	bound_min = *min ;
}

static inline void _GetBound( FVECTOR *max, FVECTOR *min )
{
	*max = bound_max ;
	*min = bound_min ;
}

static inline void _RotTransBoundCheck( FVECTOR *v )
{
	FVECTOR		tmp ;
	__SetMatrix( &pshade_mat );
	_RotTrans( &tmp, v );

	bound_max.vx = DG_MAX( bound_max.vx, tmp.vx );
	bound_max.vy = DG_MAX( bound_max.vy, tmp.vy );
	bound_max.vz = DG_MAX( bound_max.vz, tmp.vz );	
	bound_min.vx = DG_MIN( bound_min.vx, tmp.vx );
	bound_min.vy = DG_MIN( bound_min.vy, tmp.vy );
	bound_min.vz = DG_MIN( bound_min.vz, tmp.vz );
}


/*----------------------------------------------------------------*/

	/*
		各頂点についての照明データを計算する

	*/
#ifndef NEW_KMX_FORMAT
static void MakeLight( CVECTOR *rgbs, SVECTOR *verts, SVECTOR *norms, int n_verts, CVECTOR *base )
{
	/*
		新バージョンプリシェード計算ルーチン
	*/
	FVECTOR		one = {1.0F,1.0F,1.0F,255.0F}, two = {1.5f,-0.75f,0.0f,0.0f}, color ;
	LIT_POINT	*lit ;
	LIT_SPOT	*spot ;
	LIT_LINE	*line ;
	LIT_BLACKPOINT	*black ;
	u_long64	tmp ;
	int		i, count ;
	ScrpadWork *work = SCRPAD_ADDR ;
	FVECTOR		v, n ;

	while ( -- n_verts >= 0 ) {
		/* 座標変換 */
		v.vx = verts->vx ;
		v.vy = verts->vy ;
		v.vz = verts->vz ;
		v.vw = 1.0f ;
		n.vx = norms->vx * (1.0f/4096.0f) ;
		n.vy = norms->vy * (1.0f/4096.0f) ;
		n.vz = norms->vz * (1.0f/4096.0f) ;
		_SetMatrix( &pshade_mat );
		_RotTrans( &v, &v );
		_RotVector( &n, &n );
		work->w_vert = v ;
		if ( base == NULL ){/* ステージ並行光源＆アンビエントによるカラー算出 */
			float	f ;
			f = n.vx * DG_LightMatrix.m[0][0] + n.vy * DG_LightMatrix.m[1][0] + n.vz * DG_LightMatrix.m[2][0] ;
			f = DG_MAX( f, 0.0f );
			_sceVu0ScaleVector( &color, (FVECTOR*)&DG_ColorMatrix.m[0][0], f );
			_sceVu0AddVector( &color, &color, (FVECTOR*)&DG_ColorMatrix.m[3][0] );
		} else {
			/* デフォルトに対してさらに加算ライトを計算する場合 */
			color.vx = (float)base->b ;
			color.vy = (float)base->g ;
			color.vz = (float)base->r ;
			base++ ;
		}

		lit = work->points ;

		/* 点光源 */
		count = 0 ;
		for ( i = work->n_point ; i > 0 ; -- i, lit++ ) {
			float	f, r, scale, len, lenlen, rangerange, range ;
			//r = lit->r_range * 2.0f ;
			r = lit->e_range ;
			work->w_vec.vx = f = work->w_vert.vx - lit->point.vx ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vy = f = work->w_vert.vy - lit->point.vy ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vz = f = work->w_vert.vz - lit->point.vz ;
			if ( FABS( f ) > r ) continue ;
			range = lit->r_range * 2 ;
			/*
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; total_color += color * force * 1.5 ;
			   ; 以上の計算を最適化したもの
			*/
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				f = _sceVu0InnerProduct( &work->w_vec, &n );
				if ( f > 0.0f ){
					len = DG_SQRT( lenlen );
					/* 距離による減衰計算 */
					scale = ( range - len ) / range ;
					/* 光源影響度計算＆頂点カラーの加算 */
					f = f / len * 1.5f * scale ;
					color.vx += (float)lit->color.r * f ;
					color.vy += (float)lit->color.g * f ;
					color.vz += (float)lit->color.b * f ;
				}
			}
			
		}

		/* スポット光源 */
		spot = work->spot ;
		for ( i = work->n_spot ; i > 0 ; -- i, spot++ ) {
			float	f, r, scale, len, lenlen, rangerange, range, dir ;
			r = spot->dir.vw * 2.0f ;
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > spot->bound_max.vx || work->w_vert.vx < spot->bound_min.vx ) continue ;
			if ( work->w_vert.vy > spot->bound_max.vy || work->w_vert.vy < spot->bound_min.vy ) continue ;
			if ( work->w_vert.vz > spot->bound_max.vz || work->w_vert.vz < spot->bound_min.vz ) continue ;
			work->w_vec.vx = work->w_vert.vx - spot->point.vx ;
			work->w_vec.vy = work->w_vert.vy - spot->point.vy ;
			work->w_vec.vz = work->w_vert.vz - spot->point.vz ;
			/*
			   ; ＜計算内容について＞
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; cos_angle = VecInnerProduct( dir , vec )
			   ; if ( cos_angle > umbra ) spot_force = 1.0 ;
			   ; else if ( cos_angle < penumbra ) spot_force = 0.0 ;
			   ; else spot_force = ( penumbra - cos_angle ) / ( penumbra - umbra ) ;
			   ; total_color += color * force * spot_force * 1.5 ;
			   ; 以上の計算を最適化したもの
			*/
			range = r ;
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				f = _sceVu0InnerProduct( &work->w_vec, &n );
				if ( f > 0.0f ){
					len = DG_SQRT( lenlen );
					/* スポットによる減衰計算 */
					dir = _sceVu0InnerProduct( &spot->dir, &work->w_vec );
					if ( dir > ( len * spot->umbra ) ) {
						dir = 1.0f ;
					} else if ( dir < ( len * spot->penumbra ) ) {
						dir = 0.0f ;
					} else {
						dir = ( spot->penumbra * len - dir ) / ( ( spot->penumbra - spot->umbra ) * len ) ;
					}
					/* 距離による減衰計算 */
					scale = ( range - len ) / range ;
					/* 光源影響度計算＆頂点カラーの加算 */
					f = f / len * 1.5f * scale * dir ;
					color.vx += (float)spot->color.r * f ;
					color.vy += (float)spot->color.g * f ;
					color.vz += (float)spot->color.b * f ;
				}
			}
		}

		/* 線光源 */
		line = work->line ;
		for ( i = work->n_line ; i > 0 ; -- i, line++ ) {
			float	f, r, scale, len, lenlen, rangerange, range ;
			r = line->dir.vw * 2.0f ;
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > line->bound_max.vx || work->w_vert.vx < line->bound_min.vx ) continue ;
			if ( work->w_vert.vy > line->bound_max.vy || work->w_vert.vy < line->bound_min.vy ) continue ;
			if ( work->w_vert.vz > line->bound_max.vz || work->w_vert.vz < line->bound_min.vz ) continue ;
			range = line->r_range * 2 ;
			/*
			   ; ＜計算内容について＞
			   ; end_pos = start_pos + dir * dir.w ;
			   ; n1 = VecInnerProduct( start_pos, dir );
			   ; n2 = VecInnerProduct( end_pos, dir );
			   ; n3 = VecInnerProduct( pos, dir );
			   ; if ( n1 > n3 ){
			   ;  vec = pos - start_pos ;
			   ; } else if ( n2 < n3 ){
			   ;  vec = pos - end_pos ;
			   ; }else {
			   ;  vec = pos - ( start_pos + dir * ( n3 - n1 ) ) ;
			   ; }
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; total_color += color * force * 1.5 ;
			   ; 以上の計算を最適化したもの
			*/
			{/* 頂点に一番近い線分中の点を算出 */
				FVECTOR	start, end ;
				float		n1, n2, n3 ;
				start = line->point ;
				_sceVu0ScaleVector( &end, &line->dir, line->dir.vw );
				_sceVu0AddVector( &end, &end, &start );
				n1 = _sceVu0InnerProduct( &start, &line->dir );
				n2 = _sceVu0InnerProduct( &end, &line->dir );
				n3 = _sceVu0InnerProduct( &work->w_vert, &line->dir );
				if ( n1 > n3 ){
					_sceVu0SubVector( &work->w_vec, &work->w_vert, &start );
				} else if ( n2 < n3 ){
					_sceVu0SubVector( &work->w_vec, &work->w_vert, &end );
				} else {
					_sceVu0ScaleVector( &end, &line->dir, n3 - n1 );
					_sceVu0AddVector( &end, &end, &start );
					_sceVu0SubVector( &work->w_vec, &work->w_vert, &end );
				}
			}
			/* 残りは点光源と同じ */
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				f = _sceVu0InnerProduct( &work->w_vec, &n );
				if ( f > 0.0f ){
					len = DG_SQRT( lenlen );
					/* 距離による減衰計算 */
					scale = ( range - len ) / range ;
					/* 光源影響度計算＆頂点カラーの加算 */
					f = f / len * 1.5f * scale ;
					color.vx += (float)line->color.r * f ;
					color.vy += (float)line->color.g * f ;
					color.vz += (float)line->color.b * f ;
				}
			}
		}

		/* 黒点光源 */
		black = work->blackpoints ;
		for ( i = work->n_black ; i > 0 ; -- i, black++ ) {
			float	f, r, scale, len, lenlen, rangerange, range ;
			if ( ( black->bound_max.vx <= work->w_vert.vx ) || ( black->bound_min.vx >= work->w_vert.vx ) ) continue ;
			if ( ( black->bound_max.vy <= work->w_vert.vy ) || ( black->bound_min.vy >= work->w_vert.vy ) ) continue ;
			if ( ( black->bound_max.vz <= work->w_vert.vz ) || ( black->bound_min.vz >= work->w_vert.vz ) ) continue ;
			work->w_vec.vx = work->w_vert.vx - black->point.vx ;
			work->w_vec.vy = work->w_vert.vy - black->point.vy ;
			work->w_vec.vz = work->w_vert.vz - black->point.vz ;
			range = black->r_range * 2 ;
			/*
			   ; ＜計算内容について＞
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; total_color *= ( 1.0 - force ) ;
			   ; 以上の計算を最適化したもの
			*/
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				len = DG_SQRT( lenlen );
				/* 距離による減衰計算 */
				scale = len / range ;
				/* 光源影響度計算＆頂点カラーの加算 */
				f = 1.0f - scale ;
#if 0
				/* 本当はこっちが正しいのだが・・・ */
				color.vx = color.vx * f ;
				color.vy = color.vy * f ;
				color.vz = color.vz * f ;
#else
				/* ＰＳ２のルーチンがバグっていてこうなってしまっていた */
				color.vx = color.vx - color.vx * f ;
				color.vy = color.vy - color.vy * f ;
				color.vz = color.vz - color.vz * f ;
#endif
			}
			break ;
		}

		/* 結果の書き戻し */
		color.vx = DG_MIN( color.vx, 255.0f );
		color.vy = DG_MIN( color.vy, 255.0f );
		color.vz = DG_MIN( color.vz, 255.0f );
		/* PS2とXBOXは並びが違うので入れ替えておく */
		rgbs->r = (unsigned char)color.vz ;
		rgbs->g = (unsigned char)color.vy ;
		rgbs->b = (unsigned char)color.vx ;
		rgbs->cd = 0x80 ;

		verts++ ;
		norms++ ;
		rgbs++ ;
	}
}
#else
static void MakeLight( CVECTOR *rgbs, DG_VERTEX_KMSS *verts, int n_verts, CVECTOR *base )
{
	/*
		新バージョンプリシェード計算ルーチン
	*/
	FVECTOR		one = {1.0F,1.0F,1.0F,255.0F}, two = {1.5f,-0.75f,0.0f,0.0f}, color ;
	LIT_POINT	*lit ;
	LIT_SPOT	*spot ;
	LIT_LINE	*line ;
	LIT_BLACKPOINT	*black ;
	u_long64	tmp ;
	int		i, count ;
	ScrpadWork *work = SCRPAD_ADDR ;
	FVECTOR		v, n ;

	while ( -- n_verts >= 0 ) {
		/* 座標変換 */
		v.vx = verts->vx ;
		v.vy = verts->vy ;
		v.vz = verts->vz ;
		v.vw = 1.0f ;
		n.vx = verts->nx * (1.0f/32767.0f) ;
		n.vy = verts->ny * (1.0f/32767.0f) ;
		n.vz = verts->nz * (1.0f/32767.0f) ;
		_SetMatrix( &pshade_mat );
		_RotTrans( &v, &v );
		_RotVector( &n, &n );
		work->w_vert = v ;
		if ( base == NULL ){/* ステージ並行光源＆アンビエントによるカラー算出 */
			float	f ;
			f = n.vx * DG_LightMatrix.m[0][0] + n.vy * DG_LightMatrix.m[1][0] + n.vz * DG_LightMatrix.m[2][0] ;
			f = DG_MAX( f, 0.0f );
			_sceVu0ScaleVector( &color, (FVECTOR*)&DG_ColorMatrix.m[0][0], f );
			_sceVu0AddVector( &color, &color, (FVECTOR*)&DG_ColorMatrix.m[3][0] );
		} else {
			/* デフォルトに対してさらに加算ライトを計算する場合 */
			color.vx = (float)base->b ;
			color.vy = (float)base->g ;
			color.vz = (float)base->r ;
			base++ ;
		}

		lit = work->points ;

		/* 点光源 */
		count = 0 ;
		for ( i = work->n_point ; i > 0 ; -- i, lit++ ) {
			float	f, r, scale, len, lenlen, rangerange, range ;
			//r = lit->r_range * 2.0f ;
			r = lit->e_range ;
			work->w_vec.vx = f = work->w_vert.vx - lit->point.vx ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vy = f = work->w_vert.vy - lit->point.vy ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vz = f = work->w_vert.vz - lit->point.vz ;
			if ( FABS( f ) > r ) continue ;
			range = lit->r_range * 2 ;
			/*
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; total_color += color * force * 1.5 ;
			   ; 以上の計算を最適化したもの
			*/
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				f = _sceVu0InnerProduct( &work->w_vec, &n );
				if ( f > 0.0f ){
					len = DG_SQRT( lenlen );
					/* 距離による減衰計算 */
					scale = ( range - len ) / range ;
					/* 光源影響度計算＆頂点カラーの加算 */
					f = f / len * 1.5f * scale ;
					color.vx += (float)lit->color.r * f ;
					color.vy += (float)lit->color.g * f ;
					color.vz += (float)lit->color.b * f ;
				}
			}
			
		}

		/* スポット光源 */
		spot = work->spot ;
		for ( i = work->n_spot ; i > 0 ; -- i, spot++ ) {
			float	f, r, scale, len, lenlen, rangerange, range, dir ;
			r = spot->dir.vw * 2.0f ;
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > spot->bound_max.vx || work->w_vert.vx < spot->bound_min.vx ) continue ;
			if ( work->w_vert.vy > spot->bound_max.vy || work->w_vert.vy < spot->bound_min.vy ) continue ;
			if ( work->w_vert.vz > spot->bound_max.vz || work->w_vert.vz < spot->bound_min.vz ) continue ;
			work->w_vec.vx = work->w_vert.vx - spot->point.vx ;
			work->w_vec.vy = work->w_vert.vy - spot->point.vy ;
			work->w_vec.vz = work->w_vert.vz - spot->point.vz ;
			/*
			   ; ＜計算内容について＞
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; cos_angle = VecInnerProduct( dir , vec )
			   ; if ( cos_angle > umbra ) spot_force = 1.0 ;
			   ; else if ( cos_angle < penumbra ) spot_force = 0.0 ;
			   ; else spot_force = ( penumbra - cos_angle ) / ( penumbra - umbra ) ;
			   ; total_color += color * force * spot_force * 1.5 ;
			   ; 以上の計算を最適化したもの
			*/
			range = r ;
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				f = _sceVu0InnerProduct( &work->w_vec, &n );
				if ( f > 0.0f ){
					len = DG_SQRT( lenlen );
					/* スポットによる減衰計算 */
					dir = _sceVu0InnerProduct( &spot->dir, &work->w_vec );
					if ( dir > ( len * spot->umbra ) ) {
						dir = 1.0f ;
					} else if ( dir < ( len * spot->penumbra ) ) {
						dir = 0.0f ;
					} else {
						dir = ( spot->penumbra * len - dir ) / ( ( spot->penumbra - spot->umbra ) * len ) ;
					}
					/* 距離による減衰計算 */
					scale = ( range - len ) / range ;
					/* 光源影響度計算＆頂点カラーの加算 */
					f = f / len * 1.5f * scale * dir ;
					color.vx += (float)spot->color.r * f ;
					color.vy += (float)spot->color.g * f ;
					color.vz += (float)spot->color.b * f ;
				}
			}
		}

		/* 線光源 */
		line = work->line ;
		for ( i = work->n_line ; i > 0 ; -- i, line++ ) {
			float	f, r, scale, len, lenlen, rangerange, range ;
			r = line->dir.vw * 2.0f ;
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > line->bound_max.vx || work->w_vert.vx < line->bound_min.vx ) continue ;
			if ( work->w_vert.vy > line->bound_max.vy || work->w_vert.vy < line->bound_min.vy ) continue ;
			if ( work->w_vert.vz > line->bound_max.vz || work->w_vert.vz < line->bound_min.vz ) continue ;
			range = line->r_range * 2 ;
			/*
			   ; ＜計算内容について＞
			   ; end_pos = start_pos + dir * dir.w ;
			   ; n1 = VecInnerProduct( start_pos, dir );
			   ; n2 = VecInnerProduct( end_pos, dir );
			   ; n3 = VecInnerProduct( pos, dir );
			   ; if ( n1 > n3 ){
			   ;  vec = pos - start_pos ;
			   ; } else if ( n2 < n3 ){
			   ;  vec = pos - end_pos ;
			   ; }else {
			   ;  vec = pos - ( start_pos + dir * ( n3 - n1 ) ) ;
			   ; }
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; total_color += color * force * 1.5 ;
			   ; 以上の計算を最適化したもの
			*/
			{/* 頂点に一番近い線分中の点を算出 */
				FVECTOR	start, end ;
				float		n1, n2, n3 ;
				start = line->point ;
				_sceVu0ScaleVector( &end, &line->dir, line->dir.vw );
				_sceVu0AddVector( &end, &end, &start );
				n1 = _sceVu0InnerProduct( &start, &line->dir );
				n2 = _sceVu0InnerProduct( &end, &line->dir );
				n3 = _sceVu0InnerProduct( &work->w_vert, &line->dir );
				if ( n1 > n3 ){
					_sceVu0SubVector( &work->w_vec, &work->w_vert, &start );
				} else if ( n2 < n3 ){
					_sceVu0SubVector( &work->w_vec, &work->w_vert, &end );
				} else {
					_sceVu0ScaleVector( &end, &line->dir, n3 - n1 );
					_sceVu0AddVector( &end, &end, &start );
					_sceVu0SubVector( &work->w_vec, &work->w_vert, &end );
				}
			}
			/* 残りは点光源と同じ */
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				f = _sceVu0InnerProduct( &work->w_vec, &n );
				if ( f > 0.0f ){
					len = DG_SQRT( lenlen );
					/* 距離による減衰計算 */
					scale = ( range - len ) / range ;
					/* 光源影響度計算＆頂点カラーの加算 */
					f = f / len * 1.5f * scale ;
					color.vx += (float)line->color.r * f ;
					color.vy += (float)line->color.g * f ;
					color.vz += (float)line->color.b * f ;
				}
			}
		}

		/* 黒点光源 */
		black = work->blackpoints ;
		for ( i = work->n_black ; i > 0 ; -- i, black++ ) {
			float	f, r, scale, len, lenlen, rangerange, range ;
			if ( ( black->bound_max.vx <= work->w_vert.vx ) || ( black->bound_min.vx >= work->w_vert.vx ) ) continue ;
			if ( ( black->bound_max.vy <= work->w_vert.vy ) || ( black->bound_min.vy >= work->w_vert.vy ) ) continue ;
			if ( ( black->bound_max.vz <= work->w_vert.vz ) || ( black->bound_min.vz >= work->w_vert.vz ) ) continue ;
			work->w_vec.vx = work->w_vert.vx - black->point.vx ;
			work->w_vec.vy = work->w_vert.vy - black->point.vy ;
			work->w_vec.vz = work->w_vert.vz - black->point.vz ;
			range = black->r_range * 2 ;
			/*
			   ; ＜計算内容について＞
			   ; len = VecLength( vec ) ;
			   ; scale = ( r_range - len ) / r_range ;
			   ; vec = VecNormalize( vec ) ;
			   ; force = VecInnerProduct( normal , vec ) ;
			   ; if ( force < 0 ) force = 0 ;
			   ; total_color *= ( 1.0 - force ) ;
			   ; 以上の計算を最適化したもの
			*/
			lenlen = _sceVu0InnerProduct( &work->w_vec, &work->w_vec );
			rangerange = range * range ;
			if ( lenlen < rangerange ){
				len = DG_SQRT( lenlen );
				/* 距離による減衰計算 */
				scale = len / range ;
				/* 光源影響度計算＆頂点カラーの加算 */
				f = 1.0f - scale ;
#if 0
				/* 本当はこっちが正しいのだが・・・ */
				color.vx = color.vx * f ;
				color.vy = color.vy * f ;
				color.vz = color.vz * f ;
#else
				/* ＰＳ２のルーチンがバグっていてこうなってしまっていた */
				color.vx = color.vx - color.vx * f ;
				color.vy = color.vy - color.vy * f ;
				color.vz = color.vz - color.vz * f ;
#endif
			}
			break ;
		}

		/* 結果の書き戻し */
		color.vx = DG_MIN( color.vx, 255.0f );
		color.vy = DG_MIN( color.vy, 255.0f );
		color.vz = DG_MIN( color.vz, 255.0f );
		/* PS2とXBOXは並びが違うので入れ替えておく */
		rgbs->r = (unsigned char)color.vz ;
		rgbs->g = (unsigned char)color.vy ;
		rgbs->b = (unsigned char)color.vx ;
		rgbs->cd = 0x80 ;

		verts++ ;
		rgbs++ ;
	}
}
#endif

/*----------------------------------------------------------------*/
	/*
		モデルのバウンディングに触れる全光源をスクラッチパッドへ転送する
	*/
static void CreateLightBuffer( DG_MDL *mdl, LIT_DEF *lit_def )
{
	static FVECTOR	_bound_max = {-99999999.0f,-99999999.0f,-99999999.0f,0.0f};
	static FVECTOR	_bound_min = {99999999.0f,99999999.0f,99999999.0f,0.0f};
	ScrpadWork *work = SCRPAD_ADDR ;
	int		i, j ;
	LIT_GRP	*grp ;
	void	*ptr ;
	FVECTOR		tmp, mdl_max, mdl_min ;
	{/* モデルバウンディング生成 */
		tmp.vw = 1.0F ;
		_SetDefaultBound( &_bound_max, &_bound_min );
		for ( i = 8 ; i > 0 ; ){
			for ( j = 4 ; j > 0 ; j--, i-- ){
				tmp.vx = ( i & 1 ) ? mdl->lx : mdl->ux ;
				tmp.vy = ( i & 2 ) ? mdl->ly : mdl->uy ;
				tmp.vz = ( i & 4 ) ? mdl->lz : mdl->uz ;
				_RotTransBoundCheck( &tmp );
			}
		}
		_GetBound( &mdl_max, &mdl_min );
	}

	work->hit_flag = 0 ;
	ptr = work->buffer ;
	/* 点光源検索 */
	work->n_point = 0 ;
	work->points = ptr ;
	for ( grp = lit_def->grp, i = lit_def->n_lit_group ; i > 0 ; grp++, i-- ){
		LIT_POINT	*src, *dst ;
		/* 属性チェック */
		if ( grp->type & LIT_TYPE_DISABLE ) continue ;
		if ( !( grp->type & LIT_TYPE_POINT ) ) continue ;
		/* バウンディングチェック */
		if ( !( grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz ) ) continue ;
		/* スクラッチパッドへコピー */
		if ( grp->type & LIT_TYPE_CHANGE ) work->hit_flag = 1 ;
		src = grp->lit ;
		dst = ptr ;
		for ( j = grp->n_lights ; j > 0 ; src++, j-- ){
			if ( src->flag & LIT_FLAG_DISABLE ) continue ;
			if ( !( src->flag & LIT_FLAG_BGONLY ) ) continue ;
			/* さらに個々のあたり判定をチェックする */
			if ( ( src->point.vx + src->e_range ) < mdl_min.vx || ( src->point.vx - src->e_range ) > mdl_max.vx ||
				( src->point.vy + src->e_range ) < mdl_min.vy || ( src->point.vy - src->e_range ) > mdl_max.vy ||
				( src->point.vz + src->e_range ) < mdl_min.vz || ( src->point.vz - src->e_range ) > mdl_max.vz ){
				continue ;
			}
			*dst++ = *src ;
			work->n_point++ ;
		}
		ptr = dst ;
	}
	/* スポット光源検索 */
	work->n_spot = 0 ;
	work->spot = ptr ;
	for ( grp = lit_def->grp, i = lit_def->n_lit_group ; i > 0 ; grp++, i-- ){
		LIT_SPOT	*src, *dst ;
		/* 属性チェック */
		if ( grp->type & LIT_TYPE_DISABLE ) continue ;
		if ( !( grp->type & LIT_TYPE_SPOT ) ) continue ;
		/* バウンディングチェック */
		if ( !( grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz ) ) continue ;
		if ( grp->type & LIT_TYPE_CHANGE ) work->hit_flag = 1 ;
		src = grp->lit ;
		dst = ptr ;
		for ( j = grp->n_lights ; j > 0 ; src++, j-- ){
			if ( src->flag & LIT_FLAG_DISABLE ) continue ;
			if ( !( src->flag & LIT_FLAG_BGONLY ) ) continue ;
			/* さらに個々のあたり判定をチェックする */
			if ( ( src->bound_max.vx ) < mdl_min.vx || ( src->bound_min.vx ) > mdl_max.vx ||
				( src->bound_max.vy ) < mdl_min.vy || ( src->bound_min.vy ) > mdl_max.vy ||
				( src->bound_max.vz ) < mdl_min.vz || ( src->bound_min.vz ) > mdl_max.vz ){
				continue ;
			}
			*dst++ = *src ;
			work->n_spot++ ;
		}
		ptr = dst ;
	}
	/* 線光源検索 */
	work->n_line = 0 ;
	work->line = ptr ;
	for ( grp = lit_def->grp, i = lit_def->n_lit_group ; i > 0 ; grp++, i-- ){
		LIT_LINE	*src, *dst ;
		/* 属性チェック */
		if ( grp->type & LIT_TYPE_DISABLE ) continue ;
		if ( !( grp->type & LIT_TYPE_LINE ) ) continue ;
		/* バウンディングチェック */
		if ( !( grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz ) ) continue ;
		if ( grp->type & LIT_TYPE_CHANGE ) work->hit_flag = 1 ;
		src = grp->lit ;
		dst = ptr ;
		for ( j = grp->n_lights ; j > 0 ; src++, j-- ){
			if ( src->flag & LIT_FLAG_DISABLE ) continue ;
			if ( !( src->flag & LIT_FLAG_BGONLY ) ) continue ;
			/* さらに個々のあたり判定をチェックする */
			if ( ( src->bound_max.vx ) < mdl_min.vx || ( src->bound_min.vx ) > mdl_max.vx ||
				( src->bound_max.vy ) < mdl_min.vy || ( src->bound_min.vy ) > mdl_max.vy ||
				( src->bound_max.vz ) < mdl_min.vz || ( src->bound_min.vz ) > mdl_max.vz ){
				continue ;
			}
			*dst++ = *src ;
			work->n_line++ ;
		}
		ptr = dst ;
	}
	/* 黒点光源検索 */
	work->n_black = 0 ;
	work->blackpoints = ptr ;
	for ( grp = lit_def->grp, i = lit_def->n_lit_group ; i > 0 ; grp++, i-- ){
		LIT_BLACKPOINT	*src, *dst ;
		/* 属性チェック */
		if ( grp->type & LIT_TYPE_DISABLE ) continue ;
		if ( !( grp->type & LIT_TYPE_BLACKPOINT ) ) continue ;
		/* バウンディングチェック */
		if ( !( grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz ) ) continue ;
		if ( grp->type & LIT_TYPE_CHANGE ) work->hit_flag = 1 ;
		src = grp->lit ;
		dst = ptr ;
		for ( j = grp->n_lights ; j > 0 ; src++, j-- ){
			if ( src->flag & LIT_FLAG_DISABLE ) continue ;
			if ( !( src->flag & LIT_FLAG_BGONLY ) ) continue ;
			/* さらに個々のあたり判定をチェックする */
			if ( ( src->bound_max.vx ) < mdl_min.vx || ( src->bound_min.vx ) > mdl_max.vx ||
				( src->bound_max.vy ) < mdl_min.vy || ( src->bound_min.vy ) > mdl_max.vy ||
				( src->bound_max.vz ) < mdl_min.vz || ( src->bound_min.vz ) > mdl_max.vz ){
				continue ;
			}
			*dst++ = *src ;
			work->n_black++ ;
		}
		ptr = dst ;
	}
}

/*----------------------------------------------------------------*/


	/*
		プレシェイドバッファにＲＧＢ値を書き込む（シェイドなし）
	*/
static	CVECTOR	*NoShadeRGB( DG_MDL *mdl, CVECTOR *rgbs )
{
	DG_MDLPACK	*pack ;
	CVECTOR		code = {0x80,0x80,0x80,0x80};
	int		i, j ;

#ifndef NEW_KMX_FORMAT
	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		for ( j = pack->n_verts ; j > 0 ; j-- ) {
			*rgbs = code ;
			rgbs++ ;
		}
		pack++ ;
	}
#else
	for ( j = mdl->n_verts ; j > 0 ; j-- ) {
		*rgbs = code ;
		rgbs++ ;
	}
#endif
	return rgbs ;
}

	/*
		プレシェイドバッファにＲＧＢ値を書き込む（シェイドあり）
	*/
static	CVECTOR	*ShadeRGB( DG_MDL *mdl, CVECTOR *rgbs, LIT_DEF *lit_def )
{
	DG_MDLPACK	*pack ;
	int		i ;

	/* 対象となる光源データをスクラッチパッドへ転送する */
	CreateLightBuffer( mdl, lit_def );

#ifndef NEW_KMX_FORMAT
#if 1
	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		MakeLight( rgbs, (SVECTOR*)pack->verts, (SVECTOR*)pack->norms, pack->n_verts, NULL );
		rgbs += pack->n_verts ;
		pack++ ;
	}
	return rgbs ;
#else
	{
		int		total_verts, n_verts ;
		SVECTOR	*verts, *norms ;
		pack = mdl->packs ;
		total_verts = 0 ;
		for ( i = mdl->n_packs ; i > 0 ; i-- ){
			total_verts += ( pack->n_verts + 1 ) & 0xfffe ;
			pack++ ;
		}

		verts = (SVECTOR*)mdl->packs->verts ;
		norms = (SVECTOR*)mdl->packs->norms ;
		while ( total_verts ){
			n_verts = ( total_verts > 64 ) ? 64 : total_verts ;
			total_verts -= n_verts ;
			MakeLight( rgbs, verts, norms, n_verts, NULL );
			verts += n_verts ;
			norms += n_verts ;
			rgbs += n_verts ;
			
		}
	}
	return rgbs ;
#endif
#else
	MakeLight( rgbs, mdl->vbuff, mdl->n_verts, NULL );
	rgbs += mdl->n_verts ;
	return rgbs ;
#endif
}

	/*
		プレシェイドバッファの再計算
	*/
static	CVECTOR	*ReshadeRGB( DG_MDL *mdl, CVECTOR *rgbs, LIT_DEF *lit_def, int first_flag )
{
	DG_MDLPACK	*pack ;
	ScrpadWork *work = SCRPAD_ADDR ;
	int		i ;

	/* 対象となる光源データをスクラッチパッドへ転送する */
	CreateLightBuffer( mdl, lit_def );

	if ( work->hit_flag == 0 ) return ( rgbs );

#ifndef NEW_KMX_FORMAT
	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		MakeLight( rgbs, (SVECTOR*)pack->verts, (SVECTOR*)pack->norms, pack->n_verts, NULL );
		//rgbs += ( pack->n_verts + 1 ) & 0xfffe ;
		rgbs += pack->n_verts ;
		pack++ ;
	}
#else
	MakeLight( rgbs, mdl->vbuff, mdl->n_verts, NULL );
	rgbs += mdl->n_verts ;
#endif
	return rgbs ;
}

/*----------------------------------------------------------------*/

	/*
		プレシェイドバッファのサイズを計算する
	*/
static	int	SizeofRGB( DG_OBJS *objs )
{
	DG_MDL		*mdl ;
	DG_OBJ		*obj ;
	DG_OBJ_PACKET	*pack ;
	int		size ;
	int		i, j ;

	size = 0 ;
	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; -- i ) {
		mdl = obj->model ;
		pack = obj->packets ;
#ifndef NEW_KMX_FORMAT
		for ( j = obj->n_packs ; j > 0 ; j-- ){
			//size += ( pack->n_verts + 1 ) & 0xfffe ;
			size += pack->n_verts ;
			pack++ ;
		}
#else
		size += mdl->n_verts ;
#endif
		obj++ ;
	}
	return sizeof( int ) * size ;
}

/*----------------------------------------------------------------*/
	/*
		オブジェクトに対してプリシェード計算を行なう
	*/
int		DG_MakePreshade( DG_OBJS *objs, LIT_DEF *lit_def )
{
	FMATRIX		tmp_mat ;
	CVECTOR		*rgbs ;
	DG_DEF		*def ;
	DG_OBJ		*obj ;
	int		i, flag = 0, time ;


#if 0
	objs->flag &= ~DG_FLAG_PAINT ;
	for ( i = 0 ; i < objs->n_models ; i++ ){
		objs->objs[ i ].flag &= ~DG_FLAG_PAINT ;
	}
	return ;
#endif

	def = objs->def ;
	ASSERT( def != NULL ) ;
	/*
		プレシェイドバッファの確認
		なければ新たに確保する
	*/
	rgbs = objs->rgb_buff ;
	if( rgbs == NULL ){
		//objs->rgb_buff = rgbs = (CVECTOR *)DG_AllocLocalVideoMemory( SizeofRGB( objs ) ) ;
		objs->rgb_buff = rgbs = (CVECTOR *)GV_Malloc( SizeofRGB( objs ) ) ;
		if ( rgbs == NULL ) return -1 ;
		flag = 1 ;
	}

	/*
		プレシェイドデータを計算する
	*/
	tmp_mat = objs->world ;
	tmp_mat.m[3][0] += objs->def->tx ;
	tmp_mat.m[3][1] += objs->def->ty ;
	tmp_mat.m[3][2] += objs->def->tz ;
	obj = objs->objs;
	for( i = objs->n_models; i > 0; -- i, obj++ ){
		DG_MDL		*mdl ;
		obj->rgbs = rgbs ;
		mdl = obj->model;
		_SetMatrix( &tmp_mat );
		if ( ( DG_TYPE_NOSHADE & mdl->type ) || ( DG_FLAG_SHADOWVOL & objs->flag ) || ( lit_def == NULL ) ){
			rgbs = NoShadeRGB( mdl, NO_CACHED(rgbs) ) ;
		} else {
			rgbs = ShadeRGB( mdl, NO_CACHED(rgbs), lit_def ) ;
		}
	}
	//if ( flag == 0 ) return 0 ;

	/* カラーバッファを法線情報の代わりにする */
	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; -- i ) {
		obj->norms = obj->rgbs ;
		obj ++ ;
	}

	/* 自動再計算の為にポインタを記憶しておく */
	objs->fix_light = lit_def ;

	return 0 ;
}

	/*
		ライトの変更部分のみに対してプリシェード計算を行なう
	*/
int		DG_MakePreshadeParts( DG_OBJS *objs, LIT_DEF *lit_def )
{
	FMATRIX		tmp_mat ;
	DG_OBJ		*obj ;
	int		i, time, first_flag ;

	first_flag = 1 ;
	/*
		プレシェイドデータを計算する
	*/
	tmp_mat = objs->world ;
	tmp_mat.m[3][0] += objs->def->tx ;
	tmp_mat.m[3][1] += objs->def->ty ;
	tmp_mat.m[3][2] += objs->def->tz ;
	obj = objs->objs;
	for( i = objs->n_models; i > 0; -- i, obj++ ){
		DG_MDL		*mdl ;
		mdl = obj->model;
		_SetMatrix( &tmp_mat );
		if ( DG_TYPE_NOSHADE & mdl->type ) continue ;

		ReshadeRGB( mdl, NO_CACHED(obj->rgbs), lit_def, first_flag ) ;
		first_flag = 0 ;
	}

	return 0 ;
}

void		DG_FreePreshade( DG_OBJS *objs )
{
	CVECTOR		*rgbs ;

	if ( !( objs->flag & DG_FLAG_PAINT ) ) return ;

	rgbs = objs->rgb_buff;
	if( rgbs != NULL ){
		//DG_DelayedFreeLocalVideoMemory( rgbs );
		GV_DelayedFree( rgbs );
		objs->rgb_buff = NULL;
	}
}



/*----------------------------------------------------------------*/

	/*
		プレシェイドバッファにＲＧＢ値を書き込む（シェイドあり）
	*/
void DG_TempPreshadeRGB( DG_OBJ *obj, LIT_DEF *lit_def, int which, int start_flag )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*mdl_pack ;
	int		i, packet_size ;
	CVECTOR		*tmp_color, *org_rgbs ;
	ScrpadWork *work = SCRPAD_ADDR ;

	mdl = obj->model ;

	/* 対象となる光源データをスクラッチパッドへ転送する */
	_SetMatrix( &obj->world );
	CreateLightBuffer( mdl, lit_def );

	if ( ( work->n_point + work->n_spot + work->n_black ) == 0 ){
		return ;
	}

#ifndef NEW_KMX_FORMAT
	/* パケットサイズ計算 */
	packet_size = 0 ;
	mdl_pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; mdl_pack++, i-- ){
		//packet_size += ( mdl_pack->n_verts + 1 ) & 0xfffe ;
		packet_size += mdl_pack->n_verts ;
	}
	packet_size = ( packet_size + 3 ) & ~3 ;

	/* メモリ確保（ＸＢＯＸ版は動的頂点バッファメモリから確保） */
	tmp_color = DG_AllocDynamicVertexBuffer( sizeof(CVECTOR), packet_size );

	obj->vanime_flag &= ~DG_VANIME_DELNORMS ;	/* 法線アドレス復帰フラグを消しておく */
	obj->vanime_flag |= DG_VANIME_NORMS ;	/* 一時法線（＝頂点カラー）アニメのフラグを立てる */
	obj->norms = tmp_color ;
	mdl_pack = mdl->packs ;
	org_rgbs = obj->rgbs ;
	for ( i = obj->n_packs ; i > 0 ; i-- ){
		MakeLight( tmp_color, (SVECTOR*)mdl_pack->verts, (SVECTOR*)mdl_pack->norms, mdl_pack->n_verts, org_rgbs );
		tmp_color += mdl_pack->n_verts ;
		org_rgbs += mdl_pack->n_verts ;
		//count += mdl_pack->n_verts ;
		mdl_pack++ ;
	}
#else
	/* パケットサイズ計算 */
	packet_size = mdl->n_verts ;

	/* メモリ確保（ＸＢＯＸ版は動的頂点バッファメモリから確保） */
	tmp_color = DG_AllocDynamicVertexBuffer( sizeof(CVECTOR), packet_size );

	obj->vanime_flag &= ~DG_VANIME_DELNORMS ;	/* 法線アドレス復帰フラグを消しておく */
	obj->vanime_flag |= DG_VANIME_NORMS ;	/* 一時法線（＝頂点カラー）アニメのフラグを立てる */
	obj->norms = tmp_color ;
	org_rgbs = obj->rgbs ;
	MakeLight( tmp_color, mdl->vbuff, mdl->n_verts, org_rgbs );
#endif
	return ;
}


/*----------------------------------------------------------------*/

	/*
		テンポラリライトプリシェーディングを行なう
	*/
void		DG_TmpLightPreshadeChanl( DG_CHANL *cp, int which )
{
	DG_TLIGHT *flp;
	DG_OBJ_QUEUE	*queue ;
	DG_OBJS	**oque, *objs ;
	DG_OBJ_BUFFER	*obj_buff ;
	int			i, j, start_flag = 1 ;
	extern DG_TLIGHT	DG_TLights[ 2 ] ;
	extern int			DG_LightClock ;
	//int			time ;

	flp = &DG_TLights[ 1 - DG_LightClock ] ;

	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->objs_buffer ;

	if ( ( flp->TmpLightGrp[0].n_lights + flp->TmpLightGrp[1].n_lights + flp->TmpLightGrp[2].n_lights ) == 0 ) return ;

	//printf("tmp preshade start\n");
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; i-- ){
		objs = *oque++ ;
		if ( !( objs->flag & DG_FLAG_PAINT ) ) continue ;
		//printf("%s: %08x\n", objs->fname, objs );
		for ( j = 0 ; j < objs->n_models ; j++ ){
			if ( objs->objs[j].rgbs == NULL ) continue ;
			DG_TempPreshadeRGB( &objs->objs[j], &flp->TmpLightDef, which, start_flag );
			start_flag = 0 ;
		}
	}
}

/*----------------------------------------------------------------*/
static	void	FreeVanimeObjs( DG_OBJS *objs, int which )
{
	DG_OBJ		*obj ;
	int			i ;

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		if ( !( objs->flag & DG_FLAG_PAINT ) ){
			if ( obj->vanime_flag & DG_VANIME_DELVERTS ) obj->verts = (SVECTOR*)obj->model->packs->verts ;
			if ( obj->vanime_flag & DG_VANIME_DELNORMS ) obj->norms = (SVECTOR*)obj->model->packs->norms ;
			if ( obj->vanime_flag & DG_VANIME_DELUVS ) obj->uvs[0] = (short*)obj->model->packs->uvs ;
		} else {
			if ( obj->vanime_flag & DG_VANIME_DELVERTS ) obj->verts = (SVECTOR*)obj->model->packs->verts ;
			if ( obj->vanime_flag & DG_VANIME_DELNORMS ) obj->norms = obj->rgbs ;
			if ( obj->vanime_flag & DG_VANIME_DELUVS ) obj->uvs[0] = (short*)obj->model->packs->uvs ;
		}
		obj->vanime_flag = ( obj->vanime_flag & ~0xff ) | ( ( obj->vanime_flag << 4 ) & 0xf0 ) ;
	}
}
	/*
		自動復元指定された頂点アニメデータアドレスの復元
	*/
void	DG_RefreshVAnimeChanl( DG_CHANL *cp, int which )
{
	DG_OBJ_QUEUE	*queue ;
	DG_OBJS	**oque, *objs ;
	DG_OBJ_BUFFER	*obj_buff ;
	int			i ;
	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->objs_buffer ;

	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; i-- ){
		objs = *oque++ ;
		//if ( !( objs->flag & DG_FLAG_PAINT ) ) continue ;
		FreeVanimeObjs( objs, which );
	}

}


#endif
