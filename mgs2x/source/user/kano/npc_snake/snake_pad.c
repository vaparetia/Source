//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_pad.c
		NPCスネークPADルーチン

	2001/03/06 K.Kano
	$Id: snake_pad.c,v 1.1.1.3 2002/11/19 11:43:22 Yoshizawa1 Exp $
 */


#include "npc_snake.h"
#include "rand.h"


#include "snake_pad2.c"


#define PUNCH_DAMAGE_POINT(_npc)			5
#define KICK_DAMAGE_POINT(_npc)				25
#define BULLET_DAMAGE_POINT(_npc)			40
#define KNOCKDOWN_DAMAGE_POINT(_npc)		80
#define THROW_DAMAGE_POINT(_npc)			10

#define KNOCKDOWN_FAINT_POINT(_npc)			2


#define PUNCH_FAINT_POINT			1
#define KICK_FAINT_POINT			3
#define THROW_FAINT_POINT			5
#define GUN_PUNCH_FAINT_POINT		2
#define BLADE_FAINT_POINT			1
#define NIKITA_PUNCH_FAINT_POINT	100
#define M4_PUNCH_FAINT_POINT		3

#define STUNGRENADE_FAINT_POINT		100
#define BLOW_FAINT_POINT			5


/* ライデンへの振動のデータ */
#define VIBDATA_BLADE			0x00677a2a		/* rai_katana_01 */
#define VIBDATA_BLADE_STAB		0x00677a2b		/* rai_katana_02 */
#define VIBDATA_BLADE_FAINT		0x00677a2c		/* rai_katana_03 */

#define VIBDATA_KICK			0x00f3179d		/* rai_kick_01 */
#define VIBDATA_2ND_KICK		0x00f3179e		/* rai_kick_02 */

#define VIBDATA_PUNCH			0x00e85ae4		/* rai_punch_01 */
#define VIBDATA_PUNCH_GUN		0x00e85ae5		/* rai_punch_02 */
#define VIBDATA_PUNCH_NIKITA	0x00e85ae6		/* rai_punch_03 */

#define VIBDATA_CAPTURE			0x00f3de01		/* sna_npc_m4a_iya4rai_resist */


/* -------------------------------------------------------------------- */
/* Check Motion Status                                                  */
/* -------------------------------------------------------------------- */

static void CheckWalkingMotion(NPCWORK *npc,NPCACT *act)
{
	switch(act->pad){
	case PAD_WALK:
	case PAD_WALK_WUSP:
	case PAD_WALK_WFMS:
		WALKING_MOTION(npc);
		MOVING_MOTION(npc);
		break;
	case PAD_RUN:
	case PAD_RUN_WUSP:
	case PAD_RUN_WFMS:

	case PAD_TUMBLE_START:
	case PAD_TUMBLE_START_WUSP:
	case PAD_TUMBLE_START_WFMS:
	case PAD_TUMBLE_END:
	case PAD_TUMBLE_END_WUSP:
	case PAD_TUMBLE_END_WFMS:
		NOT_WALKING_MOTION(npc);
		MOVING_MOTION(npc);
		break;
	default:
		NOT_WALKING_MOTION(npc);
		NOT_MOVING_MOTION(npc);
		break;
	}
}

static void CheckSittingMotion(NPCWORK *npc,NPCACT *act)
{
	switch(act->pad){
	case PAD_SITTING:
	case PAD_SITTING_WUSP:
	case PAD_SITTING_WFMS:

	case PAD_SITDOWN:
	case PAD_SITDOWN_WUSP:
	case PAD_SITDOWN_WFMS:

	case PAD_SITTING_AND_START_SHOOT_USP:
	case PAD_SITTING_AND_SHOOT_USP:
	case PAD_SITTING_AND_START_SHOOT_FMS:
	case PAD_SITTING_AND_SHOOT_FMS:

	case PAD_BEHIND_AND_SITTING:
	case PAD_START_BEHIND_AND_SITTING:
	case PAD_END_BEHIND_AND_SITTING:

	case PAD_BEHIND_AND_SITTING_WUSP:
	case PAD_START_BEHIND_AND_SITTING_WUSP:
	case PAD_END_BEHIND_AND_SITTING_WUSP:

	case PAD_BEHIND_AND_SITTING_WFMS:
	case PAD_START_BEHIND_AND_SITTING_WFMS:
	case PAD_END_BEHIND_AND_SITTING_WFMS:

	case PAD_START_LOOK_L_BEHIND_AND_SITTING:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING:
	case PAD_LOOK_L_BEHIND_AND_SITTING:
	case PAD_LOOK_R_BEHIND_AND_SITTING:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING:

	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WUSP:
	case PAD_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_LOOK_R_BEHIND_AND_SITTING_WUSP:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING_WUSP:

	case PAD_START_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:

	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WFMS:
	case PAD_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_LOOK_R_BEHIND_AND_SITTING_WFMS:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING_WFMS:

	case PAD_START_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:

	case PAD_BRINGING_USP_AND_SITTING:
	case PAD_BRINGING_FMS_AND_SITTING:
		NPC_ChangeTargetSize(npc,NPCSNAKE_TARGET_SITTING);
		SITTING_MOTION(npc);
		break;

	case PAD_SITTING_AND_SLEEP_WM4A1:
	case PAD_SITTING_AND_BRINGING_WM4A1:
	case PAD_SITTING_AND_START_SHOOT_WM4A1:
	case PAD_SITTING_AND_SHOOT_WM4A1:

	case PAD_SITTING_AND_DAMAGED:
	case PAD_SITTING_AND_M9SLEEP:
		NPC_ChangeTargetSize(npc,NPCSNAKE_TARGET_SITTING_W14A);
		SITTING_MOTION(npc);
		break;

	case PAD_CROUCHON:
	case PAD_CROUCHING:
	case PAD_CROUCHING_WPSG:
	case PAD_DAMAGE_CROUCHING:

	case PAD_DAMAGE_SLEEP_F:
	case PAD_DAMAGE_SLEEP_B:
		NPC_ChangeTargetSize(npc,NPC_TARGET_SIZE_DOWN);
		SITTING_MOTION(npc);
		break;

	case PAD_BEHIND:

	case PAD_START_LOOK_L_BEHIND:
	case PAD_START_LOOK_R_BEHIND:
	case PAD_LOOK_L_BEHIND:
	case PAD_LOOK_R_BEHIND:
	case PAD_END_LOOK_L_BEHIND:
	case PAD_END_LOOK_R_BEHIND:

	case PAD_BEHIND_WUSP:

	case PAD_START_LOOK_L_BEHIND_WUSP:
	case PAD_START_LOOK_R_BEHIND_WUSP:
	case PAD_LOOK_L_BEHIND_WUSP:
	case PAD_LOOK_R_BEHIND_WUSP:
	case PAD_END_LOOK_L_BEHIND_WUSP:
	case PAD_END_LOOK_R_BEHIND_WUSP:

	case PAD_START_SHOOT_USP_L_FROM_BEHIND:
	case PAD_END_SHOOT_USP_L_FROM_BEHIND:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND:
	case PAD_END_SHOOT_USP_R_FROM_BEHIND:

	case PAD_BEHIND_WFMS:

	case PAD_START_LOOK_L_BEHIND_WFMS:
	case PAD_START_LOOK_R_BEHIND_WFMS:
	case PAD_LOOK_L_BEHIND_WFMS:
	case PAD_LOOK_R_BEHIND_WFMS:
	case PAD_END_LOOK_L_BEHIND_WFMS:
	case PAD_END_LOOK_R_BEHIND_WFMS:

	case PAD_START_SHOOT_FMS_L_FROM_BEHIND:
	case PAD_END_SHOOT_FMS_L_FROM_BEHIND:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND:
	case PAD_END_SHOOT_FMS_R_FROM_BEHIND:
		NPC_ChangeTargetSize(npc,NPCSNAKE_TARGET_BEHIND);
		STANDING_MOTION(npc);
		break;

	default:
		NPC_ChangeTargetSize(npc,NPC_TARGET_SIZE_STAND);
		STANDING_MOTION(npc);
		break;
	}
}

static void CheckMotionWithWeapon(NPCWORK *npc,NPCACT *act)
{
	switch(act->pad){
	case PAD_STAND:
	case PAD_RUN:
	case PAD_WALK:
	case PAD_STANDUP:
	case PAD_SITDOWN:
	case PAD_SITTING:
	case PAD_BEHIND:
	case PAD_BEHIND_AND_SITTING:
	case PAD_START_LOOK_L_BEHIND:
	case PAD_START_LOOK_R_BEHIND:
	case PAD_LOOK_L_BEHIND:
	case PAD_LOOK_R_BEHIND:
	case PAD_END_LOOK_L_BEHIND:
	case PAD_END_LOOK_R_BEHIND:

	case PAD_CROUCHON:
	case PAD_CROUCHING:

	case PAD_PUNCH:

	case PAD_TUMBLE_START:
	case PAD_TUMBLE_END:
		HAS_NONE_MOTION(npc);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->fms);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->psg);
		break;

	case PAD_STAND_WUSP:
	case PAD_RUN_WUSP:
	case PAD_WALK_WUSP:
	case PAD_STANDUP_WUSP:
	case PAD_SITDOWN_WUSP:
	case PAD_SITTING_WUSP:
	case PAD_BEHIND_WUSP:
	case PAD_BEHIND_AND_SITTING_WUSP:
	case PAD_START_LOOK_L_BEHIND_WUSP:
	case PAD_START_LOOK_R_BEHIND_WUSP:
	case PAD_LOOK_L_BEHIND_WUSP:
	case PAD_LOOK_R_BEHIND_WUSP:
	case PAD_END_LOOK_L_BEHIND_WUSP:
	case PAD_END_LOOK_R_BEHIND_WUSP:

	case PAD_START_SHOOT_USP:
	case PAD_SHOOT_USP:
	case PAD_SITTING_AND_START_SHOOT_USP:
	case PAD_SITTING_AND_SHOOT_USP:

	case PAD_START_SHOOT_USP_L_FROM_BEHIND:
	case PAD_END_SHOOT_USP_L_FROM_BEHIND:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND:
	case PAD_END_SHOOT_USP_R_FROM_BEHIND:
	case PAD_START_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:

	case PAD_BRINGING_USP:
	case PAD_BRINGING_USP_AND_SITTING:

	case PAD_PUNCH_WUSP:

	case PAD_TUMBLE_START_WUSP:
	case PAD_TUMBLE_END_WUSP:
		HAS_USP_MOTION(npc);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->fms);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->psg);
		break;

	case PAD_STAND_WFMS:
	case PAD_RUN_WFMS:
	case PAD_WALK_WFMS:
	case PAD_STANDUP_WFMS:
	case PAD_SITDOWN_WFMS:
	case PAD_SITTING_WFMS:
	case PAD_BEHIND_WFMS:
	case PAD_BEHIND_AND_SITTING_WFMS:
	case PAD_START_LOOK_L_BEHIND_WFMS:
	case PAD_START_LOOK_R_BEHIND_WFMS:
	case PAD_LOOK_L_BEHIND_WFMS:
	case PAD_LOOK_R_BEHIND_WFMS:
	case PAD_END_LOOK_L_BEHIND_WFMS:
	case PAD_END_LOOK_R_BEHIND_WFMS:

	case PAD_START_SHOOT_FMS:
	case PAD_SHOOT_FMS:
	case PAD_SITTING_AND_START_SHOOT_FMS:
	case PAD_SITTING_AND_SHOOT_FMS:

	case PAD_START_SHOOT_FMS_L_FROM_BEHIND:
	case PAD_END_SHOOT_FMS_L_FROM_BEHIND:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND:
	case PAD_END_SHOOT_FMS_R_FROM_BEHIND:
	case PAD_START_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:

	case PAD_BRINGING_FMS:
	case PAD_BRINGING_FMS_AND_SITTING:

	case PAD_PUNCH_WFMS:

	case PAD_SITTING_AND_SLEEP_WM4A1:
	case PAD_SITTING_AND_BRINGING_WM4A1:
	case PAD_SITTING_AND_START_SHOOT_WM4A1:
	case PAD_SITTING_AND_SHOOT_WM4A1:

	case PAD_SITTING_AND_DAMAGED:

	case PAD_TUMBLE_START_WFMS:
	case PAD_TUMBLE_END_WFMS:
		HAS_FMS_MOTION(npc);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->fms);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->psg);
		break;

	case PAD_CROUCHING_WPSG:
	case PAD_SHOOT_PSG:
		HAS_PSG_MOTION(npc);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->fms);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->psg);
		break;
	}
}

static void CheckBreathMotion(NPCWORK *npc,NPCACT *act)
{
	switch(act->pad){
	case PAD_STAND:
	case PAD_SITTING:
	case PAD_STANDUP:
	case PAD_SITDOWN:
	case PAD_STAND_WUSP:
	case PAD_SITTING_WUSP:
	case PAD_STANDUP_WUSP:
	case PAD_SITDOWN_WUSP:
	case PAD_STAND_WFMS:
	case PAD_SITTING_WFMS:
	case PAD_STANDUP_WFMS:
	case PAD_SITDOWN_WFMS:

	case PAD_BEHIND:
	case PAD_BEHIND_AND_SITTING:
	case PAD_BEHIND_WUSP:
	case PAD_BEHIND_AND_SITTING_WUSP:
	case PAD_BEHIND_WFMS:
	case PAD_BEHIND_AND_SITTING_WFMS:
		ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_AUTO_BREATH;
		break;
	default:
		ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_AUTO_BREATH;
		break;
	}
}

int NPCSnake_CheckPadAndSetMotion(NPCWORK *npc)
{
	NPCACT *act;

	if(!MOTION_CANCELABLE_FLAG(npc)) return 0;

	act=&npc->action;

	if(act->pad==PAD_NONE) return 0;

	npc->body->m_ctrl->adjust_flag=0;

	CheckWalkingMotion(npc,act);
	CheckSittingMotion(npc,act);
	CheckMotionWithWeapon(npc,act);
	CheckBreathMotion(npc,act);


	switch(act->pad){
		 /* 立ち、歩き、走り */
	case PAD_STAND:
	case PAD_STAND_WUSP:
	case PAD_STAND_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_Stand,npc->base_mar,act->pad,act->pad);
		break;
	case PAD_RUN:
	case PAD_RUN_WUSP:
	case PAD_RUN_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_Move,npc->base_mar,act->pad,act->pad);
		break;
	case PAD_WALK:
	case PAD_WALK_WUSP:
	case PAD_WALK_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_Move,npc->base_mar,act->pad,act->pad);
		break;

		/* 座り続ける */
	case PAD_SITTING:
	case PAD_SITTING_WUSP:
	case PAD_SITTING_WFMS:
		NPC_SetModeFromPad(npc,ActSittingMotion,npc->base_mar,act->pad,act->pad);
		break;

		/* ビハインドモードでの動作 */
	case PAD_BEHIND:
	case PAD_BEHIND_AND_SITTING:
	case PAD_BEHIND_WUSP:
	case PAD_BEHIND_AND_SITTING_WUSP:
	case PAD_BEHIND_WFMS:
	case PAD_BEHIND_AND_SITTING_WFMS:
		NPC_SetModeFromPad(npc,ActLoopMotionWLock,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		SHOOT_ACTION_FINISHED(npc);
		break;

	case PAD_LOOK_L_BEHIND:
	case PAD_LOOK_R_BEHIND:
	case PAD_LOOK_L_BEHIND_AND_SITTING:
	case PAD_LOOK_R_BEHIND_AND_SITTING:
	case PAD_LOOK_L_BEHIND_WUSP:
	case PAD_LOOK_R_BEHIND_WUSP:
	case PAD_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_LOOK_R_BEHIND_AND_SITTING_WUSP:
	case PAD_LOOK_L_BEHIND_WFMS:
	case PAD_LOOK_R_BEHIND_WFMS:
	case PAD_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_LOOK_R_BEHIND_AND_SITTING_WFMS:
		NPC_SetModeFromPad(npc,ActLoopMotionWLock,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* しゃがみこみ、立ち上がり */
	case PAD_SITDOWN:
		NPC_SetModeFromPad(npc,ActMotion_Sitdown,npc->base_mar,
						   PAD_SITTING,PAD_SITDOWN);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITDOWN_WUSP:
		NPC_SetModeFromPad(npc,ActMotion_Sitdown,npc->base_mar,
						   PAD_SITTING_WUSP,PAD_SITDOWN_WUSP);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITDOWN_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_Sitdown,npc->base_mar,
						   PAD_SITTING_WFMS,PAD_SITDOWN_WFMS);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_STANDUP:
		NPC_SetModeFromPad(npc,ActMotion_Standup,npc->base_mar,
						   PAD_STAND,PAD_STANDUP);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_STANDUP_WUSP:
		NPC_SetModeFromPad(npc,ActMotion_Standup,npc->base_mar,
						   PAD_STAND_WUSP,PAD_STANDUP_WUSP);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_STANDUP_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_Standup,npc->base_mar,
						   PAD_STAND_WFMS,PAD_STANDUP_WFMS);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* ビハインドモードへの移行モーション */
	case PAD_START_BEHIND:
		NPC_SetModeFromPad(npc,ActMotion_StartBehind,npc->base_mar,
						   PAD_BEHIND,PAD_START_BEHIND);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_START_BEHIND_WUSP:
		NPC_SetModeFromPad(npc,ActMotion_StartBehind,npc->base_mar,
						   PAD_BEHIND_WUSP,PAD_START_BEHIND_WUSP);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_START_BEHIND_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_StartBehind,npc->base_mar,
						   PAD_BEHIND_WFMS,PAD_START_BEHIND_WFMS);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_START_BEHIND_AND_SITTING:
		NPC_SetModeFromPad(npc,ActMotion_StartBehind,npc->base_mar,
						   PAD_BEHIND_AND_SITTING,PAD_START_BEHIND_AND_SITTING);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_START_BEHIND_AND_SITTING_WUSP:
		NPC_SetModeFromPad(npc,ActMotion_StartBehind,npc->base_mar,
						   PAD_BEHIND_AND_SITTING_WUSP,PAD_START_BEHIND_AND_SITTING_WUSP);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_START_BEHIND_AND_SITTING_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_StartBehind,npc->base_mar,
						   PAD_BEHIND_AND_SITTING_WFMS,PAD_START_BEHIND_AND_SITTING_WFMS);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* ビハインドモードからの移行モーション */
	case PAD_END_BEHIND:
		NPC_SetModeFromPad(npc,ActMotion_EndBehind,npc->base_mar,
						   PAD_STAND,PAD_END_BEHIND);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_END_BEHIND_WUSP:
		NPC_SetModeFromPad(npc,ActMotion_EndBehind,npc->base_mar,
						   PAD_STAND_WUSP,PAD_END_BEHIND_WUSP);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_END_BEHIND_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_EndBehind,npc->base_mar,
						   PAD_STAND_WFMS,PAD_END_BEHIND_WFMS);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_END_BEHIND_AND_SITTING:
		NPC_SetModeFromPad(npc,ActMotion_EndBehind,npc->base_mar,
						   PAD_SITTING,PAD_END_BEHIND_AND_SITTING);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_END_BEHIND_AND_SITTING_WUSP:
		NPC_SetModeFromPad(npc,ActMotion_EndBehind,npc->base_mar,
						   PAD_SITTING_WUSP,PAD_END_BEHIND_AND_SITTING_WUSP);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_END_BEHIND_AND_SITTING_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_EndBehind,npc->base_mar,
						   PAD_SITTING_WFMS,PAD_END_BEHIND_AND_SITTING_WFMS);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

	case PAD_START_LOOK_L_BEHIND:
	case PAD_END_LOOK_L_BEHIND:
	case PAD_START_LOOK_R_BEHIND:
	case PAD_END_LOOK_R_BEHIND:

	case PAD_START_LOOK_L_BEHIND_AND_SITTING:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING:

	case PAD_START_LOOK_L_BEHIND_WUSP:
	case PAD_END_LOOK_L_BEHIND_WUSP:
	case PAD_START_LOOK_R_BEHIND_WUSP:
	case PAD_END_LOOK_R_BEHIND_WUSP:

	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WUSP:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING_WUSP:

	case PAD_START_LOOK_L_BEHIND_WFMS:
	case PAD_END_LOOK_L_BEHIND_WFMS:
	case PAD_START_LOOK_R_BEHIND_WFMS:
	case PAD_END_LOOK_R_BEHIND_WFMS:

	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WFMS:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_StartEndLookBehind,npc->base_mar,
						   act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* 射撃 */
	case PAD_START_SHOOT_USP:
		NPC_SetModeFromPad(npc,ActMotion_BeginUSP,npc->base_mar,act->pad,act->pad);
		break;
	case PAD_START_SHOOT_USP_L_FROM_BEHIND:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND:
	case PAD_START_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:
		NPC_SetModeFromPad(npc,ActMotion_BeginUSPFromBehind,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_START_SHOOT_FMS:
		NPC_SetModeFromPad(npc,ActMotion_BeginFMS,npc->base_mar,act->pad,act->pad);
		break;
	case PAD_START_SHOOT_FMS_L_FROM_BEHIND:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND:
	case PAD_START_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:
		NPC_SetModeFromPad(npc,ActMotion_BeginFMSFromBehind,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

	case PAD_END_SHOOT_USP_L_FROM_BEHIND:
	case PAD_END_SHOOT_USP_R_FROM_BEHIND:
	case PAD_END_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_FMS_L_FROM_BEHIND:
	case PAD_END_SHOOT_FMS_R_FROM_BEHIND:
	case PAD_END_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
	case PAD_END_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:
		NPC_SetModeFromPad(npc,ActMotion_EndShootFromBehind,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

	case PAD_SHOOT_USP:
		NPC_SetModeFromPad(npc,ActMotion_ShootUSP,npc->base_mar,act->pad,act->pad);
		break;
	case PAD_SHOOT_FMS:
		NPC_SetModeFromPad(npc,ActMotion_ShootFMS,npc->base_mar,act->pad,act->pad);
		break;

		/* 座りながら射撃 */
	case PAD_SITTING_AND_START_SHOOT_USP:
		NPC_SetModeFromPad(npc,ActMotion_SittingAndBeginUSP,
						   npc->base_mar,PAD_SITTING,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_START_SHOOT_FMS:
		NPC_SetModeFromPad(npc,ActMotion_SittingAndBeginFMS,
						   npc->base_mar,PAD_SITTING,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_SHOOT_USP:
		NPC_SetModeFromPad(npc,ActMotion_SittingAndShootUSP,
						   npc->base_mar,PAD_SITTING,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_SHOOT_FMS:
		NPC_SetModeFromPad(npc,ActMotion_SittingAndShootFMS,
						   npc->base_mar,PAD_SITTING,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

	case PAD_BRINGING_USP:
		NPC_SetModeFromPad(npc,ActBringingMotion,npc->base_mar,PAD_START_SHOOT_USP,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_BRINGING_FMS:
		NPC_SetModeFromPad(npc,ActBringingMotion,npc->base_mar,PAD_START_SHOOT_FMS,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_BRINGING_USP_AND_SITTING:
		NPC_SetModeFromPad(npc,ActBringingMotion,npc->base_mar,PAD_START_SHOOT_USP,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_BRINGING_FMS_AND_SITTING:
		NPC_SetModeFromPad(npc,ActBringingMotion,npc->base_mar,PAD_START_SHOOT_FMS,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

	case PAD_PUNCH:
		NPC_SetModeFromPad(npc,ActMotion_PunchKick,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_PUNCH_WUSP:
		NPC_SetModeFromPad(npc,ActMotion_PunchKick,npc->base_mar,PAD_PUNCH,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_PUNCH_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_PunchKickWM4A,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

	case PAD_DAMAGE_F:
		NPC_SetModeFromPad(npc,ActDamage,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_F;
		break;
	case PAD_DAMAGE_B:
		NPC_SetModeFromPad(npc,ActDamage,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_B;
		break;
	case PAD_DAMAGE_F_SHORT:
		NPC_SetModeFromPad(npc,ActDamage2,npc->base_mar,PAD_DAMAGE_F,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_F;
		break;
	case PAD_DAMAGE_B_SHORT:
		NPC_SetModeFromPad(npc,ActDamage2,npc->base_mar,PAD_DAMAGE_B,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_B;
		break;
	case PAD_DAMAGE_KNOCKDOWN:
		NPC_SetModeFromPad(npc,ActKnockdown,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_B;
		break;
	case PAD_DAMAGE_SLEEP_F:
	case PAD_DAMAGE_SLEEP_B:
		SET_SCN_FAINT();

		ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

		NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
		NPC_CallHeadMark( npc, HMK2_TYPE_PIYO  ) ;
		NPC_SetActMode( npc, NPC_ActFaint ) ;

		// ZZZSOUND_ON(ACCESS_WORK(npc));

		act->pad=act->set_pad=PAD_NONE;

		// NPC_SetModeFromPad(npc,ActLaydown,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_DAMAGE_GAMEOVER:
		NPC_SetModeFromPad(npc,ActGameover,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_B;
		break;
	case PAD_HANG_GAMEOVER:
		NPC_SetModeFromPad(npc,ActGameover2,npc->base_mar,PAD_DAMAGE_SLEEP_F,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* ダンボールプリスキン用 */
	case PAD_BOX_IDLE:
		NPC_SetModeFromPad(npc,ActMotion_Stand,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_BOX_WALK:
	case PAD_BOX_RUN:
	case PAD_BOX_DASH:
		NPC_SetModeFromPad(npc,ActMotion_Move,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_BOX_STOP:
		NPC_SetModeFromPad(npc,ActMotion_StandW17A,npc->base_mar,PAD_BOX_WALK,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* 狙撃スネーク用 */
	case PAD_CROUCHON:
		NPC_SetModeFromPad(npc,ActMotion_CrouchOn,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_CROUCHING:
	case PAD_CROUCHING_WPSG:
		NPC_SetModeFromPad(npc,ActLoopMotionWLock,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_DAMAGE_CROUCHING:
		NPC_SetModeFromPad(npc,ActMotion_CrouchDamage,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SHOOT_PSG:
		NPC_SetModeFromPad(npc,ActMotion_ShootPSG,npc->base_mar,PAD_CROUCHING_WPSG,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* w14a居眠りスネーク */
	case PAD_SITTING_AND_SLEEP_WM4A1:
		NPC_SetModeFromPad(npc,ActLoopMotionWLockW14A,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_BRINGING_WM4A1:
		NPC_SetModeFromPad(npc,ActBringM4A1W14A,
						   npc->base_mar,PAD_SITTING_AND_START_SHOOT_WM4A1,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_START_SHOOT_WM4A1:
		NPC_SetModeFromPad(npc,ActStartShootM4A1W14A,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_SHOOT_WM4A1:
		NPC_SetModeFromPad(npc,ActShootM4A1W14A,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_DAMAGED:
		NPC_SetModeFromPad(npc,ActDamageW14A,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_SITTING_AND_M9SLEEP:
		NPC_SetModeFromPad(npc,ActLoopMotionWLockW14A,
						   npc->base_mar,PAD_SITTING_AND_SLEEP_WM4A1,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* 麻酔銃 */
#if 0
	case PAD_SLEEP_F:
		NPC_SetModeFromPad( npc, ActSleepDown, npc->base_mar, act->pad, act->pad ) ;
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_F;
		break;
#endif
	case PAD_SLEEP_B:
		NPC_SetModeFromPad( npc, ActSleepDown, npc->base_mar, act->pad, act->pad ) ;
		ActSubMotionLock(ACCESS_WORK(npc));
		act->down_side=NPC_DOWN_SIDE_B;
		break;
#if 1
	case PAD_DAMAGE_WAKEUP_F:
	// case PAD_SLEEP_WAKEUP_F:
	case PAD_DAMAGE_WAKEUP_B:
	case PAD_SLEEP_WAKEUP_B:
		NPC_SetModeFromPad(npc,ActWakeup,npc->base_mar,act->pad,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
#endif
	case PAD_DAMAGE_PSG1T_CROUNING:
		NPC_SetModeFromPad(npc,ActMotion_CrouchSleep,npc->base_mar,PAD_DAMAGE_CROUCHING,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

		/* ローリング */
	case PAD_TUMBLE_START:
	case PAD_TUMBLE_START_WUSP:
	case PAD_TUMBLE_START_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_Rolling,npc->base_mar,PAD_TUMBLE_START,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;
	case PAD_TUMBLE_END:
	case PAD_TUMBLE_END_WUSP:
	case PAD_TUMBLE_END_WFMS:
		NPC_SetModeFromPad(npc,ActMotion_RollingEnd,npc->base_mar,PAD_TUMBLE_END,act->pad);
		ActSubMotionLock(ACCESS_WORK(npc));
		break;

	default:
		return 0;
	}

	return 1 ;
}


/* ------------------------------------------------------------------------ */
/*  Check Damage                                                            */
/* ------------------------------------------------------------------------ */


/* レーザーサイトもチェックの対象にするため、TARGET_SEEKも判定する */
static int NPC_PowerAndSeekChildTargetCheck(NPCWORK *npc)
{
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	int			i ;
	
	npctrg = &npc->target ;
	def_child = npctrg->def_child ;

	for(i=0;i<npctrg->child_trg_num;i++){
		if((TARGET_POWER|TARGET_SEEK) & def_child->damaged){
			return i ;
		}
		def_child ++ ;
	}

	return -1 ;
}


#if 0
#define DIS_LIMIT		CVM2N(0.5f)
#elif 0
/* 2001/6/26 Ken Kano 変更 */
#define DIS_LIMIT		CVM2N(0.4f)
#else
#define DIS_LIMIT		CVM2N(0.7f)
#endif

#if 0
#define DIS_STOP_LIMIT	CVM2N(0.4f)
#else
#define DIS_STOP_LIMIT	CVM2N(0.6f)
#endif

#define DIS2_LIMIT		CVM2N(1.0f)
#define YDIS_LIMIT		CVM2N(1.5f)

// #define MOVE_DIS		CVM2N(0.6f)
// #define MOVE_DIS		CVM2N(0.3f)
#define MOVE_DIS		CVC2N(6.4f)
// #define MOVE_DIS2		CVM2N(30.0f)
#define MOVE_DIS2		CVM2N(0.0f)


int NPCSnake_CheckRaidenDisForFaint(NPCWORK *npc)
{
	float x=npc->ctrl->mov.vx;
	float y=npc->ctrl->mov.vy;
	float z=npc->ctrl->mov.vz;
	float rx=GM_PlayerPosition.vx;
	float ry=GM_PlayerPosition.vy;
	float rz=GM_PlayerPosition.vz;

	float dis;

	x-=rx; y-=ry; z-=rz;
	dis=x*x+z*z;

	if(dis<=DIS2_LIMIT*DIS2_LIMIT && y>=-YDIS_LIMIT && y<=YDIS_LIMIT) return 1;
	return 0;
}

int NPCSnake_CheckRaidenDisForPush(NPCWORK *npc,FVECTOR *v)
{
	float x=npc->ctrl->mov.vx;
	float y=npc->ctrl->mov.vy;
	float z=npc->ctrl->mov.vz;
	float rx=GM_PlayerPosition.vx;
	float ry=GM_PlayerPosition.vy;
	float rz=GM_PlayerPosition.vz;

	float dis;
	float cmp_dis;

	/* しゃがみ、匍匐、張りつきは条件変更 */
	if(GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND|PLAYER_CAUTION|PLAYER_ROLLING)) return 0;

	/* 移動中でも条件変更 */
	if(GM_CheckPlayerStatus(PLAYER_MOVE)){
		cmp_dis=DIS_LIMIT*DIS_LIMIT;
	}
	else{
		cmp_dis=DIS_STOP_LIMIT*DIS_STOP_LIMIT;
	}

	x-=rx; y-=ry; z-=rz;
	dis=x*x+z*z;

	if(dis<=cmp_dis && y>=-YDIS_LIMIT && y<=YDIS_LIMIT){
		float vx,vz;
		float a;

		if(dis<0.0001f){
			x=npc->body->objs->world.m[2][0];
			z=npc->body->objs->world.m[2][2];
		}

		a=frnd();
		if(a>=0.0f && a<0.5f) a=0.5f;
		else if(a<0.0f && a>=-0.5f) a=-0.5f;

		a=MOVE_DIS2*a;
		vx=x-z*a;
		vz=z+x*a;

		a=vx*vx+vz*vz;
		a=fpu_Rsqrt(a,MOVE_DIS);

		v->vx=vx*a;
		v->vy=0.0f;
		v->vz=vz*a;

		return 1;
	}
	return 0;
}


static void SnakeBlood(OBJECT *body,int n_obj,FVECTOR *pos,int mode)
{
	FVECTOR	vans,nans;
	void *NewBlood( FMATRIX *world, FVECTOR *pos, FVECTOR *force, int mode, int white );

	VertexSearch(&vans,&nans,body->objs,n_obj,pos);

	vu0_Ldv0(&vans);
	vu0_Ldv1(&nans);
	vu0_Setv0w1();
	vu0_Setv1w0();
	vu0_Ldm0(&(body->objs->objs[n_obj].world));
	vu0_Mulv0m0v0();
	vu0_Mulv1m0v1();
	vu0_Stv0(&vans);
	vu0_Stv1(&nans);

	NewBlood(&(body->objs->objs[n_obj].world),&vans,&nans,mode,0);
}


/* ------------------------------------------------------------------------ */

static void DamageForBlade(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit)
{
#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	/* 刀の振り当たり */

#ifdef DEBUG_MODE
	if(npcsnake_nodamage_flag==0) act->life-=BULLET_DAMAGE_POINT(npc);
#else
	act->life-=BULLET_DAMAGE_POINT(npc);
#endif

	fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));

	if(act->life<=0){
		NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
		ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
	}
	else{
		if(irnd()>ULONG_MAX/2){
			ChangeNewPad(npc,PAD_DAMAGE_F);
		}
		else{
			ChangeNewPad(npc,PAD_DAMAGE_B);
		}
	}
}

static void EffectForBlade(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit,int voice)
{
	int life=act->life;

	/* 刀の振り当たり */
	life-=BULLET_DAMAGE_POINT(npc);

	NewPadVibration2(VIBDATA_BLADE,0);

	if(life<=0){
		SnakeBlood(npc->body,part,hit,1);
	}
	else{
		GM_SeSetMode(SD_A_SWORDCUT,
					 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

		SnakeBlood(npc->body,part,hit,0);
	}
}

static void DamageForBladeStab(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit)
{
#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	/* 刀の突き当たり */

#ifdef DEBUG_MODE
	if(npcsnake_nodamage_flag==0) act->life-=BULLET_DAMAGE_POINT(npc);
#else
	act->life-=BULLET_DAMAGE_POINT(npc);
#endif

	fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));

	if(act->life<=0){
		NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
		ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
	}
	else{
		if(irnd()>ULONG_MAX/2){
			ChangeNewPad(npc,PAD_DAMAGE_F);
		}
		else{
			ChangeNewPad(npc,PAD_DAMAGE_B);
		}
	}
}

static void EffectForBladeStab(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit,int voice)
{
	int life=act->life;

	/* 刀の突き当たり */
	life-=BULLET_DAMAGE_POINT(npc);

	NewPadVibration2(VIBDATA_BLADE_STAB,0);

	if(life<=0){
		SnakeBlood(npc->body,part,hit,1);
	}
	else{
 		// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_SWORDBIT);
 		GM_SeSetMode(SD_A_SWORDBIT,(FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
 
 		SnakeBlood(npc->body,part,hit,0);
	}
}

static void DamageForBladeFaint(NPCWORK *npc,NPCACT *act)
{
	/* 峰撃ち */

	act->faint-=BLADE_FAINT_POINT;

	fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));

	if(act->faint<0){
		ChangeNewPad(npc,PAD_DAMAGE_KNOCKDOWN);
	}
	else{
		if(irnd()>ULONG_MAX/2){
			ChangeNewPad(npc,PAD_DAMAGE_F);
		}
		else{
			ChangeNewPad(npc,PAD_DAMAGE_B);
		}
	}
}

static void EffectForBladeFaint(NPCWORK *npc,NPCACT *act,int voice)
{
	/* 峰撃ち */
	NewPadVibration2(VIBDATA_BLADE_FAINT,0);

	// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_SWORDHIT);
	GM_SeSetMode(SD_A_SWORDHIT,(FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
}

static void DamageForPunch(NPCWORK *npc,NPCACT *act,long64 weapon)
{
#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	/* パンチの場合 */

#ifdef DEBUG_MODE
	// if(npcsnake_nodamage_flag==0) act->life-=PUNCH_DAMAGE_POINT(npc);
#else
	// act->life-=PUNCH_DAMAGE_POINT(npc);
#endif

	if((weapon & WP_PUNCHR) && !(weapon & WP_NOPLAYER)){
		switch(PL_GetPlayerWeapon()){
		case WP_None:
			act->faint-=PUNCH_FAINT_POINT;
			break;
		case WP_m4:
		case WP_Aks:
		case WP_Rgb6:
			act->faint-=M4_PUNCH_FAINT_POINT;
			break;
		case WP_Nikita:
			act->faint-=NIKITA_PUNCH_FAINT_POINT;
			break;
		default:
			act->faint-=GUN_PUNCH_FAINT_POINT;
			break;
		}
	}
	else{
		act->faint-=PUNCH_FAINT_POINT;
	}

	fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));

	if(act->life<=0){
		NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
		ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
	}
	else if(act->faint<0){
		ChangeNewPad(npc,PAD_DAMAGE_KNOCKDOWN);
	}
	else{
		ChangeNewPad(npc,PAD_DAMAGE_B);
	}
}

static void EffectForPunch(NPCWORK *npc,NPCACT *act,long64 weapon,int voice)
{
	int life=act->life;
	int faint=act->faint;

	// life-=PUNCH_DAMAGE_POINT(npc);

	if((weapon & WP_PUNCHR) && !(weapon & WP_NOPLAYER)){
		switch(PL_GetPlayerWeapon()){
		case WP_None:
			faint-=PUNCH_FAINT_POINT;

			GM_SeSetMode(SD_P_PUNCH02,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

			NewPadVibration2(VIBDATA_PUNCH,0);
			break;
		case WP_m4:
		case WP_Aks:
		case WP_Rgb6:
			faint-=M4_PUNCH_FAINT_POINT;

			GM_SeSetMode(SD_P_GUNPNC01,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

			NewPadVibration2(VIBDATA_PUNCH_GUN,0);
			break;
		case WP_Nikita:
			faint-=NIKITA_PUNCH_FAINT_POINT;

			GM_SeSetMode(SD_P_GUNPNC01,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

			NewPadVibration2(VIBDATA_PUNCH_NIKITA,0);
			break;
		default:
			faint-=GUN_PUNCH_FAINT_POINT;

			GM_SeSetMode(SD_P_GUNPNC01,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

			NewPadVibration2(VIBDATA_PUNCH,0);
			break;
		}
	}
	else{
		faint-=PUNCH_FAINT_POINT;

		GM_SeSetMode(SD_P_PUNCH02,
					 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

		if(!(weapon & WP_NOPLAYER)) NewPadVibration2(VIBDATA_PUNCH,0);
	}

	if(life<=0){
		;
	}
	else if(faint<0){
		/* うぉっ */
		if(voice){
			// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG01);
			GM_SeSetMode(SD_V_SNADMG01,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
		}
		else{
			act->adj_piku_time=SHORT_REACTION_COUNT;
		}
	}
	else{
		/* うっ */
		if(voice){
			// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG02q);
			GM_SeSetMode(SD_V_SNADMG02,(FVECTOR *)&(npc->body->objs->world.m[3][0]),
						 GM_SEMODE_BOMB);
		}
		else{
			act->adj_piku_time=SHORT_REACTION_COUNT;
		}
	}
}

static void DamageForKick(NPCWORK *npc,NPCACT *act,long64 weapon)
{
#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	if(!(weapon & WP_NOPLAYER)){
		if(PL_GetPlayerWeapon()==WP_Nikita){
#ifdef DEBUG_MODE
			// if(npcsnake_nodamage_flag==0) act->life-=PUNCH_DAMAGE_POINT(npc);
#else
			// act->life-=PUNCH_DAMAGE_POINT(npc);
#endif

			act->faint-=NIKITA_PUNCH_FAINT_POINT;

			if(act->life<=0){
				NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
				ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
			}
			else{
				ChangeNewPad(npc,PAD_DAMAGE_KNOCKDOWN);
			}
			return;
		}
		if(GM_CheckPlayerStatus(PLAYER_ROLLING)){
#ifdef DEBUG_MODE
			// if(npcsnake_nodamage_flag==0) act->life-=KICK_DAMAGE_POINT(npc);
#else
			// act->life-=KICK_DAMAGE_POINT(npc);
#endif

			act->faint-=KICK_FAINT_POINT;

			if(act->life<=0){
				NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
				ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
			}
			else{
				ChangeNewPad(npc,PAD_DAMAGE_KNOCKDOWN);
			}
			return;
		}
	}

	/* ふっ飛ぶモーションの場合 */

#ifdef DEBUG_MODE
	// if(npcsnake_nodamage_flag==0) act->life-=KICK_DAMAGE_POINT(npc);
#else
	// act->life-=KICK_DAMAGE_POINT(npc);
#endif

	fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));

	if(act->life<=0){
		NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
		ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
	}
	else{
		act->faint-=KICK_FAINT_POINT;

		if(act->faint<0 ||
		   (!(weapon & WP_NOPLAYER) && (weapon & WP_KICK) && !(weapon & WP_KICK1))){

			ChangeNewPad(npc,PAD_DAMAGE_KNOCKDOWN);
			// NPCSNAKE_SLEEP_COUNTSET(npc,KNOCKDOWN_COUNT);
		}
		else{
			ChangeNewPad(npc,PAD_DAMAGE_B);
		}
	}
}

static void EffectForKick(NPCWORK *npc,NPCACT *act,long64 weapon,int voice)
{
#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	int life=act->life;

	if(!(weapon & WP_NOPLAYER)){
		if(PL_GetPlayerWeapon()==WP_Nikita){
#ifdef DEBUG_MODE
			// if(npcsnake_nodamage_flag==0) life-=PUNCH_DAMAGE_POINT(npc);
#else
			// life-=PUNCH_DAMAGE_POINT(npc);
#endif

			GM_SeSetMode(SD_P_GUNPNC01,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

			NewPadVibration2(VIBDATA_PUNCH_NIKITA,0);

			if(life<=0){
				;
			}
			else{
				/* うぉっ */
				if(voice){
					// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG01);
					GM_SeSetMode(SD_V_SNADMG01,(FVECTOR *)&(npc->body->objs->world.m[3][0]),
								 GM_SEMODE_BOMB);
				}
				else{
					act->adj_piku_time=SHORT_REACTION_COUNT;
				}
			}
			return;
		}
		if(GM_CheckPlayerStatus(PLAYER_ROLLING)){
#ifdef DEBUG_MODE
			// if(npcsnake_nodamage_flag==0) life-=KICK_DAMAGE_POINT(npc);
#else
			// life-=KICK_DAMAGE_POINT(npc);
#endif

			GM_SeSetMode(SD_P_KICK02,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

			NewPadVibration2(VIBDATA_2ND_KICK,0);

			if(life<=0){
				;
			}
			else{
				/* うぉっ */
				if(voice){
					// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG01);
					GM_SeSetMode(SD_V_SNADMG01,(FVECTOR *)&(npc->body->objs->world.m[3][0]),
								 GM_SEMODE_BOMB);
				}
				else{
					act->adj_piku_time=SHORT_REACTION_COUNT;
				}
			}
			return;
		}
	}


	GM_SeSetMode(SD_P_KICK02,
				 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

	/* ふっ飛ぶモーションの場合 */
#ifdef DEBUG_MODE
	// if(npcsnake_nodamage_flag==0) life-=KICK_DAMAGE_POINT(npc);
#else
	// life-=KICK_DAMAGE_POINT(npc);
#endif

	if(life<=0){
		;
	}
	else{
		/* うぉっ */
		if(voice){
			// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG01);
			GM_SeSetMode(SD_V_SNADMG01,(FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
		}
		else{
			act->adj_piku_time=SHORT_REACTION_COUNT;
		}
	}

	if(!(weapon & WP_NOPLAYER)){
		if(weapon & WP_KICK1) NewPadVibration2(VIBDATA_KICK,0);
		else NewPadVibration2(VIBDATA_2ND_KICK,0);
	}
}

static void DamageForStunGrenade(NPCWORK *npc,NPCACT *act)
{
	/* スタングレネードによる気絶 */
	act->faint-=STUNGRENADE_FAINT_POINT;

	ChangeNewPad(npc,PAD_DAMAGE_KNOCKDOWN);
	// NPCSNAKE_SLEEP_COUNTSET(npc,KNOCKDOWN_COUNT);
}

static void EffectForStunGrenade(NPCWORK *npc,NPCACT *act,int voice)
{
	/* スタングレネードによる気絶 */

	/* うぉっ */
	if(voice){
		// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG01);
		GM_SeSetMode(SD_V_SNADMG01,
					 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
	}
	else{
		act->adj_piku_time=SHORT_REACTION_COUNT;
	}
}

static void ReactionForWeaponCore(NPCWORK *npc,NPCACT *act)
{
	/* マガジンを当てた場合の反応 */
	act->adj_piku_time=SHORT_REACTION_COUNT;
}

static void EffectForWeaponCore(NPCWORK *npc,NPCACT *act,int voice)
{
	/* マガジンを当てた場合の反応 */
}

static void ReactionForColdSpray(NPCWORK *npc,NPCACT *act)
{
	/* 冷却スプレーによる反応 */
	fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));

	if(irnd()>ULONG_MAX/2){
		ChangeNewPad(npc,PAD_DAMAGE_F);
	}
	else{
		ChangeNewPad(npc,PAD_DAMAGE_B);
	}
}

static void EffectForColdSpray(NPCWORK *npc,NPCACT *act,int voice)
{
	/* 冷却スプレーによる反応 */

	/* うっ */
	if(voice){
		// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG02);
		GM_SeSetMode(SD_V_SNADMG02,
					 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
	}
}

static void DamageForBlow(NPCWORK *npc,NPCACT *act)
{
#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	/* ふっ飛ぶモーションの場合 */

#ifdef DEBUG_MODE
	if(npcsnake_nodamage_flag==0) act->life-=KNOCKDOWN_DAMAGE_POINT(npc);
#else
	act->life-=KNOCKDOWN_DAMAGE_POINT(npc);
#endif

	if(act->life<=0){
		NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
		ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
	}
	else{
		act->faint-=BLOW_FAINT_POINT;

		ChangeNewPad(npc,PAD_DAMAGE_KNOCKDOWN);
		// NPCSNAKE_SLEEP_COUNTSET(npc,KNOCKDOWN_COUNT);
	}
}

static void EffectForBlow(NPCWORK *npc,NPCACT *act,int voice)
{
	int life=act->life;

	life-=KNOCKDOWN_DAMAGE_POINT(npc);

	if(life<=0){
		;
	}
	else{
		/* うぉっ */
		// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG01);
		if(voice){
			GM_SeSetMode(SD_V_SNADMG01,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
		}
		else{
			act->adj_piku_time=SHORT_REACTION_COUNT;
		}
	}
}

static void ReactionForNeedl(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit)
{
	/* 麻酔弾 */
#if 0
	NPC_SetModeFromPad( npc, ActSleepDown, npc->base_mar, PAD_SLEEP_B, act->pad ) ;
	act->down_side=NPC_DOWN_SIDE_B;
#else
	ChangeNewPad(npc,PAD_SLEEP_B);
#endif
}

static void EffectForNeedl(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit,int voice)
{
	/* 麻酔弾 */

	GM_SeSetMode(SD_A_M_NINE01,
				 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

	if(voice){
		/* うっ */
		// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG02q);
		GM_SeSetMode(SD_V_SNADMG02,
					 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
	}

	NPC_SetNeedl(npc->body,part,hit,MASUIDAN_MODEL);
}

static void DamageNormal(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit)
{
#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	/* その他よろけダメージの場合 */

#ifdef DEBUG_MODE
	if(npcsnake_nodamage_flag==0) act->life-=BULLET_DAMAGE_POINT(npc);
#else
	act->life-=BULLET_DAMAGE_POINT(npc);
#endif

	fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));

	if(act->life<=0){
		NPC_ActStatus(act,NPC_ACT_STATUS_DEATH);
		ChangeNewPad(npc,PAD_DAMAGE_GAMEOVER);
	}
	else{
		if(irnd()>ULONG_MAX/2){
			ChangeNewPad(npc,PAD_DAMAGE_F);
		}
		else{
			ChangeNewPad(npc,PAD_DAMAGE_B);
		}
	}
}

static void EffectNormal(NPCWORK *npc,NPCACT *act,int part,FVECTOR *hit,int voice)
{
	int life=act->life;

	/* その他よろけダメージの場合 */

	life-=BULLET_DAMAGE_POINT(npc);

	if(life<=0){
		SnakeBlood(npc->body,part,hit,1);
	}
	else{
		if(voice){
			/* うっ */
			// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG02);
			GM_SeSetMode(SD_V_SNADMG02,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);
		}
		else{
			act->adj_piku_time=SHORT_REACTION_COUNT;
		}

		SnakeBlood(npc->body,part,hit,0);
	}
}


/* ------------------------------------------------------------------------ */

void NPCSnake_DefTargetCallback(TARGET *off,TARGET *def,void *_work)
{
#if 1

	Work *work=(Work *)_work;
	NPCWORK *npc=&(work->npc);
	NPCTARGET	*trg;
	NPCACT	*act;
	long64	weapon;
	int		dam_child_num;
	int     needl_part=HUMAN21_MUNE;
	TARGET	*dam_trg=NULL;
	int voice=1;

#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif


	// DEBUG_PRINT_NPCSNAKE("Callback Start\n");


#ifdef DEBUG_MODE
	if(npcsnake_nodamage_flag==1) return;
#endif

	if(GM_CheckGameStatus(STATE_SCN_DEMO)) return;

	if(work->snake_status2 & SNAKE_STATUS2_CALLED_DAMAGE) return;

	trg=&npc->target;
	act=&npc->action;
	weapon=0;

	if((act->status & (NPC_ACT_STATUS_DEATH|NPC_ACT_STATUS_TRG_OFF)) ||
	   NPCSNAKE_NODAMAGE_COUNTCHECK(npc)) return;

	if((dam_child_num=NPC_ChildTargetCheck(npc))>=0){
		dam_trg=trg->def_child+dam_child_num ;
		weapon=dam_trg->weapon_type;
		needl_part=trg->connect_obj[dam_child_num];
	}
	else if(TARGET_POWER & def->damaged){
		dam_trg=def;
		weapon=def->weapon_type;
		needl_part=HUMAN21_MUNE;
	}

	/* スネークの弾 */
	if((weapon & WP_NOPLAYER) && ((weapon & WP_USP) || (weapon & WP_M4))) return;

	/* 頭に当たったら即死 */
	if(work->stage==NPCSNAKE_STAGE_BETWEEN_STAGES){
		if(needl_part==HUMAN21_ATAMA &&
		   (weapon & WP_BULLET) && !(weapon & WP_NOPLAYER)){

			DEBUG_PRINT_NPCSNAKE("Death at All !!!!!\n");

			act->life=-1;
		}
	}


	// if(act->status & (NPC_ACT_STATUS_FAINT|NPC_ACT_STATUS_FAINT_END)) voice=0;
	if(work->snake_status2 & SNAKE_STATUS2_FAINT) voice=0;

	if(weapon){
		if(!(weapon & WP_NOPLAYER)){
			ACCESS_WORK(npc)->ret_flag|=NPCSNAKE_RETFLAG_DAMAGED_FROM_PLAYER;
			ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
		}
		else{
			ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
		}

		if(weapon & WP_BLADE){
			/* 刀の振り当たり */

			EffectForBlade(npc,act,needl_part,&dam_trg->hit,voice);
			if(!(weapon & WP_NOPLAYER)) NewPadVibration2(VIBDATA_BLADE,0);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if(weapon & WP_BLADESTAB){
			/* 刀の突き当たり */

			EffectForBladeStab(npc,act,needl_part,&dam_trg->hit,voice);
			if(!(weapon & WP_NOPLAYER)) NewPadVibration2(VIBDATA_BLADE_STAB,0);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if(weapon & WP_BLADEFAINT){
			/* 峰撃ち */

			EffectForBladeFaint(npc,act,voice);
			if(!(weapon & WP_NOPLAYER)) NewPadVibration2(VIBDATA_BLADE_FAINT,0);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if(weapon & WP_PUNCHALL){
			/* パンチの場合 */

			EffectForPunch(npc,act,weapon,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if(weapon & WP_KICK){
			/* ふっ飛ぶモーションの場合 */

			EffectForKick(npc,act,weapon,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if(weapon & (WP_STUNGRENADE|WP_STUNFAR)){
			/* スタングレネードによる気絶 */

			EffectForStunGrenade(npc,act,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if(weapon & WP_WEAPONCORE){
			/* マガジンを当てた場合の反応 */

			EffectForWeaponCore(npc,act,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if( weapon & WP_COLDSPRAY){
			/* 冷却スプレーによる反応 */

			EffectForColdSpray(npc,act,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if(weapon & WP_BLOW){
			/* ふっ飛ぶモーションの場合 */

			EffectForBlow(npc,act,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if ( weapon & (WP_M92|WP_PSG1T) ) {
			/* 麻酔弾 */

			EffectForNeedl(npc,act,needl_part,&dam_trg->hit,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
		else if( weapon & ~(WP_MIC|WP_PLAYER|WP_NOPLAYER|WP_BODY|WP_TUMBLE|WP_HANG|WP_ONCORPSE|
							WP_LASERSIGHT|WP_BOOK)){

			/* その他よろけダメージの場合 */

			EffectNormal(npc,act,needl_part,&dam_trg->hit,voice);
			work->snake_status2|=SNAKE_STATUS2_CALLED_DAMAGE;
		}
	}

#endif
}

/* ------------------------------------------------------------------------ */


int NPCSnake_CheckDamage(NPCWORK *npc)
{
	TARGET	*def;
	NPCACT	*act;
	NPCTARGET	*trg;
	CAPTURE_TARGET	*cap;
	long64	weapon;
	int		dam_child_num;
	int     needl_part=HUMAN21_MUNE;

#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	weapon = 0 ;

#ifdef DEBUG_MODE
	if(npcsnake_nodamage_flag==1){
		NPC_DamageFlagClear( npc );		/* ダメージのみクリア */
		goto next;
	}
#endif

	if(GM_CheckGameStatus(STATE_SCN_DEMO)){
		NPC_DamageCaptureFlagClear(npc);
		return 0;
	}


	if((dam_child_num=NPC_ChildTargetCheck(npc))>=0){
		trg->dam_trg = trg->def_child + dam_child_num ;
		trg->dam_obj = trg->connect_obj[ dam_child_num ] ;
		weapon=trg->dam_trg->weapon_type;
		needl_part=trg->connect_obj[dam_child_num];
	}
	else if(TARGET_POWER & def->damaged){
		trg->dam_trg = def;
		trg->dam_obj = HUMAN21_MUNE;
		weapon=def->weapon_type;
		needl_part=HUMAN21_MUNE;
	}

	/* スネークの弾 */
	if((weapon & WP_NOPLAYER) && ((weapon & WP_USP) || (weapon & WP_M4))){
		DEBUG_PRINT_NPCSNAKE("Kill oneself\n");
		goto next;
	}

	if(weapon){
		DEBUG_PRINT_NPCSNAKE("npc damage weapon type [%lx]  \n",weapon);

//		act->damtrg = def ;
		NPC_DamageCaptureFlagClear( npc ) ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		ACCESS_WORK(npc)->snake_status
			&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

		NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

		MOTION_CANCELABLE_ON(npc);
		UNLOCK_ENEMY(npc);

		NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_STAY);
		NPCSNAKE_SHOOT_SET(ACCESS_WORK(npc),NPCSNAKE_SHOOT_NONE);
		NPCSNAKE_HIDE_SET(ACCESS_WORK(npc),NPCSNAKE_HIDE_NO_HIDING);

		ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
		ActSubMotionReset(ACCESS_WORK(npc));

		if(!(weapon & WP_NOPLAYER)){
			ACCESS_WORK(npc)->ret_flag|=NPCSNAKE_RETFLAG_DAMAGED_FROM_PLAYER;
			ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
		}
		else{
			ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_DAMAGED_FOR_BEHIND;
		}

		if(weapon & WP_BLADE){
			/* 刀の振り当たり */

			DamageForBlade(npc,act,needl_part,&trg->dam_trg->hit);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else if(weapon & WP_BLADESTAB){
			/* 刀の突き当たり */

			DamageForBladeStab(npc,act,needl_part,&trg->dam_trg->hit);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else if(weapon & WP_BLADEFAINT){
			/* 峰撃ち */

			DamageForBladeFaint(npc,act);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else if(weapon & WP_PUNCHALL){
			/* パンチの場合 */

			DamageForPunch(npc,act,weapon);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else if(weapon & WP_KICK){
			/* ふっ飛ぶモーションの場合 */

			DamageForKick(npc,act,weapon);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else if(weapon & (WP_STUNGRENADE|WP_STUNFAR)){
			/* スタングレネードによる気絶 */

			DamageForStunGrenade(npc,act);
		}
		else if(weapon & WP_WEAPONCORE){
			/* マガジンを当てた場合の反応 */

			ReactionForWeaponCore(npc,act);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else if( weapon & WP_COLDSPRAY){
			/* 冷却スプレーによる反応 */

			ReactionForColdSpray(npc,act);

			if(ACCESS_WORK(npc)->spray_count>0){
				NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_VLONG_COUNT);
			}
			else{
				NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
			}
			ACCESS_WORK(npc)->spray_count=SPRAY_TIME;
		}
		else if(weapon & WP_BLOW){
			/* ふっ飛ぶモーションの場合 */

			DamageForBlow(npc,act);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else if ( weapon & (WP_M92|WP_PSG1T) ) {
			/* 麻酔弾 */

			ReactionForNeedl(npc,act,needl_part,&trg->dam_trg->hit);
		}
		else if( weapon & ~(WP_MIC|WP_PLAYER|WP_NOPLAYER|WP_BODY|WP_TUMBLE|WP_HANG|WP_ONCORPSE|
							WP_LASERSIGHT|WP_WEAPONCORE)){

			/* その他よろけダメージの場合 */

			DamageNormal(npc,act,needl_part,&trg->dam_trg->hit);

			NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
		}
		else goto next;

		return 1 ;
	}


next:

#if 1
	if ( TARGET_CAPTURE & def->damaged ) {
		NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
		cap = trg->capture ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		ACCESS_WORK(npc)->snake_status
			&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

		NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

		MOTION_CANCELABLE_ON(npc);
		UNLOCK_ENEMY(npc);

		NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_STAY);
		NPCSNAKE_SHOOT_SET(ACCESS_WORK(npc),NPCSNAKE_SHOOT_NONE);
		NPCSNAKE_HIDE_SET(ACCESS_WORK(npc),NPCSNAKE_HIDE_NO_HIDING);

		ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
		ActSubMotionReset(ACCESS_WORK(npc));

		if(cap->capture!=NULL){
			if((int)(cap->capture->ctrl)==(int)GM_PlayerControl){
				ACCESS_WORK(npc)->ret_flag|=NPCSNAKE_RETFLAG_CAPTURED_FROM_PLAYER;
				ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_CAPTURED_BY_RAIDEN;
			}
			else{
				ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_CAPTURED_BY_RAIDEN;
			}
		}

		if ( cap->flag & CAPTURE_HANG ) {	/* 首絞め */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;
			act->down_side=NPC_DOWN_SIDE_F;
			NPC_SetModeFromPad( npc, NPC_ActHang, npc->capture_mar, NPC_MOT_HANG_TIE, act->pad ) ;
		}
		else if ( cap->flag & CAPTURE_THROW ){	/* 投げ */
			act->faint-=THROW_FAINT_POINT;
			act->down_side=NPC_DOWN_SIDE_F;
			NPC_SetModeFromPad( npc, NPC_ActThrow, npc->capture_mar, NPC_MOT_THROW, act->pad ) ;
		}

#if 1
/*2001.11.22 やっぱりフタ*/
		cap->flag = 0 ;
#else
/*2001.10.22 sigeno 羽交い絞めフラグは絞めた人が離した時に倒す*/
		cap->flag &= (~CAPTURE_HANG) ;
#endif


#ifdef DEBUG_MODE
		if(npcsnake_nodamage_flag==0) act->life -= THROW_DAMAGE_POINT(npc) ;
#else
		act->life -= THROW_DAMAGE_POINT(npc) ;
#endif

		return 1 ;
	}
#endif

	NPC_DamageCaptureFlagClear(npc);

	if(!(act->status & NPC_ACT_STATUS_TRG_OFF) &&
	   !(act->status & NPC_ACT_STATUS_DAMAGE) &&
	   !(act->status & NPC_ACT_SNAKE_FORCE_MOVE)){

		FVECTOR a;

		if(NPCSnake_CheckRaidenDisForPush(npc,&a)){
			/* ライデンに押された */
#if 0
			npc->ctrl->step.vx+=a.vx;
			npc->ctrl->step.vz+=a.vz;
#else
			ACCESS_WORK(npc)->damage_mov_v.vx=a.vx;
			ACCESS_WORK(npc)->damage_mov_v.vz=a.vz;
#endif

			/* パンチキックを出している間は、押されたことだけ計算しておく */
			if(act->status & NPC_ACT_SNAKE_PUNCH_KICK) return 0;

			/* このフレームではダメージを受けない */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE ) ;

			// ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

			// NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

			ACCESS_WORK(npc)->snake_status
				&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

			NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

			MOTION_CANCELABLE_ON(npc);
			SHOOT_ACTION_FINISHED(npc);
			UNLOCK_ENEMY(npc);

			NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_STAY);
			NPCSNAKE_SHOOT_SET(ACCESS_WORK(npc),NPCSNAKE_SHOOT_NONE);
			NPCSNAKE_HIDE_SET(ACCESS_WORK(npc),NPCSNAKE_HIDE_NO_HIDING);

			ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
			ActSubMotionReset(ACCESS_WORK(npc));

			ACCESS_WORK(npc)->i_infinity++;
			if(ACCESS_WORK(npc)->i_infinity>=INFINITY_MOVE_SIZE) ACCESS_WORK(npc)->i_infinity=0;


#if 0
			/* ローリングを入れてみたが、他のアルゴリズムへの影響が大きく断念 */

			if(ACCESS_WORK(npc)->roll_count>0){
				if(ACCESS_WORK(npc)->roll_counter>1 || ACCESS_WORK(npc)->roll_counter<-1){
					NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

					ACCESS_WORK(npc)->roll_count=0;
					ACCESS_WORK(npc)->roll_counter=0;

					if(ACCESS_WORK(npc)->snake_status & SNAKE_STATUS_HAS_USP){
						ChangeNewPad(npc,PAD_TUMBLE_START_WUSP);
					}
					else if(ACCESS_WORK(npc)->snake_status & SNAKE_STATUS_HAS_FMS){
						ChangeNewPad(npc,PAD_TUMBLE_START_WFMS);
					}
					else{
						ChangeNewPad(npc,PAD_TUMBLE_START);
					}
					ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

					return 0;
				}
				else{
					// NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_SHORT_COUNT);
					ACCESS_WORK(npc)->roll_count=ROLLING_TIME;
				}
			}
			else{
				// NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_SHORT_COUNT);
				ACCESS_WORK(npc)->roll_count=ROLLING_TIME;
				ACCESS_WORK(npc)->roll_counter=0;
			}
#endif

			if(a.vx*npc->body->objs->world.m[2][0]+a.vz*npc->body->objs->world.m[2][2]>0){
				ChangeNewPad(npc,PAD_DAMAGE_B_SHORT);
				// ACCESS_WORK(npc)->roll_counter++;
			}
			else{
				ChangeNewPad(npc,PAD_DAMAGE_F_SHORT);
				// ACCESS_WORK(npc)->roll_counter--;
			}

			return 1;
		}
		else{
			fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));
		}
	}
	else if(act->status & NPC_ACT_SNAKE_PUNCH_KICK){
		FVECTOR a;

		/* パンチキックを出している間は、押されたことだけ計算しておく */

		if(NPCSnake_CheckRaidenDisForPush(npc,&a)){
			/* ライデンに押された */
#if 0
			npc->ctrl->step.vx+=a.vx;
			npc->ctrl->step.vz+=a.vz;
#else
			ACCESS_WORK(npc)->damage_mov_v.vx=a.vx;
			ACCESS_WORK(npc)->damage_mov_v.vz=a.vz;
#endif
		}
		else{
			fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));
		}
	}

	return 0 ;
}


int NPCSnake_CheckDamageBETWEENSTAGES(NPCWORK *npc)
{
	TARGET	*def;
	NPCACT	*act;
	NPCTARGET	*trg;
	CAPTURE_TARGET	*cap;
	long64	weapon;
	int		dam_child_num;
	int     needl_part=HUMAN21_MUNE;

#ifdef DEBUG_MODE
	extern int npcsnake_nodamage_flag;
#endif

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	weapon = 0 ;

#ifdef DEBUG_MODE
	if(npcsnake_nodamage_flag==1){
		NPC_DamageFlagClear( npc );		/* ダメージのみクリア */
		goto next;
	}
#endif

	if(GM_CheckGameStatus(STATE_SCN_DEMO)){
		NPC_DamageCaptureFlagClear(npc);
		return 0;
	}


	ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_LASERSIGHT;

	if((dam_child_num=NPC_PowerAndSeekChildTargetCheck(npc))>=0){
		weapon=(trg->def_child+dam_child_num)->weapon_type;
		trg->dam_trg = trg->def_child + dam_child_num ;
		trg->dam_obj = trg->connect_obj[ dam_child_num ] ;
		needl_part=trg->connect_obj[dam_child_num];
	}
	else if((TARGET_POWER|TARGET_SEEK) & def->damaged){
		trg->dam_trg = def;
		trg->dam_obj = HUMAN21_MUNE;
		weapon=def->weapon_type;
		needl_part=HUMAN21_MUNE;
	}

	/* スネークの弾 */
	if((weapon & WP_NOPLAYER) && ((weapon & WP_USP) || (weapon & WP_M4))){
		DEBUG_PRINT_NPCSNAKE("Kill oneself\n");
		goto next;
	}

	if(weapon){
		DEBUG_PRINT_NPCSNAKE("npc damage weapon type [%lx]  \n",weapon);

//		act->damtrg = def ;
		NPC_DamageCaptureFlagClear( npc ) ;

		if(weapon & ~(WP_MIC|WP_PLAYER|WP_NOPLAYER|WP_HANG|WP_BODY|WP_TUMBLE|WP_ONCORPSE|
					  WP_LASERSIGHT|WP_WEAPONCORE|WP_BOOK)){

			/* このフレームではダメージを受けない */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

			ACCESS_WORK(npc)->snake_status
				&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

			MOTION_CANCELABLE_ON(npc);
			UNLOCK_ENEMY(npc);

			NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

			ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
			ActSubMotionReset(ACCESS_WORK(npc));

			if(!(weapon & WP_NOPLAYER)){
				ACCESS_WORK(npc)->ret_flag|=NPCSNAKE_RETFLAG_DAMAGED_FROM_PLAYER;
				ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
			}
			else{
				ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_DAMAGED_FOR_BEHIND;
			}

			if(weapon & WP_BLADE){
				/* 刀の振り当たり */
				DamageForBlade(npc,act,needl_part,&trg->dam_trg->hit);

				NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT);
				// NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT);
				NPCSnake_GetPlayerTargetPos(npc);

				ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			}
			else if(weapon & WP_BLADESTAB){
				/* 刀の突き当たり */
				DamageForBladeStab(npc,act,needl_part,&trg->dam_trg->hit);

				NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT);
				// NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT);
				NPCSnake_GetPlayerTargetPos(npc);

				ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			}
			else if(weapon & WP_BLADEFAINT){
				/* 峰撃ち */
				DamageForBladeFaint(npc,act);

				NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT);
				// NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT);
				NPCSnake_GetPlayerTargetPos(npc);

				ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			}
			else if(weapon & WP_PUNCHALL){
				/* パンチの場合 */
				DamageForPunch(npc,act,weapon);

				NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_VSHORT_COUNT);

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT);
				ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				// NPCSnake_GetPlayerTargetPos(npc);
			}
			else if(weapon & WP_KICK){
				/* ふっ飛ぶモーションの場合 */
				DamageForKick(npc,act,weapon);

				// NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT);
				ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			}
			else if(weapon & (WP_STUNGRENADE|WP_STUNFAR)){
				/* スタングレネードによる気絶 */
				DamageForStunGrenade(npc,act);
			}
			else if( weapon & WP_COLDSPRAY){
				ReactionForColdSpray(npc,act);

				if(ACCESS_WORK(npc)->spray_count>0){
					NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_VLONG_COUNT);
				}
				else{
					NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_SHORT_COUNT);
				}
				ACCESS_WORK(npc)->spray_count=SPRAY_TIME;

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT);
				ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			}
			else if(weapon & WP_BLOW){
				/* ふっ飛ぶモーションの場合 */
				DamageForBlow(npc,act);

				// NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT);
				ACCESS_WORK(npc)->snake_status
					&=~(SNAKE_STATUS_ACTION_FINISHED|SNAKE_STATUS_SHOOT_ACTION_FINISHED);
				NPCSnake_GetPlayerTargetPos(npc);
			}
			else if ( weapon & (WP_M92|WP_PSG1T) ) {
				/* 麻酔弾 */
				ReactionForNeedl(npc,act,needl_part,&trg->dam_trg->hit);
			}
			else{
				/* その他よろけダメージの場合 */
				DamageNormal(npc,act,needl_part,&trg->dam_trg->hit);

				NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_SHORT_COUNT);

				SET_SCN_DAMAGE();

				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT);
				ACCESS_WORK(npc)->snake_status
					&=~(SNAKE_STATUS_ACTION_FINISHED|SNAKE_STATUS_SHOOT_ACTION_FINISHED);
				NPCSnake_GetPlayerTargetPos(npc);
			}
		}
		else{
			if( weapon & WP_LASERSIGHT ){
				/* レーザーサイトが当たってる */
				ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_LASERSIGHT;
				return 0;
			}
			else if(weapon & WP_WEAPONCORE){
				// マガジンを当てた場合の反応
				act->adj_piku_time=SHORT_REACTION_COUNT;

				/* レーザーサイトが当たってる時と同じ反応をさせる */
				ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_LASERSIGHT;

				return 0;
			}
			else goto next;
		}

		return 1 ;
	}

next:

#if 1
	if ( TARGET_CAPTURE & def->damaged ) {
		NPC_DamageFlagClear( npc ) ;	/* ダメージのみクリア */
		cap = trg->capture ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		ACCESS_WORK(npc)->snake_status
			&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

		NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

		MOTION_CANCELABLE_ON(npc);
		SHOOT_ACTION_FINISHED(npc);
		UNLOCK_ENEMY(npc);

		ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
		ActSubMotionReset(ACCESS_WORK(npc));

		if(cap->capture!=NULL){
			if((int)(cap->capture->ctrl)==(int)GM_PlayerControl){
				ACCESS_WORK(npc)->ret_flag|=NPCSNAKE_RETFLAG_CAPTURED_FROM_PLAYER;
				ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_CAPTURED_BY_RAIDEN;

				DEBUG_PRINT_NPCSNAKE("Hang by Player\n");
			}
			else{
				ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_CAPTURED_BY_RAIDEN;
			}
		}

		if ( cap->flag & CAPTURE_HANG ) {	/* 首絞め */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE  ) ;
			act->down_side=NPC_DOWN_SIDE_F;
			NPC_SetModeFromPad( npc, NPC_ActHang, npc->capture_mar, NPC_MOT_HANG_TIE, act->pad ) ;
		}
		else if ( cap->flag & CAPTURE_THROW ){	/* 投げ */
			act->faint-=KNOCKDOWN_FAINT_POINT(npc);
			act->down_side=NPC_DOWN_SIDE_F;
			NPC_SetModeFromPad( npc, NPC_ActThrow, npc->capture_mar, NPC_MOT_THROW, act->pad ) ;
		}

		SET_SCN_DAMAGE();

		NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT);
		ACCESS_WORK(npc)->snake_status
			&=~(SNAKE_STATUS_ACTION_FINISHED|SNAKE_STATUS_SHOOT_ACTION_FINISHED);

		cap->flag = 0 ;

#if 0

#ifdef DEBUG_MODE
		if(npcsnake_nodamage_flag==0) act->life -= THROW_DAMAGE_POINT(npc) ;
#else
		act->life -= THROW_DAMAGE_POINT(npc) ;
#endif

#endif

		return 1 ;
	}
#endif

	NPC_DamageCaptureFlagClear(npc);

	if(!(act->status & NPC_ACT_STATUS_TRG_OFF) &&
	   !(act->status & NPC_ACT_STATUS_DAMAGE) &&
	   !(act->status & NPC_ACT_SNAKE_FORCE_MOVE)){

		FVECTOR a;

		if(NPCSnake_CheckRaidenDisForPush(npc,&a)){
			/* ライデンに押された */

#if 0
			npc->ctrl->step.vx+=a.vx;
			npc->ctrl->step.vz+=a.vz;
#else
			ACCESS_WORK(npc)->damage_mov_v.vx=a.vx;
			ACCESS_WORK(npc)->damage_mov_v.vz=a.vz;
#endif

			/* パンチキックを出している間は、押されたことだけ計算しておく */
			if(act->status & NPC_ACT_SNAKE_PUNCH_KICK) return 0;

			/* このフレームではダメージを受けない */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE ) ;

			// ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

			if(ACCESS_WORK(npc)->roll_count>0){
				if(ACCESS_WORK(npc)->roll_counter>1 || ACCESS_WORK(npc)->roll_counter<-1){
					NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);
					ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_START_ROLLING;

					ACCESS_WORK(npc)->roll_count=0;
					ACCESS_WORK(npc)->roll_counter=0;

					return 0;
				}
				else{
					// NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_SHORT_COUNT);
					ACCESS_WORK(npc)->roll_count=ROLLING_TIME;
				}
			}
			else{
				// NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_SHORT_COUNT);
				ACCESS_WORK(npc)->roll_count=ROLLING_TIME;
				ACCESS_WORK(npc)->roll_counter=0;
			}

			NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

			ACCESS_WORK(npc)->snake_status
				&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

			MOTION_CANCELABLE_ON(npc);
			UNLOCK_ENEMY(npc);

			ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
			ActSubMotionReset(ACCESS_WORK(npc));

			if(a.vx*npc->body->objs->world.m[2][0]+a.vz*npc->body->objs->world.m[2][2]>0){
				ChangeNewPad(npc,PAD_DAMAGE_B_SHORT);
				fpu_CopyVector(&(ACCESS_WORK(npc)->neighbor_zone_using),
							   &(ACCESS_WORK(npc)->neighbor_zone));

				ACCESS_WORK(npc)->roll_counter++;
			}
			else{
				ChangeNewPad(npc,PAD_DAMAGE_F_SHORT);
				fpu_CopyVector(&(ACCESS_WORK(npc)->neighbor_zone_using),
							   &(ACCESS_WORK(npc)->neighbor_zone));

				ACCESS_WORK(npc)->roll_counter--;
			}

			return 1;
		}
		else{
			fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));
		}
	}
	else if(act->status & NPC_ACT_SNAKE_PUNCH_KICK){
		FVECTOR a;

		/* パンチキックを出している間は、押されたことだけ計算しておく */

		if(NPCSnake_CheckRaidenDisForPush(npc,&a)){
			/* ライデンに押された */
#if 0
			npc->ctrl->step.vx+=a.vx;
			npc->ctrl->step.vz+=a.vz;
#else
			ACCESS_WORK(npc)->damage_mov_v.vx=a.vx;
			ACCESS_WORK(npc)->damage_mov_v.vz=a.vz;
#endif
		}
		else{
			fpu_ClearVector(&(ACCESS_WORK(npc)->damage_mov_v));
		}
	}

	return 0 ;
}


static void PlayerVibEffect(long64 weapon)
{
	if(weapon & WP_NOPLAYER) return;

	if(weapon & WP_BLADE){
		NewPadVibration2(VIBDATA_BLADE,0);
	}
	else if(weapon & WP_BLADESTAB){
		NewPadVibration2(VIBDATA_BLADE_STAB,0);
	}
	else if(weapon & WP_BLADEFAINT){
		NewPadVibration2(VIBDATA_BLADE_FAINT,0);
	}
	else if(weapon & WP_PUNCHALL){
		switch(PL_GetPlayerWeapon()){
		case WP_m4:
		case WP_Aks:
		case WP_Rgb6:
			NewPadVibration2(VIBDATA_PUNCH_GUN,0);
			break;
		case WP_Nikita:
			NewPadVibration2(VIBDATA_PUNCH_NIKITA,0);
			break;
		default:
			NewPadVibration2(VIBDATA_PUNCH,0);
			break;
		}
	}
	else if(weapon & WP_KICK){
		if(PL_GetPlayerWeapon()==WP_Nikita){
			NewPadVibration2(VIBDATA_PUNCH_NIKITA,0);
		}
		else if(weapon & WP_KICK1){
			NewPadVibration2(VIBDATA_KICK,0);
		}
		else{
			NewPadVibration2(VIBDATA_2ND_KICK,0);
		}
	}
}

int NPCSnake_CheckDamageW14A(NPCWORK *npc)
{
	TARGET	*def;
	NPCACT	*act ;
	NPCTARGET	*trg ;
	long64	weapon ;
	int		dam_child_num ;
	int needl_part=HUMAN21_MUNE;

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	weapon = 0 ;

	if(GM_CheckGameStatus(STATE_SCN_DEMO)){
		NPC_DamageCaptureFlagClear(npc);
		return 0;
	}


	ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_LASERSIGHT;

	if((dam_child_num=NPC_PowerAndSeekChildTargetCheck(npc))>=0){
		trg->dam_trg = trg->def_child + dam_child_num ;
		trg->dam_obj = trg->connect_obj[ dam_child_num ] ;
		weapon=(npc->target.def_child+dam_child_num)->weapon_type;
		needl_part=trg->connect_obj[dam_child_num];
	}
	else if((TARGET_POWER|TARGET_SEEK) & def->damaged){
		trg->dam_trg = def;
		trg->dam_obj = HUMAN21_MUNE;
		weapon=def->weapon_type;
		needl_part=HUMAN21_MUNE;
	}

	NPC_DamageFlagClear(npc);

	if(weapon){
		DEBUG_PRINT_NPCSNAKE("npc damage weapon type [%lx]  \n",weapon);

		if((weapon & ~(WP_MIC|WP_PLAYER|WP_NOPLAYER|WP_HANG|WP_BODY|WP_TUMBLE|WP_ONCORPSE|
					   WP_LASERSIGHT|WP_WEAPONCORE)) &&
		   !NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){

			/* このフレームではダメージを受けない */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

			ACCESS_WORK(npc)->snake_status
				&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

			// ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ORDER_TO_SNIPE;

			MOTION_CANCELABLE_ON(npc);
			UNLOCK_ENEMY(npc);

			NPCSnake_GetPlayerTargetPos(npc);

			if(!(weapon & WP_NOPLAYER)){
				DEBUG_PRINT_NPCSNAKE("Raiden Attack\n");

				ACCESS_WORK(npc)->ret_flag|=NPCSNAKE_RETFLAG_DAMAGED_FROM_PLAYER;
				ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
			}

			PlayerVibEffect(weapon);

			NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

			if ( weapon & (WP_M92|WP_PSG1T) ) {
				NPC_SetNeedl( npc->body, needl_part, &trg->dam_trg->hit, MASUIDAN_MODEL ) ;

				GM_SeSetMode(SD_A_M_NINE01,
							 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

				if(NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))!=NPCSNAKE_ACT_W14A_3){
					NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W14A_3);

					/* ZZZZzzzz */
					NPC_CallHeadMark(npc,HMK2_TYPE_KILL);
					NPC_CallHeadMark(npc,HMK2_TYPE_PIYO_A);
					ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_ALREADY_DISP_QUESTION_MARK;

					ZZZSOUND_ON(ACCESS_WORK(npc));
				}

				ACCESS_WORK(npc)->w14a_count=0;

				// NPC_SetModeFromPad( npc, ActSleepDown, npc->base_mar, PAD_SLEEP_B, act->pad ) ;
			}
			else if(weapon & (WP_STUNGRENADE|WP_STUNFAR)){
				/* うぉっ */
				// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG01);
				GM_SeSetMode(SD_V_SNADMG01,
							 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

				if(NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))!=NPCSNAKE_ACT_W14A_4){
					NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W14A_4);

					/* ☆☆☆ */
					NPC_CallHeadMark(npc,HMK2_TYPE_KILL);
					NPC_CallHeadMark(npc,HMK2_TYPE_PIYO);
					ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_ALREADY_DISP_QUESTION_MARK;
				}
				ACCESS_WORK(npc)->w14a_count=0;
			}
			else{
				ChangeNewPad(npc,PAD_SITTING_AND_DAMAGED);

				if(!(weapon & (WP_BLOW|WP_THROWG))){
					SnakeBlood(npc->body,needl_part,&trg->dam_trg->hit,0);
				}

				if(weapon & WP_PUNCHALL){
					/* パンチの場合 */
					switch(PL_GetPlayerWeapon()){
					case WP_None:
						GM_SeSetMode(SD_P_PUNCH02,
									 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

						NewPadVibration2(VIBDATA_PUNCH,0);
						break;
					case WP_m4:
					case WP_Aks:
					case WP_Rgb6:
						GM_SeSetMode(SD_P_GUNPNC01,
									 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

						NewPadVibration2(VIBDATA_PUNCH_GUN,0);
						break;
					case WP_Nikita:
						GM_SeSetMode(SD_P_GUNPNC01,
									 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

						NewPadVibration2(VIBDATA_PUNCH_NIKITA,0);
						break;
					default:
						GM_SeSetMode(SD_P_GUNPNC01,
									 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

						NewPadVibration2(VIBDATA_PUNCH,0);
						break;
					}
				}
				else if(weapon & WP_KICK){
					/* キックの場合 */
					if(GM_CheckPlayerStatus(PLAYER_ROLLING)){
						GM_SeSetMode(SD_P_KICK02,
									 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

						NewPadVibration2(VIBDATA_2ND_KICK,0);
					}
					else{
						GM_SeSetMode(SD_P_KICK02,
									 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

						if(weapon & WP_KICK1) NewPadVibration2(VIBDATA_KICK,0);
						else NewPadVibration2(VIBDATA_2ND_KICK,0);
					}
				}

				/* うっ */
				// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG02);
				GM_SeSetMode(SD_V_SNADMG02,(FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

				NPC_CallHeadMark(npc,HMK2_TYPE_KILL);
				NPC_ClearNeedl(npc->body);

				if(NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))==NPCSNAKE_ACT_W14A_2 ||
				   NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))==NPCSNAKE_ACT_W14A_2_1 ||
				   NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))==NPCSNAKE_ACT_W14A_2_INTERVAL){

					NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W14A_2_1);
					ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;
					ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_START_QUATATION_MARK;
					ACCESS_WORK(npc)->shoot_count=ACCESS_WORK(npc)->shoot_limit;
				}
				else if(NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))==NPCSNAKE_ACT_W14A_1){
					NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W14A_2);
					ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;
					ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_START_QUATATION_MARK;
					ACCESS_WORK(npc)->shoot_count=ACCESS_WORK(npc)->shoot_limit;
				}
				else if(ACCESS_WORK(npc)->snake_status2 &
						SNAKE_STATUS2_ALREADY_DISP_QUESTION_MARK){

					NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W14A_2);
					ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;
					ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_START_QUATATION_MARK;
					ACCESS_WORK(npc)->shoot_count=ACCESS_WORK(npc)->shoot_limit;
				}
				else{
					NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W14A_0);
					ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_START_QUESTION_MARK;
				}

				ACCESS_WORK(npc)->w14a_count=0;
			}
		}
		else if( weapon & WP_LASERSIGHT ){
			/* レーザーサイトが当たってる */
			ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_LASERSIGHT;

#if 0
			DEBUG_PRINT_NPCSNAKE("%f %f %f\n",
								 ACCESS_WORK(npc)->homing_p->vx,
								 ACCESS_WORK(npc)->homing_p->vy,
								 ACCESS_WORK(npc)->homing_p->vz);

			DEBUG_PRINT_NPCSNAKE("LaserSight\n");
#endif

			return 0;
		}
		else if(weapon & WP_WEAPONCORE){
			// マガジンを当てた場合の反応
			act->adj_piku_time=SHORT_REACTION_COUNT;

			// レーザーサイトと同じ反応にする
			ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_LASERSIGHT;

			return 0;
		}
		else return 0;

		return 1 ;
	}

	return 0 ;
}

int NPCSnake_CheckDamageW17A(NPCWORK *npc)
{
	NPCACT	*act ;

	act=&npc->action;

	NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);

	return 0;
}

void NPCSnake_CheckPlayerCapture(Work *work)
{
	switch(work->stage){
	case NPCSNAKE_STAGE_W43A:
	case NPCSNAKE_STAGE_W44A:
	case NPCSNAKE_STAGE_W45A:
	case NPCSNAKE_STAGE_BETWEEN_STAGES:
		if(work->npc.action.current_mar!=work->cap_marfile){
			work->snake_status2&=~SNAKE_STATUS2_CALLED_CAPTURE_VIB;
			return;
		}
		
		if(work->npc.action.current_mot!=NPC_MOT_HANG_RESISIT) return;

		if(work->snake_status2 & SNAKE_STATUS2_CALLED_CAPTURE_VIB) return;

		if(!(work->snake_status2 & SNAKE_STATUS2_CAPTURED_BY_RAIDEN)) return;

		NewPadVibration2(VIBDATA_CAPTURE,0);

		work->snake_status2|=SNAKE_STATUS2_CALLED_CAPTURE_VIB;
		break;
	}
}
