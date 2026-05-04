//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xevmobjs.c
	マルチウェイトエンベロープオブジェクト処理ルーチン

	2002/03/15 K.Takabe
	$Id: xevmobjs.c,v 1.1.1.3 2002/11/19 11:42:35 Yoshizawa1 Exp $

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

#ifdef KP_XBOX //BP


#ifdef KP_XBOX
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


static DG_VERTEXSHADER	DG_EvmVertexShader[3];
static DG_VERTEXFORMAT	DG_EvmVertexFormat[1] ;
extern DG_PIXELSHADER	DG_MultiTexPixelShader[DG_MULTITEX_MAXNUM] ;

extern unsigned char VERTEX_SHADER_evm[];
extern unsigned char VERTEX_SHADER_evm_emap[];
extern unsigned char VERTEX_SHADER_evm_bmap[];
void DG_InitEvmVertexShader(void)
{
#if 0 //BP_RENDER
	DWORD dwEvmDecl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),		/* pos */
		D3DVSD_REG(2,  D3DVSDT_NORMSHORT3),	/* norm */
		D3DVSD_REG(7,  D3DVSDT_NORMSHORT3),	/* uv0 */
		D3DVSD_REG(8,  D3DVSDT_NORMSHORT3),	/* uv1 */
		D3DVSD_REG(9,  D3DVSDT_NORMSHORT3),	/* uv2 */
		D3DVSD_REG(11, D3DVSDT_SHORT4), /* blend */
		D3DVSD_REG(12, D3DVSDT_SHORT4), /* index */
		D3DVSD_END()
	};
	/* 頂点フォーマットを生成する */
	DG_MakeVertexFormat( &DG_EvmVertexFormat[0], dwEvmDecl );

	DG_MakeVertexShader( &DG_EvmVertexShader[0], VERTEX_SHADER_evm, dwEvmDecl );
	DG_MakeVertexShader( &DG_EvmVertexShader[1], VERTEX_SHADER_evm_emap, dwEvmDecl );
	DG_MakeVertexShader( &DG_EvmVertexShader[2], VERTEX_SHADER_evm_bmap, dwEvmDecl );
#endif
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
	FVECTOR		scale ;
	FMATRIX		world ;				/* オブジェクトマトリクス */
	FMATRIX		screen ;			/* カメラへの透視変換用マトリクス */
	FMATRIX		light[2] ;			/* ライトマトリクス */
	FMATRIX		support_mat[2] ;	/* 拡張用 */
	FMATRIX		tmp_mat ;			/* 一時使用マトリクス */
	FMATRIX		ir_light_mat[2] ;
	FVECTOR		tmp_vec[4] ;		/* 一時使用ベクトル */
	u_long64		matrix_list ;		/* 転送済みマトリクス情報（８ビット×８個分） */
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
extern u_long128	DG_AS_GetLocalMemoryAddress()[1024] ;

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
static inline int SetEvmMatrix( u_long64 matrix_list, FMATRIX *mat )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	int		i, num, matrix_trans_flag = 0 ;
	void		**last_matrix_addr, *addr ;

	/* 前回と違う場合のみ転送するようにする */
	last_matrix_addr = scrpad->last_matrix_addr ;
	for ( i = 0 ; i < 8 ; i++ ){
		num = matrix_list & 0xff ;
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

	DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);

	/* 頂点シェーダー設定 */
	//DG_SetVertexShader( DG_EvmVertexShader[0].handle );
	DG_ClearVertexShader();
	DG_LoadVertexShader( &DG_EvmVertexShader[0] );
	DG_LoadVertexShader( &DG_EvmVertexShader[1] );
	DG_LoadVertexShader( &DG_EvmVertexShader[2] );
}

/* 各種セットアップルーチン */
static void EvmobjSetup( DG_EVMOBJ *evmobj )
{
	static FVECTOR	model_scale = {1.0f/4096.0f, 1.0f/255.0f, 1.0f/16, 1.0f/32767.0f};
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

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
/* ＤＭＡ接続ルーチン  */
static void ChainEvmObj( DG_EVMOBJ *evmobj )
{
#if 0
	int		i, vu_prog_addr, nowait_count = 0 ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	DG_EVMPACK	*pack ;
	u_long128	*verts, *norms, *uvs0, *uvs1, *uvs2, *weight ;
	PACKET_BUFFER	*buff ;
	FMATRIX		*mats ;
	EVM_PACK	*evmpack ;

	/* マトリクス転送セットアップ */
	SetMatrixInitPacket( &scrpad->param_packet );
	/* メモリ書き出し（最終的にはＤＭＡを使用する） */
	GV_PREFECH( evmobj->packs );
	DG_SendCacheFIFO( scrpad->local_work, &scrpad->param_packet, SIZEOF_QWORD(PARAM_MAT_PACKET) );

	/* データバッファへのアドレス取得 */
	verts = evmobj->verts_ptr ;
	norms = evmobj->norms_ptr ;
	uvs0 = evmobj->uvs0_ptr ;
	uvs1 = evmobj->uvs1_ptr ;
	uvs2 = evmobj->uvs2_ptr ;
	weight = evmobj->weight_ptr ;
	mats = evmobj->matrix[ evmobj->use_buffer ] ;
	/* モデルタイプチェック */
	if ( evmobj->def->type & DG_EVMTYPE_LARGE ){
		vu_prog_addr = scrpad->vu_prog_list[1] ;
	} else {
		vu_prog_addr = scrpad->vu_prog_list[0] ;
	}

	/* バウンディング設定 */
	scrpad->dma_packet[ 0 ].evm_packet.param.header.bound_mode = scrpad->bound_mode ;
	scrpad->dma_packet[ 1 ].evm_packet.param.header.bound_mode = scrpad->bound_mode ;

	pack = evmobj->packs ;
	evmpack = evmobj->def->packet ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model.n_packs += evmobj->n_packet ;
#endif
	for ( i = evmobj->n_packet ; i > 0 ; i--, pack++, evmpack++ ){
		/* バッファの設定処理 */
		GV_PREFECH( pack + 1 );
		buff = &scrpad->dma_packet[ scrpad->packet_clock ] ;
		scrpad->packet_clock = 1 - scrpad->packet_clock ;
		/* ＤＭＡタグ作成 */
		verts = SetPacketParam1( &buff->evm_packet.verts_trans, verts, 0, pack->n_step );
		norms = SetPacketParam1( &buff->evm_packet.norms_trans, norms, 1, pack->n_step );
		uvs0 = SetPacketParam1( &buff->evm_packet.uvs0_trans, uvs0, 2, pack->n_uv_step[0] );
		uvs1 = SetPacketParam1( &buff->evm_packet.uvs1_trans, uvs1, 3, pack->n_uv_step[1] );
		uvs2 = SetPacketParam1( &buff->evm_packet.uvs2_trans, uvs2, 4, pack->n_uv_step[2] );
		//weight = SetPacketParam1( &buff->evm_packet.weight_trans, weight, 5, pack->n_step * 2 );
		weight = SetPacketParam4( &buff->evm_packet.weight_trans, weight, 5, pack->n_step );
		buff->evm_packet.param.header.n_verts = pack->n_verts ;
		buff->evm_packet.param.header.n_mat = pack->max_mats ;
		buff->evm_packet.param.header.flag = pack->flag ;
#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_verts += pack->n_verts ;
#endif
		/* テクスチャパラメータ転送パケットの設定 */
		SetPacketParam2( &buff->tex_packet.tex0_trans, pack->tex0_ptr, 0 );
		SetPacketParam2( &buff->tex_packet.tex1_trans, pack->tex1_ptr, 1 );
		SetPacketParam2( &buff->tex_packet.tex2_trans, pack->tex2_ptr, 2 );
		/* マトリクス転送パケットの設定 */
		if ( SetPacketParam3( buff->mat_packet.skel_mat_trans, pack->matrix_list, mats ) ){
			/* マトリクスの転送が発生する場合にはＶＵ実行終了待ちを入れる */
			buff->prog_wait.vifcode[1] = SCE_VIF1_SET_FLUSHE( 0 ) ;
		} else {
			/* マトリクスの転送が発生しない場合にはＶＵ実行終了待ちを入れない */
			buff->prog_wait.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
			nowait_count++ ;
		}
		/* 実行プログラム選択 */
		buff->prog_exec.vifcode[1] = SCE_VIF1_SET_MSCAL( vu_prog_addr,0 ) ;
		//buff->prog_exec.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
		DG_SendCacheFIFO( scrpad->local_work, buff, SIZEOF_QWORD(PACKET_BUFFER) );
	}
#else
	int		i, vu_prog_addr, nowait_count = 0, no_tex_flag = 0 ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	DG_EVMPACK	*pack ;
	FMATRIX		*mats ;
	EVM_PACK	*evmpack ;

	mats = evmobj->matrix[ evmobj->use_buffer ] ;

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
			DG_SelectVertexShader( &DG_EvmVertexShader[0], &DG_EvmVertexFormat[0] );
			DG_SetTexture(0, NULL );
			DG_SetTexture(1, NULL );
			DG_SetTexture(2, NULL );
			DG_SetAlphaMode( 0 );
		}
	}

	pack = evmobj->packs ;
	evmpack = evmobj->def->packet ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model.n_packs += evmobj->n_packet ;
#endif

	//DG_SetPixelShader( NULL );
	//DG_SetVertexShader( DG_EvmVertexShader[0].handle );
	//DG_SelectVertexShader( &DG_EvmVertexShader[0], NULL );
	///* 頂点バッファの記録方法については今後改良の予定・・・ */
	//DG_SetVertexBuffer( 0, *(void**)&(evmobj->def->skeleton[ 0 ].pad0), sizeof(DG_VERTEX_EVM) );
	DG_SetVertexBuffer( 0, evmobj->vbuff, sizeof(DG_VERTEX_EVM) );
	DG_SetVertexStream( &DG_EvmVertexFormat[0] );
	for ( i = evmobj->n_packet ; i > 0 ; i--, pack++, evmpack++ ){
#if 0
		pack->n_verts ;
		pack->max_mats ;
		pack->flag ;
#endif
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}

		if ( no_tex_flag == 0 ){
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
				DG_SelectVertexShader( &DG_EvmVertexShader[0], &DG_EvmVertexFormat[0] );
				break ;
			  case DG_MULTITEX_EMAP:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP] );
				DG_SelectVertexShader( &DG_EvmVertexShader[1], &DG_EvmVertexFormat[0] );
				break ;
			  case DG_MULTITEX_BMAP:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_BMAP] );
				DG_SelectVertexShader( &DG_EvmVertexShader[2], &DG_EvmVertexFormat[0] );
				DG_SetTexture(2, (DG_TEX_TRANS*)pack->tex1_ptr );	/* エンボス用に同じテクスチャをセット */
				break ;
			  case DG_MULTITEX_EMAP2:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP2] );
				DG_SelectVertexShader( &DG_EvmVertexShader[1], &DG_EvmVertexFormat[0] );
				DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
				break ;
			  case DG_MULTITEX_EMAP3:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP3] );
				DG_SelectVertexShader( &DG_EvmVertexShader[1], &DG_EvmVertexFormat[0] );
				DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
				break ;
			  case DG_MULTITEX_EMAP4:
				DG_SetPixelShader( &DG_MultiTexPixelShader[DG_MULTITEX_EMAP4] );
				DG_SelectVertexShader( &DG_EvmVertexShader[1], &DG_EvmVertexFormat[0] );
				break ;
			}
		}

		/* マトリクスの設定 */
		SetEvmMatrix( pack->matrix_list, mats );

		/* パケット単位で細かく頂点シェーダーを切り替えながら実行する必要有り */
		DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, evmpack->n_indices, evmpack->index );
#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_verts += pack->n_verts ;
#endif
	}

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
	
#endif
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
	int		old_tex_code, size, last_semi_trans = 0, use_semi_trans = 0 ;

	MARK( __FILE__ );
	if ( cp->obj_queue == NULL ) return ;
	obj_buff = &cp->obj_queue->evmobj_buffer ;

	c_gid = cp->group_id ;

	/* スクラッチパッドワークの初期化 */
	scrpad->pers_mat = cp->pers ;
	scrpad->eye_inv = cp->eye_inv ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->eye_xpers = cp->eye_xpers ;
	scrpad->scale.vx = cp->width / 2 ;
	scrpad->scale.vy = cp->height / 2 ;
	scrpad->packet_clock = 0 ;
	scrpad->count = 0 ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model = DG_PerformanceData.mulwt_model ;
#endif
	{/* 赤外線モードライトマトリクス生成 */
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

	/* オブジェクトのリストをスクラッチパッドにコピー */
	invisible_flag = DG_EVMOBJ_INVISIBLE0 << cp->chanl_num ;
	max_objs = 0 ;
	que = (DG_EVMOBJ**)obj_buff->queue ;
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	for ( i = obj_buff->n_queue ; i > 0 ; i--, que++ ){
		evmobj = *que ;
		if ( !( evmobj->group_id & cp->group_id ) ) continue ;
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
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
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
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
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
		EvmobjSetup( evmobj );

		/* 描画 */
#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_obj++ ;
#endif
		ChainEvmObj( evmobj );

		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
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
	_DG_ChainEvmChanl( cp, which, 0 );
}

void DG_ChainEvmChanlLatter( DG_CHANL *cp, int which )
{
	_DG_ChainEvmChanl( cp, which, 1 );
}


/*----------------------------------------------------------------*/
static int SetMultiTexType( DG_TEX *tex0, DG_TEX *tex1, DG_TEX *tex2 )
{
	int		type ;

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
		マルチウェイトエンベロープ対応オブジェクト作成
	*/
DG_EVMOBJ *DG_MakeEvmObj( EVM_DEF *def, int flag, int chanl )
{
	int		buf_size ;
	int		i, n_skeletons ;
	DG_EVMOBJ	*evmobj ;
	DG_EVMPACK	*evmpack ;
	EVM_PACK	*pack ;

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

	evmobj->vbuff = def->vbuff ;

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
	pack = def->packet ;
	evmpack = evmobj->packs ;
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
		evmpack->matrix_list = *(long64*)&pack->mat_id[0] ;
		evmpack++ ;
		pack++ ;
	}
	return evmobj ;
}

	/*
		オブジェクトの開放
	*/
void DG_FreeEvmObj( DG_EVMOBJ *evmobj )
{
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
















































#if 0	
	/* スクラッチパッドのオブジェクトをテクスチャ毎に順番に登録 */
	for ( tex_list = DG_TextureList, j = DG_MaxTextures ; j > 0 ; tex_list++, j-- ){
		int		old_tex_code ;
		if ( tex_list->header->compress_flag & TRI_FLAG_TRANS ) continue ;
#ifdef LIBDG_CHECK_MULTILOADTEX
		//if ( GV_Time & 2 ) if ( tex_list->flag ) continue ;
#endif
		old_tex_code = -1 ;
		for ( i = 0 ; i < scrpad->max_obj_list ; i++ ) {
			/* ＴＲＩに対応したオブジェクトを検索する */
			if ( tex_list->code != scrpad->obj_list[i].tri_code ) continue ;
			if ( old_tex_code == -1 ){
				int		size ;
				old_tex_code = scrpad->obj_list[i].tri_code ;
#ifdef LIBDG_PERFORMANCE
				DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
				tex_list->flag = 1 ;
				//size = DG_WriteTextureChangePacks( scrpad->datas, &tex_list->tex_packet[which] );
				size = DG_WriteTextureChangePacks2( scrpad->datas, tex_list, which );
				DG_EndSprToMem();
				DG_StartSprToMem( DG_CurrentDmaAddr, &scrpad->datas, size );
				DG_CurrentDmaAddr += size ;
			}
			evmobj = scrpad->obj_list[i].obj ;

			/* マトリクス設定 */
			if ( evmobj->root != NULL ) evmobj->world = *( evmobj->root ) ;
			scrpad->world = evmobj->world ;

			/* バウンディングチェック */
			_sceVu0MulMatrix( &scrpad->screen, &scrpad->eye_pers, &scrpad->world );
			bound = BoundCheck( &scrpad->screen, &evmobj->def->lx );
			if ( bound == 2 ) continue ;

			/* オブジェクト描画パケット初期化関連 */
			scrpad->matrix_list = 0xffffffffffffffff ;
			scrpad->last_matrix_addr[0] = NULL ;
			scrpad->last_matrix_addr[1] = NULL ;
			scrpad->last_matrix_addr[2] = NULL ;
			scrpad->last_matrix_addr[3] = NULL ;
			scrpad->last_matrix_addr[4] = NULL ;
			scrpad->last_matrix_addr[5] = NULL ;
			scrpad->last_matrix_addr[6] = NULL ;
			scrpad->last_matrix_addr[7] = NULL ;

			/* マトリクスセットアップ */
			MatrixSetup( evmobj );

			/* 描画 */
#ifdef LIBDG_PERFORMANCE
			scrpad->mulwt_model.n_obj++ ;
#endif
			ChainEvmObj( evmobj );
		}
	}
#endif

#endif
