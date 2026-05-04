/*
	vufpucmb.h
		PS2用ベクトル／マトリクス演算ライブラリ
		(FPU、VU0両方を使用)

	1999/08/04 K.Kano
	$Id: vufpucmb_xbox.h,v 1.1.1.3 2002/11/19 11:43:01 Yoshizawa1 Exp $
*/

/* -----------------------------------------------------------------
   Title    : PS2用ベクトル／マトリクス演算ライブラリ
              (FPU、VU0両方を使用)
   Author   : 狩野  賢一郎 ( Kenichiro Kano ) KCEJ 制作一部
   Date     : 8/4/99
   Histtory : 8/6/99
              InverseMatrixを追加
   ----------------------------------------------------------------- */

#ifndef _vufpucmb_xbox_h_
#define _vufpucmb_xbox_h_

/* ベクトルの長さを算出 */
static inline float VectorLengthv0(void)
{
    float ans;
    ans=vu0_VectorLength2v0();
    ans=fpu_Sqrt(ans);
    return ans;
}

static inline float VectorLengthv1(void)
{
    float ans;
    ans=vu0_VectorLength2v1();
    ans=fpu_Sqrt(ans);
    return ans;
}

static inline float VectorLengthv2(void)
{
    float ans;
    ans=vu0_VectorLength2v2();
    ans=fpu_Sqrt(ans);
    return ans;
}

static inline float VectorLength(const VECTOR * const a)
{
    vu0_Ldv0(a);
    return VectorLengthv0();
}

/* ベクトルを正規化する */
static inline void VectorNormalv0(void)
{
    float l;
    l=vu0_VectorLength2v0();
    l=fpu_Rsqrt(l,1.0f);
    vu0_Mulv0a(l);
}

static inline void VectorNormalv1(void)
{
    float l;
    l=vu0_VectorLength2v1();
    l=fpu_Rsqrt(l,1.0f);
    vu0_Mulv1a(l);
}

static inline void VectorNormalv2(void)
{
    float l;
    l=vu0_VectorLength2v2();
    l=fpu_Rsqrt(l,1.0f);
    vu0_Mulv2a(l);
}

static inline void VectorNormal(VECTOR *a)
{
    vu0_Ldv0(a);
    VectorNormalv0();
    vu0_Stv0(a);
}

/* 逆行列を求める。(Gauss-Jordan法) */
static inline void InverseMatrix(MATRIX *tans,const MATRIX * const tm)
{
	_sceVu0InversMatrix( tans, tm ) ;
}

/* 拡大縮小がなかったら、こちらでもいいはず */
static inline void FastInverseMatrix(MATRIX *tans,const MATRIX * const tm)
{
    float (*m)[4][4]=(float (*)[4][4])tm;
    float (*ans)[4][4]=(float (*)[4][4])tans;

#if 1
	_sceVu0InversMatrix( tans, tm ) ;
#else
    (*ans)[0][0]=(*m)[0][0];
    (*ans)[0][1]=(*m)[1][0];
    (*ans)[0][2]=(*m)[2][0];

    (*ans)[1][0]=(*m)[0][1];
    (*ans)[1][1]=(*m)[1][1];
    (*ans)[1][2]=(*m)[2][1];

    (*ans)[2][0]=(*m)[0][2];
    (*ans)[2][1]=(*m)[1][2];
    (*ans)[2][2]=(*m)[2][2];

	vu0_ApplyMatrix( (VECTOR *)tans->m[3], tans, (VECTOR *)tm->m[3] ) ;
	fpu_MulVectorScaler( (VECTOR *)tans->m[3],
						(VECTOR *)tans->m[3],
						-1.0f ) ;
#endif
}

#endif /* _vufpucmb_xbox_h_ */
