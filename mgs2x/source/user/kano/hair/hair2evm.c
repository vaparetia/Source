/*
	hair2evm.c
		髪の毛(長髪タイプ)シミュレーション

		髪の毛のように、形に最初から癖が付いているものには、
	  バンダナの動きはうまく働かないようである。
	  そこで、別の考えとして、角度を基準に動かすことにする。
	  つまり、基準位置に戻るようにではなく、基準角度に戻るように
	  計算を変更する。

	2000/08/29 K.Kano
	$Id: hair2evm.c,v 1.1.1.3 2002/11/19 11:43:11 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"
#include "utl_dma.h"


#include "../rope/rope.h"


extern const SAMPLE_MODEL_PARAMETER rope_sample[];


typedef struct {
	/* モデル */
	DG_EVMOBJ *evm;

	/* モデルデータ */
	EVM_DEF *def;

	FMATRIX light[2];

    /* モデルに取り付ける場合に用いるワールドマトリクス */
    FMATRIX root;
    FMATRIX *tmat;

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum;



    /* オブジェごとの質量の配列 */
	float m;

    /* 空気によって加えられる力の割合の配列 */
	float pa;

    /* オブジェごとの質量の逆数の配列 */
    float inv_m;

    /* 速度に対する抵抗値 */
    float k;

    /* 壁に対する反発係数 */
    float we;

    /* 壁検出距離 */
    int wl;

    /* 楕円球用パラメータ */
	float param_oval;

    /* 当たり判定を行なうかどうかのフラグ */
    int collision_flag;

    /* 強制的に表示するかどうかのフラグ */
    int visible_flag;

    /* バウンディングのチェックに使うモデル */
    DG_DEF *boundmodel;



	/* モデル数 */
	int n_models;

	/* 仮想モデルを含めた数 */
	int n_ex_models;

	/* 前回の回転、配列個数はn_ex_models */
	FVECTOR *lrots;

	/* 前回のワールドマトリクス、配列個数はn_ex_models */
	FMATRIX *wmat;

	/* 前回の位置、速度、配列個数はn_ex_models */
	FVECTOR *x;
	FVECTOR *v;

	/* 親から自分の原点までのオフセット、配列個数はn_ex_models */
	FVECTOR *t;

	/* 親のスケルトン番号、配列個数はn_ex_models */
	int *parent;

} HAIR_WORK;


#define HAIR_TAIL_LENGTH		50.0f

static void EvmCalcFirst(HAIR_WORK *work)
{
	EVM_DEF *def;
	int n_models;
	int i,j;
	EVM_SKEL *skel;
	int parent;


	def=work->def;
	n_models=work->n_models;

	skel=def->skeleton;
	j=n_models;
	for(i=0;i<n_models;i++){
		if(skel[i].parent==0){
			parent=i-1;

			fpu_ClearVector(work->lrots+j);
			work->lrots[j].vw=1.0f;

			work->t[j].vx=0.0f;
			work->t[j].vy=HAIR_TAIL_LENGTH;
			work->t[j].vz=0.0f;
			work->t[j].vw=1.0f;

			fpu_AddVectors(work->x+j,work->x+parent,work->t+j);
			fpu_ClearVector(work->v+j);

			fpu_CopyUnitMatrix(&(work->wmat[j]));
			fpu_CopyVector((FVECTOR *)&(work->wmat[j].m[3][0]),work->x+j);

			work->parent[j]=parent;

			j++;
		}

		parent=skel[i].parent;

		fpu_ClearVector(work->lrots+i);
		work->lrots[i].vw=1.0f;

		work->t[i].vx=skel[i].tx;
		work->t[i].vy=skel[i].ty;
		work->t[i].vz=skel[i].tz;
		work->t[i].vw=1.0f;

		fpu_AddVectors(work->x+i,work->x+parent,work->t+i);
		fpu_ClearVector(work->v+i);

		fpu_CopyUnitMatrix(&(work->wmat[i]));
		fpu_CopyVector((FVECTOR *)&(work->wmat[i].m[3][0]),work->x+i);

		work->parent[i]=parent;
	}

	/* 最後のオブジェにもつける */
	parent=i-1;

	fpu_ClearVector(work->lrots+j);
	work->lrots[j].vw=1.0f;

	work->t[j].vx=0.0f;
	work->t[j].vy=HAIR_TAIL_LENGTH;
	work->t[j].vz=0.0f;
	work->t[j].vw=1.0f;

	fpu_AddVectors(work->x+j,work->x+parent,work->t+j);
	fpu_ClearVector(work->v+j);

	fpu_CopyUnitMatrix(&(work->wmat[j]));
	fpu_CopyVector((FVECTOR *)&(work->wmat[j].m[3][0]),work->x+j);

	work->parent[j]=parent;
}

int InitHairEvm(HAIR_WORK *work,int name)
{
	EVM_DEF *def;
	int n_models;
	int n_ex_models;
	int i;
	EVM_SKEL *skel;

	work->lrots=NULL;

	if((def=(EVM_DEF *)GV_GetCache(GV_CacheID(name,'e')))==NULL) return 0;

	if((work->evm=DG_MakeEvmObj(def,0,0))==NULL){
		GV_Free(work);
		return 0;
	}
	DG_QueueEvmObj(work->evm);

	n_models=def->n_x_models;
	n_ex_models=n_models;

	skel=def->skeleton;
	for(i=0;i<n_models;i++,skel++){
		if(skel->parent==0) n_ex_models++;
	}
	n_ex_models++;

	work->def=def;
	work->n_models=n_models;
	work->n_ex_models=n_ex_models;


	if((work->lrots=(FVECTOR *)GV_Malloc((sizeof(FMATRIX)
										  +sizeof(FVECTOR)*4
										  +sizeof(int))*n_ex_models))==NULL) return 0;
	work->x=work->lrots+n_ex_models;
	work->v=work->x+n_ex_models;
	work->t=work->v+n_ex_models;
	work->wmat=(FMATRIX *)(work->t+n_ex_models);
	work->parent=(int *)(work->wmat+n_ex_models);
	
	EvmCalcFirst(work);

	return 1;
}

void ExitHairEvm(HAIR_WORK *work)
{
	DG_DequeueEvmObj(work->evm);
	DG_FreeEvmObj(work->evm);
	if(work->lrots!=NULL) GV_Free(work->lrots);
}


static void MoveHairEvm(HAIR_WORK *work,HZX_GROUP_ID hzx_id)
{
	extern FVECTOR	G_wind;
	int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );

	int n_models=work->n_models;
	int n_ex_models=work->n_ex_models;
	int i;

    struct _scratchpad {
		FMATRIX rmat;

		FVECTOR tvec0,tvec1,tvec2,tvec3;
		FVECTOR root;

		FMATRIX *inv_m;

		FVECTOR *f;
		FVECTOR *t;
		FVECTOR *ma;
		FVECTOR *x;
		FVECTOR *arots;

		FMATRIX *world;
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

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc2((sizeof(FVECTOR)*5
													  +sizeof(FMATRIX))*n_ex_models);
    scratchpad->t      = scratchpad->f+n_ex_models;
    scratchpad->ma     = scratchpad->t+n_ex_models;
    scratchpad->x      = scratchpad->ma+n_ex_models;
    scratchpad->arots  = scratchpad->x+n_ex_models;
    scratchpad->world  = (FMATRIX *)(scratchpad->arots+n_ex_models);

	/* スクラッチに位置をコピー */
	for(i=0;i<n_ex_models;i++){
		fpu_CopyVector(scratchpad->x+i,work->x+i);
	}

    if(work->tmat){
		vu0_Ldm0(work->tmat);
		vu0_Ldm1(&(work->root));

		vu0_Mulm2m0m1();

		vu0_Stm2(&(scratchpad->rmat));
    }
    else{
		fpu_CopyMatrix(&(scratchpad->rmat),&(work->root));
    }

    MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));


    if(work->collision_flag && work->target){
		DG_OBJS *tobjs=work->target->objs;
		int tsize=work->target->objs->n_models;

		for(i=0;i<tsize;i++){
			FastInverseMatrix(scratchpad->inv_m+i,&(tobjs->objs[i].world));
		}
    }



    /* 力の計算 */
    for(i=1;i<n_ex_models;i++){
		FVECTOR wind,*pwind=&wind;
		FVECTOR *nx,*px;
		int parent_i=work->parent[i];

		nx=&(scratchpad->x[i]);
		px=&(scratchpad->x[parent_i]);

		/* 風の取得 */
		if(!OK_GetLocalWind(nx,&wind)){
			pwind=&G_wind;
		}

		/* 重力計算はしない */

		/* 風 */
		vu0_Ldv2(pwind);

		vu0_Clrv1();

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

		/* f = lq * t * sine^2 * a
		   lq ... ローカルクオータニオンの軸方向ベクトル
		   t  ... 前回の自位置と親位置の差分( = x - xp )
		   a  ... 係数 */

#if 1
		if(parent_i>0){

#if 1

			int pparent_i=work->parent[parent_i];
			float len;
			float sine,cosine;

			vu0_Ldv0(nx);
			vu0_Ldv2(px);

			vu0_Subv0v2();

			vu0_Ldv2(&(work->lrots[parent_i]));
			vu0_Ldm0(&(work->wmat[pparent_i]));
			vu0_Setv2w0();

			cosine=work->lrots[parent_i].vw;
			sine=1.0f-cosine*cosine;

			vu0_Mulv2m0v2();

			/*  sine*=work->quat_a;  */
			/* sine*=100.0f/3.1425926f; */
			sine*=0.01f;

			vu0_OuterProductv0v2v0();

			len=vu0_VectorLength2v0();
			len=fpu_Rsqrt(len,sine);
			vu0_Mulv0a(len);

#else

			int pparent_i=work->parent[parent_i];
			float len;
			float sine,cosine;
			FVECTOR *t=work->t+parent_i;

			vu0_Ldv0(t);
			vu0_Ldm0(&(work->wmat[pparent_i]));

			vu0_Mulv0m0v0();

			cosine=work->lrots[parent_i].vw;
			sine=1.0f-cosine*cosine;

			vu0_Ldv2(nx);

			vu0_Subv0v2();

			/*  sine*=work->quat_a;  */
			/* sine*=100.0f/3.1425926f; */
			sine*=0.1f;

			len=vu0_VectorLength2v0();
			len=fpu_Rsqrt(len,sine);
			vu0_Mulv0a(len);

#endif

		}
#endif

		vu0_Addv1v0();


		/* VU0が使われるため値を保存する */
		vu0_Stv1(&(scratchpad->f[i]));

#if 0
		printf("%f %f %f\n",scratchpad->f[i].vx,scratchpad->f[i].vy,scratchpad->f[i].vz);
#endif


#if 0
		/* 当たり判定 */
        if(work->collision_flag){
			if(work->target){
				float l;

				if(work->boundmodel){
					if(!CalcObjsCollisionWithInvM2(&(scratchpad->tvec0),nx,
												   work->boundmodel,work->target->objs,work->param_oval,
												   scratchpad->inv_m)) goto next;
				}
				else{
					if(!CalcObjsCollisionWithInvM(&(scratchpad->tvec0),nx,
												  work->target->objs,work->param_oval,
												  scratchpad->inv_m)) goto next;
				}

				// printf("Check 1\n");

				vu0_Ldv0(&(scratchpad->tvec0));
				vu0_Ldv1(nx);
				vu0_Stv0(nx);
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
				;

			}
			else{
				int fhzd=HZX_NearHazardCheck(hzx_id,nx,
											 work->wl, /* 検出半径 */
											 HZX_CHK_ALL,0,
											 work->wl /* 反発半径 */);

				float h;

				if(fhzd){
					float s,t;

					HZX_GetReactVector(&(scratchpad->tvec0));

					/* 保存した値を再格納 */
					vu0_Ldv1(&(scratchpad->f[i]));

					vu0_Ldv0(&(scratchpad->tvec0));
					vu0_Ldv2(nx);
					s=vu0_VectorLength2v0();
					vu0_Addv2v0();
					vu0_Stv2(nx);

					vu0_Ldv2(&(work->v[i]));
					t=vu0_InnerProductv0v2();
					vu0_Mulv2v0a(t/s*work->m*work->we);
					vu0_Addv1v2();

					vu0_Stv1(&(scratchpad->f[i]));
				}

				HZX_LevelHazardCheck(hzx_id,nx,
									 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
				h=HZX_GetFloorLevel()+work->wl;
				if(nx->vy<h){
					static const FVECTOR v={ 0.0f,1.0f,0.0f,0.0f, };

					/* 保存した値を再格納 */
					vu0_Ldv1(&(scratchpad->f[i]));

					nx->vy=h;
					vu0_Ldv0(&v);
					vu0_Mulv2v0a(work->v[i].vy*work->m*work->we);
					vu0_Addv1v2();

					vu0_Stv1(&(scratchpad->f[i]));
				}
			}
		}
#endif

    }


    fpu_CopyMatrix(&(scratchpad->world[0]),&(scratchpad->rmat));
	fpu_CopyVector(&(scratchpad->x[0]),(FVECTOR *)&(scratchpad->rmat.m[3][0]));

	fpu_ClearVector(scratchpad->arots+0);
	scratchpad->arots[0].vw=1.0f;


    /* 加速度の算出 */

#if 0

	/* 加速度を、現在のtxyzに対して垂直に限定する。
	   加速度の大きさを限定。
	   クォータニオン計算。*/

    for(i=1;i<n_ex_models;i++){
		FVECTOR *nx,*px;
		int parent_i=work->parent[i];
		float l,s;

		nx=&(scratchpad->x[i]);
		px=&(scratchpad->x[parent_i]);


		// t=scratchpad->x[i]-scratchpad->x[i-1];

		/* tを正規化して、加速度の長さをかける(加速度のt方向の値を算出) */

		/* 加速度から、加速度のt方向を引く */

		vu0_Ldv0(nx);
		vu0_Ldv1(px);
		vu0_Ldv2(&(scratchpad->f[i]));

#if 1
		vu0_Subv0v1();

		l=vu0_VectorLength2v0();
		s=vu0_InnerProductv0v2();

		vu0_Mulv0a(s/l);

		vu0_Subv2v0();
#endif

		vu0_Mulv2a(work->inv_m);

		vu0_Stv2(&(scratchpad->ma[i]));


#if 1
		printf("%f %f %f\n",scratchpad->ma[i].vx,scratchpad->ma[i].vy,scratchpad->ma[i].vz);
#endif

	}

#else

	/* 張力計算有り */
    {
		float s,t;

		// printf("Check 2\n");

		for(i=n_ex_models-1;i>0;i--){
			int parent_i=work->parent[i];

#if 1
			printf("f  = %f %f %f\n",
				   scratchpad->f[i].vx,scratchpad->f[i].vy,scratchpad->f[i].vz);
#endif

			vu0_Ldv0(&(work->x[i]));
			vu0_Ldv1(&(work->x[parent_i]));
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

#if 1
			printf("t  = %f %f %f\n",
				   scratchpad->t[parent_i].vx,scratchpad->t[parent_i].vy,scratchpad->t[parent_i].vz);
			printf("ma = %f %f %f\n",
				   scratchpad->ma[i].vx,scratchpad->ma[i].vy,scratchpad->ma[i].vz);
#endif
		}
    }

#endif


	for(i=1;i<n_ex_models;i++){
		FVECTOR *nx,*px,*t;
		int parent_i=work->parent[i];


		nx=&(scratchpad->x[i]);
		px=&(scratchpad->x[parent_i]);
		t=&(work->t[i]);

		if(parent_i==0){
			vu0_Ldv0(t);
			vu0_Ldm0(&(scratchpad->rmat));

			vu0_Mulv0m0v0();

			vu0_Stv0(nx);
		}
		else{
			int pparent_i=work->parent[parent_i];

			/* 加速度と速度と、次位置の計算 */
			vu0_Ldv0(&(scratchpad->ma[i]));
			vu0_Ldv1(&(work->v[i]));
			vu0_Ldv2(nx);

			vu0_Addv1v0();
			vu0_Addv2v1();

			vu0_Stv1(&(work->v[i]));
			vu0_Stv2(nx);


			/* 次の位置から、絶対、相対クォータニオンの算出 */
			UTL_MakeQuatM(scratchpad->arots+parent_i,px,nx,&(scratchpad->rmat),t);
			MT_QuatInverse(&(scratchpad->tvec0),scratchpad->arots+pparent_i);

			MT_QuatMul(work->lrots+parent_i,&(scratchpad->tvec0),scratchpad->arots+parent_i);


			/* 絶対クオータニオンからworldマトリクスを計算 */
			MT_QuatMul(&(scratchpad->tvec0),&(scratchpad->root),scratchpad->arots+parent_i);
			MT_QuatToMat(scratchpad->world+parent_i,&(scratchpad->tvec0));

			fpu_CopyVector((FVECTOR *)&(scratchpad->world[parent_i].m[3][0]),px);

			if(work->lrots[parent_i].vw<0){
				work->lrots[parent_i].vx=-work->lrots[parent_i].vx;
				work->lrots[parent_i].vy=-work->lrots[parent_i].vy;
				work->lrots[parent_i].vz=-work->lrots[parent_i].vz;
				work->lrots[parent_i].vw=-work->lrots[parent_i].vw;
			}


			/* worldマトリクスから、下階層のオブジェクトの中心位置の計算 */
			vu0_Ldv0(t);
			vu0_Ldm0(scratchpad->world+parent_i);

			vu0_Mulv0m0v0();

#if 1
			vu0_Ldv1(nx);
			// vu0_Stv0((FVECTOR *)&((scratchpad->world+i)->m[3][0]));

#if 0
			vu0_Mulv0a(0.9f);
			vu0_Mulv1a(0.1f);
#else
			vu0_Mulv0a(0.5f);
			vu0_Mulv1a(0.5f);
#endif

			vu0_Addv0v1();
			vu0_Setv0w1();
#endif

			vu0_Stv0(nx);
		}
	}

#if 0
	printf("inv_m = %f\n",work->inv_m);
#endif

	/* スクラッチから計算結果をコピー */
	for(i=0;i<n_models;i++){
		fpu_CopyMatrix(work->wmat+i,scratchpad->world+i);
	}
	for(i=0;i<n_ex_models;i++){
		vu0_Ldv0(scratchpad->x+i);
		vu0_Ldv1(work->x+i);

		vu0_Subv1v0v1();

		vu0_Stv0(work->x+i);
		vu0_Stv1(work->v+i);
	}


	/* ワールドマトリクスをEVM用に変更 */
	{
		EVM_SKEL *skel=work->def->skeleton;
		FMATRIX *mat;
		int n_models=work->n_models;

		fpu_CopyMatrix(&(work->evm->world),&(scratchpad->world[0]));


#if 0
		/* TEST */
		for(i=1;i<n_models;i++){
			FVECTOR vec;
			int parent_i=work->parent[i];

			vec.vx=work->t[i].vx;
			vec.vy=work->t[i].vy;
			vec.vz=work->t[i].vz;
			vec.vw=1.0f;

			vu0_Ldv0(&vec);
			vu0_Ldm0(&(scratchpad->world[parent_i]));

			vu0_Mulv0m0v0();

			fpu_CopyMatrix(&(scratchpad->world[i]),&(scratchpad->world[0]));
			vu0_Stv0((FVECTOR *)&(scratchpad->world[i].m[3][0]));
		}
#endif


		for(i=0;i<n_models;i++){
			FVECTOR vec;

			vec.vx=-skel[i].rt_tx;
			vec.vy=-skel[i].rt_ty;
			vec.vz=-skel[i].rt_tz;
			vec.vw=1.0f;
			vu0_Ldv0(&vec);
			vu0_Ldm0(&(scratchpad->world[i]));
			vu0_Mulv0m0v0();
			vu0_Stv0((FVECTOR *)&(scratchpad->world[i].m[3][0]));
		}


      DG_Arm_SwitchEvmBuffer(work->evm);
		mat=work->evm->matrix[work->evm->use_buffer];

		for(i=0;i<n_models;i++){
			fpu_CopyMatrix(&(mat[i]),&(scratchpad->world[i]));
		}
	}



    ScratchpadFree2();
    if(work->target){
		ScratchpadFree2();
    }
    ScratchpadFree2();
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

typedef struct {
	GV_ACT	actor;
	HAIR_WORK hair;
} Work;


static void Act(Work *_work)
{
    HZX_GROUP_ID hzx_id=HZX_CurrentGroupID;
	HAIR_WORK *work=&(_work->hair);

    if(work->target!=NULL){
		if(work->visible_flag){
			work->evm->flag&=~DG_EVMOBJ_INVISIBLE;
		}
		else if(work->target->objs->flag & DG_FLAG_INVISIBLE){
			work->evm->flag|=DG_EVMOBJ_INVISIBLE;
		}
		else if(work->target->objs->objs[work->tobjnum].flag & DG_FLAG_INVISIBLE){
			work->evm->flag|=DG_EVMOBJ_INVISIBLE;
		}
		else{
			work->evm->flag&=~DG_EVMOBJ_INVISIBLE;
		}
    }

	MoveHairEvm(work,hzx_id);
}

static void Die(Work *work)
{
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

    work->param_oval=1.2f;

    work->collision_flag=0;
    work->visible_flag=0;

    work->tmat=NULL;
    fpu_CopyUnitMatrix(&(work->root));
    fpu_ClearVector(&x);
    rot=DG_ZeroSVector;

    work->target=NULL;
    work->tobjnum=0;

    work->boundmodel=NULL;

    if(GCL_GetOption('d')!=NULL){
		int d=GCL_GetNextInt();

		work->m=rope_sample[d].m;
		work->inv_m=1.0f/work->m;
		work->pa=rope_sample[d].pa;
		work->k=-rope_sample[d].k;
		work->we=-rope_sample[d].we;
		work->wl=rope_sample[d].wl;
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
			work->target=(OBJECT *)(ctrl+1);
		}
    }
    if( GCL_GetOption( 'z' ) != NULL ){
		work->tobjnum=GCL_GetNextInt();
    }
    if(work->target!=NULL){
		work->tmat=&(work->target->objs->objs[work->tobjnum].world);
		// DG_SetLightMatrix(work->evm,work->target->objs->light);
		work->evm->light=work->target->objs->light;
    }
    else{
		DG_GetLightMatrix(&x,work->light);
		// DG_SetLightMatrix(work->evm,work->light);
		work->evm->light=work->light;
    }

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

    /* 当たり判定フラグ */
    if( GCL_GetOption('f') != NULL ){
		int f=GCL_GetNextInt();
        work->collision_flag=(f!=0);
    }

    /* 強制表示フラグ */
    if( GCL_GetOption('v') != NULL ){
        work->visible_flag=1;
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
    DG_GetPos(&(work->root));

    fpu_CopyVector((FVECTOR *)&(work->root.m[3][0]),&x);
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
    int name;

    if(!Rope_GetModelName(&name)) return 0;
	if(!InitHairEvm(&(work->hair),name)) return 0;
    GetOptionValue(work);

    return 1;
}

/* 初期化部メイン */
void *NewEvmHairModel2(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
