/********************************************************************************/
/*	harrier.h								*/
/*	ハリアメインワーク ヘッダ						*/
/*	2001/02/23 H.Satoyoshi							*/
/*	$Id: harrier.h,v 1.1.1.3 2002/11/19 11:48:27 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <float.h>
#include "libmt.h"
#include "utl_dma.h"
#include "gameheader.h"
#include "libutl.h"

#if 1
#include "camera.h"
#endif

#include	"../util/sato_util.h"
#include	"../../okajima/etc/ok_util.h"
#include	"../../kira/radar/outrange.h"

#include "har_initfst.h"
#include "har_missile.h"
#include "har_claster.h"
#include "har_mpod.h"
#include "har_pirots.h"
#include "har_kasacka.h"

#ifdef KP_XBOX
#include "har_sdmng.h"
#endif



#if 0
#define	SATO_DEBUG
#endif

//#define MODE_E3

/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern float _FVecLen2( FVECTOR* );
extern int _FVecLen3( FVECTOR* );
extern int _FVecToRotXY( FVECTOR*, SVECTOR* );

#ifdef DEBUG_MODE
extern int HAR_DbgViewSnkStat;
extern int HAR_KacatcaMoveflg;
extern int HAR_ViewKacRout;
extern int HAR_DbgMode;
extern int HAR_KRD;
extern int HAR_Muteki;
extern int HAR_TGT_VIEW;
extern int HAR_RoutView;
extern int HAR_ItemDbg;
#endif

#ifndef __har_harrier_h__
#define __har_harrier_h__

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define ST_LEV_EASY   99
#define ST_LEV_NORMAL   GM_LEVEL_EASY
#define ST_LEV_UPNORM   GM_LEVEL_NORMAL

#define MUTEKI_TIME   (60*5)

#define M_PI 3.14159265358979323846264338327950288419716939937510f

#ifdef PSX2   // T. Morita
#define PERROR(...) { printf( __VA_ARGS__ ) ; return -1 ; }
#else
#define PERROR(...)  return (printf(__VA_ARGS__), -1)
#endif



#define MIGI_MISPOD		(33123)	// ターゲット名前 右ミサイルポッド
#define HIDA_MISPOD		(33125)	// ターゲット名前 左ミサイルポッド

#define	DMGD_POINT_NEW_FLAG	((0x01)<<31)

#define	SET_FLG_ON		0x100 
#define	SET_FLG_OFF		0x200 

#define	HAR_N_STREAM		17 //ストリーム音声の数

#define	KOWARE_POINT_NUM	12 //壊れ箇所の数

#define GET_IN	0
#define GET_OUT	1

// ===============ロケットミサイル浮上攻撃
#define	HAR_EVENT_RM_START	0		//開始
#define	HAR_EVENT_RM_FIRE	2		//発射
#define	HAR_EVENT_RM_END	4		//終了
// ===============クラスター爆弾
#define	HAR_EVENT_CLS_START	10		//開始
#define	HAR_EVENT_CLS_TYOKUSIN	11		//直進開始
#define	HAR_EVENT_CLS_BOMB	12		//発射
#define	HAR_EVENT_CLS_END	14		//終了
// ===============焼き攻撃
#define	HAR_EVENT_BURN_START	20		//開始
#define	HAR_EVENT_BURN_ATACK	22		//発射
#define	HAR_EVENT_BURN_END	24		//終了
// ===============ホヴァー機銃攻撃
#define	HAR_EVENT_VULC_START	30		//開始
#define	HAR_EVENT_VULC_FIRE	32		//攻撃開始
#define	HAR_EVENT_VULC_FIREEND	34		//攻撃終了
#define	HAR_EVENT_VULC_END	36		//終了
// ===============アムラーム攻撃
#define	HAR_EVENT_AMR_START	40		//開始
#define	HAR_EVENT_AMR_FIRE	42		//発射
#define	HAR_EVENT_AMR_END	44		//終了
// ===============アクロバット飛行
#define	HAR_EVENT_ACRO_START	50		//開始
#define	HAR_EVENT_ACRO_MID	52		//?
#define	HAR_EVENT_ACRO_END	54		//終了
// ===============バルカンつっこみ攻撃
#define	HAR_EVENT_GUNATCK_START	60		//開始
#define	HAR_EVENT_GUNATCK_KIRA	62		//キラリ
#define	HAR_EVENT_GUNATCK_FIRE	64		//機銃発射
#define	HAR_EVENT_GUNATCK_END	66		//終了
// ===============クラスターによる橋壊れ
#define	HAR_EVENT_BRIDGE_BRAKE	100		




// int			mispod_dmg_flg;	//ミサイルポッド用ダメージ
// ***ミサイルポッドの左右を表すマスク
#define POD_RIGHT_DAMAGE	0x0F
#define POD_LEFT_DAMAGE		0xF0


#define B_SIZE			(300)		//動作ランプのサイズ
#define B_RISE			(40.0F)		//動作ランプの表示優先
#define	HAR_SIGNAL_EQUIP	(0x0000)
#define	HAR_SIGNAL_FIRE		(0x0001)
#define	HAR_SIGNAL_FLY		(0x0002)
#define	HAR_SIGNAL_FIRENOW	(0x0005)

#define	HAR_SIGNAL_NONE		(0x000F)

#define W25A_STAGE_Z_SHIFT	(150250.0f)	//連絡橋中央を原点とする為のSHIFT値


//	フォグの値
#define FOG_NEAR 40000.0f
#define FOG_FAR  300000.0f


	//ミラーフラグ
#define	MILLOR_X_FLAG		(0x01)
#define	MILLOR_Z_FLAG		(0x02)



#define SE_COS_10	(0.9848077530122f)
#define SE_COS_15	(0.9659258262891f)
#define SE_COS_20	(0.9396926207859f)
#define SE_COS_30	(0.8660254037844f)
#define SE_COS_45	(0.7071067811865f)
#define SE_COS_60	(0.5f)
#define SE_COS_75	(0.2588190451025f)
#define SE_COS_80	(0.1736481776669f)
#define SE_COS_85	(0.08715574274766f)
#define SE_COS_90	(0.0f)


#define	PILLAR_MAX		(50)



#define P_MOVE_X_MIL ( (work->gun_attack_millor_flg&MILLOR_X_FLAG)? -1.0f : 1.0f )
#define P_MOVE_Z_MIL ( (work->gun_attack_millor_flg&MILLOR_Z_FLAG)? -1.0f : 1.0f )


//		**********************アクション関連
#define	HAR_MOVE_FRAMES		(3*60*60)
#define	HAR_ACTION_END		(65535)

//		**********************メッセージ関連
#define	HAR_MES_SET_POSITION	(0x0001)	//現在位置指定 (ワープ)
#define	HAR_MES_SET_DIRECTION	(0x0007)	//角度指定	
#define	HAR_MES_SET_GOAL	(0x0002)	//目標地点設定 (移動開始)
#define	HAR_MES_SET_SPEED	(0x0003)	//速度設定	
#define	HAR_MES_SET_ROTATE	(0x0004)	//速度設定	
#define	HAR_MES_MP_FIRE		(0x0005)	//ミサイルポッド発射	
#define	HAR_MES_GUN_FIRE	(0x0006)	//機銃発射
#define	HAR_MES_ACCEL		(0x0008)	//加速減速
#define	HAR_MES_AMRAM_FIRE	(0x0009)	//ミサイル発射
#define	HAR_MES_RELOAD		(0x000A)	//リロード
#define	HAR_MES_CLASTER		(0x000B)	//クラスター
#define	HAR_MES_MIS_FOLLOW	(0x000C)	//ミサイル追尾 ON/OFF

//		**********************ハリアのプログラム移動モード

#define	HAR_PMOVE_START		(0x0000)
#define	HAR_PMOVE_PLAYER	(0x0001)
#define	HAR_PMOVE_ESCAPE	(0x0002)
#define	HAR_PMOVE_BRIDGE	(0x0003)
#define	HAR_PMOVE_CHANGE	(0x0004)
#define	HAR_PMOVE_ACTION	(0x0005)
#define	HAR_PMOVE_MISSILE	(0x0006)
#define HAR_PMOVE_BOMB_START	(0x0007)
#define HAR_PMOVE_BOMB		(0x0008)
#define HAR_PMOVE_BOMB_END	(0x0009)
#define	HAR_PMOVE_MIS_START	(0x000A)


#define KAS_MODE		(work->kas_mode)

//		**********************ハリアのモード
#define HAR_MODE_L1		(work->har_mode_l1)

#define	H_MOD_L1_START		(0x0000)	//開始
#define	H_MOD_L1_PROGRAM_MOVE	(0x0001)	//機銃
#define H_MOD_L1_ACROBAT_MOVE	(0x0002)	//みせつけ
#define	H_MOD_L1_MISSILEPOD	(0x0003)	//ホヴァーロケット
#define H_MOD_L1_HOVER_GUN	(0x0004)	//ホヴァー機銃
#define	H_MOD_L1_BURNING	(0x0005)	//焼き
#define H_MOD_L1_CLASTER	(0x0006)	//クラスター
#define H_MOD_L1_AMRAM		(0x0007)	//アムラーム
#define H_MOD_L1_HOVER_MP	(0x0008)	//ホヴァーミサイルポッド
#define H_MOD_L1_STOP		(0x0099)	//停止


#define HAR_MODE_L2		(work->har_mode_l2)
#define	H_MOD_ATACK_UP		(0x0000)
//#define	H_MOD_CHANGE		(0x0001)
//#define	H_MOD_MPOD_END		(0x0002)

#define HAR_MODE_L3		(work->har_mode_l3)

#define	L3_ATCK_NORM_INIT		(0x0000)
#define	L3_ATCK_NORM_START		(0x0001)
#define	L3_ATCK_NORM_PLAYER_UP		(0x0002)
#define	L3_ATCK_NORM_PLAYER_DOWN	(0x0003)
#define	L3_ATCK_NORM_ESCAPE		(0x0004)
#define	L3_ATCK_NORM_BRIDGE		(0x0005)

#define HAR_MODE_L4		(work->har_mode_l4)


#define	H_MOD_L3_MISSILE	(0x0007)

#if 0
#define H_MOD_L3_BOMB_START	(0x0008)
#define H_MOD_L3_BOMB		(0x0009)
#define H_MOD_L3_BOMB_END	(0x000A)
#endif

#define	H_MOD_L3_MIS_START	(0x000B)



//		**********************ダメージモード (dmg_mode)
#define	HAR_NO_DAMAGE_MOD	(0x0000)
#define	HAR_DAMAGE_MOD_FAR	(0x0001)
#define	HAR_DAMAGE_MOD_NEAR	(0x0002)
#define	HAR_DAMAGE_MOD_MPODR	(0x0003)
#define	HAR_DAMAGE_MOD_MPODL	(0x0004)

//		**********************デバッグメニューのモード
#define	HAR_DEB_MENU_OPEN	(0x0001)
#define	HAR_DEB_RECORD_FLAG	(0x0002)
#define	HAR_DEB_PLAY_FLAG	(0x0004)
#define	HAR_DEB_CONTROL_FLAG	(0x0008)

//		**********************ハリアのモード (har_flag)
#define	HAR_DOUSA_FLAG_DEBUG	(0x0001)	//ターゲット表示
#define	HAR_DOUSA_FLAG_BOSSMD	(0x0002)	//ボスモード
#define	HAR_DOUSA_FLAG_FOLLOW	(0x0004)	//追尾フラグ



//		**********************ハリアの思考 (THINK)
#if 0
#define	HAR_THINK_NONE		(0x0000)
#define	HAR_THINK_BURN		(0x1000)

#define	HAR_THINK_ACTION	(0x2000)
#define	HAR_THINK_P_MOVE	(0x4000)

#define	HAR_THINK_WAIT		(0xF000)
#endif

//		**********************バルカンの状態フラグ (vul_fire_flg)
#define	GUN_FIRE_FLG_NOFIRE		(0x00)	//撃ってない
#define	GUN_FIRE_FLG_NORM		(0x01)	//通常撃ち
#define	GUN_FIRE_FLG_AIM		(0x02)	//狙い撃ち
#define	GUN_FIRE_FLG_DELAY		(0x04)	//狙い撃ち
#define	GUN_FIRE_FLG_NOHIT		(0x00)	// 攻撃判定無し
#define	GUN_FIRE_FLG_HIT		(0x10)	// 攻撃判定あり

#define	GUN_FIRE_FLG_HITCHECK		(0xF0)	// 攻撃判定あり
#define	GUN_FIRE_FLG_MODCHECK		(0x0F)	// 攻撃判定あり


#define SET_GUN_MODE(_flg1, _flg2)\
work->vul_fire_flg=(_flg1);\
work->vul_hit_flg=(_flg2)



//		**********************操作関連
#define RIGHT_ANALOGX	(work->act_work.right_x-128)
#define RIGHT_ANALOGY	(work->act_work.right_y-128)
#define LEFT_ANALOGX	(work->act_work.left_x-128)
#define LEFT_ANALOGY	(work->act_work.left_y-128)

#define PRESS_PAD_A	(work->act_work.buton&PAD_A)
#define PRESS_PAD_B	(work->act_work.buton&PAD_B)
#define PRESS_PAD_X	(work->act_work.buton&PAD_X)
#define PRESS_PAD_Y	(work->act_work.buton&PAD_Y)
#define PRESS_PAD_L1	(work->act_work.buton&PAD_L1)
#define PRESS_PAD_R1	(work->act_work.buton&PAD_R1)
#define PRESS_PAD_L2	(work->act_work.buton&PAD_L2)
#define PRESS_PAD_R2	(work->act_work.buton&PAD_R2)


#define	HAR_GUN_FIRE_TIME	(240)

#define MACRO_HARRIER_STOP      (work->control.step.vx = work->control.step.vy = work->control.step.vz = 0.0f)


#define	HAR_FLARE_NUM	(24)

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

//********パッドデータ
typedef struct {
    unsigned char	left_x;
    unsigned char	left_y;
    unsigned char	right_x;
    unsigned char	right_y;
    unsigned short	buton;
}PAD_DATA;

//********アクションデータ
typedef ALIGN16_DECL(struct) {
    FVECTOR	begin_pos;
    SVECTOR	begin_rot;
    PAD_DATA	p_data[HAR_MOVE_FRAMES];
}HAR_MOVE ;



//********ハリアワーク
typedef ALIGN16_DECL(struct) _harrier_work {
    GV_ACT_EX		actor ;
    DG_OBJS		*obj ;
    GM_GageSet		gageset;	//ライフゲージ



    ACRO_POINT_EX	acpoint;	//つなぎのポイント
    FVECTOR		Oldpos;		//本体の前の位置
    FVECTOR		Speed;		//本体のスピード

    float		zturn_sp;	//ミサイルホヴァーＺ回転
    int			pan_hokan;
    int			save_before_claster_act;


    //  =================ダメージ関連
    int			damage_num;	// ダメージ受けた回数
    int			dmg_mtk_count;	//ダメージタイマ
    int			dmg_mode;	//ハリアダメージモード
    int			damage_step;	//ダメージ挙動ステップ
    int			damage_rotvx;	//ダメージ揺れワーク
    int			damage_rotvy;	//ダメージ揺れワーク
    int			damage_rotvz;	//ダメージ揺れワーク
    int			damage_downy;	//ダメージ降下ワーク
    int			damaged_point;
    int			mispod_dmg_flg;	//ミサイルポッド用ダメージフラグ
    int			r_pod_dmg;	//右ポッドのダメージ
    int			l_pod_dmg;	//左ポッドのダメージ
    int			rpod_bomb_timer;	
    int			lpod_bomb_timer;	
    int			fire_damage_point;	//クラスター爆弾後の炎ダメージポイント
    DG_TEX_MOVEREPLACE* koware_p[KOWARE_POINT_NUM];	//壊れパケット
    FVECTOR		dam_smk_pos[5];	//やられ煙の場所	
    FVECTOR		dam_mpsmk_pos[2];	//やられ煙のミサイルポッド
    char		dam_mpsmk_flg[2];	//やられ煙のＭＰフラグ
    char		dam_smk_flg[5];	//やられ煙のフラグ
    char		missilepod_damage;
    char		camera_ensyutu_flag;
    char		GAME_OVER_FLAG;

    // *********排気煙攻撃ターゲット
    TARGET		noz_of_tgt[2];
    FVECTOR		burn_to_pos;	//焼き狙い場所
    FVECTOR		burn_smoke_pos;	//煙り発生位置
    FVECTOR		burn_dir[2];	//焼き方向
    char	       	burn_flag;	//焼きフラグ
    int			se_time;	//焼きSEタイマ
    int			burn_dm_time;	//焼きダメージタイマ
    int			azi_m_time[3];
    int			azi_m_time_end[3];
    short		azi_mov_now[3];
    short		azi_mov_old[3];


    // =================イベント関連
    int			event_id;

    //  =================行動選択関連
    int			mode_num;	//モード番号

    //  =================音・ストリーム関連
    int			str_id[HAR_N_STREAM];	//ストリームＩＤ
    int			str_hdl;		//ストリームハンドラ
    int			str_num;		//音声番号
    int			sol_speak_time;		//喋り時間
    int			har_se_time;		//SE時間
    int			se_num;			//SE番号
    float		distance;
    int			gun_hit_count;		//銃ヒット数

    //  *****************ハリア思考
    int			har_damage;	//ハリアダメージ



    
    short		har_flag;	//動作フラグ

    short		har_mode_l1;	//ハリア動作モード レイヤー１
    short		har_mode_l2;	//		   レイヤー２
    short		har_mode_l3;	//		   レイヤー３
    short		har_mode_l4;	//		   レイヤー４

    int			l1_timer;
    int			l2_timer;
    int			l3_timer;
    int			l4_timer;

    FVECTOR		her_fromp;	//エルミート補間元場所
    FVECTOR		her_froms;	//エルミート補間元スピード

    FVECTOR		r_cls_pos;	//クラスター位置
    FVECTOR		l_cls_pos;	//クラスター位置
    FVECTOR		r_amr_pos;	//アムラム右位置
    FVECTOR		l_amr_pos;	//アムラム左位置


    int			har_act_suki;	//アクションの後の隙
    
    OUTRANGE_CTRL	rctrl;		//レーダーコントロール
    
    short		aiming_flg;	//狙い用フラグ
    FVECTOR 		p_position;	//プレイヤ位置
    
    int			name;		//アクターの名前

    int			brake_proc_id;	//ステージ破壊プロック
    int			die_proc_id;	//死にプロック
    int			event_proc_id;	//イベントプロック
    int			item_proc_id;	//アイテムプロック
    int			headmark;	//びっくりマーク用
    
    // *********基本動作/オブジェクト表示用ワーク
    OBJECT		body ;
    CONTROL		control ;
    FMATRIX		lights[2] ;
    FVECTOR		rots[20];
    
    HOMING_TRG		hom ;		//ホーミング


    // *********フレア
    TARGET		target_flare[HAR_FLARE_NUM];
    FVECTOR		flare_pos[HAR_FLARE_NUM];
    FVECTOR		flare_speed[HAR_FLARE_NUM];
    int			flare_timer[HAR_FLARE_NUM];
    char		flare_shoot_num;

    // ******ターゲット設定
    TARGET		target_wheel_bar[2];//
    TARGET		target_b_nzl[2];//
    TARGET		target_body_all;//全体
    TARGET		target_body_centertop;//
    TARGET		target_body_center;//
    TARGET		target_body_side[4];//
    TARGET		target_body_pod[3];//
    TARGET		target_tale_wing[3];//
    TARGET		target_wepon_mpod[2];//
    TARGET		target_body_canopy;//
    TARGET		target_body_nose;//
    TARGET		target_body_tale;//
    TARGET		target_rwing[2];//
    TARGET		target_lwing[2];//
    POWER_TARGET	har_pow_tgt;
    
    //パーツ動作用
    float		intake_rot;
    float		noz_bk_rot;
    float		noz_fr_rot;
    float		r_flap_rot;
    float		l_flap_rot;
    float		l_elr_rot;
    float		r_elr_rot;
    float		lader_rot;
    float		cover_rot;
    float		talewing_rot;
    float		front_power;


    //===========================ホヴァー機銃攻撃
    float		hover_rot;	// ***ホヴァー機銃目標回転
    float		hov_nowroty;	// ***ホヴァー機銃本体回転
    FVECTOR		target_pos;	//狙い位置
    int			hov_now_point;
    int			hov_mv_timer;	//移動変化タイマー
    float		hov_accell;
    float		hov_atk_timer;



    // 前傾姿勢の角度
    short		zenkei_rot;
    



    // *********排気煙
    ALIGN16_PRE FMATRIX ALIGN16_POST	nozel[4] ;

    int		      	claster_num;
    // ステージ破壊のタイミングにも

    FVECTOR		claster_bomb_pos;
    int			claster_bomb_flag;
    


    //ライト用ワーク
    FVECTOR		body_light_pos1;
    FVECTOR		body_light_pos2;
    //筋雲エフェクト
    FVECTOR		wing_edge1;
    FVECTOR		wing_edge2;

    FVECTOR		pos_hanna;
    FVECTOR		pos_sippo;
    float		wsmoke_width;
    int			wing_smoke_flg;	//雲引きフラグ

    //びっくりマーク位置
    FMATRIX	headmark_pos;
    int		headmark_timer;
    
    // *********機銃
    FVECTOR	gun_pos;	//機銃の位置
    FVECTOR	gun_vec;	//機銃のベクトル

    FVECTOR	waterp_pos[PILLAR_MAX];	//水柱の場所
    short		use_pillar_num;

    short	gun_time;
    short	gun_time_first;
    char	vul_fire_flg;	//バルカンフラグ
    char	vul_hit_flg;	//バルカンフラグ

    // *********機銃射撃行動
    short	gun_attack_millor_flg;	//ミラーフラグ
    int		gun_attack_last_dist;
    float	gun_atk_dmg_flag;
    int		save_stinger_num;
    char	gun_attack_flag;

    
    //*************アクションデータ
    HAR_MOVE	har_move;
    HAR_MOVE	*act_move;
    //アクション再生用ワーク
    PAD_DATA	act_work;
    
    //************目標地点移動
    FVECTOR	goal_pos;
    float	max_speed;
    float	target_speed;
    short	rot_speed;
    short	z_turn_speed;
    short	accel_speed;
    
    //**********クォータニオン補間移動
    ALIGN16_PRE FVECTOR ALIGN16_POST	now_quaut;
    ALIGN16_PRE FMATRIX ALIGN16_POST	from_mat;
    int		count_frame;
    int		end_frame;

    //*********ルート移動
    int		rout_num_max;
    int		last_rout_num;

    float	dist_old;
    float	dist_new;

    //============カラー関連
    u_char	nozl_blur_col[4];	//	ノズルブラー	
    u_char	bonbori_col[4];		//	ボンボリ色
    u_char	temp_light_col[4];	//	Tempライトカラー
    u_char	nozl_atack_col[8];	//	ノズル攻撃色
    u_char	missile_burn_col[8];	//	ロケット
    u_char	missile_burn_col2[8];	//	ミサイルバーニア 

    int		hokan_timer;
    int		play_timer;
    int		play_flag;

#ifdef KP_XBOX	// Sound
	int		hTarbin;
	int		hEngine;
	int		hJet;
	SD_3D_SNG_TRACK* pSngTrack;
#endif	
	

#ifdef DEBUG_MODE
    //デバッグメニュー
    int		menu_flag;
    int		camera_flag;
    int		record_flag;
    //カメラ設定
    GM_CameraSet	*camera;
#endif
}Work ;

//yano 2002.02.22 実体を har_main.c に移しました。
extern FVECTOR	KAS_ITEM_DROP_POS;
extern short	KAS_ITEM_FLAG;
extern short	KAS_ITEM_NUM;
extern void	*GET_HAR_WORK;
extern void	*GET_KAK_WORK;
extern void	*GET_MPOD_WORK_R;
extern void	*GET_MPOD_WORK_L;
extern void	*GET_MISS_WORK_R;
extern void	*GET_MISS_WORK_L;
extern ACRO_POINT* LAST_ACROPOINT;
extern ACRO_POINT* pointdata[30];
extern ACRO_POINT_EX* pointexdat[30];
extern int HAR_Canp_Break;


#endif
