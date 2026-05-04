/*
	fpu_pack.h
		PS2用ベクトル／マトリクス演算ライブラリ (FPU使用)

	1999/08/04 K.Kano
	$Id: fpu_pack.h,v 1.1.1.3 2002/11/19 11:42:54 Yoshizawa1 Exp $
*/

/* -----------------------------------------------------------------
   Title    : PS2用ベクトル／マトリクス演算ライブラリ (FPU使用)
   Author   : 狩野  賢一郎 ( Kenichiro Kano ) KCEJ 制作一部
   Date     : 8/4/99
   History  : 8/6/99
              CopyMatrix,CopyUnitMatrixを追加
   ----------------------------------------------------------------- */

#ifndef _fpu_pack_h_
#define _fpu_pack_h_

#ifdef KP_XBOX /***************XBOX用の専用関数(書換え版)************************  T.Morita 2002.02.04 */

#include "fpu_pack_xbox.h"

#else        /************** PS2用の専用関数(オリジナル)***********************  T.Morita 2002.02.04 */

/* FPUで、マトリクスやベクトル演算する場合のためのマクロ */

#if 1 //BP_ASM

/* 絶対値 */
static inline float fpu_Abs(const float a)
{
	int f = *(int *)(&a) & 0x7fffffff ;
	float ans = *(float *)&f ;

    return ans;
}

/* 最大値／最小値 */
static inline float fpu_Max(const float a,const float b)
{
    return a>b ? a : b ;
}

static inline float fpu_Min(const float a,const float b)
{
    return a<b ? a : b ;
}

/* ベクトルのクリア */
static inline void fpu_ClearVector(VECTOR *r)
{
	r->vx = 0.0f ;
	r->vy = 0.0f ;
	r->vz = 0.0f ;
	r->vw = 0.0f ;
}

/* ベクトルのコピー */
static inline void fpu_CopyVector(VECTOR *r,const VECTOR * const a)
{
	r->vx = a->vx ;
	r->vy = a->vy ;
	r->vz = a->vz ;
	r->vw = a->vw ;
}

/* マトリクスのコピー */
static inline void fpu_CopyMatrix(MATRIX *r,const MATRIX * const a)
{
	int i ;

	for ( i=0 ; i<4 ; i++ )
	{
		r->m[i][0] = a->m[i][0] ;
		r->m[i][1] = a->m[i][1] ;
		r->m[i][2] = a->m[i][2] ;
		r->m[i][3] = a->m[i][3] ;
	}
}

/* 単位行列の作成 */
static inline void fpu_CopyUnitMatrix(MATRIX *m)
{
    typedef float _matrix_[4][4];
    static ALIGN16_PRE const _matrix_ unit ALIGN16_POST={
	{ 1.0, 0.0, 0.0, 0.0, },
	{ 0.0, 1.0, 0.0, 0.0, },
	{ 0.0, 0.0, 1.0, 0.0, },
	{ 0.0, 0.0, 0.0, 1.0, },
    };
    fpu_CopyMatrix(m,(MATRIX *)&unit);
}

/* 加算 */
static inline void fpu_AddVectors(VECTOR *r,const VECTOR * const a,const VECTOR * const b)
{
    ((float *)r)[0]=((float *)a)[0]+((float *)b)[0];
    ((float *)r)[1]=((float *)a)[1]+((float *)b)[1];
    ((float *)r)[2]=((float *)a)[2]+((float *)b)[2];
}

/* 減算 */
static inline void fpu_SubVectors(VECTOR *r,const VECTOR * const a,const VECTOR * const b)
{
    ((float *)r)[0]=((float *)a)[0]-((float *)b)[0];
    ((float *)r)[1]=((float *)a)[1]-((float *)b)[1];
    ((float *)r)[2]=((float *)a)[2]-((float *)b)[2];
}

/* ベクトルとスカラーの乗算 */
static inline void fpu_MulVectors(VECTOR *r,const VECTOR * const a,const VECTOR * const b)
{
    ((float *)r)[0]=((float *)a)[0]*((float *)b)[0];
    ((float *)r)[1]=((float *)a)[1]*((float *)b)[1];
    ((float *)r)[2]=((float *)a)[2]*((float *)b)[2];
}


/* ベクトルとスカラーの乗算 */
static inline void fpu_MulVectorScaler(VECTOR *r,const VECTOR * const a,const float b)
{
    ((float *)r)[0]=((float *)a)[0]*b;
    ((float *)r)[1]=((float *)a)[1]*b;
    ((float *)r)[2]=((float *)a)[2]*b;
}

/* ベクトルとスカラーの除算 */
static inline void fpu_DivVectorScaler(VECTOR *r,const VECTOR * const a,const float b)
{
    float ab ;

	if ( b>0.0001f && b<-0.0001f ) {
		ab=1.0f/b;
	} else {
		ab=0.0f ;
	}
	fpu_MulVectorScaler(r,a,ab);
}

/* 内積 */
static inline float fpu_InnerProduct(const VECTOR * const a,const VECTOR * const b)
{
	return a->vx*b->vx + a->vy*b->vy + a->vz*b->vz ;
}

/* 外積 */
static inline void fpu_OuterProduct(VECTOR *r,const VECTOR * const a,const VECTOR * const b)
{
	VECTOR v ;

    v.vx =  a->vy * b->vz -  a->vz * b->vy ;
    v.vy =  a->vz * b->vx -  a->vx * b->vz ;
    v.vz =  a->vx * b->vy -  a->vy * b->vx ;

	*r = v ;
}

/* ベクトルの長さの平方  */
static inline float fpu_VectorLength2(const VECTOR * const a)
{
	return a->vx*a->vx + a->vy*a->vy + a->vz*a->vz ;
}

/* 平方根 */
static inline float fpu_Sqrt(const float a)
{
#if 1//BP_MATH - emulate PS2 sqrt
   return bp_sqrtf(a);  //BP_MATH - emulate PS2 sqrtf
#else
	return sqrtf( fpu_Abs(a) ) ;
#endif
}

/* 平方根の逆数 = b/sqrt(a) */
static inline float fpu_Rsqrt(const float a,const float b)
{
	float sq ;

#if 1//BP_MATH - emulate PS2 sqrt
   sq = bp_sqrtf(a); //BP_MATH - emulate PS2 sqrtf
#else
	sq = sqrtf( fpu_Abs(a) ) ;
#endif
	if ( sq != 0.0f )
	  return b/sq ;
	else
	  return 1.0e+4f ;
}

/* ベクトルの長さを正規化する */
static inline void fpu_VectorNormal(VECTOR *a)
{
    float l;
    l=fpu_VectorLength2(a);
    l=fpu_Rsqrt(l,1.0f);
    fpu_MulVectorScaler(a,a,l);
}

/* マトリクス同士を掛け合わせる */
static inline void fpu_MulMatrices(MATRIX *r,const MATRIX * const a,const MATRIX * const b)
{
	_sceVu0MulMatrix( r, a, b ) ;
}

/* マトリクスとベクトルを掛け合わせる */
static inline void fpu_MulMatrixVector(VECTOR *r,const MATRIX * const m,const VECTOR * const a)
{
	_sceVu0ApplyMatrix( r, m, a ) ;
}

#else //BP_ASM

/* 絶対値 */
static inline float fpu_Abs(const float a)
{
    float ans;
    asm volatile ("
    abs.s	%0,%1
    " : "=f"(ans) : "f"(a) );
    return ans;
}

/* 最大値／最小値 */
static inline float fpu_Max(const float a,const float b)
{
    float ans;
    asm volatile ("
    max.s	%0,%1,%2
    " : "=f"(ans) : "f"(a) , "f"(b) );
    return ans;
}

static inline float fpu_Min(const float a,const float b)
{
    float ans;
    asm volatile ("
    min.s	%0,%1,%2
    " : "=f"(ans) : "f"(a) , "f"(b) );
    return ans;
}

/* ベクトルのクリア */
static inline void fpu_ClearVector(VECTOR *r)
{
    asm("
    sq		$0,0(%0)
    ": : "r"(r) : "memory");
}

/* ベクトルのコピー */
static inline void fpu_CopyVector(VECTOR *r,const VECTOR * const a)
{
    asm ("
    lq    $8,0x0(%1)
    sq    $8,0x0(%0)
    ": : "r"(r), "r"(a) : "$8", "memory");
}

/* マトリクスのコピー */
static inline void fpu_CopyMatrix(MATRIX *r,const MATRIX * const a)
{
    asm ("
    lq    $8,0x00(%1)
    lq    $9,0x10(%1)
    lq    $10,0x20(%1)
    lq    $11,0x30(%1)
    sq    $8,0x00(%0)
    sq    $9,0x10(%0)
    sq    $10,0x20(%0)
    sq    $11,0x30(%0)
    ": : "r"(r), "r"(a) : "$8", "$9", "$10", "$11", "memory");
}

/* 単位行列の作成 */
static inline void fpu_CopyUnitMatrix(MATRIX *m)
{
    typedef float _matrix_[4][4];
    static ALIGN16_PRE const _matrix_ unit ALIGN16_POST={
	{ 1.0, 0.0, 0.0, 0.0, },
	{ 0.0, 1.0, 0.0, 0.0, },
	{ 0.0, 0.0, 1.0, 0.0, },
	{ 0.0, 0.0, 0.0, 1.0, },
    };
    fpu_CopyMatrix(m,(MATRIX *)&unit);
}

/* 加算 */
static inline void fpu_AddVectors(VECTOR *r,const VECTOR * const a,const VECTOR * const b)
{
#if 0
    ((float *)r)[0]=((float *)a)[0]+((float *)b)[0];
    ((float *)r)[1]=((float *)a)[1]+((float *)b)[1];
    ((float *)r)[2]=((float *)a)[2]+((float *)b)[2];
#else
    asm volatile ("
    lwc1	$f1,0x00(%1)
    lwc1	$f2,0x04(%1)
    lwc1	$f3,0x08(%1)
    lwc1	$f4,0x00(%2)
    lwc1	$f5,0x04(%2)
    lwc1	$f6,0x08(%2)
    add.s	$f1,$f1,$f4
    add.s	$f2,$f2,$f5
    add.s	$f3,$f3,$f6
    swc1	$f1,0x00(%0)
    swc1	$f2,0x04(%0)
    swc1	$f3,0x08(%0)
    " : : "r"(r),"r"(a),"r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6","memory");
#endif
}

/* 減算 */
static inline void fpu_SubVectors(VECTOR *r,const VECTOR * const a,const VECTOR * const b)
{
#if 0
    ((float *)r)[0]=((float *)a)[0]-((float *)b)[0];
    ((float *)r)[1]=((float *)a)[1]-((float *)b)[1];
    ((float *)r)[2]=((float *)a)[2]-((float *)b)[2];
#else
    asm volatile ("
    lwc1	$f1,0x00(%1)
    lwc1	$f2,0x04(%1)
    lwc1	$f3,0x08(%1)
    lwc1	$f4,0x00(%2)
    lwc1	$f5,0x04(%2)
    lwc1	$f6,0x08(%2)
    sub.s	$f1,$f1,$f4
    sub.s	$f2,$f2,$f5
    sub.s	$f3,$f3,$f6
    swc1	$f1,0x00(%0)
    swc1	$f2,0x04(%0)
    swc1	$f3,0x08(%0)
    " : : "r"(r),"r"(a),"r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6","memory");
#endif
}

/* ベクトルとスカラーの乗算 */
static inline void fpu_MulVectorScaler(VECTOR *r,const VECTOR * const a,const float b)
{
#if 0
    ((float *)r)[0]=((float *)a)[0]*b;
    ((float *)r)[1]=((float *)a)[1]*b;
    ((float *)r)[2]=((float *)a)[2]*b;
#else
    asm volatile ("
    lwc1	$f1,0x00(%1)
    lwc1	$f2,0x04(%1)
    lwc1	$f3,0x08(%1)
    mul.s	$f1,$f1,%2
    mul.s	$f2,$f2,%2
    mul.s	$f3,$f3,%2
    swc1	$f1,0x00(%0)
    swc1	$f2,0x04(%0)
    swc1	$f3,0x08(%0)
    " : : "r"(r),"r"(a),"f"(b) : "$f1","$f2","$f3","memory");
#endif
}

/* ベクトルとスカラーの除算 */
static inline void fpu_DivVectorScaler(VECTOR *r,const VECTOR * const a,const float b)
{
    float ab=1.0f/b;
    fpu_MulVectorScaler(r,a,ab);
}

/* 内積 */
static inline float fpu_InnerProduct(const VECTOR * const a,const VECTOR * const b)
{
    float ans;
    asm volatile ("
    lwc1	$f1,0(%1)
    lwc1	$f2,0(%2)
    lwc1	$f3,4(%1)
    lwc1	$f4,4(%2)
    lwc1	$f5,8(%1)
    lwc1	$f6,8(%2)
    mula.s	$f1,$f2
    madda.s	$f3,$f4
    madd.s	%0,$f5,$f6
    " : "=f"(ans) : "r"(a), "r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6");
    return ans;
}

/* 外積 */
static inline void fpu_OuterProduct(VECTOR *r,const VECTOR * const a,const VECTOR * const b)
{
    asm volatile ("
    lwc1	$f3,4(%1)
    lwc1	$f6,8(%2)
    lwc1	$f4,4(%2)
    lwc1	$f5,8(%1)
    lwc1	$f2,0(%2)
    lwc1	$f1,0(%1)
    mula.s	$f3,$f6
    msub.s      $f7,$f4,$f5
    mula.s	$f5,$f2
    swc1	$f7,0(%0)
    msub.s	$f8,$f6,$f1
    mula.s	$f1,$f4
    swc1	$f8,4(%0)
    msub.s      $f7,$f2,$f3
    swc1	$f7,8(%0)
    " :  : "r"(r), "r"(a), "r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6","$f7","$f8","memory");
}

/* ベクトルの長さの平方 */
static inline float fpu_VectorLength2(const VECTOR * const a)
{
    float ans;
    asm volatile("
    lwc1	$f1,0(%1)
    lwc1	$f2,4(%1)
    lwc1	$f3,8(%1)
    mula.s	$f1,$f1
    madda.s	$f2,$f2
    madd.s	%0,$f3,$f3
    " : "=f"(ans) : "r"(a) : "$f1","$f2","$f3");
    return ans;
}

/* 平方根 */
static inline float fpu_Sqrt(const float a)
{
    float ans;
    asm volatile ("
    sqrt.s	%0,%1
    " : "=f"(ans) : "f"(a) );
    return ans;
}

/* 平方根の逆数 = b/sqrt(a) */
static inline float fpu_Rsqrt(const float a,const float b)
{
    float ans;
    asm volatile ("
    rsqrt.s	%0,%2,%1
    " : "=f"(ans) : "f"(a),"f"(b) );
    return ans;
}

/* ベクトルの長さを正規化する */
static inline void fpu_VectorNormal(VECTOR *a)
{
    float l;
    l=fpu_VectorLength2(a);
    l=fpu_Rsqrt(l,1.0f);
    fpu_MulVectorScaler(a,a,l);
}

/* マトリクス同士を掛け合わせる */
static inline void fpu_MulMatrices(MATRIX *r,const MATRIX * const a,const MATRIX * const b)
{
    asm volatile ("
    lwc1	$f1,0x00(%1)
    lwc1	$f5,0x00(%2)
    lwc1	$f2,0x10(%1)
    lwc1	$f6,0x04(%2)
    lwc1	$f3,0x20(%1)
    lwc1	$f7,0x08(%2)
    lwc1	$f4,0x30(%1)
    lwc1	$f8,0x0c(%2)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x04(%1)
    lwc1	$f2,0x14(%1)
    lwc1	$f3,0x24(%1)
    lwc1	$f4,0x34(%1)
    swc1	$f9,0x00(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x08(%1)
    lwc1	$f2,0x18(%1)
    lwc1	$f3,0x28(%1)
    lwc1	$f4,0x38(%1)
    swc1	$f9,0x04(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x0c(%1)
    lwc1	$f2,0x1c(%1)
    lwc1	$f3,0x2c(%1)
    lwc1	$f4,0x3c(%1)
    swc1	$f9,0x08(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8

    lwc1	$f5,0x10(%2)
    lwc1	$f6,0x14(%2)
    lwc1	$f7,0x18(%2)
    lwc1	$f8,0x1c(%2)
    swc1	$f9,0x0c(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x08(%1)
    lwc1	$f2,0x18(%1)
    lwc1	$f3,0x28(%1)
    lwc1	$f4,0x38(%1)
    swc1	$f9,0x1c(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x04(%1)
    lwc1	$f2,0x14(%1)
    lwc1	$f3,0x24(%1)
    lwc1	$f4,0x34(%1)
    swc1	$f9,0x18(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x00(%1)
    lwc1	$f2,0x10(%1)
    lwc1	$f3,0x20(%1)
    lwc1	$f4,0x30(%1)
    swc1	$f9,0x14(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8

    lwc1	$f5,0x20(%2)
    lwc1	$f6,0x24(%2)
    lwc1	$f7,0x28(%2)
    lwc1	$f8,0x2c(%2)
    swc1	$f9,0x10(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x04(%1)
    lwc1	$f2,0x14(%1)
    lwc1	$f3,0x24(%1)
    lwc1	$f4,0x34(%1)
    swc1	$f9,0x20(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x08(%1)
    lwc1	$f2,0x18(%1)
    lwc1	$f3,0x28(%1)
    lwc1	$f4,0x38(%1)
    swc1	$f9,0x24(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x0c(%1)
    lwc1	$f2,0x1c(%1)
    lwc1	$f3,0x2c(%1)
    lwc1	$f4,0x3c(%1)
    swc1	$f9,0x28(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8

    lwc1	$f5,0x30(%2)
    lwc1	$f6,0x34(%2)
    lwc1	$f7,0x38(%2)
    lwc1	$f8,0x3c(%2)
    swc1	$f9,0x2c(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x08(%1)
    lwc1	$f2,0x18(%1)
    lwc1	$f3,0x28(%1)
    lwc1	$f4,0x38(%1)
    swc1	$f9,0x3c(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x04(%1)
    lwc1	$f2,0x14(%1)
    lwc1	$f3,0x24(%1)
    lwc1	$f4,0x34(%1)
    swc1	$f9,0x38(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x00(%1)
    lwc1	$f2,0x10(%1)
    lwc1	$f3,0x20(%1)
    lwc1	$f4,0x30(%1)
    swc1	$f9,0x34(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    swc1	$f9,0x30(%0)
    " :  : "r"(r), "r"(a), "r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6","$f7","$f8","$f9","memory");
}

/* マトリクスとベクトルを掛け合わせる */
static inline void fpu_MulMatrixVector(VECTOR *r,const MATRIX * const m,const VECTOR * const a)
{
    asm volatile ("
    lwc1	$f1,0x00(%1)
    lwc1	$f5,0x00(%2)
    lwc1	$f2,0x10(%1)
    lwc1	$f6,0x04(%2)
    lwc1	$f3,0x20(%1)
    lwc1	$f7,0x08(%2)
    lwc1	$f4,0x30(%1)
    lwc1	$f8,0x0c(%2)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x04(%1)
    lwc1	$f2,0x14(%1)
    lwc1	$f3,0x24(%1)
    lwc1	$f4,0x34(%1)
    swc1	$f9,0x00(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x08(%1)
    lwc1	$f2,0x18(%1)
    lwc1	$f3,0x28(%1)
    lwc1	$f4,0x38(%1)
    swc1	$f9,0x04(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    lwc1	$f1,0x0c(%1)
    lwc1	$f2,0x1c(%1)
    lwc1	$f3,0x2c(%1)
    lwc1	$f4,0x3c(%1)
    swc1	$f9,0x08(%0)
    mula.s	$f1,$f5
    madda.s	$f2,$f6
    madda.s	$f3,$f7
    madd.s	$f9,$f4,$f8
    swc1	$f9,0x0c(%0)
    " :  : "r"(r), "r"(m), "r"(a) : "$f1","$f2","$f3","$f4","$f5","$f6","$f7","$f8","$f9","memory");
}

#endif //BP_ASM

#endif /* KP_XBOX ************** PS2用の専用関数(オリジナル)ここまで***********************  T.Morita 2002.02.04 */

#endif
