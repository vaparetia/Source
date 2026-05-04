//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------

/*
	pshade.c
	プレシェイド（ＲＧＢ前処理計算）ルーチン

	1999/07/07 K.Takabe
	$Id: pshade.c,v 1.1.1.3 2002/11/19 11:42:21 Yoshizawa1 Exp $

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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

#include "BP_RenderObj.h"

#define NO_CACHED(a) a

/*----------------------------------------------------------------*/

#if 1//BP_ASM - micro code storage

typedef struct  
{
   FMATRIX  matrix;     //vf04,vf05,vf06,vf07
   FVECTOR  bound_max;  //vf09
   FVECTOR  bound_min;  //vf10

} MicroCodeData;

static MicroCodeData gMicroCodeData;
int gBP_EnableDynamicLighting = 1;

#endif

/*----------------------------------------------------------------*/

typedef struct pshade_ScrpadWork {
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

typedef struct {
   FVECTOR		bound_max ;	/**< バウンディング（最大） */
   FVECTOR		bound_min ;	/**< バウンディング（最小） */
   FVECTOR		dir ;		/**< 光源方向 */
   CVECTOR		color ;		/**< 光源色 */
   CVECTOR		ambient ;	/**< 光源色 */
   float		force ;		/**< 光源強さ */
   int			flag ;		/**< フラグ */
} LIT_PARALLEL ;

/*----------------------------------------------------------------*/
#if 1//BP_ASM
static inline float VEC_LEN( float _x, float _y, float _z )
{
   FVECTOR v;
   v.vx = _x;
   v.vy = _y;
   v.vz = _z;
   return BP_Vec3_Length( &v );
}

#define FABS(_x) fabs(_x)

#else

/* ベクトルの長さを求める */
#define VEC_LEN(_x,_y,_z) ({float _a; \
	asm( "mula.s %1,%1 ; madda.s %2,%2 ; madd.s %0,%3,%3 ; sqrt.s %0,%0 ":"+f"(_a):"f"(_x),"f"(_y),"f"(_z) ); \
	(_a);})

/* 絶対値を取る */
#define FABS(_x) ({float _a = _x;asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})

#endif

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

#if 0 //BP_ASM
extern qword PreshadeVu0 ;
extern qword PreshadeVu0_Func ;
#endif


/* ---------------------------------------------------------------- */
	/*
		計算マイクロプログラム読み込み
	*/
static inline void LoadMicroProgram( void )
{
#if 0 //BP_ASM - not needed. Replaced vu0 program with in place C
	DG_DmaStartCheck( 0 );
	DPUT_D0_MADR( (int)((DG_DMATAG*)PreshadeVu0)->addr ) ;
	DPUT_D0_QWC( ( ((DG_DMATAG*)PreshadeVu0)->qwc ) & 0x7fff ) ;
	//DPUT_D0_CHCR( 0x0141 ) ; /* STR:1 TIE:0 TTE:1 ASP:0 MOD:0 DIR:1 */
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D0_CHCR),"r"(0x141) );
#endif
}

	/*
		ＶＩＦ０ＤＭＡ転送終了待ち
	*/
static inline void WaitLoadMicroProgram( void )
{
#if 0 //BP_ASM - not needed. Replaced vu0 program with in place C
	DG_WaitDma( 0 );
#endif
}
/*----------------------------------------------------------------*/

static inline void _SetMatrix( FMATRIX *m )
{
#if 1//BP_ASM
   BP_Mat_Copy( &gMicroCodeData.matrix, m );
#else
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	": : "r"(m) );
#endif
}

static inline void _SetMatrixOffset( FMATRIX *m, FVECTOR *offset )
{
   BP_Mat_TransformVec3( offset, m, offset );
}

static inline void _SetDefaultBound( FVECTOR *max, FVECTOR *min )
{
#if 1//BP_ASM
   BP_Vec4_Copy( &gMicroCodeData.bound_max, max );
   BP_Vec4_Copy( &gMicroCodeData.bound_min, min );
#else
	asm("
		lqc2		vf9,0(%0)	# bound_max
		lqc2		vf10,0(%1)	# bound_max
	"::"r"(max),"r"(min));
#endif
}

static inline void _GetBound( FVECTOR *max, FVECTOR *min )
{
#if 1//BP_ASM
   BP_Vec4_Copy( max, &gMicroCodeData.bound_max );
   BP_Vec4_Copy( min, &gMicroCodeData.bound_min );
#else
	asm("
		sqc2		vf9,0(%0)	# bound_max
		sqc2		vf10,0(%1)	# bound_max
	"::"r"(max),"r"(min):"memory");
#endif
}

static inline void _RotTransBoundCheck( FVECTOR *v )
{
#if 1//BP_ASM
   FVECTOR tv;
   BP_Mat_TransformVec3( &tv, &gMicroCodeData.matrix, v );
   BP_Vec3_MaxVec( &gMicroCodeData.bound_max, &gMicroCodeData.bound_max, &tv );
   BP_Vec3_MinVec( &gMicroCodeData.bound_min, &gMicroCodeData.bound_min, &tv );
#else
	asm ("
	lqc2			vf8,0x00(%0)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf0
	vmax.xyz		vf9,vf9,vf8
	vmini.xyz		vf10,vf10,vf8
	":: "r"(v) );
#endif
}
static void MakeBound( FVECTOR *rot_bound_max, FVECTOR *rot_bound_min, FVECTOR *bound_max, FVECTOR *bound_min )
{
   BP_Mat_TransformBound3( rot_bound_min, rot_bound_max, &gMicroCodeData.matrix, bound_min, bound_max );
}
static inline int CheckBoundHit( FVECTOR *max0, FVECTOR *min0, FVECTOR *max1, FVECTOR *min1 )
{
   // Check valid
   bp_math_assert( BP_Vec3_Check(max0) );
   bp_math_assert( BP_Vec3_Check(min0) );
   bp_math_assert( BP_Vec3_Check(max1) );
   bp_math_assert( BP_Vec3_Check(min1) );

	if ( (max0->vx >= min1->vx) && (max1->vx > min0->vx) &&
		(max0->vy >= min1->vy) && (max1->vy > min0->vy) &&
		(max0->vz >= min1->vz) && (max1->vz > min0->vz) )
   {
		return ( 1 );
	} 
   else 
   {
		return ( 0 );
	}
}

// pshadevu.dsm, ParallelAmbientCalc
void BP_ParallelAmbientCalc( const FVECTOR* vert_pos, const FVECTOR* vert_norm, const LIT_PARALLEL* parallel, FVECTOR* color )
{
   float i;

   // Check valid
   bp_math_assert( BP_Vec3_Check(vert_pos) );
   bp_math_assert( BP_Vec3_Check(vert_norm) );
   bp_math_assert( BP_Vec3_Check(&parallel->bound_min) );
   bp_math_assert( BP_Vec3_Check(&parallel->bound_max) );
   bp_math_assert( BP_Vec3_Check(&parallel->dir) );

   // Outside of light bounds?
   if ( ( parallel->bound_max.vx < vert_pos->vx ) || ( parallel->bound_min.vx > vert_pos->vx ) ) return;
   if ( ( parallel->bound_max.vy < vert_pos->vy ) || ( parallel->bound_min.vy > vert_pos->vy ) ) return;
   if ( ( parallel->bound_max.vz < vert_pos->vz ) || ( parallel->bound_min.vz > vert_pos->vz ) ) return;

   // Compute directional intensity
   i = (vert_norm->vx * parallel->dir.vx) + 
      (vert_norm->vy * parallel->dir.vy) +
      (vert_norm->vz * parallel->dir.vz);
   if( i < 0.0f )
   {
      i = 0.0f;
   }

   // Set color directly since this is always the first light
   color->vx = (float)parallel->ambient.r + ((float)parallel->color.r * i);
   color->vy = (float)parallel->ambient.g + ((float)parallel->color.g * i);
   color->vz = (float)parallel->ambient.b + ((float)parallel->color.b * i);
}

// pshadevu.dsm : PointLightCalc
void BP_PointLightCalc( const FVECTOR* vert_pos, const FVECTOR* vert_norm, const LIT_POINT* lit, FVECTOR* color )
{
   FVECTOR delta, dir;
   float	len, i, range, bound_range;

   // Check valid
   bp_math_assert( BP_Vec3_Check(vert_pos) );
   bp_math_assert( BP_Vec3_Check(vert_norm) );
   bp_math_assert( BP_Vec3_Check(&lit->point) );
   bp_math_assert( BP_Float_Check(lit->r_range) );
   bp_math_assert( BP_Float_Check(lit->e_range) );

   // Compute delta from light
   BP_Vec3_SubVec( &delta, vert_pos, &lit->point );

   // Outside of light bounds?
   bound_range = lit->e_range;
   if(      ( FABS( delta.vx ) > bound_range )
         || ( FABS( delta.vy ) > bound_range )
         || ( FABS( delta.vz ) > bound_range ) )
   {
      return;
   }
   
   // Compute distance from point light
   len = BP_Vec3_Length( &delta );

   // Outside of light range?
   range = lit->r_range * 2;
   if( len >= range )
   {
      return;
   }

   // Compute directional intensity
   BP_Vec3_MulFloat( &dir, &delta, (len > 0.000001f) ? (1.0f / len) : 0.0f );
   i = (vert_norm->vx * dir.vx) + 
       (vert_norm->vy * dir.vy) + 
       (vert_norm->vz * dir.vz);
   if( i < 0.0f )
   {
      return;
   }

   // Apply distance fade
   i *= 1.5f * (range - len) / range;

   // Apply to final color
   color->vx += (float)lit->color.r * i;
   color->vy += (float)lit->color.g * i;
   color->vz += (float)lit->color.b * i;
}

// pshadevu.dsm: SpotLightCalc
void BP_SpotLightCalc( const FVECTOR* vert_pos, const FVECTOR* vert_norm, const LIT_SPOT* spot, FVECTOR* color )
{
   FVECTOR delta, dir;
   float	len, i, range, cos_angle;

   // Check valid
   bp_math_assert( BP_Vec3_Check(vert_pos) );
   bp_math_assert( BP_Vec3_Check(vert_norm) );
   bp_math_assert( BP_Vec3_Check(&spot->bound_max) );
   bp_math_assert( BP_Vec3_Check(&spot->bound_min) );
   bp_math_assert( BP_Vec3_Check(&spot->point) );
   bp_math_assert( BP_Vec3_Check(&spot->dir) );
   bp_math_assert( BP_Float_Check(spot->umbra) );
   bp_math_assert( BP_Float_Check(spot->penumbra) );

   // Outside of light bounds?
   if ( vert_pos->vx > spot->bound_max.vx || vert_pos->vx < spot->bound_min.vx ) return;
   if ( vert_pos->vy > spot->bound_max.vy || vert_pos->vy < spot->bound_min.vy ) return;
   if ( vert_pos->vz > spot->bound_max.vz || vert_pos->vz < spot->bound_min.vz ) return;

   // Compute distance from spot light
   BP_Vec3_SubVec( &delta, vert_pos, &spot->point );
   len = BP_Vec3_Length( &delta );

   // Outside of light range?
   range = spot->dir.vw * 2;
   if( len >= range )
   {
      return;
   }

   // Compute directional intensity
   BP_Vec3_MulFloat( &dir, &delta, (len > 0.000001f) ? (1.0f / len) : 0.0f );
   i = (vert_norm->vx * dir.vx) + 
       (vert_norm->vy * dir.vy) + 
       (vert_norm->vz * dir.vz);
   if( i < 0.0f )
   {
      return;
   }

   // Apply distance fade
   i *= 1.5f * (range - len) / range;

   // Apply angle fade
   cos_angle = BP_Vec3_InnerProduct( &dir, &spot->dir );
   if( cos_angle < spot->penumbra )
   {
      return;
   }
   else if( cos_angle < spot->umbra )
   {
      i *= (spot->penumbra - cos_angle) / (spot->penumbra - spot->umbra);
   }

   // Apply to final color
   color->vx += (float)spot->color.r * i;
   color->vy += (float)spot->color.g * i;
   color->vz += (float)spot->color.b * i;
}

/*----------------------------------------------------------------*/

void BP_LineLightCalc( const FVECTOR* vert_pos, const FVECTOR* vert_norm, const LIT_LINE* line, FVECTOR* color )
{
   FVECTOR start, end, point_on_line, delta, dir;
   float n1, n2, n3, len, range, i;

   // Check valid
   bp_math_assert( BP_Vec3_Check(vert_pos) );
   bp_math_assert( BP_Vec3_Check(vert_norm) );
   bp_math_assert( BP_Vec3_Check(&line->bound_max) );
   bp_math_assert( BP_Vec3_Check(&line->bound_min) );
   bp_math_assert( BP_Vec3_Check(&line->point) );
   bp_math_assert( BP_Vec3_Check(&line->dir) );
   bp_math_assert( BP_Float_Check(line->r_range) );

   // Branch tests are slower than just doing the work for PS3
   // Outside of light bounds?
   //if ( vert_pos->vx > line->bound_max.vx || vert_pos->vx < line->bound_min.vx ) return;
   //if ( vert_pos->vy > line->bound_max.vy || vert_pos->vy < line->bound_min.vy ) return;
   //if ( vert_pos->vz > line->bound_max.vz || vert_pos->vz < line->bound_min.vz ) return;

   // Compute line start, end points
   BP_Vec3_Copy( &start, &line->point );
   BP_Vec3_MulAddFloat( &end, &line->point, &line->dir, line->dir.vw );

   // Compute nearest point on line to vertex
   n1 = BP_Vec3_InnerProduct( &start,   &line->dir );
   n2 = BP_Vec3_InnerProduct( &end,     &line->dir );
   n3 = BP_Vec3_InnerProduct( vert_pos, &line->dir );
   if( n3 < n1 ) n3 = n1;
   if( n3 > n2 ) n3 = n2;
   BP_Vec3_MulAddFloat( &point_on_line, &start, &line->dir, n3 - n1 );

   // Compute distance from line point to vertex
   BP_Vec3_SubVec( &delta, vert_pos, &point_on_line );
   len = BP_Vec3_Length( &delta );

   // Outside of light range?
   range = line->r_range * 2;
   if( len >= range )
   {
      return;
   }

   // Compute directional intensity
   BP_Vec3_MulFloat( &dir, &delta, 1.0f / len );
   i = (vert_norm->vx * dir.vx) + 
      (vert_norm->vy * dir.vy) + 
      (vert_norm->vz * dir.vz);
   if( i < 0.0f )
   {
      return;
   }

   // Apply distance fade
   i *= 1.5f * (range - len) / range;

   // Apply to final color
   color->vx += (float)line->color.r * i;
   color->vy += (float)line->color.g * i;
   color->vz += (float)line->color.b * i;
}

// pshadevu.dsm: BlackLightCalc
int BP_BlackLightCalc( const FVECTOR* vert_pos, const FVECTOR* vert_norm, const LIT_BLACKPOINT* black, FVECTOR* color )
{
   FVECTOR delta;
   float len, i, range;

   // Check valid
   bp_math_assert( BP_Vec3_Check(vert_pos) );
   bp_math_assert( BP_Vec3_Check(vert_norm) );
   bp_math_assert( BP_Vec3_Check(&black->bound_max) );
   bp_math_assert( BP_Vec3_Check(&black->bound_min) );
   bp_math_assert( BP_Vec3_Check(&black->point) );
   bp_math_assert( BP_Float_Check(black->r_range) );

   // Outside of light bounds?
   if ( ( black->bound_max.vx <= vert_pos->vx ) || ( black->bound_min.vx >= vert_pos->vx ) ) return 0;
   if ( ( black->bound_max.vy <= vert_pos->vy ) || ( black->bound_min.vy >= vert_pos->vy ) ) return 0;
   if ( ( black->bound_max.vz <= vert_pos->vz ) || ( black->bound_min.vz >= vert_pos->vz ) ) return 0;

   BP_Vec3_SubVec( &delta, vert_pos, &black->point );
   len = BP_Vec3_Length( &delta );

   range = black->r_range * 2;

   if ( len < range )
   {
      i = 1.0f - (len / range);

#if 0 //BP NOTE
      //This is what the PS2 code claims it does
      color.vx = color.vx * i;
      color.vy = color.vy * i;
      color.vz = color.vz * i;
#else
      //This is what it ACTUALLY does.
      color->vx = color->vx - color->vx * i;
      color->vy = color->vy - color->vy * i;
      color->vz = color->vz - color->vz * i;
#endif
   }
   
   // This light was processed (NOTE: Only the very first black light is applied to a given vertex!)
   return 1;
}

/*----------------------------------------------------------------*/
	/*
		各頂点についての照明データを計算する

	*/
static void MakeLight( u_int *rgbs, SVECTOR *verts, SVECTOR *norms, int n_verts )
{
	/*
		新バージョンプリシェード計算ルーチン
	*/
#if 0//BP_ASM - not needed - loop pre-load
	FVECTOR		one = {1.0F,1.0F,1.0F,255.0F}, two = {1.5f,-0.75f,0.0f,0.0f}, color ;
#endif
	LIT_POINT	*lit ;
	LIT_SPOT	*spot ;
	LIT_LINE	*line ;
	LIT_BLACKPOINT	*black ;
	u_int    tmp ;
	int		i, count ;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR ;

#if 1//BP_ASM
   LIT_PARALLEL globalParallel;
   FVECTOR  vert_pos, vert_norm, color;

   // Reset color
   BP_Vec4_SetZero( &color );

   // Setup global parallel light
   BP_Vec4_Set( &globalParallel.bound_min, -FLT_MAX, -FLT_MAX, -FLT_MAX, 0.0f );
   BP_Vec4_Set( &globalParallel.bound_max, +FLT_MAX, +FLT_MAX, +FLT_MAX, 0.0f );
   globalParallel.dir.vx = DG_LightMatrix.m[0][0];
   globalParallel.dir.vy = DG_LightMatrix.m[1][0];
   globalParallel.dir.vz = DG_LightMatrix.m[2][0];
   globalParallel.color.r = (unsigned char)DG_ColorMatrix.m[0][0];
   globalParallel.color.g = (unsigned char)DG_ColorMatrix.m[0][1];
   globalParallel.color.b = (unsigned char)DG_ColorMatrix.m[0][2];
   globalParallel.color.cd = (unsigned char)DG_ColorMatrix.m[0][3];
   globalParallel.ambient.r = (unsigned char)DG_ColorMatrix.m[3][0];
   globalParallel.ambient.g = (unsigned char)DG_ColorMatrix.m[3][1];
   globalParallel.ambient.b = (unsigned char)DG_ColorMatrix.m[3][2];
   globalParallel.ambient.cd = (unsigned char)DG_ColorMatrix.m[3][3];

#endif

#if 0//BP_ASM - not needed - loop pre-load
	/* ＶＵ０レジスタ初期化 */
	asm ("
		lqc2	vf24,0(%0)			# vf24 = 平行光源ベクトル
		lqc2	vf25,16(%0)			# vf25 = 平行光源ベクトル
		lqc2	vf26,32(%0)			# vf26 = 平行光源ベクトル
		lqc2	vf27,64(%0)			# vf27 = 平行光源カラー
		lqc2	vf31,64+48(%0)		# vf10 = アンビエントカラー
		lqc2	vf3,0(%1)			# vf3 = 1.0, 1.0, 1.0, 255.0
		lqc2	vf2,0(%2)			# vf2 = 2.0, 0.0, 0.0, 0.0
	"::"r"(&DG_LightMatrix),"r"(&one),"r"(&two));
#endif

	while ( -- n_verts >= 0 ) {
		/* 座標変換 */

#if 0
		asm volatile ("
			ld		$10,0(%0)
			ld		$11,0(%1)
			pextlh	$10,$10,$0		# 各要素を３２ビットに符号拡張してＶＵ０へ渡す
			pextlh	$11,$11,$0		# 
			psraw	$10,$10,16		# 
			psraw	$11,$11,16		# 
			qmtc2	$10,vf8			# 
			qmtc2	$11,vf9			# 
			vcallms	0x00			# 
			qmtc2.i	$11,vf0			# 
			sqc2 vf8,0(%2)			# 
		"::"r"(verts),"r"(norms),"r"(&work->w_vert):"$10","$11","memory");
#else
      // Reset color
      BP_Vec4_SetZero( &color );
      color.w = 128.0f;

      // call 0x00
      // pshadevu.dsm : StartPreshade

      // Lookup vertex local position, normal
      BP_Vec4_ShortToFloat0( &vert_pos, (const SVECTOR*)&verts->vx );
      BP_Vec4_ShortToFloat12( &vert_norm, (const SVECTOR*)&norms->vx );

      // Transform into world space
      BP_Mat_TransformVec3( &vert_pos, &gMicroCodeData.matrix, &vert_pos );
      BP_Mat_RotateVec3( &vert_norm, &gMicroCodeData.matrix, &vert_norm );

      // Start with global parallel light
      BP_ParallelAmbientCalc( &vert_pos, &vert_norm, &globalParallel, &color );

#endif
		/* 点光源 */
		count = 0 ;
		lit = work->points ;
		for ( i = work->n_point ; i > 0 ; -- i, lit++ ) {

      #if 0//BP_ASM
			float	f, r ;
			//r = lit->r_range * 2.0f ;
			r = lit->e_range ;
			work->w_vec.vx = f = work->w_vert.vx - lit->point.vx ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vy = f = work->w_vert.vy - lit->point.vy ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vz = f = work->w_vert.vz - lit->point.vz ;
			if ( FABS( f ) > r ) continue ;
			work->w_param.vy = lit->r_range * 2 ;
			asm ("
				lw			$4,0(%2)
				pextlb		$4,$0,$4
				pextlh		$4,$0,$4
				qmtc2.i		$4,vf18
				lqc2		vf16,0(%0)
				lqc2		vf17,0(%1)
				vcallms		0x10
			"::"r"(&work->w_vec),"r"(&work->w_param),"r"(&lit->color):"$4" );
      #else
         BP_PointLightCalc( &vert_pos, &vert_norm, lit, &color );
      #endif
			//if ( ++count >= 3 ) break ;
		}

		/* スポット光源 */
		spot = work->spot ;
		for ( i = work->n_spot ; i > 0 ; -- i, spot++ ) {

   #if 0//BP_ASM

			float	r ;
			r = spot->dir.vw * 2.0f ;
#if 0
			/* バウンディングボックス非対応バージョン */
			work->w_vec.vx = f = work->w_vert.vx - spot->point.vx ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vy = f = work->w_vert.vy - spot->point.vy ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vz = f = work->w_vert.vz - spot->point.vz ;
			if ( FABS( f ) > r ) continue ;
#else
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > spot->bound_max.vx || work->w_vert.vx < spot->bound_min.vx ) continue ;
			if ( work->w_vert.vy > spot->bound_max.vy || work->w_vert.vy < spot->bound_min.vy ) continue ;
			if ( work->w_vert.vz > spot->bound_max.vz || work->w_vert.vz < spot->bound_min.vz ) continue ;
			work->w_vec.vx = work->w_vert.vx - spot->point.vx ;
			work->w_vec.vy = work->w_vert.vy - spot->point.vy ;
			work->w_vec.vz = work->w_vert.vz - spot->point.vz ;
#endif
			work->w_param.vy = r ;
			work->w_param.vz = spot->umbra ;
			work->w_param.vw = spot->penumbra ;
			asm ("
				lw			$4,0(%2)
				pextlb		$4,$0,$4
				pextlh		$4,$0,$4
				qmtc2.i		$4,vf18
				lqc2		vf16,0(%0)
				lqc2		vf17,0(%1)
				lqc2		vf19,0(%3)
				vcallms		0x20
			"::"r"(&work->w_vec),"r"(&work->w_param),"r"(&spot->color),"r"(&spot->dir):"$4" );
   #else
         BP_SpotLightCalc( &vert_pos, &vert_norm, spot, &color );
   #endif
		}

		/* 線光源 */
		line = work->line ;
		for ( i = work->n_line ; i > 0 ; -- i, line++ ) {
#if 0//BP_ASM
			float	r ;
			r = line->dir.vw * 2.0f ;
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > line->bound_max.vx || work->w_vert.vx < line->bound_min.vx ) continue ;
			if ( work->w_vert.vy > line->bound_max.vy || work->w_vert.vy < line->bound_min.vy ) continue ;
			if ( work->w_vert.vz > line->bound_max.vz || work->w_vert.vz < line->bound_min.vz ) continue ;
			work->w_param.vy = line->r_range * 2 ;
			asm ("
				lw			$4,0(%2)
				pextlb		$4,$0,$4
				pextlh		$4,$0,$4
				qmtc2.i		$4,vf18
				lqc2		vf16,0(%0)
				lqc2		vf17,0(%1)
				lqc2		vf19,0(%3)
				lqc2		vf08,0(%4)
				vcallms		0x50
			"::"r"(&line->point),"r"(&work->w_param),"r"(&line->color),"r"(&line->dir),"r"(&work->w_vert):"$4" );
   #else
         BP_LineLightCalc( &vert_pos, &vert_norm, line, &color );
   #endif
		}

		/* 黒点光源 */
		black = work->blackpoints ;
		for ( i = work->n_black ; i > 0 ; -- i, black++ ) {
#if 0//BP_ASM
			if ( ( black->bound_max.vx <= work->w_vert.vx ) || ( black->bound_min.vx >= work->w_vert.vx ) ) continue ;
			if ( ( black->bound_max.vy <= work->w_vert.vy ) || ( black->bound_min.vy >= work->w_vert.vy ) ) continue ;
			if ( ( black->bound_max.vz <= work->w_vert.vz ) || ( black->bound_min.vz >= work->w_vert.vz ) ) continue ;
			work->w_vec.vx = work->w_vert.vx - black->point.vx ;
			work->w_vec.vy = work->w_vert.vy - black->point.vy ;
			work->w_vec.vz = work->w_vert.vz - black->point.vz ;
			work->w_param.vy = black->r_range * 2 ;
			asm ("
				qmtc2.i		$4,vf18
				lqc2		vf16,0(%0)
				lqc2		vf17,0(%1)
				vcallms		0x30
			"::"r"(&work->w_vec),"r"(&work->w_param):"$4" );
         break;
   #else
         // IMPORTANT: Only break out of the loop if the black light was actually applied (i.e. bounds check succeeded)!
         if( BP_BlackLightCalc( &vert_pos, &vert_norm, black, &color ) )
            break;
   #endif
		}

   #if 0//BP_ASM
		asm ("
			vnop			# 終了ウェイト
			vminiw.xyz		vf10,vf10,vf3
			vftoi0.xyzw		vf10,vf10
			qmfc2			$10,vf10
			ppach			$10,$0,$10
			move			%0,$10
		":"=r"(tmp)::"$10");
   #else
      // Convert float color to final integer rgb
      tmp = BP_Vec4_ToColor( &color );
   #endif

		*rgbs = tmp ;
		verts++ ;
		norms++ ;
		rgbs++ ;
	}
}

/*----------------------------------------------------------------*/
	/*
		モデルのバウンディングに触れる全光源をスクラッチパッドへ転送する
	*/
static void CreateLightBuffer( DG_MDL *mdl, LIT_DEF *lit_def )
{
#if 0//BP_LIGHTING - not needed
	static FVECTOR	_bound_max = {-99999999.0f,-99999999.0f,-99999999.0f,0.0f};
	static FVECTOR	_bound_min = {99999999.0f,99999999.0f,99999999.0f,0.0f};
#endif
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR ;
	int		i, j ;
	LIT_GRP	*grp ;
	void	*ptr ;
	FVECTOR		mdl_max, mdl_min ;
#if 0
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
#else
	MakeBound( &mdl_max, &mdl_min, (FVECTOR*)&mdl->ux, (FVECTOR*)&mdl->lx );
#endif

	work->hit_flag = 0 ;
	ptr = work->buffer ;
	/* 点光源検索 */
	work->n_point = 0 ;
	work->points = ptr ;

   //BP_MATH - Check valid
   bp_math_assert( BP_Vec3_Check(&lit_def->dir) );

	for ( grp = lit_def->grp, i = lit_def->n_lit_group ; i > 0 ; grp++, i-- ){
		LIT_POINT	*src, *dst ;

      //BP_MATH - Check valid
      bp_math_assert( BP_Vec3_Check(&grp->bound_max) );
      bp_math_assert( BP_Vec3_Check(&grp->bound_min) );

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
#if 0
		if ( !( grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz ) ) continue ;
#else
		if ( CheckBoundHit( &grp->bound_max, &grp->bound_min, &mdl_max, &mdl_min ) == 0 ) continue ;
#endif
		if ( grp->type & LIT_TYPE_CHANGE ) work->hit_flag = 1 ;
		src = grp->lit ;
		dst = ptr ;
		for ( j = grp->n_lights ; j > 0 ; src++, j-- ){
			if ( src->flag & LIT_FLAG_DISABLE ) continue ;
			if ( !( src->flag & LIT_FLAG_BGONLY ) ) continue ;
			/* さらに個々のあたり判定をチェックする */
#if 0
			if ( ( src->bound_max.vx ) < mdl_min.vx || ( src->bound_min.vx ) > mdl_max.vx ||
				( src->bound_max.vy ) < mdl_min.vy || ( src->bound_min.vy ) > mdl_max.vy ||
				( src->bound_max.vz ) < mdl_min.vz || ( src->bound_min.vz ) > mdl_max.vz ){
				continue ;
			}
#else
			if ( CheckBoundHit( &src->bound_max, &src->bound_min, &mdl_max, &mdl_min ) == 0 ) continue ;
#endif
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
#if 0
		if ( !( grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz ) ) continue ;
#else
		if ( CheckBoundHit( &grp->bound_max, &grp->bound_min, &mdl_max, &mdl_min ) == 0 ) continue ;
#endif
		if ( grp->type & LIT_TYPE_CHANGE ) work->hit_flag = 1 ;
		src = grp->lit ;
		dst = ptr ;
		for ( j = grp->n_lights ; j > 0 ; src++, j-- ){
			if ( src->flag & LIT_FLAG_DISABLE ) continue ;
			if ( !( src->flag & LIT_FLAG_BGONLY ) ) continue ;
			/* さらに個々のあたり判定をチェックする */
#if 0
			if ( ( src->bound_max.vx ) < mdl_min.vx || ( src->bound_min.vx ) > mdl_max.vx ||
				( src->bound_max.vy ) < mdl_min.vy || ( src->bound_min.vy ) > mdl_max.vy ||
				( src->bound_max.vz ) < mdl_min.vz || ( src->bound_min.vz ) > mdl_max.vz ){
				continue ;
			}
#else
			if ( CheckBoundHit( &src->bound_max, &src->bound_min, &mdl_max, &mdl_min ) == 0 ) continue ;
#endif
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
#if 0
		if ( !( grp->bound_max.vx >= mdl_min.vx && grp->bound_min.vx <= mdl_max.vx &&
			   grp->bound_max.vy >= mdl_min.vy && grp->bound_min.vy <= mdl_max.vy &&
			   grp->bound_max.vz >= mdl_min.vz && grp->bound_min.vz <= mdl_max.vz ) ) continue ;
#else
		if ( CheckBoundHit( &grp->bound_max, &grp->bound_min, &mdl_max, &mdl_min ) == 0 ) continue ;
#endif
		if ( grp->type & LIT_TYPE_CHANGE ) work->hit_flag = 1 ;
		src = grp->lit ;
		dst = ptr ;
		for ( j = grp->n_lights ; j > 0 ; src++, j-- ){
			if ( src->flag & LIT_FLAG_DISABLE ) continue ;
			if ( !( src->flag & LIT_FLAG_BGONLY ) ) continue ;
			/* さらに個々のあたり判定をチェックする */
#if 0
			if ( ( src->bound_max.vx ) < mdl_min.vx || ( src->bound_min.vx ) > mdl_max.vx ||
				( src->bound_max.vy ) < mdl_min.vy || ( src->bound_min.vy ) > mdl_max.vy ||
				( src->bound_max.vz ) < mdl_min.vz || ( src->bound_min.vz ) > mdl_max.vz ){
				continue ;
			}
#else
			if ( CheckBoundHit( &src->bound_max, &src->bound_min, &mdl_max, &mdl_min ) == 0 ) continue ;
#endif
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
static	u_int	*NoShadeRGB( DG_MDL *mdl, u_int *rgbs )
{
	DG_MDLPACK	*pack ;
	int		i, j ;

	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		j = pack->n_verts;
		for (  ; j > 0 ; j-- ) 
      {
         *rgbs++ = 0x80808080;
		}
		pack++ ;
	}
	return rgbs ;
}

	/*
		プレシェイドバッファにＲＧＢ値を書き込む（シェイドあり）
	*/
static	u_int	*ShadeRGB( DG_MDL *mdl, u_int *rgbs, LIT_DEF *lit_def )
{
	DG_MDLPACK	*pack ;
	int		i ;

	/* 対象となる光源データをスクラッチパッドへ転送する */
	CreateLightBuffer( mdl, lit_def );

	/* ＶＵ０マイクロプログラム読み込み終了待ち */
	WaitLoadMicroProgram();

#if 0 //BP_PS2
	{
		int		total_verts, n_verts ;
		SVECTOR	*verts, *norms ;
		pack = mdl->packs ;
		total_verts = 0 ;
		for ( i = mdl->n_packs ; i > 0 ; i-- ){
			total_verts += pack->n_verts;
			pack++ ;
		}

		verts = (SVECTOR*)mdl->packs->verts ;
		norms = (SVECTOR*)mdl->packs->norms ;
		while ( total_verts ){
			n_verts = ( total_verts > 64 ) ? 64 : total_verts ;
			total_verts -= n_verts ;
			MakeLight( rgbs, verts, norms, n_verts );
			verts += n_verts ;
			norms += n_verts ;
			rgbs += n_verts ;
			
		}
	}
#else
   {
      pack = mdl->packs;
      for( i = 0; i < mdl->n_packs; ++i )
      {
         MakeLight(rgbs, (SVECTOR*)pack->verts, (SVECTOR*)pack->norms, pack->n_verts);
         rgbs += pack->n_verts;
         ++pack;
      }
   }
#endif
	return rgbs ;
}

	/*
		プレシェイドバッファの再計算
	*/
static	u_int	*ReshadeRGB( DG_MDL *mdl, u_int *rgbs, LIT_DEF *lit_def, int first_flag )
{
	DG_MDLPACK	*pack ;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR ;
	int		i ;

	/* 対象となる光源データをスクラッチパッドへ転送する */
	CreateLightBuffer( mdl, lit_def );

	if ( work->hit_flag == 0 ) return ( rgbs );

	/* ＶＵ０マイクロプログラム読み込み終了待ち */
	if ( first_flag ) WaitLoadMicroProgram();

	pack = mdl->packs ;
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		MakeLight( rgbs, (SVECTOR*)pack->verts, (SVECTOR*)pack->norms, pack->n_verts );
		rgbs += pack->n_verts;
		pack++ ;
	}
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
		for ( j = obj->n_packs ; j > 0 ; j-- ){
			size += pack->n_verts;
			pack++ ;
		}
		obj++ ;
	}
	return sizeof( u_int ) * size ;
}

/*----------------------------------------------------------------*/
	/*
		オブジェクトに対してプリシェード計算を行なう
	*/
int		DG_MakePreshade( DG_OBJS *objs, LIT_DEF *lit_def )
{
	FMATRIX		tmp_mat ;
	u_int		*rgbs ;
	DG_DEF		*def ;
	DG_OBJ		*obj ;
	int		i, flag = 0, time ;


	def = objs->def ;
	ASSERT( def != NULL ) ;
	/*
		プレシェイドバッファの確認
		なければ新たに確保する
	*/
	rgbs = objs->rgb_buff ;
	if( rgbs == NULL ){
		objs->rgb_buff = rgbs = (u_int *)GV_Malloc( SizeofRGB( objs ) ) ;
		if ( rgbs == NULL ) return -1 ;
		flag = 1 ;
      BP_AllocPreshadeBuffer(objs);
	}

	GV_SET_PRFC_CLOCK();
	/*
		ＶＵ０用マイクロプログラム読み込み
	*/
	LoadMicroProgram();
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
	time = GV_GET_PRFC_CLOCK();
//	printf("preshade time : %d\n", time);
	//if ( flag == 0 ) return 0 ;

	/* カラーバッファを法線情報の代わりにする */
#if 0 //BP_WEIRD
	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; -- i ) {
		obj->norms = (SVECTOR*)obj->rgbs ;
		obj ++ ;
	}
#endif

   BP_UpdatePreshadeBuffer(objs, objs->rgb_buff);

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

	GV_SET_PRFC_CLOCK();
	/*
		ＶＵ０用マイクロプログラム読み込み
	*/
	LoadMicroProgram();
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
	time = GV_GET_PRFC_CLOCK();
	//printf("represhade time : %d\n", time);

   if ( first_flag ) 
      WaitLoadMicroProgram();

   BP_UpdatePreshadeBuffer(objs, objs->rgb_buff);

	return 0 ;
}

void		DG_FreePreshade( DG_OBJS *objs )
{
	u_int		*rgbs ;

	if ( !( objs->flag & DG_FLAG_PAINT ) ) return ;

	rgbs = objs->rgb_buff;
	if( rgbs != NULL ){
		GV_DelayedFree( rgbs );
      BP_FreePreshadeBuffer(objs);
		objs->rgb_buff = NULL;
	}
}



/*----------------------------------------------------------------*/

	/*
		各頂点についての照明データを計算する

	*/
static void MakeLight2( u_int *rgbs, SVECTOR *verts, SVECTOR *norms, int n_verts, u_int *base )
{
	/*
		新バージョンプリシェード計算ルーチン
	*/
#if 0//BP_ASM - loop preload not needed
	FVECTOR		one = {1.0F,1.0F,1.0F,255.0F}, two = {1.5f,-0.75f,0.0f,0.0f} ;
#endif
	LIT_POINT	*lit ;
	LIT_SPOT	*spot ;
	//LIT_LINE	*line ;
	//u_long64	tmp ;
	int		i ;
	//int		count ;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR ;

#if 1//BP_ASM
   FVECTOR  vert_pos, vert_norm, color;

   // Reset color
   BP_Vec4_SetZero( &color );
#endif

#if 0//BP_ASM - loop preload not needed
	/* ＶＵ０レジスタ初期化 */
	asm ("
		lqc2	vf24,0(%0)			# vf24 = 平行光源ベクトル
		lqc2	vf25,16(%0)			# vf25 = 平行光源ベクトル
		lqc2	vf26,32(%0)			# vf26 = 平行光源ベクトル
		lqc2	vf27,64(%0)			# vf27 = 平行光源カラー
		lqc2	vf31,64+48(%0)		# vf10 = アンビエントカラー
		lqc2	vf3,0(%1)			# vf3 = 1.0, 1.0, 1.0, 255.0
		lqc2	vf2,0(%2)			# vf2 = 2.0, 0.0, 0.0, 0.0
	"::"r"(&DG_LightMatrix),"r"(&one),"r"(&two));
#endif

#if 0//BP_ASM - call to pshadevu2.dsm: StartSetup which stores converted verts in vu0 memory
	while ( -- n_verts >= 0 ) {
		/* 座標変換 */
		asm volatile ("
			ld		$10,0(%0)
			ld		$11,0(%1)
			pextlh	$10,$10,$0		# 各要素を３２ビットに符号拡張してＶＵ０へ渡す
			pextlh	$11,$11,$0		# 
			psraw	$10,$10,16		# 
			psraw	$11,$11,16		# 
			qmtc2	$10,vf8			# 
			qmtc2	$11,vf9			# 
			vcallms	0x00			# 
			vcallms	0x50			# ウェイト
			sqc2 vf8,0(%2)			# 
			ld		$10,0(%3)		# オリジナルのカラー読み込み
			pextlh	$10,$0,$10		# 
			qmtc2	$10,vf9			# 
			vitof0	vf10,vf9		#
		"::"r"(verts),"r"(norms),"r"(&work->w_vert),"r"(base):"$10","$11","memory");
		/* １頂点毎に累積輝度を算出する */
#endif

#if 1//BP_ASM - light vertices in place
   while ( -- n_verts >= 0 ) 
   {
      // Lookup vertex local position, normal
      BP_Vec4_ShortToFloat0( &vert_pos, (const SVECTOR*)&verts->vx );
      BP_Vec4_ShortToFloat12( &vert_norm, (const SVECTOR*)&norms->vx );

      // Transform into world space
      BP_Mat_TransformVec3( &vert_pos, &gMicroCodeData.matrix, &vert_pos );
      BP_Mat_RotateVec3( &vert_norm, &gMicroCodeData.matrix, &vert_norm );

      // Decode original color
      BP_Vec4_FromColor( &color, *base++ );
#endif
		/* 点光源 */
		lit = work->points ;
		//count = 0 ;
		for ( i = work->n_point ; i > 0 ; -- i, lit++ ) {
#if 0//BP_ASM
			float	f, r ;
			//r = lit->r_range * 2.0f ;
			r = lit->e_range ;
			work->w_vec.vx = f = work->w_vert.vx - lit->point.vx ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vy = f = work->w_vert.vy - lit->point.vy ;
			if ( FABS( f ) > r ) continue ;
			work->w_vec.vz = f = work->w_vert.vz - lit->point.vz ;
			if ( FABS( f ) > r ) continue ;
			work->w_param.vy = lit->r_range * 2 ;
			asm ("
				lw			$4,0(%2)
				pextlb		$4,$0,$4
				pextlh		$4,$0,$4
				qmtc2.i		$4,vf18
				lqc2		vf16,0(%0)
				lqc2		vf17,0(%1)
				vcallms		0x10
				#vcallms		0x40
			"::"r"(&work->w_vec),"r"(&work->w_param),"r"(&lit->color):"$4" );
			//if ( ++count >= 3 ) break ;
#else
      BP_PointLightCalc( &vert_pos, &vert_norm, lit, &color );
#endif
		}

		/* スポット光源 */
		spot = work->spot ;
		for ( i = work->n_spot ; i > 0 ; -- i, spot++ ) {
#if 0//BP_ASM
			float	r ;
			r = spot->dir.vw * 2.0f ;
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > spot->bound_max.vx || work->w_vert.vx < spot->bound_min.vx ) continue ;
			if ( work->w_vert.vy > spot->bound_max.vy || work->w_vert.vy < spot->bound_min.vy ) continue ;
			if ( work->w_vert.vz > spot->bound_max.vz || work->w_vert.vz < spot->bound_min.vz ) continue ;
			work->w_vec.vx = work->w_vert.vx - spot->point.vx ;
			work->w_vec.vy = work->w_vert.vy - spot->point.vy ;
			work->w_vec.vz = work->w_vert.vz - spot->point.vz ;
			work->w_param.vy = r ;
			work->w_param.vz = spot->umbra ;
			work->w_param.vw = spot->penumbra ;
			asm ("
				lw			$4,0(%2)
				pextlb		$4,$0,$4
				pextlh		$4,$0,$4
				qmtc2.i		$4,vf18
				lqc2		vf16,0(%0)
				lqc2		vf17,0(%1)
				lqc2		vf19,0(%3)
				vcallms		0x20
			"::"r"(&work->w_vec),"r"(&work->w_param),"r"(&spot->color),"r"(&spot->dir):"$4" );
#else
      BP_SpotLightCalc( &vert_pos, &vert_norm, spot, &color );
#endif
		}

#if 1//BP_ASM - light vertices in place
      // Store results
      *rgbs++ = BP_Vec4_ToColor( &color );
      verts++;
      norms++;
   }
#endif

#if 0 //BP_ASM - This code is disabled since there are no dynamic line lights ever added
      // See the DG_SetTmp??? functions in light.c - only point lights and spot lights are supported
		/* 線光源 */
		line = work->line ;
		for ( i = work->n_line ; i > 0 ; -- i, line++ ) 
      {
			float	r ;
			r = line->dir.vw * 2.0f ;
			/* バウンディングボックス対応バージョン */
			if ( work->w_vert.vx > line->bound_max.vx || work->w_vert.vx < line->bound_min.vx ) continue ;
			if ( work->w_vert.vy > line->bound_max.vy || work->w_vert.vy < line->bound_min.vy ) continue ;
			if ( work->w_vert.vz > line->bound_max.vz || work->w_vert.vz < line->bound_min.vz ) continue ;
			work->w_param.vy = line->r_range * 2 ;
			asm ("
				lw			$4,0(%2)
				pextlb		$4,$0,$4
				pextlh		$4,$0,$4
				qmtc2.i		$4,vf18
				lqc2		vf16,0(%0)
				lqc2		vf17,0(%1)
				lqc2		vf19,0(%3)
				vcallms		0x10
			"::"r"(&work->w_vec),"r"(&work->w_param),"r"(&line->color),"r"(&line->dir):"$4" );
		}
#else
   ASSERT( work->n_line == 0 );
#endif

#if 0//BP_ASM - results are computed in place above
		/* 累積結果を取り出す */
		asm ("
			vnop			# 終了ウェイト
			vminiw.xyz		vf10,vf10,vf3
			vftoi0.xyzw		vf10,vf10
			qmfc2			$10,vf10
			ppach			$10,$0,$10
			move			%0,$10
		":"=r"(tmp)::"$10");
		tmp |= ( *(u_long64*)norms ) & 0xffff000000000000ULL ;
		*(u_long64*)rgbs = tmp ;
		verts++ ;
		norms++ ;
		rgbs++ ;
		base++ ;
	}
	if ( (int)rgbs & 0x8 ) rgbs->pad = norms->pad ;
#endif

}

//BP_LIGHTING - tracking information for temp (dynamic) lighting

#define BP_MAX_TEMP_PRESHADE_OBJECTS   (1024*50)
#define BP_MAX_PRESHADE_COLORS         (1024*100)

// Object color info
typedef struct 
{
   u_int*   colors;     // Source colors
   int      n_colors;   // Number of colors
} BP_TempPreshadeObjInfo;

// Object dynamic lighting info
static int                    g_BP_TempPreshadeObjLitCount = 0;                        // # of objects re-lit
static int                    g_BP_TempPreshadeObjInfoCount = 0;                       // # of objects processed
static BP_TempPreshadeObjInfo g_BP_TempPreshadeObjInfo[BP_MAX_TEMP_PRESHADE_OBJECTS];  // Info of objects processed
static u_int                  g_BP_TempPreshadeColors[BP_MAX_PRESHADE_COLORS];         // Dynamic colors

//BP_LIGHTING - tracking information for temp (dynamic) lighting

	/*
		プレシェイドバッファにＲＧＢ値を書き込む（シェイドあり）
	*/
static void _DG_TempPreshadeRGB( DG_OBJ *obj, LIT_DEF *lit_def, int which, int start_flag, int hackAlwaysLight )
{
	DG_MDL		*mdl ;
	DG_MDLPACK	*mdl_pack ;
	int		i, packet_size ;
	u_int		*tmp_color, *org_rgbs ;
	ScrpadWork *work = (ScrpadWork *)SCRPAD_ADDR ;
   
   //BP_LIGHTING - add next object info entry
   BP_TempPreshadeObjInfo* pInfo;
   ASSERT( g_BP_TempPreshadeObjInfoCount < BP_MAX_TEMP_PRESHADE_OBJECTS );
   pInfo = &g_BP_TempPreshadeObjInfo[g_BP_TempPreshadeObjInfoCount++];
   //BP_LIGHTING - add next object info entry

	mdl = obj->model ;
	/*
		ＶＵ０用マイクロプログラム読み込み
	*/
	if ( start_flag ) LoadMicroProgram();

	/* 対象となる光源データをスクラッチパッドへ転送する */
	_SetMatrix( &obj->world );
	CreateLightBuffer( mdl, lit_def );

   //BP_LIGHTING - moved here since we need this info for lit or not lit objects
   /* パケットサイズ計算 */
   packet_size = 0 ;
   mdl_pack = mdl->packs ;
   for ( i = mdl->n_packs ; i > 0 ; mdl_pack++, i-- )
   {
      packet_size += mdl_pack->n_verts;
   }
   //BP_LIGHTING - moved here since we need this info for lit or not lit objects

	if ( ( work->n_point + work->n_spot + work->n_black ) == 0 )
   {
      //BP_LIGHTING - set info to reference original colors
      g_BP_TempPreshadeObjLitCount += hackAlwaysLight;

      pInfo->colors = obj->rgbs;
      pInfo->n_colors = packet_size;
      //BP_LIGHTING - set info to reference original colors
		return ;
	}

	/* メモリ確保（ＤＭＡパケット領域から確保） */
	tmp_color = (u_int*)DG_CurrentDmaAddr ;
	DG_CurrentDmaAddr = (void*)( (int)DG_CurrentDmaAddr + sizeof(u_int) * ((packet_size + 3) & (~3)) );

   //BP_LIGHTING - update lit count and set info to reference new colors
   g_BP_TempPreshadeObjLitCount++;
   pInfo->colors = tmp_color;
   pInfo->n_colors = packet_size;
   //BP_LIGHTING - update lit count and set info to reference new colors

	/* ＶＵ０マイクロプログラム読み込み終了待ち */
	if ( start_flag ) WaitLoadMicroProgram();

	obj->vanime_flag &= ~DG_VANIME_DELNORMS ;	/* 法線アドレス復帰フラグを消しておく */
	obj->vanime_flag |= DG_VANIME_NORMS ;	/* 一時法線（＝頂点カラー）アニメのフラグを立てる */

#if 0 //BP_WEIRD
	obj->norms = (SVECTOR*)tmp_color ;
#endif
	mdl_pack = mdl->packs ;
	org_rgbs = obj->rgbs ;
	for ( i = obj->n_packs ; i > 0 ; i-- ){
		MakeLight2( tmp_color, (SVECTOR*)mdl_pack->verts, (SVECTOR*)mdl_pack->norms, mdl_pack->n_verts, org_rgbs );
		tmp_color += mdl_pack->n_verts;
		org_rgbs += mdl_pack->n_verts;
		//count += mdl_pack->n_verts ;
		mdl_pack++ ;
	}
	FlushCache( 0 );
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
   int         hackAlwaysLight = 0;

   // AS(JM) Control variable for knowing if we did lighting
   // the prior frame.
   static int  sHackLitPriorFrame = 0;

	//int			time ;

   //BP_LIGHTING - disabled?
   if(!gBP_EnableDynamicLighting)
   {
      return;
   }

   //BP_LIGHTING - disabled?

   if ( 
      !strcmp( GM_GetArea(), "sp24a" ) || 
      !strcmp( GM_GetArea(), "w01d" ) /* MGSTWO-2948 */
      )
   {
      // AS(JM) - Note that the proper fix here is to detect when preshade
      // was being used in the prior frame, and always light one extra frame.
      // But this is a lot less risky and simpler to make it just check for
      // the area this seems to happen in.

      hackAlwaysLight = 1;
   }

	flp = &DG_TLights[ 1 - DG_LightClock ] ;

	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->objs_buffer ;

   if ( ( flp->TmpLightGrp[0].n_lights + flp->TmpLightGrp[1].n_lights + flp->TmpLightGrp[2].n_lights ) == 0 ) 
   {
      // AS(JM) - If we're in "hackAlwaysLight" mode and all lights are off, but they WERE on, then 
      // keep on trucking.

      if ( hackAlwaysLight )
      {
         if ( !sHackLitPriorFrame )
         {
            return;
         }
         else
         {
            sHackLitPriorFrame = 0;
         }
      }
      else
      {
         sHackLitPriorFrame = 0;
         return;
      }
   }
   else
   {
      sHackLitPriorFrame = 1;
   }


	//printf("tmp preshade start\n");
//GV_SET_PRFC_CLOCK();
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; i-- )
   {
		objs = *oque++ ;

		if ( !( objs->flag & DG_FLAG_PAINT ) ) 
         continue ;

      //BP_LIGHTING - init tracking of preshaded objects
      g_BP_TempPreshadeObjLitCount = 0;
      g_BP_TempPreshadeObjInfoCount = 0;
      //BP_LIGHTING - init tracking of preshaded objects

      // Light objects
      for ( j = 0 ; j < objs->n_models ; j++ )
      {
			if ( objs->objs[j].rgbs == NULL ) 
            continue ;
		
         _DG_TempPreshadeRGB( &objs->objs[j], (LIT_DEF*)&flp->TmpLightDef, which, start_flag, hackAlwaysLight );
			start_flag = 0 ;
		}

      //BP_LIGHTING - update BP preshade buffer?
      if( g_BP_TempPreshadeObjLitCount )
      {
         // Collect all object colors into a linear array
         u_int*                  pDst  = g_BP_TempPreshadeColors;
         BP_TempPreshadeObjInfo* pInfo = g_BP_TempPreshadeObjInfo;

         // Loop over all objects
         for( j = 0; j < g_BP_TempPreshadeObjInfoCount; j++ )
         {
            u_int *newDst = pDst + pInfo->n_colors;
            // Append object colors into linear buffer
            ASSERT( newDst < &g_BP_TempPreshadeColors[BP_MAX_PRESHADE_COLORS] );
            memcpy( pDst, pInfo->colors, pInfo->n_colors * sizeof(u_int) );

            // Next object
            pDst = newDst;
            pInfo++;
         }

         // Finally update BP vertex buffer
         BP_UpdatePreshadeBuffer( objs, g_BP_TempPreshadeColors );
      }
      //BP_LIGHTING - update BP preshade buffer?
	}
//time = GV_GET_PRFC_CLOCK();
//printf("tmp preshade: %d\n", time );
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
#if 0 //BP_WEIRD
			if ( obj->vanime_flag & DG_VANIME_DELNORMS ) obj->norms = (SVECTOR*)obj->rgbs ;
#endif
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
