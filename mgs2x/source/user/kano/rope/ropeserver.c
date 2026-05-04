//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ropeserver.c
		ロープ(紐)シミュレート

	1999/09/21 K.Kano
	$Id: ropeserver.c,v 1.1.1.3 2002/11/19 11:43:35 Yoshizawa1 Exp $
*/


#include "rope.h"

#include "../../mode/demo/libdemo.h"


typedef struct {
    GV_ACT		actor ;		/* リンク */

	int			size;

    DG_OBJS		**objs;
    FMATRIX		*light;
    MODEL_PARAMETER		*mp;
} Work;


static Work *allocated_work=NULL;


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void Act(Work *work)
{
	HZX_GROUP_ID hzx_id=HZX_CurrentGroupID;
    DG_OBJS		**objs=work->objs;
    MODEL_PARAMETER		*mp=work->mp;
	int size=work->size;
	int i;
	int init_flag;

	init_flag=(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT));

	for(i=0;i<size;i++,objs++,mp++){
		if(*objs==NULL) continue;

		if(mp->target!=NULL){
			if(mp->visible_flag){
				DG_VisibleObjs(*objs);
			}
			else if(mp->target->objs->flag & DG_FLAG_INVISIBLE){
				DG_InvisibleObjs(*objs);
			}
			else if(mp->target->objs->objs[mp->tobjnum].flag & DG_FLAG_INVISIBLE){
				DG_InvisibleObjs(*objs);
			}
			else{
				DG_VisibleObjs(*objs);
			}
		}

		if(init_flag) InitLocate(*objs,mp);

		MoveRope3(*objs,hzx_id,mp);
	}
}

static void Die(Work *work)
{
    DG_OBJS		**objs=work->objs;
    MODEL_PARAMETER		*mp=work->mp;
	int size=work->size;
	int i;

	if(objs==NULL || mp==NULL) return;

	for(i=0;i<size;i++,objs++,mp++){
		if(*objs==NULL) continue;

		ExitRope(*objs,mp);
		DG_DequeueObjs(*objs);
		DG_FreeObjs(*objs);
    }

	GV_Free(work->light);

	allocated_work=NULL;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static int GetResources(Work *work)
{
	int tsize;

	work->size=16;

    if(GCL_GetOption('s')!=NULL){
		int s=GCL_GetNextInt();
		work->size=s;
	}

	tsize=(sizeof(DG_OBJS *)+sizeof(MODEL_PARAMETER)+sizeof(FMATRIX)*2)*work->size;

	if((work->light=(FMATRIX *)GV_Malloc(tsize))==NULL){
		return 0;
	}
	memset(work->light,0x00,tsize);

	work->mp=(MODEL_PARAMETER *)(work->light+work->size*2);
	work->objs=(DG_OBJS **)(work->mp+work->size);

	return 1;
}

void *NewRopeServer(void)
{
    Work *work ;

	if(allocated_work!=NULL) return NULL;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));

    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die);

		if(!GetResources(work)){
			GV_DestroyActor(work);
			return NULL;
		}

		allocated_work=work;
    }
    return (void *)work;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


typedef struct {
	GV_ACT	actor;
} DummyWork;

static void NullAct(DummyWork *work)
{
	GV_DestroyActor(work);
}

static void NullDie(DummyWork *work)
{
}


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


extern const SAMPLE_MODEL_PARAMETER rope_sample[];


/* オブジェクトを初期化 */
static DG_OBJS *InitObject(int name)
{
    DG_DEF *def ;
	DG_OBJS *objs;

    def = (DG_DEF*) GV_GetCache( GV_CacheID( name, 'k' ) );
    if((objs = DG_MakeObjs( def, OBJECT_FLAG, 0 ))==NULL) return NULL;
    DG_QueueObjs( objs );

    return objs;
}

/* 初期設定値を取得 */
static void GetOptionValue(DG_OBJS *objs,FMATRIX *light,MODEL_PARAMETER *mp)
{
    FVECTOR x;
    SVECTOR rot;

    mp->m=0.001f;
    mp->inv_m=1.0f/mp->m;
    mp->pa=0.001f;
    mp->k=-0.0001f;
    mp->we=-1.0f;
    mp->wl=(int)CVC2N(10);

    mp->param_oval=1.2f;

    mp->collision_flag=0;
    mp->visible_flag=0;
	mp->mode=0;

    mp->tmat=NULL;
    fpu_CopyUnitMatrix(&(mp->root));
    fpu_ClearVector(&x);
    rot=DG_ZeroSVector;

    mp->target=NULL;
    mp->tobjnum=0;

    mp->boundmodel=NULL;

	mp->mov_rate=0.0f;
	mp->ymov_rate=0.0f;

    if(GCL_GetOption('d')!=NULL){
		int d=GCL_GetNextInt();

		mp->m=rope_sample[d].m;
		mp->inv_m=1.0f/mp->m;
		mp->pa=rope_sample[d].pa;
		mp->k=-rope_sample[d].k;
		mp->we=-rope_sample[d].we;
		mp->wl=rope_sample[d].wl;
    }

    /* 初期位置 */
    if( GCL_GetOption( 'x' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV( GCL_NextStr(), buf );
		vu0_IV0toFV((IVECTOR *)buf, &x);
    } 
    x.vw=1.0f;

    /* 特定charaの特定objに付いていく */
    if( GCL_GetOption( 'y' ) != NULL ){
		CONTROL *ctrl;
		int name;

		name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
		if(ctrl!=NULL){
			mp->target=(OBJECT *)(ctrl+1);
		}
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		mp->tobjnum=GCL_GetNextInt();
    }
    if(mp->target!=NULL){
		mp->tmat=&(mp->target->objs->objs[mp->tobjnum].world);
		DG_SetLightMatrix(objs,mp->target->objs->light);
    }
    else{
		DG_GetLightMatrix(&x,light);
		DG_SetLightMatrix(objs,light);
    }

    if(GCL_GetOption('b')!=NULL){
		int modelnum=GCL_GetNextInt();
		mp->boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(modelnum,'k'));
    }

    /* 質量 */
    if( GCL_GetOption('m') != NULL ){
		int m=GCL_GetNextInt();
		float fm,inv_fm;

		/* グラム単位 */
		fm=(float)m*0.001f;
		inv_fm=1.0f/fm;

		mp->m=fm;
		mp->inv_m=inv_fm;
    }

    /* 風によって受ける力 */
    if( GCL_GetOption('p') != NULL ){
		int p=GCL_GetNextInt();
		float fp;

		fp=(float)p*0.000001f;

		mp->pa=fp;
    }

    /* 速度に対する抵抗値 */
    if( GCL_GetOption('k') != NULL ){
		int k=GCL_GetNextInt();
		float fk;

		fk=(float)k*0.000001f;
		mp->k=-fk;
    }

    /* 壁に対する反発係数 */
    if( GCL_GetOption('e') != NULL ){
		int e=GCL_GetNextInt();
		float fe;

		fe=(float)e*0.001f+1.0f;
		mp->we=-fe;
    }

    /* 壁検出距離 */
    if( GCL_GetOption('l') != NULL ){
		int l=GCL_GetNextInt();
		mp->wl=l;
    }

    /* 楕円球当たりのパラメータ */
    if( GCL_GetOption('o') != NULL ){
		int o=GCL_GetNextInt();
		float fo;

		fo=(float)o*0.01f;
		mp->param_oval=fo;
    }

    /* 当たり判定フラグ */
    if( GCL_GetOption('f') != NULL ){
		int f=GCL_GetNextInt();
        mp->collision_flag=(f!=0);
    }

    /* 強制表示フラグ */
    if( GCL_GetOption('v') != NULL ){
        mp->visible_flag=1;
    }

    /* バンダナモードか髪の毛モードか選択 */
    if( GCL_GetOption('h') != NULL ){
        mp->mode=GCL_GetNextInt();
    }

    if( GCL_GetOption( 'r' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		rot.vx=buf[0];
		rot.vy=buf[1];
		rot.vz=buf[2];
    }
    DG_SetPos(&DG_UnitMatrix);
    DG_RotatePosZYX(&rot);
    DG_GetPos(&(mp->root));

    fpu_CopyVector((FVECTOR *)&(mp->root.m[3][0]),&x);
}

/* 資源を獲得 */
static void GetAndRegistResources(void)
{
	int size=allocated_work->size;
	int i;

	for(i=0;i<size;i++){
		if(*(allocated_work->objs+i)==NULL) break;
	}
	if(i==size) return;

	{
		int name;
		DG_OBJS		**objs=allocated_work->objs+i;
		MODEL_PARAMETER		*mp=allocated_work->mp+i;
		FMATRIX		*light=allocated_work->light+i*2;

		if(!Rope_GetModelName(&name)){
			return;
		}
		if(!(*objs=InitObject(name))){
			return;
		}
		GetOptionValue(*objs,light,mp);
		if(!InitRope(*objs,mp)){
			DG_QueueObjs(*objs);
			DG_FreeObjs(*objs);
			*objs=NULL;
			return;
		}
		InitLocate(*objs,mp);
	}
}

void *NewRegistRope(void)
{
    DummyWork *work;

    work=(DummyWork *)GV_NewActor(GV_ACTOR_USER,sizeof(DummyWork));
    if(work!=NULL) {
		GV_SetActor(&(work->actor),NullAct,NullDie);
		GetAndRegistResources();
    }
    return (void *)work;
}


/* 初期設定値を取得 */
static void GetOptionValue_called(DG_OBJS *objs,FMATRIX *light,MODEL_PARAMETER *mp,
								  int model_name,int sample_num,
								  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
								  float oval_param,int collision_flag,int visible_flag,int mode)
{
    FVECTOR px;

    mp->m=0.001f;
    mp->inv_m=1.0f/mp->m;
    mp->pa=0.001f;
    mp->k=-0.0001f;
    mp->we=-1.0f;
    mp->wl=(int)CVC2N(10);

    mp->param_oval=1.2f;

    mp->collision_flag=0;
    mp->visible_flag=0;
	mp->mode=0;

    mp->tmat=NULL;
    fpu_CopyUnitMatrix(&(mp->root));
    fpu_ClearVector(x);

    mp->target=NULL;
    mp->tobjnum=0;

    mp->boundmodel=NULL;

	mp->mov_rate=0.0f;
	mp->ymov_rate=0.0f;

    mp->m=rope_sample[sample_num].m;
    mp->inv_m=1.0f/mp->m;
    mp->pa=rope_sample[sample_num].pa;
    mp->k=-rope_sample[sample_num].k;
    mp->we=-rope_sample[sample_num].we;
    mp->wl=rope_sample[sample_num].wl;


    /* 初期位置 */
    if(x!=NULL){
		fpu_CopyVector(&px,x);
    } 
    px.vw=1.0f;

    /* 特定charaの特定objに付いていく */
    mp->target=target;
    mp->tobjnum=objnum;

    if(mp->target!=NULL){
		mp->tmat=&(mp->target->objs->objs[mp->tobjnum].world);
		DG_SetLightMatrix(objs,mp->target->objs->light);
    }
    else{
		DG_GetLightMatrix(&px,light);
		DG_SetLightMatrix(objs,light);
    }


    /* 楕円球当たりのパラメータ */
    mp->param_oval=oval_param;

    /* 当たり判定フラグ */
    mp->collision_flag=(collision_flag!=0);

    /* 表示フラグ */
    mp->visible_flag=(visible_flag!=0);

    mp->mode=mode;

    DG_SetPos(&DG_UnitMatrix);
    if(r!=NULL) DG_RotatePosZYX(r);
    DG_GetPos(&(mp->root));

    fpu_CopyVector((FVECTOR *)&(mp->root.m[3][0]),&px);
}

/* 資源を獲得 */
static void GetAndRegistResources_called(int model_name,int sample_num,
										 OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
										 float oval_param,int collision_flag,int visible_flag,int mode)
{
	int size=allocated_work->size;
	int i;

	for(i=0;i<size;i++){
		if(*(allocated_work->objs+i)==NULL) break;
	}
	if(i==size) return;

	{
		int name;
		DG_OBJS		**objs=allocated_work->objs+i;
		MODEL_PARAMETER		*mp=allocated_work->mp+i;
		FMATRIX		*light=allocated_work->light+i*2;

		if(!Rope_GetModelName(&name)){
			return;
		}
		if(!(*objs=InitObject(name))){
			return;
		}

		GetOptionValue_called(*objs,light,mp,
							  model_name,sample_num,target,objnum,x,r,
							  oval_param,collision_flag,visible_flag,mode);

		if(!InitRope(*objs,mp)){
			DG_QueueObjs(*objs);
			DG_FreeObjs(*objs);
			*objs=NULL;
			return;
		}
		InitLocate(*objs,mp);
	}
}

void *NewRegistRope_called(int model_name,int sample_num,
						   OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
						   float oval_param,int collision_flag,int visible_flag,int mode)
{
    DummyWork *work;

    work=(DummyWork *)GV_NewActor(GV_ACTOR_USER,sizeof(DummyWork));
    if(work!=NULL) {
		GV_SetActor(&(work->actor),NullAct,NullDie);
		GetAndRegistResources_called(model_name,sample_num,target,objnum,x,r,
									 oval_param,collision_flag,visible_flag,mode);
    }
    return (void *)work;
}
