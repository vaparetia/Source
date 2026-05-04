//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_arkms.c
   ARKMS専用 キャラ

   2000/12/22 T. Morita
   $Id: demo_arkms.c,v 1.1.1.3 2002/11/19 11:45:54 Yoshizawa1 Exp $
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
    GV_ACT_EX   actor   ;

    int         name    ;
    OBJECT     *object  ;
    OBJECT_CHG  changed ;
} Work ;


int  InitObject_USSoldier(OBJECT *object,int flag,OBJECT_CHG *object_chg,int type);
void FreeObject_USSoldier(OBJECT *object,OBJECT_CHG *work);

static void Act( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
    {
//	printf( msg->message[0] ? "INVISIBLE\n" : "VISIBLE\n" ) ;
        if ( msg->message[0] )
	    work->object->objs->flag |=  DG_FLAG_INVISIBLE ;
	else
	    work->object->objs->flag &= ~DG_FLAG_INVISIBLE ;
    }
}

static void Die( Work *work )
{

    if ( work->changed.objch )
		GV_Free(work->changed.objch );
	//FreeObject_USSoldier( work->object, &work->changed ) ;
}

static int GetResources( Work *work, int name, OBJECT *object, int type )
{
    DG_OBJS *objs ;
    extern int InitObject_USSoldier( OBJECT *object, int flag,
				     OBJECT_CHG *object_chg, int type ) ;

    work->name = name ;

    objs = object->objs ;
    if ( !InitObject_USSoldier( object, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, &work->changed, type ) )
	PERROR( "Cannnot initialize InitObject_USSoldier()  : NewDemoArkms" ) ;
    if ( objs )
	DG_DequeueObjs( objs ), DG_FreeObjs( objs ) ;
    work->object = object ;
    if ( work->object->evmobj )
	work->object->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;

    return 0 ;
}

void *NewDemoArkms( int name, OBJECT *object, int type )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, name, object, type ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
