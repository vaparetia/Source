/*
	snake_pad2.c
		NPCスネークPADルーチン

	2001/03/06 K.Kano
	$Id: snake_pad2.c,v 1.1.1.3 2002/11/19 11:43:23 Yoshizawa1 Exp $
 */


#define FRAME_READY_TO_FIRE_USP		DIRECT_TICK(10)
#define FRAME_READY_TO_FIRE_FMS		DIRECT_TICK(10)
#define FRAME_FINISH_TO_SHOOT_USP	DIRECT_TICK(4)
#define FRAME_FINISH_TO_SHOOT_FMS	DIRECT_TICK(3)

#define FRAME_FINISH_TO_SHOOT_PSG	DIRECT_TICK(10)


#define FRAME_SITDOWN				DIRECT_TICK(6)
#define FRAME_STANDUP				DIRECT_TICK(6)
#define FRAME_START_BEHIND					DIRECT_TICK(6)
#define FRAME_START_BEHIND_AND_SITTING		DIRECT_TICK(6)

#define FRAME_W32A_SLEEP_MOTION		DIRECT_TICK(5)


#define FINISH_CAREFUL				DIRECT_TICK(120)


#define NPCSNAKE_PUNCH1_COUNT		DIRECT_TICK(8)
#define NPCSNAKE_PUNCH2_COUNT		DIRECT_TICK(22)
#define NPCSNAKE_KICK1_COUNT		DIRECT_TICK(46)

#define NPCSNAKE_PUNCH_WM4A_COUNT		DIRECT_TICK(8)
#define NPCSNAKE_KICK_WM4A_COUNT		DIRECT_TICK(31)

#define VS_PUNCH_VITAL_POINT		5
#define VS_PUNCH_ATTACK_POINT		5
#define VS_PUNCH_FAINT_POINT		5
#define VS_KICK_VITAL_POINT			7
#define VS_KICK_ATTACK_POINT		7
#define VS_KICK_FAINT_POINT			7
#define VS_ROLLING_VITAL_POINT		10
#define VS_ROLLING_ATTACK_POINT		10
#define VS_ROLLING_FAINT_POINT		10


#define CalcAdjust0(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_CALCADJUST0)
#define CalcAdjust(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_CALCADJUST)
#define ShootBullet(_npc) \
	(ACCESS_WORK(_npc)->snake_status|=SNAKE_STATUS_MAKEBULLET)


/* ------------------------------------------------------------------------ */
/* Basic Motion                                                             */
/* ------------------------------------------------------------------------ */


static void ChangeSubMotion(NPCWORK *npc,int pad,void (*act)(NPCWORK *npc,int time));
static void SubMotion_None(NPCWORK *npc,int time);


void ActMotion_Stand( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;
	ctrl = npc->ctrl ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		ctrl->turn.vx = 0 ;
		ctrl->turn.vz = 0 ;
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

		MOTION_CANCELABLE_ON(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("Stand\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	if ( act->dir >= 0 ) {
		npc->ctrl->turn.vy = act->dir ;

		switch(act->set_pad){
		case PAD_STAND_WUSP:
			ChangeNewPad(npc,PAD_RUN_WUSP);
			break;
		case PAD_STAND_WFMS:
			ChangeNewPad(npc,PAD_RUN_WFMS);
			break;
		case PAD_STAND:
		default:
			ChangeNewPad(npc,PAD_RUN);
			break;
		}

		if(ACCESS_WORK(npc)->sub_pad==PAD_BRINGING_USP ||
		   ACCESS_WORK(npc)->sub_pad==PAD_BRINGING_FMS){

			ChangeSubMotion(npc,PAD_NONE,SubMotion_None);
		}

		return ;
	}

	/* 射撃の際には向いている方向を変更 */
	if(ACCESS_WORK(npc)->sub_pad!=PAD_NONE &&
	   ACCESS_WORK(npc)->sub_pad!=PAD_LOCK){

		ctrl->turn.vy=ACCESS_WORK(npc)->homing_dir;
	}
}

void ActMotion_Stand_BeCareful( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;
	ctrl = npc->ctrl ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		ctrl->turn.vx = 0 ;
		ctrl->turn.vz = 0 ;

		if(ACCESS_WORK(npc)->aftershoot_count>0){
			;
		}

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

		MOTION_CANCELABLE_ON(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("Stand BeCareful\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	if ( act->dir >= 0 ) {
		npc->ctrl->turn.vy = act->dir ;

		switch(act->set_pad){
		case PAD_STAND_WUSP:
			ChangeNewPad(npc,PAD_RUN_WUSP);
			break;
		case PAD_STAND_WFMS:
			ChangeNewPad(npc,PAD_RUN_WFMS);
			break;
		case PAD_STAND:
		default:
			ChangeNewPad(npc,PAD_RUN);
			break;
		}
		return ;
	}

	/* 射撃の際には向いている方向を変更 */
	if(ACCESS_WORK(npc)->sub_pad!=PAD_NONE &&
	   ACCESS_WORK(npc)->sub_pad!=PAD_LOCK){

		ctrl->turn.vy=ACCESS_WORK(npc)->homing_dir;
	}
}

void ActMotion_StandW17A( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;
	ctrl = npc->ctrl ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		ctrl->turn.vx = 0 ;
		ctrl->turn.vz = 0 ;
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

		MOTION_CANCELABLE_ON(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("Box Stop\n");
	}
	else{
		npc->body->m_ctrl->mt3_ctrl->flag|=MT3_SLEEP;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	if ( act->dir >= 0 ) {
		npc->ctrl->turn.vy = act->dir ;
	}
}

static void ActMotion_Move( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;
	ctrl = npc->ctrl ;

	NPC_ActStatus(act,NPC_ACT_STATUS_STAND|NPC_ACT_STATUS_MOVE);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;

		MOTION_CANCELABLE_ON(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("Move\n");
	}

	if ( npc->CheckDamage( npc ) ) {
		ctrl->turn.vz = 0 ;
		return ;
	}
	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) {
			ctrl->turn.vz = 0 ;
			return ;
		}
	}
	if ( act->dir < 0 ) {
		ctrl->turn.vz = 0 ;

		switch(act->set_pad){
		case PAD_WALK_WUSP:
		case PAD_RUN_WUSP:
			ChangeNewPad(npc,PAD_STAND_WUSP);
			break;
		case PAD_WALK_WFMS:
		case PAD_RUN_WFMS:
			ChangeNewPad(npc,PAD_STAND_WFMS);
			break;
		case PAD_WALK:
		case PAD_RUN:
		default:
			ChangeNewPad(npc,PAD_STAND);
			break;
		}

		if(ACCESS_WORK(npc)->sub_pad==PAD_BRINGING_USP ||
		   ACCESS_WORK(npc)->sub_pad==PAD_BRINGING_FMS){

			ChangeSubMotion(npc,PAD_NONE,SubMotion_None);
		}

		return ;
	}

	/* 方向転換時の体の傾き */
	NPC_Incline( ctrl ) ;
	/* 進行方向 */
	ctrl->turn.vy = act->dir ;

#if 0
	if ( speed != 0.0F ) {
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
		ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
	}
#endif
}

static void ActMotion_Sitdown( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("Sitdown\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( time>FRAME_SITDOWN ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		act->act_end = 1 ;

		switch(act->set_pad){
		case PAD_SITDOWN_WUSP:
			ChangeNewPad(npc,PAD_SITTING_WUSP);
			break;
		case PAD_SITDOWN_WFMS:
			ChangeNewPad(npc,PAD_SITTING_WFMS);
			break;
		case PAD_SITDOWN:
		default:
			ChangeNewPad(npc,PAD_SITTING);
			break;
		}

		ActSubMotionUnlock(ACCESS_WORK(npc));

		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActMotion_Standup( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("Standup\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( time>FRAME_STANDUP ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		act->act_end = 1 ;

		switch(act->set_pad){
		case PAD_STANDUP_WUSP:
			ChangeNewPad(npc,PAD_STAND_WUSP);
			break;
		case PAD_STANDUP_WFMS:
			ChangeNewPad(npc,PAD_STAND_WFMS);
			break;
		case PAD_STANDUP:
		default:
			ChangeNewPad(npc,PAD_STAND);
			break;
		}

		ActSubMotionUnlock(ACCESS_WORK(npc));

		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActMotion_CrouchOn( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("CrouchOn\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( time>FRAME_SITDOWN ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_CROUCHING);
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}


/* ------------------------------------------------------------------------ */
/* Behind                                                                   */
/* ------------------------------------------------------------------------ */


static void ActMotion_StartBehind( NPCWORK *npc, int time )
{
	NPCACT *act ;
	FVECTOR *pos=ACCESS_WORK(npc)->behind_pos;

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}
	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("StartBehind\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( time>=FRAME_START_BEHIND ) {
		MOTION_CANCELABLE_ON(npc);

		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);

		act->act_end = 1 ;

		fpu_CopyVector(&(npc->ctrl->mov),pos);

		switch(act->set_pad){
		case PAD_START_BEHIND_AND_SITTING_WUSP:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WUSP);
			break;
		case PAD_START_BEHIND_AND_SITTING_WFMS:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WFMS);
			break;
		case PAD_START_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING);
			break;
		case PAD_START_BEHIND_WUSP:
			ChangeNewPad(npc,PAD_BEHIND_WUSP);
			break;
		case PAD_START_BEHIND_WFMS:
			ChangeNewPad(npc,PAD_BEHIND_WFMS);
			break;
		case PAD_START_BEHIND:
		default:
			ChangeNewPad(npc,PAD_BEHIND);
			break;
		}
		return ;
	}
	else{
		vu0_Ldv0(&(npc->ctrl->mov));
		vu0_Ldv1(pos);

		vu0_Subv1v0();
		vu0_Mulv1a(1.0f/(float)(FRAME_START_BEHIND-time));

		vu0_Addv0v1();

		vu0_Stv0(&(npc->ctrl->mov));
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActMotion_EndBehind( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}
	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("EndBehind\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( time>=FRAME_START_BEHIND ) {
		MOTION_CANCELABLE_ON(npc);
		act->act_end = 1 ;

		switch(act->set_pad){
		case PAD_END_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_SITTING);
			ActSubMotionUnlock(ACCESS_WORK(npc));
			break;
		case PAD_END_BEHIND_AND_SITTING_WUSP:
			ChangeNewPad(npc,PAD_SITTING_WUSP);
			ActSubMotionUnlock(ACCESS_WORK(npc));
			break;
		case PAD_END_BEHIND_AND_SITTING_WFMS:
			ChangeNewPad(npc,PAD_SITTING_WFMS);
			ActSubMotionUnlock(ACCESS_WORK(npc));
			break;
		case PAD_END_BEHIND:
			ChangeNewPad(npc,PAD_STAND);
			ActSubMotionUnlock(ACCESS_WORK(npc));
			break;
		case PAD_END_BEHIND_WUSP:
			ChangeNewPad(npc,PAD_STAND_WUSP);
			ActSubMotionUnlock(ACCESS_WORK(npc));
			break;
		case PAD_END_BEHIND_WFMS:
			ChangeNewPad(npc,PAD_STAND_WFMS);
			ActSubMotionUnlock(ACCESS_WORK(npc));
			break;
		}

		return ;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActMotion_StartEndLookBehind( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}
	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("StartEndLookBehind\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);

		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);

		act->act_end = 1 ;

		switch(act->set_pad){
		case PAD_START_LOOK_L_BEHIND_AND_SITTING_WUSP:
			ChangeNewPad(npc,PAD_LOOK_L_BEHIND_AND_SITTING_WUSP);
			break;
		case PAD_START_LOOK_L_BEHIND_AND_SITTING_WFMS:
			ChangeNewPad(npc,PAD_LOOK_L_BEHIND_AND_SITTING_WFMS);
			break;
		case PAD_START_LOOK_L_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_LOOK_L_BEHIND_AND_SITTING);
			break;
		case PAD_START_LOOK_L_BEHIND_WUSP:
			ChangeNewPad(npc,PAD_LOOK_L_BEHIND_WUSP);
			break;
		case PAD_START_LOOK_L_BEHIND_WFMS:
			ChangeNewPad(npc,PAD_LOOK_L_BEHIND_WFMS);
			break;
		case PAD_START_LOOK_L_BEHIND:
			ChangeNewPad(npc,PAD_LOOK_L_BEHIND);
			break;
		case PAD_START_LOOK_R_BEHIND_AND_SITTING_WUSP:
			ChangeNewPad(npc,PAD_LOOK_R_BEHIND_AND_SITTING_WUSP);
			break;
		case PAD_START_LOOK_R_BEHIND_AND_SITTING_WFMS:
			ChangeNewPad(npc,PAD_LOOK_R_BEHIND_AND_SITTING_WFMS);
			break;
		case PAD_START_LOOK_R_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_LOOK_R_BEHIND_AND_SITTING);
			break;
		case PAD_START_LOOK_R_BEHIND_WUSP:
			ChangeNewPad(npc,PAD_LOOK_R_BEHIND_WUSP);
			break;
		case PAD_START_LOOK_R_BEHIND_WFMS:
			ChangeNewPad(npc,PAD_LOOK_R_BEHIND_WFMS);
			break;
		case PAD_START_LOOK_R_BEHIND:
			ChangeNewPad(npc,PAD_LOOK_R_BEHIND);
			break;

		case PAD_END_LOOK_L_BEHIND:
		case PAD_END_LOOK_R_BEHIND:
			ChangeNewPad(npc,PAD_BEHIND);
			break;
		case PAD_END_LOOK_L_BEHIND_WUSP:
		case PAD_END_LOOK_R_BEHIND_WUSP:
			ChangeNewPad(npc,PAD_BEHIND_WUSP);
			break;
		case PAD_END_LOOK_L_BEHIND_WFMS:
		case PAD_END_LOOK_R_BEHIND_WFMS:
			ChangeNewPad(npc,PAD_BEHIND_WFMS);
			break;
		case PAD_END_LOOK_L_BEHIND_AND_SITTING:
		case PAD_END_LOOK_R_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING);
			break;
		case PAD_END_LOOK_L_BEHIND_AND_SITTING_WUSP:
		case PAD_END_LOOK_R_BEHIND_AND_SITTING_WUSP:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WUSP);
			break;
		case PAD_END_LOOK_L_BEHIND_AND_SITTING_WFMS:
		case PAD_END_LOOK_R_BEHIND_AND_SITTING_WFMS:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WFMS);
			break;
		}
		return ;
	}
}


/* ------------------------------------------------------------------------ */
/* Punch & Kick                                                             */
/* ------------------------------------------------------------------------ */


static void ActMotion_PunchKick(NPCWORK *npc,int time)
{
	NPCACT *act ;
	FVECTOR force={ 0.0f,0.0f,0.0f,1.0f, };

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	NPC_ActStatus(act,NPC_ACT_SNAKE_PUNCH_KICK);


	// printf("Time = %d\n",time);

////






   if ( time == 0 )
   {
      npc->ctrl->turn.vx = 0 ;
      npc->ctrl->turn.vz = 0 ;

      NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
      NPC_MotionLayerOverOff(npc,1);
      NPC_MotionLayerOverOff(npc,2);

      MOTION_CANCELABLE_OFF(npc);

      if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

      // ASSERT(0);
      DEBUG_PRINT_NPCSNAKE("PunchKick\n");
   }
   else if ( (time == NPCSNAKE_PUNCH1_COUNT-1) ||
             (time == NPCSNAKE_PUNCH1_COUNT) ||
             (time == NPCSNAKE_PUNCH1_COUNT+1) )
   {
#if 1
      GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[0]),&(ACCESS_WORK(npc)->pow[0]),POWER_ONCE,
         VS_PUNCH_VITAL_POINT,VS_PUNCH_FAINT_POINT,VS_PUNCH_ATTACK_POINT,&force);
      GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[1]),&(ACCESS_WORK(npc)->pow[1]),POWER_ONCE,
         VS_PUNCH_VITAL_POINT,VS_PUNCH_FAINT_POINT,VS_PUNCH_ATTACK_POINT,&force);
#endif

      GM_PutTarget(&(ACCESS_WORK(npc)->attack[0]));
      GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[0]),
         &(npc->body->objs->objs[HUMAN21_MIGI_UDE1].world));
      GM_PutTarget(&(ACCESS_WORK(npc)->attack[1]));
      GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[1]),
         &(npc->body->objs->objs[HUMAN21_MIGI_UDE2].world));
   }
   else if ( (time == NPCSNAKE_PUNCH2_COUNT-1) ||
      (time == NPCSNAKE_PUNCH2_COUNT) ||
      (time == NPCSNAKE_PUNCH2_COUNT+1) )
   {
#if 1
      GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[2]),&(ACCESS_WORK(npc)->pow[2]),POWER_ONCE,
         VS_PUNCH_VITAL_POINT,VS_PUNCH_FAINT_POINT,VS_PUNCH_ATTACK_POINT,&force);
      GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[3]),&(ACCESS_WORK(npc)->pow[3]),POWER_ONCE,
         VS_PUNCH_VITAL_POINT,VS_PUNCH_FAINT_POINT,VS_PUNCH_ATTACK_POINT,&force);
#endif

      GM_PutTarget(&(ACCESS_WORK(npc)->attack[2]));
      GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[2]),
         &(npc->body->objs->objs[HUMAN21_HIDARI_UDE1].world));
      GM_PutTarget(&(ACCESS_WORK(npc)->attack[3]));
      GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[3]),
         &(npc->body->objs->objs[HUMAN21_HIDARI_UDE2].world));
   }
   else if ( (time == NPCSNAKE_KICK1_COUNT-3) ||
             (time == NPCSNAKE_KICK1_COUNT-2) ||
             (time == NPCSNAKE_KICK1_COUNT-1) ||
             (time == NPCSNAKE_KICK1_COUNT) ||
             (time == NPCSNAKE_KICK1_COUNT+1) ||
             (time == NPCSNAKE_KICK1_COUNT+2) ||
             (time == NPCSNAKE_KICK1_COUNT+3) )
   {
#if 1
      GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[4]),&(ACCESS_WORK(npc)->pow[4]),POWER_ONCE,
         VS_KICK_VITAL_POINT,VS_KICK_FAINT_POINT,VS_KICK_ATTACK_POINT,&force);
      GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[5]),&(ACCESS_WORK(npc)->pow[5]),POWER_ONCE,
         VS_KICK_VITAL_POINT,VS_KICK_FAINT_POINT,VS_KICK_ATTACK_POINT,&force);
#endif

      GM_PutTarget(&(ACCESS_WORK(npc)->attack[4]));
      GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[4]),
         &(npc->body->objs->objs[HUMAN21_HIDARI_ASHI1].world));
      GM_PutTarget(&(ACCESS_WORK(npc)->attack[5]));
      GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[5]),
         &(npc->body->objs->objs[HUMAN21_HIDARI_ASHI2].world));
   }

   if(time>=NPCSNAKE_KICK1_COUNT+DIRECT_TICK(6)){
      npc->ctrl->step.vx+=ACCESS_WORK(npc)->damage_mov_v.vx;
      npc->ctrl->step.vz+=ACCESS_WORK(npc)->damage_mov_v.vz;
   }

   if ( npc->CheckDamage( npc ) ) return ;

   if ( act->pad != act->set_pad ) {
      if ( npc->CheckPad( npc ) ) return;
   }

   if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
      ACTION_FINISHED(npc);
      SHOOT_ACTION_FINISHED(npc);

      MOTION_CANCELABLE_ON(npc);

      if(ACCESS_WORK(npc)->snake_status & SNAKE_STATUS_HAS_USP){
         ChangeNewPad(npc,PAD_STAND_WUSP);
      }
      else if(ACCESS_WORK(npc)->snake_status & SNAKE_STATUS_HAS_FMS){
         ChangeNewPad(npc,PAD_STAND_WFMS);
      }
      else{
         ChangeNewPad(npc,PAD_STAND);
      }
      ActSubMotionUnlock(ACCESS_WORK(npc));
   }
}

static void ActMotion_PunchKickWM4A(NPCWORK *npc,int time)
{
	NPCACT *act ;
	FVECTOR force={ 0.0f,0.0f,0.0f,1.0f, };

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	NPC_ActStatus(act,NPC_ACT_SNAKE_PUNCH_KICK);


	// printf("Time = %d\n",time);


   if ( time== 0 )
   {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("PunchKick\n");
   }
   else if ((time==NPCSNAKE_PUNCH_WM4A_COUNT-1) ||
	         (time==NPCSNAKE_PUNCH_WM4A_COUNT) ||
            (time==NPCSNAKE_PUNCH_WM4A_COUNT+1) )
   {
#if 1
		GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[2]),&(ACCESS_WORK(npc)->pow[2]),POWER_ONCE,
						  VS_PUNCH_VITAL_POINT,VS_PUNCH_FAINT_POINT,VS_PUNCH_ATTACK_POINT,&force);
		GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[3]),&(ACCESS_WORK(npc)->pow[3]),POWER_ONCE,
						  VS_PUNCH_VITAL_POINT,VS_PUNCH_FAINT_POINT,VS_PUNCH_ATTACK_POINT,&force);
#endif

		GM_PutTarget(&(ACCESS_WORK(npc)->attack[2]));
		GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[2]),
					   &(npc->body->objs->objs[HUMAN21_HIDARI_UDE1].world));
		GM_PutTarget(&(ACCESS_WORK(npc)->attack[3]));
		GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[3]),
					   &(npc->body->objs->objs[HUMAN21_HIDARI_UDE2].world));
   }
   else if ( (time==NPCSNAKE_KICK_WM4A_COUNT-3) ||
             (time==NPCSNAKE_KICK_WM4A_COUNT-2) ||
             (time==NPCSNAKE_KICK_WM4A_COUNT-1) ||
             (time==NPCSNAKE_KICK_WM4A_COUNT) ||
             (time==NPCSNAKE_KICK_WM4A_COUNT+1) ||
             (time==NPCSNAKE_KICK_WM4A_COUNT+2) ||
             (time==NPCSNAKE_KICK_WM4A_COUNT+3) )
   {
#if 1
		GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[4]),&(ACCESS_WORK(npc)->pow[4]),POWER_ONCE,
						  VS_KICK_VITAL_POINT,VS_KICK_FAINT_POINT,VS_KICK_ATTACK_POINT,&force);
		GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[5]),&(ACCESS_WORK(npc)->pow[5]),POWER_ONCE,
						  VS_KICK_VITAL_POINT,VS_KICK_FAINT_POINT,VS_KICK_ATTACK_POINT,&force);
#endif

		GM_PutTarget(&(ACCESS_WORK(npc)->attack[4]));
		GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[4]),
					   &(npc->body->objs->objs[HUMAN21_HIDARI_ASHI1].world));
		GM_PutTarget(&(ACCESS_WORK(npc)->attack[5]));
		GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[5]),
					   &(npc->body->objs->objs[HUMAN21_HIDARI_ASHI2].world));
	}

	if(time>=NPCSNAKE_KICK_WM4A_COUNT+DIRECT_TICK(6)){
		npc->ctrl->step.vx+=ACCESS_WORK(npc)->damage_mov_v.vx;
		npc->ctrl->step.vz+=ACCESS_WORK(npc)->damage_mov_v.vz;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);

		MOTION_CANCELABLE_ON(npc);

		if(ACCESS_WORK(npc)->snake_status & SNAKE_STATUS_HAS_USP){
			ChangeNewPad(npc,PAD_STAND_WUSP);
		}
		else if(ACCESS_WORK(npc)->snake_status & SNAKE_STATUS_HAS_FMS){
			ChangeNewPad(npc,PAD_STAND_WFMS);
		}
		else{
			ChangeNewPad(npc,PAD_STAND);
		}
		ActSubMotionUnlock(ACCESS_WORK(npc));
	}
}


/* ------------------------------------------------------------------------ */
/* Shoot                                                                    */
/* ------------------------------------------------------------------------ */


static void ActMotion_BeginUSP(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_STAND);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		LOCKON_ENEMY(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("BeginUSP\n");
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ){
			UNLOCK_ENEMY(npc);
			return;
		}
	}

	if ( time>=FRAME_READY_TO_FIRE_USP ) {
		MOTION_CANCELABLE_ON(npc);
		ChangeNewPad(npc,PAD_SHOOT_USP);
		MOTION_CANCELABLE_OFF(npc);
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);
}

static void ActMotion_BeginUSPFromBehind(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}
	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		LOCKON_ENEMY(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("BeginUSPFromBehind\n");
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ){
			UNLOCK_ENEMY(npc);
			return;
		}
	}

#if 1
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
#else
	if ( GM_CheckObject_PlayEnd( npc->body, 0 ) ) {
#endif
		int dir;

		dir=(ACCESS_WORK(npc)->homing_dir-npc->ctrl->rot.vy+0x800) & 0xfff;

		MOTION_CANCELABLE_ON(npc);

		if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
			if(SITTING_MOTION_FLAG(npc)){
				ChangeNewPad(npc,PAD_SITTING_AND_START_SHOOT_USP);
			}
			else{
				ChangeNewPad(npc,PAD_START_SHOOT_USP);
			}
		}
		else{
			if(SITTING_MOTION_FLAG(npc)){
				ChangeNewPad(npc,PAD_SITTING_AND_SHOOT_USP);
			}
			else{
				ChangeNewPad(npc,PAD_SHOOT_USP);
			}
		}

		MOTION_CANCELABLE_OFF(npc);
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

	CalcAdjust(npc);
}

static void ActMotion_BeginFMS(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_STAND);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		LOCKON_ENEMY(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("BeginFMS\n");
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ){
			UNLOCK_ENEMY(npc);
			return;
		}
	}

	if ( time>=FRAME_READY_TO_FIRE_FMS ) {
		MOTION_CANCELABLE_ON(npc);
		ChangeNewPad(npc,PAD_SHOOT_FMS);
		MOTION_CANCELABLE_OFF(npc);
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);
}

static void ActMotion_BeginFMSFromBehind(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}
	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		LOCKON_ENEMY(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("BeginFMSFromBehind\n");
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ){
			UNLOCK_ENEMY(npc);
			return;
		}
	}

#if 1
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
#else
	if ( GM_CheckObject_PlayEnd( npc->body, 0 ) ) {
#endif

		int dir;

		dir=(ACCESS_WORK(npc)->homing_dir-npc->ctrl->rot.vy+0x800) & 0xfff;

		MOTION_CANCELABLE_ON(npc);

		if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
			if(SITTING_MOTION_FLAG(npc)){
				ChangeNewPad(npc,PAD_SITTING_AND_START_SHOOT_FMS);
			}
			else{
				ChangeNewPad(npc,PAD_START_SHOOT_FMS);
			}
		}
		else{
			if(SITTING_MOTION_FLAG(npc)){
				ChangeNewPad(npc,PAD_SITTING_AND_SHOOT_FMS);
			}
			else{
				ChangeNewPad(npc,PAD_SHOOT_FMS);
			}
		}

		MOTION_CANCELABLE_OFF(npc);
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

	CalcAdjust(npc);
}

static void ActMotion_SittingAndBeginUSP(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		/* layer 0は最後に設定しないといけないという仕様が
		   あったため、この順序になっている */
		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_START_SHOOT_USP,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

#if 0
		NPC_SetActMotion(npc,act->set_mar,PAD_SITTING);
#else
		NPC_SetActMotionEX(npc,0,act->set_mar,PAD_SITTING,
						   MOTION_MASK_LOWER,ACT_INTERP_DEF,(float)TIME_BASE);
#endif

		MOTION_CANCELABLE_OFF(npc);

		LOCKON_ENEMY(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("SittingAndBeginUSP\n");
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ){
			UNLOCK_ENEMY(npc);
			return;
		}
	}

	if ( time>=FRAME_READY_TO_FIRE_USP ) {
		MOTION_CANCELABLE_ON(npc);
		ChangeNewPad(npc,PAD_SITTING_AND_SHOOT_USP);
		MOTION_CANCELABLE_OFF(npc);
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);
}

static void ActMotion_SittingAndBeginFMS(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		/* layer 0は最後に設定しないといけないという仕様が
		   あったため、この順序になっている */
		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_START_SHOOT_FMS,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

#if 0
		NPC_SetActMotion(npc,act->set_mar,PAD_SITTING);
#else
		NPC_SetActMotionEX(npc,0,act->set_mar,PAD_SITTING,
						   MOTION_MASK_LOWER,ACT_INTERP_DEF,(float)TIME_BASE);
#endif

		MOTION_CANCELABLE_OFF(npc);

		LOCKON_ENEMY(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("SittingAndBeginFMS\n");
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ){
			UNLOCK_ENEMY(npc);
			return;
		}
	}

	if ( time>=FRAME_READY_TO_FIRE_FMS ) {
		MOTION_CANCELABLE_ON(npc);
		ChangeNewPad(npc,PAD_SITTING_AND_SHOOT_FMS);
		MOTION_CANCELABLE_OFF(npc);
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);
}

static void ActMotion_ShootUSP(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_STAND);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	switch(time){
	case 0:
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		LOCKON_ENEMY(npc);
		MOTION_CANCELABLE_OFF(npc);

		break;
	case 1:
		/* 弾丸発射 */
		ShootBullet(npc);

		DEBUG_PRINT_NPCSNAKE("ShootUSP %f\n",ACCESS_WORK(npc)->homing_dis);
		break;
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

#if 0
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
#endif

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);

	if(time>=FRAME_FINISH_TO_SHOOT_USP+1){
		MOTION_CANCELABLE_ON(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_STAND_WUSP);
		return ;
	}
	if(time>=FRAME_FINISH_TO_SHOOT_USP){
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		UNLOCK_ENEMY(npc);
		return ;
	}
}

static void ActMotion_ShootFMS(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_STAND);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	switch(time){
	case 0:
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		LOCKON_ENEMY(npc);
		MOTION_CANCELABLE_OFF(npc);
		break;
	case 1:
		/* 弾丸発射 */
		ShootBullet(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("ShootFMS %f\n",ACCESS_WORK(npc)->homing_dis);
		break;
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

#if 0
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
#endif

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);

	if(time>=FRAME_FINISH_TO_SHOOT_FMS+1){
		MOTION_CANCELABLE_ON(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_STAND_WFMS);
		return ;
	}
	if(time>=FRAME_FINISH_TO_SHOOT_FMS){
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;
}

static void ActMotion_SittingAndShootUSP(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	switch(time){
	case 0:
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		/* layer 0は最後に設定しないといけないという仕様が
		   あったため、この順序になっている */
		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_SHOOT_USP,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

#if 0
		NPC_SetActMotion(npc,act->set_mar,PAD_SITTING);
#else
		NPC_SetActMotionEX(npc,0,act->set_mar,PAD_SITTING,
						   MOTION_MASK_LOWER,ACT_INTERP_DEF,(float)TIME_BASE);
#endif

		LOCKON_ENEMY(npc);
		MOTION_CANCELABLE_OFF(npc);
		break;
	case 1:
		/* 弾丸発射 */
		ShootBullet(npc);

		DEBUG_PRINT_NPCSNAKE("SittingAndShootUSP %f\n",ACCESS_WORK(npc)->homing_dis);
		break;
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}
#if 0
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
#endif

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);

	if(time>=FRAME_FINISH_TO_SHOOT_USP+1){
		MOTION_CANCELABLE_ON(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_SITTING_WUSP);
		return ;
	}
	if(time>=FRAME_FINISH_TO_SHOOT_USP){
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;
}

static void ActMotion_SittingAndShootFMS(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	switch(time){
	case 0:
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		/* layer 0は最後に設定しないといけないという仕様が
		   あったため、この順序になっている */
		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_SHOOT_FMS,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

#if 0
		NPC_SetActMotion(npc,act->set_mar,PAD_SITTING);
#else
		NPC_SetActMotionEX(npc,0,act->set_mar,PAD_SITTING,
						   MOTION_MASK_LOWER,ACT_INTERP_DEF,(float)TIME_BASE);
#endif

		LOCKON_ENEMY(npc);
		MOTION_CANCELABLE_OFF(npc);
		break;
	case 1:
		/* 弾丸発射 */
		ShootBullet(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("SittingAndShootFMS %f\n",ACCESS_WORK(npc)->homing_dis);
		break;
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}
#if 0
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
#endif

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);

	if(time>=FRAME_FINISH_TO_SHOOT_FMS+1){
		MOTION_CANCELABLE_ON(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_SITTING_WFMS);
		return ;
	}
	if(time>=FRAME_FINISH_TO_SHOOT_FMS){
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		UNLOCK_ENEMY(npc);
		return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	// npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir ;
}

static void ActMotion_EndShootFromBehind(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("EndShootFromBehind\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		act->act_end = 1 ;

		switch(act->set_pad){
		case PAD_END_SHOOT_USP_L_FROM_BEHIND:
		case PAD_END_SHOOT_USP_R_FROM_BEHIND:
			ChangeNewPad(npc,PAD_BEHIND_WUSP);
			break;
		case PAD_END_SHOOT_USP_L_FROM_BEHIND_AND_SITTING:
		case PAD_END_SHOOT_USP_R_FROM_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WUSP);
			break;
		case PAD_END_SHOOT_FMS_L_FROM_BEHIND:
		case PAD_END_SHOOT_FMS_R_FROM_BEHIND:
			ChangeNewPad(npc,PAD_BEHIND_WFMS);
			break;
		case PAD_END_SHOOT_FMS_L_FROM_BEHIND_AND_SITTING:
		case PAD_END_SHOOT_FMS_R_FROM_BEHIND_AND_SITTING:
			ChangeNewPad(npc,PAD_BEHIND_AND_SITTING_WFMS);
			break;
		}
		return ;
	}
}

static void ActMotion_ShootPSG(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	// NPC_ActStatus(act,NPC_ACT_STATUS_STAND);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	switch(time){
	case 0:
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		/* 弾丸発射 */
		ShootBullet(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("ShootPSG %f\n",ACCESS_WORK(npc)->homing_dis);

		LOCKON_ENEMY(npc);
		MOTION_CANCELABLE_OFF(npc);
		break;
	}

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

#if 0
	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
#endif

	if(time>=FRAME_FINISH_TO_SHOOT_PSG+1){
		MOTION_CANCELABLE_ON(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_CROUCHING_WPSG);
		return ;
	}
	else if(time>=FRAME_FINISH_TO_SHOOT_PSG){
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		return ;
	}
}


/* ------------------------------------------------------------------------ */
/* Loop Motion                                                              */
/* ------------------------------------------------------------------------ */


static void ActSittingMotion(NPCWORK *npc,int time)
{
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);

		DEBUG_PRINT_NPCSNAKE("Sitting Motion\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

	/* 射撃の際には向いている方向を変更 */
	if(ACCESS_WORK(npc)->sub_pad!=PAD_NONE &&
	   ACCESS_WORK(npc)->sub_pad!=PAD_LOCK){

		npc->ctrl->turn.vy=ACCESS_WORK(npc)->homing_dir;
	}
}

static void ActLoopMotionWLock(NPCWORK *npc,int time)
{
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);

		DEBUG_PRINT_NPCSNAKE("Loop Motion With Lock\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActBringingMotion(NPCWORK *npc,int time)
{
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		/* layer 0は最後に設定しないといけないという仕様が
		   あったため、この順序になっている */
		NPC_SetActMotionEX(npc,1,act->set_mar,act->set_mot,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		if(SITTING_MOTION_FLAG(npc)){
			if(HAS_USP_MOTION_FLAG(npc)){
				NPC_SetActMotion(npc,act->set_mar,PAD_SITTING_WUSP);
			}
			else if(HAS_FMS_MOTION_FLAG(npc)){
				NPC_SetActMotion(npc,act->set_mar,PAD_SITTING_WFMS);
			}
			else{
				NPC_SetActMotion(npc,act->set_mar,PAD_SITTING);
			}
		}
		else{
			if(HAS_USP_MOTION_FLAG(npc)){
				NPC_SetActMotion(npc,act->set_mar,PAD_STAND_WUSP);
			}
			else if(HAS_FMS_MOTION_FLAG(npc)){
				NPC_SetActMotion(npc,act->set_mar,PAD_STAND_WFMS);
			}
			else{
				NPC_SetActMotion(npc,act->set_mar,PAD_STAND);
			}
		}

		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);

		DEBUG_PRINT_NPCSNAKE("Bringing Motion\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
	npc->ctrl->turn.vy = ACCESS_WORK(npc)->homing_dir;
}


/* ------------------------------------------------------------------------ */
/* w14a専用                                                                     */
/* ------------------------------------------------------------------------ */


static void ActLowerMotionW14A(NPCWORK *npc,NPCACT *act)
{
	int dir=(ACCESS_WORK(npc)->homing_dir-npc->ctrl->rot.vy) & 0x0fff;

	if(dir>0x200 && dir<0x800){
		/* 左45度以上 */
		if(npc->body->m_ctrl->mt3_ctrl[0].motion_num!=PAD_SITTING_AND_LOOK_LEFT_WM4A1){
			NPC_SetActMotion(npc,act->set_mar,PAD_SITTING_AND_LOOK_LEFT_WM4A1);
		}
	}
	else if(dir>=0x800 && dir<0xe00){
		/* 右45度以上 */
		if(npc->body->m_ctrl->mt3_ctrl[0].motion_num!=PAD_SITTING_AND_LOOK_RIGHT_WM4A1){
			NPC_SetActMotion(npc,act->set_mar,PAD_SITTING_AND_LOOK_RIGHT_WM4A1);
		}
	}
	else{
		if(npc->body->m_ctrl->mt3_ctrl[0].motion_num!=act->set_mot){
			NPC_SetActMotion(npc,act->set_mar,act->set_mot);
		}
	}
}

void ActLoopMotionWLockW14A(NPCWORK *npc,int time)
{
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}

	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);

		DEBUG_PRINT_NPCSNAKE("Loop Motion With Lock\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActBringM4A1W14A(NPCWORK *npc,int time)
{
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;

	if(!SITTING_MOTION_FLAG(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_STAND);
	}

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotionEX(npc,1,act->set_mar,act->set_mot,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);

		DEBUG_PRINT_NPCSNAKE("Bringing M4A1 Motion\n");
	}

	ActLowerMotionW14A(npc,act);

	if ( npc->CheckDamage( npc ) ) return ;

	CalcAdjust(npc);

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
}

static void ActStartShootM4A1W14A(NPCWORK *npc,int time)
{
	NPCACT *act ;

	// printf("\tNPC_SNAKE : time = %d\n",time);

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotionEX(npc,1,act->set_mar,act->set_mot,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("Shooting M4A1 Motion\n");
	}

	ActLowerMotionW14A(npc,act);

	if ( npc->CheckDamage( npc ) ){
		return ;
	}

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}

	CalcAdjust(npc);

	if ( time>=FRAME_READY_TO_FIRE_FMS ) {
		MOTION_CANCELABLE_ON(npc);
		ChangeNewPad(npc,PAD_SITTING_AND_SHOOT_WM4A1);
		MOTION_CANCELABLE_OFF(npc);
	}
}

static void ActShootM4A1W14A(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);

	switch(time){
	case 0:
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotionEX(npc,1,act->set_mar,act->set_mot,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);
		break;
	case 1:
		/* 弾丸発射 */
		ShootBullet(npc);

		// ASSERT(0);
		DEBUG_PRINT_NPCSNAKE("SittingAndShootM4A1 %f\n",ACCESS_WORK(npc)->homing_dis);
		break;
	}

	ActLowerMotionW14A(npc,act);

	if ( npc->CheckDamage( npc ) ){
		UNLOCK_ENEMY(npc);
		return ;
	}

	CalcAdjust(npc);

	if(time>=FRAME_FINISH_TO_SHOOT_FMS+1){
		MOTION_CANCELABLE_ON(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_SITTING_AND_BRINGING_WM4A1);
		return ;
	}
	if(time>=FRAME_FINISH_TO_SHOOT_FMS){
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);

		NPCSnake_GetPlayerTargetPos(npc);

		return ;
	}
}

static void ActDamageW14A(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion(npc,act->set_mar,PAD_SITTING_AND_DAMAGED_WM4A1) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);
		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("Damaged W14A\n");
	}

	if ( GM_CheckObject_IsEnd( npc->body,0 ) ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_SITTING_AND_SLEEP_WM4A1);

		ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_FLUSH_EYE;
		ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_EVM_CLOSE_EYE;

		return ;
	}
}


/* ------------------------------------------------------------------------ */
/* Damage                                                                   */
/* ------------------------------------------------------------------------ */


#define REGIST_A	0.6f


static void ActDamage(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	// NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_DAMAGE);
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

#if 0
	npc->ctrl->step.vx+=ACCESS_WORK(npc)->damage_mov_v.vx;
	npc->ctrl->step.vz+=ACCESS_WORK(npc)->damage_mov_v.vz;

	ACCESS_WORK(npc)->damage_mov_v.vx*=REGIST_A;
	ACCESS_WORK(npc)->damage_mov_v.vz*=REGIST_A;
#else
	if(time<DIRECT_TICK(4)){
		npc->ctrl->step.vx+=ACCESS_WORK(npc)->damage_mov_v.vx;
		npc->ctrl->step.vz+=ACCESS_WORK(npc)->damage_mov_v.vz;
	}
#endif

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);
		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("DamageFB\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

#if 0
	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		act->act_end = 1 ;

		if(!NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
			/* ダメージモーションが終った後、すぐに押しチェックが
			   入ると反撃がキャンセルされてしまうことへの対処 */
			NPCSNAKE_NODAMAGE_COUNTSET(npc,DIRECT_TICK(3));
			NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
		}

		if(HAS_USP_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WUSP);
		else if(HAS_FMS_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WFMS);
		else ChangeNewPad(npc,PAD_STAND);

		ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_FLUSH_EYE;
		ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_EVM_CLOSE_EYE;

		return ;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActDamage2(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	// NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_DAMAGE);
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);
	NPC_ActStatus(act,NPC_ACT_SNAKE_FORCE_MOVE);

	if(NPCSNAKE_NODAMAGE_COUNTCHECK(npc)){
		NPC_ActStatus(act,NPC_ACT_STATUS_TRG_OFF);
	}

	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

#if 0
	npc->ctrl->step.vx+=ACCESS_WORK(npc)->damage_mov_v.vx;
	npc->ctrl->step.vz+=ACCESS_WORK(npc)->damage_mov_v.vz;

	ACCESS_WORK(npc)->damage_mov_v.vx*=REGIST_A;
	ACCESS_WORK(npc)->damage_mov_v.vz*=REGIST_A;
#else
	if(time<DIRECT_TICK(8)){
		npc->ctrl->step.vx+=ACCESS_WORK(npc)->damage_mov_v.vx;
		npc->ctrl->step.vz+=ACCESS_WORK(npc)->damage_mov_v.vz;
	}
#endif

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		NPC_MotionLayerOverOff(npc,1);
		NPC_MotionLayerOverOff(npc,2);
		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("DamageFB\n");
	}

	if ( npc->CheckDamage( npc ) ) return ;

#if 0
	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if ( time>=DIRECT_TICK(15) ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		act->act_end = 1 ;

		if(HAS_USP_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WUSP);
		else if(HAS_FMS_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WFMS);
		else ChangeNewPad(npc,PAD_STAND);

		ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_FLUSH_EYE;
		ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_EVM_CLOSE_EYE;

		return ;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActWakeup(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);
		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("Wakeup\n");

		ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_FLUSH_EYE;
		ACCESS_WORK(npc)->snake_status2&=~SNAKE_STATUS2_EVM_CLOSE_EYE;
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		act->act_end = 1 ;

		ACCESS_WORK(npc)->ctrl.turn.vy=ACCESS_WORK(npc)->ctrl.rot.vy;

		if(HAS_USP_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WUSP);
		else if(HAS_FMS_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WFMS);
		else ChangeNewPad(npc,PAD_STAND);

		return ;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActKnockdown(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);
		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("Knockdown\n");
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);

		if(act->faint<0){
			if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
				ChangeNewPad(npc,PAD_DAMAGE_SLEEP_F);
			}
			else{
				ChangeNewPad(npc,PAD_DAMAGE_SLEEP_B);
			}
		}
		else{
			if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
				DEBUG_PRINT_NPCSNAKE("Wakeup F\n");
				ChangeNewPad(npc,PAD_DAMAGE_WAKEUP_F);
			}
			else{
				DEBUG_PRINT_NPCSNAKE("Wakeup B\n");
				ChangeNewPad(npc,PAD_DAMAGE_WAKEUP_B);
			}
		}
		MOTION_CANCELABLE_OFF(npc);
		return ;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}


/* Zzzzzzzzzzz */
static void ActSleepDown( NPCWORK *npc, int time )
{
	NPCACT	*act ;

	SET_SCN_SLEEP();

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}
	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;

		NPC_ChangeTargetSize(npc,NPC_TARGET_SIZE_DOWN);

		NPC_SetFaintCount( npc, act->sleep_max, NPC_FAINT_ZZZ ) ;
		NPC_CallHeadMark( npc, HMK2_TYPE_PIYO_A  ) ;
		NPC_SetActMode( npc, NPC_ActFaint ) ;

		act->pad=act->set_pad=PAD_NONE;

		ZZZSOUND_ON(ACCESS_WORK(npc));

		return ;
	}
}


#define ChangeHazardSphere(_ctrl,_r,_s) \
do{ \
    (_ctrl)->r_sphere=(_r); \
    (_ctrl)->s_sphere=(_s); \
}while(0)

#define R_SHPERE_DEATH		650
#define S_SHPERE_DEATH		700

static void ActGameover(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	NPC_ActStatus(act,NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_DEATH);
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	ChangeHazardSphere(npc->ctrl,R_SHPERE_DEATH,S_SHPERE_DEATH);

	ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_GAMEOVER;
	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

	if ( time == 0 ) {
		NPC_CallHeadMark(npc,HMK2_TYPE_KILL);
		NPC_SetFaintCount(npc,0,0);

		NPCSnake_CallGameoverProc(ACCESS_WORK(npc));

		GM_GameOverProcStart(&(ACCESS_WORK(npc)->actor));

		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);

		DEBUG_PRINT_NPCSNAKE("Gameover\n");
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if(GM_CheckObject_IsEnd(npc->body,0)){
		DEBUG_PRINT_NPCSNAKE("Gameover End\n");

		GM_GameOverProcEnd(&(ACCESS_WORK(npc)->actor));
		return;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActGameover2(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	NPC_ActStatus(act,NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF|NPC_ACT_STATUS_DEATH);
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	ACCESS_WORK(npc)->snake_status|=SNAKE_STATUS_GAMEOVER;
	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

	ChangeHazardSphere(npc->ctrl,R_SHPERE_DEATH,S_SHPERE_DEATH);

	if ( time == 0 ) {
		NPC_CallHeadMark(npc,HMK2_TYPE_KILL);
		NPC_SetFaintCount(npc,0,0);

		NPCSnake_CallGameoverProc(ACCESS_WORK(npc));

		GM_GameOverProcStart(&(ACCESS_WORK(npc)->actor));

		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		// NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);

		if(!(ACCESS_WORK(npc)->snake_status2 & SNAKE_STATUS2_CALLED_GAMEOVER_VOICE)){
			if(ACCESS_WORK(npc)->snake_status2 & SNAKE_STATUS2_DAMAGED_BY_RAIDEN){
				GM_KillCount++;
			}
		}

		DEBUG_PRINT_NPCSNAKE("Gameover2\n");
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if(GM_CheckObject_IsEnd(npc->body,0)){
		GM_GameOverProcEnd(&(ACCESS_WORK(npc)->actor));
		return;
	}

	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}


/* 狙撃スネークのダメージ */
static void ActMotion_CrouchDamage(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	SET_SCN_DAMAGE();

	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);
		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("CrouchingDamage\n");
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_CROUCHING_WPSG);

		NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);


		if(NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))==NPCSNAKE_ACT_W32A_SLEEP){
			NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W32A_AFTER_SLEEP);
		}
		else{
			/* 赤！ */
			NPC_CallHeadMark(npc,HMK2_TYPE_RED_AT);
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */
			ACCESS_WORK(npc)->headmark_count++;

			NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W32A_PRE_COUNTER);
		}

		ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_CYPHER_IN_SIGHT;

		LOCKON_ENEMY(npc);
		ACCESS_WORK(npc)->homing=NULL;
		fpu_CopyVector(&(ACCESS_WORK(npc)->homing_pos),
					   (FVECTOR *)&(GM_PlayerBody->objs->objs[HUMAN21_MUNE].world.m[3][0]));
		ACCESS_WORK(npc)->homing_p=&(ACCESS_WORK(npc)->homing_pos);

		ACCESS_WORK(npc)->gcounter=0;

		return ;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}

static void ActMotion_CrouchSleep(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF ) ;
	NPC_ActStatus(act,NPC_ACT_STATUS_EYE_CLOSE);

	ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;

#if 0
	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);
		MOTION_CANCELABLE_OFF(npc);

		DEBUG_PRINT_NPCSNAKE("CrouchingSleep\n");
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if ( time>FRAME_W32A_SLEEP_MOTION ) {
		MOTION_CANCELABLE_ON(npc);
		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		act->act_end = 1 ;
		ChangeNewPad(npc,PAD_CROUCHING_WPSG);

		NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

		ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_CYPHER_IN_SIGHT;

		LOCKON_ENEMY(npc);
		ACCESS_WORK(npc)->homing=NULL;
		fpu_CopyVector(&(ACCESS_WORK(npc)->homing_pos),
					   (FVECTOR *)&(GM_PlayerBody->objs->objs[HUMAN21_MUNE].world.m[3][0]));
		ACCESS_WORK(npc)->homing_p=&(ACCESS_WORK(npc)->homing_pos);

		ACCESS_WORK(npc)->gcounter=0;

		return ;
	}
	// if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
#else
	MOTION_CANCELABLE_ON(npc);
	ACTION_FINISHED(npc);
	SHOOT_ACTION_FINISHED(npc);
	act->act_end = 1 ;

	ChangeNewPad(npc,PAD_CROUCHING_WPSG);

	NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

	ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_CYPHER_IN_SIGHT;

	LOCKON_ENEMY(npc);
	ACCESS_WORK(npc)->homing=NULL;
	fpu_CopyVector(&(ACCESS_WORK(npc)->homing_pos),
				   (FVECTOR *)&(GM_PlayerBody->objs->objs[HUMAN21_MUNE].world.m[3][0]));
	ACCESS_WORK(npc)->homing_p=&(ACCESS_WORK(npc)->homing_pos);

	ACCESS_WORK(npc)->gcounter=0;
#endif

}


/* ローリングアクション */
static void ActMotion_Rolling(NPCWORK *npc,int time)
{
	NPCACT *act ;
	FVECTOR force={ 0.0f,0.0f,0.0f,1.0f, };

	act = &npc->action ;

	NPC_ActStatus( act,NPC_ACT_STATUS_TRG_OFF ) ;

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif


	GM_SetPowerTarget(&(ACCESS_WORK(npc)->attack[6]),&(ACCESS_WORK(npc)->pow[6]),POWER_ONCE,
					  VS_ROLLING_VITAL_POINT,VS_ROLLING_FAINT_POINT,VS_ROLLING_ATTACK_POINT,&force);

	GM_PutTarget(&(ACCESS_WORK(npc)->attack[6]));
	GM_MoveTarget2(&(ACCESS_WORK(npc)->attack[6]),
				   &(npc->body->objs->objs[HUMAN21_KOSHI].world));


	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);

		if(HAS_USP_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_TUMBLE_END_WUSP);
		else if(HAS_FMS_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_TUMBLE_END_WFMS);
		else ChangeNewPad(npc,PAD_TUMBLE_END);

		MOTION_CANCELABLE_OFF(npc);
	}

	if(act->dir>=0) npc->ctrl->turn.vy=act->dir;
}

static void ActMotion_RollingEnd(NPCWORK *npc,int time)
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus( act,NPC_ACT_STATUS_TRG_OFF ) ;

	if ( time == 0 ) {
		npc->ctrl->turn.vx = 0 ;
		npc->ctrl->turn.vz = 0 ;

		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
		// NPC_MotionLayerOverOff(npc,1);
		// NPC_MotionLayerOverOff(npc,2);

		MOTION_CANCELABLE_OFF(npc);
	}

#if 0
	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( npc->CheckPad( npc ) ) return ;
	}
#endif

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		MOTION_CANCELABLE_ON(npc);

		ACTION_FINISHED(npc);
		SHOOT_ACTION_FINISHED(npc);
		act->act_end = 1 ;

		if(HAS_USP_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WUSP);
		else if(HAS_FMS_MOTION_FLAG(npc)) ChangeNewPad(npc,PAD_STAND_WFMS);
		else ChangeNewPad(npc,PAD_STAND);
	}

	// if(act->dir>=0) npc->ctrl->turn.vy=act->dir;
}


/* -------------------------------------------------------------------- */
/* Sub Motions                                                          */
/* -------------------------------------------------------------------- */

static void ChangeSubMotion(NPCWORK *npc,int pad,void (*act)(NPCWORK *npc,int time))
{
	ACCESS_WORK(npc)->sub_pad=pad;
	ACCESS_WORK(npc)->sub_pad_time=0;
	ACCESS_WORK(npc)->SubMotionAct=act;
}

static void SubMotion_None(NPCWORK *npc,int time)
{
	if(ACCESS_WORK(npc)->sub_pad==PAD_NONE){
		// DEBUG_PRINT_NPCSNAKE("SubMotion None\n");

		if(time==0){
			NPC_MotionLayerOverOff(npc,1);
			MT_SetMotionInterp(npc->body->m_ctrl,ACT_INTERP_DEF,MOTION_MASK_UPPER);
			NPC_MotionLayerOverOff(npc,2);

			DEBUG_PRINT_NPCSNAKE("SubMotion None Init\n");
		}
		SHOOT_ACTION_FINISHED(npc);
	}
}

static void SubMotion_ShootUSP(NPCWORK *npc,int time)
{
	NPCACT *act;
	int StartSubMotion_BringUSP(Work *work);

	act=&npc->action;

	switch(time){
	case 0:
		HAS_USP_MOTION(npc);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->fms);

		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_SHOOT_USP,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		LOCKON_ENEMY(npc);

		DEBUG_PRINT_NPCSNAKE("++ShootUSP %f\n",ACCESS_WORK(npc)->homing_dis);
		break;

	case 1:
		/* 弾丸発射 */
		ShootBullet(npc);
		break;
	}

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);

	if(time>=FRAME_FINISH_TO_SHOOT_USP+1){
		// ChangeSubMotion(npc,PAD_NONE,SubMotion_None);
		StartSubMotion_BringUSP(ACCESS_WORK(npc));
	}
	if(time>=FRAME_FINISH_TO_SHOOT_USP){
		SHOOT_ACTION_FINISHED(npc);
		UNLOCK_ENEMY(npc);
	}
}

static void SubMotion_BeginUSP(NPCWORK *npc,int time)
{
	NPCACT *act;

	act=&npc->action;

	if(time==0){
		HAS_USP_MOTION(npc);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->fms);

		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_START_SHOOT_USP,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		DEBUG_PRINT_NPCSNAKE("++BeginUSP\n");

		LOCKON_ENEMY(npc);
	}
	if(time>=FRAME_READY_TO_FIRE_USP){
		ChangeSubMotion(npc,PAD_SHOOT_USP,SubMotion_ShootUSP);
	}

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);
}

static void SubMotion_BringUSP(NPCWORK *npc,int time)
{
	NPCACT *act;

	act=&npc->action;

	if(time==0){
		HAS_USP_MOTION(npc);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->fms);

		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_START_SHOOT_USP,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		DEBUG_PRINT_NPCSNAKE("++BringUSP\n");
	}
	if(time>=FINISH_CAREFUL){
		ChangeSubMotion(npc,PAD_NONE,SubMotion_None);
	}
}

static void SubMotion_ShootFMS(NPCWORK *npc,int time)
{
	NPCACT *act;
	int StartSubMotion_BringFMS(Work *work);

	act=&npc->action;

	switch(time){
	case 0:
		HAS_FMS_MOTION(npc);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->fms);

		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_SHOOT_FMS,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		LOCKON_ENEMY(npc);

		DEBUG_PRINT_NPCSNAKE("++ShootFMS %f\n",ACCESS_WORK(npc)->homing_dis);
		break;

	case 1:
		/* 弾丸発射 */
		ShootBullet(npc);
		break;
	}

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);

	if(time>=FRAME_FINISH_TO_SHOOT_FMS+1){
		// ChangeSubMotion(npc,PAD_NONE,SubMotion_None);
		StartSubMotion_BringFMS(ACCESS_WORK(npc));
	}
	if(time>=FRAME_FINISH_TO_SHOOT_FMS){
		SHOOT_ACTION_FINISHED(npc);
		UNLOCK_ENEMY(npc);
	}
}

static void SubMotion_BeginFMS(NPCWORK *npc,int time)
{
	NPCACT *act;

	act=&npc->action;

	if(time==0){
		HAS_FMS_MOTION(npc);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->fms);

		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_START_SHOOT_FMS,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		DEBUG_PRINT_NPCSNAKE("++BeginFMS\n");

		LOCKON_ENEMY(npc);
	}
	if(time>=FRAME_READY_TO_FIRE_FMS){
		ChangeSubMotion(npc,PAD_SHOOT_FMS,SubMotion_ShootFMS);
	}

	if(LOCKON_ENEMY_FLAG(npc)) CalcAdjust0(npc);
}

static void SubMotion_BringFMS(NPCWORK *npc,int time)
{
	NPCACT *act;

	act=&npc->action;

	if(time==0){
		HAS_FMS_MOTION(npc);
		NPCSnakeInvisibleWeapon(ACCESS_WORK(npc)->usp);
		NPCSnakeVisibleWeapon(ACCESS_WORK(npc)->fms);

		NPC_SetActMotionEX(npc,1,act->set_mar,PAD_START_SHOOT_FMS,
						   MOTION_MASK_UPPER,ACT_INTERP_DEF,(float)TIME_BASE);
		NPC_MotionLayerOverOff(npc,2);

		DEBUG_PRINT_NPCSNAKE("++BringFMS\n");
	}
	if(time>=FINISH_CAREFUL){
		ChangeSubMotion(npc,PAD_NONE,SubMotion_None);
	}
}

void ActSubMotion(Work *work)
{
	int time;
	time=work->sub_pad_time;
	work->sub_pad_time++;
	(*(work->SubMotionAct))(&(work->npc),time);
}

int ActSubMotionReset(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_NONE,SubMotion_None);
	work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int ActSubMotionLock(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_LOCK,SubMotion_None);
	work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int ActSubMotionUnlock(Work *work)
{
	if(work->sub_pad!=PAD_LOCK) return 0;

	work->sub_pad=PAD_NONE;
	work->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int StartSubMotion_BeginUSP(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		// DEBUG_PRINT_NPCSNAKE("------ 5 ------\n");
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_START_SHOOT_USP,SubMotion_BeginUSP);
	work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int StartSubMotion_BringUSP(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		// DEBUG_PRINT_NPCSNAKE("------ 5 ------\n");
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_BRINGING_USP,SubMotion_BringUSP);
	// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int StartSubMotion_ShootUSP(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		// DEBUG_PRINT_NPCSNAKE("------ 6 ------\n");
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_SHOOT_USP,SubMotion_ShootUSP);
	work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int StartSubMotion_BeginFMS(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_START_SHOOT_FMS,SubMotion_BeginFMS);
	work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int StartSubMotion_BringFMS(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_BRINGING_FMS,SubMotion_BringFMS);
	// work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

int StartSubMotion_ShootFMS(Work *work)
{
	if(!(work->snake_status & SNAKE_STATUS_SHOOT_ACTION_FINISHED)){
		return 0;
	}

	ChangeSubMotion(&(work->npc),PAD_SHOOT_FMS,SubMotion_ShootFMS);
	work->snake_status&=~SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	return 1;
}

