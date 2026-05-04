//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ropemain.c
		ロープ(紐)シミュレート

	1999/09/21 K.Kano
	$Id: ropemain.c,v 1.1.1.3 2002/11/19 11:43:35 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include "rope.h"

#include "../../mode/demo/libdemo.h"


/* ワーク */
typedef	struct _Ropemain_Work {
    GV_ACT_EX	actor ;		/* リンク */
    DG_OBJS		*objs;

    FMATRIX		light[2];

    MODEL_PARAMETER	mp;

    int name;

	int	mesg_disp ;			/* Sono */

   int as_clock;
} RopemainWork ;

#define Work RopemainWork

int kn_rope_flag=1;


#define	ROPEMODEL_NAME	GV_StrCode( "ロープモデル" )
#define	ROPEMODEL3_NAME	GV_StrCode( "ロープモデル３" )
#define	MODEL_NAME	GV_StrCode( "chain" )
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC \
					 |DG_FLAG_SHADOWMAKE|DG_FLAG_IRREACTION|DG_FLAG_NOMSAA)


extern const SAMPLE_MODEL_PARAMETER rope_sample[];


/* ------------------------------------------------------------------------ */
/* メイン処理 */

#if 0

static void Act(Work *work)
{
    HZX_GROUP_ID hzx_id=HZX_CurrentGroupID;

    if(kn_rope_flag){
		DG_VisibleObjs(work->objs);
    }
    else{
		DG_InvisibleObjs(work->objs);
    }

    MoveRope1(work->objs,hzx_id,&(work->mp));
}

#endif

static void Ropemain_Act3(Work *work)
{
    HZX_GROUP_ID hzx_id=HZX_CurrentGroupID;


    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		switch(command){
		case 0:
			/* 表示／非表示 */
			{
				int disp_sw;
				disp_sw=*(msg->message+1);

				switch(disp_sw){
				case 0:
				case 1:
//					work->mp.visible_flag=disp_sw;
                    work->mesg_disp = disp_sw ;
					break;
				case -1:
//					work->mp.visible_flag^=1;
                    work->mesg_disp ^= 1 ;
					break;
				}
			}
			break;
		}

		msg++;
		n_msg--;
    }


#if 0
    if(kn_rope_flag){
		DG_VisibleObjs(work->objs);
    }
    else{
		DG_InvisibleObjs(work->objs);
    }
#endif

#if 0
    if(work->mp.target!=NULL){
		if(work->mp.visible_flag){
			DG_VisibleObjs(work->objs); 
		}
		else if(work->mp.target->evmobj!=NULL){
			if(work->mp.target->evmobj->flag & DG_EVMOBJ_INVISIBLE){
				DG_InvisibleObjs(work->objs);
			}
			else{
				DG_VisibleObjs(work->objs);
			}
		}
		else if(work->mp.target->objs->flag & DG_FLAG_INVISIBLE){
			DG_InvisibleObjs(work->objs);
		}
		else if(work->mp.target->objs->objs[work->mp.tobjnum].flag & DG_FLAG_INVISIBLE){
			DG_InvisibleObjs(work->objs);
		}
		else{
			if ( work->mesg_disp ) DG_VisibleObjs(work->objs);
			else 				   DG_InvisibleObjs(work->objs);
		}
	}
#else
    if(work->mp.target!=NULL){
		if(work->mp.target->evmobj==NULL){
			work->objs->group_id=work->mp.target->objs->group_id;
		}
		else{
			work->objs->group_id=work->mp.target->evmobj->group_id;
		}

		if(work->mp.visible_flag){
			DG_VisibleObjs(work->objs);
			work->objs->flag|=DG_FLAG_SHADOWMAKE;
		}
		else if(work->mesg_disp){
			int flag=0x000f;

			if(work->mp.target->evmobj!=NULL){
				flag&=((work->mp.target->evmobj->flag &
						(DG_EVMOBJ_INVISIBLE|DG_EVMOBJ_INVISIBLE2|DG_EVMOBJ_INVISIBLE3))>>8);
			}

			flag&=(((work->mp.target->objs->flag &
					 (DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3)) |
					(work->mp.target->objs->objs[work->mp.tobjnum].flag &
					 (DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3)))>>12);

			flag<<=12;

			work->objs->flag
				=((work->objs->flag &
				   ~(DG_FLAG_INVISIBLE|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3)) | flag);

			if((flag & DG_FLAG_INVISIBLE)==DG_FLAG_INVISIBLE){
				work->objs->flag&=~DG_FLAG_SHADOWMAKE;
			}
			else{
				work->objs->flag|=DG_FLAG_SHADOWMAKE;
			}
		}
		else{
			DG_InvisibleObjs(work->objs);
			work->objs->flag&=~DG_FLAG_SHADOWMAKE;
		}
	}
#endif

	if(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT)){
		InitLocate(work->objs,&(work->mp));
	}

   // Update rope every other frame in the hold areas
   if ( gAS_UsedBufferSceneNOP )
   {
      work->as_clock ^= 1;

      if ( work->as_clock )
      {
       MoveRope3(work->objs,hzx_id,&(work->mp));
      }
   }
   else
   {
      MoveRope3(work->objs,hzx_id,&(work->mp));
   }
}

static void Die(Work *work)
{
    ExitRope(work->objs,&(work->mp));
    if ( work->objs != NULL ){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
    }
}

static void InitAct(Work *work)
{
	InitLocate(work->objs,&(work->mp));
	GV_ChangeActFunc(&(work->actor),Ropemain_Act3);
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */

/* オブジェクトを初期化 */
static int InitObject(Work *work,int name)
{
    DG_DEF *def ;

    def = (DG_DEF*) GV_GetCache( GV_CacheID( name, 'k' ) );
    if((work->objs = DG_MakeObjs( def, OBJECT_FLAG, 0 ))==NULL) return 0;
    DG_QueueObjs( work->objs );
	DG_InvisibleObjs(work->objs);

    return 1;
}

/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
    FVECTOR x;
    SVECTOR rot;

    work->mp.m=0.001f;
    work->mp.inv_m=1.0f/work->mp.m;
    work->mp.pa=0.001f;
    work->mp.k=-0.0001f;
    work->mp.we=-1.0f;
    work->mp.wl=(int)CVC2N(10);

    work->mp.param_oval=1.2f;

    work->mp.collision_flag=0;
    work->mp.visible_flag=0;
    work->mp.mode=0;

    work->mp.tmat=NULL;
    fpu_CopyUnitMatrix(&(work->mp.root));
    fpu_ClearVector(&x);
    rot=DG_ZeroSVector;

    work->mp.target=NULL;
    work->mp.tobjnum=0;

    work->mp.boundmodel=NULL;

	work->mp.mov_rate=0.0f;
	work->mp.ymov_rate=0.0f;

    if(GCL_GetOption('d')!=NULL){
		int d=GCL_GetNextInt();

		work->mp.m=rope_sample[d].m;
		work->mp.inv_m=1.0f/work->mp.m;
		work->mp.pa=rope_sample[d].pa;
		work->mp.k=-rope_sample[d].k;
		work->mp.we=-rope_sample[d].we;
		work->mp.wl=rope_sample[d].wl;
		work->mp.mov_rate=rope_sample[d].mov_rate;
		work->mp.ymov_rate=rope_sample[d].ymov_rate;
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
			if((work->mp.target=ctrl->object)==NULL){
				work->mp.target=(OBJECT *)(ctrl+1);
			}
		}
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		work->mp.tobjnum=GCL_GetNextInt();
    }
    if(work->mp.target!=NULL){
		work->mp.tmat=&(work->mp.target->objs->objs[work->mp.tobjnum].world);
		DG_SetLightMatrix(work->objs,work->mp.target->objs->light);
    }
    else{
		DG_GetLightMatrix(&x,work->light);
		DG_SetLightMatrix(work->objs,work->light);
    }

    if(GCL_GetOption('b')!=NULL){
		int modelnum=GCL_GetNextInt();
		work->mp.boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(modelnum,'k'));
    }

    /* 質量 */
    if( GCL_GetOption('m') != NULL ){
		int m=GCL_GetNextInt();
		float fm,inv_fm;

		/* グラム単位 */
		fm=(float)m*0.001f;
		inv_fm=1.0f/fm;

		work->mp.m=fm;
		work->mp.inv_m=inv_fm;
    }

    /* 風によって受ける力 */
    if( GCL_GetOption('p') != NULL ){
		int p=GCL_GetNextInt();
		float fp;

		fp=(float)p*0.000001f;

		work->mp.pa=fp;
    }

    /* 速度に対する抵抗値 */
    if( GCL_GetOption('k') != NULL ){
		int k=GCL_GetNextInt();
		float fk;

		fk=(float)k*0.000001f;
		work->mp.k=-fk;
    }

    /* 壁に対する反発係数 */
    if( GCL_GetOption('e') != NULL ){
		int e=GCL_GetNextInt();
		float fe;

		fe=(float)e*0.001f+1.0f;
		work->mp.we=-fe;
    }

    /* 壁検出距離 */
    if( GCL_GetOption('l') != NULL ){
		int l=GCL_GetNextInt();
		work->mp.wl=l;
    }

    /* 楕円球当たりのパラメータ */
    if( GCL_GetOption('o') != NULL ){
		int o=GCL_GetNextInt();
		float fo;

		fo=(float)o*0.01f;
		work->mp.param_oval=fo;
    }

	/* 全体の移動の何割を無視するかのパラメータ */
    if( GCL_GetOption('w') != NULL ){
		int w=GCL_GetNextInt();
		float fw;

		fw=(float)w*0.01f;
		work->mp.mov_rate=fw;
    }
    if( GCL_GetOption('j') != NULL ){
		int w=GCL_GetNextInt();
		float fw;

		fw=(float)w*0.01f;
		work->mp.ymov_rate=fw;
    }

    /* 当たり判定フラグ */
    if( GCL_GetOption('f') != NULL ){
		int f=GCL_GetNextInt();
        work->mp.collision_flag=(f!=0);
    }

    /* 強制表示フラグ */
    if( GCL_GetOption('v') != NULL ){
        work->mp.visible_flag=1;
    }

    /* バンダナモードか髪の毛モードか選択 */
    if( GCL_GetOption('h') != NULL ){
        work->mp.mode=GCL_GetNextInt();
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
    DG_GetPos(&(work->mp.root));

    fpu_CopyVector((FVECTOR *)&(work->mp.root.m[3][0]),&x);
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    int name;

    if(!Rope_GetModelName(&name)) return 0;
    if(!InitObject( work,name )) return 0;
    GetOptionValue(work);
    if(!InitRope(work->objs,&(work->mp))) return 0;
	work->mesg_disp = 1 ;	/* Sono */
    return 1;
}

/* 初期化部メイン */
void *NewRopeModel(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));

		work->name=name;
		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}

/* 初期化部メイン */
void *NewRopeModel3(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));

		work->name=name;
		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



/* 初期設定値を取得 */
static void GetOptionValue_called(Work *work,int sample_num,
								  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
								  float oval_param,int collision_flag,int visible_flag,int mode,
								  int boundmodel_name)
{
    FVECTOR px;

    work->mp.m=0.001f;
    work->mp.inv_m=1.0f/work->mp.m;
    work->mp.pa=0.001f;
    work->mp.k=-0.0001f;
    work->mp.we=-1.0f;
    work->mp.wl=(int)CVC2N(10);

    work->mp.param_oval=1.2f;

    work->mp.collision_flag=0;
    work->mp.visible_flag=0;
    work->mp.mode=0;

    work->mp.tmat=NULL;
    fpu_CopyUnitMatrix(&(work->mp.root));
    fpu_ClearVector(&px);

    work->mp.target=NULL;
    work->mp.tobjnum=0;

    work->mp.boundmodel=NULL;

	work->mp.mov_rate=0.0f;
	work->mp.ymov_rate=0.0f;

    work->mp.m=rope_sample[sample_num].m;
    work->mp.inv_m=1.0f/work->mp.m;
    work->mp.pa=rope_sample[sample_num].pa;
    work->mp.k=-rope_sample[sample_num].k;
    work->mp.we=-rope_sample[sample_num].we;
    work->mp.wl=rope_sample[sample_num].wl;
    work->mp.mov_rate=rope_sample[sample_num].mov_rate;
    work->mp.ymov_rate=rope_sample[sample_num].ymov_rate;


    /* 初期位置 */
    if(x!=NULL){
		fpu_CopyVector(&px,x);
    } 
    px.vw=1.0f;

    /* 特定charaの特定objに付いていく */
    work->mp.target=target;
    work->mp.tobjnum=objnum;

    if(work->mp.target!=NULL){
		work->mp.tmat=&(work->mp.target->objs->objs[work->mp.tobjnum].world);
		DG_SetLightMatrix(work->objs,work->mp.target->objs->light);
    }
    else{
		DG_GetLightMatrix(&px,work->light);
		DG_SetLightMatrix(work->objs,work->light);
    }

    if(boundmodel_name!=0){
		work->mp.boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(boundmodel_name,'k'));
    }

    /* 楕円球当たりのパラメータ */
    work->mp.param_oval=oval_param;

    /* 当たり判定フラグ */
    work->mp.collision_flag=(collision_flag!=0);

    /* 表示フラグ */
    work->mp.visible_flag=(visible_flag!=0);

    work->mp.mode=mode;

    DG_SetPos(&DG_UnitMatrix);
    if(r!=NULL) DG_RotatePosZYX(r);
    DG_GetPos(&(work->mp.root));

    fpu_CopyVector((FVECTOR *)&(work->mp.root.m[3][0]),&px);
}

/* 資源を獲得 */
static int GetResources_called(Work *work,int model_name,int sample_num,
							   OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
							   float oval_param,int collision_flag,int visible_flag,int mode,
							   int boundmodel_name)
{
    if(!InitObject( work,model_name )) return 0;
    GetOptionValue_called(work,sample_num,target,objnum,x,r,oval_param,collision_flag,
						  visible_flag,mode,boundmodel_name);
    if(!InitRope(work->objs,&(work->mp))) return 0;

	work->mesg_disp = 1 ;	/* Sono */
    return 1;
}

/* 初期化部メイン */
void *NewRopeModel_called(int model_name,int sample_num,
						  FVECTOR *x,SVECTOR *r,int collision_flag)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));
	
		work->name=0;
		if(!GetResources_called(work,model_name,sample_num,NULL,0,x,r,
								0.0f,collision_flag,0,0,0)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}

/* 初期化部メイン */
void *NewRopeModel3_called(int model_name,int sample_num,
						   OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
						   float oval_param,int collision_flag,int visible_flag,int mode,
						   int boundmodel_name)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));

		work->name=0;
		if(!GetResources_called(work,model_name,sample_num,target,(objnum<0 ? 0 : objnum),x,r,
								oval_param,collision_flag,visible_flag,mode,boundmodel_name)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }

    return (void *)work ;
}
