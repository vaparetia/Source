//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	opt_cmf.c
	チャンネル処理ユニット／光学迷彩オブジェクトＤＭＡ接続ルーチン

	2000/03/22 K.Takabe
	$Id: opt_cmf.c,v 1.1.1.3 2002/11/19 11:42:16 Yoshizawa1 Exp $

*/
/*

	void		DG_OptCamouflageChainChanl( cp, which )
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

#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#include "bp_matrix.h"

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_Debug.h"

//#define DG_EndSprToMem()	UTL_EndSprToMem()
//#define DG_StartSprToMem( _a, _b, _c )	UTL_StartSprToMem( _a, _b, _c )

/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(7419565)	/* "opt_cmf" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_ENABLE)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_NORMAL)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(0)



/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	DmapackPlugin ;

	/*
		プラグイン固有ワーク
	*/
/* 光学迷彩用テクスチャ設定パケット */
static ALIGN16_PRE DG_TEX_TRANS	frame_buffer_tex[2] ALIGN16_POST;

/* 光学迷彩用テクスチャ設定パケット */
static ALIGN16_PRE DG_TEX_TRANS	null_solid_tex ALIGN16_POST = {
	{ SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7), 0x0fffeeee },
	{ SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1), SCE_GS_CLAMP_1 },
	{ 0,SCE_GS_TEX2_1 },
	{ SCE_GS_SET_TEX0( BUFFER_PAGE(0) / 64 ,BUFFER_WIDTH/64,SCE_GS_PSMCT24,
					  9,9,0,1,0,0,0,0,0),SCE_GS_TEX0_1 },
	{ SCE_GS_SET_ALPHA( 2, 2, 2, 1, 128 ),SCE_GS_ALPHA_1 },
	{0,SCE_GS_NOP},
	{0,0,0,0},
	{0,0,0,0}
};


/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );
static void		DG_OptcmfChainChanl( DG_CHANL *cp, int which );

/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/
void DG_AddPluginOptcmf( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &DmapackPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &DmapackPlugin );
	/*
		プラグイン固有初期化処理
	*/
	frame_buffer_tex[0].giftag.tag = SCE_GIF_SET_TAG( 1, 1, 0, 0, 0, 7) ;
	frame_buffer_tex[0].giftag.regs = 0x0fffeeee ;/* A+D */
	frame_buffer_tex[0].clamp.reg = SCE_GS_CLAMP_1 ;
	frame_buffer_tex[0].tex0.reg = SCE_GS_TEX0_1 ;
	frame_buffer_tex[0].tex2.reg = SCE_GS_TEX2_1 ;
	frame_buffer_tex[0].alpha.reg = SCE_GS_ALPHA_1 ;
	//frame_buffer_tex[0].clamp.data = SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1, 1, DRAW_HEIGHT-1) ;
	frame_buffer_tex[0].clamp.data = SCE_GS_SET_CLAMP(0,0,0,0,0,0) ;
	frame_buffer_tex[0].alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ;
	frame_buffer_tex[0].tex2.data = 0 ;
	frame_buffer_tex[1] = frame_buffer_tex[0] ;

#ifndef HIGHRESO_FFI
	frame_buffer_tex[0].tex0.data = 
	  SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
					  9,8,0,0,0,0,0,0,0);
	frame_buffer_tex[1].tex0.data = 
	  SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
					  9,8,0,0,0,0,0,0,0);
#else
	frame_buffer_tex[0].tex0.data = 
	  SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
					  9,9,0,0,0,0,0,0,0);
	frame_buffer_tex[1].tex0.data = 
	  SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
					  9,9,0,0,0,0,0,0,0);
#endif

	PluginStartFlag = 1 ;
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginOptcmf( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &DmapackPlugin );
		DG_FreePlugin( &DmapackPlugin );
	}
	PluginStartFlag = 0 ;
}
/* ---------------------------------------------------------------- */
	/*
		プラグイン実行アクター
	*/
static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
	DG_OptcmfChainChanl( cp, which );
}

/* ---------------------------------------------------------------- */


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


#if 0 //BP_PS2
/* マイクロプログラム読み込みタグ */
extern qword Vu1DrawOptcmfObject ;	/* 光学迷彩用 */
/* マイクロプログラム実行アドレスリスト */
extern int Vu1DrawOptcmfObject_Func[] ;
#endif

/*----------------------------------------------------------------*/
	/*
		各種ローカル構造体定義
	*/

/* １パケット分のＤＭＡ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _pack_dma_packet {
	DG_DMATAG		verts_trans ;		/* 頂点転送ＤＭＡタグ */
	DG_DMATAG		norms_trans ;		/* 法線転送ＤＭＡタグ */
	DG_DMATAG		trans_tag ;			/* パラメータ転送ＤＭＡタグ */
	struct _pack_dma_datas{
		int			n_verts ;
		int			flag ;
		int			fog ;
		int			pad ;
		DG_GIFTAG	giftag_normal ;	/* 通常描画用描画ＧＩＦタグ */
		DG_GIFTAG	giftag_clip ;	/* クリップポリゴン用描画ＧＩＦタグ */
		IVECTOR		gif_regslist ;	/* マルチテクスチャ用レジスタリスト */
	} datas ;
	DG_VIFCODE		prog_exec ;
} PACK_DMA_PACKET /* BP  */;

/* オブジェクト用マトリクス転送構造体 */
typedef ALIGN16_DECL(struct) _mat_dma_packet {
	DG_DMATAG	trans_tag ;
	struct _mat_dma_datas {
		FMATRIX		screen ;
		FMATRIX		local_light ;
		FMATRIX		local_color ;
		FMATRIX		connection ;
	} datas ;
} MAT_DMA_PACKET  ;

/* テクスチャ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _tex_dma_packet {
	DG_DMATAG		tex0_trans ;		/* テクスチャ０転送ＤＭＡパケット */
} TEX_DMA_PACKET ;

/* ＤＭＡ転送データパケット */
typedef ALIGN16_DECL(struct) _packet_buffer {
	/* マトリクス転送 */
	MAT_DMA_PACKET		mat_packet ;
	/* テクスチャ情報転送 */
	TEX_DMA_PACKET		tex_packet ;
	/* 頂点情報転送 */
	PACK_DMA_PACKET		pack_packet ;
} PACKET_BUFFER ;


/*----------------------------------------------------------------*/
	/*
		スクラッチパッドワーク定義
	*/
typedef struct _Opt_Cmf_SORT_TAG {
	void		*tag ;
	u_int		z ;
	void		*obj ;
} SORT_TAG ;

/* 通常モデル描画用ワーク */
typedef struct _Opt_Cmf_OBJ_LIST {
	DG_OBJS		*objs ;
	int			use_tri ;
} OBJ_LIST ;

typedef struct _Opt_Cmf_ScrpadWork {
	FMATRIX		eye_pers ;
	FMATRIX		eye_inv ;
	FMATRIX		rot_mat ;
	int			invisible_flag ;
	int			buffer_switch ;
	int			pad[2] ;
	PACKET_BUFFER	dma_packet[2] ;		/* ＤＭＡパケット（ダブルバッファ） */
	u_long128		dma_buffer[128] ;	/* 汎用ＤＭＡパケット生成バッファ */
	/* オブジェクト状態管理用 */
	FMATRIX		screen ;
	FMATRIX		local_light ;
	FMATRIX		local_color ;
	FMATRIX		connection ;
	int			fog ;
	void		*texture ;

	/* ローカルワーク */
	u_long128	local_work[0] ;
} ScrpadWork ;

/* 汎用メモリワーク（主にスクラッチパッド上で構成し終わったデータなどを退避） */
extern char	*DG_AS_GetLocalMemoryAddress();

static inline PACKET_BUFFER* NEXT_BUFF(ScrpadWork* scrpad)	{ scrpad->buffer_switch = 1 - scrpad->buffer_switch ; return (&scrpad->dma_packet[ scrpad->buffer_switch ] ) ;}

/*----------------------------------------------------------------*/

struct _backup_vram {
	DG_DMATAG		dmatag ;
	struct _gif_packet {
		DG_GIFTAG	giftag ;
		struct _data {
			DG_GSREG	bitbltbuf ;
			DG_GSREG	trxpos ;
			DG_GSREG	trxreg ;
			DG_GSREG	trxdir ;
			DG_GSREG	texflush ;
		} data ;
	} gif_packet ;
} backup_vram_packet = {
	{ DMATAG_SET_QWC( DMATAG_ID_CNT, sizeof(struct _gif_packet)/16 ), NULL,
		{ SCE_VIF1_SET_FLUSHA( 0 ), SCE_VIF1_SET_DIRECT( sizeof(struct _gif_packet)/16,0 ) }},
	{
		{ SCE_GIF_SET_TAG( sizeof(struct _data)/16, 1, 0, 0, SCE_GIF_PACKED, 1), 0x0e },
		{
			{ SCE_GS_SET_BITBLTBUF( 0/64, DRAW_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
								   TEXTURE_TOP_PAGE()/64, DRAW_WIDTH/64,FRAME_BUFFER_COLOR_MODE()), SCE_GS_BITBLTBUF },
			{ SCE_GS_SET_TRXPOS( 0, 0, 0, 0, 0 ), SCE_GS_TRXPOS },
			{ SCE_GS_SET_TRXREG( DRAW_WIDTH, DRAW_HEIGHT ), SCE_GS_TRXREG },
			{ 2, SCE_GS_TRXDIR },
			{ 0, SCE_GS_TEXFLUSH }
		}
	}
};

static int SetVramBackupPacket( void *addr, int which )
{
	*(struct _backup_vram*)addr = backup_vram_packet ;
	((struct _backup_vram*)addr)->gif_packet.data.bitbltbuf.data = 
	  SCE_GS_SET_BITBLTBUF( BUFFER_PAGE(which)/64, DRAW_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
						   TEXTURE_TOP_PAGE()/64, DRAW_WIDTH/64,FRAME_BUFFER_COLOR_MODE()) ;
	return ( sizeof(struct _backup_vram)/16 );
}


/*----------------------------------------------------------------*/
#if 0
static void SetLightMatrix( FMATRIX *res_light, FMATRIX *light, DG_OBJ *obj )
{
	asm ("
		lqc2		vf4,0x00(%1)
		lqc2		vf5,0x10(%1)
		lqc2		vf6,0x20(%1)
		lqc2		vf7,0x30(%1)
		li			$7,4
_loopMulMatrix:
		lqc2		vf8,0x0(%2)
		vmulax.xyz		ACC,vf4,vf8
		vmadday.xyz		ACC,vf5,vf8
		vmaddaz.xyz		ACC,vf6,vf8
		vmaddw.xyz		vf12,vf7,vf8
		sqc2		vf12,0x0(%0)
		addi		$7,-1
		addi		%2,0x10
		addi		%0,0x10
		bne			$0,$7,_loopMulMatrix
	"::"r"(res_light),"r"(light),"r"(&obj->world):"memory","$7");
}
#endif

/*----------------------------------------------------------------*/
	/*
		パケット初期化関連
	*/

/* デフォルトパケット初期化ルーチン */
static void SetupDmaPacket( PACKET_BUFFER *buff, int which )
{
#if 1 //BP_GCC
   static PACK_DMA_PACKET	def_pack_packet = {
      //verts_trans
      { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,
         {
            SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
            SCE_VIF1_SET_UNPACKR( 0x100, 0, VIF_VERT_PACK, 0 )
         }
      },
      //norms_trans
      { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,
         {
            SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
            SCE_VIF1_SET_UNPACKR( 0x101, 0, VIF_NORM_PACK, 0 )
         }
      },
      //trans_tag
      { DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _pack_dma_datas) + 1 ),NULL,
         {
            SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
            SCE_VIF1_SET_UNPACKR( 0x14, SIZEOF_QWORD(struct _pack_dma_datas), VIF_DATA128, 0 )
         }
      },
      // datas
      {
         0, //n_verts
         0, //flag
         0, //fog
         0, //pad
         //giftag_normal
         { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 1, 0, 0),
            SCE_GIF_PACKED, 3),
            GS_REGS_3(GS_REGS_UV,GS_REGS_RGBA,GS_REGS_XYZF2) },
         //giftag_clip
         { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 1, 0, 0),
         SCE_GIF_PACKED, 3),
         GS_REGS_3(GS_REGS_UV,GS_REGS_RGBA,GS_REGS_XYZF2) },
         //gif_regslist
         {GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
         GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
         GS_REGS_5( GS_REGS_NOP, GS_REGS_STQ, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
         GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2)},
      },
      //prog_exec
      {{SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 )}},
   };
#else
	static PACK_DMA_PACKET	def_pack_packet = {
		.verts_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x100, 0, VIF_VERT_PACK, 0 )}},
		.norms_trans = { DMATAG_SET_QWC( DMATAG_ID_REF, 0 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 3, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x101, 0, VIF_NORM_PACK, 0 )}},
		.trans_tag = { DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _pack_dma_datas) + 1 ),NULL,{
			SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
			SCE_VIF1_SET_UNPACKR( 0x14, SIZEOF_QWORD(struct _pack_dma_datas), VIF_DATA128, 0 ) }},
		.datas = {
			.giftag_normal = { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRISTRIP, 1, 1, 1, 1, 0, 1, 0, 0),
											   SCE_GIF_PACKED, 3),
								 GS_REGS_3(GS_REGS_UV,GS_REGS_RGBA,GS_REGS_XYZF2) },
			.giftag_clip = { SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_TRIFAN, 1, 1, 1, 1, 0, 1, 0, 0),
											 SCE_GIF_PACKED, 3),
							   GS_REGS_3(GS_REGS_UV,GS_REGS_RGBA,GS_REGS_XYZF2) },
			.gif_regslist = {GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_NOP, GS_REGS_STQ, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2),
							   GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2)},
		},
		.prog_exec = {{SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 ),SCE_VIF1_SET_NOP( 0 )}},
	} ;
#endif

	buff->pack_packet = def_pack_packet ;

	buff->tex_packet.tex0_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
	buff->tex_packet.tex0_trans.addr = &frame_buffer_tex[which] ;
	buff->tex_packet.tex0_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->tex_packet.tex0_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xe8, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;

	buff->mat_packet.trans_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _mat_dma_datas) );
	buff->mat_packet.trans_tag.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->mat_packet.trans_tag.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x00, SIZEOF_QWORD(struct _mat_dma_datas), VIF_DATA128, 0 );


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
	*(u_long128*)&packet->init_vu1_prog = *(u_long128*)Vu1DrawOptcmfObject ;
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
static	void	ChainObj( DG_OBJ *obj )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int			j ;
	int			first_flag, prog_no ;
	u_long128	*verts, *norms ;/* スクラッチパッドに入れた方がいいかも */
	PACKET_BUFFER	*buff ;

	buff = NEXT_BUFF(scrpad);	/* 次のＤＭＡパケットバッファ取得 */

	/* 法線投影マトリクスを計算 */
	_sceVu0MulMatrix( &scrpad->local_light, &scrpad->eye_inv, &obj->world );
	//scrpad->local_light = obj->world ;
	/* 法線投影マトリクスへの回転エフェクト */
	_sceVu0MulMatrix( &scrpad->local_light, &scrpad->rot_mat, &scrpad->local_light );

	/* フォグ計算 */
	if ( !( obj->flag & DG_FLAG_NOFOG ) ){
		float	f ;
		int		ifog ;
		f =  DG_FogParam1 * obj->sort_z + DG_FogParam2 ;
		f = DG_MAX( f, 0.0f );
		f = DG_MIN( f, 255.0f );
		ifog = DG_FTOI( f ) ;
		scrpad->fog = ( ifog << 4 ) | 0x8000 ;
	} else {
		scrpad->fog = ( 255 << 4 ) | 0x8000 ;
	}

	/* オブジェクト描画パラメータ構造体にデータをセット */
	_sceVu0MulMatrix( &scrpad->screen, &scrpad->eye_pers, &obj->world );
	scrpad->connection = obj->inv_mat ;

	/* ＤＭＡパケット生成準備 */
	first_flag = 2 ;
	pack = obj->packets ;
	verts = (u_long128*)obj->verts ;
	norms = (u_long128*)obj->norms ;
	for ( j = obj->n_packs ; j > 0 ; j-- ){
		GV_PREFECH( pack + 1 );
		/* パケット生成 */
		verts = SetVertsTrans( &buff->pack_packet.verts_trans,
							  0x100, pack->verts_offset*2, pack->verts_offset, verts, VIF_VERT_PACK );
		norms = SetVertsTrans( &buff->pack_packet.norms_trans,
							  0x101, pack->norms_offset*2, pack->norms_offset, norms, VIF_NORM_PACK );
		buff->pack_packet.datas.n_verts = pack->n_verts ;
		buff->pack_packet.datas.flag = 0 ;

		{/* 実行するＶＵ１プログラムを決定 */
			prog_no = 0 ;
			/* エンベロープチェック */
			if ( pack->flag & DG_PACKFLAG_ENVELOPE ) prog_no = 1 ;
		}
#if 0 //BP_PS2
		buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_MSCAL( Vu1DrawOptcmfObject_Func[prog_no], 0 ) ;
#endif
		//buff->pack_packet.prog_exec.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
		//buff->pack_packet.prog_exec.vifcode[1] = SCE_VIF1_SET_MARK( 123, 0 ) ;

		/* パケットをメモリへ転送 */
		if ( first_flag > 0 ){/* 最初の２回のみ */
			buff->mat_packet.datas.screen = scrpad->screen ;
			buff->mat_packet.datas.local_light = scrpad->local_light ;
			buff->mat_packet.datas.local_color = scrpad->local_color ;
			buff->mat_packet.datas.connection = scrpad->connection ;
			buff->pack_packet.datas.fog = scrpad->fog ;
			buff->tex_packet.tex0_trans.addr = scrpad->texture ;

			DG_SendCacheFIFO( scrpad->local_work, &buff->mat_packet,
							 SIZEOF_QWORD(MAT_DMA_PACKET) + SIZEOF_QWORD(PACK_DMA_PACKET) +
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
static void ChainObj( DG_OBJ *obj )
{
   ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR;
   DG_OBJS* objs = (DG_OBJS*)obj->BP_DGObjs;
   DG_OBJ_PACKET	*pack = obj->packets;
   int i;

   // Queue BP local param packet
   for ( i = 0; i < obj->n_packs; ++i, ++pack  )
   {
      KP_MatrixData* pMatrixData = (KP_MatrixData*)obj->matrix_addr;

      if( pMatrixData )
      {
         SBP_OBJ_OptCmf_Render* pBPData = (SBP_OBJ_OptCmf_Render*)BP_RB_Alloc(sizeof(SBP_OBJ_OptCmf_Render));
         pBPData->obj_flag = obj->flag;
         pBPData->flag = pack->flag;

        pBPData->correction = obj->world;
         if (obj->parent != -1) 
         {
            _sceVu0MulMatrix( &pBPData->correction, &obj->world, &obj->inv_mat );
         }
         pBPData->world = obj->world;

         //pBPData->eye_pers = pMatrixData->screen;
         //pBPData->correction = pMatrixData->connection;
         //BP_Mat_SetUnit(&pBPData->world);
         pBPData->lightDir = pMatrixData->local_light;
         pBPData->lightCol = scrpad->local_color;

         pBPData->model = obj->BP_Model;
         pBPData->unit = obj->BP_ObjIndex;

         pBPData->startPacket = i;
         pBPData->packetCount = 1;

         if( scrpad->texture == &null_solid_tex )
         {
            pBPData->passFlag = 0;
         }
         else // &frame_buffer_tex[which]
         {
            pBPData->passFlag = 1;
         }

         BP_RB_AddCommand(kCmd_Obj_OptCmf_Render, (char*)pBPData);
      }
   }
}
#endif

static void BP_Obj_OptCmf_LocalParam(DG_OBJS *objs)
{
   //ScrpadWork	*scrpad = SCRPAD_ADDR ;

   SBP_OBJ_OptCmf_LocalParam* pBPData = (SBP_OBJ_OptCmf_LocalParam*)BP_RB_Alloc(sizeof(SBP_OBJ_OptCmf_LocalParam));
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

   BP_RB_AddCommand(kCmd_Obj_OptCmf_LocalParam, (char*)pBPData);
}

static void BP_Obj_Opt_Cmf_InitPacket(DG_CHANL *cp)
{
   SBP_OBJ_OptCmf_InitPacket* pBPData = (SBP_OBJ_OptCmf_InitPacket*)BP_RB_Alloc(sizeof(SBP_OBJ_OptCmf_InitPacket));
   pBPData->eye_pers = cp->eye_pers;
   pBPData->eyeInv = cp->eye_inv;
   BP_RB_AddCommand(kCmd_Obj_OptCmf_InitPacket, (char*)pBPData);
}

static void BP_MSAA_ParticlesPacket(DG_CHANL *cp)
{
   if (!cp->chanl_num)
      BP_RB_AddCommand(kCmd_MSAA_Particles, NULL);
}

static void BP_Obj_OptCmf_PassParam(u_long64 alphaData)
{
   //ScrpadWork	*scrpad = SCRPAD_ADDR ;

   SBP_OBJ_OptCmf_PassParam* pBPData = (SBP_OBJ_OptCmf_PassParam*)BP_RB_Alloc(sizeof(SBP_OBJ_OptCmf_PassParam));
   pBPData->alphaData = alphaData;

   BP_RB_AddCommand(kCmd_Obj_OptCmf_PassParam, (char*)pBPData);
}

/*----------------------------------------------------------------*/
	/*
		ＤＭＡに接続する
	*/
static	void	ChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;

   BP_Obj_OptCmf_LocalParam(objs);

	if ( objs->extend_data == NULL ){
		printf("opt_cmf.c:error\n");
		return ;
	}
	scrpad->local_color = *(FMATRIX*)objs->extend_data ;
	scrpad->rot_mat = ((FMATRIX*)objs->extend_data)[1] ;

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;
		if ( obj->flag & scrpad->invisible_flag ) continue ;

		ChainObj( obj );
	}

}

#if 0 // BP_PS2
/*----------------------------------------------------------------*/

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
static void		DG_OptcmfChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, size, exec_flag = 0, max_objs ;
	int		c_gid, gid ;
	int		last_semi_trans = 0 ;
	OBJ_LIST		*obj_list, *mem_obj_list ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	c_gid = cp->group_id ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->eye_pers = cp->raise_eye_pers ;
	scrpad->eye_inv = cp->eye_inv ;

	/* 描画オブジェクトをスクラッチパッドへ */
	obj_list = (OBJ_LIST*)scrpad->local_work ;
	obj_buff = &que->objs_buffer ;
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		if ( !( objs->flag & ( DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK ) ) ) continue ;
		if ( objs->flag & DG_FLAG_PAINT ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & c_gid ) ) continue ;

		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;
		obj_list[max_objs].objs = objs ;
		max_objs++ ;
	}
	if ( max_objs == 0 ) return ;

	/* スクラッチパッド上のオブジェクトリストをメモリに書き出す */
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
	for ( i = 0 ; i < max_objs ; i++ ) {
		mem_obj_list->objs = obj_list[i].objs ;
		mem_obj_list++ ;
	}
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;


	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache( 0 );

	/* スクラッチパッド上のパケットバッファを初期化 */
	SetupDmaPacket( &scrpad->dma_packet[0], which );
	SetupDmaPacket( &scrpad->dma_packet[1], which );

	/* パケット書き出し用ＦＩＦＯの初期化 */
	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* 初期化パケットの設定 */
	size = DG_WriteObjsPacketInit( &scrpad->dma_buffer[0], cp, 0 );
	/* ＶＵ１マイクロプログラム転送ＤＭＡタグ追加 */
#if 0 //BP_PS2
	scrpad->dma_buffer[size++] = *(u_long128*)Vu1DrawOptcmfObject ;
#endif
	/* ＶＲＡＭの内容待避パケット生成 */
	size += SetVramBackupPacket( &scrpad->dma_buffer[size], which );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );


	/* 描画オブジェクトをスクラッチパッドへ */
	for ( i = max_objs ; i > 0 ; -- i, mem_obj_list++ ) {
		objs = mem_obj_list->objs ;
		if ( !( objs->flag & ( DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK ) ) ) continue ;
		if ( objs->flag & DG_FLAG_PAINT ) continue ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;

		/* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
		if ( !( objs->group_id & c_gid ) ) continue ;

		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;

		/* 強制半透明描画チェック */
		if ( objs->flag & DG_FLAG_SEMITRANS ){
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

		/* 同じモデルをテクスチャを変えて２回連続で描画 */
		scrpad->texture = &null_solid_tex ;
		ChainObjs( objs );
		scrpad->texture = &frame_buffer_tex[which] ;
		ChainObjs( objs );
		exec_flag = 1 ;
	}

	/* 半透明処理無効化 */
	if ( last_semi_trans != 0 ){
		size = DG_WriteRasterMaskPacks( scrpad->dma_buffer, 0 );
		DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
	}

#if 0
	/* フレームバッファ上書き型アンチエイリアス実験（失敗） */
	scrpad->dma_packet[0].pack_packet.datas.giftag_normal.tag = 
	  SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_LINESTRIP, 1, 1, 0, 1, 1, 1, 0, 0),
					  SCE_GIF_PACKED, 3) ;
	scrpad->dma_packet[0].pack_packet.datas.giftag_clip.tag =
	  SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_LINESTRIP, 1, 1, 0, 1, 1, 1, 0, 0),
					  SCE_GIF_PACKED, 3) ;
	scrpad->dma_packet[1].pack_packet.datas.giftag_normal.tag = 
	  SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_LINESTRIP, 1, 1, 0, 1, 1, 1, 0, 0),
					  SCE_GIF_PACKED, 3) ;
	scrpad->dma_packet[1].pack_packet.datas.giftag_clip.tag =
	  SCE_GIF_SET_TAG(0, 1, 1, SCE_GS_SET_PRIM( SCE_GS_PRIM_LINESTRIP, 1, 1, 0, 1, 1, 1, 0, 0),
					  SCE_GIF_PACKED, 3) ;

	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
		objs = *oque ;
		if ( objs->flag & scrpad->invisible_flag ) continue ;
		//if ( !( objs->flag & DG_FLAG_PAINT ) ) continue ;
		if ( !( objs->group_id & c_gid ) ) continue ;
		if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		if ( objs->bound_mode == 2 ) continue ;

		scrpad->texture = &frame_buffer_tex[which] ;
		ChainObjs( objs );

	}
#endif

	/* 終了パケットの設定 */
	size = DG_WriteObjsPacketEnd( scrpad->dma_buffer );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

	DG_CurrentDmaAddr = DG_EndCacheFIFO( scrpad->local_work );

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();
}
#else
#if BP_VITA
static void	DG_OptcmfChainChanl( DG_CHANL *cp, int which )
{
   ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
   DG_OBJS		**oque, *objs ;
   DG_OBJ_QUEUE	*que ;
   DG_OBJ_BUFFER	*obj_buff ;
   int		i, size, exec_flag = 0, max_objs ;
   int		c_gid, gid ;
   OBJ_LIST		*obj_list, *mem_obj_list ;

   que = cp->obj_queue ;
   if ( que == NULL ) goto nocmf ;
   obj_buff = &que->objs_buffer ;

   c_gid = cp->group_id ;
   scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
   scrpad->buffer_switch = 0 ;
   scrpad->eye_pers = cp->raise_eye_pers ;
   scrpad->eye_inv = cp->eye_inv ;

   /* 描画オブジェクトをスクラッチパッドへ */
   obj_list = (OBJ_LIST*)scrpad->local_work ;
   obj_buff = &que->objs_buffer ;
   oque = (DG_OBJS**)obj_buff->queue ;
   for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
      objs = *oque ;
      if ( !( objs->flag & ( DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK ) ) ) continue ;
      if ( objs->flag & DG_FLAG_PAINT ) continue ;
      if ( objs->flag & scrpad->invisible_flag ) continue ;

      /* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
      if ( !( objs->group_id & c_gid ) ) continue ;

      if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
      if ( objs->bound_mode == 2 ) continue ;
      obj_list[max_objs].objs = objs ;
      max_objs++ ;
   }
   if ( max_objs == 0 ) goto nocmf ;

   /* スクラッチパッド上のオブジェクトリストをメモリに書き出す */
   mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
   for ( i = 0 ; i < max_objs ; i++ ) {
      mem_obj_list->objs = obj_list[i].objs ;
      mem_obj_list++ ;
   }
   mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;

   BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   BP_RB_PushRegionMarker(kProfileColor_Camouflage, "Optical Camouflage");

   // Sets up some of the state needed, such as depth writes
   BP_Obj_Opt_Cmf_InitPacket( cp );

   /* 描画オブジェクトをスクラッチパッドへ */
   BP_RB_PushMarker("Optical Camouflage Alpha Pass");
   for ( i = max_objs ; i > 0 ; -- i, mem_obj_list++ ) {
      objs = mem_obj_list->objs;
      // already filtered them above

      /* 同じモデルをテクスチャを変えて２回連続で描画 */
      scrpad->texture = &null_solid_tex ;
      //       DG_SetTexture( 0, NULL );
      //       DG_SetAlphaMode( SCE_GS_SET_ALPHA(2,2,2,1,128) );	
      BP_Obj_OptCmf_PassParam(SCE_GS_SET_ALPHA(2,2,2,1,128));
      ChainObjs( objs );
   }
   BP_RB_PopMarker();

   // switches to the half-res buffer with multisampled Z
   BP_MSAA_ParticlesPacket( cp );

   mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;

   BP_RB_PushMarker("Optical Camouflage Z Pass");
   for ( i = max_objs ; i > 0 ; -- i, mem_obj_list++ ) {
      objs = mem_obj_list->objs;
      scrpad->texture = &frame_buffer_tex[which] ;
      //       DG_SetTextureDirect( 0, DG_SystemTexture[ 2 ].tex_trans.ptex );
      //       DG_SetAlphaMode( SCE_GS_SET_ALPHA(0,1,0,1,0) );
      BP_Obj_OptCmf_PassParam(SCE_GS_SET_ALPHA(0,0,0,0,0));
      ChainObjs( objs );
      exec_flag = 1 ;
   }
   BP_RB_PopMarker();

   mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;

   BP_RB_PushMarker("Optical Camouflage Apply Pass");
   for ( i = max_objs ; i > 0 ; -- i, mem_obj_list++ ) {
      objs = mem_obj_list->objs;
      scrpad->texture = &frame_buffer_tex[which] ;
      //       DG_SetTextureDirect( 0, DG_SystemTexture[ 2 ].tex_trans.ptex );
      //       DG_SetAlphaMode( SCE_GS_SET_ALPHA(0,1,0,1,0) );
      BP_Obj_OptCmf_PassParam(SCE_GS_SET_ALPHA(0,1,0,1,0));
      ChainObjs( objs );
      exec_flag = 1 ;
   }
   BP_RB_PopMarker();

   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();
   return;

   extern int gVTAHalfresAlways;
nocmf:
   if (gVTAHalfresAlways)
      BP_MSAA_ParticlesPacket( cp );
}
#else
static void	DG_OptcmfChainChanl( DG_CHANL *cp, int which )
{
   ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
   DG_OBJS		**oque, *objs ;
   DG_OBJ_QUEUE	*que ;
   DG_OBJ_BUFFER	*obj_buff ;
   int		i, size, exec_flag = 0, max_objs ;
   int		c_gid, gid ;
   OBJ_LIST		*obj_list, *mem_obj_list ;

   que = cp->obj_queue ;
   if ( que == NULL ) return ;
   obj_buff = &que->objs_buffer ;

   c_gid = cp->group_id ;
   scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
   scrpad->buffer_switch = 0 ;
   scrpad->eye_pers = cp->raise_eye_pers ;
   scrpad->eye_inv = cp->eye_inv ;

   /* 描画オブジェクトをスクラッチパッドへ */
   obj_list = (OBJ_LIST*)scrpad->local_work ;
   obj_buff = &que->objs_buffer ;
   oque = (DG_OBJS**)obj_buff->queue ;
   for ( max_objs = 0, i = obj_buff->n_queue ; i > 0 ; -- i, oque++ ) {
      objs = *oque ;
      if ( !( objs->flag & ( DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK ) ) ) continue ;
      if ( objs->flag & DG_FLAG_PAINT ) continue ;
      if ( objs->flag & scrpad->invisible_flag ) continue ;

      /* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
      if ( !( objs->group_id & c_gid ) ) continue ;

      if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
      if ( objs->bound_mode == 2 ) continue ;
      obj_list[max_objs].objs = objs ;
      max_objs++ ;
   }
   if ( max_objs == 0 ) return ;

   /* スクラッチパッド上のオブジェクトリストをメモリに書き出す */
   mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
   for ( i = 0 ; i < max_objs ; i++ ) {
      mem_obj_list->objs = obj_list[i].objs ;
      mem_obj_list++ ;
   }
   mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;

   BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   BP_RB_PushRegionMarker(kProfileColor_Camouflage, "Optical Camouflage");
   BP_Debug_PushCPUMarker( "Optical Camouflage" );
   BP_Obj_Opt_Cmf_InitPacket( cp );

   /* 描画オブジェクトをスクラッチパッドへ */
   for ( i = max_objs ; i > 0 ; -- i, mem_obj_list++ ) {
      objs = mem_obj_list->objs ;
      if ( !( objs->flag & ( DG_FLAG_OPTCMF & DG_FLAG_PLUGINMASK ) ) ) continue ;
      if ( objs->flag & DG_FLAG_PAINT ) continue ;
      if ( objs->flag & scrpad->invisible_flag ) continue ;

      /* CurrentGroupID 実験 （2000/01/13 M.Sonoyama） */
      if ( !( objs->group_id & c_gid ) ) continue ;

      if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
      if ( objs->bound_mode == 2 ) continue ;

      /* 同じモデルをテクスチャを変えて２回連続で描画 */
      scrpad->texture = &null_solid_tex ;
      //       DG_SetTexture( 0, NULL );
      //       DG_SetAlphaMode( SCE_GS_SET_ALPHA(2,2,2,1,128) );	
      BP_RB_PushMarker("Optical Camouflage Pass0");
      BP_Obj_OptCmf_PassParam(SCE_GS_SET_ALPHA(2,2,2,1,128));
      ChainObjs( objs );
      BP_RB_PopMarker();
      scrpad->texture = &frame_buffer_tex[which] ;
      //       DG_SetTextureDirect( 0, DG_SystemTexture[ 2 ].tex_trans.ptex );
      //       DG_SetAlphaMode( SCE_GS_SET_ALPHA(0,1,0,1,0) );
      BP_RB_PushMarker("Optical Camouflage Pass1");
      BP_Obj_OptCmf_PassParam(SCE_GS_SET_ALPHA(0,1,0,1,0));
      ChainObjs( objs );
      BP_RB_PopMarker();
      exec_flag = 1 ;
   }

   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();
}
#endif
#endif

