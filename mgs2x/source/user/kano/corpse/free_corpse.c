/*
	free_corpse.c
		死体の姿勢制御(自由落下系統)

	1999/10/14 K.Kano
	$Id: free_corpse.c,v 1.1.1.3 2002/11/19 11:43:09 Yoshizawa1 Exp $
*/

#include "corpse.h"

#define DEG_BACKBONE		(M_PI*15.0f/180.0f)
#define VAL_BACKBONE		30.0f
#define DEG_BACKBONE_HIP	(M_PI*10.0f/180.0f)
#define VAL_BACKBONE_HIP	50.0f

#if 0
#define SWITCH2HOLD_FRAMES	20
#else
#define SWITCH2HOLD_FRAMES	(60*3)
#endif


static void StartCorpse2(struct _scratchpad *scratchpad,OBJECT *obj,CORPSE_PARAMETER *corpsep)
{
    //MAP *map;
    int size=HUMAN_MODEL_OBJN;
    int i;

    for(i=0;i<size;i++){
	fpu_CopyVector(&(scratchpad->abs_rots[i]),&(obj->m_ctrl->abs_rots[i]));
    }

#if 0
    if((map=GM_GetMap(GM_CurrentMap))==NULL){
	scratchpad->hzd=NULL;
    }
    else{
	scratchpad->hzd=map->hzd;
    }
#else
    scratchpad->hzd=HZD_GetCurrentHzd(GM_CurrentMap);
#endif

    scratchpad->flags=0;
    scratchpad->flags2=corpsep->flags2;

    DG_GetPos(&(scratchpad->m_rmat));
    MT_MatToQuat(&(scratchpad->m_root),&(scratchpad->m_rmat));
    MT_QuatInverse(&(scratchpad->inv_m_root),&(scratchpad->m_root));

    for(i=0;i<size;i++){
	fpu_CopyVector(&(scratchpad->x[i]),
		       (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
    }

    fpu_CopyVector(&(scratchpad->root),&(corpsep->main_abs_rots));

    MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));

    MT_QuatToMat(&(scratchpad->rmat),&(scratchpad->root));

#if 0
    fpu_CopyVector((FVECTOR *)&(scratchpad->rmat.m[3][0]),
		   &(scratchpad->x[HUMAN21_KOSHI]));
#else
    fpu_CopyVector((FVECTOR *)&(scratchpad->rmat.m[3][0]),
		   (FVECTOR *)&(scratchpad->m_rmat.m[3][0]));
#endif

    MT_QuatMul(&(scratchpad->conv_root),&(scratchpad->inv_m_root),&(scratchpad->root));
}


#define SLERP_PARAM	(1.0f/4.0f)

#if 0
#define SLERP_LIMIT	(0.965925826f) /* cos 15 */
#define SLERP_LIMIT2	(0.707106781f) /* cos 45 */
#elif 0
#define SLERP_LIMIT	(0.866025404f) /* cos 30 */
#define SLERP_LIMIT2	(0.5f)         /* cos 60 */
#else
#define SLERP_LIMIT	(0.866025404f) /* cos 30 */
#define SLERP_LIMIT2	(0.707106781f) /* cos 45 */
#endif

#if 1

static void CheckBackBone(struct _scratchpad *scratchpad,Work *work)
{
    OBJECT *obj=work->obj;
    DG_MDL *mdl=obj->objs->def->models;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    FMATRIX m;
    FVECTOR vec;
    FVECTOR q;
    float p;

    /* KAKATO+ASHI2+ASHI1 */
    vec.vx=mdl[HUMAN21_MIGI_ASHI2].tx+mdl[HUMAN21_MIGI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_MIGI_ASHI2].ty+mdl[HUMAN21_MIGI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_MIGI_ASHI2].tz+mdl[HUMAN21_MIGI_KAKATO].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_MIGI_ASHI1]),
		 &(scratchpad->x[HUMAN21_MIGI_KAKATO]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->root));

    MT_QuatMul(&q,&(scratchpad->inv_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_ASHI2))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_ASHI2);
	}
    }

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_KAKATO))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_KAKATO);
	}
    }


    vec.vx=mdl[HUMAN21_HIDARI_ASHI2].tx+mdl[HUMAN21_HIDARI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_HIDARI_ASHI2].ty+mdl[HUMAN21_HIDARI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_HIDARI_ASHI2].tz+mdl[HUMAN21_HIDARI_KAKATO].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_HIDARI_ASHI1]),
		 &(scratchpad->x[HUMAN21_HIDARI_KAKATO]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->root));

    MT_QuatMul(&q,&(scratchpad->inv_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_ASHI2))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_ASHI2);
	}
    }

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_KAKATO))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_KAKATO);
	}
    }


    /* ASHI2+ASHI1+KOSHI */
    MT_QuatSlerp(&q,
		 &(scratchpad->abs_rots[HUMAN21_KOSHI]),
		 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_ASHI2))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_ASHI2);
	}
    }

    MT_QuatSlerp(&q,
		 &(scratchpad->abs_rots[HUMAN21_KOSHI]),
		 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_ASHI2))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_ASHI2);
	}
    }


    /* ASHI1+KOSHI+ONAKA */
    MT_QuatSlerp(&q,
		 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    MT_QuatSlerp(&q,&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_KOSHI]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     &(scratchpad->abs_rots[HUMAN21_KOSHI]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_KOSHI))){
	    scratchpad->flags&=~(1<<HUMAN21_KOSHI);
	}
    }

    /* KOSHI+ONAKA+MUNE */
    vec.vx=mdl[HUMAN21_ONAKA].tx+mdl[HUMAN21_MUNE].tx;
    vec.vy=mdl[HUMAN21_ONAKA].ty+mdl[HUMAN21_MUNE].ty;
    vec.vz=mdl[HUMAN21_ONAKA].tz+mdl[HUMAN21_MUNE].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_KOSHI]),
		 &(scratchpad->x[HUMAN21_MUNE]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->root));

    MT_QuatMul(&q,&(scratchpad->inv_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_KOSHI]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     &(scratchpad->abs_rots[HUMAN21_KOSHI]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_KOSHI))){
	    scratchpad->flags&=~(1<<HUMAN21_KOSHI);
	}
    }

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     &(scratchpad->abs_rots[HUMAN21_ONAKA]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_ONAKA))){
	    scratchpad->flags&=~(1<<HUMAN21_ONAKA);
	}
    }

    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_KOSHI]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_KOSHI]));

    vec.vx=mdl[HUMAN21_ONAKA].tx;
    vec.vy=mdl[HUMAN21_ONAKA].ty;
    vec.vz=mdl[HUMAN21_ONAKA].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vec.vx=mdl[HUMAN21_MIGI_ASHI1].tx;
    vec.vy=mdl[HUMAN21_MIGI_ASHI1].ty;
    vec.vz=mdl[HUMAN21_MIGI_ASHI1].tz;
    vu0_Ldv1(&vec);

    vec.vx=mdl[HUMAN21_HIDARI_ASHI1].tx;
    vec.vy=mdl[HUMAN21_HIDARI_ASHI1].ty;
    vec.vz=mdl[HUMAN21_HIDARI_ASHI1].tz;
    vu0_Ldv2(&vec);

    vu0_Mulv0m0v0();
    vu0_Mulv1m0v1();
    vu0_Mulv2m0v2();

    vu0_Stv0(&(scratchpad->x[HUMAN21_ONAKA]));
    vu0_Stv1(&(scratchpad->x[HUMAN21_MIGI_ASHI1]));
    vu0_Stv2(&(scratchpad->x[HUMAN21_HIDARI_ASHI1]));


    /* ASHI2 CALC */
    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MIGI_ASHI1]));

    vec.vx=mdl[HUMAN21_MIGI_ASHI2].tx;
    vec.vy=mdl[HUMAN21_MIGI_ASHI2].ty;
    vec.vz=mdl[HUMAN21_MIGI_ASHI2].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_MIGI_ASHI2]));


    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_HIDARI_ASHI1]));

    vec.vx=mdl[HUMAN21_HIDARI_ASHI2].tx;
    vec.vy=mdl[HUMAN21_HIDARI_ASHI2].ty;
    vec.vz=mdl[HUMAN21_HIDARI_ASHI2].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_HIDARI_ASHI2]));


    /* KAKATO CALC */
    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MIGI_ASHI2]));

    vec.vx=mdl[HUMAN21_MIGI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_MIGI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_MIGI_KAKATO].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_MIGI_KAKATO]));


    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_HIDARI_ASHI2]));

    vec.vx=mdl[HUMAN21_HIDARI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_HIDARI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_HIDARI_KAKATO].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_HIDARI_KAKATO]));



    /* ONAKA+MUNE+KUBI */
    vec.vx=mdl[HUMAN21_MUNE].tx+mdl[HUMAN21_KUBI].tx;
    vec.vy=mdl[HUMAN21_MUNE].ty+mdl[HUMAN21_KUBI].ty;
    vec.vz=mdl[HUMAN21_MUNE].tz+mdl[HUMAN21_KUBI].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_ONAKA]),
		 &(scratchpad->x[HUMAN21_KUBI]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->root));
    MT_QuatMul(&q,&(scratchpad->inv_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     &(scratchpad->abs_rots[HUMAN21_ONAKA]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_ONAKA))){
	    scratchpad->flags&=~(1<<HUMAN21_ONAKA);
	}
    }

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MUNE]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		     &(scratchpad->abs_rots[HUMAN21_MUNE]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MUNE))){
	    scratchpad->flags&=~(1<<HUMAN21_MUNE);
	}
    }

    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_ONAKA]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_ONAKA]));

    vec.vx=mdl[HUMAN21_MUNE].tx;
    vec.vy=mdl[HUMAN21_MUNE].ty;
    vec.vz=mdl[HUMAN21_MUNE].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_MUNE]));


    /* MUNE+KUBI+ATAMA */
    vec.vx=mdl[HUMAN21_KUBI].tx+mdl[HUMAN21_ATAMA].tx;
    vec.vy=mdl[HUMAN21_KUBI].ty+mdl[HUMAN21_ATAMA].ty;
    vec.vz=mdl[HUMAN21_KUBI].tz+mdl[HUMAN21_ATAMA].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_MUNE]),
		 &(scratchpad->x[HUMAN21_ATAMA]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->root));

    MT_QuatMul(&q,&(scratchpad->inv_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MUNE]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		     &(scratchpad->abs_rots[HUMAN21_MUNE]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_ATAMA))){
	    scratchpad->flags&=~(1<<HUMAN21_ATAMA);
	}
    }

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_KUBI]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KUBI]),
		     &(scratchpad->abs_rots[HUMAN21_KUBI]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KUBI]),
			 &(scratchpad->abs_rots[HUMAN21_KUBI]));
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_ATAMA))){
	    scratchpad->flags&=~(1<<HUMAN21_ATAMA);
	}
    }


    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_MUNE]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MUNE]));

    vec.vx=mdl[HUMAN21_KUBI].tx;
    vec.vy=mdl[HUMAN21_KUBI].ty;
    vec.vz=mdl[HUMAN21_KUBI].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vec.vx=mdl[HUMAN21_MIGI_KATA].tx;
    vec.vy=mdl[HUMAN21_MIGI_KATA].ty;
    vec.vz=mdl[HUMAN21_MIGI_KATA].tz;
    vu0_Ldv1(&vec);

    vec.vx=mdl[HUMAN21_HIDARI_KATA].tx;
    vec.vy=mdl[HUMAN21_HIDARI_KATA].ty;
    vec.vz=mdl[HUMAN21_HIDARI_KATA].tz;
    vu0_Ldv2(&vec);

    vu0_Mulv0m0v0();
    vu0_Mulv1m0v1();
    vu0_Mulv2m0v2();

    vu0_Stv0(&(scratchpad->x[HUMAN21_KUBI]));
    vu0_Stv1(&(scratchpad->x[HUMAN21_MIGI_KATA]));
    vu0_Stv2(&(scratchpad->x[HUMAN21_HIDARI_KATA]));
}

#endif


#undef SLERP_PARAM

#if 0
#define SLERP_PARAM	(1.0f/4.0f)
#define SLERP_PARAM2	(4.0/5.0f)
#elif 0
#define SLERP_PARAM	(4.0/5.0f)
#define SLERP_PARAM2	(1.0f/4.0f)
#else
#define SLERP_PARAM	(0.0f)
#define SLERP_PARAM2	(1.0f/3.0f)
#define SLERP_PARAM3	(1.0f/8.0f)
#define SLERP_PARAM4	(1.0f/4.0f)
#endif


static void CheckMotion(struct _scratchpad *scratchpad,Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    float p;

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_KOSHI]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	printf("KOSHI\n");
	printf("KOSHI S = %f %f %f %f\n",
	       scratchpad->abs_rots[HUMAN21_KOSHI].vx,
	       scratchpad->abs_rots[HUMAN21_KOSHI].vy,
	       scratchpad->abs_rots[HUMAN21_KOSHI].vz,
	       scratchpad->abs_rots[HUMAN21_KOSHI].vw);
	printf("KOSHI M = %f %f %f %f\n",
	       obj->m_ctrl->abs_rots[HUMAN21_KOSHI].vx,
	       obj->m_ctrl->abs_rots[HUMAN21_KOSHI].vy,
	       obj->m_ctrl->abs_rots[HUMAN21_KOSHI].vz,
	       obj->m_ctrl->abs_rots[HUMAN21_KOSHI].vw);

	if(!(scratchpad->flags2 & (1<<HUMAN21_KOSHI))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_KOSHI]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			     &(scratchpad->abs_rots[HUMAN21_KOSHI]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_KOSHI]),SLERP_PARAM2);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			     &(scratchpad->abs_rots[HUMAN21_KOSHI]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_KOSHI))){
	    scratchpad->flags&=~(1<<HUMAN21_KOSHI);
	}
    }
#endif

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_ONAKA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	printf("ONAKA\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_ONAKA))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_ONAKA]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			     &(scratchpad->abs_rots[HUMAN21_ONAKA]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_ONAKA]),SLERP_PARAM2);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			     &(scratchpad->abs_rots[HUMAN21_ONAKA]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_ONAKA))){
	    scratchpad->flags&=~(1<<HUMAN21_ONAKA);
	}
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_MUNE]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MUNE\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_ATAMA))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MUNE]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			     &(scratchpad->abs_rots[HUMAN21_MUNE]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MUNE]),SLERP_PARAM2);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			     &(scratchpad->abs_rots[HUMAN21_MUNE]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_ATAMA))){
	    scratchpad->flags&=~(1<<HUMAN21_ATAMA);
	}
    }

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_KATA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MIGI KATA\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_MIGI_UDE1))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_KATA]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_KATA]),SLERP_PARAM2);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_UDE1))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_UDE1);
	}
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("MIGI UDE1\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_MIGI_UDE2))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE1]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE1]),SLERP_PARAM3);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_UDE2))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_UDE2);
	}
    }

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("MIGI UDE2\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_MIGI_TE))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE2]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE2]),SLERP_PARAM3);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_TE))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_TE);
	}
    }


#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_KATA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	printf("HIDARI KATA\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_HIDARI_UDE1))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_KATA]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_KATA]),SLERP_PARAM2);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_UDE1))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_UDE1);
	}
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("HIDARI UDE1\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_HIDARI_UDE2))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE1]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE1]),SLERP_PARAM3);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_UDE2))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_UDE2);
	}
    }

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("HIDARI UDE2\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_HIDARI_TE))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE2]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE2]),SLERP_PARAM3);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_TE))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_TE);
	}
    }

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MIGI ASHI1\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_MIGI_ASHI2))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI1]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI1]),SLERP_PARAM4);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_ASHI2))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_ASHI2);
	}
    }

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MIGI ASHI2\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_MIGI_KAKATO))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI2]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));

	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
			   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
			   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI2]),SLERP_PARAM4);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));

	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
			   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
			   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_MIGI_KAKATO))){
	    scratchpad->flags&=~(1<<HUMAN21_MIGI_KAKATO);
	}
    }

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("HIDARI ASHI1\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_HIDARI_ASHI2))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI1]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI1]),SLERP_PARAM4);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_ASHI2))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_ASHI2);
	}
    }

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("HIDARI ASHI2\n");

	if(!(scratchpad->flags2 & (1<<HUMAN21_HIDARI_KAKATO))){
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI2]),SLERP_PARAM);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));

	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
			   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
			   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
	}
	else{
	    MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI2]),SLERP_PARAM4);
	    MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));

	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
			   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
			   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
	}
    }
    else{
	if(!(corpsep->flags & (1<<HUMAN21_HIDARI_KAKATO))){
	    scratchpad->flags&=~(1<<HUMAN21_HIDARI_KAKATO);
	}
    }
}


static void RecalcAll(struct _scratchpad *scratchpad,Work *work,OBJECT *obj)
{
    /* 体の回転の再計算 */
    AdjustHip(CHEST_COLLISION,CHEST_HEIGHT,3,0,scratchpad,work);

#if 0
    AdjustJoint(HUMAN21_MUNE,CHEST_COLLISION,CHEST_HEIGHT,3,0,
		scratchpad,work);
#endif

    AdjustChest(CHEST_COLLISION,CHEST_HEIGHT,3,0,scratchpad,work);

    AdjustJoint(HUMAN21_ATAMA,HEAD_COLLISION,HEAD_HEIGHT,3,0,
		scratchpad,work);

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_ATAMA]),
		   &(scratchpad->abs_rots[HUMAN21_KUBI]));


    AdjustShoulder(HUMAN21_MIGI_KATA,0,scratchpad,work);

    AdjustJoint(HUMAN21_MIGI_UDE2,ARM_COLLISION,ARM_HEIGHT,3,0,
		scratchpad,work);

    AdjustJoint(HUMAN21_MIGI_TE,HAND_COLLISION,HAND_HEIGHT,3,0,
		scratchpad,work);

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));


    AdjustShoulder(HUMAN21_HIDARI_KATA,0,scratchpad,work);

    AdjustJoint(HUMAN21_HIDARI_UDE2,ARM_COLLISION,ARM_HEIGHT,3,0,
		scratchpad,work);

    AdjustJoint(HUMAN21_HIDARI_TE,HAND_COLLISION,HAND_HEIGHT,3,0,
		scratchpad,work);

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));


    AdjustJoint(HUMAN21_MIGI_ASHI2,NEE_COLLISION,NEE_HEIGHT,3,0,
		scratchpad,work);

    AdjustJoint(HUMAN21_MIGI_KAKATO,HEEL_COLLISION,HEEL_HEIGHT,3,0,
		scratchpad,work);

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));


    AdjustJoint(HUMAN21_HIDARI_ASHI2,NEE_COLLISION,NEE_HEIGHT,3,0,
		scratchpad,work);

    AdjustJoint(HUMAN21_HIDARI_KAKATO,HEEL_COLLISION,HEEL_HEIGHT,3,0,
		scratchpad,work);

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
}

static void HoldCorpse(Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    int size=HUMAN_MODEL_OBJN;
    int i;

    for(i=0;i<size;i++){
	fpu_CopyVector(&(obj->m_ctrl->abs_rots[i]),&(corpsep->abs_rots[i]));
	fpu_CopyVector(&(corpsep->x[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
	fpu_ClearVector(&(corpsep->v[i]));
    }
}

int FreeCorpse(Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    //DG_MDL *mdl=obj->objs->def->models;
    int size=HUMAN_MODEL_OBJN;
    int i,ans;
    struct _scratchpad *scratchpad;


    if(corpsep->count>SWITCH2HOLD_FRAMES){

#ifdef DEBUG
	if(corpsep->count==SWITCH2HOLD_FRAMES+1){
	    corpsep->count++;
	    printf("Finish FreeCorpse & Start HoldCorpse\n");
	}
#endif

	HoldCorpse(work);
	return 0;
    }

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#endif

    StartCorpse2(scratchpad,obj,corpsep);

#if 0

    {
	float s=0.0f;

	vu0_Ldv0(&(scratchpad->x[0]));
	vu0_Ldv1(&(corpsep->x[0]));
	vu0_Subv2v0v1();
	for(i=1;i<size;i++){
	    vu0_Stv0(&(corpsep->x[i-1]));
	    vu0_Ldv0(&(scratchpad->x[i]));
	    vu0_Ldv1(&(corpsep->x[i]));
	    s+=vu0_VectorLength2v2();
	    vu0_Stv2(&(scratchpad->v[i-1]));
	    vu0_Subv2v0v1();
	}

	vu0_Stv0(&(corpsep->x[size-1]));
	s+=vu0_VectorLength2v2();
	vu0_Stv2(&(scratchpad->v[size-1]));

	if(s>0.01f){
	    ans=1;
	    corpsep->count=0;
	}
	else{
	    ans=0;
	    corpsep->count++;
	}
    }

#else

    vu0_Ldv0(&(scratchpad->x[0]));
    vu0_Ldv1(&(corpsep->x[0]));
    vu0_Subv2v0v1();
    for(i=1;i<size;i++){
	vu0_Stv0(&(corpsep->x[i-1]));
	vu0_Ldv0(&(scratchpad->x[i]));
	vu0_Ldv1(&(corpsep->x[i]));
	vu0_Stv2(&(scratchpad->v[i-1]));
	vu0_Subv2v0v1();
    }

    vu0_Stv0(&(corpsep->x[size-1]));
    vu0_Stv2(&(scratchpad->v[size-1]));
    corpsep->count++;
    ans=1;

#endif



#if 0
    printf("a%d = %f %f %f\n",HUMAN21_KUBI,
	   scratchpad->a[HUMAN21_KUBI].vx,
	   scratchpad->a[HUMAN21_KUBI].vy,
	   scratchpad->a[HUMAN21_KUBI].vz);
    printf("a%d = %f %f %f\n",HUMAN21_MUNE,
	   scratchpad->a[HUMAN21_KUBI].vx,
	   scratchpad->a[HUMAN21_KUBI].vy,
	   scratchpad->a[HUMAN21_KUBI].vz);
    printf("a%d = %f %f %f\n",HUMAN21_ONAKA,
	   scratchpad->a[HUMAN21_KUBI].vx,
	   scratchpad->a[HUMAN21_KUBI].vy,
	   scratchpad->a[HUMAN21_KUBI].vz);
#endif

#if 0
    for(i=0;i<size;i++){
	printf("a%d = %f %f %f\n",i,
	       scratchpad->a[i].vx,scratchpad->a[i].vy,scratchpad->a[i].vz);
    }
#endif

    vu0_Ldv1(&(scratchpad->v[0]));
    vu0_Ldv0(&(scratchpad->x[0]));

    vu0_Mulv1a(0.7f);

    vu0_Addv0v1();

    for(i=1;i<size;i++){
	vu0_Ldv1(&(scratchpad->v[i]));
	vu0_Stv0(&(scratchpad->x[i-1]));
	vu0_Mulv1a(0.7f);
	vu0_Ldv0(&(scratchpad->x[i]));
	vu0_Addv0v1();
    }
    vu0_Stv0(&(scratchpad->x[size-1]));

    RecalcAll(scratchpad,work,obj);

#if 0
    CheckBackBone(scratchpad,work);

    MT_QuatMul(&(corpsep->main_abs_rots),&(scratchpad->root),
	       &(scratchpad->abs_rots[HUMAN21_KOSHI]));


    fpu_CopyVector((FVECTOR *)&(scratchpad->m_rmat.m[3][0]),
		   &(scratchpad->x[HUMAN21_KOSHI]));
    DG_SetPos(&(scratchpad->m_rmat));

    for(i=0;i<size;i++){
	MT_QuatMul(&(obj->m_ctrl->abs_rots[i]),&(scratchpad->conv_root),
		   &(scratchpad->abs_rots[i]));
	fpu_CopyVector(&(corpsep->abs_rots[i]),&(obj->m_ctrl->abs_rots[i]));
    }

#else

    for(i=2;i<size;i++){
	MT_QuatMul(&(scratchpad->abs_rots[i]),&(scratchpad->conv_root),
		   &(scratchpad->abs_rots[i]));
    }

    CheckMotion(scratchpad,work);

    MT_QuatMul(&(corpsep->main_abs_rots),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_KOSHI]));

    fpu_CopyVector((FVECTOR *)&(scratchpad->m_rmat.m[3][0]),
		   &(scratchpad->x[HUMAN21_KOSHI]));
    DG_SetPos(&(scratchpad->m_rmat));

    for(i=0;i<size;i++){
	fpu_CopyVector(&(obj->m_ctrl->abs_rots[i]),&(scratchpad->abs_rots[i]));
	fpu_CopyVector(&(corpsep->abs_rots[i]),&(scratchpad->abs_rots[i]));
    }

    corpsep->flags=scratchpad->flags;
    corpsep->flags2=scratchpad->flags2;

#if 0
    printf("CorpseF , CorpseF2 = 0x%08x , 0x%08x\n",
	   corpsep->flags,corpsep->flags2);
#endif

#endif

#if 0
    ScratchpadFree(scratchpad);
#else
    ScratchpadFree2();
#endif

    return ans;
}

void JumpFreeCorpse(Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    int size=HUMAN_MODEL_OBJN;
    int i;

    for(i=0;i<size;i++){
	corpsep->x[i].vy+=(10.0f+rnd())*P_GRAVITY;
    }

    corpsep->count=0;
    corpsep->flags=0;
}

void JumpFreeCorpse2(Work *work,int objnum)
{
    CORPSE_PARAMETER *corpsep=&(work->corpsep);

    corpsep->x[objnum].vy+=(10.0f+rnd())*P_GRAVITY;
    corpsep->x[HUMAN21_MUNE].vy+=(3.0f+rnd())*P_GRAVITY;

    corpsep->count=0;
    corpsep->flags=0;
}
