/*
	prim.c
	チャンネル処理ユニット／プリミティブ処理ルーチン

	1999/07/07 K.Takabe
	$Id: prim.c,v 1.1.1.3 2002/11/19 11:42:20 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／プリミティブ処理ルーチン


	void		DG_PrimChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされたプリミティブのパケット作成などを行なう

	------------------------------------------------

	DG_PRIM *DG_MakePrim( int type, int n_packet, int n_prims, int chanl, FVECTOR *pos, FVECTOR *vec );
	int		type ;		プリミティブタイプ
	int		n_packet ;	パケット数（パケット＝ソートの基準になる塊）
	int		n_prims ;	１パケットに含まれるプリミティブ数
	int		chanl ;		チャンネル番号
	FVECTOR		*pos ;		座標計算バッファ
	FVECTOR		*uvs ;		ＵＶ座標データ又は矩形サイズ指定データ

		プリミティブを確保し、ＤＭＡタグ等の必要最小限の初期化を行う
		物体ハンドラと違い、パケットメモリは生成時に確保される
		プリセットプリミティブの初期化は、直後にユーザーがおこなうこと

		座標計算バッファは、頂点数×ポリゴン数だけ必要
		ＵＶ座標データはテクスチャ補正タイプのプリミティブを使用した
		場合に必要で、１ベクトルで２頂点分のＵＶを指定する
		また、DG_PRIM_SPRT3D2使用時にはスプライト矩形サイズ指定としても
		使用する（１ベクトルで２個分のスプライトサイズを指定する）。

	void		DG_FreePrim( prim )
	DG_PRIM		*prim ;

		プリミティブを破棄する（パケットメモリも解放）

	void DG_ConfigPrimInitTex( DG_PRIM_INIT *prim_init, DG_TEX *tex )
	DG_PRIM_INIT	*prim_init ;	設定するプリミティブ初期化パケット
	DG_TEX			*tex ;			設定するテクスチャ構造体

		プリミティブの初期化ＧＩＦパケットにテクスチャを設定する

	void DG_ConfigPrimTex( DG_PRIM *prim, DG_TEX *tex )
	DG_PRIM		*prim ;	設定するプリミティブ
	DG_TEX		*tex ;	設定するテクスチャ構造体

		プリミティブのテクスチャを設定する


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

#define __PRIM_C__
#include	"libdg.h"
#undef __PRIM_C__
#include	"libdg.cnf"

#include	"def_dma.h"
#include	"utl_dma.h"

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)


typedef struct {
	int		prim_size ;		/* １パケット当たりのサイズ（qword単位、タグ除く） */
	int		prim_nverts ;	/* パケット当たりの頂点数 */
	int		prim_voffset ;	/* 先頭頂点までのオフセット（qword単位、タグ除く） */
	int		prim_vstep ;	/* 次頂点までのオフセット */
	int		stq_flag ;		/* ＵＶ補正書き込みフラグ */
	sceGifTag	*giftag ;
} Param ;

	/*
		プリミティブ初期化パラメータ
	*/
static Param	Params[] = {
	{ 3,2,1,1,0,&DG_GIFTAG_LINE_R2},	/* LINE_R2 */
	{ 4,2,1,2,0,&DG_GIFTAG_LINE_G2},	/* LINE_G2 */
	{ 6,2,2,3,0,&DG_GIFTAG_LINE_T2},	/* LINE_T2 */
	{ 6,2,2,3,0,&DG_GIFTAG_LINE_GT2},	/* LINE_GT2 */
	{ 6,2,2,3,1,&DG_GIFTAG_LINE_GT2},	/* LINE_GTQ2 */
	{ 4,3,1,1,0,&DG_GIFTAG_POLY_R3},	/* POLY_R3 */
	{ 6,3,1,2,0,&DG_GIFTAG_POLY_G3},	/* POLY_G3 */
	{ 9,3,2,3,0,&DG_GIFTAG_POLY_T3},	/* POLY_T3 */
	{ 9,3,2,3,0,&DG_GIFTAG_POLY_GT3},	/* POLY_GT3 */
	{ 9,3,2,3,1,&DG_GIFTAG_POLY_GT3},	/* POLY_GTQ3 */
	{ 6,4,2,1,0,&DG_GIFTAG_POLY_R4},	/* POLY_R4 */
	{ 9,4,2,2,0,&DG_GIFTAG_POLY_G4},	/* POLY_G4 */
	{13,4,3,3,0,&DG_GIFTAG_POLY_T4},	/* POLY_T4 */
	{13,4,3,3,0,&DG_GIFTAG_POLY_GT4},	/* POLY_GT4 */
	{13,4,3,3,1,&DG_GIFTAG_POLY_GT4},	/* POLY_GTQ4 */
	{ 6,2,2,3,0,&DG_GIFTAG_SPRITE},		/* SPRT3D1 */
	{ 6,2,2,3,1,&DG_GIFTAG_SPRITE},		/* SPRT3D2 */
	{ 3,2,3,0,0,&DG_GIFTAG_MENU_SPRITE},/* MENU_SPRT */
	{ 2,2,2,0,0,&DG_GIFTAG_MENU_LINE},/* MENU_LINE */
	{ 3,2,2,0,0,&DG_GIFTAG_MENU_LINE_G},/* MENU_LINE_G */
};

static DG_PRIM_INIT	prim_init_data = {
	{ sizeof(DG_PRIM_INIT)/16-1,0,0/*pad*/,0,0,0,SCE_GIF_PACKED,1,  GS_REGS_AD,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{0, 1, 0, 1, 0/*pad*/, 64,0/*pad*/},SCE_GS_ALPHA_1,	/* alpha */
	{0,0,0,0,0,0,0/*pad*/},SCE_GS_CLAMP_1,		/* clamp */
	{0/*pad*/,0,0/*pad*/,0,0,0,0,0},SCE_GS_TEX2_1,		/* tex2 */
	{0,0,0,0,0,0,0,0,0,0,0,0},SCE_GS_TEX0_1		/* tex0 */
};

	/*
		スクラッチパッド割り当て
	*/
typedef	struct	{
	FMATRIX		eye_pers ;
	FMATRIX		eye_pers2 ;
	FMATRIX		pers ;
	FMATRIX		trans_mat ;
	FVECTOR		bound[8] ;
	FVECTOR		scale ;
	float		screen ;		/* Ｚ値算出用補正定数 */
	int			sort_z ;		/* Ｗより求めたＺ値 */
	int			clip_flag[256] ;/* クリップフラグ記憶用 */
	FVECTOR		xyzw[256] ;		/* Ｗ除算前の同時座標記憶バッファ */
	FVECTOR		verts[256] ;	/* プリミティブ頂点座標用バッファ */
	FVECTOR		uvs[256] ;		/* 歪み補正計算用ＵＶ格納バッファ */
} ScrPad ;

#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)
#define	PERS_MAT	(&(SCRPAD->pers))
#define	EYE_PERS	(&(SCRPAD->eye_pers))
#define	EYE_PERS2	(&(SCRPAD->eye_pers2))
#define	TRANS_MAT	(&(SCRPAD->trans_mat))
#define BOUNDS		(SCRPAD->bound)
#define SCALE		(&SCRPAD->scale)
#define SCREEN		(SCRPAD->screen)
#define SORT_Z		(SCRPAD->sort_z)
#define WORK_VERTS	(SCRPAD->verts)
#define WORK_XYZW	(SCRPAD->xyzw)
#define WORK_UVS	(SCRPAD->uvs)

/*----------------------------------------------------------------*/

static inline void _SetRotTransPersMatrix( FMATRIX *m )
{
	asm ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	": : "r"(m) );
}

static inline void _SetPersMatrix( FMATRIX *m )
{
	asm ("
	lqc2		vf24,0x00(%0)
	lqc2		vf25,0x10(%0)
	lqc2		vf26,0x20(%0)
	lqc2		vf27,0x30(%0)
	": : "r"(m) );
}

static void SetVu0Work( DG_CHANL *cp, DG_PRIM *prim )
{
	static FVECTOR param[4] = {
		/* プリミティブ座標へのスケーリングパラメータ（描画サイズにより可変） */
		{ DRAW_WIDTH/2, DRAW_HEIGHT/2, DRAW_Z_SCALE, 0.0F },
		/* プリミティブ座標へのオフセット値 */
		{ 2048.0F, 2048.0F, DRAW_Z_OFFSET, 0.0F },
		/* プリミティブ座標クランプ値（最小） */
		{0.0F,0.0F,DRAW_Z_MIN,255.0F},
		/* プリミティブ座標クランプ値（最大） */
		{4095.0F,4095.0F,DRAW_Z_MAX,255.0F},
	};
	param[0].vx = cp->width / 2.0F ;
	param[0].vy = cp->height / 2.0F ;
	param[1].vz = (float)DRAW_Z_OFFSET + (float)prim->raise ;
	asm ("
	lqc2		vf28,0x00(%0)
	lqc2		vf29,0x10(%0)
	lqc2		vf30,0x20(%0)
	lqc2		vf31,0x30(%0)
	": : "r"(param):"memory" );
}
/*----------------------------------------------------------------*/
static void _RotTransPers( FVECTOR *out, FVECTOR *in, int n )
{
	asm ("
		move			$8,%0			# ４頂点まとめて透視変換
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf8,0($9)
		lqc2			vf9,16($9)
		lqc2			vf10,32($9)
		lqc2			vf11,48($9)
		vmulax.xyzw		ACC,vf4,vf8		# 座標変換１（Ａ）
		vmadday.xyzw	ACC,vf5,vf8		# 座標変換２（Ａ）
		vmaddaz.xyzw	ACC,vf6,vf8		# 座標変換３（Ａ）
		vmaddw.xyzw		vf12,vf7,vf0	# 座標変換４（Ａ）
		vmulax.xyzw		ACC,vf4,vf9		# 座標変換１（Ｂ）
		vmadday.xyzw	ACC,vf5,vf9		# 座標変換２（Ｂ）
		vmaddaz.xyzw	ACC,vf6,vf9		# 座標変換３（Ｂ）
		vmaddw.xyzw		vf13,vf7,vf0	# 座標変換４（Ｂ）
		vmulax.xyzw		ACC,vf4,vf10	# 座標変換１（Ｃ）
		vmadday.xyzw	ACC,vf5,vf10	# 座標変換２（Ｃ）
		vmaddaz.xyzw	ACC,vf6,vf10	# 座標変換３（Ｃ）
		vmaddw.xyzw		vf14,vf7,vf0	# 座標変換４（Ｃ）
		vmulax.xyzw		ACC,vf4,vf11	# 座標変換１（Ｄ）
		vmadday.xyzw	ACC,vf5,vf11	# 座標変換２（Ｄ）
		vmaddaz.xyzw	ACC,vf6,vf11	# 座標変換３（Ｄ）
		vmaddw.xyzw		vf15,vf7,vf0	# 座標変換４（Ｄ）
		sqc2			vf12,0($8)		# 計算結果の書き出し
		sqc2			vf13,16($8)		# 計算結果の書き出し
		sqc2			vf14,32($8)		# 計算結果の書き出し
		sqc2			vf15,48($8)		# 計算結果の書き出し
		addi			$10,$10,-4		# カウンタデクリメント
		addi			$9,$9,64		# 読み込みポインタ更新
		addi			$8,$8,64		# 書き込みポインタ更新
		bgtz			$10,0b	# ループ処理
	"::"r"(out),"r"(in),"r"(n):"$8","$9","$10","$11","$12","memory");
}
static void _MakeXYZ( FVECTOR *out, FVECTOR *in, int n )
{
	asm ("
		move			$8,%0			# ４頂点まとめて透視変換
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf8,0($9)
		lqc2			vf9,16($9)
		lqc2			vf10,32($9)
		lqc2			vf11,48($9)
		vmaxz.w			vf8,vf8,vf8		# Ｗのクランプ処理
		vmaxz.w			vf9,vf9,vf9		# Ｗのクランプ処理
		vmaxz.w			vf10,vf10,vf10	# Ｗのクランプ処理
		vmaxz.w			vf11,vf11,vf11	# Ｗのクランプ処理
		vdiv			Q,vf0w,vf8w		# １／ｗ算出
		vwaitq							# ウェイト
		vmulq.xyz		vf8,vf8,Q		# 座標算出
		vdiv			Q,vf0w,vf9w		# １／ｗ算出
		vwaitq							# ウェイト
		vmulq.xyz		vf9,vf9,Q		# 座標算出
		vdiv			Q,vf0w,vf10w	# １／ｗ算出
		vwaitq							# ウェイト
		vmulq.xyz		vf10,vf10,Q		# 座標算出
		vdiv			Q,vf0w,vf11w	# １／ｗ算出
		vwaitq							# ウェイト
		vmulq.xyz		vf11,vf11,Q		# 座標算出
		vmul.xyz		vf8,vf8,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vmul.xyz		vf9,vf9,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vmul.xyz		vf10,vf10,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vmul.xyz		vf11,vf11,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vadd.xyz		vf8,vf8,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vadd.xyz		vf9,vf9,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vadd.xyz		vf10,vf10,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vadd.xyz		vf11,vf11,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vmax.xyzw		vf8,vf8,vf30	# クランプ処理
		vmax.xyzw		vf9,vf9,vf30	# クランプ処理
		vmax.xyzw		vf10,vf10,vf30	# クランプ処理
		vmax.xyzw		vf11,vf11,vf30	# クランプ処理
		vmini.xyzw		vf8,vf8,vf31	# クランプ処理
		vmini.xyzw		vf9,vf9,vf31	# クランプ処理
		vmini.xyzw		vf10,vf10,vf31	# クランプ処理
		vmini.xyzw		vf11,vf11,vf31	# クランプ処理
		vftoi4.xyzw		vf8,vf8			# 整数化
		vftoi4.xyzw		vf9,vf9			# 整数化
		vftoi4.xyzw		vf10,vf10		# 整数化
		vftoi4.xyzw		vf11,vf11		# 整数化
		sqc2			vf8,0($8)		# 計算結果の書き出し
		sqc2			vf9,16($8)		# 計算結果の書き出し
		sqc2			vf10,32($8)		# 計算結果の書き出し
		sqc2			vf11,48($8)		# 計算結果の書き出し
		addi			$10,$10,-4		# カウンタデクリメント
		addiu			$9,$9,64		# 読み込みポインタ更新
		addiu			$8,$8,64		# 書き込みポインタ更新
		bgtz			$10,0b	# ループ処理
	"::"r"(out),"r"(in),"r"(n):"$8","$9","$10","memory");
}

/* 頂点の透視変換（ＳＴＱ計算付き） */
static void _MakeXYZSTQ( FVECTOR *out, FVECTOR *in, FVECTOR *uv_out, FVECTOR *uv_in, int n )
{
	asm ("								# ４頂点まとめて透視変換
		move			$8,%0			# $8:頂点読み込み
		move			$9,%1			# $9:頂点書き込み
		move			$10,%4			# $10:ループカウンタ
		move			$11,%2			# $11:ＵＶ書き込み
		move			$12,%3			# $12:ＵＶ読み込み
0:
		lqc2			vf8,0($9)
		lqc2			vf9,16($9)
		lqc2			vf10,32($9)
		lqc2			vf11,48($9)
		vmaxz.w			vf8,vf8,vf8		# Ｗのクランプ処理
		vmaxz.w			vf9,vf9,vf9		# Ｗのクランプ処理
		vmaxz.w			vf10,vf10,vf10	# Ｗのクランプ処理
		vmaxz.w			vf11,vf11,vf11	# Ｗのクランプ処理
		vdiv			Q,vf0w,vf8w		# １／ｗ算出
		lqc2			vf12,0($12)		# ＵＶ値読み込み
		ld				$13,8($12)
		qmtc2			$13,vf13
		vmr32.z			vf12,vf0		# ＵＶ
		vwaitq							# ウェイト
		vmulq.xyz		vf8,vf8,Q		# 座標算出
		vmulq.xyz		vf12,vf12,Q		# 歪み補正
		vdiv			Q,vf0w,vf9w		# １／ｗ算出
		vmr32.z			vf13,vf0		# ＵＶ
		lqc2			vf14,16($12)	# ＵＶ値読み込み
		ld				$13,24($12)
		qmtc2			$13,vf15
		vwaitq							# ウェイト
		vmulq.xyz		vf9,vf9,Q		# 座標算出
		vmulq.xyz		vf13,vf13,Q		# 歪み補正
		vdiv			Q,vf0w,vf10w	# １／ｗ算出
		vmr32.z			vf14,vf0		# ＵＶ
		vwaitq							# ウェイト
		vmulq.xyz		vf10,vf10,Q		# 座標算出
		vmulq.xyz		vf14,vf14,Q		# 歪み補正
		vdiv			Q,vf0w,vf11w	# １／ｗ算出
		vmr32.z			vf15,vf0		# ＵＶ
		vwaitq							# ウェイト
		vmulq.xyz		vf11,vf11,Q		# 座標算出
		vmulq.xyz		vf15,vf15,Q		# 歪み補正
		vmul.xyz		vf8,vf8,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vmul.xyz		vf9,vf9,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vmul.xyz		vf10,vf10,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vmul.xyz		vf11,vf11,vf28	# ＸＹＺをプリミティブ座標へ（乗算）
		vadd.xyz		vf8,vf8,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vadd.xyz		vf9,vf9,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vadd.xyz		vf10,vf10,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vadd.xyz		vf11,vf11,vf29	# ＸＹＺをプリミティブ座標へ（加算）
		vmax.xyzw		vf8,vf8,vf30	# クランプ処理
		vmax.xyzw		vf9,vf9,vf30	# クランプ処理
		vmax.xyzw		vf10,vf10,vf30	# クランプ処理
		vmax.xyzw		vf11,vf11,vf30	# クランプ処理
		vmini.xyzw		vf8,vf8,vf31	# クランプ処理
		vmini.xyzw		vf9,vf9,vf31	# クランプ処理
		vmini.xyzw		vf10,vf10,vf31	# クランプ処理
		vmini.xyzw		vf11,vf11,vf31	# クランプ処理
		vftoi4.xyzw		vf8,vf8			# 整数化
		vftoi4.xyzw		vf9,vf9			# 整数化
		vftoi4.xyzw		vf10,vf10		# 整数化
		vftoi4.xyzw		vf11,vf11		# 整数化
		sqc2			vf8,0($8)		# 計算結果の書き出し
		sqc2			vf9,16($8)		# 計算結果の書き出し
		sqc2			vf10,32($8)		# 計算結果の書き出し
		sqc2			vf11,48($8)		# 計算結果の書き出し
		sqc2			vf12,0($11)		# 計算結果の書き出し
		sqc2			vf13,16($11)	# 計算結果の書き出し
		sqc2			vf14,32($11)	# 計算結果の書き出し
		sqc2			vf15,48($11)	# 計算結果の書き出し
		addi			$10,$10,-4		# カウンタデクリメント
		addiu			$9,$9,64		# 読み込みポインタ更新
		addiu			$8,$8,64		# 書き込みポインタ更新
		addiu			$11,$11,64		# ＵＶ書き込みポインタ更新
		addiu			$12,$12,32		# ＵＶ読み込みポインタ更新
		bgtz			$10,0b	# ループ処理
	"::"r"(out),"r"(in),"r"(uv_out),"r"(uv_in),"r"(n):"$8","$9","$10","$11","$12","$13","memory");
}

/* 頂点の透視変換 */
static void _RotTransPers2D1( FVECTOR *out, FVECTOR *in, int n )
{
	asm ("
		move			$8,%0			# ４頂点まとめて透視変換
		move			$9,%1
		move			$10,%2
		vmulx			vf1,vf20,vf0	#
0:
		lqc2			vf8,0($9)
		lqc2			vf9,16($9)
		lqc2			vf10,32($9)
		lqc2			vf11,48($9)
.if 1
		vmulaw.xyzw		ACC,vf24,vf8	# 矩形作成
		vmaddw.xyzw		vf16,vf25,vf8	# 矩形作成
		vmulaw.xyzw		ACC,vf24,vf9	# 矩形作成
		vmaddw.xyzw		vf17,vf25,vf9	# 矩形作成
		vmulaw.xyzw		ACC,vf24,vf10	# 矩形作成
		vmaddw.xyzw		vf18,vf25,vf10	# 矩形作成
		vmulaw.xyzw		ACC,vf24,vf11	# 矩形作成
		vmaddw.xyzw		vf19,vf25,vf11	# 矩形作成
		vmulax.xyzw		ACC,vf4,vf8		# 座標変換１（Ａ）
		vmadday.xyzw	ACC,vf5,vf8		# 座標変換２（Ａ）
		vmaddaz.xyzw	ACC,vf6,vf8		# 座標変換３（Ａ）
		vmaddw.xyzw		vf12,vf7,vf0	# 座標変換４（Ａ）
		vmulax.xyzw		ACC,vf4,vf9		# 座標変換１（Ｂ）
		vmadday.xyzw	ACC,vf5,vf9		# 座標変換２（Ｂ）
		vmaddaz.xyzw	ACC,vf6,vf9		# 座標変換３（Ｂ）
		vmaddw.xyzw		vf13,vf7,vf0	# 座標変換４（Ｂ）
		vmulax.xyzw		ACC,vf4,vf10	# 座標変換１（Ｃ）
		vmadday.xyzw	ACC,vf5,vf10	# 座標変換２（Ｃ）
		vmaddaz.xyzw	ACC,vf6,vf10	# 座標変換３（Ｃ）
		vmaddw.xyzw		vf14,vf7,vf0	# 座標変換４（Ｃ）
		vmulax.xyzw		ACC,vf4,vf11	# 座標変換１（Ｄ）
		vmadday.xyzw	ACC,vf5,vf11	# 座標変換２（Ｄ）
		vmaddaz.xyzw	ACC,vf6,vf11	# 座標変換３（Ｄ）
		vmaddw.xyzw		vf15,vf7,vf0	# 座標変換４（Ｄ）
		vsub.xyzw		vf20,vf12,vf16	# 矩形左上頂点生成
		vadd.xyzw		vf16,vf12,vf16	# 矩形右下頂点生成
		vsub.xyzw		vf21,vf13,vf17	# 矩形左上頂点生成
		vadd.xyzw		vf17,vf13,vf17	# 矩形右下頂点生成
		vsub.xyzw		vf22,vf14,vf18	# 矩形左上頂点生成
		vadd.xyzw		vf18,vf14,vf18	# 矩形右下頂点生成
		vsub.xyzw		vf23,vf15,vf19	# 矩形左上頂点生成
		vadd.xyzw		vf19,vf15,vf19	# 矩形右下頂点生成
.else
		vmulaw.xyzw		ACC,vf4,vf8		# 矩形作成
		vmaddaw.xyzw	ACC,vf5,vf8		# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf8		# 矩形作成
		vmaddw.xyzw		vf16,vf7,vf0	# 矩形作成
		vmulaw.xyzw		ACC,vf4,vf9		# 矩形作成
		vmaddaw.xyzw	ACC,vf5,vf9		# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf9		# 矩形作成
		vmaddw.xyzw		vf17,vf8,vf0	# 矩形作成
		vmulaw.xyzw		ACC,vf4,vf10	# 矩形作成
		vmaddaw.xyzw	ACC,vf5,vf10	# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf10	# 矩形作成
		vmaddw.xyzw		vf18,vf7,vf0	# 矩形作成
		vmulaw.xyzw		ACC,vf4,vf11	# 矩形作成
		vmaddaw.xyzw	ACC,vf5,vf11	# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf11	# 矩形作成
		vmaddw.xyzw		vf19,vf7,vf0	# 矩形作成
		vmulax.xyzw		ACC,vf4,vf8		# 座標変換１（Ａ）
		vmadday.xyzw	ACC,vf5,vf8		# 座標変換２（Ａ）
		vmaddaz.xyzw	ACC,vf6,vf8		# 座標変換３（Ａ）
		vmaddw.xyzw		vf12,vf7,vf0	# 座標変換４（Ａ）
		vmulax.xyzw		ACC,vf4,vf9		# 座標変換１（Ｂ）
		vmadday.xyzw	ACC,vf5,vf9		# 座標変換２（Ｂ）
		vmaddaz.xyzw	ACC,vf6,vf9		# 座標変換３（Ｂ）
		vmaddw.xyzw		vf13,vf7,vf0	# 座標変換４（Ｂ）
		vmulax.xyzw		ACC,vf4,vf10	# 座標変換１（Ｃ）
		vmadday.xyzw	ACC,vf5,vf10	# 座標変換２（Ｃ）
		vmaddaz.xyzw	ACC,vf6,vf10	# 座標変換３（Ｃ）
		vmaddw.xyzw		vf14,vf7,vf0	# 座標変換４（Ｃ）
		vmulax.xyzw		ACC,vf4,vf11	# 座標変換１（Ｄ）
		vmadday.xyzw	ACC,vf5,vf11	# 座標変換２（Ｄ）
		vmaddaz.xyzw	ACC,vf6,vf11	# 座標変換３（Ｄ）
		vmaddw.xyzw		vf15,vf7,vf0	# 座標変換４（Ｄ）
		vsub.xy			vf20,vf12,vf16	# 矩形左上頂点生成
		vadd.xy			vf16,vf12,vf16	# 矩形右下頂点生成
		vsub.xy			vf21,vf13,vf17	# 矩形左上頂点生成
		vadd.xy			vf17,vf13,vf17	# 矩形右下頂点生成
		vsub.xy			vf22,vf14,vf18	# 矩形左上頂点生成
		vadd.xy			vf18,vf14,vf18	# 矩形右下頂点生成
		vsub.xy			vf23,vf15,vf19	# 矩形左上頂点生成
		vadd.xy			vf19,vf15,vf19	# 矩形右下頂点生成
.endif
		sqc2			vf20,0($8)		# 計算結果の書き出し
		sqc2			vf16,16($8)		# 計算結果の書き出し
		sqc2			vf21,32($8)		# 計算結果の書き出し
		sqc2			vf17,48($8)		# 計算結果の書き出し
		sqc2			vf22,64($8)		# 計算結果の書き出し
		sqc2			vf18,80($8)		# 計算結果の書き出し
		sqc2			vf23,96($8)		# 計算結果の書き出し
		sqc2			vf19,112($8)	# 計算結果の書き出し
		addi			$10,$10,-4		# カウンタデクリメント
		addi			$9,$9,64		# 読み込みポインタ更新
		addi			$8,$8,128		# 書き込みポインタ更新
		bgtz			$10,0b	# ループ処理
	"::"r"(out),"r"(in),"r"(n):"$8","$9","$10","memory");
}
/* 頂点の透視変換 */
static void _RotTransPers2D2( FVECTOR *out, FVECTOR *in, FVECTOR *size, int n )
{
	asm ("
		move			$8,%0			# ４頂点まとめて透視変換
		move			$9,%1
		move			$10,%3
		move			$11,%2
		vmulx			vf1,vf20,vf0	#
0:
		lqc2			vf2,0($11)		# 矩形サイズ読み込み
		lqc2			vf3,16($11)		# 矩形サイズ読み込み 
		lqc2			vf8,0($9)
		lqc2			vf9,16($9)
		lqc2			vf10,32($9)
		lqc2			vf11,48($9)
.if 1
		vmulax.xyzw		ACC,vf24,vf2	# 矩形作成
		vmaddy.xyzw		vf16,vf25,vf2	# 矩形作成
		vmulaz.xyzw		ACC,vf24,vf2	# 矩形作成
		vmaddw.xyzw		vf17,vf25,vf2	# 矩形作成
		vmulax.xyzw		ACC,vf24,vf3	# 矩形作成
		vmaddy.xyzw		vf18,vf25,vf3	# 矩形作成
		vmulaz.xyzw		ACC,vf24,vf3	# 矩形作成
		vmaddw.xyzw		vf19,vf25,vf3	# 矩形作成
		vmulax.xyzw		ACC,vf4,vf8		# 座標変換１（Ａ）
		vmadday.xyzw	ACC,vf5,vf8		# 座標変換２（Ａ）
		vmaddaz.xyzw	ACC,vf6,vf8		# 座標変換３（Ａ）
		vmaddw.xyzw		vf12,vf7,vf0	# 座標変換４（Ａ）
		vmulax.xyzw		ACC,vf4,vf9		# 座標変換１（Ｂ）
		vmadday.xyzw	ACC,vf5,vf9		# 座標変換２（Ｂ）
		vmaddaz.xyzw	ACC,vf6,vf9		# 座標変換３（Ｂ）
		vmaddw.xyzw		vf13,vf7,vf0	# 座標変換４（Ｂ）
		vmulax.xyzw		ACC,vf4,vf10	# 座標変換１（Ｃ）
		vmadday.xyzw	ACC,vf5,vf10	# 座標変換２（Ｃ）
		vmaddaz.xyzw	ACC,vf6,vf10	# 座標変換３（Ｃ）
		vmaddw.xyzw		vf14,vf7,vf0	# 座標変換４（Ｃ）
		vmulax.xyzw		ACC,vf4,vf11	# 座標変換１（Ｄ）
		vmadday.xyzw	ACC,vf5,vf11	# 座標変換２（Ｄ）
		vmaddaz.xyzw	ACC,vf6,vf11	# 座標変換３（Ｄ）
		vmaddw.xyzw		vf15,vf7,vf0	# 座標変換４（Ｄ）
		vsub.xyzw		vf20,vf12,vf16	# 矩形左上頂点生成
		vadd.xyzw		vf16,vf12,vf16	# 矩形右下頂点生成
		vsub.xyzw		vf21,vf13,vf17	# 矩形左上頂点生成
		vadd.xyzw		vf17,vf13,vf17	# 矩形右下頂点生成
		vsub.xyzw		vf22,vf14,vf18	# 矩形左上頂点生成
		vadd.xyzw		vf18,vf14,vf18	# 矩形右下頂点生成
		vsub.xyzw		vf23,vf15,vf19	# 矩形左上頂点生成
		vadd.xyzw		vf19,vf15,vf19	# 矩形右下頂点生成
.else
		vmulax.xyzw		ACC,vf4,vf2		# 矩形作成
		vmadday.xyzw	ACC,vf5,vf2		# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf8		# 矩形作成
		vmaddw.xyzw		vf16,vf7,vf0	# 矩形作成
		vmulaz.xyzw		ACC,vf4,vf2		# 矩形作成
		vmaddaw.xyzw	ACC,vf5,vf2		# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf9		# 矩形作成
		vmaddw.xyzw		vf16,vf7,vf0	# 矩形作成
		vmulax.xyzw		ACC,vf4,vf3		# 矩形作成
		vmadday.xyzw	ACC,vf5,vf3		# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf10	# 矩形作成
		vmaddw.xyzw		vf16,vf7,vf0	# 矩形作成
		vmulaz.xyzw		ACC,vf4,vf3		# 矩形作成
		vmaddaw.xyzw	ACC,vf5,vf3		# 矩形作成
		vmaddaz.xyzw	ACC,vf6,vf11	# 矩形作成
		vmaddw.xyzw		vf16,vf7,vf0	# 矩形作成
		vmulax.xyzw		ACC,vf4,vf8		# 座標変換１（Ａ）
		vmadday.xyzw	ACC,vf5,vf8		# 座標変換２（Ａ）
		vmaddaz.xyzw	ACC,vf6,vf8		# 座標変換３（Ａ）
		vmaddw.xyzw		vf12,vf7,vf0	# 座標変換４（Ａ）
		vmulax.xyzw		ACC,vf4,vf9		# 座標変換１（Ｂ）
		vmadday.xyzw	ACC,vf5,vf9		# 座標変換２（Ｂ）
		vmaddaz.xyzw	ACC,vf6,vf9		# 座標変換３（Ｂ）
		vmaddw.xyzw		vf13,vf7,vf0	# 座標変換４（Ｂ）
		vmulax.xyzw		ACC,vf4,vf10	# 座標変換１（Ｃ）
		vmadday.xyzw	ACC,vf5,vf10	# 座標変換２（Ｃ）
		vmaddaz.xyzw	ACC,vf6,vf10	# 座標変換３（Ｃ）
		vmaddw.xyzw		vf14,vf7,vf0	# 座標変換４（Ｃ）
		vmulax.xyzw		ACC,vf4,vf11	# 座標変換１（Ｄ）
		vmadday.xyzw	ACC,vf5,vf11	# 座標変換２（Ｄ）
		vmaddaz.xyzw	ACC,vf6,vf11	# 座標変換３（Ｄ）
		vmaddw.xyzw		vf15,vf7,vf0	# 座標変換４（Ｄ）
		vsub.xy			vf20,vf12,vf16	# 矩形左上頂点生成
		vadd.xy			vf16,vf12,vf16	# 矩形右下頂点生成
		vsub.xy			vf21,vf13,vf17	# 矩形左上頂点生成
		vadd.xy			vf17,vf13,vf17	# 矩形右下頂点生成
		vsub.xy			vf22,vf14,vf18	# 矩形左上頂点生成
		vadd.xy			vf18,vf14,vf18	# 矩形右下頂点生成
		vsub.xy			vf23,vf15,vf19	# 矩形左上頂点生成
		vadd.xy			vf19,vf15,vf19	# 矩形右下頂点生成
.endif
		sqc2			vf20,0($8)		# 計算結果の書き出し
		sqc2			vf16,16($8)		# 計算結果の書き出し
		sqc2			vf21,32($8)		# 計算結果の書き出し
		sqc2			vf17,48($8)		# 計算結果の書き出し
		sqc2			vf22,64($8)		# 計算結果の書き出し
		sqc2			vf18,80($8)		# 計算結果の書き出し
		sqc2			vf23,96($8)		# 計算結果の書き出し
		sqc2			vf19,112($8)	# 計算結果の書き出し
		addi			$10,$10,-4		# カウンタデクリメント
		addi			$11,$11,32		# 矩形サイズ読み込みポインタ更新
		addi			$9,$9,64		# 読み込みポインタ更新
		addi			$8,$8,128		# 書き込みポインタ更新
		bgtz			$10,0b	# ループ処理
	"::"r"(out),"r"(in),"r"(size),"r"(n):"$8","$9","$10","$11","memory");
}

/*----------------------------------------------------------------*/
static void MakePrims3D( DG_PRIM *prim, int which )
{
	DG_PRIM_PACKET	*packet ;
	FVECTOR		*pos ;
	u_long128	*src, *dst, *old ;
	int			i, j, total_verts, n_prims, n_packet, max_verts ;
	float		r_nverts, total_z ;
	int			*set_z_addr ;
	static int	tmp_int[2] ;

	total_verts = prim->n_prims * prim->prim_nverts * prim->n_packet ;
	n_packet = prim->n_packet ;
	n_prims = 0 ;
	packet = (DG_PRIM_PACKET*)( (int)prim->packs[ which ] - prim->packet_size ) ;
	/* １パケット当たりの頂点数（プリミティブ頂点数×プリミティブ数）の逆数を求める */
	if ( prim->n_prims < 0 ) prim->n_prims = 1 ;
	r_nverts = 1.0F / ( (float)prim->prim_nverts * prim->n_prims ) ;

	set_z_addr = tmp_int ;
	pos = prim->pos ;
	for ( ; total_verts > 0 ; total_verts -= 252 ){	/* 252は2,3,4の公倍数 */

		i = ( total_verts > 251 ) ? 252 : total_verts ;

		/* 頂点変換 */
		_RotTransPers( WORK_XYZW, pos, i );
		_MakeXYZ( WORK_VERTS, WORK_XYZW, i );
		pos += i ;

		/* パケットの作成 */
		src = (u_long128*)WORK_VERTS ;
		for ( ; i > 0 ; i -= prim->prim_nverts ){

			/* 終了チェック */
			if ( --n_prims <= 0 ){ /* 一番最初もチェックに引っかかるようになっている */
				if ( n_packet-- > 0 ){
					n_prims = prim->n_prims ;
					packet = (DG_PRIM_PACKET*)( (int)packet + prim->packet_size ) ;
					dst = (u_long128*)packet->prim_top ;
					/* ソート用Ｚ値の格納 */
					*set_z_addr = DG_FTOI( total_z * r_nverts );
					total_z = 0.0f ;
					set_z_addr = &packet->header.sort_z ;
				} else {
					break ;
				}
			}

			old = dst ;
			dst = dst + prim->prim_voffset ;
			/* プリミティブの頂点データを書き込む */
			for ( j = prim->prim_nverts ; j > 0 ; j-- ){
				total_z += ( (FVECTOR*)src - 256 )->vw ;
#if 0
				*dst = *src++ ;
#else
				/* ＸＹＺのメンバのみをコピー */
				( (u_long*)dst )[0] = ( (u_long*)src )[0] ;
				( (u_int*)dst )[2] = ( (u_int*)src )[2] ;
				src++ ;
#endif
				dst += prim->prim_vstep ;
			}
			dst = old + prim->prim_size ;
		}
	}
	*set_z_addr = DG_FTOI( total_z * r_nverts );
}

static void MakePrims3DQ( DG_PRIM *prim, int which )
{
	DG_PRIM_PACKET	*packet ;
	FVECTOR		*pos, *uvs ;
	u_long128	*src, *dst, *old ;
	int			i, j, total_verts, n_prims, n_packet, max_verts ;
	float		r_nverts, total_z ;
	int			*set_z_addr, tmp_int ;

	total_verts = prim->n_prims * prim->prim_nverts * prim->n_packet ;
	n_packet = prim->n_packet ;
	n_prims = 0 ;
	packet = (DG_PRIM_PACKET*)( (int)prim->packs[ which ] - prim->packet_size ) ;
	/* １パケット当たりの頂点数（プリミティブ頂点数×プリミティブ数）の逆数を求める */
	r_nverts = 1.0F / ( (float)prim->prim_nverts * prim->n_prims ) ;

	set_z_addr = &tmp_int ;
	pos = prim->pos ;
	uvs = prim->uvs ;
	for ( ; total_verts > 0 ; total_verts -= 252 ){	/* 252は2,3,4の公倍数 */

		i = ( total_verts > 251 ) ? 252 : total_verts ;

		/* 頂点変換 */
		_RotTransPers( WORK_XYZW, pos, i );
		_MakeXYZSTQ( WORK_VERTS, WORK_XYZW, WORK_UVS, uvs, i );
		pos += i ;
		uvs += i/2 ;

		/* パケットの作成 */
		src = (u_long128*)WORK_VERTS ;
		for ( ; i > 0 ; i -= prim->prim_nverts ){

			/* 終了チェック */
			if ( --n_prims <= 0 ){ /* 一番最初もチェックに引っかかるようになっている */
				if ( n_packet-- > 0 ){
					n_prims = prim->n_prims ;
					packet = (DG_PRIM_PACKET*)( (int)packet + prim->packet_size ) ;
					dst = (u_long128*)packet->prim_top ;
					/* ソート用Ｚ値の格納 */
					*set_z_addr = DG_FTOI( total_z * r_nverts );
					total_z = 0.0f ;
					set_z_addr = &packet->header.sort_z ;
				} else {
					break ;
				}
			}

			old = dst ;
			dst = dst + prim->prim_voffset ;
			/* プリミティブの頂点データを書き込む */
			for ( j = prim->prim_nverts ; j > 0 ; j-- ){
				total_z += ( (FVECTOR*)src - 256 )->vw ;
#if 1
				dst[0] = src[0] ;
#else
				/* ＸＹＺのメンバのみをコピー */
				( (u_long*)dst )[0] = ( (u_long*)src )[0] ;
				( (u_int*)dst )[2] = ( (u_int*)src )[2] ;
#endif
				dst[0] = src[0] ;
				dst[-2] = src[256] ;
				src++ ;
				dst += prim->prim_vstep ;
			}
			dst = old + prim->prim_size ;
		}
	}
	*set_z_addr = DG_FTOI( total_z * r_nverts );
}


static void MakePrims2D( DG_PRIM *prim, int which )
{
	DG_PRIM_PACKET	*packet ;
	FVECTOR		*pos, *r_size ;
	u_long128	*src, *dst, *old ;
	int			i, j, total_verts, n_prims, n_packet, max_verts ;
	float		r_nverts, total_z ;
	int			*set_z_addr, tmp_int ;

	total_verts = prim->n_prims * prim->prim_nverts * prim->n_packet ;
	n_packet = prim->n_packet ;
	n_prims = 0 ;
	packet = (DG_PRIM_PACKET*)( (int)prim->packs[ which ] - prim->packet_size ) ;
	/* １パケット当たりの頂点数（プリミティブ頂点数×プリミティブ数）の逆数を求める */
	r_nverts = 1.0F / ( (float)prim->prim_nverts * prim->n_prims ) ;

	set_z_addr = &tmp_int ;
	pos = prim->pos ;
	r_size = prim->uvs ;
	for ( ; total_verts > 0 ; total_verts -= 252 ){	/* 252は2,3,4の公倍数 */

		i = ( total_verts > 251 ) ? 252 : total_verts ;

		/* 頂点変換 */
		if ( r_size == NULL ){
			_RotTransPers2D1( WORK_XYZW, pos, i/2 );
		} else {
			_RotTransPers2D2( WORK_XYZW, pos, r_size, i/2 );
			r_size += i / 4 ;
		}
		_MakeXYZ( WORK_VERTS, WORK_XYZW, i );
		pos += i/2 ;

		/* パケットの作成 */
		src = (u_long128*)WORK_VERTS ;
		for ( ; i > 0 ; i -= prim->prim_nverts ){

			/* 終了チェック */
			if ( --n_prims <= 0 ){ /* 一番最初もチェックに引っかかるようになっている */
				if ( n_packet-- > 0 ){
					n_prims = prim->n_prims ;
					packet = (DG_PRIM_PACKET*)( (int)packet + prim->packet_size ) ;
					dst = (u_long128*)packet->prim_top ;
					/* ソート用Ｚ値の格納 */
					*set_z_addr = DG_FTOI( total_z * r_nverts );
					total_z = 0.0f ;
					set_z_addr = &packet->header.sort_z ;
				} else {
					break ;
				}
			}

			old = dst ;
			dst = dst + prim->prim_voffset ;
			/* プリミティブの頂点データを書き込む */
			for ( j = prim->prim_nverts ; j > 0 ; j-- ){
				total_z += ( (FVECTOR*)src - 256 )->vw ;
				*dst = *src++ ;
				dst += prim->prim_vstep ;
			}
			dst = old + prim->prim_size ;
		}
	}
	*set_z_addr = DG_FTOI( total_z * r_nverts );
}


/*----------------------------------------------------------------*/
void DG_PrimChanl( DG_CHANL *cp, int which )
{
	DG_OBJ_QUEUE	*queue ;
	DG_PRIM			**pque ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		i, gid, c_gid, time ;

	if ( cp->chanl_num != 0 && cp->chanl_num != 4 ) return ;
	if ( ( queue = cp->obj_queue ) == NULL ) return ;
	obj_buff = &queue->prim_buffer ;
	/*
		キュー数のチェック
	*/
	if ( ( i = obj_buff->n_queue ) == 0 ) return ;
	GV_SET_PRFC_CLOCK();
	/*
		パース変換パラメータ／クリッピング領域を設定
	*/
	pque = obj_buff->queue ;
	c_gid = DG_CurrentGroupID ;

	*PERS_MAT = cp->pers ;
	*EYE_PERS = cp->eye_pers ;
	*EYE_PERS2 = cp->eye_pers2 ;
	SCALE->vx = cp->width / 2 ;
	SCALE->vy = cp->height / 2 ;

	for ( ; i > 0 ; -- i ) {
		DG_PRIM		*prim ;
		int		type ;

		prim = *( pque ++ ) ;
		type = prim->type ;
		if ( ( DG_PRIM_INVISIBLE | DG_PRIM_SORTONLY ) & type ) goto prim_end ;
		if ( ( gid = prim->group_id ) != 0 && !( gid & c_gid ) ) continue ;
		/*
			ワールド／カメラ相対処理
		*/
		if ( !( DG_PRIM_ON_CAMERA & type ) ) {
			if ( prim->root != NULL ) prim->world = *( prim->root ) ;
			sceVu0MulMatrix( TRANS_MAT, EYE_PERS, &prim->world ) ;
			_SetRotTransPersMatrix( TRANS_MAT );
		} else {
			_SetRotTransPersMatrix( PERS_MAT );
		}
		_SetPersMatrix( PERS_MAT );
		SetVu0Work( cp, prim );
		/*
			タイプ別処理
		*/
		if ( !( DG_PRIM_CALLBACK & type ) ){
			/* システム対応プリミティブ */
			if ( ( type & DG_PRIM_TYPE ) <= DG_PRIM_POLY_GTQ4 ){
				/* 頂点型プリミティブ */
				if ( prim->uvs == NULL ){
					/* テクスチャ補正なし３Ｄプリミティブ */
					MakePrims3D( prim, which );
				} else {
					/* テクスチャ補正あり３Ｄプリミティブ */
					MakePrims3DQ( prim, which );
				}
			} else {
				/* 矩形スプライトプリミティブ */
				MakePrims2D( prim, which );
			}
		} else {
			/* コールバック */
			(*prim->callback)( prim, which );
		}

	  prim_end:
#if 0
		if ( ( type & DG_PRIM_TYPE ) <= DG_PRIM_SPRT3D2 ){
			int		i, j, size, t ;
			void	*addr, *src ;
			DG_PRIM_PACKET	*packet ;
			packet = (DG_PRIM_PACKET*)( (int)prim->packs[ which ] + prim->packet_size * 0 ) ;
			for ( i = prim->n_packet ; i > 0 ; i-- ){
				packet->header.packet = DG_CurrentDmaAddr ;
				addr = DG_CurrentDmaAddr ;
				src = &packet->dma_tag ;
				size = prim->packet_size - sizeof(DG_PRIM_HEADER) ;
				while ( size > 0 ){
					t = ( size >= 16*256*2 ) ? 16*256*2 : size ;
					size -= t ;
					UTL_StartMemToSpr( WORK_XYZW, src, t>>4 );
					UTL_EndMemToSpr();
					UTL_StartSprToMem( addr, WORK_XYZW, t>>4 );
					UTL_EndSprToMem();
					addr = (void*)( (int)addr + t ) ;
					src = (void*)( (int)src + t ) ;
				}
				DG_CurrentDmaAddr = addr ;
				packet = (DG_PRIM_PACKET*)( (int)packet + prim->packet_size );
			}
		} else {
			int		i, j, size, t ;
			DG_PRIM_PACKET	*packet ;
			packet = (DG_PRIM_PACKET*)( (int)prim->packs[ which ] + prim->packet_size * 0 ) ;
			for ( i = prim->n_packet ; i > 0 ; i-- ){
				packet->header.packet = &packet->dma_tag ;
				packet = (DG_PRIM_PACKET*)( (int)packet + prim->packet_size );
			}
		}
#endif
	}
	time = GV_GET_PRFC_CLOCK();
	if ( GV_PadData[1].press & PAD_A ){
		//printf("prim.c: %d\n", time );
	}
}
/*----------------------------------------------------------------*/
DG_PRIM *DG_MakePrim( int type, int n_packet, int n_prims, int chanl, FVECTOR *pos, FVECTOR *vec )
{
	DG_PRIM		*prim ;
	Param		*param ;
	int		size, i, j ;

	param = Params + ( DG_PRIM_TYPE & type ) ;

	//size = ( param->prim_size * n_prims * sizeof(u_long128) + sizeof(DG_PRIM_PACKET) ) * n_packet;
	size = ( param->prim_size * n_prims * sizeof(u_long128) + sizeof(DG_PRIM_PACKET) );
	size = ALIGNSIZE1( size ) * n_packet;

	if ( type & DG_PRIM_FREEPRIM ) type = ( type & ~DG_PRIM_TYPE ) | DG_PRIM_FREE ;
	if ( ( type & DG_PRIM_TYPE ) >= DG_PRIM_FREE ){
		size = 0 ;
		n_packet = n_prims = 1 ;
	}

	/* プリミティブはパケットメモリも通常メモリに確保する */
	if ( ( prim = (DG_PRIM *)GV_AllocMemory( GV_NORMAL_MEMORY, NULL,
											ALIGNSIZE1( sizeof( DG_PRIM ) ) + size * 2 , 16 ) ) == NULL ){
		return NULL ;
	}
	/*
		基本パラメータの設定
	*/
	GV_ZeroMemory( prim, sizeof( DG_PRIM ) + size * 2 ) ;
//	GV_ZeroMemory( prim, sizeof( DG_PRIM ) ) ;
	prim->world = DG_UnitMatrix ;
	prim->type = type ;
	prim->n_prims = n_prims ;
	prim->n_packet = n_packet ;
	prim->chanl = chanl ;
	/* 初期値は全グループ表示 M.Sonoyama */
	prim->group_id = 0 ; /* 0で全グループ表示になるはず */

	if ( ( type & DG_PRIM_TYPE ) >= DG_PRIM_FREE ) return prim ;

	prim->near_z = 50 ;
	prim->pos = pos ;
	prim->packet_size = ALIGNSIZE1( param->prim_size * n_prims * sizeof(u_long128) + sizeof(DG_PRIM_PACKET) ) ;
	if ( param->stq_flag ) prim->uvs = vec ;
	/*
		タイプ別パラメータの設定
	*/
	prim->prim_size = param->prim_size ;
	prim->prim_nverts = param->prim_nverts ;
	prim->prim_voffset = param->prim_voffset ;
	prim->prim_vstep = param->prim_vstep ;
	/*
		パケットメモリを確保
	*/
	prim->packs[ 0 ] = ( u_long128* )( (int)prim + ALIGNSIZE1( sizeof( DG_PRIM ) ) );
	prim->packs[ 1 ] = ( u_long128* )( ( char * )prim->packs[ 0 ] + size );

	/*
		パケットメモリの初期化
	*/
	for ( j = 0 ; j < 2 ; j++ ){
		DG_PRIM_PACKET	*packet ;
		packet = (DG_PRIM_PACKET*)prim->packs[j] ;
		for ( i = n_packet ; i > 0 ; i-- ){
			size = ( sizeof(DG_PRIM_INIT) + sizeof(sceGifTag) ) / sizeof(u_long128) + param->prim_size * n_prims ;
			/* ＤＭＡタグ初期化 */
			packet->dma_tag.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, size );
			packet->dma_tag.addr = NULL ;
			packet->dma_tag.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
			packet->dma_tag.vifcode[1] = SCE_VIF1_SET_DIRECT( size, 0 ) ;
			/* プリミティブ描画設定初期化 */
			packet->prim_init = prim_init_data ;
			/* プリミティブ転送ＧＩＦタグの初期化 */
			packet->gif_tag = *(param->giftag) ;
			packet->gif_tag.NLOOP = prim->n_prims ;
			/* その他初期化 */
			packet->header.packet = NULL ;
			/* 次のパケットを処理 */
			packet = (DG_PRIM_PACKET*)( (int)packet + prim->packet_size );
		}
	}

	return prim;
}

void DG_FreePrim( DG_PRIM *prim )
{
	if ( prim == NULL ) return ;
	GV_DelayedFree( prim );
}
/*----------------------------------------------------------------*/
	/*
		プリミティブの初期化ＧＩＦパケットにテクスチャを設定する
	*/
void DG_ConfigPrimInitTex( DG_PRIM_INIT *prim_init, DG_TEX *tex )
{
	*(long64*)&prim_init->alpha = tex->tex_trans.alpha.data ;
	*(long64*)&prim_init->clamp = tex->tex_trans.clamp.data ;
	*(long64*)&prim_init->tex2 = tex->tex_trans.tex2.data ;
	*(long64*)&prim_init->tex0 = tex->tex_trans.tex0.data ;
}

	/*
		プリミティブのテクスチャを設定する
	*/
void DG_ConfigPrimTex( DG_PRIM *prim, DG_TEX *tex )
{
	DG_PRIM_PACKET	*packet0, *packet1 ;
	int				i, j ;

	packet0 = prim->packs[ 0 ] ;
	packet1 = prim->packs[ 1 ] ;
	for ( i = prim->n_packet ; i > 0 ; i-- ){
		*(long64*)&packet0->prim_init.alpha = tex->tex_trans.alpha.data ;
		*(long64*)&packet0->prim_init.clamp = tex->tex_trans.clamp.data ;
		*(long64*)&packet0->prim_init.tex2 = tex->tex_trans.tex2.data ;
		*(long64*)&packet0->prim_init.tex0 = tex->tex_trans.tex0.data ;
		*(long64*)&packet1->prim_init.alpha = tex->tex_trans.alpha.data ;
		*(long64*)&packet1->prim_init.clamp = tex->tex_trans.clamp.data ;
		*(long64*)&packet1->prim_init.tex2 = tex->tex_trans.tex2.data ;
		*(long64*)&packet1->prim_init.tex0 = tex->tex_trans.tex0.data ;
		packet0 = (DG_PRIM_PACKET*)( (int)packet0 + prim->packet_size );
		packet1 = (DG_PRIM_PACKET*)( (int)packet1 + prim->packet_size );
	}
}
