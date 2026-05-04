/*
	covershw.c
	チャンネル処理ユニット／カバーシャドウオブジェクトＤＭＡ接続ルーチン

	2002/07/25 K.Takabe
	$Id: covershw.c,v 1.2 2002/11/23 11:36:52 Yoshizawa1 Exp $

*/
/*

	void		DG_CoverShadowChainChanl( DG_CHANL *cp, int which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する


*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libpc.h>

#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#define _CopyVector( a, b ) { *(u_long128*)a = *(u_long128*)b ; }
#define _CopyMatrix( a, b ) { 0[(u_long128*)a] = 0[(u_long128*)b] ;\
								1[(u_long128*)a] = 1[(u_long128*)b] ;\
								2[(u_long128*)a] = 2[(u_long128*)b] ;\
								3[(u_long128*)a] = 3[(u_long128*)b] ; }

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#define GS_REGS_1(r0) \
((r0) << 0x00)
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)

/*----------------------------------------------------------------*/

#define NEXT_BUFF()	({ scrpad->buffer_switch = 1 - scrpad->buffer_switch ; (&scrpad->dma_packet[ scrpad->buffer_switch ] ) ;})

/*----------------------------------------------------------------*/
	/*
	*/


/*----------------------------------------------------------------*/
	/*
		ＶＵ１ワーク初期化パラメータ関連
	*/
/* ＶＵ１ワークパラメータ */
typedef ALIGN16_DECL(struct) _vu1_work {
	FVECTOR	clamp_min ;		/* クランプ処理用最小値（現在未使用） */
	FVECTOR	clamp_max ;		/* クランプ処理用最大値（現在未使用） */
	float	fog_param1, fog_param2, fog_param3, fog_param4 ;	/* フォグパラメータ */
	float	color_clip, poly_alpha, specular_mul, specular_clip ;	/* 各種定数 */
	FVECTOR	scale ;			/* プリミティブ座標算出用スケール値（xyz=pvec*scale+offset） */
	FVECTOR	offset ;		/* プリミティブ座標算出用オフセット */
	float	param1, param2, param3, param4 ;					/* 各種定数２ */
	FVECTOR	aspect ;
	FMATRIX	pers ;			/* 透視変換マトリクス */
} Vu1Work ;
/* ＶＵ１メモリ初期化パケット */
typedef ALIGN16_DECL(struct) _vu1_init_packet {
	DG_DMATAG	init_vif ;
	u_int		init_vifcode[12] ;
	DG_DMATAG	init_vu1_prog ;
	DG_DMATAG	init_param_trans ;
	Vu1Work		init_param ;
} VU1_INIT_PACKET ;


/* マイクロプログラム読み込みタグ */
extern qword Vu1DrawObject0 ;
extern qword Vu1DrawObject1 ;
#define SELECT_VU1_PROGRAM Vu1DrawObject1
/* マイクロプログラム実行アドレスリスト */
extern int Vu1DrawObject0_Func[] ;
extern int Vu1DrawObject1_Func[] ;
extern int Vu1DrawShadowObject_Func[] ;
extern int Vu1DrawShadowObject2_Func[] ;
#define SELECT_VU1_PROGRAM_FUNC Vu1DrawShadowObject2_Func

/*----------------------------------------------------------------*/
	/*
		各種ローカル構造体定義
	*/

/* １パケット分のＤＭＡ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _pack_dma_packet {
	DG_DMATAG		verts_trans ;		/* 頂点転送ＤＭＡタグ */
	DG_DMATAG		norms_trans ;		/* 法線転送ＤＭＡタグ */
	DG_DMATAG		uvs0_trans ;		/* ＵＶ０転送ＤＭＡタグ */
#if 0
	DG_DMATAG		uvs1_trans ;		/* ＵＶ１転送ＤＭＡタグ */
	DG_DMATAG		uvs2_trans ;		/* ＵＶ２転送ＤＭＡタグ */
	DG_DMATAG		rgb_trans ;			/* 頂点カラー転送ＤＭＡタグ */
#endif
	DG_DMATAG		trans_tag ;			/* パラメータ転送ＤＭＡタグ */
	struct _pack_dma_datas{
		//FMATRIX		screen ;
		//FMATRIX		local_light ;
		//FMATRIX		local_color ;
		//FMATRIX		connection ;
		//FMATRIX		option ;
		int			n_verts ;
		int			flag ;
		int			fog ;
		int			bound ;
		//DG_GIFTAG	giftag_normal ;	/* 通常描画用描画ＧＩＦタグ */
		//DG_GIFTAG	giftag_clip ;	/* クリップポリゴン用描画ＧＩＦタグ */
		//IVECTOR		gif_regslist ;	/* マルチテクスチャ用レジスタリスト */
	} datas ;
	DG_VIFCODE		prog_exec ;
} PACK_DMA_PACKET ;

/* オブジェクトマトリクススタック構造体 */
typedef struct _matrix_stack {
	DG_DMATAG	next_tag ;
	struct _stack_datas {
		FMATRIX		screen ;
		FMATRIX		local_light ;
		FMATRIX		local_color ;
		FMATRIX		connection ;
		FMATRIX		option ;
		int			n_verts ;
		int			flag ;
		int			fog ;
		int			pad ;
		DG_GIFTAG	giftag_normal ;	/* 通常描画用描画ＧＩＦタグ */
		DG_GIFTAG	giftag_clip ;	/* クリップポリゴン用描画ＧＩＦタグ */
		IVECTOR		gif_regslist ;	/* マルチテクスチャ用レジスタリスト */
	} datas ;
} MATRIX_STACK ;

/* テクスチャ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _tex_dma_packet {
	DG_DMATAG		tex0_trans ;		/* テクスチャ０転送ＤＭＡパケット */
#if 0
	DG_DMATAG		tex1_trans ;		/* テクスチャ１転送ＤＭＡパケット */
	DG_DMATAG		tex2_trans ;		/* テクスチャ２転送ＤＭＡパケット */
#endif
} TEX_DMA_PACKET ;

/* ＤＭＡ転送データパケット */
typedef ALIGN16_DECL(struct) _packet_buffer {
	/* マトリクス転送 */
	DG_DMATAG			mat_trans_tag ;
	/* テクスチャ情報転送 */
	TEX_DMA_PACKET		tex_packet ;
	/* 頂点情報転送 */
	PACK_DMA_PACKET		pack_packet ;
} PACKET_BUFFER ;

/*----------------------------------------------------------------*/
typedef struct {
	DG_OBJS		*objs ;
	int			flag ;
	int			shadow_id ;
	int			bound_mode ;
	int			group_id ;
} OBJ_LIST ;

typedef struct {
	/* オブジェクト表示用 */
	int			buffer_switch ;
	int			pad0[3] ;
	/* ＤＭＡ生成用バッファ */
	MATRIX_STACK	matrix_stack ;		/* マトリクススタック */
	PACKET_BUFFER	dma_packet[2] ;		/* ＤＭＡパケット（ダブルバッファ） */
	u_long128		dma_buffer[128] ;	/* 汎用ＤＭＡパケット生成バッファ */

	/* 各種処理用 */
	FMATRIX		eye_pers ;
	FMATRIX		eye_pers2 ;
	FMATRIX		light_view ;
	FMATRIX		light_view2 ;
	FMATRIX		light_trans ;
	FMATRIX		light_pers ;
	FMATRIX		light_pers2 ;
	FVECTOR		light_vector ;
	FVECTOR		color_vector ;
	FMATRIX		raise_eye_pers ;		/* 優先上げマトリクス（クリップオブジェクト用） */
	FMATRIX		raise_eye_pers2 ;		/* 優先上げマトリクス（非クリップオブジェクト用） */
	FMATRIX		root ;
	FMATRIX		tmp_mat ;
	FVECTOR		tmp_vec ;
	FVECTOR		verts[16] ;
	int			bound_mode ;
	float		fog_param1 ;
	float		fog_param2 ;
	float		far_range ;				/* 減衰の最大距離 */
	int			invisible_flag ;
	int			pad[2] ;

	/* ChainObj()関数固有ローカル変数 */
	void		*matrix_addr ;

	/* オブジェクト検索用リスト */
	int			max_list ;
	OBJ_LIST	list[128] ;

	/* 計算結果一時保存 */
	FMATRIX		screen ;				/* 表示オブジェクトの透視変換マトリクス */
	FMATRIX		local_light ;			/* 表示オブジェクトのローカルライトマトリクス */
	FMATRIX		local_color ;			/* 表示オブジェクトのローカルカラーマトリクス */
	FMATRIX		env_mat ;				/* 表示オブジェクトのエンベロープ計算マトリクス */
	int			fog ;					/* 表示オブジェクトの固定フォグ値 */
	int			local_bound_mode ;		/* 表示オブジェクトのバウンディングチェック結果 */
	int			para_mode ;				/* テクスチャの平行投影モード */
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	projection[2] ;
	PERFORMANCE_PACKET_INFO	shadow_draw[2] ;
	int			n_packs ;
	int			n_verts ;
#endif

	/* ローカルワーク */
	u_long128	local_work[0] ;

} ScrpadWork ;

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

extern void DG_StartBoundingCheckSupport( void );/* screen.c */
extern int DG_WriteObjsPacketInit_WriteShadow( void *tag_addr, DG_CHANL *cp, float fog_param1, float fog_param2 );

static DG_TEX_TRANS	Shadow_TransTexture ALIGN64 = {
	{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7), 0x0fffeeee },
	//{ SCE_GS_SET_CLAMP(2,2,1,254,1,254), SCE_GS_CLAMP_1 },/* なぜかハイレゾ時にw01bにてゴミがでる */
	{ SCE_GS_SET_CLAMP(2,2,1,254,1,253), SCE_GS_CLAMP_1 },
//	{SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 2 ),SCE_GS_TEST_1},
	{ 0, SCE_GS_NOP },
	{ SCE_GS_SET_TEX0( BUFFER_PAGE(2) / 64,
					  BUFFER_WIDTH/64,SCE_GS_PSMCT32,8,8,1,0,0,0,0,0,0), SCE_GS_TEX0_1 },
	{ SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ), SCE_GS_ALPHA_1 },
	{ 0, SCE_GS_NOP },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
};
static DG_TEX_TRANS	Spot_TransTexture ALIGN64 = {
	{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7), 0x0fffeeee },
	{ SCE_GS_SET_CLAMP(2,2,1,254,1,254), SCE_GS_CLAMP_1 },
//	{SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 2 ),SCE_GS_TEST_1},
	{ 0, SCE_GS_NOP },
	{ SCE_GS_SET_TEX0( BUFFER_PAGE(2) / 64,
					  BUFFER_WIDTH/64,SCE_GS_PSMCT32,8,8,1,0,0,0,0,0,0), SCE_GS_TEX0_1 },
	{ SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ), SCE_GS_ALPHA_1 },
	{ 0, SCE_GS_NOP },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
} ;

/*----------------------------------------------------------------*/
#if 0
static inline void Vu0CopyMatrix( FMATRIX *dst, FMATRIX *src )
{
	asm volatile ("
		lqc2		vf04,0x00(%1)
		lqc2		vf05,0x10(%1)
		lqc2		vf06,0x20(%1)
		lqc2		vf07,0x30(%1)
		sqc2		vf04,0x00(%0)
		sqc2		vf05,0x10(%0)
		sqc2		vf06,0x20(%0)
		sqc2		vf07,0x30(%0)
	"::"r"(dst),"r"(src));
}
#else
/* Ｖｕ０レジスタ使用スクラッチパッド間のみ使用可能バージョン */
#define Vu0CopyMatrix( _a, _b ) \
	asm volatile ("\
		lqc2		vf04,0x00(%1);\
		lqc2		vf05,0x10(%1);\
		lqc2		vf06,0x20(%1);\
		lqc2		vf07,0x30(%1);\
		sqc2		vf04,0x00(%0);\
		sqc2		vf05,0x10(%0);\
		sqc2		vf06,0x20(%0);\
		sqc2		vf07,0x30(%0);\
	"::"r"(_a),"r"(_b));

#endif
/*----------------------------------------------------------------*/
	/*
		パケット初期化関連
	*/

/* デフォルトパケット初期化ルーチン */
static void SetupDmaPacket( PACKET_BUFFER *buff )
{
	static PACK_DMA_PACKET	def_pack_packet = {
		.verts_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x100, 0, VIF_VERT_PACK, 0 )}},
		.norms_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x101, 0, VIF_NORM_PACK, 0 )}},
		.uvs0_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x102, 0, VIF_UV_PACK, 0 )}},
#if 0
		.uvs1_trans = { DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_NOP( 0 )}},
		.uvs2_trans = { DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_NOP( 0 )}},
		.rgb_trans = { DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_NOP( 0 )}},
#endif
		.trans_tag = { DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _pack_dma_datas) + 1 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x14, SIZEOF_QWORD(struct _pack_dma_datas), VIF_DATA128, 0 ) }},
#if 0
		.datas = {
			.giftag_normal = { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 0, 0, 0),
											   SCE_GIF_PACKED, 3),
								 GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) },
			.giftag_clip = { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0),
											 SCE_GIF_PACKED, 3),
							   GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) },
			.gif_regslist = {GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_NOP, GS_REGS_STQ, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2)},
		},
#endif
		.prog_exec = {{SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 )}},
	} ;

	buff->pack_packet = def_pack_packet ;

	buff->tex_packet.tex0_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	buff->tex_packet.tex0_trans.addr = NULL ;
	buff->tex_packet.tex0_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->tex_packet.tex0_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xe8, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
#if 0
	buff->tex_packet.tex1_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	buff->tex_packet.tex1_trans.addr = NULL ;
	buff->tex_packet.tex1_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->tex_packet.tex1_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xf0, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
	buff->tex_packet.tex2_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	buff->tex_packet.tex2_trans.addr = NULL ;
	buff->tex_packet.tex2_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->tex_packet.tex2_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xf8, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
	/* 無効にしておく */
	buff->tex_packet.tex1_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	buff->tex_packet.tex1_trans.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	buff->tex_packet.tex2_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	buff->tex_packet.tex2_trans.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
#endif

	buff->mat_trans_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(struct _stack_datas) );
	buff->mat_trans_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->mat_trans_tag.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x00, SIZEOF_QWORD(struct _stack_datas), VIF_DATA128, 0 );

}
static void SetupMatrixStack( MATRIX_STACK *mat_stack )
{
	static DG_GIFTAG	giftag_normal = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_clip = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0),SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
	static IVECTOR		gif_regslist = 
	  {GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
		 GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
		 GS_REGS_5( GS_REGS_NOP, GS_REGS_STQ, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
		 GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2)} ;
	mat_stack->next_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_NEXT, 0 );
	mat_stack->next_tag.addr = NULL ;
	mat_stack->next_tag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	mat_stack->next_tag.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	mat_stack->datas.giftag_normal = giftag_normal ;
	mat_stack->datas.giftag_clip = giftag_clip ;
	mat_stack->datas.gif_regslist = gif_regslist ;
}


static u_long128* SetVertsTrans( DG_DMATAG *tag, int vu_oft, int n_verts, int v_oft, u_long128 *verts, int type )
{
	if ( verts != NULL && v_oft != 0 ){
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, v_oft ) ;
		tag->addr = verts ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( vu_oft, n_verts, type, 0 ) ;
		verts += v_oft ;
	} else {
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
		tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	}
	return ( verts );
}


	/*
		ＤＭＡに接続する
	*/
static	void	ChainObj( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int			j ;
	int			first_flag, prog_no, bound ;
	u_long128	*verts, *norms, *uvs0 ;/* スクラッチパッドに入れた方がいいかも */
	PACKET_BUFFER	*buff ;

	Vu0CopyMatrix( &scrpad->matrix_stack.datas.screen, &scrpad->screen );
	Vu0CopyMatrix( &scrpad->matrix_stack.datas.local_light, &scrpad->local_light );
	Vu0CopyMatrix( &scrpad->matrix_stack.datas.local_color, &scrpad->local_color );
	Vu0CopyMatrix( &scrpad->matrix_stack.datas.connection, &scrpad->env_mat );

	/* マトリクスの転送 */
	scrpad->matrix_addr = DG_GetCacheFIFOAddr( scrpad->local_work ) + 1 ;
	scrpad->matrix_stack.next_tag.addr = DG_GetCacheFIFOAddr( scrpad->local_work ) + SIZEOF_QWORD(MATRIX_STACK) ;
	DG_SendCacheFIFO( scrpad->local_work, &scrpad->matrix_stack, SIZEOF_QWORD(MATRIX_STACK) );

	buff = NEXT_BUFF();	/* 次のＤＭＡパケットバッファ取得 */

	/* ＤＭＡパケット生成準備 */
	first_flag = 2 ;
	pack = obj->packets ;
	bound = scrpad->local_bound_mode ;
	verts = (u_long128*)obj->verts ;
	norms = (u_long128*)obj->norms ;
	uvs0 = (u_long128*)obj->uvs[0] ;
#ifdef LIBDG_PERFORMANCE
	scrpad->n_packs += obj->n_packs ;
#endif
	for ( j = obj->n_packs ; j > 0 ; j-- ){
		GV_PREFECH( pack + 1 );
		/* パケット生成 */
		verts = SetVertsTrans( &buff->pack_packet.verts_trans,
							  0x100, pack->verts_offset*2, pack->verts_offset, verts, VIF_VERT_PACK );
		norms = SetVertsTrans( &buff->pack_packet.norms_trans,
							  0x101, pack->norms_offset*2, pack->norms_offset, norms, VIF_NORM_PACK );
		uvs0 = SetVertsTrans( &buff->pack_packet.uvs0_trans,
							 0x102, pack->uvs_offset[0]*4, pack->uvs_offset[0], uvs0, VIF_UV_PACK );
		buff->pack_packet.datas.n_verts = pack->n_verts ;
		buff->pack_packet.datas.flag = pack->flag ;
		if ( type == 0 ){
			buff->tex_packet.tex0_trans.addr = pack->tex_ptr[0] ;
		}
#ifdef LIBDG_PERFORMANCE
		scrpad->n_verts += pack->n_verts ;
#endif

		if ( type == 0 ){
			{/* 実行するＶＵ１プログラムを決定 */
				/* バウンディングチェック */
				if ( bound & 1 )		prog_no = 1 ;
				else					prog_no = 0 ;
				/* エンベロープチェック */
				if ( pack->flag & DG_PACKFLAG_ENVELOPE ) prog_no += 2 ;
				/* プリシェードチェック */
				//if ( obj->flag & DG_FLAG_PAINT ) prog_no += 4 ;
			}
			//buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_MSCAL( SELECT_VU1_PROGRAM_FUNC[prog_no], 0 ) ;
		} else {
			{/* 実行するＶＵ１プログラムを決定 */
				/* バウンディングチェック */
				if ( bound & 1 )		prog_no = 9 ;
				else					prog_no = 8 ;
				if ( scrpad->para_mode ) prog_no += 2 ;
			}
			//buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_MSCAL( 0, 0 ) ;
		}
		buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_MSCAL( SELECT_VU1_PROGRAM_FUNC[prog_no], 0 ) ;
		//buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_MSCAL( prog_no*2, 0 ) ;

		/* パケットをメモリへ転送 */
		if ( first_flag > 0 ){/* 最初の２回のみ */
			//buff->mat_packet.datas.screen = scrpad->screen ;
			//buff->mat_packet.datas.local_light = scrpad->local_light ;
			//buff->mat_packet.datas.local_color = scrpad->local_color ;
			//buff->mat_packet.datas.connection = scrpad->env_mat ;
			buff->mat_trans_tag.addr = scrpad->matrix_addr ;
			buff->pack_packet.datas.fog = scrpad->fog ;
			buff->pack_packet.datas.bound = bound & 1 ;

			DG_SendCacheFIFO( scrpad->local_work, &buff->mat_trans_tag,
							 SIZEOF_QWORD(DG_DMATAG) + SIZEOF_QWORD(PACK_DMA_PACKET) +
							 SIZEOF_QWORD(TEX_DMA_PACKET) );
		} else {
			DG_SendCacheFIFO( scrpad->local_work, &buff->tex_packet,
							 SIZEOF_QWORD(PACK_DMA_PACKET) + SIZEOF_QWORD(TEX_DMA_PACKET) );

		}

		if ( j > 1 ) buff = NEXT_BUFF();	/* 次のＤＭＡパケットバッファ取得 */
		first_flag-- ;
		pack++ ;
	}

}


/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static int BoundCheck( FMATRIX *mat, FVECTOR *bound, int flag )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	int		and_flag, or_flag, prim_over_flag ;

	/* バウンディング用ベクトル生成 */
	scrpad->verts[0].vx = 256 ;
	scrpad->verts[0].vy = 256 ;
	/* バウンディングチェックマイクロプログラム呼び出し */
	asm volatile ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf1,0x00(%1)
	lqc2		vf2,0x10(%1)
	lqc2		vf3,0(%2)
	vcallms		0x00			# バウンディングチェック
	"::"r"(mat),"r"(bound),"r"(&scrpad->verts[0]) );
	/* 計算結果の取得 */
	asm volatile ("
	cfc2.i		%0,$2 /*vi2*/
	cfc2		%1,$3 /*vi3*/
	cfc2		%2,$1 /*vi1*/
	sqc2		vf31,0(%3)
	":"=&r"(and_flag),"=&r"(or_flag),"=&r"(prim_over_flag):"r"(&scrpad->verts[0]) );

	/* バウンディング中央値のＺ座標取得 */
	//SORT_Z = DG_FTOI(BOUNDS[0].vw) ;

	/* バウンディング判定 */
	if ( flag == 0 ){
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
		if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	} else {
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
	}

	return ( 0 );	/* 完全画面内 */

}

/*----------------------------------------------------------------*/

	/*
		ＤＭＡに接続する
	*/
static	void	MakeShadowChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;
	int			bound ;

	/* 強制的に光源をなくして黒で描画 */
	GV_ZeroMemory( &scrpad->local_color, sizeof(FMATRIX) );
	scrpad->local_color.m[3][0] = 128 ;
	scrpad->local_color.m[3][1] = 128 ;
	scrpad->local_color.m[3][2] = 128 ;
	/* 影処理ではローモデルがあればそれを使う */
	//if ( objs->low != NULL ) objs = objs->low ;

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		bound = obj->bound_mode ;
		if ( bound == 2 ) continue ;
		/* その他スキップ条件チェック */
		if ( obj->flag & scrpad->invisible_flag ) continue ;
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;

		/* オブジェクト描画パラメータ構造体にデータをセット */
		scrpad->screen = obj->screen ;
		scrpad->env_mat = obj->inv_mat ;
		scrpad->fog = 0x8fff ;
		scrpad->local_bound_mode = bound ;

		ChainObj( obj, 0 );
	}

}

void DG_CoverShadowChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		/*i, */j, k, size, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	c_gid = cp->group_id ;

	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->eye_pers2 = cp->eye_pers2 ;
	scrpad->raise_eye_pers = cp->raise_eye_pers ;
	scrpad->raise_eye_pers2 = cp->raise_eye_pers2 ;
	scrpad->fog_param1 = 0.0f ;
	scrpad->fog_param2 = 255.0f ;
	/* スクラッチパッド上のパケットバッファを初期化 */
	SetupMatrixStack( &scrpad->matrix_stack );
	SetupDmaPacket( &scrpad->dma_packet[0] );
	SetupDmaPacket( &scrpad->dma_packet[1] );

	DG_StartBoundingCheckSupport();

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	{/* オブジェクト検索高速化用リスト生成 */
		scrpad->max_list = 0 ;
		list = scrpad->list ;
		oque2 = (DG_OBJS**)obj_buff->queue ;
		for ( j = obj_buff->n_queue ; j > 0 ; j--, oque2++ ){
			objs = *oque2 ;
			if ( !( objs->flag & DG_FLAG_MULTITEX ) ) continue ;
			if ( objs->flag & DG_FLAG_PAINT ) continue ;
			if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( !( objs->group_id & cp->group_id ) ) continue ;
			list->objs = objs ;
			list->flag = objs->flag ;
			list->bound_mode = objs->bound_mode ;
			list->group_id = objs->group_id ;
			list++ ;
			scrpad->max_list++ ;
		}
	}

	/* パケットメモリ書き出し用ＦＩＦＯ初期化 */
	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* ＶＵ１マイクロプログラム設定 */
	DG_WritePacket_Vu1ShadowProg( scrpad->dma_buffer, 0 );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, 1 );

	/* 影の描画 */
	size = DG_WriteObjsPacketInit_CoverShadow( scrpad->dma_buffer, cp );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	list = scrpad->list ;
	for ( k = scrpad->max_list ; k > 0 ; k--, list++ ){
		MakeShadowChainObjs( list->objs );
	}

	{
		void	*end_tag ;
		end_tag = DG_PopDefaultDrawEnv( cp, scrpad->dma_buffer );
		size = ( (int)end_tag - (int)scrpad->dma_buffer ) / 16 ;
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}

	/* 終了パケットの設定 */
	//size = DG_WriteObjsPacketEnd( scrpad->dma_buffer );
	{
		DG_DMATAG	*dmatag ;
		dmatag = (DG_DMATAG*)scrpad->dma_buffer ;
		size = ( (int)DG_PopDefaultDrawEnv( cp, scrpad->dma_buffer ) - (int)scrpad->dma_buffer ) / sizeof(u_long128) ;
		dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHE(0);	/* ＶＵ実行終了待ち */
		size += DG_WriteObjsPacketEnd( &scrpad->dma_buffer[size] );
	}

	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();


}

