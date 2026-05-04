/*
	モデルデータ .kms フォーマット

	$Id: fmt_kms.h,v 1.5 2002/03/28 04:38:40 usr03700 Exp $
*/

#ifndef	__FMT_KMD__H__
#define	__FMT_KMD__H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif

/*----------------------------------------------------------------*/
typedef struct { char r, g, b, cd ; } CVECTOR ;
typedef struct { short vx, vy, vz, vw ; } SVECTOR ;
typedef struct { short vx, vy ; } STVECTOR ;
typedef struct { int vx, vy, vz, vw ; } IVECTOR ;
typedef struct { float vx, vy, vz, vw ; } FVECTOR ;
typedef struct { float m[4][4] ; } FMATRIX  ;

	/*
		パーツ単位での属性フラグ
	*/
enum {
	DG_TYPE_FT4			= 0x0000,	/* 未使用								*/
	DG_TYPE_GT4			= 0x0001,	/* 未使用								*/
	DG_TYPE_TRANS		= 0x0002,	/* 半透明化								*/
	DG_TYPE_NOSHADE		= 0x0004,	/* 光源なし（プリシェード時のみ有効）	*/
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

	MGS_MODEL_FORMAT_MASK	= 0x00ffffff,	/* 24 bit mask 上位ビットは拡張 */

	MGS_MODEL_FLAG_INDEX	= 0x01000000,	/* インデックスつきモデル（Xbox用） */
} ;

	/*
		新kms及びevmファイルフォーマット用パケットフラグ
	*/
enum {
	DG_PACKFLAG_ENVELOPE	= 0x0001,	/* エンベロープ処理付き	*/
	DG_PACKFLAG_CULLAUTO	= 0x0002,	/* プリシェード時に距離に応じてカリング処理	*/
	DG_PACKFLAG_CULLON		= 0x0004,	/* プリシェード時にカリング処理ＯＮ	*/
	/* 描画属性（マルチテクスチャ時のみ有効） */
	DG_PACKFLAG_TEX0	= 0x0008,	/* テクスチャ０使用（必須なので特に必要ない）*/
	DG_PACKFLAG_TEX1	= 0x0010,	/* テクスチャ１使用 */
	DG_PACKFLAG_TEX2	= 0x0020,	/* テクスチャ２使用 */
	DG_PACKFLAG_UV0		= 0x0040,	/* 通常ＵＶ０使用（必須なので特に必要ない） */
	DG_PACKFLAG_UV1		= 0x0080,	/* 通常ＵＶ１使用 */
	/* 拡張計算フラグ（下のフラグは共に排他でなければならない）*/
	DG_PACKFLAG_UV2		= 0x0100,	/* 通常ＵＶ２使用 */
	DG_PACKFLAG_EMAP	= 0x0200,	/* 環境マッピング（ＵＶ２へ格納） */
	DG_PACKFLAG_SMAP	= 0x0400,	/* 環境マッピング影（ＵＶ２へ格納） */
	DG_PACKFLAG_BMAP	= 0x0800,	/* バンプマッピング（ＵＶ１をずらしたものをＵＶ２へ格納） */
} ;


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
typedef struct {
	unsigned short		u, v ;
} TSVECTOR ;

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
#if 0
/* ＰＳＹメタルギア用新モデルフォーマット */

typedef	struct _KMS_OBJPACK	{
	unsigned int	flag ;			/* 属性フラグ			*/
	unsigned int	n_verts ;		/* ポリゴン数			*/
	unsigned int	tex_id ;		/* テクスチャＩＤ			*/
	unsigned int	pad ;			/* リザーブ			*/
	short			*verts ;		/* 頂点データ			*/
	short			*norms ;		/* 法線データ			*/
	short			*uvs ;			/* ＵＶデータ			*/
	CVECTOR			*rgbs ;			/* プリシェードデータへ		*/
} KMS_OBJPACK ;

typedef	struct _NEW_MDL	{
	unsigned int	type ;			/* 属性フラグ			*/
	unsigned int	n_packs ;		/* ポリゴン数			*/
	float			lx, ly, lz ;	/* バウンディングボックス	*/
	float			ux, uy, uz ;	/*				*/
	float			tx, ty, tz ;	/* ユニットの相対座標		*/
	int				parent ;		/* 親ユニット番号		*/
	KMS_OBJPACK		*packs ;		/* オブジェクトパケット		*/
	int				pad0 ;			/* リザーブ		*/
	int				pad1 ;			/* リザーブ		*/
	int				pad2 ;			/* リザーブ		*/
} KMS_MDL ;

typedef	struct	_NEW_DEF{
	unsigned int	n_models ;	/* ユニット数			*/
	unsigned int	n_x_models ;/* ユニット数（拡張ユニット込）	*/
	float		lx, ly, lz ;	/* 全体バウンディングボックス	*/
	float		ux, uy, uz ;	/*				*/
	KMS_MDL		models[ NULL_ARRAY ] ;	/* ユニット配列			*/
} KMS_DEF ;
#else
/* ＰＳ２メタルギア用マルチテクスチャ対応モデルフォーマット */
typedef	struct _DG_MDLPACK	{
	unsigned int	flag ;			/* 属性フラグ						*/
	unsigned int	n_verts ;		/* ポリゴン数						*/
	unsigned int	tex_id[3] ;		/* テクスチャＩＤ（３セット）		*/
	short			*verts ;		/* 頂点データ						*/
	short			*norms ;		/* 法線データ						*/
	short			*uvs[3] ;		/* ＵＶデータ（３セット）			*/
	CVECTOR			*rgbs ;			/* プリシェードデータへ（未使用）	*/
	int				pad ;			/* リザーブ */
} KMS_MDLPACK ;

typedef	struct _DG_MDL	{
	unsigned int	type ;			/* 属性フラグ				*/
	unsigned int	n_packs ;		/* ポリゴン数				*/
	float			lx, ly, lz ;	/* バウンディングボックス	*/
	float			ux, uy, uz ;	/* バウンディングボックス	*/
	float			tx, ty, tz ;	/* ユニットの相対座標		*/
	int				parent ;		/* 親ユニット番号			*/
	KMS_MDLPACK		*packs ;		/* オブジェクトパケット		*/
	int				pad0 ;			/* リザーブ		*/
	int				pad1 ;			/* リザーブ		*/
	int				pad2 ;			/* リザーブ		*/
} KMS_MDL ;

typedef	struct	_DG_DEF{
	unsigned int	data_format ;	/* データフォーマットＩＤ		*/
	unsigned int	n_models ;		/* ユニット数					*/
	unsigned int	n_x_models ;	/* ユニット数（拡張ユニット込）	*/
	unsigned int	texture ;		/* テクスチャファイルＩＤ		*/
	int				pad[3] ;
	float		lx, ly, lz ;		/* 全体バウンディングボックス	*/
	float		ux, uy, uz ;		/*								*/
	float		tx, ty, tz ;		/* モデルの基準表示位置			*/
	KMS_MDL		models[ NULL_ARRAY ] ;	/* ユニット配列				*/
} KMS_DEF ;

#define DG_GetMdlFormat(_def) ((_def)->data_format & MGS_MODEL_FORMAT_MASK )
#define DG_GetMdlFlag(_def)   ((_def)->data_format & ~MGS_MODEL_FORMAT_MASK )

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

/* 旧ファイルフォーマット */
typedef	struct _OLS_KMS_OBJPACK	{
	unsigned int	flag ;			/* 属性フラグ			*/
	unsigned int	n_verts ;		/* ポリゴン数			*/
	unsigned int	tex_id ;		/* テクスチャＩＤ			*/
	unsigned int	pad ;			/* リザーブ			*/
	short			*verts ;		/* 頂点データ			*/
	short			*norms ;		/* 法線データ			*/
	short			*uvs ;			/* ＵＶデータ			*/
	CVECTOR			*rgbs ;			/* プリシェードデータへ		*/
} OLD_KMS_OBJPACK ;

typedef	struct _OLD_KMS_MDL	{
	unsigned int	type ;			/* 属性フラグ			*/
	unsigned int	n_packs ;		/* ポリゴン数			*/
	float			lx, ly, lz ;	/* バウンディングボックス	*/
	float			ux, uy, uz ;	/*				*/
	float			tx, ty, tz ;	/* ユニットの相対座標		*/
	int				parent ;		/* 親ユニット番号		*/
	OLD_KMS_OBJPACK		*packs ;		/* オブジェクトパケット		*/
	int				pad0 ;			/* リザーブ		*/
	int				pad1 ;			/* リザーブ		*/
	int				pad2 ;			/* リザーブ		*/
} OLD_KMS_MDL ;

typedef	struct	_OLD_KMS_DEF{
	unsigned int	n_models ;	/* ユニット数			*/
	unsigned int	n_x_models ;/* ユニット数（拡張ユニット込）	*/
	float		lx, ly, lz ;	/* 全体バウンディングボックス	*/
	float		ux, uy, uz ;	/*				*/
	OLD_KMS_MDL		models[ NULL_ARRAY ] ;	/* ユニット配列			*/
} OLD_KMS_DEF ;

#endif

/*----------------------------------------------------------------*/

/* 共有頂点データファイルフォーマット */
typedef struct _COMMON_V_OBJ{
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
} COMMON_V_OBJ ;

typedef struct _COMMON_V_OBJS{
	int				id ;
	int				n_objs ;
	int				flag ;
	int				pad1 ;
	COMMON_V_OBJ	objs[ NULL_ARRAY ];
} COMMON_V_OBJS ;

enum {
	COMMON_VERTS		= 0x0001,
	COMMON_NORMS		= 0x0002,
	COMMON_UVS			= 0x0004,
};
/*----------------------------------------------------------------*/
	/*
		マルチウェイト対応ＤＭＡ転送モデルフォーマット
		
	*/

typedef struct _evm_pack {
	unsigned int	flag ;			/* 属性フラグ				*/
	unsigned int	tex_id0 ;		/* テクスチャＩＤ			*/
	unsigned int	tex_id1 ;		/* テクスチャＩＤ			*/
	unsigned int	tex_id2 ;		/* テクスチャＩＤ			*/
	unsigned int	n_verts ;		/* 頂点数（最大３２頂点）	*/
	unsigned int	n_mats ;		/* 使用マトリクス数			*/
	unsigned char	m0,m1,m2,m3 ;	/* 使用マトリクス番号		*/
	unsigned char	m4,m5,m6,m7 ;	/* 使用マトリクス番号		*/
	void			*verts ;		/* 頂点データ				*/
	void			*norms ;		/* 法線データ				*/
	void			*uvs0 ;			/* ＵＶデータ				*/
	void			*uvs1 ;			/* ＵＶデータ（オプション）	*/
	void			*uvs2 ;			/* ＵＶデータ（オプション）	*/
	void			*weight ;		/* ウェイトデータ			*/
	void			*rgbs ;			/* 頂点カラーデータ（現在未使用）*/
	int				pad1 ;			/* リザーブ領域				*/
} EVM_PACK ;

/* モデルスケルトン情報 */
typedef struct _evm_skel {
	int				flag ;
	int				parent ;		/* 親スケルトン番号				*/
	float			tx,ty,tz ;		/* 親からのオフセット位置		*/
	float			rt_tx, rt_ty, rt_tz ;	/* ルートからのオフセット */
} EVM_SKEL ;

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

/* 頂点データフォーマット */
/*
	各種頂点データの先頭アドレスは１６バイト境界に揃っていなければならない
	（つまり頂点データなどは８バイトなので頂点数が奇数の場合にはダミーを入れる必要がある）

	頂点データ
	short	x ;	Ｘ座標（固定小数点(1.15.0)）
	short	y ;	Ｙ座標（固定小数点(1.15.0)）
	short	z ;	Ｚ座標（固定小数点(1.15.0)）
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
	u_char	w0 ;	マトリクス０に対するウェイト値（固定小数点(1.3.12)）
	u_char	w1 ;	マトリクス１に対するウェイト値（固定小数点(1.3.12)）
	u_char	w2 ;	マトリクス２に対するウェイト値（固定小数点(1.3.12)）
	u_char	w3 ;	マトリクス３に対するウェイト値（固定小数点(1.3.12)）
	u_char	w4 ;	マトリクス０に対するウェイト値（固定小数点(1.3.12)）
	u_char	w5 ;	マトリクス１に対するウェイト値（固定小数点(1.3.12)）
	u_char	w6 ;	マトリクス２に対するウェイト値（固定小数点(1.3.12)）
	u_char	w7 ;	マトリクス３に対するウェイト値（固定小数点(1.3.12)）



*/
/*----------------------------------------------------------------*/



#ifdef __cplusplus
};
#endif

#endif

