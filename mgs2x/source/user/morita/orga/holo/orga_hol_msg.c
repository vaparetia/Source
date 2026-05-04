//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_hol_msg.c 
   オルガ オルガ戦専用プットホロオブジェ

   2000/01/21 T.Morita
   $Id: orga_hol_msg.c,v 1.1.1.3 2002/11/19 11:46:24 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "orga_holo.h"

void ORG_HOL_Message( Work *work )
{
    GV_MSG *msg ;
    int i, idx ;

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        switch( msg->message[0] )
        {
        case 0:
            *((int **)msg->message[1]) = &work->flag ;
	    break ;

        case 1:
	    if ( ORG_HOL_OrgaFreeIdx[work->flag] )
	    {
		work->key_idx = ORG_HOL_OrgaFreeIdx[work->flag] ;
		work->ratio = 256 ;
		work->key   = 0   ;
		work->tics  = 0   ;
	    }
            break ;

        case 2:
	    if ( (idx = ORG_HOL_PlyFreeLftIdx[work->flag]) )
	    {
		work->key_idx = idx ;
		work->ratio = 256 ;
		work->key   = 0   ;
		work->tics  = 0   ;
	    }
            break ;

        case 3:
	    if ( (idx = ORG_HOL_PlyFreeRgtIdx[work->flag]) ) 
	    {
		work->key_idx = idx ;
		work->ratio = 256 ;
		work->key   = 0   ;
		work->tics  = 0   ;
	    }
            break ;


        case 5:
	    work->himo_l.vx = (float)msg->message[1] ;
	    work->himo_l.vy = (float)msg->message[2] ;
	    work->himo_l.vz = (float)msg->message[3] ;
	    printf( "HOLO MESSAGE L %.2f %.2f %.2f\n",
		    work->himo_l.vx, work->himo_l.vy, work->himo_l.vz ) ;
            break ;
        case 6:
	    work->himo_r.vx = (float)msg->message[1] ;
	    work->himo_r.vy = (float)msg->message[2] ;
	    work->himo_r.vz = (float)msg->message[3] ;
	    printf( "HOLO MESSAGE R %.2f %.2f %.2f\n",
		    work->himo_r.vx, work->himo_r.vy, work->himo_r.vz ) ;
            break ;

        case 7:
	    DG_InvisibleObjs( work->objs_r ) ;
	    break ;

        case 8:/*オルガ開いている*/
	    work->key_idx = 3-1 ;
	    work->ratio = 256 ;
	    work->tics  = 0   ;
	    work->key = ORG_HOL_Frames[work->key_idx].key_frm - 1 ;
            break ;

        case 9:/*右開いている*/
	    work->key_idx = 8-1 ;
	    work->ratio = 256 ;
	    work->tics  = 0   ;
	    work->key = ORG_HOL_Frames[work->key_idx].key_frm - 1 ;
            break ;

        case 10:/*左開いている*/
	    work->key_idx = 13-1 ;
	    work->ratio = 256 ;
	    work->tics  = 0   ;
	    work->key = ORG_HOL_Frames[work->key_idx].key_frm - 1 ;
            break ;

        case 11:/*飛沫ＯＦＦ*/
	    if ( work->splash )
		ORG_SPH_VisibleSplash( work->splash, 0 ) ;
            break ;
        case 12:/*飛沫ＯＮ*/
	    if ( work->splash )
		ORG_SPH_VisibleSplash( work->splash, 1 ) ;
            break ;





        case 13:/*オルガ飛ばす*/
	    work->key_idx = 4 ;
	    work->ratio = 256 ;
	    work->tics  = 0   ;
	    work->key = ORG_HOL_Frames[work->key_idx].key_frm - 1 ;
            break ;

        case 14:/*右飛ばす*/
	    work->key_idx = 9 ;
	    work->ratio = 256 ;
	    work->tics  = 0   ;
	    work->key = ORG_HOL_Frames[work->key_idx].key_frm - 1 ;
            break ;

        case 15:/*左飛ばす*/
	    work->key_idx = 14 ;
	    work->ratio = 256 ;
	    work->tics  = 0   ;
	    work->key = ORG_HOL_Frames[work->key_idx].key_frm - 1 ;
            break ;


        case 16:/*左飛ばす*/
	    work->next = 1 ;
            break ;


#if DEBUG_MODE
        case 4:/*ホロ再生をリセットする*/
	{
	    GM_FreeTarget( &work->target ) ;
	    DG_VisibleObjs( work->objs ) ;
	    DG_VisibleObjs( work->objs_r ) ;
	    ORG_HOL_InitHoloParam( work ) ;
	    UTL_LzshDecodeChangeIn( work->lzh, GV_GetCache( GV_CacheID(work->id[work->flag], 'a') ) ) ;
	    UTL_LzshDecodeRewindOut( work->lzh ) ;
	    GM_PutTarget( &work->target ) ;
	    GM_FreeTarget( &work->bul_wall ) ;
	    _sceVu0CopyMatrix( &work->objs_r->world, &work->org_mtx ) ;
	    _sceVu0CopyMatrix( &work->objs->world  , &work->org_mtx ) ;
	    _sceVu0ScaleVector( (FVECTOR *)&work->objs_r->world.m[Y],
				(FVECTOR *)&work->objs_r->world.m[Y], -1.0f ) ;
            break ;
	}
#endif
        }
}


void ORG_HOL_GetStringPos( FVECTOR *a )
{
    ASSERT( a ) ;

    if ( ORG_HOL_Work )
	_sceVu0CopyVector( a, &ORG_HOL_Work->himo_l ) ;
}
