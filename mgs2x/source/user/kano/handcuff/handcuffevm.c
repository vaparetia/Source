//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	handcuffevm.c
		ＥＶＭ手錠の鎖を計算

	2001/06/09 K.Kano
	$Id: handcuffevm.c,v 1.1.1.3 2002/11/19 11:43:13 Yoshizawa1 Exp $
*/


#include "handcuff.h"


typedef struct {
	float vx;
	float vy;
} FVEC2;

/* ワーク */
typedef struct {
	float width,width2;
	float length;
	int n_points;

	FVEC2 *base[4];
	float *len;
} HANDCUFF_WORK;

typedef	struct _Work {
    GV_ACT_EX	actor;		/* リンク */
	DG_EVMOBJ	*evmobj;
    FMATRIX		light[2];

    int name;
	int mesg_disp;

	OBJECT *target1;
	int target1_num;
	FMATRIX *tmat1;

	OBJECT *target2;
	int target2_num;
	FMATRIX *tmat2;

    HANDCUFF_WORK handcuff;
} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)

#define VisibleObj(_obj)	((_obj)->flag&=~DG_EVMOBJ_INVISIBLE)
#define InvisibleObj(_obj)	((_obj)->flag|=DG_EVMOBJ_INVISIBLE)

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void ExitHandcuff(Work *_work)
{
	HANDCUFF_WORK *work=&(_work->handcuff);
	if(work->base[0]!=NULL) GV_Free(work->base[0]);
}


static int InitHandcuff(Work *_work)
{
	EVM_DEF *def=_work->evmobj->def;
	HANDCUFF_WORK *work=&(_work->handcuff);

	int n_models=def->n_x_models;
	int i;
	float sum_len;
	float r,r2;


	if((work->base[0]=(FVEC2 *)GV_Malloc((sizeof(FVEC2)*4+sizeof(float))*n_models))==NULL) return 0;
	work->base[1]=work->base[0]+n_models;
	work->base[2]=work->base[1]+n_models;
	work->base[3]=work->base[2]+n_models;
	work->len=(float *)(work->base[3]+n_models);

	work->n_points=n_models;

	sum_len=0.0f;

	work->len[1]=0.0f;

	for(i=2;i<n_models-1;i++){
		float ll=0.0f;

		float tt;

		tt=def->skeleton[i].tx;
		ll+=tt*tt;
		tt=def->skeleton[i].ty;
		ll+=tt*tt;
		tt=def->skeleton[i].tz;
		ll+=tt*tt;

		ll=fpu_Sqrt(ll);

		sum_len+=ll;

		work->len[i]=sum_len;
	}

	work->length=sum_len;

	/* 半径の計算 */
	r=sum_len*3.0f/(2.0f*M_PI);
	work->width=r*cosf(M_PI*(1.0f/6.0f))*2.0f;

	r2=sum_len/M_PI;
	work->width2=r2*2.0f;

	/* 直線で結ぶ */
	for(i=1;i<n_models-1;i++){
		work->base[0][i].vx=work->len[i]/sum_len;
		work->base[0][i].vy=0.0f;
	}

	/* 円の一部 */
	for(i=1;i<n_models-1;i++){
		work->base[1][i].vx=(cosf(M_PI*(1.0f/6.0f))
							 -cosf(M_PI*(2.0f/3.0f)*work->len[i]/sum_len+
								   M_PI*(1.0f/6.0f)))/(2.0f*cosf(M_PI*(1.0f/6.0f)));
		work->base[1][i].vy=(sinf(M_PI*(1.0f/6.0f))
							 -sinf(M_PI*(2.0f/3.0f)*work->len[i]/sum_len+
								   M_PI*(1.0f/6.0f)))*r;
	}

	for(i=1;i<n_models-1;i++){
		work->base[2][i].vx=(1.0f-cosf(M_PI*work->len[i]/sum_len))*0.5f;
		work->base[2][i].vy=-sinf(M_PI*work->len[i]/sum_len)*r2;
	}

	/* 二つ折り */
	for(i=1;i<n_models-1;i++){
		work->base[3][i].vx=work->len[i]/sum_len;
		if(work->len[i]<sum_len*0.5f) work->base[3][i].vy=-work->len[i];
		else work->base[3][i].vy=work->len[i]-sum_len;
	}

	return 1;
}


struct _scratchpad {
	FMATRIX world[0x80];
	FVECTOR x[0x80];
	FVECTOR q;
	FMATRIX mat,matt;
	FVECTOR qs,qe,qt;
};

static void CalcLocateHandcuff(Work *_work,struct _scratchpad *scrpad)
{
	DG_EVMOBJ *evmobj=_work->evmobj;
	HANDCUFF_WORK *work=&(_work->handcuff);

	FMATRIX *sm,*em;
	FVECTOR d,t;
	int i;
	float xlen;

	FVEC2 *base[2];
	float rate;

	static const FVECTOR ay={ 0.0f,1.0f,0.0f,1.0f };


	MT_MatToQuat(&(scrpad->qs),_work->tmat1);
	MT_MatToQuat(&(scrpad->qe),_work->tmat2);

	sm=&(scrpad->world[0]);
	em=&(scrpad->world[work->n_points-1]);

	t.vx=evmobj->def->skeleton[1].tx;
	t.vy=evmobj->def->skeleton[1].ty;
	t.vz=evmobj->def->skeleton[1].tz;
	t.vw=1.0f;

	vu0_Ldv0(&t);
	vu0_Ldm0(_work->tmat1);
	vu0_Mulv0m0v0();
	vu0_Stm0(sm);
	vu0_Stv0(&(scrpad->x[1]));

	t.vx=-evmobj->def->skeleton[work->n_points-1].tx;
	t.vy=-evmobj->def->skeleton[work->n_points-1].ty;
	t.vz=-evmobj->def->skeleton[work->n_points-1].tz;
	t.vw=1.0f;

	vu0_Ldv1(&t);
	vu0_Ldm0(_work->tmat2);
	vu0_Mulv1m0v1();
	vu0_Stm0(em);
	vu0_Stm0(&(scrpad->world[work->n_points-2]));

	vu0_Subv2v1v0();
	vu0_Stv1((FVECTOR *)&(scrpad->world[work->n_points-2].m[3][0]));
	vu0_Stv1(&(scrpad->x[work->n_points-2]));
	xlen=vu0_VectorLength2v2();
	xlen=fpu_Sqrt(xlen);

	// printf("xlen = %f\n",xlen);

	// xlen*=xlen;


	if(xlen>=work->length){
		base[0]=work->base[0];
		base[1]=work->base[0];
		rate=0.0f;
	}
	else if(xlen>=work->width){
		base[0]=work->base[0];
		base[1]=work->base[1];
		rate=(work->length-xlen)/(work->length-work->width);

		// printf("rate1 = %f\n",rate);
	}
	else if(xlen>=work->width2){
		base[0]=work->base[1];
		base[1]=work->base[2];
		rate=(work->width-xlen)/(work->width-work->width2);

		// printf("rate1 = %f\n",rate);
	}
	else{
		base[0]=work->base[2];
		base[1]=work->base[3];
		rate=(work->width2-xlen)/work->width2;
	}


	vu0_Stv2(&d);

	for(i=2;i<work->n_points-1;i++){
		float vx,vy;

		vx=base[0][i].vx*(1.0f-rate)+base[1][i].vx*rate;
		vy=base[0][i].vy*(1.0f-rate)+base[1][i].vy*rate;

		vu0_Mulv1v2a(vx);
		vu0_Ldv2(&ay);
		vu0_Addv1v0v1();
		vu0_Mulv2a(vy);
		vu0_Addv1v2();

		vu0_Ldv2(&d);

		vu0_Setv1w1();
		vu0_Stv1(&(scrpad->x[i]));
	}

	for(i=1;i<work->n_points-2;i++){
		t.vx=evmobj->def->skeleton[i+1].tx;
		t.vy=evmobj->def->skeleton[i+1].ty;
		t.vz=evmobj->def->skeleton[i+1].tz;
		t.vw=1.0f;

		MT_QuatSlerp(&(scrpad->qt),&(scrpad->qs),&(scrpad->qe),(float)i/(float)(work->n_points));
		MT_QuatNormalize(&(scrpad->qt),&(scrpad->qt));
		MT_QuatToMat(&(scrpad->matt),&(scrpad->qt));

		UTL_MakeQuatM(&(scrpad->q),&(scrpad->x[i]),&(scrpad->x[i+1]),
					  &(scrpad->matt),&t);

		// printf("%f %f %f %f\n",scrpad->qt.vx,scrpad->qt.vy,scrpad->qt.vz,scrpad->qt.vw);

		MT_QuatToMat(&(scrpad->mat),&(scrpad->q));

		vu0_Ldm0(&(scrpad->matt));
		vu0_Ldm1(&(scrpad->mat));
		vu0_Mulm2m1m0();
		vu0_Stm2(&(scrpad->world[i]));

		// printf("%f %f %f %f\n",scrpad->x[i].vx,scrpad->x[i].vy,scrpad->x[i].vz,scrpad->x[i].vw);

		fpu_CopyVector((FVECTOR *)&(scrpad->world[i].m[3][0]),&(scrpad->x[i]));
	}
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

static void Act(Work *work)
{
	struct _scratchpad *scrpad;
	int i;


    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int arg=*(msg->message+1);

		switch(command){
		case 0:
			/* 表示／非表示 */
			{
				int disp_sw;

				disp_sw=arg;

				switch(disp_sw){
				case 0:
				case 1:
                    work->mesg_disp = disp_sw ;
					break;
				case -1:
                    work->mesg_disp ^= 1 ;
					break;
				}
			}
			break;
		}

		msg++;
		n_msg--;
    }


    if(work->target1!=NULL){
		if(work->target1->evmobj!=NULL){
			if(work->target1->evmobj->flag & DG_EVMOBJ_INVISIBLE){
				InvisibleObj(work->evmobj);
			}
			else{
				if(work->mesg_disp) VisibleObj(work->evmobj);
				else InvisibleObj(work->evmobj);
			}
		}
		else if(work->target1->objs->flag & DG_FLAG_INVISIBLE){
			InvisibleObj(work->evmobj);
		}
		else if(work->target1->objs->objs[work->target1_num].flag & DG_FLAG_INVISIBLE){
			InvisibleObj(work->evmobj);
		}
		else{
			if(work->mesg_disp) VisibleObj(work->evmobj);
			else InvisibleObj(work->evmobj);
		}
	}

    if(work->target2!=NULL){
		if(work->target2->evmobj!=NULL){
			if(work->target2->evmobj->flag & DG_EVMOBJ_INVISIBLE){
				InvisibleObj(work->evmobj);
			}
			else{
				if(work->mesg_disp) VisibleObj(work->evmobj);
				else InvisibleObj(work->evmobj);
			}
		}
		else if(work->target2->objs->flag & DG_FLAG_INVISIBLE){
			InvisibleObj(work->evmobj);
		}
		else if(work->target2->objs->objs[work->target2_num].flag & DG_FLAG_INVISIBLE){
			InvisibleObj(work->evmobj);
		}
		else{
			if(work->mesg_disp) VisibleObj(work->evmobj);
			else InvisibleObj(work->evmobj);
		}
	}


    ScratchpadManInit();

	scrpad=(struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));

	CalcLocateHandcuff(work,scrpad);


	fpu_CopyMatrix(&(work->evmobj->world),&(scrpad->world[0]));

   DG_Arm_SwitchEvmBuffer(work->evmobj);

	{
		EVM_SKEL *skel=work->evmobj->def->skeleton;
		FMATRIX *mat=work->evmobj->matrix[work->evmobj->use_buffer];

		for(i=0;i<work->evmobj->n_skeleton;i++,mat++,skel++){
			FVECTOR rt;

			rt.vx=-skel->rt_tx;
			rt.vy=-skel->rt_ty;
			rt.vz=-skel->rt_tz;
			rt.vw=1.0f;

			vu0_Ldv0(&rt);
			vu0_Ldm0(&(scrpad->world[i]));
			vu0_Mulv0m0v0();
			vu0_Stm0(mat);
			vu0_Stv0((FVECTOR *)&(mat->m[3][0]));
		}
	}

    ScratchpadFree(scrpad);


#ifdef DEBUG
	// NewEvmSkeletonTest(work->evmobj,0x00ffff00);
#endif

}

static void Die(Work *work)
{
	ExitHandcuff(work);

    if(work->evmobj!=NULL){
		DG_DequeueEvmObj(work->evmobj);
		DG_FreeEvmObj(work->evmobj);
    }
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */

/* オブジェクトを初期化 */
static int InitObject(Work *work,int name)
{
    EVM_DEF *def ;

    def=(EVM_DEF*)GV_GetCache(GV_CacheID(name,'e'));
    if((work->evmobj=DG_MakeEvmObj(def,OBJECT_FLAG,0))==NULL) return 0;
    DG_QueueEvmObj(work->evmobj);
	VisibleObj(work->evmobj);

    return 1;
}

/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
	work->target1=NULL;
	work->target1_num=0;

	work->target2=NULL;
	work->target2_num=0;


    /* 特定charaの特定objに付いていく */
    if( GCL_GetOption( 'y' ) != NULL ){
		CONTROL *ctrl;
		int name;

		name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
		if(ctrl!=NULL){
			if((work->target1=ctrl->object)==NULL){
				work->target1=(OBJECT *)(ctrl+1);
			}
		}

		if(GCL_NextStr()!=NULL){
			name=GCL_GetNextInt();
			ctrl=GM_SearchWhere(name);
			if(ctrl!=NULL){
				if((work->target2=ctrl->object)==NULL){
					work->target2=(OBJECT *)(ctrl+1);
				}
			}
		}
		else{
			work->target2=work->target1;
		}
    }

    if( GCL_GetOption( 'z' ) != NULL ){
		work->target1_num=GCL_GetNextInt();
		work->target2_num=GCL_GetNextInt();
    }

    if(work->target1!=NULL){
		work->tmat1=&(work->target1->objs->objs[work->target1_num].world);
		work->evmobj->light=work->target1->objs->light;
    }
    else{
		work->evmobj->light=work->light;
    }

	if(work->target2!=NULL){
		work->tmat2=&(work->target2->objs->objs[work->target2_num].world);
	}
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	int name;

    if(!Rope_GetModelName(&name)) return 0;
    if(!InitObject(work,name)) return 0;
    GetOptionValue(work);
    if(!InitHandcuff(work)) return 0;

    return 1;
}


/* 初期化部メイン */
void *NewHandcuffEvm(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		work->name=name;
		work->mesg_disp=1;
		work->handcuff.base[0]=NULL;

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


/* ------------------------------------------------------------------------ */


/* 初期設定値を取得 */
static void GetOptionValue_called(Work *work,
								  OBJECT *target1,int num1,OBJECT *target2,int num2)
{
	work->target1=target1;
	work->target1_num=num1;

	if(target2==NULL){
		work->target2=target1;
	}
	else{
		work->target2=target2;
	}
	work->target2_num=num2;


    if(work->target1!=NULL){
		work->tmat1=&(work->target1->objs->objs[work->target1_num].world);
		work->evmobj->light=work->target1->objs->light;
    }
    else{
		work->evmobj->light=work->light;
    }

	if(work->target2!=NULL){
		work->tmat2=&(work->target2->objs->objs[work->target2_num].world);
	}
}

/* 資源を獲得 */
static int GetResources_called(Work *work,
							   int model_name,OBJECT *target1,int num1,OBJECT *target2,int num2)
{
    if(!InitObject(work,model_name)) return 0;
    GetOptionValue_called(work,target1,num1,target2,num2);
    if(!InitHandcuff(work)) return 0;

    return 1;
}


/* 初期化部メイン */
void *NewHandcuffEvm_called(int name,int model_name,OBJECT *target1,int num1,OBJECT *target2,int num2)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		work->name=name;
		work->mesg_disp=1;
		work->handcuff.base[0]=NULL;

		if(!GetResources_called(work,model_name,target1,num1,target2,num2)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
