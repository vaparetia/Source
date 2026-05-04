/*
	fpu_pack.h
		PS2用ベクトル／マトリクス演算ライブラリ (FPU使用)

	1999/08/04 K.Kano
	$Id: fpu_pack_xbox.h,v 1.1.1.3 2002/11/19 11:42:54 Yoshizawa1 Exp $
*/

/* -----------------------------------------------------------------
   Title    : PS2用ベクトル／マトリクス演算ライブラリ (FPU使用)
   Author   : 狩野  賢一郎 ( Kenichiro Kano ) KCEJ 制作一部
   Date     : 8/4/99
   History  : 8/6/99
              CopyMatrix,CopyUnitMatrixを追加
   ----------------------------------------------------------------- */

#ifndef _fpu_pack_xbox_h_
#define _fpu_pack_xbox_h_

/* FPUで、マトリクスやベクトル演算する場合のためのマクロ */

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

#endif /* _fpu_pack_xbox_h_ */
