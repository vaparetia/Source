/*
	see_inline.h
	ＳＳＥを使用したＬＩＢＤＧ用インラインライブラリ

	2002/05/01	K.Takabe
	$Id: sse_inline.h,v 1.6 2002/12/24 07:16:35 takaki Exp $
*/

#ifndef __SSE_INLINE_H__
#define __SSE_INLINE_H__

#ifdef _USE_SSE
/* XGMathとどっちが速いんだろう・・・？ */
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

static inline void OuterProduct( FVECTOR *res, FVECTOR *v0, FVECTOR *v1 )
{
	__asm {
		mov		edx, v0
		mov		ecx, v1
		movups	xmm0, [ edx ]
		movups	xmm1, [ ecx ]
		movaps	xmm2, xmm0
		movaps	xmm3, xmm1
		shufps	xmm0, xmm0, 11001001b
		shufps	xmm1, xmm1, 11010010b
		shufps	xmm2, xmm2, 11010010b
		shufps	xmm3, xmm3, 11001001b
		mulps	xmm0, xmm1
		mulps	xmm2, xmm3
		subps	xmm0, xmm2
		mov		edx, res
		movups	[ edx ], xmm0
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

		movups	xmm0, [edx+30h]
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

/*----------------------------------------------------------------*/

static inline void _SetMatrix( FMATRIX *m )
{
	__asm {
        mov		edx, m
		movups	xmm4, [edx+00h]
		movups	xmm5, [edx+10h]
		movups	xmm6, [edx+20h]
		movups	xmm7, [edx+30h]
	}
}

static inline void _GetMatrix( FMATRIX *m )
{
	__asm {
        mov		edx, m
		movups	[edx+00h], xmm4
		movups	[edx+10h], xmm5
		movups	[edx+20h], xmm6
		movups	[edx+30h], xmm7
	}
}

static inline void _RotTrans( FVECTOR *res, FVECTOR *v )
{
	__asm {
        mov		ecx, v
        mov		eax, res
		movups	xmm0, [ecx+00h]
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
		movups	[eax+00h], xmm3 
	}
}

static inline void _RotVector( FVECTOR *res, FVECTOR *v )
{
	__asm {
        mov		ecx, v
        mov		eax, res
		movups	xmm0, [ecx+00h]
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		shufps	xmm0, xmm0, 00000000b
		mulps	xmm0, xmm4
		shufps	xmm1, xmm1, 01010101b
		mulps	xmm1, xmm5
		shufps	xmm2, xmm2, 10101010b
		mulps	xmm2, xmm6
		addps	xmm1, xmm0
		addps	xmm2, xmm1
		movups	[eax+00h], xmm2 
	}
}

#else

#define	ApplyMatrix(p1_, p2_, p3_)	_sceVu0ApplyMatrix(p1_, p2_, p3_)

#if 0//BP_ASM
#define	ApplyMatrix2(p1_, p2_, p3_)	D3DXVec3Transform(p1_, p3_, p2_)
#else
#define	ApplyMatrix2(p1_, p2_, p3_)	BP_Mat_TransformVec3(p1_, p2_, p3_)
#endif

#define	OuterProduct(p1_, p2_, p3_)	_sceVu0OuterProduct(p1_, p2_, p3_)
#define	MulMatrix(p1_, p2_, p3_)	_sceVu0MulMatrix(p1_, p2_, p3_)

extern	FMATRIX	DG_SSE_CurrentMatrix ;

#define	_SetMatrix(m_)		{ DG_SSE_CurrentMatrix = *(FMATRIX *)(m_) ; }
#define	_GetMatrix(m_)		{ *(FMATRIX *)(m_) = DG_SSE_CurrentMatrix ; }

#if 0//BP_ASM
#define	_RotTrans(res_, v_)	{ D3DXVec3Transform((D3DXVECTOR4 *)(res_), (D3DVECTOR *)(v_), (D3DMATRIX *)&DG_SSE_CurrentMatrix) ; }
#else
#define	_RotTrans(res_, v_)	{ BP_Mat_TransformVec3(res_, &DG_SSE_CurrentMatrix, v_); }
#endif

static inline void _RotVector(FVECTOR *res, FVECTOR *v )
{
	float	vw ;

	vw = v->vw ;
#if 0//BP_ASM
	D3DXVec3TransformNormal((D3DVECTOR *)res,
							(D3DVECTOR *)v,
							(D3DMATRIX *)&DG_SSE_CurrentMatrix) ;
#else
   BP_Mat_RotateVec3(res, &DG_SSE_CurrentMatrix, v);
#endif
	res->vw = vw ;
}
#endif

#endif

