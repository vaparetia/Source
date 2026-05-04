//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tngdefthink.c
	天狗待機モード銃撃戦モードの思考

	2001/02/19 K.Sigeno
	$Id: tngdefthink.c,v 1.1.1.3 2002/11/19 11:49:54 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include "korekado/enemy/enemy.h"
#include	"libutl.h"


#include	"../attacker/at_thk.h"
//#include	"eve_a.h"
#include	"tng.h"
#include	"tng_enum.h"
#include	"korekado/enemy/motion.h"


#include	"../attacker/sig_conv.x"

#include	"../conv/conv_act.x"


/*extern*/
//extern void AT_ActReadyGun( ACTION	*, int  );
//extern void AT_ActShootGun( ACTION	*, int  );
//extern void AT_ActSquatReadyGun( ACTION	*, int  );

extern void AT_ActOneTimeTurn( ACTION	*, int  );
extern void AT_ActOneTimeTurn_Squat( ACTION	*, int  );
extern void AT_ActMoveRun( ACTION	*, int  );
extern void ActGrdThrHigh( ACTION	*, int  );
extern void ENE_ActNSight_ON( ACTION	*, int  );


extern void AT_ActOneTimeNoDmg( ACTION	*, int  );
extern float CheckGunPlOnline(ENETHINK *,FVECTOR * );

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SigZoneView( int , SVECTOR * ,float ) ; 
#endif

#define SIG_AT_SetMode(x,y) AT_SetMode( x,(ACTIONMODE) y ) 

void CheckDefPos( ENETHINK *) ;


#define PL_NO_SHOOT (PLAYER_DAMAGED|PLAYER_DOWNED|PLAYER_FORCE)
#define DEF_LEVEL_TIME	(AT_THK_RATE*300) 

#define EVE_TNG_SHOT_DELAY	DIRECT_TICK(12)		/*飛び出してから撃つまで*/
//#define EVE_A_SHOT_TIME		60
#define EVE_TNG_SHOT_TIME		DIRECT_TICK(48)		/*射撃時間*/
#define EVE_TNG_WAIT_TIME		DIRECT_TICK(90)		/*射撃終了後の隙*/
#define DEF_MOVE_SHOT_DELAY	DIRECT_TICK(60)		/*移動時の射撃開始*/
#define DEF_WAIT_TIME		DIRECT_TICK(60)		/*安地待避時間*/
#define DEF_ABS_HIT_TIME	DIRECT_TICK(6)		/*かならずあたる時間*/
#define DEF_GRD_THR_LIMIT	(10000.0F)

//#define NO_SHOT_LIMIT		(200.0F)
#define NO_SHOT_LIMIT		(100.0F)

//#define  DEF_POS_CHECK
void TNG_ActOneTime( act, time )
ACTION	*act ;
int		time ;
{


#if 0
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
#endif
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	else	SIG_AT_SetMode( act, ENE_ActStandStill ) ;

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}


	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
//		act->act_end = 1 ;
//		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
		SIG_AT_SetMode( act, TNG_ActOneTime ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}

void TNG_ActUnreal( act, time )
ACTION	*act ;
int		time ;
{

printf("TNG_ActUnreal\n");
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_UNREAL ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) {
//		act->ctrl->turn.vy = act->dir ;
	}else{
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}
	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}
#if 0
static void TNG_ActEndCheck( act, time )
ACTION	*act ;
int		time ;
{


	AT_SetActStatus( act, ACT_STATUS_STAND ) ;
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	else	SIG_AT_SetMode( act, ENE_ActStandStill ) ;

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}


	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}
#endif
static void TNG_ActEndCheck_Muteki( act, time )
ACTION	*act ;
int		time ;
{


#if 0
	AT_SetActStatus( act, ACT_STATUS_STAND | ACT_STATUS_ADJ_X ) ;
#else
	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_TARGET_SKIP ) ;
#endif
	if ( act->CheckDamage( act ) ) {
		return ;
	}
	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
	else	SIG_AT_SetMode( act, ENE_ActStandStill ) ;

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	}


	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}

	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
}


static void ReverseCheck(entk)
ENETHINK *entk;
{
	if(
	(entk->act->body->m_ctrl->flag & MT_FLAG_REVERSAL1)
	&&
	(entk->act->body->m_ctrl->flag & MT_FLAG_REVERSAL2)
	){
		entk->sw_gun |= SW_FLAG_SWITCH1 ;
	}else {
		entk->sw_gun &= ~SW_FLAG_SWITCH1 ;
	}
}
extern void SetMoveMode(ENETHINK *);
//extern int	CheckDirSub(int ,int);

extern int GetHomingDirY(ENETHINK *  , float );

/*prot*/
//#include "defthink.h"

/*アクションリスト*/

/**
SPリストの先頭にはno-actionを置いておく事
**/
enum {
	SP_NONE,
	SP_UNREAL ,
	SP_STAND_STILL,
	SP_SQUATHIDE,
	SP_ROLL_R,
	SP_ROLL_L,
	SP_PEEK_HIGH_START_R,
	SP_PEEK_HIGH_END_R,
	SP_PEEK_SQUAT_START_R,
	SP_PEEK_SQUAT_END_R,
	SP_PEEK_HIGH_START_L,
	SP_PEEK_HIGH_END_L,
	SP_PEEK_SQUAT_START_L,
	SP_PEEK_SQUAT_END_L,
	SP_APPROCH,
	SP_BACKWALK,
	SP_MOVE_RUN_R,
	SP_MOVE_RUN_L,
	SP_MOVE_RUN,
	SP_GRD_HIGH, 
	SP_TNG_JUMP,			/*ジャンプ*/
	SP_TNG_JUMP_LEVEL,		/*ジャンプ*/
	SP_SLIDING,
	SP_BACK_TURN,
	SP_SIDE_SOMER_R,
	SP_SIDE_SOMER_L,
	/*射撃アクションはここ以降に登録*/
	SHOT_ACT,/*if(pad > SHOT_ACT) で射撃*/
	SP_PEEK_HIGH_IDLE_R, 
	SP_PEEK_SQUAT_IDLE_R,
	SP_PEEK_HIGH_IDLE_L,
	SP_PEEK_SQUAT_IDLE_L,
	/*反転しない攻撃モーションはここ*/
	SP_READYGUN,
	SP_SQUATGUN,
	SP_SHOT_POS,
	SP_BLIND_IDLE_R ,
	SP_BLIND_IDLE_SQUAT_R ,
	SP_DATA_END
};

/*マジックナンバー 要注意*/
//#define WAIT_END	(0)
//#define IDLE_TIME	(1)
enum {
	WAIT_END = 0 ,
	IDLE_TIME ,
	MAGIC_END
};

#define BP_DUMMY_TIME (5*3)

#define START_TIME	(WAIT_END)
#define END_TIME	(WAIT_END)
#define ROLL_TIME	(WAIT_END)
//#define STILL_TIME	(AT_THK_RATE*5)
#define STILL_TIME	(AT_THK_RATE*1)
//#define SLIDE_TIME	(AT_THK_RATE*5)
#define SLIDE_TIME	(AT_THK_RATE*6)
#define PEEK_SLIDE_TIME	(AT_THK_RATE*3)


/*立ち右覗き*/
static u_char StandPeekR[16] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_PEEK_HIGH_START_R,START_TIME,
	SP_PEEK_HIGH_IDLE_R,IDLE_TIME,
	SP_PEEK_HIGH_END_R,END_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*立ち左覗き*/
static u_char StandPeekL[16] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_PEEK_HIGH_START_L,START_TIME,
	SP_PEEK_HIGH_IDLE_L,IDLE_TIME,
	SP_PEEK_HIGH_END_L,END_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*立ち右めくら*/
static u_char StandBlindR[8] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_BLIND_IDLE_R,IDLE_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*しゃがみ右めくら*/
static u_char SquatBlindR[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_BLIND_IDLE_SQUAT_R,IDLE_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*しゃがみ右覗き*/
static u_char SquatPeekR[12] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_PEEK_SQUAT_START_R,BP_DUMMY_TIME,
	SP_PEEK_SQUAT_IDLE_R,IDLE_TIME,
	SP_PEEK_SQUAT_END_R,END_TIME,
	SP_SQUATHIDE,0,
	SP_DATA_END,0
};


/*しゃがみ左覗き*/
static u_char SquatPeekL[16] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_PEEK_SQUAT_START_L,START_TIME,
	SP_PEEK_SQUAT_IDLE_L,IDLE_TIME,
	SP_PEEK_SQUAT_END_L,END_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};



#if 0
/*めくら立ち右*/
static u_char StandBlindR[8] = {
	SP_STAND_STILL,STILL_TIME,
	SP_BLIND_IDLE_R ,IDLE_TIME,
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
/*めくらしゃがみ右*/
static u_char SquatBlindR[8] = {
	SP_STAND_STILL,STILL_TIME,
	SP_BLIND_IDLE_R ,IDLE_TIME,
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*転がり右*/
static u_char RollOutR[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_ROLL_R,ROLL_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_DATA_END,0
};



/*転がり左*/
static u_char RollOutL[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_ROLL_L,ROLL_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_DATA_END,0
};



/*グレネード投げ右*/
static u_char GrdThrowR[16] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_GRD_HIGH,WAIT_END,
	SP_BACKWALK,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0,
	0,0
};



/*グレネード投げ左*/
static u_char GrdThrowL[16] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_GRD_HIGH,WAIT_END,
	SP_BACKWALK,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0,
	0,0
};


/*グレネード投げ立ち*/
#if 0
static u_char GrdThrowStand[8] = {
	SP_SQUATHIDE,STILL_TIME,
	SP_GRD_HIGH,WAIT_END,
	SP_SQUATHIDE,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*正面立ち撃ち*/
static u_char StandShot[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*スライド射撃右立ち*/
#if 0
static u_char SlideShotR[12] = {
	SP_STAND_STILL,STILL_TIME,
	SP_MOVE_RUN_R,SLIDE_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_MOVE_RUN_L,SLIDE_TIME,
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*スライド射撃右座り*/
static u_char SlideShotSquatR[12] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*スライド射撃左立ち*/
#if 0
static u_char SlideShotL[12] = {
	SP_STAND_STILL,STILL_TIME,
	SP_MOVE_RUN_L,SLIDE_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_MOVE_RUN_R,SLIDE_TIME,
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*スライド射撃左座り*/
static u_char SlideShotSquatL[12] = {
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_MOVE_RUN_L,BP_DUMMY_TIME,
	SP_SQUATGUN,IDLE_TIME,
	SP_MOVE_RUN_R,BP_DUMMY_TIME,
	SP_STAND_STILL,BP_DUMMY_TIME,
	SP_DATA_END,0
};


/*待機立ち*/
#if 0
static u_char StandWait[4] = {
	SP_STAND_STILL,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*待機しゃがみ*/
#if 0
static u_char SquatWait[4] = {
	SP_SQUATHIDE,STILL_TIME,
	SP_DATA_END,0
};
#endif
/*オブジェ破壊*/
static u_char ShotSubTrg[8] = {
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_READYGUN,IDLE_TIME,
	SP_SQUATHIDE,BP_DUMMY_TIME,
	SP_DATA_END,0
};



/*act_listの内容に準拠*/
enum {
	STAND_R,//0
	SQUAT_R,//1
	STAND_L,//2
	SQUAT_L,//3
	STAND_BACK1,//4
	SQUAT_WAIT,//5
	STAND_R_NARROW,//6
	STAND_L_NARROW,//7
	ACT_LIST_END //8
};
enum {
	DEF_LEVEL_1 = 0,
	DEF_LEVEL_2,
	DEF_LEVEL_3,
	DEF_LEVEL_4,
	DEF_LEVEL_GRD,
	DEF_LEVEL_SUBTRG,
	DEF_LEVEL_END
} ;
/**********************
０－４ プログラム でランダムチョイス
０ 右手覗き系（慎重）
１ 右手派手系（飛び出し含む 横が広い時に使用）
２ 左手覗き系（慎重）
３ 左手派手系（飛び出し含む 横が広い時に使用）
４ 立ち＆しゃがみ隠れ（しゃがまないと隠れられ無い場所 二階橋部分など）
*******************************
行動固定 必要になれば随時追加
５立ち撃ち
６しゃがみ撃ち
７刀兵
*****************************/
#define	STAND_AND_SQUAT_ACT (5)
#define	KATANA_ACT (7)
#define	CHASE_ACT	(8)
#define	HANG_ACT	(9)
#define	NO_ACTION_ACT	(10)	/*何もせず次の点へ*/

static u_char *act_list[ACT_LIST_END][DEF_LEVEL_END] =
{
	{StandPeekR,SlideShotSquatR,SquatPeekR,StandBlindR,GrdThrowR,ShotSubTrg},	/*右慎重*/
	{StandPeekR,SlideShotSquatR,RollOutR,RollOutL,GrdThrowR,ShotSubTrg},			/*右派手*/
	{StandPeekL,SlideShotSquatL,SquatPeekL,StandPeekL,GrdThrowL,ShotSubTrg},	/*左慎重*/
	{StandPeekL,SlideShotSquatL,RollOutL,RollOutL,GrdThrowL,ShotSubTrg},		/*左派手*/
	{StandShot,StandShot,StandShot,StandShot,GrdThrowL,ShotSubTrg},

	{StandShot,StandShot,StandShot,StandShot,GrdThrowL,ShotSubTrg},
	{SquatBlindR,SquatBlindR,SquatBlindR,SquatBlindR,SquatBlindR,ShotSubTrg},	/*しゃがみオンリー*/
};



void BP_SetConstants()
{

   StandPeekR[1]=STILL_TIME;
   StandPeekR[3]=PEEK_SLIDE_TIME;
   StandPeekR[11]=PEEK_SLIDE_TIME;
   StandPeekR[13]=STILL_TIME;

   StandPeekL[1]=STILL_TIME;
   StandPeekL[3]=PEEK_SLIDE_TIME;
   StandPeekL[11]=PEEK_SLIDE_TIME;
   StandPeekL[13]=STILL_TIME;

   StandBlindR[1]=STILL_TIME;
   StandBlindR[5]=STILL_TIME;

   SquatBlindR[1]=STILL_TIME;
   SquatBlindR[5]=STILL_TIME;


   SquatPeekR[1]=STILL_TIME;
   SquatPeekR[3]=START_TIME;
   SquatPeekR[9]=STILL_TIME;

   SquatPeekL[1]=STILL_TIME;
   SquatPeekL[3]=PEEK_SLIDE_TIME;
   SquatPeekL[11]=PEEK_SLIDE_TIME;
   SquatPeekL[13]=STILL_TIME;

   RollOutR[1] = STILL_TIME;

   RollOutL[1]=STILL_TIME;

   GrdThrowR[1]=STILL_TIME;
   GrdThrowR[3]=SLIDE_TIME;
   GrdThrowR[7]=SLIDE_TIME;
   GrdThrowR[9]=SLIDE_TIME;
   GrdThrowR[11]=STILL_TIME;

   GrdThrowL[1]=STILL_TIME;
   GrdThrowL[3]=SLIDE_TIME;
   GrdThrowL[7]=SLIDE_TIME;
   GrdThrowL[9]=SLIDE_TIME;
   GrdThrowL[11]=STILL_TIME;

   StandShot[1]=STILL_TIME;
   StandShot[5]=STILL_TIME;

   SlideShotSquatR[1]=STILL_TIME;
   SlideShotSquatR[3]=SLIDE_TIME;
   SlideShotSquatR[7]=SLIDE_TIME;
   SlideShotSquatR[9]=STILL_TIME;

   SlideShotSquatL[1]=STILL_TIME;
   SlideShotSquatL[3]=SLIDE_TIME;
   SlideShotSquatL[7]=SLIDE_TIME;
   SlideShotSquatL[9]=STILL_TIME;

   ShotSubTrg[1]=STILL_TIME;
   ShotSubTrg[5]=STILL_TIME;
}


#define CORRECT_SPEED	(1.0F)
void TNG_ActCorrectPos( act, time )
ACTION	*act ;
int		time ;
{

//	AT_SetActStatus( act, ACT_STATUS_STAND| ACT_STATUS_TARGET_SKIP ) ;
	AT_SetActStatus( act, ACT_STATUS_STAND ) ;

	if ( act->CheckDamage( act ) ) {
		return ;
	}

	if ( time == 0 ) {
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg), 
		ENE_BODYTRGSIZE_STAND ) ;
		AT_SetAction( act, 0, act->keep_mot, 0, MOTION_MASK_FULL,
		ACT_INTERP_DEF ) ;
//		Layer_Off(act,1);
	}
	if ( act->pad != act->keep_pad ) {
		if ( act->CheckPad( act ) ) {
			return ;
		}
	}
#if 0
	if ( GM_CheckObject_IsEnd( act->body, 0 ) ) {
		act->act_end = 1 ;
		SIG_AT_SetMode( act, ENE_ActStandStill ) ;
	}
#endif

	if ( act->dir >= 0 ) act->ctrl->turn.vy = act->dir ;
//	else	SIG_AT_SetMode( act, ENE_ActStandStill ) ;

	if ( act->tmp_dir >= 0 )
	{
		act->ctrl->step.vx +=
			CORRECT_SPEED * _RsinF( (int)act->tmp_dir ) ;
		act->ctrl->step.vz += 
			CORRECT_SPEED * _RcosF( (int)act->tmp_dir ) ;
	}
}



static	int	TngDefCheckPad( act )
ACTION	*act ;
{
extern void ActTngJumpStart( ACTION *, int ) ;
extern void ActTngJumpStartLevel( ACTION *, int ) ;

	switch ( act->pad ) {
		case SP_UNREAL :
			act->keep_mot = EM_tng_p90_nom_fire ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActUnreal );
			return 1 ;
		break ;
		case SP_READYGUN :
//			act->keep_mot = EM_kamae_gun_high ;
			act->keep_mot = EM_tng_p90_nom_fire ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
		case SP_SQUATGUN :
			act->keep_mot = EM_tng_p90_nom_squat_fire ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
		case SP_MOVE_RUN :
			act->keep_mot = EM_tng_p90_nom_run;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, AT_ActMoveRun);
			return 1 ;
		break ;
		case SP_ROLL_R :
//act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
//			act->keep_mot = EM_squat_roll_r ;
			act->keep_mot = EM_tng_p90_nom_roll_l ;
			act->keep_mot |=
				(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_ROLL_L :
//			act->keep_mot = EM_squat_roll_l ;
			act->keep_mot = EM_tng_p90_nom_roll_l ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_SLIDING :
printf("SP_SLIDING!!!!!!!");
//ASSERT(0);
			act->keep_mot = EM_tng_p90_nom_sliding ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_BACK_TURN :
			act->keep_mot = EM_tng_p90_nom_somersault ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_SIDE_SOMER_R :
			act->keep_mot = EM_tng_p90_nom_somersault2 ;
			act->keep_mot |=
			(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;

		case SP_SIDE_SOMER_L :
			act->keep_mot = EM_tng_p90_nom_somersault2 ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;

		case SP_PEEK_HIGH_START_R :
//			act->keep_mot = EM_gbs_ak_nom_peek_r_start ;
			act->keep_mot = EM_tng_p90_nom_peek_r_start ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_R :
			act->keep_mot = 
				EM_tng_p90_nom_peek_r_end ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_START_R :
//			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_start ;
			act->keep_mot = 
				EM_gbs_ak_nom_peek_r_squat_start ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_END_R :
//			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_end ;
			act->keep_mot = 
				EM_gbs_ak_nom_peek_r_squat_end ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_R : 
//			act->keep_mot = EM_gbs_ak_nom_peek_r_fire ;
			act->keep_mot = EM_tng_p90_nom_peek_r_fire ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_IDLE_R : 
//			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_fire ;
			act->keep_mot 
				= EM_gbs_ak_nom_peek_r_squat_fire ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;

		case SP_BLIND_IDLE_R : 
			act->keep_mot = EM_nom_fire_blind ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;

		case SP_BLIND_IDLE_SQUAT_R : 
			act->keep_mot = EM_nom_fire_blind_squat ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;

		case SP_PEEK_HIGH_START_L :
//			act->keep_mot = EM_gbs_ak_nom_peek_l_start ;
			act->keep_mot = EM_tng_p90_nom_peek_l_start ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_END_L :
//			act->keep_mot = EM_gbs_ak_nom_peek_l_end ;
			act->keep_mot = EM_tng_p90_nom_peek_l_end ;
//act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_START_L :
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_start ;
act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;

			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_END_L :
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_end ;
			act->keep_mot |=
				(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActEndCheck_Muteki  );
			return 1 ;
		break ;
		case SP_PEEK_HIGH_IDLE_L : 
//			act->keep_mot = EM_gbs_ak_nom_peek_l_fire ;
			act->keep_mot = EM_tng_p90_nom_peek_l_fire ;
//act->keep_mot |=(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
		case SP_PEEK_SQUAT_IDLE_L : 
			act->keep_mot = EM_gbs_ak_nom_peek_r_squat_fire ;
			act->keep_mot |=
				(ENE_MOT_FLAG_REVERSAL_U|ENE_MOT_FLAG_REVERSAL_D) ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn_Squat);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
		case SP_STAND_STILL :
//			act->keep_mot = EM_stand ;
			act->keep_mot = EM_tng_p90_nom_idle ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActCorrectPos);
			return 1 ;
		break ;
		case SP_SQUATHIDE :
			act->keep_mot = EM_squat_hide_idle ;
//			act->keep_mot = EM_tng_p90_nom_squat_fire ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActCorrectPos);
			return 1 ;
		break ;
 		case SP_BACKWALK :
//			act->keep_mot = EM_walk_b_atk;
			act->keep_mot = EM_tng_p90_nom_walk_b_atk ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
 		case SP_APPROCH :
//			act->keep_mot = EM_cle_walk ;
			act->keep_mot = EM_tng_p90_cle_walk ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
 		case SP_MOVE_RUN_R :
//			act->keep_mot = EM_slide_r_fire;
			act->keep_mot = EM_tng_p90_nom_slide_fire_r;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
 		case SP_MOVE_RUN_L :
//			act->keep_mot = EM_slide_l_fire;
			act->keep_mot = EM_tng_p90_nom_slide_fire_l;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
		case SP_SHOT_POS : /*指定ポイント向け*/
//			act->keep_mot = EM_kamae_gun_high ;
			act->keep_mot = EM_tng_p90_nom_fire ;
			act->keep_pad = act->pad ;
//			SIG_AT_SetMode( act, AT_ActOneTimeTurn);
			SIG_AT_SetMode( act, TNG_ActOneTime  );
			return 1 ;
		break ;
		/*GRD投げ*/
		case SP_GRD_HIGH :
			act->keep_mot = EM_gm_fire_high ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActGrdThrHigh ) ;
			return 1 ;
		break ;
		case SP_TNG_JUMP :
//			act->keep_mot = EM_tng_p90_nom_attack_near_shoutei ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActTngJumpStart ) ;
			return 1 ;
		case SP_TNG_JUMP_LEVEL :
//			act->keep_mot = EM_tng_p90_nom_attack_near_shoutei ;
			act->keep_pad = act->pad ;
			SIG_AT_SetMode( act, ActTngJumpStartLevel ) ;
			return 1 ;
	}
	return 0 ;
}


#if 0
ENE_ShootBullet( act, flag )
/* バレットフラグ */
#define	ENE_BULLET_NORMAL	0x00000000	/* ランダムずれ有り、狙いは頭、跳弾有り、軌跡有り、右手に銃 */
#define	ENE_BULLET_NORANDAM	0x00000001	/* ランダムずれなし */
#define	ENE_BULLET_NOWALL	0x00000002	/* 壁を突きぬける */
#define	ENE_BULLET_NOATTACK	0x00000004	/* 攻撃判定無し */
#define	ENE_BULLET_NOSPARK	0x00000010	/* 跳弾無し */
#define	ENE_BULLET_NOLINE	0x00000020	/* 軌跡無し */
#define	ENE_BULLET_LEFT		0x00000040	/* 左手に銃 */
#endif

#define	RIGHT_BULLET	(ENE_BULLET_NORMAL) 
#define	LEFT_BULLET		(ENE_BULLET_NORMAL|ENE_BULLET_LEFT)


/**  火器管制システム **/
#define SHOT_MODE_AIM	(0x01)
#define SHOT_MODE_NO_TRG	(0x02)
static void Def_Shot( entk ,mode )
ENETHINK	*entk ;
int mode ; /*0 銃の向き 1aim_pos方向 */
{
	FVECTOR *ShotPos ;
	int bullet_flag = 0 ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;


	
	if((entk->status2 & ENE_STATUS2_AIM_GUNSHOOT )
	&&(mode&SHOT_MODE_AIM)&&(!( GM_PlayerStatus & PL_NO_SHOOT))
	)
	{
		ShotPos = &entk->act->aim_pos ;
	}else {
		ShotPos = NULL ;
	}
	if( entk->sw_gun & SW_FLAG_SWITCH1 )
	{
	/*左利き*/
		bullet_flag = LEFT_BULLET ;
	}else {
		bullet_flag = RIGHT_BULLET ;
	}
/*攻撃判定無し弾丸*/
	if(mode & SHOT_MODE_NO_TRG){
		bullet_flag |= (ENE_BULLET_NOATTACK) ;
	}
	if(mode & SHOT_MODE_AIM){
		bullet_flag |= ENE_BULLET_NORANDAM ;
	}
	if(tng_com->now_hang_num == 0){
		ENE_ShootBullet( entk->act, bullet_flag,ShotPos ) ;
	}
}






static void SetAimPosEyei(ENETHINK *entk)
{
//	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->status2 |= ENE_STATUS2_AIM_GUN ;
	entk->act->aim_pos = *entk->pl_eyei.pos ;
}


void CheckAttackTarget(ENETHINK *entk){
//	int check=0 ;
	TNG_COM		*tng_com ;
	ENTK_TENG_A *eve_a ;
	float dis ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	if(tng_com->subtrg == NULL ) return ;

	if(eve_a->status & EVE_TNG_HANG){
		/*首絞めさんはＮＰＣのみ目標*/
		ENE_SetEyeInfo( &(entk->pl_eyei), &tng_com->subtrg->mov, 
			&tng_com->subtrg->addr, NULL, &tng_com->subtrg->map ) ;
		return ;
	}
	if(
	(tng_com->subtrg->mov.vy< ( tng_com->base_floor + TNG_PL_WAIST_H))
	||(tng_com->now_hang_num != 0)
	){
		ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
			&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
		return ;
	}

//	if(entk->count3 ==0){
	if(1){
//printf("TNG DEF THINK LINE %d\n",__LINE__);
		/*現在目標が視界外なら変更チェック*/
//		if( entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ){
		if( 1 ){
			if(entk->pl_eyei.pos == &(tng_com->subtrg->mov)){
//printf("TNG DEF THINK LINE %d\n",__LINE__);
				/*ＮＰＣモード*/
				/*プレイヤと距離調べ*/
				dis = GV_VecLen3F2( &GM_PlayerFindPos,
					&entk->ctrl->mov );
				if( dis < entk->pl_eyei.dis){
//printf("TNG DEF THINK LINE %d\n",__LINE__);
					ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
					&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
				}
			}else {
//printf("TNG DEF THINK LINE %d\n",__LINE__);
				/*プレイヤ*/
				/*ＮＰＣと距離調べ*/
				dis = GV_VecLen3F2( &(tng_com->subtrg->mov),
					&entk->ctrl->mov );
				if( dis < entk->pl_eyei.dis){
//printf("TNG DEF THINK LINE %d\n",__LINE__);
					ENE_SetEyeInfo( &(entk->pl_eyei), &tng_com->subtrg->mov, 
						&tng_com->subtrg->addr, NULL, &tng_com->subtrg->map ) ;
				}
			} 
		}
	}
}
static void Def_AimPosSet(ENETHINK *entk ,float sub)
{
	FVECTOR		shotpos ;
	TNG_COM		*tng_com ;
	ENTK_TENG_A *eve_a ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	if(tng_com->subtrg == NULL){
		/****
		新
		entk->status に ENE_STATUS2_AIM_GUNSHOOTを立てると
		entk->act->aim_posに向けてアジャストします。
		自動的にプレイヤーの狙う個所を設定するには
		ENE_SetAimPlayer( entk )
		を使用してください。
		****/
		entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
		GV_MatToVec( &( BODYWORLD(GM_PlayerBody,HUMAN21_ATAMA) ),&shotpos);
		entk->act->aim_pos = shotpos;
	} else {
//		entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
		if(entk->pl_eyei.pos == &(tng_com->subtrg->mov)){
			/*NPC*/
			entk->act->aim_pos = tng_com->subtrg->mov ;
			if(
			(entk->rnavi->pa_action[(int)entk->rnavi->next_node] == STAND_AND_SQUAT_ACT)&&
			(entk->act->aim_pos.vy > (entk->ctrl->mov.vy - 3000.0f))
			){
				/*プレイヤと同じフロアで低い壁の向こうなので腰を狙う*/
				/*NOP*/
			}else {
				/*目標の足元を狙う*/
				entk->act->aim_pos.vy = tng_com->subtrg->levels[0] ;
			}
		}else {
			/*プレイヤ*/
			entk->act->aim_pos = GM_PlayerPosition ;
			if(
			(entk->rnavi->pa_action[(int)entk->rnavi->next_node] == STAND_AND_SQUAT_ACT)&&
			(entk->act->aim_pos.vy > (entk->ctrl->mov.vy - 3000.0f))
			){
				/*NOP*/
			}else {
				/*目標の足元を狙う*/
				entk->act->aim_pos.vy = GM_PlayerControl->levels[0] ;
			}
		}
	}
}


static void Think3_ZoneChaseTrgp( entk )
ENETHINK	*entk ;
{
	int reach;
	AT_THK *at_thk;
	ENTK_TENG_A *eve_a ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
//col.vy =127 ;

#ifdef DEBUG_MODE
//	SVECTOR col ;
//	PosBox( &entk->trgpoint.pos ,250.0F ,NULL );
//	PosBox( &entk->ctrl->mov ,500.0F ,NULL );
	if(entk->count3 %(AT_THK_RATE*5)==0){
//		SigZoneView( entk->trgpoint.addr , NULL , 1000.0F ) ; 
	}
#endif

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
		}
	}
#endif

	entk->act->pad = SP_MOVE_RUN ;

	/*現在地アドレス更新*/
	if(!(entk->count3%(AT_THK_RATE*5))) {
		entk->znavi->this_addr = 
		HZX_GetAddress( entk->ctrl->hzx_id, 
		&( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;
	}


	if(CheckJumpPosENE(entk)){
		SIG_SetChasePos(entk ,&entk->tmp_pos) ;
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_DIRECT_CHASE_JUMP ;
		entk->count3 = 0 ;
		return ;
	}
#if 0
	{
		SVECTOR	rgb ;
		rgb.vx = 0;
		rgb.vy = 0;
		rgb.vz = 127;
		SigZoneView( entk->trgpoint.addr , &rgb , 1000.0F ) ; 
		PosBox( &entk->trgpoint.pos,500.0F ,NULL );
	}
#endif
	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );
	/*ゾーン追跡*/
	if ( ENE_ZoneTrace( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ) {
	/*ダイレクト追跡に移行*/
		entk->think2 = TH2_MOVE ;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if(entk->count3%(AT_THK_RATE*3)==0){
		if(
//			( entk->pl_eyei.dis > 2000)&&
			( entk->pl_eyei.dis > 1000)&&
			(
			(reach <= 2)
			||(entk->count3 < (DIRECT_TICK(30)))
			)
		){
//1
			SetMoveMode(entk);
		} else {
			if(eve_a->type >= 2){
//				if(entk->count3 > DIRECT_TICK(60))
				if(entk->count3 < DIRECT_TICK(30)){
//2
					SetMoveMode(entk);
				}else {
					entk->act->move_s = MoveAttackRun ;
				}
			}else {
#if 0
				if(entk->act->move_s != MoveAttackRun){
					entk->act->tmp_time = (AT_THK_RATE*2) ;
				}
				entk->act->move_s = MoveAttackRun ;
#else
				if(reach <= 2){
//3
					SetMoveMode(entk);
				}else {
					entk->act->move_s = MoveAttackRun ;
				}
#endif
			}
		}
	}
	ReverseCheck( entk );

	
	/*移動中の自衛*/
	if(
	(entk->count3 > DEF_MOVE_SHOT_DELAY)
	&&(entk->count3%(AT_THK_RATE) ==0)
	&&( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
	){
		if(eve_a->type <= 1){
			if(
			(abs(SIG_CheckDirSub(entk->trgpoint.dir,entk->pl_eyei.dir)) < 128)
			){
//				Def_Shot( entk ,SHOT_MODE_NO_TRG);
				Def_Shot( entk ,0);
			}
		}else {
				Def_Shot( entk ,SHOT_MODE_NO_TRG);
		}
	}
	if(entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		entk->act->dir = entk->pl_eyei.dir ;
		/*プレイヤ方向見ながら移動*/
//吹っ飛ばし
#if 0
		if(eve_a->type <= 1){
			if( entk->pl_eyei.dis < 4000) Def_Set_ForceDir( entk ,500.0F);
		}
#endif

	}
	entk->count3 ++ ;
	if(at_thk->at_tmptime > 0)	at_thk->at_tmptime-- ;
}

static	void	Think3_DirectChaseTrgp( entk )
ENETHINK	*entk ;
{
	int		reach;
	ENTK_TENG_A *eve_a ;
	AT_THK *at_thk;
//	SVECTOR		rgb;
	
#if 0
	rgb.vx = 255 ;	rgb.vy = 0 ;	rgb.vz = 0 ;
	PosBox( &entk->trgpoint.pos ,100.0F ,&rgb );
	rgb.vx = 0 ;	rgb.vy = 0 ;	rgb.vz = 255 ;
	PosBox( &(entk->znavi->flore_pos) ,50.0F ,&rgb );
//	SigZoneView( entk->trgpoint.addr , NULL , 1000.0F ) ; 
#endif
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	at_thk = (AT_THK *) entk->character ;

#if 0
	if(eve_a->type <= 1) {
		/*左舷兵の追っかけ移行処理*/
		if(Def2AtCheck(entk)){
			return;
		}
	}
#endif

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			/*めくら時の処理*/
		}
	}
#endif

	entk->act->pad = SP_MOVE_RUN ;


	/*目標点の高さを修正*/
//	entk->trgpoint.pos.vy = entk->znavi->flore_pos.vy ;
	/*追跡のための方向設定と距離判定*/
	if(ENE_DirectTrace( &(entk->trgpoint),
	 &(entk->znavi->flore_pos), 100 ) <0 ){
ASSERT(entk->think1 == ENE_TH1_DEFENSE) ;
		entk->act->mot_speed_correct = 0.0F;
		entk->ctrl->mov.vx = entk->trgpoint.pos.vx ;
		entk->ctrl->mov.vz = entk->trgpoint.pos.vz ;
		/*到達*/
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_WAIT ; 
		entk->count3 = 0 ;
		entk->act->dir = entk->ctrl->turn.vy ;
		entk->act->pad = SP_STAND_STILL ;
		return ;
	}

	/*距離外*/
	if( entk->count3 ==0){
		/*現在値 計算し直し*/
		entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), -1 ) ;
	}
	/* いつまでも直線じゃいられない */
	entk->znavi->this_addr = HZX_GetAddress( entk->ctrl->hzx_id, &( entk->znavi->flore_pos ), entk->znavi->this_addr ) ;

	reach = HZX_ReachTo( entk->znavi->this_addr, entk->trgpoint.addr );

	if( reach > HZX_INDIRECT_REACH ) {
	/*ゾーン到達してない*/
		entk->think3 = TH3_ZONE_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

	if(entk->count3%(AT_THK_RATE*3)==0){
		if(
//			(entk->count3 < 60)
//			||( entk->trgpoint.h_dis < 4000 )
			(1)
		){
			SetMoveMode(entk);
		}else {
			if((entk->act->move_s == MoveBack) 
			){
				SetMoveMode(entk);
			}else {
				entk->act->move_s = MoveAttackRun ;
			}
		}
	}
	if(entk->act->move_s == MoveAttackRun) {
		entk->act->dir = entk->trgpoint.dir ;
	} else {
		/*プレイヤ方向見ながら移動*/
//			ENE_SetAimPlayer( entk ) ;
			SetAimPosEyei(entk) ;
			entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
//			Def_AimPosSet(entk ,-500) ;
		entk->act->dir = entk->pl_eyei.dir ;
		if(
		(1)
		&&( entk->pl_eyei.dis < entk->sense.eye_s)
		){
		}
	}



	entk->count3 ++ ;
}


//#define DEF_STAY_LIMIT	1800
#define DEF_STAY_LIMIT	600

#define DEF_STAY_DECAY	300
static void DEF_StartDefAttack(ENETHINK *entk){
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	AT_THK *at_thk;

   BP_SetConstants();

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	/*ライト破壊モード*/
#if 1
//		eve_a->level = DEF_LEVEL_SUBTRG ;
//		entk->at_com->Pl_StayTime = 0 ;
//		eve_a->level = DEF_LEVEL_GRD ;
//		entk->act->target_pos = GM_PlayerPosition ;
//		tng_com->seq_cont = DEF_ATTACK_GRD_CYCLE ;

		eve_a->level = (irnd()>>8)%(DEF_LEVEL_GRD) ;
#endif
	at_thk->at_tmptime = 0;
	eve_a->seq_index = 0;
	entk->count3 = 0 ; 
	entk->think3 = TH3_ATTACK_SETUP ; 

//entk->rnavi->nodes[0]
//entk->rnavi->pa_action[0]
//eve_a->def_pos_num
//	eve_a->def_seq = (u_char)(tng_com->def_pos[(int)eve_a->def_pos_num].vw ) ;
	eve_a->def_seq = (u_char)(entk->rnavi->pa_action[(int)entk->rnavi->next_node]) ;
//printf("eve_a->def_seq[%d]\n",eve_a->def_seq);
//printf("(int)eve_a->level[%d]\n",(int)eve_a->level);
//printf("eve_a->seq_index[%d]\n",eve_a->seq_index);

	if(eve_a->def_seq > DEF_LEVEL_END){
		eve_a->def_seq = 0 ;
	}
	eve_a->seq_time = 
		act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
	eve_a->seq_count = 0 ;
}

/*左舷兵の追っかけ移行処理*/
/*****
注意 左舷の２フェイズ以降の突撃は
static void DEF_StartDefAttack(ENETHINK *entk)
内で分岐
******/
#if 0
static int Def2AtCheck(ENETHINK *entk){
	int 		zone_dis ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	AT_THK *at_thk;

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	if(entk->count3%(AT_THK_RATE*2) ==0){
		zone_dis = ENE_GetRouteDis(&tng_com->final_pos,
			&entk->ctrl->mov,tng_com->final_addr,
			entk->ctrl->addr,5000000);
		if( zone_dis > (tng_com->pl_fi_dis-2000)){
			eve_a->status |= EVE_TNG_NO_DEF ;
			entk->act->pad = SP_READYGUN ;
			entk->count3 = 0;
			return 1;
		}
	}

	/*2001.01.22追加 時間制限条件*/


	return 0;
}
#endif
/*攻撃兵モードから守備兵に戻る*/
void TNG_AT2DEF_Check(ENETHINK *entk)
{
	int ene_zone_dis ;
	ENTK_TENG_A *eve_a ;
	TNG_COM	*tng_com;
	AT_THK *at_thk;

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM *) eve_a->tng_com ;
	ene_zone_dis = ENE_GetRouteDis(&entk->ctrl->mov,
		&tng_com->final_pos,entk->ctrl->addr,
		tng_com->final_addr,9000000 );
		/*必ず守備復帰*/
//		TngGetDefPos2(entk);
		eve_a->status &= ~EVE_TNG_NO_DEF ;
	/**/

/*************************************/

}



static void Think3_Wait( entk )
ENETHINK	*entk ;
{
	FVECTOR		testtrg,subvec;
	float len;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	AT_THK *at_thk;
	ACTION	*act;

	at_thk = (AT_THK *) entk->character;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	act = entk->act ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	if(
//	( entk->ctrl->addr != entk->trgpoint.addr )
	(fabs(entk->znavi->flore_pos.vx - entk->def_pos.vx) > 250.0f)
	||(fabs(entk->znavi->flore_pos.vz - entk->def_pos.vz) > 250.0f)
	){
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ; 
		entk->count3 = 0 ;
		return ; 
	}

	act->pad = SP_SQUATHIDE ;
	act->dir = entk->pl_eyei.dir  ;
	/* 位置補正処理 */

	testtrg = entk->def_pos ;
	testtrg.vy = entk->ctrl->mov.vy;
    _sceVu0SubVector( &subvec, &testtrg, &entk->ctrl->mov ) ;
	len = GV_VecLen3F( &subvec );
	if( len < 25.0F) {
		act->tmp_dir = -1;
	}else {
		act->tmp_dir = GV_VecDir2( &subvec );
	}

	/*アクション開始*/
	/*同期させる場合はここで条件設定*/
#if 0
	if(entk->count3 > DEF_WAIT_TIME){
		DEF_StartDefAttack(entk);
		return ;
	}
#else
//	if(eve_a->type <= 1 ){
	if(0 ){
		if(
		/*先頭の2人のみ参戦*/
//		(at_thk->dis_rank<=1)
		(1)
		&&(entk->count3 > DEF_WAIT_TIME)
//		&&(tng_com->seq_cont%300 == 0)
		){
			DEF_StartDefAttack(entk);
			return ;
		}
	}else {
		if(
		(entk->count3 > DEF_WAIT_TIME)
		&&(tng_com->seq_cont  == 0)
		&&( entk->pl_eyei.dis < (entk->sense.eye_s))
		){
			DEF_StartDefAttack(entk);
			return ;
		}
	}
#endif
	entk->count3 ++ ;
}
/*ATTACK*/
static void Think3_AttackStand( entk )
ENETHINK	*entk ;
{
	int bure = 1,shot_mode;
	FVECTOR		testtrg,subvec;
	float len;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	AT_THK *at_thk;
	ACTION	*act;

   BP_SetConstants();

	at_thk = (AT_THK *) entk->character ;
	shot_mode = SHOT_MODE_AIM ;
	act = entk->act ;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;


	/*守備時間のカウント*/
	if(eve_a->det_time < 30000 ) eve_a->det_time++;
	/* 位置補正処理 */
	testtrg = entk->def_pos ;
	testtrg.vy = entk->ctrl->mov.vy;
    _sceVu0SubVector( &subvec, &testtrg, &entk->ctrl->mov ) ;
	len = GV_VecLen3F( &subvec );
	if( len < 25.0F) {
		act->tmp_dir = -1;
	}else {
		act->tmp_dir = GV_VecDir2( &subvec );
	}

	/*これより下でシーケンス処理*/
	if(entk->count3 ==0){
		/*一番最初*/
		at_thk->at_tmptime = 0; /*着弾予測カウンタ*/
		eve_a->seq_time = 
		act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
		eve_a->seq_count = 0 ;
	}
	/*待機状態なら再飛び出しチェックは毎フレーム*/
	if(
	(eve_a->status & EVE_TNG_WAIT)
	&&(eve_a->seq_index == 0)
	){
		if( entk->pl_eyei.dis < (entk->sense.eye_s-250)){
			eve_a->seq_index = 1;
			eve_a->seq_time = 
			act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
			eve_a->seq_count = 0 ;
			eve_a->status &= ~EVE_TNG_WAIT ;
		}
	}

	/* パッドは毎フレーム更新 */
	act->pad = act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2];
	/* 
	tmptimeは着弾予測処理の回数を減らすためのカウンタに使用
	*/
	/* inndexに従いアクション時間セット */
	/* 時間は切り替わった時のみセットして以降減衰 */

	/*左持ち兵の判定*/
	ReverseCheck( entk );
	/*プレイヤ以外を目標に*/

	/*パッドによって狙い場所変えてるのでpad設定後にaim*/
	Def_AimPosSet(entk ,-500) ;
	/* 攻撃アクションなら発砲*/
	if(	act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2] > SHOT_ACT){
		if(
			(eve_a->seq_count > tng_com->shot_delay )
			&&(eve_a->seq_count <= (tng_com->shot_delay+tng_com->shot_time) )
			&&(entk->count3%(AT_THK_RATE) ==0)
			&&(at_thk->at_tmptime == 0)
		){
			if((bure)&&(eve_a->shot_rnd > 0)){
				/*手ぶれ*/
				int		range1,range2 ;
				float	shot_time ,shot_rest;
				float	trg_dis,shot_rnd;

				trg_dis = GV_VecLen3F2( &act->aim_pos, &entk->ctrl->mov );
				shot_time =(float) (tng_com->shot_delay+tng_com->shot_time) ;
				shot_rest = shot_time - (float) eve_a->seq_count ;
#if 1
				shot_rnd = (float) eve_a->shot_rnd  * shot_rest/shot_time ;
#else
				if(shot_rest< DEF_ABS_HIT_TIME) shot_rnd = 0 ;
				else shot_rnd = (float) eve_a->shot_rnd  * shot_rest/shot_time ;
#endif
				shot_rnd *= (trg_dis/6000.0F);

//				if(shot_rest > 10 ) shot_mode |= SHOT_MODE_NO_TRG ;
//				if(shot_rnd >= 100.0F ) shot_mode |= SHOT_MODE_NO_TRG ;

				if(shot_rnd >= NO_SHOT_LIMIT ) shot_mode |= SHOT_MODE_NO_TRG ;

				if(shot_rnd >= 1.0F){
#if 0
					range1 = eve_a->shot_rnd ;
					range2 = eve_a->shot_rnd/2 ;
#else
					range1 = (int) (shot_rnd) ;
					range2 = (int) (shot_rnd/2.0F) ;
#endif

					entk->act->aim_pos.vx += ((float)((irnd()>>8)%range1)-(float)range2) ;
					entk->act->aim_pos.vy += ((float)((irnd()>>8)%range1)-(float)range2) ;
					entk->act->aim_pos.vz += ((float)((irnd()>>8)%range1)-(float)range2) ;
				}
			}
		/*
			着弾予測して自分の直前壁で遮られるようなら撃たない
		*/
			if(1000.0F < CheckGunPlOnline(entk,&entk->act->aim_pos))
			{
				Def_Shot( entk ,shot_mode );
			} else {
				(at_thk->at_tmptime = (AT_THK_RATE*2)) ;
			}
		}
	}
//PosBox(&entk->act->aim_pos,250.0F,NULL);
	/*カウンタ減衰*/
	if(at_thk->at_tmptime>0) at_thk->at_tmptime--;
	entk->bullet = 0 ;
	entk->act->dir = entk->pl_eyei.dir  ;
	entk->count3 ++ ;
	if(eve_a->seq_time != WAIT_END){
		eve_a->seq_count++ ;
	}else {
	}
	/*次のフレームで新アクションをセットする時*/
	if(
	((eve_a->seq_time == IDLE_TIME)
	&&(eve_a->seq_count>(tng_com->shot_delay+tng_com->shot_time+tng_com->wait_time)))
	||((eve_a->seq_time == WAIT_END)&&(entk->act->act_end))
	||((eve_a->seq_time >= MAGIC_END)&&(eve_a->seq_count>=eve_a->seq_time))
	){
		at_thk->at_tmptime = 0;
		eve_a->seq_index++;
		/*次にセットされる予定が終端*/
		if(act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2] == SP_DATA_END )
		{
//			ENE_SetNextnode(entk->rnavi);
			ENE_SetTrgpNextnode(entk->rnavi,&entk->trgpoint);
			entk->def_pos = 
				entk->rnavi->nodes[(int)entk->rnavi->next_node] ;
//			entk->def_mapbit = 
//				entk->rnavi->mapbit[entk->rnavi->next_node] ;
			if((fabs(entk->znavi->flore_pos.vx - entk->def_pos.vx) > 500.0f)
			||(fabs(entk->znavi->flore_pos.vz - entk->def_pos.vz) > 500.0f)
			||(fabs(entk->znavi->flore_pos.vy - entk->def_pos.vy) > 500.0f)
			){
				entk->think1 = ENE_TH1_DEFENSE ; 
				entk->think2 = TH2_MOVE ; 
				entk->think3 = TH3_ZONE_CHASE ; 
			}else {
				entk->think1 = ENE_TH1_DEFENSE ; 
				entk->think2 = TH2_ATTACK ; 
				entk->think3 = TH3_WAIT ;
			}
			entk->count3 = 0 ;
			return ;
		}
		/*次のアクションセット*/
//entk->rnavi->nodes[0]
//entk->rnavi->pa_action[0]
//eve_a->def_pos_num
//		eve_a->def_seq = (u_char)(tng_com->def_pos[(int)eve_a->def_pos_num].vw ) ;
		eve_a->def_seq = (u_char)(entk->rnavi->pa_action[(int)entk->rnavi->next_node]) ;

#if 1
		if(eve_a->def_seq > DEF_LEVEL_END){
			eve_a->def_seq = 0 ;
		}
		eve_a->seq_time = 
			act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
#else
		eve_a->seq_time = 
			act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
#endif

	}
#if 0
		/*遠ければ隠れっぱなし*/
		/*ライト破壊時間を過ぎていれば隠れない*/
	if(( entk->pl_eyei.dis > entk->sense.eye_s)
	&&(!((tng_com->count == 0)&&(eve_a->type == 2)))
	){
		eve_a->seq_index = 0;
		eve_a->status  |= EVE_A_WAIT ;
	}
	eve_a->seq_time = 
		act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2+1];
	eve_a->seq_count = 0 ;
	/* 目標ゾーンから離れていれば戻る*/
	next_pad =
	act_list[eve_a->def_seq][(int)eve_a->level][eve_a->seq_index*2];
	if(
	(next_pad == SP_STAND_STILL)
	||(next_pad == SP_SQUATHIDE )
	){
		CONTROL	ctrl;
/****
ENE_SetNextnode
ENE_SetTrgpNextnode
****/
//	if(eve_a->def_pos_req != eve_a->def_pos_num){
	if(eve_a->def_pos_req != eve_a->def_pos_num){
//		TngGetDefPos3( entk) ;
		eve_a->status &= ~EVE_A_NO_DEF ;
		ctrl.mov = entk->def_pos ;
		GM_ConfigControlMapID( &ctrl ) ;
		entk->trgpoint.pos = entk->def_pos ;
		entk->trgpoint.map = ctrl.map ;
		entk->trgpoint.addr = ctrl.addr ;
	}
	if(
//		( entk->ctrl->addr != entk->trgpoint.addr )
	(abs(entk->znavi->flore_pos.vx - entk->def_pos.vx) > 250)
	||(abs(entk->znavi->flore_pos.vz - entk->def_pos.vz) > 250)
	){
			entk->think1 = ENE_TH1_DEFENSE ; 
			entk->think2 = TH2_MOVE ; 
			entk->think3 = TH3_ZONE_CHASE ; 
			entk->count3 = 0 ;
			return ; 
		}
/*2001.01.18 test*/
		entk->think1 = ENE_TH1_DEFENSE ; 
		entk->think2 = TH2_ATTACK ; 
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0 ;
	}
#endif
}
#define PL_NO_SHOOT (PLAYER_DAMAGED|PLAYER_DOWNED|PLAYER_FORCE)

#define ALERT_DIS 7000
#define ATTACK_DIS 6000
#define ATTACK_FORCE  (1000.0F)
/*左舷 後方 無敵兵のスタンド思考*/
static void Think3_DefenseStand( entk )
ENETHINK	*entk ;
{
	FVECTOR 	testtrg,subvec;
	float len,z_dis;
	ENTK_TENG_A *eve_a ;
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;

	/*守備兵の守備経過時間用のdet_timeを*/
	/*後方兵は必要ないので覗き込みシーケンス管理に流用*/

	entk->sw_gun |= SW_FLAG_SWITCH1 ;
	Def_AimPosSet(entk ,-500) ;

	/*守備座標への補正*/
	testtrg = entk->def_pos ;
	testtrg.vy = entk->ctrl->mov.vy;
//	GV_SubVec3( &testtrg, &entk->ctrl->mov, &subvec );
    _sceVu0SubVector( &subvec, &testtrg, &entk->ctrl->mov ) ;
	len = GV_VecLen3F( &subvec );
	if( len < 25.0F) {
		entk->act->tmp_dir = -1;
	}else {
		entk->act->tmp_dir = GV_VecDir2( &subvec );
	}
	if(entk->count3 == 0){
		eve_a->det_time = 0 ;
	}
	/*吹っ飛ばし攻撃のためＺ距離判定*/
	z_dis = fabs( entk->ctrl->mov.vz - entk->pl_eyei.pos->vz) ;

	ReverseCheck( entk );

	if(z_dis > ALERT_DIS) {
		/*隠れてろ*/
		if(eve_a->det_time == 0){
			entk->act->pad = SP_STAND_STILL ;
		} else if (eve_a->det_time == 1) {
			entk->act->pad = SP_PEEK_HIGH_END_L ;
		} 
		if(entk->act->act_end) {
			eve_a->det_time = 0 ;
			entk->act->pad = SP_STAND_STILL ;
		}
	} else if (z_dis > ATTACK_DIS) {
		/*警戒しろ*/
		if(eve_a->det_time == 0){
			entk->act->pad = SP_PEEK_HIGH_START_L ;
#if 1
			if(entk->act->act_end) {
				entk->act->pad = SP_PEEK_HIGH_IDLE_L ;
				eve_a->det_time = 1 ;
				entk->act->pad = -1;
				entk->act->dir = -1 ;
			}
#endif
		}else {
			entk->act->pad = SP_PEEK_HIGH_IDLE_L ;
			if(entk->count3%AT_THK_RATE ==0) Def_Shot( entk ,SHOT_MODE_AIM );
		}
	}else {
#if 0
		/*ふっ飛ばせ*/
		if(!(GM_PlayerStatus & PL_NO_SHOOT)) {
			if(entk->count3%(AT_THK_RATE) ==0) Def_Shot( entk ,SHOT_MODE_AIM );
			Def_Set_ForceDir( entk ,ATTACK_FORCE);
		}
#endif
		eve_a->det_time = 2 ;
		entk->act->pad = SP_PEEK_HIGH_IDLE_L ;
	}


	/*パッドによって狙い場所変えてるのでpad設定後にaim*/
	Def_AimPosSet(entk ,-500) ;

	entk->bullet = 0 ;

/*射撃方向*/
	entk->act->dir = entk->pl_eyei.dir  ;
	entk->count3 ++ ;
}


static	void	Think3_DirectChaseJump( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	at_thk = (AT_THK *) entk->character ;

	at_thk->at_status &= (~AT_ST_SQUAT);
	entk->act->pad = SP_MOVE_RUN ;
	entk->act->move_s = MoveCautionWalk ;
	entk->act->dir = entk->trgpoint.dir ;
	entk->act->tmp_dir= entk->trgpoint.dir ;
	/*追跡のための方向設定と距離判定*/
	/*座標追跡*/
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif
	if(
		( ENE_DirectTrace( &(entk->trgpoint), &(entk->znavi->flore_pos), 250 ) < 0 ) 
		){
		/*到着後*/
		/*目標座標に移動*/
		entk->act->mot_speed_correct = 0.0F;
		/*目標修正*/
#if 0
	if(entk->tmp_pos.vy == entk->act->target_pos.vy){
		entk->act->pad = SP_TNG_JUMP_LEVEL ;
	}else {
		entk->act->pad = SP_TNG_JUMP ;
	}
#endif
//		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
		entk->act->dir = (int) entk->tmp_pos.vw ;
		entk->think3 = TH3_START_JUMP ;
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}
static void SetTrgpDefPos( ENETHINK *entk){
	CONTROL ctrl ;
	ctrl.mov = entk->def_pos ;
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.pos = entk->def_pos ;
	entk->trgpoint.map = ctrl.map ;
	entk->trgpoint.addr = ctrl.addr ;
}
static	void	Think3_StartJump( entk )
ENETHINK	*entk ;
{
	AT_THK *at_thk;
	FVECTOR		tmp ;

	at_thk = (AT_THK *) entk->character ;
	
#if 0
	{
	SVECTOR rgb ;
	rgb.vx = 255 ;	rgb.vy = 0 ;	rgb.vz = 0 ;
	PosBox( &entk->trgpoint.pos ,100.0F ,&rgb );
	rgb.vx = 0 ;	rgb.vy = 0 ;	rgb.vz = 255 ;
	SigZoneView( entk->znavi->next_addr , NULL , 1000.0F ) ; 
	}
#endif

	at_thk->at_status &= (~AT_ST_SQUAT);
	if(entk->count3==0){
		/*ジャンプ目標距離と方向を設定*/
		_sceVu0SubVector( &tmp ,&entk->ctrl->mov,
			&entk->act->target_pos) ;
		tmp.vy = 0.0F ;
		entk->act->target_pos.vw = GV_VecLen3F(&tmp) ;
		entk->tmp_pos.vw = (float) GV_VecDir2FromTo( &entk->ctrl->mov,
			&entk->act->target_pos) ;
	}
	if(entk->tmp_pos.vy == entk->act->target_pos.vy){
		entk->act->pad = SP_TNG_JUMP_LEVEL ;
	}else {
		entk->act->pad = SP_TNG_JUMP ;
	}
	entk->act->dir = (int) entk->tmp_pos.vw ;
	/*追跡のための方向設定と距離判定*/
	/*座標追跡*/
#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 255;	rgb.vy =   0;	rgb.vz =   0;
		PosBox(&entk->trgpoint.pos ,250.0F ,&rgb );
		rgb.vx =   0;	rgb.vy =   0;rgb.vz =  255;
		PosBox(&(entk->znavi->flore_pos) ,250.0F ,&rgb );
	}
#endif
	if(entk->act->act_end == 1){
		/*到着後*/
		/*目標座標に移動*/
		entk->act->pad = SP_READYGUN ;
#if 0
		ENE_SetTrgpPlayer( entk->znavi, &(entk->trgpoint), entk->com ) ;
#else
/*予約守備位置に復帰*/
		SetTrgpDefPos(entk);
#endif

#if 0
/*たちどまり*/
		GoNearAttack(entk);
#else
/*追跡*/
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ;
#endif
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}
static void Think3_Wait_Dynamic( entk ) 
ENETHINK	*entk ;
{
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	entk->act->pad  = SP_STAND_STILL ;
	entk->act->dir = entk->pl_eyei.dir  ;
	entk->act->tmp_dir = -1  ;
	entk->count3++ ;
}

static void Think2_Move( entk ) 
ENETHINK	*entk ;
{
//	int zone_dis ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;

	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;


//	entk->trgpoint.pos = entk->def_pos ;
//PosBox(&entk->trgpoint.pos,250.0F,NULL);
//printf("TRG POS X %f\n",entk->trgpoint.pos.vx) ;
//	if(dis_rank==0){
	if(eve_a->type <= 1){
#if 0
		zone_dis = tng_com->def_zonedis[eve_a->def_pos_num] ;
		/**/
		if((zone_dis) > (tng_com->pl_fi_dis - 3000 ) ){
printf("move TNGPOS CHECK\n");
			CheckDefPos(entk);
		}
#endif
		if(
		(eve_a->def_pos_req != -1)
		&&(eve_a->def_pos_req != eve_a->def_pos_num)
		){
			CONTROL	ctrl;
//		TngGetDefPos3( entk) ;
			eve_a->status &= ~EVE_TNG_NO_DEF ;
			ctrl.mov = entk->def_pos ;
			GM_ConfigControlMapID( &ctrl ) ;
			entk->trgpoint.pos = entk->def_pos ;
			entk->trgpoint.map = ctrl.map ;
			entk->trgpoint.addr = ctrl.addr ;
		}
	}

	switch ( entk->think3 ) {
		case TH3_ZONE_CHASE :
			Think3_ZoneChaseTrgp( entk ) ;
			break;
		case TH3_DIRECT_CHASE :
			Think3_DirectChaseTrgp( entk ) ;
			break;
		case TH3_WAIT :
			Think3_Wait_Dynamic(entk) ;
			break;
		case TH3_DIRECT_CHASE_JUMP  :
			Think3_DirectChaseJump(entk);
		break;
		case TH3_START_JUMP :
			Think3_StartJump(entk);
		break;
	}
}

static void Think2_Attack( entk ) 
ENETHINK	*entk ;
{
	ENTK_TENG_A *eve_a ;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;

	switch ( entk->think3 ) {
		case TH3_ATTACK_SETUP :
			if(eve_a->type != 3)
			{
				Think3_AttackStand( entk ) ;
			} else {
				Think3_DefenseStand( entk ) ;
			}
			break ;
		case TH3_WAIT :
				Think3_Wait( entk ) ;
			break;
	}
}
static void Think2_Wait( entk ) 
ENETHINK	*entk ;
{
	entk->act->pad  = SP_STAND_STILL ;
	entk->count3 = 0;
}

void	Think1_TngDefense( entk )
ENETHINK	*entk ;
{
//	FVECTOR force = { 0.0F,0.0F,-1000.0F } ;
	int now_point ;
	BODYPARAM	*bodyp ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;
	bodyp = &entk->act->bodyp ;

	now_point = entk->rnavi->next_node ;

#if 0
	if(CheckUnrealMode(entk)){
		return ;
	}
#endif
	CheckAttackTarget(entk) ;

	if(eve_a->status & EVE_TNG_SUPORT_WAIT)
	{
		Think2_Wait( entk ) ;
	} else {
		switch ( entk->think2 ) {
			case TH2_MOVE :		/* 移動 */
			if(
			(entk->rnavi->pa_action[now_point] == KATANA_ACT)||
			(eve_a->type == TNG_TYPE_D)
			){
				TngKatanaOn(entk);
			}else {
				TngKatanaOff(entk);
			}

			Think2_Move( entk ) ;
			break ;
		    case TH2_ATTACK :		/* 攻撃 */
				/*ポイント到達後*/
				if(!(eve_a->status & EVE_TNG_NO_DEF)){
					/*首絞めくん*/
					if(
					(entk->rnavi->pa_action[now_point] == HANG_ACT)
					){
#ifdef DEBUG_MODE						
if(tng_com->mode &TNG_COM_DEBUG){
	PosBox( &(entk->ctrl->mov) ,2000.0F ,NULL );
}
#endif
//printf("HANG WAIT routenum [%d]\n",tng_com->def_route[eve_a->def_pos_num]);
//						if(eve_a->def_pos_num == tng_com->hang_level){ 
						if(tng_com->def_route[(int)eve_a->def_pos_num] == tng_com->hang_num){ 
							eve_a->status |= EVE_TNG_NO_DEF ;
							eve_a->status |= EVE_TNG_HANG ;
						}else if(entk->com->max_res_in_alert == entk->com->res_count_in_alert){
							/*首絞め解除*/
							eve_a->status |= EVE_TNG_NO_DEF ;
						}else {
							/*そのまま待機*/
ASSERT(entk->think1 == ENE_TH1_DEFENSE) ;
							entk->think3 = TH3_WAIT ;
							entk->count3 = 0;
						}
					}else if(entk->rnavi->pa_action[now_point] == KATANA_ACT){
						TngKatanaOn(entk);
						eve_a->status |= EVE_TNG_NO_DEF ;
						return ;
					}else if(
					((entk->ctrl->mov.vz+1500.0f) > GM_PlayerPosition.vz)
					||(entk->rnavi->pa_action[now_point] == CHASE_ACT)
					||(eve_a->type >= TNG_TYPE_C)
					){
						/*突破判定は必ずプレイヤ基準で！！！！*/
						/*サブターゲットは無視*/
						eve_a->status |= EVE_TNG_NO_DEF ;
						return ;
					}else if(entk->rnavi->pa_action[now_point] == NO_ACTION_ACT){
						/*何もせず次の点へ向かう*/
						ENE_SetTrgpNextnode(entk->rnavi,&entk->trgpoint);
						entk->def_pos = 
							entk->rnavi->nodes[(int)entk->rnavi->next_node] ;
						entk->think1 = ENE_TH1_DEFENSE ; 
						entk->think2 = TH2_MOVE ; 
						entk->think3 = TH3_ZONE_CHASE ; 
						entk->count3 = 0 ;
						return ;
					}
				}
				/*配置攻撃モード*/
				Think2_Attack( entk ) ;
			break ;
		}
	}
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->alert = ALERT_LEVEL_MAX ;
	}
}

/*守備位置解放*/
void TngDefPosFree(ENETHINK *entk){
	ENTK_TENG_A *eve_a ;
	TNG_COM	*tng_com;
	eve_a = (ENTK_TENG_A *) entk->eve_a ;
//	tng_com = (TNG_COM *) entk->tng_com ;
	tng_com = (TNG_COM *) eve_a->tng_com ;
	if(
	(eve_a->def_pos_num >= 0)
	&&(eve_a->def_pos_num < EVENT_DEF_POINT_MAX)
	){
//		tng_com->def_use[ (int) eve_a->def_pos_num]= -1 ;
	}
	eve_a->seq_index = 0 ;
	eve_a->def_pos_num = -1; /*守備位置番号*/
	eve_a->def_pos_req = -1;
	entk->def_pos = DG_ZeroVector ;
}

/*守備位置確保*/
void TngGetDefPos( ENETHINK *entk)
{
	int zone_dis;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TENG_A * ) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	ASSERT(entk->at_com != NULL) ;

#if 0
	/*現在地から廊下終端までのゾーン距離*/
	zone_dis = ENE_GetRouteDis(&entk->ctrl->mov,
		&tng_com->final_pos,entk->ctrl->addr,
		tng_com->final_addr,9000000 );
#else
	/*プレイヤから廊下終端までのゾーン距離*/
	zone_dis = (tng_com->pl_fi_dis - 4000) ;
#endif

}
/*自分の守備位置が適切かチェック*/
#if 0
void TngCheckDefPos( ENETHINK *entk)
{
	int i,zone_dis;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	CONTROL		ctrl ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TENG_A * ) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;
	zone_dis = tng_com->pl_fi_dis - 3000 ;

	TngDefPosFree(entk);
	if(i >= tng_com->def_pos_num ){
	/*ポイント無し*/
	/*追跡モード*/
		eve_a->status |= EVE_TNG_NO_DEF ;
		return ;
	}
}
#endif
/***********
int		p_actstatus	PACT_STATUS 
short	c_route		現在のルート番号 
short	next_route	次のルート（メッセージで変更 ）
char	next_node	次のポイント 
char	n_nodes		ポイントの数 
char	chang_node	変更ポイント指定 
***************/
void TngReadNode( ENETHINK *entk){
	int i ;
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;

	eve_a = ( ENTK_TENG_A * ) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	entk->rnavi->next_node = 0 ;
	i = tng_com->use_num ;
printf("REQEST ROUTE NUM %d\n",i);
	if(i>= tng_com->def_pos_num) {
/*これ以上ポイントが無い*/
printf(" DEF ROUTE NO DDATA !!!\n");
		return ;
	}
//view
//	entk->rnavi->n_nodes /* ポイントの数 */
printf("TNG DEFROUTR READ %d\n",tng_com->def_route[i]);
	ENE_ReadNodes( entk->rnavi, tng_com->def_route[i]) ;
	entk->def_pos = entk->rnavi->nodes[(int)entk->rnavi->next_node] ;
//	entk->def_mapbit = entk->rnavi->mapbit[entk->rnavi->next_node] ;
//	tng_com->def_use[i] = entk->id ;
	eve_a->def_pos_num = i; /*守備位置番号*/
	eve_a->def_pos_req = i;
	eve_a->res_pos = tng_com->tng_respos_num[i] ;
	eve_a->def_level = tng_com->active_lv[i] ;
	tng_com->use_num++ ;
}

/*守備モード開始*/
void Sig_TngDefenseModeStart( ENETHINK *entk)
{
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	CONTROL		ctrl ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TENG_A * ) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	printf("DEF START ALERT MAX\n");
	entk->alert = ALERT_LEVEL_MAX ;
	entk->act->CheckPad = TngDefCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->act->dir = -1 ;
	entk->act->pad = 0 ;



	entk->think1 = ENE_TH1_DEFENSE ; 
	entk->think2 = TH2_MOVE ; 
	entk->think3 = TH3_ZONE_CHASE ; 
	entk->count3 = 0 ;


	entk->act->move_s = MoveAttackRun ;

	at_thk->safeaddr = HZX_NO_ZONE;

	ctrl.mov = entk->def_pos ;
printf("DEF_POS X[%f]Y[%f]Z[%f]\n",ctrl.mov.vx,ctrl.mov.vy,ctrl.mov.vz);
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.pos = entk->def_pos ;
	entk->trgpoint.map = ctrl.map ;
	entk->trgpoint.addr = ctrl.addr ;
	at_thk->safeaddr = entk->at_com->usezones[entk->u_id][entk->id] 
		= HZX_NO_ZONE;
	at_thk->safetype = 0;
	at_thk->sight_time =0; 
#if 1
	/*ダメージ時に戻らない用にstart処理から除外*/
	eve_a->seq_index = 0;	/** 守備シーケンス用 **/
#endif
	eve_a->seq_time = 0;	/** 守備シーケンス用 **/
//	eve_a->det_time = 0 ; /** 援護呼びまでのカウンタ **/
	at_thk->at_tmptime = 0;
	ctrl.mov = entk->def_pos ;
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.pos = entk->def_pos ;
	entk->trgpoint.map = ctrl.map ;
	entk->trgpoint.addr = ctrl.addr ;

//	eve_a->shot_delay = EVE_A_SHOT_DELAY;
//	eve_a->shot_time = EVE_A_SHOT_TIME;
//	eve_a->wait_time = EVE_A_WAIT_TIME ;

	if(tng_com->subtrg != NULL ){
printf("subtrg set \n");
		ENE_SetEyeInfo( &(entk->pl_eyei), &tng_com->subtrg->mov, 
		&tng_com->subtrg->addr, NULL, &tng_com->subtrg->map ) ;
	}
	eve_a->status = 0;
}
/*守備モード開始*/
void Sig_TngDefenseModeStart2( ENETHINK *entk)
{
	ENTK_TENG_A *eve_a ;
	TNG_COM		*tng_com ;
	CONTROL		ctrl ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;
	eve_a = ( ENTK_TENG_A * ) entk->eve_a ;
	tng_com = (TNG_COM * ) eve_a->tng_com ;

	at_thk->safeaddr = HZX_NO_ZONE;

	ctrl.mov = entk->def_pos ;
printf("DEF_POS X[%f]Y[%f]Z[%f]\n",ctrl.mov.vx,ctrl.mov.vy,ctrl.mov.vz);
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.pos = entk->def_pos ;
	entk->trgpoint.map = ctrl.map ;
	entk->trgpoint.addr = ctrl.addr ;
	at_thk->safeaddr = entk->at_com->usezones[entk->u_id][entk->id] 
		= HZX_NO_ZONE;
	at_thk->safetype = 0;
	at_thk->sight_time =0; 
	eve_a->seq_index = 0;	/** 守備シーケンス用 **/
	eve_a->seq_time = 0;	/** 守備シーケンス用 **/
	at_thk->at_tmptime = 0;
	ctrl.mov = entk->def_pos ;
	GM_ConfigControlMapID( &ctrl ) ;
	entk->trgpoint.pos = entk->def_pos ;
	entk->trgpoint.map = ctrl.map ;
	entk->trgpoint.addr = ctrl.addr ;
	eve_a->status = 0;
}

void Sig_TngDefenseModeStartDam( ENETHINK *entk ) {
	ENTK_TENG_A *eve_a ;
	eve_a = ( ENTK_TENG_A * ) entk->eve_a ;
	eve_a->status |= EVE_TNG_ATTAK_END ;
	if(eve_a->type >= 2){
		eve_a->def_seq = SQUAT_WAIT ;
printf("eve_a->def_seq %d\n",eve_a->def_seq);
	}
}


