//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	waving_cloth.c
		布シミュレート
		モデルが全て布モデルであるようなケース

	1999/08/16 K.Kano
	$Id: waving_cloth.c,v 1.1.1.3 2002/11/19 11:43:44 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif


#include "waving_cloth.h"

#include "BP_EndianSupport.h"

// #define TEST

static const MODEL_PARAMETER sample[]={
    /* 旗 */
    {
		/* pa */
		1013*100*0.0001*0.005*0.0005,

		/* spring_u */
		0.03,

		/* xlimit */
		0.6,

		/* recover_u */
		0.005,

		/* k */
		-0.1,
    },
    /* マント */
    {
		/* pa */
		1013*100*0.0001*0.005*0.0005,

		/* spring_u */
		0.08,

		/* xlimit */
		0.6,

		/* recover_u */
		0.1,

		/* k */
		-0.1,
    },
    /* マント２ */
    {
		/* pa */
		1013*100*0.0001*0.005*0.0005,

		/* spring_u */
		0.25,

		/* xlimit */
		0.6,

		/* recover_u */
		0.07,

		/* k */
		-0.06,
    },
    /* マント３ */
    {
		/* pa */
		12.0f*0.000001f,

		/* spring_u */
		50000.0f*0.000001f,

		/* xlimit */
		4000.0f*0.0001f,

		/* recover_u */
		18000.0f*0.000001f,

		/* k */
		-300000.0f*0.000001f,
    },
};


/* ------------------------------------------------------------------------ */
/* メイン処理 */

enum {
	WVCLOTH_MSG_SELCVD=0,
	WVCLOTH_MSG_COLLISION_FLAG,
};

static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->address,&msg);

    while(n_msg>0){
		int cmd=*(msg->message+0);
		int arg=*(msg->message+1);

		switch(cmd){
		case WVCLOTH_MSG_SELCVD:
			work->cvd_sel=arg;
			if(work->cvd_sel>=work->cvd_size) work->cvd_sel=0;
			break;
		case WVCLOTH_MSG_COLLISION_FLAG:
			work->collision_flag=arg;
			break;
		}

		msg++;
		n_msg--;
    }

#ifdef TEST  /* TEST */
    if(GV_PadData[0].press & PAD_SEL){
		work->cvd_sel++;
		if(work->cvd_sel>=work->cvd_size) work->cvd_sel=0;
    }
#endif

	// DG_VisibleObjs(work->body.objs);
	DG_VisibleObjs(work->objs);

    MoveWavingCloth(work);
}

static void ActW(Work *work)
{
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->address,&msg);

    while(n_msg>0){
		int cmd=*(msg->message+0);
		int arg=*(msg->message+1);

		switch(cmd){
		case WVCLOTH_MSG_SELCVD:
			work->cvd_sel=arg;
			if(work->cvd_sel>=work->cvd_size) work->cvd_sel=0;
			break;
		case WVCLOTH_MSG_COLLISION_FLAG:
			work->collision_flag=arg;
			break;
		}

		msg++;
		n_msg--;
    }

#ifdef TEST  /* TEST */
    if(GV_PadData[0].press & PAD_SEL){
		work->cvd_sel++;
		if(work->cvd_sel>=work->cvd_size) work->cvd_sel=0;
    }
#endif

	// DG_VisibleObjs(work->body.objs);
	DG_VisibleObjs(work->objs);

    if(work->target!=NULL){
		if(work->force_disp_enable){
			// DG_VisibleObjs(work->body.objs);
			DG_VisibleObjs(work->objs);
		}
		else if(work->target->evmobj!=NULL){
			if(work->target->evmobj->flag & DG_EVMOBJ_INVISIBLE){
				DG_InvisibleObjs(work->objs);
			}
			else{
				DG_VisibleObjs(work->objs);
			}
		}
		else if(work->target->objs->flag & DG_FLAG_INVISIBLE){
			// DG_InvisibleObjs(work->body.objs);
			DG_InvisibleObjs(work->objs);
		}
		else{
			// DG_VisibleObjs(work->body.objs);
			DG_VisibleObjs(work->objs);
		}
    }
    MoveWavingClothWithOBJ(work);

#if 0
	printf("%f %f %f %f\n",
		   work->objs->world.m[3][0],work->objs->world.m[3][1],work->objs->world.m[3][2],
		   work->objs->world.m[3][3]);
#endif

}

static void ActJacketW(Work *work)
{
    GV_MSG *msg;
    int n_msg;

    n_msg=GV_ReceiveMessage(work->address,&msg);

    while(n_msg>0){
		int cmd=*(msg->message+0);
		int arg=*(msg->message+1);

		switch(cmd){
		case WVCLOTH_MSG_SELCVD:
			work->cvd_sel=arg;
			if(work->cvd_sel>=work->cvd_size) work->cvd_sel=0;
			break;
		case WVCLOTH_MSG_COLLISION_FLAG:
			work->collision_flag=arg;
			break;
		}

		msg++;
		n_msg--;
    }

#ifdef TEST  /* TEST */
    if(GV_PadData[0].press & PAD_SEL){
		work->cvd_sel++;
		if(work->cvd_sel>=work->cvd_size) work->cvd_sel=0;
    }
#endif

	// DG_VisibleObjs(work->body.objs);
	DG_VisibleObjs(work->objs);

    if(work->target!=NULL){
		if(work->force_disp_enable){
			// DG_VisibleObjs(work->body.objs);
			DG_VisibleObjs(work->objs);
		}
		else if(work->target->evmobj!=NULL){
			if(work->target->evmobj->flag & DG_EVMOBJ_INVISIBLE){
				DG_InvisibleObjs(work->objs);
			}
			else{
				DG_VisibleObjs(work->objs);
			}
		}
		else if(work->target->objs->flag & DG_FLAG_INVISIBLE){
			// DG_InvisibleObjs(work->body.objs);
			DG_InvisibleObjs(work->objs);
		}
		else{
			// DG_VisibleObjs(work->body.objs);
			DG_VisibleObjs(work->objs);
		}
    }
    MoveWavingClothWithOBJ(work);
    CopyMatrixforJacket(work);
}

static void Die(Work *work)
{
    ExitClothModel(work);
    // GM_FreeObject( &( work->body ) ) ;
	DG_DequeueObjs(work->objs);
	if(work->target!=NULL){
		// DG_DisconnectObjs(work->target->objs,work->objs);
	}
	DG_FreeObjs(work->objs);
    DG_FreeAnimVertsBuffer( &(work->va) );
}

static void DieWithoutDATAFILE(Work *work)
{
    ExitClothModel_withoutDATAFILE(work);
    // GM_FreeObject( &( work->body ) ) ;
	DG_DequeueObjs(work->objs);
	if(work->target!=NULL){
		// DG_DisconnectObjs(work->target->objs,work->objs);
	}
	DG_FreeObjs(work->objs);
    DG_FreeAnimVertsBuffer( &(work->va) );
}

static void InitActW(Work *work)
{
    // WavingCloth_CalcBase(work,&(work->body.objs->objs[0].world));
	WavingCloth_CalcBase(work,&(work->objs->objs[0].world));
	GV_ChangeActFunc(&(work->actor),ActW);
}

static void InitActJacketW(Work *work)
{
	// WavingCloth_CalcBase(work,&(work->body.objs->objs[0].world));
	WavingCloth_CalcBase(work,&(work->objs->objs[0].world));
	GV_ChangeActFunc(&(work->actor),ActJacketW);
}

static void EndianSwapClothData( CLOTHDATA_FILE *file )
{
   POLY_PARAMETER *poly_params;
   LINE_PARAMETER *line_params;
   VERTEX_PARAMETER *ver_params;
   int i;
   
   BP_LE_SwapSInt_Inp( &file->id );
   BP_LE_SwapSInt_Inp( &file->n_verts );
   BP_LE_SwapSInt_Inp( &file->n_lines );
   BP_LE_SwapSInt_Inp( &file->n_polys );

   poly_params=(POLY_PARAMETER *)(file+1);
   line_params=(LINE_PARAMETER *)(poly_params+file->n_polys);
   ver_params=(VERTEX_PARAMETER *)(line_params+file->n_lines);

   // Swap the poly, line, and vertex parameters

   for ( i = 0; i < file->n_verts; ++i )
   {
      BP_LE_SwapFloat_Inp( &( ver_params[i].inv_n ) );
      BP_LE_SwapFloat_Inp( &( ver_params[i].fnest ) );
   }

   for ( i = 0; i < file->n_lines; ++i )
   {
      BP_LE_SwapSInt_Inp( &(  line_params[i].index[0] ) );
      BP_LE_SwapSInt_Inp( &(  line_params[i].index[1] ) );
      BP_LE_SwapFloat_Inp( &( line_params[i].l0 ) );
      BP_LE_SwapSInt_Inp( &(  line_params[i].dummy ) );
   }

   for ( i = 0; i < file->n_polys; ++i )
   {
      BP_LE_SwapSIntArray_Inp( &( poly_params[i].index[0] ), 3 );
      BP_LE_SwapFloat_Inp( &( poly_params[i].normal_sign ) );
   }

}

/* ------------------------------------------------------------------------ */
/* 初期化部 */

/* オブジェクトを初期化 */
static	void	InitObject(Work *work,int name)
{
    // OBJECT	*body ;
	DG_DEF *def;
	CLOTHDATA_FILE *file;
   int wasSwapped = 0;

    // body = &( work->body ) ;
    // GM_InitObject(body,name,OBJECT_FLAG);
	if((def=(DG_DEF*)GV_GetCache(GV_CacheID(name,'k')))==NULL){
		ASSERT(0);
		return;
	}
    if((work->objs = DG_MakeObjs(def,OBJECT_FLAG,0))==NULL){
		ASSERT(0);
		return;
	}
	DG_QueueObjs(work->objs);
	// DG_InvisibleObjs(body->objs);
	DG_InvisibleObjs(work->objs);

	/* データファイルの取得 */
	file=(CLOTHDATA_FILE *)GV_GetCacheMarkSwapped(GV_CacheID(name,'r'), &wasSwapped);

   if ( !wasSwapped )
   {
      EndianSwapClothData( file );
   }

   work->num_vers=file->n_verts;
	work->num_lines=file->n_lines;
	work->num_polys=file->n_polys;
	work->poly_params=(POLY_PARAMETER *)(file+1);
	work->line_params=(LINE_PARAMETER *)(work->poly_params+work->num_polys);
	work->ver_params=(VERTEX_PARAMETER *)(work->line_params+work->num_lines);

    work->cvd_def=NULL;
    work->cvd_size=1;
    if((work->cvd_def=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*work->cvd_size))!=NULL){
		*(work->cvd_def+0)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
    }
    work->cvd_sel=0;
}

static int InitObjectFromOption(Work *work)
{
    // OBJECT *body;
	DG_DEF *def;
	CLOTHDATA_FILE *file;
    int name;
    int i;
    int wasSwapped = 0;

    work->cvd_def=NULL;

    if(GCL_GetOption('n')==NULL) return 0;

    i=work->cvd_size=GCL_GetNextInt();

    name=GCL_GetNextInt();

    // body=&(work->body);
    // GM_InitObject(body,name,OBJECT_FLAG);
	if((def=(DG_DEF*)GV_GetCache(GV_CacheID(name,'k')))==NULL){
		ASSERT(0);
		return 0;
	}
    if((work->objs = DG_MakeObjs(def,OBJECT_FLAG,0))==NULL){
		ASSERT(0);
		return 0;
	}
	DG_QueueObjs(work->objs);
	// DG_InvisibleObjs(body->objs);
	DG_InvisibleObjs(work->objs);

	/* データファイルの取得 */
	file=(CLOTHDATA_FILE *)GV_GetCacheMarkSwapped(GV_CacheID(name,'r'), &wasSwapped);
   if ( !wasSwapped )
   {
      EndianSwapClothData( file );
   }
	work->num_vers=file->n_verts;
	work->num_lines=file->n_lines;
	work->num_polys=file->n_polys;
	work->poly_params=(POLY_PARAMETER *)(file+1);
	work->line_params=(LINE_PARAMETER *)(work->poly_params+work->num_polys);
	work->ver_params=(VERTEX_PARAMETER *)(work->line_params+work->num_lines);

    if((work->cvd_def=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*work->cvd_size))==NULL) return 0;

    *(work->cvd_def+0)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
    i--;
    while(i>0){
		name=GCL_GetNextInt();
		*(work->cvd_def+work->cvd_size-i)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
		i--;
    }

    work->cvd_sel=0;

	// printf("cvd_size = %d\n",work->cvd_size);

    return 1;
}

static int InitObjectFromArg(Work *work,int *model_name,int model_name_size,int boundmodel_name)
{
    // OBJECT *body;
	DG_DEF *def;
	CLOTHDATA_FILE *file;
    int name;
    int i;
    int wasSwapped = 0;

    work->cvd_def=NULL;

    i=work->cvd_size=model_name_size;

    name=model_name[0];

    // printf("Name = %d\n",name);

    // body=&(work->body);
    // GM_InitObject(body,name,OBJECT_FLAG);
	if((def=(DG_DEF*)GV_GetCache(GV_CacheID(name,'k')))==NULL){
		ASSERT(0);
		return 0;
	}
    if((work->objs=DG_MakeObjs(def,OBJECT_FLAG,0))==NULL){
		ASSERT(0);
		return 0;
	}
	DG_QueueObjs(work->objs);
	// DG_InvisibleObjs(body->objs);
	DG_InvisibleObjs(work->objs);

	/* データファイルの取得 */
	file=(CLOTHDATA_FILE *)GV_GetCacheMarkSwapped(GV_CacheID(name,'r'), &wasSwapped);
   if ( !wasSwapped )
   {
      EndianSwapClothData( file );
   }
	work->num_vers=file->n_verts;
	work->num_lines=file->n_lines;
	work->num_polys=file->n_polys;
	work->poly_params=(POLY_PARAMETER *)(file+1);
	work->line_params=(LINE_PARAMETER *)(work->poly_params+work->num_polys);
	work->ver_params=(VERTEX_PARAMETER *)(work->line_params+work->num_lines);

    work->boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(boundmodel_name,'k'));

    if((work->cvd_def=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*work->cvd_size))==NULL) return 0;

    *(work->cvd_def+0)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));

    // printf("Model Addr = 0x%08x\n",(int)*(work->cvd_def+0));

    i--;
    while(i>0){
		name=model_name[i];
		*(work->cvd_def+i)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
		i--;
    }

    work->cvd_sel=0;

    return 1;
}

static	void	InitObject_withoutDATAFILE(Work *work,int name)
{
    // OBJECT	*body ;
	DG_DEF *def;

    // body = &( work->body ) ;
    // GM_InitObject(body,name,OBJECT_FLAG);
	if((def=(DG_DEF*)GV_GetCache(GV_CacheID(name,'k')))==NULL){
		ASSERT(0);
		return;
	}
    if((work->objs=DG_MakeObjs(def,OBJECT_FLAG,0))==NULL){
		ASSERT(0);
		return;
	}
	DG_QueueObjs(work->objs);
	// DG_InvisibleObjs(body->objs);
	DG_InvisibleObjs(work->objs);

    work->cvd_def=NULL;
    work->cvd_size=1;
    if((work->cvd_def=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*work->cvd_size))!=NULL){
		*(work->cvd_def+0)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
    }
    work->cvd_sel=0;
}

static int InitObjectFromOption_withoutDATAFILE(Work *work)
{
    // OBJECT *body;
	DG_DEF *def;
    int name;
    int i;

    work->cvd_def=NULL;

    if(GCL_GetOption('n')==NULL) return 0;

    i=work->cvd_size=GCL_GetNextInt();

    name=GCL_GetNextInt();

    // body=&(work->body);
    // GM_InitObject(body,name,OBJECT_FLAG);
	if((def=(DG_DEF*)GV_GetCache(GV_CacheID(name,'k')))==NULL){
		ASSERT(0);
		return 0;
	}
    if((work->objs=DG_MakeObjs(def,OBJECT_FLAG,0))==NULL){
		ASSERT(0);
		return 0;
	}
	DG_QueueObjs(work->objs);
	// DG_InvisibleObjs(body->objs);
	DG_InvisibleObjs(work->objs);

    if((work->cvd_def=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*work->cvd_size))==NULL) return 0;

    *(work->cvd_def+0)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
    i--;
    while(i>0){
		name=GCL_GetNextInt();
		*(work->cvd_def+work->cvd_size-i)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
		i--;
    }

    work->cvd_sel=0;

	// printf("cvd_size = %d\n",work->cvd_size);

    return 1;
}

static int InitObjectFromArg_withoutDATAFILE(Work *work,int *model_name,int model_name_size,int boundmodel_name)
{
    // OBJECT *body;
	DG_DEF *def;
    int name;
    int i;

    work->cvd_def=NULL;

    i=work->cvd_size=model_name_size;

    name=model_name[0];

    // printf("Name = %d\n",name);

    // body=&(work->body);
    // GM_InitObject(body,name,OBJECT_FLAG);
	if((def=(DG_DEF*)GV_GetCache(GV_CacheID(name,'k')))==NULL){
		ASSERT(0);
		return 0;
	}
    if((work->objs=DG_MakeObjs(def,OBJECT_FLAG,0))==NULL){
		ASSERT(0);
		return 0;
	}
	DG_QueueObjs(work->objs);
	// DG_InvisibleObjs(body->objs);
	DG_InvisibleObjs(work->objs);

    work->boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(boundmodel_name,'k'));

    if((work->cvd_def=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*work->cvd_size))==NULL) return 0;

    *(work->cvd_def+0)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));

    // printf("Model Addr = 0x%08x\n",(int)*(work->cvd_def+0));

    i--;
    while(i>0){
		name=model_name[i];
		*(work->cvd_def+i)=(CV2_DEF*)GV_GetCache(GV_CacheID(name,'c'));
		i--;
    }

    work->cvd_sel=0;

    return 1;
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
static void GetOptionValue(Work *work,FMATRIX *m)
{
    MODEL_PARAMETER *mp=&(work->model_param);
    FVECTOR x;
    SVECTOR rot;
    int tobjnum=HUMAN21_MUNE;

    mp->pa=1013.0f*100.0f*0.0001f*5000.0f*0.000001f*0.0005f;
    mp->spring_u=50000.0f*0.000001f;
    mp->recover_u=10000.0f*0.000001f;
    mp->k=-100000.0f*0.000001f;
    mp->xlimit=5000.0f*0.0001f;

    work->target=NULL;
    work->param_oval=1.40f;

    work->troot=NULL;
    work->tmat=NULL;

	work->collision_flag=0;

    work->boundmodel=NULL;

    work->force_disp_enable=0;

    x=DG_ZeroVector;
    rot=DG_ZeroSVector;

    /* サンプルデータ */
    if(GCL_GetOption('d')!=NULL){
		int d=GCL_GetNextInt();
		*mp=sample[d];
    }

    /* 初期位置 */
    if ( GCL_GetOption( 'x' ) != NULL ) {
		int buf[ 3 ];

		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &x ) ;
    }
    x.vw=1.0f;

    /* ついていく先のcharaの名前 */
    if( GCL_GetOption( 'y' ) != NULL ){
		CONTROL *ctrl;
		int name;

		name=GCL_GetNextInt();
		ctrl=GM_SearchWhere(name);
		if(ctrl!=NULL) work->target=(OBJECT *)(ctrl+1);
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		tobjnum=GCL_GetNextInt();
    }
    if(work->target!=NULL){
		work->troot=&(work->target->objs->objs[0].world);
		work->tmat=&(work->target->objs->objs[tobjnum].world);
		// DG_SetLightMatrix(work->body.objs,work->target->objs->light);
		DG_SetLightMatrix(work->objs,work->target->objs->light);
		// DG_ConnectObjs(work->target->objs,work->objs);
    }
    else{
		DG_GetLightMatrix(&x,work->light);
		// DG_SetLightMatrix(work->body.objs,work->light);
		DG_SetLightMatrix(work->objs,work->light);
    }

    /* 当たり判定に用いるモデル */
    if(GCL_GetOption('b')!=NULL){
		int modelnum=GCL_GetNextInt();
		work->boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(modelnum,'k'));
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
    if( GCL_GetOption('u') != NULL ){
		int u=GCL_GetNextInt();
		float fu;

		fu=(float)u*0.000001f;
		mp->spring_u=fu;
    }

    /* 可動範囲係数 */
    if( GCL_GetOption('t') != NULL ){
		int t=GCL_GetNextInt();
		float ft;

		ft=(float)t*0.0001f;
		mp->xlimit=ft;
    }

    /* 元位置に戻るためのバネ定数 */
    if( GCL_GetOption('s') != NULL ){
		int s=GCL_GetNextInt();
		float fs;

		fs=(float)s*0.000001f;
		mp->recover_u=fs;
    }

    /* 楕円球当たりのパラメータ */
    if( GCL_GetOption('o') != NULL ){
		int o=GCL_GetNextInt();
		float fo;

		fo=(float)o*0.01f;
		work->param_oval=fo;
    }

    /* 強制表示フラグのON/OFF */
    if( GCL_GetOption('v') != NULL ) work->force_disp_enable=1;

    if( GCL_GetOption( 'r' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		rot.vx=buf[0];
		rot.vy=buf[1];
		rot.vz=buf[2];
    }
    DG_SetPos(&DG_UnitMatrix);
    DG_RotatePosZYX(&rot);
    DG_GetPos(&(work->rot_mat));

    fpu_CopyVector((FVECTOR *)&(work->rot_mat.m[3][0]),&x);
    fpu_CopyMatrix(m,&(work->rot_mat));
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    FMATRIX m;

#if 0
    int name;

    if(!GetModelName(&name)) return 0;
    InitObject( work,name );
#else
    if(!InitObjectFromOption(work)) return 0;
#endif
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel(work)) return 0;
    GetOptionValue(work,&m);
    WavingCloth_CalcBase(work,&m);

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModel(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		work->address=name;
		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



/* 資源を獲得 */
static int GetResources_withoutDATAFILE(Work *work)
{
    FMATRIX m;

#if 0
    int name;

    if(!GetModelName(&name)) return 0;
    InitObject( work,name );
#else
    if(!InitObjectFromOption_withoutDATAFILE(work)) return 0;
#endif
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel_withoutDATAFILE(work)) return 0;
    GetOptionValue(work,&m);
    WavingCloth_CalcBase(work,&m);

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModel_withoutDATAFILE(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,DieWithoutDATAFILE) ;
		GV_ActorEX(&(work->actor));

		work->address=name;
		if(!GetResources_withoutDATAFILE(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



/* 資源を獲得 */
static int GetResourcesW(Work *work)
{
    FMATRIX m;

#if 0
    int name;

    if(!GetModelName(&name)) return 0;
    InitObject( work,name );
#else
    if(!InitObjectFromOption(work)) return 0;
#endif
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel(work)) return 0;
    GetOptionValue(work,&m);
    // WavingCloth_CalcBase(work,&(work->body.objs->objs[0].world));

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModelW(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitActW,Die) ;
		GV_ActorEX(&(work->actor));

		work->address=name;
#if 1
		if(!GetResourcesW(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
#else
		GetResourcesW(work);
		GV_DestroyActor(work);
		return NULL;
#endif

    }
    return (void *)work ;
}


/* 資源を獲得 */
static int GetResourcesW_withoutDATAFILE(Work *work)
{
    FMATRIX m;

#if 0
    int name;

    if(!GetModelName(&name)) return 0;
    InitObject( work,name );
#else
    if(!InitObjectFromOption_withoutDATAFILE(work)) return 0;
#endif
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel_withoutDATAFILE(work)) return 0;
    GetOptionValue(work,&m);
    // WavingCloth_CalcBase(work,&(work->body.objs->objs[0].world));

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModelW_withoutDATAFILE(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitActW,DieWithoutDATAFILE) ;
		GV_ActorEX(&(work->actor));

		work->address=name;

		if(!GetResourcesW_withoutDATAFILE(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}

void *NewWavingJacketModelW_withoutDATAFILE(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitActJacketW,DieWithoutDATAFILE) ;
		GV_ActorEX(&(work->actor));

		work->address=name;
		if(!GetResourcesW_withoutDATAFILE(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



/* 初期設定値を取得 */
static void GetOptionValue_called(Work *work,FMATRIX *m,int sample_num,
								  OBJECT *target,int tobjnum,FVECTOR *px,SVECTOR *r,
								  float oval_param,int force_disp_flag)
{
    MODEL_PARAMETER *mp=&(work->model_param);
    FVECTOR x;
    SVECTOR rot;

    mp->pa=1013.0f*100.0f*0.0001f*5000.0f*0.000001f*0.0005f;
    mp->spring_u=50000.0f*0.000001f;
    mp->recover_u=10000.0f*0.000001f;
    mp->k=-100000.0f*0.000001f;
    mp->xlimit=5000.0f*0.0001f;

    work->target=NULL;
    work->param_oval=1.10f;

    work->troot=NULL;
    work->tmat=NULL;

	work->collision_flag=0;

    // work->boundmodel=NULL;

    work->force_disp_enable=force_disp_flag;

    x=DG_ZeroVector;
    rot=DG_ZeroSVector;

    /* サンプルデータ */
    *mp=sample[sample_num];

    /* 初期位置 */
    if(px!=NULL) {
		fpu_CopyVector(&x,px);
    }
    x.vw=1.0f;

    work->target=target;
    if(work->target!=NULL){
		work->troot=&(work->target->objs->objs[0].world);
		work->tmat=&(work->target->objs->objs[tobjnum].world);
		// DG_SetLightMatrix(work->body.objs,work->target->objs->light);
		DG_SetLightMatrix(work->objs,work->target->objs->light);
		// DG_ConnectObjs(work->target->objs,work->objs);
    }
    else{
		DG_GetLightMatrix(&x,work->light);
		// DG_SetLightMatrix(work->body.objs,work->light);
		DG_SetLightMatrix(work->objs,work->light);
    }

    /* 楕円球当たりのパラメータ */
    work->param_oval=oval_param;

    if(r!=NULL){
		rot.vx=r->vx;
		rot.vy=r->vy;
		rot.vz=r->vz;
    }
    DG_SetPos(&DG_UnitMatrix);
    DG_RotatePosZYX(&rot);
    DG_GetPos(&(work->rot_mat));

    fpu_CopyVector((FVECTOR *)&(work->rot_mat.m[3][0]),&x);
    fpu_CopyMatrix(m,&(work->rot_mat));
}


/* 資源を獲得 */
static int GetResources_called(Work *work,int model_name,int sample_num,FVECTOR *x,SVECTOR *r)
{
    FMATRIX m;

    InitObject( work,model_name );
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel(work)) return 0;
    GetOptionValue_called(work,&m,sample_num,NULL,0,x,r,0.0f,0);
    WavingCloth_CalcBase(work,&m);

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModel_called(int model_name,int sample_num,FVECTOR *x,SVECTOR *r)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		work->address=0;
		if(!GetResources_called(work,model_name,sample_num,x,r)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


/* 資源を獲得 */
static int GetResources_withoutDATAFILE_called(Work *work,int model_name,int sample_num,FVECTOR *x,SVECTOR *r)
{
    FMATRIX m;

    InitObject_withoutDATAFILE( work,model_name );
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel_withoutDATAFILE(work)) return 0;
    GetOptionValue_called(work,&m,sample_num,NULL,0,x,r,0.0f,0);
    WavingCloth_CalcBase(work,&m);

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModel_withoutDATAFILE_called(int model_name,int sample_num,FVECTOR *x,SVECTOR *r)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,DieWithoutDATAFILE) ;
		GV_ActorEX(&(work->actor));

		work->address=0;
		if(!GetResources_withoutDATAFILE_called(work,model_name,sample_num,x,r)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



/* 資源を獲得 */
static int GetResourcesW_called(Work *work,int *model_name,int model_name_size,int sample_num,
								OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
								int boundmodel_name,float oval_param,int force_disp_flag)
{
    FMATRIX m;

#if 0
    InitObject( work,model_name );
#else
    if(!InitObjectFromArg(work,model_name,model_name_size,boundmodel_name)) return 0;
#endif
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel(work)) return 0;
    GetOptionValue_called(work,&m,sample_num,target,objnum,x,r,oval_param,force_disp_flag);
    // WavingCloth_CalcBase(work,&(work->body.objs->objs[0].world));

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModelW_called(int address,int *model_name,int model_name_size,int sample_num,
								  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
								  int boundmodel_name,float oval_param,int force_disp_flag)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitActW,Die) ;
		GV_ActorEX(&(work->actor));

		work->address=address;
		if(!GetResourcesW_called(work,model_name,model_name_size,sample_num,target,objnum,x,r,
								 boundmodel_name,oval_param,force_disp_flag)){

			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



/* 資源を獲得 */
static int GetResourcesW_withoutDATAFILE_called(Work *work,int *model_name,int model_name_size,
												int sample_num,
												OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
												int boundmodel_name,float oval_param,int force_disp_flag)
{
    FMATRIX m;

#if 0
    InitObject( work,model_name );
#else
    if(!InitObjectFromArg_withoutDATAFILE(work,model_name,model_name_size,boundmodel_name)) return 0;
#endif
    if(work->cvd_def==NULL) return 0;

#if 0
    DG_MakeAnimVertsBuffer( &(work->va), work->body.objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#else
    DG_MakeAnimVertsBuffer( &(work->va), work->objs->objs,
							DG_VANIME_VERTS|DG_VANIME_NORMS );
#endif

    if(!InitClothModel_withoutDATAFILE(work)) return 0;
    GetOptionValue_called(work,&m,sample_num,target,objnum,x,r,oval_param,force_disp_flag);
    // WavingCloth_CalcBase(work,&(work->body.objs->objs[0].world));

    return 1;
}

/* 初期化部メイン */
void *NewWavingClothModelW_withoutDATAFILE_called(int address,int *model_name,int model_name_size,
												  int sample_num,
												  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
												  int boundmodel_name,float oval_param,
												  int force_disp_flag)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),InitActW,DieWithoutDATAFILE) ;
		GV_ActorEX(&(work->actor));
	
		work->address=address;
		if(!GetResourcesW_withoutDATAFILE_called(work,model_name,model_name_size,sample_num,
												 target,objnum,x,r,
												 boundmodel_name,oval_param,force_disp_flag)){

			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}



#if 0

void *NewWavingJacketModelW_called(int model_name,int sample_num,OBJECT *target,float oval_param)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),ActJacketW,Die) ;
		GV_ActorEX(&(work->actor));
	
		work->address=0;
		if(!GetResourcesW_called(work,model_name,sample_num,
								 target,HUMAN21_MUNE,NULL,NULL,oval_param)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}

#endif
