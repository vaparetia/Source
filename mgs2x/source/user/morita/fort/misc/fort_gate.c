/*
   fort_gate.c 
   昇降機 扉

   2001/01/16 T.Morita
   $Id: fort_gate.c,v 1.1.1.3 2002/11/19 11:46:17 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>

#include  "gameheader.h"
#include  "libutl.h"

#include "../../include/libdg_x.h"
#include "../../brk_utl/brk_utl.x"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

enum flag_t
{
    FRT_MOTION_NOMOVE = 0x0001,
    FRT_MOTION_LOOP   = 0x0002,
    FRT_MOTION_STOP   = 0x0004,
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR   pos    ;
    FVECTOR   vel    ;
    SVECTOR   rot    ;
    OBJECT    body   ;

    int       proc   ;

    int       flag   ;
    int       name   ;
} Work ;


#define PERROR(_s...) ({ printf(_s) ; return -1 ; } )

static inline void ActRecieveMessage( Work *work )
{
    GV_MSG *msg ;
    int i ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch( msg->message[0] )
	{
	case 0:
	    work->rot.vx = msg->message[1] ;
	    work->rot.vy = msg->message[2] ;
	    work->rot.vz = msg->message[3] ;
	    break ;
	case 1:
	    work->pos.vx = msg->message[1] ;
	    work->pos.vy = msg->message[2] ;
	    work->pos.vz = msg->message[3] ;
	    break ;
	case 2:
	    work->flag |=  FRT_MOTION_STOP ;
	    break ;
	case 3:
	    work->flag &= ~FRT_MOTION_STOP ;
	    GM_ConfigObjectAction( &work->body, 0, msg->message[1], 0, 0xfffff, msg->message[2] ) ;
	    break ;
	}
}

static void Act( Work *work )
{
    ActRecieveMessage( work ) ;

    if ( !(work->flag & FRT_MOTION_STOP) )
    {
	FVECTOR pos ;

	GM_ActMotion( &work->body ) ;
	if ( !(work->flag & FRT_MOTION_NOMOVE) )
	    _sceVu0AddVector( &work->pos, &work->pos, &work->vel ) ;
	_sceVu0CopyVector( &pos, &work->pos ) ;
	pos.vy += work->body.m_ctrl->height ;
	DG_SetPos2( &pos, &work->rot ) ;
	GM_ActObject2( &work->body ) ;

	if ( !(work->flag & FRT_MOTION_LOOP) )
	    if ( GM_CheckObject_IsEnd( &work->body, 0 )
		 work->flag |= FRT_MOTION_STOP ;
    }
}

static void Die( Work *work )
{
    GM_FreeObject( &work->body ) ;
}

static int GetResourcesCalled( Work *work, int name, int where,
			       int kms_id, int evm_id, int mar_id,
			       FVECTOR *pos, SVECTOR *rot, int flag )
{
    work->name = name ;
    work->flag = flag ;

    if ( kms_id )
	PERROR( "No kms-ID : NewFortPutMotion\n" ) ;
    GM_InitObject( &work->body, kms_id, DG_FLAG_SHADE|DG_FLAG_FINISHCALC ) ;

    _sceVu0CopyVectorXYZ( &work->pos, pos ) ;
    work->rot.vx = rot->vx ;
    work->rot.vy = rot->vy ;
    work->rot.vz = rot->vz ;

    if ( mar_id )
    {
	GM_ConfigObjectMotion( &work->body, 0, mar_id, MT_FLAG_OTHER ) ;
	GM_ConfigObjectStep( &work->body, &work->vel ) ;
	GM_ConfigObjectAction( &work->body, 0, 0, 0,0xfffff,0 ) ;
    }
    if ( evm_id )
	GM_ConfigObjectEvm( &work->body, evm_id, 0 ) ;

    DG_SetPos2( &work->pos, &DG_ZeroSVector ) ;
    GM_ActObject2( &work->body ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    IVECTOR buf ;
    FVECTOR pos ;
    SVECTOR rot ;
    int     i ;

    if ( GCL_GetOption( 'p' ) != NULL )
    {
	GCL_GetIV( GCL_NextStr(), (int *)buf ) ;
	vu0_IV0toFV( &buf, &pos ) ;
    }
    if ( GCL_GetOption( 'r' ) != NULL )
	GCL_GetSV( GCL_NextStr(), (short *)&rot ) ;

    if ( GetResourcesCalled( work, name, where,
			     GCL_GetOptionValue( 'k', 0 ),
			     GCL_GetOptionValue( 'e', 0 ),
			     GCL_GetOptionValue( 'm', 0 ),
			     &pos, &rot,
			     GCL_GetOptionValue( 'f', 0 ) ) < 0 )
	return -1 ;

    return 0 ;
}

void *NewFortPutMotion( int name, int where  )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, name, where ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
