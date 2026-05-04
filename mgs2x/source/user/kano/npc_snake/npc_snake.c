//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	npc_snake.c
		NPCスネークメイン

	2001/02/13 K.Kano
	$Id: npc_snake.c,v 1.1.1.3 2002/11/19 11:43:21 Yoshizawa1 Exp $
 */


#include "npc_snake.h"


enum {
	NPCSNAKE_MSG_CHANGE_SUPPORT_LEVEL=0,
	NPCSNAKE_MSG_CHANGE_STAGE,
	NPCSNAKE_MSG_ORDER_TO_SNIPE,
	NPCSNAKE_MSG_STOP_TO_SNIPE,
	NPCSNAKE_MSG_W44A_PARAMS,

	NPCSNAKE_MSG_W44A_CAPUTURE_FLAG,
	NPCSNAKE_MSG_WHITE_BREATH,

	NPCSNAKE_MSG_W43A_MOVE_POINT,

	NPCSNAKE_MSG_W44A_PROHIBIT_TO_HIDE,

	NPCSNAKE_MSG_W44A_FINAL_FIGHT,

	NPCSNAKE_MSG_SET_ITEM_COUNT,
	NPCSNAKE_MSG_SET_RGB6MAG_COUNT,
};


/* デフォルト値の設定 */

/* INDEPEND -> SEARCHENEMYの判定距離 */
#define LINE_TOO_OVER_RAIDEN		CVM2N(2.0f)

/* ライデンを迎えに来るまでの時間 */
#define WAIT_RAIDEN_COUNT(work)			DIRECT_TICK((int)(60.0f*10.0f))

/* ライデンの後方支援モードを続ける時間 */
#define BEHIND_RAIDEN_WAIT_COUNT		DIRECT_TICK((int)(60.0f*5.0f))

/* 隠れから覗き込みへ */
#define HIDING_TO_PEEPING_COUNT(work)	(DIRECT_TICK(hiding_to_peeping_count[(work)->support_level]))

/* 覗き込みから移動へ */
#define PEEPING_TO_MOVE_COUNT(work)		(DIRECT_TICK(peeping_to_move_count[(work)->support_level]))

/* 移動開始から射撃までのカウント */
#define MOVEPOS_COUNT(work)				(DIRECT_TICK(movepos_count[(work)->support_level]))

/* 羽交い締めされるために待つ期間 */
#define BE_CAPTURED_COUNT				DIRECT_TICK((int)(60.0f*2.0f))		/* 2秒 */


static const int hiding_to_peeping_count[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
	/*DIRECT_TICK*/((int)(60.0f*3.0f)),
	/*DIRECT_TICK*/((int)(60.0f*3.0f)),
	/*DIRECT_TICK*/((int)(60.0f*2.5f)),
	/*DIRECT_TICK*/((int)(60.0f*2.5f)),
	/*DIRECT_TICK*/((int)(60.0f*2.0f)),
	/*DIRECT_TICK*/((int)(60.0f*2.0f)),
	/*DIRECT_TICK*/((int)(60.0f*1.5f)),
	/*DIRECT_TICK*/((int)(60.0f*1.5f)),
 	/* STRONG */
};

static const int peeping_to_move_count[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
	/*DIRECT_TICK*/((int)(60.0f*3.0f)),
	/*DIRECT_TICK*/((int)(60.0f*3.0f)),
	/*DIRECT_TICK*/((int)(60.0f*2.5f)),
	/*DIRECT_TICK*/((int)(60.0f*2.5f)),
	/*DIRECT_TICK*/((int)(60.0f*2.0f)),
	/*DIRECT_TICK*/((int)(60.0f*2.0f)),
	/*DIRECT_TICK*/((int)(60.0f*1.5f)),
	/*DIRECT_TICK*/((int)(60.0f*1.0f)),
 	/* STRONG */
};

static const int movepos_count[NPC_SNAKE_SUPPORT_LEVEL_MAX]={
	/* WEEK */
	/*DIRECT_TICK*/((int)(60.0f*1.0f)),
	/*DIRECT_TICK*/((int)(60.0f*1.0f)),
	/*DIRECT_TICK*/((int)(60.0f*1.0f)),
	/*DIRECT_TICK*/((int)(60.0f*0.8f)),
	/*DIRECT_TICK*/((int)(60.0f*0.8f)),
	/*DIRECT_TICK*/((int)(60.0f*0.6f)),
	/*DIRECT_TICK*/((int)(60.0f*0.6f)),
	/*DIRECT_TICK*/((int)(60.0f*0.6f)),
 	/* STRONG */
};


static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int cmd=*(msg->message+0);
		int arg=*(msg->message+1);

		switch(cmd){
		case NPCSNAKE_MSG_CHANGE_SUPPORT_LEVEL:
			work->support_level=arg & (NPC_SNAKE_SUPPORT_LEVEL_MAX-1);
			work->support_level2=arg;
			break;
		case NPCSNAKE_MSG_CHANGE_STAGE:
			NPCSnake_SetStage(work,arg);
			break;
		case NPCSNAKE_MSG_ORDER_TO_SNIPE:
			if(work->stage==NPCSNAKE_STAGE_W32A){
				work->snake_status|=SNAKE_STATUS_ORDER_TO_SNIPE;
			}
			break;
		case NPCSNAKE_MSG_STOP_TO_SNIPE:
			if(work->stage==NPCSNAKE_STAGE_W32A){
				work->snake_status&=~SNAKE_STATUS_ORDER_TO_SNIPE;
			}
			break;
		case NPCSNAKE_MSG_W44A_PARAMS:
			work->behind_to_independ_interval=DIRECT_TICK(*(msg->message+1));
			work->independ_to_search_dis=(float)*(msg->message+2);
			work->search_to_pickup_interval=DIRECT_TICK(*(msg->message+3));
			work->hidding_to_peeping_interval=DIRECT_TICK(*(msg->message+4));
			work->hidding_to_peeping_interval2=DIRECT_TICK(*(msg->message+5));
			work->peeping_to_move_interval=DIRECT_TICK(*(msg->message+6));
			work->move_to_shoot_interval=DIRECT_TICK(*(msg->message+7));
			break;
		case NPCSNAKE_MSG_W44A_CAPUTURE_FLAG:
			if(arg){
				work->snake_status&=~SNAKE_STATUS2_DISABLE_CAPTURED;
			}
			else{
				work->snake_status|=SNAKE_STATUS2_DISABLE_CAPTURED;
			}
			break;
		case NPCSNAKE_MSG_WHITE_BREATH:
			work->breath_interval=DIRECT_TICK(arg);
			work->breath_count=0;
			break;

		case NPCSNAKE_MSG_W43A_MOVE_POINT:
			if(work->stage==NPCSNAKE_STAGE_W43A){
				NPCSNAKE_ACT_SET(work,NPCSNAKE_ACT_W43A_MOVE_POINT);
				NPC_CallHeadMark(&(work->npc),HMK2_TYPE_KILL);

				work->target_pos.vx=(float)*(msg->message+1);
				work->target_pos.vy=(float)*(msg->message+2);
				work->target_pos.vz=(float)*(msg->message+3);
				work->target_pos.vw=1.0f;
			}
			break;
		case NPCSNAKE_MSG_W44A_PROHIBIT_TO_HIDE:
			switch(arg){
			case 0:
				work->snake_status2&=~SNAKE_STATUS2_PROHIBIT_TO_HIDE;
				break;
			case 1:
				work->snake_status2|=SNAKE_STATUS2_PROHIBIT_TO_HIDE;
				break;
			case -1:
				work->snake_status2^=SNAKE_STATUS2_PROHIBIT_TO_HIDE;
				break;
			}
			break;
		case NPCSNAKE_MSG_W44A_FINAL_FIGHT:
			work->snake_status2|=SNAKE_STATUS2_W44A_FINAL_FIGHT;
			work->snake_status2
				&=~(SNAKE_STATUS2_THROWED_RATION|SNAKE_STATUS2_THROWED_SOCOMS_BULLET|
					SNAKE_STATUS2_THROWED_M4S_BULLET);
			break;

		case NPCSNAKE_MSG_SET_ITEM_COUNT:
			work->ration_count=*(msg->message+1);
			work->usp_mag_count=*(msg->message+2);
			work->m4_mag_count=*(msg->message+3);

#ifdef DEBUG_MODE
			DEBUG_PRINT_NPCSNAKE("Set Item count = %d %d %d\n",
								 work->ration_count,work->usp_mag_count,work->m4_mag_count);
#endif

			break;
		case NPCSNAKE_MSG_SET_RGB6MAG_COUNT:
			work->rgb6_mag_count=*(msg->message+1);
			break;
		}

		msg++;
		n_msg--;
    }

#ifdef DEBUG_MODE

#if 0
	if(GV_PadData[1].press & PAD_A){
		work->snake_status|=SNAKE_STATUS_ORDER_TO_SNIPE;
	}
#endif

#endif

	ActNPC(work);
}

static void Die(Work *work)
{
	ExitNPC(work);
	npcsnake_work=NULL;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 初期設定値を取得 */
static int GetOptionValue(Work *work)
{
    FVECTOR x;
    SVECTOR rot;

	work->mdlfile=GV_StrCode("sna_def");
	work->mdlfile2=0;
	work->marfile=GV_StrCode("npc_snake");
	work->cap_marfile=GV_StrCode("npc_snake_cap");
	work->drag_marfile=GV_StrCode("npc_snake_drag");
	work->farfile=0;

	work->support_points=NULL;
	work->n_support_points=0;

	work->support_level=work->support_level2=3;

	work->life_max=NPC_SNAKE_LIFE;

	work->shoot_limit=0;
	work->opt_shoot_limit=0;

	work->stage=0;

	work->w17a_dis=CVM2N(6.0f);

	work->w32a_snipe_first_time=W32A_DEFAULT_SNIPE_TIME;
	work->w32a_snipe_interval_time=W32A_DEFAULT_AFTER_SNIPE_TIME;

	work->hex_floor_flag=0;

	work->life_proc=0;
	work->end_proc=0;
	work->gameover_proc=0;

	work->behind_to_independ_interval=BEHIND_RAIDEN_WAIT_COUNT;
	work->independ_to_search_dis=LINE_TOO_OVER_RAIDEN;
	work->search_to_pickup_interval=WAIT_RAIDEN_COUNT(work);
	work->hidding_to_peeping_interval=HIDING_TO_PEEPING_COUNT(work);
	work->hidding_to_peeping_interval2=HIDING_TO_PEEPING_COUNT(work)*5;
	work->peeping_to_move_interval=PEEPING_TO_MOVE_COUNT(work);
	work->move_to_shoot_interval=MOVEPOS_COUNT(work);
	work->capture_wait_interval=BE_CAPTURED_COUNT;

	work->capture_proc=0;

	work->w32a_start_sleeping_proc=0;
	work->w32a_end_sleeping_proc=0;

	work->scenario_snake_status=0;

	work->foot_shadow=0;

#if 0
	work->sight_min.vx=-1000000000.0;
	work->sight_min.vy=-1000000000.0;
	work->sight_min.vz=-1000000000.0;
	work->sight_max.vx= 1000000000.0;
	work->sight_max.vy= 1000000000.0;
	work->sight_max.vz= 1000000000.0;
#else
	fpu_ClearVector(&(work->sight_min));
	fpu_ClearVector(&(work->sight_max));
#endif

	work->ration_count=-1;
	work->usp_mag_count=-1;
	work->m4_mag_count=-1;
	work->rgb6_mag_count=0;


    if(GCL_GetOption('m')!=NULL){
		work->mdlfile=GCL_GetNextInt();
    } 

    if(GCL_GetOption('n')!=NULL){
		work->mdlfile2=GCL_GetNextInt();
    } 

    if(GCL_GetOption('a')!=NULL){
		work->marfile=GCL_GetNextInt();
    } 
    if(GCL_GetOption('c')!=NULL){
		work->cap_marfile=GCL_GetNextInt();
    } 
    if(GCL_GetOption('d')!=NULL){
		work->drag_marfile=GCL_GetNextInt();
    } 
    if(GCL_GetOption('y')!=NULL){
		work->farfile=GCL_GetNextInt();
    } 

    if(GCL_GetOption('l')!=NULL){
		work->support_level2=GCL_GetNextInt();
		work->support_level=work->support_level2 & (NPC_SNAKE_SUPPORT_LEVEL_MAX-1);
    } 

    if(GCL_GetOption('v')!=NULL){
		work->life_max=GCL_GetNextInt();
		if(GCL_NextStr()!=NULL){
			work->life_proc=GCL_GetNextInt();
		}
    } 

    if(GCL_GetOption('t')!=NULL){
		work->stage=GCL_GetNextInt();
    } 

    if(GCL_GetOption('s')!=NULL){
		int size=GCL_GetNextInt();
		int i;

		if((work->support_points
			=(NPCSNAKE_SUPPORT_POINT *)GV_Malloc(sizeof(NPCSNAKE_SUPPORT_POINT)*size))==NULL){

			return 0;
		}
		work->n_support_points=size;

		for(i=0;i<size;i++){
			int buf[3];

			GCL_GetIV(GCL_NextStr(),buf);
			vu0_IV0toFV((IVECTOR *)buf,&((work->support_points+i)->pos));
			(work->support_points+i)->posflag=GCL_GetNextInt();
		}
	}


    fpu_ClearVector(&x);
    rot=DG_ZeroSVector;

    /* 初期位置 */
    if( GCL_GetOption( 'x' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV( GCL_NextStr(), buf );
		vu0_IV0toFV((IVECTOR *)buf, &x);
    } 

    if( GCL_GetOption( 'r' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		rot.vx=buf[0];
		rot.vy=buf[1];
		rot.vz=buf[2];
    }

	fpu_CopyVector(&(work->pos),&x);
	work->rot=rot;

    fpu_ClearVector(&x);

    if( GCL_GetOption( 'p' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV( GCL_NextStr(), buf );
		vu0_IV0toFV((IVECTOR *)buf, &x);
    } 

	fpu_CopyVector(&(work->target_pos),&x);

    if( GCL_GetOption( 'z' ) != NULL ){
		work->w17a_dis=(float)GCL_GetNextInt();
	}

    if( GCL_GetOption( 'i' ) != NULL ){
		work->w32a_snipe_first_time=DIRECT_TICK(GCL_GetNextInt());
		work->w32a_snipe_interval_time=DIRECT_TICK(GCL_GetNextInt());
	}

    if( GCL_GetOption( 'j' ) != NULL ){
		work->shoot_limit=work->opt_shoot_limit=GCL_GetNextInt();
	}

#if 1
	/* アイテムプロックセット */
	if ( GCL_GetOption( 'q' ) != NULL ){
		KRTH_GetItemProc( &(work->npc.action.item), work->ctrl.name ) ;
	}
#endif

    if( GCL_GetOption( 'k' ) != NULL ){
		work->behind_to_independ_interval=DIRECT_TICK(GCL_GetNextInt());
		work->independ_to_search_dis=(float)GCL_GetNextInt();
		work->search_to_pickup_interval=DIRECT_TICK(GCL_GetNextInt());
		work->hidding_to_peeping_interval=DIRECT_TICK(GCL_GetNextInt());
		work->hidding_to_peeping_interval2=DIRECT_TICK(GCL_GetNextInt());
		work->peeping_to_move_interval=DIRECT_TICK(GCL_GetNextInt());
		work->move_to_shoot_interval=DIRECT_TICK(GCL_GetNextInt());
	}

    if( GCL_GetOption( 'h' ) != NULL ){
		work->hex_floor_flag=GCL_GetNextInt();
	}

    if( GCL_GetOption( 'w' ) != NULL ){
		work->foot_shadow=GCL_GetNextInt();
	}

    if( GCL_GetOption( 'o' ) != NULL ){
		work->capture_proc=GCL_GetNextInt();

		if(GCL_NextStr()!=NULL){
			work->capture_wait_interval=DIRECT_TICK(GCL_GetNextInt());
		}
	}

    if( GCL_GetOption( 'e' ) != NULL ){
		work->end_proc=GCL_GetNextInt();
	}

    if( GCL_GetOption( 'g' ) != NULL ){
		work->gameover_proc=GCL_GetNextInt();
	}

    if( GCL_GetOption( 'f' ) != NULL ){
		work->w32a_start_sleeping_proc=GCL_GetNextInt();
		work->w32a_end_sleeping_proc=GCL_GetNextInt();
	}

    if( GCL_GetOption( 'b' ) != NULL ){
		work->sight_min.vx=(float)GCL_GetNextInt();
		work->sight_min.vy=(float)GCL_GetNextInt();
		work->sight_min.vz=(float)GCL_GetNextInt();
		work->sight_max.vx=(float)GCL_GetNextInt();
		work->sight_max.vy=(float)GCL_GetNextInt();
		work->sight_max.vz=(float)GCL_GetNextInt();

		if(work->sight_min.vx>work->sight_max.vx){
			float t=work->sight_min.vx;
			work->sight_min.vx=work->sight_max.vx;
			work->sight_max.vx=t;
		}
		if(work->sight_min.vy>work->sight_max.vy){
			float t=work->sight_min.vy;
			work->sight_min.vy=work->sight_max.vy;
			work->sight_max.vy=t;
		}
		if(work->sight_min.vz>work->sight_max.vz){
			float t=work->sight_min.vz;
			work->sight_min.vz=work->sight_max.vz;
			work->sight_max.vz=t;
		}
	}

	return 1;
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    if(!GetOptionValue(work)) return 0;

	InitNPC(work);

	NPC_SetDogtag(&(work->npc),DOCTAG_MDLNAME,'u');

    return 1;
}

/* 初期化部メイン */
void *NewNPCSnake(int name,int where)
{
    Work *work;

    npcsnake_work=work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;
		work->where=where;

		GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL;
		}
    }
    return (void *)work;
}
