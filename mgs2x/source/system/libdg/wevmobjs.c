/*
	xevmobjs.c
	マルチウェイトエンベロープオブジェクト処理ルーチン

	2002/03/15 K.Takabe
	$Id: wevmobjs.c,v 1.30 2003/01/08 10:39:27 takaki Exp $

*/
/*

	DG_EVMOBJ		*DG_MakeEvmObj( def, flag, chanl )
	EVM_DEF	*def ;		モデルデータ
	int		flag ;		処理フラグ
	int		chanl ;		描画チャンネル

		あるモデルデータに対応する、オブジェクトを作成する

	void		DG_FreeEvmObjs( objs )
	DG_EVMOBJ	*objs ;		物体ハンドラ

		物体ハンドラと、そのパケットメモリを解放する

	void DG_WriteEvmMdlPaketUV( int tri_code, EVM_PACK *pack )
	int			tri_code ;	使用するＴＲＩファイルのＩＤ
	EVM_PACK	*pack ;		補正するオブジェクトパケット

		テクスチャからＵＶ値を補正する

*/


#ifdef _XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

#include	"shader.h"

#include	"sse_inline.h"

/*----------------------------------------------------------------*/

static	void	ChainEvmObj( DG_EVMOBJ *evmobj ) ;
static	void	ChainEvmObjNIdx( DG_EVMOBJ *evmobj ) ;		// Index非対応版
static	void	ChainEvmObjNVS( DG_EVMOBJ *evmobj ) ;		// VertexShader非対応版

static	void	ChainEvmObjBackBumpNVS( DG_EVMOBJ *evmobj ) ;		// VertexShader非対応版

static	void	*_chain_obj_func ;

/*----------------------------------------------------------------*/

static	void	CalcEVM2FVF(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat);
static	void	CalcEVM2FVFLarge(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat);
static	void	CalcEVM2FVF_UV1(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat);
static	void	CalcEVM2FVFLarge_UV1(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat);

//#define	EVM_VERTEXBUFFER_POOL		(D3DPOOL_DEFAULT)	// 使用POOL
#define	EVM_VERTEXBUFFER_POOL		(D3DPOOL_MANAGED)	// 使用POOL
#define	EVM_VERTEXBUFFER_LOCKFLAG	(D3DLOCK_NOSYSLOCK)	// LOCK FLAG
#define	EVM_INDEXBUFFER_LOCKFLAG	(D3DLOCK_NOSYSLOCK)	// LOCK FLAG

static DG_VERTEXSHADER	DG_EvmVertexShader[3];
extern DG_PIXELSHADER	DG_MultiTexPixelShader[DG_MULTITEX_MAXNUM] ;

extern unsigned char VERTEX_SHADER_evm[];
extern unsigned char VERTEX_SHADER_evm_emap[];
extern unsigned char VERTEX_SHADER_evm_bmap[];
void DG_InitEvmVertexShader(void)
{
	DWORD dwEvmDecl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),	/* pos */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),	/* norm */
		D3DVSD_REG(7,  D3DVSDT_SHORT4),	/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_SHORT4),	/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_SHORT4),	/* uv2 */
		D3DVSD_REG(11, D3DVSDT_SHORT4), /* blend */
		D3DVSD_REG(12, D3DVSDT_SHORT4), /* index */
		D3DVSD_END()
	};

	/* 頂点フォーマットを生成する */
	if( DG_CheckEvmUseVertexShader() )
	{
		if( DG_GetMultiTexMax() >= 3 )
		{
			/*-- 3Tex --*/
			DG_MakeVertexShader( &DG_EvmVertexShader[0], VSHT_wevm, dwEvmDecl );
			DG_MakeVertexShader( &DG_EvmVertexShader[1], VSHT_wevm_emap, dwEvmDecl );
			DG_MakeVertexShader( &DG_EvmVertexShader[2], VSHT_wevm_bmap, dwEvmDecl );
		}
		else
		{
			/*-- 1Tex --*/
			DG_MakeVertexShader( &DG_EvmVertexShader[0], VSHT_wevm_1tex, dwEvmDecl );
#if FALSE
			DG_MakeVertexShader( &DG_EvmVertexShader[1], VSHT_wevm_emap_1tex, dwEvmDecl );
			DG_MakeVertexShader( &DG_EvmVertexShader[2], VSHT_wevm_bmap_1tex, dwEvmDecl );
#else
			DG_MakeVertexShader( &DG_EvmVertexShader[1], VSHT_wevm_1tex, dwEvmDecl );
			DG_MakeVertexShader( &DG_EvmVertexShader[2], VSHT_wevm_1tex, dwEvmDecl );
#endif
		}
	}

	/* 描画関数選択 */
	if( DG_CheckEvmUseVertexShader() )
	{
		if( DG_CheckIndexPrimitiveUseable() ){ _chain_obj_func = ChainEvmObj ; }
		else{ _chain_obj_func = ChainEvmObjNIdx ; }

	}
	else
	{
		/* VertexShader非対応版 */
		_chain_obj_func = ChainEvmObjNVS ;
	}
}

void DG_ReleaseEvmVertexShader(void)
{
	DG_KillVertexShader( &DG_EvmVertexShader[0] );
	DG_KillVertexShader( &DG_EvmVertexShader[1] );
	DG_KillVertexShader( &DG_EvmVertexShader[2] );
}
/*----------------------------------------------------------------*/
	/*
		ＶＵ１ワーク初期化パラメータ関連
	*/

/*----------------------------------------------------------------*/
	/*
		各種パケット定義
	*/


/*----------------------------------------------------------------*/
	/*
		スクラッチパッド上ワークデータ構造体
	*/
typedef struct _obj_list {
	void	*obj ;
	int		tri_code ;
} OBJ_LIST ;

typedef struct _scrpad_work {
	FMATRIX		pers_mat ;
	FMATRIX		eye_pers ;
	FMATRIX		eye_xpers ;
	FMATRIX		eye_inv ;
	FMATRIX		xpers ;
	FVECTOR		scale ;
	FMATRIX		world ;				/* オブジェクトマトリクス */
	FMATRIX		screen ;			/* カメラへの透視変換用マトリクス */
	FMATRIX		light[2] ;			/* ライトマトリクス */
	FMATRIX		support_mat[2] ;	/* 拡張用 */
	FMATRIX		tmp_mat ;			/* 一時使用マトリクス */
	FMATRIX		ir_light_mat[2] ;
	FVECTOR		tmp_vec[4] ;		/* 一時使用ベクトル */
	u_long		matrix_list ;		/* 転送済みマトリクス情報（８ビット×８個分） */
	int			packet_clock ;		/* ダブルバッファ切り替え用クロック */
	int			bound_mode ;		/* バウンディング結果記憶用 */
	short		vu_prog_list[8] ;
	int			count ;					/* デバッグ用 */
	void		*last_matrix_addr[8] ;	/* 転送マトリクスアドレス記憶用 */
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	mulwt_model ;
#endif
	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
extern u_long128	DG_LocalMemory[1024] ;

//#define DG_StartSprToMem	UTL_StartSprToMem
//#define DG_EndSprToMem	UTL_EndSprToMem
/*----------------------------------------------------------------*/
	/*
		各種サブルーチン
	*/

/* バウンディングチェック */
static int BoundCheck( FMATRIX *mat, float *bound )
{
	int		and_flag, or_flag, flag ;
	FVECTOR		verts, tmp_v ;
	float		w ;
	int			i ;

	and_flag = 0xffffffff ;
	or_flag = 0 ;

	for ( i = 8 ; i > 0 ; i-- ){
		verts.vx = ( i & 1 ) ? bound[0] : bound[3] ;
		verts.vy = ( i & 2 ) ? bound[1] : bound[4] ;
		verts.vz = ( i & 4 ) ? bound[2] : bound[5] ;
		verts.vw = 1.0F ;
		_sceVu0ApplyMatrix( &tmp_v, mat, &verts );
		w = DG_FABS( tmp_v.vw );

		/* クリップチェック */
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
		if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
		and_flag &= flag ;
		or_flag |= flag ;

	}
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */

	return ( 0 );	/* 完全画面内 */

}

FVECTOR bound_max, bound_min ;
/* 各マトリクスでのバウンディングから最大バウンディングを求める */
static void MakeMaxBound( FMATRIX *mat, EVM_SKEL *skel, int num )
{
	static FVECTOR	def_max = {-FLOAT_MAX,-FLOAT_MAX,-FLOAT_MAX,1.0f}, def_min = {FLOAT_MAX,FLOAT_MAX,FLOAT_MAX,1.0f};
	FVECTOR			vec ;
	int				i ;

	bound_max = def_max ;
	bound_min = def_min ;
	while ( num-- ){
		for ( i = 0 ; i < 8 ; i++ ){
			vec.vx = ( i & 1 ) ? skel->lx : skel->ux ;
			vec.vy = ( i & 2 ) ? skel->ly : skel->uy ;
			vec.vz = ( i & 4 ) ? skel->lz : skel->uz ;
			vec.vw = 1.0f ;
			_sceVu0ApplyMatrix( &vec, mat, &vec );
			bound_max.vx = DG_MAX( bound_max.vx, vec.vx );
			bound_max.vy = DG_MAX( bound_max.vy, vec.vy );
			bound_max.vz = DG_MAX( bound_max.vz, vec.vz );
			bound_min.vx = DG_MIN( bound_min.vx, vec.vx );
			bound_min.vy = DG_MIN( bound_min.vy, vec.vy );
			bound_min.vz = DG_MIN( bound_min.vz, vec.vz );
		}
		mat++ ;
		skel++ ;
	}
}
static int BoundCheck2( FMATRIX *mat )
{
	int		and_flag, or_flag, flag ;
	FVECTOR		verts, tmp_v ;
	float		w ;
	int			i ;

	and_flag = 0xffffffff ;
	or_flag = 0 ;

	for ( i = 8 ; i > 0 ; i-- ){
		verts.vx = ( i & 1 ) ? bound_max.vx : bound_min.vx ;
		verts.vy = ( i & 2 ) ? bound_max.vy : bound_min.vy ;
		verts.vz = ( i & 4 ) ? bound_max.vz : bound_min.vz ;
		verts.vw = 1.0F ;
		_sceVu0ApplyMatrix( &tmp_v, mat, &verts );
		w = DG_FABS( tmp_v.vw );

		/* クリップチェック */
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
		if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
		and_flag &= flag ;
		or_flag |= flag ;

	}
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */

	return ( 0 );	/* 完全画面内 */

}


/*----------------------------------------------------------------*/
	/*
		パケット初期化関連
	*/

/* デフォルトパケット初期化ルーチン */

/* 初期化用パケットの設定*/


/* マトリクス転送の設定 */
static inline int SetEvmMatrix( u_long matrix_list, FMATRIX *mat )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	int		i, num, matrix_trans_flag = 0 ;
	void		**last_matrix_addr, *addr ;

	/* 前回と違う場合のみ転送するようにする */
	last_matrix_addr = scrpad->last_matrix_addr ;
	for ( i = 0 ; i < 8 ; i++ ){
		num = (int)matrix_list & 0xff ;
		addr = mat + num ;
#if 1
		if ( num != 0xff && (*last_matrix_addr) != addr ){
			DG_SetVertexShaderConstant(CV_WORLD0 + i * 4, addr, 4 );
			*last_matrix_addr = addr ;
			matrix_trans_flag = 1 ;
		}
#else
		DG_SetVertexShaderConstant( CV_WORLD0 + i * 4, addr, 4 );
#endif
		matrix_list >>= 8 ;
		last_matrix_addr++ ;
	}
	return ( matrix_trans_flag );
}

/*----------------------------------------------------------------*/
static void DG_ChanlRenderState(DG_CHANL *cp)
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 1.0f/16, 1.0f/32767.0f},
	};

	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckEvmVertexBufferSoftProcessing()) ;

	/* Shader 設定  */
	if( DG_CheckEvmUseVertexShader() )
	{
		/*-- Vertex Shader --*/
		DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
		DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
		DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
		DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);

		/*-- Software Vertex Shaderの場合の処理 --*/
		//if( DG_CheckEvmVertexBufferSoftProcessing() )
		{
			DG_SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0) ;
			DG_SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, 0) ;
			DG_SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 0) ;
			DG_SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, 0) ;
		}

		/*-- Fog 初期化 --*/
		{
			float fog_start = 0.0f;
			float fog_end   = 1.0f;
			DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
			DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
		}
	}
	else
	{
		DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
		DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;

		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE) ;
	}
}

/* 各種セットアップルーチン */
static void EvmobjSetup( DG_EVMOBJ *evmobj )
{
	static FVECTOR	model_scale = {1.0f/4096.0f, 1.0f/255.0f, 1.0f/16, 1.0f/32767.0f};
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;

	/* オブジェクト描画パケット初期化関連 */
	scrpad->matrix_list = 0xffffffffffffffffui64 ;
	scrpad->last_matrix_addr[0] = NULL ;
	scrpad->last_matrix_addr[1] = NULL ;
	scrpad->last_matrix_addr[2] = NULL ;
	scrpad->last_matrix_addr[3] = NULL ;
	scrpad->last_matrix_addr[4] = NULL ;
	scrpad->last_matrix_addr[5] = NULL ;
	scrpad->last_matrix_addr[6] = NULL ;
	scrpad->last_matrix_addr[7] = NULL ;

	/* 光源計算 */
	scrpad->light[0] = evmobj->light[0] ;
	scrpad->light[1] = evmobj->light[1] ;
	/* 赤外線反応 */
	if ( DG_DisplayStatus & DG_STATE_IR_MODE ){
		if ( evmobj->flag & DG_EVMOBJ_IRREACTION ){
			scrpad->light[0] = scrpad->ir_light_mat[0];
			scrpad->light[1] = scrpad->ir_light_mat[1];
		}
	}
	DG_SetVertexShaderConstant(CV_LIGHTVEC, &scrpad->light[0], 4);
	DG_SetVertexShaderConstant(CV_LIGHTCOL, &scrpad->light[1], 4);
	{/* バンプマップ用ベクトル設定 */
		static FVECTOR	nomo_const = { 0.30f, 0.59f, 0.11f, 1.0f };
		FVECTOR		bump_light, light_force ;
#if 0
		float		lf0, lf1, lf2 ;	/* 光源色強さ */
		lf0 = scrpad->light[1].m[0][0] * 0.30f + scrpad->light[1].m[0][1] * 0.59f + scrpad->light[1].m[0][2] * 0.11f ;
		lf1 = scrpad->light[1].m[1][0] * 0.30f + scrpad->light[1].m[1][1] * 0.59f + scrpad->light[1].m[1][2] * 0.11f ;
		lf2 = scrpad->light[1].m[2][0] * 0.30f + scrpad->light[1].m[2][1] * 0.59f + scrpad->light[1].m[2][2] * 0.11f ;
		bump_light.vx = scrpad->light[0].m[0][0] * lf0 +
		  scrpad->light[0].m[0][1] * lf1 +
			scrpad->light[0].m[0][2] * lf2 ;
		bump_light.vy = scrpad->light[0].m[1][0] * lf0 +
		  scrpad->light[0].m[1][1] * lf1 +
			scrpad->light[0].m[1][2] * lf2 ;
		bump_light.vz = scrpad->light[0].m[2][0] * lf0 +
		  scrpad->light[0].m[2][1] * lf1 +
			scrpad->light[0].m[2][2] * lf2 ;
#endif
		/* 各色の輝度を算出 */
		light_force.vx = _sceVu0InnerProduct( scrpad->light[1].m[0], &nomo_const );
		light_force.vy = _sceVu0InnerProduct( scrpad->light[1].m[1], &nomo_const );
		light_force.vz = _sceVu0InnerProduct( scrpad->light[1].m[2], &nomo_const );
		/* 各光源ベクトルに輝度を乗算してベクトルの和を求める */
		bump_light.vx = _sceVu0InnerProduct( &scrpad->light[0].m[0], &light_force );
		bump_light.vy = _sceVu0InnerProduct( &scrpad->light[0].m[1], &light_force );
		bump_light.vz = _sceVu0InnerProduct( &scrpad->light[0].m[2], &light_force );
		/* 合成ベクトルの正規化 */
		_sceVu0Normalize( &bump_light, &bump_light );
		/* シェーダーに登録 */
		DG_SetVertexShaderConstant(CV_CONST0, &bump_light, 1);
	}

	/* モデルタイプチェック */
	if ( evmobj->def->type & DG_EVMTYPE_LARGE ){
		model_scale.vz = 1.0f ;
	} else {
		model_scale.vz = 1.0f/16.0f ;
	}
	DG_SetVertexShaderConstant(CV_SCALE, &model_scale, 1);
	//DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	DG_SetVertexShaderConstant(CV_EYEPERS, &scrpad->eye_xpers, 4);
	DG_SetVertexShaderConstant(CV_EYE_INV, &scrpad->eye_inv, 4);
}

/* 各種セットアップルーチン(VertexShader非対応) */
static void EvmobjSetupNVS( DG_EVMOBJ *evmobj )
{
	static FVECTOR	model_scale = {1.0f/4096.0f, 1.0f/255.0f, 1.0f/16, 1.0f/32767.0f};
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;

	/* オブジェクト描画パケット初期化関連 */
	scrpad->matrix_list = 0xffffffffffffffffui64 ;
	scrpad->last_matrix_addr[0] = NULL ;
	scrpad->last_matrix_addr[1] = NULL ;
	scrpad->last_matrix_addr[2] = NULL ;
	scrpad->last_matrix_addr[3] = NULL ;
	scrpad->last_matrix_addr[4] = NULL ;
	scrpad->last_matrix_addr[5] = NULL ;
	scrpad->last_matrix_addr[6] = NULL ;
	scrpad->last_matrix_addr[7] = NULL ;

	/* 光源計算 */
	DG_SetRenderState(D3DRS_LIGHTING, TRUE) ;

	scrpad->light[0] = evmobj->light[0] ;
	scrpad->light[1] = evmobj->light[1] ;
	/* 赤外線反応 */
	if ( DG_DisplayStatus & DG_STATE_IR_MODE ){
		if ( evmobj->flag & DG_EVMOBJ_IRREACTION ){
			scrpad->light[0] = scrpad->ir_light_mat[0];
			scrpad->light[1] = scrpad->ir_light_mat[1];
		}
	}
	DG_SetLightMatrix2Direct3D(&scrpad->light[0], &scrpad->light[1]) ;

#if FALSE
	{/* バンプマップ用ベクトル設定 */
		static FVECTOR	nomo_const = { 0.30f, 0.59f, 0.11f, 1.0f };
		FVECTOR		bump_light, light_force ;
#if 0
		float		lf0, lf1, lf2 ;	/* 光源色強さ */
		lf0 = scrpad->light[1].m[0][0] * 0.30f + scrpad->light[1].m[0][1] * 0.59f + scrpad->light[1].m[0][2] * 0.11f ;
		lf1 = scrpad->light[1].m[1][0] * 0.30f + scrpad->light[1].m[1][1] * 0.59f + scrpad->light[1].m[1][2] * 0.11f ;
		lf2 = scrpad->light[1].m[2][0] * 0.30f + scrpad->light[1].m[2][1] * 0.59f + scrpad->light[1].m[2][2] * 0.11f ;
		bump_light.vx = scrpad->light[0].m[0][0] * lf0 +
		  scrpad->light[0].m[0][1] * lf1 +
			scrpad->light[0].m[0][2] * lf2 ;
		bump_light.vy = scrpad->light[0].m[1][0] * lf0 +
		  scrpad->light[0].m[1][1] * lf1 +
			scrpad->light[0].m[1][2] * lf2 ;
		bump_light.vz = scrpad->light[0].m[2][0] * lf0 +
		  scrpad->light[0].m[2][1] * lf1 +
			scrpad->light[0].m[2][2] * lf2 ;
#endif
		/* 各色の輝度を算出 */
		light_force.vx = _sceVu0InnerProduct( scrpad->light[1].m[0], &nomo_const );
		light_force.vy = _sceVu0InnerProduct( scrpad->light[1].m[1], &nomo_const );
		light_force.vz = _sceVu0InnerProduct( scrpad->light[1].m[2], &nomo_const );
		/* 各光源ベクトルに輝度を乗算してベクトルの和を求める */
		bump_light.vx = _sceVu0InnerProduct( &scrpad->light[0].m[0], &light_force );
		bump_light.vy = _sceVu0InnerProduct( &scrpad->light[0].m[1], &light_force );
		bump_light.vz = _sceVu0InnerProduct( &scrpad->light[0].m[2], &light_force );
		/* 合成ベクトルの正規化 */
		_sceVu0Normalize( &bump_light, &bump_light );
		/* シェーダーに登録 */
		DG_SetVertexShaderConstant(CV_CONST0, &bump_light, 1);
	}
#endif

	/* モデルタイプチェック */
#if FALSE
	if ( evmobj->def->type & DG_EVMTYPE_LARGE ){
		model_scale.vz = 1.0f ;
	} else {
		model_scale.vz = 1.0f/16.0f ;
	}
	DG_SetVertexShaderConstant(CV_SCALE, &model_scale, 1);
	//DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	DG_SetVertexShaderConstant(CV_EYEPERS, &scrpad->eye_xpers, 4);
	DG_SetVertexShaderConstant(CV_EYE_INV, &scrpad->eye_inv, 4);
#else
	DG_SetTransform(D3DTS_PROJECTION, &scrpad->xpers) ;
	DG_SetTransform(D3DTS_VIEW,       &scrpad->eye_inv) ;
	DG_SetTransform(D3DTS_WORLD, &DG_UnitMatrix) ;
	//DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;
#endif

}
/* ＤＭＡ接続ルーチン マテリアル初期設定  */
static __forceinline	int	ChainEvmObj_InitMaterial( DG_EVMOBJ *evmobj )
{
	int	no_tex_flag ;

	no_tex_flag = 0 ;

#if 0 /* バウンディングデバッグ表示 */
	{
		extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
		FMATRIX		*mat ;
		EVM_SKEL	*skel ;
		int			i ;
		static int	num = 0 ;
		skel = evmobj->def->skeleton ;
		mat = evmobj->matrix[ evmobj->use_buffer ] ;
		for ( i = 0 ; i < evmobj->n_skeleton ; i++, skel++, mat++ ){
			NewBoundingBoxView_1( mat, &skel->lx, &skel->ux, 0x8040c080 );
		}
	}
#endif
	
	if ( DG_DisplayStatus & DG_STATE_IR_MODE ){
		if ( evmobj->flag & DG_EVMOBJ_IRREACTION ){
			no_tex_flag = 1 ;
			DG_SetPixelShader( NULL );
			DG_SelectVertexShader( &DG_EvmVertexShader[0] );
			DG_SetTexture(0, NULL );
			DG_SetTexture(1, NULL );
			DG_SetTexture(2, NULL );
			DG_SetAlphaMode( 0 );
		}
	}

	return(no_tex_flag) ;
}

/* ＤＭＡ接続ルーチン 終了処理  */
static __forceinline	void	ChainEvmObj_Restore( void )
{
	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
}

/* ＤＭＡ接続ルーチン マテリアル設定  */
static __forceinline	void	ChainEvmObj_SetMaterial(DG_EVMPACK *pack)
{
	if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
		DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
	} else {
		DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
	}
}

/* ＤＭＡ接続ルーチン テクスチャ設定  */
static __forceinline	void	ChainEvmObj_SetTexture(DG_EVMPACK *pack)
{
#if FALSE
	if ( pack->tex0_ptr != NULL ){
		DG_SetTexture(0, (DG_TEX_TRANS*)pack->tex0_ptr );
		//DG_SetAlphaMode( ((DG_TEX_TRANS*)pack->tex0_ptr)->alpha.data );
	} else {
		DG_SetTexture(0, NULL );
	}
	if ( pack->tex1_ptr != NULL ){
		DG_SetTexture(1, (DG_TEX_TRANS*)pack->tex1_ptr );
	} else {
		DG_SetTexture(1, NULL );
	}
#else
	DG_SetTexture(0, (DG_TEX_TRANS*)pack->tex0_ptr );
	DG_SetTexture(1, (DG_TEX_TRANS*)pack->tex1_ptr );
#endif
	if ( pack->tex2_ptr != NULL ){
		DG_SetTexture(2, (DG_TEX_TRANS*)pack->tex2_ptr );
#ifndef UVADJUST
		DG_SetVertexShaderConstant( CV_TEX2_SCALE, &((DG_TEX_TRANS*)pack->tex2_ptr)->vec1, 1);
		DG_SetVertexShaderConstant( CV_TEX2_OFFSET, &((DG_TEX_TRANS*)pack->tex2_ptr)->vec2, 1);
#endif
	} else {
		DG_SetTexture(2, NULL );
	}

	switch ( pack->mtex_type ){
	  default:
	  case DG_MULTITEX_NORMAL:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
		DG_SelectVertexShader( &DG_EvmVertexShader[0] );
		break ;
	  case DG_MULTITEX_EMAP:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
		DG_SelectVertexShader( &DG_EvmVertexShader[1] );
		break ;
	  case DG_MULTITEX_BMAP:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_BMAP] );
		DG_SelectVertexShader( &DG_EvmVertexShader[2] );
		DG_SetTextureDirect(2, ((DG_TEX_TRANS*)pack->tex1_ptr)->ptex );	/* エンボス用に同じテクスチャをセット */
		{
			static	FVECTOR	vec = {32767.0f, 32767.0f, 32767.0f, 32767.0f } ;

			DG_SetTextureDirect(2, ((DG_TEX_TRANS*)pack->tex1_ptr)->ptex );
			DG_SetVertexShaderConstant( CV_TEX2_SCALE, &vec, 1) ;	// 正規化係数
		}

		break ;
	  case DG_MULTITEX_EMAP2:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2] );
		DG_SelectVertexShader( &DG_EvmVertexShader[1]);
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
		break ;
	  case DG_MULTITEX_EMAP3:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP3] );
		DG_SelectVertexShader( &DG_EvmVertexShader[1]);
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		break ;
	  case DG_MULTITEX_EMAP4:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
		DG_SelectVertexShader( &DG_EvmVertexShader[1] );
		break ;
	}
}

/* ＤＭＡ接続ルーチン  */
static void ChainEvmObj( DG_EVMOBJ *evmobj )
{
	int		i, no_tex_flag = 0 ;
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_EVMPACK	*pack ;
	FMATRIX		*mats ;
	EVM_PACK	*evmpack ;

	mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/*-- Material初期設定 ---------------------------------------------------*/

	no_tex_flag = ChainEvmObj_InitMaterial(evmobj) ;
	/*-----------------------------------------------------------------------*/

	/*-- Packet描画 ---------------------------------------------------------*/

	pack = evmobj->packs ;
	evmpack = evmobj->def->packet ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model.n_packs += evmobj->n_packet ;
#endif

	//DG_SetPixelShader( NULL );
	//DG_SetVertexShader( DG_EvmVertexShader[0].handle );
	//DG_SelectVertexShader( &DG_EvmVertexShader[0], NULL );
	///* 頂点バッファの記録方法については今後改良の予定・・・ */

	if( evmobj->vbuff_dirty ){ DG_FlushEvmObjVertexBuffer(evmobj) ; }

	DG_SetStaticVertexBuffer(0, evmobj->d3d_vbuff, sizeof(DG_VERTEX_EVM)) ;
	DG_SetStaticIndexBuffer(evmobj->d3d_ibuff, 0) ;
	for ( i = evmobj->n_packet ; i > 0 ; i--, pack++, evmpack++ ){

		/*-- Material設定 ------------------------------------------------------*/

		ChainEvmObj_SetMaterial(pack) ;
		/*-----------------------------------------------------------------------*/

		/*-- テクスチャ設定 -----------------------------------------------------*/

		if( no_tex_flag == 0 ){ ChainEvmObj_SetTexture(pack) ; }
		/*-----------------------------------------------------------------------*/

		/* マトリクスの設定 */
		SetEvmMatrix( pack->matrix_list, mats );

		/* パケット単位で細かく頂点シェーダーを切り替えながら実行する必要有り */
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pack->idx_vtx_min, pack->idx_vtx_num,
						pack->idx_sofs, evmpack->n_indices-2) ;

#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_verts += pack->n_verts ;
#endif
	}
	/*-----------------------------------------------------------------------*/

	/*-- 終了処理 -----------------------------------------------------------*/

	ChainEvmObj_Restore() ;
	/*-----------------------------------------------------------------------*/
	
}

/* ＤＭＡ接続ルーチン(Index非対応版)  */
static void ChainEvmObjNIdx( DG_EVMOBJ *evmobj )
{
	int		i, no_tex_flag = 0 ;
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_EVMPACK	*pack ;
	FMATRIX		*mats ;
	EVM_PACK	*evmpack ;

	mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/*-- Material初期設定 ---------------------------------------------------*/

	no_tex_flag = ChainEvmObj_InitMaterial(evmobj) ;
	/*-----------------------------------------------------------------------*/

	/*-- Packet描画 ---------------------------------------------------------*/

	pack = evmobj->packs ;
	evmpack = evmobj->def->packet ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model.n_packs += evmobj->n_packet ;
#endif

	//DG_SetPixelShader( NULL );
	//DG_SetVertexShader( DG_EvmVertexShader[0].handle );
	//DG_SelectVertexShader( &DG_EvmVertexShader[0], NULL );
	///* 頂点バッファの記録方法については今後改良の予定・・・ */

	if( evmobj->vbuff_dirty ){ DG_FlushEvmObjVertexBuffer(evmobj) ; }

	DG_SetStaticVertexBuffer(0, evmobj->d3d_vbuff, sizeof(DG_VERTEX_EVM)) ;
	for ( i = evmobj->n_packet ; i > 0 ; i--, pack++, evmpack++ ){

		/*-- Material設定 ------------------------------------------------------*/

		ChainEvmObj_SetMaterial(pack) ;
		/*-----------------------------------------------------------------------*/

		/*-- テクスチャ設定 -----------------------------------------------------*/

		if( no_tex_flag == 0 ){ ChainEvmObj_SetTexture(pack) ; }
		/*-----------------------------------------------------------------------*/

		/* マトリクスの設定 */
		SetEvmMatrix( pack->matrix_list, mats );

		/* パケット単位で細かく頂点シェーダーを切り替えながら実行する必要有り */
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						pack->idx_sofs, evmpack->n_indices-2) ;
#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_verts += pack->n_verts ;
#endif
	}
	/*-----------------------------------------------------------------------*/

	/*-- 終了処理 -----------------------------------------------------------*/

	ChainEvmObj_Restore() ;
	/*-----------------------------------------------------------------------*/
	
}

/* ＤＭＡ接続ルーチン マテリアル初期設定 (VertexShader非対応版) */
static inline	int	ChainEvmObjNVS_InitMaterial( DG_EVMOBJ *evmobj )
{
	int	no_tex_flag ;

	no_tex_flag = 0 ;
	
	if ( DG_DisplayStatus & DG_STATE_IR_MODE ){
		if ( evmobj->flag & DG_EVMOBJ_IRREACTION ){
			no_tex_flag = 1 ;
			DG_SetPixelShader( NULL );
			//DG_SelectVertexShader( &DG_EvmVertexShader[0] );
			DG_SetTexture(0, NULL );
			DG_SetTexture(1, NULL );
			DG_SetTexture(2, NULL );
			DG_SetAlphaMode( 0 );
		}
	}

	return(no_tex_flag) ;
}

/* ＤＭＡ接続ルーチン VertexBuffer設定 (VertexShader非対応版) */
static inline	DWORD	ChainEvmObjNVS_SetVertexBuffer(DG_EVMOBJ *evmobj, DG_EVMPACK *pack,
							WORD *index, DWORD n_indices, FMATRIX *mat)
{
	DWORD	sofs ;
	void	*dst ;

	ASSERT( DG_GetWorkBufferSize() > (sizeof(DG_VERTEX_EVM_FVF) * n_indices) ) ;
	dst = DG_GetWorkBuffer() ;	// 作業用メモリ

	/*-- 頂点計算 -----------------------------------------------------------*/

	if( evmobj->def->type & DG_EVMTYPE_LARGE )
	{
		CalcEVM2FVFLarge(dst, evmobj->vbuff, index + pack->idx_sofs,
						n_indices, pack->matrix_list, mat) ;
	}
	else
	{
		CalcEVM2FVF(dst, evmobj->vbuff, index + pack->idx_sofs,
						n_indices, pack->matrix_list, mat) ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 頂点設定 -----------------------------------------------------------*/

	DG_SetVertexShader(D3DFVF_DG_VERTEX_EVM_FLAG) ;
	DG_SetDynamicVertexBuffer(dst, sizeof(DG_VERTEX_EVM_FVF), n_indices, &sofs) ;
	/*-----------------------------------------------------------------------*/

	return(sofs) ;
}

/* ＤＭＡ接続ルーチン VertexBuffer設定 (VertexShader非対応版) */
static inline	DWORD	ChainEvmObjNVS_SetVertexBuffer_UV1(DG_EVMOBJ *evmobj, DG_EVMPACK *pack,
							WORD *index, DWORD n_indices, FMATRIX *mat)
{
	DWORD	sofs ;
	void	*dst ;

	ASSERT( DG_GetWorkBufferSize() > (sizeof(DG_VERTEX_EVM_FVF) * n_indices) ) ;
	dst = DG_GetWorkBuffer() ;	// 作業用メモリ

	/*-- 頂点計算 -----------------------------------------------------------*/

	if( evmobj->def->type & DG_EVMTYPE_LARGE )
	{
		CalcEVM2FVFLarge_UV1(dst, evmobj->vbuff, index + pack->idx_sofs,
						n_indices, pack->matrix_list, mat) ;
	}
	else
	{
		CalcEVM2FVF_UV1(dst, evmobj->vbuff, index + pack->idx_sofs,
						n_indices, pack->matrix_list, mat) ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 頂点設定 -----------------------------------------------------------*/

	DG_SetVertexShader(D3DFVF_DG_VERTEX_EVM_FLAG) ;
	DG_SetDynamicVertexBuffer(dst, sizeof(DG_VERTEX_EVM_FVF), n_indices, &sofs) ;
	/*-----------------------------------------------------------------------*/

	return(sofs) ;
}

/* ＤＭＡ接続ルーチン テクスチャ設定 (VertexShader非対応版) */
static __forceinline	void	ChainEvmObjNVS_SetTexture(DG_EVMPACK *pack)
{
	DG_SetTextureNVS(0, (DG_TEX_TRANS*)pack->tex0_ptr );
	//DG_SetAlphaMode( ((DG_TEX_TRANS*)pack->tex0_ptr)->alpha.data );

#if FALSE
	DG_SetTextureNVS(1, (DG_TEX_TRANS*)pack->tex1_ptr );
	if ( pack->tex2_ptr != NULL ){
		DG_SetTextureNVS(2, (DG_TEX_TRANS*)pack->tex2_ptr );
#ifndef UVADJUST
		DG_SetVertexShaderConstant( CV_TEX2_SCALE, &((DG_TEX_TRANS*)pack->tex2_ptr)->vec1, 1);
		DG_SetVertexShaderConstant( CV_TEX2_OFFSET, &((DG_TEX_TRANS*)pack->tex2_ptr)->vec2, 1);
		FVECTOR	*vec ;

		vec = &((DG_TEX_TRANS*)pack->tex2_ptr)->vec1 ;
		_tex_param_mtx.m[0][0] = vec->vx ;
		_tex_param_mtx.m[1][1] = vec->vy ;
		vec = &((DG_TEX_TRANS*)pack->tex2_ptr)->vec2 ;
		_tex_param_mtx.m[2][0] = vec->vx ;
		_tex_param_mtx.m[2][1] = vec->vy ;

		DG_SetTransform( D3DTS_TEXTURE2, (void *)&_tex_param_mtx) ;
#endif
	} else {
		DG_SetTexture(2, NULL );
	}
#endif

#if FALSE
	switch ( pack->mtex_type ){
	  default:
	  case DG_MULTITEX_NORMAL:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_NORMAL] );
		break ;
	  case DG_MULTITEX_EMAP:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
		break ;
	  case DG_MULTITEX_BMAP:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_BMAP] );
		DG_SetTexture(2, (DG_TEX_TRANS*)pack->tex1_ptr );	/* エンボス用に同じテクスチャをセット */
		break ;
	  case DG_MULTITEX_EMAP2:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2] );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
		break ;
	  case DG_MULTITEX_EMAP3:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP3] );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		break ;
	  case DG_MULTITEX_EMAP4:
		DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
		break ;
	}
#endif
}


/* ＤＭＡ接続ルーチン(VertexShader非対応版)  */
static void ChainEvmObjNVS( DG_EVMOBJ *evmobj )
{
	int		i, no_tex_flag = 0 ;
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_EVMPACK	*pack ;
	FMATRIX		*mats ;
	EVM_PACK	*evmpack ;
	WORD		*index ;
	DWORD		sofs ;

	mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/*-- Material初期設定 ---------------------------------------------------*/

	no_tex_flag = ChainEvmObjNVS_InitMaterial(evmobj) ;
	/*-----------------------------------------------------------------------*/

	/*-- Packet描画 ---------------------------------------------------------*/

	index = evmobj->def->packet->index ;

	pack    = evmobj->packs ;
	evmpack = evmobj->def->packet ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model.n_packs += evmobj->n_packet ;
#endif
	for ( i = evmobj->n_packet ; i > 0 ; i--, pack++, evmpack++ ){

		/*-- Material設定 ------------------------------------------------------*/

		ChainEvmObj_SetMaterial(pack) ;
		/*-----------------------------------------------------------------------*/

		/*-- テクスチャ設定 -----------------------------------------------------*/

		if( no_tex_flag == 0 ){ ChainEvmObjNVS_SetTexture(pack) ; }
		/*-----------------------------------------------------------------------*/

		sofs = ChainEvmObjNVS_SetVertexBuffer(evmobj, pack,
										index, evmpack->n_indices, mats) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						sofs, evmpack->n_indices-2) ;

#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_verts += pack->n_verts ;
#endif
	}
	/*-----------------------------------------------------------------------*/

	/*-- 特殊描画処理 -------------------------------------------------------*/

	if( !no_tex_flag && (evmobj->def->type & DG_EVMTYPE_DRAW_BLACKBUMP) )
	{
		ChainEvmObjBackBumpNVS(evmobj) ;
	}
	/*-----------------------------------------------------------------------*/

	/*-- 終了処理 -----------------------------------------------------------*/

	ChainEvmObj_Restore() ;
	/*-----------------------------------------------------------------------*/
}

/* バンプマップを強制描画ＤＭＡ接続ルーチン(VertexShader非対応版)  */
static void ChainEvmObjBackBumpNVS( DG_EVMOBJ *evmobj )
{
	int		i ;
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_EVMPACK	*pack ;
	FMATRIX		*mats ;
	EVM_PACK	*evmpack ;
	WORD		*index ;
	DWORD		sofs ;
	DG_TEX_TRANS	*tex_trans ;
	static FMATRIX	mtx =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	} ;

	mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/*-- AlphaMode設定 ------------------------------------------------------*/

	DG_SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE ) ;
	DG_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) ;
	DG_SetRenderState( D3DRS_BLENDOP,          D3DBLENDOP_ADD) ;
	DG_SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_DESTCOLOR) ;
	DG_SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_SRCCOLOR) ;
	/*-----------------------------------------------------------------------*/

	/*-- Packet描画 ---------------------------------------------------------*/

	index = evmobj->def->packet->index ;

	pack    = evmobj->packs ;
	evmpack = evmobj->def->packet ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model.n_packs += evmobj->n_packet ;
#endif
	for ( i = evmobj->n_packet ; i > 0 ; i--, pack++, evmpack++ ){

		if( pack->mtex_type != DG_MULTITEX_BMAP ){ continue ; }

		/*-- Material設定 ------------------------------------------------------*/

		ChainEvmObj_SetMaterial(pack) ;
		/*-----------------------------------------------------------------------*/

		/*-- テクスチャ設定 -----------------------------------------------------*/

		tex_trans = (DG_TEX_TRANS*)pack->tex1_ptr ;
		if( !tex_trans )
		{
			DG_SetTextureDirect(0, NULL) ;
		}
		else
		{
			DG_SetTextureDirect(0, tex_trans->ptex) ;
			mtx.m[0][0] = tex_trans->vec1.x ;
			mtx.m[2][0] = tex_trans->vec2.x ;
			mtx.m[1][1] = tex_trans->vec1.y ;
			mtx.m[2][1] = tex_trans->vec2.y ;
			DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&mtx) ;
		}
		/*-----------------------------------------------------------------------*/

		sofs = ChainEvmObjNVS_SetVertexBuffer_UV1(evmobj, pack,
										index, evmpack->n_indices, mats) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						sofs, evmpack->n_indices-2) ;
#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_verts += pack->n_verts ;
#endif
	}
	/*-----------------------------------------------------------------------*/
	
}
/*----------------------------------------------------------------*/
	/*
		マルチウェイトエンベロープ対応オブジェクト処理
	*/
void _DG_ChainEvmChanl( DG_CHANL *cp, int which, int draw_mode )
{
	DG_OBJ_BUFFER	*obj_buff ;
	DG_EVMOBJ		*evmobj, **que ;
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	int				i, j, c_gid, bound, invisible_flag, max_mem_objs, max_objs ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int		old_tex_code, last_semi_trans = 0, use_semi_trans = 0 ;
	void			(*chain_obj_func)(DG_EVMOBJ *evmobj) ;

	MARK( __FILE__ );
	if ( cp->obj_queue == NULL ) return ;

	chain_obj_func = _chain_obj_func ;

	obj_buff = &cp->obj_queue->evmobj_buffer ;

	c_gid = DG_CurrentGroupID ;

	/* スクラッチパッドワークの初期化 */
	scrpad->pers_mat = cp->pers ;
	scrpad->eye_inv = cp->eye_inv ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->eye_xpers = cp->eye_xpers ;
	scrpad->scale.vx = (float)(cp->width / 2) ;
	scrpad->scale.vy = (float)(cp->height / 2) ;
	scrpad->packet_clock = 0 ;
	scrpad->count = 0 ;
	if( !DG_CheckEvmUseVertexShader() ){ scrpad->xpers = cp->xpers ; }
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model = DG_PerformanceData.mulwt_model ;
#endif
	{/* 赤外線モードライトマトリクス生成 */
		if( !DG_CheckIROldVersion() )
		{
			/* X-BOX版準拠演出 */
			static FMATRIX	ir_light[2] = {
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
				{{{255,255,255,0},{0,0,0,0},{0,0,0,0},{128,128,128,128}}}
			};
			scrpad->ir_light_mat[0] = ir_light[0] ;
			scrpad->ir_light_mat[1] = ir_light[1] ;
			scrpad->ir_light_mat[0].m[0][0] = cp->eye.m[2][0] ;
			scrpad->ir_light_mat[0].m[1][0] = cp->eye.m[2][1] ;
			scrpad->ir_light_mat[0].m[2][0] = cp->eye.m[2][2] ;
		}
		else
		{
			/* PS2版っぽい演出 */
			static FMATRIX	ir_light[2] = {
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
				{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{255,255,255,255}}}
			};

			ZeroMemory(&scrpad->ir_light_mat[0], sizeof(FMATRIX)) ;
			scrpad->ir_light_mat[1] = ir_light[1] ;
		}
	}

	/* オブジェクトのリストをスクラッチパッドにコピー */
	invisible_flag = DG_EVMOBJ_INVISIBLE0 << cp->chanl_num ;
	max_objs = 0 ;
	que = (DG_EVMOBJ**)obj_buff->queue ;
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	for ( i = obj_buff->n_queue ; i > 0 ; i--, que++ ){
		evmobj = *que ;
		if ( !( evmobj->group_id & DG_CurrentGroupID ) ) continue ;
		if ( evmobj->flag & invisible_flag ) continue ;
		/* 描画フェーズコントロール */
		if ( draw_mode == 0 ){
			if ( evmobj->flag & DG_EVMOBJ_LATTERDRAW ) continue ;
		} else {
			if ( !( evmobj->flag & DG_EVMOBJ_LATTERDRAW ) ) continue ;
		}
		if ( evmobj->flag & DG_EVMOBJ_SEMITRANS ) use_semi_trans = 1 ;
		obj_list[ max_objs ].obj = evmobj ;
		obj_list[ max_objs++ ].tri_code = evmobj->def->texture ;
	}
	if ( max_objs == 0 ) return ;

	/* スクラッチパッド上のオブジェクトリストをテクスチャ毎にソートしてメモリに書き出す */
	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;
	max_mem_objs = 0 ;
	for ( tex_list_p = DG_TextureList, j = DG_MaxTextures ; j > 0 ; tex_list_p++, j-- ){
		tex_list = *tex_list_p;
		if ( tex_list->header->compress_flag & TRI_FLAG_TRANS ) continue ;
#ifdef LIBDG_CHECK_MULTILOADTEX
		//if ( GV_Time & 2 ) if ( tex_list->flag ) continue ;
#endif
		for ( i = 0 ; i < max_objs ; i++ ) {
			if ( tex_list->code != obj_list[i].tri_code ) continue ;
			mem_obj_list->obj = obj_list[i].obj ;
			mem_obj_list->tri_code = (int)tex_list ;	/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list++ ;
			max_mem_objs++ ;
		}
	}
	if ( max_mem_objs == 0 ) return ;

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 半透明を使用したオブジェクトがある場合には事前にマスクを生成 */
	if ( use_semi_trans ){
		DG_MakeStencilMask();
	}

	/* 描画初期設定 */
	DG_ChanlRenderState( cp );

	/* 初期化パケットの設定 */
	//SetEvmInitPacket( (void*)&scrpad->dma_buffer, cp );

	/* ソート済みオブジェクトをＤＭＡにつなぐ */
	old_tex_code = -1 ;
	mem_obj_list = (OBJ_LIST*)DG_LocalMemory ;
	for ( i = max_mem_objs ; i > 0 ; i--, mem_obj_list++ ){
		evmobj = mem_obj_list->obj ;

		/* マトリクス設定 */
		if ( evmobj->root != NULL ) evmobj->world = *( evmobj->root ) ;
		scrpad->world = evmobj->world ;

		/* バウンディングチェック */
		/* 新方式バウンディングチェック（各スケルトンのバウンディングを合成したバウンディングを使用する） */
		MakeMaxBound( evmobj->matrix[ evmobj->use_buffer ], evmobj->def->skeleton, evmobj->n_skeleton );
		bound = BoundCheck2( &scrpad->eye_pers );
		if ( bound == 2 ) continue ;
		scrpad->bound_mode = bound ;

		/* テクスチャのチェック */
		if ( mem_obj_list->tri_code != old_tex_code ){
			tex_list = (DG_TEXTURE_LIST*)mem_obj_list->tri_code ;
#ifdef LIBDG_PERFORMANCE
			DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
			tex_list->flag = 1 ;
			//size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
			//DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			old_tex_code = mem_obj_list->tri_code ;
			tex_list++ ;
		}
		/* 強制半透明描画チェック */
		if ( evmobj->flag & DG_EVMOBJ_SEMITRANS ){
			if ( last_semi_trans == 0 ){
				DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
			}
			last_semi_trans = 1 ;
		} else {
			if ( last_semi_trans != 0 ){
				DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
			}
			last_semi_trans = 0 ;
		}

		/* マトリクスセットアップ */
		if( DG_CheckEvmUseVertexShader() ){ EvmobjSetup( evmobj ); }
		else{ EvmobjSetupNVS( evmobj ); }

		/* 描画 */
#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_obj++ ;
#endif
		chain_obj_func( evmobj );
#if FALSE
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
	}
	/* Ｚ圧縮を活用するためにステンシルをクリア */
	if ( use_semi_trans ){
		DG_Clear(0, NULL, D3DCLEAR_STENCIL, 0, 0.0f, 0);
	}

	DG_SetPixelShader( NULL );
	DG_InitTextureStageState( 0 );
	DG_InitTextureStageState( 1 );
	DG_InitTextureStageState( 2 );
	DG_InitTextureStageState( 3 );

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.mulwt_model = scrpad->mulwt_model ;
#endif
}


/*----------------------------------------------------------------*/
	/*
		マルチウェイトエンベロープ対応オブジェクト処理
	*/
void DG_ChainEvmChanl( DG_CHANL *cp, int which )
{
	DG_PerfStartD(DGPERF_CTGR_CHAIN_EVM) ;
	_DG_ChainEvmChanl( cp, which, 0 );
	DG_PerfEndD(DGPERF_CTGR_CHAIN_EVM) ;
}

void DG_ChainEvmChanlLatter( DG_CHANL *cp, int which )
{
	DG_PerfStartD(DGPERF_CTGR_CHAIN_EVM_LATTER) ;
	_DG_ChainEvmChanl( cp, which, 1 );
	DG_PerfEndD(DGPERF_CTGR_CHAIN_EVM_LATTER) ;
}


/*----------------------------------------------------------------*/
static int SetMultiTexType( DG_TEX *tex0, DG_TEX *tex1, DG_TEX *tex2 )
{
	if ( tex1 == NULL && tex2 == NULL ) return ( DG_MULTITEX_NORMAL );	/* シングルテクスチャ */
	/* 環境マップかどうかをチェック */
	if ( tex2 != NULL ){
		if ( ( tex2->flag & TEXINFO_FLAG_MAPMASK ) == TEXINFO_FLAG_EMAP ){
			if ( tex1 != NULL ){
				if ( ( tex0->tex_trans.alpha.data & 0xff ) == SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ) ){
					/* 重ね乗せ用２テクスチャ環境マップ（アルファベース＋加算環境マップ） */
					return ( DG_MULTITEX_EMAP3 );
				}
				/* ＭＧＳ２標準環境マップ */
				return ( DG_MULTITEX_EMAP );
			} else {
				if ( ( tex0->tex_trans.alpha.data & 0xff ) == SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ) ){
					/* 重ね乗せ用２テクスチャ環境マップ（減算ベース＋加算環境マップ） */
					return ( DG_MULTITEX_EMAP2 );
				}
				/* ＭＧＳ２標準環境マップ */
				return ( DG_MULTITEX_EMAP4 );
			}
		}
	}
	/* トゥーンシェードマップかどうかをチェック */
	if ( tex2 != NULL ){
		if ( ( tex2->flag & TEXINFO_FLAG_MAPMASK ) == TEXINFO_FLAG_SMAP ){
			return ( DG_MULTITEX_SMAP );
		}
	}
	/* バンプマップかどうかをチェック */
	if ( tex1 != NULL ){
		if ( ( tex1->flag & TEXINFO_FLAG_MAPMASK ) == TEXINFO_FLAG_BMAP ){
			return ( DG_MULTITEX_BMAP );
		}
	}
	/* その他特殊系かどうかをチェック */
	return ( DG_MULTITEX_NORMAL3 );
}

/*----------------------------------------------------------------*/
	/*
		頂点データ作成(非効率なので、できればコンバータで対応したい)
	*/
static	DG_VERTEX_EVM *DG_MakeVertexEvm( DG_VERTEX_EVM_ORG *src_buff, int num )
{
	int					i ;
	DG_VERTEX_EVM		*vertex_buff ;
	DG_VERTEX_EVM		*dst ;
	DG_VERTEX_EVM_ORG	*src ;

	/*-- 領域確保 --------------------------------------------------*/

	vertex_buff = GV_AllocMemory(GV_NORMAL_MEMORY, NULL, sizeof(DG_VERTEX_EVM) * num, 4) ;
	if( !vertex_buff ){ return(NULL) ; }
	/*--------------------------------------------------------------*/

	/*-- データ変換 ------------------------------------------------*/

	dst = vertex_buff ;
	src = src_buff ;
	for(i=num; i>0; i--, dst++, src++)
	{
		dst->vx = src->vx ;
		dst->vy = src->vy ;
		dst->vz = src->vz ;
		dst->vw = src->vw ;

		dst->nx   = src->nx ;
		dst->ny   = src->ny ;
		dst->nz   = src->nz ;
		dst->padn = 0 ;

		dst->u0   = src->u0 ;
		dst->v0   = src->v0 ;
		dst->q0   = 0 ;
		dst->pad0 = 1 ;		// 1.0f

		dst->u1   = src->u1 ;
		dst->v1   = src->v1 ;
		dst->q1   = 0 ;
		dst->pad1 = 1 ;		// 1.0f

		dst->u2   = src->u2 ;
		dst->v2   = src->v2 ;
		dst->q2   = 0 ;
		dst->pad2 = 1 ;		// 1.0f

		dst->weight[0] = src->weight[0] ;
		dst->weight[1] = src->weight[1] ;
		dst->weight[2] = src->weight[2] ;
		dst->weight[3] = src->weight[3] ;

		dst->index[0] = src->index[0] ;
		dst->index[1] = src->index[1] ;
		dst->index[2] = src->index[2] ;
		dst->index[3] = src->index[3] ;
	}
	/*--------------------------------------------------------------*/

	return( vertex_buff ) ;
}

/*----------------------------------------------------------------*/
	/*
		頂点データ解放
	*/
static	void	DG_FreeVertexEvm( DG_VERTEX_EVM *addr )
{
	GV_Free(addr) ;
}

static	void	DG_DelayedFreeVertexEvm( DG_VERTEX_EVM *addr )
{
	GV_DelayedFree(addr) ;
}

/*----------------------------------------------------------------*/
	/*
		マルチウェイトエンベロープ対応オブジェクト作成
	*/
DG_EVMOBJ *DG_MakeEvmObj( EVM_DEF *def, int flag, int chanl )
{
	int		buf_size ;
	int		i, n_skeletons ;
	DG_EVMOBJ	*evmobj ;
	DG_EVMPACK	*evmpack ;
	EVM_PACK	*pack ;
#ifdef _WINDOWS
	WORD		*ibuff ;
	DWORD		n_indices ;
	DWORD		n_verts ;
	DWORD		size ;
	BYTE		*dst ;
	DWORD		multi_tex_max ;
#endif

	flag |= DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3 ;
	ASSERT( def != NULL );
	n_skeletons = def->n_x_models ;
	/*
		物体ハンドラのバッファを確保する
	*/
	buf_size = sizeof( DG_EVMOBJ ) + sizeof( FMATRIX ) * n_skeletons * 2 + sizeof( DG_EVMPACK ) * def->n_packs ;
	if ( ( evmobj = GV_Malloc( buf_size ) ) == NULL ){
		return NULL ;
	}
	/*
		基本データ初期化
	*/
	GV_ZeroMemory( evmobj, sizeof(DG_EVMOBJ) ) ;
	evmobj->world = DG_UnitMatrix;
	evmobj->light = &DG_LightMatrix ;
	evmobj->def = def ;
	evmobj->flag = flag ;
	evmobj->chanl = chanl ;
	evmobj->group_id = 0x7fffffff ;	
	evmobj->n_skeleton = n_skeletons ;
	evmobj->n_packet = def->n_packs ;
	evmobj->use_buffer = 0 ;
	evmobj->matrix[0] = (FMATRIX*)&evmobj[1] ;
	evmobj->matrix[1] = &evmobj->matrix[0][n_skeletons] ;
	evmobj->packs = (DG_EVMPACK*)&evmobj->matrix[1][n_skeletons] ;
	//evmobj->skel_bounds = (FVECTOR*)def->packet[0].pad1 ;

#ifndef _WINDOWS
	evmobj->vbuff = def->vbuff ;
#else
	evmobj->vbuff = DG_MakeVertexEvm((DG_VERTEX_EVM_ORG *)def->vbuff, def->n_verts) ;
#endif

	evmobj->verts_ptr = def->packet[0].verts ;
	evmobj->norms_ptr = def->packet[0].norms ;
	evmobj->uvs0_ptr = def->packet[0].uvs[0] ;
	evmobj->uvs1_ptr = def->packet[0].uvs[1] ;
	evmobj->uvs2_ptr = def->packet[0].uvs[2] ;
	evmobj->weight_ptr = def->packet[0].weight ;
#if 0
		/* 強制バンプマッピング実験 */
	{
		SVECTOR	*uvs_mem ;
		int		total_mem ;
		total_mem = 0 ;
		pack = def->packet ;
		for ( i = 0 ; i < def->n_packs ; i++ ){
			total_mem += ( pack->n_verts + 1 ) & 0xfffe ;
			pack++ ;
		}
		uvs_mem = evmobj->uvs2_ptr = GV_Malloc( sizeof(SVECTOR) * total_mem );
#if 1
		pack = def->packet ;
		for ( i = 0 ; i < def->n_packs ; i++ ){
			pack->uvs[2] = uvs_mem ;
			uvs_mem += ( pack->n_verts + 1 ) & 0xfffe ;
			//uvs_mem += ( pack->flag & DG_PACKFLAG_UV1 ) ? ( ( pack->n_verts + 1 ) & 0xfffe ) : 0 ;
			pack++ ;
		}
#endif
		def->packet[0].uvs[1] = def->packet[0].uvs[0] ;
	}
#endif

	for ( i = 0 ; i < n_skeletons ; i++ ) {
		evmobj->matrix[0][i] = DG_UnitMatrix ;
	}
	for ( i = 0 ; i < n_skeletons ; i++ ) {
		evmobj->matrix[1][i] = DG_UnitMatrix ;
	}

	/*
		パケット管理情報初期化
	*/
#ifdef _WINDOWS
	n_indices = 0 ;
	n_verts   = 0 ;
	multi_tex_max = DG_GetMultiTexMax() ;
#endif
	pack    = def->packet ;
	evmpack = evmobj->packs ;
	ibuff   = pack->index ;
	for ( i = 0 ; i < evmobj->n_packet ; i++ ){
		if ( evmobj->uvs1_ptr == NULL && pack->uvs[1] != NULL ) evmobj->uvs1_ptr = pack->uvs[1] ;
		if ( evmobj->uvs2_ptr == NULL && pack->uvs[2] != NULL ) evmobj->uvs2_ptr = pack->uvs[2] ;
		evmpack->flag = pack->flag ;
		evmpack->max_mats = pack->n_mats ;
		evmpack->n_verts = pack->n_verts ;
		evmpack->n_step = ( pack->n_verts + 1 ) / 2 ;
		evmpack->n_uv_step[0] = ( evmpack->flag & DG_PACKFLAG_UV0 ) ? evmpack->n_step : 0 ;
		evmpack->n_uv_step[1] = ( evmpack->flag & (DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP) ) ? evmpack->n_step : 0 ;
		evmpack->n_uv_step[2] = ( evmpack->flag & (DG_PACKFLAG_UV2|DG_PACKFLAG_BMAP) ) ? evmpack->n_step : 0 ;
//printf("%08x %d %d %d %08x %08x %08x\n", evmpack->flag, 
//	   evmpack->n_uv_step[0], evmpack->n_uv_step[1], evmpack->n_uv_step[2],
//	   pack->tex_id[0], pack->tex_id[1], pack->tex_id[2] );
		evmpack->tex0_ptr = NULL ;
		evmpack->tex1_ptr = NULL ;
		evmpack->tex2_ptr = NULL ;
		if ( pack->tex_id[0] != 0 ){
			evmpack->tex0_ptr = &((DG_TEX*)pack->tex_id[0])->tex_trans ;
		}
		if ( pack->tex_id[1] != 0 ){
			evmpack->tex1_ptr = &((DG_TEX*)pack->tex_id[1])->tex_trans ;
		}
		if ( pack->tex_id[2] != 0 ){
			evmpack->tex2_ptr = &((DG_TEX*)pack->tex_id[2])->tex_trans ;
		}
		evmpack->mtex_type = SetMultiTexType( (DG_TEX*)pack->tex_id[0], (DG_TEX*)pack->tex_id[1], (DG_TEX*)pack->tex_id[2] );
#if 0
		/* 強制バンプマッピング実験（平行投影用＜要mwtrans2.dsm＞） */
		if ( pack->uvs[1] != NULL ){
			evmpack->flag |= DG_PACKFLAG_BMAP ;
			evmobj->uvs1_ptr = evmobj->uvs0_ptr ;
			pack->uvs[1] = pack->uvs[0] ;
			evmpack->n_uv_step[1] = evmpack->n_uv_step[0] ;
			evmpack->tex1_ptr = evmpack->tex0_ptr ;
			{
				int		t_size ;
				DG_TEX	*tex ;
				tex = (DG_TEX*)pack->tex_id[0] ;
				t_size = 2 << ( ( tex->tex0_base >> 26 ) & 15 ) ;
				tex->tex_trans.vec1.vx = 1.0f / (float)t_size ;
				t_size = 2 << ( ( tex->tex0_base >> 30 ) & 15 ) ;
				tex->tex_trans.vec1.vy = 1.0f / (float)t_size ;
				//tex->tex_trans.tex0.data |= SCE_GS_SET_TEX0( 0,0,0,0,0,0,1,0,0,0,0,0);
			}
		}
#endif
#if 0
		/* 強制バンプマッピング実験 */
		if ( !( evmpack->flag & DG_PACKFLAG_BMAP ) && ( pack->uvs[2] != 0 ) ){
#if 1
			{
				evmpack->flag |= DG_PACKFLAG_BMAP ;
				evmobj->uvs1_ptr = evmobj->uvs0_ptr ;
				pack->uvs[1] = pack->uvs[0] ;
				evmpack->n_uv_step[2] = evmpack->n_uv_step[1] = evmpack->n_uv_step[0] ;
				evmpack->tex1_ptr = evmpack->tex0_ptr ;
			}
#endif
#if 1
			if ( pack->tex_id[2] != 0 ){
				int		t_size ;
				DG_TEX	*tex ;
				tex = (DG_TEX*)pack->tex_id[2] ;
				t_size = 2 << ( ( tex->tex_trans.tex0.data >> 26 ) & 15 ) ;
				tex->tex_trans.vec1.vx = 1.0f / (float)t_size ;
				t_size = 2 << ( ( tex->tex_trans.tex0.data >> 30 ) & 15 ) ;
				tex->tex_trans.vec1.vy = 1.0f / (float)t_size ;
				//tex->tex_trans.tex0.data |= SCE_GS_SET_TEX0( 0,0,0,0,0,0,1,0,0,0,0,0);
			}
#endif
			{
				SVECTOR	*norm, *uvs ;
				FVECTOR	n, u, v = {0.0f,-1.0f,0.0f,0.0f};
				int		i ;
				norm = pack->norms ;
				uvs = pack->uvs[2] ;
				for ( i = 0 ; i < pack->n_verts ; i++ ){
					n.vx = (float)norm->vx / 4096.0f ;
					n.vy = (float)norm->vy / 4096.0f ;
					n.vz = (float)norm->vz / 4096.0f ;
					_sceVu0OuterProduct( &u, &n, &v );
					_sceVu0Normalize( &u, &u );
					printf("%d %d %d ", uvs->vx, uvs->vy, uvs->vz );
					uvs->vx = DG_FTOI( u.vx * 4096.0f );
					uvs->vy = DG_FTOI( u.vy * 4096.0f );
					uvs->vz = DG_FTOI( u.vz * 4096.0f );
					printf(",%d %d %d \n", uvs->vx, uvs->vy, uvs->vz );
					norm++ ;
					uvs++ ;
				}
			}
		}
#endif
#if 0
		{
			DG_TEX	*tex ;
			if ( pack->tex_id[1] != NULL && pack->tex_id[2] == NULL ){
				tex = DG_GetTexture2( def->texture, GV_StrCode("sna_spe_ovl_mod1120_alp") );
				evmpack->tex2_ptr = &tex->tex_trans ;
				evmpack->flag |= DG_PACKFLAG_EMAP | DG_PACKFLAG_TEX2 ;
			}
		}
#endif

#ifdef _WINDOWS

		/*-- 参照頂点情報検査(コンバータで対応しているのが理想) */

		{
			int		num ;
			WORD	*idx_ptr ;
			WORD	idx_sofs ;
			WORD	idx ;
			WORD	min, max ;

			idx_sofs = (WORD)((DWORD)pack->index - (DWORD)ibuff)/sizeof(WORD) ;

			num = pack->n_indices ;
			if( num )
			{
#if FALSE
				if( pack->index != ibuff )
				{
					char	strbuff[256] ;

					sprintf(strbuff, "[%d/%d] %d, 0x%08X, 0x%08X (%d)", i, evmobj->n_packet, num,
										(DWORD)pack->index, (DWORD)ibuff,
										(int)pack->index - (int)ibuff) ;
					dbgErrMessPuts(strbuff, S_OK) ;
				}
#endif

				idx_ptr = pack->index ;

				idx = *idx_ptr ;
				min = idx ;
				max = idx ;

				idx_ptr++ ;
				num-- ;
				for(; num>0; num--, idx_ptr++)
				{
					idx = *idx_ptr ;
					if( max < idx ){ max = idx ; }
					else if( min > idx ){ min = idx ; }
				}
				max++ ;	// 補正

				evmpack->idx_vtx_min = min ;
				evmpack->idx_vtx_num = max - min ;
				evmpack->idx_sofs    = idx_sofs ;

				if( n_verts < max ){ n_verts = max ; }	// 総頂点数
			}
		}
		/*------------------------------------------------------*/

		n_indices = evmpack->idx_sofs + pack->n_indices ;	// 総Index頂点数
#endif

		evmpack->matrix_list = *(long*)&pack->mat_id[0] ;
		evmpack++ ;
		pack++ ;
	}

#ifdef _WINDOWS
	/*-- 頂点数設定 --------------------------------------------*/

	evmobj->n_verts   = n_verts ;
	evmobj->n_indices = n_indices ;
	/*----------------------------------------------------------*/

	/*-- Vertex Buffer作成 -------------------------------------*/

	evmobj->d3d_vbuff = NULL ;	/* VertexBuffer */
	evmobj->d3d_ibuff = NULL ;	/* IndexBuffer */

	if( DG_CheckEvmUseVertexShader() )	// VertexShaderを使用する場合のみ必要
	{
		if( DG_CheckIndexPrimitiveUseable() )
		{
			DG_CreateVertexBuffer(sizeof(DG_VERTEX_EVM) * n_verts,
								DG_WinApp.evm_vbuff_usage | D3DUSAGE_WRITEONLY,
								0, EVM_VERTEXBUFFER_POOL,
								&evmobj->d3d_vbuff) ;
			DG_CreateIndexBuffer(n_indices * sizeof(WORD),
								DG_WinApp.evm_vbuff_usage | D3DUSAGE_WRITEONLY,
								D3DFMT_INDEX16, EVM_VERTEXBUFFER_POOL,
								&evmobj->d3d_ibuff) ;
		}
		else
		{
			DG_CreateVertexBuffer(sizeof(DG_VERTEX_EVM) * n_indices,
								DG_WinApp.evm_vbuff_usage | D3DUSAGE_WRITEONLY,
								0, EVM_VERTEXBUFFER_POOL,
								&evmobj->d3d_vbuff) ;
		}
	}
	/*----------------------------------------------------------*/

	/*-- バッファ内容の複写 ------------------------------------*/

	if( DG_CheckEvmUseVertexShader() )	// VertexShaderを使用する場合のみ必要
	{
		if( DG_CheckIndexPrimitiveUseable() )
		{
			if( evmobj->d3d_vbuff )
			{
				size = sizeof(DG_VERTEX_EVM) * n_verts ;
				IDirect3DVertexBuffer8_Lock(evmobj->d3d_vbuff, 0, size, &dst, EVM_VERTEXBUFFER_LOCKFLAG) ;
				memcpy(dst, evmobj->vbuff, size) ;
				IDirect3DVertexBuffer8_Unlock(evmobj->d3d_vbuff) ;
			}
			if( evmobj->d3d_ibuff )
			{
				size = sizeof(WORD) * n_indices ;
				IDirect3DIndexBuffer8_Lock(evmobj->d3d_ibuff, 0, size, &dst, EVM_VERTEXBUFFER_LOCKFLAG) ;
				memcpy(dst, evmobj->def->packet->index, size) ;
				IDirect3DIndexBuffer8_Unlock(evmobj->d3d_ibuff) ;
			}
		}
		else
		{
			if( evmobj->d3d_vbuff )
			{
				size = sizeof(DG_VERTEX_EVM) * n_indices ;
				IDirect3DVertexBuffer8_Lock(evmobj->d3d_vbuff, 0, size, &dst, EVM_VERTEXBUFFER_LOCKFLAG) ;
				DG_ArrangeVertexByIndex(dst, (void *)evmobj->vbuff, sizeof(DG_VERTEX_EVM),
									evmobj->def->packet->index, n_indices) ;
				IDirect3DVertexBuffer8_Unlock(evmobj->d3d_vbuff) ;
			}
		}
	}
	/*----------------------------------------------------------*/
#endif

	return evmobj ;
}

	/*
		オブジェクトの開放
	*/
void DG_FreeEvmObj( DG_EVMOBJ *evmobj )
{
	DG_ExeBufferSync(FALSE) ;	// 安全策
#ifdef _WINDOWS
	if( evmobj->d3d_vbuff  ){ DG_ReleaseD3DVertexBuffer(evmobj->d3d_vbuff) ; }
	if( evmobj->d3d_ibuff  ){ DG_ReleaseD3DIndexBuffer(evmobj->d3d_ibuff) ; }
	if( evmobj->vbuff ){ DG_DelayedFreeVertexEvm(evmobj->vbuff) ; }
#endif
	GV_DelayedFree( evmobj ) ;
}



/*----------------------------------------------------------------*/
	/*
		ＵＶ値の補正処理
	*/
void DG_WriteEvmMdlPaketUV( int tri_code, EVM_PACK *pack )
{
	float	u_scale, v_scale, u_offset, v_offset ;
	short	*uv ;
	DG_TEX	*tex ;
	int			i, j ;

	for ( i = 0 ; i < 3 ; i++ ){

		if ( i == 2 && ( pack->flag & DG_PACKFLAG_BMAP ) ){
			pack->tex_id[2] = pack->tex_id[1] ;
			continue ;
		}
		if ( pack->tex_id[ i ] == 0 ) continue ;

		/* ＵＶ値のスケール補正（既にテクスチャが読み込まれている必要あり） */
		tex = DG_GetTexture2( tri_code, pack->tex_id[i] );
		uv = pack->uvs[i] ;

		if ( tex != NULL ){
			u_scale = tex->u_scale ;
			v_scale = tex->v_scale ;
			u_offset = tex->u_offset * 4096.0F ;
			v_offset = tex->v_offset * 4096.0F ;
		} else {
			u_scale = 1.0F ; v_scale = 1.0F ;
			u_offset = 0.0F ; v_offset = 0.0F ;
		}

		if ( uv != NULL ){
			for ( j = pack->n_verts ; j > 0 ; j-- ){
#ifndef UVADJUST
				uv[0] = (short)( (float)uv[0] * u_scale + u_offset );
				uv[1] = (short)( (float)uv[1] * v_scale + v_offset );
#endif
				uv += 4 ;
			}
		}
		pack->tex_id[i] = (int)tex ;
	}

}

/*----------------------------------------------------------------*/
	/*
		Vertex Buffer変更の適用
	*/
void DG_AssignEvmObjVertexBuffer(DG_EVMOBJ *evmobj)
{
	evmobj->vbuff_dirty = TRUE ;
}

void DG_FlushEvmObjVertexBuffer(DG_EVMOBJ *evmobj)
{
	DWORD	size ;
	BYTE	*dst ;

	if( DG_CheckIndexPrimitiveUseable() )
	{
		if( evmobj->d3d_vbuff )
		{
			DWORD	n_verts ;

			n_verts   = evmobj->n_verts ;

			size = sizeof(DG_VERTEX_EVM) * n_verts ;
			IDirect3DVertexBuffer8_Lock(evmobj->d3d_vbuff, 0, size, &dst, EVM_VERTEXBUFFER_LOCKFLAG) ;
			memcpy(dst, evmobj->vbuff, size) ;
			IDirect3DVertexBuffer8_Unlock(evmobj->d3d_vbuff) ;
		}
	}
	else
	{
		if( evmobj->d3d_vbuff )
		{
			DWORD	n_indices ;

			n_indices = evmobj->n_indices ;

			size = sizeof(DG_VERTEX_EVM) * n_indices ;
			IDirect3DVertexBuffer8_Lock(evmobj->d3d_vbuff, 0, size, &dst, EVM_VERTEXBUFFER_LOCKFLAG) ;
			DG_ArrangeVertexByIndex(dst, (void *)evmobj->vbuff, sizeof(DG_VERTEX_EVM),
								evmobj->def->packet->index, n_indices) ;
			IDirect3DVertexBuffer8_Unlock(evmobj->d3d_vbuff) ;
		}
	}

	evmobj->vbuff_dirty = FALSE ;
}

/*----------------------------------------------------------------*/
	/*
		Index Buffer変更の適用
	*/
void DG_AssignEvmObjIndexBuffer(DG_EVMOBJ *evmobj)
{
	evmobj->ibuff_dirty = TRUE ;
}

void DG_FlushEvmObjIndexBuffer(DG_EVMOBJ *evmobj)
{
	DWORD	size ;
	DWORD	n_indices ;
	BYTE	*dst ;

	if( DG_CheckIndexPrimitiveUseable() )
	{
		if( evmobj->d3d_ibuff )
		{
			n_indices = evmobj->n_indices ;

			size = sizeof(WORD) * n_indices ;
			IDirect3DIndexBuffer8_Lock(evmobj->d3d_ibuff, 0, size, &dst, EVM_INDEXBUFFER_LOCKFLAG) ;
			memcpy(dst, evmobj->def->packet->index, size) ;
			IDirect3DIndexBuffer8_Unlock(evmobj->d3d_ibuff) ;
		}
	}
	else
	{
		if( evmobj->d3d_vbuff )
		{
			n_indices = evmobj->n_indices ;

			size = sizeof(DG_VERTEX_EVM) * n_indices ;
			IDirect3DVertexBuffer8_Lock(evmobj->d3d_vbuff, 0, size, &dst, EVM_VERTEXBUFFER_LOCKFLAG) ;
			DG_ArrangeVertexByIndex(dst, (void *)evmobj->vbuff, sizeof(DG_VERTEX_EVM),
								evmobj->def->packet->index, n_indices) ;
			IDirect3DVertexBuffer8_Unlock(evmobj->d3d_vbuff) ;
		}
	}

	evmobj->ibuff_dirty = FALSE ;
}

/*	FVF頂点自力計算(VertexShader未対応の場合) */
static	void	CalcEVM2FVF(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat)
{
	int					i, j ;
	DG_VERTEX_EVM_FVF	*dst_vtx ;
	DG_VERTEX_EVM		*src_vtx ;
	FMATRIX				*idx_mtx[8] ;
	FMATRIX				*mtx ;
	FVECTOR				src_pos ;
	FVECTOR				src_nrm ;
	FVECTOR				pos ;
	FVECTOR				nrm ;
	FVECTOR				vec ;
	float				wt ;
	short				*src_mtx_idx ;
	short				*weight ;
	short				idx ;

	/*-- 使用MATRIX取得 ------------------------------------------------*/

	{
		FMATRIX	**mtx_ptr ;

		mtx_ptr = idx_mtx ;
		for(i=8; i>0; i--, mtx_ptr++, matrix_list>>=8)
		{
			*mtx_ptr = mat + (matrix_list & 0xff) ;
		}
	}
	/*------------------------------------------------------------------*/

	/*-- 頂点計算 ------------------------------------------------------*/

	dst_vtx = dst ;
	for(i=num; i>0; i--, dst_vtx++, index++)
	{
		src_vtx = src + (*index) ;	// 元頂点

		src_pos.vx = (float)src_vtx->vx * (1.0f/16.0f) ;
		src_pos.vy = (float)src_vtx->vy * (1.0f/16.0f) ;
		src_pos.vz = (float)src_vtx->vz * (1.0f/16.0f) ;
		src_pos.vw = 1.0f ;
		src_nrm.vx = (float)src_vtx->nx ;
		src_nrm.vy = (float)src_vtx->ny ;
		src_nrm.vz = (float)src_vtx->nz ;
		src_nrm.vw = 0.0f ;

		pos.vx = 0.0f ;
		pos.vy = 0.0f ;
		pos.vz = 0.0f ;
		nrm.vx = 0.0f ;
		nrm.vy = 0.0f ;
		nrm.vz = 0.0f ;

		/*-- Weight計算 ------------------------------------------------*/

		src_mtx_idx = src_vtx->index ;
		weight      = src_vtx->weight ;
		for(j=4; j>0; j--, src_mtx_idx++, weight++)
		{
			if( *weight )
			{
				wt = (float)(*weight) * (1.0f/32767.0f) ;
				idx = *src_mtx_idx ;
				ASSERT( (idx >= 0) && (idx < 32) ) ;

				//mtx = idx_mtx[idx >> 2] ;		// こちらの方が正式な書き方
				mtx = *(FMATRIX **)(((DWORD)idx_mtx) + idx) ;

				ApplyMatrix(&vec, mtx, &src_pos) ;
				pos.vx += vec.vx * wt ;
				pos.vy += vec.vy * wt ;
				pos.vz += vec.vz * wt ;

				ApplyMatrix(&vec, mtx, &src_nrm) ;
				nrm.vx += vec.vx * wt ;
				nrm.vy += vec.vy * wt ;
				nrm.vz += vec.vz * wt ;
			}
		}
		_sceVu0Normalize(&nrm, &nrm) ;	// 法線正規化
		/*--------------------------------------------------------------*/

		/*-- 頂点出力 --------------------------------------------------*/

		dst_vtx->vx = pos.vx ;
		dst_vtx->vy = pos.vy ;
		dst_vtx->vz = pos.vz ;

		dst_vtx->nx = nrm.vx ;
		dst_vtx->ny = nrm.vy ;
		dst_vtx->nz = nrm.vz ;

		dst_vtx->u0 = (float)src_vtx->u0 ;
		dst_vtx->v0 = (float)src_vtx->v0 ;
		//dst_vtx->u1 = (float)src_vtx->u1 ;
		//dst_vtx->v1 = (float)src_vtx->v1 ;
		//dst_vtx->u2 = (float)src_vtx->u2 ;
		//dst_vtx->v2 = (float)src_vtx->v2 ;
		/*--------------------------------------------------------------*/
	}
	/*------------------------------------------------------------------*/
}

static	void	CalcEVM2FVF_UV1(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat)
{
	int					i, j ;
	DG_VERTEX_EVM_FVF	*dst_vtx ;
	DG_VERTEX_EVM		*src_vtx ;
	FMATRIX				*idx_mtx[8] ;
	FMATRIX				*mtx ;
	FVECTOR				src_pos ;
	FVECTOR				src_nrm ;
	FVECTOR				pos ;
	FVECTOR				nrm ;
	FVECTOR				vec ;
	float				wt ;
	short				*src_mtx_idx ;
	short				*weight ;
	short				idx ;

	/*-- 使用MATRIX取得 ------------------------------------------------*/

	{
		FMATRIX	**mtx_ptr ;

		mtx_ptr = idx_mtx ;
		for(i=8; i>0; i--, mtx_ptr++, matrix_list>>=8)
		{
			*mtx_ptr = mat + (matrix_list & 0xff) ;
		}
	}
	/*------------------------------------------------------------------*/

	/*-- 頂点計算 ------------------------------------------------------*/

	dst_vtx = dst ;
	for(i=num; i>0; i--, dst_vtx++, index++)
	{
		src_vtx = src + (*index) ;	// 元頂点

		src_pos.vx = (float)src_vtx->vx * (1.0f/16.0f) ;
		src_pos.vy = (float)src_vtx->vy * (1.0f/16.0f) ;
		src_pos.vz = (float)src_vtx->vz * (1.0f/16.0f) ;
		src_pos.vw = 1.0f ;
		src_nrm.vx = (float)src_vtx->nx ;
		src_nrm.vy = (float)src_vtx->ny ;
		src_nrm.vz = (float)src_vtx->nz ;
		src_nrm.vw = 0.0f ;

		pos.vx = 0.0f ;
		pos.vy = 0.0f ;
		pos.vz = 0.0f ;
		nrm.vx = 0.0f ;
		nrm.vy = 0.0f ;
		nrm.vz = 0.0f ;

		/*-- Weight計算 ------------------------------------------------*/

		src_mtx_idx = src_vtx->index ;
		weight      = src_vtx->weight ;
		for(j=4; j>0; j--, src_mtx_idx++, weight++)
		{
			if( *weight )
			{
				wt = (float)(*weight) * (1.0f/32767.0f) ;
				idx = *src_mtx_idx ;
				ASSERT( (idx >= 0) && (idx < 32) ) ;

				//mtx = idx_mtx[idx >> 2] ;		// こちらの方が正式な書き方
				mtx = *(FMATRIX **)(((DWORD)idx_mtx) + idx) ;

				ApplyMatrix(&vec, mtx, &src_pos) ;
				pos.vx += vec.vx * wt ;
				pos.vy += vec.vy * wt ;
				pos.vz += vec.vz * wt ;

				ApplyMatrix(&vec, mtx, &src_nrm) ;
				nrm.vx += vec.vx * wt ;
				nrm.vy += vec.vy * wt ;
				nrm.vz += vec.vz * wt ;
			}
		}
		_sceVu0Normalize(&nrm, &nrm) ;	// 法線正規化
		/*--------------------------------------------------------------*/

		/*-- 頂点出力 --------------------------------------------------*/

		dst_vtx->vx = pos.vx ;
		dst_vtx->vy = pos.vy ;
		dst_vtx->vz = pos.vz ;

		dst_vtx->nx = nrm.vx ;
		dst_vtx->ny = nrm.vy ;
		dst_vtx->nz = nrm.vz ;

		dst_vtx->u0 = (float)src_vtx->u1 ;
		dst_vtx->v0 = (float)src_vtx->v1 ;
		/*--------------------------------------------------------------*/
	}
	/*------------------------------------------------------------------*/
}

/*	FVF頂点自力計算(DG_EVMTYPE_LARGE)(VertexShader未対応の場合) */
static	void	CalcEVM2FVFLarge(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat)
{
	int					i, j ;
	DG_VERTEX_EVM_FVF	*dst_vtx ;
	DG_VERTEX_EVM		*src_vtx ;
	FMATRIX				*idx_mtx[8] ;
	FMATRIX				*mtx ;
	FVECTOR				src_pos ;
	FVECTOR				src_nrm ;
	FVECTOR				pos ;
	FVECTOR				nrm ;
	FVECTOR				vec ;
	float				wt ;
	short				*src_mtx_idx ;
	short				*weight ;
	short				idx ;

	/*-- 使用MATRIX取得 ------------------------------------------------*/

	{
		FMATRIX	**mtx_ptr ;

		mtx_ptr = idx_mtx ;
		for(i=8; i>0; i--, mtx_ptr++, matrix_list>>=8)
		{
			*mtx_ptr = mat + (matrix_list & 0xff) ;
		}
	}
	/*------------------------------------------------------------------*/

	/*-- 頂点計算 ------------------------------------------------------*/

	dst_vtx = dst ;
	for(i=num; i>0; i--, dst_vtx++, index++)
	{
		src_vtx = src + (*index) ;	// 元頂点

		src_pos.vx = (float)src_vtx->vx ;
		src_pos.vy = (float)src_vtx->vy ;
		src_pos.vz = (float)src_vtx->vz ;
		src_pos.vw = 1.0f ;
		src_nrm.vx = (float)src_vtx->nx ;
		src_nrm.vy = (float)src_vtx->ny ;
		src_nrm.vz = (float)src_vtx->nz ;
		src_nrm.vw = 0.0f ;

		pos.vx = 0.0f ;
		pos.vy = 0.0f ;
		pos.vz = 0.0f ;
		nrm.vx = 0.0f ;
		nrm.vy = 0.0f ;
		nrm.vz = 0.0f ;

		/*-- Weight計算 ------------------------------------------------*/

		src_mtx_idx = src_vtx->index ;
		weight      = src_vtx->weight ;
		for(j=4; j>0; j--, src_mtx_idx++, weight++)
		{
			if( *weight )
			{
				wt = (float)(*weight) * (1.0f/32767.0f) ;

				idx = *src_mtx_idx ;
				ASSERT( (idx >= 0) && (idx < 32) ) ;

				//mtx = idx_mtx[idx >> 2] ;		// こちらの方が正式な書き方
				mtx = *(FMATRIX **)(((DWORD)idx_mtx) + idx) ;

				ApplyMatrix(&vec, mtx, &src_pos) ;
				pos.vx += vec.vx * wt ;
				pos.vy += vec.vy * wt ;
				pos.vz += vec.vz * wt ;

				ApplyMatrix(&vec, mtx, &src_nrm) ;
				nrm.vx += vec.vx * wt ;
				nrm.vy += vec.vy * wt ;
				nrm.vz += vec.vz * wt ;
			}
		}
		_sceVu0Normalize(&nrm, &nrm) ;	// 法線正規化
		/*--------------------------------------------------------------*/

		/*-- 頂点出力 --------------------------------------------------*/

		dst_vtx->vx = pos.vx ;
		dst_vtx->vy = pos.vy ;
		dst_vtx->vz = pos.vz ;

		dst_vtx->nx = nrm.vx ;
		dst_vtx->ny = nrm.vy ;
		dst_vtx->nz = nrm.vz ;

		dst_vtx->u0 = (float)src_vtx->u0 ;
		dst_vtx->v0 = (float)src_vtx->v0 ;
		//dst_vtx->u1 = (float)src_vtx->u1 ;
		//dst_vtx->v1 = (float)src_vtx->v1 ;
		//dst_vtx->u2 = (float)src_vtx->u2 ;
		//dst_vtx->v2 = (float)src_vtx->v2 ;
		/*--------------------------------------------------------------*/
	}
	/*------------------------------------------------------------------*/
}

static	void	CalcEVM2FVFLarge_UV1(DG_VERTEX_EVM_FVF *dst, DG_VERTEX_EVM *src, WORD *index,
						int num, u_long matrix_list, FMATRIX *mat)
{
	int					i, j ;
	DG_VERTEX_EVM_FVF	*dst_vtx ;
	DG_VERTEX_EVM		*src_vtx ;
	FMATRIX				*idx_mtx[8] ;
	FMATRIX				*mtx ;
	FVECTOR				src_pos ;
	FVECTOR				src_nrm ;
	FVECTOR				pos ;
	FVECTOR				nrm ;
	FVECTOR				vec ;
	float				wt ;
	short				*src_mtx_idx ;
	short				*weight ;
	short				idx ;

	/*-- 使用MATRIX取得 ------------------------------------------------*/

	{
		FMATRIX	**mtx_ptr ;

		mtx_ptr = idx_mtx ;
		for(i=8; i>0; i--, mtx_ptr++, matrix_list>>=8)
		{
			*mtx_ptr = mat + (matrix_list & 0xff) ;
		}
	}
	/*------------------------------------------------------------------*/

	/*-- 頂点計算 ------------------------------------------------------*/

	dst_vtx = dst ;
	for(i=num; i>0; i--, dst_vtx++, index++)
	{
		src_vtx = src + (*index) ;	// 元頂点

		src_pos.vx = (float)src_vtx->vx ;
		src_pos.vy = (float)src_vtx->vy ;
		src_pos.vz = (float)src_vtx->vz ;
		src_pos.vw = 1.0f ;
		src_nrm.vx = (float)src_vtx->nx ;
		src_nrm.vy = (float)src_vtx->ny ;
		src_nrm.vz = (float)src_vtx->nz ;
		src_nrm.vw = 0.0f ;

		pos.vx = 0.0f ;
		pos.vy = 0.0f ;
		pos.vz = 0.0f ;
		nrm.vx = 0.0f ;
		nrm.vy = 0.0f ;
		nrm.vz = 0.0f ;

		/*-- Weight計算 ------------------------------------------------*/

		src_mtx_idx = src_vtx->index ;
		weight      = src_vtx->weight ;
		for(j=4; j>0; j--, src_mtx_idx++, weight++)
		{
			if( *weight )
			{
				wt = (float)(*weight) * (1.0f/32767.0f) ;

				idx = *src_mtx_idx ;
				ASSERT( (idx >= 0) && (idx < 32) ) ;

				//mtx = idx_mtx[idx >> 2] ;		// こちらの方が正式な書き方
				mtx = *(FMATRIX **)(((DWORD)idx_mtx) + idx) ;

				ApplyMatrix(&vec, mtx, &src_pos) ;
				pos.vx += vec.vx * wt ;
				pos.vy += vec.vy * wt ;
				pos.vz += vec.vz * wt ;

				ApplyMatrix(&vec, mtx, &src_nrm) ;
				nrm.vx += vec.vx * wt ;
				nrm.vy += vec.vy * wt ;
				nrm.vz += vec.vz * wt ;
			}
		}
		_sceVu0Normalize(&nrm, &nrm) ;	// 法線正規化
		/*--------------------------------------------------------------*/

		/*-- 頂点出力 --------------------------------------------------*/

		dst_vtx->vx = pos.vx ;
		dst_vtx->vy = pos.vy ;
		dst_vtx->vz = pos.vz ;

		dst_vtx->nx = nrm.vx ;
		dst_vtx->ny = nrm.vy ;
		dst_vtx->nz = nrm.vz ;

		dst_vtx->u0 = (float)src_vtx->u1 ;
		dst_vtx->v0 = (float)src_vtx->v1 ;
		/*--------------------------------------------------------------*/
	}
	/*------------------------------------------------------------------*/
}
