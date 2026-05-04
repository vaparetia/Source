/*
	comdl.c
	チャンネル処理ユニット／共有モデルオブジェクト処理ルーチン

	1999/11/17 K.Takabe
	$Id: comdl.cpp,v 1.1.1.3 2002/11/19 11:42:03 Yoshizawa1 Exp $

*/

/*
 * Xboxではモデルを1個1個描画する以外の手段が無いため、
 * 基本的にKMS描画と変わりません。パフォーマンスが出るかどうか…。
 */


/*
	チャンネル処理ユニット／共有モデルオブジェクト処理ルーチン

	この共有モデルオブジェクトとは１ストリップで構成されている
	単一のモデル対し複数のマトリクスを用意することによって
	オブジェクトを複数表示することを可能とした特殊オブジェクトです。
	シェーディング処理は行ないませんが、オブジェクト毎にＲＧＢＡを
	指定できるようになっています。
	基本的にソートなどは行なわないため、不透明モデルと同じ扱いに
	なりますが、描画結果がソートに影響しない場合に限り半透明も
	使用可能です。


	void		DG_ComdlChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた共有モデルオブジェクトのパケット作成などを行なう


	DG_COMDL*	DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl )
	DG_MDLPACK	*mdl_pack ;		１ストリップ分のモデルデータ
	int			flag ;			処理フラグ
	int			n_comdl ;		オブジェクト数
	int			chanl ;			処理チャンネル

	共有モデルオブジェクトを作成する（作成後はDG_QueueComdlObjs()で登録すること）


	void	DG_FreeComdl( DG_COMDL *comdl )
	DG_COMDL	*comdl ;		開放オブジェクト

	メモリを破棄する（これを呼ぶ前にDG_DequeueComdlObjs()でシステムから削除しておくこと）

	------------------------------------------------


*/

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"
#include "shader.h"


/* ---------------------------------------------------------------- */

/*
 * comdl用頂点シェーダ
 */
   
static DWORD DG_ComdlVertexShaderHandle;
static char *DG_ComdlVertexShaderName[] = {
	"comdl",
};
void DG_InitComdlVertexShader(void)
{
	// TODO:バンド幅節約のためには、不要なメンバが少ない方が良い。
	DWORD dwObjDecl[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT3),   // position
		D3DVSD_REG(1,  D3DVSDT_FLOAT1),   // blend
		D3DVSD_REG(2,  D3DVSDT_FLOAT3),   // normal
		D3DVSD_REG(3,  D3DVSDT_D3DCOLOR), // diffuse
		D3DVSD_REG(7,  D3DVSDT_FLOAT2),   // uv0
		D3DVSD_REG(8,  D3DVSDT_FLOAT2),   // uv1
		D3DVSD_REG(9,  D3DVSDT_FLOAT2),   // uv2
		D3DVSD_END()
	};
	
	DG_CreateVertexShader(GV_StrCode(DG_ComdlVertexShaderName[0]),
						  dwObjDecl,
						  &DG_ComdlVertexShaderHandle);
}


/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(6897906)	/* "comdl" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(128)



/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	ComdlPlugin ;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );

/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/
void DG_AddPluginComdl( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &ComdlPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &ComdlPlugin );

	PluginStartFlag = 1 ;
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginComdl( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &ComdlPlugin );
		DG_FreePlugin( &ComdlPlugin );
	}
	PluginStartFlag = 0 ;
}

/* ---------------------------------------------------------------- */
	/*
		オブジェクト登録
	*/
int DG_QueueComdlObjs( DG_COMDL *comdl )
{
	if ( PluginStartFlag == 0 ){
		return 0;
	}
	return DG_QueueUserObject( ComdlPlugin.obj_buffer, comdl );
}

	/*
		オブジェクト削除
	*/
void DG_DequeueComdlObjs( DG_COMDL *comdl )
{
	if ( PluginStartFlag == 0 ){
		return ;
	}
	DG_DequeueUserObject( ComdlPlugin.obj_buffer, comdl );
}

/* ---------------------------------------------------------------- */

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

typedef struct _scrpad_work {
	FMATRIX	mat ;
	int			buffer_switch ;
	//u_long128	dma_buffer[2][256] ;
} ScrpadWork ;

int			*func_list ;

#if 0
/* ＶＵ１ワークパラメータ */
typedef struct _vu1_work {
	FVECTOR	clamp_min ;		/* クランプ処理用最小値（現在未使用） */
	FVECTOR	clamp_max ;		/* クランプ処理用最大値（現在未使用） */
	float	fog_param1, fog_param2, fog_param3, fog_param4 ;	/* フォグパラメータ */
	float	color_clip, poly_alpha, specular_mul, specular_clip ;	/* 各種定数 */
	FVECTOR	scale ;			/* プリミティブ座標算出用スケール値（xyz=pvec*scale+offset） */
	FVECTOR	offset ;		/* プリミティブ座標算出用オフセット */
	float	param1, param2, param3, param4 ;					/* 各種定数２ */
	FVECTOR	pad ;
	FMATRIX	pers ;			/* 透視変換マトリクス */
} Vu1Work ;


/* デフォルトのＶＵ１ワークパラメータ */
static Vu1Work	default_vu1_work = {
	.clamp_min = {.vx = 0.0f, .vy = 0.0f, .vz = 0.0f, .vw = -1.0f },
	.clamp_max = {4095.0f, 4095.0f, 65536.0f * 256.0f, 1.0f },
	.fog_param1 = 0, .fog_param2 = 0, .fog_param3 = 0, .fog_param4 = 0,
	.color_clip = 255.0f, .poly_alpha = 128.0f, .specular_mul = 0.0f, .specular_clip = 0.8f, 
	.scale = { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f },
	.offset = { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f },
};
#endif

/*----------------------------------------------------------------*/
	/*
		オブジェクトパケット用初期化パケットデータの書き出し
	*/
static int ComdlObjsPacketInit( void *tag_addr, DG_CHANL *cp )
{
#if 0
	u_long128		*src, *dst ;
	unsigned int	*tag ;
	int				i ;
	Vu1Work			*vu1_work ;

	tag = (unsigned int*)tag_addr ;

	/*
		初期化データを設定
	*/
	tag = DG_WritePacket_VIF1Init( tag );

	/*
		マイクロプログラムロード設定パケットの登録
	*/
	tag = DG_WritePacket_Vu1ComdlProg( tag, 0, &func_list );

	/*
		基本パラメータ設定パケットの登録
	*/
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(Vu1Work) );
	tag[2] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	tag[3] = SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(Vu1Work), VIF_DATA128, 0 ) ;
	tag +=4 ;

	/*
		パラメータの書き込み
	*/
	src = (u_long128*)&default_vu1_work ;
	dst = tag ;
	for ( i = SIZEOF_QWORD(Vu1Work) ; i > 0 ; i-- ) *dst++ = *src++ ;

	/* 必要なパラメータの変更 */
	vu1_work = (Vu1Work*)tag ;
	vu1_work->scale.vx = cp->width / 2 ;
	vu1_work->scale.vy = cp->height / 2 ;
	vu1_work->fog_param1 = DG_FogParam1 ;
	vu1_work->fog_param2 = DG_FogParam2 ;
	vu1_work->pers = cp->eye_pers ;
	tag = (u_int*)dst ;

	return ( ( (int)tag - (int)tag_addr ) / 16 );
#else
	return 0;
#endif
}
/*----------------------------------------------------------------*/
	/*
		初期化実行パケットの設定
	*/
static int WriteObjTransPacket( void *tag_addr, u_long64 *subtag )
{
#if 0
	unsigned int	*tag ;

	tag = (unsigned int*)tag_addr ;

	/* モデルデータ転送パケット接続 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 );
	tag[1] = DMATAG_SET_ADDR( subtag );
	tag[3] = SCE_VIF1_SET_NOP( 0 ) ;
	tag[2] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	tag += 4 ;

	/* プログラムの初期化実行 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag[2] = SCE_VIF1_SET_NOP( 0 ) ;
	tag[3] = SCE_VIF1_SET_MSCAL( func_list[0], 0 );
	tag += 4 ;

	return ( ( (int)tag - (int)tag_addr ) / sizeof(u_long128) );
#else
	return 0;
#endif
}

	/*
		各オブジェクトのＤＭＡパケットの作成
	*/
static int WriteObjPacks( void *tag_addr, DG_COMDL_POS *pos, int n_pos )
{
#if 0
	/* オブジェクトパケットのＤＭＡデータを接続する */
	unsigned int	*tag ;
	u_long128		*src, *dst ;
	int				n ;

	tag = (unsigned int*)tag_addr ;

	/* マトリクス転送タグ作成 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, n_pos * 5 + 1 );
	tag[2] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	tag[3] = SCE_VIF1_SET_UNPACK( 0x040, n_pos * 5 + 1, VIF_DATA128, 0 ) ;
	tag += 4 ;

	/* マトリクス転送数設定 */
	tag[0] = n_pos ;
	tag += 4 ;

	/* マトリクス＆ポリゴンカラーデータをストアする */
	src = (u_long128*)pos ;
	dst = (u_long128*)tag ;
	for ( n = n_pos * 5 ; n > 0 ; n-- ){
		if ( ( (int)src & 0x3f ) == 0 ) asm("pref 0,64(%0)"::"r"(src));
		*dst++ = *src++ ;
	}
	tag = (unsigned int*)dst ;

	/* マイクロプログラムの再実行 */
	tag[0] = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	tag[2] = SCE_VIF1_SET_NOP( 0 ) ;
	tag[3] = SCE_VIF1_SET_MSCNT( 0 ) ;
	tag += 4 ;

	return ( ( (int)tag - (int)tag_addr ) / sizeof(u_long128) );
#else
	return 0;
#endif
}
/*----------------------------------------------------------------*/
	/*
		プラグイン実行アクター
	*/
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	static const float color_one = 1.0f / 128.0f;
	
	DG_COMDL		*comdl, **que ;
	DG_COMDL_POS	*pos ;
	int		        i, n, invisible_flag ;
	ScrpadWork		*scrpad ;

	return ;
	scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	invisible_flag = DG_COMDL_INVISIBLE0 << cp->chanl_num ;
	/*
		キュー数のチェック
	*/
	MARK( __FILE__ );
	DG_SetDrawMark();
	if ( obj_buff->n_queue == 0 ) return ;

	scrpad->buffer_switch = 0 ;

	scrpad->buffer_switch = 1 - scrpad->buffer_switch ;

	/* 全オブジェクトの検索 */
	DG_BeginScene();
	que = (DG_COMDL **)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; i--, que++ ){
		comdl = *que ;
		if ( comdl->flag & invisible_flag ) continue ;
		//if ( comdl->tri_id != tex_list->code ) continue ;
		//if ( !( comdl->group_id & cp->group_id ) ) continue ;

		scrpad->buffer_switch = 1 - scrpad->buffer_switch ;

		n = comdl->n_objs;
		pos = comdl->pos;

		/* Xbox版描画 */
		DG_VBUF_INFO *info;
		info = comdl->index->vbuf_info;
		DG_SetStreamSource(0, info->vbuf, info->stride);
		DG_SetIndices(info->ibuf, 0);
		
		DG_SetVertexShader(DG_ComdlVertexShaderHandle);
		DG_SetPixelShader(0);
		DG_SetTexture(0, comdl->ptex);
		DG_SetAlphaMode(comdl->alpha);
		while (n > 0) {
			/* 行列設定 */
			MATRIX matTmp;
			D3DXMatrixMultiply(&matTmp, &pos->world, &cp->eye_xpers);
			D3DXMatrixTranspose(&matTmp, &matTmp);
			DG_SetVertexShaderConstant(CV_WORLDEYEPERS, &matTmp, 4);

			/* 色設定 */
			VECTOR vecTmp;
			vecTmp.vx = (float)pos->color.vx * color_one;
			vecTmp.vy = (float)pos->color.vy * color_one;
			vecTmp.vz = (float)pos->color.vz * color_one;
			vecTmp.vw = (float)pos->color.vw * color_one;
			DG_SetVertexShaderConstant(CV_COMDL_COL, &vecTmp, 1);

			DG_INDEX_INFO *index;
			index = comdl->index;
			DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, index->min, index->num,
									index->start, index->prims);
			pos++;
			n--;
		}
		DG_EndScene();
	}
	scrpad->buffer_switch = 1 - scrpad->buffer_switch ;
}

/*----------------------------------------------------------------*/
	/*
		初期化ＤＭＡパケットの作成
	*/
static void SetPaket( DG_COMDL *comdl, DG_MDLPACK *mdl_pack, DG_COMDL_PACKET *packet )
{
#if 0
	DG_TEX		*tex ;
	u_long64		prim, alpha ;
	int			n ;

	/* ＤＭＡ転送タグ＆パラメータ設定 */
	tex = (DG_TEX*)mdl_pack->tex_id[0] ;

	prim = SCE_GS_SET_PRIM( 0, 1, 1, 1, 0, 0, 0, 0, 0) ;
	alpha = tex->tex_trans.alpha.data ;
	if ( comdl->flag & DG_COMDL_SEMITRANS ){
		prim |= SCE_GS_PRIM_ABE ;	/* 半透明化 */
	}
	if ( comdl->flag & DG_COMDL_NOFOG ){
		prim &= ~SCE_GS_PRIM_FGE ;	/* 半透明化 */
	}

	/* 頂点の転送（端数の場合には切りのいいところまでデータがあることにする） */
	n = ( mdl_pack->n_verts + 1 ) / 2 ;
	packet->verts_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n );
	packet->verts_tag.addr = mdl_pack->verts ;
	packet->verts_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
	packet->verts_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x31a+2, n*2, VIF_VERT_PACK, 0 ) ;
	/* 法線の転送 */
	packet->norms_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n );
	packet->norms_tag.addr = mdl_pack->norms ;
	packet->norms_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
	packet->norms_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x31a+1, n*2, VIF_NORM_PACK, 0 ) ;
	/* テクスチャ座標の転送 */
	n = ( mdl_pack->n_verts + 3 ) / 4 ;
	packet->uvs_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n );
	packet->uvs_tag.addr = DMATAG_SET_ADDR( mdl_pack->uvs[0] );
	packet->uvs_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 3, 0 ) ;
	packet->uvs_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x31a+0, n*4, VIF_UV_PACK, 0 ) ;

	/* テクスチャ及び頂点転送用ＧＩＦタグの埋め込み */
	packet->datas_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(packet->dg_comdl_packet_data) );
	packet->datas_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	packet->datas_tag.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x310, SIZEOF_QWORD(packet->dg_comdl_packet_data), VIF_DATA128, 0 ) ;

	/* データパラメータのセット */
	packet->dg_comdl_packet_data.n_verts = mdl_pack->n_verts ;	/* 頂点データ数 */
	packet->dg_comdl_packet_data.data_offset = 5+3+1+1 ;			/* データ開始オフセット */

	/* クリップポリゴン描画用設定 */
	packet->dg_comdl_packet_data.giftag0.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 1) ;
	packet->dg_comdl_packet_data.giftag0.regs = GS_REGS_1(GS_REGS_AD) ;
	/* プリミティブパケットの設定 */
	packet->dg_comdl_packet_data.prim.data = prim | SCE_GS_PRIM_TRIFAN ;
	packet->dg_comdl_packet_data.prim.reg = SCE_GS_PRIM ;
	/* プリミティブパケット用ＧＩＦタグの設定 */
	packet->dg_comdl_packet_data.giftag1.tag = SCE_GIF_SET_TAG( 1, 1, 0, 0, SCE_GIF_PACKED, 3) ;
	packet->dg_comdl_packet_data.giftag1.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;

	/* テクスチャパラメータパケットの設定 */
	packet->dg_comdl_packet_data.giftag2.tag = SCE_GIF_SET_TAG(4, 1, 0, 0, 0, 1) ;
	packet->dg_comdl_packet_data.giftag2.regs = GS_REGS_1(GS_REGS_AD) ;
	/* クランプパラメータ設定 */
	packet->dg_comdl_packet_data.clamp.data = tex->tex_trans.clamp.data ;
	packet->dg_comdl_packet_data.clamp.reg = SCE_GS_CLAMP_1 ;
	/* テクスチャパラメータ設定 */
	packet->dg_comdl_packet_data.tex2.data = tex->tex_trans.tex2.data ;
	packet->dg_comdl_packet_data.tex2.reg = SCE_GS_TEX2_1 ;
	/* テクスチャパラメータ設定 */
	//packet->dg_comdl_packet_data.tex0.data = tex->tex_trans.tex0.data | (u_long64)2<<35;
	packet->dg_comdl_packet_data.tex0.data = tex->tex_trans.tex0.data ;
	//packet->dg_comdl_packet_data.tex0.data = tex->tex_trans.tex0.data | (u_long64)1<<35;
	packet->dg_comdl_packet_data.tex0.reg = SCE_GS_TEX0_1 ;
	/* アルファブレンティング設定 */
	packet->dg_comdl_packet_data.alpha.data = alpha ;
	packet->dg_comdl_packet_data.alpha.reg = SCE_GS_ALPHA_1 ;
	/* プリミティブパケット用ＧＩＦタグの設定 */
	packet->dg_comdl_packet_data.giftag3.tag = 
	  SCE_GIF_SET_TAG( mdl_pack->n_verts, 1, 1,
					  prim | SCE_GS_PRIM_TRISTRIP,
					  SCE_GIF_PACKED, 3) ;
	packet->dg_comdl_packet_data.giftag3.regs = GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
#endif
}

/*----------------------------------------------------------------*/
	/*
		共有モデルオブジェクトの作成
	*/
DG_COMDL*	DG_MakeComdl( DG_MDLPACK *mdl_pack, int flag, int n_comdl, int chanl )
{
	DG_COMDL	*comdl ;
	int			size ;

	flag |= DG_COMDL_INVISIBLE2|DG_COMDL_INVISIBLE3 ;
	ASSERT( mdl_pack != NULL )
	size = sizeof(DG_COMDL) + sizeof(DG_COMDL_POS) * n_comdl ;
	comdl = (DG_COMDL *)GV_Malloc( size );
	if ( comdl == NULL ){
		return NULL ;
	}
	GV_ZeroMemory( comdl, size );

	comdl->flag = flag ;
	comdl->n_objs = n_comdl ;
	comdl->chanl = chanl ;
	comdl->tri_id = DG_SearchTriFromTex( (DG_TEX*)( mdl_pack->tex_id[0] ) );
	comdl->ptex = ((DG_TEX *)mdl_pack->tex_id[0])->tex_trans.ptex;
	comdl->alpha = ((DG_TEX *)mdl_pack->tex_id[0])->tex_trans.alpha.data;
	comdl->mdl_pack = mdl_pack;
	comdl->index = (DG_INDEX_INFO *)mdl_pack->index;
	
	/* パケットの初期化 */
	//SetPaket( comdl, mdl_pack, &comdl->packet[0] );
	//SetPaket( comdl, mdl_pack, &comdl->packet[1] );

	return ( comdl ) ;
}

	/*
		メモリ開放
	*/
void	DG_FreeComdl( DG_COMDL *comdl )
{
	GV_DelayedFree( comdl );
}
