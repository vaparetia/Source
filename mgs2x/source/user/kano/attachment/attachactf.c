//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachactf.c
		装備品落下

	1999/11/05 K.Kano
	$Id: attachactf.c,v 1.1.1.3 2002/11/19 11:43:03 Yoshizawa1 Exp $
*/


#include "attachmentf.h"


void InitAttachmentFall(DG_OBJS *objs,MODEL_PARAMETERF *mpf,FVECTOR *base)
{
    float l;

    if(objs->root!=NULL){
	fpu_CopyMatrix(&(objs->world),objs->root);
	objs->root=NULL;
    }

    vu0_Ldv0((FVECTOR *)&(objs->world.m[3][0]));
    vu0_Ldv1(base);

    vu0_Subv0v1();

    l=vu0_VectorLength2v0();

    vu0_Mulv0a(fpu_Rsqrt(l,CVC2N(3.0f)));

    mpf->count=60*3;

    vu0_Stv0(&(mpf->v));

	if(mpf->v.vy<0.0f) mpf->v.vy=0.0f;

    HZX_LevelHazardCheck(HZX_CurrentGroupID,(FVECTOR *)&(objs->world.m[3][0]),
			 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
    mpf->floor_height=HZX_GetFloorLevel();

	// 2000.11/01 追加しました（岡嶋）。
	mpf->v.vw=0.0f;

}

int MoveAttachmentFall(DG_OBJS *objs,MODEL_PARAMETERF *mpf)
{
    if(mpf->count>0){
	vu0_Ldv0((FVECTOR *)&(objs->world.m[3][0]));
	vu0_Ldv1(&(mpf->v));

	mpf->v.vx*=0.95f;
	mpf->v.vz*=0.95f;

	vu0_Addv0v1();

	vu0_Stv0((FVECTOR *)&(objs->world.m[3][0]));
	fpu_CopyMatrix(&(objs->objs[0].world),&(objs->world));

	if(mpf->floor_height>objs->world.m[3][1]){

		// 2000.11/01 追加しました（岡嶋）。
		if(mpf->v.vw == 0.0f){
			mpf->v.vw = 1.0f;
			GM_SeSetMode( SD_A_MOROTI01 , (FVECTOR *)objs->world.m[3], GM_SEMODE_NORMAL );	/* Ｍ９注射器落ちる */
		}

	    mpf->v.vy*=-0.5f;
	    mpf->count-=30; /* 床にぶつかったら早く消える */
	}
	mpf->v.vy+=P_GRAVITY;

	if(objs->n_models>1){
	    FVECTOR vec;
	    int i;

	    vu0_Ldm2(&(objs->objs[0].world));

	    vec.vx=objs->def->models[1].tx;
	    vec.vy=objs->def->models[1].ty;
	    vec.vz=objs->def->models[1].tz;
	    vec.vw=1.0f;

	    vu0_Ldv0(&vec);

	    for(i=1;i<objs->n_models;i++){
		vu0_Mulv1m2v0();

		vu0_Stm2(&(objs->objs[i].world));

		vec.vx=objs->def->models[i].tx;
		vec.vy=objs->def->models[i].ty;
		vec.vz=objs->def->models[i].tz;

		vu0_Stv1((FVECTOR *)&(objs->objs[i].world.m[3][0]));
		vu0_Ldv0(&vec);
	    }
	}

	mpf->count--;

	return 1;
    }
    return 0;
}
