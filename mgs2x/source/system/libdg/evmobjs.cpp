/*
	evmobjs.c
	マルチウェイトエンベロープオブジェクト処理ルーチン

	2000/01/25 K.Takabe
	$Id: evmobjs.cpp,v 1.1.1.3 2002/11/19 11:42:08 Yoshizawa1 Exp $

*/
/*

	DG_EVMOBJ		*DG_MakeEvmObj(def, flag, chanl)
	EVM_DEF	*def ;		モデルデータ
	int		flag ;		処理フラグ
	int		chanl ;		描画チャンネル

		あるモデルデータに対応する、オブジェクトを作成する

	void		DG_FreeEvmObjs(objs)
	DG_EVMOBJ	*objs ;		物体ハンドラ

		物体ハンドラと、そのパケットメモリを解放する

	void DG_WriteEvmMdlPaketUV(int tri_code, EVM_PACK *pack)
	int			tri_code ;	使用するＴＲＩファイルのＩＤ
	EVM_PACK	*pack ;		補正するオブジェクトパケット

		テクスチャからＵＶ値を補正する

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "shader.h"

//-----------------------------------------------------------------------------

enum {
	VS_LIGHT_OFF,
	VS_LIGHT_ON,	
};
static DWORD DG_EvmVertexShaderHandle[2];

void DG_InitEvmVertexShader(void)
{
	DWORD dwEvmDecl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),
		D3DVSD_REG(2,  D3DVSDT_FLOAT3),
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR),
		D3DVSD_REG(7,  D3DVSDT_FLOAT2),
		D3DVSD_REG(8,  D3DVSDT_FLOAT2),
		D3DVSD_REG(9,  D3DVSDT_FLOAT2),
		D3DVSD_REG(10, D3DVSDT_FLOAT2),	
		D3DVSD_REG(11, D3DVSDT_FLOAT4), // blend
		D3DVSD_REG(12, D3DVSDT_FLOAT4), // index
		D3DVSD_END()
	};
	DG_CreateVertexShader(GV_StrCode("evm"),  // 光源計算無し
						  dwEvmDecl,
						  &DG_EvmVertexShaderHandle[VS_LIGHT_OFF]);
	DG_CreateVertexShader(GV_StrCode("evml"), // 光源計算有り
						  dwEvmDecl,
						  &DG_EvmVertexShaderHandle[VS_LIGHT_ON]);
}

//-----------------------------------------------------------------------------

// ＥＶＭオブジェクト専用ＤＭＡ転送パケット構造体
typedef ALIGN16_DECL(struct) _evm_dma_packet {
	unsigned int    verts_trans;    /* 頂点転送ＤＭＡタグ */
	unsigned int    norms_trans;    /* 法線転送ＤＭＡタグ */
	unsigned int    uvs0_trans;     /* ＵＶ０転送ＤＭＡタグ */
	unsigned int    uvs1_trans;     /* ＵＶ１転送ＤＭＡタグ */
	unsigned int    uvs2_trans;     /* ＵＶ２転送ＤＭＡタグ */
	unsigned int    weight_trans;   /* ウェイト値転送ＤＭＡタグ */
	unsigned int    param_trans;    /* パラメータ転送ＤＭＡタグ */
	struct _trans_param {
		struct _header {
			int n_verts;    /* 頂点数 */
			int work_end;   /* リザーブ */
			int n_mat;      /* 使用最大マトリクス数 */
			int flag;       /* フラグ */
			int pad;
			int tex0_reglist;	/* ＴＥＸ０描画用レジスタリスト */
			int tex1_reglist;	/* ＴＥＸ１描画用レジスタリスト */
			int tex2_reglist;	/* ＴＥＸ２描画用レジスタリスト */
		} header;
		u_long64 clip_tag;	/* クリッピングポリゴン用ＧＩＦタグ */
		u_long64 draw_tag;	/* 通常描画用ＧＩＦタグ */
	} param;
} EVM_DMA_PACKET ;

/* テクスチャ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _tex_dma_packet {
	unsigned int		tex0_trans;		/* テクスチャ０転送ＤＭＡパケット */
	unsigned int		tex1_trans;		/* テクスチャ１転送ＤＭＡパケット */
	unsigned int		tex2_trans;		/* テクスチャ２転送ＤＭＡパケット */
} TEX_DMA_PACKET ;

/* マトリクス転送パケット構造体 */
typedef ALIGN16_DECL(struct) _mat_dma_packet {
	unsigned int		skel_mat_trans[8];	/* マトリクス転送パケット */
} MAT_DMA_PACKET ;


/* オブジェクト固有設定転送パケット */
typedef ALIGN16_DECL(struct) _param_mat_packet {
	unsigned int	trans_wait;
	unsigned int	param_mat_trans;
	struct _param_mat{
		MATRIX		option_mat[2];		/* 将来の拡張用マトリクス */
		MATRIX		local_light;		/* ローカルライトマトリクス */
		MATRIX		local_color;		/* ローカルカラーマトリクス */
	} param_mat;
} PARAM_MAT_PACKET ;

/* ＤＭＡ転送データパケット */
typedef ALIGN16_DECL(struct) _packet_buffer {
	EVM_DMA_PACKET		evm_packet;
	TEX_DMA_PACKET		tex_packet;
	unsigned int			prog_wait;			/* プログラム転送終了待ちタグ */
	MAT_DMA_PACKET		mat_packet;
	unsigned int			prog_exec;			/* プログラム実行開始タグ */
} PACKET_BUFFER ;


//-----------------------------------------------------------------------------

	/*
		スクラッチパッド上ワークデータ構造体
	*/
typedef struct _obj_list {
	void	*obj;
	int		tri_code;
} OBJ_LIST;

typedef struct _scrpad_work {
	MATRIX		pers_mat;
	MATRIX		eye_pers;
	MATRIX		eye_inv;
	VECTOR		scale;
	MATRIX		world;				/* オブジェクトマトリクス */
	MATRIX		screen;			/* カメラへの透視変換用マトリクス */
	MATRIX		light[2];			/* ライトマトリクス */
	MATRIX		support_mat[2];	/* 拡張用 */
	MATRIX		tmp_mat;			/* 一時使用マトリクス */
	VECTOR		tmp_vec[4];		/* 一時使用ベクトル */
	MATRIX		xpers_mat;		/* XBOXT&L用 */
	MATRIX		eye_xpers;		/* XBOXT&L用 */
	PARAM_MAT_PACKET	param_packet;		/* オブジェクト固有マトリクスパラメータパケット */
	PACKET_BUFFER		dma_packet[2];		/* ＤＭＡパケット（ダブルバッファ） */
	u_long64			dma_buffer[64];	/* 汎用パケット生成エリア */
	u_long64		matrix_list;		/* 転送済みマトリクス情報（８ビット×８個分） */
	int			packet_clock;		/* ダブルバッファ切り替え用クロック */
	short		vu_prog_list[8];
	int			count;					/* デバッグ用 */
	void		*last_matrix_addr[8];	/* 転送マトリクスアドレス記憶用 */

	//MATRIX      matrix_buf[256];        /* 転送用マトリクス */
	//MATRIX      *matrix_buf;            /* 転送用マトリクス */	
	/* ローカルワーク */
	u_long64	local_work[0];
} ScrpadWork;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
u_long64	DG_AS_GetLocalMemoryAddress()[1024*2];
//extern u_long64	DG_AS_GetLocalMemoryAddress()[1024];

//-----------------------------------------------------------------------------

/* バウンディングチェック */
static int BoundCheck(MATRIX *mat, float *bound)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	//int        and_flag, or_flag, prim_over_flag;

	// バウンディング用ベクトル生成
	scrpad->tmp_vec[0].vx = bound[0];
	scrpad->tmp_vec[0].vy = bound[1];
	scrpad->tmp_vec[0].vz = bound[2];
	scrpad->tmp_vec[1].vx = bound[3];
	scrpad->tmp_vec[1].vy = bound[4];
	scrpad->tmp_vec[1].vz = bound[5];

	// バウンディングチェックマイクロプログラム呼び出し
	/*
	 * vf4～vf7 = mat;
	 * vf1 = tmp_vec[0] = bound[0]～[2]
	 * vf2 = tmp_vec[1] = bound[3]～[5]
	 * vf3 = scrpad->scale
	 */

	// 計算結果の取得
	// and_flag, or_flag, prim_over_flag, scrpad->tmp_vec が返ってくる
#if 0
	if (and_flag & 0x3f) return 2; /* 画面外 */
	if (or_flag & 0x30) return 1;  /* クリップ必要 */
	if (prim_over_flag) return 1;  /* クリップ必要 */
#endif
	return 0;	/* 完全画面内 */
}

//-----------------------------------------------------------------------------

static inline void SetPacketParam3(unsigned int *tag, u_long64 matrix_list, MATRIX *mat)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	int        i, num;
	void       **last_matrix_addr, *addr;

	// 前回と違う場合のみ転送するようにする
	last_matrix_addr = scrpad->last_matrix_addr;
	for (i = 0; i < 8; i ++) {
		num = (int)(matrix_list & 0xff);
		addr = mat + num;
		if (num != 0xff && (*last_matrix_addr) != addr) {
			// マトリクスアドレスが異なる場合にはマトリクスの転送を行なう
			*last_matrix_addr = addr;
		} else {
			// マトリクスが同じ又は未使用の場合は転送を行なわない
		}
		matrix_list >>= 8;
		last_matrix_addr ++;
		tag ++;
	}
}

//-----------------------------------------------------------------------------

static void PrintMatrix(char *str, MATRIX *mat)
{
	printf(str);
	printf("%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n"
		   "%.3f %.3f %.3f %.3f\n",
		   mat->m[0][0], mat->m[0][1], mat->m[0][2], mat->m[0][3],
		   mat->m[1][0], mat->m[1][1], mat->m[1][2], mat->m[1][3],
		   mat->m[2][0], mat->m[2][1], mat->m[2][2], mat->m[2][3],
		   mat->m[3][0], mat->m[3][1], mat->m[3][2], mat->m[3][3]);
}

// EVMオブジェクトに初期値を設定する
static void EvmobjSetup(DG_EVMOBJ *evmobj)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;

	/* オブジェクト描画パケット初期化関連 */
	scrpad->matrix_list = 0xffffffffffffffff;
	scrpad->last_matrix_addr[0] = NULL;
	scrpad->last_matrix_addr[1] = NULL;
	scrpad->last_matrix_addr[2] = NULL;
	scrpad->last_matrix_addr[3] = NULL;
	scrpad->last_matrix_addr[4] = NULL;
	scrpad->last_matrix_addr[5] = NULL;
	scrpad->last_matrix_addr[6] = NULL;
	scrpad->last_matrix_addr[7] = NULL;

	/* 光源計算 */
	scrpad->light[0] = evmobj->light[0];
	scrpad->light[1] = evmobj->light[1];

	if (DG_DisplayStatus & DG_STATE_IR_MODE) {     // 赤外線モード
		//u_long64 prim;
		if (evmobj->flag & DG_EVMOBJ_IRREACTION) { // 赤外線に反応するモデル
			*(VECTOR*)&scrpad->light[0].m[0][0] = DG_ZeroVector;
			*(VECTOR*)&scrpad->light[0].m[1][0] = DG_ZeroVector;
			*(VECTOR*)&scrpad->light[0].m[2][0] = DG_ZeroVector;
			scrpad->light[1].m[3][0] = 255.0f;
			scrpad->light[1].m[3][1] = 64.0f;
			scrpad->light[1].m[3][2] = 64.0f;
			// PS2/プリミティブタイプの設定
		} else {
			// PS2/プリミティブタイプの設定
		}
	}
}

// ＤＭＡ接続ルーチン
static void ChainEvmObj(DG_EVMOBJ *evmobj)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	int           i, j;
	DG_EVMPACK    *pack;
	u_long64        *verts, *norms, *uvs0, *uvs1, *uvs2, *weight;
	//PACKET_BUFFER *buff;
	MATRIX        *mats;
	EVM_PACK      *evmpack;
	DG_VBUF_INFO  *info;
	DG_INDEX_INFO *index;
	
	// データバッファへのアドレス取得
	verts = (u_long64 *)evmobj->verts_ptr;
	norms = (u_long64 *)evmobj->norms_ptr;
	uvs0 = (u_long64 *)evmobj->uvs0_ptr;
	uvs1 = (u_long64 *)evmobj->uvs1_ptr;
	uvs2 = (u_long64 *)evmobj->uvs2_ptr;
	weight = (u_long64 *)evmobj->weight_ptr;
	mats = (MATRIX *)evmobj->matrix[evmobj->use_buffer];

	pack = evmobj->packs;
	evmpack = evmobj->def->packet;
	//info = (DG_VBUF_INFO *)evmpack->index->vbuf_info;
	//index = (DG_INDEX_INFO *)info->index;
	index = (DG_INDEX_INFO *)evmpack->index;
	info = index->vbuf_info;

	DG_SetStreamSource(0, info->vbuf, info->stride);
	DG_SetVertexShader(DG_EvmVertexShaderHandle[VS_LIGHT_ON]);
	DG_SetIndices(info->ibuf, 0);

	// シェーダに渡す定数
	DG_SetVertexShaderConstantMatrix(CV_EYEPERS, &scrpad->eye_xpers, 1);
	DG_SetVertexShaderConstantMatrix(CV_LIGHTVEC, &evmobj->light[0], 1);
	DG_SetVertexShaderConstant(CV_LIGHTCOL, &evmobj->light[1], 4);
	
	for (i = evmobj->n_packet; i > 0; i --, pack++, evmpack++, index++) {
		if ( pack->tex0_ptr != NULL ){
			DG_SetTexture(0, ((DG_TEX_TRANS *)pack->tex0_ptr)->ptex);
			DG_SetAlphaMode(((DG_TEX_TRANS *)pack->tex0_ptr)->alpha.data);
		}

		// 8個の行列を転送する
		// TODO : 同じ行列は転送しない処理を入れる
		for (j = 0; j < 8; j++) {
			DG_SetVertexShaderConstantMatrix(CV_WORLD0 + j * 4, &mats[evmpack->mat_id[j]], 1);
		}
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, index->min, index->num,
								index->start, index->prims);
	}
}

//-----------------------------------------------------------------------------

// マルチウェイトエンベロープ対応オブジェクト処理
static void _DG_ChainEvmChanl(DG_CHANL *cp, int which, int draw_mode)
{
	ScrpadWork *scrpad = (ScrpadWork *)SCRPAD_ADDR;
	DG_OBJ_BUFFER   *obj_buff;
	DG_EVMOBJ       *evmobj, **que;
	//int             i, j, gid, c_gid, bound, invisible_flag, max_mem_objs, max_objs;
	//DG_TEXTURE_LIST *tex_list;
	int             i, c_gid, bound, invisible_flag, max_mem_objs, max_objs;
	OBJ_LIST        *obj_list, *mem_obj_list;
	//int             old_tex_code, size;

	MARK( __FILE__ );
	DG_SetDrawMark();
	if (cp->obj_queue == NULL) return;
	obj_buff = &cp->obj_queue->evmobj_buffer;

	c_gid = cp->group_id;

	// スクラッチパッドワークの初期化
	scrpad->pers_mat = cp->pers;
	scrpad->eye_inv = cp->eye_inv;
	scrpad->eye_pers = cp->eye_pers;
	scrpad->scale.vx = cp->width / 2.0f;
	scrpad->scale.vy = cp->height / 2.0f;
	scrpad->packet_clock = 0;
	scrpad->count = 0;
	scrpad->xpers_mat = cp->xpers;
	scrpad->eye_xpers = cp->eye_xpers;

	// オブジェクトのリストをスクラッチパッドにコピー
	invisible_flag = DG_EVMOBJ_INVISIBLE0 << cp->chanl_num;
	max_objs = 0;
	que = (DG_EVMOBJ **)obj_buff->queue;
	obj_list = (OBJ_LIST *)scrpad->local_work;

	for (i = obj_buff->n_queue; i > 0; i --, que ++) {
		evmobj = *que;
		if (!(evmobj->group_id & cp->group_id)) continue;
		if (evmobj->flag & invisible_flag) continue;
		// 描画フェーズコントロール
		if (draw_mode == 0){
			if (evmobj->flag & DG_EVMOBJ_LATTERDRAW) continue;
		} else {
			if (!(evmobj->flag & DG_EVMOBJ_LATTERDRAW)) continue;
		}
		obj_list[max_objs].obj = evmobj;
		obj_list[max_objs].tri_code = evmobj->def->texture;
		max_objs ++;
	}

	if (max_objs == 0) return;

	// スクラッチパッド上のオブジェクトリストをテクスチャ毎にソートしてメモリに書き出す
	mem_obj_list = (OBJ_LIST *)DG_AS_GetLocalMemoryAddress();
	max_mem_objs = 0;
#if 0
	for (tex_list = DG_TextureList, j = DG_MaxTextures; j > 0; tex_list ++, j --){
		if (tex_list->header->compress_flag & TRI_FLAG_TRANS) continue;
		for (i = 0; i < max_objs; i++) {
			if (tex_list->code != obj_list[i].tri_code) continue;
			mem_obj_list->obj = obj_list[i].obj;
			/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list->tri_code = (int)tex_list;
			mem_obj_list++;
			max_mem_objs++;
		}
	}
	if (max_mem_objs == 0) return;
#else
	// 仮で全部書き出す
	for (i = 0; i < max_objs; i ++) {
		mem_obj_list->obj = obj_list[i].obj;
		/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
		mem_obj_list->tri_code = NULL; // TODO:仮
		mem_obj_list ++;
		max_mem_objs ++;
	}
#endif
	//printf("max_mem_objs = %d\n", max_mem_objs);

	//DG_BeginScene();

	//TMP
	DG_SetRenderState(D3DRS_FOGENABLE, FALSE);

	// ソート済みオブジェクトをＤＭＡにつなぐ
	mem_obj_list = (OBJ_LIST *)DG_AS_GetLocalMemoryAddress();
	for (i = max_mem_objs; i > 0; i--, mem_obj_list ++) {
		evmobj = (DG_EVMOBJ *)mem_obj_list->obj;

		/* マトリクス設定 */
		if (evmobj->root != NULL) {
			evmobj->world = *evmobj->root;
		}
		scrpad->world = evmobj->world;

		/* バウンディングチェック */
		D3DXMatrixMultiply(&scrpad->screen, &scrpad->world, &scrpad->eye_pers);
		bound = BoundCheck(&scrpad->screen, &evmobj->def->lx);
		if (bound == 2) continue;

		// マトリクスセットアップ
		EvmobjSetup(evmobj);

		// 描画
		ChainEvmObj(evmobj);
	}
	//DG_EndScene();

	// TODO:インデックス付き頂点ブレンディングOFF
	// XBOXではサポートされない？？
	//g_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	//g_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
}


//-----------------------------------------------------------------------------

// マルチウェイトエンベロープ対応オブジェクト処理
void DG_ChainEvmChanl(DG_CHANL *cp, int which)
{
	_DG_ChainEvmChanl(cp, which, 0);
}

// マルチウェイトエンベロープ対応オブジェクト処理
void DG_ChainEvmChanlLatter(DG_CHANL *cp, int which)
{
	_DG_ChainEvmChanl(cp, which, 1);
}

//-----------------------------------------------------------------------------
		
// マルチウェイトエンベロープ対応オブジェクト作成
DG_EVMOBJ *DG_MakeEvmObj(EVM_DEF *def, int flag, int chanl)
{
	int         buf_size;
	int         i, n_skeletons;
	DG_EVMOBJ   *evmobj;
	DG_EVMPACK  *evmpack;
	EVM_PACK    *pack;
	//DG_TEX      *tex;

	flag |= DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3;
	ASSERT(def != NULL);
	n_skeletons = def->n_x_models;

	// 物体ハンドラのバッファを確保する
	buf_size = sizeof(DG_EVMOBJ)
			+ sizeof(MATRIX) * n_skeletons * 2
			+ sizeof(DG_EVMPACK) * def->n_packs;
	evmobj = (DG_EVMOBJ *)GV_Malloc(buf_size);
	if (evmobj == NULL) {
		return NULL;
	}

	// 基本データ初期化
	GV_ZeroMemory(evmobj, sizeof(DG_EVMOBJ));
	evmobj->world = DG_UnitMatrix;
	evmobj->light = &DG_LightMatrix;
	evmobj->def = def;
	evmobj->flag = flag;
	evmobj->chanl = chanl;
	evmobj->group_id = 0x7fffffff;	
	evmobj->n_skeleton = n_skeletons;
	evmobj->n_packet = def->n_packs;
	evmobj->use_buffer = 0;
	evmobj->matrix[0] = (MATRIX *)&evmobj[1];
	evmobj->matrix[1] = &evmobj->matrix[0][n_skeletons];
	evmobj->packs = (DG_EVMPACK*)&evmobj->matrix[1][n_skeletons];

	evmobj->verts_ptr = def->packet[0].verts;
	evmobj->norms_ptr = def->packet[0].norms;
	evmobj->uvs0_ptr = def->packet[0].uvs[0];
	evmobj->uvs1_ptr = def->packet[0].uvs[1];
	evmobj->uvs2_ptr = def->packet[0].uvs[2];
	evmobj->weight_ptr = def->packet[0].weight;

	for (i = 0; i < n_skeletons; i ++) {
		evmobj->matrix[0][i] = DG_UnitMatrix;
	}
	for (i = 0; i < n_skeletons; i ++) {
		evmobj->matrix[1][i] = DG_UnitMatrix;
	}

	// パケット管理情報初期化
	pack = def->packet;
	evmpack = evmobj->packs;
	for (i = 0; i < evmobj->n_packet; i ++){
		if (evmobj->uvs1_ptr == NULL && pack->uvs[1] != NULL) evmobj->uvs1_ptr = pack->uvs[1];
		if (evmobj->uvs2_ptr == NULL && pack->uvs[2] != NULL) evmobj->uvs2_ptr = pack->uvs[2];
		evmpack->flag = pack->flag;
		evmpack->max_mats = pack->n_mats;
		evmpack->n_verts = pack->n_verts;
		evmpack->n_step = (pack->n_verts + 1) / 2;
		evmpack->n_uv_step[0] = (evmpack->flag & DG_PACKFLAG_UV0) ? evmpack->n_step : 0;
		evmpack->n_uv_step[1] = (evmpack->flag & (DG_PACKFLAG_UV1|DG_PACKFLAG_BMAP)) ? evmpack->n_step : 0;
		evmpack->n_uv_step[2] = (evmpack->flag & (DG_PACKFLAG_UV2|DG_PACKFLAG_BMAP)) ? evmpack->n_step : 0;
		evmpack->tex0_ptr = NULL;
		evmpack->tex1_ptr = NULL;
		evmpack->tex2_ptr = NULL;
		if (pack->tex_id[0] != 0){
			evmpack->tex0_ptr = &((DG_TEX*)pack->tex_id[0])->tex_trans;
		}
		if (pack->tex_id[1] != 0){
			evmpack->tex1_ptr = &((DG_TEX*)pack->tex_id[1])->tex_trans;
		}
		if (pack->tex_id[2] != 0){
			evmpack->tex2_ptr = &((DG_TEX*)pack->tex_id[2])->tex_trans;
		}
		evmpack->matrix_list = *(long64 *)&pack->mat_id[0];
		evmpack ++;
		pack ++;
	}
	return evmobj;
}

// オブジェクトの開放
void DG_FreeEvmObj(DG_EVMOBJ *evmobj)
{
	GV_Free(evmobj);
}


//-----------------------------------------------------------------------------

// ＵＶ値の補正処理
void DG_WriteEvmMdlPaketUV(int tri_code, EVM_PACK *pack)
{
	float   u_scale, v_scale, u_offset, v_offset;
	short   *uv;
	DG_TEX  *tex;
	int     i, j;

	for (i = 0; i < 3; i ++) {
		// TODO:バンプマップフラグ / XBOXでは違う実装になる
		if (i == 2 && (pack->flag & DG_PACKFLAG_BMAP)) {
			pack->tex_id[2] = pack->tex_id[1];
			continue;
		}
		if (pack->tex_id[i] == 0) continue;

		// ＵＶ値のスケール補正(既にテクスチャが読み込まれている必要あり)
		tex = DG_GetTexture2(tri_code, pack->tex_id[i]);
		uv = (short *)pack->uvs[i];

		if (tex != NULL) {
			u_scale = tex->u_scale;
			v_scale = tex->v_scale;
			u_offset = tex->u_offset * 4096.0f;
			v_offset = tex->v_offset * 4096.0f;
		} else {
			u_scale = 1.0f;
			v_scale = 1.0f;
			u_offset = 0.0f;
			v_offset = 0.0f;
		}

		if (uv != NULL) {
			for (j = pack->n_verts; j > 0; j --) {
				uv[0] = (short)((float)uv[0] * u_scale + u_offset);
				uv[1] = (short)((float)uv[1] * v_scale + v_offset);
				uv += 4;
			}
		}
		pack->tex_id[i] = (int)tex;
	}
}
