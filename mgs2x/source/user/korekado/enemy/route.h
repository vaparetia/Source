/*
	route.h
	ルート関連ヘッダ

	2001/04/25 Y.Korekado
	$Id: route.h,v 1.1.1.3 2002/11/19 11:44:10 Yoshizawa1 Exp $
*/

#ifndef __ROUTE___
#define	__ROUTE___ 1


/*----- ------*/
#define	MAX_ROOT_NODE	64

#define	TOPADATA_A(a)	( (a)&0x1f )
#define	TOPADATA_T(a)	( ((a)&0xe0) >> 5 )
#define	TOPADATA_D(a)	( ((a)&0x300) >> 8 )
#define	TOPADATA_C(a)	( ((a)&0x1c00) >> 10 )

#define	PA_TIME_MOTION	-1		/* モーション終了まで */
#define	PA_TIME_SCENE 	29000	/* 進行タイミング */
#define	PA_TIME_CLE_START 	30000	/* クリアリング開始 */
#define	PA_TIME_CLE_END 30100	/* クリアリング終了 */
#define	PA_TIME_AVO_END 30200	/* 回避モード終了 */

/* 注！！short */
#define PA_CON_NONE				0x0000
#define PA_CON_AIM_FACE_MOVE	0x0001	/* 注視点を見つめながら移動する(顔ハーフ) */
#define PA_CON_AIM_GUN_MOVE		0x0002	/* 注視点を警戒しながら移動する */
#define PA_CON_AIM_FACE			0x0004	/* 注視点を見つめる(顔ハーフ) */
#define PA_CON_AIM_GUN			0x0008	/* 注視点を警戒する */
#define PA_CON_NO_VOICE			0x0010	/* SE無し */
#define PA_CON_PINPOINT			0x0020	/* ピンポイント移動 */
#define PA_CON_STAGE_ONE		0x0040	/* ステージ固有アクション */
#define PA_CON_STAGE_LOOP		0x0080	/* ステージ固有ループアクション */
#define PA_CON_FACE_ONLY_MOVE	0x0100	/* 注視点を見つめながら移動する(顔のみ) */
#define PA_CON_FACE_ONLY		0x0200	/* 注視点を見つめる(顔のみ) */
#define PA_CON_NEAR_WALL		0x0400	/* 通常より壁に接近する(r_sphere=100) */
#define PA_CON_COUTION_STAND	0x0800	/* 警戒モードでは立ちモーション */
#define PA_CON_COUTION_LOOK		0x1000	/* 警戒モードではきょろきょろモーション */
#define PA_CON_CALL_PROC		0x2000	/* プロックコール */
#define PA_CON_TALK_ONESELF		0x4000	/* 独り言 */

//ルートデモ人形 独自フラグ
#define PA_CON_NO_CHECK_DIS		0x00000040	/* 目的地までの距離判定なし（必ず到達したことにする）*/


	/* 巡回ルート */
#define	PTIME_MOTIONEND (-1)
#define PTIME_FLAGWAIT	(-2)
#define PTIME_MESWAIT	(-3)	/* メッセージ待ち(アクション終了) */
#define PTIME_VOICEEND	(-4)	/* 音声終了待ち */

typedef	struct	{
	FVECTOR		nodes[ MAX_ROOT_NODE ] ;		/* ポイントデータ */
	FVECTOR		aimnodes[ MAX_ROOT_NODE ] ;		/* 注視ポイントデータ */
	int			mapbit[ MAX_ROOT_NODE ] ;		/* マップビット */
	short		pa_action[ MAX_ROOT_NODE ] ;	/* アクションデータ */
	short		pa_time[ MAX_ROOT_NODE ] ;		/* 時間データ */
	short		pa_dir[ MAX_ROOT_NODE ] ;		/* 方向データ */
	short		pa_con[ MAX_ROOT_NODE ] ;		/* 条件 */
	int			pa_flag[ MAX_ROOT_NODE ] ;		/* フラグ */
	short		p_action ;
	short		p_acttime ;
	short		p_dir ;
	short		p_con ;
	int			p_actstatus ;					/* PACT_STATUS */

	short		c_route ;						/* 現在のルート番号 */
	short		next_route ;					/* 次のルート（メッセージで変更 ） */
	char		next_node ;						/* 次のポイント */
	char		n_nodes ;						/* ポイントの数 */
	char		chang_node ;					/* 変更ポイント指定 */
char 		pad ;
} ROUTENAVI ;


#endif
