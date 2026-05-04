/*
	cloth.c
		布シミュレート
		モデルが全て布モデルであるようなケース

	1999/08/16 K.Kano
	$Id: cloth.c,v 1.1.1.3 2002/11/19 11:43:08 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include "cloth.h"

#include "nest.c"
#include "clothsub.c"


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    GM_ActControl( &( work->control ) ) ;
    GM_ActObject( &( work->body ) ) ;
    MoveCloth(work);
}

static void Die(Work *work)
{
    ExitClothModel(work);
    GM_FreeControl( &( work->control ) ) ;
    GM_FreeObject( &( work->body ) ) ;
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */

/* コントロール初期化 */
static void InitControl(CONTROL *ctrl)
{
    GM_InitControl( ctrl, CLOTHMODEL_NAME, 0 ) ;
    ctrl->levels[ 0 ] = 0.0F ;
    ctrl->levels[ 1 ] = 32000.0F ;
    ctrl->mov = DG_ZeroVector ; 
    ctrl->step = DG_ZeroVector ;
    ctrl->rot = DG_ZeroSVector ;
    ctrl->turn = DG_ZeroSVector ;

    ctrl->seg_flag |= HZX_TYPE_PLAYER ;
    ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
}

/* オブジェクトを初期化 */
static	void	InitObject(Work *work,int name)
{
    OBJECT	*body ;

    body = &( work->body ) ;
    GM_InitObject(body,name,OBJECT_FLAG);
}

static int GetModelName(int *name)
{
#if 0
    char *x;

    if( GCL_GetOption( 'n' ) != NULL ){
	x=GCL_GetNextString();
    }
    else return 0;

    for( ;x!='\0';x++,name++) *name=*x;
    *name='\0';
#else
    if( GCL_GetOption( 'n' ) != NULL ){
	*name=GCL_GetNextInt();
    }
    else return 0;
#endif

    return 1;
}

/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
    MODEL_PARAMETER *mp=&(work->model_param);

    mp->pa=1013.0f*100.0f*0.0001f*5000.0f*0.000001f;
    mp->spring_u=50000.0f*0.000001f;
    mp->recover_u=10000.0f*0.000001f;
    mp->k=-100000.0f*0.000001f;
    mp->xlimit=2000.0f*0.0001f;

    /* サンプルデータ */
    if(GCL_GetOption('d')!=NULL){
	MODEL_PARAMETER sample[]={
	    {
		/* pa */
		1013*100*0.0001*0.005,

		/* spring_u */
		0.08,

		/* xlimit */
		0.4,

		/* recover_u */
		0.01,

		/* k */
		-0.15,
	    },
	    {
		/* pa */
		1013*100*0.0001*0.005,

		/* spring_u */
		0.08,

		/* xlimit */
		0.4,

		/* recover_u */
		0.03,

		/* k */
		-0.3,
	    },
	};
	int d=GCL_GetNextInt();

	*mp=sample[d];
    }

    /* 初期位置 */
    if ( GCL_GetOption( 'x' ) != NULL ) {
	int buf[ 3 ];

	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &( work->control.mov ) ) ;
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

    /* バネ定数 */
    if( GCL_GetOption('s') != NULL ){
	int s=GCL_GetNextInt();
	float fs;

	fs=(float)s*0.000001f;
	mp->spring_u=fs;
    }

    /* 可動範囲係数 */
    if( GCL_GetOption('t') != NULL ){
	int t=GCL_GetNextInt();
	float ft;

	ft=(float)t*0.0001f;
	mp->xlimit=ft;
    }

    /* 元位置に戻るためのバネ定数 */
    if( GCL_GetOption('u') != NULL ){
	int u=GCL_GetNextInt();
	float fu;

	fu=(float)u*0.000001f;
	mp->spring_u=fu;
    }

    if( GCL_GetOption( 'r' ) != NULL ){
	int buf[ 3 ];

	GCL_GetIV(GCL_NextStr(),buf);
	work->control.turn.vx=work->control.rot.vx=buf[0];
	work->control.turn.vy=work->control.rot.vy=buf[1];
	work->control.turn.vz=work->control.rot.vz=buf[2];
    }
}

/* 資源を獲得 */
static int GetResources(Work *work)
{

#if 0
    static MODEL_PARAMETER modelp_test={
	/* pa */
	1013*100*0.0001*0.005,

#if 0
	/* spring_u */
	0.01,

	/* xlimit */
	1.1,
#else
	/* spring_u */
	0.05,

	/* xlimit */
	0.2,
#endif

	/* recover_u */
	0.01,

	/* k */
	-0.1,
    };
#endif

    int name;

    if(!GetModelName(&name)) return 0;

    InitControl( &( work->control ) ) ;
    InitObject( work,name );

    if(!InitClothModel(work)) return 0;
    GetOptionValue( work );

    return 1;
}

/* 初期化部メイン */
void *NewClothModel(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	
	if(!GetResources(work)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
