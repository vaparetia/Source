//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
#include "BP_Memory.h"
//BP
//----------------------------------------------------------------------------
/*
	chain.c
	チャンネル処理ユニット／オブジェクトＤＭＡ接続ルーチン

	1999/07/07 K.Takabe
	$Id: chain.c,v 1.1.1.3 2002/11/19 11:42:02 Yoshizawa1 Exp $

*/
/*

	void		DG_ChainChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する


	void		DG_SortChainChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをソートしてＤＭＡパケットに接続する
	同時にソート済みのプリミティブオブジェクトも表示する

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

//#include	"break.h"
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

//#define DG_EndSprToMem() UTL_EndSprToMem()
//#define DG_StartSprToMem( a, b, c ) UTL_StartSprToMem( a, b, c ) 


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

#ifndef MGS_MASTER
//#define	MARK(n)		( *(volatile char**)&GV_DebugMes = (char*)n )
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
#if 0 //BP_PS2
extern qword Vu1DrawObject0 ;
extern qword Vu1DrawObject1 ;
extern qword Vu1DrawObject2 ;
#define SELECT_VU1_PROGRAM Vu1DrawObject2
/* マイクロプログラム実行アドレスリスト */
extern int Vu1DrawObject0_Func[] ;
extern int Vu1DrawObject1_Func[] ;
extern int Vu1DrawObject2_Func[] ;
#define SELECT_VU1_PROGRAM_FUNC Vu1DrawObject2_Func
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
/* ソート用ワーク */
typedef struct {
	void		*tag ;
	u_int		z ;
	void		*obj ;
} SORT_TAG ;
typedef struct {
	SORT_TAG	list[768] ;
	void		*ot3[64] ;
	void		*ot2[64] ;
	void		*ot1[64] ;
} SORT_WORK ;
/* 通常モデル描画用ワーク */
typedef struct {
	DG_OBJS		*objs ;
	int			use_tri ;
} OBJ_LIST ;
/* オブジェクトマトリクスストア用ワーク */
typedef struct {
	FMATRIX			world ;
	FMATRIX			screen ;
	FMATRIX			inv_mat ;
} OBJ_MATRIX ;
typedef struct {
	u_long128		*current_matrix_addr ;
	int				pad[3] ;	
	OBJ_MATRIX		buffer[2][8] ;
	u_long128		local_work[0] ;
} MATRIXSTORE_WORK ;


/* ベーススクラッチパッドワーク定義 */
typedef struct {
	/* ＤＭＡ生成用バッファ */
	MATRIX_STACK	matrix_stack ;		/* マトリクススタック */
	PACKET_BUFFER	pre_packet[2] ;		/* ＤＭＡパケット */
	u_long128		dma_buffer[128] ;	/* 汎用ＤＭＡパケット生成バッファ */
	/* オブジェクト状態管理用 */
	FMATRIX		screen ;
	FMATRIX		local_light ;
	FMATRIX		local_color ;
	FMATRIX		connection ;
	int			fog ;
	float		fog_param1, fog_param2 ;
	short		vu_prog_list[16] ;
	FMATRIX		eye_pers ;
	FMATRIX		eye_inv ;
	/* 各種ワーク */
	int			max_num ;
	void		*now ;
	int			invisible_flag ;
	int			buffer_switch ;
	/* StoreMatrixObjs()関数用ローカル変数 */
	u_long128	*matrix_store_addr ;
	/* ChainObj()関数用ローカル変数 */
	void		*matrix_addr ;

	/* デバッグ用 */
	DG_OBJS		*current_objs ;
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	model[ 2 ] ;
	int			verts_count ;
	int			pack_count ;
#endif
	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

static inline PACKET_BUFFER* NEXT_BUFF(ScrpadWork	*scrpad)	{ scrpad->buffer_switch = 1 - scrpad->buffer_switch ; return (&scrpad->pre_packet[ scrpad->buffer_switch ] ) ;}

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
#define LOCAL_MEMORY_SIZE (16*1024)

unsigned char *DG_AS_GetLocalMemoryAddress()
{
   return GV_AS_GetDGLocalWork();
}


/*----------------------------------------------------------------*/

static inline void Vu0CopyMatrix( FMATRIX *dst, FMATRIX *src )
{
#if 0 //BP_ASM
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
#endif
}

/*----------------------------------------------------------------*/
#if 0 //BP_ASM - unused
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
								 GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) },
			.giftag_clip = { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0),
											 SCE_GIF_PACKED, 3),
							   GS_REGS_3( GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) },
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

	buff->pack_packet = def_pack_packet ;

	buff->tex_packet.tex0_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	buff->tex_packet.tex0_trans.addr = NULL ;
	buff->tex_packet.tex0_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->tex_packet.tex0_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xe8, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;

	buff->mat_trans_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(struct _stack_datas) );
	buff->mat_trans_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->mat_trans_tag.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x00, SIZEOF_QWORD(struct _stack_datas), VIF_DATA128, 0 );

}

static void SetupMatrixStack( MATRIX_STACK *mat_stack )
{
#if 1
	static DG_GIFTAG	giftag_normal = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_clip = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 0, 0, 0),SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
#else
	static DG_GIFTAG	giftag_normal = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 0, 1, 1, 0, 0, 0, 0), SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
	static DG_GIFTAG	giftag_clip = 
	  { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 0, 1, 1, 0, 0, 0, 0),SCE_GIF_PACKED, 3),
		  GS_REGS_3(GS_REGS_STQ,GS_REGS_RGBA,GS_REGS_XYZF2) } ;
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

static void BP_ChainObjectLocalParam(DG_OBJS* objs)
{
   SBP_OBJ_LocalParam* pBPData = (SBP_OBJ_LocalParam*)BP_RB_Alloc(sizeof(SBP_OBJ_LocalParam));
//   pBPData->model = (unsigned int)objs->BP_Model;
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
	int			first_flag, prog_no, bound, obj_flag ;
	u_long128	*verts, *norms, *uvs0 ;/* スクラッチパッドに入れた方がいいかも */
	PACKET_BUFFER	*buff ;

	obj_flag = obj->flag ;

	/* ＤＭＡパケット生成準備 */
	first_flag = 2 ;
	pack = obj->packets ;
	bound = obj->bound_mode ;
	verts = (u_long128*)obj->verts ;
	norms = (u_long128*)obj->norms ;
	uvs0 = (u_long128*)obj->uvs[0] ;
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
		buff->pack_packet.datas.n_verts = pack->n_verts ;
		buff->pack_packet.datas.flag = 0 ;
		buff->tex_packet.tex0_trans.addr = pack->tex_ptr[0] ;
		//SetTexture( &buff->tex_packet.tex0_trans, pack->tex_ptr[0], 0xe8 );
#ifdef LIBDG_PERFORMANCE
		scrpad->verts_count += pack->n_verts ;
#endif

		{/* 実行するＶＵ１プログラムを決定 */
			/* バウンディングチェック */
			if ( bound & 1 )		prog_no = 1 ;
			else					prog_no = 0 ;
			/* エンベロープチェック */
			if ( pack->flag & DG_PACKFLAG_ENVELOPE ) prog_no += 2 ;
			/* プリシェードチェック */
			if ( obj_flag & DG_FLAG_PAINT ){
				prog_no += 4 ;
				/* カリングフラグのチェック */
				if ( pack->flag & DG_PACKFLAG_CULLON ) prog_no += 2 ;
				else if ( pack->flag & DG_PACKFLAG_CULLAUTO ){
					if ( bound & 1 ) prog_no += 2 ;
				}
			}
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

#define OBJ_RENDER_DATASIZE ( sizeof(SBP_OBJ_Render) + sizeof( DG_TEX_BP ) )

static void BuildRenderCmds( DG_OBJ *obj, SBP_OBJ_Render *pFirstObject, int isIRMode, int additionalSize, int *pOutCount, int *pOutTotalSize )
{
   DG_OBJS* objs = (DG_OBJS*)obj->BP_DGObjs;
   int packetIdx;
   int returnCount = 0;
   int returnDataSize = 0;
   SBP_OBJ_Render *pNextObject = pFirstObject;

   unsigned int lastTex = 0xFFFFFFFF;
   int lastFlag = -1;
   DG_OBJ_PACKET* packet = obj->packets;
   SBP_OBJ_Render* pLastRenderCmd = NULL;

   // Queue BP render packet
   for ( packetIdx = 0; packetIdx < obj->n_packs; ++packetIdx, ++packet  )
   {
      unsigned int const curTex = (unsigned int)packet->tex_ptr[0];
      int const curFlag = packet->flag;

      int const textureDiff = curTex ^ lastTex;
      int flagsDiff = curFlag ^ lastFlag;
      if( packetIdx != 0 && flagsDiff && !(flagsDiff & ~DG_PACKFLAG_ENVELOPE) )
      {
         flagsDiff = 0;
      }

      // Arm fix:
      // Identify a particular model in a particular scene in objs.c
      // then here identify a unit (20) and packet (0) and ignore it
      // fixes MGSTWO-2893
      if (obj->flag & DG_FLAG_ARM_HIDE_PART)
      {
         if (obj->BP_ObjIndex == 20 && packetIdx == 0)
         {
            continue;
         }
      }
      // End Arm fix

      if( textureDiff | flagsDiff )
      {
         DG_TEX_BP *tex_storage;

         lastTex = curTex;
         lastFlag = curFlag;

         pLastRenderCmd = pNextObject;
         pNextObject = (SBP_OBJ_Render*) (( (char *) pNextObject ) + OBJ_RENDER_DATASIZE + additionalSize );

         pLastRenderCmd->allocSize = OBJ_RENDER_DATASIZE + additionalSize;
         returnDataSize += pLastRenderCmd->allocSize;
         
         tex_storage = (DG_TEX_BP *) ( pLastRenderCmd + 1 );
         pLastRenderCmd->obj_flag = obj->flag;
         pLastRenderCmd->flag = curFlag;

         pLastRenderCmd->model = obj->BP_Model;
         pLastRenderCmd->unit = obj->BP_ObjIndex;

         //            BP_RB_CopyTexture(&pLastRenderCmd->tex[0], curTex);
         BP_CopyDGTexToDGTexBP_Optional( &pLastRenderCmd->tex[0], tex_storage, (DG_TEX *) curTex );
         pLastRenderCmd->tex[1] = 0;
         pLastRenderCmd->tex[2] = 0;
         pLastRenderCmd->startPacket = packetIdx;
         pLastRenderCmd->packetCount = 1;
         pLastRenderCmd->builtForIR = isIRMode;
         pLastRenderCmd->isIRMode = 0;
         pLastRenderCmd->BP_animatedVertexBuffer = (void *) objs->BP_VAnimBuffer;
#if BP_VITA
         pLastRenderCmd->BP_precomp = NULL;
#endif
         if ( ( obj->flag & ( DG_FLAG_PAINT | DG_FLAG_IRREACTION ) ) == DG_FLAG_IRREACTION )
         {
            if ( isIRMode )
            {
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

         ++returnCount;
      }
      else
      {
         ++pLastRenderCmd->packetCount;
      }
   }

   *pOutTotalSize = returnDataSize;
   *pOutCount = returnCount;
}

#if 1

typedef struct _s_chain_outputcache
{
   int mCount;
   int mIRByteOffset;
} SChainOutputCache;

struct _SBP_OBJ_Render *DG_AS_PrecomputeChainBuffersObjs( DG_OBJS *objs, struct _SBP_OBJ_Render *pListHead )
{
   SBP_OBJ_Render *pCurrentHead = pListHead;

   if ( objs )
   {
      int i;

      for ( i = 0; i < objs->n_models; ++i )
      {
         pCurrentHead = DG_AS_PrecomputeChainBuffers( &objs->objs[ i ], pCurrentHead );
      }
   }

   return pCurrentHead;
}

void DG_AS_FreePrecomputedChainBuffersObjs( DG_OBJS *objs )
{
   if ( objs )
   {
      int i;

      for ( i = 0; i < objs->n_models; ++i )
      {
         DG_AS_FreePrecomputedChainBuffers( &objs->objs[ i ] );
      }
   }
}

static void FixupPrecomputedBuffer( SChainOutputCache const *pCacheDef, void *dst, void const *src )
{
   // Count is *2 because of the ir ones
   int count = pCacheDef->mCount * 2;
   int i;
   SBP_OBJ_Render const *pSrcPacket = (SBP_OBJ_Render const *) src;
   SBP_OBJ_Render *pDstPacket = (SBP_OBJ_Render *) dst;

   for ( i = 0; i < count; ++i )
   {
      // Fixup the tex[0] ptr
      if ( pSrcPacket->tex[0] )
      {
         pDstPacket->tex[0] = (DG_TEX_BP *) ( (char *) dst + 
            ( ( char const * ) pSrcPacket->tex[0] - ( char const *) src ) );
      }

      pSrcPacket = (SBP_OBJ_Render const *)( (char const *) pSrcPacket + pSrcPacket->allocSize );
      pDstPacket = (SBP_OBJ_Render *)( (char *) pDstPacket + pDstPacket->allocSize );
   }
}

#if BP_VITA

#  define DEBUG_VALIDATE_PRECOMP_LIST 0

static void precomplist_validate( SBP_OBJ_Render *pListHeader )
{
   SBP_OBJ_Render *pListFirst = pListHeader;
   
   ASSERT( pListHeader->BP_precomp->mpPrevStart == NULL );   
   // First let's traverse

   {
      SBP_OBJ_Render *pCurrent = pListFirst;

      while ( pCurrent )
      {
         ASSERT( pCurrent->BP_precomp->mIsStart );
         if ( pCurrent->BP_precomp->mpNextStart )
         {
            ASSERT( pCurrent == pCurrent->BP_precomp->mpNextStart->BP_precomp->mpPrevStart );
         }

         if ( pCurrent->BP_precomp->mpPrevStart )
         {
            ASSERT( pCurrent == pCurrent->BP_precomp->mpPrevStart->BP_precomp->mpNextStart );
         }

         pCurrent = pCurrent->BP_precomp->mpNextStart;
      }
   }

   // Now let's look at interior items
   {
      SBP_OBJ_Render *pCurrent = pListFirst;

      while ( pCurrent )
      {
         SBP_OBJ_Render *pInterior = pCurrent->BP_precomp->mpNextInterior;

         while ( pInterior )
         {
            ASSERT( pInterior->BP_precomp->mpPrevStart == pCurrent );
            ASSERT( pInterior->BP_precomp->mpNextStart == NULL );
            ASSERT( pInterior->BP_precomp->mIsStart == 0 );

            pInterior = pInterior->BP_precomp->mpNextInterior;
         }

         pCurrent = pCurrent->BP_precomp->mpNextStart;
      }
   }

}

static SBP_OBJ_Render *precomplist_find_compatible_obj( SBP_OBJ_Render *pListHeader, SBP_OBJ_Render const *pNewEntry )
{
   SBP_OBJ_Render *pListCnt = pListHeader;
   int lastFlag = pNewEntry->flag;

   for ( ;pListCnt; pListCnt = pListCnt->BP_precomp->mpNextStart )
   {
      int const curFlag = pListCnt->flag;
      int flagsDiff = curFlag ^ lastFlag;

      if ( 
         pListCnt->builtForIR != pNewEntry->builtForIR ||
         pListCnt->isIRMode != pNewEntry->isIRMode || 
         pListCnt->model != pNewEntry->model ||
         pListCnt->unit != pNewEntry->unit ||
         pListCnt->startPacket != pNewEntry->startPacket ||
         pListCnt->packetCount != pNewEntry->packetCount )
      {
         continue;
      }

      if( flagsDiff && !(flagsDiff & ~DG_PACKFLAG_ENVELOPE) )
      {
         flagsDiff = 0;
      }

      if ( !flagsDiff /* && pListCnt->BP_animatedVertexBuffer == pNewEntry->BP_animatedVertexBuffer */ )
      {
         // compare textures
         if ( !pListCnt->tex[0] )
         {
            ASSERT( !pNewEntry->tex[0] );

            // No textures on either, we're compatible
            return pListCnt;
         }

         if ( pListCnt->tex[0]->pDebugOrigSrc == pNewEntry->tex[0]->pDebugOrigSrc )
         {
            return pListCnt;
         }
      }
   }

   return NULL;
}

static SBP_OBJ_Render *precomplist_find_or_add( SBP_OBJ_Render *pListHead, SBP_OBJ_Render *pCurrentEntry )
{
   SBP_OBJ_PreComputedInfo *pComputedCurrentEntry = ( SBP_OBJ_PreComputedInfo *)( ( (char *) pCurrentEntry ) + pCurrentEntry->allocSize - sizeof( SBP_OBJ_PreComputedInfo ) );

   // First, let's add precomputed info for pCurrentEntry
   pCurrentEntry->BP_precomp = pComputedCurrentEntry;
   memset( pComputedCurrentEntry, 0, sizeof( *pComputedCurrentEntry ) );
   pCurrentEntry->BP_precomp->mLastFrameTouched[0] = -1;
   pCurrentEntry->BP_precomp->mLastFrameTouched[1] = -1;

   {
      SBP_OBJ_Render *pCompatible = precomplist_find_compatible_obj( pListHead, pCurrentEntry );

      if ( pCompatible )
      {
         // If we have a compatible object, let's simply chain ourselves as its first child
         pCurrentEntry->BP_precomp->mIsStart = 0;

         // Make sure that the compatible packet was a start packet
         ASSERT( pCompatible->BP_precomp->mIsStart );

         // We are the first child
         pCurrentEntry->BP_precomp->mpNextInterior = pCompatible->BP_precomp->mpNextInterior;
         pCompatible->BP_precomp->mpNextInterior = pCurrentEntry;

         // Since we're interior, prev start is the compatible unit, next start
         // is null
         pCurrentEntry->BP_precomp->mpPrevStart = pCompatible;
         pCurrentEntry->BP_precomp->mpNextStart = NULL;

#if DEBUG_VALIDATE_PRECOMP_LIST
         precomplist_validate( pListHead );
#endif
         return pListHead;
      }
      else
      {
         // There is no compatible entry.  Make us the head of the list, and 
         // make it so that our batch is the start and the end

//         printf( "obj: %8.8x new\n", pCurrentEntry );
         pCurrentEntry->BP_precomp->mIsStart = 1;

         if ( pListHead )
         {
            pListHead->BP_precomp->mpPrevStart = pCurrentEntry;
            pCurrentEntry->BP_precomp->mpNextStart = pListHead;
            pCurrentEntry->BP_precomp->mpPrevStart = NULL;
            pCurrentEntry->BP_precomp->mpNextInterior = NULL;

            // We are the new head
#if DEBUG_VALIDATE_PRECOMP_LIST
            precomplist_validate( pCurrentEntry );
#endif
            return pCurrentEntry;
         }
         else
         {
            // No head, everything's null

            pCurrentEntry->BP_precomp->mpNextStart = NULL;
            pCurrentEntry->BP_precomp->mpPrevStart = NULL;
            pCurrentEntry->BP_precomp->mpNextInterior = NULL;
            
#if DEBUG_VALIDATE_PRECOMP_LIST
            precomplist_validate( pCurrentEntry );
#endif
            return pCurrentEntry;
         }
      }
   }
}

static SBP_OBJ_Render *precomplist_find_or_add_cache_items( SChainOutputCache *pCache, SBP_OBJ_Render *pListHead )
{
   char *curObject = (char *) ( pCache + 1 );
   int i;
   SBP_OBJ_Render *pCurrentHead = pListHead;

   for ( i = 0; i < pCache->mCount * 2; ++i )
   {
      SBP_OBJ_Render *pObj = (SBP_OBJ_Render *) curObject;
      curObject += pObj->allocSize;

      pCurrentHead = precomplist_find_or_add( pCurrentHead, pObj );
   }
   
   return pCurrentHead;
}

void DG_AS_DumpPrecomputedChainBufferObjInfo( struct _SBP_OBJ_Render *pHead )
{
   struct _SBP_OBJ_Render *pStartPacket = pHead;
   int packetCount = 0;

   for ( ; pStartPacket; pStartPacket = pStartPacket->BP_precomp->mpNextStart )
   {
      // Count children
      int count = 0;
      SBP_OBJ_Render *pChild = pStartPacket->BP_precomp->mpNextInterior;
      for ( ; pChild; pChild = pChild->BP_precomp->mpNextInterior )
      {
         ++count;
      }

      ++packetCount;
//      printf( "%05d: %8.8x %d children\n", packetCount, pStartPacket, count );
   }
}

#endif // BP_VITA

SBP_OBJ_Render *DG_AS_PrecomputeChainBuffers( DG_OBJ *obj, SBP_OBJ_Render *pListHead )
{
#if BP_VITA
   SChainOutputCache cacheDef;
   unsigned char *pNoIRRenderCmdsBegin;
   unsigned char *pIRRenderCmdsBegin;
   int irCount, irSize;
   int totalDataSize;

   pNoIRRenderCmdsBegin = (unsigned char *)( BP_ScratchPadGV );
   BuildRenderCmds( obj, (SBP_OBJ_Render *) pNoIRRenderCmdsBegin, 0, sizeof( SBP_OBJ_PreComputedInfo ), &cacheDef.mCount, &cacheDef.mIRByteOffset );
   pIRRenderCmdsBegin = pNoIRRenderCmdsBegin + cacheDef.mIRByteOffset;
   BuildRenderCmds( obj, (SBP_OBJ_Render *) pIRRenderCmdsBegin, 1, sizeof( SBP_OBJ_PreComputedInfo ), &irCount, &irSize );
   totalDataSize = irSize + cacheDef.mIRByteOffset;

   if ( totalDataSize > 16 * 1024 )
   {
      printf( "DG_PrecomputeChainBuffers did an overrun of scratch!\n" );
      HANGUP();

      // If we're too big for the scratchpad, then we really have nothing to do
      // but let the default run.  Note that at this point, we've TOTALLY screwed memory.
      return NULL;
   }

   {
      int const totalChainBufferSize = totalDataSize + sizeof( cacheDef );
      SChainOutputCache *chainBufferRet = (SChainOutputCache *) BP_Memory_Alloc( totalChainBufferSize, 4, kMT_Normal, kMC_Renderer );

      *chainBufferRet = cacheDef;
      memcpy( chainBufferRet + 1, BP_ScratchPadGV, totalDataSize );

      FixupPrecomputedBuffer( &cacheDef, chainBufferRet + 1, BP_ScratchPadGV );

      obj->BP_PrecomputedChainBuffer = (int) chainBufferRet;
      return precomplist_find_or_add_cache_items( chainBufferRet, pListHead );
   }
#else 
   return NULL;
#endif // BP_VITA
}


void DG_AS_FreePrecomputedChainBuffers( DG_OBJ *obj )
{
#if BP_VITA
   if ( obj->BP_PrecomputedChainBuffer )
   {
      BP_WaitForLastRenderThreadToComplete();
      BP_Memory_Free( (void *) obj->BP_PrecomputedChainBuffer );
      obj->BP_PrecomputedChainBuffer = 0;
   }
#endif
}

#if BP_VITA

__attribute__((noinline))
static void DrawPrecomputedBuffer( int irMode, void *pChainBuffer, SBP_OBJ_PreRender_Vita *pUniformBuffer )
{
   SChainOutputCache *pCacheDef = (SChainOutputCache *) pChainBuffer;
   SBP_OBJ_Render *packetCurrent;
   int i;
   int const renderBufferIndex = gBP_RB_WritableBuffer;
   int const renderFrameCount = DG_Endframe_FrameCount;

   if ( irMode )
   {
      packetCurrent = (SBP_OBJ_Render *)( (char *)( pCacheDef + 1 ) + pCacheDef->mIRByteOffset );
   }
   else
   {
      packetCurrent = (SBP_OBJ_Render *)( pCacheDef + 1 );
   }
 
   for ( i = 0; i < pCacheDef->mCount; ++i )
   {
      SBP_OBJ_Render *pStartPacket = packetCurrent->BP_precomp->mIsStart ? packetCurrent : packetCurrent->BP_precomp->mpPrevStart;

      // Only flag the "start packet" to render and only once per update cycle
      if ( pStartPacket->BP_precomp->mLastFrameTouched[renderBufferIndex] != renderFrameCount )
      {
#if 0
         SBP_OBJ_Render *pPacket;
         for ( pPacket = pStartPacket; pPacket; pPacket = pPacket->BP_precomp->mpNextInterior )
         {
//            ASSERT( pPreComp->mpUniformBuffers[ pStartPacket->BP_precomp->mUpdateClock ] == NULL );
            pPreComp->mpUniformBuffers[ gBP_RB_WritableBuffer ] = NULL;
         }
#endif
         pStartPacket->BP_precomp->mLastFrameTouched[renderBufferIndex] = renderFrameCount;
         pStartPacket->BP_precomp->mpUniformBuffers[ renderBufferIndex ] = NULL;
         ASSERT( pStartPacket->BP_precomp->mIsStart );
         BP_RB_AddCommand( kCmd_Obj_Render, pStartPacket );
      }

      // But still update them...
      packetCurrent->BP_precomp->mpUniformBuffers[ renderBufferIndex ] = pUniformBuffer;
      packetCurrent->BP_precomp->mLastFrameTouched[ renderBufferIndex ] = renderFrameCount;

      packetCurrent = (SBP_OBJ_Render *)( ( (char *) packetCurrent ) + packetCurrent->allocSize );
   }
}

#endif // BP_VITA

static void CopyPreRenderPacketData( SBP_OBJ_PreRender *pPreRenderCmd, DG_OBJ *obj, KP_MatrixData const *pMatrixData )
{
   pPreRenderCmd->eye_pers = obj->screen; // pMatrixData->screen;
   pPreRenderCmd->correction = obj->inv_mat; // pMatrixData->connection;
   pPreRenderCmd->lightDir = pMatrixData->local_light;
   pPreRenderCmd->lightCol = pMatrixData->local_color;
//   pPreRenderCmd->BP_Flags = 0;
}

#if BP_VITA

#  define CHAIN_ULT_QUEUE_SIZE (25)

typedef struct _SChainUltParam
{
   int mCount;
   SBP_OBJ_PreRender_Vita *mpChainTop;
} SChainUltParam;

KP_CTASSERT( sizeof( SChainUltParam ) <= sizeof( SULTParam ) );

static SChainUltParam sWorkQueue = { 0 };

static void _AS_ChainWorkQueue_ProcessUltItem( SULTParam const *pParam )
{
   SChainUltParam const *pChainParam = (SChainUltParam const *) pParam;
   SBP_OBJ_PreRender_Vita *pChainCurrent = pChainParam->mpChainTop;

   for ( ; pChainCurrent; pChainCurrent = pChainCurrent->mpUltChain )
   {
      BP_Obj_PreRender_MakeVitaUniforms( pChainCurrent );
   }
}

static void _AS_ChainWorkQueue_Push( SBP_OBJ_PreRender_Vita *pPreRenderCmd, DG_OBJ *obj )
{
   pPreRenderCmd->mpUniformBufferPtr = (void *) 0xDEADBEEF;
   pPreRenderCmd->mpOrigObj = obj;

   if ( sWorkQueue.mCount == CHAIN_ULT_QUEUE_SIZE )
   {
      BP_Render_PostUltWork( _AS_ChainWorkQueue_ProcessUltItem, &sWorkQueue, sizeof( sWorkQueue ) );

      sWorkQueue.mCount = 1;
      pPreRenderCmd->mpUltChain = NULL;
      sWorkQueue.mpChainTop = pPreRenderCmd;
   }
   else
   {
      ++sWorkQueue.mCount;

      pPreRenderCmd->mpUltChain = sWorkQueue.mpChainTop;
      sWorkQueue.mpChainTop = pPreRenderCmd;
   }
}

static void _AS_ChainWorkQueue_Flush()
{
   if ( sWorkQueue.mCount > 0 )
   {
      BP_Render_PostUltWork( _AS_ChainWorkQueue_ProcessUltItem, &sWorkQueue, sizeof( sWorkQueue ) );
      sWorkQueue.mCount = 0;
      sWorkQueue.mpChainTop = NULL;
   }
}

#endif // BP_VITA
#endif // 1

static	void	ChainObj( DG_OBJ *obj )
{
   DG_OBJS* objs = (DG_OBJS*)obj->BP_DGObjs;
   KP_MatrixData* pMatrixData = (KP_MatrixData*)obj->matrix_addr;

#if 0
   if ( objs->flag & DG_FLAG_AS_NOBOUNDCHECK )
   {
      return;
   }
#endif

   if( pMatrixData )
   {
#if BP_VITA && 1
      SBP_OBJ_PreRender_Vita *pPreRenderCmdVita = (SBP_OBJ_PreRender_Vita *) BP_RB_Alloc( sizeof( SBP_OBJ_PreRender_Vita ) );

      BP_RB_AddCommand( kCmd_Obj_PreRenderVita, pPreRenderCmdVita );
      _AS_ChainWorkQueue_Push( pPreRenderCmdVita, obj );
#else
#  if BP_VITA
      SBP_OBJ_PreRender_Vita *pPreRenderCmdVita = NULL;

      if ( gAS_UsedBufferSceneNOP )
      {
         // Set up the PreRender
         pPreRenderCmdVita = (SBP_OBJ_PreRender_Vita *) BP_RB_Alloc( sizeof( SBP_OBJ_PreRender_Vita ) );

         BP_RB_AddCommand( kCmd_Obj_PreRenderVita, pPreRenderCmdVita );
         _AS_ChainWorkQueue_Push( pPreRenderCmdVita, obj );
      }
      else
#  endif
      {
         SBP_OBJ_PreRender *pPreRenderCmd;

         // Set up the PreRender
         pPreRenderCmd = (SBP_OBJ_PreRender *) BP_RB_Alloc( sizeof( SBP_OBJ_PreRender ) );
         BP_RB_AddCommand( kCmd_Obj_PreRender, pPreRenderCmd );
         CopyPreRenderPacketData( pPreRenderCmd, obj, pMatrixData );
      }
#endif

#if BP_VITA
      if ( obj->BP_PrecomputedChainBuffer && pPreRenderCmdVita )
      {
         DrawPrecomputedBuffer( ( DG_DisplayStatus & DG_STATE_IR_MODE ), (void *) obj->BP_PrecomputedChainBuffer, pPreRenderCmdVita );
      }
      else
#endif
      {
         int renderCmdCount, renderTotalSize;
         char *pRenderBuffer = BP_RB_GetCurrentPtr();
         SBP_OBJ_Render *pCurrentObject = (SBP_OBJ_Render *) pRenderBuffer;
         int i;

         if(obj->BP_Model)
            BP_Obj_Render_ComputeAnimatedBuffers((char *) objs, (char *) obj->BP_Model);

         BuildRenderCmds( obj, (SBP_OBJ_Render *) pRenderBuffer, ( DG_DisplayStatus & DG_STATE_IR_MODE ), 0, &renderCmdCount, &renderTotalSize );

         BP_RB_SetCurrentPtr( pRenderBuffer + renderCmdCount * OBJ_RENDER_DATASIZE );
         for ( i = 0; i < renderCmdCount; ++i )
         {
            BP_RB_AddCommand( kCmd_Obj_Render, pCurrentObject );
            pCurrentObject = (SBP_OBJ_Render *) ( ((char *) pCurrentObject ) + pCurrentObject->allocSize );
         }
      }
   }
}
#endif
/*----------------------------------------------------------------*/

	/*
		オブジェクトのソートリストを作成する
	*/
static	void	MakeSortList( DG_OBJS *objs )
{
	ScrpadWork	*work = (ScrpadWork*)SCRPAD_ADDR ;
	SORT_WORK	*sort_work ;
	SORT_TAG	*list ;
	DG_OBJ		*obj ;
	int			i ;
	u_int		z ;
	void		**ot ;
#define CHAIN_PRINT_MODEL_NAMES 0
#if CHAIN_PRINT_MODEL_NAMES
   char const *lastModelName = NULL;
#endif
	//sort_work = (SORT_WORK*)work->local_work ;
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;

	list = work->now ;
	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
#if CHAIN_PRINT_MODEL_NAMES
      char const *modelName;
#endif
      if ( obj->bound_mode & 2 ) continue ;
		if ( !( obj->mdl_type & DG_TYPE_TRANS ) ) continue ;
		if ( obj->flag & work->invisible_flag ) continue ;
		/* ソート用データを作成 */
#if BP_ENABLE_TESTNODE
      if ( !BP_Debug_EvalTestNode() ) continue;
#endif
      
#if CHAIN_PRINT_MODEL_NAMES
      modelName = BP_Debug_GetModelName( obj->BP_Model );
      if ( modelName != lastModelName )
      {
         printf( "obj: %s\n", modelName );
         lastModelName = modelName;
      }

#endif
		z = ( ( (u_int)obj->sort_z + 0x7fffff ) >> 6 ) ;
		list->obj = obj ;
		list->z = z ; z &= 63 ;
		ot = sort_work->ot1 + z ;
		list->tag = *ot ; *ot = list ;
		list++ ;
		work->max_num++ ;
	}
	work->now = list ;

}

	/*
		ソートを行う
	*/
static	void	SortList( void )
{
	//ScrpadWork	*work = (ScrpadWork*)SCRPAD_ADDR ;
	SORT_WORK	*sort_work ;
	int			i, z ;
	SORT_TAG	*list ;
	void		**ot2, **ot3, *next_addr ;

	//sort_work = (SORT_WORK*)work->local_work ;
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;

	/* 大きい方からソートしていく */
	for ( i = 63 ; i >= 0  ; i-- ){
		next_addr = sort_work->ot1[i] ;
		while ( next_addr != NULL ){
			list = next_addr ;
			next_addr = list->tag ;
			z = list->z ; z = ( z >> 6 ) & 63 ;
			ot2 = sort_work->ot2 + z ;
			list->tag = *ot2 ;
			*ot2 = list ;
		}
	}

	/* 小さい方からソートしていく */
	for ( i = 0 ; i < 64   ; i++ ){
		next_addr = sort_work->ot2[i] ;
		while ( next_addr != NULL ){
			list = next_addr ;
			next_addr = list->tag ;
			z = list->z ; z = ( z >> 12 ) & 63 ;
			ot3 = sort_work->ot3 + z ;
			list->tag = *ot3 ;
			*ot3 = list ;
		}
	}

}


	/*
		ＤＭＡに接続する
	*/
static	void	SortChainObjs( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	SORT_WORK	*sort_work ;
	DG_OBJ		*obj ;
	int			k, size, obj_flag ;
	void		*next_addr, *next_addr_prim ;
	SORT_TAG	*list ;
	DG_PRIM2_PACKET	*prim_packet ;
	extern void	*DG_Prim2OT[64] ;
   
   void const *pAS_LastDrawPrimRoot = NULL;
   int   as_wasLastDrawPrim = 0;
   int   as_lastRelevantDrawFlags = 0;

	///* スクラッチパッド上のオブジェクトのソート結果をメインメモリに書き出す */
	//FlushCache( 0 );
	//DG_StartSprToMem( DG_AS_GetLocalMemoryAddress(), scrpad->local_work, sizeof(SORT_WORK)/16 );
	//asm volatile( "sync.l" );
	//DG_EndSprToMem();
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;

#if BP_VITA
   DG_BP_WritePrimInitPacket( cp );
#endif


	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );
	for ( k = 63 ; k >= 0 ; k-- )
   {
		next_addr = sort_work->ot3[k] ;
		next_addr_prim = DG_Prim2OT[k];
		DG_Prim2OT[k] = NULL ;
		while ( next_addr != NULL || next_addr_prim != NULL )
      {
			list = next_addr ;
			prim_packet = next_addr_prim ;

			/* 処理を行うオブジェクトを決定（マージソート） */
			if ( next_addr_prim == NULL || next_addr == NULL )
         {
				if ( next_addr == NULL )
            {
               obj_flag = 0 ;
            }
				else
            {
               obj_flag = 1 ;
            }
			}
         else
         {
				if ( list->z > prim_packet->sort_z )
            {
               obj_flag = 1 ;
            }
				else
            {
               obj_flag = 0 ;
            }
			}

			if ( obj_flag )
         {
				//if ( GV_PadData[1].press & PAD_A ) printf( "objs: %d\n", list->z*64-0x7fffff );
				/* ソートしたオブジェクトを接続 */
            as_wasLastDrawPrim = 0;
				next_addr = list->tag ;
				obj = list->obj ;
#ifdef LIBDG_PERFORMANCE
				scrpad->verts_count = 0 ;
				scrpad->pack_count = 0 ;
#endif
				scrpad->fog = obj->fog ;
				scrpad->matrix_addr = obj->matrix_addr ;
            BP_ChainObjectLocalParam((DG_OBJS*)obj->BP_DGObjs);
				ChainObj( obj );
#ifdef LIBDG_PERFORMANCE
				scrpad->model[ obj->bound_mode ].n_obj++ ;
				scrpad->model[ obj->bound_mode ].n_packs += scrpad->pack_count ;
				scrpad->model[ obj->bound_mode ].n_verts += scrpad->verts_count ;
#endif
			}
         else
         {
				//if ( GV_PadData[1].press & PAD_A ) printf( "prim: %d\n", prim_packet->sort_z*64-0x7fffff );
				/* ソートしたプリミティブを接続 */

#if BP_VITA
            DG_PRIM2 *prim = prim_packet->prim;
            // The only draw flags that we care about in terms of local param state changes are
            // if we've switched into/out of on camera mode, or if we've switched from CustomWorld to
            // something else.  Every customworld prim needs to have localparams sent up.
            //
            // These are the only draw flags that affect the screen matrix, which is all LocalParam is.

            int currentRelevantDrawFlags = prim->flag & ( DG_PRIM_AS_CUSTOMWORLD | DG_PRIM2_ON_CAMERA );

            if ( 
               !as_wasLastDrawPrim ||                     // If the last thing drawn wasn't a prim...
               prim->root != pAS_LastDrawPrimRoot ||      // If the root is different
               ( prim->flag & DG_PRIM_AS_CUSTOMWORLD ) || // If the system has customized its world matrix
               currentRelevantDrawFlags != as_lastRelevantDrawFlags       // If relevant draw flags changed
               )
            {
               DG_BP_WritePrimLocalParam( prim );
               pAS_LastDrawPrimRoot = prim->root;
               as_lastRelevantDrawFlags = currentRelevantDrawFlags;
            }
            as_wasLastDrawPrim = 1;
#endif
				next_addr_prim = prim_packet->next_addr ;
				size = DG_BP_WritePrimPacksAndPendRender( cp, prim_packet, -1 ) ;
				DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			}
		}
	}

   DG_BP_WritePrimPacksFinalize();

   DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

}

/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをソートしてＤＭＡパケットに接続する
	*/
void		_DG_SortChainChanl( DG_CHANL *cp, int which, int draw_mode )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		i, size, gid, c_gid ;
	SORT_WORK		*sort_work ;

	que = cp->obj_queue ;

	MARK( "chain.c(sort)" );
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;
	c_gid = cp->group_id ;

   //if( !(cp->flag & DG_VIEWPORT_STAGE_DISABLE_MSAA) )
   if( 1 )
   {
      BP_GetCurrentViewportInfo()->hasMSAAStuff = 1;
   }
   else
   {
      BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   }
   BP_RB_PushRegionMarker(kProfileColor_Semitrans, draw_mode ? "Sort Chain First" : "Sort Chain");
   BP_Debug_PushCPUMarker( draw_mode ? "Sort Chain First" : "Sort Chain" );
   BP_RB_AddCommand(kCmd_Obj_SortChainInitPacket, NULL);
	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	/* 初期化パケットの設定 */
	size = DG_WriteObjsPacketInit2( scrpad->dma_buffer, cp );
	DG_StartSprToMem( DG_CurrentDmaAddr, scrpad->dma_buffer, size );
	DG_CurrentDmaAddr += size ;

	/* ワーク初期化 */
	//sort_work = (SORT_WORK*)scrpad->local_work ;
	sort_work = (SORT_WORK*)DG_AS_GetLocalMemoryAddress() ;
	//memset( sort_work->ot3, 0, sizeof(int)*64*3 );
	GV_ZeroMemory( sort_work->ot3, sizeof(int)*64*3 );
	scrpad->max_num = 0 ;
	scrpad->now = sort_work->list ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
#ifdef LIBDG_PERFORMANCE
	scrpad->model[0] = DG_PerformanceData.trans_model[0] ;
	scrpad->model[1] = DG_PerformanceData.trans_model[1] ;
#endif

#if 0 //BP_PS2
   for ( i = 0 ; i < 16 ; i++ ){
		scrpad->vu_prog_list[i] = SELECT_VU1_PROGRAM_FUNC[i] ;
	}
#endif
	/* スクラッチパッド上のパケットバッファを初期化 */
	SetupMatrixStack( &scrpad->matrix_stack );
	SetupDmaPacket( &scrpad->pre_packet[0] );
	SetupDmaPacket( &scrpad->pre_packet[1] );

	if ( cp->chanl_num < 4 ){/* メニューチャンネルの時以外読み込むようにする */
		/* 半透明テクスチャの読み込み */
		for ( tex_list_p = DG_TextureList, i = DG_MaxTextures ; i > 0 ; tex_list_p++, i-- ){
			tex_list = *tex_list_p;
			if ( tex_list->header->compress_flag & TRI_FLAG_TRANS ){
				DG_EndSprToMem();
#ifdef LIBDG_PERFORMANCE
				DG_PerformanceData.use_tex_size += tex_list->tex_size ;
#endif
				//size = DG_WriteTextureChangePacks( scrpad->dma_buffer, &tex_list->tex_packet[which] );
				size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
				DG_StartSprToMem( DG_CurrentDmaAddr, scrpad->dma_buffer, size );
				DG_CurrentDmaAddr += size ;
			}
		}
	}

	/* リストの作成 */
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		if ( objs->flag & DG_FLAG_MULTITEX ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) continue ;

      // AS(JM) draw_mode, excellently documented, controls if it's in the msaa scene
      // draw_mode 0 is MSAA, draw_mode 1 is not.  Therefore, check against alternating
      // flags
      if ( draw_mode == 0 && ( objs->flag & DG_FLAG_NOMSAA ) ) continue;
      if ( draw_mode == 1 && !( objs->flag & DG_FLAG_NOMSAA ) ) continue;

      /* cp->group_id実験 （2000/01/11 M.Sonoyama） */
		if ( !( objs->group_id & cp->group_id ) ) continue ;

		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		MakeSortList( *oque );
	}

	/* ソート */
	SortList();

	/* ＤＭＡ書き出し処理 */
	SortChainObjs( cp, which );

	/* 終了パケットの設定 */
	DG_EndSprToMem();
	size = DG_WriteObjsPacketEnd( scrpad->dma_buffer );
	DG_StartSprToMem( DG_CurrentDmaAddr, scrpad->dma_buffer, size );
	DG_CurrentDmaAddr += size ;
	DG_EndSprToMem();

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();

   BP_RB_AddCommand(kCmd_Obj_SortChainEndPacket, NULL);
   BP_Debug_PopCPUMarker();

#if BP_VITA
   _AS_ChainWorkQueue_Flush();
#endif
   BP_RB_PopRegionMarker();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.trans_model[0] = scrpad->model[0] ;
	DG_PerformanceData.trans_model[1] = scrpad->model[1] ;
#endif
}

/*----------------------------------------------------------------*/
	/*
		ＤＭＡに接続する
	*/
/*----------------------------------------------------------------*/

static	void	ChainObjs( DG_CHANL *cp, DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;

   BP_ChainObjectLocalParam(objs);

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->mdl_type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & scrpad->invisible_flag ) continue ;
#if BP_ENABLE_TESTNODE
      if ( !BP_Debug_EvalTestNode() ) continue;
#endif

#ifdef LIBDG_PERFORMANCE
		scrpad->verts_count = 0 ;
		scrpad->pack_count = 0 ;
#endif
		scrpad->fog = obj->fog ;
		scrpad->matrix_addr = obj->matrix_addr ;
		ChainObj( obj );
#ifdef LIBDG_PERFORMANCE
		scrpad->model[ obj->bound_mode ].n_obj++ ;
		scrpad->model[ obj->bound_mode ].n_packs += scrpad->pack_count ;
		scrpad->model[ obj->bound_mode ].n_verts += scrpad->verts_count ;
#endif
	}
	//DG_EndSprToMem();

}

/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
void		_DG_ChainChanl( DG_CHANL *cp, int which, int draw_mode )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, j, size, old_tex_code = -1, max_objs, max_mem_objs ;
	DG_TEXTURE_LIST	*tex_list, **tex_list_p ;
	int		c_gid, gid ;
	OBJ_LIST		*obj_list, *mem_obj_list ;
	int		last_semi_trans = 0 ;
	//int		time ;

	que = cp->obj_queue ;
	MARK( "chain.c(normal)" );
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	/* スクラッチパッド内の必要なワークを初期化する */
	c_gid = cp->group_id ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->eye_inv = cp->eye_inv ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
#ifdef LIBDG_PERFORMANCE
	scrpad->model[0] = DG_PerformanceData.normal_model[0] ;
	scrpad->model[1] = DG_PerformanceData.normal_model[1] ;
#endif
#if 0 //BP_PS2
	for ( i = 0 ; i < 16 ; i++ ){
		scrpad->vu_prog_list[i] = SELECT_VU1_PROGRAM_FUNC[i] ;
	}
#endif
	FlushCache( 0 );

	/* スクラッチパッド上のパケットバッファを初期化 */
	SetupMatrixStack( &scrpad->matrix_stack );
	SetupDmaPacket( &scrpad->pre_packet[0] );
	SetupDmaPacket( &scrpad->pre_packet[1] );

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) 
   {
		objs = *oque ;
		if ( objs->flag & DG_FLAG_MULTITEX ) 
         continue ;
		if ( objs->flag & scrpad->invisible_flag ) 
         continue ;
		if ( objs->flag & DG_FLAG_PLUGINDRAW ) 
         continue ;
      if ( draw_mode == 0 ){
         if ( objs->flag & DG_FLAG_LATTERDRAW ) continue ;
         if ( objs->flag & DG_FLAG_FORCEMSAA ) continue ;
      } else if ( draw_mode == 1 ){
         if ( !( objs->flag & DG_FLAG_LATTERDRAW ) ) continue ;
         if ( objs->flag & DG_FLAG_FORCEMSAA ) continue ;
      } else if ( draw_mode == 2 ){
         if ( !( objs->flag & DG_FLAG_FORCEMSAA ) ) continue;
      }

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & c_gid ) ) 
         continue ;

		//if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWMAKE|DG_FLAG_SHADOWWRITE ) ) continue ;
		if ( objs->flag & ( DG_FLAG_SHADOWVOL|DG_FLAG_SHADOWWRITE ) ) 
         continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) 
         continue ;
		if ( objs->bound_mode == 2 ) 
         continue ;
		obj_list[max_objs].objs = objs ;
		obj_list[max_objs].use_tri = objs->tri_id ;
		max_objs++ ;
	}

	if ( max_objs == 0 ) 
      return ;

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

   BP_RB_PushRegionMarker(kProfileColor_FixModel, "Chain");
   BP_Debug_PushCPUMarker( "Chain" );

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* 初期化パケットの設定 */
	size = DG_WriteObjsPacketInit( scrpad->dma_buffer, cp, 0 );
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

   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();

#if BP_VITA
   _AS_ChainWorkQueue_Flush();
#endif

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.normal_model[0] = scrpad->model[0] ;
	DG_PerformanceData.normal_model[1] = scrpad->model[1] ;
#endif
}

void DG_ChainChanl( DG_CHANL *cp, int which )
{
	_DG_ChainChanl( cp, which, 0 );
}

void DG_ChainChanlLast( DG_CHANL *cp, int which )
{
	_DG_ChainChanl( cp, which, 2 );
}

void DG_SortChainChanl( DG_CHANL *cp, int which )
{
   _DG_SortChainChanl( cp, which, 0 );
}

void DG_SortChainFirstChanl( DG_CHANL *cp, int which )
{
   _DG_SortChainChanl( cp, which, 1 );
}
