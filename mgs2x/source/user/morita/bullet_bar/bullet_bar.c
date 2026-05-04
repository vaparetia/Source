//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   blind_shot.c
   弾が見えない所からとんで来る

   2000/10/06 T.Morita
   $Id: bullet_bar.c,v 1.1.1.3 2002/11/19 11:45:53 Yoshizawa1 Exp $
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

enum { X, Y, Z, XY=Z, W, XYZ=W, XYZW } ;

typedef struct work_bar_t
{
    GV_ACT   actor  ;

    FVECTOR  pos    ;
    int      name   ;
    int      mode   ;
    int      tics   ;
    int      shoot  ;
    float    dist   ;
} Work ;

static void Act( Work *work )
{
    extern void *NewBullet( FMATRIX *world, u_int type, u_int side, u_int size,
			    u_int damage, u_int length, u_int speed, int weapon ) ;
    FMATRIX dir ;
    FVECTOR pos ;
    GV_MSG *msg ;
    int i ;
    float dx ;
    float dz ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        if ( msg->message[0] )
	{
	    printf( "owakri\n" ) ;
	    work->shoot = 1, work->tics = -1 ;
	}
	else
	{
	    printf( "hajimari\n" ) ;
	    work->shoot = 0, work->tics = 180 ;
	}

    if ( work->tics == 170 || work->tics == 164 || work->tics == 160 ||
	 work->tics ==  80 || work->tics ==  73 || work->tics ==  40 )
    {
	_sceVu0CopyVector( &pos, &GM_PlayerControl->mov ) ;
	pos.vx += 300.0f*frnd() ;
	pos.vy += 300.0f*frnd() ;
	pos.vz += 300.0f*frnd() ;

	dir = DG_UnitMatrix ;
	_sceVu0CopyVector( (FVECTOR*)&dir.m[W], &work->pos ) ;
	_sceVu0SubVector( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[W], &pos ) ;
	_sceVu0Normalize( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[Y] ) ;
	NewBullet( &dir, BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_VISIBLE, PLAYER_SIDE, 25, 5, 15000, 800, WP_Usp ) ;
    }

    if ( work->tics <= 180 && !work->shoot )
    {
	if ( PL_PlayerResetInvincible() )
	{
	    work->shoot = 1 ;
	    dir = DG_UnitMatrix ;
	    _sceVu0CopyVector( (FVECTOR*)&dir.m[W], &work->pos ) ;
	    _sceVu0SubVector( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[W], &GM_PlayerControl->mov ) ;
	    dir.m[Y][W] = 0.0f ;
	    _sceVu0Normalize( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[Y] ) ;

	    NewBullet( &dir, BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_VISIBLE, PLAYER_SIDE, 25, 5, 15000, 800, WP_Punch ) ;
	    //GM_SeSetMode( ORGA_SE_USP_FIRE, &work->pos, GM_SEMODE_BOMB ) ;
	}
    }
    else if ( GM_PlayerStatus & PLAYER_DOWNED )
    {
	dx = work->pos.vx - GM_PlayerControl->mov.vx ;
	dz = work->pos.vz - GM_PlayerControl->mov.vz ;
	work->dist = dx * dx + dz * dz  ;
    }

    if ( work->tics > 0 )
	work->tics-- ;
    else if ( work->tics == 0 )
    {
	dx = work->pos.vx - GM_PlayerControl->mov.vx ;
	dz = work->pos.vz - GM_PlayerControl->mov.vz ;
	if ( work->dist > dx * dx + dz * dz || !(GM_PlayerStatus & PLAYER_DOWNED) )
	    work->shoot = 0, work->tics = 180 ;
    }
}

static int GetResourcesCalled( Work *work, int name, int mode, FVECTOR *pos )
{
    work->mode  = mode ;
    work->pos   = *pos ;
    work->name  = name ;
    work->shoot = 1    ;
    work->tics = -1    ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    FVECTOR pos    ;
    int     buf[3] ;

    if ( GCL_GetOption( 'p' ) )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
    }
    if ( GetResourcesCalled( work,
			     name,
			     GCL_GetOptionValue( 'm', 0 ),
			     &pos ) < 0 )
	return -1 ;
    return  0 ;
}

void *NewBulletBarriar( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
