//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_hol_act.c 
   オルガ オルガ戦専用プットホロオブジェ

   2000/01/21 T.Morita
   $Id: orga_hol_act.c,v 1.4 2002/11/23 12:28:38 Yoshizawa1 Exp $
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

//BP - endian functions.
#include "BP_EndianSupport.h"


static void MakeupShape( short *a, short *b, int i, int ratio )
{
    SVECTOR *svec = (SVECTOR*)SCRPAD_ADDR ;

    while( --i>=0 )
    {
	svec->vx  = ((int)a[X]*ratio + (int)b[X]*(256-ratio))/256 ;
	svec->vy  = ((int)a[Y]*ratio + (int)b[Y]*(256-ratio))/256 ;
	svec->vz  = ((int)a[Z]*ratio + (int)b[Z]*(256-ratio))/256 ;
#ifndef KP_WINDOWS
	svec->pad = 4096 ;
#else
	svec->pad = 0x7fff ;
#endif
	svec++ ;
	a += XYZ ;
	b += XYZ ;
    }
}

static void MakeupNormal( short *a, short *b, int i, int ratio )
{
    SVECTOR *svec = (SVECTOR*)SCRPAD_ADDR ;

    while( --i>=0 )
    {
	svec->vx  = ((int)a[X]*ratio + (int)b[X]*(256-ratio))/256 ;
	svec->vy  = ((int)a[Y]*ratio + (int)b[Y]*(256-ratio))/256 ;
	svec->vz  = ((int)a[Z]*ratio + (int)b[Z]*(256-ratio))/256 ;
#ifdef XBOX
		svec->vx *= 16 ;
		svec->vy *= 16 ;
		svec->vz *= 16 ;
#endif

#ifndef KP_WINDOWS
	svec->pad = 4096 ;
#else
	svec->pad = 0x7fff ;
#endif
	svec++ ;
	a += XYZ ;
	b += XYZ ;
    }
}


void ORG_HOL_ActSound( Work *work )
{
    static int toriaezu  = 0 ;
    static int toriaezu2 = 1 ;

    switch ( work->flag )
    {
    case ORGA_HOL_ORG_GONE:
    case ORGA_HOL_PLY_GONL:
    case ORGA_HOL_PLY_GONR:
	if ( toriaezu2 )
	{
	    if ( work->tics++ == 60/2 )
		GM_SeSetMode( ORGA_SE_BLW_HOLO1, (FVECTOR*)&work->objs->world.m[3], GM_SEMODE_BOMB ) ;
	    if ( work->tics == 120/2 )
		toriaezu2 = 0 ;
	}
	else
	{
	    work->tics = (work->tics + 1) %54 ;
	    if ( work->tics == 7+toriaezu )
		GM_SeSetMode( ORGA_SE_BLW_HOLO2, (FVECTOR*)&work->objs->world.m[3], GM_SEMODE_NORMAL );
	    else if ( work->tics == 38/2 )
		toriaezu = irnd()&7,
		    GM_SeSetMode( ORGA_SE_BLW_HOLO3, (FVECTOR*)&work->objs->world.m[3], GM_SEMODE_NORMAL ) ;
	}
	break ;

    case ORGA_HOL_ORG_FREE:
	work->tics = (work->tics + 1) %(48/2) ;
	if ( work->tics == 7+toriaezu )
	    GM_SeSetMode( ORGA_SE_BLW_HOLO2, (FVECTOR*)&work->objs->world.m[3], GM_SEMODE_NORMAL );
	else if ( work->tics == 32/2 )
	    toriaezu = irnd()&7,
		GM_SeSetMode( ORGA_SE_BLW_HOLO3, (FVECTOR*)&work->objs->world.m[3], GM_SEMODE_NORMAL ) ;
	break ;

    case ORGA_HOL_ORG_OPEN:
	if ( work->tics++ == 60/2 )
	    GM_SeSetMode( ORGA_SE_BLW_HOLO1, (FVECTOR*)&work->objs->world.m[3], GM_SEMODE_BOMB ) ;
    }
}

void ORG_HOL_ActDecode( Work *work )
{
    if ( (work->ratio += work->rate) >=256 )
    {
	   work->key++ ;
	   if ( work->flag == ORGA_HOL_ORG_GONE ||
	        work->flag == ORGA_HOL_PLY_GONL ||
	        work->flag == ORGA_HOL_PLY_GONR )
	       work->rate  = 64  ;
	   else
	       work->rate  = 128 ;
	   work->ratio = 0   ;
	   work->tgl  ^= 1   ;

	   if ( ORG_HOL_Frames[work->key_idx].key_frm == work->key )
	   {
	       if ( work->next && ORG_HOL_Frames[work->key_idx].nxt_idx != -1  )
	       {
		   if ( work->key_idx == ORG_HOL_Frames[work->key_idx].nxt_idx )
		       work->key_idx++ ;
		   else if ( ORG_HOL_Frames[work->key_idx].nxt_idx ==
			     ORG_HOL_Frames[ORG_HOL_Frames[work->key_idx].nxt_idx].nxt_idx )
		       work->key_idx += 2 ;
	       }
	       work->key     = 0 ;
	       work->flag    = ORG_HOL_Frames[work->key_idx].nxt_flg ;
	       work->rate    = ORG_HOL_Frames[work->key_idx].rate    ;
	       work->key_idx = ORG_HOL_Frames[work->key_idx].nxt_idx ;
	       if ( work->key_idx == -1 )
	       {
		   work->key = 0 ;
		   return ;
	       }
	       UTL_LzshDecodeChangeIn( work->lzh, GV_GetCache( GV_CacheID(work->id[work->flag], 'a') ) ) ;
	   }

	   if ( !UTL_LzshStreamDecode( work->lzh ) )
	       UTL_LzshDecodeRewindIn( work->lzh ) ;
	   if ( work->tgl )
	       UTL_LzshDecodeRewindOut( work->lzh ) ;

       //BP - byte swap streaming data.
       BP_LE_SwapSShortArray_Inp( work->buffer[work->tgl], 2 * XYZ * ORGA_HOL_N_VERTS );
    }

    DG_SwitchVAnimeBuffer( &work->v_anm ) ;

    MakeupShape( work->buffer[work->tgl  ],
		 work->buffer[work->tgl^1],
		 ORGA_HOL_N_VERTS, work->ratio ) ;
    DG_RefineStripVertex( &work->v_anm, work->cdef->models[0].verts_index ) ; 
    MakeupNormal( work->buffer[work->tgl  ] + ORGA_HOL_N_VERTS*XYZ,
		 work->buffer[work->tgl^1] + ORGA_HOL_N_VERTS*XYZ,
		 ORGA_HOL_N_VERTS, work->ratio ) ;
    DG_RefineStripNormal( &work->v_anm, work->cdef->models[0].verts_index ) ; 
}


void ORG_HOL_ActTurnOffMirror( Work *work )
{
    /* 映り込みを消す */
    if ( work->flag == ORGA_HOL_ORG_GONE ||
	 work->flag == ORGA_HOL_PLY_GONL ||
	 work->flag == ORGA_HOL_PLY_GONR )
	if ( !work->ratio )
	{
	    if ( work->key == 1 )
	    {
		extern void ORG_SendOrgaExitHoloMessage() ;
		ORG_SendOrgaExitHoloMessage() ;
		GM_FreeTarget( &work->bul_wall ) ;
	    }
	    else if ( work->key == 100 )
		if ( work->pr_name )
		    GCL_ExecProc( work->pr_name, NULL ) ;
	}
}

void ORG_HOL_ActHoloGone( Work *work )
{
    if ( work->flag == ORGA_HOL_ORG_GONE ||
	 work->flag == ORGA_HOL_PLY_GONL ||
	 work->flag == ORGA_HOL_PLY_GONR )
    {
	static FVECTOR HoloAccel  = { 0.0f, 0.125f, 0.5f, 0.0f } ;
	static FVECTOR HoloAccel2 = { 0.0f,   1.0f, 4.0f, 0.0f } ;

	/* カットイン後 さらに加速 カットインのフレーム数は１２０ */
	if ( work->key > 33 )
	    _sceVu0AddVector( &work->vel, &work->vel, &HoloAccel2 ) ;
	if ( work->key == 18 )
	    DG_InvisibleObjs( work->objs_r ) ;

	_sceVu0AddVector( &work->vel, &work->vel, &HoloAccel ) ;
	_sceVu0AddVector( (FVECTOR*)&work->objs->world.m[W],
			  (FVECTOR*)&work->objs->world.m[W], &work->vel ) ;
	work->objs->world.m[W][W] = 1.0f ;
    }
}
