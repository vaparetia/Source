//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    algfunc.c
    数学関数 : Z.O.Eシステムより移植 + カスタマイズ
    2001/03/23 Masafumi Okuta
    $Id: algfunc.c,v 1.1.1.3 2002/11/19 11:47:48 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include "mgs_type.h"
#include "libdg.h"
#include "libmt.h"
#include "algfunc.h"

// 相対位置を計算
void MAO_RelPos(FVECTOR* pvecRes,
		FVECTOR* pvec,
		FMATRIX* pmat)
{
    FVECTOR vec;
    FMATRIX mat;
    
    vec = *pvec;
    mat = *pmat;
    _sceVu0InversMatrix( &mat, &mat);
    _sceVu0ApplyMatrix( pvecRes, &mat, &vec);
}
// 絶対位置を計算
void MAO_AbsPos(FVECTOR* pvecRes,
		FVECTOR* pvec,
		FMATRIX* pmat)
{
    _sceVu0ApplyMatrix( pvecRes, pmat, pvec);
}
// 相対ベクトルを計算
void MAO_RelVec(FVECTOR* pvecRes,
		FVECTOR* pvec,
		FMATRIX* pmat)
{
    FVECTOR vec;
    FMATRIX mat;
    
    vec = *pvec;
    mat = *pmat;
    mat.m[3][0] = 0.f;
    mat.m[3][1] = 0.f;
    mat.m[3][2] = 0.f;
    mat.m[3][3] = 1.f;
    _sceVu0InversMatrix( &mat, &mat);
    _sceVu0ApplyMatrix( pvecRes, &mat, &vec);
}
// 絶対ベクトルを計算
void MAO_AbsVec(FVECTOR* pvecRes,
		FVECTOR* pvec,
		FMATRIX* pmat)
{
    FMATRIX mat;
    
    mat = *pmat;
    mat.m[3][0] = 0.f;
    mat.m[3][1] = 0.f;
    mat.m[3][2] = 0.f;
    mat.m[3][3] = 1.f;
    _sceVu0ApplyMatrix( pvecRes, &mat, pvec);
}

// 相対マトリクス計算
void MAO_RelMat(FMATRIX*	pmatRes,
		FMATRIX*	pmat1,
		FMATRIX*	pmat2)
{
    FMATRIX mat;

    mat = *pmat2;
    _sceVu0InversMatrix( &mat, &mat);
    _sceVu0MulMatrix( pmatRes, &mat, pmat1);
}

// 相対ローテーション計算
void MAO_RelRot(FMATRIX*	pmatRes,
		FMATRIX*	pmat1,
		FMATRIX*	pmat2)
{
    FMATRIX mat;

    mat = *pmat2;
    mat.m[3][0] = 0.f;
    mat.m[3][1] = 0.f;
    mat.m[3][2] = 0.f;
    mat.m[3][3] = 1.f;
    _sceVu0InversMatrix( &mat, &mat);
    _sceVu0MulMatrix( pmatRes, &mat, pmat1);
}

// 絶対マトリクス計算
void MAO_AbsMat(FMATRIX*	pmatRes,
		FMATRIX*	pmat1,
		FMATRIX*	pmat2)
{
    _sceVu0MulMatrix( pmatRes, pmat1, pmat2);
}

// 絶対ローテーション計算
void MAO_AbsRot(FMATRIX*	pmatRes,
		FMATRIX*	pmat1,
		FMATRIX*	pmat2)
{
    _sceVu0MulMatrix( pmatRes, pmat1, pmat2);
    pmatRes->m[3][0] = 0.f;
    pmatRes->m[3][1] = 0.f;
    pmatRes->m[3][2] = 0.f;
    pmatRes->m[3][3] = 1.f;
}
// Y軸設定
void MAO_SetAxisY( FMATRIX* pmatRes, FMATRIX* pmat, FVECTOR* pvec)
{
    FMATRIX mat;

    // マトリクス退避
    _sceVu0CopyMatrix( &mat, pmat);

    // 軸方向計算
    // Y軸設定
    _sceVu0CopyVector( (FVECTOR*)&mat.m[1][0], pvec);
    // X軸設定
    _sceVu0OuterProduct( (FVECTOR*)&mat.m[0][0], (FVECTOR*)&mat.m[1][0], (FVECTOR*)&mat.m[2][0]);
    if ( _MAO_FVecLen3( (FVECTOR*)&mat.m[0][0] ) == 0.f){
	_sceVu0CopyMatrix( pmatRes, &DG_UnitMatrix);
	return;	// 失敗
    }
    _sceVu0Normalize( (FVECTOR*)&mat.m[0][0], (FVECTOR*)&mat.m[0][0]);
    // Z軸設定
    _sceVu0OuterProduct( (FVECTOR*)&mat.m[2][0], (FVECTOR*)&mat.m[0][0], (FVECTOR*)&mat.m[1][0]);
    if ( _MAO_FVecLen3( (FVECTOR*)&mat.m[2][0] ) == 0.f){
	_sceVu0CopyMatrix( pmatRes, &DG_UnitMatrix);
	return;	// 失敗
    }
    _sceVu0Normalize( (FVECTOR*)&mat.m[2][0], (FVECTOR*)&mat.m[2][0]);
    _sceVu0CopyMatrix( pmatRes, &mat);
}
// Z軸設定
void MAO_SetAxisZ( FMATRIX* pmatRes, FMATRIX* pmat, FVECTOR* pvec)
{
    FMATRIX mat;

    // マトリクス退避
    _sceVu0CopyMatrix( &mat, pmat);

    // 軸方向計算
    // Z軸設定
    _sceVu0CopyVector( (FVECTOR*)&mat.m[2][0], pvec);
    // X軸設定
    _sceVu0OuterProduct( (FVECTOR*)&mat.m[0][0], (FVECTOR*)&mat.m[1][0], (FVECTOR*)&mat.m[2][0]);

    if ( _MAO_FVecLen3( (FVECTOR*)&mat.m[0][0] ) == 0.f){
	_sceVu0CopyMatrix( pmatRes, &DG_UnitMatrix);
	return;	// 失敗
    }

    _sceVu0Normalize( (FVECTOR*)&mat.m[0][0], (FVECTOR*)&mat.m[0][0]);
    // Y軸設定
    _sceVu0OuterProduct( (FVECTOR*)&mat.m[1][0], (FVECTOR*)&mat.m[2][0], (FVECTOR*)&mat.m[0][0]);

    if ( _MAO_FVecLen3( (FVECTOR*)&mat.m[1][0] ) == 0.f){
	_sceVu0CopyMatrix( pmatRes, &DG_UnitMatrix);
	return;	// 失敗
    }

    _sceVu0Normalize( (FVECTOR*)&mat.m[1][0], (FVECTOR*)&mat.m[1][0]);
    _sceVu0CopyMatrix( pmatRes, &mat);
}
// ベクトルの補間
void MAO_InterpVec( FVECTOR* pvecRes, FVECTOR* pvecBase, FVECTOR* pvecAim, float fRate)
{
    FVECTOR vec1, vec2;

    _sceVu0ScaleVector( &vec1, pvecBase, 1.f - fRate);
    _sceVu0ScaleVector( &vec2, pvecAim, fRate);
    _sceVu0AddVector( pvecRes, &vec1, &vec2);
}
// １→２の線分に垂直なベクトルで画面に表示された時もっとも長く見えるベクトル方向を求める。
// 通称びじぶるべくたー
void MAO_CalcVisibleVector( FVECTOR* pvecRes, 	// 返り値
			    FVECTOR* pvec1,	// 頂点１
			    FVECTOR* pvec2,	// 頂点２
			    float    fWidth)	// 幅
{
// yano
#ifdef BP_PSX2_ASM  /*未検証*/
    FVECTOR* pvecCamPos;

    pvecCamPos = (FVECTOR*)DG_Chanls[0].eye.m[3]; // カメラの座標

    asm volatile("
		lqc2	vf04, 0(%1)
		lqc2	vf05, 0(%2)
		lqc2	vf06, 0(%3)
		vsub.xyz	vf06, vf04, vf06
		vsub.xyz	vf05, vf04, vf05
		mfc1	$8,%4
		qmtc2	$8, vf07
		vopmula.xyz	ACC,vf06,vf05
		vopmsub.xyz vf06,vf05,vf06
		/* vf06 外積 vf05 = vf06.vf06 */
		vmul.xyz	vf05,vf06,vf06
		vaddw.x		vf04,vf00,vf00
		vmula.x		ACC,vf06,vf06
		vmadday.x	ACC,vf04,vf05
		vmaddz.x	vf05,vf04,vf05
		vrsqrt		Q,vf07x,vf05x
		vmove.w		vf07,vf00
		vwaitq
		vmulq.xyz	vf07,vf06,Q
		sqc2		vf07,0(%0)
	" : : "r"(pvecRes), "r"(pvec1), "r"(pvec2), "r"(pvecCamPos), "f"(fWidth)
	  : "memory", "$8");
#else
    FVECTOR* pvecCamPos;
	FVECTOR fvtemp0, fvtemp1;
	float ftemp0, ftemp1, ftempQ;

    pvecCamPos = (FVECTOR*)DG_Chanls[0].eye.m[3]; // カメラの座標

	_sceVu0SubVector( &fvtemp0, pvec1, pvecCamPos );
	_sceVu0SubVector( &fvtemp1, pvec1, pvec2 );
	_sceVu0OuterProduct( &fvtemp0, &fvtemp0, &fvtemp1 );
	ftemp0 = fvtemp0.vx*fvtemp0.vx + fvtemp0.vy*fvtemp0.vy + fvtemp0.vz*fvtemp0.vz;
	ftemp1 = DG_SQRT( ftemp0 );
	ftempQ = fWidth / ftemp1;
	_sceVu0ScaleVectorXYZ( pvecRes, &fvtemp0, ftempQ );
	pvecRes->vw = 1.0f ;
#endif
}


// エルミート計算用マトリックス
static FMATRIX c_mat2 = {{
	{ 2.0f,-2.0f, 1.0f, 1.0f},
	{-3.0f, 3.0f,-2.0f,-1.0f},
	{ 0.0f, 0.0f, 1.0f, 0.0f},
	{ 1.0f, 0.0f, 0.0f, 0.0f}
}};
static FMATRIX c_mat3 = {{
	{ 0.0f, 0.0f, 0.0f, 0.0f},
	{ 6.0f,-6.0f, 3.0f, 3.0f},
	{-6.0f, 6.0f,-4.0f,-2.0f},
	{ 0.0f, 0.0f, 1.0f, 0.0f}
}};
static void MAO_LoadMatHermiteLerp(void)
{
// yano
#ifdef BP_PSX2_ASM
	asm volatile("
		lqc2			vf04,0x00(%0)	# 定数マトリクス読み込み
		lqc2			vf05,0x10(%0)	# 定数マトリクス読み込み
		lqc2			vf06,0x20(%0)	# 定数マトリクス読み込み
		lqc2			vf07,0x30(%0)	# 定数マトリクス読み込み

		lqc2			vf08,0x00(%1)	# 定数マトリクス読み込み
		lqc2			vf09,0x10(%1)	# 定数マトリクス読み込み
		lqc2			vf10,0x20(%1)	# 定数マトリクス読み込み
		lqc2			vf11,0x30(%1)	# 定数マトリクス読み込み
	"::"r"(&c_mat2),"r"(&c_mat3));
#else
   BP_BREAK;
	printf( "ASM ERROR !!!!! in user/okuta/conv/algfunc.c\n" );
#endif
}
void MAO_HermiteLerpVec( FVECTOR *res, 		// 座標計算結果
			 FVECTOR *dvec, 	// 変化量計算結果
			 FVECTOR *q0, 		// 補間元座標
			 FVECTOR *q1, 		// 補間先座標
			 FVECTOR *d0, 		// 補間元変化量
			 FVECTOR *d1, 		// 補間先変化量
			 float t )		// 時間 0.f ～ 1.f
{
#ifdef BP_PSX2_ASM //yano　/*未検証*/

    MAO_LoadMatHermiteLerp();

	asm volatile("
		mfc1			$4,%6			# 補間パラメータの読み込み
		lqc2			vf12,0x00(%2)	# 補間座標０読み込み
		lqc2			vf13,0x00(%3)	# 補間座標１読み込み
		lqc2			vf14,0x00(%4)	# 微分成分０の読み込み
		lqc2			vf15,0x00(%5)	# 微分成分１の読み込み
		qmtc2			$4,vf01			# 補間パラメータの読み込み

		vmulax.xyzw		ACC ,vf12,vf04
		vmadday.xyzw	ACC ,vf13,vf04
		vmaddaz.xyzw	ACC ,vf14,vf04
		vmaddw.xyzw		vf16,vf15,vf04
		vaddx.xyz		vf02,vf00,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf05
		vmadday.xyzw	ACC ,vf13,vf05
		vmaddaz.xyzw	ACC ,vf14,vf05
		vmaddw.xyzw		vf17,vf15,vf05
		vmulx.xy		vf02,vf02,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf06
		vmadday.xyzw	ACC ,vf13,vf06
		vmaddaz.xyzw	ACC ,vf14,vf06
		vmaddw.xyzw		vf18,vf15,vf06
		vmulx.x			vf02,vf02,vf01	# tvec make (t^3,t^2,t,?)
		vmulax.xyzw		ACC ,vf12,vf07
		vmadday.xyzw	ACC ,vf13,vf07
		vmaddaz.xyzw	ACC ,vf14,vf07
		vmaddw.xyzw		vf19,vf15,vf07

		vmulax.xyzw		ACC ,vf12,vf08
		vmadday.xyzw	ACC ,vf13,vf08
		vmaddaz.xyzw	ACC ,vf14,vf08
		vmaddw.xyzw		vf20,vf15,vf08
		vmulax.xyzw		ACC ,vf12,vf09
		vmadday.xyzw	ACC ,vf13,vf09
		vmaddaz.xyzw	ACC ,vf14,vf09
		vmaddw.xyzw		vf21,vf15,vf09
		vmulax.xyzw		ACC ,vf12,vf10
		vmadday.xyzw	ACC ,vf13,vf10
		vmaddaz.xyzw	ACC ,vf14,vf10
		vmaddw.xyzw		vf22,vf15,vf10
		vmulax.xyzw		ACC ,vf12,vf11
		vmadday.xyzw	ACC ,vf13,vf11
		vmaddaz.xyzw	ACC ,vf14,vf11
		vmaddw.xyzw		vf23,vf15,vf11

		vmulax.xyzw		ACC ,vf16,vf02
		vmadday.xyzw	ACC ,vf17,vf02
		vmaddaz.xyzw	ACC ,vf18,vf02
		vmaddw.xyzw		vf24,vf19,vf00
		vmulax.xyzw		ACC ,vf20,vf02
		vmadday.xyzw	ACC ,vf21,vf02
		vmaddaz.xyzw	ACC ,vf22,vf02
		vmaddw.xyzw		vf25,vf23,vf00

		sqc2			vf24,0x00(%0)
		sqc2			vf25,0x00(%1)

	"::"r"(res),"r"(dvec),"r"(q0),"r"(q1),"r"(d0),"r"(d1),"f"(t):"$4");

#else
	FMATRIX	data_mat;
	FVECTOR	tvec ;

	tvec.vx = t * t * t ;
	tvec.vy = t * t ;
	tvec.vz = t ;
	tvec.vw = 1.0f ;
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;
	_sceVu0MulMatrix( &data_mat, &data_mat, &c_mat2 );
	_sceVu0ApplyMatrix( res, &data_mat, &tvec );

	_sceVu0MulMatrix( &data_mat, &data_mat, &c_mat3 );
	_sceVu0ApplyMatrix( dvec, &data_mat, &tvec );
#endif	
}


// ３点ベジェ補間 変換マトリクス生成
// 	    |  1 -2  1  0 ||    vec1 |
// matRes = | -2  2  0  0 ||    vec2 |
// 	    |  1  0  0  0 ||    vec3 |
// 	    |  0  0  0  1 || 0 0 0 1 |
void MAO_MakeBezierMatrix( FMATRIX* pmatBezier, FVECTOR* pvec1, FVECTOR* pvec2, FVECTOR* pvec3)
{
    static FMATRIX MAO_BEZIER_MAT = { // ベジェ補間用係数
        { {  1.f, -2.f, 1.f, 0.f },
          { -2.f,  2.f, 0.f, 0.f },
	  {  1.f,  0.f, 0.f, 0.f },
	  {  0.f,  0.f, 0.f, 1.f } }
    };
    _sceVu0CopyVector( (FVECTOR*)&pmatBezier->m[0][0], pvec1 );
    _sceVu0CopyVector( (FVECTOR*)&pmatBezier->m[1][0], pvec2 );
    _sceVu0CopyVector( (FVECTOR*)&pmatBezier->m[2][0], pvec3 );
    _sceVu0CopyVector( (FVECTOR*)&pmatBezier->m[3][0], &DG_ZeroVector );

    _sceVu0MulMatrix( pmatBezier, pmatBezier, &MAO_BEZIER_MAT);
}
// ３点ベジェ補間 変換マトリクス生成済版
// vecRes = (1-t)^2 * vec1 + 2t*(1-t) * vec2 + t^2 * vec3
// 	    |  1 -2  1  0 ||    vec1 || t^2 t 1 0 |
// vecRes = | -2  2  0  0 ||    vec2 |
// 	    |  1  0  0  0 ||    vec3 |
// 	    |  0  0  0  1 || 0 0 0 1 |
// 
void MAO_Bezier3InterpQuick( FVECTOR* pvecRes, FMATRIX* pmatBezier, float fRate)
{
    FVECTOR vecRate;

    // 割合から係数用のベクトルを生成
    vecRate.vx = fRate * fRate;
    vecRate.vy = fRate;
    vecRate.vz = 1.f;
    vecRate.vw = 1.f;

    _sceVu0ApplyMatrix( pvecRes, pmatBezier, &vecRate);
}
// ３点ベジェ補間 ３点指定
void MAO_Bezier3Interp( FVECTOR* pvecRes, FVECTOR* pvec1, FVECTOR* pvec2, FVECTOR* pvec3, float fRate)
{
    FVECTOR vecRate;
    FMATRIX matBezier;

    MAO_MakeBezierMatrix( &matBezier, pvec1, pvec2, pvec3);

    // 割合から係数用のベクトルを生成
    vecRate.vx = fRate * fRate;
    vecRate.vy = fRate;
    vecRate.vz = 1.f;
    vecRate.vw = 1.f;

    _sceVu0ApplyMatrix( pvecRes, &matBezier, &vecRate);
}

// 可変フレーム版
void MAO_HermiteLerpVecScale( FVECTOR *res, 	// 座標計算結果
			      FVECTOR *dvec, 	// 変化量計算結果
			      FVECTOR *q0, 	// 補間元座標
			      FVECTOR *q1, 	// 補間先座標
			      FVECTOR *d0, 	// 補間元変化量
			      FVECTOR *d1, 	// 補間先変化量
			      float    t,	// 時間 0.f ～ 1.f
			      float    scale)	// サイズ(フレーム数)
{
// yano
#ifdef BP_PSX2_ASM /*未検証*/
    MAO_LoadMatHermiteLerp();

    asm volatile("
		mfc1			$4,%7			# スケール値の読み込み
		lqc2			vf12,0x00(%2)	# 補間座標０読み込み
		qmtc2			$4,vf31			# スケール値の読み込み
		lqc2			vf13,0x00(%3)	# 補間座標１読み込み
		mfc1			$4,%6			# 補間パラメータの読み込み
		lqc2			vf14,0x00(%4)	# 微分成分０の読み込み
		qmtc2			$4,vf01			# 補間パラメータの読み込み
		lqc2			vf15,0x00(%5)	# 微分成分１の読み込み
		vmulx.xyzw		vf14,vf14,vf31	# 微分成分０のスケーリング
		vmulx.xyzw		vf15,vf15,vf31	# 微分成分１のスケーリング

		vmulax.xyzw		ACC ,vf12,vf04
		vmadday.xyzw	ACC ,vf13,vf04
		vmaddaz.xyzw	ACC ,vf14,vf04
		vmaddw.xyzw		vf16,vf15,vf04
		vaddx.xyz		vf02,vf00,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf05
		vmadday.xyzw	ACC ,vf13,vf05
		vmaddaz.xyzw	ACC ,vf14,vf05
		vmaddw.xyzw		vf17,vf15,vf05
		vmulx.xy		vf02,vf02,vf01	# tvec make
		vmulax.xyzw		ACC ,vf12,vf06
		vmadday.xyzw	ACC ,vf13,vf06
		vmaddaz.xyzw	ACC ,vf14,vf06
		vmaddw.xyzw		vf18,vf15,vf06
		vmulx.x			vf02,vf02,vf01	# tvec make (t^3,t^2,t,?)
		vmulax.xyzw		ACC ,vf12,vf07
		vmadday.xyzw	ACC ,vf13,vf07
		vmaddaz.xyzw	ACC ,vf14,vf07
		vmaddw.xyzw		vf19,vf15,vf07

		vmulax.xyzw		ACC ,vf12,vf08
		vmadday.xyzw	ACC ,vf13,vf08
		vmaddaz.xyzw	ACC ,vf14,vf08
		vmaddw.xyzw		vf20,vf15,vf08
		vmulax.xyzw		ACC ,vf12,vf09
		vmadday.xyzw	ACC ,vf13,vf09
		vmaddaz.xyzw	ACC ,vf14,vf09
		vmaddw.xyzw		vf21,vf15,vf09
		vmulax.xyzw		ACC ,vf12,vf10
		vmadday.xyzw	ACC ,vf13,vf10
		vmaddaz.xyzw	ACC ,vf14,vf10
		vmaddw.xyzw		vf22,vf15,vf10
		vmulax.xyzw		ACC ,vf12,vf11
		vmadday.xyzw	ACC ,vf13,vf11
		vmaddaz.xyzw	ACC ,vf14,vf11
		vmaddw.xyzw		vf23,vf15,vf11

		vmulax.xyzw		ACC ,vf16,vf02
		vmadday.xyzw	ACC ,vf17,vf02
		vmaddaz.xyzw	ACC ,vf18,vf02
		vmaddw.xyzw		vf24,vf19,vf00
		vmulax.xyzw		ACC ,vf20,vf02
		vmadday.xyzw	ACC ,vf21,vf02
		vmaddaz.xyzw	ACC ,vf22,vf02
		vmaddw.xyzw		vf25,vf23,vf00

		sqc2			vf24,0x00(%0)
		sqc2			vf25,0x00(%1)

    "::"r"(res),"r"(dvec),"r"(q0),"r"(q1),"r"(d0),"r"(d1),"f"(t), "f"(scale):"$4");
#else
	FMATRIX	data_mat;
	FVECTOR	tvec ;

	tvec.vx = t * t * t ;
	tvec.vy = t * t ;
	tvec.vz = t ;
	tvec.vw = 1.0f ;
	_sceVu0ScaleVector( d0, d0, scale );
	_sceVu0ScaleVector( d1, d1, scale );
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;

	_sceVu0MulMatrix( &data_mat, &data_mat, &c_mat2 );
	_sceVu0ApplyMatrix( res, &data_mat, &tvec );

	_sceVu0MulMatrix( &data_mat, &data_mat, &c_mat3 );
	_sceVu0ApplyMatrix( dvec, &data_mat, &tvec );

#endif
}


// ベクトルの長さ取得
float _MAO_FVecLen3( FVECTOR* vec)
{
	FVECTOR		tmp ;

	tmp.vx = vec->vx * vec->vx ;
	tmp.vy = vec->vy * vec->vy ;
	tmp.vz = vec->vz * vec->vz ;
	return bp_sqrtf( tmp.vx + tmp.vy + tmp.vz ) ;   //BP_MATH - emulate PS2 sqrtf
}
// 二点間の長さ取得
float _MAO_FVec2Len3( FVECTOR* pvec1, FVECTOR* pvec2)
{
    FVECTOR	tmp, vec;

    _sceVu0SubVector( &vec, pvec1, pvec2);
    tmp.vx = vec.vx * vec.vx ;
    tmp.vy = vec.vy * vec.vy ;
    tmp.vz = vec.vz * vec.vz ;
    return bp_sqrtf( tmp.vx + tmp.vy + tmp.vz ) ;  //BP_MATH - emulate PS2 sqrtf
}
// 二点間の長さ取得(高さ無視)
float _MAO_FVec2Len2( FVECTOR* pvec1, FVECTOR* pvec2)
{
    FVECTOR	tmp, vec;

    _sceVu0SubVector( &vec, pvec1, pvec2);
    tmp.vx = vec.vx * vec.vx ;
    tmp.vz = vec.vz * vec.vz ;
    return bp_sqrtf( tmp.vx + tmp.vz ) ;  //BP_MATH - emulate PS2 sqrtf
}
// 注視するマトリクスを生成
void  MAO_GetAimMatrix( FMATRIX* pmat,		// 結果
		        FVECTOR* pvecView,	// 視点
			FVECTOR* pvecLook,	// 注視点
		        FVECTOR* pvecY)		// 参考Y軸
{
    _sceVu0CopyMatrix( pmat, &DG_UnitMatrix);
    _sceVu0CopyVector( (FVECTOR*)&pmat->m[1][0], pvecY);
    _sceVu0CopyVector( (FVECTOR*)&pmat->m[3][0], pvecView);

    // Z軸算出
    _sceVu0SubVector( (FVECTOR*)&pmat->m[2][0], pvecView, pvecLook);
    if ( _MAO_FVecLen3( (FVECTOR*)&pmat->m[2][0] ) == 0.f){
	_sceVu0CopyMatrix( pmat, &DG_UnitMatrix);
	return;
    }
    _sceVu0Normalize( (FVECTOR*)&pmat->m[2][0], (FVECTOR*)&pmat->m[2][0]);

    // X軸算出
    _sceVu0OuterProduct( (FVECTOR*)&pmat->m[0][0], (FVECTOR*)&pmat->m[1][0], (FVECTOR*)&pmat->m[2][0]);
    if ( _MAO_FVecLen3( (FVECTOR*)&pmat->m[0][0] ) == 0.f){
	_sceVu0CopyMatrix( pmat, &DG_UnitMatrix);
	return;
    }
    _sceVu0Normalize( (FVECTOR*)&pmat->m[0][0], (FVECTOR*)&pmat->m[0][0]);

    // Y軸算出
    _sceVu0OuterProduct( (FVECTOR*)&pmat->m[1][0], (FVECTOR*)&pmat->m[2][0], (FVECTOR*)&pmat->m[0][0]);
    if ( _MAO_FVecLen3( (FVECTOR*)&pmat->m[1][0] ) == 0.f){
	_sceVu0CopyMatrix( pmat, &DG_UnitMatrix);
	return;
    }
    _sceVu0Normalize( (FVECTOR*)&pmat->m[1][0], (FVECTOR*)&pmat->m[1][0]);
}

// 移動量を指定座標系に絶対化して平行移動を行ない結果の座標を返す
void MAO_TransVector( FVECTOR*	out, 	// 出力先
		      FMATRIX*	world, 	// 基準となる座標系	
		      FVECTOR*	in )	// 移動値
{
// yano /*未検証*/
#ifdef BP_PSX2_ASM
	asm ("
	lqc2				vf8,0x00(%2)
	lqc2				vf4,0x00(%1)
	lqc2				vf5,0x10(%1)
	lqc2				vf6,0x20(%1)
	lqc2				vf7,0x30(%1)
	vmulax.xyzw			ACC, vf4,vf8
	vmadday.xyzw		ACC, vf5,vf8
	vmaddaz.xyzw		ACC, vf6,vf8
	vmaddw.xyzw			vf8, vf7,vf8
	sqc2				vf8,0x00(%0)
	": : "r"(out), "r"(world), "r"(in) );
#else
	_sceVu0ApplyMatrix( out, world, in );
#endif
}

// マトリクスの補間
void MAO_InterpMatrix( FMATRIX* pmatRes, FMATRIX* pmatFrom, FMATRIX* pmatTo, float fRate)
{
    FVECTOR quatFrom, quatTo, quatRes;

    // マトリクス->クォータニオン
    MT_MatToQuat( &quatFrom, pmatFrom);
    MT_MatToQuat( &quatTo, pmatTo);
    
    // 補間
    MT_QuatSlerp( &quatRes, &quatFrom, &quatTo, fRate);
    MT_QuatNormalize( &quatRes, &quatRes);

    // クォータニオン->マトリクス
    MT_QuatToMat( pmatRes, &quatRes);
    
}
// ベクトルの補間
void MAO_InterpVector( FVECTOR* pvecRes, FVECTOR* pvec1, FVECTOR* pvec2, float fRate)
{
// yano /*未検証*/
#ifndef  BP_PSX2_ASM
    FVECTOR vec1, vec2;

    _sceVu0ScaleVectorXYZ( &vec1, pvec1, fRate);
    _sceVu0ScaleVectorXYZ( &vec2, pvec2, (1.f - fRate));
    _sceVu0AddVector( pvecRes, &vec1, &vec2 );
#else
    float fRateRev;

    fRateRev = (1.f - fRate);
    /*
      ２点を指定されたレートで補間する
      vf01x : $4 : fRate
      vf02x : $5 : 1.f - fRate
      vf06  : pvec1 * fRateの結果一時格納
      vf07  : pvec2 * (1.f - fRate)の結果一時格納
      vf08  : pvecRes : 結果
      vf10  : pvec1   : 補間座標元
      vf11  : pvec2   : 補間座標先
    */
    asm volatile("
        mfc1		$4, %3			# 補間パラメータ(fRate)の読み込み
        mfc1		$5, %4			# 補間パラメータ(1.f - fRate)の読み込み
        lqc2		vf10, 0x00(%1)		# 補間座標元読み込み
		lqc2		vf11, 0x00(%2)		# 補間座標先読み込み
        qmtc2		$4, vf01		# 補間パラメータ(fRate)の読み込み
        qmtc2		$5, vf02		# 補間パラメータ(1.f - fRate)の読み込み
        vmulx.xyz	vf06, vf10, vf01x	# (*pvec1) *= fRate
        vmulx.xyz	vf07, vf11, vf02x	# (*pvec2) *= fRateRev
        vadd.xyzw	vf08, vf06, vf07	# (*pvec1) + (*pvec2)	
        sqc2		vf08, 0(%0)		# pvecResに出力
    ":: "r"(pvecRes), "r"(pvec1), "r"(pvec2), "f"(fRate), "f"(fRateRev) : "$4", "$5");
#endif
}



