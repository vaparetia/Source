/*
  loader.cpp
  ロード初期化ルーチン
  
  1999/07/07 K.Takabe
  $Id: loader.cpp,v 1.1.1.3 2002/11/19 11:42:11 Yoshizawa1 Exp $
  
*/

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include "libgv.h"
#include "libdg.h"
#include "private.h"

//-----------------------------------------------------------------------------

#define SET_ADDR(a, b) ((a) == NULL ? NULL : (void *)((int)(a) + (int)(b)))


//-----------------------------------------------------------------------------
// KMS/EVM共用頂点バッファ管理関係
//-----------------------------------------------------------------------------

#define DG_VBUF_MAX (0x7fff)   // 最大数(適当)
static int n_vertex_buffer = 0; // 割り当て済みの頂点バッファの数
static int n_index_buffer = 0;  // 割り当て済みのインデックスバッファの数
static DG_VBUF_INFO DG_VbufInfo[DG_VBUF_MAX];

// KMS/EVM用の頂点バッファ情報を初期化する
void DG_InitKmsVertexBuffer(void)
{
	printf("DG_InitKmsVertexBuffer\n");
	GV_ZeroMemory(DG_VbufInfo, sizeof(DG_VbufInfo));
}

// KMS/EVM用の頂点バッファを開放する
void DG_ReleaseVertexBuffer(DG_VBUF_INFO *info)
{
	if (info->vbuf) {
		info->vbuf->Release();
		n_vertex_buffer--;
	}
	if (info->ibuf) {
		info->ibuf->Release();
		n_index_buffer--;
	}
	if (info->index != NULL) {
		GV_Free(info->index);
	}
	GV_ZeroMemory(info, sizeof(DG_VBUF_INFO));
}

// KMS/EVM用の頂点バッファを全て開放する
// ステージ切り替え時にこれを呼べばいいのかな？
void DG_ReleaseKmsVertexBuffer(void)
{
	int i;
	DG_VBUF_INFO *info;

	info = DG_VbufInfo;
	for (i = DG_VBUF_MAX; i > 0; i--, info++) {
		DG_ReleaseVertexBuffer(info);
	}
}

// KMS/EVM用の頂点バッファを割り当てる
#if 0
static DG_VBUF_INFO *KMS_AllocVertexBuffer(int n_verts)
{
	int          i;
	HRESULT      hr;
	DG_VBUF_INFO *info;	
	
	// 空き領域を探して割り当てる
	info = DG_VbufInfo;
	for (i = DG_VBUF_MAX; i > 0; i--, info++) {
		if (info->vbuf == NULL) {
			n_vertex_buffer++;
			break;
		}
	}
	if (i <= 0) {
		printf("KMS_VERTEX_BUFFER FULL\n");
		ASSERT(0);
	}

	// とりあえず全部同じ頂点フォーマットで考える。
	info->stride = sizeof(DG_BLENDVERTEX);
	info->rgba_ofs = 28; // 先頭からRGBデータへのオフセット(プリシェード用)
	info->size = info->stride * n_verts;
	info->fvf = D3DFVF_BLENDVERTEX;
	
	// vertex buffer 作成
	DG_CreateVertexBuffer(info->size, D3DUSAGE_WRITEONLY, info->fvf,
						  D3DPOOL_DEFAULT, &info->vbuf);
	return info;
}
#endif

// EVM用の頂点バッファ・インデックスバッファを割り当てる
static DG_VBUF_INFO *EVM_AllocVertexBuffer(int n_verts, int n_index, int n_packs)
{
	int          i;
	DG_VBUF_INFO *info;	
	
	// 空き領域を探して割り当てる
	info = DG_VbufInfo;
	for (i = DG_VBUF_MAX; i > 0; i--, info++) {
		if (info->vbuf == NULL) {
			break;
		}
	}
	if (i <= 0) {
		printf("DG_VERTEX_BUFFER FULL\n");
		ASSERT(0);
	}
	
	// とりあえず全部同じ頂点フォーマットで考える。
	info->stride = sizeof(DG_EVMVERTEX);
	info->rgba_ofs = 28; // 先頭からRGBデータへのオフセット(プリシェード用)
	info->size = info->stride * n_verts;
	info->fvf = D3DFVF_EVMVERTEX;
	
	// vertex buffer 作成
	DG_CreateVertexBuffer(info->size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
						  info->fvf, D3DPOOL_DEFAULT, &info->vbuf);
	// index buffer 作成
	DG_CreateIndexBuffer(sizeof(short) * n_index, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
						 D3DFMT_INDEX16, D3DPOOL_DEFAULT, &info->ibuf);

	// インデックスバッファ用の領域確保
	info->index = (DG_INDEX_INFO *)GV_Malloc(n_packs * sizeof(DG_INDEX_INFO));
	ASSERT(info->index != NULL);
	
	n_vertex_buffer++;
	n_index_buffer++;

	return info;
}

// KMS用の頂点バッファ・インデックスバッファを割り当てる
static DG_VBUF_INFO *KMS_AllocVertexBuffer(int n_verts, int n_index, int n_packs)
{
	int          i;
	DG_VBUF_INFO *info;	
	
	// 空き領域を探して割り当てる
	info = DG_VbufInfo;
	for (i = DG_VBUF_MAX; i > 0; i--, info++) {
		if (info->vbuf == NULL) {
			break;
		}
	}
	if (i <= 0) {
		printf("DG_VERTEX_BUFFER FULL\n");
		ASSERT(0);
	}
	
	// とりあえず全部同じ頂点フォーマットで考える。
	info->stride = sizeof(DG_BLENDVERTEX3);
	info->rgba_ofs = 28; // 先頭からRGBデータへのオフセット(プリシェード用)
	info->size = info->stride * n_verts;
	info->fvf = D3DFVF_BLENDVERTEX3;
	
	// vertex buffer 作成
	DG_CreateVertexBuffer(info->size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
						  info->fvf, D3DPOOL_DEFAULT, &info->vbuf);
	// index buffer 作成
	DG_CreateIndexBuffer(sizeof(short) * n_index, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
						 D3DFMT_INDEX16, D3DPOOL_DEFAULT, &info->ibuf);

	// インデックスバッファ用の領域確保
	info->index = (DG_INDEX_INFO *)GV_Malloc(n_packs * sizeof(DG_INDEX_INFO));
	ASSERT(info->index != NULL);
	
	n_vertex_buffer++;
	n_index_buffer++;

	return info;
}

//-----------------------------------------------------------------------------
// データ初期化処理実体
//-----------------------------------------------------------------------------

// KMSデータから頂点数/インデックス数を計算する。
// ※ 頂点数とインデックス数が違うのは、
//    PS2の描画キック/頂点キックフラグをエミュレートする際に、
//    インデックスデータを増やしてしまうため。
static void KMS_GetVertexNum(DG_DEF *def, int *n_vertex, int *n_index, int *n_packs)
{
	DG_MDL     *mdl;
	DG_MDLPACK *pack;
	int i, j, k;
	int kick_num; // 頂点キック数
	SVECTOR *norms;
	
	*n_vertex = 0;
	*n_index = 0;
	*n_packs = 0;
	
	mdl = def->models;
	for (i = def->n_x_models; i > 0; i--, mdl++) {
		pack = mdl->packs;
		for (j = mdl->n_packs; j > 0; j--, pack++) {
			kick_num = 0;
			norms = (SVECTOR *)pack->norms;
			for (k = pack->n_verts; k > 0; k--, norms++) {
				if (norms->pad & 0x8000) { // 頂点キックのみ
					kick_num++;
				}
			}
			*n_vertex += pack->n_verts;               // 頂点数
			*n_index += pack->n_verts + kick_num - 2; // 増加分のインデックス数
		}
		*n_packs += mdl->n_packs;
	}
//	printf("n_vertex = %d\n",*n_vertex);
//	printf("n_index  = %d\n",*n_index);
//	printf("n_pack  = %d\n", *n_packs);
}

// 頂点バッファ/インデックスバッファを確保し、
// それぞれのバッファにデータを設定する
static void KMS_SetVertexBuffer(DG_DEF *def, int n_vertex, int n_index, int n_packs)
{
	int i, j, k;
	DG_MDL          *mdl;
	DG_MDLPACK      *pack;
	DG_BLENDVERTEX3 *pVertices; // 頂点バッファへの書き込みポインタ
	float           r;          // 固定小数点→float変換
	SVECTOR         *verts, *norms;
	TVECTOR_S       *uvs[3];
	short           *pIndices;  // インデックスバッファへの書き込みポインタ
	HRESULT         hr;
	int             min, num, start, prims;
	DG_VBUF_INFO    *info;
	DG_INDEX_INFO   *index;


	info = KMS_AllocVertexBuffer(n_vertex, n_index, n_packs);
	ASSERT(info != NULL);

	r = 1.0f / 4096.0f;
	// PS2では空きメンバだったところに、
	// 頂点バッファ情報へのポインタを入れておく
	def->vbuf_info = info;

	hr = info->vbuf->Lock(0, info->size, (unsigned char **)&pVertices, 0);
	if (FAILED(hr)) ASSERT(0);

	mdl = def->models;
	for (i = def->n_x_models; i > 0; i--, mdl++) {
		pack = mdl->packs;
		for (j = mdl->n_packs; j > 0; j--, pack++) {
			verts = (SVECTOR *)pack->verts;
			norms = (SVECTOR *)pack->norms;
			uvs[0] = (TVECTOR_S *)pack->uvs[0];
			uvs[1] = (TVECTOR_S *)pack->uvs[1];
			uvs[2] = (TVECTOR_S *)pack->uvs[2];
			//pack->vbuf_info = info; // 各パケットにも情報を入れておく
			for (k = pack->n_verts; k > 0; k--, pVertices++, verts++, norms++) {
				pVertices->v.x = (float)verts->vx;
				pVertices->v.y = (float)verts->vy;
				pVertices->v.z = (float)verts->vz;
				pVertices->blend = (float)verts->pad * r;
				pVertices->n.x = (float)norms->vx;// * r;
				pVertices->n.y = (float)norms->vy;// * r;
				pVertices->n.z = (float)norms->vz;// * r;
				pVertices->rgba = 0x80808080;
				if (uvs[0] != NULL) {
					pVertices->tu0 = (float)uvs[0]->u * r;
					pVertices->tv0 = (float)uvs[0]->v * r;
					uvs[0] ++;
				} else {
					pVertices->tu0 = 0.0f;
					pVertices->tv0 = 0.0f;
				}
				if (uvs[1] != NULL) {
					pVertices->tu1 = (float)uvs[1]->u * r;
					pVertices->tv1 = (float)uvs[1]->v * r;
					uvs[1] ++;
				} else {
					pVertices->tu1 = 0.0f;
					pVertices->tv1 = 0.0f;
				}
				if (uvs[2] != NULL) {
					pVertices->tu2 = (float)uvs[2]->u * r;
					pVertices->tv2 = (float)uvs[2]->v * r;
					uvs[2] ++;
				} else {
					pVertices->tu2 = 0.0f;
					pVertices->tv2 = 0.0f;
				}
			}
		}
	}
	info->vbuf->Unlock();

	// インデックスバッファへの書き込み
	info->ibuf->Lock(0, n_index * sizeof(short), (unsigned char **)&pIndices, 0);
	
	mdl = def->models;
	index = (DG_INDEX_INFO *)info->index;
	start = 0; // 描画を開始するインデックスの位置
	num = 0;   // 頂点番号

	for (i = def->n_x_models; i > 0; i--, mdl++) {
		pack = mdl->packs;
		for (j = mdl->n_packs; j > 0; j--, pack++, index++) {
			index->vbuf_info = info; // インデックスから頂点バッファ情報を参照出来るように
			pack->index = index;     // インデックス情報を入れておく
			prims = 0;               // パケットに含まれるプリミティブの数
			min = num;               // パケットの中で最小のインデックス番号
			norms = (SVECTOR *)pack->norms;
			index->start = start;  // 最初のインデックス番号を記録
			*pIndices++ = num++;   // 最初のインデックスをバッファに格納
			for (k = pack->n_verts; ;) {
				norms++;
				if (-- k <= 0) break;
				if (min > num) {   // 最小インデックスを更新
					min = num;
				}
				if (!(norms->pad & 0x8000)) {
					// 描画キックならそのまま繋げる。
					*pIndices++ = num++;
					prims++;
				} else if ((norms - 1)->pad & 0x8000) {
					// 一個前が頂点キックならそのまま繋げる。ポリゴン数は増えない。
					*pIndices++ = num++;
				} else {
					// それ以外は頂点を増やしてストリップを切る
					*pIndices++ = num - 1;
					*pIndices++ = num;
					*pIndices++ = num++;
					prims += 4; // このポリゴンは見えない
				}
			}
			start += prims + 2;    // 次の開始位置
			index->num = num - min;
			index->prims = prims;
			index->min = min;
			//printf("init\n");
			//printf("index->min = %d\n", index->min);
			//printf("index->num = %d\n", index->num);
			//printf("index->start = %d\n", index->start);
			//printf("index->prims = %d\n", index->prims);
		}
	}
	info->ibuf->Unlock();	
}

// KMS初期化処理実体
static void KmsDataSetup(DG_DEF *def)
{
	DG_MDL      *mdl;
	DG_MDLPACK  *pack;
	int         i, j, tri_code;

	tri_code = - 1;
	mdl = def->models;
	for (i = def->n_x_models; i > 0; i--, mdl++) {
		mdl->packs = (DG_MDLPACK *)((int)mdl->packs + (int)def);
		pack = mdl->packs;
		for (j = mdl->n_packs; j > 0; j--, pack++) {
			pack->verts = (short *)((int)pack->verts + (int)def);
			pack->norms = (short *)((int)pack->norms + (int)def);
			if (pack->uvs[0] != 0) pack->uvs[0] = (short *)((int)pack->uvs[0] + (int)def);
			if (pack->uvs[1] != 0) pack->uvs[1] = (short *)((int)pack->uvs[1] + (int)def);
			if (pack->uvs[2] != 0) pack->uvs[2] = (short *)((int)pack->uvs[2] + (int)def);
			//printf("%08x %08x %08x %08x %08x \n",
			//pack->verts, pack->norms, pack->uvs[0], pack->uvs[1], pack->uvs[2]);
			//printf("%08x %08x %08x\n",
			//pack->tex_id[0], pack->tex_id[1], pack->tex_id[2]);
			if (pack->rgbs != 0) pack->rgbs = (CVECTOR *)((int)pack->rgbs + (int)def);
			// UV座標を補正
			DG_WriteMdlPaketUV(def->texture, pack);
			//KMS_SetVertexBuffer(pack);
		}
	}

}

static void KmsMakePrePacket(DG_DEF *def)
{
	// 頂点数/インデックス数を計算→頂点バッファ/インデックスバッファにデータセット
	{
		int n_vertex, n_index, n_packs;
		KMS_GetVertexNum(def, &n_vertex, &n_index, &n_packs);
		KMS_SetVertexBuffer(def, n_vertex, n_index, n_packs);
	}
}



/*----------------------------------------------------------------*/
static void CvdDataSetup( CVD_DEF *cvd_def )
{
	CVD_MDL		*cvd_mdl ;
	int			i ;

	cvd_mdl = cvd_def->models ;
	for ( i = cvd_def->n_models ; i > 0 ; i--, cvd_mdl++ ){
		cvd_mdl->verts = (FVECTOR*)( (int)cvd_mdl->verts + (int)cvd_def );
		cvd_mdl->norms = (FVECTOR*)( (int)cvd_mdl->norms + (int)cvd_def );
		cvd_mdl->uvs = (FVECTOR*)( (int)cvd_mdl->uvs + (int)cvd_def );
		cvd_mdl->verts_index = (short*)( (int)cvd_mdl->verts_index + (int)cvd_def );
		cvd_mdl->norms_index = (short*)( (int)cvd_mdl->norms_index + (int)cvd_def );
		cvd_mdl->uvs_index = (short*)( (int)cvd_mdl->uvs_index + (int)cvd_def );
	}
}

/*----------------------------------------------------------------*/
static void Cv2DataSetup( CV2_DEF *cvd_def )
{
	CV2_MDL		*cvd_mdl ;
	int			i ;

	cvd_mdl = cvd_def->models ;
	for ( i = cvd_def->n_models ; i > 0 ; i--, cvd_mdl++ ){
		if(cvd_def->flag & COMMON_VERTS){
			cvd_mdl->verts = (FVECTOR*)( (int)cvd_mdl->verts + (int)cvd_def );
			cvd_mdl->verts_index = (short*)( (int)cvd_mdl->verts_index + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_NORMS){
			cvd_mdl->norms = (FVECTOR*)( (int)cvd_mdl->norms + (int)cvd_def );
			cvd_mdl->norms_index = (short*)( (int)cvd_mdl->norms_index + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_UVS){
			cvd_mdl->uvs = (FVECTOR*)( (int)cvd_mdl->uvs + (int)cvd_def );
			cvd_mdl->uvs_index = (short*)( (int)cvd_mdl->uvs_index + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_VERT_USRDATA){
			cvd_mdl->vert_usrdata = (VERT_USRDATA*)( (int)cvd_mdl->vert_usrdata
							    + (int)cvd_def );
		}
		if(cvd_def->flag & COMMON_NORM_USRDATA){
			cvd_mdl->norm_usrdata = (NORM_USRDATA*)( (int)cvd_mdl->norm_usrdata
							    + (int)cvd_def );
		}
	}
}

// ライト初期化処理実体・？何もしてない？
static void Lt2DataSetup(LIT_DEF *def)
{
	LIT_GRP *grp;
	int     i, j;

	grp = (LIT_GRP *)&def[1];
	for (i = 0; i < def->n_lit_group; grp++, i++) {
		grp->lit = (void *)((int)grp->lit + (int)def);
		if (grp->type & LIT_TYPE_POINT) {
			/* 点光源固有初期化 */
			LIT_POINT *lit = (LIT_POINT *)grp->lit;
			for (j = grp->n_lights; j > 0; lit++, j--) {
			}
		} else if (grp->type & LIT_TYPE_SPOT) {
			/* スポット光源固有初期化 */
			LIT_SPOT *lit = (LIT_SPOT *)grp->lit;
			for (j = grp->n_lights; j > 0; lit++, j--) {
			}
		} else if (grp->type & LIT_TYPE_BLACKPOINT) {
			/* 黒点光源固有初期化 */
			LIT_BLACKPOINT *lit = (LIT_BLACKPOINT *)grp->lit;
			for (j = grp->n_lights; j > 0; lit++, j--) {
#if 0
				printf("bound max:%f %f %f\n",
					   lit->bound_max.vx, lit->bound_max.vx, lit->bound_max.vx);
				printf("bound min %f %f %f\n",
					   lit->bound_min.vx, lit->bound_min.vx, lit->bound_min.vx);
				printf("pos %f %f %f\n", lit->point.vx, lit->point.vy, lit->point.vz);
				printf("r_range %f flag %08x\n", lit->r_range, lit->flag);
#endif
			}
		}
	}
}

// EVMデータから頂点数/インデックス数を計算する。
// ※ 頂点数とインデックス数が違うのは、
//    PS2の描画キック/頂点キックフラグをエミュレートする際に、
//    インデックスデータを増やしてしまうため。
static void EVM_GetVertexNum(EVM_DEF *def, int *n_vertex, int *n_index)
{
	EVM_PACK *pack;
	int i, j;
	int kick_num; // EVM_PACKに含まれる頂点キック数
	SVECTOR *verts;
	
	*n_vertex = 0;
	*n_index = 0;
	
	pack = def->packet;
	for (i = def->n_packs; i > 0; i--, pack++) {
		kick_num = 0;
		verts = (SVECTOR *)pack->verts;
		for (j = pack->n_verts; j > 0; j--, verts++) {
			if (verts->pad & 0x8000) { // 頂点キックのみ
				kick_num++;
			}
		}
		*n_vertex += pack->n_verts;               // 頂点数
		*n_index += pack->n_verts + kick_num - 2; // 増加分のインデックス数
	}
}

// 頂点バッファ/インデックスバッファを確保し、
// それぞれのバッファにデータを設定する
static void EVM_SetVertexBuffer(EVM_DEF *def, int n_vertex, int n_index)
{
	int i, j;
	DG_VBUF_INFO  *info;
	EVM_PACK      *pack;
	SVECTOR       *verts, *norms, *uvs[3];
	u_char        *weight;
	DG_EVMVERTEX  *pVertices; // 頂点バッファへの書き込みポインタ
	short         *pIndices;  // インデックスバッファへの書き込みポインタ
	float         rv, rn;     // 頂点データ/法線データ補正値
	HRESULT       hr;
	int           min, num, start, prims;
	DG_INDEX_INFO *index;
	
	info = EVM_AllocVertexBuffer(n_vertex, n_index, def->n_packs);
	
	// モデルタイプによって頂点座標の小数点位置が違う
	if (def->type & DG_EVMTYPE_LARGE) {
		printf("DG_EVMTYPE_LARGE\n");
		rv = 1.0f;
	} else {
		printf("NOT DG_EVMTYPE_LARGE\n");
		rv = 1.0f / 16.0f;
	}
	rn = 1.0f / 4096.0f;
	
	ASSERT(info != NULL);
	// PS2では 'pad1' メンバになっている領域に、
	// 頂点バッファ情報へのポインタを入れておく
	//def->packet->vbuf_info = info;

	hr = info->vbuf->Lock(0, info->size, (unsigned char **)&pVertices, 0);
	if (FAILED(hr)) ASSERT(0);

	pack = def->packet;
	for (i = def->n_packs; i > 0; i--, pack++) {
		verts = (SVECTOR *)pack->verts;
		norms = (SVECTOR *)pack->norms;
		weight = (u_char *)pack->weight;
		uvs[0] = (SVECTOR *)pack->uvs[0];
		uvs[1] = (SVECTOR *)pack->uvs[1];
		uvs[2] = (SVECTOR *)pack->uvs[2];

		for (j = pack->n_verts; j > 0; j--, pVertices++) {
			pVertices->v.x = verts->vx * rv; // 値を補正する
			pVertices->v.y = verts->vy * rv;
			pVertices->v.z = verts->vz * rv;
			verts ++;
			
			// weight[0]～weight[3]がマトリクスインデックス
			// weight[4]～weight[7]がマトリクスインデックス * 4
			pVertices->blend[0] = weight[0] / 128.0f;
			pVertices->blend[1] = weight[1] / 128.0f;
			pVertices->blend[2] = weight[2] / 128.0f;
			pVertices->blend[3] = weight[3] / 128.0f;
			pVertices->index[0] = weight[4];
			pVertices->index[1] = weight[5];
			pVertices->index[2] = weight[6];
			pVertices->index[3] = weight[7];			
			weight += 8;
			
			//※法線ブレンディングの関係。値を補正しないので注意。
			//pVertices->n.x = norms->vx * rn;
			//pVertices->n.y = norms->vy * rn;
			//pVertices->n.z = norms->vz * rn;
			pVertices->n.x = norms->vx;
			pVertices->n.y = norms->vy;
			pVertices->n.z = norms->vz;
			norms ++;
			pVertices->rgba = 0x80808080;
			if (uvs[0] != NULL) {
				pVertices->tu0 = uvs[0]->vx * rn;
				pVertices->tv0 = uvs[0]->vy * rn;
				uvs[0]++;
			}
			if (uvs[1] != NULL) {
				pVertices->tu1 = uvs[1]->vx * rn;
				pVertices->tv1 = uvs[1]->vy * rn;
				uvs[1]++;
			}
			if (uvs[2] != NULL) {
				pVertices->tu2 = uvs[2]->vx * rn;
				pVertices->tv2 = uvs[2]->vy * rn;
				uvs[2]++;
			}
		}
	}
	info->vbuf->Unlock();

	// インデックスバッファへの書き込み
	info->ibuf->Lock(0, n_index * sizeof(short), (unsigned char **)&pIndices, 0);
	
	pack = def->packet;
	index = (DG_INDEX_INFO *)info->index;
	start = 0; // 描画を開始するインデックスの位置
	num = 0;   // 頂点番号
	for (i = def->n_packs; i > 0; i--, pack++, index++) {
		prims = 0;     // パケットに含まれるプリミティブの数
		min = num;     // パケットの中で最小のインデックス番号

		index->vbuf_info = info; // インデックスから頂点バッファ情報を参照出来るように
		pack->index = index;
		verts = (SVECTOR *)pack->verts;
		index->start = start;    // 最初のインデックス番号を記録
		*pIndices++ = num++;   // 最初のインデックスをバッファに格納
		for (j = pack->n_verts; ;) {
			verts++;
			if (-- j <= 0) break;
			if (min > num) {   // 最小インデックスを更新
				min = num;
			}
			if (!(verts->pad & 0x8000)) {
				// 描画キックならそのまま繋げる。
				*pIndices++ = num++;
				prims++;
			} else if ((verts - 1)->pad & 0x8000) {
				// 一個前が頂点キックならそのまま繋げる。ポリゴン数は増えない。
				*pIndices++ = num++;
			} else {
				// それ以外は頂点を増やしてストリップを切る
				*pIndices++ = num - 1;
				*pIndices++ = num;
				*pIndices++ = num++;
				prims += 4; // このポリゴンは見えない
			}
		}
		start += prims + 2;    // 次の開始位置
		index->num = num - min;
		index->prims = prims;
		index->min = min;
	}
	info->ibuf->Unlock();
}


// EVM初期化処理実体
//#define PRINT_EVM_PARAMS // デバッグ用パラメータ表示
static void EvmDataSetup(EVM_DEF *def)
{
	int			i, j, k;
	EVM_PACK	*pack;
	int			count = 0, err_flag = 0;

	printf("EvmDataSetup\n");
	
	def->packet = (EVM_PACK *)SET_ADDR(def->packet, def);
	pack = def->packet;

#ifdef PRINT_EVM_PARAMS
	printf("n_models     = %04x\n", def->n_models);
	printf("n_x_models   = %d\n", def->n_x_models);
	printf("l(%f, %f, %f)\n", def->lx, def->ly, def->lz);
	printf("u(%f, %f, %f)\n", def->ux, def->uy, def->uz);
	printf("type         = %d\n", def->type);
	printf("texture      = %d\n", def->texture);
#endif
	for (i = def->n_packs; i > 0; i--, pack++) {
		pack->verts = SET_ADDR(pack->verts, def);
		pack->norms = SET_ADDR(pack->norms, def);
		pack->uvs[0] = SET_ADDR(pack->uvs[0], def);
		pack->uvs[1] = SET_ADDR(pack->uvs[1], def);
		pack->uvs[2] = SET_ADDR(pack->uvs[2], def);
		pack->weight = SET_ADDR(pack->weight, def);
		DG_WriteEvmMdlPaketUV(def->texture, pack);

#ifdef PRINT_EVM_PARAMS
		// テスト用/パラメータ表示
		printf("flag         = %08x\n", pack->flag);
		printf("tex_id       = %08x, %08x, %08x\n",
			   pack->tex_id[0], pack->tex_id[1], pack->tex_id[2]);
		printf("n_verts      = %d\n", pack->n_verts);
		printf("n_mats       = %d\n", pack->n_mats);
		printf("mat_id       = %d %d %d %d %d %d %d %d\n",
			   pack->mat_id[0], pack->mat_id[1], pack->mat_id[2], pack->mat_id[3],
			   pack->mat_id[4], pack->mat_id[5], pack->mat_id[6], pack->mat_id[7]);
		SVECTOR *verts = (SVECTOR *)pack->verts;
		SVECTOR *norms = (SVECTOR *)pack->norms;
		SVECTOR *uvs0 = (SVECTOR *)pack->uvs[0];
		SVECTOR *uvs1 = (SVECTOR *)pack->uvs[1];
		SVECTOR *uvs2 = (SVECTOR *)pack->uvs[2]; 
		u_char  *w = (u_char *)pack->weight;
		printf("----------------\n");
		//if (i == def->n_packs || i == def->n_packs - 1) 
		for (j = pack->n_verts; j > 0; j--) {
			printf("verts : %d, %d, %d, %04x\n",
				   verts->vx, verts->vy, verts->vz, verts->pad);
			printf("norms : %d, %d, %d, %04x\n",
				   norms->vx, norms->vy, norms->vz, norms->pad);
			if (uvs0 != NULL)
				printf("uvs0  : %d, %d, %d, %d\n",
					   uvs0->vx, uvs0->vy, uvs0->vz, uvs0->pad);
			if (uvs1 != NULL)
				printf("uvs1  : %d, %d, %d, %d\n",
					   uvs1->vx, uvs1->vy, uvs1->vz, uvs1->pad);
			if (uvs2 != NULL)
				printf("uvs2  : %d, %d, %d, %d\n",
					   uvs2->vx, uvs2->vy, uvs2->vz, uvs2->pad);
			printf("w     : ");
			for (k = 0; k < 8; k++, w++) {
				printf("%d ", *w);
			}
			printf("\n");
			verts++;
			norms++;
			if (uvs0 != NULL) uvs0++;
			if (uvs1 != NULL) uvs1++;
			if (uvs2 != NULL) uvs2++;
		}
#endif
		
		// マトリクスパレット対応
		if (!(def->type & DG_EVMTYPE_FORMAT_V2)) {
			char          *weight;
			char          new_weight[8], index[8];
			unsigned int  n_matrix, max_matrix;

			printf("DG_EVMTYPE_FORMAT_V2\n");
			
			weight = (char *)pack->weight;
			max_matrix = 0;
			
			for (j = pack->n_mats; j < 8; j++) {
				pack->mat_id[j] = 0xff;
			}
			for (j = 0; j < (int)pack->n_verts; j++, weight += 8) {
				// 重み情報のパレットインデックス化
				n_matrix = 0;
				for (k = 0; k < (int)pack->n_mats; k++) {
					if (weight[k] != 0) {
						new_weight[n_matrix] = weight[k];
						index[n_matrix] = k;
						n_matrix++;
					}
				}
				if (n_matrix > 4) {
					printf("loader.c: evm matrix error!! (%d %d %d %d %d %d %d %d)\n",
						   weight[0], weight[1], weight[2], weight[3],
						   weight[4], weight[5], weight[6], weight[7]);
					n_matrix = 4;
				}
				/* 書き込み */
				for (k = 0; k < (int)n_matrix; k++) {
					weight[k] = new_weight[k];
					weight[k + 4] = index[k] * 4;
				}
				for (; k < 4; k++) {
					weight[k] = 0;
					weight[k + 4] = 0;
				}
				/* 最大マトリクス数の調整 */
				if (n_matrix > max_matrix) {
					max_matrix = n_matrix;
				}
			}
			// 最大マトリクス数補正
			if (pack->n_mats != max_matrix) {
				printf("convert %d to %d\n", pack->n_mats, max_matrix);
				count++;
			}
			pack->n_mats = max_matrix;
		}
	}

	printf("converted packet %d / %d\n", count, def->n_packs);
	if (err_flag) {
		printf("err------------------------------------------------------\n");
	}
}

void EvmMakePrePacket( EVM_DEF *def )
{
	// XBOX追加/頂点バッファ・インデックスバッファにデータ設定
	{
		int n_vertex, n_index;
		EVM_GetVertexNum(def, &n_vertex, &n_index);
		EVM_SetVertexBuffer(def, n_vertex, n_index);
	}
}

//-----------------------------------------------------------------------------
// データ初期化処理インタフェース
//-----------------------------------------------------------------------------

// モデルの初期化
int DG_LoadInitKms(void *buf, int id)
{
	int		format_id;
#if 1
	// TODO : XBOXでは実装不要？
	if (id & GV_CACHEID_RESIDENT) {
		KmsMakePrePacket((DG_DEF*) buf);	/* プリパケットのみ生成しなおす必要があるため */
		return 1;
	}
#endif
	format_id = ((DG_DEF *)buf)->data_format;
	if (format_id != MGS_MODEL_NORM
		&& format_id != MGS_MODEL_MULTITEX
		&& format_id != MGS_MODEL_NORM_A
		&& format_id != MGS_MODEL_MULTITEX_A){
		printf("file format error!!(%d)\n", format_id);
		return 0;
	}
	KmsDataSetup((DG_DEF *)buf);
	//((DG_DEF*)buf)->pad[2] = id; /* デバッグ用 */
	KmsMakePrePacket((DG_DEF*) buf);

	return 1;
}

//-----------------------------------------------------------------------------

// 共有頂点データの初期化   2002.02.05 T.Morita added
int		DG_LoadInitCvd( void *buf, int id )
{
	if ( id & GV_CACHEID_RESIDENT ) return (1);
	if ( ( (CVD_DEF*)buf )->id == 6754556 ){
		CvdDataSetup( (CVD_DEF*) buf );
	} else {
		Cv2DataSetup( (CV2_DEF*) buf );
	}
	return (1);
}

//-----------------------------------------------------------------------------

// 共有頂点データの初期化   2002.02.05 T.Morita added
int		DG_LoadInitCv2( void *buf, int id )
{
	if ( id & GV_CACHEID_RESIDENT ) return (1);
	Cv2DataSetup( (CV2_DEF*) buf );
	return (1);
}

//-----------------------------------------------------------------------------

// ライトの初期化
int DG_LoadInitLt2(void *buf, int id)
{
	//if (id & GV_CACHEID_RESIDENT) return 1;
	Lt2DataSetup((LIT_DEF *)buf);
	return 1;
}

//-----------------------------------------------------------------------------

// マルチウェイトエンベロープモデルの初期化
int DG_LoadInitEvm(void *buf, int id)
{
	if (id & GV_CACHEID_RESIDENT){
		EvmMakePrePacket((EVM_DEF*) buf);	/* プリパケットのみ生成しなおす必要があるため */
		return 1;
	}
	EvmDataSetup((EVM_DEF *)buf);
	EvmMakePrePacket((EVM_DEF*) buf);
	return 1;
}

//-----------------------------------------------------------------------------
