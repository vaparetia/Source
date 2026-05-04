/*
	fmt_kmd.h
	モデルデータ .kmd フォーマット

	1999/07/07 K.Takabe
	$Id: fmt_kmdw.h,v 1.4 2003/01/05 04:46:34 takaki Exp $

*/

#ifndef	__FMT_KMD__H__
#define	__FMT_KMD__H__

#ifdef WIN32
// サイズ0配列警告の抑制
#pragma warning( disable : 4200 )
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif

#error THIS FILE SHOULD NOT BE INCLUDED

/*----------------------------------------------------------------*/

	/*
		パーツ単位での属性フラグ
	*/
enum {
	DG_TYPE_FT4			= 0x0000,	/* 未使用								*/
	DG_TYPE_GT4			= 0x0001,	/* 未使用								*/
	DG_TYPE_TRANS		= 0x0002,	/* 半透明化								*/
	DG_TYPE_NOSHADE		= 0x0004,	/* 光源なし（プリシェード時のみ有効）	*/

	DG_TYPE_REVWT		= 0x0010,
	DG_TYPE_REVWT_WT0	= 0x0020,
	DG_TYPE_REVWT_WT1	= 0x0040,

	DG_TYPE_AUTOSIDE	= 0x0200,	/* 自動片面（プリシェード時のみ有効）	*/
	DG_TYPE_SINGLESIDE	= 0x0400,	/* 片面（プリシェード時のみ有効）		*/
	DG_TYPE_OVERLAY0	= 0x0800,	/* テクスチャの重ねぬり					*/
	DG_TYPE_OVERLAY1	= 0x1000,	/* テクスチャの重ねぬり					*/
	DG_TYPE_OVERLAY2	= 0x2000,	/* テクスチャの重ねぬり					*/
	DG_TYPE_EXTEND		= 0x4000,	/* 拡張モデル（parentに付随する）		*/
} ;

	/*
		オブジェクトパケットフラグ
	*/
enum {
	DG_PACKFLAG_NORMAL = 0x0000,		/* 	*/
	DG_PACKET_CULLRIGHT	= 0x00010000,	/* ストリップの右回りスタート */
	DG_PACKET_CULLLEFT	= 0x00020000,	/* ストリップの左回りスタート */
} ;


	/*
		データフォーマットＩＤ
	*/
enum {
	MGS_MODEL_NORM			= 13112177,	/* 通常モデル */
	MGS_MODEL_MULTITEX		= 11686819,	/* 通常マルチテクスチャモデル */
	MGS_MODEL_MULTIWEIGHT	= 13335939,	/* マルチウェイトエンベロープモデル */
	MGS_MODEL_NORM_A		= 5100225,	/* 通常モデルのパック化対応データ("MGS_MODEL_NORM_A") */
	MGS_MODEL_MULTITEX_A	= 5151458,	/* 通常マルチテクスチャモデルのパック化対応データ("MGS_MODEL_MULTITEX_A") */

	MGS_MODEL_FORMAT_MASK	= 0x00ffffff,	// フォーマットマスク
	MGS_MODEL_FLAG_INDEX	= 0x01000000,	// インデックス付
} ;

	/*
		新kms及びevmファイルフォーマット用パケットフラグ
	*/
enum {
	DG_PACKFLAG_ENVELOPE	= 0x0001,	/* エンベロープ処理付き	*/
	DG_PACKFLAG_CULLAUTO	= 0x0002,	/* プリシェード時に距離に応じてカリング処理	*/
	DG_PACKFLAG_CULLON		= 0x0004,	/* プリシェード時にカリング処理ＯＮ	*/
	/* 描画属性（マルチテクスチャ時のみ有効） */
	DG_PACKFLAG_TEX0	= 0x0008,	/* テクスチャ０使用 */
	DG_PACKFLAG_TEX1	= 0x0010,	/* テクスチャ１使用 */
	DG_PACKFLAG_TEX2	= 0x0020,	/* テクスチャ２使用 */
	DG_PACKFLAG_UV0		= 0x0040,	/* 通常ＵＶ０使用 */
	DG_PACKFLAG_UV1		= 0x0080,	/* 通常ＵＶ１使用 */
	/* 拡張計算フラグ（下のフラグは共に排他でなければならない）*/
	DG_PACKFLAG_UV2		= 0x0100,	/* 通常ＵＶ２使用 */
	DG_PACKFLAG_EMAP	= 0x0200,	/* 環境マッピング（ＵＶ２へ格納） */
	DG_PACKFLAG_SMAP	= 0x0400,	/* 環境マッピング影（ＵＶ２へ格納） */
	DG_PACKFLAG_BMAP	= 0x0800,	/* バンプマッピング（ＵＶ１をずらしたものをＵＶ２へ格納） */
} ;

#define DG_TYPE_PRIO_VALUE_SHIFT	( 12 )		/* 優先の上げ下げ４段階 */
		/* PRIO 0 .. +-1000 */
		/* PRIO 1 .. +-750 */
		/* PRIO 2 .. +-500 */
		/* PRIO 3 .. +-250 */

/* ＥＶＭモデル属性 */
enum {
	DG_EVMTYPE_LARGE			= 0x00000001,	/* 大型モデル */
	DG_EVMTYPE_FORMAT_V2		= 0x01000000,	/* フォーマットVer.2.00 */
	DG_EVMTYPE_INDEX			= 0x00000002,	/* インデックスデータ付 */

	DG_EVMTYPE_DRAW_BLACKBUMP	= 0x00000004,	/* バンプマップを強制描画(EDドッグタグ用) */
};


/*----------------------------------------------------------------*/
	/*
		テクスチャ座標ベクトル
	*/
typedef	struct	{
	unsigned char		u, v ;
} TVECTOR ;

typedef struct _ftvector{
	float	u, v ;
} FTVECTOR ;

typedef struct _tvector_s{
	short u, v;
} TVECTOR_S ;

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
#define DG_OBJPACK	DG_MDLPACK

/*

＜ＸＢＯＸ用ＫＭＳ（ＫＭＸ）ファイルフォーマットの変更について＞

■ 従来のフォーマットからの変更

・DG_MDLのメンバにn_vertsを追加。
・DG_MDL->vbuffの先にパックされた頂点配列を付加
・DG_MDL->strideにはパック化された頂点データサイズを指定
・DG_MDLPACK->n_indicesを追加（これは以前から追加済み）
　描画するトライアングルストリップを構成するためのインデックスで
　ＰＳ２固有のキックフラグは頂点をダブらせることで実現するため
  従来からあるDG_MDLPACK->n_vertsとは一致しないので注意。
・DG_MDLPACKあたりの最大頂点数制限を廃止（これは以前から追加済み）
・従来からあるDG_MDLPACKのn_verts, verts, norms, uvs, rgbsは未使用に



■ＣＶ２（ＣＶＸ）との関係

・DG_MDLの頂点データ配列とCV2の頂点並びは完全に同一とする
・CV2_MDL->n_verts == CV2_MDL->n_norms == CV2_MDL->n_uvs == DG_MDL->n_verts
・CV2_MDL->n_verts_index == CV2_MDL->n_verts であり、インデックスデータは
　0からn_verts-1まで連続（頂点分配の必要がないので）
・CV2_MDL->verts_indexとDG_MDLPACK->indexは別データなので注意

*/


#if 0 /* libdgx.hにて定義済み */
/* パック化された各頂点データ */
/*
	但し法線及びＵＶ値は(1.3.12)ではなく(1.0.15)形式
*/
/* シングルテクスチャ */
typedef struct _dg_vertex_kmss {
	short		vx, vy, vz ;	/* 頂点データ */
	short		wt ;			/* ウェイトデータ */
	short		nx, ny, nz ;	/* 法線 */
	short		f ;				/* フラグ（未使用） */
	short		u0, v0 ;		/* テクスチャ座標 */
} DG_VERTEX_KMSS ;
/* マルチテクスチャ */
typedef struct _dg_vertex_kmsm {
	short		vx, vy, vz ;	/* 頂点データ */
	short		wt ;			/* ウェイトデータ */
	short		nx, ny, nz ;	/* 法線 */
	short		f ;				/* フラグ（未使用） */
	short		u0, v0 ;		/* テクスチャ座標 */
	short		u1, v1 ;		/* テクスチャ座標 */
	short		u2, v2 ;		/* テクスチャ座標 */
} DG_VERTEX_KMSM ;
#endif

#define NEW_KMX_FORMAT

#ifdef NEW_KMX_FORMAT

typedef	struct _DG_MDLPACK	{
	unsigned int	flag ;			/* 属性フラグ						*/
	unsigned short	n_verts ;		/* 頂点数 *//* = 0 */
	unsigned short	n_indices ;		/* インデックス数 */
	unsigned int	tex_id[3] ;		/* テクスチャＩＤ（３セット）		*/
	short			*verts ;		/* 頂点データ						*//* = NULL */
	short			*norms ;		/* 法線データ						*//* = NULL */
	short			*uvs[3] ;		/* ＵＶデータ（３セット）			*//* = NULL */
	CVECTOR			*rgbs ;			/* プリシェードデータへ（未使用）	*//* = NULL */
	void 			*index;			/* インデックスデータ */
} DG_MDLPACK ;

typedef	struct _DG_MDL	{
	unsigned int	type ;			/* 属性フラグ				*/
	unsigned int	n_packs ;		/* パケット数				*/
	float			lx, ly, lz ;	/* バウンディングボックス	*/
	float			ux, uy, uz ;	/* バウンディングボックス	*/
	float			tx, ty, tz ;	/* ユニットの相対座標		*/
	int				parent ;		/* 親ユニット番号			*/
	DG_MDLPACK		*packs ;		/* オブジェクトパケット		*/
#ifdef PSX2
	void			*pre_packet ;	/* プリパケット（ロード時に生成）	*/
#else
	void			*vbuff ;		/* 頂点バッファアドレス *//* XBOXでは頂点バッファメモリのアドレス */
									/* DG_VERTEX_KMSSまたはDG_VERTEX_KMSM形式 */
#endif
	int				stride ;		/* 頂点バッファストライド	*/
									/* sizeof(DG_VERTEX_KMSS)またはsizeof(DG_VERTEX_KMSM) */
	int				n_verts ;		/* 共有化された頂点数の最大数 */
} DG_MDL ;

typedef	struct _DG_MDL_DEF	{
	unsigned int	type ;			/* 属性フラグ				*/
	unsigned int	n_packs ;		/* パケット数				*/
	float			lx, ly, lz ;	/* バウンディングボックス	*/
	float			ux, uy, uz ;	/* バウンディングボックス	*/
	float			tx, ty, tz ;	/* ユニットの相対座標		*/
	int				parent ;		/* 親ユニット番号			*/
	DG_MDLPACK		*packs ;		/* オブジェクトパケット		*/
#ifdef PSX2
	void			*pre_packet ;	/* プリパケット（ロード時に生成）	*/
#else
	void			*vbuff ;		/* 頂点バッファアドレス *//* XBOXでは頂点バッファメモリのアドレス */
									/* DG_VERTEX_KMSSまたはDG_VERTEX_KMSM形式 */
#endif
	int				stride ;		/* 頂点バッファストライド	*/
									/* sizeof(DG_VERTEX_KMSS)またはsizeof(DG_VERTEX_KMSM) */
	int				n_verts ;		/* 共有化された頂点数の最大数 */
} DG_MDL_DEF ;

typedef	struct	_DG_DEF{
	unsigned int	data_format ;	/* データフォーマットＩＤ		*/
	unsigned int	n_models ;		/* ユニット数					*/
	unsigned int	n_x_models ;	/* ユニット数（拡張ユニット込）	*/
	unsigned int	texture ;		/* テクスチャファイルＩＤ		*/
	int				pad[3] ;
	float			lx, ly, lz ;	/* 全体バウンディングボックス	*/
	float			ux, uy, uz ;	/*								*/
	float			tx, ty, tz ;	/* モデルの基準表示位置			*/
	DG_MDL_DEF		models[ NULL_ARRAY ] ;	/* ユニット配列				*/
} DG_DEF ;

#else
/* 時代遅れkmx用 */
/* ＰＳ２メタルギア用マルチテクスチャ対応モデルフォーマット */
typedef	struct _DG_MDLPACK	{
	unsigned int	flag ;			/* 属性フラグ						*/
	unsigned short	n_verts ;		/* 頂点数 */
	unsigned short	n_indices ;		/* インデックス数 */
	unsigned int	tex_id[3] ;		/* テクスチャＩＤ（３セット）		*/
	short			*verts ;		/* 頂点データ						*/
	short			*norms ;		/* 法線データ						*/
	short			*uvs[3] ;		/* ＵＶデータ（３セット）			*/
	CVECTOR			*rgbs ;			/* プリシェードデータへ（未使用）	*/
	void 			*index;			/* KMS/EVM用頂点バッファ情報へのポインタ */	
} DG_MDLPACK ;

typedef	struct _DG_MDL	{
	unsigned int	type ;			/* 属性フラグ				*/
	unsigned int	n_packs ;		/* ポリゴン数				*/
	float			lx, ly, lz ;	/* バウンディングボックス	*/
	float			ux, uy, uz ;	/* バウンディングボックス	*/
	float			tx, ty, tz ;	/* ユニットの相対座標		*/
	int				parent ;		/* 親ユニット番号			*/
	DG_MDLPACK		*packs ;		/* オブジェクトパケット		*/
#ifdef PSX2
	void			*pre_packet ;	/* プリパケット（ロード時に生成）	*//* XBOXでは頂点バッファメモリのアドレス */
#else
	void			*vbuff ;		/* 頂点バッファアドレス */
#endif
	int				stride ;		/* 頂点バッファストライド	*/
	int				pad1 ;			/* リザーブ		*/
} DG_MDL ;

typedef	struct	_DG_DEF{
	unsigned int	data_format ;	/* データフォーマットＩＤ		*/
	unsigned int	n_models ;		/* ユニット数					*/
	unsigned int	n_x_models ;	/* ユニット数（拡張ユニット込）	*/
	unsigned int	texture ;		/* テクスチャファイルＩＤ		*/
	void            *vbuf_info;     /* 頂点バッファ情報へのポインタ */
	//int				pad[3] ;
	int				pad[2] ;
	float			lx, ly, lz ;	/* 全体バウンディングボックス	*/
	float			ux, uy, uz ;	/*								*/
	float			tx, ty, tz ;	/* モデルの基準表示位置			*/
	DG_MDL		models[ NULL_ARRAY ] ;	/* ユニット配列				*/
} DG_DEF ;
#endif

/* 頂点データフォーマット */
/*
	各種頂点データの先頭アドレスは１６バイト境界に揃っていなければならない
	（つまり頂点データなどは８バイトなので頂点数が奇数の場合にはダミーを入れる必要がある）

	頂点データ
	short	x ;	Ｘ座標（固定小数点(1.15.0)）
	short	y ;	Ｙ座標（固定小数点(1.15.0)）
	short	z ;	Ｚ座標（固定小数点(1.15.0)）
	short	w ; ウェイト値（固定小数点(1.3.12)）
				ウェイト値は1.0でその関節に100%、0.0で親オブジェクトに100%になる

	法線データ
	short	x ;	法線ベクトルＸ（固定小数点(1.3.12)）
	short	y ;	法線ベクトルＹ（固定小数点(1.3.12)）
	short	z ;	法線ベクトルＺ（固定小数点(1.3.12)）
	short	f ;	キックフラグ（0x0fff:描画キック、0x8fff:頂点キックのみ）
				0x0fff ... 必須（ＶＵプログラムでのマスクに使用するため）
				0x8000 ... 描画キックを行なわない
				0x4000 ... ＶＵ内部で使用するため使用してはならない
				0x2000 ... 右回りをカリング（将来の拡張用）
				0x1000 ... 左回りをカリング（将来の拡張用）

	ＵＶデータ
	short	u ;	Ｕ座標（固定小数点(1.3.12)）
	short	v ;	Ｖ座標（固定小数点(1.3.12)）


	＜カリングフラグについての補足＞（実際にはまだ未実装）
	キックフラグのカリング用フラグはカメラから見て右回りの時にクリップする
	ポリゴン（つまり左回りが表の場合）には0x2000を立て、逆に左回りの時に
	クリップするポリゴン（右回りが表の場合）には0x1000を立てる。どちらの
	フラグも立てないと両面ポリゴン扱いになる。
	また、ＭＧＳ２では右手座標系が採用されているので注意すること。

	＜右手座標系＞
	  |+Y
	  |
	  |_____+X
	  /
	 /
	/+Z

*/


/*----------------------------------------------------------------*/
	/*
		マルチウェイト対応ＤＭＡ転送モデルフォーマット
		
	*/

#if 0
typedef struct _evm_pack {
	unsigned int	flag ;			/* 属性フラグ				*/
	unsigned int	tex_id[3] ;		/* テクスチャＩＤ			*/
	unsigned short	n_verts ;		/* 頂点数（最大３２頂点）	*/
	unsigned short	n_indices ;		/* インデックス数	*/
	unsigned int	n_mats ;		/* 使用マトリクス数			*/
	unsigned char	mat_id[8] ;		/* 使用マトリクス番号		*/
	void			*verts ;		/* 頂点データ				*/
	void			*norms ;		/* 法線データ				*/
	void			*uvs[3] ;		/* ＵＶデータ				*/
	void			*weight ;		/* ウェイトデータ			*/
	void			*rgbs ;			/* 頂点カラーデータ（現在未使用）*/
	//int				pad1 ;			/* リザーブ領域				*/
	//void 			*vbuf_info;		/* KMS/EVM用頂点バッファ情報へのポインタ */
	void	 		*index;			/* インデックスバッファ情報 */
} EVM_PACK ;

/* モデルスケルトン情報 */
typedef struct _evm_skel {
	int				flag ;
	int				parent ;		/* 親スケルトン番号				*/
	float			tx,ty,tz ;		/* 親からのオフセット位置		*/
	float			rt_tx, rt_ty, rt_tz ;	/* ルートからのオフセット */
	float			lx, ly, lz, pad0 ;	/*  */
	float			ux, uy, uz, pad1 ;	/*  */
} EVM_SKEL ;

#if 0
/* モデルスケルトン情報 */
typedef struct _evm_skel {
	int				flag ;
	int				parent ;		/* 親スケルトン番号				*/
	float			tx,ty,tz ;		/* 親からのオフセット位置		*/
	float			rt_tx, rt_ty, rt_tz ;	/* ルートからのオフセット */
} EVM_SKEL ;
#endif

/* モデルフォーマットヘッダ */
typedef struct _dg_mwdef {
	unsigned int	n_models ;		/* スケルトン数						*/
	unsigned int	n_x_models ;	/* スケルトン数（拡張ユニット込）	*/
	float			lx, ly, lz ;	/* 全体バウンディングボックス		*/
	float			ux, uy, uz ;	/*									*/
	int				type ;			/*									*/
	int				texture ;		/* 使用テクスチャグループＩＤ		*/
	int				n_packs ;		/* ポリゴンパケット数				*/
	EVM_PACK		*packet ;		/* ポリゴンパケット					*/
	EVM_SKEL		skeleton[ NULL_ARRAY ] ;
} EVM_DEF ;

#else

#if 0	/* libdgx.hにて宣言済み */
typedef struct _dg_vertex_evm {
	short		vx, vy, vz, vw ;		/* 頂点データ */
	short		nx, ny, nz ;		/* 法線 */
	short		u0, v0, q0 ;	/* テクスチャ座標 */
	short		u1, v1, q1 ;	/* テクスチャ座標 */
	short		u2, v2, q2 ;	/* テクスチャ座標 */
	short		weight[4] ;		/* 各マトリクスに対するウェイト */
	short		index[4] ;		/* ウェイトに対応したマトリクス番号（×４） */
} DG_VERTEX_EVM ;
#endif

#define NEW_EVX_FORMAT
typedef struct _evm_pack {
	unsigned int	flag ;			/* 属性フラグ				*/
	unsigned int	tex_id[3] ;		/* テクスチャＩＤ			*/
	unsigned short	n_verts ;		/* 頂点数（最大３２頂点）	*/
	unsigned short	n_indices ;		/* インデックス数	*/
	unsigned int	n_mats ;		/* 使用マトリクス数			*/
	unsigned char	mat_id[8] ;		/* 使用マトリクス番号		*/
	void			*verts ;		/* 頂点データ				*/
	void			*norms ;		/* 法線データ				*/
	void			*uvs[3] ;		/* ＵＶデータ				*/
	void			*weight ;		/* ウェイトデータ			*/
	void			*rgbs ;			/* 頂点カラーデータ（現在未使用）*/
	void	 		*index;			/* インデックスバッファ情報 */
} EVM_PACK ;

/* モデルスケルトン情報 */
typedef struct _evm_skel {
	int				flag ;
	int				parent ;		/* 親スケルトン番号				*/
	float			tx,ty,tz ;		/* 親からのオフセット位置		*/
	float			rt_tx, rt_ty, rt_tz ;	/* ルートからのオフセット */
	float			lx, ly, lz, pad0 ;	/*  */
	float			ux, uy, uz, pad1 ;	/*  */
} EVM_SKEL ;

/* モデルフォーマットヘッダ */
typedef struct _dg_mwdef {
	int				data_format ;	/*  */
	unsigned int	n_models ;		/* スケルトン数						*/
	unsigned int	n_x_models ;	/* スケルトン数（拡張ユニット込）	*/
	float			lx, ly, lz ;	/* 全体バウンディングボックス		*/
	float			ux, uy, uz ;	/*									*/
	int				type ;			/*									*/
	int				texture ;		/* 使用テクスチャグループＩＤ		*/
	int				n_packs ;		/* ポリゴンパケット数				*/
	EVM_PACK		*packet ;		/* ポリゴンパケット					*/
	void			*vbuff ;		/* 共有頂点データ */
	int				stride ;		/* 頂点サイズ */
	int				n_verts ;		/* 共有頂点数 */
	EVM_SKEL		skeleton[ NULL_ARRAY ] ;
} EVM_DEF ;



#endif


/* データの並び順について */
/*

	<ヘッダー>
	EVM_DEF * 1

	<スケルトン情報>
	EVM_SKEL * n_models

	<パケット情報>
	EVM_PACK * n_packs

	<頂点情報>
	[verts data 0]
	[verts data 1]
	[verts data 2]
	...
	[verts data n_packs-1]

	<法線情報>
	[norms data 0]
	......
	[norms data n_packs-1]

	<ＵＶ０情報>
	[uvs[0] data 0]
	......
	[uvs[0] data n_packs-1]

	<ＵＶ１情報>（場合によっては省略可）
	[uvs[1] data 0]
	......
	[uvs[1] data n_packs-1]

	<ＵＶ２情報>（場合によっては省略可）
	[uvs[2] data 0]
	......
	[uvs[2] data n_packs-1]

	<ウェイト値情報>
	[weight data 0]
	......
	[weight data n_packs-1]

	<EOF>
*/


/* 頂点データフォーマット */
/*
	各種頂点データの先頭アドレスは１６バイト境界に揃っていなければならない
	（つまり頂点データなどは８バイトなので頂点数が奇数の場合にはダミーを入れる必要がある）

	頂点データ
	short	x ;	Ｘ座標（固定小数点(1.15.0)）
	short	y ;	Ｙ座標（固定小数点(1.15.0)）
	short	z ;	Ｚ座標（固定小数点(1.15.0)）
	short	f ;	キックフラグ（0x0fff:描画キック、0x8fff:頂点キックのみ）
	又は
	short	x ;	Ｘ座標（固定小数点(1.11.4)）
	short	y ;	Ｙ座標（固定小数点(1.11.4)）
	short	z ;	Ｚ座標（固定小数点(1.11.4)）
	short	f ;	キックフラグ（0x0fff:描画キック、0x8fff:頂点キックのみ）

	法線データ
	short	x ;	法線ベクトルＸ（固定小数点(1.3.12)）
	short	y ;	法線ベクトルＹ（固定小数点(1.3.12)）
	short	z ;	法線ベクトルＺ（固定小数点(1.3.12)）
	short	pad ;	

	ＵＶデータ
	short	u ;	Ｕ座標（固定小数点(1.3.12)）
	short	v ;	Ｖ座標（固定小数点(1.3.12)）
	short	w ;	Ｗ座標（固定小数点(1.3.12)）但し精度が低いので1.0固定とする
	short	pad ;

	ウェイトデータ
	//u_char	w0 ;	マトリクス０に対するウェイト値（固定小数点(0.1.7)）
	//u_char	w1 ;	マトリクス１に対するウェイト値（固定小数点(0.1.7)）
	//u_char	w2 ;	マトリクス２に対するウェイト値（固定小数点(0.1.7)）
	//u_char	w3 ;	マトリクス３に対するウェイト値（固定小数点(0.1.7)）
	//u_char	w4 ;	マトリクス４に対するウェイト値（固定小数点(0.1.7)）
	//u_char	w5 ;	マトリクス５に対するウェイト値（固定小数点(0.1.7)）
	//u_char	w6 ;	マトリクス６に対するウェイト値（固定小数点(0.1.7)）
	//u_char	w7 ;	マトリクス７に対するウェイト値（固定小数点(0.1.7)）
	u_char	w0 ;	ウェイト値０（固定小数点(0.1.7)）
	u_char	w1 ;	ウェイト値１（固定小数点(0.1.7)）
	u_char	w2 ;	ウェイト値２（固定小数点(0.1.7)）
	u_char	w3 ;	ウェイト値３（固定小数点(0.1.7)）
	u_char	index0 ;	マトリクス０に対するマトリクスインデックス番号(0~7)x4）
	u_char	index1 ;	マトリクス１に対するマトリクスインデックス番号(0~7)x4）
	u_char	index2 ;	マトリクス２に対するマトリクスインデックス番号(0~7)x4）
	u_char	index3 ;	マトリクス３に対するマトリクスインデックス番号(0~7)x4）



*/

/*----------------------------------------------------------------*/

/* 共有頂点データファイルフォーマット */
typedef struct _CVD_MDL{
	int			n_verts ;			/* 共有頂点数 */
	int			n_verts_index ;		/* 展開頂点数 */
	FVECTOR		*verts ;			/* 共有頂点データアドレス */
	short		*verts_index ;		/* 頂点分配インデックステーブルアドレス */
	int			n_norms ;			/* 共有法線数 */
	int			n_norms_index ;		/* 展開法線数 */
	FVECTOR		*norms ;			/* 共有法線データアドレス */
	short		*norms_index ;		/* 法線分配インデックステーブルアドレス */
	int			n_uvs ;				/* 共有UV数 */
	int			n_uvs_index ;		/* 展開UV数 */
	FVECTOR		*uvs ;				/* 共有UVデータアドレス */
	short		*uvs_index ;		/* UV分配インデックステーブルアドレス */
} CVD_MDL ;

typedef struct _CVD_DEF{
	int			id ;				/* 6754556 = GV_StrCode("Common Vertex Data") */
	int			n_models ;
	int			flag ;
	int			pad1 ;
	CVD_MDL		models[ NULL_ARRAY ];
} CVD_DEF ;

enum {
	COMMON_VERTS		= 0x0001,
	COMMON_NORMS		= 0x0002,
	COMMON_UVS		= 0x0004,

	COMMON_VERT_USRDATA	= 0x0008,
	COMMON_NORM_USRDATA	= 0x0010,
};



/* 共有頂点データファイルフォーマット Ver 1.2 */
typedef struct {
	int nVertexSwing;	// ゆれ属性

	/* ver 1.2 */
	short dWeight[4];	// 重み
} VERT_USRDATA;

typedef struct {
	int	dummy[ NULL_ARRAY ];
} NORM_USRDATA;

typedef struct _CV2_MDL{
	int			n_verts ;			/* 共有頂点数 */
	int			n_verts_index ;		/* 展開頂点数 */
	FVECTOR		*verts ;			/* 共有頂点データアドレス */
	short		*verts_index ;		/* 頂点分配インデックステーブルアドレス */
	int			n_norms ;			/* 共有法線数 */
	int			n_norms_index ;		/* 展開法線数 */
	FVECTOR		*norms ;			/* 共有法線データアドレス */
	short		*norms_index ;		/* 法線分配インデックステーブルアドレス */
	int			n_uvs ;				/* 共有UV数 */
	int			n_uvs_index ;		/* 展開UV数 */
	FVECTOR		*uvs ;				/* 共有UVデータアドレス */
	short		*uvs_index ;		/* UV分配インデックステーブルアドレス */

	VERT_USRDATA	*vert_usrdata;		/* ユーザー定義情報 */
	NORM_USRDATA	*norm_usrdata;		/* ユーザー定義情報 */

} CV2_MDL ;

typedef struct _CV2_DEF{
	int			id ;  /* 705644 = StrCode("Common Vertex Data ver1.1") */
	int			n_models ;
	int			flag ;
	int			pad1 ;
	CV2_MDL		models[ NULL_ARRAY ];
} CV2_DEF ;



























#if 0
	/*
		.kmd ユニットヘッダ
	*/
typedef	struct _KMD_MDL	{
	unsigned int		type ;		/* 属性フラグ			*/
	unsigned int		n_prims ;	/* ポリゴン数			*/
	int		lx, ly, lz ;	/* バウンディングボックス	*/
	int		ux, uy, uz ;	/*				*/
	int		tx, ty, tz ;	/* ユニットの相対座標		*/
	int		parent ;	/* 親ユニット番号		*/
	int		extend ;	/* 拡張ユニット番号		*/
	/*
		頂点データ
	*/
	unsigned int	n_verts ;	/* 頂点数			*/
	SVECTOR			*verts ;	/* 頂点座標配列（n_verts）	*/
	unsigned char	*vid ;		/* 頂点ＩＤ配列（n_prims * 4）	*/
	/*
		法線データ
	*/
	unsigned int	n_norms ;	/* 法線数			*/
	SVECTOR			*norms ;	/* 法線方向配列（n_norms）	*/
	unsigned char	*nid ;		/* 法線ＩＤ配列（n_prims * 4）	*/
	/*
		テクスチャデータ
	*/
	TVECTOR			*uvs ;		/* ＵＶ座標配列（n_prims * 4）	*/
	unsigned short	*tid ;		/* ＩＤ配列（n_prims）		*/
	/*
		ＲＧＢデータ（通常は NULL）
	*/
	CVECTOR		*rgbs ;		/* ＲＧＢ配列（n_prims * 4）	*/
} KMD_MDL ;
typedef	struct _KM2_MDL	{
	unsigned int		type ;		/* 属性フラグ			*/
	unsigned int		n_prims ;	/* ポリゴン数			*/
	int		lx, ly, lz ;	/* バウンディングボックス	*/
	int		ux, uy, uz ;	/*				*/
	int		tx, ty, tz ;	/* ユニットの相対座標		*/
	int		parent ;	/* 親ユニット番号		*/
	int		extend ;	/* 拡張ユニット番号		*/
	/*
		頂点データ
	*/
	unsigned int	n_verts ;	/* 頂点数			*/
	SVECTOR			*verts ;	/* 頂点座標配列（n_verts）	*/
	unsigned short	*vid ;		/* 頂点ＩＤ配列（n_prims * 4）	*/
	/*
		法線データ
	*/
	unsigned int	n_norms ;	/* 法線数			*/
	SVECTOR			*norms ;	/* 法線方向配列（n_norms）	*/
	unsigned short	*nid ;		/* 法線ＩＤ配列（n_prims * 4）	*/
	/*
		テクスチャデータ
	*/
	TVECTOR			*uvs ;		/* ＵＶ座標配列（n_prims * 4）	*/
	unsigned short	*tid ;		/* ＩＤ配列（n_prims）		*/
	/*
		ＲＧＢデータ（通常は NULL）
	*/
	CVECTOR		*rgbs ;		/* ＲＧＢ配列（n_prims * 4）	*/
} KM2_MDL ;

/*----------------------------------------------------------------*/

	/*
		.kmd ファイルヘッダ
	*/
typedef	struct	_KMD_DEF{
	unsigned int		n_models ;	/* ユニット数			*/
	unsigned int		n_x_models ;	/* ユニット数（拡張ユニット込）	*/
	int		lx, ly, lz ;	/* 全体バウンディングボックス	*/
	int		ux, uy, uz ;	/*				*/
	KMD_MDL		models[ 0 ] ;	/* ユニット配列			*/
} KMD_DEF ;
typedef	struct	_KM2_DEF{
	unsigned int		n_models ;	/* ユニット数			*/
	unsigned int		n_x_models ;	/* ユニット数（拡張ユニット込）	*/
	int		lx, ly, lz ;	/* 全体バウンディングボックス	*/
	int		ux, uy, uz ;	/*				*/
	KM2_MDL		models[ 0 ] ;	/* ユニット配列			*/
} KM2_DEF2 ;

#endif


/*----------------------------------------------------------------*/
	/*
		汎用アーカイバファイルフォーマット
	*/
#define ZAR_FORMAT_KMS		(113171)	/*"kms"*/
typedef struct {
	int			data_offset ;
	int			data_id ;
	int			pad[ 2 ] ;
} ZAR_LIST ;
typedef struct {
	int				format_type ;	/*   */
	int				version ;		/* 0 */
	int				type ;			/* 0 */
	int				n_datas ;
	ZAR_LIST	list[ NULL_ARRAY ];
} ZAR_HEADER ;



#ifdef __cplusplus
};
#endif

#endif

