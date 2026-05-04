//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_decide.c
		NPCスネークメインルーチン(w14a,w17a,w32a,w44a,w45a)

	2001/04/21 K.Kano
	$Id: snake_decide.c,v 1.1.1.3 2002/11/19 11:43:22 Yoshizawa1 Exp $
 */


#include "npc_snake.h"


extern const int npcsnake_usp_shoot_limit[NPC_SNAKE_SUPPORT_LEVEL_MAX];
extern const int npcsnake_fms_shoot_limit[NPC_SNAKE_SUPPORT_LEVEL_MAX];
extern const int npcsnake_psg1_shoot_limit[NPC_SNAKE_SUPPORT_LEVEL_MAX];


#define FIRST_SHOOT_DIS(work)		(first_shoot_dis[(work)->support_level])
#define SECOND_SHOOT_DIS(work)		(second_shoot_dis[(work)->support_level])
#define PUNCH_KICK_DIS				CVM2N(1.3f)

#define MOVESHOOT_DIS				CVM2N(12.0f)
#define MOVESHOOT_ANGLE				((int)(5.0f/360.0f*4096.0f))


#define LASERSIGHT_OFF_TIME			DIRECT_TICK((int)(60.0f*6.0f))		/* 6秒 */
#define W14A_ZZZ_SLEEP_TIME			DIRECT_TICK((int)(60.0f*30.0f))	/* 30秒 */
#define W14A_PIYO_TIME				DIRECT_TICK((int)(60.0f*30.0f))	/* 30秒 */


#define USP_SHOOT_LIMIT(work)		(npcsnake_usp_shoot_limit[(work)->support_level])
#define FMS_SHOOT_LIMIT(work)		(npcsnake_fms_shoot_limit[(work)->support_level])
#define PSG_SHOOT_LIMIT(work)		(npcsnake_psg1_shoot_limit[(work)->support_level])


/* ビハインドモード時に同じ場所に留まり続ける時間 */
#define BEHIND_RAIDEN_STAY_COUNT	DIRECT_TICK((int)(60.0f*2.0f))


/* 寝るマークが出るまでの時間 */
#define W14A_FIND_ZZZ_FRAME			DIRECT_TICK((int)(10.0f*60.0f))			/* 10秒 */

/* 連続撃ちモードに入った時の連射の間の間隔 */
#define W14A_SHOOT_INTERVAL			DIRECT_TICK((int)(1.0f*60.0f))			/* 1秒 */

/* 連続撃ちモードに入った時の連射の間の間隔 */
#define BETWEEN_STAGES_SHOOT_INTERVAL		DIRECT_TICK((int)(1.0f*60.0f))	/* 1秒 */


/* 狙撃のメッセージを受けて、実際狙撃するまでの時間 */
#define SNIPE_TIME(work)			(work->w32a_snipe_first_time)

/* 狙撃後次の狙撃が出来るまでの時間 */
#define AFTER_SNIPE_TIME(work)		(work->w32a_snipe_interval_time)

/* 狙撃スネークが寝ている期間 */
#define W32A_SNAKE_SLEEP_COUNTER	DIRECT_TICK(60*30)			/* 30秒 */


static const float first_shoot_dis[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
#if 0
	CVM2N(10.0f),
	CVM2N(12.0f),
	CVM2N(14.0f),
	CVM2N(16.0f),
	CVM2N(18.0f),
	CVM2N(19.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
#else
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
#endif
	/* STRONG */
};

static const float second_shoot_dis[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
#if 0
	CVM2N(15.0f),
	CVM2N(17.0f),
	CVM2N(19.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
#else
	CVM2N(10.0f),
	CVM2N(12.0f),
	CVM2N(14.0f),
	CVM2N(16.0f),
	CVM2N(18.0f),
	CVM2N(19.0f),
	CVM2N(20.0f),
	CVM2N(20.0f),
#endif
	/* STRONG */
};


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static int ChangeWeapon(Work *work)
{
	NPCWORK	*npc;

	npc=&(work->npc);

	if(!(work->snake_status & SNAKE_STATUS_ACTION_FINISHED) ||
	   !(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)) return 0;

	switch(work->snake_status & (SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS|
								 SNAKE_STATUS_HAS_USP|SNAKE_STATUS_HAS_FMS)){
	case SNAKE_STATUS_HAS_USP:
	case SNAKE_STATUS_HAS_FMS:
		// printf("Check 1\n");

		switch(npc->action.set_pad){
		case PAD_STAND_WUSP:
		case PAD_STAND_WFMS:
			ChangeNewPad(npc,PAD_STAND);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_SITDOWN_WUSP:
		case PAD_SITDOWN_WFMS:
			ChangeNewPad(npc,PAD_SITTING);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_WALK_WUSP:
		case PAD_WALK_WFMS:
			ChangeNewPad(npc,PAD_WALK);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_RUN_WUSP:
		case PAD_RUN_WFMS:
			ChangeNewPad(npc,PAD_RUN);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_WUSP:
		case PAD_BEHIND_WFMS:
			ChangeNewPad(npc,PAD_BEHIND);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_AND_SITTING_WUSP:
		case PAD_BEHIND_AND_SITTING_WFMS:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		}
		break;
	case SNAKE_STATUS_USE_USP:
		if(work->opt_shoot_limit==0){
			work->shoot_limit=USP_SHOOT_LIMIT(work);
		}

		switch(npc->action.set_pad){
		case PAD_STAND:
			ChangeNewPad(npc,PAD_STAND_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_SITDOWN:
			ChangeNewPad(npc,PAD_SITTING_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_WALK:
			ChangeNewPad(npc,PAD_WALK_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_RUN:
			ChangeNewPad(npc,PAD_RUN_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND:
			ChangeNewPad(npc,PAD_BEHIND_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		}
		break;
	case SNAKE_STATUS_USE_FMS:
		if(work->opt_shoot_limit==0){
			work->shoot_limit=FMS_SHOOT_LIMIT(work);
		}

		switch(npc->action.set_pad){
		case PAD_STAND:
			ChangeNewPad(npc,PAD_STAND_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_SITDOWN:
			ChangeNewPad(npc,PAD_SITTING_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_WALK:
			ChangeNewPad(npc,PAD_WALK_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_RUN:
			ChangeNewPad(npc,PAD_RUN_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND:
			ChangeNewPad(npc,PAD_BEHIND_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		}
		break;
	case SNAKE_STATUS_HAS_USP|SNAKE_STATUS_USE_FMS:
		if(work->opt_shoot_limit==0){
			work->shoot_limit=FMS_SHOOT_LIMIT(work);
		}

		switch(npc->action.set_pad){
		case PAD_STAND_WUSP:
			ChangeNewPad(npc,PAD_STAND_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_SITDOWN_WUSP:
			ChangeNewPad(npc,PAD_SITTING_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_WALK_WUSP:
			ChangeNewPad(npc,PAD_WALK_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_RUN_WUSP:
			ChangeNewPad(npc,PAD_RUN_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_WUSP:
			ChangeNewPad(npc,PAD_BEHIND_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_AND_SITTING_WUSP:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WFMS);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		}
		break;
	case SNAKE_STATUS_HAS_FMS|SNAKE_STATUS_USE_USP:
		if(work->opt_shoot_limit==0){
			work->shoot_limit=USP_SHOOT_LIMIT(work);
		}

		switch(npc->action.set_pad){
		case PAD_STAND_WFMS:
			ChangeNewPad(npc,PAD_STAND_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_SITDOWN_WFMS:
			ChangeNewPad(npc,PAD_SITTING_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_WALK_WFMS:
			ChangeNewPad(npc,PAD_WALK_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_RUN_WFMS:
			ChangeNewPad(npc,PAD_RUN_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_WFMS:
			ChangeNewPad(npc,PAD_BEHIND_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		case PAD_BEHIND_AND_SITTING_WFMS:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WUSP);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			return 1;
		}
		break;
	}

	return 0;
}

static void Mode_ToHide(Work *work)
{
	if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_MOVEPOS &&
	   NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY){

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEPOS);
		work->movepos_count=0;
	}
	NPCSNAKE_SHOOT_RESET(work);

	if(work->hide_pos_index<0){
		DEBUG_PRINT_NPCSNAKE("-- +MOVE ??\n");

		NPCSNAKE_HIDE_RESET(work);
		work->hide_count=0;

		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		return;
	}

	switch(NPCSNAKE_HIDE_MODE(work)){
	case NPCSNAKE_HIDE_NO_HIDING:
	case NPCSNAKE_HIDE_ATTACK:
		DEBUG_PRINT_NPCSNAKE("-- +HIDE !!   %f %f %f\n",
							 work->hide_pos->vx,work->hide_pos->vy,work->hide_pos->vz);

		/* HIDE状態が禁止されている時は、通常の動きを強制する */
		if(work->snake_status2 & SNAKE_STATUS2_PROHIBIT_TO_HIDE){
			NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_NO_HIDING);
		}
		else{
			NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_MOVE_TO_HIDE);
			work->hide_count=0;
		}
		// work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		break;
	case NPCSNAKE_HIDE_SITDOWN_TO_HIDE:
	case NPCSNAKE_HIDE_HIDING:
	case NPCSNAKE_HIDE_MOVE_TO_HIDE:
		// work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		break;
	case NPCSNAKE_HIDE_PEEPING:
	case NPCSNAKE_HIDE_BACK_TO_HIDE:
		break;
	}

	work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
}

enum {
	HIDE_FLAG_MOVEZONE=0,
	HIDE_FLAG_MOVEZONE2,
	HIDE_FLAG_MOVEHIDE,
	HIDE_FLAG_HIDING,
	HIDE_FLAG_OTHERS,

	/* SHOOT_ACTION_FINISHEDをチェックしない */
	SHOOT_FLAG_DISABLE=0x0010,
};


extern int npcsnake_noattack_flag;

static int DecideShootSetMove(Work *work,int hide_flag)
{
	switch(hide_flag){
	case HIDE_FLAG_HIDING:
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_BACK_TO_HIDE);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		break;
	case HIDE_FLAG_MOVEHIDE:
		Mode_ToHide(work);
		break;
	case HIDE_FLAG_MOVEZONE:
		/* 位置移動 */
		DEBUG_PRINT_NPCSNAKE("MOVE ZONE\n");

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEZONE);
		NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_NO_HIDING);
		NPCSNAKE_SHOOT_RESET(work);

		fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		break;
	case HIDE_FLAG_MOVEZONE2:
		/* 位置移動2 */
		DEBUG_PRINT_NPCSNAKE("MOVE ZONE 2\n");

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEZONE2);
		NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_NO_HIDING);
		NPCSNAKE_SHOOT_RESET(work);

		fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		break;
	default:
		return 0;
	}

	return 1;
}

static int DecideShoot(Work *work,int hide_flag)
{
	if(!(hide_flag & SHOOT_FLAG_DISABLE)){
		if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)) return 1;
	}

	hide_flag&=~SHOOT_FLAG_DISABLE;

	if(work->homing!=NULL){
		// DEBUG_PRINT_NPCSNAKE("------ 2 ------\n");

		switch(NPCSNAKE_SHOOT_MODE(work)){
		case NPCSNAKE_SHOOT:
			NPCSNAKE_SHOOT_RESET(work);

			/* 撃った回数のカウント */
			work->shoot_count--;

			if(work->shoot_count<0){
				// DEBUG_PRINT_NPCSNAKE("------ //// ------\n");

				if(DecideShootSetMove(work,hide_flag)){
					work->pre_homing=work->homing;
					return 1;
				}
				else{
					return 0;
				}
			}

			/* 敵との距離の判定 */
			if(work->pre_homing_part==-1){
				/* 前回の狙いが外れていた場合は、
				   弾の発射の判定を最初の判定距離で行う */

				// DEBUG_PRINT_NPCSNAKE("------ !!! ------\n");

				if(work->homing_dis<FIRST_SHOOT_DIS(work)){

#ifdef DEBUG_MODE
					if(npcsnake_noattack_flag!=1) goto next;
#endif

					NPCSNAKE_SHOOT_SET(work,NPCSNAKE_SHOOT);
					// work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
					// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

					work->pre_homing_part=work->homing_part;

					return 1;
				}
			}
			else{
				// DEBUG_PRINT_NPCSNAKE("------ <><><><> ------\n");

				if(work->homing_dis<SECOND_SHOOT_DIS(work)){

#ifdef DEBUG_MODE
					if(npcsnake_noattack_flag!=1) goto next;
#endif

					NPCSNAKE_SHOOT_SET(work,NPCSNAKE_SHOOT);
					// work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
					// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

					work->pre_homing_part=work->homing_part;

					return 1;
				}
			}
			break;
		default:
			/* 敵との距離の判定 */
			if(work->homing_dis<PUNCH_KICK_DIS &&
			   NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_NO_HIDING &&
			   !(work->snake_status & SNAKE_STATUS_SITTING_MOTION) &&
			   (work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){

#ifdef DEBUG_MODE
				if(npcsnake_noattack_flag==0) goto next;
#endif

				DEBUG_PRINT_NPCSNAKE("Punch !!\n");

				NPCSNAKE_SHOOT_SET(work,NPCSNAKE_PUNCH);
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;
				return 1;
			}
			else if(work->homing_dis<FIRST_SHOOT_DIS(work)){
				if(work->shoot_count>0){

#ifdef DEBUG_MODE
					if(npcsnake_noattack_flag!=1) goto next;
#endif

					NPCSNAKE_SHOOT_SET(work,NPCSNAKE_SHOOT);
					// work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
					// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

					work->pre_homing_part=work->homing_part;

					return 1;
				}
				else if(DecideShootSetMove(work,hide_flag)) return 1;
				else return 0;
			}
			break;
		}
	}

	// DEBUG_PRINT_NPCSNAKE("------ 3 ------ %d %f\n",work->shoot_limit,work->homing_dis);

next:
	NPCSNAKE_SHOOT_RESET(work);

	work->pre_homing=NULL;

	return 0;
}

static int DecideMoveAndShoot(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)) return 1;

	if(work->homing!=NULL){

		/* 敵との距離の判定 */
		if(work->homing_dis<PUNCH_KICK_DIS){
			DEBUG_PRINT_NPCSNAKE("Punch !!\n");

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);

#ifdef DEBUG_MODE
			if(npcsnake_noattack_flag==0) goto next;
#endif

			NPCSNAKE_SHOOT_SET(work,NPCSNAKE_PUNCH);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;
			return 1;
		}
		else if(work->homing_dis<FIRST_SHOOT_DIS(work)){

			if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY &&
			   work->movepos_count>=work->move_to_shoot_interval &&
			   work->shoot_count>0){

				int ang;

				work->movepos_count=0;
				ang=(work->ctrl.rot.vy-work->homing_dir) & 0xfff;
				
				if(work->homing_dis<MOVESHOOT_DIS ||
				   (MOVESHOOT_ANGLE<ang && ang<(0x1000-MOVESHOOT_ANGLE))){

					NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);
				}

#ifdef DEBUG_MODE
				if(npcsnake_noattack_flag!=1) goto next;
#endif

				NPCSNAKE_SHOOT_SET(work,NPCSNAKE_SHOOT);

				// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

				work->pre_homing_part=work->homing_part;

				return 1;
			}
		}
	}

next:
	NPCSNAKE_SHOOT_RESET(work);

	work->pre_homing=NULL;

	return 0;
}

static int DecidePunchKick(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)) return 1;

	if(work->homing!=NULL){

		/* 敵との距離の判定 */
		/* 天狗が気絶中で無いことを判定 */
		if(!(work->homing->status & HOMING_TNG_FAINT) &&
		   work->homing_dis<PUNCH_KICK_DIS){

			DEBUG_PRINT_NPCSNAKE("Punch !!\n");

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);

#ifdef DEBUG_MODE
			if(npcsnake_noattack_flag==0) goto next;
#endif

			NPCSNAKE_SHOOT_SET(work,NPCSNAKE_PUNCH);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;
			return 1;
		}
	}

next:
	return 0;
}

static int DecideActionForStay(Work *work)
{
	int hide_mode=NPCSNAKE_HIDE_MODE(work);

	static const int voice_attack[]={
		// スネークが飛び出し撃ちをしている間
		VOICE_INDEX_GO_RAIDEN0,
		// スネーク　　「今だ、行け！」
		VOICE_INDEX_GO_RAIDEN1,
		// スネーク　　「突っ込め！」
	};


	switch(hide_mode){
	case NPCSNAKE_HIDE_NO_HIDING:
#if 0
		if(work->mode!=NPCSNAKE_MODE_SEARCHENEMY &&
		   work->mode!=NPCSNAKE_MODE_INDEPEND){

			return 0;
		}
		break;
#else
		return 0;
#endif

	case NPCSNAKE_HIDE_MOVE_TO_HIDE:
		if(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED){
			NPCSNAKE_SHOOT_RESET(work);
		}
		break;
	case NPCSNAKE_HIDE_SITDOWN_TO_HIDE:
	case NPCSNAKE_HIDE_BACK_TO_HIDE:
		break;

	case NPCSNAKE_HIDE_HIDING:
		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			int cmp_hide_count;

			if(work->mode!=NPCSNAKE_MODE_SEARCHENEMY &&
			   work->mode!=NPCSNAKE_MODE_INDEPEND &&
			   work->mode!=NPCSNAKE_MODE_BEHIND_RAIDEN &&
			   work->mode!=NPCSNAKE_MODE_PICKUP_RAIDEN){

				return 0;
			}

			// DEBUG_PRINT_NPCSNAKE("------ 1 ------\n");

			if(work->snake_status2 & SNAKE_STATUS2_BE_CAPTURED_MODE) break;

			/* HIDE状態が禁止されている時は、通常の動きを強制する */
			if(work->snake_status2 & SNAKE_STATUS2_PROHIBIT_TO_HIDE) return 0;

			if(DecideShoot(work,HIDE_FLAG_MOVEZONE /* |SHOOT_FLAG_DISABLE */ )){
				work->hide_count=0;
				break;
			}

#if 0
			if(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED){
				NPCSNAKE_SHOOT_RESET(work);
			}
#endif

			if(work->snake_status & SNAKE_STATUS_WAIT_RAIDEN_OVERTIME &&
			   work->mode!=NPCSNAKE_MODE_PICKUP_RAIDEN){

				cmp_hide_count=work->hidding_to_peeping_interval2;
			}
			else{
				cmp_hide_count=work->hidding_to_peeping_interval;
			}

			if(work->hide_count>=cmp_hide_count){
				if(work->mode==NPCSNAKE_MODE_BEHIND_RAIDEN ||
				   work->mode==NPCSNAKE_MODE_PICKUP_RAIDEN) return 0;

				NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_PEEPING);
				NPCSNAKE_SHOOT_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			}
		}
		break;

	case NPCSNAKE_HIDE_PEEPING:
		// DEBUG_PRINT_NPCSNAKE("PEEPING\n");

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			// DEBUG_PRINT_NPCSNAKE("PEEPING 1\n");

			if(work->homing!=NULL){
				if(work->mode!=NPCSNAKE_MODE_SEARCHENEMY &&
				   work->mode!=NPCSNAKE_MODE_INDEPEND){

					return 0;
				}

#ifdef DEBUG_MODE
				if(npcsnake_noattack_flag!=1) goto next;
#endif

				NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_ATTACK);

				NPCSNAKE_VOICE_RNDCALL(work,voice_attack,
									   sizeof(voice_attack)/sizeof(voice_attack[0]));
				
next: ;
			}
			else{
				if(work->mode!=NPCSNAKE_MODE_SEARCHENEMY &&
				   work->mode!=NPCSNAKE_MODE_INDEPEND){

					return 0;
				}

				if(work->hide_count>=work->peeping_to_move_interval){
					// DEBUG_PRINT_NPCSNAKE("PEEPING 2\n");

					if(work->mode==NPCSNAKE_MODE_SEARCHENEMY ||
					   (work->snake_status2 & SNAKE_STATUS2_BE_CAPTURED_MODE)){

						NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_BACK_TO_HIDE);
						NPCSNAKE_SHOOT_RESET(work);
					}
					else{
						NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEPOS);
						NPCSNAKE_SHOOT_RESET(work);
						NPCSNAKE_HIDE_RESET(work);
					}
					work->movepos_count=0;
					work->hide_count=0;
					work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				}
			}
		}
		break;

	case NPCSNAKE_HIDE_ATTACK:
		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			if(!DecideShoot(work,HIDE_FLAG_HIDING /* |SHOOT_FLAG_DISABLE */)){
				if(work->mode!=NPCSNAKE_MODE_SEARCHENEMY &&
				   work->mode!=NPCSNAKE_MODE_INDEPEND){

					return 0;
				}

				if(work->mode==NPCSNAKE_MODE_SEARCHENEMY ||
				   (work->snake_status2 & SNAKE_STATUS2_BE_CAPTURED_MODE)){

					NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_BACK_TO_HIDE);
					NPCSNAKE_SHOOT_RESET(work);
				}
				else{
					NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEPOS);
					NPCSNAKE_SHOOT_RESET(work);
					NPCSNAKE_HIDE_RESET(work);
				}
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			}
		}
		break;
	}

	return 1;
}


static const int voice_angry[]={
	// ★ライデンは眠ったスネークを殴ることも出来る。スネークは怒る。
	VOICE_INDEX_ANGRY0,
	//スネーク　「何をする！」
	VOICE_INDEX_ANGRY1,
	// スネーク　「ふざけたことをするな！」
	VOICE_INDEX_ANGRY2,
	// スネーク　「何のつもりだ！」
};

static const int voice_counter_shoot[]={
	// ★ライデンがスネークを本当に撃つとスネークも撃ち返す。
	VOICE_INDEX_SHOOT_GUN0,
	// スネーク　「馬鹿者」
	VOICE_INDEX_SHOOT_GUN1,
	// スネーク　「ふざけるな」
	VOICE_INDEX_SHOOT_GUN2,
	// スネーク　「何を考えている」
};


static inline int DecideAction_COMMON(Work *work)
{
	if((work->stage==NPCSNAKE_STAGE_W44A ||
		work->stage==NPCSNAKE_STAGE_W45A) &&
	   (work->snake_status2 & SNAKE_STATUS2_DAMAGED_BY_RAIDEN)){

		DEBUG_PRINT_NPCSNAKE("Voice Call Counter\n");

#if 0
		NPCSNAKE_VOICE_RNDCALL(work,voice_counter_shoot,
							   sizeof(voice_counter_shoot)/sizeof(voice_counter_shoot[0]));

		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
#endif

	}

	if(ChangeWeapon(work)){
		NPCSNAKE_SHOOT_RESET(work);
		return 1;
	}

	// DEBUG_PRINT_NPCSNAKE("---1--- %d\n",NPCSNAKE_ACT_MODE(work));

	if(NPCSNAKE_ACT_MODE(work)==NPCSNAKE_ACT_STAY){
		if(DecideActionForStay(work)) return 1;
	}

	// DEBUG_PRINT_NPCSNAKE("---2--- %d %d\n",NPCSNAKE_ACT_MODE(work),work->mode);

	return 0;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


void DecideAction_SEARCHENEMY(Work *work)
{
	if(DecideAction_COMMON(work)) return;

	if(CHECK_SHOOT_AND_ACT_FINISH_FLAG(work)){
		if(DecideShoot(work,HIDE_FLAG_MOVEHIDE)) return;

		/* 何もなければ位置移動 */
		Mode_ToHide(work);
	}
	else{
		if(DecideMoveAndShoot(work)) return;

		NPCSNAKE_SHOOT_RESET(work);
	}
}

void DecideAction_INDEPEND(Work *work)
{
	static const int voice_independ[]={
		// スネークが突撃していく時
		VOICE_INDEX_SNAKE_GO0,
		// スネーク　　「行くぞ！」
		VOICE_INDEX_SNAKE_GO1,
		// スネーク　　「ついてこい！」
		VOICE_INDEX_SNAKE_GO2,
		// スネーク　　「遅れるな！」
	};

	if(DecideAction_COMMON(work)) return;

	if(CHECK_SHOOT_AND_ACT_FINISH_FLAG(work)){
		int hide_mode=HIDE_FLAG_MOVEHIDE;

		if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
			hide_mode=HIDE_FLAG_OTHERS;
		}

		if(DecideShoot(work,hide_mode)) return;

		if(!(work->snake_status2 & SNAKE_STATUS2_VOICE_INDEPEND)){
			NPCSNAKE_VOICE_RNDCALL(work,voice_independ,
								   sizeof(voice_independ)/sizeof(voice_independ[0]));

			work->snake_status2|=SNAKE_STATUS2_VOICE_INDEPEND;
		}

		/* 何もなければ位置移動 */
		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEPOS);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
	}
	else{
		if(!(work->snake_status2 & SNAKE_STATUS2_VOICE_INDEPEND)){
			NPCSNAKE_VOICE_RNDCALL(work,voice_independ,
								   sizeof(voice_independ)/sizeof(voice_independ[0]));

			work->snake_status2|=SNAKE_STATUS2_VOICE_INDEPEND;
		}

		if(DecideMoveAndShoot(work)) return;

		NPCSNAKE_SHOOT_RESET(work);
	}
}

void DecideAction_FOLLOW_RAIDEN(Work *work)
{
	if(DecideAction_COMMON(work)) return;

	// DEBUG_PRINT_NPCSNAKE("START !!!!!\n");

	if(CHECK_SHOOT_FINISH_FLAG(work)){
		NPCSNAKE_SHOOT_RESET(work);
		if(DecidePunchKick(work)) return;
	}
	if(CHECK_ACTION_FINISH_FLAG(work)){
		/* ライデンを追いかける */
		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_FLWRAIDEN);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		// DEBUG_PRINT_NPCSNAKE("END !!!!!\n");
	}
}

void DecideAction_BEHIND_RAIDEN(Work *work)
{
	static const int voice_enemy_arround[]={
		// ライデンが敵に囲まれている時
		VOICE_INDEX_ENEMY_ARROUND0,
		// スネーク　　「援護する！」
		VOICE_INDEX_ENEMY_ARROUND1,
		// スネーク　　「先に行け！」
	};
	static const int voice_enemy_arround2[]={
		// ライデンが敵に囲まれている時
		VOICE_INDEX_ENEMY_ARROUND0,
		// スネーク　　「援護する！」
	};

	if(DecideAction_COMMON(work)) return;

	// DEBUG_PRINT_NPCSNAKE("START b !!!!!\n");

	if(CHECK_SHOOT_AND_ACT_FINISH_FLAG(work)){
		int hide_mode=HIDE_FLAG_MOVEZONE;

		if((work->snake_status & SNAKE_STATUS_DAMAGED_FOR_BEHIND) &&
		   !(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT)){

			hide_mode=HIDE_FLAG_MOVEHIDE;
		}
		if(DecideShoot(work,hide_mode)){
			if(!(work->snake_status2 & SNAKE_STATUS2_VOICE_ENEMY_ARROUND)){
				if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
					NPCSNAKE_VOICE_RNDCALL(work,voice_enemy_arround2,
										   sizeof(voice_enemy_arround2)/sizeof(voice_enemy_arround2[0]));
				}
				else{
					NPCSNAKE_VOICE_RNDCALL(work,voice_enemy_arround,
										   sizeof(voice_enemy_arround)/sizeof(voice_enemy_arround[0]));
				}

				work->snake_status2|=SNAKE_STATUS2_VOICE_ENEMY_ARROUND;
			}
			return;
		}

		/* 何もなければその場で待機、もしくは位置移動 */
		if(work->homing==NULL &&

		   /* 2001/9/17 K.Kano 追加 */
		   !(work->raiden_status & NPCSNAKE_RAIDEN_ENEMY_EXIST)){

			/* HIDE状態が禁止されている時は、通常の動きを強制する */
			if((work->snake_status & SNAKE_STATUS_DAMAGED_FOR_BEHIND) &&
			   !(work->snake_status2 & SNAKE_STATUS2_PROHIBIT_TO_HIDE) &&
			   !(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT)){

				Mode_ToHide(work);
			}
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);
			}
		}
		else{
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEZONE);
			// NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);
			fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		}

		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		// DEBUG_PRINT_NPCSNAKE("MOVE !!!!!\n");
	}
	else{
		if(DecideMoveAndShoot(work)){
			if(!(work->snake_status2 & SNAKE_STATUS2_VOICE_ENEMY_ARROUND)){
				if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
					NPCSNAKE_VOICE_RNDCALL(work,voice_enemy_arround2,
										   sizeof(voice_enemy_arround2)/sizeof(voice_enemy_arround2[0]));
				}
				else{
					NPCSNAKE_VOICE_RNDCALL(work,voice_enemy_arround,
										   sizeof(voice_enemy_arround)/sizeof(voice_enemy_arround[0]));
				}

				work->snake_status2|=SNAKE_STATUS2_VOICE_ENEMY_ARROUND;
			}
			return;
		}

		NPCSNAKE_SHOOT_RESET(work);
	}
}

void DecideAction_PICKUP_RAIDEN(Work *work)
{
	static const int voice_enemy_arround2[]={
		VOICE_INDEX_ENEMY_ARROUND2,
		// スネーク　　「今行くぞ！」
	};

	if(DecideAction_COMMON(work)) return;

	if(CHECK_ACTION_FINISH_FLAG(work)){
		if(!(work->snake_status2 & SNAKE_STATUS2_VOICE_ENEMY_ARROUND2)){
			if(!(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT)){
				NPCSNAKE_VOICE_RNDCALL(work,voice_enemy_arround2,
									   sizeof(voice_enemy_arround2)/sizeof(voice_enemy_arround2[0]));
			}
			work->snake_status2|=SNAKE_STATUS2_VOICE_ENEMY_ARROUND2;
		}

		/* ライデンを追いかける */
		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_FLWRAIDEN);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		// DEBUG_PRINT_NPCSNAKE("END !!!!!\n");
	}
	else{
		if(!(work->snake_status2 & SNAKE_STATUS2_VOICE_ENEMY_ARROUND2)){
			if(!(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT)){
				NPCSNAKE_VOICE_RNDCALL(work,voice_enemy_arround2,
									   sizeof(voice_enemy_arround2)/sizeof(voice_enemy_arround2[0]));
			}

			work->snake_status2|=SNAKE_STATUS2_VOICE_ENEMY_ARROUND2;
		}

		if(DecideMoveAndShoot(work)) return;

		NPCSNAKE_SHOOT_RESET(work);
	}
}

void DecideAction_GURD_RAIDEN(Work *work)
{
	if(DecideAction_COMMON(work)) return;

	if(CHECK_SHOOT_AND_ACT_FINISH_FLAG(work)){
		if(DecideShoot(work,HIDE_FLAG_MOVEZONE2)) return;

		/* 何もなければ位置移動 */
		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEAROUNDRAIDEN);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
	}
	else{
		if(DecideMoveAndShoot(work)) return;

		NPCSNAKE_SHOOT_RESET(work);
	}
}


#define GURD_W44A_MOVEZONE2_LIMIT		CVM2N(5.0f)

/* 2001/9/19  K.Kano
   w44aでの最後の山場でのスネークの動きを改善
   新モードを追加 */
void DecideAction_GURD_RAIDEN_W44A(Work *work)
{
	if(DecideAction_COMMON(work)) return;

	if(CHECK_SHOOT_AND_ACT_FINISH_FLAG(work)){
		if(DecideShoot(work,HIDE_FLAG_MOVEZONE2)){
			if(work->raiden_dis>GURD_W44A_MOVEZONE2_LIMIT){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEAROUNDRAIDEN2);
			}
			return;
		}

		/* 何もなければ位置移動 */
		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEAROUNDRAIDEN2);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
	}
	else{
		if(DecideMoveAndShoot(work)) return;

		NPCSNAKE_SHOOT_RESET(work);
	}
}


void DecideAction_ATTACK_ENEMY(Work *work)
{
	if(DecideAction_COMMON(work)) return;

	if(CHECK_SHOOT_AND_ACT_FINISH_FLAG(work)){
		if(DecideShoot(work,HIDE_FLAG_MOVEZONE2)) return;

		/* 何もなければ位置移動 */
		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_MOVEPOS);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
	}
	else{
		if(DecideMoveAndShoot(work)) return;

		NPCSNAKE_SHOOT_RESET(work);
	}
}


void DecideAction_BETWEEN_STAGES(Work *work)
{
	// DEBUG_PRINT_NPCSNAKE("snake status = 0x%08x\n",work->snake_status);

	if(DecideAction_COMMON(work)) return;

	switch(NPCSNAKE_ACT_MODE(work)){
	case NPCSNAKE_ACT_FLWRAIDEN_ONLY:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

		if(work->snake_status2 & SNAKE_STATUS2_START_ROLLING){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ROLLING);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		}
#if 0
		else if(work->raiden_dis<CVM2N(0.5f) &&
				GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

			/* 気絶中のライデンを避ける */
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_AVOID_RAIDEN);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		}
#endif
		else if(work->snake_status & SNAKE_STATUS_LASERSIGHT){
			/* 白！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			NPCSnake_GetPlayerTargetPos(&(work->npc));

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);

#if 1
			NPCSNAKE_VOICE_RNDCALL(work,voice_angry,
								   sizeof(voice_angry)/sizeof(voice_angry[0]));
#endif

			work->w14a_count=0;
		}
#if 0
		else if(work->snake_status2 & SNAKE_STATUS2_CRASH_ZONE){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_AVOID_RAIDEN);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		}
#endif
		break;
	case NPCSNAKE_ACT_ROLLING:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

		SET_SCN_NORMAL();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_FLWRAIDEN_ONLY);
			work->snake_status2&=~SNAKE_STATUS2_START_ROLLING;
		}
		break;

#if 0
	case NPCSNAKE_ACT_AVOID_RAIDEN:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

		if(work->snake_status & SNAKE_STATUS_LASERSIGHT){
			/* 白！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			NPCSnake_GetPlayerTargetPos(&(work->npc));

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);

#if 1
			NPCSNAKE_VOICE_RNDCALL(work,voice_angry,
								   sizeof(voice_angry)/sizeof(voice_angry[0]));
#endif

			work->w14a_count=0;
		}
		else if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			if(work->snake_status2 & SNAKE_STATUS2_CRASH_ZONE){
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
			}
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_FLWRAIDEN_ONLY);
			}
		}
		break;
#endif

	case NPCSNAKE_ACT_POINT_TARGET:
		NPCSnake_GetPlayerTargetPos(&(work->npc));

		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_POINT_TARGET();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

#if 0
		if(work->raiden_dis<CVM2N(0.5f) &&
		   GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

			/* 気絶中のライデンを避ける */
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_AVOID_RAIDEN);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		}
		else
#endif

		if(!(work->snake_status & SNAKE_STATUS_LASERSIGHT)){
			if(work->raiden_dis>=CVM2N(3.0f)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_FLWRAIDEN_ONLY);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

				ActSubMotionReset(work);

#if 0
				if(work->snake_status & SNAKE_STATUS_HAS_USP){
					ChangeNewPad(&(work->npc),PAD_STAND_WUSP);
					ActSubMotionReset(work);
				}
				else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
					ChangeNewPad(&(work->npc),PAD_STAND_WFMS);
					ActSubMotionReset(work);
				}
#endif

			}
			else{
				work->w14a_count++;
				if(work->w14a_count>LASERSIGHT_OFF_TIME){
					NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_FLWRAIDEN_ONLY);
					NPCSNAKE_SHOOT_RESET(work);
					NPCSNAKE_HIDE_RESET(work);
					work->movepos_count=0;
					work->hide_count=0;
					work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

					ActSubMotionReset(work);

#if 0
					if(work->snake_status & SNAKE_STATUS_HAS_USP){
						ChangeNewPad(&(work->npc),PAD_STAND_WUSP);
						ActSubMotionReset(work);
					}
					else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
						ChangeNewPad(&(work->npc),PAD_STAND_WFMS);
						ActSubMotionReset(work);
					}
#endif
				}

			}
		}
		break;
	case NPCSNAKE_ACT_WAIT_POINTING:
		NPCSnake_GetPlayerTargetPos(&(work->npc));

		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_POINT_TARGET();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(!GM_CheckPlayerStatus(PLAYER_GROUND)){
			if(work->raiden_dis>CVM2N(5.0f)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				work->shoot_count=work->shoot_limit;
				break;
			}
			if(work->raiden_dis<CVM2N(1.0f) &&
			   !GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}

		    if(HZX_OnlineHazardCheck(work->ctrl.hzx_id,&(work->ctrl.mov),work->homing_p,
									 HZX_CHK_ALL,HZX_SEG_NO_BULLET|HZX_SEG_RECOIL_TYPE,
									 HZX_FLOOR_NO_BULLET|HZX_FLOOR_RECOIL_TYPE|HZX_FLOOR_IK)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
		    }
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
			}

			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;
		}
		break;
	case NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_DAMAGE();

#if 1
		NPCSNAKE_VOICE_RNDCALL(work,voice_counter_shoot,
							   sizeof(voice_counter_shoot)/sizeof(voice_counter_shoot[0]));
#endif

		if(work->raiden_dis<CVM2N(1.0f)){
			if(work->shoot_limit>=20){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
			}
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN);
			}
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			break;
		}

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		work->shoot_count=work->shoot_limit;

#ifdef DEBUG
		printf("Shoot Count = %d\n",work->shoot_count);
#endif

		break;
	case NPCSNAKE_ACT_SHOOT_RAIDEN:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		if(!(work->snake_status & SNAKE_STATUS_ACTION_FINISHED)) break;

		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		NPCSnake_GetPlayerTargetPos(&(work->npc));

		work->shoot_count--;
		if(work->shoot_count<0){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;

			work->shoot_limit+=2;
			if(work->shoot_limit>=20){
				// NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN_INTERVAL);
				work->shoot_limit=20;
			}
			else if(work->shoot_limit>=10){
				work->shoot_limit=20;
			}
			else if(work->shoot_limit>=7){
				work->shoot_limit=10;
			}
			work->shoot_count=work->shoot_limit;
		}
		break;
	case NPCSNAKE_ACT_SHOOT_RAIDEN_INTERVAL:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		NPCSnake_GetPlayerTargetPos(&(work->npc));

		if(work->raiden_status & NPCSNAKE_RAIDEN_DAMAGED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			break;
		}

		if(work->raiden_dis>=CVM2N(5.0f)){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;

#if 0
			if(work->snake_status & SNAKE_STATUS_HAS_USP){
				ChangeNewPad(&(work->npc),PAD_STAND_WUSP);
				ActSubMotionReset(work);
			}
			else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
				ChangeNewPad(&(work->npc),PAD_STAND_WFMS);
				ActSubMotionReset(work);
			}
#endif

			break;
		}

		if(work->raiden_dis<CVM2N(1.0f)){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			break;
		}

		work->w14a_count++;
		if(work->w14a_count>=BETWEEN_STAGES_SHOOT_INTERVAL &&
		   (work->ctrl.map & GM_CurrentChanlMap[0])){

			if(work->raiden_dis<CVM2N(1.0f) &&
			   !GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}
		    if(HZX_OnlineHazardCheck(work->ctrl.hzx_id,&(work->ctrl.mov),work->homing_p,
									 HZX_CHK_ALL,HZX_SEG_NO_BULLET|HZX_SEG_RECOIL_TYPE,
									 HZX_FLOOR_NO_BULLET|HZX_FLOOR_RECOIL_TYPE|HZX_FLOOR_IK)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
		    }
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
			}

			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;
		}
		break;

	case NPCSNAKE_ACT_ANGRY2:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		}
		break;

	case NPCSNAKE_ACT_PUNCH_RAIDEN2:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			if(work->raiden_status & NPCSNAKE_RAIDEN_DAMAGED){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}
			if(GM_CheckPlayerStatus(PLAYER_GROUND)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_WAIT_POINTING);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}
			if(work->raiden_dis>CVM2N(5.0f)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				work->shoot_count=work->shoot_limit;
				break;
			}
			if(work->raiden_dis<CVM2N(1.0f) &&
			   !(GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND))){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}

		    if(HZX_OnlineHazardCheck(work->ctrl.hzx_id,&(work->ctrl.mov),work->homing_p,
									 HZX_CHK_ALL,HZX_SEG_NO_BULLET|HZX_SEG_RECOIL_TYPE,
									 HZX_FLOOR_NO_BULLET|HZX_FLOOR_RECOIL_TYPE|HZX_FLOOR_IK)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
		    }
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
			}

			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;
		}
		break;


	case NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_DAMAGE();

#if 1
		NPCSNAKE_VOICE_RNDCALL(work,voice_counter_shoot,
							   sizeof(voice_counter_shoot)/sizeof(voice_counter_shoot[0]));
#endif

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		break;
	case NPCSNAKE_ACT_ANGRY:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		}
		break;
	case NPCSNAKE_ACT_PUNCH_RAIDEN:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_FLWRAIDEN_ONLY);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

			ActSubMotionReset(work);
		}
		break;
	}
}


void DecideAction_W14A(Work *work)
{
	static const int voice_brigngun[]={
		// ★ライデンが眠ったスネークに銃を向けると目を覚ます。
		VOICE_INDEX_BRING_GUN0,
		// スネーク　「（不敵に）若いの、何のつもりだ？」
		VOICE_INDEX_BRING_GUN1,
		// スネーク　「（不敵に）やめておけ」
		VOICE_INDEX_BRING_GUN2,
		// スネーク　「（不敵に）お前に撃てるのか？」
	};

	static const int voice_shootgun[]={
		// ★ライデンがスネークを本当に撃つとスネークも撃ち返す。
		VOICE_INDEX_SHOOT_GUN0,
		// スネーク　「馬鹿者」
		VOICE_INDEX_SHOOT_GUN1,
		// スネーク　「ふざけるな」
		VOICE_INDEX_SHOOT_GUN2,
		// スネーク　「何を考えている」
	};


	if(DecideAction_COMMON(work)) return;

	switch(NPCSNAKE_ACT_MODE(work)){
	case NPCSNAKE_ACT_W14A_0:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		/* 座り込んでいる */
		work->w14a_count++;
		if(work->w14a_count==W14A_FIND_ZZZ_FRAME){
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_ZZZ);
			ZZZSOUND_ON(work);
			// NPCSNAKE_VOICE_CALL(work,VOICE_INDEX_ZZZ);
			SET_SCN_SLEEP();
			work->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;
		}
		else if(work->w14a_count>W14A_FIND_ZZZ_FRAME){
			SET_SCN_SLEEP();
			work->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;
		}

		if(work->snake_status & SNAKE_STATUS_LASERSIGHT){
			/* 白！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_KILL);
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			NPCSNAKE_VOICE_STOP(work);
			NPCSNAKE_VOICE_RNDCALL(work,voice_brigngun,
								   sizeof(voice_brigngun)/sizeof(voice_brigngun[0]));

			NPCSnake_GetPlayerTargetPos(&(work->npc));

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_1);

			work->w14a_count=0;

			work->snake_status2&=~SNAKE_STATUS2_EVM_CLOSE_EYE;
			work->snake_status2|=SNAKE_STATUS2_EVM_FLUSH_EYE;
		}
		work->shoot_count=work->shoot_limit;
		break;
	case NPCSNAKE_ACT_W14A_1:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_POINT_TARGET();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		/* ライデンに向かって構える */
		NPCSnake_GetPlayerTargetPos(&(work->npc));

		ZZZSOUND_OFF(work);

		if(!(work->snake_status & SNAKE_STATUS_LASERSIGHT)){
			work->w14a_count++;
			if(work->w14a_count>LASERSIGHT_OFF_TIME){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_0);
			}
		}
		work->shoot_count=work->shoot_limit;
		break;
	case NPCSNAKE_ACT_W14A_2:
		NPCSNAKE_VOICE_RNDCALL(work,voice_shootgun,
							   sizeof(voice_shootgun)/sizeof(voice_shootgun[0]));
		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_2_1);

	case NPCSNAKE_ACT_W14A_2_1:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		/* ライデンに向かって撃つ */
		ZZZSOUND_OFF(work);

		if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)) break;

		work->shoot_count--;
		if(work->shoot_count<0){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_1);
			work->w14a_count=0;

			work->shoot_limit+=2;
			if(work->shoot_limit>=20){
				work->shoot_limit=20;
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_2_INTERVAL);
			}
			else{
				if(work->shoot_limit>=10){
					work->shoot_limit=20;
				}
				else if(work->shoot_limit>=7){
					work->shoot_limit=10;
				}
			}
			work->shoot_count=work->shoot_limit;
		}
		break;
	case NPCSNAKE_ACT_W14A_2_INTERVAL:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		SET_SCN_COUNTERATTACK();

		/* ライデンに向かって構える */
		NPCSnake_GetPlayerTargetPos(&(work->npc));

		ZZZSOUND_OFF(work);

		work->w14a_count++;
		if(work->w14a_count>W14A_SHOOT_INTERVAL &&
		   (work->ctrl.map & GM_CurrentChanlMap[0])){

		    if(HZX_OnlineHazardCheck(work->ctrl.hzx_id,&(work->ctrl.mov),work->homing_p,
									 HZX_CHK_ALL,HZX_SEG_NO_BULLET|HZX_SEG_RECOIL_TYPE,
									 HZX_FLOOR_NO_BULLET|HZX_FLOOR_RECOIL_TYPE|HZX_FLOOR_IK)){
				break;
		    }

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_2_1);
			work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;
		}
		break;
	case NPCSNAKE_ACT_W14A_3:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;
		work->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

		SET_SCN_SLEEP();

		/* 寝ています。ZZZZZZZZ */
		work->w14a_count++;
		if(work->w14a_count>W14A_ZZZ_SLEEP_TIME){
			/* 復活 */
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_0);
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_KILL);
			NPC_ClearNeedl(&(work->body));
			work->w14a_count=0;

			work->snake_status2&=~SNAKE_STATUS2_EVM_CLOSE_EYE;
			work->snake_status2|=SNAKE_STATUS2_EVM_FLUSH_EYE;
		}
		break;
	case NPCSNAKE_ACT_W14A_4:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;
		work->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

		SET_SCN_FAINT();

		/* 星三つ */
		work->w14a_count++;
		work->npc.action.headmark_num = 3 ;
		if(work->w14a_count>W14A_PIYO_TIME*1/3){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_5);
			*(work->npc.action.headmark) = HMK2_TYPE_PIYO_VALUE|2 ;
			work->npc.action.headmark_num = 2 ;
		}
		break;
	case NPCSNAKE_ACT_W14A_5:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;
		work->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

		SET_SCN_FAINT();

		/* 星二つ */
		work->w14a_count++;
		if(work->w14a_count>W14A_PIYO_TIME*2/3){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_6);
			*(work->npc.action.headmark) = HMK2_TYPE_PIYO_VALUE|1 ;
			work->npc.action.headmark_num = 1 ;
		}
		break;
	case NPCSNAKE_ACT_W14A_6:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;
		work->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

		SET_SCN_FAINT();

		/* 星一つ */
		work->w14a_count++;
		if(work->w14a_count>W14A_PIYO_TIME*3/3){
			/* 復活 */
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W14A_0);
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_KILL);
			NPC_ClearNeedl(&(work->body));
			work->w14a_count=0;

			work->snake_status2&=~SNAKE_STATUS2_EVM_CLOSE_EYE;
			work->snake_status2|=SNAKE_STATUS2_EVM_FLUSH_EYE;
		}
		break;
	}
}

void DecideAction_W17A(Work *work)
{
	if(DecideAction_COMMON(work)) return;

	// DEBUG_PRINT_NPCSNAKE("---3--- %d\n",NPCSNAKE_ACT_MODE(work));

	switch(NPCSNAKE_ACT_MODE(work)){
	case NPCSNAKE_ACT_W17A_0:
		GM_RadarResetFlag(&(work->rctrl),RADAR_VISIBLE);

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(!(work->raiden_status & NPCSNAKE_RAIDEN_LOOKS_SNAKE)){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_1);
		}
		break;
	case NPCSNAKE_ACT_W17A_1:
		GM_RadarResetFlag(&(work->rctrl),RADAR_VISIBLE);

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(work->raiden_status & NPCSNAKE_RAIDEN_LOOKS_SNAKE){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_2);
		}
		break;
	case NPCSNAKE_ACT_W17A_2:
		GM_RadarSetFlag(&(work->rctrl),RADAR_VISIBLE);

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(work->raiden_dis<work->w17a_dis){

#if 0
			/* 白！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			work->headmark_count++;

			DEBUG_PRINT_NPCSNAKE("HeadMark = %d\n",work->headmark_count);

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_3);

			work->gcounter=0;
#else
			/* 赤！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_RED_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			work->headmark_count++;

			DEBUG_PRINT_NPCSNAKE("HeadMark = %d\n",work->headmark_count);

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_5);
#endif

		}
		break;
	case NPCSNAKE_ACT_W17A_3:
		GM_RadarSetFlag(&(work->rctrl),RADAR_VISIBLE);

		SET_SCN_RUNAWAY();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->gcounter++;
		if(work->gcounter>W17A_RUN_COUNT){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_4);
			work->gcounter=0;
		}
		break;
	case NPCSNAKE_ACT_W17A_4:
		GM_RadarSetFlag(&(work->rctrl),RADAR_VISIBLE);

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->gcounter++;
		if(work->raiden_dis<work->w17a_dis){
			/* 赤！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_RED_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			work->headmark_count++;

			DEBUG_PRINT_NPCSNAKE("HeadMark = %d\n",work->headmark_count);

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_5);

			work->gcounter=0;
		}
		else if(work->gcounter>W17A_IDLE_COUNT){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_2);
			work->gcounter=0;
		}
		break;
	case NPCSNAKE_ACT_W17A_5:
		GM_RadarSetFlag(&(work->rctrl),RADAR_VISIBLE);

		SET_SCN_RUNAWAY();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		break;
	}
}

void DecideAction_W32A(Work *work)
{
	if(DecideAction_COMMON(work)) return;

	switch(NPCSNAKE_ACT_MODE(work)){
	case NPCSNAKE_ACT_W32A_WAIT:
		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)) break;

#if 0
		if(work->snake_status & SNAKE_STATUS_ORDER_TO_SNIPE){
			work->shoot_interval_count++;
			if(work->shoot_interval_count>SNIPE_TIME(work)){
				if(work->homing!=NULL){
					/* 狙撃 !!!! */
					DEBUG_PRINT_NPCSNAKE("Snipe !!!\n");

					NPCSnake_DecideTarget(work);

					NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_SHOOT);
					work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
					work->shoot_interval_count=0;
				}
				else{
					/* 物陰に隠れられてしまった */
					DEBUG_PRINT_NPCSNAKE("Not found target\n");

					// work->snake_status&=~SNAKE_STATUS_ORDER_TO_SNIPE;
					work->shoot_interval_count=0;
				}

				work->w32a_request_count++;
			}
		}
#else
		if((work->snake_status & SNAKE_STATUS_ORDER_TO_SNIPE) &&

		   /* 2001/9/18  K.Kano
			  クレイモアを狙撃するために判定 */

		   (work->homing!=NULL || work->claymore!=NULL)){

			work->shoot_interval_count++;
			if(work->shoot_interval_count>SNIPE_TIME(work)){

				if(NPCSnake_SearchHomingAfterShoot(work)){
					/* 狙撃 !!!! */
					DEBUG_PRINT_NPCSNAKE("Snipe !!!\n");

					NPCSnake_DecideTarget(work);

					NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_SHOOT);
					work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				}

				work->shoot_interval_count=0;
			}
		}
#endif
		else{
			work->shoot_interval_count=0;
			work->shoot_count=work->shoot_limit;
			work->snake_status&=~SNAKE_STATUS_LOCKON_ENEMY;
		}
		break;
	case NPCSNAKE_ACT_W32A_SHOOT:
		SET_SCN_SHOOT();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_AFTER_SHOOT);
			work->shoot_interval_count=0;
		}
		break;
	case NPCSNAKE_ACT_W32A_AFTER_SHOOT:
		SET_SCN_SHOOT();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->shoot_interval_count++;
		if(work->shoot_interval_count>AFTER_SNIPE_TIME(work)){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_WAIT);
			work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

#if 0
			work->shoot_count--;
			if(work->shoot_count<=0){
				/* 規定回数撃った */

				work->snake_status
					&=~(SNAKE_STATUS_ACTION_FINISHED|
						SNAKE_STATUS_ORDER_TO_SNIPE|
						SNAKE_STATUS_LOCKON_ENEMY);
				work->shoot_interval_count=0;

				DEBUG_PRINT_NPCSNAKE("Completed to Snipe\n");

				break;
			}
#endif

			if(NPCSnake_SearchHomingAfterShoot(work)){
				DEBUG_PRINT_NPCSNAKE("Continue to Snipe !!!\n");
			}

			work->shoot_interval_count=SNIPE_TIME(work);
		}
		break;


		/* ライデンへのやり返し */
	case NPCSNAKE_ACT_W32A_PRE_COUNTER:
		SET_SCN_COUNTERATTACK();
		// SET_SCN_POINT_TARGET();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->gcounter++;
		if(work->gcounter>INTERVAL_BEFORE_COUNTER){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_PRE_COUNTER2);

			work->gcounter=0;

#if 1
			if(PL_GetPlayerWeapon()!=WP_Mic){
				GM_JimakuSeSetMode(SD_V_SNADKN01,
								   (FVECTOR *)&(work->body.objs->world.m[3][0]),GM_SEMODE_BOMB);
			}
#endif
		}
		break;

	case NPCSNAKE_ACT_W32A_PRE_COUNTER2:
		SET_SCN_COUNTERATTACK();
		// SET_SCN_POINT_TARGET();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->gcounter++;
		if(work->gcounter>INTERVAL_BEFORE_COUNTER){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_COUNTER);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

#if 0
			if(PL_GetPlayerWeapon()!=WP_Mic){
				GM_JimakuSeSetMode(SD_V_SNADKN01,
								   (FVECTOR *)&(work->body.objs->world.m[3][0]),GM_SEMODE_BOMB);
			}
#endif
		}
		break;

	case NPCSNAKE_ACT_W32A_COUNTER:
		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_WAIT);
			work->snake_status&=~SNAKE_STATUS_LOCKON_ENEMY;

#if 0
			if(PL_GetPlayerWeapon()!=WP_Mic){
				GM_JimakuSeSetMode(SD_V_SNADKN01,
								   (FVECTOR *)&(work->body.objs->world.m[3][0]),GM_SEMODE_BOMB);
			}
#endif
		}
		break;

	case NPCSNAKE_ACT_W32A_SLEEP:
		SET_SCN_SLEEP();

		work->gcounter++;
		if(work->gcounter>W32A_SNAKE_SLEEP_COUNTER){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_AFTER_SLEEP);
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

			/* Ｚｚｚｚをクリア */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_KILL);
		}
		break;
	case NPCSNAKE_ACT_W32A_AFTER_SLEEP:
		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W32A_WAIT);

			/* 白 ? */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_QE);

			// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			if(work->w32a_end_sleeping_proc!=0){
				/* PROC呼び出し */
				GCL_ExecProc(work->w32a_end_sleeping_proc,NULL);
			}
		}
		break;
	}
}

void DecideAction_DUMMY(Work *work)
{
	if(DecideAction_COMMON(work)) return;
}

void DecideAction_W43A(Work *work)
{
	// DEBUG_PRINT_NPCSNAKE("snake status = 0x%08x\n",work->snake_status);

	if(DecideAction_COMMON(work)) return;

	switch(NPCSNAKE_ACT_MODE(work)){
	case NPCSNAKE_ACT_W43A:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_CALCADJUST_W43A;

		/* 雑誌の判定 */
		NPCSnake_CheckBook(work);

		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(work->snake_status2 & SNAKE_STATUS2_START_ROLLING){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ROLLING);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		}
#if 0
		else if(work->raiden_dis<CVM2N(0.5f) &&
				GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

			/* 気絶中のライデンを避ける */
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_AVOID_RAIDEN);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		}
#endif
		else if(work->snake_status & SNAKE_STATUS_LASERSIGHT){
			/* 白！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			NPCSnake_GetPlayerTargetPos(&(work->npc));

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);

#if 1
			NPCSNAKE_VOICE_RNDCALL(work,voice_angry,
								   sizeof(voice_angry)/sizeof(voice_angry[0]));
#endif

			work->w14a_count=0;
		}
#if 0
		else if(work->snake_status2 & SNAKE_STATUS2_CRASH_ZONE){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_AVOID_RAIDEN);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		}
#endif
		break;
	case NPCSNAKE_ACT_ROLLING:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

		SET_SCN_NORMAL();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W43A);
			work->snake_status2&=~SNAKE_STATUS2_START_ROLLING;
		}
		break;

#if 0
	case NPCSNAKE_ACT_AVOID_RAIDEN:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_CALCADJUST_W43A;

		/* 雑誌の判定 */
		NPCSnake_CheckBook(work);

		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(work->snake_status & SNAKE_STATUS_LASERSIGHT){
			/* 白！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			NPCSnake_GetPlayerTargetPos(&(work->npc));

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);

#if 1
			NPCSNAKE_VOICE_RNDCALL(work,voice_angry,
								   sizeof(voice_angry)/sizeof(voice_angry[0]));
#endif

			work->w14a_count=0;
		}
		else if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			if(work->snake_status2 & SNAKE_STATUS2_CRASH_ZONE){
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
			}
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W43A);
			}
		}
		break;
#endif

	case NPCSNAKE_ACT_W43A_MOVE_POINT:
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;
		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);
		SET_SCN_NORMAL();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
		break;

	case NPCSNAKE_ACT_POINT_TARGET:
		NPCSnake_GetPlayerTargetPos(&(work->npc));

		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_USE_USP;

		SET_SCN_POINT_TARGET();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		work->raiden_status&=~NPCSNAKE_RAIDEN_DAMAGED;

#if 0
		if(work->raiden_dis<CVM2N(0.5f) &&
		   GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

			/* 気絶中のライデンを避ける */
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_AVOID_RAIDEN);
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			fpu_CopyVector(&(work->neighbor_zone_using),&(work->neighbor_zone));
		}
		else
#endif
		if(work->raiden_dis>=CVM2N(3.0f)){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W43A);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

			ActSubMotionReset(work);
#if 0
			if(work->snake_status & SNAKE_STATUS_HAS_USP){
				ChangeNewPad(&(work->npc),PAD_STAND_WUSP);
				ActSubMotionReset(work);
			}
			else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
				ChangeNewPad(&(work->npc),PAD_STAND_WFMS);
				ActSubMotionReset(work);
			}
#endif

		}
		else if(!(work->snake_status & SNAKE_STATUS_LASERSIGHT)){
			work->w14a_count++;
			if(work->w14a_count>LASERSIGHT_OFF_TIME){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W43A);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

				ActSubMotionReset(work);

#if 0
				if(work->snake_status & SNAKE_STATUS_HAS_USP){
					ChangeNewPad(&(work->npc),PAD_STAND_WUSP);
					ActSubMotionReset(work);
				}
				else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
					ChangeNewPad(&(work->npc),PAD_STAND_WFMS);
					ActSubMotionReset(work);
				}
#endif

			}
		}
		break;
	case NPCSNAKE_ACT_WAIT_POINTING:
		NPCSnake_GetPlayerTargetPos(&(work->npc));

		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_USE_USP;

		SET_SCN_POINT_TARGET();
		work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

		if(work->raiden_status & NPCSNAKE_RAIDEN_DAMAGED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			break;
		}

		if(!GM_CheckPlayerStatus(PLAYER_GROUND)){
			if(work->raiden_dis>CVM2N(5.0f)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				work->shoot_count=work->shoot_limit;
				break;
			}
			if(work->raiden_dis<CVM2N(1.0f) &&
			   !GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}

		    if(HZX_OnlineHazardCheck(work->ctrl.hzx_id,&(work->ctrl.mov),work->homing_p,
									 HZX_CHK_ALL,HZX_SEG_NO_BULLET|HZX_SEG_RECOIL_TYPE,
									 HZX_FLOOR_NO_BULLET|HZX_FLOOR_RECOIL_TYPE|HZX_FLOOR_IK)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
		    }
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
			}

			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;
		}
		break;
	case NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_USE_USP;

		SET_SCN_DAMAGE();

		if(!(work->snake_status & SNAKE_STATUS_HAS_USP)) break;

#if 1
		NPCSNAKE_VOICE_RNDCALL(work,voice_counter_shoot,
							   sizeof(voice_counter_shoot)/sizeof(voice_counter_shoot[0]));
#endif

		if(work->raiden_dis<CVM2N(1.0f)){
#if 0
			if(work->shoot_limit>=20){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
			}
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN);
			}
#else
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN);
#endif
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			break;
		}

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		work->shoot_count=work->shoot_limit;

#ifdef DEBUG
		printf("Shoot Count = %d\n",work->shoot_count);
#endif

		break;
	case NPCSNAKE_ACT_SHOOT_RAIDEN:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_USE_USP;

		SET_SCN_COUNTERATTACK();

		if(!(work->snake_status & SNAKE_STATUS_ACTION_FINISHED)) break;

		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		NPCSnake_GetPlayerTargetPos(&(work->npc));

		work->shoot_count--;
		if(work->shoot_count<0){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;

			work->shoot_limit+=2;
			if(work->shoot_limit>=20){
				// NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN_INTERVAL);
				work->shoot_limit=20;
			}
			else if(work->shoot_limit>=10){
				work->shoot_limit=20;
			}
			else if(work->shoot_limit>=7){
				work->shoot_limit=10;
			}
			work->shoot_count=work->shoot_limit;
		}
		break;
	case NPCSNAKE_ACT_SHOOT_RAIDEN_INTERVAL:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_USE_USP;

		SET_SCN_COUNTERATTACK();

		NPCSnake_GetPlayerTargetPos(&(work->npc));

		if(work->raiden_status & NPCSNAKE_RAIDEN_DAMAGED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			break;
		}

		if(work->raiden_dis>=CVM2N(5.0f)){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;

#if 0
			if(work->snake_status & SNAKE_STATUS_HAS_USP){
				ChangeNewPad(&(work->npc),PAD_STAND_WUSP);
				ActSubMotionReset(work);
			}
			else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
				ChangeNewPad(&(work->npc),PAD_STAND_WFMS);
				ActSubMotionReset(work);
			}
#endif

			break;
		}

		work->w14a_count++;
		if(work->w14a_count>=BETWEEN_STAGES_SHOOT_INTERVAL &&
		   (work->ctrl.map & GM_CurrentChanlMap[0])){

			if(work->raiden_dis<CVM2N(1.0f) &&
			   !GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}
		    if(HZX_OnlineHazardCheck(work->ctrl.hzx_id,&(work->ctrl.mov),work->homing_p,
									 HZX_CHK_ALL,HZX_SEG_NO_BULLET|HZX_SEG_RECOIL_TYPE,
									 HZX_FLOOR_NO_BULLET|HZX_FLOOR_RECOIL_TYPE|HZX_FLOOR_IK)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
		    }
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
			}

			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;
		}
		break;

	case NPCSNAKE_ACT_ANGRY2:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_USE_USP;

		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		}
		break;

	case NPCSNAKE_ACT_PUNCH_RAIDEN2:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status|=SNAKE_STATUS_USE_USP;

		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			if(work->raiden_status & NPCSNAKE_RAIDEN_DAMAGED){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_POINT_TARGET);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}
			if(GM_CheckPlayerStatus(PLAYER_GROUND)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_WAIT_POINTING);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}
			if(work->raiden_dis>CVM2N(5.0f)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->w14a_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				work->shoot_count=work->shoot_limit;
				break;
			}
			if(work->raiden_dis<CVM2N(1.0f) &&
			   !(GM_CheckPlayerStatus(PLAYER_SQUAT|PLAYER_GROUND))){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN2);
				NPCSNAKE_SHOOT_RESET(work);
				NPCSNAKE_HIDE_RESET(work);
				work->movepos_count=0;
				work->hide_count=0;
				work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
				break;
			}

		    if(HZX_OnlineHazardCheck(work->ctrl.hzx_id,&(work->ctrl.mov),work->homing_p,
									 HZX_CHK_ALL,HZX_SEG_NO_BULLET|HZX_SEG_RECOIL_TYPE,
									 HZX_FLOOR_NO_BULLET|HZX_FLOOR_RECOIL_TYPE|HZX_FLOOR_IK)){

				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY2);
		    }
			else{
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_SHOOT_RAIDEN);
			}

			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->w14a_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
			work->shoot_count=work->shoot_limit;
		}
		break;


	case NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		SET_SCN_COUNTERATTACK();

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_ANGRY);
		NPCSNAKE_SHOOT_RESET(work);
		NPCSNAKE_HIDE_RESET(work);
		work->movepos_count=0;
		work->hide_count=0;
		work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		break;
	case NPCSNAKE_ACT_ANGRY:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		SET_SCN_COUNTERATTACK();

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){

#if 1
			NPCSNAKE_VOICE_RNDCALL(work,voice_counter_shoot,
								   sizeof(voice_counter_shoot)/sizeof(voice_counter_shoot[0]));
#endif

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_PUNCH_RAIDEN);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;
		}
		break;
	case NPCSNAKE_ACT_PUNCH_RAIDEN:
		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		// work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		SET_SCN_COUNTERATTACK();

		printf("W43A PUNCH RAIDEN = 0x%08x\n",work->snake_status);

		if(work->snake_status & SNAKE_STATUS_ACTION_FINISHED){
			printf("W43A PUNCH RAIDEN Finished\n");

			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W43A);
			NPCSNAKE_SHOOT_RESET(work);
			NPCSNAKE_HIDE_RESET(work);
			work->movepos_count=0;
			work->hide_count=0;
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

			ActSubMotionReset(work);
		}
		break;
	}
}
