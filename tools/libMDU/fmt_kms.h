/*
   fmt_kms.h : .km2 .kmsフォーマット定義ファイル [[KMS definition file format]]

   モデルデータを扱う時はインクルードしてください [[When working with data, please include model]]

   by M.Sonoyama 1999.Sep.～
   $Id: fmt_kms.h,v 1.13 2002/06/04 13:45:38 usr01363 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST 
*/

#ifndef __FMT_KMS_H__
#define __FMT_KMS_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 1
#endif


/*-----------------------------------------------------------------*/

/* 頂点 と 法線データを扱う [[Vertex and normal data deal]] */
typedef struct {
    u_char	r, g, b, pad ;
} CVECTOR ;

typedef struct {
   //BP - this renamed to _this to fix for .cpp
    u_char     	_this, parent, id, pad ;
} ENVDATA ;

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

/* .kmsフォーマット [[.kms format]] */

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

   [[Object Attribute Types
   Currently, the old model format (*. kmd) has to be the same as, in some cases could also note a significant change.]]
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
	DG_TYPE_FT4			= 0x0000,	/* 未使用	[[Unused]]							*/
	DG_TYPE_GT4			= 0x0001,	/* 未使用	[[Unused]]							*/
	DG_TYPE_TRANS		= 0x0002,	/* 半透明化 [[Semi-transparent]]								*/
	DG_TYPE_NOSHADE		= 0x0004,	/* 光源なし（プリシェード時のみ有効）[[No light (only available when preshade)]]	*/
	DG_TYPE_AUTOSIDE	= 0x0200,	/* 自動片面（プリシェード時のみ有効）[[Auto-sided (only available when preshade)]]	*/
	DG_TYPE_SINGLESIDE	= 0x0400,	/* 片面（プリシェード時のみ有効）[[Side (only available when preshade)]]	*/
	DG_TYPE_OVERLAY0	= 0x0800,	/* テクスチャの重ねぬり [[texture of superimposed coloring]]					*/
	DG_TYPE_OVERLAY1	= 0x1000,	/* テクスチャの重ねぬり	[[texture of superimposed coloring]]				*/
	DG_TYPE_OVERLAY2	= 0x2000,	/* テクスチャの重ねぬり	[[texture of superimposed coloring]]				*/
	DG_TYPE_EXTEND		= 0x4000,	/* 拡張モデル（parentに付随する）[[Extended model (parent accompanying)]]		*/
} ;


#endif

/* オブジェクトパケットフラグ種類 [[Object packet flag type]] */
enum {
    DG_PACKET_NORMAL	= 0x00000000,	/* 通常処理 [[Normal processing]] */
    DG_PACKET_ENVELOPE	= 0x00000001,	/* エンベロープ処理有効 [[Effective envelope processing]] */
    DG_PACKET_CULLRIGHT	= 0x00010000,	/* ストリップの右回りスタート [[Clockwise starting strip]] */
    DG_PACKET_CULLLEFT	= 0x00020000,	/* ストリップの左回りスタート [[Anticlockwise starting strip]] */
} ;


/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/


/* KMSで、使われるマルチテクスチャの枚数の制限値 [[The limit in the number of multi-texture is used]] */
#define N_KMS_MULTI_TEXTURES	3


	/*
		データフォーマットＩＤ [[Data format iD]]
	*/
enum {
	MGS_MODEL_NORM			= 13112177,	/* 通常モデル [[Normal model]] */
	MGS_MODEL_MULTITEX		= 11686819,	/* 通常マルチテクスチャモデル [[Normal multi-texture model]] */
	MGS_MODEL_MULTIWEIGHT	= 13335939,	/* マルチウェイトエンベロープモデル [[Multi-weight envelope model]] */

	MGS_MODEL_FORMAT_MASK	= 0x00ffffff,	/* 24 bit mask 上位ビットは拡張 [[extended hi-order bit]] */

	MGS_MODEL_FLAG_INDEX	= 0x01000000,	/* インデックスつきモデル（Xbox用）[[Indexed model (for xbox)]] */
} ;

	/*
		新kms及びevmファイルフォーマット用パケットフラグ [[New KMS and EVM packet flags for file format]]
	*/
enum {
	DG_PACKFLAG_ENVELOPE = 0x0001,	/* エンベロープ処理付き	[[with envelope processing]] */
	DG_PACKFLAG_CULLAUTO	= 0x0002,	/* プリシェード時に距離に応じてカリング処理 [[Preshade depending on the culling distance]]	 */
	DG_PACKFLAG_CULLON		= 0x0004,	/* プリシェード時にカリング処理ＯＮ	[[during the culling process preshade on]] */
	/* 描画属性（マルチテクスチャ時のみ有効）[[drawing attributes (only available when multiple textures)]] */
	DG_PACKFLAG_TEX0	= 0x0008,	/* テクスチャ０使用（必須なので特に必要ない）[[0 used textures (not necessary, especially because it is required)]] */
	DG_PACKFLAG_TEX1	= 0x0010,	/* テクスチャ１使用 */
	DG_PACKFLAG_TEX2	= 0x0020,	/* テクスチャ２使用 */
	DG_PACKFLAG_UV0		= 0x0040,	/* 通常ＵＶ０使用（必須なので特に必要ない） */
	DG_PACKFLAG_UV1		= 0x0080,	/* 通常ＵＶ１使用 */
	/* 拡張計算フラグ（下のフラグは共に排他でなければならない）[[Extended calculation flag (the flag must be under both exclusive)]] */
	DG_PACKFLAG_UV2		= 0x0100,	/* 通常ＵＶ２使用 */
	DG_PACKFLAG_EMAP	= 0x0200,	/* 環境マッピング（ＵＶ２へ格納） [[Environment mapping (UV2 stores)]] */
	DG_PACKFLAG_SMAP	= 0x0400,	/* 環境マッピング影（ＵＶ２へ格納）[[Shadow environment mapping (UV2 stores)]] */
	DG_PACKFLAG_BMAP	= 0x0800,	/* バンプマッピング（ＵＶ１をずらしたものをＵＶ２へ格納） [[Bump mapping (UV1 UV2 something shifted stores)]] */
} ;


/* 2002/6/4 K.Kano
   X-BOX用頂点フォーマット [[Vertex format for the xbox]] */
/* シングルテクスチャ [[single texture]] */
typedef struct _dg_vertex_kmss {
	short		vx, vy, vz ;	/* 頂点データ [[vertex data]] */
	short		wt ;			/* ウェイトデータ [[weight data]] */
	short		nx, ny, nz ;	/* 法線 [[normal]] */
	short		f ;				/* フラグ（未使用）[[flags (not used)]] */
	short		u0, v0 ;		/* テクスチャ座標 [[texture coords]] */
} KMSS_VERTEX ;

/* マルチテクスチャ [[multi-texturing]] */
typedef struct _dg_vertex_kmsm {
	short		vx, vy, vz ;	/* 頂点データ [[vertex data]] */
	short		wt ;			/* ウェイトデータ [[weight data]] */
	short		nx, ny, nz ;	/* 法線 [[normal]] */
	short		f ;				/* フラグ（未使用）[[flags (not used)]] */
	short		u0, v0 ;		/* テクスチャ座標 [[tex coords]] */
	short		u1, v1 ;		/* テクスチャ座標 */
	short		u2, v2 ;		/* テクスチャ座標 */
} KMSM_VERTEX ;


/* ＰＳ２メタルギア用マルチテクスチャ対応モデルフォーマット [[PS2 metal gear model format for multi-texture support]] */
typedef	struct _KMS2_MDLPACK	{
    unsigned int	flag ;		/* 属性フラグ		[[Attribute flags]]		*/
#if 0
    unsigned int	n_verts ;	/* ポリゴン数				*/
#else
 	unsigned short	n_verts ;		/* 頂点数 [[Number of vertices]] */
	unsigned short	n_indices ;		/* インデックス数 */
#endif
    unsigned int	tex_id[3] ;	/* テクスチャＩＤ（３セット）		*/
    short		*verts ;	/* 頂点データ				*/
    short		*norms ;	/* 法線データ				*/
    short		*uvs[3] ;	/* ＵＶデータ（３セット）		*/
    CVECTOR		*rgbs ;		/* プリシェードデータへ（未使用）[[to preshade data (unused)]]	*/
	//    int			pad ;		/* リザーブ [[reserved]] */
	unsigned short	*index;			/* KMS/EVM用頂点バッファ情報へのポインタ [[kms/evm pointer to vertex buffer for information]] */	
} KMS2_MDLPACK ;

typedef	struct _KMS2_MDL	{
    unsigned int	type ;		/* 属性フラグ		[[attribute flags]]		*/
    unsigned int	n_packs ;	/* ポリゴン数		[[number of polygons]]		*/
    float		lx, ly, lz ;	/* バウンディングボックス	[[bounding box]]	*/
    float		ux, uy, uz ;	/* バウンディングボックス	[[bounding box]]	*/
    float		tx, ty, tz ;	/* ユニットの相対座標		[[coordinates relative to the unit]]	*/
    int			parent ;	/* 親ユニット番号		[[parent unit number]]	*/
    KMS2_MDLPACK	*packs ;	/* オブジェクトパケット		[[packet objects]]	*/

#if 0
    int			pad0 ;		/* リザーブ		*/
    int			pad1 ;		/* リザーブ		*/
    int			pad2 ;		/* リザーブ		*/
#else
	/* 2002/6/4 K.Kano
	   X-BOX専用データも出力できるように修正 [[xbox can be modified to output-only data]] */
	void			*vbuff ;		/* 頂点バッファアドレス *//* XBOXでは頂点バッファメモリのアドレス */
									/* DG_VERTEX_KMSSまたはDG_VERTEX_KMSM形式 [[address of vertex buffer in _KMSS or _KMSM format]] */
	int				stride ;		/* 頂点バッファストライド	[[vertex buffer stride]] */
									/* sizeof(DG_VERTEX_KMSS)またはsizeof(DG_VERTEX_KMSM) [[sizeof(DG_VERTEX_KMSS) or sizeof(DG_VERTEX_KMSM)]] */
	int				n_verts ;		/* 共有化された頂点数の最大数 [[maximum number of shared vertices]] */
#endif

} KMS2_MDL ;

typedef	struct	_KMS2_DEF{
    unsigned int	data_format ;	/* データフォーマットＩＤ	[[data format id]]	*/
    unsigned int	n_models ;	/* ユニット数		[[number of units]]		*/
    unsigned int	n_x_models ;	/* ユニット数（拡張ユニット込）	[[number of units (including expansion units)]]	*/
    unsigned int	texture ;	/* テクスチャファイルＩＤ	[[texture file id]]	*/
    int			pad[3] ;
    float		lx, ly, lz ;	/* 全体バウンディングボックス	 [[entire bounding box]]	*/
    float		ux, uy, uz ;	/*					*/
    float		tx, ty, tz ;	/* モデル全体のシフト値	[[shift (scale?) value of the whole model]]		*/
    KMS2_MDL		models[ NULL_ARRAY ] ;	/* ユニット配列	[[array unit]]		*/
} KMS2_DEF ;


/* 頂点データフォーマット [[vertex data format]] */
/*
	各種頂点データの先頭アドレスは１６バイト境界に揃っていなければならない
	（つまり頂点データなどは８バイトなので頂点数が奇数の場合にはダミーを入れる必要がある）

   [[Start address of the vertex data must be aligned with the various 16-byte boundaries
   (ie, vertex data, such as when an odd number of vertices, so there is a need to put an 8-byte dummy)]]

	頂点データ [[vertex data]]
	short	x ;	Ｘ座標（固定小数点(1.15.0)）[[x coordinate (fixed point 1.15.0)]]
	short	y ;	Ｙ座標（固定小数点(1.15.0)）
	short	z ;	Ｚ座標（固定小数点(1.15.0)）
	short	w ; ウェイト値（固定小数点(1.3.12)）[[weight value]]
				ウェイト値は1.0でその関節に100%、0.0で親オブジェクトに100%になる [[weight value 1.0 = joint 100%, 0.0 = parent object 100%]]

	法線データ [[normal data]]
	short	x ;	法線ベクトルＸ（固定小数点(1.3.12)）
	short	y ;	法線ベクトルＹ（固定小数点(1.3.12)）
	short	z ;	法線ベクトルＺ（固定小数点(1.3.12)）
	short	f ;	キックフラグ（0x0fff:描画キック、0x8fff:頂点キックのみ）[[kick flag (0x0fff: drawing kick, 0x8fff: only the top kick)]]
				0x8000 ... 描画キックを行なわない [[but do not kick drawing]]
				0x4000 ... ＶＵ内部で使用するため使用してはならない [[VU for internal use should not be used because]]
				0x0fff ... カリングＯＮ時に両面表示（従来のデフォルト）[[Culling ON display at both (the default before)]]
				0x0020 ... カリングＯＮ時に左回りをカリング（将来の拡張用）[[Culling ON culled during counter-clockwise (for future expansion)]]
				0x0000 ... カリングＯＮ時に右回りをカリング（将来の拡張用）[[Culling ON culled during clockwise (for future expansion)]]

	ＵＶデータ [[data]]
	short	u ;	Ｕ座標（固定小数点(1.3.12)）
	short	v ;	Ｖ座標（固定小数点(1.3.12)）


	＜カリングフラグについての補足＞（実際にはまだ未実装）
	キックフラグのカリング用フラグはカメラから見て右回りの時にクリップする
	ポリゴン（つまり左回りが表の場合）にはキックフラグの下位１２ビットを
	0x0fffではなく0x0000にし、逆に左回りの時にクリップするポリゴン（右回りが表の場合）
	には0x0020を設定する。両面ポリゴンであれば0x0fffのままにする。
	また、ＭＧＳ２では右手座標系が採用されているので注意すること。


   [[<SUPPLEMENTAL culling flag> (actually, not yet implemented)
   Caring for the flag when the flag kick clip clockwise as viewed from the camera
   Polygons (ie if the table clockwise) is the lower 12 bits of the flag kick
   Instead of 0x0000 to 0x0fff polygon clipping at the opposite counterclockwise (clockwise if the table)
   Is set to 0x0020. If you leave 0x0fff-sided polygon.
   Also, MGS2, so attention should be given that right in the coordinate system is adopted.]]

	＜右手座標系＞ [[Right handed coordinate system]]
	  |+Y
	  |
	  |_____+X
	  /
	 /
	/+Z

*/

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* EVMで、使われるマルチテクスチャの枚数の制限値 [[The EVM, and limits the number of multi-texture is used]] */
#define N_EVM_MULTI_TEXTURES	3


#define EVM_TYPE_VER1			0x00000000
#define EVM_TYPE_VER2			0x01000000
#define EVM_TYPE_VERMASK		0xff000000
#define EVM_TYPE_INDEX			0x00000002


	/*
   マルチウェイト対応ＤＭＡ転送モデルフォーマット [[Model format for DMA transfers multi-weight]]		
	*/

typedef struct _evm_pack {
    unsigned int	flag ;			/* 属性フラグ		[[attribute flag]]		*/
    unsigned int	tex_id[3] ;		/* テクスチャＩＤ	[[texture id]]		*/
#if 0
    unsigned int	n_verts ;		/* 頂点数（最大３２頂点）[[number of vertices (up top 32)]]	*/
#else
	unsigned short	n_verts ;		/* 頂点数 [[num verts]] */
	unsigned short	n_indices ;		/* インデックス数 [[num indices]] */
#endif
    unsigned int	n_mats ;		/* 使用マトリクス数	[[number of matrices used]]		*/
    unsigned char	mat_id[8] ;		/* 使用マトリクス番号	 [[use matrix number]]	*/
    void		*verts ;		/* 頂点データ		[[vertex data]]		*/
    void		*norms ;		/* 法線データ		[[normal data]]		*/
    void		*uvs[3] ;		/* ＵＶデータ	[[uv data]]			*/
    void		*weight ;		/* ウェイトデータ	[[weight data]]		*/
    void		*rgbs ;			/* 頂点カラーデータ（現在未使用）[[vertex color data (currently unused)]] */
	//    int			pad1 ;			/* リザーブ領域				*/
	unsigned short	*index;			/* KMS/EVM用頂点バッファ情報へのポインタ [[KMS/EVM pointer to vertex buffer for info]] */	
} EVM_PACK ;

/* モデルスケルトン情報 [[ Skeleton model info]] */
typedef struct _evm_skel {
    int			flag ;
    int			parent ;		/* 親スケルトン番号	[[number of the parent skeleton]]		*/
    float		tx,ty,tz ;		/* 親からのオフセット位置	[[offset from the parent]]	*/
    float		rt_tx, rt_ty, rt_tz ;	/* ルートからのオフセット [[offset from the root]] */
} EVM_SKEL ;

/* モデルフォーマットヘッダ [[model format header]] */
typedef struct _dg_mwdef {
    unsigned int	n_models ;		/* スケルトン数		[[number of skeletons]]		*/
    unsigned int	n_x_models ;		/* スケルトン数（拡張ユニット込）[[number of skeletons (including expansion units)]]	*/
    float		lx, ly, lz ;		/* 全体バウンディングボックス	 [[bounding box]]	*/
    float		ux, uy, uz ;		/*					*/
    int			type ;			/*					*/
    int			texture ;		/* 使用テクスチャグループＩＤ	 [[use the texture group id]]	*/
    int			n_packs ;		/* ポリゴンパケット数		[[number of polygon packets]]	*/
    EVM_PACK		*packet ;		/* ポリゴンパケット	[[polygon packets]]		*/
    EVM_SKEL		skeleton[ NULL_ARRAY ] ;
} EVM_DEF ;

/* データの並び順について [[the order of the data]] */
/*

   <ヘッダー>  [[header]]
	EVM_DEF * 1

   <スケルトン情報> [[skeleton information]]
	EVM_SKEL * n_models

   <パケット情報> [[packet information]]
	EVM_PACK * n_packs

   <頂点情報> [[vertex information]]
	[verts data 0]
	[verts data 1]
	[verts data 2]
	...
	[verts data n_packs-1]

   <法線情報> [[normal information]]
	[norms data 0]
	......
	[norms data n_packs-1]

   <ＵＶ０情報> [[uv0 information]]
	[uvs[0] data 0]
	......
	[uvs[0] data n_packs-1]

   <ＵＶ１情報>（場合によっては省略可）[[sometimes optional]]
	[uvs[1] data 0]
	......
	[uvs[1] data n_packs-1]

   <ＵＶ２情報>（場合によっては省略可）[[sometimes optional]]
	[uvs[2] data 0]
	......
	[uvs[2] data n_packs-1]

   <ウェイト値情報> [[weights info]]
	[weight data 0]
	......
	[weight data n_packs-1]

	<EOF>
*/


/* 頂点データフォーマット [[vertex data format]] */
/*
	各種頂点データの先頭アドレスは１６バイト境界に揃っていなければならない
	（つまり頂点データなどは８バイトなので頂点数が奇数の場合にはダミーを入れる必要がある）

   [[Start address of the vertex data must be aligned with the various 16-byte boundaries
   (Ie, vertex data, such as when an odd number of vertices, so there is a need to put an 8-byte dummy)]]

   頂点データ [[vertex data]]
   short	x ;	Ｘ座標（固定小数点(1.15.0)）[[x coord fixed point 1.15.0]]
	short	y ;	Ｙ座標（固定小数点(1.15.0)）
	short	z ;	Ｚ座標（固定小数点(1.15.0)）
   short	f ;	キックフラグ（0x0fff:描画キック、0x8fff:頂点キックのみ）[[kick flag (0x0fff: Drawing kick, 0x8fff: Only the top kick)]]
   又は [[or]]
	short	x ;	Ｘ座標（固定小数点(1.11.4)）
	short	y ;	Ｙ座標（固定小数点(1.11.4)）
	short	z ;	Ｚ座標（固定小数点(1.11.4)）
	short	f ;	キックフラグ（0x0fff:描画キック、0x8fff:頂点キックのみ）

   法線データ [[normal data]]
	short	x ;	法線ベクトルＸ（固定小数点(1.3.12)）
	short	y ;	法線ベクトルＹ（固定小数点(1.3.12)）
	short	z ;	法線ベクトルＺ（固定小数点(1.3.12)）
	short	pad ;	

   ＵＶデータ [[uv data]]
	short	u ;	Ｕ座標（固定小数点(1.3.12)）
	short	v ;	Ｖ座標（固定小数点(1.3.12)）
   short	w ;	Ｗ座標（固定小数点(1.3.12)）但し精度が低いので1.0固定とする [[but because of the low precision fixed at 1.0]]
	short	pad ;

   ウェイトデータ  [[weight data]]
   u_char	w0 ;	マトリクス０に対するウェイト値（固定小数点(0.1.7)）[[weight for matrix 0]]
   u_char	w1 ;	マトリクス１に対するウェイト値（固定小数点(0.1.7)）[[weight for matrix 1]]
   u_char	w2 ;	マトリクス２に対するウェイト値（固定小数点(0.1.7)）[[...]]
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
