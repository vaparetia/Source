/*
	poly_utl.h
	ポリゴン関連ユーティリティ

	2000/01/22 K.Takabe
	$Id: poly_utl.h,v 1.4 2000/02/05 06:30:53 usr02774 Exp $
*/

#ifndef __POLY_UTL_H__
#define __POLY_UTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define WEIGHT_MAX		(8)

/* 重み情報定義 */
typedef struct _weight_element {
	int		skel_num ;				/* スケルトン番号 */
	float	weight ;				/* スケルトン番号に対応した重み値 */
} WEIGHT_ELEMENT ;
typedef struct _weight {
	int				max_use ;				/* 最大使用数 */
	WEIGHT_ELEMENT	weight[WEIGHT_MAX] ;	/* ウェイト値 */
} WEIGHT ;

/* 頂点情報定義 */
typedef struct _vertex {
	FVECTOR		v ;			/* 頂点 */
	FVECTOR		n ;			/* 法線 */
	FVECTOR		uv[ 3 ] ;	/* UV座標（最大3セット） */
	WEIGHT		weight ;	/* 重み */
	int			id ;		/* 頂点認識用ＩＤ（現在未使用） */
} VERTEX ;

/* マテリアル情報定義 */
typedef struct _material {
	int			tex_id ;	/* テクスチャID */
} MATERIAL ;

/* ポリゴン情報定義 */
typedef struct _polygon {
	/* ポリゴングループ管理用 */
	struct _polygon		*prev ;				/* リンクリスト用 */
	struct _polygon		*next ;				/* リンクリスト用 */
	/* 内部使用拡張ワーク（ストリップポリゴン生成の高速化に使用する） */
	struct _polygon		*link[3] ;			/* 辺共有リンク */
	int					link_side[3] ;		/* 共有先の辺番号（0-1:0,1-2:1,2-0:2） */
	int					flag ;				/* 制御フラグ（ポリゴン情報とは無関係） */
	/* データの実体 */
	int					n_material ;		/* 有効マテリアル数 */
	VERTEX				vertex[ 3 ] ;		/* 頂点データ */
	MATERIAL			material[ 3 ] ;		/* マテリアルデータ */
} POLYGON ;

/* ポリゴンパケット（双方向リンクリスト） */
typedef struct _poly_packet {
	int					n_polygons ;
	POLYGON				start, end ;
} POLY_PACKET ;





/* ================================================================ */
/*
	ポリゴンパケット関連サブルーチン
 */
/* ポリゴンパケットの初期化 */
extern void InitPolygonPacket( POLY_PACKET *packet );
/* ポリゴンパケットにポリゴンを追加 */
extern void AddPolygon( POLY_PACKET *packet, POLYGON *poly );
/* ポリゴンパケットから指定したポリゴンを削除 */
extern void DeletePolygon( POLY_PACKET *packet, POLYGON *poly );
/* ポリゴンパケットから先頭のポリゴンを取得する（リストからは削除しない） */
extern POLYGON* GetFirstPolygon( POLY_PACKET *packet );
/* 次のポリゴンを取得する（リストからは削除しない） */
extern POLYGON* GetNextPolygon( POLYGON *poly );
/* ポリゴン内容をコピーする（リンクリストの構造は変更しない） */
extern void CopyPolygon( POLYGON *dst_poly, POLYGON *src_poly );
/* ポリゴンパケットを複製する（パケットの初期化も行なう） */
extern void DuplicatePolygonPacket( POLY_PACKET *dst_packet, POLY_PACKET *src_packet );
/* ポリゴンパケットを加える（元は残らない） */
extern void MovePolygonPacket( POLY_PACKET *packet, POLY_PACKET *add_packet );
/* ポリゴンパケットを加える（元も残る） */
extern void AddPolygonPacket( POLY_PACKET *packet, POLY_PACKET *add_packet );
/* ポリゴンパケットを全て開放する（パケットの初期化も行なう） */
extern void FreePolygonPacket( POLY_PACKET *packet );
/* パケットに含まれるポリゴン数を返す */
extern int GetTotalPolygons( POLY_PACKET *packet );


/* ================================================================ */
/*
	ポリゴン管理処理
 */
/* ポリゴン管理システムの初期化 */
extern void InitPolygonManagement( void );
/* ポリゴン管理システムの解放 */
extern void EndPolygonManagement( void );
/* ポリゴンデータの確保 */
extern POLYGON* AllocPolygon( void );
/* ポリゴンデータの解放 */
extern void FreePolygon( POLYGON *poly );

/* ================================================================ */
/*
	デバッグ関連
 */
/* 内部の情報を表示 */
extern void DisplayPolygonManagementStatus( void );
/* ベクトルの表示 */
extern void DisplayVector( char *mes, FVECTOR *vec );
/* 重み情報の表示 */
extern void DisplayWeight( char *mes, WEIGHT *w );
/* 頂点情報の表示 */
extern void DisplayVertex( char *mes, VERTEX *v );
/* ポリゴン情報の表示 */
extern void DisplayPolygon( char *mes, POLYGON *poly );

/* ================================================================ */
/*
	関連ユーティリティ
 */
/* 頂点情報の初期化 */
extern void InitVertex( VERTEX *v );
/* ２つの重み情報を揃える */
extern void ReorderVertexWeight( WEIGHT *new1, WEIGHT *new2, WEIGHT *we1, WEIGHT *we2 );
/* スケルトン番号の若い醇に並べ替えを行なう */
extern void SortWeight( WEIGHT *res, WEIGHT *we );

/* ２つの実数が等しいかチェックする（誤差を考慮する） */
extern int IsEqualFloatAbout( float f1, float f2, float error );
/* ２つの頂点が等しいかチェックする（誤差を考慮する） */
extern int IsEqualVectorAbout( FVECTOR *vec1, FVECTOR *vec2 );
/* ２つの頂点が等しいかチェックする */
extern int IsEqualVector( FVECTOR *vec1, FVECTOR *vec2 );
/* ２つの法線が等しいかチェックする */
extern int IsEqualNormal( FVECTOR *norm1, FVECTOR *norm2 );
/* ２つのＵＶが等しいかチェックする */
extern int IsEqualUV( FVECTOR *uv1, FVECTOR *uv2 );
/* ２つの頂点重みが等しいかチェックする */
extern int IsEqualWeight( WEIGHT *we1, WEIGHT *we2 );
/* ２つの頂点が共有できるかチェックする */
extern int IsEqualVertex( VERTEX *v1, VERTEX *v2 );
/* ２つのポリゴンが同じかチェックする */
extern int IsEqualPolygon( POLYGON *poly1, POLYGON *poly2 );
/* 頂点ＩＤから２つの頂点が共有できるかチェックする */
extern int IsEqualVertexFast( VERTEX *v1, VERTEX *v2 );

/* ２つのポリゴンがマルチテクスチャとして合成できるかチェックする */
extern int IsJoinMultiTexturePolygon( POLYGON *org_poly, POLYGON *add_poly );
/* ２つのポリゴンをマルチテクスチャとして合成する */
extern void JoinMultiTexturePolygon( POLYGON *org_poly, POLYGON *add_poly );


/* パケットで使用しているスケルトン数を調べる（重たいかも） */
extern int GetUseSkeletonWeight( POLY_PACKET *packet );
/* ポリゴンパケット情報を整理する（ただし使用スケルトン数が８以内であることが保証されている場合に限る） */
extern void OptimizePolygonPacket( POLY_PACKET *packet );

/* 現在の頂点ＩＤ総計を初期化する */
extern void IndexedVertexInitialize( void );
/* 現在の頂点ＩＤ総計を取得する */
extern int GetIndexedVertexCount( void );
/* パケット内のポリゴンの頂点ＩＤを初期化する */
extern void IndexedVertexClear( POLY_PACKET *packet );
/* パケット内の頂点共有をチェックし、独立した頂点ＩＤを割り当てる */
extern void IndexedVertex( POLY_PACKET *packet );


/* ストリップ化を行なうためにパケットを初期化する */
/* （以後直接触ってはいけないので注意！！！） */
extern void Strip_InitPolygonPacketLink( POLY_PACKET *packet );
/* パケットのフラグのみを初期化する */
extern void Strip_InitPolygonPacketFlag( POLY_PACKET *packet );
/* 指定したポリゴンから始まるストリップの接続数を計算する */
extern int Strip_GetStripPolygons( POLY_PACKET *packet, POLYGON *poly, int type );
/* 指定したポリゴンから始まるストリップを切り出す（リンク情報も補正する） */
extern int Strip_CutStripPolygons( POLY_PACKET *strip, POLY_PACKET *packet, POLYGON *poly, int type );
/* 先頭のポリゴンからストリップになるように頂点を入れ替える */
/* （但し全ポリゴンのストリップ化が保証されていること） */
extern void Strip_OptimizeStripPacket( POLY_PACKET *strip_packet, int type );


#ifdef __cplusplus
}
#endif

#endif
