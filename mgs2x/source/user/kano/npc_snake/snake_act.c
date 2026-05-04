//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_act.c
		NPCスネークACTメイン

	2001/02/13 K.Kano
	$Id: snake_act.c,v 1.1.1.3 2002/11/19 11:43:21 Yoshizawa1 Exp $
 */


#include "npc_snake.h"


#define ChangeHazardSphere(_ctrl,_r,_s) \
do{ \
    (_ctrl)->r_sphere=(_r); \
    (_ctrl)->s_sphere=(_s); \
}while(0)

#define R_SHPERE_NORMAL		450
#define S_SHPERE_NORMAL		500
#define R_SHPERE_HIDE		200
#define S_SHPERE_HIDE		220
#define R_SHPERE_W14A		10
#define S_SHPERE_W14A		20

#define SWITCH_DIS			700.0f


int ActPreMove(Work *work,NPCWORK *npc)
{
	/* 前処理
	   返り値が1の場合は前処理中。
	   0の場合は前処理は終了済み。*/

	switch(npc->action.set_pad){
	case PAD_SITDOWN:
	case PAD_SITTING:
		ChangeNewPad(npc,PAD_STANDUP);
		return 1;

	case PAD_SITDOWN_WUSP:
	case PAD_SITTING_WUSP:
		ChangeNewPad(npc,PAD_STANDUP_WUSP);
		return 1;

	case PAD_SITDOWN_WFMS:
	case PAD_SITTING_WFMS:
		ChangeNewPad(npc,PAD_STANDUP_WFMS);
		return 1;

	case PAD_START_BEHIND_AND_SITTING:
	case PAD_BEHIND_AND_SITTING:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_BEHIND_AND_SITTING);
		return 1;

	case PAD_START_BEHIND_AND_SITTING_WUSP:
	case PAD_BEHIND_AND_SITTING_WUSP:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_BEHIND_AND_SITTING_WUSP);
		return 1;

	case PAD_START_BEHIND_AND_SITTING_WFMS:
	case PAD_BEHIND_AND_SITTING_WFMS:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_BEHIND_AND_SITTING_WFMS);
		return 1;


	case PAD_START_LOOK_L_BEHIND:
	case PAD_LOOK_L_BEHIND:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_L_BEHIND);
		return 1;

	case PAD_START_LOOK_R_BEHIND:
	case PAD_LOOK_R_BEHIND:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_R_BEHIND);
		return 1;

	case PAD_START_LOOK_L_BEHIND_WUSP:
	case PAD_LOOK_L_BEHIND_WUSP:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_L_BEHIND_WUSP);
		return 1;

	case PAD_START_LOOK_R_BEHIND_WUSP:
	case PAD_LOOK_R_BEHIND_WUSP:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_R_BEHIND_WUSP);
		return 1;

	case PAD_START_LOOK_L_BEHIND_WFMS:
	case PAD_LOOK_L_BEHIND_WFMS:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_L_BEHIND_WFMS);
		return 1;

	case PAD_START_LOOK_R_BEHIND_WFMS:
	case PAD_LOOK_R_BEHIND_WFMS:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_R_BEHIND_WFMS);
		return 1;


	case PAD_START_LOOK_L_BEHIND_AND_SITTING:
	case PAD_LOOK_L_BEHIND_AND_SITTING:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_L_BEHIND_AND_SITTING);
		return 1;

	case PAD_START_LOOK_R_BEHIND_AND_SITTING:
	case PAD_LOOK_R_BEHIND_AND_SITTING:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_R_BEHIND_AND_SITTING);
		return 1;

	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_LOOK_L_BEHIND_AND_SITTING_WUSP:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_L_BEHIND_AND_SITTING_WUSP);
		return 1;

	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WUSP:
	case PAD_LOOK_R_BEHIND_AND_SITTING_WUSP:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_R_BEHIND_AND_SITTING_WUSP);
		return 1;

	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_LOOK_L_BEHIND_AND_SITTING_WFMS:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_L_BEHIND_AND_SITTING_WFMS);
		return 1;

	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WFMS:
	case PAD_LOOK_R_BEHIND_AND_SITTING_WFMS:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_LOOK_R_BEHIND_AND_SITTING_WFMS);
		return 1;


	case PAD_START_BEHIND:
	case PAD_BEHIND:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_BEHIND);
		return 1;

	case PAD_START_BEHIND_WUSP:
	case PAD_BEHIND_WUSP:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_BEHIND_WUSP);
		return 1;

	case PAD_START_BEHIND_WFMS:
	case PAD_BEHIND_WFMS:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		ChangeNewPad(npc,PAD_END_BEHIND_WFMS);
		return 1;

	case PAD_PUNCH:
		ChangeNewPad(npc,PAD_STAND);
		break;
	case PAD_PUNCH_WUSP:
		ChangeNewPad(npc,PAD_STAND_WUSP);
		break;
	case PAD_PUNCH_WFMS:
		ChangeNewPad(npc,PAD_STAND_WFMS);
		break;
#if 0
	case PAD_BRINGING_USP:
		ChangeNewPad(npc,PAD_STAND_WUSP);
		ActSubMotionReset(work);
		return 1;
	case PAD_BRINGING_FMS:
		ChangeNewPad(npc,PAD_STAND_WFMS);
		ActSubMotionReset(work);
		return 1;
	case PAD_BRINGING_USP_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WUSP);
		return 1;
	case PAD_BRINGING_FMS_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WFMS);
		return 1;
#endif

	case PAD_STANDUP:
	case PAD_STANDUP_WUSP:
	case PAD_STANDUP_WFMS:
		return 1;

	case PAD_END_BEHIND:
	case PAD_END_BEHIND_WUSP:
	case PAD_END_BEHIND_WFMS:

	case PAD_END_LOOK_L_BEHIND:
	case PAD_END_LOOK_R_BEHIND:
	case PAD_END_LOOK_L_BEHIND_WUSP:
	case PAD_END_LOOK_R_BEHIND_WUSP:
	case PAD_END_LOOK_L_BEHIND_WFMS:
	case PAD_END_LOOK_R_BEHIND_WFMS:

	case PAD_END_BEHIND_AND_SITTING:
	case PAD_END_BEHIND_AND_SITTING_WUSP:
	case PAD_END_BEHIND_AND_SITTING_WFMS:

	case PAD_END_LOOK_L_BEHIND_AND_SITTING:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING_WUSP:
	case PAD_END_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_END_LOOK_R_BEHIND_AND_SITTING_WFMS:
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);
		return 1;
	}

	return 0;
}

int ActPreMoveEx(Work *work,NPCWORK *npc)
{
	if(ActPreMove(work,npc)) return 1;

	switch(npc->action.set_pad){
	case PAD_BRINGING_USP:
		ChangeNewPad(npc,PAD_STAND_WUSP);
		ActSubMotionReset(work);
		return 1;
	case PAD_BRINGING_FMS:
		ChangeNewPad(npc,PAD_STAND_WFMS);
		ActSubMotionReset(work);
		return 1;
	case PAD_BRINGING_USP_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WUSP);
		return 1;
	case PAD_BRINGING_FMS_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WFMS);
		return 1;
	}
	return 0;
}

int NPCSnake_NaviNear(Work *work,NAVIGATE *navi,NAVITARGET *nvtrg,int dis)
{
	if(GM_SaveArea==GV_StrCode("w45a")) return GM_NaviNearExtend(navi,nvtrg,dis);
	else return GM_NaviNear(navi,nvtrg,dis);
}


static int ActStay(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMoveEx(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
	npc->action.dir=-1;

	return 1;
}

/* 指定位置に移動 */
static int ActMovePos(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	if(work->support_pos==NULL){
		npc->action.dir=-1;
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		return 1;
	}

	/* nvtrg に 目標のposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,work->support_pos,0);

	/* nvtrgに向ってゾーン移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,700)){
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}

/* 目的ゾーンを元に移動 */
static int ActMoveZone(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	/* nvtrg に 目標のposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,&(work->neighbor_zone_using),0);

	/* nvtrgに向ってゾーン移動 */
#if 1
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,700)){
#else
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,(int)CVM2N(1.0f))){
#endif
		DEBUG_PRINT_NPCSNAKE("MOVE ZONE FINISH\n");

		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

		work->i_infinity++;
		if(work->i_infinity>=INFINITY_MOVE_SIZE) work->i_infinity=0;

		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}

/* ライデンを目標に移動 */
static int ActFollowRaiden(Work *work)
{
	NPCWORK	*npc ;
	FVECTOR pos;

	npc = &work->npc ;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	fpu_CopyVector(&pos,&GM_PlayerPosition);
#if 0
	pos.vx=work->ctrl.mov.vx;
	pos.vz+=CVM2N(1.5f); /* ライデンから1.5m後方へ */
#endif

	/* nvtrg に 目標のCONTROLのposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,&pos,0);

	/* ライデンに向かって移動 */
#if 0
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,500)){
#else
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,(int)CVM2N(0.75f))){
#endif
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}

static int ActHide(Work *work)
{
	NPCWORK	*npc;
	FVECTOR *pos;
	float dis;
	float x,z;

	npc=&(work->npc);

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	pos=work->hide_pos;

	x=pos->vx-GM_PlayerPosition.vx;
	z=pos->vz-GM_PlayerPosition.vz;
	dis=x*x+z*z;

	if(dis<CVM2N(1.0f)){
		/* ライデンがすでに隠れるポイント近辺にいた場合、
		   その場所に隠れることをあきらめ、別の場所を探す。*/

		NPCSNAKE_HIDE_RESET(work);
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		return 1;
	}


	/* nvtrg に 目標のposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,pos,0);

	x=pos->vx-work->ctrl.mov.vx;
	z=pos->vz-work->ctrl.mov.vz;
	dis=x*x+z*z;

	// printf("%f\n",fpu_Sqrt(dis));

	if(dis<SWITCH_DIS*SWITCH_DIS){
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);

		if(!(work->snake_status & SNAKE_STATUS_WALKING_MOTION)){
			if(work->snake_status & SNAKE_STATUS_HAS_USP){
				ChangeNewPad(npc,PAD_WALK_WUSP);
			}
			else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
				ChangeNewPad(npc,PAD_WALK_WFMS);
			}
			else{
				ChangeNewPad(npc,PAD_WALK);
			}
		}
	}
	else{
		if(work->snake_status & SNAKE_STATUS_WALKING_MOTION){
			if(work->snake_status & SNAKE_STATUS_HAS_USP){
				ChangeNewPad(npc,PAD_RUN_WUSP);
			}
			else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
				ChangeNewPad(npc,PAD_RUN_WFMS);
			}
			else{
				ChangeNewPad(npc,PAD_RUN);
			}
		}
	}

	/* nvtrgに向ってゾーン移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,400)){
		if(CHECK_SHOOT_FINISH_FLAG(work)){

			/* MOVEPOS+MOVE_TO_HIDEを許容したので、隠れるポイントまで
			   移動したことが確定した場合にSTAYを上書きセットしておく。*/
			NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);
			work->movepos_count=0;

			work->behind_pos=work->hide_pos;
			work->behind_dir=work->hide_dir;
			work->behind_posflag=work->hide_posflag;

			work->snake_status&=~SNAKE_STATUS_DAMAGED_FOR_BEHIND;

			if((work->behind_posflag & POS_BE_CAPTURED) &&
			   !(work->snake_status & SNAKE_STATUS2_DISABLE_CAPTURED)){

				NPCSNAKE_SUPPORT_POINT *point;
				int num=(work->behind_posflag & POS_CAPTURE_NUMBER_MASK);
				int i;

				/* 羽交い締めは一回だけ */
				work->behind_posflag&=~POS_BE_CAPTURED;
				work->hide_posflag&=~POS_BE_CAPTURED;

#if 0
				point=work->support_points+work->hide_pos_index;
				// point->posflag&=~POS_BE_CAPTURED;
#else
				/* 同じ場所番号の羽交い締めフラグを消す */
				point=work->support_points;
				i=work->n_support_points;

				while(i>0){
					if((point->posflag & POS_CAPTURE_NUMBER_MASK)==num){
						point->posflag&=~POS_BE_CAPTURED;
					}
					point++;
					i--;
				}
#endif

				/* 羽交い締め待ち !!! */

				work->snake_status2|=SNAKE_STATUS2_BE_CAPTURED_MODE;
				work->capture_count=0;

				if(work->capture_proc!=0){
					GCL_ARGS args;
					int argv[1];

					args.argc=1;
					args.argv=argv;
					argv[0]=(num>>16);

					/* PROC呼び出し */
					GCL_ExecProc(work->capture_proc,&args);
				}
			}

			if(work->behind_posflag & POS_FLAG_SITTING_HIDEPOS){
				NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_SITDOWN_TO_HIDE);
			}
			else{
				NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_HIDING);
			}
			work->hide_count=0;
		}
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}

/* 目的の場所まで歩く */
static int ActWalkPoint(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	/* nvtrg に 目標のposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,&(work->neighbor_zone_using),0);

#if 1
	if(!(work->snake_status & SNAKE_STATUS_WALKING_MOTION)){
		if(work->snake_status & SNAKE_STATUS_HAS_USP){
			ChangeNewPad(npc,PAD_WALK_WUSP);
		}
		else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
			ChangeNewPad(npc,PAD_WALK_WFMS);
		}
		else{
			ChangeNewPad(npc,PAD_WALK);
		}
	}
#endif

	/* nvtrgに向ってゾーン移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,700)){
		DEBUG_PRINT_NPCSNAKE("MOVE ZONE FINISH\n");
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}

/* ローリング攻撃 */
static int DecideRollingDir(Work *work)
{
	FVECTOR *snake_dir=(FVECTOR *)&(work->body.objs->world.m[2][0]);
	HZX_ZONE_ADD zone=work->ctrl.addr;
	HZX_ZON *link;
	int tdir=work->ctrl.turn.vy;
	float vcos_max=-FLT_MAX;
	int no;
	int i;

	link=HZX_GetZoneFromAdd(zone);
	no=HZX_ZoneMapNo(zone);

	for(i=0;i<5;i++){
		HZX_ZON *zon;
		FVECTOR v;
		float vcos,len;

		if(link->nears[i]==0xff) continue;
		if(link->near_flag[i] & HZX_ROOT_JUMP) continue;

		zon=HZX_GetZoneNo(no,link->nears[i]);

		v.vx=zon->x-work->ctrl.mov.vx;
		v.vy=0.0f;
		v.vz=zon->z-work->ctrl.mov.vz;

		vu0_Ldv0(&v);
		vu0_Ldv1(snake_dir);

		vcos=vu0_InnerProductv0v1();
		len=vu0_VectorLength2v0();
		vcos=fpu_Rsqrt(len,vcos);

		if(vcos>vcos_max){
			vcos_max=vcos;
			tdir=GV_VecDir2(&v);
		}
	}
	return tdir;
}

static int ActRolling(Work *work)
{
	NPCWORK	*npc ;
	int dir;

	npc = &work->npc ;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	switch(npc->action.set_pad){
	case PAD_TUMBLE_START:
	case PAD_TUMBLE_START_WUSP:
	case PAD_TUMBLE_START_WFMS:
	case PAD_TUMBLE_END:
	case PAD_TUMBLE_END_WUSP:
	case PAD_TUMBLE_END_WFMS:
		break;
	default:
		dir=DecideRollingDir(work);

		if(work->snake_status & SNAKE_STATUS_HAS_USP){
			ChangeNewPad(npc,PAD_TUMBLE_START_WUSP);
			npc->action.dir=dir;
		}
		else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
			ChangeNewPad(npc,PAD_TUMBLE_START_WFMS);
			npc->action.dir=dir;
		}
		else{
			ChangeNewPad(npc,PAD_TUMBLE_START);
			npc->action.dir=dir;
		}
		break;
	}

	return 0;
}


/* -------------------------------------------------------------------- */
/* w14a専用                                                             */
/* -------------------------------------------------------------------- */


static int ActMoveW14A_0(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	// fpu_CopyVector(&(work->ctrl.mov),&(work->pos));

	if(!(work->snake_status & SNAKE_STATUS_SYS_DAMAGED)){
		if(work->snake_status2 & SNAKE_STATUS2_START_QUESTION_MARK){
			/* 白 ? */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_WHT_QE);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			work->snake_status2&=~SNAKE_STATUS2_START_QUESTION_MARK;
			work->snake_status2|=SNAKE_STATUS2_ALREADY_DISP_QUESTION_MARK;
		}
	}

	/* 座り続ける */
	if(npc->action.set_pad!=PAD_SITTING_AND_SLEEP_WM4A1){
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_W14A,S_SHPERE_W14A);

		DEBUG_PRINT_NPCSNAKE("Sleeping\n");
		ChangeNewPad(npc,PAD_SITTING_AND_SLEEP_WM4A1);
	}

	return 1;
}

static int ActMoveW14A_1(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	/* M4を構える */
	if(npc->action.set_pad!=PAD_SITTING_AND_BRINGING_WM4A1){
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_W14A,S_SHPERE_W14A);
		ChangeNewPad(npc,PAD_SITTING_AND_BRINGING_WM4A1);
	}

	return 1;
}

static int ActMoveW14A_2(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if(!(work->snake_status & SNAKE_STATUS_SYS_DAMAGED)){
		if(work->snake_status2 & SNAKE_STATUS2_START_QUATATION_MARK){
			/* 赤！ */
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_KILL);
			NPC_CallHeadMark(&(work->npc),HMK2_TYPE_RED_AT);

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

			work->snake_status2&=~SNAKE_STATUS2_START_QUATATION_MARK;
		}
	}

	/* M4を撃つ */
	if(npc->action.set_pad==PAD_SITTING_AND_BRINGING_WM4A1 ||
	   npc->action.set_pad==PAD_SITTING_AND_START_SHOOT_WM4A1 ||
	   npc->action.set_pad==PAD_SITTING_AND_SHOOT_WM4A1){

		ChangeHazardSphere(&(work->ctrl),R_SHPERE_W14A,S_SHPERE_W14A);
		ChangeNewPad(npc,PAD_SITTING_AND_SHOOT_WM4A1);
	}
	else{
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_W14A,S_SHPERE_W14A);
		ChangeNewPad(npc,PAD_SITTING_AND_START_SHOOT_WM4A1);
	}

	return 1;
}

static int ActMoveW14A_3(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	/* 座り続ける */
	if(npc->action.set_pad!=PAD_SITTING_AND_M9SLEEP){
		ChangeHazardSphere(&(work->ctrl),R_SHPERE_W14A,S_SHPERE_W14A);

		DEBUG_PRINT_NPCSNAKE("Sleeping ZZZZ\n");
		ChangeNewPad(npc,PAD_SITTING_AND_M9SLEEP);
	}

	return 1;
}


/* -------------------------------------------------------------------- */
/* w17a専用                                                             */
/* -------------------------------------------------------------------- */


static int ActMoveW17A(Work *work,int flag)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	/* ヘッドマーク用マトリクスの作成 */
	DG_GetPos(&(work->headmark_mat));
	work->headmark_mat.m[3][1]+=HEADMARK_HEIGHT_OFFSET;

	switch(flag){
	case 0:
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		if(npc->action.set_pad!=PAD_BOX_STOP){
			ChangeNewPad(npc,PAD_BOX_STOP);
		}
		npc->action.dir = -1;
		return 1;
	case 1:
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		if(npc->action.set_pad!=PAD_BOX_IDLE){
			ChangeNewPad(npc,PAD_BOX_IDLE);
		}
		npc->action.dir = -1;
		return 1;
	}

	/* nvtrg に 目標のposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,&(work->target_pos),0);

	/* nvtrgに向ってゾーン移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,500)){
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

		if(npc->action.set_pad!=PAD_BOX_IDLE){
			ChangeNewPad(npc,PAD_BOX_IDLE);
		}

		return 1;
	}

	switch(flag){
	case 2:
		if(npc->action.set_pad!=PAD_BOX_WALK){
			ChangeNewPad(npc,PAD_BOX_WALK);
		}
		break;
	case 3:
		if(npc->action.set_pad!=PAD_BOX_RUN){
			ChangeNewPad(npc,PAD_BOX_RUN);
		}
		break;
	case 4:
		if(npc->action.set_pad!=PAD_BOX_DASH){
			ChangeNewPad(npc,PAD_BOX_DASH);
		}
		break;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}


/* -------------------------------------------------------------------- */
/* w43a専用                                                             */
/* -------------------------------------------------------------------- */

/* 目標地点に移動 */
static int ActW43AMovePoint(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	/* nvtrg に 目標のCONTROLのposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,&(work->target_pos),0);

	/* ライデンに向かって移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,500)){
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}


/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */


static void ActPunch(Work *work);

static int ActMoveSelecter(Work *work)
{
	int ans=0;

	switch(NPCSNAKE_ACT_MODE(work)){
	case NPCSNAKE_ACT_MOVEPOS:
		if(NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_NO_HIDING) ans=ActMovePos(work);
		else ans=ActHide(work);
		break;
	case NPCSNAKE_ACT_MOVEZONE:
	case NPCSNAKE_ACT_MOVEZONE2:
		ans=ActMoveZone(work);
		break;
	case NPCSNAKE_ACT_FLWRAIDEN:
		ans=ActFollowRaiden(work);
		break;
	case NPCSNAKE_ACT_MOVEAROUNDRAIDEN:
		ans=ActMoveAroundRaiden(work);
		break;
	case NPCSNAKE_ACT_MOVEAROUNDRAIDEN2:
		ans=ActMoveAroundRaiden2(work);
		break;

	case NPCSNAKE_ACT_FLWRAIDEN_ONLY:
		ActMoveFlwRaidenOnly(work);
		return 0;

	case NPCSNAKE_ACT_POINT_TARGET:
	case NPCSNAKE_ACT_SHOOT_RAIDEN_INTERVAL:
	case NPCSNAKE_ACT_WAIT_POINTING:
		ActPointTarget(work);
		return 0;

	case NPCSNAKE_ACT_SHOOT_RAIDEN_DAMAGE_WAIT:
	case NPCSNAKE_ACT_ANGRY_DAMAGE_WAIT:
	case NPCSNAKE_ACT_W43A:
		ActStay(work);
		return 0;

	case NPCSNAKE_ACT_SHOOT_RAIDEN:
		ActShootRaiden(work);
		return 0;

	case NPCSNAKE_ACT_ANGRY:
	case NPCSNAKE_ACT_ANGRY2:
		ActAngry(work);
		return 0;

	case NPCSNAKE_ACT_PUNCH_RAIDEN:
	case NPCSNAKE_ACT_PUNCH_RAIDEN2:
		ActPunch(work);
		return 0;

	case NPCSNAKE_ACT_W14A_0:
		/* 座り込んでいる */
		ActMoveW14A_0(work);
		return 0;
	case NPCSNAKE_ACT_W14A_1:
	case NPCSNAKE_ACT_W14A_2_INTERVAL:
		/* ライデンに向かって構える */
		ActMoveW14A_1(work);
		return 0;
	case NPCSNAKE_ACT_W14A_2:
	case NPCSNAKE_ACT_W14A_2_1:
		/* ライデンに向かって撃つ */
		ActMoveW14A_2(work);
		return 0;
	case NPCSNAKE_ACT_W14A_3:
	case NPCSNAKE_ACT_W14A_4:
	case NPCSNAKE_ACT_W14A_5:
	case NPCSNAKE_ACT_W14A_6:
		/* 寝る */
		ActMoveW14A_3(work);
		return 0;

	case NPCSNAKE_ACT_W17A_0:
	case NPCSNAKE_ACT_W17A_1:
		/* 停止 */
		ans=ActMoveW17A(work,0);
		break;
	case NPCSNAKE_ACT_W17A_2:
		/* 歩き */
		ans=ActMoveW17A(work,2);
		break;
	case NPCSNAKE_ACT_W17A_3:
		/* 走り */
		ans=ActMoveW17A(work,3);
		break;
	case NPCSNAKE_ACT_W17A_4:
		/* 座っている */
		ans=ActMoveW17A(work,1);
		break;
	case NPCSNAKE_ACT_W17A_5:
		/* ダッシュ逃げ */
		ans=ActMoveW17A(work,4);
		break;

	case NPCSNAKE_ACT_DUMMY:
		ActWaitW32A0(work);
		break;
	case NPCSNAKE_ACT_W32A_WAIT:
		ActWaitW32A(work);
		break;
	case NPCSNAKE_ACT_W32A_SHOOT:
		ActShootW32A(work);
		break;
	case NPCSNAKE_ACT_W32A_AFTER_SHOOT:
		ActWaitW32A(work);
		break;
	case NPCSNAKE_ACT_W32A_PRE_COUNTER:
	case NPCSNAKE_ACT_W32A_PRE_COUNTER2:
		ActPreCounterW32A(work);
		break;
	case NPCSNAKE_ACT_W32A_COUNTER:
		ActCounterW32A(work);
		break;
	case NPCSNAKE_ACT_W32A_SLEEP:
		ActWaitW32A(work);
		break;
	case NPCSNAKE_ACT_W32A_AFTER_SLEEP:
		ActWaitW32A(work);
		break;

	case NPCSNAKE_ACT_W43A_MOVE_POINT:
		ActW43AMovePoint(work);
		break;

	case NPCSNAKE_ACT_AVOID_RAIDEN:
		ActWalkPoint(work);
		break;
	case NPCSNAKE_ACT_ROLLING:
		ActRolling(work);
		break;
	}

	return ans;
}

static void ActHiding(Work *work)
{
	NPCWORK	*npc;

	npc=&(work->npc);

	work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;

	ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	switch(work->npc.action.set_pad){
	case PAD_STAND:
		ChangeNewPad(&(work->npc),PAD_START_BEHIND);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_STAND_WUSP:
	case PAD_SHOOT_USP:
		ChangeNewPad(&(work->npc),PAD_START_BEHIND_WUSP);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_STAND_WFMS:
	case PAD_SHOOT_FMS:
		ChangeNewPad(&(work->npc),PAD_START_BEHIND_WFMS);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_SITTING:
		ChangeNewPad(&(work->npc),PAD_START_BEHIND_AND_SITTING);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_SITTING_WUSP:
	case PAD_SITTING_AND_SHOOT_USP:
		ChangeNewPad(&(work->npc),PAD_START_BEHIND_AND_SITTING_WUSP);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_SITTING_WFMS:
	case PAD_SITTING_AND_SHOOT_FMS:
		ChangeNewPad(&(work->npc),PAD_START_BEHIND_AND_SITTING_WFMS);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	}
}

static void ActHidingSitdown(Work *work)
{
	ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	switch(work->npc.action.set_pad){
	case PAD_SITTING:
	case PAD_SITTING_WUSP:
	case PAD_SITTING_WFMS:
		DEBUG_PRINT_NPCSNAKE("Hiding & Sitting\n");
		NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_HIDING);
		break;
	case PAD_SITDOWN:
	case PAD_SITDOWN_WUSP:
	case PAD_SITDOWN_WFMS:
		break;
	default:
		switch(work->snake_status & (SNAKE_STATUS_HAS_USP|SNAKE_STATUS_HAS_FMS)){
		case SNAKE_STATUS_HAS_USP:
			DEBUG_PRINT_NPCSNAKE("Sitdown\n");
			ChangeNewPad(&(work->npc),PAD_SITDOWN_WUSP);
			break;
		case SNAKE_STATUS_HAS_FMS:
			DEBUG_PRINT_NPCSNAKE("Sitdown\n");
			ChangeNewPad(&(work->npc),PAD_SITDOWN_WFMS);
			break;
		default:
			DEBUG_PRINT_NPCSNAKE("Sitdown\n");
			ChangeNewPad(&(work->npc),PAD_SITDOWN);
			break;
		}
		break;
	}
}

static void ActPeeping(Work *work)
{
	ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	switch(work->npc.action.set_pad){
	case PAD_START_LOOK_L_BEHIND:
	case PAD_START_LOOK_R_BEHIND:
	case PAD_START_LOOK_L_BEHIND_AND_SITTING:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING:
	case PAD_START_LOOK_L_BEHIND_WUSP:
	case PAD_START_LOOK_R_BEHIND_WUSP:
	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WUSP:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WUSP:
	case PAD_START_LOOK_L_BEHIND_WFMS:
	case PAD_START_LOOK_R_BEHIND_WFMS:
	case PAD_START_LOOK_L_BEHIND_AND_SITTING_WFMS:
	case PAD_START_LOOK_R_BEHIND_AND_SITTING_WFMS:

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
		break;

	default:
		switch(work->snake_status &
			   (SNAKE_STATUS_HAS_USP|SNAKE_STATUS_HAS_FMS|SNAKE_STATUS_SITTING_MOTION)){
		case SNAKE_STATUS_HAS_USP:
			DEBUG_PRINT_NPCSNAKE("Start Peeping\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_LOOK_R_BEHIND_WUSP);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_LOOK_L_BEHIND_WUSP);
			}

			/* モーションの方向 */
			work->npc.action.dir=work->behind_dir;
			break;
		case SNAKE_STATUS_HAS_USP|SNAKE_STATUS_SITTING_MOTION:
			DEBUG_PRINT_NPCSNAKE("Start Peeping\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_LOOK_R_BEHIND_AND_SITTING_WUSP);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_LOOK_L_BEHIND_AND_SITTING_WUSP);
			}

			/* モーションの方向 */
			work->npc.action.dir=work->behind_dir;
			break;
		case SNAKE_STATUS_HAS_FMS:
			DEBUG_PRINT_NPCSNAKE("Start Peeping\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_LOOK_R_BEHIND_WFMS);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_LOOK_L_BEHIND_WFMS);
			}

			/* モーションの方向 */
			work->npc.action.dir=work->behind_dir;
			break;
		case SNAKE_STATUS_HAS_FMS|SNAKE_STATUS_SITTING_MOTION:
			DEBUG_PRINT_NPCSNAKE("Start Peeping\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_LOOK_R_BEHIND_AND_SITTING_WFMS);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_LOOK_L_BEHIND_AND_SITTING_WFMS);
			}

			/* モーションの方向 */
			work->npc.action.dir=work->behind_dir;
			break;
		case SNAKE_STATUS_SITTING_MOTION:
			DEBUG_PRINT_NPCSNAKE("Start Peeping\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_LOOK_R_BEHIND_AND_SITTING);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_LOOK_L_BEHIND_AND_SITTING);
			}

			/* モーションの方向 */
			work->npc.action.dir=work->behind_dir;
			break;
		default:
			DEBUG_PRINT_NPCSNAKE("Start Peeping\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_LOOK_R_BEHIND);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_LOOK_L_BEHIND);
			}

			/* モーションの方向 */
			work->npc.action.dir=work->behind_dir;
			break;
		}
		break;
	}
}

static void ActAttack(Work *work)
{
	ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);

	SET_SCN_SHOOT();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	switch(work->npc.action.set_pad){
	case PAD_START_SHOOT_USP_L_FROM_BEHIND:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND:
	case PAD_START_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_FMS_L_FROM_BEHIND:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND:
	case PAD_START_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
	case PAD_START_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:
		break;

	case PAD_SHOOT_USP:
		{
			int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

			if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_USP);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_SHOOT_USP);
			}
		}
		work->npc.action.dir=work->homing_dir;
		break;
	case PAD_SHOOT_FMS:
		{
			int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

			if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_FMS);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_SHOOT_FMS);
			}
		}
		work->npc.action.dir=work->homing_dir;
		break;
	case PAD_SITTING_AND_SHOOT_USP:
		{
			int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

			if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
				ChangeNewPad(&(work->npc),PAD_SITTING_AND_START_SHOOT_USP);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_SITTING_AND_SHOOT_USP);
			}
		}
		work->npc.action.dir=work->homing_dir;
		break;
	case PAD_SITTING_AND_SHOOT_FMS:
		{
			int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

			if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
				ChangeNewPad(&(work->npc),PAD_SITTING_AND_START_SHOOT_FMS);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_SITTING_AND_SHOOT_FMS);
			}
		}
		work->npc.action.dir=work->homing_dir;
		break;

	default:
		switch(work->snake_status &
			   (SNAKE_STATUS_HAS_USP|SNAKE_STATUS_HAS_FMS|SNAKE_STATUS_SITTING_MOTION)){
		case SNAKE_STATUS_HAS_USP:
		case 0:   /* この条件は入れるべき？？？ */
			DEBUG_PRINT_NPCSNAKE("Start Shoot from hiding\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_USP_R_FROM_BEHIND);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_USP_L_FROM_BEHIND);
			}
			work->npc.action.dir=work->homing_dir;
			break;
		case SNAKE_STATUS_HAS_USP|SNAKE_STATUS_SITTING_MOTION:
		case SNAKE_STATUS_SITTING_MOTION:   /* この条件は入れるべき？？？ */
			DEBUG_PRINT_NPCSNAKE("Start Shoot from hiding\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_USP_R_FROM_BEHIND_AND_SITTING);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_USP_L_FROM_BEHIND_AND_SITTING);
			}
			work->npc.action.dir=work->homing_dir;
			break;
		case SNAKE_STATUS_HAS_FMS:
			DEBUG_PRINT_NPCSNAKE("Start Shoot from hiding\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_FMS_R_FROM_BEHIND);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_FMS_L_FROM_BEHIND);
			}
			work->npc.action.dir=work->homing_dir;
			break;
		case SNAKE_STATUS_HAS_FMS|SNAKE_STATUS_SITTING_MOTION:
			DEBUG_PRINT_NPCSNAKE("Start Shoot from hiding\n");
			if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING);
			}
			else{
				ChangeNewPad(&(work->npc),PAD_START_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING);
			}
			work->npc.action.dir=work->homing_dir;
			break;
		default:
			DEBUG_PRINT_NPCSNAKE("Error : Start Shoot from hiding\n");
			break;
		}
		break;
	}
}

static void ActBackToHide(Work *work)
{
	NPCWORK	*npc;

	npc=&work->npc;

	ChangeHazardSphere(&(work->ctrl),R_SHPERE_HIDE,S_SHPERE_HIDE);

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	switch(npc->action.set_pad){
	case PAD_BEHIND:
	case PAD_BEHIND_WUSP:
	case PAD_BEHIND_WFMS:

	case PAD_BEHIND_AND_SITTING:
	case PAD_BEHIND_AND_SITTING_WUSP:
	case PAD_BEHIND_AND_SITTING_WFMS:
		NPCSNAKE_HIDE_SET(work,NPCSNAKE_HIDE_HIDING);
		work->hide_count=0;
		break;

	case PAD_LOOK_L_BEHIND:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_L_BEHIND);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_R_BEHIND:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_R_BEHIND);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_L_BEHIND_AND_SITTING:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_L_BEHIND_AND_SITTING);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_R_BEHIND_AND_SITTING:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_R_BEHIND_AND_SITTING);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_L_BEHIND_WUSP:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_L_BEHIND_WUSP);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_R_BEHIND_WUSP:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_R_BEHIND_WUSP);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_L_BEHIND_AND_SITTING_WUSP:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_L_BEHIND_AND_SITTING_WUSP);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_R_BEHIND_AND_SITTING_WUSP:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_R_BEHIND_AND_SITTING_WUSP);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_L_BEHIND_WFMS:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_L_BEHIND_WFMS);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_R_BEHIND_WFMS:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_R_BEHIND_WFMS);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_L_BEHIND_AND_SITTING_WFMS:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_L_BEHIND_AND_SITTING_WFMS);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;
	case PAD_LOOK_R_BEHIND_AND_SITTING_WFMS:
		ChangeNewPad(&(work->npc),PAD_END_LOOK_R_BEHIND_AND_SITTING_WFMS);

		/* モーションの方向 */
		npc->action.dir=work->behind_dir;
		break;

	case PAD_STAND_WUSP:
	case PAD_SHOOT_USP:
		if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_USP_R_FROM_BEHIND);
		}
		else{
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_USP_L_FROM_BEHIND);
		}
		/* モーションの方向 */
		npc->action.dir=work->behind_dir+0x800;
		break;
	case PAD_STAND_WFMS:
	case PAD_SHOOT_FMS:
		if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_FMS_R_FROM_BEHIND);
		}
		else{
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_FMS_L_FROM_BEHIND);
		}
		/* モーションの方向 */
		npc->action.dir=work->behind_dir+0x800;
		break;
	case PAD_SITTING_WUSP:
	case PAD_SITTING_AND_SHOOT_USP:
		if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_USP_R_FROM_BEHIND_AND_SITTING);
		}
		else{
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_USP_L_FROM_BEHIND_AND_SITTING);
		}
		/* モーションの方向 */
		npc->action.dir=work->behind_dir+0x800;
		break;
	case PAD_SITTING_WFMS:
	case PAD_SITTING_AND_SHOOT_FMS:
		if(work->behind_posflag & POS_FLAG_PEEP_RIGHT){
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING);
		}
		else{
			ChangeNewPad(&(work->npc),PAD_END_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING);
		}
		/* モーションの方向 */
		npc->action.dir=work->behind_dir+0x800;
		break;
	}
}

static void ActPunch(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	npc->action.dir=work->homing_dir;

	SET_SCN_NORMAL();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	if(work->snake_status & SNAKE_STATUS_HAS_USP){
		ChangeNewPad(npc,PAD_PUNCH_WUSP);
	}
	else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
		ChangeNewPad(npc,PAD_PUNCH_WFMS);
	}
	else{
		ChangeNewPad(npc,PAD_PUNCH);
	}
}

static void ActShootUSP(Work *work)
{
	NPCWORK	*npc ;
	CONTROL *ctrl;

	npc = &work->npc ;
	ctrl = npc->ctrl;

	ChangeHazardSphere(ctrl,R_SHPERE_HIDE,S_SHPERE_HIDE);

	SET_SCN_SHOOT();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY){
		if(ActMoveSelecter(work)) NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);
		// work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		{
			int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

			if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);

				/* 移動方向指定 */
				npc->action.dir=-1;
			}
		}
	}
   // BP_WARNING - This if does nothing..  something&&something_else
	else if(NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_NO_HIDING &&
			NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_MOVE_TO_HIDE){

		/* 移動方向指定 */
		npc->action.dir=-1;
	}

	switch(work->npc.action.set_pad){
	case PAD_BEHIND:
	case PAD_BEHIND_WUSP:
	case PAD_BEHIND_WFMS:
		ChangeNewPad(&(work->npc),PAD_STAND_WUSP);
		ActSubMotionUnlock(work);
		break;
	case PAD_BEHIND_AND_SITTING:
	case PAD_BEHIND_AND_SITTING_WUSP:
	case PAD_BEHIND_AND_SITTING_WFMS:
		ChangeNewPad(&(work->npc),PAD_SITTING_WUSP);
		ActSubMotionUnlock(work);
		break;
	}

	if(npc->action.dir==-1){
		int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

		if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
			StartSubMotion_BeginUSP(work);
			return;
		}
	}
	if(work->sub_pad==PAD_SHOOT_USP){
		StartSubMotion_ShootUSP(work);
	}
	else{
		StartSubMotion_BeginUSP(work);
	}
}

static void ActShootFMS(Work *work)
{
	NPCWORK	*npc ;
	CONTROL *ctrl;

	npc = &work->npc ;
	ctrl = npc->ctrl;

	ChangeHazardSphere(ctrl,R_SHPERE_HIDE,S_SHPERE_HIDE);

	SET_SCN_SHOOT();
	work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;

	if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY){
		if(ActMoveSelecter(work)) NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);
		// work->snake_status&=~SNAKE_STATUS_ACTION_FINISHED;

		{
			int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

			if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_STAY);

				/* 移動方向指定 */
				npc->action.dir=-1;
			}
		}
	}
   // BP_WARNING - This if does nothing..  something&&something_else
	else if(NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_NO_HIDING &&
			NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_MOVE_TO_HIDE){

		/* 移動方向指定 */
		npc->action.dir=-1;
	}

	switch(work->npc.action.set_pad){
	case PAD_BEHIND:
	case PAD_BEHIND_WUSP:
	case PAD_BEHIND_WFMS:
		ChangeNewPad(&(work->npc),PAD_STAND_WFMS);
		ActSubMotionUnlock(work);
		break;
	case PAD_BEHIND_AND_SITTING:
	case PAD_BEHIND_AND_SITTING_WUSP:
	case PAD_BEHIND_AND_SITTING_WFMS:
		ChangeNewPad(&(work->npc),PAD_SITTING_WFMS);
		ActSubMotionUnlock(work);
		break;
	}

	if(npc->action.dir==-1){
		int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

		if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
			StartSubMotion_BeginFMS(work);
			return;
		}
	}
	if(work->sub_pad==PAD_SHOOT_FMS){
		StartSubMotion_ShootFMS(work);
	}
	else{
		StartSubMotion_BeginFMS(work);
	}
}


void ExecAction(Work *work)
{

#ifdef DEBUG_MODE
	if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY &&
	   NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_PEEPING){

		// DEBUG_PRINT_NPCSNAKE("Now Peeping : Mode = 0x%08x\n",NPCSNAKE_ACT_MODE(work));
	}
#endif

	ChangeHazardSphere(&(work->ctrl),R_SHPERE_NORMAL,S_SHPERE_NORMAL);

	switch(NPCSNAKE_SHOOT_MODE(work)){
	case NPCSNAKE_SHOOT:
		if(NPCSNAKE_HIDE_MODE(work)==NPCSNAKE_HIDE_ATTACK){
			ActAttack(work);
		}
		else{
			if(work->snake_status & SNAKE_STATUS_HAS_FMS){
				ActShootFMS(work);
			}
			else{
				ActShootUSP(work);
			}
		}
		break;
	case NPCSNAKE_PUNCH:
		ActPunch(work);
		break;
	default:
		if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY){
			ActMoveSelecter(work);
		}
		else{
			switch(NPCSNAKE_HIDE_MODE(work)){
			case NPCSNAKE_HIDE_NO_HIDING:
				ActStay(work);
				break;
			case NPCSNAKE_HIDE_MOVE_TO_HIDE:
				ActHide(work);
				break;
			case NPCSNAKE_HIDE_HIDING:
				ActHiding(work);
				break;
			case NPCSNAKE_HIDE_SITDOWN_TO_HIDE:
				ActHidingSitdown(work);
				break;
			case NPCSNAKE_HIDE_PEEPING:
				ActPeeping(work);
				break;
			case NPCSNAKE_HIDE_ATTACK:
				ActAttack(work);
				break;
			case NPCSNAKE_HIDE_BACK_TO_HIDE:
				ActBackToHide(work);
				break;
			default:
				break;
			}
		}
		break;
	}

}
