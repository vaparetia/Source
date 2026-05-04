/********************************************************************************/
/*	cypher.h								*/
/*	サイファ用のヘッダファイル群						*/
/*	2000/01/24 H.Satoyoshi							*/
/*	$Id: cypher.h,v 1.1.1.3 2002/11/19 11:48:15 Yoshizawa1 Exp $			*/
/********************************************************************************/

#ifndef __dev_cypher_h__
#define __dev_cypher_h__

int	CYPHER_UNIQ_ID = 0;

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define M_PI 3.14159265358979323846264338327950288419716939937510f
#define SET_COLOR_CYP(_r, _g, _b)\
{					\
     work->bonbori_color.vx = (_r);	\
     work->bonbori_color.vy = (_g);	\
     work->bonbori_color.vz = (_b);	\
     work->bonbori_color.vw = 128;	\
}

#define SET_COLOR_CYP_A(_r, _g, _b, _a)\
{					\
     work->bonbori_color.vx = (_r);	\
     work->bonbori_color.vy = (_g);	\
     work->bonbori_color.vz = (_b);	\
     work->bonbori_color.vw = (_a);	\
}




#define		CYP_CHECK_NUM	(7)
#define		CAMERA_PARTS (1)
#define		CYP_EYE_S_DEF (6000)
#define		CYP_DIR_Y_MAX	(512)
#define		DECAY_RATE		(0.990F)
#define		BRAKE_RATE		(0.980F)
#define		BRAKE_RATE2		(0.955F)
#define		CYP_SPEED_LOW_LIMIT (0.005F)
#define		CYP_ACCELE	(work->accel)	//加速度
#define		DEF_MAX_SPEED	(80) 	//最高速
#define		ROT_SPEED	(300)
#define		CYP_BODY_VITAL	(13)
#define		CYP_HEAD_VITAL	(3)
#define		OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
#define		UNIQ_LIST_MAX	10
#define		MAX_POINT_NUM	16

#define		GO1		0
#define		GO2		1
#define		BK1		2
#define		BK2		3

//サイファのモード
#define		CYP_NORMAL	0
#define		CYP_ALERT	2
#define		CYP_EMMA_ALERT	3

#define		CYP_GOHOME	5

#define		CYP_CALL	6
#define		CYP_WBK		7

#define		CYP_EVASION	9

#define		CYP_WAIT4RESURECT 42

#define		CYP_CHAFF	10

#define		CYP_DEMO	20

#define		CYP_DAMAGE	99

//ゲームステップ
#define		G1_STEP	work->G1_step
#define		G2_STEP	work->G2_step
#define		G3_STEP	work->G3_step
#define		G4_STEP	work->G4_step

//サイファのフラグ
#define		CYP_FLAG_NONE	0
#define		CYP_FLAG_FIXROT	1


//メッセージ
#define		CYP_ZIDOU_POINT_CALL 77
#define		CYP_BIKKURI 192
#define		CYP_KILL 42
#define		CYP_FORCE_NORMAL_MODE 43

#define	SV_DISCV_DELAY		(90*5)	//	捉えてから通報までの時間差
#define	SV_HMARK_DELAY		(60*5)	//	捉えてからビックリまで*/
#define	SV_ALERT_TIME		(120*5)	//	捉えてからビックリまで*/

#define		SUB_MOD_WARN	0
#define		SUB_MOD_ALERT	1

#define		CYP_MSG_SET_UNREAL 2
#define		CYP_MSG_UNSET_UNREAL 3
#define		CYP_MSG_ROUTE_WARP4ZIDOUST 4


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
	CYP_WAIT,CYP_ACTIVE
};


/********************************************************************************/
/*	extern      								*/
/********************************************************************************/
#ifdef DEBUG_MODE
extern void *NewEyeView( FMATRIX *, int , int , int , int , COMMANDER *, int*);
#endif

extern void *NewCypherPlasma( FMATRIX *body, float width, FVECTOR *color );
extern void* NewCypherLight( FMATRIX *mat, FVECTOR *offset, FVECTOR *color );
extern FVECTOR *EMA_CommandGetPosition();
extern void  *NewControl_Headmark2( FMATRIX *world, int *type, TARGET *trg, CONTROL *cntrl );
extern void GM_InitRadarControl( RADAR_CTRL *, FVECTOR *, int , int );
extern void *NewCypherExplosion( FVECTOR* center, float size, int flag );
void *NewCypherRisingSmoke( FVECTOR* pos, float size );
extern void CYP_HMK_Check(EYEPARAM *eye , FMATRIX *head, void* p);
extern int DEV_EnemyCheck(EYEPARAM *eye , HZX_GROUP_ID hzx_id, int *uniq_id_list );
extern int GM_GetHzxGroupID( int );

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/
/* ワーク */
typedef	struct {
    GV_ACT_EX	actor;
    int		com_uniq_id;
    int		uniq_id;
    int		name;
    OBJECT	body;
    FMATRIX	lights[2];
    CONTROL	control;
    ROUTENAVI	rnavi;		//ルート誘導
    TRGPOINT	trg;		//移動目標情報


    // ===============帰宅関連
    int		go_home_time;	//お家に帰る時間
    FVECTOR	home_pos[1];	//お家の場所
    int		go_home_flag;

    void*	smokep;
    void*	hmk_work_p;	//ヘッドマークワーク
    FVECTOR	bonbori_color;

    int		is_player_atack;

    int		high_root;	//高高度指定

    // *****プロック類
    int		brake_proc_id;	//破壊時プロック
    int		reach_proc_id;
//  char	reach_proc_route;
//	char	reach_proc_node;
    int	reach_proc_route;
    int	reach_proc_node;

    // ****ツイビ用ワーク
    short	tp_num;				//トレースポイントの数
    FVECTOR	trace_point[MAX_POINT_NUM];	//トレース用座標
    int		look_timer;			//きょろきょろタイマ
    char	loop_flag;			//ノードがループしているか？
    char	skip_flag;			//スキップフラグ

    int		discv_time;
    int		alert_time;

    // ****一度見た眠り・死に敵兵を保存しておくワーク
    int		uniq_id_list[UNIQ_LIST_MAX]; 
    int		route;		//ルート番号
    SVECTOR	look_dir;	//向き固定方向
    char	rot_flag;	//向き固定移動用フラグ	
    EYEPARAM	eye;

    int		call_uid;
    int		call_type;	//通報タイプ
    int		unreal;		//アンリアルフラグ

    /******ホーミングワーク******/
    HOMING_TRG	hom;
    SVECTOR	rot_body;	//胴体回転
    FVECTOR	camera;		//視界の開始点
    //    int		alert;
    short	rot_fin;	//羽回転
    u_char	mode;		//サイファの行動モード
    u_char	type;		//サイファのタイプ



    /******ルート上プレイヤ追跡用変数******/
    long64	distans[MAX_ROOT_NODE ];	//各ノードの距離
    int		still_se_num;
    int		move_se_num;
    int		yota_se_num;
    int		tugi_se_num;
    int		fin_counter;
    int		se_timer;
    int		face_y_buf;

    int		rot_vx_buf;
    int		rot_vy_buf;
    int		turn_se_num;
    int 	chaf_se_num;

    int		se_tick;

    /**************ターゲット***************/
    //ボディ(ドーナツ部分)のターゲット
    TARGET	b_trg[6];
    POWER_TARGET b_power;
    //頭(カメラ)のターゲット
    TARGET	h_trg;
    POWER_TARGET h_power;
    //ローターのターゲット
    TARGET	r_trg;
    //プロペラのターゲット
    TARGET	p_trg;



    FVECTOR	nowpos;		//現在
    FMATRIX	damage_shift;	//ダメージを受けた場所
    FMATRIX	damage_pos;	//ダメージを受けた場所
    FVECTOR	damage_force;	//ダメージのパワー
    SVECTOR	damage_rot;	//ダメージの方向

    float	max_speed; 	//最高速度
    float	max_speed_norm;	//最高速度
    float	max_speed_warn;	//最高速度
    float	accel;
    float	accel_norm;
    float	accel_alert;

    int		status;
    RADAR_CTRL	rctrl;		//レーダーコントロール

    FVECTOR	nigeru_vec;	//逃げる方向
    SVECTOR	nigeru_dir;

    int		kyodo_timer;	//いろいろな演出タイマ
    int		damage_timer;
    int		tuiraku_timer;
    int		act_end_time;	//死へのカウントダウン

    int		gun_count;
    FVECTOR	pc_pos[30];
    float	rotvy[30];

    int		zido_root[3];	//自動移動ルート保存

    int		chaff_time;

    int		ACTION_LAYER;

    int		serch_timer;

    char	zidouset;
    char	mokuhyo_flg;	//目標地点と状況を保持

    char	noreturn;		// 戻り
    char	cyp_call_flag;		// 呼び


    

    //    char	mode_bkup;	//チャフ用のモードバックアップ

    int		life;			//残り機数
    int		local_life;		//残り機数
    int		max_life;		//残り全体機数
    char	dmg_flag;
    char	fire_flag;
    char	eyeck_flag;	//敵発見フラグ
    char	emma_flag;	//エマを攻撃するかどうか

    /**** 里吉追加 ****/
    char		G1_step;	//ステップ処理 レイヤー１
    char		G2_step;	//ステップ処理 レイヤー２
    char		G3_step;	//ステップ処理 レイヤー３
    char		G4_step;	//ステップ処理 レイヤー４

#ifdef DEBUG_MODE 
    FMATRIX	eyeview;	//視界表示のためだけのマトリクス
    short	sp_flag;
#endif

} Work;


#define SE_COS_20	(0.9396926207859f)
#define SE_COS_30	(0.8660254037844f)
#define SE_COS_45	(0.7071067811865f)
#define SE_COS_60	(0.5f)
#define SE_COS_75	(0.2588190451025f)
#define SE_COS_80	(0.1736481776669f)
#define SE_COS_85	(0.08715574274766f)
#define SE_COS_90	(0.0f)

static VOLUMECURVE cyp_normal = {
    2,
    SE_COS_75,
    SE_COS_90,
    {6000, 30000, -1, -1},
    {0x3f, 0x20, 0, 0, 0}, 
    {5000, 24000, -1, -1},
    {0x3f, 0x20, 0, 0, 0}, 
    1.0f
};


static VOLUMECURVE cyp_ownview = {
    2,
    SE_COS_60,
    SE_COS_90,
    {0, 24000, -1, -1},
    {0x3f, 0x3f, 0, 0, 0}, 
    {0, 21000, -1, -1},
    {0x3f, 0x3f, 0, 0, 0}, 
    1.0f
};


static VOLCURVES cyp_curves = {
    &cyp_ownview,
    &cyp_normal,
    &cyp_normal,
    &cyp_normal
};



#endif
