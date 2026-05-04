/*
   fmt_hzx.h 
   hzx ファイルフォーマット

   1999/07/07 M.Sonoyama
   2000/01/24 K.Uehara		hzd->hzx
   $Id: fmt_hzx.h,v 1.1.1.3 2002/11/19 11:41:58 Yoshizawa1 Exp $
*/

#ifndef	__FMT_HZX__H__
#define	__FMT_HZX__H__

#define		HZX_VERSION	(5)

#define	ROUTE_COURSE_TEST	(1)

#define	HW_SHORT_TO_FLOAT	(1)

/* ベクトルデータ */
typedef	struct	{
    float	x, z, y, h ;
} HZX_VEC ;

#if 0

/* 壁データ */
typedef struct	{
    HZX_VEC	p1, p2 ;
	void	*ptr ;
	int		attribute ;
	int		type ;
	int		reserved ;
} HZX_SEG ;

/* 床データ */
typedef struct 	{
    HZX_VEC	b1, b2 ;	/* バウンディングボックス */
    HZX_VEC	p1, p2, p3, p4 ;
	void	*ptr ;
	int		attribute ;
	int		type ;
	int		reserved ;
} HZX_FLR ;

#else

/* 共通データ */
typedef	struct	{
	HZX_VEC		p1, p2, p3, p4 ;
	void		*ptr ;
	int			attribute ;
	int			type ;
	int			reserved ;
} HZX_HZD ;

#define	HZX_SEG		HZX_HZD
#define	HZX_FLR		HZX_HZD

#endif

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
} HZX_PTP ;

/* 巡回ルートデータ */
typedef	struct	{
    short	n_points ;	/* ポイント数 */
    short	pad ;
    HZX_PTP	*points ;	/* ポイントデータへのポインタ */
    int		flag ;		/* ルートフラグ */
} HZX_PAT ;

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
} HZX_CLE_PTP ;

/* ルートデータ */
typedef	struct	{
    short		n_points ;	/* ポイント数 */
    short		pad ;
    int			align_16[2] ;
    HZX_CLE_PTP		*points ;	/* ルート別ポイントデータ先頭アドレス */
} HZX_CLE_ROOT ;

/* エリアデータ */
typedef	struct	{
    FVECTOR		b1, b2 ;	/* バウンディングボックス */
    int			name ;		/* 名前 strcode */
    u_short		go_time ;	/* 突入までの時間 */
    u_short		n_root ;	/* クリアリングルート数 */
    HZX_CLE_ROOT	*roots ;	/* エリア別ルートデータ先頭アドレス */
    int			align_16 ;
} HZX_CLE_AREA ;

/*----------------------------------------*/

#define	HZX_MAX_VUSEG_VERTS	(84)	/* １ＶｕＳＥＧ中最大頂点数 */
typedef	struct	{
    SVECTOR	b1, b2 ;	/* 壁群バウンディング */
    SVECTOR	*verts ;	/* 頂点列 */
    u_int	atr ;		/* 壁アトリビュート */
    u_int	size ;		/* タグサイズ */
    u_int	*tag ;		/* Ｖｕ０設定タグ */
} HZX_VuSEG ;
/* b1.pad には頂点数が入る */
/* 床群の場合、b2.pad に構成床形状が入る（三角形／四角形：３／４） */

typedef	struct	{
    SVECTOR	b1, b2 ;
    int		name_id ;
} HZX_TRP ;

/* ビハインドデータ */
typedef	struct	{
    FVECTOR	b1, b2 ;	/* バウンディング */
    float	v[ 6 ] ;	/* カメラデータ */
    float	right[ 6 ] ;	
    float	left[ 6 ] ;	
    int		value ;
    int		flag ;
} HZX_BEHIND ;

/* ゾーンデータ */
#define	HZX_MAX_SAFEZONE_NUM	(4)
typedef	struct	{
    float	x, z, y ;
#ifdef	HW_SHORT_TO_FLOAT
    float	w, h ;		/* ゾーンサイズ */
#else
    short	w, h ;		/* ゾーンサイズ */
#endif
    short	flag ;		/* ゾーンフラグ */
    u_char	nears[ 6 ] ;	/* 隣接ゾーン */
    u_char	dists[ 6 ] ;	/* 隣接ゾーンへの距離 */
    u_char	near_flag[ 6 ] ;	/* 隣接ゾーン間のフラグ */

    /* 安全地帯 */
    u_char	safes[ HZX_MAX_SAFEZONE_NUM ] ; /* 安全地帯ゾーン番号 */
    u_char	safe_dists[ HZX_MAX_SAFEZONE_NUM ] ;/* 安地ゾーンへ距離 */
    u_int	safe_types[ HZX_MAX_SAFEZONE_NUM ] ; /* 安地ゾーンタイプ */
} HZX_ZON ;

/* flag */
#define HZX_ZONE_ZINTRPT	0x01	/* ゾーン通行止め */
#define HZX_ZONE_SAFE		0x02	/* プレイヤー逃げ場所 */
#define HZX_ZONE_TOINTRUDE	0x04	/* イントルードへ至るゾーン(体験版仮) */
#define HZX_ZONE_INTRUDE	0x08	/* イントルード */
#define	HZX_ZONE_SLIDEDOOR	0x10	/* 自動ドアゾーン */
#define HZX_ZONE_LOW_HEIGHT	0x20	/* InZone判定の高さが低い */
#define HZX_ZONE_NO_AVOID	0x40	/*  */


/* near_flag */
#define HZX_ROOT_RINTRPT	0x01	/* ルート通行止め */ 
#define HZX_ROOT_COURSE1	0x02	/* ルートコース１ */ 
#define HZX_ROOT_COURSE2	0x04	/* ルートコース２ */ 

#define HZX_ROOT_JUMP		HZX_ROOT_COURSE1	/* ジャンプでのみ通れる*/ 

#define MAX_ROUTE_COURSE	4
/* 
   ゾーンのフラグ
*/
enum {
    HZX_ZON_BEHIND	= 0x0001,	/* 強制敵ビハインドゾーン */
    HZX_ZON_ALERT	= 0x0002,	/* 強制敵アラートゾーン	  */

    HZX_ZON_LINK	= 0x8000	/* リンクゾーン		  */
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
    HZX_VuSEG	*segs ;			/* 壁データへのポインタ */
    HZX_VuSEG	*flrs ;			/* 床データへのポインタ */
    HZX_VuSEG	*bul_segs ;		/* 弾壁データへのポインタ */
    HZX_VuSEG	*bul_flrs ;		/* 弾床データへのポインタ */ 
    HZX_TRP	*traps ;		/* トラップへのポインタ */
} HZX_BLOCK ;

/* 普通はこっち */
#define	HZX_TYPE_SEGMENT	(0x0000)
#define	HZX_TYPE_FLOOR		(0x0001)

/* Ｏｎｌｉｎｅの結果だけこっち */
#define	HZX_ONLINE_TYPE_SEGMENT	(1)
#define	HZX_ONLINE_TYPE_FLOOR	(2)

typedef	struct _HZX_D_CALLBACK {
	void						( *callback )( void	*, void *, void *, int ) ;
	void						*work1 ;	/* 引数１ */
	void						*work2 ;	/* 引数２ */
	struct	_HZX_D_CALLBACK		*next ;		/* 次のコールバック */
} HZX_D_CALLBACK ;

typedef	void	( *HZX_CALLBACK_FUNC )( void *, void *, void *, int ) ;

enum {
	HZX_CALLBACK_FLAG_SEGMENT = 	HZX_TYPE_SEGMENT,
	HZX_CALLBACK_FLAG_FLOOR = 		HZX_TYPE_FLOOR,
	HZX_CALLBACK_FLAG_DESTROY = 	0x1000,	/* 動的ハザード壊れた */
	HZX_CALLBACK_FLAG_PITFALL =		0x2000,	/* 落とし穴起動した */
} ;

typedef	ALIGN16_DECL(struct) _HZX_D_SEGMENT {
    int						atr ;
    struct _HZX_D_SEGMENT	*next ;
	HZX_D_CALLBACK			*callback ;
	void					*target ;
	/* align16 */
    int						tag[ 4 ] ;
    IVECTOR					p1, p2  ;
    IVECTOR					b1, b2 ;

    IVECTOR					def[ 2 ] ;
	FMATRIX					world ;
	FMATRIX					world2 ;
} HZX_D_SEGMENT  ;

typedef	ALIGN16_DECL(struct) _HZX_D_FLOOR {
    int						atr ;
    struct _HZX_D_FLOOR		*next ;
	HZX_D_CALLBACK			*callback ;
	void					*target ;
	/* align16 */
    int						tag[ 12 ] ;
    IVECTOR					p1, p2, p3, p4 ;
    IVECTOR					b1, b2 ;

    IVECTOR					def[ 4 ] ;
	FMATRIX					world ;
	FMATRIX					world2 ;
} HZX_D_FLOOR  ;

typedef	struct _HZX_D_TRP {
    FVECTOR				b1, b2 ;
    int					name_id ;
	struct _HZX_D_TRP	*next ;
	int					attribute ;
	int					reserved ;
} HZX_D_TRP ;

typedef	struct	{
    HZX_D_SEGMENT	*segs ;		
    HZX_D_FLOOR		*flrs ;
	HZX_D_TRP		*traps ;
    u_int			n_segs ; 
	u_int			n_flrs ;
	u_int			n_traps ;
	int				reserved[ 2 ] ;
} HZX_DYNAMICS ; 

typedef	struct	{
    /* 全ブロック共通 */
    u_int		n_behinds ;
    u_int		n_zones ;
    HZX_BEHIND	*behinds ;
    HZX_ZON		*zones ;
    u_int		n_link_zones ;
    u_char		*link_zone ;
    u_char		*route ;
    u_int		*online_info ;
#ifdef	ROUTE_COURSE_TEST
	int			courses ;
    u_short		*route_course[MAX_ROUTE_COURSE] ;	/* コース別テーブル */
#endif
    /*-----------------------------*/

    int			bound_min_x, bound_min_y, bound_min_z ;
    int			block_size_x, block_size_y, block_size_z ;
    u_char		div_x, div_y, div_z, pad ;
    u_short		n_blocks ;
    u_short		n_x_blocks ;
    HZX_BLOCK		*blocks ;
    HZX_DYNAMICS	*dynamics ;
} HZX_GRP ;
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
	u_short		vr_pat_offset[8]; //<- ＶＲ敵ルートオフセット配列です。
	u_short		vr_clr_offset[8]; //<- ＶＲクリアリングオフセット配列です。
    HZX_PAT		*patrols ;
    HZX_PTP		*points ;	
    HZX_CLE_AREA	*cle_areas ;
    HZX_CLE_ROOT	*cle_roots ;
    HZX_CLE_PTP		*cle_points ;

    /*------------------------------*/

    HZX_GRP		*groups ;
} HZX_DEF ;

#endif /* __FMT_HZX__H__ */
