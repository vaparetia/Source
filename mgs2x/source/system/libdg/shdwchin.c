//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shdwchin.c
	チャンネル処理ユニット／影オブジェクトＤＭＡ接続ルーチン

	1999/07/07 K.Takabe
	$Id: shdwchin.c,v 1.1.1.3 2002/11/19 11:42:25 Yoshizawa1 Exp $

*/
/*

	void		DG_ShadowChainChanl( DG_CHANL *cp, int which )
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

#include "bp_matrix.h"

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"

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

//BP #define NEXT_BUFF()	({ scrpad->buffer_switch = 1 - scrpad->buffer_switch ; (&scrpad->dma_packet[ scrpad->buffer_switch ] ) ;})

/*----------------------------------------------------------------*/
	/*
	*/

/* スポットライトのベーステクスチャイメージへのポインタ */
extern DG_TEX *DG_SpotLightBaseTexture;
#if BP_VITA
SBP_ProjectorNew_StartPacket *BP_ShadowList;

void BP_RB_RelinkShadowCommands(SRenderCommand *s, SRenderCommand *e)
{
   /*
     Have:
     List: ...
     ... s -> mask/creation -> e -> Marker("Apply Shadow") ...

     Want:
     List: ... -> mask/creation -> e
     ... s -> Marker("Apply Shadow") ...

     Cases:

     List empty (!head): head = s->mpNext
     List not empty (tail): tail->mpNext = s->mpNext
     tail = e
     s->mpNext = e->mpNext
    */
   if (!BP_ShadowList->head)
      BP_ShadowList->head = s->mpNext;
   if (BP_ShadowList->tail)
      BP_ShadowList->tail->mpNext = s->mpNext;
   BP_ShadowList->tail = e;

   s->mpNext = e->mpNext;
}

void BP_RB_AddShadowCommand(unsigned int id, void* data)
{
   SRenderCommand* pCmd = (SRenderCommand*)BP_RB_Alloc(sizeof(SRenderCommand));

   pCmd->mId = id;
   pCmd->mData = (char*)data;
   pCmd->mpNext = NULL;

   if (!BP_ShadowList->head)
      BP_ShadowList->head = pCmd;
   if (BP_ShadowList->tail)
      BP_ShadowList->tail->mpNext = pCmd;
   BP_ShadowList->tail = pCmd;
}
#endif

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

typedef struct _shdwchin_ScrpadWork {
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
#if 1 // BP_RENDERER
   FVECTOR		light_pos ;
#endif
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

int	DG_ActiveShadowFlag = 0 ;
int gBP_ShadowStageEnabled = 1;
int gBP_SpotStageEnabled = 1;

/*----------------------------------------------------------------*/

PACKET_BUFFER* NEXT_BUFF(ScrpadWork	*scrpad)	{ scrpad->buffer_switch = 1 - scrpad->buffer_switch ; return (&scrpad->dma_packet[ scrpad->buffer_switch ] ) ;}

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

extern void DG_StartBoundingCheckSupport( void );/* screen.c */
extern int DG_WriteObjsPacketInit_WriteShadow( void *tag_addr, DG_CHANL *cp, float fog_param1, float fog_param2 );

static ALIGN64_PRE DG_TEX_TRANS	Shadow_TransTexture ALIGN64_POST = {
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
static ALIGN64_PRE DG_TEX_TRANS	Spot_TransTexture ALIGN64_POST = {
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
#if 1 //BP_PS2
      //verts_trans
      { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
         SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
            SCE_VIF1_SET_UNPACKR( 0x100, 0, VIF_VERT_PACK, 0 )}},
      //norms_trans
      { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
         SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
            SCE_VIF1_SET_UNPACKR( 0x101, 0, VIF_NORM_PACK, 0 )}},
      //uvs0_trans
      { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
         SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
            SCE_VIF1_SET_UNPACKR( 0x102, 0, VIF_UV_PACK, 0 )}},
      //trans_tag
      { DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _pack_dma_datas) + 1 ),NULL,{
         SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
            SCE_VIF1_SET_UNPACKR( 0x14, SIZEOF_QWORD(struct _pack_dma_datas), VIF_DATA128, 0 ) }},
      //datas
      {
         0
      },
      //prog_exec
      {{SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 )}},
#else
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
#endif
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

#if 0
/* ＶＵ１プログラム用定数初期化用パケットの設定*/
static void SetVu1VarInitPacket( VU1_INIT_PACKET *packet, DG_CHANL *cp )
{
	/* ＶＵ１メモリ初期化パケット生成 */
	static VU1_INIT_PACKET DefVu1InitPacket = {
		.init_vif = {
			.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 3 ),
			.vifcode = { SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_FLUSHE( 0 ) }
		},
		.init_vifcode = {
			SCE_VIF1_SET_BASE( 0x40, 0 ),			/* ダブルバッファのベースアドレス */
			SCE_VIF1_SET_OFFSET( 0x1E0, 0 ),		/* ダブルバッファのオフセット */
			SCE_VIF1_SET_STMOD( 0, 0 ),				/* 加算書き込みＯＦＦ */
			SCE_VIF1_SET_STMASK( 0 ), 0x00FFFF10,	/* １サイクルめのＺにROWの値を書き込む（ＵＶデータ用設定） */
			SCE_VIF1_SET_STROW( 0 ),0,0,4096,0,		/* Ｚメンバに4096（小数点以下１２ビットの固定小数点で1.0を表す） */
			SCE_VIF1_SET_MSKPATH3( 0, 0 ),
			SCE_VIF1_SET_NOP( 0 ),
		},
		.init_param_trans = {/* 転送用ＤＭＡタグ */
			.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _vu1_work) ),
			.vifcode = {
				SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
				SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(struct _vu1_work), VIF_DATA128, 0 )
			}
		},
		.init_param = {/* ＶＵＭｅｍ転送データ */
			.fog_param1 = 0, .fog_param2 = 0, .fog_param3 = 0, .fog_param4 = 0,
			.color_clip = 255.0f, .poly_alpha = 128.0f, .specular_mul = 0.0f, .specular_clip = 0.8f, 
			.scale = { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f },
			.offset = { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f },
			.param1 = 1.0f / 4096.0f, .param2 = 0.5f, .param3 = 0.0f, .param4 = 0.0f, 
			.aspect = {0.0f, 0.0f, 1.0f, 1.0f },
		}
	};

	*packet = DefVu1InitPacket ;
	*(u_long128*)&packet->init_vu1_prog = *(u_long128*)SELECT_VU1_PROGRAM ;
	packet->init_param.pers = cp->eye_pers ;
	packet->init_param.scale.vx = cp->width / 2 ;
	packet->init_param.scale.vy = cp->height / 2 ;
	packet->init_param.aspect.vx = cp->screen * ASPECT_X() ;
	packet->init_param.aspect.vy = cp->screen * ASPECT_Y() * cp->width / cp->height ;
	packet->init_param.fog_param1 = DG_FogParam1 ;
	packet->init_param.fog_param2 = DG_FogParam2 ;
}
#endif

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

#if 0 //BP_PS2
	/*
		ＤＭＡに接続する
	*/
static	void	ChainObj( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
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

	buff = NEXT_BUFF(scrpad);	/* 次のＤＭＡパケットバッファ取得 */

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
#if 0 //BP_RENDER
		buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_MSCAL( SELECT_VU1_PROGRAM_FUNC[prog_no], 0 ) ;
#endif
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

		if ( j > 1 ) buff = NEXT_BUFF(scrpad);	/* 次のＤＭＡパケットバッファ取得 */
		first_flag-- ;
		pack++ ;
	}
}
#else
static void ChainObj( DG_OBJ *obj, int type )
{
   ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR;
   DG_OBJS* objs = (DG_OBJS*)obj->BP_DGObjs;
   DG_OBJ_PACKET	*pack = obj->packets;
   int i;

   // Queue BP local param packet
   for ( i = 0; i < obj->n_packs; ++i, ++pack  )
   {
      SBP_OBJ_Projector_Render* pBPData = (SBP_OBJ_Projector_Render*)BP_RB_Alloc(sizeof(SBP_OBJ_Projector_Render));
      pBPData->obj_flag = obj->flag;
      pBPData->flag = pack->flag;

      pBPData->correction = obj->world;
      if (obj->parent != -1) 
      {
         _sceVu0MulMatrix( &pBPData->correction, &obj->world, &obj->inv_mat );
      }
      pBPData->world = obj->world;
     
      pBPData->lightDir = scrpad->local_light;
      pBPData->lightCol = scrpad->local_color;

      pBPData->model = obj->BP_Model;
      pBPData->unit = obj->BP_ObjIndex;

      BP_RB_CopyTexture(&pBPData->tex, pack->tex_ptr[0]);
      pBPData->startPacket = i;
      pBPData->packetCount = 1;

      pBPData->passFlag = type;

      BP_RB_AddCommand(kCmd_Obj_Projector_Render, (char*)pBPData);
   }
}

static void BP_Projector_LocalParam(DG_OBJS *objs, int type)
{
   //ScrpadWork	*scrpad = SCRPAD_ADDR ;

   SBP_OBJ_Projector_LocalParam* pBPData = (SBP_OBJ_Projector_LocalParam*)BP_RB_Alloc(sizeof(SBP_OBJ_Projector_LocalParam));
   //pBPData->model = (unsigned int)objs->BP_Model;
   pBPData->preshadeBuffer = objs->BP_PreshadeBuffer;

   if ( objs->flag & DG_FLAG_FOGPARAM )
   {
      pBPData->fogParam.vx = objs->fog_param[0];
      pBPData->fogParam.vy = objs->fog_param[1];
      pBPData->fogParam.vz = 0.0f;
      pBPData->fogParam.vw = 1.0f;
   }
   else 
   {
      pBPData->fogParam = BP_FogParam;
   }
   pBPData->shaderType = type;

   BP_RB_AddCommand(kCmd_Obj_Projector_LocalParam, (char*)pBPData);
}
#endif

/*----------------------------------------------------------------*/
#if 0
static void DG_MakeCameraMatrix( FMATRIX *mat, FVECTOR *from, FVECTOR *to )
{
	static FVECTOR	Lower = { 0.0F, -1.0F, 0.0F, 1.0F } ;
	static FVECTOR	prev_vx;
	FVECTOR		z_vec, y_vec, x_vec ;

	mat->m[3][ 0 ] = from->vx ;
	mat->m[3][ 1 ] = from->vy ;
	mat->m[3][ 2 ] = from->vz ;
	mat->m[3][ 3 ] = 1.0 ;
	/*
		カメラ行列を計算する
		Ｚ軸は、「カメラ位置→注目点」ベクトルを正規化
		Ｘ軸は、「下向きベクトル」×Ｚ軸の結果を正規化
		Ｙ軸は、Ｚ軸×Ｘ軸
	*/
	z_vec.vx = ( to->vx - from->vx ) ;
	z_vec.vy = ( to->vy - from->vy ) ;
	z_vec.vz = ( to->vz - from->vz ) ;
	_sceVu0OuterProduct( &x_vec, &Lower, &z_vec ) ;
	if( x_vec.vx == 0.0F && x_vec.vy == 0.0F && x_vec.vz == 0.0F ){
		x_vec = prev_vx;
	} else {
		prev_vx = x_vec;
	}

	_sceVu0Normalize( &x_vec, &x_vec ) ;
	_sceVu0Normalize( &z_vec, &z_vec ) ;
	_sceVu0OuterProduct( &y_vec, &z_vec, &x_vec ) ;
	mat->m[ 0 ][ 0 ] = x_vec.vx ;
	mat->m[ 0 ][ 1 ] = x_vec.vy ;
	mat->m[ 0 ][ 2 ] = x_vec.vz ;
	mat->m[ 0 ][ 3 ] = 0.0F ;
	mat->m[ 1 ][ 0 ] = y_vec.vx ;
	mat->m[ 1 ][ 1 ] = y_vec.vy ;
	mat->m[ 1 ][ 2 ] = y_vec.vz ;
	mat->m[ 1 ][ 3 ] = 0.0F ;
	mat->m[ 2 ][ 0 ] = z_vec.vx ;
	mat->m[ 2 ][ 1 ] = z_vec.vy ;
	mat->m[ 2 ][ 2 ] = z_vec.vz ;
	mat->m[ 2 ][ 3 ] = 0.0F ;
}
#endif
/*----------------------------------------------------------------*/
static void GetLightViewport( DG_OBJS *objs, int num )
{
	FVECTOR	lit_pos, lit_trg ;
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	float		f, r ;

	/* ライトのマトリクスを生成 */
	scrpad->tmp_vec.vx = objs->light[0].m[0][num] ;
	scrpad->tmp_vec.vy = objs->light[0].m[1][num] ;
	scrpad->tmp_vec.vz = objs->light[0].m[2][num] ;
	f = bp_sqrtf( scrpad->tmp_vec.vx * scrpad->tmp_vec.vx + scrpad->tmp_vec.vy * scrpad->tmp_vec.vy + scrpad->tmp_vec.vz * scrpad->tmp_vec.vz ); //BP_MATH - emulate PS2 sqrtf
	r = 1.0f / f ;
	scrpad->tmp_vec.vx *= r ;
	scrpad->tmp_vec.vy *= r ;
	scrpad->tmp_vec.vz *= r ;
	scrpad->color_vector.vx = 128.0f ;
	scrpad->color_vector.vy = 128.0f ;
	scrpad->color_vector.vz = 128.0f ;
	scrpad->color_vector.vw = f * 64.0f ;
	scrpad->light_vector = scrpad->tmp_vec ;
	lit_pos.vx = objs->world.m[3][0] - scrpad->tmp_vec.vx * 6000.0f ;
	lit_pos.vy = objs->world.m[3][1] - scrpad->tmp_vec.vy * 6000.0f ;
	lit_pos.vz = objs->world.m[3][2] - scrpad->tmp_vec.vz * 6000.0f ;
	lit_trg.vx = objs->world.m[3][0] ;
	lit_trg.vy = objs->world.m[3][1] ;
	lit_trg.vz = objs->world.m[3][2] ;
	DG_MakeCameraMatrix( &scrpad->light_trans, &lit_pos, &lit_trg );
	/* ライトのビューポートマトリクスを生成 */
#if 0
	/* パース付き投影 */
	_sceVu0ViewScreenMatrix( &scrpad->light_view,
						   2.0f,
						   1.0f, 1.0f, 0.0f, 0.0f,
						   0.0f, 1.0f, 1000.0f, 16000.0f );
	_sceVu0ViewScreenMatrix( &scrpad->light_view2,
						   2.0f*256.0f,
						   1.0f, 1.0f, 2048.0f, 2048.0f,
						   0.0f, 1.0f, 1000.0f, 16000.0f );
	/* ライト用の透視変換マトリクスの作成 */
	_sceVu0InversMatrix( &scrpad->tmp_mat, &scrpad->light_trans );
	_sceVu0MulMatrix( &scrpad->light_pers, &scrpad->light_view, &scrpad->tmp_mat );
	_sceVu0MulMatrix( &scrpad->light_pers2, &scrpad->light_view2, &scrpad->tmp_mat );
#else
	/* 平行投影 */
	_sceVu0UnitMatrix( &scrpad->light_view );
	scrpad->light_view.m[0][0] = 1.0f/1500.0f ;
	scrpad->light_view.m[1][1] = 1.0f/1500.0f ;
	scrpad->light_view.m[2][2] = -1.0f/65536.0f ;
	scrpad->light_view.m[3][2] = 1.0f ;
	scrpad->light_view2 = scrpad->light_view ;
	scrpad->light_view2.m[0][0] = 1.0f/1500.0f * 128.0f ;
	scrpad->light_view2.m[1][1] = 1.0f/1500.0f * 128.0f ;
	scrpad->light_view2.m[3][0] = 2048.0f ;
	scrpad->light_view2.m[3][1] = 2048.0f ;
	scrpad->light_view2.m[2][2] = -65536.0f/65536.0f ;
	scrpad->light_view2.m[3][2] = 65536.0f ;
	_sceVu0InversMatrix( &scrpad->tmp_mat, &scrpad->light_trans );
	_sceVu0MulMatrix( &scrpad->light_pers, &scrpad->light_view, &scrpad->tmp_mat );
	_sceVu0MulMatrix( &scrpad->light_pers2, &scrpad->light_view2, &scrpad->tmp_mat );
#endif
}
#if 0
static	void	ShadowScreenObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int		i ;

	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; -- i ) {
		//_sceVu0MulMatrix( &obj->screen, &scrpad->light_pers2, &obj->world );
		_sceVu0MulMatrix( &obj->screen, &scrpad->light_pers, &obj->world );
		obj->bound_mode = 0 ;
		obj ++ ;
	}
}
#endif

/*----------------------------------------------------------------*/

static int BoundCheck( FMATRIX *mat, FVECTOR *bound, int mode )
{
#if 1 //BP_ASM
   //BP - copied from xshdwchn.c
   int		and_flag, or_flag, prim_over_flag, flag ;
   FVECTOR		verts, tmp_v ;
   float		w, total_w ;
   int			i ;

   and_flag = 0xffffffff ;
   or_flag = 0 ;
   prim_over_flag = 0 ;
   total_w = 0 ;

   for ( i = 8 ; i > 0 ; i-- ){
      verts.vx = ( i & 1 ) ? bound[0].vx : bound[1].vx ;
      verts.vy = ( i & 2 ) ? bound[0].vy : bound[1].vy ;
      verts.vz = ( i & 4 ) ? bound[0].vz : bound[1].vz ;
      verts.vw = 1.0F ;
      _sceVu0ApplyMatrix( &tmp_v, mat, &verts );
      total_w += tmp_v.vw ;
      w = DG_FABS( tmp_v.vw );

      /* クリップチェック */
      flag = 0 ;
      if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
      if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
      if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
      if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
      if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
      if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
      //if ( tmp_v.vz < 0.0f ) flag |= CLIP_Z1_FLAG;	/* XBOXはPS2とは違うので注意 */
      and_flag &= flag ;
      or_flag |= flag ;
   }
   /* バウンディング中央値のＺ座標取得 */
   //SORT_Z = total_w / 8.0f ;

   /* ＸＢＯＸではクリップによるコストは無視しても大丈夫そうなので */
   if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
   if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */

   return ( 0 );	/* 完全画面内 */
#else
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
	if ( mode == 0 ){
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
		if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	} else {
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
	}
#endif
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

   BP_Projector_LocalParam(objs, 0);
	/* 強制的に光源をなくして黒で描画 */
	GV_ZeroMemory( &scrpad->local_color, sizeof(FMATRIX) );
	//scrpad->local_color.m[3][0] = 128 ;
	//scrpad->local_color.m[3][1] = 256 ;
	//scrpad->local_color.m[3][2] = 256 ;
	/* 影処理ではローモデルがあればそれを使う */
	if ( objs->low != NULL ) objs = objs->low ;

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &obj->world );
		if ( ( bound = scrpad->bound_mode ) == 1 ){
			bound = BoundCheck( &scrpad->tmp_mat, &obj->bound_min, 0 ) ;
		}
		if ( bound == 2 ) continue ;
		/* その他スキップ条件チェック */
		if ( objs->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ){
			/* 影生成フラグがついている場合のみ関節の表示・非表示を見るようにする */
			if ( obj->flag & scrpad->invisible_flag ) continue ;
		}
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;

		/* オブジェクト描画パラメータ構造体にデータをセット */
		if ( bound == 0 ){
			//_sceVu0MulMatrix( &scrpad->screen, &scrpad->light_pers2, &obj->world );
			scrpad->screen = scrpad->tmp_mat ;
		} else {
			scrpad->screen = scrpad->tmp_mat ;
		}
		scrpad->env_mat = obj->inv_mat ;
		scrpad->fog = 0x8fff ;
		scrpad->local_bound_mode = bound ;

#ifdef LIBDG_PERFORMANCE
		scrpad->n_packs = 0 ;
		scrpad->n_verts = 0 ;
#endif
		ChainObj( obj, 0 );
#ifdef LIBDG_PERFORMANCE
		scrpad->shadow_draw[bound].n_obj++ ;
		scrpad->shadow_draw[bound].n_packs += scrpad->n_packs ;
		scrpad->shadow_draw[bound].n_verts += scrpad->n_verts ;
#endif
	}

}

static	void	WriteShadowChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i, bound ;

   BP_Projector_LocalParam(objs, 1);
	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		/* その他スキップ条件チェック */
		if ( obj->flag & scrpad->invisible_flag ) continue ;
		if ( obj->model->type & ( DG_TYPE_TRANS | DG_TYPE_OVERLAY0 | DG_TYPE_OVERLAY1 | DG_TYPE_OVERLAY2 ) ) continue ;
		/* スポットライトから見たバウンディングをチェック */
		_sceVu0MulMatrix( &scrpad->local_light, &scrpad->light_pers, &obj->world );
		bound = BoundCheck( &scrpad->local_light, &obj->bound_min, 0 ) ;
		if ( bound == 2 ) continue ;
		if ( scrpad->para_mode == 0 ){
			/* スポットライト時 */
			scrpad->local_bound_mode = obj->bound_mode | bound ;
#if 1 // BP_RENDERER
         scrpad->local_light = scrpad->light_pers;
#endif
		} else {
			/* 平行投影時 */
			/* 現在は投影によるニアクリップを行っていないため */
			scrpad->local_bound_mode = obj->bound_mode ;
			/* 並行投影でシャドウマッピング描画を行う場合にテクスチャ射影マトリクスに細工をする */
			scrpad->tmp_mat = scrpad->light_pers ;
			//scrpad->tmp_mat.m[2][3] = 1.0f ;
			//scrpad->tmp_mat.m[3][3] = 0.0f ;
#if 0 // BP_RENDERER
			_sceVu0MulMatrix( &scrpad->local_light, &scrpad->tmp_mat, &obj->world );
#else
         scrpad->local_light = scrpad->light_pers;
#endif
		}

		/* オブジェクト描画パラメータ構造体にデータをセット */
		//scrpad->screen = obj->screen ;
#if 1 /* 優先上げ実験 */
		if ( scrpad->local_bound_mode == 0 ){
			//_sceVu0MulMatrix( &scrpad->screen, &scrpad->raise_eye_pers2, &obj->world );
			_sceVu0MulMatrix( &scrpad->screen, &scrpad->raise_eye_pers, &obj->world );
		} else {
			_sceVu0MulMatrix( &scrpad->screen, &scrpad->raise_eye_pers, &obj->world );
		}
#endif
		scrpad->env_mat = obj->inv_mat ;
		{/* フォグ計算 */
			float	f ;
			int		ifog ;
			f = scrpad->fog_param1 * obj->sort_z + scrpad->fog_param2 ;
			//f =  DG_FogParam1 * obj->sort_z + DG_FogParam2 ;
			f = DG_MAX( f, 0.0f );
			f = DG_MIN( f, 255.0f );
			ifog = DG_FTOI( f ) ;
			scrpad->fog = ( ifog << 4 ) | 0x8000 ;
		}
		*(FVECTOR*)scrpad->local_color.m[0] = scrpad->color_vector ;
		scrpad->local_color.m[1][3] = scrpad->far_range ;
#if 1 // BP_RENDERER
      *(FVECTOR*)scrpad->local_color.m[2] = scrpad->light_vector ;
      *(FVECTOR*)scrpad->local_color.m[3] = scrpad->light_pos ;
#else
		/* 光源ベクトルをモデル座標相対に変換 */
		// *(FVECTOR*)scrpad->local_color.m[2] = scrpad->light_vector ;
#if 1 // BP_ASM
		{
			FMATRIX	tmp_mat ;
			_sceVu0InversMatrix( &tmp_mat, &obj->world );
			scrpad->light_vector.vw = 0.0f ;
			_sceVu0ApplyMatrix( (FVECTOR*)scrpad->local_color.m[2], &tmp_mat, &scrpad->light_vector );
		}
#else
		asm ("
			lqc2		vf09,0x00(%1)
			lqc2		vf10,0x10(%1)
			lqc2		vf11,0x20(%1)
			lqc2		vf08,0x00(%2)
			vaddax.x	ACC ,vf00,vf09
			vaddax.y	ACC ,vf00,vf10
			vaddax.z	ACC ,vf00,vf11
			vmaddx.xyz	vf04,vf00,vf00
			vadday.x	ACC ,vf00,vf09
			vadday.y	ACC ,vf00,vf10
			vadday.z	ACC ,vf00,vf11
			vmaddx.xyz	vf05,vf00,vf00
			vaddaz.x	ACC ,vf00,vf09
			vaddaz.y	ACC ,vf00,vf10
			vaddaz.z	ACC ,vf00,vf11
			vmaddx.xyz	vf06,vf00,vf00
			vmulax.xyz	ACC ,vf04,vf08
			vmadday.xyz	ACC ,vf05,vf08
			vmaddz.xyz	vf08,vf06,vf08
			sqc2		vf08,0x00(%0)
		"::"r"(scrpad->local_color.m[2]), "r"(&obj->world),"r"(&scrpad->light_vector) );
#endif
#endif

#ifdef LIBDG_PERFORMANCE
		scrpad->n_packs = 0 ;
		scrpad->n_verts = 0 ;
#endif
		ChainObj( obj, 1 );
#ifdef LIBDG_PERFORMANCE
		scrpad->projection[scrpad->local_bound_mode].n_obj++ ;
		scrpad->projection[scrpad->local_bound_mode].n_packs += scrpad->n_packs ;
		scrpad->projection[scrpad->local_bound_mode].n_verts += scrpad->n_verts ;
#endif
	}
}

/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
void _DG_ShadowChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs, **oque2 ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, j, size, gid, c_gid ;

   BP_TODO_BREAK;
   return;

	if ( DG_ActiveShadowFlag != 1 ) return ;

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
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
	scrpad->far_range = 1000000.0f ;
	/* スクラッチパッド上のパケットバッファを初期化 */
	SetupMatrixStack( &scrpad->matrix_stack );
	SetupDmaPacket( &scrpad->dma_packet[0] );
	SetupDmaPacket( &scrpad->dma_packet[1] );
#ifdef LIBDG_PERFORMANCE
	scrpad->projection[0] = DG_PerformanceData.shadow_projection[0] ;
	scrpad->projection[1] = DG_PerformanceData.shadow_projection[1] ;
	scrpad->shadow_draw[0] = DG_PerformanceData.shadow_draw[0] ;
	scrpad->shadow_draw[1] = DG_PerformanceData.shadow_draw[1] ;
#endif

	DG_StartBoundingCheckSupport();

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	/* 初期化パケットの設定 */
	//size = DG_WriteObjsPacketInit( next_buff, cp );
	//next_buff = DG_SendScrpadDmaBuffer( size );

	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* ＶＵ１マイクロプログラム設定 */
	DG_WritePacket_Vu1ShadowProg( scrpad->dma_buffer, 0 );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, 1 );

	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		//if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
		if ( !( objs->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;

		/* ライトの描画 */
		size = DG_WriteObjsPacketInit_MakeShadow( scrpad->dma_buffer, cp );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
		GetLightViewport( *oque, 0 );		/* 影投影マトリクス生成 */
#if 0
		scrpad->root = objs->world ;
		if ( objs->flag & DG_FLAG_ONEPIECE ){
			scrpad->root.m[3][0] += objs->def->tx ;
			scrpad->root.m[3][1] += objs->def->ty ;
			scrpad->root.m[3][2] += objs->def->tz ;
		}
		_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
		if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) == 2 ) continue ;
#else
		scrpad->bound_mode = 1 ;
#endif
		while ( objs != NULL ){
			//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
			MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
			objs = objs->next_dgobjs ;				/* 関連付けされた他のオブジェクトも処理 */
		}

		/* 影の描画 */
		size = DG_WriteObjsPacketInit_WriteShadow( scrpad->dma_buffer, cp, scrpad->fog_param1, scrpad->fog_param2 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
		scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
		scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
		oque2 = (DG_OBJS**)obj_buff->queue ;
		for ( j = obj_buff->n_queue ; j > 0 ; j--, oque2++ ){
			objs = *oque2 ;
			if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			WriteShadowChainObjs( *oque2 );
		}
	}

	/* 終了パケットの設定 */
	//size = DG_WriteObjsPacketEnd( scrpad->dma_buffer );
	size = ( (int)DG_PopDefaultDrawEnv( cp, scrpad->dma_buffer ) - (int)scrpad->dma_buffer ) / sizeof(u_long128) ;
	size += DG_WriteObjsPacketEnd( &scrpad->dma_buffer[size] );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();
#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.shadow_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.shadow_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

}



/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

static void GetSpotViewport( DG_SPOT *spot )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	float		angle, near ;

	near = 100.0f ;
	angle = 2.0f ;
	angle = spot->angle ;
	/* ライトのビューポートマトリクスを生成 */
	scrpad->color_vector.vx = 128.0f ;
	scrpad->color_vector.vy = 128.0f ;
	scrpad->color_vector.vz = 128.0f ;
	scrpad->color_vector.vw = 128.0f ;
	scrpad->light_vector = *(FVECTOR*)spot->world.m[2] ;
#if 1 // BP_RENDER
   scrpad->light_pos = *(FVECTOR*)spot->world.m[3] ;
#endif
	scrpad->far_range = spot->range * 2 - near ;
	scrpad->para_mode = 0 ;
	/* パース付き投影 */
	_sceVu0ViewScreenMatrix( &scrpad->light_view,
						   angle,
						   1.0f, 1.0f, 0.0f, 0.0f,
						   -1.0f, 1.0f, near, spot->range * 2 );
#if 1 // BP_RENDERER
	_sceVu0ViewScreenMatrix( &scrpad->light_view2,
						   angle*128.0f,
						   1.0f, 1.0f, 2048.0f, 2048.0f,
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, near, spot->range * 2 );
#else
   _sceVu0ViewScreenMatrix( &scrpad->light_view2,
      angle*128.0f,
      1.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, near, spot->range * 2 );
#endif
	/* ライト用の透視変換マトリクスの作成 */
	_sceVu0InversMatrix( &scrpad->tmp_mat, &spot->world );
	_sceVu0MulMatrix( &scrpad->light_pers, &scrpad->light_view, &scrpad->tmp_mat );
	_sceVu0MulMatrix( &scrpad->light_pers2, &scrpad->light_view2, &scrpad->tmp_mat );
}

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
#if BP_VITA
void DG_SpotChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		/*i, */j, k, size, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;
   SRenderCommand *mstart, *mend;

	if ( DG_ActiveShadowFlag != 2 ) return ;

   if( !gBP_SpotStageEnabled )
      return;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;

   BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   BP_RB_PushMarker("DG_SpotChainChanl");

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
#ifdef LIBDG_PERFORMANCE
	scrpad->projection[0] = DG_PerformanceData.spot_projection[0] ;
	scrpad->projection[1] = DG_PerformanceData.spot_projection[1] ;
	scrpad->shadow_draw[0] = DG_PerformanceData.shadow_draw[0] ;
	scrpad->shadow_draw[1] = DG_PerformanceData.shadow_draw[1] ;
#endif

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
			//if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( ( DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE ) & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
			list->objs = objs ;
			list->flag = objs->flag ;
			list->shadow_id = objs->shadow_id ;
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

	spot_queue = (DG_SPOT**)que->spot_buffer.queue ;
	for ( k = que->spot_buffer.n_queue ; k > 0 ; k--, spot_queue++ ){
		spot = *spot_queue ;
		if ( spot->flag & DG_FLAG_INVISIBLE ) continue ;

		shadow_id = spot->shadow_id ;
		enable = 0 ;

		gid = 0 ;
		/* 画面内表示チェック */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->flag & scrpad->invisible_flag ) continue ;
			/* 同じＩＤのスポットライト投影モデルが存在する場合に処理を行なう */
			if ( list->shadow_id == shadow_id ){
				if ( list->bound_mode != 2 ){
					enable = 1 ;
					gid |= list->group_id ;
				}
			}
		}
		if ( enable == 0 ) continue ;

		GetSpotViewport( spot );

      mstart = (SRenderCommand*)BP_RB_GetCurrentPtr();
      BP_RB_AddCommand(kCmd_ProjectorNew_Dummy, NULL);

      BP_RB_PushMarker("SpotShadow Mask");

		/* 影マスク描画 */
		size = DG_WriteObjsPacketInit_MakeShadowMask( scrpad->dma_buffer, cp, spot->color );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_ProjectorNew_InitPacket* pBPData = (SBP_ProjectorNew_InitPacket*)BP_RB_Alloc(sizeof(SBP_ProjectorNew_InitPacket));
         pBPData->eye_pers = scrpad->light_pers;
         pBPData->shadow = BP_ShadowList->nspots;
         BP_RB_AddCommand(kCmd_ProjectorNew_InitPacket, (char*)pBPData);
      }

		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->shadow_id != shadow_id ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
		}

      BP_RB_PopMarker();

		//GV_SET_PRFC_CLOCK();
		/* ライトの描画 */
		size = DG_WriteObjsPacketInit_MakeSpot( scrpad->dma_buffer, cp, spot->color, DG_SpotLightBaseTexture, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_Projector_SpotStage_PostZScreenPacket* pBPData = (SBP_Projector_SpotStage_PostZScreenPacket*)BP_RB_Alloc(sizeof(SBP_Projector_SpotStage_PostZScreenPacket));
         BP_RB_CopyTexture(&pBPData->tex, DG_SpotLightBaseTexture);
         pBPData->mgsColor = spot->color;
         BP_RB_AddCommand(kCmd_Projector_SpotStage_PostZScreenPacket, (char*)pBPData);
      }

      BP_RB_PushMarker("SpotShadow creation");

		//time1 = GV_GET_PRFC_CLOCK();

		//GV_SET_PRFC_CLOCK();
		//debug_count = 0 ;
		/* 影の生成 */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			//if ( list->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( list->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( !( list->group_id & gid ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) == 2 ) continue ;
			//if ( GV_PadData[1].press & PAD_A ) printf("%d %d %08x\n", shadow_id, objs->n_models, list->group_id );
			//if ( GV_PadData[1].press & PAD_A ) if ( objs->n_models > 20 ) objs->n_models = 5 ;
			while ( objs != NULL ){
				//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
				MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
				objs = objs->next_dgobjs ;				/* 関連付けされた他のオブジェクトも処理 */
				//debug_count++ ;
				scrpad->bound_mode = 1 ;
			}
		}
		//time2 = GV_GET_PRFC_CLOCK();

      BP_RB_PopMarker();

      mend = BP_RB_GetCurrentPtr();
      BP_RB_AddCommand(kCmd_ProjectorNew_Dummy, NULL);

		//GV_SET_PRFC_CLOCK();
		/* 影の描画 */
		scrpad->raise_eye_pers = cp->raise_eye_pers ;
		scrpad->raise_eye_pers2 = cp->raise_eye_pers2 ;
		size = DG_WriteObjsPacketInit_WriteShadow( scrpad->dma_buffer, cp, scrpad->fog_param1, scrpad->fog_param2 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_ProjectorNew_ShadowProjStart* pBPData = (SBP_ProjectorNew_ShadowProjStart*)BP_RB_Alloc(sizeof(SBP_ProjectorNew_ShadowProjStart));
         pBPData->eye_pers = cp->raise_eye_pers;
         pBPData->shadow = BP_ShadowList->nspots;
         BP_RB_AddCommand(kCmd_ProjectorNew_SpotProjStart, (char*)pBPData);
         BP_ShadowList->nspots++;
      }

      BP_RB_PushMarker("Apply Spot");

      BP_RB_RelinkShadowCommands(mstart, mend);

		if ( 0 == 0 ){
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
		} else {
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
		}
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->shadow_id != shadow_id ) continue ;
			WriteShadowChainObjs( list->objs );
		}
		//time3 = GV_GET_PRFC_CLOCK();

		//if ( GV_PadData[1].press & PAD_A )printf("shdwchin:%d %d %d %d %08x\n", time1, time2, time3, debug_count, shadow_id );

      BP_RB_PopMarker();
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
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();

#if 0
	scePcStop();
	if ( GV_PadData[1].press & PAD_A ){
		int		t0, t1 ;
		t0 = scePcGetCounter0();
		t1 = scePcGetCounter1();
		printf("spot %d %d\n", t0, t1);
	}
#endif
#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

   {
      BP_RB_AddCommand(kCmd_ProjectorNew_EndPacket, NULL);
   }
   BP_RB_PopMarker();
}
#else
void DG_SpotChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		/*i, */j, k, size, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;

	if ( DG_ActiveShadowFlag != 2 ) return ;

   if( !gBP_SpotStageEnabled )
      return;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;

   BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   BP_RB_PushMarker("DG_SpotChainChanl");
   {
      BP_RB_AddCommand(kCmd_Projector_SpotStage_StartPacket, NULL);
   }

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
#ifdef LIBDG_PERFORMANCE
	scrpad->projection[0] = DG_PerformanceData.spot_projection[0] ;
	scrpad->projection[1] = DG_PerformanceData.spot_projection[1] ;
	scrpad->shadow_draw[0] = DG_PerformanceData.shadow_draw[0] ;
	scrpad->shadow_draw[1] = DG_PerformanceData.shadow_draw[1] ;
#endif

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
			//if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( ( DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE ) & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
			list->objs = objs ;
			list->flag = objs->flag ;
			list->shadow_id = objs->shadow_id ;
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

	spot_queue = (DG_SPOT**)que->spot_buffer.queue ;
	for ( k = que->spot_buffer.n_queue ; k > 0 ; k--, spot_queue++ ){
		spot = *spot_queue ;
		if ( spot->flag & DG_FLAG_INVISIBLE ) continue ;

		shadow_id = spot->shadow_id ;
		enable = 0 ;

		gid = 0 ;
		/* 画面内表示チェック */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->flag & scrpad->invisible_flag ) continue ;
			/* 同じＩＤのスポットライト投影モデルが存在する場合に処理を行なう */
			if ( list->shadow_id == shadow_id ){
				if ( list->bound_mode != 2 ){
					enable = 1 ;
					gid |= list->group_id ;
				}
			}
		}
		if ( enable == 0 ) continue ;

		GetSpotViewport( spot );

      BP_RB_PushMarker("SpotShadow Mask");

		/* 影マスク描画 */
		size = DG_WriteObjsPacketInit_MakeShadowMask( scrpad->dma_buffer, cp, spot->color );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_Projector_SpotStage_InitPacket* pBPData = (SBP_Projector_SpotStage_InitPacket*)BP_RB_Alloc(sizeof(SBP_Projector_SpotStage_InitPacket));
         pBPData->eye_pers = scrpad->light_pers;
         BP_RB_AddCommand(kCmd_Projector_SpotStage_InitPacket, (char*)pBPData);
      }

		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->shadow_id != shadow_id ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
		}

      BP_RB_PopMarker();

		//GV_SET_PRFC_CLOCK();
		/* ライトの描画 */
		size = DG_WriteObjsPacketInit_MakeSpot( scrpad->dma_buffer, cp, spot->color, DG_SpotLightBaseTexture, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_Projector_SpotStage_PostZScreenPacket* pBPData = (SBP_Projector_SpotStage_PostZScreenPacket*)BP_RB_Alloc(sizeof(SBP_Projector_SpotStage_PostZScreenPacket));
         BP_RB_CopyTexture(&pBPData->tex, DG_SpotLightBaseTexture);
         pBPData->mgsColor = spot->color;
         BP_RB_AddCommand(kCmd_Projector_SpotStage_PostZScreenPacket, (char*)pBPData);
      }

      BP_RB_PushMarker("SpotShadow creation");

		//time1 = GV_GET_PRFC_CLOCK();

		//GV_SET_PRFC_CLOCK();
		//debug_count = 0 ;
		/* 影の生成 */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			//if ( list->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( list->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( !( list->group_id & gid ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) == 2 ) continue ;
			//if ( GV_PadData[1].press & PAD_A ) printf("%d %d %08x\n", shadow_id, objs->n_models, list->group_id );
			//if ( GV_PadData[1].press & PAD_A ) if ( objs->n_models > 20 ) objs->n_models = 5 ;
			while ( objs != NULL ){
				//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
				MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
				objs = objs->next_dgobjs ;				/* 関連付けされた他のオブジェクトも処理 */
				//debug_count++ ;
				scrpad->bound_mode = 1 ;
			}
		}
		//time2 = GV_GET_PRFC_CLOCK();

		//GV_SET_PRFC_CLOCK();
		/* 影の描画 */
		scrpad->raise_eye_pers = cp->raise_eye_pers ;
		scrpad->raise_eye_pers2 = cp->raise_eye_pers2 ;
		size = DG_WriteObjsPacketInit_WriteShadow( scrpad->dma_buffer, cp, scrpad->fog_param1, scrpad->fog_param2 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_Projector_SpotStage_ShadowProjStart* pBPData = (SBP_Projector_SpotStage_ShadowProjStart*)BP_RB_Alloc(sizeof(SBP_Projector_SpotStage_ShadowProjStart));
         pBPData->eye_pers = cp->raise_eye_pers;
         BP_RB_AddCommand(kCmd_Projector_SpotStage_ShadowProjStart, (char*)pBPData);
      }

      BP_RB_PopMarker();

      BP_RB_PushMarker("Apply Spot");

		if ( 0 == 0 ){
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
		} else {
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
		}
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->shadow_id != shadow_id ) continue ;
			WriteShadowChainObjs( list->objs );
		}
		//time3 = GV_GET_PRFC_CLOCK();

		//if ( GV_PadData[1].press & PAD_A )printf("shdwchin:%d %d %d %d %08x\n", time1, time2, time3, debug_count, shadow_id );

      BP_RB_PopMarker();
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
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();

#if 0
	scePcStop();
	if ( GV_PadData[1].press & PAD_A ){
		int		t0, t1 ;
		t0 = scePcGetCounter0();
		t1 = scePcGetCounter1();
		printf("spot %d %d\n", t0, t1);
	}
#endif
#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

   {
      BP_RB_AddCommand(kCmd_Projector_SpotStage_EndPacket, NULL);
   }
   BP_RB_PopMarker();
}
#endif





/* ---------------------------------------------------------------- */
static void OrthoScreenMatrix( FMATRIX *res, float width, float height, float scale,
					   float zmin, float zmax, float nearz, float farz )
{
	float	z_width, z_center ;

	_sceVu0UnitMatrix( res );
	/* X */
	res->m[0][0] = 2.0f * scale / width ;
	/* Y */
	res->m[1][1] = 2.0f * scale / height ;
	/* Z */
	z_width = ( farz - nearz ) / ( zmax - zmin ) ;
	z_center = ( -farz * zmin + nearz * zmax ) / ( zmax - zmin ) ;
	res->m[2][2] = -1.0f / z_width ;
	res->m[3][2] = z_center / z_width ;
	/* W */
	res->m[3][3] = 1.0f ;
	
}
static void GetParallelViewport( DG_SPOT *spot )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	float		angle, near ;

	near = 100.0f ;
	angle = spot->angle ;
	/* ライトのビューポートマトリクスを生成 */
	scrpad->color_vector.vx = 128.0f ;
	scrpad->color_vector.vy = 128.0f ;
	scrpad->color_vector.vz = 128.0f ;
	scrpad->color_vector.vw = (float)( spot->color >> 24 ) ;
	scrpad->light_vector = *(FVECTOR*)spot->world.m[2] ;
   scrpad->light_pos = *(FVECTOR*)spot->world.m[3] ;
	scrpad->far_range = spot->range * 2 - near ;

	if ( angle == 0.0f ){
		OrthoScreenMatrix( &scrpad->light_view, 3000.0f, 3000.0f, 1.0f,
						  -1.0f, 1.0f, near, spot->range * 2 );
#if 1 // BP_RENDERER
		OrthoScreenMatrix( &scrpad->light_view2, 3000.0f, 3000.0f, 128.0f,
						  (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, near, spot->range * 2 );
#else
      OrthoScreenMatrix( &scrpad->light_view2, 3000.0f, 3000.0f, 128.0f,
         0.0f, 1.0f, near, spot->range * 2 );
#endif
#if 0 // BP_RENDERER
		scrpad->light_view2.m[3][0] = 2048.0f ;
		scrpad->light_view2.m[3][1] = 2048.0f ;
#endif
		scrpad->para_mode = 1 ;
	} else {
		/* パース付き投影 */
		_sceVu0ViewScreenMatrix( &scrpad->light_view,
								angle,
								1.0f, 1.0f, 0.0f, 0.0f,
								-1.0f, 1.0f, near, spot->range * 2 );
#if 0 // BP_RENDERER
		_sceVu0ViewScreenMatrix( &scrpad->light_view2,
								angle*128.0f,
								1.0f, 1.0f, 2048.0f, 2048.0f,
								(float)DRAW_Z_MIN, (float)DRAW_Z_MAX, near, spot->range * 2 );
#else
      _sceVu0ViewScreenMatrix( &scrpad->light_view2,
         angle*128.0f,
         1.0f, 1.0f, 0.0f, 0.0f,
         0.0f, 1.0f, near, spot->range * 2 );
#endif

		scrpad->para_mode = 0 ;
	}
	/* ライト用の透視変換マトリクスの作成 */
	_sceVu0InversMatrix( &scrpad->tmp_mat, &spot->world );
	_sceVu0MulMatrix( &scrpad->light_pers, &scrpad->light_view, &scrpad->tmp_mat );
	_sceVu0MulMatrix( &scrpad->light_pers2, &scrpad->light_view2, &scrpad->tmp_mat );
}


#if BP_VITA
void DG_ShadowChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		/*i, */j, k, size, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;
   SRenderCommand *mstart, *mend;

	if ( DG_ActiveShadowFlag != 1 ) return ;

   if( !gBP_ShadowStageEnabled )
      return;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;

   BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   BP_RB_PushMarker("DG_ShadowChainChanl");

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
#ifdef LIBDG_PERFORMANCE
	scrpad->projection[0] = DG_PerformanceData.spot_projection[0] ;
	scrpad->projection[1] = DG_PerformanceData.spot_projection[1] ;
	scrpad->shadow_draw[0] = DG_PerformanceData.shadow_draw[0] ;
	scrpad->shadow_draw[1] = DG_PerformanceData.shadow_draw[1] ;
#endif

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
			//if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( ( DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE ) & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
			list->objs = objs ;
			list->flag = objs->flag ;
			list->shadow_id = objs->shadow_id ;
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

	spot_queue = (DG_SPOT**)que->spot_buffer.queue ;
	for ( k = que->spot_buffer.n_queue ; k > 0 ; k--, spot_queue++ ){
		spot = *spot_queue ;
		if ( spot->flag & DG_FLAG_INVISIBLE ) continue ;

		shadow_id = spot->shadow_id ;
		enable = 0 ;

		gid = 0 ;
		/* 画面内表示チェック */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->flag & scrpad->invisible_flag ) continue ;
			/* 同じＩＤのスポットライト投影モデルが存在する場合に処理を行なう */
			if ( list->shadow_id == shadow_id || shadow_id == 0 ){
				if ( list->bound_mode != 2 ){
					enable = 1 ;
					gid |= list->group_id ;
				}
			}
		}
		if ( enable == 0 ) continue ;

		GetParallelViewport( spot );

      mstart = (SRenderCommand*)BP_RB_GetCurrentPtr();
      BP_RB_AddCommand(kCmd_ProjectorNew_Dummy, NULL);

      BP_RB_PushMarker("Shadow Mask");

		/* 影マスク描画 */
		size = DG_WriteObjsPacketInit_MakeShadowMask( scrpad->dma_buffer, cp, spot->color );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_ProjectorNew_InitPacket* pBPData = (SBP_ProjectorNew_InitPacket*)BP_RB_Alloc(sizeof(SBP_ProjectorNew_InitPacket));
         pBPData->eye_pers = scrpad->light_pers;
         pBPData->shadow = BP_ShadowList->nspots;
         BP_RB_AddCommand(kCmd_ProjectorNew_InitPacket, (char*)pBPData);
      }

		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			//if ( list->shadow_id != shadow_id ) continue ;
			if ( ( list->shadow_id != shadow_id ) && ( shadow_id != 0 ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
		}

      BP_RB_PopMarker();

		/* ライトの描画 */
		size = DG_WriteObjsPacketInit_MakeSpot( scrpad->dma_buffer, cp, spot->color, DG_SpotLightBaseTexture, 1 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         BP_RB_AddCommand(kCmd_Projector_ShadowStage_PostZScreenPacket, NULL);
      }

      BP_RB_PushMarker("Shadow creation");
		//debug_count = 0 ;
		/* 影の生成 */
#if 0
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			//if ( list->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( list->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( !( list->group_id & gid ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) == 2 ) continue ;
			while ( objs != NULL ){
				//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
				MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
				objs = objs->next ;				/* 関連付けされた他のオブジェクトも処理 */
				//debug_count++ ;
				scrpad->bound_mode = 1 ;
			}
		}
#else
		if ( ( objs = spot->objs ) != NULL )
      {
			if ( objs->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) )
         {
				scrpad->root = objs->world ;
				if ( objs->flag & DG_FLAG_ONEPIECE ){
					scrpad->root.m[3][0] += objs->def->tx ;
					scrpad->root.m[3][1] += objs->def->ty ;
					scrpad->root.m[3][2] += objs->def->tz ;
				}
				_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
				if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) != 2 )
            {
					while ( objs != NULL ){
						//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
						MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
						objs = objs->next_dgobjs ;				/* 関連付けされた他のオブジェクトも処理 */
						//debug_count++ ;
						scrpad->bound_mode = 1 ;
					}
				}
			}
		}
#endif

      BP_RB_PopMarker();

      mend = BP_RB_GetCurrentPtr();
      BP_RB_AddCommand(kCmd_ProjectorNew_Dummy, NULL);

		/* 影の描画 */
		scrpad->raise_eye_pers = cp->raise_eye_pers ;
		scrpad->raise_eye_pers2 = cp->raise_eye_pers2 ;
		size = DG_WriteObjsPacketInit_WriteShadow( scrpad->dma_buffer, cp, scrpad->fog_param1, scrpad->fog_param2 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_ProjectorNew_ShadowProjStart* pBPData = (SBP_ProjectorNew_ShadowProjStart*)BP_RB_Alloc(sizeof(SBP_ProjectorNew_ShadowProjStart));
         pBPData->eye_pers = cp->raise_eye_pers;
         pBPData->shadow = BP_ShadowList->nspots;
         BP_RB_AddCommand(kCmd_ProjectorNew_ShadowProjStart, (char*)pBPData);
         BP_ShadowList->nspots++;
      }

      BP_RB_PushMarker("Apply Shadow");

      BP_RB_RelinkShadowCommands(mstart, mend);

		if ( 1 == 0 ){
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
		} else {
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
		}
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( list->shadow_id != shadow_id ) && ( shadow_id != 0 ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			//_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			//if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			scrpad->bound_mode = objs->bound_mode ;
			WriteShadowChainObjs( list->objs );
		}

      BP_RB_PopMarker();
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

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

   {
      BP_RB_AddCommand(kCmd_ProjectorNew_EndPacket, NULL);
   }
   BP_RB_PopMarker();
}
#else
//void DG_SpotChainChanl( DG_CHANL *cp, int which )
void DG_ShadowChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		/*i, */j, k, size, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;

	if ( DG_ActiveShadowFlag != 1 ) return ;

   if( !gBP_ShadowStageEnabled )
      return;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;

   BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   BP_RB_PushMarker("DG_ShadowChainChanl");
   {
      BP_RB_AddCommand(kCmd_Projector_ShadowStage_StartPacket, NULL);
   }

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
#ifdef LIBDG_PERFORMANCE
	scrpad->projection[0] = DG_PerformanceData.spot_projection[0] ;
	scrpad->projection[1] = DG_PerformanceData.spot_projection[1] ;
	scrpad->shadow_draw[0] = DG_PerformanceData.shadow_draw[0] ;
	scrpad->shadow_draw[1] = DG_PerformanceData.shadow_draw[1] ;
#endif

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
			//if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( ( DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE ) & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
			list->objs = objs ;
			list->flag = objs->flag ;
			list->shadow_id = objs->shadow_id ;
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

	spot_queue = (DG_SPOT**)que->spot_buffer.queue ;
	for ( k = que->spot_buffer.n_queue ; k > 0 ; k--, spot_queue++ ){
		spot = *spot_queue ;
		if ( spot->flag & DG_FLAG_INVISIBLE ) continue ;

		shadow_id = spot->shadow_id ;
		enable = 0 ;

		gid = 0 ;
		/* 画面内表示チェック */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->flag & scrpad->invisible_flag ) continue ;
			/* 同じＩＤのスポットライト投影モデルが存在する場合に処理を行なう */
			if ( list->shadow_id == shadow_id || shadow_id == 0 ){
				if ( list->bound_mode != 2 ){
					enable = 1 ;
					gid |= list->group_id ;
				}
			}
		}
		if ( enable == 0 ) continue ;

		GetParallelViewport( spot );

      BP_RB_PushMarker("Shadow Mask");

		/* 影マスク描画 */
		size = DG_WriteObjsPacketInit_MakeShadowMask( scrpad->dma_buffer, cp, spot->color );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_Projector_ShadowStage_InitPacket* pBPData = (SBP_Projector_ShadowStage_InitPacket*)BP_RB_Alloc(sizeof(SBP_Projector_ShadowStage_InitPacket));
         pBPData->eye_pers = scrpad->light_pers;
         BP_RB_AddCommand(kCmd_Projector_ShadowStage_InitPacket, (char*)pBPData);
      }

		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			//if ( list->shadow_id != shadow_id ) continue ;
			if ( ( list->shadow_id != shadow_id ) && ( shadow_id != 0 ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
		}

      BP_RB_PopMarker();

		/* ライトの描画 */
		size = DG_WriteObjsPacketInit_MakeSpot( scrpad->dma_buffer, cp, spot->color, DG_SpotLightBaseTexture, 1 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         BP_RB_AddCommand(kCmd_Projector_ShadowStage_PostZScreenPacket, NULL);
      }

      BP_RB_PushMarker("Shadow creation");
		//debug_count = 0 ;
		/* 影の生成 */
#if 0
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			//if ( list->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( list->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( !( list->group_id & gid ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) == 2 ) continue ;
			while ( objs != NULL ){
				//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
				MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
				objs = objs->next ;				/* 関連付けされた他のオブジェクトも処理 */
				//debug_count++ ;
				scrpad->bound_mode = 1 ;
			}
		}
#else
		if ( ( objs = spot->objs ) != NULL )
      {
			if ( objs->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) )
         {
				scrpad->root = objs->world ;
				if ( objs->flag & DG_FLAG_ONEPIECE ){
					scrpad->root.m[3][0] += objs->def->tx ;
					scrpad->root.m[3][1] += objs->def->ty ;
					scrpad->root.m[3][2] += objs->def->tz ;
				}
				_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
				if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) != 2 )
            {
					while ( objs != NULL ){
						//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
						MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
						objs = objs->next_dgobjs ;				/* 関連付けされた他のオブジェクトも処理 */
						//debug_count++ ;
						scrpad->bound_mode = 1 ;
					}
				}
			}
		}
#endif

		/* 影の描画 */
		scrpad->raise_eye_pers = cp->raise_eye_pers ;
		scrpad->raise_eye_pers2 = cp->raise_eye_pers2 ;
		size = DG_WriteObjsPacketInit_WriteShadow( scrpad->dma_buffer, cp, scrpad->fog_param1, scrpad->fog_param2 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
      {
         SBP_Projector_ShadowStage_ShadowProjStart* pBPData = (SBP_Projector_ShadowStage_ShadowProjStart*)BP_RB_Alloc(sizeof(SBP_Projector_ShadowStage_ShadowProjStart));
         pBPData->eye_pers = cp->raise_eye_pers;
         BP_RB_AddCommand(kCmd_Projector_ShadowStage_ShadowProjStart, (char*)pBPData);
      }

      BP_RB_PopMarker();

      BP_RB_PushMarker("Apply Shadow");

		if ( 1 == 0 ){
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Spot_TransTexture ;
		} else {
			scrpad->dma_packet[0].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
			scrpad->dma_packet[1].tex_packet.tex0_trans.addr = &Shadow_TransTexture ;
		}
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( list->shadow_id != shadow_id ) && ( shadow_id != 0 ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			//_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			//if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			scrpad->bound_mode = objs->bound_mode ;
			WriteShadowChainObjs( list->objs );
		}

      BP_RB_PopMarker();
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

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

   {
      BP_RB_AddCommand(kCmd_Projector_ShadowStage_EndPacket, NULL);
   }
   BP_RB_PopMarker();
}
#endif

void DG_NewShadowChainSetup()
{
#if BP_VITA
   {
      // save the current pointer so we can stitch this list into the main one after we're done, nspots is so we know how large
      // the shadows can be
      SBP_ProjectorNew_StartPacket *pPacket = (SBP_ProjectorNew_StartPacket*)BP_RB_Alloc(sizeof(SBP_ProjectorNew_StartPacket));
      pPacket->me = (SRenderCommand*)BP_RB_GetCurrentPtr();
      pPacket->head = pPacket->tail = NULL;
      pPacket->nspots = 0;
      BP_RB_AddCommand(kCmd_ProjectorNew_StartPacket, pPacket);
      BP_ShadowList = pPacket;
   }
#endif
}
