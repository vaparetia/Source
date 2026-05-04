//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_w45a.c
		NPCスネークw45a用差し替え部分

	2001/03/29 K.Kano
	$Id: snake_w45a.c,v 1.1.1.3 2002/11/19 11:43:24 Yoshizawa1 Exp $
 */


#include "npc_snake.h"


#define RAIDEN_LIMIT1		CVM2N(3.0f)
#define RAIDEN_LIMIT2		CVM2N(5.0f)
#if 0
#define RAIDEN_LIMIT3		CVM2N(2.0f)
#define RAIDEN_LIMIT4		CVM2N(3.0f)
#else
#define RAIDEN_LIMIT3		CVM2N(2.5f)
#define RAIDEN_LIMIT4		CVM2N(3.0f)
#endif
// #define ENEMY_LIMIT			CVM2N(10.0f)
#define ENEMY_LIMIT			CVM2N(5.0f)

#define PUNCH_TO_RAIDEN		CVM2N(1.0f)

#define RAIDEN_LIMIT5		CVM2N(3.0f)
#define RAIDEN_LIMIT6		CVM2N(3.5f)


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 2001/9/18  K.Kano
   w44aでの最後の山場でのスネークの動きを改善
   新モードを追加 */
int GetSupportPosW44AFinal(Work *work)
{
	extern NPCSNAKE_SUPPORT_POINT w44afinal_support_points[];
	NPCSNAKE_SUPPORT_POINT *support_points=w44afinal_support_points;
	int n_support_points=7;

	HOMING_TRG *hom,*cur,*res;
	FVECTOR *pos;
	FVECTOR	*vec,sub_vec;
	float dis2;

	int i,index;


	work->support_pos=NULL;
	work->support_pos_index=-1;


	hom=GM_GetHoming();
	res=NULL;

	/* 
	   スネークの次に目指すポイントの決定
	   1. ライデンから一定距離離れた敵に、一番近いサポートポイント
	   2. ライデンからある程度近い場所
	   */

	pos=&GM_PlayerPosition;
	dis2=FLT_MAX;
	res=NULL;

	/* 敵の検出 */
	while(hom!=NULL){
		float len;

		cur=hom;
		hom=hom->next;

		if((cur->status & HOMING_SKIP) ||
		   (cur->status & HOMING_NPC)) continue;

		vec=(FVECTOR *)&(cur->body->objs->world.m[3][0]);
		fpu_SubVectors(&sub_vec,pos,vec);

		len=fpu_VectorLength2(&sub_vec);
		if(len<ENEMY_LIMIT*ENEMY_LIMIT) continue;

		if(len<dis2){
			dis2=len;
			res=cur;
		}
	}

	if(res==NULL){
		DEBUG_PRINT_NPCSNAKE("Enemy 0\n");
		return 0;
	}


	dis2=FLT_MAX;
	pos=NULL;
	index=-1;

#if 0
	DEBUG_PRINT_NPCSNAKE("%f %f %f\n",
						 res->body->objs->world.m[3][0],
						 res->body->objs->world.m[3][1],
						 res->body->objs->world.m[3][2]);
#endif

	/* 敵に最も近いポイントを目指す */
	for(i=0;i<n_support_points;i++){
		float len;

		fpu_SubVectors(&sub_vec,&(support_points[i].pos),
					   (FVECTOR *)&(res->body->objs->world.m[3][0]));
		len=fpu_VectorLength2(&sub_vec);

		if(len<dis2){
			// DEBUG_PRINT_NPCSNAKE("%d %f\n",i,len);

			pos=&(support_points[i].pos);
			index=i;
			dis2=len;
		}
	}

	if(pos==NULL) return 0;

	work->support_pos=pos;
	work->support_pos_index=index;

	// DEBUG_PRINT_NPCSNAKE("Support Point : %d\n",work->support_pos_index);

	return 1;
}


int GetSupportPosW45A(Work *work)
{
	NPCSNAKE_SUPPORT_POINT *support_points=work->support_points;
	int n_support_points=work->n_support_points;

	HOMING_TRG *hom,*cur,*res;
	FVECTOR *pos;
	FVECTOR	*vec,sub_vec;
	float dis2;

	int i,index;


	work->support_pos=NULL;
	work->support_pos_index=-1;


	hom=GM_GetHoming();
	res=NULL;

	/* 
	   スネークの次に目指すポイントの決定
	   1. ライデンから一定距離離れた敵に、一番近いサポートポイント
	   2. ライデンからある程度近い場所
	   */

	pos=&GM_PlayerPosition;
	dis2=FLT_MAX;
	res=NULL;

	/* 敵の検出 */
	while(hom!=NULL){
		float len;

		cur=hom;
		hom=hom->next;

		if((cur->status & HOMING_SKIP) ||
		   (cur->status & HOMING_NPC)) continue;

		vec=(FVECTOR *)&(cur->body->objs->world.m[3][0]);
		fpu_SubVectors(&sub_vec,pos,vec);

		len=fpu_VectorLength2(&sub_vec);
		if(len<ENEMY_LIMIT*ENEMY_LIMIT) continue;

		if(len<dis2){
			dis2=len;
			res=cur;
		}
	}

	if(res==NULL){
		DEBUG_PRINT_NPCSNAKE("Enemy 0\n");
		return 0;
	}


	dis2=FLT_MAX;
	pos=NULL;
	index=-1;

#if 0
	DEBUG_PRINT_NPCSNAKE("%f %f %f\n",
						 res->body->objs->world.m[3][0],
						 res->body->objs->world.m[3][1],
						 res->body->objs->world.m[3][2]);
#endif

	/* 敵に最も近いポイントを目指す */
	for(i=0;i<n_support_points;i++){
		float len;

		fpu_SubVectors(&sub_vec,&(support_points[i].pos),
					   (FVECTOR *)&(res->body->objs->world.m[3][0]));
		len=fpu_VectorLength2(&sub_vec);

		if(len<dis2){
			// DEBUG_PRINT_NPCSNAKE("%d %f\n",i,len);

			pos=&(support_points[i].pos);
			index=i;
			dis2=len;
		}
	}

	if(pos==NULL) return 0;

	work->support_pos=pos;
	work->support_pos_index=index;

	// DEBUG_PRINT_NPCSNAKE("Support Point : %d\n",work->support_pos_index);

	return 1;
}

void GetNeighborHzxW45A(Work *work)
{
	HZX_ZONE_ADD zone=work->ctrl.addr;
	HZX_ZON *link;
	float len,maxlen=-FLT_MAX;
	int no;
	int i;

	if(work->homing==NULL){
		if(work->support_pos!=NULL){
			fpu_CopyVector(&(work->neighbor_zone),work->support_pos);
		}
		else{
			link=HZX_GetZoneFromAdd(GM_PlayerControl->addr);

			work->neighbor_zone.vx=link->x;
			work->neighbor_zone.vy=work->ctrl.mov.vy;
			work->neighbor_zone.vz=link->z;
		}
		return;
	}

	link=HZX_GetZoneFromAdd(zone);
	no=HZX_ZoneMapNo(zone);

	for(i=0;i<5;i++){
		HZX_ZON *zon;
		FVECTOR pos;

		if(link->nears[i]==0xff) continue;
		if(link->near_flag[i] & HZX_ROOT_JUMP) continue;

		zon=HZX_GetZoneNo(no,link->nears[i]);

		pos.vx=zon->x;
		pos.vy=work->ctrl.mov.vy;
		pos.vz=zon->z;

		vu0_Ldv0(&pos);
		vu0_Ldv1(&(work->ctrl.mov));

		vu0_Subv0v1();
		len=vu0_VectorLength2v0();

		if(len>maxlen){
			maxlen=len;
			fpu_CopyVector(&(work->neighbor_zone),&pos);
		}
	}

	if(maxlen<0.0f){
		if(work->support_pos!=NULL){
			fpu_CopyVector(&(work->neighbor_zone),work->support_pos);
		}
		else{
			link=HZX_GetZoneFromAdd(GM_PlayerControl->addr);

			work->neighbor_zone.vx=link->x;
			work->neighbor_zone.vy=work->ctrl.mov.vy;
			work->neighbor_zone.vz=link->z;
		}
	}
}


/* ライデンを目標に移動 */
int ActMoveAroundRaiden(Work *work)
{
	NPCWORK	*npc;
	FVECTOR *pos;
	FVECTOR sub_vec;
	float len;

	npc=&work->npc;

	pos=&GM_PlayerPosition;

	/* nvtrg に 目標のCONTROLのposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,pos,0);

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	fpu_SubVectors(&sub_vec,pos,&(work->ctrl.mov));
	len=fpu_VectorLength2(&sub_vec);

	if(work->snake_status & SNAKE_STATUS_MOVING_MOTION){
		/* ライデンに３メートルまで近づいた */
		if(len<RAIDEN_LIMIT1*RAIDEN_LIMIT1){
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
			npc->action.dir = -1;
			return 1;
		}
	}
	else{
		/* ライデンから５メートル以内のところにいる */
		if(len<RAIDEN_LIMIT2*RAIDEN_LIMIT2){
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
			npc->action.dir = -1;
			return 1;
		}
	}

	/* ライデンに向かって移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,500)){
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		npc->action.dir = -1;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}


/* 2001/9/19  K.Kano
   w44aでの最後の山場でのスネークの動きを改善
   新モードを追加 */
int ActMoveAroundRaiden2(Work *work)
{
	NPCWORK	*npc;
	FVECTOR *pos;
	FVECTOR sub_vec;
	float len;

	npc=&work->npc;

	pos=&GM_PlayerPosition;

	/* nvtrg に 目標のCONTROLのposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,pos,0);

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	fpu_SubVectors(&sub_vec,pos,&(work->ctrl.mov));
	len=fpu_VectorLength2(&sub_vec);

	if(work->snake_status & SNAKE_STATUS_MOVING_MOTION){
		/* ライデンに3メートルまで近づいた */
		if(len<RAIDEN_LIMIT5*RAIDEN_LIMIT5){
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
			npc->action.dir = -1;
			return 1;
		}
	}
	else{
		/* ライデンから3.5メートル以内のところにいる */
		if(len<RAIDEN_LIMIT6*RAIDEN_LIMIT6){
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
			npc->action.dir = -1;
			return 1;
		}
	}

	/* ライデンに向かって移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,500)){
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		npc->action.dir = -1;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}


/* ライデンを目標に移動 */
int ActMoveFlwRaidenOnly(Work *work)
{
	NPCWORK	*npc;
	FVECTOR *pos;
	FVECTOR sub_vec;
	float len;

	npc=&work->npc;

	pos=&GM_PlayerPosition;

	/* nvtrg に 目標のCONTROLのposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,pos,0);

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	switch(npc->action.set_pad){
	case PAD_BRINGING_USP:
		ChangeNewPad(npc,PAD_STAND_WUSP);
		ActSubMotionReset(work);
		return 0;
	case PAD_BRINGING_FMS:
		ChangeNewPad(npc,PAD_STAND_WFMS);
		ActSubMotionReset(work);
		return 0;
	case PAD_BRINGING_USP_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WUSP);
		return 0;
	case PAD_BRINGING_FMS_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WFMS);
		return 0;
	}

	fpu_SubVectors(&sub_vec,pos,&(work->ctrl.mov));
	len=fpu_VectorLength2(&sub_vec);

	if(work->snake_status & SNAKE_STATUS_MOVING_MOTION){
		/* ライデンに２メートルまで近づいた */
		if(len<RAIDEN_LIMIT3*RAIDEN_LIMIT3){
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
			npc->action.dir = -1;
			return 1;
		}
	}
	else{
		/* ライデンから３メートル以内のところにいる */
		if(len<RAIDEN_LIMIT4*RAIDEN_LIMIT4){
			work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
			npc->action.dir = -1;

			work->snake_status|=SNAKE_STATUS_CALCADJUST_W43A;

			/* 雑誌の判定 */
			NPCSnake_CheckBook(work);

			return 1;
		}
	}

	/* ライデンに向かって移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,500)){
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		npc->action.dir = -1;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}

int ActPointTarget(Work *work)
{
	NPCWORK	*npc;

	npc=&work->npc;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	if(work->snake_status & SNAKE_STATUS_HAS_USP){
		if(npc->action.set_pad!=PAD_BRINGING_USP){
			ChangeNewPad(npc,PAD_BRINGING_USP);
		}
	}
	else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
		if(npc->action.set_pad!=PAD_BRINGING_FMS){
			ChangeNewPad(npc,PAD_BRINGING_FMS);
		}
	}

	return 1;
}

/* ライデンに向かって撃つ */
int ActShootRaiden(Work *work)
{
	NPCWORK	*npc;
	FVECTOR *pos;


	npc=&work->npc;

	pos=&GM_PlayerPosition;

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	// fpu_SubVectors(&sub_vec,pos,&(work->ctrl.mov));
	// len=fpu_VectorLength2(&sub_vec);

	work->npc.action.dir=work->homing_dir;

	{
		int dir=(work->homing_dir-work->ctrl.rot.vy) & 0xfff;

		if(dir>DIR_ENABLE_SHOOT && dir<(0x1000-DIR_ENABLE_SHOOT)){
			if(work->snake_status & SNAKE_STATUS_HAS_USP){
				ChangeNewPad(npc,PAD_START_SHOOT_USP);
			}
			else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
				ChangeNewPad(npc,PAD_START_SHOOT_FMS);
			}
			return 1;
		}
	}

	if(work->snake_status & SNAKE_STATUS_HAS_USP){
		if(npc->action.set_pad==PAD_BRINGING_USP ||
		   npc->action.set_pad==PAD_START_SHOOT_USP ||
		   npc->action.set_pad==PAD_SHOOT_USP){

			ChangeNewPad(npc,PAD_SHOOT_USP);
		}
		else{
			ChangeNewPad(npc,PAD_START_SHOOT_USP);
		}
	}
	else if(work->snake_status & SNAKE_STATUS_HAS_FMS){
		if(npc->action.set_pad==PAD_BRINGING_FMS ||
		   npc->action.set_pad==PAD_START_SHOOT_FMS ||
		   npc->action.set_pad==PAD_SHOOT_FMS){

			ChangeNewPad(npc,PAD_SHOOT_FMS);
		}
		else{
			ChangeNewPad(npc,PAD_START_SHOOT_FMS);
		}
	}

	return 1;
}

/* ライデンを目標に移動 */
int ActAngry(Work *work)
{
	NPCWORK	*npc;
	FVECTOR *pos;

	npc=&work->npc;

	pos=&GM_PlayerPosition;

	/* nvtrg に 目標のCONTROLのposとzoneaddress をセット */
	GM_SetNaviTargetFromPos(npc->nvtrg,pos,0);

	/* 前処理 */
	if(ActPreMove(work,npc)){
		npc->action.dir=-1;
		return 0;
	}

	switch(npc->action.set_pad){
	case PAD_BRINGING_USP:
		ChangeNewPad(npc,PAD_STAND_WUSP);
		ActSubMotionReset(work);
		return 0;
	case PAD_BRINGING_FMS:
		ChangeNewPad(npc,PAD_STAND_WFMS);
		ActSubMotionReset(work);
		return 0;
	case PAD_BRINGING_USP_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WUSP);
		return 0;
	case PAD_BRINGING_FMS_AND_SITTING:
		ChangeNewPad(npc,PAD_STANDUP_WFMS);
		return 0;
	}

	/* ライデンに向かって移動 */
	if(NPCSnake_NaviNear(work,npc->navi,npc->nvtrg,PUNCH_TO_RAIDEN)){
		NPCSnake_GetPlayerTargetPos(npc);
		work->snake_status|=SNAKE_STATUS_ACTION_FINISHED;
		npc->action.dir = -1;
		return 1;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	return 0;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
