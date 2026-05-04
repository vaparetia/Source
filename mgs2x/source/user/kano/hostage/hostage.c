//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	hostage.c
		人質初期化関数群

	2000/01/24 K.Kano
	$Id: hostage.c,v 1.1.1.3 2002/11/19 11:43:13 Yoshizawa1 Exp $
*/

#include "hostage.h"
#include "hos_all_def.kh"
#include "hos_all_mid.kh"
#include "hos_all_low.kh"

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

enum {
    HOSTAGE_TYPE_MIN=0,
    HOSTAGE_TYPE_MAX=sizeof(kls_hos_all_def)/(sizeof(int)*21),
};


int InitObject_Hostage(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type)
{
    if(type<HOSTAGE_TYPE_MIN || type>HOSTAGE_TYPE_MAX) return 0;

    if((object_chg->objch=GM_InitObjectForObjChange(object,
//						    GV_StrCode("ric_def"),
						    GV_StrCode("hos_malea_def"),
						    GV_StrCode("hos_all_def"),
						    /* DEFAULT_HOS_ALL_DEF, */
						    kls_hos_all_def[type],
						    flag))==NULL) return 0;

    object_chg->type=type;

    return 1;
}

int InitObject_MidHostage(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type)
{
    if(type<HOSTAGE_TYPE_MIN || type>HOSTAGE_TYPE_MAX) return 0;

    if((object_chg->objch=GM_InitObjectForObjChange(object,
//						    GV_StrCode("ric_mid"),
						    GV_StrCode("hos_malea_mid"),
						    GV_StrCode("hos_all_mid"),
						    /* DEFAULT_HOS_ALL_DEF, */
						    kls_hos_all_mid[type],
						    flag))==NULL) return 0;

    object_chg->type=type;

    return 1;
}

int InitObject_LowHostage(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type)
{
    if(type<HOSTAGE_TYPE_MIN || type>HOSTAGE_TYPE_MAX) return 0;

    if((object_chg->objch=GM_InitObjectForObjChange(object,
//						    GV_StrCode("ric_low"),
						    GV_StrCode("hos_malea_low"),
						    GV_StrCode("hos_all_low"),
						    /* DEFAULT_HOS_ALL_DEF, */
						    kls_hos_all_low[type],
						    flag))==NULL) return 0;

    object_chg->type=type;

    return 1;
}

void FreeObject_Hostage(OBJECT *object,OBJECT_CHG *work)
{
    GM_FreeObjectForObjChange(object,work->objch);
}
