//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	us_soldier.c
		アメリカ兵初期化関数群

	2000/01/24 K.Kano
	$Id: us_soldier.c,v 1.1.1.3 2002/11/19 11:43:43 Yoshizawa1 Exp $
*/

#include "us_soldier.h"
#include "w04a_us_def.kh"

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

enum {
    US_SOLDIER_TYPE_MIN=0,
    US_SOLDIER_TYPE_MAX=sizeof(kls_w04a_us_def)/(sizeof(int)*21),
};


int InitObject_USSoldier(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type)
{
    if(type<US_SOLDIER_TYPE_MIN || type>US_SOLDIER_TYPE_MAX) return 0;

    if((object_chg->objch=GM_InitObjectForObjChange(object,
						    GV_StrCode("us_def_1"),
						    GV_StrCode("w04a_us_def"),
						    /* DEFAULT_W04A_US_DEF, */
						    kls_w04a_us_def[type],
						    flag))==NULL) return 0;

    object_chg->type=type;

    return 1;
}

void FreeObject_USSoldier(OBJECT *object,OBJECT_CHG *work)
{
    GM_FreeObjectForObjChange(object,work->objch);
}
