/*
   fmt_kms.h : .km2 .kmsフォーマット定義ファイル

   モデルデータを扱う時はインクルードしてください

   by M.Sonoyama 1999.Sep.～
   $Id: fmt_kms.h,v 1.1 2002/03/08 07:23:38 usr02774 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST 
*/

#ifndef __FMT_KMS_H__
#define __FMT_KMS_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif


/*-----------------------------------------------------------------*/

/* 頂点 と 法線データを扱う */
typedef struct {
    u_char	r, g, b, pad ;
} CVECTOR ;

/*
typedef struct {
    u_char     	this, parent, id, pad ;
} ENVDATA ;
*/
typedef struct {
    short 	vx, vy, vz, vw ;
} SVECTOR ;

typedef struct {
    long 	vx, vy, vz, vw ;
} IVECTOR ;

typedef struct {
    float 	vx, vy, vz, vw ;
} FVECTOR ;

typedef struct {
    u_char 	u, v ;
} TVECTOR_OLD ;

typedef struct {
    float 	u, v, w ;
} TVECTOR ;

typedef struct {
    short 	u, v ;
} TVECTOR_S ;

/*-----------------------------------------------------------------*/

/* .kmsフォーマット */

typedef struct {
    long   	flag ;
    long   	n_verts ;
    long   	tid ;
    long	pad ;
    SVECTOR     *verts ;
    SVECTOR     *norms ;
    TVECTOR_S  	*uvs ;
    CVECTOR     *rgbs ;
} KMS_PKT;

typedef struct	{
    long        type ;
    long        n_packs ;
    float       lx, ly, lz ;
    float       ux, uy, uz ;
    float       tx, ty, tz ;
    long        parent ;
    KMS_PKT     *pack ;
    int		pad[ 3 ] ;
} KMS_OBJ ;

typedef struct {
    long        n_models ;
    long        n_x_models ;
    float       lx, ly, lz ;
    float       ux, uy, uz ;
    KMS_OBJ	objs[ 0 ] ;
} KMS_DEF ;

/*
   オブジェクト属性種類
   現在、旧モデルフォーマット（*.kmd）と同じ内容にしているが、
   場合によっては大幅な変更もありえるので注意。
*/

#if 0

enum {
    DG_TYPE_FT4		= 0x0000,	/* フラット（禁止）	*/
    DG_TYPE_GT4		= 0x0001,	/* グーロー（必須）	*/
    DG_TYPE_TRANS	= 0x0002,	/* 半透明化		*/
    DG_TYPE_NOSHADE	= 0x0004,	/* 光源なし		*/
    DG_TYPE_RAISED	= 0x0100,	/* 優先アップ		*/
    DG_TYPE_SUNKEN	= 0x0200,	/* 優先ダウン		*/
    DG_TYPE_SINGLESIDE	= 0x0400,	/* 片面			*/
    DG_TYPE_OVERLAY0	= 0x0800,	
    DG_TYPE_OVERLAY1	= 0x1000,	
    DG_TYPE_OVERLAY2	= 0x2000,
    DG_TYPE_EXTEND	= 0x4000,
} ;

#else

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


#endif

/* オブジェクトパケットフラグ種類 */
enum {
    DG_PACKET_NORMAL	= 0x00000000,	/* 通常処理 */
    DG_PACKET_ENVELOPE	= 0x00000001,	/* エンベロープ処理有効 */
    DG_PACKET_CULLRIGHT	= 0x00010000,	/* ストリップの右回りスタート */
    DG_PACKET_CULLLEFT	= 0x00020000,	/* ストリップの左回りスタート */
} ;


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/


/* KMSで、使われるマルチテクスチャの枚数の制限値 */
#define N_KMS_MULTI_TEXTURES	3


	/*
		データフォーマットＩＤ
	*/
enum {
	MGS_MODEL_NORM			= 13112177,	/* 通常モデル */
	MGS_MODEL_MULTITEX		= 11686819,	/* 通常マルチテクスチャモデル */
	MGS_MODEL_MULTIWEIGHT	= 13335939,	/* マルチウェイトエンベロープモデル */
} ;

	/*
		新kms及びevmファイルフォーマット用パケットフラグ
	*/
enum {
	DG_PACKFLAG_ENVELOPE = 0x0001,	/* エンベロープ処理付き	*/
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


/* ＰＳ２メタルギア用マルチテクスチャ対応モデルフォーマット */
typedef	struct _KMS2_MDLPACK	{
    unsigned int	flag ;		/* 属性フラグ				*/
    unsigned int	n_verts ;	/* ポリゴン数				*/
    unsigned int	tex_id[3] ;	/* テクスチャＩＤ（３セット）		*/
    short		*verts ;	/* 頂点データ				*/
    short		*norms ;	/* 法線データ				*/
    short		*uvs[3] ;	/* ＵＶデータ（３セット）		*/
    CVECTOR		*rgbs ;		/* プリシェードデータへ（未使用）	*/
    int			pad ;		/* リザーブ */
} KMS2_MDLPACK ;

typedef	struct _KMS2_MDL	{
    unsigned int	type ;		/* 属性フラグ				*/
    unsigned int	n_packs ;	/* ポリゴン数				*/
    float		lx, ly, lz ;	/* バウンディングボックス		*/
    float		ux, uy, uz ;	/* バウンディングボックス		*/
    float		tx, ty, tz ;	/* ユニットの相対座標			*/
    int			parent ;	/* 親ユニット番号			*/
    KMS2_MDLPACK	*packs ;	/* オブジェクトパケット			*/
    int			pad0 ;		/* リザーブ		*/
    int			pad1 ;		/* リザーブ		*/
    int			pad2 ;		/* リザーブ		*/
} KMS2_MDL ;

typedef	struct	_KMS2_DEF{
    unsigned int	data_format ;	/* データフォーマットＩＤ		*/
    unsigned int	n_models ;	/* ユニット数				*/
    unsigned int	n_x_models ;	/* ユニット数（拡張ユニット込）		*/
    unsigned int	texture ;	/* テクスチャファイルＩＤ		*/
    int			pad[3] ;
    float		lx, ly, lz ;	/* 全体バウンディングボックス		*/
    float		ux, uy, uz ;	/*					*/
    float		tx, ty, tz ;	/* モデル全体のシフト値			*/
    KMS2_MDL		models[ NULL_ARRAY ] ;	/* ユニット配列			*/
} KMS2_DEF ;


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
				0x8000 ... 描画キックを行なわない
				0x4000 ... ＶＵ内部で使用するため使用してはならない
				0x0fff ... カリングＯＮ時に両面表示（従来のデフォルト）
				0x0020 ... カリングＯＮ時に左回りをカリング（将来の拡張用）
				0x0000 ... カリングＯＮ時に右回りをカリング（将来の拡張用）

	ＵＶデータ
	short	u ;	Ｕ座標（固定小数点(1.3.12)）
	short	v ;	Ｖ座標（固定小数点(1.3.12)）


	＜カリングフラグについての補足＞（実際にはまだ未実装）
	キックフラグのカリング用フラグはカメラから見て右回りの時にクリップする
	ポリゴン（つまり左回りが表の場合）にはキックフラグの下位１２ビットを
	0x0fffではなく0x0000にし、逆に左回りの時にクリップするポリゴン（右回りが表の場合）
	には0x0020を設定する。両面ポリゴンであれば0x0fffのままにする。
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
/*----------------------------------------------------------------*/

/* EVMで、使われるマルチテクスチャの枚数の制限値 */
#define N_EVM_MULTI_TEXTURES	3


#define EVM_TYPE_VER1			0x00000000
#define EVM_TYPE_VER2			0x01000000
#define EVM_TYPE_VERMASK		0xff000000


	/*
		マルチウェイト対応ＤＭＡ転送モデルフォーマット
		
	*/

typedef struct _evm_pack {
    unsigned int	flag ;			/* 属性フラグ				*/
    unsigned int	tex_id[3] ;		/* テクスチャＩＤ			*/
    unsigned int	n_verts ;		/* 頂点数（最大３２頂点）	*/
    unsigned int	n_mats ;		/* 使用マトリクス数			*/
    unsigned char	mat_id[8] ;		/* 使用マトリクス番号		*/
    void		*verts ;		/* 頂点データ				*/
    void		*norms ;		/* 法線データ				*/
    void		*uvs[3] ;		/* ＵＶデータ				*/
    void		*weight ;		/* ウェイトデータ			*/
    void		*rgbs ;			/* 頂点カラーデータ（現在未使用）*/
    int			pad1 ;			/* リザーブ領域				*/
} EVM_PACK ;

/* モデルスケルトン情報 */
typedef struct _evm_skel {
    int			flag ;
    int			parent ;		/* 親スケルトン番号			*/
    float		tx,ty,tz ;		/* 親からのオフセット位置		*/
    float		rt_tx, rt_ty, rt_tz ;	/* ルートからのオフセット */
} EVM_SKEL ;

/* モデルフォーマットヘッダ */
typedef struct _dg_mwdef {
    unsigned int	n_models ;		/* スケルトン数				*/
    unsigned int	n_x_models ;		/* スケルトン数（拡張ユニット込）	*/
    float		lx, ly, lz ;		/* 全体バウンディングボックス		*/
    float		ux, uy, uz ;		/*					*/
    int			type ;			/*					*/
    int			texture ;		/* 使用テクスチャグループＩＤ		*/
    int			n_packs ;		/* ポリゴンパケット数			*/
    EVM_PACK		*packet ;		/* ポリゴンパケット			*/
    EVM_SKEL		skeleton[ NULL_ARRAY ] ;
} EVM_DEF ;

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
	u_char	w0 ;	マトリクス０に対するウェイト値（固定小数点(0.1.7)）
	u_char	w1 ;	マトリクス１に対するウェイト値（固定小数点(0.1.7)）
	u_char	w2 ;	マトリクス２に対するウェイト値（固定小数点(0.1.7)）
	u_char	w3 ;	マトリクス３に対するウェイト値（固定小数点(0.1.7)）
	u_char	w4 ;	マトリクス４に対するウェイト値（固定小数点(0.1.7)）
	u_char	w5 ;	マトリクス５に対するウェイト値（固定小数点(0.1.7)）
	u_char	w6 ;	マトリクス６に対するウェイト値（固定小数点(0.1.7)）
	u_char	w7 ;	マトリクス７に対するウェイト値（固定小数点(0.1.7)）



*/

#ifdef __cplusplus
}
#endif

#endif
