/*
   g_struct.h
   構造体定義ファイル

   1999/07/07 M.Sonoyama
   $Id: g_struct.h,v 1.1.1.3 2002/11/19 11:41:50 Yoshizawa1 Exp $			

   METAL GEAR SOLID 2 project ( from 1999 )
*/

#ifndef _sprite_2d_h_
#include	<sprite_2d.h>
#endif


#ifndef _g_struct_h_
#define _g_struct_h_

#ifdef __cplusplus
#define class class_flag
#endif


/*----------------------------------------------------

   マップ情報構造体

-----------------------------------------------------*/

#define	GM_MAX_MAPS	HZX_MAX_GROUPS	/* libhzx.h */
#define	GM_MAP_ALL	(0x7fffffff)

typedef	struct	{
    u_int	name ;		/* シナリオからの名前 */
    u_int	id ;		/* マップ識別ＩＤ */
    u_int	dg_group ;	/* 表示グループＩＤ */
    u_int	hzx_group ;	/* 当たりグループＩＤ */
    LIT_DEF	*light ;	/* ライト */
} MAP ;

/*----------------------------------------------------------------*/

        /*
                表示モデル用 OBJECT 構造体
        */

typedef struct _object{
    DG_OBJS                 *objs ;
    MOTION_CONTROL  	    *m_ctrl ;
    FVECTOR                 *step ;
    float                   height ;
    FVECTOR                 slide ;
    DG_EVMOBJ               *evmobj ;
    int			    flag ;
    int			    map_name ;
} OBJECT ;

#define		OBJECT_MOTIONSTEP_THROUGH	0x00000001	/* モーション移動量無視 */

/*----------------------------------------------------------------

 CONTROL : 移動制御構造体

----------------------------------------------------------------*/

typedef ALIGN16_DECL(struct) _Control {
    /* 移動用 */
    FVECTOR		mov ;		/* 位置			*/ 
    FVECTOR		step ;		/* 移動量		*/

    SVECTOR		rot ;		/* 回転			*/
    SVECTOR		turn ;		/* 目標方向		*/

    int			interp ;	/* 回転補間時間		*/
    /* 当たりパラメータ */
    float		height ;	/* 地表面からの高さ	*/
    float		hzx_height ;	/* 壁検出高さ		*/
    float		hzx_base ;	/* 壁床検出基準高さ	*/

    short		r_sphere ;	/* 障害物反発エリア	*/
    short		s_sphere ;	/* 障害物検出エリア	*/
    u_short		skip_flag ;	/* 処理スキップフラグ 	*/
    /* メッセージ関連 */
    u_short		n_msg ;		/* 自分宛メッセージ数	*/
    GV_MSG		*msg ;		/* 自分宛メッセージ	*/
    u_int		name ;		/* キャラクタＩＤ	*/
#if 0	/* 廃止 */
    /* 直方体チェック用 */
    float		col_width ;	/* 直方体チェック用 	*/
    float		col_length ;	/* 直方体チェック用 	*/
#endif
	float		hzx_height2 ;	/* ダブルニアチェック用高さ */
	short		motion_num ;	/* レイヤー０モーション番号（RESET_HEIGHTに使用）*/
	short		reserved ;

    /* 当たりチェック結果格納用 */
    u_int		atrs[ 2 ];	/* 検出障害物アトリビュート */

    FVECTOR		vecs[ 2 ] ;	/* 検出障害物方向	*/
    HZX_SEG		*segs[ 2 ] ;	/* 検出障害物データ	*/
    HZX_FLR		*level[ 2 ] ;	/* 検出床／天井データ */
    float		levels[ 2 ] ;	/* 床／天井高さ		*/
    signed char		is_edge[ 2 ] ;  /* エッジ？ */
    signed char		grounded ;	/* フロア接触		*/
    signed char		n_touches ;	/* 検出カベ数（０～２）	*/
    u_int		seg_flag;	/* セグメント検出フラグ */

    /* マップ定義 */
    int				map ;		/* 現在いるマップＩＤ */
    HZX_GROUP_ID	hzx_id;		/* HZXのグループID */
    int				dg_group_id;	/* DGのグループID */
    int			addr ;		/* グループ内アドレス */

    FVECTOR		*addr_mov ;	/* アドレスチェック位置へのポインタ */

    u_int		flr_flag ;	/* 床検出フラグ */
    u_int		flr_atrs[ 2 ] ;	/* 床天井アトリビュート */

    u_int		hzx_check_type ;/* 検索する属性 */

    u_int		level_found ;	/* 床存在フラグ */
    FVECTOR		*root_offset ;	/* ルート位置へのシフト量 */
    HZX_ZON		*link_zone ;	/* 現在いるリンクゾーン */

    OBJECT		*object ;	/* オブジェクト構造体へのポインタ */
#ifdef DEBUG_MODE
    int			runtime ;
    int			count ;
    int			attribute ;
#else
	int			attribute ;
	int			padding[ 2 ] ;
#endif
    /* トラップイベント用 */
    HZX_EVT		evt ;
} CONTROL  ;

typedef ALIGN16_DECL(struct) _ControlNoEvt {
    /* 移動用 */
    FVECTOR		mov ;		/* 位置			*/ 
    FVECTOR		step ;		/* 移動量		*/
    SVECTOR		rot ;		/* 回転			*/
    SVECTOR		turn ;		/* 目標方向		*/
    int			interp ;	/* 回転補間時間		*/

    /* 当たりパラメータ */
    float		height ;	/* 地表面からの高さ	*/
    float		hzx_height ;	/* 壁検出高さ		*/
    float		hzx_base ;	/* 壁床検出基準高さ	*/
    short		r_sphere ;	/* 障害物反発エリア	*/
    short		s_sphere ;	/* 障害物検出エリア	*/
    u_short		skip_flag ;	/* 処理スキップフラグ 	*/

    /* メッセージ関連 */
    u_short		n_msg ;		/* 自分宛メッセージ数	*/
    GV_MSG		*msg ;		/* 自分宛メッセージ	*/
    u_int		name ;		/* キャラクタＩＤ	*/

#if 0
    /* 直方体チェック用（廃止） */
    float		col_width ;	/* 直方体チェック用 */
    float		col_length ;	/* 直方体チェック用 	*/
#endif
	float		hzx_height2 ;	/* ダブルニアチェック用高さ */
	short		motion_num ;	/* レイヤー０モーション番号（RESET_HEIGHTに使用）*/
	short		reserved ;

    /* 当たりチェック結果格納用 */
    u_int		atrs[ 2 ];	/* 検出障害物アトリビュート */
    FVECTOR		vecs[ 2 ] ;	/* 検出障害物方向	*/
    HZX_SEG		*segs[ 2 ] ;	/* 検出障害物データ	*/
    HZX_FLR		*level[ 2 ] ;	/* 検出床／天井データ */
    float		levels[ 2 ] ;	/* 床／天井高さ		*/
    signed char		is_edge[ 2 ] ;  /* エッジ？ */
    signed char		grounded ;	/* フロア接触		*/
    signed char		n_touches ;	/* 検出カベ数（０～２）	*/
    u_int		seg_flag;	/* セグメント検出フラグ */

    /* マップ定義 */
    int			map ;		/* 現在いるマップＩＤ */
    HZX_GROUP_ID	hzx_id;		/* HZXのグループID */
    int			dg_group_id;	/* DGのグループID */
    int			addr ;		/* グループ内アドレス */
    FVECTOR		*addr_mov ;	/* アドレスチェック位置へのポインタ */

    u_int		flr_flag ;	/* 床検出フラグ */
    u_int		flr_atrs[ 2 ] ;	/* 床天井アトリビュート */
    u_int		hzx_check_type ;/* 検索する属性 */

    u_int		level_found ;	/* 床存在フラグ */
    FVECTOR		*root_offset ;	/* ルート位置へのシフト量 */
    HZX_ZON		*link_zone ;	/* 現在いるリンクゾーン */
    OBJECT		*object ;	/* オブジェクト構造体へのポインタ */
#ifdef DEBUG_MODE
    int			runtime ;
    int			count ;
    int			attribute ;
#else
	int			attribute ;
	int			padding[ 2 ] ;
#endif
} CONTROL_NOEVT  ;

/* 処理フラグ */
#define	CTRL_SKIP_SEG_CHECK		(0x0001)
#define	CTRL_SKIP_FLR_CHECK		(0x0002)
#define	CTRL_HZX_SEG_HORIZON_CHECK	(0x0004) /* 壁チェックは水平方向チェック */
#define	CTRL_NEAR_CHECK2		(0x0008)  	 /* ニアチェック高さが２つ ( col_width ) */
#define	CTRL_SKIP_TRAP			(0x0010)
#define	CTRL_SKIP_MESSAGE		(0x0020)
#define	CTRL_COLLIDE_CHECK		(0x0040)
#define	CTRL_RESET_HZX_BASE		(0x0080)
#define	CTRL_SKIP_NEAR_CHECK		(0x0100)
#define	CTRL_SKIP_ONLINE_CHECK		(0x0400)
#define	CTRL_SKIP_CHANGE_MAP		(0x1000) /* マップ変更なし */
#define	CTRL_SKIP_GET_ADDRESS		(0x2000) /* ゾーン変更なし */
#define	CTRL_SKIP_HZX			(CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK | \
								 CTRL_SKIP_ONLINE_CHECK)
#define	CTRL_RESET_HEIGHT		(0x0200)

/* キャラ属性 */
enum {
	CTRL_ATR_NOTHING 		= 0x00000000,
	CTRL_ATR_ITEMCHECK 		= 0x00000001,	/* アイテムを取れる */
	CTRL_ATR_NOCHK_HZX		= 0x00000002,	/* 壁床チェック無し */
	CTRL_ATR_PITFALLCHECK	= 0x00000004,	/* 落とし穴系に反応する */

	CTRL_ATR_NPC 			= 0x00000010,	/* ＮＰＣ */
	CTRL_ATR_HANG_THROUGH 	= 0x00000020,	/* 首太い */

	CTRL_ATR_NEARONLINE		= 0x00001000,	/* ニアチェック後、オンラインではじく */

	CTRL_ATR_ONLINE_SEG		= 0x10000000,	/* オンラインチェック壁当たり */
	CTRL_ATR_ONLINE_FLR		= 0x20000000,	/* オンラインチェック床当たり */
} ;

/* 壁床格納用 */
typedef	ALIGN16_DECL(struct)	{
    HZX_SEG	segments[ 2 ] ;
    HZX_FLR	floors[ 2 ] ;
} GM_SEGMENTS  ;

/*----------------------------------------------------------------

 TARGET : キャラ当たり判定構造体
 
----------------------------------------------------------------*/

#define	TARGET_PARTS_IS_LINK_LIST
#define	MAX_TARGET_PARTS_LEVELS	(4)

/* ダメージ攻撃用 */
typedef	ALIGN16_DECL(struct) 	{
    FVECTOR	force ;			/* パワー力積 */
    u_short	type ;			/* パワー攻撃タイプ */
    short	vital ;			/* 耐久力 */
    short	damage ;		/* ダメージ値 */
    short	faint ;			/* 気絶値 */
    void	*next ;			/* リンク */
    void	*reserved ;
} POWER_TARGET  ;

/* つかみ攻撃用 */
typedef	ALIGN16_DECL(struct) _CAPTURE_TARGET {
    CONTROL			*ctrl ;	/* 自分のCONTROLへのポインタ */
    OBJECT			*body ; /* 自分のOBJECTへのポインタ */
    struct _CAPTURE_TARGET	*capture ; /* つかみ相手 or つかまれ相手 */
    u_int			flag ;	/* フラグ */
} CAPTURE_TARGET  ;

/* プッシュ攻撃用 */
typedef ALIGN16_DECL(struct)	{
    FVECTOR	force ;			/* プッシュ量 */
    float	depth ;			/* プッシュ比率（ 0.0 ～ 1.0 ）*/
    int		pad[ 3 ] ;
} PUSH_TARGET  ;

/* 設置物用 */
typedef ALIGN16_DECL(struct) 	{
    FMATRIX	*world ;		/* 設置位置 */
    u_int	free ;			/* 設置終了フラグ */
    int		pad[ 2 ] ;
} STICK_TARGET  ;

/* 子情報 */
typedef	struct _TARGET_PARTS {
    u_int	flag ;		/* フラグ */
    u_int	n_parts ;	/* パーツ数 */
    void	*parts ;	/* パーツターゲットへのポインタ */
    int		level ;
    struct _TARGET_PARTS	*next ;
} TARGET_PARTS ;

typedef ALIGN16_DECL(struct) _TARGET {
    /* リンク情報 */
    struct _TARGET	*next ;			/* 次のターゲット */
    /* タイプ情報 */
    u_int		class ;			/* 検出クラス */
    u_int		map ;			/* マップ */
    u_int		side ;			/* 敵味方区別 */

    /* 位置情報 */
    FMATRIX		world ; 		/* 中心位置 */
    FVECTOR		center ;		/* 中心位置 */
    FVECTOR		offset ;		/* 中心からのオフセット */
    FVECTOR		size ;			/* サイズ */
    /* class に ONLINE が設定されている時は、
       center, offset に それぞれ from, to が入る */
    /* 当たった場合、sizeに、当たった面の法線をいれよう */
    FVECTOR		hit ;			/* 交差位置（オンライン） */

    /* 攻撃手段 */
    long64		weapon_type ;		/* 攻撃武器 */
    long64		damaged ;		/* ダメージフラグ */

    /* パーツ情報 */
    TARGET_PARTS	*parts ;

    /* 各種ターゲットパラメータ */
    POWER_TARGET	*power ;
    CAPTURE_TARGET	*capture ;
    PUSH_TARGET		*push ;
    STICK_TARGET	*stick ;

    /* コールバック */
    void		( *callback )( struct _TARGET *, struct _TARGET *, void * ) ;
    void		*work ;

    u_int		name ;
} TARGET  ; 

typedef void	( *TARGET_CALLBACK )( TARGET *, TARGET *, void * ) ;

/* 敵味方 */
enum {
	NO_SIDE =	  0x0000,
    PLAYER_SIDE = 0x0001,
    ENEMY_SIDE  = 0x0002,
    BOTH_SIDE   = 0x0003,
} ;

/* ターゲット処理クラス */
enum {
    TARGET_POWER = 	0x00000001,	/* ダメージを受ける	*/
    TARGET_CAPTURE = 	0x00000002,	/* キャプチャされる	*/
    TARGET_PUSH = 	0x00000004,	/* プッシュされる	*/
    TARGET_STICK = 	0x00000008,	/* Ｃ４をつけられる	*/

    TARGET_SEEK = 	0x00000010,	/* シーク可能		*/
    TARGET_DOWN = 	0x00000020,	/* ダウン状態		*/
	TARGET_HAZARD = 0x00000040,	/* 壁ターゲット */
    TARGET_TOUCH = 	0x00000080,	/* 交差ダメージ		*/

    TARGET_DEAD = 	0x00000100,	/* ターゲットは死んでいる */
    TARGET_LOCKON = 	0x00000200,	/* ロックオンする */
    TARGET_GET_NORMAL = 0x00000400,	/* 当たった面の法線を求める */
    TARGET_HAS_NAME =	0x00000800,	/* 名前もちターゲット */

    TARGET_THROUGH =	0x00001000,	/* スルー（当たったと思わない） */
    TARGET_DIE = 	0x00002000,	/* 一発死に用（レーション使わない） */
    TARGET_ROTATE = 	0x00004000,	/* 回転ターゲット */
	TARGET_LOCKON_HIGH_PRIO = 0x00008000,	/* 優先的にロックオンされる */

    TARGET_ONLINE = 	0x00010000,	/* オンラインチェックする */
    TARGET_CHILD_ALWAYS = 0x00020000,	/* 攻撃側属性に関わらず子ターゲットもチェックする */
    TARGET_CHILD = 	0x00040000,	/* 子ターゲットもチェックする */
    TARGET_SKIP = 	0x00080000,	/* チェックをスキップする */

	TARGET_NO_CLAYMORE = 0x00100000,	/* クレイモアに反応しない */
	TARGET_LOCKON_AUTOSIZE = 0x00200000,	/* ロックオンサイトの大きさを
											   ターゲットの大きさから計算 */
	TARGET_HIT_SAMENAME =	 0x00400000,	/* 攻撃と防御が同じ名前の場合のみチェック */
	TARGET_NAME_IS_SE =		 0x00800000,	/* name にはヒットＳＥが入っている */

    TARGET_OFFENSE = 	0x01000000,	/* 攻撃属性ターゲット */
    TARGET_DEFENSE = 	0x02000000,	/* 防御属性ターゲット */
    TARGET_ONLINE_MIN = 0x04000000,	/* オンラインチェック（最近接検出） */
	TARGET_CALL_CALLBACK_THROUGH_HIT = 0x08000000,	/* THROUGHが当たったときでも
													   コールバックを呼ぶ（防御専用） */
    TARGET_CHECK_ONE = 	0x10000000,	/* 一つ当たったらチェック終了 */
    TARGET_CHILD_SKIP = 0x20000000,	/* 子ターゲットチェックスキップ */
	TARGET_DIRECT_ATTACK = 0x40000000,	/* 防御側指定の攻撃ターゲット */
} ;

#define	TARGET_CHECK_CLASS	(TARGET_POWER | TARGET_CAPTURE | TARGET_PUSH | \
							 TARGET_STICK | TARGET_SEEK | TARGET_TOUCH)
#define	TARGET_NO_LOCKON	(0)

/* パワータイプ */
enum {
    POWER_ONCE = 0,			/* 一撃のみで壊れる	*/
    POWER_DECREASE = 1,		/* 耐久力減衰で壊れる	*/
    POWER_THRESHOLD = 2,	/* 一定破壊力で壊れる	*/
    POWER_CONST = 3,		/* 壊れない		*/
    POWER_EXPLODE = 4,		/* 爆発力		*/
    POWER_CLASS = 0x7 
} ;

/* つかみ用フラグ */
enum {
    CAPTURE_FREE = 		0x00000001,	/* つかみ終わり */
    CAPTURE_BACK = 		0x00000002,	/* あおむけ死に */
    CAPTURE_FRONT = 	0x00000004,	/* うつぶせ死に */
    CAPTURE_HEAD = 		0x00000010,	/* つかみ頭側 */
    CAPTURE_FOOT = 		0x00000020,	/* つかみ足側 */
    CAPTURE_MOVE = 		0x00000100,	/* 運ばれている */

    CAPTURE_HANG = 		0x00001000,	/* 首絞め */
    CAPTURE_BREAK = 	0x00004000,	/* 首折り */
    CAPTURE_THROW = 	0x00008000,	/* なげ */
    CAPTURE_HOLDUP = 	0x00010000,	/* ホールドアップ */
    CAPTURE_ESCAPE = 	0x00020000,	/* くび締め脱出 */
	CAPTURE_LOCKER =	0x00040000,	/* 引きずられてロッカーへ */

	CAPTURE_C4EXIST =	0x00100000,	/* Ｃ４がついている */
} ;

/*---------------------------------------------------------------*/

/* ゲージ管理 */

typedef	ALIGN16_DECL(struct) _GM_GageSet {
    char	name[ 16 ] ;		/* ゲージ表題 */

    u_short	xs, ys, w, h ;		/* ゲージ位置・大きさ */
    short	value ;			/* ゲージ値 */
    short	max ;			/* 最大値 */
    short	min ;			/* 最小値 */
    u_short	delay ;			/* 減衰遅れ時間 */

    u_short	x, y ;			/* 表示座標 */
    short	level ;			/* 表示優先レベル */
    u_short	flag ;			/* フラグ */
	short	m9_value ;		/* Ｍ９耐久値 */
	short	m9_max ;		/* Ｍ９耐久値最大 */
	short	m9_min ;		/* Ｍ９耐久値最小 */
	u_short	m9_delay ;		/* Ｍ９減衰後れ時間 */

    u_short	m9_dec ;		/* 減衰ゲージ値 */
    u_short	m9_prev ;		/* 前回の値 */
    u_short	m9_count ;		/* 減衰カウンタ */
    u_short	reserved1 ;

//#ifdef PSX2 ///	まだない
    DG_MENU2_PRIM	*prim ;			/* プリミティブ */
//#endif	///
    struct _GM_GageSet	*next ;		/* リンクリスト */

    u_short	dec ;			/* 減衰ゲージ値 */
    u_short	prev ;			/* 前回の値 */
    u_short	count ;			/* 減衰カウンタ */
    u_short	reserved2 ;

    u_char	r[ 4 ], g[ 4 ], b[ 4 ] ;/* ゲージ色 */
    int		text_len ;		/* 表題のピクセル長 */

	u_char	m9col_left[ 3 ] ;
	u_char	m9col_right[ 3 ] ;
	u_char	m9delay_col_left[ 3 ] ;
	u_char	m9delay_col_right[ 3 ] ;
	u_char	pad[ 4 ] ;

} GM_GageSet  ;

enum {
    GM_GAGE_VISIBLE   = 0x0000,
    GM_GAGE_INVISIBLE = 0x0001,
	GM_GAGE_WARNING   = 0x0010,
	GM_GAGE_M9EXIST   =	0x0020,
} ;

enum {
    GM_GAGE_STATE_VISIBLE = 0,
    GM_GAGE_STATE_INVISIBLE,
    GM_GAGE_STATE_APPEAR,
    GM_GAGE_STATE_DISAPPEAR,
} ;

enum {
	GM_GAGE_LEVEL_PLAYER_LIFE 	= 0,
	GM_GAGE_LEVEL_PLAYER_GRIP 	= 2,
	GM_GAGE_LEVEL_PLAYER_O2 	= 3,
	GM_GAGE_LEVEL_NPC_LIFE 		= 6,
	GM_GAGE_LEVEL_NPC_O2 		= 8,
	GM_GAGE_LEVEL_ENEMY			= 10,
} ;

enum {
	GM_GAGE_COLOR_TYPE_PLAYER_LIFE	= 0,
	GM_GAGE_COLOR_TYPE_NPC_LIFE		= 1,
	GM_GAGE_COLOR_TYPE_ENEMY_LIFE	= 2,
	GM_GAGE_COLOR_TYPE_PLAYER_O2	= 3,
} ;

#define	GM_DEFAULT_GAGE_HEIGHT	(4)

/*----------------------------------------------------------------

 跳弾・弾痕管理
 
----------------------------------------------------------------*/

#define	GM_MAX_SPARK_FUNCS		(16)
#define	GM_MAX_SCAR_FUNCS		GM_MAX_SPARK_FUNCS

typedef	void	*( *GM_SPARK_FUNC )( FMATRIX *, FVECTOR *, int ) ;
typedef	void	*( *GM_SCAR_FUNC )( FMATRIX *, HZX_SEG *, HZX_FLR * ) ;

/*----------------------------------------------------------------*/
	/*
		ホーミングターゲット用 HOMING_TRG 構造体
		便利なキャラクターステータスとしても利用
		*/
typedef struct _HomingTrg{
	FMATRIX	*world ;	/* 対象マトリクス */
	OBJECT	*body ;		/* 対象ボディ */
	int		*map ;		/* マップ情報 */
	int		status ;	/* ステータス */
	CONTROL	*ctrl ;		/* 対象のコントロール */
	TARGET	*trg ;		/* 対象の親ターゲット */

	struct _HomingTrg	*before ;
	struct _HomingTrg	*next ;
} HOMING_TRG ;

#define	HOMING_SKIP		0x00000001	/* 検索スキップ */
#define	HOMING_ENEMY	0x00000002	/* 敵兵である */
#define	HOMING_YOU		0x00000004	/* 狙われているぜー（システムは関与しない） */
#define	HOMING_UNREAL	0x00000008	/* 非表示状態である */
#define	HOMING_DEAD		0x00000010	/* 死体である */
#define	HOMING_NPC		0x00000020	/* ＮＰＣである */
#define	HOMING_MECA		0x00000040	/* メカである（サイファー、カメラ） */

/* statusの上位4bitは、各々のステージに依存 */
#define HOMING_EMA_IN_SIGHT		0x10000000
/*天狗兵とNPCスネークのやりとり用*/
#define HOMING_TNG_FAINT		0x10000000	/*気絶or眠り*/

/*
	ホーミング用 HOMING 構造体
*/
typedef struct {
	/* ホーミングデータ */
	FVECTOR		*pos ;		/* 位置のアドレス */
	int			*dir ;		/* 方向のアドレス */
	int			*map ;		/* マップのアドレス */
	int			length ;	/* ホーミング距離 */
	int			range ;		/* ホーミング範囲 */
	HOMING_TRG	*hom_trg ;	/* 現在のホーミングターゲット */
} HOMING ;

#define	CENTER_HOMING	(-1)
#define	RIGHT_HOMING 	(0)
#define	LEFT_HOMING 	(1)

/* モーションアジャスト ボディパーツ */
#define	ADJ_PARTS_HEAD	0x01	/* 頭 */
#define	ADJ_PARTS_RARM	0x02	/* 右腕 */
#define	ADJ_PARTS_LARM	0x04	/* 左腕 */
#define	ADJ_PARTS_WEST	0x08	/* 腰 */
#define	ADJ_PARTS_BOWELS	0x10	/* 腹 */
#define	ADJ_PARTS_WEST_HALF	0x20	/* 腰半分 */

#define	ADJ_PARTS_Y_ONLY	0x80	/* Ｙ方向のみ回転 */

#define ADJ_PARTS_HOMING_XY	ADJ_PARTS_HEAD | ADJ_PARTS_RARM | ADJ_PARTS_LARM | ADJ_PARTS_WEST
#define	ADJ_PARTS_HOMING_X	ADJ_PARTS_HEAD | ADJ_PARTS_RARM | ADJ_PARTS_LARM


/*----------------------------------------------------------------
Navigation
----------------------------------------------------------------*/
typedef	struct	{
	CONTROL		*ctrl ;
	int			going_addr ;		/* 最終目標ゾーンアドレス	*/
	int			next_addr ;			/* 一時目標ゾーンアドレス	*/
	FVECTOR		next_zonepos ;		/* 一時目標位置		*/
	short		next_dir ;			/* 一時目標方向		*/
	short		routes ;			/* 目標までのゾーンの個数（ルート数）	*/
} NAVIGATE ;

typedef	struct	{
	FVECTOR		pos ;		/* 目標位置 */
	int			addr ;		/* 目標ゾーンアドレス	*/
} NAVITARGET ;


/*----------------------------------------------------------------
Zone Interrupt
----------------------------------------------------------------*/

/* ルート遮断 */
typedef	ALIGN16_DECL(struct)	_R_INTRPT {
	HZX_ZONE_ADD zone1 ;	/* from */
	HZX_ZONE_ADD zone2 ;	/* to */

	FVECTOR	pos ;			/* 場所(モーションの立ち位置の計算基準） */
	int		dir ;			/* 方向 */
	int		status ;			/* ステータス */
	short	kind ;				/* 種類（モーション判別用） */
	short	id ;				/* ユニークＩＤ */

    /* コールバック 返り値は ０：今は× １：ＯＫ */
    int		( *callback )( int motion_name, int motion_num, int order, void *ptr ) ;
    void	*work ;

    struct	_R_INTRPT	*next ;
} R_INTRPT  ;

typedef int	( *RINTRPT_CALLBACK )( int, int, int, void * ) ;
/* ステータス */
#define ROOT_INTRPT_CLOSE	0x00000001	/* 閉まっている */
#define ROOT_INTRPT_MOVE	0x00000002	/* 動いている */
#define ROOT_INTRPT_OPEN	0x00000004	/* 開いている */
#define ROOT_INTRPT_BREAK	0x00000008	/* 壊れている */
#define ROOT_INTRPT_AUTO	0x00000010	/* 自動で開閉 */
#define ROOT_INTRPT_SLIDE	0x00000020	/* スライド式 */
#define ROOT_INTRPT_HINGED	0x00000040	/* 開き戸式 */
#define ROOT_INTRPT_NOENE	0x00000080	/* 敵兵開けれない */
#define ROOT_INTRPT_JUMP	0x00000100	/* 天狗跳びで通れる*/
#define ROOT_INTRPT_TOILET	0x00000200	/* トイレ */
#define ROOT_INTRPT_NOKEY	0x00000400	/* カードキーを持っていないので開かない */
#define ROOT_INTRPT_NONPC	0x00000800	/* NPC(エマ、スネーク)は通れない */
#define ROOT_INTRPT_ELV		0x00001000	/* エレベータ */

/* オーダー */
enum {
	RINTRP_ORDER_OPEN,
	RINTRP_ORDER_CLOSE,
} ;


/* ゾーン遮断 */
typedef	ALIGN16_DECL(struct)	_Z_INTRPT {
	CONTROL	*ctrl ;
	HZX_ZONE_ADD zone ;			/* 遮断ゾーン:ctrlがNULLの場合参照 */

	int		status ;			/* ステータス */
	short	kind ;				/* 種類 */
	short	id ;				/* ユニークＩＤ */

    struct _Z_INTRPT	*next ;
} Z_INTRPT  ;

/* kind 種類 */
#define	ZONE_INTRPT_NONE	0x0000
#define	ZONE_INTRPT_PLAYER	0x0001	/* プレイヤー */
#define	ZONE_INTRPT_ENEMY	0x0002	/* 敵兵 */
#define	ZONE_INTRPT_CORP	0x0004	/* 死体 */
#define	ZONE_INTRPT_OBJ		0x0008	/* 物体 */

/*----------------------------------------------------------------
EneFind
----------------------------------------------------------------*/
typedef	struct _ENEFIND {
	FVECTOR		pos ;			/* 場所		*/
	int			zoneaddr ;		/* ゾーンアドレス	*/
	int			type ;			/* タイプ	*/
	u_short		id ;			/* ID */
	short		dir ;			/* 方向 */

    struct _ENEFIND	*next ;
} ENEFIND ;

typedef struct __ENEFINDLIST{
	ENEFIND	*start ;
	ENEFIND	*end ;
	u_short	num ;
	u_short	unique_id ;
	int		listtype ;
	u_short	id ;
	short	padd ;

	struct __ENEFINDLIST *next ;
} ENEFINDLIST	;

typedef struct {
	ENEFINDLIST	*start ;
	ENEFINDLIST	*end ;
	u_short	num ;
	u_short	unique_id ;
} ENEFINDLISTLIST	;

/* list type */
#define	EF_LIST_TYPE_ONE		0x00000001	/* 単独でENEFIND(GM_EneFindList) */
#define	EF_LIST_TYPE_LINK		0x00000002	/* 足跡、血跡等の連続した繋がりがあるもの */
#define	EF_LIST_TYPE_NO_FIND	0x00000004	/* 敵が発見しない */
#define	EF_LIST_TYPE_BLOOD		0x00000008	/* 血痕 */
#define	EF_LIST_TYPE_FOOT		0x00000010	/* 足跡 */

/* ゲット側 */
#define	EF_LSIT_TYPE_TRACE		0x10000000	/* 敵、追跡中 */

/* type */
/* セット側 */
#define	EF_TYPE_NO_FIND		0x00000001	/* 敵が発見しない */
#define	EF_TYPE_LV1			0x00000002	/* 発見Lv1 ちらっと見やる */
#define	EF_TYPE_LV2			0x00000004	/* 発見Lv2 調べに行って「？」 */
#define	EF_TYPE_LV3			0x00000008	/* 発見Lv3 調べに行って探索モード */
#define	EF_TYPE_ITEM		0x00000010	/* アイテム、見つけたら拾う */
#define	EF_TYPE_ADULT		0x00000020	/* 大人のアイテム */
#define	EF_TYPE_DARK_AREA	0x00000040	/* 場所が暗闇 */
#define	EF_TYPE_C4			0x00000080	/* C4 */
#define	EF_TYPE_FOOTSTAMP	0x00000100	/* 足跡 */
#define	EF_TYPE_CLAYMORE	0x00000200	/* クレイモア */
#define	EF_TYPE_INTRUDE		0x00000400	/* イントルードの位置 */
#define	EF_TYPE_LOCKER		0x00000800	/* ロッカーの位置 */
#define	EF_TYPE_MOVE		0x00001000	/* 動いた */

/* ゲット側 */
#define	EF_TYPE_FOUND		0x10000000	/* 敵、発見済 */
#define	EF_TYPE_USING		0x20000000	/* 敵、使用中 */
#define	EF_TYPE_GET			0x40000000	/* 敵、取った */

/*----------------------------------------------------------------

 RADAR : レーダー表示用構造体

----------------------------------------------------------------*/

typedef struct _radar_ctrl{
	struct _radar_ctrl *next;
	unsigned short flag;
	unsigned short dir;			/* 視線方向 */
	unsigned short angle;		/* 視野 */
	unsigned short col;			/* 視野描画色 */
	unsigned int map;
	float same_floor_rate;		/* 同一階判定用 */

	float range;				/* 視力 */
	float range_center;			/* 上下レンジ用の中心 */
	float range_zoom_rate;		/* 上下レンジ用の倍率 */
	FVECTOR *pos;				/* 位置へのポインタ */

	SPR_OBJ	* face ;	/* 2D描画 顔位置*/
	SPR_OBJ	* sight ;	/* 2D描画 顔方向表示*/

} RADAR_CTRL;

#define DEFAULT_SAME_FLOOR_RATE		( 1.0F / 0.3F )

// flag フィールドの値
enum {
	RADAR_VISIBLE	= 0x0001,		/* レーダー可視 */
	RADAR_SIGHT		= 0x0002,		/* 視界表示 */
	RADAR_RADIO		= 0x0004,		/* 無線中 */
	RADAR_NOFIX_SIGHT	= 0x0008,	/* 視界長さが有効 */
	/* 2001/04/26  Y.Kira */
	RADAR_OUTRANGE          = 0x0010,       /* 領域外に出たら方向表示 */
	RADAR_MINE		= 0x0100,		/* 地雷探知機にしか反応しない */
	RADAR_NODE		= 0x0200,		/* ノードにアクセスした時のみ表示 */
};

// color フィールドの値
enum {
	RADAR_COLOR_PLAYER,
	RADAR_COLOR_BLUE,
	RADAR_COLOR_RED,
	RADAR_COLOR_YELOW,
};

/* 操作関数 */

void GM_InitRadarControl( RADAR_CTRL *rctrl, FVECTOR *pos, int flag, int map );
						// map に-1を入れればすべてのマップで表示
void GM_FreeRadarControl( RADAR_CTRL *rctrl );

RADAR_CTRL *GM_RadarControlGetTop( void );

void GM_InitRadarSystem( void );

void GM_RadarSetCenter( FVECTOR *vec );
FVECTOR *GM_RadarGetCenter( void );

EXTERN_INLINE void GM_RadarSetFlag( RADAR_CTRL *rctrl, int flag )
{
/*2000.10.05 orに変更*/
//	rctrl->flag = flag;
	rctrl->flag |= flag;
}
EXTERN_INLINE void GM_RadarResetFlag( RADAR_CTRL *rctrl, int flag )
{
	rctrl->flag &= ~flag;
}

EXTERN_INLINE void GM_RadarSetSight( RADAR_CTRL *rctrl, int dir, int angle
									 , float len, int color )
{
	rctrl->dir = dir;
	rctrl->angle = angle;
	rctrl->col = color;
	rctrl->range = len;
}

EXTERN_INLINE void GM_RadarSetVRange( RADAR_CTRL *rctrl, float upper_range
									  , float lower_range )
{
	rctrl->range_zoom_rate = ( 2.0F / ( upper_range - lower_range ) );
	rctrl->range_center = ( -(upper_range+lower_range )/2.0F )
		* rctrl->range_zoom_rate;
}

EXTERN_INLINE void GM_RadarSetRangeRate( RADAR_CTRL *rctrl, float rate )
{
	rctrl->same_floor_rate = 1.0F / rate;
}

#define GM_RadarSetDir( _rc, _dir )	  ( (_rc)->dir = _dir )
#define GM_RadarSetAngle( _rc, _ang ) ( (_rc)->angle = _ang )
#define GM_RadarSetColor( _rc, _col ) ( (_rc)->col = _col )
#define GM_RadarSetRange( _rc, _len ) ( (_rc)->range = _len )

#define RADAR_VRANGE_UPPER	(1000.0F)
#define RADAR_VRANGE_LOWER  (-800.0F)

/*----------------------------------------------------------------*/
typedef struct {
    u_char  r ;              /* red   */
    u_char  g ;              /* green */
    u_char  b ;              /* blue  */
    u_char  a ;              /* alpha */
} UCVECTOR ; 

typedef struct {
    short  right ;    /* 右回転（０～２０４８）*/
    short  left ;     /* 左回転（０～２０４８）*/
    short  upper ;    /* 上向き（０～１０２４）*/
    short  lower ;    /* 下向き（０～１０２４）*/
} ROTLIMIT ; 

/*----------------------------------------------------------------*/

/* 振動アーカイブ構造体 */
#define	VAR_MAGIC				(10101010)
#define	VAR_VERSION				(2)
//#define	VAR_VERSION			(1)

enum {
	GM_VAR_FLAG_MLS	=	0x0000,
	GM_VAR_FLAG_VLS =	0x0001,
	GM_VAR_FLAG_INITIALIZE =	0x1000,
} ;

/* version 2 */
typedef	struct	{
	int			magic_le ;
	u_short		version_le ;
	u_short		flag_le ;
	int			n_elems ;
	u_short		*elemNo ;
	int			*name_id ;
	u_char		**elems ;
} VAR_DEF ;

#if 0
/* version 1 */
typedef	struct	{
	int			magic ;
	u_short		version ;
	u_short		flag ;
	int			n_elems ;
	u_short		*elemNo ;
	u_char		**elems ;
} VAR_DEF ;
#endif

typedef	struct	{
	GV_ACT_EX		*actor ;
	void			*vibact ;
	MOTION_CONTROL	*m_ctrl ;
	VAR_DEF			*def ;
	int				vctrl[ 2 ] ;
	int				active ;
	int				vib ;
} VAR_CONTROL ;

enum {
	VAR_FLAG_IDLE = 		0x0000,
	VAR_FLAG_PLAYING =		0x0001,
	VAR_FLAG_LOOP = 		0x0100,
	VAR_FLAG_FORCE = 		0x0200,
	VAR_FLAG_RESTART =		0x0800,
	VAR_FLAG_PLAY_STOP =	0x1000,
	VAR_FLAG_PLAY_SKIP =	0x2000,
} ;


/*----------------------------------------------------------------*/
/* サウンド */
/* 拡張カーブはこっちを使う */
#define	MAX_VOL_CURVE_DEV	(4)
typedef struct {
	int		dev ;
	float	in_cos ;		/* 有効角度 */
	float	out_cos ;		/* 減衰最大角 */

	int		in_dis[MAX_VOL_CURVE_DEV] ;		/* 内側距離(俯瞰時はプレイヤーの距離） */
	int		in_vol[MAX_VOL_CURVE_DEV+1] ;	/* 内側最小距離時音量 */
	int		out_dis[MAX_VOL_CURVE_DEV] ;	/* 外側計算最小距離 */
	int		out_vol[MAX_VOL_CURVE_DEV+1] ;	/* 外側最小距離時音量 */

	float	pan ;			/* パン係数 */
} VOLUMECURVE ;

typedef struct {
	VOLUMECURVE		*normal ;
	VOLUMECURVE		*ownview ;
	VOLUMECURVE		*demo ;
	VOLUMECURVE		*mic ;
} VOLCURVES ;

#if 0
/* 参考値 */
static	VOLUMECURVE normal_volcurves = {
	2,
	SE_COS_75,
	SE_COS_90,

	6000, 12000, -1, -1,
	0x3F, 0x18, 0x00, 0xff, 0xff,
	5000, 10000, -1, -1,
	0x3f, 0x18, 0x00, 0xff, 0xff,
	1.0f
} ;
static	VOLUMECURVE ownview_volcurves = {
	1,
	SE_COS_60,
	SE_COS_90,

	10000, -1, -1, -1,
	0x3F, 0x00, 0xff, 0xff, 0xff,
	9800, -1, -1, -1,
	0x3F, 0x00, 0xff, 0xff, 0xff,
	1.0f
} ;
static	VOLUMECURVE demo_volcurves = {
	2,
	SE_COS_75,
	SE_COS_90,

	6000, 12000, -1, -1,
	0x3F, 0x18, 0x00, 0xff, 0xff,
	5000, 10000, -1, -1,
	0x3f, 0x18, 0x00, 0xff, 0xff,
	1.0f
} ;
static	VOLUMECURVE mic_volcurves = {
	2,
	SE_COS_10,
	SE_COS_20,

	4000, 12000, -1, -1,
	0x3F, 0x3F, 0x00, 0xff, 0xff,
	0, 2000,  -1, -1,
	0x08, 0x00, 0x00, 0xff, 0xff,
	1.0f
} ;

static	VOLCURVES extra_volcurves = {
	&normal_volcurves,
	&ownview_volcurves,
	&demo_volcurves,
	&mic_volcurves
} ;
#endif
/*----------------------------------------------------------------*/

#ifdef __cplusplus
#undef class
#endif

#endif
