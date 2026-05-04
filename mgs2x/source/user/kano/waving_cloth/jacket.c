//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	jacket.c
		布シミュレート
		モデルが全て布モデルであるようなケース

	1999/11/19 K.Kano
	$Id: jacket.c,v 1.1.1.3 2002/11/19 11:43:44 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif


#include "waving_cloth.h"


void CopyMatrixforJacket(Work *work)
{
    DG_OBJ *sobj=work->target->objs->objs;
    // DG_OBJ *dobj=work->body.objs->objs;
	DG_OBJ *dobj=work->objs->objs;

    fpu_CopyMatrix(&(dobj[4].world),&(sobj[HUMAN21_MIGI_KATA].world));
    fpu_CopyMatrix(&(dobj[5].world),&(sobj[HUMAN21_MIGI_UDE1].world));
    fpu_CopyMatrix(&(dobj[6].world),&(sobj[HUMAN21_MIGI_UDE2].world));

    fpu_CopyMatrix(&(dobj[1].world),&(sobj[HUMAN21_HIDARI_KATA].world));
    fpu_CopyMatrix(&(dobj[2].world),&(sobj[HUMAN21_HIDARI_UDE1].world));
    fpu_CopyMatrix(&(dobj[3].world),&(sobj[HUMAN21_HIDARI_UDE2].world));

    // if(work->body.objs->n_models<8) return;
	if(work->objs->n_models<8) return;

    fpu_CopyMatrix(&(dobj[7].world),&(sobj[HUMAN21_KUBI].world));

    // if(work->body.objs->n_models<9) return;
	if(work->objs->n_models<9) return;

    fpu_CopyMatrix(&(dobj[8].world),&(sobj[HUMAN21_ATAMA].world));
}
