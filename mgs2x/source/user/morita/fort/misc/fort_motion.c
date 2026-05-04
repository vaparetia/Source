//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_motion.c 
   モーション再生

   2001/01/16 T.Morita
   $Id: fort_motion.c,v 1.1.1.3 2002/11/19 11:46:18 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../../include/util.h"
#include "../../include/libdg_x.h"

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

typedef void (* FUNC)( int ) ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR   pos    ;
    FVECTOR   vel    ;
    SVECTOR   rot    ;
    OBJECT    body   ;

    int       proc     ;
    FUNC      callback ;

    int       flag   ;
    int       name   ;
} Work ;



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
	    work->pos.vx = (float)msg->message[1] ;
	    work->pos.vy = (float)msg->message[2] ;
	    work->pos.vz = (float)msg->message[3] ;
	    break ;
	case 2:
	    work->flag |=  FRT_MOTION_STOP ;
	    break ;
	case 3:
	    if ( work->callback )
		GM_SeSetMode( SD_A_EVDMOV01, &work->pos, GM_SEMODE_NORMAL ) ;
	    work->flag &= ~FRT_MOTION_STOP ;
	    GM_ConfigObjectAction( &work->body, 0, msg->message[1], 0, 0xfffff, msg->message[2] ) ;
	    break ;
	case 4:
	    if ( work->callback )
		GM_SeSetMode( SD_A_EVDMOV01, &work->pos, GM_SEMODE_NORMAL ) ;
	    work->flag &= ~FRT_MOTION_STOP ;
	    work->flag |=  FRT_MOTION_LOOP ;
	    GM_ConfigObjectAction( &work->body, 0, msg->message[1], 0, 0xfffff, msg->message[2] ) ;
	    break ;

	}
}

static void ActMotion( Work *work )
{
    FVECTOR pos ;

    /* モーション計算 */
    GM_ActMotion( &work->body ) ;
    if ( !(work->flag & FRT_MOTION_NOMOVE) )
	_sceVu0AddVector( &work->pos, &work->pos, &work->vel ) ;
    _sceVu0CopyVector( &pos, &work->pos ) ;

    /* 高さを腰に合わせる */
    pos.vy += work->body.m_ctrl->height ;

    /* モデルに反映 */
    DG_SetPos2( &pos, &work->rot ) ;
    GM_ActObject2( &work->body ) ;

    /* ループが掛かる */
    if ( !(work->flag & FRT_MOTION_LOOP) )
	if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
	{
	    if ( work->proc )
		GCL_ExecProc( work->proc, NULL ) ;
	    if ( work->callback )
	    {
		(*work->callback)( work->body.m_ctrl->mt3_ctrl->motion_num ) ;
		switch( work->body.m_ctrl->mt3_ctrl->motion_num )
		{
		case 0:
		    GM_SeSetMode( SD_A_EVDOPN01, &work->pos, GM_SEMODE_NORMAL ) ;
		    break ;
		case 1:
		    GM_SeSetMode( SD_A_EVDCLO01, &work->pos, GM_SEMODE_NORMAL ) ;
		    break ; 
		}
	    }

	    work->flag |= FRT_MOTION_STOP ;
	}
}

static void Act( Work *work )
{
    ActRecieveMessage( work ) ;

    if ( !(work->flag & FRT_MOTION_STOP) )
	ActMotion( work ) ;
}

static void Die( Work *work )
{
    GM_FreeObject( &work->body ) ;
}

static int GetResourcesCalled( Work *work, int name, int where,
			       int kms_id, int evm_id, int mar_id, int mar_num,
			       FVECTOR *pos, SVECTOR *rot,
			       int proc, int flag, FUNC callback )
{
    work->name = name ;
    work->flag = flag ;
    work->proc = proc ;

    _sceVu0CopyVectorXYZ( &work->pos, pos ) ;
    work->rot.vx = rot->vx ;
    work->rot.vy = rot->vy ;
    work->rot.vz = rot->vz ;

    work->callback = callback ;

    if ( !kms_id )
	PERROR( "No kms-ID : NewFortPutMotion\n" ) ;
    GM_InitObject( &work->body, kms_id, DG_FLAG_SHADE|DG_FLAG_FINISHCALC ) ;

    if ( !mar_id )
	PERROR( "No mar-ID : NewFortPutMotion\n" ) ;
    GM_ConfigObjectMotion( &work->body, 0, mar_id, MT_FLAG_OBJECT ) ;
    GM_ConfigObjectStep( &work->body, &work->vel ) ;
    GM_ConfigObjectAction( &work->body, 0, mar_num, 0,0xfffff,0 ) ;

    if ( evm_id )
	GM_ConfigObjectEvm( &work->body, evm_id, 0 ) ;

    ActMotion( work ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    IVECTOR buf ;
    FVECTOR pos ;
    SVECTOR rot ;

    if ( GCL_GetOption( 'p' ) != NULL )
    {
	GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	vu0_IV0toFV( &buf, &pos ) ;
    }
    if ( GCL_GetOption( 'r' ) != NULL )
	GCL_GetSV( GCL_NextStr(), (short *)&rot ) ;

    if ( GetResourcesCalled( work, name, where,
			     GCL_GetOptionValue( 'k', 0 ),
			     GCL_GetOptionValue( 'e', 0 ),
			     GCL_GetOptionValue( 'm', 0 ),
			     GCL_GetOptionValue( 'N', 0 ),
			     &pos, &rot,
			     GCL_GetOptionValue( 'O', 0 ),
			     GCL_GetOptionValue( 'f', 0 ),
			     NULL ) < 0 )
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

void *NewFortPutMotion_Called( int name, int where,
			       int kms_id, int evm_id, int mar_id, int mar_num,
			       FVECTOR *pos, SVECTOR *rot,
			       int flag, void *callback )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	if ( GetResourcesCalled( work, name, where,
				 kms_id, evm_id, mar_id, mar_num,
				 pos, rot,
				 0, flag, callback ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
