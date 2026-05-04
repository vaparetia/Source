//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	corpse.c
		死体の姿勢制御

	1999/10/5 K.Kano
	$Id: corpse.c,v 1.1.1.3 2002/11/19 11:43:08 Yoshizawa1 Exp $
*/


#include "corpse.h"


/* ------------------------------------------------------------------------ */
/* 初期化部 */

static void InitCorpse(Work *work,CONTROL *ctrl,OBJECT *obj)
{
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
#if 0
    FMATRIX rmat;
    FVECTOR root;
#endif
    int size=HUMAN_MODEL_OBJN;
    int i;

    work->control=ctrl;
    work->obj=obj;

    corpsep->count=0;

    for(i=0;i<size;i++){
		fpu_CopyVector(&(corpsep->mtn_abs_rots[i]),&(obj->m_ctrl->abs_rots[i]));
		fpu_CopyVector(&(corpsep->abs_rots[i]),&(obj->m_ctrl->abs_rots[i]));
		fpu_ClearVector(&(corpsep->diff_rots[i]));
		corpsep->diff_rots[i].vw=1.0f;
		fpu_CopyVector(&(corpsep->x[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
		fpu_CopyVector(&(corpsep->basex[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
		fpu_ClearVector(&(corpsep->v[i]));
    }
#if 0
    DG_GetPos(&rmat);

    MT_MatToQuat(&root,&rmat);
    MT_QuatMul(&(corpsep->main_abs_rots),&root,
			   &(obj->m_ctrl->abs_rots[HUMAN21_KOSHI]));
#else
    MT_MatToQuat(&(corpsep->main_abs_rots),&(obj->objs->objs[HUMAN21_KOSHI].world));
#endif
}

void DummyFreeCorpse(Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
#if 0
    FVECTOR root;
#endif
    int size=HUMAN_MODEL_OBJN;
    int i;

    corpsep->count=0;

    for(i=0;i<size;i++){
		fpu_CopyVector(&(corpsep->mtn_abs_rots[i]),&(obj->m_ctrl->abs_rots[i]));
		fpu_CopyVector(&(corpsep->abs_rots[i]),&(obj->m_ctrl->abs_rots[i]));
		fpu_ClearVector(&(corpsep->diff_rots[i]));
		corpsep->diff_rots[i].vw=1.0f;
		fpu_CopyVector(&(corpsep->x[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
		fpu_CopyVector(&(corpsep->basex[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
		fpu_ClearVector(&(corpsep->v[i]));
    }

#if 0
    MT_MatToQuat(&root,&(obj->objs->world));
    MT_QuatMul(&(corpsep->main_abs_rots),&root,
			   &(obj->m_ctrl->abs_rots[HUMAN21_KOSHI]));
#else
    MT_MatToQuat(&(corpsep->main_abs_rots),&(obj->objs->objs[HUMAN21_KOSHI].world));
#endif
}

/* 初期化部メイン */
void *StartCorpseIK(CONTROL *ctrl,OBJECT *obj)
{
    Work *work ;

    work=(Work *)GV_Malloc(sizeof(Work)) ;
    if(work!=NULL) {
		InitCorpse(work,ctrl,obj);
    }
    return (void *)work ;
}

void ExitCorpseIK(Work *work)
{
    GV_Free(work);
}

/* libmtでモーションを変更する際に、以前のabs_rotsとを用いて、補間を行なっている。
   そのため、モーションを設定する際、以前のabs_rotsをコピーして保管されてしまう。
   しかし、そのabs_rotsがFreeCorpseで変更済のものであった場合、次のフレームでFreeCorpseが
   呼ばれると、変更に変更を重ねてしまい、多重に補正がかかってしまう。
   そこで、この関数を用いて、モーション設定前(GM_ConfigObjectActionを呼び出す前)に
   補正前のabs_rotsに戻し、補正が多重にかからないようにする。*/
void Corpse_ChangeAction(Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    int size=HUMAN_MODEL_OBJN;
    int i;

    for(i=0;i<size;i++){
		fpu_CopyVector(obj->m_ctrl->abs_rots+i,corpsep->mtn_abs_rots+i);
    }
}
