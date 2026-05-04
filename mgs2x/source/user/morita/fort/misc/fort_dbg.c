//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_dsp.c 
   フォーチュン デバッグ用関数群

   1999/12/22 T.Morita
   $Id: fort_dbg.c,v 1.1.1.3 2002/11/19 11:46:17 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "../include/fort.h"


#ifdef DEBUG_MODE
int FRT_DbgAction = 0 ;/* 撮影用デバグアクション 本当に要らない */
int FRT_PrevAction = 0 ;

extern int FRT_DbgStatus ;


/*
  Initialize Function
*/
static Work *FRT_Work = NULL ;
void FRT_InitDebugWork( void *ptr )
{
    FRT_Work = (Work *)ptr ;
}
void FRT_FreeDebugWork()
{
    FRT_Work = NULL ;
}


/*
  Display parameters
 */
void FRT_DispParams( Work *work )
{
    DEBUG_Locate( 260, 12, 0 ) ;
    DEBUG_Printf( "LIFE %d FLG %08x ACT %04x\n", work->vitality, work->flag, work->act_flg ) ;
    if ( work->trgt_pos )
    {
	DEBUG_Printf( "TRGT %.0f %.0f ", work->trgt_pos->vx,work->trgt_pos->vz ) ;
	if ( work->trgt_pos == &work->trgt_frc )
	    DEBUG_Printf( "Trgt_Frc\n" ) ;
	else if ( work->trgt_pos == &FRT_Phase_00Pos )
	    DEBUG_Printf( "Phase_00Pos\n" ) ;
	else if ( work->trgt_pos == &FRT_Phase_0cPos )
	    DEBUG_Printf( "Phase_0cPos\n" ) ;
	else if ( work->trgt_pos == &FRT_LeftPos )
	    DEBUG_Printf( "LeftPos\n" ) ;
	else if ( work->trgt_pos == &FRT_RightPos )
	    DEBUG_Printf( "RightPos\n" ) ;
	else if ( work->trgt_pos == &FRT_HidingPos )
	    DEBUG_Printf( "HidePos\n" ) ;
	else
	    DEBUG_Printf( "NONE\n" ) ;
    }
    if ( work->ply_hide & FRT_F_PLYR_HIDING )
	DEBUG_Printf( "PLAYER HIDE %d ID%d\n",
		     work->ply_hide & FRT_F_PLYR_HIDTIM,
		     (work->ply_hide & FRT_F_PLYR_HIDPOS)>>6 ) ;
    else
	DEBUG_Printf( "PLAYER HIDEEXP %d\n", (work->ply_hide & FRT_F_PLYR_HIDEXP) ) ;
    DEBUG_Printf( "PHASE %04x PlyTime%d\n", work->act_phase, work->ply_time ) ;
}


void FRT_DummyFunc( char *fmt, ... )
{
}

void FRT_Debug( Work *work )
{
    if( GM_DebugModeEnable )
    {
#if 0
	static int flg = 0 ;

	if ( (GV_PadData[1].status & PAD_R1) && (GV_PadData[1].press & PAD_X) )
	    flg ^= 1 ;
	if ( flg )
	{
	    work->lights[1].m[3][0] = 68 ;
	    work->lights[1].m[3][1] = 68 ;
	    work->lights[1].m[3][2] = 68 ;
	}

	if ( GV_PadData[1].press & PAD_U )
	    FRT_PrevAction++ ;
	if ( GV_PadData[1].press & PAD_D )
	    FRT_PrevAction-- ;
	if ( GV_PadData[1].press & PAD_X )
	    FRT_DbgAction = FRT_PrevAction ;
	if ( GV_PadData[1].press & PAD_R1 )
	    FRT_SendHoloMessage( 4 ) ;
#endif

	/*for debug EYES*/
	if ( FRT_DbgStatus & 1 )
	{
	    if ( work->trgt_pos )
		AN_Test_Eye2( work->trgt_pos, 2 ) ;
	    if ( work->trgt_aim )
		AN_Test_Eye2( work->trgt_aim, 2 ) ;
	}
    }

}




#undef printf
#define SOUND_TEST 0
#define MAX_FUNCLIST 20 

typedef struct
{
    GV_ACT  actor ;
    int name ;
} DbgWork  ;

static void Act( DbgWork *work )
{
    int i, j ;
    extern void FRT_DispParams( Work *work ) ;
    extern struct dbglst_t { void *adr ; char *name ; } FRT_ActionFunctionLists[] ;
    struct dbglst_t *lst ;
    static void (**FuncList[MAX_FUNCLIST])( Work * ) = {
	NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL, NULL,
    } ;
    static int    nFuncList = 0 ;

    if( GM_DebugModeEnable )
    {
	if ( FRT_DbgStatus )
	    FRT_DispParams( FRT_Work ) ;
	if ( FuncList[nFuncList] != FRT_Work->act )
	    FuncList[ nFuncList = (nFuncList + 1) % MAX_FUNCLIST ] = FRT_Work->act ;

#if SOUND_TEST
	/* voice test */
	if ( FRT_DbgStatus & 1 )
	{
	    static int voice_id = 0 ;
	    static int voice_ids[] = 
	    {
	    } ;

	    if ( GV_PadData[1].press & PAD_U )
		voice_id++, printf( "%d\n", voice_id ) ;
	    if ( GV_PadData[1].press & PAD_D )
		voice_id--, printf( "%d\n", voice_id ) ;
	    if ( voice_id >= sizeof(voice_ids)/sizeof(int) )
		voice_id = 0 ;
	    if ( voice_id < 0 )
		voice_id = sizeof(voice_ids)/sizeof(int)-1 ;
	    if ( GV_PadData[1].press & PAD_A )
		GM_SeSetMode( voice_ids[voice_id], &FRT_Work->control.mov, GM_SEMODE_BOMB ) ;
	}
#endif

	if ( GV_PadData[1].press & PAD_L1 && FRT_DbgStatus )
	{
	    printf( "\n-----------FortInfo DebugDump-----------\n" ) ;

	    if ( FRT_Work )
	    {
		printf( "Control mov%.1f %.1f %.1f\n",
			FRT_Work->control.mov.vx , FRT_Work->control.mov.vy , FRT_Work->control.mov.vz
		    ) ;
		printf( "        rot  %4d %4d %4d\n        turn %4d %4d %4d \n",
			FRT_Work->control.rot.vx , FRT_Work->control.rot.vy , FRT_Work->control.rot.vz ,
			FRT_Work->control.turn.vx , FRT_Work->control.turn.vy , FRT_Work->control.turn.vz 
		    ) ;
		printf( "Object layer0 mtn%d plytime%f flag%x\n       layer1 mtn%d plytime%f flag %x\n",
			FRT_Work->body.m_ctrl->mt3_ctrl[0].motion_num,
			FRT_Work->body.m_ctrl->mt3_ctrl[0].play_time,
			FRT_Work->body.m_ctrl->mt3_ctrl[0].flag,
			FRT_Work->body.m_ctrl->mt3_ctrl[1].motion_num,
			FRT_Work->body.m_ctrl->mt3_ctrl[1].play_time,
			FRT_Work->body.m_ctrl->mt3_ctrl[1].flag
		    ) ;
		printf( "PosAdj %.1f %.1f %.1f  w%.0f \n",
			FRT_Work->pos_adj.vx , FRT_Work->pos_adj.vy , FRT_Work->pos_adj.vz, FRT_Work->pos_adj.vw 
		    ) ;
		printf( "  Flag %08x ActFlag %04x\n",
			FRT_Work->flag,  FRT_Work->act_flg
		    ) ;

#if 1
		if ( FRT_Work->act )
		{
		    for( j=0, i=nFuncList ; j<MAX_FUNCLIST ; j++, i=(i+(MAX_FUNCLIST-1))%MAX_FUNCLIST )
		    {
			for ( lst=FRT_ActionFunctionLists ; lst->adr ; lst++ )
			    if ( FuncList[i] )
				if ( *FuncList[i] == lst->adr )
				    break ;
			printf( "%2d Act %s ", j, lst->name ) ;
			for ( lst=FRT_ActionFunctionLists ; lst->adr ; lst++ )
			    if ( FuncList[i] )
				if ( FuncList[i] == lst->adr )
				    break ;
			if ( strcmp( lst->name, "Undefined Function" ) )
			    printf( "[%s]", lst->name ) ;
			printf( "\n" ) ;
		    }
		}
#endif

		printf( "StllMtn %d StllTim %d WaitSec %d Vitality %d\n",
			FRT_Work->stll_mtn, FRT_Work->stll_tim, FRT_Work->wait_sec, FRT_Work->vitality
		    ) ;

		printf( " WeapBlt %d\n WeapLnr %.0f %.0f %.0f %.0f\n"
			" WeapAmo %d\n",
			FRT_Work->weap_blt,
			FRT_Work->weap_lnr.vx,FRT_Work->weap_lnr.vy,FRT_Work->weap_lnr.vz,FRT_Work->weap_lnr.vw,
			FRT_Work->weap_amo
		    ) ;


		if ( FRT_Work->trgt_eye )
		    printf( "TrgtEye %.0f %.0f %.0f\n",
			    FRT_Work->trgt_eye->vx,FRT_Work->trgt_eye->vy,FRT_Work->trgt_eye->vz ) ;
		if (FRT_Work->trgt_aim )
		    printf( "TrgtAim %.0f %.0f %.0f\n",
			    FRT_Work->trgt_aim->vx,FRT_Work->trgt_aim->vy,FRT_Work->trgt_aim->vz ) ;
		printf( "TrgtAimPOS %.0f %.0f %.0f\n",
			FRT_Work->trgt_aim_pos.vx,FRT_Work->trgt_aim_pos.vy,FRT_Work->trgt_aim_pos.vz ) ;
		if (FRT_Work->trgt_pos )
		    printf( "TrgtPos %.0f %.0f %.0f\n",
			    FRT_Work->trgt_pos->vx,FRT_Work->trgt_pos->vy,FRT_Work->trgt_pos->vz ) ;
		printf( "TrgtHid %.0f %.0f %.0f\n",
			FRT_Work->trgt_hid.vx,FRT_Work->trgt_hid.vy,FRT_Work->trgt_hid.vz ) ;
		printf( "TrgtNse %.0f %.0f %.0f %.1f\n",
			FRT_Work->trgt_nse.vx,FRT_Work->trgt_nse.vy,FRT_Work->trgt_nse.vz,FRT_Work->trgt_nse.vw ) ;

	    }

	    printf( "\n-----------PlayerInfo DebugDump-----------\n" ) ;
	    printf( "Control mov%.1f %.1f %.1f\n",
		    GM_PlayerControl->mov.vx , GM_PlayerControl->mov.vy , GM_PlayerControl->mov.vz
		) ;
	    printf( "        rot  %4d %4d %4d\n        turn %4d %4d %4d \n",
		    GM_PlayerControl->rot.vx , GM_PlayerControl->rot.vy , GM_PlayerControl->rot.vz ,
		    GM_PlayerControl->turn.vx, GM_PlayerControl->turn.vy, GM_PlayerControl->turn.vz 
		) ;
	    printf( "Status %08x \n", GM_PlayerStatus ) ;
	    printf( "Waist Diff %f\n", FRT_PLY_CAMERAPOS.vy - GM_PlayerControl->mov.vy ) ;

	    /* プレイヤー情報 */
	    printf( "  ply_shoot %.0f %.0f %.0f\n",
		    FRT_Work->ply_shoot.vx,FRT_Work->ply_shoot.vy,FRT_Work->ply_shoot.vz ) ;
	    printf( "  ply_throw %.0f %.0f %.0f\n",
		    FRT_Work->ply_throw.vx,FRT_Work->ply_throw.vy,FRT_Work->ply_throw.vz ) ;
	    printf( "  ply_time  %d\n",FRT_Work->ply_time );
	    printf( "  ply_hide  HIDING%d HIDLNG%d HIDTIM%d HIDEXP%d HIDPOS%d\n",
		    (FRT_Work->ply_hide & FRT_F_PLYR_HIDING)>>15,
		    (FRT_Work->ply_hide & FRT_F_PLYR_HIDLNG)>>14,
		    (FRT_Work->ply_hide & FRT_F_PLYR_HIDTIM),
		    (FRT_Work->ply_hide & FRT_F_PLYR_HIDEXP)>>12,
		    (FRT_Work->ply_hide & FRT_F_PLYR_HIDPOS)>>6 ) ;
	    printf( "  ply_stop  %d\n",FRT_Work->ply_stop );
	    printf( "  ply_amo_m9 %d\n",FRT_Work->ply_amo_m9 ) ;
	    printf( "  ply_amo_us %d\n",FRT_Work->ply_amo_us ) ;

	}
    }
}

void *NewFortDebugDump( int name, int where )
{
    DbgWork *work ;

    if ( (work = (DbgWork *)GV_NewActor( GV_ACTOR_PREV2, sizeof( DbgWork ) )) )
    {
	GV_SetActor( &work->actor, Act, NULL ) ;
	work->name = name ;
    }
    return work ;
}





#endif  /*一番上の DEBUG_MODE */
