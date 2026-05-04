//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_equip.c
   オルガ専用 装備品キャラ

   2000/10/03 T. Morita
   $Id: demo_equip.c,v 1.1.1.3 2002/11/19 11:45:58 Yoshizawa1 Exp $
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
    ORG_EQUIP_RADIO  = 0x0001,
    ORG_EQUIP_KNIFE  = 0x0002,
    ORG_EQUIP_HOLSTER= 0x0004,
} ;

typedef struct work_t
{
    GV_ACT_EX actor ;
    int       name  ;
    OBJECT   *body  ;
    DG_OBJS  *gun   ;
} Work ;

static void Die( Work *work )
{
    if ( work->gun )
	DG_DequeueObjs( work->gun ), DG_FreeObjs( work->gun ) ;
}

static void Act( Work *work )
{
    int     i ;
    GV_MSG *msg ;

    if ( work->gun )
    {
	for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	    switch( msg->message[0] )
	    {
	    case 0:
		DG_VisibleObjs( work->gun ) ;
		break ;
	    case 1:
		DG_InvisibleObjs( work->gun ) ;
		break ;
	    }	
	_sceVu0CopyMatrix( &work->gun->world, &work->body->objs->objs[HUMAN21_MIGI_TE].world ) ;
    }
}

static int GetResources( Work *work, int name, OBJECT *body, int flag, int wpname )
{
    extern void *NewAttachment_called( int model_name, FVECTOR *v,SVECTOR *r,
				       OBJECT *target, int objnum, FVECTOR *x,
				       int *a, int angle_limit, float oval_param ) ;
    DG_DEF *def ;
    void   *child ;

    work->name = name ;
    work->body = body ;
    if ( wpname )
    {
	if ( !(def = GV_GetCache( GV_CacheID( wpname, 'k' ) )) )
	    PERROR( "Cannot find Weapon KMS-Model( Not in data.cnf ) : NewOrgaEquip\n" ) ;
	if ( !(work->gun = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Cannot create Weapon DG_OBJS( Maybe no memory ) : NewOrgaEquip\n" ) ;
	DG_QueueObjs( work->gun ) ;
	//GM_GroupObjs( work->gun, where ) ;
    }

    /* 装備品Ａ オルガ無線機 */
    if ( flag & ORG_EQUIP_RADIO )
    {
	static FVECTOR pos = { 67.0f,   90.0f, -51.0f, 1.0f } ;
	static FVECTOR aim = {  0.0f, -243.0f,   0.0f, 1.0f } ;
	static SVECTOR rot = { (0*4096/360), (157*4096/360), (0*4096/360), 0 } ;
	static int axis[3] = {1,0,0} ;
	
	if ( !(child = NewAttachment_called( GV_StrCode( "org_radio" ),
					     &aim, &rot,
					     body, HUMAN21_KOSHI, &pos,
					     axis, (20*4096/360), 1.1f ) ) )
	    PERROR( "NewAttachment_called Failed for RADIO : NewOrgaEquip\n" ) ;
	GV_SetActorChild( work, child ) ;
    }

    /* 装備品Ａ オルガスカウトナイフ */
    if ( flag & ORG_EQUIP_KNIFE )
    {
	static FVECTOR pos = { 76.0f,   90.0f, 107.0f, 1.0f } ;
	static FVECTOR aim = {  0.0f, -243.0f,   0.0f, 1.0f } ;
	static SVECTOR rot = { (0*4096/360), (8*4096/360), (15*4096/360), 0 } ;
	static int axis[3] = {1,0,1} ;
	
	if ( !(child = NewAttachment_called( GV_StrCode( "org_sk_cover" ),
					     &aim, &rot,
					     body, HUMAN21_KOSHI, &pos,
					     axis, (45*4096/360), 1.1f ) ) )
	    PERROR( "NewAttachment_called Failed for KNIFE : NewOrgaEquip\n" ) ;
	GV_SetActorChild( work, child ) ;
    }

    /* 装備品Ａ オルガＵＳＰホルスター */
    if ( flag & ORG_EQUIP_HOLSTER )
    {
	static FVECTOR pos = { -110.0f,   90.0f,  55.0f, 1.0f } ;
	static FVECTOR aim = {    0.0f, -243.0f,   0.0f, 1.0f } ;
	static SVECTOR rot = { (0*4096/360), (-55*4096/360), (-10*4096/360), 0 } ;
	static int axis[3] = {1,0,1} ;
	
	if ( !(child = NewAttachment_called( GV_StrCode( "org_gbhlst" ),
					     &aim, &rot,
					     body, HUMAN21_KOSHI, &pos,
					     axis, (20*4096/360), 0.9f ) ) )
	    PERROR( "NewAttachment_called Failed for KNIFE : NewOrgaEquip\n" ) ;
	GV_SetActorChild( work, child ) ;
    }

    return 0 ;
}

void *NewOrgaEquip( int name, OBJECT *body, int flag, int wpname )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work, name, body, flag, wpname ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
