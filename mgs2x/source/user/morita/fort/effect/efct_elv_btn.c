//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  efct_elv_btn.c
  フォーチュン戦 エレベータボタン

  2000/12/18 T.Morita
  $Id: efct_elv_btn.c,v 1.1.1.3 2002/11/19 11:46:09 Yoshizawa1 Exp $
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
#include "../../brk_utl/brk_utl.x"

#include "BP_Misc.h"
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    FVECTOR   up    ;
    FVECTOR   down  ;

    DG_PRIM2 *button ;
    int mode ;
    int name ;
} Work ;

extern int NewComPUT_ELV_PlayerInside() ;

static void ActRecieveMessage( Work *work, DG_PRIM2_UVRGBWH *nxt_bu )
{
    GV_MSG *msg ;
    int     i   ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch ( msg->message[0] )
	{
	case 0:
	    work->mode = 4 ;
	    break ;
	case 1:
	    work->mode =-4 ;
	    break ;

	case 2:/*緑に変更*/
printf( "Change to Blue\n" );
	    nxt_bu->r =  16 ;
	    nxt_bu->g =  64 ;
	    nxt_bu->b = 100 ;
	    break ;
	case 3:/*赤に変更*/
	    nxt_bu->r = 100 ;
	    nxt_bu->g =  32 ;
	    nxt_bu->b =   8 ;
	    break ;

	case 4:/*上に変更*/
	    nxt_bu->a  =  0 ;
	    work->mode = -2 ;
	    break ;
	case 5:/*下に変更*/
	    nxt_bu->a  =  0 ;
	    work->mode =  2 ;
	    break ;
	}
}

static void Act( Work *work )
{
    DG_PRIM2         *b ;
    FVECTOR          *nxt_bp, *prv_bp ;
    DG_PRIM2_UVRGBWH *nxt_bu, *prv_bu ;

    /*プリミティブを更新*/
    b = work->button  ;
    prv_bp = b->pos  [b->buffer_clock] ;
    prv_bu = b->uvrgb[b->buffer_clock] ;
    DG_SwitchBuffPrim2( b ) ;
    nxt_bp = b->pos  [b->buffer_clock] ;
    nxt_bu = b->uvrgb[b->buffer_clock] ;

    /*色を更新*/
    nxt_bu->a = prv_bu->a ;
    nxt_bu->r = prv_bu->r ;
    nxt_bu->g = prv_bu->g ;
    nxt_bu->b = prv_bu->b ;

    /* メッセージ処理 */
    ActRecieveMessage( work, nxt_bu ) ;

    /* 点滅かどうか */
    if ( work->mode/2 )
    {
	if ( nxt_bu->a )
	{

      if ( BP_IsPAL()==TRUE )
      {
         if ( GV_Time % 6 )
		      nxt_bu->a -= nxt_bu->a>3 ? 3 : nxt_bu->a ;
	      else
		      nxt_bu->a -= nxt_bu->a>4 ? 4 : nxt_bu->a ;
      }
      else
         nxt_bu->a -= nxt_bu->a>3 ? 3 : nxt_bu->a ;

	}
	else
	{
	    nxt_bu->a = 128 ;
	    work->mode += work->mode>0 ? -1 : 1 ;
	    if ( !(work->mode/2) )
	    {
		if ( work->mode > 0 )
		{
		    _sceVu0CopyVector( nxt_bp, &work->up   ) ;
		    nxt_bu->r = 100 ;
		    nxt_bu->g =  32 ;
		    nxt_bu->b =   8 ;
		}
		else
		{
		    _sceVu0CopyVector( nxt_bp, &work->down ) ;
		    nxt_bu->r = 100 ;
		    nxt_bu->g =  32 ;
		    nxt_bu->b =   8 ;
		}
		if ( !NewComPUT_ELV_PlayerInside() )
		    GM_SeSetMode( SD_A_EVCALL03, &work->up, GM_SEMODE_BOMB ) ;
	    }
	    else if ( !NewComPUT_ELV_PlayerInside() )
		GM_SeSetMode( SD_A_EVCALL02, &work->up, GM_SEMODE_BOMB ) ;
	}
    }
    else
	_sceVu0CopyVector( nxt_bp, prv_bp ) ;
}

static void Die( Work *work )
{
    if ( work->button )
	GM_FreePrim2( work->button ) ;
}

#define BUTTON_SIZE 400.0f
static int GetResourcesCalled( Work *work, int name, FVECTOR *up, FVECTOR *down, int mode )
{
    _sceVu0CopyVector( &work->up  , up   ) ;
    _sceVu0CopyVector( &work->down, down ) ;
    work->up.vw   = 1.0f ;
    work->down.vw = 1.0f ;
    work->name = name ;

    if ( !(work->button = BRK_UTL_MakeSPRTWH( 1,
					      2055109/*ray_eye_bonbori_alp*/,
					      SCE_GS_SET_ALPHA(0,2,0,1,0),
					      BUTTON_SIZE,
					      (128<<24)|(16 << 0)|(64<< 8)|(100<<16))) )
	PERROR( "No Prim(no memory) : NewFortElevatorButton\n" ) ;

    _sceVu0CopyVector( work->button->pos[0], mode ? &work->up : &work->down ) ;
    _sceVu0CopyVector( work->button->pos[1], mode ? &work->up : &work->down ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    FVECTOR fvtemp0 ;
    FVECTOR fvtemp1 ;

    if ( !GCL_GetOption( 'u' ) )
	PERROR( "No up position (-up option missing)  : NewFortElevatorButton\n" ) ;
    fvtemp0.vx = (float)GCL_GetNextInt() ;
    fvtemp0.vy = (float)GCL_GetNextInt() ;
    fvtemp0.vz = (float)GCL_GetNextInt() ;

    if ( !GCL_GetOption( 'd' ) )
	PERROR( "No down position (-down option missing)  : NewFortElevatorButton\n" ) ;
    fvtemp1.vx = (float)GCL_GetNextInt() ;
    fvtemp1.vy = (float)GCL_GetNextInt() ;
    fvtemp1.vz = (float)GCL_GetNextInt() ;

    if ( GetResourcesCalled( work, name,
			     &fvtemp0, &fvtemp1,
			     GCL_GetOptionValue( 'm', 1 ) ) < 0 )
	return -1 ;
    return 0 ;
}


void *NewFortElevatorButton( int name, int where )
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

void *NewFortElevatorButtonCalled( int name, FVECTOR *up, FVECTOR *down, int mode )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResourcesCalled( work, name, up, down, mode ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }

    return (void *)work ;
}
