//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	snake_weapon.c
		NPCスネークの使用武器

	2001/03/15 K.Kano
	$Id: snake_weapon.c,v 1.1.1.3 2002/11/19 11:43:24 Yoshizawa1 Exp $
 */


#include "npc_snake.h"

#define USP_FILE_NAME	MDL_USP
#define FMS_FILE_NAME	MDL_M4A
#define PSG_FILE_NAME	MDL_PSG


#define OBJECT_FLAG \
	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)


#define BULLET_TYPE					(BUL_TYPE_VISIBLE|BUL_TYPE_SPARK| \
									 BUL_TYPE_SCAR|BUL_TYPE_HIT_CHILD|BUL_TYPE_NO_PLAYER)
#define PSG1_BULLET_TYPE			(BUL_TYPE_SPARK|BUL_TYPE_SCAR| \
									 BUL_TYPE_HIT_CHILD|BUL_TYPE_NO_PLAYER)
#define BULLET_SIZE					25
#define BULLET_DAMAGE				5
#define BULLET_LENGTH				((int)CVM2N(20.0f))
#define BULLET_SPEED				((int)CVM2N(1.0f))

#define PSG1_LENGTH					((int)CVM2N(5000.0f))


/* cos 45度/2 */
#define COS_ADJUST_LR_LIMIT			0.923879533f
/* sin 45度/2 */
#define SIN_ADJUST_LR_LIMIT			0.382683432f

/* cos 60度/2 */
#define COS_ADJUST_UDLR_LIMIT		0.866025404f
#define SIN_ADJUST_UDLR_LIMIT		0.5f


/* w44aでのアイテムを投げられる位置の判定座標値 */
#define POSZ_DISABLE_THROW_ITEM		(-136000.0f)


#if 1

#include "../../skoba/weapon/arms_data.h"

#else

enum {
	// 麻酔銃
	ARMS_DATA_M92_SNA = 0,  // 0
	// 弾丸
	ARMS_DATA_USP,          // 1
	ARMS_DATA_SCM,          // 2
	ARMS_DATA_SCM_S,        // 3
	ARMS_DATA_MKR,          // 4
	ARMS_DATA_M92,          // 5
	ARMS_DATA_SAA,          // 6
	// 機銃
	ARMS_DATA_FMS,          // 7
	ARMS_DATA_ABK,          // 8
	ARMS_DATA_AKS,          // 9
	ARMS_DATA_AKS_S,        // 10
	ARMS_DATA_M4A_NM,       // 11
	ARMS_DATA_P90,          // 12
	// 特殊
	ARMS_DATA_STG,          // 13
	ARMS_DATA_M4B_GL,       // 14
	ARMS_DATA_LNR,          // 15

	// 追加
	ARMS_DATA_M4B_GUN,         // 16
	ARMS_DATA_M4B_GUN_GRENADE, // 17
	ARMS_DATA_M4B_GUN_SEL,     // 18
	ARMS_DATA_M4B_GL_SEL,      // 19
	ARMS_DATA_M4A1_GUN_SNK,     // 20
	ARMS_DATA_M4A1_GL_SNK,     // 21
	ARMS_DATA_M4A1_GUN_HI,     // 22

	ARMS_DATA_PSG,     // 23
	// 尻
	ARMS_DATA_BOTTOM,          // 24
};


#endif



void *NewArmsEffectControl(int name,int arms_id,FMATRIX *world);
void *NewBullet(FMATRIX	*world,u_int type,u_int	side,u_int size,u_int damage,
				u_int length,u_int speed,int weapon);
void *NewItemBox_Prog(int name,int where,int splash,int model_name,
					  int shadow_name,int label_name,FVECTOR *pos,float height,
					  int spec,int id,int num,int pattern,FVECTOR *force);


#include "../../mode/demo/libdemo.h"


void *NPCSnakeInitUsp(Work *work)
{
	FMATRIX *m=&(work->body.objs->objs[HUMAN21_MIGI_TE].world);
	FMATRIX *light=work->lights;

	DG_OBJS *objs;
	DG_DEF *def;

	def=(DG_DEF *)GV_GetCache(GV_CacheID(USP_FILE_NAME,'k'));
	if(def==NULL){
		ASSERT(0);
		return NULL;
	}

	objs=DG_MakeObjs(def,OBJECT_FLAG,0);
	if(objs==NULL){
		ASSERT(0);
		return NULL;
	}

    DG_QueueObjs(objs);
	DG_InvisibleObjs(objs);

	objs->root=m;
	objs->light=light;

	GV_SetActorChild(work,NewArmsEffectControl(NPCSNAKE_USP_EFFECT_NAME,ARMS_DATA_USP,m));

	return objs;
}

void *NPCSnakeInitFms(Work *work)
{
	FMATRIX *m=&(work->body.objs->objs[HUMAN21_MIGI_TE].world);
	FMATRIX *light=work->lights;

	DG_OBJS *objs;
	DG_DEF *def;

	def=(DG_DEF *)GV_GetCache(GV_CacheID(FMS_FILE_NAME,'k'));
	if(def==NULL){
		ASSERT(0);
		return NULL;
	}

	objs=DG_MakeObjs(def,OBJECT_FLAG,0);
	if(objs==NULL){
		ASSERT(0);
		return NULL;
	}

    DG_QueueObjs(objs);
	DG_InvisibleObjs(objs);

	objs->root=m;
	objs->light=light;

	GV_SetActorChild(work,NewArmsEffectControl(NPCSNAKE_FMS_EFFECT_NAME,ARMS_DATA_M4A1_GUN_SNK,m));

	return objs;
}

void *NPCSnakeInitPsg(Work *work)
{
	FMATRIX *m=&(work->body.objs->objs[HUMAN21_MIGI_TE].world);
	FMATRIX *light=work->lights;

	DG_OBJS *objs;
	DG_DEF *def;

	def=(DG_DEF *)GV_GetCache(GV_CacheID(PSG_FILE_NAME,'k'));
	if(def==NULL){
		ASSERT(0);
		return NULL;
	}

	objs=DG_MakeObjs(def,OBJECT_FLAG,0);
	if(objs==NULL){
		ASSERT(0);
		return NULL;
	}

    DG_QueueObjs(objs);
	DG_InvisibleObjs(objs);

	objs->root=m;
	objs->light=light;

	GV_SetActorChild(work,NewArmsEffectControl(NPCSNAKE_PSG_EFFECT_NAME,ARMS_DATA_PSG,m));

	return objs;
}

void NPCSnakeVisibleWeapon(void *objs)
{
	if(objs==NULL) return;
	DG_VisibleObjs((DG_OBJS *)objs);
}

void NPCSnakeInvisibleWeapon(void *objs)
{
	if(objs==NULL) return;
	DG_InvisibleObjs((DG_OBJS *)objs);
}

void NPCSnakeEndWeapon(void *objs)
{
	if(objs==NULL) return;
    DG_DequeueObjs((DG_OBJS *)objs);
	DG_FreeObjs((DG_OBJS *)objs);
}

static inline void CalcBulletMatrix(FMATRIX *m,FVECTOR * const target,
									FMATRIX * const base,const FVECTOR * const shift)
{
	float len;

	vu0_Ldv0(shift);
	vu0_Ldm0(base);
	vu0_Mulv0m0v0();
	vu0_Ldv1(target);
	vu0_Ldv2((FVECTOR *)&(base->m[2][0]));
	vu0_Subv1v0();
	vu0_Stv0((FVECTOR *)&(m->m[3][0]));
	len=vu0_VectorLength2v1();
	vu0_Mulv1a(fpu_Rsqrt(len,-1.0f));
	vu0_OuterProductv2v1v2();
	vu0_Stv1((FVECTOR *)&(m->m[1][0]));
	vu0_Stv2((FVECTOR *)&(m->m[0][0]));
	vu0_OuterProductv2v1();
	vu0_Stv2((FVECTOR *)&(m->m[2][0]));
}

void NPCSnakeShootUsp(Work *work,void *objs,FVECTOR * const target)
{
	static const FVECTOR shift={  17.5f,-115.9f,-1.2f, 1.0f };
	FMATRIX m;

	GV_MSG weapon_msg;
	int message[8];


	CalcBulletMatrix(&m,target,((DG_OBJS *)objs)->root,&shift);

	NewBullet(&m,BULLET_TYPE,ENEMY_SIDE,BULLET_SIZE,
			  BULLET_DAMAGE,BULLET_LENGTH,BULLET_SPEED,WP_Usp);

	weapon_msg.address=NPCSNAKE_USP_EFFECT_NAME;
	weapon_msg.message=message;
	weapon_msg.message[0]=0; /* MSG_ARMSCON_MUZZLE */
	weapon_msg.message[1]=1; /* AMO */
	weapon_msg.message[2]=0; /* LINE */
	weapon_msg.message[3]=0; /* REC */
	weapon_msg.message[4]=0; /* TMP */
	weapon_msg.message[5]=0; /* SPARK */
	weapon_msg.message[6]=0; /* SAA LEFT */
	weapon_msg.message_len=7;

	GV_SendMessage(&(weapon_msg));

	// GM_SeSetMode(SD_W_SOCOM01,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
	GM_SeSetMode(SD_W_SOCOMNP1,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
}

void NPCSnakeShootUspP(Work *work,void *objs,FVECTOR * const target)
{
	static const FVECTOR shift={  17.5f,-115.9f,-1.2f, 1.0f };
	FMATRIX m;

	GV_MSG weapon_msg;
	int message[8];


	CalcBulletMatrix(&m,target,((DG_OBJS *)objs)->root,&shift);

	NewBullet(&m,BULLET_TYPE,PLAYER_SIDE,BULLET_SIZE,
			  BULLET_DAMAGE,BULLET_LENGTH,BULLET_SPEED,WP_Usp);

	weapon_msg.address=NPCSNAKE_USP_EFFECT_NAME;
	weapon_msg.message=message;
	weapon_msg.message[0]=0; /* MSG_ARMSCON_MUZZLE */
	weapon_msg.message[1]=1; /* AMO */
	weapon_msg.message[2]=0; /* LINE */
	weapon_msg.message[3]=0; /* REC */
	if(work->stage==NPCSNAKE_STAGE_W43A) weapon_msg.message[4]=1; /* TMP */
	else weapon_msg.message[4]=0; /* TMP */
	weapon_msg.message[5]=0; /* SPARK */
	weapon_msg.message[6]=0; /* SAA LEFT */
	weapon_msg.message_len=7;

	GV_SendMessage(&(weapon_msg));

	// GM_SeSetMode(SD_W_SOCOM01,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
	GM_SeSetMode(SD_W_SOCOMNP1,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
}

void NPCSnakeShootFms(Work *work,void *objs,FVECTOR * const target)
{
	static const FVECTOR shift={  19.5f,-537.5f,65.5f, 1.0f };
	FMATRIX m;

	GV_MSG weapon_msg;
	int message[8];


	CalcBulletMatrix(&m,target,((DG_OBJS *)objs)->root,&shift);

	NewBullet(&m,BULLET_TYPE,ENEMY_SIDE,BULLET_SIZE,
			  BULLET_DAMAGE,BULLET_LENGTH,BULLET_SPEED,WP_m4);

	weapon_msg.address=NPCSNAKE_FMS_EFFECT_NAME;
	weapon_msg.message=message;
	weapon_msg.message[0]=0; /* MSG_ARMSCON_MUZZLE */
	weapon_msg.message[1]=1; /* AMO */
	weapon_msg.message[2]=0; /* LINE */
	weapon_msg.message[3]=0; /* REC */
	weapon_msg.message[4]=0; /* TMP */
	weapon_msg.message[5]=0; /* SPARK */
	weapon_msg.message[6]=0; /* SAA LEFT */
	weapon_msg.message_len=7;

	GV_SendMessage(&(weapon_msg));

	// GM_SeSetMode(SD_W_FAMAS02,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
	GM_SeSetMode(SD_W_FAMASNP1,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
	{ // DEBUG
		FVECTOR* pv = (FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]);
		printf("%f,%f,%f\n", pv->vx, pv->vy, pv->vz );
	}
}

void NPCSnakeShootFmsP(Work *work,void *objs,FVECTOR * const target)
{
	static const FVECTOR shift={  19.5f,-537.5f,65.5f, 1.0f };
	FMATRIX m;

	GV_MSG weapon_msg;
	int message[8];


	CalcBulletMatrix(&m,target,((DG_OBJS *)objs)->root,&shift);

	NewBullet(&m,BULLET_TYPE,PLAYER_SIDE,BULLET_SIZE,
			  BULLET_DAMAGE,BULLET_LENGTH,BULLET_SPEED,WP_m4);

	weapon_msg.address=NPCSNAKE_FMS_EFFECT_NAME;
	weapon_msg.message=message;
	weapon_msg.message[0]=0; /* MSG_ARMSCON_MUZZLE */
	weapon_msg.message[1]=1; /* AMO */
	weapon_msg.message[2]=0; /* LINE */
	weapon_msg.message[3]=0; /* REC */
	if(work->stage==NPCSNAKE_STAGE_W43A) weapon_msg.message[4]=1; /* TMP */
	else weapon_msg.message[4]=0; /* TMP */
	weapon_msg.message[5]=0; /* SPARK */
	weapon_msg.message[6]=0; /* SAA LEFT */
	weapon_msg.message_len=7;

	GV_SendMessage(&(weapon_msg));

	// GM_SeSetMode(SD_W_FAMAS02,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
	GM_SeSetMode(SD_W_FAMASNP1,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
	{ // DEBUG
		FVECTOR* pv = (FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]);
		printf("%f,%f,%f\n", pv->vx, pv->vy, pv->vz );
	}
}

void NPCSnakeShootPsg(Work *work,void *objs,FVECTOR * const target,int flag)
{
	static const FVECTOR shift={  17.5f,-954.0f,89.7f, 1.0f };
	FMATRIX m;

	GV_MSG weapon_msg;
	int message[8];


	CalcBulletMatrix(&m,target,((DG_OBJS *)objs)->root,&shift);

	vu0_Ldv1((FVECTOR *)&(m.m[1][0]));
	vu0_Ldv0(target);

	if(flag){
		vu0_Mulv1a(CVM2N(2.0f));
	}
	else{
		vu0_Mulv1a(CVM2N(0.5f));
	}
	vu0_Addv0v1();

	vu0_Stv0((FVECTOR *)&(m.m[3][0]));

	NewBullet(&m,PSG1_BULLET_TYPE,BOTH_SIDE,BULLET_SIZE,
			  BULLET_DAMAGE,PSG1_LENGTH,BULLET_SPEED,WP_Psg1);

	weapon_msg.address=NPCSNAKE_PSG_EFFECT_NAME;
	weapon_msg.message=message;
	weapon_msg.message[0]=0; /* MSG_ARMSCON_MUZZLE */
	weapon_msg.message[1]=0; /* AMO */
	weapon_msg.message[2]=0; /* LINE */
	weapon_msg.message[3]=0; /* REC */
	weapon_msg.message[4]=0; /* TMP */
	weapon_msg.message[5]=0; /* SPARK */
	weapon_msg.message[6]=0; /* SAA LEFT */
	weapon_msg.message_len=7;

	GV_SendMessage(&(weapon_msg));

	GM_SeSetMode(SD_W_RIFLENP1,(FVECTOR *)&(((DG_OBJS *)objs)->root->m[3][0]),GM_SEMODE_BOMB);
}

void NPCSnakeActWeapon(void *objs)
{
	DG_OBJ *obj;
	int n_models;
	int i;

	if(objs==NULL) return;

	obj=((DG_OBJS *)objs)->objs;
	n_models=((DG_OBJS *)objs)->n_models;

	for(i=0;i<n_models;i++){
		int parent=obj[i].parent;

		vu0_Ldv0(&(obj[i].trans));
		vu0_Setv0w1();

		if(parent<0){
			vu0_Ldm0(((DG_OBJS *)objs)->root);
		}
		else{
			vu0_Ldm0(&(obj[parent].world));
		}

		vu0_Mulv0m0v0();

		vu0_Stm0(&(obj[i].world));
		vu0_Stv0((FVECTOR *)&(obj[i].world.m[3][0]));
	}
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* w14a用。プレイヤーの位置と方向を取得して、銃を構える方向に反映する。*/
void NPCSnake_GetPlayerTargetPos(NPCWORK *npc)
{
	FVECTOR d;

	if(GM_CheckPlayerStatus(PLAYER_WATCH)){
		/* 主観 */
		fpu_CopyVector(&(ACCESS_WORK(npc)->homing_pos),
					   (FVECTOR *)&(GM_PlayerSubjectCamera[0]->position));
	}
	else{
		/* 位置を決め直す */
		fpu_CopyVector(&(ACCESS_WORK(npc)->homing_pos),
					   (FVECTOR *)&(GM_PlayerBody->objs->objs[HUMAN21_MUNE].world.m[3][0]));
	}

	ACCESS_WORK(npc)->homing_p=&(ACCESS_WORK(npc)->homing_pos);

	d.vx=ACCESS_WORK(npc)->homing_p->vx-npc->ctrl->mov.vx;
	d.vy=ACCESS_WORK(npc)->homing_p->vy-npc->ctrl->mov.vy;
	d.vz=ACCESS_WORK(npc)->homing_p->vz-npc->ctrl->mov.vz;

	ACCESS_WORK(npc)->homing_dir=GV_VecDir2(&d);
}


/* ------------------------------------------------------------------------ */
/* Shoot Adjust                                                             */
/* ------------------------------------------------------------------------ */


#define HUMANEVM_FACESTART		21
#define HUMANEVM_FACEEND		(HUMANEVM_FACESTART+32)


static inline void RecalcWorldMatrix(Work *work,FMATRIX *am)
{
	DG_OBJ *objs=work->body.objs->objs;
	DG_EVMOBJ *evm=work->body.evmobj;
	FMATRIX *evmmat;
	int i;

	if(evm!=NULL) evmmat=evm->matrix[evm->use_buffer];
	else evmmat=NULL;

	vu0_Ldm0(&(am[1]));
	vu0_Ldm1(&(objs[HUMAN21_ONAKA].world));
	vu0_Mulm2m0m1();
	vu0_Stm2(&(objs[HUMAN21_ONAKA].world));

	if(evmmat!=NULL){
		vu0_Ldm1(&(evmmat[HUMAN21_ONAKA]));
		vu0_Mulm2m0m1();
		vu0_Stm2(&(evmmat[HUMAN21_ONAKA]));
	}

	vu0_Ldm1(&(am[0]));
	vu0_Mulm2m1m0();

	for(i=HUMAN21_MUNE;i<=HUMAN21_ATAMA;i++){
		vu0_Ldm1(&(objs[i].world));
		vu0_Mulm0m2m1();
		vu0_Stm0(&(objs[i].world));
	}
	if(evmmat!=NULL){
		for(i=HUMAN21_MUNE;i<=HUMAN21_ATAMA;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
		for(i=HUMANEVM_FACESTART;i<HUMANEVM_FACEEND;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
	}
}


#define ADJUST_UD_PARAM_1		0.25f
#define ADJUST_UD_PARAM_0		0.075f

#define ADJUST_LR_PARAM_1		0.25f
#define ADJUST_LR_PARAM_0		0.05f
#define ADJUST_LR_PARAM_2		0.35f

#ifdef KP_XBOX
static void CheckQuart(
					   FVECTOR *q
#ifdef DEBUG_MODE
					   , char *name
#endif
					   )
{
	 if ( _isnan( q->vx ) ) {
#ifdef DEBUG_MODE
		 printf( "snake_weapon.c : QuartX is nan(%x) %s\n", *(int *)&q->vx, name ) ;
#endif
		q->vx = 0.0f ;
	}
	if ( _isnan( q->vy ) ) {
#ifdef DEBUG_MODE
		printf( "snake_weapon.c : QuartY is nan(%x) %s\n", *(int *)&q->vy, name ) ;
#endif
		q->vy = 0.0f ;
	}
	if ( _isnan( q->vz ) ) {
#ifdef DEBUG_MODE
		printf( "snake_weapon.c : QuartZ is nan(%x) %s\n", *(int *)&q->vz, name ) ;
#endif
		q->vz = 0.0f ;
	}
	if ( _isnan( q->vw ) ) {
#ifdef DEBUG_MODE
		printf( "snake_weapon.c : QuartW is nan(%x) %s\n", *(int *)&q->vw, name ) ;
#endif
		q->vw = 1.0f ;
	}
}
#endif

static inline void CalcAdjustUD(Work *work,int flag,FMATRIX *am)
{
	DG_OBJS *objs=work->body.objs;
	FVECTOR vec0,vec1,vec2,q,qq;
	float param;

	if(flag){
		FMATRIX m;

		vu0_Ldm0(&(am[1]));
		vu0_Ldm1(&(objs->objs[HUMAN21_KUBI].world));
		vu0_Mulm2m0m1();
		vu0_Stm2(&m);

		vu0_Ldv0(work->homing_p);
		vu0_Ldv1((FVECTOR *)&(m.m[3][0]));

		vu0_Subv2v0v1();

		vu0_Stv0(&vec1);
		vu0_Stv1(&vec0);
		vu0_Stv2(&vec2);

		vec2.vy=0.0f;

		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);

		param=ADJUST_UD_PARAM_1;
	}
	else{
		fpu_ClearVector(&qq);
		qq.vw=1.0f;

		param=ADJUST_UD_PARAM_0;
	}
#ifdef KP_XBOX
	CheckQuart( &qq
#ifdef DEBUG_MODE
			   , "AdjustUD qq"
#endif
			   ) ;
	CheckQuart( &work->pre_adjust0
#ifdef DEBUG_MODE
			   , "AdjustUD pre_adjust0"
#endif
			   ) ;
#endif

	MT_QuatSlerp(&q,&(work->pre_adjust0),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust0),&q);

	MT_QuatToMat(&(am[0]),&q);


	vu0_Clrv0();
	vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_MUNE].world.m[3][0]));

	fpu_CopyVector((FVECTOR *)&(am[0].m[3][0]),
				   (FVECTOR *)&(objs->objs[HUMAN21_MUNE].world.m[3][0]));

	vu0_Subv1v0v1();

	vu0_Ldm0(&(am[0]));
	vu0_Mulv1m0v1();

	vu0_Stv1((FVECTOR *)&(am[0].m[3][0]));
}

static inline void CalcAdjustLR(Work *work,int flag,FMATRIX *am)
{
	DG_OBJS *objs=work->body.objs;
	FVECTOR vec0,vec1,vec2,q,qq;
	float param;

	if(flag){
		fpu_CopyVector(&vec1,work->homing_p);
		fpu_CopyVector(&vec0,(FVECTOR *)&(objs->world.m[3][0]));
		fpu_CopyVector(&vec2,(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));

		vec0.vy=0.0f;
		vec1.vy=0.0f;
		vec2.vy=0.0f;

		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);

		/* 曲がり過ぎに注意 */
		if(qq.vw<COS_ADJUST_LR_LIMIT && flag==1){
			float s=fpu_Rsqrt(1.0f-qq.vw*qq.vw,SIN_ADJUST_LR_LIMIT);
			qq.vx*=s;
			qq.vy*=s;
			qq.vz*=s;
			qq.vw=COS_ADJUST_LR_LIMIT;
		}

		param=ADJUST_LR_PARAM_1;
	}
	else{
		fpu_ClearVector(&qq);
		qq.vw=1.0f;

		param=ADJUST_LR_PARAM_0;
	}

#ifdef KP_XBOX
	CheckQuart( &qq
#ifdef DEBUG_MODE
			   , "AdjustLR qq"
#endif
			   ) ;
	CheckQuart( &work->pre_adjust1
#ifdef DEBUG_MODE
			   , "AdjustLR pre_adjust1"
#endif
			   ) ;
#endif

	MT_QuatSlerp(&q,&(work->pre_adjust1),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust1),&q);

	MT_QuatToMat(&(am[1]),&q);


	vu0_Clrv0();
	vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_ONAKA].world.m[3][0]));

	fpu_CopyVector((FVECTOR *)&(am[1].m[3][0]),
				   (FVECTOR *)&(objs->objs[HUMAN21_ONAKA].world.m[3][0]));

	vu0_Subv1v0v1();

	vu0_Ldm0(&(am[1]));
	vu0_Mulv1m0v1();

	vu0_Stv1((FVECTOR *)&(am[1].m[3][0]));
}



#if 0
/* 10度 */
#define CALCADJUST_COS	0.984807753f
#define CALCADJUST_SIN	0.173648178f
#elif 1
/* 20度 */
#define CALCADJUST_COS	0.939692621f
#define CALCADJUST_SIN	0.342020143f
#else
/* 30度 */
#define CALCADJUST_COS	0.866025404f
#define CALCADJUST_SIN	0.5f
#endif


/* ある平面に投影したベクトルを算出するマトリクス
   ある平面の単位ベクトルとの内積をとり、
   その値を再度単位ベクトルと演算する */
static const FMATRIX im={
	{
		{ 1.0f, 0.0f,                          0.0f,                          0.0f, },
		{ 0.0f, 0.0f,                          CALCADJUST_SIN*CALCADJUST_COS, 0.0f, },
		{ 0.0f, CALCADJUST_SIN*CALCADJUST_COS, CALCADJUST_COS*CALCADJUST_COS, 0.0f, },
		{ 0.0f, 0.0f,                          0.0f,                          1.0f, },
	},
};

static inline void CalcAdjustUDW14A(Work *work,int flag,FVECTOR *vz,FMATRIX *am)
{
	DG_OBJS *objs=work->body.objs;
	FVECTOR vec0,vec1,vec2,q,qq;
	float param;

	if(flag){
		FMATRIX m;

		vu0_Ldm0(&(am[1]));
		vu0_Ldm1(&(objs->objs[HUMAN21_KUBI].world));
		vu0_Mulm2m0m1();
		vu0_Stm2(&m);

		fpu_CopyVector(&vec1,work->homing_p);
		fpu_CopyVector(&vec0,(FVECTOR *)&(m.m[3][0]));
		fpu_CopyVector(&vec2,vz);

		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);

		param=ADJUST_UD_PARAM_1;
	}
	else{
		fpu_ClearVector(&qq);
		qq.vw=1.0f;

		param=ADJUST_UD_PARAM_0;
	}

#ifdef KP_XBOX
	CheckQuart( &qq
#ifdef DEBUG_MODE
			   , "AdjustUD14a qq"
#endif
			   ) ;
	CheckQuart( &work->pre_adjust1
#ifdef DEBUG_MODE
			   , "AdjustUD14a pre_adjust1"
#endif
			   ) ;
#endif

	MT_QuatSlerp(&q,&(work->pre_adjust0),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust0),&q);

	MT_QuatToMat(&(am[0]),&q);


	vu0_Clrv0();
	vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_MUNE].world.m[3][0]));

	fpu_CopyVector((FVECTOR *)&(am[0].m[3][0]),
				   (FVECTOR *)&(objs->objs[HUMAN21_MUNE].world.m[3][0]));

	vu0_Subv1v0v1();

	vu0_Ldm0(&(am[0]));
	vu0_Mulv1m0v1();

	vu0_Stv1((FVECTOR *)&(am[0].m[3][0]));
}


/* 45/2度 */
#define QUAT_COS	0.923879533f


static inline void CalcAdjustLRW14A(Work *work,int flag,FVECTOR *vz,FMATRIX *am)
{
	DG_OBJS *objs=work->body.objs;
	FVECTOR vec0,vec1,vec2,q,qq;
	float param;

	if(flag){
		fpu_CopyVector(&vec1,work->homing_p);
		fpu_CopyVector(&vec0,(FVECTOR *)&(objs->world.m[3][0]));
		fpu_CopyVector(&vec2,(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));

		vec0.vy=0.0f;
		vec1.vy=0.0f;
		vec2.vy=0.0f;

		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);

		if(qq.vy<0.0f && qq.vw<QUAT_COS){
			vu0_Ldv0(work->homing_p);
			vu0_Ldv1((FVECTOR *)&(objs->world.m[3][0]));
			vu0_Ldv2((FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));
			vu0_Ldm0(&im);
			vu0_Mulv0m0v0();
			vu0_Mulv1m0v1();
			vu0_Mulv2m0v2();
			vu0_Stv0(&vec1);
			vu0_Stv1(&vec0);
			vu0_Stv2(&vec2);

			UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);
		}

		param=ADJUST_LR_PARAM_1;
	}
	else{
		fpu_ClearVector(&qq);
		qq.vw=1.0f;

		param=ADJUST_LR_PARAM_0;
	}

	MT_QuatSlerp(&q,&(work->pre_adjust1),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust1),&q);

#ifdef KP_XBOX
	CheckQuart( &q
#ifdef DEBUG_MODE
			   , "AdjustLR14a q"
#endif
			   ) ;
	CheckQuart( &work->pre_adjust1
#ifdef DEBUG_MODE
			   , "AdjustLR14a pre_adjust1"
#endif
			   ) ;
#endif


	MT_QuatToMat(&(am[1]),&q);

	vu0_Ldv2(vz);
	vu0_Ldm0(&(am[1]));
	vu0_Mulv2m0v2();
	vu0_Stv2(vz);


	vu0_Clrv0();
	vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_ONAKA].world.m[3][0]));

	fpu_CopyVector((FVECTOR *)&(am[1].m[3][0]),
				   (FVECTOR *)&(objs->objs[HUMAN21_ONAKA].world.m[3][0]));

	vu0_Subv1v0v1();

	vu0_Ldm0(&(am[1]));
	vu0_Mulv1m0v1();

	vu0_Stv1((FVECTOR *)&(am[1].m[3][0]));
}

void NPCSnake_CalcAdjust0(Work *work)
{
	FMATRIX *mat=SCRPAD_ADDR;

	fpu_CopyUnitMatrix(&(mat[0]));
	fpu_CopyUnitMatrix(&(mat[1]));

	if(work->homing_p==NULL){
		CalcAdjustLR(work,0,mat);
		CalcAdjustUD(work,0,mat);
		RecalcWorldMatrix(work,mat);
		return;
	}
	CalcAdjustLR(work,1,mat);
	CalcAdjustUD(work,1,mat);
	RecalcWorldMatrix(work,mat);
}

void NPCSnake_CalcAdjust(Work *work)
{
	FMATRIX *mat=SCRPAD_ADDR;

	fpu_CopyUnitMatrix(&(mat[0]));
	fpu_CopyUnitMatrix(&(mat[1]));

	if(work->homing_p==NULL){
		CalcAdjustLR(work,0,mat);
		CalcAdjustUD(work,0,mat);
		RecalcWorldMatrix(work,mat);
		return;
	}
	CalcAdjustLR(work,0,mat);
	CalcAdjustUD(work,1,mat);
	RecalcWorldMatrix(work,mat);
}

void NPCSnake_BackAdjust(Work *work)
{
	FMATRIX *mat=SCRPAD_ADDR;

	fpu_CopyUnitMatrix(&(mat[0]));
	fpu_CopyUnitMatrix(&(mat[1]));

	CalcAdjustLR(work,0,mat);
	CalcAdjustUD(work,0,mat);
	RecalcWorldMatrix(work,mat);
}

void NPCSnake_CalcAdjustW14A(Work *work)
{
	FMATRIX *mat=SCRPAD_ADDR;
	FVECTOR vz;

	fpu_CopyUnitMatrix(&(mat[0]));
	fpu_CopyUnitMatrix(&(mat[1]));

	fpu_CopyVector(&vz,(FVECTOR *)&(work->body.objs->world.m[2][0]));

	if(work->homing_p==NULL){
		CalcAdjustLRW14A(work,0,&vz,mat);
		CalcAdjustUDW14A(work,0,&vz,mat);
		RecalcWorldMatrix(work,mat);
		return;
	}
	CalcAdjustLRW14A(work,1,&vz,mat);
	CalcAdjustUDW14A(work,1,&vz,mat);
	RecalcWorldMatrix(work,mat);
}

void NPCSnake_CalcAdjustW32A(Work *work,int flag)
{
	DG_OBJS *objs=work->body.objs;
	DG_EVMOBJ *evm=work->body.evmobj;
	FMATRIX *evmmat;
	FVECTOR vec0,vec1,vec2,q,qq;
	FMATRIX m;
	int i;
	float param;


	if(evm!=NULL) evmmat=evm->matrix[evm->use_buffer];
	else evmmat=NULL;


	if(flag==0 || work->homing_p==NULL){
		fpu_ClearVector(&qq);
		qq.vw=1.0f;

		param=ADJUST_LR_PARAM_0;
	}
	else{
		fpu_CopyVector(&vec1,work->homing_p);
		fpu_CopyVector(&vec0,(FVECTOR *)&(objs->world.m[3][0]));
		fpu_CopyVector(&vec2,(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));

		vec0.vy=0.0f;
		vec1.vy=0.0f;
		vec2.vy=0.0f;

		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);

		param=ADJUST_LR_PARAM_1;
	}

	MT_QuatSlerp(&q,&(work->pre_adjust0),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust0),&q);

	MT_QuatToMat(&m,&q);


	vu0_Clrv0();
	vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_MUNE].world.m[3][0]));

	fpu_CopyVector((FVECTOR *)&(m.m[3][0]),
				   (FVECTOR *)&(objs->objs[HUMAN21_MUNE].world.m[3][0]));

	vu0_Subv1v0v1();

	vu0_Ldm0(&m);
	vu0_Mulv1m0v1();

	vu0_Stv1((FVECTOR *)&(m.m[3][0]));


	vu0_Ldm2(&m);

	for(i=HUMAN21_MUNE;i<=HUMAN21_ATAMA;i++){
		vu0_Ldm1(&(objs->objs[i].world));
		vu0_Mulm0m2m1();
		vu0_Stm0(&(objs->objs[i].world));
	}
	if(evmmat!=NULL){
		for(i=HUMAN21_MUNE;i<=HUMAN21_ATAMA;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
		for(i=HUMANEVM_FACESTART;i<HUMANEVM_FACEEND;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
	}
}


/* 頭と首を回す */
static inline void CalcAdjustHeadLR(Work *work,int flag)
{
	DG_OBJS *objs=work->body.objs;
	DG_EVMOBJ *evm=work->body.evmobj;
	FMATRIX *evmmat;
	FVECTOR vec0,vec1,vec2,q,qq;
	FMATRIX m;
	int i;
	float param;


	if(evm!=NULL) evmmat=evm->matrix[evm->use_buffer];
	else evmmat=NULL;

	if(flag){
		fpu_CopyVector(&vec1,&GM_PlayerPosition);
		fpu_CopyVector(&vec0,(FVECTOR *)&(objs->world.m[3][0]));
		fpu_CopyVector(&vec2,(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));

		vec0.vy=0.0f;
		vec1.vy=0.0f;
		vec2.vy=0.0f;

		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);

		/* 曲がり過ぎに注意 */
		if(qq.vw<0.0001f){
			fpu_ClearVector(&qq);
			qq.vw=1.0f;

			vec1.vx-=vec0.vx;
			vec1.vz-=vec0.vz;
			work->ctrl.turn.vy=GV_VecDir2(&vec1);
		}
		else if(qq.vw<COS_ADJUST_LR_LIMIT){
			float s=fpu_Rsqrt(1.0f-qq.vw*qq.vw,SIN_ADJUST_LR_LIMIT);
			qq.vx*=s;
			qq.vy*=s;
			qq.vz*=s;
			qq.vw=COS_ADJUST_LR_LIMIT;

			vec1.vx-=vec0.vx;
			vec1.vz-=vec0.vz;
			work->ctrl.turn.vy=GV_VecDir2(&vec1);
		}

		param=ADJUST_LR_PARAM_1;
	}
	else{
		fpu_ClearVector(&qq);
		qq.vw=1.0f;

		param=ADJUST_LR_PARAM_2;
	}

	MT_QuatSlerp(&q,&(work->pre_adjust2),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust2),&q);

	MT_QuatToMat(&m,&q);


	vu0_Clrv0();
	vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_KUBI].world.m[3][0]));

	fpu_CopyVector((FVECTOR *)&(m.m[3][0]),
				   (FVECTOR *)&(objs->objs[HUMAN21_KUBI].world.m[3][0]));

	vu0_Subv1v0v1();

	vu0_Ldm0(&m);
	vu0_Mulv1m0v1();

	vu0_Stv1((FVECTOR *)&(m.m[3][0]));


	vu0_Ldm2(&m);

	for(i=HUMAN21_KUBI;i<=HUMAN21_ATAMA;i++){
		vu0_Ldm1(&(objs->objs[i].world));
		vu0_Mulm0m2m1();
		vu0_Stm0(&(objs->objs[i].world));
	}

	if(evmmat!=NULL){
		for(i=HUMAN21_KUBI;i<=HUMAN21_ATAMA;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
		for(i=HUMANEVM_FACESTART;i<HUMANEVM_FACEEND;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
	}
}

/* 頭と首を回す */
static inline int CalcAdjustHeadLR2(Work *work)
{
	DG_OBJS *objs=work->body.objs;
	DG_EVMOBJ *evm=work->body.evmobj;
	FMATRIX *evmmat;
	FVECTOR vec0,vec1,vec2,q,qq;
	FMATRIX m;
	int i;
	float param;


	if(evm!=NULL) evmmat=evm->matrix[evm->use_buffer];
	else evmmat=NULL;


	fpu_CopyVector(&vec1,&(work->book_pos));
	fpu_CopyVector(&vec0,(FVECTOR *)&(objs->world.m[3][0]));
	fpu_CopyVector(&vec2,(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));

	UTL_MakeQuat(&qq,(FVECTOR *)&(objs->world.m[3][0]),
				 &(work->book_pos),(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));

	/* 曲がり過ぎに注意 */
	if(qq.vw<0.0001f){
		fpu_ClearVector(&qq);
		qq.vw=1.0f;

		vec1.vx-=vec0.vx;
		vec1.vz-=vec0.vz;
		work->ctrl.turn.vy=GV_VecDir2(&vec1);
	}
	else if(qq.vw<COS_ADJUST_UDLR_LIMIT){
		float s=fpu_Rsqrt(1.0f-qq.vw*qq.vw,SIN_ADJUST_UDLR_LIMIT);
		qq.vx*=s;
		qq.vy*=s;
		qq.vz*=s;
		qq.vw=COS_ADJUST_LR_LIMIT;

		vec1.vx-=vec0.vx;
		vec1.vz-=vec0.vz;
		work->ctrl.turn.vy=GV_VecDir2(&vec1);
	}

	param=ADJUST_LR_PARAM_1;


	MT_QuatSlerp(&q,&(work->pre_adjust2),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust2),&q);

	MT_QuatToMat(&m,&q);


	vu0_Clrv0();
	vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_KUBI].world.m[3][0]));

	fpu_CopyVector((FVECTOR *)&(m.m[3][0]),
				   (FVECTOR *)&(objs->objs[HUMAN21_KUBI].world.m[3][0]));

	vu0_Subv1v0v1();

	vu0_Ldm0(&m);
	vu0_Mulv1m0v1();

	vu0_Stv1((FVECTOR *)&(m.m[3][0]));


	vu0_Ldm2(&m);

	for(i=HUMAN21_KUBI;i<=HUMAN21_ATAMA;i++){
		vu0_Ldm1(&(objs->objs[i].world));
		vu0_Mulm0m2m1();
		vu0_Stm0(&(objs->objs[i].world));
	}

	if(evmmat!=NULL){
		for(i=HUMAN21_KUBI;i<=HUMAN21_ATAMA;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
		for(i=HUMANEVM_FACESTART;i<HUMANEVM_FACEEND;i++){
			vu0_Ldm1(&(evmmat[i]));
			vu0_Mulm0m2m1();
			vu0_Stm0(&(evmmat[i]));
		}
	}

	return 1;
}

void NPCSnake_ClearAdjustW43A(Work *work)
{
	fpu_ClearVector(&(work->pre_adjust2));
	work->pre_adjust2.vw=1.0f;
}

void NPCSnake_CalcAdjustW43A(Work *work)
{
	if(work->snake_status & SNAKE_STATUS_CALCADJUST_BOOK){
		if(CalcAdjustHeadLR2(work)) return;
	}
	CalcAdjustHeadLR(work,1);
}

void NPCSnake_BackAdjustW43A(Work *work)
{
	CalcAdjustHeadLR(work,0);
}


/* cos 45度 */
#define COS_ADJUST_LR_LIMIT2		0.707106781f


static int CheckBook(Work *work)
{
	ENEFINDLIST	*efl;
	ENEFIND *ef,*tef;

	efl = &GM_EneFindList ;

	if( efl->start == NULL ) return 0;

	ef = efl->start ;
	while( ef != NULL ) {
		float c,len;

		//printf("EF ef->id[%d]  ef->type[%x]\n",ef->id, ef->type)  ;

		tef=ef;
		ef=ef->next;

		if(tef->type & EF_TYPE_FOUND) continue;

		if(tef->type & EF_TYPE_ADULT){
			FVECTOR vec0,vec1,vec2;
			DG_OBJS *objs=work->body.objs;

			fpu_CopyVector(&vec1,&(tef->pos));
			fpu_CopyVector(&vec0,&(work->ctrl.mov));
			fpu_CopyVector(&vec2,(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));

			vec0.vy=0.0f;
			vec1.vy=0.0f;
			vec2.vy=0.0f;

			vu0_Ldv0(&vec1);
			vu0_Ldv1(&vec0);
			vu0_Ldv2(&vec2);

			vu0_Subv0v1();
			c=vu0_InnerProductv0v2();
			len=vu0_VectorLength2v0();

			if(len>CVM2N(2.5f)*CVM2N(2.5f)) continue;

			c=fpu_Rsqrt(len,c);

			if(c<COS_ADJUST_LR_LIMIT2) continue;

			tef->type|=EF_TYPE_FOUND;

			fpu_CopyVector(&(work->book_pos),&(tef->pos));

			return 1;
		}
	}

	return 0;
}

void NPCSnake_CheckBook(Work *work)
{
	static const int voice_book[]={
		// ★ライデンがスネークを本当に撃つとスネークも撃ち返す。
		VOICE_INDEX_SHOOT_GUN0,
		// スネーク　「馬鹿者」
		VOICE_INDEX_SHOOT_GUN1,
		// スネーク　「ふざけるな」
		VOICE_INDEX_SHOOT_GUN2,
		// スネーク　「何を考えている」
	};

	if(work->book_interval>0){
		if(work->book_count<DIRECT_TICK(0.6f*60.0f)){
			work->snake_status|=SNAKE_STATUS_CALCADJUST_BOOK;
		}
		else if(work->book_count<DIRECT_TICK(3.0f*60.0f)){
			if(irnd()<(unsigned int)(0.001f*((float)ULONG_MAX+1.0f))){
				work->book_count=0;
				work->snake_status|=SNAKE_STATUS_CALCADJUST_BOOK;
			}
		}
		else{
			if(irnd()<(unsigned int)(0.05f*((float)ULONG_MAX+1.0f))){
				work->book_count=0;
				work->snake_status|=SNAKE_STATUS_CALCADJUST_BOOK;
			}
		}
		work->book_count++;
	}
	else{
		if(CheckBook(work)){
			work->book_count=0;
			work->book_interval=DIRECT_TICK(5.0f*60.0f);

			work->snake_status|=SNAKE_STATUS_CALCADJUST_BOOK;

			NPCSNAKE_VOICE_RNDCALL(work,voice_book,sizeof(voice_book)/sizeof(voice_book[0]));
		}
	}
}


/* ------------------------------------------------------------------------ */
/* Set Radar Direction                                                      */
/* ------------------------------------------------------------------------ */


void NPCSnake_SetRadarDir(Work *work)
{
	NPCWORK *npc=&(work->npc);
	DG_OBJS *objs=work->body.objs;

	switch(work->stage){
	case NPCSNAKE_STAGE_W43A:
	case NPCSNAKE_STAGE_W44A:
	case NPCSNAKE_STAGE_W45A:
	case NPCSNAKE_STAGE_BETWEEN_STAGES:
		if(work->snake_status & SNAKE_STATUS_SYS_DAMAGED){
			NPC_ActStatus(&(npc->action),NPC_ACT_STATUS_EYE_CLOSE);
		}
		break;
	}
	if(work->snake_status2 & SNAKE_STATUS2_SIGHT_CLOSE){
		NPC_ActStatus(&(npc->action),NPC_ACT_STATUS_EYE_CLOSE);
	}

	npc->action.face_dir=GV_VecDir2((FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));
}


/* ------------------------------------------------------------------------ */
/* Throw Item                                                               */
/* ------------------------------------------------------------------------ */


static void ThrowItemBox(Work *work,int spec,int id,int n)
{
	static const int voice_item[]={
		// アイテムをなげてよこす時
		VOICE_INDEX_THROW_ITEM0,
		// スネーク　　「これを使え！」
		VOICE_INDEX_THROW_ITEM1,
		// スネーク　　「ライデン、受け取れ！」
		VOICE_INDEX_THROW_ITEM2,
		// スネーク　　「ライデン、アイテムだ！」
	};

	FVECTOR pos;
	FVECTOR	force;
	int code_box,code_box_sh,code_label;
	float len;

	fpu_CopyVector(&pos,(FVECTOR *)&(work->body.objs->objs[HUMAN21_ONAKA].world.m[3][0]));

	force.vx=GM_PlayerPosition.vx-pos.vx;
	force.vz=GM_PlayerPosition.vz-pos.vz;

	len=force.vx*force.vx+force.vz*force.vz;

	len=fpu_Rsqrt(len,1.0f);

#if 0
	force.vx = vu0_SinS(work->ctrl.rot.vy);
	force.vy = 1.0f;
	force.vz = vu0_CosS(work->ctrl.rot.vy);
#else
	force.vx*=len;
	force.vy=1.0f;
	force.vz*=len;
#endif

	switch(id){
	case IT_Ration:
		code_box=GV_StrCode("ration_ibox");
		code_box_sh=GV_StrCode("ration_ibox_sh");
		code_label=GV_StrCode("rtn_label");
		break;
	case WP_Socom:
		code_box=GV_StrCode("handgun_amo_ibox");
		code_box_sh=GV_StrCode("handgun_amo_ibox_sh");
		code_label=GV_StrCode("scm_amo_label");
		break;
	case WP_m4:
		code_box=GV_StrCode("rifle_amo_ibox");
		code_box_sh=GV_StrCode("rifle_amo_ibox_sh");
		code_label=GV_StrCode("m4_amo_label");
		break;
	case WP_Aks:
		code_box=GV_StrCode("rifle_amo_ibox");
		code_box_sh=GV_StrCode("rifle_amo_ibox_sh");
		code_label=GV_StrCode("aks_amo_label");
		break;
	case WP_Rgb6:
		code_box=GV_StrCode("launcher_amo_ibox");
		code_box_sh=GV_StrCode("launcher_amo_ibox_sh");
		code_label=GV_StrCode("rgb_amo_label");
		break;
	default:
		return;
	}

	if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
		if(force.vz>=0.0f) force.vz=-force.vz;
	}

	NewItemBox_Prog(0,GM_CurrentStageMap,0,
					code_box,code_box_sh,code_label,
					&pos,300.0f,
					spec,id,n,2,&force);

	NPCSNAKE_VOICE_RNDCALL(work,voice_item,
						   sizeof(voice_item)/sizeof(voice_item[0]));

	work->throwitem_count=THROW_ITEM_COUNT;
}

void NPCSnake_ThrowItem(Work *work)
{
	static const int voice_worry[]={
		// ライデンのＬＩＦＥが少ない
		VOICE_INDEX_WORRY0,
		// スネーク　　「大丈夫か」
		VOICE_INDEX_WORRY1,
		// スネーク　　「無理するな！」
	};

	if(work->stage!=NPCSNAKE_STAGE_W44A &&
	   work->stage!=NPCSNAKE_STAGE_W45A) return;

	if(work->throwitem_count>0){
		work->throwitem_count--;
		return;
	}

	if(work->snake_status2 & SNAKE_STATUS2_W44A_FINAL_FIGHT){
		/* w44aでは透明壁が存在するのでそこより奥でアイテムを投げる */
		if(work->ctrl.mov.vz>POSZ_DISABLE_THROW_ITEM) return;
	}

	if(work->mode==NPCSNAKE_MODE_FOLLOW_RAIDEN ||
	   work->mode==NPCSNAKE_MODE_PICKUP_RAIDEN
	   /* ||  work->mode==NPCSNAKE_MODE_SEARCHENEMY */ ) return;

	if(NPCSNAKE_SHOOT_MODE(work)==NPCSNAKE_SHOOT_NONE &&
	   (NPCSNAKE_ACT_MODE(work)!=NPCSNAKE_ACT_STAY ||
		(NPCSNAKE_HIDE_MODE(work)!=NPCSNAKE_HIDE_ATTACK &&
		 NPCSNAKE_HIDE_MODE(work)!=NPCSNAKE_HIDE_PEEPING &&
		 NPCSNAKE_HIDE_MODE(work)!=NPCSNAKE_HIDE_BACK_TO_HIDE))){

		/* レーションを出すかどうかの判定 */
		if(work->snake_status2 & SNAKE_STATUS2_THROWED_RATION){
			if(!(work->raiden_status & NPCSNAKE_RAIDEN_LIFE_DANGER)){
				work->snake_status2&=~SNAKE_STATUS2_THROWED_RATION;

				DEBUG_PRINT_NPCSNAKE("Ready to Throw Ration\n");
			}
		}
		else{
			if(work->raiden_status & NPCSNAKE_RAIDEN_LIFE_DANGER){
				work->snake_status2|=SNAKE_STATUS2_THROWED_RATION;

				if(work->ration_count!=0 &&
				   GM_ItemNum(IT_Ration)<3 &&
				   GM_ItemNum(IT_Ration)<GM_ItemMaxNum(IT_Ration)){

					ThrowItemBox(work,GM_ITEM_BOX,IT_Ration,1);

					if(work->ration_count>0) work->ration_count--;

					DEBUG_PRINT_NPCSNAKE("Throwed Ration\n");
				}
				else{
					NPCSNAKE_VOICE_RNDCALL(work,voice_worry,
										   sizeof(voice_worry)/sizeof(voice_worry[0]));
				}
				return;
			}
		}

		/* SOCOMの弾を出すかどうかの判定 */
		if(work->snake_status2 & SNAKE_STATUS2_THROWED_SOCOMS_BULLET){
			if(GM_WeaponNum(WP_Socom)>=MAX_BULLETS_IN_SOCOM){

				work->snake_status2&=~SNAKE_STATUS2_THROWED_SOCOMS_BULLET;

				DEBUG_PRINT_NPCSNAKE("Ready to Throw Socom\'s bullet\n");
			}
		}
		else{
			if(PL_GetPlayerWeapon()==WP_Socom &&
			   GM_WeaponNum(WP_Socom)<MAX_BULLETS_IN_SOCOM &&
			   work->usp_mag_count!=0){

				work->snake_status2|=SNAKE_STATUS2_THROWED_SOCOMS_BULLET;

				ThrowItemBox(work,GM_WEAPON_BOX,WP_Socom,MAX_BULLETS_IN_SOCOM);

				if(work->usp_mag_count>0) work->usp_mag_count--;

				DEBUG_PRINT_NPCSNAKE("Throwed Socom\'s bullet\n");

				return;
			}
		}

		/* M4の弾を出すかどうかの判定 */
		if(work->snake_status2 & SNAKE_STATUS2_THROWED_M4S_BULLET){
			if(GM_WeaponNum(WP_m4)>=MAX_BULLETS_IN_M4){

				work->snake_status2&=~SNAKE_STATUS2_THROWED_M4S_BULLET;

				DEBUG_PRINT_NPCSNAKE("Ready to Throw M4\'s bullet\n");
			}
		}
		else{
			if(PL_GetPlayerWeapon()==WP_m4 &&
			   GM_WeaponNum(WP_m4)<MAX_BULLETS_IN_M4 &&
			   work->m4_mag_count!=0){

				work->snake_status2|=SNAKE_STATUS2_THROWED_M4S_BULLET;

				ThrowItemBox(work,GM_WEAPON_BOX,WP_m4,MAX_BULLETS_IN_M4);

				if(work->m4_mag_count>0) work->m4_mag_count--;

				DEBUG_PRINT_NPCSNAKE("Throwed M4\'s bullet\n");

				return;
			}
		}

#if 0
		/* AKの弾を出すかどうかの判定 */
		if(work->snake_status2 & SNAKE_STATUS2_THROWED_AKS_BULLET){
			if(GM_WeaponNum(WP_Aks)>=MAX_BULLETS_IN_AKS){

				work->snake_status2&=~SNAKE_STATUS2_THROWED_AKS_BULLET;

				DEBUG_PRINT_NPCSNAKE("Ready to Throw AKS\'s bullet\n");
			}
		}
		else{
			if(PL_GetPlayerWeapon()==WP_Aks &&
			   GM_WeaponNum(WP_Aks)<MAX_BULLETS_IN_AKS){

				work->snake_status2|=SNAKE_STATUS2_THROWED_AKS_BULLET;

				ThrowItemBox(work,GM_WEAPON_BOX,WP_Aks,MAX_BULLETS_IN_AKS);

				DEBUG_PRINT_NPCSNAKE("Throwed AKS\'s bullet\n");

				return;
			}
		}
#endif

		/* RGB6の弾を出すかどうかの判定 */
		if(work->snake_status2 & SNAKE_STATUS2_THROWED_RGB6_BULLET){
			if(GM_WeaponNum(WP_Rgb6)>=MAX_BULLETS_IN_RGB6){

				work->snake_status2&=~SNAKE_STATUS2_THROWED_RGB6_BULLET;

				DEBUG_PRINT_NPCSNAKE("Ready to Throw RGB6\'s bullet\n");
			}
		}
		else{
			if(PL_GetPlayerWeapon()==WP_Rgb6 &&
			   GM_WeaponNum(WP_Rgb6)<MAX_BULLETS_IN_RGB6 &&
			   work->rgb6_mag_count!=0){

				work->snake_status2|=SNAKE_STATUS2_THROWED_RGB6_BULLET;

				ThrowItemBox(work,GM_WEAPON_BOX,WP_Rgb6,MAX_BULLETS_IN_RGB6);

				if(work->rgb6_mag_count>0) work->rgb6_mag_count--;

				DEBUG_PRINT_NPCSNAKE("Throwed RGB6\'s bullet\n");

				return;
			}
		}
	}
}


/* ------------------------------------------------------------------------ */
/* Breath                                                                   */
/* ------------------------------------------------------------------------ */


#define MAGIC_RATE(x)				((x)*3/4)


void NPCSNAKE_BreathControl(Work *work)
{
	static FVECTOR shift={
		0.0f,-36.0f,107.0f,1.0f,
	};
	static int sw=1;

	if(work->breath_interval){
		if(work->snake_status2 & SNAKE_STATUS2_AUTO_BREATH){
			MT3_CONTROL *mt3=work->body.m_ctrl->mt3_ctrl;

			if(work->breath_count){
				if(mt3->play_time>MAGIC_RATE(mt3->motion_total_time)){
					/* 息吐き出し */
					work->breath_count=0;

					NewBreath(&(work->body.objs->objs[HUMAN21_ATAMA].world),
							  &shift,&sw);
					{
						extern void *NewSnakeBreathOnCamera2( FMATRIX *world );
						FMATRIX	fmat;
						DG_COPY_MAT( &fmat, &work->body.objs->objs[HUMAN21_ATAMA].world );
						DG_SetPos( &fmat ) ;
						DG_PutVector( &shift, (FVECTOR *)fmat.m[3], 1) ;
						NewSnakeBreathOnCamera2( &fmat );
					}
				}
			}
			else{
				if(mt3->play_time<MAGIC_RATE(mt3->motion_total_time)){
					work->breath_count=1;
				}
			}
		}
		else{
			work->breath_count++;
			if(work->breath_count>work->breath_interval){
				/* 息吐き出し */
				work->breath_count=0;

				NewBreath(&(work->body.objs->objs[HUMAN21_ATAMA].world),
						  &shift,&sw);

				{
					extern void *NewSnakeBreathOnCamera2( FMATRIX *world );
					FMATRIX	fmat;
					DG_COPY_MAT( &fmat, &work->body.objs->objs[HUMAN21_ATAMA].world );
					DG_SetPos( &fmat ) ;
					DG_PutVector( &shift, (FVECTOR *)fmat.m[3], 1) ;
					NewSnakeBreathOnCamera2( &fmat );
				}

			}
		}
	}
}


/* ------------------------------------------------------------------------ */
/* ZZZ Sound                                                                */
/* ------------------------------------------------------------------------ */


#define ZZZ_MAGIC_RATE(x)		((x)*1/2)

enum {
	X=0,
	Y,
	Z,
	W,
};

void NPCSNAKE_ZZZSound(Work *work)
{
	if(work->snake_status2 & SNAKE_STATUS2_ZZZ_SOUND){
		MT3_CONTROL *mt3=work->body.m_ctrl->mt3_ctrl;

		if(work->zzz_sound_count){
			if(mt3->play_time>ZZZ_MAGIC_RATE(mt3->motion_total_time)){
				/* 胸しぼみ */
				work->zzz_sound_count=0;

				GM_SeSetMode(SD_V_SNASLP01,&(work->ctrl.mov),GM_SEMODE_NORMAL);

				DEBUG_PRINT_NPCSNAKE("ZZZ Sound Call !\n");
			}
		}
		else{
			if(mt3->play_time<ZZZ_MAGIC_RATE(mt3->motion_total_time)){
				/* 胸膨らみ */
				work->zzz_sound_count=1;
			}
		}
	}
	else{
		work->zzz_sound_count=0;
	}
}


/* ------------------------------------------------------------------------ */
/* Evm Close Eye                                                            */
/* ------------------------------------------------------------------------ */


#define sna_eye_close		0
#define sna_eye_pachi		1


void NPCSNAKE_EvmCloseEye(Work *work)
{
	GV_MSG msg;
	int message[6];

	if(work->npc.action.current_mar==work->cap_marfile &&
	   (work->npc.action.current_mot==NPC_MOT_DOWN_F ||
		work->npc.action.current_mot==NPC_MOT_DOWN_B ||
		work->npc.action.current_mot==NPC_MOT_HANG_DIE ||
		work->npc.action.current_mot==NPC_MOT_HANG_RELEASE_DOWN)){

		/* 首絞めによりダウンした場合の処理 */
		work->snake_status2|=SNAKE_STATUS2_EVM_CLOSE_EYE;
	}

	msg.address=NPCSNAKE_FACEANIM_ACTOR;
	msg.message=message;
	msg.message_len=sizeof(message)/sizeof(message[0]);

	if(work->snake_status2 & SNAKE_STATUS2_EVM_CLOSE_EYE){
		message[0]=0;
		message[1]=sna_eye_close;

		work->eye_flush_count=0;

		GV_SendMessage(&msg);
	}
	else if(work->snake_status2 & SNAKE_STATUS2_EVM_FLUSH_EYE){
		message[0]=0;
		message[1]=sna_eye_pachi;

		work->eye_flush_count=0;

		GV_SendMessage(&msg);
	}
	else{
		work->eye_flush_count++;
		if(work->eye_flush_count>NPCSNAKE_EYE_FLUSH_INTERVAL){
			message[0]=0;
			message[1]=sna_eye_pachi;

			work->eye_flush_count=0;

			GV_SendMessage(&msg);
		}
	}

	if(!(work->snake_status & SNAKE_STATUS_SYS_DAMAGED)){
		work->snake_status2&=~(SNAKE_STATUS2_EVM_CLOSE_EYE|SNAKE_STATUS2_EVM_FLUSH_EYE);
	}
}


/* ------------------------------------------------------------------------ */
/* Voice Control                                                            */
/* ------------------------------------------------------------------------ */


void NPCSnake_VoiceStop(Work *work)
{
	if(work->voice_handle>0){
		GM_StreamStop(work->voice_handle);

		work->voice_handle=0;
	}
}

void NPCSnake_VoiceControl(Work *work)
{
	if(work->snake_status2 & SNAKE_STATUS2_VOICE){
		/* 音声のコール */
		if(work->voice_handle<=0){
//			work->voice_handle=GM_VoxStream(work->voice_num,0);
			work->voice_handle=GM_VoxStream(work->voice_num, GM_STREAM_FLAG_3D );
		}
	}

    if(work->voice_handle>0){
		int vol,pan;
      float bp_angle;

		DEMO_MOTION *mtn ;
		FMATRIX  *skel_mats, *mats ;
		EVM_SKEL *skel ;
		FVECTOR  *rot, *trans, vec ;
		int i;
		extern DEMO_MOTION * GM_StreamGetMotion( int id ) ;


        /* 再生が終っていれば ハンドラを使っていない状態に戻す*/
        if(GM_StreamStatus(work->voice_handle)==GM_STREAM_STATE_END){
			work->voice_handle=0;
			goto next;
		}

        GM_SeGetVolPan(&work->ctrl.mov,GM_SEMODE_BOMB,&vol,&pan,&bp_angle);
//        GM_VoxStreamSetPan(work->voice_handle,vol,pan);
        GM_VoxStreamSetParam(work->voice_handle, &work->ctrl.mov, work->ctrl.addr, vol,pan, bp_angle);

		if(work->body.evmobj==NULL) goto next;

		mtn=GM_StreamGetMotion(GV_StrCode("プリスキン"));
		if(mtn==NULL){
			mtn=GM_StreamGetMotion(GV_StrCode("スネーク"));
		}
		if(mtn==NULL) goto next;

		/* 現在のフレームのモーションを得る */
		/* 顔の全関節の値を設定する */
		skel_mats=work->body.evmobj->matrix[work->body.evmobj->use_buffer];
		skel=work->body.evmobj->def->skeleton+21;
		vec.vw=1.0F;

		for( i=0 ; i<32 ; i++, skel++ ){
			mats = &skel_mats[i + 21] ;

			/* クォータニオンとトランスレーションの取得 */
			rot   = &mtn->motion[i*2  ] ;
			trans = &mtn->motion[i*2+1] ;
			/* ローカルマトリックスを作成 */
			MT_QuatToMat( mats, rot ) ;
			mats->m[W][X] = skel->rt_tx + trans->vx ;
			mats->m[W][Y] = skel->rt_ty + trans->vy ;
			mats->m[W][Z] = skel->rt_tz + trans->vz ;
			/* 親を掛け ワールドマトリックスに変換 */
			_sceVu0MulMatrix( mats, &skel_mats[ skel->parent ], mats ) ;

			vec.vx = -skel->rt_tx;
			vec.vy = -skel->rt_ty;
			vec.vz = -skel->rt_tz;
			_sceVu0ApplyMatrix( (FVECTOR *)mats->m[W], mats, &vec ) ;
		}
	}

next:
	work->snake_status2&=~SNAKE_STATUS2_VOICE;
}
