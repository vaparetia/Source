//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_allchg.c
   ARKMS菅変え専用 キャラ

   2000/12/22 T. Morita
   $Id: demo_allchg.c,v 1.1.1.3 2002/11/19 11:45:54 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"
#include "../include/util.h"


typedef struct work_t
{
    GV_ACT_EX   actor    ;

    OBJECT_CHG  changed  ;
    OBJECT     *object   ;
    int         name     ;
    int         kms_flag ;
} Work ;

extern int InitObject_USSoldier( OBJECT *object, int flag,
				 OBJECT_CHG *object_chg, int type ) ;
extern int InitObject_NYPD(OBJECT *object,int flag,
			   OBJECT_CHG *object_chg,int type) ;
extern int InitObject_CitizenFemale( OBJECT     *object     ,
				     OBJECT_CHG *object_chg , int type ) ;
extern int InitObject_CitizenMale( OBJECT *object,
				   OBJECT_CHG *object_chg, int type ) ;
extern int InitObject_Hostage( OBJECT *object, int flag,
			       OBJECT_CHG *object_chg, int type ) ;

static void Die( Work *work )
{
    if ( work->changed.objch )
	GV_Free(work->changed.objch ) ;
}

static void Act( Work *work )
{
    if ( work->kms_flag )
	work->object->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;
}

static int GetResourcesCalled( Work *work, int name, OBJECT *object,
			       int mode, int type, int kms_flag )
{
    int det ;
    DG_OBJS *objs ;

    work->name     = name     ;
    work->object   = object   ;
    work->kms_flag = kms_flag ;

    objs = object->objs ;

    det = 0 ;
    if ( mode == 1 )
	det = InitObject_USSoldier( object, DG_FLAG_SHADE|DG_FLAG_FINISHCALC,
				    &work->changed, type ) ;
    else if ( mode == 2 )
	det = InitObject_NYPD( object, DG_FLAG_SHADE|DG_FLAG_FINISHCALC,
			       &work->changed, type ) ;
    else if ( mode == 3 )
	det = InitObject_CitizenFemale( object, &work->changed, type ) ;
    else if ( mode == 4 )
	det = InitObject_CitizenMale( object, &work->changed, type ) ;
    else if ( mode == 5 )
	det = InitObject_Hostage( object, DG_FLAG_SHADE|DG_FLAG_FINISHCALC,
				  &work->changed, type ) ;
    else if ( mode == 6 )
	det = InitObject_CitizenFemale( object, &work->changed, 12 ) ;
    else if ( !kms_flag )
	return -1 ;

    if ( !det )
    {
	if ( !kms_flag || !object->evmobj )
	    PERROR( "Cannot initialize  : NewDemoAllChange\n" ) ;
    }
    else if ( objs )
	DG_DequeueObjs( objs ), DG_FreeObjs( objs ) ;
    object->objs->flag |= DG_FLAG_INVISIBLE ;

    return 0 ;
}


void *NewDemoAllChange( int name, OBJECT *object, int mode, int type, int flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if ( GetResourcesCalled( work, name, object, mode, type, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

