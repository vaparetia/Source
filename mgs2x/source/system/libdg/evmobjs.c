//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	evmobjs.c
	マルチウェイトエンベロープオブジェクト処理ルーチン

	2000/01/25 K.Takabe
	$Id: evmobjs.c,v 1.1.1.3 2002/11/19 11:42:07 Yoshizawa1 Exp $

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

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

#include "BP_Renderer.h"
#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"

#include "BP_BaseRenderer.h"
#include "BP_Debug.h"

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)
#define SIZEOF_WORD(_v)	(sizeof(_v)/sizeof(int))
#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)

#if 0 //BP_PS2
extern int		Vu1DrawMultiWeight[] ;
extern int		Vu1DrawMultiWeight_Func[] ;
extern int		Vu1DrawMultiWeight3[] ;
extern int		Vu1DrawMultiWeight3_Func[] ;
extern int		Vu1DrawMultiWeight4[] ;
extern int		Vu1DrawMultiWeight4_Func[] ;
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
} Vu1Work /* BP  */;
/* ＶＵ１メモリ初期化パケット */
typedef ALIGN16_DECL(struct) _init_packet {
	DG_DMATAG	trans_wait ;
	DG_DMATAG	init_vu1_prog ;
	DG_DMATAG	init_param_trans ;
	Vu1Work		init_param ;
} INIT_PACKET ;

#if 1 //BP_GCC
static INIT_PACKET DefInitPacket =
{
   //trans_wait
   {
      DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ), //qwc
      NULL, //addr
      //vifcode
      {
         SCE_VIF1_SET_NOP( 0 ),
         SCE_VIF1_SET_FLUSHE( 0 )
      }
   },
   //init_vu1_prog
   {
      0
   },
   //init_param_trans
   {
      DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _vu1_work) ), //qwc
      NULL, //addr
      //vifcode
      {
         SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
         SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(struct _vu1_work), VIF_DATA128, 0 )
      }
   },
   //init_param
   {
      { 0 }, //clamp_min
      { 0 }, //clamp_max
      0, //fog_param1
      0, //fog_param2
      0, //fog_param3
      0, //fog_param4
      255.0f, //color_clip
      128.0f, //poly_alpha
      0.0f, //specular_mul
      0.8f, //specular_clip
      { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f }, //scale
      { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f }, //offset
      0
   },
};

#else
/* ＶＵ１メモリ初期化パケット生成 */
static INIT_PACKET DefInitPacket = {
	.trans_wait = {
		.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ),
		.vifcode = {
			SCE_VIF1_SET_NOP( 0 ),
			SCE_VIF1_SET_FLUSHE( 0 )
		}
	},
	.init_param_trans = {/* 転送用ＤＭＡタグ */
		.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _vu1_work) ),
		.vifcode = {
			SCE_VIF1_SET_STCYCL( 1, 1, 0 ),
			SCE_VIF1_SET_UNPACK( 0x00, SIZEOF_QWORD(struct _vu1_work), VIF_DATA128, 0 )
		}
	},
	.init_param = {/* 転送データ */
		.fog_param1 = 0, .fog_param2 = 0, .fog_param3 = 0, .fog_param4 = 0,
		.color_clip = 255.0f, .poly_alpha = 128.0f, .specular_mul = 0.0f, .specular_clip = 0.8f, 
		.scale = { DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0f },
		.offset = { 2048.0f, 2048.0f, DRAW_Z_OFFSET, 0.0f }
	}
};
#endif

/*----------------------------------------------------------------*/
	/*
		各種パケット定義
	*/

/* ＥＶＭオブジェクト専用ＤＭＡ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _evm_dma_packet {
	DG_DMATAG		verts_trans ;		/* 頂点転送ＤＭＡタグ */
	DG_DMATAG		norms_trans ;		/* 法線転送ＤＭＡタグ */
	DG_DMATAG		uvs0_trans ;		/* ＵＶ０転送ＤＭＡタグ */
	DG_DMATAG		uvs1_trans ;		/* ＵＶ１転送ＤＭＡタグ */
	DG_DMATAG		uvs2_trans ;		/* ＵＶ２転送ＤＭＡタグ */
	DG_DMATAG		weight_trans ;		/* ウェイト値転送ＤＭＡタグ */
	DG_DMATAG		param_trans ;		/* パラメータ転送ＤＭＡタグ */
	struct _trans_param{
		struct _header {
			int				n_verts ;	/* 頂点数 */
			int				work_end ;	/* リザーブ */
			int				n_mat ;		/* 使用最大マトリクス数 */
			int				flag ;		/* フラグ */
			int				bound_mode ;/* バウンディング結果 */
			int				tex0_reglist ;	/* ＴＥＸ０描画用レジスタリスト */
			int				tex1_reglist ;	/* ＴＥＸ１描画用レジスタリスト */
			int				tex2_reglist ;	/* ＴＥＸ２描画用レジスタリスト */
		} header ;
		DG_GIFTAG			clip_tag ;	/* クリッピングポリゴン用ＧＩＦタグ */
		DG_GIFTAG			draw_tag ;	/* 通常描画用ＧＩＦタグ */
	} param ;
} EVM_DMA_PACKET ;

/* テクスチャ転送パケット構造体 */
typedef ALIGN16_DECL(struct) _tex_dma_packet {
	DG_DMATAG		tex0_trans ;		/* テクスチャ０転送ＤＭＡパケット */
	DG_DMATAG		tex1_trans ;		/* テクスチャ１転送ＤＭＡパケット */
	DG_DMATAG		tex2_trans ;		/* テクスチャ２転送ＤＭＡパケット */
} TEX_DMA_PACKET ;

/* マトリクス転送パケット構造体 */
typedef ALIGN16_DECL(struct) _mat_dma_packet {
	DG_DMATAG		skel_mat_trans[8] ;	/* マトリクス転送パケット */
} MAT_DMA_PACKET ;


/* オブジェクト固有設定転送パケット */
typedef ALIGN16_DECL(struct) _param_mat_packet {
	DG_DMATAG	trans_wait ;
	DG_DMATAG	param_mat_trans ;
	struct _param_mat{
		FMATRIX		option_mat[2] ;		/* 将来の拡張用マトリクス */
		FMATRIX		local_light ;		/* ローカルライトマトリクス */
		FMATRIX		local_color ;		/* ローカルカラーマトリクス */
	} param_mat ;
} PARAM_MAT_PACKET ;

/* ＤＭＡ転送データパケット */
typedef ALIGN16_DECL(struct) _packet_buffer {
	EVM_DMA_PACKET		evm_packet ;
	TEX_DMA_PACKET		tex_packet ;
	DG_DMATAG			prog_wait ;			/* プログラム転送終了待ちタグ */
	MAT_DMA_PACKET		mat_packet ;
	DG_DMATAG			prog_exec ;			/* プログラム実行開始タグ */
} PACKET_BUFFER ;



/*----------------------------------------------------------------*/
	/*
		スクラッチパッド上ワークデータ構造体
	*/
typedef struct _obj_list {
	void	*obj ;
	int		tri_code ;
} OBJ_LIST ;

typedef struct evmobjs_scrpad_work {
	FMATRIX		pers_mat ;
	FMATRIX		eye_pers ;
	FMATRIX		eye_inv ;
	FMATRIX		eye ;
	FVECTOR		scale ;
	FMATRIX		world ;				/* オブジェクトマトリクス */
	FMATRIX		screen ;			/* カメラへの透視変換用マトリクス */
	FMATRIX		light[2] ;			/* ライトマトリクス */
	FMATRIX		support_mat[2] ;	/* 拡張用 */
	FMATRIX		tmp_mat ;			/* 一時使用マトリクス */
	FVECTOR		tmp_vec[4] ;		/* 一時使用ベクトル */
	PARAM_MAT_PACKET	param_packet ;		/* オブジェクト固有マトリクスパラメータパケット */
	PACKET_BUFFER		dma_packet[2] ;		/* ＤＭＡパケット（ダブルバッファ） */
	u_long128			dma_buffer[64] ;	/* 汎用パケット生成エリア */
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
extern char	*DG_AS_GetLocalMemoryAddress();

//#define DG_StartSprToMem	UTL_StartSprToMem
//#define DG_EndSprToMem	UTL_EndSprToMem
/*----------------------------------------------------------------*/
	/*
		各種サブルーチン
	*/

/* バウンディングチェック */
#if 1 //BP - Using the original version of the XBox function.
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

#else
static int BoundCheck( FMATRIX *mat, float *bound )
{
	int		and_flag, or_flag, prim_over_flag ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

	/* バウンディング用ベクトル生成 */
	scrpad->tmp_vec[0].vx = bound[0] ; scrpad->tmp_vec[0].vy = bound[1] ; scrpad->tmp_vec[0].vz = bound[2] ;
	scrpad->tmp_vec[1].vx = bound[3] ; scrpad->tmp_vec[1].vy = bound[4] ; scrpad->tmp_vec[1].vz = bound[5] ;
	/* バウンディングチェックマイクロプログラム呼び出し */
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf1,0x00(%1)
	lqc2		vf2,0x10(%1)
	lqc2		vf3,0(%2)
	vcallms		0x00			# バウンディングチェック
	"::"r"(mat),"r"(scrpad->tmp_vec),"r"(&scrpad->scale) );
	/* 計算結果の取得 */
	asm ("
	cfc2.i		%0,$2 /*vi2*/
	cfc2		%1,$3 /*vi3*/
	cfc2		%2,$1 /*vi1*/
	sqc2		vf31,0(%3)
	":"=&r"(and_flag),"=&r"(or_flag),"=&r"(prim_over_flag):"r"(scrpad->tmp_vec) );
	/* バウンディング中央値のＺ座標取得 */
	//SORT_Z = DG_FTOI(BOUNDS[0].vw) ;
	/* バウンディング判定 */
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
	if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	return ( 0 );	/* 完全画面内 */
}
#endif

#if 0 //BP_ASM - unused by C version of MakeMaxBound
static void SetBoundVertex( FVECTOR *bound )
{
	/* vf8 ~ vf15 に頂点を格納 */
	asm volatile ("
		lqc2		vf4,0x00(%0)
		lqc2		vf5,0x10(%0)
		vmove.xyz	vf8,vf4
		vmove.yz	vf9,vf4
		vmove.xz	vf10,vf4
		vmove.z		vf11,vf4
		vmove.xy	vf12,vf4
		vmove.y		vf13,vf4
		vmove.x		vf14,vf4
		vadd.x		vf9,vf0,vf5
		vadd.y		vf10,vf0,vf5
		vadd.xy		vf11,vf0,vf5
		vadd.z		vf12,vf0,vf5
		vadd.xz		vf13,vf0,vf5
		vadd.yz		vf14,vf0,vf5
		vadd.xyz	vf15,vf0,vf5
	"::"r"(bound) );
}
#endif

#if 1 //BP - Using XBox version of MakeMaxBound.
FVECTOR bound_max, bound_min ;

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


#else
/* 各マトリクスでのバウンディングから最大バウンディングを求める */
static void MakeMaxBound( FMATRIX *mat, EVM_SKEL *skel, int num )
{
	static FVECTOR	max = {-FLOAT_MAX,-FLOAT_MAX,-FLOAT_MAX,1.0f}, min = {FLOAT_MAX,FLOAT_MAX,FLOAT_MAX,1.0f};
	asm volatile ("
		lqc2		vf1,0x00(%0)
		lqc2		vf2,0x00(%1)
	"::"r"(&min),"r"(&max) );
	while ( num-- ){
		SetBoundVertex( (FVECTOR*)&skel->lx );
		asm volatile ("
			lqc2			vf4,0x00(%0)	; 
			lqc2			vf5,0x10(%0)	; 
			lqc2			vf6,0x20(%0)	; 
			lqc2			vf7,0x30(%0)	; 
			vmulax.xyzw		ACC,vf4,vf8		; 
			vmadday.xyzw	ACC,vf5,vf8		; 
			vmaddaz.xyzw	ACC,vf6,vf8		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmulax.xyzw		ACC,vf4,vf9		; 
			vmadday.xyzw	ACC,vf5,vf9		; 
			vmaddaz.xyzw	ACC,vf6,vf9		; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmulax.xyzw		ACC,vf4,vf10	; 
			vmadday.xyzw	ACC,vf5,vf10	; 
			vmaddaz.xyzw	ACC,vf6,vf10	; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmulax.xyzw		ACC,vf4,vf11	; 
			vmadday.xyzw	ACC,vf5,vf11	; 
			vmaddaz.xyzw	ACC,vf6,vf11	; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmulax.xyzw		ACC,vf4,vf12	; 
			vmadday.xyzw	ACC,vf5,vf12	; 
			vmaddaz.xyzw	ACC,vf6,vf12	; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmulax.xyzw		ACC,vf4,vf13	; 
			vmadday.xyzw	ACC,vf5,vf13	; 
			vmaddaz.xyzw	ACC,vf6,vf13	; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmulax.xyzw		ACC,vf4,vf14	; 
			vmadday.xyzw	ACC,vf5,vf14	; 
			vmaddaz.xyzw	ACC,vf6,vf14	; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmulax.xyzw		ACC,vf4,vf15	; 
			vmadday.xyzw	ACC,vf5,vf15	; 
			vmaddaz.xyzw	ACC,vf6,vf15	; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
			vmaddw.xyzw		vf3,vf7,vf0		; 
			vmini.xyzw		vf1,vf1,vf3		; 
			vmax.xyzw		vf2,vf2,vf3		; 
		"::"r"(mat) );
		mat++ ;
		skel++ ;
	}
}
#endif

#if 1 //BP - Using the XBox version of BoundCheck2.
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
#else
static int BoundCheck2( FMATRIX *mat )
{
   int		and_flag, or_flag, prim_over_flag ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

	/* バウンディングチェックマイクロプログラム呼び出し */
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf3,0(%1)
	vcallms		0x00			# バウンディングチェック
	"::"r"(mat),"r"(&scrpad->scale) );
	/* 計算結果の取得 */
	asm ("
	cfc2.i		%0,$2 /*vi2*/
	cfc2		%1,$3 /*vi3*/
	cfc2		%2,$1 /*vi1*/
	sqc2		vf31,0(%3)
	":"=&r"(and_flag),"=&r"(or_flag),"=&r"(prim_over_flag):"r"(scrpad->tmp_vec) );

	/* バウンディング中央値のＺ座標取得 */
	//SORT_Z = DG_FTOI(BOUNDS[0].vw) ;
	/* バウンディング判定 */
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
	if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */

	return ( 0 );	/* 完全画面内 */

}
#endif

/*----------------------------------------------------------------*/
	/*
		パケット初期化関連
	*/

/* デフォルトパケット初期化ルーチン */
static void SetupDmaPacket( PACKET_BUFFER *buff )
{
	u_long64		prim ;
	int			i ;

	buff->evm_packet.verts_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	buff->evm_packet.verts_trans.addr = NULL ;
	buff->evm_packet.verts_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 7, 0 ) ;
	buff->evm_packet.verts_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100, 0, VIF_VERT_PACK, 0 ) ;
	buff->evm_packet.norms_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	buff->evm_packet.norms_trans.addr = NULL ;
	buff->evm_packet.norms_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 7, 0 ) ;
	buff->evm_packet.norms_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x101, 0, VIF_VERT_PACK, 0 ) ;
	buff->evm_packet.uvs0_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	buff->evm_packet.uvs0_trans.addr = NULL ;
	buff->evm_packet.uvs0_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 7, 0 ) ;
	buff->evm_packet.uvs0_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x102, 0, VIF_VERT_PACK, 0 ) ;
	buff->evm_packet.uvs1_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	buff->evm_packet.uvs1_trans.addr = NULL ;
	buff->evm_packet.uvs1_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 7, 0 ) ;
	buff->evm_packet.uvs1_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x103, 0, VIF_VERT_PACK, 0 ) ;
	buff->evm_packet.uvs2_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	buff->evm_packet.uvs2_trans.addr = NULL ;
	buff->evm_packet.uvs2_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 7, 0 ) ;
	buff->evm_packet.uvs2_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x104, 0, VIF_VERT_PACK, 0 ) ;
	buff->evm_packet.weight_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, 0 );
	buff->evm_packet.weight_trans.addr = NULL ;
	buff->evm_packet.weight_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 2, 7, 0 ) ;
	buff->evm_packet.weight_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x4105, 0, VIF_RGBA_PACK, 0 ) ;
	buff->evm_packet.param_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _trans_param) );
	buff->evm_packet.param_trans.addr = NULL ;
	buff->evm_packet.param_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	buff->evm_packet.param_trans.vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x00, SIZEOF_QWORD(struct _trans_param), VIF_DATA128, 0 ) ;
	/* 転送パラメータ設定 */
	buff->evm_packet.param.header.n_verts = 0 ;
	buff->evm_packet.param.header.work_end = 5 ;
	buff->evm_packet.param.header.n_mat = 0 ;
	buff->evm_packet.param.header.flag = 0 ;
	buff->evm_packet.param.header.tex0_reglist = 
	  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) ;
	buff->evm_packet.param.header.tex1_reglist =
	  GS_REGS_5( GS_REGS_NOP, GS_REGS_STQ, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) ;
	buff->evm_packet.param.header.tex2_reglist =
	  GS_REGS_5( GS_REGS_NOP, GS_REGS_NOP, GS_REGS_STQ, GS_REGS_RGBA, GS_REGS_XYZF2) ;
	//buff->evm_packet.param.header.pad = 0 ;
	buff->evm_packet.param.header.bound_mode = 0 ;

	prim = SCE_GS_SET_PRIM( 0, 1, 1, 1, 1, 0, 0, 0, 0) ;
	buff->evm_packet.param.clip_tag.tag = 
	  SCE_GIF_SET_TAG( 0, 1, 1, prim | SCE_GS_PRIM_TRIFAN, SCE_GIF_PACKED, 5) ;
	buff->evm_packet.param.clip_tag.regs = 
	  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) ;
	buff->evm_packet.param.draw_tag.tag =
	  SCE_GIF_SET_TAG( 0, 1, 1, prim | SCE_GS_PRIM_TRISTRIP, SCE_GIF_PACKED, 5) ;
	buff->evm_packet.param.draw_tag.regs = 
	  GS_REGS_5( GS_REGS_STQ, GS_REGS_NOP, GS_REGS_NOP, GS_REGS_RGBA, GS_REGS_XYZF2) ;

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

	for ( i = 0 ; i < 8 ; i++ ){
		buff->mat_packet.skel_mat_trans[i].qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(FMATRIX) );
		buff->mat_packet.skel_mat_trans[i].addr = NULL ;
		buff->mat_packet.skel_mat_trans[i].vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
		buff->mat_packet.skel_mat_trans[i].vifcode[1] = SCE_VIF1_SET_UNPACK( 0x20+i*4, SIZEOF_QWORD(FMATRIX), VIF_DATA128, 0 ) ;
	}

	buff->prog_wait.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	buff->prog_wait.addr = NULL ;
	buff->prog_wait.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	buff->prog_wait.vifcode[1] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	buff->prog_exec.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	buff->prog_exec.addr = NULL ;
	buff->prog_exec.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	buff->prog_exec.vifcode[1] = SCE_VIF1_SET_MSCAL( 0 ,0 ) ;
	//buff->prog_exec.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;

}

/* 初期化用パケットの設定*/
static void SetEvmInitPacket( INIT_PACKET *packet, DG_CHANL *cp )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

#if 0 //BP_PS2
	*packet = DefInitPacket ;
	*(u_long128*)&packet->init_vu1_prog = *(u_long128*)Vu1DrawMultiWeight4 ;
	packet->init_param.pers = scrpad->eye_pers ;
	packet->init_param.scale.vx = cp->width / 2 ;
	packet->init_param.scale.vy = cp->height / 2 ;
	packet->init_param.aspect.vx = cp->screen * ASPECT_X() ;
	packet->init_param.aspect.vy = cp->screen * ASPECT_Y() * cp->width / cp->height ;
	packet->init_param.fog_param1 = DG_FogParam1 ;
	packet->init_param.fog_param2 = DG_FogParam2 ;
#else
   SBP_EVM_InitPacket* pBPData = (SBP_EVM_InitPacket*)BP_RB_Alloc(sizeof(SBP_EVM_InitPacket));
   pBPData->eye_pers = scrpad->eye_pers;
   pBPData->eye_inv = scrpad->eye_inv;
   pBPData->fogParam = BP_FogParam;
   BP_RB_AddCommand(kCmd_Evm_InitPacket, (char*)pBPData);
#endif
}

/* オブジェクト固有設定マトリクス転送の設定*/
static void SetMatrixInitPacket( DG_EVMOBJ *evmobj, PARAM_MAT_PACKET *packet )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

#if 0 //BP_PS2
	packet->trans_wait.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	packet->trans_wait.addr = NULL ;
	packet->trans_wait.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->trans_wait.vifcode[1] = SCE_VIF1_SET_FLUSHE( 0 ) ;
	packet->param_mat_trans.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _param_mat) );
	packet->param_mat_trans.addr = NULL ;
	packet->param_mat_trans.vifcode[0] = SCE_VIF1_SET_STCYCL( 1, 1, 0 ) ;
	packet->param_mat_trans.vifcode[1] = SCE_VIF1_SET_UNPACK( 0x10, SIZEOF_QWORD(struct _param_mat), VIF_DATA128, 0 ) ;
	//packet->param_mat.option_mat[0] = DG_UnitMatrix ;
	//packet->param_mat.option_mat[1] = DG_UnitMatrix ;
	packet->param_mat.local_light = scrpad->light[0] ;
	packet->param_mat.local_color = scrpad->light[1] ;
	packet->param_mat.option_mat[0] = scrpad->eye_inv ;
	packet->param_mat.option_mat[1].m[0][0] = -1.0f ;
	packet->param_mat.option_mat[1].m[1][0] = 0.0f ;
	packet->param_mat.option_mat[1].m[2][0] = 0.0f ;
	packet->param_mat.option_mat[1].m[0][1] = 0.0f ;
	packet->param_mat.option_mat[1].m[1][1] = 1.0f ;
	packet->param_mat.option_mat[1].m[2][1] = 0.0f ;
	packet->param_mat.option_mat[1].m[0][2] = 0.0f ;
	packet->param_mat.option_mat[1].m[1][2] = 0.0f ;
	packet->param_mat.option_mat[1].m[2][2] = 0.0f ;

	packet->param_mat.option_mat[1].m[0][0] = 0.0f ;
	packet->param_mat.option_mat[1].m[1][1] = 0.0f ;
#else
   SBP_EVM_LocalParam* pBPData = (SBP_EVM_LocalParam*)BP_RB_Alloc(sizeof(SBP_EVM_LocalParam));
   pBPData->model = (unsigned int)evmobj->BP_Model;
   // copy the matrices to avoid pointing to trampled memory
   pBPData->matrix = (FMATRIX *)BP_RB_Alloc(evmobj->n_skeleton*sizeof(FMATRIX));
   memcpy(pBPData->matrix, evmobj->matrix[evmobj->use_buffer], evmobj->n_skeleton*sizeof(FMATRIX));
   pBPData->lightDir = scrpad->light[0];
   pBPData->lightCol = scrpad->light[1];
   pBPData->isInIRMode = 0;

   pBPData->specialModel = evmobj->BP_SpecialModelFlag;
   pBPData->specialModelValue = evmobj->BP_SpecialModelValue;

   if ( DG_DisplayStatus & DG_STATE_IR_MODE )
   {
      if ( evmobj->flag & DG_EVMOBJ_IRREACTION )
      {
         {/* ＸＢＯＸ方式互換設定 */
            static FMATRIX	ir_light[2] = {
               {{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
               {{{255,255,255,0},{0,0,0,0},{0,0,0,0},{128,128,128,128}}}
            };
            pBPData->lightDir = ir_light[0] ;
            pBPData->lightCol = ir_light[1] ;
            /* かならずカメラ方向から光が当たっている状態に */
            pBPData->lightDir.m[0][0] = scrpad->eye.m[2][0] ;
            pBPData->lightDir.m[1][0] = scrpad->eye.m[2][1] ;
            pBPData->lightDir.m[2][0] = scrpad->eye.m[2][2] ;
         }
         pBPData->isInIRMode = 1;
      } 
   }

   BP_RB_AddCommand(kCmd_Evm_LocalParam, (char*)pBPData);
#endif
}

/* 頂点データ転送の設定 */
static inline u_long128* SetPacketParam1( DG_DMATAG *tag, u_long128 *addr, int offset, int n_step )
{
	if ( addr != NULL && n_step != 0 ){
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n_step );
		tag->addr = addr ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x100 + offset, n_step * 2, VIF_VERT_PACK, 0 ) ;
		return ( addr + n_step );
	} else {
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
		tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
		return ( addr );
	}
}
/* 頂点データ転送の設定 */
static inline u_long128* SetPacketParam4( DG_DMATAG *tag, u_long128 *addr, int offset, int n_step )
{
	if ( addr != NULL ){
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, n_step );
		tag->addr = addr ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0x4100 + offset, n_step * 4, VIF_RGBA_PACK, 0 ) ;
		return ( addr + n_step );
	} else {
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
		tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
		return ( addr );
	}
}
/* テクスチャ設定転送の設定 */
static inline void SetPacketParam2( DG_DMATAG *tag, void *tex_trans, int offset )
{
	if ( tex_trans != NULL && tag->addr != tex_trans ){
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_TEX_TRANS) );
		tag->addr = tex_trans ;
		tag->vifcode[1] = SCE_VIF1_SET_UNPACKR( 0xe8 + offset*8, SIZEOF_QWORD(DG_TEX_TRANS), VIF_DATA128, 0 ) ;
	} else {
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
		tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	}
}
/* マトリクス転送の設定 */
static inline int SetPacketParam3( DG_DMATAG *tag, u_long64 matrix_list, FMATRIX *mat )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	int		i, num, matrix_trans_flag = 0 ;
	void		**last_matrix_addr, *addr ;

	/* 前回と違う場合のみ転送するようにする */
	last_matrix_addr = scrpad->last_matrix_addr ;
	for ( i = 0 ; i < 8 ; i++ ){
		num = matrix_list & 0xff ;
		addr = mat + num ;
		if ( num != 0xff && (*last_matrix_addr) != addr ){
			/* マトリクスアドレスが異なる場合にはマトリクスの転送を行なう */
			tag->qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(FMATRIX) );
			tag->addr = addr ;
			tag->vifcode[1] = SCE_VIF1_SET_UNPACK( 0x20+i*4, SIZEOF_QWORD(FMATRIX), VIF_DATA128, 0 ) ;
			*last_matrix_addr = addr ;
			matrix_trans_flag = 1 ;
		} else {
			/* マトリクスが同じ又は未使用の場合は転送を行なわない */
			tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
			tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 );
		}
		matrix_list >>= 8 ;
		last_matrix_addr++ ;
		tag++ ;
	}
	return ( matrix_trans_flag );
}

/*----------------------------------------------------------------*/
/* 各種セットアップルーチン */
static void EvmobjSetup( DG_EVMOBJ *evmobj )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

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

	/* 光源計算 */
	//SetLightMatrix( &scrpad->light[0], evmobj->light, &scrpad->world );
	scrpad->light[0] = evmobj->light[0] ;
	scrpad->light[1] = evmobj->light[1] ;
	/* 赤外線反応 */
	if ( DG_DisplayStatus & DG_STATE_IR_MODE ){
		u_long64		prim ;
		if ( evmobj->flag & DG_EVMOBJ_IRREACTION ){
			{/* ＸＢＯＸ方式互換設定 */
				static FMATRIX	ir_light[2] = {
					{{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
					{{{255,255,255,0},{0,0,0,0},{0,0,0,0},{128,128,128,128}}}
				};
				scrpad->light[0] = ir_light[0] ;
				scrpad->light[1] = ir_light[1] ;
				/* かならずカメラ方向から光が当たっている状態に */
				scrpad->light[0].m[0][0] = scrpad->eye.m[2][0] ;
				scrpad->light[0].m[1][0] = scrpad->eye.m[2][1] ;
				scrpad->light[0].m[2][0] = scrpad->eye.m[2][2] ;
			}

			prim = SCE_GS_SET_PRIM( 0, 1, 0, 1, 0, 0, 0, 0, 0) ;
		} else {
			prim = SCE_GS_SET_PRIM( 0, 1, 1, 1, 1, 0, 0, 0, 0) ;
		}
		scrpad->dma_packet[0].evm_packet.param.clip_tag.tag =
		  SCE_GIF_SET_TAG( 0, 1, 1, prim | SCE_GS_PRIM_TRIFAN, SCE_GIF_PACKED, 5) ;
		scrpad->dma_packet[0].evm_packet.param.draw_tag.tag =
		  SCE_GIF_SET_TAG( 0, 1, 1, prim | SCE_GS_PRIM_TRISTRIP, SCE_GIF_PACKED, 5) ;
		scrpad->dma_packet[1].evm_packet.param.clip_tag.tag =
		  SCE_GIF_SET_TAG( 0, 1, 1, prim | SCE_GS_PRIM_TRIFAN, SCE_GIF_PACKED, 5) ;
		scrpad->dma_packet[1].evm_packet.param.draw_tag.tag =
		  SCE_GIF_SET_TAG( 0, 1, 1, prim | SCE_GS_PRIM_TRISTRIP, SCE_GIF_PACKED, 5) ;
	}
}
#if 0 //BP_PS2
/* ＤＭＡ接続ルーチン  */
static void ChainEvmObj( DG_EVMOBJ *evmobj )
{
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
}
#else
static void ChainEvmObj( DG_EVMOBJ *evmobj )
{
   ScrpadWork	*scrpad = SCRPAD_ADDR ;
   DG_EVMPACK	*pack ;
   int i;

   /* マトリクス転送セットアップ */
   SetMatrixInitPacket( evmobj, &scrpad->param_packet );

   pack = evmobj->packs;

   {
      unsigned int prev_tex0 = ( (unsigned int) pack->tex0_ptr ) ^ 0xFFFFFFFF;
      unsigned int prev_tex1 = ( (unsigned int) pack->tex1_ptr ) ^ 0xFFFFFFFF;
      unsigned int prev_tex2 = ( (unsigned int) pack->tex2_ptr ) ^ 0xFFFFFFFF;
      unsigned int prev_flag = ( (unsigned int) pack->flag ) ^ 0xFFFFFFFF;
      SBP_EVM_Render *pBPData = NULL;

      for ( i = 0; i < evmobj->n_packet; i++, pack++ )
      {
         unsigned int tex0 = (unsigned int)pack->tex0_ptr;
         unsigned int tex1 = (unsigned int)pack->tex1_ptr;
         unsigned int tex2 = (unsigned int)pack->tex2_ptr;
         unsigned int flag = pack->flag;

         unsigned int diffs = 
            ( tex0 ^ prev_tex0 ) |
            ( tex1 ^ prev_tex1 ) |
            ( tex2 ^ prev_tex2 ) |
            ( flag ^ prev_flag );

         prev_tex0 = tex0;
         prev_tex1 = tex1;
         prev_tex2 = tex2;
         prev_flag = flag;

         if ( diffs )
         {
            int const isMultiTexture = tex1 | tex2;

            pBPData = (SBP_EVM_Render *) BP_RB_Alloc( sizeof( SBP_EVM_Render ) );

            pBPData->flag = flag;
            BP_RB_CopyTexture(&pBPData->tex[0], tex0);
            BP_RB_CopyTexture(&pBPData->tex[1], tex1);
            BP_RB_CopyTexture(&pBPData->tex[2], tex2);

            pBPData->model = evmobj->BP_Model;
            pBPData->startPacket = i;
            pBPData->packetCount = 1;

            BP_RB_AddCommand(isMultiTexture ? kCmd_Evm_RenderMulti : kCmd_Evm_Render, (char*)pBPData);
         }
         else
         {
            ++pBPData->packetCount;
         }
      }
   }
}
#endif
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
	int		old_tex_code, size, last_semi_trans = 0 ;

	if ( cp->obj_queue == NULL ) return ;
	obj_buff = &cp->obj_queue->evmobj_buffer ;

	c_gid = cp->group_id ;

	/* スクラッチパッドワークの初期化 */
	scrpad->pers_mat = cp->pers ;
	scrpad->eye_inv = cp->eye_inv ;
	scrpad->eye = cp->eye ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->scale.vx = cp->width / 2 ;
	scrpad->scale.vy = cp->height / 2 ;
	scrpad->packet_clock = 0 ;
	scrpad->count = 0 ;
#ifdef LIBDG_PERFORMANCE
	scrpad->mulwt_model = DG_PerformanceData.mulwt_model ;
#endif
#if 0 //BP_PS2
	for ( i = 0 ; i < 8 ; i++ ){
		scrpad->vu_prog_list[i] = Vu1DrawMultiWeight4_Func[i] ;
	}
#endif
	/* ベースパケットの初期化 */
	SetupDmaPacket( &scrpad->dma_packet[0] );
	SetupDmaPacket( &scrpad->dma_packet[1] );

	/* ＶＵ０マイクロプログラム準備 */
	DG_StartBoundingCheckSupport();

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
#if BP_ENABLE_TESTNODE
      if ( !BP_Debug_EvalTestNode() ) continue;
#endif
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

   //if( !(cp->flag & DG_VIEWPORT_STAGE_DISABLE_MSAA) )
   if( 1 )
   {
      BP_GetCurrentViewportInfo()->hasMSAAStuff = 1;
   }
   else
   {
      BP_GetCurrentViewportInfo()->hasNonMSAAStuff = 1;
   }

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
	FlushCache(0);	/* キャッシュのフラッシュ */

	DG_InitCacheFIFO( scrpad->local_work, DG_CurrentDmaAddr, DG_CurrentDmaEnd );

	/* 初期化パケットの設定 */
	SetEvmInitPacket( (void*)&scrpad->dma_buffer, cp );
	DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, SIZEOF_QWORD(INIT_PACKET) );

	/* ソート済みオブジェクトをＤＭＡにつなぐ */
	old_tex_code = -1 ;
	mem_obj_list = (OBJ_LIST*)DG_AS_GetLocalMemoryAddress() ;
	for ( i = max_mem_objs ; i > 0 ; i--, mem_obj_list++ ){
		evmobj = mem_obj_list->obj ;

		/* マトリクス設定 */
		if ( evmobj->root != NULL ) evmobj->world = *( evmobj->root ) ;
		scrpad->world = evmobj->world ;

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
			//size = DG_WriteTextureChangePacks( scrpad->dma_buffer, &tex_list->tex_packet[which] );
			size = DG_WriteTextureChangePacks2( scrpad->dma_buffer, tex_list, which );
			DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
			old_tex_code = mem_obj_list->tri_code ;
			tex_list++ ;
		}
		/* 強制半透明描画チェック */
		if ( evmobj->flag & DG_EVMOBJ_SEMITRANS ){
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

		/* マトリクスセットアップ */
		EvmobjSetup( evmobj );

			/* 描画 */
#ifdef LIBDG_PERFORMANCE
		scrpad->mulwt_model.n_obj++ ;
#endif
		ChainEvmObj( evmobj );
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
   BP_RB_PushRegionMarker(kProfileColor_Model, "ChainEvm");
   BP_Debug_PushCPUMarker( "ChainEvm" );
	_DG_ChainEvmChanl( cp, which, 0 );
   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();
}

void DG_ChainEvmChanlLatter( DG_CHANL *cp, int which )
{
   BP_RB_PushRegionMarker(kProfileColor_Model_After, "ChainEvmLatter");
   BP_Debug_PushCPUMarker( "ChainEvmLatter" );
	_DG_ChainEvmChanl( cp, which, 1 );
   BP_Debug_PopCPUMarker();
   BP_RB_PopRegionMarker();
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
   evmobj->BP_Model = BP_GetMeshByMGSAddr((unsigned int)def);
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
	evmobj->skel_bounds = (FVECTOR*)def->packet[0].pad1 ;

	evmobj->verts_ptr = def->packet[0].verts ;
	evmobj->norms_ptr = def->packet[0].norms ;
	evmobj->uvs0_ptr = def->packet[0].uvs[0] ;
	evmobj->uvs1_ptr = def->packet[0].uvs[1] ;
	evmobj->uvs2_ptr = def->packet[0].uvs[2] ;
	evmobj->weight_ptr = def->packet[0].weight ;

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
			evmpack->tex0_ptr = (DG_TEX*)pack->tex_id[0];
		}
		if ( pack->tex_id[1] != 0 ){
			evmpack->tex1_ptr = (DG_TEX*)pack->tex_id[1];
		}
		if ( pack->tex_id[2] != 0 ){
			evmpack->tex2_ptr = (DG_TEX*)pack->tex_id[2];
		}

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
				uv[0] = (short)( (float)uv[0] * u_scale + u_offset );
				uv[1] = (short)( (float)uv[1] * v_scale + v_offset );
				uv += 4 ;
			}
		}
		pack->tex_id[i] = (int)tex ;
	}

}

/*----------------------------------------------------------------*/

void DG_Arm_SwitchEvmBuffer(DG_EVMOBJ *evmobj)
{
   if (DG_Arm_SkipThisFrame())
   {
      evmobj->use_buffer = 1 - evmobj->use_buffer;
   }
}
