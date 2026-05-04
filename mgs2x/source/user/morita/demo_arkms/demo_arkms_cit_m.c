//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_arkms.c
   ARKMS専用 キャラ

   2000/12/22 T. Morita
   $Id: demo_arkms_cit_m.c,v 1.1.1.3 2002/11/19 11:45:54 Yoshizawa1 Exp $
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


int  InitObject_CitizenMale(OBJECT *object,OBJECT_CHG *object_chg,int type ) ;

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
	GV_Free( work->changed.objch ) ;
}

static int GetResources( Work *work, int name, OBJECT *object,
			 int type, int flag )
{
    DG_OBJS *objs ;
    extern int InitObject_CitizenMale( OBJECT *object,
				       OBJECT_CHG *object_chg , int type ) ;
    extern void *NewCreateEquipment(OBJECT *object, short ID, int Data_Num) ;

    /* もう髪の毛がある物には付けられない */
    if ( type % 6 == 2 || type % 6 == 4 || type % 6 == 5 )
	flag &= ~0xf ;

    /* 装備品 */
    GV_SetActorChild( work,
		      NewCreateEquipment( object, 1, flag ) ) ;

    work->name = name ;
    objs = object->objs ;
    if ( !InitObject_CitizenMale( object, &work->changed, type ) )
	PERROR( "Cannot initialize InitObject_Citizen() : NewDemoArkms" ) ;
    if ( objs )
	DG_DequeueObjs( objs ), DG_FreeObjs( objs ) ;
    work->object = object ;
    if ( work->object->evmobj )
	work->object->evmobj->flag |= DG_EVMOBJ_INVISIBLE ;

    return 0 ;
}

void *NewDemoArkms_CitMale( int name, OBJECT *object, int type, int flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, name, object, type, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
