/*
	holdene.h
	船倉兵

	2001/01/10 Y.Korekado
	$Id: holdene.h,v 1.1.1.3 2002/11/19 11:44:16 Yoshizawa1 Exp $
*/

#ifndef __USHOLD___
#define	__USHOLD___ 1

#define HOLD_MOT_NEW	(1)


/* イベント */
#define HLD_EVENT_LIGHTER	0x00000001	/* ライター落とした */
#define HLD_EVENT_DAMAGE	0x00000002	/* ダメージ発生 */
#define HLD_EVENT_TOUCH		0x00000004	/* たっち発生 */
#define HLD_EVENT_HANG		0x00000008	/* 首締め */
#define HLD_EVENT_THROW		0x00000010	/* 投げ */
#define HLD_EVENT_CAPTURE	0x00000020	/* つかみ */
#define HLD_EVENT_HOLDUP	0x00000040	/* ホールドアップ */


#define HLD_STATUS_ACTIVE			0x00000001	/* 活動中 */
#define HLD_STATUS_ACTIVE_END		0x00000002	/* 活動停止 */
#define HLD_STATUS_NOISE			0x00000004	/* 物音モード */
#define HLD_STATUS_DETECT			0x00000008	/* 発見モード */
#define HLD_STATUS_DOWN_DETECT		0x00000010	/* 下方向にいるプレイヤーを発見 */
#define HLD_STATUS_SCOT				0x00000020	/* スコットドルフ */
#define HLD_STATUS_CAMERAMN			0x00000040	/* カメラマン */
#define HLD_STATUS_NORMAL_DETECT	0x00000080	/* 普通にプレイヤーを発見 */
#define HLD_STATUS_EYE_CLOSE		0x00000100	/* 視力OFF */
#define HLD_STATUS_EAR_CLOSE		0x00000200	/* 聴力OFF */
#define HLD_STATUS_CAMERA1_DETECT	0x00000400	/* カメラ１発見 */
#define HLD_STATUS_CAMERA2_DETECT	0x00000800	/* カメラ２発見 */
#define HLD_STATUS_SCOT_DETECT		0x00001000	/* スコット発見 */
#define HLD_STATUS_DEATH			0x00002000	/* 死んだ */
#define HLD_STATUS_DETECT_DELAY		0x00004000	/* 発見ディレイ */
#define HLD_STATUS_KAITAIC4			0x00008000	/* 解体Ｃ４ついている */

/* 毎フレーム更新 status2 or event_id */
#define HLD_STATUS2_NOISE_S		0x00000001	/* 物音Ｓ聞こえた */
#define HLD_STATUS2_NOISE_M		0x00000002	/* 物音Ｍ聞こえた */
#define HLD_STATUS2_PROJECTOR	0x00000004	/* プロジェクターに映った */
#define HLD_STATUS2_DETECT		0x00000010	/* 発見 */
#define HLD_STATUS2_INDISTINCT	0x00000020	/*  */
#define HLD_STATUS2_LIGHTER		0x00000040	/* ライター落とした */
#define HLD_STATUS2_SLEEP		0x00000080	/* 麻酔刺さった */
#define HLD_STATUS2_EYE_CLOSE	0x00000100	/* 視力無し */
#define HLD_STATUS2_EAR_CLOSE	0x00000200	/* 聴力無し */
#define HLD_STATUS2_CAPTURE		0x00000400	/* 首締められた */
#define HLD_STATUS2_TOUCH		0x00000800	/* ぶつかった */
#define HLD_STATUS2_TARGET_SKIP	0x00001000	/* ターゲットスキップ */
#define HLD_STATUS2_DAMAGE		0x00002000	/* ダメージ食らった */
#define HLD_STATUS2_GUN_LEFT	0x00004000	/* 銃左手 */
#define HLD_STATUS2_GUN_RIGHT	0x00008000	/* 銃右手 */
#define HLD_STATUS2_GUN_SHOLDER	0x00010000	/* 銃肩 */
#define HLD_STATUS2_SLEEPING	0x00020000	/* 居眠り中 */
#define HLD_STATUS2_ACTIVE		0x00040000	/* 活動中 */
#define HLD_STATUS2_HOLD_UP		0x00080000	/* ホールドアップ */
#define HLD_STATUS2_C4_BOMB		0x00100000	/* Ｃ４破壊 */

enum {
	WP_POS_SHOLDER,
	WP_POS_RIGHT_HAND,
	WP_POS_LEFT_HAND,
} ;

typedef	struct	{
    TARGET			deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	capture ;	/* 捕まり情報 */
    POWER_TARGET	power ;	/* 防御属性 */

    TARGET			def_child1[ PTARGET_LEVEL1_NUM ] ;
    POWER_TARGET	power_child1[ PTARGET_LEVEL1_NUM ] ;	/* 防御属性 */
    TARGET			def_child2[ PTARGET_LEVEL2_NUM ] ;
    POWER_TARGET	power_child2[ PTARGET_LEVEL2_NUM ] ;	/* 防御属性 */
    TARGET			def_child3[ PTARGET_LEVEL3_NUM ] ;
    POWER_TARGET	power_child3[ PTARGET_LEVEL3_NUM ] ;	/* 防御属性 */

	FVECTOR			off_center ;

	u_char			faint ;
	u_char			life ;
	u_char			pad1 ;
	u_char			pad2 ;
} HOLDBODYPARAM ;

#define MAX_LOD	3	/* ＬＯＤ */
#define MODEL_OBJS	21
typedef	struct	{
	OBJECT		body[MAX_LOD] ;
	OBJECT_CHG	object_chg[MAX_LOD] ;
	FMATRIX		lights[2] ;
	CONTROL		control ;

	RADAR_CTRL	rctrl ;

	FVECTOR		abs_rots[ MODEL_OBJS ] ;

	CNCTOBJ		cnct ;
	OBJECT		weapon ;	/* 武器 */
	OBJECT		atama ;		/* 頭のアイテム */
	OBJECT		glass ;		/* 眼鏡 */
	FVECTOR		pos ;	/* 立ち位置、高さはフロア */
	SVECTOR		rot ;	/* 方向 */
	FVECTOR		mov ;	/* 腰の座標 */
	float		scale ;	/* スケール */
	int			name ;
	int			id ;
	int			status ;
	int			status2 ;
	short		c_obj ;	/* 表示モデル番号 */
	short		mdl_num ;	/* モデル番号 */
	short		mot_num ;	/* モーション番号 */
	short		items ;		/* アイテム */
	short		mot_type ;
	short		c_mot ;
	short		screen1_dir ;	/* スクリーン1方向 */
	short		screen2_dir ;	/* スクリーン2方向 */
	short		aim_dir ;		/* 注目する方向 */
	short		face_dir ;		/* 顔の向き */

	short		adj_turn_y ;	/* 顔アジャスト目標値 */
	short		adj_turn_x ;	/* 顔アジャスト目標値 */

	short		adj_y ;			/* 顔アジャスト値 */
	short		adj_x ;			/* 顔アジャスト値 */

	u_int		m_time ;	/* モーション再生時間  */
	float		interp_time ;	/* モーション補完時間 */
	short		interp_count ;
	short		interp_inc ;
	short		reverse_flag ;	/* 逆回し */
	short		reverse_padd ;
	void		*active ;
	void		*headmark ;

	short		think ;		/* 思考 */
	short		think2 ;	/* 思考2 */
	u_int		count ;		/* 思考カウント */
	u_int		p_time ;	/* ポーズ維持時間 */
	u_int		p_dis ;		/* プレイヤーまでの距離 */
	short		p_dir ;
	short		turn_y ;	/* 補完用 */

	HOLDBODYPARAM	bodyparam ;
	int			head_num ;	/* デバッグ用 */
	int			tmp_count ;

	ITEM_PROC	dogtag_item ;	/* ドッグタグアイテム */
	int			dogtag_id ;		/* ドッグタグＩＤ */
	int			dogtag_sw ;		/* ドッグタグsw */

	HOMING_TRG	hom ;
} HOLD_ENE ;


extern OBJECT *HLD_GetCameraObject( int num ) ;
extern void	*NewHoldCamera( int where, FVECTOR *pos, int dir, int name, int *flag, int num, int *status2 ) ;
extern void *NewRootEnemy( OBJECT *, OBJECT *, FVECTOR *, int, FVECTOR *, int, int *, int,
	 CONTROL *, HOLD_ENE *hold ) ;

#endif
