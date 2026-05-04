//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
#include "BP_Renderer.h"
//BP
//----------------------------------------------------------------------------
/*
	screen.c
	チャンネル処理ユニット／モデルスクリーン座標計算ルーチン

	1999/07/07 K.Takabe
	$Id: screen.c,v 1.1.1.3 2002/11/19 11:42:23 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／モデルスクリーン座標計算ルーチン
	＆バウンディングチェック

	void		DG_ScreenChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各物体のワールド座標と関節回転データをもとに
	各ユニットのワールド座標／スクリーン座標を計算する
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

#include "bp_matrix.h"
#include "BP_Asm.h"

#include "BP_RenderBufferTypes.h"

#if BP_VITA
#  include <sce_atomic.h>
#endif

#ifdef DEBUG_MODE
int		DG_OnePieceSkip = 0 ;
#endif

//#define STAGE_CLIP_DEBUG

static inline FMATRIX *_as_get_obj_parent_world_matrix(DG_OBJS *objs, DG_OBJ *obj)
{
   return obj->parent == -1 ? &objs->world : &objs->objs[obj->parent].world;
}

#if 1 //BP_PS2

static inline void MulMatrix( FMATRIX *res, FMATRIX *m0, FMATRIX *m1 )
{
   BP_Mat_MulMatrix( res, m0, m1 );
}

static inline void ApplyMatrix2( FVECTOR *res, FMATRIX *m0, FVECTOR *v0 )
{
   BP_Mat_TransformVec3( res, m0, v0 );
}
#endif

#define _CopyVector( a, b )   BP_Vec4_Copy( a, b )

#if 1//BP_ASM

static inline void _CopyMatrix( FMATRIX *m0, FMATRIX *m1 )
{
   BP_Mat_Copy( m0, m1 );
}

#else

#if 1
#define _CopyMatrix( a, b ) { asm ("lq $8,0(%1);lq $9,16(%1);lq $10,32(%1);lq $11,48(%1);\
									sq $8,0(%0);sq $9,16(%0);sq $10,32(%0);sq $11,48(%0);":\
								   :"r"(a),"r"(b):"$8","$9","$10","$11","memory"); }
#else
#define _CopyMatrix( a, b ) { 0[(u_long128*)a] = 0[(u_long128*)b] ;\
								1[(u_long128*)a] = 1[(u_long128*)b] ;\
								2[(u_long128*)a] = 2[(u_long128*)b] ;\
								3[(u_long128*)a] = 3[(u_long128*)b] ; }
#endif

#endif //BP_PS2

#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)


	/*
		スクラッチパッド割り当て
	*/
typedef	struct screen_ScrPad	{
	FMATRIX		eye_pers ;
//	FMATRIX		eye_pers2 ;
	FMATRIX		eye_inv ;
//	FMATRIX		eye ;

//	FVECTOR		bound[8] ;
//	FVECTOR		scale ;
//	FVECTOR		tmp_vec ;
//	float		screen ;		/* Ｚ値算出用補正定数 */
//	int			group_bound ;
	int			invisible_flag ;
	float		fog_param1, fog_param2 ;
//	int			store_clock ;
//	int			pad[1] ;
//	char		bound_cache[ DG_MAX_JOINTS ];

//	FMATRIX		j_root ;
//	FMATRIX		joints[ DG_MAX_JOINTS ] ;
//	FMATRIX		j_root2 ;
//	FMATRIX		tmp[4];

	u_long128	local_work[0] ;

} ScrPad ;

typedef struct screen_ObjsState
{
   int			sort_z ;		/* Ｗより求めたＺ値 */
} SObjsState;


//#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)
#define	EYE_PERS	(&(scrpad->eye_pers))
//#define	EYE_PERS2	(&(scrpad->eye_pers2))
//#define	J_ROOT		(&(scrpad->j_root))
//#define	J_ROOT2		(&(scrpad->j_root2))
//#define	JOINTS		(scrpad->joints)
//#define BOUNDS		(scrpad->bound)
//#define SCALE		(&scrpad->scale)
//#define SCREEN		(scrpad->screen)
//#define SORT_Z		(scrpad->sort_z)
//#define GBOUND		(scrpad->group_bound)
//#define	WORK_MAT	(&scrpad->tmp[0])
//#define	WORK_MAT2	(&scrpad->tmp[1])

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
//static	void	OnePieceObjs( DG_OBJS *, int, FMATRIX const *eye_pers, FMATRIX const *j_root ) ;
//static	void	ScreenObjs( DG_OBJS *, int ) ;
//static	void	SlideFrameObjs( DG_OBJS *, int ) ;
//static	void	JointFrameObjs( DG_OBJS *, int ) ;
#endif

extern qword BoundingCheckVu0 ;
extern qword BoundingCheckVu0_Func ;


/* ---------------------------------------------------------------- */
	/*
		バウンディングチェック計算マイクロプログラム読み込み
	*/
void DG_StartBoundingCheckSupport( void )
{
   //BP_TODO - bring over bp_boundvu.c from MGS3
   BP_ASM_TODO_BOUND_BREAK;
#if 0 
	DG_DmaStartCheck( 0 );
	DPUT_D0_MADR( (int)((DG_DMATAG*)BoundingCheckVu0)->addr ) ;
	DPUT_D0_QWC( ( ((DG_DMATAG*)BoundingCheckVu0)->qwc ) & 0x7fff ) ;
	//DPUT_D0_CHCR( 0x0141 ) ; /* STR:1 TIE:0 TTE:1 ASP:0 MOD:0 DIR:1 */
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D0_CHCR),"r"(0x141) );
	DG_WaitDma( 0 );
#endif
}

/* ---------------------------------------------------------------- */
#if 1 //BP_ASM

static inline void Vu0CopyMatrix( FMATRIX *dst, FMATRIX *src )
{
   BP_Mat_Copy( dst, src );
}

#else

#if 0
static inline void Vu0CopyMatrix( FMATRIX *dst, FMATRIX *src )
{
	asm ("
		lqc2		vf04,0x00(%1)
		lqc2		vf05,0x10(%1)
		lqc2		vf06,0x20(%1)
		lqc2		vf07,0x30(%1)
		sqc2		vf04,0x00(%0)
		sqc2		vf05,0x10(%0)
		sqc2		vf06,0x20(%0)
		sqc2		vf07,0x30(%0)
	"::"r"(dst),"r"(src));
}
#else
/* Ｖｕ０レジスタ使用スクラッチパッド間のみ使用可能バージョン */
#define Vu0CopyMatrix( _a, _b ) \
	asm ("\
		lqc2		vf04,0x00(%1);\
		lqc2		vf05,0x10(%1);\
		lqc2		vf06,0x20(%1);\
		lqc2		vf07,0x30(%1);\
		sqc2		vf04,0x00(%0);\
		sqc2		vf05,0x10(%0);\
		sqc2		vf06,0x20(%0);\
		sqc2		vf07,0x30(%0);\
	"::"r"(_a),"r"(_b));

#endif

#endif //BP_ASM

/*----------------------------------------------------------------*/
static inline void SetLightMatrix( FMATRIX *res_light, FMATRIX *light, FMATRIX *world )
{
#if 1 //BP_ASM
   FMATRIX		tmp_mat ;
   BP_Mat_Copy( &tmp_mat, world );
   tmp_mat.m[3][0] = 0 ;
   tmp_mat.m[3][1] = 0 ;
   tmp_mat.m[3][2] = 0 ;
   _sceVu0MulMatrix( res_light, light, &tmp_mat );
#else
	/* 書き込み先はスクラッチパッドでなければならない */
	asm volatile ("
		lqc2		vf4,0x00(%2)
		lqc2		vf5,0x10(%2)
		lqc2		vf6,0x20(%2)
		lqc2		vf7,0x30(%2)
		li			$7,4
0:
		lqc2		vf8,0x0(%1)
		vmulax.xyz		ACC,vf4,vf8
		vmadday.xyz		ACC,vf5,vf8
		vmaddaz.xyz		ACC,vf6,vf8
		vmaddw.xyz		vf12,vf7,vf8
		sqc2		vf12,0x0(%0)
		addi		$7,-1
		addi		%1,0x10
		addi		%0,0x10
		bne			$0,$7,0b
	":"+r"(res_light),"+r"(world):"r"(light):"$7");
#endif
}

/* ---------------------------------------------------------------- */
#if 0
static inline void _SetRotTransPersMatrix( FMATRIX *m )
{
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	": : "r"(m) );
}

static inline u_int _RotTransPers( FVECTOR *v )
{
	u_int	ret ;
	asm ("
	lqc2			vf8,0x00(%1)
	lqc2			vf9,0x10(%1)
	lqc2			vf10,0x20(%1)
	lqc2			vf11,0x30(%1)
	lqc2			vf1,0(%2)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf8
	vmulax.xyzw		ACC, vf4,vf9
	vmadday.xyzw	ACC, vf5,vf9
	vmaddaz.xyzw	ACC, vf6,vf9
	vmaddw.xyzw		vf9, vf7,vf9
	vmulax.xyzw		ACC, vf4,vf10
	vmadday.xyzw	ACC, vf5,vf10
	vmaddaz.xyzw	ACC, vf6,vf10
	vmaddw.xyzw		vf10, vf7,vf10
	vmulax.xyzw		ACC, vf4,vf11
	vmadday.xyzw	ACC, vf5,vf11
	vmaddaz.xyzw	ACC, vf6,vf11
	vmaddw.xyzw		vf11, vf7,vf11
	vclipw.xyz		vf8xyz,vf8w
	vclipw.xyz		vf9xyz,vf9w
	vclipw.xyz		vf10xyz,vf10w
	vclipw.xyz		vf11xyz,vf11w
	vdiv			Q,vf0w,vf8w
	vwaitq
	vmulq.xy		vf8,vf8,Q
	vdiv			Q,vf0w,vf9w
	vmul.xy			vf8,vf8,vf1
	vwaitq
	vmulq.xy		vf9,vf9,Q
	vdiv			Q,vf0w,vf10w
	vmul.xy			vf9,vf9,vf1
	vwaitq
	vmulq.xy		vf10,vf10,Q
	vdiv			Q,vf0w,vf11w
	vmul.xy			vf10,vf10,vf1
	vwaitq
	vmulq.xy		vf11,vf11,Q
	cfc2			%0,$18 /*clipping*/
	sqc2			vf8,0x00(%1)
	vmul.xy			vf11,vf11,vf1
	sqc2			vf9,0x10(%1)
	sqc2			vf10,0x20(%1)
	sqc2			vf11,0x30(%1)
	": "&=r"(ret) : "r" (v),"r"(SCALE):"memory" );
	return ( ret );
}

/*----------------------------------------------------------------*/

	/*
		透視変換後の同時座標とクリッピングフラグを計算（４頂点同時に）
	*/
static u_long64 MakeBoundVerts( FMATRIX *eye_pers, float *org, FVECTOR *verts )
{
	int		lx,ly,lz,ux,uy,uz ;
	int		*i_org ;
	int		i, j ;
	IVECTOR	*iverts ;
	u_long64	flag = 0 ;

	i_org = (int*)org ;
	iverts = (IVECTOR*)verts ;
	lx = i_org[0] ;
	ly = i_org[1] ;
	lz = i_org[2] ;
	ux = i_org[3] ;
	uy = i_org[4] ;
	uz = i_org[5] ;
	_SetRotTransPersMatrix( eye_pers );
	for ( i = 8 ; i > 0 ; ){
		for ( j = 4 ; j > 0 ; j--, i-- ){
			iverts->vx = ( i & 1 ) ? lx : ux ;
			iverts->vy = ( i & 2 ) ? ly : uy ;
			iverts->vz = ( i & 4 ) ? lz : uz ;
			verts->vw = 1.0F ;
			iverts++ ;
			verts++ ;
		}
		flag = ( flag << 24 ) | ( (u_long64)_RotTransPers( verts-4 ) & 0x00ffffff ) ;
	}
	return ( flag );
}

	/*
		バウンディングボックスと透視変換マトリクスからバウンディング結果を求める
	*/
static int BoundCheck( FMATRIX *eye_pers, float *bound )
{
	FVECTOR		*verts ;
	u_long64		flag, and_flag, or_flag ;
	int			i, zz, bound_flag = 0 ;
	float		z ;

	verts = BOUNDS ;
	flag = MakeBoundVerts( eye_pers, bound, verts );

	z = ( ( verts[0].vw + verts[1].vw ) * 0.5f ) ;

	/* フラグの累積チェック */
	or_flag = and_flag = flag ;
	for ( i = 7 ; i > 0 ; i-- ){
		flag >>= 6 ;
		and_flag &= flag ;
		or_flag |= flag ;
	}

	SORT_Z = DG_FTOI(z) ;

	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */

	/* Ｚ距離のチェック */
	bound_flag = 0 ;
	for ( i = 8 ; i > 0 ; i-- ){
		/* プリミティブ座標オーバーフラグ */
		if ( verts->vx < -2048.0F || verts->vx > 2047.0F ) bound_flag = 1 ;
		if ( verts->vy < -2048.0F || verts->vy > 2047.0F ) bound_flag = 1 ;
		verts++ ;
	}

	if ( bound_flag ) return ( 1 );	/* クリップ必要 */

	return ( 0 );	/* 完全画面内 */
}
#else
#if 1 //BP - changed this function to match the XBox version.
static int BoundCheck( FMATRIX *mat, FVECTOR *bound, int mode, int *pOutSortZ )
{
	int		and_flag, or_flag, flag ;
   //int prim_over_flag = 0;
	FVECTOR		verts, tmp_v, bound_verts[8] ;
	float		w, total_w ;
	int			i ;

	and_flag = 0xffffffff ;
	or_flag = 0 ;
	total_w = 0 ;

	/* バウンディング用頂点の生成 */
	bound_verts[0].vx = bound_verts[2].vx = bound_verts[4].vx = bound_verts[6].vx = bound[0].vx ;
	bound_verts[1].vx = bound_verts[3].vx = bound_verts[5].vx = bound_verts[7].vx = bound[1].vx ;
	bound_verts[0].vy = bound_verts[1].vy = bound_verts[4].vy = bound_verts[5].vy = bound[0].vy ;
	bound_verts[2].vy = bound_verts[3].vy = bound_verts[6].vy = bound_verts[7].vy = bound[1].vy ;
	bound_verts[0].vz = bound_verts[1].vz = bound_verts[2].vz = bound_verts[3].vz = bound[0].vz ;
	bound_verts[4].vz = bound_verts[5].vz = bound_verts[6].vz = bound_verts[7].vz = bound[1].vz ;

	for ( i = 0 ; i < 8 ; i++ )
   {
      BP_Mat_TransformVec3(&tmp_v, mat, &bound_verts[i]);	/* wは1.0と見なす */
		total_w += tmp_v.vw ;					/* 射影マトリクスを掛けているのでvwにはカメラからの距離 */
		w = DG_FABS( tmp_v.vw );

		/* クリップチェック */
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
		if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
		//if ( tmp_v.vz < 0.0f ) flag |= CLIP_Z1_FLAG;	/* XBOXはPS2とは違うので注意 */
		and_flag &= flag ;
		or_flag |= flag ;
	}
	/* バウンディング中央値のＺ座標取得 */
	*pOutSortZ = total_w * ( 1.0f / 8.0f ) ;

#if 0
	/* バウンディング判定 */
	if ( mode == 0 ){
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
//		if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	} else {
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
	}
#else
	/* ＸＢＯＸではクリップによるコストは無視しても大丈夫そうなので */
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
#endif

	return ( 0 );	/* 完全画面内 */

}

#else
static int BoundCheck( FMATRIX *mat, FVECTOR *bound, int flag )
{
	int		and_flag, or_flag, prim_over_flag ;

	/* バウンディングチェックマイクロプログラム呼び出し */
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf1,0x00(%1)
	lqc2		vf2,0x10(%1)
	lqc2		vf3,0(%2)
	vcallms		0x00			# バウンディングチェック
	"::"r"(mat),"r"(bound),"r"(SCALE) );
	/* 計算結果の取得 */
	asm volatile ("
	cfc2.i		%0,$2 /*vi2*/
	cfc2		%1,$3 /*vi3*/
	cfc2		%2,$1 /*vi1*/
	sqc2		vf31,0(%3)
	":"=&r"(and_flag),"=&r"(or_flag),"=&r"(prim_over_flag):"r"(BOUNDS) );

	/* バウンディング中央値のＺ座標取得 */
	SORT_Z = DG_FTOI(BOUNDS[0].vw) ;

	/* バウンディング判定 */
	if ( flag == 0 ){
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
		if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	} else {
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
	}
	return ( 0 );	/* 完全画面内 */

}
#endif

static int CheckSortZ( FMATRIX *mat, FVECTOR *bound, int *pSortZOut )
{
#if 1 //BP_ASM
   FVECTOR		verts, tmp_v ;

   verts.vx = ( bound[0].vx + bound[1].vx ) * 0.5f ;
   verts.vy = ( bound[0].vy + bound[1].vy ) * 0.5f ;
   verts.vz = ( bound[0].vz + bound[1].vz ) * 0.5f ;
   verts.vw = 1.0f;
   BP_Mat_TransformVec3( &tmp_v, mat, &verts );

   *pSortZOut = (int)tmp_v.vw ;
#else
	/* ソート用Ｚ値計算マイクロプログラム呼び出し */
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf1,0x00(%1)
	lqc2		vf2,0x10(%1)
	vcallms		0x10			# ソート用Ｚ値チェック
	"::"r"(mat),"r"(bound) );
	/* 計算結果の取得 */
	asm ("
	qmfc2.i		$12,vf31
	sq			$12,0(%0)
	"::"r"(BOUNDS): "$12" );

	/* バウンディング中央値のＺ座標取得 */
	SORT_Z = DG_FTOI(BOUNDS[0].vw) ;
#endif
	return ( 0 );	/* 完全画面内 */
}
#endif

/*----------------------------------------------------------------*/
	/*
		エンベロープ用補正マトリクスの生成
	*/
#if 0
static void GetEnvelopeMatrix( FMATRIX *offset_mat, FMATRIX *base, FMATRIX *target, FVECTOR *trans )
{
	//(SCRPAD)->tmp_vec.vx = trans[0] ;
	//(SCRPAD)->tmp_vec.vy = trans[1] ;
	//(SCRPAD)->tmp_vec.vz = trans[2] ;
	//(SCRPAD)->tmp_vec.vw = 1.0f ;
#if 0
	_CopyMatrix( WORK_MAT2, base );
	_CopyMatrix( WORK_MAT, target );
	// *WORK_MAT2 = *base ;
	// *WORK_MAT = *target ;
	WORK_MAT->m[3][0] = 0.0F ; WORK_MAT->m[3][1] = 0.0F ; WORK_MAT->m[3][2] = 0.0F ; 
	WORK_MAT2->m[3][0] = 0.0F ; WORK_MAT2->m[3][1] = 0.0F ; WORK_MAT2->m[3][2] = 0.0F ; 
	sceVu0InversMatrix( WORK_MAT2, WORK_MAT2 );
	sceVu0MulMatrix( offset_mat, WORK_MAT2, WORK_MAT );
	sceVu0ApplyMatrix( &(SCRPAD)->tmp_vec, target, &(SCRPAD)->tmp_vec );
	sceVu0SubVector( offset_mat->m[3], &(SCRPAD)->tmp_vec, base->m[3] );
	sceVu0ApplyMatrix( offset_mat->m[3], WORK_MAT2, offset_mat->m[3] );
	/* (base)*(offset_mat)=(target) */
#else
	/* エンベロープ用マトリクス計算マイクロプログラム呼び出し */
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf8,0x00(%1)
	lqc2		vf9,0x10(%1)
	lqc2		vf10,0x20(%1)
	lqc2		vf11,0x30(%1)
	lqc2		vf3,0(%2)
	vcallms		0x20			# エンベロープマトリクス計算
	"::"r"(base),"r"(target),"r"(trans) );
	GV_PREFECH( offset_mat );
	/* 計算結果の取得 */
	asm ("
	qmfc2.i		$12,vf16
#	sq			$12,0x00(%0)
	sqc2		vf16,0x00(%0)
	sqc2		vf17,0x10(%0)
	sqc2		vf18,0x20(%0)
	sqc2		vf19,0x30(%0)
	"::"r"(offset_mat):"$12" );
#endif
}
#endif
/*----------------------------------------------------------------*/
	/*
		マトリクス生成＆エンベロープ用補正マトリクスの生成
	*/
#if 0//BP_ASM
static inline void MakeMatrixStart( FMATRIX *base, FMATRIX *target, FVECTOR *trans )
{
   /* エンベロープ用マトリクス計算マイクロプログラム呼び出し */
   asm ("
      lqc2		vf4,0x00(%0)		# load base0
      lqc2		vf20,0x00(%3)		# laod EYE_PERS0
      lqc2		vf21,0x10(%3)		# laod EYE_PERS1
      lqc2		vf22,0x20(%3)		# laod EYE_PERS2
      lqc2		vf23,0x30(%3)		# laod EYE_PERS3
      lqc2		vf5,0x10(%0)		# load base1
      lqc2		vf6,0x20(%0)		# load base2
      lqc2		vf7,0x30(%0)		# load base3
      lqc2		vf8,0x00(%1)		# load target0 (parent0)
      lqc2		vf9,0x10(%1)		# load target1 (parent1)   
      lqc2		vf10,0x20(%1)		# load target2 (parent2)
      lqc2		vf11,0x30(%1)		# load target3 (parent0)
      lqc2		vf3,0(%2)			# load trans
      vcallms		0x30			# エンベロープマトリクス計算
      "::"r"(base),"r"(target),"r"(trans),"r"(EYE_PERS) );
}
static inline void MakeMatrixEnd( FMATRIX *screen, FMATRIX *offset_mat )
{
   /* 計算結果の取得 */
   asm ("
      qmfc2.i		$12,vf16
      sqc2		vf16,0x00(%0)
      sqc2		vf17,0x10(%0)
      sqc2		vf18,0x20(%0)
      sqc2		vf19,0x30(%0)
      sqc2		vf20,0x00(%1)
      sqc2		vf21,0x10(%1)
      sqc2		vf22,0x20(%1)
      sqc2		vf23,0x30(%1)
      "::"r"(offset_mat),"r"(screen):"$12" );
}

#else

void BP_MakeMatrix( FMATRIX const *world, FMATRIX const *parent, FVECTOR const *trans, FMATRIX const *eye_pers, FMATRIX *screen, FMATRIX *offset_mat )
{
   FMATRIX inv_world, adj_parent;

   // bound_vu.dsm: 
   //    MakeMatrix
   
   // MulMatrix( screen, eye_pers, world );
   BP_Mat_MulMatrix( screen, eye_pers, world );

   
   // bound_vu.dsm:
   //    GetEnvelopeMatrix
   
   // ApplyMatrix( parent, trans );
   BP_Mat_Copy( &adj_parent, parent );
   BP_Mat_TransformVec3( (FVECTOR*)&adj_parent.m[3][0], parent, trans );

   // InversMatrix( base )
   BP_Mat_FastInverse( &inv_world, world );

   // MulMatrix( inv_base, parent );
   BP_Mat_MulMatrix( offset_mat, &inv_world, &adj_parent );
}

#endif

/*----------------------------------------------------------------*/

	/*
		関節なし一体型モデルについて処理

		各ユニットのワールドマトリクス／スクリーンマトリクスは
		全て物体そのものと同じになる
	*/
static	void	OnePieceObjs( DG_OBJS *objs, int n_objs, SObjsState *pState, FMATRIX const *eye_pers, FMATRIX const *j_root )
{
	DG_OBJ		*obj ;
	int		i, n = 0, last_objs, flag, type ;
   FMATRIX new_screen;
//	ScrPad		*work = SCRPAD_ADDR ;

	/* トランスレーションとバウンディング情報をスクラッチパッドへコピーする */
	last_objs = n_objs ;

	GV_PREFECH( &objs->objs->flag );
	_sceVu0MulMatrix( &new_screen, eye_pers, j_root ) ;
//	_sceVu0MulMatrix( J_ROOT2, eye_pers2, j_root );
	obj = objs->objs ;
	for ( i = n_objs ; i > 0 ; -- i, obj++, n-- ) {
		/* バウンディング及びトランスレーション情報の読み込み（１度に最大１２８オブジェクト分まで） */
		if ( n == 0 && last_objs != 0 ){
			n = ( last_objs > 128 ) ? 128 : last_objs ;
			last_objs -= n ;
		}
		obj->world = *j_root ;

		type = obj->mdl_type ;
		/* 拡張モデル処理 */
		if ( type & DG_TYPE_EXTEND ){
			DG_OBJ	*parent = &objs->objs[ obj->parent ] ;
			obj->screen = parent->screen ;
			obj->bound_mode = parent->bound_mode ;
			obj->sort_z = parent->sort_z ;
			continue ;
		}

		GV_PREFECH( &obj[1].world );
		GV_PREFECH( &obj[1].screen );
		GV_PREFECH( &obj[1].flag );
		/* バウンドチェック */
		if ( objs->bound_mode == 1 )
      {
			flag = BoundCheck( &new_screen, &obj->bound_min, 0, &pState->sort_z );
		} 
      else
      {
			flag = objs->bound_mode ;
			if ( type & DG_TYPE_TRANS || !( obj->flag & DG_FLAG_PAINT ) )
         {
            CheckSortZ( &new_screen, &obj->bound_min, &pState->sort_z );
         }
		}
#if 0
	{/* バウンディングのデバッグ表示 */
		extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
		//static FVECTOR max = {100,100,100,1}, min = {-100,-100,-100,1} ;
		//NewBoundingBoxView_1( joints, &max, &min, 0x00ffff00 );
		switch ( flag ){
		  case 0:
			NewBoundingBoxView_1( J_ROOT, &obj->model->lx, &obj->model->ux, 0x0000ffff );
			break ;
		  case 1:
			NewBoundingBoxView_1( J_ROOT, &obj->model->lx, &obj->model->ux, 0x008080ff );
			break ;
		  default:
			break ;
		}
	}
#endif
		/* バウンディングフラグ設定（０：クリップ必要なし、１：クリップ必要、２：完全画面外） */
		obj->bound_mode = flag ;
		/* 投影マトリクス設定 */
		if ( flag != 2 ) _CopyMatrix( &obj->screen, &new_screen );
		/* ソート値設定 */
		obj->sort_z = pState->sort_z ;
	}
}

#if 0
static void ScreenObjsNoBounds( DG_OBJS *objs, int n_objs )
{
   DG_OBJ		*obj ;
   int		i, flag, type ;

   _sceVu0MulMatrix( JOINTS, EYE_PERS, J_ROOT ) ;

   obj = objs->objs ;
   for ( 
      i = 0 ; 
      i < n_objs ; 
      i++, ++obj ) {

      //GV_PREFECH( &obj->world );
      //GV_PREFECH( &obj->screen );
      GV_PREFECH( &obj->flag );
      type = obj->mdl_type ;
      /* 拡張モデル処理 */
      if ( type & DG_TYPE_EXTEND ){
         DG_OBJ	*parent = &objs->objs[ obj->parent ] ;
         obj->inv_mat = parent->inv_mat ;
         obj->world = parent->world ;
         obj->screen = parent->screen ;
         obj->bound_mode = parent->bound_mode ;
         obj->sort_z = parent->sort_z ;
      }
      else
      {
         BP_MakeMatrix( &obj->world, _as_get_obj_parent_world_matrix( objs, obj ), &objs->objs[obj->parent].world, &obj->trans, EYE_PERS, &obj->screen, &obj->inv_mat );

         if ( type & DG_TYPE_TRANS || !( obj->flag & DG_FLAG_PAINT ) ){
            /* 半透明及び光源計算モデルの場合のみＺ座標代表値を求める（フォグ＆ソート用） */
            CheckSortZ( &obj->screen, &obj->bound_min, &SORT_Z );
         }

         obj->bound_mode = 1;
         obj->sort_z = SORT_Z ;
      }
   }

}
#endif

/*----------------------------------------------------------------*/

	/*
		各ユニットのワールドマトリクスから
		スクリーンマトリクスを計算する
	*/
static	void	ScreenObjs( DG_OBJS *objs, int n_objs, SObjsState *pState, FMATRIX const *eye_pers )
{
	DG_OBJ		*obj ;
	int		i, flag, type ;
   FMATRIX  tmp_screen;
   FMATRIX *parentMatrix;

	/* トランスレーションとバウンディング情報をスクラッチパッドへコピーする */
	obj = objs->objs ;
	for ( i = 0 ; i < n_objs ; i++ ) {

		//GV_PREFECH( &obj->world );
		//GV_PREFECH( &obj->screen );
		GV_PREFECH( &obj->flag );
		type = obj->mdl_type ;
		/* 拡張モデル処理 */
		if ( type & DG_TYPE_EXTEND ){
			DG_OBJ	*parent = &objs->objs[ obj->parent ] ;
			obj->inv_mat = parent->inv_mat ;
			obj->world = parent->world ;
			obj->screen = parent->screen ;
			obj->bound_mode = parent->bound_mode ;
			obj->sort_z = parent->sort_z ;
			obj++ ;
			continue ;
		}

#if 0//BP_ASM
		/* 大幅な計算手順変更バージョン */

		/* シングルウェイトエンベロープ用補正マトリクスを計算 */
		MakeMatrixStart( joints, JOINTS + obj->parent, &boundtrans->trans );
		GV_PREFECH( &obj[1].flag );
		GV_PREFECH( &obj[1].screen );
#if 0 /* バグってたので消した */
		{
			/* 奥行き値の計算（フォグ＆ソート用） */
			float	x, y, z ;
			DG_MULA( boundtrans->max.vx, 0.5f );
			x = DG_MADD( boundtrans->min.vx, 0.5f );	/* Ｘ中央値 */
			DG_MULA( boundtrans->max.vy, 0.5f );
			y = DG_MADD( boundtrans->min.vy, 0.5f );	/* Ｙ中央値 */
			DG_MULA( boundtrans->max.vz, 0.5f );
			z = DG_MADD( boundtrans->min.vz, 0.5f );	/* Ｚ中央値 */
			DG_MULA( work->tmp[0].m[0][3], x );
			DG_MADDA( work->tmp[0].m[1][3], y );
			DG_MADDA( work->tmp[0].m[2][3], z );
			z = DG_MADD( work->tmp[0].m[3][3], 1.0f );	/* Ｗ成分のみ計算（＝カメラからの距離） */
			obj->sort_z = DG_FTOI( z ) ;
		}
#endif
		MakeMatrixEnd( &work->tmp[0], &obj->inv_mat );
#else
      parentMatrix = _as_get_obj_parent_world_matrix(objs, obj);
      BP_MakeMatrix( &obj->world, parentMatrix, &obj->trans, eye_pers, &tmp_screen, &obj->inv_mat );
#endif//BP_ASM

		/* バウンドチェック */
		if ( objs->bound_mode == 1 ){
			/* バウンディング計算＆代表点Ｚ値計算 */
			flag = BoundCheck( &tmp_screen, &obj->bound_min, 0, &pState->sort_z );
		} else {
			flag = objs->bound_mode ;
#if 1
			if ( type & DG_TYPE_TRANS || !( obj->flag & DG_FLAG_PAINT ) ){
				/* 半透明及び光源計算モデルの場合のみＺ座標代表値を求める（フォグ＆ソート用） */
				CheckSortZ( &tmp_screen, &obj->bound_min, &pState->sort_z );
			}
#endif
		}

		/* バウンディングフラグ設定（０：クリップ必要なし、１：クリップ必要、２：完全画面外） */
		//obj->bound_mode = flag ;

      if ( obj->parent != -1 && objs->objs[ obj->parent ].bound_mode != flag )
      {
         obj->bound_mode = 1;
      }
      else
      {
         obj->bound_mode = flag;
      }

#if 0
      // Original JPN code
		work->bound_cache[ i ] = flag ;	/* 関節単体のバウンディング結果を保存 */
		if ( obj->parent != -1 &&
			 work->bound_cache[ i ] != work->bound_cache[ obj->parent ] )
      {
			obj->bound_mode = 1 ;	/* 親のバウンディング結果と食い違っている場合には念のためクリップありを設定 */
		}
      else 
      {
			obj->bound_mode = flag ;/* そうでなければ単体でのバウンディング結果を設定 */
		}
#endif
		/* 投影マトリクス設定 */
		if ( obj->bound_mode != 2 ) _CopyMatrix( &obj->screen, &tmp_screen );
		/* ソート値設定 */
		obj->sort_z = pState->sort_z ;

		obj ++ ;
	}
}


/*----------------------------------------------------------------*/

	/*
		横すべり関節型モデルについて処理

		objs->movs は、親ユニットからの相対位置
	*/
static	void	SlideFrameObjs( DG_OBJS *objs, int n_objs )
{
	FVECTOR		*movs, tmp ;
	DG_OBJ		*obj ;
	int		i ;

	movs = objs->movs ;
	obj = objs->objs ;
	for ( i = n_objs ; i > 0 ; -- i )
   {
      FMATRIX *parentMatrix = _as_get_obj_parent_world_matrix(objs, obj);
		_sceVu0ApplyMatrix( &tmp, parentMatrix, movs );

#if 0
      *joints = *parent ;
		joints->m[3][0] = tmp.vx ;
		joints->m[3][1] = tmp.vy ;
		joints->m[3][2] = tmp.vz ;
		obj->world = *joints ;
#else
      // AS - ELIMINATED TEMP STORAGE
      obj->world = *parentMatrix;
      obj->world.m[3][0] = tmp.vx ;
      obj->world.m[3][1] = tmp.vy ;
      obj->world.m[3][2] = tmp.vz ;
#endif
		movs ++ ;
		obj ++ ;
	}
}

/*----------------------------------------------------------------*/
	/*
		回転関節型モデルについて処理

		objs->rots は、ルートユニットからの相対回転
	*/
static	void	JointFrameObjs( DG_OBJS *objs, int n_objs )
{
	int			i ;
	DG_OBJ		*obj ;
	FVECTOR		*rots ;
   FMATRIX     work_mat;
   FMATRIX     *parentMatrix;

	_sceVu0UnitMatrix( &work_mat );

	obj = objs->objs ;
	rots = objs->rots ;
	for ( i = 0 ; i < objs->def->n_models ; i++, obj++, rots++ ){
		if ( !( objs->flag & DG_FLAG_QUATROT ) ){
			_sceVu0UnitMatrix( &work_mat );
			if ( rots->vx != 0.0f ) _sceVu0RotMatrixX( &work_mat, &work_mat, rots->vx );
			if ( rots->vy != 0.0f ) _sceVu0RotMatrixY( &work_mat, &work_mat, rots->vy );
			if ( rots->vz != 0.0f ) _sceVu0RotMatrixZ( &work_mat, &work_mat, rots->vz );
		} else {
			extern void MT_QuatToMat( FMATRIX*, FVECTOR* );
			MT_QuatToMat( &work_mat, rots );
		}
		*(FVECTOR*)(&work_mat.m[3]) = obj->trans ;
      parentMatrix = _as_get_obj_parent_world_matrix(objs, obj);
		_sceVu0MulMatrix( &obj->world, parentMatrix, (void*)&work_mat );
	}
}

/*----------------------------------------------------------------*/
	/*
		オブジェクトのマトリクスをセットアップする
	*/
static void		DG_SetObjsMatrix( ScrPad const *scrpad, DG_OBJS *objs )
{
	int		n_objs, n_joints ;
   SObjsState objsState;
   FMATRIX work_mat;
   FMATRIX j_root;

	n_objs = objs->n_models ;
	n_joints = objs->def->n_models ;
	/*
		物体自体に、親物体が指定されていた場合
		物体のワールド座標をコピーする
	*/
	if ( objs->root != NULL ) objs->world = *( objs->root ) ;
	j_root = objs->world ;
	if ( objs->flag & DG_FLAG_ONEPIECE ){
		/* 原点オフセット付加 */
		j_root.m[3][0] += objs->trans.vx ;
		j_root.m[3][1] += objs->trans.vy ;
		j_root.m[3][2] += objs->trans.vz ;
	}
	/*
		全体バウンディングを行う
	*/
	_sceVu0MulMatrix( &work_mat, EYE_PERS, &j_root ) ;

   if ( objs->flag & DG_FLAG_AS_NOBOUNDCHECK ) 
   {
      objs->bound_mode = 0;
   }
   else
   {
   	objs->bound_mode = BoundCheck( &work_mat, &objs->bound_min, ( objs->flag & DG_FLAG_ONEPIECE ) == 0, &objsState.sort_z );
   }
#if 0
	{/* ルートバウンディングのデバッグ表示 */
		extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
		//static FVECTOR max = {100,100,100,1}, min = {-100,-100,-100,1} ;
		//NewBoundingBoxView_1( J_ROOT, &max, &min, 0x00ff0000 );
		if ( objs->bound_mode != 2 )
		NewBoundingBoxView_1( J_ROOT, &objs->def->lx, &objs->def->ux, 0x000000ff );
	}
#endif
	/*
		処理フラグにしたがって、マトリクスのセットアップ
	*/
#if 0
   if ( ( DG_FLAG_AS_NOBOUNDCHECK & objs->flag ) )
   {
      if ( ( objs->flag & scrpad->invisible_flag ) && !( objs->flag & DG_FLAG_SHADOWMAKE ) ) return ;

      ScreenObjsNoBounds( objs, n_objs );
   } 
   else 
#endif
   if ( DG_FLAG_ONEPIECE & objs->flag ) {
		OnePieceObjs( objs, n_objs, &objsState, EYE_PERS, &j_root ) ;
#ifdef STAGE_CLIP_DEBUG
		if ( GV_PadData[0].press & PAD_AL ){
			int		i ;
			for ( i = 0 ; i < objs->n_models ; i++ )
			  if ( objs->objs[i].bound_mode != 1 ) objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
		}
		if ( GV_PadData[0].press & PAD_AR ){
			int		i ;
			for ( i = 0 ; i < objs->n_models ; i++ ) objs->objs[i].flag = objs->flag ;
		}
#endif
	} else if ( DG_FLAG_FINISHCALC & objs->flag ){
		if ( ( objs->flag & scrpad->invisible_flag ) && !( objs->flag & DG_FLAG_SHADOWMAKE ) ) return ;
		/* オブジェクトのマトリクスをまとめてスクラッチパッドへコピーする */
		ScreenObjs( objs, n_objs, &objsState, EYE_PERS ) ;
	} else {
		if ( objs->rots != NULL ) {
			JointFrameObjs( objs, n_joints ) ;
		} else if ( objs->movs != NULL ) {
			SlideFrameObjs( objs, n_joints ) ;
		} else {
			OnePieceObjs( objs, n_objs, &objsState, EYE_PERS, &j_root ) ;
			return ;
		}
		FlushCache( 0 );
		ScreenObjs( objs, n_objs, &objsState, EYE_PERS ) ;
	}
}

typedef struct _SObjsMatrixUltWork
{
   ScrPad const *mScrpad;
   DG_OBJS **ppQueue;
   int mStart;
   int mEnd;
} SObjsMatrixUltWork;

static void DG_SetObjsMatrixUltWork( SULTParam const *ult )
{
   SObjsMatrixUltWork const *work = (SObjsMatrixUltWork const *) ult;
   ScrPad const *scrpad = work->mScrpad;
   DG_OBJS **queue = work->ppQueue;
   int const startIndex = work->mStart;
   int const endIndex = work->mEnd;
   int i;

   for ( i = startIndex; i < endIndex; ++i )
   {
      DG_SetObjsMatrix( scrpad, queue[i] );
   }
}

static void DG_SetObjsMatrix_PostUlt( ScrPad const *scrpad, DG_OBJS **queue, int const n_queue)
{
   int i = 0;
   static const int skObjsPerUlt = 5;
   SObjsMatrixUltWork p = { scrpad, queue };

   for ( i = 0; i < n_queue; i += skObjsPerUlt )
   {
      int next = i + skObjsPerUlt;
      if ( next > n_queue )
      {
         next = n_queue;
      }

      p.mStart = i;
      p.mEnd = next;

      BP_Render_PostUltWork( DG_SetObjsMatrixUltWork, &p, sizeof( p ) );
   }

}

/*----------------------------------------------------------------*/
	/*
		オブジェクトのマトリクスをパケットメモリへ書き出す（後の描画処理時に参照する）
	*/

static void StoreMatrixObjs_Arm( DG_OBJS *objs, volatile int *dgBufferOffset, float const fog_param1, float const fog_param2 )
{
   int mdlIndex;
   KP_MatrixData *pCurrentMatrix;
   int const thisMatrixDataSize = sizeof( KP_MatrixData ) * objs->n_models;

   // Get our next write address
#if BP_VITA
   // On vita, do this as an atomic
   // that's because this is threaded
   int dgOldBufferOffset = sceAtomicAdd32( dgBufferOffset, thisMatrixDataSize );
#else
   // On other platforms, don't
   int dgOldBufferOffset = *dgBufferOffset;
   (*dgBufferOffset) += thisMatrixDataSize;
#endif

   pCurrentMatrix = (KP_MatrixData *) ( ((char *) DG_CurrentDmaAddr ) + dgOldBufferOffset );

   if ( ( DG_DisplayStatus & DG_STATE_IR_MODE ) && ( objs->flag & DG_FLAG_IRREACTION ) )
   {
      for ( 
         mdlIndex = 0; 
         mdlIndex < objs->n_models; 
         ++mdlIndex, ++pCurrentMatrix )
      {
         static FMATRIX	const ir_light[2] = {
            {{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
            {{{255,255,255,0},{0,0,0,0},{0,0,0,0},{128,128,128,128}}}
         };

         DG_OBJ *obj = &( objs->objs[ mdlIndex ] );
         pCurrentMatrix->local_light = ir_light[0];
         pCurrentMatrix->local_color = ir_light[1];

         pCurrentMatrix->local_light.m[0][0] = -obj->screen.m[0][2];
         pCurrentMatrix->local_light.m[1][0] = -obj->screen.m[1][2];
         pCurrentMatrix->local_light.m[2][0] = -obj->screen.m[2][2];

         obj->matrix_addr = pCurrentMatrix;
      }
   }
   else if ( !( objs->flag & DG_FLAG_PAINT ) )
   {
      if ( objs->flag & DG_FLAG_NOFOG )
      {
         for ( 
            mdlIndex = 0; 
            mdlIndex < objs->n_models; 
            ++mdlIndex, ++pCurrentMatrix )
         {
            DG_OBJ *obj = &( objs->objs[ mdlIndex ] );

            SetLightMatrix( &( pCurrentMatrix->local_light ), &obj->light[0], &obj->world );
            pCurrentMatrix->local_color = obj->light[1];

            obj->fog = ( 255 << 4 ) | 0x8000;

            obj->matrix_addr = pCurrentMatrix;
         }
      }
      else
      {
         for ( 
            mdlIndex = 0; 
            mdlIndex < objs->n_models; 
            ++mdlIndex, ++pCurrentMatrix )
         {
            DG_OBJ *obj = &( objs->objs[ mdlIndex ] );
            float f;
            int ifog;

            SetLightMatrix( &( pCurrentMatrix->local_light ), &obj->light[0], &obj->world );
            pCurrentMatrix->local_color = obj->light[1];

            f = fog_param2 + fog_param1 * ((float) obj->sort_z );

            ifog = ( f < 0.f ) ? 0 : ( ( f > 255.f ) ? 255 : ( DG_FTOI( f ) ) );
            obj->fog = ( ifog << 4 ) | 0x8000;

            obj->matrix_addr = pCurrentMatrix;
         }
      }
   }
   else
   {
      // Loop through and just set a garbage pCurrentMatrix
      for ( 
         mdlIndex = 0; 
         mdlIndex < objs->n_models; 
         ++mdlIndex, ++pCurrentMatrix )
      {
         DG_OBJ *obj = &( objs->objs[ mdlIndex ] );

         obj->matrix_addr = pCurrentMatrix;
      }
   }
}

#if 0
static void StoreMatrixObjs_old( ScrPad const	*scrpad, DG_OBJS *objs )
{
	DG_OBJ		*obj ;
	int			i, j, n, last, clock ;
	typedef struct _stack_data {
		FMATRIX		screen ;
		FMATRIX		local_light ;
		FMATRIX		local_color ;
		FMATRIX		connection ;
		FMATRIX		option ;
		int			n_verts ;		/* ここでは意味を持たない */
		int			flag ;			/* ここでは意味を持たない */
		int			fog ;			/* ここでは意味を持たない */
		int			bound ;			/* ここでは意味を持たない */
		DG_GIFTAG	giftag_normal ;	/* 通常描画用描画ＧＩＦタグ */
		DG_GIFTAG	giftag_clip ;	/* クリップポリゴン用描画ＧＩＦタグ */
		IVECTOR		gif_regslist ;	/* マルチテクスチャ用レジスタリスト */
	} StackData ;
	typedef struct {
		struct {
			FMATRIX			world ;
			FMATRIX			screen ;
			FMATRIX			inv_mat ;
		} matrix[8] ;
		FMATRIX				lights[8][2] ;
		StackData	m_stack[8] ;
	} OBJ_MATRIX ;
	typedef struct {
		OBJ_MATRIX		buffer[2] ;
	} MATRIX_WORK ;
	MATRIX_WORK	*local_work ;
	OBJ_MATRIX	*obj_matrix ;
	static DG_GIFTAG	giftag_normal = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_clip = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0),SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_normal_mt = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 5),
		  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_clip_mt = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 5),
		  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) };
	static IVECTOR		gif_regslist = 
	  {GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
		 GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
		 GS_REGS_5( GS_REGS_NOP, GS_REGS_STQ, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
		 GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2)} ;

	local_work = (MATRIX_WORK*)scrpad->joints ;
	obj = objs->objs ;

	clock = 0; // scrpad->store_clock ;

	/* 先行読み込み */
	n = ( objs->n_models > 8 ) ? 8 : objs->n_models ;
	DG_StartMemToSprInterleave( &local_work->buffer[clock].matrix[0].world, &obj->world,
							   SIZEOF_QWORD(FMATRIX) * 3 * n,
							   SIZEOF_QWORD(FMATRIX) * 3,
							   SIZEOF_QWORD(DG_OBJ) - SIZEOF_QWORD(FMATRIX) * 3 );

#if 0 //BP_ASM - this is PS2 specific - it waits for preceding load/stores to complete and is not needed on other platforms
	asm volatile( "sync.l;sync.p" );
#endif

	last = objs->n_models ;
	for ( i = 0 ; i < objs->n_models ; i+=8 ){

		/* マトリクスプリロードバッファの切り替え処理 */
		n = ( last > 8 ) ? 8 : last ;
		last -= n ;
		obj_matrix = &local_work->buffer[clock] ;
		clock = 1 - clock ;
		DG_EndMemToSpr();

		/* 先行読み込みチェック */
		if ( last > 0 ){
			int		nn ;
			nn = ( last > 8 ) ? 8 : last ;
			DG_StartMemToSprInterleave( &local_work->buffer[clock].matrix[0].world, &obj[8].world,
									   SIZEOF_QWORD(FMATRIX) * 3 * nn,
									   SIZEOF_QWORD(FMATRIX) * 3,
									   SIZEOF_QWORD(DG_OBJ) - SIZEOF_QWORD(FMATRIX) * 3 );
		}

		/* 読み込んだマトリクスに対して処理を行う */
		for ( j = 0 ; j < n ; j++, obj++ )
      {
			if ( !( objs->flag & DG_FLAG_PAINT ) )
         {
				/* ライトマトリクス計算 */
				SetLightMatrix( &obj_matrix->m_stack[j].local_light,
							   &obj->light[0], &obj_matrix->matrix[j].world );
				Vu0CopyMatrix( &obj_matrix->m_stack[j].local_color, &obj->light[1] );

				/* フォグ計算 */
				if ( !( objs->flag & DG_FLAG_NOFOG ) )
            {
					float	f ;
					int		ifog ;
					GV_PREFECH( &obj[1].sort_z );
					//f =  scrpad->fog_param1 * obj->sort_z + scrpad->fog_param2 ;
					f = (float)obj->sort_z ;
					DG_ADDA( scrpad->fog_param2, 0.0f );
					f = DG_MADD( scrpad->fog_param1, f );
					f = DG_MAX( f, 0.0f );
					f = DG_MIN( f, 255.0f );
					ifog = DG_FTOI( f ) ;
					obj->fog = ( ifog << 4 ) | 0x8000 ;
				} 
            else 
            {
					obj->fog = ( 255 << 4 ) | 0x8000 ;
				}

				/* エンベロープ用マトリクス設定 */
				Vu0CopyMatrix( &obj_matrix->m_stack[j].connection, &obj_matrix->matrix[j].inv_mat );
			}
         else
         {
            Vu0CopyMatrix( &obj_matrix->m_stack[j].connection, &obj_matrix->matrix[j].inv_mat );
         }

			/* 透視変換マトリクス設定 */
			Vu0CopyMatrix( &obj_matrix->m_stack[j].screen, &obj_matrix->matrix[j].screen );

			if ( !( objs->flag & DG_FLAG_MULTITEX ) ){
				/* その他のデータを設定（ＶＩＦ内部で使用する定数など） */
				obj_matrix->m_stack[j].giftag_normal = giftag_normal ;
				obj_matrix->m_stack[j].giftag_clip = giftag_clip ;
				obj_matrix->m_stack[j].gif_regslist = gif_regslist ;
			} else {
				/* その他のデータを設定（ＶＩＦ内部で使用する定数など） */
				obj_matrix->m_stack[j].giftag_normal = giftag_normal_mt ;
				obj_matrix->m_stack[j].giftag_clip = giftag_clip_mt ;
				obj_matrix->m_stack[j].gif_regslist = gif_regslist ;
				/* オプションマトリクスを計算 */
				if ( 1 ){	/* 現在は環境マッピングのみ */
					//_sceVu0MulMatrix( &scrpad->option_mat, &scrpad->eye_inv, &obj->world );
					_sceVu0MulMatrix( &obj_matrix->m_stack[j].option, &scrpad->eye_inv, &obj_matrix->matrix[j].world );
				}
			}


#if 1
			if ( ( DG_DisplayStatus & DG_STATE_IR_MODE ) && ( objs->flag & DG_FLAG_IRREACTION ) ){
				obj_matrix->m_stack[j].giftag_normal.tag &= ~SCE_GIF_SET_TAG(0, 0, 0, SCE_GS_SET_PRIM( 0, 0, 1, 0, 1, 0, 0, 0, 0), 0, 0) ;
				obj_matrix->m_stack[j].giftag_clip.tag &= ~SCE_GIF_SET_TAG(0, 0, 0, SCE_GS_SET_PRIM( 0, 0, 1, 0, 1, 0, 0, 0, 0), 0, 0) ;
#if 0
#ifndef PSX2
			*(FVECTOR*)&obj_matrix->m_stack[j].local_color.m[0][0] = DG_ZeroVector ;
			*(FVECTOR*)&obj_matrix->m_stack[j].local_color.m[1][0] = DG_ZeroVector ;
			*(FVECTOR*)&obj_matrix->m_stack[j].local_color.m[2][0] = DG_ZeroVector ;
#else
			asm ("sqc2	$vf0,0(%0)"::"r"(&obj_matrix->m_stack[j].local_color));
			asm ("sqc2	$vf0,16(%0)"::"r"(&obj_matrix->m_stack[j].local_color));
			asm ("sqc2	$vf0,32(%0)"::"r"(&obj_matrix->m_stack[j].local_color));
#endif
				obj_matrix->m_stack[j].local_color.m[3][0] = 255 ;
				obj_matrix->m_stack[j].local_color.m[3][1] = 168 ;
				obj_matrix->m_stack[j].local_color.m[3][2] = 0 ;
#else
				{/* ＸＢＯＸ方式互換設定 */
					static FMATRIX	ir_light[2] = {
						{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
						{{{255,255,255,0},{0,0,0,0},{0,0,0,0},{128,128,128,128}}}
					};
					obj_matrix->m_stack[j].local_light = ir_light[0] ;
					obj_matrix->m_stack[j].local_color = ir_light[1] ;
					/* かならずカメラ方向から光が当たっている状態に */
					obj_matrix->m_stack[j].local_light.m[0][0] = -obj->screen.m[0][2] ;
					obj_matrix->m_stack[j].local_light.m[1][0] = -obj->screen.m[1][2] ;
					obj_matrix->m_stack[j].local_light.m[2][0] = -obj->screen.m[2][2] ;
				}
#endif
			}
#endif

			/* データ書き込み位置の記憶 */
			obj->matrix_addr = DG_CurrentDmaAddr + SIZEOF_QWORD(struct _stack_data) * j ;
		}

		/* マトリクス設定データをまとめてパケット領域に転送 */
#if 0 //BP_ASM - this is PS2 specific - it waits for preceding load/stores to complete and is not needed on other platforms
		asm volatile( "sync.l;sync.p" );
#endif
		DG_EndSprToMem();
		DG_StartSprToMem( DG_CurrentDmaAddr, &obj_matrix->m_stack[0], SIZEOF_QWORD(struct _stack_data) * n );
		DG_CurrentDmaAddr += SIZEOF_QWORD(struct _stack_data) * n ;
	}
//	scrpad->store_clock = clock ;
}
#endif

/*----------------------------------------------------------------*/
static void SetupLowObjs( DG_OBJS *objs )
{
	DG_OBJS		*low_objs ;
	DG_OBJ		*obj, *low_obj ;
	int		i ;
	obj = objs->objs ;
	low_objs = objs->low ;
	low_obj = low_objs->objs ;
	/* objsの内容を複製する */
	low_objs->world = objs->world ;
	low_objs->group_id = objs->group_id ;
	low_objs->bound_mode = objs->bound_mode ;

	for ( i = 0 ; i < low_objs->n_models ; i++ ){
		low_obj->world = obj->world ;
		low_obj->screen = obj->screen ;
		low_obj->inv_mat = obj->inv_mat ;
		low_obj->sort_z = obj->sort_z ;
		low_obj->fog = obj->fog ;
		low_obj->matrix_addr = obj->matrix_addr ;

		obj++ ;
		low_obj++ ;
	}
}

/*----------------------------------------------------------------*/
// DG_PreScreen is used by the buffered drawing code
// it copies the matrices around like the ScreenChanl code did
// but doesn't do any bounding checks

static void	_preOnePieceObjs( DG_OBJS *objs, FMATRIX *pWorld )
{
   int i;
   DG_OBJ *obj = objs->objs;
   for ( i = objs->n_models ; i > 0 ; --i, ++obj )
   {
      obj->world = *pWorld;
   }
}

static void _preSetupLowObjs( DG_OBJS *objs )
{
   DG_OBJS *low_objs;
   DG_OBJ *obj, *low_obj;
   int i;

   low_objs = objs->low;
   low_objs->world = objs->world;

   /* objsの内容を複製する */

   low_obj = low_objs->objs;
   obj = objs->objs;
   for ( i = low_objs->n_models; i > 0; --i, ++low_obj, ++obj )
   {
      low_obj->world = obj->world;
   }
}


void DG_PreScreenObjs( DG_OBJS *objs )
{
	/*
		物体自体に、親物体が指定されていた場合
		物体のワールド座標をコピーする
	*/
   FMATRIX worldMatrix;

   if ( objs->root != NULL )
   {
      objs->world = *( objs->root );
   }
	worldMatrix = objs->world;
	/*
		処理フラグにしたがって、マトリクスのセットアップ
	*/
	if ( objs->flag & DG_FLAG_ONEPIECE )
   {
      /* 原点オフセット付加 */
      worldMatrix.m[3][0] += objs->trans.vx;
      worldMatrix.m[3][1] += objs->trans.vy;
      worldMatrix.m[3][2] += objs->trans.vz;
		_preOnePieceObjs( objs, &worldMatrix );
	}
   else if ( objs->flag & DG_FLAG_FINISHCALC )
   {
	}
   else
   {
		if ( objs->rots != NULL )
      {
			JointFrameObjs( objs, objs->def->n_models );
		}
      else if ( objs->movs != NULL )
      {
			SlideFrameObjs( objs, objs->def->n_models );
		}
      else
      {
			_preOnePieceObjs( objs, &worldMatrix );
		}
	}
   if (objs->low != NULL)
   {
      _preSetupLowObjs(objs);
   }
}

void DG_PreScreen( DG_OBJ_QUEUE *pQueue )
{
	if ( pQueue != NULL )
   {
	   DG_OBJ_BUFFER *pObjectBuffer = &pQueue->objs_buffer;
      int i;
      /* マトリクスのセットアップ */
      DG_OBJS **ppObjs = (DG_OBJS **)pObjectBuffer->queue;
      for ( i = pObjectBuffer->n_queue; i > 0; --i, ++ppObjs )
      {
	      DG_PreScreenObjs( *ppObjs );
      }
   }
}

static void FinalizeObjs( ScrPad const *scrpad, volatile int *dgBufferOffset, DG_OBJS *objs )
{
   float fog_param1;
   float fog_param2;

#if 0
   if ( cp->chanl_num < 2 ){
      if ( ( objs->flag & scrpad->invisible_flag ) && !( objs->flag & DG_FLAG_SHADOWMAKE ) ) continue ;
   } else {
      if ( objs->flag & scrpad->invisible_flag ) continue ;
   }
#else
   if ( ( objs->flag & scrpad->invisible_flag ) || ( objs->bound_mode == 2 ) )
   {
      /* ローポリ用モデルが関連付けされている場合にはセットアップ内容を複製する */
      /* このときは親が非表示になっているためセットアップパケットアドレスは意味をなさない */
      if ( objs->low != NULL )
      {
         SetupLowObjs( objs );
      }
      return ;
   }
#endif
   /* オブジェクト固有フォグパラメータ設定 */

   if ( objs->flag & DG_FLAG_FOGPARAM )
   {
      fog_param1 = objs->fog_param[0] ;
      fog_param2 = objs->fog_param[1] ;
   }
   else
   {
      fog_param1 = scrpad->fog_param1;
      fog_param2 = scrpad->fog_param2;
   }

   StoreMatrixObjs_Arm( objs, dgBufferOffset, fog_param1, fog_param2 );
   //StoreMatrixObjs_old( *oque );

   /* ローポリ用モデルが関連付けされている場合にはセットアップ内容を複製する */
   if ( objs->low != NULL ){
      SetupLowObjs( objs );
   }
}

typedef struct _SFinalizeObjsUltWorkUltWork
{
   ScrPad const *mScrpad;
   DG_OBJS **ppQueue;
   volatile int *mDGOffset;
   int mStart;
   int mEnd;
} SFinalizeObjsUltWorkUltWork;


static void FinalizeObjsUltWork( SULTParam const *ult )
{
   SFinalizeObjsUltWorkUltWork const *work = (SFinalizeObjsUltWorkUltWork const *) ult;
   ScrPad const *scrpad = work->mScrpad;
   DG_OBJS **queue = work->ppQueue;
   volatile int *dgOffset = work->mDGOffset;
   int const startIndex = work->mStart;
   int const endIndex = work->mEnd;
   int i;

   for ( i = startIndex; i < endIndex; ++i )
   {
      FinalizeObjs( scrpad, dgOffset, queue[i] );
   }
}

static void FinalizeObjs_PostUlt( ScrPad const *scrpad, volatile int *dgOffset, DG_OBJS **queue, int const n_queue)
{
   int i = 0;
   static const int skObjsPerUlt = 20;
   SFinalizeObjsUltWorkUltWork p = { scrpad, queue, dgOffset };

   for ( i = 0; i < n_queue; i += skObjsPerUlt )
   {
      int next = i + skObjsPerUlt;
      if ( next > n_queue )
      {
         next = n_queue;
      }

      p.mStart = i;
      p.mEnd = next;

      BP_Render_PostUltWork( FinalizeObjsUltWork, &p, sizeof( p ) );
   }

}

/*----------------------------------------------------------------*/
	/*
		カメラに応じたマトリクスのセットアップを行う
	*/
void		DG_ScreenChanl( DG_CHANL *cp, int which )
{
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	ScrPad	*scrpad = (ScrPad*)SCRPAD_ADDR ;
	int		i ;

	que = cp->obj_queue ;
	MARK( "screen.c" );
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;
	if ( obj_buff->n_queue == 0 ) return ;
	GV_SET_PRFC_CLOCK();

	/* マイクロプログラム準備 */
	DG_StartBoundingCheckSupport();

	/*
		カメラ逆行列を、スクラッチパッドに複製しておく
		その際、ピクセルアスペクト比率を調整しておく
	*/
	*EYE_PERS = cp->eye_pers ;
//	*EYE_PERS2 = cp->eye_pers2 ;
	scrpad->eye_inv = cp->eye_inv ;
//	scrpad->eye = cp->eye ;
//	SCALE->vx = cp->width / 2 ;
//	SCALE->vy = cp->height / 2 ;
//	SCREEN = 0.5F ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;

	FlushCache( 0 );

   // AS(JM) - Drain "screen" work in case "screen" is called multiple times for the
   // same obj's so that the objs' transforms can make it to the renderer. This
   // happens during alert status when you see the picture-in-picture.
   BP_Render_HelpDrainUltWork();

	/* マトリクスのセットアップ */
   oque = (DG_OBJS**)obj_buff->queue ;
   if( gAS_UsedBufferSceneNOP )
   {
      DG_SetObjsMatrix_PostUlt( scrpad, (DG_OBJS **) obj_buff->queue, obj_buff->n_queue );
      BP_Render_HelpDrainUltWork();
   }
   else
   {
      for ( i = obj_buff->n_queue ; i > 0 ; --i, ++oque )
      {
         DG_SetObjsMatrix( scrpad, *oque );
      }
   }
	if ( DG_UnDrawFrameCount != 0 ) return ;

	FlushCache( 0 );
	/* マトリクス情報のメモリへの書き出し */
	oque = (DG_OBJS**)obj_buff->queue ;

   {
      volatile int dgBufferOffset = 0;

      if ( gAS_UsedBufferSceneNOP )
      {
         FinalizeObjs_PostUlt( scrpad, &dgBufferOffset, (DG_OBJS **) obj_buff->queue, obj_buff->n_queue );
         BP_Render_HelpDrainUltWork();
      }
      else
      {
         for ( i = obj_buff->n_queue ; i > 0 ; i--, oque++ )
         {
            objs = *oque ;

            FinalizeObjs( scrpad, &dgBufferOffset, objs );
	      }
      }

      // Increment DG_CurrentDmaAddr.  Make sure it's aligned to 16-bytes
      {
         KP_CTASSERT( sizeof( *DG_CurrentDmaAddr ) == 16 );
         DG_CurrentDmaAddr += ( dgBufferOffset + 15 ) >> 4;
      }
   }

	DG_EndSprToMem();

#if 0
	if ( GV_PadData[1].press & PAD_A )
   {
      int time = GV_GET_PRFC_CLOCK();
		printf("screen.c: %d\n", time );
	}
#endif
#ifdef DEBUG_MODE
	if ( DG_OnePieceSkip ){
		oque = (DG_OBJS**)obj_buff->queue ;
		for ( i = obj_buff->n_queue ; i > 0 ; i--, oque++ ) {
			objs = *oque ;
			if ( cp->chanl_num < 2 ){
				if ( ( objs->flag & scrpad->invisible_flag ) && !( objs->flag & DG_FLAG_SHADOWMAKE ) ) continue ;
				if ( objs->flag & DG_FLAG_ONEPIECE ) {
					if ( DG_OnePieceSkip == 1 ) {
						objs->flag |= DG_FLAG_SEMITRANS ;
					} else {
						objs->flag &= ~DG_FLAG_SEMITRANS ;
					}
				}
			}
		}
		DG_OnePieceSkip = 0 ;
	}
#endif
}

