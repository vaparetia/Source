/********************************************************************************/
/*	har_missile.h								*/
/*	ハリアミサイルワーク							*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_missile.h,v 1.1.1.3 2002/11/19 11:48:22 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

#ifndef __har_harmiss_h__
#define __har_harmiss_h__

#define	HAR_SIGNAL_AMRAM_MASK		(0xFFF0)
#define	HAR_SIGNAL_AMRAM		(0xAA20)

typedef struct {
    GV_ACT_EX		actor ;
    void		*oya_work;	//親のワーク
    OBJECT		body;		//本体のオブジェクト
    FVECTOR		shift_pos;	//ミサイル装備位置
    FVECTOR		position;	//ミサイル現在位置
    FVECTOR		vector;		//ミサイル速度
    SVECTOR		rotation;	//ミサイル向き
    int			map;

    int			am_timer;	

    int			oya_speed;	//親のスピード
    TARGET		am_def_tgt;	//防御ターゲット
    TARGET		am_off_tgt;	//攻撃ターゲット
    TARGET		vul_target;	//攻撃ターゲット
    POWER_TARGET	am_pow_tgt;	//パワーターゲット
    char		mis_flag;	//ミサイルのフラグ
    short		mis_mode;	//ミサイルのモード
    short		timer;		//挙動用タイマ
    char		bk_flg;
}Mis_Work;

#endif

