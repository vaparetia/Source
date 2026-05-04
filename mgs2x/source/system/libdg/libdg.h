/*
	libdg.h
	ディスプレイジェネレータライブラリヘッダ

	1999/07/07 K.Takabe
	$Id: libdg.h,v 1.4 2002/11/23 11:36:53 Yoshizawa1 Exp $

*/

#ifndef __LIBDG__H__
#define __LIBDG__H__

#if 1 //BP_RENDER
#include "bp_ps2emu.h"
#endif

#define BP_MAKEPRIM_DEBUG_INFO 1
#define BP_MAKEMODEL_DEBUG_INFO 1

#if defined(BP_VITA)
   #define ARMATURE_RENDER_FRAME_SKIP 1
#endif

#ifdef KP_WINDOWS
#include "xbox_ps2emu.h"
#include "libdgw.h"

#else
#ifdef KP_XBOX

#include "xbox_ps2emu.h"
#include "libdgx.h"

#else

#include "mgs_type.h"
#include "fmt_kmd.h"
#include "fmt_cm2.h"
#include	"fmt_lit.h"
#include	"libgv.h"

#ifdef __cplusplus
extern "C" {
#endif

	/*
		関節モデルの最大ユニット数
	*/
#define	DG_MAX_JOINTS	(128)
	/*
		オブジェクトキューサイズ
	*/
#define MAX_OBJ_QUEUE	(8)		/* 旧方式用（将来はなくなる） */

#define	DG_MAX_USR_OBJQUEUE	(8)

/* パフォーマンス詳細記録モードＯＮ */
#ifdef DEBUG_MODE
#define LIBDG_PERFORMANCE
#endif

/* テクスチャ多重ロードチェックモードＯＮ */
#if defined(DEBUG_MODE) && !defined(DESIGN_PREVIEW)
#define LIBDG_CHECK_MULTILOADTEX
#endif

/*----------------------------------------------------------------*/
/* ＜ＬＩＢＤＧ汎用定義＞ */

/* ＰＳ２ＤＭＡタグ構造体 */
typedef ALIGN16_DECL(struct) _dg_dmatag{
	u_int	qwc ;
	void	*addr ;
	u_int	vifcode[2] ;
} DG_DMATAG /* BP  */;

/* ＰＳ２ＧＳレジスタ（Ａ＋Ｄ）構造体 */
typedef ALIGN16_DECL(struct) _dg_gsreg {
	u_long64	data ;
	u_long64	reg ;
} DG_GSREG /* BP  */;

/* ＰＳ２ＧＩＦタグ構造体 */
typedef ALIGN16_DECL(struct) _dg_giftag{
	u_long64	tag ;
	u_long64	regs ;
} DG_GIFTAG /* BP  */;

/* ＰＳ２ＶＩＦデータ構造体（1qword分） */
typedef ALIGN16_DECL(struct) _dg_vifcode {
	u_int	vifcode[4] ;
} DG_VIFCODE /*  */;

/*----------------------------------------------------------------*/
	/*
		イメージ転送構造体
	*/
typedef ALIGN16_DECL(struct) _dg_loadimage {
	DG_DMATAG		dmatag0 ;			/* レジスタ設定値転送ＤＭＡタグ */
	DG_GIFTAG		giftag0 ;			/* レジスタ設定 */
	DG_GSREG		bitbltbuf ;
	DG_GSREG		trxpos ;
	DG_GSREG		trxreg ;
	DG_GSREG		trxdir ;
	DG_GIFTAG		giftag1 ;
	DG_DMATAG		dmatag1 ;			/* イメージ転送ＤＭＡタグ */
} DG_LOADIMAGE  ;

	/*
		ＶＲＡＭ内イメージ転送構造体
	*/
typedef ALIGN16_DECL(struct) _dg_moveimage {
	DG_DMATAG		dmatag ;			/* レジスタ設定値転送ＤＭＡタグ */
	DG_GIFTAG		giftag ;			/* レジスタ設定 */
	DG_GSREG		bitbltbuf ;
	DG_GSREG		trxpos ;
	DG_GSREG		trxreg ;
	DG_GSREG		trxdir ;
} DG_MOVEIMAGE  ;

	/*
		イメージ取り込み構造体（システム専用）
	*/
typedef ALIGN16_DECL(struct) _dg_storeimage {
	DG_DMATAG		dmatag ;			/* レジスタ設定値転送ＤＭＡタグ */
	DG_GIFTAG		giftag ;			/* レジスタ設定 */
	DG_GSREG		bitbltbuf ;
	DG_GSREG		trxpos ;
	DG_GSREG		trxreg ;
	DG_GSREG		finish ;
	DG_GSREG		trxdir ;
} DG_STOREIMAGE  ;

/*----------------------------------------------------------------*/
/* ＜光源関連＞ */

#define	MAX_TMPLIGHTS	(16)

	/*
		一時光源バッファ（ダブルバッファの片方）
	*/
typedef	struct	{
#if 0//BP_PS2 - use header that does not contain zero length array so it compiles on other platforms
   LIT_DEF			TmpLightDef ;
#else
   LIT_DEF_HEADER TmpLightDef ;
#endif
	LIT_GRP			TmpLightGrp[3] ;
	LIT_POINT		points[ MAX_TMPLIGHTS ] ;
	LIT_SPOT		spots[ MAX_TMPLIGHTS ] ;
	LIT_BLACKPOINT	blacks[ MAX_TMPLIGHTS ] ;
} DG_TLIGHT ;


/*----------------------------------------------------------------*/
/* ＜テクスチャ関連＞ */

	/*
		テクスチャパラメータＶＵ１転送パケット
	*/
typedef struct _dg_tex_trans {
	DG_GIFTAG	giftag ;	/* 描画設定ＧＩＦタグ */
	DG_GSREG	clamp ;		/* 描画設定プリミティブ（clamp） */
	DG_GSREG	tex2 ;		/* 描画設定プリミティブ（tex2） */
	DG_GSREG	tex0 ;		/* 描画設定プリミティブ（tex0） */
	DG_GSREG	alpha ;		/* 描画設定プリミティブ（alpha） */
	DG_GSREG	pad0 ;		/* 描画設定プリミティブ（リザーブ） */
	FVECTOR		vec1 ;		/* パラメータ格納用 */
	FVECTOR		vec2 ;		/* パラメータ格納用 */
} DG_TEX_TRANS ;

typedef struct _DG_BP_GSREG_DATA_BP
{
   u_long64 data;
} DG_BP_GSREG_DATA;

typedef struct _dg_tex_trans_BP {
//   DG_GIFTAG giftag;
//   DG_BP_GSREG_DATA tex2;
   DG_BP_GSREG_DATA tex0;
   DG_BP_GSREG_DATA alpha;
   DG_BP_GSREG_DATA clamp;
   FVECTOR vec1;
   FVECTOR vec2;
} DG_TEX_TRANS_BP;

static inline void BP_CopyDGTexTransToDGTexBP( DG_TEX_TRANS_BP* pDst, DG_TEX_TRANS* pOrig )
{
//   pDst->giftag = pOrig->giftag;
//   pDst->tex2 = pOrig->tex2;
   pDst->tex0.data = pOrig->tex0.data;
   pDst->alpha.data = pOrig->alpha.data;
   pDst->clamp.data = pOrig->clamp.data;
   pDst->vec1 = pOrig->vec1;
   pDst->vec2 = pOrig->vec2;
}

	/*
		新テクスチャ情報
	*/
typedef struct _dg_tex {
	float				u_offset ;	/* 使用テクスチャの開始Ｕ座標 */
	float				v_offset ;	/* 使用テクスチャの開始Ｖ座標 */
	float				u_scale ;	/* 使用テクスチャのスケール補正値 */
	float				v_scale ;	/* 使用テクスチャのスケール補正値 */
	unsigned int		tex_id ;	/* テクスチャＩＤ */
	unsigned int		tri_id ;	/* テクスチャアーカイバＩＤ */
	int					BP_flag ;		/* テクスチャフラグ */
#if 0 //BP_PS2
	int					pad[1] ;	/*  */
#else
   unsigned int      BP_TextureHandle;
#endif

	DG_TEX_TRANS		tex_trans ;/* テクスチャパラメータ転送パケット */
} DG_TEX ;

typedef struct _dg_tex_BP {
   int isValid;
   float u_offset;
   float v_offset;
   float u_scale;
   float v_scale;
   unsigned int tex_id ;
   int BP_flag;
   unsigned int BP_TextureHandle;

   DG_TEX_TRANS_BP tex_trans;
   DG_TEX* pDebugOrigSrc;
} DG_TEX_BP;

static inline void BP_CopyDGTexToDGTexBP( DG_TEX_BP* pDst, DG_TEX* pOrig )
{
   if( pOrig )
   {
      pDst->isValid = 1;
      pDst->u_offset = pOrig->u_offset;
      pDst->v_offset = pOrig->v_offset;
      pDst->u_scale = pOrig->u_scale;
      pDst->v_scale = pOrig->v_scale;
      pDst->tex_id = pOrig->tex_id;
      pDst->BP_flag = pOrig->BP_flag;
      pDst->BP_TextureHandle = pOrig->BP_TextureHandle;

      BP_CopyDGTexTransToDGTexBP(&pDst->tex_trans, &pOrig->tex_trans);
   }
   else
   {
      pDst->isValid = 0;
   }
   pDst->pDebugOrigSrc = pOrig;
}

extern void BP_CopyDGTexToDGTexBP_Optional( DG_TEX_BP **ppDst, DG_TEX_BP *pDst, DG_TEX *pOrig );

enum
{
   DG_TEXFLAG_RENDERTARGET = 0x00000001,
   DG_TEXFLAG_NO_UV_PULLIN = 0x00000002,
   DG_TEXFLAG_UV_CLAMP     = 0x00000004,
   DG_TEXFLAG_INJECTED_TEX = 0x00000008
};

/* 線形テクスチャ（ＸＢＯＸ専用＆DG_DMAPACKでのみ使用可能） */
typedef struct _dg_tex_lin 
{
   u_short				width;   /* 横幅 */
   u_short				height;	/* 縦幅 */
   u_short				format;	/* 画像フォーマット */
   u_short				flag;		/* 各種フラグ */
   void*             image;	/* イメージデータ */
   int               image_dirty;
   
   unsigned int      BP_TextureHandle;
} DG_TEX_LIN;

enum 
{
   DG_TEXLIN_FORMAT_A8R8G8B8,		/* ３２ビットテクスチャ */
   DG_TEXLIN_FORMAT_A1R5G5B5,		/* １６ビットテクスチャ */
   DG_TEXLIN_FORMAT_A4R4G4B4,		/* １６ビットテクスチャ */
   DG_TEXLIN_FORMAT_P8,			/* ８ビットパレットテクスチャ（実際には８ビット輝度テクスチャ） */
   DG_TEXLIN_FORMAT_R5G6B5,		/* １６ビットアルファ無し */
   DG_TEXLIN_FORMAT_L8,			/* ８ビット輝度テクスチャ */
   DG_TEXLIN_FORMAT_A8,			/* ８ビットアルファテクスチャ */
   DG_TEXLIN_FORMAT_X1R5G5B5,		/* １６ビットテクスチャアルファ無し２ */
};

	/*
		転送最大サイズのテクスチャ転送パケット構造体
	*/
typedef ALIGN16_DECL(struct) _dg_tex_packet {
	DG_LOADIMAGE	texel0 ;			/* 第１テクセルデータ（最大５０４ＫＢ） */
	DG_LOADIMAGE	texel1 ;			/* 第２テクセルデータ（最大５０４ＫＢ） */
	DG_LOADIMAGE	clut ;				/* ＣＬＵＴデータ */
	DG_DMATAG		rettag ;			/* 終端（RETタグ） */
} DG_TEX_LOAD  ;
typedef DG_TEX_LOAD DG_TEX_PACKET  ;

	/*
		部分テクスチャ転送パケット
	*/
typedef ALIGN16_DECL(struct) _dg_tex_loadreplace {
	DG_LOADIMAGE	texel_load ;
	DG_LOADIMAGE	clut_load ;
	DG_DMATAG		rettag ;			/* 転送終了ＤＭＡタグ */
	/* 接続制御関連 */
	struct _dg_texture_list		*tex_list ;	/* 対象ＴＲＩ */
	struct _dg_tex_loadreplace	*prev ;		/* リンク */
	struct _dg_tex_loadreplace	*next ;		/* リンク */
	int				pad ;
} DG_TEX_LOADREPLACE  ;

	/*
		部分テクスチャ移動パケット
	*/
typedef ALIGN16_DECL(struct) _dg_tex_movereplace {
	DG_MOVEIMAGE	texel_move ;
	DG_MOVEIMAGE	clut_move ;
	DG_DMATAG		rettag ;			/* 転送終了ＤＭＡタグ */
	/* 接続制御関連 */
	struct _dg_texture_list		*tex_list ;	/* 対象ＴＲＩ */
	struct _dg_tex_movereplace	*prev ;		/* リンク */
	struct _dg_tex_movereplace	*next ;		/* リンク */
	int				pad ;
} DG_TEX_MOVEREPLACE  ;

	/*
		テクスチャファイル管理構造体
	*/
typedef ALIGN16_DECL(struct) _dg_texture_list {
	DG_TEX_PACKET	tex_packet[2] ;		/* テクスチャデータ転送ＤＭＡパケット */
	u_long64	load_tex0 ;			/* ＶＲＡＭに読み込まれた際のベースアドレス設定値 */
	int				code ;				/* 読み込んだテクスチャアーカイバのコードＩＤ */
	int				load_tex_base ;		/* ＶＲＡＭに読み込まれた際のベースアドレス */
	int				load_clut_base ;	/* ＶＲＡＭに読み込まれた際のベースアドレス */
	TRI_FILEHEADER	*header ;
	DG_TEX			*textures ;			/* 各テクスチャの情報 */
	int				*texture_image ;	/* テクスチャイメージアドレス */
	int				*clut_image ;		/* ＣＬＵＴイメージアドレス */
	int				*tmp_clut_image ;	/* 一時的ＣＬＵＴイメージアドレス(要8qwordアラインアドレス) */
	int				tex_size ;			/* デバッグ用 */
	short			unused_flag ; // flag ;				/* テクスチャ制御用フラグ */ // texture control flag - set but not read
	short			prio ;				/* テクスチャプライオリティ（＝描画プライオリティ） */
	DG_TEX_LOADREPLACE	*load_link ;		/* テクスチャ部分入れ替えパケットリンク（追加読み込み） */
	DG_TEX_MOVEREPLACE	*move_link ;		/* テクスチャ部分入れ替えパケットリンク（ＶＲＡＭ内移動） */
	int				pad[2] ;
} DG_TEXTURE_LIST  ;

	/*
		ミップマップ管理構造体
	*/
typedef struct _dg_mipmap{
	int			max_level ;		/* 最大ミップマップレベル */
	int			flag ;			/* フラグ */
	float		z0 ;			/* （スクリーン距離＝1.0時の）レベル０距離 */
	int			lod_l ;			/* ミップマップ変化量スケーリング（0(default)~3） */
	int			pad[1] ;		/*  */
	int			tri_id ;		/* ＴＲＩコード */
	int			tex_id[7] ;		/* 各テクスチャＩＤ */
	DG_TEX		*tex[7] ;		/* DG_MakeMipmapInfo()関数で設定 */
	DG_GSREG	tex1 ;			/* DG_MakeMipmapInfo()関数で設定 */
	DG_GSREG	miptbp1 ;		/* DG_MakeMipmapInfo()関数で設定 */
	DG_GSREG	miptbp2 ;		/* DG_MakeMipmapInfo()関数で設定 */
} DG_MIPMAP ;


/*----------------------------------------------------------------*/
/* ＜オブジェクト関連＞ */

	/*
		処理フラグ
	*/
enum {
	DG_FLAG_TEXT			= 0x0000,	/* テクスチャ座標あり（必須）（現在は無視）	*/
	DG_FLAG_TRANS			= 0x0000,	/* 表示座標計算（必須）（現在は無視）		*/
	DG_FLAG_BOUND			= 0x0000,	/* 単位バウンディング（現在は無視）		*/
	DG_FLAG_GBOUND			= 0x0000,	/* 全体バウンディング（現在は無視）		*/
	DG_FLAG_AMBIENT			= 0x0000,	/* アンビエントを固有設定（現在は無視） */
	DG_FLAG_IRTEXTURE		= 0x0000,	/* 赤外線ゴーグル反応モデル（現在は無視） */
	DG_FLAG_ENVMAP			= 0x0000,	/* Enviroment Mapping（現在は無視） */

	DG_FLAG_PAINT			= 0x0001,	/* 点光源計算によるプレシェイド使用		*/
	DG_FLAG_SHADE			= 0x0002,	/* 光源マトリクスによる平行光源シェーディング */
	DG_FLAG_NOFOG			= 0x0004,	/* フォグ効果なし（DG_FLAG_SHADE使用時のみ） */
	DG_FLAG_AUTOREPAINT		= 0x0008,	/* プレシェイドの自動再計算あり		*/
	DG_FLAG_ONEPIECE		= 0x0010,	/* 一体型モデル			*/
	DG_FLAG_FINISHCALC		= 0x0020,	/* オブジェクトマトリクス計算済み（旧名） */
	DG_FLAG_FINISHEDCALC	= 0x0020,	/* オブジェクトマトリクス計算済み（新名） */
	DG_FLAG_QUATROT			= 0x0040,	/* 関節回転をオイラー角ではなくクォータニオンで指定 */
	DG_FLAG_PLUGINDRAW		= 0x0080,	/* 通常描画フェーズでの描画キャンセル（プラグイン用） */
	DG_FLAG_IRREACTION		= 0x0100,	/* 赤外線センサーに反応 */
	DG_FLAG_SEMITRANS		= 0x0200,	/* 強制５０％半透明描画（objs->flagでのみ有効） */
	DG_FLAG_FOGPARAM		= 0x0400,	/* フォグパラメータ指定（DG_FLAG_SHADE使用時のみ） */
	DG_FLAG_LATTERDRAW		= 0x0800,	/* 後回し描画（マルチテクスチャモデルのみ対応） */
	DG_FLAG_INVISIBLE		= 0x3000,	/* 不可視属性（随時変更可能）	*/
	DG_FLAG_INVISIBLE0		= 0x1000,	/* 不可視属性（随時変更可能）（チャンネル０のみ）	*/
	DG_FLAG_INVISIBLE1		= 0x2000,	/* 不可視属性（随時変更可能）（チャンネル１のみ）	*/
	DG_FLAG_INVISIBLE2		= 0x4000,	/* 不可視属性（随時変更可能）（チャンネル２のみ）	*/
	DG_FLAG_INVISIBLE3		= 0x8000,	/* 不可視属性（随時変更可能）（チャンネル３のみ）	*/

	/* システム内部で使用するフラグ */
	DG_FLAG_MULTITEX		= 0x00010000,	/* マルチテクスチャモデル */
	DG_FLAG_DELAYED			= 0x00020000,	/* メモリの１フレーム遅らせ開放フラグ */
   DG_FLAG_AS_NOBOUNDCHECK  = 0x00040000,

   DG_FLAG_FORCEMSAA        = 0x00080000,
   DG_FLAG_NOMSAA           = 0x00100000,
   DG_FLAG_ARM_HIDE_PART    = 0x00200000,

	/* 拡張プラグインで使用されるもの */
	DG_FLAG_PLUGINMASK		= 0xff000000,
	DG_FLAG_SHADOWMAKE		= 0x01000000,	/* 影生成フラグ（要 DG_FLAG_SHADE） */
	DG_FLAG_SHADOWWRITE		= 0x02000080,	/* 影投影モデルフラグ（要 DG_FLAG_SHADE|DG_FLAG_ONEPIECE） */
	DG_FLAG_OPTCMF			= 0x04000080,	/* 光学迷彩処理（要 DG_FLAG_SHADE） */
	DG_FLAG_SHADOWVOL		= 0x08000080,	/* シャドーボリュームモデル（未使用） */
} ;

	/*
		パケット（＝ＤＭＡ転送単位）情報管理構造体
	*/
typedef struct _dg_obj_packet{
	int				flag ;			/* パケットのフラグ */
	void			*data_ptr ;		/* オリジナルＤＭＡデータへのアドレス（RETタグ）未使用 */
	int				pad2 ;			/* リザーブ */
	u_char			n_verts ;		/* そのパケットの頂点数 */
	u_char			pad3 ;			/* リザーブ */
	u_char			verts_offset ;	/* 次の頂点へのオフセット（qword単位） */
	u_char			norms_offset ;	/* 次の法線へのオフセット（qword単位） */
	u_char			uvs_offset[3] ;	/* 次のＵＶへのオフセット（qword単位） */
	u_char			rgbs_offset ;	/* 次のカラーへのオフセット（qword単位） */
	DG_TEX      	*tex_ptr[3] ;	/* テクスチャ設定パケット転送タグへのポインタ */
} DG_OBJ_PACKET ;

	/*
		オブジェクトユニット（＝関節）管理構造体
	*/
typedef ALIGN16_DECL(struct) _DG_OBJ {
	FMATRIX		world ;		/* ワールド座標				*/
	FMATRIX		screen ;	/* スクリーン座標			*/
	FMATRIX		inv_mat ;	/* エンベロープ参照先への相対マトリクス */
	int			flag ;		/* パケットフラグ	*/
	int			sort_z ;	/* 半透明ソート用Ｚ値（符号付き） */
	short		bound_mode ;	/* バウンディング結果（ bound.c ）	*/
	short		parent ;	/* 親オブジェクト番号（model->parentの値） */
	short		n_packs ;	/* オブジェクトパケット数（model->n_packsの値）		*/
	short		mdl_type ;	/* モデルタイプ（model->typeの値） */
	int			vanime_flag ;	/* 頂点アニメーション用フラグ */
	FMATRIX		*light ;	/* 固有光源（ MATRIX×２ ）（半透明オブジェクトのみ有効） */
	DG_MDL		*model ;	/* モデルデータ（ロードデータ）		*/
	SVECTOR		*verts ;	/* 頂点配列バッファ */
	SVECTOR		*norms ;	/* 法線配列バッファ */
	short		*uvs[3] ;	/* ＵＶ配列バッファ（最大３セット） */
	u_int		*rgbs;		/* プリシェードバッファ */
	DG_OBJ_PACKET	*packets ;	/* パケット情報（そのオブジェクトユニットに含まれるパケット数分） */
	int			fog ;			/* 固定フォグ値（システム内部で使用） */
	void		*matrix_addr ;	/* パケットメモリ上に構成したマトリクスパラメータへのポインタ（システム内部で使用） */
	//int			pad[0] ;
	FVECTOR		bound_min ;	/* バウンディング最小値（model->lx,ly,lzの値） */
	FVECTOR		bound_max ;	/* バウンディング最大値（model->ux,uy,uzの値） */
	FVECTOR		trans ;		/* 親オブジェクトからのオフセット（model->tx,ty,tzの値） */
#if 1 //BP
   void*       BP_DGObjs;
   int         BP_ObjIndex;
   int         BP_Model;
   int         BP_PrecomputedChainBuffer;
#endif
} DG_OBJ  ;


	/*
		オブジェクト（＝モデル）管理構造体
	*/
typedef ALIGN16_DECL(struct) _DG_OBJS {
	FMATRIX		world ;			/* ワールド座標				*/
	FMATRIX		*root ;			/* 親座標系（指定された場合のみ）	*/
	DG_DEF		*def ;			/* モデルデータ（ロードデータ）		*/
	struct _DG_OBJS	*next_dgobjs ;		/* 次の関連オブジェクトへのポインタ */
	u_int		flag ;			/* 処理フラグ（ DG_FLAG_* ）		*/
	int			group_id ;		/* 表示グループ				*/
	int			shadow_id ;		/* 影グループ */
	short		n_models ;		/* ユニット数				*/
	short		chanl ;			/* 登録オブジェクトキュー番号（現在未使用）	*/
	short		bound_mode ;	/* バウンディング結果（ bound.c ）	*/
	short		pad1 ;			/* 				*/
	FMATRIX		*light ;		/* 固有光源（ MATRIX×２ ）		*/
	FVECTOR		*rots ;			/* 回転関節データ（指定された場合のみ）	*/
	FVECTOR		*movs ;			/* 横すべり関節（指定された場合のみ）	*/
	u_int		*rgb_buff ;		/* プリシェード用カラーバッファ先頭アドレス */
	int			tri_id ;		/* テクスチャ入れ替え用ＴＲＩ */
	LIT_DEF		*fix_light ;	/* プリシェードに使用したライトへのポインタ（自動再計算用） */
	float		fog_param[2] ;	/* オブジェクト固有フォグパラメータ（DG_FLAG_FOGPARAM指定時のみ有効） */
	void		*extend_data ;	/* 拡張ワークへのポインタ（主にプラグイン関連） */
	char		*fname ;		/* デバッグ用 */
	struct _DG_OBJS	*low ;		/* ローモデルへのポインタ */
#if 0 //BP_PS2
   int			pad2[1] ;		/*  */
#else
   unsigned int   BP_PreshadeBuffer;
   unsigned int   BP_VAnimBuffer;
   int            BP_VAnimBufferTimestamp;
   struct _DG_OBJS *mASBufferedCopy;
   int            mASBufferedCopyTimestamp;
   // Align structure size to 16 bytes (NOTE: Currently ALIGN16 doesn't do anything!)
#endif
	FVECTOR		bound_min ;		/* バウンディング最小値（def->lx,ly,lzの値） */
	FVECTOR		bound_max ;		/* バウンディング最大値（def->ux,uy,uzの値） */
	FVECTOR		trans ;			/* 中心からのオフセット（def->tx,ty,tzの値） */
	DG_OBJ		objs[0] ;		/* 物体ハンドラのユニット		*/
} DG_OBJS  ;

	/*
		可視化
	*/
static	inline	void	DG_VisibleObjs( DG_OBJS *objs )
{
	objs->flag &= ~DG_FLAG_INVISIBLE ;
}

static	inline	void	DG_VisibleObjsChanl( DG_OBJS *objs, int chanl )
{
	objs->flag &= ~( DG_FLAG_INVISIBLE0 << chanl ) ;
}

	/*
		不可視化
	*/
static	inline	void	DG_InvisibleObjs( DG_OBJS *objs )
{
	objs->flag |= DG_FLAG_INVISIBLE ;
}

static	inline	void	DG_InvisibleObjsChanl( DG_OBJS *objs, int chanl )
{
	objs->flag |= ( DG_FLAG_INVISIBLE0 << chanl ) ;
}

/*----------------------------------------------------------------*/
/* ＜マルチウェイトエンベロープオブジェクト関連＞ */

	/*
		処理フラグ
	*/
enum {
	DG_EVMOBJ_IRREACTION	= 0x0010,	/* 赤外線センサーに反応 */
	DG_EVMOBJ_SEMITRANS		= 0x0020,	/* 強制５０％半透明描画 */
	DG_EVMOBJ_LATTERDRAW	= 0x0080,	/* 後回し描画	*/
	DG_EVMOBJ_INVISIBLE		= 0x0300,	/* 不可視属性（随時変更可能）	*/
	DG_EVMOBJ_INVISIBLE0	= 0x0100,	/* 不可視属性（随時変更可能）（チャンネル０のみ）	*/
	DG_EVMOBJ_INVISIBLE1	= 0x0200,	/* 不可視属性（随時変更可能）（チャンネル１のみ）	*/
	DG_EVMOBJ_INVISIBLE2	= 0x0400,	/* 不可視属性（随時変更可能）（チャンネル２のみ）	*/
	DG_EVMOBJ_INVISIBLE3	= 0x0800,	/* 不可視属性（随時変更可能）（チャンネル３のみ）	*/
} ;

	/*
		パケット管理構造体
	*/
typedef struct _dg_evmpack {
	short		flag ;				/* 処理フラグ */
	short		max_mats ;			/* 最大使用スケルトン数 */
	short		n_verts ;			/* パケットに含まれる頂点数 */
	short		n_step ;			/* データのステップ数（1qword単位） */
	u_char		n_uv_step[3] ;		/* 各ＵＶデータのステップ数（1qword単位） */
	char		pad ;
	DG_TEX	*tex0_ptr ;			/* テクスチャ設定パケット転送タグへのポインタ */
	DG_TEX	*tex1_ptr ;			/* テクスチャ設定パケット転送タグへのポインタ */
	DG_TEX	*tex2_ptr ;			/* テクスチャ設定パケット転送タグへのポインタ */
	long64		matrix_list ;		/* 使用スケルトン番号リスト */
} DG_EVMPACK ;

	/*
		マルチウェイトエンベロープオブジェクト管理構造体
	*/
typedef ALIGN16_DECL(struct) _dg_evmobj {
	FMATRIX		world ;				/* 設置中心マトリクス */
	FMATRIX		*root ;				/* 接続先マトリクス */
	FMATRIX		*light ;			/* ライトマトリクスへのポインタ */
	EVM_DEF		*def ;				/* モデルデータへのポインタ */
	int			flag ;				/* 各種処理フラグ */
	int			chanl ;				/* チャンネル */
	int			group_id ;			/* 表示グループ */
	void		*next_unused ;			/* 将来の拡張用 */
	FVECTOR		*skel_bounds ;		/* 各スケルトンのバウンディング(min,max)*n_skeleton */
	int			n_skeleton ;		/* スケルトン数 */
	int			n_packet ;			/* パケットの総数 */
	int			use_buffer ;		/* マトリクスバッファ切り替え */
	FMATRIX		*matrix[2] ;		/* スケルトンのマトリクス配列（ダブルバッファ） */
	void		*verts_ptr ;		/* 頂点データへのポインタ */
	void		*norms_ptr ;		/* 法線データへのポインタ */
	void		*uvs0_ptr ;			/* ＵＶ０データへのポインタ */
	void		*uvs1_ptr ;			/* ＵＶ１データへのポインタ */
	void		*uvs2_ptr ;			/* ＵＶ２データへのポインタ */
	void		*weight_ptr ;		/* ウェイト値データへのポインタ */
	DG_EVMPACK	*packs ;			/* パケット管理構造体 */

#if 1 //BP
   unsigned int   BP_Model;
   int            BP_SpecialModelFlag;
   float          BP_SpecialModelValue;
   int            BP_Pad[1];
#endif

} DG_EVMOBJ  ;

extern int DG_Arm_SkipThisFrame();
extern void DG_Arm_SwitchEvmBuffer(DG_EVMOBJ *evmobj);
extern void DG_WaitForThreadedRenderComplete();

static	inline	void	DG_VisibleEvmChanl( DG_EVMOBJ *evmobj, int chanl )
{
	evmobj->flag &= ~( DG_EVMOBJ_INVISIBLE0 << chanl ) ;
}

static	inline	void	DG_InvisibleEvmChanl( DG_EVMOBJ *evmobj, int chanl )
{
	evmobj->flag |= DG_EVMOBJ_INVISIBLE0 << chanl ;
}

/*----------------------------------------------------------------*/
/* ＜頂点アニメーション関連＞ */

	/*
		頂点アニメ管理ワーク
	*/
typedef struct _dg_v_anime{
	void	*mem[2] ;			/* 取得メモリアドレスポインタ */
	int		flag ;				/* 初期化フラグ記憶 */
	DG_OBJ	*obj ;				/* 初期化オブジェクト */
	void	*verts_top[2] ;	/* 頂点バッファの先頭アドレス */
	void	*norms_top[2] ;	/* 法線バッファの先頭アドレス */
	void	*uvs_top[2] ;	/* ＵＶバッファの先頭アドレス */
	int		buffer_clock ;	/* バッファ切り替え用クロック */
	int		pad[1] ;
} DG_VERTS_ANIME ;

enum {
	DG_VANIME_VERTS = 0x0001,	/* 頂点アニメーションフラグ */
	DG_VANIME_NORMS = 0x0002,	/* 法線アニメーションフラグ */
	DG_VANIME_UVS   = 0x0004,	/* ＵＶアニメーションフラグ */
	DG_VANIME_SINGLE = 0x8000,	/* シングルバッファ作成 */

	/* DG_SUBPACK構造体で使用する */
	DG_VANIME_DELVERTS = 0x0010,	/* 頂点アニメーションフラグ */
	DG_VANIME_DELNORMS = 0x0020,	/* 法線アニメーションフラグ */
	DG_VANIME_DELUVS   = 0x0040,	/* ＵＶアニメーションフラグ */
} ;

static inline void DG_SwitchVAnimeBuffer( DG_VERTS_ANIME *v_anime )
{
   if(!(v_anime->flag & DG_VANIME_SINGLE))
   {
      if (!DG_Arm_SkipThisFrame())
      {
         v_anime->buffer_clock = 1 - v_anime->buffer_clock;
      }
   }
}

/*----------------------------------------------------------------*/
/* ＜共有モデルオブジェクト関連＞ */

enum {
	DG_COMDL_DEFAULT =			0x00000,	/* デフォルトフラグ */
	DG_COMDL_SEMITRANS =		0x00002,	/* 半透明（＝オーバーレイ属性）フラグ */
	DG_COMDL_NOFOG =			0x00004,	/* フォグ無効フラグ */
	DG_COMDL_INVISIBLE =		0x00030,	/* 不可視フラグ */
	DG_COMDL_INVISIBLE0 =		0x00010,	/* 不可視フラグ（チャンネル０のみ） */
	DG_COMDL_INVISIBLE1 =		0x00020,	/* 不可視フラグ（チャンネル１のみ） */
	DG_COMDL_INVISIBLE2 =		0x00040,	/* 不可視フラグ（チャンネル２のみ） */
	DG_COMDL_INVISIBLE3 =		0x00080,	/* 不可視フラグ（チャンネル３のみ） */
} ;

typedef struct _dg_comdl_packet{
	DG_DMATAG	verts_tag ;		/* 頂点転送用ＤＭＡタグ */
	DG_DMATAG	norms_tag ;		/* 法線転送用ＤＭＡタグ */
	DG_DMATAG	uvs_tag ;		/* ＵＶ転送用ＤＭＡタグ */
	DG_DMATAG	datas_tag ;		/* ワークデータ転送用ＤＭＡタグ(STCYCL,UNPACKR) */
	struct _dg_comdl_packet_data {
		u_int		n_verts ;		/* ＶＵ１に渡すデータ（処理頂点数） */
		u_int		data_offset ;	/* ＶＵ１に渡すデータ（バッファ先頭から使用許可領域までのオフセット＝10） */
		u_int		pad1 ;			/* 現在未使用 */
		u_int		pad2 ;			/* 現在未使用 */
		DG_GIFTAG	giftag0 ;		/* クリップポリゴン用描画設定ＧＩＦタグ */
		DG_GSREG	prim ;			/* クリップポリゴン用描画設定プリミティブ（prim） */
		DG_GIFTAG	giftag1 ;		/* クリップポリゴン用描画ＧＩＦタグ */
		DG_GIFTAG	giftag2 ;		/* 描画設定ＧＩＦタグ */
		DG_GSREG	clamp ;			/* 描画設定プリミティブ（clamp） */
		DG_GSREG	tex2 ;			/* 描画設定プリミティブ（tex2） */
		DG_GSREG	tex0 ;			/* 描画設定プリミティブ（tex0） */
		DG_GSREG	alpha ;			/* 描画設定プリミティブ（alpha） */
		DG_GIFTAG	giftag3 ;		/* 描画ＧＩＦタグ */
	} dg_comdl_packet_data ;		/* ワークデータ */
} DG_COMDL_PACKET ;

typedef struct _dg_comdl_pos{
	FMATRIX		world ;
	IVECTOR		color ;
} DG_COMDL_POS ;

typedef struct _dg_comdl {
	int				flag ;				/* 各種フラグ */
	int				n_objs ;			/* オブジェクト表示数 */
	int				group_id ;			/* グループＩＤ */
	int				chanl ;				/* 処理チャンネル */
	int				tri_id ;			/* 使用テクスチャ */
   DG_TEX*        tex;
	int				pad[2] ;
	DG_COMDL_PACKET	packet[2] ;			/* 初期化パケット */
	DG_COMDL_POS	pos[0] ;			/* 各オブジェクトの座標及び色 */
} DG_COMDL ;


/*----------------------------------------------------------------*/
/* ＜プリミティブ関連＞ */
/* !!! 現在未使用 !!! */

	/*
		タイプ（使用ポリゴン）
	*/
#define	DG_PRIM_TYPE	(31)
enum {
	DG_PRIM_LINE_R2,DG_PRIM_LINE_G2,DG_PRIM_LINE_T2,DG_PRIM_LINE_GT2,DG_PRIM_LINE_GTQ2,
	DG_PRIM_POLY_R3,DG_PRIM_POLY_G3,DG_PRIM_POLY_T3,DG_PRIM_POLY_GT3,DG_PRIM_POLY_GTQ3,
	DG_PRIM_POLY_R4,DG_PRIM_POLY_G4,DG_PRIM_POLY_T4,DG_PRIM_POLY_GT4,DG_PRIM_POLY_GTQ4,
	DG_PRIM_SPRT3D1,DG_PRIM_SPRT3D2,
	DG_PRIM_MENU_SPRT,DG_PRIM_MENU_LINE,DG_PRIM_MENU_LINE_G,
	DG_PRIM_FREE,DG_PRIM_VU1
};
/*
	（補足）
	???_R  :単色生プリミティブ
	???_G  :グローシェーディング
	???_T  :補正なしテクスチャ（但し、テクスチャ座標にはＳＴＱを使用する）
	???_GT :グロー＆補正なしテクスチャ
	???_GTQ:グロー＆Ｑ値補正付きテクスチャ（プリミティブ自体は_GTと同じ物であるが初期化時に
			頂点配列意外にＵＶ配列も必要とする）
	SPRT3D1:３次元スプライト。頂点配列のvwでサイズを指定（＝正方形のみ）
	SPRT3D2:３次元スプライト。初期化時に頂点配列と同時にサイズ指定配列を必要とする。

	MENU_SPRT  :メニュー表示用省サイズ型プリミティブ群
	MENU_LINE  :システム側はＤＭＡに繋ぐことしか行なわない（＝２Ｄのみ）
	MENU_LINE_G:パケット自体はPACKEDモードではなくREGLISTモードを使用している
*/

	/*
		タイプ（処理フラグ）
	*/
enum {
	/*
		可視属性
	*/
	DG_PRIM_VISIBLE		= 0x00000000,	/* 可視（随時変更可能）		*/
	DG_PRIM_INVISIBLE	= 0x00010000,	/* 不可視（随時変更可能）	*/
	/*
		座標系
	*/
	DG_PRIM_ON_WORLD	= 0x00000000,	/* 絶対座標			*/
	DG_PRIM_ON_CAMERA	= 0x00020000,	/* カメラ相対			*/
	/*
		処理タイプ
	*/
	DG_PRIM_VERTICES	= 0x00000000,	/* 頂点指定型			*//* 現在未使用 */
	DG_PRIM_RECTANGLE	= 0x00040000,	/* 矩形指定型			*//* 現在未使用 */
	DG_PRIM_ONESIDE		= 0x00080000,	/* 片面ポリゴン *//* 現在未使用 */
	/*
		その他
	*/
	DG_PRIM_SORTONLY	= 0x00100000,	/* ソートのみ（＝固定プリミティブ）			*/
	DG_PRIM_CALLBACK	= 0x00200000,	/* 透視変換フェーズでのコールバック			*/
	DG_PRIM_FREEPRIM	= 0x00400000,	/* 自由形式プリミティブ（初期化、開放も自前で行うこと）			*/
	DG_PRIM_SCREEN		= 0x00800000,	/* 最前面描画型プリミティブ			*/
	DG_PRIM_DESTROY_VU1	= 0x10000000,	/* ＶＵ１マイクロプログラム領域破壊			*/
} ;


typedef ALIGN16_DECL(struct) _DG_PRIM {
	FMATRIX		world ;		/* ワールド座標				*/
	FMATRIX		*root ;		/* 親座標系（指定された場合のみ）	*/
	u_int		type ;		/* タイプ（ DG_PRIM_* ）		*/
	u_int		group_id ;	/* 表示グループ				*/
	int			packet_size ;	/* １パケットのサイズ */
	int			chanl ;		/* 登録オブジェクトキュー番号		*/
	int			n_prims ;	/* １パケット中のポリゴン数				*/
	int			n_packet ;	/* パケット数				*/
	short		prim_size ;		/* タイプ別パラメータ（プリミティブサイズ）			*/
	short		prim_nverts ;	/* タイプ別パラメータ（プリミティブの頂点数）			*/
	short		prim_voffset ;	/* タイプ別パラメータ（先頭頂点へのオフセット）			*/
	short		prim_vstep ;	/* タイプ別パラメータ（次頂点へのオフセット）			*/
	int			raise ;		/* 優先調整（プラスで手前へ）（±4095以内） */
	int			near_z ;	/* クリップＺ */
	void		(*callback)(struct _DG_PRIM *, int which );	/* DG_PRIM_CALLBACK用コールバックポインタ */
	FVECTOR		*pos ;		/* 計算座標バッファ			*/
	FVECTOR		*uvs ;		/* パースペクティブコレクト用基準ＳＴ値 */
	u_long128	*packs[ 2 ] ;	/* パケット配列（つねに確保されている）	*/
} DG_PRIM  ;

	/*
		可視化
	*/
static	inline	void	DG_VisiblePrim( DG_PRIM *prim )
{
	prim->type &= ~DG_PRIM_INVISIBLE ;
}

	/*
		不可視化
	*/
static	inline	void	DG_InvisiblePrim( DG_PRIM *prim )
{
	prim->type |= DG_PRIM_INVISIBLE ;
}

	/*
		パケット優先値設定
	*/
static	inline	void	DG_RaisePrim( DG_PRIM *prim, int raise )
{
	prim->raise = raise ;
}
/*----------------------------------------------------------------*/
/* ＜新プリミティブ関連＞ */

enum {
	/* プリミティブタイプ */
	DG_PRIM2_LINE =			0x0001,		/* ラインストリップ */
	DG_PRIM2_POLY =			0x0002,		/* ポリゴンストリップ */
	DG_PRIM2_CULLPOLY =		0x0003,		/* カリング付きポリゴンストリップ */
	DG_PRIM2_SPRT =			0x0004,		/* 通常スプライト */
	DG_PRIM2_RSPRT =		0x0005,		/* 回転スプライト */
	/* 制御関連 */
	DG_PRIM2_TYPEMASK =		0x000f,		/* プログラム内部で使用 */
	DG_PRIM2_NOBUFFER =		0x0100,		/* 頂点用メモリを確保しない（DG_SetPrim2Buffer()の呼び出し必須） */
	DG_PRIM2_SINGLEBUFF =	0x0200,		/* シングルバッファフラグ（メモリ節約） */
	DG_PRIM2_ON_CAMERA =	0x0400,		/* カメラからの相対座標による指定 */
	DG_PRIM2_BOUNDCHECK =	0x0800,		/* バウンディングチェックの許可 */
	DG_PRIM2_INVISIBLE =	0x3000,		/* 非表示フラグ */
	DG_PRIM2_INVISIBLE0 =	0x1000,		/* 非表示フラグ（チャンネル０のみ） */
	DG_PRIM2_INVISIBLE1 =	0x2000,		/* 非表示フラグ（チャンネル１のみ） */
	DG_PRIM2_INVISIBLE2 =	0x4000,		/* 非表示フラグ（チャンネル２のみ） */
	DG_PRIM2_INVISIBLE3 =	0x8000,		/* 非表示フラグ（チャンネル３のみ） */
	/* プリミティブ属性 */
	DG_PRIM2_SHADE =		0x00010000,	/* シェーディングＯＮ（スプライトでは禁止） */
	DG_PRIM2_TEX =			0x00020000,	/* テクスチャＯＮ */
	DG_PRIM2_FOG =			0x00040000,	/* フォグＯＮ */
	DG_PRIM2_ALPHA =		0x00080000,	/* アルファブレンドＯＮ */
	DG_PRIM2_ANTIALIASING =	0x00100000,	/* アンチエイリアシングＯＮ（使用注意！） */
   /* XBOX拡張 */
   DG_PRIM2_CW = 			   0x01000000,    /* 時計回り */
   DG_PRIM2_CCW = 			0x02000000,    /* 時計回り */
   DG_PRIM2_NOMSAA =       0x04000000,
   DG_PRIM2_FRAMETEX =		0x10000000,		/* フレームバッファテクスチャ使用フラグ */

   DG_PRIM_AS_CUSTOMWORLD  = 0x20000000,
   DG_PRIM_BP_OFFSCREEN    = 0x40000000,
   DG_PRIM_NO_DEPTH_TEST	= 0x80000000,
};

typedef struct _dg_prim2_param{
	FMATRIX		screen ;		/* 透視変換マトリクス */
	int			n_verts ;		/* 頂点数 */
	int			flag ;			/* 処理フラグ */
	int			pad0 ;
	int			pad1 ;
	DG_GIFTAG	giftag_normal ;	/* 通常描画用描画ＧＩＦタグ */
	DG_GIFTAG	giftag_clip ;	/* クリップポリゴン用描画ＧＩＦタグ */
} DG_PRIM2_PARAM ;				/* サブパケットデータ */

/* ライン・ポリゴン用頂点要素 */
typedef struct _dg_prim2_uvrgb {
#if 0 //BP_PS2
   u_short		r, g, b, a ;	/* 頂点カラー */
#else
   unsigned char r, r_pad, g, g_pad, b, b_pad, a, a_pad;
#endif
	u_short		u, v, q, f ;	/* ＵＶＱ,フラグ（描画キック：0x0fff、頂点キックのみ：0x8fff） */
	/* ＜補足＞DG_PRIM2_CULLPOLYの時のフラグ設定
	   両面表示：0x0fff 右回り表示：0x0020 左回り表示：0x0000 非表示：0x8fff */
} DG_PRIM2_UVRGB ;

/* スプライト用頂点要素 */
typedef struct _dg_prim2_wh {
#if 0 //BP_PS2
	u_short		r, g, b, a ;		/* スプライトカラー */
#else
   unsigned char r, r_pad, g, g_pad, b, b_pad, a, a_pad;
#endif
	u_short		u0, v0, q0, f0 ;	/* 左上ＵＶＱ（f0は無視） */
	short		w, h, pad0, pad1 ;	/* スプライトサイズ */
	u_short		u1, v1, q1, f1 ;	/* 右下ＵＶＱ（f1は無視） */
} DG_PRIM2_UVRGBWH ;

/* パケット管理構造体 */
typedef struct _dg_prim2_packet {
	int				sort_z ;		/* ソート用Ｚ値 */
	void			*next_addr ;	/* ソート用リスト構造ポインタ */
	void			*prim ;			/* DG_PRIM2へのポインタ */
	short			type ;			/* プリミティブタイプ */
	u_short			flag ;			/* ローカルフラグ */
	short			n_verts ;		/* 転送頂点数 */
	short			pad ;
	int				pad2 ;
	FVECTOR			*pos_addr ;		/* 頂点座標アドレス */
	void			*uvrgb_addr ;	/* 頂点要素アドレス（ＵＶ、ＲＧＢなど） */
#if 1 //BP
   int         BP_startIndex;
   int         BP_indexCount;
#endif
} DG_PRIM2_PACKET ;

	/*
		新プリミティブオブジェクト
	*/
typedef ALIGN16_DECL(struct) _dg_prim2 {
	FMATRIX			as_world ;			/* マトリクス */
	FMATRIX			screen ;		/* 透視変換マトリクス */
	FVECTOR			bound_min ;		/* バウンディング最小値（DG_PRIM2_BOUNDCHECK指定時のみ有効） */
	FVECTOR			bound_max ;		/* バウンディング最大値（DG_PRIM2_BOUNDCHECK指定時のみ有効） */
	FMATRIX			*root ;			/* 接続マトリクスへのポインタ */
	int				flag ;			/* 処理フラグ */
	int				group_id ;		/* 表示グループ */
	short			chanl ;			/* 使用チャンネル */
	short			type ;			/* プリミティブタイプ */
	short			n_prims ;		/* パケットの個数 */
	short			packet_verts ;	/* １プリミティブの頂点数 */
	int				buffer_clock ;	/* ダブルバッファ切り替え用 */
	FVECTOR			*pos[2] ;		/* 頂点座標配列 */
	void			*uvrgb[2] ;		/* ＵＶ、ＲＧＢ値他頂点データ配列 */
	DG_PRIM2_PARAM	prim_param[2] ;	/* プリミティブパラメータ */
	DG_TEX_TRANS	tex_trans ;		/* テクスチャパラメータ転送パケット */
#if 1 //BP
   int            BP_forceZWrite;
   int            BP_forceDisableAlphaTest;
   DG_TEX*        BP_tex;
   void*          BP_vertexBuffer;
   int            BP_vertexCount;
   int            BP_indexBuffer[3];
   int            BP_instanced;
   void*          BP_localUniformBuffer;
#endif

	DG_PRIM2_PACKET	*packet[2] ;	/* パケットデータ */
	int				raise ;			/* ソート用優先上げ値 */
	char			*fname ;		/* デバッグ用 */
} DG_PRIM2  ;


	/*
		パケットバッファ切り替え
	*/
static	inline	int	DG_SwitchBuffPrim2( DG_PRIM2 *prim )
{
   if (!DG_Arm_SkipThisFrame())
   {
   	prim->buffer_clock = 1 - prim->buffer_clock;
      return 1;
   }
   return 0;
}

static inline int AS_WillPrimBuffSwitch()
{
   return !DG_Arm_SkipThisFrame();
}

static inline int AS_IsRunningAt30Fps()
{
#ifdef ARMATURE_RENDER_FRAME_SKIP
   extern int BP_Renderer_MGS2_3060_IsAt60();

   return !BP_Renderer_MGS2_3060_IsAt60();
#else
   return 0;
#endif
}

	/*
		可視化
	*/
static	inline	void	DG_VisiblePrim2( DG_PRIM2 *prim )
{
	prim->flag &= ~DG_PRIM2_INVISIBLE ;
}

static	inline	void	DG_VisiblePrim2Chanl( DG_PRIM2 *prim, int chanl )
{
	prim->flag &= ~( DG_PRIM2_INVISIBLE0 << chanl ) ;
}

	/*
		不可視化
	*/
static	inline	void	DG_InvisiblePrim2( DG_PRIM2 *prim )
{
	prim->flag |= DG_PRIM2_INVISIBLE ;
}

static	inline	void	DG_InvisiblePrim2Chanl( DG_PRIM2 *prim, int chanl )
{
	prim->flag |= DG_PRIM2_INVISIBLE0 << chanl ;
}

	/*
		パケット優先値設定
	*/
static	inline	void	DG_RaisePrim2( DG_PRIM2 *prim, int raise )
{
	prim->raise = raise ;
}

	/*
		アルファブレンディングモードの設定
	*/
static inline void DG_SetPrim2Alpha( DG_PRIM2 *prim, u_long64 alpha )
{
	prim->tex_trans.alpha.data = alpha ;
}


/*----------------------------------------------------------------*/
/* ＜その他のオブジェクト関連＞ */

typedef struct _dg_spot {
	FMATRIX		world ;			/*  */
	FMATRIX		screen ;		/* 投影マトリクス */
	FMATRIX		*root ;			/*  */
	int			flag ;			/* フラグ */
	u_int		color ;			/* 色 */
	float		angle ;			/* 影響角度  */
	float		range ;			/* 影響距離 */
	int			shadow_id ;		/* 影グループ */
	void		*objs ;			/* 平行投影影用オブジェクトへのポインタ */
	int			pad[1] ;
} DG_SPOT ;

/*----------------------------------------------------------------*/
/* ＜描画環境関連＞ */

	/*
		描画環境設定構造体
	*/
typedef ALIGN16_DECL(struct) _dg_drawenv {
	u_long64			gif_tag ;
	u_long64			reg_list ;
	struct {
		sceGsFrame		frame1 ;
		u_long64			frame1_addr ;
		sceGsZbuf		zbuf1 ;
		u_long64			zbuf1_addr ;
		sceGsXyoffset	xyoffset1 ;
		u_long64			xyoffset1_addr ;
		sceGsScissor	scissor1 ;
		u_long64			scissor1_addr ;
		sceGsPrmodecont	prmodecont ;
		u_long64			prmodecont_addr ;
		sceGsColclamp	colclamp ;
		u_long64			colclamp_addr ;
		sceGsTest		test1 ;
		u_long64			test1_addr ;
		sceGsPabe		pabe ;
		u_long64			pabe_addr ;
		sceGsTexa		texa ;
		u_long64			texa_addr ;
		sceGsDthe		dthe ;
		u_long64			dthe_addr ;
		sceGsDimx		dimx ;
		u_long64			dimx_addr ;
		sceGsClamp		clamp ;
		u_long64			clamp_addr ;
	} datas ;
} DG_DRAWENV  ;
typedef ALIGN16_DECL(struct) _dg_drawoffset {
	DG_GIFTAG		gif_tag ;
	struct {
		sceGsXyoffset	xyoffset1 ;
		u_long64			xyoffset1_addr ;
	} datas ;
} DG_DRAWOFFSET  ;
typedef ALIGN16_DECL(struct) _dg_drawfog {
	DG_GIFTAG		gif_tag ;
	struct {
		sceGsFogcol		fogcol ;
		u_long64			fogcol_addr ;
	} datas ;
} DG_DRAWFOG  ;

#if 1 //BP_PS2

#define DG_DRAWENV_FRAME		I64(0x0000000000000001)
#define DG_DRAWENV_ZBUF			I64(0x0000000000000010)
#define DG_DRAWENV_XYOFFSET	I64(0x0000000000000100)
#define DG_DRAWENV_SCISSOR		I64(0x0000000000001000)
#define DG_DRAWENV_PRMODECONT	I64(0x0000000000010000)
#define DG_DRAWENV_COLCLAMP	I64(0x0000000000100000)
#define DG_DRAWENV_TEST			I64(0x0000000001000000)
#define DG_DRAWENV_PABE			I64(0x0000000010000000)
#define DG_DRAWENV_TEXA			I64(0x0000000100000000)
#define DG_DRAWENV_DTHE			I64(0x0000001000000000)
#define DG_DRAWENV_DIMX			I64(0x0000010000000000)
#define DG_DRAWENV_CLAMP		I64(0x0000100000000000)

#else

enum {
	DG_DRAWENV_FRAME		= 0x0000000000000001,
	DG_DRAWENV_ZBUF			= 0x0000000000000010,
	DG_DRAWENV_XYOFFSET		= 0x0000000000000100,
	DG_DRAWENV_SCISSOR		= 0x0000000000001000,
	DG_DRAWENV_PRMODECONT	= 0x0000000000010000,
	DG_DRAWENV_COLCLAMP		= 0x0000000000100000,
	DG_DRAWENV_TEST			= 0x0000000001000000,
	DG_DRAWENV_PABE			= 0x0000000010000000,
	DG_DRAWENV_TEXA			= 0x0000000100000000,
	DG_DRAWENV_DTHE			= 0x0000001000000000,
	DG_DRAWENV_DIMX			= 0x0000010000000000,
	DG_DRAWENV_CLAMP		= 0x0000100000000000,
};

#endif

#define DG_ENABLE_DRAWENV( _env, flag ) {(_env)->reg_list = I64(0xffffffffffffffff) - (flag) ;}


/*----------------------------------------------------------------*/
/* ＜チャンネル関連＞ */

	/*
		オブジェクトキューバッファ
	*/
typedef struct _dg_obj_buffer{
	int			max_queue ;			/* 使用可能な最大数 */
	int			n_queue ;			/* 現在使用中のキューの数 */
	void		**queue ;			/* キューバッファへのポインタ */
	int			id ;				/* 識別ＩＤ */
} DG_OBJ_BUFFER ;

	/*
		オブジェクトキュー
	*/
typedef struct {
	/*
		新管理方式構造体
	*/
	DG_OBJ_BUFFER	objs_buffer ;				/* 通常オブジェクト用キューバッファ */
	DG_OBJ_BUFFER	evmobj_buffer ;				/* マルチウェイトエンベロープ用キューバッファ */
	DG_OBJ_BUFFER	prim2_buffer ;				/* 新プリミティブ用キューバッファ */
	/* 以下拡張 */
	DG_OBJ_BUFFER	comdl_buffer ;				/* 共有モデルオブジェクト用キューバッファ */
	DG_OBJ_BUFFER	shdwwrite_buffer ;			/* 影投影オブジェクト用キューバッファ */
	DG_OBJ_BUFFER	spot_buffer ;				/* 投影スポットオブジェクト用キューバッファ */
	DG_OBJ_BUFFER	user_buffer[DG_MAX_USR_OBJQUEUE] ;	/* 拡張用リザーブバッファ（プラグインで使用） */
} DG_OBJ_QUEUE ;

	/*
		チャンネルデータ
	*/
typedef ALIGN16_DECL(struct)	_dg_chanl {
	/*
		カメラデータ
	*/
	FMATRIX		eye_pers ;			/* カメラ透視変換マトリクス（=pers*eye_inv） */
	FMATRIX		eye_inv ;			/* カメラ逆行列				*/
	FMATRIX		eye ;				/* カメラ行列				*/
	FMATRIX		pers ;				/* 透視変換マトリクス */
	FMATRIX		eye_pers2 ;			/* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */
	FMATRIX		pers2 ;				/* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
	FMATRIX		raise_pers ;		/* 透視変換マトリクス */
	FMATRIX		raise_pers2 ;		/* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
	FMATRIX		raise_eye_pers ;	/* カメラ透視変換マトリクス */
	FMATRIX		raise_eye_pers2 ;	/* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */

   //BP_CAMERA - extra matrices that do not include camera tweak offset
   FMATRIX		pers_no_offset;
   FMATRIX		eye_pers_no_offset;
   //BP_CAMERA - extra matrices that do not include camera tweak offset

	float		screen ;			/* カメラ→スクリーンキョリ（default = 2.0）		*/

	/*
		オブジェクト管理情報
	*/
	int				flag ;			/* チャンネルフラグ */
	int				group_id ;		/* 有効グループＩＤ */
	DG_OBJ_QUEUE	*obj_queue ;	/* 処理対象オブジェクトキュー */
	int				n_stage ;		/* チャンネル処理ステージ数 */
	void	(**stage_list)( struct _dg_chanl *, int ) ;
									/* チャンネル処理ステージ関数リスト */
	/*
		描画環境など
	*/
	int			width ;				/* 描画幅（最大512） */
	int			height ;			/* 描画高さ（最大256） */
	int			offset_x ;
	int			offset_y ;
	DG_DRAWENV		draw_env[2] ;	/* デフォルト描画環境設定 */
	DG_DRAWOFFSET	draw_offset[2] ;/* ハイレゾ化に必要な環境設定 */
	int			bg_clear_flag ;		/* 背景クリアフラグ */
	int			chanl_num ;			/* チャンネル番号 */
	int			high_reso ;			/* オフセット変更によるハイレゾ化有効フラグ */
} DG_CHANL  ;


	/*
		チャンネル処理ユニット
	*/
typedef	void	( *QueFunc )( DG_CHANL *, int ) ;


	/*
		チャンネル、オブジェクトキューのバッファサイズ
	*/
enum {
	DG_CHANL_MAIN,
	DG_CHANL_SUB1,
	DG_CHANL_SUB2,
	DG_CHANL_SUB3,
	DG_CHANL_MENU,
	DG_MAX_CHANLS
};
//#define	DG_MAX_CHANLS	(5)

	/*
		チャンネル番号
	*/



	/*
		番号→チャンネル構造体
	*/
#define DG_Chanl( _chanl ) ( DG_Chanls + _chanl )

/*----------------------------------------------------------------*/
/* ＜ＬＩＢＤＧプラグイン管理機構＞ */

/* 処理フェーズレベル */
enum {
	DG_PLUGIN_PHASE_FIRST		= 0x01,	/* 描画開始前 */
	DG_PLUGIN_PHASE_NORMAL		= 0x02,	/* 不透明モデル描画後 */
	DG_PLUGIN_PHASE_AFTER		= 0x04,	/* 半透明モデル描画後 */
	DG_PLUGIN_PHASE_LAST		= 0x08,	/* 最終描画 */
	DG_PLUGIN_PHASE_END1		= 0x10,	/* リザーブ */
} ;

/* プライオリティ設定 */
enum {
	DG_PLUGIN_PRIO_NORMAL		= 0,		/* デフォルトプライオリティ */
};

/* 初期化フラグ */
enum {
	DG_PLUGIN_FLAG_OBJBUFFER	= 0x0001,	/* オブジェクトバッファ使用 */
	DG_PLUGIN_FLAG_ENABLE0		= 0x0010,	/* チャンネル０で動作 */
	DG_PLUGIN_FLAG_ENABLE1		= 0x0020,	/* チャンネル１で動作 */
	DG_PLUGIN_FLAG_ENABLE2		= 0x0040,	/* チャンネル２で動作 */
	DG_PLUGIN_FLAG_ENABLE3		= 0x0080,	/* チャンネル３で動作 */
	DG_PLUGIN_FLAG_ENABLEMENU	= 0x0100,	/* チャンネル４（メニュー用）で動作 */
	DG_PLUGIN_FLAG_ENABLE		= 0x00f0,	/* 全チャンネルで動作 */
};

typedef void (*DG_PluginCallback)( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buffer, int status );

/* プラグイン管理構造体 */
typedef struct _dg_plugin {
	struct _dg_plugin	*prev ;
	struct _dg_plugin	*next ;
	int		flag ;							/* 初期化フラグ */
	int		object_id ;						/* オブジェクト固有ＩＤ */
	int		phase ;							/* 実行フェーズ指定 */
	int		priority ;						/* 同フェーズ内での優先レベル */
	DG_OBJ_BUFFER	*obj_buffer ;        /* 確保オブジェクトバッファ */
	DG_PluginCallback		exec_func ;		/* 実行コールバック関数 */
} DG_PLUGIN ;

DG_PLUGIN *DG_AS_GetFirstPlugin();
DG_PLUGIN *DG_AS_GetLastPlugin();

/*----------------------------------------------------------------*/
/* ＜ＤＭＡ制御関連＞ */

	/*
		ＤＭＡ関連構造体
	*/
typedef void (*DG_DmaCallback)( int ) ;

	/*
		現在のＤＭＡ書き込みバッファアドレスを取得
	*/
#define DG_GetCurrentDmaAddr()	( (void*)DG_CurrentDmaAddr )

	/*
		バッファサイズなど
	*/
#define DG_MAX_DMATASK	(64)

	/*
		DG_OpenDmaTask()関数用ＤＭＡタイプ
	*/
#define DG_OPEN_DMA_VIF1	(0)		/* VIF1によるソースチェインＤＭＡ */
#define DG_OPEN_DMA_GIF		(1)		/* GIFによるソースチェインＤＭＡ（現在未使用） */
#define DG_OPEN_DMA_STORE	(2)		/* ストアイメージ専用ＤＭＡ（VIF1） */


/*----------------------------------------------------------------*/
	/*
		パフォーマンス詳細構造体
	*/
typedef struct _performance_packet_info {
	int		n_obj ;			/* 処理したオブジェクト数 */
	int		n_packs ;		/* 転送したＤＭＡパケット数 */
	int		n_verts ;		/* 転送したＤＭＡパケットに含まれる頂点数 */
} PERFORMANCE_PACKET_INFO ;
typedef struct _dg_performance_data {
	PERFORMANCE_PACKET_INFO	normal_model[ 2 ] ;	/* 通常モデル */
	PERFORMANCE_PACKET_INFO	trans_model[ 2 ] ;	/* 半透明モデル */
	PERFORMANCE_PACKET_INFO	multex_model[ 2 ] ;	/* マルチテクスチャモデル */
	PERFORMANCE_PACKET_INFO	mulwt_model ;		/* マルチテクスチャモデル */
	PERFORMANCE_PACKET_INFO	prim_poly ;			/* ポリゴンプリミティブ */
	PERFORMANCE_PACKET_INFO	prim_line ;			/* ラインプリミティブ */
	PERFORMANCE_PACKET_INFO	prim_sprt ;			/* スプライトプリミティブ */
	PERFORMANCE_PACKET_INFO	prim_rsprt ;		/* 回転スプライトプリミティブ */
	PERFORMANCE_PACKET_INFO	shadow_projection[2] ;	/* 影投影モデル */
	PERFORMANCE_PACKET_INFO	spot_projection[2] ;	/* スポット投影モデル */
	PERFORMANCE_PACKET_INFO	shadow_draw[2] ;		/* 影モデル */
	PERFORMANCE_PACKET_INFO	comodel ;			/* 共有モデルオブジェクト */
	int						use_tex_size ;		/* 使用テクスチャサイズ */
} DG_PERFORMANCE_DATA ;
extern DG_PERFORMANCE_DATA	DG_PerformanceData ;
#ifdef LIBDG_PERFORMANCE
#endif

/*----------------------------------------------------------------*/
	/*
		DG_DisplayStatus用定義
	*/
enum {
	DG_STATE_IR_MODE			= 0x00000001,		/* 赤外線モード */
};
/*----------------------------------------------------------------*/

	/*
		ＬＩＢＤＧ定義グローバル変数
	*/

typedef struct SPerThreadDGVars_s
{
   int mClock;
   int mUnDrawFrameCount;
   int mLastWhich;
   int mLightClock;
   int mPrivilegeMode;
   int pad[3];
   FMATRIX mLightMatrix; // Make sure that it's "color matrix" then 
   FMATRIX mColorMatrix; // "light matrix" in case some jerk assumes
   // that they're back to back
   FVECTOR mBP_FogParam;
   float mFogParam1;
   float mFogParam2;
   float pad2[2];
} SPerThreadDGVars;

extern SPerThreadDGVars DG_AS_PerThreadVars[2];
#if BP_VITA
__thread
#endif
extern int DG_AS_CurrentThreadVarsIndex;

#define DG_AS_CurrentThreadVars ( DG_AS_PerThreadVars[ DG_AS_CurrentThreadVarsIndex ] )
#define DG_Clock ( DG_AS_CurrentThreadVars.mClock )
#define DG_UnDrawFrameCount ( DG_AS_CurrentThreadVars.mUnDrawFrameCount )
#define DG_LastWhich ( DG_AS_CurrentThreadVars.mLastWhich )
#define DG_LightClock ( DG_AS_CurrentThreadVars.mLightClock )
#define DG_LightMatrix ( DG_AS_CurrentThreadVars.mLightMatrix )
#define DG_ColorMatrix ( DG_AS_CurrentThreadVars.mColorMatrix )
#define DG_PrivilegeMode ( DG_AS_CurrentThreadVars.mPrivilegeMode )
#define DG_FogParam1 ( DG_AS_CurrentThreadVars.mFogParam1 )
#define DG_FogParam2 ( DG_AS_CurrentThreadVars.mFogParam2 )
#define BP_FogParam ( DG_AS_CurrentThreadVars.mBP_FogParam )

extern	int			DG_TickCount ;			/* 起動からの総VSyncカウント数 */
extern	int			DG_PassageTick ;		/* １アクト中に発生したVSyncカウント数 */
extern	int			DG_FrameRate ;			/* フレームレート調整 */
extern	int			DG_CurrentField;		/* odd or even */
extern	DG_CHANL	DG_Chanls[] ;			/* チャンネル構造体 */
extern	DG_OBJ_QUEUE DG_ObjQueue ;		/* オブジェクトキュー構造体 */
extern	u_long128	*DG_CurrentDmaAddr ;	/* カレントＤＭＡ先頭バッファ */
extern	u_long128	*DG_CurrentDmaEnd;		/* ＤＭＡパケットの書き込み限界アドレス */
extern	CVECTOR		DG_FogColorMaster ;		/* 遠景色（設定値） */
extern	CVECTOR		DG_FogColor ;			/* 実効遠景色（場合により加工されたものが入る） */
//extern	float		DG_FogParam1 ;			/* フォグ計算パラメータ */
//extern	float		DG_FogParam2 ;			/* フォグ計算パラメータ */
//extern   FVECTOR  BP_FogParam;
extern	int			DG_FrameCount;			/*  */
//extern	int			DG_PrivilegeMode ;		/* 特権モードフラグ */
extern	int			DG_DisplayStatus ;		/* 各種画面表示状態 */
extern   int         DG_Endframe_FrameCount ;
extern   int         DG_Renderframe_FrameCount; // synced with the endframe framecount

extern SVECTOR	DG_ZeroSVector ;
extern IVECTOR	DG_ZeroIVector ;
extern FVECTOR	DG_ZeroVector ;
extern FMATRIX	DG_UnitMatrix ;

extern int				DG_MaxTextures ;
extern DG_TEXTURE_LIST	*DG_TextureList[] ;
extern DG_TEX			DG_SystemTexture[] ;
extern DG_TEX        BP_PreviousFrameTexture;
extern DG_TEX        BP_CurrentFrameTexture;

/*
   DG_SystemTexture[0]:フレームバッファ０（512x512x32bit）
   DG_SystemTexture[1]:フレームバッファ１（512x512x32bit）
   DG_SystemTexture[2]:テクスチャバッファ（512x512x32bit）
   DG_SystemTexture[3]:テクスチャバッファ（512x512x16bit）
*/

//extern	u_short	DG_PaletteBuffer[];

/*----------------------------------------------------------------*/

	/*
		インラインマクロ
	*/

#if 1 //BP_PS2


#if 0 //BP_MATH - handle unaligned memory copies
#define DG_COPY_VEC(a, b) (*(FVECTOR *)(a) = *(FVECTOR *)(b))
#define DG_COPY_MAT(a, b) (*(FMATRIX *)(a) = *(FMATRIX *)(b))

typedef struct tagDG_ULONG128 {
   u_long64 l, m;
} DG_ULONG128;
#define DG_COPY128(a, b) {*(DG_ULONG128 *)(a) = *(DG_ULONG128 *)(b);}
#define DG_COPY64(a, b)  {*(u_long64 *)(a) = *(u_long64 *)(b);}

#else

static inline void bp_copy_vec( FVECTOR* dst, const FVECTOR* src )
{
   dst->vx = src->vx;
   dst->vy = src->vy;
   dst->vz = src->vz;
   dst->vw = src->vw;
}

static inline void bp_copy_mat( FMATRIX* dst, const FMATRIX* src )
{
   dst->m[0][0] = src->m[0][0];
   dst->m[0][1] = src->m[0][1];
   dst->m[0][2] = src->m[0][2];
   dst->m[0][3] = src->m[0][3];
                         
   dst->m[1][0] = src->m[1][0];
   dst->m[1][1] = src->m[1][1];
   dst->m[1][2] = src->m[1][2];
   dst->m[1][3] = src->m[1][3];
                         
   dst->m[2][0] = src->m[2][0];
   dst->m[2][1] = src->m[2][1];
   dst->m[2][2] = src->m[2][2];
   dst->m[2][3] = src->m[2][3];
                         
   dst->m[3][0] = src->m[3][0];
   dst->m[3][1] = src->m[3][1];
   dst->m[3][2] = src->m[3][2];
   dst->m[3][3] = src->m[3][3];
}

static inline void bp_copy_64( int* dst, const int* src )
{
   dst[0] = src[0];
   dst[1] = src[1];
}

static inline void bp_copy_128( int* dst, const int* src )
{
   dst[0] = src[0];
   dst[1] = src[1];
   dst[2] = src[2];
   dst[3] = src[3];
}

#define DG_COPY_VEC(a, b)  bp_copy_vec( (FVECTOR*)(a), (const FVECTOR*)(b) )
#define DG_COPY_MAT(a, b)  bp_copy_mat( (FMATRIX*)(a), (const FMATRIX*)(b) )

#define DG_COPY64(a, b)    bp_copy_64( (int*)(a), (const int*)(b) )
#define DG_COPY128(a, b)   bp_copy_128( (int*)(a), (const int*)(b) )

#endif


#define DG_FTOI(_f)      ((int)(_f))

static float inline DG_MAX( float a, float b )
{
   return (a > b) ? a : b;
}

static float inline DG_MIN( float a, float b )
{
   return (a > b) ? b : a;
}

static float inline DG_SQRT( float a )
{
   return bp_sqrtf(a);  //BP_MATH - emulate PS2 sqrtf
}

static float inline DG_RSQRT( float a )
{
   return 1.0f / DG_SQRT(a);
}

static float inline DG_FABS( float x ) 
{
   return (float)fabs(x);
}

extern float DG_ACC; // アキュムレータ 
#define	DG_ADDA(_x,_y)	{DG_ACC = (_x) + (_y);} /* ACC = x + y */
#define	DG_SUBA(_x,_y)	{DG_ACC = (_x) - (_y);} /* ACC = x - y */
#define	DG_MULA(_x,_y)	{DG_ACC = (_x) * (_y);} /* ACC = x * y */
#define	DG_MADDA(_x,_y)	{DG_ACC += (_x) * (_y);}/* ACC += x * y */
#define	DG_MSUBA(_x,_y)	{DG_ACC -= (_x) * (_y);}/* ACC -= x * y */
static inline float DG_MADD(float _x, float _y) {return DG_ACC + _x * _y;}
static inline float DG_MSUB(float _x, float _y) {return DG_ACC - _x * _y;}

#define DG_SetScratchStack( addr )
#define DG_ResetScratchStack()

//BP - copied from libgdx.h / libgdw.h
// clipw 命令エミュレーション用フラグ(VU User's Manual参照) '
enum {
   CLIP_X0_FLAG = (1 << 0),  // x > + w
   CLIP_X1_FLAG = (1 << 1),  // x < - w
   CLIP_Y0_FLAG = (1 << 2),  // y > + w
   CLIP_Y1_FLAG = (1 << 3),  // y < - w
   CLIP_Z0_FLAG = (1 << 4),  // z > + w
   CLIP_Z1_FLAG = (1 << 5),  // z < - w

   // 上のフラグをまとめた定義 
   CLIP_X_FLAG = (CLIP_X0_FLAG|CLIP_X1_FLAG),
   CLIP_Y_FLAG = (CLIP_Y0_FLAG|CLIP_Y1_FLAG),
   CLIP_Z_FLAG = (CLIP_Z0_FLAG|CLIP_Z1_FLAG),
   CLIP_XY_FLAG = (CLIP_X_FLAG|CLIP_Y_FLAG),
   CLIP_YZ_FLAG = (CLIP_Y_FLAG|CLIP_Z_FLAG),
   CLIP_ZX_FLAG = (CLIP_Z_FLAG|CLIP_X_FLAG),
   CLIP_FLAG = (CLIP_X_FLAG|CLIP_Y_FLAG|CLIP_Z_FLAG),
};

#else

/* ベクトルのコピー */
#define DG_COPY_VEC( a, b ) { asm ("lq $8,0(%1);sq $8,0(%0);"::"r"(a),"r"(b):"$8","memory"); }
/* マトリクスのコピー */
#define DG_COPY_MAT( a, b ) { asm ("lq $8,0(%1);lq $9,16(%1);lq $10,32(%1);lq $11,48(%1);\
									sq $8,0(%0);sq $9,16(%0);sq $10,32(%0);sq $11,48(%0);":\
								   :"r"(a),"r"(b):"$8","$9","$10","$11","memory"); }

/* 汎用128ビットコピー ( Ptr ) */
#define DG_COPY128( a, b ) { *( u_long128 * )(a) = *( u_long128 * )(b); }

/* 汎用 64ビットコピー ( Ptr ) */
#define DG_COPY64( a, b ) { *( u_long64 * )(a) = *( u_long64 * )(b); }

/* スタックをスクラッチパッドに設定（扱い注意） */
#define DG_SetScratchStack( addr ) {\
	__asm__ volatile ( "move $8,%0"		:: "r"(addr) : "$8", "memory" );\
	__asm__ volatile ( "sq $29,0($8)"	::: "$8", "memory" );\
	__asm__ volatile ( "addiu $8,$8,-16" ::: "$8", "memory" );\
	__asm__ volatile ( "move $29,$8"	::: "$8", "memory" );\
}
/* スクラッチパッドスタックを元に戻す */
#define DG_ResetScratchStack(){\
	__asm__ volatile ( "addiu $29,$29,16"	::: "$29", "memory" );\
	__asm__ volatile ( "lq $29,0($29)"		::: "$29", "memory" );\
}

/* 内部関数が呼ばれないように直接整数に変換する */
#define DG_FTOI( _f ) ({\
						  int _ti;\
						  float _tf ;\
						  asm("cvt.w.s %1,%2;mfc1 %0,%1":"=r"(_ti),"=f"(_tf):"f"(_f));\
						  (_ti) ;})

/* ＦＰＵ直たたきＭＡＸ */
#define DG_MAX( _a, _b )({ float _t ;asm( "max.s %0,%1,%2":"=f"(_t):"f"(_a),"f"(_b));_t ;})

/* ＦＰＵ直たたきＭＩＮＩ */
#define DG_MIN( _a, _b )({float _t ;asm( "min.s %0,%1,%2":"=f"(_t):"f"(_a),"f"(_b));_t ;})

/* ＦＰＵ直たたきＳＱＲＴ */
#define DG_SQRT( _m ) ({float _f=(_m);asm("sqrt.s %0,%0":"+f"(_f));_f;})

/* ＦＰＵ直たたきＳＱＲＴ逆数 */
#define DG_RSQRT( _m ) ({float _f=(_m);asm("rsqrt.s %0,%1,%0":"+f"(_f):"f"(1.0f));_f;})

/* ＦＰＵ直たたきＦＡＢＳ */
#define DG_FABS(_x) ({float _a = (_x);asm("abs.s %0,%1":"=f"(_a):"f"(_a));(_a);})

/* ＦＰＵアキュムレータ演算 */
#define	DG_ADDA(_x,_y)	{float _a=(_x),_b=(_y) ;asm volatile("adda.s  %0,%1"::"f"(_a),"f"(_b));}/* ACC = x + y */
#define	DG_SUBA(_x,_y)	{float _a=(_x),_b=(_y) ;asm volatile("suba.s  %0,%1"::"f"(_a),"f"(_b));}/* ACC = x - y */
#define	DG_MULA(_x,_y)	{float _a=(_x),_b=(_y) ;asm volatile("mula.s  %0,%1"::"f"(_a),"f"(_b));}/* ACC = x * y */
#define	DG_MADDA(_x,_y)	{float _a=(_x),_b=(_y) ;asm volatile("madda.s %0,%1"::"f"(_a),"f"(_b));}/* ACC += x * y */
#define	DG_MSUBA(_x,_y)	{float _a=(_x),_b=(_y) ;asm volatile("msuba.s %0,%1"::"f"(_a),"f"(_b));}/* ACC -= x * y */
#define	DG_MADD(_x,_y)	({float _a=(_x),_b=(_y) ;asm volatile("madd.s %0,%1,%2":"=f"(_a):"f"(_a),"f"(_b));(_a);})/* ACC+x*y */
#define	DG_MSUB(_x,_y)	({float _a=(_x),_b=(_y) ;asm volatile("msub.s %0,%1,%2":"=f"(_a):"f"(_a),"f"(_b));(_a);})/* ACC+x*y */

/* ＥＥ固有命令マクロ */
#define DG_MOVZ( _a, _b, _c ) asm ("movz %0,%1,%2":"+r"(_a):"r"(_b),"r"(_c))
#define DG_MOVN( _a, _b, _c ) asm ("movn %0,%1,%2":"+r"(_a):"r"(_b),"r"(_c))
//#define DG_MOVZ( _a, _b, _c ) if ( (_c) == 0 ) (_a) = (_b) ;
//#define DG_MOVN( _a, _b, _c ) if ( (_c) != 0 ) (_a) = (_b) ;

#endif //BP_PS2

/*----------------------------------------------------------------*/

/* GIF TAG を設定 */
//#define DG_SET_GIFTAG( a, b... ) ( DG_COPY128( a, ( ( sceGifTag ){ ## b } ) ) )

#if 0 //BP_GCC
#define DG_SET_GIFTAG( a, b... ) \
{ \
	static const sceGifTag __tmp = { b }; \
	*a = __tmp;\
	DG_COPY128( a, &__tmp );\
}

/* GS レジスタを設定 */
//#define DG_SET_GS_REG( a, b... ) ( DG_COPY64( a, ( ( typeof( a ) ) { ## b } ) ) )

#define DG_SET_GS_REG( a, b... ) \
{ \
	static const typeof( *a ) __tmp = { b }; \
	*a = __tmp;\
}
#endif

/*----------------------------------------------------------------*/

	/*
		チャンネル処理ユニットタイプ(1997/10/17 By Takabe)
	*/
enum {
	DG_SCREEN_CHANL,
	DG_BOUND_CHANL,
	DG_PRIM_CHANL,
	DG_CHAIN_CHANL,
	DG_CHANL_UNIT_MAX
};



/*----------------------------------------------------------------*/

	/*
		各種プリミティブ構造体定義
	*/
#include "prims.h"

#include "plugin.h"
#include "m_prim2.h" /* 2000/05/02 M.Sonoyama */

/*----------------------------------------------------------------*/
typedef struct DG_DEFMDLPAIR_s
{
   DG_DEF *def;
   DG_MDL *mdl;
} DG_DEFMDLPAIR;

#include	"libdg.x"

/*----------------------------------------------------------------*/

#define DG_UNDRAW_MAX	0x7fff0000


// BP_PS2 #define	SCRPAD_ADDR	((void*)0x70000000)


/*----------------------------------------------------------------*/
	/*
		機能追加型 DG_MakePrim()
	*/
static	inline	DG_PRIM	*GM_MakePrimChanl( int type, int n_packet, int n_prims, FVECTOR *pos, FVECTOR *uvs, int chanl )
{
#if 0
	DG_PRIM		*prim ;
	extern int GM_CurrentMap ;
	static inline void GM_GroupPrim( DG_PRIM *, int ) ;

	prim = DG_MakePrim( type, n_packet, n_prims, chanl, pos, uvs ) ;
	if ( prim != NULL ) {
		DG_QueuePrim( prim ) ;
		if ( chanl != DG_CHANL_MENU ) GM_GroupPrim( prim, GM_CurrentMap ) ;
	}
	return prim ;
#endif
	return ( NULL );
}

	/*
		機能追加型 DG_MakePrim()
		（３Ｄチャンネル固定）
	*/
static	inline	DG_PRIM	*GM_MakePrim( int type, int n_packet, int n_prims, FVECTOR *pos, FVECTOR *uvs )
{
	return GM_MakePrimChanl( type, n_packet, n_prims, pos, uvs, 0 ) ;
}

	/*
		機能追加型 DG_FreePrim()
	*/
static	inline	void	GM_FreePrim( DG_PRIM *prim )
{
#if 0
	if ( prim != NULL ) {
		DG_DequeuePrim( prim ) ;
		DG_FreePrim( prim ) ;
	}
#endif
}
	/*
		機能追加型 DG_MakePrim2()
	*/
static inline void GM_GroupPrim2( DG_PRIM2 *, int ) ;
static	inline	DG_PRIM2	*GM_MakePrim2Chanl( int type, int n_prims, int n_verts, int chanl )
{
	DG_PRIM2		*prim ;
	extern int GM_CurrentMap ;

	prim = DG_MakePrim2( type, n_prims, n_verts, chanl ) ;
	if ( prim != NULL ) {
		if ( DG_QueuePrim2( prim ) == -1 ){
			//DG_FreePrim2( prim );
			//return ( NULL );
		}
		if ( chanl != DG_CHANL_MENU ) GM_GroupPrim2( prim, GM_CurrentMap ) ;
	}
	return prim ;
}

	/*
		機能追加型 DG_MakePrim2()
		（３Ｄチャンネル固定）
	*/
static	inline	DG_PRIM2	*GM_MakePrim2( int type, int n_prims, int n_verts )
{
	return GM_MakePrim2Chanl( type, n_prims, n_verts, 0 ) ;
}

	/*
		機能追加型 DG_FreePrim2()
	*/
static	inline	void	GM_FreePrim2( DG_PRIM2 *prim )
{
	if ( prim != NULL ) {
		DG_DequeuePrim2( prim ) ;
		DG_FreePrim2( prim ) ;
	}
}

	/*
		プリミティブが付随する物体を指定
	*/
//static	inline	void	GM_ConfigPrimRoot( DG_PRIM *prim, OBJECT *object, int unit )
//{
//	prim->root = &( object->objs->objs[ unit ].world ) ;
//}

/*----------------------------------------------------------------*/
/*
	デバッグ関連
*/
#ifdef BP_MAKEMODEL_DEBUG_INFO

extern DG_OBJS		*DG_MakeObjsD( DG_DEF *def, int flag, int chanl, char *fname );
extern DG_OBJS		*DG_MakeObjs2D( DG_DEF *def, int flag, int chanl, DG_DEFMDLPAIR *mdl_list, int n_list, char *fname );
#define DG_MakeObjs(_d,_f,_c) DG_MakeObjsD(_d,_f,_c, __FILE__ )
#define DG_MakeObjs2(_d,_f,_c,_m,_n) DG_MakeObjs2D(_d,_f,_c,_m,_n, __FILE__ )

#endif

#ifdef BP_MAKEPRIM_DEBUG_INFO

extern DG_PRIM2	*GM_MakePrim2D( int type, int n_prims, int n_verts, char *fname );
//#define DG_MakePrim2(_f,_np,_nv,_c) DG_MakePrim2D(_f,_np,_nv,_c, __FILE__ )
#define	GM_MakePrim2(_t,_np,_nv) GM_MakePrim2D(_t,_np,_nv, __FILE__ )

#endif

extern DG_TEX_LIN *DG_MakeLinerTexture( int width, int height, int format );
extern void DG_MakeLinerTexture2( DG_TEX_LIN *tex, int width, int height, int format, void *addr );
extern void DG_FreeLinerTexture( DG_TEX_LIN *tex );
static inline void DG_LinerTextureSetImageDirty(DG_TEX_LIN *tex){ tex->image_dirty = 1; }

extern void DG_AS_EndframeThread_WaitDone();

enum EDGHackArea
{
   kDGHA_Unknown,
   kDGHA_w32a
};

extern enum EDGHackArea gAS_DG_HackArea;

/*----------------------------------------------------------------*/
/*
	マクロ
*/

/*
	プリミティブ座標設定用ユーティリティ
	x, y は画面座標
	u, v はテクスチャ座標
*/

#define DG_POS_X( _x ) ( ( (_x) + 2048 - DRAW_WIDTH/2 ) << 4 )
#define DG_POS_Y( _y ) ( ( (_y) + 2048 - DRAW_HEIGHT/2 ) << 4 )
#define DG_POS_U( _u ) ( (_u) << 4 )
#define DG_POS_V( _v ) ( (_v) << 4 )

extern void		DG_RotVectorW( FVECTOR *from, FVECTOR *to, int n );
extern void		DG_PutVectorW( FVECTOR *from, FVECTOR *to, int n );

// WARNING - ONLY TO BE USED IN CALLBACKS IN VERY VERY VERY LIMITED SITUATIONS
void *DG_AS_SceneBufferAlloc(int size );
void *DG_AS_SceneBufferAllocCopy(void const *src, int size );
extern DG_CHANL *DG_Chanls_Buf;

extern void AS_BufferScene(int which);
extern void AS_BufferSceneNOP(int which);
extern int gAS_UsedBufferSceneNOP;

struct _SBP_OBJ_Render;

extern struct _SBP_OBJ_Render *DG_AS_PrecomputeChainBuffers( DG_OBJ *obj, struct _SBP_OBJ_Render *pListHead );
extern struct _SBP_OBJ_Render *DG_AS_PrecomputeChainBuffersObjs( DG_OBJS *objs, struct _SBP_OBJ_Render *pListHead );
extern void DG_AS_FreePrecomputedChainBuffers( DG_OBJ *obj );
extern void DG_AS_FreePrecomputedChainBuffersObjs( DG_OBJS *objs );
extern void DG_AS_DumpPrecomputedChainBufferObjInfo( struct _SBP_OBJ_Render *pHead );

#ifdef __cplusplus
};
#endif

#endif	// KP_XBOX
#endif	// KP_WINDOWS

#endif

