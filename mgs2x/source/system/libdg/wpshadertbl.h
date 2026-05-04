/*--------------------------------------------------------------*/
/*	wpshadertbl.h												*/
/*					Pixel Shader Table							*/
/*--------------------------------------------------------------*/
#ifndef	__WPSHADER_TBL_H__
#define	__WPSHADER_TBL_H__

#ifdef	__WPSHADER_TBL_C__
#define	EXTERN
#else
#define	EXTERN	extern
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------*/
/*	typedef														*/
/*--------------------------------------------------------------*/
typedef	struct	PShInfo_
{
	DWORD			act ;		// 使用可能フラグ
	LPD3DXBUFFER	buff ;		// 頂点シェーダ機能トークン配列バッファ
} PShInfo ;

/*--------------------------------------------------------------*/
/*	Pixel Shader Type											*/
/*--------------------------------------------------------------*/
#undef	PSH_TYPE_EQU
#define	PSH_TYPE_EQU(name_)	PSHT_ ## name_,
enum
{
	PSHT_NULL,
#include "wpshadertbl.h"
	MAX_PSHT,
} ;
#undef	PSH_TYPE_EQU

/*--------------------------------------------------------------*/
/*	Pixel Shader Table											*/
/*--------------------------------------------------------------*/
EXTERN	PShInfo	_psh_tbl[MAX_PSHT] ;

/*--------------------------------------------------------------*/
/*	Pixel Shader Name Table									*/
/*--------------------------------------------------------------*/
extern	char	*_psh_name_tbl[MAX_PSHT] ;

/*--------------------------------------------------------------*/
/*	マクロ														*/
/*--------------------------------------------------------------*/
#define	checkPixelShader(type_)		(_psh_tbl[(type_)].act)
#define	getPixelShaderBuffer(type_)	(_psh_tbl[(type_)].buff)
#define	getPixelShaderInfo(type_)		(&_psh_tbl[(type_)])

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/
EXTERN	void	PShInfoInit(void) ;
EXTERN	void	PShInfoDest(void) ;


#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* !defined(__WPSHADER_TBL_H__) */

/*--------------------------------------------------------------*/
/*	Pixel Shader Type Equ										*/
/*--------------------------------------------------------------*/
#ifdef	PSH_TYPE_EQU

	/*--			file		--*/
	PSH_TYPE_EQU(	shdwmap0	)
	PSH_TYPE_EQU(	mtex_1tex	)
	PSH_TYPE_EQU(	wmtex_emap	)
	PSH_TYPE_EQU(	wmtex_bmap	)
	PSH_TYPE_EQU(	mtex_gray	)
	PSH_TYPE_EQU(	wmtex_smap	)
	PSH_TYPE_EQU(	wmtex_emap2	)
	PSH_TYPE_EQU(	wmtex_emap3	)
	PSH_TYPE_EQU(	wmtex_emap4	)
	PSH_TYPE_EQU(	ps_irmode	)
	PSH_TYPE_EQU(	wshdwmap0_1tex	)

#endif /* defined(PSH_TYPE_EQU) */


/*-- End Of File --*/
