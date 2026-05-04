/********************************************************************************/
/*	har_claster.h								*/
/*	ハリアクラスター爆弾ワーク						*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_claster.h,v 1.1.1.3 2002/11/19 11:48:20 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

#ifndef __har_harclst_h__
#define __har_harclst_h__

#define	HAR_SIGNAL_CLASTER_MASK		(0xFFF0)
#define	HAR_SIGNAL_CLASTER		(0xAA10)

#define	ATTACK_TARG_NUM			(24)	//攻撃ターゲットの数

#define CLST_BOMB_NUM			(204)
#define CLST_SHELL_NUM			(8)
#define CLST_SHELL_TAIL			(2)


#define	CLST_BUNRETU_TIME		(50)
#define	CLST_END_TIME			(300)


#define	CLASTER_FIRED			(9)
#define	CLASTER_DROPED			(HAR_SIGNAL_FIRE)
#define	CLASTER_EQUIP			(HAR_SIGNAL_EQUIP)
#define	CLASTER_NONE			(HAR_SIGNAL_NONE)

#define	CLASTER_BOMED				(1)
#define	CLASTER_NOT_YET				(0)

//#define	CLASTER_GRAVITY		4.0		(36.0f) extream
#define	CLASTER_GRAVITY				(work->grav_speed)


typedef struct {
    GV_ACT_EX		actor ;
    void *		oya_work;
    OBJECT		body ;
    FVECTOR		drp_sp;
    int			map;

    int			sp_num;	//プレイヤーに直接飛ばす

    float		grav_speed;	//重力加速度


    DG_COMDL		*clst_warhead;	//クラスター爆弾弾頭 コモデル
    FVECTOR		c_bomb_speed[CLST_BOMB_NUM];	//弾頭スピード
    FVECTOR		shell_speed;			//ケーススピード

    FVECTOR		shift_pos;	//シフト量
    FVECTOR		oldpos;
    FVECTOR		c_shell_speed[CLST_SHELL_NUM + CLST_SHELL_TAIL];//ケーススピード
    SVECTOR		c_shell_rot[CLST_SHELL_NUM + CLST_SHELL_TAIL];	//ケース弾頭回転

    short		mis_mode;	//発射モード
    short		timer_child;	//子どもタイマ
    short		timer_parent;	//親タイマ
    short		hit_flag[CLST_BOMB_NUM];

}Cls_Work;

#endif

