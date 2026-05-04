/*--------------------------------------------------------------*/
/*	wvshadertbl.c												*/
/*					Vertex Shader Table							*/
/*--------------------------------------------------------------*/
#define	__WVSHADER_TBL_C__

#include <stdio.h>

#include "xtl.h"
#include "libfs.h"
#include "libgv.h"

#include "libdg.h"
#include "wvshh.h"

#define	__VERTEXSHADER_MODIFY_DEBUG_PRINT__	(TRUE)

/*--------------------------------------------------------------*/
/*	typedef														*/
/*--------------------------------------------------------------*/
typedef	struct	VShCodeTbl_
{
	char	*str ;
	DWORD	size ;

	char	*name ;
} VShCodeTbl ;

/*--------------------------------------------------------------*/
/*	Vertex Shader name 展開										*/
/*--------------------------------------------------------------*/
#undef	VSH_TYPE_EQU
#define	VSH_TYPE_EQU(name_)	#name_ ,
char	*_vsh_name_tbl[MAX_VSHT] =
{
	"NULL",
#include "wvshadertbl.h"
} ;
#undef	VSH_TYPE_EQU

/*--------------------------------------------------------------*/
/*	Vertex Shader Code Table展開								*/
/*--------------------------------------------------------------*/
#define	VSH_TYPE_EQU(name_)	{ (char *)vshh_##name_ , sizeof(vshh_##name_), #name_ },
static	VShCodeTbl	_vsh_code_tbl[MAX_VSHT] =
{
	{ NULL, 0, NULL },
#include "wvshadertbl.h"
} ;
#undef	VSH_TYPE_EQU

/*--------------------------------------------------------------*/
/*	static														*/
/*--------------------------------------------------------------*/
static	BOOL	VShInfoModifyToken(VShInfo *vsh) ;	// 必要があればトークンを修正する

/*--------------------------------------------------------------*/
/*	VShInfoInit		初期化										*/
/*--------------------------------------------------------------*/
void	VShInfoInit(void)
{
	int				i ;
	VShInfo			*vsh ;
	HRESULT			hr ;
	VShCodeTbl		*tbl ;
	ID3DXBuffer		*err_str_buff ;

	/*-- メモリ初期化 ------------------------------------------*/

	memset(_vsh_tbl, 0x00, sizeof(_vsh_tbl)) ;
	/*----------------------------------------------------------*/

	/*-- Vertex Shader作成 -------------------------------------*/

	tbl = _vsh_code_tbl ;
	vsh = _vsh_tbl ;
	for(i=MAX_VSHT; i>0; i--, tbl++, vsh++)
	{
		if( !tbl->str ){ continue ; }

		/*-- Vertex Shader作成 ---------------------------------*/

		err_str_buff = NULL ;
		hr = D3DXAssembleShader(tbl->str, tbl->size - 1, 0, NULL, &vsh->buff, &err_str_buff) ;
		if( !FAILED(hr) )
		{
			/*-- 出力の修正 ------------------------------------*/

#if FALSE
			VShInfoModifyToken(vsh) ;	// 危険なのでやらない
#endif
			/*--------------------------------------------------*/

			vsh->act = TRUE ;
		}
		else
		{
			char	strbuff[256] ;

			sprintf(strbuff, "%s:%d", tbl->name, tbl->size) ;
			dbgErrMessPuts(strbuff, hr) ;	// アセンブル失敗(未対応形式？)

			dbgErrMessPuts(err_str_buff->lpVtbl->GetBufferPointer(err_str_buff), hr) ;
										// アセンブル失敗(未対応形式？)

			dbgErrMessPuts(tbl->str, hr) ;
		}

		if( err_str_buff ){ err_str_buff->lpVtbl->Release(err_str_buff) ; }
		/*------------------------------------------------------*/
	}
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	VShInfoDest		終了処理									*/
/*--------------------------------------------------------------*/
void	VShInfoDest(void)
{
	int				i ;
	VShInfo			*vsh ;

	vsh = _vsh_tbl ;
	for(i=MAX_VSHT; i>0; i--, vsh++)
	{
		/*-- 解放 ----------------------------------------------*/

		if( vsh->buff ){ (vsh->buff)->lpVtbl->Release(vsh->buff) ; }	// Release
		/*------------------------------------------------------*/
	}

	memset(_vsh_tbl, 0x00, sizeof(_vsh_tbl)) ;
}

/*--------------------------------------------------------------*/
/*	VShInfoModifyToken		必要があればコードを修正する		*/
/*--------------------------------------------------------------*/

typedef	struct	ShaderOPCodeTbl_
{
	DWORD	op ;
	int		pnum ;	// パラメータ数
#ifdef DEBUG_MODE
	char	*opname ;
#endif
} ShaderOPCodeTbl ;

#ifndef DEBUG_MODE
#define	SET_OPCODE_PARAM(code_, n_)	{	D3DSIO_ ## code_, n_ }

#else
#define	SET_OPCODE_PARAM(code_, n_)	{	D3DSIO_ ## code_, n_, #code_ }
#endif

static	ShaderOPCodeTbl	_opcode_tbl[] =
{
	SET_OPCODE_PARAM(	NOP,	0 ),
	SET_OPCODE_PARAM(	MOV,	2 ),
	SET_OPCODE_PARAM(	ADD,	3 ),
	SET_OPCODE_PARAM(	SUB,	3 ),
	SET_OPCODE_PARAM(	MAD,	4 ),
	SET_OPCODE_PARAM(	MUL, 	3 ),
    SET_OPCODE_PARAM(	RCP,	2 ),
    SET_OPCODE_PARAM(	RSQ,	2 ),
    SET_OPCODE_PARAM(	DP3,	3 ),
    SET_OPCODE_PARAM(	DP4,	-1 ),
    SET_OPCODE_PARAM(	MIN,	3 ),
	SET_OPCODE_PARAM(	MAX,	3 ),
    SET_OPCODE_PARAM(	SLT,	3 ),
    SET_OPCODE_PARAM(	SGE,	-1 ),
    SET_OPCODE_PARAM(	EXP,	-1 ),
    SET_OPCODE_PARAM(	LOG,	-1 ),
    SET_OPCODE_PARAM(	LIT,	-1 ),
    SET_OPCODE_PARAM(	DST,	-1 ),
    SET_OPCODE_PARAM(	LRP,	-1 ),
    SET_OPCODE_PARAM(	FRC,	-1 ),
    SET_OPCODE_PARAM(	M4x4,	-1 ),
    SET_OPCODE_PARAM(	M4x3,	-1 ),
    SET_OPCODE_PARAM(	M3x4,	-1 ),
    SET_OPCODE_PARAM(	M3x3,	-1 ),
    SET_OPCODE_PARAM(	M3x2,	-1 ),
    SET_OPCODE_PARAM(	TEXCOORD,		-1 ),
    SET_OPCODE_PARAM(	TEXKILL,		-1 ),
    SET_OPCODE_PARAM(	TEX,			-1 ),
    SET_OPCODE_PARAM(	TEXBEM,			-1 ),
    SET_OPCODE_PARAM(	TEXBEML,		-1 ),
    SET_OPCODE_PARAM(	TEXREG2AR,		-1 ),
    SET_OPCODE_PARAM(	TEXREG2GB,		-1 ),
    SET_OPCODE_PARAM(	TEXM3x2PAD,		-1 ),
    SET_OPCODE_PARAM(	TEXM3x2TEX,		-1 ),
    SET_OPCODE_PARAM(	TEXM3x3PAD,		-1 ),
    SET_OPCODE_PARAM(	TEXM3x3TEX,		-1 ),
    SET_OPCODE_PARAM(	TEXM3x3DIFF,	-1 ),
    SET_OPCODE_PARAM(	TEXM3x3SPEC,	-1 ),
    SET_OPCODE_PARAM(	TEXM3x3VSPEC,	-1 ),
    SET_OPCODE_PARAM(	EXPP,			-1 ),
    SET_OPCODE_PARAM(	LOGP,			-1 ),
    SET_OPCODE_PARAM(	CND,			-1 ),
    SET_OPCODE_PARAM(	DEF,			-1 ),
    SET_OPCODE_PARAM(	TEXREG2RGB,		-1 ),
    SET_OPCODE_PARAM(	TEXDP3TEX,		-1 ),
    SET_OPCODE_PARAM(	TEXM3x2DEPTH,	-1 ),
    SET_OPCODE_PARAM(	TEXDP3,			-1 ),
    SET_OPCODE_PARAM(	TEXM3x3,		-1 ),
    SET_OPCODE_PARAM(	TEXDEPTH ,		-1 ),
    SET_OPCODE_PARAM(	CMP,			-1 ),
    SET_OPCODE_PARAM(	BEM,			-1 ),
} ;
#define	MAX_OPCODE_TBL_NUM		(sizeof(_opcode_tbl)/sizeof(_opcode_tbl[0]))

// destination/source parameter register type
//
static	char	*_dstsrc_reg_type_name[] =
{
    "r",				// (0<<D3DSP_REGTYPE_SHIFT) Temporary Register File
    "v",				// (1<<D3DSP_REGTYPE_SHIFT) Input Register File
    "c",				// (2<<D3DSP_REGTYPE_SHIFT) Constant Register File
    "?D3DSPR_ADDR",		// (3<<D3DSP_REGTYPE_SHIFT) Address Register(VS)
						//							Texture Register File (PS)
    "?D3DSPR_RASTOUT",	// (4<<D3DSP_REGTYPE_SHIFT) Rasterizer Register File
    "oD",				// (5<<D3DSP_REGTYPE_SHIFT) Attribute Output Register File
    "oT",				// (6<<D3DSP_REGTYPE_SHIFT) Texture Coordinate Output Register File
} ;

static	ShaderOPCodeTbl	*GetShaderOPCodeTbl(DWORD op) ;

static	BOOL	VShInfoModifyToken(VShInfo *vsh)
{
	DWORD			*code_ptr ;
	DWORD			code ;
	DWORD			op ;
	BOOL			modify ;
	ShaderOPCodeTbl	*opcode_tbl ;
	DWORD			tex_max ;

	modify  = TRUE ;
	tex_max = DG_WinApp.multi_tex_max ;	// 同時に使用できる最大Texture

	/*-- コード解析 ------------------------------------------------*/

#if __VERTEXSHADER_MODIFY_DEBUG_PRINT__
	printf("\n\n/*-- Start --*/\n") ;
#endif

	code_ptr = (vsh->buff)->lpVtbl->GetBufferPointer(vsh->buff) ;
	code_ptr++ ;	// 最初はVersion
	while( TRUE )
	{
		code = *code_ptr ;
		op   = (code & D3DSI_OPCODE_MASK) ;

		/*-- 終了コード --------------------------------------------*/

		if( op == D3DSIO_END ){ break ; }
		/*----------------------------------------------------------*/

		/*-- コメント ----------------------------------------------*/

		if( op == D3DSIO_COMMENT )
		{
			code_ptr += ((code & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT) + 1 ;
			continue ;	// 次へ。
		}

		if( op == D3DSIO_NOP )
		{
			code_ptr++ ;
			continue ;
		}
		/*----------------------------------------------------------*/

		/*-- パラメータ解析 ----------------------------------------*/

		opcode_tbl = GetShaderOPCodeTbl(op) ;
		if( !opcode_tbl )
		{
			printf("[ WARNING ]Unknown Operand :%04X!!\n", op) ;
			ASSERT(0) ;
		}
		else
		{
			int		i ;
			int		pnum ;
			DWORD	pcode ;
			DWORD	reg_id ;

			pnum = opcode_tbl->pnum ;
			ASSERT( pnum >= 0 ) ;
			if( pnum >= 0 )
			{
				/*-- DEBUG出力 -------------------------------------*/
#if __VERTEXSHADER_MODIFY_DEBUG_PRINT__
				if( code & D3DSI_COISSUE ){ printf("[COISSUE]") ; }
#ifdef DEBUG_MODE
				printf("\t%s ", opcode_tbl->opname) ;
#endif

				for(i=0; i<pnum; i++)
				{
					pcode = code_ptr[1 + i] ;

					printf("%s%d ",
					_dstsrc_reg_type_name[(pcode & D3DSP_REGTYPE_MASK)
								>> D3DSP_REGTYPE_SHIFT]
								,pcode & 0xfff) ;
				}
				printf("\n") ;
#endif
				/*--------------------------------------------------*/

				/*-- Texture座標出力修正 ---------------------------*/

				pcode = code_ptr[1] ;	// Dstのみ調べる
				if( (pcode & D3DSP_REGTYPE_MASK) == (6<<D3DSP_REGTYPE_SHIFT) )	// oT(n)
				{
					reg_id = pcode & 0xfff ;	// Register番号

					/*-- 使用可能最大数判定 ------------------------*/

					if( reg_id >= tex_max )
					{
						printf("[ Erase Dest:oT%d ]\n", reg_id) ;

						/*-- NOP化 ---------------------------------*/
						
						code = 0x00000000 ;
						op   = D3DSIO_NOP ;
						memset(code_ptr, 0x00, sizeof(DWORD) * (pnum+1)) ;

						*(code_ptr + (pnum + 1)) &= ~D3DSI_COISSUE ;
												// 次命令が並列動作だった場合の保険
						/*------------------------------------------*/
					}
					/*----------------------------------------------*/
				}
				/*--------------------------------------------------*/

				code_ptr += pnum ;	// パラメータ分進める
			}
		}
		/*----------------------------------------------------------*/
		code_ptr++ ;
	}

#if __VERTEXSHADER_MODIFY_DEBUG_PRINT__
	printf("/*-- End --*/\n") ;
#endif
	/*--------------------------------------------------------------*/

}

static	ShaderOPCodeTbl	*GetShaderOPCodeTbl(DWORD op)
{
	int				i ;
	ShaderOPCodeTbl	*tbl ;

	tbl = _opcode_tbl ;
	for(i=MAX_OPCODE_TBL_NUM; i>0; i--, tbl++)
	{
		if( tbl->op == op ){ break ; }
	}
	if( !i ){ return(NULL) ; }	// 検索失敗

	return(tbl) ;
}

/*-- End Of File --*/
