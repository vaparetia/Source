/*
	pos.c
	三次元座標計算ユーティリティ

	1999/07/07 K.Takabe
	$Id: pos.cpp,v 1.1.1.3 2002/11/19 11:42:20 Yoshizawa1 Exp $

*/
/*

	void		DG_GetPos( MATRIX *world )
	MATRIX		*world ;	取得マトリクス

		現在位置を取得する

	void		DG_SetPos( world )
	MATRIX		*world ;	ワールドマトリクス

		現在位置を、マトリクス代入で初期化する
	
	void		DG_SetPos2( mov, rot )
	VECTOR		*mov ;		位置
	SVECTOR		*rot ;		方向

		現在位置を、位置と方向で初期化する
	
	void		DG_MovePos( mov )
	VECTOR		*mov ;		相対移動量

		現在位置を、相対移動させる
	
	void		DG_RotatePos( rot )
	SVECTOR		*rot ;		相対回転量

		現在位置を、相対回転させる
	
	void		DG_RotatePosYZX( rot )
	SVECTOR		*rot ;		相対回転量

		現在位置を、相対回転させる（Ｘ、Ｙ、Ｚの順＝SoftImage準拠）

	void		DG_ScalePos( VECTOR *scale )
	VECTOR		*scale ;	現在位置にスケールをかける（x:Ｘ成分 y:Ｙ成分 z:Ｚ成分 w:中心座標）

		現在位置にスケールをかける
	
	void		DG_PutObjs( objs )
	DG_OBJS		*objs ;		物体ハンドラ

		物体ハンドラを、現在位置に配置する

	void		DG_PutPrim( prim )
	DG_PRIM		*prim ;		プリミティブ

		プリミティブを、現在位置に配置する

	void		DG_PutVector( from, to, n )
	VECTOR		*from ;		相対位置ベクトル（入力）
	VECTOR		*to ;		絶対位置ベクトル（出力）
	int		n ;		ベクトル数

		相対位置ベクトルを、現在位置に配置する

	void		DG_RotVector( from, to, n )
	VECTOR		*from ;		相対方向ベクトル（入力）
	VECTOR		*to ;		絶対方向ベクトル（出力）
	int		n ;		ベクトル数

		相対方向ベクトルを、現在位置に配置する

	int DG_PointCheckOne( VECTOR *vec, int flag )
	VECTOR		*vec ;		チェック座標
	int			flag ;		チェックフラグ（チャンネルの指定）

		指定したベクトルが画面内に入っているかどうかをチェック
		その点が画面内に入っていたら０以外が返る

	int DG_BoundCheck( MATRIX *world, VECTOR *bound_max, VECTOR *bound_min )
	MATRIX		*world ;		バウンディングのローカルマトリクス
	VECTOR		*bound_max ;	バウンディングを表す最大座標
	VECTOR		*bound_min ;	バウンディングを表す最小座標

		指定したカメラの視界内に入っているかをチェック
		カメラ外であれば対応したビットが１になるが（カメラの最大は４つまで）
		全てのカメラから見えなければ－１が返る
		（０なら全てのカメラから見える）

	void DG_ReflectVector(VECTOR *pole, VECTOR *vec1, VECTOR *vec2);
	VECTOR 		*pole ;			入力（保存）:法線（非正規ＯＫ）
	VECTOR 		*vec1 ;			入力（保存）:入射ベクトル      
	VECTOR		*vec2 ;			出力（破壊）:反射ベクトル      

		反射ベクトルを求める

		
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

#ifndef KP_XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"

//-----------------------------------------------------------------------------

#define _CopyVector(a, b) { *(VECTOR *)(a) = *(VECTOR *)(b); }
#define _CopyMatrix(a, b) { *(MATRIX *)(a) = *(MATRIX *)(b); }

SVECTOR DG_ZeroSVector = {0, 0, 0, 0};
IVECTOR DG_ZeroIVector = {0, 0, 0, 0};
VECTOR  DG_ZeroVector = {0.0f, 0.0f, 0.0f, 1.0f};
MATRIX  DG_UnitMatrix = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};
MATRIX  DG_ZeroMatrix = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
};

// ＰＳの定数回転マトリクスに相当
static MATRIX WorkMatrix;

static inline void ApplyMatrix( FVECTOR *res, FMATRIX *m0, FVECTOR *v0 )
{
	__asm {
        mov		edx, m0
        mov		ecx, v0
        mov		eax, res
		movups	xmm0, [ecx+00h]
		movups	xmm4, [edx+00h]
		movups	xmm5, [edx+10h]
		movups	xmm6, [edx+20h]
		movups	xmm7, [edx+30h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		movaps	xmm3, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		shufps	xmm3, xmm3, 11111111b
		mulps	xmm3, xmm7
		addps	xmm1, xmm0
		addps	xmm3, xmm2
		addps	xmm3, xmm1
		movups	[eax+00h], xmm3 
	}
}

/* v0->vw = 1.0f と見なすバージョン */
static inline void ApplyMatrix2( FVECTOR *res, FMATRIX *m0, FVECTOR *v0 )
{
	__asm {
        mov		edx, m0
        mov		ecx, v0
        mov		eax, res
		movaps	xmm0, [ecx+00h]
		movaps	xmm4, [edx+00h]
		movaps	xmm5, [edx+10h]
		movaps	xmm6, [edx+20h]
		movaps	xmm7, [edx+30h]
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
		movaps	[eax+00h], xmm3 
	}
}

static inline void MulMatrix( FMATRIX *res, FMATRIX *m0, FMATRIX *m1 )
{
	__asm {
        mov		edx, m0
		movups	xmm4, [edx+00h]
		movups	xmm5, [edx+10h]
		movups	xmm6, [edx+20h]
		movups	xmm7, [edx+30h]

        mov		edx, m1
        mov		ecx, res

		movups	xmm0, [edx+00h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		movaps	xmm3, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		shufps	xmm3, xmm3, 11111111b
		mulps	xmm3, xmm7
		addps	xmm1, xmm0
		addps	xmm3, xmm2
		addps	xmm3, xmm1
		movups	[ecx+00h],xmm3 

		movups	xmm0, [edx+10h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		movaps	xmm3, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		shufps	xmm3, xmm3, 11111111b
		mulps	xmm3, xmm7
		addps	xmm1, xmm0
		addps	xmm3, xmm2
		addps	xmm3, xmm1
		movups	[ecx+10h],xmm3 

		movups	xmm0, [edx+20h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		movaps	xmm3, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		shufps	xmm3, xmm3, 11111111b
		mulps	xmm3, xmm7
		addps	xmm1, xmm0
		addps	xmm3, xmm2
		addps	xmm3, xmm1
		movups	[ecx+20h],xmm3 

		movaps	xmm3, [edx+30h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		movaps	xmm3, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		shufps	xmm3, xmm3, 11111111b
		mulps	xmm3, xmm7
		addps	xmm1, xmm0
		addps	xmm3, xmm2
		addps	xmm3, xmm1
		movups	[ecx+30h],xmm3 


    }
}

//-----------------------------------------------------------------------------

static inline void _SetMatrix(MATRIX *m)
{
	WorkMatrix = *m;
}

static inline void _GetMatrix(MATRIX *m)
{
	*m = WorkMatrix;
}

static inline void _RotTrans(VECTOR *res, VECTOR *v)
{
	VECTOR vecTmp = *v;
	vecTmp.vw = 1.0f;
	D3DXVec4Transform(res, &vecTmp, &WorkMatrix);
}

static inline void _RotVector(VECTOR *res, VECTOR *v)
{
//	D3DXVec3Transform(res, v, &WorkMatrix);
	D3DXVec3TransformNormal(res, v, &WorkMatrix);
}

static inline void _RotVectorW(VECTOR *res, VECTOR *v)
{
	D3DXVec3Transform(res, v, &WorkMatrix);
}

static inline void _ScaleMatrix(VECTOR *v)
{
	WorkMatrix.m[0][0] *= v->vx;
	WorkMatrix.m[0][1] *= v->vx;
	WorkMatrix.m[0][2] *= v->vx;

	WorkMatrix.m[1][0] *= v->vy;
	WorkMatrix.m[1][1] *= v->vy;
	WorkMatrix.m[1][2] *= v->vy;

	WorkMatrix.m[2][0] *= v->vz;
	WorkMatrix.m[2][1] *= v->vz;
	WorkMatrix.m[2][2] *= v->vz;

	WorkMatrix.m[3][0] *= v->vw;
	WorkMatrix.m[3][1] *= v->vw;
	WorkMatrix.m[3][2] *= v->vw;
}

//-----------------------------------------------------------------------------

void DG_GetPos(MATRIX *world)
{
	DG_COPY_MAT(world, &WorkMatrix);
}

void DG_SetPos(MATRIX *world)
{
	DG_COPY_MAT(&WorkMatrix, world);
}

void DG_SetPos2(VECTOR *mov, SVECTOR *rot)
{
	MATRIX matTmp;
	float rot_x, rot_y, rot_z;
	int   tmp;

	tmp = rot->vz; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_z = (float)tmp * (float)M_PI / 2048.0f;

	tmp = rot->vx; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_x = (float)tmp * (float)M_PI / 2048.0f;

	tmp = rot->vy; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_y = (float)tmp * (float)M_PI / 2048.0f;

	// WorkMatrix = RotY * RotX * RotZ
	D3DXMatrixRotationY(&WorkMatrix, rot_y); // Y回転
	D3DXMatrixRotationX(&matTmp, rot_x);     // X回転
////	D3DXMatrixMultiply(&WorkMatrix, &WorkMatrix, &matTmp);
	D3DXMatrixMultiply(&WorkMatrix, &matTmp, &WorkMatrix);
	D3DXMatrixRotationZ(&matTmp, rot_z);     // Z回転
////	D3DXMatrixMultiply(&WorkMatrix, &WorkMatrix, &matTmp);
	D3DXMatrixMultiply(&WorkMatrix, &matTmp, &WorkMatrix);

	WorkMatrix.m[3][0] = mov->vx;
	WorkMatrix.m[3][1] = mov->vy;
	WorkMatrix.m[3][2] = mov->vz;
}

void DG_MovePos(VECTOR *mov)
{
	VECTOR tmp_vec;

////	_SetMatrix(&WorkMatrix);
	_RotTrans(&tmp_vec, mov);
	WorkMatrix.m[3][0] = tmp_vec.vx;
	WorkMatrix.m[3][1] = tmp_vec.vy;
	WorkMatrix.m[3][2] = tmp_vec.vz;
}

void DG_RotatePos(SVECTOR *rot)
{
	MATRIX matTmp, matRes;
	float   rot_x, rot_y, rot_z;
	int     tmp;

	tmp = rot->vz; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_z = (float)tmp * (float)M_PI / 2048.0f;
	tmp = rot->vx; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_x = (float)tmp * (float)M_PI / 2048.0f;
	tmp = rot->vy; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_y = (float)tmp * (float)M_PI / 2048.0f;

	D3DXMatrixRotationY(&matRes, rot_y); // Y回転
	D3DXMatrixRotationX(&matTmp, rot_x); // X回転
	D3DXMatrixMultiply(&matRes, &matRes, &matTmp);
	D3DXMatrixRotationZ(&matTmp, rot_z); // Z回転
	D3DXMatrixMultiply(&matRes, &matRes, &matTmp);
	D3DXMatrixMultiply(&WorkMatrix, &matRes, &WorkMatrix);
}

void DG_RotatePosZYX(SVECTOR *rot)
{
	MATRIX matTmp, matRes;
	float   rot_x, rot_y, rot_z;
	int     tmp;

	tmp = rot->vz; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_z = (float)tmp * (float)M_PI / 2048.0f;
	tmp = rot->vx; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_x = (float)tmp * (float)M_PI / 2048.0f;
	tmp = rot->vy; tmp = (tmp & 0x0800) ? (tmp | 0xfffff000) : (tmp & 0xfff);
	rot_y = (float)tmp * (float)M_PI / 2048.0f;

	D3DXMatrixRotationZ(&matRes, rot_z); // まずZ回転
	D3DXMatrixRotationY(&matTmp, rot_y); // Y回転
	D3DXMatrixMultiply(&matRes, &matRes, &matTmp);
	D3DXMatrixRotationX(&matTmp, rot_x); // X回転
	D3DXMatrixMultiply(&matRes, &matRes, &matTmp);
	D3DXMatrixMultiply(&WorkMatrix, &matRes, &WorkMatrix);
}

void DG_ScalePos(VECTOR *scale)
{
////	_SetMatrix(&WorkMatrix);
	_ScaleMatrix(scale);
	_GetMatrix(&WorkMatrix);
}


/*----------------------------------------------------------------*/

void DG_PutObjs(DG_OBJS *objs)
{
	DG_COPY_MAT(&objs->world, &WorkMatrix);
}

#if 0
void DG_PutPrim(DG_PRIM *prim)
{
	DG_COPY_MAT(&prim->world, &WorkMatrix);
}
#endif

void DG_PutVector(VECTOR *from, VECTOR *to, int n)
{
	//..
////	_SetMatrix(&WorkMatrix);
	while (-- n >= 0) {
		_RotTrans(to, from);
		//ApplyMatrix2( (FVECTOR*)to, (FMATRIX*)&WorkMatrix, (FVECTOR*)from );
		from ++;
		to ++;
	}
}

void DG_RotVector(VECTOR *from, VECTOR *to, int n)
{
////	_SetMatrix(&WorkMatrix);
	while (-- n >= 0) {
		_RotVector(to, from);
		from ++;
		to ++;
	}
}

void DG_RotVectorW(VECTOR *from, VECTOR *to, int n)
{
	while (-- n >= 0) {
		_RotVectorW(to, from);
		from ++;
		to ++;
	}
}

void DG_ReflectMatrix(VECTOR *pole, MATRIX *m1, MATRIX *m2)
{
    MATRIX tmp_mat;	
    VECTOR ip_vec;
    float   ip;

	D3DXMatrixTranspose(&tmp_mat, m1);
    DG_COPY_MAT(&WorkMatrix, &tmp_mat);
////    _SetMatrix(&WorkMatrix);
    _RotVector(&ip_vec, pole);
    ip = ip_vec.vx;
    m2->m[0][0] = m1->m[0][0] - pole->vx * ip * 2.0f;
    m2->m[0][1] = m1->m[0][1] - pole->vy * ip * 2.0f;
    m2->m[0][2] = m1->m[0][2] - pole->vz * ip * 2.0f;
    ip = ip_vec.vy;
    m2->m[1][0] = m1->m[1][0] - pole->vx * ip * 2.0f;
    m2->m[1][1] = m1->m[1][1] - pole->vy * ip * 2.0f;
    m2->m[1][2] = m1->m[1][2] - pole->vz * ip * 2.0f;
    ip = ip_vec.vz;
    m2->m[2][0] = m1->m[2][0] - pole->vx * ip * 2.0f;
    m2->m[2][1] = m1->m[2][1] - pole->vy * ip * 2.0f;
    m2->m[2][2] = m1->m[2][2] - pole->vz * ip * 2.0f;
}

void DG_ReflectVector(VECTOR *pole, VECTOR *vec1, VECTOR *vec2)
{
#if 1
	float	ftemp;
	FVECTOR	fvtemp;
	DG_COPY_VEC( &fvtemp, pole );
	_sceVu0Normalize( &fvtemp, &fvtemp );
	ftemp = _sceVu0InnerProduct( &fvtemp, vec1 );
	_sceVu0ScaleVectorXYZ( &fvtemp, &fvtemp, -ftemp*2.0f );
	_sceVu0AddVector( vec2, &fvtemp, vec1 );

#else
	float	ftemp;
	VECTOR	fvtemp;
	DG_COPY_VEC(&fvtemp, pole);

	D3DXVec3Normalize(&fvtemp, &fvtemp); /*D3DXVec4Normalize->D3DXVec3Normalize に変更しました。yano*/
	ftemp = D3DXVec4Dot(&fvtemp, vec1);
	D3DXVec3Scale(&fvtemp, &fvtemp, - ftemp * 2.0f);
	D3DXVec4Add(vec2, &fvtemp, vec1);
#endif
}

/*----------------------------------------------------------------*/

int DG_PointCheckOne(VECTOR *vec, int flag)
{
	// TODO: 仮ルーチン ???
	VECTOR	res;
	int		clip_flag = 0, mask = 1, i;
	float	vw;

	if (flag == 0) flag = 1;

	for (i = 0; i < 4; i++){
		if (DG_Chanls[i].flag && (mask & flag)) {
			_SetMatrix(&DG_Chanls[i].eye_pers);
			_RotTrans(&res, vec);
			vw = (res.vw >= 0.0f) ? res.vw : -res.vw;
			if (res.vx > vw || res.vx < -vw
				|| res.vy > vw || res.vy < -vw
				|| res.vz > vw || res.vz < -vw) {
				clip_flag |= mask;
			}
		} else {
			clip_flag |= mask;
		}
		mask <<= 1;
	}

	return clip_flag;
}

// 指定したカメラの視界内に入っているかをチェック（カメラフラグ付き）
// 動作をそのままエミュレートする
int DG_BoundCheckFlag(MATRIX *world, VECTOR *bound_max, VECTOR *bound_min, int flag)
{
	float  w;
	int i, j, mask, check, clip_flag;
	MATRIX	tmp_mat;	
	VECTOR vec[8];
	VECTOR *verts;

	mask = 1;
	clip_flag = 0;	

	// bound_max, bound_min から8頂点を計算
	for (i = 7; i >= 0; i --) {
		vec[i].vx = (i & 1) ? bound_max->vx : bound_min->vx;
		vec[i].vy = (i & 2) ? bound_max->vy : bound_min->vy;
		vec[i].vz = (i & 4) ? bound_max->vz : bound_min->vz;
		vec[i].vw = 1.0f;
	}

	// チャンネルごとに処理する
	for (i = 0; i < 4; i ++) {
		if (DG_Chanls[i].flag && (mask & flag)) {
			D3DXMatrixMultiply(&tmp_mat, world, &DG_Chanls[i].eye_pers);

			// 各頂点の透視変換→クリップ計算
			verts = vec;
			check = CLIP_FLAG;
			for (j = 0; j < 8; j ++, verts ++) {
				D3DXVec4Transform(verts, verts, &tmp_mat);
				// clipw エミュレート
				w = DG_FABS(verts->vw);
				if (verts->vx >   w) clip_flag |= CLIP_X0_FLAG;
				if (verts->vx < - w) clip_flag |= CLIP_X1_FLAG;
				if (verts->vy >   w) clip_flag |= CLIP_Y0_FLAG;
				if (verts->vy < - w) clip_flag |= CLIP_Y1_FLAG;
				if (verts->vz >   w) clip_flag |= CLIP_Z0_FLAG;
				if (verts->vz < - w) clip_flag |= CLIP_Z1_FLAG;
				check &= flag;
			}
			if (check != 0) {
				clip_flag |= mask;
			}
		} else {
			clip_flag |= mask;
		}
		mask <<= 1;
	}

	if (clip_flag == 0xf) {
		clip_flag = - 1;
	}
	return clip_flag;
}


// 指定したカメラの視界内に入っているかをチェック
int DG_BoundCheck(MATRIX *world, VECTOR *bound_max, VECTOR *bound_min)
{
	return DG_BoundCheckFlag(world, bound_max, bound_min, 0x0f);
}

int	DG_ObjsBoundCheckChanl(DG_OBJS	*objs, int chanl)
{
	VECTOR max, min;

	max.vx = objs->def->ux;
	max.vy = objs->def->uy;
	max.vz = objs->def->uz;
	min.vx = objs->def->lx;
	min.vy = objs->def->ly;
	min.vz = objs->def->lz;
	return DG_BoundCheckFlag(&objs->world, &max, &min, 0x0f) & (1 << chanl);
}

int	DG_ObjBoundCheckChanl(DG_OBJS *objs, int joint, int chanl)
{
	VECTOR		max, min;

	max.vx = objs->def->models[joint].ux;
	max.vy = objs->def->models[joint].uy;
	max.vz = objs->def->models[joint].uz;
	min.vx = objs->def->models[joint].lx;
	min.vy = objs->def->models[joint].ly;
	min.vz = objs->def->models[joint].lz;
	return DG_BoundCheckFlag(&objs->objs[joint].world, &max, &min, 0x0f) & (1 << chanl);
}

// あるポイントが画面のどの位置にくるかを返す
void DG_TransPersOneChanl(VECTOR *res, VECTOR *pos, int chanl)
{
	MATRIX *eye_pers;

	ASSERT(chanl >= 0 && chanl < DG_MAX_CHANLS);
	eye_pers = &DG_Chanl(chanl)->eye_pers;
	pos->vw = 1.0f;
	D3DXVec4Transform(res, pos, eye_pers);
	//※ PS2版とはy方向が逆
	res->vy = -res->vy;
}

void DG_TransPersOne(VECTOR *res, VECTOR *pos)
{
	float w;

	DG_TransPersOneChanl(res, pos, 0);
	w = DG_FABS(res->vw);
	res->vx /= w;
	res->vy /= w;
	res->vz /= w;
	res->vx = res->vx * (float)(DRAW_WIDTH / 2) + (float)(DRAW_WIDTH / 2);
	res->vy = res->vy * (float)(DRAW_HEIGHT / 2) + (float)(DRAW_HEIGHT / 2);
}
