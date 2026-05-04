//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_equip.c
   オルガ専用 装備品キャラ

   2000/10/03 T. Morita
   $Id: demo_meryl.c,v 1.1.1.3 2002/11/19 11:45:58 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "../include/util.h"


enum org_equop_t
{
    MRL_EQUIP_PACK = 0x0001,
    MRL_EQUIP_MAG  = 0x0002,
    MRL_EQUIP_HAIR = 0x0004,
} ;

typedef struct work_t
{
    GV_ACT_EX actor ;
    int       name  ;
    OBJECT   *body  ;
    DG_OBJS  *pack  ;
    DG_OBJS  *mag   ;
} Work ;


static void Die( Work *work )
{
    if ( work->mag )
	DG_DequeueObjs( work->mag ), DG_FreeObjs( work->mag ) ;
    if ( work->pack )
	DG_DequeueObjs( work->pack ), DG_FreeObjs( work->pack ) ;
}

void MRL_SendHairMessage( int flag )
{
    GV_MSG msg ;
    int buffer[] = { 0, flag } ;

    msg.address = 5115309 /*メリルの髪の毛*/ ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

static void Act( Work *work )
{
    if ( work->mag && work->pack ) {
	if ( work->body->objs->flag &  DG_FLAG_INVISIBLE &&
	     !(work->body->evmobj->flag & DG_EVMOBJ_INVISIBLE) ) {
	    DG_VisibleObjs( work->pack ) ;
	    DG_VisibleObjs( work->mag  ) ;
	    MRL_SendHairMessage( 1 ) ;
	} else {
	    DG_InvisibleObjs( work->pack ) ;
	    DG_InvisibleObjs( work->mag  ) ;
	    MRL_SendHairMessage( 0 ) ;
	}
	_sceVu0CopyMatrix( &work->pack->world,
			   &work->body->objs->objs[HUMAN21_KOSHI].world ) ;
	_sceVu0CopyMatrix( &work->mag->world,
			   &work->body->objs->objs[HUMAN21_KOSHI].world ) ;
    }
}

static int GetResources( Work *work, int name, CONTROL *ctrl, int flag )
{
    DG_DEF *def ;

    work->name = name ;
    work->body = ctrl->object ;

    /* メリル装備 */
    if ( flag & MRL_EQUIP_PACK )
    {
	def = GV_GetCache( GV_CacheID( 5197317/*mrl_waist_pack*/, 'k' ) ) ;
	if ( def==NULL )
	    PERROR( "No model for Meryle MagPack : NewMeryleEquip\n" ) ;
	work->pack = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
	if ( !work->pack )
	    PERROR( "Cannot create Pack for Meryle DG_OBJS(no memory) : NewMeryleEquip\n" ) ;
	DG_QueueObjs( work->pack ) ;
	//GM_GroupObjs( work->pack, where ) ;
    }

    /* メリル装備 */
    if ( flag & MRL_EQUIP_MAG )
    {
	def = GV_GetCache( GV_CacheID( 13790832/*mrl_waist_mag*/, 'k' ) ) ;
	if ( def==NULL )
	    PERROR( "No model for Meryle Mag : NewMeryleEquip\n" ) ;
	work->mag = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
	if ( !work->mag )
	    PERROR( "Cannot create Mag for Meryle DG_OBJS(no memory) : NewMeryleEquip\n" ) ;
	DG_QueueObjs( work->mag ) ;
	//GM_GroupObjs( work->mag, where ) ;
    }


    /* メリル装備 */
    if ( flag & MRL_EQUIP_HAIR )
    {
	void *NewDemoAllEquip( int name, CONTROL *ctrl, int type, int flag ) ;

	NewDemoAllEquip( 5115309 /*メリルの髪の毛*/,
			 ctrl,
			 5494549, /* mrl_def_mh_mt.evm      本体   */
			 0 ) ;
    }

    return 0 ;
}

void *NewMeryleEquip( int name, CONTROL *ctrl, int flag )
{
    Work *work ;

    work = (Work *)GV_NewActorPrio( GV_ACTOR_USER, sizeof(Work), 0x08 ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, ctrl, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
