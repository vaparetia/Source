//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ik_sub.c

	1999/09/02 K.Kano
	$Id: ik_sub.c,v 1.1.1.3 2002/11/19 11:43:13 Yoshizawa1 Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>

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


static void WorldCalc(FMATRIX *t,OBJECT *obj,int child)
{
    int parent;
    DG_MDL *mdl;
    FMATRIX mat;
    FVECTOR vec;

    vu0_Clrv0();
    fpu_ClearVector(&vec);

    MT_QuatToMat(t,&(obj->m_ctrl->abs_rots[child]));

    vu0_Setv0w1();

    mdl = obj->objs->def->models ;

    while ( child != 0 ){
		parent = mdl[child].parent ;
		MT_QuatToMat(&mat,&(obj->m_ctrl->abs_rots[parent]));

		vec.vx = mdl[child].tx ;
		vec.vy = mdl[child].ty ;
		vec.vz = mdl[child].tz ;

		vu0_Ldm0(&mat);
		vu0_Ldv1(&vec);
		vu0_Mulv1m0v1();
		vu0_Addv0v1();
		child=parent ;
    }

    vu0_Stv0((FVECTOR *)&(t->m[3][0]));
}

static void WorldCalc2(FMATRIX *t,FMATRIX *s,OBJECT *obj,int child)
{
    DG_MDL *mdl;
    FVECTOR vec;

    MT_QuatToMat(t,&(obj->m_ctrl->abs_rots[child]));

    mdl = obj->objs->def->models ;

    vec.vx = mdl[child].tx ;
    vec.vy = mdl[child].ty ;
    vec.vz = mdl[child].tz ;
    vec.vw = 1.0f;

    vu0_Ldm0(s);
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();

    vu0_Stv0((FVECTOR *)&(t->m[3][0]));
}


/* s : 支点
   t : 目標点
   n : 回転軸方向
   i,j : 回転X,Y軸
   l1,l2 : sからtまでを支える連結の長さ
   q1,q2 : 計算結果のクオータニオン
   */

static int IKsub(FVECTOR *s,FVECTOR *t,FVECTOR *n,FVECTOR *i,FVECTOR *j,
				 float l1,float l2,FVECTOR *q1,FVECTOR *q2,int flag)
{
    float x,y;
    float k1,k2;
    float c1,s1,c2,s2;
    float c1m,s1m;
    float sita1,sita2;
    float sita1m=0.0f,sita2m=0.0f;
    int sign1,sign2;

    vu0_Ldv1(s);
    vu0_Ldv2(t);
    vu0_Ldv0(j);
    vu0_Subv2v1();
    vu0_Ldv1(i);
    x=vu0_InnerProductv1v2();
    y=vu0_InnerProductv0v2();

    c2=(x*x+y*y-l1*l1-l2*l2)/(2*l1*l2);

    if(c2>1.0001f || c2<-1.0001f) return 0;
    if(c2>1.0f) c2=1.0f;
    if(c2<-1.0f) c2=-1.0f;

    sita2=acosf(c2);
    s2=sinf(sita2);
    c1m=-2.0f;
	s1m=2.0f;
    sita1m=7.0f;

#if 1
    for(sign2=0;sign2<2;sign2++,s2=-s2,sita2=-sita2){

		k1=l1+l2*c2;
		k2=l2*s2;

		c1=(k1*x+k2*y)/(k1*k1+k2*k2);

		if(c1>1.0001f || c1<-1.0001f) continue;
		if(c1>1.0f) c1=1.0f;
		if(c1<-1.0f) c1=-1.0f;

		sita1=acosf(c1);
		s1=sinf(sita1);

		for(sign1=0;sign1<2;sign1++,s1=-s1,sita1=-sita1){
			float tx,ty;

			tx=l1*c1+l2*(c1*c2-s1*s2);
			ty=l1*s1+l2*(c1*s2+s1*c2);

			if(tx<x+0.01f && tx>x-0.01f &&
			   ty<y+0.01f && ty>y-0.01f){

				if(flag){
					if(s1m>s1){
						/* sinが小さい方を優先する */
						s1m=s1;
						c1m=c1;
						sita1m=sita1;
						sita2m=sita2;
					}
				}
				else{
					if(c1m<c1 || sita1m>sita1){
						/* 補正角は0に近い方を優先する */
						/* 現在の曲がり方向を保持する方を優先する。*/
						c1m=c1;
						sita1m=sita1;
						sita2m=sita2;
					}
				}
			}
		}
    }
#else

#if 0
	if(s2<0){
		s2=-s2;
		sita2=-sita2;
	}
#endif

	{
		k1=l1+l2*c2;
		k2=l2*s2;

		c1=(k1*x+k2*y)/(k1*k1+k2*k2);

		if(c1>1.0001f || c1<-1.0001f) return 0;
		if(c1>1.0f) c1=1.0f;
		if(c1<-1.0f) c1=-1.0f;

		sita1=acosf(c1);
		s1=sinf(sita1);

		for(sign1=0;sign1<2;sign1++,s1=-s1,sita1=-sita1){
			float tx,ty;

			tx=l1*c1+l2*(c1*c2-s1*s2);
			ty=l1*s1+l2*(c1*s2+s1*c2);

			if(tx<x+0.01f && tx>x-0.01f &&
			   ty<y+0.01f && ty>y-0.01f){

				if(flag){
					if(s1m>s1){
						/* sinが小さい方を優先する */
						s1m=s1;
						c1m=c1;
						sita1m=sita1;
						sita2m=sita2;
					}
				}
				else{
					if(c1m<c1 || sita1m>sita1){
						/* 補正角は0に近い方を優先する */
						/* 現在の曲がり方向を保持する方を優先する。*/
						c1m=c1;
						sita1m=sita1;
						sita2m=sita2;
					}
				}
			}
		}
    }
#endif

    if(c1m<-1.0f) return 0;

    sita1m*=0.5f;
    sita2m*=0.5f;

    s1=sinf(sita1m);
    c1=cosf(sita1m);
    s2=sinf(sita2m);
    c2=cosf(sita2m);

    q2->vx=n->vx*s2;
    q2->vy=n->vy*s2;
    q2->vz=n->vz*s2;
    q2->vw=c2;

    q1->vx=n->vx*s1;
    q1->vy=n->vy*s1;
    q1->vz=n->vz*s1;
    q1->vw=c1;

    return 1;
}


#define LEG_R0	HUMAN21_MIGI_ASHI1
#define LEG_R1	HUMAN21_MIGI_ASHI2
#define LEG_R2	HUMAN21_MIGI_KAKATO
#define LEG_L0	HUMAN21_HIDARI_ASHI1
#define LEG_L1	HUMAN21_HIDARI_ASHI2
#define LEG_L2	HUMAN21_HIDARI_KAKATO


/* スネーク */
#if 1
#define SNAKE_ADJUST		(-CVMCM2N(0.0f,4.0f,0.0f))
#define SNAKE_HEELP			(CVMCM2N(0.0f,13.0f,0.0f)+SNAKE_ADJUST)
#define SNAKE_HEELP2		(SNAKE_HEELP+CVMCM2N(0.0f,6.0f,0.0f))
#else
#define SNAKE_ADJUST		(-CVMCM2N(0.0f,1.0f,0.0f))
#define SNAKE_HEELP			(CVMCM2N(0.0f,13.0f,0.0f)+SNAKE_ADJUST)
#define SNAKE_HEELP2		(SNAKE_HEELP+CVMCM2N(0.0f,3.0f,0.0f))
#endif
#if 0
#define SNAKE_D_HEELP		CVMCM2N(0.0f,3.0f,0.0f)
#else
#define SNAKE_D_HEELP		CVMCM2N(0.0f,4.0f,0.0f)
#endif
#define SNAKE_FOOTL			CVMCM2N(0.0f,16.0f,0.0f)

/* ライデン */
#define RAIDEN_ADJUST		(-CVMCM2N(0.0f,3.0f,0.0f))
#define RAIDEN_HEELP		(CVMCM2N(0.0f,13.7f,0.0f)+RAIDEN_ADJUST)
#define RAIDEN_HEELP2		(RAIDEN_HEELP+CVMCM2N(0.0f,0.0f,0.0f))
#define RAIDEN_D_HEELP		CVMCM2N(0.0f,0.0f,0.0f)
#define RAIDEN_FOOTL		CVMCM2N(0.0f,16.0f,0.0f)

/* エマ */
#define EMMA_ADJUST		(-CVMCM2N(0.0f,4.0f,0.0f))
#define EMMA_HEELP		(CVMCM2N(0.0f,7.7f,0.0f))
#define EMMA_HEELP2		(EMMA_HEELP+EMMA_ADJUST)
#define EMMA_D_HEELP		CVMCM2N(0.0f,3.0f,0.0f)
#define EMMA_FOOTL		CVMCM2N(0.0f,11.0f,0.0f)

/* ファットマン */
#define FAT_ADJUST		(-CVMCM2N(0.0f,6.0f,0.0f))
#define FAT_HEELP		(CVMCM2N(0.0f,13.7f,0.0f))
#define FAT_HEELP2		(FAT_HEELP+FAT_ADJUST)
#define FAT_D_HEELP		CVMCM2N(0.0f,3.0f,0.0f)
#define FAT_FOOTL		CVMCM2N(0.0f,24.0f,0.0f)


#if 1
#define MAGIC_NUMBER	CVC2N(6.0f)
#else
#define MAGIC_NUMBER	(foot.vz)
#endif


static const float heelp[]={
	SNAKE_HEELP,
	RAIDEN_HEELP,
	EMMA_HEELP,
	FAT_HEELP,
};

static const float heelp2[]={
	SNAKE_HEELP2,
	RAIDEN_HEELP2,
	EMMA_HEELP2,
	FAT_HEELP2,
};

static const float d_heelp[]={
	SNAKE_D_HEELP,
	RAIDEN_D_HEELP,
	EMMA_D_HEELP,
	FAT_D_HEELP,
};

static const float footl[]={
	SNAKE_FOOTL,
	RAIDEN_FOOTL,
	EMMA_FOOTL,
	FAT_FOOTL,
};


/* IKにより足の位置を計算する */
static void _Leg_IKcalc(CONTROL *control,OBJECT *obj,int param_num)
{
    struct _scratchpad {
		FMATRIX rmat;
		FMATRIX inv_rmat;
		FVECTOR root,inv_root;

		FMATRIX rlm[3];
		FMATRIX llm[3];
		FVECTOR rvt,lvt;

		FVECTOR tn,tr;
		FVECTOR tn0,tr0;

		FVECTOR s0,sn,vi,vj;
		FVECTOR q1,q2;
		float l1,l2;
		float rfh,lfh;
		int root_flag;
		int flag;
    };
#if 1
    struct _scratchpad * const scratchpad
		=(struct _scratchpad * const)(SCRPAD_ADDR+0x4000-sizeof(struct _scratchpad));
#else
    struct _scratchpad dummy;
    struct _scratchpad * const scratchpad=&dummy;
#endif
    FVECTOR foot;
    HZX_GROUP_ID hzx_id;
	int hzx_check_type=control->hzx_check_type;
	int flr_flag=control->flr_flag & ~HZX_FLOOR_IK;


	foot.vx=0.0f;
	foot.vy=-heelp[param_num];
	foot.vz=footl[param_num];
	foot.vw=1.0f;


    hzx_id=control->hzx_id;

    scratchpad->flag=0;
    scratchpad->root_flag=0;

    /* ルートマトリクスの取り出し */
    DG_GetPos(&(scratchpad->rmat));

    /* マトリクス計算 */
    WorldCalc(scratchpad->rlm+0,obj,LEG_R0);
    WorldCalc2(scratchpad->rlm+1,scratchpad->rlm+0,obj,LEG_R1);
    WorldCalc2(scratchpad->rlm+2,scratchpad->rlm+1,obj,LEG_R2);

    WorldCalc(scratchpad->llm+0,obj,LEG_L0);
    WorldCalc2(scratchpad->llm+1,scratchpad->llm+0,obj,LEG_L1);
    WorldCalc2(scratchpad->llm+2,scratchpad->llm+1,obj,LEG_L2);

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldm1(scratchpad->rlm+0);

    vu0_Mulm2m0m1();

    vu0_Ldm1(scratchpad->rlm+1);
    vu0_Stm2(scratchpad->rlm+0);

    vu0_Mulm2m0m1();

    vu0_Ldm1(scratchpad->rlm+2);
    vu0_Stm2(scratchpad->rlm+1);

    vu0_Mulm2m0m1();

    vu0_Ldm1(scratchpad->llm+0);
    vu0_Stm2(scratchpad->rlm+2);

    vu0_Mulm2m0m1();

    vu0_Ldm1(scratchpad->llm+1);
    vu0_Stm2(scratchpad->llm+0);

    vu0_Mulm2m0m1();

    vu0_Ldm1(scratchpad->llm+2);
    vu0_Stm2(scratchpad->llm+1);

    vu0_Mulm2m0m1();

    vu0_Stm2(scratchpad->llm+2);

    fpu_CopyVector(&(scratchpad->rvt),(FVECTOR *)&(scratchpad->rlm[2].m[3][0]));
    fpu_MulVectorScaler(&(scratchpad->s0),(FVECTOR *)&(scratchpad->rlm[2].m[2][0]),MAGIC_NUMBER);
    fpu_AddVectors(&(scratchpad->s0),&(scratchpad->rvt),&(scratchpad->s0));


	// printf("HZX CHECK TYPE = 0x%08x  0x%08x\n",control->hzx_check_type,control->flr_flag);


    /* 少し高いところに座標を設定し、確実に足元の床を計算する */
    scratchpad->s0.vy+=CVM2N(1.0f); /* 1 meter */

    HZX_LevelHazardCheck(hzx_id,&(scratchpad->s0),hzx_check_type,flr_flag);
    scratchpad->rfh=HZX_GetFloorLevel();

    fpu_CopyVector(&(scratchpad->lvt),(FVECTOR *)&(scratchpad->llm[2].m[3][0]));
    fpu_MulVectorScaler(&(scratchpad->s0),(FVECTOR *)&(scratchpad->llm[2].m[2][0]),MAGIC_NUMBER);
    fpu_AddVectors(&(scratchpad->s0),&(scratchpad->lvt),&(scratchpad->s0));

    /* 少し高いところに座標を設定し、確実に足元の床を計算する */
    scratchpad->s0.vy+=CVM2N(1.0f); /* 1 meter */

    HZX_LevelHazardCheck(hzx_id,&(scratchpad->s0),hzx_check_type,flr_flag);
    scratchpad->lfh=HZX_GetFloorLevel();

	/* 2001/6/19 Ken Kano
	   中心で検出した床の高さとの高低差があり過ぎる時は、
	   中心位置の床に合わせる */
	if(fpu_Abs(scratchpad->rfh-control->levels[0])>CVM2N(0.75f)){
		scratchpad->rfh=control->levels[0];
	}
	if(fpu_Abs(scratchpad->lfh-control->levels[0])>CVM2N(0.75f)){
		scratchpad->lfh=control->levels[0];
	}

	// printf("height = %f %f\n",scratchpad->rfh,scratchpad->lfh); // DEBUG


#if 1 // old version
    if(!(control->flr_atrs[0] & HZX_FLOOR_STEP) &&
	   !(control->flr_atrs[0] & HZX_FLOOR_FLAT)){

		/* スロープの場合にこの部分を通る */

		float fh;
		float afh;

		afh=fpu_Abs(fh=scratchpad->rfh-scratchpad->lfh);

#if 0
		if(afh>CVC2N(5.0f) /* 5 centi-meter */ &&
		   afh<CVC2N(75.0f) /* 75 centi-meter */ ){
#else
		if(afh<CVC2N(75.0f) /* 75 centi-meter */ ){
#endif

			/* 左右の足の高低差がある場合 */

			float d;

#if 0
			printf("RH , LH : %f , %f\n",scratchpad->rfh,scratchpad->lfh);
			printf("RHV : %f %f %f\n",scratchpad->rvt.vx,scratchpad->rvt.vy,scratchpad->rvt.vz);
			printf("LHV : %f %f %f\n",scratchpad->lvt.vx,scratchpad->lvt.vy,scratchpad->lvt.vz);
#endif

			if(fh<0){
				/* 左足が高い位置にあるので、右足に全て合わせる */
				d=scratchpad->rfh-(scratchpad->rmat.m[3][1]-obj->height);

				if(d>0.0f){
					// printf("Over Left Leg %f\n",d);
					d=0.0f;
				}
			}
			else{
				/* 右足が高い位置にあるので、左足に全て合わせる */
				d=scratchpad->lfh-(scratchpad->rmat.m[3][1]-obj->height);

				if(d>0.0f){
					// printf("Over Right Leg %f\n",d);
					d=0.0f;
				}
			}

			scratchpad->rmat.m[3][1]+=d;
			DG_SetPos(&(scratchpad->rmat));
			scratchpad->rlm[0].m[3][1]+=d;
			scratchpad->rlm[1].m[3][1]+=d;
			scratchpad->rlm[2].m[3][1]+=d;
			scratchpad->llm[0].m[3][1]+=d;
			scratchpad->llm[1].m[3][1]+=d;
			scratchpad->llm[2].m[3][1]+=d;
			scratchpad->rvt.vy+=d;
			scratchpad->lvt.vy+=d;

			// printf("D = %f -------------------\n",d);
		}
    }
#endif

    scratchpad->rfh+=heelp2[param_num];
    scratchpad->lfh+=heelp2[param_num];

    if(obj->m_ctrl->mt3_ctrl[0].flag & MT3_SLEEP){
		if(scratchpad->rvt.vy>=scratchpad->rfh+d_heelp[param_num]) scratchpad->flag|=1;
		scratchpad->flag|=4;
		// printf("MT3 SLEEP\n");
    }
    if(scratchpad->rvt.vy<=scratchpad->rfh-d_heelp[param_num]) scratchpad->flag|=(1|4);
    scratchpad->rvt.vy=scratchpad->rfh;

    if(obj->m_ctrl->mt3_ctrl[0].flag & MT3_SLEEP){
		if(scratchpad->lvt.vy>=scratchpad->lfh+d_heelp[param_num]) scratchpad->flag|=2;
		scratchpad->flag|=8;
		// printf("MT3 SLEEP\n");
    }
    if(scratchpad->lvt.vy<=scratchpad->lfh-d_heelp[param_num]) scratchpad->flag|=(2|8);
    scratchpad->lvt.vy=scratchpad->lfh;

	/* 高部修正部分----2000/11/12 */
	//printf("height = %f\n", obj->height );
	if ( obj->height < 750.0f ){
		/* 腰の高さが低い場合 */
		float	dh ;
		dh = DG_FABS( scratchpad->rfh - scratchpad->lfh ) ;	/* 左右の足の接地高さの差 */
		//printf("dh = %f\n", dh );
		if ( dh > 300.0f ){
			/* 接地の差が大ぎる場合に高いほうのＩＫをＯＦＦにする */
			if ( scratchpad->rfh > scratchpad->lfh ){
				/* 右足のＩＫをＯＦＦに */
				scratchpad->flag &= ~( 1 | 4 );
			} else {
				/* 左足のＩＫをＯＦＦに */
				scratchpad->flag &= ~( 2 | 8 );
			}
		}
	}
	/* 高部修正部分----end */

	// printf("flag = %d\n",scratchpad->flag); // DEBUG

    if(scratchpad->flag & 1){
		/* 右足 */

		/* 足首 */
		vu0_Ldv2((FVECTOR *)&(scratchpad->rlm[2].m[3][0]));

		/* すね */
		vu0_Ldv1((FVECTOR *)&(scratchpad->rlm[1].m[3][0]));

		/* 付け根 */
		vu0_Ldv0((FVECTOR *)&(scratchpad->rlm[0].m[3][0]));

		/* それぞれの関節間の距離、関節の法線を計算 */
		vu0_Subv2v1();
		vu0_Subv1v0();

		vu0_Stv0(&(scratchpad->s0));

		scratchpad->l1=vu0_VectorLength2v1();

		vu0_Ldv0((FVECTOR *)&(scratchpad->rlm[1].m[0][0]));
		vu0_Ldv1((FVECTOR *)&(scratchpad->rlm[0].m[1][0]));

		scratchpad->l1=fpu_Sqrt(scratchpad->l1);

		vu0_Mulv1a(-1.0f);

		vu0_Stv0(&(scratchpad->sn));
		vu0_Stv1(&(scratchpad->vi));


		{
			float s;
			vu0_OuterProductv0v1();
			s=vu0_InnerProductv1v2();
			scratchpad->l2=s*s;
			vu0_Stv0(&(scratchpad->vj));
			s=vu0_InnerProductv0v2();
#if 0
			if(s<0.0f){
				scratchpad->vj.vx=-scratchpad->vj.vx;
				scratchpad->vj.vy=-scratchpad->vj.vy;
				scratchpad->vj.vz=-scratchpad->vj.vz;
				scratchpad->sn.vx=-scratchpad->sn.vx;
				scratchpad->sn.vy=-scratchpad->sn.vy;
				scratchpad->sn.vz=-scratchpad->sn.vz;
			}
#endif
			scratchpad->l2+=s*s;
			scratchpad->l2=fpu_Sqrt(scratchpad->l2);
		}

		/* クオータニオン算出 */
		if(IKsub(&(scratchpad->s0),&(scratchpad->rvt),&(scratchpad->sn),
				 &(scratchpad->vi),&(scratchpad->vj),
				 scratchpad->l1,scratchpad->l2,&(scratchpad->q1),&(scratchpad->q2),1)){

			if(!scratchpad->root_flag){
				/* OBJの変換マトリクスをクオータニオンに変換 */
				MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));
				//MT_QuatNormalize(&(scratchpad->root),&(scratchpad->root));
				MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));
				MT_QuatToMat(&(scratchpad->inv_rmat),&(scratchpad->inv_root));
				scratchpad->root_flag=1;
			}

			MT_QuatMul(&(scratchpad->q1),&(scratchpad->q1),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q1),&(scratchpad->inv_root),&(scratchpad->q1));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[LEG_R0]),&(scratchpad->q1),
					   &(obj->m_ctrl->abs_rots[LEG_R0]));

#if 0
			printf("R : Sn : %f %f %f %f\nR : Q1 : %f %f %f %f\n",
				   scratchpad->sn.vx,scratchpad->sn.vy,scratchpad->sn.vz,scratchpad->sn.vw,
				   scratchpad->q1.vx,scratchpad->q1.vy,scratchpad->q1.vz,scratchpad->q1.vw);
#endif

			vu0_Ldm0(&(scratchpad->inv_rmat));
			vu0_Ldv0(&(scratchpad->sn));
			vu0_Setv0w0();
			vu0_Mulv0m0v0();
			vu0_Stv0(&(scratchpad->tn0));
			vu0_Stv0(&(scratchpad->tn));

			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[LEG_R0]),&(scratchpad->tn0),&(scratchpad->tr0));
			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[LEG_R1]),&(scratchpad->tn),&(scratchpad->tr));

			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->tn0));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->inv_root),&(scratchpad->q2));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[LEG_R1]),&(scratchpad->q2),&(scratchpad->tr));
		}
    }


    if(scratchpad->flag & 4){
		/* 右足首の補正 */
		float x,y,d;

		WorldCalc(scratchpad->rlm+0,obj,LEG_R2);

		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Ldm1(scratchpad->rlm+0);
		vu0_Ldv0(&foot);

		vu0_Mulm2m0m1();
		vu0_Mulv0m2v0();

		scratchpad->q1.vx=scratchpad->rlm[0].m[0][0];
		scratchpad->q1.vy=scratchpad->rlm[0].m[0][1];
		scratchpad->q1.vz=scratchpad->rlm[0].m[0][2];

		vu0_Stm2(scratchpad->rlm+0);
		vu0_Stv0(&(scratchpad->s0));

		/* 少し高いところに座標を設定し、確実に足元の床を計算する */
		scratchpad->s0.vy+=CVM2N(0.5f); /* 50 centi-meter */

		HZX_LevelHazardCheck(hzx_id,&(scratchpad->s0),hzx_check_type,flr_flag);
		scratchpad->rfh=HZX_GetFloorLevel();

		/* 戻す */
		scratchpad->s0.vy-=CVM2N(0.5f);

		y=scratchpad->s0.vy-scratchpad->rfh;
		x=footl[param_num];

		if(fpu_Abs(y)>CVC2N(20.0f)){
			/* 20 cm以上の差があった場合は止める */
			goto next1;
		}

		d=atan2f(y,x);
		// if(d>=M_PI) d=M_PI*2.0f-d;
		d*=0.5f;

		scratchpad->q1.vw=cosf(d);
		d=sinf(d);
		scratchpad->q1.vx*=d;
		scratchpad->q1.vy*=d;
		scratchpad->q1.vz*=d;

#if 0
		printf("Mat = %f %f %f\n",
			   scratchpad->rlm[0].m[3][0],scratchpad->rlm[0].m[3][1],scratchpad->rlm[0].m[3][2]);

		printf("R Y H : %f %f\n",scratchpad->s0.vy,scratchpad->rfh);

		printf("R ADJ : %f %f %f %f\n",
			   scratchpad->q1.vx,scratchpad->q1.vy,scratchpad->q1.vz,scratchpad->q1.vw);
#endif

		MT_QuatMul(&(obj->m_ctrl->abs_rots[LEG_R2]),&(scratchpad->q1),
				   &(obj->m_ctrl->abs_rots[LEG_R2]));
    }

next1:

    if(scratchpad->flag & 2){
		/* 左足 */

		/* 足首 */
		vu0_Ldv2((FVECTOR *)&(scratchpad->llm[2].m[3][0]));

		/* すね */
		vu0_Ldv1((FVECTOR *)&(scratchpad->llm[1].m[3][0]));

		/* 付け根 */
		vu0_Ldv0((FVECTOR *)&(scratchpad->llm[0].m[3][0]));

		/* それぞれの関節間の距離、関節の法線を計算 */
		vu0_Subv2v1();
		vu0_Subv1v0();

		vu0_Stv0(&(scratchpad->s0));

		scratchpad->l1=vu0_VectorLength2v1();

		vu0_Ldv0((FVECTOR *)&(scratchpad->llm[1].m[0][0]));
		vu0_Ldv1((FVECTOR *)&(scratchpad->llm[0].m[1][0]));

		scratchpad->l1=fpu_Sqrt(scratchpad->l1);

		vu0_Mulv1a(-1.0f);

		vu0_Stv0(&(scratchpad->sn));
		vu0_Stv1(&(scratchpad->vi));

		{
			float s;
			vu0_OuterProductv0v1();
			s=vu0_InnerProductv1v2();
			scratchpad->l2=s*s;
			vu0_Stv0(&(scratchpad->vj));
			s=vu0_InnerProductv0v2();
#if 0
			if(s<0.0f){
				scratchpad->vj.vx=-scratchpad->vj.vx;
				scratchpad->vj.vy=-scratchpad->vj.vy;
				scratchpad->vj.vz=-scratchpad->vj.vz;
				scratchpad->sn.vx=-scratchpad->sn.vx;
				scratchpad->sn.vy=-scratchpad->sn.vy;
				scratchpad->sn.vz=-scratchpad->sn.vz;
			}
#endif
			scratchpad->l2+=s*s;
			scratchpad->l2=fpu_Sqrt(scratchpad->l2);
		}

		/* クオータニオン算出 */
		if(IKsub(&(scratchpad->s0),&(scratchpad->lvt),&(scratchpad->sn),
				 &(scratchpad->vi),&(scratchpad->vj),
				 scratchpad->l1,scratchpad->l2,&(scratchpad->q1),&(scratchpad->q2),1)){

			if(!scratchpad->root_flag){
				/* OBJの変換マトリクスをクオータニオンに変換 */
				MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));
				//MT_QuatNormalize(&(scratchpad->root),&(scratchpad->root));
				MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));
				MT_QuatToMat(&(scratchpad->inv_rmat),&(scratchpad->inv_root));
				scratchpad->root_flag=1;
			}

			MT_QuatMul(&(scratchpad->q1),&(scratchpad->q1),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q1),&(scratchpad->inv_root),&(scratchpad->q1));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[LEG_L0]),&(scratchpad->q1),
					   &(obj->m_ctrl->abs_rots[LEG_L0]));

#if 0
			printf("L : Sn : %f %f %f %f\nL : Q1 : %f %f %f %f\n",
				   scratchpad->sn.vx,scratchpad->sn.vy,scratchpad->sn.vz,scratchpad->sn.vw,
				   scratchpad->q1.vx,scratchpad->q1.vy,scratchpad->q1.vz,scratchpad->q1.vw);
#endif

			vu0_Ldm0(&(scratchpad->inv_rmat));
			vu0_Ldv0(&(scratchpad->sn));
			vu0_Setv0w0();
			vu0_Mulv0m0v0();
			vu0_Stv0(&(scratchpad->tn0));
			vu0_Stv0(&(scratchpad->tn));

			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[LEG_L0]),&(scratchpad->tn0),&(scratchpad->tr0));
			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[LEG_L1]),&(scratchpad->tn),&(scratchpad->tr));

			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->tn0));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->inv_root),&(scratchpad->q2));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[LEG_L1]),&(scratchpad->q2),&(scratchpad->tr));
		}
    }


    if(scratchpad->flag & 8){
		/* 左足首の補正 */
		float x,y,d;

		WorldCalc(scratchpad->llm+0,obj,LEG_L2);

		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Ldm1(scratchpad->llm+0);
		vu0_Ldv0(&foot);

		vu0_Mulm2m0m1();
		vu0_Mulv0m2v0();

		scratchpad->q1.vx=scratchpad->llm[0].m[0][0];
		scratchpad->q1.vy=scratchpad->llm[0].m[0][1];
		scratchpad->q1.vz=scratchpad->llm[0].m[0][2];

		vu0_Stm2(scratchpad->llm+0);
		vu0_Stv0(&(scratchpad->s0));

		/* 少し高いところに座標を設定し、確実に足元の床を計算する */
		scratchpad->s0.vy+=CVM2N(0.5f); /* 50 centi-meter */

		HZX_LevelHazardCheck(hzx_id,&(scratchpad->s0),hzx_check_type,flr_flag);
		scratchpad->lfh=HZX_GetFloorLevel();

		/* 戻す */
		scratchpad->s0.vy-=CVM2N(0.5f);

		y=scratchpad->s0.vy-scratchpad->lfh;
		x=footl[param_num];

		if(fpu_Abs(y)>CVC2N(20.0f)){
			/* 20 cm以上の差があった場合は止める */
			goto next2;
		}

		d=atan2f(y,x);
		// if(d>=M_PI) d=M_PI*2.0f-d;
		d*=0.5f;

		scratchpad->q1.vw=cosf(d);
		d=sinf(d);
		scratchpad->q1.vx*=d;
		scratchpad->q1.vy*=d;
		scratchpad->q1.vz*=d;

#if 0
		printf("L Y H : %f %f\n",scratchpad->s0.vy,scratchpad->lfh);

		printf("L ADJ : %f %f %f %f\n",
			   scratchpad->q1.vx,scratchpad->q1.vy,scratchpad->q1.vz,scratchpad->q1.vw);
#endif

		MT_QuatMul(&(obj->m_ctrl->abs_rots[LEG_L2]),&(scratchpad->q1),
				   &(obj->m_ctrl->abs_rots[LEG_L2]));
    }

next2:
		;
}

void Leg_IKcalc(CONTROL *control,OBJECT *obj)
{
	_Leg_IKcalc(control,obj,0);
}

void Leg_IKcalcForRaiden(CONTROL *control,OBJECT *obj)
{
	_Leg_IKcalc(control,obj,1);
}

void Leg_IKcalcForEmma(CONTROL *control,OBJECT *obj)
{
	_Leg_IKcalc(control,obj,2);
}

void Leg_IKcalcForFatman(CONTROL *control,OBJECT *obj)
{
	_Leg_IKcalc(control,obj,3);
}

#if 1
#define ARM_R0	HUMAN21_MIGI_UDE1
#define ARM_R1	HUMAN21_MIGI_UDE2
#define ARM_R2	HUMAN21_MIGI_TE
#define ARM_L0	HUMAN21_HIDARI_UDE1
#define ARM_L1	HUMAN21_HIDARI_UDE2
#define ARM_L2	HUMAN21_HIDARI_TE
#else
#define ARM_R0	HUMAN16_MIGI_UDE1
#define ARM_R1	HUMAN16_MIGI_UDE2
#define ARM_R2	HUMAN16_MIGI_TE
#define ARM_L0	HUMAN16_HIDARI_UDE1
#define ARM_L1	HUMAN16_HIDARI_UDE2
#define ARM_L2	HUMAN16_HIDARI_TE
#endif

/* objのobjnum(手のobj)をtargetのtobjnumのtpointに添える */
int Arm_IKcalc(OBJECT *obj,int objnum,OBJECT *target,int tobjnum,FVECTOR *tpoint)
{
    float s,t,u;
#ifndef BP_PSX2_ASM
	FVECTOR _vf30, _vf31 ;
#endif

    struct _scratchpad {
		FMATRIX rmat;
		FMATRIX inv_rmat;
		FVECTOR root,inv_root;

		FMATRIX lm[3];
		FVECTOR vt;

		FVECTOR tn,tr;
		FVECTOR tn0,tr0;

		FVECTOR s0,sn,vi,vj;
		FVECTOR q0,q1,q2;
		float l1,l2;
    };
#if 1
    struct _scratchpad * const scratchpad
		=(struct _scratchpad * const)(SCRPAD_ADDR+0x4000-sizeof(struct _scratchpad));
#else
    struct _scratchpad dummy;
    struct _scratchpad * const scratchpad=&dummy;
#endif
    //FVECTOR f={ 87.5f, -110.0f , -97.5f, 1.0f };

    /* ターゲットワールド座標の演算 */
    WorldCalc(scratchpad->lm+0,target,tobjnum);

    vu0_Ldm0(&(target->objs->world));
    vu0_Ldm1(scratchpad->lm+0);
    vu0_Ldv0(tpoint);
    //vu0_Ldv0(&f);

    vu0_Mulm2m0m1();

    vu0_Setv0w1();

    vu0_Mulv0m2v0();

    vu0_Stv0(&(scratchpad->vt));
    //asm volatile ("sqc2 vf15,0(%0)" : : "r"(&(scratchpad->vt)));

    /* ルートマトリクスの取り出し */
    //DG_GetPos(&(scratchpad->rmat));
    fpu_CopyMatrix(&(scratchpad->rmat),&(obj->objs->world));


    switch(objnum){
    case HUMAN21_MIGI_TE:
		/* 右肩の修正 */

		/* マトリクス計算 */
		WorldCalc(scratchpad->lm+0,obj,ARM_R0);
		WorldCalc2(scratchpad->lm+1,scratchpad->lm+0,obj,ARM_R1);
		WorldCalc2(scratchpad->lm+2,scratchpad->lm+1,obj,ARM_R2);

		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Ldm1(scratchpad->lm+0);

		vu0_Mulm2m0m1();

		vu0_Ldm1(scratchpad->lm+2);

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf3,vf15");
#else
		vu0_Ldv2( (FVECTOR *)vu0_Ptrm2()->m[3] ) ;
#endif
		vu0_Mulm2m0m1();

		vu0_Ldv1(&(scratchpad->vt));
#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf1,vf15");
#else
		vu0_Ldv0( (FVECTOR *)vu0_Ptrm2()->m[3] ) ;
#endif

		vu0_Subv1v2();
		vu0_Subv0v2();

#ifdef BP_PSX2_ASM
		asm volatile ("
		vmulx.y		vf2,vf2,vf0
		vmulx.y		vf1,vf1,vf0
		");
#else
		vu0_Clrv0() ;
		vu0_Clrv1() ;
#endif

		if((t=vu0_VectorLength2v1())==0.0f){
			vu0_Stv0(&(scratchpad->sn));
		}
		else{
			vu0_Stv1(&(scratchpad->sn));
		}

		t*=vu0_VectorLength2v0();

		{
			float tmp=scratchpad->sn.vx;
			scratchpad->sn.vx=scratchpad->sn.vz;
			scratchpad->sn.vz=-tmp;
		}

		vu0_OuterProductv2v0v1();
		s=vu0_InnerProductv0v1();
		s=fpu_Rsqrt(t,s);

		vu0_Ldv0(&(scratchpad->sn));

		t=vu0_VectorLength2v2();
		u=vu0_VectorLength2v0();
		t=fpu_Rsqrt(t,1.0f);
		u=fpu_Rsqrt(u,1.0f);
		vu0_Mulv2a(t);
		vu0_Mulv0a(u);
		vu0_Stv2(&(scratchpad->q0));
		vu0_Stv0(&(scratchpad->sn));

		/* クオータニオン生成 */
		//if(s<0.0f) return;  /* ±90度内に制限する */
#if 0
		s=acosf(s);
		s*=0.5f;
		t=sinf(s);
		s=cosf(s);
#else
		t=fpu_Sqrt((1.0f-s)*0.5f);
		s=fpu_Sqrt((1.0f+s)*0.5f);
#endif
		scratchpad->q0.vx*=t;
		scratchpad->q0.vz*=t;
		scratchpad->q0.vy*=t;
		scratchpad->q0.vw=s;

		/* クオータニオン演算 */
		MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));
		MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));
		MT_QuatMul(&(scratchpad->q0),&(scratchpad->q0),&(scratchpad->root));
		MT_QuatMul(&(scratchpad->q0),&(scratchpad->inv_root),&(scratchpad->q0));
		MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_R0]),&(scratchpad->q0),
				   &(obj->m_ctrl->abs_rots[ARM_R0]));


		/* 右腕の計算 */

		/* マトリクス計算 */
		WorldCalc(scratchpad->lm+0,obj,ARM_R0);
		WorldCalc2(scratchpad->lm+1,scratchpad->lm+0,obj,ARM_R1);
		WorldCalc2(scratchpad->lm+2,scratchpad->lm+1,obj,ARM_R2);

		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Ldm1(scratchpad->lm+0);

		vu0_Mulm2m0m1();

		vu0_Ldm1(scratchpad->lm+1);
		vu0_Stm2(scratchpad->lm+0);

		vu0_Mulm2m0m1();

		vu0_Ldm1(scratchpad->lm+2);
		vu0_Stm2(scratchpad->lm+1);

		vu0_Mulm2m0m1();

		//vu0_Ldv0((FVECTOR *)&(scratchpad->lm[2].m[3][0]));
		vu0_Ldv1((FVECTOR *)&(scratchpad->lm[1].m[3][0]));
#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf1,vf15");
#else
		vu0_Ldv1( (FVECTOR *)vu0_Ptrm2()->m[3] ) ;
#endif
		vu0_Ldv2((FVECTOR *)&(scratchpad->lm[0].m[3][0]));

		vu0_Subv0v1();
		vu0_Subv1v2();

		vu0_Stv2(&(scratchpad->s0));

		vu0_Ldv2(&(scratchpad->sn));
		vu0_Stm2(scratchpad->lm+2);

#ifdef BP_PSX2_ASM
		asm volatile ("
		vmove.xyzw vf30,vf1
		vmove.xyzw vf31,vf2
		");

#else
        vu0_Stv0( &_vf30 ) ;
        vu0_Stv1( &_vf31 ) ;
#endif

		vu0_OuterProductv0v2v1(); /* y=zx */
		vu0_OuterProductv1v0v2(); /* x=yz */
		s=vu0_VectorLength2v0();
		t=vu0_VectorLength2v1();
		s=fpu_Rsqrt(s,1.0f);
		t=fpu_Rsqrt(t,1.0f);
		vu0_Mulv0a(s);
		vu0_Mulv1a(t);

		vu0_Stv0(&(scratchpad->vj));
		vu0_Stv1(&(scratchpad->vi));

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf3,vf31");
#else
        vu0_Ldv2( &_vf31 ) ;
#endif

#if 0
		s=vu0_InnerProductv1v2();
		t=vu0_InnerProductv0v2();
		scratchpad->l1=fpu_Sqrt(s*s+t*t);
#else
		scratchpad->l1=vu0_InnerProductv1v2();
#endif

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf3,vf30");
#else
        vu0_Ldv2( &_vf30 ) ;
#endif
		s=vu0_InnerProductv1v2();
		t=vu0_InnerProductv0v2();
		if(t<0.0f){
			scratchpad->vj.vx=-scratchpad->vj.vx;
			scratchpad->vj.vy=-scratchpad->vj.vy;
			scratchpad->vj.vz=-scratchpad->vj.vz;
			scratchpad->sn.vx=-scratchpad->sn.vx;
			scratchpad->sn.vy=-scratchpad->sn.vy;
			scratchpad->sn.vz=-scratchpad->sn.vz;
		}
		scratchpad->l2=fpu_Sqrt(s*s+t*t);

		/* クオータニオン算出 */
		if(IKsub(&(scratchpad->s0),&(scratchpad->vt),
				 &(scratchpad->sn),&(scratchpad->vi),&(scratchpad->vj),
				 scratchpad->l1,scratchpad->l2,&(scratchpad->q1),&(scratchpad->q2),1)){

			MT_QuatToMat(&(scratchpad->inv_rmat),&(scratchpad->inv_root));

			MT_QuatMul(&(scratchpad->q1),&(scratchpad->q1),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q1),&(scratchpad->inv_root),&(scratchpad->q1));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_R0]),&(scratchpad->q1),
					   &(obj->m_ctrl->abs_rots[ARM_R0]));

			vu0_Ldm0(&(scratchpad->inv_rmat));
			vu0_Ldv0(&(scratchpad->sn));
			vu0_Setv0w0();
			vu0_Mulv0m0v0();
			vu0_Stv0(&(scratchpad->tn0));
			vu0_Stv0(&(scratchpad->tn));

			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[ARM_R0]),&(scratchpad->tn0),&(scratchpad->tr0));
			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[ARM_R1]),&(scratchpad->tn),&(scratchpad->tr));

			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->tn0));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->inv_root),&(scratchpad->q2));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_R1]),&(scratchpad->q2),&(scratchpad->tr));

			return 1;
		}
		else{
			MT_QuatInverse(&(scratchpad->q0),&(scratchpad->q0));
			MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_R0]),&(scratchpad->q0),
					   &(obj->m_ctrl->abs_rots[ARM_R0]));

			return 0;
		}
		break;

    case HUMAN21_HIDARI_TE:
		/* 左肩の修正 */

		/* マトリクス計算 */
		WorldCalc(scratchpad->lm+0,obj,ARM_L0);
		WorldCalc2(scratchpad->lm+1,scratchpad->lm+0,obj,ARM_L1);
		WorldCalc2(scratchpad->lm+2,scratchpad->lm+1,obj,ARM_L2);

		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Ldm1(scratchpad->lm+0);

		vu0_Mulm2m0m1();

		vu0_Ldm1(scratchpad->lm+2);
#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf3,vf15");
#else
		vu0_Ldv2( (FVECTOR *)vu0_Ptrm2()->m[3] ) ;
#endif

		vu0_Mulm2m0m1();

		vu0_Ldv1(&(scratchpad->vt));
#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf1,vf15");
#else
		vu0_Ldv0( (FVECTOR *)vu0_Ptrm2()->m[3] ) ;
#endif

		vu0_Subv1v2();
		vu0_Subv0v2();

#ifdef BP_PSX2_ASM
		asm volatile ("
		vmulx.y		vf2,vf2,vf0
		vmulx.y		vf1,vf1,vf0
		");
#else
		vu0_Clrv0() ;
		vu0_Clrv1() ;
#endif

		if((t=vu0_VectorLength2v1())==0.0f){
			vu0_Stv0(&(scratchpad->sn));
		}
		else{
			vu0_Stv1(&(scratchpad->sn));
		}

		t*=vu0_VectorLength2v0();

		{
			float tmp=scratchpad->sn.vx;
			scratchpad->sn.vx=scratchpad->sn.vz;
			scratchpad->sn.vz=-tmp;
		}

		vu0_OuterProductv2v0v1();
		s=vu0_InnerProductv0v1();
		s=fpu_Rsqrt(t,s);

		vu0_Ldv0(&(scratchpad->sn));

		t=vu0_VectorLength2v2();
		u=vu0_VectorLength2v0();
		t=fpu_Rsqrt(t,1.0f);
		u=fpu_Rsqrt(u,1.0f);
		vu0_Mulv2a(t);
		vu0_Mulv0a(u);
		vu0_Stv2(&(scratchpad->q0));
		vu0_Stv0(&(scratchpad->sn));

		/* クオータニオン生成 */
		//if(s<0.0f) return; /* ±90度内に制限する */
#if 0
		s=acosf(s);
		s*=0.5f;
		t=sinf(s);
		s=cosf(s);
#else
		t=fpu_Sqrt((1.0f-s)*0.5f);
		s=fpu_Sqrt((1.0f+s)*0.5f);
#endif
		scratchpad->q0.vx*=t;
		scratchpad->q0.vz*=t;
		scratchpad->q0.vy*=t;
		scratchpad->q0.vw=s;

		/* クオータニオン演算 */
		MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));
		MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));
		MT_QuatMul(&(scratchpad->q0),&(scratchpad->q0),&(scratchpad->root));
		MT_QuatMul(&(scratchpad->q0),&(scratchpad->inv_root),&(scratchpad->q0));
		MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_L0]),&(scratchpad->q0),
				   &(obj->m_ctrl->abs_rots[ARM_L0]));


		/* 左腕の計算 */

		/* マトリクス計算 */
		WorldCalc(scratchpad->lm+0,obj,ARM_L0);
		WorldCalc2(scratchpad->lm+1,scratchpad->lm+0,obj,ARM_L1);
		WorldCalc2(scratchpad->lm+2,scratchpad->lm+1,obj,ARM_L2);

		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Ldm1(scratchpad->lm+0);

		vu0_Mulm2m0m1();

		vu0_Ldm1(scratchpad->lm+1);
		vu0_Stm2(scratchpad->lm+0);

		vu0_Mulm2m0m1();

		vu0_Ldm1(scratchpad->lm+2);
		vu0_Stm2(scratchpad->lm+1);

		vu0_Mulm2m0m1();

		//vu0_Ldv0((FVECTOR *)&(scratchpad->lm[2].m[3][0]));
		vu0_Ldv1((FVECTOR *)&(scratchpad->lm[1].m[3][0]));
#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf1,vf15");
#else
		vu0_Ldv0( (FVECTOR *)vu0_Ptrm2()->m[3] ) ;
#endif
		vu0_Ldv2((FVECTOR *)&(scratchpad->lm[0].m[3][0]));

		vu0_Subv0v1();
		vu0_Subv1v2();

		vu0_Stv2(&(scratchpad->s0));

		vu0_Ldv2(&(scratchpad->sn));
		vu0_Stm2(scratchpad->lm+2);

#ifdef BP_PSX2_ASM
		asm volatile ("
		vmove.xyzw vf30,vf1
		vmove.xyzw vf31,vf2
		");
#else
        vu0_Stv0( &_vf30 ) ;
        vu0_Stv1( &_vf31 ) ;
#endif

		vu0_OuterProductv0v2v1(); /* y=zx */
		vu0_OuterProductv1v0v2(); /* x=yz */
		s=vu0_VectorLength2v0();
		t=vu0_VectorLength2v1();
		s=fpu_Rsqrt(s,1.0f);
		t=fpu_Rsqrt(t,1.0f);
		vu0_Mulv0a(s);
		vu0_Mulv1a(t);

		vu0_Stv0(&(scratchpad->vj));
		vu0_Stv1(&(scratchpad->vi));

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf3,vf31");
#else
        vu0_Ldv2( &_vf31 ) ;
#endif
#if 0
		s=vu0_InnerProductv1v2();
		t=vu0_InnerProductv0v2();
		scratchpad->l1=fpu_Sqrt(s*s+t*t);
#else
		scratchpad->l1=vu0_InnerProductv1v2();
#endif

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf3,vf30");
#else
        vu0_Ldv2( &_vf30 ) ;
#endif
		s=vu0_InnerProductv1v2();
		t=vu0_InnerProductv0v2();
		if(t<0.0f){
			scratchpad->vj.vx=-scratchpad->vj.vx;
			scratchpad->vj.vy=-scratchpad->vj.vy;
			scratchpad->vj.vz=-scratchpad->vj.vz;
			scratchpad->sn.vx=-scratchpad->sn.vx;
			scratchpad->sn.vy=-scratchpad->sn.vy;
			scratchpad->sn.vz=-scratchpad->sn.vz;
		}
#if 1
		scratchpad->l2=fpu_Sqrt(s*s+t*t);
#else
		scratchpad->l2=250.0f;
#endif


#if 0
		printf("vi    = %f %f %f\n",scratchpad->vi.vx,scratchpad->vi.vy,scratchpad->vi.vz);
		printf("vj    = %f %f %f\n",scratchpad->vj.vx,scratchpad->vj.vy,scratchpad->vj.vz);
		printf("Sn    = %f %f %f\n",scratchpad->sn.vx,scratchpad->sn.vy,scratchpad->sn.vz);
#endif

#if 0
		printf("S  T  : %f %f\n",s,t);
		printf("L1 L2 : %f %f\n",scratchpad->l1,scratchpad->l2);
#endif

		/* クオータニオン算出 */
		if(IKsub(&(scratchpad->s0),&(scratchpad->vt),
				 &(scratchpad->sn),&(scratchpad->vi),&(scratchpad->vj),
				 scratchpad->l1,scratchpad->l2,&(scratchpad->q1),&(scratchpad->q2),1)){

			MT_QuatToMat(&(scratchpad->inv_rmat),&(scratchpad->inv_root));

			MT_QuatMul(&(scratchpad->q1),&(scratchpad->q1),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q1),&(scratchpad->inv_root),&(scratchpad->q1));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_L0]),&(scratchpad->q1),
					   &(obj->m_ctrl->abs_rots[ARM_L0]));

			vu0_Ldm0(&(scratchpad->inv_rmat));
			vu0_Ldv0(&(scratchpad->sn));
			vu0_Setv0w0();
			vu0_Mulv0m0v0();
			vu0_Stv0(&(scratchpad->tn0));
			vu0_Stv0(&(scratchpad->tn));

			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[ARM_L0]),&(scratchpad->tn0),&(scratchpad->tr0));
			MT_QuatSeparate2(&(obj->m_ctrl->abs_rots[ARM_L1]),&(scratchpad->tn),&(scratchpad->tr));

			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->root));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->q2),&(scratchpad->tn0));
			MT_QuatMul(&(scratchpad->q2),&(scratchpad->inv_root),&(scratchpad->q2));

			MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_L1]),&(scratchpad->q2),&(scratchpad->tr));

#if 0
			printf("Q1 : %f %f %f %f\nARM_L1 : %f %f %f %f\n",
				   scratchpad->q1.vx,scratchpad->q1.vy,scratchpad->q1.vz,scratchpad->q1.vw,
				   obj->m_ctrl->abs_rots[ARM_L1].vx,obj->m_ctrl->abs_rots[ARM_L1].vy,
				   obj->m_ctrl->abs_rots[ARM_L1].vz,obj->m_ctrl->abs_rots[ARM_L1].vw);
#endif
			return 1;
		}
		else{
			MT_QuatInverse(&(scratchpad->q0),&(scratchpad->q0));
			MT_QuatMul(&(obj->m_ctrl->abs_rots[ARM_L0]),&(scratchpad->q0),
					   &(obj->m_ctrl->abs_rots[ARM_L0]));

			return 0;
		}
		break;
    }

	return 0;
}
