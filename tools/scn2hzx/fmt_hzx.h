/*
   fmt_hzx.h : hzx ファイルフォーマット

   written by M.Sonoyama 1999.Jun ～ 
   $Id: fmt_hzx.h,v 1.16 2002/05/10 07:15:29 usr03635 Exp $
*/

#ifndef __FMT_HZX_H__
#define	__FMT_HZX_H__

#include "HZXStruct2.h"

// 4 -> 5 田中修正。ＶＲ対応
#define	HZX_VERSION	(5)

#define	ROUTE_COURSE_TEST	(1)

//#define	OLD

#ifndef __FMT_KMS_H__
typedef struct {
    short 	vx, vy, vz, vw ;
} SVECTOR ;

typedef	struct	{
    float	vx, vy, vz, vw ;
} FVECTOR ;
#endif

typedef	struct	{
    int		vx, vy, vz, vw ;
} IVECTOR ;

typedef	struct	{
    float	vx, vy ;
} DVECTOR ;

/* ビハインドデータ */
typedef	struct	{
    FVECTOR	b1, b2 ;	/* バウンディング */
    float	v[ 6 ] ;	/* カメラデータ */
    float	right[ 6 ] ;	
    float	left[ 6 ] ;	
    int		pad[ 2 ] ;	
} HZD_BEHIND ;

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

/*
   ※ブロック番号の付け方

   ( div_x, div_y, div_z ) = ( 4, 2, 3 ) の場合

   y = 0		y = 1
   -----------> x	-------------> x
   |0  1  2  3 		|12 13 14 15
   |			|
   |4  5  6  7 		|16 17 18 19
   |			|
   |8  9 10 11		|20 21 22 23
   |			|
   z			z

   拡張ブロックは、( div_x * div_y * div_z )番以降に割り当てる。

   ※拡張ブロック
   
   １ブロックの壁数が６４、床数が２８を超える場合は、
   拡張ブロックを作成する。拡張ブロックの拡張ブロックも
   存在してよい。

   ※ブロックへの割り振り方（現状。変更するかも）

   ------------------------------------------------------
   |block1		      |block2			|
   |			      |				|
   |			------------			|
   |			|     |	   |			| 両方に入る
   |			------------			|
   |			      |				|
   |			------|				| 
   |			|     |				| 両方に入る
   |			------|				|
   |			      |				|
   |			      |-----			|
   |			      |	   |			| block2にのみ入る
   |			      |-----			|
   |			      |				|
   ------------------------------------------------------

   ※分割数は、space上で指定するようにしてください。
*/

/*-----------------------------------------------------------*/

/* 最終ＨＺＸフォーマット */

#define	HZX_MAX_VUSEG_VERTS	(84)	/* １ＶｕＳＥＧ中最大頂点数 */
typedef	struct	{
    SVECTOR	b1, b2 ;	/* バウンディング */
    SVECTOR	*verts ;	/* 頂点データ列 */
    u_int	atr ;		/* アトリビュート */
    u_int	*tag ;		/* システム内部処理用 */
    u_int	size ;		/* システム内部処理用 */ 
} HZD_VuSEG ;
/* b1.vw(pad) には頂点数が入っている */
/* verts の しくみ */
/*
   ＜壁データ＞
   ラインストリップ
   
   ＜床データ＞
   三角形ＯＲ四角形ベタデータ
   b2.vwが３か４かで判断できる
   vertsのvwには、長さを３２０００に正規化した法線データ
   が入っている。
   verts->vw = normal->vx ;
   ( verts + 1 )->vw = normal->vy ;
   ( verts + 2 )->vw = normal->vz ;
*/
/*
   tag, size についてはツール内では
   使用しません。
   ＮＵＬＬや０を入れておいて下さい。
*/

typedef	struct	{
    SVECTOR	b1, b2 ;
    int		name_id ;
} HZD_TRP ;

/* ゾーンデータ */
#define	HZX_MAX_SAFEZONE_NUM	(4)
typedef	struct	{
    float	x, z, y ;
    float	w, h ;		/* ゾーンサイズ */
    short	flag ;		/* ゾーンフラグ */
    u_char	nears[ 6 ] ;	/* 隣接ゾーン */
    u_char	dists[ 6 ] ;	/* 隣接ゾーンへの距離 */
    u_char	near_flag[ 6 ] ;/* 隣接ゾーン間のフラグ */
#ifndef OLD
    u_char	safes[ HZX_MAX_SAFEZONE_NUM ] ; /* 安全地帯ゾーン番号 */
    u_char	safe_dists[ HZX_MAX_SAFEZONE_NUM ] ;/* 安地ゾーンへ距離 */
    u_int	safe_types[ HZX_MAX_SAFEZONE_NUM ] ; /* 安地ゾーンタイプ */
#endif
} HZD_ZON ;

/* near_flag */
#define HZX_ROOT_RINTRPT	0x01	/* ルート通行止め */ 
#define HZX_ROOT_COURSE1	0x02	/* ルートコース１ */ 
#define HZX_ROOT_COURSE2	0x04	/* ルートコース２ */ 

#define HZX_ROOT_JUMP		HZX_ROOT_COURSE1	/* ジャンプでのみ通れる*/ 

#define MAX_COURSE	(4)

/* 
   ゾーンのフラグ
*/
enum {
    HZD_ZON_BEHIND	= 0x0001,	/* 強制敵ビハインドゾーン */
    HZD_ZON_ALERT	= 0x0002,	/* 強制敵アラートゾーン	  */

    HZD_ZON_LINK	= 0x8000	/* リンクゾーン		  */
} ;

#define HZX_ZONE_ZINTRPT	0x01	/* ゾーン通行止め */
#define HZX_ZONE_SAFE		0x02	/* プレイヤー逃げ場所 */
#define HZX_ZONE_TOINTRUDE	0x04	/* イントルードへ至るゾーン(体験版仮) */
#define HZX_ZONE_INTRUDE	0x08	/* イントルード */
#define	HZX_ZONE_SLIDEDOOR	0x10	/* 自動ドアゾーン */
#define HZX_ZONE_LOW_HEIGHT	0x20	/* InZone判定の高さが低い */
#define HZX_ZONE_NO_AVOID	0x40	/*  */

/* 
   リンクゾーンのしくみ 

   flag => linkzoneフラグが立っている、のとき、
   このゾーンはグループ接続ゾーン（リンクゾーン）。
   このとき、
            nears[ 5 ] : 繋がっている先のグループの接続ゾーン番号
	    dists[ 5 ] : 繋がっている先のグループの接続ゾーンへの距離
            near_flag[ 5 ] : 繋がっている先のグループ番号
   になっている。（詳細未定）
*/

typedef	struct	{
    int		tx, tz, ty, th ;	/* ブロック中心 */
    int		extension ;		/* 拡張ブロック番号 */
    u_short	n_segs, n_flrs ;	/* 壁VuSEG数・床VuSEG数 */
    u_short	n_bul_segs, n_bul_flrs ; /* 弾壁VuSEG数・弾床VuSEG数 */
    u_short	n_traps, block_no;	/* トラップ数、 ブロックナンバー */
    HZD_VuSEG	*segs ;			/* 壁データへのポインタ */
    HZD_VuSEG	*flrs ;			/* 床データへのポインタ */
    HZD_VuSEG	*bul_segs ;		/* 弾用壁 */
    HZD_VuSEG	*bul_flrs ;		/* 弾用床 */
    HZD_TRP	*traps ;		/* トラップへのポインタ */
} HZD_BLOCK ;

typedef	struct	{
    HZD_VuSEG	*segs ;		
    HZD_VuSEG	*flrs ;
    u_short	n_segs, n_flrs ;
} HZD_DYNAMICS ; /* ALIGN16 */

typedef	struct	{
    /* 全ブロック共通 */
    u_int		n_behinds ;
    u_int		n_zones ;
    HZD_BEHIND		*behinds ;
    HZD_ZON		*zones ;
    u_int		n_link_zones ;
    u_char		*link_zone ;
    u_char		*route ;
#ifndef OLD
    u_int		*online_info ;
#endif
#ifdef ROUTE_COURSE_TEST
	int			courses ;
    u_short		*route_course[MAX_COURSE] ;	/* コース別テーブル */
#endif
    /*-----------------------------*/

    int			bound_min_x, bound_min_y, bound_min_z ;
    int			block_size_x, block_size_y, block_size_z ;
    u_char		div_x, div_y, div_z, pad ;
    u_short		n_blocks ;
    u_short		n_x_blocks ;
    HZD_BLOCK		*blocks ;
    HZD_DYNAMICS	*dynamics ;
} HZD_GRP ;
/* 
   link_zonesとは？ 

   ゾーンのなかで、グループ間を接続しているものを
   リンクゾーンといいます。
   配列link_zonesには、このリンクゾーンのリストが入っています。
   （今は何も入っていません。）
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
    u_short		vr_pat_offset[HZX_VR_CHARACTER_GROUP_MAX];
    u_short		vr_clr_offset[HZX_VR_CLEARING_GROUP_MAX];
    HZD_PAT		*patrols ;
    HZD_PTP		*points ;	
    HZD_CLE_AREA	*cle_areas ;
    HZD_CLE_ROOT	*cle_roots ;
    HZD_CLE_PTP		*cle_points ;

    /*------------------------------*/

    HZD_GRP		*groups ;
} HZD_DEF ;

/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/

#endif

