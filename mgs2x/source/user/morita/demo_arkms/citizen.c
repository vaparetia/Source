//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  citizen.c
  ＮＹ市民 初期化関数群

  2000/01/24 K.Kano
  $Id: citizen.c,v 1.1.1.3 2002/11/19 11:45:54 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libdma.h>
#include <libdev.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "cit_male_all_def.kh"
#include "cti_female.kh"

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

#define CITIZEN_M_TYPE_MIN 0
#define CITIZEN_M_TYPE_MAX sizeof(kls_cit_male_all_def)/(sizeof(int)*21)

#define CITIZEN_F_TYPE_MIN 0
#define CITIZEN_F_TYPE_MAX sizeof(kls_cti_female)/(sizeof(int)*21)


int InitObject_CitizenMale( OBJECT     *object     ,
			    OBJECT_CHG *object_chg , int type )
{
    if ( type < CITIZEN_M_TYPE_MIN ||
	 type >= CITIZEN_M_TYPE_MAX )
    {
	printf( "Number Exceed %d < %d : InitObject_CitizenMale\n", type, CITIZEN_M_TYPE_MAX ) ;
	return 0 ;
    }
    object_chg->type = type ;
    object_chg->objch = GM_InitObjectForObjChange( object,

						   9938738/*cit_malea_def*/,
						   481642 /*cit_male_all_def*/,
						   kls_cit_male_all_def[type],
						   DG_FLAG_SHADE|DG_FLAG_FINISHCALC ) ;
    if ( object_chg->objch == NULL )
	return 0 ;

    return 1 ;
}

int InitObject_CitizenFemale( OBJECT     *object     ,
			      OBJECT_CHG *object_chg , int type )
{
    if ( type < CITIZEN_F_TYPE_MIN ||
	 type >= CITIZEN_F_TYPE_MAX )
    {
	printf( "Number Exceed %d < %d : InitObject_CitizenFemale\n", type, CITIZEN_F_TYPE_MAX ) ;
	return 0 ;
    }
printf( "TYPE%d\n", type ) ;
{
    int i ;
    for( i=0 ; i<21 ; i++ )
	printf( "%d ", kls_cti_female[type][i] ) ;
    printf( "\n" ) ;
}
    object_chg->type = type ;
    object_chg->objch = GM_InitObjectForObjChange( object,
						   15381012/*cit_femalea_def*/,
						   6255236 /*cti_female*/,
						   kls_cti_female[type],
						   DG_FLAG_SHADE|DG_FLAG_FINISHCALC ) ;
    if ( object_chg->objch == NULL )
	return 0 ;

    return 1 ;
}

void FreeObject_Citizen( OBJECT *object, OBJECT_CHG *work )
{
    GM_FreeObjectForObjChange( object, work->objch ) ;
}

