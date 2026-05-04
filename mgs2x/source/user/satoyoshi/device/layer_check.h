/********************************************************************************/
/*	structure      								*/
/********************************************************************************/


#ifndef __def_lay_check_h__
#define __def_lay_check_h__

#define	CHECK_PC_FLG( _flg )\
    (work->eye.PC_CK_FLG & (_flg))

#define	CHECK_ENE_FLG( _flg )\
    (work->eye.ENE_CK_FLG & (_flg))

//	通報の種類
#define		CALL_PC_MISTAKEN_DAMAGED	(0)
#define		CALL_ENE_DOWN_BODY		(1)


//	レイヤー情報
#define		EYE_LAYER_System0		(10)
#define		EYE_LAYER_System2		(12)
#define		EYE_LAYER_System4		(14)
#define		EYE_LAYER_Alert0		(30)
#define		EYE_LAYER_Alert2		(32)
#define		EYE_LAYER_Alert4		(34)

#define		EYE_LAYER_Check0		(50)
#define		EYE_LAYER_Check2		(52)
#define		EYE_LAYER_Check4		(54)


#define		EYE_LAYER_Low0			(60)
#define		EYE_LAYER_Low1			(61)
#define		EYE_LAYER_Low2			(62)

#define		EYE_LAYER_Surprise0		(70)

#define		EYE_LAYER_Normal		(9999)

#define		EYE_LAYER_BREAK			(EYE_LAYER_System0)
#define		EYE_LAYER_CHAFF			(EYE_LAYER_System2)

#define		EYE_LAYER_GOHOME		(EYE_LAYER_System4)


#define		EYE_LAYER_PC_ALERT		(EYE_LAYER_Alert0)
#define		EYE_LAYER_EMMA_ALERT		(EYE_LAYER_Alert2)
#define		EYE_LAYER_NIKITA_ALERT		(EYE_LAYER_Alert4)
#define		EYE_LAYER_EVASION		(EYE_LAYER_Check0)
#define		EYE_LAYER_CALL_ENEMY		(EYE_LAYER_Check2)

#define		EYE_LAYER_BIKKURI		(EYE_LAYER_Low0)







//	フラグ情報
#define  EYECK_PC_NOT_IN_SIGHT		(0x00000000)
#define  EYECK_PC_IN_SIGHT		(0x00000001)	//見える
#define  EYECK_PC_IN_UNIFORM		(0x00000002)	//変装のユニフォームを着ている
#define  EYECK_PC_AIMING_NEAR		(0x00000004)	//近くを狙っている
#define  EYECK_PC_ROLLING		(0x00000008)	//ローリング
#define  EYECK_PC_FIRE_ARMS		(0x00000010)	//武器発射
#define  EYECK_PC_PLONE			(0x00000020)	//ホフク
#define  EYECK_PC_NO_AVOID		(0x00000040)	//のーあぼいど
#define  EYECK_PC_EMMA_IN_SIGHT		(0x00000080)	//エマみっけ

#define  EYECK_NIKITA_IN_SIGHT		(0x80000000)	//ニキータ発見


#define  EYECK_ENE_HOLDUP		(0x00000001)	//ホールドアップ
#define  EYECK_ENE_MASUI		(0x00000002)	//麻酔ささり
#define  EYECK_ENE_DAMAGED		(0x00000004)	//ダメージ
#define  EYECK_ENE_FAINT		(0x00000008)	//気絶
#define  EYECK_ENE_CAPTURED		(0x00000010)	//捕マッチョ
#define  EYECK_ENE_SLEEPING		(0x00000020)	//眠り
#define  EYECK_ENE_OUCH			(0x00000040)	//ダメージ中
#define  EYECK_ENE_DEAD			(0x00000080)	//死亡
#define  EYECK_ENE_FALLING		(0x00000100)	//落下中


#define		G1_STEP	work->G1_step
#define		G2_STEP	work->G2_step
#define		G3_STEP	work->G3_step
#define		G4_STEP	work->G4_step


typedef	int	( *SV_FUNCLIST )( Work * ) ;


static inline void SV_G1_STEP(Work *p, short num){
    p->G1_step = num;
    p->G2_step = 0;
    p->G3_step = 0;
    p->G4_step = 0;
    p->discv_time = 0;
    p->alert_time = 0;
}

static inline void SV_G2_STEP(Work *p, short num){
    p->G2_step = num;
    p->G3_step = 0;
    p->G4_step = 0;
    p->discv_time = 0;
    p->alert_time = 0;
}

static inline void SV_G3_STEP(Work *p, short num){
    p->G3_step = num;
    p->G4_step = 0;
}

static inline void SV_G4_STEP(Work *p, short num){
    p->G4_step = num;
}


#endif


