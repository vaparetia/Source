//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	npc_common.c
		NPCスネーク
		NPC系関数の初期化、実行、終了処理

	2001/02/13 K.Kano
	$Id: npc_common.c,v 1.1.1.3 2002/11/19 11:43:21 Yoshizawa1 Exp $
 */


#include "npc_snake.h"

#include "targets.c"


#define InitNPCByUserDefined(_s)		InitNPCSnake(_s)
#define ActNPCByUserDefined(_s)			ActNPCSnake(_s)
#define ExitNPCByUserDefined(_s)		ExitNPCSnake(_s)

#define CheckPadByUserDefined			NPCSnake_CheckPadAndSetMotion
#define CheckDamageByUserDefined		NPCSnake_CheckDamage
#define CheckDamageW14AByUserDefined	NPCSnake_CheckDamageW14A
#define CheckDamageW17AByUserDefined	NPCSnake_CheckDamageW17A
#define CheckDamageW32AByUserDefined	NPCSnake_CheckDamageW32A


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


#define OBJECT_FLAG \
	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

void InitNPC(Work *work)
{
	NPCWORK *npc ;
	
	npc = &work->npc ;

	/* NPC初期化 */
	NPC_InitNPC(npc,work,&work->body,&work->ctrl,work->lights);

	/* コントロールの初期化 */
	NPC_InitControl(npc,work->name);

	/* オブジェクトの初期化 */
	NPC_InitObject(npc,work->mdlfile);

	/* マルチウェイトモデルの初期化 */
	work->hiobjs=NULL;

	if(work->mdlfile2!=0){
		if(work->stage==NPCSNAKE_STAGE_W14A) NPC_InitMWObject(npc,work->mdlfile2,(int)CVM2N(10000.0f));
		else NPC_InitMWObject(npc,work->mdlfile2,2000);
		if(npc->body->evmobj!=NULL){
			if(work->farfile!=0){
				GV_SetActorChild(work,NewFaceAnimation(NPCSNAKE_FACEANIM_ACTOR,
													   npc->body->evmobj,work->farfile));
			}
		}

#if 0
		DG_DEF *def;

		def=(DG_DEF *)GV_GetCache(GV_CacheID(work->mdlfile2,'k'));
		if(def!=NULL){
			if((work->hiobjs=DG_MakeObjs(def,OBJECT_FLAG,0))!=NULL){
				DG_QueueObjs(work->hiobjs);
				DG_InvisibleObjs(work->hiobjs);
			}
		}
#endif

	}

	/* モーションの初期化 */
	NPC_InitMotion(npc,work->marfile,0,work->cap_marfile,work->drag_marfile);

	/* ターゲットの初期化 */
	NPC_InitDefenceTarget(npc,&work->deftrg,BOTH_SIDE,
						  SampleTrgSize,SampleDragShift,&DG_ZeroVector);
	NPC_InitDefenceChildTarget(npc,work->def_child,
							   BOTH_SIDE,Child_Target_Size,Child_Target_Shift,
							   Child_Level_Num,Child_Target_Connect);
	NPC_InitDefenceCapture(npc,&work->capture,&work->ctrl,&work->body);

	/* 首締めシフト座標セット */
	NPC_SetHangShift(npc,&HangShift);

	// npc->target.deftrg->class|=TARGET_SKIP ;

	/* ナビシステム初期化 */
	NPC_InitNavi(npc,&work->ctrl,&work->navigate,&work->navitrg);

	/* 初期位置、方向、モーションのセット */
	NPC_InitPose(npc,&(work->pos),work->rot.vy,work->marfile,0);

	/* レーダー光点の初期化 */
	NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
				   5000.0f, -5000.0f, 512, 4000, RADAR_COLOR_RED ) ;

	/* 頭の上に出るマーク */
	if(work->stage==NPCSNAKE_STAGE_W17A){

#if 0

		extern	void  *NewControl_Headmark2( FMATRIX *world, int *type, TARGET *, CONTROL * ) ;

		npc->action.headmark=&work->headmark;

		fpu_CopyMatrix(&(work->headmark_mat),&(work->body.objs->world));
		work->headmark_mat.m[3][1]+=HEADMARK_HEIGHT_OFFSET;

		GV_SetActorChild(work,NewControl_Headmark2(&(work->headmark_mat),npc->action.headmark,
												   &work->deftrg,&work->ctrl));

#else

		npc->action.headmark=&work->headmark;
		npc->headmark=NewControl_Headmark3(&(work->headmark_mat),
										   &work->deftrg,&work->ctrl);
		GV_SetActorChild(work,npc->headmark);

#endif

	}
	else{
		NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->ctrl, &work->deftrg ) ;
	}

	/* ライフ、気絶、気絶回復までの時間の設定 */
	NPC_SetActionParam(&npc->action,work->life_max,NPC_SNAKE_FAINT,NPC_SNAKE_FAINT_COUNT,
					   NPC_SNAKE_SLEEP_COUNT,NPC_SNAKE_DOWN_DAMAGE);

	/* モーションの設定部の指定 */
	NPC_SetCheckPad(&work->npc,CheckPadByUserDefined);

	/* 心音の設定 */
	NPC_SetHeartBeat(&work->npc,NPC_HEART_BEAT_NORMAL);

#if 0
	/* ダメージの設定部の設定 */
	switch(work->stage){
	case NPCSNAKE_STAGE_W14A:
		NPC_SetCheckDamage(&work->npc,CheckDamageW14AByUserDefined);
		break;
	case NPCSNAKE_STAGE_W17A:
	case NPCSNAKE_STAGE_DUMMY:
		NPC_SetCheckDamage(&work->npc,CheckDamageW17AByUserDefined);
		break;
	case NPCSNAKE_STAGE_W32A:
		NPC_SetCheckDamage(&work->npc,CheckDamageW32AByUserDefined);
		break;
	default:
		NPC_SetCheckDamage(&work->npc,CheckDamageByUserDefined);
		break;
	}
#endif


	/* 足影の起動 */
	if(work->foot_shadow){
		void *NewShadow( DG_OBJ *l_foot, DG_OBJ *r_foot, CONTROL *control, FMATRIX *lights, int *flag );
		GV_SetActorChild(work,NewShadow(&work->body.objs->objs[HUMAN21_MIGI_TSUMASAKI],
										&work->body.objs->objs[HUMAN21_HIDARI_TSUMASAKI],
										&work->ctrl,work->lights,&work->foot_shadow));
		work->foot_shadow=1;
	}

	/* w45aでの亀甲フロアエフェクトの追加 */
	if(work->hex_floor_flag){
		DEBUG_PRINT_NPCSNAKE("Regist\n");
		HEX_RegistControlInfo(&(work->hex_floor),&(work->ctrl));
	}

	/* パンチキックの際の攻撃ターゲット */
	{
		static const long64 punchkick_id[]={
			WP_PUNCH|WP_NOPLAYER,
			WP_PUNCH|WP_NOPLAYER,
			WP_PUNCH|WP_NOPLAYER,
			WP_PUNCH|WP_NOPLAYER,
			WP_KICK|WP_NOPLAYER,
			WP_KICK|WP_NOPLAYER,

			WP_KICK|WP_NOPLAYER,	/* for Rolling */
		};
		int i;
		for(i=0;i<sizeof(PunchKick_Shift)/sizeof(PunchKick_Shift[0]);i++){
			GM_SetTarget(&(work->attack[i]),TARGET_OFFENSE|TARGET_ROTATE|TARGET_POWER,
						 0,BOTH_SIDE,(FVECTOR *)&(PunchKick_Size[i]),(FVECTOR *)&(PunchKick_Shift[i]));
			GM_SetTargetWeaponType(&(work->attack[i]),punchkick_id[i]);
		}
	}

	/* w14aでライデンにスネークを狙わせる */
	if(work->stage==NPCSNAKE_STAGE_W14A){
		GM_SetHomingTrg(&(work->my_homing),&(work->body.objs->objs[HUMAN21_KUBI].world),
						&(work->body),&(work->ctrl.map),&(work->ctrl),HOMING_NPC);
		GM_PutHomingTrg(&(work->my_homing));
	}

	/* 首絞められ中は、銃のダメージ無し */
	NPC_SetStatus(npc,NPC_STATUS_NO_HANGDAM);

	/* ユーザー初期化部分 */
	InitNPCByUserDefined(work);
}

void ExitNPC(Work *work)
{
	/* ユーザー終了処理 */
	ExitNPCByUserDefined(work);

	/* w14aでライデンにスネークを狙わせる */
	if(work->stage==NPCSNAKE_STAGE_W14A){
		GM_FreeHomingTrg(&(work->my_homing));
	}

	/* w45aでの亀甲フロアエフェクトの削除 */
	if(work->hex_floor_flag){
		DEBUG_PRINT_NPCSNAKE("Remove\n");
		HEX_RemoveControlInfo(&(work->hex_floor));
	}

	if(work->hiobjs!=NULL){
		DG_QueueObjs(work->hiobjs);
		DG_FreeObjs(work->hiobjs);
	}

	/* NPC共通終了処理 */
	NPC_FreeResources(&(work->npc));
}

void ActNPC(Work *work)
{
	NPCWORK	*npc;

	HZX_ChangeRouteCourse(HZX_ROOT_COURSE1);

	npc=&work->npc;

	/* ＮＰＣシステム系コントロール処理 */
	NPC_ActControl(npc);

	/* ＮＰＣ前処理 */
	NPC_PreProcess(npc);

	/* ユーザーメイン処理*/
	npc->action.pad=npc->action.set_pad;

	ActNPCByUserDefined(work);

	/* モーション再生 */
	NPC_Action(npc);

	/* レーダー表示の方向を変更 */
	NPCSnake_SetRadarDir(work);

	/* ダウンIKはOFF */
	/* 2001/9/4  IK関係は全てOFF */
	npc->action.status&=~(NPC_ACT_STATUS_IK_DOWN|NPC_ACT_STATUS_IK_FOOT|NPC_ACT_STATUS_IK_HAND);

	/* 是角追加 */
	NPC_ActStatusCheck( npc ) ;

	/* 重力計算 */
	NPC_Gravitation(npc);

	if(npc->action.status &
	   (NPC_ACT_STATUS_NPCSYS_DAM|NPC_ACT_STATUS_HANG|NPC_ACT_STATUS_FAINT|
		NPC_ACT_STATUS_DEATH|NPC_ACT_STATUS_DAMAGE)){

		work->snake_status|=SNAKE_STATUS_SYS_DAMAGED;
	}
	else{
		work->snake_status&=~SNAKE_STATUS_SYS_DAMAGED;
	}

	/* ＮＰＣ後処理 */
	NPC_AfterProcess(npc);

	HZX_ClearRouteCourse(0);
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
