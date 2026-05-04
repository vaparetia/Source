/*
	pos.c
	三次元座標計算ユーティリティ

	1999/07/07 K.Takabe
	$Id: wpos.c,v 1.4 2002/12/24 07:14:03 takaki Exp $

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

	void DG_PointCheck( SVECTOR *vec, long n_vec );

		現在のマトリクスをセーブしてから、カメラ行列をセットし、
		ポイントチェックの開始を宣言。スクラッチパッドの内容を破壊
		vecのポイント列が画面上に見えているかどうかを計算する。
		結果はvec->padに 0 or 1で入る。

*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef _XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"sse_inline.h"

#define _CopyVector(a, b) { *(VECTOR *)(a) = *(VECTOR *)(b); }
#define _CopyMatrix(a, b) { *(MATRIX *)(a) = *(MATRIX *)(b); }


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
static FMATRIX	WorkMatrix ;

/* ＰＳ式角度を-2048~2047に正規化する */
#define REGULAR_ANGLE( _a )	( (signed short)( ((unsigned short)(_a)) << 4 ) >> 4 )


/*----------------------------------------------------------------*/
/* この関数はきちんと動いてないっぽい・・・ */
#if FALSE
static inline void _ScaleMatrix( FVECTOR *v )
{
	IVECTOR		mask[2] = {{0,0,0,-1},{-1,-1,-1,0}} ;
	__asm {
        mov		ecx, v
        mov		eax, mask
		movups	xmm0, [ecx+00h]
		movups	xmm1, [eax+00h]
		movups	xmm2, [eax+10h]

		movaps	xmm3, xmm0
		shufps	xmm3, xmm3, 00000000b
		mulps	xmm3, xmm4
		andps	xmm3, xmm2
		andps	xmm4, xmm1
		addps	xmm4, xmm3

		movaps	xmm3, xmm0
		shufps	xmm3, xmm3, 01010101b
		mulps	xmm3, xmm5
		andps	xmm3, xmm2
		andps	xmm5, xmm1
		addps	xmm5, xmm3

		movaps	xmm3, xmm0
		shufps	xmm3, xmm3, 10101010b
		mulps	xmm3, xmm6
		andps	xmm3, xmm2
		andps	xmm6, xmm1
		addps	xmm6, xmm3

		movaps	xmm3, xmm0
		shufps	xmm3, xmm3, 11111111b
		mulps	xmm3, xmm7
		andps	xmm3, xmm2
		andps	xmm7, xmm1
		addps	xmm7, xmm3
	}
}
#endif

void DG_SinCos( float *s, float *c, float rot )
{
	__asm {
		fld		rot
		mov		ecx, dword ptr[c]
		mov		eax, dword ptr[s]
		fsincos
		fstp	dword ptr[ecx]
		fstp	dword ptr[eax]
	}
}

void DG_RotMatrixX( FMATRIX *res, FMATRIX *mat, float rot_x )
{
	FMATRIX		rot_mat ;
	float	s, c ;
	//s = sin( rot_x );
	//c = cos( rot_x );
	DG_SinCos( &s, &c, rot_x );
	rot_mat = DG_UnitMatrix ;
	rot_mat.m[1][1] =  c ; rot_mat.m[2][1] = -s ;
	rot_mat.m[1][2] =  s ; rot_mat.m[2][2] =  c ;
	//_sceVu0MulMatrix( res, &rot_mat, mat );
	MulMatrix( res, &rot_mat, mat );
}

void DG_RotMatrixY( FMATRIX *res, FMATRIX *mat, float rot_y )
{
	FMATRIX		rot_mat ;
	float	s, c ;
	//s = sin( rot_y );
	//c = cos( rot_y );
	DG_SinCos( &s, &c, rot_y );
	rot_mat = DG_UnitMatrix ;
	rot_mat.m[2][2] =  c ; rot_mat.m[0][2] = -s ;
	rot_mat.m[2][0] =  s ; rot_mat.m[0][0] =  c ;
	//_sceVu0MulMatrix( res, &rot_mat, mat );
	MulMatrix( res, &rot_mat, mat );
}

void DG_RotMatrixZ( FMATRIX *res, FMATRIX *mat, float rot_z )
{
	FMATRIX		rot_mat ;
	float	s, c ;
	//s = sin( rot_z );
	//c = cos( rot_z );
	DG_SinCos( &s, &c, rot_z );
	rot_mat = DG_UnitMatrix ;
	rot_mat.m[0][0] =  c ; rot_mat.m[1][0] = -s ;
	rot_mat.m[0][1] =  s ; rot_mat.m[1][1] =  c ;
	//_sceVu0MulMatrix( res, &rot_mat, mat );
	MulMatrix( res, &rot_mat, mat );
}

/* 逆行列算出（正規化回転マトリクスのみ対応） */
void DG_InversMatrix( FMATRIX *res, FMATRIX *mat )
{
	FMATRIX		m ;
	FVECTOR		v ;
	m.m[0][0] = mat->m[0][0] ;
	m.m[1][1] = mat->m[1][1] ;
	m.m[2][2] = mat->m[2][2] ;
	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f ;
	m.m[0][1] = mat->m[1][0] ;
	m.m[0][2] = mat->m[2][0] ;
	m.m[1][0] = mat->m[0][1] ;
	m.m[1][2] = mat->m[2][1] ;
	m.m[2][0] = mat->m[0][2] ;
	m.m[2][1] = mat->m[1][2] ;
	v.vx = - mat->m[3][0] ;
	v.vy = - mat->m[3][1] ;
	v.vz = - mat->m[3][2] ;
	v.vw = 1.0f ;
	*res = m ;
	_SetMatrix( &m );
	_RotVector( (FVECTOR *)&res->m[3][0], &v );	/* PS2と違いVWは保存されないので注意！ */
	res->m[3][3] = 1.0f ;
}


/*----------------------------------------------------------------*/
void		DG_GetPos( FMATRIX *world )
{
	DG_COPY_MAT( world, &WorkMatrix );
}

void		DG_SetPos( FMATRIX *world )
{
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
#if 0
	_sceVu0RotMatrixZ( &WorkMatrix, &WorkMatrix, rot_z );
	_sceVu0RotMatrixX( &WorkMatrix, &WorkMatrix, rot_x );
	_sceVu0RotMatrixY( &WorkMatrix, &WorkMatrix, rot_y );
#else
	DG_RotMatrixZ( &WorkMatrix, &WorkMatrix, rot_z );
	DG_RotMatrixX( &WorkMatrix, &WorkMatrix, rot_x );
	DG_RotMatrixY( &WorkMatrix, &WorkMatrix, rot_y );
#endif

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
	//int			tmp ;


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
	_sceVu0UnitMatrix( &tmp_mat );
#if 0
	_sceVu0RotMatrixZ( &tmp_mat, &tmp_mat, rot_z );
	_sceVu0RotMatrixX( &tmp_mat, &tmp_mat, rot_x );
	_sceVu0RotMatrixY( &tmp_mat, &tmp_mat, rot_y );
#else
	DG_RotMatrixZ( &tmp_mat, &tmp_mat, rot_z );
	DG_RotMatrixX( &tmp_mat, &tmp_mat, rot_x );
	DG_RotMatrixY( &tmp_mat, &tmp_mat, rot_y );
#endif
	_sceVu0MulMatrix( &WorkMatrix, &WorkMatrix, &tmp_mat );
}

void		DG_RotatePosZYX( SVECTOR *rot )
{
	FMATRIX		tmp_mat ;
	float		rot_x, rot_y, rot_z ;
	//int			tmp ;


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
	_sceVu0UnitMatrix( &tmp_mat );
#if 0
	_sceVu0RotMatrixX( &tmp_mat, &tmp_mat, rot_x );
	_sceVu0RotMatrixY( &tmp_mat, &tmp_mat, rot_y );
	_sceVu0RotMatrixZ( &tmp_mat, &tmp_mat, rot_z );
#else
	DG_RotMatrixX( &tmp_mat, &tmp_mat, rot_x );
	DG_RotMatrixY( &tmp_mat, &tmp_mat, rot_y );
	DG_RotMatrixZ( &tmp_mat, &tmp_mat, rot_z );
#endif
	_sceVu0MulMatrix( &WorkMatrix, &WorkMatrix, &tmp_mat );
}

void		DG_ScalePos( FVECTOR *scale )
{
#if 0
	_SetMatrix( &WorkMatrix );
	_ScaleMatrix( scale );			/* 現在バグってます */
	_GetMatrix( &WorkMatrix );
#else
	WorkMatrix.m[0][0] *= scale->vx ;
	WorkMatrix.m[0][1] *= scale->vx ;
	WorkMatrix.m[0][2] *= scale->vx ;
	WorkMatrix.m[1][0] *= scale->vy ;
	WorkMatrix.m[1][1] *= scale->vy ;
	WorkMatrix.m[1][2] *= scale->vy ;
	WorkMatrix.m[2][0] *= scale->vz ;
	WorkMatrix.m[2][1] *= scale->vz ;
	WorkMatrix.m[2][2] *= scale->vz ;
	WorkMatrix.m[3][0] *= scale->vw ;
	WorkMatrix.m[3][1] *= scale->vw ;
	WorkMatrix.m[3][2] *= scale->vw ;
#endif
}


/*----------------------------------------------------------------*/

void		DG_PutObjs( DG_OBJS *objs )
{
	// objs->world = WorkMatrix ;
	DG_COPY_MAT( &objs->world, &WorkMatrix );
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
	//float	vw ;
	_SetMatrix( &WorkMatrix );
	while ( -- n >= 0 ) {
		{
			//vw = from->vw ;
			_RotVector( to, from );	/* PS2と違いVWは保存されないので注意！ */
			//to->vw = vw ;
		}
		from ++ ;
		to ++ ;
	}
}

void		DG_PutVectorW( FVECTOR *from, FVECTOR *to, int n )
{/* user/shibataの下で是角さんが読んでいる。名前と機能が違うが、いずれなくす予定 */
	while ( -- n >= 0 ) {
		_sceVu0ApplyMatrix( to, &WorkMatrix, from );
		from ++ ;
		to ++ ;
	}
}
void		DG_RotVectorW( FVECTOR *from, FVECTOR *to, int n )
{/* user/shibataの下で是角さんが読んでいる。名前と機能が違うが、いずれなくす予定 */
	while ( -- n >= 0 ) {
		_sceVu0ApplyMatrix( to, &WorkMatrix, from );
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



/*----------------------------------------------------------------*/

/*
	あるポイント列が画面上に見えているか見えていないかの計算
*/

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
