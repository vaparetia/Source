/*--------------------------------------------------------------*/
/*	wvshadertbl.h												*/
/*					Vertex Shader Table							*/
/*--------------------------------------------------------------*/
#ifndef	__WVSHADER_TBL_H__
#define	__WVSHADER_TBL_H__

#ifdef	__WVSHADER_TBL_C__
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
typedef	struct	VShInfo_
{
	DWORD			act ;		// 使用可能フラグ
	LPD3DXBUFFER	buff ;		// 頂点シェーダ機能トークン配列バッファ
} VShInfo ;

/*--------------------------------------------------------------*/
/*	Vertex Shader Type											*/
/*--------------------------------------------------------------*/
#undef	VSH_TYPE_EQU
#define	VSH_TYPE_EQU(name_)	VSHT_ ## name_,
enum
{
	VSHT_NULL,
#include "wvshadertbl.h"
	MAX_VSHT,
} ;
#undef	VSH_TYPE_EQU

/*--------------------------------------------------------------*/
/*	Vertex Shader Table											*/
/*--------------------------------------------------------------*/
EXTERN	VShInfo	_vsh_tbl[MAX_VSHT] ;

/*--------------------------------------------------------------*/
/*	Vertex Shader Name Table									*/
/*--------------------------------------------------------------*/
extern	char	*_vsh_name_tbl[MAX_VSHT] ;

/*--------------------------------------------------------------*/
/*	マクロ														*/
/*--------------------------------------------------------------*/
#define	checkVertexShader(type_)		(_vsh_tbl[(type_)].act)
#define	getVertexShaderBuffer(type_)	(_vsh_tbl[(type_)].buff)
#define	getVertexShaderInfo(type_)		(&_vsh_tbl[(type_)])

/*--------------------------------------------------------------*/
/*	関数プロトタイプ宣言										*/
/*--------------------------------------------------------------*/
EXTERN	void	VShInfoInit(void) ;
EXTERN	void	VShInfoDest(void) ;


#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* !defined(__WVSHADER_TBL_H__) */

/*--------------------------------------------------------------*/
/*	Vertex Shader Type Equ										*/
/*--------------------------------------------------------------*/
#ifdef	VSH_TYPE_EQU

	/*--			file		--*/
	VSH_TYPE_EQU(	wkms_sl		)
	VSH_TYPE_EQU(	wkms_sp		)
	VSH_TYPE_EQU(	wkms_ml		)
	VSH_TYPE_EQU(	wkms_emap	)
	VSH_TYPE_EQU(	wkms_smap	)
	VSH_TYPE_EQU(	wevm		)
	VSH_TYPE_EQU(	wevm_emap	)
	VSH_TYPE_EQU(	wevm_bmap	)
	VSH_TYPE_EQU(	wprm2poly	)
	VSH_TYPE_EQU(	wprm2sprt	)
	VSH_TYPE_EQU(	wcomdl		)
	VSH_TYPE_EQU(	woptcmf00	)
	VSH_TYPE_EQU(	wpatch		)
	VSH_TYPE_EQU(	wpatch_emap	)
	VSH_TYPE_EQU(	wpatch2_emap)
	VSH_TYPE_EQU(	dmapk_2d	)
	VSH_TYPE_EQU(	dmapk_2d4	)
	VSH_TYPE_EQU(	wshdw_mk	)
	VSH_TYPE_EQU(	wshdw_wt	)
	VSH_TYPE_EQU(	vs_radar	)
	VSH_TYPE_EQU(	wvs_radar2	)
	VSH_TYPE_EQU(	wkms_mlvc	)
	VSH_TYPE_EQU(	wkms_emapvc	)
	VSH_TYPE_EQU(	wkms_smapvc	)
	VSH_TYPE_EQU(	wkms_ml_1tex )
	VSH_TYPE_EQU(	wkms_emap_1tex )
	VSH_TYPE_EQU(	wkms_smap_1tex )
	VSH_TYPE_EQU(	wkms_mlvc_1tex )
	VSH_TYPE_EQU(	wkms_emapvc_1tex )
	VSH_TYPE_EQU(	wkms_smapvc_1tex )
	VSH_TYPE_EQU(	wevm_1tex		)
	VSH_TYPE_EQU(	wevm_emap_1tex	)
	VSH_TYPE_EQU(	wevm_bmap_1tex	)
	VSH_TYPE_EQU(	wshdw_wt_1tex	)
	VSH_TYPE_EQU(	wshdw_wt_fvf	)
	VSH_TYPE_EQU(	wshdw_wt_fvf_1tex )
	VSH_TYPE_EQU(	woptcmf00_fvf )
	VSH_TYPE_EQU(	wkms_ml_kmss )
	VSH_TYPE_EQU(	wkms_emap_kmss )
	VSH_TYPE_EQU(	wkms_smap_kmss )
	VSH_TYPE_EQU(	wkms_mlvc_kmss )
	VSH_TYPE_EQU(	wkms_emapvc_kmss )
	VSH_TYPE_EQU(	wkms_smapvc_kmss )

#endif /* defined(VSH_TYPE_EQU) */


/*-- End Of File --*/
