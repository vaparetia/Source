//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_w32a.c
		狙撃スネーク専用

	2001/04/10 K.Kano
	$Id: snake_w32a.c,v 1.1.1.3 2002/11/19 11:43:24 Yoshizawa1 Exp $
 */


#include "npc_snake.h"
#include "../../morita/emma/include/emma_com.h"


#define SIGHT_RANGE		0.9f

#define CHECK_RATE		0.2f

#define NAME_CLAYMORE	0x00cb7e6e		/* claymore */


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 2001/9/18  K.Kano
   クレイモアを狙撃するために判定 */

/* --- Begin --- */

static void ClaymoreCheckInSight(Work *work,float dis2,float cos2_max)
{
	TARGET *tar,*cur,*res;
	FVECTOR *emapos=EMA_CommandGetPosition();
	DG_CHANL *chanl;
	float cos2,tcos2;

	int pl_weapon=PL_GetPlayerWeapon();
	int pl_item=PL_GetPlayerItem();
	float chanl_cos2[2];


	if(emapos==NULL ||
	   (pl_weapon!=WP_Psg1 &&
		pl_weapon!=WP_Psg1T) ||
	   pl_item!=IT_Thermal){

		work->claymore=NULL;
		return;
	}


	chanl_cos2[0]=DG_Chanl(0)->screen*DG_Chanl(0)->screen;
	chanl_cos2[0]/=(SIGHT_RANGE*SIGHT_RANGE+chanl_cos2[0]);

	chanl_cos2[1]=DG_Chanl(1)->screen*DG_Chanl(1)->screen;
	chanl_cos2[1]/=(SIGHT_RANGE*SIGHT_RANGE+chanl_cos2[1]);


	tar=GM_TrgDefListGetTop();
	res=NULL;


	while(tar!=NULL){
		FMATRIX world;
		FVECTOR min,max;
		int flag;
		float tlen2;
		float tdis2;


		cur=tar ;
		tar=tar->next ;


		if((cur->class & (TARGET_DEFENSE|TARGET_HAS_NAME))
		   !=(TARGET_DEFENSE|TARGET_HAS_NAME)){

			continue;
		}

		if(cur->class & (TARGET_DEAD|TARGET_SKIP)){
			continue;
		}

		if(cur->name!=NAME_CLAYMORE){
			continue;
		}

		// DEBUG_PRINT_NPCSNAKE("Find Claymore\n");

		fpu_CopyMatrix(&world,&(cur->world));
		fpu_CopyVector((FVECTOR *)&(world.m[3][0]),&(cur->center));
		world.m[3][3]=1.0f;

		if(vu0_CheckBoundingBox((FVECTOR *)&(world.m[3][0]),
								(FVECTOR *)&(work->sight_min),
								(FVECTOR *)&(work->sight_max))){

			// DEBUG_PRINT_NPCSNAKE("Failed Sight BoundCheck\n");
			continue;
		}


		vu0_Ldv0(&(cur->size));
		vu0_Ldv1(&(cur->size));

		vu0_Mulv0a(-1.0f);

		vu0_Stv0(&min);
		vu0_Stv1(&max);


		/* 画面内にある */
		if((flag=DG_BoundCheck(&world,&max,&min))==-1){
			// DEBUG_PRINT_NPCSNAKE("Failed BoundCheck\n");
			continue;
		}


		if(!(flag & 1) && DG_Chanl(0)->flag==1){
			chanl=DG_Chanl(0);
			cos2=chanl_cos2[0];
		}
		else if(!(flag & 2) && DG_Chanl(1)->flag==1){
			chanl=DG_Chanl(1);
			cos2=chanl_cos2[1];
		}
		else{
			// DEBUG_PRINT_NPCSNAKE("Flag ???\n");
			continue;
		}


		vu0_Ldv1((FVECTOR *)&(world.m[3][0]));
		vu0_Ldv2((FVECTOR *)&(chanl->eye.m[3][0]));
		vu0_Ldv0((FVECTOR *)&(chanl->eye.m[2][0]));

		vu0_Subv1v2();

#if 1
		tlen2=vu0_VectorLength2v0();
		tlen2*=vu0_VectorLength2v1();
		tcos2=vu0_InnerProductv0v1();
		if(tcos2<0.0f) continue;
		tcos2*=tcos2;
		tcos2/=tlen2;
#else
		tlen2=vu0_VectorLength2v1();
		tlen2=fpu_Rsqrt(tlen2,1.0f);
		vu0_Mulv1a(tlen2);
		tcos2=vu0_InnerProductv0v1();
		if(tcos2<0.0f) continue;
		tcos2*=tcos2;
#endif

		/* PSG1装備時は、サイト内にあることを確認 */
		if(pl_weapon==WP_Psg1 ||
		   pl_weapon==WP_Psg1T){

			if(cos2>tcos2){
				// DEBUG_PRINT_NPCSNAKE("Not Insight of PSG-1 , %f %f\n",cos2,tcos2);
				continue;
			}
		}


		vu0_Ldv0(emapos);
		vu0_Ldv1((FVECTOR *)&(world.m[3][0]));

		vu0_Subv0v1();

		tdis2=vu0_VectorLength2v0();


		if(!HZX_OnlineHazardCheck(GM_PlayerControl->hzx_id,
								  (FVECTOR *)&(chanl->eye.m[3][0]),(FVECTOR *)&(world.m[3][0]),
								  HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE_ONLY,
								  0,0)){

			/* 壁に遮られずに見える */

#if 0
			/* エマとの距離を条件にした場合 */
			if(tdis2<dis2){
#else
			/* 視界内で中心に近いもの選んだ場合 */
			if(cos2_max<tcos2){
#endif
				dis2=tdis2;
				res=cur;
				cos2_max=tcos2;

				// DEBUG_PRINT_NPCSNAKE("Claymore Finished\n");
			}
		}
		else{
			// DEBUG_PRINT_NPCSNAKE("Failed to check Online HZX\n");
		}
	}

	if(res!=NULL){
		work->homing=NULL;

		// DEBUG_PRINT_NPCSNAKE("Claymore Insight\n");
	}
	work->claymore=res;
}

/* 狙う場所の決定 */
static void ClaymoreDecideTarget(Work *work)
{
	TARGET *tar;

	tar=work->claymore;

	if(tar!=NULL){
		vu0_Ldv0(&(tar->offset));
		vu0_Ldm0(&(tar->world));
		vu0_Setv0w0();
		vu0_Ldv1(&(tar->center));

		vu0_Mulv0m0v0();

		vu0_Setv1w1();

		vu0_Addv1v0();

		vu0_Stv1(&(work->homing_pos));

		work->homing_p=&(work->homing_pos);

		// DEBUG_PRINT_NPCSNAKE("Decide Claymore target\n");
	}
}

static int ClaymoreRecheckInSight(Work *work)
{
	TARGET *tar;
	FMATRIX world;
	FVECTOR min,max;
	int flag;

	DG_CHANL *chanl;
	float cos2;

	int pl_weapon=PL_GetPlayerWeapon();
	int pl_item=PL_GetPlayerItem();


	if((pl_weapon!=WP_Psg1 &&
		pl_weapon!=WP_Psg1T) ||
	   pl_item!=IT_Thermal){

		return 0;
	}

	tar=work->claymore;

	fpu_CopyMatrix(&world,&(tar->world));
	fpu_CopyVector((FVECTOR *)&(world.m[3][0]),&(tar->center));
	world.m[3][3]=1.0f;

	if(vu0_CheckBoundingBox((FVECTOR *)&(world.m[3][0]),
							(FVECTOR *)&(work->sight_min),
							(FVECTOR *)&(work->sight_max))){

		return 0;
	}


	vu0_Ldv0(&(tar->size));
	vu0_Ldv1(&(tar->size));

	vu0_Mulv0a(-1.0f);

	vu0_Stv0(&min);
	vu0_Stv1(&max);


	/* 画面内にある */
	if((flag=DG_BoundCheck(&world,&max,&min))==-1){

#ifdef DEBUG_MODE
		// DEBUG_PRINT_NPCSNAKE("Failed BoundCheck\n");
#endif

		return 0;
	}

	if(!(flag & 1) && DG_Chanl(0)->flag==1){
		chanl=DG_Chanl(0);
	}
	else if(!(flag & 2) && DG_Chanl(1)->flag==1){
		chanl=DG_Chanl(1);
	}
	else return 0;


	cos2=chanl->screen*chanl->screen;
	cos2/=(SIGHT_RANGE*SIGHT_RANGE+cos2);


#if 1
	/* PSG1装備時は、サイト内にあることを確認 */
	if(pl_weapon==WP_Psg1 ||
	   pl_weapon==WP_Psg1T){

		float tcos2,tlen2;

		vu0_Ldv1((FVECTOR *)&(world.m[3][0]));
		vu0_Ldv2((FVECTOR *)&(chanl->eye.m[3][0]));
		vu0_Ldv0((FVECTOR *)&(chanl->eye.m[2][0]));

		vu0_Subv1v2();

		tlen2=vu0_VectorLength2v0();
		tlen2*=vu0_VectorLength2v1();
		tcos2=vu0_InnerProductv0v1();
		if(tcos2<0.0f) return 0;
		tcos2*=tcos2;
		tcos2/=tlen2;

		if(cos2>tcos2){

#ifdef DEBUG_MODE
			// DEBUG_PRINT_NPCSNAKE("Not Insight of PSG-1 , %f %f\n",cos2,tcos2);
#endif

			return 0;
		}
	}
#endif

	return 1;
}

static int SearchClaymoreAfterShoot(Work *work)
{
	TARGET *tar,*cur;

	if(work->claymore==NULL) goto failed;

	// if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)) goto failed;

	tar=GM_TrgDefListGetTop();

	while(tar!=NULL){
		cur=tar;
		tar=tar->next;

		if(cur->class & (TARGET_DEAD|TARGET_SKIP)) continue;

		if((int)cur==(int)(work->claymore)){
			if(ClaymoreRecheckInSight(work)) return 1;
			else{
				// DEBUG_PRINT_NPCSNAKE("--- Failed to Recheck\n");
				break;
			}
		}
	}

failed:
	work->claymore=NULL;

	return 0;
}

/* --- End --- */



/* ライデンの視界内にいるかどうかの判定(w32a) */
void NPCSnake_CheckInSight(Work *work)
{
	HOMING_TRG *hom,*cur,*res;
	FVECTOR *emapos=EMA_CommandGetPosition();
	float dis2=FLT_MAX;
	DG_CHANL *chanl;
	float cos2,tcos2;
	float cos2_max=-FLT_MAX;

	int pl_weapon=PL_GetPlayerWeapon();
	int pl_item=PL_GetPlayerItem();
	float chanl_cos2[2];


	/* 2001/8/2  Ken Kano
	   PSG1Tと双眼鏡、カメラに追加対応 */
	if(emapos==NULL ||
	   (pl_weapon!=WP_Psg1 &&
		pl_weapon!=WP_Psg1T &&
		pl_item!=IT_Camera &&
		pl_item!=IT_TnkCamera &&
		pl_item!=IT_Scope)){

		work->homing=NULL;
		return;
	}


	chanl_cos2[0]=DG_Chanl(0)->screen*DG_Chanl(0)->screen;
	chanl_cos2[0]/=(SIGHT_RANGE*SIGHT_RANGE+chanl_cos2[0]);

	chanl_cos2[1]=DG_Chanl(1)->screen*DG_Chanl(1)->screen;
	chanl_cos2[1]/=(SIGHT_RANGE*SIGHT_RANGE+chanl_cos2[1]);


	hom=GM_GetHoming();
	res=NULL;


	while(hom!=NULL){
		FVECTOR min,max;
		FVECTOR k[4];
		OBJECT *object;
		int flag;
		float tlen2;
		float tdis2;


		cur=hom ;
		hom=hom->next ;

		if(cur->status & HOMING_EMA_IN_SIGHT){
			if((cur->status & HOMING_SKIP) ||
			   (cur->status & HOMING_NPC)){

				// DEBUG_PRINT_NPCSNAKE("???? Skip And Ema Homing\n");
			}
		}

		if((cur->status & HOMING_SKIP) ||
		   (cur->status & HOMING_NPC)){

#ifdef DEBUG_MODE
			// if(cur->status & HOMING_NPC) DEBUG_PRINT_NPCSNAKE("Skip For NPC\n");
#endif

			continue;
		}

		// if(!(cur->status & HOMING_EMA_IN_SIGHT)) continue;


		/* エマを狙うものを優先するが、基本的に画面内に写るもの全てが対象となる */

		object=cur->body;

		if(vu0_CheckBoundingBox((FVECTOR *)&(object->objs->world.m[3][0]),
								(FVECTOR *)&(work->sight_min),
								(FVECTOR *)&(work->sight_max))){

			continue;
		}


		vu0_Ldv0(&(object->objs->bound_min));
		vu0_Ldv1(&(object->objs->bound_max));

		vu0_Mulv0a(CHECK_RATE);
		vu0_Mulv1a(CHECK_RATE);

		vu0_Stv0(&min);
		vu0_Stv1(&max);


		/* 画面内にある */
		if((flag=DG_BoundCheck(&(object->objs->world),&max,&min))==-1){

#ifdef DEBUG_MODE
			// DEBUG_PRINT_NPCSNAKE("Failed BoundCheck\n");
#endif

			continue;
		}


		if(!(flag & 1) && DG_Chanl(0)->flag==1){
			chanl=DG_Chanl(0);
			cos2=chanl_cos2[0];
		}
		else if(!(flag & 2) && DG_Chanl(1)->flag==1){
			chanl=DG_Chanl(1);
			cos2=chanl_cos2[1];
		}
		else{
#ifdef DEBUG_MODE
			DEBUG_PRINT_NPCSNAKE("Flag ???\n");
#endif
			continue;
		}


		vu0_Ldv1((FVECTOR *)&(object->objs->world.m[3][0]));
		vu0_Ldv2((FVECTOR *)&(chanl->eye.m[3][0]));
		vu0_Ldv0((FVECTOR *)&(chanl->eye.m[2][0]));

		vu0_Subv1v2();

#if 1
		tlen2=vu0_VectorLength2v0();
		tlen2*=vu0_VectorLength2v1();
		tcos2=vu0_InnerProductv0v1();
		if(tcos2<0.0f) continue;
		tcos2*=tcos2;
		tcos2/=tlen2;
#else
		tlen2=vu0_VectorLength2v1();
		tlen2=fpu_Rsqrt(tlen2,1.0f);
		vu0_Mulv1a(tlen2);
		tcos2=vu0_InnerProductv0v1();
		if(tcos2<0.0f) continue;
		tcos2*=tcos2;
#endif

		/* PSG1装備時は、サイト内にあることを確認 */
		if(pl_weapon==WP_Psg1 ||
		   pl_weapon==WP_Psg1T){

			if(cos2>tcos2){

#ifdef DEBUG_MODE
				FVECTOR v;

				vu0_Stv1(&v);

				DEBUG_PRINT_NPCSNAKE("Not Insight of PSG-1 , %f %f\n",cos2,tcos2);
				DEBUG_PRINT_NPCSNAKE("%f %f %f , %f %f %f\n",
									 v.vx,v.vy,v.vz,
									 chanl->eye.m[2][0],chanl->eye.m[2][1],chanl->eye.m[2][2]);
#endif

				continue;
			}
		}


		vu0_Ldv0(emapos);
		vu0_Ldv1((FVECTOR *)&(object->objs->world.m[3][0]));

		vu0_Subv0v1();

		tdis2=vu0_VectorLength2v0();

		/* エマを見つけている敵は無条件で狙撃！！！ */
		if(cur->status & HOMING_EMA_IN_SIGHT){
			if(res==NULL){
				dis2=tdis2;
				res=cur;
				cos2_max=tcos2;
			}
			else if(!(res->status & HOMING_EMA_IN_SIGHT)){
				dis2=tdis2;
				res=cur;
				cos2_max=tcos2;
			}
			else{
#if 0
				/* エマとの距離を条件にした場合 */
				if(tdis2<dis2){
#else
				/* 視界内で中心に近いもの選んだ場合 */
				if(cos2_max<tcos2){
#endif
					dis2=tdis2;
					res=cur;
					cos2_max=tcos2;
				}
			}
			continue;
		}


		/* Onlineチェック点の作成 */
		fpu_CopyVector(&(k[0]),&min);
		fpu_CopyVector(&(k[1]),&max);
		fpu_CopyVector(&(k[2]),&max);
		fpu_CopyVector(&(k[3]),&max);

		k[1].vy=min.vy;
		k[2].vx=min.vx;
		k[3].vz=min.vz;

		vu0_Ldv0(&(k[0]));
		vu0_Ldv1(&(k[1]));
		vu0_Ldv2(&(k[2]));

		vu0_Ldm0(&(object->objs->world));

		vu0_Mulv0m0v0();
		vu0_Mulv1m0v1();

		vu0_Stv0(&(k[0]));

		vu0_Ldv0(&(k[3]));

		vu0_Mulv2m0v2();
		vu0_Mulv0m0v0();

		vu0_Stv1(&(k[1]));
		vu0_Stv2(&(k[2]));
		vu0_Stv0(&(k[3]));


		{
			int i;

			for(i=0;i<4;i++){
				if(!HZX_OnlineHazardCheck(GM_PlayerControl->hzx_id,
										  (FVECTOR *)&(chanl->eye.m[3][0]),&(k[i]),
										  HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE_ONLY,
										  0,0)){

					/* 壁に遮られずに見える */

					/* エマを狙う敵を優先 */
					if(res==NULL){
						dis2=tdis2;
						res=cur;
						cos2_max=tcos2;

#ifdef DEBUG_MODE
						// DEBUG_PRINT_NPCSNAKE("Finished\n");
#endif

					}
					else if(!(cur->status & HOMING_EMA_IN_SIGHT)){
						if(!(res->status & HOMING_EMA_IN_SIGHT)){
#if 0
							/* エマとの距離を条件にした場合 */
							if(tdis2<dis2){
#else
							/* 視界内で中心に近いもの選んだ場合 */
							if(cos2_max<tcos2){
#endif
								dis2=tdis2;
								res=cur;
								cos2_max=tcos2;

#ifdef DEBUG_MODE
								// DEBUG_PRINT_NPCSNAKE("Finished\n");
#endif

							}
						}
					}
					else{
						if(res->status & HOMING_EMA_IN_SIGHT){
#if 0
							/* エマとの距離を条件にした場合 */
							if(tdis2<dis2){
#else
							/* 視界内で中心に近いもの選んだ場合 */
							if(cos2_max<tcos2){
#endif
								dis2=tdis2;
								res=cur;
								cos2_max=tcos2;

#ifdef DEBUG_MODE
								// DEBUG_PRINT_NPCSNAKE("Finished\n");
#endif

							}
						}
						else{
							dis2=tdis2;
							res=cur;
							cos2_max=tcos2;

#ifdef DEBUG_MODE
							// DEBUG_PRINT_NPCSNAKE("Finished\n");
#endif
						}
					}

					break;
				}
			}

#ifdef DEBUG_MODE
			if(i==4) DEBUG_PRINT_NPCSNAKE("Failed to check Online HZX\n");
#endif

		}
	}

	work->homing=res;

	/* 2001/9/18  K.Kano
	   クレイモアを狙撃するために判定 */
	ClaymoreCheckInSight(work,dis2,cos2_max);
}

/* 狙う場所の決定 */
void NPCSnake_DecideTarget(Work *work)
{
	HOMING_TRG *res;

	res=work->homing;

	if(res!=NULL){
		if(res->body->m_ctrl!=NULL){
			int part_num;

			part_num=NPCSnake_SearchPart(work);
			if(part_num<0) part_num=HUMAN21_HIDARI_UDE1;

			switch(res->body->m_ctrl->flag & MT_FLAG_TYPE_MASK){
			case MT_FLAG_HUMAN1:
			case MT_FLAG_HUMAN2:
			case MT_FLAG_HUMAN3:
				/* 人間モデル */
				work->snake_status&=~SNAKE_STATUS_CYPHER_IN_SIGHT;
				fpu_CopyVector(&(work->homing_pos),
							   (FVECTOR *)&(res->body->objs->objs[part_num].world.m[3][0]));
				work->homing_p=&(work->homing_pos);
				break;
			default:
				/* サイファー */
				if(part_num==HUMAN21_ATAMA){
					part_num=1;
					fpu_CopyVector(&(work->homing_pos),
								   (FVECTOR *)&(res->body->objs->objs[part_num].world.m[3][0]));
					work->homing_pos.vy-=CVM2N(0.2f);
					work->homing_p=&(work->homing_pos);
				}
				else{
					part_num=0;
					fpu_CopyVector(&(work->homing_pos),
								   (FVECTOR *)&(res->body->objs->objs[part_num].world.m[3][0]));
					work->homing_p=&(work->homing_pos);
				}
				work->snake_status|=SNAKE_STATUS_CYPHER_IN_SIGHT;
				break;
			}
		}
		else{
			/* 多分サイファー */

			int part_num;

			part_num=NPCSnake_SearchPart(work);

			if(part_num==HUMAN21_ATAMA){
				part_num=1;
				fpu_CopyVector(&(work->homing_pos),
							   (FVECTOR *)&(res->body->objs->objs[part_num].world.m[3][0]));
				work->homing_pos.vy-=CVM2N(0.2f);
				work->homing_p=&(work->homing_pos);
			}
			else{
				part_num=0;
				fpu_CopyVector(&(work->homing_pos),
							   (FVECTOR *)&(res->body->objs->objs[part_num].world.m[3][0]));
				work->homing_p=&(work->homing_pos);
			}
			work->snake_status|=SNAKE_STATUS_CYPHER_IN_SIGHT;
		}
	}
	else{
		/* 2001/9/18  K.Kano
		   クレイモアを狙撃するために判定 */

		ClaymoreDecideTarget(work);
		work->snake_status&=~SNAKE_STATUS_CYPHER_IN_SIGHT;
	}
}


static int RecheckInSight(Work *work)
{
	FVECTOR min,max;
	OBJECT *object;
	int flag;

	DG_CHANL *chanl;
	float cos2;

	int pl_weapon=PL_GetPlayerWeapon();
	int pl_item=PL_GetPlayerItem();


	/* 2001/8/2  Ken Kano
	   PSG1Tと双眼鏡、カメラに追加対応 */
	if(pl_weapon!=WP_Psg1 &&
	   pl_weapon!=WP_Psg1T &&
	   pl_item!=IT_Camera &&
	   pl_item!=IT_TnkCamera &&
	   pl_item!=IT_Scope){

		return 0;
	}


	object=work->homing->body;


	if(vu0_CheckBoundingBox((FVECTOR *)&(object->objs->world.m[3][0]),
							(FVECTOR *)&(work->sight_min),
							(FVECTOR *)&(work->sight_max))){

		return 0;
	}


	vu0_Ldv0(&(object->objs->bound_min));
	vu0_Ldv1(&(object->objs->bound_max));

	vu0_Mulv0a(CHECK_RATE);
	vu0_Mulv1a(CHECK_RATE);

	vu0_Stv0(&min);
	vu0_Stv1(&max);


	/* 画面内にある */
	if((flag=DG_BoundCheck(&(object->objs->world),&max,&min))==-1){

#ifdef DEBUG_MODE
		// DEBUG_PRINT_NPCSNAKE("Failed BoundCheck\n");
#endif

		return 0;
	}

	if(!(flag & 1) && DG_Chanl(0)->flag==1){
		chanl=DG_Chanl(0);
	}
	else if(!(flag & 2) && DG_Chanl(1)->flag==1){
		chanl=DG_Chanl(1);
	}
	else return 0;


	cos2=chanl->screen*chanl->screen;
	cos2/=(SIGHT_RANGE*SIGHT_RANGE+cos2);


#if 1
	/* PSG1装備時は、サイト内にあることを確認 */
	if(pl_weapon==WP_Psg1 ||
	   pl_weapon==WP_Psg1T){

		float tcos2,tlen2;

		vu0_Ldv1((FVECTOR *)&(object->objs->world.m[3][0]));
		vu0_Ldv2((FVECTOR *)&(chanl->eye.m[3][0]));
		vu0_Ldv0((FVECTOR *)&(chanl->eye.m[2][0]));

		vu0_Subv1v2();

		tlen2=vu0_VectorLength2v0();
		tlen2*=vu0_VectorLength2v1();
		tcos2=vu0_InnerProductv0v1();
		if(tcos2<0.0f) return 0;
		tcos2*=tcos2;
		tcos2/=tlen2;

		if(cos2>tcos2){

#ifdef DEBUG_MODE
			// DEBUG_PRINT_NPCSNAKE("Not Insight of PSG-1 , %f %f\n",cos2,tcos2);
#endif

			return 0;
		}
	}
#endif

	return 1;
}

int NPCSnake_SearchHomingAfterShoot(Work *work)
{
	HOMING_TRG *hom,*cur;

	if(work->homing==NULL) goto failed;

	// if(!(work->snake_status & SNAKE_STATUS_LOCKON_ENEMY)) goto failed;

	hom=GM_GetHoming();

	while(hom!=NULL){
		cur=hom;
		hom=hom->next;

		if((cur->status & HOMING_SKIP) ||
		   (cur->status & HOMING_NPC)) continue;

		if((int)cur==(int)(work->homing)){
			if(RecheckInSight(work)) return 1;
			else{
				// DEBUG_PRINT_NPCSNAKE("--- Failed to Recheck\n");
				break;
			}
		}
	}

failed:
	/* 2001/9/18  K.Kano
	   クレイモアを狙撃するために判定 */
	if(SearchClaymoreAfterShoot(work)) return 1;

	work->homing=NULL;
	work->homing_p=NULL;

	work->snake_status&=~SNAKE_STATUS_LOCKON_ENEMY;

	return 0;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


void ActWaitW32A0(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

#if 1
	if(npc->action.set_pad==PAD_STAND){
		ChangeNewPad(npc,PAD_SITDOWN);
	}
	else if(npc->action.set_pad==PAD_SITTING){
		ChangeNewPad(npc,PAD_CROUCHON);
	}
	else if(npc->action.set_pad==PAD_CROUCHING){
		work->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
	}
#endif

}

void ActWaitW32A(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

#if 1
	if(npc->action.set_pad==PAD_STAND){
		ChangeNewPad(npc,PAD_SITDOWN);
	}
	else if(npc->action.set_pad==PAD_SITTING){
		ChangeNewPad(npc,PAD_CROUCHON);
	}
	else if(npc->action.set_pad==PAD_CROUCHING){
		ChangeNewPad(npc,PAD_CROUCHING_WPSG);
	}
	else if(npc->action.set_pad==PAD_CROUCHING_WPSG){
		work->snake_status|=SNAKE_STATUS_SHOOT_ACTION_FINISHED;
	}
#endif

}

void ActShootW32A(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if(npc->action.set_pad!=PAD_SHOOT_PSG){
		ChangeNewPad(npc,PAD_SHOOT_PSG);
	}
}

void ActPreCounterW32A(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if(npc->action.set_pad!=PAD_CROUCHING_WPSG){
		ChangeNewPad(npc,PAD_CROUCHING_WPSG);
	}
	work->snake_status|=SNAKE_STATUS_CALCADJUST;
}

void ActCounterW32A(Work *work)
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if(npc->action.set_pad!=PAD_SHOOT_PSG){
		ChangeNewPad(npc,PAD_SHOOT_PSG);
	}
	work->snake_status|=SNAKE_STATUS_CALCADJUST;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


int NPCSnake_CheckDamageW32A(NPCWORK *npc)
{
	TARGET	*def;
	NPCACT	*act ;
	NPCTARGET	*trg ;
	long64	weapon ;
	int		dam_child_num ;

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	weapon = 0 ;

	if(GM_CheckGameStatus(STATE_SCN_DEMO)){
		NPC_DamageCaptureFlagClear(npc);
		return 0;
	}

	if((dam_child_num=NPC_ChildTargetCheck(npc))>=0){
		weapon=(npc->target.def_child+dam_child_num)->weapon_type;
	}

	if(TARGET_POWER & def->damaged){
		weapon=def->weapon_type;
	}

	if(weapon){
		// DEBUG_PRINT_NPCSNAKE("npc damage weapon type [%lx]  \n",weapon);

//		act->damtrg = def ;
		NPC_DamageCaptureFlagClear( npc ) ;

		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if(!(weapon & WP_NOPLAYER)){
			ACCESS_WORK(npc)->ret_flag|=NPCSNAKE_RETFLAG_DAMAGED_FROM_PLAYER;
			ACCESS_WORK(npc)->snake_status2|=SNAKE_STATUS2_DAMAGED_BY_RAIDEN;
		}

		ACCESS_WORK(npc)->snake_status
			&=~(SNAKE_STATUS_CALCADJUST|SNAKE_STATUS_CALCADJUST0|SNAKE_STATUS_MAKEBULLET);

		NPCSNAKE_VOICE_STOP(ACCESS_WORK(npc));

		// ACCESS_WORK(npc)->snake_status&=~SNAKE_STATUS_ORDER_TO_SNIPE;

		MOTION_CANCELABLE_ON(npc);
		UNLOCK_ENEMY(npc);

		NPCSNAKE_NODAMAGE_COUNTSET(npc,NODAMAGE_COUNT);

		if(weapon & (WP_M92|WP_PSG1T)){
			GM_SeSetMode(SD_A_M_NINE01,
						 (FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

			if(NPCSNAKE_ACT_MODE(ACCESS_WORK(npc))!=NPCSNAKE_ACT_W32A_SLEEP){
				NPCSNAKE_ACT_SET(ACCESS_WORK(npc),NPCSNAKE_ACT_W32A_SLEEP);

				/* ZZZZzzzz */
				NPC_CallHeadMark(npc,HMK2_TYPE_PIYO_A);

				if(ACCESS_WORK(npc)->w32a_start_sleeping_proc!=0){
					/* PROC呼び出し */
					GCL_ExecProc(ACCESS_WORK(npc)->w32a_start_sleeping_proc,NULL);
				}

				ZZZSOUND_ON(ACCESS_WORK(npc));
			}

			ChangeNewPad(npc,PAD_DAMAGE_PSG1T_CROUNING);

			ACCESS_WORK(npc)->gcounter=0;
		}
		else{
			ChangeNewPad(npc,PAD_DAMAGE_CROUCHING);

			/* Ｚｚｚｚをクリア */
			NPC_CallHeadMark(npc,HMK2_TYPE_KILL);
		}

		/* うっ */
		// GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_V_SNADMG02);
		GM_SeSetMode(SD_V_SNADMG02,(FVECTOR *)&(npc->body->objs->world.m[3][0]),GM_SEMODE_BOMB);

		return 1 ;
	}

	return 0 ;
}
