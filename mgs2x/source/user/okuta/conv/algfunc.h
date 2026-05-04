/*
    algfunc.c
    数学関数
    2001/03/23 Masafumi Okuta
    $Id: algfunc.h,v 1.1.1.3 2002/11/19 11:47:48 Yoshizawa1 Exp $
*/

#ifndef __ALGFUNC_H__
#define __ALGFUNC_H__

#include	"../../../system/libutl/rand.h"

// 円周率,角度用マクロ
#define _MAO_PI   (3.14159265358979323846264338327950288419716939937510f)
#define _MAO_DEG  (_MAO_PI / 180.f)
#define _MAO_RAD  (180.f)

// radian <-> degree 変換マクロ
#define MAO_RAD(d) ((d) * _MAO_DEG)
#define MAO_DEG(r) ((r) * _MAO_RAD)

// ランダム用マクロ
#define MAO_Rand( a ) 		    ( (BP_PS2_rand() % a) ) 
#define MAO_GetRandom( min, max )   ( (BP_PS2_rand() % ((max - min) + 1) + min) ) // min ～ max で生成

// シード固定ランダム(シードをワークに持つタイプ。デモ対応が必要なものに使用)
// AS(JM) - WARNING - MAO_DemoRand is not updating seed that's passed in.  It looks like someone changed a #define 
// to a function, but this is the original code.
static inline int MAO_DemoRand( int seed, int a ){  return (GM_IRnd( &seed ) % a); } 
static inline int MAO_GetDemoRandom( int seed, int min, int max ){ return (GM_IRnd( &seed ) % ((max - min) + 1) + min); }

// ベクトル設定マクロ
#define MAO_SETFVEC( vec, x, y, z) { vec.vx = x; vec.vy = y; vec.vz = z; vec.vw = 1.f; }
#define MAO_SETSVEC( vec, x, y, z) { vec.vx = x; vec.vy = y; vec.vz = z; }

// トランス移動
void MAO_TransVector( FVECTOR*, FMATRIX*, FVECTOR*);

// 絶対,相対変換
void MAO_RelPos(FVECTOR* pvecRes, FVECTOR* pvec, FMATRIX* pmat);
void MAO_AbsPos(FVECTOR* pvecRes, FVECTOR* pvec, FMATRIX* pmat);
void MAO_RelVec(FVECTOR* pvecRes, FVECTOR* pvec, FMATRIX* pmat);
void MAO_AbsVec(FVECTOR* pvecRes, FVECTOR* pvec, FMATRIX* pmat);
void MAO_RelMat(FMATRIX* pmatRes, FMATRIX* pmat1, FMATRIX* pmat2);
void MAO_AbsMat(FMATRIX* pmatRes, FMATRIX* pmat1, FMATRIX* pmat2);
void MAO_RelRot(FMATRIX* pmatRes, FMATRIX* pmat1, FMATRIX* pmat2);
void MAO_AbsRot(FMATRIX* pmatRes, FMATRIX* pmat1, FMATRIX* pmat2);

// 軸設定関数
void MAO_SetAxisY( FMATRIX* pmatRes, FMATRIX* pmat, FVECTOR* pvec);
void MAO_SetAxisZ( FMATRIX* pmatRes, FMATRIX* pmat, FVECTOR* pvec);

// 補間関数
void MAO_InterpVec( FVECTOR* pvecRes, FVECTOR* pvecBase, FVECTOR* pvecAim, float fRate);

// びじぶるべくたー
void MAO_CalcVisibleVector( FVECTOR*, FVECTOR*, FVECTOR*, float);

// エルミート補間
void MAO_HermiteLerpVec( FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float);
void MAO_HermiteLerpVecScale( FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float, float);

// ベジェ補間
void MAO_MakeBezierMatrix( FMATRIX*, FVECTOR*, FVECTOR*, FVECTOR*);
void MAO_Bezier3InterpQuick( FVECTOR*, FMATRIX*, float);
void MAO_Bezier3Interp( FVECTOR*, FVECTOR*, FVECTOR*, FVECTOR*, float);

// ベクトルの長さ取得
float _MAO_FVecLen3( FVECTOR* vec);
float _MAO_FVec2Len3( FVECTOR* vec1, FVECTOR* vec2);
float _MAO_FVec2Len2( FVECTOR* pvec1, FVECTOR* pvec2);

// 注視マトリクス生成
void MAO_GetAimMatrix( FMATRIX*, FVECTOR*, FVECTOR*, FVECTOR*);

// マトリクス補間
void MAO_InterpMatrix( FMATRIX*, FMATRIX*, FMATRIX*, float);
void MAO_InterpVector( FVECTOR*, FVECTOR*, FVECTOR*, float);

#endif





