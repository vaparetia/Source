/*
   fmt_km4.h : .km4 宣言ファイル
	基本は、km3と変わりませんが、mdl内のUserData領域を保存できるように
	変更しました。

   by M.Sonoyama 1999.Sep.～
   Modified by K.Kano , 11/11/1999

   $Id: fmt_km4.h,v 1.14 2002/08/26 11:22:09 usr01363 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST 
*/

#ifndef __FMT_KM4_H__
#define __FMT_KM4_H__

/*-----------------------------------------------------------------*/
#include <sys/types.h>

#include "fmt_kms.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY  1
#endif


#define	MAX_TEXTURES	(1024)
#define	MAX_NAME_LEN	(256)

/*----------------------------------------------------------------*/

/* .km3フォーマット（ツール内部フォーマット） */

/* km3モデルデータの操作やデータ確保 */
typedef struct _KM3_MDL {
    u_int 	type ;
    u_int 	n_prims ;
    float 	lx, ly, lz, ux, uy, uz ;
    float 	tx, ty, tz ;
    int		parent ;
    int 	extend ;
    
    /* 頂点データ */
    u_int       n_verts ;
    FVECTOR     *verts ;
    u_short     *vid ;
    
    /* 法線データ */
    u_int        n_norms ;
    FVECTOR     *norms ;
    u_short     *nid ;
    
    /* テクスチャデータ */
    TVECTOR     *uvs ;
    u_int	*tid ;

    /* エンベロープデータ */
    FVECTOR	*envs ;
    int		pad ; 

    /* RGBデータ (通常は NULL）*/
    CVECTOR	*rgbs ;      
} KM3_MDL;

/* km3 モデルのデータフォーマット */
typedef struct _KM3_DEF {
    u_int 	n_models ;
    u_int 	n_x_models ;
    float 	lx, ly, lz, ux, uy, uz ;
    KM3_MDL	models[ 0 ] ;
} KM3_DEF;

/* ツール用 */
typedef	struct	_KM3_DEF2 {
    KM3_DEF	*def ;
    int		n_texs ;
    char	texs[ MAX_TEXTURES ][ MAX_NAME_LEN ] ;
} KM3_DEF2 ;


/*----------------------------------------------------------------*/

#define MGS2_MODELVERTEXSTATUSKEYNAME         "KONAMI MGS2 Model Vertex Status"

typedef struct tagMGS2MODELVERTEX	// MGS2 モデルの頂点属性
{
    int nVertexSwing;	// ゆれ属性
    double dWeight[4];	// 重み
} MGS2MODELVERTEX, *PMGS2MODELVERTEX, *LPMGS2MODELVERTEX;


#if 0

typedef struct {
    int dummy[NULL_ARRAY];
} VERTEX_USERDATA;

#else

typedef struct {
    int nVertexSwing;	// ゆれ属性
} OLD1_VERTEX_USERDATA;

typedef struct {
    int nVertexSwing;	// ゆれ属性
    short dWeight[4];	// 重み
} VERTEX_USERDATA;

#endif


typedef struct {
    int dummy[NULL_ARRAY];
} NORMAL_USERDATA;

typedef struct {
    int dummy[NULL_ARRAY];
} PRIMITIVE_USERDATA;


/* km4モデルデータの操作やデータ確保 */
typedef struct _KM4_MDL {
    u_int 	type ;
    u_int 	n_prims ;
    float 	lx, ly, lz, ux, uy, uz ;
    float 	tx, ty, tz ;
    int		parent ;
    int 	extend ;
    
    /* 頂点データ */
    u_int       n_verts ;
    FVECTOR     *verts ;
    u_short     *vid ;
    
    /* 法線データ */
    u_int        n_norms ;
    FVECTOR     *norms ;
    u_short     *nid ;
    
    /* テクスチャデータ */
    TVECTOR     *uvs ;
    u_int	*tid ;

    /* エンベロープデータ */
    FVECTOR	*envs ;
    int		pad ; 

    /* RGBデータ (通常は NULL）*/
    CVECTOR	*rgbs ;      

    /* 追加 */
    /* 頂点の追加情報、総数はn_verts */
    VERTEX_USERDATA	*vert_usrdata;

    /* 法線の追加情報、総数はn_norms */
    NORMAL_USERDATA	*norm_usrdata;

    /* プリミティブの追加情報、総数はn_prim */
    PRIMITIVE_USERDATA	*prim_usrdata;
} KM4_MDL;

/* km4 モデルのデータフォーマット */
typedef struct _KM4_DEF {
    u_int 	n_models ;
    u_int 	n_x_models ;
    float 	lx, ly, lz, ux, uy, uz ;
    KM4_MDL	models[ 0 ] ;
} KM4_DEF;

/* ツール用 */
typedef	struct	_KM4_DEF2 {
    KM4_DEF	*def ;
    int		n_texs ;
    char	texs[ MAX_TEXTURES ][ MAX_NAME_LEN ] ;
} KM4_DEF2 ;

/*----------------------------------------------------------------*/

/* マルチテクスチャ対応フォーマット KM5 */

typedef struct _KM5_MDL {
    u_int 	type ;
    u_int 	n_prims ;
    float 	lx, ly, lz, ux, uy, uz ;
    float 	tx, ty, tz ;
    int		parent ;
    int 	extend ;
    
    /* 頂点データ */
    u_int       n_verts ;
    FVECTOR     *verts ;
    u_short     *vid ;
    
    /* 法線データ */
    u_int        n_norms ;
    FVECTOR     *norms ;
    u_short     *nid ;
    
    /* テクスチャデータ */
    TVECTOR     *uvs ;
    u_int	*tid ;

    /* エンベロープデータ */
    FVECTOR	*envs ;
    int		pad ; 

    /* RGBデータ (通常は NULL）*/
    CVECTOR	*rgbs ;      

    /* 追加 */
    /* 頂点の追加情報、総数はn_verts */
    VERTEX_USERDATA	*vert_usrdata;

    /* 法線の追加情報、総数はn_norms */
    NORMAL_USERDATA	*norm_usrdata;

    /* プリミティブの追加情報、総数はn_prim */
    PRIMITIVE_USERDATA	*prim_usrdata;
} KM5_MDL;

/* km5 モデルのデータフォーマット */
typedef struct _KM5_DEF {
    u_int 	n_models ;
    u_int 	n_x_models ;
    float 	lx, ly, lz, ux, uy, uz ;
    KM5_MDL	models[ 0 ] ;
} KM5_DEF;

/* ツール用 */
typedef	struct	_KM5_DEF2 {
    KM5_DEF	*def ;
    int		n_texs ;
    char	texs[ MAX_TEXTURES ][ MAX_NAME_LEN ] ;
} KM5_DEF2 ;

/*----------------------------------------------------------------*/

#define N_VERTEX_UVS		8

/* KM*形式を、共通の内部形式で管理する
   KM3,KM4,KM5は、ファイル出力段階で選択する */

typedef struct _KMX_ENV {
    float val[2];
    int objid,parentid,id;
} KMX_ENV;

typedef struct _KMX_VERTS {
    FVECTOR v;

    /* エンベロープ情報 */
    KMX_ENV env;

    /* RGBデータ */
    CVECTOR rgbs;

    /* 頂点の追加情報 */
    VERTEX_USERDATA vert_usrdata;
} KMX_VERTS;

typedef struct _KMX_NORMS {
    FVECTOR n;

    /* 法線の追加情報 */
    NORMAL_USERDATA norm_usrdata;
} KMX_NORMS;

typedef struct _KMX_UVS {
	int n_tid;
	u_int tid[N_VERTEX_UVS];
	TVECTOR uvs[N_VERTEX_UVS];
} KMX_UVS;

typedef struct _KMX_PRIM {
    /* 頂点情報 */
    int n_id;
    u_int *vid;
    u_int *nid;

    /* テクスチャ */
    int n_tid;
    u_int *tid;

    /* 総数は、n_id * n_tid */
    TVECTOR *uvs;

    /* プリミティブの追加情報 */
    PRIMITIVE_USERDATA prim_usrdata;

	int flag;
} KMX_PRIMS;

/* X-Box用頂点情報 */
typedef struct _KMX4X_VERTS {
	int vid,nid;
	KMX_VERTS vertex;
	KMX_NORMS normal;
	KMX_UVS uv;
} KMX_XVERTS;

typedef struct _KMX_MDL {
    u_int type ;
    float lx, ly, lz, ux, uy, uz ;
    float tx, ty, tz ;
    int parent ;
    int extend ;

    int n_verts;
    KMX_VERTS *verts;

    int n_norms;
    KMX_NORMS *norms;

    int n_prims;
    KMX_PRIMS *prims;

	int n_xverts;
	KMX_XVERTS *xverts;
} KMX_MDL;

typedef struct _TEXINFO {
	char *name;
	float alpha;
} TEXINFO;

typedef struct _KMX_DEF {
    u_int 	n_models ;
    u_int 	n_x_models ;

    /* バウンディングボックス */
    float 	lx, ly, lz ;
    float	ux, uy, uz ;

    /* モデル全体のシフト座標 */
    float	tx, ty, tz ;

    /* テクスチャのファイル情報 */
    int		n_texs;
    TEXINFO *texs;

    KMX_MDL	models[NULL_ARRAY] ;
} KMX_DEF;


/*----------------------------------------------------------------*/

/* マルチウェイトモデル(ワンスキンモデル) */

typedef struct _EVF_ENV {
    int objid;
    float env;
} EVF_ENV;

typedef struct _EVF_VERTS {
    /* 頂点データ */
    FVECTOR v;

    /* エンベロープ、サイズはn_models */
    int n_envs;
    EVF_ENV *envs;

    /* 頂点カラー */
    CVECTOR rgbs;

    /* 頂点の追加情報 */
    VERTEX_USERDATA vert_usrdata;
} EVF_VERTS;

typedef KMX_NORMS EVF_NORMS;
typedef KMX_PRIMS EVF_PRIMS;

/* メッシュデータ */
typedef struct _EVF_MESH {
    /* 共有頂点列 */
    u_int n_verts;
    EVF_VERTS *verts;

    /* 共有法線列 */
    u_int n_norms;
    EVF_NORMS *norms;

    /* プリミティブ情報 */
    u_int n_prims;
    EVF_PRIMS *prims;
} EVF_MESH;

/* スケルトンデータ */
typedef struct _EVF_SKEL {
    int flag;
    int parent;
    float tx,ty,tz;
    float rt_tx,rt_ty,rt_tz;
} EVF_SKEL;

/* モデルデータ */
typedef struct _EVF_DEF {
    u_int n_models;			/* スケルトン数 */
    u_int n_x_models;			/* スケルトン数(拡張部込) */
    float lx,ly,lz,ux,uy,uz;		/* バウンディングボックス */

    int type ;

    int	n_texs;
    TEXINFO *texs;

    EVF_MESH mesh;			/* メッシュデータ */
    EVF_SKEL skeletons[NULL_ARRAY];	/* スケルトンデータ */
} EVF_DEF;


/*----------------------------------------------------------------*/

/* 共有頂点データファイルフォーマット、第二版 */
typedef struct _CV2_OBJ{
    int			n_verts ;		/* 共有頂点数 */
    int			n_verts_index ;		/* 展開頂点数 */
    FVECTOR		*verts ;		/* 共有頂点データアドレス */
    short		*verts_index ;		/* 頂点分配インデックステーブルアドレス */
    int			n_norms ;		/* 共有法線数 */
    int			n_norms_index ;		/* 展開法線数 */
    FVECTOR		*norms ;		/* 共有法線データアドレス */
    short		*norms_index ;		/* 法線分配インデックステーブルアドレス */
    int			n_uvs ;			/* 共有UV数 */
    int			n_uvs_index ;		/* 展開UV数 */
    FVECTOR		*uvs ;			/* 共有UVデータアドレス */
    short		*uvs_index ;		/* UV分配インデックステーブルアドレス */

    /* 追加 */
    /* 頂点の追加情報、総数はn_verts */
    VERTEX_USERDATA	*vert_usrdata;

    /* 法線の追加情報、総数はn_norms */
    NORMAL_USERDATA	*norm_usrdata;
} CV2_OBJ ;

typedef struct _CV2_OBJS{
    int			id ;  /* 705644 = StrCode("Common Vertex Data ver1.1") */
    int			n_objs ;
    int			flag ;
    int			pad1 ;
    CV2_OBJ		objs[ NULL_ARRAY ];
} CV2_OBJS ;

enum {
    COMMON_VERTS	= 0x0001,
    COMMON_NORMS	= 0x0002,
    COMMON_UVS		= 0x0004,

    COMMON_VERT_USRDATA	= 0x0008,
    COMMON_NORM_USRDATA	= 0x0010,
};


#ifdef __cplusplus
};
#endif

#endif
