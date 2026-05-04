/*--------------------------------------------------------------*/
/*	wexebuffer.h												*/
/*					描画実行バッファ							*/
/*--------------------------------------------------------------*/
#ifndef	__WEXEBUFFER_H__
#define	__WEXEBUFFER_H__

#ifdef	__WEXEBUFFER_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------*/
/*	Define														*/
/*--------------------------------------------------------------*/
#define	__EXEBUFFER_CALLFILE_DEBUG__	(FALSE)

#ifndef DEBUG_MODE
#undef	__EXEBUFFER_CALLFILE_DEBUG__
#define	__EXEBUFFER_CALLFILE_DEBUG__	(FALSE)	// 強制的にFALSE
#endif

/*--------------------------------------------------------------*/
/*	Typedef														*/
/*--------------------------------------------------------------*/
typedef	struct	DG_DYNAMIC_VBUFF_UNIT_
{
	LPDIRECT3DVERTEXBUFFER8	vbuff ;	// Vertex Buffer
	DWORD	max ;					// Buffer Size
	DWORD	used_ofs ;				// オフセット

	DWORD	(*callback)(DWORD mess) ;	// CallBack
} DG_DYNAMIC_VBUFF_UNIT ;

typedef	struct	DG_DYNAMIC_IBUFF_UNIT_
{
	LPDIRECT3DINDEXBUFFER8	ibuff ;	// Index Buffer
	DWORD	max ;					// Buffer Size
	DWORD	used_ofs ;				// オフセット

	DWORD	(*callback)(DWORD mess) ;	// CallBack
} DG_DYNAMIC_IBUFF_UNIT ;

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/

	/*----------------------------------------------------------*/
	/*	管理用													*/
	/*----------------------------------------------------------*/
extern void	DG_InitExeBuffer(void) ;
extern void	DG_ReleaseExeBuffer(void) ;

extern void	DG_RunExeBuffer(void) ;
extern void	DG_KickExeBuffer(void) ;
extern void	DG_FlipExeBuffer(void) ;
extern void	DG_ResetExeBuffer(void) ;

extern BOOL	DG_ExeBufferSync(BOOL imm) ;

	/*----------------------------------------------------------*/
	/*	コマンド発行											*/
	/*----------------------------------------------------------*/
extern void	DG_Clear_EB(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
			  D3DCOLOR Color, float Z, DWORD Stencil) ;
extern void DG_SetRenderState_EB(D3DRENDERSTATETYPE State, DWORD Value) ;
extern void DG_SetTransform_EB( D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
extern void DG_SetRenderTarget_EB(IDirect3DSurface8* pRenderTarget,
							IDirect3DSurface8* pNewZStencil) ;
extern void DG_SetTexture_EB(DWORD stage, LPDIRECT3DTEXTURE8 pTexture) ;
extern void DG_SetTextureStageState_EB(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) ;
extern void DG_DrawPrimitiveUP_EB(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
						CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) ;


extern void DG_DrawIndexedPrimitive_EB(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices,
							 UINT StartIndex, UINT PrimitiveCount) ;
extern void DG_DrawIndexedPrimitiveUP_EB(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex,
							   UINT NumVertices, UINT PrimitiveCount,
							   CONST void* pIndexData, D3DFORMAT IndexDataFormat,
							   CONST void* pVertexStreamZeroData,
							   UINT VertexStreamZeroStride) ;
extern void DG_SetStreamSource_EB(UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride) ;
extern void DG_SetIndices_EB(IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex) ;
extern void DG_SetVertexShader_EB(DWORD Handle) ;
extern void DG_SetPixelShader_EB(DWORD Handle) ;
extern void DG_SetVertexShaderConstant_EB(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount) ;
extern void DG_SetPixelShaderConstant_EB(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount) ;
extern void DG_SetViewport_EB(D3DVIEWPORT8 *viewport) ;

extern void DG_StoreImage_EB(void *addr, int which, int x, int y, int w, int h, int mode) ;

extern void DG_ExeBufferCallback(void (*func)(DWORD), DWORD arg) ;

extern void DG_SetLight_EB(DWORD Index, D3DLIGHT8 *pLight) ;

extern void DG_LightEnable_EB(DWORD Index, BOOL bEnable) ;

#if	!__EXEBUFFER_CALLFILE_DEBUG__
extern void DG_DrawPrimitive_EB(D3DPRIMITIVETYPE PrimitiveType,
								UINT StartVertex, UINT PrimitiveCount) ;
#else
#define	DG_DrawPrimitive_EB(p1_, p2_, p3_)	_DG_DrawPrimitive_EB(p1_,p2_,p3_,__FILE__,__LINE__)

extern void _DG_DrawPrimitive_EB(D3DPRIMITIVETYPE PrimitiveType,
								UINT StartVertex, UINT PrimitiveCount,
								char *filename, DWORD line) ;


#endif


#ifdef __cplusplus
}
#endif

#undef EXTERN
#endif	/* !defined(__WEXEBUFFER_H__) */
