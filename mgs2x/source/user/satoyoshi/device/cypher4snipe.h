/********************************************************************************/
/*	cypher.h								*/
/*	サイファ用のヘッダファイル群						*/
/*	2000/01/24 H.Satoyoshi							*/
/*	$Id: cypher4snipe.h,v 1.1.1.3 2002/11/19 11:48:16 Yoshizawa1 Exp $			*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

#define		CAMERA_PARTS (1)
#define		CYP_EYE_S_DEF (6000)
#define		CYP_DIR_Y_MAX	(512)
#define		DECAY_RATE		(0.990F)
#define		BRAKE_RATE		(0.980F)
#define		BRAKE_RATE2		(0.955F)
#define		CYP_SPEED_LOW_LIMIT (0.005F)
#define		CYP_ACCELE	(2.0F)	//加速度
#define		DEF_MAX_SPEED	(80) 	//最高速
#define		ROT_SPEED	(300)
#define		CYP_BODY_VITAL	(13)
#define		CYP_HEAD_VITAL	(3)
#define		OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
#define		UNIQ_LIST_MAX	10
#define		MAX_POINT_NUM	16

#define		MES_CYP_SNP_EYE_CONTROL	(33)

//サイファのモード
#define		CYP_NORMAL	0
#define		CYP_WARN	1
#define		CYP_ALERT	2
#define		CYP_W_WARN	3
#define		CYP_W_ALERT	4

//ゲームステップ
#define		G1_STEP	work->G1_step
#define		G2_STEP	work->G2_step
#define		G3_STEP	work->G3_step
#define		G4_STEP	work->G4_step

//サイファのフラグ
#define		CYP_FLAG_NONE	0
#define		CYP_FLAG_FIXROT	1

/*ポイントタイムの値がこの時、mesg受信まで待機*/
#define		P_TIME_WAIT	(-3)	

enum {
	MOVE_MODE,ACTION_MODE
};
enum {
	MSG_ROUTE_CHANGE = 1
};
enum {
	NORMAL_TYPE,GUN_TYPE
};
enum {
	CYP_ACTIVE,CYP_WAIT
};

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/
/* ワーク */
typedef	struct _Work {
    GV_ACT_EX	actor;
    int		name;
    OBJECT	body;
    FMATRIX	lights[2];
    CONTROL	control;
    ROUTENAVI	rnavi;		//ルート誘導
    TRGPOINT	trg;		//移動目標情報


    FVECTOR	damage_force;	//ダメージのパワー
    SVECTOR	damage_rot;	//ダメージの方向
    int		cyp_end_destroy;//壊れ２重呼び帽子

    int		cyp_eye_close;

    int		is_player_atack;

    short	is_attack;	//エマ攻撃中か？

    // ****ツイビ用ワーク
    short	tp_num;		//トレースポイントの数
    FVECTOR	trace_point[MAX_POINT_NUM];	//トレース用座標
    char	loop_flag;	//ノードがループしているか？

    // ****一度見た眠り・死に敵兵を保存しておくワーク
    short	uniq_id_list[UNIQ_LIST_MAX]; 

    int		route;		//ルート番号
    SVECTOR	look_dir;	//向き固定方向
    char	rot_flag;	//向き固定移動用フラグ	


    int		brake_proc_id;
    void*	hmk_work_p;
    EYEPARAM	eye;

    /******ホーミングワーク******/
    HOMING_TRG	hom;

    /******簡易物理演算用 変数******/
//    DEV_CTRL	b_ctrl;		//簡易CTRL   内部に傾き

    FVECTOR	camera;		//視界の開始点
    int		alert;

    int		kyodo_timer;

    SVECTOR	rot_body;	//胴体回転
    short	rot_fin;	//羽回転

    u_char	mode;		//サイファの行動モード
    u_char	type;		//サイファのタイプ

    /******ルート上プレイヤ追跡用変数******/
    long64	distans[MAX_ROOT_NODE ];//各ノードの距離

    
    u_short	still_se_num ;
    u_short	se_num;
    u_short	se_sw;
    int		face_y_buf;
    int		gun_count;	//射撃時間
    int		rest_count;	//休み時間
    int		guntime_fix;	
    int		guntime_rand;
    int		resttime_fix;	
    int		resttime_rand;

    // ======到達プロック
    int		reach_proc_id;
    int		reach_proc_route;
    int		reach_proc_node;

    /**************ターゲット***************/
    //ボディ(ドーナツ部分)のターゲット
    TARGET	b_trg[6];
    POWER_TARGET b_power;
    //頭(カメラ)のターゲット
    TARGET	h_trg;
    POWER_TARGET h_power;
    TARGET	r_trg;
    //プロペラのターゲット
    TARGET	p_trg;

    FVECTOR	nowpos;	//現在

    float	max_speed; 	//最高速度
    int		status;
    RADAR_CTRL	rctrl;		//レーダーコントロール


    char	fire_flag;

    /**** 里吉追加 ****/
    char		G1_step;	//ステップ処理 レイヤー１
    char		G2_step;	//ステップ処理 レイヤー２
    char		G3_step;	//ステップ処理 レイヤー３
    char		G4_step;	//ステップ処理 レイヤー４

    char		kaihi_flag;
    FVECTOR		kaihi_pos;

#ifdef DEBUG_MODE 
 FMATRIX	eyeview;	//視界表示のためだけのマトリクス
    short	sp_flag;
#endif

} Work ;








