/*
	vufpucmb.h
		PS2用ベクトル／マトリクス演算ライブラリ
		(FPU、VU0両方を使用)

	1999/08/04 K.Kano
	$Id: vufpucmb.h,v 1.1.1.3 2002/11/19 11:43:01 Yoshizawa1 Exp $
*/

/* -----------------------------------------------------------------
   Title    : PS2用ベクトル／マトリクス演算ライブラリ
              (FPU、VU0両方を使用)
   Author   : 狩野  賢一郎 ( Kenichiro Kano ) KCEJ 制作一部
   Date     : 8/4/99
   Histtory : 8/6/99
              InverseMatrixを追加
   ----------------------------------------------------------------- */

#ifndef _vufpucmb_h_
#define _vufpucmb_h_


#include "vu0_pack.h"
#include "fpu_pack.h"

#ifdef KP_XBOX /***************XBOX用の専用関数(書換え版)************************  T.Morita 2002.02.04 */

#include "vufpucmb_xbox.h"

#else        /************** PS2用の専用関数(オリジナル)***********************  T.Morita 2002.02.04 */

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
#if 1 //BP_ASM
   _sceVu0InversMatrix( tans, tm ) ;
#else
    float m[4][4];
    float (*ans)[4][4]=(float (*)[4][4])tans;
    int i,mi;

    fpu_CopyUnitMatrix((MATRIX *)ans);
    fpu_CopyMatrix((MATRIX *)m,(MATRIX *)tm);

    mi=0;
    for(i=1;i<4;i++) if(fpu_Abs(m[0][mi])<fpu_Abs(m[0][i])) mi=i;
    for(i=0;i<4;i++){
	float tmp,tmp2;
	tmp=m[mi][i], m[mi][i]=m[0][i], m[0][i]=tmp;
	tmp2=(*ans)[mi][i], (*ans)[mi][i]=(*ans)[0][i], (*ans)[0][i]=tmp2;
    }
    asm volatile ("
    lqc2	vf16,0x00(%0)
    lqc2	vf17,0x10(%0)
    lqc2	vf18,0x20(%0)
    lqc2	vf19,0x30(%0)
    vdiv	Q,vf00w,vf16x
    lqc2	vf24,0x00(%1)
    lqc2	vf25,0x10(%1)
    lqc2	vf26,0x20(%1)
    lqc2	vf27,0x30(%1)
    vmulq.x	vf16,vf16,Q
    vmulq.x	vf17,vf17,Q
    vmulq.x	vf18,vf18,Q
    vmulq.x	vf19,vf19,Q
    vmulq.x	vf24,vf24,Q
    vmulq.x	vf25,vf25,Q
    vmulq.x	vf26,vf26,Q
    vmulq.x	vf27,vf27,Q
    vmulx.yzw	vf20,vf16,vf16
    vmulx.yzw	vf21,vf16,vf17
    vmulx.yzw	vf22,vf16,vf18
    vmulx.yzw	vf23,vf16,vf19
    vmulx.yzw	vf28,vf16,vf24
    vmulx.yzw	vf29,vf16,vf25
    vmulx.yzw	vf30,vf16,vf26
    vmulx.yzw	vf31,vf16,vf27
    vsub.yzw	vf16,vf16,vf20
    vsub.yzw	vf17,vf17,vf21
    vsub.yzw	vf18,vf18,vf22
    vsub.yzw	vf19,vf19,vf23
    vsub.yzw	vf24,vf24,vf28
    vsub.yzw	vf25,vf25,vf29
    vsub.yzw	vf26,vf26,vf30
    vsub.yzw	vf27,vf27,vf31
    sqc2	vf16,0x00(%0)
    sqc2	vf17,0x10(%0)
    sqc2	vf18,0x20(%0)
    sqc2	vf19,0x30(%0)
    sqc2	vf24,0x00(%1)
    sqc2	vf25,0x10(%1)
    sqc2	vf26,0x20(%1)
    sqc2	vf27,0x30(%1)
    " : : "r"(m) , "r"(ans) : "memory");

    mi=1;
    for(i=2;i<4;i++) if(fpu_Abs(m[1][mi])<fpu_Abs(m[1][i])) mi=i;
    for(i=0;i<4;i++){
	float tmp,tmp2;
	tmp=m[mi][i], m[mi][i]=m[1][i], m[1][i]=tmp;
	tmp2=(*ans)[mi][i], (*ans)[mi][i]=(*ans)[1][i], (*ans)[1][i]=tmp2;
    }
    asm volatile ("
    lqc2	vf17,0x10(%0)
    lqc2	vf18,0x20(%0)
    lqc2	vf19,0x30(%0)
    vdiv	Q,vf00w,vf17y
    lqc2	vf24,0x00(%1)
    lqc2	vf25,0x10(%1)
    lqc2	vf26,0x20(%1)
    lqc2	vf27,0x30(%1)
    vmulq.y	vf17,vf17,Q
    vmulq.y	vf18,vf18,Q
    vmulq.y	vf19,vf19,Q
    vmulq.y	vf24,vf24,Q
    vmulq.y	vf25,vf25,Q
    vmulq.y	vf26,vf26,Q
    vmulq.y	vf27,vf27,Q
    vmuly.xzw	vf21,vf17,vf17
    vmuly.xzw	vf22,vf17,vf18
    vmuly.xzw	vf23,vf17,vf19
    vmuly.xzw	vf28,vf17,vf24
    vmuly.xzw	vf29,vf17,vf25
    vmuly.xzw	vf30,vf17,vf26
    vmuly.xzw	vf31,vf17,vf27
    vsub.xzw	vf17,vf17,vf21
    vsub.xzw	vf18,vf18,vf22
    vsub.xzw	vf19,vf19,vf23
    vsub.xzw	vf24,vf24,vf28
    vsub.xzw	vf25,vf25,vf29
    vsub.xzw	vf26,vf26,vf30
    vsub.xzw	vf27,vf27,vf31
    sqc2	vf17,0x10(%0)
    sqc2	vf18,0x20(%0)
    sqc2	vf19,0x30(%0)
    sqc2	vf24,0x00(%1)
    sqc2	vf25,0x10(%1)
    sqc2	vf26,0x20(%1)
    sqc2	vf27,0x30(%1)
    " : : "r"(m) , "r"(ans) : "memory");

    mi=2;
    for(i=3;i<4;i++) if(fpu_Abs(m[2][mi])<fpu_Abs(m[2][i])) mi=i;
    for(i=0;i<4;i++){
	float tmp,tmp2;
	tmp=m[mi][i], m[mi][i]=m[2][i], m[2][i]=tmp;
	tmp2=(*ans)[mi][i], (*ans)[mi][i]=(*ans)[2][i], (*ans)[2][i]=tmp2;
    }
    asm volatile ("
    lqc2	vf18,0x20(%0)
    lqc2	vf19,0x30(%0)
    vdiv	Q,vf00w,vf18z
    lqc2	vf24,0x00(%1)
    lqc2	vf25,0x10(%1)
    lqc2	vf26,0x20(%1)
    lqc2	vf27,0x30(%1)
    vmulq.z	vf18,vf18,Q
    vmulq.z	vf19,vf19,Q
    vmulq.z	vf24,vf24,Q
    vmulq.z	vf25,vf25,Q
    vmulq.z	vf26,vf26,Q
    vmulq.z	vf27,vf27,Q
    vmulz.xyw	vf22,vf18,vf18
    vmulz.xyw	vf23,vf18,vf19
    vmulz.xyw	vf28,vf18,vf24
    vmulz.xyw	vf29,vf18,vf25
    vmulz.xyw	vf30,vf18,vf26
    vmulz.xyw	vf31,vf18,vf27
    vsub.xyw	vf18,vf18,vf22
    vsub.xyw	vf19,vf19,vf23
    vsub.xyw	vf24,vf24,vf28
    vsub.xyw	vf25,vf25,vf29
    vsub.xyw	vf26,vf26,vf30
    vsub.xyw	vf27,vf27,vf31
    sqc2	vf18,0x20(%0)
    sqc2	vf19,0x30(%0)
    sqc2	vf24,0x00(%1)
    sqc2	vf25,0x10(%1)
    sqc2	vf26,0x20(%1)
    sqc2	vf27,0x30(%1)
    " : : "r"(m) , "r"(ans) : "memory");

    asm volatile ("
    lqc2	vf19,0x30(%0)
    vdiv	Q,vf00w,vf19w
    lqc2	vf24,0x00(%1)
    lqc2	vf25,0x10(%1)
    lqc2	vf26,0x20(%1)
    lqc2	vf27,0x30(%1)
    vmulq.w	vf19,vf19,Q
    vmulq.w	vf24,vf24,Q
    vmulq.w	vf25,vf25,Q
    vmulq.w	vf26,vf26,Q
    vmulq.w	vf27,vf27,Q
    vmulw.xyz	vf23,vf19,vf19
    vmulw.xyz	vf28,vf19,vf24
    vmulw.xyz	vf29,vf19,vf25
    vmulw.xyz	vf30,vf19,vf26
    vmulw.xyz	vf31,vf19,vf27
    vsub.xyz	vf19,vf19,vf23
    vsub.xyz	vf24,vf24,vf28
    vsub.xyz	vf25,vf25,vf29
    vsub.xyz	vf26,vf26,vf30
    vsub.xyz	vf27,vf27,vf31
    sqc2	vf19,0x30(%0)
    sqc2	vf24,0x00(%1)
    sqc2	vf25,0x10(%1)
    sqc2	vf26,0x20(%1)
    sqc2	vf27,0x30(%1)
    " : : "r"(m) , "r"(ans) : "memory");
#endif //BP_ASM
}

/* 拡大縮小がなかったら、こちらでもいいはず */
static inline void FastInverseMatrix(MATRIX *tans,const MATRIX * const tm)
{
#if 1 //BP_ASM
    _sceVu0InversMatrix( tans, tm ) ;
#else
    float (*m)[4][4]=(float (*)[4][4])tm;
    float (*ans)[4][4]=(float (*)[4][4])tans;

    (*ans)[0][0]=(*m)[0][0];
    (*ans)[0][1]=(*m)[1][0];
    (*ans)[0][2]=(*m)[2][0];

    (*ans)[1][0]=(*m)[0][1];
    (*ans)[1][1]=(*m)[1][1];
    (*ans)[1][2]=(*m)[2][1];

    (*ans)[2][0]=(*m)[0][2];
    (*ans)[2][1]=(*m)[1][2];
    (*ans)[2][2]=(*m)[2][2];

    asm volatile ("
    lqc2	vf16,0x30(%1)
    lqc2	vf20,0x00(%0)
    lqc2	vf21,0x10(%0)
    lqc2	vf22,0x20(%0)
    qmtc2	$0,vf17

    vmulax.xyz		ACC,vf20,vf16x
    vmadday.xyz		ACC,vf21,vf16y
    vmaddz.xyz		vf16,vf22,vf16z

    sw		$0,0x0c(%0)
    sw		$0,0x1c(%0)
    sw		$0,0x2c(%0)

    vsub.xyz		vf16,vf17,vf16

    sqc2	vf16,0x30(%0)
    " : : "r"(&((*ans)[0][0])),"r"(&((*m)[0][0])) : "memory" );
#endif //BP_ASM
}
#endif /* KP_XBOX ************** PS2用の専用関数(オリジナル)ここまで***********************  T.Morita 2002.02.04 */

#endif /* _vufpucmb_h_ */
