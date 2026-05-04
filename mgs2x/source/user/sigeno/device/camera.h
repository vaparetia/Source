/********************************************************************************/
/*	structure      								*/
/********************************************************************************/


#ifndef __def_dev_camera_h__
#define __def_dev_camera_h__

#define	UNIQ_LIST_MAX	(10)	//ユニークIDの保存数


/*******/
/*sigeno 監視カメラ系 視界情報*/
typedef	struct{
	FVECTOR *eyepos ;	/*開始点*/
	FVECTOR *trgpos ;	/*目標点*/
	SVECTOR rot ;		/*視界方向*/
	SVECTOR range ;		/*視界範囲*/
	float length ; 		/*視界距離*/
	short	alert_time;	//警戒時間	
	short	discv_time;	//発見するまでの時間
	/*白びっくり発見用*/
	short	alert2 ;
	short	discv2 ;
	int		accident_uniq_id ;
	FVECTOR	accident_pos ;
	HZX_GROUP_ID	accident_hzx_id ;
/*首振りＳＥ用*/
	int		se_tick;

#ifdef DEBUG_MODE
	int debug_st;	/*デバッグ用*/
#endif
} EYEPARAM ;


typedef struct {
    GV_ACT_EX	actor ;
    OBJECT		body ;
    DG_DEF		*broken ;	/*破壊後モデル*/
    CONTROL		ctrl ;
    FMATRIX		lights[2] ;
    FVECTOR		camera_pos;	/*カメラ座標*/
    FVECTOR		eye_pos;	/*レンズ座標*/
    SVECTOR		rot;		/*土台方向*/
    int			map ;
    int			name ;
    int			alert ;
    int			headmark ;
    short 		sight_stat ; 	//視界のステータス
    short		max_dir ;	/*可動角度*/
    short		now_dir ;	/*土台からの相対角度*/
    short		mode ;
    
    int			center ;	/*首振り中心角*/
    EYEPARAM		eye ;
    RADAR_CTRL		rctrl ;
    
    u_short		wait_cnt ;
    /*****チャフ揺れ関係*****/
    u_short		chf_cnt ;
    
    SVECTOR		rnd_speed ;
    /*****ターゲット*****/
    TARGET		h_trg;
    TARGET		lens_trg;
    POWER_TARGET	h_power;
    POWER_TARGET	lens_power;
    /*****ボンボリ用*****/
    int		        rgba ;
    int		        b_mode ;
    FVECTOR		b_pos ;
    void	        *b_work ;
    /*****実行ブロック関係*****/
    GCL_ARGS		args ;
    int			buf[ 2 ] ;
    int		        exec ;	//破壊時実行関数

    int			scn_status ;

#ifdef DEBUG_MODE 
    FMATRIX		eyeview ; /*視界表示のためだけのマトリクス*/
#endif

    /*首振りＳＥ関係*/
    int			rot_vy_buf ;	//首振りチェック用ワーク
    int			se_cnt ;	//破壊時用SEカウンンタ


    /**** 里吉追加 ****/
    char		G1_step;	//ステップ処理 レイヤー１
    char		G2_step;	//ステップ処理 レイヤー２
    char		G3_step;	//ステップ処理 レイヤー３
    char		G4_step;	//ステップ処理 レイヤー４

    short	uniq_id_list[UNIQ_LIST_MAX]; //発見した死体や眠り兵を覚えておく

}Work;





Work work;

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

//ゲームステップＧ１
#define		MOD_NORMAL	0
#define		MOD_BROKEN	1
#define		MOD_CHAFF	3
#define		MOD_NORMAL3	4
#define		MOD_NORMAL4	5
#define		MOD_NORMAL5	6

//ゲームステップＧ２
#define		SUB_NORMAL	0
#define		SUB_WARN	1
#define		SUB_ALERT	2
#define		SUB_WARN2	4
#define		SUB_ALERT2	5
#define		SUB_DANGER	3

#define		G1_STEP	work->G1_step
#define		G2_STEP	work->G2_step
#define		G3_STEP	work->G3_step
#define		G4_STEP	work->G4_step


#endif
