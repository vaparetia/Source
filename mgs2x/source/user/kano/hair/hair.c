//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	hair.c
		髪の毛(長髪タイプ)シミュレーション

		関節０に、多数のロープモデルがつながっていると考える。
		ロープの同じ深さの関節どうしの関係に制限をつけるかどうかは、
		後々考えていくことにする。
		本当は、関節を動かして且つ頂点アニメさせたかったが、
		それを行なうと処理時間が大変である。
		それよりも、捻りを演算することで、
		表面のうねりが出てくれたらいいのであるが、
		どうなるだろうか？

	2000/06/02 K.Kano
	$Id: hair.c,v 1.1.1.3 2002/11/19 11:43:11 Yoshizawa1 Exp $
*/


#include "hair.h"
#include "../rope/rope.h"

#include "../../mode/demo/libdemo.h"


#define	OBJECT_FLAG	\
	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE| \
	 DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_LATTERDRAW)


extern const SAMPLE_HAIR_PARAMETER hair_sample[];

#if 0

int Rope_GetModelName(int *name);

extern FVECTOR	G_wind;
int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );

#endif


static void KmsCalcFirst(HAIR_WORK *work)
{
	DG_DEF *def;
    DG_MDL *mdl;
	int n_models;
	int i,j;
	int parent;

	int n_parents=work->n_parents;


	def=work->kms->def;
	n_models=work->n_models;

	mdl=def->models;
	j=n_models;

	for(i=0;i<n_models;i++){

#if 1
		if(mdl[i].parent!=i-1){
			parent=i-1;

			fpu_ClearVector(work->lrots+j);
			work->lrots[j].vw=1.0f;

			work->t[j].vx=0.0f;
			work->t[j].vy=-HAIR_TAIL_LENGTH;
			work->t[j].vz=0.0f;
			work->t[j].vw=1.0f;

			fpu_AddVectors(work->x+j,work->x+parent,work->t+j);
			work->x[j].vw=1.0f;
			fpu_ClearVector(work->v+j);

			fpu_CopyUnitMatrix(&(work->wmat[j]));
			fpu_CopyVector((FVECTOR *)&(work->wmat[j].m[3][0]),work->x+j);

			work->parent[j]=parent;
			work->b_parent[i]=work->b_parent[parent];

#if 0
			printf("%d -> P:%d\n",j,parent);
#endif

			j++;
		}
#endif

		parent=mdl[i].parent;

		fpu_ClearVector(work->lrots+i);
		work->lrots[i].vw=1.0f;

		work->t[i].vx=mdl[i].tx;
		work->t[i].vy=mdl[i].ty;
		work->t[i].vz=mdl[i].tz;
		work->t[i].vw=1.0f;

		if(i<n_parents){
			if(work->tmat[i]) fpu_CopyVector(work->x+i,(FVECTOR *)&(work->tmat[i]->m[3][0]));
			else fpu_CopyVector(work->x+i,(FVECTOR *)&(work->root.m[3][0]));
			work->b_parent[i]=i;
		}
		else{
			fpu_AddVectors(work->x+i,work->x+parent,work->t+i);
			work->x[i].vw=1.0f;
			work->b_parent[i]=work->b_parent[parent];
		}

		fpu_ClearVector(work->v+i);

		fpu_CopyUnitMatrix(&(work->wmat[i]));
		fpu_CopyVector((FVECTOR *)&(work->wmat[i].m[3][0]),work->x+i);

		work->parent[i]=parent;

#if 0
		printf("%d -> P:%d\n",i,parent);
#endif

	}

	/* 最後のオブジェにもつける */
	parent=i-1;

	fpu_ClearVector(work->lrots+j);
	work->lrots[j].vw=1.0f;

	work->t[j].vx=0.0f;
	work->t[j].vy=-HAIR_TAIL_LENGTH;
	work->t[j].vz=0.0f;
	work->t[j].vw=1.0f;

	fpu_AddVectors(work->x+j,work->x+parent,work->t+j);
	fpu_ClearVector(work->v+j);

	fpu_CopyUnitMatrix(&(work->wmat[j]));
	fpu_CopyVector((FVECTOR *)&(work->wmat[j].m[3][0]),work->x+j);

	work->parent[j]=parent;
	work->b_parent[j]=work->b_parent[parent];

#if 0
	printf("%d -> P:%d\n",j,parent);
#endif

    {
		FVECTOR f;
		int i,j;

		for(j=n_models;j<work->n_ex_models;j++){
			f.vx=0.0f;
			f.vy=0.0f;
			f.vz=0.0f;

			i=j;
			while(i>=0){
				FVECTOR t;
				float n;
				int parent=work->parent[i];

				f.vy+=P_GRAVITY;

				fpu_CopyVector(&t,&(work->t[i]));

				n=f.vy/t.vy;
				fpu_MulVectorScaler(&t,&t,n);

#if 1
				work->base_t[i].vx=t.vx-f.vx;
				work->base_t[i].vy=0.0f;
				work->base_t[i].vz=t.vz-f.vz;
				work->base_t[i].vw=0.0f;
#else
				work->base_t[i].vx=0.0f;
				work->base_t[i].vy=0.0f;
				work->base_t[i].vz=0.0f;
				work->base_t[i].vw=0.0f;
#endif

				fpu_CopyVector(&f,&t);

				i=parent;
			}
		}
    }
}

int InitHair(HAIR_WORK *work,int model_name)
{
	DG_DEF *def;
    DG_MDL *mdl;
	int n_models;
	int n_ex_models;
	int i;

	if((def=(DG_DEF *)GV_GetCache(GV_CacheID(model_name,'k')))==NULL) return 0;
	mdl=def->models;

	n_models=def->n_x_models;
	n_ex_models=n_models;

#if 1
	for(i=0;i<n_models;i++,mdl++){
		if(i-1!=mdl->parent) n_ex_models++;
	}
	n_ex_models++;
#endif

	if((work->kms=DG_MakeObjs(def,OBJECT_FLAG,0))==NULL) return 0;
	DG_QueueObjs(work->kms);
	DG_InvisibleObjs(work->kms);

	if((work->x=(FVECTOR *)GV_Malloc((sizeof(FMATRIX)+
									  sizeof(FVECTOR)*5+
									  sizeof(int)*2)*n_ex_models))==NULL){

		DG_DequeueObjs(work->kms);
		DG_FreeObjs(work->kms);
		return 0;
	}
	work->v=work->x+n_ex_models;
	work->base_t=work->v+n_ex_models;
	work->t=work->base_t+n_ex_models;
	work->lrots=work->t+n_ex_models;
	work->wmat=(FMATRIX *)(work->lrots+n_ex_models);
	work->parent=(int *)(work->wmat+n_ex_models);
	work->b_parent=work->parent+n_ex_models;

	work->n_models=n_models;
	work->n_ex_models=n_ex_models;

	work->invisible_flags=DG_FLAG_INVISIBLE;

	// KmsCalcFirst(work);
	// CalcHairCoordinate(work);

	return 1;
}

int InitHair_Wireless(HAIR_WORK *work,DG_DEF *def,int chanl_num)
{
    DG_MDL *mdl;
	int n_models;
	int n_ex_models;
	int i;

	mdl=def->models;

	n_models=def->n_x_models;
	n_ex_models=n_models;

#if 1
	for(i=0;i<n_models;i++,mdl++){
		if(i-1!=mdl->parent) n_ex_models++;
	}
	n_ex_models++;
#endif

	if((work->kms=DG_MakeObjs(def,OBJECT_FLAG|DG_FLAG_INVISIBLE2|
							  DG_FLAG_INVISIBLE3,chanl_num))==NULL) return 0;
	DG_QueueObjs(work->kms);
	DG_InvisibleObjs(work->kms);

	if((work->x=(FVECTOR *)GV_Malloc((sizeof(FMATRIX)+
									  sizeof(FVECTOR)*5+
									  sizeof(int)*2)*n_ex_models))==NULL){

		DG_DequeueObjs(work->kms);
		DG_FreeObjs(work->kms);
		return 0;
	}
	work->v=work->x+n_ex_models;
	work->base_t=work->v+n_ex_models;
	work->t=work->base_t+n_ex_models;
	work->lrots=work->t+n_ex_models;
	work->wmat=(FMATRIX *)(work->lrots+n_ex_models);
	work->parent=(int *)(work->wmat+n_ex_models);
	work->b_parent=work->parent+n_ex_models;

	work->n_models=n_models;
	work->n_ex_models=n_ex_models;

	work->invisible_flags=(DG_FLAG_INVISIBLE0<<chanl_num);

	// KmsCalcFirst(work);
	// CalcHairCoordinate(work);

	return 1;
}


void ExitHair(HAIR_WORK *work)
{
	if(work->kms!=NULL){
		DG_DequeueObjs(work->kms);
		DG_FreeObjs(work->kms);
	}
	if(work->x!=NULL) GV_Free(work->x);
}


void MoveHair(HAIR_WORK *work,HZX_GROUP_ID hzx_id)
{
	DG_OBJS *objs=work->kms;
	int size=work->n_models;
	int n_ex_models=work->n_ex_models;
	int i;

	FMATRIX *tmat=work->tmat[work->n_parents-1];
	int n_parents=work->n_parents;

    struct _scratchpad {
		FMATRIX rmat[N_OBJ_MATRIX];

		FVECTOR tvec0,tvec1,tvec2,tvec3;
		FVECTOR root[N_OBJ_MATRIX];

		FVECTOR gravity;

		FMATRIX *inv_m;

		FVECTOR *f;
		FVECTOR *t;
		FVECTOR *ma;
		FVECTOR *x;
		FVECTOR *arots;

		FMATRIX *world;

		HZX_FLR flrs[2];
		int flratrs[2];
		float h;
		int flrflag;
    };
    struct _scratchpad *scratchpad;

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));

    if(work->target){
		int tsize=work->target->objs->n_models;
		scratchpad->inv_m  = (FMATRIX *)ScratchpadAlloc2(sizeof(FMATRIX)*tsize);
    }

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc2((sizeof(FVECTOR)*5+sizeof(FMATRIX))*n_ex_models);
    scratchpad->t      = scratchpad->f+n_ex_models;
    scratchpad->ma     = scratchpad->t+n_ex_models;
    scratchpad->x      = scratchpad->ma+n_ex_models;
    scratchpad->arots  = scratchpad->x+n_ex_models;
    scratchpad->world  = (FMATRIX *)(scratchpad->arots+n_ex_models);


	/* 重力の修正 */
	{
//		static const FVECTOR g={ 0.0f, P_GRAVITY, 0.0f, 0.0f, }; //BP JG - runtime 50/60
      static FVECTOR g={ 0.0f, 0.0f, 0.0f, 0.0f, };
      g.y = P_GRAVITY;

		vu0_Ldv0((FVECTOR *)&g);

		if(work->calc_angree_frame ||
		   work->calc_angree_fade_frame){

			/* 髪の毛の逆立ちは、逆方向の重力で実現 */
			vu0_Mulv0a(-work->calc_angree_param);
		}

		vu0_Stv0(&(scratchpad->gravity));
	}


    if(tmat){
		for(i=0;i<n_parents;i++){
			vu0_Ldm0(work->tmat[i]);
			vu0_Ldm1(&(work->root));

			vu0_Mulm2m0m1();

			vu0_Stm2(&(scratchpad->rmat[i]));
		}
    }
    else{
		fpu_CopyMatrix(&(scratchpad->rmat[0]),&(work->root));
    }

	for(i=0;i<n_parents;i++){
		MT_MatToQuat(&(scratchpad->root[i]),&(scratchpad->rmat[i]));
	}

	scratchpad->flrflag=0;
    if(work->collision_flag){
		if((scratchpad->flrflag
			=HZX_LevelHazardCheck(hzx_id,(FVECTOR *)&(scratchpad->rmat[0].m[3][0]),
								  HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0))!=0){

			HZX_GetLevelHazard(scratchpad->flrs,scratchpad->flratrs);
		}

		if(work->target){
			DG_OBJS *tobjs=work->target->objs;
			int tsize=work->target->objs->n_models;

			for(i=0;i<tsize;i++){
				FastInverseMatrix(scratchpad->inv_m+i,&(tobjs->objs[i].world));
			}
		}
    }


	/* 前位置のスクラッチにコピー */
	for(i=0;i<n_ex_models;i++){
		fpu_CopyVector(&(scratchpad->x[i]),&(work->x[i]));
	}


#if 0
	/* 移動の何割かは反映しないようにしてみる。*/

	vu0_Ldv0((FVECTOR *)&(scratchpad->rmat[0].m[3][0]));
	vu0_Ldv1(&(scratchpad->x[0]));

	vu0_Subv0v1();

	vu0_Mulv0a(work->mov_rate);
	vu0_Setv2w1();

	for(i=0;i<n_ex_models;i++){
		vu0_Addv2v0v1();
		vu0_Ldv1(&(scratchpad->x[i+1]));
		vu0_Stv2(&(work->x[i]));
		vu0_Stv2(&(scratchpad->x[i]));
	}
#endif

#if 1
	/* 移動無視の際に参照する座標を変更し、
	   OBJECTの中心にする。*/

	if(work->target!=NULL){
		vu0_Ldv0((FVECTOR *)&(work->target->objs->world.m[3][0]));
		vu0_Ldv1(&(work->target_pos));

		vu0_Subv0v1();

		fpu_CopyVector(&(work->target_pos),
					   (FVECTOR *)&(work->target->objs->world.m[3][0]));
		vu0_Ldv1(&(scratchpad->x[0]));

#ifdef BP_PSX2_ASM
		asm volatile ("
	    qmtc2.ni	%0,vf16
	    qmtc2.ni	%1,vf17
	    vmulx.xz	vf1,vf1,vf16x
	    vmulx.y		vf1,vf1,vf17x
		" : : "r"(work->mov_rate),"r"(work->ymov_rate) );
#else
		{
			extern FVECTOR REG_vf1 ;
			REG_vf1.vx *= work->mov_rate ;
			REG_vf1.vz *= work->mov_rate ;
			REG_vf1.vy *= work->ymov_rate ;
		}
#endif

		vu0_Setv2w1();

		for(i=0;i<n_ex_models;i++){
			vu0_Addv2v0v1();
			vu0_Stv2(&(scratchpad->x[i]));
			vu0_Stv2(&(work->x[i]));
			vu0_Ldv1(&(scratchpad->x[i+1]));
		}
	}
#endif


    /* 力の計算 */
    for(i=n_parents;i<n_ex_models;i++){
		FVECTOR wind,*pwind=&wind;
		int parent=work->parent[i];
		int b_parent=work->b_parent[i];

		if(!OK_GetLocalWind(&(scratchpad->x[i]),&wind)){
			pwind=&G_wind;
		}

		vu0_Clrv1();
		vu0_Ldm0(&(scratchpad->rmat[b_parent]));

		/* 基準張力 */
		vu0_Ldv1(&(work->base_t[i]));

		/* 重力 */
		vu0_Ldv0(&(scratchpad->gravity));

		vu0_Mulv1m0v1();

		vu0_Mulv0a(work->m);
		vu0_Mulv1a(work->m);

		/* 風 */
		vu0_Ldv2(pwind);

		vu0_Addv1v0();

		vu0_Mulv2a(work->pa);

    	/* 空気抵抗 */
		vu0_Ldv0(&(work->v[i]));

		vu0_Addv1v2();

		vu0_Mulv0a(work->k);
		vu0_Addv1v0();


		/* 現クォータニオンから、ばねの戻す力を算出 */
		/* forceの加算 */

		/* 力のかかり方は、現在の回転を打ち消すようにかからなければ
		   ならない。そこで、現在の回転量と回転向きが必要となる。
		   回転向き、回転量は、ともにローカルクオータニオン
		   そのものの事であるので、ワークとしてlrotsを用意し、
		   前回の回転量を保存しておく事とする。*/

		/* cosine = (work->rots+i)->vw;             */
		/* sine^2 = 1-cosine^2                      */

		/* f = ( lq * t ) * sine^2 * a    ( )内は外積
		   lq ... ローカルクオータニオンの軸方向ベクトル
		   t  ... 前回の自位置と親位置の差分( = x - xp )
		   a  ... 係数 */

		if(parent>=n_parents){
			int pparent=work->parent[parent];
			float len;
			float sine,cosine;
			FVECTOR *nx,*px;


			nx=&(scratchpad->x[i]);
			px=&(scratchpad->x[parent]);

			vu0_Ldv0(nx);
			vu0_Ldv2(px);

			vu0_Subv0v2();

			vu0_Ldv2(&(work->lrots[parent]));
			vu0_Ldm0(&(work->wmat[pparent]));
			vu0_Setv2w0();

			cosine=work->lrots[parent].vw;
			sine=1.0f-cosine*cosine;
			// sine=fpu_Sqrt(sine);

			vu0_Mulv2m0v2();

			sine*=work->deg_param;

			vu0_OuterProductv0v2();
			// vu0_OuterProductv0v2v0();

			len=vu0_VectorLength2v0();
			len=fpu_Rsqrt(len,sine);
			vu0_Mulv0a(len);

			// printf("llll = %f\n",len);
		}

		vu0_Addv1v0();


		/* VU0が使われるため値を保存する */
		vu0_Stv1(&(scratchpad->f[i]));

		/* 当たり判定 */
        if(work->collision_flag){
			if(work->target){
				float l;

				if(work->collision_flag<0){
					if(work->boundmodel){
						if(!CalcObjsCollisionWithInvM2(&(scratchpad->tvec0),&(scratchpad->x[i]),
													   work->boundmodel,work->target->objs,
													   work->param_oval,
													   scratchpad->inv_m)) goto next;
					}
					else{
						if(!CalcObjsCollisionWithInvM(&(scratchpad->tvec0),&(scratchpad->x[i]),
													  work->target->objs,work->param_oval,
													  scratchpad->inv_m)) goto next;
					}
				}
				else{
					int n_objs=work->collision_flag;
					int ll;
					int flag=0;

					fpu_CopyVector(&(scratchpad->tvec0),&(scratchpad->x[i]));

					if(work->boundmodel){
						for(ll=0;ll<n_objs;ll++){
							int objnum=work->collision_objs[ll];

							flag|=CalcObjCollisionWithInvM2(&(scratchpad->tvec0),&(scratchpad->tvec0),
															work->boundmodel,work->target->objs,objnum,
															work->param_oval,
															scratchpad->inv_m+objnum);
						}
					}
					else{
						for(ll=0;ll<n_objs;ll++){
							int objnum=work->collision_objs[ll];

							flag|=CalcObjCollisionWithInvM(&(scratchpad->tvec0),&(scratchpad->tvec0),
														   work->target->objs,objnum,work->param_oval,
														   scratchpad->inv_m+objnum);
						}
					}

					if(!flag) goto next;
				}

				// printf("Check 1\n");

				vu0_Ldv0(&(scratchpad->tvec0));
				vu0_Ldv1(&(scratchpad->x[i]));
				vu0_Stv0(&(scratchpad->x[i]));
				vu0_Subv0v1();
				vu0_Ldv2(&(work->v[i]));
				l=vu0_VectorLength2v0();
				l=fpu_Rsqrt(l,1.0f);
				vu0_Mulv0a(l);
				l=vu0_InnerProductv0v2();
				vu0_Ldv1(&(scratchpad->f[i]));
				vu0_Mulv0a(l*work->m*work->we);
				vu0_Addv1v0();
				vu0_Stv1(&(scratchpad->f[i]));

			next:
				/* 床の判定 */
				if((scratchpad->flrflag & 1) && work->wl>-(int)CVM2N(2.0f)){

					HZX_SlopeFloorLevel(&(scratchpad->h),&(scratchpad->x[i]),scratchpad->flrs);
					scratchpad->h+=work->wl;

					if(scratchpad->x[i].vy<scratchpad->h){
						static const FVECTOR v={ 0.0f,1.0f,0.0f,0.0f, };

						/* 保存した値を再格納 */
						vu0_Ldv1(&(scratchpad->f[i]));

						scratchpad->x[i].vy=scratchpad->h;

						vu0_Ldv0(&v);
						vu0_Mulv2v0a(work->v[i].vy*work->m*work->we);
						vu0_Addv1v2();

						vu0_Stv1(&(scratchpad->f[i]));
					}
				}
			}
			else{
				int fhzd=HZX_NearHazardCheck(hzx_id,&(work->x[i]),
											 work->wl, /* 検出半径 */
											 HZX_CHK_ALL,0,
											 work->wl /* 反発半径 */);

				if(fhzd){
					float s,t;

					HZX_GetReactVector(&(scratchpad->tvec0));

					/* 保存した値を再格納 */
					vu0_Ldv1(&(scratchpad->f[i]));

					vu0_Ldv0(&(scratchpad->tvec0));
					vu0_Ldv2(&(work->x[i]));
					s=vu0_VectorLength2v0();
					vu0_Addv2v0();
					vu0_Stv2(&(work->x[i]));

					vu0_Ldv2(&(work->v[i]));
					t=vu0_InnerProductv0v2();
					vu0_Mulv2v0a(t/s*work->m*work->we);
					vu0_Addv1v2();

					vu0_Stv1(&(scratchpad->f[i]));
				}

				if(HZX_LevelHazardCheck(hzx_id,&(work->x[i]),
										HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0)){

					scratchpad->h=HZX_GetFloorLevel()+work->wl;

				   if(work->x[i].vy<scratchpad->h){
					   static const FVECTOR v={ 0.0f,1.0f,0.0f,0.0f, };

					   /* 保存した値を再格納 */
					   vu0_Ldv1(&(scratchpad->f[i]));

					   work->x[i].vy=scratchpad->h;
					   vu0_Ldv0(&v);
					   vu0_Mulv2v0a(work->v[i].vy*work->m*work->we);
					   vu0_Addv1v2();

					   vu0_Stv1(&(scratchpad->f[i]));
				   }
				}
			}
		}
    }


    /* 加速度の算出 */
    {
		float s,t;

		// printf("Check 2\n");

		for(i=0;i<n_parents;i++){
			fpu_CopyVector(&(scratchpad->x[i]),(FVECTOR *)&(scratchpad->rmat[i].m[3][0]));
			fpu_CopyVector(&(work->x[i]),(FVECTOR *)&(scratchpad->rmat[i].m[3][0]));
		}

		for(i=n_ex_models-1;i>=n_parents;i--){
			int parent_i=work->parent[i];

#if 0
			printf("f  = %f %f %f\n",
				   scratchpad->f[i].vx,scratchpad->f[i].vy,scratchpad->f[i].vz);
#endif

			vu0_Ldv0(&(scratchpad->x[i]));
			vu0_Ldv1(&(scratchpad->x[parent_i]));
			vu0_Ldv2(&(scratchpad->f[i]));

			vu0_Subv0v1();

			/* 正規化 + 内積 + 張力算出 */
			t=vu0_VectorLength2v0();
			s=vu0_InnerProductv0v2();

			vu0_Mulv0a(s/t);

			vu0_Ldv1(&(scratchpad->f[parent_i]));

			vu0_Subv2v0();

			vu0_Stv0(&(scratchpad->t[parent_i]));

			// vu0_Mulv2a(work->inv_m);

			vu0_Addv1v0();

			vu0_Stv2(&(scratchpad->ma[i]));
			vu0_Stv1(&(scratchpad->f[parent_i]));

#if 0
			printf("t  = %f %f %f\n",
				   scratchpad->t[parent_i].vx,scratchpad->t[parent_i].vy,scratchpad->t[parent_i].vz);
			printf("ma = %f %f %f\n",
				   scratchpad->ma[i].vx,scratchpad->ma[i].vy,scratchpad->ma[i].vz);
#endif
		}
    }


#if 0
    for(i=0;i<size;i++){
		printf("F%d = %f %f %f\n",i,
			   scratchpad->f[i].vx,scratchpad->f[i].vy,scratchpad->f[i].vz);
    }
#endif

#if 0
    for(i=0;i<size-1;i++){
		printf("T%d = %f %f %f\n",i,
			   scratchpad->t[i].vx,scratchpad->t[i].vy,scratchpad->t[i].vz);
    }
#endif

#if 0
    for(i=0;i<size;i++){
		printf("ma%d = %f %f %f\n",i,
			   scratchpad->ma[i].vx,scratchpad->ma[i].vy,scratchpad->ma[i].vz);
    }
#endif

#if 0
    for(i=0;i<size;i++){
		printf("V%d = %f %f %f\n",i,
			   work->v[i].vx,work->v[i].vy,work->v[i].vz);
    }
#endif



    // printf("Check 3\n");

    /* 速度、次位置の算出 */
    for(i=n_ex_models-1;i>=n_parents;i--){
		vu0_Ldv2(&(scratchpad->ma[i]));
		vu0_Ldv1(&(work->v[i]));

		vu0_Mulv2a(work->inv_m);

		vu0_Ldv0(&(work->x[i]));

		vu0_Addv1v2();
		vu0_Addv0v1();

		vu0_Stv0(&(scratchpad->x[i]));
    }

    // printf("Check 5\n");

	for(i=0;i<n_parents;i++){
		fpu_CopyMatrix(&(scratchpad->world[i]),&(scratchpad->rmat[i]));
		MT_MatToQuat(&(scratchpad->arots[i]),&(scratchpad->rmat[i]));
	}


    // fpu_CopyVector(&(scratchpad->tvec2),&(scratchpad->root));

    // fpu_CopyMatrix(&(objs->objs[0].world),&(scratchpad->rmat));

    /* 次位置から、次abs_rotsを算出 */
    for(i=n_parents;i<n_ex_models;i++){
		int parent,pparent,b_parent;
		float s,t;
		float a,b;

		/* 親を参照しないと一部正しくなくなってしまう */
		parent=work->parent[i];
		b_parent=work->b_parent[i];
		fpu_CopyVector(&(scratchpad->tvec0),&(work->t[i]));

		if(parent==0){
#if 1
			scratchpad->tvec0.vw=1.0f;

#if 0
			if(i!=1){
				fpu_CopyMatrix(&(scratchpad->world[i-1]),&(scratchpad->world[i-2]));
				fpu_CopyVector((FVECTOR *)&(scratchpad->world[i-1].m[3][0]),&(scratchpad->x[i-1]));
			}
#endif

			vu0_Ldv0(&(scratchpad->tvec0));
			vu0_Ldm0(&(scratchpad->rmat[b_parent]));
			vu0_Mulv0m0v0();

			vu0_Ldv1(&(scratchpad->x[i]));
			vu0_Subv1v0v1();
			vu0_Setv0w1();
			vu0_Stv0(&(scratchpad->x[i]));
			vu0_Stv0(&(work->x[i]));
			vu0_Stv1(&(work->v[i]));

			continue;
#else
			if(i!=1){
				fpu_CopyMatrix(&(scratchpad->world[i-1]),&(scratchpad->world[i-2]));
				fpu_CopyVector((FVECTOR *)&(scratchpad->world[i-1].m[3][0]),&(scratchpad->x[i-1]));
			}

			scratchpad->tvec0.vw=0.0f;

			vu0_Ldv0(&(scratchpad->x[i]));
			vu0_Ldv1(&(scratchpad->x[parent]));
			vu0_Ldv2(&(scratchpad->tvec0));
			vu0_Ldm0(&(scratchpad->rmat));
 
			vu0_Subv0v1();
			vu0_Mulv2m0v2();

			a=vu0_VectorLength2v0();
			b=vu0_VectorLength2v2();

			vu0_Stv0(&(scratchpad->tvec0));

			a/=b;

			goto next_calc;
#endif

		}


		scratchpad->tvec0.vw=0.0f;

		pparent=work->parent[parent];

		vu0_Ldv0(&(scratchpad->x[i]));
		vu0_Ldv1(&(scratchpad->x[parent]));
		vu0_Ldv2(&(scratchpad->tvec0));
		vu0_Ldm0(&(scratchpad->rmat[b_parent]));
 
		vu0_Subv0v1();
		vu0_Mulv2m0v2();
 
		s=a=vu0_VectorLength2v0();
		s*=(b=vu0_VectorLength2v2());
		t=vu0_InnerProductv0v2();
		vu0_Stv0(&(scratchpad->tvec0));
		vu0_OuterProductv2v0();
		s=fpu_Rsqrt(s,t);
		t=vu0_VectorLength2v2();
		t=fpu_Sqrt((1.0f-s)*0.5f/t);
		vu0_Mulv2a(t);
		s=fpu_Sqrt((1.0f+s)*0.5f);
 
		a/=b;
 
		vu0_Stv2(&(scratchpad->tvec1));
		scratchpad->tvec1.vw=s;
 
		MT_QuatMul(&(scratchpad->tvec1),&(scratchpad->tvec1),&(scratchpad->root[b_parent]));

#if 0

#if 0
#define LIMIT_COS	0.707106781f	/* cos 90/2 */
#define LIMIT_SIN	0.707106781f	/* sin 90/2 */
#elif 0
#define LIMIT_COS	0.866025404f	/* cos 60/2 */
#define LIMIT_SIN	0.500000000f	/* sin 60/2 */
#else
#define LIMIT_COS	0.965925826f	/* cos 30/2 */
#define LIMIT_SIN	0.258819045f	/* sin 30/2 */
#endif

		if(i>=n_parents){
			MT_QuatInverse(&(scratchpad->tvec3),&(scratchpad->tvec2));
			MT_QuatMul(&(scratchpad->tvec3),&(scratchpad->tvec3),&(scratchpad->tvec1));
			if(scratchpad->tvec3.vw<LIMIT_COS){
				float r;
				r=fpu_Rsqrt(1.0f-scratchpad->tvec3.vw*scratchpad->tvec3.vw,LIMIT_SIN);
				scratchpad->tvec3.vx*=r;
				scratchpad->tvec3.vy*=r;
				scratchpad->tvec3.vz*=r;
				scratchpad->tvec3.vw=LIMIT_COS;

				MT_QuatMul(&(scratchpad->tvec1),&(scratchpad->tvec2),&(scratchpad->tvec3));
			}
			fpu_CopyVector(&(scratchpad->tvec2),&(scratchpad->tvec1));
		}
#endif

		fpu_CopyVector(&(scratchpad->arots[parent]),&(scratchpad->tvec1));
		MT_QuatInverse(&(scratchpad->tvec2),&(scratchpad->arots[pparent]));

		MT_QuatMul(&(work->lrots[parent]),&(scratchpad->tvec2),&(scratchpad->arots[parent]));


		MT_QuatToMat(&(scratchpad->world[parent]),&(scratchpad->tvec1));
		fpu_CopyVector((FVECTOR *)&(scratchpad->world[parent].m[3][0]),&(scratchpad->x[parent]));


		if(work->lrots[parent].vw<0){
			work->lrots[parent].vx=-work->lrots[parent].vx;
			work->lrots[parent].vy=-work->lrots[parent].vy;
			work->lrots[parent].vz=-work->lrots[parent].vz;
			work->lrots[parent].vw=-work->lrots[parent].vw;
		}


// next_calc:
		/* 速度に制限を加えて、震えを抑制 */
		vu0_Ldv1(&(scratchpad->tvec0));
		vu0_Ldv0(&(scratchpad->x[parent]));


#if 0

		/* 20% */
#define MARGIN		0.02f
#define LIMIT		0.2f
#define MARGIN_BAND	0.05f

#elif 0

		/* 5% */
#define MARGIN		0.005f
#define LIMIT		0.05f
#define MARGIN_BAND	0.0125f

#else

		/* 1% */
#define MARGIN		0.001f
#define LIMIT		0.01f
#define MARGIN_BAND	0.0025f

#endif

#define UPPER_LIMIT_CHECK	(1.0f+LIMIT)
#define LOWER_LIMIT_CHECK	(1.0f-LIMIT)
#define UPPER_LIMIT		(UPPER_LIMIT_CHECK-MARGIN)
#define LOWER_LIMIT		(LOWER_LIMIT_CHECK+MARGIN)
#define UPPER_BAND_START	(UPPER_LIMIT_CHECK-MARGIN_BAND)
#define LOWER_BAND_START	(LOWER_LIMIT_CHECK+MARGIN_BAND)

		if(a>UPPER_LIMIT_CHECK*UPPER_LIMIT_CHECK){
			//a=fpu_Rsqrt(a,UPPER_LIMIT);
			a=UPPER_LIMIT/bp_sqrtf(a); //BP_MATH - emulate PS2 sqrtf
			vu0_Mulv1a(a);
			vu0_Addv0v1();
			vu0_Stv0(&(scratchpad->x[i]));

			a=MARGIN/MARGIN_BAND;
		}
		else if(a<LOWER_LIMIT_CHECK*LOWER_LIMIT_CHECK){
			//a=fpu_Rsqrt(a,LOWER_LIMIT);
			a=LOWER_LIMIT/bp_sqrtf(a); //BP_MATH - emulate PS2 sqrtf
			vu0_Mulv1a(a);
			vu0_Addv0v1();
			vu0_Stv0(&(scratchpad->x[i]));

			a=MARGIN/MARGIN_BAND;
		}
		else if(a>UPPER_BAND_START*UPPER_BAND_START){
			//a=fpu_Sqrt(a);
			a=bp_sqrtf(a); //BP_MATH - emulate PS2 sqrtf
			a=(UPPER_LIMIT_CHECK-a)/MARGIN_BAND;
		}
		else if(a<LOWER_BAND_START*LOWER_BAND_START){
			//a=fpu_Sqrt(a);
			a=bp_sqrtf(a); //BP_MATH - emulate PS2 sqrtf
			a=(a-LOWER_LIMIT_CHECK)/MARGIN_BAND;
		}
		else a=1.0f;

		vu0_Ldv0(&(scratchpad->x[i]));
		vu0_Ldv1(&(work->x[i]));
		vu0_Subv1v0v1();
		vu0_Setv0w1();

		vu0_Mulv1a(a);

		vu0_Stv0(&(work->x[i]));
		vu0_Stv1(&(work->v[i]));
    }

	for(i=0;i<size;i++){
		fpu_CopyMatrix(work->wmat+i,scratchpad->world+i);
		fpu_CopyMatrix(&(objs->objs[i].world),scratchpad->world+i);
	}

    fpu_CopyMatrix(&(objs->world),&(scratchpad->world[0]));


    ScratchpadFree2();
    if(work->target){
		ScratchpadFree2();
    }
    ScratchpadFree2();
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

typedef struct {
	GV_ACT_EX actor;
	HAIR_WORK hair;
	int name;
} Work;


static int SignalFunc( void *_work, int signal, int value )
{
	Work *work=(Work *)_work;

	if(signal==HAIR_SIGNAL_CALC_FLAG){
		if(!work->hair.calc_flag && value) work->hair.calc_flag=2;
		else work->hair.calc_flag=value;
	}
	return GV_DefaultSignalFunc(work,signal,value);
}

static void Act(Work *_work)
{
    HZX_GROUP_ID hzx_id=HZX_CurrentGroupID;
	HAIR_WORK *work=&(_work->hair);
	int calc_flag=work->calc_flag;

    GV_MSG *msg;
    int n_msg ;

	if(calc_flag==2){
		calc_flag=0;
		work->calc_flag=1;
	}

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
				work->mov_rate=hair_sample[d].mov_rate;
				work->ymov_rate=hair_sample[d].ymov_rate;
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
				work->calc_flag=arg;
				break;
			case -1:
				work->calc_flag^=1;
				break;
			}
			break;
		case 4:
			/* 髪の毛を逆立てる */
			{
				int arg2,arg3;

				arg2=*(msg->message+2);
				arg3=*(msg->message+3);

				switch(arg2){
				case 0:
					work->calc_angree_frame=arg2;
					if(arg3>0) work->calc_angree_fade_frame=arg3;
					break;
				default:
					work->calc_angree_param=(float)arg*0.01f;
					if(arg2<0) work->calc_angree_frame=-1;
					else work->calc_angree_frame=arg2;
					work->calc_angree_fade_frame=arg3;
					break;
				}
			}
			break;

		case 5:
			/* 位置の直接制御 */
			{
				FVECTOR p;

				p.vx=(float)arg;
				p.vy=(float)*(msg->message+2);
				p.vz=(float)*(msg->message+3);
				p.vw=1.0f;

				fpu_CopyVector((FVECTOR *)&(work->root.m[3][0]),&p);
			}
			break;
		case 6:
			/* 位置の相対制御 */
			work->root.m[3][0]+=(float)arg;
			work->root.m[3][1]+=(float)*(msg->message+2);
			work->root.m[3][2]+=(float)*(msg->message+3);
			break;
		case 7:
			/* 回転の直接制御 */
			{
				SVECTOR r;
				FVECTOR p;

				r.vx=arg;
				r.vy=*(msg->message+2);
				r.vz=*(msg->message+3);

				fpu_CopyVector(&p,(FVECTOR *)&(work->root.m[3][0]));

				DG_SetPos(&DG_UnitMatrix);
				DG_RotatePosZYX(&r);
				DG_GetPos(&(work->root));

				fpu_CopyVector((FVECTOR *)&(work->root.m[3][0]),&p);
			}
			break;
		case 8:
			/* 回転の相対制御 */
			{
				SVECTOR r;
				FVECTOR p;

				r.vx=arg;
				r.vy=*(msg->message+2);
				r.vz=*(msg->message+3);

				fpu_CopyVector(&p,(FVECTOR *)&(work->root.m[3][0]));

				DG_SetPos(&(work->root));
				DG_RotatePosZYX(&r);
				DG_GetPos(&(work->root));

				fpu_CopyVector((FVECTOR *)&(work->root.m[3][0]),&p);
			}
			break;
		}

		msg++;
		n_msg--;
    }



    if(work->target!=NULL){
		work->kms->group_id=work->target->objs->group_id;

		if(work->visible_flag){
			if(work->mesg_disp) work->kms->flag&=~work->invisible_flags;
			else work->kms->flag|=work->invisible_flags;
		}
		else if(work->target->evmobj!=NULL){
			if(work->target->evmobj->flag & DG_EVMOBJ_INVISIBLE){
				work->kms->flag|=work->invisible_flags;
			}
			else{
				if(work->mesg_disp) work->kms->flag&=~work->invisible_flags;
				else work->kms->flag|=work->invisible_flags;
			}
		}
		else if(work->target->objs->flag & DG_FLAG_INVISIBLE){
			work->kms->flag|=work->invisible_flags;
		}
		else if(work->target->objs->objs[work->tobjnum[work->n_parents-1]].flag & DG_FLAG_INVISIBLE){
			work->kms->flag|=work->invisible_flags;
		}
		else{
			if(work->mesg_disp) work->kms->flag&=~work->invisible_flags;
			else work->kms->flag|=work->invisible_flags;
		}
		fpu_CopyMatrix(&(work->light[0]),&(work->target->objs->light[0]));
		fpu_CopyMatrix(&(work->light[1]),&(work->target->objs->light[1]));
    }
	else{
		if(work->mesg_disp) work->kms->flag&=~work->invisible_flags;
		else work->kms->flag|=work->invisible_flags;

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

	if(work->calc_flag) MoveHair(work,hzx_id);

	if(work->calc_angree_frame>0) work->calc_angree_frame--;
	else if(work->calc_angree_fade_frame>0){
		work->calc_angree_param
			-=work->calc_angree_param/(float)(work->calc_angree_fade_frame);
		work->calc_angree_fade_frame--;
	}


#ifdef DEBUG_MODE
	if(work->debug_flag){
		void *NewKmsSkeletonTest(DG_OBJS *objs,int color);
		NewKmsSkeletonTest(work->kms,0x00ffff00);
	}
#endif
}

static void InitAct(Work *_work)
{
	KmsCalcFirst(&(_work->hair));
	CalcHairCoordinate(&(_work->hair));
	GV_ChangeActFunc(&(_work->actor),Act);
}

static void Die(Work *work)
{
	ExitHair(&(work->hair));
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 初期設定値を取得 */
static void GetOptionValue(Work *_work)
{
	HAIR_WORK *work=&(_work->hair);
    FVECTOR x;
    SVECTOR rot;

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
    }
    if(work->target!=NULL){
		work->tmat[0]=&(work->target->objs->objs[work->tobjnum[0]].world);

		// DG_SetLightMatrix(work->kms,work->target->objs->light);

		fpu_CopyMatrix(&(work->light[0]),&(work->target->objs->light[0]));
		fpu_CopyMatrix(&(work->light[1]),&(work->target->objs->light[1]));
    }
    else{
		DG_GetLightMatrix(&x,work->light);
    }
	DG_SetLightMatrix(work->kms,work->light);

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
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    int name;

    if(!Rope_GetModelName(&name)) return 0;
	if(!InitHair(&(work->hair),name)) return 0;
    GetOptionValue(work);

    return 1;
}

/* 初期化部メイン */
void *NewHairModel(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);
		GV_SetActorSignalFunc(work,SignalFunc);

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 初期設定値を取得 */
static void GetOptionValue_called(Work *_work,int sample_num,
								  OBJECT *target,int objnum,FVECTOR *px,SVECTOR *pr,
								  float oval_param,int collision_flag,unsigned char *collision_objs,
								  int visible_flag,int light_flag,int boundmodel_name)
{
	HAIR_WORK *work=&(_work->hair);
    FVECTOR x;
    SVECTOR rot;

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

    {
		int d=sample_num;

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
	if(px!=NULL) fpu_CopyVector(&x,px);
    x.vw=1.0f;

    /* 特定charaの特定objに付いていく */
	work->target=target;
	work->tobjnum[0]=objnum;

    if(work->target!=NULL){
		work->tmat[0]=&(work->target->objs->objs[work->tobjnum[0]].world);

		// DG_SetLightMatrix(work->kms,work->target->objs->light);

		fpu_CopyMatrix(&(work->light[0]),&(work->target->objs->light[0]));
		fpu_CopyMatrix(&(work->light[1]),&(work->target->objs->light[1]));
    }
    else{
		DG_GetLightMatrix(&x,work->light);
    }
	DG_SetLightMatrix(work->kms,work->light);


    /* 楕円球当たりのパラメータ */
	work->param_oval=oval_param;

    /* 当たり判定フラグ */
	work->collision_flag=collision_flag;

	/* 当たりの判定をするオブジェクトの番号の配列 */
	if(collision_flag>0 && collision_objs!=NULL){
		int nobjs=collision_flag;
		int i;

		// printf("N OBJS = %d\n",nobjs);

		for(i=0;i<nobjs;i++){
			int n=collision_objs[i];
			work->collision_objs[i]=n;

			// printf("OBJNUM = %d\n",n);
		}
		if(work->target!=NULL && work->collision_flag){
			work->collision_flag=nobjs;
		}
	}

    if(boundmodel_name!=0){
		work->boundmodel=(DG_DEF*)GV_GetCache(GV_CacheID(boundmodel_name,'k'));
    }

    /* 強制表示フラグ */
	work->visible_flag=visible_flag;

    /* ライトフラグ */
	work->light_flag=light_flag;

	if(pr!=NULL) rot=*pr;

    DG_SetPos(&DG_UnitMatrix);
    DG_RotatePosZYX(&rot);
    DG_GetPos(&(work->root));

    fpu_CopyVector((FVECTOR *)&(work->root.m[3][0]),&x);
}

/* 資源を獲得 */
static int GetResources_called(Work *work,int model_name,int sample_num,
							   OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
							   float oval_param,int collision_flag,unsigned char *collision_objs,
							   int visible_flag,int light_flag,int boundmodel_name)
{
	if(!InitHair(&(work->hair),model_name)) return 0;
    GetOptionValue_called(work,sample_num,
						  target,objnum,x,r,
						  oval_param,collision_flag,collision_objs,
						  visible_flag,light_flag,boundmodel_name);

    return 1;
}

/* 初期化部メイン */
void *NewHairModel_called(int name,int model_name,int sample_num,
						  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
						  float oval_param,int collision_flag,unsigned char *collision_objs,
						  int visible_flag,int light_flag,int boundmodel_name)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);
		GV_SetActorSignalFunc(work,SignalFunc);

		if(!GetResources_called(work,model_name,sample_num,
								target,objnum,x,r,
								oval_param,collision_flag,collision_objs,
								visible_flag,light_flag,boundmodel_name)){

			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


//collision_flag 2 or 3
//light_flag 0<=>6
void *NewHairModel_Demo(int name,int model_name,int sample_num,
                        OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
                        float oval_param,int collision_flag,
                        int visible_flag,int light_flag,int boundmodel_name)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
        work->name=name;

        GV_SetActor(&(work->actor),InitAct,Die) ;
        GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);
		GV_SetActorSignalFunc(work,SignalFunc);

        if(!GetResources_called(work,model_name,sample_num,
                                target,objnum,x,r,
                                oval_param,collision_flag,(unsigned char *)Demo_CollisionObjs_Rev,
                                visible_flag,light_flag,boundmodel_name)){

            GV_DestroyActor(work) ;
            return NULL ;
        }
    }
    return (void *)work ;
}


/* 資源を獲得 */
static int GetResources_Wireless(Work *work,DG_DEF *kms,DG_DEF *bound,int sample_num,
								 OBJECT *target,int light_flag,int chanl_num)
{
	static const unsigned char collision_objs[]={ HUMAN21_MUNE,HUMAN21_KUBI,HUMAN21_ATAMA, };

	if(!InitHair_Wireless(&(work->hair),kms,chanl_num)) return 0;
    GetOptionValue_called(work,sample_num,
						  target,HUMAN21_ATAMA,NULL,NULL,
						  1.0f,sizeof(collision_objs)/sizeof(collision_objs[0]),
						  (unsigned char *)collision_objs,1,light_flag,0);
	work->hair.boundmodel=bound;

    return 1;
}

/* 無線機用初期化部メイン */
void *NewHairModel_Wireless(int name,DG_DEF *kms,DG_DEF *bound,int sample_num,
							OBJECT *target,int light_flag,int chanl_num)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_DAEMON2,sizeof( Work )) ;
    if(work!=NULL) {
		work->name=name;

		GV_SetActor(&(work->actor),InitAct,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);
		GV_SetActorSignalFunc(work,SignalFunc);

		if(!GetResources_Wireless(work,kms,bound,sample_num,target,light_flag,chanl_num)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
