/*
	solmant.c
		ソリダスのマント消し

	2001/05/17 K.Kano
	$Id: solmant.c,v 1.1.1.3 2002/11/19 11:43:35 Yoshizawa1 Exp $
*/


#include "solmant.h"

#include "../hair/hair.h"
#include "../rope/rope.h"

#include "../../mode/demo/libdemo.h"


#define UV_FIRESTART	((int)(0.4f*0x1000))
#define UV_LIMIT		((int)(0.42f*0x1000))


extern const SAMPLE_HAIR_PARAMETER hair_sample[];

void *NewPointFire( FVECTOR* pos );


void EvmVertexAnimeSwitchBuffer(EVM_VERTEXANIM *anim)
{
	anim->evm->verts_ptr=anim->vertex[anim->buffer_flag];
	anim->evm->uvs0_ptr=anim->uvs0[anim->buffer_flag];
}

void InitEvmVertexAnime(DG_EVMOBJ *evm,EVM_VERTEXANIM *anim)
{
	DG_EVMPACK *pack=evm->packs;
	int n_steps=0;
	int i;

	anim->evm=evm;
	anim->buffer_flag=0;
	anim->vertex_org=(short *)(evm->verts_ptr);
	anim->normal_org=(short *)(evm->norms_ptr);
	anim->uvs0_org=(short *)(evm->uvs0_ptr);
	anim->uvs1_org=(short *)(evm->uvs1_ptr);
	anim->uvs2_org=(short *)(evm->uvs2_ptr);

	/* 頂点情報が消費するメモリを計算 */
	for(i=evm->n_packet;i>0;i--,pack++){
		n_steps+=pack->n_step;
	}

	if((anim->vertex[0]=GV_Malloc(n_steps*16*4))==NULL){
#ifdef DEBUG_MODE
		printf("EvmVertexAnime : Memory Overflow\n");
#endif
		ASSERT(0);
	}
	anim->vertex[1]=anim->vertex[0]+n_steps*16/sizeof(short);
	anim->uvs0[0]=anim->vertex[1]+n_steps*16/sizeof(short);
	anim->uvs0[1]=anim->uvs0[0]+n_steps*16/sizeof(short);

	anim->buffer_size=n_steps*16/sizeof(short [4]);

	for(i=0;i<n_steps;i++){
		/* メモリコピーはこちらの方が速いので代用 */
		fpu_CopyVector((FVECTOR *)(anim->vertex[0])+i,(FVECTOR *)(anim->vertex_org)+i);
		fpu_CopyVector((FVECTOR *)(anim->vertex[1])+i,(FVECTOR *)(anim->vertex_org)+i);
		fpu_CopyVector((FVECTOR *)(anim->uvs0[0])+i,(FVECTOR *)(anim->uvs0_org)+i);
		fpu_CopyVector((FVECTOR *)(anim->uvs0[1])+i,(FVECTOR *)(anim->uvs0_org)+i);
		anim->uvs0[1][i*8+0]=0x1000-anim->uvs0[1][i*8+0];
		anim->uvs0[1][i*8+4]=0x1000-anim->uvs0[1][i*8+4];
	}

	EvmVertexAnimeSwitchBuffer(anim);
}

void ExitEvmVertexAnime(EVM_VERTEXANIM *anim)
{
	anim->evm->verts_ptr=anim->vertex_org;
	anim->evm->norms_ptr=anim->normal_org;
	anim->evm->uvs0_ptr=anim->uvs0_org;
	anim->evm->uvs1_ptr=anim->uvs1_org;
	anim->evm->uvs2_ptr=anim->uvs2_org;

	if(anim->vertex[0]!=NULL){
		GV_Free(anim->vertex[0]);
	}
}

void LoadSolmant(void *file)
{
	ANIM_DELETEPOLY_HEADER *header=(ANIM_DELETEPOLY_HEADER *)file;
	int i;

	if(header->flags!=0) return;

	header->flags=1;
	for(i=0;i<header->n_frames;i++){
		header->frame_data[i]=(ANIM_DELETEPOLY *)((int)file+(int)(header->frame_data[i]));
	}
}

int ActAnimDeletePoly(void *file,int frame,int uv_speed,EVM_VERTEXANIM *anim)
{
	ANIM_DELETEPOLY_HEADER *header=(ANIM_DELETEPOLY_HEADER *)file;
	ANIM_DELETEPOLY *pframe,*nframe;
	ANIM_POLYPTR *ptr;
	short *mvert;
	short *vert;
	short *uv;
	unsigned char *weight;
	int n_verts;
	int i;
	int flag=anim->evm->def->type & 1;
	FMATRIX *mat=anim->evm->matrix[anim->evm->use_buffer];


	if(frame>header->n_frames) return 0;

	if(frame>0){
		pframe=header->frame_data[frame-1];
	}
	else{
		pframe=NULL;
	}
	if(frame<header->n_frames){
		nframe=header->frame_data[frame];
	}
	else{
		nframe=NULL;
	}

#if 0
	mvert=anim->vertex[anim->buffer_flag];

	if(pframe!=NULL){
		n_verts=pframe->n_verts;
		ptr=pframe->ptr;

		for(i=n_verts;i>0;i--,ptr++){
			vert=(short *)((int)mvert+ptr->ptr);
			vert[3]=0x8fff;
		}
	}
	if(nframe!=NULL){
		n_verts=nframe->n_verts;
		ptr=nframe->ptr;

		for(i=n_verts;i>0;i--,ptr++){
			FVECTOR pos;
			int j;

			vert=(short *)((int)mvert+ptr->ptr);

			// printf("ptr = 0x%08x\n",*ptr);
			// printf("0x%04x\n",vert[3]);

			vert[3]=0x8fff;

			pos.vx=vert[0-4*2];
			pos.vy=vert[1-4*2];
			pos.vz=vert[2-4*2];
			pos.vw=1.0f;

			vu0_Clrv2();
			vu0_Ldv0(&pos);
			vu0_Setv2w1();

			if(!flag){
				vu0_Mulv0a(1.0f/16.0f);
			}

			for(j=0;j<4;j++){
				vu0_Ldm0(mat+ptr->weight[0][j+4]);
				vu0_Mulv1m0v0();
				vu0_Mulv1a((float)(ptr->weight[0][j])/(128.0f*3.0f));

				// printf("%d\n",ptr->weight[0][j+4]);

				vu0_Addv2v1();
			}

#if 1
			pos.vx=vert[0-4*1];
			pos.vy=vert[1-4*1];
			pos.vz=vert[2-4*1];
			pos.vw=1.0f;

			vu0_Ldv0(&pos);

			if(!flag){
				vu0_Mulv0a(1.0f/16.0f);
			}

			for(j=0;j<4;j++){
				vu0_Ldm0(mat+ptr->weight[1][j+4]);
				vu0_Mulv1m0v0();
				vu0_Mulv1a(ptr->weight[1][j]/(128.0f*3.0f));
				vu0_Addv2v1();
			}

			pos.vx=vert[0-4*0];
			pos.vy=vert[1-4*0];
			pos.vz=vert[2-4*0];
			pos.vw=1.0f;

			vu0_Ldv0(&pos);

			if(!flag){
				vu0_Mulv0a(1.0f/16.0f);
			}

			for(j=0;j<4;j++){
				vu0_Ldm0(mat+ptr->weight[2][j+4]);
				vu0_Mulv1m0v0();
				vu0_Mulv1a(ptr->weight[2][j]/(128.0f*3.0f));
				vu0_Addv2v1();
			}
#endif

			vu0_Stv2(&pos);

			NewPointFire(&pos);
		}
	}
#endif

	vert=anim->vertex[anim->buffer_flag];
	uv=anim->uvs0[anim->buffer_flag];
	weight=anim->evm->weight_ptr;

	for(i=anim->buffer_size;i>0;i--,vert+=4,uv+=4,weight+=8){
		uv[1]+=uv_speed;
		if(uv[1]>0x1000) uv[1]=0x1000;

		if(vert[3]!=0x8fff){
			if(uv[1-4*0]>=UV_FIRESTART &&
			   uv[1-4*0]-uv_speed<=UV_FIRESTART){

				FVECTOR pos;
				int j;

				pos.vx=vert[0-4*2];
				pos.vy=vert[1-4*2];
				pos.vz=vert[2-4*2];
				pos.vw=1.0f;

				vu0_Clrv2();
				vu0_Ldv0(&pos);
				vu0_Setv2w1();

				if(!flag){
					vu0_Mulv0a(1.0f/16.0f);
				}

				for(j=0;j<4;j++){
					vu0_Ldm0(mat+weight[j+4-8*2]);
					vu0_Mulv1m0v0();
					vu0_Mulv1a((float)(weight[j-8*2])/(128.0f*3.0f));

					// printf("%d\n",ptr->weight[0][j+4]);

					vu0_Addv2v1();
				}

#if 1
				pos.vx=vert[0-4*1];
				pos.vy=vert[1-4*1];
				pos.vz=vert[2-4*1];
				pos.vw=1.0f;

				vu0_Ldv0(&pos);

				if(!flag){
					vu0_Mulv0a(1.0f/16.0f);
				}

				for(j=0;j<4;j++){
					vu0_Ldm0(mat+weight[j+4-8*1]);
					vu0_Mulv1m0v0();
					vu0_Mulv1a((float)(weight[j-8*1])/(128.0f*3.0f));
					vu0_Addv2v1();
				}

				pos.vx=vert[0-4*0];
				pos.vy=vert[1-4*0];
				pos.vz=vert[2-4*0];
				pos.vw=1.0f;

				vu0_Ldv0(&pos);

				if(!flag){
					vu0_Mulv0a(1.0f/16.0f);
				}

				for(j=0;j<4;j++){
					vu0_Ldm0(mat+weight[j+4-8*0]);
					vu0_Mulv1m0v0();
					vu0_Mulv1a((float)(weight[j-8*0])/(128.0f*3.0f));
					vu0_Addv2v1();
				}
#endif

				vu0_Stv2(&pos);

				NewPointFire(&pos);
			}

			if(uv[1-4*0]>=UV_LIMIT &&
			   uv[1-4*1]>=UV_LIMIT &&
			   uv[1-4*2]>=UV_LIMIT){

				vert[3]=0x8fff;
			}
		}
	}

	return 1;
}


/* -------------------------------------------------------------- */
/* -------------------------------------------------------------- */


typedef struct {
	GV_ACT_EX actor;
	HAIR_WORK hair;
	EVM_VERTEXANIM vanim;
	void *file;
	int frame;
	int name;
	int uv_speed;
} Work;


static void Act(Work *_work)
{
    HZX_GROUP_ID hzx_id=HZX_CurrentGroupID;
	HAIR_WORK *work=&(_work->hair);
	int calc_flag=work->calc_flag;


    GV_MSG *msg;
    int n_msg ;

    n_msg=GV_ReceiveMessage(_work->name,&msg);

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
		case 1:
			/* パラメータの変更 */
			{
				int d;

				d=arg;

				work->m=hair_sample[d].m;
				work->inv_m=1.0f/work->m;
				work->pa=hair_sample[d].pa;
				work->k=-hair_sample[d].k;
				work->deg_param=hair_sample[d].deg_param;
				work->we=-hair_sample[d].we;
				work->wl=hair_sample[d].wl;
			}
			break;
		case 2:
			/* ライトフラグの変更 */
			work->light_flag=arg;
			break;
		case 3:
			/* 髪の毛計算を行うか行わないかのフラグ */
			switch(arg){
			case 0:
			case 1:
				work->calc_flag=*(msg->message+1);
				break;
			case -1:
				work->calc_flag^=1;
				break;
			}
			break;
		case 4:
			if(_work->frame<0) _work->frame=0;
			break;
		}

		msg++;
		n_msg--;
    }



    if(work->target!=NULL){
		if(work->visible_flag){
			if(work->mesg_disp) work->evm->flag&=~work->invisible_flags;
			else work->evm->flag|=work->invisible_flags;
		}
		else if(work->target->evmobj!=NULL){
			if(work->target->evmobj->flag & work->invisible_flags){
				work->evm->flag|=work->invisible_flags;
			}
			else{
				if(work->mesg_disp) work->evm->flag&=~work->invisible_flags;
				else work->evm->flag|=work->invisible_flags;
			}
		}
		else if(work->target->objs->flag & DG_FLAG_INVISIBLE){
			work->evm->flag|=work->invisible_flags;
		}
		else if(work->target->objs->objs[work->tobjnum[work->n_parents-1]].flag & DG_FLAG_INVISIBLE){
			work->evm->flag|=work->invisible_flags;
		}
		else{
			if(work->mesg_disp) work->evm->flag&=~work->invisible_flags;
			else work->evm->flag|=work->invisible_flags;
		}

		fpu_CopyMatrix(&(work->light[0]),&(work->target->objs->light[0]));
		fpu_CopyMatrix(&(work->light[1]),&(work->target->objs->light[1]));
    }
	else{
		if(work->mesg_disp) work->evm->flag&=~DG_EVMOBJ_INVISIBLE;
		else work->evm->flag|=DG_EVMOBJ_INVISIBLE;

		DG_GetLightMatrix((FVECTOR *)&(work->root.m[3][0]),work->light);
	}

	if(work->light_flag){
		extern const float p_array[][2];
		float pp,np;
		int i;

		pp=p_array[work->light_flag][0];
		np=p_array[work->light_flag][1];

		for(i=0;i<3;i++){
			float c=0.0f;

			c+=work->light[1].m[0][i]*pp;
			work->light[1].m[0][i]*=np;
			c+=work->light[1].m[1][i]*pp;
			work->light[1].m[1][i]*=np;
			c+=work->light[1].m[2][i]*pp;
			work->light[1].m[2][i]*=np;

			work->light[1].m[3][i]+=c;
			if(work->light[1].m[3][i]>255.0f) work->light[1].m[3][i]=255.0f;
		}
	}

	if(GM_CheckGameStatus(STATE_DEMO) && (DM_EffectInitFlag & DM_EFFECT_INIT)){
		CalcHairCoordinate(work);
	}
	else if(!calc_flag && work->calc_flag){
		CalcHairCoordinate(work);
	}

	if(work->calc_flag) MoveHairEvm(work,hzx_id);

	if(_work->file!=NULL && _work->frame>=0){
		_work->vanim.buffer_flag=1-_work->vanim.buffer_flag;

		if(ActAnimDeletePoly(_work->file,_work->frame,_work->uv_speed,&(_work->vanim))){
			EvmVertexAnimeSwitchBuffer(&(_work->vanim));
			_work->frame++;
		}
		else{
			ExitEvmVertexAnime(&(_work->vanim));
			InitEvmVertexAnime(work->evm,&(_work->vanim));
			_work->frame=-1;
		}
	}

#ifdef DEBUG_MODE
	if(work->debug_flag){
		void *NewEvmSkeletonTest(DG_EVMOBJ *obj,int color);
		NewEvmSkeletonTest(work->evm,0x00ffff00);
	}
#endif

}

static void InitAct(Work *_work)
{
	HairEvmCalcFirst(&(_work->hair));
	CalcHairCoordinate(&(_work->hair));
	GV_ChangeActFunc(&(_work->actor),Act);
}

static void Die(Work *work)
{
	ExitEvmVertexAnime(&(work->vanim));
	ExitHairEvm(&(work->hair));
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 初期設定値を取得 */
static void GetOptionValue(Work *_work)
{
    FVECTOR x;
    SVECTOR rot;
	HAIR_WORK *work=&(_work->hair);

    work->m=0.001f;
    work->inv_m=1.0f/work->m;
    work->pa=0.001f;
    work->k=-0.0001f;
    work->we=-1.0f;
    work->wl=(int)CVC2N(10);
	work->deg_param=0.1f;
    work->param_oval=1.2f;
	work->mov_rate=0.0f;
	work->ymov_rate=0.0f;

    work->collision_flag=0;
    work->visible_flag=0;
	work->light_flag=0;
	work->mesg_disp=1;
	work->calc_flag=1;

	work->calc_angree_param=0.0f;
	work->calc_angree_frame=0;
	work->calc_angree_fade_frame=0;

    work->tmat[0]=NULL;
    fpu_CopyUnitMatrix(&(work->root));
    fpu_ClearVector(&x);
    rot=DG_ZeroSVector;

    work->target=NULL;
    work->tobjnum[0]=0;
	work->n_parents=1;

    work->boundmodel=NULL;

#ifdef DEBUG_MODE
	work->debug_flag=0;
#endif

	_work->uv_speed=2*0x1000/60;

    if(GCL_GetOption('d')!=NULL){
		int d=GCL_GetNextInt();

		work->m=hair_sample[d].m;
		work->inv_m=1.0f/work->m;
		work->pa=hair_sample[d].pa;
		work->k=-hair_sample[d].k;
		work->deg_param=hair_sample[d].deg_param;
		work->we=-hair_sample[d].we;
		work->wl=hair_sample[d].wl;
		work->mov_rate=hair_sample[d].mov_rate;
		work->ymov_rate=hair_sample[d].ymov_rate;
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
			work->target=ctrl->object;
			if(work->target==NULL) work->target=(OBJECT *)(ctrl+1);
		}
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		char *str;
		int cnt=0;

		while((str=GCL_NextStr())!=NULL){
			work->tobjnum[cnt]=GCL_GetInt(str);
			cnt++;
		}
		work->n_parents=cnt;

		printf("n_parents = %d\n",cnt);
    }
    if(work->target!=NULL){
		int i=work->n_parents;

		while(i>0){
			i--;
			work->tmat[i]=&(work->target->objs->objs[work->tobjnum[i]].world);
		}

		// DG_SetLightMatrix(work->evm,work->target->objs->light);
		// work->evm->light=work->target->objs->light;

		fpu_CopyMatrix(&(work->light[0]),&(work->target->objs->light[0]));
		fpu_CopyMatrix(&(work->light[1]),&(work->target->objs->light[1]));
    }
    else{
		DG_GetLightMatrix(&x,work->light);
    }
	// DG_SetLightMatrix(work->evm,work->light);
	work->evm->light=work->light;

    if(GCL_GetOption('b')!=NULL){
		int modelnum=GCL_GetNextInt();
		work->boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(modelnum,'k'));
    }

    /* 質量 */
    if( GCL_GetOption('m') != NULL ){
		int m=GCL_GetNextInt();
		float fm,inv_fm;

		/* グラム単位 */
		fm=(float)m*0.001f;
		inv_fm=1.0f/fm;

		work->m=fm;
		work->inv_m=inv_fm;
    }

    /* 風によって受ける力 */
    if( GCL_GetOption('p') != NULL ){
		int p=GCL_GetNextInt();
		float fp;

		fp=(float)p*0.000001f;

		work->pa=fp;
    }

    /* 速度に対する抵抗値 */
    if( GCL_GetOption('k') != NULL ){
		int k=GCL_GetNextInt();
		float fk;

		fk=(float)k*0.000001f;
		work->k=-fk;
    }

    /* 曲がりを正すパラメータ */
    if( GCL_GetOption('a') != NULL ){
		int a=GCL_GetNextInt();
		float fa;

		fa=(float)a*0.01f;
		work->deg_param=fa;
    }

    /* 壁に対する反発係数 */
    if( GCL_GetOption('e') != NULL ){
		int e=GCL_GetNextInt();
		float fe;

		fe=(float)e*0.001f+1.0f;
		work->we=-fe;
    }

    /* 壁検出距離 */
    if( GCL_GetOption('l') != NULL ){
		int l=GCL_GetNextInt();
		work->wl=l;
    }

    /* 楕円球当たりのパラメータ */
    if( GCL_GetOption('o') != NULL ){
		int o=GCL_GetNextInt();
		float fo;

		fo=(float)o*0.01f;
		work->param_oval=fo;
    }

	/* 全体の移動の何割を無視するかのパラメータ */
    if( GCL_GetOption('w') != NULL ){
		int w=GCL_GetNextInt();
		float fw;

		fw=(float)w*0.01f;
		work->mov_rate=fw;
    }
    if( GCL_GetOption('j') != NULL ){
		int w=GCL_GetNextInt();
		float fw;

		fw=(float)w*0.01f;
		work->ymov_rate=fw;
    }

    /* 当たり判定フラグ */
    if( GCL_GetOption('f') != NULL ){
		int f=GCL_GetNextInt();
        work->collision_flag=(f!=0 ? -1 : 0);
    }

	/* 当たりの判定をするオブジェクトの番号の配列 */
	if( GCL_GetOption('c') != NULL ){
		int nobjs=GCL_GetNextInt();
		int i;

		// printf("N OBJS = %d\n",nobjs);

		for(i=0;i<nobjs;i++){
			int n=GCL_GetNextInt();
			work->collision_objs[i]=n;

			// printf("OBJNUM = %d\n",n);
		}
		if(work->target!=NULL && work->collision_flag){
			work->collision_flag=nobjs;
		}
	}

    /* 強制表示フラグ */
    if( GCL_GetOption('v') != NULL ){
        work->visible_flag=1;
    }

    /* ライトフラグ */
    if( GCL_GetOption('q') != NULL ){
		int l=GCL_GetNextInt();
        work->light_flag=l;
    }

#ifdef DEBUG_MODE
	/* デバッグモード */
    if( GCL_GetOption('g') != NULL ){
        work->debug_flag=1;
    }
#endif

    if( GCL_GetOption( 'r' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		rot.vx=buf[0];
		rot.vy=buf[1];
		rot.vz=buf[2];
    }
    DG_SetPos(&DG_UnitMatrix);
    DG_RotatePosZYX(&rot);
    DG_GetPos(&(work->root));

    fpu_CopyVector((FVECTOR *)&(work->root.m[3][0]),&x);

	/* マント消去アニメ用のデータファイル名 */
    if( GCL_GetOption( 'h' ) != NULL ){
		int vanim_name=GCL_GetNextInt();
		int uv_frame;

		_work->file=(void *)GV_GetCache(GV_CacheID(vanim_name,'r'));
		LoadSolmant(_work->file);

		uv_frame=GCL_GetNextInt();
		if(uv_frame>0){
			_work->uv_speed=2*0x1000/uv_frame;
		}
	}
	_work->frame=-1;

	InitEvmVertexAnime(work->evm,&(_work->vanim));
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    int name;

    if(!Rope_GetModelName(&name)) return 0;
	if(!InitHairEvm(&(work->hair),name,1)) return 0;
    GetOptionValue(work);

    return 1;
}

/* 初期化部メイン */
void *NewSolMant(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


typedef struct {
	GV_ACT_EX actor;

	EVM_VERTEXANIM vanim;
	void *file;
	int frame;
	int name;
	int uv_speed;
} DemoWork;


static void DemoAct(DemoWork *work)
{
	work->vanim.buffer_flag=1-work->vanim.buffer_flag;

	if(ActAnimDeletePoly(work->file,work->frame,work->uv_speed,&(work->vanim))){
		EvmVertexAnimeSwitchBuffer(&(work->vanim));
		work->frame++;
	}
	else{
		// GV_DestroyActor(work);
	}
}

static void DemoDie(DemoWork *work)
{
	ExitEvmVertexAnime(&(work->vanim));
}

void *NewSolMant_Demo(int name,DG_EVMOBJ *evm,int vanim_name)
{
    DemoWork *work;
	void *file;

	file=(void *)GV_GetCache(GV_CacheID(vanim_name,'r'));
	if(file==NULL) return NULL;

	LoadSolmant(file);

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL){
		GV_SetActor(&(work->actor),DemoAct,DemoDie);
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		work->file=file;
		work->frame=0;
		work->name=name;
		work->uv_speed=2*0x1000/60;

		InitEvmVertexAnime(evm,&(work->vanim));
    }
    return (void *)work;
}


typedef struct {
	GV_ACT_EX actor;

	int name;
	DG_EVMOBJ *evm;
	int vanim_name;

	int count;
} TestWork;


void TestAct(TestWork *work)
{
	if(work->count>0){
		work->count--;
		return;
	}

	if(GV_PadData[1].press & PAD_L1){
		NewSolMant_Demo(work->name,work->evm,work->vanim_name);
		work->count=700;
	}
}

void TestDie(TestWork *work)
{
}

void *NewSolMant_Test(int name,DG_EVMOBJ *evm,int vanim_name)
{
    TestWork *work;

    work=(TestWork *)GV_NewActor(GV_ACTOR_MANAGER,sizeof(TestWork));
    if(work!=NULL){
		GV_SetActor(&(work->actor),TestAct,TestDie);
		GV_ActorEX(&(work->actor));

		work->name=name;
		work->evm=evm;
		work->vanim_name=vanim_name;

		work->count=0;
    }
    return (void *)work;
}

int ComSolMantStart(void)
{
	int name=GV_StrCode("ソリダスマント");
	int vanim_name=GV_StrCode("sol_mant.rva");
	DG_EVMOBJ *evm=NULL;

    if( GCL_GetOption('n') != NULL ){
		name=GCL_GetNextInt();
    }

    if( GCL_GetOption('f') != NULL ){
		vanim_name=GCL_GetNextInt();
    }

    if( GCL_GetOption('e') != NULL ){
		int ctrlname;
		CONTROL *ctrl;

		ctrlname=GCL_GetNextInt();
		ctrl=GM_SearchWhere(ctrlname);

		if(ctrl!=NULL){
			OBJECT *object=ctrl->object;

			if(object==NULL){
				object=(OBJECT *)(ctrl+1);
			}
			evm=object->evmobj;
		}
    }

	if(evm!=NULL){
		NewSolMant_Test(name,evm,vanim_name);
	}

	return 0;
}
