/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

#ifndef __def_dev_camera_h__
#define __def_dev_camera_h__


#define SET_COLOR_CAM(_r, _g, _b, _a, _p)\
{					\
     (_p)->vx = (_r);	\
     (_p)->vy = (_g);	\
     (_p)->vz = (_b);	\
     (_p)->vw = (_a);	\
}



//	ユニークIDの保存数
#define	UNIQ_LIST_MAX	(10)

//	カメラタイプ
#define	NORMAL		0
#define	PLANT		1
#define	GUN		2		

//	壊れフラグ
#define		SV_BREAK_CRUSH		(1)
#define		SV_BREAK_VANISH		(2)

         /******シナリオ指定ステータス******/
#define SVC_SCN_BROKEN1 (0x01)

         /*****work.status*****/
#define	SV_BLIND	(0x01)	/*視界閉じ レーダーそのまま*/

#define	OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)

#define	SVC_EYE_S_DEF		(4000)	//デフォルトの視力
#define	DIR_X_MAX		(750)	//限界値
#define	DIR_Y_MAX		(2000)	//限界値


#define SVC_BROKEN_MDL		(2526173)	//破壊監視カメラモデルID
#define SVC_PL_BROKEN_MDL	(8359178)	//破壊プラントカメラモデルID
#define SVC_NORMAL_MDL		(121635)	//監視カメラモデルID
#define SVC_PLANT_MDL		(11426745)	//プラント監視カメラモデルID

#define B_SIZE			(80)		//動作ランプのサイズ
#define B_RISE			(40.0F)		//動作ランプの表示優先

//	カメラ部分のモデルナンバ
#define	CAMERA_PARTS		(1)	


#define	PC_CHECK_FLAG_IS_CHECK		0x01
#define	PC_CHECK_FLAG_W_BIKKURI		0x02
#define	PC_CHECK_FLAG_W_BIKKURI_END	0x04


#define	BIKKURI_AIM_BY_EWAPON	1


#define SE_COS_20	(0.9396926207859f)
#define SE_COS_30	(0.8660254037844f)
#define SE_COS_45	(0.7071067811865f)
#define SE_COS_60	(0.5f)
#define SE_COS_75	(0.2588190451025f)
#define SE_COS_80	(0.1736481776669f)
#define SE_COS_85	(0.08715574274766f)
#define SE_COS_90	(0.0f)

extern int BP_BASE_TICK();
#define	TURN_SPEED	(BP_BASE_TICK())

#define	SV_WAIT_TIME	(TIME_BASE*work->wait_time)	/*首振り終端で停止時間*/


static VOLUMECURVE svc_normal = {
    2,
    SE_COS_75,
    SE_COS_90,
    {3000, 15000, -1, -1},
    {0x3f, 0x20, 0, 0, 0}, 
    {2500, 13000, -1, -1},
    {0x3f, 0x20, 0, 0, 0}, 
    1.0f
};


static VOLUMECURVE svc_ownview = {
    2,
    SE_COS_60,
    SE_COS_90,
    {0, 12000, -1, -1},
    {0x3f, 0x3f, 0, 0, 0}, 
    {0, 10000, -1, -1},
    {0x3f, 0x3f, 0, 0, 0}, 
    1.0f
};


VOLCURVES svc_curves = {
    &svc_ownview,
    &svc_normal,
    &svc_normal,
    &svc_normal
};



/*駆動制御*/
enum{
	RIGHT_TURN ,
	LEFT_TURN ,
};

enum {
MSG_SIGHT_OFF = 1,
MSG_SIGHT_ON
};



extern void CYP_HMK_Check(EYEPARAM *eye , FMATRIX *head, void* p);



typedef struct _SV_WORK {
    GV_ACT_EX	actor ;
    OBJECT		body ;
    DG_DEF		*broken ;	/*破壊後モデル*/
    CONTROL		ctrl ;
    FMATRIX		lights[2] ;
    FMATRIX		mazzle_mat ;
    FVECTOR		camera_pos;	/*カメラ座標*/
    FVECTOR		eye_pos;	/*レンズ座標*/
    FVECTOR		radar_pos;	/*レンズ座標*/
    SVECTOR		rot;		/*土台方向*/
    void *		hmk_work_p;	/*びっくりマーク*/


    // =================コマンダー登録
    int			com_uniq_id;


    int			chaff_time;
    
    int			call_id;

    int			is_player_atack;

    // =================ＳＥ関連
    int			move_se_num;
    int			chaf_se_num;
    int			se_tick;

    // =================行動レイヤー
    int			ACTION_LAYER;
    int			SV_Break_Flag;
    int			discv_time;	// 通報までの時間
    int			alert_time;	// 警戒維持時間

    int			call_type;	//通報内容

    int			map ;
    int			name ;
    //    int			alert ;
    int 		sight_stat ; 	//視界のステータス
    int		max_dir ;	/*可動角度*/
    int		alert_max_dir ;	/*可動角度*/
    int		now_dir ;	/*土台からの相対角度*/
    int		mode ;
    int		wait_time;

    // ======================視野
    float	eye_len_norm;
    float	eye_len_alert;
    float	eye_len_warn;
    float	eye_len_avoid;
    SVECTOR	range;

    char		camera_type;
    
    int			center ;	/*首振り中心角*/
    EYEPARAM		eye ;
    RADAR_CTRL		rctrl ;		//レーダーコントロール
    
    u_short		wait_cnt ;
    /*****チャフ揺れ関係*****/
    u_short		chf_cnt ;
    
    SVECTOR		rnd_speed ;
    /*****ターゲット*****/
    TARGET		h_trg;
    TARGET		mag_trg;
    TARGET		bar_trg;
    TARGET		lens_trg;
    TARGET		pilor_trg;

    /*****ボンボリ用*****/
    FVECTOR	        rgba ;
    int		        b_mode ;
    void	        *b_work ;
    /*****実行ブロック関係*****/
    int		        exec ;	//破壊時実行関数

    int			gun_count ;
    int			scn_status ;
    int			fire_flag ;

    int			nikita_count;
    int			nikita_deadend;

#ifdef DEBUG_MODE 
    FMATRIX		eyeview ; /*視界表示のためだけのマトリクス*/
#endif

    /*首振りＳＥ関係*/
    int			rot_vy_buf ;	//首振りチェック用ワーク
    int			se_cnt ;	//破壊時用SEカウンンタ

    char		last_bikkuri;	

    /**** 里吉追加 ****/
    char		G1_step;	//ステップ処理 レイヤー１
    char		G2_step;	//ステップ処理 レイヤー２
    char		G3_step;	//ステップ処理 レイヤー３
    char		G4_step;	//ステップ処理 レイヤー４


}Work;





static Work work;

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

//ゲームステップＧ１
#define		MOD_NORMAL		0
#define		MOD_BROKEN		1
#define		MOD_ALERT		2
#define		MOD_CHAFF		3
#define		MOD_CALL		4
#define		MOD_NIKITA_SHOOT	5
#define		MOD_EVASION		6
#define		MOD_EMMA_ALERT		7
#define		MOD_WBK			8


#define		SUB_MOD_WARN	0
#define		SUB_MOD_ALERT	1


//ゲームステップＧ２
#define		SUB_NORMAL	0
#define		SUB_WARN	1
#define		SUB_ALERT	2
#define		SUB_WARN2	4
#define		SUB_ALERT2	5
#define		SUB_DANGER	3
#define		SUB_BIKKURI	6

#endif
