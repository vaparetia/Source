//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	nypd.c
		ニューヨーク市警初期化関数群

	2000/01/24 K.Kano
	$Id: nypd.c,v 1.1.1.3 2002/11/19 11:43:25 Yoshizawa1 Exp $
*/

#include "nypd.h"
#include "nyp_def_epa.kh"

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

enum {
    NYPD_TYPE_MIN=0,
    NYPD_TYPE_MAX=sizeof(kls_nyp_def_epa)/(sizeof(int)*21),
};


int InitObject_NYPD(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type)
{
    if(type<NYPD_TYPE_MIN || type>NYPD_TYPE_MAX) return 0;

    if((object_chg->objch=GM_InitObjectForObjChange(object,
						    GV_StrCode("nyp_def1"),
						    GV_StrCode("nyp_def_epa"),
						    /* DEFAULT_NYP_DEF_EPA, */
						    kls_nyp_def_epa[type],
						    flag))==NULL) return 0;

    object_chg->type=type;

    return 1;
}

void FreeObject_NYPD(OBJECT *object,OBJECT_CHG *work)
{
    GM_FreeObjectForObjChange(object,work->objch);
}
