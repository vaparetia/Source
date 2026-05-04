//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pos.c
	三次元座標計算ユーティリティ

	1999/07/07 K.Takabe
	$Id: pos.c,v 1.1.1.3 2002/11/19 11:42:20 Yoshizawa1 Exp $

*/
/*

	void		DG_GetPos( FMATRIX *world )
	FMATRIX		*world ;	取得マトリクス

		現在位置を取得する

	void		DG_SetPos( world )
	FMATRIX		*world ;	ワールドマトリクス

		現在位置を、マトリクス代入で初期化する
	
	void		DG_SetPos2( mov, rot )
	FVECTOR		*mov ;		位置
	SVECTOR		*rot ;		方向

		現在位置を、位置と方向で初期化する
	
	void		DG_MovePos( mov )
	FVECTOR		*mov ;		相対移動量

		現在位置を、相対移動させる
	
	void		DG_RotatePos( rot )
	SVECTOR		*rot ;		相対回転量

		現在位置を、相対回転させる
	
	void		DG_RotatePosYZX( rot )
	SVECTOR		*rot ;		相対回転量

		現在位置を、相対回転させる（Ｘ、Ｙ、Ｚの順＝SoftImage準拠）

	void		DG_ScalePos( FVECTOR *scale )
	FVECTOR		*scale ;	現在位置にスケールをかける（x:Ｘ成分 y:Ｙ成分 z:Ｚ成分 w:中心座標）

		現在位置にスケールをかける
	
	void		DG_PutObjs( objs )
	DG_OBJS		*objs ;		物体ハンドラ

		物体ハンドラを、現在位置に配置する

	void		DG_PutPrim( prim )
	DG_PRIM		*prim ;		プリミティブ

		プリミティブを、現在位置に配置する

	void		DG_PutVector( from, to, n )
	FVECTOR		*from ;		相対位置ベクトル（入力）
	FVECTOR		*to ;		絶対位置ベクトル（出力）
	int		n ;		ベクトル数

		相対位置ベクトルを、現在位置に配置する

	void		DG_RotVector( from, to, n )
	FVECTOR		*from ;		相対方向ベクトル（入力）
	FVECTOR		*to ;		絶対方向ベクトル（出力）
	int		n ;		ベクトル数

		相対方向ベクトルを、現在位置に配置する

	int DG_PointCheckOne( FVECTOR *vec, int flag )
	FVECTOR		*vec ;		チェック座標
	int			flag ;		チェックフラグ（チャンネルの指定）

		指定したベクトルが画面内に入っているかどうかをチェック
		その点が画面内に入っていたら０以外が返る

	int DG_BoundCheck( FMATRIX *world, FVECTOR *bound_max, FVECTOR *bound_min )
	FMATRIX		*world ;		バウンディングのローカルマトリクス
	FVECTOR		*bound_max ;	バウンディングを表す最大座標
	FVECTOR		*bound_min ;	バウンディングを表す最小座標

		指定したカメラの視界内に入っているかをチェック
		カメラ外であれば対応したビットが１になるが（カメラの最大は４つまで）
		全てのカメラから見えなければ－１が返る
		（０なら全てのカメラから見える）

##### no support

	void		DG_PersVector( from, to, n )
	SVECTOR		*from ;		相対位置ベクトル（入力）
	DVECTOR		*to ;		表示座標ベクトル（出力）
	int		n ;		ベクトル数

		相対位置ベクトルを、現在位置に配置し
		パース変換した表示座標を計算する

	void DG_PointCheck( SVECTOR *vec, long64 n_vec );

		現在のマトリクスをセーブしてから、カメラ行列をセットし、
		ポイントチェックの開始を宣言。スクラッチパッドの内容を破壊
		vecのポイント列が画面上に見えているかどうかを計算する。
		結果はvec->padに 0 or 1で入る。

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

#include "bp_matrix.h"//BP_ASM
#include "BP_Asm.h"//BP_ASM

#define _CopyVector( a, b ) { *(u_long128*)a = *(u_long128*)b ; }
#define _CopyMatrix( a, b ) { 0[(u_long128*)a] = 0[(u_long128*)b] ;\
								1[(u_long128*)a] = 1[(u_long128*)b] ;\
								2[(u_long128*)a] = 2[(u_long128*)b] ;\
								3[(u_long128*)a] = 3[(u_long128*)b] ; }


SVECTOR	DG_ZeroSVector = {0,0,0,0};
IVECTOR	DG_ZeroIVector = {0,0,0,0};
FVECTOR	DG_ZeroVector = {0.0F,0.0F,0.0F,1.0F};
FMATRIX	DG_UnitMatrix = {{
	{1.0F,0.0F,0.0F,0.0F},
	{0.0F,1.0F,0.0F,0.0F},
	{0.0F,0.0F,1.0F,0.0F},
	{0.0F,0.0F,0.0F,1.0F}
}};



/* ＰＳの定数回転マトリクスに相当 */
#if BP_VITA
__thread
#endif
static FMATRIX	WorkMatrix ;

/* ＰＳ式角度を-2048~2047に正規化する */
#define REGULAR_ANGLE( _a )	( (signed short)( ((unsigned short)(_a)) << 4 ) >> 4 )

/*----------------------------------------------------------------*/

#if BP_VITA
__thread
#endif
static FMATRIX  gMatrix = {0};//BP_ASM

static inline void _SetMatrix( FMATRIX *m )
{
#if 0 //BP_ASM
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	": : "r"(m) );
#else
    BP_Mat_Copy( &gMatrix, m );
#endif
}

static inline void _GetMatrix( FMATRIX *m )
{
#if 0 //BP_ASM
	asm ("
	sqc2		vf4,0x00(%0)
	sqc2		vf5,0x10(%0)
	sqc2		vf6,0x20(%0)
	sqc2		vf7,0x30(%0)
	": : "r"(m) );
#else
    BP_Mat_Copy( m, &gMatrix );
#endif
}

static inline void _RotTrans( FVECTOR *res, FVECTOR *v )
{
#if 0 //BP_ASM
	asm ("
	lqc2			vf8,0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
#else
    BP_Mat_TransformVec3( res, &gMatrix, v );
#endif
}

static inline void _RotVector( FVECTOR *res, FVECTOR *v )
{
#if 0 //BP_ASM
	asm ("
	lqc2			vf8,0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddz.xyzw		vf8, vf6,vf8
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
#else
    BP_Mat_RotateVec3( res, &gMatrix, v );
#endif
}

static inline void _ScaleMatrix( FVECTOR *v )
{
#if 0 //BP_ASM
	asm ("
	lqc2			vf8,0x00(%0)
	vmulx.xyz		vf4,vf4,vf8
	vmuly.xyz		vf5,vf5,vf8
	vmulz.xyz		vf6,vf6,vf8
	vmulw.xyz		vf7,vf7,vf8
	":: "r"(v) );
#else
    FVECTOR* row = (FVECTOR*)(&gMatrix.m[0][0]);
    BP_Vec3_MulFloat( &row[0], &row[0], v->vx );
    BP_Vec3_MulFloat( &row[1], &row[1], v->vy );
    BP_Vec3_MulFloat( &row[2], &row[2], v->vz );
    BP_Vec3_MulFloat( &row[3], &row[3], v->vw );
#endif
}

/*----------------------------------------------------------------*/

void DG_SinCos( float *s, float *c, float rot )
{
   *s = sinf(rot);
   *c = cosf(rot);
}

void		DG_GetPos( FMATRIX *world )
{
	DG_COPY_MAT( world, &WorkMatrix );
}

void		DG_SetPos( FMATRIX *world )
{
	// WorkMatrix = *world ;
	DG_COPY_MAT( &WorkMatrix, world );
}

void		DG_SetPos2( FVECTOR *mov, SVECTOR *rot )
{
	float		rot_x, rot_y, rot_z ;
	//int			tmp ;

	//RotMatrixYXZ_gte( rot, &tmp_mat ) ;
#if 0
	tmp = rot->vz ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_z = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vx ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_x = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vy ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_y = (float)tmp * (float)M_PI / 2048.0F ;
#else
	rot_z = (float)REGULAR_ANGLE(rot->vz) * (float)M_PI / 2048.0f ;
	rot_x = (float)REGULAR_ANGLE(rot->vx) * (float)M_PI / 2048.0f ;
	rot_y = (float)REGULAR_ANGLE(rot->vy) * (float)M_PI / 2048.0f ;
#endif
	_sceVu0UnitMatrix( &WorkMatrix );
	_sceVu0RotMatrixZ( &WorkMatrix, &WorkMatrix, rot_z );
	_sceVu0RotMatrixX( &WorkMatrix, &WorkMatrix, rot_x );
	_sceVu0RotMatrixY( &WorkMatrix, &WorkMatrix, rot_y );
	WorkMatrix.m[ 3 ][ 0 ] = mov->vx ;
	WorkMatrix.m[ 3 ][ 1 ] = mov->vy ;
	WorkMatrix.m[ 3 ][ 2 ] = mov->vz ;
}

void		DG_MovePos( FVECTOR *mov )
{
	FVECTOR		tmp_vec ;

	_SetMatrix( &WorkMatrix );
	_RotTrans( &tmp_vec, mov );
	WorkMatrix.m[ 3 ][ 0 ] = tmp_vec.vx ;
	WorkMatrix.m[ 3 ][ 1 ] = tmp_vec.vy ;
	WorkMatrix.m[ 3 ][ 2 ] = tmp_vec.vz ;
}

void		DG_RotatePos( SVECTOR *rot )
{
	FMATRIX		tmp_mat ;
	float		rot_x, rot_y, rot_z ;
	int			tmp ;


	tmp = rot->vz ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_z = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vx ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_x = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vy ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_y = (float)tmp * (float)M_PI / 2048.0F ;
	_sceVu0UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixZ( &tmp_mat, &tmp_mat, rot_z );
	_sceVu0RotMatrixX( &tmp_mat, &tmp_mat, rot_x );
	_sceVu0RotMatrixY( &tmp_mat, &tmp_mat, rot_y );
	_sceVu0MulMatrix( &WorkMatrix, &WorkMatrix, &tmp_mat );
}

void		DG_RotatePosZYX( SVECTOR *rot )
{
	FMATRIX		tmp_mat ;
	float		rot_x, rot_y, rot_z ;
	int			tmp ;


	tmp = rot->vz ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_z = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vx ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_x = (float)tmp * (float)M_PI / 2048.0F ;
	tmp = rot->vy ; tmp = ( tmp & 0x0800 ) ? ( tmp | 0xfffff000 ) : ( tmp & 0xfff );
	rot_y = (float)tmp * (float)M_PI / 2048.0F ;
	_sceVu0UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixX( &tmp_mat, &tmp_mat, rot_x );
	_sceVu0RotMatrixY( &tmp_mat, &tmp_mat, rot_y );
	_sceVu0RotMatrixZ( &tmp_mat, &tmp_mat, rot_z );
	_sceVu0MulMatrix( &WorkMatrix, &WorkMatrix, &tmp_mat );
}

void		DG_ScalePos( FVECTOR *scale )
{
	_SetMatrix( &WorkMatrix );
	_ScaleMatrix( scale );
	_GetMatrix( &WorkMatrix );
}


/*----------------------------------------------------------------*/

void		DG_PutObjs( DG_OBJS *objs )
{
	// objs->world = WorkMatrix ;
	DG_COPY_MAT( &objs->world, &WorkMatrix );
}

void		DG_PutPrim( DG_PRIM *prim )
{
	// prim->world = WorkMatrix ;
	DG_COPY_MAT( &prim->world, &WorkMatrix );
}

void		DG_PutVector( FVECTOR *from, FVECTOR *to, int n )
{
	_SetMatrix( &WorkMatrix );
	while ( -- n >= 0 ) {
		_RotTrans( to, from );
		from ++ ;
		to ++ ;
	}
}

void		DG_RotVector( FVECTOR *from, FVECTOR *to, int n )
{
	_SetMatrix( &WorkMatrix );
	while ( -- n >= 0 ) {
		_RotVector( to, from );
		from ++ ;
		to ++ ;
	}
}

void		DG_ReflectMatrix( FVECTOR *pole, FMATRIX *m1, FMATRIX *m2 )
{
    FMATRIX	tmp_mat ;	
    FVECTOR	ip_vec ;
    float	ip ;

    _sceVu0TransposeMatrix( &tmp_mat, m1 ) ;
    DG_COPY_MAT( &WorkMatrix, &tmp_mat );
    _SetMatrix( &WorkMatrix );
    _RotVector( &ip_vec, pole ) ;
    ip = ip_vec.vx ;
    m2->m[ 0 ][ 0 ] = m1->m[ 0 ][ 0 ] - pole->vx * ip * 2.0F ;
    m2->m[ 0 ][ 1 ] = m1->m[ 0 ][ 1 ] - pole->vy * ip * 2.0F ;
    m2->m[ 0 ][ 2 ] = m1->m[ 0 ][ 2 ] - pole->vz * ip * 2.0F ;
    ip = ip_vec.vy ;
    m2->m[ 1 ][ 0 ] = m1->m[ 1 ][ 0 ] - pole->vx * ip * 2.0F ;
    m2->m[ 1 ][ 1 ] = m1->m[ 1 ][ 1 ] - pole->vy * ip * 2.0F ;
    m2->m[ 1 ][ 2 ] = m1->m[ 1 ][ 2 ] - pole->vz * ip * 2.0F ;
    ip = ip_vec.vz ;
    m2->m[ 2 ][ 0 ] = m1->m[ 2 ][ 0 ] - pole->vx * ip * 2.0F ;
    m2->m[ 2 ][ 1 ] = m1->m[ 2 ][ 1 ] - pole->vy * ip * 2.0F ;
    m2->m[ 2 ][ 2 ] = m1->m[ 2 ][ 2 ] - pole->vz * ip * 2.0F ;
}

/* 反射ベクトルを求める         */
/* pole:入力（保存）:法線（非正規ＯＫ） */
/* vec1:入力（保存）:入射ベクトル       */
/* vec2:出力（破壊）:反射ベクトル       */
void		DG_ReflectVector( FVECTOR *pole, FVECTOR *vec1, FVECTOR *vec2 )
{
	float	ftemp;
	FVECTOR	fvtemp;
	DG_COPY_VEC( &fvtemp, pole );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	ftemp = _sceVu0InnerProduct( &fvtemp, vec1 );
	_sceVu0ScaleVectorXYZ( &fvtemp, &fvtemp, -ftemp*2.0f );
	_sceVu0AddVector( vec2, &fvtemp, vec1 );
//	vec2->vw=1.0f; /* なんとなく */
}

#if 0
void			DG_MatrixRotZYX( mat, rot )
FMATRIX				*mat ;
SVECTOR				*rot ;
{
	float			sx, cx, sy, cy, sz, cz ;
	float			rx, ry, rz ;

	sy = -mat->m[ 0 ][ 2 ] ;
	cy = bp_sqrtf( 1.0F * 1.0F - sy * sy ) ;  //BP_MATH - emulate PS2 sqrtf
	if ( cy < 0.00001F ) {
		ry = ( sy > 0.0F ) ? M_PI / 2.0F : -M_PI / 2.0F ;
		rx = atan2f( - mat->m[ 2 ][ 1 ], mat->m[ 1 ][ 1 ] ) ;
		rz = 0.0F ;
	} else {
		ry = atan2f( sy, cy ) ;
		sx = mat->m[ 1 ][ 2 ] / cy ;
		cx = mat->m[ 2 ][ 2 ] / cy ;
		rx = atan2f( sx, cx ) ;
		sz = mat->m[ 0 ][ 1 ] / cy ;
		cz = mat->m[ 0 ][ 0 ] / cy ;
		rz = atan2f( sz, cz ) ;
	}
	rot->vx = ( int )( rx * 2048.0F / M_PI ) & 4095 ;
	rot->vy = ( int )( ry * 2048.0F / M_PI ) & 4095 ;
	rot->vz = ( int )( rz * 2048.0F / M_PI ) & 4095 ;

	if ( rot->vz >= 2048 ) rot->vz -= 4096 ;
	if ( rot->vz > 1024 || rot->vz < -1024 ) {
		rot->vz = ( rot->vz + 2048 ) & 4095 ;
		rot->vy = ( rot->vy + 2048 ) & 4095 ;
		rot->vx = ( 2048 - rot->vx ) & 4095 ;
	}
}
#endif

#if 0
void		DG_PersVector( from, to, n )
SVECTOR		*from ;
DVECTOR		*to ;
int		n ;
{
	while ( -- n >= 0 ) {
		gte_ldv0( from ) ;
		gte_rtps() ;
		gte_stsxy2( to ) ;
		from ++ ;
		to ++ ;
	}
}
#endif

/*----------------------------------------------------------------*/

/*
	あるポイント列が画面上に見えているか見えていないかの計算
*/

#if 0
static inline void DG_PointCheckStart( void )
{
}

static inline void DG_PointCheckEnd( void )
{
}

static inline void check_point( DVECTOR *dvp, long64 *szp, SVECTOR *vec, long n_vec )
{
}

void DG_PointCheck( FVECTOR *vec, long64 n_vec, int flag )
{
	/*
		未実装
	*/
}
#endif

int DG_PointCheckOne( FVECTOR *vec, int flag )
{
	/*
		仮ルーチン
	*/
	FVECTOR	res ;
	int		clip_flag = 0, mask = 1, i ;
	float	vw ;

	if ( flag == 0 ) flag = 1 ;

	for ( i = 0 ; i < 4 ; i++ ){
		if ( DG_Chanls[i].flag && ( mask & flag ) ){
			_SetMatrix( &DG_Chanls[i].eye_pers );
			_RotTrans( &res, vec );
			vw = ( res.vw >= 0.0f ) ? res.vw : -res.vw ;
			if ( res.vx > vw || res.vx < -vw || res.vy > vw || res.vy < -vw || res.vz > vw || res.vz < -vw ){
				clip_flag |= mask ;
			}
		} else {
			clip_flag |= mask ;
		}
		mask <<= 1 ;
	}

	return ( clip_flag );
}

/* 指定したカメラの視界内に入っているかをチェック（カメラフラグ付き） */
int DG_BoundCheckFlag( FMATRIX *world, FVECTOR *bound_max, FVECTOR *bound_min, int flag )
{
#if 0 //BP_ASM
	int		clip_flag = 0, clip_flag1, clip_flag2, mask = 1, i, check ;
	//FMATRIX	tmp_mat ;
	//FVECTOR	debug_vec[16] ;

	clip_flag1 = clip_flag2 = 0 ;

	/* バウンディングを８頂点に分解する */
	asm("
		lqc2			vf08,0x00(%0)
		lqc2			vf15,0x00(%1)
		vmove.yz		vf09,vf08
		vmove.xz		vf10,vf08
		vmove.z			vf11,vf08
		vmove.xy		vf12,vf08
		vmove.y			vf13,vf08
		vmove.x			vf14,vf08
		vmove.x			vf09,vf15
		vmove.y			vf10,vf15
		vmove.xy		vf11,vf15
		vmove.z			vf12,vf15
		vmove.xz		vf13,vf15
		vmove.yz		vf14,vf15
	"::"r"(bound_max),"r"(bound_min));


	for ( i = 0 ; i < 4 ; i++ ){
		if ( DG_Chanls[i].flag && ( mask & flag ) ){
			/* カメラビューマトリクスと頂点のローカルマトリクスを合成する */
#if 0
			/* この方法だとレジスタ内の８頂点座標を破壊してしまうので使用出来ない */
			_sceVu0MulMatrix( &tmp_mat, &DG_Chanls[i].eye_pers, world );
			_SetMatrix( world );
#else
			asm ("
				lqc2			vf28,0x00(%0)
				lqc2			vf29,0x10(%0)
				lqc2			vf30,0x20(%0)
				lqc2			vf31,0x30(%0)
				lqc2			vf04,0x00(%1)
				lqc2			vf05,0x10(%1)
				lqc2			vf06,0x20(%1)
				lqc2			vf07,0x30(%1)
				vmulax.xyzw		ACC ,vf28,vf04
				vmadday.xyzw	ACC ,vf29,vf04
				vmaddaz.xyzw	ACC ,vf30,vf04
				vmaddw.xyzw		vf04,vf31,vf04
				vmulax.xyzw		ACC ,vf28,vf05
				vmadday.xyzw	ACC ,vf29,vf05
				vmaddaz.xyzw	ACC ,vf30,vf05
				vmaddw.xyzw		vf05,vf31,vf05
				vmulax.xyzw		ACC ,vf28,vf06
				vmadday.xyzw	ACC ,vf29,vf06
				vmaddaz.xyzw	ACC ,vf30,vf06
				vmaddw.xyzw		vf06,vf31,vf06
				vmulax.xyzw		ACC ,vf28,vf07
				vmadday.xyzw	ACC ,vf29,vf07
				vmaddaz.xyzw	ACC ,vf30,vf07
				vmaddw.xyzw		vf07,vf31,vf07
			"::"r"(&DG_Chanls[i].eye_pers),"r"(world) );
#endif
			/* ８頂点分のクリップ計算を行なう */
			asm ("
				vmulax.xyzw		ACC, vf04,vf08
				vmadday.xyzw	ACC, vf05,vf08
				vmaddaz.xyzw	ACC, vf06,vf08
				vmaddw.xyzw		vf16,vf07,vf00
				vmulax.xyzw		ACC, vf04,vf09
				vmadday.xyzw	ACC, vf05,vf09
				vmaddaz.xyzw	ACC, vf06,vf09
				vmaddw.xyzw		vf17,vf07,vf00
				vmulax.xyzw		ACC, vf04,vf10
				vmadday.xyzw	ACC, vf05,vf10
				vmaddaz.xyzw	ACC, vf06,vf10
				vmaddw.xyzw		vf18,vf07,vf00
				vmulax.xyzw		ACC, vf04,vf11
				vmadday.xyzw	ACC, vf05,vf11
				vmaddaz.xyzw	ACC, vf06,vf11
				vmaddw.xyzw		vf19,vf07,vf00
				vmulax.xyzw		ACC, vf04,vf12
				vmadday.xyzw	ACC, vf05,vf12
				vmaddaz.xyzw	ACC, vf06,vf12
				vmaddw.xyzw		vf20,vf07,vf00
				vmulax.xyzw		ACC, vf04,vf13
				vmadday.xyzw	ACC, vf05,vf13
				vmaddaz.xyzw	ACC, vf06,vf13
				vmaddw.xyzw		vf21,vf07,vf00
				vmulax.xyzw		ACC, vf04,vf14
				vmadday.xyzw	ACC, vf05,vf14
				vmaddaz.xyzw	ACC, vf06,vf14
				vmaddw.xyzw		vf22,vf07,vf00
				vclipw.xyz		vf16,vf16			# vf08 data
				vclipw.xyz		vf17,vf17			# vf09 data
				vclipw.xyz		vf18,vf18			# vf10 data
				vclipw.xyz		vf19,vf19			# vf11 data
				vmulax.xyzw		ACC, vf04,vf15
				vmadday.xyzw	ACC, vf05,vf15
				vmaddaz.xyzw	ACC, vf06,vf15
				vmaddw.xyzw		vf23,vf07,vf00
				cfc2			%0,$18				# clipping
				vclipw.xyz		vf20,vf20			# vf12 data
				vclipw.xyz		vf21,vf21			# vf13 data
				vclipw.xyz		vf22,vf22			# vf14 data
				vclipw.xyz		vf23,vf23			# vf15 data
				vnop
				vnop
				vnop
				vnop
				cfc2			%1,$18				# clipping
			":"=r"(clip_flag1),"=r"(clip_flag2) );
			check = 0x3f ;
			check &= clip_flag1 ; clip_flag1 >>= 6 ;
			check &= clip_flag1 ; clip_flag1 >>= 6 ;
			check &= clip_flag1 ; clip_flag1 >>= 6 ;
			check &= clip_flag1 ;
			check &= clip_flag2 ; clip_flag2 >>= 6 ;
			check &= clip_flag2 ; clip_flag2 >>= 6 ;
			check &= clip_flag2 ; clip_flag2 >>= 6 ;
			check &= clip_flag2 ;
			if ( check != 0 ){
				clip_flag |= mask ;
			}
		} else {
			clip_flag |= mask ;
		}
		mask <<= 1 ;
	}

	if ( clip_flag == 0xf ) clip_flag = -1 ;

	return ( clip_flag );
#else
   //BP - Copied from xpos.c version of this function.
   int		clip_flag = 0, mask = 1, i, j, check ;
   FVECTOR		vec[8], *verts, v ;
   FMATRIX		tmp_mat ;

   /* バウンディングを８頂点に分解する */
   for ( i = 7; i >= 0 ; i-- ){
      vec[i].vx = (i & 1) ? bound_max->vx : bound_min->vx;
      vec[i].vy = (i & 2) ? bound_max->vy : bound_min->vy;
      vec[i].vz = (i & 4) ? bound_max->vz : bound_min->vz;
      vec[i].vw = 1.0f;
   }

   for ( i = 0 ; i < 4 ; i++ ){
      if ( DG_Chanls[i].flag && ( mask & flag ) ){
         /* カメラビューマトリクスと頂点のローカルマトリクスを合成する */
         _sceVu0MulMatrix( &tmp_mat, &DG_Chanls[i].eye_pers, world );
         _SetMatrix( &tmp_mat );
         /* ８頂点分のクリップ計算を行なう */
         verts = vec ;
         check = 0x3f ;
         for ( j = 0; j < 8; j++, verts++ ){
            int		clip ;
            float	w ;
            _RotTrans( &v, &vec[j] );
            w = DG_FABS( v.vw );
            clip = 0 ;
            if ( v.vx >   w) clip |= CLIP_X0_FLAG;
            if ( v.vx < - w) clip |= CLIP_X1_FLAG;
            if ( v.vy >   w) clip |= CLIP_Y0_FLAG;
            if ( v.vy < - w) clip |= CLIP_Y1_FLAG;
            if ( v.vz >   w) clip |= CLIP_Z0_FLAG;
            if ( v.vz < - w) clip |= CLIP_Z1_FLAG;
            check &= clip ;
         }
         if ( check != 0 ){
            clip_flag |= mask ;
         }
      } else {
         clip_flag |= mask ;
      }
      mask <<= 1 ;
   }

   if ( clip_flag == 0xf ) clip_flag = -1 ;

   return ( clip_flag );
#endif
}

/* 指定したカメラの視界内に入っているかをチェック */
int DG_BoundCheck( FMATRIX *world, FVECTOR *bound_max, FVECTOR *bound_min )
{
	return DG_BoundCheckFlag( world, bound_max, bound_min, 0x0f );
}

int	DG_ObjsBoundCheckChanl( DG_OBJS	*objs, int chanl )
{
	FVECTOR		max, min ;

	max.vx = objs->def->ux ;
	max.vy = objs->def->uy ;
	max.vz = objs->def->uz ;
	min.vx = objs->def->lx ;
	min.vy = objs->def->ly ;
	min.vz = objs->def->lz ;
	return ( DG_BoundCheckFlag( &objs->world, &max, &min, 0x0f ) & ( 1 << chanl ) ) ;
}

int	DG_ObjBoundCheckChanl( DG_OBJS *objs, int joint, int chanl )
{
	FVECTOR		max, min ;

	max.vx = objs->def->models[ joint ].ux ;
	max.vy = objs->def->models[ joint ].uy ;
	max.vz = objs->def->models[ joint ].uz ;
	min.vx = objs->def->models[ joint ].lx ;
	min.vy = objs->def->models[ joint ].ly ;
	min.vz = objs->def->models[ joint ].lz ;
	return ( DG_BoundCheckFlag( &objs->objs[ joint ].world, &max, &min, 0x0f ) & ( 1 << chanl ) ) ;
}

/* あるポイントが画面のどの位置にくるかを返すのを
   つくってみた */
void	DG_TransPersOneChanl( FVECTOR *res, FVECTOR *pos, int chanl )
{
	FMATRIX		*eye_pers ;

	ASSERT( chanl >= 0 && chanl < DG_MAX_CHANLS ) ;
	eye_pers = &DG_Chanl( chanl )->eye_pers ;
	pos->vw = 1.0F ;
	_sceVu0ApplyMatrix( res, eye_pers, pos ) ;
}

void	DG_TransPersOne( FVECTOR *res, FVECTOR *pos )
{
	float		w ;

	DG_TransPersOneChanl( res, pos, 0 ) ;
	w = DG_FABS( res->vw ) ;
	res->vx /= w ;
	res->vy /= w ;
	res->vz /= w ;
	res->vx = res->vx * ( float )( DRAW_WIDTH / 2 ) + ( float )( DRAW_WIDTH / 2 ) ;
	res->vy = res->vy * ( float )( DRAW_HEIGHT / 2 ) + ( float )( DRAW_HEIGHT / 2 ) ;
}

//BP - copied from xpos.c
void		DG_RotVectorW( FVECTOR *from, FVECTOR *to, int n )
{/* user/shibataの下で是角さんが読んでいる。名前と機能が違うが、いずれなくす予定 */
   while ( -- n >= 0 ) {
      _sceVu0ApplyMatrix( to, &WorkMatrix, from );
      from ++ ;
      to ++ ;
   }
}

