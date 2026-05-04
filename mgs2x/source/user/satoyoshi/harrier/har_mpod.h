/********************************************************************************/
/*	har_mpod.h								*/
/*	ハリアミサイルポッド弾頭						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: har_mpod.h,v 1.1.1.3 2002/11/19 11:48:23 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

#ifndef __har_harmpod_h__
#define __har_harmpod_h__

#define	HAR_SIGNAL_MPOD_MASK		(0xFFF0)
#define	HAR_SIGNAL_MPOD			(0xAA30)

#define	HAR_MPMOD_EQUIP			(0x00)
#define	HAR_MPMOD_FIRE			(0x01)
#define	HAR_MPMOD_FLYING		(0x02)
#define	HAR_MPMOD_BOMED			(0x03)
#define MPOD_MISSILE_NUM		(19)

#define	RP_TIMER			(work->mp_timer[loop])
#define	RP_MODE				(work->mp_flag[loop])

#define	MPOD_RIGHT				(-1)
#define	MPOD_LEFT				(1)



typedef struct {
    GV_ACT_EX	actor ;
    CONTROL	*oya_control;

    DG_COMDL	*mpod_warhead;

    OBJECT	mpod_body;

    TARGET		mp_tgt_body[MPOD_MISSILE_NUM];	//防御ターゲット
    TARGET		mp_target[MPOD_MISSILE_NUM];
    POWER_TARGET	mp_pow_tgt;

    FVECTOR	position[MPOD_MISSILE_NUM];
    FVECTOR	vector[MPOD_MISSILE_NUM];
    int		allcount;			//ミサイル発射タイミングカウント
    int		shootcount;			//ミサイル発射数カウント
    short	mis_mode;
    short	mp_timer[MPOD_MISSILE_NUM];
    char	mp_fire_flg[MPOD_MISSILE_NUM];
    char	mp_end_flg[MPOD_MISSILE_NUM];


    char	mp_flag[MPOD_MISSILE_NUM];
    char	which;
    char	break_flag;

}Mpod_Work;

#endif




