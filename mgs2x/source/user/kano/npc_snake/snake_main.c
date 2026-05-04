//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_main.c
		NPCスネークメインルーチン(w14a,w17a,w32a,w44a,w45a)

	2001/03/15 K.Kano
	$Id: snake_main.c,v 1.1.1.3 2002/11/19 11:43:22 Yoshizawa1 Exp $
 */


#include "npc_snake.h"

#include "../attachment/attachments.h"

#include "support.c"

#define N_NPCSNAKE_SUPPORT_POINTS \
	(sizeof(init_support_points)/sizeof(init_support_points[0]))


/* しょっぱなからマシンガンを使うレベル
   ゲームのレベルのdefineとは無関係にした。*/
#define FAMAS_GAME_LEVEL			(30-1)


/* 敵がこれ以下の距離にいた場合は、INDEPENDからSEARCH_ENEMYに変更する */
#define INDEPEND_TO_SEARCHENEMY_DIS		CVM2N(5.0f)


#define USP_SHOOT_LIMIT(work)		(npcsnake_usp_shoot_limit[(work)->support_level])
#define FMS_SHOOT_LIMIT(work)		(npcsnake_fms_shoot_limit[(work)->support_level])
#define PSG_SHOOT_LIMIT(work)		(npcsnake_psg1_shoot_limit[(work)->support_level])


const int npcsnake_usp_shoot_limit[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
#if 0
	2,
	2,
	3,
	3,
	4,
	5,
	6,
	6,
#else
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
#endif
	/* STRONG */
};

const int npcsnake_fms_shoot_limit[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
#if 0
	5,
	5,
	6,
	6,
	7,
	8,
	9,
	10,
#else
	10,
	12,
	15,
	18,
	21,
	24,
	27,
	30,
#endif
	/* STRONG */
};

const int npcsnake_psg1_shoot_limit[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
	2,
	2,
	3,
	3,
	4,
	4,
	5,
	5,
	/* STRONG */
};

/* 音声のコントロール */
void NPCSNAKE_VOICE_CALL(void *_work, int id)
{
   Work *work = (Work *)_work;
   int *voice_le;
   if((work)->voice_handle>0) return;
   voice_le=(int *)GetLocalResource(NPCSNAKE_VOICE_RESOURCE,id);
   DEBUG_PRINT_NPCSNAKE("Voice = 0x%08x\n",voice_le);
   if(voice_le==NULL) return;
   (work)->snake_status2|=SNAKE_STATUS2_VOICE;
   (work)->voice_num=GCL_GetLong( voice_le );
}

void NPCSNAKE_VOICE_RNDCALL(void *_work,int *ids, int size)
{
   Work *work = (Work *)_work;
   int _i;
   if((work)->voice_handle>0)
   {
      return;
   }
   if(size>1)
   {
      _i=irnd() % (size-1);
      _i=(work)->voice_index+_i+1;
      if(_i>=size) _i-=size;
   }
   else
   {
      if(irnd() & 0x10000) return;
      _i=0;
   }
   (work)->voice_index=_i;
   NPCSNAKE_VOICE_CALL(_work, (ids)[_i]);
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


void SetModeAndFunc(Work *work,int mode)
{
	work->mode=mode;

	switch(mode){
	case NPCSNAKE_MODE_SEARCHENEMY:
		/* 周囲索敵モード */
		work->GetSituation=GetSituation_SEARCHENEMY;
		work->DecideAction=DecideAction_SEARCHENEMY;
		break;
	case NPCSNAKE_MODE_INDEPEND:
		/* 自己判断モード */
		work->GetSituation=GetSituation_INDEPEND;
		work->DecideAction=DecideAction_INDEPEND;
		break;
	case NPCSNAKE_MODE_FOLLOW_RAIDEN:
		/* ライデン追跡モード */
		work->GetSituation=GetSituation_FOLLOW_RAIDEN;
		work->DecideAction=DecideAction_FOLLOW_RAIDEN;
		break;
	case NPCSNAKE_MODE_BEHIND_RAIDEN:
		/* ライデン後方援護モード */
		work->GetSituation=GetSituation_BEHIND_RAIDEN;
		work->DecideAction=DecideAction_BEHIND_RAIDEN;
		work->snake_status&=~SNAKE_STATUS_DAMAGED_FOR_BEHIND;
		break;
	case NPCSNAKE_MODE_PICKUP_RAIDEN:
		/* ライデンお迎えモード */
		work->GetSituation=GetSituation_PICKUP_RAIDEN;
		work->DecideAction=DecideAction_PICKUP_RAIDEN;
		break;

		/* --- w45a --- */
	case NPCSNAKE_MODE_GURD_RAIDEN:
		work->GetSituation=GetSituation_W45A;
		work->DecideAction=DecideAction_GURD_RAIDEN;
		break;
	case NPCSNAKE_MODE_ATTACK_ENEMY:
		work->GetSituation=GetSituation_W45A;
		work->DecideAction=DecideAction_ATTACK_ENEMY;
		break;

	case NPCSNAKE_MODE_BETWEEN_STAGES:
		/* --- ステージ間移動 --- */
		work->GetSituation=GetSituation_BETWEEN_STAGES;
		work->DecideAction=DecideAction_BETWEEN_STAGES;
		break;

	case NPCSNAKE_MODE_W14A:
		/* --- w14a --- */
		work->GetSituation=GetSituation_W14A;
		work->DecideAction=DecideAction_W14A;
		break;

	case NPCSNAKE_MODE_W17A:
		/* --- w17a --- */
		work->GetSituation=GetSituation_W17A;
		work->DecideAction=DecideAction_W17A;
		break;

	case NPCSNAKE_MODE_W32A:
		/* --- w32a --- */
		work->GetSituation=GetSituation_W32A;
		work->DecideAction=DecideAction_W32A;
		break;

	case NPCSNAKE_MODE_W43A:
		work->GetSituation=GetSituation_BETWEEN_STAGES;
		work->DecideAction=DecideAction_W43A;
		break;

	case NPCSNAKE_MODE_DUMMY:
		work->GetSituation=GetSituation_DUMMY;
		work->DecideAction=DecideAction_DUMMY;
		break;


		/* 2001/9/18  K.Kano
		   w44aでの最後の山場でのスネークの動きを改善
		   新モードを追加 */
	case NPCSNAKE_MODE_GURD_RAIDEN_W44A:
		work->GetSituation=GetSituation_W44AFinal;
		work->DecideAction=DecideAction_GURD_RAIDEN_W44A;
		break;
	case NPCSNAKE_MODE_ATTACK_ENEMY_W44A:
		work->GetSituation=GetSituation_W44AFinal;
		work->DecideAction=DecideAction_ATTACK_ENEMY;
		break;


	default:
		ASSERT(0);
		work->GetSituation=NULL;
		work->DecideAction=NULL;
		break;
	}
}

static void ModeChange(Work *work)
{
	static const int voice_searchenemy[]={
		// ライデンが遅れ気味
		VOICE_INDEX_RAIDEN_LATE0,
		// スネーク　　「何をしてる？」
		VOICE_INDEX_RAIDEN_LATE1,
		// スネーク　　「早く来い！」
		VOICE_INDEX_RAIDEN_LATE2,
		// スネーク　　「おいていくぞ！」
	};

	switch(work->mode){
	case NPCSNAKE_MODE_SEARCHENEMY:
		/* 索敵モード */
		if(work->snake_status2 & SNAKE_STATUS2_BE_CAPTURED_MODE) break;

		/* 2001/9/18  K.Kano
		   w44aでの最後の山場でのスネークの動きを改善
		   新モードを追加 */
		if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
			DEBUG_PRINT_NPCSNAKE("W44A Final Start\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_ATTACK_ENEMY_W44A);
			break;
		}

		if(!(work->snake_status & SNAKE_STATUS_OVER_RAIDEN)){
			DEBUG_PRINT_NPCSNAKE("NORMAL\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_INDEPEND);
			break;
		}
		if(work->raiden_status & NPCSNAKE_RAIDEN_MANY_ENEMYS){
			DEBUG_PRINT_NPCSNAKE("PICKUP\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_PICKUP_RAIDEN);

			work->snake_status2&=~SNAKE_STATUS2_VOICE_ENEMY_ARROUND2;
			break;
		}
		if((work->snake_status & SNAKE_STATUS_WAIT_RAIDEN_OVERTIME) &&
		   ((work->raiden_status & NPCSNAKE_RAIDEN_ENEMY_BEHIND) ||
			(work->raiden_status & NPCSNAKE_RAIDEN_2ENEMYS) ||
			(work->raiden_status & NPCSNAKE_RAIDEN_MANY_ENEMYS))){

			DEBUG_PRINT_NPCSNAKE("PICKUP\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_PICKUP_RAIDEN);

			work->snake_status2&=~SNAKE_STATUS2_VOICE_ENEMY_ARROUND2;
			break;
		}
		if((work->snake_status & SNAKE_STATUS_WAIT_RAIDEN_OVERTIME) &&
		   !(work->snake_status2 & SNAKE_STATUS2_VOICE_SEARCHENEMY)){

			NPCSNAKE_VOICE_RNDCALL(work,voice_searchenemy,
								   sizeof(voice_searchenemy)/sizeof(voice_searchenemy[0]));
			work->snake_status2|=SNAKE_STATUS2_VOICE_SEARCHENEMY;
		}
		break;

	case NPCSNAKE_MODE_INDEPEND:
		/* 自己判断モード */
		if(work->snake_status2 & SNAKE_STATUS2_BE_CAPTURED_MODE) break;

		/* 2001/9/18  K.Kano
		   w44aでの最後の山場でのスネークの動きを改善
		   新モードを追加 */
		if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
			DEBUG_PRINT_NPCSNAKE("W44A Final Start\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_ATTACK_ENEMY_W44A);
			break;
		}

		if((work->raiden_status & NPCSNAKE_RAIDEN_2ENEMYS) &&
		   (work->snake_status & SNAKE_STATUS_OVER_RAIDEN)){

			DEBUG_PRINT_NPCSNAKE("PICKUP\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_PICKUP_RAIDEN);

			work->snake_status2&=~SNAKE_STATUS2_VOICE_ENEMY_ARROUND2;
			break;
		}
		if(work->snake_status & SNAKE_STATUS_TOO_OVER_RAIDEN){
			DEBUG_PRINT_NPCSNAKE("SEARCH ENEMY\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_SEARCHENEMY);
			break;
		}
		if(work->raiden_status & NPCSNAKE_RAIDEN_MOVING){
			if(work->snake_status & SNAKE_STATUS_BEHIND_RAIDEN_L2){

				DEBUG_PRINT_NPCSNAKE("FOLLOW RAIDEN\n");
				SetModeAndFunc(work,NPCSNAKE_MODE_FOLLOW_RAIDEN);
				break;
			}
		}
		if(work->snake_status & SNAKE_STATUS_BEHIND_RAIDEN_L3){

			DEBUG_PRINT_NPCSNAKE("FOLLOW RAIDEN\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_FOLLOW_RAIDEN);
			break;
		}
		if(work->homing!=NULL){
			if(work->homing_dis<INDEPEND_TO_SEARCHENEMY_DIS){
				DEBUG_PRINT_NPCSNAKE("SEARCH ENEMY\n");
				SetModeAndFunc(work,NPCSNAKE_MODE_SEARCHENEMY);
				break;
			}
		}
		break;

	case NPCSNAKE_MODE_FOLLOW_RAIDEN:
		/* ライデン追跡モード */
		if(!(work->snake_status & SNAKE_STATUS_BEHIND_RAIDEN_L1)){

			DEBUG_PRINT_NPCSNAKE("BEHIND RAIDEN\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_BEHIND_RAIDEN);

			// work->snake_status2&=~SNAKE_STATUS2_VOICE_ENEMY_ARROUND;
			work->snake_status2&=~SNAKE_STATUS2_VOICE_SEARCHENEMY;
			break;
		}
		break;

	case NPCSNAKE_MODE_BEHIND_RAIDEN:
		/* ライデン後方援護モード */

		/* 2001/9/18  K.Kano
		   w44aでの最後の山場でのスネークの動きを改善
		   新モードを追加 */
		if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
			DEBUG_PRINT_NPCSNAKE("W44A Final Start\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_ATTACK_ENEMY_W44A);
			break;
		}

		if(work->snake_status & SNAKE_STATUS_BEHIND_RAIDEN_L3){
			DEBUG_PRINT_NPCSNAKE("FOLLOW RAIDEN\n");
			SetModeAndFunc(work,NPCSNAKE_MODE_FOLLOW_RAIDEN);
			break;
		}
		if(!(work->raiden_status & NPCSNAKE_RAIDEN_MANY_ENEMYS)){
			if(work->behind_raiden_count>work->behind_to_independ_interval){
				if(work->snake_status & SNAKE_STATUS_OVER_RAIDEN0){

					DEBUG_PRINT_NPCSNAKE("NORMAL\n");
					SetModeAndFunc(work,NPCSNAKE_MODE_INDEPEND);

					if(!(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT)){
						work->snake_status2&=~SNAKE_STATUS2_VOICE_INDEPEND;
					}
					else{
						work->snake_status2|=SNAKE_STATUS2_VOICE_INDEPEND;
					}
					break;
				}
				if(work->homing==NULL){
					DEBUG_PRINT_NPCSNAKE("NORMAL\n");
					SetModeAndFunc(work,NPCSNAKE_MODE_INDEPEND);

					if(!(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT)){
						work->snake_status2&=~SNAKE_STATUS2_VOICE_INDEPEND;
					}
					else{
						work->snake_status2|=SNAKE_STATUS2_VOICE_INDEPEND;
					}
					break;
				}
			}
		}
		break;

	case NPCSNAKE_MODE_PICKUP_RAIDEN:
		/* ライデンお迎えモード */
		work->snake_status2&=~SNAKE_STATUS2_VOICE_ENEMY_ARROUND;

		if(!(work->snake_status & SNAKE_STATUS_OVER_RAIDEN)){
			DEBUG_PRINT_NPCSNAKE("BEHIND RAIDEN\n");

			SetModeAndFunc(work,NPCSNAKE_MODE_BEHIND_RAIDEN);

			work->snake_status2&=~SNAKE_STATUS2_VOICE_SEARCHENEMY;
			break;
		}
		break;


		/* --- w45a --- */
	case NPCSNAKE_MODE_GURD_RAIDEN:
		if(work->homing!=NULL &&
		   !(work->raiden_status & NPCSNAKE_RAIDEN_MANY_ENEMYS) &&
		   !(work->raiden_status & NPCSNAKE_RAIDEN_LIFE_DANGER)){

			SetModeAndFunc(work,NPCSNAKE_MODE_ATTACK_ENEMY);
			break;
		}
		break;

	case NPCSNAKE_MODE_ATTACK_ENEMY:
		if(((work->homing==NULL ||
			 work->homing_dis>CVM2N(10.0f)) &&
			(work->raiden_status & NPCSNAKE_RAIDEN_MANY_ENEMYS)) ||
		   (work->raiden_status & NPCSNAKE_RAIDEN_LIFE_DANGER)){

			SetModeAndFunc(work,NPCSNAKE_MODE_GURD_RAIDEN);
			break;
		}
		break;

	case NPCSNAKE_MODE_GURD_RAIDEN_W44A:
		if((work->homing!=NULL ||
			(work->raiden_status & NPCSNAKE_RAIDEN_ENEMY_EXIST)) &&
		   !(work->raiden_status & NPCSNAKE_RAIDEN_MANY_ENEMYS) &&
		   !(work->raiden_status & NPCSNAKE_RAIDEN_LIFE_DANGER)){

			SetModeAndFunc(work,NPCSNAKE_MODE_ATTACK_ENEMY_W44A);
			break;
		}
		break;

	case NPCSNAKE_MODE_ATTACK_ENEMY_W44A:
		if((work->raiden_status & NPCSNAKE_RAIDEN_MANY_ENEMYS) ||
		   (work->raiden_status & NPCSNAKE_RAIDEN_LIFE_DANGER)){

			SetModeAndFunc(work,NPCSNAKE_MODE_GURD_RAIDEN_W44A);
			break;
		}
		break;

		/* --- ステージ間移動 --- */
	case NPCSNAKE_MODE_BETWEEN_STAGES:
		break;

		/* --- w14a --- */
	case NPCSNAKE_MODE_W14A:
		break;

		/* --- w17a --- */
	case NPCSNAKE_MODE_W17A:
		break;

		/* --- w32a --- */
	case NPCSNAKE_MODE_W32A:
		break;

	case NPCSNAKE_MODE_DUMMY:
		break;

		/* --- w43a --- */
	case NPCSNAKE_MODE_W43A:
		break;
	}
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


#ifdef DEBUG_MODE

static int npcsnake_debug_flag;
int npcsnake_nodamage_flag;
int npcsnake_noattack_flag;
int npcsnake_slevel;
int npcsnake_use_m4a;


static char * dbg_menu0_tems[]  =  { "VISIBLE", "INVISIBLE" } ;
static int    dbg_menu0_values[] = { 1, 0 } ;

static char * dbg_menu1_tems[]  =  		{ "ON", "USE MOTION", "OFF" } ;
static int    dbg_menu1_values[] = 		{ 1,2,0 };

static char * dbg_menu2_tems[]  =  		{ "ON", "PUNCH ONLY", "OFF" } ;
static int    dbg_menu2_values[] = 		{ 1,2,0 };

static char * dbg_menu3_tems[]  =  		{ "ON","DEFAULT" } ;
static int    dbg_menu3_values[] = 		{ 1,0 } ;


static char * dbg_menu4_tems[]  =  		{ "0/WEEK--", "1/WEEK-", "2/WEEK",
 					 "3/STANDARD", "4/STANDARD+",
					 "5/STRONG", "6/STRONG+", "7/STRONG++",
					 "8/W32AWEEK--", "9/W32AWEEK-", "10/W32AWEEK",
					 "11/W32ASTANDARD", "12/W32ASTANDARD+",
					 "13/W32ASTRONG", "14/W32ASTRONG+", "15/W32ASTRONG++", } ;
static int    dbg_menu4_values[] =		{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, } ;
static GM_DEBUG_MENU debug_menu[] = 
{
	{
		NULL,
		"NPCSNK",
		"STATUS",
		dbg_menu0_tems,
		dbg_menu0_values,
		(int*)&npcsnake_debug_flag,	/*intの変数へのポインタ。代入される。*/
		0x00000001,
		NULL, /*func*/
		0, /*strid*/
		0, /*types */
		2,
		0,
	},
	{
		NULL,
		"NPCSNK",
		"MUTEKI",
		dbg_menu1_tems,
		dbg_menu1_values,
		(int*)&npcsnake_nodamage_flag,	/* intの変数へのポインタ。代入される。*/
		0x00000003,
		NULL, /*func*/
		0, /*strid*/
		0, /*types */
		3,
		0,0
	},
	{
		NULL,
		"NPCSNK",
		"ATTACK",
		dbg_menu2_tems,
		dbg_menu2_values,
		(int*)&npcsnake_noattack_flag,	/* intの変数へのポインタ。代入される。*/
		0x00000003,
		NULL, /*func*/
		0, /*strid*/
		0, /*types */
		3,
		0,0
	},
	{
		NULL,
		"NPCSNK",
		"USE M4A1",
		dbg_menu3_tems,
		dbg_menu3_values,
		(int*)&npcsnake_use_m4a,	/* intの変数へのポインタ。代入される。*/
		0x00000001,
		NULL, /*func*/
		0, /*strid*/
		0, /*types */
		2,
		0,0
	},
	{
		NULL,
		"NPCSNK",
		"S.LEVEL",
		dbg_menu4_tems,
		dbg_menu4_values,
		(int*)&npcsnake_slevel,
		0x0000000f,
		NULL, /*func*/
		0, /*strid*/
		0, /*types */
		16,
		0,0
	},
};

void DebugDispNPCSnakeStatus(Work *work)
{
	static const char * const modename[]={
		"INDEPEND",
		"SEARCH ENEMY",
		"FOLLOW RAIDEN",
		"BEHIND RAIDEN",
		"PICKUP RAIDEN",

		"GURD RAIDEN",
		"ATTACK ENEMY",

		"MOVE STAGE",

		"W14A",

		"W17A",

		"W32A",

		"DUMMY",

		"W43A",

		/* 2001/9/18  K.Kano
		   w44aでの最後の山場でのスネークの動きを改善
		   新モードを追加 */
		"GURD RAIDEN W44A",
		"ATTACK ENEMY W44A",
	};
	static const char * const muteki[]={
		"OFF",
		"ON",
		"USE MOTION",
	};
	static const char * const attack[]={
		"OFF",
		"ON",
		"PUNCH ONLY",
	};

	DEBUG_Locate(50,336,MENU_MODE_NORMAL);
	DEBUG_Printf("Snake Mode               : %s\n",modename[work->mode]);
	DEBUG_Printf("Snake HIDE / SHOOT / ACT : %01x / %01x / %02x\n",
				 (work->action_num>>12) & 0x0f,
				 (work->action_num>>8) & 0x0f,
				 work->action_num & 0xff);
	DEBUG_Printf("Snake Status             : %08x\n",work->snake_status);
	DEBUG_Printf("Snake Muteki             : %s\n",muteki[npcsnake_nodamage_flag]);
	DEBUG_Printf("Snake Attack             : %s\n",attack[npcsnake_noattack_flag]);
	DEBUG_Printf("Snake Scenario Status    : %d\n",work->scenario_snake_status);

	work->support_level=npcsnake_slevel & (NPC_SNAKE_SUPPORT_LEVEL_MAX-1);
	work->support_level2=npcsnake_slevel;
}

#endif


#define CheckDamageByUserDefined				NPCSnake_CheckDamage
#define CheckDamageBETWEENSTAGESByUserDefined	NPCSnake_CheckDamageBETWEENSTAGES
#define CheckDamageW14AByUserDefined			NPCSnake_CheckDamageW14A
#define CheckDamageW17AByUserDefined			NPCSnake_CheckDamageW17A
#define CheckDamageW32AByUserDefined			NPCSnake_CheckDamageW32A


static void SetTargetCallback0(Work *work)
{
	NPCWORK		*npc;
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	int i;

	npc=&(work->npc);
	npctrg = &npc->target ;
	GM_SetTargetCallBack(npctrg->deftrg,NULL,NULL);

	def_child = npctrg->def_child ;
	for( i=0; i<npctrg->child_trg_num ; i++ ) {
	    GM_SetTargetCallBack(def_child,NULL,NULL);
		def_child ++ ;
	}
}

static void SetTargetCallback(Work *work)
{
	NPCWORK		*npc;
	NPCTARGET	*npctrg ;
	TARGET		*def_child ;
	int i;

	npc=&(work->npc);
	npctrg = &npc->target ;
	GM_SetTargetCallBack(npctrg->deftrg,NPCSnake_DefTargetCallback,(void *)work);

	def_child = npctrg->def_child ;
	for( i=0; i<npctrg->child_trg_num ; i++ ) {
	    GM_SetTargetCallBack(def_child,NPCSnake_DefTargetCallback,(void *)work);
		def_child ++ ;
	}
}

void NPCSnake_SetStage(Work *work,int stage)
{
	work->stage=stage;
	work->snake_status&=~SNAKE_STATUS_LASERSIGHT;

	switch(work->stage){
	case NPCSNAKE_STAGE_W45A: /* w45a */
		SetModeAndFunc(work,NPCSNAKE_MODE_GURD_RAIDEN);
		work->action_num=NPCSNAKE_ACT_MOVEAROUNDRAIDEN;

		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;

		DG_VisibleObjs(work->body.objs);
		break;

	case NPCSNAKE_STAGE_BETWEEN_STAGES: /* ステージ間移動 */
		SetModeAndFunc(work,NPCSNAKE_MODE_BETWEEN_STAGES);
		work->action_num=NPCSNAKE_ACT_FLWRAIDEN_ONLY;
		if(!(work->snake_status & (SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS))){
			work->snake_status|=SNAKE_STATUS_USE_FMS;
		}
		// work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		DG_VisibleObjs(work->body.objs);
		break;

	case NPCSNAKE_STAGE_W17A: /* w17a */
		GM_RadarResetFlag(&(work->rctrl),RADAR_VISIBLE|RADAR_SIGHT);

		SetModeAndFunc(work,NPCSNAKE_MODE_W17A);
		work->action_num=NPCSNAKE_ACT_W17A_0;
		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		DG_VisibleObjs(work->body.objs);
		break;

	case NPCSNAKE_STAGE_W14A: /* w14a */
		SetModeAndFunc(work,NPCSNAKE_MODE_W14A);
		work->action_num=NPCSNAKE_ACT_W14A_0;
		work->snake_status&=~SNAKE_STATUS_USE_USP;
		work->snake_status|=SNAKE_STATUS_USE_FMS;

		DG_VisibleObjs(work->body.objs);
		break;

	case NPCSNAKE_STAGE_W32A: /* w32a */
		GM_RadarResetFlag(&(work->rctrl),RADAR_SIGHT);
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		SetModeAndFunc(work,NPCSNAKE_MODE_W32A);
		work->action_num=NPCSNAKE_ACT_W32A_WAIT;
		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		DG_VisibleObjs(work->body.objs);
		break;

	case NPCSNAKE_STAGE_DUMMY:
		GM_RadarResetFlag(&(work->rctrl),RADAR_VISIBLE|RADAR_SIGHT);
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		SetModeAndFunc(work,NPCSNAKE_MODE_DUMMY);
		work->action_num=NPCSNAKE_ACT_DUMMY;
		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);

		DG_InvisibleObjs(work->body.objs);
		break;

	case NPCSNAKE_STAGE_W43A:
		SetModeAndFunc(work,NPCSNAKE_MODE_W43A);
		work->action_num=NPCSNAKE_ACT_W43A;
		work->snake_status&=~(SNAKE_STATUS_USE_USP|SNAKE_STATUS_USE_FMS);
		work->snake_status2|=SNAKE_STATUS2_SIGHT_CLOSE;

		DG_InvisibleObjs(work->body.objs);
		break;

	case NPCSNAKE_STAGE_W44A:
	default: /* w44a */
		SetModeAndFunc(work,NPCSNAKE_MODE_BEHIND_RAIDEN);
		work->action_num=NPCSNAKE_ACT_STAY;

		work->snake_status2&=~SNAKE_STATUS2_SIGHT_CLOSE;
		work->snake_status2|=SNAKE_STATUS2_VOICE_ENEMY_ARROUND;

		DG_VisibleObjs(work->body.objs);
		break;
	}

	/* ダメージの設定部の設定 */
	switch(work->stage){
	case NPCSNAKE_STAGE_W14A:
		NPC_SetCheckDamage(&work->npc,CheckDamageW14AByUserDefined);
		work->npc.target.deftrg->class|=TARGET_LOCKON;
		SetTargetCallback0(work);
		break;
	case NPCSNAKE_STAGE_W17A:
		NPC_SetCheckDamage(&work->npc,CheckDamageW17AByUserDefined);
		work->npc.target.deftrg->class|=TARGET_LOCKON;
		SetTargetCallback0(work);
		break;
	case NPCSNAKE_STAGE_DUMMY:
		NPC_SetCheckDamage(&work->npc,CheckDamageW17AByUserDefined);
		work->npc.target.deftrg->class&=~TARGET_LOCKON;
		SetTargetCallback0(work);
		break;
	case NPCSNAKE_STAGE_W32A:
		NPC_SetCheckDamage(&work->npc,CheckDamageW32AByUserDefined);
		work->npc.target.deftrg->class|=TARGET_LOCKON;
		SetTargetCallback0(work);
		break;
	case NPCSNAKE_STAGE_BETWEEN_STAGES:
	case NPCSNAKE_STAGE_W43A:
		NPC_SetCheckDamage(&work->npc,CheckDamageBETWEENSTAGESByUserDefined);
		work->npc.target.deftrg->class|=TARGET_LOCKON;
		SetTargetCallback(work);
		break;
	default:
		NPC_SetCheckDamage(&work->npc,CheckDamageByUserDefined);
		work->npc.target.deftrg->class|=TARGET_LOCKON;
		SetTargetCallback(work);
		break;
	}
}

static void Damaged(void *w)
{
	Work *work=(Work *)w;
	NPCWORK *npc=&(work->npc);

#if 0
	switch(work->headmark_count){
	case 0:
		NPC_CallHeadMark(npc,HMK2_TYPE_WHT_AT);
		break;
	case 1:
		NPC_CallHeadMark(npc,HMK2_TYPE_YLW_AT);
		break;
	default:
		NPC_CallHeadMark(npc,HMK2_TYPE_RED_AT);
		break;
	}
#endif

	work->ret_flag|=NPCSNAKE_RETFLAG_DAMAGED_FROM_PLAYER;

	if(NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_W17A_5){
		/* 赤！ */
		NPC_CallHeadMark(npc,HMK2_TYPE_RED_AT);

		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_E_BIKKRI01); /* びっくり音 */

		work->headmark_count++;

		DEBUG_PRINT_NPCSNAKE("HeadMark = %d\n",work->headmark_count);

		NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W17A_5);
	}
}

static void InitMagazines(Work *work)
{
	static const FVECTOR mag1 = { 95.0f,85.0f,107.5f,0.0f } ;
	static const FVECTOR mag2 = { -95.0f,85.0f,107.5f,0.0f } ;
	static const FVECTOR mag3 = { 72.5f,37.5f,120.0f,0.0f } ;
	static const FVECTOR mag4 = { -72.5f,37.5f,120.0f,0.0f } ;

	static const ATTACHMENT_ARGUMENT3 attch[] = {
		{
			0x2bc559		/* GV_StrCode("sna_mag1") */ ,
			NULL,
			2,				/* 胸 */
			( FVECTOR * )&mag1,
			3,				/* frames */
		},
		{
			0x2bc55a		/* GV_StrCode("sna_mag2") */ ,
			NULL,
			2,				/* 胸 */
			( FVECTOR * )&mag2,
			3,				/* frames */
		},
		{
			0x2bc55b		/* GV_StrCode("sna_mag3") */,
			NULL,
			2,				/* 胸 */
			( FVECTOR * )&mag3,
			3,				/* frames */
		},
		{
			0x2bc55c		/* GV_StrCode("sna_mag4") */,
			NULL,
			2,				/* 胸 */
			( FVECTOR * )&mag4,
			3,				/* frames */
		},
	};

	work->magazines
		= NewAttachments_called(&(work->body),
								NULL, 0,
								NULL, 0,
								attch, sizeof(attch)/sizeof(attch[0]) ) ;
}

static void EndMagazine(Work *work)
{
	if(work->magazines!=NULL) GV_DestroyOtherActor(work->magazines);
}

void InitNPCSnake(Work *work)
{
	NPCSNAKE_SUPPORT_POINT *support_points=work->support_points;
	int n_support_points=work->n_support_points;
	int i;

	if(support_points==NULL){
		support_points=init_support_points;
		n_support_points=N_NPCSNAKE_SUPPORT_POINTS;
	}

	for(i=0;i<n_support_points;i++){
		int id;
		HZX_Pos2Zone(&(support_points[i].pos),
					 &id,&(support_points[i].zone));
		// printf("Zone = 0x%02x\n",support_points[i].zone);
	}

	work->movepos_count=0;
	work->hide_count=0;
	work->wait_raiden_count=0;
	work->sleep_count=0;
	work->nodamage_count=0;
	work->behind_raiden_count=0;

	work->i_infinity=0;

	work->snake_status=SNAKE_STATUS_CANCELABLE|
		SNAKE_STATUS_ACTION_FINISHED|SNAKE_STATUS_SHOOT_ACTION_FINISHED;

	work->raiden_status=0;
	work->pre_raiden_zone=GM_PlayerControl->addr;
	fpu_CopyVector(&(work->raiden_stay_pos),&GM_PlayerPosition);
	work->raiden_stay_count=0;

	work->support_pos_index=0;
	work->hide_pos_index=0;

	work->shoot_interval_count=0;

	work->headmark_count=0;

	work->w32a_request_count=0;

	work->breath_interval=0;
	work->breath_count=0;

	work->aftershoot_count=0;

	fpu_ClearVector(&(work->pre_adjust0));
	fpu_ClearVector(&(work->pre_adjust1));
	work->pre_adjust0.vw=1.0f;
	work->pre_adjust1.vw=1.0f;

	work->ret_flag=0;

	work->pre_homing=NULL;
	work->cmp_homing=NULL;

	work->eye_flush_count=0;
	work->pre_homing_part=0;

	work->zzz_sound_count=0;
	work->voice_num=0;
	work->voice_handle=0;
	work->voice_index=0;

	work->spray_count=0;
	work->roll_count=0;
	work->roll_counter=0;

	work->throwitem_count=3;

	fpu_ClearVector(&(work->damage_mov_v));

	work->book_count=0;
	work->book_interval=0;


	/* 思考処理初期化 */
	NPCSnake_SetStage(work,work->stage);

	switch(work->stage){
	case NPCSNAKE_STAGE_W14A:
		/* ダミー */
		work->ctrl.r_sphere=10;
		work->ctrl.s_sphere=10;

#if 1
		ChangeNewPad(&(work->npc),PAD_SITTING_AND_SLEEP_WM4A1);
#else
		NPC_SetModeFromPad(&(work->npc),ActLoopMotionWLockW14A,work->npc.base_mar,
						   PAD_SITTING_AND_SLEEP_WM4A1,PAD_SITTING_AND_SLEEP_WM4A1);
		NPC_ChangeTargetSize(&(work->npc),NPCSNAKE_TARGET_SITTING_W14A);
		SITTING_MOTION(&(work->npc));
#endif

#if 0
		work->deftrg.class|=TARGET_SEEK;
		for(i=0;i<CHILD_TARGET_NUM;i++){
			work->def_child[i].class|=TARGET_SEEK;
		}
#endif
		break;
	case NPCSNAKE_STAGE_W17A:
		{
			static OBJECT *body;
			static int unit;
			static int trigger;
			void *NewC_Box_ForNPC(CONTROL *ctrl,OBJECT *body,int unit,
								  void (*Damaged)(void *w),void *w);

			body=&(work->body);
			unit=HUMAN21_KOSHI;
			trigger=0;

			NPC_SetModeFromPad(&(work->npc),ActMotion_StandW17A,work->npc.base_mar,
							   PAD_BOX_WALK,PAD_BOX_STOP);

			GV_SetActorChild(work,NewC_Box_ForNPC(&(work->ctrl),body,unit,Damaged,(void *)work));
		}
		break;
	default:
		NPC_SetModeFromPad(&(work->npc),ActMotion_Stand,work->npc.base_mar,
						   PAD_STAND,PAD_STAND);
		break;
	}

	ActSubMotionReset(work);

	switch(work->stage){
	case NPCSNAKE_STAGE_W44A:
	case NPCSNAKE_STAGE_W45A:
		if(GM_GameLevel<FAMAS_GAME_LEVEL){
			work->snake_status|=SNAKE_STATUS_USE_FMS;
		}
		else{
			work->snake_status|=SNAKE_STATUS_USE_USP;
		}
	case NPCSNAKE_STAGE_W43A:
	case NPCSNAKE_STAGE_BETWEEN_STAGES:
		NewNPCSnakeLife(&(work->npc),work->life_max);
		break;
	}


#ifdef DEBUG_MODE
	GM_AddDebugMenu(&(debug_menu[0]));
	GM_AddDebugMenu(&(debug_menu[1]));
	GM_AddDebugMenu(&(debug_menu[2]));
	GM_AddDebugMenu(&(debug_menu[3]));
	GM_AddDebugMenu(&(debug_menu[4]));
	npcsnake_debug_flag=0;
	npcsnake_nodamage_flag=0;
	npcsnake_noattack_flag=1;
	npcsnake_slevel=work->support_level2;
	npcsnake_use_m4a=0;
#endif

	work->usp=NULL;
	work->fms=NULL;
	work->psg=NULL;
	work->magazines=NULL;

	switch(work->stage){
	case NPCSNAKE_STAGE_BETWEEN_STAGES:
	case NPCSNAKE_STAGE_W43A:
		work->usp=NPCSnakeInitUsp(work);
		work->fms=NPCSnakeInitFms(work);
		NPCSnakeInvisibleWeapon(work->usp);
		NPCSnakeInvisibleWeapon(work->fms);
		work->shoot_limit=1;
		InitMagazines(work);
		break;
	case NPCSNAKE_STAGE_W14A:
		work->fms=NPCSnakeInitFms(work);
		NPCSnakeVisibleWeapon(work->fms);
		work->shoot_limit=1;
		work->ctrl.skip_flag|=CTRL_SKIP_SEG_CHECK|CTRL_SKIP_ONLINE_CHECK|CTRL_SKIP_FLR_CHECK;
		break;
	case NPCSNAKE_STAGE_W17A:
		break;
	case NPCSNAKE_STAGE_W32A:
		work->psg=NPCSnakeInitPsg(work);
		NPCSnakeInvisibleWeapon(work->psg);
		if(work->shoot_limit==0) work->shoot_limit=PSG_SHOOT_LIMIT(work);
		work->ctrl.skip_flag|=CTRL_SKIP_SEG_CHECK|CTRL_SKIP_ONLINE_CHECK;
		break;
	case NPCSNAKE_STAGE_DUMMY:
		work->psg=NPCSnakeInitPsg(work);
		NPCSnakeInvisibleWeapon(work->psg);
		if(work->shoot_limit==0) work->shoot_limit=PSG_SHOOT_LIMIT(work);
		break;
	default:
		work->usp=NPCSnakeInitUsp(work);
		work->fms=NPCSnakeInitFms(work);
		NPCSnakeInvisibleWeapon(work->usp);
		NPCSnakeInvisibleWeapon(work->fms);
		InitMagazines(work);
		break;
	}

	if(work->shoot_limit==0) work->shoot_limit=USP_SHOOT_LIMIT(work);
	work->shoot_count=work->shoot_limit;
}

void NPCSnake_CallGameoverProc(Work *work)
{
	if(work->gameover_proc!=0){
		GCL_ARGS args;

		args.argc=0;
		args.argv=NULL;

		/* EndProc呼び出し */
		GCL_ExecProc(work->gameover_proc,&args);
	}
}

void ActNPCSnake(Work *work)
{
	void ExecAction(Work *work);

#ifdef DEBUG_MODE
	if(npcsnake_use_m4a){
		work->snake_status&=~SNAKE_STATUS_USE_USP;
		work->snake_status|=SNAKE_STATUS_USE_FMS;
	}
#endif

	GetDisToRaiden(work);

	if(work->snake_status & SNAKE_STATUS_GAMEOVER) goto gameover_break;

	if(work->npc.action.life<=0 &&
	   !(work->snake_status2 & SNAKE_STATUS2_CALLED_GAMEOVER_VOICE)){

		/* ゲームオーバー音声はポーリングで検知して鳴らす。*/

		// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNAOUT01);
		GM_SeSetMode(SD_V_SNAOUT01,(FVECTOR *)&(work->body.objs->world.m[3][0]),GM_SEMODE_BOMB);

		work->snake_status2|=SNAKE_STATUS2_CALLED_GAMEOVER_VOICE;

		// DEBUG_PRINT_NPCSNAKE("Wuaaaa-a---------a---------a--------a---------\n");

		if(work->snake_status2 & SNAKE_STATUS2_DAMAGED_BY_RAIDEN){
			GM_KillCount++;
		}
	}

#ifdef DEBUG_MODE
	if(work->snake_status2 & SNAKE_STATUS2_DAMAGED_BY_RAIDEN){
		DEBUG_PRINT_NPCSNAKE("Damaged by RAIDEN\n");
	}
#endif

#if 1
	if(work->npc.action.life<=0 &&
	   (work->snake_status2 & SNAKE_STATUS2_FAINT) &&
	   !(work->npc.action.status & NPC_ACT_STATUS_DEATH)){

		/* 気絶中にライフが０なら死亡 */

		work->npc.action.life=-1;

		ZZZSOUND_OFF(work);

		MOTION_CANCELABLE_ON(&(work->npc));
		ChangeNewPad(&(work->npc),PAD_HANG_GAMEOVER);

#if 0
		NPCSnake_CallGameoverProc(work);

		GM_GameOverProcStart(&(work->actor));
		GM_GameOverProcEnd(&(work->actor));
#endif

		DEBUG_PRINT_NPCSNAKE("GameOver (0) !!!!\n");

		return;
	}
#endif

	if(work->npc.action.current_mar==work->cap_marfile &&
	   work->npc.action.current_mot==NPC_MOT_HANG_DIE){

		if(work->snake_status2 & SNAKE_STATUS2_CAPTURED_BY_RAIDEN){
			/* ライデンに絞め殺された場合 */
			work->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
		}
		else{
			work->snake_status2&=~SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
		}
	}
		
	if((work->npc.action.status & NPC_ACT_STATUS_NPCSYS_DAM) &&
	   (work->npc.action.life<=0 || (work->npc.action.status & NPC_ACT_STATUS_DEATH)) &&
	   !(work->npc.action.status & NPC_ACT_STATUS_HANG)){

		/* 首締め中に死んでしまった */

		work->npc.action.life=-1;

		ZZZSOUND_OFF(work);

		MOTION_CANCELABLE_ON(&(work->npc));
		ChangeNewPad(&(work->npc),PAD_HANG_GAMEOVER);

#if 0
		NPCSnake_CallGameoverProc(work);

		GM_GameOverProcStart(&(work->actor));
		GM_GameOverProcEnd(&(work->actor));
#endif

		DEBUG_PRINT_NPCSNAKE("GameOver (1) !!!!\n");

		return;
	}

	if(work->npc.action.status & NPC_ACT_STATUS_FAINT){
		/* 気絶中のはず */
		if(work->npc.action.faint<=0){
			SET_SCN_FAINT();
		}
		work->snake_status2|=SNAKE_STATUS2_FAINT;
	}
	else{
		work->snake_status2&=~SNAKE_STATUS2_FAINT;
	}

	if((work->npc.action.status & NPC_ACT_STATUS_FAINT_END) ||

	   /* 距離に応じて強制的に起こす */
	   ((work->npc.action.status & NPC_ACT_STATUS_FAINT) &&
		work->raiden_dis>NPC_SNAKE_FAINT_DIS)){

		/* 気絶値の回復 */
		if(work->npc.action.faint<=0) work->npc.action.faint=NPC_SNAKE_FAINT;

		MOTION_CANCELABLE_ON(&(work->npc));

		if(work->snake_status2 & SNAKE_STATUS2_ZZZ_SOUND){
			if ( work->npc.action.down_side == NPC_DOWN_SIDE_F ) {
				DEBUG_PRINT_NPCSNAKE("Wakeup F (Sleep)\n");
				ChangeNewPad(&(work->npc),PAD_SLEEP_WAKEUP_F);
			}
			else{
				DEBUG_PRINT_NPCSNAKE("Wakeup B (Sleep)\n");
				ChangeNewPad(&(work->npc),PAD_SLEEP_WAKEUP_B);
			}
		}
		else{
			if ( work->npc.action.down_side == NPC_DOWN_SIDE_F ) {
				DEBUG_PRINT_NPCSNAKE("Wakeup F (Damage)\n");
				ChangeNewPad(&(work->npc),PAD_DAMAGE_WAKEUP_F);
			}
			else{
				DEBUG_PRINT_NPCSNAKE("Wakeup B (Damage)\n");
				ChangeNewPad(&(work->npc),PAD_DAMAGE_WAKEUP_B);
			}
		}

		MOTION_CANCELABLE_OFF(&(work->npc));

		NPC_CallHeadMark(&(work->npc),HMK2_TYPE_KILL);
		NPC_ClearNeedl(&(work->body));
		NPC_DamageCaptureFlagClear(&(work->npc));	/* ダメージのみクリア */

		ZZZSOUND_OFF(work);
	}


gameover_break:

	/* ダメージを受けてる最中はOFF */
	if(!(work->snake_status & SNAKE_STATUS_SYS_DAMAGED)){
		// DEBUG_PRINT_NPCSNAKE("%d\n",NPCSNAKE_ACT_MODE(work));

		/* スネークの状態変更 */
		ModeChange(work);

		// DEBUG_PRINT_NPCSNAKE("- %d\n",NPCSNAKE_ACT_MODE(work));

		/* 状況の取得 */
#ifdef DEBUG_MODE
		if(work->GetSituation!=NULL) (*(work->GetSituation))(work);
		else{
			ASSERT(0);
			DEBUG_PRINT_NPCSNAKE("ERROR : Function is NULL\n");
		}
#else
		(*(work->GetSituation))(work);
#endif

		// DEBUG_PRINT_NPCSNAKE("-- %d\n",NPCSNAKE_ACT_MODE(work));

#ifdef DEBUG_MODE
		/* 行動の決定 */
		if(work->DecideAction!=NULL) (*(work->DecideAction))(work);
		else{
			ASSERT(0);
			DEBUG_PRINT_NPCSNAKE("ERROR : Function is NULL\n");
		}
#else
		(*(work->DecideAction))(work);
#endif

		if(work->homing==NULL &&
		   NPCSNAKE_SHOOT_MODE(work)==NPCSNAKE_SHOOT){

			DEBUG_PRINT_NPCSNAKE("ERROR : %01x / %01x / %02x\n",
								 (work->action_num>>12) & 0x0f,
								 (work->action_num>>8) & 0x0f,
								 work->action_num & 0xff);
		}

		// DEBUG_PRINT_NPCSNAKE("--- %d\n",NPCSNAKE_ACT_MODE(work));

		/* 具体的なモーションの決定 */
		ExecAction(work);

		/* 事後処理 */
		work->pre_raiden_zone=GM_PlayerControl->addr;

		/* スネークがアイテムを投げる */
		NPCSnake_ThrowItem(work);
	}
	else if(work->npc.action.status & NPC_ACT_STATUS_FAINT){
		if(NPCSnake_CheckRaidenDisForFaint(&(work->npc))){
			/* スネークの上をライデンが通った */
			GM_SetPlayerStatus(PLAYER_ON_CORPSE);
		}
	}

	if(work->nodamage_count>0) work->nodamage_count--;

	ActSubMotion(work);

#ifdef DEBUG_MODE
	if(npcsnake_debug_flag){
		DebugDispNPCSnakeStatus(work);
	}
#endif


	/* 表示処理の残り */

	/* 射線調整 */
	if(work->snake_status & SNAKE_STATUS_CALCADJUST){
		// DEBUG_PRINT_NPCSNAKE("Calc Adjust\n");

		switch(work->stage){
		case NPCSNAKE_STAGE_W32A:
			NPCSnake_CalcAdjustW32A(work,1);
			break;
		case NPCSNAKE_STAGE_W14A:
			NPCSnake_CalcAdjustW14A(work);
			break;
		default:
			NPCSnake_CalcAdjust(work);
			break;
		}
		NPCSnake_ClearAdjustW43A(work);
	}
	else if(work->snake_status & SNAKE_STATUS_CALCADJUST0){
		// DEBUG_PRINT_NPCSNAKE("Calc Adjust 0\n");

		NPCSnake_CalcAdjust0(work);
		NPCSnake_ClearAdjustW43A(work);
	}
	else if(work->snake_status & SNAKE_STATUS_CALCADJUST_W43A){
		NPCSnake_CalcAdjustW43A(work);
	}
	else{
		switch(work->stage){
		case NPCSNAKE_STAGE_W32A:
			NPCSnake_CalcAdjustW32A(work,0);
			break;
		default:
			NPCSnake_BackAdjust(work);
			break;
		}
		NPCSnake_BackAdjustW43A(work);
	}

	if(work->snake_status & SNAKE_STATUS_MAKEBULLET){
		if(work->snake_status & SNAKE_STATUS_HAS_USP){
			if(work->stage==NPCSNAKE_STAGE_W14A ||
			   work->stage==NPCSNAKE_STAGE_W43A ||
			   work->stage==NPCSNAKE_STAGE_BETWEEN_STAGES){

				NPCSnakeShootUspP(work,work->usp,work->homing_p);
			}
			else{
				NPCSnakeShootUsp(work,work->usp,work->homing_p);
			}
		}
		else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
			if(work->stage==NPCSNAKE_STAGE_W14A ||
			   work->stage==NPCSNAKE_STAGE_W43A ||
			   work->stage==NPCSNAKE_STAGE_BETWEEN_STAGES){

				NPCSnakeShootFmsP(work,work->fms,work->homing_p);
			}
			else{
				NPCSnakeShootFms(work,work->fms,work->homing_p);
			}
		}
		else if(work->snake_status & SNAKE_STATUS_HAS_PSG){
			if(work->snake_status & SNAKE_STATUS_CYPHER_IN_SIGHT){
				NPCSnakeShootPsg(work,work->psg,work->homing_p,1);
			}
			else{
				NPCSnakeShootPsg(work,work->psg,work->homing_p,0);
			}
		}
	}

	work->snake_status&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|
						  SNAKE_STATUS_CALCADJUST_W43A|SNAKE_STATUS_CALCADJUST_BOOK|
						  SNAKE_STATUS_MAKEBULLET);

	/* 武器の表示処理 */
	NPCSnakeActWeapon(work->usp);
	NPCSnakeActWeapon(work->fms);
	NPCSnakeActWeapon(work->psg);

	/* 白い息 */
	NPCSNAKE_BreathControl(work);

	/* いびき */
	NPCSNAKE_ZZZSound(work);

	/* 目閉じ、まばたき */
	NPCSNAKE_EvmCloseEye(work);

	/* 音声処理 */
	NPCSnake_VoiceControl(work);

	/* スプレーへの対応 */
	if(work->spray_count>0) work->spray_count--;

	/* 連続押しへの対応 */
	if(work->roll_count>0) work->roll_count--;

	/* コールバックが二度呼びされないようにするためのフラグ */
	work->snake_status2&=~SNAKE_STATUS2_CALLED_DAMAGE;

	/* 雑誌への対応 */
	if(work->book_interval>0) work->book_interval--;

	/* ライデンに捕まれた時の抵抗に、振動を追加 */
	NPCSnake_CheckPlayerCapture(work);

#ifdef DEBUG_MODE

#if 0
	{
	    void SigZoneView(int addr,SVECTOR *rgb,float height);
	    SVECTOR a={ 0x80,0x80,0x80,0x40, };
	    SVECTOR b={ 0x20,0x20,0x80,0x40, };
	    SVECTOR c={ 0x80,0x20,0x20,0x40, };

	    SigZoneView(work->npc.navi->next_addr,&a,CVM2N(0.5f));
	    SigZoneView(work->ctrl.addr,&b,CVM2N(0.5f));
	    SigZoneView(work->npc.navi->going_addr,&c,CVM2N(0.5f));

	    if(GV_PadData[1].press & PAD_L1){
		DEBUG_PRINT_NPCSNAKE("%f %f %f\n",
				     work->neighbor_zone_using.vx,
				     work->neighbor_zone_using.vy,
				     work->neighbor_zone_using.vz);
		DEBUG_PRINT_NPCSNAKE("next = 0x%08x , now = 0x%08x , target = 0x%08x\n",
				     work->npc.navi->next_addr,work->ctrl.addr,work->npc.navi->going_addr);
	    }
	}
#endif

#endif

}

void ExitNPCSnake(Work *work)
{
	NPCSnake_VoiceStop(work);

	if(work->life_proc!=0){
		GCL_ARGS args;
		int argv[1];

		args.argc=1;
		args.argv=argv;
		argv[0]=work->npc.action.life;

		/* PROC呼び出し */
		GCL_ExecProc(work->life_proc,&args);
	}
	if(work->end_proc!=0){
		GCL_ARGS args;
		int argv[1];

		args.argc=1;
		args.argv=argv;
		argv[0]=work->ret_flag;

		/* EndProc呼び出し */
		GCL_ExecProc(work->end_proc,&args);
	}

	if(work->support_points!=NULL) GV_Free(work->support_points);

	EndMagazine(work);

	NPCSnakeEndWeapon(work->usp);
	NPCSnakeEndWeapon(work->fms);
	NPCSnakeEndWeapon(work->psg);
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
