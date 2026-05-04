/********************************************************************************/
/*	har_kasacka.h								*/
/*	ハリアカサッカワーク ヘッダ						*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: 	*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	extern									*/
/********************************************************************************/

#ifndef __har_harkasack_h__
#define __har_harkasack_h__

#ifdef KP_XBOX
#include "har_sdmng.h"
#endif


/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define		SET_SNK_IDEL		0
#define		SET_SNK_M4FIRE		1
#define		SET_SNK_M4END		2
#define		SET_SNK_GLFIRE		3
#define		SET_SNK_DAM		4
#define		SET_SNK_M4IDEL		5
#define		SET_SNK_ITEM		6
#define		SET_SNK_DAMING		7
#define		SET_SNK_ENABLE		8	//セット可能

#define		MODE_SNK_IDEL		0
#define		MODE_SNK_M4FIRE		1
#define		MODE_SNK_M4END		2
#define		MODE_SNK_GLFIRE		3
#define		MODE_SNK_DAM		4
#define		MODE_SNK_M4IDEL		5
#define		MODE_SNK_ITEM		6

#define		SNAKE_GRND_NUM		4


#define		CHECK_GRN_ACTV(_num)	(work->grn_speed[(_num)].vw == 1.0f)

#define		SET_GRN_ACTV(_num)	{\
    work->grn_speed[(_num)].vw = 1.0f;				\
    printf (" %x ", (_num));	\
    work->grn_sel[(_num)].objs->flag = (work->grn_sel[(_num)].objs->flag)&(~DG_FLAG_INVISIBLE);	\
    work->grn_sel[(_num)].objs->objs[0].flag = (work->grn_sel[(_num)].objs->objs[0].flag)&(~DG_FLAG_INVISIBLE);	\
    printf ("Set Ative!! %x \n", (_num));	\
}


#define		SET_GRN_NONACTV(_num)	{\
    work->grn_speed[(_num)].vw = 0.0f;				\
    printf (" %x ", (_num));	\
    work->grn_sel[(_num)].objs->flag = (work->grn_sel[(_num)].objs->flag)|(DG_FLAG_INVISIBLE);	\
    work->grn_sel[(_num)].objs->objs[0].flag = (work->grn_sel[(_num)].objs->objs[0].flag)|(DG_FLAG_INVISIBLE);	\
    printf ("Set Nonactive!! %x \n", (_num));	\
}





#define		KAC_N_STREAM	(10)

/* カサッカ 音関連 */
#define KACATKA_TARBIN	0x14
#define KACATKA_WING	0x13

// 300000

#define	KTAR_DIS  70000/100
#define	TAR_VOL	 63/100

extern VOLCURVES kac_se_curves;
static int har_gameover_check_ok();

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

//********ポイントワーク
typedef struct {

    FVECTOR	goal_pos;
    float	max_speed;
    short	rot_speed;
    short	max_node;

}KAS_POINT;


typedef struct {
    GV_ACT_EX		actor ;
    DG_OBJS		*obj ;
    GM_GageSet		gageset;	//ライフゲージ

    int			item_reserve_flag;

    // ==========================スタート移動
    int			start_move_flg;
    int			kas_mode;
    int			kas_timer;
    int			makasero_count; //連続して任せろいわない為のカウンタ


    float		rout_num;

    RADAR_CTRL		rctrl ;

    void *		oya_work;

    int			item_roty;	//落下するアイテムの回転速度

    int			die_proc_id;	//死にプロック
    int			event_proc_id;	//イベントプロック
    int			str_hdl_snk;	//スネーク用ストリームハンドラ
    int			str_hdl_ota;	//オタコン用ストリームハンドラ
    int			str_id[KAC_N_STREAM];
    int			ot_speek_time;	//オタコン喋りタイマ

    int			snk_atk_voice_num;	//スネーク攻撃声ナンバー

    int			timer;
    int			damage;
    int			damage_act_time;
    int			snk_mtk_time;

    int			snake_ikari;		//攻撃やダメージ関連
    int			snake_ikari_tame;
    int			snake_sizume_time;
    int			shoottime;

    int			rout_number;
    float		rout_time;

    int			m4time;
    float		ks_speed;

    ALIGN16_PRE FVECTOR ALIGN16_POST	aimtarget;	// スネークの狙う相手

    FVECTOR		snk_back;	// スネーク後ろ
    FVECTOR		snk_ue;		// スネーク真上

    int			name;

    // *********基本動作/オブジェクト表示用ワーク
    OBJECT		body ;		// カサッカ本体
    OBJECT		mrot ;		// メインローター
    OBJECT		trot ;		// テイルローター
    OBJECT		snake ;		// スネーク
    OBJECT		m4gl ;		// Ｍ４
    OBJECT		chair ;		// 椅子
    OBJECT		zabuton;	// ざぶとん
    OBJECT		grn_sel[SNAKE_GRND_NUM];// グレネード弾頭

    ALIGN16_PRE FVECTOR ALIGN16_POST	grn_speed[SNAKE_GRND_NUM];
    TARGET		grn_tgt[SNAKE_GRND_NUM];

    CONTROL		control ;
    ALIGN16_PRE FMATRIX ALIGN16_POST	lights[2] ;
    ALIGN16_PRE FMATRIX ALIGN16_POST	snk_lights[2] ;
    
    ALIGN16_PRE FVECTOR ALIGN16_POST	*homing_p;
    ALIGN16_PRE FVECTOR ALIGN16_POST	pre_adjust0;
    ALIGN16_PRE FVECTOR ALIGN16_POST	pre_adjust1;

    ALIGN16_PRE FVECTOR ALIGN16_POST	item_aim_pos;	// アイテムの落下目標位置

    ALIGN16_PRE SVECTOR ALIGN16_POST	rots[2];	//味付け

    int		m4gun_timer;
    int		set_snake;
    int		mode_snake;
    int		snake_motion;

    OBJECT		hako ;
    FVECTOR		hako_step ;
    FVECTOR		hako_shift;

    FMATRIX	item_box_mat;
    FMATRIX	drop_box_mat;
    DG_COMDL	*kak_itembox[30];
    FVECTOR	itemb_speed;

    TARGET		tgt_body[8];	//スネーク
    TARGET		kak_body[20];	//カサッカ
    POWER_TARGET	har_snk_ptgt;

    int			rout_num_max;

    short		azi_rot_now[3];
    short		azi_rot_old[3];
    short		azi_mov_now[3];
    short		azi_mov_old[3];
    int			azi_r_time[3];
    int			azi_r_time_end[3];
    int			azi_m_time[3];
    int			azi_m_time_end[3];

    ACRO_POINT_EX	kas_data[4];

    char		grn_hit[SNAKE_GRND_NUM];
    char		grn_flg[SNAKE_GRND_NUM];
    char		item_drop;
    char		is_bossrush;

#ifdef KP_XBOX
	int			hTarbin;
	int			hRoter;
	SD_3D_SNG_TRACK* pSngTrack;
#endif	
}Kas_Work;


#define			KMODE_ZYUNKAI_NORM	10
#define			KMODE_ZYUNKAI_STOP	256
#define			KMODE_ZYUNKAI_AIM	12
#define			KMODE_DROP_ITEM		20



#ifdef _MAIN_KASACKA_
static ACRO_POINT_EX	kas_data_far[4] = {
    {       {{ 35000.0f, 20000.0f,  -140000.0f,  1.0f},
	     {0.0f,    0.0f,   -200.0f,    1.0f}},
	    -1, 0, 400*5
    },
    {       {{0.0f, 20000.0f,  -175000.0f,  1.0f},
	     {0.0f,    0.0f,  200.0f,    1.0f}},
	    0.001f, 1, 400*5
    },
    {       {{-35000.0f, 20000.0f,  -140000.0f,  1.0f},
	     {0.0f,    0.0f,   200.0f,    1.0f}},
	    -1, 0, 400*5
    },
    {       {{0.0f, 20000.0f,  -105000.0f,  1.0f},
	     {200.0f,    0.0f,   0.0f,    1.0f}},
	    -1, 0, 400*5
    }
};

static ACRO_POINT_EX	kas_data_near[4] = {
    {       {{ 15000.0f, 4000.0f,  -151000.0f,  1.0f},
	     {0.0f,    0.0f,   -100.0f,    1.0f}},
	    -1, 0, 200*5
    },
    {       {{0.0f, 9000.0f,  -160000.0f,  1.0f},
	     {-100.0f,    0.0f,  0.0f,    1.0f}},
	    0.001f, 1, 200*5
    },
    {       {{-15000.0f, 4000.0f,  -149000.0f,  1.0f},
	     {0.0f,    0.0f,   100.0f,    1.0f}},
	    -1, 0, 200*5
    },
    {       {{0.0f, 9000.0f,  -140000.0f,  1.0f},
	     {100.0f,    0.0f,   0.0f,    1.0f}},
	    -1, 0, 200*5
    }
};
#endif /* _MAIN_KASACKA_*/


#endif
