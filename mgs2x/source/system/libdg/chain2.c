//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	chain2.c
	チャンネル処理ユニット／マルチテクスチャオブジェクトＤＭＡ接続ルーチン

	2000/02/21 K.Takabe
	$Id: chain2.c,v 1.1.1.3 2002/11/19 11:42:03 Yoshizawa1 Exp $

*/
/*

	void		DG_ChainChanl2( cp, which )
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

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_RenderObj.h"
#include "BP_Debug.h"

#define _CopyVector( a, b ) { *(u_long128*)a = *(u_long128*)b ; }
#define _CopyMatrix( a, b ) { 0[(u_long128*)a] = 0[(u_long128*)b] ;\
								1[(u_long128*)a] = 1[(u_long128*)b] ;\
								2[(u_long128*)a] = 2[(u_long128*)b] ;\
								3[(u_long128*)a] = 3[(u_long128*)b] ; }

#define GS_REGS_1(r0) \
((r0) << 0x00)
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)

/*----------------------------------------------------------------*/

/* マイクロプログラム読み込みタグ */
#if 0 //BP_PS2
extern qword Vu1DrawObject0 ;
extern qword Vu1DrawObject1 ;
extern qword Vu1DrawObject2 ;
extern qword Vu1DrawObject3 ;
#define SELECT_VU1_PROGRAM Vu1DrawObject3
/* マイクロプログラム実行アドレスリスト */
extern int Vu1DrawObject0_Func[] ;
extern int Vu1DrawObject1_Func[] ;
extern int Vu1DrawObject2_Func[] ;
extern int Vu1DrawObject3_Func[] ;
#define SELECT_VU1_PROGRAM_FUNC Vu1DrawObject3_Func
#endif
/*----------------------------------------------------------------*/
	/*
		各種ローカル構造体定義
	*/

/* １パケット分のＤＭＡ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _pack_dma_packet {
	DG_DMATAG		verts_trans ;		/* 頂点転送ＤＭＡタグ */
	DG_DMATAG		norms_trans ;		/* 法線転送ＤＭＡタグ */
	DG_DMATAG		uvs0_trans ;		/* ＵＶ０転送ＤＭＡタグ */
	DG_DMATAG		uvs1_trans ;		/* ＵＶ１転送ＤＭＡタグ */
	DG_DMATAG		uvs2_trans ;		/* ＵＶ２転送ＤＭＡタグ */
	DG_DMATAG		rgb_trans ;			/* 頂点カラー転送ＤＭＡタグ */
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
		int			bound ;
		DG_GIFTAG	giftag_normal ;	/* 通常描画用描画ＧＩＦタグ */
		DG_GIFTAG	giftag_clip ;	/* クリップポリゴン用描画ＧＩＦタグ */
		IVECTOR		gif_regslist ;	/* マルチテクスチャ用レジスタリスト */
	} datas ;
} MATRIX_STACK ;

/* テクスチャ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _tex_dma_packet {
	DG_DMATAG		tex0_trans ;		/* テクスチャ０転送ＤＭＡパケット */
	DG_DMATAG		tex1_trans ;		/* テクスチャ１転送ＤＭＡパケット */
	DG_DMATAG		tex2_trans ;		/* テクスチャ２転送ＤＭＡパケット */
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
	/*
		スクラッチパッドワーク定義
	*/

/* 通常モデル描画用ワーク */
typedef struct {
	DG_OBJS		*objs ;
	int			use_tri ;
} OBJ_LIST ;

/* ベーススクラッチパッドワーク定義 */
typedef struct chain2_ScrpadWork {
	/* ＤＭＡ生成用バッファ */
	MATRIX_STACK	matrix_stack ;		/* マトリクススタック */
	PACKET_BUFFER	pre_packet[2] ;		/* ＤＭＡパケット */
	u_long128		dma_buffer[128] ;	/* 汎用ＤＭＡパケット生成バッファ */
	/* オブジェクト状態管理用 */
	FMATRIX		screen ;
	FMATRIX		local_light ;
	FMATRIX		local_color ;
	FMATRIX		connection ;
	FMATRIX		option_mat ;
	int			fog ;
	float		fog_param1, fog_param2 ;
	short		vu_prog_list[16] ;
	int			debug_count ;
	/* 各種ワーク */
	FMATRIX		eye_inv ;
	FMATRIX		eye_pers ;
	int			max_num ;
	void		*now ;
	int			invisible_flag ;
	int			buffer_switch ;
	/* StoreMatrixObjs()関数用ローカル変数 */
	u_long128	*matrix_store_addr ;
	/* ChainObj()関数固有ローカル変数 */
	void		*matrix_addr ;

#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	model[ 2 ] ;
	int			verts_count ;
	int			pack_count ;
#endif

	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
extern char	*DG_AS_GetLocalMemoryAddress();

/*----------------------------------------------------------------*/

static inline PACKET_BUFFER* NEXT_BUFF(ScrpadWork	*scrpad)	{ scrpad->buffer_switch = 1 - scrpad->buffer_switch ; return (&scrpad->pre_packet[ scrpad->buffer_switch ] ) ;}

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

static inline void _debug_print_matrix( char *str, FMATRIX *mat )
{
	int		i ;
	printf("%s\n", str);
	for ( i = 0 ; i < 4 ; i++ ){
		printf("%10f %10f %10f %10f\n", mat->m[i][0], mat->m[i][1], mat->m[i][2], mat->m[i][3] );
	}
}

#if 0
static inline void Vu0CopyMatrix( FMATRIX *dst, FMATRIX *src )
{
	asm ("
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
	asm ("\
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
#if 0 //BP_ASM - not used
static inline void SetLightMatrix( FMATRIX *res_light, FMATRIX *light, FMATRIX *world )
{
   /* 書き込み先はスクラッチパッドでなければならない */
	asm volatile ("
		lqc2		vf4,0x00(%2)
		lqc2		vf5,0x10(%2)
		lqc2		vf6,0x20(%2)
		lqc2		vf7,0x30(%2)
		li			$7,4
0:
		lqc2		vf8,0x0(%1)
		vmulax.xyz		ACC,vf4,vf8
		vmadday.xyz		ACC,vf5,vf8
		vmaddaz.xyz		ACC,vf6,vf8
		vmaddw.xyz		vf12,vf7,vf8
		sqc2		vf12,0x0(%0)
		addi		$7,-1
		addi		%1,0x10
		addi		%0,0x10
		bne			$0,$7,0b
	":"+r"(res_light),"+r"(world):"r"(light):"$7");
}
#endif

/*----------------------------------------------------------------*/
	/*
		パケット初期化関連
	*/

/* デフォルトパケット初期化ルーチン */
static void SetupDmaPacket( PACKET_BUFFER *buff )
{
	static PACK_DMA_PACKET	def_pack_packet = {
#if 0 //BP_PS2
		.verts_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x100, 0, VIF_VERT_PACK, 0 )}},
		.norms_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x101, 0, VIF_NORM_PACK, 0 )}},
		.uvs0_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x102, 0, VIF_UV_PACK, 0 )}},
		.uvs1_trans = { DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_NOP( 0 )}},
		.uvs2_trans = { DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_NOP( 0 )}},
		.rgb_trans = { DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 6, 0 ),
			SCE_VIF1_SET_NOP( 0 )}},
		.trans_tag = { DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _pack_dma_datas) + 1 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x14, SIZEOF_QWORD(struct _pack_dma_datas), VIF_DATA128, 0 ) }},
#if 0
		.datas = {
			.giftag_normal = { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 0, 0, 0),
											   SCE_GIF_PACKED, 5),
								 GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) },
			.giftag_clip = { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0),
											 SCE_GIF_PACKED, 5),
							   GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) },
			.gif_regslist = {GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_NOP, GS_REGS_STQ, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2)},
		},
#endif
		.prog_exec = {{SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 )}},
#else
      0
#endif
	} ;

   BP_RENDER_TODO_BREAK; //BP_GCC TODO above

	buff->pack_packet = def_pack_packet ;

	buff->tex_packet.tex0_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	buff->tex_packet.tex0_trans.addr = NULL ;
	buff->tex_packet.tex0_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->tex_packet.tex0_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xe8, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
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

	buff->mat_trans_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(struct _stack_datas) );
	buff->mat_trans_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->mat_trans_tag.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x00, SIZEOF_QWORD(struct _stack_datas), VIF_DATA128, 0 );

}

static void SetupMatrixStack( MATRIX_STACK *mat_stack )
{
#if 1
	static DG_GIFTAG	giftag_normal = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 5),
		  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_clip = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 5),
		  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) };
#else
	static DG_GIFTAG	giftag_normal = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 0, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 5),
		  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_clip = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 0, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 5),
		  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) };
#endif
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

static inline u_long128* SetVertsTrans( DG_DMATAG *tag, int vu_oft, int n_verts, int v_oft, u_long128 *verts, int type )
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

static inline void SetTexture( DG_DMATAG *tag, DG_TEX_TRANS *tex_trans, int offset )
{
	if ( ( tex_trans != NULL ) && ( tex_trans != tag->addr ) ){
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) ) ;
		tag->addr = tex_trans ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( offset, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
	} else {
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
		tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	}
}

	/*
		ＤＭＡに接続する
	*/

#if 0 //BP_PS2
static	void	ChainObj( DG_OBJ *obj )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int			j ;
	int			first_flag, prog_no, bound ;
	u_long128	*verts, *norms, *uvs0, *uvs1, *uvs2, *rgbs ;/* スクラッチパッドに入れた方がいいかも */
	PACKET_BUFFER	*buff ;

	/* ＤＭＡパケット生成準備 */
	first_flag = 2 ;
	pack = obj->packets ;
	bound = obj->bound_mode ;
	verts = (u_long128*)obj->verts ;
	norms = (u_long128*)obj->norms ;
	uvs0 = (u_long128*)obj->uvs[0] ;
	uvs1 = (u_long128*)obj->uvs[1] ;
	uvs2 = (u_long128*)obj->uvs[2] ;
	rgbs = (u_long128*)obj->rgbs ;
	GV_PREFECH( pack );
#ifdef LIBDG_PERFORMANCE
	scrpad->pack_count += obj->n_packs ;
#endif
	for ( j = obj->n_packs ; j > 0 ; j-- ){

		buff = NEXT_BUFF(scrpad);	/* 次のＤＭＡパケットバッファ取得 */

		/* パケット生成 */
		verts = SetVertsTrans( &buff->pack_packet.verts_trans,
							  0x100, pack->verts_offset*2, pack->verts_offset, verts, VIF_VERT_PACK );
		norms = SetVertsTrans( &buff->pack_packet.norms_trans,
							  0x101, pack->norms_offset*2, pack->norms_offset, norms, VIF_NORM_PACK );
		uvs0 = SetVertsTrans( &buff->pack_packet.uvs0_trans,
							 0x102, pack->uvs_offset[0]*4, pack->uvs_offset[0], uvs0, VIF_UV_PACK );
		uvs1 = SetVertsTrans( &buff->pack_packet.uvs1_trans,
							 0x103, pack->uvs_offset[1]*4, pack->uvs_offset[1], uvs1, VIF_UV_PACK );
		uvs2 = SetVertsTrans( &buff->pack_packet.uvs2_trans,
							 0x104, pack->uvs_offset[2]*4, pack->uvs_offset[2], uvs2, VIF_UV_PACK );
		rgbs = SetVertsTrans( &buff->pack_packet.rgb_trans,
							 0x105, pack->rgbs_offset*2, pack->rgbs_offset, rgbs, VIF_VERT_PACK );
		SetTexture( &buff->tex_packet.tex0_trans, pack->tex_ptr[0], 0xe8 );
		SetTexture( &buff->tex_packet.tex1_trans, pack->tex_ptr[1], 0xf0 );
		SetTexture( &buff->tex_packet.tex2_trans, pack->tex_ptr[2], 0xf8 );
		buff->pack_packet.datas.n_verts = pack->n_verts ;
		buff->pack_packet.datas.flag = pack->flag ;
#ifdef LIBDG_PERFORMANCE
		scrpad->verts_count += pack->n_verts ;
#endif
#ifdef DEBUG_MODE
#if 0
		if ( pack->n_verts > 32 ){
			pack++ ;
			continue ;
		}
#else
		ASSERT( pack->n_verts <= 32 );
#endif
#endif

		{/* 実行するＶＵ１プログラムを決定 */
			/* バウンディングチェック */
			if ( pack->flag & DG_PACKFLAG_ENVELOPE )	prog_no = 1 ;
			else										prog_no = 0 ;
			/* 頂点カラー付きオブジェクトチェック */
			if ( rgbs != NULL ) prog_no += 2 ;
		}
		GV_PREFECH( pack + 1 );
		buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_MSCAL( scrpad->vu_prog_list[prog_no], 0 ) ;

		/* パケットをメモリへ転送 */
		if ( first_flag > 0 ){/* 最初の２回のみ */
			buff->mat_trans_tag.addr = scrpad->matrix_addr ;
			buff->pack_packet.datas.fog = scrpad->fog ;
			buff->pack_packet.datas.bound = bound & 1 ;
			DG_SendCacheFIFO( scrpad->local_work, &buff->mat_trans_tag, 
						  SIZEOF_QWORD(DG_DMATAG) + SIZEOF_QWORD(PACK_DMA_PACKET) + SIZEOF_QWORD(TEX_DMA_PACKET) );
		} else {
			DG_SendCacheFIFO( scrpad->local_work, &buff->tex_packet,
						  SIZEOF_QWORD(PACK_DMA_PACKET) + SIZEOF_QWORD(TEX_DMA_PACKET) );
		}

		first_flag-- ;
		pack++ ;
	}
}
#else

static	void	ChainObj( DG_CHANL *cp, DG_OBJ *obj )
{
   ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;

   DG_OBJS* objs = (DG_OBJS*)obj->BP_DGObjs;
   KP_MatrixData* pMatrixData = (KP_MatrixData*)obj->matrix_addr;

   if( pMatrixData )
   {
      int packetIdx;

      unsigned int lastTex[3] = { -1, -1, -1 };
      int lastFlag = -1;

      DG_OBJ_PACKET* packet = obj->packets;

      SBP_OBJ_Render* pLastRenderCmd = NULL;

      SBP_OBJ_PreRender *pPreRenderCmd;

      // Set up the PreRender
      pPreRenderCmd = BP_RB_Alloc( sizeof( SBP_OBJ_PreRender ) );
      pPreRenderCmd->eye_pers = obj->screen;
      _sceVu0MulMatrix( &pPreRenderCmd->eye_inv, &cp->eye_inv, &obj->world );

//      pPreRenderCmd->eye_inv = pMatrixData->option;
      pPreRenderCmd->correction = obj->inv_mat;
      pPreRenderCmd->lightDir = pMatrixData->local_light;
      pPreRenderCmd->lightCol = pMatrixData->local_color;

      BP_RB_AddCommand( kCmd_Obj_PreRender, pPreRenderCmd );

      // Set up the PreRender
      pPreRenderCmd = BP_RB_Alloc( sizeof( SBP_OBJ_PreRender ) );

      if(obj->BP_Model)
         BP_Obj_Render_ComputeAnimatedBuffers(objs, obj->BP_Model);

      // Queue BP render packet
      for ( packetIdx = 0; packetIdx < obj->n_packs; ++packetIdx, ++packet  )
      {
         unsigned int const curTex[3] = { (unsigned int)packet->tex_ptr[0], (unsigned int)packet->tex_ptr[1], (unsigned int)packet->tex_ptr[2] };
         int const curFlag = packet->flag;

         int isMultiPass = (curTex[1] | curTex[2]);
         int const textureDiff = (curTex[0] ^ lastTex[0]) | (curTex[1] ^ lastTex[1]) | (curTex[2] ^ lastTex[2]);
         int flagsDiff = curFlag ^ lastFlag;
         if( packetIdx != 0 && flagsDiff && !(flagsDiff & ~DG_PACKFLAG_ENVELOPE) )
         {
            flagsDiff = 0;
         }

         if( textureDiff | flagsDiff )
         {
            lastTex[0] = curTex[0];
            lastTex[1] = curTex[1];
            lastTex[2] = curTex[2];
            lastFlag = curFlag;

            pLastRenderCmd = (SBP_OBJ_Render*)BP_RB_Alloc(sizeof(SBP_OBJ_Render));
            pLastRenderCmd->allocSize = sizeof( SBP_OBJ_Render );
            pLastRenderCmd->obj_flag = obj->flag;
            pLastRenderCmd->flag = curFlag;

            pLastRenderCmd->model = obj->BP_Model;
            pLastRenderCmd->unit = obj->BP_ObjIndex;
            
            BP_RB_CopyTexture(&pLastRenderCmd->tex[0], curTex[0]);
            BP_RB_CopyTexture(&pLastRenderCmd->tex[1], curTex[1]);
            BP_RB_CopyTexture(&pLastRenderCmd->tex[2], curTex[2]);
            pLastRenderCmd->startPacket = packetIdx;
            pLastRenderCmd->packetCount = 1;
            pLastRenderCmd->builtForIR = 0;
            pLastRenderCmd->isIRMode = 0;
            pLastRenderCmd->BP_animatedVertexBuffer = objs->BP_VAnimBuffer;
#if BP_VITA
            pLastRenderCmd->BP_precomp = NULL;
#endif
            if ( !( obj->flag & DG_FLAG_PAINT ) )
            {
               if ( ( DG_DisplayStatus & DG_STATE_IR_MODE ) && ( obj->flag & DG_FLAG_IRREACTION ) )
               {
                  isMultiPass = 0;
                  pLastRenderCmd->isIRMode = 1;
               }
            }

#if TRACK_RENDEROBJ_STATS
            if( packetIdx != 0 )
            {
               pLastRenderCmd->count = 1;
               pLastRenderCmd->texDiffCount =  textureDiff ? 1 : 0;
               pLastRenderCmd->flagDiffCount = flagsDiff ? 1 : 0;
            }
            else
            {
               pLastRenderCmd->count = 0;
               pLastRenderCmd->texDiffCount =  0;
               pLastRenderCmd->flagDiffCount = 0;
            }
#endif
            if( isMultiPass )
            {
//               pPreRenderCmd->BP_Flags |= kPRF_MultiPass;
               BP_RB_AddCommand(kCmd_Obj_RenderMulti, (char*)pLastRenderCmd);
            }
            else
            {
               BP_RB_AddCommand(kCmd_Obj_Render, (char*)pLastRenderCmd);
            }
         }
         else
         {
            ++pLastRenderCmd->packetCount;
         }
      }
   }
}

#endif

/*----------------------------------------------------------------*/
	/*
		ＤＭＡに接続する
	*/
static	void	ChainObjs( DG_CHANL *cp, DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;

   // Queue BP local param packet
   {
      SBP_OBJ_LocalParam* pBPData = (SBP_OBJ_LocalParam*)BP_RB_Alloc(sizeof(SBP_OBJ_LocalParam));
//      pBPData->model = (unsigned int)objs->BP_Model;
      pBPData->preshadeBuffer = objs->BP_PreshadeBuffer;
      pBPData->dgObjs = (unsigned int)objs;
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

      BP_RB_AddCommand(kCmd_Obj_LocalParam, (char*)pBPData);
   }

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->mdl_type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & scrpad->invisible_flag ) continue ;

#ifdef LIBDG_PERFORMANCE
		scrpad->verts_count = 0 ;
		scrpad->pack_count = 0 ;
#endif
		scrpad->fog = obj->fog ;
		scrpad->matrix_addr = obj->matrix_addr ;
		ChainObj( cp, obj );
#ifdef LIBDG_PERFORMANCE
		scrpad->model[ obj->bound_mode ].n_obj++ ;
		scrpad->model[ obj->bound_mode ].n_packs += scrpad->pack_count ;
		scrpad->model[ obj->bound_mode ].n_verts += scrpad->verts_count ;
#endif
	}

}

/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
void _DG_Chain2Chanl( DG_CHANL *cp, int which, int draw_mode )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, j, size, old_tex_code = -1, max_objs, max_mem_objs ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		c_gid, gid ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int			last_semi_trans = 0 ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	MARK( "chain2.c" );
	obj_buff = &que->objs_buffer ;
#if 0
	scePcStart( (SCE_PC_CTE|SCE_PC_U0|SCE_PC_U1)|
			   SCE_PC0_CPU_CYCLE|
			   //SCE_PC0_ICACHE_MISS|
			   //SCE_PC0_SINGLE_ISSUE|
			   //SCE_PC1_DUAL_ISSUE|
			   //SCE_PC1_INST_COMP|
			   //SCE_PC0_COP2_COMP|
			   //SCE_PC1_COP1_COMP|
			   SCE_PC0_LOAD_COMP|
			   //SCE_PC1_STORE_COMP|
			   //SCE_PC0_ADDR_BUS_BUSY|
			   //SCE_PC1_DATA_BUS_BUSY|
			   0,
			   0, 0 );
#endif

	c_gid = cp->group_id ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
#if 0 //BP_PS2
	for ( i = 0 ; i < 16 ; i++ ){
		scrpad->vu_prog_list[i] = SELECT_VU1_PROGRAM_FUNC[i] ;
	}
#endif
	/* スクラッチパッド上のパケットバッファを初期化 */
	SetupMatrixStack( &scrpad->matrix_stack );
#if 0 //BP_PS2
	SetupDmaPacket( &scrpad->pre_packet[0] );
	SetupDmaPacket( &scrpad->pre_packet[1] );
#endif

	scrpad->eye_inv = cp->eye_inv ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
#ifdef LIBDG_PERFORMANCE
	scrpad->model[0] = DG_PerformanceData.multex_model[0] ;
	scrpad->model[1] = DG_PerformanceData.multex_model[1] ;
#endif
	scrpad->debug_count = 0 ;

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		if ( !( objs->flag & DG_FLAG_MULTITEX ) ) continue ;
		if ( objs->flag & DG_FLAG_PAINT ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) continue ;
		/* 描画フェーズコントロール */
		if ( draw_mode == 0 ){
			if ( objs->flag & DG_FLAG_LATTERDRAW ) continue ;
         if ( objs->flag & DG_FLAG_FORCEMSAA ) continue;
		} else if ( draw_mode == 1 ) {
			if ( !( objs->flag & DG_FLAG_LATTERDRAW ) ) continue ;
         if ( objs->flag & DG_FLAG_FORCEMSAA ) continue;
		} else if ( draw_mode == 2 ) {
         if ( !( objs->flag & DG_FLAG_FORCEMSAA ) ) continue ;
      }

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & c_gid ) ) continue ;

		//if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWMAKE|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
#if BP_ENABLE_TESTNODE
      if ( !BP_Debug_EvalTestNode() ) continue;
#endif
		obj_list[max_objs].objs = objs ;
		obj_list[max_objs].use_tri = objs->tri_id ;
		max_objs++ ;
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
			if ( tex_list->code != obj_list[i].use_tri ) continue ;
			mem_obj_list->objs = obj_list[i].objs ;
			mem_obj_list->use_tri = (int)tex_list ;	/* tri_codeの替わりにDG_TEXTUER_LISTへのポインタを入れる */
			mem_obj_list++ ;
			max_mem_objs++ ;
		}
	}
	if ( max_mem_objs == 0 ) return ;

	FlushCache( 0 );

#if 0	/* screen.cに移動 */
	/* オブジェクト描画用マトリクスデータをパケット展開メモリに転送する */
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
	scrpad->matrix_store_addr = DG_CurrentDmaAddr ;
	for ( i = max_mem_objs ; i > 0 ; i-- ){
		/* マトリクスデータをパケットメモリにストアする */
		StoreMatrixObjs( mem_obj_list->objs );
		mem_obj_list++ ;
	}
	DG_EndSprToMem();
#endif

   //if( !(cp->flag & DG_VIEWPORT_STAGE_DISABLE_MSAA) )
   if( 1 )
   {
      BP_GetCurrentViewportInfo()->hasMSAAStuff = 1;
   }
   else
   {
      BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   }
   BP_RB_PushRegionMarker(kProfileColor_FixModel_After, "Chain2");
   BP_Debug_PushCPUMarker( "Chain2" );

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* 初期化パケットの設定 */
	size = DG_WriteObjsPacketInit( scrpad->dma_buffer, cp, 1 );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	/* メモリに書き出したソート完了済みオブジェクトをテクスチャ転送を挟みながら描画する */
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
	//tex_list = DG_TextureList ;
	old_tex_code = -1 ;
	for ( i = max_mem_objs ; i > 0 ; i-- ){
		/* テクスチャのチェック */
		if ( mem_obj_list->use_tri != old_tex_code ){
			tex_list = (DG_TEXTURE_LIST*)mem_obj_list->use_tri ;
#ifdef LIBDG_PERFORMANCE
			DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
			//size = DG_WriteTextureChangePacks( scrpad->dma_buffer, &tex_list->tex_packet[which] );
			size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
			DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			old_tex_code = mem_obj_list->use_tri ;
			tex_list++ ;
		}
		/* 強制半透明描画チェック */
      if ( mem_obj_list->objs->flag & DG_FLAG_FORCEMSAA ) {
         if (!cp->chanl_num)
            BP_RB_AddCommand(kCmd_MSAA_Particles, NULL);
      }
		if ( mem_obj_list->objs->flag & DG_FLAG_SEMITRANS ){
			if ( last_semi_trans == 0 ){
				size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 1 );
				DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			}
			last_semi_trans = 1 ;
		} else {
			if ( last_semi_trans != 0 ){
				size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
				DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			}
			last_semi_trans = 0 ;
		}
		ChainObjs( cp, mem_obj_list->objs );
		mem_obj_list++ ;
	}

	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}

	/* 終了パケットの設定 */
	size = DG_WriteObjsPacketEnd( scrpad->dma_buffer );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();

   BP_RB_PopRegionMarker();
   BP_Debug_PopCPUMarker();

#if 0
	scePcStop();
	if ( GV_PadData[1].press & PAD_A ){
		int		t0, t1 ;
		t0 = scePcGetCounter0();
		t1 = scePcGetCounter1();
		printf("chain2.c %d %d\n", t0, t1);
	}
#endif
#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.multex_model[0] = scrpad->model[0] ;
	DG_PerformanceData.multex_model[1] = scrpad->model[1] ;
#endif
}

/* ---------------------------------------------------------------- */
void DG_Chain2Chanl( DG_CHANL *cp, int which )
{
	_DG_Chain2Chanl( cp, which, 0 );
}

void DG_Chain2ChanlLatter( DG_CHANL *cp, int which )
{
	_DG_Chain2Chanl( cp, which, 1 );
}

void DG_Chain2ChanlLast( DG_CHANL *cp, int which )
{
	_DG_Chain2Chanl( cp, which, 2 );
}
