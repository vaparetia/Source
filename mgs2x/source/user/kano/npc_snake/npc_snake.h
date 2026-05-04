/*
	npc_snake.h
		NPCスネーク

	2001/02/13 K.Kano
	$Id: npc_snake.h,v 1.1.1.3 2002/11/19 11:43:21 Yoshizawa1 Exp $
 */


#ifndef _npc_snake_h_
#define _npc_snake_h_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"
#include "camera.h"

#include "debugmenu.h"


#include "../../korekado/npc/npc.h"

#include "../../kira/effect/floor/floor.h"


#include "npc_snake.mh"

#include "args.h"

#include "../resman/resman.h"

#include "snake_voice.h"


typedef struct {
	/* サポート位置 */
	FVECTOR pos;

	/* サポート位置に対しての属性 */
	int posflag;

	/* ゾーンアドレス */
	HZX_ZONE_ADD zone;
} NPCSNAKE_SUPPORT_POINT;


typedef struct _Work {
	GV_ACT_EX actor;

	CONTROL ctrl;
	OBJECT body;

	/* LODハイポリゴンモデル */
	DG_OBJS *hiobjs;

	FMATRIX lights[2];

	NPCWORK		npc ;
	NAVIGATE 	navigate ;
	NAVITARGET	navitrg ;
	RADAR_CTRL	rctrl ;

    TARGET			deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	capture ;	/* 捕まりターゲット */
    TARGET			pushtrg ;	/* ぶつかりターゲット */

    TARGET			def_child[CHILD_TARGET_NUM] ;	/* 子ターゲット*/

    int			headmark ;

	TARGET		attack[7];
	POWER_TARGET	pow[7];

	/* w14a用 */
	HOMING_TRG	my_homing;

	int name;
	int where;


	/* 状況判定／次行動決定関数 */
	void (*GetSituation)(struct _Work *work);
	void (*DecideAction)(struct _Work *work);


	/* ライデンに関する情報 */

	/* ライデンの状態フラグ */
	int				raiden_status;

	/* ライデンの状態がSTAYの場合の位置ライン */
	FVECTOR			raiden_stay_pos;

	/* ライデンが以前いたゾーンの値 */
	HZX_ZONE_ADD	pre_raiden_zone;

	/* ゾーンを跨いだ移動をした場合のカウント数 */
	int				raiden_stay_count;

	/* ライデンの現在位置とスネークの現在位置の距離 */
	float			raiden_dis;


	/* 算出したスネークのいるべき位置 */
	int	support_pos_index;
	int hide_pos_index;
	FVECTOR neighbor_zone,neighbor_zone_using;
	FVECTOR *support_pos;
	int i_infinity;

	FVECTOR *hide_pos;
	int hide_dir;
	int hide_posflag;

	FVECTOR *behind_pos;
	int behind_dir;
	int behind_posflag;

	/* スネークが狙うべき敵キャラのホーミング構造体 */
	HOMING_TRG	*pre_homing;
	HOMING_TRG	*cmp_homing;
	HOMING_TRG	*homing;
	FVECTOR *homing_p;
	FVECTOR homing_pos;
	int homing_dir;
	float homing_dis;
	short homing_part;
	short pre_homing_part;

	/* 2001/9/17  K.Kano
	   クレイモア */
	TARGET *claymore;

	/* 一度に撃つ弾数の制限 */
	short shoot_limit;
	short shoot_count;
	short opt_shoot_limit;
	int shoot_interval_count;

	/* 行動方針の番号 */
	/* 自己判断モード、ライデン追跡モード、ライデン援護モード */
	int mode;

	/* 位置移動、ライデン追跡、USP射撃、FAMAS掃射 */
	int action_num;

	/* カウンター */
	int movepos_count;
	int stay_count;
	int hide_count;
	int wait_raiden_count;
	int sleep_count;
	int nodamage_count;
	int behind_raiden_count;

	int capture_count;

	/* 汎用カウンター */
	int gcounter;

	/* 現在のモーションの状態 */
	unsigned int snake_status;
	unsigned int snake_status2;

	/* オプション設定 */
	int support_level,support_level2;

	NPCSNAKE_SUPPORT_POINT *support_points;
	int n_support_points;

	void *usp;
	void *fms;
	void *psg;
	void *magazines;

	/* 現在再生しているモーションの情報 */
	int pad;

	/* 上半身の再生モーション(通常はNONE、移動しながら射撃等への対応のため) */
	int sub_pad;
	int sub_pad_time;
	void (*SubMotionAct)(NPCWORK *npc,int time);

	/* 以前の補正クォータニオン
	   CalcAdjust用 */
	FVECTOR pre_adjust0,pre_adjust1,pre_adjust2;

	/* 初期値 */
	FVECTOR pos,target_pos;
	SVECTOR rot;

	int mdlfile;
	int mdlfile2;
	int marfile;
	int cap_marfile;
	int drag_marfile;
	int farfile;

	int life_max;

	/* 'w44a'  --> 0
	   'w45a'  --> 1
	   'STAGE' --> 2
	   'w14a'  --> 3
	   'w17a'  --> 4
	   'w32a'  --> 5
	   'DUMMY' --> 6 */
	int stage;

	/* w44aでのスネークの行動を決定する変数 */
	int behind_to_independ_interval;
	float independ_to_search_dis;
	int search_to_pickup_interval;
	int hidding_to_peeping_interval;
	int hidding_to_peeping_interval2;
	int peeping_to_move_interval;
	int move_to_shoot_interval;
	int capture_wait_interval;

	int capture_proc;

	int w14a_count;

	float w17a_dis;

	/* w17a、w32aでヘッドマークを出した回数 */
	int headmark_count;
	FMATRIX headmark_mat;

	/* w32aで、最初の狙撃までの時間と狙撃と狙撃の間のインターバル */
	int w32a_snipe_first_time;
	int w32a_snipe_interval_time;

	/* w32aで狙撃を依頼した回数 */
	int w32a_request_count;
	int w32a_start_sleeping_proc;
	int w32a_end_sleeping_proc;

	/* w45a用の亀甲フロアエフェクト */
	int hex_floor_flag;
	HEX_FLOOR_INFO	hex_floor;

	int life_proc;
	int end_proc;
	int gameover_proc;

	int ret_flag;

	int breath_interval;
	int breath_count;

	/* 撃った後一秒ほど残心の構えをするようにする。*/
	int aftershoot_count;

	int zzz_sound_count;
	int voice_num;
	int voice_handle;
	int voice_index;

	/* 足影用 */
	int foot_shadow;

	/* スネークの状態 : commandを利用してシナリオ側で受け取る値 */
	short scenario_snake_status;

	/* まばたき用 */
	short eye_flush_count;

	/* スプレーへの対応用 */
	short spray_count;

	/* ローリングを出す条件 */
	short roll_count;
	short roll_counter;

	/* アイテムとアイテムが重ならないように時間差で出す */
	short throwitem_count;

	FVECTOR sight_min,sight_max;

	/* ダメージ時限定の移動速度 */
	FVECTOR damage_mov_v;

	/* 雑誌の位置 */
	FVECTOR book_pos;
	short book_interval;
	short book_count;

	/* アイテムを出す数の制限 */
	short ration_count;
	short usp_mag_count;
	short m4_mag_count;
	short rgb6_mag_count;
} Work;


extern Work *npcsnake_work;


/* npc_common.c */
void InitNPC(Work *work);
void ExitNPC(Work *work);
void ActNPC(Work *work);


/* snake_main.c */
void NPCSnake_CallGameoverProc(Work *work);
int NPCSnake_SearchPart(Work *work);
void NPCSnake_SetStage(Work *work,int stage);
void InitNPCSnake(Work *work);
void ActNPCSnake(Work *work);
void ExitNPCSnake(Work *work);


/* snake_situation.c */
void GetSituation_SEARCHENEMY(Work *work);
void GetSituation_INDEPEND(Work *work);
void GetSituation_FOLLOW_RAIDEN(Work *work);
void GetSituation_BEHIND_RAIDEN(Work *work);
void GetSituation_PICKUP_RAIDEN(Work *work);
void GetSituation_W44AFinal(Work *work);
void GetSituation_W45A(Work *work);
void GetSituation_BETWEEN_STAGES(Work *work);
void GetSituation_W14A(Work *work);
void GetSituation_W17A(Work *work);
void GetSituation_W32A(Work *work);
void GetSituation_DUMMY(Work *work);

void GetDisToRaiden(Work *work);


/* snake_decide.c */
void DecideAction_SEARCHENEMY(Work *work);
void DecideAction_INDEPEND(Work *work);
void DecideAction_FOLLOW_RAIDEN(Work *work);
void DecideAction_BEHIND_RAIDEN(Work *work);
void DecideAction_PICKUP_RAIDEN(Work *work);
void DecideAction_GURD_RAIDEN(Work *work);
void DecideAction_GURD_RAIDEN_W44A(Work *work);
void DecideAction_ATTACK_ENEMY(Work *work);
void DecideAction_BETWEEN_STAGES(Work *work);
void DecideAction_W14A(Work *work);
void DecideAction_W17A(Work *work);
void DecideAction_W32A(Work *work);
void DecideAction_DUMMY(Work *work);
void DecideAction_W43A(Work *work);


/* snake_act.c */
int ActPreMove(Work *work,NPCWORK *npc);
int ActPreMoveEx(Work *work,NPCWORK *npc);
int NPCSnake_NaviNear(Work *work,NAVIGATE *navi,NAVITARGET *nvtrg,int dis);


/* snake_pad.c */
void NPCSnake_GetPlayerTargetPos(NPCWORK *npc);
int NPCSnake_CheckPadAndSetMotion(NPCWORK *npc);
void ActMotion_Stand(NPCWORK *npc,int time);
void ActMotion_StandW17A(NPCWORK *npc,int time);
void ActLoopMotionWLockW14A(NPCWORK *npc,int time);

void NPCSnake_DefTargetCallback(TARGET *off,TARGET *def,void *_work);

int NPCSnake_CheckRaidenDisForFaint(NPCWORK *npc);
int NPCSnake_CheckRaidenDisForPush(NPCWORK *npc,FVECTOR *v);
int NPCSnake_CheckDamage(NPCWORK *npc);
int NPCSnake_CheckDamageBETWEENSTAGES(NPCWORK *npc);
int NPCSnake_CheckDamageW14A(NPCWORK *npc);
int NPCSnake_CheckDamageW17A(NPCWORK *npc);
int NPCSnake_CheckDamageW32A(NPCWORK *npc);

void NPCSnake_CheckPlayerCapture(Work *work);

void ActSubMotion(Work *work);
int ActSubMotionReset(Work *work);
int ActSubMotionLock(Work *work);
int ActSubMotionUnlock(Work *work);
int StartSubMotion_BeginUSP(Work *work);
int StartSubMotion_ShootUSP(Work *work);
int StartSubMotion_BeginFMS(Work *work);
int StartSubMotion_ShootFMS(Work *work);


/* snake_life.c */
void *NewNPCSnakeLife(NPCWORK *npc,int max);


/* snake_weapon.c */
void *NPCSnakeInitUsp(Work *work);
void *NPCSnakeInitFms(Work *work);
void *NPCSnakeInitPsg(Work *work);
void NPCSnakeVisibleWeapon(void *objs);
void NPCSnakeInvisibleWeapon(void *objs);
void NPCSnakeShootUsp(Work *work,void *objs,FVECTOR * const target);
void NPCSnakeShootUspP(Work *work,void *objs,FVECTOR * const target);
void NPCSnakeShootFms(Work *work,void *objs,FVECTOR * const target);
void NPCSnakeShootFmsP(Work *work,void *objs,FVECTOR * const target);
void NPCSnakeShootPsg(Work *work,void *objs,FVECTOR * const target,int flag);
void NPCSnakeEndWeapon(void *objs);
void NPCSnakeActWeapon(void *objs);

void NPCSnake_CalcAdjust0(Work *work);
void NPCSnake_CalcAdjust(Work *work);
void NPCSnake_BackAdjust(Work *work);
void NPCSnake_CalcAdjustW14A(Work *work);
void NPCSnake_CalcAdjustW32A(Work *work,int flag);
void NPCSnake_ClearAdjustW43A(Work *work);
void NPCSnake_CalcAdjustW43A(Work *work);
void NPCSnake_BackAdjustW43A(Work *work);
void NPCSnake_CheckBook(Work *work);

void NPCSnake_SetRadarDir(Work *work);

void NPCSnake_ThrowItem(Work *work);

void NPCSNAKE_BreathControl(Work *work);
void NPCSNAKE_ZZZSound(Work *work);
void NPCSNAKE_EvmCloseEye(Work *work);

void NPCSnake_VoiceStop(Work *work);
void NPCSnake_VoiceControl(Work *work);


/* snake_w45a.c */
int GetSupportPosW44AFinal(Work *work);
int GetSupportPosW45A(Work *work);
void GetNeighborHzxW45A(Work *work);
int ActMoveAroundRaiden(Work *work);
int ActMoveAroundRaiden2(Work *work);
int ActMoveFlwRaidenOnly(Work *work);
int ActPointTarget(Work *work);
int ActShootRaiden(Work *work);
int ActAngry(Work *work);


/* snake_w32a.c */
void NPCSnake_CheckInSight(Work *work);
void NPCSnake_DecideTarget(Work *work);
int NPCSnake_SearchHomingAfterShoot(Work *work);
void ActWaitW32A0(Work *work);
void ActWaitW32A(Work *work);
void ActShootW32A(Work *work);
void ActPreCounterW32A(Work *work);
void ActCounterW32A(Work *work);


/* source/user/korekado/npc/npc.c */   
void NPC_SetDogtag( NPCWORK *npc, int model, char c );

/* source/user/takabe/effect1/fanim.c
   表情アニメーション関数 */
void *NewFaceAnimation( int name, DG_EVMOBJ *evmobj, int fanim_id );

/* ??? */
void *NewBreath(FMATRIX *world,FVECTOR *shift,int *sw);


#define POS_FLAG_HIDEPOS_INDEX(x)		((x)>>16)

#define NPCSNAKE_ACT_MODE(work)			((work)->action_num & NPCSNAKE_ACT_MASK)
#define NPCSNAKE_ACT_RESET(work)		((work)->action_num&=~NPCSNAKE_ACT_MASK)
#define NPCSNAKE_ACT_SET(work,mode) \
	((work)->action_num=(((work)->action_num & ~NPCSNAKE_ACT_MASK)|(mode)))

#define NPCSNAKE_SHOOT_MODE(work)		((work)->action_num & NPCSNAKE_SHOOT_MASK)
#define NPCSNAKE_SHOOT_RESET(work)		((work)->action_num&=~NPCSNAKE_SHOOT_MASK)
#define NPCSNAKE_SHOOT_SET(work,mode) \
	(((work)->action_num=(((work)->action_num & ~NPCSNAKE_SHOOT_MASK)|(mode))), \
	 (work)->shoot_interval_count=0)

#define NPCSNAKE_HIDE_MODE(work)		((work)->action_num & NPCSNAKE_HIDE_MASK)
#define NPCSNAKE_HIDE_RESET(work)		((work)->action_num&=~NPCSNAKE_HIDE_MASK)
#define NPCSNAKE_HIDE_SET(work,mode) \
	((work)->action_num=(((work)->action_num & ~NPCSNAKE_HIDE_MASK)|(mode)))



#define ACCESS_WORK(_npc)		((Work *)((_npc)->character))

#if 0
#define ChangeNewPad(_npc,_pad)	((_npc)->action.pad=(_pad),(_npc)->CheckPad(_npc))
#else
#define ChangeNewPad(_npc,_pad)	((_npc)->action.pad=(_pad),NPCSnake_CheckPadAndSetMotion(_npc))
#endif


#define NPCSNAKE_SLEEP_COUNTSET(_npc,_d) \
	(ACCESS_WORK(_npc)->sleep_count=(_d))
#define NPCSNAKE_SLEEP_COUNTDOWN(_npc) \
	(ACCESS_WORK(_npc)->sleep_count--)
#define NPCSNAKE_SLEEP_COUNTCHECK(_npc) \
	(ACCESS_WORK(_npc)->sleep_count<0)

#define NPCSNAKE_NODAMAGE_COUNTSET(_npc,_d) \
	(ACCESS_WORK(_npc)->nodamage_count=(_d))
#define NPCSNAKE_NODAMAGE_COUNTDOWN(_npc) \
	(ACCESS_WORK(_npc)->nodamage_count>0 ? ACCESS_WORK(_npc)->nodamage_count-- : 0)
#define NPCSNAKE_NODAMAGE_COUNTCHECK(_npc) \
	(ACCESS_WORK(_npc)->nodamage_count>0)

#define MOTION_CANCELABLE_ON(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_CANCELABLE)
#define MOTION_CANCELABLE_OFF(_npc) \
	(ACCESS_WORK(_npc)->snake_status&=~SNAKE_STATUS_CANCELABLE)
#define MOTION_CANCELABLE_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_CANCELABLE)


#define ACTION_FINISHED(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_ACTION_FINISHED)

#define SHOOT_ACTION_FINISHED(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED)

#define CHECK_ACTION_FINISH_FLAG(x) \
	((x)->snake_status & SNAKE_STATUS_ACTION_FINISHED)

#define CHECK_SHOOT_FINISH_FLAG(x) \
	((x)->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)

#define CHECK_SHOOT_AND_ACT_FINISH_FLAG(x) \
	(((x)->snake_status & (SNAKE_STATUS_ACTION_FINISHED|SNAKE_STATUS_SHOOT_ACTION_FINISHED)) \
	 ==(SNAKE_STATUS_ACTION_FINISHED|SNAKE_STATUS_SHOOT_ACTION_FINISHED))


#define LOCKON_ENEMY(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_LOCKON_ENEMY)
#define UNLOCK_ENEMY(_npc) \
	(ACCESS_WORK(_npc)->snake_status&=~SNAKE_STATUS_LOCKON_ENEMY)
#define LOCKON_ENEMY_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_LOCKON_ENEMY)

#define SITTING_MOTION(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_SITTING_MOTION)
#define STANDING_MOTION(_npc) \
	(ACCESS_WORK(_npc)->snake_status&=~SNAKE_STATUS_SITTING_MOTION)
#define SITTING_MOTION_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_SITTING_MOTION)

#define WALKING_MOTION(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_WALKING_MOTION)
#define NOT_WALKING_MOTION(_npc) \
	(ACCESS_WORK(_npc)->snake_status&=~SNAKE_STATUS_WALKING_MOTION)
#define WALKING_MOTION_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_WALKING_MOTION)

#define MOVING_MOTION(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_MOVING_MOTION)
#define NOT_MOVING_MOTION(_npc) \
	(ACCESS_WORK(_npc)->snake_status&=~SNAKE_STATUS_MOVING_MOTION)
#define MOVING_MOTION_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_MOVING_MOTION)

#define HAS_NONE_MOTION(_npc) \
	(ACCESS_WORK(_npc)->snake_status&=~(SNAKE_STATUS_HAS_USP|SNAKE_STATUS_HAS_FMS))
#define HAS_USP_MOTION(_npc) \
	(HAS_NONE_MOTION(_npc),ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_HAS_USP)
#define HAS_FMS_MOTION(_npc) \
	(HAS_NONE_MOTION(_npc),ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_HAS_FMS)
#define HAS_PSG_MOTION(_npc) \
	(HAS_NONE_MOTION(_npc),ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_HAS_PSG)
#define HAS_USP_MOTION_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_HAS_USP)
#define HAS_FMS_MOTION_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_HAS_FMS)
#define HAS_PSG_MOTION_FLAG(_npc) \
	(ACCESS_WORK(_npc)->snake_status & SNAKE_STATUS_HAS_PSG)


#define ZZZSOUND_ON(_work) \
	((_work)->snake_status2|=SNAKE_STATUS2_ZZZ_SOUND)

#define ZZZSOUND_OFF(_work) \
	((_work)->snake_status2&=~SNAKE_STATUS2_ZZZ_SOUND)


#ifdef __GNUC__

#ifdef DEBUG_MODE
#define DEBUG_PRINT_NPCSNAKE(_s...) printf("NPC_SNAKE : " ## _s )
#else
#define DEBUG_PRINT_NPCSNAKE(_s...)
#endif

#else

#ifdef DEBUG_MODE
#define DEBUG_PRINT_NPCSNAKE printf
#else
#define DEBUG_PRINT_NPCSNAKE
#endif

#endif


/* 音声のリソース名 */
#define NPCSNAKE_VOICE_RESOURCE			0x0025f5d8		/* ＮＰＣスネーク音声 */


void NPCSNAKE_VOICE_CALL(void *work, int id);
void NPCSNAKE_VOICE_RNDCALL(void *work, int *ids, int size);


#define NPCSNAKE_VOICE_STOP(work)		NPCSnake_VoiceStop(work)


#endif
