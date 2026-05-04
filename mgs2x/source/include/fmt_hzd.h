/*
   fmt_hzx.h 
   hzx ファイルフォーマット

   1999/07/07 M.Sonoyama
   2000/01/24 K.Uehara		hzd->hzx
   $Id: fmt_hzd.h,v 1.1.1.3 2002/11/19 11:41:58 Yoshizawa1 Exp $
*/

#ifndef	__FMT_HZX__H__
#define	__FMT_HZX__H__

#define		HZX_VERSION	(3)

/* ベクトルデータ */
typedef	struct	{
    float	x, z, y, h ;
} HZD_VEC ;

/* 壁データ */
typedef struct	{
    HZD_VEC	p1, p2 ;
} HZD_SEG ;

/* 床データ */
typedef struct 	{
    HZD_VEC	b1, b2 ;	/* バウンディングボックス */
    HZD_VEC	p1, p2, p3, p4 ;
} HZD_FLR ;

/* 巡回ポイントデータ */
typedef	struct	{
    float	x, z, y ;		/* 位置ベクトル */
    float	ax, az, ay ;	/* 注視点位置 */
    short	act ;			/* アクション番号 */
    short	time ;			/* 継続時間 */
    short	dir ;			/* 方向 ( 0 ～ 4095 ) */
    short	pad ;
    int		flag ;			/* 巡回ポイントフラグ */
    int		group_id ;
} HZD_PTP ;

/* 巡回ルートデータ */
typedef	struct	{
    short	n_points ;	/* ポイント数 */
    short	pad ;
    HZD_PTP	*points ;	/* ポイントデータへのポインタ */
    int		flag ;		/* ルートフラグ */
} HZD_PAT ;

/*--- クリアリングデータ ---*/

/* ポイントデータ */
typedef	struct	{
    float	x, z, y ;	/* 座標 */
    float	ax, ay, az ;	/* 注視点座標 */
    short	act ;		/* アクション */
    short	time ;		/* アクション時間 */
    short	dir ;		/* アクション方向 0-4095 */
    short	pad ;		/*  */
    int		group_id ;
} HZD_CLE_PTP ;

/* ルートデータ */
typedef	struct	{
    short		n_points ;	/* ポイント数 */
    short		pad ;
    int			align_16[2] ;
    HZD_CLE_PTP		*points ;	/* ルート別ポイントデータ先頭アドレス */
} HZD_CLE_ROOT ;

/* エリアデータ */
typedef	struct	{
    FVECTOR		b1, b2 ;	/* バウンディングボックス */
    int			name ;		/* 名前 strcode */
    u_short		go_time ;	/* 突入までの時間 */
    u_short		n_root ;	/* クリアリングルート数 */
    HZD_CLE_ROOT	*roots ;	/* エリア別ルートデータ先頭アドレス */
    int			align_16 ;
} HZD_CLE_AREA ;

/*----------------------------------------*/

#define	HZD_MAX_VUSEG_VERTS	(320)	/* １ＶｕＳＥＧ中最大頂点数 */
typedef	struct	{
    SVECTOR	b1, b2 ;	/* 壁群バウンディング */
    SVECTOR	*verts ;	/* 頂点列 */
    u_int	atr ;		/* 壁アトリビュート */
    u_int	size ;		/* タグサイズ */
    u_int	*tag ;		/* Ｖｕ０設定タグ */
} HZD_VuSEG ;
/* b1.pad には頂点数が入る */
/* 床群の場合、b2.pad に構成床形状が入る（三角形／四角形：３／４） */

typedef	struct	{
    SVECTOR	b1, b2 ;
    int		name_id ;
} HZD_TRP ;

/* ビハインドデータ */
typedef	struct	{
    FVECTOR	b1, b2 ;	/* バウンディング */
    float	v[ 6 ] ;	/* カメラデータ */
    float	right[ 6 ] ;	
    float	left[ 6 ] ;	
    int		pad[ 2 ] ;	
} HZD_BEHIND ;

/* ゾーンデータ */
#define	HZD_MAX_SAFEZONE_NUM	(4)
typedef	struct	{
    float	x, z, y ;
    short	w, h ;		/* ゾーンサイズ */
    short	flag ;		/* ゾーンフラグ */
    u_char	nears[ 6 ] ;	/* 隣接ゾーン */
    u_char	dists[ 6 ] ;	/* 隣接ゾーンへの距離 */
    u_char	near_flag[ 6 ] ;	/* 隣接ゾーン間のフラグ */

    /* 安全地帯 */
    u_char	safes[ HZD_MAX_SAFEZONE_NUM ] ; /* 安全地帯ゾーン番号 */
    u_char	safe_dists[ HZD_MAX_SAFEZONE_NUM ] ;/* 安地ゾーンへ距離 */
    u_int	safe_types[ HZD_MAX_SAFEZONE_NUM ] ; /* 安地ゾーンタイプ */
} HZD_ZON ;

/* 
   ゾーンのフラグ
*/
enum {
    HZD_ZON_BEHIND	= 0x0001,	/* 強制敵ビハインドゾーン */
    HZD_ZON_ALERT	= 0x0002,	/* 強制敵アラートゾーン	  */

    HZD_ZON_LINK	= 0x8000	/* リンクゾーン		  */
} ;
/* 
   リンクゾーンのしくみ 

   flag => linkzoneフラグが立っている、のとき、
   このゾーンはグループ接続ゾーン（リンクゾーン）。
   このとき、
            nears[ 5 ] : 繋がっている先のグループの接続ゾーン番号
	    dists[ 5 ] : 繋がっている先のグループの接続ゾーンへの距離
            near_flag[ 5 ] : 繋がっている先のグループ番号
   になっている。
*/

typedef	struct	{
    int		tx, tz, ty, th ;	/* ブロック中心 */
    int		extension ;		/* 拡張ブロック番号 */
    u_short	n_segs, n_flrs ;	/* 壁VuSEG数・床VuSEG数 */
    u_short	n_bul_segs, n_bul_flrs ;/* 弾壁VuSEG数・床VuSEG数 */
    u_short	n_traps, block_no;	/* トラップ数、 ブロックナンバー */
    HZD_VuSEG	*segs ;			/* 壁データへのポインタ */
    HZD_VuSEG	*flrs ;			/* 床データへのポインタ */
    HZD_VuSEG	*bul_segs ;		/* 弾壁データへのポインタ */
    HZD_VuSEG	*bul_flrs ;		/* 弾床データへのポインタ */ 
    HZD_TRP	*traps ;		/* トラップへのポインタ */
} HZD_BLOCK ;

typedef	ALIGN16_DECL(struct) _HZD_D_SEGMENT {
    int				tag[ 8 ] ;
    IVECTOR			p1, p2  ;
    IVECTOR			b1, b2 ;
    int				atr ;
    struct _HZD_D_SEGMENT	*next ;
} HZD_D_SEGMENT  ;

typedef	ALIGN16_DECL(struct) _HZD_D_FLOOR {
    int				tag[ 12 ] ;
    IVECTOR			p1, p2, p3, p4 ;
    IVECTOR			b1, b2 ;
    int				atr ;
    struct _HZD_D_FLOOR		*next ;
} HZD_D_FLOOR  ;

typedef	struct	{
    HZD_D_SEGMENT	*segs ;		
    HZD_D_FLOOR		*flrs ;
    u_int		n_segs, n_flrs ;
} HZD_DYNAMICS ; 

typedef	struct	{
    /* 全ブロック共通 */
    u_int		n_behinds ;
    u_int		n_zones ;
    HZD_BEHIND		*behinds ;
    HZD_ZON		*zones ;
    u_int		n_link_zones ;
    u_char		*link_zone ;
    u_char		*route ;
    u_int		*online_info ;

    /*-----------------------------*/

    int			bound_min_x, bound_min_y, bound_min_z ;
    int			block_size_x, block_size_y, block_size_z ;
    u_char		div_x, div_y, div_z, pad ;
    u_short		n_blocks ;
    u_short		n_x_blocks ;
    HZD_BLOCK		*blocks ;
    HZD_DYNAMICS	*dynamics ;
} HZD_GRP ;
#define	MAX_LINK_ZONE	16	/* １グループのリンクゾーン最大数 */
/* 
   link_zonesとは？ 

   ゾーンのなかで、グループ間を接続しているものを
   リンクゾーンといいます。
   配列link_zonesには、このリンクゾーンのリストが入っています。
   これは次のような構造になっています。
   link_zones[ 0 ] : リンクゾーンの所属するグループ番号
   link_zones[ 1 ] : リンクゾーンの、グループ内でのゾーン番号
                   ・  
		   ・	
		   ・
   （リンクゾーンの数( n_link_zones )だけ続く。）
*/

typedef	struct	{
    u_short		version ;

    u_short		n_groups ;
    /* 敵設定 （全グループ共通） */
    u_short		n_patrols ;
    u_short		n_clears ;
    HZD_PAT		*patrols ;
    HZD_PTP		*points ;	
    HZD_CLE_AREA	*cle_areas ;
    HZD_CLE_ROOT	*cle_roots ;
    HZD_CLE_PTP		*cle_points ;

    /*------------------------------*/

    HZD_GRP		*groups ;
} HZD_DEF ;

#endif /* __FMT_HZX__H__ */
